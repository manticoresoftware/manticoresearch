//
// Copyright (c) 2020-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarlib.h"
#include "sphinx.h"
#include "libutils.h"

using CreateStorageReader_fn =	columnar::Columnar_i * (*) ( const std::string & sFilename, uint32_t uTotalDocs, std::string & sError );
using CreateBuilder_fn =		columnar::Builder_i * (*) ( const columnar::Settings_t & tSettings, const columnar::Schema_t & tSchema, const std::string & sFile, std::string & sError );
using Setup_fn =				void (*) ( columnar::Malloc_fn, columnar::Free_fn );
using VersionStr_fn =			const char * (*)();

static void *					g_pColumnarLib = nullptr;
static CreateStorageReader_fn	g_fnCreateColumnarStorage = nullptr;
static CreateBuilder_fn 		g_fnCreateColumnarBuilder = nullptr;
static Setup_fn 				g_fnSetupColumnar;
static VersionStr_fn			g_fnVersionStr = nullptr;

/////////////////////////////////////////////////////////////////////

static columnar::AttrType_e ToColumnarType ( ESphAttr eAttrType, int iBitCount )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_NONE:			return columnar::AttrType_e::NONE;
	case SPH_ATTR_INTEGER:		return iBitCount==1 ? columnar::AttrType_e::BOOLEAN : columnar::AttrType_e::UINT32;
	case SPH_ATTR_TIMESTAMP:	return columnar::AttrType_e::TIMESTAMP;
	case SPH_ATTR_BOOL:			return columnar::AttrType_e::BOOLEAN;
	case SPH_ATTR_FLOAT:		return columnar::AttrType_e::FLOAT;
	case SPH_ATTR_BIGINT:		return iBitCount==1 ? columnar::AttrType_e::BOOLEAN : columnar::AttrType_e::INT64;
	case SPH_ATTR_STRING:		return columnar::AttrType_e::STRING;
	case SPH_ATTR_UINT32SET:	return columnar::AttrType_e::UINT32SET;
	case SPH_ATTR_INT64SET:		return columnar::AttrType_e::INT64SET;
	default:
		assert ( 0 && "Unknown columnar type");
		return columnar::AttrType_e::NONE;
	}
}


columnar::Columnar_i * CreateColumnarStorageReader ( const CSphString & sFile, DWORD uNumDocs, CSphString & sError )
{
	if ( !IsColumnarLibLoaded() )
	{
		sError = "columnar library not loaded";
		return nullptr;
	}

	std::string sErrorSTL;
	assert ( g_fnCreateColumnarStorage );
	columnar::Columnar_i * pColumnar = g_fnCreateColumnarStorage ( sFile.cstr(), uNumDocs, sErrorSTL );
	if ( !pColumnar )
		sError = sErrorSTL.c_str();

	return pColumnar;
}


columnar::Builder_i * CreateColumnarBuilder ( const ISphSchema & tSchema, const columnar::Settings_t & tSettings, const CSphString & sFilename, CSphString & sError )
{
	if ( !IsColumnarLibLoaded() )
	{
		sError = "columnar library not loaded";
		return nullptr;
	}

	columnar::Schema_t tColumnarSchema;
	std::string sErrorSTL;

	// convert our data types to columnars storage data types
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( !tAttr.IsColumnar() )
			continue;

		columnar::StringHash_fn fnStringCalcHash = nullptr;
		columnar::AttrType_e eAttrType = ToColumnarType ( tAttr.m_eAttrType, tAttr.m_tLocator.m_iBitCount );

		// fixme! make default collation configurable
		if ( eAttrType==columnar::AttrType_e::STRING && tAttr.HasStringHashes() )
			fnStringCalcHash = LibcCIHash_fn::Hash;

		tColumnarSchema.push_back ( { tAttr.m_sName.cstr(), eAttrType, fnStringCalcHash } );
	}

	if ( tColumnarSchema.empty() )
		return nullptr;

	assert ( g_fnCreateColumnarBuilder );
	columnar::Builder_i * pBuilder = g_fnCreateColumnarBuilder ( tSettings, tColumnarSchema, sFilename.cstr(), sErrorSTL );
	if ( !pBuilder )
		sError = sErrorSTL.c_str();

	return pBuilder;
}

#if HAVE_DLOPEN
template <typename T>
static bool LoadFunc ( T & pFunc, void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError )
{
	pFunc = (T) dlsym ( pHandle, szFunc );
	if ( !pFunc )
	{
		sError.SetSprintf ( "symbol '%s' not found in '%s'", szFunc, sLib.cstr() );
		dlclose ( pHandle );
		return false;
	}

	return true;
}


class ScopedHandle_c
{
public:
	ScopedHandle_c ( void * pHandle )
		: m_pHandle ( pHandle )
	{}

	~ScopedHandle_c()
	{
		if ( m_pHandle )
			dlclose(m_pHandle);
	}

	void * Leak()
	{
		void * pHandle = m_pHandle;
		m_pHandle = nullptr;
		return pHandle;
	}

	void * Get() { return m_pHandle; }

private:
	void * m_pHandle = nullptr;
};

bool InitColumnar ( CSphString & sError )
{
	assert ( !g_pColumnarLib );

	CSphString sLibfile = GET_COLUMNAR_FULLPATH();
	ScopedHandle_c tHandle ( dlopen ( sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL ) );
	if ( !tHandle.Get() )
	{
		const char * szDlError = dlerror();
		sError.SetSprintf ( "dlopen() failed: %s", szDlError ? szDlError : "(null)" );
		return true;		// if dlopen fails, don't report an error
	}

	sphLogDebug ( "dlopen(%s)=%p", sLibfile.cstr(), tHandle.Get() );

	using GetVersion_fn = int (*)();
	GetVersion_fn fnGetVersion;
	if ( !LoadFunc ( fnGetVersion, tHandle.Get(), "GetColumnarLibVersion", sLibfile, sError ) )
		return false;

	int iLibVersion = fnGetVersion();
	if ( iLibVersion!=columnar::LIB_VERSION )
	{
		sError.SetSprintf ( "daemon requires columnar library v%d (trying to load v%d)", columnar::LIB_VERSION, iLibVersion );
		return false;
	}

	if ( !LoadFunc ( g_fnCreateColumnarStorage, tHandle.Get(), "CreateColumnarStorageReader", sLibfile, sError ) )	return false;
	if ( !LoadFunc ( g_fnCreateColumnarBuilder, tHandle.Get(), "CreateColumnarBuilder", sLibfile, sError ) )		return false;
	if ( !LoadFunc ( g_fnSetupColumnar, tHandle.Get(), "SetupColumnar", sLibfile, sError ) )						return false;
	if ( !LoadFunc ( g_fnVersionStr, tHandle.Get(), "GetColumnarLibVersionStr", sLibfile, sError ) )				return false;

	g_fnSetupColumnar ( malloc, free );
	g_pColumnarLib = tHandle.Leak();

	return true;
}

void ShutdownColumnar()
{
	if ( g_pColumnarLib )
		dlclose(g_pColumnarLib);
}

#else
bool InitColumnar ( CSphString & sError )
{
	return false;
}


void ShutdownColumnar()
{
}
#endif

const char * GetColumnarVersionStr()
{
	if ( !IsColumnarLibLoaded() )
		return nullptr;

	assert ( g_fnVersionStr );
	return g_fnVersionStr();
}


bool IsColumnarLibLoaded()
{
	return !!g_pColumnarLib;
}
