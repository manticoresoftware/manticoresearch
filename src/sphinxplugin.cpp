//
// $Id$
//

//
// Copyright (c) 2001-2013, Andrew Aksyonoff
// Copyright (c) 2008-2013, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxplugin.h"

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif // HAVE_DLOPEN
#endif // !USE_WINDOWS

#if !USE_WINDOWS
#ifndef HAVE_DLERROR
#define dlerror() ""
#endif // HAVE_DLERROR
#endif // !USE_WINDOWS

//////////////////////////////////////////////////////////////////////////
// TYPES
//////////////////////////////////////////////////////////////////////////

/// loaded plugin library
struct PluginLib_t
{
	void *				m_pHandle;				///< handle from dlopen()
	int					m_dCount[PLUGIN_TOTAL]; ///< per-type plugin counts from this library
	PluginReinit_fn		m_fnReinit;				///< per-library reinitialization func (for prefork), optional
};

/// plugin key
struct PluginKey_t
{
	PluginType_e		m_eType;
	CSphString			m_sName;

	PluginKey_t()
	{}

	PluginKey_t ( PluginType_e eType, const char * sName )
		: m_eType ( eType )
		, m_sName ( sName )
	{
		m_sName.ToLower();
	}

	static int Hash ( const PluginKey_t & v )
	{
		return sphCRC32 ( (const BYTE*)v.m_sName.cstr(), v.m_sName.Length(),
			sphCRC32 ( (const BYTE*)&v.m_eType, sizeof(v.m_eType) ) );
	}

	bool operator == ( const PluginKey_t & rhs )
	{
		return m_eType==rhs.m_eType && m_sName==rhs.m_sName;
	}
};

//////////////////////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////////////////////

const char * g_dPluginTypes[PLUGIN_TOTAL] = { "udf", "ranker", "index_token_filter", "query_token_filter" };

//////////////////////////////////////////////////////////////////////////

static bool								g_bPluginsEnabled = false;	///< is there any plugin support all?
static bool								g_bPluginsLocked = false;	///< do we allow CREATE/DROP at this point?
static CSphString						g_sPluginDir;
static CSphStaticMutex					g_tPluginMutex;				///< common plugin mutex (access to lib, func and ranker hashes)
static SmallStringHash_T<PluginLib_t>	g_hPluginLibs;				///< key is the filename (no path)

static CSphOrderedHash<PluginDesc_c*, PluginKey_t, PluginKey_t, 256>	g_hPlugins;

//////////////////////////////////////////////////////////////////////////

void PluginDesc_c::Release() const
{
	g_tPluginMutex.Lock ();
	m_iUserCount--;
	assert ( m_iUserCount>=0 );
	g_tPluginMutex.Unlock ();
}

//////////////////////////////////////////////////////////////////////////
// PLUGIN MANAGER
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#define HAVE_DLOPEN		1
#define RTLD_LAZY		0
#define RTLD_LOCAL		0

void * dlsym ( void * lib, const char * name )
{
	return GetProcAddress ( (HMODULE)lib, name );
}

void * dlopen ( const char * libname, int )
{
	return LoadLibraryEx ( libname, NULL, 0 );
}

int dlclose ( void * lib )
{
	return FreeLibrary ( (HMODULE)lib )
		? 0
		: GetLastError();
}

const char * dlerror()
{
	static char sError[256];
	DWORD uError = GetLastError();
	FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		uError, LANG_SYSTEM_DEFAULT, (LPTSTR)sError, sizeof(sError), NULL );
	return sError;
}
#endif // USE_WINDOWS

//////////////////////////////////////////////////////////////////////////

void sphPluginInit ( const char * sDir )
{
	if ( !sDir || !*sDir )
		return;

	g_sPluginDir = sDir;
	g_bPluginsEnabled = true;
	g_bPluginsLocked = false;
}


void sphPluginLock ( bool bLocked )
{
	g_bPluginsLocked = bLocked;
}


bool sphPluginParseSpec ( const CSphString & sParams, CSphVector<CSphString> & dParams, CSphString & sError )
{
	dParams.Resize ( 0 );
	sphSplit ( dParams, sParams.cstr(), ":" );

	switch ( dParams.GetLength() )
	{
	case 0:
		return true;

	case 1:
		sError = "filter name required in spec string; example: \"plugins.so:myfilter\"";
		return false;

	case 2:
		dParams.Add ( "" );
		return true;

	case 3:
		return true;
	}

	sError = "too many parts in spec string; must be in \"plugins.so:myfilter:options\" format";
	return false;
}

struct SymbolDesc_t
{
	int				m_iOffsetOf;	///< pointer member location in the descriptor structure
	const char *	m_sPostfix;		///< symbol name postfix
	bool			m_bRequired;	///< whether this symbol must be present
};


static bool PluginLoadSymbols ( void * pDesc, const SymbolDesc_t * pSymbol, void * pHandle, const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	CSphString s;
	while ( pSymbol->m_iOffsetOf >= 0 )
	{
		s.SetSprintf ( pSymbol->m_sPostfix[0] ? "%s_%s" : "%s%s", sName, pSymbol->m_sPostfix );
		void ** ppFunc = (void**)((BYTE*)pDesc + pSymbol->m_iOffsetOf);
		*ppFunc = dlsym ( pHandle, s.cstr() );
		if ( !*ppFunc && pSymbol->m_bRequired )
		{
			sError.SetSprintf ( "symbol %s() not found", s.cstr() );
			return false;
		}
		pSymbol++;
	}
	return true;
#endif // HAVE_DLOPEN
}

#if !USE_WINDOWS
#define offsetof(T, M) \
	(reinterpret_cast<char*>(&(((T*)1000)->M)) - reinterpret_cast<char*>(1000))
#endif

static SymbolDesc_t g_dSymbolsUDF[] =
{
	{ offsetof(PluginUDF_c, m_fnInit),		"init",		false },
	{ offsetof(PluginUDF_c, m_fnFunc),		"",			true },
	{ offsetof(PluginUDF_c, m_fnDeinit),	"deinit",	false },
	{ -1, 0, 0 }
};


static SymbolDesc_t g_dSymbolsRanker[] =
{
	{ offsetof(PluginRanker_c, m_fnInit),		"init",		false },
	{ offsetof(PluginRanker_c, m_fnUpdate),		"update",	false },
	{ offsetof(PluginRanker_c, m_fnFinalize),	"finalize",	true },
	{ offsetof(PluginRanker_c, m_fnDeinit),		"deinit",	false },
	{ -1, 0, 0 }
};


static SymbolDesc_t g_dSymbolsTokenFilter[] =
{
	{ offsetof(PluginTokenFilter_c, m_fnInit),			"init",				false },
	{ offsetof(PluginTokenFilter_c, m_fnBeginDocument),	"begin_document",	false },
	{ offsetof(PluginTokenFilter_c, m_fnBeginField),	"begin_field",		false },
	{ offsetof(PluginTokenFilter_c, m_fnPushToken),		"push_token",		true },
	{ offsetof(PluginTokenFilter_c, m_fnGetExtraToken),	"get_extra_token",	false },
	{ offsetof(PluginTokenFilter_c, m_fnEndField),		"end_field",		false },
	{ offsetof(PluginTokenFilter_c, m_fnDeinit),		"deinit",			false },
	{ -1, 0, 0 }
};


static SymbolDesc_t g_dSymbolsQueryTokenFilter[] =
{
	{ offsetof(PluginQueryTokenFilter_c, m_fnInit),			"init",			false },
	{ offsetof(PluginQueryTokenFilter_c, m_fnPreMorph),		"pre_morph",	false },
	{ offsetof(PluginQueryTokenFilter_c, m_fnPostMorph),	"post_morph",	false },
	{ offsetof(PluginQueryTokenFilter_c, m_fnDeinit),		"deinit",		false },
	{ -1, 0, 0 }
};


static bool PluginCreate ( const char * szLib, const char * szName,
	PluginType_e eType, PluginDesc_c * pPlugin, const SymbolDesc_t * pSymbols, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	if ( !g_bPluginsEnabled )
	{
		sError = "plugin support disabled (requires a valid plugin_dir)";
		return false;
	}

	if ( g_bPluginsLocked )
	{
		sError = "CREATE is disabled (fully dynamic plugins require workers=threads)";
		return false;
	}

	// validate library name
	for ( const char * p = szLib; *p; p++ )
		if ( *p=='/' || *p=='\\' )
		{
			sError = "restricted character (path delimiter) in a library file name";
			return false;
		}

	CSphString sLib = szLib;
	sLib.ToLower();
	
	// from here, we need a lock (we intend to update the plugin hash)
	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );

	// validate function name
	PluginKey_t k ( eType, szName );
	if ( g_hPlugins(k) )
	{
		sError.SetSprintf ( "plugin '%s' already exists", k.m_sName.cstr() );
		return false;
	}

	// lookup or load library
	CSphString sLibfile;
	sLibfile.SetSprintf ( "%s/%s", g_sPluginDir.cstr(), sLib.cstr() );

	bool bJustLoaded = false;
	void * pHandle = NULL;
	pPlugin->m_pLib = g_hPluginLibs ( sLib.cstr() );
	if ( !pPlugin->m_pLib )
	{
		bJustLoaded = true;
		pHandle = dlopen ( sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL );
		if ( !pHandle )
		{
			const char * sDlerror = dlerror();
			sError.SetSprintf ( "dlopen() failed: %s", sDlerror ? sDlerror : "(null)" );
			return false;
		}
		sphLogDebug ( "dlopen(%s)=%p", sLibfile.cstr(), pHandle );

	} else
		pHandle = pPlugin->m_pLib->m_pHandle;

	assert ( pHandle );

	if ( !PluginLoadSymbols ( pPlugin, pSymbols, pHandle, k.m_sName.cstr(), sError ) )
	{
		if ( bJustLoaded )
			dlclose ( pHandle );

		sError.SetSprintf ( "%s in %s", sError.cstr(), sLib.cstr() );
		return false;
	}

	// add library
	if ( bJustLoaded )
	{
		CSphString sBasename = sLib.cstr();
		const char * pDot = strchr ( sBasename.cstr(), '.' );
		if ( pDot )
			sBasename = sBasename.SubString ( 0, pDot-sBasename.cstr() );

		CSphString sTmp;
		PluginVer_fn fnVer = (PluginVer_fn) dlsym ( pHandle, sTmp.SetSprintf ( "%s_ver", sBasename.cstr() ).cstr() );
		if ( !fnVer )
		{
			sError.SetSprintf ( "symbol '%s_ver' not found in '%s': update your UDF implementation", sBasename.cstr(), sLib.cstr() );
			dlclose ( pHandle );
			return false;
		}

		if ( fnVer() < SPH_UDF_VERSION )
		{
			sError.SetSprintf ( "library '%s' was compiled using an older version of sphinxudf.h; it needs to be recompiled", sLib.cstr() );
			dlclose ( pHandle );
			return false;
		}

		PluginLib_t tLib;
		memset ( tLib.m_dCount, 0, sizeof(tLib.m_dCount) );
		tLib.m_dCount[eType] = 1;
		tLib.m_pHandle = pHandle;
		tLib.m_fnReinit = (PluginReinit_fn) dlsym ( pHandle, sTmp.SetSprintf ( "%s_reinit", sBasename.cstr() ).cstr() );
		Verify ( g_hPluginLibs.Add ( tLib, sLib.cstr() ) );
		pPlugin->m_pLib = g_hPluginLibs ( sLib.cstr() );
	} else
		pPlugin->m_pLib->m_dCount[eType]++;

	pPlugin->m_pLibName = g_hPluginLibs.GetKeyPtr ( sLib );
	assert ( pPlugin->m_pLib );

	// add function
	Verify ( g_hPlugins.Add ( pPlugin, k ) );
	return true;
#endif // HAVE_DLOPEN
}


bool sphPluginCreate ( const char * sLib, PluginType_e eType, const char * sName, ESphAttr eUDFRetType, CSphString & sError )
{
	// FIXME? preregister known rankers instead?
	if ( eType==PLUGIN_RANKER )
	{
		for ( int i=0; i<SPH_RANK_TOTAL; i++ )
		{
			const char * r = sphGetRankerName ( ESphRankMode(i) );
			if ( r && strcasecmp ( sName, r )==0 )
			{
				sError.SetSprintf ( "%s is a reserved ranker name", r );
				return false;
			}
		}
	}

	PluginDesc_c * pDesc = NULL;
	const SymbolDesc_t * pSym = NULL;
	switch ( eType )
	{
		case PLUGIN_RANKER:					pDesc = new PluginRanker_c(); pSym = g_dSymbolsRanker; break;
		case PLUGIN_INDEX_TOKEN_FILTER:		pDesc = new PluginTokenFilter_c(); pSym = g_dSymbolsTokenFilter; break;
		case PLUGIN_QUERY_TOKEN_FILTER:		pDesc = new PluginQueryTokenFilter_c(); pSym = g_dSymbolsQueryTokenFilter; break;
		case PLUGIN_FUNCTION:				pDesc = new PluginUDF_c ( eUDFRetType ); pSym = g_dSymbolsUDF; break;
		default:
			sError.SetSprintf ( "INTERNAL ERROR: unknown plugin type %d in CreatePlugin()", (int)eType );
			return false;
	}
	return PluginCreate ( sLib, sName, eType, pDesc, pSym, sError );
}


bool sphPluginDrop ( PluginType_e eType, const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	if ( g_bPluginsLocked )
	{
		sError = "DROP is disabled (fully dynamic plugins require workers=threads)";
		return false;
	}

	g_tPluginMutex.Lock();

	PluginKey_t tKey ( eType, sName );
	PluginDesc_c ** ppPlugin = g_hPlugins(tKey);
	if ( !ppPlugin || !*ppPlugin || (**ppPlugin).m_bToDrop ) // handle concurrent drop in progress as "not exists"
	{
		sError.SetSprintf ( "plugin '%s' does not exist", sName );
		g_tPluginMutex.Unlock();
		return false;
	}

	static const int UDF_DROP_TIMEOUT_SEC = 30; // in seconds
	int64_t tmEnd = sphMicroTimer() + UDF_DROP_TIMEOUT_SEC*1000000;

	// mark for deletion, to prevent new users
	PluginDesc_c * pPlugin = *ppPlugin;
	pPlugin->m_bToDrop = true;
	if ( pPlugin->m_iUserCount )
		for ( ;; )
		{
			// release lock and wait
			// so that concurrent users could complete and release the plugin
			g_tPluginMutex.Unlock();
			sphSleepMsec ( 50 );

			// re-acquire lock
			g_tPluginMutex.Lock();

			// everyone out? proceed with dropping
			assert ( pPlugin->m_iUserCount>=0 );
			if ( pPlugin->m_iUserCount<=0 )
				break;

			// timed out? clear deletion flag, and bail
			if ( sphMicroTimer() > tmEnd )
			{
				pPlugin->m_bToDrop = false;
				g_tPluginMutex.Unlock();

				sError.SetSprintf ( "DROP timed out in (still got %d users after waiting for %d seconds); please retry",
					pPlugin->m_iUserCount, UDF_DROP_TIMEOUT_SEC );
				return false;
			}
		}

	PluginLib_t * pLib = pPlugin->m_pLib;
	const CSphString * pLibName = pPlugin->m_pLibName;

	Verify ( g_hPlugins.Delete(tKey) );
	pLib->m_dCount[eType]--;

	bool bCanDrop = true;
	for ( int i=0; i<PLUGIN_TOTAL && bCanDrop; i++ )
		if ( pLib->m_dCount[i]>0 )
			bCanDrop = false;

	if ( bCanDrop )
	{
		// FIXME! running queries might be using this function/ranker
		int iRes = dlclose ( pLib->m_pHandle );
		sphLogDebug ( "dlclose(%s)=%d", pLibName->cstr(), iRes );
		Verify ( g_hPluginLibs.Delete ( *pLibName ) );
	}

	g_tPluginMutex.Unlock();
	return true;
#endif // HAVE_DLOPEN
}


PluginDesc_c * sphPluginAcquire ( const char * szLib, PluginType_e eType, const char * szName, CSphString & sError )
{
	PluginDesc_c * pDesc = sphPluginGet ( eType, szName );
	if ( !pDesc )
	{
		if ( !sphPluginCreate ( szLib, eType, szName, SPH_ATTR_NONE, sError ) )
			return false;
		return sphPluginGet ( eType, szName );
	}

	CSphString sLib ( szLib );
	sLib.ToLower();
	if ( *(pDesc->m_pLibName) == sLib )
		return pDesc;

	sError.SetSprintf ( "unable to load plugin '%s' from '%s': it has already been loaded from library '%s'",
		szName, sLib.cstr(), pDesc->m_pLibName->cstr() );
	pDesc->Release();
	return NULL;
}


static const char * UdfReturnType ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_INTEGER:		return "INT";
		case SPH_ATTR_FLOAT:		return "FLOAT";
		case SPH_ATTR_STRINGPTR:	return "STRING";
		default:					assert ( 0 && "unknown UDF return type" ); return "???";
	}
}


void sphPluginSaveState ( CSphWriter & tWriter )
{
	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );
	g_hPlugins.IterateStart();
	while ( g_hPlugins.IterateNext() )
	{
		const PluginKey_t & k = g_hPlugins.IterateGetKey();
		const PluginDesc_c * v = g_hPlugins.IterateGet();
		if ( v->m_bToDrop )
			continue;

		CSphString sBuf;
		if ( k.m_eType==PLUGIN_FUNCTION )
			sBuf.SetSprintf ( "CREATE FUNCTION %s RETURNS %s SONAME '%s';\n", k.m_sName.cstr(),
				UdfReturnType ( ((PluginUDF_c*)v)->m_eRetType ), v->m_pLibName->cstr() );
		else
			sBuf.SetSprintf ( "CREATE PLUGIN %s TYPE '%s' SONAME '%s';\n",
				k.m_sName.cstr(), g_dPluginTypes[k.m_eType], v->m_pLibName->cstr() );

		tWriter.PutBytes ( sBuf.cstr(), sBuf.Length() );
	}
}


void sphPluginReinit()
{
	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );
	g_hPluginLibs.IterateStart();
	while ( g_hPluginLibs.IterateNext() )
	{
		const PluginLib_t & tLib = g_hPluginLibs.IterateGet();
		if ( tLib.m_fnReinit )
			tLib.m_fnReinit();
	}
}


PluginType_e sphPluginGetType ( const CSphString & s )
{
	if ( s=="ranker" )				return PLUGIN_RANKER;
	if ( s=="index_token_filter" )	return PLUGIN_INDEX_TOKEN_FILTER;
	if ( s=="query_token_filter" )	return PLUGIN_QUERY_TOKEN_FILTER;
	return PLUGIN_TOTAL;
}


bool sphPluginExists ( PluginType_e eType, const char * sName )
{
	if ( !g_bPluginsEnabled )
		return false;
	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDesc_c ** pp = g_hPlugins(k);
	return pp && *pp && !(**pp).m_bToDrop;
}


PluginDesc_c * sphPluginGet ( PluginType_e eType, const char * sName )
{
	if ( !g_bPluginsEnabled )
		return NULL;

	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDesc_c ** pp = g_hPlugins(k);
	if ( !pp || !*pp || (**pp).m_bToDrop )
		return NULL; // either not found, or DROP in progress, can not use
	(**pp).m_iUserCount++; // protection against concurrent DROP, gets decremented in PluginDesc_c::Release()
	return *pp;
}


void sphPluginList ( CSphVector<PluginInfo_t> & dResult )
{
	if ( !g_bPluginsEnabled )
		return;
	CSphScopedLock<CSphStaticMutex> tLock ( g_tPluginMutex );
	g_hPlugins.IterateStart();
	while ( g_hPlugins.IterateNext() )
	{
		const PluginKey_t & k = g_hPlugins.IterateGetKey();
		const PluginDesc_c *v = g_hPlugins.IterateGet();

		PluginInfo_t & p = dResult.Add();
		p.m_eType = k.m_eType;
		p.m_sName = k.m_sName;
		p.m_sLib = v->m_pLibName->cstr();
		p.m_iUsers = v->m_iUserCount;
		if ( p.m_eType==PLUGIN_FUNCTION )
			p.m_sExtra = UdfReturnType ( ((PluginUDF_c*)v)->m_eRetType );
	}
}

//
// $Id$
//
