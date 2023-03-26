//
// Copyright (c) 2020-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include "sphinxutils.h"
#include "libutils.h"
#include "fileutils.h"
#include "schema/columninfo.h"
#include "schema/schema.h"
#include "columnarmisc.h"
#include "secondarylib.h"
#include "std/cpuid.h"

using CheckStorage_fn =			void (*) ( const std::string & sFilename, uint32_t uNumRows, std::function<void (const char*)> & fnError, std::function<void (const char*)> & fnProgress );
using VersionStr_fn =			const char * (*)();
using GetVersion_fn	=			int (*)();
using CreateSI_fn =				SI::Index_i * (*) ( const char * sFile, std::string & sError );
using CreateBuilder_fn =		SI::Builder_i *	(*) ( const SI::Settings_t & tSettings, const common::Schema_t & tSchema, int iMemoryLimit, const std::string & sFile, std::string & sError );

static void *					g_pSecondaryLib = nullptr;
static VersionStr_fn			g_fnVersionStr = nullptr;
static GetVersion_fn			g_fnStorageVersion  = nullptr;
static CreateSI_fn				g_fnCreateSI  = nullptr;
static CreateBuilder_fn			g_fnCreateBuilder  = nullptr;

/////////////////////////////////////////////////////////////////////

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
	CSphString sPath = GET_SECONDARY_FULLPATH();
	if ( sphFileExists ( sPath.cstr() ) )
		return sPath;

#if _WIN32
	CSphString sPathToExe = GetPathOnly ( GetExecutablePath() );
	sPath.SetSprintf ( "%s%s", sPathToExe.cstr(), sLibfile.cstr() );
	if ( sphFileExists ( sPath.cstr() ) )
		return sPath;
#endif

	return "";
}


bool InitSecondary ( CSphString & sError )
{
	assert ( !g_pSecondaryLib );

	CSphString sLibfile = TryDifferentPaths ( LIB_MANTICORE_SECONDARY );
	if ( sLibfile.IsEmpty() )
		return true;

	if ( !IsSSE42Supported() )
	{
		sError.SetSprintf ( "MCL requires a CPU that supports SSE 4.2" );
		return false;
	}

	ScopedHandle_c tHandle ( dlopen ( sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL ) );
	if ( !tHandle.Get() )
	{
		const char * szDlError = dlerror();
		sError.SetSprintf ( "dlopen() failed: %s", szDlError ? szDlError : "(null)" );
		return true;		// if dlopen fails, don't report an error
	}

	sphLogDebug ( "dlopen(%s)=%p", sLibfile.cstr(), tHandle.Get() );

	GetVersion_fn fnGetVersion;
	if ( !LoadFunc ( fnGetVersion, tHandle.Get(), "GetSecondaryLibVersion", sLibfile, sError ) )
		return false;

	int iLibVersion = fnGetVersion();
	if ( iLibVersion!=SI::LIB_VERSION )
	{
		sError.SetSprintf ( "daemon requires secondary library v%d (trying to load v%d)", SI::LIB_VERSION, iLibVersion );
		return false;
	}

	if ( !LoadFunc ( g_fnVersionStr, tHandle.Get(), "GetSecondaryLibVersionStr", sLibfile, sError ) )				return false;
	if ( !LoadFunc ( g_fnStorageVersion, tHandle.Get(), "GetSecondaryStorageVersion", sLibfile, sError ) )			return false;
	if ( !LoadFunc ( g_fnCreateSI, tHandle.Get(), "CreateSecondaryIndex", sLibfile, sError ) )						return false;
	if ( !LoadFunc ( g_fnCreateBuilder, tHandle.Get(), "CreateBuilder", sLibfile, sError ) )						return false;

	g_pSecondaryLib = tHandle.Leak();

	return true;
}

void ShutdownSecondary()
{
	if ( g_pSecondaryLib )
		dlclose ( g_pSecondaryLib );
}

#else
bool InitSecondary ( CSphString & sError )
{
	return false;
}


void ShutdownSecondary()
{
}
#endif

const char * GetSecondaryVersionStr()
{
	if ( !IsSecondaryLibLoaded() )
		return nullptr;

	assert ( g_fnVersionStr );
	return g_fnVersionStr();
}


int GetSecondaryStorageVersion()
{
	if ( !IsSecondaryLibLoaded() )
		return -1;

	assert ( g_fnStorageVersion );
	return g_fnStorageVersion();
}


bool IsSecondaryLibLoaded()
{
	return !!g_pSecondaryLib;
}

SI::Index_i * CreateSecondaryIndex ( const char * sFile, CSphString & sError )
{
	if ( !IsSecondaryLibLoaded() )
	{
		sError = "secondary index library not loaded";
		return nullptr;
	}

	assert ( g_fnCreateSI );

	std::string sTmpError;
	SI::Index_i * pSIdx = g_fnCreateSI ( sFile, sTmpError );
	if ( !pSIdx )
		sError = sTmpError.c_str();

	return pSIdx;
}

std::unique_ptr<SI::Builder_i> CreateSecondaryIndexBuilder ( const common::Schema_t & tSchema, int iMemoryLimit, const CSphString & sFile, CSphString & sError )
{
	if ( !IsSecondaryLibLoaded() )
	{
		sError = "secondary index library not loaded";
		return nullptr;
	}

	assert ( g_fnCreateBuilder );

	std::string sTmpError;
	SI::Settings_t tSettings; // FIXME! use config?
	std::unique_ptr<SI::Builder_i> pBuilder { g_fnCreateBuilder ( tSettings, tSchema, iMemoryLimit, sFile.cstr(), sTmpError ) };
	if ( !pBuilder )
		sError = sTmpError.c_str();

	return pBuilder;
}