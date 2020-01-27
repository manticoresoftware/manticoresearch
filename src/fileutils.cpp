//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "fileutils.h"
#include "sphinxint.h"

#include <sys/stat.h>


#if USE_WINDOWS
	#define getcwd		_getcwd
#else
	#include <glob.h>
#endif


bool sphIsReadable ( const char * sPath, CSphString * pError )
{
	int iFD = ::open ( sPath, SPH_O_READ );

	if ( iFD<0 )
	{
		if ( pError )
			pError->SetSprintf ( "%s unreadable: %s", sPath, strerror(errno) );
		return false;
	}

	close ( iFD );
	return true;
}


bool sphIsReadable ( const CSphString & sPath, CSphString * pError )
{
	return sphIsReadable ( sPath.cstr(), pError );
}


bool sphDirExists ( const char * szFilename, CSphString * pError )
{
	struct_stat st = {0};
	if( stat( szFilename, &st ) != 0 )
	{
		if ( pError )
			pError->SetSprintf ( "cannot access %s", szFilename );

		return false;
	}
	else if ( !(st.st_mode & S_IFDIR) )
	{
		if ( pError )
			pError->SetSprintf ( "%s is not a directory", szFilename );

		return false;
	}

	return true;
}


int sphOpenFile ( const char * sFile, CSphString & sError, bool bWrite )
{
	int iFlags = bWrite ? O_RDWR : SPH_O_READ;
	int iFD = ::open ( sFile, iFlags, 0644 );
	if ( iFD<0 )
	{
		sError.SetSprintf ( "failed to open file '%s': '%s'", sFile, strerror(errno) );
		return -1;
	}

	return iFD;
}


int64_t sphGetFileSize ( int iFD, CSphString * sError )
{
	if ( iFD<0 )
	{
		if ( sError )
			sError->SetSprintf ( "invalid descriptor to fstat '%d'", iFD );
		return -1;
	}

	struct_stat st;
	if ( fstat ( iFD, &st )<0 )
	{
		if ( sError )
			sError->SetSprintf ( "failed to fstat file '%d': '%s'", iFD, strerror(errno) );
		return -1;
	}

	return st.st_size;
}


int64_t sphGetFileSize ( const CSphString& sFile, CSphString * sError )
{
	struct_stat st = {0};
	if ( stat ( sFile.cstr(), &st )<0 )
	{
		if ( sError )
			sError->SetSprintf ( "failed to stat file '%s': '%s'", sFile.cstr(), strerror ( errno ) );
		return -1;
	}

	return st.st_size;
}


bool sphTruncate ( int iFD )
{
#if USE_WINDOWS
	return SetEndOfFile ( (HANDLE) _get_osfhandle(iFD) )!=0;
#else
	auto iPos = ::lseek ( iFD, 0, SEEK_CUR );
	if ( iPos>0 )
		return ::ftruncate ( iFD, iPos )==0;
	sphWarning ( "sphTruncate: failed seek. Error: %d '%s'", errno, strerrorm ( errno ) );
	return false;
#endif
}


CSphString sphNormalizePath( const CSphString& sOrigPath )
{
	CSphVector<Str_t> dChunks;
	const char* sBegin = sOrigPath.scstr();
	const char* sEnd = sBegin + sOrigPath.Length();
	const char* sPath = sBegin;
	int iLevel = 0;

	while ( sPath<sEnd )
	{
		const char* sSlash = ( char* ) memchr( sPath, '/', sEnd - sPath );
		if ( !sSlash )
			sSlash = sEnd;

		auto iChunkLen = sSlash - sPath;

		switch ( iChunkLen )
		{
		case 0: // empty chunk skipped
			++sPath;
			continue;
		case 1: // simple dot chunk skipped
			if ( *sPath=='.' )
			{
				sPath += 2;
				continue;
			}
			break;
		case 2: // double dot abandons chunks, then decrease level
			if ( sPath[0]=='.' && sPath[1]=='.' )
			{
				if ( dChunks.IsEmpty())
					--iLevel;
				else
					dChunks.Pop();
				sPath += 3;
				continue;
			}
		default: break;
		}
		dChunks.Add( { sPath, iChunkLen } );
		sPath = sSlash + 1;
	}

	StringBuilder_c sResult( "/" );
	if ( *sBegin=='/' )
		sResult.AppendRawChunk ( {"/", 1} );
	else
		while ( iLevel++<0 )
			sResult << "..";

	for ( const auto& dChunk: dChunks )
		sResult.AppendChunk ( dChunk );

	return sResult.cstr();
}


CSphString sphGetCwd()
{
	CSphVector<char> sBuf (65536);
	return getcwd( sBuf.begin(), sBuf.GetLength());
}


#if USE_WINDOWS
static void AddFile ( StrVec_t & dFilesFound, const CSphString & sPath, const WIN32_FIND_DATA & tFFData, bool bNeedDirs )
{
	bool bDir = !!( tFFData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
	if ( bDir && !bNeedDirs )
		return;

	if ( sPath.IsEmpty() )
		dFilesFound.Add ( tFFData.cFileName );
	else
		dFilesFound.Add().SetSprintf ( "%s%s", sPath.cstr(), tFFData.cFileName );
}
#endif


StrVec_t FindFiles ( const char * szPath, bool bNeedDirs )
{
	StrVec_t dFilesFound;

#if USE_WINDOWS
	WIN32_FIND_DATA tFFData;
	const char * szLastSlash = NULL;
	for ( const char * s = szPath; *s; s++ )
		if ( *s=='/' || *s=='\\' )
			szLastSlash = s;

	CSphString sPath;
	if ( szLastSlash )
	{
		sPath = szPath;
		sPath = sPath.SubString ( 0, szLastSlash - szPath + 1 );
	}

	HANDLE hFind = FindFirstFile ( szPath, &tFFData );
	if ( hFind!=INVALID_HANDLE_VALUE )
	{
		AddFile ( dFilesFound, sPath, tFFData, bNeedDirs );
		while ( FindNextFile ( hFind, &tFFData )!=0 )
			AddFile ( dFilesFound, sPath, tFFData, bNeedDirs );

		FindClose(hFind);
	}
#else
	glob_t tGlob;
	glob ( szPath, GLOB_MARK | GLOB_NOSORT, NULL, &tGlob );
	if ( tGlob.gl_pathv )
		for ( int i = 0; i < (int)tGlob.gl_pathc; i++ )
		{
			const char * szPathName = tGlob.gl_pathv[i];
			if ( !szPathName )
				continue;

			size_t iLen = strlen ( szPathName );
			if ( !iLen || ( !bNeedDirs && szPathName[iLen-1]=='/' ) )
				continue;

			dFilesFound.Add ( szPathName );
		}

	globfree ( &tGlob );
#endif

	dFilesFound.Uniq();
	return dFilesFound;
}


bool MkDir ( const char * szDir )
{
	if ( sphDirExists ( szDir ) )
		return true;

#if USE_WINDOWS
	if ( mkdir ( szDir ) )
#else
	if ( mkdir ( szDir, S_IRWXU ) )
#endif
		return false;

	return true;
}


// check path exists and also check daemon could write there
bool CheckPath ( const CSphString & sPath, bool bCheckWrite, CSphString & sError, const char * sCheckFileName )
{
	if ( !sphDirExists ( sPath.cstr(), &sError ) )
	{
		sError.SetSprintf ( "cannot access directory %s, %s", sPath.cstr(), sError.cstr() );
		return false;
	}

	if ( bCheckWrite )
	{
		CSphString sTmp;
		sTmp.SetSprintf ( "%s/%s", sPath.cstr(), sCheckFileName );
		CSphAutofile tFile ( sTmp, SPH_O_NEW, sError, true );
		if ( tFile.GetFD()<0 )
		{
			sError.SetSprintf ( "directory %s write error: %s", sPath.cstr(), sError.cstr() );
			return false;
		}
	}

	return true;
}
