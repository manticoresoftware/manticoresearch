//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

std::optional<CSphString> TryPath ( const CSphString& sFullpath, int iVersion )
{
	// first try versioned variant. So, that non-versioned libs from 'ancient age' doesn't suppress new one one
	auto sVersionedFullPath = SphSprintf ( "%s.%i", sFullpath.cstr(), iVersion );
	if ( sphFileExists ( sVersionedFullPath.cstr() ) )
		return sVersionedFullPath;

	if ( sphFileExists ( sFullpath.cstr() ) )
		return sFullpath;

	return std::nullopt;
}

CSphString TryDifferentPaths ( const CSphString & sLibfile, const CSphString & sFullpath, int iVersion )
{
	auto sAnyPath = TryPath ( sFullpath, iVersion );
	if ( sAnyPath )
		return sAnyPath.value();

#if _WIN32
	CSphString sPathToExe = GetPathOnly ( GetExecutablePath() );
	CSphString sPath;
	sPath.SetSprintf ( "%s%s", sPathToExe.cstr(), sLibfile.cstr() );
	sAnyPath = TryPath ( sPath, iVersion );
#endif

	return sAnyPath.value_or("");
}
