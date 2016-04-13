//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
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
class PluginLib_c : public ISphRefcountedMT
{
protected:
	CSphString			m_sName;
	void *				m_pHandle;				///< handle from dlopen()

public:
	int					m_iHashedPlugins;		///< how many active g_hPlugins entries reference this handle

	explicit			PluginLib_c ( void * pHandle, const char * sName );
	const CSphString &	GetName() const { return m_sName; }
	void *				GetHandle() const { return m_pHandle; }

protected:
						~PluginLib_c();
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
		return sphCRC32 ( v.m_sName.cstr(), v.m_sName.Length(),
			sphCRC32 ( &v.m_eType, sizeof(v.m_eType) ) );
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

static bool								g_bPluginsEnabled = false;	///< is there any plugin support at all?
static CSphString						g_sPluginDir;
static CSphMutex						g_tPluginMutex;				///< common plugin mutex (access to lib, func and ranker hashes)
static SmallStringHash_T<PluginLib_c*>	g_hPluginLibs;				///< key is the filename (no path)

static CSphOrderedHash<PluginDesc_c*, PluginKey_t, PluginKey_t, 256>	g_hPlugins;


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

PluginLib_c::PluginLib_c ( void * pHandle, const char * sName )
{
	assert ( pHandle );
	m_pHandle = pHandle;
	m_iHashedPlugins = 0;
	m_sName = sName;
	m_sName.ToLower();
}

PluginLib_c::~PluginLib_c()
{
	int iRes = dlclose ( m_pHandle );
	sphLogDebug ( "dlclose(%s)=%d", m_sName.cstr(), iRes );
}

PluginDesc_c::PluginDesc_c ( PluginLib_c * pLib )
{
	assert ( pLib );
	m_pLib = pLib;
	m_pLib->AddRef();
}

PluginDesc_c::~PluginDesc_c()
{
	m_pLib->Release();
}

const CSphString & PluginDesc_c::GetLibName() const
{
	assert ( m_pLib );
	return m_pLib->GetName();
}

//////////////////////////////////////////////////////////////////////////

void sphPluginInit ( const char * sDir )
{
	if ( !sDir || !*sDir )
		return;

	g_sPluginDir = sDir;
	g_bPluginsEnabled = true;
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
	while ( pSymbol->m_iOffsetOf>=0 )
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


static PluginLib_c * LoadPluginLibrary ( const char * sLibName, CSphString & sError, bool bLinuxReload=false )
{
	CSphString sTmpfile;
	CSphString sLibfile;
	sLibfile.SetSprintf ( "%s/%s", g_sPluginDir.cstr(), sLibName );

	// dlopen caches the old file content, even if file was updated
	// let's reload library from the temporary file to invalidate the cache
	if ( bLinuxReload )
	{
		sTmpfile.SetSprintf ( "%s/%s.%u", g_sPluginDir.cstr(), sLibName, sphRand() );
		if ( ::rename ( sLibfile.cstr(), sTmpfile.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename file (src=%s, dst=%s, errno=%d, error=%s)", sLibfile.cstr(), sTmpfile.cstr(), errno, strerror(errno) );
			return NULL;
		}
	}

	void * pHandle = dlopen ( bLinuxReload ? sTmpfile.cstr() : sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL );
	if ( !pHandle )
	{
		const char * sDlerror = dlerror();
		sError.SetSprintf ( "dlopen() failed: %s", sDlerror ? sDlerror : "(null)" );
		return NULL;
	}
	sphLogDebug ( "dlopen(%s)=%p", bLinuxReload ? sTmpfile.cstr() : sLibfile.cstr(), pHandle );

	// rename file back to the original name
	if ( bLinuxReload )
	{
		if ( ::rename ( sTmpfile.cstr(), sLibfile.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename file (src=%s, dst=%s, errno=%d, error=%s)", sTmpfile.cstr(), sLibfile.cstr(), errno, strerror(errno) );
			return NULL;
		}
	}

	CSphString sBasename = sLibName;
	const char * pDot = strchr ( sBasename.cstr(), '.' );
	if ( pDot )
		sBasename = sBasename.SubString ( 0, pDot-sBasename.cstr() );

	CSphString sTmp;
	PluginVer_fn fnVer = (PluginVer_fn) dlsym ( pHandle, sTmp.SetSprintf ( "%s_ver", sBasename.cstr() ).cstr() );
	if ( !fnVer )
	{
		sError.SetSprintf ( "symbol '%s_ver' not found in '%s': update your UDF implementation", sBasename.cstr(), sLibName );
		dlclose ( pHandle );
		return NULL;
	}

	if ( fnVer() < SPH_UDF_VERSION )
	{
		sError.SetSprintf ( "library '%s' was compiled using an older version of sphinxudf.h; it needs to be recompiled", sLibName );
		dlclose ( pHandle );
		return NULL;
	}

	return new PluginLib_c ( pHandle, sLibName );
}


bool sphPluginCreate ( const char * szLib, PluginType_e eType, const char * sName, ESphAttr eUDFRetType, CSphString & sError )
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

	// validate library name
	for ( const char * p = szLib; *p; p++ )
		if ( *p=='/' || *p=='\\' )
	{
		sError = "restricted character (path delimiter) in a library file name";
		return false;
	}

	CSphString sLib = szLib;
	sLib.ToLower();

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

	// from here, we need a lock (we intend to update the plugin hash)
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );

	// validate function name
	PluginKey_t k ( eType, sName );
	if ( g_hPlugins(k) )
	{
		sError.SetSprintf ( "plugin '%s' already exists", k.m_sName.cstr() );
		return false;
	}

	// lookup or load library
	PluginLib_c * pLib = NULL;
	if ( g_hPluginLibs ( sLib ) )
	{
		pLib = g_hPluginLibs [ sLib ];
		pLib->AddRef();
	} else
	{
		pLib = LoadPluginLibrary ( sLib.cstr(), sError );
		if ( !pLib )
			return false;
	}
	assert ( pLib->GetHandle() );

	PluginDesc_c * pPlugin = NULL;
	const SymbolDesc_t * pSym = NULL;
	switch ( eType )
	{
		case PLUGIN_RANKER:					pPlugin = new PluginRanker_c ( pLib ); pSym = g_dSymbolsRanker; break;
		case PLUGIN_INDEX_TOKEN_FILTER:		pPlugin = new PluginTokenFilter_c ( pLib ); pSym = g_dSymbolsTokenFilter; break;
		case PLUGIN_QUERY_TOKEN_FILTER:		pPlugin = new PluginQueryTokenFilter_c ( pLib ); pSym = g_dSymbolsQueryTokenFilter; break;
		case PLUGIN_FUNCTION:				pPlugin = new PluginUDF_c ( pLib, eUDFRetType ); pSym = g_dSymbolsUDF; break;
		default:
			sError.SetSprintf ( "INTERNAL ERROR: unknown plugin type %d in CreatePlugin()", (int)eType );
			pLib->Release();
			return false;
	}

	// release the refcount that this very function is holding
	// or in other words, transfer the refcount to newly created plugin instance (it does its own addref)
	pLib->Release();

	if ( !PluginLoadSymbols ( pPlugin, pSym, pLib->GetHandle(), k.m_sName.cstr(), sError ) )
	{
		sError.SetSprintf ( "%s in %s", sError.cstr(), sLib.cstr() );
		pPlugin->Release();
		return false;
	}

	// add library if needed
	if ( !g_hPluginLibs ( sLib ) )
	{
		Verify ( g_hPluginLibs.Add ( pLib, pLib->GetName() ) );
		pLib->AddRef(); // the hash reference
	}

	// add function
	Verify ( g_hPlugins.Add ( pPlugin, k ) );
	pPlugin->GetLib()->m_iHashedPlugins++;
	return true;
#endif // HAVE_DLOPEN
}


bool sphPluginDrop ( PluginType_e eType, const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );

	PluginKey_t tKey ( eType, sName );
	PluginDesc_c ** ppPlugin = g_hPlugins(tKey);
	if ( !ppPlugin || !*ppPlugin )
	{
		sError.SetSprintf ( "plugin '%s' does not exist", sName );
		return false;
	}

	PluginDesc_c * pPlugin = *ppPlugin;
	PluginLib_c * pLib = pPlugin->GetLib();

	Verify ( g_hPlugins.Delete(tKey) );
	pPlugin->Release();

	if ( --pLib->m_iHashedPlugins==0 )
	{
		g_hPluginLibs.Delete ( pLib->GetName() );
		pLib->Release();
	}

	return true;
#endif // HAVE_DLOPEN
}


bool sphPluginReload ( const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	// find all plugins from the given library
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );

	CSphVector<PluginKey_t> dKeys;
	CSphVector<PluginDesc_c*> dPlugins;

	g_hPlugins.IterateStart();
	while ( g_hPlugins.IterateNext() )
	{
		PluginDesc_c * v = g_hPlugins.IterateGet();
		if ( v->GetLibName()==sName )
		{
			dKeys.Add ( g_hPlugins.IterateGetKey() );
			dPlugins.Add ( g_hPlugins.IterateGet() );
		}
	}

	// no plugins loaded? oops
	if ( dPlugins.GetLength()==0 )
	{
		sError.SetSprintf ( "no active plugins loaded from %s", sName );
		return false;
	}

	// load new library and check every plugin
#if !USE_WINDOWS
	PluginLib_c * pNewLib = LoadPluginLibrary ( sName, sError, true );
#else
	PluginLib_c * pNewLib = LoadPluginLibrary ( sName, sError );
#endif
	if ( !pNewLib )
		return false;

	// load all plugins
	CSphVector<PluginDesc_c*> dNewPlugins;
	ARRAY_FOREACH ( i, dPlugins )
	{
		PluginDesc_c * pDesc = NULL;
		const SymbolDesc_t * pSym = NULL;
		switch ( dKeys[i].m_eType )
		{
			case PLUGIN_RANKER:					pDesc = new PluginRanker_c ( pNewLib ); pSym = g_dSymbolsRanker; break;
			case PLUGIN_INDEX_TOKEN_FILTER:		pDesc = new PluginTokenFilter_c ( pNewLib ); pSym = g_dSymbolsTokenFilter; break;
			case PLUGIN_QUERY_TOKEN_FILTER:		pDesc = new PluginQueryTokenFilter_c ( pNewLib ); pSym = g_dSymbolsQueryTokenFilter; break;
			case PLUGIN_FUNCTION:				pDesc = new PluginUDF_c ( pNewLib, dPlugins[i]->GetUdfRetType() ); pSym = g_dSymbolsUDF; break;
			default:
				sphDie ( "INTERNAL ERROR: unknown plugin type %d in sphPluginReload()", (int)dKeys[i].m_eType );
				return false;
		}

		if ( !PluginLoadSymbols ( pDesc, pSym, pNewLib->GetHandle(), dKeys[i].m_sName.cstr(), sError ) )
		{
			pDesc->Release();
			break;
		}

		dNewPlugins.Add ( pDesc );
	}

	// we can now release the reference that this function was holding
	pNewLib->Release();

	// if there was a problem loading any of the plugins, time to fail
	if ( dPlugins.GetLength()!=dNewPlugins.GetLength() )
	{
		ARRAY_FOREACH ( i, dNewPlugins )
			dNewPlugins[i]->Release();

		sError.SetSprintf ( "failed to import plugin %s: %s", dKeys [ dNewPlugins.GetLength() ].m_sName.cstr(), sError.cstr() );
		return false;
	}

	// unregister and release the old references
	PluginLib_c * pOldLib = dPlugins[0]->GetLib();
	ARRAY_FOREACH ( i, dPlugins )
	{
		assert ( dPlugins[i]->GetLib()==pOldLib );
		Verify ( g_hPlugins.Delete ( dKeys[i] ) );
		SafeRelease ( dPlugins[i] );
	}
	assert ( pOldLib->m_iHashedPlugins==dPlugins.GetLength() );
	pOldLib->m_iHashedPlugins = 0;
	Verify ( g_hPluginLibs.Delete ( pOldLib->GetName() ) );
	SafeRelease ( pOldLib );

	// register new references
	g_hPluginLibs.Add ( pNewLib, pNewLib->GetName() );
	pNewLib->AddRef(); // the hash reference

	ARRAY_FOREACH ( i, dNewPlugins )
		Verify ( g_hPlugins.Add ( dNewPlugins[i], dKeys[i] ) );
	assert ( pNewLib->m_iHashedPlugins==0 );
	pNewLib->m_iHashedPlugins = dNewPlugins.GetLength();

	sphLogDebug ( "reloaded %d plugins", dNewPlugins.GetLength() );

	return true;
#endif // HAVE_DLOPEN
}


PluginDesc_c * sphPluginAcquire ( const char * szLib, PluginType_e eType, const char * szName, CSphString & sError )
{
	PluginDesc_c * pDesc = sphPluginGet ( eType, szName );
	if ( !pDesc )
	{
		if ( !sphPluginCreate ( szLib, eType, szName, SPH_ATTR_NONE, sError ) )
			return NULL;
		return sphPluginGet ( eType, szName );
	}

	CSphString sLib ( szLib );
	sLib.ToLower();
	if ( pDesc->GetLibName()==sLib )
		return pDesc;

	sError.SetSprintf ( "unable to load plugin '%s' from '%s': it has already been loaded from library '%s'",
		szName, sLib.cstr(), pDesc->GetLibName().cstr() );
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
		case SPH_ATTR_BIGINT:		return "BIGINT";
		default:					assert ( 0 && "unknown UDF return type" ); return "???";
	}
}


void sphPluginSaveState ( CSphWriter & tWriter )
{
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );
	g_hPlugins.IterateStart();
	while ( g_hPlugins.IterateNext() )
	{
		const PluginKey_t & k = g_hPlugins.IterateGetKey();
		const PluginDesc_c * v = g_hPlugins.IterateGet();

		CSphString sBuf;
		if ( k.m_eType==PLUGIN_FUNCTION )
			sBuf.SetSprintf ( "CREATE FUNCTION %s RETURNS %s SONAME '%s';\n", k.m_sName.cstr(),
				UdfReturnType ( ((PluginUDF_c*)v)->m_eRetType ), v->GetLibName().cstr() );
		else
			sBuf.SetSprintf ( "CREATE PLUGIN %s TYPE '%s' SONAME '%s';\n",
				k.m_sName.cstr(), g_dPluginTypes[k.m_eType], v->GetLibName().cstr() );

		tWriter.PutBytes ( sBuf.cstr(), sBuf.Length() );
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
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDesc_c ** pp = g_hPlugins(k);
	return pp && *pp;
}


PluginDesc_c * sphPluginGet ( PluginType_e eType, const char * sName )
{
	if ( !g_bPluginsEnabled )
		return NULL;

	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDesc_c ** pp = g_hPlugins(k);
	if ( !pp || !*pp )
		return NULL;
	(**pp).AddRef();
	return *pp;
}


void sphPluginList ( CSphVector<PluginInfo_t> & dResult )
{
	if ( !g_bPluginsEnabled )
		return;
	CSphScopedLock<CSphMutex> tLock ( g_tPluginMutex );
	g_hPlugins.IterateStart();
	while ( g_hPlugins.IterateNext() )
	{
		const PluginKey_t & k = g_hPlugins.IterateGetKey();
		const PluginDesc_c *v = g_hPlugins.IterateGet();

		PluginInfo_t & p = dResult.Add();
		p.m_eType = k.m_eType;
		p.m_sName = k.m_sName;
		p.m_sLib = v->GetLibName().cstr();
		p.m_iUsers = v->GetRefcount() - 1; // except the one reference from the hash itself
		if ( p.m_eType==PLUGIN_FUNCTION )
			p.m_sExtra = UdfReturnType ( ((PluginUDF_c*)v)->m_eRetType );
	}
}

//
// $Id$
//
