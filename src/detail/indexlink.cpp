//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "indexlink.h"

#include "fileutils.h"
#include "fileio.h"

// uncomment if need to store absolute (real) new path. String sNewPath should be passed by value for it
//#define STORE_ABSOLUTE 1

bool WriteLinkFile ( const CSphString& sBase, const CSphString& sNewPath, CSphString& sError )
{
	sError = "";
	auto sLink = SphSprintf ( "%s.link", sBase.cstr() );

	// if sNewPath points to base - .link file is not necessary, and we will delete it, if exists.
	if ( RealPath ( sBase ) == RealPath ( sNewPath ) && sphIsReadable ( sLink ) )
	{
		auto iRes = ::unlink ( sLink.cstr() );
		if (!iRes)
			return true;
		sError = strerrorm ( errno );
		return false;
	}

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sLink, sError ) )
		return false;

#if defined( STORE_ABSOLUTE ) && STORE_ABSOLUTE
	auto sStorePath = RealPath ( sNewPath );
#else
	const auto& sStorePath = sNewPath;
#endif

	tWriter.PutBytes ( sStorePath.cstr(), sStorePath.Length() );
	tWriter.CloseFile();
	return !tWriter.IsError();
}

CSphString RedirectToRealPath ( CSphString sBase )
{
	auto sLinkFile = SphSprintf ( "%s.link", sBase.cstr() );
	if ( !sphIsReadable ( sLinkFile ) )
		return sBase;

	CSphAutofile rdFile;
	CSphString sError;
	auto iFileSize = sphGetFileSize ( sLinkFile );
	if ( iFileSize < 0 )
		return sBase;

	CSphAutofile tFile;
	if ( tFile.Open ( sLinkFile, SPH_O_READ, sError ) < 0 )
		return sBase;

	CSphFixedVector<char> dBytes { iFileSize };
	auto iRead = (int64_t)sphReadThrottled ( tFile.GetFD(), dBytes.begin(), iFileSize );
	if ( iRead != iFileSize )
		return sBase;

	sBase.SetBinary ( dBytes.begin(), iFileSize );
	return sBase;
}