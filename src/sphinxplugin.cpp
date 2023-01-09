//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxint.h"
#include "sphinxutils.h"
#include "sphinxplugin.h"
#include "libutils.h"

//////////////////////////////////////////////////////////////////////////
// TYPES
//////////////////////////////////////////////////////////////////////////

/// loaded plugin library
class PluginLib_c final : public ISphRefcountedMT
{
protected:
	CSphString			m_sName;
	void *				m_pHandle;				///< handle from dlopen()

public:
	int					m_iHashedPlugins;		///< how many active g_hPlugins entries reference this handle
	bool				m_bDlGlobal = false;

						PluginLib_c ( void * pHandle, const char * sName, bool bDlGlobal );
	const CSphString &	GetName() const { return m_sName; }
	void *				GetHandle() const { return m_pHandle; }

protected:
						~PluginLib_c() final;
};

PluginLibRefPtr_c PluginDesc_c::GetLib() const
{
	return m_pLib;
}


/// plugin key
struct PluginKey_t
{
	PluginType_e		m_eType { PLUGIN_FUNCTION };
	CSphString			m_sName;

	PluginKey_t() = default;
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

static bool								g_bPluginsEnabled = true;	///< is there any plugin support at all?
static CSphString						g_sPluginDir = HARDCODED_PLUGIN_DIR;
static CSphMutex						g_tPluginMutex;				///< common plugin mutex (access to lib, func and ranker hashes)
static SmallStringHash_T<PluginLibRefPtr_c>	g_hPluginLibs GUARDED_BY ( g_tPluginMutex );			///< key is the filename (no path)

static CSphOrderedHash<PluginDescRefPtr_c, PluginKey_t, PluginKey_t, 256>	g_hPlugins GUARDED_BY ( g_tPluginMutex );


//////////////////////////////////////////////////////////////////////////
// PLUGIN MANAGER
//////////////////////////////////////////////////////////////////////////

PluginLib_c::PluginLib_c ( void * pHandle, const char * sName, bool bDlGlobal )
{
	assert ( pHandle );
	m_pHandle = pHandle;
	m_iHashedPlugins = 0;
	m_sName = sName;
	m_sName.ToLower();
	m_bDlGlobal = bDlGlobal;
}

PluginLib_c::~PluginLib_c()
{
#if HAVE_DLOPEN
	int iRes = dlclose ( m_pHandle );
	sphLogDebug ( "dlclose(%s)=%d", m_sName.cstr(), iRes );
#endif
}

PluginDesc_c::PluginDesc_c ( PluginLibRefPtr_c pLib )
{
	assert ( pLib );
	m_pLib = std::move (pLib);
}

PluginDesc_c::~PluginDesc_c() {}

const CSphString & PluginDesc_c::GetLibName() const
{
	assert ( m_pLib );
	return m_pLib->GetName();
}

//////////////////////////////////////////////////////////////////////////

void sphPluginInit ( const char * sDir )
{
	if ( !sDir || !*sDir )
	{
		g_bPluginsEnabled = false;
		return;
	}

	g_sPluginDir = sDir;
	g_bPluginsEnabled = true;
}


bool sphPluginParseSpec ( const CSphString & sParams, StrVec_t & dParams, CSphString & sError )
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

#if HAVE_DLOPEN
static bool PluginLoadSymbols ( void * pDesc, const SymbolDesc_t * pSymbol, void * pHandle, const char * sName, CSphString & sError )
{
//	sError = "no dlopen(), no plugins";
//	return false;
	CSphString s;
	while ( pSymbol->m_iOffsetOf>=0 )
	{
		s.SetSprintf ( pSymbol->m_sPostfix[0] ? "%s_%s" : "%s%s", sName, pSymbol->m_sPostfix );
		auto ** ppFunc = (void**)((BYTE*)pDesc + pSymbol->m_iOffsetOf);
		*ppFunc = dlsym ( pHandle, s.cstr() );
		if ( !*ppFunc && pSymbol->m_bRequired )
		{
			sError.SetSprintf ( "symbol %s() not found", s.cstr() );
			return false;
		}
		pSymbol++;
	}
	return true;
}
#endif // HAVE_DLOPEN

#if !_WIN32
#ifndef offsetof

#define offsetof(T, M) \
	(reinterpret_cast<char*>(&(((T*)1000)->M)) - reinterpret_cast<char*>(1000))

#endif
#endif

#if HAVE_DLOPEN

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

static SymbolDesc_t g_dSymbolsUDF[] =
{
	{ static_cast<int>( offsetof(PluginUDF_c, m_fnInit)),		"init",		false },
	{ static_cast<int>( offsetof(PluginUDF_c, m_fnFunc)),		"",			true },
	{ static_cast<int>( offsetof(PluginUDF_c, m_fnDeinit)),	"deinit",	false },
	{ -1, nullptr, false }
};


static SymbolDesc_t g_dSymbolsRanker[] =
{
	{ static_cast<int>( offsetof(PluginRanker_c, m_fnInit)),		"init",		false },
	{ static_cast<int>( offsetof(PluginRanker_c, m_fnUpdate)),		"update",	false },
	{ static_cast<int>( offsetof(PluginRanker_c, m_fnFinalize)),	"finalize",	true },
	{ static_cast<int>( offsetof(PluginRanker_c, m_fnDeinit)),		"deinit",	false },
	{ -1, nullptr, false }
};


static SymbolDesc_t g_dSymbolsTokenFilter[] =
{
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnInit)),			"init",				false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnBeginDocument)),	"begin_document",	false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnBeginField)),	"begin_field",		false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnPushToken)),		"push_token",		true },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnGetExtraToken)),	"get_extra_token",	false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnEndField)),		"end_field",		false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnDeinit)),		"deinit",			false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnTokenIsBlended)),		"is_blended", false },
	{ static_cast<int>( offsetof(PluginTokenFilter_c, m_fnTokenIsBlendedPart)),	"is_blended_part", false },
	{ -1, nullptr, false }
};


static SymbolDesc_t g_dSymbolsQueryTokenFilter[] =
{
	{ static_cast<int>( offsetof(PluginQueryTokenFilter_c, m_fnInit)),			"init",			false },
	{ static_cast<int>( offsetof(PluginQueryTokenFilter_c, m_fnPreMorph)),		"pre_morph",	false },
	{ static_cast<int>( offsetof(PluginQueryTokenFilter_c, m_fnPostMorph)),	"post_morph",	false },
	{ static_cast<int>( offsetof(PluginQueryTokenFilter_c, m_fnPushToken)),	"push_token",	false },
	{ static_cast<int>( offsetof(PluginQueryTokenFilter_c, m_fnDeinit)),		"deinit",		false },
	{ -1, nullptr, false }
};

void PluginLog ( const char * szMsg, int iLen )
{
	if ( iLen<0 )
		sphWarning ( "PLUGIN: %s", szMsg );
	else
		sphWarning ( "PLUGIN: %.*s", (int) iLen, szMsg );
}

static bool PluginOnLoadLibrary ( const PluginLibRefPtr_c& pLib, CSphString & sError ) REQUIRES ( g_tPluginMutex )
{
	// library already loaded - no need to call plugin_load function
	if ( g_hPluginLibs ( pLib->GetName() ) )
		return true;

	auto fnPluginLoad = (PluginLoad_fn) dlsym ( pLib->GetHandle(), "plugin_load" );
	if ( fnPluginLoad )
	{
		char sErrBuf [ SPH_UDF_ERROR_LEN ] = { 0 };
		if ( fnPluginLoad ( sErrBuf )!=0 )
		{
			sError = sErrBuf;
			return false;
		}
	}

	return true;
}

static bool PluginOnUnloadLibrary ( const PluginLibRefPtr_c& pLib, CSphString & sError )
{
	auto fnPluginUnload = (PluginLoad_fn) dlsym ( pLib->GetHandle(), "plugin_unload" );
	if ( fnPluginUnload )
	{
		char sErrBuf [ SPH_UDF_ERROR_LEN ] = { 0 };
		if ( fnPluginUnload ( sErrBuf )!=0 )
		{
			sError = sErrBuf;
			return false;
		}
	}

	return true;
}

static PluginLibRefPtr_c LoadPluginLibrary ( const char * sLibName, CSphString & sError, bool bDlGlobal, bool bLinuxReload )
{

	CSphString sTmpfile;
	CSphString sLibfile;
	sLibfile.SetSprintf ( "%s/%s", g_sPluginDir.cstr(), sLibName );

	// dlopen caches the old file content, even if file was updated
	// let's reload library from the temporary file to invalidate the cache
	if ( bLinuxReload )
	{
		sTmpfile.SetSprintf ( "%s/%s.%u", g_sPluginDir.cstr(), sLibName, sphRand() );
		if ( sph::rename ( sLibfile.cstr(), sTmpfile.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename file (src=%s, dst=%s, errno=%d, error=%s)", sLibfile.cstr(), sTmpfile.cstr(), errno, strerrorm(errno) );
			return nullptr;
		}
	}

	int iFlags = ( bDlGlobal ? ( RTLD_LAZY | RTLD_GLOBAL ) : ( RTLD_LAZY | RTLD_LOCAL ) );
	void * pHandle = dlopen ( bLinuxReload ? sTmpfile.cstr() : sLibfile.cstr(), iFlags );
	if ( !pHandle )
	{
		const char * sDlerror = dlerror();
		sError.SetSprintf ( "dlopen() failed: %s", sDlerror ? sDlerror : "(null)" );
		return nullptr;
	}
	sphLogDebug ( "dlopen(%s)=%p", bLinuxReload ? sTmpfile.cstr() : sLibfile.cstr(), pHandle );

	// rename file back to the original name
	if ( bLinuxReload )
	{
		if ( sph::rename ( sTmpfile.cstr(), sLibfile.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename file (src=%s, dst=%s, errno=%d, error=%s)", sTmpfile.cstr(), sLibfile.cstr(), errno, strerrorm(errno) );
			dlclose ( pHandle );
			return nullptr;
		}
	}

	CSphString sBasename = sLibName;
	const char * pDot = strchr ( sBasename.cstr(), '.' );
	if ( pDot )
		sBasename = sBasename.SubString ( 0, pDot-sBasename.cstr() );

	CSphString sTmp;
	auto fnVer = (PluginVer_fn) dlsym ( pHandle, sTmp.SetSprintf ( "%s_ver", sBasename.cstr() ).cstr() );
	if ( !fnVer )
	{
		sError.SetSprintf ( "symbol '%s_ver' not found in '%s': update your UDF implementation", sBasename.cstr(), sLibName );
		dlclose ( pHandle );
		return nullptr;
	}

	if ( fnVer() < SPH_UDF_VERSION )
	{
		sError.SetSprintf ( "library '%s' was compiled using an older version of sphinxudf.h; it needs to be recompiled", sLibName );
		dlclose ( pHandle );
		return nullptr;
	}

	auto fnLogCb = (PluginLogCb_fn) dlsym ( pHandle, sTmp.SetSprintf ( "%s_setlogcb", sBasename.cstr ()).cstr ());
	if ( fnLogCb ) {
		fnLogCb(PluginLog);
	}

	return PluginLibRefPtr_c { new PluginLib_c ( pHandle, sLibName, bDlGlobal ) };
}
#endif

bool sphPluginCreate ( const char * szLib, PluginType_e eType, const char * sName, ESphAttr eUDFRetType, CSphString & sError )
{
	return sphPluginCreate ( szLib, eType, sName, eUDFRetType, false, sError );
}

bool sphPluginCreate ( const char * szLib, PluginType_e eType, const char * sName, ESphAttr eUDFRetType, bool bDlGlobal, CSphString & sError )
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
	ScopedMutex_t tLock ( g_tPluginMutex );

	// validate function name
	PluginKey_t k ( eType, sName );
	if ( g_hPlugins(k) )
	{
		sError.SetSprintf ( "plugin '%s' already exists", k.m_sName.cstr() );
		return false;
	}

	// lookup or load library
	PluginLibRefPtr_c pLib;
	if ( g_hPluginLibs ( sLib ) )
		pLib = g_hPluginLibs [ sLib ];
	else
		pLib = LoadPluginLibrary ( sLib.cstr(), sError, bDlGlobal, false );

	if ( !pLib )
		return false;

	assert ( pLib->GetHandle() );
	if ( !PluginOnLoadLibrary ( pLib, sError ) )
		return false;

	PluginDescRefPtr_c pPlugin;
	const SymbolDesc_t * pSym = nullptr;
	switch ( eType )
	{
		case PLUGIN_RANKER:					pPlugin = new PluginRanker_c ( pLib ); pSym = g_dSymbolsRanker; break;
		case PLUGIN_INDEX_TOKEN_FILTER:		pPlugin = new PluginTokenFilter_c ( pLib ); pSym = g_dSymbolsTokenFilter; break;
		case PLUGIN_QUERY_TOKEN_FILTER:		pPlugin = new PluginQueryTokenFilter_c ( pLib ); pSym = g_dSymbolsQueryTokenFilter; break;
		case PLUGIN_FUNCTION:				pPlugin = new PluginUDF_c ( pLib, eUDFRetType ); pSym = g_dSymbolsUDF; break;
		default:
			sError.SetSprintf ( "INTERNAL ERROR: unknown plugin type %d in CreatePlugin()", (int)eType );
			return false;
	}

	if ( !PluginLoadSymbols ( pPlugin, pSym, pLib->GetHandle(), k.m_sName.cstr(), sError ) )
	{
		sError.SetSprintf ( "%s in %s", sError.cstr(), sLib.cstr() );
		return false;
	}

	// add library if needed
	if ( !g_hPluginLibs ( sLib ) )
		Verify ( g_hPluginLibs.Add ( pLib, pLib->GetName() ) );

	// add function
	Verify ( g_hPlugins.Add ( pPlugin, k ) );
	++pPlugin->GetLib()->m_iHashedPlugins;
	return true;
#endif // HAVE_DLOPEN
}


bool sphPluginDrop ( PluginType_e eType, const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	ScopedMutex_t tLock ( g_tPluginMutex );

	PluginKey_t tKey ( eType, sName );
	PluginDescRefPtr_c* ppPlugin = g_hPlugins(tKey);
	if ( !ppPlugin || !*ppPlugin )
	{
		sError.SetSprintf ( "plugin '%s' does not exist", sName );
		return false;
	}

	PluginLibRefPtr_c pLib = ( *ppPlugin )->GetLib();
	Verify ( g_hPlugins.Delete(tKey) );

	bool bUnloaded = true;
	if ( --pLib->m_iHashedPlugins==0 )
	{
		bUnloaded = PluginOnUnloadLibrary ( pLib, sError );
		g_hPluginLibs.Delete ( pLib->GetName() );
	}

	return bUnloaded;
#endif // HAVE_DLOPEN
}


bool sphPluginReload ( const char * sName, CSphString & sError )
{
#if !HAVE_DLOPEN
	sError = "no dlopen(), no plugins";
	return false;
#else
	// find all plugins from the given library
	ScopedMutex_t tLock ( g_tPluginMutex );

	CSphVector<PluginKey_t> dKeys;
	CSphVector<PluginDescRefPtr_c> dPlugins;

	bool bDlGlobal = false;
	for ( const auto& tPlugin : g_hPlugins )
	{
		PluginDescRefPtr_c v = tPlugin.second;
		if ( v->GetLibName()==sName )
		{
			dKeys.Add ( tPlugin.first );
			dPlugins.Add ( v );
			bDlGlobal = v->GetLib()->m_bDlGlobal;
		}
	}

	// no plugins loaded? oops
	if ( dPlugins.GetLength()==0 )
	{
		sError.SetSprintf ( "no active plugins loaded from %s", sName );
		return false;
	}

	// load new library and check every plugin
	PluginLibRefPtr_c pNewLib = LoadPluginLibrary ( sName, sError, bDlGlobal,
#if !_WIN32
		true
#else
		false
#endif
				);
	if ( !pNewLib )
		return false;

	// load all plugins
	CSphVector<PluginDescRefPtr_c> dNewPlugins;
	ARRAY_FOREACH ( i, dPlugins )
	{
		PluginDescRefPtr_c pDesc;
		const SymbolDesc_t * pSym = nullptr;
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
			break;

		dNewPlugins.Add ( pDesc );
	}

	// if there was a problem loading any of the plugins, time to fail
	if ( dPlugins.GetLength()!=dNewPlugins.GetLength() )
	{
		sError.SetSprintf ( "failed to import plugin %s: %s", dKeys [ dNewPlugins.GetLength() ].m_sName.cstr(), sError.cstr() );
		return false;
	}

	// unregister and release the old references
	PluginLibRefPtr_c pOldLib = dPlugins[0]->GetLib();
	ARRAY_FOREACH ( i, dPlugins )
	{
		assert ( dPlugins[i]->GetLib()==pOldLib );
		Verify ( g_hPlugins.Delete ( dKeys[i] ) );
	}
	assert ( pOldLib->m_iHashedPlugins==dPlugins.GetLength() );
	pOldLib->m_iHashedPlugins = 0;
	PluginOnUnloadLibrary ( pOldLib, sError );
	Verify ( g_hPluginLibs.Delete ( pOldLib->GetName() ) );

	if ( !PluginOnLoadLibrary ( pNewLib, sError ) )
		return false;

	// register new references
	g_hPluginLibs.Add ( pNewLib, pNewLib->GetName() );

	ARRAY_FOREACH ( i, dNewPlugins )
		Verify ( g_hPlugins.Add ( dNewPlugins[i], dKeys[i] ) );
	assert ( pNewLib->m_iHashedPlugins==0 );
	pNewLib->m_iHashedPlugins = dNewPlugins.GetLength();

	sphLogDebug ( "reloaded %d plugins", dNewPlugins.GetLength() );

	return true;
#endif // HAVE_DLOPEN
}


PluginDescRefPtr_c PluginAcquireDesc ( const char * szLib, PluginType_e eType, const char * szName, CSphString & sError )
{
	PluginDescRefPtr_c pDesc = PluginGetDesc ( eType, szName );
	if ( !pDesc )
	{
		if ( !sphPluginCreate ( szLib, eType, szName, SPH_ATTR_NONE, false, sError ) )
			return nullptr;
		return PluginGetDesc ( eType, szName );
	}

	CSphString sLib ( szLib );
	sLib.ToLower();
	if ( pDesc->GetLibName()==sLib )
		return pDesc;

	sError.SetSprintf ( "unable to load plugin '%s' from '%s': it has already been loaded from library '%s'",
		szName, sLib.cstr(), pDesc->GetLibName().cstr() );
	return nullptr;
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
	ScopedMutex_t tLock ( g_tPluginMutex );
	for ( const auto& tPlugin : g_hPlugins )
	{
		const PluginKey_t & k = tPlugin.first;
		const PluginDescRefPtr_c v = tPlugin.second;

		CSphString sBuf;
		if ( k.m_eType==PLUGIN_FUNCTION )
			sBuf.SetSprintf ( "CREATE FUNCTION %s RETURNS %s SONAME '%s';\n", k.m_sName.cstr(),
				UdfReturnType ( ((PluginUDF_c*)v.Ptr())->m_eRetType ), v->GetLibName().cstr() );
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
	ScopedMutex_t tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDescRefPtr_c* pp = g_hPlugins(k);
	return pp && *pp;
}


PluginDescRefPtr_c PluginGetDesc ( PluginType_e eType, const char * sName )
{
	if ( !g_bPluginsEnabled )
		return nullptr;

	ScopedMutex_t tLock ( g_tPluginMutex );
	PluginKey_t k ( eType, sName );
	PluginDescRefPtr_c* pp = g_hPlugins(k);
	if ( !pp || !*pp )
		return nullptr;
	return *pp;
}


void sphPluginList ( CSphVector<PluginInfo_t> & dResult )
{
	if ( !g_bPluginsEnabled )
		return;
	ScopedMutex_t tLock ( g_tPluginMutex );
	for ( const auto& tPlugin : g_hPlugins )
	{
		const PluginKey_t & k = tPlugin.first;
		const PluginDescRefPtr_c v = tPlugin.second;

		PluginInfo_t & p = dResult.Add();
		p.m_eType = k.m_eType;
		p.m_sName = k.m_sName;
		p.m_sLib = v->GetLibName().cstr();
		p.m_iUsers = v->GetRefcount() - 1; // except the one reference from the hash itself
		if ( p.m_eType==PLUGIN_FUNCTION )
			p.m_sExtra = UdfReturnType ( ((PluginUDF_c*)v.Ptr())->m_eRetType );
	}
}

PluginUDF_c::PluginUDF_c ( PluginLibRefPtr_c pLib, ESphAttr eRetType )
	: PluginDesc_c ( std::move ( pLib ) )
	, m_eRetType ( eRetType )
{}

PluginRanker_c::PluginRanker_c ( PluginLibRefPtr_c pLib )
	: PluginDesc_c ( std::move ( pLib ) )
{}

PluginTokenFilter_c::PluginTokenFilter_c ( PluginLibRefPtr_c pLib )
	: PluginDesc_c ( std::move ( pLib ) )
{}

PluginQueryTokenFilter_c::PluginQueryTokenFilter_c ( PluginLibRefPtr_c pLib )
	: PluginDesc_c ( std::move ( pLib ) )
{}
