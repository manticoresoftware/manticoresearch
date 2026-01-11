//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "libutils.h"
#include "fileutils.h"

#include <optional>

#if _WIN32
void * dlsym ( void * lib, const char * name )
{
	return (void*)GetProcAddress ( (HMODULE)lib, name );
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
	FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, uError, LANG_SYSTEM_DEFAULT, (LPTSTR)sError, sizeof(sError), NULL );
	return sError;
}

#endif // _WIN32

ScopedHandle_c::~ScopedHandle_c ()
{
#if HAVE_DLOPEN
	if ( m_pHandle )
		dlclose ( m_pHandle );
#endif
}

void * DlSym ( void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError )
{
#if HAVE_DLOPEN
	auto * pSym = dlsym ( pHandle, szFunc );
	if ( !pSym )
	{
		sError.SetSprintf ( "symbol '%s' not found in '%s'", szFunc, sLib.cstr() );
		dlclose ( pHandle );
	}

	return pSym;
#else
	return nullptr;
#endif
}


std::optional<CSphString> TryPath ( const CSphString & sFullpath, int iVersion )
{
	// first try versioned variant. So, that non-versioned libs from 'ancient age' doesn't suppress new one
	auto sVersionedFullPath = SphSprintf ( "%s.%i", sFullpath.cstr(), iVersion );
	if ( sphFileExists ( sVersionedFullPath.cstr() ) )
		return sVersionedFullPath;

	if ( sphFileExists ( sFullpath.cstr() ) )
		return sFullpath;

	return std::nullopt;
}


static CSphString AddLibPostfix ( const CSphString & sFilename, const char * szPostfix )
{
	if ( !szPostfix )
		return sFilename;

	CSphString sExt = GetExtension(sFilename);
	CSphString sPathName = GetPathNoExtension(sFilename);
	CSphString sRes;
	sRes.SetSprintf ( "%s%s.%s", sPathName.cstr(), szPostfix, sExt.cstr() );
	return sRes;
}


CSphString TryDifferentPaths ( const CSphString & sLibfile, const CSphString & sFullpath, int iVersion, const char * szPostfix )
{
	CSphString sPathWithPostfix = AddLibPostfix ( sFullpath, szPostfix );

	auto sAnyPath = TryPath ( sPathWithPostfix, iVersion );
	if ( sAnyPath )
		return sAnyPath.value();

#if _WIN32
	CSphString sPathToExe = GetPathOnly ( GetExecutablePath() );
	CSphString sPath;
	CSphString sLibWithPostfix = AddLibPostfix (  sLibfile, szPostfix );
	sPath.SetSprintf ( "%s%s", sPathToExe.cstr(), sLibWithPostfix.cstr() );
	sAnyPath = TryPath ( sPath, iVersion );
#endif

	return sAnyPath.value_or("");
}
