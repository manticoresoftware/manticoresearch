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

#ifndef _fileutils_
#define _fileutils_

#include "sphinxstd.h"
#include "std/strerrorm.h"
#include <fcntl.h>

#include <sys/stat.h>

#if !_WIN32
 #include <sys/mman.h>
#endif

#if _WIN32
	#include <direct.h>

	#define stat		_stat64
	#define fstat		_fstat64
	#if _MSC_VER<1400
		#define struct_stat	__stat64
	#else
		#define struct_stat	struct _stat64
	#endif
#else
	#define struct_stat        struct stat
#endif

#ifdef O_BINARY
	#define SPH_O_BINARY O_BINARY
#else
	#define SPH_O_BINARY 0
#endif

#define SPH_O_READ		( O_RDONLY | SPH_O_BINARY )
#define SPH_O_NEW		( O_CREAT | O_RDWR | O_TRUNC | SPH_O_BINARY )
#define SPH_O_APPEND	( O_CREAT | O_RDWR | O_APPEND | SPH_O_BINARY )

class CSphIOStats
{
public:
	int64_t		m_iReadTime = 0;
	DWORD		m_iReadOps = 0;
	int64_t		m_iReadBytes = 0;
	int64_t		m_iWriteTime = 0;
	DWORD		m_iWriteOps = 0;
	int64_t		m_iWriteBytes = 0;

				~CSphIOStats();

	void		Start();
	void		Stop();

	void		Add ( const CSphIOStats & b );
	bool		IsEnabled() { return m_bEnabled; }

private:
	bool		m_bEnabled = false;
	CSphIOStats * m_pPrev = nullptr;
};


class CSphReader;
namespace bson { class Bson_c; }

struct CSphSavedFile
{
	CSphString		m_sFilename;
	SphOffset_t		m_uSize = 0;
	SphOffset_t		m_uCTime = 0;
	SphOffset_t		m_uMTime = 0;
	DWORD			m_uCRC32 = 0;

	bool			Collect ( const char * szFilename, CSphString * pError=nullptr );
	void			Read ( CSphReader & tReader, const char * szFilename, bool bSharedStopwords, CSphString * sWarning = nullptr );
	void			Read ( const bson::Bson_c& tNode, const char* szFilename, bool bSharedStopwords, CSphString* sWarning = nullptr );
};


/// open file for reading
int				sphOpenFile ( const char * sFile, CSphString & sError, bool bWrite );

/// check if file exists and is a readable file
bool			sphIsReadable ( const char * szFilename, CSphString * pError=NULL );
bool			sphIsReadable ( const CSphString & sFilename, CSphString * pError = NULL );

bool			sphFileExists ( const char * szFilename, CSphString * pError=nullptr );
bool			sphFileExists ( const CSphString& sFilename, CSphString * pError=nullptr );

bool			sphDirExists ( const char * szFilename, CSphString * pError=nullptr );
bool			sphDirExists ( const CSphString& sFilename, CSphString * pError=nullptr );

/// return size of file descriptor
int64_t			sphGetFileSize ( int iFD, CSphString * sError = nullptr );
int64_t			sphGetFileSize ( const CSphString & sFile, CSphString * sError = nullptr );

/// truncate file
bool			sphTruncate ( int iFD );

/// initialize IO statistics collecting
void			sphInitIOStats();

/// clean up IO statistics collector
void			sphDoneIOStats();

CSphIOStats *	GetIOStats();

/// calculate file crc32
bool			sphCalcFileCRC32 ( const char * szFilename, DWORD & uCRC32 );

// unwind different tricks like "../../../etc/passwd"
CSphString		sphNormalizePath ( const CSphString& sOrigPath );
CSphString		sphGetCwd();

// a tiny wrapper over ::read() which additionally performs IO stats update
int64_t			sphRead ( int iFD, void * pBuf, size_t iCount );

/// try to obtain an exclusive lock on specified file
/// bWait specifies whether to wait
bool			sphLockEx ( int iFile, bool bWait );

/// remove existing locks
void			sphLockUn ( int iFile );

/// create and lock the file
bool			RawFileLock ( const CSphString& sFile, int& iLockFD, CSphString& sError );

/// unlock and unlink file
void			RawFileUnLock ( const CSphString& sFile, int& iLockFD );

void			SafeClose ( int& iFD );

/// simple write wrapper
/// simplifies partial write checks, and also supresses "fortified" glibc warnings
bool			sphWrite ( int iFD, const void * pBuf, size_t iSize );
bool			sphWrite ( int iFD, const Str_t& dBuf );

StrVec_t		FindFiles ( const char * szPath, bool bNeedDirs=false );
bool			MkDir ( const char * szDir );
bool			CopyFile ( const CSphString & sSource, const CSphString & sDest, CSphString & sError, int iMode=SPH_O_NEW );
bool			RenameFiles ( const StrVec_t & dSrc, const StrVec_t & dDst, CSphString & sError );
bool			RenameWithRollback ( const StrVec_t & dSrc, const StrVec_t & dDst, CSphString & sError );

// check if path exists and also check if daemon can write there
bool			CheckPath ( const CSphString & sPath, bool bCheckWrite, CSphString & sError, const char * sCheckFileName="tmp" );

bool			IsPathAbsolute ( const CSphString & sPath );
CSphString		GetExecutablePath();

CSphString &	StripPath ( CSphString & sPath );
CSphString		GetPathOnly ( const CSphString & sFullPath );
const char *	GetExtension ( const CSphString & sFullPath );

CSphString		RealPath ( const CSphString& sPath );
bool			IsSymlink ( const CSphString & sFile );
bool			ResolveSymlink ( const CSphString & sFile, CSphString & sResult );

class CSphWriter;
void			SeekAndPutOffset ( CSphWriter & tWriter, SphOffset_t tOffset, SphOffset_t tValue );

// FIXME! unify this weird zoo of file function naming
namespace sph
{
	int rename ( const char * sOld, const char * sNew );
}


template < typename T >
class CSphMappedBuffer : public CSphBufferTrait < T >
{
public:
	/// ctor
	CSphMappedBuffer ()
	{
#if _WIN32
		m_iFD = INVALID_HANDLE_VALUE;
		m_iMap = INVALID_HANDLE_VALUE;
#else
		m_iFD = -1;
#endif
	}

	/// dtor
	~CSphMappedBuffer() override
	{
		this->Reset();
	}

	bool Setup ( const CSphString & sFile, CSphString & sError, bool bWrite = false )
	{
		m_sFilename = sFile;
		m_bWrite = bWrite;

#if _WIN32
		assert ( m_iFD==INVALID_HANDLE_VALUE );
#else
		assert ( m_iFD==-1 );
#endif
		assert ( !this->GetReadPtr() && !this->GetLength64() );

		T * pData = NULL;
		int64_t iCount = 0;

#if _WIN32
		int iAccessMode = GENERIC_READ;
		if ( bWrite )
			iAccessMode |= GENERIC_WRITE;

		DWORD uShare = FILE_SHARE_READ | FILE_SHARE_DELETE;
		if ( bWrite )
			uShare |= FILE_SHARE_WRITE; // wo this flag indexer and indextool unable to open attribute file that was opened by daemon

		HANDLE iFD = CreateFile ( sFile.cstr(), iAccessMode, uShare, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
		if ( iFD==INVALID_HANDLE_VALUE )
		{
			sError.SetSprintf ( "failed to open file '%s' (errno %u)", sFile.cstr(), ::GetLastError() );
			return false;
		}
		m_iFD = iFD;

		LARGE_INTEGER tLen;
		if ( GetFileSizeEx ( iFD, &tLen )==0 )
		{
			sError.SetSprintf ( "failed to fstat file '%s' (errno %u)", sFile.cstr(), ::GetLastError() );
			Reset();
			return false;
		}

		// FIXME!!! report abount tail, ie m_iLen*sizeof(T)!=tLen.QuadPart
		iCount = tLen.QuadPart / sizeof(T);

		// mmap fails to map zero-size file
		if ( tLen.QuadPart>0 )
		{
			int iProtectMode = PAGE_READONLY;
			if ( bWrite )
				iProtectMode = PAGE_READWRITE;
			m_iMap = ::CreateFileMapping ( iFD, NULL, iProtectMode, 0, 0, NULL );
			int iMapAccessMode = FILE_MAP_READ;
			if ( bWrite )
				iMapAccessMode |= FILE_MAP_WRITE;
			pData = (T *)::MapViewOfFile ( m_iMap, iMapAccessMode, 0, 0, 0 );
			if ( !pData )
			{
				sError.SetSprintf ( "failed to map file '%s': (errno %u, length=" INT64_FMT ")", sFile.cstr(), ::GetLastError(), (int64_t)tLen.QuadPart );
				Reset();
				return false;
			}
		}
#else

		int iFD = sphOpenFile ( sFile.cstr(), sError, bWrite );
		if ( iFD<0 )
			return false;
		m_iFD = iFD;

		int64_t iFileSize = sphGetFileSize ( iFD, &sError );
		if ( iFileSize<0 )
			return false;

		// FIXME!!! report about tail, ie m_iLen*sizeof(T)!=st.st_size
		iCount = iFileSize / sizeof(T);

		// mmap fails to map zero-size file
		if ( iFileSize>0 )
		{
			int iProt = PROT_READ;

			if ( bWrite )
				iProt |= PROT_WRITE;

			pData = (T *)mmap ( NULL, iFileSize, iProt, MAP_SHARED, iFD, 0 );
			if ( pData==MAP_FAILED )
			{
				sError.SetSprintf ( "failed to mmap file '%s': %s (length=" INT64_FMT ")", sFile.cstr(), strerrorm(errno), iFileSize );
				Reset();
				return false;
			}

			mmadvise ( pData, iFileSize, Advise_e::NOFORK );
			mmadvise ( pData, iFileSize, Advise_e::NODUMP );
		}
#endif

		this->Set ( pData, iCount );
		return true;
	}

	void Reset() override
	{
		this->MemUnlock();

#if _WIN32
		if ( this->GetReadPtr() )
			::UnmapViewOfFile ( this->GetReadPtr() );

		if ( m_iMap!=INVALID_HANDLE_VALUE )
			::CloseHandle ( m_iMap );
		m_iMap = INVALID_HANDLE_VALUE;

		if ( m_iFD!=INVALID_HANDLE_VALUE )
			::CloseHandle ( m_iFD );
		m_iFD = INVALID_HANDLE_VALUE;
#else
		if ( this->GetReadPtr() )
			::munmap ( this->GetWritePtr(), this->GetLengthBytes() );

		SafeClose ( m_iFD );
#endif

		this->Set ( NULL, 0 );
	}

	bool Resize ( uint64_t uNewSize, CSphString & sWarning, CSphString & sError )
	{
		if ( !this->GetReadPtr() )
			return false;

		bool bMlock = this->m_bMemLocked;
		if ( bMlock )
			this->MemUnlock();

#if _WIN32
		assert ( m_iMap );

		::UnmapViewOfFile ( this->GetReadPtr() );
		::CloseHandle ( m_iMap );

		m_iMap = ::CreateFileMapping ( m_iFD, nullptr, m_bWrite ? PAGE_READWRITE : PAGE_READONLY, (DWORD)( uNewSize >> 32 ), (DWORD) ( uNewSize & 0xFFFFFFFFULL ), nullptr );
		if ( !m_iMap )
		{
			sError.SetSprintf ( "CreateFileMapping failed for '%s': (errno %u, length=" UINT64_FMT ")", m_sFilename.cstr(), ::GetLastError(), uNewSize );
			Reset();
			return false;
		}

		void * pMapped = (T *)::MapViewOfFile ( m_iMap, FILE_MAP_READ | ( m_bWrite ? FILE_MAP_WRITE : 0 ), 0, 0, 0 );
		if ( !pMapped )
		{
			sError.SetSprintf ( "MapViewOfFile failed for '%s': (errno %u, length=" UINT64_FMT ")", m_sFilename.cstr(), ::GetLastError(), uNewSize );
			Reset();
			return false;
		}

#else
		if ( sphSeek ( m_iFD, uNewSize, SEEK_SET ) < 0 )
		{
			sError.SetSprintf ( "failed to seek '%s': %s (length=" UINT64_FMT ")", m_sFilename.cstr(), strerror(errno), uNewSize );
			Reset();
			return false;
		}

		if ( !sphTruncate(m_iFD) )
		{
			sError.SetSprintf ( "failed to truncate '%s': %s (length=" UINT64_FMT ")", m_sFilename.cstr(), strerror(errno), uNewSize );
			Reset();
			return false;
		}

#if HAVE_MREMAP
		void * pMapped = mremap ( this->GetWritePtr(), this->GetLengthBytes(), uNewSize, MREMAP_MAYMOVE );
		if ( pMapped==MAP_FAILED )
		{
			sError.SetSprintf ( "mremap failed for '%s': %s (length=" UINT64_FMT ")", m_sFilename.cstr(), strerror(errno), uNewSize );
			Reset();
			return false;
		}
#else
		void * pMapped = mmap ( nullptr, uNewSize, PROT_READ | ( m_bWrite ? PROT_WRITE : 0 ), MAP_SHARED, m_iFD, 0 );
		if ( pMapped==MAP_FAILED )
		{
			sError.SetSprintf ( "mmap failed for '%s': %s (length=" UINT64_FMT ")", m_sFilename.cstr(), strerror(errno), uNewSize );
			Reset();
			return false;
		}

		if ( this->GetReadPtr() )
			::munmap ( this->GetWritePtr(), this->GetLengthBytes() );
#endif
#endif

		if ( bMlock )
			this->MemLock ( sWarning );

		this->Set ( (T*)pMapped, uNewSize / sizeof(T) );

		return true;
	}

	uint64_t GetCoreSize () const
	{
#if !_WIN32
				using PAGETYPE =
	#if HAVE_UNSIGNED_MINCORE
							BYTE
	#else
							char
	#endif
								;

		auto uPageSize = GetMemPageSize ();
		auto uSize = this->GetLengthBytes();
		auto uPages = ( uSize+uPageSize-1 ) / uPageSize;
		CSphFixedVector<PAGETYPE> dMap ( uPages );
		if ( mincore ( (void *) this->GetWritePtr (), uSize, dMap.begin () )==0 )
		{
			for ( auto uData : dMap )
				if ( !( ((BYTE) uData) & 1U ) )
					--uPages;
		}
		return uPages * uPageSize;
#else
		return 0; // fixme! implement...
#endif
	}

	bool Flush ( bool bWaitComplete, CSphString & sError ) const
	{
		if ( !this->GetReadPtr() )
			return true;

#if _WIN32
		if ( !::FlushViewOfFile ( this->GetReadPtr(), this->GetLengthBytes() ) )
		{
			sError.SetSprintf ( "FlushViewOfFile failed for '%s': errno %u", m_sFilename.cstr(), ::GetLastError() );
			return false;
		}

		if ( bWaitComplete && !::FlushFileBuffers(m_iFD) )
		{
			sError.SetSprintf ( "FlushFileBuffers failed for '%s': errno %u", m_sFilename.cstr(), ::GetLastError() );
			return false;
		}
#else
		if ( ::msync ( this->GetWritePtr(), this->GetLengthBytes(), bWaitComplete ? MS_SYNC : MS_ASYNC ) )
		{
			sError.SetSprintf ( "msync failed for '%s': %s", m_sFilename.cstr(), strerror(errno) );
			return false;
		}
#endif

		return true;
	}

	const char * GetFileName() const { return m_sFilename.cstr(); }

private:
#if _WIN32
	HANDLE		m_iFD;
	HANDLE		m_iMap;
#else
	int			m_iFD;
#endif

	bool		m_bWrite {false};
	CSphString	m_sFilename;
};

#endif // _fileutils_
