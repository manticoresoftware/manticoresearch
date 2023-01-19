//
// Copyright (c) 2020-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarlib.h"
#include "sphinxutils.h"
#include "sphinxexpr.h"
#include "libutils.h"
#include "fileutils.h"
#include "schema/columninfo.h"
#include "schema/schema.h"

using CreateStorageReader_fn =	columnar::Columnar_i * (*) ( const std::string & sFilename, uint32_t uTotalDocs, std::string & sError );
using CreateBuilder_fn =		columnar::Builder_i * (*) ( const columnar::Settings_t & tSettings, const common::Schema_t & tSchema, const std::string & sFile, std::string & sError );
using CheckStorage_fn =			void (*) ( const std::string & sFilename, uint32_t uNumRows, std::function<void (const char*)> & fnError, std::function<void (const char*)> & fnProgress );
using VersionStr_fn =			const char * (*)();
using GetVersion_fn	=			int (*)();

static void *					g_pColumnarLib = nullptr;
static CreateStorageReader_fn	g_fnCreateColumnarStorage = nullptr;
static CreateBuilder_fn 		g_fnCreateColumnarBuilder = nullptr;
static CheckStorage_fn			g_fnCheckColumnarStorage = nullptr;
static VersionStr_fn			g_fnVersionStr = nullptr;
static GetVersion_fn			g_fnStorageVersion  = nullptr;

/////////////////////////////////////////////////////////////////////

common::AttrType_e ToColumnarType ( ESphAttr eAttrType, int iBitCount )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_NONE:			return common::AttrType_e::NONE;
	case SPH_ATTR_TOKENCOUNT:
	case SPH_ATTR_INTEGER:		return iBitCount==1 ? common::AttrType_e::BOOLEAN : common::AttrType_e::UINT32;
	case SPH_ATTR_TIMESTAMP:	return common::AttrType_e::TIMESTAMP;
	case SPH_ATTR_BOOL:			return common::AttrType_e::BOOLEAN;
	case SPH_ATTR_FLOAT:		return common::AttrType_e::FLOAT;
	case SPH_ATTR_BIGINT:		return iBitCount==1 ? common::AttrType_e::BOOLEAN : common::AttrType_e::INT64;
	case SPH_ATTR_STRING:		return common::AttrType_e::STRING;
	case SPH_ATTR_UINT32SET:	return common::AttrType_e::UINT32SET;
	case SPH_ATTR_INT64SET:		return common::AttrType_e::INT64SET;
	default:
		assert ( 0 && "Unknown columnar type");
		return common::AttrType_e::NONE;
	}
}


std::unique_ptr<columnar::Columnar_i> CreateColumnarStorageReader ( const CSphString & sFile, DWORD uNumDocs, CSphString & sError )
{
	if ( !IsColumnarLibLoaded() )
	{
		sError = "columnar library not loaded";
		return nullptr;
	}

	std::string sErrorSTL;
	assert ( g_fnCreateColumnarStorage );
	std::unique_ptr<columnar::Columnar_i> pColumnar { g_fnCreateColumnarStorage ( sFile.cstr(), uNumDocs, sErrorSTL ) };
	if ( !pColumnar )
		sError = sErrorSTL.c_str();

	return pColumnar;
}


std::unique_ptr<columnar::Builder_i> CreateColumnarBuilder ( const ISphSchema & tSchema, const columnar::Settings_t & tSettings, const CSphString & sFilename, CSphString & sError )
{
	if ( !IsColumnarLibLoaded() )
	{
		sError = "columnar library not loaded";
		return nullptr;
	}

	common::Schema_t tColumnarSchema;
	std::string sErrorSTL;

	// convert our data types to columnar storage data types
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( !tAttr.IsColumnar() )
			continue;

		common::StringHash_fn fnStringCalcHash = nullptr;
		common::AttrType_e eAttrType = ToColumnarType ( tAttr.m_eAttrType, tAttr.m_tLocator.m_iBitCount );

		// fixme! make default collation configurable
		if ( eAttrType==common::AttrType_e::STRING && tAttr.HasStringHashes() )
			fnStringCalcHash = LibcCIHash_fn::Hash;

		tColumnarSchema.push_back ( { tAttr.m_sName.cstr(), eAttrType, fnStringCalcHash } );
	}

	if ( tColumnarSchema.empty() )
		return nullptr;

	assert ( g_fnCreateColumnarBuilder );
	std::unique_ptr<columnar::Builder_i> pBuilder { g_fnCreateColumnarBuilder ( tSettings, tColumnarSchema, sFilename.cstr(), sErrorSTL ) };
	if ( !pBuilder )
		sError = sErrorSTL.c_str();

	return pBuilder;
}


void CheckColumnarStorage ( const CSphString & sFile, DWORD uNumRows, std::function<void (const char*)> fnError, std::function<void (const char*)> fnProgress )
{
	if ( !IsColumnarLibLoaded() )
	{
		fnError ( "columnar library not loaded" );
		return;
	}

	assert ( g_fnCheckColumnarStorage );
	g_fnCheckColumnarStorage ( sFile.cstr(), (uint32_t)uNumRows, fnError, fnProgress );
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

static CSphString TryDifferentPaths ( const CSphString & sLibfile )
{
	CSphString sPath = GET_COLUMNAR_FULLPATH();
	if ( sphFileExists ( sPath.cstr() ) )
		return sPath;

	CSphString sPathToExe = GetPathOnly ( GetExecutablePath() );
	sPath.SetSprintf ( "%s%s", sPathToExe.cstr(), sLibfile.cstr() );
	if ( sphFileExists ( sPath.cstr() ) )
		return sPath;

	return "";
}


bool InitColumnar ( CSphString & sError )
{
	assert ( !g_pColumnarLib );

	CSphString sLibfile = TryDifferentPaths ( LIB_MANTICORE_COLUMNAR );
	if ( sLibfile.IsEmpty() )
		return true;

	ScopedHandle_c tHandle ( dlopen ( sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL ) );
	if ( !tHandle.Get() )
	{
		const char * szDlError = dlerror();
		sError.SetSprintf ( "dlopen() failed: %s", szDlError ? szDlError : "(null)" );
		return true;		// if dlopen fails, don't report an error
	}

	sphLogDebug ( "dlopen(%s)=%p", sLibfile.cstr(), tHandle.Get() );

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
	if ( !LoadFunc ( g_fnCheckColumnarStorage, tHandle.Get(), "CheckColumnarStorage", sLibfile, sError ) )			return false;
	if ( !LoadFunc ( g_fnVersionStr, tHandle.Get(), "GetColumnarLibVersionStr", sLibfile, sError ) )				return false;
	if ( !LoadFunc ( g_fnStorageVersion, tHandle.Get(), "GetColumnarStorageVersion", sLibfile, sError ) )			return false;

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


int GetColumnarStorageVersion()
{
	if ( !IsColumnarLibLoaded() )
		return -1;

	assert ( g_fnStorageVersion );
	return g_fnStorageVersion();
}


bool IsColumnarLibLoaded()
{
	return !!g_pColumnarLib;
}


std::unique_ptr<columnar::Iterator_i> CreateColumnarIterator ( const columnar::Columnar_i * pColumnar, const std::string&  sName, std::string & sError, const columnar::IteratorHints_t & tHints, columnar::IteratorCapabilities_t * pCapabilities )
{
	return std::unique_ptr<columnar::Iterator_i> { pColumnar->CreateIterator ( sName, tHints, pCapabilities, sError ) };
}
