//
// Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
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

namespace sec {
using CheckStorage_fn =			void (*) ( const std::string & sFilename, uint32_t uNumRows, std::function<void (const char*)> & fnError, std::function<void (const char*)> & fnProgress );
using VersionStr_fn =			const char * (*)();
using GetVersion_fn	=			int (*)();
using CreateSI_fn =				SI::Index_i * (*) ( const char * sFile, std::string & sError );
using CreateBuilder_fn =		SI::Builder_i *	(*) ( const common::Schema_t & tSchema, size_t tMemoryLimit, const std::string & sFile, size_t tBufferSize, std::string & sError );
}

static void *					g_pSecondaryLib = nullptr;
static sec::VersionStr_fn		g_fnVersionSecStr = nullptr;
static sec::CreateSI_fn			g_fnCreateSI = nullptr;
static sec::CreateBuilder_fn	g_fnCreateBuilder = nullptr;

/////////////////////////////////////////////////////////////////////

#if HAVE_DLOPEN
bool InitSecondary ( CSphString & sError )
{
	assert ( !g_pSecondaryLib );

	CSphString sLibfile = TryDifferentPaths ( LIB_MANTICORE_SECONDARY, GetSecondaryFullpath(), SI::LIB_VERSION );
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

	sec::GetVersion_fn fnGetVersion;
	if ( !LoadFunc ( fnGetVersion, tHandle.Get(), "GetSecondaryLibVersion", sLibfile, sError ) )
		return false;

	int iLibVersion = fnGetVersion();
	if ( iLibVersion!=SI::LIB_VERSION )
	{
		sError.SetSprintf ( "daemon requires secondary library v%d (trying to load v%d)", SI::LIB_VERSION, iLibVersion );
		return false;
	}

	if ( !LoadFunc ( g_fnVersionSecStr, tHandle.Get(), "GetSecondaryLibVersionStr", sLibfile, sError ) )				return false;
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

	assert ( g_fnVersionSecStr );
	return g_fnVersionSecStr();
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

std::unique_ptr<SI::Builder_i> CreateSecondaryIndexBuilder ( const common::Schema_t & tSchema, int64_t iMemoryLimit, const CSphString & sFile, int iBufferSize, CSphString & sError )
{
	if ( !IsSecondaryLibLoaded() )
	{
		sError = "secondary index library not loaded";
		return nullptr;
	}

	assert ( g_fnCreateBuilder );

	std::string sTmpError;
	std::unique_ptr<SI::Builder_i> pBuilder { g_fnCreateBuilder ( tSchema, iMemoryLimit, sFile.cstr(), iBufferSize, sTmpError ) };
	if ( !pBuilder )
		sError = sTmpError.c_str();

	return pBuilder;
}
