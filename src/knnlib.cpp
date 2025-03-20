//
// Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "knnlib.h"
#include "columnarlib.h"
#include "sphinxutils.h"
#include "libutils.h"
#include "schema/schema.h"

using Create_fn =				knn::KNN_i * (*) ();
using CreateBuilder_fn =		knn::Builder_i * (*) ( const knn::Schema_t & tSchema, int64_t iNumElements );
using CreateDistanceCalc_fn =	knn::Distance_i * (*) ( const knn::IndexSettings_t & tSettings );
using VersionStr_fn =			const char * (*)();
using GetVersion_fn	=			int (*)();

static void *					g_pKNNLib = nullptr;
static Create_fn 				g_fnCreate = nullptr;
static CreateBuilder_fn 		g_fnCreateKNNBuilder = nullptr;
static CreateDistanceCalc_fn	g_fnCreateDistanceCalc = nullptr;
static VersionStr_fn			g_fnVersionStr = nullptr;

/////////////////////////////////////////////////////////////////////

std::unique_ptr<knn::KNN_i>	CreateKNN ( CSphString & sError )
{
	if ( !IsKNNLibLoaded() )
	{
		sError = "knn library not loaded";
		return nullptr;
	}

	assert ( g_fnCreate );
	return std::unique_ptr<knn::KNN_i> ( g_fnCreate() );
}


std::unique_ptr<knn::Builder_i>	CreateKNNBuilder ( const ISphSchema & tSchema, int64_t iNumElements, CSphString & sError )
{
	if ( !IsKNNLibLoaded() )
	{
		sError = "knn library not loaded";
		return nullptr;
	}

	knn::Schema_t tKNNSchema;

	// convert our data types to columnar storage data types
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( !tAttr.IsIndexedKNN() )
			continue;

		common::AttrType_e eAttrType = ToColumnarType ( tAttr.m_eAttrType, tAttr.m_tLocator.m_iBitCount );

		knn::AttrWithSettings_t tKNNAttr;
		(knn::IndexSettings_t &)tKNNAttr = tAttr.m_tKNN;
		tKNNAttr.m_sName = tAttr.m_sName.cstr();
		tKNNAttr.m_eType = eAttrType;
		tKNNSchema.push_back(tKNNAttr);
	}

	if ( tKNNSchema.empty() )
		return nullptr;

	assert ( g_fnCreateKNNBuilder );
	std::unique_ptr<knn::Builder_i> pBuilder { g_fnCreateKNNBuilder ( tKNNSchema, iNumElements ) };
	if ( !pBuilder )
		sError = "error creating knn index builder";

	return pBuilder;
}


std::unique_ptr<knn::Distance_i> CreateKNNDistanceCalc ( const knn::IndexSettings_t & tSettings )
{
	return std::unique_ptr<knn::Distance_i> ( g_fnCreateDistanceCalc(tSettings) );
}


#if HAVE_DLOPEN
bool InitKNN ( CSphString & sError )
{
	assert ( !g_pKNNLib );

	CSphString sLibfile = TryDifferentPaths ( LIB_MANTICORE_KNN, GetKNNFullpath(), knn::LIB_VERSION );
	if ( sLibfile.IsEmpty() )
		return true;

	if ( !IsSSE42Supported() || !IsPOPCNTSupported() )
	{
		sError.SetSprintf ( "MCL requires a CPU that supports SSE 4.2 and POPCNT instruction" );
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
	if ( !LoadFunc ( fnGetVersion, tHandle.Get(), "GetKNNLibVersion", sLibfile, sError ) )
		return false;

	int iLibVersion = fnGetVersion();
	if ( iLibVersion!=knn::LIB_VERSION )
	{
		sError.SetSprintf ( "daemon requires knn library v%d (trying to load v%d)", knn::LIB_VERSION, iLibVersion );
		return false;
	}

	if ( !LoadFunc ( g_fnCreate, tHandle.Get(), "CreateKNN", sLibfile, sError ) )						return false;
	if ( !LoadFunc ( g_fnCreateKNNBuilder, tHandle.Get(), "CreateKNNBuilder", sLibfile, sError ) )		return false;
	if ( !LoadFunc ( g_fnCreateDistanceCalc, tHandle.Get(), "CreateDistanceCalc", sLibfile, sError ) )	return false;
	if ( !LoadFunc ( g_fnVersionStr, tHandle.Get(), "GetKNNLibVersionStr", sLibfile, sError ) )			return false;

	g_pKNNLib = tHandle.Leak();

	return true;
}

void ShutdownKNN()
{
	if ( g_pKNNLib )
		dlclose(g_pKNNLib);
}

#else
bool InitKNN ( CSphString & sError )	{ return false; }
void ShutdownKNN()						{}
#endif

const char * GetKNNVersionStr()
{
	if ( !IsKNNLibLoaded() )
		return nullptr;

	assert ( g_fnVersionStr );
	return g_fnVersionStr();
}


bool IsKNNLibLoaded()
{
	return !!g_pKNNLib;
}
