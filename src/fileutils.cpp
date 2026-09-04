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

#include "fileutils.h"
#include "sphinxint.h"
#include "std/hash.h"
#include "std/crc32.h"
#include <climits>
#include <string>
#include <sys/stat.h>
#include <vector>

#if _WIN32
	#define getcwd		_getcwd
	#include <shlwapi.h>

#else
	#include <glob.h>
#endif

// whether to collect IO stats
static bool g_bCollectIOStats = false;
static thread_local CSphIOStats* g_pTlsIOStats;

namespace
{

static constexpr int TABLE_LITERAL_MAX = 64;
static constexpr int TABLE_ENCODE_MAX = 64;
bool IsSlash ( char c )
{
	return c=='/' || c=='\\';
}


bool IsPortableTableLiteral ( const CSphString & sName )
{
	if ( sName.IsEmpty() || sName.Length()>TABLE_LITERAL_MAX )
		return false;

	for ( const BYTE * p = (const BYTE *)sName.cstr(); *p; ++p )
		if ( !( *p>='a' && *p<='z' ) && !( *p>='0' && *p<='9' ) && *p!='_' )
			return false;

	static const char * dReserved[] =
	{
		"con", "prn", "aux", "nul",
		"com1", "com2", "com3", "com4", "com5", "com6", "com7", "com8", "com9",
		"lpt1", "lpt2", "lpt3", "lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9"
	};

	for ( const char * szReserved : dReserved )
		if ( sName==szReserved )
			return false;

	return true;
}

} // namespace


CSphString sphGetTablePhysicalName ( const CSphString & sName )
{
	if ( IsPortableTableLiteral(sName) )
		return sName;

	static const char dHex[] = "0123456789abcdef";
	StringBuilder_c sPhysical;
	if ( sName.Length()<=TABLE_ENCODE_MAX )
	{
		sPhysical << "@manticore_u_";
		for ( const BYTE * p = (const BYTE *)sName.cstr(); *p; ++p )
			sPhysical.Appendf ( "%c%c", dHex[*p>>4], dHex[*p&0x0f] );
	}
	else
	{
		sPhysical << "@manticore_h_";
		static constexpr uint64_t dSeeds[] =
		{
			0x243f6a8885a308d3ULL,
			0x13198a2e03707344ULL,
			0xa4093822299f31d0ULL,
			0x082efa98ec4e6c89ULL
		};
		for ( uint64_t uSeed : dSeeds )
		{
			uint64_t uHash = HashWithSeed ( sName.cstr(), sName.Length(), uSeed );
			for ( int iShift=60; iShift>=0; iShift-=4 )
				sPhysical.Appendf ( "%c", dHex[(uHash>>iShift)&0x0f] );
		}
	}

	return sPhysical.cstr();
}


CSphString sphGetConfiglessTablePath ( const CSphString & sDataDir, const CSphString & sName )
{
	CSphString sPhysicalName = sphGetTablePhysicalName ( sName );
	CSphString sPath;
	if ( sDataDir.Length() && !IsSlash ( sDataDir.cstr()[sDataDir.Length()-1] ) )
		sPath.SetSprintf ( "%s/%s", sDataDir.cstr(), sPhysicalName.cstr() );
	else
		sPath.SetSprintf ( "%s%s", sDataDir.cstr(), sPhysicalName.cstr() );

	return sPath;
}


CSphString sphGetExistingConfiglessTablePath ( const CSphString & sDataDir, const CSphString & sName )
{
	CSphString sPath = sphGetConfiglessTablePath ( sDataDir, sName );
	CSphString sLegacyPath;
	if ( sDataDir.Length() && !IsSlash ( sDataDir.cstr()[sDataDir.Length()-1] ) )
		sLegacyPath.SetSprintf ( "%s/%s", sDataDir.cstr(), sName.cstr() );
	else
		sLegacyPath.SetSprintf ( "%s%s", sDataDir.cstr(), sName.cstr() );

	if ( sPath!=sLegacyPath && !sphDirExists ( sPath.cstr() ) && sphDirExists ( sLegacyPath.cstr() ) )
		return sLegacyPath;

	return sPath;
}


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

void SafeClose ( int& iFD )
{
	if ( iFD >= 0 )
		::close ( iFD );
	iFD = -1;
}

#if !_WIN32
bool OpenPidFile ( const CSphString & sPath, int & iFD, int & iPid, CSphString & sError )
{
	iFD = -1;
	iPid = 0;
	int iFlags = O_RDONLY | O_NONBLOCK;
#ifdef O_CLOEXEC
	iFlags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
	iFlags |= O_NOFOLLOW;
#endif
	iFD = open ( sPath.cstr(), iFlags );
	if ( iFD<0 )
	{
		sError.SetSprintf ( "cannot open PID file '%s': %s", sPath.cstr(), strerrorm(errno) );
		return false;
	}

	struct stat tStat {};
	if ( fstat(iFD,&tStat)<0 )
	{
		sError.SetSprintf ( "cannot inspect PID file '%s': %s", sPath.cstr(), strerrorm(errno) );
		SafeClose ( iFD );
		return false;
	}
	if ( !S_ISREG(tStat.st_mode) )
	{
		sError.SetSprintf ( "PID file '%s' is not a regular file", sPath.cstr() );
		SafeClose ( iFD );
		return false;
	}

	char sPid[65] {};
	ssize_t iLength = read ( iFD, sPid, sizeof(sPid) );
	if ( iLength<=0 || iLength>=(ssize_t)sizeof(sPid) )
	{
		sError.SetSprintf ( "invalid PID file '%s'", sPath.cstr() );
		SafeClose ( iFD );
		return false;
	}

	size_t iDigits = 0;
	int64_t iValue = 0;
	while ( iDigits<(size_t)iLength && sPid[iDigits]>='0' && sPid[iDigits]<='9' )
	{
		iValue = iValue*10 + sPid[iDigits]-'0';
		if ( iValue>INT_MAX )
			break;
		++iDigits;
	}
	size_t iTail = iDigits;
	if ( iTail<(size_t)iLength && sPid[iTail]=='\r' ) ++iTail;
	if ( iTail<(size_t)iLength && sPid[iTail]=='\n' ) ++iTail;
	if ( !iDigits || iValue<=0 || iValue>INT_MAX || iTail!=(size_t)iLength )
	{
		sError.SetSprintf ( "invalid PID file '%s'", sPath.cstr() );
		SafeClose ( iFD );
		return false;
	}
	iPid = (int)iValue;
	return true;
}

bool ValidatePidFileOwner ( int iFD, int iPid, CSphString & sError )
{
	struct flock tLock {};
	tLock.l_type = F_RDLCK;
	tLock.l_whence = SEEK_SET;
	if ( fcntl(iFD,F_GETLK,&tLock)<0 )
	{
		sError.SetSprintf ( "cannot inspect PID-file lock: %s", strerrorm(errno) );
		return false;
	}
	if ( tLock.l_type==F_UNLCK || tLock.l_pid!=iPid )
	{
		sError.SetSprintf ( "PID file is not owned by recorded PID %d", iPid );
		return false;
	}
	return true;
}

bool UnlinkFileIfSameDescriptor ( int iFD, const CSphString & sPath, CSphString & sError )
{
	struct stat tOwned {};
	if ( fstat(iFD,&tOwned)<0 )
	{
		sError.SetSprintf ( "cannot inspect owned file '%s': %s", sPath.cstr(), strerrorm(errno) );
		return false;
	}
	std::string sTemplate = std::string(sPath.cstr()) + ".unlink.XXXXXX";
	std::vector<char> dTemplate ( sTemplate.begin(), sTemplate.end() );
	dTemplate.push_back ( '\0' );
	int iPlaceholderFD = mkstemp ( dTemplate.data() );
	if ( iPlaceholderFD<0 )
	{
		sError.SetSprintf ( "cannot reserve PID cleanup path for '%s': %s", sPath.cstr(), strerrorm(errno) );
		return false;
	}
	CSphString sQuarantine = dTemplate.data();
	close ( iPlaceholderFD );
	if ( rename(sPath.cstr(),sQuarantine.cstr())<0 )
	{
		int iError = errno;
		unlink ( sQuarantine.cstr() );
		sError.SetSprintf ( "cannot quarantine PID metadata '%s': %s", sPath.cstr(), strerrorm(iError) );
		return false;
	}

	struct stat tQuarantine {};
	if ( lstat(sQuarantine.cstr(),&tQuarantine)<0 || !S_ISREG(tQuarantine.st_mode) ||
		tOwned.st_dev!=tQuarantine.st_dev || tOwned.st_ino!=tQuarantine.st_ino )
	{
		// A replacement occupied the public pathname. Restore regular metadata
		// only if no newer replacement has appeared since the atomic rename.
		if ( S_ISREG(tQuarantine.st_mode) && link(sQuarantine.cstr(),sPath.cstr())==0 )
			unlink ( sQuarantine.cstr() );
		sError.SetSprintf ( "file path '%s' no longer identifies the owned file", sPath.cstr() );
		return false;
	}
	if ( unlink(sQuarantine.cstr())<0 )
	{
		sError.SetSprintf ( "cannot unlink quarantined PID metadata for '%s': %s", sPath.cstr(), strerrorm(errno) );
		return false;
	}
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////////

#if _WIN32

bool sphLockEx ( int iFile, bool bWait )
{
	HANDLE hHandle = (HANDLE)_get_osfhandle ( iFile );
	if ( hHandle != INVALID_HANDLE_VALUE )
	{
		OVERLAPPED tOverlapped;
		memset ( &tOverlapped, 0, sizeof ( tOverlapped ) );
		return !!LockFileEx ( hHandle, LOCKFILE_EXCLUSIVE_LOCK | ( bWait ? 0 : LOCKFILE_FAIL_IMMEDIATELY ), 0, 1, 0, &tOverlapped );
	}

	return false;
}

void sphLockUn ( int iFile )
{
	HANDLE hHandle = (HANDLE)_get_osfhandle ( iFile );
	if ( hHandle != INVALID_HANDLE_VALUE )
	{
		OVERLAPPED tOverlapped;
		memset ( &tOverlapped, 0, sizeof ( tOverlapped ) );
		UnlockFileEx ( hHandle, 0, 1, 0, &tOverlapped );
	}
}

#else

bool sphLockEx ( int iFile, bool bWait )
{
	struct flock tLock;
	tLock.l_type = F_WRLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;
	if ( !bWait )
		return ( fcntl ( iFile, F_SETLK, &tLock ) != -1 );

#if HAVE_F_SETLKW
	return ( fcntl ( iFile, F_SETLKW, &tLock ) != -1 );
#else
	for ( ;; )
	{
		int iResult = fcntl ( iFile, F_SETLK, &tLock ) if ( iResult != -1 ) return true;
		if ( errno == EACCES || errno == EAGAIN )
			sphSleepMsec ( 10 );
		else
			return false;
	}
#endif
}


void sphLockUn ( int iFile )
{
	struct flock tLock;
	tLock.l_type = F_UNLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;

	if ( fcntl ( iFile, F_SETLK, &tLock ) == -1 )
		sphWarning ( "sphLockUn: failed fcntl. Error: %d '%s'", errno, strerror ( errno ) );
}
#endif

bool RawFileLock ( const CSphString & sFile, int & iLockFD, CSphString & sError )
{
	if ( iLockFD<0 )
	{
		iLockFD = ::open ( sFile.cstr (), SPH_O_NEW, 0644 );
		if ( iLockFD<0 )
		{
			sError.SetSprintf ( "failed to open '%s': %u '%s'", sFile.cstr (), errno, strerrorm ( errno ) );
			sphLogDebug ( "failed to open '%s': %u '%s'", sFile.cstr (), errno, strerrorm ( errno ) );
			return false;
		}
	}

	if ( !sphLockEx ( iLockFD, false ) )
	{
		sError.SetSprintf ( "failed to lock '%s': %u '%s'", sFile.cstr (), errno, strerrorm ( errno ) );
		SafeClose ( iLockFD );
		return false;
	}
	sphLogDebug ( "lock %s success", sFile.cstr () );
	return true;
}

void RawFileUnLock ( const CSphString& sFile, int& iLockFD )
{
	if ( iLockFD < 0 )
		return;
	sphLogDebug ( "File ID ok, closing lock FD %d, unlinking %s", iLockFD, sFile.cstr() );
	sphLockUn ( iLockFD );
	SafeClose ( iLockFD );
	::unlink ( sFile.cstr() );
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

namespace
{
	std::atomic<uint64_t> m_uGlobalWritten {0};
}

void GlobalWrite ( int iWritten ) noexcept
{
	if ( !iWritten )
		return;

	m_uGlobalWritten.fetch_add ( iWritten, std::memory_order_relaxed );
}

uint64_t GlobalWritten () noexcept
{
	return m_uGlobalWritten.load ( std::memory_order_relaxed );
}

//////////////////////////////////////////////////////////////////////////

bool CSphSavedFile::Collect ( const char * szFilename, CSphString * pError )
{
	if ( !szFilename || !*szFilename )
	{
		m_sFilename.SetBinary ( nullptr, 0 );
		m_uSize = m_uCTime = m_uMTime = 0;
		m_uCRC32 = 0;
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


static void ReadSavedFile ( CSphSavedFile & tFile, const char * szFilename, bool bSharedStopwords, CSphString * sWarning )
{
	tFile.m_sFilename = szFilename;

	CSphString sName ( szFilename );

	if ( !sName.IsEmpty() && sWarning )
	{
		if ( !sphIsReadable ( sName ) && bSharedStopwords )
		{
			StripPath ( sName );
			sName.SetSprintf ( "%s/stopwords/%s", GET_FULL_SHARE_DIR (), sName.cstr() );
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
			{
				sWarning->SetSprintf ( "failed to calculate CRC32 for %s", sName.cstr() );
			} else
			{
				if ( uMyCRC32!=tFile.m_uCRC32 || tFileInfo.st_size!=tFile.m_uSize )
					sWarning->SetSprintf ( "'%s' differs from the original", sName.cstr() );
			}
		}
	}
}

void CSphSavedFile::Read ( CSphReader & tReader, const char * szFilename, bool bSharedStopwords, CSphString * sWarning )
{
	m_uSize = tReader.GetOffset ();
	m_uCTime = tReader.GetOffset ();
	m_uMTime = tReader.GetOffset ();
	m_uCRC32 = tReader.GetDword ();
	ReadSavedFile ( *this, szFilename, bSharedStopwords, sWarning );
}


void CSphSavedFile::Read ( const bson::Bson_c& tNode, const char* szFilename, bool bSharedStopwords, CSphString* sWarning )
{
	using namespace bson;
	m_uSize = Int ( tNode.ChildByName ( "size" ) );
	m_uCTime = Int ( tNode.ChildByName ( "ctime" ) );
	m_uMTime = Int ( tNode.ChildByName ( "mtime" ) );
	m_uCRC32 = Int ( tNode.ChildByName ( "crc32" ) );
	ReadSavedFile ( *this, szFilename, bSharedStopwords, sWarning );
}

//////////////////////////////////////////////////////////////////////////

bool sphCalcFileCRC32 ( const char * szFilename, DWORD & uCRC32 )
{
	uCRC32 = 0;

	if ( !szFilename )
		return false;

	FILE * pFile = fopen ( szFilename, "rb" );
	if ( !pFile )
		return false;

	const int BUFFER_SIZE = 131072;
	CSphFixedVector<BYTE> dBuffer (BUFFER_SIZE);

	int iBytesRead;
	while ( ( iBytesRead = (int)fread ( dBuffer.begin(), 1, BUFFER_SIZE, pFile ) ) != 0 )
		uCRC32 = sphCRC32 ( dBuffer.begin(), iBytesRead, uCRC32 );

	fclose ( pFile );
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

bool sphFileExists ( const CSphString& sFilename, CSphString* pError )
{
	return sphFileExists ( sFilename.cstr(), pError );
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

bool sphDirExists ( const CSphString& sFilename, CSphString* pError )
{
	return sphDirExists ( sFilename.cstr(), pError );
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
#if _WIN32
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


CSphString sphNormalizePath( const CSphString & sOrigPath )
{
	CSphVector<Str_t> dChunks;
	const char * szBegin = sOrigPath.scstr();
	const char * szEnd = szBegin + sOrigPath.Length();
	const char * szPath = szBegin;
	int iLevel = 0;

	while ( szPath<szEnd )
	{
		const char * szSlash = szEnd;
		for ( const char * p = szPath; p < szEnd; p++ )
			if ( IsSlash(*p) )
			{
				szSlash = p;
				break;
			}

		auto iChunkLen = szSlash - szPath;

		switch ( iChunkLen )
		{
		case 0: // empty chunk skipped
			++szPath;
			continue;
		case 1: // simple dot chunk skipped
			if ( *szPath=='.' )
			{
				szPath += 2;
				continue;
			}
			break;
		case 2: // double dot abandons chunks, then decrease level
			if ( szPath[0]=='.' && szPath[1]=='.' )
			{
				if ( dChunks.IsEmpty())
					--iLevel;
				else
					dChunks.Pop();
				szPath += 3;
				continue;
			}
			break;
		default:
			break;
		}

		dChunks.Add( { szPath, iChunkLen } );
		szPath = szSlash + 1;
	}

	StringBuilder_c sResult( "/" );
	if ( *szBegin=='/' )
		sResult.AppendRawChunk ( {"/", 1} );
	else
		while ( iLevel++<0 )
			sResult << "..";

	for ( const auto & dChunk : dChunks )
		sResult.AppendChunk(dChunk);

	return sResult.cstr();
}


CSphString sphGetCwd()
{
	CSphFixedVector<char> sBuf ( 65536 );
	return getcwd ( sBuf.begin(), sBuf.GetLength() );
}


int64_t sphRead ( int iFD, void * pBuf, size_t iCount )
{
	CSphIOStats * pIOStats = GetIOStats();
	int64_t tmStart = 0;
	if ( pIOStats )
		tmStart = sphMicroTimer();

	int64_t iRead = ::read ( iFD, pBuf, (int) iCount );

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
	return ( iSize==(size_t)::write ( iFD, pBuf, (int) iSize ) );
}

bool sphWrite ( int iFD, const Str_t & dBuf )
{
	return ( dBuf.second==(int) ::write ( iFD, dBuf.first, (int) dBuf.second) );
}


#if _WIN32
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

#if _WIN32
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

bool wrap_mkdir ( const char* szPath)
{
	return mkdir ( szPath
#if !_WIN32
	, S_IRWXU
#endif
		);
}


bool MkDir ( const char * szDir )
{
	if ( !szDir || !*szDir )
		return false;

	CSphString sDir = szDir;
	while ( sDir.Length()>1 && IsSlash ( sDir.cstr()[sDir.Length()-1] ) )
	{
#if _WIN32
		if ( ( sDir.Length()==3 && sDir.cstr()[1]==':' ) || ( sDir.Length()==2 && IsSlash ( sDir.cstr()[0] ) ) )
			break;
#endif
		sDir = sDir.SubString ( 0, sDir.Length()-1 );
	}

	if ( sphDirExists ( sDir.cstr() ) )
		return true;

	if ( !wrap_mkdir ( sDir.cstr() ) )
		return true;

	if ( errno==EEXIST )
		return sphDirExists ( sDir.cstr() );

	if ( errno!=ENOENT )
		return false;

	CSphString sParent = GetPathOnly ( sDir );
	if ( sParent==sDir || !MkDir ( sParent.cstr() ) )
		return false;

	return !wrap_mkdir ( sDir.cstr() ) || ( errno==EEXIST && sphDirExists ( sDir.cstr() ) );
}

bool CopyFile ( const CSphString & sSource, const CSphString & sDest, CSphString & sError, int iMode )
{
	const int BUFFER_SIZE = 1048576;
	CSphFixedVector<BYTE> dBuffer(BUFFER_SIZE);

	CSphAutofile tSource;
	int iSrcFD = tSource.Open ( sSource, SPH_O_READ, sError );
	if ( iSrcFD<0 )
		return false;

	CSphAutofile tDest;
	int iDstFD = tDest.Open ( sDest, iMode, sError );
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
		sError.SetSprintf ( "Unable to copy file '%s' to '%s': %s", sSource.cstr(), sDest.cstr(), strerrorm(errno) );
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
#if _WIN32
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


bool IsPathAbsolute ( const CSphString & sPath )
{
	if ( !sPath.Length() )
		return false;

#if _WIN32
	return !PathIsRelative ( sPath.cstr() );
#else
	return sPath.cstr() && IsSlash ( sPath.cstr()[0] );
#endif
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
		return {};

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


CSphString GetPathNoExtension ( const CSphString & sFullPath )
{
    if ( sFullPath.IsEmpty() )
		return "";

    const char * pDot = strchr ( sFullPath.cstr(), '.' );
    if ( !pDot || pDot==sFullPath.cstr() )
        return sFullPath.cstr();

    return sFullPath.SubString ( 0, pDot-sFullPath.cstr() );
}


CSphString RealPath ( const CSphString& sPath )
{
#if _WIN32
	char szFullPath[_MAX_PATH];
	if ( _fullpath( szFullPath, sPath.cstr(), _MAX_PATH ) )
	{
		char * pStart = szFullPath;
		while ( *pStart )
		{
			if ( *pStart=='\\' )
				*pStart = '/';

			pStart++;
		}

		return szFullPath;
	}
#else
	char szPath[PATH_MAX];
	auto szResult = realpath ( sPath.cstr(), szPath );
	if ( szResult )
		return szResult;
#endif

	return sPath;
}


std::pair<bool,bool> IsSymlink ( const CSphString & sFile )
{
#if _WIN32
	DWORD uAttrs = GetFileAttributes ( sFile.cstr() );
	if ( uAttrs==INVALID_FILE_ATTRIBUTES )
		return { false, false };

	if ( !( uAttrs & FILE_ATTRIBUTE_REPARSE_POINT ) )
		return { false, false };

	WIN32_FIND_DATA tFindData;
	HANDLE hFind = FindFirstFile ( sFile.cstr(), &tFindData );
	if ( hFind==INVALID_HANDLE_VALUE )
		return { false, false };

	bool bSymlink = tFindData.dwReserved0==IO_REPARSE_TAG_SYMLINK;
	FindClose(hFind);

	return { true, bSymlink };
#else
	struct_stat tStat = {0};

	if ( lstat ( sFile.cstr(), &tStat ) )
		return { false, false };	// not found

	return { true, S_ISLNK(tStat.st_mode) };
#endif
}


bool ResolveSymlink ( const CSphString & sFile, CSphString & sResult )
{
	sResult = sFile;

#if _WIN32
	HANDLE hFile = CreateFile ( sFile.cstr(), 0, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr );
	if ( hFile==INVALID_HANDLE_VALUE )
		return false;

	CHAR szTargetPath[MAX_PATH];
	DWORD uBytesRead = GetFinalPathNameByHandle ( hFile, szTargetPath, MAX_PATH, FILE_NAME_NORMALIZED );
	if ( uBytesRead )
	{
		sResult.SetBinary ( szTargetPath, uBytesRead );
		if ( sResult.Begins ( R"(\\?\)" ) )
			sResult = sResult.SubString ( 4, sResult.Length()-4 );

		return true;
	}

	CloseHandle(hFile);
	return false;
#else
	char szPath[PATH_MAX];
	ssize_t tLen = ::readlink ( sFile.cstr(), szPath, sizeof(szPath)-1 );
	if ( tLen!=-1 )
	{
		sResult = CSphString ( szPath, tLen );
		return true;
	}

	return false;
#endif
}


CSphString GetExecutablePath()
{
#if _WIN32
	HMODULE hModule = GetModuleHandle(NULL);
	CHAR szPath[MAX_PATH];
	GetModuleFileName ( hModule, szPath, MAX_PATH );
	return szPath;
#else
	char szPath[PATH_MAX];
	ssize_t tLen;

	tLen = ::readlink ( "/proc/self/exe", szPath, sizeof(szPath)-1 );
	if ( tLen!=-1 )
		return CSphString ( szPath, tLen );

	tLen = ::readlink ( "/proc/curproc/file", szPath, sizeof(szPath)-1 );
	if ( tLen!=-1 )
		return CSphString ( szPath, tLen );

	tLen = ::readlink ( "/proc/self/path/a.out", szPath, sizeof(szPath)-1 );
	if ( tLen!=-1 )
		return CSphString ( szPath, tLen );

	return "";
#endif
}


void SeekAndPutOffset ( CSphWriter & tWriter, SphOffset_t tOffset, SphOffset_t tValue )
{
	SphOffset_t tTotalSize = tWriter.GetPos();

	// order matters here
	tWriter.Flush(); // store collected data as SeekTo may get rid of buffer collected so far
	tWriter.SeekTo(tOffset); 
	tWriter.PutOffset(tValue);
	tWriter.SeekTo(tTotalSize);
}

//////////////////////////////////////////////////////////////////////////

SharedMemory_c::SharedMemory_c ( const CSphString & sPath )
	: m_sPath ( sPath )
{
}

SharedMemory_c::~SharedMemory_c()
{
	if ( m_sPath.IsEmpty() )
		return;

#ifndef _WIN32
	if ( !Close() )
		sphFatalLog ( "%s", m_sError.cstr() );
#endif
}

SharedMemory_c::OpenResult_e SharedMemory_c::Open()
{
	if ( m_sPath.IsEmpty() )
	{
		m_sError.SetSprintf ( "empty path to shared memory" );
		return SharedMemory_c::OpenResult_e::FAILURE;
	}

	if ( !Close() )
		return SharedMemory_c::OpenResult_e::FAILURE;

	int iFD = -1;
	int64_t iBytes = 0;
#ifndef _WIN32

	iFD = ::shm_open ( m_sPath.cstr(), O_RDWR | O_EXCL, 0644 );
	if ( iFD==-1 )
	{
		m_sError.SetSprintf ( "failed to open shared memory %s: %s" , m_sPath.cstr(), strerror(errno) );
		return SharedMemory_c::OpenResult_e::NO_FILE;
	}
	m_bHasFile = true;

	iBytes = sphGetFileSize ( iFD, &m_sError );
	if ( iBytes==-1 )
		return SharedMemory_c::OpenResult_e::FAILURE;

#endif
	bool bOk = MapFile ( iFD, iBytes );
	return ( bOk ? SharedMemory_c::OpenResult_e::OK : SharedMemory_c::OpenResult_e::FAILURE );
}

bool SharedMemory_c::Create ( int64_t iBytes )
{
	if ( !Close () )
		return false;

	int iFD = -1;
#ifndef _WIN32

	iFD = ::shm_open ( m_sPath.cstr(), O_CREAT | O_RDWR | O_EXCL, 0644 );
	if ( iFD==-1 )
	{
		m_sError.SetSprintf ( "failed to open shared memory %s: %s" , m_sPath.cstr(), strerror(errno) );
		return false;
	}
	m_bHasFile = true;

	if ( ::ftruncate ( iFD, iBytes)==-1 )
	{
		m_sError.SetSprintf ( "failed to set size " INT64_FMT " for shared memory %s: %s" , iBytes, m_sPath.cstr(), strerror(errno) );
		SafeClose ( iFD );
		Close ();
		return false;
	}
#endif
	return MapFile ( iFD, iBytes );
}

bool SharedMemory_c::Reset ( int64_t iBytes )
{
	if ( !Close() )
		return false;

	return Create ( iBytes );
}

bool SharedMemory_c::Close ()
{
	if ( !IsValid() )
		return false;

#ifndef _WIN32
	if ( m_pData )
		::munmap ( m_pData, m_iCount );

	m_pData = nullptr;
	m_iCount = 0;
	bool bCloseFile = m_bHasFile;
	m_bHasFile = false;

	if ( bCloseFile && ::shm_unlink ( m_sPath.cstr() )==-1 )
	{
		m_sError.SetSprintf ( "failed to unlink shared memory %s: %s" , m_sPath.cstr(), strerror(errno) );
		return false;
	}
#endif
	return true;
}

bool SharedMemory_c::MapFile ( int iFD, int64_t iBytes )
{
#ifndef _WIN32
	BYTE * pData = (BYTE *)::mmap ( nullptr, iBytes, PROT_WRITE | PROT_READ, MAP_SHARED, iFD, 0 );
	if ( pData==MAP_FAILED )
	{
		m_sError.SetSprintf ( "failed to mmap size " INT64_FMT " for shared memory %s: %s" , iBytes, m_sPath.cstr(), strerror(errno) );
		SafeClose ( iFD );
		Close();
		return false;
	}

	SafeClose ( iFD );
	mmadvise ( pData, iBytes, Advise_e::NOFORK );
	mmadvise ( pData, iBytes, Advise_e::NODUMP );

	m_pData = pData;
	m_iCount = iBytes;
#endif
	return true;
}

bool SharedMemory_c::IsValid()
{
#ifdef _WIN32
	m_sError = "shared memory unsupported";
	return false;
#else
	if ( m_sPath.IsEmpty() )
	{
		m_sError = "empty path to shared memory";
		return false;
	}

	return true;
#endif
}

bool SharedMemory_c::IsSupported () const
{
#ifdef _WIN32
	return false;
#else
	return true;
#endif
}
