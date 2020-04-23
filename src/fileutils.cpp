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

#if USE_WINDOWS
	#define getcwd		_getcwd
#else
	#include <glob.h>
#endif

// whether to collect IO stats
static bool g_bCollectIOStats = false;
static TLS_T<CSphIOStats*> g_pTlsIOStats;


CSphIOStats::~CSphIOStats ()
{
	Stop();
}

void CSphIOStats::Start()
{
	if ( !g_bCollectIOStats )
		return;

	m_pPrev = g_pTlsIOStats;
	g_pTlsIOStats = this;
	m_bEnabled = true;
}

void CSphIOStats::Stop()
{
	if ( !g_bCollectIOStats )
		return;

	m_bEnabled = false;
	g_pTlsIOStats = m_pPrev;
}


void CSphIOStats::Add ( const CSphIOStats & b )
{
	m_iReadTime += b.m_iReadTime;
	m_iReadOps += b.m_iReadOps;
	m_iReadBytes += b.m_iReadBytes;
	m_iWriteTime += b.m_iWriteTime;
	m_iWriteOps += b.m_iWriteOps;
	m_iWriteBytes += b.m_iWriteBytes;
}


//////////////////////////////////////////////////////////////////////////

CSphIOStats * GetIOStats()
{
	if ( !g_bCollectIOStats )
		return nullptr;

	CSphIOStats * pIOStats = g_pTlsIOStats;

	if ( !pIOStats || !pIOStats->IsEnabled() )
		return nullptr;
	return pIOStats;
}

//////////////////////////////////////////////////////////////////////////

bool CSphSavedFile::Collect ( const char * szFilename, CSphString * pError )
{
	if ( !szFilename || !*szFilename )
	{
		memset ( this, 0, sizeof(*this) );
		return true;
	}

	m_sFilename = szFilename;

	struct_stat tStat = {0};
	if ( stat ( szFilename, &tStat ) < 0 )
	{
		if ( pError )
			*pError = strerrorm ( errno );
		return false;
	}

	m_uSize = tStat.st_size;
	m_uCTime = tStat.st_ctime;
	m_uMTime = tStat.st_mtime;

	DWORD uCRC32 = 0;
	if ( !sphCalcFileCRC32 ( szFilename, uCRC32 ) )
		return false;

	m_uCRC32 = uCRC32;

	return true;
}


void CSphSavedFile::Read ( CSphReader & tReader, const char * szFilename, bool bSharedStopwords, CSphString * sWarning )
{
	m_uSize = tReader.GetOffset ();
	m_uCTime = tReader.GetOffset ();
	m_uMTime = tReader.GetOffset ();
	m_uCRC32 = tReader.GetDword ();
	m_sFilename = szFilename;

	CSphString sName ( szFilename );

	if ( !sName.IsEmpty() && sWarning )
	{
		if ( !sphIsReadable ( sName ) && bSharedStopwords )
		{
			StripPath ( sName );
			sName.SetSprintf ( "%s/stopwords/%s", FULL_SHARE_DIR, sName.cstr() );
		}

		struct_stat tFileInfo;
		if ( stat ( sName.cstr(), &tFileInfo ) < 0 )
		{
			if ( bSharedStopwords )
				sWarning->SetSprintf ( "failed to stat either %s or %s: %s", szFilename, sName.cstr(), strerrorm(errno) );
			else
				sWarning->SetSprintf ( "failed to stat %s: %s", szFilename, strerrorm(errno) );
		}
		else
		{
			DWORD uMyCRC32 = 0;
			if ( !sphCalcFileCRC32 ( sName.cstr(), uMyCRC32 ) )
				sWarning->SetSprintf ( "failed to calculate CRC32 for %s", sName.cstr() );
			else
				if ( uMyCRC32!=m_uCRC32 || tFileInfo.st_size!=m_uSize
					|| tFileInfo.st_ctime!=m_uCTime || tFileInfo.st_mtime!=m_uMTime )
					sWarning->SetSprintf ( "'%s' differs from the original", sName.cstr() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

extern DWORD g_dSphinxCRC32 [ 256 ];

bool sphCalcFileCRC32 ( const char * szFilename, DWORD & uCRC32 )
{
	uCRC32 = 0;

	if ( !szFilename )
		return false;

	FILE * pFile = fopen ( szFilename, "rb" );
	if ( !pFile )
		return false;

	DWORD crc = ~((DWORD)0);

	const int BUFFER_SIZE = 131072;
	static BYTE * pBuffer = NULL;
	if ( !pBuffer )
		pBuffer = new BYTE [ BUFFER_SIZE ];

	int iBytesRead;
	while ( ( iBytesRead = fread ( pBuffer, 1, BUFFER_SIZE, pFile ) )!=0 )
	{
		for ( int i=0; i<iBytesRead; i++ )
			crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ pBuffer[i]) & 0xff ];
	}

	fclose ( pFile );

	uCRC32 = ~crc;
	return true;
}


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


bool sphFileExists ( const char * szFilename, CSphString * pError )
{
	struct_stat st = {0};
	if( stat( szFilename, &st ) != 0 )
	{
		if ( pError )
			pError->SetSprintf ( "cannot access %s", szFilename );

		return false;
	}
	else if ( st.st_mode & S_IFDIR )
	{
		if ( pError )
			pError->SetSprintf ( "%s is not a file", szFilename );

		return false;
	}

	return true;
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


void sphInitIOStats()
{
	g_bCollectIOStats = true;
}


void sphDoneIOStats()
{
	g_bCollectIOStats = false;
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


int64_t sphRead ( int iFD, void * pBuf, size_t iCount )
{
	CSphIOStats * pIOStats = GetIOStats();
	int64_t tmStart = 0;
	if ( pIOStats )
		tmStart = sphMicroTimer();

	int64_t iRead = ::read ( iFD, pBuf, iCount );

	if ( pIOStats )
	{
		pIOStats->m_iReadTime += sphMicroTimer() - tmStart;
		pIOStats->m_iReadOps++;
		pIOStats->m_iReadBytes += (-1==iRead) ? 0 : iCount;
	}

	return iRead;
}


bool sphWrite ( int iFD, const void * pBuf, size_t iSize )
{
	return ( iSize==(size_t)::write ( iFD, pBuf, iSize ) );
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


bool CopyFile ( const CSphString & sSource, const CSphString & sDest, CSphString & sError )
{
	const int BUFFER_SIZE = 1048576;
	CSphFixedVector<BYTE> dBuffer(BUFFER_SIZE);

	CSphAutofile tSource;
	int iSrcFD = tSource.Open ( sSource, SPH_O_READ, sError );
	if ( iSrcFD<0 )
		return false;

	CSphAutofile tDest;
	int iDstFD = tDest.Open ( sDest, SPH_O_NEW, sError );
	if ( iDstFD<0 )
		return false;

	int64_t iRead = 0;
	while ( ( iRead = sphRead ( iSrcFD, dBuffer.Begin(), dBuffer.GetLength() ) ) > 0 )
	{
		if ( !sphWrite ( iDstFD, dBuffer.Begin(), iRead ) )
		{
			iRead = -1;
			break;
		}
	}

	if ( iRead<0 )
	{
		sError.SetSprintf ( "Unable to copy file '%s' to '%s': %s", sSource.cstr(), sDest.cstr(), strerror(errno) );
		return false;
	}

	return true;
}


bool RenameFiles ( const StrVec_t & dSrc, const StrVec_t & dDst, CSphString & sError )
{
	bool bError = false;
	ARRAY_FOREACH ( i, dSrc )
	{
		if ( sph::rename ( dSrc[i].cstr(), dDst[i].cstr() ) )
		{
			sError.SetSprintf ( "failed to rename %s to %s", dSrc[i].cstr(), dDst[i].cstr() );
			bError = true;
		}
	}

	return bError;
}


bool RenameWithRollback ( const StrVec_t & dSrc, const StrVec_t & dDst, CSphString & sError )
{
	assert ( dSrc.GetLength()==dDst.GetLength() );
	if ( !dSrc.GetLength() )
		return true;

	CSphBitvec dRenamed ( dSrc.GetLength() );
	bool bError = false;
	ARRAY_FOREACH_COND ( i, dSrc, !bError )
	{
		if ( sph::rename ( dSrc[i].cstr(), dDst[i].cstr() ) )
		{
			sError.SetSprintf ( "failed to rename %s to %s", dSrc[i].cstr(), dDst[i].cstr() );
			bError = true;
		}
		else
			dRenamed.BitSet(i);
	}

	if ( !bError )
		return true;

	// roll back renaming
	ARRAY_FOREACH ( i, dSrc )
	{
		if ( dRenamed.BitGet(i) )
			sph::rename ( dDst[i].cstr(), dSrc[i].cstr() ); // ignore errors
	}

	return false;
}


namespace sph
{
	int rename ( const char * sOld, const char * sNew )
	{
#if USE_WINDOWS
		if ( MoveFileEx ( sOld, sNew, MOVEFILE_REPLACE_EXISTING ) )
			return 0;
		errno = GetLastError();
		return -1;
#else
		return ::rename ( sOld, sNew );
#endif
	}
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


static bool IsSlash ( char c )
{
	return c=='/' || c=='\\';
}


CSphString & StripPath ( CSphString & sPath )
{
	if ( sPath.IsEmpty() )
		return sPath;

	const char * s = sPath.cstr();
	const char * sLastSlash = s;
	for ( ; *s; ++s )
		if ( IsSlash(*s) )
			sLastSlash = s;

	if ( !IsSlash ( *sLastSlash ) )
		return sPath;

	auto iPos = (int)( sLastSlash - sPath.cstr() + 1 );
	auto iLen = (int)( s - sPath.cstr() );
	sPath = sPath.SubString ( iPos, iLen - iPos );
	return sPath;
}


CSphString GetPathOnly ( const CSphString & sFullPath )
{
	if ( sFullPath.IsEmpty() )
		return CSphString();

	const char * pStart = sFullPath.cstr();
	const char * pCur = pStart + sFullPath.Length() - 1;

	if ( IsSlash(*pCur) )
		return sFullPath;

	while ( pCur>pStart && !IsSlash ( pCur[-1] ) )
		pCur--;

	CSphString sPath;
	if ( pCur==pStart )
		sPath = sFullPath;
	else
		sPath.SetBinary ( pStart, pCur-pStart );

	return sPath;

}


const char * GetExtension ( const CSphString & sFullPath )
{
	if ( sFullPath.IsEmpty() )
		return nullptr;

	const char * pDot = strchr ( sFullPath.cstr(), '.' );
	if ( !pDot || pDot[1]=='\0' )
		return nullptr;

	return pDot+1;
}