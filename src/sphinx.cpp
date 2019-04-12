//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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
#include "sphinxstem.h"
#include "sphinxquery.h"
#include "sphinxutils.h"
#include "sphinxexpr.h"
#include "sphinxfilter.h"
#include "sphinxint.h"
#include "sphinxsearch.h"
#include "searchnode.h"
#include "sphinxjson.h"
#include "sphinxplugin.h"
#include "sphinxqcache.h"
#include "sphinxrlp.h"
#include "attribute.h"
#include "secondaryindex.h"
#include "killlist.h"

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <float.h>

#define SPH_UNPACK_BUFFER_SIZE	4096
#define SPH_READ_PROGRESS_CHUNK (8192*1024)
#define SPH_READ_NOPROGRESS_CHUNK (32768*1024)

#if USE_LIBSTEMMER
#include <libstemmer.h>
#endif

#if USE_LIBEXPAT
#define XMLIMPORT
#include "expat.h"

// workaround for expat versions prior to 1.95.7
#ifndef XMLCALL
#define XMLCALL
#endif
#endif

#if USE_LIBICONV
#include "iconv.h"
#endif

#if USE_ZLIB
#include <zlib.h>
#endif

#if USE_ODBC
#include <sql.h>
#endif

#if USE_RE2
#include <string>
#include <re2/re2.h>
#endif

#if USE_WINDOWS
	// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
	#define snprintf	_snprintf

	#define stat		_stat64
	#define fstat		_fstat64
	#if _MSC_VER<1400
	#define struct_stat	__stat64
	#else
	#define struct_stat	struct _stat64
	#endif

	#define ICONV_INBUF_CONST	1
#else
	#include <unistd.h>
	#include <sys/time.h>

	#define struct_stat		struct stat
#endif

#if ( USE_WINDOWS && !BUILD_WITH_CMAKE ) // on windows with cmake manual linkage is not necessary
#if ( USE_MYSQL )
	#pragma comment(linker, "/defaultlib:libmysql.lib")
	#pragma message("Automatically linking with libmysql.lib")
#endif

#if ( USE_PGSQL )
	#pragma comment(linker, "/defaultlib:libpq.lib")
	#pragma message("Automatically linking with libpq.lib")
#endif

#if ( USE_LIBSTEMMER )
	#pragma comment(linker, "/defaultlib:libstemmer_c.lib")
	#pragma message("Automatically linking with libstemmer_c.lib")
#endif

#if ( USE_LIBEXPAT )
	#pragma comment(linker, "/defaultlib:libexpat.lib")
	#pragma message("Automatically linking with libexpat.lib")
#endif

#if ( USE_LIBICONV )
	#pragma comment(linker, "/defaultlib:iconv.lib")
	#pragma message("Automatically linking with iconv.lib")
#endif

#if ( USE_RE2 )
	#pragma comment(linker, "/defaultlib:re2.lib")
	#pragma message("Automatically linking with re2.lib")
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

// logf() is not there sometimes (eg. Solaris 9)
#if !USE_WINDOWS && !HAVE_LOGF
static inline float logf ( float v )
{
	return (float) log ( v );
}
#endif

#if USE_WINDOWS
void localtime_r ( const time_t * clock, struct tm * res )
{
	*res = *localtime ( clock );
}

void gmtime_r ( const time_t * clock, struct tm * res )
{
	*res = *gmtime ( clock );
}
#endif

// forward decl
void sphWarn ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );

static auto& g_bShutdown = sphGetShutdown();

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *		SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451";

const char *		MAGIC_WORD_SENTENCE		= "\3sentence";		// emitted from source on sentence boundary, stored in dictionary
const char *		MAGIC_WORD_PARAGRAPH	= "\3paragraph";	// emitted from source on paragraph boundary, stored in dictionary

bool				g_bJsonStrict				= false;
bool				g_bJsonAutoconvNumbers		= false;
bool				g_bJsonKeynamesToLowercase	= false;

static const int	DEFAULT_READ_BUFFER		= 262144;
static const int	DEFAULT_READ_UNHINTED	= 32768;
static const int	MIN_READ_BUFFER			= 8192;
static const int	MIN_READ_UNHINTED		= 1024;
#define READ_NO_SIZE_HINT 0

static int 			g_iReadUnhinted 		= DEFAULT_READ_UNHINTED;

#ifndef FULL_SHARE_DIR
#define FULL_SHARE_DIR "."
#endif

CSphString			g_sLemmatizerBase		= FULL_SHARE_DIR;

// quick hack for indexer crash reporting
// one day, these might turn into a callback or something
int64_t		g_iIndexerCurrentDocID		= 0;
int64_t		g_iIndexerCurrentHits		= 0;
int64_t		g_iIndexerCurrentRangeMin	= 0;
int64_t		g_iIndexerCurrentRangeMax	= 0;
int64_t		g_iIndexerPoolStartDocID	= 0;
int64_t		g_iIndexerPoolStartHit		= 0;

/// global IDF
class CSphGlobalIDF
{
public:
	bool			Touch ( const CSphString & sFilename );
	bool			Preread ( const CSphString & sFilename, CSphString & sError );
	DWORD			GetDocs ( const CSphString & sWord ) const;
	float			GetIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF );

protected:
#pragma pack(push,4)
	struct IDFWord_t
	{
		uint64_t				m_uWordID;
		DWORD					m_iDocs;
	};
#pragma pack(pop)
	STATIC_SIZE_ASSERT			( IDFWord_t, 12 );

	static const int			HASH_BITS = 16;
	int64_t						m_iTotalDocuments = 0;
	int64_t						m_iTotalWords = 0;
	SphOffset_t					m_uMTime = 0;
	CSphLargeBuffer<IDFWord_t>	m_pWords;
	CSphLargeBuffer<int64_t>	m_pHash;
};


/// global idf definitions hash
static SmallStringHash_T <CSphGlobalIDF * >	g_hGlobalIDFs;
static CSphMutex							g_tGlobalIDFLock;

struct BuildHeader_t;
struct WriteHeader_t;
static bool IndexBuildDone ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, const CSphString & sFileName, CSphString & sError );

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

STATIC_SIZE_ASSERT ( SphOffset_t, 8 );

/////////////////////////////////////////////////////////////////////////////

// whatever to collect IO stats
static bool g_bCollectIOStats = false;
static SphThreadKey_t g_tIOStatsTls;


bool sphInitIOStats ()
{
	if ( !sphThreadKeyCreate ( &g_tIOStatsTls ) )
		return false;

	g_bCollectIOStats = true;
	return true;
}

void sphDoneIOStats ()
{
	sphThreadKeyDelete ( g_tIOStatsTls );
	g_bCollectIOStats = false;
}

CSphIOStats::~CSphIOStats ()
{
	Stop();
}

void CSphIOStats::Start()
{
	if ( !g_bCollectIOStats )
		return;

	m_pPrev = (CSphIOStats *)sphThreadGet ( g_tIOStatsTls );
	sphThreadSet ( g_tIOStatsTls, this );
	m_bEnabled = true;
}

void CSphIOStats::Stop()
{
	if ( !g_bCollectIOStats )
		return;

	m_bEnabled = false;
	sphThreadSet ( g_tIOStatsTls, m_pPrev );
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


static CSphIOStats * GetIOStats ()
{
	if ( !g_bCollectIOStats )
		return nullptr;

	CSphIOStats * pIOStats = (CSphIOStats *)sphThreadGet ( g_tIOStatsTls );

	if ( !pIOStats || !pIOStats->IsEnabled() )
		return nullptr;
	return pIOStats;
}

// a tiny wrapper over ::read() which additionally performs IO stats update
static int64_t sphRead ( int iFD, void * pBuf, size_t iCount )
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


static bool GetFileStats ( const char * szFilename, CSphSavedFile & tInfo, CSphString * pError );

/////////////////////////////////////////////////////////////////////////////
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////
CSphAutofile::CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp )
{
	Open ( sName, iMode, sError, bTemp );
}


CSphAutofile::~CSphAutofile ()
{
	Close ();
}


int CSphAutofile::Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp )
{
	assert ( m_iFD==-1 && m_sFilename.IsEmpty () );
	assert ( !sName.IsEmpty() );

#if USE_WINDOWS
	if ( iMode==SPH_O_READ )
	{
		intptr_t tFD = (intptr_t)CreateFile ( sName.cstr(), GENERIC_READ , FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		m_iFD = _open_osfhandle ( tFD, 0 );
	} else
		m_iFD = ::open ( sName.cstr(), iMode, 0644 );
#else
	m_iFD = ::open ( sName.cstr(), iMode, 0644 );
#endif
	m_sFilename = sName; // not exactly sure why is this uncoditional. for error reporting later, i suppose

	if ( m_iFD<0 )
		sError.SetSprintf ( "failed to open %s: %s", sName.cstr(), strerrorm(errno) );
	else
	{
		m_bTemporary = bTemp; // only if we managed to actually open it
		m_bWouldTemporary = true; // if a shit happen - we could delete the file.
	}

	return m_iFD;
}


void CSphAutofile::Close ()
{
	if ( m_iFD>=0 )
	{
		::close ( m_iFD );
		if ( m_bTemporary )
			::unlink ( m_sFilename.cstr() );
	}

	m_iFD = -1;
	m_sFilename = "";
	m_bTemporary = false;
	m_bWouldTemporary = false;
}

void CSphAutofile::SetTemporary()
{
	m_bTemporary = m_bWouldTemporary;
}


const char * CSphAutofile::GetFilename () const
{
	assert ( m_sFilename.cstr() );
	return m_sFilename.cstr();
}


SphOffset_t CSphAutofile::GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError )
{
	struct_stat st;
	if ( stat ( GetFilename(), &st )<0 )
	{
		sError.SetSprintf ( "failed to stat %s: %s", GetFilename(), strerrorm(errno) );
		return -1;
	}
	if ( st.st_size<iMinSize )
	{
		sError.SetSprintf ( "failed to load %s: bad size " INT64_FMT " (at least " INT64_FMT " bytes expected)",
			GetFilename(), (int64_t)st.st_size, (int64_t)iMinSize );
		return -1;
	}
	if ( bCheckSizeT )
	{
		size_t uCheck = (size_t)st.st_size;
		if ( st.st_size!=SphOffset_t(uCheck) )
		{
			sError.SetSprintf ( "failed to load %s: bad size " INT64_FMT " (out of size_t; 4 GB limit on 32-bit machine hit?)",
				GetFilename(), (int64_t)st.st_size );
			return -1;
		}
	}
	return st.st_size;
}


SphOffset_t CSphAutofile::GetSize ()
{
	CSphString sTmp;
	return GetSize ( 0, false, sTmp );
}


bool CSphAutofile::Read ( void * pBuf, int64_t iCount, CSphString & sError )
{
	int64_t iToRead = iCount;
	BYTE * pCur = (BYTE *)pBuf;
	while ( iToRead>0 )
	{
		int64_t iToReadOnce = ( m_pStat )
			? Min ( iToRead, SPH_READ_PROGRESS_CHUNK )
			: Min ( iToRead, SPH_READ_NOPROGRESS_CHUNK );
		int64_t iGot = sphRead ( GetFD(), pCur, (size_t)iToReadOnce );

		if ( iGot==-1 )
		{
			// interrupted by a signal - try again
			if ( errno==EINTR )
				continue;

			sError.SetSprintf ( "read error in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
							GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
			return false;
		}

		// EOF
		if ( iGot==0 )
		{
			sError.SetSprintf ( "unexpected EOF in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
							GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
			return false;
		}

		iToRead -= iGot;
		pCur += iGot;

		if ( m_pStat )
		{
			m_pStat->m_iBytes += iGot;
			m_pStat->Show ( false );
		}
	}

	if ( iToRead!=0 )
	{
		sError.SetSprintf ( "read error in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
							GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
		return false;
	}
	return true;
}


void CSphAutofile::SetProgressCallback ( CSphIndexProgress * pStat )
{
	m_pStat = pStat;
}


//////////////////////////////////////////////////////////////////////////

/// possible bin states
enum ESphBinState
{
	BIN_ERR_READ	= -2,	///< bin read error
	BIN_ERR_END		= -1,	///< bin end
	BIN_POS			= 0,	///< bin is in "expects pos delta" state
	BIN_DOC			= 1,	///< bin is in "expects doc delta" state
	BIN_WORD		= 2		///< bin is in "expects word delta" state
};


enum ESphBinRead
{
	BIN_READ_OK,			///< bin read ok
	BIN_READ_EOF,			///< bin end
	BIN_READ_ERROR,			///< bin read error
	BIN_PRECACHE_OK,		///< precache ok
	BIN_PRECACHE_ERROR		///< precache failed
};


/// aggregated hit info
struct CSphAggregateHit
{
	RowID_t			m_tRowID {INVALID_ROWID};	///< document ID
	SphWordID_t		m_uWordID {0};				///< word ID in current dictionary
	const BYTE *	m_sKeyword {nullptr};		///< word itself (in keywords dictionary case only)
	Hitpos_t		m_iWordPos {0};				///< word position in current document, or hit count in case of aggregate hit
	FieldMask_t		m_dFieldMask;				///< mask of fields containing this word, 0 for regular hits, non-0 for aggregate hits

	int GetAggrCount () const
	{
		assert ( !m_dFieldMask.TestAll ( false ) );
		return m_iWordPos;
	}

	void SetAggrCount ( int iVal )
	{
		m_iWordPos = iVal;
	}
};


static const int MAX_KEYWORD_BYTES = SPH_MAX_WORD_LEN*3+4;


/// bin, block input buffer
struct CSphBin
{
	static const int	MIN_SIZE	= 8192;
	static const int	WARN_SIZE	= 262144;

protected:
	ESphHitless			m_eMode;
	int					m_iSize = 0;

	BYTE *				m_dBuffer = nullptr;
	BYTE *				m_pCurrent = nullptr;
	int					m_iLeft = 0;
	int					m_iDone = 0;
	ESphBinState		m_eState { BIN_POS };
	bool				m_bWordDict;
	bool				m_bError = false;	// FIXME? sort of redundant, but states are a mess

	CSphAggregateHit	m_tHit;									///< currently decoded hit
	BYTE				m_sKeyword [ MAX_KEYWORD_BYTES ];	///< currently decoded hit keyword (in keywords dict mode)

#ifndef NDEBUG
	SphWordID_t			m_iLastWordID = 0;
	BYTE				m_sLastKeyword [ MAX_KEYWORD_BYTES ];
#endif

	int					m_iFile = -1;	///< my file
	SphOffset_t *		m_pFilePos = nullptr;	///< shared current offset in file

public:
	SphOffset_t			m_iFilePos = 0;		///< my current offset in file
	int					m_iFileLeft = 0;	///< how much data is still unread from the file

public:
	explicit 			CSphBin ( ESphHitless eMode = SPH_HITLESS_NONE, bool bWordDict = false );
						~CSphBin ();

	static int			CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn = true );
	void				Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize );

	SphWordID_t			ReadVLB ();
	int					ReadByte ();
	ESphBinRead			ReadBytes ( void * pDest, int iBytes );
	int					ReadHit ( CSphAggregateHit * pOut );

	DWORD				UnzipInt ();
	SphOffset_t			UnzipOffset ();

	bool				IsEOF () const;
	bool				IsDone () const;
	bool				IsError () const { return m_bError; }
	ESphBinRead			Precache ();
};

/////////////////////////////////////////////////////////////////////////////

class CSphIndex_VLN;

// Reader from file or filemap
class IFileBlockReader : public ISphRefcountedMT
{
public:
	virtual SphOffset_t GetPos () const { return 0; }
	virtual void SeekTo ( SphOffset_t iPos, int iSizeHint ) {};
	virtual DWORD UnzipInt () { return 0; };
	virtual uint64_t UnzipOffset () { return 0; };
	virtual void Reset () {};
	RowID_t UnzipRowid () { return UnzipInt (); };
	SphWordID_t UnzipWordid () { return UnzipOffset (); };
};

using IFileBlockReaderPtr_t = CSphRefcountedPtr<IFileBlockReader>;

// imitate CSphReader but fully in memory (intended to use with mmap)
class ThinMMapReader_c : public IFileBlockReader
{
	friend class MMapFabric_c;
	const BYTE* m_pBase = nullptr;
	const BYTE* m_pPointer = nullptr;
	SphOffset_t m_iSize = 0;

	ThinMMapReader_c ( const BYTE* pArena, SphOffset_t iSize )
	{
		m_pPointer = m_pBase = pArena;
		m_iSize = iSize;
	}
protected:
	~ThinMMapReader_c() final {}

public:

	SphOffset_t GetPos () const final
	{
		if ( !m_pPointer )
			return 0;
		assert ( m_pBase );
		return m_pPointer - m_pBase;
	}

	void SeekTo ( SphOffset_t iPos, int /*iSizeHint*/ ) final
	{ m_pPointer = m_pBase + iPos; }

	DWORD UnzipInt () final;
	uint64_t UnzipOffset () final;

	void Reset () final
	{
		m_pPointer = m_pBase;
	}
};

class DirectFileReader_c: public IFileBlockReader, protected FileReader_c
{
	friend class DirectFabric_c;
protected:
	~DirectFileReader_c() final {}
	explicit DirectFileReader_c ( BYTE* pBuf, int iSize )
		: FileReader_c ( pBuf, iSize )
	{}

public:

	SphOffset_t GetPos () const final
	{
		return FileReader_c::GetPos();
	}

	void SeekTo ( SphOffset_t iPos, int iSizeHint ) final
	{
		FileReader_c::SeekTo (iPos, iSizeHint);
	}

	DWORD UnzipInt () final
	{
		return FileReader_c::UnzipInt();
	}

	uint64_t UnzipOffset () final
	{
		return FileReader_c::UnzipOffset ();
	}

	void Reset () final
	{
		FileReader_c::Reset ();
	}
};

// producer of readers from file or filemap
class DataReaderFabric_c: public ISphRefcountedMT
{
	bool m_bValid = false;

protected:
	~DataReaderFabric_c () override	{}
	void SetValid ( bool bValid ) { m_bValid = bValid; }

public:
	enum EKind
	{
		eDocs = 0, eHits
	};

	bool IsValid () const { return m_bValid; }

	virtual SphOffset_t GetFilesize () const = 0;
	virtual SphOffset_t GetPos () const = 0;
	virtual void SeekTo ( SphOffset_t ) = 0;
	virtual IFileBlockReader* MakeReader ( BYTE* pBuf, int iSize ) = 0;
	virtual void SetProfile ( CSphQueryProfile* ) {};
};

using DataReaderFabricPtr_t = CSphRefcountedPtr<DataReaderFabric_c>;

inline static ESphQueryState StateByKind ( DataReaderFabric_c::EKind eKind )
{
	switch ( eKind )
	{
		case DataReaderFabric_c::eDocs: return SPH_QSTATE_READ_DOCS;
		case DataReaderFabric_c::eHits: return SPH_QSTATE_READ_HITS;
		default: return SPH_QSTATE_IO;
	}
}

// producer of readers which access by Seek + Read
class DirectFabric_c: public DataReaderFabric_c
{
	CSphAutoreader m_dReader;
	ESphQueryState m_eWorkState;
	SphOffset_t m_iPos = 0;
	int m_iReadBuffer = 0;
	int m_iReadUnhinted = 0;

protected:
	~DirectFabric_c() final {} // d-tr only by Release

public:
	DirectFabric_c ( const CSphString& sFile, CSphString& sError,
		ESphQueryState eState, int iReadBuffer, int iReadUnhinted )
		: m_eWorkState ( eState )
		, m_iReadBuffer ( iReadBuffer )
		, m_iReadUnhinted ( iReadUnhinted )
	{
		SetValid ( m_dReader.Open ( sFile, sError ) );
	}

	SphOffset_t GetFilesize () const final
	{
		return m_dReader.GetFilesize();
	}

	SphOffset_t GetPos () const final
	{
		return m_iPos;
	}

	void SeekTo ( SphOffset_t iPos ) final
	{
		m_iPos = iPos;
	}

	// returns depended reader sharing same FD as maker
	IFileBlockReader* MakeReader ( BYTE* pBuf, int iSize ) final
	{
		auto pFileReader = new DirectFileReader_c ( pBuf, iSize );
		pFileReader->SetFile ( m_dReader.GetFD(), m_dReader.GetFilename().cstr() );
		pFileReader->SetBuffers ( m_iReadBuffer, m_iReadUnhinted );
		if ( m_iPos )
			pFileReader->SeekTo ( m_iPos, READ_NO_SIZE_HINT );
		pFileReader->m_pProfile = m_dReader.m_pProfile;
		pFileReader->m_eProfileState = m_eWorkState;
		return pFileReader;
	}

	void SetProfile ( CSphQueryProfile* pProfile ) final
	{
		m_dReader.m_pProfile = pProfile;
	}
};

// producer of readers which access by MMap
class MMapFabric_c: public DataReaderFabric_c
{
	CSphMappedBuffer<BYTE> m_tBackendFile;
	SphOffset_t m_iPos = 0;

protected:
	~MMapFabric_c () final {} // d-tr only by Release

public:
	MMapFabric_c ( const CSphString& sFile, CSphString& sError )
	{
		SetValid ( m_tBackendFile.Setup ( sFile, sError ) );
	}

	SphOffset_t GetFilesize () const final
	{
		return m_tBackendFile.GetLength64 ();
	}

	SphOffset_t GetPos () const final
	{
		return m_iPos;
	}

	void SeekTo ( SphOffset_t iPos ) final
	{
		m_iPos = iPos;
	}

	// returns depended reader sharing same mmap as maker
	IFileBlockReader* MakeReader ( BYTE*, int ) final
	{
		auto pReader = new ThinMMapReader_c ( m_tBackendFile.GetWritePtr (),
											  m_tBackendFile.GetLength64 () );
		if ( m_iPos )
			pReader->SeekTo ( m_iPos, 0 );
		return pReader;
	}
};

static DataReaderFabric_c* NewProxyReader ( const CSphString& sFile, CSphString& sError,
	DataReaderFabric_c::EKind eKind, int iReadBuffer, bool bOnDisk )
{
	auto eState = StateByKind ( eKind );
	DataReaderFabric_c* pReader = nullptr;

	if ( bOnDisk )
		pReader = new DirectFabric_c ( sFile, sError, eState, iReadBuffer, g_iReadUnhinted );
	else
		pReader = new MMapFabric_c ( sFile, sError );

	if ( !pReader->IsValid ())
		SafeRelease ( pReader )
	return pReader;
}


/// everything required to setup search term
class DiskIndexQwordSetup_c: public ISphQwordSetup
{
public:
	DataReaderFabricPtr_t m_pDoclist;
	DataReaderFabricPtr_t m_pHitlist;
	bool m_bSetupReaders = false;
	const BYTE* m_pSkips;
	int m_iSkiplistBlockSize = 0;

public:
	DiskIndexQwordSetup_c ( DataReaderFabric_c* pDoclist, DataReaderFabric_c* pHitlist, const BYTE* pSkips,
		int iSkiplistBlockSize )
		: m_pDoclist ( pDoclist ), m_pHitlist ( pHitlist ), m_pSkips ( pSkips ),
		m_iSkiplistBlockSize ( iSkiplistBlockSize )
	{
		SafeAddRef( pDoclist )
		SafeAddRef( pHitlist )
	}

	ISphQword* QwordSpawn ( const XQKeyword_t& tWord ) const final;
	bool QwordSetup ( ISphQword* ) const final;

	bool Setup ( ISphQword* ) const;
};

/// query word from the searcher's point of view
class DiskIndexQwordTraits_c : public ISphQword
{
	static const int	MINIBUFFER_LEN = 1024;

public:
	/// tricky bit
	/// m_uHitPosition is always a current position in the .spp file
	/// base ISphQword::m_iHitlistPos carries the inlined hit data when m_iDocs==1
	/// but this one is always a real position, used for delta coding
	SphOffset_t		m_uHitPosition = 0;
	Hitpos_t		m_uInlinedHit {0};
	DWORD			m_uHitState = 0;

	CSphMatch		m_tDoc;			///< current match (partial)
	Hitpos_t		m_iHitPos {EMPTY_HIT};	///< current hit postition, from hitlist

	BYTE			m_dHitlistBuf [ MINIBUFFER_LEN ];
	BYTE			m_dDoclistBuf [ MINIBUFFER_LEN ];

	BYTE*			m_pHitsBuf = nullptr;
	BYTE*			m_pDocsBuf = nullptr;

	IFileBlockReaderPtr_t		m_rdDoclist;	///< my doclist accessor
	IFileBlockReaderPtr_t		m_rdHitlist;	///< my hitlist accessor

#ifndef NDEBUG
	bool			m_bHitlistOver = true;
#endif

public:
	explicit DiskIndexQwordTraits_c ( bool bUseMini, bool bExcluded )
	{
		m_bExcluded = bExcluded;
		if ( bUseMini ) {
			m_pDocsBuf = m_dDoclistBuf;
			m_pHitsBuf = m_dHitlistBuf;
			//m_rdDoclist.SetMiniBuf ( m_dDoclistBuf, MINIBUFFER_LEN );
		}
	}

	void SetDocReader ( DataReaderFabric_c* pReader )
	{
		if ( !pReader )
			return;
		m_rdDoclist = pReader->MakeReader ( m_pDocsBuf, MINIBUFFER_LEN );
	}

	void SetHitReader ( DataReaderFabric_c* pReader )
	{
		if ( !pReader )
			return;
		m_rdHitlist = pReader->MakeReader ( m_pHitsBuf, MINIBUFFER_LEN );
	}

	void ResetDecoderState ()
	{
		ISphQword::Reset();
		m_uHitPosition = 0;
		m_uInlinedHit = 0;
		m_uHitState = 0;
		m_tDoc.m_tRowID = INVALID_ROWID;
		m_iHitPos = EMPTY_HIT;
	}

	virtual bool Setup ( const DiskIndexQwordSetup_c * pSetup ) = 0;
};


bool operator < ( const SkiplistEntry_t & a, RowID_t b )	{ return a.m_tBaseRowIDPlus1<b; }
bool operator == ( const SkiplistEntry_t & a, RowID_t b )	{ return a.m_tBaseRowIDPlus1==b; }
bool operator < ( RowID_t a, const SkiplistEntry_t & b )	{ return a<b.m_tBaseRowIDPlus1; }


/// query word from the searcher's point of view
template < bool INLINE_HITS, bool DISABLE_HITLIST_SEEK >
class DiskIndexQword_c : public DiskIndexQwordTraits_c
{
public:
	DiskIndexQword_c ( bool bUseMinibuffer, bool bExcluded )
		: DiskIndexQwordTraits_c ( bUseMinibuffer, bExcluded )
	{}

	void Reset () final
	{
		if ( m_rdDoclist )
			m_rdDoclist->Reset ();
		if ( m_rdHitlist )
			m_rdHitlist->Reset ();
		ResetDecoderState();
	}

	void GetHitlistEntry ()
	{
		assert ( !m_bHitlistOver );
		DWORD iDelta = m_rdHitlist->UnzipInt ();
		if ( iDelta )
		{
			m_iHitPos += iDelta;
		} else
		{
			m_iHitPos = EMPTY_HIT;
#ifndef NDEBUG
			m_bHitlistOver = true;
#endif
		}
	}


	RowID_t AdvanceTo ( RowID_t tRowID ) final
	{
		HintRowID ( tRowID );

		do
		{
			ReadNext();
		}
		while ( m_tDoc.m_tRowID < tRowID );
		
		return m_tDoc.m_tRowID;
	}


	void HintRowID ( RowID_t tRowID ) final
	{
		// tricky bit
		// FindSpan() will match a block where tBaseRowIDPlus1[i] <= tRowID < tBaseRowIDPlus1[i+1]
		// meaning that the subsequent ids decoded will be strictly > RefValue
		// meaning that if previous (!) blocks end with tRowID exactly,
		// and we use tRowID itself as RefValue, that document gets lost!

		// first check if we're still inside the last block
		if ( m_iSkipListBlock==-1 )
		{
			m_iSkipListBlock = FindSpan ( m_dSkiplist, tRowID );
			if ( m_iSkipListBlock<0 )
				return;
		}
		else
		{
			if ( m_iSkipListBlock<m_dSkiplist.GetLength()-1 )
			{
				int iNextBlock = m_iSkipListBlock+1;
				RowID_t tNextBlockRowID = m_dSkiplist[iNextBlock].m_tBaseRowIDPlus1;
				if ( tRowID>=tNextBlockRowID )
				{
					auto dSkips = VecTraits_T<SkiplistEntry_t> ( &m_dSkiplist[iNextBlock], m_dSkiplist.GetLength()-iNextBlock );
					m_iSkipListBlock = FindSpan ( dSkips, tRowID );
					if ( m_iSkipListBlock<0 )
						return;

					m_iSkipListBlock += iNextBlock;
				}
			}
			else // we're already at our last block, no need to search
				return;
		}

		const SkiplistEntry_t & t = m_dSkiplist[m_iSkipListBlock];
		if ( t.m_iOffset<=m_rdDoclist->GetPos() )
			return;

		m_rdDoclist->SeekTo ( t.m_iOffset, -1 );
		m_tDoc.m_tRowID = t.m_tBaseRowIDPlus1-1;
		m_uHitPosition = m_iHitlistPos = t.m_iBaseHitlistPos;
	}

	const CSphMatch & GetNextDoc() override
	{
		ReadNext();
		return m_tDoc;
	}

	void SeekHitlist ( SphOffset_t uOff ) final
	{
		if ( uOff >> 63 )
		{
			m_uHitState = 1;
			m_uInlinedHit = (DWORD)uOff; // truncate high dword
		} else
		{
			m_uHitState = 0;
			m_iHitPos = EMPTY_HIT;
			if_const ( DISABLE_HITLIST_SEEK )
				assert ( m_rdHitlist->GetPos()==uOff ); // make sure we're where caller thinks we are.
			else
				m_rdHitlist->SeekTo ( uOff, READ_NO_SIZE_HINT );
		}
#ifndef NDEBUG
		m_bHitlistOver = false;
#endif
	}

	Hitpos_t GetNextHit () final
	{
		assert ( m_bHasHitlist );
		switch ( m_uHitState )
		{
			case 0: // read hit from hitlist
				GetHitlistEntry ();
				return m_iHitPos;

			case 1: // return inlined hit
				m_uHitState = 2;
				return m_uInlinedHit;

			case 2: // return end-of-hitlist marker after inlined hit
				#ifndef NDEBUG
				m_bHitlistOver = true;
				#endif
				m_uHitState = 0;
				return EMPTY_HIT;
		}
		sphDie ( "INTERNAL ERROR: impossible hit emitter state" );
		return EMPTY_HIT;
	}

	bool Setup ( const DiskIndexQwordSetup_c * pSetup ) override
	{
		return pSetup->Setup ( this );
	}

private:
	int m_iSkipListBlock = -1;

	inline void ReadNext()
	{
		RowID_t uDelta = m_rdDoclist->UnzipRowid();
		if ( uDelta )
		{
			m_bAllFieldsKnown = false;
			m_tDoc.m_tRowID += uDelta;

			if_const ( INLINE_HITS )
			{
				m_uMatchHits = m_rdDoclist->UnzipInt();
				const DWORD uFirst = m_rdDoclist->UnzipInt();
				if ( m_uMatchHits==1 && m_bHasHitlist )
				{
					DWORD uField = m_rdDoclist->UnzipInt(); // field and end marker
					m_iHitlistPos = uFirst | ( uField << 23 ) | ( U64C(1)<<63 );
					m_dQwordFields.UnsetAll();
					// want to make sure bad field data not cause crash
					m_dQwordFields.Set ( ( uField >> 1 ) & ( (DWORD)SPH_MAX_FIELDS-1 ) );
					m_bAllFieldsKnown = true;
				} else
				{
					m_dQwordFields.Assign32 ( uFirst );
					m_uHitPosition += m_rdDoclist->UnzipOffset();
					m_iHitlistPos = m_uHitPosition;
				}
			} else
			{
				SphOffset_t iDeltaPos = m_rdDoclist->UnzipOffset();
				assert ( iDeltaPos>=0 );

				m_iHitlistPos += iDeltaPos;

				m_dQwordFields.Assign32 ( m_rdDoclist->UnzipInt() );
				m_uMatchHits = m_rdDoclist->UnzipInt();
			}
		} else
			m_tDoc.m_tRowID = INVALID_ROWID;
	}
};

//////////////////////////////////////////////////////////////////////////////

#define WITH_QWORD(INDEX, NO_SEEK, NAME, ACTION)							\
{																			\
	CSphIndex_VLN * INDEX##pIndex = (CSphIndex_VLN *)INDEX;					\
	if ( INDEX##pIndex->m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )	\
		{ typedef DiskIndexQword_c < true, NO_SEEK > NAME; ACTION; }		\
	else																	\
		{ typedef DiskIndexQword_c < false, NO_SEEK > NAME; ACTION; }		\
}

/////////////////////////////////////////////////////////////////////////////

#define HITLESS_DOC_MASK 0x7FFFFFFF
#define HITLESS_DOC_FLAG 0x80000000


struct Slice64_t
{
	uint64_t	m_uOff;
	int			m_iLen;
};

struct DiskSubstringPayload_t : public ISphSubstringPayload
{
	explicit DiskSubstringPayload_t ( int iDoclists )
		: m_dDoclist ( iDoclists )
	{}
	CSphFixedVector<Slice64_t>	m_dDoclist;
};


template < bool INLINE_HITS >
class DiskPayloadQword_c : public DiskIndexQword_c<INLINE_HITS, false>
{
	typedef DiskIndexQword_c<INLINE_HITS, false> BASE;

public:
	explicit DiskPayloadQword_c ( const DiskSubstringPayload_t * pPayload, bool bExcluded,
		DataReaderFabric_c * pDoclist, DataReaderFabric_c * pHitlist )
		: BASE ( true, bExcluded )
	{
		m_pPayload = pPayload;
		this->m_iDocs = m_pPayload->m_iTotalDocs;
		this->m_iHits = m_pPayload->m_iTotalHits;
		m_iDoclist = 0;

		this->SetDocReader ( pDoclist );
		this->SetHitReader ( pHitlist );
	}

	const CSphMatch & GetNextDoc() final
	{
		const CSphMatch & tMatch = BASE::GetNextDoc();
		assert ( &tMatch==&this->m_tDoc );
		if ( tMatch.m_tRowID==INVALID_ROWID && m_iDoclist<m_pPayload->m_dDoclist.GetLength() )
		{
			BASE::ResetDecoderState();
			SetupReader();
			BASE::GetNextDoc();
			assert ( this->m_tDoc.m_tRowID!=INVALID_ROWID );
		}

		return this->m_tDoc;
	}

	bool Setup ( const DiskIndexQwordSetup_c * ) final
	{
		if ( m_iDoclist>=m_pPayload->m_dDoclist.GetLength() )
			return false;

		SetupReader();
		return true;
	}

private:
	void SetupReader ()
	{
		uint64_t uDocOff = m_pPayload->m_dDoclist[m_iDoclist].m_uOff;
		int iHint = m_pPayload->m_dDoclist[m_iDoclist].m_iLen;
		m_iDoclist++;

		this->m_rdDoclist->SeekTo ( uDocOff, iHint );
	}

	const DiskSubstringPayload_t *	m_pPayload;
	int								m_iDoclist;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

struct CSphWordlistCheckpoint
{
	union
	{
		SphWordID_t		m_uWordID;
		const char *	m_sWord;
	};
	SphOffset_t			m_iWordlistOffset;
};

/////////////////////////////////////////////////////////////////////////////

void ReadFileInfo ( CSphReader & tReader, const char * szFilename, bool bSharedStopwords, CSphSavedFile & tFile, CSphString * sWarning )
{
	tFile.m_uSize = tReader.GetOffset ();
	tFile.m_uCTime = tReader.GetOffset ();
	tFile.m_uMTime = tReader.GetOffset ();
	tFile.m_uCRC32 = tReader.GetDword ();
	tFile.m_sFilename = szFilename;

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
				if ( uMyCRC32!=tFile.m_uCRC32 || tFileInfo.st_size!=tFile.m_uSize
					|| tFileInfo.st_ctime!=tFile.m_uCTime || tFileInfo.st_mtime!=tFile.m_uMTime )
						sWarning->SetSprintf ( "'%s' differs from the original", sName.cstr() );
		}
	}
}


static void WriteFileInfo ( CSphWriter & tWriter, const CSphSavedFile & tInfo )
{
	tWriter.PutOffset ( tInfo.m_uSize );
	tWriter.PutOffset ( tInfo.m_uCTime );
	tWriter.PutOffset ( tInfo.m_uMTime );
	tWriter.PutDword ( tInfo.m_uCRC32 );
}


/// dict=keywords block reader
class KeywordsBlockReader_c : public CSphDictEntry
{
public:
					KeywordsBlockReader_c ( const BYTE * pBuf, int iSkiplistBlockSize );

	void			Reset ( const BYTE * pBuf );
	bool			UnpackWord();

	const char *	GetWord() const			{ return (const char*)m_sWord; }
	int				GetWordLen() const		{ return m_iLen; }

private:
	const BYTE *	m_pBuf;
	BYTE			m_sWord [ MAX_KEYWORD_BYTES ];
	int				m_iLen;
	BYTE			m_uHint = 0;
	int				m_iSkiplistBlockSize = 0;
};


// dictionary header
struct DictHeader_t
{
	int				m_iDictCheckpoints = 0;			///< how many dict checkpoints (keyword blocks) are there
	SphOffset_t		m_iDictCheckpointsOffset = 0;	///< dict checkpoints file position

	int				m_iInfixCodepointBytes = 0;		///< max bytes per infix codepoint (0 means no infixes)
	int64_t			m_iInfixBlocksOffset = 0;		///< infix blocks file position (stored as unsigned 32bit int as keywords dictionary is pretty small)
	int				m_iInfixBlocksWordsSize = 0;	///< infix checkpoints size
};


struct ISphCheckpointReader
{
	virtual ~ISphCheckpointReader() {}
	virtual const BYTE * ReadEntry ( const BYTE * pBuf, CSphWordlistCheckpoint & tCP ) const = 0;
	int m_iSrcStride = 0;
};


// !COMMIT eliminate this, move it to proper dict impls
class CWordlist : public ISphWordlist, public DictHeader_t, public ISphWordlistSuggest
{
public:
	// !COMMIT slow data
	CSphMappedBuffer<BYTE>						m_tBuf;					///< my cache
	CSphFixedVector<CSphWordlistCheckpoint>		m_dCheckpoints {0};		///< checkpoint offsets
	CSphVector<InfixBlock_t>					m_dInfixBlocks {0};
	CSphFixedVector<BYTE>						m_pWords {0};			///< arena for checkpoint's words
	BYTE *										m_pInfixBlocksWords = nullptr;	///< arena for infix checkpoint's words
	int											m_iSkiplistBlockSize {0};

	SphOffset_t									m_iWordsEnd = 0;		///< end of wordlist
	CSphScopedPtr<ISphCheckpointReader>			m_tMapedCpReader { nullptr };

public:
										~CWordlist () override;
	void								Reset();
	bool								Preread ( const CSphString & sName, bool bWordDict, int iSkiplistBlockSize, CSphString & sError );

	const CSphWordlistCheckpoint *		FindCheckpoint ( const char * sWord, int iWordLen, SphWordID_t iWordID, bool bStarMode ) const;
	bool								GetWord ( const BYTE * pBuf, SphWordID_t iWordID, CSphDictEntry & tWord ) const;

	const BYTE *						AcquireDict ( const CSphWordlistCheckpoint * pCheckpoint ) const;
	void								GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const override;
	void								GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const override;

	void								SuffixGetChekpoints ( const SuggestResult_t & tRes, const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const override;
	void								SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const override;
	bool								ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const override;

	void								DebugPopulateCheckpoints();

private:
	bool								m_bWordDict = false;
};

//////////////////////////////////////////////////////////////////////////
// dirty hack for some build systems which not has LLONG_MAX
#ifndef LLONG_MAX
#define LLONG_MAX (((unsigned long long)(-1))>>1)
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#endif


AttrIndexBuilder_c::AttrIndexBuilder_c ( const CSphSchema & tSchema )
	: m_uStride ( tSchema.GetRowSize() )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_BIGINT:
		case SPH_ATTR_TOKENCOUNT:
			m_dIntAttrs.Add ( tCol.m_tLocator );
			break;

		case SPH_ATTR_FLOAT:
			m_dFloatAttrs.Add ( tCol.m_tLocator );
			break;

		default:
			break;
		}
	}

	m_dIntMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatMax.Resize ( m_dFloatAttrs.GetLength() );

	m_dIntIndexMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntIndexMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatIndexMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatIndexMax.Resize ( m_dFloatAttrs.GetLength() );

	for ( auto & i : m_dIntIndexMin )
		i = LLONG_MAX;

	for ( auto & i : m_dIntIndexMax )
		i = LLONG_MIN;

	for ( auto & i : m_dFloatIndexMin )
		i = FLT_MAX;

	for ( auto & i : m_dFloatIndexMax )
		i = FLT_MIN;

	ResetLocal();
}


void AttrIndexBuilder_c::Collect ( const CSphRowitem * pRow )
{
	// check if it is time to flush already collected values
	if ( m_nLocalCollected>=DOCINFO_INDEX_FREQ )
		FlushComputed();

	m_nLocalCollected++;

	// ints
	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		SphAttr_t tVal = sphGetRowAttr ( pRow, m_dIntAttrs[i] );
		m_dIntMin[i] = Min ( m_dIntMin[i], tVal );
		m_dIntMax[i] = Max ( m_dIntMax[i], tVal );
	}

	// floats
	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, m_dFloatAttrs[i] ) );
		m_dFloatMin[i] = Min ( m_dFloatMin[i], fVal );
		m_dFloatMax[i] = Max ( m_dFloatMax[i], fVal );
	}
}


void AttrIndexBuilder_c::FinishCollect()
{
	if ( m_nLocalCollected )
		FlushComputed();

	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntIndexMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntIndexMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMax[i] ) );
	}
}


const CSphTightVector<CSphRowitem> & AttrIndexBuilder_c::GetCollected() const
{
	return m_dMinMaxRows;
}


void AttrIndexBuilder_c::ResetLocal()
{
	for ( auto & i : m_dIntMin )
		i = LLONG_MAX;

	for ( auto & i : m_dIntMax )
		i = LLONG_MIN;

	for ( auto & i : m_dFloatMin )
		i = FLT_MAX;

	for ( auto & i : m_dFloatMax )
		i = FLT_MIN;

	m_nLocalCollected = 0;
}


void AttrIndexBuilder_c::FlushComputed ()
{
	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		m_dIntIndexMin[i] = Min ( m_dIntIndexMin[i], m_dIntMin[i] );
		m_dIntIndexMax[i] = Max ( m_dIntIndexMax[i], m_dIntMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		m_dFloatIndexMin[i] = Min ( m_dFloatIndexMin[i], m_dFloatMin[i] );
		m_dFloatIndexMax[i] = Max ( m_dFloatIndexMax[i], m_dFloatMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMax[i] ) );
	}

	ResetLocal();
}


//////////////////////////////////////////////////////////////////////////

class CSphHitBuilder;

struct BuildHeader_t : public CSphSourceStats, public DictHeader_t
{
	explicit BuildHeader_t ( const CSphSourceStats & tStat )
	{
		m_iTotalDocuments = tStat.m_iTotalDocuments;
		m_iTotalBytes = tStat.m_iTotalBytes;
	}

	int64_t				m_iDocinfo {0};
	int64_t				m_iDocinfoIndex {0};
	int64_t				m_iMinMaxIndex {0};
};

struct WriteHeader_t
{
	const CSphIndexSettings * m_pSettings;
	const CSphSchema * m_pSchema;
	ISphTokenizerRefPtr_c m_pTokenizer;
	CSphDictRefPtr_c m_pDict;
	ISphFieldFilterRefPtr_c m_pFieldFilter;
	const int64_t * m_pFieldLens;
};

const char* CheckFmtMagic ( DWORD uHeader )
{
	if ( uHeader!=INDEX_MAGIC_HEADER )
	{
		FlipEndianess ( &uHeader );
		if ( uHeader==INDEX_MAGIC_HEADER )
#if USE_LITTLE_ENDIAN
			return "This instance is working on little-endian platform, but %s seems built on big-endian host.";
#else
			return "This instance is working on big-endian platform, but %s seems built on little-endian host.";
#endif
		else
			return "%s is invalid header file (too old index version?)";
	}
	return NULL;
}


static IndexFileExt_t g_dIndexFilesExts[SPH_EXT_TOTAL] =
{
	{ SPH_EXT_SPH,	".sph",		1,	false,	true,	"header file" },
	{ SPH_EXT_SPA,	".spa",		1,	false,	true,	"attribute values" },
	{ SPH_EXT_SPB,	".spb",		50,	true,	true,	"var-length attrs: strings, jsons, mva" },
	{ SPH_EXT_SPI,	".spi",		1,	false,	true,	"dictionary (aka wordlist)" },
	{ SPH_EXT_SPD,	".spd",		1,	false,	true,	"document lists (aka doclists)"},
	{ SPH_EXT_SPP,	".spp",		3,	false,	true,	"keyword positions lists (aka hitlists)" },
	{ SPH_EXT_SPK,	".spk",		10, true,	true,	"kill list (aka klist)" },
	{ SPH_EXT_SPE,	".spe",		31,	false,	true,	"skip-lists to speed up doc-list filtering" },
	{ SPH_EXT_SPM,	".spm",		4,	false,	true,	"dead row map" },
	{ SPH_EXT_SPT,	".spt",		53,	true,	true,	"docid lookup table" },
	{ SPH_EXT_SPHI,	".sphi",	53,	true,	true,	"secondary index histograms" },
	{ SPH_EXT_SPL,	".spl",		1,	true,	false,	"file lock for the index" }
};


CSphString sphGetExt ( ESphExt eExt )
{
	if ( eExt<SPH_EXT_SPH || eExt>=SPH_EXT_TOTAL )
		return "";

	return g_dIndexFilesExts[eExt].m_szExt;
}


CSphVector<IndexFileExt_t> sphGetExts()
{
	// we may add support for older index versions in the future
	CSphVector<IndexFileExt_t> dResult;
	for ( int i = 0; i < SPH_EXT_TOTAL; i++ )
		dResult.Add ( g_dIndexFilesExts[i] );

	return dResult;
}


/// this pseudo-index used to store and manage the tokenizer
/// without any footprint in real files
//////////////////////////////////////////////////////////////////////////
static CSphSourceStats g_tTmpDummyStat;
class CSphTokenizerIndex : public CSphIndex
{
public:
	CSphTokenizerIndex () : CSphIndex ( nullptr, nullptr ) {}
	int							Kill ( DocID_t tDocID ) override { return 0; }

	int							Build ( const CSphVector<CSphSource*> & , int , int ) override { return 0; }
	bool						Merge ( CSphIndex * , const CSphVector<CSphFilterSettings> &, bool ) override {return false; }
	bool				Prealloc ( bool ) final { return false; }
	void				Dealloc () final {}
	void				Preread () final {}
	void				SetMemorySettings ( const FileAccessSettings_t & ) final {}
	void				SetBase ( const char * ) final {}
	bool				Rename ( const char * ) final { return false; }
	bool				Lock () final { return true; }
	void				Unlock () final {}
	bool				EarlyReject ( CSphQueryContext * , CSphMatch & ) const final { return false; }
	const CSphSourceStats &	GetStats () const final { return g_tTmpDummyStat; }
	void			GetStatus ( CSphIndexStatus* pRes ) const final { assert (pRes); if ( pRes ) { pRes->m_iDiskUse = 0; pRes->m_iRamUse = 0;}}
	bool				MultiQuery ( const CSphQuery * , CSphQueryResult * , int , ISphMatchSorter ** , const CSphMultiQueryArgs & ) const final { return false; }
	bool				MultiQueryEx ( int , const CSphQuery * , CSphQueryResult ** , ISphMatchSorter ** , const CSphMultiQueryArgs & ) const final { return false; }
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , const GetKeywordsSettings_t & tSettings, CSphString * ) const final ;
	bool				FillKeywords ( CSphVector <CSphKeywordInfo> & ) const final { return true; }
	int					UpdateAttributes ( const CSphAttrUpdate & , int , bool &, CSphString & , CSphString & ) final { return -1; }
	bool				SaveAttributes ( CSphString & ) const final { return true; }
	DWORD				GetAttributeStatus () const final { return 0; }
	bool				AddRemoveAttribute ( bool, const CSphString &, ESphAttr, CSphString & ) final { return true; }
	void				DebugDumpHeader ( FILE *, const char *, bool ) final {}
	void				DebugDumpDocids ( FILE * ) final {}
	void				DebugDumpHitlist ( FILE * , const char * , bool ) final {}
	int					DebugCheck ( FILE * ) final { return 0; } // NOLINT
	void				DebugDumpDict ( FILE * ) final {}
	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) final {}
};


struct CSphTemplateQueryFilter : public ISphQueryFilter
{
	void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) final
	{
		SphWordID_t iWord = m_pDict->GetWordID ( sWord );
		if ( !iWord )
			return;

		CSphKeywordInfo & tInfo = dKeywords.Add();
		tInfo.m_sTokenized = (const char *)sTokenized;
		tInfo.m_sNormalized = (const char*)sWord;
		tInfo.m_iDocs = 0;
		tInfo.m_iHits = 0;
		tInfo.m_iQpos = iQpos;

		RemoveDictSpecials ( tInfo.m_sNormalized );
	}
};


bool CSphTokenizerIndex::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * ) const
{
	// short-cut if no query or keywords to fill
	if ( !szQuery || !szQuery[0] )
		return true;

	ISphTokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_INDEX ) };
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( IsStarDict() )
	{
		pTokenizer->AddPlainChar ( '*' );
		pDict = new CSphDictStar ( pDict );
	}

	if ( m_tSettings.m_bIndexExactWords )
	{
		pTokenizer->AddPlainChar ( '=' );
		pDict = new CSphDictExact ( pDict );
	}

	dKeywords.Resize ( 0 );

	CSphVector<BYTE> dFiltered;
	ISphFieldFilterRefPtr_c pFieldFilter;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, strlen ( (char*)sModifiedQuery ), dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	pTokenizer->SetBuffer ( sModifiedQuery, strlen ( (const char*)sModifiedQuery) );

	CSphTemplateQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = pTokenizer;
	tAotFilter.m_pDict = pDict;
	tAotFilter.m_pSettings = &m_tSettings;
	tAotFilter.m_tFoldSettings = tSettings;
	tAotFilter.m_tFoldSettings.m_bStats = false;
	tAotFilter.m_tFoldSettings.m_bFoldWildcards = true;

	ExpansionContext_t tExpCtx;

	tAotFilter.GetKeywords ( dKeywords, tExpCtx );

	return true;
}


CSphIndex * sphCreateIndexTemplate ( )
{
	return new CSphTokenizerIndex();
}

//////////////////////////////////////////////////////////////////////////

UpdateContext_t::UpdateContext_t ( const CSphAttrUpdate & tUpd, const ISphSchema & tSchema, const HistogramContainer_c * pHistograms, int iFirst, int iLast )
	: m_tUpd ( tUpd )
	, m_tSchema ( tSchema )
	, m_pHistograms ( pHistograms )
	, m_dUpdatedRows ( iLast-iFirst )
	, m_dUpdatedAttrs ( tUpd.m_dAttributes.GetLength() )
	, m_dSchemaUpdateMask ( tSchema.GetAttrsCount() )
	, m_iFirst ( iFirst )
	, m_iLast ( iLast )
{}


UpdatedRowData_t & UpdateContext_t::GetRowData ( int iUpd )
{
	return m_dUpdatedRows[iUpd-m_iFirst];
}

//////////////////////////////////////////////////////////////////////////

bool IndexUpdateHelper_c::Update_FixupData ( UpdateContext_t & tCtx, CSphString & sError )
{
	ARRAY_FOREACH ( i, tCtx.m_tUpd.m_dAttributes )
	{
		const CSphString & sUpdAttrName = tCtx.m_tUpd.m_dAttributes[i].m_sName;
		ESphAttr eUpdAttrType = tCtx.m_tUpd.m_dAttributes[i].m_eType;
		UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[i];

		int iUpdAttrId = tCtx.m_tSchema.GetAttrIndex ( sUpdAttrName.cstr() );

		if ( iUpdAttrId<0 )
		{
			CSphString sJsonCol, sJsonKey;
			if ( sphJsonNameSplit ( sUpdAttrName.cstr(), &sJsonCol, &sJsonKey ) )
			{
				iUpdAttrId = tCtx.m_tSchema.GetAttrIndex ( sJsonCol.cstr() );
				if ( iUpdAttrId>=0 )
					tUpdAttr.m_pExpr = sphExprParse ( sUpdAttrName.cstr(), tCtx.m_tSchema, NULL, NULL, sError, NULL );
			}
		}

		if ( iUpdAttrId>=0 )
		{
			// forbid updates on non-int columns
			const CSphColumnInfo & tCol = tCtx.m_tSchema.GetAttr(iUpdAttrId);
			if ( !( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP
				|| tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_STRING
				|| tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_FLOAT || tCol.m_eAttrType==SPH_ATTR_JSON ))
			{
				sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, bigint, float, timestamp, string, MVA or JSON)", sUpdAttrName.cstr() );
				return false;
			}

			bool bSrcMva = ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );
			bool bDstMva = ( eUpdAttrType==SPH_ATTR_UINT32SET || eUpdAttrType==SPH_ATTR_INT64SET );
			if ( bSrcMva!=bDstMva )
			{
				sError.SetSprintf ( "attribute '%s' MVA flag mismatch", sUpdAttrName.cstr() );
				return false;
			}

			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_UINT32SET:
				if ( eUpdAttrType==SPH_ATTR_INT64SET )
				{
					sError.SetSprintf ( "attribute '%s' MVA bits (dst=%d, src=%d) mismatch", sUpdAttrName.cstr(), tCol.m_eAttrType, eUpdAttrType );
					return false;
				}
				break;

			case SPH_ATTR_FLOAT:
				if ( eUpdAttrType==SPH_ATTR_BIGINT )
					tUpdAttr.m_eConversion = CONVERSION_BIGINT2FLOAT;
				break;

			case SPH_ATTR_BIGINT:
				if ( eUpdAttrType==SPH_ATTR_FLOAT )
					tUpdAttr.m_eConversion = CONVERSION_FLOAT2BIGINT;
				break;

			default:
				break;
			}

			tUpdAttr.m_eAttrType = tCol.m_eAttrType;
			tUpdAttr.m_tLocator = tCol.m_tLocator;
			tUpdAttr.m_pHistogram = tCtx.m_pHistograms ? tCtx.m_pHistograms->Get(tCol.m_sName) : nullptr;
			tUpdAttr.m_bExisting = true;

			tCtx.m_dSchemaUpdateMask.BitSet(iUpdAttrId);
			tCtx.m_bBlobUpdate |= sphIsBlobAttr ( tCol.m_eAttrType );
		}
		else if ( tCtx.m_tUpd.m_bIgnoreNonexistent )
			continue;
		else
		{
			sError.SetSprintf ( "attribute '%s' not found", sUpdAttrName.cstr() );
			return false;
		}

		// this is a hack
		// Query parser tries to detect an attribute type. And this is wrong because, we should
		// take attribute type from schema. Probably we'll rewrite updates in future but
		// for now this fix just works.
		// Fixes cases like UPDATE float_attr=1 WHERE id=1;
		assert ( iUpdAttrId>=0 );
		if ( eUpdAttrType==SPH_ATTR_INTEGER && tCtx.m_tSchema.GetAttr(iUpdAttrId).m_eAttrType==SPH_ATTR_FLOAT )
		{
			const_cast<CSphAttrUpdate &>(tCtx.m_tUpd).m_dAttributes[i].m_eType = SPH_ATTR_FLOAT;
			const_cast<CSphAttrUpdate &>(tCtx.m_tUpd).m_dPool[i] = sphF2DW ( (float)tCtx.m_tUpd.m_dPool[i] );
		}
	}

	return true;
}


static void IncUpdatePoolPos ( UpdateContext_t & tCtx, int iAttr, int & iPos )
{
	switch ( tCtx.m_tUpd.m_dAttributes[iAttr].m_eType )
	{
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		iPos += tCtx.m_tUpd.m_dPool[iPos] + 1;
		break;

	case SPH_ATTR_STRING:
	case SPH_ATTR_BIGINT:
		iPos += 2;
		break;

	default:
		iPos += 1;
		break;
	}
}


static bool FitsInplaceJsonUpdate ( const UpdateContext_t & tCtx, int iAttr )
{
	// only json fields and no strings (strings go as full json updates)
	return tCtx.m_dUpdatedAttrs[iAttr].m_eAttrType==SPH_ATTR_JSON && tCtx.m_tUpd.m_dAttributes[iAttr].m_eType!=SPH_ATTR_STRING;
}


bool IndexUpdateHelper_c::Update_InplaceJson ( UpdateContext_t & tCtx, CSphString & sError, bool bDryRun )
{
	for ( int iUpd=tCtx.m_iFirst; iUpd<tCtx.m_iLast; iUpd++ )
	{
		UpdatedRowData_t & tRow = tCtx.GetRowData(iUpd);
		if ( !tRow.m_pRow )
			continue;

		int iPos = tCtx.m_tUpd.m_dRowOffset[iUpd];
		ARRAY_FOREACH ( i, tCtx.m_tUpd.m_dAttributes )
		{
			if ( !FitsInplaceJsonUpdate ( tCtx, i ) || !tCtx.m_dUpdatedAttrs[i].m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, i, iPos );
				continue;
			}

			ESphAttr eAttr = tCtx.m_tUpd.m_dAttributes[i].m_eType;

			bool bBigint = eAttr==SPH_ATTR_BIGINT;
			bool bDouble = eAttr==SPH_ATTR_FLOAT;

			ESphJsonType eType = bDouble ? JSON_DOUBLE : ( bBigint ? JSON_INT64 : JSON_INT32 );
			SphAttr_t uValue = bDouble ? sphD2QW ( (double)sphDW2F ( tCtx.m_tUpd.m_dPool[iPos] ) ) : ( bBigint ? MVA_UPSIZE ( &tCtx.m_tUpd.m_dPool[iPos] ) : tCtx.m_tUpd.m_dPool[iPos] );

			if ( sphJsonInplaceUpdate ( eType, uValue, tCtx.m_dUpdatedAttrs[i].m_pExpr, tRow.m_pBlobPool, tRow.m_pRow, !bDryRun ) )
			{
				tRow.m_bUpdated = true;
				tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
			}
			else
			{
				if ( bDryRun )
				{
					sError.SetSprintf ( "attribute '%s' can not be updated (not found or incompatible types)", tCtx.m_tUpd.m_dAttributes[i].m_sName.cstr() );
					return false;
				} else
					tCtx.m_iJsonWarnings++;
			}

			IncUpdatePoolPos ( tCtx, i, iPos );
		}
	}

	return true;
}


bool IndexUpdateHelper_c::Update_Blobs ( UpdateContext_t & tCtx, bool & bCritical, CSphString & sError )
{
	// any blobs supplied in the update?
	if ( !tCtx.m_bBlobUpdate )
		return true;

	//	create a remap from attribute id in UPDATE to blob attr id
	CSphVector<int> dRemap ( tCtx.m_tUpd.m_dAttributes.GetLength() );
	dRemap.Fill(-1);

	CSphVector<int> dBlobAttrIds;

	bool bNeedBlobBuilder = false;
	int iBlobAttrId = 0;
	for ( int i = 0; i < tCtx.m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tCtx.m_tSchema.GetAttr(i);
		if ( sphIsBlobAttr ( tAttr.m_eAttrType ) )
		{
			dBlobAttrIds.Add(i);
			ARRAY_FOREACH ( iUpd, tCtx.m_tUpd.m_dAttributes )
			{
				const TypedAttribute_t & tTypedAttr = tCtx.m_tUpd.m_dAttributes[iUpd];

				if ( sphIsBlobAttr ( tTypedAttr.m_eType ) && tAttr.m_sName==tTypedAttr.m_sName )
				{
					dRemap[iUpd] = iBlobAttrId;
					bNeedBlobBuilder = true;
				}
			}

			iBlobAttrId++;
		}
	}

	if ( !bNeedBlobBuilder )
		return true;

	CSphTightVector<BYTE> tBlobPool;
	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder ( sphCreateBlobRowBuilderUpdate ( tCtx.m_tSchema, tBlobPool, tCtx.m_dSchemaUpdateMask ) );

	for ( int iUpd=tCtx.m_iFirst; iUpd<tCtx.m_iLast; iUpd++ )
	{
		UpdatedRowData_t & tRow = tCtx.GetRowData(iUpd);
		if ( !tRow.m_pRow )
			continue;

		tBlobPool.Resize(0);
		ARRAY_FOREACH ( iBlobId, dBlobAttrIds )
		{
			int iCol = dBlobAttrIds[iBlobId];
			if ( tCtx.m_dSchemaUpdateMask.BitGet(iCol) )
				continue;

			const CSphColumnInfo & tAttr = tCtx.m_tSchema.GetAttr(iCol);
			int iLengthBytes = 0;
			const BYTE * pData = sphGetBlobAttr ( tRow.m_pRow, tAttr.m_tLocator, tRow.m_pBlobPool, iLengthBytes );
			if ( !pBlobRowBuilder->SetAttr ( iBlobId, pData, iLengthBytes, sError ) )
				return false;
		}

		int iPos = tCtx.m_tUpd.m_dRowOffset[iUpd];
		ARRAY_FOREACH ( iCol, tCtx.m_tUpd.m_dAttributes )
		{
			ESphAttr eAttr = tCtx.m_tUpd.m_dAttributes[iCol].m_eType;

			if ( !sphIsBlobAttr(eAttr) || FitsInplaceJsonUpdate ( tCtx, iCol ) || !tCtx.m_dUpdatedAttrs[iCol].m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				continue;
			}

			int iBlobId = dRemap[iCol];

			switch ( eAttr )
			{
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
				{
					DWORD uLength = tCtx.m_tUpd.m_dPool[iPos++];
					if ( iBlobId!=-1 )
					{
						pBlobRowBuilder->SetAttr ( iBlobId, (const BYTE *)(tCtx.m_tUpd.m_dPool.Begin()+iPos), uLength*sizeof(DWORD), sError );
						tRow.m_bUpdated = true;
						tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
					}

					iPos += uLength;
				}
				break;

			case SPH_ATTR_STRING:
				{
					DWORD uOffset = tCtx.m_tUpd.m_dPool[iPos++];
					DWORD uLength = tCtx.m_tUpd.m_dPool[iPos++];
					if ( iBlobId!=-1 )
					{

						pBlobRowBuilder->SetAttr ( iBlobId, &tCtx.m_tUpd.m_dBlobs[uOffset], uLength, sError );
						tRow.m_bUpdated = true;
						tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
					}
				}
				break;

			default:
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				break;
			}
		}

		pBlobRowBuilder->Flush();

		if ( !Update_WriteBlobRow ( tCtx, iUpd, tRow.m_pRow, tBlobPool.Begin(), tBlobPool.GetLength(), iBlobAttrId, bCritical, sError ) )
			return false;
	}

	return true;
}


bool IndexUpdateHelper_c::Update_HandleJsonWarnings ( UpdateContext_t & tCtx, int iUpdated, CSphString & sWarning, CSphString & sError )
{
	if ( !tCtx.m_iJsonWarnings )
		return true;

	sWarning.SetSprintf ( "%d attribute(s) can not be updated (not found or incompatible types)", tCtx.m_iJsonWarnings );
	if ( !iUpdated )
	{
		sError = sWarning;
		return false;
	}

	return true;
}


void IndexUpdateHelper_c::Update_Plain ( UpdateContext_t & tCtx )
{
	for ( int iUpd=tCtx.m_iFirst; iUpd < tCtx.m_iLast; iUpd++ )
	{
		UpdatedRowData_t & tRow = tCtx.GetRowData(iUpd);
		if ( !tRow.m_pRow )
			continue; // no such id

		int iPos = tCtx.m_tUpd.m_dRowOffset[iUpd];
		ARRAY_FOREACH ( iCol, tCtx.m_tUpd.m_dAttributes )
		{
			ESphAttr eAttr = tCtx.m_tUpd.m_dAttributes[iCol].m_eType;
			const UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[iCol];

			// already updated?
			if ( sphIsBlobAttr(eAttr) || tUpdAttr.m_eAttrType==SPH_ATTR_JSON || !tUpdAttr.m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				continue;
			}

			const CSphAttrLocator & tLoc = tUpdAttr.m_tLocator;

			bool bBigint = eAttr==SPH_ATTR_BIGINT;
			SphAttr_t uValue = bBigint ? MVA_UPSIZE ( &tCtx.m_tUpd.m_dPool[iPos] ) : tCtx.m_tUpd.m_dPool[iPos];

			if ( tUpdAttr.m_eConversion==CONVERSION_BIGINT2FLOAT ) // handle bigint(-1) -> float attr updates
				uValue = sphF2DW ( float((int64_t)uValue) );
			else if ( tUpdAttr.m_eConversion==CONVERSION_FLOAT2BIGINT ) // handle float(1.0) -> bigint attr updates
				uValue = (int64_t)sphDW2F((DWORD)uValue);

			Histogram_i * pHistogram = tUpdAttr.m_pHistogram;
			if ( pHistogram )
			{
				SphAttr_t tOldValue = sphGetRowAttr ( tRow.m_pRow, tLoc );
				pHistogram->Delete ( tOldValue );
				pHistogram->Insert ( uValue );
			}

			sphSetRowAttr ( tRow.m_pRow, tLoc, uValue );
			tRow.m_bUpdated = true;
			tCtx.m_uUpdateMask |= ATTRS_UPDATED;

			// next
			IncUpdatePoolPos ( tCtx, iCol, iPos );
		}
	}
}


//////////////////////////////////////////////////////////////////////////

class WriteWrapper_Disk_c : public WriteWrapper_c
{
public:
	WriteWrapper_Disk_c ( CSphWriter & tWriter )
		: m_tWriter ( tWriter )
	{}

	void PutBytes ( const BYTE * pData, int iSize ) override
	{
		m_tWriter.PutBytes ( pData, iSize );
	}

	SphOffset_t	GetPos() const override
	{
		return m_tWriter.GetPos();
	}

	bool IsError() const override
	{
		return m_tWriter.IsError();
	}

private:
	CSphWriter & m_tWriter;
};


bool IndexAlterHelper_c::Alter_AddRemoveAttr ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, const CSphRowitem * pDocinfoMax,
	const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter, bool bAddAttr, const CSphString & sAttrName )
{
	int nOldBlobs = 0;
	for ( int i = 0; i < tOldSchema.GetAttrsCount(); i++ )
		if ( sphIsBlobAttr ( tOldSchema.GetAttr(i).m_eAttrType ) )
			nOldBlobs++;

	int nNewBlobs = 0;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		if ( sphIsBlobAttr ( tNewSchema.GetAttr(i).m_eAttrType ) )
			nNewBlobs++;

	bool bHadBlobs = nOldBlobs>0;
	bool bHaveBlobs = nNewBlobs>0;

	const CSphColumnInfo * pAttr = bAddAttr ? tNewSchema.GetAttr ( sAttrName.cstr() ) : tOldSchema.GetAttr ( sAttrName.cstr() ) ;
	assert ( pAttr );

	bool bBlob = sphIsBlobAttr ( pAttr->m_eAttrType );
	bool bBlobsModified = bBlob && ( bAddAttr || bHaveBlobs==bHadBlobs );

	int iOldStride = tOldSchema.GetRowSize();
	int iNewStride = tNewSchema.GetRowSize();

	const CSphRowitem * pNextDocinfo = nullptr;
	CSphFixedVector<DWORD> dAttrRow ( iNewStride );
	CSphTightVector<BYTE> dBlobRow;

	if ( bAddAttr )
	{
		CSphVector<int> dAttrMap;
		if ( bHadBlobs!=bHaveBlobs )
			CreateAttrMap ( dAttrMap, tOldSchema, tNewSchema, -1 );

		const CSphColumnInfo * pNewAttr = tNewSchema.GetAttr ( sAttrName.cstr() );
		assert ( pNewAttr );

		while ( pDocinfo < pDocinfoMax )
		{
			if ( bHadBlobs==bHaveBlobs )
				pNextDocinfo = CopyRow ( pDocinfo, dAttrRow.Begin(), pNewAttr, iOldStride );
			else
				pNextDocinfo = CopyRowAttrByAttr ( pDocinfo, dAttrRow.Begin(), tOldSchema, tNewSchema, dAttrMap, iOldStride );

			if ( bBlob && !Alter_IsMinMax ( pDocinfo, iOldStride ) )
			{
				sphAddAttrToBlobRow ( pDocinfo, dBlobRow, pBlobPool, nOldBlobs );

				SphOffset_t tRowOffset = tSPBWriter.GetPos();
				tSPBWriter.PutBytes ( dBlobRow.Begin(), dBlobRow.GetLength() );
				if ( tSPBWriter.IsError() )
					return false;

				sphSetBlobRowOffset ( dAttrRow.Begin(), tRowOffset );
			}

			tSPAWriter.PutBytes ( (const BYTE *)dAttrRow.Begin(), iNewStride*sizeof(CSphRowitem) );
			if ( tSPAWriter.IsError() )
				return false;

			pDocinfo = pNextDocinfo;
		}
	} else
	{
		int iAttrToRemove = tOldSchema.GetAttrIndex ( sAttrName.cstr() );
		const CSphColumnInfo & tOldAttr = tOldSchema.GetAttr ( iAttrToRemove );
		assert ( iAttrToRemove>=0 );

		CSphVector<int> dAttrMap;
		CreateAttrMap ( dAttrMap, tOldSchema, tNewSchema, iAttrToRemove );

		while ( pDocinfo < pDocinfoMax )
		{
			pNextDocinfo = CopyRowAttrByAttr ( pDocinfo, dAttrRow.Begin(), tOldSchema, tNewSchema, dAttrMap, iOldStride );

			if ( bBlobsModified && !Alter_IsMinMax ( pDocinfo, iOldStride ) )
			{
				sphRemoveAttrFromBlobRow ( pDocinfo, dBlobRow, pBlobPool, nOldBlobs, tOldAttr.m_tLocator.m_iBlobAttrId );

				SphOffset_t tRowOffset = tSPBWriter.GetPos();
				tSPBWriter.PutBytes ( dBlobRow.Begin(), dBlobRow.GetLength() );
				if ( tSPBWriter.IsError() )
					return false;

				sphSetBlobRowOffset ( dAttrRow.Begin(), tRowOffset );
			}

			tSPAWriter.PutBytes ( (const BYTE *)dAttrRow.Begin(), iNewStride*sizeof(CSphRowitem) );
			if ( tSPAWriter.IsError() )
				return false;

			pDocinfo = pNextDocinfo;
		}
	}

	return true;
}


bool IndexAlterHelper_c::Alter_AddRemoveFromSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, bool bAdd, CSphString & sError )
{
	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

	bool bBlob = sphIsBlobAttr ( eAttrType );
	if ( bAdd )
	{
		if ( bBlob && !pBlobLocator )
		{
			CSphColumnInfo tCol ( sphGetBlobLocatorName() );
			tCol.m_eAttrType = SPH_ATTR_BIGINT;

			// should be right after docid
			tSchema.InsertAttr ( 1, tCol, false );

			// rebuild locators in the schema
			const char * szTmpColName = "$_tmp";
			CSphColumnInfo tTmpCol ( szTmpColName, SPH_ATTR_BIGINT );
			tSchema.AddAttr ( tTmpCol, false );
			tSchema.RemoveAttr ( szTmpColName, false );
		}

		CSphColumnInfo tInfo ( sAttrName.cstr(), eAttrType );
		tSchema.AddAttr ( tInfo, false );
	} else
	{
		tSchema.RemoveAttr ( sAttrName.cstr(), false );

		if ( bBlob && pBlobLocator )
		{
			// remove blob locator if no blobs are left
			int nBlobs = 0;
			for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
				if ( sphIsBlobAttr ( tSchema.GetAttr(i).m_eAttrType ) )
					nBlobs++;

			if ( !nBlobs )
				tSchema.RemoveAttr ( sphGetBlobLocatorName(), false );
		}

		if ( !tSchema.GetAttrsCount() )
		{
			sError = "unable to remove the only attribute left";
			return false;
		}
	}

	return true;
}


bool IndexAlterHelper_c::Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const
{
	return false;
}


void IndexAlterHelper_c::CreateAttrMap ( CSphVector<int> & dAttrMap, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, int iAttrToRemove )
{
	dAttrMap.Resize ( tOldSchema.GetAttrsCount() );
	for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
		if ( iAttr!=iAttrToRemove )
			dAttrMap[iAttr] = tNewSchema.GetAttrIndex ( tOldSchema.GetAttr(iAttr).m_sName.cstr() );
		else
			dAttrMap[iAttr] = -1;
}


const CSphRowitem * IndexAlterHelper_c::CopyRow ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, const CSphColumnInfo * pNewAttr, int iOldStride )
{
	memcpy ( pTmpDocinfo, pDocinfo, iOldStride*sizeof(DWORD) );
	if ( !pNewAttr->m_tLocator.IsBlobAttr() )
		sphSetRowAttr ( pTmpDocinfo, pNewAttr->m_tLocator, 0 );

	return pDocinfo + iOldStride;
}


const CSphRowitem * IndexAlterHelper_c::CopyRowAttrByAttr ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphVector<int> & dAttrMap, int iOldStride )
{
	for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
	{
		if ( dAttrMap[iAttr]==-1 )
			continue;

		const CSphColumnInfo & tOldAttr = tOldSchema.GetAttr(iAttr);
		const CSphColumnInfo & tNewAttr = tNewSchema.GetAttr(dAttrMap[iAttr]);

		if ( sphIsBlobAttr ( tOldAttr.m_eAttrType ) )
			continue;

		SphAttr_t tValue = sphGetRowAttr ( pDocinfo, tOldAttr.m_tLocator );
		sphSetRowAttr ( pTmpDocinfo, tNewAttr.m_tLocator, tValue );
	}

	return pDocinfo + iOldStride;
}

//////////////////////////////////////////////////////////////////////////

struct FileDebugCheckReader_c : public DebugCheckReader_i
{
	FileDebugCheckReader_c ( CSphAutoreader * pReader )
		: m_pReader ( pReader )
	{}
	~FileDebugCheckReader_c () final
	{}
	int64_t GetLengthBytes () final
	{
		return ( m_pReader ? m_pReader->GetFilesize() : 0 );
	}
	bool GetBytes ( void * pData, int iSize ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->GetBytes ( pData, iSize );
		return !m_pReader->GetErrorFlag();
	}

	bool SeekTo ( int64_t iOff, int iHint ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->SeekTo ( iOff, iHint );
		return !m_pReader->GetErrorFlag();
	}

	CSphAutoreader * m_pReader = nullptr;
};


void DebugCheckHelper_c::DebugCheck_Attributes ( DebugCheckReader_i & tAttrs, DebugCheckReader_i & tBlobs, int64_t nRows, int64_t iMinMaxBytes, const CSphSchema & tSchema, DebugCheckError_c & tReporter )
{
	// empty?
	if ( !tAttrs.GetLengthBytes() )
		return;

	tReporter.Msg ( "checking rows..." );

	if ( !tSchema.GetAttrsCount() )
		tReporter.Fail ( "no attributes in schema; schema should at least have '%s' attr", sphGetDocidName() );

	if ( tSchema.GetAttr(0).m_sName!=sphGetDocidName() )
		tReporter.Fail ( "first attribute in schema should be '%s'", tSchema.GetAttr(0).m_sName.cstr() );

	if ( tSchema.GetAttr(0).m_eAttrType!=SPH_ATTR_BIGINT )
		tReporter.Fail ( "%s attribute should be BIGINT", sphGetDocidName() );

	const CSphColumnInfo * pBlobLocator = nullptr;
	int nBlobAttrs = 0;

	if ( tSchema.HasBlobAttrs() )
	{
		pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

		if ( !pBlobLocator )
			tReporter.Fail ( "schema has blob attrs, but no blob locator '%s'", sphGetBlobLocatorName() );

		if ( tSchema.GetAttr(1).m_sName!=sphGetBlobLocatorName() )
			tReporter.Fail ( "second attribute in schema should be '%s'", sphGetBlobLocatorName() );

		if ( tSchema.GetAttr(1).m_eAttrType!=SPH_ATTR_BIGINT )
			tReporter.Fail ( "%s attribute should be BIGINT", sphGetBlobLocatorName() );

		if ( !tBlobs.GetLengthBytes() )
			tReporter.Fail ( "schema has blob attrs, but blob file is empty" );

		for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
			if ( sphIsBlobAttr(  tSchema.GetAttr(i).m_eAttrType ) )
				nBlobAttrs++;
	} else
	{
		if ( tBlobs.GetLengthBytes() )
			tReporter.Fail ( "schema has no blob attrs but has blob rows" );
	}

	// sizes and counts
	DWORD uStride = tSchema.GetRowSize();

	int64_t iAttrElemCount = ( tAttrs.GetLengthBytes() - iMinMaxBytes ) / sizeof(CSphRowitem);
	int64_t iAttrExpected = nRows*uStride;
	if ( iAttrExpected > iAttrElemCount )
		tReporter.Fail ( "rowitems count mismatch (expected=" INT64_FMT ", loaded=" INT64_FMT ")", iAttrExpected, iAttrElemCount );

	CSphVector<CSphAttrLocator> dFloatItems;
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
			dFloatItems.Add	( tAttr.m_tLocator );
	}

	CSphFixedVector<CSphRowitem> dRow ( tSchema.GetRowSize() );
	const CSphRowitem * pRow = dRow.Begin();
	tAttrs.SeekTo ( 0, dRow.GetLengthBytes() );

	for ( int64_t iRow=0; iRow<nRows; iRow++ )
	{
		tAttrs.GetBytes ( dRow.Begin(), dRow.GetLengthBytes() );
		DocID_t tDocID = sphGetDocID(pRow);

		///////////////////////////
		// check blobs
		///////////////////////////

		if ( pBlobLocator )
		{
			int64_t iBlobOffset1 = sphGetBlobRowOffset(pRow);
			int64_t iBlobOffset2 = sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );

			if ( iBlobOffset1!=iBlobOffset2 )
				tReporter.Fail ( "blob row locator mismatch (row=" INT64_FMT ", docid=" INT64_FMT ", offset1=" INT64_FMT ", offset2=" INT64_FMT ", rowid=" INT64_FMT " of " INT64_FMT ")",
					iRow, tDocID, iBlobOffset1, iBlobOffset2, iRow, nRows );

			CSphString sError;
			if ( !sphCheckBlobRow ( iBlobOffset1, tBlobs, tSchema, sError ) )
				tReporter.Fail ( "%s at offset " INT64_FMT ", docid=" INT64_FMT ", rowid=" INT64_FMT " of " INT64_FMT, sError.cstr(), iBlobOffset1, tDocID, iRow, nRows );
		}

		///////////////////////////
		// check floats
		///////////////////////////

		ARRAY_FOREACH ( iItem, dFloatItems )
		{
			const DWORD uValue = (DWORD)sphGetRowAttr ( pRow, dFloatItems[ iItem ] );
			const DWORD uExp = ( uValue >> 23 ) & 0xff;
			const DWORD uMantissa = uValue & 0x003fffff;

			// check normalized
			if ( uExp==0 && uMantissa!=0 )
				tReporter.Fail ( "float attribute value is unnormalized (row=" INT64_FMT ", attr=%d, id=" INT64_FMT ", raw=0x%x, value=%f)", 	iRow, iItem, tDocID, uValue, sphDW2F ( uValue ) );

			// check +-inf
			if ( uExp==0xff && uMantissa==0 )
				tReporter.Fail ( "float attribute is infinity (row=" INT64_FMT ", attr=%d, id=" INT64_FMT ", raw=0x%x, value=%f)", iRow, iItem, tDocID, uValue, sphDW2F ( uValue ) );
		}
	}
}


void DebugCheckHelper_c::DebugCheck_DeadRowMap ( int64_t iSizeBytes, int64_t nRows, DebugCheckError_c & tReporter ) const
{
	tReporter.Msg ( "checking dead row map..." );

	int64_t nExpectedEntries = int(( nRows+31 ) / 32);
	int64_t iExpectedSize = nExpectedEntries*sizeof(DWORD);
	if ( iSizeBytes!=iExpectedSize )
		tReporter.Fail ( "unexpected dead row map: " INT64_FMT ", expected: " INT64_FMT " bytes", iSizeBytes, iExpectedSize );
}

//////////////////////////////////////////////////////////////////////////
struct DebugCheckContext_t
{
	CSphAutoreader			m_tDictReader;
	DataReaderFabricPtr_t	m_pDocsReader;
	DataReaderFabricPtr_t	m_pHitsReader;
	CSphAutoreader			m_tSkipsReader;
	CSphAutoreader			m_tDeadRowReader;
	CSphAutoreader			m_tAttrReader;
	CSphAutoreader			m_tBlobReader;
	CSphVector<SphWordID_t> m_dHitlessWords;

	bool					m_bHasBlobs = false;
};


class QueryMvaContainer_c
{
public:
	CSphVector<OpenHash_T<CSphVector<int64_t>, int64_t, HashFunc_Int64_t>*> m_tContainer;

	~QueryMvaContainer_c()
	{
		for ( auto i : m_tContainer )
			delete i;
	}
};


/// this is my actual VLN-compressed phrase index implementation
class CSphIndex_VLN : public CSphIndex, public IndexUpdateHelper_c, public IndexAlterHelper_c, public DebugCheckHelper_c
{
	friend class DiskIndexQwordSetup_c;
	friend class CSphMerger;
	friend class AttrIndexBuilder_c;
	friend struct SphFinalMatchCalc_t;
	friend class KeepAttrs_c;

public:
	explicit					CSphIndex_VLN ( const char* sIndexName, const char * sFilename );
								~CSphIndex_VLN ();

	int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) final;
	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn pfnProgress ) final { m_tProgress.m_fnProgress = pfnProgress; }

	bool				LoadHeader ( const char * sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning );

	void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) final;
	void				DebugDumpDocids ( FILE * fp ) final;
	void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) final;
	void				DebugDumpDict ( FILE * fp ) final;
	void				SetDebugCheck () final;
	int					DebugCheck ( FILE * fp ) final;
	template <class Qword> void		DumpHitlist ( FILE * fp, const char * sKeyword, bool bID );

	bool				Prealloc ( bool bStripPath ) final;
	void				Dealloc () final;
	void				Preread () final;
	void				SetMemorySettings ( const FileAccessSettings_t & tFileAccessSettings ) final;

	void				SetBase ( const char * sNewBase ) final;
	bool				Rename ( const char * sNewBase ) final;

	bool				Lock () final;
	void				Unlock () final;
	void				PostSetup() final {}

	bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const final;
	bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const final;
	 bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const final;
	template <class Qword> bool		DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const;
	bool 				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const final;
	void				GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const final;

	bool				Merge ( CSphIndex * pSource, const CSphVector<CSphFilterSettings> & dFilters, bool bSupressDstDocids ) final;

	template <class QWORDDST, class QWORDSRC>
	static bool					MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, const CSphVector<RowID_t> & dDstRows, const CSphVector<RowID_t> & dSrcRows, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat, CSphIndexProgress & tProgress, volatile bool * pLocalStop );
	static bool					DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress, volatile bool * pLocalStop, bool bSrcSettings, bool bSupressDstDocids );

	int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, bool & bCritical, CSphString & sError, CSphString & sWarning ) final;
	bool				SaveAttributes ( CSphString & sError ) const final;
	DWORD				GetAttributeStatus () const final;

	bool				WriteLookupAndHistograms ( const CSphString & sSPA, const CSphString & sSPT, const CSphString & sSPHI, DWORD uTotalDocs, const AttrIndexBuilder_c & tMinMax, CSphString & sError ) const;

	bool				AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError ) final;
	void				FlushDeadRowMap ( bool bWaitComplete ) const final;
	bool				LoadKillList ( CSphFixedVector<DocID_t> * pKillList, CSphVector<KillListTarget_t> & dTargets, CSphString & sError ) final;
	bool				AlterKillListTarget ( CSphVector<KillListTarget_t> & dTargets, CSphString & sError ) final;
	void				KillExistingDocids ( CSphIndex * pTarget ) final;

	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const final;

	void				SetKeepAttrs ( const CSphString & sKeepAttrs, const StrVec_t & dAttrs ) final { m_sKeepAttrs = sKeepAttrs; m_dKeepAttrs = dAttrs; }
	RowID_t				GetRowidByDocid ( DocID_t tDocID ) const;
	int					Kill ( DocID_t tDocID ) final;
	int					KillMulti ( const DocID_t * pKlist, int iKlistSize ) final;

	const CSphSourceStats &		GetStats () const final { return m_tStats; }
	int64_t *			GetFieldLens() const final { return m_tSettings.m_bIndexFieldLens ? m_dFieldLens.begin() : nullptr; }
	void				GetStatus ( CSphIndexStatus* ) const final;
	bool 				BuildDocList ( SphAttr_t ** ppDocList, int64_t * pCount, CSphString * pError ) const final;

private:
	static const int			MIN_WRITE_BUFFER		= 262144;	///< min write buffer size
	static const int			DEFAULT_WRITE_BUFFER	= 1048576;	///< default write buffer size

private:
	// common stuff
	int								m_iLockFD;
	CSphSourceStats					m_tStats;			///< my stats
	CSphFixedVector<int64_t>		m_dFieldLens;	///< total per-field lengths summed over entire indexed data, in tokens
	CSphString						m_sKeepAttrs;			///< retain attributes of that index reindexing
	StrVec_t						m_dKeepAttrs;

private:
	mutable CSphIndexProgress		m_tProgress;

	bool						LoadHitlessWords ( CSphVector<SphWordID_t> & dHitlessWords );

private:
	int64_t						m_iDocinfo;				///< my docinfo cache size
	int64_t						m_iDocinfoIndex;		///< docinfo "index" entries count (each entry is 2x docinfo rows, for min/max)
	DWORD *						m_pDocinfoIndex;		///< docinfo "index", to accelerate filtering during full-scan (2x rows for each block, and 2x rows for the whole index, 1+m_uDocinfoIndex entries)
	int64_t						m_iMinMaxIndex;			///< stored min/max cache offset (counted in DWORDs)

	// !COMMIT slow setup data
	CSphMappedBuffer<DWORD>		m_tAttr;
	CSphMappedBuffer<BYTE>		m_tBlobAttrs;
	CSphMappedBuffer<BYTE>		m_tSkiplists;		///< (compressed) skiplists data
	CWordlist					m_tWordlist;		///< my wordlist

	DeadRowMap_Disk_c 			m_tDeadRowMap;

	CSphMappedBuffer<BYTE>		m_tDocidLookup;		///< speeds up docid-rowid lookups + used for applying killlist on startup
	LookupReader_c				m_tLookupReader;	///< used by getrowidbydocid

	FileAccessSettings_t		m_tFiles;

	DWORD						m_uVersion;				///< data files version
	volatile bool				m_bPassedRead;
	volatile bool				m_bPassedAlloc;
	bool						m_bIsEmpty;				///< do we have actually indexed documents (m_iTotalDocuments is just fetched documents, not indexed!)
	bool						m_bDebugCheck;

	DWORD						m_uAttrsStatus;

	DataReaderFabricPtr_t		m_pDoclistFile;			///< doclist file
	DataReaderFabricPtr_t		m_pHitlistFile;			///< hitlist file

	HistogramContainer_c *		m_pHistograms {nullptr};

private:
	CSphString					GetIndexFileName ( ESphExt eExt, bool bTemp=false ) const;
	CSphString					GetIndexFileName ( const char * szExt ) const;

	bool						ParsedMultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const XQQuery_t & tXQ, CSphDict * pDict, const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, const SphWordStatChecker_t & tStatDiff ) const;

	void						ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, CSphMatch & tMatch, int iCutoff, bool bReverse, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const;
	bool						MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const;

	template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS>
	void						MatchExtended ( CSphQueryContext * pCtx, const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters, ISphRanker * pRanker, int iTag, int iIndexWeight ) const;

	const CSphRowitem *			FindDocinfo ( DocID_t tDocID ) const;

	const DWORD *				GetDocinfoByRowID ( RowID_t tRowID ) const;
	RowID_t						GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const;

	bool						IsStarDict() const override ;
	void						SetupStarDict ( CSphDictRefPtr_c &pDict ) const;
	void						SetupExactDict ( CSphDictRefPtr_c &pDict ) const;

	bool						RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize, SphOffset_t * pFileSize, CSphBin * pMinBin, SphOffset_t * pSharedOffset );

	bool						SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock );
	bool						CreateHistograms ( const AttrIndexBuilder_c & tMinMax );

	bool						CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer );
	bool						StoreBlobAttrs ( DocID_t tDocId, SphOffset_t & tOffset, BlobRowBuilder_i & tBlobRowBuilderconst, QueryMvaContainer_c & tMvaContainer, BlobSource_i & tSource, bool bForceSource );

private:
	mutable HashCollection_c	m_dHashes;

	bool						JuggleFile ( ESphExt eExt, CSphString & sError, bool bNeedOrigin=true ) const;
	XQNode_t *					ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta * pResult, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags ) const;
	void						PopulateHistograms();

	static void					CreateRowMaps ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, CSphVector<RowID_t> & dSrcRowMap, CSphVector<RowID_t> & dDstRowMap, bool bSupressDstDocids );
	static bool					MergeAttributes ( volatile bool * pLocalStop, const CSphIndex_VLN * pIndex, const CSphVector<RowID_t> & dRowMap, AttrIndexBuilder_c & tMinMax,	CSphWriter & tWriterSPA, BlobRowBuilder_i * pBlobRowBuilder, RowID_t & tResultRowID );

	void						Update_CollectRowPtrs ( UpdateContext_t & tCtx );
	bool						Update_WriteBlobRow ( UpdateContext_t & tCtx, int iUpd, CSphRowitem * pDocinfo, const BYTE * pBlob, int iLength, int nBlobAttrs, bool & bCritical, CSphString & sError ) override;
	void						Update_MinMax ( UpdateContext_t & tCtx );

	bool						Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const override;

	void						DebugCheck_Dictionary ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter );
	void						DebugCheck_Docs ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter );
	void						DebugCheck_KillList ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter ) const;
	void						DebugCheck_BlockIndex ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter );
};


/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/// indexer warning
void sphWarn ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	fprintf ( stdout, "WARNING: " );
	vfprintf ( stdout, sTemplate, ap );
	fprintf ( stdout, "\n" );
	va_end ( ap );
}

//////////////////////////////////////////////////////////////////////////

static int 		g_iIOpsDelay = 0;
static int 		g_iMaxIOSize = 0;
static CSphAtomicL g_tmNextIOTime;

void sphSetThrottling ( int iMaxIOps, int iMaxIOSize )
{
	g_iIOpsDelay = iMaxIOps ? 1000000/iMaxIOps : iMaxIOps;
	g_iMaxIOSize = iMaxIOSize;
}


static inline void ThrottleSleep ()
{
	if ( !g_iIOpsDelay )
		return;

	auto tmTimer = sphMicroTimer ();
	while ( tmTimer < g_tmNextIOTime ) // m.b. >1 sleeps if another thread more lucky
	{
		sphSleepMsec ( ( int ) ( g_tmNextIOTime - tmTimer ) / 1000 );
		tmTimer = sphMicroTimer();
	}
	g_tmNextIOTime = tmTimer + g_iIOpsDelay;
}


bool sphWriteThrottled ( int iFD, const void * pBuf, int64_t iCount, const char * sName, CSphString & sError )
{
	if ( iCount<=0 )
		return true;

	// by default, slice ios by at most 1 GB
	int iChunkSize = ( 1UL<<30 );

	// when there's a sane max_iosize (4K to 1GB), use it
	if ( g_iMaxIOSize>=4096 )
		iChunkSize = Min ( iChunkSize, g_iMaxIOSize );

	CSphIOStats * pIOStats = GetIOStats();

	// while there's data, write it chunk by chunk
	auto * p = (const BYTE*) pBuf;
	while ( iCount )
	{
		// wait for a timely occasion
		ThrottleSleep ();

		// write (and maybe time)
		int64_t tmTimer = 0;
		if ( pIOStats )
			tmTimer = sphMicroTimer();

		auto iToWrite = (int) Min ( iCount, iChunkSize );
		int iWritten = ::write ( iFD, p, iToWrite );

		if ( pIOStats )
		{
			pIOStats->m_iWriteTime += sphMicroTimer() - tmTimer;
			pIOStats->m_iWriteOps++;
			pIOStats->m_iWriteBytes += iWritten;
		}

		// success? rinse, repeat
		if ( iWritten==iToWrite )
		{
			iCount -= iToWrite;
			p += iToWrite;
			continue;
		}

		// failure? report, bailout
		if ( iWritten<0 )
			sError.SetSprintf ( "%s: write error: %s", sName, strerrorm(errno) );
		else
			sError.SetSprintf ( "%s: write error: %d of %d bytes written", sName, iWritten, iToWrite );
		return false;
	}
	return true;
}


static size_t sphReadThrottled ( int iFD, void * pBuf, size_t iCount )
{
	if ( iCount<=0 )
		return iCount;

	auto iStep = g_iMaxIOSize ? Min ( iCount, (size_t)g_iMaxIOSize ) : iCount;
	auto * p = ( BYTE * ) pBuf;
	size_t nBytesToRead = iCount;
	while ( iCount )
	{
		ThrottleSleep();
		auto iChunk = (long) Min ( iCount, iStep );
		auto iRead = sphRead ( iFD, p, iChunk );
		p += iRead;
		iCount -= iRead;
		if ( iRead!=iChunk )
			break;
	}
	return nBytesToRead-iCount; // FIXME? we sure this is under 2gb?
}


void SafeClose ( int & iFD )
{
	if ( iFD>=0 )
		::close ( iFD );
	iFD = -1;
}

//////////////////////////////////////////////////////////////////////////

#if !USE_WINDOWS
char * strlwr ( char * s )
{
	while ( *s )
	{
		*s = tolower ( *s );
		s++;
	}
	return s;
}
#endif


static char * sphStrMacro ( const char * sTemplate, const char * sMacro, int64_t iValue )
{
	// expand macro
	char sExp[32];
	snprintf ( sExp, sizeof(sExp), INT64_FMT, iValue );

	// calc lengths
	int iExp = strlen ( sExp );
	int iMacro = strlen ( sMacro );
	int iDelta = iExp-iMacro;

	// calc result length
	int iRes = strlen ( sTemplate );
	const char * sCur = sTemplate;
	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
	{
		iRes += iDelta;
		sCur++;
	}

	// build result
	char * sRes = new char [ iRes+1 ];
	char * sOut = sRes;
	const char * sLast = sTemplate;
	sCur = sTemplate;

	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
	{
		strncpy ( sOut, sLast, sCur-sLast ); sOut += sCur-sLast;
		strcpy ( sOut, sExp ); sOut += iExp; // NOLINT
		sCur += iMacro;
		sLast = sCur;
	}

	if ( *sLast )
		strcpy ( sOut, sLast ); // NOLINT

	assert ( (int)strlen(sRes)==iRes );
	return sRes;
}


static float sphToFloat ( const char * s )
{
	if ( !s ) return 0.0f;
	return (float)strtod ( s, NULL );
}


static DWORD sphToDword ( const char * s )
{
	if ( !s ) return 0;
	return strtoul ( s, NULL, 10 );
}


static int64_t sphToInt64 ( const char * szNumber, CSphString * pError=nullptr )
{
	if ( !szNumber )
		return 0;

	char * szEndPtr = nullptr;
	errno = 0;

	int64_t iNumber = strtoll ( szNumber, &szEndPtr, 10 );
	if ( pError )
	{
		if ( szNumber==szEndPtr )
			pError->SetSprintf ( "invalid number \"%s\", " INT64_FMT " assumed", szNumber, iNumber );
		else if ( errno==ERANGE && iNumber==LONG_MIN )
			pError->SetSprintf ( "underflow detected \"%s\", " INT64_FMT " assumed", szNumber, iNumber );
		else if ( errno==ERANGE && iNumber==LONG_MAX )
			pError->SetSprintf ( "overflow detected \"%s\", " INT64_FMT " assumed", szNumber, iNumber );
	}

	return iNumber;
}

#if USE_WINDOWS

bool sphLockEx ( int iFile, bool bWait )
{
	HANDLE hHandle = (HANDLE) _get_osfhandle ( iFile );
	if ( hHandle!=INVALID_HANDLE_VALUE )
	{
		OVERLAPPED tOverlapped;
		memset ( &tOverlapped, 0, sizeof ( tOverlapped ) );
		return !!LockFileEx ( hHandle, LOCKFILE_EXCLUSIVE_LOCK | ( bWait ? 0 : LOCKFILE_FAIL_IMMEDIATELY ), 0, 1, 0, &tOverlapped );
	}

	return false;
}

void sphLockUn ( int iFile )
{
	HANDLE hHandle = (HANDLE) _get_osfhandle ( iFile );
	if ( hHandle!=INVALID_HANDLE_VALUE )
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
		return ( fcntl ( iFile, F_SETLK, &tLock )!=-1 );

#if HAVE_F_SETLKW
	return ( fcntl ( iFile, F_SETLKW, &tLock )!=-1 );
#else
	for (;;)
	{
		int iResult = fcntl ( iFile, F_SETLK, &tLock )
		if ( iResult!=-1 )
			return true;
		if ( errno==EACCES || errno==EAGAIN )
			sphSleepMsec(10);
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

	if ( fcntl ( iFile, F_SETLK, &tLock )==-1 )
		sphWarning ( "sphLockUn: failed fcntl. Error: %d '%s'", errno, strerror ( errno ) );

}
#endif


void sphSleepMsec ( int iMsec )
{
	if ( iMsec<0 )
		return;

#if USE_WINDOWS
	Sleep ( iMsec );

#else
	struct timeval tvTimeout;
	tvTimeout.tv_sec = iMsec / 1000; // full seconds
	tvTimeout.tv_usec = ( iMsec % 1000 ) * 1000; // remainder is msec, so *1000 for usec

	select ( 0, NULL, NULL, NULL, &tvTimeout ); // FIXME? could handle EINTR
#endif
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

void SetUnhintedBuffer ( int iReadUnhinted )
{
	if ( iReadUnhinted<=0 )
		iReadUnhinted = DEFAULT_READ_UNHINTED;
	g_iReadUnhinted = Max ( iReadUnhinted, MIN_READ_UNHINTED );
}

// returns correct size even if iBuf is 0
int GetReadBuffer ( int iBuf )
{
	if ( !iBuf )
		return DEFAULT_READ_BUFFER;
	return Max ( iBuf, MIN_READ_BUFFER );
}

bool IsMlock ( FileAccess_e eType ) { return eType==FileAccess_e::MLOCK; }
bool IsOndisk ( FileAccess_e eType ) { return eType==FileAccess_e::FILE || eType==FileAccess_e::MMAP; }

bool FileAccessSettings_t::operator== ( const FileAccessSettings_t & tOther ) const
{
	return ( m_eAttr==tOther.m_eAttr && m_eBlob==tOther.m_eBlob && m_eDoclist==tOther.m_eDoclist && m_eHitlist==tOther.m_eHitlist
		&& m_iReadBufferDocList==tOther.m_iReadBufferDocList && m_iReadBufferHitList==tOther.m_iReadBufferHitList );
}

bool FileAccessSettings_t::operator!= ( const FileAccessSettings_t & tOther ) const
{
	return !operator==( tOther );
}


//////////////////////////////////////////////////////////////////////////
// DOCINFO
//////////////////////////////////////////////////////////////////////////

const BYTE * CSphMatch::FetchAttrData ( const CSphAttrLocator & tLoc, const BYTE * pPool, int & iLengthBytes ) const
{
	if ( tLoc.IsBlobAttr() )
		return sphGetBlobAttr ( *this, tLoc, pPool, iLengthBytes );
	else
	{
		const BYTE * pResult {nullptr};
		iLengthBytes = sphUnpackPtrAttr ( (const BYTE*)GetAttr(tLoc), &pResult );
		return pResult;
	}
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZING EXCEPTIONS
/////////////////////////////////////////////////////////////////////////////

/// exceptions trie, stored in a tidy simple blob
/// we serialize each trie node as follows:
///
/// int result_offset, 0 if no output mapping
/// BYTE num_bytes, 0 if no further valid bytes can be accepted
/// BYTE values[num_bytes], known accepted byte values
/// BYTE offsets[num_bytes], and the respective next node offsets
///
/// output mappings themselves are serialized just after the nodes,
/// as plain old ASCIIZ strings
class ExceptionsTrie_c
{
	friend class		ExceptionsTrieGen_c;

protected:
	int					m_dFirst[256];	///< table to speedup 1st byte lookup
	CSphVector<BYTE>	m_dData;		///< data blob
	int					m_iCount;		///< number of exceptions
	int					m_iMappings;	///< offset where the nodes end, and output mappings start

public:
	const BYTE * GetMapping ( int i ) const
	{
		assert ( i>=0 && i<m_iMappings );
		int p = *(int*)&m_dData[i];
		if ( !p )
			return NULL;
		assert ( p>=m_iMappings && p<m_dData.GetLength() );
		return &m_dData[p];
	}

	int GetFirst ( BYTE v ) const
	{
		return m_dFirst[v];
	}

	int GetNext ( int i, BYTE v ) const
	{
		assert ( i>=0 && i<m_iMappings );
		if ( i==0 )
			return m_dFirst[v];
		const BYTE * p = &m_dData[i];
		int n = p[4];
		p += 5;
		for ( i=0; i<n; i++ )
			if ( p[i]==v )
				return *(int*)&p [ n + 4*i ]; // FIXME? unaligned
		return -1;
	}

public:
	void Export ( CSphWriter & w ) const
	{
		CSphVector<BYTE> dPrefix;
		int iCount = 0;

		w.PutDword ( m_iCount );
		Export ( w, dPrefix, 0, &iCount );
		assert ( iCount==m_iCount );
	}

protected:
	void Export ( CSphWriter & w, CSphVector<BYTE> & dPrefix, int iNode, int * pCount ) const
	{
		assert ( iNode>=0 && iNode<m_iMappings );
		const BYTE * p = &m_dData[iNode];

		int iTo = *(int*)p;
		if ( iTo>0 )
		{
			CSphString s;
			const char * sTo = (char*)&m_dData[iTo];
			s.SetBinary ( (char*)dPrefix.Begin(), dPrefix.GetLength() );
			s.SetSprintf ( "%s => %s\n", s.cstr(), sTo );
			w.PutString ( s.cstr() );
			(*pCount)++;
		}

		int n = p[4];
		if ( n==0 )
			return;

		p += 5;
		for ( int i=0; i<n; i++ )
		{
			dPrefix.Add ( p[i] );
			Export ( w, dPrefix, *(int*)&p[n+4*i], pCount );
			dPrefix.Pop();
		}
	}
};


/// intermediate exceptions trie node
/// only used by ExceptionsTrieGen_c, while building a blob
class ExceptionsTrieNode_c
{
	friend class						ExceptionsTrieGen_c;

protected:
	struct Entry_t
	{
		BYTE					m_uValue;
		ExceptionsTrieNode_c *	m_pKid;
	};

	CSphString					m_sTo;		///< output mapping for current prefix, if any
	CSphVector<Entry_t>			m_dKids;	///< known and accepted incoming byte values

public:
	~ExceptionsTrieNode_c()
	{
		ARRAY_FOREACH ( i, m_dKids )
			SafeDelete ( m_dKids[i].m_pKid );
	}

	/// returns false on a duplicate "from" part, or true on success
	bool AddMapping ( const BYTE * sFrom, const BYTE * sTo )
	{
		// no more bytes to consume? this is our output mapping then
		if ( !*sFrom )
		{
			if ( !m_sTo.IsEmpty() )
				return false;
			m_sTo = (const char*)sTo;
			return true;
		}

		int i;
		for ( i=0; i<m_dKids.GetLength(); i++ )
			if ( m_dKids[i].m_uValue==*sFrom )
				break;
		if ( i==m_dKids.GetLength() )
		{
			Entry_t & t = m_dKids.Add();
			t.m_uValue = *sFrom;
			t.m_pKid = new ExceptionsTrieNode_c();
		}
		return m_dKids[i].m_pKid->AddMapping ( sFrom+1, sTo );
	}
};


/// exceptions trie builder
/// plain old text mappings in, nice useful trie out
class ExceptionsTrieGen_c
{
protected:
	ExceptionsTrieNode_c *	m_pRoot;
	int						m_iCount;

public:
	ExceptionsTrieGen_c()
	{
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
	}

	~ExceptionsTrieGen_c()
	{
		SafeDelete ( m_pRoot );
	}

	/// trims left/right whitespace, folds inner whitespace
	void FoldSpace ( char * s ) const
	{
		// skip leading spaces
		char * d = s;
		while ( *s && sphIsSpace(*s) )
			s++;

		// handle degenerate (empty string) case
		if ( !*s )
		{
			*d = '\0';
			return;
		}

		while ( *s )
		{
			// copy another token, add exactly 1 space after it, and skip whitespace
			while ( *s && !sphIsSpace(*s) )
				*d++ = *s++;
			*d++ = ' ';
			while ( sphIsSpace(*s) )
				s++;
		}

		// replace that last space that we added
		d[-1] = '\0';
	}

	bool ParseLine ( char * sBuffer, CSphString & sError )
	{
		#define LOC_ERR(_arg) { sError = _arg; return false; }
		assert ( m_pRoot );

		// extract map-from and map-to parts
		char * sSplit = strstr ( sBuffer, "=>" );
		if ( !sSplit )
			LOC_ERR ( "mapping token (=>) not found" );

		char * sFrom = sBuffer;
		char * sTo = sSplit + 2; // skip "=>"
		*sSplit = '\0';

		// trim map-from, map-to
		FoldSpace ( sFrom );
		FoldSpace ( sTo );
		if ( !*sFrom )
			LOC_ERR ( "empty map-from part" );
		if ( !*sTo )
			LOC_ERR ( "empty map-to part" );
		if ( (int)strlen(sFrom) > MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );
		if ( (int)strlen(sTo)>MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );

		// all parsed ok; add it!
		if ( m_pRoot->AddMapping ( (BYTE*)sFrom, (BYTE*)sTo ) )
			m_iCount++;
		else
			LOC_ERR ( "duplicate map-from part" );

		return true;
		#undef LOC_ERR
	}

	ExceptionsTrie_c * Build()
	{
		if ( !m_pRoot || !m_pRoot->m_sTo.IsEmpty() || m_pRoot->m_dKids.GetLength()==0 )
			return NULL;

		ExceptionsTrie_c * pRes = new ExceptionsTrie_c();
		pRes->m_iCount = m_iCount;

		// save the nodes themselves
		CSphVector<BYTE> dMappings;
		SaveNode ( pRes, m_pRoot, dMappings );

		// append and fixup output mappings
		CSphVector<BYTE> & d = pRes->m_dData;
		pRes->m_iMappings = d.GetLength();
		d.Append ( dMappings );

		BYTE * p = d.Begin();
		BYTE * pMax = p + pRes->m_iMappings;
		while ( p<pMax )
		{
			// fixup offset in the current node, if needed
			int * pOff = (int*)p; // FIXME? unaligned
			if ( (*pOff)<0 )
				*pOff = 0; // convert -1 to 0 for non-outputs
			else
				(*pOff) += pRes->m_iMappings; // fixup offsets for outputs

			// proceed to the next node
			int n = p[4];
			p += 5 + 5*n;
		}
		assert ( p==pMax );

		// build the speedup table for the very 1st byte
		for ( int i=0; i<256; i++ )
			pRes->m_dFirst[i] = -1;
		int n = d[4];
		for ( int i=0; i<n; i++ )
			pRes->m_dFirst [ d[5+i] ] = *(int*)&pRes->m_dData [ 5+n+4*i ];

		SafeDelete ( m_pRoot );
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
		return pRes;
	}

protected:
	void SaveInt ( CSphVector<BYTE> & v, int p, int x )
	{
#if USE_LITTLE_ENDIAN
		v[p] = x & 0xff;
		v[p+1] = (x>>8) & 0xff;
		v[p+2] = (x>>16) & 0xff;
		v[p+3] = (x>>24) & 0xff;
#else
		v[p] = (x>>24) & 0xff;
		v[p+1] = (x>>16) & 0xff;
		v[p+2] = (x>>8) & 0xff;
		v[p+3] = x & 0xff;
#endif
	}

	int SaveNode ( ExceptionsTrie_c * pRes, ExceptionsTrieNode_c * pNode, CSphVector<BYTE> & dMappings )
	{
		CSphVector<BYTE> & d = pRes->m_dData; // shortcut

		// remember the start node offset
		int iRes = d.GetLength();
		int n = pNode->m_dKids.GetLength();
		assert (!( pNode->m_sTo.IsEmpty() && n==0 ));

		// save offset into dMappings, or temporary (!) save -1 if there is no output mapping
		// note that we will fixup those -1's to 0's afterwards
		int iOff = -1;
		if ( !pNode->m_sTo.IsEmpty() )
		{
			iOff = dMappings.GetLength();
			int iLen = pNode->m_sTo.Length();
			memcpy ( dMappings.AddN ( iLen+1 ), pNode->m_sTo.cstr(), iLen+1 );
		}
		d.AddN(4);
		SaveInt ( d, d.GetLength()-4, iOff );

		// sort children nodes by value
		pNode->m_dKids.Sort ( bind ( &ExceptionsTrieNode_c::Entry_t::m_uValue ) );

		// save num_values, and values[]
		d.Add ( (BYTE)n );
		ARRAY_FOREACH ( i, pNode->m_dKids )
			d.Add ( pNode->m_dKids[i].m_uValue );

		// save offsets[], and the respective child nodes
		int p = d.GetLength();
		d.AddN ( 4*n );
		for ( int i=0; i<n; i++, p+=4 )
			SaveInt ( d, p, SaveNode ( pRes, pNode->m_dKids[i].m_pKid, dMappings ) );
		assert ( p==iRes+5+5*n );

		// done!
		return iRes;
	}
};

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

inline int sphUTF8Decode ( const BYTE * & pBuf ); // forward ref for GCC
inline int sphUTF8Encode ( BYTE * pBuf, int iCode ); // forward ref for GCC


class CSphTokenizerBase : public ISphTokenizer
{
protected:
	~CSphTokenizerBase ();

public:
	CSphTokenizerBase();

	bool			SetCaseFolding ( const char * sConfig, CSphString & sError ) final;
	bool			LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, CSphString & sError ) final;
	void			WriteSynonyms ( CSphWriter & tWriter ) const final;
	void			CloneBase ( const CSphTokenizerBase * pFrom, ESphTokenizerClone eMode );

	const char *	GetTokenStart () const final	{ return (const char *) m_pTokenStart; }
	const char *	GetTokenEnd () const final		{ return (const char *) m_pTokenEnd; }
	const char *	GetBufferPtr () const final		{ return (const char *) m_pCur; }
	const char *	GetBufferEnd () const final		{ return (const char *) m_pBufferMax; }
	void			SetBufferPtr ( const char * sNewPtr ) final;
	uint64_t		GetSettingsFNV () const final;

	bool			SetBlendChars ( const char * sConfig, CSphString & sError ) final;
	bool			WasTokenMultiformDestination ( bool &, int & ) const final { return false; }

public:
	// lightweight clones must impose a lockdown on some methods
	// (specifically those that change the lowercaser data table)

	void AddPlainChar ( char c ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::AddPlainChar ( c );
	}

	void AddSpecials ( const char * sSpecials ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::AddSpecials ( sSpecials );
	}

	void Setup ( const CSphTokenizerSettings & tSettings ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::Setup ( tSettings );
	}

	bool RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		return ISphTokenizer::RemapCharacters ( sConfig, uFlags, sSource, bCanRemap, sError );
	}

protected:
	bool	BlendAdjust ( const BYTE * pPosition );
	int		CodepointArbitrationI ( int iCodepoint );
	int		CodepointArbitrationQ ( int iCodepoint, bool bWasEscaped, BYTE uNextByte );

protected:
	const BYTE *		m_pBuffer = nullptr;				///< my buffer
	const BYTE *		m_pBufferMax = nullptr;				///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	const BYTE *		m_pCur = nullptr;					///< current position
	const BYTE *		m_pTokenStart = nullptr;			///< last token start point
	const BYTE *		m_pTokenEnd = nullptr;				///< last token end point

	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< folded token accumulator
	BYTE *				m_pAccum = nullptr;					///< current accumulator position
	int					m_iAccum = 0;						///< boundary token size

	BYTE				m_sAccumBlend [ 3*SPH_MAX_WORD_LEN+3 ];	///< blend-acc, an accumulator copy for additional blended variants
	int					m_iBlendNormalStart = 0;			///< points to first normal char in the accumulators (might be NULL)
	int					m_iBlendNormalEnd = 0;				///< points just past (!) last normal char in the accumulators (might be NULL)

	ExceptionsTrie_c *	m_pExc = nullptr;					///< exceptions trie, if any

	bool				m_bHasBlend = false;
	const BYTE *		m_pBlendStart = nullptr;
	const BYTE *		m_pBlendEnd = nullptr;

	ESphTokenizerClone m_eMode { SPH_CLONE_INDEX };
};


/// methods that get specialized with regards to charset type
/// aka GetCodepoint() decoder and everything that depends on it
class CSphTokenizerBase2 : public CSphTokenizerBase
{
protected:
	/// get codepoint
	inline int GetCodepoint ()
	{
		while ( m_pCur<m_pBufferMax )
		{
			int iCode = sphUTF8Decode ( m_pCur );
			if ( iCode>=0 )
				return iCode; // successful decode
		}
		return -1; // eof
	}

	/// accum codepoint
	inline void AccumCodepoint ( int iCode )
	{
		assert ( iCode>0 );
		assert ( m_iAccum>=0 );

		// throw away everything which is over the token size
		bool bFit = ( m_iAccum<SPH_MAX_WORD_LEN );
		bFit &= ( m_pAccum-m_sAccum+SPH_MAX_UTF8_BYTES<=(int)sizeof(m_sAccum));

		if ( bFit )
		{
			m_pAccum += sphUTF8Encode ( m_pAccum, iCode );
			assert ( m_pAccum>=m_sAccum && m_pAccum<m_sAccum+sizeof(m_sAccum) );
			m_iAccum++;
		}
	}

protected:
	BYTE *	GetBlendedVariant ();
	bool	CheckException ( const BYTE * pStart, const BYTE * pCur, bool bQueryMode );

	template < bool IS_QUERY, bool IS_BLEND >
	BYTE *	DoGetToken();

	void	FlushAccum ();

public:
	int		SkipBlended () final;
};


/// UTF-8 tokenizer
template < bool IS_QUERY >
class CSphTokenizer_UTF8 : public CSphTokenizerBase2
{
public:
						CSphTokenizer_UTF8 ();
	void				SetBuffer ( const BYTE * sBuffer, int iLength ) final;
	BYTE *				GetToken () override;
	ISphTokenizer *		Clone ( ESphTokenizerClone eMode ) const final;
	int					GetCodepointLength ( int iCode ) const final;
	int					GetMaxCodepointLength () const final { return m_tLC.GetMaxCodepointLength(); }
};


/// UTF-8 tokenizer with n-grams
template < bool IS_QUERY >
class CSphTokenizer_UTF8Ngram : public CSphTokenizer_UTF8<IS_QUERY>
{
public:
	bool		SetNgramChars ( const char * sConfig, CSphString & sError ) final;
	void		SetNgramLen ( int iLen ) final;
	BYTE *		GetToken () final;

protected:
	int			m_iNgramLen = 1;
};


struct CSphNormalForm
{
	CSphString				m_sForm;
	int						m_iLengthCP;
};

struct CSphMultiform
{
	int								m_iFileId;
	CSphTightVector<CSphNormalForm>	m_dNormalForm;
	CSphTightVector<CSphString>		m_dTokens;
};


struct CSphMultiforms
{
	int							m_iMinTokens;
	int							m_iMaxTokens;
	CSphVector<CSphMultiform*>	m_pForms;		// OPTIMIZE? blobify?
};


struct CSphMultiformContainer
{
							CSphMultiformContainer () : m_iMaxTokens ( 0 ) {}

	int						m_iMaxTokens;
	typedef CSphOrderedHash < CSphMultiforms *, CSphString, CSphStrHashFunc, 131072 > CSphMultiformHash;
	CSphMultiformHash	m_Hash;
};


/// token filter for multiforms support
class CSphMultiformTokenizer : public CSphTokenFilter
{
	using Base = CSphTokenFilter;
public:
	CSphMultiformTokenizer ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer );


public:
	void	SetBuffer ( const BYTE * sBuffer, int iLength ) final;
	BYTE *	GetToken () final;
	void	EnableTokenizedMultiformTracking () final	{ m_bBuildMultiform = true; }
	int		GetLastTokenLen () const final				{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iTokenLen : Base::GetLastTokenLen(); }
	bool	GetBoundary () final						{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBoundary : Base::GetBoundary(); }
	bool	WasTokenSpecial () final					{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bSpecial : Base::WasTokenSpecial(); }
	int		GetOvershortCount () final					{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iOvershortCount : Base::GetOvershortCount(); }
	BYTE *	GetTokenizedMultiform () final				{ return m_sTokenizedMultiform[0] ? m_sTokenizedMultiform : nullptr; }
	bool	TokenIsBlended () const final				{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlended : Base::TokenIsBlended(); }
	bool	TokenIsBlendedPart () const final			{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlendedPart : Base::TokenIsBlendedPart(); }
	int		SkipBlended () final;

public:
	ISphTokenizer *			Clone ( ESphTokenizerClone eMode ) const final;
	const char *			GetTokenStart () const final	{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenStart : Base::GetTokenStart(); }
	const char *			GetTokenEnd () const final		{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenEnd : Base::GetTokenEnd(); }
	const char *			GetBufferPtr () const final		{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_pBufferPtr : Base::GetBufferPtr(); }
	void					SetBufferPtr ( const char * sNewPtr ) final;
	uint64_t				GetSettingsFNV () const final;
	bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const final;

private:
	const CSphMultiformContainer *	m_pMultiWordforms;
	int								m_iStart = 0;
	int								m_iOutputPending = -1;
	const CSphMultiform *			m_pCurrentForm = nullptr;

	bool				m_bBuildMultiform = false;
	BYTE				m_sTokenizedMultiform [ 3*SPH_MAX_WORD_LEN+4 ];

	CSphVector<StoredToken_t>		m_dStoredTokens;
};


/// token filter for bigram indexing
///
/// passes tokens through until an eligible pair is found
/// then buffers and returns that pair as a blended token
/// then returns the first token as a regular one
/// then pops the first one and cycles again
///
/// pair (aka bigram) eligibility depends on bigram_index value
/// "all" means that all token pairs gets indexed
/// "first_freq" means that 1st token must be from bigram_freq_words
/// "both_freq" means that both tokens must be from bigram_freq_words
class CSphBigramTokenizer : public CSphTokenFilter
{
protected:
	enum
	{
		BIGRAM_CLEAN,	///< clean slate, nothing accumulated
		BIGRAM_PAIR,	///< just returned a pair from m_sBuf, and m_iFirst/m_pSecond are correct
		BIGRAM_FIRST	///< just returned a first token from m_sBuf, so m_iFirst/m_pSecond are still good
	}		m_eState = BIGRAM_CLEAN;
	BYTE	m_sBuf [ MAX_KEYWORD_BYTES ];	///< pair buffer
	BYTE *	m_pSecond = nullptr;			///< second token pointer
	int		m_iFirst = 0;					///< first token length, bytes

	ESphBigram			m_eMode;			///< bigram indexing mode
	int					m_iMaxLen = 0;			///< max bigram_freq_words length
	int					m_dWordsHash[256];	///< offsets into m_dWords hashed by 1st byte
	CSphVector<BYTE>	m_dWords;			///< case-folded, sorted bigram_freq_words

public:
	CSphBigramTokenizer ( ISphTokenizer * pTok, ESphBigram eMode, StrVec_t & dWords )
		: CSphTokenFilter ( pTok )
	{
		assert ( pTok );
		assert ( eMode!=SPH_BIGRAM_NONE );
		assert ( eMode==SPH_BIGRAM_ALL || dWords.GetLength() );

		m_sBuf[0] = 0;
		memset ( m_dWordsHash, 0, sizeof(m_dWordsHash) );

		m_eMode = eMode;

		// only keep unique, real, short enough words
		dWords.Uniq();
		ARRAY_FOREACH ( i, dWords )
		{
			int iLen = Min ( dWords[i].Length(), 255 );
			if ( !iLen )
				continue;
			m_iMaxLen = Max ( m_iMaxLen, iLen );

			// hash word blocks by the first letter
			BYTE uFirst = *(BYTE*)( dWords[i].cstr() );
			if ( !m_dWordsHash [ uFirst ] )
			{
				m_dWords.Add ( 0 ); // end marker for the previous block
				m_dWordsHash [ uFirst ] = m_dWords.GetLength(); // hash new block
			}

			// store that word
			int iPos = m_dWords.GetLength();
			m_dWords.Resize ( iPos+iLen+1 );

			m_dWords[iPos] = (BYTE)iLen;
			memcpy ( &m_dWords [ iPos+1 ], dWords[i].cstr(), iLen );
		}
		m_dWords.Add ( 0 );
	}

	CSphBigramTokenizer ( ISphTokenizer * pTok, const CSphBigramTokenizer * pBase )
		: CSphTokenFilter ( pTok )
	{
		m_sBuf[0] = 0;
		m_eMode = pBase->m_eMode;
		m_iMaxLen = pBase->m_iMaxLen;
		memcpy ( m_dWordsHash, pBase->m_dWordsHash, sizeof(m_dWordsHash) );
		m_dWords = pBase->m_dWords;
	}

	ISphTokenizer * Clone ( ESphTokenizerClone eMode ) const final
	{
		return new CSphBigramTokenizer ( ISphTokenizerRefPtr_c ( m_pTokenizer->Clone ( eMode ) ), this );
	}


	bool TokenIsBlended() const final
	{
		if ( m_eState==BIGRAM_PAIR )
			return true;
		if ( m_eState==BIGRAM_FIRST )
			return false;
		return m_pTokenizer->TokenIsBlended();
	}

	bool IsFreq ( int iLen, BYTE * sWord )
	{
		// early check
		if ( iLen>m_iMaxLen )
			return false;

		// hash lookup, then linear scan
		int iPos = m_dWordsHash [ *sWord ];
		if ( !iPos )
			return false;
		while ( m_dWords[iPos] )
		{
			if ( m_dWords[iPos]==iLen && !memcmp ( sWord, &m_dWords[iPos+1], iLen ) )
				break;
			iPos += 1+m_dWords[iPos];
		}
		return m_dWords[iPos]!=0;
	}

	BYTE * GetToken() final
	{
		if ( m_eState==BIGRAM_FIRST || m_eState==BIGRAM_CLEAN )
		{
			BYTE * pFirst;
			if ( m_eState==BIGRAM_FIRST )
			{
				// first out, clean slate again, actually
				// and second will now become our next first
				assert ( m_pSecond );
				m_eState = BIGRAM_CLEAN;
				pFirst = m_pSecond;
				m_pSecond = NULL;
			} else
			{
				// just clean slate
				// assure we're, well, clean
				assert ( !m_pSecond );
				pFirst = CSphTokenFilter::GetToken();
			}

			// clean slate
			// get first non-blended token
			if ( !pFirst )
				return NULL;

			// pass through blended
			// could handle them as first too, but.. cumbersome
			if ( CSphTokenFilter::TokenIsBlended() )
				return pFirst;

			// check pair
			// in first_freq and both_freq modes, 1st token must be listed
			m_iFirst = strlen ( (const char*)pFirst );
			if ( m_eMode!=SPH_BIGRAM_ALL && !IsFreq ( m_iFirst, pFirst ) )
					return pFirst;

			// copy it
			// subsequent calls can and will override token accumulator
			memcpy ( m_sBuf, pFirst, m_iFirst+1 );

			// grow a pair!
			// get a second one (lookahead, in a sense)
			BYTE * pSecond = CSphTokenFilter::GetToken();

			// eof? oi
			if ( !pSecond )
				return m_sBuf;

			// got a pair!
			// check combined length
			m_pSecond = pSecond;
			int iSecond = strlen ( (const char*)pSecond );
			if ( m_iFirst+iSecond+1 > SPH_MAX_WORD_LEN )
			{
				// too long pair
				// return first token as is
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// check pair
			// in freq2 mode, both tokens must be listed
			if ( m_eMode==SPH_BIGRAM_BOTHFREQ && !IsFreq ( iSecond, m_pSecond ) )
			{
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// ok, this is a eligible pair
			// begin with returning first+second pair (as blended)
			m_eState = BIGRAM_PAIR;
			m_sBuf [ m_iFirst ] = MAGIC_WORD_BIGRAM;
			assert ( m_iFirst + strlen ( (const char*)pSecond ) < sizeof(m_sBuf) );
			strcpy ( (char*)m_sBuf+m_iFirst+1, (const char*)pSecond ); //NOLINT
			return m_sBuf;

		} else if ( m_eState==BIGRAM_PAIR )
		{
			// pair (aka bigram) out, return first token as a regular token
			m_eState = BIGRAM_FIRST;
			m_sBuf [ m_iFirst ] = 0;
			return m_sBuf;
		}

		assert ( 0 && "unhandled bigram tokenizer internal state" );
		return NULL;
	}

	uint64_t GetSettingsFNV () const final
	{
		uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
		uHash = sphFNV64 ( m_dWords.Begin(), m_dWords.GetLength(), uHash );
		return uHash;
	}
};

/////////////////////////////////////////////////////////////////////////////

void FillStoredTokenInfo ( StoredToken_t & tToken, const BYTE * sToken, ISphTokenizer * pTokenizer )
{
	assert ( sToken && pTokenizer );
	strncpy ( (char *)tToken.m_sToken, (const char *)sToken, sizeof(tToken.m_sToken)-1 );
	tToken.m_szTokenStart = pTokenizer->GetTokenStart ();
	tToken.m_szTokenEnd = pTokenizer->GetTokenEnd ();
	tToken.m_iOvershortCount = pTokenizer->GetOvershortCount ();
	tToken.m_iTokenLen = pTokenizer->GetLastTokenLen ();
	tToken.m_pBufferPtr = pTokenizer->GetBufferPtr ();
	tToken.m_pBufferEnd = pTokenizer->GetBufferEnd();
	tToken.m_bBoundary = pTokenizer->GetBoundary ();
	tToken.m_bSpecial = pTokenizer->WasTokenSpecial ();
	tToken.m_bBlended = pTokenizer->TokenIsBlended();
	tToken.m_bBlendedPart = pTokenizer->TokenIsBlendedPart();
}


//////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphCreateUTF8Tokenizer ()
{
	return new CSphTokenizer_UTF8<false> ();
}

ISphTokenizer * sphCreateUTF8NgramTokenizer ()
{
	return new CSphTokenizer_UTF8Ngram<false> ();
}

/////////////////////////////////////////////////////////////////////////////

enum
{
	MASK_CODEPOINT			= 0x00ffffffUL,	// mask off codepoint flags
	MASK_FLAGS				= 0xff000000UL, // mask off codepoint value
	FLAG_CODEPOINT_SPECIAL	= 0x01000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL		= 0x02000000UL,	// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM	= 0x04000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_BOUNDARY	= 0x10000000UL,	// this codepoint is phrase boundary
	FLAG_CODEPOINT_IGNORE	= 0x20000000UL,	// this codepoint is ignored
	FLAG_CODEPOINT_BLEND	= 0x40000000UL	// this codepoint is "blended" (indexed both as a character, and as a separator)
};

void CSphLowercaser::Reset()
{
	SafeDeleteArray ( m_pData );
	m_pData = new int [ CHUNK_SIZE ];
	memset ( m_pData, 0, CHUNK_SIZE*sizeof(int) ); // NOLINT sizeof(int)
	m_iChunks = 1;
	m_pChunk[0] = m_pData; // chunk 0 must always be allocated, for utf-8 tokenizer shortcut to work
	for ( int i=1; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = NULL;
}


CSphLowercaser::~CSphLowercaser ()
{
	SafeDeleteArray ( m_pData );
}


void CSphLowercaser::SetRemap ( const CSphLowercaser * pLC )
{
	if ( !pLC )
		return;

	SafeDeleteArray ( m_pData );

	m_iChunks = pLC->m_iChunks;
	m_pData = new int [ m_iChunks*CHUNK_SIZE ];
	memcpy ( m_pData, pLC->m_pData, sizeof(int)*m_iChunks*CHUNK_SIZE ); // NOLINT sizeof(int)

	for ( int i=0; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = pLC->m_pChunk[i]
			? pLC->m_pChunk[i] - pLC->m_pData + m_pData
			: NULL;
}


void CSphLowercaser::AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags )
{
	if ( !dRemaps.GetLength() )
		return;

	// build new chunks map
	// 0 means "was unused"
	// 1 means "was used"
	// 2 means "is used now"
	int dUsed [ CHUNK_COUNT ];
	for ( int i=0; i<CHUNK_COUNT; i++ )
		dUsed[i] = m_pChunk[i] ? 1 : 0;

	int iNewChunks = m_iChunks;

	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & tRemap = dRemaps[i];

#define LOC_CHECK_RANGE(_a) assert ( (_a)>=0 && (_a)<MAX_CODE );
		LOC_CHECK_RANGE ( tRemap.m_iStart );
		LOC_CHECK_RANGE ( tRemap.m_iEnd );
		LOC_CHECK_RANGE ( tRemap.m_iRemapStart );
		LOC_CHECK_RANGE ( tRemap.m_iRemapStart + tRemap.m_iEnd - tRemap.m_iStart );
#undef LOC_CHECK_RANGE

		for ( int iChunk=( tRemap.m_iStart >> CHUNK_BITS ); iChunk<=( tRemap.m_iEnd >> CHUNK_BITS ); iChunk++ )
			if ( dUsed[iChunk]==0 )
			{
				dUsed[iChunk] = 2;
				iNewChunks++;
			}
	}

	// alloc new tables and copy, if necessary
	if ( iNewChunks>m_iChunks )
	{
		int * pData = new int [ iNewChunks*CHUNK_SIZE ];
		memset ( pData, 0, sizeof(int)*iNewChunks*CHUNK_SIZE ); // NOLINT sizeof(int)

		int * pChunk = pData;
		for ( int i=0; i<CHUNK_COUNT; i++ )
		{
			int * pOldChunk = m_pChunk[i];

			// build new ptr
			if ( dUsed[i] )
			{
				m_pChunk[i] = pChunk;
				pChunk += CHUNK_SIZE;
			}

			// copy old data
			if ( dUsed[i]==1 )
				memcpy ( m_pChunk[i], pOldChunk, sizeof(int)*CHUNK_SIZE ); // NOLINT sizeof(int)
		}
		assert ( pChunk-pData==iNewChunks*CHUNK_SIZE );

		SafeDeleteArray ( m_pData );
		m_pData = pData;
		m_iChunks = iNewChunks;
	}

	// fill new stuff
	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & tRemap = dRemaps[i];

		DWORD iRemapped = tRemap.m_iRemapStart;
		for ( int j=tRemap.m_iStart; j<=tRemap.m_iEnd; j++, iRemapped++ )
		{
			assert ( m_pChunk [ j >> CHUNK_BITS ] );
			int & iCodepoint = m_pChunk [ j >> CHUNK_BITS ] [ j & CHUNK_MASK ];
			bool bWordPart = ( iCodepoint & MASK_CODEPOINT )!=0;
			int iNew = iRemapped | uFlags | ( iCodepoint & MASK_FLAGS );
			if ( bWordPart && ( uFlags & FLAG_CODEPOINT_SPECIAL ) )
				iNew |= FLAG_CODEPOINT_DUAL;
			iCodepoint = iNew;
		}
	}
}


void CSphLowercaser::AddSpecials ( const char * sSpecials )
{
	assert ( sSpecials );
	int iSpecials = strlen(sSpecials);

	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Resize ( iSpecials );
	ARRAY_FOREACH ( i, dRemaps )
		dRemaps[i].m_iStart = dRemaps[i].m_iEnd = dRemaps[i].m_iRemapStart = sSpecials[i];

	AddRemaps ( dRemaps, FLAG_CODEPOINT_SPECIAL );
}

CSphLowercaser & CSphLowercaser::operator = ( const CSphLowercaser & rhs )
{
	SetRemap ( &rhs );
	return * this;
}

uint64_t CSphLowercaser::GetFNV () const
{
	int iLen = ( sizeof(int) * m_iChunks * CHUNK_SIZE ) / sizeof(BYTE); // NOLINT
	return sphFNV64 ( m_pData, iLen );
}

int CSphLowercaser::GetMaxCodepointLength () const
{
	int iMax = 0;
	for ( int iChunk=0; iChunk<CHUNK_COUNT; iChunk++ )
	{
		int * pChunk = m_pChunk[iChunk];
		if ( !pChunk )
			continue;

		int * pMax = pChunk + CHUNK_SIZE;
		while ( pChunk<pMax )
		{
			int iCode = *pChunk++ & MASK_CODEPOINT;
			iMax = Max ( iMax, iCode );
		}
	}
	if ( iMax<0x80 )
		return 1;
	if ( iMax<0x800 )
		return 2;
	return 3; // actually, 4 once we hit 0x10000
}

/////////////////////////////////////////////////////////////////////////////

const char * CSphCharsetDefinitionParser::GetLastError ()
{
	return m_bError ? m_sError : NULL;
}


bool CSphCharsetDefinitionParser::IsEof ()
{
	return ( *m_pCurrent )==0;
}


bool CSphCharsetDefinitionParser::CheckEof ()
{
	if ( IsEof() )
	{
		Error ( "unexpected end of line" );
		return true;
	} else
	{
		return false;
	}
}


bool CSphCharsetDefinitionParser::Error ( const char * sMessage )
{
	char sErrorBuffer[32];
	strncpy ( sErrorBuffer, m_pCurrent, sizeof(sErrorBuffer) );
	sErrorBuffer [ sizeof(sErrorBuffer)-1 ] = '\0';

	snprintf ( m_sError, sizeof(m_sError), "%s near '%s'",
		sMessage, sErrorBuffer );
	m_sError [ sizeof(m_sError)-1 ] = '\0';

	m_bError = true;
	return false;
}


int CSphCharsetDefinitionParser::HexDigit ( int c )
{
	if ( c>='0' && c<='9' ) return c-'0';
	if ( c>='a' && c<='f' ) return c-'a'+10;
	if ( c>='A' && c<='F' ) return c-'A'+10;
	return 0;
}


void CSphCharsetDefinitionParser::SkipSpaces ()
{
	while ( ( *m_pCurrent ) && isspace ( (BYTE)*m_pCurrent ) )
		m_pCurrent++;
}


int CSphCharsetDefinitionParser::ParseCharsetCode ()
{
	const char * p = m_pCurrent;
	int iCode = 0;

	if ( p[0]=='U' && p[1]=='+' )
	{
		p += 2;
		while ( isxdigit(*p) )
		{
			iCode = iCode*16 + HexDigit ( *p++ );
		}
		while ( isspace(*p) )
			p++;

	} else
	{
		if ( (*(BYTE*)p)<32 || (*(BYTE*)p)>127 )
		{
			Error ( "non-ASCII characters not allowed, use 'U+00AB' syntax" );
			return -1;
		}

		iCode = *p++;
		while ( isspace(*p) )
			p++;
	}

	m_pCurrent = p;
	return iCode;
}

bool CSphCharsetDefinitionParser::AddRange ( const CSphRemapRange & tRange, CSphVector<CSphRemapRange> & dRanges )
{
	if ( tRange.m_iRemapStart>=0x20 )
	{
		dRanges.Add ( tRange );
		return true;
	}

	CSphString sError;
	sError.SetSprintf ( "dest range (U+%x) below U+20, not allowed", tRange.m_iRemapStart );
	Error ( sError.cstr() );
	return false;
}


struct CharsetAlias_t
{
	CSphString					m_sName;
	int							m_iNameLen;
	CSphVector<CSphRemapRange>	m_dRemaps;
};

// Charsets relocated to folder 'charsets', each one in separate .txt file.
// When you change the content of the folder,
// reconfigure the project with cmake in order to pick the changes.

#if defined(HAVE_GLOBALALIASES_H)
#include "globalaliases.h"
#else
static const char * globalaliases[] = { "A..Z->a..z, a..z", nullptr, "U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451", nullptr };
static const char * globalaliases_names[] = { "english", "russian" };
enum E_GLOBALALIASES { CHARSET_ENGLISH, CHARSET_RUSSIAN, CHARSET_TOTAL };
#endif

static CSphVector<CharsetAlias_t> g_dCharsetAliases;


bool sphInitCharsetAliasTable ( CSphString & sError ) // FIXME!!! move alias generation to config common section
{
	g_dCharsetAliases.Reset();
	CSphCharsetDefinitionParser tParser;
	CSphVector<CharsetAlias_t> dAliases;
	CSphVector<char> dConcat;

	const int iTotalChunks = sizeof(globalaliases)/sizeof(globalaliases[0]);
	int iCurAliasChunk = 0;
	for ( int i=0; i<CHARSET_TOTAL; ++i )
	{
		CharsetAlias_t & tCur = dAliases.Add();
		tCur.m_sName = globalaliases_names[i];
		tCur.m_iNameLen = tCur.m_sName.Length();

		dConcat.Resize(0);
		while ( iCurAliasChunk<iTotalChunks && globalaliases[iCurAliasChunk] )
		{
			int iChunkLen = strlen(globalaliases[iCurAliasChunk]);
			char * szChunk = dConcat.AddN(iChunkLen);
			memcpy ( szChunk, globalaliases[iCurAliasChunk], iChunkLen );
			iCurAliasChunk++;
		}

		dConcat.Add(0);
		iCurAliasChunk++;

		if ( !tParser.Parse ( dConcat.Begin(), tCur.m_dRemaps ) )
		{
			sError = tParser.GetLastError();
			return false;
		}
	}

	g_dCharsetAliases.SwapData ( dAliases );
	return true;
}


bool CSphCharsetDefinitionParser::Parse ( const char * sConfig, CSphVector<CSphRemapRange> & dRanges )
{
	m_pCurrent = sConfig;
	dRanges.Reset ();

	// do parse
	while ( *m_pCurrent )
	{
		SkipSpaces ();
		if ( IsEof () )
			break;

		// check for stray comma
		if ( *m_pCurrent==',' )
			return Error ( "stray ',' not allowed, use 'U+002C' instead" );

		// alias
		bool bGotAlias = false;
		ARRAY_FOREACH_COND ( i, g_dCharsetAliases, !bGotAlias )
		{
			const CharsetAlias_t & tCur = g_dCharsetAliases[i];
			bGotAlias = ( strncmp ( tCur.m_sName.cstr(), m_pCurrent, tCur.m_iNameLen )==0 && ( !m_pCurrent[tCur.m_iNameLen] || m_pCurrent[tCur.m_iNameLen]==',' ) );
			if ( !bGotAlias )
				continue;

			// skip to next definition
			m_pCurrent += tCur.m_iNameLen;
			if ( *m_pCurrent && *m_pCurrent==',' )
				m_pCurrent++;

			ARRAY_FOREACH ( iDef, tCur.m_dRemaps )
			{
				if ( !AddRange ( tCur.m_dRemaps[iDef], dRanges ) )
					return false;
			}
		}
		if ( bGotAlias )
			continue;

		// parse char code
		const char * pStart = m_pCurrent;
		int iStart = ParseCharsetCode();
		if ( iStart<0 )
			return false;

		// stray char?
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			// stray char
			if ( !AddRange ( CSphRemapRange ( iStart, iStart, iStart ), dRanges ) )
				return false;

			if ( IsEof () )
				break;
			m_pCurrent++;
			continue;
		}

		// stray remap?
		if ( m_pCurrent[0]=='-' && m_pCurrent[1]=='>' )
		{
			// parse and add
			m_pCurrent += 2;
			int iDest = ParseCharsetCode ();
			if ( iDest<0 )
				return false;
			if ( !AddRange ( CSphRemapRange ( iStart, iStart, iDest ), dRanges ) )
				return false;

			// it's either end of line now, or must be followed by comma
			if ( *m_pCurrent )
				if ( *m_pCurrent++!=',' )
					return Error ( "syntax error" );
			continue;
		}

		// range start?
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "syntax error" );
		m_pCurrent += 2;

		SkipSpaces ();
		if ( CheckEof () )
			return false;

		// parse range end char code
		int iEnd = ParseCharsetCode ();
		if ( iEnd<0 )
			return false;
		if ( iStart>iEnd )
		{
			m_pCurrent = pStart;
			return Error ( "range end less than range start" );
		}

		// stray range?
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			if ( !AddRange ( CSphRemapRange ( iStart, iEnd, iStart ), dRanges ) )
				return false;

			if ( IsEof () )
				break;
			m_pCurrent++;
			continue;
		}

		// "checkerboard" range?
		if ( m_pCurrent[0]=='/' && m_pCurrent[1]=='2' )
		{
			for ( int i=iStart; i<iEnd; i+=2 )
			{
				if ( !AddRange ( CSphRemapRange ( i, i, i+1 ), dRanges ) )
					return false;
				if ( !AddRange ( CSphRemapRange ( i+1, i+1, i+1 ), dRanges ) )
					return false;
			}

			// skip "/2", expect ","
			m_pCurrent += 2;
			SkipSpaces ();
			if ( *m_pCurrent )
				if ( *m_pCurrent++!=',' )
					return Error ( "expected end of line or ','" );
			continue;
		}

		// remapped range?
		if (!( m_pCurrent[0]=='-' && m_pCurrent[1]=='>' ))
			return Error ( "expected end of line, ',' or '-><char>'" );
		m_pCurrent += 2;

		SkipSpaces ();
		if ( CheckEof () )
			return false;

		// parse dest start
		const char * pRemapStart = m_pCurrent;
		int iRemapStart = ParseCharsetCode ();
		if ( iRemapStart<0 )
			return false;

		// expect '..'
		if ( CheckEof () )
			return false;
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "expected '..'" );
		m_pCurrent += 2;

		// parse dest end
		int iRemapEnd = ParseCharsetCode ();
		if ( iRemapEnd<0 )
			return false;

		// check dest range
		if ( iRemapStart>iRemapEnd )
		{
			m_pCurrent = pRemapStart;
			return Error ( "dest range end less than dest range start" );
		}

		// check for length mismatch
		if ( ( iRemapEnd-iRemapStart )!=( iEnd-iStart ) )
		{
			m_pCurrent = pStart;
			return Error ( "dest range length must match src range length" );
		}

		// remapped ok
		if ( !AddRange ( CSphRemapRange ( iStart, iEnd, iRemapStart ), dRanges ) )
			return false;

		if ( IsEof () )
			break;
		if ( *m_pCurrent!=',' )
			return Error ( "expected ','" );
		m_pCurrent++;
	}

	dRanges.Sort ();
	for ( int i=0; i<dRanges.GetLength()-1; i++ )
	{
		if ( dRanges[i].m_iEnd>=dRanges[i+1].m_iStart )
		{
			// FIXME! add an ambiguity check
			dRanges[i].m_iEnd = Max ( dRanges[i].m_iEnd, dRanges[i+1].m_iEnd );
			dRanges.Remove ( i+1 );
			i--;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool sphParseCharset ( const char * sCharset, CSphVector<CSphRemapRange> & dRemaps )
{
	CSphCharsetDefinitionParser tParser;
	return tParser.Parse ( sCharset, dRemaps );
}

/////////////////////////////////////////////////////////////////////////////
void CSphEmbeddedFiles::Reset()
{
	m_dSynonyms.Reset();
	m_dStopwordFiles.Reset();
	m_dStopwords.Reset();
	m_dWordforms.Reset();
	m_dWordformFiles.Reset();
}


bool LoadTokenizerSettings ( CSphReader & tReader, CSphTokenizerSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning )
{
	tSettings.m_iType = tReader.GetByte ();
	if ( tSettings.m_iType!=TOKENIZER_UTF8 && tSettings.m_iType!=TOKENIZER_NGRAM )
	{
		sWarning = "can't load an old index with SBCS tokenizer";
		return false;
	}

	tSettings.m_sCaseFolding = tReader.GetString ();
	tSettings.m_iMinWordLen = tReader.GetDword ();
	tEmbeddedFiles.m_bEmbeddedSynonyms = false;
	tEmbeddedFiles.m_bEmbeddedSynonyms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedSynonyms )
	{
		int nSynonyms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dSynonyms.Resize ( nSynonyms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dSynonyms )
			tEmbeddedFiles.m_dSynonyms[i] = tReader.GetString();
	}

	tSettings.m_sSynonymsFile = tReader.GetString ();
	ReadFileInfo ( tReader, tSettings.m_sSynonymsFile.cstr (), false,
		tEmbeddedFiles.m_tSynonymFile, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	tSettings.m_sBoundary = tReader.GetString ();
	tSettings.m_sIgnoreChars = tReader.GetString ();
	tSettings.m_iNgramLen = tReader.GetDword ();
	tSettings.m_sNgramChars = tReader.GetString ();
	tSettings.m_sBlendChars = tReader.GetString ();
	tSettings.m_sBlendMode = tReader.GetString();

	return true;
}


/// gets called from and MUST be in sync with RtIndex_c::SaveDiskHeader()!
/// note that SaveDiskHeader() occasionaly uses some PREVIOUS format version!
void SaveTokenizerSettings ( CSphWriter & tWriter, const ISphTokenizer * pTokenizer, int iEmbeddedLimit )
{
	assert ( pTokenizer );

	const CSphTokenizerSettings & tSettings = pTokenizer->GetSettings ();
	tWriter.PutByte ( tSettings.m_iType );
	tWriter.PutString ( tSettings.m_sCaseFolding.cstr () );
	tWriter.PutDword ( tSettings.m_iMinWordLen );

	bool bEmbedSynonyms = ( iEmbeddedLimit>0 && pTokenizer->GetSynFileInfo ().m_uSize<=(SphOffset_t)iEmbeddedLimit );
	tWriter.PutByte ( bEmbedSynonyms ? 1 : 0 );
	if ( bEmbedSynonyms )
		pTokenizer->WriteSynonyms ( tWriter );

	tWriter.PutString ( tSettings.m_sSynonymsFile.cstr () );
	WriteFileInfo ( tWriter, pTokenizer->GetSynFileInfo () );
	tWriter.PutString ( tSettings.m_sBoundary.cstr () );
	tWriter.PutString ( tSettings.m_sIgnoreChars.cstr () );
	tWriter.PutDword ( tSettings.m_iNgramLen );
	tWriter.PutString ( tSettings.m_sNgramChars.cstr () );
	tWriter.PutString ( tSettings.m_sBlendChars.cstr () );
	tWriter.PutString ( tSettings.m_sBlendMode.cstr () );
}


void LoadDictionarySettings ( CSphReader & tReader, CSphDictSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning )
{
	tSettings.m_sMorphology = tReader.GetString ();
	tSettings.m_sMorphFields = tReader.GetString();

	tEmbeddedFiles.m_bEmbeddedStopwords = false;
	tEmbeddedFiles.m_bEmbeddedStopwords = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedStopwords )
	{
		int nStopwords = (int)tReader.GetDword();
		tEmbeddedFiles.m_dStopwords.Resize ( nStopwords );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dStopwords )
			tEmbeddedFiles.m_dStopwords[i] = (SphWordID_t)tReader.UnzipOffset();
	}

	tSettings.m_sStopwords = tReader.GetString ();
	int nFiles = tReader.GetDword ();

	CSphString sFile;
	tEmbeddedFiles.m_dStopwordFiles.Resize ( nFiles );
	for ( int i = 0; i < nFiles; i++ )
	{
		sFile = tReader.GetString ();
		ReadFileInfo ( tReader, sFile.cstr(), true, tEmbeddedFiles.m_dStopwordFiles[i], tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	}

	tEmbeddedFiles.m_bEmbeddedWordforms = false;
	tEmbeddedFiles.m_bEmbeddedWordforms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedWordforms )
	{
		int nWordforms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dWordforms.Resize ( nWordforms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dWordforms )
			tEmbeddedFiles.m_dWordforms[i] = tReader.GetString();
	}

	tSettings.m_dWordforms.Resize ( tReader.GetDword() );

	tEmbeddedFiles.m_dWordformFiles.Resize ( tSettings.m_dWordforms.GetLength() );
	ARRAY_FOREACH ( i, tSettings.m_dWordforms )
	{
		tSettings.m_dWordforms[i] = tReader.GetString();
		ReadFileInfo ( tReader, tSettings.m_dWordforms[i].cstr(), false,
			tEmbeddedFiles.m_dWordformFiles[i], tEmbeddedFiles.m_bEmbeddedWordforms ? NULL : &sWarning );
	}

	tSettings.m_iMinStemmingLen = tReader.GetDword ();

	tSettings.m_bWordDict = ( tReader.GetByte()!=0 );

	tSettings.m_bStopwordsUnstemmed = ( tReader.GetByte()!=0 );
	tSettings.m_sMorphFingerprint = tReader.GetString();
}


/// gets called from and MUST be in sync with RtIndex_c::SaveDiskHeader()!
/// note that SaveDiskHeader() occasionaly uses some PREVIOUS format version!
void SaveDictionarySettings ( CSphWriter & tWriter, const CSphDict * pDict, bool bForceWordDict, int iEmbeddedLimit )
{
	assert ( pDict );
	const CSphDictSettings & tSettings = pDict->GetSettings ();

	tWriter.PutString ( tSettings.m_sMorphology.cstr () );
	tWriter.PutString ( tSettings.m_sMorphFields.cstr() );

	const CSphVector <CSphSavedFile> & dSWFileInfos = pDict->GetStopwordsFileInfos ();
	SphOffset_t uTotalSize = 0;
	ARRAY_FOREACH ( i, dSWFileInfos )
		uTotalSize += dSWFileInfos[i].m_uSize;

	// embed only in case it allowed
	bool bEmbedStopwords = ( iEmbeddedLimit>0 && uTotalSize<=(SphOffset_t)iEmbeddedLimit );
	tWriter.PutByte ( bEmbedStopwords ? 1 : 0 );
	if ( bEmbedStopwords )
		pDict->WriteStopwords ( tWriter );

	tWriter.PutString ( tSettings.m_sStopwords.cstr () );
	tWriter.PutDword ( dSWFileInfos.GetLength () );
	ARRAY_FOREACH ( i, dSWFileInfos )
	{
		tWriter.PutString ( dSWFileInfos[i].m_sFilename.cstr () );
		WriteFileInfo ( tWriter, dSWFileInfos[i] );
	}

	const CSphVector <CSphSavedFile> & dWFFileInfos = pDict->GetWordformsFileInfos ();
	uTotalSize = 0;
	ARRAY_FOREACH ( i, dWFFileInfos )
		uTotalSize += dWFFileInfos[i].m_uSize;

	bool bEmbedWordforms = uTotalSize<=(SphOffset_t)iEmbeddedLimit;
	tWriter.PutByte ( bEmbedWordforms ? 1 : 0 );
	if ( bEmbedWordforms )
		pDict->WriteWordforms ( tWriter );

	tWriter.PutDword ( dWFFileInfos.GetLength() );
	ARRAY_FOREACH ( i, dWFFileInfos )
	{
		tWriter.PutString ( dWFFileInfos[i].m_sFilename.cstr() );
		WriteFileInfo ( tWriter, dWFFileInfos[i] );
	}

	tWriter.PutDword ( tSettings.m_iMinStemmingLen );
	tWriter.PutByte ( tSettings.m_bWordDict || bForceWordDict );
	tWriter.PutByte ( tSettings.m_bStopwordsUnstemmed );
	tWriter.PutString ( pDict->GetMorphDataFingerprint() );
}


void LoadFieldFilterSettings ( CSphReader & tReader, CSphFieldFilterSettings & tFieldFilterSettings )
{
	int nRegexps = tReader.GetDword();
	if ( !nRegexps )
		return;

	tFieldFilterSettings.m_dRegexps.Resize ( nRegexps );
	ARRAY_FOREACH ( i, tFieldFilterSettings.m_dRegexps )
		tFieldFilterSettings.m_dRegexps[i] = tReader.GetString();
}


void SaveFieldFilterSettings ( CSphWriter & tWriter, const ISphFieldFilter * pFieldFilter )
{
	CSphFieldFilterSettings tSettings;
	if ( pFieldFilter )
		pFieldFilter->GetSettings ( tSettings );

	SaveFieldFilterSettings ( tWriter, tSettings );
}


void SaveFieldFilterSettings ( CSphWriter & tWriter, const CSphFieldFilterSettings & tSettings )
{
	tWriter.PutDword ( tSettings.m_dRegexps.GetLength() );
	ARRAY_FOREACH ( i, tSettings.m_dRegexps )
		tWriter.PutString ( tSettings.m_dRegexps[i] );
}


static inline bool ShortTokenFilter ( BYTE * pToken, int iLen )
{
	return pToken[0]=='*' || ( iLen > 0 && pToken[iLen-1]=='*' );
}

/////////////////////////////////////////////////////////////////////////////

bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	const int MIN_CODE = 0x21;
	ARRAY_FOREACH ( i, dRemaps )
	{
		CSphRemapRange & tMap = dRemaps[i];

		if ( tMap.m_iStart<MIN_CODE || tMap.m_iStart>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character mapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iStart, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iStart = Min ( Max ( tMap.m_iStart, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		if ( tMap.m_iEnd<MIN_CODE || tMap.m_iEnd>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character mapping end specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iEnd, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iEnd = Min ( Max ( tMap.m_iEnd, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		if ( tMap.m_iRemapStart<MIN_CODE || tMap.m_iRemapStart>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character remapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iRemapStart, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iRemapStart = Min ( Max ( tMap.m_iRemapStart, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		int iRemapEnd = tMap.m_iRemapStart+tMap.m_iEnd-tMap.m_iStart;
		if ( iRemapEnd<MIN_CODE || iRemapEnd>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character remapping end specified: U+%x, should be between U+%x and U+%x (inclusive); IGNORED",
				iRemapEnd, MIN_CODE, m_tLC.MAX_CODE-1 );
			dRemaps.Remove(i);
			i--;
		}
	}

	m_tLC.Reset ();
	m_tLC.AddRemaps ( dRemaps, 0 );
	return true;
}


void ISphTokenizer::AddPlainChar ( char c )
{
	CSphVector<CSphRemapRange> dTmp ( 1 );
	dTmp[0].m_iStart = dTmp[0].m_iEnd = dTmp[0].m_iRemapStart = c;
	m_tLC.AddRemaps ( dTmp, 0 );
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	m_tLC.AddSpecials ( sSpecials );
}


void ISphTokenizer::Setup ( const CSphTokenizerSettings & tSettings )
{
	m_tSettings = tSettings;
}


ISphTokenizer * ISphTokenizer::Create ( const CSphTokenizerSettings & tSettings, const CSphEmbeddedFiles * pFiles, CSphString & sError )
{
	ISphTokenizerRefPtr_c pTokenizer;

	switch ( tSettings.m_iType )
	{
	case TOKENIZER_UTF8:	pTokenizer = sphCreateUTF8Tokenizer (); break;
	case TOKENIZER_NGRAM:	pTokenizer = sphCreateUTF8NgramTokenizer (); break;
	default:
		sError.SetSprintf ( "failed to create tokenizer (unknown charset type '%d')", tSettings.m_iType );
		return nullptr;
	}

	pTokenizer->Setup ( tSettings );

	if ( !tSettings.m_sCaseFolding.IsEmpty () && !pTokenizer->SetCaseFolding ( tSettings.m_sCaseFolding.cstr (), sError ) )
	{
		sError.SetSprintf ( "'charset_table': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sSynonymsFile.IsEmpty () && !pTokenizer->LoadSynonyms ( tSettings.m_sSynonymsFile.cstr (),
		pFiles && pFiles->m_bEmbeddedSynonyms ? pFiles : nullptr, sError ) )
	{
		sError.SetSprintf ( "'synonyms': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sBoundary.IsEmpty () && !pTokenizer->SetBoundary ( tSettings.m_sBoundary.cstr (), sError ) )
	{
		sError.SetSprintf ( "'phrase_boundary': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sIgnoreChars.IsEmpty () && !pTokenizer->SetIgnoreChars ( tSettings.m_sIgnoreChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ignore_chars': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sBlendChars.IsEmpty () && !pTokenizer->SetBlendChars ( tSettings.m_sBlendChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_chars': %s", sError.cstr() );
		return nullptr;
	}

	if ( !pTokenizer->SetBlendMode ( tSettings.m_sBlendMode.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_mode': %s", sError.cstr() );
		return nullptr;
	}

	pTokenizer->SetNgramLen ( tSettings.m_iNgramLen );

	if ( !tSettings.m_sNgramChars.IsEmpty () && !pTokenizer->SetNgramChars ( tSettings.m_sNgramChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ngram_chars': %s", sError.cstr() );
		return nullptr;
	}

	return pTokenizer.Leak ();
}


ISphTokenizer * ISphTokenizer::CreateMultiformFilter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
{
	if ( !pContainer )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}
	return new CSphMultiformTokenizer ( pTokenizer, pContainer );
}


ISphTokenizer * ISphTokenizer::CreateBigramFilter ( ISphTokenizer * pTokenizer, ESphBigram eBigramIndex, const CSphString & sBigramWords, CSphString & sError )
{
	assert ( pTokenizer );

	if ( eBigramIndex==SPH_BIGRAM_NONE )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}

	StrVec_t dFreq;
	if ( eBigramIndex!=SPH_BIGRAM_ALL )
	{
		const BYTE * pTok = NULL;
		pTokenizer->SetBuffer ( (const BYTE*)sBigramWords.cstr(), sBigramWords.Length() );
		while ( ( pTok = pTokenizer->GetToken() )!=NULL )
			dFreq.Add ( (const char*)pTok );

		if ( !dFreq.GetLength() )
		{
			SafeDelete ( pTokenizer );
			sError.SetSprintf ( "bigram_freq_words does not contain any valid words" );
			return NULL;
		}
	}

	return new CSphBigramTokenizer ( pTokenizer, eBigramIndex, dFreq );
}


class PluginFilterTokenizer_c : public CSphTokenFilter
{
protected:
	const PluginTokenFilter_c *	m_pFilter;		///< plugin descriptor
	CSphString					m_sOptions;		///< options string for the plugin init()
	void *						m_pUserdata = nullptr;	///< userdata returned from by the plugin init()
	bool						m_bGotExtra = false;	///< are we looping through extra tokens?
	int							m_iPosDelta = 0;		///< position delta for the current token, see comments in GetToken()
	bool						m_bWasBlended = false;	///< whether the last raw token was blended

	~PluginFilterTokenizer_c () final
	{
		if ( m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );
		m_pFilter->Release ();
	}

public:
	PluginFilterTokenizer_c ( ISphTokenizer * pTok, const PluginTokenFilter_c * pFilter, const char * sOptions )
		: CSphTokenFilter ( pTok )
		, m_pFilter ( pFilter )
		, m_sOptions ( sOptions )
	{
		assert ( pTok );
		assert ( m_pFilter );
		m_pFilter->AddRef();
		// FIXME!!! handle error in constructor \ move to setup?
		CSphString sError;
		SetFilterSchema ( CSphSchema(), sError );
	}

	ISphTokenizer * Clone ( ESphTokenizerClone eMode ) const final
	{
		ISphTokenizerRefPtr_c pTok { m_pTokenizer->Clone ( eMode ) };
		return new PluginFilterTokenizer_c ( pTok, m_pFilter, m_sOptions.cstr() );
	}

	bool SetFilterSchema ( const CSphSchema & s, CSphString & sError ) final
	{
		if ( m_pUserdata && m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );

		CSphVector<const char*> dFields;
		for ( int i = 0; i < s.GetFieldsCount(); i++ )
			dFields.Add ( s.GetFieldName(i) );

		char sErrBuf[SPH_UDF_ERROR_LEN+1];
		if ( m_pFilter->m_fnInit ( &m_pUserdata, dFields.GetLength(), dFields.Begin(), m_sOptions.cstr(), sErrBuf )==0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	bool SetFilterOptions ( const char * sOptions, CSphString & sError ) final
	{
		char sErrBuf[SPH_UDF_ERROR_LEN+1];
		if ( m_pFilter->m_fnBeginDocument ( m_pUserdata, sOptions, sErrBuf )==0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	void BeginField ( int iField ) final
	{
		if ( m_pFilter->m_fnBeginField )
			m_pFilter->m_fnBeginField ( m_pUserdata, iField );
	}

	BYTE * GetToken () final
	{
		// we have two principal states here
		// a) have pending extra tokens, keep looping and returning those
		// b) no extras, keep pushing until plugin returns anything
		//
		// we also have to handle position deltas, and that story is a little tricky
		// positions are not assigned in the tokenizer itself (we might wanna refactor that)
		// however, tokenizer has some (partial) control over the keyword positions, too
		// when it skips some too-short tokens, it returns a non-zero value via GetOvershortCount()
		// when it returns a blended token, it returns true via TokenIsBlended()
		// so while the default position delta is 1, overshorts can increase it by N,
		// and blended flag can decrease it by 1, and that's under tokenizer's control
		//
		// so for the plugins, we simplify (well i hope!) this complexity a little
		// we compute a proper position delta here, pass it, and let the plugin modify it
		// we report all tokens as regular, and return the delta via GetOvershortCount()

		// state (a), just loop the pending extras
		if ( m_bGotExtra )
		{
			m_iPosDelta = 1; // default delta is 1
			BYTE * pTok = (BYTE*) m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
			if ( pTok )
				return pTok;
			m_bGotExtra = false;
		}

		// state (b), push raw tokens, return results
		for ( ;; )
		{
			// get next raw token, handle field end
			BYTE * pRaw = CSphTokenFilter::GetToken();
			if ( !pRaw )
			{
				// no more hits? notify plugin of a field end,
				// and check if there are pending tokens
				m_bGotExtra = 0;
				if ( m_pFilter->m_fnEndField )
					if ( !m_pFilter->m_fnEndField ( m_pUserdata ) )
						return NULL;

				// got them, start fetching
				m_bGotExtra = true;
				return (BYTE*)m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
			}

			// compute proper position delta
			m_iPosDelta = ( m_bWasBlended ? 0 : 1 ) + CSphTokenFilter::GetOvershortCount();
			m_bWasBlended = CSphTokenFilter::TokenIsBlended();

			// push raw token to plugin, return a processed one, if any
			int iExtra = 0;
			BYTE * pTok = (BYTE*)m_pFilter->m_fnPushToken ( m_pUserdata, (char*)pRaw, &iExtra, &m_iPosDelta );
			m_bGotExtra = ( iExtra!=0 );
			if ( pTok )
				return pTok;
		}
	}

	int GetOvershortCount() final
	{
		return Max ( 0, m_iPosDelta-1 );
	}

	bool TokenIsBlended() const final
	{
		return false;
	}
};


ISphTokenizer * ISphTokenizer::CreatePluginFilter ( ISphTokenizer * pTokenizer, const CSphString & sSpec, CSphString & sError )
{
	StrVec_t dPlugin; // dll, filtername, options
	if ( !sphPluginParseSpec ( sSpec, dPlugin, sError ) )
		return nullptr;

	if ( !dPlugin.GetLength() )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}

	CSphRefcountedPtr<PluginTokenFilter_c> p { ( PluginTokenFilter_c *) sphPluginAcquire ( dPlugin[0].cstr(), PLUGIN_INDEX_TOKEN_FILTER, dPlugin[1].cstr(), sError ) };
	if ( !p )
	{
		sError.SetSprintf ( "INTERNAL ERROR: plugin %s:%s loaded ok but lookup fails, error: %s", dPlugin[0].cstr(), dPlugin[1].cstr(), sError.cstr() );
		return nullptr;
	}
	return new PluginFilterTokenizer_c ( pTokenizer, p, dPlugin[2].cstr () );
}


bool ISphTokenizer::AddSpecialsSPZ ( const char * sSpecials, const char * sDirective, CSphString & sError )
{
	for ( int i=0; sSpecials[i]; i++ )
	{
		int iCode = m_tLC.ToLower ( sSpecials[i] );
		if ( iCode & ( FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_BOUNDARY | FLAG_CODEPOINT_IGNORE ) )
		{
			sError.SetSprintf ( "%s requires that character '%c' is not in ngram_chars, phrase_boundary, or ignore_chars",
				sDirective, sSpecials[i] );
			return false;
		}
	}

	AddSpecials ( sSpecials );
	return true;
}


bool ISphTokenizer::EnableSentenceIndexing ( CSphString & sError )
{
	const char sSpecials[] = { '.', '?', '!', MAGIC_CODE_PARAGRAPH, 0 };

	if ( !AddSpecialsSPZ ( sSpecials, "index_sp", sError ) )
		return false;

	m_bDetectSentences = true;
	return true;
}


bool ISphTokenizer::EnableZoneIndexing ( CSphString & sError )
{
	static const char sSpecials[] = { MAGIC_CODE_ZONE, 0 };
	return AddSpecialsSPZ ( sSpecials, "index_zones", sError );
}

uint64_t ISphTokenizer::GetSettingsFNV () const
{
	uint64_t uHash = m_tLC.GetFNV();

	DWORD uFlags = 0;
	if ( m_bBlendSkipPure )
		uFlags |= 1<<1;
	if ( m_bShortTokenFilter )
		uFlags |= 1<<2;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );
	uHash = sphFNV64 ( &m_uBlendVariants, sizeof(m_uBlendVariants), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iType, sizeof(m_tSettings.m_iType), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iMinWordLen, sizeof(m_tSettings.m_iMinWordLen), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iNgramLen, sizeof(m_tSettings.m_iNgramLen), uHash );

	return uHash;
}

//////////////////////////////////////////////////////////////////////////

CSphTokenizerBase::CSphTokenizerBase ()
{
	m_pAccum = m_sAccum;
}


CSphTokenizerBase::~CSphTokenizerBase()
{
	SafeDelete ( m_pExc );
}


bool CSphTokenizerBase::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	if ( m_pExc )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}
	m_bHasBlend = false;
	return ISphTokenizer::SetCaseFolding ( sConfig, sError );
}


bool CSphTokenizerBase::SetBlendChars ( const char * sConfig, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	m_bHasBlend = ISphTokenizer::SetBlendChars ( sConfig, sError );
	return m_bHasBlend;
}


bool CSphTokenizerBase::LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );

	ExceptionsTrieGen_c g;
	if ( pFiles )
	{
		m_tSynFileInfo = pFiles->m_tSynonymFile;
		ARRAY_FOREACH ( i, pFiles->m_dSynonyms )
		{
			if ( !g.ParseLine ( (char*)pFiles->m_dSynonyms[i].cstr(), sError ) )
				sphWarning ( "%s line %d: %s", pFiles->m_tSynonymFile.m_sFilename.cstr(), i, sError.cstr() );
		}
	} else
	{
		if ( !sFilename || !*sFilename )
			return true;

		GetFileStats ( sFilename, m_tSynFileInfo, NULL );

		CSphAutoreader tReader;
		if ( !tReader.Open ( sFilename, sError ) )
			return false;

		char sBuffer[1024];
		int iLine = 0;
		while ( tReader.GetLine ( sBuffer, sizeof(sBuffer) )>=0 )
		{
			iLine++;
			if ( !g.ParseLine ( sBuffer, sError ) )
				sphWarning ( "%s line %d: %s", sFilename, iLine, sError.cstr() );
		}
	}

	m_pExc = g.Build();
	return true;
}


void CSphTokenizerBase::WriteSynonyms ( CSphWriter & tWriter ) const
{
	if ( m_pExc )
		m_pExc->Export ( tWriter );
	else
		tWriter.PutDword ( 0 );
}


void CSphTokenizerBase::CloneBase ( const CSphTokenizerBase * pFrom, ESphTokenizerClone eMode )
{
	m_eMode = eMode;
	m_pExc = NULL;
	if ( pFrom->m_pExc )
	{
		m_pExc = new ExceptionsTrie_c();
		*m_pExc = *pFrom->m_pExc;
	}
	m_tSettings = pFrom->m_tSettings;
	m_bHasBlend = pFrom->m_bHasBlend;
	m_uBlendVariants = pFrom->m_uBlendVariants;
	m_bBlendSkipPure = pFrom->m_bBlendSkipPure;
	m_bShortTokenFilter = ( eMode!=SPH_CLONE_INDEX );
	m_bDetectSentences = pFrom->m_bDetectSentences;

	switch ( eMode )
	{
		case SPH_CLONE_INDEX:
			assert ( pFrom->m_eMode==SPH_CLONE_INDEX );
			m_tLC = pFrom->m_tLC;
			break;

		case SPH_CLONE_QUERY:
		{
			assert ( pFrom->m_eMode==SPH_CLONE_INDEX || pFrom->m_eMode==SPH_CLONE_QUERY );
			m_tLC = pFrom->m_tLC;

			CSphVector<CSphRemapRange> dRemaps;
			CSphRemapRange Range;
			Range.m_iStart = Range.m_iEnd = Range.m_iRemapStart = '\\';
			dRemaps.Add ( Range );
			m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_SPECIAL );

			m_uBlendVariants = BLEND_TRIM_NONE;
			break;
		}

		case SPH_CLONE_QUERY_LIGHTWEIGHT:
		{
			// FIXME? avoid double lightweight clones, too?
			assert ( pFrom->m_eMode!=SPH_CLONE_INDEX );
			assert ( pFrom->m_tLC.ToLower ( '\\' ) & FLAG_CODEPOINT_SPECIAL );

			// lightweight tokenizer clone
			// copy 3 KB of lowercaser chunk pointers, but do NOT copy the table data
			SafeDeleteArray ( m_tLC.m_pData );
			m_tLC.m_iChunks = 0;
			m_tLC.m_pData = NULL;
			for ( int i=0; i<CSphLowercaser::CHUNK_COUNT; i++ )
				m_tLC.m_pChunk[i] = pFrom->m_tLC.m_pChunk[i];
			break;
		}
	}
}

uint64_t CSphTokenizerBase::GetSettingsFNV () const
{
	uint64_t uHash = ISphTokenizer::GetSettingsFNV();

	DWORD uFlags = 0;
	if ( m_bHasBlend )
		uFlags |= 1<<0;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );

	return uHash;
}


void CSphTokenizerBase::SetBufferPtr ( const char * sNewPtr )
{
	assert ( (BYTE*)sNewPtr>=m_pBuffer && (BYTE*)sNewPtr<=m_pBufferMax );
	m_pCur = Min ( m_pBufferMax, Max ( m_pBuffer, (const BYTE*)sNewPtr ) );
	m_iAccum = 0;
	m_pAccum = m_sAccum;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;
}


int CSphTokenizerBase2::SkipBlended()
{
	if ( !m_pBlendEnd )
		return 0;

	const BYTE * pMax = m_pBufferMax;
	m_pBufferMax = m_pBlendEnd;

	// loop until the blended token end
	int iBlended = 0; // how many blended subtokens we have seen so far
	int iAccum = 0; // how many non-blended chars in a row we have seen so far
	while ( m_pCur < m_pBufferMax )
	{
		int iCode = GetCodepoint();
		if ( iCode=='\\' )
			iCode = GetCodepoint(); // no boundary check, GetCP does it
		iCode = m_tLC.ToLower ( iCode ); // no -1 check, ToLower does it
		if ( iCode<0 )
			iCode = 0;
		if ( iCode & FLAG_CODEPOINT_BLEND )
			iCode = 0;
		if ( iCode & MASK_CODEPOINT )
		{
			iAccum++;
			continue;
		}
		if ( iAccum>=m_tSettings.m_iMinWordLen )
			iBlended++;
		iAccum = 0;
	}
	if ( iAccum>=m_tSettings.m_iMinWordLen )
		iBlended++;

	m_pBufferMax = pMax;
	return iBlended;
}


/// adjusts blending magic when we're about to return a token (any token)
/// returns false if current token should be skipped, true otherwise
bool CSphTokenizerBase::BlendAdjust ( const BYTE * pCur )
{
	// check if all we got is a bunch of blended characters (pure-blended case)
	if ( m_bBlended && !m_bNonBlended )
	{
		// we either skip this token, or pretend it was normal
		// in both cases, clear the flag
		m_bBlended = false;

		// do we need to skip it?
		if ( m_bBlendSkipPure )
		{
			m_pBlendStart = NULL;
			return false;
		}
	}
	m_bNonBlended = false;

	// adjust buffer pointers
	if ( m_bBlended && m_pBlendStart )
	{
		// called once per blended token, on processing start
		// at this point, full blended token is in the accumulator
		// and we're about to return it
		m_pCur = m_pBlendStart;
		m_pBlendEnd = pCur;
		m_pBlendStart = nullptr;
		m_bBlendedPart = true;
	} else if ( pCur>=m_pBlendEnd )
	{
		// tricky bit, as at this point, token we're about to return
		// can either be a blended subtoken, or the next one
		m_bBlendedPart = m_pTokenStart && ( m_pTokenStart<m_pBlendEnd );
		m_pBlendEnd = nullptr;
		m_pBlendStart = nullptr;
	} else if ( !m_pBlendEnd )
	{
		// we aren't re-parsing blended; so clear the "blended subtoken" flag
		m_bBlendedPart = false;
	}
	return true;
}


static inline void CopySubstring ( BYTE * pDst, const BYTE * pSrc, int iLen )
{
	while ( iLen-->0 && *pSrc )
		*pDst++ = *pSrc++;
	*pDst++ = '\0';
}


BYTE * CSphTokenizerBase2::GetBlendedVariant ()
{
	// we can get called on several occasions
	// case 1, a new blended token was just accumulated
	if ( m_bBlended && !m_bBlendAdd )
	{
		// fast path for the default case (trim_none)
		if ( m_uBlendVariants==BLEND_TRIM_NONE )
			return m_sAccum;

		// analyze the full token, find non-blended bounds
		m_iBlendNormalStart = -1;
		m_iBlendNormalEnd = -1;

		// OPTIMIZE? we can skip this based on non-blended flag from adjust
		const BYTE * p = m_sAccum;
		while ( *p )
		{
			int iLast = (int)( p-m_sAccum );
			int iCode = sphUTF8Decode(p);
			if (!( m_tLC.ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ))
			{
				m_iBlendNormalEnd = (int)( p-m_sAccum );
				if ( m_iBlendNormalStart<0 )
					m_iBlendNormalStart = iLast;
			}
		}

		// build todo mask
		// check and revert a few degenerate cases
		m_uBlendVariantsPending = m_uBlendVariants;
		if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
		{
			if ( m_iBlendNormalStart<0 )
			{
				// no heading blended; revert BOTH to TAIL
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_TAIL;
			} else if ( m_iBlendNormalEnd<0 )
			{
				// no trailing blended; revert BOTH to HEAD
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_HEAD;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
		{
			// either no heading blended, or pure blended; revert HEAD to NONE
			if ( m_iBlendNormalStart<=0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
		{
			// either no trailing blended, or pure blended; revert TAIL to NONE
			if ( m_iBlendNormalEnd<=0 || m_sAccum[m_iBlendNormalEnd]==0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}

		// ok, we are going to return a few variants after all, flag that
		// OPTIMIZE? add fast path for "single" variants?
		m_bBlendAdd = true;
		assert ( m_uBlendVariantsPending );

		// we also have to stash the original blended token
		// because accumulator contents may get trashed by caller (say, when stemming)
		strncpy ( (char*)m_sAccumBlend, (char*)m_sAccum, sizeof(m_sAccumBlend)-1 );
	}

	// case 2, caller is checking for pending variants, have we even got any?
	if ( !m_bBlendAdd )
		return NULL;

	// handle trim_none
	// this MUST be the first handler, so that we could avoid copying below, and just return the original accumulator
	if ( m_uBlendVariantsPending & BLEND_TRIM_NONE )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_NONE;
		m_bBlended = true;
		return m_sAccum;
	}

	// handle trim_all
	if ( m_uBlendVariantsPending & BLEND_TRIM_ALL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_ALL;
		m_bBlended = true;
		const BYTE * pSrc = m_sAccumBlend;
		BYTE * pDst = m_sAccum;
		while ( *pSrc )
		{
			int iCode = sphUTF8Decode ( pSrc );
			if ( !( m_tLC.ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ) )
				pDst += sphUTF8Encode ( pDst, ( iCode & MASK_CODEPOINT ) );
		}
		*pDst = '\0';

		return m_sAccum;
	}

	// handle trim_both
	if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
		if ( m_iBlendNormalStart<0 )
			m_uBlendVariantsPending |= BLEND_TRIM_TAIL; // no heading blended; revert BOTH to TAIL
		else if ( m_iBlendNormalEnd<0 )
			m_uBlendVariantsPending |= BLEND_TRIM_HEAD; // no trailing blended; revert BOTH to HEAD
		else
		{
			assert ( m_iBlendNormalStart<m_iBlendNormalEnd );
			CopySubstring ( m_sAccum, m_sAccumBlend+m_iBlendNormalStart, m_iBlendNormalEnd-m_iBlendNormalStart );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_HEAD
	if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
		if ( m_iBlendNormalStart>=0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend+m_iBlendNormalStart, sizeof(m_sAccum) );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_TAIL
	if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
		if ( m_iBlendNormalEnd>0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend, m_iBlendNormalEnd );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// all clear, no more variants to go
	m_bBlendAdd = false;
	return NULL;
}


static inline bool IsCapital ( int iCh )
{
	return iCh>='A' && iCh<='Z';
}


static inline bool IsWhitespace ( BYTE c )
{
	return ( c=='\0' || c==' ' || c=='\t' || c=='\r' || c=='\n' );
}


static inline bool IsWhitespace ( int c )
{
	return ( c=='\0' || c==' ' || c=='\t' || c=='\r' || c=='\n' );
}


static inline bool IsBoundary ( BYTE c, bool bPhrase )
{
	// FIXME? sorta intersects with specials
	// then again, a shortened-down list (more strict syntax) is reasonble here too
	return IsWhitespace(c) || c=='"' || ( !bPhrase && ( c=='(' || c==')' || c=='|' ) );
}


static inline bool IsPunctuation ( int c )
{
	return ( c>=33 && c<=47 ) || ( c>=58 && c<=64 ) || ( c>=91 && c<=96 ) || ( c>=123 && c<=126 );
}


int CSphTokenizerBase::CodepointArbitrationI ( int iCode )
{
	if ( !m_bDetectSentences )
		return iCode;

	// detect sentence boundaries
	// FIXME! should use charset_table (or add a new directive) and support languages other than English
	int iSymbol = iCode & MASK_CODEPOINT;
	if ( iSymbol=='?' || iSymbol=='!' )
	{
		// definitely a sentence boundary
		return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
	}

	if ( iSymbol=='.' )
	{
		// inline dot ("in the U.K and"), not a boundary
		bool bInwordDot = ( sphIsAlpha ( m_pCur[0] ) || m_pCur[0]==',' );

		// followed by a small letter or an opening paren, not a boundary
		// FIXME? might want to scan for more than one space
		// Yoyodine Inc. exists ...
		// Yoyodine Inc. (the company) ..
		bool bInphraseDot = ( sphIsSpace ( m_pCur[0] )
			&& ( ( 'a'<=m_pCur[1] && m_pCur[1]<='z' )
				|| ( m_pCur[1]=='(' && 'a'<=m_pCur[2] && m_pCur[2]<='z' ) ) );

		// preceded by something that looks like a middle name, opening first name, salutation
		bool bMiddleName = false;
		switch ( m_iAccum )
		{
			case 1:
				// 1-char capital letter
				// example: J. R. R. Tolkien, who wrote Hobbit ...
				// example: John D. Doe ...
				bMiddleName = IsCapital ( m_pCur[-2] );
				break;
			case 2:
				// 2-char token starting with a capital
				if ( IsCapital ( m_pCur[-3] ) )
				{
					// capital+small
					// example: Known as Mr. Doe ...
					if ( !IsCapital ( m_pCur[-2] ) )
						bMiddleName = true;

					// known capital+capital (MR, DR, MS)
					if (
						( m_pCur[-3]=='M' && m_pCur[-2]=='R' ) ||
						( m_pCur[-3]=='M' && m_pCur[-2]=='S' ) ||
						( m_pCur[-3]=='D' && m_pCur[-2]=='R' ) )
							bMiddleName = true;
				}
				break;
			case 3:
				// preceded by a known 3-byte token (MRS, DRS)
				// example: Survived by Mrs. Doe ...
				if ( ( m_sAccum[0]=='m' || m_sAccum[0]=='d' ) && m_sAccum[1]=='r' && m_sAccum[2]=='s' )
					bMiddleName = true;
				break;
		}

		if ( !bInwordDot && !bInphraseDot && !bMiddleName )
		{
			// sentence boundary
			return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
		} else
		{
			// just a character
			if ( ( iCode & MASK_FLAGS )==FLAG_CODEPOINT_SPECIAL )
				return 0; // special only, not dual? then in this context, it is a separator
			else
				return iCode & ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL ); // perhaps it was blended, so return the original code
		}
	}

	// pass-through
	return iCode;
}


int CSphTokenizerBase::CodepointArbitrationQ ( int iCode, bool bWasEscaped, BYTE uNextByte )
{
	if ( iCode & FLAG_CODEPOINT_NGRAM )
		return iCode; // ngrams are handled elsewhere

	int iSymbol = iCode & MASK_CODEPOINT;

	// codepoints can't be blended and special at the same time
	if ( ( iCode & FLAG_CODEPOINT_BLEND ) && ( iCode & FLAG_CODEPOINT_SPECIAL ) )
	{
		bool bBlend =
			bWasEscaped || // escaped characters should always act as blended
			( m_bPhrase && !sphIsModifier ( iSymbol ) && iSymbol!='"' ) || // non-modifier special inside phrase
			( m_iAccum && ( iSymbol=='@' || iSymbol=='/' || iSymbol=='-' ) ); // some specials in the middle of a token

		// clear special or blend flags
		iCode &= bBlend
			? ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_SPECIAL )
			: ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_BLEND );
	}

	// escaped specials are not special
	// dash and dollar inside the word are not special (however, single opening modifier is not a word!)
	// non-modifier specials within phrase are not special
	bool bDashInside = ( m_iAccum && iSymbol=='-' && !( m_iAccum==1 && sphIsModifier ( m_sAccum[0] ) ));
	if ( iCode & FLAG_CODEPOINT_SPECIAL )
		if ( bWasEscaped
			|| bDashInside
			|| ( m_iAccum && iSymbol=='$' && !IsBoundary ( uNextByte, m_bPhrase ) )
			|| ( m_bPhrase && iSymbol!='"' && !sphIsModifier ( iSymbol ) ) )
	{
		if ( iCode & FLAG_CODEPOINT_DUAL )
			iCode &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
		else
			iCode = 0;
	}

	// if we didn't remove special by now, it must win
	if ( iCode & FLAG_CODEPOINT_DUAL )
	{
		assert ( iCode & FLAG_CODEPOINT_SPECIAL );
		iCode = iSymbol | FLAG_CODEPOINT_SPECIAL;
	}

	// ideally, all conflicts must be resolved here
	// well, at least most
	assert ( sphBitCount ( iCode & MASK_FLAGS )<=1 );
	return iCode;
}

#if !USE_WINDOWS
#define __forceinline inline
#endif

static __forceinline bool IsSeparator ( int iFolded, bool bFirst )
{
	// eternal separator
	if ( iFolded<0 || ( iFolded & MASK_CODEPOINT )==0 )
		return true;

	// just a codepoint
	if (!( iFolded & MASK_FLAGS ))
		return false;

	// any magic flag, besides dual
	if (!( iFolded & FLAG_CODEPOINT_DUAL ))
		return true;

	// FIXME? n-grams currently also set dual
	if ( iFolded & FLAG_CODEPOINT_NGRAM )
		return true;

	// dual depends on position
	return bFirst;
}

// handles escaped specials that are not in the character set
// returns true if the codepoint should be processed as a simple codepoint,
// returns false if it should be processed as a whitespace
// for example: aaa\!bbb => aaa bbb
static inline bool Special2Simple ( int & iCodepoint )
{
	if ( ( iCodepoint & FLAG_CODEPOINT_DUAL ) || !( iCodepoint & FLAG_CODEPOINT_SPECIAL ) )
	{
		iCodepoint &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
		return true;
	}

	return false;
}

static bool CheckRemap ( CSphString & sError, const CSphVector<CSphRemapRange> & dRemaps, const char * sSource, bool bCanRemap, const CSphLowercaser & tLC )
{
	// check
	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & r = dRemaps[i];

		for ( int j=r.m_iStart; j<=r.m_iEnd; j++ )
		{
			if ( tLC.ToLower ( j ) )
			{
				sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
				return false;
			}
		}

		if ( bCanRemap )
		{
			for ( int j=r.m_iRemapStart; j<=r.m_iRemapStart + r.m_iEnd - r.m_iStart; j++ )
			{
				if ( tLC.ToLower ( j ) )
				{
					sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
					return false;
				}
			}
		}
	}

	return true;
}


bool ISphTokenizer::RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError )
{
	// parse
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	if ( !CheckRemap ( sError, dRemaps, sSource, bCanRemap, m_tLC ) )
		return false;

	// add mapping
	m_tLC.AddRemaps ( dRemaps, uFlags );
	return true;
}

bool ISphTokenizer::SetBoundary ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_BOUNDARY, "phrase boundary", false, sError );
}

bool ISphTokenizer::SetIgnoreChars ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_IGNORE, "ignored", false, sError );
}

bool ISphTokenizer::SetBlendChars ( const char * sConfig, CSphString & sError )
{
	return sConfig ? RemapCharacters ( sConfig, FLAG_CODEPOINT_BLEND, "blend", true, sError ) : false;
}


static bool sphStrncmp ( const char * sCheck, int iCheck, const char * sRef )
{
	return ( iCheck==(int)strlen(sRef) && memcmp ( sCheck, sRef, iCheck )==0 );
}


bool ISphTokenizer::SetBlendMode ( const char * sMode, CSphString & sError )
{
	if ( !sMode || !*sMode )
	{
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return true;
	}

	m_uBlendVariants = 0;
	const char * p = sMode;
	while ( *p )
	{
		while ( !sphIsAlpha(*p) )
			p++;
		if ( !*p )
			break;

		const char * sTok = p;
		while ( sphIsAlpha(*p) )
			p++;
		if ( sphStrncmp ( sTok, p-sTok, "trim_none" ) )
			m_uBlendVariants |= BLEND_TRIM_NONE;
		else if ( sphStrncmp ( sTok, p-sTok, "trim_head" ) )
			m_uBlendVariants |= BLEND_TRIM_HEAD;
		else if ( sphStrncmp ( sTok, p-sTok, "trim_tail" ) )
			m_uBlendVariants |= BLEND_TRIM_TAIL;
		else if ( sphStrncmp ( sTok, p-sTok, "trim_both" ) )
			m_uBlendVariants |= BLEND_TRIM_BOTH;
		else if ( sphStrncmp ( sTok, p-sTok, "trim_all" ) )
			m_uBlendVariants |= BLEND_TRIM_ALL;
		else if ( sphStrncmp ( sTok, p-sTok, "skip_pure" ) )
			m_bBlendSkipPure = true;
		else
		{
			sError.SetSprintf ( "unknown blend_mode option near '%s'", sTok );
			return false;
		}
	}

	if ( !m_uBlendVariants )
	{
		sError.SetSprintf ( "blend_mode must define at least one variant to index" );
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

template < bool IS_QUERY >
CSphTokenizer_UTF8<IS_QUERY>::CSphTokenizer_UTF8 ()
{
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE, sTmp );
	m_bHasBlend = false;
}


template < bool IS_QUERY >
void CSphTokenizer_UTF8<IS_QUERY>::SetBuffer ( const BYTE * sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;

	m_iOvershortCount = 0;
	m_bBoundary = m_bTokenBoundary = false;
}


template < bool IS_QUERY >
BYTE * CSphTokenizer_UTF8<IS_QUERY>::GetToken ()
{
	m_bWasSpecial = false;
	m_bBlended = false;
	m_iOvershortCount = 0;
	m_bTokenBoundary = false;
	m_bWasSynonym = false;

	return m_bHasBlend
		? DoGetToken<IS_QUERY,true>()
		: DoGetToken<IS_QUERY,false>();
}


bool CSphTokenizerBase2::CheckException ( const BYTE * pStart, const BYTE * pCur, bool bQueryMode )
{
	assert ( m_pExc );
	assert ( pStart );

	// at this point [pStart,pCur) is our regular tokenization candidate,
	// and pCur is pointing at what normally is considered separtor
	//
	// however, it might be either a full exception (if we're really lucky)
	// or (more likely) an exception prefix, so lets check for that
	//
	// interestingly enough, note that our token might contain a full exception
	// as a prefix, for instance [USAF] token vs [USA] exception; but in that case
	// we still need to tokenize regularly, because even exceptions need to honor
	// word boundaries

	// lets begin with a special (hopefully fast) check for the 1st byte
	const BYTE * p = pStart;
	if ( m_pExc->GetFirst ( *p )<0 )
		return false;

	// consume all the (character data) bytes until the first separator
	int iNode = 0;
	while ( p<pCur )
	{
		if ( bQueryMode && *p=='\\' )
		{
			p++;
			continue;;
		}
		iNode = m_pExc->GetNext ( iNode, *p++ );
		if ( iNode<0 )
			return false;
	}

	const BYTE * pMapEnd = nullptr; // the longest exception found so far is [pStart,pMapEnd)
	const BYTE * pMapTo = nullptr; // the destination mapping

	// now, we got ourselves a valid exception prefix, so lets keep consuming more bytes,
	// ie. until further separators, and keep looking for a full exception match
	while ( iNode>=0 )
	{
		// in query mode, ignore quoting slashes
		if ( bQueryMode && *p=='\\' )
		{
			p++;
			continue;
		}

		// decode one more codepoint, check if it is a separator
		bool bSep = true;
		bool bSpace = sphIsSpace(*p); // okay despite utf-8, cause hard whitespace is all ascii-7

		const BYTE * q = p;
		if ( p<m_pBufferMax )
			bSep = IsSeparator ( m_tLC.ToLower ( sphUTF8Decode(q) ), false ); // FIXME? sometimes they ARE first

		// there is a separator ahead, so check if we have a full match
		if ( bSep && m_pExc->GetMapping(iNode) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping(iNode);
		}

		// eof? bail
		if ( p>=m_pBufferMax )
			break;

		// not eof? consume those bytes
		if ( bSpace )
		{
			// and fold (hard) whitespace while we're at it!
			while ( sphIsSpace(*p) )
				p++;
			iNode = m_pExc->GetNext ( iNode, ' ' );
		} else
		{
			// just consume the codepoint, byte-by-byte
			while ( p<q && iNode>=0 )
				iNode = m_pExc->GetNext ( iNode, *p++ );
		}

		// we just consumed a separator, so check for a full match again
		if ( iNode>=0 && bSep && m_pExc->GetMapping(iNode) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping(iNode);
		}
	}

	// found anything?
	if ( !pMapTo )
		return false;

	strncpy ( (char*)m_sAccum, (char*)pMapTo, sizeof(m_sAccum)-1 );
	m_pCur = pMapEnd;
	m_pTokenStart = pStart;
	m_pTokenEnd = pMapEnd;
	m_iLastTokenLen = strlen ( (char*)m_sAccum );

	m_bWasSynonym = true;
	return true;
}

template < bool IS_QUERY, bool IS_BLEND >
BYTE * CSphTokenizerBase2::DoGetToken ()
{
	// return pending blending variants
	if_const ( IS_BLEND )
	{
		BYTE * pVar = GetBlendedVariant ();
		if ( pVar )
			return pVar;
		m_bBlendedPart = ( m_pBlendEnd!=nullptr );
	}

	// in query mode, lets capture (soft-whitespace hard-whitespace) sequences and adjust overshort counter
	// sample queries would be (one NEAR $$$) or (one | $$$ two) where $ is not a valid character
	bool bGotNonToken = ( !IS_QUERY || m_bPhrase ); // only do this in query mode, never in indexing mode, never within phrases
	bool bGotSoft = false; // hey Beavis he said soft huh huhhuh

	m_pTokenStart = nullptr;
	for ( ;; )
	{
		// get next codepoint
		const BYTE * const pCur = m_pCur; // to redo special char, if there's a token already

		int iCodePoint;
		int iCode;
		if ( pCur<m_pBufferMax && *pCur<128 )
		{
			iCodePoint = *m_pCur++;
			iCode = m_tLC.m_pChunk[0][iCodePoint];
		} else
		{
			iCodePoint = GetCodepoint(); // advances m_pCur
			iCode = m_tLC.ToLower ( iCodePoint );
		}

		// handle escaping
		const bool bWasEscaped = ( IS_QUERY && iCodePoint=='\\' ); // whether current codepoint was escaped
		if ( bWasEscaped )
		{
			iCodePoint = GetCodepoint();
			iCode = m_tLC.ToLower ( iCodePoint );
			if ( !Special2Simple ( iCode ) )
				iCode = 0;
		}

		// handle eof
		if ( iCode<0 )
		{
			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
				return m_sAccum;

			// skip trailing short word
			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen )
			{
				if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) )
				{
					if ( m_iLastTokenLen )
						m_iOvershortCount++;
					m_iLastTokenLen = 0;
					if_const ( IS_BLEND )
						BlendAdjust ( pCur );
					return nullptr;
				}
			}

			// keep token end here as BlendAdjust might change m_pCur
			m_pTokenEnd = m_pCur;

			// return trailing word
			if_const ( IS_BLEND && !BlendAdjust ( pCur ) )
				return nullptr;
			if_const ( IS_BLEND && m_bBlended )
				return GetBlendedVariant();
			return m_sAccum;
		}

		// handle all the flags..
		if_const ( IS_QUERY )
			iCode = CodepointArbitrationQ ( iCode, bWasEscaped, *m_pCur );
		else if ( m_bDetectSentences )
			iCode = CodepointArbitrationI ( iCode );

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle blended characters
		if_const ( IS_BLEND && ( iCode & FLAG_CODEPOINT_BLEND ) )
		{
			if ( m_pBlendEnd )
				iCode = 0;
			else
			{
				m_bBlended = true;
				m_pBlendStart = m_iAccum ? m_pTokenStart : pCur;
			}
		}

		// handle soft-whitespace-only tokens
		if ( !bGotNonToken && !m_iAccum )
		{
			if ( !bGotSoft )
			{
				// detect opening soft whitespace
				if ( ( iCode==0 && !IsWhitespace ( iCodePoint ) && !IsPunctuation ( iCodePoint ) )
					|| ( ( iCode & FLAG_CODEPOINT_BLEND ) && !m_iAccum ) )
				{
					bGotSoft = true;
				}
			} else
			{
				// detect closing hard whitespace or special
				// (if there was anything meaningful in the meantime, we must never get past the outer if!)
				if ( IsWhitespace ( iCodePoint ) || ( iCode & FLAG_CODEPOINT_SPECIAL ) )
				{
					m_iOvershortCount++;
					bGotNonToken = true;
				}
			}
		}

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) )
		{
			m_bTokenBoundary = true;
			m_iBoundaryOffset = pCur - m_pBuffer - 1;
		}
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;

		// handle separator (aka, most likely a token!)
		if ( iCode==0 || m_bBoundary )
		{
			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && CheckException ( m_pTokenStart ? m_pTokenStart : pCur, pCur, IS_QUERY ) )
				return m_sAccum;

			if_const ( IS_BLEND && !BlendAdjust ( pCur ) )
				continue;

			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen
				&& !( m_bShortTokenFilter && ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) ) )
			{
				if ( m_iLastTokenLen )
					m_iOvershortCount++;
				continue;
			} else
			{
				m_pTokenEnd = pCur;
				if_const ( IS_BLEND && m_bBlended )
					return GetBlendedVariant();
				return m_sAccum;
			}
		}

		// handle specials
		if ( iCode & FLAG_CODEPOINT_SPECIAL )
		{
			// skip short words preceding specials
			if ( m_iAccum<m_tSettings.m_iMinWordLen )
			{
				m_sAccum[m_iAccum] = '\0';

				if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iAccum ) )
				{
					if ( m_iAccum )
						m_iOvershortCount++;

					FlushAccum ();
				}
			}

			if ( m_iAccum==0 )
			{
				m_bNonBlended = m_bNonBlended || ( !( iCode & FLAG_CODEPOINT_BLEND ) && !( iCode & FLAG_CODEPOINT_SPECIAL ) );
				m_bWasSpecial = !( iCode & FLAG_CODEPOINT_NGRAM );
				m_pTokenStart = pCur;
				m_pTokenEnd = m_pCur;
				AccumCodepoint ( iCode & MASK_CODEPOINT ); // handle special as a standalone token
			} else
			{
				m_pCur = pCur; // we need to flush current accum and then redo special char again
				m_pTokenEnd = pCur;
			}

			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
				return m_sAccum;

			if_const ( IS_BLEND )
			{
				if ( !BlendAdjust ( pCur ) )
					continue;
				if ( m_bBlended )
					return GetBlendedVariant();
			}
			return m_sAccum;
		}

		if ( m_iAccum==0 )
			m_pTokenStart = pCur;

		// tricky bit
		// heading modifiers must not (!) affected blended status
		// eg. we want stuff like '=-' (w/o apostrophes) thrown away when pure_blend is on
		if_const ( IS_BLEND )
			if_const (!( IS_QUERY && !m_iAccum && sphIsModifier ( iCode & MASK_CODEPOINT ) ) )
				m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );

		// just accumulate
		// manual inlining of utf8 encoder gives us a few extra percent
		// which is important here, this is a hotspot
		if ( m_iAccum<SPH_MAX_WORD_LEN && ( m_pAccum-m_sAccum+SPH_MAX_UTF8_BYTES<=(int)sizeof(m_sAccum) ) )
		{
			iCode &= MASK_CODEPOINT;
			m_iAccum++;
			SPH_UTF8_ENCODE ( m_pAccum, iCode );
		}
	}
}


void CSphTokenizerBase2::FlushAccum ()
{
	assert ( m_pAccum-m_sAccum < (int)sizeof(m_sAccum) );
	m_iLastTokenLen = m_iAccum;
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}


template < bool IS_QUERY >
ISphTokenizer * CSphTokenizer_UTF8<IS_QUERY>::Clone ( ESphTokenizerClone eMode ) const
{
	CSphTokenizerBase * pClone;
	if ( eMode!=SPH_CLONE_INDEX )
		pClone = new CSphTokenizer_UTF8<true>();
	else
		pClone = new CSphTokenizer_UTF8<false>();
	pClone->CloneBase ( this, eMode );
	return pClone;
}


template < bool IS_QUERY >
int CSphTokenizer_UTF8<IS_QUERY>::GetCodepointLength ( int iCode ) const
{
	if ( iCode<128 )
		return 1;

	int iBytes = 0;
	while ( iCode & 0x80 )
	{
		iBytes++;
		iCode <<= 1;
	}

	assert ( iBytes>=2 && iBytes<=4 );
	return iBytes;
}

/////////////////////////////////////////////////////////////////////////////

template < bool IS_QUERY >
bool CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramChars ( const char * sConfig, CSphString & sError )
{
	assert ( this->m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	if ( !CheckRemap ( sError, dRemaps, "ngram", true, this->m_tLC ) )
		return false;

	// gcc braindamage requires this
	this->m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL ); // !COMMIT support other n-gram lengths than 1
	return true;
}


template < bool IS_QUERY >
void CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramLen ( int iLen )
{
	assert ( this->m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	assert ( iLen>0 );
	m_iNgramLen = iLen;
}


template < bool IS_QUERY >
BYTE * CSphTokenizer_UTF8Ngram<IS_QUERY>::GetToken ()
{
	// !COMMIT support other n-gram lengths than 1
	assert ( m_iNgramLen==1 );
	return CSphTokenizer_UTF8<IS_QUERY>::GetToken ();
}

//////////////////////////////////////////////////////////////////////////

CSphMultiformTokenizer::CSphMultiformTokenizer ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
	: CSphTokenFilter ( pTokenizer )
	, m_pMultiWordforms ( pContainer )
{
	assert ( pTokenizer && pContainer );
	m_dStoredTokens.Reserve ( pContainer->m_iMaxTokens + 6 ); // max form tokens + some blended tokens
	m_sTokenizedMultiform[0] = '\0';
}

BYTE * CSphMultiformTokenizer::GetToken ()
{
	if ( m_iOutputPending > -1 && m_pCurrentForm )
	{
		if ( ++m_iOutputPending>=m_pCurrentForm->m_dNormalForm.GetLength() )
		{
			m_iOutputPending = -1;
			m_pCurrentForm = nullptr;
		} else
		{
			StoredToken_t & tStart = m_dStoredTokens[m_iStart];
			strncpy ( (char *)tStart.m_sToken, m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_sForm.cstr(), sizeof(tStart.m_sToken)-1 );

			tStart.m_iTokenLen = m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_iLengthCP;
			tStart.m_bBoundary = false;
			tStart.m_bSpecial = false;
			tStart.m_bBlended = false;
			tStart.m_bBlendedPart = false;
			return tStart.m_sToken;
		}
	}

	m_sTokenizedMultiform[0] = '\0';
	m_iStart++;

	if ( m_iStart>=m_dStoredTokens.GetLength() )
	{
		m_iStart = 0;
		m_dStoredTokens.Resize ( 0 );
		const BYTE * pToken = CSphTokenFilter::GetToken();
		if ( !pToken )
			return NULL;

		FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		while ( m_dStoredTokens.Last().m_bBlended || m_dStoredTokens.Last().m_bBlendedPart )
		{
			pToken = CSphTokenFilter::GetToken ();
			if ( !pToken )
				break;

			FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		}
	}

	CSphMultiforms ** pWordforms = NULL;
	int iTokensGot = 1;
	bool bBlended = false;

	// check multi-form
	// only blended parts checked for multi-form with blended
	// in case ALL blended parts got transformed primary blended got replaced by normal form
	// otherwise blended tokens provided as is
	if ( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart )
	{
		if ( m_dStoredTokens[m_iStart].m_bBlended && m_iStart+1<m_dStoredTokens.GetLength() && m_dStoredTokens[m_iStart+1].m_bBlendedPart )
		{
			pWordforms = m_pMultiWordforms->m_Hash ( (const char *)m_dStoredTokens[m_iStart+1].m_sToken );
			if ( pWordforms )
			{
				bBlended = true;
				for ( int i=m_iStart+2; i<m_dStoredTokens.GetLength(); i++ )
				{
					// break out on blended over or got completely different blended
					if ( m_dStoredTokens[i].m_bBlended || !m_dStoredTokens[i].m_bBlendedPart )
						break;

					iTokensGot++;
				}
			}
		}
	} else
	{
		pWordforms = m_pMultiWordforms->m_Hash ( (const char *)m_dStoredTokens[m_iStart].m_sToken );
		if ( pWordforms )
		{
			int iTokensNeed = (*pWordforms)->m_iMaxTokens + 1;
			int iCur = m_iStart;
			bool bGotBlended = false;

			// collect up ahead to multi-form tokens or all blended tokens
			while ( iTokensGot<iTokensNeed || bGotBlended )
			{
				iCur++;
				if ( iCur>=m_dStoredTokens.GetLength() )
				{
					// fetch next token
					const BYTE* pToken = CSphTokenFilter::GetToken ();
					if ( !pToken )
						break;

					FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
				}

				bool bCurBleneded = ( m_dStoredTokens[iCur].m_bBlended || m_dStoredTokens[iCur].m_bBlendedPart );
				if ( bGotBlended && !bCurBleneded )
					break;

				bGotBlended = bCurBleneded;
				// count only regular tokens; can not fold mixed (regular+blended) tokens to form
				iTokensGot += ( bGotBlended ? 0 : 1 );
			}
		}
	}

	if ( !pWordforms || iTokensGot<(*pWordforms)->m_iMinTokens+1 )
		return m_dStoredTokens[m_iStart].m_sToken;

	int iStartToken = m_iStart + ( bBlended ? 1 : 0 );
	ARRAY_FOREACH ( i, (*pWordforms)->m_pForms )
	{
		const CSphMultiform * pCurForm = (*pWordforms)->m_pForms[i];
		int iFormTokCount = pCurForm->m_dTokens.GetLength();

		if ( iTokensGot<iFormTokCount+1 || ( bBlended && iTokensGot!=iFormTokCount+1 ) )
			continue;

		int iForm = 0;
		for ( ; iForm<iFormTokCount; iForm++ )
		{
			const StoredToken_t & tTok = m_dStoredTokens[iStartToken + 1 + iForm];
			const char * szStored = (const char*)tTok.m_sToken;
			const char * szNormal = pCurForm->m_dTokens[iForm].cstr ();

			if ( *szNormal!=*szStored || strcasecmp ( szNormal, szStored ) )
				break;
		}

		// early out - no destination form detected
		if ( iForm!=iFormTokCount )
			continue;

		// tokens after folded form are valid tail that should be processed next time
		if ( m_bBuildMultiform )
		{
			BYTE * pOut = m_sTokenizedMultiform;
			BYTE * pMax = pOut + sizeof(m_sTokenizedMultiform);
			for ( int j=0; j<iFormTokCount+1 && pOut<pMax; j++ )
			{
				const StoredToken_t & tTok = m_dStoredTokens[iStartToken+j];
				const BYTE * sTok = tTok.m_sToken;
				if ( j && pOut<pMax )
					*pOut++ = ' ';
				while ( *sTok && pOut<pMax )
					*pOut++ = *sTok++;
			}
			*pOut = '\0';
			*(pMax-1) = '\0';
		}

		if ( !bBlended )
		{
			// fold regular tokens to form
			const StoredToken_t & tStart = m_dStoredTokens[m_iStart];
			StoredToken_t & tEnd = m_dStoredTokens[m_iStart+iFormTokCount];
			m_iStart += iFormTokCount;

			strncpy ( (char *)tEnd.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof(tEnd.m_sToken)-1 );
			tEnd.m_szTokenStart = tStart.m_szTokenStart;
			tEnd.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;

			tEnd.m_bBoundary = false;
			tEnd.m_bSpecial = false;
			tEnd.m_bBlended = false;
			tEnd.m_bBlendedPart = false;

			if ( pCurForm->m_dNormalForm.GetLength()>1 )
			{
				m_iOutputPending = 0;
				m_pCurrentForm = pCurForm;
			}
		} else
		{
			// replace blended by form
			// FIXME: add multiple destination token support here (if needed)
			assert ( pCurForm->m_dNormalForm.GetLength()==1 );
			StoredToken_t & tDst = m_dStoredTokens[m_iStart];
			strncpy ( (char *)tDst.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof(tDst.m_sToken)-1 );
			tDst.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;
		}
		break;
	}

	return m_dStoredTokens[m_iStart].m_sToken;
}


ISphTokenizer * CSphMultiformTokenizer::Clone ( ESphTokenizerClone eMode ) const
{
	ISphTokenizerRefPtr_c pClone { m_pTokenizer->Clone ( eMode ) };
	return CreateMultiformFilter ( pClone, m_pMultiWordforms );
}


void CSphMultiformTokenizer::SetBufferPtr ( const char * sNewPtr )
{
	m_iStart = 0;
	m_iOutputPending = -1;
	m_pCurrentForm = nullptr;
	m_dStoredTokens.Resize ( 0 );
	CSphTokenFilter::SetBufferPtr ( sNewPtr );
}

void CSphMultiformTokenizer::SetBuffer ( const BYTE * sBuffer, int iLength )
{
	CSphTokenFilter::SetBuffer ( sBuffer, iLength );
	SetBufferPtr ( (const char *)sBuffer );
}

uint64_t CSphMultiformTokenizer::GetSettingsFNV () const
{
	uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
	uHash ^= (uint64_t)m_pMultiWordforms;
	return uHash;
}

int CSphMultiformTokenizer::SkipBlended ()
{
	bool bGotBlended = ( m_iStart<m_dStoredTokens.GetLength() &&
		( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart ) );
	if ( !bGotBlended )
		return 0;

	int iWasStart = m_iStart;
	for ( int iTok=m_iStart+1; iTok<m_dStoredTokens.GetLength() && m_dStoredTokens[iTok].m_bBlendedPart && !m_dStoredTokens[iTok].m_bBlended; iTok++ )
		m_iStart = iTok;

	return m_iStart-iWasStart;
}

bool CSphMultiformTokenizer::WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const
{
	if ( m_iOutputPending>-1 && m_pCurrentForm && m_pCurrentForm->m_dNormalForm.GetLength()>1 && m_iOutputPending<m_pCurrentForm->m_dNormalForm.GetLength() )
	{
		bHead = ( m_iOutputPending==0 );
		iDestCount = m_pCurrentForm->m_dNormalForm.GetLength();
		return true;
	} else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////
void CSphFilterSettings::SetExternalValues ( const SphAttr_t * pValues, int nValues )
{
	m_pValues = pValues;
	m_nValues = nValues;
}


bool CSphFilterSettings::operator == ( const CSphFilterSettings & rhs ) const
{
	// check name, mode, type
	if ( m_sAttrName!=rhs.m_sAttrName || m_bExclude!=rhs.m_bExclude || m_eType!=rhs.m_eType )
		return false;

	switch ( m_eType )
	{
		case SPH_FILTER_RANGE:
			return m_iMinValue==rhs.m_iMinValue && m_iMaxValue==rhs.m_iMaxValue;

		case SPH_FILTER_FLOATRANGE:
			return m_fMinValue==rhs.m_fMinValue && m_fMaxValue==rhs.m_fMaxValue;

		case SPH_FILTER_VALUES:
			if ( m_dValues.GetLength()!=rhs.m_dValues.GetLength() )
				return false;

			ARRAY_FOREACH ( i, m_dValues )
				if ( m_dValues[i]!=rhs.m_dValues[i] )
					return false;

			return true;

		case SPH_FILTER_STRING:
		case SPH_FILTER_USERVAR:
		case SPH_FILTER_STRING_LIST:
			if ( m_dStrings.GetLength ()!=rhs.m_dStrings.GetLength () )
				return false;
			ARRAY_FOREACH ( i, m_dStrings )
				if ( m_dStrings[i]!=rhs.m_dStrings[i] )
					return false;
			return ( m_eMvaFunc==rhs.m_eMvaFunc );

		default:
			assert ( 0 && "internal error: unhandled filter type in comparison" );
			return false;
	}
}


uint64_t CSphFilterSettings::GetHash() const
{
	uint64_t h = sphFNV64 ( &m_eType, sizeof(m_eType) );
	h = sphFNV64 ( &m_bExclude, sizeof(m_bExclude), h );
	switch ( m_eType )
	{
		case SPH_FILTER_VALUES:
			{
				int t = m_dValues.GetLength();
				h = sphFNV64 ( &t, sizeof(t), h );
				h = sphFNV64 ( m_dValues.Begin(), t*sizeof(SphAttr_t), h );
				break;
			}
		case SPH_FILTER_RANGE:
			h = sphFNV64 ( &m_iMaxValue, sizeof(m_iMaxValue), sphFNV64 ( &m_iMinValue, sizeof(m_iMinValue), h ) );
			break;
		case SPH_FILTER_FLOATRANGE:
			h = sphFNV64 ( &m_fMaxValue, sizeof(m_fMaxValue), sphFNV64 ( &m_fMinValue, sizeof(m_fMinValue), h ) );
			break;
		case SPH_FILTER_STRING:
		case SPH_FILTER_USERVAR:
		case SPH_FILTER_STRING_LIST:
			ARRAY_FOREACH ( iString, m_dStrings )
				h = sphFNV64cont ( m_dStrings[iString].cstr(), h );
			if ( m_eMvaFunc!=SPH_MVAFUNC_NONE )
				h = sphFNV64 ( &m_eMvaFunc, sizeof ( m_eMvaFunc ), h );
		break;
		case SPH_FILTER_NULL:
			break;
		default:
			assert ( 0 && "internal error: unhandled filter type in GetHash()" );
	}
	return h;
}


bool FilterTreeItem_t::operator == ( const FilterTreeItem_t & rhs ) const
{
	return ( m_iLeft==rhs.m_iLeft && m_iRight==rhs.m_iRight && m_iFilterItem==rhs.m_iFilterItem && m_bOr==rhs.m_bOr );
}


uint64_t FilterTreeItem_t::GetHash() const
{
	uint64_t uHash = sphFNV64 ( &m_iLeft, sizeof(m_iLeft) );
	uHash = sphFNV64 ( &m_iRight, sizeof(m_iRight), uHash );
	uHash = sphFNV64 ( &m_iFilterItem, sizeof(m_iFilterItem), uHash );
	uHash = sphFNV64 ( &m_bOr, sizeof(m_bOr), uHash );
	return uHash;
}


/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

struct SelectBounds_t
{
	int		m_iStart;
	int		m_iEnd;
};
#define YYSTYPE SelectBounds_t
class SelectParser_t;

#ifdef CMAKE_GENERATED_GRAMMAR
	#include "bissphinxselect.h"
#else
	#include "yysphinxselect.h"
#endif


class SelectParser_t
{
public:
	int				GetToken ( YYSTYPE * lvalp );
	void			AddItem ( YYSTYPE * pExpr, ESphAggrFunc eAggrFunc=SPH_AGGR_NONE, YYSTYPE * pStart=NULL, YYSTYPE * pEnd=NULL );
	void			AddItem ( const char * pToken, YYSTYPE * pStart=NULL, YYSTYPE * pEnd=NULL );
	void			AliasLastItem ( YYSTYPE * pAlias );
	void			AddOption ( YYSTYPE * pOpt, YYSTYPE * pVal );

private:
	void			AutoAlias ( CSphQueryItem & tItem, YYSTYPE * pStart, YYSTYPE * pEnd );
	bool			IsTokenEqual ( YYSTYPE * pTok, const char * sRef );

public:
	CSphString		m_sParserError;
	const char *	m_pLastTokenStart;

	const char *	m_pStart;
	const char *	m_pCur;

	CSphQuery *		m_pQuery;
};

int yylex ( YYSTYPE * lvalp, SelectParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( SelectParser_t * pParser, const char * sMessage )
{
	pParser->m_sParserError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#ifdef CMAKE_GENERATED_GRAMMAR
#include "bissphinxselect.c"
#else

#include "yysphinxselect.c"

#endif


int SelectParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace ( *m_pCur ) )
		m_pCur++;
	if ( !*m_pCur )
		return 0;

	// begin working that token
	m_pLastTokenStart = m_pCur;
	lvalp->m_iStart = m_pCur-m_pStart;

	// check for constant
	if ( isdigit ( *m_pCur ) )
	{
		char * pEnd = NULL;
		double VARIABLE_IS_NOT_USED fDummy = strtod ( m_pCur, &pEnd );
		m_pCur = pEnd;
		lvalp->m_iEnd = m_pCur-m_pStart;
		return SEL_TOKEN;
	}

	// check for token
	if ( sphIsAttr ( m_pCur[0] ) || ( m_pCur[0]=='@' && sphIsAttr ( m_pCur[1] ) && !isdigit ( m_pCur[1] ) ) )
	{
		m_pCur++;
		while ( sphIsAttr ( *m_pCur ) ) m_pCur++;
		lvalp->m_iEnd = m_pCur-m_pStart;

		#define LOC_CHECK(_str,_len,_ret) \
			if ( lvalp->m_iEnd==_len+lvalp->m_iStart && strncasecmp ( m_pStart+lvalp->m_iStart, _str, _len )==0 ) return _ret;

		LOC_CHECK ( "ID", 2, SEL_ID );
		LOC_CHECK ( "AS", 2, SEL_AS );
		LOC_CHECK ( "OR", 2, TOK_OR );
		LOC_CHECK ( "AND", 3, TOK_AND );
		LOC_CHECK ( "NOT", 3, TOK_NOT );
		LOC_CHECK ( "DIV", 3, TOK_DIV );
		LOC_CHECK ( "MOD", 3, TOK_MOD );
		LOC_CHECK ( "AVG", 3, SEL_AVG );
		LOC_CHECK ( "MIN", 3, SEL_MIN );
		LOC_CHECK ( "MAX", 3, SEL_MAX );
		LOC_CHECK ( "SUM", 3, SEL_SUM );
		LOC_CHECK ( "GROUP_CONCAT", 12, SEL_GROUP_CONCAT );
		LOC_CHECK ( "GROUPBY", 7, SEL_GROUPBY );
		LOC_CHECK ( "COUNT", 5, SEL_COUNT );
		LOC_CHECK ( "DISTINCT", 8, SEL_DISTINCT );
		LOC_CHECK ( "WEIGHT", 6, SEL_WEIGHT );
		LOC_CHECK ( "OPTION", 6, SEL_OPTION );
		LOC_CHECK ( "IS", 2, TOK_IS );
		LOC_CHECK ( "NULL", 4, TOK_NULL );
		LOC_CHECK ( "FOR", 3, TOK_FOR );
		LOC_CHECK ( "IN", 2, TOK_FUNC_IN );
		LOC_CHECK ( "RAND", 4, TOK_FUNC_RAND );

		#undef LOC_CHECK

		return SEL_TOKEN;
	}

	// check for equality checks
	lvalp->m_iEnd = 1+lvalp->m_iStart;
	switch ( *m_pCur )
	{
		case '<':
			m_pCur++;
			if ( *m_pCur=='>' ) { m_pCur++; lvalp->m_iEnd++; return TOK_NE; }
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; return TOK_LTE; }
			return '<';

		case '>':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; return TOK_GTE; }
			return '>';

		case '=':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; }
			return TOK_EQ;

		case '\'':
		{
			const char cEnd = *m_pCur;
			for ( const char * s = m_pCur+1; *s; s++ )
			{
				if ( *s==cEnd && s-1>=m_pCur && *(s-1)!='\\' )
				{
					m_pCur = s+1;
					return TOK_CONST_STRING;
				}
			}
			return -1;
		}
	}

	// check for comment begin/end
	if ( m_pCur[0]=='/' && m_pCur[1]=='*' )
	{
		m_pCur += 2;
		lvalp->m_iEnd += 1;
		return SEL_COMMENT_OPEN;
	}
	if ( m_pCur[0]=='*' && m_pCur[1]=='/' )
	{
		m_pCur += 2;
		lvalp->m_iEnd += 1;
		return SEL_COMMENT_CLOSE;
	}

	// return char as a token
	return *m_pCur++;
}

void SelectParser_t::AutoAlias ( CSphQueryItem & tItem, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	if ( pStart && pEnd )
	{
		tItem.m_sAlias.SetBinary ( m_pStart + pStart->m_iStart, pEnd->m_iEnd - pStart->m_iStart );
		sphColumnToLowercase ( const_cast<char *>( tItem.m_sAlias.cstr() ) ); // as in SqlParser_c
	} else
		tItem.m_sAlias = tItem.m_sExpr;
}

void SelectParser_t::AddItem ( YYSTYPE * pExpr, ESphAggrFunc eAggrFunc, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr.SetBinary ( m_pStart + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	tItem.m_eAggrFunc = eAggrFunc;
	AutoAlias ( tItem, pStart, pEnd );
}

void SelectParser_t::AddItem ( const char * pToken, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = pToken;
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	AutoAlias ( tItem, pStart, pEnd );
}

void SelectParser_t::AliasLastItem ( YYSTYPE * pAlias )
{
	if ( pAlias )
	{
		CSphQueryItem & tItem = m_pQuery->m_dItems.Last();
		tItem.m_sAlias.SetBinary ( m_pStart + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
		tItem.m_sAlias.ToLower();
	}
}

bool SelectParser_t::IsTokenEqual ( YYSTYPE * pTok, const char * sRef )
{
	int iLen = strlen(sRef);
	if ( iLen!=( pTok->m_iEnd - pTok->m_iStart ) )
		return false;
	return strncasecmp ( m_pStart + pTok->m_iStart, sRef, iLen )==0;
}

void SelectParser_t::AddOption ( YYSTYPE * pOpt, YYSTYPE * pVal )
{
	if ( IsTokenEqual ( pOpt, "reverse_scan" ) )
	{
		if ( IsTokenEqual ( pVal, "1" ) )
			m_pQuery->m_bReverseScan = true;
	} else if ( IsTokenEqual ( pOpt, "sort_method" ) )
	{
		if ( IsTokenEqual ( pVal, "kbuffer" ) )
			m_pQuery->m_bSortKbuffer = true;
	} else if ( IsTokenEqual ( pOpt, "max_predicted_time" ) )
	{
		char szNumber[256];
		int iLen = pVal->m_iEnd-pVal->m_iStart;
		assert ( iLen < (int)sizeof(szNumber) );
		strncpy ( szNumber, m_pStart+pVal->m_iStart, iLen );
		int64_t iMaxPredicted = strtoull ( szNumber, NULL, 10 );
		m_pQuery->m_iMaxPredictedMsec = int(iMaxPredicted > INT_MAX ? INT_MAX : iMaxPredicted );
	}
}

bool ParseSelectList ( CSphString & sError, CSphQuery & tQuery )
{
	tQuery.m_dItems.Reset();
	if ( tQuery.m_sSelect.IsEmpty() )
		return true; // empty is ok; will just return everything

	SelectParser_t tParser;
	tParser.m_pStart = tQuery.m_sSelect.cstr();
	tParser.m_pCur = tParser.m_pStart;
	tParser.m_pQuery = &tQuery;

	yyparse ( &tParser );

	sError = tParser.m_sParserError;
	return sError.IsEmpty ();
}


int ExpandKeywords ( int iIndexOpt, QueryOption_e eQueryOpt, const CSphIndexSettings & tSettings )
{
	if ( tSettings.m_iMinInfixLen<=0 && tSettings.m_iMinPrefixLen<=0 && !tSettings.m_bIndexExactWords )
		return KWE_DISABLED;

	int iOpt = KWE_DISABLED;
	if ( eQueryOpt==QUERY_OPT_DEFAULT )
		iOpt = iIndexOpt;
	else if ( eQueryOpt==QUERY_OPT_MORPH_NONE )
		iOpt = KWE_MORPH_NONE;
	 else
		iOpt = ( eQueryOpt==QUERY_OPT_ENABLED ? KWE_ENABLED : KWE_DISABLED );

	if ( ( iOpt & KWE_STAR )==KWE_STAR && tSettings.m_iMinInfixLen<=0 && tSettings.m_iMinPrefixLen<=0 )
		iOpt ^= KWE_STAR;

	if ( ( iOpt & KWE_EXACT )==KWE_EXACT && !tSettings.m_bIndexExactWords )
		iOpt ^= KWE_EXACT;

	if ( ( iOpt & KWE_MORPH_NONE )==KWE_MORPH_NONE && !tSettings.m_bIndexExactWords )
		iOpt ^= KWE_MORPH_NONE;

	return iOpt;
}


/////////////////////////////////////////////////////////////////////////////
// QUERY STATS
/////////////////////////////////////////////////////////////////////////////
void CSphQueryStats::Add ( const CSphQueryStats & tStats )
{
	m_iFetchedDocs += tStats.m_iFetchedDocs;
	m_iFetchedHits += tStats.m_iFetchedHits;
	m_iSkips += tStats.m_iSkips;
}


/////////////////////////////////////////////////////////////////////////////
// SCHEMAS
/////////////////////////////////////////////////////////////////////////////

static CSphString sphDumpAttr ( const CSphColumnInfo & tAttr )
{
	CSphString sRes;
	sRes.SetSprintf ( "%s %s:%d@%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_sName.cstr(),
		tAttr.m_tLocator.m_iBitCount, tAttr.m_tLocator.m_iBitOffset );
	return sRes;
}


/// make string lowercase but keep case of JSON.field
void sphColumnToLowercase ( char * sVal )
{
	if ( !sVal || !*sVal )
		return;

	// make all chars lowercase but only prior to '.', ',', and '[' delimiters
	// leave quoted values unchanged
	for ( bool bQuoted=false; *sVal && *sVal!='.' && *sVal!=',' && *sVal!='['; sVal++ )
	{
		if ( !bQuoted )
			*sVal = (char) tolower ( *sVal );
		if ( *sVal=='\'' )
			bQuoted = !bQuoted;
	}
}

//////////////////////////////////////////////////////////////////////////

CSphColumnInfo::CSphColumnInfo ( const char * sName, ESphAttr eType )
	: m_sName ( sName )
	, m_eAttrType ( eType )
{
	sphColumnToLowercase ( const_cast<char *>( m_sName.cstr() ) );
}


bool CSphColumnInfo::IsDataPtr() const
{
	return sphIsDataPtrAttr ( m_eAttrType );
}


//////////////////////////////////////////////////////////////////////////

void CSphSchemaHelper::InsertAttr ( CSphVector<CSphColumnInfo> & dAttrs, CSphVector<int> & dUsed, int iPos, const CSphColumnInfo & tCol, bool bDynamic )
{
	assert ( 0<=iPos && iPos<=dAttrs.GetLength() );
	assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
	if ( tCol.m_eAttrType==SPH_ATTR_NONE )
		return;

	dAttrs.Insert ( iPos, tCol );
	CSphAttrLocator & tLoc = dAttrs [ iPos ].m_tLocator;

	int iBits = ROWITEM_BITS;
	if ( tLoc.m_iBitCount>0 )
		iBits = tLoc.m_iBitCount;
	if ( tCol.m_eAttrType==SPH_ATTR_BOOL )
		iBits = 1;
	if ( tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		iBits = 64;

	if ( tCol.IsDataPtr() )
	{
		assert ( bDynamic );
		iBits = ROWITEMPTR_BITS;
		m_dDataPtrAttrs.Add ( dUsed.GetLength() );
	}

	tLoc.m_iBitCount = iBits;
	tLoc.m_bDynamic = bDynamic;

	if ( sphIsBlobAttr ( tCol.m_eAttrType ) )
	{
		tLoc.m_iBitCount = 0;
		tLoc.m_iBitOffset = 0;

		int iBlobAttrId = 0;
		for ( auto & i : dAttrs )
			if ( sphIsBlobAttr ( i.m_eAttrType ) )
				i.m_tLocator.m_iBlobAttrId = iBlobAttrId++;

		for ( auto & i : dAttrs )
			if ( sphIsBlobAttr ( i.m_eAttrType ) )
				i.m_tLocator.m_nBlobAttrs = iBlobAttrId;

	} else if ( iBits>=ROWITEM_BITS )
	{
		tLoc.m_iBitOffset = dUsed.GetLength()*ROWITEM_BITS;
		int iItems = (iBits+ROWITEM_BITS-1) / ROWITEM_BITS;
		for ( int i=0; i<iItems; i++ )
			dUsed.Add ( ROWITEM_BITS );
	} else
	{
		int iItem;
		for ( iItem=0; iItem<dUsed.GetLength(); iItem++ )
			if ( dUsed[iItem]+iBits<=ROWITEM_BITS )
				break;
		if ( iItem==dUsed.GetLength() )
			dUsed.Add ( 0 );
		tLoc.m_iBitOffset = iItem*ROWITEM_BITS + dUsed[iItem];
		dUsed[iItem] += iBits;
	}
}


void CSphSchemaHelper::Reset()
{
	m_dDataPtrAttrs.Reset();
	m_dDynamicUsed.Reset ();
}

void CSphSchemaHelper::CloneMatch ( CSphMatch * pDst, const CSphMatch & rhs ) const
{
	CloneMatchSpecial ( pDst, rhs, m_dDataPtrAttrs );
}

void CSphSchemaHelper::FreeDataPtrs ( CSphMatch * pMatch ) const
{
	FreeDataSpecial ( pMatch, m_dDataPtrAttrs );
}

void CSphSchemaHelper::CopyPtrs ( CSphMatch * pDst, const CSphMatch &rhs ) const
{
	// notes on PACKEDFACTORS
	// not immediately obvious: this is not needed while pushing matches to sorters; factors are held in an outer hash table
	// but it is necessary to copy factors when combining results from several indexes via a sorter because at this moment matches are the owners of factor data
	CopyPtrsSpecial ( pDst, rhs.m_pDynamic, m_dDataPtrAttrs );
}

CSphVector<int> CSphSchemaHelper::SubsetPtrs ( CSphVector<int> &dDiscarded ) const
{
	CSphVector<int> dFiltered;
	dDiscarded.Uniq ();
	for ( int iPtr : m_dDataPtrAttrs )
		if ( !dDiscarded.BinarySearch ( iPtr ) )
			dFiltered.Add ( iPtr );
	dFiltered.Uniq ();
	return dFiltered;
}

void CSphSchemaHelper::CloneMatchSpecial ( CSphMatch * pDst, const CSphMatch &rhs, const CSphVector<int> &dSpecials ) const
{
	assert ( pDst );
	FreeDataSpecial ( pDst, dSpecials );
	pDst->Combine ( rhs, GetDynamicSize () );
	CopyPtrsSpecial ( pDst, rhs.m_pDynamic, dSpecials );
}

// declared in sphinxstd.h
void sphDeallocatePacked ( BYTE * pBlob )
{
	if ( !pBlob )
		return;
	//const BYTE * pFoo = pBlob;
	//sphDeallocateSmall ( pBlob, sphCalcPackedLength ( sphUnzipInt ( pFoo ) ) );
	sphDeallocateSmall ( pBlob );
}

void CSphSchemaHelper::FreeDataSpecial ( CSphMatch * pMatch, const CSphVector<int> &dSpecials )
{
	assert ( pMatch );
	if ( !pMatch->m_pDynamic )
		return;

	for ( auto iOffset : dSpecials )
	{
		BYTE * &pData = *( BYTE ** ) ( pMatch->m_pDynamic + iOffset );
		sphDeallocatePacked ( pData );
		pData = nullptr;
	}
}

inline int CSphRsetSchema::ActualLen() const
{
	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttrsCount () - m_dRemoved.GetLength ();
	return 0;
}

void CSphSchemaHelper::CopyPtrsSpecial ( CSphMatch * pDst, const void* _pSrc, const CSphVector<int> &dSpecials )
{
	auto pSrc = (const CSphRowitem *) _pSrc;
	for ( auto i : dSpecials )
	{
		const BYTE * pData = *( BYTE ** ) ( pSrc + i );
		if ( pData )
		{
			int nBytes = sphUnpackPtrAttr ( pData, &pData );
			*( BYTE ** ) ( pDst->m_pDynamic + i ) = sphPackPtrAttr ( pData, nBytes );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

CSphSchema::CSphSchema ( const char * sName )
	: m_sName ( sName )
{
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ) );
}


CSphSchema::CSphSchema ( const CSphSchema & rhs )
{
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;
	m_iFirstFieldLenAttr = rhs.m_iFirstFieldLenAttr;
	m_iLastFieldLenAttr = rhs.m_iLastFieldLenAttr;
	m_iRowSize = rhs.m_iRowSize;
	m_sName = rhs.m_sName;
	m_dFields = rhs.m_dFields;
	m_dAttrs = rhs.m_dAttrs;
	m_dStaticUsed = rhs.m_dStaticUsed;
	m_dDynamicUsed = rhs.m_dDynamicUsed;
	memcpy ( m_dBuckets, rhs.m_dBuckets, sizeof ( m_dBuckets ));
}

CSphSchema::CSphSchema ( CSphSchema && rhs ) noexcept
	: CSphSchema ( nullptr )
{
	Swap(rhs);
}

void CSphSchema::Swap ( CSphSchema& rhs ) noexcept
{
	::Swap ( m_dDataPtrAttrs, rhs.m_dDataPtrAttrs );
	::Swap ( m_iFirstFieldLenAttr, rhs.m_iFirstFieldLenAttr );
	::Swap ( m_iLastFieldLenAttr, rhs.m_iLastFieldLenAttr );
	::Swap ( m_iRowSize, rhs.m_iRowSize );
	::Swap ( m_sName, rhs.m_sName );
	::Swap ( m_dFields, rhs.m_dFields );
	::Swap ( m_dAttrs, rhs.m_dAttrs );
	::Swap ( m_dStaticUsed, rhs.m_dStaticUsed );
	::Swap ( m_dDynamicUsed, rhs.m_dDynamicUsed );
	std::swap ( m_dBuckets, rhs.m_dBuckets );
}


CSphSchema & CSphSchema::operator = ( const ISphSchema & rhs )
{
	Reset();

	m_sName = rhs.GetName();

	for ( int i = 0; i < rhs.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = rhs.GetAttr(i);

		if ( tAttr.m_tLocator.m_bDynamic )
			AddAttr ( tAttr, true );
		else // static attr, keep previous storage
			m_dAttrs.Add ( tAttr );
	}

	for ( int i = 0 ; i < rhs.GetFieldsCount(); i++ )
		AddField ( rhs.GetField(i) );

	RebuildHash();

	return *this;
}



CSphSchema & CSphSchema::operator = ( CSphSchema rhs )
{
	Swap ( rhs );
	return *this;
}

const char * CSphSchema::GetName() const
{
	return m_sName.cstr();
}


bool CSphSchema::CompareTo ( const CSphSchema & rhs, CSphString & sError, bool bFullComparison ) const
{
	// check attr count
	if ( GetAttrsCount()!=rhs.GetAttrsCount() )
	{
		sError.SetSprintf ( "attribute count mismatch (me=%s, in=%s, myattrs=%d, inattrs=%d)", m_sName.cstr(), rhs.m_sName.cstr(), GetAttrsCount(), rhs.GetAttrsCount() );
		return false;
	}

	// check attrs
	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const CSphColumnInfo & tAttr1 = rhs.m_dAttrs[i];
		const CSphColumnInfo & tAttr2 = m_dAttrs[i];

		bool bMismatch;
		if ( bFullComparison )
			bMismatch = !(tAttr1==tAttr2);
		else
		{
			ESphAttr eAttr1 = tAttr1.m_eAttrType;
			ESphAttr eAttr2 = tAttr2.m_eAttrType;

			bMismatch = tAttr1.m_sName!=tAttr2.m_sName || eAttr1!=eAttr2 || tAttr1.m_eWordpart!=tAttr2.m_eWordpart ||
				tAttr1.m_tLocator.m_iBitCount!=tAttr2.m_tLocator.m_iBitCount ||
				tAttr1.m_tLocator.m_iBitOffset!=tAttr2.m_tLocator.m_iBitOffset;
		}

		if ( bMismatch )
		{
			sError.SetSprintf ( "attribute mismatch (me=%s, in=%s, idx=%d, myattr=%s, inattr=%s)",
				m_sName.cstr(), rhs.m_sName.cstr(), i, sphDumpAttr ( m_dAttrs[i] ).cstr(), sphDumpAttr ( rhs.m_dAttrs[i] ).cstr() );
			return false;
		}
	}

	// check field count
	if ( rhs.m_dFields.GetLength()!=m_dFields.GetLength() )
	{
		sError.SetSprintf ( "fulltext fields count mismatch (me=%s, in=%s, myfields=%d, infields=%d)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			m_dFields.GetLength(), rhs.m_dFields.GetLength() );
		return false;
	}

	// check fulltext field names
	ARRAY_FOREACH ( i, rhs.m_dFields )
		if ( rhs.m_dFields[i].m_sName!=m_dFields[i].m_sName )
	{
		sError.SetSprintf ( "fulltext field mismatch (me=%s, myfield=%s, idx=%d, in=%s, infield=%s)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			i, m_dFields[i].m_sName.cstr(), rhs.m_dFields[i].m_sName.cstr() );
		return false;
	}

	return true;
}


int CSphSchema::GetFieldIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;
	ARRAY_FOREACH ( i, m_dFields )
		if ( strcasecmp ( m_dFields[i].m_sName.cstr(), sName )==0 )
			return i;
	return -1;
}


int CSphSchema::GetAttrIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;

	if ( m_dAttrs.GetLength()>=HASH_THRESH )
	{
		DWORD uCrc = sphCRC32 ( sName );
		DWORD uPos = m_dBuckets [ uCrc%BUCKET_COUNT ];
		while ( uPos!=0xffff && m_dAttrs [ uPos ].m_sName!=sName )
			uPos = m_dAttrs [ uPos ].m_uNext;

		return (short)uPos; // 0xffff == -1 is our "end of list" marker
	}

	ARRAY_FOREACH ( i, m_dAttrs )
		if ( m_dAttrs[i].m_sName==sName )
			return i;

	return -1;
}


const CSphColumnInfo * CSphSchema::GetAttr ( const char * sName ) const
{
	int iIndex = GetAttrIndex ( sName );
	if ( iIndex>=0 )
		return &m_dAttrs[iIndex];
	return NULL;
}


void CSphSchema::Reset ()
{
	CSphSchemaHelper::Reset();

	m_dFields.Reset();
	m_dAttrs.Reset();
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ));
	m_dStaticUsed.Reset();
	m_iRowSize = 0;
}


void CSphSchema::InsertAttr ( int iPos, const CSphColumnInfo & tCol, bool bDynamic )
{
	// it's redundant in case of AddAttr
	if ( iPos!=m_dAttrs.GetLength() )
		UpdateHash ( iPos-1, 1 );

	CSphSchemaHelper::InsertAttr ( m_dAttrs, bDynamic ? m_dDynamicUsed : m_dStaticUsed, iPos, tCol, bDynamic );

	// update field length locators
	if ( tCol.m_eAttrType==SPH_ATTR_TOKENCOUNT )
	{
		m_iFirstFieldLenAttr = m_iFirstFieldLenAttr==-1 ? iPos : Min ( m_iFirstFieldLenAttr, iPos );
		m_iLastFieldLenAttr = Max ( m_iLastFieldLenAttr, iPos );
	}

	// do hash add
	if ( m_dAttrs.GetLength()==HASH_THRESH )
		RebuildHash();
	else if ( m_dAttrs.GetLength()>HASH_THRESH )
	{
		WORD & uPos = GetBucketPos ( m_dAttrs [ iPos ].m_sName.cstr() );
		m_dAttrs [ iPos ].m_uNext = uPos;
		uPos = (WORD)iPos;
	}

	m_iRowSize = GetRowSize();
}


void CSphSchema::RemoveAttr ( const char * szAttr, bool bDynamic )
{
	int iIndex = GetAttrIndex ( szAttr );
	if ( iIndex<0 )
		return;

	CSphVector<CSphColumnInfo> dBackup = m_dAttrs;

	if ( bDynamic )
		m_dDynamicUsed.Reset();
	else
		m_dStaticUsed.Reset();

	CSphSchemaHelper::Reset();
	m_dAttrs.Reset();
	m_iFirstFieldLenAttr = -1;
	m_iLastFieldLenAttr = -1;

	ARRAY_FOREACH ( i, dBackup )
		if ( i!=iIndex )
			AddAttr ( dBackup[i], bDynamic );
}


void CSphSchema::AddAttr ( const CSphColumnInfo & tCol, bool bDynamic )
{
	InsertAttr ( m_dAttrs.GetLength(), tCol, bDynamic );
}


void CSphSchema::AddField ( const char * szFieldName )
{
	CSphColumnInfo & tField = m_dFields.Add();
	tField.m_sName = szFieldName;
}


void CSphSchema::AddField ( const CSphColumnInfo & tField )
{
	m_dFields.Add ( tField );
}


int CSphSchema::GetAttrId_FirstFieldLen() const
{
	return m_iFirstFieldLenAttr;
}


int CSphSchema::GetAttrId_LastFieldLen() const
{
	return m_iLastFieldLenAttr;
}


bool CSphSchema::IsReserved ( const char * szToken )
{
	static const char * dReserved[] =
	{
		"AND", "AS", "AT", "BY", "DIV", "DEBUG", "FACET", "FALSE", "FROM", "FORCE", "IGNORE", "IN", "INDEXES", "IS", "JOIN", "LIMIT",
		"LOGS",	"MOD", "NOT", "NULL", "OR", "ORDER", "REGEX", "RELOAD", "SELECT", "SYSFILTERS", "TRUE", "USE", "KILLLIST_TARGET",
		"WAIT_TIMEOUT", nullptr
	};

	const char ** p = dReserved;
	while ( *p )
		if ( strcasecmp ( szToken, *p++ )==0 )
			return true;
	return false;
}


WORD & CSphSchema::GetBucketPos ( const char * sName )
{
	DWORD uCrc = sphCRC32 ( sName );
	return m_dBuckets [ uCrc % BUCKET_COUNT ];
}


void CSphSchema::RebuildHash ()
{
	if ( m_dAttrs.GetLength()<HASH_THRESH )
		return;

	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ));

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		WORD & uPos = GetBucketPos ( m_dAttrs[i].m_sName.cstr() );
		m_dAttrs[i].m_uNext = uPos;
		uPos = WORD(i);
	}
}


void CSphSchema::UpdateHash ( int iStartIndex, int iAddVal )
{
	if ( m_dAttrs.GetLength()<HASH_THRESH )
		return;

	for ( auto &dAttr : m_dAttrs )
	{
		WORD &uPos = dAttr.m_uNext;
		if ( uPos!=0xffff && uPos>iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
	}
	for ( WORD &uPos : m_dBuckets )
		if ( uPos!=0xffff && uPos>iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
}


void CSphSchema::AssignTo ( CSphRsetSchema & lhs ) const
{
	lhs = *this;
}


void CSphSchema::CloneWholeMatch ( CSphMatch * pDst, const CSphMatch & rhs ) const
{
	assert ( pDst );
	FreeDataPtrs ( pDst );
	pDst->Combine ( rhs, GetRowSize() );
	CopyPtrs ( pDst, rhs );
}


void CSphSchema::SetFieldWordpart ( int iField, ESphWordpart eWordpart )
{
	m_dFields[iField].m_eWordpart = eWordpart;
}


void CSphSchema::SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs )
{
	m_dAttrs.SwapData(dAttrs);
	RebuildHash();
	m_iRowSize = GetRowSize();
}


bool CSphSchema::HasBlobAttrs() const
{
	for ( const auto & i : m_dAttrs )
		if ( sphIsBlobAttr ( i.m_eAttrType ) )
			return true;

	return false;
}


inline int CSphSchema::GetCachedRowSize() const
{
	return m_iRowSize;
}

//////////////////////////////////////////////////////////////////////////

void CSphRsetSchema::Reset ()
{
	CSphSchemaHelper::Reset();

	m_pIndexSchema = nullptr;
	m_dExtraAttrs.Reset();
}


void CSphRsetSchema::AddAttr ( const CSphColumnInfo & tCol, bool bDynamic )
{
	assert ( bDynamic );
	InsertAttr ( m_dExtraAttrs, m_dDynamicUsed, m_dExtraAttrs.GetLength(), tCol, true );
}


const char * CSphRsetSchema::GetName() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetName() : nullptr;
}


int CSphRsetSchema::GetRowSize() const
{
	// we copy over dynamic map in case index schema has dynamic attributes
	// (that happens in case of inline attributes, or RAM segments in RT indexes)
	// so there is no need to add GetDynamicSize() here
	return GetDynamicSize () + ( m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0 );
}


int CSphRsetSchema::GetStaticSize() const
{
	// result set schemas additions are always dynamic
	return m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0;
}


int CSphRsetSchema::GetAttrsCount() const
{
	return m_dExtraAttrs.GetLength () + ActualLen();
}


int CSphRsetSchema::GetFieldsCount() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetFieldsCount() : 0;
}


int CSphRsetSchema::GetAttrIndex ( const char * sName ) const
{
	ARRAY_FOREACH ( i, m_dExtraAttrs )
		if ( m_dExtraAttrs[i].m_sName==sName )
			return i + ActualLen();

	if ( !m_pIndexSchema )
		return -1;

	int iRes = m_pIndexSchema->GetAttrIndex(sName);
	if ( iRes>=0 )
	{
		if ( m_dRemoved.Contains ( iRes ) )
			return -1;
		int iSub = 0;
		ARRAY_FOREACH_COND ( i, m_dRemoved, iRes>=m_dRemoved[i] )
			iSub++;
		return iRes - iSub;
	}
	return -1;
}


const CSphColumnInfo & CSphRsetSchema::GetField ( int iIndex ) const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetField(iIndex);
}


const CSphVector<CSphColumnInfo> & CSphRsetSchema::GetFields() const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetFields();
}


const CSphColumnInfo & CSphRsetSchema::GetAttr ( int iIndex ) const
{
	if ( !m_pIndexSchema )
		return m_dExtraAttrs[iIndex];

	if ( iIndex<ActualLen () )
	{
		ARRAY_FOREACH_COND ( i, m_dRemoved, iIndex>=m_dRemoved[i] )
			++iIndex;

		return m_pIndexSchema->GetAttr(iIndex);
	}

	return m_dExtraAttrs [ iIndex - ActualLen() ];
}


const CSphColumnInfo * CSphRsetSchema::GetAttr ( const char * sName ) const
{
	for ( auto & tExtraAttr : m_dExtraAttrs )
		if ( tExtraAttr.m_sName==sName )
			return &tExtraAttr;

	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttr ( sName );

	return nullptr;
}


int CSphRsetSchema::GetAttrId_FirstFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_FirstFieldLen();
}


int CSphRsetSchema::GetAttrId_LastFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_LastFieldLen();
}


CSphRsetSchema & CSphRsetSchema::operator = ( const ISphSchema & rhs )
{
	rhs.AssignTo ( *this );
	return *this;
}


CSphRsetSchema & CSphRsetSchema::operator = ( const CSphSchema & rhs )
{
	Reset();
	m_pIndexSchema = &rhs;

	// copy over dynamic rowitems map
	// so that the new attributes we might add would not overlap
	m_dDynamicUsed = rhs.m_dDynamicUsed;

	// copy data ptr map. we might want to add proper access via virtual funcs later
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;

	return *this;
}


void CSphRsetSchema::RemoveStaticAttr ( int iAttr )
{
	assert ( m_pIndexSchema );
	assert ( iAttr>=0 );
	assert ( iAttr<ActualLen() );

	// map from rset indexes (adjusted for removal) to index schema indexes (the original ones)
	ARRAY_FOREACH_COND ( i, m_dRemoved, iAttr>=m_dRemoved[i] )
		iAttr++;
	m_dRemoved.Add ( iAttr );
	m_dRemoved.Uniq();
}


void CSphRsetSchema::SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs )
{
#ifndef NDEBUG
	// ensure that every incoming column has a matching original column
	// only check locators and attribute types, because at this stage,
	// names that are used in dAttrs are already overwritten by the aliases
	// (example: SELECT col1 a, col2 b, count(*) c FROM test)
	//
	// FIXME? maybe also lockdown the schema from further swaps, adds etc from here?

	for ( auto &dAttr : dAttrs )
	{
		auto fComparer = [&dAttr] ( const CSphColumnInfo &dInfo ) {
			return dInfo.m_tLocator==dAttr.m_tLocator
				&& dInfo.m_eAttrType==dAttr.m_eAttrType;
		};
		bool bFound1 = m_pIndexSchema ? m_pIndexSchema->m_dAttrs.FindFirst ( fComparer ) : false;
		bool bFound2 = m_dExtraAttrs.FindFirst ( fComparer );
		assert ( bFound1 || bFound2 );
	}
#endif
	m_dExtraAttrs.SwapData ( dAttrs );
	m_pIndexSchema = nullptr;
}


///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE OUTPUT
///////////////////////////////////////////////////////////////////////////////
void CSphWriter::SetBufferSize ( int iBufferSize )
{
	if ( iBufferSize!=m_iBufferSize )
	{
		m_iBufferSize = Max ( iBufferSize, 262144 );
		SafeDeleteArray ( m_pBuffer );
	}
}


bool CSphWriter::OpenFile ( const CSphString & sName, CSphString & sErrorBuffer )
{
	assert ( !sName.IsEmpty() );
	assert ( m_iFD<0 && "already open" );

	m_bOwnFile = true;
	m_sName = sName;
	m_pError = &sErrorBuffer;

	if ( !m_pBuffer )
		m_pBuffer = new BYTE [ m_iBufferSize ];

	m_iFD = ::open ( m_sName.cstr(), SPH_O_NEW, 0644 );
	m_pPool = m_pBuffer;
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_iDiskPos = 0;
	m_bError = ( m_iFD<0 );

	if ( m_bError )
		m_pError->SetSprintf ( "failed to create %s: %s" , sName.cstr(), strerror(errno) );

	return !m_bError;
}


void CSphWriter::SetFile ( CSphAutofile & tAuto, SphOffset_t * pSharedOffset, CSphString & sError )
{
	assert ( m_iFD<0 && "already open" );
	m_bOwnFile = false;

	if ( !m_pBuffer )
		m_pBuffer = new BYTE [ m_iBufferSize ];

	m_iFD = tAuto.GetFD();
	m_sName = tAuto.GetFilename();
	m_pPool = m_pBuffer;
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_iDiskPos = 0;
	m_pSharedOffset = pSharedOffset;
	m_pError = &sError;
	assert ( m_pError );
}


CSphWriter::~CSphWriter ()
{
	CloseFile ();
	SafeDeleteArray ( m_pBuffer );
}


void CSphWriter::CloseFile ( bool bTruncate )
{
	if ( m_iFD>=0 )
	{
		Flush ();
		if ( bTruncate )
			sphTruncate ( m_iFD );
		if ( m_bOwnFile )
			::close ( m_iFD );
		m_iFD = -1;
	}
}

void CSphWriter::UnlinkFile()
{
	if ( m_bOwnFile )
	{
		if ( m_iFD>=0 )
			::close ( m_iFD );

		m_iFD = -1;
		::unlink ( m_sName.cstr() );
		m_sName = "";
	}
	SafeDeleteArray ( m_pBuffer );
}


void CSphWriter::UpdatePoolUsed()
{
	if ( m_pPool-m_pBuffer > m_iPoolUsed )
		m_iPoolUsed = m_pPool-m_pBuffer;
}


void CSphWriter::PutByte ( int data )
{
	assert ( m_pPool );
	if ( m_iPoolUsed==m_iBufferSize )
		Flush ();
	*m_pPool++ = BYTE ( data & 0xff );
	UpdatePoolUsed();
	m_iPos++;
}


void CSphWriter::PutBytes ( const void * pData, int64_t iSize )
{
	assert ( m_pPool );
	const BYTE * pBuf = (const BYTE *) pData;
	while ( iSize>0 )
	{
		int iPut = ( iSize<m_iBufferSize ? int(iSize) : m_iBufferSize ); // comparison int64 to int32
		if ( m_iPoolUsed+iPut>m_iBufferSize )
			Flush ();
		assert ( m_iPoolUsed+iPut<=m_iBufferSize );

		memcpy ( m_pPool, pBuf, iPut );
		m_pPool += iPut;
		UpdatePoolUsed();
		m_iPos += iPut;

		pBuf += iPut;
		iSize -= iPut;
	}
}


void CSphWriter::ZipInt ( DWORD uValue )
{
	sphZipValue ( uValue, this, &CSphWriter::PutByte );
}


void CSphWriter::ZipOffset ( uint64_t uValue )
{
	sphZipValue ( uValue, this, &CSphWriter::PutByte );
}


void CSphWriter::Flush ()
{
	if ( m_pSharedOffset && *m_pSharedOffset!=m_iDiskPos )
	{
		auto uMoved = sphSeek ( m_iFD, m_iDiskPos, SEEK_SET );
		if ( uMoved!= m_iDiskPos )
		{
			m_bError = true;
			return;
		}
	}

	if ( !sphWriteThrottled ( m_iFD, m_pBuffer, m_iPoolUsed, m_sName.cstr(), *m_pError ) )
		m_bError = true;

	m_iDiskPos += m_iPoolUsed;
	m_iPoolUsed = 0;
	m_pPool = m_pBuffer;

	if ( m_pSharedOffset )
		*m_pSharedOffset = m_iDiskPos;
}


void CSphWriter::PutString ( const char * szString )
{
	int iLen = szString ? strlen ( szString ) : 0;
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void CSphWriter::PutString ( const CSphString & sString )
{
	int iLen = sString.Length();
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( sString.cstr(), iLen );
}


void CSphWriter::Tag ( const char * sTag )
{
	assert ( sTag && *sTag ); // empty tags are nonsense
	assert ( strlen(sTag)<64 ); // huge tags are nonsense
	PutBytes ( sTag, strlen(sTag) );
}


static bool SeekAndWarn ( int iFD, SphOffset_t iPos, const char* sWarnPrefix )
{
	assert ( sWarnPrefix );
	auto iSeek = sphSeek ( iFD, iPos, SEEK_SET );
	if ( iSeek!=iPos )
	{
		if ( iSeek<0 )
			sphWarning ( "%s : seek error. Error: %d '%s'", sWarnPrefix, errno, strerrorm (errno));
		else
			sphWarning ( "%s : seek error. Expected: " INT64_FMT ", got " INT64_FMT,
				sWarnPrefix, (int64_t) iPos, (int64_t) iSeek );
		return false;
	}

	assert ( iSeek==iPos );
	return true;
}


void CSphWriter::SeekTo ( SphOffset_t iPos, bool bTruncate )
{
	assert ( iPos>=0 );

	if ( iPos>=m_iDiskPos && iPos<=( m_iDiskPos + m_iPoolUsed ) )
	{
		// seeking inside the buffer
		// m_iPoolUsed should be always in sync with m_iPos
		// or it breaks seek back at cidxHit
		m_iPoolUsed = (int)( iPos - m_iDiskPos );
		m_pPool = m_pBuffer + m_iPoolUsed;
	} else
	{
		Flush ();
		SeekAndWarn ( m_iFD, iPos, "CSphWriter::SeekTo" );

		if ( bTruncate )
			sphTruncate(m_iFD);

		m_pPool = m_pBuffer;
		m_iPoolUsed = 0;
		m_iDiskPos = iPos;
	}
	m_iPos = iPos;
}


///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE INPUT
///////////////////////////////////////////////////////////////////////////////

CSphReader::CSphReader ( BYTE * pBuf, int iSize )
	: m_pBuff ( pBuf )
	, m_iBufSize ( iSize )
	, m_iReadUnhinted ( DEFAULT_READ_UNHINTED )
{
	assert ( pBuf==NULL || iSize>0 );
}


CSphReader::~CSphReader ()
{
	if ( m_bBufOwned )
		SafeDeleteArray ( m_pBuff );
}


void CSphReader::SetBuffers ( int iReadBuffer, int iReadUnhinted )
{
	if ( !m_pBuff )
		m_iBufSize = iReadBuffer;
	m_iReadUnhinted = iReadUnhinted;
}


void CSphReader::SetFile ( int iFD, const char * sFilename )
{
	m_iFD = iFD;
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
	m_sFilename = sFilename;
}


void CSphReader::SetFile ( const CSphAutofile & tFile )
{
	SetFile ( tFile.GetFD(), tFile.GetFilename() );
}


void CSphReader::Reset ()
{
	SetFile ( -1, "" );
}


/// sizehint > 0 means we expect to read approx that much bytes
/// sizehint == 0 means no hint, use default (happens later in UpdateCache())
/// sizehint == -1 means reposition and adjust current hint
void CSphReader::SeekTo ( SphOffset_t iPos, int iSizeHint )
{
	assert ( iPos>=0 );
	assert ( iSizeHint>=-1 );

#ifndef NDEBUG
#if PARANOID
	struct_stat tStat;
	fstat ( m_iFD, &tStat );
	if ( iPos > tStat.st_size )
		sphDie ( "INTERNAL ERROR: seeking past the end of file" );
#endif
#endif

	if ( iPos>=m_iPos && iPos<m_iPos+m_iBuffUsed )
	{
		m_iBuffPos = (int)( iPos-m_iPos ); // reposition to proper byte
		m_iSizeHint = iSizeHint - ( m_iBuffUsed - m_iBuffPos ); // we already have some bytes cached, so let's adjust size hint
		assert ( m_iBuffPos<m_iBuffUsed );
	} else
	{
		m_iPos = iPos;
		m_iBuffPos = 0; // for GetPos() to work properly, aaaargh
		m_iBuffUsed = 0;

		if ( iSizeHint==-1 )
		{
			// the adjustment bureau
			// we need to seek but still keep the current hint
			// happens on a skiplist jump, for instance
			int64_t iHintLeft = m_iPos + m_iSizeHint - iPos;
			if ( iHintLeft>0 && iHintLeft<INT_MAX )
				iSizeHint = (int)iHintLeft;
			else
				iSizeHint = 0;
		}

		// get that hint
		assert ( iSizeHint>=0 );
		m_iSizeHint = iSizeHint;
	}
}


void CSphReader::SkipBytes ( int iCount )
{
	// 0 means "no hint", so this clamp works alright
	SeekTo ( m_iPos+m_iBuffPos+iCount, Max ( m_iSizeHint-m_iBuffPos-iCount, 0 ) );
}


#if USE_WINDOWS

// atomic seek+read for Windows
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( iBytes==0 )
		return 0;

	CSphIOStats * pIOStats = GetIOStats();
	int64_t tmStart = 0;
	if ( pIOStats )
		tmStart = sphMicroTimer();

	HANDLE hFile;
	hFile = (HANDLE) _get_osfhandle ( iFD );
	if ( hFile==INVALID_HANDLE_VALUE )
		return -1;

	STATIC_SIZE_ASSERT ( SphOffset_t, 8 );
	OVERLAPPED tOverlapped = { 0 };
	tOverlapped.Offset = (DWORD)( iOffset & I64C(0xffffffff) );
	tOverlapped.OffsetHigh = (DWORD)( iOffset>>32 );

	DWORD uRes;
	if ( !ReadFile ( hFile, pBuf, iBytes, &uRes, &tOverlapped ) )
	{
		DWORD uErr = GetLastError();
		if ( uErr==ERROR_HANDLE_EOF )
			return 0;

		errno = uErr; // FIXME! should remap from Win to POSIX
		return -1;
	}

	if ( pIOStats )
	{
		pIOStats->m_iReadTime += sphMicroTimer() - tmStart;
		pIOStats->m_iReadOps++;
		pIOStats->m_iReadBytes += iBytes;
	}

	return uRes;
}

#else
#if HAVE_PREAD

// atomic seek+read for non-Windows systems with pread() call
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	CSphIOStats * pIOStats = GetIOStats();
	if ( !pIOStats )
		return ::pread ( iFD, pBuf, iBytes, iOffset );

	int64_t tmStart = sphMicroTimer();
	int iRes = (int) ::pread ( iFD, pBuf, iBytes, iOffset );
	if ( pIOStats )
	{
		pIOStats->m_iReadTime += sphMicroTimer() - tmStart;
		pIOStats->m_iReadOps++;
		pIOStats->m_iReadBytes += iBytes;
	}
	return iRes;
}

#else

// generic fallback; prone to races between seek and read
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( sphSeek ( iFD, iOffset, SEEK_SET )==-1 )
		return -1;

	return sphReadThrottled ( iFD, pBuf, iBytes, &g_tThrottle );
}

#endif // HAVE_PREAD
#endif // USE_WINDOWS


void CSphReader::UpdateCache ()
{
	CSphScopedProfile tProf ( m_pProfile, m_eProfileState );

	assert ( m_iFD>=0 );

	// alloc buf on first actual read
	if ( !m_pBuff )
	{
		if ( m_iBufSize<=0 )
			m_iBufSize = DEFAULT_READ_BUFFER;

		m_bBufOwned = true;
		m_pBuff = new BYTE [ m_iBufSize ];
	}

	// stream position could be changed externally
	// so let's just hope that the OS optimizes redundant seeks
	SphOffset_t iNewPos = m_iPos + Min ( m_iBuffPos, m_iBuffUsed );

	if ( m_iSizeHint<=0 )
		m_iSizeHint = ( m_iReadUnhinted>0 ) ? m_iReadUnhinted : DEFAULT_READ_UNHINTED;
	int iReadLen = Min ( m_iSizeHint, m_iBufSize );

	m_iBuffPos = 0;
	m_iBuffUsed = sphPread ( m_iFD, m_pBuff, iReadLen, iNewPos ); // FIXME! what about throttling?

	if ( m_iBuffUsed<0 )
	{
		m_iBuffUsed = m_iBuffPos = 0;
		m_bError = true;
		m_sError.SetSprintf ( "pread error in %s: pos=" INT64_FMT ", len=%d, code=%d, msg=%s",
			m_sFilename.cstr(), (int64_t)iNewPos, iReadLen, errno, strerror(errno) );
		return;
	}

	// all fine, adjust offset and hint
	m_iSizeHint -= m_iBuffUsed;
	m_iPos = iNewPos;
}


int CSphReader::GetByte ()
{
	if ( m_iBuffPos>=m_iBuffUsed )
	{
		UpdateCache ();
		if ( m_iBuffPos>=m_iBuffUsed )
			return 0; // unexpected io failure
	}

	assert ( m_iBuffPos<m_iBuffUsed );
	return m_pBuff [ m_iBuffPos++ ];
}


void CSphReader::GetBytes ( void * pData, int iSize )
{
	BYTE * pOut = (BYTE*) pData;

	while ( iSize>m_iBufSize )
	{
		int iLen = m_iBuffUsed - m_iBuffPos;
		assert ( iLen<=m_iBufSize );

		memcpy ( pOut, m_pBuff+m_iBuffPos, iLen );
		m_iBuffPos += iLen;
		pOut += iLen;
		iSize -= iLen;
		m_iSizeHint = Max ( m_iReadUnhinted, iSize );

		if ( iSize>0 )
		{
			UpdateCache ();
			if ( !m_iBuffUsed )
			{
				memset ( pData, 0, iSize );
				return; // unexpected io failure
			}
		}
	}

	if ( m_iBuffPos+iSize>m_iBuffUsed )
	{
		// move old buffer tail to buffer head to avoid losing the data
		const int iLen = m_iBuffUsed - m_iBuffPos;
		if ( iLen>0 )
		{
			memcpy ( pOut, m_pBuff+m_iBuffPos, iLen );
			m_iBuffPos += iLen;
			pOut += iLen;
			iSize -= iLen;
		}

		m_iSizeHint = Max ( m_iReadUnhinted, iSize );
		UpdateCache ();
		if ( m_iBuffPos+iSize>m_iBuffUsed )
		{
			memset ( pData, 0, iSize ); // unexpected io failure
			return;
		}
	}

	assert ( (m_iBuffPos+iSize)<=m_iBuffUsed );
	memcpy ( pOut, m_pBuff+m_iBuffPos, iSize );
	m_iBuffPos += iSize;
}


int CSphReader::GetBytesZerocopy ( const BYTE ** ppData, int iMax )
{
	if ( m_iBuffPos>=m_iBuffUsed )
	{
		UpdateCache ();
		if ( m_iBuffPos>=m_iBuffUsed )
			return 0; // unexpected io failure
	}

	int iChunk = Min ( m_iBuffUsed-m_iBuffPos, iMax );
	*ppData = m_pBuff + m_iBuffPos;
	m_iBuffPos += iChunk;
	return iChunk;
}


int CSphReader::GetLine ( char * sBuffer, int iMaxLen )
{
	int iOutPos = 0;
	iMaxLen--; // reserve space for trailing '\0'

	// grab as many chars as we can
	while ( iOutPos<iMaxLen )
	{
		// read next chunk if necessary
		if ( m_iBuffPos>=m_iBuffUsed )
		{
			UpdateCache ();
			if ( m_iBuffPos>=m_iBuffUsed )
			{
				if ( iOutPos==0 ) return -1; // current line is empty; indicate eof
				break; // return current line; will return eof next time
			}
		}

		// break on CR or LF
		if ( m_pBuff[m_iBuffPos]=='\r' || m_pBuff[m_iBuffPos]=='\n' )
			break;

		// one more valid char
		sBuffer[iOutPos++] = m_pBuff[m_iBuffPos++];
	}

	// skip everything until the newline or eof
	while (true)
	{
		// read next chunk if necessary
		if ( m_iBuffPos>=m_iBuffUsed )
			UpdateCache ();

		// eof?
		if ( m_iBuffPos>=m_iBuffUsed )
			break;

		// newline?
		if ( m_pBuff[m_iBuffPos++]=='\n' )
			break;
	}

	// finalize
	sBuffer[iOutPos] = '\0';
	return iOutPos;
}

void CSphReader::ResetError()
{
	m_bError = false;
	m_sError = "";
}

/////////////////////////////////////////////////////////////////////////////

#if PARANOID

#define SPH_VARINT_DECODE(_type,_getexpr) \
	register DWORD b = 0; \
	register _type v = 0; \
	int it = 0; \
	do { b = _getexpr; v = ( v<<7 ) + ( b&0x7f ); it++; } while ( b&0x80 ); \
	assert ( (it-1)*7<=sizeof(_type)*8 ); \
	return v;

#else

#define SPH_VARINT_DECODE(_type,_getexpr) \
	register DWORD b = _getexpr; \
	register _type res = 0; \
	while ( b & 0x80 ) \
	{ \
		res = ( res<<7 ) + ( b & 0x7f ); \
		b = _getexpr; \
	} \
	res = ( res<<7 ) + b; \
	return res;

#endif // PARANOID

DWORD sphUnzipInt ( const BYTE * & pBuf )			{ SPH_VARINT_DECODE ( DWORD, *pBuf++ ); }
SphOffset_t sphUnzipOffset ( const BYTE * & pBuf )	{ SPH_VARINT_DECODE ( SphOffset_t, *pBuf++ ); }

DWORD CSphReader::UnzipInt ()			{ SPH_VARINT_DECODE ( DWORD, GetByte() ); }
uint64_t CSphReader::UnzipOffset ()		{ SPH_VARINT_DECODE ( uint64_t, GetByte() ); }

DWORD ThinMMapReader_c::UnzipInt ()		{ SPH_VARINT_DECODE ( DWORD, *m_pPointer++ ); }
uint64_t ThinMMapReader_c::UnzipOffset ()	{ SPH_VARINT_DECODE ( uint64_t, *m_pPointer++ ); }

#define sphUnzipWordid sphUnzipOffset

/////////////////////////////////////////////////////////////////////////////

CSphReader & CSphReader::operator = ( const CSphReader & rhs )
{
	SetFile ( rhs.m_iFD, rhs.m_sFilename.cstr() );
	SeekTo ( rhs.m_iPos + rhs.m_iBuffPos, rhs.m_iSizeHint );
	return *this;
}


DWORD CSphReader::GetDword ()
{
	DWORD uRes = 0;
	GetBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


SphOffset_t CSphReader::GetOffset ()
{
	SphOffset_t uRes = 0;
	GetBytes ( &uRes, sizeof(SphOffset_t) );
	return uRes;
}


CSphString CSphReader::GetString ()
{
	CSphString sRes;

	DWORD iLen = GetDword ();
	if ( iLen )
	{
		char * sBuf = new char [ iLen ];
		GetBytes ( sBuf, iLen );
		sRes.SetBinary ( sBuf, iLen );
		SafeDeleteArray ( sBuf );
	}

	return sRes;
}

bool CSphReader::Tag ( const char * sTag )
{
	if ( m_bError )
		return false;

	assert ( sTag && *sTag ); // empty tags are nonsense
	assert ( strlen(sTag)<64 ); // huge tags are nonsense

	int iLen = strlen(sTag);
	char sBuf[64];
	GetBytes ( sBuf, iLen );
	if ( !memcmp ( sBuf, sTag, iLen ) )
		return true;
	m_bError = true;
	m_sError.SetSprintf ( "expected tag %s was not found", sTag );
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CSphAutoreader::Open ( const CSphString & sFilename, CSphString & sError )
{
	assert ( m_iFD<0 );
	assert ( !sFilename.IsEmpty() );

	m_iFD = ::open ( sFilename.cstr(), SPH_O_READ, 0644 );
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
	m_sFilename = sFilename;

	if ( m_iFD<0 )
		sError.SetSprintf ( "failed to open %s: %s", sFilename.cstr(), strerror(errno) );
	return ( m_iFD>=0 );
}


void CSphAutoreader::Close ()
{
	if ( m_iFD>=0 )
		::close ( m_iFD	);
	m_iFD = -1;
}


SphOffset_t FileReader_c::GetFilesize () const
{
	assert ( m_iFD>=0 );

	return sphGetFileSize ( m_iFD, nullptr );
}

/////////////////////////////////////////////////////////////////////////////
// QUERY RESULT
/////////////////////////////////////////////////////////////////////////////
CSphQueryResult::~CSphQueryResult ()
{
	ARRAY_FOREACH ( i, m_dMatches )
		m_tSchema.FreeDataPtrs ( &m_dMatches[i] );
}

/////////////////////////////////////////////////////////////////////////////
// CHUNK READER
/////////////////////////////////////////////////////////////////////////////

CSphBin::CSphBin ( ESphHitless eMode, bool bWordDict )
	: m_eMode ( eMode )
	, m_bWordDict ( bWordDict )
{
	m_tHit.m_sKeyword = bWordDict ? m_sKeyword : nullptr;
	m_sKeyword[0] = '\0';

#ifndef NDEBUG
	m_sLastKeyword[0] = '\0';
#endif
}


int CSphBin::CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn )
{
	if ( iBlocks<=0 )
		return CSphBin::MIN_SIZE;

	int iBinSize = ( ( iMemoryLimit/iBlocks + 2048 ) >> 12 ) << 12; // round to 4k

	if ( iBinSize<CSphBin::MIN_SIZE )
	{
		iBinSize = CSphBin::MIN_SIZE;
		sphWarn ( "%s: mem_limit=%d kb extremely low, increasing to %d kb",
			sPhase, iMemoryLimit/1024, iBinSize*iBlocks/1024 );
	}

	if ( iBinSize<CSphBin::WARN_SIZE && bWarn )
	{
		sphWarn ( "%s: merge_block_size=%d kb too low, increasing mem_limit may improve performance",
			sPhase, iBinSize/1024 );
	}

	return iBinSize;
}


void CSphBin::Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize )
{
	assert ( !m_dBuffer );
	assert ( iBinSize>=MIN_SIZE );
	assert ( pSharedOffset );

	m_iFile = iFD;
	m_pFilePos = pSharedOffset;

	m_iSize = iBinSize;
	m_dBuffer = new BYTE [ iBinSize ];
	m_pCurrent = m_dBuffer;

	m_tHit.m_tRowID = INVALID_ROWID;
	m_tHit.m_uWordID = 0;
	m_tHit.m_iWordPos = EMPTY_HIT;
	m_tHit.m_dFieldMask.UnsetAll();

	m_bError = false;
}


CSphBin::~CSphBin ()
{
	SafeDeleteArray ( m_dBuffer );
}


int CSphBin::ReadByte ()
{
	BYTE r;

	if ( !m_iLeft )
	{
		if ( *m_pFilePos!=m_iFilePos )
		{
			if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::ReadBytes" ) )
			{
				m_bError = true;
				return BIN_READ_ERROR;
			}
			*m_pFilePos = m_iFilePos;
		}

		int n = m_iFileLeft > m_iSize
			? m_iSize
			: (int)m_iFileLeft;
		if ( n==0 )
		{
			m_iDone = 1;
			m_iLeft = 1;
		} else
		{
			assert ( m_dBuffer );

			if ( sphReadThrottled ( m_iFile, m_dBuffer, n )!=(size_t)n )
			{
				m_bError = true;
				return -2;
			}
			m_iLeft = n;

			m_iFilePos += n;
			m_iFileLeft -= n;
			m_pCurrent = m_dBuffer;
			*m_pFilePos += n;
		}
	}
	if ( m_iDone )
	{
		m_bError = true; // unexpected (!) eof
		return -1;
	}

	m_iLeft--;
	r = *(m_pCurrent);
	m_pCurrent++;
	return r;
}


ESphBinRead CSphBin::ReadBytes ( void * pDest, int iBytes )
{
	assert ( iBytes>0 );
	assert ( iBytes<=m_iSize );

	if ( m_iDone )
		return BIN_READ_EOF;

	if ( m_iLeft<iBytes )
	{
		if ( *m_pFilePos!=m_iFilePos )
		{
			if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::ReadBytes" ))
			{
				m_bError = true;
				return BIN_READ_ERROR;
			}
			*m_pFilePos = m_iFilePos;
		}

		int n = Min ( m_iFileLeft, m_iSize - m_iLeft );
		if ( n==0 )
		{
			m_iDone = 1;
			m_bError = true; // unexpected (!) eof
			return BIN_READ_EOF;
		}

		assert ( m_dBuffer );
		memmove ( m_dBuffer, m_pCurrent, m_iLeft );

		if ( sphReadThrottled ( m_iFile, m_dBuffer + m_iLeft, n )!=(size_t)n )
		{
			m_bError = true;
			return BIN_READ_ERROR;
		}

		m_iLeft += n;
		m_iFilePos += n;
		m_iFileLeft -= n;
		m_pCurrent = m_dBuffer;
		*m_pFilePos += n;
	}

	assert ( m_iLeft>=iBytes );
	m_iLeft -= iBytes;

	memcpy ( pDest, m_pCurrent, iBytes );
	m_pCurrent += iBytes;

	return BIN_READ_OK;
}


SphWordID_t CSphBin::ReadVLB ()
{
	SphWordID_t uValue = 0;
	int iByte, iOffset = 0;
	do
	{
		if ( ( iByte = ReadByte() )<0 )
			return 0;
		uValue += ( ( SphWordID_t ( iByte & 0x7f ) ) << iOffset );
		iOffset += 7;
	}
	while ( iByte & 0x80 );
	return uValue;
}

DWORD CSphBin::UnzipInt ()
{
	register int b = 0;
	register DWORD v = 0;
	do
	{
		b = ReadByte();
		if ( b<0 )
			b = 0;
		v = ( v<<7 ) + ( b & 0x7f );
	} while ( b & 0x80 );
	return v;
}

SphOffset_t CSphBin::UnzipOffset ()
{
	register int b = 0;
	register SphOffset_t v = 0;
	do
	{
		b = ReadByte();
		if ( b<0 )
			b = 0;
		v = ( v<<7 ) + ( b & 0x7f );
	} while ( b & 0x80 );
	return v;
}

int CSphBin::ReadHit ( CSphAggregateHit * pOut )
{
	// expected EOB
	if ( m_iDone )
	{
		pOut->m_uWordID = 0;
		return 1;
	}

	CSphAggregateHit & tHit = m_tHit; // shortcut
	while (true)
	{
		// SPH_MAX_WORD_LEN is now 42 only to keep ReadVLB() below
		// technically, we can just use different functions on different paths, if ever needed
		STATIC_ASSERT ( SPH_MAX_WORD_LEN*3<=127, KEYWORD_TOO_LONG );
		SphWordID_t uDelta = ReadVLB();

		if ( uDelta )
		{
			switch ( m_eState )
			{
				case BIN_WORD:
					if ( m_bWordDict )
					{
#ifdef NDEBUG
						// FIXME?! move this under PARANOID or something?
						// or just introduce an assert() checked release build?
						if ( uDelta>=sizeof(m_sKeyword) )
							sphDie ( "INTERNAL ERROR: corrupted keyword length (len=" UINT64_FMT ", deltapos=" UINT64_FMT ")",
								(uint64_t)uDelta, (uint64_t)(m_iFilePos-m_iLeft) );
#else
						assert ( uDelta>0 && uDelta<sizeof(m_sKeyword)-1 );
#endif

						ReadBytes ( m_sKeyword, (int)uDelta );
						m_sKeyword[uDelta] = '\0';
						tHit.m_uWordID = sphCRC32 ( m_sKeyword ); // must be in sync with dict!

#ifndef NDEBUG
						assert ( ( m_iLastWordID<tHit.m_uWordID )
							|| ( m_iLastWordID==tHit.m_uWordID && strcmp ( (char*)m_sLastKeyword, (char*)m_sKeyword )<0 ) );
						strncpy ( (char*)m_sLastKeyword, (char*)m_sKeyword, sizeof(m_sLastKeyword) );
#endif

					} else
						tHit.m_uWordID += uDelta;

					tHit.m_tRowID = INVALID_ROWID;
					tHit.m_iWordPos = EMPTY_HIT;
					tHit.m_dFieldMask.UnsetAll();
					m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					m_eState = BIN_POS;
					tHit.m_tRowID += uDelta;
					tHit.m_iWordPos = EMPTY_HIT;
					break;

				case BIN_POS:
					if ( m_eMode==SPH_HITLESS_ALL )
					{
						tHit.m_dFieldMask.Assign32 ( (DWORD)ReadVLB() );
						m_eState = BIN_DOC;

					} else if ( m_eMode==SPH_HITLESS_SOME )
					{
						if ( uDelta & 1 )
						{
							tHit.m_dFieldMask.Assign32 ( (DWORD)ReadVLB() );
							m_eState = BIN_DOC;
						}
						uDelta >>= 1;
					}
					tHit.m_iWordPos += (DWORD)uDelta;
					*pOut = tHit;
					return 1;

				default:
					sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		} else
		{
			switch ( m_eState )
			{
				case BIN_POS:	m_eState = BIN_DOC; break;
				case BIN_DOC:	m_eState = BIN_WORD; break;
				case BIN_WORD:	m_iDone = 1; pOut->m_uWordID = 0; return 1;
				default:		sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		}
	}
}


bool CSphBin::IsEOF () const
{
	return m_iDone!=0 || m_iFileLeft<=0;
}


bool CSphBin::IsDone () const
{
	return m_iDone!=0 || ( m_iFileLeft<=0 && m_iLeft<=0 );
}


ESphBinRead CSphBin::Precache ()
{
	if ( m_iFileLeft > m_iSize-m_iLeft )
	{
		m_bError = true;
		return BIN_PRECACHE_ERROR;
	}

	if ( !m_iFileLeft )
		return BIN_PRECACHE_OK;

	if ( *m_pFilePos!=m_iFilePos )
	{
		if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::Precache" ))
		{
			m_bError = true;
			return BIN_PRECACHE_ERROR;
		}

		*m_pFilePos = m_iFilePos;
	}

	assert ( m_dBuffer );
	memmove ( m_dBuffer, m_pCurrent, m_iLeft );

	if ( sphReadThrottled ( m_iFile, m_dBuffer+m_iLeft, m_iFileLeft )!=(size_t)m_iFileLeft )
	{
		m_bError = true;
		return BIN_READ_ERROR;
	}

	m_iLeft += m_iFileLeft;
	m_iFilePos += m_iFileLeft;
	m_iFileLeft -= m_iFileLeft;
	m_pCurrent = m_dBuffer;
	*m_pFilePos += m_iFileLeft;

	return BIN_PRECACHE_OK;
}


CSphMultiQueryArgs::CSphMultiQueryArgs ( int iIndexWeight )
	: m_iIndexWeight ( iIndexWeight )
{
	assert ( iIndexWeight>0 );
}


/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphAtomic CSphIndex::m_tIdGenerator;

CSphIndex::CSphIndex ( const char * sIndexName, const char * sFilename )
	: m_tSchema ( sFilename )
	, m_sIndexName ( sIndexName )
	, m_sFilename ( sFilename )
{
	m_iIndexId = m_tIdGenerator.Inc();
}


CSphIndex::~CSphIndex ()
{
	QcacheDeleteIndex ( m_iIndexId );
}


void CSphIndex::SetInplaceSettings ( int iHitGap, float fRelocFactor, float fWriteFactor )
{
	m_iHitGap = iHitGap;
	m_fRelocFactor = fRelocFactor;
	m_fWriteFactor = fWriteFactor;
	m_bInplaceSettings = true;
}


void CSphIndex::SetFieldFilter ( ISphFieldFilter * pFieldFilter )
{
	SafeAddRef ( pFieldFilter );
	m_pFieldFilter = pFieldFilter;
}


void CSphIndex::SetTokenizer ( ISphTokenizer * pTokenizer )
{
	SafeAddRef ( pTokenizer );
	m_pTokenizer = pTokenizer;
}


void CSphIndex::SetupQueryTokenizer()
{
	// create and setup a master copy of query time tokenizer
	// that we can then use to create lightweight clones
	m_pQueryTokenizer = m_pTokenizer->Clone ( SPH_CLONE_QUERY );
	sphSetupQueryTokenizer ( m_pQueryTokenizer, IsStarDict(), m_tSettings.m_bIndexExactWords, false );

	m_pQueryTokenizerJson = m_pTokenizer->Clone ( SPH_CLONE_QUERY );
	sphSetupQueryTokenizer ( m_pQueryTokenizerJson, IsStarDict(), m_tSettings.m_bIndexExactWords, true );
}


ISphTokenizer *	CSphIndex::LeakTokenizer ()
{
	return m_pTokenizer.Leak();
}


void CSphIndex::SetDictionary ( CSphDict * pDict )
{
	SafeAddRef ( pDict );
	m_pDict = pDict;
}


CSphDict * CSphIndex::LeakDictionary ()
{
	return m_pDict.Leak();
}


void CSphIndex::Setup ( const CSphIndexSettings & tSettings )
{
	m_bStripperInited = true;
	m_tSettings = tSettings;
}

void CSphIndex::PostSetup ()
{
	// in case infixes got enabled and no prefix set let's enable prefix of any length 
	if ( m_pDict && m_pDict->GetSettings().m_bWordDict
		&& m_tSettings.m_iMinPrefixLen==0 && m_tSettings.m_iMinInfixLen>0 )
		m_tSettings.m_iMinPrefixLen = 1;
}

void CSphIndex::SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits )
{
	m_iMaxCachedDocs = iMaxCachedDocs;
	m_iMaxCachedHits = iMaxCachedHits;
}


float CSphIndex::GetGlobalIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF ) const
{
	g_tGlobalIDFLock.Lock ();
	CSphGlobalIDF ** ppGlobalIDF = g_hGlobalIDFs ( m_sGlobalIDFPath );
	float fIDF = ppGlobalIDF && *ppGlobalIDF ? ( *ppGlobalIDF )->GetIDF ( sWord, iDocsLocal, bPlainIDF ) : 0.0f;
	g_tGlobalIDFLock.Unlock ();
	return fIDF;
}


bool CSphIndex::BuildDocList ( SphAttr_t ** ppDocList, int64_t * pCount, CSphString * ) const
{
	assert ( *ppDocList && pCount );
	*ppDocList = nullptr;
	*pCount = 0;
	return true;
}

void CSphIndex::GetFieldFilterSettings ( CSphFieldFilterSettings & tSettings )
{
	if ( m_pFieldFilter )
		m_pFieldFilter->GetSettings ( tSettings );
}

//////////////////////////////////////////////////////////////////////////

MatchesToNewSchema_c::MatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
	: m_pOldSchema ( pOldSchema )
	, m_pNewSchema ( pNewSchema )
{
	assert ( pOldSchema && pNewSchema );

	for ( int i=0; i < m_pNewSchema->GetAttrsCount(); i++ )
		if ( !m_pOldSchema->GetAttr ( m_pNewSchema->GetAttr(i).m_sName.cstr() ) )
			m_dNewAttrs.Add ( m_pNewSchema->GetAttr(i).m_tLocator );

	m_dOld2New.Resize ( m_pOldSchema->GetAttrsCount() );
	for ( int i=0; i < m_pOldSchema->GetAttrsCount(); i++ )
		m_dOld2New[i] = m_pNewSchema->GetAttrIndex ( m_pOldSchema->GetAttr(i).m_sName.cstr() );

	// need to update @int_str2ptr_ locator to use new schema
	// no need to pass pOldSchema as we remapping only new schema pointers
	// also need to update group sorter keypart to be str_ptr in caller code SetSchema
	sphSortGetStringRemap ( *pNewSchema, *pNewSchema, m_dRemapStringCmp );
}


void MatchesToNewSchema_c::Process ( CSphMatch * pMatch )
{
	CSphMatch tResult;
	tResult.Reset ( m_pNewSchema->GetDynamicSize() );

	const BYTE * pBlobPool = GetBlobPool ( pMatch );

	for ( int i=0; i < m_pOldSchema->GetAttrsCount(); i++ )
	{
		if ( m_dOld2New[i]==-1 )
			continue;

		const CSphColumnInfo & tOld = m_pOldSchema->GetAttr(i);
		const CSphColumnInfo & tNew = m_pNewSchema->GetAttr(m_dOld2New[i]);

		if ( tOld.m_eAttrType==tNew.m_eAttrType )
		{
			tResult.SetAttr ( tNew.m_tLocator, pMatch->GetAttr ( tOld.m_tLocator ) );
			continue;
		}

		assert ( !sphIsDataPtrAttr(tOld.m_eAttrType) && sphIsDataPtrAttr(tNew.m_eAttrType) );

		switch ( tOld.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:
			{
				int iLengthBytes = 0;
				const BYTE * pData = sphGetBlobAttr ( *pMatch, tOld.m_tLocator, pBlobPool, iLengthBytes );
				tResult.SetAttr ( tNew.m_tLocator, (SphAttr_t)sphPackPtrAttr ( pData, iLengthBytes ) );
			}
			break;

		case SPH_ATTR_JSON_FIELD:
			{
				SphAttr_t uPacked = pMatch->GetAttr ( tOld.m_tLocator );

				const BYTE * pStr = uPacked ? pBlobPool + sphJsonUnpackOffset ( uPacked ) : nullptr;
				ESphJsonType eJson = sphJsonUnpackType ( uPacked );

				if ( pStr && eJson!=JSON_NULL )
				{
					int iLengthBytes = sphJsonNodeSize ( eJson, pStr );

					BYTE * pData = nullptr;
					BYTE * pPacked = sphPackPtrAttr ( iLengthBytes+1, &pData );

					// store field type before the field
					*pData = (BYTE)eJson;
					memcpy ( pData+1, pStr, iLengthBytes );

					tResult.SetAttr ( tNew.m_tLocator, (SphAttr_t)pPacked );
				} else
					tResult.SetAttr ( tNew.m_tLocator, 0 );
			}
			break;

		default:
			assert ( 0 && "Unexpected attribute type" );
			break;
		}
	}

	for ( const auto & i : m_dNewAttrs )
		tResult.SetAttr ( i, 0 );

	for ( const SphStringSorterRemap_t & tRemap : m_dRemapStringCmp )
		tResult.SetAttr ( tRemap.m_tDst, tResult.GetAttr ( tRemap.m_tSrc ) );

	Swap ( pMatch->m_pDynamic, tResult.m_pDynamic );
	pMatch->m_pStatic = nullptr;
}


class DiskMatchesToNewSchema_c : public MatchesToNewSchema_c
{
public:
	DiskMatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, const BYTE * pBlobPool )
		: MatchesToNewSchema_c ( pOldSchema, pNewSchema )
		, m_pBlobPool ( pBlobPool )
	{}

private:
	const BYTE *	m_pBlobPool {nullptr};

	const BYTE * GetBlobPool ( const CSphMatch * /*pMatch*/ ) final
	{
		return m_pBlobPool;
	}
};

//////////////////////////////////////////////////////////////////////////
ISphSchema * sphCreateStandaloneSchema ( const ISphSchema * pSchema )
{
	assert ( pSchema );

	CSphSchema * pResult = new CSphSchema ( "standalone" );
	for ( int i = 0; i < pSchema->GetFieldsCount(); i++ )
		pResult->AddField ( pSchema->GetField(i) );

	for ( int i = 0; i < pSchema->GetAttrsCount(); i++ )
	{
		CSphColumnInfo tAttr = pSchema->GetAttr(i);
		tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );
		tAttr.m_tLocator.m_iBitOffset = -1;
		tAttr.m_tLocator.m_iBitCount = -1;
		tAttr.m_tLocator.m_iBlobAttrId = -1;
		tAttr.m_tLocator.m_nBlobAttrs = 0;
		pResult->AddAttr ( tAttr, true );
	}

	for ( int i = 0; i < pResult->GetAttrsCount(); i++ )
	{
		auto & pExpr = pResult->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->FixupLocator ( pSchema, pResult );
	}

	return pResult;
}

//////////////////////////////////////////////////////////////////////////

static void PooledAttrsToPtrAttrs ( ISphMatchSorter ** ppSorters, int nSorters, const BYTE * pBlobPool )
{
	assert ( ppSorters );

	for ( int i=0; i<nSorters; ++i )
	{
		ISphMatchSorter * pSorter = ppSorters[i];
		if ( !pSorter )
			continue;

		const ISphSchema * pOldSchema = pSorter->GetSchema();
		ISphSchema * pNewSchema =  sphCreateStandaloneSchema ( pOldSchema );
		assert ( pOldSchema && pNewSchema );

		DiskMatchesToNewSchema_c fnFinal ( pOldSchema, pNewSchema, pBlobPool );
		pSorter->Finalize ( fnFinal, false );

		pSorter->SetSchema ( pNewSchema, true );
		SafeDelete ( pOldSchema );
	}
}


CSphIndex * sphCreateIndexPhrase ( const char* szIndexName, const char * sFilename )
{
	return new CSphIndex_VLN ( szIndexName, sFilename );
}

//////////////////////////////////////////////////////////////////////////


CSphIndex_VLN::CSphIndex_VLN ( const char* sIndexName, const char * sFilename )
	: CSphIndex ( sIndexName, sFilename )
	, m_iLockFD ( -1 )
	, m_dFieldLens ( SPH_MAX_FIELDS )
{
	m_sFilename = sFilename;

	m_iDocinfo = 0;
	m_iDocinfoIndex = 0;
	m_pDocinfoIndex = NULL;

	m_uVersion = INDEX_FORMAT_VERSION;
	m_bPassedRead = false;
	m_bPassedAlloc = false;
	m_bIsEmpty = true;
	m_bDebugCheck = false;
	m_uAttrsStatus = 0;

	m_iMinMaxIndex = 0;

	ARRAY_FOREACH ( i, m_dFieldLens )
		m_dFieldLens[i] = 0;
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	SafeDelete ( m_pHistograms );
	Unlock();
}

struct DocidIndex_t
{
	DocID_t	m_tDocID;
	int		m_iIndex;
};

template <typename READER1, typename READER2, typename FUNCTOR>
void Intersect ( READER1 & tReader1, READER2 & tReader2, FUNCTOR & tFunctor )
{
	RowID_t tRowID1 = INVALID_ROWID;
	DocID_t tDocID1 = 0, tDocID2 = 0;
	bool bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
	bool bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
	
	while ( bHaveDocs1 && bHaveDocs2 )
	{
		if ( tDocID1 < tDocID2 )
		{
			tReader1.HintDocID ( tDocID2 );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
		}
		else if (  tDocID1 > tDocID2 )
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		else
		{
			tFunctor.Process ( tRowID1, tReader2 );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		}
	}
}


class DocIdIndexReader_c
{
public:
	DocIdIndexReader_c ( const DocidIndex_t * pList, int iListSize )
		: m_pIterator ( pList )
		, m_pMaxIterator ( pList+iListSize )
	{}

	inline bool ReadDocID ( DocID_t & tDocID )
	{
		if ( m_pIterator>=m_pMaxIterator )
			return false;

		tDocID = m_pIterator->m_tDocID;
		m_pIterator++;
		return true;
	}

	int GetIndex() const
	{
		return (m_pIterator-1)->m_iIndex;
	}

private:
	const DocidIndex_t * m_pIterator {nullptr};
	const DocidIndex_t * m_pMaxIterator {nullptr};
};


void CSphIndex_VLN::Update_CollectRowPtrs ( UpdateContext_t & tCtx )
{
	struct RowFinder_c
	{
		UpdateContext_t &	m_tCtx;
		CSphIndex_VLN *		m_pIndex {nullptr};

		RowFinder_c ( CSphIndex_VLN * pIndex, UpdateContext_t & tCtx )
			: m_tCtx ( tCtx )
			, m_pIndex ( pIndex )
		{
			assert ( m_pIndex );
		}

		void Process ( RowID_t tRowID, const DocIdIndexReader_c & tReader )
		{
			m_tCtx.GetRowData(tReader.GetIndex()).m_pRow = const_cast<CSphRowitem*> ( m_pIndex->GetDocinfoByRowID(tRowID) );
		}
	};

	CSphVector<DocidIndex_t> tSorted (tCtx.m_iLast-tCtx.m_iFirst);
	for ( int i = tCtx.m_iFirst; i<tCtx.m_iLast; i++ )
	{
		auto & tPair = tSorted[i-tCtx.m_iFirst];
		tPair.m_tDocID = tCtx.m_tUpd.m_dDocids[i];
		tPair.m_iIndex = i;
	}

	tSorted.Sort ( bind ( &DocidIndex_t::m_tDocID ) );

	LookupReader_c tLookupReader ( m_tDocidLookup.GetWritePtr() );
	DocIdIndexReader_c tSortedReader ( tSorted.Begin(), tSorted.GetLength() );
	RowFinder_c tFunctor ( this, tCtx );
	Intersect ( tLookupReader, tSortedReader, tFunctor );

	for ( int i = tCtx.m_iFirst; i<tCtx.m_iLast; i++ )
	{
		UpdatedRowData_t & tRow = tCtx.GetRowData(i);
		tRow.m_pAttrPool = m_tAttr.GetWritePtr();
		tRow.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
		tRow.m_pSegment = this;
	}
}


bool CSphIndex_VLN::Update_WriteBlobRow ( UpdateContext_t & tCtx, int /*iUpd*/, CSphRowitem * pDocinfo, const BYTE * pBlob, int iLength, int nBlobAttrs, bool & bCritical, CSphString & sError )
{
	BYTE * pExistingBlob = m_tBlobAttrs.GetWritePtr() + sphGetBlobRowOffset(pDocinfo);
	DWORD uExistingBlobLen = sphGetBlobTotalLen ( pExistingBlob, nBlobAttrs );

	bCritical = false;

	// overwrite old record (because we have the write lock)
	if ( (DWORD)iLength<=uExistingBlobLen )
	{
		memcpy ( pExistingBlob, pBlob, iLength );
		return true;
	}

	BYTE * pOldBlobPool = m_tBlobAttrs.GetWritePtr();
	SphOffset_t tBlobSpaceUsed = *(SphOffset_t*)pOldBlobPool;
	SphOffset_t tSpaceLeft = m_tBlobAttrs.GetLengthBytes()-tBlobSpaceUsed;

	// not great: we have to resize our .spb file and create new memory maps
	if ( (SphOffset_t)iLength > tSpaceLeft )
	{
		SphOffset_t tSizeDelta = Max ( (SphOffset_t)iLength-tSpaceLeft, m_tSettings.m_tBlobUpdateSpace );
		CSphString sWarning;
		size_t tOldSize = m_tBlobAttrs.GetLengthBytes();
		if ( !m_tBlobAttrs.Resize ( tOldSize + tSizeDelta, sWarning, sError ) )
		{
			// try to map again, using old size
			if ( !m_tBlobAttrs.Resize ( tOldSize, sWarning, sError ) )
				bCritical = true;	// real bad, index unusable

			sError = "unable to resize .SPB file";
			return false;
		}

		// update blob pool ptrs since they could have changed after the resize
		for ( auto & i : tCtx.m_dUpdatedRows )
			if ( i.m_pBlobPool==pOldBlobPool )
				i.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	}

	BYTE * pEnd = m_tBlobAttrs.GetWritePtr() + tBlobSpaceUsed;
	memcpy ( pEnd, pBlob, iLength );

	sphSetBlobRowOffset ( pDocinfo, tBlobSpaceUsed );

	tBlobSpaceUsed += iLength;

	*(SphOffset_t*)m_tBlobAttrs.GetWritePtr() = tBlobSpaceUsed;

	return true;
}


void CSphIndex_VLN::Update_MinMax ( UpdateContext_t & tCtx )
{
	int iRowStride = tCtx.m_tSchema.GetRowSize();

	for ( int iUpd=tCtx.m_iFirst; iUpd < tCtx.m_iLast; iUpd++ )
	{
		const UpdatedRowData_t & tRow = tCtx.GetRowData(iUpd);
		if ( !tRow.m_pRow )
			continue; // no such id

		int64_t iBlock = int64_t ( tRow.m_pRow-tRow.m_pAttrPool ) / ( iRowStride*DOCINFO_INDEX_FREQ );
		DWORD * pBlockRanges = m_pDocinfoIndex + ( iBlock * iRowStride * 2 );
		DWORD * pIndexRanges = m_pDocinfoIndex + ( m_iDocinfoIndex * iRowStride * 2 );
		assert ( iBlock>=0 && iBlock<m_iDocinfoIndex );

		ARRAY_FOREACH ( iCol, tCtx.m_tUpd.m_dAttributes )
		{
			const UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[iCol];
			if ( !tUpdAttr.m_bExisting )
				continue;

			const CSphAttrLocator & tLoc = tUpdAttr.m_tLocator;
			SphAttr_t uValue = sphGetRowAttr ( tRow.m_pRow, tLoc );

			// update block and index ranges
			for ( int i=0; i<2; i++ )
			{
				DWORD * pBlock = i ? pBlockRanges : pIndexRanges;
				SphAttr_t uMin = sphGetRowAttr ( pBlock, tLoc );
				SphAttr_t uMax = sphGetRowAttr ( pBlock+iRowStride, tLoc );
				if ( tCtx.m_dUpdatedAttrs[iCol].m_eAttrType==SPH_ATTR_FLOAT ) // update float's indexes assumes float comparision
				{
					float fValue = sphDW2F ( (DWORD) uValue );
					float fMin = sphDW2F ( (DWORD) uMin );
					float fMax = sphDW2F ( (DWORD) uMax );
					if ( fValue<fMin )
						sphSetRowAttr ( pBlock, tLoc, sphF2DW ( fValue ) );
					if ( fValue>fMax )
						sphSetRowAttr ( pBlock+iRowStride, tLoc, sphF2DW ( fValue ) );
				} else // update usual integers
				{
					if ( uValue<uMin )
						sphSetRowAttr ( pBlock, tLoc, uValue );
					if ( uValue>uMax )
						sphSetRowAttr ( pBlock+iRowStride, tLoc, uValue );
				}
			}
		}
	}
}


int CSphIndex_VLN::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, bool & bCritical, CSphString & sError, CSphString & sWarning )
{
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRowOffset.GetLength() );
	DWORD uRows = tUpd.m_dDocids.GetLength();

	// check if we have to
	if ( !m_iDocinfo || !uRows )
		return 0;

	UpdateContext_t tCtx ( tUpd, m_tSchema, m_pHistograms, ( iIndex<0 ) ? 0 : iIndex, ( iIndex<0 ) ? uRows : iIndex+1 );
	Update_CollectRowPtrs ( tCtx );
	if ( !Update_FixupData ( tCtx, sError ) )
		return -1;

	// FIXME! FIXME! FIXME! overwriting just-freed blocks might hurt concurrent searchers;
	// should implement a simplistic MVCC-style delayed-free to avoid that

	// first pass, if needed
	if ( tUpd.m_bStrict )
		if ( !Update_InplaceJson ( tCtx, sError, true ) )
			return -1;

	// second pass
	tCtx.m_iJsonWarnings = 0;
	Update_InplaceJson ( tCtx, sError, false );

	if ( !Update_Blobs ( tCtx, bCritical, sError ) )
		return -1;

	Update_Plain ( tCtx );
	Update_MinMax ( tCtx );

	int iUpdated = 0;
	for ( const auto & i : tCtx.m_dUpdatedRows )
		if ( i.m_bUpdated )
			iUpdated++;

	if ( !Update_HandleJsonWarnings ( tCtx, iUpdated, sWarning, sError ) )
		return -1;

	if ( tCtx.m_uUpdateMask && m_bBinlog && g_pBinlog )
		g_pBinlog->BinlogUpdateAttributes ( &m_iTID, m_sIndexName.cstr(), tUpd );

	m_uAttrsStatus |= tCtx.m_uUpdateMask; // FIXME! add lock/atomic?

	return iUpdated;
}

// safely rename an index file
bool CSphIndex_VLN::JuggleFile ( ESphExt eExt, CSphString & sError, bool bNeedOrigin ) const
{
	CSphString sExt = GetIndexFileName ( eExt );
	CSphString sExtNew = GetIndexFileName ( eExt, true );
	CSphString sExtOld;
	sExtOld.SetSprintf ( "%s.old", sExt.cstr() );

	if ( sph::rename ( sExt.cstr(), sExtOld.cstr() ) )
	{
		if ( bNeedOrigin )
		{
			sError.SetSprintf ( "rename '%s' to '%s' failed: %s", sExt.cstr(), sExtOld.cstr(), strerror(errno) );
			return false;
		}
	}

	if ( sph::rename ( sExtNew.cstr(), sExt.cstr() ) )
	{
		if ( bNeedOrigin && !sph::rename ( sExtOld.cstr(), sExt.cstr() ) )
		{
			// rollback failed too!
			sError.SetSprintf ( "rollback rename to '%s' failed: %s; INDEX UNUSABLE; FIX FILE NAMES MANUALLY", sExt.cstr(), strerror(errno) );
		} else
		{
			// rollback went ok
			sError.SetSprintf ( "rename '%s' to '%s' failed: %s", sExtNew.cstr(), sExt.cstr(), strerror(errno) );
		}
		return false;
	}

	// all done
	::unlink ( sExtOld.cstr() );
	return true;
}

bool CSphIndex_VLN::SaveAttributes ( CSphString & sError ) const
{
	if ( !m_uAttrsStatus || !m_iDocinfo )
		return true;

	DWORD uAttrStatus = m_uAttrsStatus;

	sphLogDebugvv ( "index '%s' attrs (%d) saving...", m_sIndexName.cstr(), uAttrStatus );

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_UPDATED )
	{
		if ( !m_tAttr.Flush ( true, sError ) )
			return false;

		if ( m_pHistograms && !m_pHistograms->Save ( GetIndexFileName(SPH_EXT_SPHI), sError ) )
			return false;
	}

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_BLOB_UPDATED )
	{
		if ( !m_tBlobAttrs.Flush ( true, sError ) )
			return false;
	}

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED )
	{
		if ( !m_tDeadRowMap.Flush ( true, sError ) )
			return false;
	}

	if ( m_bBinlog && g_pBinlog )
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	if ( m_uAttrsStatus==uAttrStatus )
		const_cast<DWORD &>( m_uAttrsStatus ) = 0;

	sphLogDebugvv ( "index '%s' attrs (%d) saved", m_sIndexName.cstr(), m_uAttrsStatus );

	return true;
}

DWORD CSphIndex_VLN::GetAttributeStatus () const
{
	return m_uAttrsStatus;
}


//////////////////////////////////////////////////////////////////////////

struct CmpDocidLookup_fn
{
	static inline bool IsLess ( const DocidRowidPair_t & a, const DocidRowidPair_t & b )
	{
		if ( a.m_tDocID==b.m_tDocID )
			return a.m_tRowID < b.m_tRowID;

		return a.m_tDocID < b.m_tDocID;
	}
};


struct CmpQueuedLookup_fn
{
	static DocidRowidPair_t * m_pStorage;

	static inline bool IsLess ( const int a, const int b )
	{
		if ( m_pStorage[a].m_tDocID==m_pStorage[b].m_tDocID )
			return m_pStorage[a].m_tRowID < m_pStorage[b].m_tRowID;

		return m_pStorage[a].m_tDocID < m_pStorage[b].m_tDocID;
	}
};

DocidRowidPair_t * CmpQueuedLookup_fn::m_pStorage = nullptr;


bool CSphIndex_VLN::WriteLookupAndHistograms ( const CSphString & sSPA, const CSphString & sSPT, const CSphString & sSPHI, DWORD uTotalDocs, const AttrIndexBuilder_c & tMinMax, CSphString & sError ) const
{
	if ( !uTotalDocs )
		return true;

	CSphAutofile tSPA ( sSPA.cstr(), SPH_O_READ, sError );
	if ( tSPA.GetFD()==-1 )
		return false;

	CSphReader tSPAReader;
	tSPAReader.SetFile(tSPA);

	const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMax.GetCollected();
	int iStride = m_tSchema.GetRowSize();
	const CSphRowitem * pMinRow = dMinMaxRows.Begin()+dMinMaxRows.GetLength()-iStride*2;
	const CSphRowitem * pMaxRow = pMinRow+iStride;

	HistogramContainer_c tHistogramContainer;
	CSphVector<Histogram_i *> dHistograms;
	CSphVector<CSphColumnInfo> dPOD;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
		{
			Verify ( tHistogramContainer.Add ( pHistogram ) );
			dHistograms.Add ( pHistogram );
			dPOD.Add ( tAttr );
			pHistogram->Setup ( sphGetRowAttr ( pMinRow, tAttr.m_tLocator ), sphGetRowAttr ( pMaxRow, tAttr.m_tLocator ) );
		}
	}

	CSphVector<CSphRowitem> dRow ( iStride );
	CSphRowitem * pRow = dRow.Begin();

	CSphFixedVector<DocidRowidPair_t> dDocidLookup ( uTotalDocs );
	for ( RowID_t tRowID = 0; tRowID < uTotalDocs; tRowID++ )
	{
		tSPAReader.GetBytes ( pRow, iStride*sizeof(CSphRowitem) );
		if ( tSPAReader.GetErrorFlag() )
		{
			sError = tSPAReader.GetErrorMessage();
			return false;
		}

		ARRAY_FOREACH ( i, dHistograms )
			dHistograms[i]->Insert ( sphGetRowAttr ( pRow, dPOD[i].m_tLocator ) );

		dDocidLookup[tRowID].m_tDocID = sphGetDocID(pRow);
		dDocidLookup[tRowID].m_tRowID = tRowID;
	}

	dDocidLookup.Sort ( CmpDocidLookup_fn() );
	if ( !::WriteDocidLookup ( sSPT, dDocidLookup, sError ) )
		return false;

	if ( !tHistogramContainer.Save ( sSPHI, sError ) )
		return false;

	return true;
}


bool CSphIndex_VLN::Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const
{
	return pDocinfo-m_tAttr.GetWritePtr() >= m_iDocinfo*iStride;
}


bool CSphIndex_VLN::AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError )
{
	CSphSchema tNewSchema = m_tSchema;
	if ( !Alter_AddRemoveFromSchema ( tNewSchema, sAttrName, eAttrType, bAddAttr, sError ) )
		return false;

	int iOldStride = m_tSchema.GetRowSize();
	int iNewStride = tNewSchema.GetRowSize();

	int64_t iNewMinMaxIndex = m_iDocinfo * iNewStride;

	BuildHeader_t tBuildHeader ( m_tStats );
	tBuildHeader.m_iMinMaxIndex = iNewMinMaxIndex;

	*(DictHeader_t*)&tBuildHeader = *(DictHeader_t*)&m_tWordlist;

	CSphString sHeaderName = GetIndexFileName ( SPH_EXT_SPH, true );
	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &tNewSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();

	// save the header
	bool bBuildRes = IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, sError );

	if ( !bBuildRes )
		return false;

	// generate new .SPA, .SPB files
	WriterWithHash_c tSPAWriter ( "spa", &m_dHashes );
	WriterWithHash_c tSPBWriter ( "spb", &m_dHashes );
	tSPAWriter.SetBufferSize ( 524288 );
	tSPBWriter.SetBufferSize ( 524288 );

	WriteWrapper_Disk_c tSPAWriteWrapper(tSPAWriter);
	WriteWrapper_Disk_c tSPBWriteWrapper(tSPBWriter);

	CSphString sSPAfile = GetIndexFileName ( SPH_EXT_SPA, true );
	CSphString sSPBfile = GetIndexFileName ( SPH_EXT_SPB, true );
	CSphString sSPHIfile = GetIndexFileName ( SPH_EXT_SPHI, true );
	if ( !tSPAWriter.OpenFile ( sSPAfile, sError ) )
		return false;

	bool bHadBlobs = false;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		bHadBlobs |= sphIsBlobAttr ( m_tSchema.GetAttr(i).m_eAttrType );

	bool bHaveBlobs = false;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		bHaveBlobs |= sphIsBlobAttr ( tNewSchema.GetAttr(i).m_eAttrType );

	bool bBlob = sphIsBlobAttr ( eAttrType );
	bool bBlobsModified = bBlob && ( bAddAttr || bHaveBlobs==bHadBlobs );
	if ( bBlobsModified )
	{
		if ( !tSPBWriter.OpenFile ( sSPBfile, sError ) )
			return false;

		tSPBWriter.PutOffset(0);
	}

	const CSphRowitem * pDocinfo = m_tAttr.GetWritePtr();
	if ( !pDocinfo )
	{
		sError = "index must have at least one attribute";
		return false;
	}

	const CSphRowitem * pDocinfoMax = pDocinfo + (m_iDocinfo + (m_iDocinfoIndex+1)*2)*iOldStride;

	Alter_AddRemoveAttr ( m_tSchema, tNewSchema, pDocinfo, pDocinfoMax, m_tBlobAttrs.GetWritePtr(), tSPAWriteWrapper, tSPBWriteWrapper, bAddAttr, sAttrName );

	if ( m_pHistograms )
	{
		if ( bAddAttr )
		{
			Histogram_i * pNewHistogram = CreateHistogram ( sAttrName, eAttrType );
			if ( pNewHistogram )
			{
				pNewHistogram->Setup(0,0);
				for ( DWORD i = 0; i < m_iDocinfo; i++ )
					pNewHistogram->Insert(0);

				m_pHistograms->Add ( pNewHistogram );
			}
		}
		else
			m_pHistograms->Remove ( sAttrName );

		CSphString sError;
		if ( !m_pHistograms->Save ( sSPHIfile, sError ) )
			return false;
	}

	if ( tSPAWriter.IsError() )
	{
		sError.SetSprintf ( "error writing to %s", sSPAfile.cstr() );
		return false;
	}

	tSPAWriter.CloseFile();
	m_dHashes.SaveSHA();

	if ( !JuggleFile ( SPH_EXT_SPA, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPH, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPHI, sError ) )
		return false;

	m_tAttr.Reset();
	if ( !m_tAttr.Setup ( GetIndexFileName(SPH_EXT_SPA), sError, true ) )
		return false;

	if ( bBlob )
	{
		m_tBlobAttrs.Reset();

		if ( bAddAttr || bHaveBlobs==bHadBlobs )
		{
			if ( tSPBWriter.IsError() )
			{
				sError.SetSprintf ( "error writing to %s", sSPBfile.cstr() );
				return false;
			}

			SphOffset_t tPos = tSPBWriter.GetPos();
			// FIXME!!! made single function from this mess as order matters here
			tSPBWriter.Flush(); // store collected data as SeekTo might got rid of buffer collected so far
			tSPBWriter.SeekTo ( 0 );
			tSPBWriter.PutOffset ( tPos );
			tSPBWriter.SeekTo ( tPos + m_tSettings.m_tBlobUpdateSpace, true );
			tSPBWriter.CloseFile();

			if ( !JuggleFile ( SPH_EXT_SPB, sError, bHadBlobs ) )
				return false;

			if ( !m_tBlobAttrs.Setup ( GetIndexFileName(SPH_EXT_SPB), sError, true ) )
				return false;
		} else
			::unlink ( GetIndexFileName(SPH_EXT_SPB).cstr() );
	}

	m_tSchema = tNewSchema;
	m_iMinMaxIndex = iNewMinMaxIndex;
	m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

	PrereadMapping ( m_sIndexName.cstr(), "attributes", IsMlock ( m_tFiles.m_eAttr ), IsOndisk ( m_tFiles.m_eAttr ), m_tAttr );

	if ( bBlobsModified )
		PrereadMapping ( m_sIndexName.cstr(), "blob attributes", IsMlock ( m_tFiles.m_eBlob ), IsOndisk ( m_tFiles.m_eBlob ), m_tBlobAttrs );

	return true;
}


void CSphIndex_VLN::FlushDeadRowMap ( bool bWaitComplete ) const
{
	// FIXME! handle errors
	CSphString sError;
	m_tDeadRowMap.Flush ( bWaitComplete, sError );
}


bool CSphIndex_VLN::LoadKillList ( CSphFixedVector<DocID_t> *pKillList, CSphVector<KillListTarget_t> & dTargets, CSphString & sError )
{
	CSphString sSPK = GetIndexFileName(SPH_EXT_SPK);
	if ( !sphIsReadable ( sSPK.cstr() ) )
		return true;

	CSphAutoreader tReader;
	if ( !tReader.Open ( sSPK, sError ) )
		return false;

	DWORD nIndexes = tReader.GetDword();
	dTargets.Resize ( nIndexes );
	for ( auto & tIndex : dTargets )
	{
		tIndex.m_sIndex = tReader.GetString();
		tIndex.m_uFlags = tReader.GetDword();
	}

	if ( pKillList )
	{
		DWORD nKills = tReader.GetDword();

		pKillList->Reset(nKills);
		DocID_t tDocID = 0;
		for ( int i = 0; i < (int)nKills; i++ )
		{
			DocID_t tDelta = tReader.UnzipOffset();
			assert ( tDelta>0 );
			tDocID += tDelta;
			(*pKillList)[i] = tDocID;
		}
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	return true;
}


bool WriteKillList ( const CSphString & sFilename, const DocID_t * pKlist, int nEntries, const CSphVector<KillListTarget_t> & dTargets, CSphString & sError )
{
	if ( !nEntries && !dTargets.GetLength() )
		return true;

	CSphWriter tKillList;
	if ( !tKillList.OpenFile ( sFilename, sError ) )
		return false;

	tKillList.PutDword ( dTargets.GetLength() );
	for ( const auto & tTarget : dTargets )
	{
		tKillList.PutString ( tTarget.m_sIndex );
		tKillList.PutDword ( tTarget.m_uFlags );
	}

	tKillList.PutDword ( nEntries );

	if ( pKlist )
	{
		DocID_t tPrevDocID = 0;
		for ( int i = 0; i < nEntries; i++ )
		{
			DocID_t tDocID = pKlist[i];
			tKillList.ZipOffset ( tDocID-tPrevDocID );
			tPrevDocID = tDocID;
		}
	}

	tKillList.CloseFile();
	if ( tKillList.IsError() )
	{
		sError.SetSprintf ( "error writing kill list to %s", sFilename.cstr() );
		return false;
	}

	return true;
}


bool CSphIndex_VLN::AlterKillListTarget ( CSphVector<KillListTarget_t> & dTargets, CSphString & sError )
{
	CSphFixedVector<DocID_t> dKillList(0);
	CSphVector<KillListTarget_t> dOldTargets;
	if ( !LoadKillList ( &dKillList, dOldTargets, sError ) )
		return false;

	if ( !WriteKillList ( GetIndexFileName ( SPH_EXT_SPK, true ), dKillList.Begin(), dKillList.GetLength(), dTargets, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPK, sError, false ) )
		return false;

	return true;
}


void CSphIndex_VLN::KillExistingDocids ( CSphIndex * pTarget )
{
	// FIXME! collecting all docids is a waste of memory
	LookupReader_c tLookup ( m_tDocidLookup.GetWritePtr() );
	CSphFixedVector<DocID_t> dKillList ( m_iDocinfo );
	DocID_t tDocID;
	DWORD uDocidIndex = 0;
	while ( tLookup.ReadDocID(tDocID) )
		dKillList[uDocidIndex++] = tDocID;

	pTarget->KillMulti ( dKillList.Begin(), dKillList.GetLength() );
}


int CSphIndex_VLN::KillMulti ( const DocID_t * pKlist, int iKlistSize )
{
	LookupReader_c tTargetReader ( m_tDocidLookup.GetWritePtr() );
	DocidListReader_c tKillerReader ( pKlist, iKlistSize );

	int iTotalKilled = KillByLookup ( tTargetReader, tKillerReader, m_tDeadRowMap );
	if ( iTotalKilled )
		m_uAttrsStatus |= IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED;

	return iTotalKilled;
}


/////////////////////////////////////////////////////////////////////////////

struct CmpHit_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b ) const
	{
		return ( a.m_uWordID<b.m_uWordID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_uWordPos )<HITMAN::GetPosWithField ( b.m_uWordPos ) );
	}
};


CSphString CSphIndex_VLN::GetIndexFileName ( ESphExt eExt, bool bTemp ) const
{
	CSphString sRes;
	sRes.SetSprintf ( bTemp ? "%s.tmp%s" : "%s%s", m_sFilename.cstr(), sphGetExt(eExt).cstr() );
	return sRes;
}


CSphString CSphIndex_VLN::GetIndexFileName ( const char * szExt ) const
{
	CSphString sRes;
	sRes.SetSprintf ( "%s.%s", m_sFilename.cstr(), szExt );
	return sRes;
}


class CSphHitBuilder
{
public:
	CSphHitBuilder ( const CSphIndexSettings & tSettings, const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize, CSphDict * pDict, CSphString * sError );
	~CSphHitBuilder () {}

	bool	CreateIndexFiles ( const char * sDocName, const char * sHitName, const char * sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset );
	void	HitReset ();

	void	cidxHit ( CSphAggregateHit * pHit );
	bool	cidxDone ( int iMemLimit, int & iMinInfixLen, int iMaxCodepointLen, DictHeader_t * pDictHeader );
	int		cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits );

	SphOffset_t		GetHitfilePos () const { return m_wrHitlist.GetPos (); }
	void			CloseHitlist () { m_wrHitlist.CloseFile (); }
	bool			IsError () const { return ( m_pDict->DictIsError() || m_wrDoclist.IsError() || m_wrHitlist.IsError() ); }
	void			HitblockBegin () { m_pDict->HitblockBegin(); }
	bool			IsWordDict () const { return m_pDict->GetSettings().m_bWordDict; }

private:
	void	DoclistBeginEntry ( RowID_t tDocid );
	void	DoclistEndEntry ( Hitpos_t uLastPos );
	void	DoclistEndList ();

	CSphWriter					m_wrDoclist;			///< wordlist writer
	CSphWriter					m_wrHitlist;			///< hitlist writer
	CSphWriter					m_wrSkiplist;			///< skiplist writer
	CSphFixedVector<BYTE>		m_dWriteBuffer;			///< my write buffer (for temp files)

	CSphAggregateHit			m_tLastHit;				///< hitlist entry
	Hitpos_t					m_iPrevHitPos {0};		///< previous hit position
	bool						m_bGotFieldEnd = false;
	BYTE						m_sLastKeyword [ MAX_KEYWORD_BYTES ];

	const CSphVector<SphWordID_t> &	m_dHitlessWords;
	CSphDictRefPtr_c			m_pDict;
	CSphString *				m_pLastError;

	int							m_iSkiplistBlockSize = 0;
	SphOffset_t					m_iLastHitlistPos = 0;		///< doclist entry
	SphOffset_t					m_iLastHitlistDelta = 0;	///< doclist entry
	FieldMask_t					m_dLastDocFields;		///< doclist entry
	DWORD						m_uLastDocHits = 0;			///< doclist entry

	CSphDictEntry				m_tWord;				///< dictionary entry

	ESphHitFormat				m_eHitFormat;
	ESphHitless					m_eHitless;

	CSphVector<SkiplistEntry_t>	m_dSkiplist;
#ifndef NDEBUG
	bool m_bMerging;
#endif
};


CSphHitBuilder::CSphHitBuilder ( const CSphIndexSettings & tSettings,
	const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize,
	CSphDict * pDict, CSphString * sError )
	: m_dWriteBuffer ( iBufSize )
	, m_dHitlessWords ( dHitless )
	, m_pDict ( pDict )
	, m_pLastError ( sError )
	, m_iSkiplistBlockSize ( tSettings.m_iSkiplistBlockSize )
	, m_eHitFormat ( tSettings.m_eHitFormat )
	, m_eHitless ( tSettings.m_eHitless )
#ifndef NDEBUG
	, m_bMerging ( bMerging )
#endif
{
	m_sLastKeyword[0] = '\0';
	HitReset();
	m_dLastDocFields.UnsetAll();
	SafeAddRef ( pDict );
	assert ( m_pDict );
	assert ( m_pLastError );

	m_pDict->SetSkiplistBlockSize ( m_iSkiplistBlockSize );
}


bool CSphHitBuilder::CreateIndexFiles ( const char * sDocName, const char * sHitName, const char * sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset )
{
	// doclist and hitlist files
	m_wrDoclist.CloseFile();
	m_wrHitlist.CloseFile();
	m_wrSkiplist.CloseFile();

	m_wrDoclist.SetBufferSize ( m_dWriteBuffer.GetLength() );
	m_wrHitlist.SetBufferSize ( bInplace ? iWriteBuffer : m_dWriteBuffer.GetLength() );

	if ( !m_wrDoclist.OpenFile ( sDocName, *m_pLastError ) )
		return false;

	if ( bInplace )
	{
		sphSeek ( tHit.GetFD(), 0, SEEK_SET );
		m_wrHitlist.SetFile ( tHit, pSharedOffset, *m_pLastError );
	} else
	{
		if ( !m_wrHitlist.OpenFile ( sHitName, *m_pLastError ) )
			return false;
	}

	if ( !m_wrSkiplist.OpenFile ( sSkipName, *m_pLastError ) )
		return false;

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	m_wrDoclist.PutBytes ( &bDummy, 1 );
	m_wrHitlist.PutBytes ( &bDummy, 1 );
	m_wrSkiplist.PutBytes ( &bDummy, 1 );
	return true;
}


void CSphHitBuilder::HitReset()
{
	m_tLastHit.m_tRowID = INVALID_ROWID;
	m_tLastHit.m_uWordID = 0;
	m_tLastHit.m_iWordPos = EMPTY_HIT;
	m_tLastHit.m_sKeyword = m_sLastKeyword;
	m_iPrevHitPos = 0;
	m_bGotFieldEnd = false;
}


// doclist entry format
// (with the new and shiny "inline hit" format, that is)
//
// zint docid_delta
// zint doc_hits
// if doc_hits==1:
// 		zint field_pos
// 		zint field_no
// else:
// 		zint field_mask
// 		zint hlist_offset_delta
//
// so 4 bytes/doc minimum
// avg 4-6 bytes/doc according to our tests


void CSphHitBuilder::DoclistBeginEntry ( RowID_t tRowid )
{
	assert ( m_iSkiplistBlockSize>0 );

	// build skiplist
	// that is, save decoder state and doclist position per every 128 documents
	if ( ( m_tWord.m_iDocs & ( m_iSkiplistBlockSize-1 ) )==0 )
	{
		SkiplistEntry_t & tBlock = m_dSkiplist.Add();
		tBlock.m_tBaseRowIDPlus1 = m_tLastHit.m_tRowID+1;
		tBlock.m_iOffset = m_wrDoclist.GetPos();
		tBlock.m_iBaseHitlistPos = m_iLastHitlistPos;
	}

	// begin doclist entry
	m_wrDoclist.ZipInt ( tRowid - m_tLastHit.m_tRowID );
}


void CSphHitBuilder::DoclistEndEntry ( Hitpos_t uLastPos )
{
	// end doclist entry
	if ( m_eHitFormat==SPH_HIT_FORMAT_INLINE )
	{
		bool bIgnoreHits =
			( m_eHitless==SPH_HITLESS_ALL ) ||
			( m_eHitless==SPH_HITLESS_SOME && ( m_tWord.m_iDocs & HITLESS_DOC_FLAG ) );

		// inline the only hit into doclist (unless it is completely discarded)
		// and finish doclist entry
		m_wrDoclist.ZipInt ( m_uLastDocHits );
		if ( m_uLastDocHits==1 && !bIgnoreHits )
		{
			m_wrHitlist.SeekTo ( m_iLastHitlistPos );
			m_wrDoclist.ZipInt ( uLastPos & 0x7FFFFF );
			m_wrDoclist.ZipInt ( uLastPos >> 23 );
			m_iLastHitlistPos -= m_iLastHitlistDelta;
			assert ( m_iLastHitlistPos>=0 );

		} else
		{
			m_wrDoclist.ZipInt ( m_dLastDocFields.GetMask32() );
			m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
		}
	} else // plain format - finish doclist entry
	{
		assert ( m_eHitFormat==SPH_HIT_FORMAT_PLAIN );
		m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
		m_wrDoclist.ZipInt ( m_dLastDocFields.GetMask32() );
		m_wrDoclist.ZipInt ( m_uLastDocHits );
	}
	m_dLastDocFields.UnsetAll();
	m_uLastDocHits = 0;

	// update keyword stats
	m_tWord.m_iDocs++;
}


void CSphHitBuilder::DoclistEndList ()
{
	assert ( m_iSkiplistBlockSize>0 );

	// emit eof marker
	m_wrDoclist.ZipInt ( 0 );

	// emit skiplist
	// OPTIMIZE? placing it after doclist means an extra seek on searching
	// however placing it before means some (longer) doclist data moves while indexing
	if ( m_tWord.m_iDocs>m_iSkiplistBlockSize )
	{
		assert ( m_dSkiplist.GetLength() );
		assert ( m_dSkiplist[0].m_iOffset==m_tWord.m_iDoclistOffset );
		assert ( m_dSkiplist[0].m_tBaseRowIDPlus1==0 );
		assert ( m_dSkiplist[0].m_iBaseHitlistPos==0 );

		m_tWord.m_iSkiplistOffset = m_wrSkiplist.GetPos();

		// delta coding, but with a couple of skiplist specific tricks
		// 1) first entry is omitted, it gets reconstructed from dict itself
		// both base values are zero, and offset equals doclist offset
		// 2) docids are at least SKIPLIST_BLOCK apart
		// doclist entries are at least 4*SKIPLIST_BLOCK bytes apart
		// so we additionally subtract that to improve delta coding
		// 3) zero deltas are allowed and *not* used as any markers,
		// as we know the exact skiplist entry count anyway
		SkiplistEntry_t tLast = m_dSkiplist[0];
		for ( int i=1; i<m_dSkiplist.GetLength(); i++ )
		{
			const SkiplistEntry_t & t = m_dSkiplist[i];
			assert ( t.m_tBaseRowIDPlus1 - tLast.m_tBaseRowIDPlus1>=(DWORD)m_iSkiplistBlockSize );
			assert ( t.m_iOffset - tLast.m_iOffset>=4*m_iSkiplistBlockSize );
			m_wrSkiplist.ZipInt ( t.m_tBaseRowIDPlus1 - tLast.m_tBaseRowIDPlus1 - m_iSkiplistBlockSize );
			m_wrSkiplist.ZipOffset ( t.m_iOffset - tLast.m_iOffset - 4*m_iSkiplistBlockSize );
			m_wrSkiplist.ZipOffset ( t.m_iBaseHitlistPos - tLast.m_iBaseHitlistPos );
			tLast = t;
		}
	}

	// in any event, reset skiplist
	m_dSkiplist.Resize ( 0 );
}


void CSphHitBuilder::cidxHit ( CSphAggregateHit * pHit )
{
	assert (
		( pHit->m_uWordID!=0 && pHit->m_iWordPos!=EMPTY_HIT && pHit->m_tRowID!=INVALID_ROWID ) || // it's either ok hit
		( pHit->m_uWordID==0 && pHit->m_iWordPos==EMPTY_HIT ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	const bool bNextWord = ( m_tLastHit.m_uWordID!=pHit->m_uWordID ||	( m_pDict->GetSettings().m_bWordDict && strcmp ( (char*)m_tLastHit.m_sKeyword, (char*)pHit->m_sKeyword ) ) ); // OPTIMIZE?
	const bool bNextDoc = bNextWord || ( m_tLastHit.m_tRowID!=pHit->m_tRowID );

	if ( m_bGotFieldEnd && ( bNextWord || bNextDoc ) )
	{
		// writing hits only without duplicates
		assert ( HITMAN::GetPosWithField ( m_iPrevHitPos )!=HITMAN::GetPosWithField ( m_tLastHit.m_iWordPos ) );
		HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );
		m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
		m_bGotFieldEnd = false;
	}


	if ( bNextDoc )
	{
		// finish hitlist, if any
		Hitpos_t uLastPos = m_tLastHit.m_iWordPos;
		if ( m_tLastHit.m_iWordPos!=EMPTY_HIT )
		{
			m_wrHitlist.ZipInt ( 0 );
			m_tLastHit.m_iWordPos = EMPTY_HIT;
			m_iPrevHitPos = EMPTY_HIT;
		}

		// finish doclist entry, if any
		if ( m_tLastHit.m_tRowID!=INVALID_ROWID )
			DoclistEndEntry ( uLastPos );
	}

	if ( bNextWord )
	{
		// finish doclist, if any
		if ( m_tLastHit.m_tRowID!=INVALID_ROWID )
		{
			// emit end-of-doclist marker
			DoclistEndList ();

			// emit dict entry
			m_tWord.m_uWordID = m_tLastHit.m_uWordID;
			m_tWord.m_sKeyword = m_tLastHit.m_sKeyword;
			m_tWord.m_iDoclistLength = m_wrDoclist.GetPos() - m_tWord.m_iDoclistOffset;
			m_pDict->DictEntry ( m_tWord );

			// reset trackers
			m_tWord.m_iDocs = 0;
			m_tWord.m_iHits = 0;

			m_tLastHit.m_tRowID = INVALID_ROWID;
			m_iLastHitlistPos = 0;
		}

		// flush wordlist, if this is the end
		if ( pHit->m_iWordPos==EMPTY_HIT )
		{
			m_pDict->DictEndEntries ( m_wrDoclist.GetPos() );
			return;
		}
#ifndef NDEBUG
		assert ( pHit->m_uWordID > m_tLastHit.m_uWordID
			|| ( m_pDict->GetSettings().m_bWordDict &&
				pHit->m_uWordID==m_tLastHit.m_uWordID && strcmp ( (char*)pHit->m_sKeyword, (char*)m_tLastHit.m_sKeyword )>0 )
			|| m_bMerging );
#endif // usually assert excluded in release, but this is 'paranoid' clause
		m_tWord.m_iDoclistOffset = m_wrDoclist.GetPos();
		m_tLastHit.m_uWordID = pHit->m_uWordID;
		if ( m_pDict->GetSettings().m_bWordDict )
		{
			assert ( strlen ( (char *)pHit->m_sKeyword )<sizeof(m_sLastKeyword)-1 );
			strncpy ( (char*)m_tLastHit.m_sKeyword, (char*)pHit->m_sKeyword, sizeof(m_sLastKeyword) ); // OPTIMIZE?
		}
	}

	if ( bNextDoc )
	{
		// begin new doclist entry for new doc id
		assert ( m_tLastHit.m_tRowID==INVALID_ROWID || pHit->m_tRowID>m_tLastHit.m_tRowID );
		assert ( m_wrHitlist.GetPos()>=m_iLastHitlistPos );

		DoclistBeginEntry ( pHit->m_tRowID );
		m_iLastHitlistDelta = m_wrHitlist.GetPos() - m_iLastHitlistPos;

		m_tLastHit.m_tRowID = pHit->m_tRowID;
		m_iLastHitlistPos = m_wrHitlist.GetPos();
	}

	///////////
	// the hit
	///////////

	if ( !pHit->m_dFieldMask.TestAll(false) ) // merge aggregate hits into the current hit
	{
		int iHitCount = pHit->GetAggrCount();
		assert ( m_eHitless );
		assert ( iHitCount );
		assert ( !pHit->m_dFieldMask.TestAll(false) );

		m_uLastDocHits += iHitCount;
		for ( int i=0; i<FieldMask_t::SIZE; i++ )
			m_dLastDocFields[i] |= pHit->m_dFieldMask[i];
		m_tWord.m_iHits += iHitCount;

		if ( m_eHitless==SPH_HITLESS_SOME )
			m_tWord.m_iDocs |= HITLESS_DOC_FLAG;

	} else // handle normal hits
	{
		Hitpos_t iHitPosPure = HITMAN::GetPosWithField ( pHit->m_iWordPos );

		// skip any duplicates and keep only 1st position in place
		// duplicates are hit with same position: [N, N] [N, N | FIELDEND_MASK] [N | FIELDEND_MASK, N] [N | FIELDEND_MASK, N | FIELDEND_MASK]
		if ( iHitPosPure==m_tLastHit.m_iWordPos )
			return;

		// storing previous hit that might have a field end flag
		if ( m_bGotFieldEnd )
		{
			if ( HITMAN::GetField ( pHit->m_iWordPos )!=HITMAN::GetField ( m_tLastHit.m_iWordPos ) ) // is field end flag real?
				HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );

			m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
			m_bGotFieldEnd = false;
		}

		/* duplicate hits from duplicated documents
		... 0x03, 0x03 ... 
		... 0x8003, 0x8003 ... 
		... 1, 0x8003, 0x03 ... 
		... 1, 0x03, 0x8003 ... 
		... 1, 0x8003, 0x04 ... 
		... 1, 0x03, 0x8003, 0x8003 ... 
		... 1, 0x03, 0x8003, 0x03 ... 
		*/

		assert ( m_tLastHit.m_iWordPos < pHit->m_iWordPos );

		// add hit delta without field end marker
		// or postpone adding to hitlist till got another uniq hit
		if ( iHitPosPure==pHit->m_iWordPos )
		{
			m_wrHitlist.ZipInt ( pHit->m_iWordPos - m_tLastHit.m_iWordPos );
			m_tLastHit.m_iWordPos = pHit->m_iWordPos;
		} else
		{
			assert ( HITMAN::IsEnd ( pHit->m_iWordPos ) );
			m_bGotFieldEnd = true;
			m_iPrevHitPos = m_tLastHit.m_iWordPos;
			m_tLastHit.m_iWordPos = HITMAN::GetPosWithField ( pHit->m_iWordPos );
		}

		// update matched fields mask
		m_dLastDocFields.Set ( HITMAN::GetField ( pHit->m_iWordPos ) );

		m_uLastDocHits++;
		m_tWord.m_iHits++;
	}
}


static void ReadSchemaColumn ( CSphReader & rdInfo, CSphColumnInfo & tCol )
{
	tCol.m_sName = rdInfo.GetString ();
	if ( tCol.m_sName.IsEmpty () )
		tCol.m_sName = "@emptyname";

	tCol.m_sName.ToLower ();
	tCol.m_eAttrType = (ESphAttr) rdInfo.GetDword (); // FIXME? check/fixup?

	rdInfo.GetDword (); // ignore rowitem
	tCol.m_tLocator.m_iBitOffset = rdInfo.GetDword ();
	tCol.m_tLocator.m_iBitCount = rdInfo.GetDword ();
	tCol.m_bPayload = ( rdInfo.GetByte()!=0 );

	// WARNING! max version used here must be in sync with RtIndex_c::Prealloc
}


void ReadSchema ( CSphReader & rdInfo, CSphSchema & m_tSchema )
{
	m_tSchema.Reset ();

	int iNumFields = rdInfo.GetDword();
	for ( int i=0; i<iNumFields; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		m_tSchema.AddField ( tCol );
	}

	int iNumAttrs = rdInfo.GetDword();
	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		m_tSchema.AddAttr ( tCol, false );
	}
}


static void WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol )
{
	int iLen = strlen ( tCol.m_sName.cstr() );
	fdInfo.PutDword ( iLen );
	fdInfo.PutBytes ( tCol.m_sName.cstr(), iLen );

	ESphAttr eAttrType = tCol.m_eAttrType;
	fdInfo.PutDword ( eAttrType );

	fdInfo.PutDword ( tCol.m_tLocator.CalcRowitem() ); // for backwards compatibility
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitOffset );
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitCount );

	fdInfo.PutByte ( tCol.m_bPayload );
}


void WriteSchema ( CSphWriter & fdInfo, const CSphSchema & tSchema )
{
	fdInfo.PutDword ( tSchema.GetFieldsCount() );
	for ( int i=0; i<tSchema.GetFieldsCount(); i++ )
		WriteSchemaColumn ( fdInfo, tSchema.GetField(i) );

	fdInfo.PutDword ( tSchema.GetAttrsCount() );
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( fdInfo, tSchema.GetAttr(i) );
}


void SaveIndexSettings ( CSphWriter & tWriter, const CSphIndexSettings & tSettings )
{
	tWriter.PutDword ( tSettings.m_iMinPrefixLen );
	tWriter.PutDword ( tSettings.m_iMinInfixLen );
	tWriter.PutDword ( tSettings.m_iMaxSubstringLen );
	tWriter.PutByte ( tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( tSettings.m_eHitless );
	tWriter.PutDword ( tSettings.m_eHitFormat );
	tWriter.PutByte ( tSettings.m_bIndexSP );
	tWriter.PutString ( tSettings.m_sZones );
	tWriter.PutDword ( tSettings.m_iBoundaryStep );
	tWriter.PutDword ( tSettings.m_iStopwordStep );
	tWriter.PutDword ( tSettings.m_iOvershortStep );
	tWriter.PutDword ( tSettings.m_iEmbeddedLimit );
	tWriter.PutByte ( tSettings.m_eBigramIndex );
	tWriter.PutString ( tSettings.m_sBigramWords );
	tWriter.PutByte ( tSettings.m_bIndexFieldLens );
	tWriter.PutByte ( tSettings.m_eChineseRLP );
	tWriter.PutString ( tSettings.m_sRLPContext );
	tWriter.PutString ( tSettings.m_sIndexTokenFilter );
	tWriter.PutOffset ( tSettings.m_tBlobUpdateSpace );
	tWriter.PutDword ( tSettings.m_iSkiplistBlockSize );
}


bool IndexWriteHeader ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, CSphWriter & fdInfo )
{
	// version
	fdInfo.PutDword ( INDEX_MAGIC_HEADER );
	fdInfo.PutDword ( INDEX_FORMAT_VERSION );

	// schema
	WriteSchema ( fdInfo, *tWriteHeader.m_pSchema );

	// wordlist checkpoints
	fdInfo.PutOffset ( tBuildHeader.m_iDictCheckpointsOffset );
	fdInfo.PutDword ( tBuildHeader.m_iDictCheckpoints );
	fdInfo.PutByte ( tBuildHeader.m_iInfixCodepointBytes );
	fdInfo.PutDword ( (DWORD)tBuildHeader.m_iInfixBlocksOffset );
	fdInfo.PutDword ( tBuildHeader.m_iInfixBlocksWordsSize );

	// index stats
	fdInfo.PutDword ( (DWORD)tBuildHeader.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	fdInfo.PutOffset ( tBuildHeader.m_iTotalBytes );

	// index settings
	SaveIndexSettings ( fdInfo, *tWriteHeader.m_pSettings );

	// tokenizer info
	assert ( tWriteHeader.m_pTokenizer );
	SaveTokenizerSettings ( fdInfo, tWriteHeader.m_pTokenizer, tWriteHeader.m_pSettings->m_iEmbeddedLimit );

	// dictionary info
	assert ( tWriteHeader.m_pDict );
	SaveDictionarySettings ( fdInfo, tWriteHeader.m_pDict, false, tWriteHeader.m_pSettings->m_iEmbeddedLimit );

	fdInfo.PutOffset ( tBuildHeader.m_iDocinfo );
	fdInfo.PutOffset ( tBuildHeader.m_iDocinfoIndex );
	fdInfo.PutOffset ( tBuildHeader.m_iMinMaxIndex );

	// field filter info
	SaveFieldFilterSettings ( fdInfo, tWriteHeader.m_pFieldFilter );

	// average field lengths
	if ( tWriteHeader.m_pSettings->m_bIndexFieldLens )
		for ( int i=0; i < tWriteHeader.m_pSchema->GetFieldsCount(); i++ )
			fdInfo.PutOffset ( tWriteHeader.m_pFieldLens[i] );

	return true;
}


bool IndexBuildDone ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, const CSphString & sFileName, CSphString & sError )
{
	CSphWriter fdInfo;
	if ( !fdInfo.OpenFile ( sFileName, sError ) )
		return false;

	return IndexWriteHeader ( tBuildHeader, tWriteHeader, fdInfo );
}


bool CSphHitBuilder::cidxDone ( int iMemLimit, int & iMinInfixLen, int iMaxCodepointLen, DictHeader_t * pDictHeader )
{
	assert ( pDictHeader );

	if ( m_bGotFieldEnd )
	{
		HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );
		m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
		m_bGotFieldEnd = false;
	}

	// finalize dictionary
	// in dict=crc mode, just flushes wordlist checkpoints
	// in dict=keyword mode, also creates infix index, if needed

	if ( iMinInfixLen>0 && m_pDict->GetSettings().m_bWordDict )
	{
		pDictHeader->m_iInfixCodepointBytes = iMaxCodepointLen;
		if ( iMinInfixLen==1 )
		{
			sphWarn ( "min_infix_len must be greater than 1, changed to 2" );
			iMinInfixLen = 2;
		}
	}

	if ( !m_pDict->DictEnd ( pDictHeader, iMemLimit, *m_pLastError ) )
		return false;

	// close all data files
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ( true );
	return !IsError();
}


inline int encodeVLB ( BYTE * buf, DWORD v )
{
	register BYTE b;
	register int n = 0;

	do
	{
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if ( v )
			b |= 0x80;
		*buf++ = b;
		n++;
	} while ( v );
	return n;
}


inline int encodeKeyword ( BYTE * pBuf, const char * pKeyword )
{
	int iLen = strlen ( pKeyword ); // OPTIMIZE! remove this and memcpy and check if thats faster
	assert ( iLen>0 && iLen<128 ); // so that ReadVLB()

	*pBuf = (BYTE) iLen;
	memcpy ( pBuf+1, pKeyword, iLen );
	return 1+iLen;
}


int CSphHitBuilder::cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits )
{
	assert ( pHit );
	assert ( iHits>0 );

	///////////////////////////////////////
	// encode through a small write buffer
	///////////////////////////////////////

	BYTE *pBuf, *maxP;
	int n = 0, w;
	SphWordID_t d1, l1 = 0;
	RowID_t d2, l2 = (RowID_t)-1; // rowids start from 0 and we can't have delta=0
	DWORD d3, l3 = 0; // !COMMIT must be wide enough

	int iGap = Max ( 16*sizeof(DWORD) + ( m_pDict->GetSettings().m_bWordDict ? MAX_KEYWORD_BYTES : 0 ), 128u );
	pBuf = m_dWriteBuffer.Begin();
	maxP = m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() - iGap;

	// hit aggregation state
	DWORD uHitCount = 0;
	DWORD uHitFieldMask = 0;

	const int iPositionShift = m_eHitless==SPH_HITLESS_SOME ? 1 : 0;

	while ( iHits-- )
	{
		// calc deltas
		d1 = pHit->m_uWordID - l1;
		d2 = pHit->m_tRowID - l2;
		d3 = pHit->m_uWordPos - l3;

		// ignore duplicate hits
		if ( d1==0 && d2==0 && d3==0 ) // OPTIMIZE? check if ( 0==(d1|d2|d3) ) is faster
		{
			pHit++;
			continue;
		}

		// checks below are intended handle several "fun" cases
		//
		// case 1, duplicate documents (same docid), different field contents, but ending with
		// the same keyword, ending up in multiple field end markers within the same keyword
		// eg. [foo] in positions {1, 0x800005} in 1st dupe, {3, 0x800007} in 2nd dupe
		//
		// case 2, blended token in the field end, duplicate parts, different positions (as expected)
		// for those parts but still multiple field end markers, eg. [U.S.S.R.] in the end of field

		// replacement of hit itself by field-end form
		if ( d1==0 && d2==0 && HITMAN::GetPosWithField ( pHit->m_uWordPos )==HITMAN::GetPosWithField ( l3 ) )
		{
			l3 = pHit->m_uWordPos;
			pHit++;
			continue;
		}

		// reset field-end inside token stream due of document duplicates
		if ( d1==0 && d2==0 && HITMAN::IsEnd ( l3 ) && HITMAN::GetField ( pHit->m_uWordPos )==HITMAN::GetField ( l3 ) )
		{
			l3 = HITMAN::GetPosWithField ( l3 );
			d3 = HITMAN::GetPosWithField ( pHit->m_uWordPos ) - l3;

			if ( d3==0 )
			{
				pHit++;
				continue;
			}
		}

		// non-zero delta restarts all the fields after it
		// because their deltas might now be negative
		if ( d1 ) d2 = pHit->m_tRowID+1;
		if ( d2 ) d3 = pHit->m_uWordPos;

		// when we moved to the next word or document
		bool bFlushed = false;
		if ( d1 || d2 )
		{
			// flush previous aggregate hit
			if ( uHitCount )
			{
				// we either skip all hits or the high bit must be available for marking
				// failing that, we can't produce a consistent index
				assert ( m_eHitless!=SPH_HITLESS_NONE );
				assert ( m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

				if ( m_eHitless!=SPH_HITLESS_ALL )
					uHitCount = ( uHitCount << 1 ) | 1;
				pBuf += encodeVLB ( pBuf, uHitCount );
				pBuf += encodeVLB ( pBuf, uHitFieldMask );
				assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );

				uHitCount = 0;
				uHitFieldMask = 0;

				bFlushed = true;
			}

			// start aggregating if we're skipping all hits or this word is in a list of ignored words
			if ( ( m_eHitless==SPH_HITLESS_ALL ) ||
				( m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.BinarySearch ( pHit->m_uWordID ) ) )
			{
				uHitCount = 1;
				uHitFieldMask |= 1 << HITMAN::GetField ( pHit->m_uWordPos );
			}

		} else if ( uHitCount ) // next hit for the same word/doc pair, update state if we need it
		{
			uHitCount++;
			uHitFieldMask |= 1 << HITMAN::GetField ( pHit->m_uWordPos );
		}

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 && !bFlushed ) pBuf += encodeVLB ( pBuf, 0 );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );

		// encode deltas

		// encode keyword
		if ( d1 )
		{
			if ( m_pDict->GetSettings().m_bWordDict )
				pBuf += encodeKeyword ( pBuf, m_pDict->HitblockGetKeyword ( pHit->m_uWordID ) ); // keyword itself in case of keywords dict
			else
				pBuf += sphEncodeVLB8 ( pBuf, d1 ); // delta in case of CRC dict

			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		// encode docid delta
		if ( d2 )
		{
			pBuf += sphEncodeVLB8 ( pBuf, d2 );
			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		assert ( d3 );
		if ( !uHitCount ) // encode position delta, unless accumulating hits
		{
			pBuf += encodeVLB ( pBuf, d3 << iPositionShift );
			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		// update current state
		l1 = pHit->m_uWordID;
		l2 = pHit->m_tRowID;
		l3 = pHit->m_uWordPos;

		pHit++;

		if ( pBuf>maxP )
		{
			w = (int)(pBuf - m_dWriteBuffer.Begin());
			assert ( w<m_dWriteBuffer.GetLength() );
			if ( !sphWriteThrottled ( fd, m_dWriteBuffer.Begin(), w, "raw_hits", *m_pLastError ) )
				return -1;
			n += w;
			pBuf = m_dWriteBuffer.Begin();
		}
	}

	// flush last aggregate
	if ( uHitCount )
	{
		assert ( m_eHitless!=SPH_HITLESS_NONE );
		assert ( m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

		if ( m_eHitless!=SPH_HITLESS_ALL )
			uHitCount = ( uHitCount << 1 ) | 1;
		pBuf += encodeVLB ( pBuf, uHitCount );
		pBuf += encodeVLB ( pBuf, uHitFieldMask );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
	}

	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
	w = (int)(pBuf - m_dWriteBuffer.Begin());
	assert ( w<m_dWriteBuffer.GetLength() );
	if ( !sphWriteThrottled ( fd, m_dWriteBuffer.Begin(), w, "raw_hits", *m_pLastError ) )
		return -1;
	n += w;

	return n;
}

/////////////////////////////////////////////////////////////////////////////

// OPTIMIZE?
inline bool SPH_CMPAGGRHIT_LESS ( const CSphAggregateHit & a, const CSphAggregateHit & b )
{
	if ( a.m_uWordID < b.m_uWordID )
		return true;

	if ( a.m_uWordID > b.m_uWordID )
		return false;

	if ( a.m_sKeyword )
	{
		int iCmp = strcmp ( (char*)a.m_sKeyword, (char*)b.m_sKeyword ); // OPTIMIZE?
		if ( iCmp!=0 )
			return ( iCmp<0 );
	}

	return
		( a.m_tRowID < b.m_tRowID ) ||
		( a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_iWordPos )<HITMAN::GetPosWithField ( b.m_iWordPos ) );
}


/// hit priority queue entry
struct CSphHitQueueEntry : public CSphAggregateHit
{
	int m_iBin;
};


/// hit priority queue
struct CSphHitQueue
{
public:
	CSphHitQueueEntry *		m_pData;
	int						m_iSize;
	int						m_iUsed;

public:
	/// create queue
	explicit CSphHitQueue ( int iSize )
	{
		assert ( iSize>0 );
		m_iSize = iSize;
		m_iUsed = 0;
		m_pData = new CSphHitQueueEntry [ iSize ];
	}

	/// destroy queue
	~CSphHitQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	void Push ( CSphAggregateHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		auto & tEntry = m_pData[m_iUsed];

		tEntry.m_tRowID = tHit.m_tRowID;
		tEntry.m_uWordID = tHit.m_uWordID;
		tEntry.m_sKeyword = tHit.m_sKeyword; // bin must hold the actual data for the queue
		tEntry.m_iWordPos = tHit.m_iWordPos;
		tEntry.m_dFieldMask = tHit.m_dFieldMask;
		tEntry.m_iBin = iBin;

		int iEntry = m_iUsed++;

		// sift up if needed
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) >> 1;
			if ( SPH_CMPAGGRHIT_LESS ( m_pData[iEntry], m_pData[iParent] ) )
			{
				// entry is less than parent, should float to the top
				Swap ( m_pData[iEntry], m_pData[iParent] );
				iEntry = iParent;
			} else
				break;
		}
	}

	/// remove root (ie. top priority) entry
	void Pop ()
	{
		assert ( m_iUsed );
		if ( !(--m_iUsed) ) // empty queue? just return
			return;

		// make the last entry my new root
		m_pData[0] = m_pData[m_iUsed];

		// sift down if needed
		int iEntry = 0;
		while (true)
		{
			// select child
			int iChild = (iEntry<<1) + 1;
			if ( iChild>=m_iUsed )
				break;

			// select smallest child
			if ( iChild+1<m_iUsed )
				if ( SPH_CMPAGGRHIT_LESS ( m_pData[iChild+1], m_pData[iChild] ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( SPH_CMPAGGRHIT_LESS ( m_pData[iChild], m_pData[iEntry] ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}
};


#define MAX_SOURCE_HITS	32768
static const int MIN_KEYWORDS_DICT	= 4*1048576;	// FIXME! ideally must be in sync with impl (ENTRY_CHUNKS, KEYWORD_CHUNKS)

/////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize,
	SphOffset_t * pFileSize, CSphBin * pMinBin, SphOffset_t * pSharedOffset )
{
	assert ( pBuffer && pFileSize && pMinBin && pSharedOffset );

	SphOffset_t iBlockStart = pMinBin->m_iFilePos;
	SphOffset_t iBlockLeft = pMinBin->m_iFileLeft;

	ESphBinRead eRes = pMinBin->Precache ();
	switch ( eRes )
	{
	case BIN_PRECACHE_OK:
		return true;
	case BIN_READ_ERROR:
		m_sLastError = "block relocation: preread error";
		return false;
	default:
		break;
	}

	int nTransfers = (int)( ( iBlockLeft+iRelocationSize-1) / iRelocationSize );

	SphOffset_t uTotalRead = 0;
	SphOffset_t uNewBlockStart = *pFileSize;

	for ( int i = 0; i < nTransfers; i++ )
	{
		if ( !SeekAndWarn ( iFile, iBlockStart + uTotalRead, "block relocation" ))
			return false;

		int iToRead = i==nTransfers-1 ? (int)( iBlockLeft % iRelocationSize ) : iRelocationSize;
		size_t iRead = sphReadThrottled ( iFile, pBuffer, iToRead );
		if ( iRead!=size_t(iToRead) )
		{
			m_sLastError.SetSprintf ( "block relocation: read error (%d of %d bytes read): %s", (int)iRead, iToRead, strerrorm(errno) );
			return false;
		}

		if ( !SeekAndWarn ( iFile, *pFileSize, "block relocation" ))
			return false;

		uTotalRead += iToRead;

		if ( !sphWriteThrottled ( iFile, pBuffer, iToRead, "block relocation", m_sLastError ) )
			return false;

		*pFileSize += iToRead;
	}

	assert ( uTotalRead==iBlockLeft );

	// update block pointers
	pMinBin->m_iFilePos = uNewBlockStart;
	*pSharedOffset = *pFileSize;

	return true;
}


bool CSphIndex_VLN::LoadHitlessWords ( CSphVector<SphWordID_t> & dHitlessWords )
{
	assert ( dHitlessWords.GetLength()==0 );

	if ( m_tSettings.m_sHitlessFiles.IsEmpty() )
		return true;

	const char * szStart = m_tSettings.m_sHitlessFiles.cstr();

	while ( *szStart )
	{
		while ( *szStart && ( sphIsSpace ( *szStart ) || *szStart==',' ) )
			++szStart;

		if ( !*szStart )
			break;

		const char * szWordStart = szStart;

		while ( *szStart && !sphIsSpace ( *szStart ) && *szStart!=',' )
			++szStart;

		if ( szStart - szWordStart > 0 )
		{
			CSphString sFilename;
			sFilename.SetBinary ( szWordStart, szStart-szWordStart );

			CSphAutofile tFile ( sFilename.cstr(), SPH_O_READ, m_sLastError );
			if ( tFile.GetFD()==-1 )
				return false;

			CSphVector<BYTE> dBuffer ( (int)tFile.GetSize() );
			if ( !tFile.Read ( &dBuffer[0], dBuffer.GetLength(), m_sLastError ) )
				return false;

			// FIXME!!! dict=keywords + hitless_words=some
			m_pTokenizer->SetBuffer ( &dBuffer[0], dBuffer.GetLength() );
			while ( BYTE * sToken = m_pTokenizer->GetToken() )
				dHitlessWords.Add ( m_pDict->GetWordID ( sToken ) );
		}
	}

	dHitlessWords.Uniq();
	return true;
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

class DeleteOnFail_c : public ISphNoncopyable
{
public:
	DeleteOnFail_c() : m_bShitHappened ( true )
	{}
	~DeleteOnFail_c()
	{
		if ( m_bShitHappened )
		{
			ARRAY_FOREACH ( i, m_dWriters )
				m_dWriters[i]->UnlinkFile();

			ARRAY_FOREACH ( i, m_dAutofiles )
				m_dAutofiles[i]->SetTemporary();
		}
	}
	void AddWriter ( CSphWriter * pWr )
	{
		if ( pWr )
			m_dWriters.Add ( pWr );
	}
	void AddAutofile ( CSphAutofile * pAf )
	{
		if ( pAf )
			m_dAutofiles.Add ( pAf );
	}
	void AllIsDone()
	{
		m_bShitHappened = false;
	}
private:
	bool	m_bShitHappened;
	CSphVector<CSphWriter*> m_dWriters;
	CSphVector<CSphAutofile*> m_dAutofiles;
};


bool CSphIndex_VLN::CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer )
{
	CSphBitvec dQueryMvas ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( tAttr.m_eSrc!=SPH_ATTRSRC_FIELD && ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) )
			dQueryMvas.BitSet(i);
	}

	if ( !dQueryMvas.BitCount() )
		return true;

	assert ( !tMvaContainer.m_tContainer.GetLength() );
	tMvaContainer.m_tContainer.Resize ( m_tSchema.GetAttrsCount() );
	for ( auto & i : tMvaContainer.m_tContainer )
		i = nullptr;

	for ( auto & pSource : dSources )
	{
		assert ( pSource );
		if ( !pSource->Connect ( m_sLastError ) )
			return false;

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			if ( !dQueryMvas.BitGet(i) )
				continue;

			auto * & pHash = tMvaContainer.m_tContainer[i];
			if ( !pHash )
				pHash = new OpenHash_T<CSphVector<int64_t>, int64_t, HashFunc_Int64_t>;

			if ( !pSource->IterateMultivaluedStart ( i, m_sLastError ) )
				return false;

			int64_t iDocID;
			int64_t iMvaValue;
			while ( pSource->IterateMultivaluedNext ( iDocID, iMvaValue) )
			{
				auto & tMva = pHash->Acquire ( iDocID );
				tMva.Add ( iMvaValue );
			}
		}

		pSource->Disconnect ();
	}

	return true;
}

struct Mva32Uniq_fn
{
	bool IsLess ( const uint64_t & iA, const uint64_t & iB ) const
	{
		DWORD uA = (DWORD)iA;
		DWORD uB = (DWORD)iB;

		return uA<uB;
	}

	bool IsEq ( const uint64_t & iA, const uint64_t & iB ) const
	{
		DWORD uA = (DWORD)iA;
		DWORD uB = (DWORD)iB;
		return uA==uB;
	}
};


static void SortMva ( CSphVector<int64_t> & tMva, bool bMva32 )
{
	if ( !bMva32 )
	{
		tMva.Uniq();
		return;
	}

	tMva.Sort ( Mva32Uniq_fn() );
	int iLeft = sphUniq ( tMva.Begin(), tMva.GetLength(), Mva32Uniq_fn() );
	tMva.Resize ( iLeft );
}


bool CSphIndex_VLN::StoreBlobAttrs ( DocID_t tDocId, SphOffset_t & tOffset, BlobRowBuilder_i & tBlobRowBuilder, QueryMvaContainer_c & tMvaContainer, BlobSource_i & tSource, bool bForceSource )
{
	CSphString sError;
	int iBlobAttr = 0;

	for ( int i=0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);

		bool bOk = true;
		bool bFatal = false;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			{
				CSphVector<int64_t> * pMva;
				if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD || bForceSource )
					pMva = tSource.GetFieldMVA(i);
				else
				{
					assert ( tMvaContainer.m_tContainer[i] );
					pMva = tMvaContainer.m_tContainer[i]->Find(tDocId);
				}

				if ( pMva )
				{
					SortMva ( *pMva, ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET ) );
					bOk = tBlobRowBuilder.SetAttr ( iBlobAttr++, (const BYTE*)(pMva->Begin()), pMva->GetLength()*sizeof(int64_t), sError );
				} else
					bOk = tBlobRowBuilder.SetAttr ( iBlobAttr++, nullptr, 0, sError );
			}
			break;

		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:
			{
				const CSphString & sStrAttr = tSource.GetStrAttr(i);
				bOk = tBlobRowBuilder.SetAttr ( iBlobAttr++, (const BYTE*)sStrAttr.cstr(), sStrAttr.Length(), sError );
				if ( !bOk )
					bFatal = tAttr.m_eAttrType==SPH_ATTR_JSON && g_bJsonStrict;
			}
			break;

		default:
			break;
		}

		if ( !bOk )
		{
			sError.SetSprintf ( "document " INT64_FMT ", attribute %s: %s", tDocId, tAttr.m_sName.cstr(), sError.cstr() );
			if ( bFatal )
			{
				m_sLastError = sError;
				return false;
			}
			else
				sphWarning ( "%s", sError.cstr() );
		}
	}

	tOffset = tBlobRowBuilder.Flush();
	return true;
}

template <typename T>
void SourceCopyMva ( const BYTE * pData, int iLenBytes, CSphVector<int64_t> & dDst )
{
	const T * pSrc = (const T *)pData;
	int iValues = iLenBytes / sizeof(T);

	dDst.Resize ( iValues );
	int64_t * pDst = dDst.Begin();
	const int64_t * pDstEnd = pDst + iValues;
	while ( pDst<pDstEnd )
	{
		*pDst = sphUnalignedRead ( *pSrc );
		pSrc++;
		pDst++;
	}
}

class KeepAttrs_c : public BlobSource_i
{
public:
	explicit KeepAttrs_c ( QueryMvaContainer_c & tMvaContainer )
		: m_pIndex ( nullptr )
		, m_tMvaContainer ( tMvaContainer )
	{}

	virtual ~KeepAttrs_c() override
	{}

	void SetBlobSource ( BlobSource_i * pSource )
	{
		m_pBlobSource = pSource;
	}

	bool Init ( const CSphString & sKeepAttrs, const StrVec_t & dKeepAttrs, const CSphSchema & tSchema )
	{
		if ( sKeepAttrs.IsEmpty() && !dKeepAttrs.GetLength() )
			return false;

		m_bHasBlobAttrs = tSchema.HasBlobAttrs();
		m_iStride = tSchema.GetRowSize();

		CSphString sError;
		CSphString sWarning;

		m_pIndex = (CSphIndex_VLN *)sphCreateIndexPhrase ( "keep-attrs", sKeepAttrs.cstr() );
		m_pIndex->SetMemorySettings ( FileAccessSettings_t() );

		if ( !m_pIndex->Prealloc ( false ) )
		{
			if ( !sWarning.IsEmpty() )
				sError.SetSprintf ( "warning: '%s',", sWarning.cstr() );
			if ( !m_pIndex->GetLastError().IsEmpty() )
				sError.SetSprintf ( "%s error: '%s'", sError.scstr(), m_pIndex->GetLastError().cstr() );
			sphWarn ( "unable to load 'keep-attrs' index (%s); ignoring --keep-attrs", sError.cstr() );

			m_pIndex.Reset();
		} else
		{
			// check schema
			if ( !tSchema.CompareTo ( m_pIndex->GetMatchSchema(), sError, false ) )
			{
				sphWarn ( "schemas are different (%s); ignoring --keep-attrs", sError.cstr() );
				m_pIndex.Reset();
			}
		}

		if ( m_pIndex.Ptr() )
		{
			if ( dKeepAttrs.GetLength() )
			{
				m_dLocMva.Init ( tSchema.GetAttrsCount() );
				m_dLocString.Init ( tSchema.GetAttrsCount() );
				m_bKeepSomeAttrs = true;

				ARRAY_FOREACH ( i, dKeepAttrs )
				{
					int iCol = tSchema.GetAttrIndex ( dKeepAttrs[i].cstr() );
					if ( iCol==-1 )
					{
						sphWarn ( "no attribute found '%s'; ignoring --keep-attrs", dKeepAttrs[i].cstr() );
						m_pIndex.Reset();
						break;
					}

					const CSphColumnInfo & tCol = tSchema.GetAttr ( iCol );
					if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
					{
						m_dLocMva.BitSet ( iCol );
						m_bHasMva = true;
					} else if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_JSON )
					{
						m_dLocString.BitSet ( iCol );
						m_bHasString = true;
					} else
					{
						m_dLocPlain.Add ( tCol.m_tLocator );
					}
				}
			}

			m_dMvaField.Init ( tSchema.GetAttrsCount() );
			for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
			{
				const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
				if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tCol.m_eSrc==SPH_ATTRSRC_FIELD )
				{
					m_dMvaField.BitSet ( i );
				}
			}
		}

		if ( m_pIndex.Ptr() )
			m_pIndex->Preread();

		return ( m_pIndex.Ptr()!=nullptr );
	}

	bool Keep ( DocID_t tDocid )
	{
		if ( !m_pIndex.Ptr() )
			return false;

		m_tDocid = tDocid;
		m_pRow = m_pIndex->FindDocinfo ( tDocid );
		return ( m_pRow!=nullptr );
	}

	virtual CSphVector<int64_t> * GetFieldMVA ( int iAttr ) override
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return nullptr;

		// fallback to indexed data
		if ( m_bKeepSomeAttrs && ( !m_bHasMva || !m_dLocMva.BitGet ( iAttr ) ) )
		{
			if ( m_dMvaField.BitGet ( iAttr ) )
			{
				return m_pBlobSource->GetFieldMVA ( iAttr );
			} else
			{
				assert ( m_tMvaContainer.m_tContainer[iAttr] );
				return m_tMvaContainer.m_tContainer[iAttr]->Find ( m_tDocid );
			}
		}

		int iLen = 0;
		ESphAttr eAttr = SPH_ATTR_NONE;
		const BYTE * pData = GetBlobData ( iAttr, iLen, eAttr );
		assert ( eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET );

		if ( eAttr==SPH_ATTR_INT64SET )
		{
			SourceCopyMva<int64_t> ( pData, iLen, m_dDataMva );
		} else
		{
			SourceCopyMva<DWORD> ( pData, iLen, m_dDataMva );
		}

		return &m_dDataMva;
	}

	/// returns string attributes for a given attribute
	virtual const CSphString & GetStrAttr ( int iAttr ) override
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return m_sEmpty;

		// fallback to indexed data
		if ( m_bKeepSomeAttrs && ( !m_bHasString || !m_dLocString.BitGet ( iAttr ) ) )
		{
			assert ( m_pBlobSource );
			return m_pBlobSource->GetStrAttr ( iAttr );
		}

		int iLen = 0;
		ESphAttr eAttr = SPH_ATTR_NONE;
		const BYTE * pData = GetBlobData ( iAttr, iLen, eAttr );
		assert ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_JSON );

		if ( !iLen )
			return m_sEmpty;

		m_sDataString.SetBinary ( (const char *)pData, iLen );
		return m_sDataString;
	}

	const CSphRowitem * GetRow ( CSphRowitem * pSrc )
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return pSrc;

		// keep only blob attributes
		if ( m_bKeepSomeAttrs && !m_dLocPlain.GetLength() )
			return pSrc;

		if ( m_bKeepSomeAttrs )
		{
			// keep only some plain attributes
			ARRAY_FOREACH ( i, m_dLocPlain )
			{
				const CSphAttrLocator & tLoc = m_dLocPlain[i];
				SphAttr_t tAtrr = sphGetRowAttr ( m_pRow, tLoc );
				sphSetRowAttr ( pSrc, tLoc, tAtrr );
			}
			return pSrc;
		}
		else if ( m_bHasBlobAttrs )
		{
			// copy whole row except blob row offset
			SphOffset_t tOffset = sphGetBlobRowOffset(pSrc);
			memcpy ( pSrc, m_pRow, m_iStride*sizeof(CSphRowitem) );
			sphSetBlobRowOffset ( pSrc, tOffset );
			return pSrc;
		}

		// keep whole row
		return m_pRow;
	}

	void Reset()
	{
		m_pIndex.Reset();
	}

private:
	CSphScopedPtr<CSphIndex_VLN>	m_pIndex;
	CSphVector<CSphAttrLocator>		m_dLocPlain;
	CSphBitvec						m_dLocMva;
	CSphBitvec						m_dMvaField;
	CSphBitvec						m_dLocString;

	bool							m_bKeepSomeAttrs = false;
	bool							m_bHasMva = false;
	bool							m_bHasString = false;
	bool							m_bHasBlobAttrs = false;

	int								m_iStride = 0;
	const CSphRowitem *				m_pRow = nullptr;
	CSphVector<int64_t>				m_dDataMva;
	CSphString						m_sDataString;
	const CSphString				m_sEmpty = "";

	BlobSource_i *					m_pBlobSource = nullptr;
	DocID_t							m_tDocid = 0;
	QueryMvaContainer_c &			m_tMvaContainer;


	const BYTE * GetBlobData ( int iAttr, int & iLen, ESphAttr & eAttr )
	{
		const BYTE * pPool = m_pIndex->m_tBlobAttrs.GetWritePtr();
		const CSphColumnInfo & tCol = m_pIndex->GetMatchSchema().GetAttr ( iAttr );
		assert ( tCol.m_tLocator.IsBlobAttr() );
		eAttr = tCol.m_eAttrType;

		return sphGetBlobAttr ( m_pRow, tCol.m_tLocator, pPool, iLen );
	}
};


void WarnAboutKillList ( const CSphVector<DocID_t> & dKillList, const CSphVector<KillListTarget_t> & dTargets )
{
	if ( dKillList.GetLength() && !dTargets.GetLength() )
		sphWarn ( "kill-list not empty but no killlist_target specified" );

	if ( !dKillList.GetLength() )
	{
		for ( const auto & tTarget : dTargets )
			if ( tTarget.m_uFlags==KillListTarget_t::USE_KLIST )
			{
				sphWarn ( "killlist_target is specified but kill-list is empty" );
				break;
			}
	}
}


bool CSphIndex_VLN::SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock )
{
	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_LOOKUP;
	m_tProgress.m_iDocids = 0;
	m_tProgress.m_iDocidsTotal = m_tStats.m_iTotalDocuments;

	if ( !nBlocks )
		return true;

	DocidLookupWriter_c tWriter ( (DWORD)m_tStats.m_iTotalDocuments );
	if ( !tWriter.Open ( GetIndexFileName(SPH_EXT_SPT), m_sLastError ) )
		return false;

	DeleteOnFail_c tWatchdog;
	tWatchdog.AddWriter ( &tWriter.GetWriter() );

	CSphVector<CSphBin*> dBins;
	SphOffset_t iSharedOffset = -1;

	dBins.Reserve ( nBlocks );

	int iBinSize = CSphBin::CalcBinSize ( iMemoryLimit, nBlocks, "sort_lookup" );

	for ( int i=0; i<nBlocks; i++ )
	{
		dBins.Add ( new CSphBin() );
		dBins[i]->m_iFileLeft = ( ( i==nBlocks-1 ) ? nLookupsInLastBlock : nLookupsInBlock )*sizeof(DocidRowidPair_t);
		dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( iFD, &iSharedOffset, iBinSize );
	}

	CSphFixedVector<DocidRowidPair_t> dTopDocIDs ( nBlocks );
	CSphQueue<int, CmpQueuedLookup_fn> tLookupQueue ( nBlocks );

	CmpQueuedLookup_fn::m_pStorage = dTopDocIDs.Begin();

	for ( int i=0; i<nBlocks; i++ )
	{
		if ( dBins[i]->ReadBytes ( &dTopDocIDs[i], sizeof(DocidRowidPair_t) )!=BIN_READ_OK )
		{
			m_sLastError.SetSprintf ( "sort_lookup: warmup failed (io error?)" );
			return 0;
		}

		tLookupQueue.Push(i);
	}

	DWORD tProcessed = 0;
	while ( tLookupQueue.GetLength() )
	{
		int iBin = tLookupQueue.Root();

		tWriter.AddPair ( dTopDocIDs[iBin] );

		tLookupQueue.Pop();
		ESphBinRead eRes = dBins[iBin]->ReadBytes ( &dTopDocIDs[iBin], sizeof(DocidRowidPair_t) );
		if ( eRes==BIN_READ_ERROR )
		{
			m_sLastError.SetSprintf ( "sort_lookup: failed to read entry" );
			return false;
		}

		if ( eRes==BIN_READ_OK )
			tLookupQueue.Push(iBin);

		tProcessed++;
		if ( ( tProcessed % 10000 )==0 )
		{
			m_tProgress.m_iDocids = tProcessed;
			m_tProgress.Show ( false );
		}
	}

	if ( !tWriter.Finalize ( m_sLastError ) )
		return false;

	// clean up readers
	ARRAY_FOREACH ( i, dBins )
		SafeDelete ( dBins[i] );

	tWatchdog.AllIsDone();

	m_tProgress.m_iDocids = m_tProgress.m_iDocidsTotal;
	m_tProgress.Show(true);

	return true;
}


bool CSphIndex_VLN::CreateHistograms ( const AttrIndexBuilder_c & tMinMax )
{
	if ( !m_tStats.m_iTotalDocuments )
		return true;

	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_HISTOGRAMS;
	m_tProgress.m_iDocids = 0;
	m_tProgress.m_iDocidsTotal = m_tStats.m_iTotalDocuments;

	const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMax.GetCollected();
	int iStride = m_tSchema.GetRowSize();
	const CSphRowitem * pMinRow = dMinMaxRows.Begin()+dMinMaxRows.GetLength()-iStride*2;
	const CSphRowitem * pMaxRow = pMinRow+iStride;

	CSphAutofile tSPA ( GetIndexFileName(SPH_EXT_SPA), SPH_O_READ, m_sLastError );
	if ( tSPA.GetFD()==-1 )
		return false;

	CSphReader tSPAReader;
	tSPAReader.SetFile(tSPA);

	HistogramContainer_c tHistogramContainer;
	CSphVector<Histogram_i *> dHistograms;
	CSphVector<CSphColumnInfo> dPOD;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
		{
			Verify ( tHistogramContainer.Add ( pHistogram ) );
			dHistograms.Add ( pHistogram );
			dPOD.Add ( tAttr );
			pHistogram->Setup ( sphGetRowAttr ( pMinRow, tAttr.m_tLocator ), sphGetRowAttr ( pMaxRow, tAttr.m_tLocator ) );
		}
	}

	CSphVector<CSphRowitem> dRow ( iStride );
	CSphRowitem * pRow = dRow.Begin();

	DWORD uTotalDocs = m_tStats.m_iTotalDocuments;
	for ( RowID_t tRowID = 0; tRowID < uTotalDocs; tRowID++ )
	{
		tSPAReader.GetBytes ( pRow, iStride*sizeof(CSphRowitem) );
		if ( tSPAReader.GetErrorFlag() )
		{
			m_sLastError = tSPAReader.GetErrorMessage();
			return false;
		}

		ARRAY_FOREACH ( i, dHistograms )
			dHistograms[i]->Insert ( sphGetRowAttr ( pRow, dPOD[i].m_tLocator ) );

		if ( ( tRowID % 10000 )==0 )
		{
			m_tProgress.m_iDocids = tRowID;
			m_tProgress.Show ( false );
		}
	}

	if ( !tHistogramContainer.Save ( GetIndexFileName(SPH_EXT_SPHI), m_sLastError ) )
		return false;

	m_tProgress.m_iDocids = m_tProgress.m_iDocidsTotal;
	m_tProgress.Show(true);

	return true;
}


int CSphIndex_VLN::Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer )
{
	assert ( dSources.GetLength() );

	CSphVector<SphWordID_t> dHitlessWords;

	if ( !LoadHitlessWords ( dHitlessWords ) )
		return 0;

	int iHitBuilderBufferSize = ( iWriteBuffer>0 )
		? Max ( iWriteBuffer, MIN_WRITE_BUFFER )
		: DEFAULT_WRITE_BUFFER;

	// vars shared between phases
	CSphVector<CSphBin*> dBins;
	SphOffset_t iSharedOffset = -1;

	m_pDict->HitblockBegin();

	// setup sources
	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		pSource->SetDict ( m_pDict );
		pSource->Setup ( m_tSettings );
	}

	// connect 1st source and fetch its schema
	// and don't disconnect it because some sources can't survive connect/disconnect
	CSphSource * pSource0 = dSources[0];
	if ( !pSource0->Connect ( m_sLastError )
		|| !pSource0->IterateStart ( m_sLastError )
		|| !pSource0->UpdateSchema ( &m_tSchema, m_sLastError )
		|| !pSource0->SetupMorphFields ( m_sLastError ) )
	{
		return 0;
	}

	bool bHaveQueryMVAs = false;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			bHaveQueryMVAs = true;
			break;
		}
	}

	// non-SQL sources don't survive connect+disconnect+reconnect
	// and if we don't disconnect now we won't be able to fetch query MVAs from SQL sources
	QueryMvaContainer_c tQueryMvaContainer;
	if ( bHaveQueryMVAs )
	{
		pSource0->Disconnect();

		// temporary storage for MVAs that we fetch from queries
		// we might want to dump that to file later
		if ( !CollectQueryMvas ( dSources, tQueryMvaContainer ) )
			return 0;
	}

	if ( m_tSchema.GetFieldsCount()==0 )
	{
		m_sLastError.SetSprintf ( "No fields in schema - will not index" );
		return 0;
	}

	int iFieldLens = m_tSchema.GetAttrId_FirstFieldLen();

	const CSphColumnInfo * pBlobLocatorAttr = m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	bool bHaveBlobAttrs = !!pBlobLocatorAttr;

	if ( !m_pTokenizer->SetFilterSchema ( m_tSchema, m_sLastError ) )
		return 0;

	CSphHitBuilder tHitBuilder ( m_tSettings, dHitlessWords, false, iHitBuilderBufferSize, m_pDict, &m_sLastError );

	////////////////////////////////////////////////
	// collect and partially sort hits
	////////////////////////////////////////////////

	CSphVector <DocID_t> dKillList;

	// adjust memory requirements
	int iOldLimit = iMemoryLimit;

	// book at least 2 MB for keywords dict, if needed
	int iDictSize = 0;
	if ( m_pDict->GetSettings().m_bWordDict )
		iDictSize = Max ( MIN_KEYWORDS_DICT, iMemoryLimit/8 );

	// reserve for sorting docid-rowid pairs
	int iDocidLookupSize = Max ( 32768, iMemoryLimit/16 );

	iMemoryLimit -= iDocidLookupSize+iDictSize;

	// do we have enough left for hits?
	int iHitsMax = 1048576;
	if ( iMemoryLimit < iHitsMax*(int)sizeof(CSphWordHit) )
	{
		iMemoryLimit = iOldLimit + iHitsMax*sizeof(CSphWordHit) - iMemoryLimit;
		sphWarn ( "collect_hits: mem_limit=%d kb too low, increasing to %d kb",	iOldLimit/1024, iMemoryLimit/1024 );
	} else
		iHitsMax = iMemoryLimit / sizeof(CSphWordHit);

	// allocate raw hits block
	CSphFixedVector<CSphWordHit> dHits ( iHitsMax + MAX_SOURCE_HITS );
	CSphWordHit * pHits = dHits.Begin();
	CSphWordHit * pHitsMax = dHits.Begin() + iHitsMax;

	int nDocidLookupsPerBlock = iDocidLookupSize/sizeof(DocidRowidPair_t);
	int nDocidLookup = 0;
	int nDocidLookupBlocks = 0;
	CSphFixedVector<DocidRowidPair_t> dDocidLookup ( nDocidLookupsPerBlock );

	// fallback blob source (for mva)
	KeepAttrs_c tPrevAttrs ( tQueryMvaContainer );
	const bool bGotPrevIndex = tPrevAttrs.Init ( m_sKeepAttrs, m_dKeepAttrs, m_tSchema );

	// create temp files
	CSphString sFileSPP = m_bInplaceSettings ? GetIndexFileName(SPH_EXT_SPP) : GetIndexFileName("tmp1");

	CSphAutofile fdLock ( GetIndexFileName("tmp0"), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdHits ( sFileSPP, SPH_O_NEW, m_sLastError, !m_bInplaceSettings );
	CSphAutofile fdTmpLookup ( GetIndexFileName("tmp2"), SPH_O_NEW, m_sLastError, true );

	CSphWriter tWriterSPA;

	// write to temp file because of possible --keep-attrs option which loads prev index
	CSphString sSPA = GetIndexFileName ( SPH_EXT_SPA, true );
	if ( !tWriterSPA.OpenFile ( sSPA, m_sLastError ) )
		return 0;

	DeleteOnFail_c dFileWatchdog;

	if ( m_bInplaceSettings )
		dFileWatchdog.AddAutofile ( &fdHits );

	if ( fdLock.GetFD()<0 || fdHits.GetFD()<0 )
		return 0;

	CSphString sSPB = GetIndexFileName ( SPH_EXT_SPB, true );
	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder(nullptr);
	if ( bHaveBlobAttrs )
	{
		pBlobRowBuilder = sphCreateBlobRowBuilder ( m_tSchema, sSPB, m_tSettings.m_tBlobUpdateSpace, m_sLastError );
		if ( !pBlobRowBuilder.Ptr() )
			return 0;
	}

	SphOffset_t iHitsGap = 0;

	if ( m_bInplaceSettings )
	{
		const int HIT_SIZE_AVG = 4;
		const float HIT_BLOCK_FACTOR = 1.0f;

		if ( m_iHitGap )
			iHitsGap = (SphOffset_t) m_iHitGap;
		else
			iHitsGap = (SphOffset_t)( iHitsMax*HIT_BLOCK_FACTOR*HIT_SIZE_AVG );

		iHitsGap = Max ( iHitsGap, 1 );
		if ( !SeekAndWarn ( fdHits.GetFD (), iHitsGap, "CSphIndex_VLN::Build" ))
			return 0;
	}

	if ( !sphLockEx ( fdLock.GetFD(), false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock '%s': another indexer running?", fdLock.GetFilename() );
		return 0;
	}

	m_tStats.Reset ();
	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;

	CSphVector<int> dHitBlocks;
	dHitBlocks.Reserve ( 1024 );

	AttrIndexBuilder_c tMinMax(m_tSchema);
	RowID_t tRowID = 0;

	ARRAY_FOREACH ( iSource, dSources )
	{
		// connect and check schema
		CSphSource * pSource = dSources[iSource];

		bool bNeedToConnect = iSource>0 || bHaveQueryMVAs;
		if ( bNeedToConnect )
		{
			if ( !pSource->Connect ( m_sLastError )
				|| !pSource->IterateStart ( m_sLastError )
				|| !pSource->UpdateSchema ( &m_tSchema, m_sLastError )
				|| !pSource->SetupMorphFields ( m_sLastError ) )
			{
				return 0;
			}
		}

		// joined filter
		bool bGotJoined = pSource->HasJoinedFields();

		// fallback blob source (for string and json )
		if ( bGotPrevIndex )
			tPrevAttrs.SetBlobSource ( pSource );

		// fetch documents
		for ( ;; )
		{
			// get next doc, and handle errors
			bool bEOF = false;
			if ( !pSource->IterateDocument ( bEOF, m_sLastError ) )
				return 0;

			// check if we have no more documents
			if ( bEOF )
				break;

			pSource->m_tDocInfo.m_tRowID = tRowID++;
			DocID_t tDocID = sphGetDocID ( pSource->m_tDocInfo.m_pDynamic );

			pSource->RowIDAssigned ( tDocID, tRowID-1 );
			bool bKeepRow = ( bGotPrevIndex && tPrevAttrs.Keep ( tDocID ) );

			// show progress bar
			if ( ( pSource->GetStats().m_iTotalDocuments % 1000 )==0 )
			{
				m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				m_tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				m_tProgress.Show ( false );
			}

			// update crashdump
			g_iIndexerCurrentDocID = pSource->m_tDocInfo.m_tRowID;
			g_iIndexerCurrentHits = pHits-dHits.Begin();

			// store mva, strings and JSON blobs
			if ( pBlobRowBuilder.Ptr() )
			{
				BlobSource_i * pBlobSource = pSource;
				if ( bKeepRow )
					pBlobSource = &tPrevAttrs;
				SphOffset_t tBlobOffset = 0;
				if ( !StoreBlobAttrs ( tDocID, tBlobOffset, *pBlobRowBuilder.Ptr(), tQueryMvaContainer, *pBlobSource, bKeepRow ) )
					return 0;

				pSource->m_tDocInfo.SetAttr ( pBlobLocatorAttr->m_tLocator, tBlobOffset );
			}

			// store hits
			while ( const ISphHits * pDocHits = pSource->IterateHits ( m_sLastWarning ) )
			{
				int iDocHits = pDocHits->Length();
#if PARANOID
				for ( int i=0; i<iDocHits; i++ )
				{
					assert ( pDocHits->m_dData[i].m_uDocID==pSource->m_tDocInfo.m_uDocID );
					assert ( pDocHits->m_dData[i].m_uWordID );
					assert ( pDocHits->m_dData[i].m_iWordPos );
				}
#endif

				assert ( ( pHits+iDocHits )<=( pHitsMax+MAX_SOURCE_HITS ) );

				memcpy ( pHits, pDocHits->First(), iDocHits*sizeof(CSphWordHit) );
				pHits += iDocHits;

				// check if we need to flush
				if ( pHits<pHitsMax	&& !( iDictSize && m_pDict->HitblockGetMemUse() > iDictSize ) )
					continue;

				// update crashdump
				g_iIndexerPoolStartDocID = tDocID;
				g_iIndexerPoolStartHit = pHits-dHits.Begin();

				// sort hits
				int iHits = pHits - dHits.Begin();
				{
					sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
					m_pDict->HitblockPatch ( dHits.Begin(), iHits );
				}
				pHits = dHits.Begin();

				// flush hits, docs are flushed independently
				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );

				m_pDict->HitblockReset ();

				if ( dHitBlocks.Last()<0 )
					return 0;

				// progress bar
				m_tProgress.m_iHitsTotal += iHits;
				m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				m_tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				m_tProgress.Show ( false );
			}

			// update total field lengths
			if ( iFieldLens>=0 )
			{
				for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
					m_dFieldLens[i] += pSource->m_tDocInfo.GetAttr ( m_tSchema.GetAttr ( i+iFieldLens ).m_tLocator );
			}

			// store docinfo
			// with the advent of SPH_ATTR_TOKENCOUNT, now MUST be done AFTER iterating the hits
			// because field lengths are computed during that iterating
			const CSphRowitem * pRow = pSource->m_tDocInfo.m_pDynamic;
			if ( bKeepRow )
				pRow = tPrevAttrs.GetRow ( pSource->m_tDocInfo.m_pDynamic );
			tMinMax.Collect ( pRow );
			tWriterSPA.PutBytes ( pRow, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );

			dDocidLookup[nDocidLookup].m_tDocID = tDocID;
			dDocidLookup[nDocidLookup].m_tRowID = pSource->m_tDocInfo.m_tRowID;
			nDocidLookup++;

			if ( nDocidLookup==dDocidLookup.GetLength() )
			{
				dDocidLookup.Sort ( CmpDocidLookup_fn() );
				if ( !sphWriteThrottled ( fdTmpLookup.GetFD (), &dDocidLookup[0], nDocidLookup*sizeof(DocidRowidPair_t), "temp_docid_lookup", m_sLastError ) )
					return 0;

				nDocidLookup = 0;
				nDocidLookupBlocks++;
			}

			// go on, loop next document
		}

		// FIXME! uncontrolled memory usage; add checks and/or diskbased sort in the future?
		if ( pSource->IterateKillListStart ( m_sLastError ) )
		{
			DocID_t tKilllistDocID;
			while ( pSource->IterateKillListNext ( tKilllistDocID ) )
				dKillList.Add ( tKilllistDocID );
		} else if ( !m_sLastError.IsEmpty() )
			return 0;

		// fetch joined fields
		if ( bGotJoined )
		{
			// flush tail of regular hits
			int iHits = pHits - dHits.Begin();
			if ( iDictSize && m_pDict->HitblockGetMemUse() && iHits )
			{
				sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
				m_pDict->HitblockPatch ( dHits.Begin(), iHits );
				pHits = dHits.Begin();
				m_tProgress.m_iHitsTotal += iHits;
				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );
				if ( dHitBlocks.Last()<0 )
					return 0;
				m_pDict->HitblockReset ();
			}

			while (true)
			{
				// get next doc, and handle errors
				ISphHits * pJoinedHits = pSource->IterateJoinedHits ( m_sLastError );
				if ( !pJoinedHits )
					return 0;

				// check for eof
				if ( pSource->m_tDocInfo.m_tRowID==INVALID_ROWID )
					break;

				int iJoinedHits = pJoinedHits->Length();
				memcpy ( pHits, pJoinedHits->First(), iJoinedHits*sizeof(CSphWordHit) );
				pHits += iJoinedHits;

				// check if we need to flush
				if ( pHits<pHitsMax && !( iDictSize && m_pDict->HitblockGetMemUse() > iDictSize ) )
					continue;

				// store hits
				int iStoredHits = pHits - dHits.Begin();
				sphSort ( dHits.Begin(), iStoredHits, CmpHit_fn() );
				m_pDict->HitblockPatch ( dHits.Begin(), iStoredHits );

				pHits = dHits.Begin();
				m_tProgress.m_iHitsTotal += iStoredHits;

				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iStoredHits ) );
				if ( dHitBlocks.Last()<0 )
					return 0;
				m_pDict->HitblockReset ();
			}
		}

		// this source is over, disconnect and update stats
		pSource->Disconnect ();

		m_tStats.m_iTotalDocuments += pSource->GetStats().m_iTotalDocuments;
		m_tStats.m_iTotalBytes += pSource->GetStats().m_iTotalBytes;
	}

	if ( m_tStats.m_iTotalDocuments>=INT_MAX )
	{
		m_sLastError.SetSprintf ( "index over %d documents not supported (got documents count=" INT64_FMT ")", INT_MAX, m_tStats.m_iTotalDocuments );
		return 0;
	}

	// flush last hit block
	if ( pHits>dHits.Begin() )
	{
		int iHits = pHits - dHits.Begin();
		{
			sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
			m_pDict->HitblockPatch ( dHits.Begin(), iHits );
		}
		m_tProgress.m_iHitsTotal += iHits;

		dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );

		m_pDict->HitblockReset ();

		if ( dHitBlocks.Last()<0 )
			return 0;
	}

	// reset hits pool
	dHits.Reset(0);

	if ( nDocidLookup )
	{
		dDocidLookup.Sort ( CmpDocidLookup_fn(), 0, nDocidLookup-1 );
		if ( !sphWriteThrottled ( fdTmpLookup.GetFD(), &dDocidLookup[0], nDocidLookup*sizeof(DocidRowidPair_t), "temp_docid_lookup", m_sLastError ) )
			return 0;

		nDocidLookupBlocks++;
	}

	dDocidLookup.Reset(0);

	m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments;
	m_tProgress.m_iBytes = m_tStats.m_iTotalBytes;
	m_tProgress.Show ( true );

	if ( m_tStats.m_iTotalDocuments )
	{
		tMinMax.FinishCollect();
		const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMax.GetCollected();
		tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLength()*sizeof(CSphRowitem) );

		m_iDocinfoIndex = ( dMinMaxRows.GetLength() / m_tSchema.GetRowSize() / 2 ) - 1;
	}

	tWriterSPA.CloseFile();
	if ( tWriterSPA.IsError() )
	{
		m_sLastError = "error writing .SPA";
		return false;
	}

	if ( pBlobRowBuilder.Ptr() && !pBlobRowBuilder->Done ( m_sLastError ) )
		return 0;

	if ( bGotPrevIndex )
		tPrevAttrs.Reset();

	if ( sph::rename ( sSPA.cstr(), GetIndexFileName(SPH_EXT_SPA).cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPA.cstr(), GetIndexFileName(SPH_EXT_SPA).cstr() );
		return false;
	}

	if ( bHaveBlobAttrs && sph::rename ( sSPB.cstr(), GetIndexFileName(SPH_EXT_SPB).cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPB.cstr(), GetIndexFileName(SPH_EXT_SPB).cstr() );
		return false;
	}

	if ( !WriteDeadRowMap ( GetIndexFileName(SPH_EXT_SPM), m_tStats.m_iTotalDocuments, m_sLastError ) )
		return 0;

	dKillList.Uniq();
	if ( !WriteKillList ( GetIndexFileName(SPH_EXT_SPK), dKillList.Begin(), dKillList.GetLength(), m_tSettings.m_dKlistTargets, m_sLastError ) )
		return 0;

	WarnAboutKillList ( dKillList, m_tSettings.m_dKlistTargets );

	m_iMinMaxIndex = m_tStats.m_iTotalDocuments*m_tSchema.GetRowSize();
	if ( !SortDocidLookup ( fdTmpLookup.GetFD(), nDocidLookupBlocks, iMemoryLimit, nDocidLookupsPerBlock, nDocidLookup ) )
		return 0;

	CreateHistograms ( tMinMax );

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	// initialize readers
	assert ( dBins.GetLength()==0 );
	dBins.Reserve ( dHitBlocks.GetLength() );

	iSharedOffset = -1;

	float fReadFactor = 1.0f;
	int iRelocationSize = 0;
	iWriteBuffer = iHitBuilderBufferSize;

	if ( m_bInplaceSettings )
	{
		assert ( m_fRelocFactor > 0.005f && m_fRelocFactor < 0.95f );
		assert ( m_fWriteFactor > 0.005f && m_fWriteFactor < 0.95f );
		assert ( m_fWriteFactor+m_fRelocFactor < 1.0f );

		fReadFactor -= m_fRelocFactor + m_fWriteFactor;

		iRelocationSize = int ( iMemoryLimit * m_fRelocFactor );
		iWriteBuffer = int ( iMemoryLimit * m_fWriteFactor );
	}

	int iBinSize = CSphBin::CalcBinSize ( int ( iMemoryLimit * fReadFactor ),
		dHitBlocks.GetLength() + m_pDict->GetSettings().m_bWordDict, "sort_hits" );

	CSphFixedVector <BYTE> dRelocationBuffer ( iRelocationSize );
	iSharedOffset = -1;

	ARRAY_FOREACH ( i, dHitBlocks )
	{
		dBins.Add ( new CSphBin ( m_tSettings.m_eHitless, m_pDict->GetSettings().m_bWordDict ) );
		dBins[i]->m_iFileLeft = dHitBlocks[i];
		dBins[i]->m_iFilePos = ( i==0 ) ? iHitsGap : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( fdHits.GetFD(), &iSharedOffset, iBinSize );
	}

	// if there were no hits, create zero-length index files
	int iRawBlocks = dBins.GetLength();

	//////////////////////////////
	// create new index files set
	//////////////////////////////

	tHitBuilder.CreateIndexFiles ( GetIndexFileName(SPH_EXT_SPD).cstr(), GetIndexFileName(SPH_EXT_SPP).cstr(), GetIndexFileName(SPH_EXT_SPE).cstr(), m_bInplaceSettings, iWriteBuffer, fdHits, &iSharedOffset );

	// dict files
	CSphAutofile fdTmpDict ( GetIndexFileName("tmp8"), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdDict ( GetIndexFileName(SPH_EXT_SPI), SPH_O_NEW, m_sLastError, false );
	if ( fdTmpDict.GetFD()<0 || fdDict.GetFD()<0 )
		return 0;

	m_pDict->DictBegin ( fdTmpDict, fdDict, iBinSize );

	//////////////
	// final sort
	//////////////

	if ( iRawBlocks )
	{
		int iLastBin = dBins.GetLength () - 1;
		SphOffset_t iHitFileSize = dBins[iLastBin]->m_iFilePos + dBins [iLastBin]->m_iFileLeft;

		CSphHitQueue tQueue ( iRawBlocks );
		CSphAggregateHit tHit;

		// initialize hitlist encoder state
		tHitBuilder.HitReset();

		// initial fill
		CSphFixedVector<BYTE> dActive ( iRawBlocks );
		for ( int i=0; i<iRawBlocks; i++ )
		{
			if ( !dBins[i]->ReadHit ( &tHit ) )
			{
				m_sLastError.SetSprintf ( "sort_hits: warmup failed (io error?)" );
				return 0;
			}
			dActive[i] = ( tHit.m_uWordID!=0 );
			if ( dActive[i] )
				tQueue.Push ( tHit, i );
		}

		// init progress meter
		m_tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT;
		m_tProgress.m_iHits = 0;

		// while the queue has data for us
		// FIXME! analyze binsRead return code
		int iHitsSorted = 0;
		int iMinBlock = -1;
		while ( tQueue.m_iUsed )
		{
			int iBin = tQueue.m_pData->m_iBin;

			// pack and emit queue root
			if ( m_bInplaceSettings )
			{
				if ( iMinBlock==-1 || dBins[iMinBlock]->IsEOF () || !dActive[iMinBlock] )
				{
					iMinBlock = -1;
					ARRAY_FOREACH ( i, dBins )
						if ( !dBins[i]->IsEOF () && dActive[i] && ( iMinBlock==-1 || dBins[i]->m_iFilePos < dBins[iMinBlock]->m_iFilePos ) )
							iMinBlock = i;
				}

				int iToWriteMax = 3*sizeof(DWORD);
				if ( iMinBlock!=-1 && ( tHitBuilder.GetHitfilePos() + iToWriteMax ) > dBins[iMinBlock]->m_iFilePos )
				{
					if ( !RelocateBlock ( fdHits.GetFD (), dRelocationBuffer.Begin(), iRelocationSize, &iHitFileSize, dBins[iMinBlock], &iSharedOffset ) )
						return 0;

					iMinBlock = (iMinBlock+1) % dBins.GetLength ();
				}
			}

			tHitBuilder.cidxHit ( tQueue.m_pData );
			if ( tHitBuilder.IsError() )
				return 0;

			// pop queue root and push next hit from popped bin
			tQueue.Pop ();
			if ( dActive[iBin] )
			{
				dBins[iBin]->ReadHit ( &tHit );
				dActive[iBin] = ( tHit.m_uWordID!=0 );
				if ( dActive[iBin] )
					tQueue.Push ( tHit, iBin );
			}

			// progress
			if ( ++iHitsSorted==1000000 )
			{
				m_tProgress.m_iHits += iHitsSorted;
				m_tProgress.Show ( false );
				iHitsSorted = 0;
			}
		}

		m_tProgress.m_iHits = m_tProgress.m_iHitsTotal; // sum might be less than total because of dupes!
		m_tProgress.Show ( true );

		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );

		dBins.Reset ();

		CSphAggregateHit tFlush;
		tFlush.m_tRowID = INVALID_ROWID;
		tFlush.m_uWordID = 0;
		tFlush.m_sKeyword = NULL;
		tFlush.m_iWordPos = EMPTY_HIT;
		tFlush.m_dFieldMask.UnsetAll();
		tHitBuilder.cidxHit ( &tFlush );

		if ( m_bInplaceSettings )
		{
			tHitBuilder.CloseHitlist();
			if ( !sphTruncate ( fdHits.GetFD () ) )
				sphWarn ( "failed to truncate %s", fdHits.GetFilename() );
		}
	}

	BuildHeader_t tBuildHeader ( m_tStats );
	if ( !tHitBuilder.cidxDone ( iMemoryLimit, m_tSettings.m_iMinInfixLen, m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return 0;

	dRelocationBuffer.Reset(0);

	tBuildHeader.m_iDocinfo = m_tStats.m_iTotalDocuments;
	tBuildHeader.m_iDocinfoIndex = m_iDocinfoIndex;
	tBuildHeader.m_iMinMaxIndex = m_iMinMaxIndex;

	CSphString sHeaderName = GetIndexFileName(SPH_EXT_SPH);
	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &m_tSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();

	// we're done
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, m_sLastError ) )
		return 0;

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	dFileWatchdog.AllIsDone();
	return 1;
} // NOLINT function length


/////////////////////////////////////////////////////////////////////////////
// MERGER HELPERS
/////////////////////////////////////////////////////////////////////////////

static const int DOCLIST_HINT_THRESH = 256;

// let uDocs be DWORD here to prevent int overflow in case of hitless word (highest bit is 1)
static int DoclistHintUnpack ( DWORD uDocs, BYTE uHint )
{
	if ( uDocs<(DWORD)DOCLIST_HINT_THRESH )
		return (int)Min ( 8*(int64_t)uDocs, INT_MAX );
	else
		return (int)Min ( 4*(int64_t)uDocs+( int64_t(uDocs)*uHint/64 ), INT_MAX );
}

BYTE sphDoclistHintPack ( SphOffset_t iDocs, SphOffset_t iLen )
{
	// we won't really store a hint for small lists
	if ( iDocs<DOCLIST_HINT_THRESH )
		return 0;

	// for bigger lists len/docs varies 4x-6x on test indexes
	// so lets assume that 4x-8x should be enough for everybody
	SphOffset_t iDelta = Min ( Max ( iLen-4*iDocs, 0 ), 4*iDocs-1 ); // len delta over 4x, clamped to [0x..4x) range
	BYTE uHint = (BYTE)( 64*iDelta/iDocs ); // hint now must be in [0..256) range
	while ( uHint<255 && ( iDocs*uHint/64 )<iDelta ) // roundoff (suddenly, my guru math skillz failed me)
		uHint++;

	return uHint;
}

// !COMMIT eliminate this, move to dict (or at least couple with CWordlist)
class CSphDictReader
{
public:
	// current word
	SphWordID_t		m_uWordID = 0;
	SphOffset_t		m_iDoclistOffset = 0;
	int				m_iDocs = 0;
	int				m_iHits = 0;
	bool			m_bHasHitlist = true;
	int				m_iHint = 0;

private:
	ESphHitless		m_eHitless { SPH_HITLESS_NONE };
	CSphAutoreader	m_tMyReader;
	CSphReader *	m_pReader = nullptr;
	SphOffset_t		m_iMaxPos = 0;
	int				m_iSkiplistBlockSize = 0;

	bool			m_bWordDict = true;
	char			m_sWord[MAX_KEYWORD_BYTES];

	int				m_iCheckpoint = 1;

public:
	CSphDictReader ( int iSkiplistBlockSize )
		: m_iSkiplistBlockSize ( iSkiplistBlockSize )
	{
		m_sWord[0] = '\0';
	}

	bool Setup ( const CSphString & sFilename, SphOffset_t iMaxPos, ESphHitless eHitless, CSphString & sError, bool bWordDict )
	{
		if ( !m_tMyReader.Open ( sFilename, sError ) )
			return false;

		Setup ( &m_tMyReader, iMaxPos, eHitless, bWordDict );
		return true;
	}

	void Setup ( CSphReader * pReader, SphOffset_t iMaxPos, ESphHitless eHitless, bool bWordDict )
	{
		m_pReader = pReader;
		m_pReader->SeekTo ( 1, READ_NO_SIZE_HINT );

		m_iMaxPos = iMaxPos;
		m_eHitless = eHitless;
		m_bWordDict = bWordDict;
		m_sWord[0] = '\0';
		m_iCheckpoint = 1;
	}

	bool Read()
	{
		assert ( m_iSkiplistBlockSize>0 );

		if ( m_pReader->GetPos()>=m_iMaxPos )
			return false;

		// get leading value
		SphWordID_t iWord0 = m_bWordDict ? m_pReader->GetByte() : m_pReader->UnzipWordid();
		if ( !iWord0 )
		{
			// handle checkpoint
			m_iCheckpoint++;
			m_pReader->UnzipOffset();

			m_uWordID = 0;
			m_iDoclistOffset = 0;
			m_sWord[0] = '\0';

			if ( m_pReader->GetPos()>=m_iMaxPos )
				return false;

			iWord0 = m_bWordDict ? m_pReader->GetByte() : m_pReader->UnzipWordid(); // get next word
		}
		if ( !iWord0 )
			return false; // some failure

		// get word entry
		if ( m_bWordDict )
		{
			// unpack next word
			// must be in sync with DictEnd()!
			assert ( iWord0<=255 );
			auto uPack = (BYTE) iWord0;

			int iMatch, iDelta;
			if ( uPack & 0x80 )
			{
				iDelta = ( ( uPack>>4 ) & 7 ) + 1;
				iMatch = uPack & 15;
			} else
			{
				iDelta = uPack & 127;
				iMatch = m_pReader->GetByte();
			}
			assert ( iMatch+iDelta<(int)sizeof(m_sWord)-1 );
			assert ( iMatch<=(int)strlen(m_sWord) );

			m_pReader->GetBytes ( m_sWord + iMatch, iDelta );
			m_sWord [ iMatch+iDelta ] = '\0';

			m_iDoclistOffset = m_pReader->UnzipOffset();
			m_iDocs = m_pReader->UnzipInt();
			m_iHits = m_pReader->UnzipInt();
			m_iHint = 0;
			if ( m_iDocs>=DOCLIST_HINT_THRESH )
				m_iHint = m_pReader->GetByte();
			if ( m_iDocs > m_iSkiplistBlockSize )
				m_pReader->UnzipInt();

			m_uWordID = (SphWordID_t) sphCRC32 ( GetWord() ); // set wordID for indexing

		} else
		{
			m_uWordID += iWord0;
			m_iDoclistOffset += m_pReader->UnzipOffset();
			m_iDocs = m_pReader->UnzipInt();
			m_iHits = m_pReader->UnzipInt();
			if ( m_iDocs > m_iSkiplistBlockSize )
				m_pReader->UnzipOffset();
		}

		m_bHasHitlist =
			( m_eHitless==SPH_HITLESS_NONE ) ||
			( m_eHitless==SPH_HITLESS_SOME && !( m_iDocs & HITLESS_DOC_FLAG ) );
		m_iDocs = m_eHitless==SPH_HITLESS_SOME ? ( m_iDocs & HITLESS_DOC_MASK ) : m_iDocs;

		return true; // FIXME? errorflag?
	}

	int CmpWord ( const CSphDictReader & tOther ) const
	{
		if ( m_bWordDict )
			return strcmp ( m_sWord, tOther.m_sWord );

		int iRes = 0;
		iRes = m_uWordID<tOther.m_uWordID ? -1 : iRes;
		iRes = m_uWordID>tOther.m_uWordID ? 1 : iRes;
		return iRes;
	}

	BYTE * GetWord() const { return (BYTE *)m_sWord; }

	int GetCheckpoint() const { return m_iCheckpoint; }
};

static ISphFilter * CreateMergeFilters ( const CSphVector<CSphFilterSettings> & dSettings, const CSphSchema & tSchema, const BYTE * pBlobPool )
{
	CSphString sError, sWarning;
	ISphFilter * pResult = nullptr;
	CreateFilterContext_t tCtx;
	tCtx.m_pSchema = &tSchema;
	tCtx.m_pBlobPool = pBlobPool;

	ARRAY_FOREACH ( i, dSettings )
	{
		ISphFilter * pFilter = sphCreateFilter ( dSettings[i], tCtx, sError, sWarning );
		if ( pFilter )
			pResult = sphJoinFilters ( pResult, pFilter );
	}
	return pResult;
}

static bool CheckDocsCount ( int64_t iDocs, CSphString & sError )
{
	if ( iDocs<INT_MAX )
		return true;

	sError.SetSprintf ( "index over %d documents not supported (got " INT64_FMT " documents)", INT_MAX, iDocs );
	return false;
}


class CSphMerger
{
public:
	explicit CSphMerger ( CSphHitBuilder * pHitBuilder )
		: m_pHitBuilder ( pHitBuilder )
	{}

	template < typename QWORD >
	static inline void PrepareQword ( QWORD & tQword, const CSphDictReader & tReader, bool bWordDict ) //NOLINT
	{
		tQword.m_tDoc.m_tRowID = INVALID_ROWID;

		tQword.m_iDocs = tReader.m_iDocs;
		tQword.m_iHits = tReader.m_iHits;
		tQword.m_bHasHitlist = tReader.m_bHasHitlist;

		tQword.m_uHitPosition = 0;
		tQword.m_iHitlistPos = 0;

		if ( bWordDict )
			tQword.m_rdDoclist->SeekTo ( tReader.m_iDoclistOffset, tReader.m_iHint );
	}

	template < typename QWORD >
	inline bool NextDocument ( QWORD & tQword, const CSphIndex_VLN * pSourceIndex, const ISphFilter * pFilter, const CSphVector<RowID_t> & dRows )
	{
		while (true)
		{
			tQword.GetNextDoc();
			if ( tQword.m_tDoc.m_tRowID==INVALID_ROWID )
				return false;

			tQword.SeekHitlist ( tQword.m_iHitlistPos );

			if ( dRows[tQword.m_tDoc.m_tRowID]==INVALID_ROWID )
			{
				while ( tQword.m_bHasHitlist && tQword.GetNextHit()!=EMPTY_HIT );
				continue;
			}

			if ( pFilter )
			{
				CSphMatch tMatch;
				tMatch.m_tRowID = tQword.m_tDoc.m_tRowID;
				tMatch.m_pStatic = pSourceIndex->GetDocinfoByRowID ( tQword.m_tDoc.m_tRowID );

				if ( !pFilter->Eval ( tMatch ) )
				{
					while ( tQword.m_bHasHitlist && tQword.GetNextHit()!=EMPTY_HIT );
					continue;
				}
			}

			return true;
		}
	}

	template < typename QWORD >
	inline void TransferData ( QWORD & tQword, SphWordID_t iWordID, const BYTE * sWord,
							const CSphIndex_VLN * pSourceIndex, const ISphFilter * pFilter,
							const CSphVector<RowID_t> & dRows, volatile bool * pLocalStop )
	{
		CSphAggregateHit tHit;
		tHit.m_uWordID = iWordID;
		tHit.m_sKeyword = sWord;
		tHit.m_dFieldMask.UnsetAll();

		while ( CSphMerger::NextDocument ( tQword, pSourceIndex, pFilter, dRows ) && !g_bShutdown  && !*pLocalStop )
		{
			if ( tQword.m_bHasHitlist )
				TransferHits ( tQword, tHit, dRows );
			else
			{
				// convert to aggregate if there is no hit-list
				tHit.m_tRowID = dRows[tQword.m_tDoc.m_tRowID];
				tHit.m_dFieldMask = tQword.m_dQwordFields;
				tHit.SetAggrCount ( tQword.m_uMatchHits );
				m_pHitBuilder->cidxHit ( &tHit );
			}
		}
	}

	template < typename QWORD >
	inline void TransferHits ( QWORD & tQword, CSphAggregateHit & tHit, const CSphVector<RowID_t> & dRows )
	{
		assert ( tQword.m_bHasHitlist );
		tHit.m_tRowID = dRows[tQword.m_tDoc.m_tRowID];
		for ( Hitpos_t uHit = tQword.GetNextHit(); uHit!=EMPTY_HIT; uHit = tQword.GetNextHit() )
		{
			tHit.m_iWordPos = uHit;
			m_pHitBuilder->cidxHit ( &tHit );
		}
	}

	template < typename QWORD >
	static inline void ConfigureQword ( QWORD & tQword, DataReaderFabric_c * pHits, DataReaderFabric_c * pDocs, int iDynamic )
	{
		tQword.SetHitReader ( pHits );
		tQword.m_rdHitlist->SeekTo ( 1, READ_NO_SIZE_HINT );

		tQword.SetDocReader ( pDocs );
		tQword.m_rdDoclist->SeekTo ( 1, READ_NO_SIZE_HINT );

		tQword.m_tDoc.Reset ( iDynamic );
	}

private:
	CSphHitBuilder *	m_pHitBuilder;
};


template < typename QWORDDST, typename QWORDSRC >
bool CSphIndex_VLN::MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, const CSphVector<RowID_t> & dDstRows, const CSphVector<RowID_t> & dSrcRows,
	CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat,	CSphIndexProgress & tProgress, volatile bool * pLocalStop )
{
	CSphAutofile tDummy;
	pHitBuilder->CreateIndexFiles ( pDstIndex->GetIndexFileName("tmp.spd").cstr(), pDstIndex->GetIndexFileName("tmp.spp").cstr(), pDstIndex->GetIndexFileName("tmp.spe").cstr(), false, 0, tDummy, NULL );

	CSphDictReader tDstReader ( pDstIndex->GetSettings().m_iSkiplistBlockSize );
	CSphDictReader tSrcReader ( pSrcIndex->GetSettings().m_iSkiplistBlockSize );

	bool bWordDict = pHitBuilder->IsWordDict();

	if ( !tDstReader.Setup ( pDstIndex->GetIndexFileName(SPH_EXT_SPI), pDstIndex->m_tWordlist.m_iWordsEnd, pDstIndex->m_tSettings.m_eHitless, sError, bWordDict ) )
		return false;
	if ( !tSrcReader.Setup ( pSrcIndex->GetIndexFileName(SPH_EXT_SPI), pSrcIndex->m_tWordlist.m_iWordsEnd, pSrcIndex->m_tSettings.m_eHitless, sError, bWordDict ) )
		return false;

	/// prepare for indexing
	pHitBuilder->HitblockBegin();
	pHitBuilder->HitReset();

	/// setup qwords

	QWORDDST tDstQword ( false, false );
	QWORDSRC tSrcQword ( false, false );

	DataReaderFabricPtr_t tSrcDocs {
		NewProxyReader ( pSrcIndex->GetIndexFileName ( SPH_EXT_SPD ), sError,
			DataReaderFabric_c::eDocs, pSrcIndex->m_tFiles.m_iReadBufferDocList, true )
	};
	if ( !tSrcDocs )
		return false;

	DataReaderFabricPtr_t tSrcHits {
		NewProxyReader ( pSrcIndex->GetIndexFileName ( SPH_EXT_SPP ), sError,
			DataReaderFabric_c::eHits, pSrcIndex->m_tFiles.m_iReadBufferHitList, true  )
	};
	if ( !tSrcHits )
		return false;

	if ( !sError.IsEmpty () || g_bShutdown || *pLocalStop )
		return false;

	DataReaderFabricPtr_t tDstDocs {
		NewProxyReader ( pDstIndex->GetIndexFileName ( SPH_EXT_SPD ), sError,
			DataReaderFabric_c::eDocs, pDstIndex->m_tFiles.m_iReadBufferDocList, true )
	};
	if ( !tDstDocs )
		return false;

	DataReaderFabricPtr_t tDstHits {
		NewProxyReader ( pDstIndex->GetIndexFileName ( SPH_EXT_SPP ), sError,
			DataReaderFabric_c::eHits, pDstIndex->m_tFiles.m_iReadBufferHitList, true )
	};
	if ( !tDstHits )
		return false;

	if ( !sError.IsEmpty() || g_bShutdown || *pLocalStop )
		return false;

	CSphMerger tMerger(pHitBuilder);

	CSphMerger::ConfigureQword<QWORDDST> ( tDstQword, tDstHits, tDstDocs, pDstIndex->m_tSchema.GetDynamicSize() );
	CSphMerger::ConfigureQword<QWORDSRC> ( tSrcQword, tSrcHits, tSrcDocs, pSrcIndex->m_tSchema.GetDynamicSize() );

	/// merge

	bool bDstWord = tDstReader.Read();
	bool bSrcWord = tSrcReader.Read();

	tProgress.m_ePhase = CSphIndexProgress::PHASE_MERGE;
	tProgress.Show ( false );

	int iWords = 0;
	int iHitlistsDiscarded = 0;
	for ( ; bDstWord || bSrcWord; iWords++ )
	{
		if ( iWords==1000 )
		{
			tProgress.m_iWords += 1000;
			tProgress.Show ( false );
			iWords = 0;
		}

		if ( g_bShutdown || *pLocalStop )
			return false;

		const int iCmp = tDstReader.CmpWord ( tSrcReader );

		if ( !bSrcWord || ( bDstWord && iCmp<0 ) )
		{
			// transfer documents and hits from destination
			CSphMerger::PrepareQword<QWORDDST> ( tDstQword, tDstReader, bWordDict );
			tMerger.TransferData<QWORDDST> ( tDstQword, tDstReader.m_uWordID, tDstReader.GetWord(), pDstIndex, pFilter, dDstRows, pLocalStop );
			bDstWord = tDstReader.Read();

		} else if ( !bDstWord || ( bSrcWord && iCmp>0 ) )
		{
			// transfer documents and hits from source
			CSphMerger::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader, bWordDict );
			tMerger.TransferData<QWORDSRC> ( tSrcQword, tSrcReader.m_uWordID, tSrcReader.GetWord(), pSrcIndex, NULL, dSrcRows, pLocalStop );
			bSrcWord = tSrcReader.Read();

		} else // merge documents and hits inside the word
		{
			assert ( iCmp==0 );

			bool bHitless = !tDstReader.m_bHasHitlist;
			if ( tDstReader.m_bHasHitlist!=tSrcReader.m_bHasHitlist )
			{
				iHitlistsDiscarded++;
				bHitless = true;
			}

			CSphMerger::PrepareQword<QWORDDST> ( tDstQword, tDstReader, bWordDict );
			CSphMerger::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader, bWordDict );

			CSphAggregateHit tHit;
			tHit.m_uWordID = tDstReader.m_uWordID; // !COMMIT m_sKeyword anyone?
			tHit.m_sKeyword = tDstReader.GetWord();
			tHit.m_dFieldMask.UnsetAll();

			// we assume that all the duplicates have been removed
			// and we don't need to merge hits from the same document

			// transfer hits from destination
			while ( tMerger.NextDocument ( tDstQword, pDstIndex, pFilter, dDstRows ) )
			{
				if ( g_bShutdown || *pLocalStop )
					return false;

				if ( bHitless )
				{
					while ( tDstQword.m_bHasHitlist && tDstQword.GetNextHit()!=EMPTY_HIT );

					tHit.m_tRowID = dDstRows[tDstQword.m_tDoc.m_tRowID];
					tHit.m_dFieldMask = tDstQword.m_dQwordFields;
					tHit.SetAggrCount ( tDstQword.m_uMatchHits );
					pHitBuilder->cidxHit ( &tHit );
				} else
					tMerger.TransferHits ( tDstQword, tHit, dDstRows );
			}

			// transfer hits from source
			while ( tMerger.NextDocument ( tSrcQword, pSrcIndex, NULL, dSrcRows ) )
			{
				if ( g_bShutdown || *pLocalStop )
					return false;

				if ( bHitless )
				{
					while ( tSrcQword.m_bHasHitlist && tSrcQword.GetNextHit()!=EMPTY_HIT );

					tHit.m_tRowID = dSrcRows[tSrcQword.m_tDoc.m_tRowID];
					tHit.m_dFieldMask = tSrcQword.m_dQwordFields;
					tHit.SetAggrCount ( tSrcQword.m_uMatchHits );
					pHitBuilder->cidxHit ( &tHit );
				} else
					tMerger.TransferHits ( tSrcQword, tHit, dSrcRows );
			}

			// next word
			bDstWord = tDstReader.Read();
			bSrcWord = tSrcReader.Read();
		}
	}

	tStat.m_iTotalDocuments += pSrcIndex->m_tStats.m_iTotalDocuments;
	tStat.m_iTotalBytes += pSrcIndex->m_tStats.m_iTotalBytes;

	tProgress.m_iWords += iWords;
	tProgress.Show ( false );

	if ( iHitlistsDiscarded )
		sphWarning ( "discarded hitlists for %u words", iHitlistsDiscarded );

	return true;
}


bool CSphIndex_VLN::Merge ( CSphIndex * pSource, const CSphVector<CSphFilterSettings> & dFilters, bool bSupressDstDocids )
{
	SetMemorySettings ( FileAccessSettings_t() );
	if ( !Prealloc ( false ) )
		return false;
	Preread ();
	pSource->SetMemorySettings ( FileAccessSettings_t() );
	if ( !pSource->Prealloc ( false ) )
	{
		m_sLastError.SetSprintf ( "source index preload failed: %s", pSource->GetLastError().cstr() );
		return false;
	}
	pSource->Preread();

	// create filters
	CSphScopedPtr<ISphFilter> pFilter ( CreateMergeFilters ( dFilters, m_tSchema, m_tBlobAttrs.GetWritePtr() ) );

	bool bLocalStop = false;
	return CSphIndex_VLN::DoMerge ( this, (const CSphIndex_VLN *)pSource, pFilter.Ptr(), m_sLastError, m_tProgress, &bLocalStop, false, bSupressDstDocids );
}


void CSphIndex_VLN::CreateRowMaps ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, CSphVector<RowID_t> & dSrcRowMap, CSphVector<RowID_t> & dDstRowMap, bool bSupressDstDocids )
{
	int iStride = pDstIndex->m_tSchema.GetRowSize();

	DeadRowMap_Ram_c tExtraDeadMap(0);
	if ( bSupressDstDocids )
	{
		tExtraDeadMap.Reset ( dDstRowMap.GetLength() );

		LookupReader_c tDstLookupReader ( pDstIndex->m_tDocidLookup.GetWritePtr() );
		LookupReader_c tSrcLookupReader ( pSrcIndex->m_tDocidLookup.GetWritePtr() );

		KillByLookup ( tDstLookupReader, tSrcLookupReader, tExtraDeadMap );
	}

	dSrcRowMap.Fill ( INVALID_ROWID );
	dDstRowMap.Fill ( INVALID_ROWID );

	const DWORD * pRow = pDstIndex->m_tAttr.GetWritePtr();
	RowID_t tRowID = 0;

	for ( int i = 0; i < dDstRowMap.GetLength(); i++, pRow+=iStride )
	{
		if ( pDstIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		if ( bSupressDstDocids && tExtraDeadMap.IsSet(i) )
			continue;

		if ( pFilter )
		{
			CSphMatch tFakeMatch;
			tFakeMatch.m_pStatic = pRow;
			if ( !pFilter->Eval ( tFakeMatch ) )
				continue;
		}

		dDstRowMap[i] = tRowID++;
	}

	for ( int i = 0; i < dSrcRowMap.GetLength(); i++ )
	{
		if ( pSrcIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		dSrcRowMap[i] = tRowID++;
	}
}


bool CSphIndex_VLN::MergeAttributes ( volatile bool * pLocalStop, const CSphIndex_VLN * pIndex, const CSphVector<RowID_t> & dRowMap, AttrIndexBuilder_c & tMinMax,
	CSphWriter & tWriterSPA, BlobRowBuilder_i * pBlobRowBuilder, RowID_t & tResultRowID )
{
	int iStride = pIndex->m_tSchema.GetRowSize();

	CSphFixedVector<CSphRowitem> dTmpRow ( iStride );
	auto iStrideBytes = dTmpRow.GetLengthBytes ();
	const CSphRowitem * pRow = pIndex->m_tAttr.GetWritePtr();
	const CSphColumnInfo * pBlobLocator = pIndex->m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	for ( int i = 0; i < dRowMap.GetLength(); i++, pRow += iStride )
	{
		if ( g_bShutdown || *pLocalStop )
			return false;

		if ( dRowMap[i]==INVALID_ROWID )
			continue;

		if ( tResultRowID==INVALID_ROWID )
			return false;

		tMinMax.Collect ( pRow );

		if ( pBlobRowBuilder )
		{
			const BYTE * pOldBlobRow = pIndex->m_tBlobAttrs.GetWritePtr() + sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );
			uint64_t uNewOffset = pBlobRowBuilder->Flush ( pOldBlobRow );

			memcpy ( dTmpRow.Begin(), pRow, iStrideBytes);
			sphSetRowAttr ( dTmpRow.Begin(), pBlobLocator->m_tLocator, uNewOffset );

			tWriterSPA.PutBytes ( dTmpRow.Begin(), iStrideBytes );
		} else
			tWriterSPA.PutBytes ( pRow, iStrideBytes );

		tResultRowID++;
	}

	return true;
}


bool CSphIndex_VLN::DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter,
	CSphString & sError, CSphIndexProgress & tProgress, volatile bool * pLocalStop, bool bSrcSettings, bool bSupressDstDocids )
{
	assert ( pDstIndex && pSrcIndex );

	const CSphSchema & tDstSchema = pDstIndex->m_tSchema;
	const CSphSchema & tSrcSchema = pSrcIndex->m_tSchema;
	if ( !tDstSchema.CompareTo ( tSrcSchema, sError ) )
		return false;

	if ( pDstIndex->m_tSettings.m_eHitless!=pSrcIndex->m_tSettings.m_eHitless )
	{
		sError = "hitless settings must be the same on merged indices";
		return false;
	}

	if ( pDstIndex->m_pDict->GetSettings().m_bWordDict!=pSrcIndex->m_pDict->GetSettings().m_bWordDict )
	{
		sError.SetSprintf ( "dictionary types must be the same (dst dict=%s, src dict=%s )",
			pDstIndex->m_pDict->GetSettings().m_bWordDict ? "keywords" : "crc",
			pSrcIndex->m_pDict->GetSettings().m_bWordDict ? "keywords" : "crc" );
		return false;
	}

	BuildHeader_t tBuildHeader ( pDstIndex->m_tStats );
	const CSphIndex_VLN * pSettings = ( bSrcSettings ? pSrcIndex : pDstIndex );

	/////////////////////////////////////////
	// merging attributes (.spa, .spb)
	/////////////////////////////////////////
	CSphString sSPA = pDstIndex->GetIndexFileName ( SPH_EXT_SPA, true );
	CSphString sSPT = pDstIndex->GetIndexFileName ( SPH_EXT_SPT, true );
	CSphString sSPHI = pDstIndex->GetIndexFileName ( SPH_EXT_SPHI, true );
	CSphWriter tWriterSPA;
	if ( !tWriterSPA.OpenFile ( sSPA, sError ) )
		return false;

	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder(nullptr);
	if ( pSrcIndex->m_tSchema.HasBlobAttrs() )
	{
		pBlobRowBuilder = sphCreateBlobRowBuilder ( pSrcIndex->m_tSchema, pDstIndex->GetIndexFileName ( SPH_EXT_SPB, true ), pSrcIndex->GetSettings().m_tBlobUpdateSpace, sError );
		if ( !pBlobRowBuilder.Ptr() )
			return false;
	}

	AttrIndexBuilder_c tMinMax ( pDstIndex->m_tSchema );

	CSphVector<RowID_t> dSrcRows(pSrcIndex->m_iDocinfo);
	CSphVector<RowID_t> dDstRows(pDstIndex->m_iDocinfo);

	CreateRowMaps ( pDstIndex, pSrcIndex, pFilter, dSrcRows, dDstRows, bSupressDstDocids );

	RowID_t tResultRowID = 0;
	if ( !MergeAttributes ( pLocalStop, pDstIndex, dDstRows, tMinMax, tWriterSPA, pBlobRowBuilder.Ptr(), tResultRowID ) )
		return false;

	if ( !MergeAttributes ( pLocalStop, pSrcIndex, dSrcRows, tMinMax, tWriterSPA, pBlobRowBuilder.Ptr(), tResultRowID ) )
		return false;

	if ( tResultRowID )
	{
		tBuildHeader.m_iMinMaxIndex = tWriterSPA.GetPos() / sizeof(CSphRowitem);

		tMinMax.FinishCollect();
		const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMax.GetCollected();
		tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLength()*sizeof(CSphRowitem) );

		tBuildHeader.m_iDocinfo = tResultRowID;
		tBuildHeader.m_iDocinfoIndex = ( dMinMaxRows.GetLength() / tDstSchema.GetRowSize() / 2 ) - 1;
	}

	tWriterSPA.CloseFile();
	if ( tWriterSPA.IsError() )
		return false;

	if ( pBlobRowBuilder.Ptr() && !pBlobRowBuilder->Done ( sError ) )
		return false;

	if ( !CheckDocsCount ( tResultRowID, sError ) )
		return false;

	CSphAutofile tTmpDict ( pDstIndex->GetIndexFileName("spi.tmp"), SPH_O_NEW, sError, true );
	CSphAutofile tDict ( pDstIndex->GetIndexFileName ( SPH_EXT_SPI, true ), SPH_O_NEW, sError );

	if ( !sError.IsEmpty() || tTmpDict.GetFD()<0 || tDict.GetFD()<0 || g_bShutdown || *pLocalStop )
		return false;

	CSphDictRefPtr_c pDict { pSettings->m_pDict->Clone() };

	int iHitBufferSize = 8 * 1024 * 1024;
	CSphVector<SphWordID_t> dDummy;
	CSphHitBuilder tHitBuilder ( pSettings->m_tSettings, dDummy, true, iHitBufferSize, pDict, &sError );

	// FIXME? is this magic dict block constant any good?..
	pDict->DictBegin ( tTmpDict, tDict, iHitBufferSize );

	// merge dictionaries, doclists and hitlists
	if ( pDict->GetSettings().m_bWordDict )
	{
		WITH_QWORD ( pDstIndex, false, QwordDst,
			WITH_QWORD ( pSrcIndex, false, QwordSrc,
		{
			if ( !CSphIndex_VLN::MergeWords < QwordDst, QwordSrc > ( pDstIndex, pSrcIndex, pFilter, dDstRows, dSrcRows, &tHitBuilder, sError, tBuildHeader, tProgress, pLocalStop ) )
				return false;
		} ) );
	} else
	{
		WITH_QWORD ( pDstIndex, true, QwordDst,
			WITH_QWORD ( pSrcIndex, true, QwordSrc,
		{
			if ( !CSphIndex_VLN::MergeWords < QwordDst, QwordSrc > ( pDstIndex, pSrcIndex, pFilter, dDstRows, dSrcRows, &tHitBuilder, sError, tBuildHeader, tProgress, pLocalStop ) )
				return false;
		} ) );
	}

	tBuildHeader.m_iTotalDocuments = tResultRowID;

	if ( !WriteDeadRowMap ( pDstIndex->GetIndexFileName ( SPH_EXT_SPM, true ), tResultRowID, sError ) )
		return false;

	if ( g_bShutdown || *pLocalStop )
		return false;

	// finalize
	CSphAggregateHit tFlush;
	tFlush.m_tRowID = INVALID_ROWID;
	tFlush.m_uWordID = 0;
	tFlush.m_sKeyword = (BYTE*)""; // tricky: assertion in cidxHit calls strcmp on this in case of empty index!
	tFlush.m_iWordPos = EMPTY_HIT;
	tFlush.m_dFieldMask.UnsetAll();
	tHitBuilder.cidxHit ( &tFlush );

	int iMinInfixLen = pSettings->m_tSettings.m_iMinInfixLen;
	if ( !tHitBuilder.cidxDone ( iHitBufferSize, iMinInfixLen, pSettings->m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return false;

	if ( !pDstIndex->WriteLookupAndHistograms ( sSPA.cstr(), sSPT.cstr(), sSPHI.cstr(), tResultRowID, tMinMax, sError ) )
		return false;

	CSphString sHeaderName = pDstIndex->GetIndexFileName ( SPH_EXT_SPH, true );

	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &pSettings->m_tSettings;
	tWriteHeader.m_pSchema = &pSettings->m_tSchema;
	tWriteHeader.m_pTokenizer = pSettings->m_pTokenizer;
	tWriteHeader.m_pDict = pSettings->m_pDict;
	tWriteHeader.m_pFieldFilter = pSettings->m_pFieldFilter;
	tWriteHeader.m_pFieldLens = pSettings->m_dFieldLens.Begin();

	IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, sError ); // FIXME? is this magic dict block constant any good?..

	return true;
}


bool sphMerge ( const CSphIndex * pDst, const CSphIndex * pSrc,	CSphString & sError, CSphIndexProgress & tProgress, volatile bool * pLocalStop, bool bSrcSettings )
{
	auto pDstIndex = ( const CSphIndex_VLN * ) pDst;
	auto pSrcIndex = ( const CSphIndex_VLN * ) pSrc;

	return CSphIndex_VLN::DoMerge ( pDstIndex, pSrcIndex, nullptr, sError, tProgress, pLocalStop, bSrcSettings, false );
}


/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

SphWordID_t CSphDictTraits::GetWordID ( BYTE * )
{
	assert ( 0 && "not implemented" );
	return 0;
}


SphWordID_t CSphDictStar::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];
	assert ( strlen ( (const char*)pWord ) < 16+3*SPH_MAX_WORD_LEN );

	if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && m_pDict->IsStopWord ( pWord ) )
		return 0;

	m_pDict->ApplyStemmers ( pWord );

	int iLen = strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 1 );
	// stemmer might squeeze out the word
	if ( iLen && !pWord[0] )
		return 0;

	memcpy ( sBuf, pWord, iLen+1 );

	if ( iLen )
	{
		if ( sBuf[iLen-1]=='*' )
		{
			iLen--;
			sBuf[iLen] = '\0';
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL;
			iLen++;
			sBuf[iLen] = '\0';
		}
	}

	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !m_pDict->GetSettings().m_bStopwordsUnstemmed );
}


SphWordID_t	CSphDictStar::GetWordIDNonStemmed ( BYTE * pWord )
{
	return m_pDict->GetWordIDNonStemmed ( pWord );
}


//////////////////////////////////////////////////////////////////////////

SphWordID_t	CSphDictStarV8::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iLen = strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	bool bHeadStar = ( pWord[0]=='*' );
	bool bTailStar = ( pWord[iLen-1]=='*' ) && ( iLen>1 );
	bool bMagic = ( pWord[0]<' ' );

	if ( !bHeadStar && !bTailStar && !bMagic )
	{
		if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;

		m_pDict->ApplyStemmers ( pWord );

		// stemmer might squeeze out the word
		if ( !pWord[0] )
			return 0;

		if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;
	}

	iLen = strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 2 );

	if ( !iLen || ( bHeadStar && iLen==1 ) )
		return 0;

	if ( bMagic ) // pass throu MAGIC_* words
	{
		memcpy ( sBuf, pWord, iLen );
		sBuf[iLen] = '\0';

	} else if ( m_bInfixes )
	{
		////////////////////////////////////
		// infix or mixed infix+prefix mode
		////////////////////////////////////

		// handle head star
		if ( bHeadStar )
		{
			memcpy ( sBuf, pWord+1, iLen-- ); // chops star, copies trailing zero, updates iLen
		} else
		{
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf+1, pWord, ++iLen ); // copies everything incl trailing zero, updates iLen
		}

		// handle tail star
		if ( bTailStar )
		{
			sBuf[--iLen] = '\0'; // got star, just chop it away
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL; // no star, add tail marker
			sBuf[++iLen] = '\0';
		}

	} else
	{
		////////////////////
		// prefix-only mode
		////////////////////

		// always ignore head star in prefix mode
		if ( bHeadStar )
		{
			pWord++;
			iLen--;
		}

		// handle tail star
		if ( !bTailStar )
		{
			// exact word search request, always (ie. both in infix/prefix mode) mangles to "\1word\1" in v.8+
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf+1, pWord, iLen );
			sBuf[iLen+1] = MAGIC_WORD_TAIL;
			sBuf[iLen+2] = '\0';
			iLen += 2;

		} else
		{
			// prefix search request, mangles to word itself (just chop away the star)
			memcpy ( sBuf, pWord, iLen );
			sBuf[--iLen] = '\0';
		}
	}

	// calc id for mangled word
	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !bHeadStar && !bTailStar );
}

//////////////////////////////////////////////////////////////////////////

SphWordID_t CSphDictExact::GetWordID ( BYTE * pWord )
{
	int iLen = strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	if ( pWord[0]=='=' )
		pWord[0] = MAGIC_WORD_HEAD_NONSTEMMED;

	if ( pWord[0]<' ' )
		return m_pDict->GetWordIDNonStemmed ( pWord );

	return m_pDict->GetWordID ( pWord );
}


/////////////////////////////////////////////////////////////////////////////

void CSphMatchComparatorState::FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, bool bRemapKeyparts )
{
	for ( int i = 0; i < CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( m_eKeypart[i]==SPH_KEYPART_DOCID_S || m_eKeypart[i]==SPH_KEYPART_DOCID_D )
		{
			const CSphColumnInfo * pAttrInNewSchema = pNewSchema->GetAttr ( sphGetDocidName() );
			assert ( pAttrInNewSchema && pAttrInNewSchema->m_tLocator.m_iBitOffset==0 );
			m_eKeypart[i] = pAttrInNewSchema->m_tLocator.m_bDynamic ? SPH_KEYPART_DOCID_D : SPH_KEYPART_DOCID_S;
		}
		else
			sphFixupLocator ( m_tLocator[i], pOldSchema, pNewSchema );

		// update string keypart into str_ptr
		if ( bRemapKeyparts && m_eKeypart[i]==SPH_KEYPART_STRING )
			m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
	}
}


//////////////////////////////////////////////////////////////////////////

inline bool sphGroupMatch ( SphAttr_t iGroup, const SphAttr_t * pGroups, int iGroups )
{
	if ( !pGroups ) return true;
	const SphAttr_t * pA = pGroups;
	const SphAttr_t * pB = pGroups+iGroups-1;
	if ( iGroup==*pA || iGroup==*pB ) return true;
	if ( iGroup<(*pA) || iGroup>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		const SphAttr_t * pM = pA + ((pB-pA)/2);
		if ( iGroup==(*pM) )
			return true;
		if ( iGroup<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}


bool CSphIndex_VLN::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	tMatch.m_pStatic = GetDocinfoByRowID ( tMatch.m_tRowID );
	pCtx->CalcFilter ( tMatch );
	if ( !pCtx->m_pFilter )
		return false;

	if ( !pCtx->m_pFilter->Eval ( tMatch ) )
	{
		pCtx->FreeDataFilter ( tMatch );
		return true;
	}

	return false;
}


bool CSphIndex_VLN::BuildDocList ( SphAttr_t ** ppDocList, int64_t * pCount, CSphString * pError ) const
{
	assert ( ppDocList && pCount && pError );
	*ppDocList = nullptr;
	*pCount = 0;
	if ( !m_iDocinfo )
		return true;

	// new[] might fail on 32bit here
	int64_t iSizeMax = (size_t)m_iDocinfo;
	if ( iSizeMax!=m_iDocinfo )
	{
		pError->SetSprintf ( "doc-list build size_t overflow (docs count=" INT64_FMT ", size max=" INT64_FMT ")", m_iDocinfo, iSizeMax );
		return false;
	}

	int iStride = m_tSchema.GetRowSize();
	auto * pDst = new SphAttr_t [(size_t)m_iDocinfo];
	*ppDocList = pDst;
	*pCount = m_iDocinfo;

	const CSphRowitem * pRow = m_tAttr.GetWritePtr();
	const CSphRowitem * pEnd = m_tAttr.GetWritePtr() + m_iDocinfo*iStride;
	while ( pRow<pEnd )
	{
		*pDst++ = sphGetDocID ( pRow );
		pRow += iStride;
	}

	return true;
}


RowID_t CSphIndex_VLN::GetRowidByDocid ( DocID_t tDocID ) const
{
	return m_tLookupReader.Find ( tDocID );
}


int	CSphIndex_VLN::Kill ( DocID_t tDocID )
{
	// FIXME! docid might not be unique
	if ( m_tDeadRowMap.Set ( GetRowidByDocid ( tDocID ) ) )
	{
		m_uAttrsStatus |= IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED;
		return 1;
	}

	return 0;
}


inline const CSphRowitem * CSphIndex_VLN::FindDocinfo ( DocID_t tDocID ) const
{
	RowID_t tRowID = GetRowidByDocid ( tDocID );
	return tRowID==INVALID_ROWID ? nullptr : GetDocinfoByRowID ( tRowID );
}


inline const CSphRowitem * CSphIndex_VLN::GetDocinfoByRowID ( RowID_t tRowID ) const
{
	//  GetCachedRowSize() is used to avoid several virtual calls
	return m_tAttr.GetWritePtr() + (int64_t)tRowID*m_tSchema.GetCachedRowSize();
}


inline RowID_t CSphIndex_VLN::GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const
{
	return ( pDocinfo - m_tAttr.GetWritePtr() )/m_tSchema.GetCachedRowSize();
}


static inline void CalcContextItems ( CSphMatch & tMatch, const CSphVector<CSphQueryContext::CalcItem_t> & dItems )
{
	for ( const CSphQueryContext::CalcItem_t & tCalc : dItems )
	{
		switch ( tCalc.m_eType )
		{
			case SPH_ATTR_INTEGER:
				tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->IntEval(tMatch) );
				break;

			case SPH_ATTR_BIGINT:
			case SPH_ATTR_JSON_FIELD:
				tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->Int64Eval(tMatch) );
				break;

			case SPH_ATTR_STRINGPTR:
				tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->StringEvalPacked ( tMatch ) );
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->FactorEvalPacked ( tMatch ) ); // FIXME! a potential leak of *previous* value?
				break;

			case SPH_ATTR_INT64SET_PTR:
			case SPH_ATTR_UINT32SET_PTR:
				tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->Int64Eval ( tMatch ) );
				break;

			default:
				tMatch.SetAttrFloat ( tCalc.m_tLoc, tCalc.m_pExpr->Eval(tMatch) );
				break;
		}
	}
}

void CSphQueryContext::CalcFilter ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcFilter );
}


void CSphQueryContext::CalcSort ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcSort );
}


void CSphQueryContext::CalcFinal ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcFinal );
}

static inline void FreeDataPtrAttrs ( CSphMatch & tMatch, const CSphVector<CSphQueryContext::CalcItem_t> & dItems )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( const auto & i : dItems )
		if ( sphIsDataPtrAttr ( i.m_eType ) )
		{
			BYTE * pData = (BYTE *)tMatch.GetAttr ( i.m_tLoc );
			// delete[] pData;
			if ( pData )
			{
				sphDeallocatePacked ( pData );
				tMatch.SetAttr ( i.m_tLoc, 0 );
			}
		}
}

void CSphQueryContext::FreeDataFilter ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcFilter );
}


void CSphQueryContext::FreeDataSort ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcSort );
}

void CSphQueryContext::ExprCommand ( ESphExprCommand eCmd, void * pArg )
{
	ARRAY_FOREACH ( i, m_dCalcFilter )
		m_dCalcFilter[i].m_pExpr->Command ( eCmd, pArg );
	ARRAY_FOREACH ( i, m_dCalcSort )
		m_dCalcSort[i].m_pExpr->Command ( eCmd, pArg );
	ARRAY_FOREACH ( i, m_dCalcFinal )
		m_dCalcFinal[i].m_pExpr->Command ( eCmd, pArg );
}


void CSphQueryContext::SetBlobPool ( const BYTE * pBlobPool )
{
	ExprCommand ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	if ( m_pFilter )
		m_pFilter->SetBlobStorage ( pBlobPool );
	if ( m_pWeightFilter )
		m_pWeightFilter->SetBlobStorage ( pBlobPool );
}


/// FIXME, perhaps
/// this rather crappy helper class really serves exactly 1 (one) simple purpose
///
/// it passes a sorting queue internals (namely, weight and float sortkey, if any,
/// of the current-worst queue element) to the MIN_TOP_WORST() and MIN_TOP_SORTVAL()
/// expression classes that expose those to the cruel outside world
///
/// all the COM-like EXTRA_xxx message back and forth is needed because expressions
/// are currently parsed and created earlier than the sorting queue
///
/// that also is the reason why we mischievously return 0 instead of clearly failing
/// with an error when the sortval is not a dynamic float; by the time we are parsing
/// expressions, we do not *yet* know that; but by the time we create a sorting queue,
/// we do not *want* to leak select expression checks into it
///
/// alternatively, we probably want to refactor this and introduce Bind(), to parse
/// expressions once, then bind them to actual searching contexts (aka index or segment,
/// and ranker, and sorter, and whatever else might be referenced by the expressions)
struct ContextExtra : public ISphExtra
{
	ISphRanker * m_pRanker;
	ISphMatchSorter * m_pSorter;

	virtual bool ExtraDataImpl ( ExtraData_e eData, void ** ppArg )
	{
		if ( eData==EXTRA_GET_QUEUE_WORST || eData==EXTRA_GET_QUEUE_SORTVAL )
		{
			if ( !m_pSorter )
				return false;
			const CSphMatch * pWorst = m_pSorter->GetWorst();
			if ( !pWorst )
				return false;
			if ( eData==EXTRA_GET_QUEUE_WORST )
			{
				*ppArg = (void*)pWorst;
				return true;
			} else
			{
				assert ( eData==EXTRA_GET_QUEUE_SORTVAL );
				const CSphMatchComparatorState & tCmp = m_pSorter->GetState();
				if ( tCmp.m_eKeypart[0]==SPH_KEYPART_FLOAT && tCmp.m_tLocator[0].m_bDynamic
					&& tCmp.m_tLocator[0].m_iBitCount==32 && ( tCmp.m_tLocator[0].m_iBitOffset%32==0 )
					&& tCmp.m_eKeypart[1]==SPH_KEYPART_ROWID && tCmp.m_dAttrs[1]==-1 )
				{
					*(int*)ppArg = tCmp.m_tLocator[0].m_iBitOffset/32;
					return true;
				} else
				{
					// min_top_sortval() only works with order by float_expr for now
					return false;
				}
			}
		}
		return m_pRanker->ExtraData ( eData, ppArg );
	}
};


void CSphQueryContext::SetupExtraData ( ISphRanker * pRanker, ISphMatchSorter * pSorter )
{
	ContextExtra tExtra;
	tExtra.m_pRanker = pRanker;
	tExtra.m_pSorter = pSorter;
	ExprCommand ( SPH_EXPR_SET_EXTRA_DATA, &tExtra );
}


template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS>
void CSphIndex_VLN::MatchExtended ( CSphQueryContext * pCtx, const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters,
									ISphRanker * pRanker, int iTag, int iIndexWeight ) const
{
	CSphQueryProfile * pProfile = pCtx->m_pProfile;
	ESphQueryState eOldState = SPH_QSTATE_UNKNOWN;
	if ( pProfile )
		eOldState = pProfile->m_eState;

	int iCutoff = pQuery->m_iCutoff;
	if ( iCutoff<=0 )
		iCutoff = -1;

	// do searching
	CSphMatch * pMatch = pRanker->GetMatchesBuffer();
	while (true)
	{
		// ranker does profile switches internally in GetMatches()
		int iMatches = pRanker->GetMatches();
		if ( iMatches<=0 )
			break;

		if ( pProfile )
			pProfile->Switch ( SPH_QSTATE_SORT );

		for ( int i=0; i<iMatches; i++ )
		{
			CSphMatch & tMatch = pMatch[i];
			assert ( tMatch.m_pStatic ); // should already be set in EarlyReject

			if_const ( USE_KLIST )
			{
				if ( m_tDeadRowMap.IsSet ( tMatch.m_tRowID ) )
					continue;
			}

			tMatch.m_iWeight *= iIndexWeight;
			pCtx->CalcSort ( tMatch );

			if ( pCtx->m_pWeightFilter && !pCtx->m_pWeightFilter->Eval ( tMatch ) )
			{
				pCtx->FreeDataSort ( tMatch );
				continue;
			}

			tMatch.m_iTag = iTag;

			bool bRand = false;
			bool bNewMatch = false;
			for ( int iSorter=0; iSorter<iSorters; iSorter++ )
			{
				// all non-random sorters are in the beginning,
				// so we can avoid the simple 'first-element' assertion
				if_const ( RANDOMIZE )
				{
					if ( !bRand && ppSorters[iSorter]->m_bRandomize )
					{
						bRand = true;
						tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

						if ( pCtx->m_pWeightFilter && !pCtx->m_pWeightFilter->Eval ( tMatch ) )
							break;
					}
				}

				bNewMatch |= ppSorters[iSorter]->Push ( tMatch );

				if_const ( USE_FACTORS )
				{
					pRanker->ExtraData ( EXTRA_SET_MATCHPUSHED, (void**)&(ppSorters[iSorter]->m_iJustPushed) );
					pRanker->ExtraData ( EXTRA_SET_MATCHPOPPED, (void**)&(ppSorters[iSorter]->m_dJustPopped) );
				}
			}

			pCtx->FreeDataFilter ( tMatch );
			pCtx->FreeDataSort ( tMatch );

			if ( bNewMatch )
				if ( --iCutoff==0 )
					break;
		}

		if ( iCutoff==0 )
			break;
	}

	if ( pProfile )
		pProfile->Switch ( eOldState );
}

//////////////////////////////////////////////////////////////////////////


struct SphFinalMatchCalc_t : ISphMatchProcessor, ISphNoncopyable
{
	const CSphQueryContext &	m_tCtx;
	int64_t						m_iBadRows {0};
	int							m_iTag;

	SphFinalMatchCalc_t ( int iTag, const CSphQueryContext & tCtx )
		: m_tCtx ( tCtx )
		, m_iTag ( iTag )
	{ }

	void Process ( CSphMatch * pMatch ) final
	{
		// fixme! tag is signed int,
		// for distr. tags from remotes set with | 0x80000000,
		// i e in terms of signed int they're <0!
		// Is it intention, or bug?
		// If intention, lt us use uniformely either <0, either &0x80000000
		// conditions to avoid messing. If bug, shit already happened!
		if ( pMatch->m_iTag>=0 )
			return;

		m_tCtx.CalcFinal ( *pMatch );
		pMatch->m_iTag = m_iTag;
	}
};


/// scoped thread scheduling helper
/// either makes the current thread low priority while the helper lives, or does nothing
class ScopedThreadPriority_c
{
private:
	bool m_bRestore;

public:
	ScopedThreadPriority_c ( bool bLowPriority )
	{
		m_bRestore = false;
		if ( !bLowPriority )
			return;

#if USE_WINDOWS
		if ( !SetThreadPriority ( GetCurrentThread(), THREAD_PRIORITY_IDLE ) )
			return;
#else
		struct sched_param p;
		p.sched_priority = 0;
#ifdef SCHED_IDLE
		int iPolicy = SCHED_IDLE;
#else
		int iPolicy = SCHED_OTHER;
#endif
		if ( pthread_setschedparam ( pthread_self (), iPolicy, &p ) )
			return;
#endif

		m_bRestore = true;
	}

	~ScopedThreadPriority_c ()
	{
		if ( !m_bRestore )
			return;

#if USE_WINDOWS
		if ( !SetThreadPriority ( GetCurrentThread(), THREAD_PRIORITY_NORMAL ) )
			return;
#else
		struct sched_param p;
		p.sched_priority = 0;
		if ( pthread_setschedparam ( pthread_self (), SCHED_OTHER, &p ) )
			return;
#endif
	}
};

//////////////////////////////////////////////////////////////////////////

class RowIterator_c : public ISphNoncopyable
{
public:
	RowIterator_c ( const CSphRowitem * pRow, const CSphRowitem * pRowEnd, RowID_t tRowID, int iStride, bool bReverse, const DeadRowMap_Disk_c & tDeadRowMap )
		: m_pFirstRow	( pRow )
		, m_pRow		( pRow )
		, m_pRowEnd		( pRowEnd )
		, m_iRowStep	( bReverse ? -iStride : iStride )
		, m_iRowIDStep	( bReverse ? -1 : 1 )
		, m_tRowID		( tRowID )
		, m_tDeadRowMap	( tDeadRowMap )
	{}

	inline const CSphRowitem * GetNextRow()
	{
		while ( m_pRow!=m_pRowEnd )
		{
			m_pRow += m_iRowStep;
			m_tRowID += m_iRowIDStep;

			if ( !m_tDeadRowMap.IsSet ( m_tRowID-m_iRowIDStep ) )
				return m_pRow-m_iRowStep;
		}

		m_tRowID = INVALID_ROWID;
		return nullptr;
	}

	inline RowID_t GetRowID() const
	{
		return m_tRowID-m_iRowIDStep;
	}

	DWORD GetNumProcessed() const
	{
		return (m_pRow-m_pFirstRow)/m_iRowStep;
	}

private:
	const CSphRowitem *			m_pFirstRow {nullptr};
	const CSphRowitem *			m_pRow {nullptr};
	const CSphRowitem *			m_pRowEnd {nullptr};
	int							m_iRowStep {1};
	int							m_iRowIDStep {1};
	RowID_t						m_tRowID {INVALID_ROWID};
	const DeadRowMap_Disk_c &	m_tDeadRowMap;
};


// adds killlist filtering to a rowid iterator
class RowIteratorAlive_c : public ISphNoncopyable
{
public:
	RowIteratorAlive_c ( RowidIterator_i * pIterator, const CSphRowitem * pRows, int iStride, const DeadRowMap_Disk_c & tDeadRowMap )
		: m_pIterator	( pIterator )
		, m_pRows		( pRows )
		, m_iStride		( iStride )
		, m_tDeadRowMap	( tDeadRowMap )
	{
		assert ( m_pIterator );
	}

	~RowIteratorAlive_c()
	{
		SafeDelete ( m_pIterator );
	}

	inline const CSphRowitem * GetNextRow()
	{
		do
		{
			m_tRowID = m_pIterator->GetNextRowID();
		}
		while ( m_tRowID!=INVALID_ROWID && m_tDeadRowMap.IsSet ( m_tRowID ) );

		if ( m_tRowID==INVALID_ROWID )
			return nullptr;

		return m_pRows + (int64_t)m_tRowID*m_iStride;
	}

	inline RowID_t GetRowID() const
	{
		return m_tRowID;
	}

	DWORD GetNumProcessed() const
	{
		return m_pIterator->GetNumProcessed();
	}

private:
	RowidIterator_i *			m_pIterator;
	const CSphRowitem *			m_pRows;
	int							m_iStride;
	const DeadRowMap_Disk_c &	m_tDeadRowMap;
	RowID_t						m_tRowID {INVALID_ROWID};
};

//////////////////////////////////////////////////////////////////////////


template <typename T>
void Fullscan ( T & tIterator, const CSphQueryContext & tCtx, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, CSphMatch & tMatch, int iCutoff,
	bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, bool & bStop )
{
	const CSphRowitem * pRow;
	while ( !!(pRow=tIterator.GetNextRow()) )
	{
		tMatch.m_pStatic = pRow;
		tMatch.m_tRowID = tIterator.GetRowID();

		// early filter only (no late filters in full-scan because of no @weight)
		tCtx.CalcFilter ( tMatch );
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
		{
			tCtx.FreeDataFilter ( tMatch );
			continue;
		}

		if ( bRandomize )
			tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

		// submit match to sorters
		tCtx.CalcSort ( tMatch );

		bool bNewMatch = false;
		for ( int iSorter=0; iSorter<iSorters; iSorter++ )
			bNewMatch |= ppSorters[iSorter]->Push ( tMatch );

		// stringptr expressions should be duplicated (or taken over) at this point
		tCtx.FreeDataFilter ( tMatch );
		tCtx.FreeDataSort ( tMatch );

		// handle cutoff
		if ( bNewMatch && --iCutoff==0 )
		{
			bStop = true;
			break;
		}

		// handle timer
		if ( tmMaxTimer && sphMicroTimer()>=tmMaxTimer )
		{
			pResult->m_sWarning = "query time exceeded max_query_time";
			bStop = true;
			break;
		}
	}

	pResult->m_tStats.m_iFetchedDocs = tIterator.GetNumProcessed();
}


void CSphIndex_VLN::ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, CSphMatch & tMatch, int iCutoff, bool bReverse, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const
{
	int iStride = m_tSchema.GetRowSize();
	int64_t iStart = bReverse ? m_iDocinfoIndex-1 : 0;
	int64_t iEnd = bReverse ? -1 : m_iDocinfoIndex;
	int64_t iStep = bReverse ? -1 : 1;
	for ( int64_t iIndexEntry=iStart; iIndexEntry!=iEnd; iIndexEntry+=iStep )
	{
		// block-level filtering
		const DWORD * pMin = &m_pDocinfoIndex[ iIndexEntry*iStride*2 ];
		const DWORD * pMax = pMin + iStride;
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->EvalBlock ( pMin, pMax ) )
			continue;

		// row-level filtering
		const DWORD * pBlockStart = m_tAttr.GetWritePtr() + ( iIndexEntry*iStride*DOCINFO_INDEX_FREQ );
		const DWORD * pBlockEnd = m_tAttr.GetWritePtr() + ( Min ( ( iIndexEntry+1 )*DOCINFO_INDEX_FREQ, m_iDocinfo )*iStride );
		if ( bReverse )
		{
			pBlockStart = m_tAttr.GetWritePtr() + ( ( Min ( ( iIndexEntry+1 )*DOCINFO_INDEX_FREQ, m_iDocinfo ) - 1 ) * iStride );
			pBlockEnd = m_tAttr.GetWritePtr() + iStride*( iIndexEntry*DOCINFO_INDEX_FREQ-1 );
		}

		tMatch.m_tRowID = GetRowIDByDocinfo ( pBlockStart );

		int iDocinfoStep = bReverse ? -(int)iStride : (int)iStride;
		int iRowIDStep = bReverse ? -1 : 1;

		if ( tCtx.m_pFilter && iCutoff==-1 && !tCtx.m_dCalcFilter.GetLength() && !tCtx.m_dCalcSort.GetLength() && !tmMaxTimer )
		{
			// kinda fastpath
			for ( const DWORD * pDocinfo=pBlockStart; pDocinfo!=pBlockEnd; pDocinfo+=iDocinfoStep, tMatch.m_tRowID+=iRowIDStep )
			{
				pResult->m_tStats.m_iFetchedDocs++;

				if ( m_tDeadRowMap.IsSet ( tMatch.m_tRowID ) )
					continue;

				tMatch.m_pStatic = pDocinfo;

				if ( tCtx.m_pFilter->Eval ( tMatch ) )
				{
					if ( bRandomize )
						tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;
					for ( int iSorter=0; iSorter<iSorters; iSorter++ )
						ppSorters[iSorter]->Push ( tMatch );
				}
				// stringptr expressions should be duplicated (or taken over) at this point
				tCtx.FreeDataFilter ( tMatch );
			}
		}
		else
		{
			RowIterator_c tIt ( pBlockStart, pBlockEnd, tMatch.m_tRowID, iStride, bReverse, m_tDeadRowMap );

			bool bStop = false;
			Fullscan ( tIt, tCtx, pResult, iSorters, ppSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );

			if ( bStop || !iCutoff )
				break;
		}
	}
}


bool CSphIndex_VLN::MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	assert ( tArgs.m_iTag>=0 );

	// check if index is ready
	if ( !m_bPassedAlloc )
	{
		pResult->m_sError = "index not preread";
		return false;
	}

	// check if index supports scans
	if ( !m_tSchema.GetAttrsCount() )
	{
		pResult->m_sError = "need attributes to run fullscan";
		return false;
	}

	// we count documents only (before filters)
	if ( pQuery->m_iMaxPredictedMsec )
		pResult->m_bHasPrediction = true;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		pResult->m_sWarning.SetSprintf ( "packedfactors() will not work with a fullscan; you need to specify a query" );

	// check if index has data
	if ( m_bIsEmpty || m_iDocinfo<=0 || m_tAttr.IsEmpty() )
		return true;

	// start counting
	int64_t tmQueryStart = sphMicroTimer();
	int64_t tmMaxTimer = 0;
	if ( pQuery->m_uMaxQueryMsec>0 )
		tmMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time

	ScopedThreadPriority_c tPrio ( pQuery->m_bLowPriority );

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i]->GetSchema()->GetAttrsCount() > iMaxSchemaSize )
		{
			iMaxSchemaSize = ppSorters[i]->GetSchema()->GetAttrsCount();
			iMaxSchemaIndex = i;
		}

	const ISphSchema & tMaxSorterSchema = *(ppSorters[iMaxSchemaIndex]->GetSchema());

	CSphVector< const ISphSchema * > dSorterSchemas;
	SorterSchemas ( ppSorters, iSorters, iMaxSchemaIndex, dSorterSchemas );

	// setup calculations and result schema
	CSphQueryContext tCtx ( *pQuery );
	if ( !tCtx.SetupCalc ( pResult, tMaxSorterSchema, m_tSchema, m_tBlobAttrs.GetWritePtr(), dSorterSchemas ) )
		return false;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetWritePtr() );

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &pQuery->m_dFilters;
	tFlx.m_pFilterTree = &pQuery->m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_pBlobPool = m_tBlobAttrs.GetWritePtr ();
	tFlx.m_eCollation = pQuery->m_eCollation;
	tFlx.m_bScan = true;
	tFlx.m_pHistograms = m_pHistograms;

	if ( !tCtx.CreateFilters ( tFlx, pResult->m_sError, pResult->m_sWarning ) )
			return false;

	int iStride = m_tSchema.GetRowSize();

	// check if we can early reject the whole index
	if ( tCtx.m_pFilter && m_iDocinfoIndex )
	{
		DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ m_iDocinfoIndex*iStride*2 ] );
		DWORD * pMaxEntry = pMinEntry + iStride;

		if ( !tCtx.m_pFilter->EvalBlock ( pMinEntry, pMaxEntry ) )
		{
			pResult->m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
			return true;
		}
	}

	// setup sorters vs. MVA
	for ( int i=0; i<iSorters; i++ )
		(ppSorters[i])->SetBlobPool ( m_tBlobAttrs.GetWritePtr() );

	// prepare to work them rows
	bool bRandomize = ppSorters[0]->m_bRandomize;

	CSphMatch tMatch;
	tMatch.Reset ( tMaxSorterSchema.GetDynamicSize() );
	tMatch.m_iWeight = tArgs.m_iIndexWeight;
	// fixme! tag also used over bitmask | 0x80000000,
	// which marks that match comes from remote.
	// using -1 might be also interpreted as 0xFFFFFFFF in such context!
	// Does it intended?
	tMatch.m_iTag = tCtx.m_dCalcFinal.GetLength() ? -1 : tArgs.m_iTag;

	if ( pResult->m_pProfile )
		pResult->m_pProfile->Switch ( SPH_QSTATE_FULLSCAN );

	// run full scan with block and row filtering for everything else
	bool bReverse = pQuery->m_bReverseScan; // shortcut
	int iCutoff = ( pQuery->m_iCutoff<=0 ) ? -1 : pQuery->m_iCutoff;

	// we don't modify the original filters because iterators may use some data from them (to avoid copying)
	CSphVector<CSphFilterSettings> dModifiedFilters;
	RowidIterator_i * pIterator = nullptr;
	if ( m_pHistograms )
		pIterator = CreateFilteredIterator ( pQuery->m_dFilters, dModifiedFilters, pQuery->m_dFilterTree, pQuery->m_dIndexHints, *m_pHistograms, m_tDocidLookup.GetWritePtr() );

	if ( pIterator )
	{
		// one or several filters got replaced by an iterator, need to re-create the remaining filters
		SafeDelete ( tCtx.m_pFilter );
		tFlx.m_pFilters = &dModifiedFilters;
		tCtx.CreateFilters ( tFlx, pResult->m_sError, pResult->m_sWarning );

		bool bStop;
		RowIteratorAlive_c tIt ( pIterator, m_tAttr.GetWritePtr(), iStride, m_tDeadRowMap );
		Fullscan ( tIt, tCtx, pResult, iSorters, ppSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer, bStop );
	}
	else
		ScanByBlocks ( tCtx, pResult, iSorters, ppSorters, tMatch, iCutoff, bReverse, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );

	if ( pResult->m_pProfile )
		pResult->m_pProfile->Switch ( SPH_QSTATE_FINALIZE );

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength() )
	{
		SphFinalMatchCalc_t tFinal ( tArgs.m_iTag, tCtx );
		for ( int iSorter=0; iSorter<iSorters; iSorter++ )
		{
			ISphMatchSorter * pTop = ppSorters[iSorter];
			pTop->Finalize ( tFinal, false );
		}
		tCtx.m_iBadRows += tFinal.m_iBadRows;
	}

	if ( tArgs.m_bModifySorterSchemas )
	{
		if ( pResult->m_pProfile )
			pResult->m_pProfile->Switch ( SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( ppSorters, iSorters, m_tBlobAttrs.GetWritePtr() );
	}

	// done
	pResult->m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	pResult->m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
	pResult->m_iBadRows += tCtx.m_iBadRows;

	return true;
}

//////////////////////////////////////////////////////////////////////////////

ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( !tWord.m_pPayload )
	{
		WITH_QWORD ( m_pIndex, false, Qword, return new Qword ( tWord.m_bExpanded, tWord.m_bExcluded ) );
	} else
	{
		if ( m_pIndex->GetSettings().m_eHitFormat==SPH_HIT_FORMAT_INLINE )
		{
			return new DiskPayloadQword_c<true> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist );
		} else
		{
			return new DiskPayloadQword_c<false> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist );
		}
	}
	return NULL;
}


bool DiskIndexQwordSetup_c::QwordSetup ( ISphQword * pWord ) const
{
	DiskIndexQwordTraits_c * pMyWord = (DiskIndexQwordTraits_c*)pWord;

	// setup attrs
	pMyWord->m_tDoc.Reset ( m_iDynamicRowitems );
	pMyWord->m_tDoc.m_tRowID = INVALID_ROWID;

	return pMyWord->Setup ( this );
}


bool DiskIndexQwordSetup_c::Setup ( ISphQword * pWord ) const
{
	// there was a dynamic_cast here once but it's not necessary
	// maybe it worth to rewrite class hierarchy to avoid c-cast here?
	DiskIndexQwordTraits_c & tWord = *(DiskIndexQwordTraits_c*)pWord;

	// setup stats
	tWord.m_iDocs = 0;
	tWord.m_iHits = 0;

	CSphIndex_VLN * pIndex = (CSphIndex_VLN *)m_pIndex;

	// !COMMIT FIXME!
	// the below stuff really belongs in wordlist
	// which in turn really belongs in dictreader
	// which in turn might or might not be a part of dict

	// binary search through checkpoints for a one whose range matches word ID
	assert ( pIndex->m_bPassedAlloc );
	assert ( !pIndex->m_tWordlist.m_tBuf.IsEmpty() );

	// empty index?
	if ( !pIndex->m_tWordlist.m_dCheckpoints.GetLength() )
		return false;

	const char * sWord = tWord.m_sDictWord.cstr();
	const bool bWordDict = pIndex->m_pDict->GetSettings().m_bWordDict;
	int iWordLen = sWord ? strlen ( sWord ) : 0;
	if ( bWordDict && tWord.m_sWord.Ends("*") )
	{
		iWordLen = Max ( iWordLen-1, 0 );

		// might match either infix or prefix
		int iMinLen = Max ( pIndex->m_tSettings.m_iMinPrefixLen, pIndex->m_tSettings.m_iMinInfixLen );
		if ( pIndex->m_tSettings.m_iMinPrefixLen )
			iMinLen = Min ( iMinLen, pIndex->m_tSettings.m_iMinPrefixLen );
		if ( pIndex->m_tSettings.m_iMinInfixLen )
			iMinLen = Min ( iMinLen, pIndex->m_tSettings.m_iMinInfixLen );

		// bail out term shorter than prefix or infix allowed
		if ( iWordLen<iMinLen )
			return false;
	}

	// leading special symbols trimming
	if ( bWordDict && tWord.m_sDictWord.Begins("*") )
	{
		sWord++;
		iWordLen = Max ( iWordLen-1, 0 );
		// bail out term shorter than infix allowed
		if ( iWordLen<pIndex->m_tSettings.m_iMinInfixLen )
			return false;
	}

	const CSphWordlistCheckpoint * pCheckpoint = pIndex->m_tWordlist.FindCheckpoint ( sWord, iWordLen, tWord.m_uWordID, false );
	if ( !pCheckpoint )
		return false;

	// decode wordlist chunk
	const BYTE * pBuf = pIndex->m_tWordlist.AcquireDict ( pCheckpoint );
	assert ( pBuf );

	assert ( m_iSkiplistBlockSize>0 );

	CSphDictEntry tRes;
	if ( bWordDict )
	{
		KeywordsBlockReader_c tCtx ( pBuf, m_iSkiplistBlockSize );
		while ( tCtx.UnpackWord() )
		{
			// block is sorted
			// so once keywords are greater than the reference word, no more matches
			assert ( tCtx.GetWordLen()>0 );
			int iCmp = sphDictCmpStrictly ( sWord, iWordLen, tCtx.GetWord(), tCtx.GetWordLen() );
			if ( iCmp<0 )
				return false;
			if ( iCmp==0 )
				break;
		}
		if ( tCtx.GetWordLen()<=0 )
			return false;
		tRes = tCtx;

	} else
	{
		if ( !pIndex->m_tWordlist.GetWord ( pBuf, tWord.m_uWordID, tRes ) )
			return false;
	}

	const ESphHitless eMode = pIndex->m_tSettings.m_eHitless;
	tWord.m_iDocs = eMode==SPH_HITLESS_SOME ? ( tRes.m_iDocs & HITLESS_DOC_MASK ) : tRes.m_iDocs;
	tWord.m_iHits = tRes.m_iHits;
	tWord.m_bHasHitlist =
		( eMode==SPH_HITLESS_NONE ) ||
		( eMode==SPH_HITLESS_SOME && !( tRes.m_iDocs & HITLESS_DOC_FLAG ) );

	if ( m_bSetupReaders )
	{
		tWord.SetDocReader ( m_pDoclist );

		// read in skiplist
		// OPTIMIZE? maybe cache hot decompressed lists?
		// OPTIMIZE? maybe add an option to decompress on preload instead?
		if ( m_pSkips && tRes.m_iDocs>m_iSkiplistBlockSize )
		{
			const BYTE * pSkip = m_pSkips + tRes.m_iSkiplistOffset;

			tWord.m_dSkiplist.Add();
			tWord.m_dSkiplist.Last().m_tBaseRowIDPlus1 = 0;
			tWord.m_dSkiplist.Last().m_iOffset = tRes.m_iDoclistOffset;
			tWord.m_dSkiplist.Last().m_iBaseHitlistPos = 0;

			for ( int i=1; i<( tWord.m_iDocs/m_iSkiplistBlockSize ); i++ )
			{
				SkiplistEntry_t & t = tWord.m_dSkiplist.Add();
				SkiplistEntry_t & p = tWord.m_dSkiplist [ tWord.m_dSkiplist.GetLength()-2 ];
				t.m_tBaseRowIDPlus1 = p.m_tBaseRowIDPlus1 + m_iSkiplistBlockSize + sphUnzipInt ( pSkip );
				t.m_iOffset = p.m_iOffset + 4*m_iSkiplistBlockSize + sphUnzipOffset ( pSkip );
				t.m_iBaseHitlistPos = p.m_iBaseHitlistPos + sphUnzipOffset ( pSkip );
			}
		}

		tWord.m_rdDoclist->SeekTo ( tRes.m_iDoclistOffset, tRes.m_iDoclistHint );
		tWord.SetHitReader ( m_pHitlist );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool RawFileLock ( const CSphString sFile, int &iLockFD, CSphString &sError )
{
	if ( iLockFD<0 )
	{
		iLockFD = ::open ( sFile.cstr (), SPH_O_NEW, 0644 );
		if ( iLockFD<0 )
		{
			sError.SetSprintf ( "failed to open %s: %s", sFile.cstr (), strerrorm ( errno ) );
			sphLogDebug ( "failed to open %s: %s", sFile.cstr (), strerrorm ( errno ) );
			return false;
		}
	}

	if ( !sphLockEx ( iLockFD, false ) )
	{
		sError.SetSprintf ( "failed to lock %s: %s", sFile.cstr (), strerrorm ( errno ) );
		::close ( iLockFD );
		iLockFD = -1;
		return false;
	}
	sphLogDebug ( "lock %s success", sFile.cstr () );
	return true;
}

void RawFileUnLock ( const CSphString sFile, int &iLockFD )
{
	if ( iLockFD<0 )
		return;
	sphLogDebug ( "File ID ok, closing lock FD %d, unlinking %s", iLockFD, sFile.cstr () );
	sphLockUn ( iLockFD );
	::close ( iLockFD );
	::unlink ( sFile.cstr () );
	iLockFD = -1;
}

bool CSphIndex_VLN::Lock ()
{
	CSphString sName = GetIndexFileName(SPH_EXT_SPL);
	sphLogDebug ( "Locking the index via file %s", sName.cstr() );
	return RawFileLock ( sName, m_iLockFD, m_sLastError );
}

void CSphIndex_VLN::Unlock()
{
	CSphString sName = GetIndexFileName(SPH_EXT_SPL);
	if ( m_iLockFD<0 )
		return;
	sphLogDebug ( "Unlocking the index (lock %s)", sName.cstr() );
	RawFileUnLock ( sName, m_iLockFD );
}


void CSphIndex_VLN::Dealloc ()
{
	if ( !m_bPassedAlloc )
		return;

	m_pDoclistFile = nullptr;
	m_pHitlistFile = nullptr;

	m_tAttr.Reset ();
	m_tBlobAttrs.Reset();
	m_tSkiplists.Reset ();
	m_tWordlist.Reset ();
	m_tDeadRowMap.Dealloc();
	m_tDocidLookup.Reset();

	m_iDocinfo = 0;
	m_iMinMaxIndex = 0;

	m_bPassedRead = false;
	m_bPassedAlloc = false;
	m_uAttrsStatus = false;

	QcacheDeleteIndex ( m_iIndexId );
	m_iIndexId = m_tIdGenerator.Inc();
}


void LoadIndexSettings ( CSphIndexSettings & tSettings, CSphReader & tReader, DWORD uVersion )
{
	tSettings.m_iMinPrefixLen = tReader.GetDword ();
	tSettings.m_iMinInfixLen = tReader.GetDword ();
	tSettings.m_iMaxSubstringLen = tReader.GetDword();

	tSettings.m_bHtmlStrip = !!tReader.GetByte ();
	tSettings.m_sHtmlIndexAttrs = tReader.GetString ();
	tSettings.m_sHtmlRemoveElements = tReader.GetString ();

	tSettings.m_bIndexExactWords = !!tReader.GetByte ();
	tSettings.m_eHitless = (ESphHitless)tReader.GetDword();

	tSettings.m_eHitFormat = (ESphHitFormat)tReader.GetDword();
	tSettings.m_bIndexSP = !!tReader.GetByte();

	tSettings.m_sZones = tReader.GetString();

	tSettings.m_iBoundaryStep = (int)tReader.GetDword();
	tSettings.m_iStopwordStep = (int)tReader.GetDword();

	tSettings.m_iOvershortStep = (int)tReader.GetDword();
	tSettings.m_iEmbeddedLimit = (int)tReader.GetDword();

	tSettings.m_eBigramIndex = (ESphBigram)tReader.GetByte();
	tSettings.m_sBigramWords = tReader.GetString();

	tSettings.m_bIndexFieldLens = ( tReader.GetByte()!=0 );

	tSettings.m_eChineseRLP = (ESphRLPFilter)tReader.GetByte();
	tSettings.m_sRLPContext = tReader.GetString();

	tSettings.m_sIndexTokenFilter = tReader.GetString();
	tSettings.m_tBlobUpdateSpace = tReader.GetOffset();

	if ( uVersion<56 )
		tSettings.m_iSkiplistBlockSize = 128;
	else
		tSettings.m_iSkiplistBlockSize = (int)tReader.GetDword();
}


bool CSphIndex_VLN::LoadHeader ( const char * sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning )
{
	const int MAX_HEADER_SIZE = 32768;
	CSphFixedVector<BYTE> dCacheInfo ( MAX_HEADER_SIZE );

	CSphAutoreader rdInfo ( dCacheInfo.Begin(), MAX_HEADER_SIZE ); // to avoid mallocs
	if ( !rdInfo.Open ( sHeaderName, m_sLastError ) )
		return false;

	// magic header
	const char* sFmt = CheckFmtMagic ( rdInfo.GetDword () );
	if ( sFmt )
	{
		m_sLastError.SetSprintf ( sFmt, sHeaderName );
		return false;
	}

	// version
	m_uVersion = rdInfo.GetDword();
	if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sHeaderName, m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	// we don't support anything prior to v54
	DWORD uMinFormatVer = 54;
	if ( m_uVersion<uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "indexes prior to v.%d are no longer supported (use index_converter tool); %s is v.%d", uMinFormatVer, sHeaderName, m_uVersion );
		return false;
	}

	// schema
	// 4th arg means that inline attributes need be dynamic in searching time too
	ReadSchema ( rdInfo, m_tSchema );

	// check schema for dupes
	for ( int iAttr=0; iAttr<m_tSchema.GetAttrsCount(); iAttr++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(iAttr);
		for ( int i=0; i<iAttr; i++ )
			if ( m_tSchema.GetAttr(i).m_sName==tCol.m_sName )
				sWarning.SetSprintf ( "duplicate attribute name: %s", tCol.m_sName.cstr() );
	}

	// dictionary header (wordlist checkpoints, infix blocks, etc)
	m_tWordlist.m_iDictCheckpointsOffset = rdInfo.GetOffset();
	m_tWordlist.m_iDictCheckpoints = rdInfo.GetDword();
	m_tWordlist.m_iInfixCodepointBytes = rdInfo.GetByte();
	m_tWordlist.m_iInfixBlocksOffset = rdInfo.GetDword();
	m_tWordlist.m_iInfixBlocksWordsSize = rdInfo.GetDword();

	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );

	// index stats
	m_tStats.m_iTotalDocuments = rdInfo.GetDword ();
	m_tStats.m_iTotalBytes = rdInfo.GetOffset ();

	LoadIndexSettings ( m_tSettings, rdInfo, m_uVersion );

	CSphTokenizerSettings tTokSettings;

	// tokenizer stuff
	if ( !LoadTokenizerSettings ( rdInfo, tTokSettings, tEmbeddedFiles, m_sLastError ) )
		return false;

	if ( bStripPath )
		StripPath ( tTokSettings.m_sSynonymsFile );

	ISphTokenizerRefPtr_c pTokenizer { ISphTokenizer::Create ( tTokSettings, &tEmbeddedFiles, m_sLastError ) };
	if ( !pTokenizer )
		return false;

	// dictionary stuff
	CSphDictSettings tDictSettings;
	LoadDictionarySettings ( rdInfo, tDictSettings, tEmbeddedFiles, sWarning );

	if ( bStripPath )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	CSphDictRefPtr_c pDict { tDictSettings.m_bWordDict
		? sphCreateDictionaryKeywords ( tDictSettings, &tEmbeddedFiles, pTokenizer, m_sIndexName.cstr(), bStripPath, m_tSettings.m_iSkiplistBlockSize, m_sLastError )
		: sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, pTokenizer, m_sIndexName.cstr(), bStripPath, m_tSettings.m_iSkiplistBlockSize, m_sLastError )};

	if ( !pDict )
		return false;

	if ( tDictSettings.m_sMorphFingerprint!=pDict->GetMorphDataFingerprint() )
		sWarning.SetSprintf ( "different lemmatizer dictionaries (index='%s', current='%s')",
			tDictSettings.m_sMorphFingerprint.cstr(),
			pDict->GetMorphDataFingerprint().cstr() );

	SetDictionary ( pDict );

	pTokenizer = ISphTokenizer::CreateMultiformFilter ( pTokenizer, pDict->GetMultiWordforms () );
	SetTokenizer ( pTokenizer );
	SetupQueryTokenizer();

	// initialize AOT if needed
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );

	m_iDocinfo = rdInfo.GetOffset ();
	m_iDocinfoIndex = rdInfo.GetOffset ();
	m_iMinMaxIndex = rdInfo.GetOffset ();

	ISphFieldFilterRefPtr_c pFieldFilter;
	CSphFieldFilterSettings tFieldFilterSettings;
	LoadFieldFilterSettings ( rdInfo, tFieldFilterSettings );
	if ( tFieldFilterSettings.m_dRegexps.GetLength() )
		pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

	if ( !sphSpawnRLPFilter ( pFieldFilter, m_tSettings, tTokSettings, sHeaderName, m_sLastError ) )

		return false;


	SetFieldFilter ( pFieldFilter );

	if ( m_tSettings.m_bIndexFieldLens )
		for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
			m_dFieldLens[i] = rdInfo.GetOffset(); // FIXME? ideally 64bit even when off is 32bit..

	// post-load stuff.. for now, bigrams
	CSphIndexSettings & s = m_tSettings;
	if ( s.m_eBigramIndex!=SPH_BIGRAM_NONE && s.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		BYTE * pTok;
		m_pTokenizer->SetBuffer ( (BYTE*)s.m_sBigramWords.cstr(), s.m_sBigramWords.Length() );
		while ( ( pTok = m_pTokenizer->GetToken() )!=NULL )
			s.m_dBigramWords.Add() = (const char*)pTok;
		s.m_dBigramWords.Sort();
	}


	if ( rdInfo.GetErrorFlag() )
		m_sLastError.SetSprintf ( "%s: failed to parse header (unexpected eof)", sHeaderName );

	return !rdInfo.GetErrorFlag();
}


void CSphIndex_VLN::DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig )
{
	CSphEmbeddedFiles tEmbeddedFiles;
	CSphString sWarning;
	if ( !LoadHeader ( sHeaderName, false, tEmbeddedFiles, sWarning ) )
	{
		sphDie ( "failed to load header: %s", m_sLastError.cstr ());
	}

	if ( !sWarning.IsEmpty () )
		fprintf ( fp, "WARNING: %s\n", sWarning.cstr () );

	///////////////////////////////////////////////
	// print header in index config section format
	///////////////////////////////////////////////

	if ( bConfig )
	{
		fprintf ( fp, "\nsource $dump\n{\n" );

		fprintf ( fp, "\tsql_query = SELECT id \\\n" );
		for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
			fprintf ( fp, "\t, %s \\\n", m_tSchema.GetFieldName(i) );
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			fprintf ( fp, "\t, %s \\\n", tAttr.m_sName.cstr() );
		}
		fprintf ( fp, "\tFROM documents\n" );

		if ( m_tSchema.GetAttrsCount() )
			fprintf ( fp, "\n" );

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET )
				fprintf ( fp, "\tsql_attr_multi = uint %s from field\n", tAttr.m_sName.cstr() );
			else if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
				fprintf ( fp, "\tsql_attr_multi = bigint %s from field\n", tAttr.m_sName.cstr() );
			else if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.IsBitfield() )
				fprintf ( fp, "\tsql_attr_uint = %s:%d\n", tAttr.m_sName.cstr(), tAttr.m_tLocator.m_iBitCount );
			else if ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			{; // intendedly skip, as these are autogenerated by index_field_lengths=1
			} else
				fprintf ( fp, "\t%s = %s\n", sphTypeDirective ( tAttr.m_eAttrType ), tAttr.m_sName.cstr() );
		}

		fprintf ( fp, "}\n\nindex $dump\n{\n\tsource = $dump\n\tpath = $dump\n" );

		if ( m_tSettings.m_iMinPrefixLen )
			fprintf ( fp, "\tmin_prefix_len = %d\n", m_tSettings.m_iMinPrefixLen );
		if ( m_tSettings.m_iMinInfixLen )
			fprintf ( fp, "\tmin_prefix_len = %d\n", m_tSettings.m_iMinInfixLen );
		if ( m_tSettings.m_iMaxSubstringLen )
			fprintf ( fp, "\tmax_substring_len = %d\n", m_tSettings.m_iMaxSubstringLen );
		if ( m_tSettings.m_bIndexExactWords )
			fprintf ( fp, "\tindex_exact_words = %d\n", m_tSettings.m_bIndexExactWords ? 1 : 0 );
		if ( m_tSettings.m_bHtmlStrip )
			fprintf ( fp, "\thtml_strip = 1\n" );
		if ( !m_tSettings.m_sHtmlIndexAttrs.IsEmpty() )
			fprintf ( fp, "\thtml_index_attrs = %s\n", m_tSettings.m_sHtmlIndexAttrs.cstr () );
		if ( !m_tSettings.m_sHtmlRemoveElements.IsEmpty() )
			fprintf ( fp, "\thtml_remove_elements = %s\n", m_tSettings.m_sHtmlRemoveElements.cstr () );
		if ( !m_tSettings.m_sZones.IsEmpty() )
			fprintf ( fp, "\tindex_zones = %s\n", m_tSettings.m_sZones.cstr() );
		if ( m_tSettings.m_bIndexFieldLens )
			fprintf ( fp, "\tindex_field_lengths = 1\n" );
		if ( m_tSettings.m_bIndexSP )
			fprintf ( fp, "\tindex_sp = 1\n" );
		if ( m_tSettings.m_iBoundaryStep!=0 )
			fprintf ( fp, "\tphrase_boundary_step = %d\n", m_tSettings.m_iBoundaryStep );
		if ( m_tSettings.m_iStopwordStep!=1 )
			fprintf ( fp, "\tstopword_step = %d\n", m_tSettings.m_iStopwordStep );
		if ( m_tSettings.m_iOvershortStep!=1 )
			fprintf ( fp, "\tovershort_step = %d\n", m_tSettings.m_iOvershortStep );
		if ( m_tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE )
			fprintf ( fp, "\tbigram_index = %s\n", sphBigramName ( m_tSettings.m_eBigramIndex ) );
		if ( !m_tSettings.m_sBigramWords.IsEmpty() )
			fprintf ( fp, "\tbigram_freq_words = %s\n", m_tSettings.m_sBigramWords.cstr() );
		if ( !m_tSettings.m_sRLPContext.IsEmpty() )
			fprintf ( fp, "\trlp_context = %s\n", m_tSettings.m_sRLPContext.cstr() );
		if ( !m_tSettings.m_sIndexTokenFilter.IsEmpty() )
			fprintf ( fp, "\tindex_token_filter = %s\n", m_tSettings.m_sIndexTokenFilter.cstr() );


		CSphFieldFilterSettings tFieldFilter;
		GetFieldFilterSettings ( tFieldFilter );
		ARRAY_FOREACH ( i, tFieldFilter.m_dRegexps )
			fprintf ( fp, "\tregexp_filter = %s\n", tFieldFilter.m_dRegexps[i].cstr() );

		if ( m_pTokenizer )
		{
			const CSphTokenizerSettings & tSettings = m_pTokenizer->GetSettings ();
			fprintf ( fp, "\tcharset_type = %s\n", ( tSettings.m_iType==TOKENIZER_UTF8 || tSettings.m_iType==TOKENIZER_NGRAM )
					? "utf-8"
					: "unknown tokenizer (deprecated sbcs?)" );
			if ( !tSettings.m_sCaseFolding.IsEmpty() )
				fprintf ( fp, "\tcharset_table = %s\n", tSettings.m_sCaseFolding.cstr () );
			if ( tSettings.m_iMinWordLen>1 )
				fprintf ( fp, "\tmin_word_len = %d\n", tSettings.m_iMinWordLen );
			if ( tSettings.m_iNgramLen && !tSettings.m_sNgramChars.IsEmpty() )
				fprintf ( fp, "\tngram_len = %d\nngram_chars = %s\n",
					tSettings.m_iNgramLen, tSettings.m_sNgramChars.cstr () );
			if ( !tSettings.m_sSynonymsFile.IsEmpty() )
				fprintf ( fp, "\texceptions = %s\n", tSettings.m_sSynonymsFile.cstr () );
			if ( !tSettings.m_sBoundary.IsEmpty() )
				fprintf ( fp, "\tphrase_boundary = %s\n", tSettings.m_sBoundary.cstr () );
			if ( !tSettings.m_sIgnoreChars.IsEmpty() )
				fprintf ( fp, "\tignore_chars = %s\n", tSettings.m_sIgnoreChars.cstr () );
			if ( !tSettings.m_sBlendChars.IsEmpty() )
				fprintf ( fp, "\tblend_chars = %s\n", tSettings.m_sBlendChars.cstr () );
			if ( !tSettings.m_sBlendMode.IsEmpty() )
				fprintf ( fp, "\tblend_mode = %s\n", tSettings.m_sBlendMode.cstr () );
		}

		if ( m_pDict )
		{
			const CSphDictSettings & tSettings = m_pDict->GetSettings ();
			if ( tSettings.m_bWordDict )
				fprintf ( fp, "\tdict = keywords\n" );
			if ( !tSettings.m_sMorphology.IsEmpty() )
				fprintf ( fp, "\tmorphology = %s\n", tSettings.m_sMorphology.cstr () );
			if ( !tSettings.m_sStopwords.IsEmpty() )
				fprintf ( fp, "\tstopwords = %s\n", tSettings.m_sStopwords.cstr () );
			if ( tSettings.m_dWordforms.GetLength() )
			{
				fprintf ( fp, "\twordforms =" );
				ARRAY_FOREACH ( i, tSettings.m_dWordforms )
					fprintf ( fp, " %s", tSettings.m_dWordforms[i].cstr () );
				fprintf ( fp, "\n" );
			}
			if ( tSettings.m_iMinStemmingLen>1 )
				fprintf ( fp, "\tmin_stemming_len = %d\n", tSettings.m_iMinStemmingLen );
			if ( tSettings.m_bStopwordsUnstemmed )
				fprintf ( fp, "\tstopwords_unstemmed = 1\n" );
		}

		fprintf ( fp, "}\n" );
		return;
	}

	///////////////////////////////////////////////
	// print header and stats in "readable" format
	///////////////////////////////////////////////

	fprintf ( fp, "version: %d\n",			m_uVersion );
	fprintf ( fp, "idbits: 64\n" );

	fprintf ( fp, "fields: %d\n", m_tSchema.GetFieldsCount() );
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
		fprintf ( fp, "  field %d: %s\n", i, m_tSchema.GetFieldName(i) );

	fprintf ( fp, "attrs: %d\n", m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		fprintf ( fp, "  attr %d: %s, %s", i, tAttr.m_sName.cstr(), sphTypeName ( tAttr.m_eAttrType ) );
		if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount!=32 )
			fprintf ( fp, ", bits %d", tAttr.m_tLocator.m_iBitCount );
		fprintf ( fp, ", bitoff %d\n", tAttr.m_tLocator.m_iBitOffset );
	}

	// skipped min doc, wordlist checkpoints
	fprintf ( fp, "total-documents: " INT64_FMT "\n", m_tStats.m_iTotalDocuments );
	fprintf ( fp, "total-bytes: " INT64_FMT "\n", int64_t(m_tStats.m_iTotalBytes) );

	fprintf ( fp, "min-prefix-len: %d\n", m_tSettings.m_iMinPrefixLen );
	fprintf ( fp, "min-infix-len: %d\n", m_tSettings.m_iMinInfixLen );
	fprintf ( fp, "max-substring-len: %d\n", m_tSettings.m_iMaxSubstringLen );
	fprintf ( fp, "exact-words: %d\n", m_tSettings.m_bIndexExactWords ? 1 : 0 );
	fprintf ( fp, "html-strip: %d\n", m_tSettings.m_bHtmlStrip ? 1 : 0 );
	fprintf ( fp, "html-index-attrs: %s\n", m_tSettings.m_sHtmlIndexAttrs.cstr () );
	fprintf ( fp, "html-remove-elements: %s\n", m_tSettings.m_sHtmlRemoveElements.cstr () );
	fprintf ( fp, "index-zones: %s\n", m_tSettings.m_sZones.cstr() );
	fprintf ( fp, "index-field-lengths: %d\n", m_tSettings.m_bIndexFieldLens ? 1 : 0 );
	fprintf ( fp, "index-sp: %d\n", m_tSettings.m_bIndexSP ? 1 : 0 );
	fprintf ( fp, "phrase-boundary-step: %d\n", m_tSettings.m_iBoundaryStep );
	fprintf ( fp, "stopword-step: %d\n", m_tSettings.m_iStopwordStep );
	fprintf ( fp, "overshort-step: %d\n", m_tSettings.m_iOvershortStep );
	fprintf ( fp, "bigram-index: %s\n", sphBigramName ( m_tSettings.m_eBigramIndex ) );
	fprintf ( fp, "bigram-freq-words: %s\n", m_tSettings.m_sBigramWords.cstr() );
	fprintf ( fp, "rlp-context: %s\n", m_tSettings.m_sRLPContext.cstr() );
	fprintf ( fp, "index-token-filter: %s\n", m_tSettings.m_sIndexTokenFilter.cstr() );
	CSphFieldFilterSettings tFieldFilter;
	GetFieldFilterSettings ( tFieldFilter );
	ARRAY_FOREACH ( i, tFieldFilter.m_dRegexps )
		fprintf ( fp, "regexp-filter: %s\n", tFieldFilter.m_dRegexps[i].cstr() );

	if ( m_pTokenizer )
	{
		const CSphTokenizerSettings & tSettings = m_pTokenizer->GetSettings ();
		fprintf ( fp, "tokenizer-type: %d\n", tSettings.m_iType );
		fprintf ( fp, "tokenizer-case-folding: %s\n", tSettings.m_sCaseFolding.cstr () );
		fprintf ( fp, "tokenizer-min-word-len: %d\n", tSettings.m_iMinWordLen );
		fprintf ( fp, "tokenizer-ngram-chars: %s\n", tSettings.m_sNgramChars.cstr () );
		fprintf ( fp, "tokenizer-ngram-len: %d\n", tSettings.m_iNgramLen );
		fprintf ( fp, "tokenizer-exceptions: %s\n", tSettings.m_sSynonymsFile.cstr () );
		fprintf ( fp, "tokenizer-phrase-boundary: %s\n", tSettings.m_sBoundary.cstr () );
		fprintf ( fp, "tokenizer-ignore-chars: %s\n", tSettings.m_sIgnoreChars.cstr () );
		fprintf ( fp, "tokenizer-blend-chars: %s\n", tSettings.m_sBlendChars.cstr () );
		fprintf ( fp, "tokenizer-blend-mode: %s\n", tSettings.m_sBlendMode.cstr () );
		fprintf ( fp, "tokenizer-blend-mode: %s\n", tSettings.m_sBlendMode.cstr () );

		fprintf ( fp, "dictionary-embedded-exceptions: %d\n", tEmbeddedFiles.m_bEmbeddedSynonyms ? 1 : 0 );
		if ( tEmbeddedFiles.m_bEmbeddedSynonyms )
		{
			ARRAY_FOREACH ( i, tEmbeddedFiles.m_dSynonyms )
				fprintf ( fp, "\tdictionary-embedded-exception [%d]: %s\n", i, tEmbeddedFiles.m_dSynonyms[i].cstr () );
		}
	}

	if ( m_pDict )
	{
		const CSphDictSettings & tSettings = m_pDict->GetSettings ();
		fprintf ( fp, "dict: %s\n", tSettings.m_bWordDict ? "keywords" : "crc" );
		fprintf ( fp, "dictionary-morphology: %s\n", tSettings.m_sMorphology.cstr () );

		fprintf ( fp, "dictionary-stopwords-file: %s\n", tSettings.m_sStopwords.cstr () );
		fprintf ( fp, "dictionary-embedded-stopwords: %d\n", tEmbeddedFiles.m_bEmbeddedStopwords ? 1 : 0 );
		if ( tEmbeddedFiles.m_bEmbeddedStopwords )
		{
			ARRAY_FOREACH ( i, tEmbeddedFiles.m_dStopwords )
				fprintf ( fp, "\tdictionary-embedded-stopword [%d]: " UINT64_FMT "\n", i, tEmbeddedFiles.m_dStopwords[i] );
		}

		ARRAY_FOREACH ( i, tSettings.m_dWordforms )
			fprintf ( fp, "dictionary-wordform-file [%d]: %s\n", i, tSettings.m_dWordforms[i].cstr () );

		fprintf ( fp, "dictionary-embedded-wordforms: %d\n", tEmbeddedFiles.m_bEmbeddedWordforms ? 1 : 0 );
		if ( tEmbeddedFiles.m_bEmbeddedWordforms )
		{
			ARRAY_FOREACH ( i, tEmbeddedFiles.m_dWordforms )
				fprintf ( fp, "\tdictionary-embedded-wordform [%d]: %s\n", i, tEmbeddedFiles.m_dWordforms[i].cstr () );
		}

		fprintf ( fp, "min-stemming-len: %d\n", tSettings.m_iMinStemmingLen );
		fprintf ( fp, "stopwords-unstemmed: %d\n", tSettings.m_bStopwordsUnstemmed ? 1 : 0 );
	}

	for ( const auto & i : m_tSettings.m_dKlistTargets )
		fprintf ( fp, "killlist-target: %s %u\n", i.m_sIndex.cstr(), i.m_uFlags );

	fprintf ( fp, "min-max-index: " INT64_FMT "\n", m_iMinMaxIndex );
}


void CSphIndex_VLN::DebugDumpDocids ( FILE * fp )
{
	const int iRowStride = m_tSchema.GetRowSize();

	const int64_t iNumMinMaxRow = (m_iDocinfoIndex+1)*iRowStride*2;
	const int64_t iNumRows = m_iDocinfo;

	const int64_t iDocinfoSize = iRowStride*m_iDocinfo*sizeof(DWORD);
	const int64_t iMinmaxSize = iNumMinMaxRow*sizeof(CSphRowitem);

	fprintf ( fp, "docinfo-bytes: docinfo=" INT64_FMT ", min-max=" INT64_FMT ", total=" UINT64_FMT "\n", iDocinfoSize, iMinmaxSize, (uint64_t)m_tAttr.GetLengthBytes() );
	fprintf ( fp, "docinfo-stride: %d\n", (int)(iRowStride*sizeof(DWORD)) );
	fprintf ( fp, "docinfo-rows: " INT64_FMT "\n", iNumRows );

	if ( !m_tAttr.GetLength64() )
		return;

	DWORD * pDocinfo = m_tAttr.GetWritePtr();
	for ( int64_t iRow=0; iRow<iNumRows; iRow++, pDocinfo+=iRowStride )
		printf ( INT64_FMT". id=" INT64_FMT "\n", iRow+1, sphGetDocID ( pDocinfo ) );

	printf ( "--- min-max=" INT64_FMT " ---\n", iNumMinMaxRow );
	for ( int64_t iRow=0; iRow<(m_iDocinfoIndex+1)*2; iRow++, pDocinfo+=iRowStride )
		printf ( "id=" INT64_FMT "\n", sphGetDocID ( pDocinfo ) );
}


void CSphIndex_VLN::DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID )
{
	WITH_QWORD ( this, false, Qword, DumpHitlist<Qword> ( fp, sKeyword, bID ) );
}


template < class Qword >
void CSphIndex_VLN::DumpHitlist ( FILE * fp, const char * sKeyword, bool bID )
{
	// get keyword id
	SphWordID_t uWordID = 0;
	BYTE * sTok = NULL;
	if ( !bID )
	{
		CSphString sBuf ( sKeyword );

		m_pTokenizer->SetBuffer ( (BYTE*)sBuf.cstr(), strlen ( sBuf.cstr() ) );
		sTok = m_pTokenizer->GetToken();

		if ( !sTok )
			sphDie ( "keyword=%s, no token (too short?)", sKeyword );

		uWordID = m_pDict->GetWordID ( sTok );
		if ( !uWordID )
			sphDie ( "keyword=%s, tok=%s, no wordid (stopped?)", sKeyword, sTok );

		fprintf ( fp, "keyword=%s, tok=%s, wordid=" UINT64_FMT "\n", sKeyword, sTok, uint64_t(uWordID) );

	} else
	{
		uWordID = (SphWordID_t) strtoull ( sKeyword, NULL, 10 );
		if ( !uWordID )
			sphDie ( "failed to convert keyword=%s to id (must be integer)", sKeyword );

		fprintf ( fp, "wordid=" UINT64_FMT "\n", uint64_t(uWordID) );
	}

	// open files
	DataReaderFabricPtr_t pDoclist {
		NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), m_sLastError, DataReaderFabric_c::eDocs, m_tFiles.m_iReadBufferDocList, true )
	};
	if ( !pDoclist )
		sphDie ( "failed to open doclist: %s", m_sLastError.cstr() );

	DataReaderFabricPtr_t pHitlist {
		NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), m_sLastError, DataReaderFabric_c::eHits, m_tFiles.m_iReadBufferHitList, true )
	};
	if ( !pHitlist )
		sphDie ( "failed to open hitlist: %s", m_sLastError.cstr ());


	// aim
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist,
		m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize );
	tTermSetup.SetDict ( m_pDict );
	tTermSetup.m_pIndex = this;
	tTermSetup.m_bSetupReaders = true;

	Qword tKeyword ( false, false );
	tKeyword.m_uWordID = uWordID;
	tKeyword.m_sWord = sKeyword;
	tKeyword.m_sDictWord = (const char *)sTok;
	if ( !tTermSetup.QwordSetup ( &tKeyword ) )
		sphDie ( "failed to setup keyword" );

	// press play on tape
	while (true)
	{
		tKeyword.GetNextDoc();
		if ( tKeyword.m_tDoc.m_tRowID==INVALID_ROWID )
			break;

		tKeyword.SeekHitlist ( tKeyword.m_iHitlistPos );

		int iHits = 0;
		if ( tKeyword.m_bHasHitlist )
			for ( Hitpos_t uHit = tKeyword.GetNextHit(); uHit!=EMPTY_HIT; uHit = tKeyword.GetNextHit() )
			{
				fprintf ( fp, "doc=%u, hit=0x%08x\n", tKeyword.m_tDoc.m_tRowID, uHit );
				iHits++;
			}

		if ( !iHits )
		{
			uint64_t uOff = tKeyword.m_iHitlistPos;
			fprintf ( fp, "doc=%u, NO HITS, inline=%d, off=" UINT64_FMT "\n", tKeyword.m_tDoc.m_tRowID, (int)(uOff>>63), (uOff<<1)>>1 );
		}
	}
}


void CSphIndex_VLN::DebugDumpDict ( FILE * fp )
{
	if ( !m_pDict->GetSettings().m_bWordDict )
	{
		sphDie ( "DebugDumpDict() only supports dict=keywords for now" );
	}

	fprintf ( fp, "keyword,docs,hits,offset\n" );
	m_tWordlist.DebugPopulateCheckpoints();
	ARRAY_FOREACH ( i, m_tWordlist.m_dCheckpoints )
	{
		KeywordsBlockReader_c tCtx ( m_tWordlist.AcquireDict ( &m_tWordlist.m_dCheckpoints[i] ), m_tSettings.m_iSkiplistBlockSize );
		while ( tCtx.UnpackWord() )
			fprintf ( fp, "%s,%d,%d," INT64_FMT "\n", tCtx.GetWord(), tCtx.m_iDocs, tCtx.m_iHits, int64_t(tCtx.m_iDoclistOffset) );
	}
}

//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::PopulateHistograms()
{
	if ( m_pHistograms || !m_pDocinfoIndex )
		return;

	m_pHistograms = new HistogramContainer_c;

	int iStride = m_tSchema.GetRowSize();

	DWORD * pMinRow = const_cast<DWORD*> ( &m_pDocinfoIndex [ m_iDocinfoIndex*iStride*2 ] );
	DWORD * pMaxRow = pMinRow + iStride;

	CSphVector<Histogram_i *> dHistograms;
	CSphVector<CSphColumnInfo> dPOD;

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
		{
			dHistograms.Add ( pHistogram );
			dPOD.Add ( tAttr );
			m_pHistograms->Add ( pHistogram );
			pHistogram->Setup ( sphGetRowAttr ( pMinRow, tAttr.m_tLocator ), sphGetRowAttr ( pMaxRow, tAttr.m_tLocator ) );
		}
	}

	const DWORD * pAttr = m_tAttr.GetWritePtr();
	const DWORD * pAttrMax = pAttr + m_iDocinfo*iStride;

	while ( pAttr<pAttrMax )
	{
		ARRAY_FOREACH ( i, dHistograms )
			dHistograms[i]->Insert ( sphGetRowAttr ( pAttr,  dPOD[i].m_tLocator ) );

		pAttr += iStride;
	}

	CSphString sError; // ignored for now
	m_pHistograms->Save ( GetIndexFileName(SPH_EXT_SPHI), sError );
}


bool CSphIndex_VLN::Prealloc ( bool bStripPath )
{
	MEMORY ( MEM_INDEX_DISK );

	// reset
	Dealloc ();

	CSphEmbeddedFiles tEmbeddedFiles;

	// preload schema
	if ( !LoadHeader ( GetIndexFileName(SPH_EXT_SPH).cstr(), bStripPath, tEmbeddedFiles, m_sLastWarning ) )
		return false;

	tEmbeddedFiles.Reset();

	// verify that data files are readable
	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPD).cstr(), &m_sLastError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPP).cstr(), &m_sLastError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPE).cstr(), &m_sLastError ) )
		return false;

	// preopen doclist for mmap or keep files open (mmap also force open files)
	if ( m_bKeepFilesOpen || m_tFiles.m_eDoclist!=FileAccess_e::FILE )
	{
		m_pDoclistFile = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), m_sLastError,
										  DataReaderFabric_c::eDocs, m_tFiles.m_iReadBufferDocList, ( m_tFiles.m_eDoclist==FileAccess_e::FILE ) );
		if ( !m_pDoclistFile )
			return false;
	}

	// preopen hitlist for mmap or keep files open (mmap also force open files)
	if ( m_bKeepFilesOpen || m_tFiles.m_eHitlist!=FileAccess_e::FILE )
	{
		m_pHitlistFile = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), m_sLastError,
										  DataReaderFabric_c::eHits, m_tFiles.m_iReadBufferHitList, ( m_tFiles.m_eHitlist==FileAccess_e::FILE ) );
		if ( !m_pHitlistFile )
			return false;
	}

	/////////////////////
	// prealloc wordlist
	/////////////////////

	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPI).cstr(), &m_sLastError ) )
		return false;

	// might be no dictionary at this point for old index format
	bool bWordDict = m_pDict && m_pDict->GetSettings().m_bWordDict;

	// only checkpoint and wordlist infixes are actually read here; dictionary itself is just mapped
	if ( !m_tWordlist.Preread ( GetIndexFileName(SPH_EXT_SPI), bWordDict, m_tSettings.m_iSkiplistBlockSize, m_sLastError ) )
		return false;

	CSphAutofile tDocinfo ( GetIndexFileName(SPH_EXT_SPA), SPH_O_READ, m_sLastError );
	if ( tDocinfo.GetFD()<0 )
		return false;

	m_bIsEmpty = ( tDocinfo.GetSize ( 0, false, m_sLastError )==0 );

	if ( ( m_tWordlist.m_tBuf.GetLengthBytes()<=1 )!=( m_tWordlist.m_dCheckpoints.GetLength()==0 ) )
		sphWarning ( "wordlist size mismatch (size=%zu, checkpoints=%d)", m_tWordlist.m_tBuf.GetLengthBytes(), m_tWordlist.m_dCheckpoints.GetLength() );

	// make sure checkpoints are loadable
	// pre-11 indices use different offset type (this is fixed up later during the loading)
	assert ( m_tWordlist.m_iDictCheckpointsOffset>0 );

	/////////////////////
	// prealloc docinfos
	/////////////////////

	if ( !m_bIsEmpty && !m_bDebugCheck )
	{
		/////////////
		// attr data
		/////////////

		if ( !m_tAttr.Setup ( GetIndexFileName(SPH_EXT_SPA), m_sLastError, true ) )
			return false;

		if ( !CheckDocsCount ( m_iDocinfo, m_sLastError ) )
			return false;

		m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

		// MVA/string/json data
		if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		{
			if ( !m_tBlobAttrs.Setup ( GetIndexFileName(SPH_EXT_SPB), m_sLastError, true ) )
				return false;
		}

		if ( !m_tDocidLookup.Setup ( GetIndexFileName(SPH_EXT_SPT), m_sLastError, false ) )
			return false;

		m_tLookupReader.SetData ( m_tDocidLookup.GetWritePtr() );
	}

	// prealloc killlist
	if ( !m_bDebugCheck && !m_tDeadRowMap.Prealloc ( m_iDocinfo, GetIndexFileName(SPH_EXT_SPM), m_sLastError ) )
		return false;

	CSphString sHistogramFile = GetIndexFileName(SPH_EXT_SPHI);
	if ( !m_bDebugCheck && m_uVersion>=55 && sphIsReadable ( sHistogramFile.cstr() ) )
	{
		SafeDelete ( m_pHistograms );
		m_pHistograms = new HistogramContainer_c;
		if ( !m_pHistograms->Load ( sHistogramFile, m_sLastError ) )
			return false;
	}

	// prealloc skiplist
	if ( !m_bDebugCheck && !m_tSkiplists.Setup ( GetIndexFileName(SPH_EXT_SPE), m_sLastError, false ) )
		return false;

	// almost done
	m_bPassedAlloc = true;

	return true;
}


void CSphIndex_VLN::Preread()
{
	MEMORY ( MEM_INDEX_DISK );

	sphLogDebug ( "CSphIndex_VLN::Preread invoked '%s'", m_sIndexName.cstr() );

	assert ( m_bPassedAlloc );
	if ( m_bPassedRead )
		return;

	///////////////////
	// read everything
	///////////////////

	volatile BYTE uRead = 0; // just need all side-effects
	uRead ^= PrereadMapping ( m_sIndexName.cstr(), "attributes", IsMlock ( m_tFiles.m_eAttr ), IsOndisk ( m_tFiles.m_eAttr ), m_tAttr );
	uRead ^= PrereadMapping ( m_sIndexName.cstr(), "blobs", IsMlock ( m_tFiles.m_eBlob ), IsOndisk ( m_tFiles.m_eBlob ), m_tBlobAttrs );
	uRead ^= PrereadMapping ( m_sIndexName.cstr(), "skip-list", IsMlock ( m_tFiles.m_eAttr ), false, m_tSkiplists );
	uRead ^= PrereadMapping ( m_sIndexName.cstr(), "dictionary", IsMlock ( m_tFiles.m_eAttr ), false, m_tWordlist.m_tBuf );
	uRead ^= PrereadMapping ( m_sIndexName.cstr(), "docid-lookup", IsMlock ( m_tFiles.m_eAttr ), false, m_tDocidLookup );
	uRead ^= m_tDeadRowMap.Preread ( m_sIndexName.cstr(), "kill-list", IsMlock ( m_tFiles.m_eAttr ) );

	PopulateHistograms();

	m_bPassedRead = true;
	sphLogDebug ( "Preread successfully finished, hash=%u", (DWORD)uRead );
}


void CSphIndex_VLN::SetMemorySettings ( const FileAccessSettings_t & tFileAccessSettings )
{
	m_tFiles = tFileAccessSettings;
}

void CSphIndex_VLN::SetBase ( const char * sNewBase )
{
	m_sFilename = sNewBase;
}


bool CSphIndex_VLN::Rename ( const char * sNewBase )
{
	if ( m_sFilename==sNewBase )
		return true;

	IndexFiles_c dFiles ( m_sFilename, m_uVersion );
	if ( !dFiles.RenameBase ( sNewBase ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return false;
	}

	if ( !dFiles.RenameLock ( sNewBase, m_iLockFD ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return false;
	}

	SetBase ( sNewBase );

	return true;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryContext::CSphQueryContext ( const CSphQuery & q )
	: m_tQuery ( q )
{}

CSphQueryContext::~CSphQueryContext ()
{
	ResetFilters();
}

void CSphQueryContext::ResetFilters()
{
	SafeDelete ( m_pFilter );
	SafeDelete ( m_pWeightFilter );

	ARRAY_FOREACH ( i, m_dUserVals )
		m_dUserVals[i]->Release();
	m_dUserVals.Reset();
}

void CSphQueryContext::BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sWarning )
{
	const int HEAVY_FIELDS = SPH_MAX_FIELDS;

	// defaults
	m_iWeights = Min ( tSchema.GetFieldsCount(), HEAVY_FIELDS );
	for ( int i=0; i<m_iWeights; i++ )
		m_dWeights[i] = 1;

	// name-bound weights
	CSphString sFieldsNotFound;
	if ( pQuery->m_dFieldWeights.GetLength() )
	{
		ARRAY_FOREACH ( i, pQuery->m_dFieldWeights )
		{
			int j = tSchema.GetFieldIndex ( pQuery->m_dFieldWeights[i].m_sName.cstr() );
			if ( j<0 )
			{
				if ( sFieldsNotFound.IsEmpty() )
					sFieldsNotFound = pQuery->m_dFieldWeights[i].m_sName;
				else
					sFieldsNotFound.SetSprintf ( "%s %s", sFieldsNotFound.cstr(), pQuery->m_dFieldWeights[i].m_sName.cstr() );
			}

			if ( j>=0 && j<HEAVY_FIELDS )
				m_dWeights[j] = pQuery->m_dFieldWeights[i].m_iValue;
		}

		if ( !sFieldsNotFound.IsEmpty() )
			sWarning.SetSprintf ( "Fields specified in field_weights option not found: [%s]", sFieldsNotFound.cstr() );

		return;
	}

	// order-bound weights
	if ( pQuery->m_dWeights.GetLength() )
	{
		for ( int i=0; i<Min ( m_iWeights, pQuery->m_dWeights.GetLength() ); i++ )
			m_dWeights[i] = (int)pQuery->m_dWeights[i];
	}
}

static ESphEvalStage GetEarliestStage ( ESphEvalStage eStage, const CSphColumnInfo & tIn, const CSphVector<const ISphSchema *> & dSchemas )
{
	ARRAY_FOREACH ( iSchema, dSchemas )
	{
		const ISphSchema * pSchema = dSchemas[iSchema];
		const CSphColumnInfo * pCol = pSchema->GetAttr ( tIn.m_sName.cstr() );
		if ( !pCol )
			continue;

		eStage = Min ( eStage, pCol->m_eStage );
	}

	return eStage;
}

bool CSphQueryContext::SetupCalc ( CSphQueryResult * pResult, const ISphSchema & tInSchema,	const CSphSchema & tSchema, const BYTE * pBlobPool, const CSphVector<const ISphSchema *> & dInSchemas )
{
	m_dCalcFilter.Resize ( 0 );
	m_dCalcSort.Resize ( 0 );
	m_dCalcFinal.Resize ( 0 );

	// quickly verify that all my real attributes can be stashed there
	if ( tInSchema.GetAttrsCount() < tSchema.GetAttrsCount() )
	{
		pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (incount=%d, mycount=%d)",
			tInSchema.GetAttrsCount(), tSchema.GetAttrsCount() );
		return false;
	}

	// now match everyone
	for ( int iIn=0; iIn<tInSchema.GetAttrsCount(); iIn++ )
	{
		const CSphColumnInfo & tIn = tInSchema.GetAttr(iIn);

		// recalculate stage as sorters set column at earlier stage
		// FIXME!!! should we update column?
		ESphEvalStage eStage = GetEarliestStage ( tIn.m_eStage, tIn, dInSchemas );

		switch ( eStage )
		{
			case SPH_EVAL_STATIC:
			{
				// this check may significantly slow down queries with huge schema attribute count
#ifndef NDEBUG
				const CSphColumnInfo * pMy = tSchema.GetAttr ( tIn.m_sName.cstr() );
				if ( !pMy )
				{
					pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema attr missing from index-schema (in=%s)",
						sphDumpAttr(tIn).cstr() );
					return false;
				}

				// static; check for full match
				if (!( tIn==*pMy ))
				{
					assert ( 0 );
					pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (in=%s, my=%s)",
						sphDumpAttr(tIn).cstr(), sphDumpAttr(*pMy).cstr() );
					return false;
				}
#endif
				break;
			}

			case SPH_EVAL_PREFILTER:
			case SPH_EVAL_PRESORT:
			case SPH_EVAL_FINAL:
			{
				ISphExpr * pExpr = tIn.m_pExpr;
				if ( !pExpr )
				{
					pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema expression missing evaluator (stage=%d, in=%s)",
						(int)eStage, sphDumpAttr(tIn).cstr() );
					return false;
				}

				// an expression that index/searcher should compute
				CalcItem_t tCalc;
				tCalc.m_eType = tIn.m_eAttrType;
				tCalc.m_tLoc = tIn.m_tLocator;
				tCalc.m_pExpr = pExpr;
				SafeAddRef ( pExpr );
				tCalc.m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)&pBlobPool );

				switch ( eStage )
				{
					case SPH_EVAL_PREFILTER:	m_dCalcFilter.Add ( tCalc ); break;
					case SPH_EVAL_PRESORT:		m_dCalcSort.Add ( tCalc ); break;
					case SPH_EVAL_FINAL:		m_dCalcFinal.Add ( tCalc ); break;

					default:					break;
				}
				break;
			}

			case SPH_EVAL_SORTER:
				// sorter tells it will compute itself; so just skip it
			case SPH_EVAL_POSTLIMIT:
				break;

			default:
				pResult->m_sError.SetSprintf ( "INTERNAL ERROR: unhandled eval stage=%d", (int)eStage );
				return false;
		}
	}

	// ok, we can emit matches in this schema (incoming for sorter, outgoing for index/searcher)
	return true;
}


bool CSphIndex_VLN::IsStarDict () const
{
	return m_tSettings.m_iMinPrefixLen>0 || m_tSettings.m_iMinInfixLen>0;
}


void CSphIndex_VLN::SetupStarDict ( CSphDictRefPtr_c &pDict ) const
{
	// spawn wrapper, and put it in the box
	// wrapper type depends on version; v.8 introduced new mangling rules
	if ( IsStarDict() )
		pDict = new CSphDictStarV8 ( pDict, m_tSettings.m_iMinInfixLen>0 );

	// FIXME? might wanna verify somehow that the tokenizer has '*' as a character
}

void CSphIndex_VLN::SetupExactDict ( CSphDictRefPtr_c &pDict ) const
{
	if ( m_tSettings.m_bIndexExactWords )
		pDict = new CSphDictExact ( pDict );
}


bool CSphIndex_VLN::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords,
	const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const
{
	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, szQuery, tSettings, false, pError ) );
	return false;
}

void CSphIndex_VLN::GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const
{
	if ( m_tWordlist.m_tBuf.IsEmpty() || !m_tWordlist.m_dCheckpoints.GetLength() )
		return;

	assert ( !tRes.m_pWordReader );
	tRes.m_pWordReader = new KeywordsBlockReader_c ( m_tWordlist.m_tBuf.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize );
	tRes.m_bHasExactDict = m_tSettings.m_bIndexExactWords;

	sphGetSuggest ( &m_tWordlist, m_tWordlist.m_iInfixCodepointBytes, tArgs, tRes );

	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	SafeDelete ( pReader );
	tRes.m_pWordReader = NULL;
}


DWORD sphParseMorphAot ( const char * sMorphology )
{
	if ( !sMorphology || !*sMorphology )
		return 0;

	StrVec_t dMorphs;
	sphSplit ( dMorphs, sMorphology );

	DWORD uAotFilterMask = 0;
	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf_all[20];
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT
		ARRAY_FOREACH ( i, dMorphs )
		{
			if ( dMorphs[i]==buf_all )
			{
				uAotFilterMask |= (1UL) << j;
				break;
			}
		}
	}

	return uAotFilterMask;
}


void ISphQueryFilter::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const ExpansionContext_t & tCtx )
{
	assert ( m_pTokenizer && m_pDict && m_pSettings );

	BYTE sTokenized[3*SPH_MAX_WORD_LEN+4];
	BYTE * sWord;
	int iQpos = 1;
	CSphVector<int> dQposWildcards;

	// FIXME!!! got rid of duplicated term stat and qword setup
	while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		const BYTE * sMultiform = m_pTokenizer->GetTokenizedMultiform();
		strncpy ( (char *)sTokenized, sMultiform ? (const char*)sMultiform : (const char*)sWord, sizeof(sTokenized)-1 );

		if ( ( !m_tFoldSettings.m_bFoldWildcards || m_tFoldSettings.m_bStats ) && sphHasExpandableWildcards ( (const char *)sWord ) )
		{
			dQposWildcards.Add ( iQpos );

			ISphWordlist::Args_t tWordlist ( false, tCtx.m_iExpansionLimit, tCtx.m_bHasExactForms, tCtx.m_eHitless, tCtx.m_pIndexData );
			bool bExpanded = sphExpandGetWords ( (const char *)sWord, tCtx, tWordlist );

			int iDocs = 0;
			int iHits = 0;

			// might fold wildcards but still want to sum up stats
			if ( m_tFoldSettings.m_bFoldWildcards && m_tFoldSettings.m_bStats )
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					iDocs += tWordlist.m_dExpanded[i].m_iDocs;
					iHits += tWordlist.m_dExpanded[i].m_iHits;
				}
				bExpanded = false;
			} else
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					CSphKeywordInfo & tInfo = dKeywords.Add();
					tInfo.m_sTokenized = (const char *)sWord;
					tInfo.m_sNormalized = tWordlist.GetWordExpanded ( i );
					tInfo.m_iDocs = tWordlist.m_dExpanded[i].m_iDocs;
					tInfo.m_iHits = tWordlist.m_dExpanded[i].m_iHits;
					tInfo.m_iQpos = iQpos;

					RemoveDictSpecials ( tInfo.m_sNormalized );
				}
			}

			if ( !bExpanded || !tWordlist.m_dExpanded.GetLength() )
			{
				CSphKeywordInfo & tInfo = dKeywords.Add ();
				tInfo.m_sTokenized = (const char *)sWord;
				tInfo.m_sNormalized = (const char *)sWord;
				tInfo.m_iDocs = iDocs;
				tInfo.m_iHits = iHits;
				tInfo.m_iQpos = iQpos;
			}
		} else
		{
			AddKeywordStats ( sWord, sTokenized, iQpos, dKeywords );
		}

		// FIXME!!! handle consecutive blended wo blended parts
		bool bBlended = m_pTokenizer->TokenIsBlended();

		if ( bBlended )
		{
			if ( m_tFoldSettings.m_bFoldBlended )
				iQpos += m_pTokenizer->SkipBlended();
		} else
		{
			iQpos++;
		}
	}

	if ( !m_pSettings->m_uAotFilterMask )
		return;

	XQLimitSpec_t tSpec;
	BYTE sTmp[3*SPH_MAX_WORD_LEN+4];
	BYTE sTmp2[3*SPH_MAX_WORD_LEN+4];
	CSphVector<XQNode_t *> dChildren ( 64 );

	CSphBitvec tSkipTransform;
	if ( dQposWildcards.GetLength () )
	{
		tSkipTransform.Init ( iQpos+1 );
		ARRAY_FOREACH ( i, dQposWildcards )
			tSkipTransform.BitSet ( dQposWildcards[i] );
	}

	int iTokenizedTotal = dKeywords.GetLength();
	for ( int iTokenized=0; iTokenized<iTokenizedTotal; iTokenized++ )
	{
		int iKeywordQpos = dKeywords[iTokenized].m_iQpos;

		// do not transform expanded wild-cards
		if ( !tSkipTransform.IsEmpty() && tSkipTransform.BitGet ( iKeywordQpos ) )
			continue;

		// MUST copy as Dict::GetWordID changes word and might add symbols
		strncpy ( (char *)sTokenized, dKeywords[iTokenized].m_sNormalized.scstr(), sizeof(sTokenized)-1 );
		int iPreAotCount = dKeywords.GetLength();

		XQNode_t tAotNode ( tSpec );
		tAotNode.m_dWords.Resize ( 1 );
		tAotNode.m_dWords.Begin()->m_sWord = (char *)sTokenized;
		TransformAotFilter ( &tAotNode, m_pDict->GetWordforms(), *m_pSettings );

		dChildren.Resize ( 0 );
		dChildren.Add ( &tAotNode );

		// recursion unfolded
		ARRAY_FOREACH ( iChild, dChildren )
		{
			// process all words at node
			ARRAY_FOREACH ( iAotKeyword, dChildren[iChild]->m_dWords )
			{
				// MUST copy as Dict::GetWordID changes word and might add symbols
				strncpy ( (char *)sTmp, dChildren[iChild]->m_dWords[iAotKeyword].m_sWord.scstr(), sizeof(sTmp)-1 );
				// prevent use-after-free-bug due to vector grow: AddKeywordsStats() calls dKeywords.Add()
				strncpy ( (char *)sTmp2, dKeywords[iTokenized].m_sTokenized.scstr (), sizeof ( sTmp2 )-1 );
				AddKeywordStats ( sTmp, sTmp2, iKeywordQpos, dKeywords );
			}

			// push all child nodes at node to process list
			const XQNode_t * pChild = dChildren[iChild];
			ARRAY_FOREACH ( iRec, pChild->m_dChildren )
				dChildren.Add ( pChild->m_dChildren[iRec] );
		}

		bool bGotLemmas = ( iPreAotCount!=dKeywords.GetLength() );

		// remove (replace) original word in case of AOT taken place
		if ( bGotLemmas )
		{
			if ( !m_tFoldSettings.m_bFoldLemmas )
			{
				::Swap ( dKeywords[iTokenized], dKeywords.Last() );
				dKeywords.Resize ( dKeywords.GetLength()-1 );
			} else
			{
				int iKeywordWithMaxHits = iPreAotCount;
				for ( int i=iPreAotCount+1; i<dKeywords.GetLength(); i++ )
					if ( dKeywords[i].m_iHits > dKeywords[iKeywordWithMaxHits].m_iHits )
						iKeywordWithMaxHits = i;

				CSphString sNormalizedWithMaxHits = dKeywords[iKeywordWithMaxHits].m_sNormalized;

				int iDocs = 0;
				int iHits = 0;
				if ( m_tFoldSettings.m_bStats )
				{
					for ( int i=iPreAotCount; i<dKeywords.GetLength(); i++ )
					{
						iDocs += dKeywords[i].m_iDocs;
						iHits += dKeywords[i].m_iHits;
					}
				}
				::Swap ( dKeywords[iTokenized], dKeywords[iPreAotCount] );
				dKeywords.Resize ( iPreAotCount );
				dKeywords[iTokenized].m_iDocs = iDocs;
				dKeywords[iTokenized].m_iHits = iHits;
				dKeywords[iTokenized].m_sNormalized = sNormalizedWithMaxHits;

				RemoveDictSpecials ( dKeywords[iTokenized].m_sNormalized );
			}
		}
	}

	// sort by qpos
	if ( dKeywords.GetLength()!=iTokenizedTotal )
		sphSort ( dKeywords.Begin(), dKeywords.GetLength(), bind ( &CSphKeywordInfo::m_iQpos ) );
}


struct CSphPlainQueryFilter : public ISphQueryFilter
{
	const ISphQwordSetup *	m_pTermSetup;
	ISphQword *				m_pQueryWord;

	virtual void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords )
	{
		assert ( !m_tFoldSettings.m_bStats || ( m_pTermSetup && m_pQueryWord ) );

		SphWordID_t iWord = m_pDict->GetWordID ( sWord );
		if ( !iWord )
			return;

		if ( m_tFoldSettings.m_bStats )
		{
			m_pQueryWord->Reset ();
			m_pQueryWord->m_sWord = (const char*)sWord;
			m_pQueryWord->m_sDictWord = (const char*)sWord;
			m_pQueryWord->m_uWordID = iWord;
			m_pTermSetup->QwordSetup ( m_pQueryWord );
		}

		CSphKeywordInfo & tInfo = dKeywords.Add();
		tInfo.m_sTokenized = (const char *)sTokenized;
		tInfo.m_sNormalized = (const char*)sWord;
		tInfo.m_iDocs = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iDocs : 0;
		tInfo.m_iHits = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iHits : 0;
		tInfo.m_iQpos = iQpos;

		RemoveDictSpecials ( tInfo.m_sNormalized );
	}
};


template < class Qword >
bool CSphIndex_VLN::DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords,
	const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const
{
	if ( !bFillOnly )
		dKeywords.Resize ( 0 );

	if ( !m_bPassedAlloc )
	{
		if ( pError )
			*pError = "index not preread";
		return false;
	}

	// short-cut if no query or keywords to fill
	if ( ( bFillOnly && !dKeywords.GetLength() ) || ( !bFillOnly && ( !szQuery || !szQuery[0] ) ) )
		return true;

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	ISphFieldFilterRefPtr_c pFieldFilter;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, strlen ( (char*)sModifiedQuery ), dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// FIXME!!! missed bigram, add flags to fold blended parts, show expanded terms

	// prepare for setup
	DataReaderFabric_c * tDummy1 = nullptr;
	DataReaderFabric_c * tDummy2 = nullptr;

	DiskIndexQwordSetup_c tTermSetup ( tDummy1, tDummy2,
		m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize );
	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;

	Qword tQueryWord ( false, false );

	if ( bFillOnly )
	{
		BYTE sWord[MAX_KEYWORD_BYTES];

		ARRAY_FOREACH ( i, dKeywords )
		{
			CSphKeywordInfo &tInfo = dKeywords[i];
			int iLen = tInfo.m_sTokenized.Length ();
			memcpy ( sWord, tInfo.m_sTokenized.cstr (), iLen );
			sWord[iLen] = '\0';

			SphWordID_t iWord = pDict->GetWordID ( sWord );
			if ( iWord )
			{
				tQueryWord.Reset ();
				tQueryWord.m_sWord = tInfo.m_sTokenized;
				tQueryWord.m_sDictWord = ( const char * ) sWord;
				tQueryWord.m_uWordID = iWord;
				tTermSetup.QwordSetup ( &tQueryWord );

				tInfo.m_iDocs += tQueryWord.m_iDocs;
				tInfo.m_iHits += tQueryWord.m_iHits;
			}
		}
		return true;
	}

	ISphTokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_INDEX ) };
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually
	if ( IsStarDict () )
		pTokenizer->AddPlainChar ( '*' );
	if ( m_tSettings.m_bIndexExactWords )
		pTokenizer->AddPlainChar ( '=' );

	ExpansionContext_t tExpCtx;
	// query defined options
	tExpCtx.m_iExpansionLimit = ( tSettings.m_iExpansionLimit ? tSettings.m_iExpansionLimit : m_iExpansionLimit );
	bool bExpandWildcards = ( pDict->GetSettings ().m_bWordDict && IsStarDict() && !tSettings.m_bFoldWildcards );

	CSphPlainQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = pTokenizer;
	tAotFilter.m_pDict = pDict;
	tAotFilter.m_pSettings = &m_tSettings;
	tAotFilter.m_pTermSetup = &tTermSetup;
	tAotFilter.m_pQueryWord = &tQueryWord;
	tAotFilter.m_tFoldSettings = tSettings;
	tAotFilter.m_tFoldSettings.m_bFoldWildcards = !bExpandWildcards;

	tExpCtx.m_pWordlist = &m_tWordlist;
	tExpCtx.m_iMinPrefixLen = m_tSettings.m_iMinPrefixLen;
	tExpCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
	tExpCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
	tExpCtx.m_bMergeSingles = false;
	tExpCtx.m_eHitless = m_tSettings.m_eHitless;

	pTokenizer->SetBuffer ( sModifiedQuery, strlen ( (const char *)sModifiedQuery) );

	tAotFilter.GetKeywords ( dKeywords, tExpCtx );
	return true;
}


bool CSphIndex_VLN::FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const
{
	GetKeywordsSettings_t tSettings;
	tSettings.m_bStats = true;

	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, NULL, tSettings, true, NULL ) );
	return false;
}


// fix MSVC 2005 fuckup, template DoGetKeywords() just above somehow resets forScope
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif


bool CSphQueryContext::CreateFilters ( CreateFilterContext_t &tCtx, CSphString &sError, CSphString &sWarning )
{
	if ( !tCtx.m_pFilters || tCtx.m_pFilters->IsEmpty () )
		return true;
	if ( !sphCreateFilters ( tCtx, sError, sWarning ) )
		return false;

	m_pFilter = tCtx.m_pFilter;
	m_pWeightFilter = tCtx.m_pWeightFilter;
	m_dUserVals.SwapData ( tCtx.m_dUserVals );
	tCtx.m_pFilter = nullptr;
	tCtx.m_pWeightFilter = nullptr;

	return true;
}


static int sphQueryHeightCalc ( const XQNode_t * pNode )
{
	if ( !pNode->m_dChildren.GetLength() )
	{
		// exception, pre-cached OR of tiny (rare) keywords is just one node
		if ( pNode->GetOp()==SPH_QUERY_OR )
		{
#ifndef NDEBUG
			// sanity checks
			// this node must be only created for a huge OR of tiny expansions
			assert ( pNode->m_dWords.GetLength() );
			ARRAY_FOREACH ( i, pNode->m_dWords )
			{
				assert ( pNode->m_dWords[i].m_iAtomPos==pNode->m_dWords[0].m_iAtomPos );
				assert ( pNode->m_dWords[i].m_bExpanded );
			}
#endif
			return 1;
		}
		return pNode->m_dWords.GetLength();
	}

	if ( pNode->GetOp()==SPH_QUERY_BEFORE )
		return 1;

	int iMaxChild = 0;
	int iHeight = 0;
	ARRAY_FOREACH ( i, pNode->m_dChildren )
	{
		int iBottom = sphQueryHeightCalc ( pNode->m_dChildren[i] );
		int iTop = pNode->m_dChildren.GetLength()-i-1;
		if ( iBottom+iTop>=iMaxChild+iHeight )
		{
			iMaxChild = iBottom;
			iHeight = iTop;
		}
	}

	return iMaxChild+iHeight;
}

#define SPH_EXTNODE_STACK_SIZE 160

bool sphCheckQueryHeight ( const XQNode_t * pRoot, CSphString & sError )
{
	int iHeight = 0;
	if ( pRoot )
		iHeight = sphQueryHeightCalc ( pRoot );

	int64_t iQueryStack = sphGetStackUsed() + iHeight*SPH_EXTNODE_STACK_SIZE;
	bool bValid = ( g_iThreadStackSize>=iQueryStack );
	if ( !bValid )
		sError.SetSprintf ( "query too complex, not enough stack (thread_stack=%dK or higher required)",
			(int)( ( iQueryStack + 1024 - ( iQueryStack%1024 ) ) / 1024 ) );
	return bValid;
}

static XQNode_t * CloneKeyword ( const XQNode_t * pNode )
{
	assert ( pNode );

	XQNode_t * pRes = new XQNode_t ( pNode->m_dSpec );
	pRes->m_dWords = pNode->m_dWords;
	return pRes;
}


static XQNode_t * ExpandKeyword ( XQNode_t * pNode, const CSphIndexSettings & tSettings, int iExpandKeywords )
{
	assert ( pNode );

	if ( tSettings.m_bIndexExactWords && ( iExpandKeywords & KWE_MORPH_NONE )==KWE_MORPH_NONE )
	{
		pNode->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		return pNode;
	}

	XQNode_t * pExpand = new XQNode_t ( pNode->m_dSpec );
	pExpand->SetOp ( SPH_QUERY_OR, pNode );

	if ( tSettings.m_iMinInfixLen>0 && ( iExpandKeywords & KWE_STAR )==KWE_STAR )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pInfix = CloneKeyword ( pNode );
		pInfix->m_dWords[0].m_sWord.SetSprintf ( "*%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pInfix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pInfix );
	} else if ( tSettings.m_iMinPrefixLen>0 && ( iExpandKeywords & KWE_STAR )==KWE_STAR )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pPrefix = CloneKeyword ( pNode );
		pPrefix->m_dWords[0].m_sWord.SetSprintf ( "%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pPrefix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pPrefix );
	}

	if ( tSettings.m_bIndexExactWords && ( iExpandKeywords & KWE_EXACT )==KWE_EXACT )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pExact = CloneKeyword ( pNode );
		pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		pExact->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pExact );
	}

	return pExpand;
}

XQNode_t * sphQueryExpandKeywords ( XQNode_t * pNode, const CSphIndexSettings & tSettings, int iExpandKeywords )
{
	// only if expansion makes sense at all
	assert ( tSettings.m_iMinInfixLen>0 || tSettings.m_iMinPrefixLen>0 || tSettings.m_bIndexExactWords );
	assert ( iExpandKeywords!=KWE_DISABLED );

	// process children for composite nodes
	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			pNode->m_dChildren[i] = sphQueryExpandKeywords ( pNode->m_dChildren[i], tSettings, iExpandKeywords );
			pNode->m_dChildren[i]->m_pParent = pNode;
		}
		return pNode;
	}

	// if that's a phrase/proximity node, create a very special, magic phrase/proximity node
	if ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM )
	{
		assert ( pNode->m_dWords.GetLength()>1 );
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pWord = new XQNode_t ( pNode->m_dSpec );
			pWord->m_dWords.Add ( pNode->m_dWords[i] );
			pNode->m_dChildren.Add ( ExpandKeyword ( pWord, tSettings, iExpandKeywords ) );
			pNode->m_dChildren.Last()->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Last()->m_pParent = pNode;
		}
		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return pNode;
	}

	// skip empty plain nodes
	if ( pNode->m_dWords.GetLength()<=0 )
		return pNode;

	// process keywords for plain nodes
	assert ( pNode->m_dWords.GetLength()==1 );

	XQKeyword_t & tKeyword = pNode->m_dWords[0];
	if ( tKeyword.m_sWord.Begins("=")
		|| tKeyword.m_sWord.Begins("*")
		|| tKeyword.m_sWord.Ends("*") )
	{
		return pNode;
	}

	// do the expansion
	return ExpandKeyword ( pNode, tSettings, iExpandKeywords );
}


// transform the "one two three"/1 quorum into one|two|three (~40% faster)
static void TransformQuorum ( XQNode_t ** ppNode )
{
	XQNode_t *& pNode = *ppNode;

	// recurse non-quorum nodes
	if ( pNode->GetOp()!=SPH_QUERY_QUORUM )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TransformQuorum ( &pNode->m_dChildren[i] );
		return;
	}

	// skip quorums with thresholds other than 1
	if ( pNode->m_iOpArg!=1 )
		return;

	// transform quorums with a threshold of 1 only
	assert ( pNode->GetOp()==SPH_QUERY_QUORUM && pNode->m_dChildren.GetLength()==0 );
	CSphVector<XQNode_t*> dArgs;
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		XQNode_t * pAnd = new XQNode_t ( pNode->m_dSpec );
		pAnd->m_dWords.Add ( pNode->m_dWords[i] );
		dArgs.Add ( pAnd );
	}
	pNode->m_dWords.Reset();
	pNode->SetOp ( SPH_QUERY_OR, dArgs );
}


struct BinaryNode_t
{
	int m_iLo;
	int m_iHi;
};

static void BuildExpandedTree ( const XQKeyword_t & tRootWord, ISphWordlist::Args_t & dWordSrc, XQNode_t * pRoot )
{
	assert ( dWordSrc.m_dExpanded.GetLength() );
	pRoot->m_dWords.Reset();

	// build a binary tree from all the other expansions
	CSphVector<BinaryNode_t> dNodes;
	dNodes.Reserve ( dWordSrc.m_dExpanded.GetLength() );

	XQNode_t * pCur = pRoot;

	dNodes.Add();
	dNodes.Last().m_iLo = 0;
	dNodes.Last().m_iHi = ( dWordSrc.m_dExpanded.GetLength()-1 );

	while ( dNodes.GetLength() )
	{
		BinaryNode_t tNode = dNodes.Pop();
		if ( tNode.m_iHi<tNode.m_iLo )
		{
			pCur = pCur->m_pParent;
			continue;
		}

		int iMid = ( tNode.m_iLo+tNode.m_iHi ) / 2;
		dNodes.Add ();
		dNodes.Last().m_iLo = tNode.m_iLo;
		dNodes.Last().m_iHi = iMid-1;
		dNodes.Add ();
		dNodes.Last().m_iLo = iMid+1;
		dNodes.Last().m_iHi = tNode.m_iHi;

		if ( pCur->m_dWords.GetLength() )
		{
			assert ( pCur->m_dWords.GetLength()==1 );
			XQNode_t * pTerm = CloneKeyword ( pRoot );
			Swap ( pTerm->m_dWords, pCur->m_dWords );
			pCur->m_dChildren.Add ( pTerm );
			pTerm->m_pParent = pCur;
		}

		XQNode_t * pChild = CloneKeyword ( pRoot );
		pChild->m_dWords.Add ( tRootWord );
		pChild->m_dWords.Last().m_sWord = dWordSrc.GetWordExpanded ( iMid );
		pChild->m_dWords.Last().m_bExpanded = true;
		pChild->m_bNotWeighted = pRoot->m_bNotWeighted;

		pChild->m_pParent = pCur;
		pCur->m_dChildren.Add ( pChild );
		pCur->SetOp ( SPH_QUERY_OR );

		pCur = pChild;
	}
}


/// do wildcard expansion for keywords dictionary
/// (including prefix and infix expansion)
XQNode_t * sphExpandXQNode ( XQNode_t * pNode, ExpansionContext_t & tCtx )
{
	assert ( pNode );
	assert ( tCtx.m_pResult );

	// process children for composite nodes
	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			pNode->m_dChildren[i] = sphExpandXQNode ( pNode->m_dChildren[i], tCtx );
			pNode->m_dChildren[i]->m_pParent = pNode;
		}
		return pNode;
	}

	// if that's a phrase/proximity node, create a very special, magic phrase/proximity node
	if ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM )
	{
		assert ( pNode->m_dWords.GetLength()>1 );
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pWord = new XQNode_t ( pNode->m_dSpec );
			pWord->m_dWords.Add ( pNode->m_dWords[i] );
			pNode->m_dChildren.Add ( sphExpandXQNode ( pWord, tCtx ) );
			pNode->m_dChildren.Last()->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Last()->m_pParent = pNode;

			// tricky part
			// current node may have field/zone limits attached
			// normally those get pushed down during query parsing
			// but here we create nodes manually and have to push down limits too
			pWord->CopySpecs ( pNode );
		}
		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return pNode;
	}

	// skip empty plain nodes
	if ( pNode->m_dWords.GetLength()<=0 )
		return pNode;

	// process keywords for plain nodes
	assert ( pNode->m_dChildren.GetLength()==0 );
	assert ( pNode->m_dWords.GetLength()==1 );

	// check the wildcards
	const char * sFull = pNode->m_dWords[0].m_sWord.cstr();

	// no wildcards, or just wildcards? do not expand
	if ( !sphHasExpandableWildcards ( sFull ) )
		return pNode;

	bool bUseTermMerge = ( tCtx.m_bMergeSingles && pNode->m_dSpec.m_dZones.GetLength()==0 );
	ISphWordlist::Args_t tWordlist ( bUseTermMerge, tCtx.m_iExpansionLimit, tCtx.m_bHasExactForms, tCtx.m_eHitless, tCtx.m_pIndexData );

	if ( !sphExpandGetWords ( sFull, tCtx, tWordlist ) )
	{
		tCtx.m_pResult->m_sWarning.SetSprintf ( "Query word length is less than min %s length. word: '%s' ", ( tCtx.m_iMinInfixLen>0 ? "infix" : "prefix" ), sFull );
		return pNode;
	}

	// no real expansions?
	// mark source word as expanded to prevent warning on terms mismatch in statistics
	if ( !tWordlist.m_dExpanded.GetLength() && !tWordlist.m_pPayload )
	{
		tCtx.m_pResult->AddStat ( pNode->m_dWords.Begin()->m_sWord, 0, 0 );
		pNode->m_dWords.Begin()->m_bExpanded = true;
		return pNode;
	}

	// copy the original word (iirc it might get overwritten),
	const XQKeyword_t tRootWord = pNode->m_dWords[0];
	tCtx.m_pResult->AddStat ( tRootWord.m_sWord, tWordlist.m_iTotalDocs, tWordlist.m_iTotalHits );

	// and build a binary tree of all the expansions
	if ( tWordlist.m_dExpanded.GetLength() )
	{
		BuildExpandedTree ( tRootWord, tWordlist, pNode );
	}

	if ( tWordlist.m_pPayload )
	{
		ISphSubstringPayload * pPayload = tWordlist.m_pPayload;
		tWordlist.m_pPayload = NULL;
		tCtx.m_pPayloads->Add ( pPayload );

		if ( pNode->m_dWords.GetLength() )
		{
			// all expanded fit to single payload
			pNode->m_dWords.Begin()->m_bExpanded = true;
			pNode->m_dWords.Begin()->m_pPayload = pPayload;
		} else
		{
			// payload added to expanded binary tree
			assert ( pNode->GetOp()==SPH_QUERY_OR );
			assert ( pNode->m_dChildren.GetLength() );

			XQNode_t * pSubstringNode = new XQNode_t ( pNode->m_dSpec );
			pSubstringNode->SetOp ( SPH_QUERY_OR );

			XQKeyword_t tSubstringWord = tRootWord;
			tSubstringWord.m_bExpanded = true;
			tSubstringWord.m_pPayload = pPayload;
			pSubstringNode->m_dWords.Add ( tSubstringWord );

			pNode->m_dChildren.Add ( pSubstringNode );
			pSubstringNode->m_pParent = pNode;
		}
	}

	return pNode;
}


bool sphHasExpandableWildcards ( const char * sWord )
{
	const char * pCur = sWord;
	int iWilds = 0;

	for ( ; *pCur; pCur++ )
		if ( sphIsWild ( *pCur ) )
			iWilds++;

	int iLen = pCur - sWord;

	return ( iWilds && iWilds<iLen );
}

bool sphExpandGetWords ( const char * sWord, const ExpansionContext_t & tCtx, ISphWordlist::Args_t & tWordlist )
{
	if ( !sphIsWild ( *sWord ) || tCtx.m_iMinInfixLen==0 )
	{
		// do prefix expansion
		// remove exact form modifier, if any
		const char * sPrefix = sWord;
		if ( *sPrefix=='=' )
			sPrefix++;

		// skip leading wildcards
		// (in case we got here on non-infixed index path)
		const char * sWildcard = sPrefix;
		while ( sphIsWild ( *sPrefix ) )
		{
			sPrefix++;
			sWildcard++;
		}

		// compute non-wildcard prefix length
		int iPrefix = 0;
		const char * sCodes = sPrefix;
		for ( ; *sCodes && !sphIsWild ( *sCodes ); sCodes+=sphUtf8CharBytes ( *sCodes ) )
			iPrefix++;

		// do not expand prefixes under min length
		if ( iPrefix<tCtx.m_iMinPrefixLen )
			return false;

		int iBytes = sCodes - sPrefix;
		// prefix expansion should work on nonstemmed words only
		char sFixed[MAX_KEYWORD_BYTES];
		if ( tCtx.m_bHasExactForms )
		{
			sFixed[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			memcpy ( sFixed+1, sPrefix, iBytes );
			sPrefix = sFixed;
			iBytes++;
		}

		tCtx.m_pWordlist->GetPrefixedWords ( sPrefix, iBytes, sWildcard, tWordlist );

	} else
	{
		// do infix expansion
		assert ( sphIsWild ( *sWord ) );
		assert ( tCtx.m_iMinInfixLen>0 );

		// find the longest substring of non-wildcards
		int iCodepoints = 0;
		int iInfixCodepoints = 0;
		int iInfixBytes = 0;
		const char * sMaxInfix = NULL;
		const char * sInfix = sWord;

		for ( const char * s = sWord; *s; )
		{
			int iCodeLen = sphUtf8CharBytes ( *s );

			if ( sphIsWild ( *s ) )
			{
				sInfix = s + 1;
				iCodepoints = 0;
			} else
			{
				iCodepoints++;
				if ( s - sInfix + iCodeLen > iInfixBytes )
				{
					sMaxInfix = sInfix;
					iInfixBytes = s - sInfix + iCodeLen;
					iInfixCodepoints = iCodepoints;
				}
			}

			s += iCodeLen;
		}

		// do not expand infixes under min_infix_len
		if ( iInfixCodepoints < tCtx.m_iMinInfixLen )
			return false;

		// ignore heading star
		tCtx.m_pWordlist->GetInfixedWords ( sMaxInfix, iInfixBytes, sWord, tWordlist );
	}

	return true;
}

XQNode_t * CSphIndex_VLN::ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta * pResult, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags ) const
{
	if ( !pNode || !m_pDict->GetSettings().m_bWordDict
			|| ( m_tSettings.m_iMinPrefixLen<=0 && m_tSettings.m_iMinInfixLen<=0 ) )
		return pNode;

	assert ( m_bPassedAlloc );
	assert ( !m_tWordlist.m_tBuf.IsEmpty() );

	ExpansionContext_t tCtx;
	tCtx.m_pWordlist = &m_tWordlist;
	tCtx.m_pResult = pResult;
	tCtx.m_iMinPrefixLen = m_tSettings.m_iMinPrefixLen;
	tCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
	tCtx.m_iExpansionLimit = m_iExpansionLimit;
	tCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
	tCtx.m_bMergeSingles = ( uQueryDebugFlags & QUERY_DEBUG_NO_PAYLOAD )==0;
	tCtx.m_pPayloads = pPayloads;
	tCtx.m_eHitless = m_tSettings.m_eHitless;

	pNode = sphExpandXQNode ( pNode, tCtx );
	pNode->Check ( true );

	return pNode;
}


// transform the (A B) NEAR C into A NEAR B NEAR C
static void TransformNear ( XQNode_t ** ppNode )
{
	XQNode_t *& pNode = *ppNode;
	if ( pNode->GetOp()==SPH_QUERY_NEAR )
	{
		assert ( pNode->m_dWords.GetLength()==0 );
		CSphVector<XQNode_t*> dArgs;
		int iStartFrom;

		// transform all (A B C) NEAR D into A NEAR B NEAR C NEAR D
		do
		{
			dArgs.Reset();
			iStartFrom = 0;
			ARRAY_FOREACH ( i, pNode->m_dChildren )
			{
				XQNode_t * pChild = pNode->m_dChildren[i]; ///< shortcut
				if ( pChild->GetOp()==SPH_QUERY_AND && pChild->m_dChildren.GetLength()>0 )
				{
					ARRAY_FOREACH ( j, pChild->m_dChildren )
					{
						if ( j==0 && iStartFrom==0 )
						{
							// we will remove the node anyway, so just replace it with 1-st child instead
							pNode->m_dChildren[i] = pChild->m_dChildren[j];
							pNode->m_dChildren[i]->m_pParent = pNode;
							iStartFrom = i+1;
						} else
						{
							dArgs.Add ( pChild->m_dChildren[j] );
						}
					}
					pChild->m_dChildren.Reset();
					SafeDelete ( pChild );
				} else if ( iStartFrom!=0 )
				{
					dArgs.Add ( pChild );
				}
			}

			if ( iStartFrom!=0 )
			{
				pNode->m_dChildren.Resize ( iStartFrom + dArgs.GetLength() );
				ARRAY_FOREACH ( i, dArgs )
				{
					pNode->m_dChildren [ i + iStartFrom ] = dArgs[i];
					pNode->m_dChildren [ i + iStartFrom ]->m_pParent = pNode;
				}
			}
		} while ( iStartFrom!=0 );
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		TransformNear ( &pNode->m_dChildren[i] );
}


/// tag excluded keywords (rvals to operator NOT)
static void TagExcluded ( XQNode_t * pNode, bool bNot )
{
	if ( pNode->GetOp()==SPH_QUERY_ANDNOT )
	{
		assert ( pNode->m_dChildren.GetLength()==2 );
		assert ( pNode->m_dWords.GetLength()==0 );
		TagExcluded ( pNode->m_dChildren[0], bNot );
		TagExcluded ( pNode->m_dChildren[1], !bNot );

	} else if ( pNode->m_dChildren.GetLength() )
	{
		// FIXME? check if this works okay with "virtually plain" stuff?
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TagExcluded ( pNode->m_dChildren[i], bNot );
	} else
	{
		// tricky bit
		// no assert on length here and that is intended
		// we have fully empty nodes (0 children, 0 words) sometimes!
		ARRAY_FOREACH ( i, pNode->m_dWords )
			pNode->m_dWords[i].m_bExcluded = bNot;
	}
}


/// optimize phrase queries if we have bigrams
static void TransformBigrams ( XQNode_t * pNode, const CSphIndexSettings & tSettings )
{
	assert ( tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE );
	assert ( tSettings.m_eBigramIndex==SPH_BIGRAM_ALL || tSettings.m_dBigramWords.GetLength() );

	if ( pNode->GetOp()!=SPH_QUERY_PHRASE )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TransformBigrams ( pNode->m_dChildren[i], tSettings );
		return;
	}

	CSphBitvec bmRemove;
	bmRemove.Init ( pNode->m_dWords.GetLength() );

	for ( int i=0; i<pNode->m_dWords.GetLength()-1; i++ )
	{
		// check whether this pair was indexed
		bool bBigram = false;
		switch ( tSettings.m_eBigramIndex )
		{
			case SPH_BIGRAM_NONE:
				break;
			case SPH_BIGRAM_ALL:
				bBigram = true;
				break;
			case SPH_BIGRAM_FIRSTFREQ:
				bBigram = tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i].m_sWord )!=NULL;
				break;
			case SPH_BIGRAM_BOTHFREQ:
				bBigram =
					( tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i].m_sWord )!=NULL ) &&
					( tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i+1].m_sWord )!=NULL );
				break;
		}
		if ( !bBigram )
			continue;

		// replace the pair with a bigram keyword
		// FIXME!!! set phrase weight for this "word" here
		pNode->m_dWords[i].m_sWord.SetSprintf ( "%s%c%s",
			pNode->m_dWords[i].m_sWord.cstr(),
			MAGIC_WORD_BIGRAM,
			pNode->m_dWords[i+1].m_sWord.cstr() );

		// only mark for removal now, we will sweep later
		// so that [a b c] would convert to ["a b" "b c"], not just ["a b" c]
		bmRemove.BitClear ( i );
		bmRemove.BitSet ( i+1 );
	}

	// remove marked words
	int iOut = 0;
	ARRAY_FOREACH ( i, pNode->m_dWords )
		if ( !bmRemove.BitGet(i) )
			pNode->m_dWords[iOut++] = pNode->m_dWords[i];
	pNode->m_dWords.Resize ( iOut );

	// fixup nodes that are not real phrases any more
	if ( pNode->m_dWords.GetLength()==1 )
		pNode->SetOp ( SPH_QUERY_AND );
}


/// create a node from a set of lemmas
/// WARNING, tKeyword might or might not be pointing to pNode->m_dWords[0]
/// Called from the daemon side (searchd) in time of query
static void TransformAotFilterKeyword ( XQNode_t * pNode, const XQKeyword_t & tKeyword, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	assert ( pNode->m_dWords.GetLength()<=1 );
	assert ( pNode->m_dChildren.GetLength()==0 );

	XQNode_t * pExact = NULL;
	if ( pWordforms )
	{
		// do a copy, because patching in place is not an option
		// short => longlonglong wordform mapping would crash
		// OPTIMIZE? forms that are not found will (?) get looked up again in the dict
		char sBuf [ MAX_KEYWORD_BYTES ];
		strncpy ( sBuf, tKeyword.m_sWord.cstr(), sizeof(sBuf)-1 );
		if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, true, false ) )
		{
			if ( !pNode->m_dWords.GetLength() )
				pNode->m_dWords.Add ( tKeyword );
			pNode->m_dWords[0].m_sWord = sBuf;
			pNode->m_dWords[0].m_bMorphed = true;
			return;
		}
	}

	StrVec_t dLemmas;
	DWORD uLangMask = tSettings.m_uAotFilterMask;
	for ( int i=AOT_BEGIN; i<AOT_LENGTH; ++i )
	{
		if ( uLangMask & (1UL<<i) )
		{
			if ( i==AOT_RU )
				sphAotLemmatizeRu ( dLemmas, (BYTE*)tKeyword.m_sWord.cstr() );
			else if ( i==AOT_DE )
				sphAotLemmatizeDe ( dLemmas, (BYTE*)tKeyword.m_sWord.cstr() );
			else
				sphAotLemmatize ( dLemmas, (BYTE*)tKeyword.m_sWord.cstr(), i );
		}
	}

	// post-morph wordforms
	if ( pWordforms && pWordforms->m_bHavePostMorphNF )
	{
		char sBuf [ MAX_KEYWORD_BYTES ];
		ARRAY_FOREACH ( i, dLemmas )
		{
			strncpy ( sBuf, dLemmas[i].cstr(), sizeof(sBuf)-1 );
			if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, false, false ) )
				dLemmas[i] = sBuf;
		}
	}

	if ( dLemmas.GetLength() && tSettings.m_bIndexExactWords )
	{
		pExact = CloneKeyword ( pNode );
		if ( !pExact->m_dWords.GetLength() )
			pExact->m_dWords.Add ( tKeyword );

		pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", tKeyword.m_sWord.cstr() );
		pExact->m_pParent = pNode;
	}

	if ( !pExact && dLemmas.GetLength()<=1 )
	{
		// zero or one lemmas, update node in-place
		if ( !pNode->m_dWords.GetLength() )
			pNode->m_dWords.Add ( tKeyword );
		if ( dLemmas.GetLength() )
		{
			pNode->m_dWords[0].m_sWord = dLemmas[0];
			pNode->m_dWords[0].m_bMorphed = true;
		}
	} else
	{
		// multiple lemmas, create an OR node
		pNode->SetOp ( SPH_QUERY_OR );
		ARRAY_FOREACH ( i, dLemmas )
		{
			pNode->m_dChildren.Add ( new XQNode_t ( pNode->m_dSpec ) );
			pNode->m_dChildren.Last()->m_pParent = pNode;
			XQKeyword_t & tLemma = pNode->m_dChildren.Last()->m_dWords.Add();
			tLemma.m_sWord = dLemmas[i];
			tLemma.m_iAtomPos = tKeyword.m_iAtomPos;
			tLemma.m_bFieldStart = tKeyword.m_bFieldStart;
			tLemma.m_bFieldEnd = tKeyword.m_bFieldEnd;
			tLemma.m_bMorphed = true;
		}
		pNode->m_dWords.Reset();
		if ( pExact )
			pNode->m_dChildren.Add ( pExact );
	}
}


/// AOT morph guesses transform
/// replaces tokens with their respective morph guesses subtrees
/// used in lemmatize_ru_all morphology processing mode that can generate multiple guesses
/// in other modes, there is always exactly one morph guess, and the dictionary handles it
/// Called from the daemon side (searchd)
void TransformAotFilter ( XQNode_t * pNode, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	if ( !pNode )
		return;
	// case one, regular operator (and empty nodes)
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		TransformAotFilter ( pNode->m_dChildren[i], pWordforms, tSettings );
	if ( pNode->m_dChildren.GetLength() || pNode->m_dWords.GetLength()==0 )
		return;

	// case two, operator on a bag of words
	// FIXME? check phrase vs expand_keywords vs lemmatize_ru_all?
	if ( pNode->m_dWords.GetLength()
		&& ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM ) )
	{
		assert ( pNode->m_dWords.GetLength() );

		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pNew = new XQNode_t ( pNode->m_dSpec );
			pNew->m_pParent = pNode;
			pNew->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Add ( pNew );
			TransformAotFilterKeyword ( pNew, pNode->m_dWords[i], pWordforms, tSettings );
		}

		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return;
	}

	// case three, plain old single keyword
	assert ( pNode->m_dWords.GetLength()==1 );
	TransformAotFilterKeyword ( pNode, pNode->m_dWords[0], pWordforms, tSettings );
}


void sphTransformExtendedQuery ( XQNode_t ** ppNode, const CSphIndexSettings & tSettings, bool bHasBooleanOptimization, const ISphKeywordsStat * pKeywords )
{
	TransformQuorum ( ppNode );
	( *ppNode )->Check ( true );
	TransformNear ( ppNode );
	( *ppNode )->Check ( true );
	if ( tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE )
		TransformBigrams ( *ppNode, tSettings );
	TagExcluded ( *ppNode, false );
	( *ppNode )->Check ( true );

	// boolean optimization
	if ( bHasBooleanOptimization )
		sphOptimizeBoolean ( ppNode, pKeywords );
}

/// one regular query vs many sorters
bool CSphIndex_VLN::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult,
	int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	assert ( pQuery );
	CSphQueryProfile * pProfile = pResult->m_pProfile;

	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( iSorters );
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i] )
			dSorters.Add ( ppSorters[i] );

	iSorters = dSorters.GetLength();

	// if we have anything to work with
	if ( iSorters==0 )
		return false;

	// non-random at the start, random at the end
	dSorters.Sort ( CmpPSortersByRandom_fn() );

	const QueryParser_i * pQueryParser = pQuery->m_pQueryParser;
	assert ( pQueryParser );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( *pQuery ) )
		return MultiScan ( pQuery, pResult, iSorters, &dSorters[0], tArgs );

	if ( pProfile )
		pProfile->Switch ( SPH_QSTATE_DICT_SETUP );

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (BYTE *)pQuery->m_sQuery.cstr();

	ISphFieldFilterRefPtr_c pFieldFilter;
	if ( m_pFieldFilter && sModifiedQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, strlen ( (char*)sModifiedQuery ), dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// parse query
	if ( pProfile )
		pProfile->Switch ( SPH_QSTATE_PARSE );

	XQQuery_t tParsed;
	if ( !pQueryParser->ParseQuery ( tParsed, (const char*)sModifiedQuery, pQuery, m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings ) )
	{
		// FIXME? might wanna reset profile to unknown state
		pResult->m_sError = tParsed.m_sParseError;
		return false;
	}

	// check again for fullscan
	if ( pQueryParser->IsFullscan ( tParsed ) )
		return MultiScan ( pQuery, pResult, iSorters, &dSorters[0], tArgs );

	if ( !tParsed.m_sParseWarning.IsEmpty() )
		pResult->m_sWarning = tParsed.m_sParseWarning;

	// transform query if needed (quorum transform, etc.)
	if ( pProfile )
		pProfile->Switch ( SPH_QSTATE_TRANSFORMS );
	sphTransformExtendedQuery ( &tParsed.m_pRoot, m_tSettings, pQuery->m_bSimplify, this );

	int iExpandKeywords = ExpandKeywords ( m_iExpandKeywords, pQuery->m_eExpandKeywords, m_tSettings );
	if ( iExpandKeywords!=KWE_DISABLED )
	{
		tParsed.m_pRoot = sphQueryExpandKeywords ( tParsed.m_pRoot, m_tSettings, iExpandKeywords );
		tParsed.m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	if ( m_tSettings.m_uAotFilterMask )
		TransformAotFilter ( tParsed.m_pRoot, pDict->GetWordforms(), m_tSettings );

	SphWordStatChecker_t tStatDiff;
	tStatDiff.Set ( pResult->m_hWordStats );

	// expanding prefix in word dictionary case
	CSphScopedPayload tPayloads;
	XQNode_t * pPrefixed = ExpandPrefix ( tParsed.m_pRoot, pResult, &tPayloads, pQuery->m_uDebugFlags );
	if ( !pPrefixed )
		return false;
	tParsed.m_pRoot = pPrefixed;

	if ( !sphCheckQueryHeight ( tParsed.m_pRoot, pResult->m_sError ) )
		return false;

	// flag common subtrees
	int iCommonSubtrees = 0;
	if ( m_iMaxCachedDocs && m_iMaxCachedHits )
		iCommonSubtrees = sphMarkCommonSubtrees ( 1, &tParsed );

	tParsed.m_bNeedSZlist = pQuery->m_bZSlist;

	CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
	bool bResult = ParsedMultiQuery ( pQuery, pResult, iSorters, &dSorters[0], tParsed, pDict, tArgs, &tNodeCache, tStatDiff );

	if ( tArgs.m_bModifySorterSchemas )
	{
		if ( pProfile )
			pProfile->Switch ( SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( &dSorters[0], iSorters, m_tBlobAttrs.GetWritePtr() );
	}

	return bResult;
}


/// many regular queries with one sorter attached to each query.
/// returns true if at least one query succeeded. The failed queries indicated with pResult->m_iMultiplier==-1
bool CSphIndex_VLN::MultiQueryEx ( int iQueries, const CSphQuery * pQueries,
	CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// ensure we have multiple queries
	assert ( ppResults );
	if ( iQueries==1 )
		return MultiQuery ( pQueries, ppResults[0], 1, ppSorters, tArgs );

	MEMORY ( MEM_DISK_QUERYEX );

	assert ( pQueries );
	assert ( ppSorters );

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphFixedVector<XQQuery_t> dXQ ( iQueries );
	CSphFixedVector<SphWordStatChecker_t> dStatChecker ( iQueries );
	CSphScopedPayload tPayloads;
	bool bResult = false;
	bool bResultScan = false;
	for ( int i=0; i<iQueries; ++i )
	{
		// nothing to do without a sorter
		if ( !ppSorters[i] )
		{
			ppResults[i]->m_iMultiplier = -1; ///< show that this particular query failed
			continue;
		}

		// fast path for scans
		if ( pQueries[i].m_sQuery.IsEmpty() )
		{
			if ( MultiScan ( pQueries + i, ppResults[i], 1, &ppSorters[i], tArgs ) )
				bResultScan = true;
			else
				ppResults[i]->m_iMultiplier = -1; ///< show that this particular query failed
			continue;
		}

		ppResults[i]->m_tIOStats.Start();

		// parse query
		const QueryParser_i * pQueryParser = pQueries[i].m_pQueryParser;
		assert ( pQueryParser );

		if ( pQueryParser->ParseQuery ( dXQ[i], pQueries[i].m_sQuery.cstr(), &(pQueries[i]), m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings ) )
		{
			// transform query if needed (quorum transform, keyword expansion, etc.)
			sphTransformExtendedQuery ( &dXQ[i].m_pRoot, m_tSettings, pQueries[i].m_bSimplify, this );

			int iExpandKeywords = ExpandKeywords ( m_iExpandKeywords, pQueries[i].m_eExpandKeywords, m_tSettings );
			if ( iExpandKeywords!=KWE_DISABLED )
			{
				dXQ[i].m_pRoot = sphQueryExpandKeywords ( dXQ[i].m_pRoot, m_tSettings, iExpandKeywords );
				dXQ[i].m_pRoot->Check ( true );
			}

			// this should be after keyword expansion
			if ( m_tSettings.m_uAotFilterMask )
				TransformAotFilter ( dXQ[i].m_pRoot, pDict->GetWordforms(), m_tSettings );

			dStatChecker[i].Set ( ppResults[i]->m_hWordStats );

			// expanding prefix in word dictionary case
			XQNode_t * pPrefixed = ExpandPrefix ( dXQ[i].m_pRoot, ppResults[i], &tPayloads, pQueries[i].m_uDebugFlags );
			if ( pPrefixed )
			{
				dXQ[i].m_pRoot = pPrefixed;

				if ( sphCheckQueryHeight ( dXQ[i].m_pRoot, ppResults[i]->m_sError ) )
				{
					bResult = true;
				} else
				{
					ppResults[i]->m_iMultiplier = -1;
					SafeDelete ( dXQ[i].m_pRoot );
				}
			} else
			{
				ppResults[i]->m_iMultiplier = -1;
				SafeDelete ( dXQ[i].m_pRoot );
			}
		} else
		{
			ppResults[i]->m_sError = dXQ[i].m_sParseError;
			ppResults[i]->m_iMultiplier = -1;
		}
		if ( !dXQ[i].m_sParseWarning.IsEmpty() )
			ppResults[i]->m_sWarning = dXQ[i].m_sParseWarning;

		ppResults[i]->m_tIOStats.Stop();
	}

	// continue only if we have at least one non-failed
	if ( bResult )
	{
		int iCommonSubtrees = 0;
		if ( m_iMaxCachedDocs && m_iMaxCachedHits )
			iCommonSubtrees = sphMarkCommonSubtrees ( iQueries, &dXQ[0] );

		CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
		bResult = false;
		for ( int j=0; j<iQueries; j++ )
		{
			// fullscan case
			if ( pQueries[j].m_sQuery.IsEmpty() )
				continue;

			ppResults[j]->m_tIOStats.Start();

			if ( dXQ[j].m_pRoot && ppSorters[j]
					&& ParsedMultiQuery ( &pQueries[j], ppResults[j], 1, &ppSorters[j], dXQ[j], pDict, tArgs, &tNodeCache, dStatChecker[j] ) )
			{
				bResult = true;
				ppResults[j]->m_iMultiplier = iCommonSubtrees ? iQueries : 1;
			} else
			{
				ppResults[j]->m_iMultiplier = -1;
			}

			ppResults[j]->m_tIOStats.Stop();
		}
	}

	if ( tArgs.m_bModifySorterSchemas )
	{
		if ( ppResults[0] && ppResults[0]->m_pProfile )
			ppResults[0]->m_pProfile->Switch ( SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( ppSorters, iQueries, m_tBlobAttrs.GetWritePtr() );
	}

	return bResult | bResultScan;
}

bool CSphIndex_VLN::ParsedMultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult,
	int iSorters, ISphMatchSorter ** ppSorters, const XQQuery_t & tXQ, CSphDict * pDict,
	const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, const SphWordStatChecker_t & tStatDiff ) const
{
	assert ( pQuery );
	assert ( pResult );
	assert ( ppSorters );
	assert ( !pQuery->m_sQuery.IsEmpty() && pQuery->m_eMode!=SPH_MATCH_FULLSCAN ); // scans must go through MultiScan()
	assert ( tArgs.m_iTag>=0 );

	// start counting
	int64_t tmQueryStart = sphMicroTimer();

	CSphQueryProfile * pProfile = pResult->m_pProfile;
	ESphQueryState eOldState = SPH_QSTATE_UNKNOWN;
	if ( pProfile )
		eOldState = pProfile->Switch ( SPH_QSTATE_INIT );

	ScopedThreadPriority_c tPrio ( pQuery->m_bLowPriority );

	///////////////////
	// setup searching
	///////////////////

	// non-ready index, empty response!
	if ( !m_bPassedAlloc )
	{
		pResult->m_sError = "index not preread";
		return false;
	}

	// select the sorter with max schema
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i]->GetSchema()->GetRowSize() > iMaxSchemaSize )
		{
			iMaxSchemaSize = ppSorters[i]->GetSchema()->GetRowSize();
			iMaxSchemaIndex = i;
		}

	const ISphSchema & tMaxSorterSchema = *(ppSorters[iMaxSchemaIndex]->GetSchema());

	CSphVector< const ISphSchema * > dSorterSchemas;
	SorterSchemas ( ppSorters, iSorters, iMaxSchemaIndex, dSorterSchemas );

	// setup calculations and result schema
	CSphQueryContext tCtx ( *pQuery );
	tCtx.m_pProfile = pProfile;
	tCtx.m_pLocalDocs = tArgs.m_pLocalDocs;
	tCtx.m_iTotalDocs = ( tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments );
	tCtx.m_pIndexSegment = this;

	if ( !tCtx.SetupCalc ( pResult, tMaxSorterSchema, m_tSchema, m_tBlobAttrs.GetWritePtr(), dSorterSchemas ) )
		return false;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetWritePtr() );

	tCtx.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;

	// open files
	DataReaderFabricPtr_t pDoclist = m_pDoclistFile;
	DataReaderFabricPtr_t pHitlist = m_pHitlistFile;
	if ( pProfile && ( !pDoclist || !pHitlist ) )
		pProfile->Switch ( SPH_QSTATE_OPEN );

	if ( !pDoclist )
	{
		pDoclist = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), pResult->m_sError,
									 DataReaderFabric_c::eDocs, m_tFiles.m_iReadBufferDocList, true );
		if ( !pDoclist )
			return false;
	}

	if ( !pHitlist )
	{
		pHitlist = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), pResult->m_sError,
									 DataReaderFabric_c::eHits, m_tFiles.m_iReadBufferHitList, true );
		if ( !pHitlist )
			return false;
	}

	pDoclist->SetProfile ( pProfile );
	pHitlist->SetProfile ( pProfile );

	if ( pProfile )
		pProfile->Switch ( SPH_QSTATE_INIT );

	// setup search terms
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist,
		m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize );

	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;
	tTermSetup.m_iDynamicRowitems = tMaxSorterSchema.GetDynamicSize();

	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time
	tTermSetup.m_pWarning = &pResult->m_sWarning;
	tTermSetup.m_bSetupReaders = true;
	tTermSetup.m_pCtx = &tCtx;
	tTermSetup.m_pNodeCache = pNodeCache;

	// setup prediction constrain
	CSphQueryStats tQueryStats;
	bool bCollectPredictionCounters = ( pQuery->m_iMaxPredictedMsec>0 );
	int64_t iNanoBudget = (int64_t)(pQuery->m_iMaxPredictedMsec) * 1000000; // from milliseconds to nanoseconds
	tQueryStats.m_pNanoBudget = &iNanoBudget;
	if ( bCollectPredictionCounters )
		tTermSetup.m_pStats = &tQueryStats;

	// bind weights
	tCtx.BindWeights ( pQuery, m_tSchema, pResult->m_sWarning );

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( tXQ, pQuery, pResult, tTermSetup, tCtx, tMaxSorterSchema ) );
	if ( !pRanker.Ptr() )
		return false;

	tStatDiff.DumpDiffer ( pResult->m_hWordStats, m_sIndexName.cstr(), pResult->m_sWarning );

	if ( ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE ) && pQuery->m_eRanker!=SPH_RANK_EXPR )
		pResult->m_sWarning.SetSprintf ( "packedfactors() and bm25f() requires using an expression ranker" );

	tCtx.SetupExtraData ( pRanker.Ptr(), iSorters==1 ? ppSorters[0] : NULL );

	const BYTE * pBlobPool = m_tBlobAttrs.GetWritePtr();
	pRanker->ExtraData ( EXTRA_SET_BLOBPOOL, (void**)&pBlobPool );

	int iMatchPoolSize = 0;
	for ( int i=0; i<iSorters; i++ )
		iMatchPoolSize += ppSorters[i]->m_iMatchCapacity;

	pRanker->ExtraData ( EXTRA_SET_POOL_CAPACITY, (void**)&iMatchPoolSize );

	// check for the possible integer overflow in m_dPool.Resize
	int64_t iPoolSize = 0;
	if ( pRanker->ExtraData ( EXTRA_GET_POOL_SIZE, (void**)&iPoolSize ) && iPoolSize>INT_MAX )
	{
		pResult->m_sError.SetSprintf ( "ranking factors pool too big (%d Mb), reduce max_matches", (int)( iPoolSize/1024/1024 ) );
		return false;
	}

	// empty index, empty response!
	if ( m_bIsEmpty )
		return true;

	assert ( !m_tAttr.IsEmpty() ); // check that docinfo is preloaded

	// setup filters
 	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &pQuery->m_dFilters;
	tFlx.m_pFilterTree = &pQuery->m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_pBlobPool = m_tBlobAttrs.GetWritePtr ();
	tFlx.m_eCollation = pQuery->m_eCollation;
	tFlx.m_bScan = pQuery->m_sQuery.IsEmpty ();
	tFlx.m_pHistograms = m_pHistograms;

	if ( !tCtx.CreateFilters ( tFlx, pResult->m_sError, pResult->m_sWarning ) )
		return false;

	// check if we can early reject the whole index
	if ( tCtx.m_pFilter && m_iDocinfoIndex )
	{
		DWORD uStride = m_tSchema.GetRowSize();
		DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ m_iDocinfoIndex*uStride*2 ] );
		DWORD * pMaxEntry = pMinEntry + uStride;

		if ( !tCtx.m_pFilter->EvalBlock ( pMinEntry, pMaxEntry ) )
			return true;
	}

	// setup sorters vs. MVA
	for ( int i=0; i<iSorters; i++ )
		(ppSorters[i])->SetBlobPool ( m_tBlobAttrs.GetWritePtr() );

	bool bHaveRandom = false;
	for ( int i=0; i<iSorters; i++ )
		bHaveRandom |= ppSorters[i]->m_bRandomize;

	bool bUseFactors = !!( tCtx.m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
	bool bHaveDead = m_tDeadRowMap.HasDead();

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	bool bFinalPass = !!tCtx.m_dCalcFinal.GetLength();
	int iMyTag = bFinalPass ? -1 : tArgs.m_iTag;

	// a shortcut to avoid listing all args every time
	void (CSphIndex_VLN::*pFunc)(CSphQueryContext *, const CSphQuery *, int, ISphMatchSorter **, ISphRanker *, int, int) const = nullptr;

	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:
		case SPH_MATCH_PHRASE:
		case SPH_MATCH_ANY:
		case SPH_MATCH_EXTENDED:
		case SPH_MATCH_EXTENDED2:
		case SPH_MATCH_BOOLEAN:
			if ( bHaveDead )
			{
				if ( bHaveRandom )
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<true, true, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<true, true, false>;
				}
				else
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<true, false, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<true, false, false>;
				}
			}
			else
			{
				if ( bHaveRandom )
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<false, true, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<false, true, false>;
				}
				else
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<false, false, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<false, false, false>;
				}
			}

			(*this.*pFunc)( &tCtx, pQuery, iSorters, ppSorters, pRanker.Ptr(), iMyTag, tArgs.m_iIndexWeight );
			break;

		default:
			sphDie ( "INTERNAL ERROR: unknown matching mode (mode=%d)", pQuery->m_eMode );
	}

	////////////////////
	// cook result sets
	////////////////////

	if ( pProfile )
		pProfile->Switch ( SPH_QSTATE_FINALIZE );

	// adjust result sets
	if ( bFinalPass )
	{
		// GotUDF means promise to UDFs that final-stage calls will be evaluated
		// a) over the final, pre-limit result set
		// b) in the final result set order
		bool bGotUDF = false;
		ARRAY_FOREACH_COND ( i, tCtx.m_dCalcFinal, !bGotUDF )
			tCtx.m_dCalcFinal[i].m_pExpr->Command ( SPH_EXPR_GET_UDF, &bGotUDF );

		SphFinalMatchCalc_t tProcessor ( tArgs.m_iTag, tCtx );
		for ( int iSorter=0; iSorter<iSorters; iSorter++ )
		{
			ISphMatchSorter * pTop = ppSorters[iSorter];
			pTop->Finalize ( tProcessor, bGotUDF );
		}
		pResult->m_iBadRows += tProcessor.m_iBadRows;
	}

	pRanker->FinalizeCache ( tMaxSorterSchema );

	// mva and string pools ptrs
	pResult->m_pBlobPool = m_tBlobAttrs.GetWritePtr();

	// query timer
	int64_t tmWall = sphMicroTimer() - tmQueryStart;
	pResult->m_iQueryTime += (int)( tmWall/1000 );

#if 0
	printf ( "qtm %d, %d, %d, %d, %d\n", int(tmWall), tQueryStats.m_iFetchedDocs,
		tQueryStats.m_iFetchedHits, tQueryStats.m_iSkips, ppSorters[0]->GetTotalCount() );
#endif

	if ( pProfile )
		pProfile->Switch ( eOldState );

	if ( bCollectPredictionCounters )
	{
		pResult->m_tStats.Add ( tQueryStats );
		pResult->m_bHasPrediction = true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// INDEX STATUS
//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::GetStatus ( CSphIndexStatus* pRes ) const
{
	assert ( pRes );
	if ( !pRes )
		return;

	pRes->m_iRamUse = sizeof(CSphIndex_VLN)
		+ m_dFieldLens.GetLengthBytes()

		+ m_tAttr.GetLengthBytes()
		+ m_tBlobAttrs.GetLengthBytes()
		+ m_tWordlist.m_tBuf.GetLengthBytes()
		+ m_tDeadRowMap.GetLengthBytes()
		+ m_tSkiplists.GetLengthBytes();

	pRes->m_iDiskUse = 0;

	CSphVector<IndexFileExt_t> dExts = sphGetExts();
	for ( const auto & i : dExts )
		if ( i.m_eExt!=SPH_EXT_SPL )
		{
			CSphString sFile;
			sFile.SetSprintf ( "%s%s", m_sFilename.cstr(), i.m_szExt );
			struct_stat st;
			if ( stat ( sFile.cstr(), &st )==0 )
				pRes->m_iDiskUse += st.st_size;
		}
}

//////////////////////////////////////////////////////////////////////////
// SHA1 digests
//////////////////////////////////////////////////////////////////////////

// SHA1 from https://github.com/shodanium/nanomysql/blob/master/nanomysql.cpp

// nanomysql, a tiny MySQL client
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

class SHA1_c
{
	DWORD state[5], count[2];
	BYTE buffer[64];

	void Transform ( const BYTE buf[64] )
	{
		DWORD a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], block[16];
		memset ( block, 0, sizeof ( block ) ); // initial conversion to big-endian units
		for ( int i = 0; i<64; i++ )
			block[i >> 2] += buf[i] << ( ( 3 - ( i & 3 ) ) * 8 );
		for ( int i = 0; i<80; i++ ) // do hashing rounds
		{
#define _LROT( value, bits ) ( ( (value) << (bits) ) | ( (value) >> ( 32-(bits) ) ) )
			if ( i>=16 )
				block[i & 15] = _LROT (
					block[( i + 13 ) & 15] ^ block[( i + 8 ) & 15] ^ block[( i + 2 ) & 15] ^ block[i & 15], 1 );

			if ( i<20 )
				e += ( ( b & ( c ^ d ) ) ^ d ) + 0x5A827999;
			else if ( i<40 )
				e += ( b ^ c ^ d ) + 0x6ED9EBA1;
			else if ( i<60 )
				e += ( ( ( b | c ) & d ) | ( b & c ) ) + 0x8F1BBCDC;
			else
				e += ( b ^ c ^ d ) + 0xCA62C1D6;

			e += block[i & 15] + _LROT ( a, 5 );
			DWORD t = e;
			e = d;
			d = c;
			c = _LROT ( b, 30 );
			b = a;
			a = t;
		}
		state[0] += a; // save state
		state[1] += b;
		state[2] += c;
		state[3] += d;
		state[4] += e;
	}

public:
	SHA1_c & Init()
	{
		const DWORD dInit[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
		memcpy ( state, dInit, sizeof ( state ) );
		count[0] = count[1] = 0;
		return *this;
	}

	SHA1_c & Update ( const BYTE * data, int len )
	{
		int i, j = ( count[0] >> 3 ) & 63;
		count[0] += ( len << 3 );
		if ( count[0]<( DWORD ) ( len << 3 ) )
			count[1]++;
		count[1] += ( len >> 29 );
		if ( ( j + len )>63 )
		{
			i = 64 - j;
			memcpy ( &buffer[j], data, i );
			Transform ( buffer );
			for ( ; i + 63<len; i += 64 )
				Transform ( data + i );
			j = 0;
		} else
			i = 0;
		memcpy ( &buffer[j], &data[i], len - i );
		return *this;
	}

	void Final ( BYTE digest[SHA1_SIZE] )
	{
		BYTE finalcount[8];
		for ( auto i = 0; i<8; i++ )
			finalcount[i] = ( BYTE ) ( ( count[( i>=4 ) ? 0 : 1] >> ( ( 3 - ( i & 3 ) ) * 8 ) )
				& 255 ); // endian independent
		Update ( ( BYTE * ) "\200", 1 ); // add padding
		while ( ( count[0] & 504 )!=448 )
			Update ( ( BYTE * ) "\0", 1 );
		Update ( finalcount, 8 ); // should cause a SHA1_Transform()
		for ( auto i = 0; i<SHA1_SIZE; i++ )
			digest[i] = ( BYTE ) ( ( state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
	}
};


CSphString BinToHex ( const CSphFixedVector<BYTE> & dHash )
{
	const char * sDigits = "0123456789abcdef";
	if ( dHash.IsEmpty() )
		return "";

	CSphString sRes;
	auto iLen = 2*dHash.GetLength()*2;
	sRes.Reserve ( iLen );
	auto sHash = const_cast<char *> (sRes.cstr ());

	ARRAY_FOREACH ( i, dHash )
	{
		sHash[i << 1] = sDigits[dHash[i] >> 4];
		sHash[1 + ( i << 1 )] = sDigits[dHash[i] & 0x0f];
	}
	sHash[iLen] = '\0';
	return sRes;
}

CSphString CalcSHA1 ( const void * pData, int iLen )
{
	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();
	dHasher.Update ( (const BYTE*)pData, iLen );
	dHasher.Final ( dHashValue.begin() );
	return BinToHex ( dHashValue );
}

bool CalcSHA1 ( const CSphString & sFileName, CSphString & sRes, CSphString & sError )
{
	CSphAutofile tFile ( sFileName, SPH_O_READ, sError, false );
	if ( tFile.GetFD()<0 )
		return false;

	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();

	const int64_t iFileSize = tFile.GetSize();
	const int iBufSize = Min ( iFileSize, DEFAULT_READ_BUFFER );
	int64_t iOff = 0;
	CSphFixedVector<BYTE> dFileData ( iBufSize );
	while ( iOff<iFileSize )
	{
		const int iLen = Min ( iBufSize, iFileSize - iOff );
		if ( !tFile.Read ( dFileData.Begin(), iLen, sError ) )
			return false;

		dHasher.Update ( dFileData.Begin(), iLen );
		iOff += iLen;
	}

	dHasher.Final ( dHashValue.Begin() );
	sRes = BinToHex ( dHashValue );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// WriterWithHash_c - CSphWriter which also calc SHA1 on-the-fly
//////////////////////////////////////////////////////////////////////////

WriterWithHash_c::WriterWithHash_c ( const char * sExt, HashCollection_c * pCollector)
	: m_pCollection { pCollector }
	, m_sExt { sExt }
{
	m_pHasher = new SHA1_c;
	m_pHasher->Init();
}

WriterWithHash_c::~WriterWithHash_c ()
{
	SafeDelete ( m_pHasher );
}

void WriterWithHash_c::Flush()
{
	assert ( !m_bHashDone ); // can't do anything with already finished hash
	if ( m_iPoolUsed>0 )
	{
		m_pHasher->Update ( m_pBuffer, m_iPoolUsed );
		CSphWriter::Flush();
	}
}

/*const BYTE * WriterWithHash_c::GetHASHBlob () const
{
	assert ( m_bHashDone );
	return m_dHashValue;
}*/

void WriterWithHash_c::CloseFile ()
{
	assert ( !m_bHashDone );
	CSphWriter::CloseFile ();
	m_pHasher->Final ( m_dHashValue );
	if ( m_pCollection )
		m_pCollection->AppendNewHash ( m_sExt, m_dHashValue );
	m_bHashDone = true;
}


//////////////////////////////////////////////////////////////////////////
// TaggedHash20_t - string tag (filename) with 20-bytes binary hash
//////////////////////////////////////////////////////////////////////////

const BYTE TaggedHash20_t::m_dZeroHash[HASH20_SIZE] = { 0 };

// by tag + hash
TaggedHash20_t::TaggedHash20_t ( const char* sTagName, const BYTE* pHashValue )
	: m_sTagName ( sTagName )
{
	if ( !pHashValue )
		pHashValue = m_dZeroHash;
	memcpy ( m_dHashValue, pHashValue, HASH20_SIZE );
}

// serialize to FIPS form
CSphString TaggedHash20_t::ToFIPS () const
{
	const char * sDigits = "0123456789abcdef";
	char sHash [41];
	StringBuilder_c sResult;

	if ( Empty() )
		return "";

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		sHash[i << 1] = sDigits[m_dHashValue[i] >> 4];
		sHash[1 + (i << 1)] = sDigits[m_dHashValue[i] & 0x0f];
	}
	sHash[40] = '\0';

	// FIPS-180-1 - checksum, space, "*" (indicator of binary mode), tag
	sResult.Appendf ("%s *%s\n", sHash, m_sTagName.cstr());
	return sResult.cstr();
}

inline BYTE hex_char ( unsigned char c )
{
	if ( c>=0x30 && c<=0x39 )
		return c - '0';
	else if ( c>=0x61 && c<=0x66 )
		return c - 'a' + 10;
	assert ( false && "broken hex num - expected digits and a..f letters in the num" );
	return 0;
}

// de-serialize from FIPS, returns len of parsed chunk of sFIPS or -1 on error
int TaggedHash20_t::FromFIPS ( const char * sFIPS )
{
	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n
	// i.e. 40 symbols hex hash in small letters, space, '*' and tag, finished by '\n'

	assert ( sFIPS[HASH20_SIZE * 2]==' ' && "broken FIPS - space expected after hash" );
	assert ( sFIPS[HASH20_SIZE * 2 + 1]=='*' && "broken FIPS - * expected after hash and space" );

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		auto &uCode = m_dHashValue[i];
		uCode = hex_char ( sFIPS[i * 2] );
		uCode = ( uCode << 4 ) + hex_char ( sFIPS[i * 2 + 1] );
	}

	sFIPS += 2 + HASH20_SIZE * 2;
	auto len = strlen ( sFIPS );

	if ( sFIPS[len - 1]!='\n' )
		return -1;

	m_sTagName.SetBinary ( sFIPS, len - 1 );
	return len;
}

bool TaggedHash20_t::operator== ( const BYTE * pRef ) const
{
	assert ( pRef );
	return !memcmp ( m_dHashValue, pRef, HASH20_SIZE );
}

//////////////////////////////////////////////////////////////////////////
// HashCollection_c
//////////////////////////////////////////////////////////////////////////

void HashCollection_c::AppendNewHash ( const char * sExt, const BYTE * pHash )
{
	m_dHashes.Add ( TaggedHash20_t ( sExt, pHash ) );
}

//////////////////////////////////////////////////////////////////////////
// INDEX CHECKING
//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::SetDebugCheck ()
{
	m_bDebugCheck = true;
}


// no strnlen on some OSes (Mac OS)
#if !HAVE_STRNLEN
size_t strnlen ( const char * s, size_t iMaxLen )
{
	if ( !s )
		return 0;

	size_t iRes = 0;
	while ( *s++ && iRes<iMaxLen )
		++iRes;
	return iRes;
}
#endif


DebugCheckError_c::DebugCheckError_c ( FILE * pFile )
	: m_pFile ( pFile )
{
	assert ( pFile );
	m_bProgress = isatty ( fileno ( pFile ) )!=0;
	m_tStartTime = sphMicroTimer();
}


void DebugCheckError_c::Msg ( const char * szFmt, ... )
{
	assert ( m_pFile );
	va_list ap;
	va_start ( ap, szFmt );
	vfprintf ( m_pFile, szFmt, ap );
	fprintf ( m_pFile, "\n" );
	va_end ( ap );
}


void DebugCheckError_c::Fail ( const char * szFmt, ... )
{
	assert ( m_pFile );
	const int FAILS_THRESH = 100;
	if ( ++m_nFails>=FAILS_THRESH )
		return;

	va_list ap;
	va_start ( ap, szFmt );
	fprintf ( m_pFile, "FAILED, " );
	vfprintf ( m_pFile, szFmt, ap );
	if ( m_iSegment>=0 )
		fprintf ( m_pFile, " (segment: %d)", m_iSegment );

	fprintf ( m_pFile, "\n" );
	va_end ( ap );

	m_nFailsPrinted++;
	if ( m_nFailsPrinted==FAILS_THRESH )
		fprintf ( m_pFile, "(threshold reached; suppressing further output)\n" );
}


void DebugCheckError_c::Progress ( const char * szFmt, ... )
{
	if ( !m_bProgress )
		return;

	assert ( m_pFile );

	va_list ap;
	va_start ( ap, szFmt );
	vfprintf ( m_pFile, szFmt, ap );
	fprintf ( m_pFile, "\r" );
	va_end ( ap );

	fflush ( m_pFile );
}


void DebugCheckError_c::Done()
{
	assert ( m_pFile );

	// well, no known kinds of failures, maybe some unknown ones
	int64_t tmCheck = sphMicroTimer() - m_tStartTime;
	if ( !m_nFails )
		fprintf ( m_pFile, "check passed" );
	else if ( m_nFails!=m_nFailsPrinted )
		fprintf ( m_pFile, "check FAILED, " INT64_FMT " of " INT64_FMT " failures reported", m_nFailsPrinted, m_nFails );
	else
		fprintf ( m_pFile, "check FAILED, " INT64_FMT " failures reported", m_nFails );

	fprintf ( m_pFile, ", %d.%d sec elapsed\n", (int)(tmCheck/1000000), (int)((tmCheck/100000)%10) );
}


void DebugCheckError_c::SetSegment ( int iSegment )
{
	m_iSegment = iSegment;
}


int64_t DebugCheckError_c::GetNumFails() const
{
	return m_nFails;
}


void CSphIndex_VLN::DebugCheck_Dictionary ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter )
{
	tReporter.Msg ( "checking dictionary..." );

	SphWordID_t uWordid = 0;
	int64_t iDoclistOffset = 0;
	int iWordsTotal = 0;

	char sWord[MAX_KEYWORD_BYTES], sLastWord[MAX_KEYWORD_BYTES];
	memset ( sWord, 0, sizeof(sWord) );
	memset ( sLastWord, 0, sizeof(sLastWord) );

	const int iWordPerCP = SPH_WORDLIST_CHECKPOINT;
	const bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	CSphVector<CSphWordlistCheckpoint> dCheckpoints;
	dCheckpoints.Reserve ( m_tWordlist.m_dCheckpoints.GetLength() );
	CSphVector<char> dCheckpointWords;
	dCheckpointWords.Reserve ( m_tWordlist.m_pWords.GetLength() );

	CSphAutoreader & tDictReader = tCtx.m_tDictReader;

	tDictReader.GetByte();
	int iLastSkipsOffset = 0;
	SphOffset_t iWordsEnd = m_tWordlist.m_iWordsEnd;

	while ( tDictReader.GetPos()!=iWordsEnd && !m_bIsEmpty )
	{
		// sanity checks
		if ( tDictReader.GetPos()>=iWordsEnd )
		{
			tReporter.Fail ( "reading past checkpoints" );
			break;
		}

		// store current entry pos (for checkpointing later), read next delta
		const int64_t iDictPos = tDictReader.GetPos();
		SphWordID_t iDeltaWord = 0;
		if ( bWordDict )
			iDeltaWord = tDictReader.GetByte();
		else
			iDeltaWord = tDictReader.UnzipWordid();

		// checkpoint encountered, handle it
		if ( !iDeltaWord )
		{
			tDictReader.UnzipOffset();

			if ( ( iWordsTotal%iWordPerCP )!=0 && tDictReader.GetPos()!=iWordsEnd )
				tReporter.Fail ( "unexpected checkpoint (pos=" INT64_FMT ", word=%d, words=%d, expected=%d)", iDictPos, iWordsTotal, ( iWordsTotal%iWordPerCP ), iWordPerCP );

			uWordid = 0;
			iDoclistOffset = 0;
			continue;
		}

		SphWordID_t uNewWordid = 0;
		SphOffset_t iNewDoclistOffset = 0;
		int iDocs = 0;
		int iHits = 0;
		bool bHitless = false;

		if ( bWordDict )
		{
			// unpack next word
			// must be in sync with DictEnd()!
			BYTE uPack = (BYTE)iDeltaWord;
			int iMatch, iDelta;
			if ( uPack & 0x80 )
			{
				iDelta = ( ( uPack>>4 ) & 7 ) + 1;
				iMatch = uPack & 15;
			} else
			{
				iDelta = uPack & 127;
				iMatch = tDictReader.GetByte();
			}
			const int iLastWordLen = strlen(sLastWord);
			if ( iMatch+iDelta>=(int)sizeof(sLastWord)-1 || iMatch>iLastWordLen )
			{
				tReporter.Fail ( "wrong word-delta (pos=" INT64_FMT ", word=%s, len=%d, begin=%d, delta=%d)", iDictPos, sLastWord, iLastWordLen, iMatch, iDelta );
				tDictReader.SkipBytes ( iDelta );
			} else
			{
				tDictReader.GetBytes ( sWord+iMatch, iDelta );
				sWord [ iMatch+iDelta ] = '\0';
			}

			iNewDoclistOffset = tDictReader.UnzipOffset();
			iDocs = tDictReader.UnzipInt();
			iHits = tDictReader.UnzipInt();
			int iHint = 0;
			if ( iDocs>=DOCLIST_HINT_THRESH )
				iHint = tDictReader.GetByte();

			iHint = DoclistHintUnpack ( iDocs, (BYTE)iHint );

			if ( m_tSettings.m_eHitless==SPH_HITLESS_SOME && ( iDocs & HITLESS_DOC_FLAG )!=0 )
			{
				iDocs = ( iDocs & HITLESS_DOC_MASK );
				bHitless = true;
			}

			const int iNewWordLen = strlen(sWord);

			if ( iNewWordLen==0 )
				tReporter.Fail ( "empty word in dictionary (pos=" INT64_FMT ")", iDictPos );

			if ( iLastWordLen && iNewWordLen )
				if ( sphDictCmpStrictly ( sWord, iNewWordLen, sLastWord, iLastWordLen )<=0 )
					tReporter.Fail ( "word order decreased (pos=" INT64_FMT ", word=%s, prev=%s)", iDictPos, sLastWord, sWord );

			if ( iHint<0 )
				tReporter.Fail ( "invalid word hint (pos=" INT64_FMT ", word=%s, hint=%d)", iDictPos, sWord, iHint );

			if ( iDocs<=0 || iHits<=0 || iHits<iDocs )
				tReporter.Fail ( "invalid docs/hits (pos=" INT64_FMT ", word=%s, docs=" INT64_FMT ", hits=" INT64_FMT ")", (int64_t)iDictPos, sWord, (int64_t)iDocs, (int64_t)iHits );

			memcpy ( sLastWord, sWord, sizeof(sLastWord) );
		} else
		{
			// finish reading the entire entry
			uNewWordid = uWordid + iDeltaWord;
			iNewDoclistOffset = iDoclistOffset + tDictReader.UnzipOffset();
			iDocs = tDictReader.UnzipInt();
			iHits = tDictReader.UnzipInt();
			bHitless = ( tCtx.m_dHitlessWords.BinarySearch ( uNewWordid )!=NULL );
			if ( bHitless )
				iDocs = ( iDocs & HITLESS_DOC_MASK );

			if ( uNewWordid<=uWordid )
				tReporter.Fail ( "wordid decreased (pos=" INT64_FMT ", wordid=" UINT64_FMT ", previd=" UINT64_FMT ")", (int64_t)iDictPos, (uint64_t)uNewWordid, (uint64_t)uWordid );

			if ( iNewDoclistOffset<=iDoclistOffset )
				tReporter.Fail ( "doclist offset decreased (pos=" INT64_FMT ", wordid=" UINT64_FMT ")", (int64_t)iDictPos, (uint64_t)uNewWordid );

			if ( iDocs<=0 || iHits<=0 || iHits<iDocs )
				tReporter.Fail ( "invalid docs/hits (pos=" INT64_FMT ", wordid=" UINT64_FMT ", docs=" INT64_FMT ", hits=" INT64_FMT ", hitless=%s)",
					(int64_t)iDictPos, (uint64_t)uNewWordid, (int64_t)iDocs, (int64_t)iHits, ( bHitless?"true":"false" ) );
		}

		assert ( m_tSettings.m_iSkiplistBlockSize>0 );

		// skiplist
		if ( iDocs>m_tSettings.m_iSkiplistBlockSize && !bHitless )
		{
			int iSkipsOffset = tDictReader.UnzipInt();
			if ( !bWordDict && iSkipsOffset<iLastSkipsOffset )
				tReporter.Fail ( "descending skiplist pos (last=%d, cur=%d, wordid=%llu)", iLastSkipsOffset, iSkipsOffset, UINT64 ( uNewWordid ) );

			iLastSkipsOffset = iSkipsOffset;
		}

		// update stats, add checkpoint
		if ( ( iWordsTotal%iWordPerCP )==0 )
		{
			CSphWordlistCheckpoint & tCP = dCheckpoints.Add();
			tCP.m_iWordlistOffset = iDictPos;

			if ( bWordDict )
			{
				const int iLen = strlen ( sWord );
				char * sArenaWord = dCheckpointWords.AddN ( iLen + 1 );
				memcpy ( sArenaWord, sWord, iLen );
				sArenaWord[iLen] = '\0';
				tCP.m_uWordID = sArenaWord - dCheckpointWords.Begin();
			} else
				tCP.m_uWordID = uNewWordid;
		}

		// TODO add back infix checking

		uWordid = uNewWordid;
		iDoclistOffset = iNewDoclistOffset;
		iWordsTotal++;
	}

	// check the checkpoints
	if ( dCheckpoints.GetLength()!=m_tWordlist.m_dCheckpoints.GetLength() )
		tReporter.Fail ( "checkpoint count mismatch (read=%d, calc=%d)", m_tWordlist.m_dCheckpoints.GetLength(), dCheckpoints.GetLength() );

	m_tWordlist.DebugPopulateCheckpoints();
	for ( int i=0; i < Min ( dCheckpoints.GetLength(), m_tWordlist.m_dCheckpoints.GetLength() ); i++ )
	{
		CSphWordlistCheckpoint tRefCP = dCheckpoints[i];
		const CSphWordlistCheckpoint & tCP = m_tWordlist.m_dCheckpoints[i];
		const int iLen = bWordDict ? strlen ( tCP.m_sWord ) : 0;
		if ( bWordDict )
			tRefCP.m_sWord = dCheckpointWords.Begin() + tRefCP.m_uWordID;
		if ( bWordDict && ( tRefCP.m_sWord[0]=='\0' || tCP.m_sWord[0]=='\0' ) )
		{
			tReporter.Fail ( "empty checkpoint %d (read_word=%s, read_len=%u, readpos=" INT64_FMT ", calc_word=%s, calc_len=%u, calcpos=" INT64_FMT ")",
				i, tCP.m_sWord, (DWORD)strlen ( tCP.m_sWord ), (int64_t)tCP.m_iWordlistOffset,
				tRefCP.m_sWord, (DWORD)strlen ( tRefCP.m_sWord ), (int64_t)tRefCP.m_iWordlistOffset );

		} else if ( sphCheckpointCmpStrictly ( tCP.m_sWord, iLen, tCP.m_uWordID, bWordDict, tRefCP ) || tRefCP.m_iWordlistOffset!=tCP.m_iWordlistOffset )
		{
			if ( bWordDict )
			{
				tReporter.Fail ( "checkpoint %d differs (read_word=%s, readpos=" INT64_FMT ", calc_word=%s, calcpos=" INT64_FMT ")",
					i,
					tCP.m_sWord,
					(int64_t)tCP.m_iWordlistOffset,
					tRefCP.m_sWord,
					(int64_t)tRefCP.m_iWordlistOffset );
			} else
			{
				tReporter.Fail ( "checkpoint %d differs (readid=" UINT64_FMT ", readpos=" INT64_FMT ", calcid=" UINT64_FMT ", calcpos=" INT64_FMT ")",
					i,
					(uint64_t)tCP.m_uWordID,
					(int64_t)tCP.m_iWordlistOffset,
					(uint64_t)tRefCP.m_uWordID,
					(int64_t)tRefCP.m_iWordlistOffset );
			}
		}
	}

	dCheckpoints.Reset();
	dCheckpointWords.Reset();
}


void CSphIndex_VLN::DebugCheck_Docs ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter )
{
	///////////////////////
	// check docs and hits
	///////////////////////

	tReporter.Msg ( "checking data..." );

	CSphAutoreader & tDictReader = tCtx.m_tDictReader;
	auto pDocsReader = tCtx.m_pDocsReader;
	auto pHitsReader = tCtx.m_pHitsReader;
	CSphAutoreader & tSkipsReader = tCtx.m_tSkipsReader;

	int64_t iDocsSize = pDocsReader->GetFilesize();
	int64_t iSkiplistLen = tSkipsReader.GetFilesize();

	tDictReader.SeekTo ( 1, READ_NO_SIZE_HINT );
	pDocsReader->SeekTo ( 1 );
	pHitsReader->SeekTo ( 1 );

	SphWordID_t uWordid = 0;
	int64_t iDoclistOffset = 0;
	int iDictDocs, iDictHits;
	bool bHitless = false;

	const bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	char sWord[MAX_KEYWORD_BYTES];
	memset ( sWord, 0, sizeof(sWord) );

	int iWordsChecked = 0;
	int iWordsTotal = 0;

	SphOffset_t iWordsEnd = m_tWordlist.m_iWordsEnd;
	while ( tDictReader.GetPos()<iWordsEnd )
	{
		bHitless = false;
		SphWordID_t iDeltaWord = 0;
		if ( bWordDict )
			iDeltaWord = tDictReader.GetByte();
		else
			iDeltaWord = tDictReader.UnzipWordid();

		if ( !iDeltaWord )
		{
			tDictReader.UnzipOffset();

			uWordid = 0;
			iDoclistOffset = 0;
			continue;
		}

		if ( bWordDict )
		{
			// unpack next word
			// must be in sync with DictEnd()!
			BYTE uPack = (BYTE)iDeltaWord;

			int iMatch, iDelta;
			if ( uPack & 0x80 )
			{
				iDelta = ( ( uPack>>4 ) & 7 ) + 1;
				iMatch = uPack & 15;
			} else
			{
				iDelta = uPack & 127;
				iMatch = tDictReader.GetByte();
			}
			const int iLastWordLen = strlen(sWord);
			if ( iMatch+iDelta>=(int)sizeof(sWord)-1 || iMatch>iLastWordLen )
				tDictReader.SkipBytes ( iDelta );
			else
			{
				tDictReader.GetBytes ( sWord+iMatch, iDelta );
				sWord [ iMatch+iDelta ] = '\0';
			}

			iDoclistOffset = tDictReader.UnzipOffset();
			iDictDocs = tDictReader.UnzipInt();
			iDictHits = tDictReader.UnzipInt();
			if ( iDictDocs>=DOCLIST_HINT_THRESH )
				tDictReader.GetByte();

			if ( m_tSettings.m_eHitless==SPH_HITLESS_SOME && ( iDictDocs & HITLESS_DOC_FLAG ) )
			{
				iDictDocs = ( iDictDocs & HITLESS_DOC_MASK );
				bHitless = true;
			}
		} else
		{
			// finish reading the entire entry
			uWordid = uWordid + iDeltaWord;
			bHitless = ( tCtx.m_dHitlessWords.BinarySearch ( uWordid )!=NULL );
			iDoclistOffset = iDoclistOffset + tDictReader.UnzipOffset();
			iDictDocs = tDictReader.UnzipInt();
			if ( bHitless )
				iDictDocs = ( iDictDocs & HITLESS_DOC_MASK );
			iDictHits = tDictReader.UnzipInt();
		}

		// FIXME? verify skiplist content too
		int iSkipsOffset = 0;
		if ( iDictDocs>m_tSettings.m_iSkiplistBlockSize && !bHitless )
			iSkipsOffset = tDictReader.UnzipInt();

		// check whether the offset is as expected
		if ( iDoclistOffset!=pDocsReader->GetPos() )
		{
			if ( !bWordDict )
				tReporter.Fail ( "unexpected doclist offset (wordid=" UINT64_FMT "(%s)(%d), dictpos=" INT64_FMT ", doclistpos=" INT64_FMT ")",
					(uint64_t)uWordid, sWord, iWordsChecked, iDoclistOffset, (int64_t) pDocsReader->GetPos() );

			if ( iDoclistOffset>=iDocsSize || iDoclistOffset<0 )
			{
				tReporter.Fail ( "unexpected doclist offset, off the file (wordid=" UINT64_FMT "(%s)(%d), dictpos=" INT64_FMT ", doclistsize=" INT64_FMT ")",
					(uint64_t)uWordid, sWord, iWordsChecked, iDoclistOffset, iDocsSize );
				iWordsChecked++;
				continue;
			} else
				pDocsReader->SeekTo ( iDoclistOffset );
		}

		// create and manually setup doclist reader
		DiskIndexQwordTraits_c * pQword = nullptr;
		if ( m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )
			pQword = new DiskIndexQword_c<true,false> ( false, false );
		else
			pQword = new DiskIndexQword_c<false,false> ( false, false );

		pQword->m_tDoc.Reset ( m_tSchema.GetDynamicSize() );
		pQword->m_tDoc.m_tRowID = INVALID_ROWID;
		pQword->m_iDocs = 0;
		pQword->m_iHits = 0;
		pQword->SetDocReader ( pDocsReader );
//		pQword->m_rdDoclist.SeekTo ( tDocsReader.GetPos(), READ_NO_SIZE_HINT );
		pQword->SetHitReader ( pHitsReader );
//		pQword->m_rdHitlist.SeekTo ( tHitsReader.GetPos(), READ_NO_SIZE_HINT );

		// loop the doclist
		int iDoclistDocs = 0;
		int iDoclistHits = 0;
		int iHitlistHits = 0;

		bHitless |= ( m_tSettings.m_eHitless==SPH_HITLESS_ALL ||
			( m_tSettings.m_eHitless==SPH_HITLESS_SOME && tCtx.m_dHitlessWords.BinarySearch ( uWordid ) ) );
		pQword->m_bHasHitlist = !bHitless;

		CSphVector<SkiplistEntry_t> dDoclistSkips;
		while (true)
		{
			// skiplist state is saved just *before* decoding those boundary entries
			if ( ( iDoclistDocs & ( m_tSettings.m_iSkiplistBlockSize-1 ) )==0 )
			{
				SkiplistEntry_t & tBlock = dDoclistSkips.Add();
				tBlock.m_tBaseRowIDPlus1 = pQword->m_tDoc.m_tRowID+1;
				tBlock.m_iOffset = pQword->m_rdDoclist->GetPos();
				tBlock.m_iBaseHitlistPos = pQword->m_uHitPosition;
			}

			// FIXME? this can fail on a broken entry (eg fieldid over 256)
			const CSphMatch & tDoc = pQword->GetNextDoc();
			if ( tDoc.m_tRowID==INVALID_ROWID )
				break;

			// checks!
			if ( tDoc.m_tRowID>m_iDocinfo )
				tReporter.Fail ( "rowid out of bounds (wordid=" UINT64_FMT "(%s), rowid=%u)",	uint64_t(uWordid), sWord, tDoc.m_tRowID );

			iDoclistDocs++;
			iDoclistHits += pQword->m_uMatchHits;

			// check position in case of regular (not-inline) hit
			if (!( pQword->m_iHitlistPos>>63 ))
			{
				if ( !bWordDict && pQword->m_iHitlistPos!=pQword->m_rdHitlist->GetPos() )
					tReporter.Fail ( "unexpected hitlist offset (wordid=" UINT64_FMT "(%s), rowid=%u, expected=" INT64_FMT ", actual=" INT64_FMT ")",
						(uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, (int64_t)pQword->m_iHitlistPos, (int64_t)pQword->m_rdHitlist->GetPos() );
			}

			// aim
			pQword->SeekHitlist ( pQword->m_iHitlistPos );

			// loop the hitlist
			int iDocHits = 0;
			FieldMask_t dFieldMask;
			dFieldMask.UnsetAll();
			Hitpos_t uLastHit = EMPTY_HIT;

			while ( !bHitless )
			{
				Hitpos_t uHit = pQword->GetNextHit();
				if ( uHit==EMPTY_HIT )
					break;

				if ( !( uLastHit<uHit ) )
					tReporter.Fail ( "hit entries sorting order decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit=%u, last=%u)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, uHit, uLastHit );

				if ( HITMAN::GetField ( uLastHit )==HITMAN::GetField ( uHit ) )
				{
					if ( !( HITMAN::GetPos ( uLastHit )<HITMAN::GetPos ( uHit ) ) )
						tReporter.Fail ( "hit decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit=%u, last=%u)",	(uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, HITMAN::GetPos ( uHit ), HITMAN::GetPos ( uLastHit ) );

					if ( HITMAN::IsEnd ( uLastHit ) )
						tReporter.Fail ( "multiple tail hits (wordid=" UINT64_FMT "(%s), rowid=%u, hit=0x%x, last=0x%x)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, uHit, uLastHit );
				} else
				{
					if ( !( HITMAN::GetField ( uLastHit )<HITMAN::GetField ( uHit ) ) )
						tReporter.Fail ( "hit field decreased (wordid=" UINT64_FMT "(%s), rowid=%u, hit field=%u, last field=%u)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, HITMAN::GetField ( uHit ), HITMAN::GetField ( uLastHit ) );
				}

				uLastHit = uHit;

				int iField = HITMAN::GetField ( uHit );
				if ( iField<0 || iField>=SPH_MAX_FIELDS )
					tReporter.Fail ( "hit field out of bounds (wordid=" UINT64_FMT "(%s), rowid=%u, field=%d)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, iField );
				else if ( iField>=m_tSchema.GetFieldsCount() )
					tReporter.Fail ( "hit field out of schema (wordid=" UINT64_FMT "(%s), rowid=%u, field=%d)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, iField );
				else
					dFieldMask.Set(iField);

				iDocHits++; // to check doclist entry
				iHitlistHits++; // to check dictionary entry
			}

			// check hit count
			if ( iDocHits!=(int)pQword->m_uMatchHits && !bHitless )
				tReporter.Fail ( "doc hit count mismatch (wordid=" UINT64_FMT "(%s), rowid=%u, doclist=%d, hitlist=%d)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID, pQword->m_uMatchHits, iDocHits );

			if ( GetMatchSchema().GetFieldsCount()>32 )
				pQword->CollectHitMask();

			// check the mask
			if ( memcmp ( dFieldMask.m_dMask, pQword->m_dQwordFields.m_dMask, sizeof(dFieldMask.m_dMask) ) && !bHitless )
				tReporter.Fail ( "field mask mismatch (wordid=" UINT64_FMT "(%s), rowid=%u)", (uint64_t)uWordid, sWord, pQword->m_tDoc.m_tRowID );

			// update my hitlist reader
			pHitsReader->SeekTo ( pQword->m_rdHitlist->GetPos() );
		}

		// do checks
		if ( iDictDocs!=iDoclistDocs )
			tReporter.Fail ( "doc count mismatch (wordid=" UINT64_FMT "(%s), dict=%d, doclist=%d, hitless=%s)", uint64_t(uWordid), sWord, iDictDocs, iDoclistDocs, ( bHitless?"true":"false" ) );

		if ( ( iDictHits!=iDoclistHits || iDictHits!=iHitlistHits ) && !bHitless )
			tReporter.Fail ( "hit count mismatch (wordid=" UINT64_FMT "(%s), dict=%d, doclist=%d, hitlist=%d)", uint64_t(uWordid), sWord, iDictHits, iDoclistHits, iHitlistHits );

		while ( iDoclistDocs>m_tSettings.m_iSkiplistBlockSize && !bHitless )
		{
			if ( iSkipsOffset<=0 || iSkipsOffset>iSkiplistLen )
			{
				tReporter.Fail ( "invalid skiplist offset (wordid=%llu(%s), off=%d, max=" INT64_FMT ")", UINT64 ( uWordid ), sWord, iSkipsOffset, iSkiplistLen );
				break;
			}

			// boundary adjustment
			if ( ( iDoclistDocs & ( m_tSettings.m_iSkiplistBlockSize-1 ) )==0 )
				dDoclistSkips.Pop();

			SkiplistEntry_t t;
			t.m_tBaseRowIDPlus1 = 0;
			t.m_iOffset = iDoclistOffset;
			t.m_iBaseHitlistPos = 0;

			// hint is: dDoclistSkips * ZIPPED( sizeof(int64_t) * 3 ) == dDoclistSkips * 8
			tSkipsReader.SeekTo ( iSkipsOffset, dDoclistSkips.GetLength ()*8 );
			int i = 0;
			while ( ++i<dDoclistSkips.GetLength() )
			{
				const SkiplistEntry_t & r = dDoclistSkips[i];

				RowID_t tRowIDDelta = tSkipsReader.UnzipRowid();
				uint64_t uOff = tSkipsReader.UnzipOffset();
				uint64_t uPosDelta = tSkipsReader.UnzipOffset();

				if ( tSkipsReader.GetErrorFlag () )
				{
					tReporter.Fail ( "skiplist reading error (wordid=%llu(%s), exp=%d, got=%d, error='%s')", UINT64 ( uWordid ), sWord, i, dDoclistSkips.GetLength (), tSkipsReader.GetErrorMessage ().cstr () );
					tSkipsReader.ResetError();
					break;
				}

				t.m_tBaseRowIDPlus1 += m_tSettings.m_iSkiplistBlockSize + tRowIDDelta;
				t.m_iOffset += 4*m_tSettings.m_iSkiplistBlockSize + uOff;
				t.m_iBaseHitlistPos += uPosDelta;
				if ( t.m_tBaseRowIDPlus1!=r.m_tBaseRowIDPlus1 || t.m_iOffset!=r.m_iOffset || t.m_iBaseHitlistPos!=r.m_iBaseHitlistPos )
				{
					tReporter.Fail ( "skiplist entry %d mismatch (wordid=%llu(%s), exp={%u, %llu, %llu}, got={%u, %llu, %llu})",
						i, UINT64 ( uWordid ), sWord,
						r.m_tBaseRowIDPlus1, UINT64 ( r.m_iOffset ), UINT64 ( r.m_iBaseHitlistPos ),
						t.m_tBaseRowIDPlus1, UINT64 ( t.m_iOffset ), UINT64 ( t.m_iBaseHitlistPos ) );
					break;
				}
			}
			break;
		}

		// move my reader instance forward too
		pDocsReader->SeekTo ( pQword->m_rdDoclist->GetPos() );

		// cleanup
		SafeDelete ( pQword );

		// progress bar
		if ( (++iWordsChecked)%1000==0 )
			tReporter.Progress ( "%d/%d", iWordsChecked, iWordsTotal );
	}
}


void CSphIndex_VLN::DebugCheck_KillList ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter ) const
{
	tReporter.Msg ( "checking kill-list..." );

	CSphString sSPK = GetIndexFileName(SPH_EXT_SPK);
	if ( !sphIsReadable ( sSPK.cstr() ) )
		return;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sSPK.cstr(), sError ) )
	{
		tReporter.Fail ( "unable to open kill-list: %s", sError.cstr() );
		return;
	}

	DWORD nIndexes = tReader.GetDword();
	for ( int i = 0; i < (int)nIndexes; i++ )
	{
		CSphString sIndex = tReader.GetString();
		if ( tReader.GetErrorFlag() )
		{
			tReporter.Fail ( "error reading index name from kill-list: %s", tReader.GetErrorMessage().cstr() );
			return;
		}

		DWORD uFlags = tReader.GetDword();
		DWORD uMask = KillListTarget_t::USE_KLIST | KillListTarget_t::USE_DOCIDS;
		if ( uFlags & (~uMask) )
		{
			tReporter.Fail ( "unknown index flags in kill-list: %u", uMask );
			return;
		}
	}

	DWORD nKills = tReader.GetDword();
	if ( tReader.GetErrorFlag() )
	{
		tReporter.Fail ( "error reading kill-list" );
		return;
	}

	for ( DWORD i = 0; i<nKills; i++ )
	{
		DocID_t tDelta = tReader.UnzipOffset();
		if ( tDelta<=0 )
		{
			tReporter.Fail ( "descending docids found in kill-list" );
			return;
		}

		if ( tReader.GetErrorFlag() )
		{
			tReporter.Fail ( "error docids from kill-list" );
			return;
		}
	}
}


void CSphIndex_VLN::DebugCheck_BlockIndex ( DebugCheckContext_t & tCtx, DebugCheckError_c & tReporter )
{
	///////////////////////////
	// check blocks index
	///////////////////////////

	tReporter.Msg ( "checking attribute blocks index..." );

	int64_t iAllRowsTotal = m_iDocinfo + (m_iDocinfoIndex+1)*2;
	DWORD uStride = m_tSchema.GetRowSize();
	int64_t iLoadedRowItems = tCtx.m_tAttrReader.GetFilesize() / sizeof(CSphRowitem);
	if ( iAllRowsTotal*uStride > iLoadedRowItems )
		tReporter.Fail ( "rowitems count mismatch (expected=" INT64_FMT ", loaded=" INT64_FMT ")", iAllRowsTotal*uStride, iLoadedRowItems );

	// check size
	const int64_t iTempDocinfoIndex = ( m_iDocinfo+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;
	if ( iTempDocinfoIndex!=m_iDocinfoIndex )
		tReporter.Fail ( "block count differs (expected=" INT64_FMT ", got=" INT64_FMT ")", iTempDocinfoIndex, m_iDocinfoIndex );

	CSphFixedVector<CSphRowitem> dRow ( m_tSchema.GetRowSize() );
	const CSphRowitem * pRow = dRow.Begin();
	tCtx.m_tAttrReader.SeekTo ( 0, dRow.GetLengthBytes() );

	const int64_t iMinMaxEnd = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * ( m_iDocinfoIndex+1 ) * uStride * 2;
	CSphFixedVector<DWORD> dMinMax ( uStride*2 );
	const DWORD * pMinEntry = dMinMax.Begin();
	const DWORD * pMinAttrs = pMinEntry;
	const DWORD * pMaxAttrs = pMinAttrs + uStride;

	for ( int64_t iIndexEntry=0; iIndexEntry<m_iDocinfo; iIndexEntry++ )
	{
		const int64_t iBlock = iIndexEntry / DOCINFO_INDEX_FREQ;

		// we have to do some checks in border cases, for example: when move from 1st to 2nd block
		const int64_t iPrevEntryBlock = ( iIndexEntry-1 )/DOCINFO_INDEX_FREQ;
		const bool bIsBordersCheckTime = ( iPrevEntryBlock!=iBlock );
		if ( bIsBordersCheckTime || iIndexEntry==0 )
		{
			int64_t iPos = tCtx.m_tAttrReader.GetPos();

			int64_t iBlockPos = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * iBlock * uStride * 2;
			// check docid vs global range
			if ( int64_t( iBlockPos + sizeof(DWORD) * uStride) > iMinMaxEnd )
				tReporter.Fail ( "unexpected block index end (row=" INT64_FMT ", block=" INT64_FMT ")", iIndexEntry, iBlock );

			tCtx.m_tAttrReader.SeekTo ( iBlockPos, dMinMax.GetLengthBytes() );
			tCtx.m_tAttrReader.GetBytes ( dMinMax.Begin(), dMinMax.GetLengthBytes() );
			if ( tCtx.m_tAttrReader.GetErrorFlag() )
				tReporter.Fail ( "unexpected block index (row=" INT64_FMT ", block=" INT64_FMT ")", iIndexEntry, iBlock );

			tCtx.m_tAttrReader.SeekTo ( iPos, dRow.GetLengthBytes() );
		}

		tCtx.m_tAttrReader.GetBytes ( dRow.Begin(), dRow.GetLengthBytes() );
		const DocID_t tDocID = sphGetDocID(pRow);

		// check values vs blocks range
		for ( int iItem=0; iItem < m_tSchema.GetAttrsCount(); iItem++ )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iItem);
			if ( tCol.m_sName==sphGetBlobLocatorName() )
				continue;

			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_BIGINT:
			{
				const SphAttr_t uVal = sphGetRowAttr ( pRow, tCol.m_tLocator );
				const SphAttr_t uMin = sphGetRowAttr ( pMinAttrs, tCol.m_tLocator );
				const SphAttr_t uMax = sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator );

				// checks is attribute min max range valid
				if ( uMin > uMax && bIsBordersCheckTime )
					tReporter.Fail ( "invalid attribute range (row=" INT64_FMT ", block=" INT64_FMT ", min=" INT64_FMT ", max=" INT64_FMT ")", iIndexEntry, iBlock, uMin, uMax );

				if ( uVal < uMin || uVal > uMax )
					tReporter.Fail ( "unexpected attribute value (row=" INT64_FMT ", attr=%u, docid=" INT64_FMT ", block=" INT64_FMT ", value=0x" UINT64_FMT ", min=0x" UINT64_FMT ", max=0x" UINT64_FMT ")",
						iIndexEntry, iItem, tDocID, iBlock, uint64_t(uVal), uint64_t(uMin), uint64_t(uMax) );
			}
			break;

			case SPH_ATTR_FLOAT:
			{
				const float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, tCol.m_tLocator ) );
				const float fMin = sphDW2F ( (DWORD)sphGetRowAttr ( pMinAttrs, tCol.m_tLocator ) );
				const float fMax = sphDW2F ( (DWORD)sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator ) );

				// checks is attribute min max range valid
				if ( fMin > fMax && bIsBordersCheckTime )
					tReporter.Fail ( "invalid attribute range (row=" INT64_FMT ", block=" INT64_FMT ", min=%f, max=%f)", iIndexEntry, iBlock, fMin, fMax );

				if ( fVal < fMin || fVal > fMax )
					tReporter.Fail ( "unexpected attribute value (row=" INT64_FMT ", attr=%u, docid=" INT64_FMT ", block=" INT64_FMT ", value=%f, min=%f, max=%f)", iIndexEntry, iItem, tDocID, iBlock, fVal, fMin, fMax );
			}
			break;

			default:
				break;
			}
		}

		// progress bar
		if ( iIndexEntry%1000==0 )
			tReporter.Progress ( INT64_FMT"/" INT64_FMT, iIndexEntry, m_iDocinfo );
	}
}


int CSphIndex_VLN::DebugCheck ( FILE * fp )
{
	DebugCheckContext_t tCtx;
	DebugCheckError_c tReporter(fp);

	// check if index is ready
	if ( !m_bPassedAlloc )
		tReporter.Fail ( "index not preread" );

	//////////////
	// open files
	//////////////

	CSphString sError;
	if ( !tCtx.m_tDictReader.Open ( GetIndexFileName(SPH_EXT_SPI), sError ) )
		tReporter.Fail ( "unable to open dictionary: %s", sError.cstr() );

	// use file reader during debug check to lower memory pressure
	tCtx.m_pDocsReader = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), sError,
		DataReaderFabric_c::eDocs, m_tFiles.m_iReadBufferDocList, true );
	if ( !tCtx.m_pDocsReader )
		tReporter.Fail ( "unable to open doclist: %s", sError.cstr() );

	// use file reader during debug check to lower memory pressure
	tCtx.m_pHitsReader = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), sError,
		DataReaderFabric_c::eHits, m_tFiles.m_iReadBufferHitList, true );
	if ( !tCtx.m_pHitsReader )
		tReporter.Fail ( "unable to open hitlist: %s", sError.cstr() );

	if ( !tCtx.m_tSkipsReader.Open ( GetIndexFileName(SPH_EXT_SPE), sError ) )
		tReporter.Fail ( "unable to open skiplist: %s", sError.cstr () );

	if ( !tCtx.m_tDeadRowReader.Open ( GetIndexFileName(SPH_EXT_SPM).cstr(), sError ) )
		tReporter.Fail ( "unable to open dead-row map: %s", sError.cstr() );

	if ( !tCtx.m_tAttrReader.Open ( GetIndexFileName(SPH_EXT_SPA).cstr(), sError ) )
		tReporter.Fail ( "unable to open attributes: %s", sError.cstr() );

	if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		if ( !tCtx.m_tBlobReader.Open ( GetIndexFileName(SPH_EXT_SPB), sError ) )
			tReporter.Fail ( "unable to open blobs: %s", sError.cstr() );
		else
			tCtx.m_bHasBlobs = true;
	}

	if ( !LoadHitlessWords ( tCtx.m_dHitlessWords ) )
		tReporter.Fail ( "unable to load hitless words: %s", m_sLastError.cstr() );

	CSphSavedFile tStat;
	const CSphTokenizerSettings & tTokenizerSettings = m_pTokenizer->GetSettings ();
	if ( !tTokenizerSettings.m_sSynonymsFile.IsEmpty() && !GetFileStats ( tTokenizerSettings.m_sSynonymsFile.cstr(), tStat, &sError ) )
		tReporter.Fail ( "unable to open exceptions '%s': %s", tTokenizerSettings.m_sSynonymsFile.cstr(), sError.cstr() );

	const CSphDictSettings & tDictSettings = m_pDict->GetSettings ();
	const char * pStop = tDictSettings.m_sStopwords.cstr();
	while (true)
	{
		// find next name start
		while ( pStop && *pStop && isspace(*pStop) ) pStop++;
		if ( !pStop || !*pStop ) break;

		const char * sNameStart = pStop;

		// find next name end
		while ( *pStop && !isspace(*pStop) ) pStop++;

		CSphString sStopFile;
		sStopFile.SetBinary ( sNameStart, pStop-sNameStart );

		if ( !GetFileStats ( sStopFile.cstr(), tStat, &sError ) )
			tReporter.Fail ( "unable to open stopwords '%s': %s", sStopFile.cstr(), sError.cstr() );
	}

	if ( !tDictSettings.m_dWordforms.GetLength() )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
		{
			if ( !GetFileStats ( tDictSettings.m_dWordforms[i].cstr(), tStat, &sError ) )
				tReporter.Fail ( "unable to open wordforms '%s': %s", tDictSettings.m_dWordforms[i].cstr(), sError.cstr() );
		}
	}

	///////////////////////////
	// perform checks
	///////////////////////////

	DebugCheck_Dictionary ( tCtx, tReporter );
	DebugCheck_Docs ( tCtx, tReporter );

	// common code with RT index
	const int64_t iMinMaxStart = sizeof(DWORD) * m_iMinMaxIndex;
	const int64_t iMinMaxEnd = sizeof(DWORD) * m_iMinMaxIndex + sizeof(DWORD) * ( m_iDocinfoIndex+1 ) * m_tSchema.GetRowSize() * 2;
	const int64_t iMinMaxBytes = iMinMaxEnd - iMinMaxStart;
	FileDebugCheckReader_c tAttrReader ( &tCtx.m_tAttrReader );
	FileDebugCheckReader_c tBlobReader ( tCtx.m_bHasBlobs ? &tCtx.m_tBlobReader : nullptr );
	DebugCheck_Attributes ( tAttrReader, tBlobReader, m_iDocinfo, iMinMaxBytes, m_tSchema, tReporter );

	DebugCheck_BlockIndex ( tCtx, tReporter );
	DebugCheck_KillList ( tCtx, tReporter );
	// more common code
	DebugCheck_DeadRowMap ( tCtx.m_tDeadRowReader.GetFilesize(), m_iDocinfo, tReporter );

	///////////////////////////
	// all finished
	///////////////////////////

	tReporter.Done();

	return (int)Min ( tReporter.GetNumFails(), 255 ); // this is the exitcode; so cap it
} // NOLINT function length


//////////////////////////////////////////////////////////////////////////

/// morphology
enum
{
	SPH_MORPH_STEM_EN,
	SPH_MORPH_STEM_RU_UTF8,
	SPH_MORPH_STEM_CZ,
	SPH_MORPH_STEM_AR_UTF8,
	SPH_MORPH_SOUNDEX,
	SPH_MORPH_METAPHONE_UTF8,
	SPH_MORPH_AOTLEMMER_BASE,
	SPH_MORPH_AOTLEMMER_RU_UTF8 = SPH_MORPH_AOTLEMMER_BASE,
	SPH_MORPH_AOTLEMMER_EN,
	SPH_MORPH_AOTLEMMER_DE_UTF8,
	SPH_MORPH_AOTLEMMER_BASE_ALL,
	SPH_MORPH_AOTLEMMER_RU_ALL = SPH_MORPH_AOTLEMMER_BASE_ALL,
	SPH_MORPH_AOTLEMMER_EN_ALL,
	SPH_MORPH_AOTLEMMER_DE_ALL,
	SPH_MORPH_LIBSTEMMER_FIRST,
	SPH_MORPH_LIBSTEMMER_LAST = SPH_MORPH_LIBSTEMMER_FIRST + 64
};


/////////////////////////////////////////////////////////////////////////////
// BASE DICTIONARY INTERFACE
/////////////////////////////////////////////////////////////////////////////

void CSphDict::DictBegin ( CSphAutofile &, CSphAutofile &, int )						{}
void CSphDict::DictEntry ( const CSphDictEntry & )										{}
void CSphDict::DictEndEntries ( SphOffset_t )											{}
bool CSphDict::DictEnd ( DictHeader_t *, int, CSphString & )							{ return true; }
bool CSphDict::DictIsError () const														{ return true; }

/////////////////////////////////////////////////////////////////////////////
// CRC32/64 DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

struct CSphTemplateDictTraits : CSphDict
{
	CSphTemplateDictTraits ();
protected:
				~CSphTemplateDictTraits () override;

public:
	void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final;
	void		LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final;
	void		WriteStopwords ( CSphWriter & tWriter ) const final;
	bool		LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) final;
	void		WriteWordforms ( CSphWriter & tWriter ) const final;
	const CSphWordforms *	GetWordforms() final { return m_pWordforms; }
	void		DisableWordforms() final { m_bDisableWordforms = true; }
	int			SetMorphology ( const char * szMorph, CSphString & sMessage ) final;
	bool		HasMorphology() const final;
	void		ApplyStemmers ( BYTE * pWord ) const final;

	void		Setup ( const CSphDictSettings & tSettings ) final { m_tSettings = tSettings; }
	const CSphDictSettings & GetSettings () const final { return m_tSettings; }
	const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () const final { return m_dSWFileInfos; }
	const CSphVector <CSphSavedFile> & GetWordformsFileInfos () const final { return m_dWFFileInfos; }
	const CSphMultiformContainer * GetMultiWordforms () const final;
	uint64_t	GetSettingsFNV () const final;
	static void			SweepWordformContainers ( const CSphVector<CSphSavedFile> & dFiles );

protected:
	CSphVector < int >	m_dMorph;
#if USE_LIBSTEMMER
	CSphVector < sb_stemmer * >	m_dStemmers;
	StrVec_t			m_dDescStemmers;
#endif

	int					m_iStopwords;	///< stopwords count
	SphWordID_t *		m_pStopwords;	///< stopwords ID list
	CSphFixedVector<SphWordID_t> m_dStopwordContainer;

protected:
	int					ParseMorphology ( const char * szMorph, CSphString & sError );
	SphWordID_t			FilterStopword ( SphWordID_t uID ) const;	///< filter ID against stopwords list
	CSphDict *			CloneBase ( CSphTemplateDictTraits * pDict ) const;
	bool				HasState () const final;

	bool				m_bDisableWordforms;

private:
	CSphWordforms *				m_pWordforms;
	CSphVector<CSphSavedFile>	m_dSWFileInfos;
	CSphVector<CSphSavedFile>	m_dWFFileInfos;
	CSphDictSettings			m_tSettings;

	static CSphVector<CSphWordforms*>		m_dWordformContainers;

	CSphWordforms *		GetWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos, const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex );
	CSphWordforms *		LoadWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos, const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex );

	int					InitMorph ( const char * szMorph, int iLength, CSphString & sError );
	int					AddMorph ( int iMorph ); ///< helper that always returns ST_OK
	bool				StemById ( BYTE * pWord, int iStemmer ) const;
	void				AddWordform ( CSphWordforms * pContainer, char * sBuffer, int iLen, ISphTokenizer * pTokenizer, const char * szFile, const CSphVector<int> & dBlended, int iFileId );
};

CSphVector<CSphWordforms*> CSphTemplateDictTraits::m_dWordformContainers;


/// common CRC32/64 dictionary stuff
struct CSphDiskDictTraits : CSphTemplateDictTraits
{
	void DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit ) override;
	void DictEntry ( const CSphDictEntry & tEntry ) override;
	void DictEndEntries ( SphOffset_t iDoclistOffset ) override;
	bool DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError ) override;
	bool DictIsError () const final { return m_wrDict.IsError(); }
	void SetSkiplistBlockSize ( int iSize ) final { m_iSkiplistBlockSize=iSize; }

protected:
	~CSphDiskDictTraits () override {} // fixme! remove

protected:
	CSphTightVector<CSphWordlistCheckpoint>	m_dCheckpoints;		///< checkpoint offsets
	CSphWriter			m_wrDict;			///< final dict file writer
	CSphString			m_sWriterError;		///< writer error message storage
	int					m_iEntries = 0;			///< dictionary entries stored
	SphOffset_t			m_iLastDoclistPos = 0;
	SphWordID_t			m_iLastWordID = 0;
	int					m_iSkiplistBlockSize = 0;
};


template < bool CRCALGO >
struct CCRCEngine
{
	inline static SphWordID_t		DoCrc ( const BYTE * pWord );
	inline static SphWordID_t		DoCrc ( const BYTE * pWord, int iLen );
};

/// specialized CRC32/64 implementations
template < bool CRC32DICT >
struct CSphDictCRC : public CSphDiskDictTraits, CCRCEngine<CRC32DICT>
{
	typedef CCRCEngine<CRC32DICT> tHASH;
	SphWordID_t		GetWordID ( BYTE * pWord ) override;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) override;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) override;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) override;
	bool			IsStopWord ( const BYTE * pWord ) const final;

	CSphDict *		Clone () const override { return CloneBase ( new CSphDictCRC<CRC32DICT>() ); }
protected:
	~CSphDictCRC() override {} // fixme! remove
};

struct CSphDictTemplate : public CSphTemplateDictTraits, CCRCEngine<false> // based on flv64
{
	SphWordID_t		GetWordID ( BYTE * pWord ) final;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) final;
	bool			IsStopWord ( const BYTE * pWord ) const final;

	CSphDict *		Clone () const final { return CloneBase ( new CSphDictTemplate() ); }

protected:
	~CSphDictTemplate () final 	{} // fixme! remove
};


/////////////////////////////////////////////////////////////////////////////

uint64_t sphFNV64 ( const void * s )
{
	return sphFNV64cont ( s, SPH_FNV64_SEED );
}


uint64_t sphFNV64 ( const void * s, int iLen, uint64_t uPrev )
{
	const BYTE * p = (const BYTE*)s;
	uint64_t hval = uPrev;
	for ( ; iLen>0; iLen-- )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*p++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}


uint64_t sphFNV64cont ( const void * s, uint64_t uPrev )
{
	const BYTE * p = (const BYTE*)s;
	if ( !p )
		return uPrev;

	uint64_t hval = uPrev;
	while ( *p )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*p++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}

/////////////////////////////////////////////////////////////////////////////

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


static bool GetFileStats ( const char * szFilename, CSphSavedFile & tInfo, CSphString * pError )
{
	if ( !szFilename || !*szFilename )
	{
		memset ( &tInfo, 0, sizeof ( tInfo ) );
		return true;
	}

	tInfo.m_sFilename = szFilename;

	struct_stat tStat = {0};
	if ( stat ( szFilename, &tStat ) < 0 )
	{
		if ( pError )
			*pError = strerrorm ( errno );
		return false;
	}

	tInfo.m_uSize = tStat.st_size;
	tInfo.m_uCTime = tStat.st_ctime;
	tInfo.m_uMTime = tStat.st_mtime;

	DWORD uCRC32 = 0;
	if ( !sphCalcFileCRC32 ( szFilename, uCRC32 ) )
		return false;

	tInfo.m_uCRC32 = uCRC32;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphWordforms::CSphWordforms()
	: m_iRefCount ( 0 )
	, m_uTokenizerFNV ( 0 )
	, m_bHavePostMorphNF ( false )
	, m_pMultiWordforms ( NULL )
{
}


CSphWordforms::~CSphWordforms()
{
	if ( m_pMultiWordforms )
	{
		m_pMultiWordforms->m_Hash.IterateStart ();
		while ( m_pMultiWordforms->m_Hash.IterateNext () )
		{
			CSphMultiforms * pWordforms = m_pMultiWordforms->m_Hash.IterateGet ();
			ARRAY_FOREACH ( i, pWordforms->m_pForms )
				SafeDelete ( pWordforms->m_pForms[i] );

			SafeDelete ( pWordforms );
		}

		SafeDelete ( m_pMultiWordforms );
	}
}


bool CSphWordforms::IsEqual ( const CSphVector<CSphSavedFile> & dFiles )
{
	if ( m_dFiles.GetLength()!=dFiles.GetLength() )
		return false;

	ARRAY_FOREACH ( i, m_dFiles )
	{
		const CSphSavedFile & tF1 = m_dFiles[i];
		const CSphSavedFile & tF2 = dFiles[i];
		if ( tF1.m_sFilename!=tF2.m_sFilename || tF1.m_uCRC32!=tF2.m_uCRC32 || tF1.m_uSize!=tF2.m_uSize ||
			tF1.m_uCTime!=tF2.m_uCTime || tF1.m_uMTime!=tF2.m_uMTime )
			return false;
	}

	return true;
}


bool CSphWordforms::ToNormalForm ( BYTE * pWord, bool bBefore, bool bOnlyCheck ) const
{
	int * pIndex = m_dHash ( (char *)pWord );
	if ( !pIndex )
		return false;

	if ( *pIndex<0 || *pIndex>=m_dNormalForms.GetLength () )
		return false;

	if ( bBefore==m_dNormalForms[*pIndex].m_bAfterMorphology )
		return false;

	if ( m_dNormalForms[*pIndex].m_sWord.IsEmpty() )
		return false;

	if ( bOnlyCheck )
		return true;

	strcpy ( (char *)pWord, m_dNormalForms[*pIndex].m_sWord.cstr() ); // NOLINT
	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphTemplateDictTraits::CSphTemplateDictTraits ()
	: m_iStopwords	( 0 )
	, m_pStopwords	( NULL )
	, m_dStopwordContainer ( 0 )
	, m_bDisableWordforms ( false )
	, m_pWordforms	( NULL )
{
}


CSphTemplateDictTraits::~CSphTemplateDictTraits ()
{
#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers[i] );
#endif

	if ( m_pWordforms )
		--m_pWordforms->m_iRefCount;
}


SphWordID_t CSphTemplateDictTraits::FilterStopword ( SphWordID_t uID ) const
{
	if ( !m_iStopwords )
		return uID;

	// OPTIMIZE: binary search is not too good, could do some hashing instead
	SphWordID_t * pStart = m_pStopwords;
	SphWordID_t * pEnd = m_pStopwords + m_iStopwords - 1;
	do
	{
		if ( uID==*pStart || uID==*pEnd )
			return 0;

		if ( uID<*pStart || uID>*pEnd )
			return uID;

		SphWordID_t * pMid = pStart + (pEnd-pStart)/2;
		if ( uID==*pMid )
			return 0;

		if ( uID<*pMid )
			pEnd = pMid;
		else
			pStart = pMid;
	} while ( pEnd-pStart>1 );

	return uID;
}


int CSphTemplateDictTraits::ParseMorphology ( const char * sMorph, CSphString & sMessage )
{
	int iRes = ST_OK;
	for ( const char * sStart=sMorph; ; )
	{
		while ( *sStart && ( sphIsSpace ( *sStart ) || *sStart==',' ) )
			++sStart;
		if ( !*sStart )
			break;

		const char * sWordStart = sStart;
		while ( *sStart && !sphIsSpace ( *sStart ) && *sStart!=',' )
			++sStart;

		if ( sStart > sWordStart )
		{
			switch ( InitMorph ( sWordStart, sStart - sWordStart, sMessage ) )
			{
				case ST_ERROR:		return ST_ERROR;
				case ST_WARNING:	iRes = ST_WARNING;
				default:			break;
			}
		}
	}
	return iRes;
}


int CSphTemplateDictTraits::InitMorph ( const char * szMorph, int iLength, CSphString & sMessage )
{
	if ( iLength==0 )
		return ST_OK;

	if ( iLength==4 && !strncmp ( szMorph, "none", iLength ) )
		return ST_OK;

	if ( iLength==7 && !strncmp ( szMorph, "stem_en", iLength ) )
	{
		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_EN ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_EN_ALL ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en_all clash" );
			return ST_ERROR;
		}

		stem_en_init ();
		return AddMorph ( SPH_MORPH_STEM_EN );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_ru", iLength ) )
	{
		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_RU_UTF8 ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_RU_ALL ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
			return ST_ERROR;
		}

		stem_ru_init ();
		return AddMorph ( SPH_MORPH_STEM_RU_UTF8 );
	}

	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf[20];
		char buf_all[20];
		sprintf ( buf, "lemmatize_%s", AOT_LANGUAGES[j] ); // NOLINT
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT

		if ( iLength==12 && !strncmp ( szMorph, buf, iLength ) )
		{
			if ( j==AOT_RU && m_dMorph.Contains ( SPH_MORPH_STEM_RU_UTF8 ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
				return ST_ERROR;
			}

			if ( j==AOT_EN && m_dMorph.Contains ( SPH_MORPH_STEM_EN ) )
			{
				sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
				return ST_ERROR;
			}

			// no test for SPH_MORPH_STEM_DE since we doesn't have it.

			if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_BASE_ALL+j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			CSphString sDictFile;
			sDictFile.SetSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			// add manually instead of AddMorph(), because we need to update that fingerprint
			int iMorph = j + SPH_MORPH_AOTLEMMER_BASE;
			if ( j==AOT_RU )
				iMorph = SPH_MORPH_AOTLEMMER_RU_UTF8;
			else if ( j==AOT_DE )
				iMorph = SPH_MORPH_AOTLEMMER_DE_UTF8;

			if ( !m_dMorph.Contains ( iMorph ) )
			{
				if ( m_sMorphFingerprint.IsEmpty() )
					m_sMorphFingerprint.SetSprintf ( "%s:%08x"
						, sphAotDictinfo(j).m_sName.cstr()
						, sphAotDictinfo(j).m_iValue );
				else
					m_sMorphFingerprint.SetSprintf ( "%s;%s:%08x"
					, m_sMorphFingerprint.cstr()
					, sphAotDictinfo(j).m_sName.cstr()
					, sphAotDictinfo(j).m_iValue );
				m_dMorph.Add ( iMorph );
			}
			return ST_OK;
		}

		if ( iLength==16 && !strncmp ( szMorph, buf_all, iLength ) )
		{
			if ( j==AOT_RU && ( m_dMorph.Contains ( SPH_MORPH_STEM_RU_UTF8 ) ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
				return ST_ERROR;
			}

			if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_BASE+j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			CSphString sDictFile;
			sDictFile.SetSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			return AddMorph ( SPH_MORPH_AOTLEMMER_BASE_ALL+j );
		}
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_cz", iLength ) )
	{
		stem_cz_init ();
		return AddMorph ( SPH_MORPH_STEM_CZ );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_ar", iLength ) )
		return AddMorph ( SPH_MORPH_STEM_AR_UTF8 );

	if ( iLength==9 && !strncmp ( szMorph, "stem_enru", iLength ) )
	{
		stem_en_init ();
		stem_ru_init ();
		AddMorph ( SPH_MORPH_STEM_EN );
		return AddMorph ( SPH_MORPH_STEM_RU_UTF8 );
	}

	if ( iLength==7 && !strncmp ( szMorph, "soundex", iLength ) )
		return AddMorph ( SPH_MORPH_SOUNDEX );

	if ( iLength==9 && !strncmp ( szMorph, "metaphone", iLength ) )
		return AddMorph ( SPH_MORPH_METAPHONE_UTF8 );

#if USE_LIBSTEMMER
	const int LIBSTEMMER_LEN = 11;
	const int MAX_ALGO_LENGTH = 64;
	if ( iLength > LIBSTEMMER_LEN && iLength - LIBSTEMMER_LEN < MAX_ALGO_LENGTH && !strncmp ( szMorph, "libstemmer_", LIBSTEMMER_LEN ) )
	{
		CSphString sAlgo;
		sAlgo.SetBinary ( szMorph+LIBSTEMMER_LEN, iLength - LIBSTEMMER_LEN );

		sb_stemmer * pStemmer = NULL;

		pStemmer = sb_stemmer_new ( sAlgo.cstr(), "UTF_8" );

		if ( !pStemmer )
		{
			sMessage.SetSprintf ( "unknown stemmer libstemmer_%s; skipped", sAlgo.cstr() );
			return ST_WARNING;
		}

		AddMorph ( SPH_MORPH_LIBSTEMMER_FIRST + m_dStemmers.GetLength () );
		ARRAY_FOREACH ( i, m_dStemmers )
		{
			if ( m_dStemmers[i]==pStemmer )
			{
				sb_stemmer_delete ( pStemmer );
				return ST_OK;
			}
		}

		m_dStemmers.Add ( pStemmer );
		m_dDescStemmers.Add ( sAlgo );
		return ST_OK;
	}
#endif

	if ( iLength==11 && !strncmp ( szMorph, "rlp_chinese", iLength ) )
		return ST_OK;

	if ( iLength==19 && !strncmp ( szMorph, "rlp_chinese_batched", iLength ) )
		return ST_OK;

	sMessage.SetBinary ( szMorph, iLength );
	sMessage.SetSprintf ( "unknown stemmer %s; skipped", sMessage.cstr() );
	return ST_WARNING;
}


int CSphTemplateDictTraits::AddMorph ( int iMorph )
{
	if ( !m_dMorph.Contains ( iMorph ) )
		m_dMorph.Add ( iMorph );
	return ST_OK;
}



void CSphTemplateDictTraits::ApplyStemmers ( BYTE * pWord ) const
{
	// try wordforms
	if ( m_pWordforms && m_pWordforms->ToNormalForm ( pWord, true, m_bDisableWordforms ) )
		return;

	// check length
	if ( m_tSettings.m_iMinStemmingLen<=1 || sphUTF8Len ( (const char*)pWord )>=m_tSettings.m_iMinStemmingLen )
	{
		// try stemmers
		ARRAY_FOREACH ( i, m_dMorph )
			if ( StemById ( pWord, m_dMorph[i] ) )
				break;
	}

	if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
		m_pWordforms->ToNormalForm ( pWord, false, m_bDisableWordforms );
}

const CSphMultiformContainer * CSphTemplateDictTraits::GetMultiWordforms () const
{
	return m_pWordforms ? m_pWordforms->m_pMultiWordforms : NULL;
}

uint64_t CSphTemplateDictTraits::GetSettingsFNV () const
{
	uint64_t uHash = (uint64_t)m_pWordforms;

	if ( m_pStopwords )
		uHash = sphFNV64 ( m_pStopwords, m_iStopwords*sizeof(*m_pStopwords), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iMinStemmingLen, sizeof(m_tSettings.m_iMinStemmingLen), uHash );
	DWORD uFlags = 0;
	if ( m_tSettings.m_bWordDict )
		uFlags |= 1<<0;
	if ( m_tSettings.m_bStopwordsUnstemmed )
		uFlags |= 1<<2;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );

	uHash = sphFNV64 ( m_dMorph.Begin(), m_dMorph.GetLength()*sizeof(m_dMorph[0]), uHash );
#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dDescStemmers )
		uHash = sphFNV64 ( m_dDescStemmers[i].cstr(), m_dDescStemmers[i].Length(), uHash );
#endif

	return uHash;
}


CSphDict * CSphTemplateDictTraits::CloneBase ( CSphTemplateDictTraits * pDict ) const
{
	assert ( pDict );
	pDict->m_tSettings = m_tSettings;
	pDict->m_iStopwords = m_iStopwords;
	pDict->m_pStopwords = m_pStopwords;
	pDict->m_dSWFileInfos = m_dSWFileInfos;
	pDict->m_dWFFileInfos = m_dWFFileInfos;
	pDict->m_pWordforms = m_pWordforms;
	if ( m_pWordforms )
		m_pWordforms->m_iRefCount++;

	pDict->m_dMorph = m_dMorph;
#if USE_LIBSTEMMER
	assert ( m_dDescStemmers.GetLength()==m_dStemmers.GetLength() );
	pDict->m_dDescStemmers = m_dDescStemmers;
	ARRAY_FOREACH ( i, m_dDescStemmers )
	{
		pDict->m_dStemmers.Add ( sb_stemmer_new ( m_dDescStemmers[i].cstr(), "UTF_8" ) );
		assert ( pDict->m_dStemmers.Last() );
	}
#endif

	return pDict;
}

bool CSphTemplateDictTraits::HasState() const
{
#if !USE_LIBSTEMMER
	return false;
#else
	return ( m_dDescStemmers.GetLength()>0 );
#endif
}

/////////////////////////////////////////////////////////////////////////////

template<>
SphWordID_t CCRCEngine<true>::DoCrc ( const BYTE * pWord )
{
	return sphCRC32 ( pWord );
}


template<>
SphWordID_t CCRCEngine<false>::DoCrc ( const BYTE * pWord )
{
	return (SphWordID_t) sphFNV64 ( pWord );
}


template<>
SphWordID_t CCRCEngine<true>::DoCrc ( const BYTE * pWord, int iLen )
{
	return sphCRC32 ( pWord, iLen );
}


template<>
SphWordID_t CCRCEngine<false>::DoCrc ( const BYTE * pWord, int iLen )
{
	return (SphWordID_t) sphFNV64 ( pWord, iLen );
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordID ( BYTE * pWord )
{
	// apply stopword filter before stemmers
	if ( GetSettings().m_bStopwordsUnstemmed && !FilterStopword ( tHASH::DoCrc ( pWord ) ) )
		return 0;

	// skip stemmers for magic words
	if ( pWord[0]>=0x20 )
		ApplyStemmers ( pWord );

	// stemmer might squeeze out the word
	if ( !pWord[0] )
		return 0;

	return GetSettings().m_bStopwordsUnstemmed
		? tHASH::DoCrc ( pWord )
		: FilterStopword ( tHASH::DoCrc ( pWord ) );
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = tHASH::DoCrc ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord + 1 );
	int iLength = strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? tHASH::DoCrc ( pWord ) : 0;
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordIDNonStemmed ( BYTE * pWord )
{
	// this method can generally receive both '=stopword' with a marker and 'stopword' without it
	// so for filtering stopwords, let's handle both
	int iOff = ( pWord[0]<' ' );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord+iOff );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return tHASH::DoCrc ( pWord );
}


template < bool CRC32DICT >
bool CSphDictCRC<CRC32DICT>::IsStopWord ( const BYTE * pWord ) const
{
	return FilterStopword ( tHASH::DoCrc ( pWord ) )==0;
}


//////////////////////////////////////////////////////////////////////////
SphWordID_t CSphDictTemplate::GetWordID ( BYTE * pWord )
{
	// apply stopword filter before stemmers
	if ( GetSettings().m_bStopwordsUnstemmed && !FilterStopword ( DoCrc ( pWord ) ) )
		return 0;

	// skip stemmers for magic words
	if ( pWord[0]>=0x20 )
		ApplyStemmers ( pWord );

	return GetSettings().m_bStopwordsUnstemmed
		? DoCrc ( pWord )
		: FilterStopword ( DoCrc ( pWord ) );
}


SphWordID_t CSphDictTemplate::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = DoCrc ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


SphWordID_t CSphDictTemplate::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	// stemmer might squeeze out the word
	if ( !pWord[1] )
		return 0;
	SphWordID_t uWordId = DoCrc ( pWord + 1 );
	int iLength = strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? DoCrc ( pWord ) : 0;
}


SphWordID_t CSphDictTemplate::GetWordIDNonStemmed ( BYTE * pWord )
{
	SphWordID_t uWordId = DoCrc ( pWord + 1 );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return DoCrc ( pWord );
}

bool CSphDictTemplate::IsStopWord ( const BYTE * pWord ) const
{
	return FilterStopword ( DoCrc ( pWord ) )==0;
}

//////////////////////////////////////////////////////////////////////////

void CSphTemplateDictTraits::LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile )
{
	assert ( !m_pStopwords );
	assert ( !m_iStopwords );

	// tokenize file list
	if ( !sFiles || !*sFiles )
		return;

	m_dSWFileInfos.Resize ( 0 );

	ISphTokenizerRefPtr_c tTokenizer ( pTokenizer->Clone ( SPH_CLONE_INDEX ) );
	CSphFixedVector<char> dList ( 1+strlen(sFiles) );
	strcpy ( dList.Begin(), sFiles ); // NOLINT

	char * pCur = dList.Begin();
	char * sName = NULL;

	CSphVector<SphWordID_t> dStop;

	while (true)
	{
		// find next name start
		while ( *pCur && ( isspace(*pCur) || *pCur==',' ) ) pCur++;
		if ( !*pCur ) break;
		sName = pCur;

		// find next name end
		while ( *pCur && !( isspace(*pCur) || *pCur==',' ) ) pCur++;
		if ( *pCur ) *pCur++ = '\0';

		CSphString sFileName = sName;
		bool bGotFile = sphIsReadable ( sFileName );
		if ( !bGotFile )
		{
			if ( bStripFile )
			{
				StripPath ( sFileName );
				bGotFile = sphIsReadable ( sFileName );
			}
			if ( !bGotFile )
			{
				if ( !bStripFile )
					StripPath ( sFileName );
				sFileName.SetSprintf ( "%s/stopwords/%s", FULL_SHARE_DIR, sFileName.cstr() );
				bGotFile = sphIsReadable ( sFileName );
			}
		}

		CSphFixedVector<BYTE> dBuffer ( 0 );
		CSphSavedFile tInfo;
		GetFileStats ( sFileName.cstr(), tInfo, NULL );

		// need to store original name to compatible with original behavior of load order
		// from path defined; from tool CWD; from SHARE_DIR
		tInfo.m_sFilename = sName; 
		m_dSWFileInfos.Add ( tInfo );

		if ( !bGotFile )
		{
			StringBuilder_c sError;
			sError.Appendf ( "failed to load stopwords from either '%s' or '%s'", sName, sFileName.cstr() );
			if ( bStripFile )
				sError += ", current work directory";
			sphWarn ( "%s", sError.cstr() );
			continue;
		}

		// open file
		FILE * fp = fopen ( sFileName.cstr(), "rb" );
		if ( !fp )
		{
			sphWarn ( "failed to load stopwords from '%s'", sFileName.cstr() );
			continue;
		}

		struct_stat st = {0};
		if ( fstat ( fileno (fp) , &st )==0 )
			dBuffer.Reset ( st.st_size );
		else
		{
			fclose ( fp );
			sphWarn ( "stopwords: failed to get file size for '%s'", sFileName.cstr() );
			continue;
		}

		// tokenize file
		int iLength = (int)fread ( dBuffer.Begin(), 1, (size_t)st.st_size, fp );

		BYTE * pToken;
		tTokenizer->SetBuffer ( dBuffer.Begin(), iLength );
		while ( ( pToken = tTokenizer->GetToken() )!=NULL )
			if ( m_tSettings.m_bStopwordsUnstemmed )
				dStop.Add ( GetWordIDNonStemmed ( pToken ) );
			else
				dStop.Add ( GetWordID ( pToken ) );

		// close file
		fclose ( fp );
	}

	// sort stopwords
	dStop.Uniq();

	// store IDs
	if ( dStop.GetLength() )
	{
		m_dStopwordContainer.Reset ( dStop.GetLength() );
		ARRAY_FOREACH ( i, dStop )
			m_dStopwordContainer[i] = dStop[i];

		m_iStopwords = m_dStopwordContainer.GetLength ();
		m_pStopwords = m_dStopwordContainer.Begin();
	}
}


void CSphTemplateDictTraits::LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords )
{
	m_dStopwordContainer.Reset ( dStopwords.GetLength() );
	ARRAY_FOREACH ( i, dStopwords )
		m_dStopwordContainer[i] = dStopwords[i];

	m_iStopwords = m_dStopwordContainer.GetLength ();
	m_pStopwords = m_dStopwordContainer.Begin();
}


void CSphTemplateDictTraits::WriteStopwords ( CSphWriter & tWriter ) const
{
	tWriter.PutDword ( (DWORD)m_iStopwords );
	for ( int i = 0; i < m_iStopwords; i++ )
		tWriter.ZipOffset ( m_pStopwords[i] );
}


void CSphTemplateDictTraits::SweepWordformContainers ( const CSphVector<CSphSavedFile> & dFiles )
{
	for ( int i = 0; i < m_dWordformContainers.GetLength (); )
	{
		CSphWordforms * WC = m_dWordformContainers[i];
		if ( WC->m_iRefCount==0 && !WC->IsEqual ( dFiles ) )
		{
			delete WC;
			m_dWordformContainers.Remove ( i );
		} else
			++i;
	}
}


static const int MAX_REPORT_LEN = 1024;

void AddStringToReport ( CSphString & sReport, const CSphString & sString, bool bLast )
{
	int iLen = sReport.Length();
	if ( iLen + sString.Length() + 2 > MAX_REPORT_LEN )
		return;

	char * szReport = (char *)sReport.cstr();
	strcat ( szReport, sString.cstr() );	// NOLINT
	iLen += sString.Length();
	if ( bLast )
		szReport[iLen] = '\0';
	else
	{
		szReport[iLen] = ' ';
		szReport[iLen+1] = '\0';
	}
}


void ConcatReportStrings ( const CSphTightVector<CSphString> & dStrings, CSphString & sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*(char *)sReport.cstr() = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i], i==dStrings.GetLength()-1 );
}


void ConcatReportStrings ( const CSphTightVector<CSphNormalForm> & dStrings, CSphString & sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*(char *)sReport.cstr() = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i].m_sForm, i==dStrings.GetLength()-1 );
}


CSphWordforms * CSphTemplateDictTraits::GetWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos,
	const StrVec_t * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	uint64_t uTokenizerFNV = pTokenizer->GetSettingsFNV();
	ARRAY_FOREACH ( i, m_dWordformContainers )
		if ( m_dWordformContainers[i]->IsEqual ( dFileInfos ) )
		{
			CSphWordforms * pContainer = m_dWordformContainers[i];
			if ( uTokenizerFNV==pContainer->m_uTokenizerFNV )
				return pContainer;

			CSphTightVector<CSphString> dErrorReport;
			ARRAY_FOREACH ( j, dFileInfos )
				dErrorReport.Add ( dFileInfos[j].m_sFilename );

			CSphString sAllFiles;
			ConcatReportStrings ( dErrorReport, sAllFiles );
			sphWarning ( "index '%s': wordforms file '%s' is shared with index '%s', "
				"but tokenizer settings are different",
				sIndex, sAllFiles.cstr(), pContainer->m_sIndexName.cstr() );
		}

	CSphWordforms * pContainer = LoadWordformContainer ( dFileInfos, pEmbedded, pTokenizer, sIndex );
	if ( pContainer )
		m_dWordformContainers.Add ( pContainer );

	return pContainer;
}


struct CmpMultiforms_fn
{
	inline bool IsLess ( const CSphMultiform * pA, const CSphMultiform * pB ) const
	{
		assert ( pA && pB );
		if ( pA->m_iFileId==pB->m_iFileId )
			return pA->m_dTokens.GetLength() > pB->m_dTokens.GetLength();

		return pA->m_iFileId > pB->m_iFileId;
	}
};


void CSphTemplateDictTraits::AddWordform ( CSphWordforms * pContainer, char * sBuffer, int iLen,
	ISphTokenizer * pTokenizer, const char * szFile, const CSphVector<int> & dBlended, int iFileId )
{
	StrVec_t dTokens;

	bool bSeparatorFound = false;
	bool bAfterMorphology = false;

	// parse the line
	pTokenizer->SetBuffer ( (BYTE*)sBuffer, iLen );

	bool bFirstToken = true;
	bool bStopwordsPresent = false;
	bool bCommentedWholeLine = false;

	BYTE * pFrom = NULL;
	while ( ( pFrom = pTokenizer->GetToken () )!=NULL )
	{
		if ( *pFrom=='#' )
		{
			bCommentedWholeLine = bFirstToken;
			break;
		}

		if ( *pFrom=='~' && bFirstToken )
		{
			bAfterMorphology = true;
			bFirstToken = false;
			continue;
		}

		bFirstToken = false;

		if ( *pFrom=='>' )
		{
			bSeparatorFound = true;
			break;
		}

		if ( *pFrom=='=' && *pTokenizer->GetBufferPtr()=='>' )
		{
			pTokenizer->GetToken();
			bSeparatorFound = true;
			break;
		}

		if ( GetWordID ( pFrom, strlen ( (const char*)pFrom ), true ) )
			dTokens.Add ( (const char*)pFrom );
		else
			bStopwordsPresent = true;
	}

	if ( !dTokens.GetLength() )
	{
		if ( !bCommentedWholeLine )
			sphWarning ( "index '%s': all source tokens are stopwords (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( !bSeparatorFound )
	{
		sphWarning ( "index '%s': no wordform separator found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	BYTE * pTo = pTokenizer->GetToken ();
	if ( !pTo )
	{
		sphWarning ( "index '%s': no destination token found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( *pTo=='#' )
	{
		sphWarning ( "index '%s': misplaced comment (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	CSphVector<CSphNormalForm> dDestTokens;
	bool bFirstDestIsStop = !GetWordID ( pTo, strlen ( (const char*)pTo ), true );
	CSphNormalForm & tForm = dDestTokens.Add();
	tForm.m_sForm = (const char *)pTo;
	tForm.m_iLengthCP = pTokenizer->GetLastTokenLen();

	// what if we have more than one word in the right part?
	const BYTE * pDestToken;
	while ( ( pDestToken = pTokenizer->GetToken() )!=NULL )
	{
		bool bStop = ( !GetWordID ( pDestToken, strlen ( (const char*)pDestToken ), true ) );
		if ( !bStop )
		{
			CSphNormalForm & tNewForm = dDestTokens.Add();
			tNewForm.m_sForm = (const char *)pDestToken;
			tNewForm.m_iLengthCP = pTokenizer->GetLastTokenLen();
		}

		bStopwordsPresent |= bStop;
	}

	// we can have wordforms with 1 destination token that is a stopword
	if ( dDestTokens.GetLength()>1 && bFirstDestIsStop )
		dDestTokens.Remove(0);

	if ( !dDestTokens.GetLength() )
	{
		sphWarning ( "index '%s': destination token is a stopword (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( bStopwordsPresent )
		sphWarning ( "index '%s': wordform contains stopwords (wordform='%s'). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );

	// we disabled all blended, so we need to filter them manually
	bool bBlendedPresent = false;
	if ( dBlended.GetLength() )
		ARRAY_FOREACH ( i, dDestTokens )
		{
			int iCode;
			const BYTE * pBuf = (const BYTE *) dDestTokens[i].m_sForm.cstr();
			while ( ( iCode = sphUTF8Decode ( pBuf ) )>0 && !bBlendedPresent )
				bBlendedPresent = ( dBlended.BinarySearch ( iCode )!=NULL );
		}

	if ( bBlendedPresent )
		sphWarning ( "invalid mapping (destination contains blended characters) (wordform='%s'). Fix your wordforms file '%s'.", sBuffer, szFile );

	if ( bBlendedPresent && dDestTokens.GetLength()>1 )
	{
		sphWarning ( "blended characters are not allowed with multiple destination tokens (wordform='%s', file='%s'). IGNORED.", sBuffer, szFile );
		return;
	}

	if ( dTokens.GetLength()>1 || dDestTokens.GetLength()>1 )
	{
		CSphMultiform * pMultiWordform = new CSphMultiform;
		pMultiWordform->m_iFileId = iFileId;
		pMultiWordform->m_dNormalForm.Resize ( dDestTokens.GetLength() );
		ARRAY_FOREACH ( i, dDestTokens )
			pMultiWordform->m_dNormalForm[i] = dDestTokens[i];

		for ( int i = 1; i < dTokens.GetLength(); i++ )
			pMultiWordform->m_dTokens.Add ( dTokens[i] );

		if ( !pContainer->m_pMultiWordforms )
			pContainer->m_pMultiWordforms = new CSphMultiformContainer;

		CSphMultiforms ** pWordforms = pContainer->m_pMultiWordforms->m_Hash ( dTokens[0] );
		if ( pWordforms )
		{
			ARRAY_FOREACH ( iMultiform, (*pWordforms)->m_pForms )
			{
				CSphMultiform * pStoredMF = (*pWordforms)->m_pForms[iMultiform];
				if ( pStoredMF->m_dTokens.GetLength()==pMultiWordform->m_dTokens.GetLength() )
				{
					bool bSameTokens = true;
					ARRAY_FOREACH_COND ( iToken, pStoredMF->m_dTokens, bSameTokens )
						if ( pStoredMF->m_dTokens[iToken]!=pMultiWordform->m_dTokens[iToken] )
							bSameTokens = false;

					if ( bSameTokens )
					{
						CSphString sStoredTokens, sStoredForms;
						ConcatReportStrings ( pStoredMF->m_dTokens, sStoredTokens );
						ConcatReportStrings ( pStoredMF->m_dNormalForm, sStoredForms );
						sphWarning ( "index '%s': duplicate wordform found - overridden ( current='%s', old='%s %s > %s' ). Fix your wordforms file '%s'.",
							pContainer->m_sIndexName.cstr(), sBuffer, dTokens[0].cstr(), sStoredTokens.cstr(), sStoredForms.cstr(), szFile );

						pStoredMF->m_dNormalForm.Resize ( pMultiWordform->m_dNormalForm.GetLength() );
						ARRAY_FOREACH ( iForm, pMultiWordform->m_dNormalForm )
							pStoredMF->m_dNormalForm[iForm] = pMultiWordform->m_dNormalForm[iForm];

						pStoredMF->m_iFileId = iFileId;

						SafeDelete ( pMultiWordform );
						break; // otherwise, we crash next turn
					}
				}
			}

			if ( pMultiWordform )
			{
				(*pWordforms)->m_pForms.Add ( pMultiWordform );

				// sort forms by files and length
				// but do not sort if we're loading embedded
				if ( iFileId>=0 )
					(*pWordforms)->m_pForms.Sort ( CmpMultiforms_fn() );

				( *pWordforms )->m_iMinTokens = Min ( ( *pWordforms )->m_iMinTokens, pMultiWordform->m_dTokens.GetLength () );
				( *pWordforms )->m_iMaxTokens = Max ( ( *pWordforms )->m_iMaxTokens, pMultiWordform->m_dTokens.GetLength () );
				pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, (*pWordforms)->m_iMaxTokens );
			}
		} else
		{
			CSphMultiforms * pNewWordforms = new CSphMultiforms;
			pNewWordforms->m_pForms.Add ( pMultiWordform );
			pNewWordforms->m_iMinTokens = pMultiWordform->m_dTokens.GetLength ();
			pNewWordforms->m_iMaxTokens = pMultiWordform->m_dTokens.GetLength ();
			pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pNewWordforms->m_iMaxTokens );
			pContainer->m_pMultiWordforms->m_Hash.Add ( pNewWordforms, dTokens[0] );
		}

		// let's add destination form to regular wordform to keep destination from being stemmed
		// FIXME!!! handle multiple destination tokens and ~flag for wordforms
		if ( !bAfterMorphology && dDestTokens.GetLength()==1 && !pContainer->m_dHash.Exists ( dDestTokens[0].m_sForm ) )
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				|| pContainer->m_dNormalForms.Last().m_sWord!=dDestTokens[0].m_sForm
				|| pContainer->m_dNormalForms.Last().m_bAfterMorphology!=bAfterMorphology )
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_dHash.Add ( pContainer->m_dNormalForms.GetLength()-1, dDestTokens[0].m_sForm );
		}
	} else
	{
		if ( bAfterMorphology )
		{
			BYTE pBuf [16+3*SPH_MAX_WORD_LEN];
			memcpy ( pBuf, dTokens[0].cstr(), dTokens[0].Length()+1 );
			ApplyStemmers ( pBuf );
			dTokens[0] = (char *)pBuf;
		}

		// check wordform that source token is a new token or has same destination token
		int * pRefTo = pContainer->m_dHash ( dTokens[0] );
		assert ( !pRefTo || ( *pRefTo>=0 && *pRefTo<pContainer->m_dNormalForms.GetLength() ) );
		if ( pRefTo )
		{
			// replace with a new wordform
			if ( pContainer->m_dNormalForms[*pRefTo].m_sWord!=dDestTokens[0].m_sForm || pContainer->m_dNormalForms[*pRefTo].m_bAfterMorphology!=bAfterMorphology )
			{
				CSphStoredNF & tRefTo = pContainer->m_dNormalForms[*pRefTo];
				sphWarning ( "index '%s': duplicate wordform found - overridden ( current='%s', old='%s%s > %s' ). Fix your wordforms file '%s'.",
					pContainer->m_sIndexName.cstr(), sBuffer, tRefTo.m_bAfterMorphology ? "~" : "", dTokens[0].cstr(), tRefTo.m_sWord.cstr(), szFile );

				tRefTo.m_sWord = dDestTokens[0].m_sForm;
				tRefTo.m_bAfterMorphology = bAfterMorphology;
				pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			} else
				sphWarning ( "index '%s': duplicate wordform found ( '%s' ). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		} else
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				|| pContainer->m_dNormalForms.Last().m_sWord!=dDestTokens[0].m_sForm
				|| pContainer->m_dNormalForms.Last().m_bAfterMorphology!=bAfterMorphology)
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_dHash.Add ( pContainer->m_dNormalForms.GetLength()-1, dTokens[0] );
		}
	}
}


CSphWordforms * CSphTemplateDictTraits::LoadWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos,
	const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	// allocate it
	CSphWordforms * pContainer = new CSphWordforms();
	pContainer->m_dFiles = dFileInfos;
	pContainer->m_uTokenizerFNV = pTokenizer->GetSettingsFNV();
	pContainer->m_sIndexName = sIndex;

	ISphTokenizerRefPtr_c pMyTokenizer ( pTokenizer->Clone ( SPH_CLONE_INDEX ) );
	const CSphTokenizerSettings & tSettings = pMyTokenizer->GetSettings();
	CSphVector<int> dBlended;

	// get a list of blend chars and set add them to the tokenizer as simple chars
	if ( tSettings.m_sBlendChars.Length() )
	{
		CSphVector<char> dNewCharset;
		dNewCharset.Append ( tSettings.m_sCaseFolding.cstr (), tSettings.m_sCaseFolding.Length ());

		CSphVector<CSphRemapRange> dRemaps;
		CSphCharsetDefinitionParser tParser;
		if ( tParser.Parse ( tSettings.m_sBlendChars.cstr(), dRemaps ) )
			ARRAY_FOREACH ( i, dRemaps )
				for ( int j = dRemaps[i].m_iStart; j<=dRemaps[i].m_iEnd; j++ )
				{
					dNewCharset.Add ( ',' );
					dNewCharset.Add ( ' ' );
					dNewCharset.Add ( char(j) );
					dBlended.Add ( j );
				}

		dNewCharset.Add(0);

		// sort dBlended for binary search
		dBlended.Sort ();

		CSphString sError;
		pMyTokenizer->SetCaseFolding ( dNewCharset.Begin(), sError );

		// disable blend chars
		pMyTokenizer->SetBlendChars ( NULL, sError );
	}

	// add wordform-specific specials
	pMyTokenizer->AddSpecials ( "#=>~" );

	if ( pEmbeddedWordforms )
	{
		CSphTightVector<CSphString> dFilenames;
		dFilenames.Resize ( dFileInfos.GetLength() );
		ARRAY_FOREACH ( i, dFileInfos )
			dFilenames[i] = dFileInfos[i].m_sFilename;

		CSphString sAllFiles;
		ConcatReportStrings ( dFilenames, sAllFiles );

		for ( auto & sWordForm : (*pEmbeddedWordforms) )
			AddWordform ( pContainer, (char*)sWordForm.cstr(),
				sWordForm.Length(), pMyTokenizer, sAllFiles.cstr(), dBlended, -1 );
	} else
	{
		char sBuffer [ 6*SPH_MAX_WORD_LEN + 512 ]; // enough to hold 2 UTF-8 words, plus some whitespace overhead

		ARRAY_FOREACH ( i, dFileInfos )
		{
			CSphAutoreader rdWordforms;
			const char * szFile = dFileInfos[i].m_sFilename.cstr();
			CSphString sError;
			if ( !rdWordforms.Open ( szFile, sError ) )
			{
				sphWarning ( "index '%s': %s", sIndex, sError.cstr() );
				SafeDelete ( pContainer );
				return nullptr;
			}

			int iLen;
			while ( ( iLen = rdWordforms.GetLine ( sBuffer, sizeof(sBuffer) ) )>=0 )
				AddWordform ( pContainer, sBuffer, iLen, pMyTokenizer, szFile, dBlended, i );
		}
	}

	return pContainer;
}


bool CSphTemplateDictTraits::LoadWordforms ( const StrVec_t & dFiles,
	const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	if ( pEmbedded )
	{
		m_dWFFileInfos.Resize ( pEmbedded->m_dWordformFiles.GetLength() );
		ARRAY_FOREACH ( i, m_dWFFileInfos )
			m_dWFFileInfos[i] = pEmbedded->m_dWordformFiles[i];
	} else
	{
		m_dWFFileInfos.Reserve ( dFiles.GetLength() );
		CSphSavedFile tFile;
		ARRAY_FOREACH ( i, dFiles )
			if ( !dFiles[i].IsEmpty() )
			{
				if ( GetFileStats ( dFiles[i].cstr(), tFile, NULL ) )
					m_dWFFileInfos.Add ( tFile );
				else
					sphWarning ( "index '%s': wordforms file '%s' not found", sIndex, dFiles[i].cstr() );
			}
	}

	if ( !m_dWFFileInfos.GetLength() )
		return false;

	SweepWordformContainers ( m_dWFFileInfos );

	m_pWordforms = GetWordformContainer ( m_dWFFileInfos, pEmbedded ? &(pEmbedded->m_dWordforms) : NULL, pTokenizer, sIndex );
	if ( m_pWordforms )
	{
		m_pWordforms->m_iRefCount++;
		if ( m_pWordforms->m_bHavePostMorphNF && !m_dMorph.GetLength() )
			sphWarning ( "index '%s': wordforms contain post-morphology normal forms, but no morphology was specified", sIndex );
	}

	return !!m_pWordforms;
}


void CSphTemplateDictTraits::WriteWordforms ( CSphWriter & tWriter ) const
{
	if ( !m_pWordforms )
	{
		tWriter.PutDword(0);
		return;
	}

	int nMultiforms = 0;
	if ( m_pWordforms->m_pMultiWordforms )
	{
		CSphMultiformContainer::CSphMultiformHash & tHash = m_pWordforms->m_pMultiWordforms->m_Hash;
		tHash.IterateStart();
		while ( tHash.IterateNext() )
		{
			CSphMultiforms * pMF = tHash.IterateGet();
			nMultiforms += pMF ? pMF->m_pForms.GetLength() : 0;
		}
	}

	tWriter.PutDword ( m_pWordforms->m_dHash.GetLength()+nMultiforms );
	m_pWordforms->m_dHash.IterateStart();
	while ( m_pWordforms->m_dHash.IterateNext() )
	{
		const CSphString & sKey = m_pWordforms->m_dHash.IterateGetKey();
		int iIndex = m_pWordforms->m_dHash.IterateGet();
		CSphString sLine;
		sLine.SetSprintf ( "%s%s > %s", m_pWordforms->m_dNormalForms[iIndex].m_bAfterMorphology ? "~" : "",
			sKey.cstr(), m_pWordforms->m_dNormalForms[iIndex].m_sWord.cstr() );
		tWriter.PutString ( sLine );
	}

	if ( m_pWordforms->m_pMultiWordforms )
	{
		CSphMultiformContainer::CSphMultiformHash & tHash = m_pWordforms->m_pMultiWordforms->m_Hash;
		tHash.IterateStart();
		while ( tHash.IterateNext() )
		{
			const CSphString & sKey = tHash.IterateGetKey();
			CSphMultiforms * pMF = tHash.IterateGet();
			if ( !pMF )
				continue;

			ARRAY_FOREACH ( i, pMF->m_pForms )
			{
				CSphString sLine, sTokens, sForms;
				ConcatReportStrings ( pMF->m_pForms[i]->m_dTokens, sTokens );
				ConcatReportStrings ( pMF->m_pForms[i]->m_dNormalForm, sForms );

				sLine.SetSprintf ( "%s %s > %s", sKey.cstr(), sTokens.cstr(), sForms.cstr() );
				tWriter.PutString ( sLine );
			}
		}
	}
}


int CSphTemplateDictTraits::SetMorphology ( const char * szMorph, CSphString & sMessage )
{
	m_dMorph.Reset ();
#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers[i] );
	m_dStemmers.Reset ();
#endif

	if ( !szMorph )
		return ST_OK;

	CSphString sOption = szMorph;
	sOption.ToLower ();

	CSphString sError;
	int iRes = ParseMorphology ( sOption.cstr(), sMessage );
	if ( iRes==ST_WARNING && sMessage.IsEmpty() )
		sMessage.SetSprintf ( "invalid morphology option %s; skipped", sOption.cstr() );
	return iRes;
}


bool CSphTemplateDictTraits::HasMorphology() const
{
	return ( m_dMorph.GetLength()>0 );
}


/// common id-based stemmer
bool CSphTemplateDictTraits::StemById ( BYTE * pWord, int iStemmer ) const
{
	char szBuf [ MAX_KEYWORD_BYTES ];

	// safe quick strncpy without (!) padding and with a side of strlen
	char * p = szBuf;
	char * pMax = szBuf + sizeof(szBuf) - 1;
	BYTE * pLastSBS = NULL;
	while ( *pWord && p<pMax )
	{
		pLastSBS = ( *pWord )<0x80 ? pWord : pLastSBS;
		*p++ = *pWord++;
	}
	int iLen = p - szBuf;
	*p = '\0';
	pWord -= iLen;

	switch ( iStemmer )
	{
	case SPH_MORPH_STEM_EN:
		stem_en ( pWord, iLen );
		break;

	case SPH_MORPH_STEM_RU_UTF8:
		// skip stemming in case of SBC at the end of the word
		if ( pLastSBS && ( pLastSBS-pWord+1 )>=iLen )
			break;

		// stem only UTF8 tail
		if ( !pLastSBS )
		{
			stem_ru_utf8 ( (WORD*)pWord );
		} else
		{
			stem_ru_utf8 ( (WORD *)( pLastSBS+1 ) );
		}
		break;

	case SPH_MORPH_STEM_CZ:
		stem_cz ( pWord );
		break;

	case SPH_MORPH_STEM_AR_UTF8:
		stem_ar_utf8 ( pWord );
		break;

	case SPH_MORPH_SOUNDEX:
		stem_soundex ( pWord );
		break;

	case SPH_MORPH_METAPHONE_UTF8:
		stem_dmetaphone ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_RU_UTF8:
		sphAotLemmatizeRuUTF8 ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_EN:
		sphAotLemmatize ( pWord, AOT_EN );
		break;

	case SPH_MORPH_AOTLEMMER_DE_UTF8:
		sphAotLemmatizeDeUTF8 ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_RU_ALL:
	case SPH_MORPH_AOTLEMMER_EN_ALL:
	case SPH_MORPH_AOTLEMMER_DE_ALL:
		// do the real work somewhere else
		// this is mostly for warning suppressing and making some features like
		// index_exact_words=1 vs expand_keywords=1 work
		break;

	default:
#if USE_LIBSTEMMER
		if ( iStemmer>=SPH_MORPH_LIBSTEMMER_FIRST && iStemmer<SPH_MORPH_LIBSTEMMER_LAST )
		{
			sb_stemmer * pStemmer = m_dStemmers [iStemmer - SPH_MORPH_LIBSTEMMER_FIRST];
			assert ( pStemmer );

			const sb_symbol * sStemmed = sb_stemmer_stem ( pStemmer, (sb_symbol*)pWord, strlen ( (const char*)pWord ) );
			int iLen = sb_stemmer_length ( pStemmer );

			memcpy ( pWord, sStemmed, iLen );
			pWord[iLen] = '\0';
		} else
			return false;

	break;
#else
		return false;
#endif
	}

	return strcmp ( (char *)pWord, szBuf )!=0;
}

void CSphDiskDictTraits::DictBegin ( CSphAutofile & , CSphAutofile & tDict, int )
{
	m_wrDict.CloseFile ();
	m_wrDict.SetFile ( tDict, NULL, m_sWriterError );
	m_wrDict.PutByte ( 1 );
}

bool CSphDiskDictTraits::DictEnd ( DictHeader_t * pHeader, int, CSphString & sError )
{
	// flush wordlist checkpoints
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength();
	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		assert ( m_dCheckpoints[i].m_iWordlistOffset );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_uWordID );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_iWordlistOffset );
	}

	// done
	m_wrDict.CloseFile ();
	if ( m_wrDict.IsError() )
		sError = m_sWriterError;
	return !m_wrDict.IsError();
}

void CSphDiskDictTraits::DictEntry ( const CSphDictEntry & tEntry )
{
	assert ( m_iSkiplistBlockSize>0 );

	// insert wordlist checkpoint
	if ( ( m_iEntries % SPH_WORDLIST_CHECKPOINT )==0 )
	{
		if ( m_iEntries ) // but not the 1st entry
		{
			assert ( tEntry.m_iDoclistOffset > m_iLastDoclistPos );
			m_wrDict.ZipInt ( 0 ); // indicate checkpoint
			m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos ); // store last length
		}

		// restart delta coding, once per SPH_WORDLIST_CHECKPOINT entries
		m_iLastWordID = 0;
		m_iLastDoclistPos = 0;

		// begin new wordlist entry
		assert ( m_wrDict.GetPos()<=UINT_MAX );

		CSphWordlistCheckpoint & tCheckpoint = m_dCheckpoints.Add();
		tCheckpoint.m_uWordID = tEntry.m_uWordID;
		tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();
	}

	assert ( tEntry.m_iDoclistOffset>m_iLastDoclistPos );
	m_wrDict.ZipOffset ( tEntry.m_uWordID - m_iLastWordID ); // FIXME! slow with 32bit wordids
	m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos );

	m_iLastWordID = tEntry.m_uWordID;
	m_iLastDoclistPos = tEntry.m_iDoclistOffset;

	assert ( tEntry.m_iDocs );
	assert ( tEntry.m_iHits );
	m_wrDict.ZipInt ( tEntry.m_iDocs );
	m_wrDict.ZipInt ( tEntry.m_iHits );

	// write skiplist location info, if any
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		m_wrDict.ZipOffset ( tEntry.m_iSkiplistOffset );

	m_iEntries++;
}

void CSphDiskDictTraits::DictEndEntries ( SphOffset_t iDoclistOffset )
{
	assert ( iDoclistOffset>=m_iLastDoclistPos );
	m_wrDict.ZipInt ( 0 ); // indicate checkpoint
	m_wrDict.ZipOffset ( iDoclistOffset - m_iLastDoclistPos ); // store last doclist length
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY, INFIX HASH BUILDER
//////////////////////////////////////////////////////////////////////////

template < int SIZE >
struct Infix_t
{
	DWORD m_Data[SIZE];

#ifndef NDEBUG
	BYTE m_TrailingZero;

	Infix_t ()
		: m_TrailingZero ( 0 )
	{}
#endif

	void Reset ()
	{
		for ( int i=0; i<SIZE; i++ )
			m_Data[i] = 0;
	}

	bool operator == ( const Infix_t<SIZE> & rhs ) const;
};


template<>
bool Infix_t<2>::operator == ( const Infix_t<2> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1];
}


template<>
bool Infix_t<3>::operator == ( const Infix_t<3> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1] && m_Data[2]==rhs.m_Data[2];
}


template<>
bool Infix_t<5>::operator == ( const Infix_t<5> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1] && m_Data[2]==rhs.m_Data[2]
		&& m_Data[3]==rhs.m_Data[3] && m_Data[4]==rhs.m_Data[4];
}


struct InfixIntvec_t
{
public:
	// do not change the order of fields in this union - it matters a lot
	union
	{
		DWORD			m_dData[4];
		struct
		{
			int				m_iDynLen;
			int				m_iDynLimit;
			DWORD *			m_pDynData;
		};
	};

public:
	InfixIntvec_t()
	{
		m_dData[0] = 0;
		m_dData[1] = 0;
		m_dData[2] = 0;
		m_dData[3] = 0;
	}

	~InfixIntvec_t()
	{
		if ( IsDynamic() )
			SafeDeleteArray ( m_pDynData );
	}

	bool IsDynamic() const
	{
		return ( m_dData[0] & 0x80000000UL )!=0;
	}

	void Add ( DWORD uVal )
	{
		if ( !m_dData[0] )
		{
			// empty
			m_dData[0] = uVal | ( 1UL<<24 );

		} else if ( !IsDynamic() )
		{
			// 1..4 static entries
			int iLen = m_dData[0] >> 24;
			DWORD uLast = m_dData [ iLen-1 ] & 0xffffffUL;

			// redundant
			if ( uVal==uLast )
				return;

			// grow static part
			if ( iLen<4 )
			{
				m_dData[iLen] = uVal;
				m_dData[0] = ( m_dData[0] & 0xffffffUL ) | ( ++iLen<<24 );
				return;
			}

			// dynamize
			DWORD * pDyn = new DWORD[16];
			pDyn[0] = m_dData[0] & 0xffffffUL;
			pDyn[1] = m_dData[1];
			pDyn[2] = m_dData[2];
			pDyn[3] = m_dData[3];
			pDyn[4] = uVal;
			m_iDynLen = 0x80000005UL; // dynamic flag, len=5
			m_iDynLimit = 16; // limit=16
			m_pDynData = pDyn;

		} else
		{
			// N dynamic entries
			int iLen = m_iDynLen & 0xffffffUL;
			if ( uVal==m_pDynData[iLen-1] )
				return;
			if ( iLen>=m_iDynLimit )
			{
				m_iDynLimit *= 2;
				DWORD * pNew = new DWORD [ m_iDynLimit ];
				for ( int i=0; i<iLen; i++ )
					pNew[i] = m_pDynData[i];
				SafeDeleteArray ( m_pDynData );
				m_pDynData = pNew;
			}

			m_pDynData[iLen] = uVal;
			m_iDynLen++;
		}
	}

	bool operator == ( const InfixIntvec_t & rhs ) const
	{
		// check dynflag, length, maybe first element
		if ( m_dData[0]!=rhs.m_dData[0] )
			return false;

		// check static data
		if ( !IsDynamic() )
		{
			for ( int i=1; i<(int)(m_dData[0]>>24); i++ )
				if ( m_dData[i]!=rhs.m_dData[i] )
					return false;
			return true;
		}

		// check dynamic data
		const DWORD * a = m_pDynData;
		const DWORD * b = rhs.m_pDynData;
		const DWORD * m = a + ( m_iDynLen & 0xffffffUL );
		while ( a<m )
			if ( *a++!=*b++ )
				return false;
		return true;
	}

public:
	int GetLength() const
	{
		if ( !IsDynamic() )
			return m_dData[0] >> 24;
		return m_iDynLen & 0xffffffUL;
	}

	DWORD operator[] ( int iIndex )const
	{
		if ( !IsDynamic() )
			return m_dData[iIndex] & 0xffffffUL;
		return m_pDynData[iIndex];
	}
};


void Swap ( InfixIntvec_t & a, InfixIntvec_t & b )
{
	::Swap ( a.m_dData[0], b.m_dData[0] );
	::Swap ( a.m_dData[1], b.m_dData[1] );
	::Swap ( a.m_dData[2], b.m_dData[2] );
	::Swap ( a.m_dData[3], b.m_dData[3] );
}


template < int SIZE >
struct InfixHashEntry_t
{
	Infix_t<SIZE>	m_tKey;		///< key, owned by the hash
	InfixIntvec_t	m_tValue;	///< data, owned by the hash
	int				m_iNext;	///< next entry in hash arena
};


template < int SIZE >
class InfixBuilder_c : public ISphInfixBuilder
{
protected:
	static const int							LENGTH = 1048576;

protected:
	int											m_dHash [ LENGTH ];		///< all the hash entries
	CSphSwapVector < InfixHashEntry_t<SIZE> >	m_dArena;
	CSphVector<InfixBlock_t>					m_dBlocks;
	CSphTightVector<BYTE>						m_dBlocksWords;

public:
					InfixBuilder_c();
	virtual void	AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology );
	virtual void	SaveEntries ( CSphWriter & wrDict );
	virtual int64_t	SaveEntryBlocks ( CSphWriter & wrDict );
	virtual int		GetBlocksWordsSize () const { return m_dBlocksWords.GetLength(); }

protected:
	/// add new entry
	void AddEntry ( const Infix_t<SIZE> & tKey, DWORD uHash, int iCheckpoint )
	{
		uHash &= ( LENGTH-1 );

		int iEntry = m_dArena.GetLength();
		InfixHashEntry_t<SIZE> & tNew = m_dArena.Add();
		tNew.m_tKey = tKey;
		tNew.m_tValue.m_dData[0] = 0x1000000UL | iCheckpoint; // len=1, data=iCheckpoint
		tNew.m_iNext = m_dHash[uHash];
		m_dHash[uHash] = iEntry;
	}

	/// get value pointer by key
	InfixIntvec_t * LookupEntry ( const Infix_t<SIZE> & tKey, DWORD uHash )
	{
		uHash &= ( LENGTH-1 );
		int iEntry = m_dHash [ uHash ];
		int iiEntry = 0;

		while ( iEntry )
		{
			if ( m_dArena[iEntry].m_tKey==tKey )
			{
				// mtf it, if needed
				if ( iiEntry )
				{
					m_dArena[iiEntry].m_iNext = m_dArena[iEntry].m_iNext;
					m_dArena[iEntry].m_iNext = m_dHash[uHash];
					m_dHash[uHash] = iEntry;
				}
				return &m_dArena[iEntry].m_tValue;
			}
			iiEntry = iEntry;
			iEntry = m_dArena[iEntry].m_iNext;
		}
		return NULL;
	}
};


template < int SIZE >
InfixBuilder_c<SIZE>::InfixBuilder_c()
{
	// init the hash
	for ( int i=0; i<LENGTH; i++ )
		m_dHash[i] = 0;
	m_dArena.Reserve ( 1048576 );
	m_dArena.Resize ( 1 ); // 0 is a reserved index
}


/// single-byte case, 2-dword infixes
template<>
void InfixBuilder_c<2>::AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord!=MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord<0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		pWord++;
		iWordLength--;
	}

	Infix_t<2> sKey;
	for ( int p=0; p<=iWordLength-2; p++ )
	{
		sKey.Reset();

		BYTE * pKey = (BYTE*)sKey.m_Data;
		const BYTE * s = pWord + p;
		const BYTE * sMax = s + Min ( 6, iWordLength-p );

		DWORD uHash = 0xffffffUL ^ g_dSphinxCRC32 [ 0xff ^ *s ];
		*pKey++ = *s++; // copy first infix byte

		while ( s<sMax )
		{
			uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
			*pKey++ = *s++; // copy another infix byte

			InfixIntvec_t * pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}
	}
}


/// UTF-8 case, 3/5-dword infixes
template < int SIZE >
void InfixBuilder_c<SIZE>::AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord!=MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord<0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		pWord++;
		iWordLength--;
	}

	int iCodes = 0; // codepoints in current word
	BYTE dBytes[SPH_MAX_WORD_LEN+1]; // byte offset for each codepoints

	// build an offsets table into the bytestring
	dBytes[0] = 0;
	for ( const BYTE * p = pWord; p<pWord+iWordLength && iCodes<SPH_MAX_WORD_LEN; )
	{
		int iLen = 0;
		BYTE uVal = *p;
		while ( uVal & 0x80 )
		{
			uVal <<= 1;
			iLen++;
		}
		if ( !iLen )
			iLen = 1;

		// skip word with large codepoints
		if ( iLen>SIZE )
			return;

		assert ( iLen>=1 && iLen<=4 );
		p += iLen;

		dBytes[iCodes+1] = dBytes[iCodes] + (BYTE)iLen;
		iCodes++;
	}
	assert ( pWord[dBytes[iCodes]]==0 || iCodes==SPH_MAX_WORD_LEN );

	// generate infixes
	Infix_t<SIZE> sKey;
	for ( int p=0; p<=iCodes-2; p++ )
	{
		sKey.Reset();
		auto pKey = (BYTE*)sKey.m_Data;

		const BYTE * s = pWord + dBytes[p];
		const BYTE * sMax = pWord + dBytes[ p+Min ( 6, iCodes-p ) ];

		// copy first infix codepoint
		DWORD uHash = 0xffffffffUL;
		do
		{
			uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
			*pKey++ = *s++;
		} while ( ( *s & 0xC0 )==0x80 );

		while ( s<sMax )
		{
			// copy next infix codepoint
			do
			{
				uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
				*pKey++ = *s++;
			} while ( ( *s & 0xC0 )==0x80 );

			InfixIntvec_t * pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}
	}
}


template < int SIZE >
struct InfixHashCmp_fn
{
	InfixHashEntry_t<SIZE> * m_pBase;

	explicit InfixHashCmp_fn ( InfixHashEntry_t<SIZE> * pBase )
		: m_pBase ( pBase )
	{}

	bool IsLess ( int a, int b ) const
	{
		return strncmp ( (const char*)m_pBase[a].m_tKey.m_Data, (const char*)m_pBase[b].m_tKey.m_Data, sizeof(DWORD)*SIZE )<0;
	}
};


static inline int ZippedIntSize ( DWORD v )
{
	if ( v < (1UL<<7) )
		return 1;
	if ( v < (1UL<<14) )
		return 2;
	if ( v < (1UL<<21) )
		return 3;
	if ( v < (1UL<<28) )
		return 4;
	return 5;
}


const char * g_sTagInfixEntries = "infix-entries";

template < int SIZE >
void InfixBuilder_c<SIZE>::SaveEntries ( CSphWriter & wrDict )
{
	// intentionally local to this function
	// we mark the block end with an editcode of 0
	const int INFIX_BLOCK_SIZE = 64;

	wrDict.PutBytes ( g_sTagInfixEntries, strlen ( g_sTagInfixEntries ) );

	CSphVector<int> dIndex;
	dIndex.Resize ( m_dArena.GetLength()-1 );
	for ( int i=0; i<m_dArena.GetLength()-1; i++ )
		dIndex[i] = i+1;

	InfixHashCmp_fn<SIZE> fnCmp ( m_dArena.Begin() );
	dIndex.Sort ( fnCmp );

	m_dBlocksWords.Reserve ( m_dArena.GetLength()/INFIX_BLOCK_SIZE*sizeof(DWORD)*SIZE );
	int iBlock = 0;
	int iPrevKey = -1;
	ARRAY_FOREACH ( iIndex, dIndex )
	{
		InfixIntvec_t & dData = m_dArena[dIndex[iIndex]].m_tValue;
		const BYTE * sKey = (const BYTE*) m_dArena[dIndex[iIndex]].m_tKey.m_Data;
		int iChars = ( SIZE==2 )
			? strnlen ( (const char*)sKey, sizeof(DWORD)*SIZE )
			: sphUTF8Len ( (const char*)sKey, sizeof(DWORD)*SIZE );
		assert ( iChars>=2 && iChars<int(1 + sizeof ( Infix_t<SIZE> ) ) );

		// keep track of N-infix blocks
		int iAppendBytes = strnlen ( (const char*)sKey, sizeof(DWORD)*SIZE );
		if ( !iBlock )
		{
			int iOff = m_dBlocksWords.GetLength();
			m_dBlocksWords.Resize ( iOff+iAppendBytes+1 );

			InfixBlock_t & tBlock = m_dBlocks.Add();
			tBlock.m_iInfixOffset = iOff;
			tBlock.m_iOffset = (DWORD)wrDict.GetPos();

			memcpy ( m_dBlocksWords.Begin()+iOff, sKey, iAppendBytes );
			m_dBlocksWords[iOff+iAppendBytes] = '\0';
		}

		// compute max common prefix
		// edit_code = ( num_keep_chars<<4 ) + num_append_chars
		int iEditCode = iChars;
		if ( iPrevKey>=0 )
		{
			const BYTE * sPrev = (const BYTE*) m_dArena[dIndex[iPrevKey]].m_tKey.m_Data;
			const BYTE * sCur = (const BYTE*) sKey;
			const BYTE * sMax = sCur + iAppendBytes;

			int iKeepChars = 0;
			if_const ( SIZE==2 )
			{
				// SBCS path
				while ( sCur<sMax && *sCur && *sCur==*sPrev )
				{
					sCur++;
					sPrev++;
				}
				iKeepChars = (int)( sCur- ( const BYTE* ) sKey );

				assert ( iKeepChars>=0 && iKeepChars<16 );
				assert ( iChars-iKeepChars>=0 );
				assert ( iChars-iKeepChars<16 );

				iEditCode = ( iKeepChars<<4 ) + ( iChars-iKeepChars );
				iAppendBytes = ( iChars-iKeepChars );
				sKey = sCur;

			} else
			{
				// UTF-8 path
				const BYTE * sKeyMax = sCur; // track max matching sPrev prefix in [sKey,sKeyMax)
				while ( sCur<sMax && *sCur && *sCur==*sPrev )
				{
					// current byte matches, move the pointer
					sCur++;
					sPrev++;

					// tricky bit
					// if the next (!) byte is a valid UTF-8 char start (or eof!)
					// then we just matched not just a byte, but a full char
					// so bump the matching prefix boundary and length
					if ( sCur>=sMax || ( *sCur & 0xC0 )!=0x80 )
					{
						sKeyMax = sCur;
						iKeepChars++;
					}
				}

				assert ( iKeepChars>=0 && iKeepChars<16 );
				assert ( iChars-iKeepChars>=0 );
				assert ( iChars-iKeepChars<16 );

				iEditCode = ( iKeepChars<<4 ) + ( iChars-iKeepChars );
				iAppendBytes -= (int)( sKeyMax-sKey );
				sKey = sKeyMax;
			}
		}

		// write edit code, postfix
		wrDict.PutByte ( iEditCode );
		wrDict.PutBytes ( sKey, iAppendBytes );

		// compute data length
		int iDataLen = ZippedIntSize ( dData[0] );
		for ( int j=1; j<dData.GetLength(); j++ )
			iDataLen += ZippedIntSize ( dData[j] - dData[j-1] );

		// write data length, data
		wrDict.ZipInt ( iDataLen );
		wrDict.ZipInt ( dData[0] );
		for ( int j=1; j<dData.GetLength(); j++ )
			wrDict.ZipInt ( dData[j] - dData[j-1] );

		// mark block end, restart deltas
		iPrevKey = iIndex;
		if ( ++iBlock==INFIX_BLOCK_SIZE )
		{
			iBlock = 0;
			iPrevKey = -1;
			wrDict.PutByte ( 0 );
		}
	}

	// put end marker
	if ( iBlock )
		wrDict.PutByte ( 0 );

	const char * pBlockWords = (const char *)m_dBlocksWords.Begin();
	ARRAY_FOREACH ( i, m_dBlocks )
		m_dBlocks[i].m_sInfix = pBlockWords+m_dBlocks[i].m_iInfixOffset;

	if ( wrDict.GetPos()>UINT_MAX ) // FIXME!!! change to int64
		sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", wrDict.GetPos() );
}


const char * g_sTagInfixBlocks = "infix-blocks";

template < int SIZE >
int64_t InfixBuilder_c<SIZE>::SaveEntryBlocks ( CSphWriter & wrDict )
{
	// save the blocks
	wrDict.PutBytes ( g_sTagInfixBlocks, strlen ( g_sTagInfixBlocks ) );

	SphOffset_t iInfixBlocksOffset = wrDict.GetPos();
	assert ( iInfixBlocksOffset<=INT_MAX );

	wrDict.ZipInt ( m_dBlocks.GetLength() );
	ARRAY_FOREACH ( i, m_dBlocks )
	{
		int iBytes = strlen ( m_dBlocks[i].m_sInfix );
		wrDict.PutByte ( iBytes );
		wrDict.PutBytes ( m_dBlocks[i].m_sInfix, iBytes );
		wrDict.ZipInt ( m_dBlocks[i].m_iOffset ); // maybe delta these on top?
	}

	return iInfixBlocksOffset;
}


ISphInfixBuilder * sphCreateInfixBuilder ( int iCodepointBytes, CSphString * pError )
{
	assert ( pError );
	*pError = CSphString();
	switch ( iCodepointBytes )
	{
	case 0:		return NULL;
	case 1:		return new InfixBuilder_c<2>(); // upto 6x1 bytes, 2 dwords, sbcs
	case 2:		return new InfixBuilder_c<3>(); // upto 6x2 bytes, 3 dwords, utf-8
	case 3:		return new InfixBuilder_c<5>(); // upto 6x3 bytes, 5 dwords, utf-8
	default:	pError->SetSprintf ( "unhandled max infix codepoint size %d", iCodepointBytes ); return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY
//////////////////////////////////////////////////////////////////////////

class CSphDictKeywords : public CSphDictCRC<true>
{
public:
	// OPTIMIZE? change pointers to 8:24 locators to save RAM on x64 gear?
	struct HitblockKeyword_t
	{
		SphWordID_t					m_uWordid;			// locally unique word id (crc value, adjusted in case of collsion)
		HitblockKeyword_t *			m_pNextHash;		// next hashed entry
		char *						m_pKeyword;			// keyword
	};

	struct HitblockException_t
	{
		HitblockKeyword_t *			m_pEntry;			// hash entry
		SphWordID_t					m_uCRC;				// original unadjusted crc

		bool operator < ( const HitblockException_t & rhs ) const
		{
			return m_pEntry->m_uWordid < rhs.m_pEntry->m_uWordid;
		}
	};

	struct DictKeyword_t
	{
		char *						m_sKeyword;
		SphOffset_t					m_uOff;
		int							m_iDocs;
		int							m_iHits;
		BYTE						m_uHint;
		int							m_iSkiplistPos;		///< position in .spe file; not exactly likely to hit 2B
	};

	struct DictBlock_t
	{
		SphOffset_t					m_iPos;
		int							m_iLen;
	};

public:
	explicit		CSphDictKeywords ();

	void			HitblockBegin () final { m_bHitblock = true; }
	void			HitblockPatch ( CSphWordHit * pHits, int iHits ) const final;
	const char *	HitblockGetKeyword ( SphWordID_t uWordID ) final;
	int				HitblockGetMemUse () final { return m_iMemUse; }
	void			HitblockReset () final;

	void			DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit ) final;
	void			DictEntry ( const CSphDictEntry & tEntry ) final;
	void			DictEndEntries ( SphOffset_t ) final {}
	bool			DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError ) final;

	SphWordID_t		GetWordID ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) final;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final;
	CSphDict *		Clone () const final { return CloneBase ( new CSphDictKeywords() ); }

protected:
					~CSphDictKeywords () final;

private:
	struct DictKeywordTagged_t : public DictKeyword_t
	{
		int m_iBlock;

		static inline bool IsLess ( const DictKeywordTagged_t & a, const DictKeywordTagged_t & b );
	};

	static const int				SLOTS			= 65536;
	static const int				ENTRY_CHUNK		= 65536;
	static const int				KEYWORD_CHUNK	= 1048576;
	static const int				DICT_CHUNK		= 65536;

	HitblockKeyword_t *				m_dHash [ SLOTS ];	///< hash by wordid (!)
	CSphVector<HitblockException_t>	m_dExceptions;

	bool							m_bHitblock;		///< should we store words on GetWordID or not
	int								m_iMemUse;			///< current memory use by all the chunks
	int								m_iDictLimit;		///< allowed memory limit for dict block collection

	CSphVector<HitblockKeyword_t*>	m_dEntryChunks;		///< hash chunks, only used when indexing hitblocks
	HitblockKeyword_t *				m_pEntryChunk;
	int								m_iEntryChunkFree;

	CSphVector<BYTE*>				m_dKeywordChunks;	///< keyword storage
	BYTE *							m_pKeywordChunk;
	int								m_iKeywordChunkFree;

	CSphVector<DictKeyword_t*>		m_dDictChunks;		///< dict entry chunks, only used when sorting final dict
	DictKeyword_t *					m_pDictChunk;
	int								m_iDictChunkFree;

	int								m_iTmpFD;			///< temp dict file descriptor
	CSphWriter						m_wrTmpDict;		///< temp dict writer
	CSphVector<DictBlock_t>			m_dDictBlocks;		///< on-disk locations of dict entry blocks

	char							m_sClippedWord[MAX_KEYWORD_BYTES]; ///< keyword storage for cliiped word

private:
	SphWordID_t				HitblockGetID ( const char * pWord, int iLen, SphWordID_t uCRC );
	HitblockKeyword_t *		HitblockAddKeyword ( DWORD uHash, const char * pWord, int iLen, SphWordID_t uID );

	void					DictReadEntry ( CSphBin * pBin, DictKeywordTagged_t & tEntry, BYTE * pKeyword );
	void					DictFlush ();
};

//////////////////////////////////////////////////////////////////////////

CSphDictKeywords::CSphDictKeywords ()
	: m_bHitblock ( false )
	, m_iMemUse ( 0 )
	, m_iDictLimit ( 0 )
	, m_pEntryChunk ( NULL )
	, m_iEntryChunkFree ( 0 )
	, m_pKeywordChunk ( NULL )
	, m_iKeywordChunkFree ( 0 )
	, m_pDictChunk ( NULL )
	, m_iDictChunkFree ( 0 )
	, m_iTmpFD ( -1 )
{
	memset ( m_dHash, 0, sizeof(m_dHash) );
}

CSphDictKeywords::~CSphDictKeywords ()
{
	HitblockReset();
}

void CSphDictKeywords::HitblockReset()
{
	m_dExceptions.Resize ( 0 );

	ARRAY_FOREACH ( i, m_dEntryChunks )
		SafeDeleteArray ( m_dEntryChunks[i] );
	m_dEntryChunks.Resize ( 0 );
	m_pEntryChunk = NULL;
	m_iEntryChunkFree = 0;

	ARRAY_FOREACH ( i, m_dKeywordChunks )
		SafeDeleteArray ( m_dKeywordChunks[i] );
	m_dKeywordChunks.Resize ( 0 );
	m_pKeywordChunk = NULL;
	m_iKeywordChunkFree = 0;

	m_iMemUse = 0;

	memset ( m_dHash, 0, sizeof(m_dHash) );
}

CSphDictKeywords::HitblockKeyword_t * CSphDictKeywords::HitblockAddKeyword ( DWORD uHash, const char * sWord, int iLen, SphWordID_t uID )
{
	assert ( iLen<MAX_KEYWORD_BYTES );

	// alloc entry
	if ( !m_iEntryChunkFree )
	{
		m_pEntryChunk = new HitblockKeyword_t [ ENTRY_CHUNK ];
		m_iEntryChunkFree = ENTRY_CHUNK;
		m_dEntryChunks.Add ( m_pEntryChunk );
		m_iMemUse += sizeof(HitblockKeyword_t)*ENTRY_CHUNK;
	}
	HitblockKeyword_t * pEntry = m_pEntryChunk++;
	m_iEntryChunkFree--;

	// alloc keyword
	iLen++;
	if ( m_iKeywordChunkFree < iLen )
	{
		m_pKeywordChunk = new BYTE [ KEYWORD_CHUNK ];
		m_iKeywordChunkFree = KEYWORD_CHUNK;
		m_dKeywordChunks.Add ( m_pKeywordChunk );
		m_iMemUse += KEYWORD_CHUNK;
	}

	// fill it
	memcpy ( m_pKeywordChunk, sWord, iLen );
	m_pKeywordChunk[iLen-1] = '\0';
	pEntry->m_pKeyword = (char*)m_pKeywordChunk;
	pEntry->m_uWordid = uID;
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	// mtf it
	pEntry->m_pNextHash = m_dHash [ uHash ];
	m_dHash [ uHash ] = pEntry;

	return pEntry;
}

SphWordID_t CSphDictKeywords::HitblockGetID ( const char * sWord, int iLen, SphWordID_t uCRC )
{
	if ( iLen>MAX_KEYWORD_BYTES-4 ) // fix of very long word (zones)
	{
		memcpy ( m_sClippedWord, sWord, MAX_KEYWORD_BYTES-4 );
		memset ( m_sClippedWord+MAX_KEYWORD_BYTES-4, 0, 4 );

		CSphString sOrig;
		sOrig.SetBinary ( sWord, iLen );
		sphWarn ( "word overrun buffer, clipped!!!\n"
			"clipped (len=%d, word='%s')\noriginal (len=%d, word='%s')",
			MAX_KEYWORD_BYTES-4, m_sClippedWord, iLen, sOrig.cstr() );

		sWord = m_sClippedWord;
		iLen = MAX_KEYWORD_BYTES-4;
		uCRC = sphCRC32 ( m_sClippedWord, MAX_KEYWORD_BYTES-4 );
	}

	// is this a known one? find it
	// OPTIMIZE? in theory we could use something faster than crc32; but quick lookup3 test did not show any improvements
	const DWORD uHash = (DWORD)( uCRC % SLOTS );

	HitblockKeyword_t * pEntry = m_dHash [ uHash ];
	HitblockKeyword_t ** ppEntry = &m_dHash [ uHash ];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid!=uCRC )
		{
			// crc mismatch, try next entry
			ppEntry = &pEntry->m_pNextHash;
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		// crc matches, check keyword
		register int iWordLen = iLen;
		register const char * a = pEntry->m_pKeyword;
		register const char * b = sWord;
		while ( *a==*b && iWordLen-- )
		{
			if ( !*a || !iWordLen )
			{
				// known word, mtf it, and return id
				(*ppEntry) = pEntry->m_pNextHash;
				pEntry->m_pNextHash = m_dHash [ uHash ];
				m_dHash [ uHash ] = pEntry;
				return pEntry->m_uWordid;
			}
			a++;
			b++;
		}

		// collision detected!
		// our crc is taken as a wordid, but keyword does not match
		// welcome to the land of very tricky magic
		//
		// pEntry might either be a known exception, or a regular keyword
		// sWord might either be a known exception, or a new one
		// if they are not known, they needed to be added as exceptions now
		//
		// in case sWord is new, we need to assign a new unique wordid
		// for that, we keep incrementing the crc until it is unique
		// a starting point for wordid search loop would be handy
		//
		// let's scan the exceptions vector and work on all this
		//
		// NOTE, beware of the order, it is wordid asc, which does NOT guarantee crc asc
		// example, assume crc(w1)==X, crc(w2)==X+1, crc(w3)==X (collides with w1)
		// wordids will be X, X+1, X+2 but crcs will be X, X+1, X
		//
		// OPTIMIZE, might make sense to use binary search
		// OPTIMIZE, add early out somehow
		SphWordID_t uWordid = uCRC + 1;
		const int iExcLen = m_dExceptions.GetLength();
		int iExc = m_dExceptions.GetLength();
		ARRAY_FOREACH ( i, m_dExceptions )
		{
			const HitblockKeyword_t * pExcWord = m_dExceptions[i].m_pEntry;

			// incoming word is a known exception? just return the pre-assigned wordid
			if ( m_dExceptions[i].m_uCRC==uCRC && strncmp ( pExcWord->m_pKeyword, sWord, iLen )==0 )
				return pExcWord->m_uWordid;

			// incoming word collided into a known exception? clear the matched entry; no need to re-add it (see below)
			if ( pExcWord==pEntry )
				pEntry = NULL;

			// find first exception with wordid greater or equal to our candidate
			if ( pExcWord->m_uWordid>=uWordid && iExc==iExcLen )
				iExc = i;
		}

		// okay, this is a new collision
		// if entry was a regular word, we have to add it
		if ( pEntry )
		{
			m_dExceptions.Add();
			m_dExceptions.Last().m_pEntry = pEntry;
			m_dExceptions.Last().m_uCRC = uCRC;
		}

		// need to assign a new unique wordid now
		// keep scanning both exceptions and keywords for collisions
		while (true)
		{
			// iExc must be either the first exception greater or equal to current candidate, or out of bounds
			assert ( iExc==iExcLen || m_dExceptions[iExc].m_pEntry->m_uWordid>=uWordid );
			assert ( iExc==0 || m_dExceptions[iExc-1].m_pEntry->m_uWordid<uWordid );

			// candidate collides with a known exception? increment it, and keep looking
			if ( iExc<iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid==uWordid )
			{
				uWordid++;
				while ( iExc<iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid<uWordid )
					iExc++;
				continue;
			}

			// candidate collides with a keyword? must be a regular one; add it as an exception, and keep looking
			HitblockKeyword_t * pCheck = m_dHash [ (DWORD)( uWordid % SLOTS ) ];
			while ( pCheck )
			{
				if ( pCheck->m_uWordid==uWordid )
					break;
				pCheck = pCheck->m_pNextHash;
			}

			// no collisions; we've found our unique wordid!
			if ( !pCheck )
				break;

			// got a collision; add it
			HitblockException_t & tColl = m_dExceptions.Add();
			tColl.m_pEntry = pCheck;
			tColl.m_uCRC = pCheck->m_uWordid; // not a known exception; hence, wordid must equal crc

			// and keep looking
			uWordid++;
			continue;
		}

		// and finally, we have that precious new wordid
		// so hash our new unique under its new unique adjusted wordid
		pEntry = HitblockAddKeyword ( (DWORD)( uWordid % SLOTS ), sWord, iLen, uWordid );

		// add it as a collision too
		m_dExceptions.Add();
		m_dExceptions.Last().m_pEntry = pEntry;
		m_dExceptions.Last().m_uCRC = uCRC;

		// keep exceptions list sorted by wordid
		m_dExceptions.Sort();

		return pEntry->m_uWordid;
	}

	// new keyword with unique crc
	pEntry = HitblockAddKeyword ( uHash, sWord, iLen, uCRC );
	return pEntry->m_uWordid;
}


inline bool CSphDictKeywords::DictKeywordTagged_t::IsLess ( const DictKeywordTagged_t & a, const DictKeywordTagged_t & b )
{
	return strcmp ( a.m_sKeyword, b.m_sKeyword ) < 0;
};


void CSphDictKeywords::DictReadEntry ( CSphBin * pBin, DictKeywordTagged_t & tEntry, BYTE * pKeyword )
{
	int iKeywordLen = pBin->ReadByte ();
	if ( iKeywordLen<0 )
	{
		// early eof or read error; flag must be raised
		assert ( pBin->IsError() );
		return;
	}

	assert ( iKeywordLen>0 && iKeywordLen<MAX_KEYWORD_BYTES-1 );
	if ( pBin->ReadBytes ( pKeyword, iKeywordLen )!=BIN_READ_OK )
	{
		assert ( pBin->IsError() );
		return;
	}
	pKeyword[iKeywordLen] = '\0';

	assert ( m_iSkiplistBlockSize>0 );

	tEntry.m_sKeyword = (char*)pKeyword;
	tEntry.m_uOff = pBin->UnzipOffset();
	tEntry.m_iDocs = pBin->UnzipInt();
	tEntry.m_iHits = pBin->UnzipInt();
	tEntry.m_uHint = (BYTE) pBin->ReadByte();
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		tEntry.m_iSkiplistPos = pBin->UnzipInt();
	else
		tEntry.m_iSkiplistPos = 0;
}


void CSphDictKeywords::DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit )
{
	m_iTmpFD = tTempDict.GetFD();
	m_wrTmpDict.CloseFile ();
	m_wrTmpDict.SetFile ( tTempDict, NULL, m_sWriterError );

	m_wrDict.CloseFile ();
	m_wrDict.SetFile ( tDict, NULL, m_sWriterError );
	m_wrDict.PutByte ( 1 );

	m_iDictLimit = Max ( iDictLimit, KEYWORD_CHUNK + DICT_CHUNK*(int)sizeof(DictKeyword_t) ); // can't use less than 1 chunk
}


bool CSphDictKeywords::DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError )
{
	DictFlush ();
	m_wrTmpDict.CloseFile (); // tricky: file is not owned, so it won't get closed, and iTmpFD won't get invalidated

	if ( !m_dDictBlocks.GetLength() )
		m_wrDict.CloseFile();

	if ( m_wrTmpDict.IsError() || m_wrDict.IsError() )
	{
		sError.SetSprintf ( "dictionary write error (out of space?)" );
		return false;
	}

	if ( !m_dDictBlocks.GetLength() )
	{
		pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos ();
		pHeader->m_iDictCheckpoints = 0;
		return true;
	}

	// infix builder, if needed
	ISphInfixBuilder * pInfixer = sphCreateInfixBuilder ( pHeader->m_iInfixCodepointBytes, &sError );
	if ( !sError.IsEmpty() )
	{
		SafeDelete ( pInfixer );
		return false;
	}

	assert ( m_iSkiplistBlockSize>0 );

	// initialize readers
	CSphVector<CSphBin*> dBins ( m_dDictBlocks.GetLength() );

	int iMaxBlock = 0;
	ARRAY_FOREACH ( i, m_dDictBlocks )
		iMaxBlock = Max ( iMaxBlock, m_dDictBlocks[i].m_iLen );

	iMemLimit = Max ( iMemLimit, iMaxBlock*m_dDictBlocks.GetLength() );
	int iBinSize = CSphBin::CalcBinSize ( iMemLimit, m_dDictBlocks.GetLength(), "sort_dict" );

	SphOffset_t iSharedOffset = -1;
	ARRAY_FOREACH ( i, m_dDictBlocks )
	{
		dBins[i] = new CSphBin();
		dBins[i]->m_iFileLeft = m_dDictBlocks[i].m_iLen;
		dBins[i]->m_iFilePos = m_dDictBlocks[i].m_iPos;
		dBins[i]->Init ( m_iTmpFD, &iSharedOffset, iBinSize );
	}

	// keywords storage
	BYTE * pKeywords = new BYTE [ MAX_KEYWORD_BYTES*dBins.GetLength() ];

	#define LOC_CLEANUP() \
		{ \
			ARRAY_FOREACH ( iIdx, dBins ) \
				SafeDelete ( dBins[iIdx] ); \
			SafeDeleteArray ( pKeywords ); \
			SafeDelete ( pInfixer ); \
		}

	// do the sort
	CSphQueue < DictKeywordTagged_t, DictKeywordTagged_t > qWords ( dBins.GetLength() );
	DictKeywordTagged_t tEntry;

	ARRAY_FOREACH ( i, dBins )
	{
		DictReadEntry ( dBins[i], tEntry, pKeywords + i*MAX_KEYWORD_BYTES );
		if ( dBins[i]->IsError() )
		{
			sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", i, dBins.GetLength() );
			LOC_CLEANUP();
			return false;
		}

		tEntry.m_iBlock = i;
		qWords.Push ( tEntry );
	}

	bool bHasMorphology = HasMorphology();
	CSphKeywordDeltaWriter tLastKeyword;
	int iWords = 0;
	while ( qWords.GetLength() )
	{
		const DictKeywordTagged_t & tWord = qWords.Root();
		const int iLen = strlen ( tWord.m_sKeyword ); // OPTIMIZE?

		// store checkpoints as needed
		if ( ( iWords % SPH_WORDLIST_CHECKPOINT )==0 )
		{
			// emit a checkpoint, unless we're at the very dict beginning
			if ( iWords )
			{
				m_wrDict.ZipInt ( 0 );
				m_wrDict.ZipInt ( 0 );
			}

			BYTE * sClone = new BYTE [ iLen+1 ]; // OPTIMIZE? pool these?
			memcpy ( sClone, tWord.m_sKeyword, iLen );
			sClone[iLen] = '\0';

			CSphWordlistCheckpoint & tCheckpoint = m_dCheckpoints.Add ();
			tCheckpoint.m_sWord = (char*) sClone;
			tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();

			tLastKeyword.Reset();
		}
		iWords++;

		// write final dict entry
		assert ( iLen );
		assert ( tWord.m_uOff );
		assert ( tWord.m_iDocs );
		assert ( tWord.m_iHits );

		tLastKeyword.PutDelta ( m_wrDict, (const BYTE *)tWord.m_sKeyword, iLen );
		m_wrDict.ZipOffset ( tWord.m_uOff );
		m_wrDict.ZipInt ( tWord.m_iDocs );
		m_wrDict.ZipInt ( tWord.m_iHits );
		if ( tWord.m_uHint )
			m_wrDict.PutByte ( tWord.m_uHint );
		if ( tWord.m_iDocs > m_iSkiplistBlockSize )
			m_wrDict.ZipInt ( tWord.m_iSkiplistPos );

		// build infixes
		if ( pInfixer )
			pInfixer->AddWord ( (const BYTE*)tWord.m_sKeyword, iLen, m_dCheckpoints.GetLength(), bHasMorphology );

		// next
		int iBin = tWord.m_iBlock;
		qWords.Pop ();

		if ( !dBins[iBin]->IsDone() )
		{
			DictReadEntry ( dBins[iBin], tEntry, pKeywords + iBin*MAX_KEYWORD_BYTES );
			if ( dBins[iBin]->IsError() )
			{
				sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", iBin, dBins.GetLength() );
				LOC_CLEANUP();
				return false;
			}

			tEntry.m_iBlock = iBin;
			qWords.Push ( tEntry );
		}
	}

	// end of dictionary block
	m_wrDict.ZipInt ( 0 );
	m_wrDict.ZipInt ( 0 );

	// flush infix hash entries, if any
	if ( pInfixer )
		pInfixer->SaveEntries ( m_wrDict );

	// flush wordlist checkpoints (blocks)
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength();

	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		const int iLen = strlen ( m_dCheckpoints[i].m_sWord );

		assert ( m_dCheckpoints[i].m_iWordlistOffset>0 );
		assert ( iLen>0 && iLen<MAX_KEYWORD_BYTES );

		m_wrDict.PutDword ( iLen );
		m_wrDict.PutBytes ( m_dCheckpoints[i].m_sWord, iLen );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_iWordlistOffset );

		SafeDeleteArray ( m_dCheckpoints[i].m_sWord );
	}

	// flush infix hash blocks
	if ( pInfixer )
	{
		pHeader->m_iInfixBlocksOffset = pInfixer->SaveEntryBlocks ( m_wrDict );
		pHeader->m_iInfixBlocksWordsSize = pInfixer->GetBlocksWordsSize();
		if ( pHeader->m_iInfixBlocksOffset>UINT_MAX ) // FIXME!!! change to int64
			sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at dictend save", pHeader->m_iInfixBlocksOffset );
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	m_wrDict.PutBytes ( "dict-header", 11 );
	m_wrDict.ZipInt ( pHeader->m_iDictCheckpoints );
	m_wrDict.ZipOffset ( pHeader->m_iDictCheckpointsOffset );
	m_wrDict.ZipInt ( pHeader->m_iInfixCodepointBytes );
	m_wrDict.ZipInt ( (DWORD)pHeader->m_iInfixBlocksOffset );

	// about it
	LOC_CLEANUP();
	#undef LOC_CLEANUP

	m_wrDict.CloseFile ();
	if ( m_wrDict.IsError() )
		sError.SetSprintf ( "dictionary write error (out of space?)" );
	return !m_wrDict.IsError();
}

struct DictKeywordCmp_fn
{
	inline bool IsLess ( CSphDictKeywords::DictKeyword_t * a, CSphDictKeywords::DictKeyword_t * b ) const
	{
		return strcmp ( a->m_sKeyword, b->m_sKeyword ) < 0;
	}
};

void CSphDictKeywords::DictFlush ()
{
	if ( !m_dDictChunks.GetLength() )
		return;

	assert ( m_dDictChunks.GetLength() && m_dKeywordChunks.GetLength() );
	assert ( m_iSkiplistBlockSize>0 );

	// sort em
	int iTotalWords = m_dDictChunks.GetLength()*DICT_CHUNK - m_iDictChunkFree;
	CSphVector<DictKeyword_t*> dWords ( iTotalWords );

	int iIdx = 0;
	ARRAY_FOREACH ( i, m_dDictChunks )
	{
		int iWords = DICT_CHUNK;
		if ( i==m_dDictChunks.GetLength()-1 )
			iWords -= m_iDictChunkFree;

		DictKeyword_t * pWord = m_dDictChunks[i];
		for ( int j=0; j<iWords; j++ )
			dWords[iIdx++] = pWord++;
	}

	dWords.Sort ( DictKeywordCmp_fn() );

	// write em
	DictBlock_t & tBlock = m_dDictBlocks.Add();
	tBlock.m_iPos = m_wrTmpDict.GetPos ();

	ARRAY_FOREACH ( i, dWords )
	{
		const DictKeyword_t * pWord = dWords[i];
		int iLen = strlen ( pWord->m_sKeyword );
		m_wrTmpDict.PutByte ( iLen );
		m_wrTmpDict.PutBytes ( pWord->m_sKeyword, iLen );
		m_wrTmpDict.ZipOffset ( pWord->m_uOff );
		m_wrTmpDict.ZipInt ( pWord->m_iDocs );
		m_wrTmpDict.ZipInt ( pWord->m_iHits );
		m_wrTmpDict.PutByte ( pWord->m_uHint );
		assert ( ( pWord->m_iDocs > m_iSkiplistBlockSize )==( pWord->m_iSkiplistPos!=0 ) );
		if ( pWord->m_iDocs > m_iSkiplistBlockSize )
			m_wrTmpDict.ZipInt ( pWord->m_iSkiplistPos );
	}

	tBlock.m_iLen = (int)( m_wrTmpDict.GetPos() - tBlock.m_iPos );

	// clean up buffers
	ARRAY_FOREACH ( i, m_dDictChunks )
		SafeDeleteArray ( m_dDictChunks[i] );
	m_dDictChunks.Resize ( 0 );
	m_pDictChunk = NULL;
	m_iDictChunkFree = 0;

	ARRAY_FOREACH ( i, m_dKeywordChunks )
		SafeDeleteArray ( m_dKeywordChunks[i] );
	m_dKeywordChunks.Resize ( 0 );
	m_pKeywordChunk = NULL;
	m_iKeywordChunkFree = 0;

	m_iMemUse = 0;
}

void CSphDictKeywords::DictEntry ( const CSphDictEntry & tEntry )
{
	// they say, this might just happen during merge
	// FIXME! can we make merge avoid sending such keywords to dict and assert here?
	if ( !tEntry.m_iDocs )
		return;

	assert ( tEntry.m_iHits );
	assert ( tEntry.m_iDoclistLength>0 );
	assert ( m_iSkiplistBlockSize>0 );

	DictKeyword_t * pWord = NULL;
	int iLen = strlen ( (char*)tEntry.m_sKeyword ) + 1;

	while (true)
	{
		// alloc dict entry
		if ( !m_iDictChunkFree )
		{
			if ( m_iDictLimit && ( m_iMemUse + (int)sizeof(DictKeyword_t)*DICT_CHUNK )>m_iDictLimit )
				DictFlush ();

			m_pDictChunk = new DictKeyword_t [ DICT_CHUNK ];
			m_iDictChunkFree = DICT_CHUNK;
			m_dDictChunks.Add ( m_pDictChunk );
			m_iMemUse += sizeof(DictKeyword_t)*DICT_CHUNK;
		}

		// alloc keyword
		if ( m_iKeywordChunkFree < iLen )
		{
			if ( m_iDictLimit && ( m_iMemUse + KEYWORD_CHUNK )>m_iDictLimit )
			{
				DictFlush ();
				continue; // because we just flushed pWord
			}

			m_pKeywordChunk = new BYTE [ KEYWORD_CHUNK ];
			m_iKeywordChunkFree = KEYWORD_CHUNK;
			m_dKeywordChunks.Add ( m_pKeywordChunk );
			m_iMemUse += KEYWORD_CHUNK;
		}
		// aw kay
		break;
	}

	pWord = m_pDictChunk++;
	m_iDictChunkFree--;
	pWord->m_sKeyword = (char*)m_pKeywordChunk;
	memcpy ( m_pKeywordChunk, tEntry.m_sKeyword, iLen );
	m_pKeywordChunk[iLen-1] = '\0';
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	pWord->m_uOff = tEntry.m_iDoclistOffset;
	pWord->m_iDocs = tEntry.m_iDocs;
	pWord->m_iHits = tEntry.m_iHits;
	pWord->m_uHint = sphDoclistHintPack ( tEntry.m_iDocs, tEntry.m_iDoclistLength );
	pWord->m_iSkiplistPos = 0;
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		pWord->m_iSkiplistPos = (int)( tEntry.m_iSkiplistOffset );
}

SphWordID_t CSphDictKeywords::GetWordID ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordID ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	int iLen = strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDWithMarkers ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordIDWithMarkers ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	int iLen = strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDNonStemmed ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordIDNonStemmed ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	int iLen = strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordID ( pWord, iLen, bFilterStops );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	return HitblockGetID ( (const char *)pWord, iLen, uCRC ); // !COMMIT would break, we kind of strcmp inside; but must never get called?
}

/// binary search for the first hit with wordid greater than or equal to reference
static CSphWordHit * FindFirstGte ( CSphWordHit * pHits, int iHits, SphWordID_t uID )
{
	if ( pHits->m_uWordID==uID )
		return pHits;

	CSphWordHit * pL = pHits;
	CSphWordHit * pR = pHits + iHits - 1;
	if ( pL->m_uWordID > uID || pR->m_uWordID < uID )
		return NULL;

	while ( pR-pL!=1 )
	{
		CSphWordHit * pM = pL + ( pR-pL )/2;
		if ( pM->m_uWordID < uID )
			pL = pM;
		else
			pR = pM;
	}

	assert ( pR-pL==1 );
	assert ( pL->m_uWordID<uID );
	assert ( pR->m_uWordID>=uID );
	return pR;
}

/// full crc and keyword check
static inline bool FullIsLess ( const CSphDictKeywords::HitblockException_t & a, const CSphDictKeywords::HitblockException_t & b )
{
	if ( a.m_uCRC!=b.m_uCRC )
		return a.m_uCRC < b.m_uCRC;
	return strcmp ( a.m_pEntry->m_pKeyword, b.m_pEntry->m_pKeyword ) < 0;
}

/// sort functor to compute collided hits reordering
struct HitblockPatchSort_fn
{
	const CSphDictKeywords::HitblockException_t * m_pExc;

	explicit HitblockPatchSort_fn ( const CSphDictKeywords::HitblockException_t * pExc )
		: m_pExc ( pExc )
	{}

	bool IsLess ( int a, int b ) const
	{
		return FullIsLess ( m_pExc[a], m_pExc[b] );
	}
};

/// do hit block patching magic
void CSphDictKeywords::HitblockPatch ( CSphWordHit * pHits, int iHits ) const
{
	if ( !pHits || iHits<=0 )
		return;

	const CSphVector<HitblockException_t> & dExc = m_dExceptions; // shortcut
	CSphVector<CSphWordHit*> dChunk;

	// reorder hit chunks for exceptions (aka crc collisions)
	for ( int iFirst = 0; iFirst < dExc.GetLength()-1; )
	{
		// find next span of collisions, iFirst inclusive, iMax exclusive ie. [iFirst,iMax)
		// (note that exceptions array is always sorted)
		SphWordID_t uFirstWordid = dExc[iFirst].m_pEntry->m_uWordid;
		assert ( dExc[iFirst].m_uCRC==uFirstWordid );

		int iMax = iFirst+1;
		SphWordID_t uSpan = uFirstWordid+1;
		while ( iMax < dExc.GetLength() && dExc[iMax].m_pEntry->m_uWordid==uSpan )
		{
			iMax++;
			uSpan++;
		}

		// check whether they are in proper order already
		bool bSorted = true;
		for ( int i=iFirst; i<iMax-1 && bSorted; i++ )
			if ( FullIsLess ( dExc[i+1], dExc[i] ) )
				bSorted = false;

		// order is ok; skip this span
		if ( bSorted )
		{
			iFirst = iMax;
			continue;
		}

		// we need to fix up these collision hits
		// convert them from arbitrary "wordid asc" to strict "crc asc, keyword asc" order
		// lets begin with looking up hit chunks for every wordid
		dChunk.Resize ( iMax-iFirst+1 );

		// find the end
		dChunk.Last() = FindFirstGte ( pHits, iHits, uFirstWordid+iMax-iFirst );
		if ( !dChunk.Last() )
		{
			assert ( iMax==dExc.GetLength() && pHits[iHits-1].m_uWordID==uFirstWordid+iMax-1-iFirst );
			dChunk.Last() = pHits+iHits;
		}

		// find the start
		dChunk[0] = FindFirstGte ( pHits, dChunk.Last()-pHits, uFirstWordid );
		assert ( dChunk[0] && dChunk[0]->m_uWordID==uFirstWordid );

		// find the chunk starts
		for ( int i=1; i<dChunk.GetLength()-1; i++ )
		{
			dChunk[i] = FindFirstGte ( dChunk[i-1], dChunk.Last()-dChunk[i-1], uFirstWordid+i );
			assert ( dChunk[i] && dChunk[i]->m_uWordID==uFirstWordid+i );
		}

		CSphWordHit * pTemp;
		if ( iMax-iFirst==2 )
		{
			// most frequent case, just two collisions
			// OPTIMIZE? allocate buffer for the smaller chunk, not just first chunk
			pTemp = new CSphWordHit [ dChunk[1]-dChunk[0] ];
			memcpy ( pTemp, dChunk[0], ( dChunk[1]-dChunk[0] )*sizeof(CSphWordHit) );
			memmove ( dChunk[0], dChunk[1], ( dChunk[2]-dChunk[1] )*sizeof(CSphWordHit) );
			memcpy ( dChunk[0] + ( dChunk[2]-dChunk[1] ), pTemp, ( dChunk[1]-dChunk[0] )*sizeof(CSphWordHit) );
		} else
		{
			// generic case, more than two
			CSphVector<int> dReorder ( iMax-iFirst );
			ARRAY_FOREACH ( i, dReorder )
				dReorder[i] = i;

			HitblockPatchSort_fn fnSort ( &dExc[iFirst] );
			dReorder.Sort ( fnSort );

			// OPTIMIZE? could skip heading and trailing blocks that are already in position
			pTemp = new CSphWordHit [ dChunk.Last()-dChunk[0] ];
			CSphWordHit * pOut = pTemp;

			ARRAY_FOREACH ( i, dReorder )
			{
				int iChunk = dReorder[i];
				int iChunkHits = dChunk[iChunk+1] - dChunk[iChunk];
				memcpy ( pOut, dChunk[iChunk], iChunkHits*sizeof(CSphWordHit) );
				pOut += iChunkHits;
			}

			assert ( ( pOut-pTemp )==( dChunk.Last()-dChunk[0] ) );
			memcpy ( dChunk[0], pTemp, ( dChunk.Last()-dChunk[0] )*sizeof(CSphWordHit) );
		}

		// patching done
		SafeDeleteArray ( pTemp );
		iFirst = iMax;
	}
}

const char * CSphDictKeywords::HitblockGetKeyword ( SphWordID_t uWordID )
{
	const DWORD uHash = (DWORD)( uWordID % SLOTS );

	HitblockKeyword_t * pEntry = m_dHash [ uHash ];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid!=uWordID )
		{
			// crc mismatch, try next entry
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		return pEntry->m_pKeyword;
	}

	assert ( m_dExceptions.GetLength() );
	ARRAY_FOREACH ( i, m_dExceptions )
		if ( m_dExceptions[i].m_pEntry->m_uWordid==uWordID )
			return m_dExceptions[i].m_pEntry->m_pKeyword;

	sphWarning ( "hash missing value in operator [] (wordid=" INT64_FMT ", hash=%d)", (int64_t)uWordID, uHash );
	assert ( 0 && "hash missing value in operator []" );
	return "\31oops";
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY
//////////////////////////////////////////////////////////////////////////

class CRtDictKeywords : public ISphRtDictWraper
{
private:
	CSphDictRefPtr_c		m_pBase;
	SmallStringHash_T<int, 8192>	m_hKeywords;

	CSphVector<BYTE>		m_dPackedKeywords {0};

	CSphString				m_sWarning;
	int						m_iKeywordsOverrun = 0;
	CSphString				m_sWord; // For allocation reuse.

protected:
	virtual ~CRtDictKeywords () final {} // fixme! remove

public:
	explicit CRtDictKeywords ( CSphDict * pBase )
		: m_pBase ( pBase )
	{
		SafeAddRef ( pBase );
		m_dPackedKeywords.Add ( 0 ); // avoid zero offset at all costs
	}

	SphWordID_t GetWordID ( BYTE * pWord ) final
	{
		return m_pBase->GetWordID ( pWord ) ? AddKeyword ( pWord ) : 0;
	}

	SphWordID_t GetWordIDWithMarkers ( BYTE * pWord ) final
	{
		return m_pBase->GetWordIDWithMarkers ( pWord ) ? AddKeyword ( pWord ) : 0;
	}

	SphWordID_t GetWordIDNonStemmed ( BYTE * pWord ) final
	{
		return m_pBase->GetWordIDNonStemmed ( pWord ) ? AddKeyword ( pWord ) : 0;
	}

	SphWordID_t GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final
	{
		return m_pBase->GetWordID ( pWord, iLen, bFilterStops ) ? AddKeyword ( pWord ) : 0;
	}

	const BYTE * GetPackedKeywords () final { return m_dPackedKeywords.Begin(); }
	int GetPackedLen () final { return m_dPackedKeywords.GetLength(); }
	void ResetKeywords() final
	{
		m_dPackedKeywords.Resize ( 0 );
		m_dPackedKeywords.Add ( 0 ); // avoid zero offset at all costs
		m_hKeywords.Reset();
	}

	void LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final { m_pBase->LoadStopwords ( sFiles, pTokenizer, bStripFile ); }
	void LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final { m_pBase->LoadStopwords ( dStopwords ); }
	void WriteStopwords ( CSphWriter & tWriter ) const final { m_pBase->WriteStopwords ( tWriter ); }
	bool LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) final
		{ return m_pBase->LoadWordforms ( dFiles, pEmbedded, pTokenizer, sIndex ); }
	void WriteWordforms ( CSphWriter & tWriter ) const final { m_pBase->WriteWordforms ( tWriter ); }
	int SetMorphology ( const char * szMorph, CSphString & sMessage ) final { return m_pBase->SetMorphology ( szMorph, sMessage ); }
	void Setup ( const CSphDictSettings & tSettings ) final { m_pBase->Setup ( tSettings ); }
	const CSphDictSettings & GetSettings () const final { return m_pBase->GetSettings(); }
	const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () const final { return m_pBase->GetStopwordsFileInfos(); }
	const CSphVector <CSphSavedFile> & GetWordformsFileInfos () const final { return m_pBase->GetWordformsFileInfos(); }
	const CSphMultiformContainer * GetMultiWordforms () const final { return m_pBase->GetMultiWordforms(); }
	bool IsStopWord ( const BYTE * pWord ) const final { return m_pBase->IsStopWord ( pWord ); }
	const char * GetLastWarning() const final { return m_iKeywordsOverrun ? m_sWarning.cstr() : NULL; }
	void ResetWarning () final { m_iKeywordsOverrun = 0; }
	uint64_t GetSettingsFNV () const final { return m_pBase->GetSettingsFNV(); }

private:
	SphWordID_t AddKeyword ( const BYTE * pWord )
	{
		int iLen = strlen ( ( const char * ) pWord );
		// stemmer might squeeze out the word
		if ( !iLen )
			return 0;

		// fix of very long word (zones)
		if ( iLen>( SPH_MAX_WORD_LEN * 3 ) )
		{
			int iClippedLen = SPH_MAX_WORD_LEN * 3;
			m_sWord.SetBinary ( ( const char * ) pWord, iClippedLen );
			if ( m_iKeywordsOverrun )
			{
				m_sWarning.SetSprintf ( "word overrun buffer, clipped!!! clipped='%s', length=%d(%d)", m_sWord.cstr ()
										, iClippedLen, iLen );
			} else
			{
				m_sWarning.SetSprintf ( ", clipped='%s', length=%d(%d)", m_sWord.cstr (), iClippedLen, iLen );
			}
			iLen = iClippedLen;
			m_iKeywordsOverrun++;
		} else
		{
			m_sWord.SetBinary ( ( const char * ) pWord, iLen );
		}

		int * pOff = m_hKeywords ( m_sWord );
		if ( pOff )
		{
			return *pOff;
		}

		int iOff = m_dPackedKeywords.GetLength ();
		m_dPackedKeywords.Resize ( iOff + iLen + 1 );
		m_dPackedKeywords[iOff] = ( BYTE ) ( iLen & 0xFF );
		memcpy ( m_dPackedKeywords.Begin () + iOff + 1, pWord, iLen );

		m_hKeywords.Add ( iOff, m_sWord );

		return iOff;
	}
};

ISphRtDictWraper * sphCreateRtKeywordsDictionaryWrapper ( CSphDict * pBase )
{
	return new CRtDictKeywords ( pBase );
}


//////////////////////////////////////////////////////////////////////////
// DICTIONARY FACTORIES
//////////////////////////////////////////////////////////////////////////

static void SetupDictionary ( CSphDictRefPtr_c& pDict, const CSphDictSettings & tSettings,
	const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
	CSphString & sError )
{
	assert ( pTokenizer );

	pDict->Setup ( tSettings );
	if ( CSphDict::ST_ERROR == pDict->SetMorphology ( tSettings.m_sMorphology.cstr (), sError ) )
	{
		pDict = nullptr;
		return;
	}

	if ( pFiles && pFiles->m_bEmbeddedStopwords )
		pDict->LoadStopwords ( pFiles->m_dStopwords );
	else
		pDict->LoadStopwords ( tSettings.m_sStopwords.cstr(), pTokenizer, bStripFile );

	pDict->LoadWordforms ( tSettings.m_dWordforms, pFiles && pFiles->m_bEmbeddedWordforms ? pFiles : NULL, pTokenizer, sIndex );
}

CSphDict * sphCreateDictionaryTemplate ( const CSphDictSettings & tSettings,
									const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
									CSphString & sError )
{
	CSphDictRefPtr_c pDict { new CSphDictTemplate() };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, sError );
	return pDict.Leak();
}


CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex,
	bool bStripFile, int iSkiplistBlockSize, CSphString & sError )
{
	CSphDictRefPtr_c pDict { new CSphDictCRC<false> };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, sError );
	// might be empty due to wrong morphology setup
	if ( pDict.Ptr() )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict.Leak ();
}


CSphDict * sphCreateDictionaryKeywords ( const CSphDictSettings & tSettings,
	const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
	int iSkiplistBlockSize, CSphString & sError )
{
	CSphDictRefPtr_c pDict { new CSphDictKeywords() };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, sError );
	// might be empty due to wrong morphology setup
	if ( pDict.Ptr() )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict.Leak ();
}


void sphShutdownWordforms ()
{
	CSphVector<CSphSavedFile> dEmptyFiles;
	CSphDiskDictTraits::SweepWordformContainers ( dEmptyFiles );
}

CSphDict * GetStatelessDict ( CSphDict * pDict )
{
	if ( !pDict )
		return nullptr;

	if ( pDict->HasState () )
		return pDict->Clone();

	SafeAddRef ( pDict );
	return pDict;
}

/////////////////////////////////////////////////////////////////////////////
// HTML STRIPPER
/////////////////////////////////////////////////////////////////////////////

static inline int sphIsTag ( int c )
{
	return sphIsAlpha(c) || c=='.' || c==':';
}

static inline int sphIsTagStart ( int c )
{
	return ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_' || c=='.' || c==':';
}

CSphHTMLStripper::CSphHTMLStripper ( bool bDefaultTags )
{
	if ( bDefaultTags )
	{
		// known inline tags
		const char * dKnown[] =
		{
			"a", "b", "i", "s", "u",
			"basefont", "big", "em", "font", "img",
			"label", "small", "span", "strike", "strong",
			"sub\0", "sup\0", // fix gcc 3.4.3 on solaris10 compiler bug
			"tt"
		};

		m_dTags.Resize ( sizeof(dKnown)/sizeof(dKnown[0]) );
		ARRAY_FOREACH ( i, m_dTags )
		{
			m_dTags[i].m_sTag = dKnown[i];
			m_dTags[i].m_iTagLen = strlen ( dKnown[i] );
			m_dTags[i].m_bInline = true;
		}
	}

	UpdateTags ();
}


int CSphHTMLStripper::GetCharIndex ( int iCh ) const
{
	if ( iCh>='a' && iCh<='z' ) return iCh-'a';
	if ( iCh>='A' && iCh<='Z' ) return iCh-'A';
	if ( iCh=='_' ) return 26;
	if ( iCh==':' ) return 27;
	return -1;
}


void CSphHTMLStripper::UpdateTags ()
{
	m_dTags.Sort ();

	for ( int i=0; i<MAX_CHAR_INDEX; i++ )
	{
		m_dStart[i] = INT_MAX;
		m_dEnd[i] = -1;
	}

	ARRAY_FOREACH ( i, m_dTags )
	{
		int iIdx = GetCharIndex ( m_dTags[i].m_sTag.cstr()[0] );
		if ( iIdx<0 )
			continue;

		m_dStart[iIdx] = Min ( m_dStart[iIdx], i );
		m_dEnd[iIdx] = Max ( m_dEnd[iIdx], i );
	}
}


bool CSphHTMLStripper::SetIndexedAttrs ( const char * sConfig, CSphString & sError )
{
	if ( !sConfig || !*sConfig )
		return true;

	char sTag[256], sAttr[256];

	const char * p = sConfig, * s;
	#define LOC_ERROR(_msg,_pos) { sError.SetSprintf ( "SetIndexedAttrs(): %s near '%s'", _msg, _pos ); return false; }

	while ( *p )
	{
		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( !*p ) break;

		// check tag name
		s = p; while ( sphIsTag(*p) ) p++;
		if ( s==p ) LOC_ERROR ( "invalid character in tag name", s );

		// get tag name
		if ( p-s>=(int)sizeof(sTag) ) LOC_ERROR ( "tag name too long", s );
		strncpy ( sTag, s, p-s );
		sTag[p-s] = '\0';

		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( *p++!='=' ) LOC_ERROR ( "'=' expected", p-1 );

		// add indexed tag entry, if not there yet
		strlwr ( sTag );

		int iIndexTag = -1;
		ARRAY_FOREACH ( i, m_dTags )
			if ( m_dTags[i].m_sTag==sTag )
		{
			iIndexTag = i;
			break;
		}
		if ( iIndexTag<0 )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = strlen ( sTag );
			iIndexTag = m_dTags.GetLength()-1;
		}

		m_dTags[iIndexTag].m_bIndexAttrs = true;
		StrVec_t & dAttrs = m_dTags[iIndexTag].m_dAttrs;

		// scan attributes
		while ( *p )
		{
			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check attr name
			s = p; while ( sphIsTag(*p) ) p++;
			if ( s==p ) LOC_ERROR ( "invalid character in attribute name", s );

			// get attr name
			if ( p-s>=(int)sizeof(sAttr) ) LOC_ERROR ( "attribute name too long", s );
			strncpy ( sAttr, s, p-s );
			sAttr[p-s] = '\0';

			// add attr, if not there yet
			int iAttr;
			for ( iAttr=0; iAttr<dAttrs.GetLength(); iAttr++ )
				if ( dAttrs[iAttr]==sAttr )
					break;

			if ( iAttr==dAttrs.GetLength() )
				dAttrs.Add ( sAttr );

			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check if there's next attr or tag
			if ( *p==',' ) { p++; continue; } // next attr
			if ( *p==';' ) { p++; break; } // next tag
			LOC_ERROR ( "',' or ';' or end of line expected", p );
		}
	}

	#undef LOC_ERROR

	UpdateTags ();
	return true;
}


bool CSphHTMLStripper::SetRemovedElements ( const char * sConfig, CSphString & )
{
	if ( !sConfig || !*sConfig )
		return true;

	const char * p = sConfig;
	while ( *p )
	{
		// skip separators
		while ( *p && !sphIsTag(*p) ) p++;
		if ( !*p ) break;

		// get tag name
		const char * s = p;
		while ( sphIsTag(*p) ) p++;

		CSphString sTag;
		sTag.SetBinary ( s, p-s );
		sTag.ToLower ();

		// mark it
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bRemove = true;
			break;
		}

		if ( iTag==m_dTags.GetLength() )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = strlen ( sTag.cstr() );
			m_dTags.Last().m_bRemove = true;
		}
	}

	UpdateTags ();
	return true;
}


void CSphHTMLStripper::EnableParagraphs ()
{
	// known block-level elements
	const char * dBlock[] = { "address", "blockquote", "caption", "center",
		"dd", "div", "dl", "dt", "h1", "h2", "h3", "h4", "h5", "li", "menu",
		"ol", "p", "pre", "table", "tbody", "td", "tfoot", "th", "thead",
		"tr", "ul", NULL };

	for ( int iBlock=0; dBlock[iBlock]; iBlock++ )
	{
		const char * sTag = dBlock[iBlock];

		// mark if known already
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bPara = true;
			break;
		}

		// add if not known yet
		if ( iTag==m_dTags.GetLength() )
		{
			StripperTag_t& dTag = m_dTags.Add();
			dTag.m_sTag = sTag;
			dTag.m_iTagLen = strlen(sTag);
			dTag.m_bPara = true;
		}
	}

	UpdateTags ();
}


bool CSphHTMLStripper::SetZones ( const char * sZones, CSphString & sError )
{
	// yet another mini parser!
	// index_zones = {tagname | prefix*} [, ...]
	if ( !sZones || !*sZones )
		return true;

	const char * s = sZones;
	while ( *s )
	{
		// skip spaces
		while ( sphIsSpace(*s) )
			s++;
		if ( !*s )
			break;

		// expect ident
		if ( !sphIsTagStart(*s) )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}

		// get ident (either tagname or prefix*)
		const char * sTag = s;
		while ( sphIsTag(*s) )
			s++;

		const char * sTagEnd = s;
		bool bPrefix = false;
		if ( *s=='*' )
		{
			s++;
			bPrefix = true;
		}

		// skip spaces
		while ( sphIsSpace(*s) )
			s++;

		// expect eof or comma after ident
		if ( *s && *s!=',' )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}
		if ( *s==',' )
			s++;

		// got valid entry, handle it
		CSphHTMLStripper::StripperTag_t & tTag = m_dTags.Add();
		tTag.m_sTag.SetBinary ( sTag, sTagEnd-sTag );
		tTag.m_iTagLen = (int)( sTagEnd-sTag );
		tTag.m_bZone = true;
		tTag.m_bZonePrefix = bPrefix;
	}

	UpdateTags ();
	return true;
}


const BYTE * SkipQuoted ( const BYTE * p )
{
	const BYTE * pMax = p + 512; // 512 bytes should be enough for a reasonable HTML attribute value, right?!
	const BYTE * pProbEnd = NULL; // (most) probable end location in case we don't find a matching quote
	BYTE cEnd = *p++; // either apostrophe or quote

	while ( p<pMax && *p && *p!=cEnd )
	{
		if ( !pProbEnd )
			if ( *p=='>' || *p=='\r' )
				pProbEnd = p;
		p++;
	}

	if ( *p==cEnd )
		return p+1;

	if ( pProbEnd )
		return pProbEnd;

	return p;
}


struct HtmlEntity_t
{
	const char *	m_sName;
	int				m_iCode;
};


static inline DWORD HtmlEntityHash ( const BYTE * str, int len )
{
	static const unsigned short asso_values[] =
	{
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 4,
		6, 22, 1, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 170, 48, 0, 5, 44,
		0, 10, 10, 86, 421, 7, 0, 1, 42, 93,
		41, 421, 0, 5, 8, 14, 421, 421, 5, 11,
		8, 421, 421, 421, 421, 421, 421, 1, 25, 27,
		9, 2, 113, 82, 14, 3, 179, 1, 81, 91,
		12, 0, 1, 180, 56, 17, 5, 31, 60, 7,
		3, 161, 2, 3, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421
	};

	register int hval = len;
	switch ( hval )
	{
		default:	hval += asso_values [ str[4] ];
		case 4:
		case 3:		hval += asso_values [ str[2] ];
		case 2:		hval += asso_values [ str[1]+1 ];
		case 1:		hval += asso_values [ str[0] ];
					break;
	}
	return hval + asso_values [ str[len-1] ];
}


static inline int HtmlEntityLookup ( const BYTE * str, int len )
{
	static const unsigned char lengthtable[] =
	{
		0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 3,
		4, 3, 3, 5, 3, 6, 5, 5, 3, 4, 4, 5, 3, 4,
		4, 0, 5, 4, 5, 6, 5, 6, 4, 5, 3, 3, 5, 0,
		0, 0, 0, 6, 0, 5, 5, 0, 5, 6, 6, 3, 0, 3,
		5, 3, 0, 6, 0, 4, 3, 6, 3, 6, 6, 6, 6, 5,
		5, 5, 5, 5, 5, 2, 6, 4, 0, 6, 3, 3, 3, 0,
		4, 5, 4, 4, 4, 3, 7, 4, 3, 6, 2, 3, 6, 4,
		3, 6, 5, 6, 5, 5, 4, 2, 0, 0, 4, 6, 8, 0,
		0, 0, 5, 5, 0, 6, 6, 2, 2, 4, 4, 6, 6, 4,
		4, 5, 6, 2, 3, 4, 6, 5, 0, 2, 0, 0, 6, 6,
		6, 6, 6, 4, 6, 5, 0, 6, 4, 5, 4, 6, 6, 0,
		0, 4, 6, 5, 6, 0, 6, 4, 5, 6, 5, 6, 4, 0,
		3, 6, 0, 4, 4, 4, 5, 4, 6, 0, 4, 4, 6, 5,
		6, 7, 2, 2, 6, 2, 5, 2, 5, 0, 0, 0, 4, 4,
		2, 4, 2, 2, 4, 0, 4, 4, 4, 5, 5, 0, 3, 7,
		5, 0, 5, 6, 5, 0, 6, 0, 6, 0, 4, 6, 4, 6,
		6, 2, 6, 0, 5, 5, 4, 6, 6, 0, 5, 6, 4, 4,
		4, 4, 0, 5, 0, 5, 0, 4, 5, 4, 0, 4, 4, 4,
		0, 0, 0, 4, 0, 0, 0, 5, 6, 5, 3, 0, 0, 6,
		5, 4, 5, 5, 5, 5, 0, 5, 5, 0, 5, 0, 0, 0,
		4, 6, 0, 3, 0, 5, 5, 0, 0, 3, 6, 5, 0, 4,
		0, 0, 0, 0, 5, 7, 5, 3, 5, 3, 0, 0, 6, 0,
		6, 0, 0, 7, 0, 0, 5, 0, 5, 0, 0, 0, 0, 5,
		4, 0, 0, 0, 0, 0, 7, 4, 0, 0, 3, 0, 0, 0,
		3, 0, 6, 0, 0, 7, 5, 5, 0, 3, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5,
		5, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0,
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5
	};

	static const struct HtmlEntity_t wordlist[] =
	{
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Rho", 929},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Chi", 935},
		{"phi", 966},
		{"iota", 953},
		{"psi", 968},
		{"int", 8747},
		{"theta", 952},
		{"amp", 38},
		{"there4", 8756},
		{"Theta", 920},
		{"omega", 969},
		{"and", 8743},
		{"prop", 8733},
		{"ensp", 8194},
		{"image", 8465},
		{"not", 172},
		{"isin", 8712},
		{"sdot", 8901},
		{"", 0},
		{"prime", 8242},
		{"prod", 8719},
		{"trade", 8482},
		{"Scaron", 352},
		{"kappa", 954},
		{"thinsp", 8201},
		{"emsp", 8195},
		{"thorn", 254},
		{"eta", 951},
		{"chi", 967},
		{"Kappa", 922},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"scaron", 353},
		{"", 0},
		{"notin", 8713},
		{"ndash", 8211},
		{"", 0},
		{"acute", 180},
		{"otilde", 245},
		{"atilde", 227},
		{"Phi", 934},
		{"", 0},
		{"Psi", 936},
		{"pound", 163},
		{"cap", 8745},
		{"", 0},
		{"otimes", 8855},
		{"", 0},
		{"nbsp", 32},
		{"rho", 961},
		{"ntilde", 241},
		{"eth", 240},
		{"oacute", 243},
		{"aacute", 225},
		{"eacute", 233},
		{"iacute", 237},
		{"nabla", 8711},
		{"Prime", 8243},
		{"ocirc", 244},
		{"acirc", 226},
		{"ecirc", 234},
		{"icirc", 238},
		{"or", 8744},
		{"Yacute", 221},
		{"nsub", 8836},
		{"", 0},
		{"Uacute", 218},
		{"Eta", 919},
		{"ETH", 208},
		{"sup", 8835},
		{"", 0},
		{"supe", 8839},
		{"Ucirc", 219},
		{"sup1", 185},
		{"para", 182},
		{"sup2", 178},
		{"loz", 9674},
		{"omicron", 959},
		{"part", 8706},
		{"cup", 8746},
		{"Ntilde", 209},
		{"Mu", 924},
		{"tau", 964},
		{"uacute", 250},
		{"Iota", 921},
		{"Tau", 932},
		{"rsaquo", 8250},
		{"alpha", 945},
		{"Ccedil", 199},
		{"ucirc", 251},
		{"oline", 8254},
		{"sup3", 179},
		{"nu", 957},
		{"", 0}, {"", 0},
		{"sube", 8838},
		{"Eacute", 201},
		{"thetasym", 977},
		{"", 0}, {"", 0}, {"", 0},
		{"Omega", 937},
		{"Ecirc", 202},
		{"", 0},
		{"lowast", 8727},
		{"iquest", 191},
		{"lt", 60},
		{"gt", 62},
		{"ordm", 186},
		{"euro", 8364},
		{"oslash", 248},
		{"lsaquo", 8249},
		{"zeta", 950},
		{"cong", 8773},
		{"mdash", 8212},
		{"ccedil", 231},
		{"ne", 8800},
		{"sub", 8834},
		{"Zeta", 918},
		{"Lambda", 923},
		{"Gamma", 915},
		{"", 0},
		{"Nu", 925},
		{"", 0}, {"", 0},
		{"ograve", 242},
		{"agrave", 224},
		{"egrave", 232},
		{"igrave", 236},
		{"frac14", 188},
		{"ordf", 170},
		{"Otilde", 213},
		{"infin", 8734},
		{"", 0},
		{"frac12", 189},
		{"beta", 946},
		{"radic", 8730},
		{"darr", 8595},
		{"Iacute", 205},
		{"Ugrave", 217},
		{"", 0}, {"", 0},
		{"harr", 8596},
		{"hearts", 9829},
		{"Icirc", 206},
		{"Oacute", 211},
		{"", 0},
		{"frac34", 190},
		{"cent", 162},
		{"crarr", 8629},
		{"curren", 164},
		{"Ocirc", 212},
		{"brvbar", 166},
		{"sect", 167},
		{"", 0},
		{"ang", 8736},
		{"ugrave", 249},
		{"", 0},
		{"Beta", 914},
		{"uarr", 8593},
		{"dArr", 8659},
		{"asymp", 8776},
		{"perp", 8869},
		{"Dagger", 8225},
		{"", 0},
		{"hArr", 8660},
		{"rang", 9002},
		{"dagger", 8224},
		{"exist", 8707},
		{"Egrave", 200},
		{"Omicron", 927},
		{"mu", 956},
		{"pi", 960},
		{"weierp", 8472},
		{"xi", 958},
		{"clubs", 9827},
		{"Xi", 926},
		{"aring", 229},
		{"", 0}, {"", 0}, {"", 0},
		{"copy", 169},
		{"uArr", 8657},
		{"ni", 8715},
		{"rarr", 8594},
		{"le", 8804},
		{"ge", 8805},
		{"zwnj", 8204},
		{"", 0},
		{"apos", 39},
		{"macr", 175},
		{"lang", 9001},
		{"gamma", 947},
		{"Delta", 916},
		{"", 0},
		{"uml", 168},
		{"alefsym", 8501},
		{"delta", 948},
		{"", 0},
		{"bdquo", 8222},
		{"lambda", 955},
		{"equiv", 8801},
		{"", 0},
		{"Oslash", 216},
		{"", 0},
		{"hellip", 8230},
		{"", 0},
		{"rArr", 8658},
		{"Atilde", 195},
		{"larr", 8592},
		{"spades", 9824},
		{"Igrave", 204},
		{"Pi", 928},
		{"yacute", 253},
		{"", 0},
		{"diams", 9830},
		{"sbquo", 8218},
		{"fnof", 402},
		{"Ograve", 210},
		{"plusmn", 177},
		{"", 0},
		{"rceil", 8969},
		{"Aacute", 193},
		{"ouml", 246},
		{"auml", 228},
		{"euml", 235},
		{"iuml", 239},
		{"", 0},
		{"Acirc", 194},
		{"", 0},
		{"rdquo", 8221},
		{"", 0},
		{"lArr", 8656},
		{"rsquo", 8217},
		{"Yuml", 376},
		{"", 0},
		{"quot", 34},
		{"Uuml", 220},
		{"bull", 8226},
		{"", 0}, {"", 0}, {"", 0},
		{"real", 8476},
		{"", 0}, {"", 0}, {"", 0},
		{"lceil", 8968},
		{"permil", 8240},
		{"upsih", 978},
		{"sum", 8721},
		{"", 0}, {"", 0},
		{"divide", 247},
		{"raquo", 187},
		{"uuml", 252},
		{"ldquo", 8220},
		{"Alpha", 913},
		{"szlig", 223},
		{"lsquo", 8216},
		{"", 0},
		{"Sigma", 931},
		{"tilde", 732},
		{"", 0},
		{"THORN", 222},
		{"", 0}, {"", 0}, {"", 0},
		{"Euml", 203},
		{"rfloor", 8971},
		{"", 0},
		{"lrm", 8206},
		{"", 0},
		{"sigma", 963},
		{"iexcl", 161},
		{"", 0}, {"", 0},
		{"deg", 176},
		{"middot", 183},
		{"laquo", 171},
		{"", 0},
		{"circ", 710},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"frasl", 8260},
		{"epsilon", 949},
		{"oplus", 8853},
		{"yen", 165},
		{"micro", 181},
		{"piv", 982},
		{"", 0}, {"", 0},
		{"lfloor", 8970},
		{"", 0},
		{"Agrave", 192},
		{"", 0}, {"", 0},
		{"Upsilon", 933},
		{"", 0}, {"", 0},
		{"times", 215},
		{"", 0},
		{"cedil", 184},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"minus", 8722},
		{"Iuml", 207},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"upsilon", 965},
		{"Ouml", 214},
		{"", 0}, {"", 0},
		{"rlm", 8207},
		{"", 0}, {"", 0}, {"", 0},
		{"reg", 174},
		{"", 0},
		{"forall", 8704},
		{"", 0}, {"", 0},
		{"Epsilon", 917},
		{"empty", 8709},
		{"OElig", 338},
		{"", 0},
		{"shy", 173},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Aring", 197},
		{"", 0}, {"", 0}, {"", 0},
		{"oelig", 339},
		{"aelig", 230},
		{"", 0},
		{"zwj", 8205},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"sim", 8764},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"yuml", 255},
		{"sigmaf", 962},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Auml", 196},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"AElig", 198}
	};

	const int MIN_WORD_LENGTH		= 2;
	const int MAX_WORD_LENGTH		= 8;
	const int MAX_HASH_VALUE		= 420;

	if ( len<=MAX_WORD_LENGTH && len>=MIN_WORD_LENGTH )
	{
		register int key = HtmlEntityHash ( str, len );
		if ( key<=MAX_HASH_VALUE && key>=0 )
			if ( len==lengthtable[key] )
		{
			register const char * s = wordlist[key].m_sName;
			if ( *str==*s && !memcmp ( str+1, s+1, len-1 ) )
				return wordlist[key].m_iCode;
		}
	}
	return 0;
}


static const BYTE * SkipPI ( const BYTE * s )
{
	assert ( s[0]=='<' && s[1]=='?' );
	s += 2;

	const BYTE * pStart = s;
	const BYTE * pMax = s + 256;
	for ( ; s<pMax && *s; s++ )
	{
		// for now, let's just bail whenever we see ">", like Firefox does!!!
		// that covers the valid case, ie. the closing "?>", just as well
		if ( s[0]=='>' )
			return s+1;
	}

	if ( !*s )
		return s;

	// no closing end marker ever found; just skip non-whitespace after "<?" then
	s = pStart;
	while ( s<pMax && *s && !sphIsSpace(*s) )
		s++;
	return s;
}


void CSphHTMLStripper::Strip ( BYTE * sData ) const
{
	if ( !sData )
		return;

	const BYTE * s = sData;
	BYTE * d = sData;
	while (true)
	{
		/////////////////////////////////////
		// scan until eof, or tag, or entity
		/////////////////////////////////////

		while ( *s && *s!='<' && *s!='&' )
		{
			if ( *s>=0x20 )
				*d++ = *s;
			else
				*d++ = ' ';
			s++;
		}
		if ( !*s )
			break;

		/////////////////
		// handle entity
		/////////////////

		if ( *s=='&' )
		{
			if ( s[1]=='#' )
			{
				// handle "&#number;" and "&#xnumber;" forms
				DWORD uCode = 0;
				s += 2;

				bool bHex = ( *s && ( *s=='x' || *s=='X') );
				if ( !bHex )
				{
					while ( isdigit(*s) )
						uCode = uCode*10 + (*s++) - '0';
				} else
				{
					s++;
					while ( *s )
					{
						if ( isdigit(*s) )
							uCode = uCode*16 + (*s++) - '0';
						else if ( *s>=0x41 && *s<=0x46 )
							uCode = uCode*16 + (*s++) - 'A' + 0xA;
						else if ( *s>=0x61 && *s<=0x66 )
							uCode = uCode*16 + (*s++) - 'a' + 0xA;
						else
							break;
					}
				}

				uCode = uCode % 0x110000; // there is no unicode code-points bigger than this value

				if ( uCode<=0x1f || *s!=';' ) // 0-31 are reserved codes
					continue;

				d += sphUTF8Encode ( d, (int)uCode );
				s++;

			} else
			{
				// skip until ';' or max length
				if ( ( s[1]>='a' && s[1]<='z' ) || ( s[1]>='A' && s[1]<='Z' ) )
				{
					const int MAX_ENTITY_LEN = 8;
					const BYTE * sStart = s+1;
					while ( *s && *s!=';' && s-sStart<=MAX_ENTITY_LEN )
						s++;

					if ( *s==';' )
					{
						int iCode = HtmlEntityLookup ( sStart, (int)(s-sStart) );
						if ( iCode>0 )
						{
							// this is a known entity; encode it
							d += sphUTF8Encode ( d, iCode );
							s++;
							continue;
						}
					}

					// rollback
					s = sStart-1;
				}

				// if we're here, it's not an entity; pass the leading ampersand and rescan
				*d++ = *s++;
			}
			continue;
		}

		//////////////
		// handle tag
		//////////////

		assert ( *s=='<' );
		if ( GetCharIndex(s[1])<0 )
		{
			if ( s[1]=='/' )
			{
				// check if it's valid closing tag
				if ( GetCharIndex(s[2])<0 )
				{
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='!' )
			{
				if ( s[2]=='-' && s[3]=='-' )
				{
					// it's valid comment; scan until comment end
					s += 4; // skip opening '<!--'
					while ( *s )
					{
						if ( s[0]=='-' && s[1]=='-' && s[2]=='>' )
							break;
						s++;
					}
					if ( !*s )
						break;
					s += 3; // skip closing '-->'
					continue;

				} else if ( isalpha(s[2]) )
				{
					// it's <!doctype> style PI; scan until PI end
					s += 2;
					while ( *s && *s!='>' )
					{
						if ( *s=='\'' || *s=='"' )
						{
							s = SkipQuoted ( s );
							while ( isspace(*s) ) s++;
						} else
						{
							s++;
						}
					}
					if ( *s=='>' )
						s++;
					continue;

				} else
				{
					// it's something malformed; just ignore
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='?' )
			{
				// scan until PI end
				s = SkipPI ( s );
				continue;

			} else
			{
				// simply malformed
				*d++ = *s++;
				continue;
			}
		}
		s++; // skip '<'

		//////////////////////////////////////
		// lookup this tag in known tags list
		//////////////////////////////////////

		const StripperTag_t * pTag = NULL;
		int iZoneNameLen = 0;
		const BYTE * sZoneName = NULL;
		s = FindTag ( s, &pTag, &sZoneName, &iZoneNameLen );

		/////////////////////////////////////
		// process tag contents
		// index attributes if needed
		// gracefully handle malformed stuff
		/////////////////////////////////////

#define LOC_SKIP_SPACES() { while ( sphIsSpace(*s) ) s++; if ( !*s || *s=='>' ) break; }

		bool bIndexAttrs = ( pTag && pTag->m_bIndexAttrs );
		while ( *s && *s!='>' )
		{
			LOC_SKIP_SPACES();
			if ( sphIsTagStart(*s) )
			{
				// skip attribute name while it's valid
				const BYTE * sAttr = s;
				while ( sphIsTag(*s) )
					s++;

				// blanks or a value after a valid attribute name?
				if ( sphIsSpace(*s) || *s=='=' )
				{
					const int iAttrLen = (int)( s - sAttr );
					LOC_SKIP_SPACES();

					// a valid name but w/o a value; keep scanning
					if ( *s!='=' )
						continue;

					// got value!
					s++;
					LOC_SKIP_SPACES();

					// check attribute name
					// OPTIMIZE! remove linear search
					int iAttr = -1;
					if ( bIndexAttrs )
					{
						for ( iAttr=0; iAttr<pTag->m_dAttrs.GetLength(); iAttr++ )
						{
							int iLen = strlen ( pTag->m_dAttrs[iAttr].cstr() );
							if ( iLen==iAttrLen && !strncasecmp ( pTag->m_dAttrs[iAttr].cstr(), (const char*)sAttr, iLen ) )
								break;
						}
						if ( iAttr==pTag->m_dAttrs.GetLength() )
							iAttr = -1;
					}

					// process the value
					const BYTE * sVal = s;
					if ( *s=='\'' || *s=='"' )
					{
						// skip quoted value until a matching quote
						s = SkipQuoted ( s );
					} else
					{
						// skip unquoted value until tag end or whitespace
						while ( *s && *s!='>' && !sphIsSpace(*s) )
							s++;
					}

					// if this one is to be indexed, copy it
					if ( iAttr>=0 )
					{
						const BYTE * sMax = s;
						if ( *sVal=='\'' || *sVal=='"' )
						{
							if ( sMax[-1]==sVal[0] )
								sMax--;
							sVal++;
						}
						while ( sVal<sMax )
							*d++ = *sVal++;
						*d++ = ' ';
					}

					// handled the value; keep scanning
					continue;
				}

				// nope, got an invalid character in the sequence (or maybe eof)
				// fall through to an invalid name handler
			}

			// keep skipping until tag end or whitespace
			while ( *s && *s!='>' && !sphIsSpace(*s) )
				s++;
		}

#undef LOC_SKIP_SPACES

		// skip closing angle bracket, if any
		if ( *s )
			s++;

		// unknown tag is done; others might require a bit more work
		if ( !pTag )
		{
			*d++ = ' '; // unknown tags are *not* inline by default
			continue;
		}

		// handle zones
		if ( pTag->m_bZone )
		{
			// should be at tag's end
			assert ( s[0]=='\0' || s[-1]=='>' );

			// emit secret codes
			*d++ = MAGIC_CODE_ZONE;
			for ( int i=0; i<iZoneNameLen; i++ )
				*d++ = (BYTE) tolower ( sZoneName[i] );
			if ( *d )
				*d++ = MAGIC_CODE_ZONE;

			if ( !*s )
				break;
			continue;
		}

		// handle paragraph boundaries
		if ( pTag->m_bPara )
		{
			*d++ = MAGIC_CODE_PARAGRAPH;
			continue;
		}

		// in all cases, the tag must be fully processed at this point
		// not a remove-tag? we're done
		if ( !pTag->m_bRemove )
		{
			if ( !pTag->m_bInline )
				*d++ = ' ';
			continue;
		}

		// sudden eof? bail out
		if ( !*s )
			break;

		// must be a proper remove-tag end, then
		assert ( pTag->m_bRemove && s[-1]=='>' );

		// short-form? we're done
		if ( s[-2]=='/' )
			continue;

		// skip everything until the closing tag
		// FIXME! should we handle insane cases with quoted closing tag within tag?
		while (true)
		{
			while ( *s && ( s[0]!='<' || s[1]!='/' ) ) s++;
			if ( !*s ) break;

			s += 2; // skip </
			if ( strncasecmp ( pTag->m_sTag.cstr(), (const char*)s, pTag->m_iTagLen )!=0 ) continue;
			if ( !sphIsTag ( s[pTag->m_iTagLen] ) )
			{
				s += pTag->m_iTagLen; // skip tag
				if ( *s=='>' ) s++;
				break;
			}
		}

		if ( !pTag->m_bInline ) *d++ = ' ';
	}
	*d++ = '\0';

	// space, paragraph sequences elimination pass
	s = sData;
	d = sData;
	bool bSpaceOut = false;
	bool bParaOut = false;
	bool bZoneOut = false;
	while ( const char c = *s++ )
	{
		assert ( d<=s-1 );

		// handle different character classes
		if ( sphIsSpace(c) )
		{
			// handle whitespace, skip dupes
			if ( !bSpaceOut )
				*d++ = ' ';

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_PARAGRAPH )
		{
			// handle paragraph marker, skip dupes
			if ( !bParaOut && !bZoneOut )
			{
				*d++ = c;
				bParaOut = true;
			}

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_ZONE )
		{
			// zone marker
			// rewind preceding paragraph, if any, it is redundant
			if ( bParaOut )
			{
				assert ( d>sData && d[-1]==MAGIC_CODE_PARAGRAPH );
				d--;
			}

			// copy \4zoneid\4
			*d++ = c;
			while ( *s && *s!=MAGIC_CODE_ZONE )
				*d++ = *s++;

			if ( *s )
				*d++ = *s++;

			// update state
			// no spaces paragraphs allowed
			bSpaceOut = bZoneOut = true;
			bParaOut = false;
			continue;

		} else
		{
			*d++ = c;
			bSpaceOut = bParaOut = bZoneOut = false;
		}
	}
	*d++ = '\0';
}

const BYTE * CSphHTMLStripper::FindTag ( const BYTE * sSrc, const StripperTag_t ** ppTag,
	const BYTE ** ppZoneName, int * pZoneNameLen ) const
{
	assert ( sSrc && ppTag && ppZoneName && pZoneNameLen );
	assert ( sSrc[0]!='/' || sSrc[1]!='\0' );

	const BYTE * sTagName = ( sSrc[0]=='/' ) ? sSrc+1 : sSrc;

	*ppZoneName = sSrc;
	*pZoneNameLen = 0;

	int iIdx = GetCharIndex ( sTagName[0] );
	assert ( iIdx>=0 && iIdx<MAX_CHAR_INDEX );

	if ( iIdx>=0 && m_dEnd[iIdx]>=0 )
	{
		int iStart = m_dStart[iIdx];
		int iEnd = m_dEnd[iIdx];

		for ( int i=iStart; i<=iEnd; i++ )
		{
			int iLen = m_dTags[i].m_iTagLen;
			int iCmp = strncasecmp ( m_dTags[i].m_sTag.cstr(), (const char*)sTagName, iLen );

			// the tags are sorted; so if current candidate is already greater, rest can be skipped
			if ( iCmp>0 )
				break;

			// do we have a match?
			if ( iCmp==0 )
			{
				// got exact match?
				if ( !sphIsTag ( sTagName[iLen] ) )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen; // skip tag name
					if ( m_dTags[i].m_bZone )
						*pZoneNameLen = sSrc - *ppZoneName;
					break;
				}

				// got wildcard match?
				if ( m_dTags[i].m_bZonePrefix )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen;
					while ( sphIsTag(*sSrc) )
						sSrc++;
					*pZoneNameLen = sSrc - *ppZoneName;
					break;
				}
			}
		}
	}

	return sSrc;
}

bool CSphHTMLStripper::IsValidTagStart ( int iCh ) const
{
	int i = GetCharIndex ( iCh );
	return ( i>=0 && i<MAX_CHAR_INDEX );
}

//////////////////////////////////////////////////////////////////////////
ISphFieldFilter::ISphFieldFilter()
	: m_pParent ( NULL )
{
}


ISphFieldFilter::~ISphFieldFilter()
{
	SafeRelease ( m_pParent );
}


void ISphFieldFilter::SetParent ( ISphFieldFilter * pParent )
{
	SafeAddRef ( pParent );
	SafeRelease ( m_pParent );
	m_pParent = pParent;
}


#if USE_RE2
class CSphFieldRegExps : public ISphFieldFilter
{
protected:
	~CSphFieldRegExps() override;
public:
						CSphFieldRegExps () = default;
						CSphFieldRegExps ( const StrVec_t& m_dRegexps, CSphString &	sError );

	int					Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool ) final;
	void				GetSettings ( CSphFieldFilterSettings & tSettings ) const final;
	ISphFieldFilter *	Clone() final;

	bool				AddRegExp ( const char * sRegExp, CSphString & sError );

private:
	struct RegExp_t
	{
		CSphString	m_sFrom;
		CSphString	m_sTo;

		RE2 *		m_pRE2;
	};

	CSphVector<RegExp_t>	m_dRegexps;
	bool					m_bCloned = true;
};

CSphFieldRegExps::CSphFieldRegExps ( const StrVec_t &dRegexps, CSphString &sError )
	: m_bCloned ( false )
{
	for ( const auto &sRegexp : dRegexps )
		AddRegExp ( sRegexp.cstr (), sError );
}


CSphFieldRegExps::~CSphFieldRegExps ()
{
	if ( !m_bCloned )
	{
		for ( auto & dRegexp : m_dRegexps )
			SafeDelete ( dRegexp.m_pRE2 );
	}
}


int CSphFieldRegExps::Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool )
{
	dStorage.Resize ( 0 );
	if ( !sField || !*sField )
		return 0;

	bool bReplaced = false;
	std::string sRe2 = ( iLength ? std::string ( (char *) sField, iLength ) : (char *) sField );
	ARRAY_FOREACH ( i, m_dRegexps )
	{
		assert ( m_dRegexps[i].m_pRE2 );
		bReplaced |= ( RE2::GlobalReplace ( &sRe2, *m_dRegexps[i].m_pRE2, m_dRegexps[i].m_sTo.cstr() )>0 );
	}

	if ( !bReplaced )
		return 0;

	int iDstLen = sRe2.length();
	dStorage.Resize ( iDstLen+4 ); // string SAFETY_GAP
	strncpy ( (char *)dStorage.Begin(), sRe2.c_str(), dStorage.GetLength() );
	return iDstLen;
}


void CSphFieldRegExps::GetSettings ( CSphFieldFilterSettings & tSettings ) const
{
	tSettings.m_dRegexps.Resize ( m_dRegexps.GetLength() );
	ARRAY_FOREACH ( i, m_dRegexps )
		tSettings.m_dRegexps[i].SetSprintf ( "%s => %s", m_dRegexps[i].m_sFrom.cstr(), m_dRegexps[i].m_sTo.cstr() );
}


bool CSphFieldRegExps::AddRegExp ( const char * sRegExp, CSphString & sError )
{
	if ( m_bCloned )
		return false;

	const char sSplitter [] = "=>";
	const char * sSplit = strstr ( sRegExp, sSplitter );
	if ( !sSplit )
	{
		sError = "mapping token (=>) not found";
		return false;
	} else if ( strstr ( sSplit + strlen ( sSplitter ), sSplitter ) )
	{
		sError = "mapping token (=>) found more than once";
		return false;
	}

	m_dRegexps.Resize ( m_dRegexps.GetLength () + 1 );
	RegExp_t & tRegExp = m_dRegexps.Last();
	tRegExp.m_sFrom.SetBinary ( sRegExp, sSplit-sRegExp );
	tRegExp.m_sTo = sSplit + strlen ( sSplitter );
	tRegExp.m_sFrom.Trim();
	tRegExp.m_sTo.Trim();

	RE2::Options tOptions;
	tOptions.set_utf8 ( true );
	tRegExp.m_pRE2 = new RE2 ( tRegExp.m_sFrom.cstr(), tOptions );

	std::string sRE2Error;
	if ( !tRegExp.m_pRE2->CheckRewriteString ( tRegExp.m_sTo.cstr(), &sRE2Error ) )
	{
		sError.SetSprintf ( "\"%s => %s\" is not a valid mapping: %s", tRegExp.m_sFrom.cstr(), tRegExp.m_sTo.cstr(), sRE2Error.c_str() );
		SafeDelete ( tRegExp.m_pRE2 );
		m_dRegexps.Remove ( m_dRegexps.GetLength() - 1 );
		return false;
	}

	return true;
}


ISphFieldFilter * CSphFieldRegExps::Clone()
{
	CSphFieldRegExps * pCloned = new CSphFieldRegExps;
	pCloned->m_dRegexps = m_dRegexps;

	return pCloned;
}
#endif


#if USE_RE2
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError )
{
	return new CSphFieldRegExps ( tFilterSettings.m_dRegexps, sError );
}
#else
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings &, CSphString & )
{
	return nullptr;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

ESphWordpart CSphSourceSettings::GetWordpart ( const char * sField, bool bWordDict )
{
	if ( bWordDict )
		return SPH_WORDPART_WHOLE;

	bool bPrefix = ( m_iMinPrefixLen>0 ) && ( m_dPrefixFields.IsEmpty () || m_dPrefixFields.Contains ( sField ) );
	bool bInfix = ( m_iMinInfixLen>0 ) && ( m_dInfixFields.IsEmpty() || m_dInfixFields.Contains ( sField ) );

	assert ( !( bPrefix && bInfix ) ); // no field must be marked both prefix and infix
	if ( bPrefix )
		return SPH_WORDPART_PREFIX;
	if ( bInfix )
		return SPH_WORDPART_INFIX;
	return SPH_WORDPART_WHOLE;
}

//////////////////////////////////////////////////////////////////////////

bool ParseMorphFields ( const CSphString & sMorphology, const CSphString & sMorphFields, const CSphVector<CSphColumnInfo> & dFields, CSphBitvec & tMorphFields, CSphString & sError )
{
	if ( sMorphology.IsEmpty() || sMorphFields.IsEmpty() )
		return true;

	CSphString sFields = sMorphFields;
	sFields.ToLower();
	sFields.Trim();
	if ( !sFields.Length() )
		return true;

	OpenHash_T<int, int64_t, HashFunc_Int64_t> hFields;
	ARRAY_FOREACH ( i, dFields )
		hFields.Add ( sphFNV64 ( dFields[i].m_sName.cstr() ), i );

	StringBuilder_c sMissed;
	int iFieldsGot = 0;
	tMorphFields.Init ( dFields.GetLength() );
	tMorphFields.Set(); // all fields have morphology by default

	for ( const char * sCur=sFields.cstr(); ; )
	{
		while ( *sCur && ( sphIsSpace ( *sCur ) || *sCur==',' ) )
			++sCur;
		if ( !*sCur )
			break;

		const char * sStart = sCur;
		while ( *sCur && !sphIsSpace ( *sCur ) && *sCur!=',' )
			++sCur;

		if ( sCur==sStart )
			break;

		int * pField = hFields.Find ( sphFNV64 ( sStart, sCur - sStart ) );
		if ( !pField )
		{
			const char * sSep = sMissed.GetLength() ? ", " : "";
			sMissed.Appendf ( "%s%.*s", sSep, (int)(sCur - sStart), sStart );
			break;
		}

		iFieldsGot++;
		tMorphFields.BitClear ( *pField );
	}

	// no fields set - need to reset bitvec to skip checks on indexing
	if ( !iFieldsGot )
		tMorphFields.Init ( 0 );

	if ( sMissed.GetLength() )
		sError.SetSprintf ( "morphology_skip_fields contains out of schema fields: %s", sMissed.cstr() );

	return ( !sMissed.GetLength() );
}

CSphSource::CSphSource ( const char * sName )
	: m_tSchema ( sName )
{
}


CSphSource::~CSphSource()
{
	SafeDelete ( m_pStripper );
}


void CSphSource::SetDict ( CSphDict * pDict )
{
	assert ( pDict );
	SafeAddRef ( pDict );
	m_pDict = pDict;
}


const CSphSourceStats & CSphSource::GetStats ()
{
	return m_tStats;
}


bool CSphSource::SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements,
	bool bDetectParagraphs, const char * sZones, CSphString & sError )
{
	if ( !m_pStripper )
		m_pStripper = new CSphHTMLStripper ( true );

	if ( !m_pStripper->SetIndexedAttrs ( sExtractAttrs, sError ) )
		return false;

	if ( !m_pStripper->SetRemovedElements ( sRemoveElements, sError ) )
		return false;

	if ( bDetectParagraphs )
		m_pStripper->EnableParagraphs ();

	if ( !m_pStripper->SetZones ( sZones, sError ) )
		return false;

	return true;
}


void CSphSource::SetFieldFilter ( ISphFieldFilter * pFilter )
{
	SafeAddRef ( pFilter );
	m_pFieldFilter = pFilter;
}

void CSphSource::SetTokenizer ( ISphTokenizer * pTokenizer )
{
	assert ( pTokenizer );
	SafeAddRef ( pTokenizer );
	m_pTokenizer = pTokenizer;
}


bool CSphSource::UpdateSchema ( CSphSchema * pInfo, CSphString & sError )
{
	assert ( pInfo );

	// fill it
	if ( pInfo->GetFieldsCount()==0 && pInfo->GetAttrsCount()==0 )
	{
		*pInfo = m_tSchema;
		return true;
	}

	// check it
	return m_tSchema.CompareTo ( *pInfo, sError );
}


void CSphSource::Setup ( const CSphSourceSettings & tSettings )
{
	m_iMinPrefixLen = Max ( tSettings.m_iMinPrefixLen, 0 );
	m_iMinInfixLen = Max ( tSettings.m_iMinInfixLen, 0 );
	m_iMaxSubstringLen = Max ( tSettings.m_iMaxSubstringLen, 0 );
	m_iBoundaryStep = Max ( tSettings.m_iBoundaryStep, -1 );
	m_bIndexExactWords = tSettings.m_bIndexExactWords;
	m_iOvershortStep = Min ( Max ( tSettings.m_iOvershortStep, 0 ), 1 );
	m_iStopwordStep = Min ( Max ( tSettings.m_iStopwordStep, 0 ), 1 );
	m_bIndexSP = tSettings.m_bIndexSP;
	m_dPrefixFields = tSettings.m_dPrefixFields;
	m_dInfixFields = tSettings.m_dInfixFields;
	m_bIndexFieldLens = tSettings.m_bIndexFieldLens;
}


bool CSphSource::SetupMorphFields ( CSphString & sError )
{
	return ParseMorphFields ( m_pDict->GetSettings().m_sMorphology, m_pDict->GetSettings().m_sMorphFields, m_tSchema.GetFields(), m_tMorphFields, sError );
}


ISphHits * CSphSource::IterateJoinedHits ( CSphString & )
{
	static ISphHits dDummy;
	m_tDocInfo.m_tRowID = INVALID_ROWID; // pretend that's an eof
	return &dDummy;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

static void FormatEscaped ( FILE * fp, const char * sLine )
{
	// handle empty lines
	if ( !sLine || !*sLine )
	{
		fprintf ( fp, "''" );
		return;
	}

	// pass one, count the needed buffer size
	int iLen = strlen(sLine);
	int iOut = 0;
	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\t':
		case '\'':
		case '\\':
			iOut += 2;
			break;

		default:
			iOut++;
			break;
	}
	iOut += 2; // quotes

	// allocate the buffer
	char sMinibuffer[8192];
	char * sMaxibuffer = NULL;
	char * sBuffer = sMinibuffer;

	if ( iOut>(int)sizeof(sMinibuffer) )
	{
		sMaxibuffer = new char [ iOut+4 ]; // 4 is just my safety gap
		sBuffer = sMaxibuffer;
	}

	// pass two, escape it
	char * sOut = sBuffer;
	*sOut++ = '\'';

	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\t':
		case '\'':
		case '\\':	*sOut++ = '\\'; // no break intended
		default:	*sOut++ = sLine[i];
	}
	*sOut++ = '\'';

	// print!
	assert ( sOut==sBuffer+iOut );
	fwrite ( sBuffer, 1, iOut, fp );

	// cleanup
	SafeDeleteArray ( sMaxibuffer );
}


CSphSource_Document::CSphBuildHitsState_t::CSphBuildHitsState_t ()
{
	Reset();
}

CSphSource_Document::CSphBuildHitsState_t::~CSphBuildHitsState_t ()
{
	Reset();
}

void CSphSource_Document::CSphBuildHitsState_t::Reset ()
{
	m_bProcessingHits = false;
	m_bDocumentDone = false;
	m_dFields = nullptr;
	m_dFieldLengths.Resize(0);
	m_iStartPos = 0;
	m_iHitPos = 0;
	m_iField = 0;
	m_iStartField = 0;
	m_iEndField = 0;
	m_iBuildLastStep = 1;

	ARRAY_FOREACH ( i, m_dTmpFieldStorage )
		SafeDeleteArray ( m_dTmpFieldStorage[i] );

	m_dTmpFieldStorage.Resize ( 0 );
	m_dTmpFieldPtrs.Resize ( 0 );
	m_dFiltered.Resize ( 0 );
}

CSphSource_Document::CSphSource_Document ( const char * sName )
	: CSphSource ( sName )
	, m_iMaxHits ( MAX_SOURCE_HITS )
{
}


bool CSphSource_Document::IterateDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_pTokenizer );
	assert ( !m_tState.m_bProcessingHits );

	m_tHits.m_dData.Resize ( 0 );

	m_tState.Reset();
	m_tState.m_iEndField = m_iPlainFieldsLength;
	m_tState.m_dFieldLengths.Resize ( m_tState.m_iEndField );

	if ( m_pFieldFilter )
	{
		m_tState.m_dTmpFieldPtrs.Resize ( m_tState.m_iEndField );
		m_tState.m_dTmpFieldStorage.Resize ( m_tState.m_iEndField );

		ARRAY_FOREACH ( i, m_tState.m_dTmpFieldPtrs )
		{
			m_tState.m_dTmpFieldPtrs[i] = NULL;
			m_tState.m_dTmpFieldStorage[i] = NULL;
		}
	}

	// fetch next document
	while (true)
	{
		m_tState.m_dFields = NextDocument ( bEOF, sError );
		if ( bEOF )
			return true;

		const int * pFieldLengths = GetFieldLengths ();
		for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
			m_tState.m_dFieldLengths[iField] = pFieldLengths[iField];

		if ( !m_tState.m_dFields )
			return false;

		// tricky bit
		// we can only skip document indexing from here, IterateHits() is too late
		// so in case the user chose to skip documents with file field problems
		// we need to check for those here
		if ( m_eOnFileFieldError==FFE_SKIP_DOCUMENT || m_eOnFileFieldError==FFE_FAIL_INDEX )
		{
			bool bOk = true;
			for ( int iField=0; iField<m_tState.m_iEndField && bOk; iField++ )
			{
				const BYTE * sFilename = m_tState.m_dFields[iField];
				if ( m_tSchema.GetField(iField).m_bFilename )
					bOk &= CheckFileField ( sFilename );

				if ( !bOk && m_eOnFileFieldError==FFE_FAIL_INDEX )
				{
					sError.SetSprintf ( "error reading file field data (docid=" INT64_FMT ", filename=%s)",	sphGetDocID ( m_tDocInfo.m_pDynamic ), sFilename );
					return false;
				}
			}
			if ( !bOk && m_eOnFileFieldError==FFE_SKIP_DOCUMENT )
				continue;
		}

		if ( m_pFieldFilter )
		{
			bool bHaveModifiedFields = false;
			for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
			{
				if ( m_tSchema.GetField(iField).m_bFilename )
				{
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
					continue;
				}

				CSphVector<BYTE> dFiltered;
				int iFilteredLen = m_pFieldFilter->Apply ( m_tState.m_dFields[iField], m_tState.m_dFieldLengths[iField], dFiltered, false );
				if ( iFilteredLen )
				{
					m_tState.m_dTmpFieldStorage[iField] = dFiltered.LeakData();
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dTmpFieldStorage[iField];
					m_tState.m_dFieldLengths[iField] = iFilteredLen;
					bHaveModifiedFields = true;
				} else
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
			}

			if ( bHaveModifiedFields )
				m_tState.m_dFields = (BYTE **)&( m_tState.m_dTmpFieldPtrs[0] );
		}

		// we're good
		break;
	}

	m_tStats.m_iTotalDocuments++;
	return true;
}


// hack notification for joined fields
void CSphSource_Document::RowIDAssigned ( DocID_t tDocID, RowID_t tRowID )
{
	if ( HasJoinedFields() )
	{
		IDPair_t & tPair = m_dAllIds.Add();
		tPair.m_tDocID = tDocID;
		tPair.m_tRowID = tRowID;
	}
}


ISphHits * CSphSource_Document::IterateHits ( CSphString & sError )
{
	if ( m_tState.m_bDocumentDone )
		return NULL;

	m_tHits.m_dData.Resize ( 0 );

	BuildHits ( sError, false );

	return &m_tHits;
}


bool CSphSource_Document::CheckFileField ( const BYTE * sField )
{
	CSphAutofile tFileSource;
	CSphString sError;

	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return false;
	}

	return true;
}


/// returns file size on success, and replaces *ppField with a pointer to data
/// returns -1 on failure (and emits a warning)
int CSphSource_Document::LoadFileField ( BYTE ** ppField, CSphString & sError )
{
	CSphAutofile tFileSource;

	BYTE * sField = *ppField;
	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return -1;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return -1;
	}

	int iFieldBytes = (int)iFileSize;
	if ( !iFieldBytes )
		return 0;

	int iBufSize = Max ( m_iReadFileBufferSize, 1 << sphLog2 ( iFieldBytes+15 ) );
	if ( m_iReadFileBufferSize < iBufSize )
		SafeDeleteArray ( m_pReadFileBuffer );

	if ( !m_pReadFileBuffer )
	{
		m_pReadFileBuffer = new char [ iBufSize ];
		m_iReadFileBufferSize = iBufSize;
	}

	if ( !tFileSource.Read ( m_pReadFileBuffer, iFieldBytes, sError ) )
	{
		sphWarning ( "read failed: %s", sError.cstr() );
		return -1;
	}

	m_pReadFileBuffer[iFieldBytes] = '\0';

	*ppField = (BYTE*)m_pReadFileBuffer;
	return iFieldBytes;
}


bool AddFieldLens ( CSphSchema & tSchema, bool bDynamic, CSphString & sError )
{
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		CSphColumnInfo tCol;
		tCol.m_sName.SetSprintf ( "%s_len", tSchema.GetFieldName(i) );

		int iGot = tSchema.GetAttrIndex ( tCol.m_sName.cstr() );
		if ( iGot>=0 )
		{
			if ( tSchema.GetAttr(iGot).m_eAttrType==SPH_ATTR_TOKENCOUNT )
			{
				// looks like we already added these
				assert ( tSchema.GetAttr(iGot).m_sName==tCol.m_sName );
				return true;
			}

			sError.SetSprintf ( "attribute %s conflicts with index_field_lengths=1; remove it", tCol.m_sName.cstr() );
			return false;
		}

		tCol.m_eAttrType = SPH_ATTR_TOKENCOUNT;
		tSchema.AddAttr ( tCol, bDynamic ); // everything's dynamic at indexing time
	}
	return true;
}


bool CSphSource_Document::AddAutoAttrs ( CSphString & sError, StrVec_t * pDefaults )
{
	// id is the first attr
	if ( m_tSchema.GetAttr ( sphGetDocidName() ) )
	{
		assert ( m_tSchema.GetAttrIndex ( sphGetDocidName() )==0 );
		assert ( m_tSchema.GetAttr ( sphGetDocidName() )->m_eAttrType==SPH_ATTR_BIGINT );
	} else
	{
		CSphColumnInfo tCol ( sphGetDocidName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;
		m_tSchema.InsertAttr ( 0, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 0, "" );
	}

	if ( m_tSchema.HasBlobAttrs() && !m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		CSphColumnInfo tCol ( sphGetBlobLocatorName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		m_tSchema.InsertAttr ( 1, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 1, "" );
	}

	// rebuild locators in the schema
	const char * szTmpColName = "$_tmp";
	CSphColumnInfo tCol ( szTmpColName, SPH_ATTR_BIGINT );
	m_tSchema.AddAttr ( tCol, true );
	m_tSchema.RemoveAttr ( szTmpColName, true );

	// auto-computed length attributes
	if ( m_bIndexFieldLens )
		return AddFieldLens ( m_tSchema, true, sError );

	return true;
}


void CSphSource_Document::AllocDocinfo()
{
	// tricky bit
	// with in-config schema, attr storage gets allocated in Setup() when source is initially created
	// so when this AddAutoAttrs() additionally changes the count, we have to change the number of attributes
	// but Reset() prohibits that, because that is usually a programming mistake, hence the Swap() dance
	CSphMatch tNew;
	tNew.Reset ( m_tSchema.GetRowSize() );
	Swap ( m_tDocInfo, tNew );

	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );

	if ( m_bIndexFieldLens && m_tSchema.GetAttrsCount() && m_tSchema.GetFieldsCount() )
	{
		int iFirst = m_tSchema.GetAttrId_FirstFieldLen();
		assert ( m_tSchema.GetAttr ( iFirst ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		assert ( m_tSchema.GetAttr ( iFirst+m_tSchema.GetFieldsCount()-1 ).m_eAttrType==SPH_ATTR_TOKENCOUNT );

		m_pFieldLengthAttrs = m_tDocInfo.m_pDynamic + ( m_tSchema.GetAttr ( iFirst ).m_tLocator.m_iBitOffset / 32 );
	}
}

//////////////////////////////////////////////////////////////////////////
// HIT GENERATORS
//////////////////////////////////////////////////////////////////////////

bool CSphSource_Document::BuildZoneHits ( RowID_t tRowID, BYTE * sWord )
{
	if ( *sWord==MAGIC_CODE_SENTENCE || *sWord==MAGIC_CODE_PARAGRAPH || *sWord==MAGIC_CODE_ZONE )
	{
		m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( (BYTE*)MAGIC_WORD_SENTENCE ), m_tState.m_iHitPos );

		if ( *sWord==MAGIC_CODE_PARAGRAPH || *sWord==MAGIC_CODE_ZONE )
			m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( (BYTE*)MAGIC_WORD_PARAGRAPH ), m_tState.m_iHitPos );

		if ( *sWord==MAGIC_CODE_ZONE )
		{
			BYTE * pZone = (BYTE*) m_pTokenizer->GetBufferPtr();
			BYTE * pEnd = pZone;
			while ( *pEnd && *pEnd!=MAGIC_CODE_ZONE )
			{
				pEnd++;
			}

			if ( *pEnd && *pEnd==MAGIC_CODE_ZONE )
			{
				*pEnd = '\0';
				m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( pZone-1 ), m_tState.m_iHitPos );
				m_pTokenizer->SetBufferPtr ( (const char*) pEnd+1 );
			}
		}

		m_tState.m_iBuildLastStep = 1;
		return true;
	}
	return false;
}


// track blended start and reset on not blended token
static int TrackBlendedStart ( const ISphTokenizer * pTokenizer, int iBlendedHitsStart, int iHitsCount )
{
	iBlendedHitsStart = ( ( pTokenizer->TokenIsBlended() || pTokenizer->TokenIsBlendedPart() ) ? iBlendedHitsStart : -1 );
	if ( pTokenizer->TokenIsBlended() )
		iBlendedHitsStart = iHitsCount;

	return iBlendedHitsStart;
}


#define BUILD_SUBSTRING_HITS_COUNT 4

void CSphSource_Document::BuildSubstringHits ( RowID_t tRowID, bool bPayload, ESphWordpart eWordpart, bool bSkipEndMarker )
{
	bool bPrefixField = ( eWordpart==SPH_WORDPART_PREFIX );
	bool bInfixMode = m_iMinInfixLen > 0;

	int iMinInfixLen = bPrefixField ? m_iMinPrefixLen : m_iMinInfixLen;
	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iIterHitCount = BUILD_SUBSTRING_HITS_COUNT;
	if ( bPrefixField )
		iIterHitCount += SPH_MAX_WORD_LEN - m_iMinPrefixLen;
	else
		iIterHitCount += ( ( m_iMinInfixLen+SPH_MAX_WORD_LEN ) * ( SPH_MAX_WORD_LEN-m_iMinInfixLen ) / 2 );

	// FIELDEND_MASK at blended token stream should be set for HEAD token too
	int iBlendedHitsStart = -1;

	// index all infixes
	while ( ( m_iMaxHits==0 || m_tHits.m_dData.GetLength()+iIterHitCount<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.Length() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
			m_tState.m_iBuildLastStep = 1;
		}

		if ( BuildZoneHits ( tRowID, sWord ) )
			continue;

		int iLen = m_pTokenizer->GetLastTokenLen ();

		// always index full word (with magic head/tail marker(s))
		int iBytes = strlen ( (const char*)sWord );
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = '\0';

		SphWordID_t uExactWordid = 0;
		if ( m_bIndexExactWords )
		{
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			uExactWordid = m_pDict->GetWordIDNonStemmed ( sBuf );
		}

		sBuf[0] = MAGIC_WORD_HEAD;

		// stemmed word w/markers
		SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
		if ( !iWord )
		{
			m_tState.m_iBuildLastStep = m_iStopwordStep;
			continue;
		}

		if ( m_bIndexExactWords )
			m_tHits.AddHit ( tRowID, uExactWordid, m_tState.m_iHitPos );
		iBlendedHitsStart = iLastBlendedStart;
		m_tHits.AddHit ( tRowID, iWord, m_tState.m_iHitPos );
		m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;

		// restore stemmed word
		int iStemmedLen = strlen ( ( const char *)sBuf );
		sBuf [iStemmedLen - 1] = '\0';

		// stemmed word w/o markers
		if ( strcmp ( (const char *)sBuf + 1, (const char *)sWord ) )
			m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( sBuf + 1, iStemmedLen - 2, true ), m_tState.m_iHitPos );

		// restore word
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = MAGIC_WORD_TAIL;
		sBuf[iBytes+2] = '\0';

		// if there are no infixes, that's it
		if ( iMinInfixLen > iLen )
		{
			// index full word
			m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( sWord ), m_tState.m_iHitPos );
			continue;
		}

		// process all infixes
		int iMaxStart = bPrefixField ? 0 : ( iLen - iMinInfixLen );

		BYTE * sInfix = sBuf + 1;

		for ( int iStart=0; iStart<=iMaxStart; iStart++ )
		{
			BYTE * sInfixEnd = sInfix;
			for ( int i = 0; i < iMinInfixLen; i++ )
				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );

			int iMaxSubLen = ( iLen-iStart );
			if ( m_iMaxSubstringLen )
				iMaxSubLen = Min ( m_iMaxSubstringLen, iMaxSubLen );

			for ( int i=iMinInfixLen; i<=iMaxSubLen; i++ )
			{
				m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix, false ), m_tState.m_iHitPos );

				// word start: add magic head
				if ( bInfixMode && iStart==0 )
					m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( sInfix - 1, sInfixEnd-sInfix + 1, false ), m_tState.m_iHitPos );

				// word end: add magic tail
				if ( bInfixMode && i==iLen-iStart )
					m_tHits.AddHit ( tRowID, m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix+1, false ), m_tState.m_iHitPos );

				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );
			}

			sInfix += m_pTokenizer->GetCodepointLength ( *sInfix );
		}
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );

	// mark trailing hits
	// and compute fields lengths
	if ( !bSkipEndMarker && !m_tState.m_bProcessingHits && m_tHits.Length() )
	{
		CSphWordHit * pTail = const_cast < CSphWordHit * > ( m_tHits.Last() );

		if ( m_pFieldLengthAttrs )
			m_pFieldLengthAttrs [ HITMAN::GetField ( pTail->m_uWordPos ) ] = HITMAN::GetPos ( pTail->m_uWordPos );

		Hitpos_t uEndPos = pTail->m_uWordPos;
		if ( iBlendedHitsStart>=0 )
		{
			assert ( iBlendedHitsStart>=0 && iBlendedHitsStart<m_tHits.Length() );
			Hitpos_t uBlendedPos = ( m_tHits.First() + iBlendedHitsStart )->m_uWordPos;
			uEndPos = Min ( uEndPos, uBlendedPos );
		}

		// set end marker for all tail hits
		const CSphWordHit * pStart = m_tHits.First();
		while ( pStart<=pTail && uEndPos<=pTail->m_uWordPos )
		{
			HITMAN::SetEndMarker ( &pTail->m_uWordPos );
			pTail--;
		}
	}
}


#define BUILD_REGULAR_HITS_COUNT 6

void CSphSource_Document::BuildRegularHits ( RowID_t tRowID, bool bPayload, bool bSkipEndMarker )
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;
	bool bGlobalPartialMatch = !bWordDict && ( m_iMinPrefixLen > 0 || m_iMinInfixLen > 0 );

	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	// FIELDEND_MASK at last token stream should be set for HEAD token too
	int iBlendedHitsStart = -1;
	bool bMorphDisabled = ( m_tMorphFields.GetBits()>0 && !m_tMorphFields.BitGet ( m_tState.m_iField ) );

	// index words only
	while ( ( m_iMaxHits==0 || m_tHits.m_dData.GetLength()+BUILD_REGULAR_HITS_COUNT<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.Length() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
		}

		if ( BuildZoneHits ( tRowID, sWord ) )
			continue;

		if ( bGlobalPartialMatch )
		{
			int iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD;
			sBuf[iBytes+1] = '\0';
			m_tHits.AddHit ( tRowID, m_pDict->GetWordIDWithMarkers ( sBuf ), m_tState.m_iHitPos );
		}

		ESphTokenMorph eMorph = m_pTokenizer->GetTokenMorph();
		if ( m_bIndexExactWords && eMorph!=SPH_TOKEN_MORPH_GUESS )
		{
			int iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			sBuf[iBytes+1] = '\0';
		}

		if ( m_bIndexExactWords && ( eMorph==SPH_TOKEN_MORPH_ORIGINAL || bMorphDisabled ) )
		{
			// can not use GetWordID here due to exception vs missed hit, ie
			// stemmed sWord hasn't got added to hit stream but might be added as exception to dictionary
			// that causes error at hit sorting phase \ dictionary HitblockPatch
			if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed )
				m_pDict->ApplyStemmers ( sWord );

			if ( !m_pDict->IsStopWord ( sWord ) )
				m_tHits.AddHit ( tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos );

			m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
			continue;
		}

		SphWordID_t iWord = ( eMorph==SPH_TOKEN_MORPH_GUESS )
			? m_pDict->GetWordIDNonStemmed ( sWord ) // tokenizer did morphology => dict must not stem
			: m_pDict->GetWordID ( sWord ); // tokenizer did not => stemmers can be applied
		if ( iWord )
		{
#if 0
			if ( HITMAN::GetPos ( m_tState.m_iHitPos )==1 )
				printf ( "\n" );
			printf ( "doc %d. pos %d. %s\n", uDocid, HITMAN::GetPos ( m_tState.m_iHitPos ), sWord );
#endif
			iBlendedHitsStart = iLastBlendedStart;
			m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
			m_tHits.AddHit ( tRowID, iWord, m_tState.m_iHitPos );
			if ( m_bIndexExactWords && eMorph!=SPH_TOKEN_MORPH_GUESS )
				m_tHits.AddHit ( tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos );
		} else
			m_tState.m_iBuildLastStep = m_iStopwordStep;
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );

	// mark trailing hit
	// and compute field lengths
	if ( !bSkipEndMarker && !m_tState.m_bProcessingHits && m_tHits.Length() )
	{
		CSphWordHit * pTail = const_cast < CSphWordHit * > ( m_tHits.Last() );

		if ( m_pFieldLengthAttrs )
			m_pFieldLengthAttrs [ HITMAN::GetField ( pTail->m_uWordPos ) ] = HITMAN::GetPos ( pTail->m_uWordPos );

		Hitpos_t uEndPos = pTail->m_uWordPos;
		if ( iBlendedHitsStart>=0 )
		{
			assert ( iBlendedHitsStart>=0 && iBlendedHitsStart<m_tHits.Length() );
			Hitpos_t uBlendedPos = ( m_tHits.First() + iBlendedHitsStart )->m_uWordPos;
			uEndPos = Min ( uEndPos, uBlendedPos );
		}

		// set end marker for all tail hits
		const CSphWordHit * pStart = m_tHits.First();
		while ( pStart<=pTail && uEndPos<=pTail->m_uWordPos )
		{
			HITMAN::SetEndMarker ( &pTail->m_uWordPos );
			pTail--;
		}
	}
}


void CSphSource_Document::BuildHits ( CSphString & sError, bool bSkipEndMarker )
{
	RowID_t tRowID = m_tDocInfo.m_tRowID;

	for ( ; m_tState.m_iField<m_tState.m_iEndField; m_tState.m_iField++ )
	{
		if ( !m_tState.m_bProcessingHits )
		{
			// get that field
			BYTE * sField = m_tState.m_dFields[m_tState.m_iField-m_tState.m_iStartField];
			int iFieldBytes = m_tState.m_dFieldLengths[m_tState.m_iField-m_tState.m_iStartField];
			if ( !sField || !(*sField) || !iFieldBytes )
				continue;

			// load files
			const BYTE * sTextToIndex;
			if ( m_tSchema.GetField(m_tState.m_iField).m_bFilename )
			{
				LoadFileField ( &sField, sError );
				sTextToIndex = sField;
				iFieldBytes = (int) strlen ( (char*)sField );
				if ( m_pFieldFilter && iFieldBytes )
				{
					m_tState.m_dFiltered.Resize ( 0 );
					int iFiltered = m_pFieldFilter->Apply ( sTextToIndex, iFieldBytes, m_tState.m_dFiltered, false );
					if ( iFiltered )
					{
						sTextToIndex = m_tState.m_dFiltered.Begin();
						iFieldBytes = iFiltered;
					}
				}
			} else
				sTextToIndex = sField;

			if ( iFieldBytes<=0 )
				continue;

			// strip html
			if ( m_pStripper )
			{
				m_pStripper->Strip ( (BYTE*)sTextToIndex );
				iFieldBytes = (int) strlen ( (char*)sTextToIndex );
			}

			// tokenize and build hits
			m_tStats.m_iTotalBytes += iFieldBytes;

			m_pTokenizer->BeginField ( m_tState.m_iField );
			m_pTokenizer->SetBuffer ( (BYTE*)sTextToIndex, iFieldBytes );

			m_tState.m_iHitPos = HITMAN::Create ( m_tState.m_iField, m_tState.m_iStartPos );
		}

		const CSphColumnInfo & tField = m_tSchema.GetField ( m_tState.m_iField );

		if ( tField.m_eWordpart!=SPH_WORDPART_WHOLE )
			BuildSubstringHits ( tRowID, tField.m_bPayload, tField.m_eWordpart, bSkipEndMarker );
		else
			BuildRegularHits ( tRowID, tField.m_bPayload, bSkipEndMarker );

		if ( m_tState.m_bProcessingHits )
			break;
	}

	m_tState.m_bDocumentDone = !m_tState.m_bProcessingHits;
}

//////////////////////////////////////////////////////////////////////////

CSphVector<int64_t> * CSphSource_Document::GetFieldMVA ( int iAttr )
{
	return &m_dMvas[iAttr];
}


const CSphString & CSphSource_Document::GetStrAttr ( int iAttr )
{
	return m_dStrAttrs[iAttr];
}


void CSphSource_Document::ParseFieldMVA ( int iAttr, const char * szValue )
{
	if ( !szValue )
		return;

	const char * pPtr = szValue;
	const char * pDigit = NULL;
	const int MAX_NUMBER_LEN = 64;
	char szBuf [MAX_NUMBER_LEN];

	while ( *pPtr )
	{
		if ( ( *pPtr>='0' && *pPtr<='9' ) || *pPtr=='-' )
		{
			if ( !pDigit )
				pDigit = pPtr;
		} else
		{
			if ( pDigit )
			{
				if ( pPtr - pDigit < MAX_NUMBER_LEN )
				{
					strncpy ( szBuf, pDigit, pPtr - pDigit );
					szBuf [pPtr - pDigit] = '\0';
					m_dMvas[iAttr].Add ( sphToInt64 ( szBuf ) );
				}

				pDigit = NULL;
			}
		}

		pPtr++;
	}

	if ( pDigit )
		m_dMvas[iAttr].Add ( sphToInt64 ( pDigit ) );
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SQL SOURCE
/////////////////////////////////////////////////////////////////////////////
const char * const CSphSource_SQL::MACRO_VALUES [ CSphSource_SQL::MACRO_COUNT ] =
{
	"$start",
	"$end"
};


CSphSource_SQL::CSphSource_SQL ( const char * sName )
	: CSphSource_Document	( sName )
{
}


bool CSphSource_SQL::Setup ( const CSphSourceParams_SQL & tParams )
{
	// checks
	assert ( !tParams.m_sQuery.IsEmpty() );

	m_tParams = tParams;

	// defaults
	#define LOC_FIX_NULL(_arg) if ( !m_tParams._arg.cstr() ) m_tParams._arg = "";
	LOC_FIX_NULL ( m_sHost );
	LOC_FIX_NULL ( m_sUser );
	LOC_FIX_NULL ( m_sPass );
	LOC_FIX_NULL ( m_sDB );
	#undef LOC_FIX_NULL

	#define LOC_FIX_QARRAY(_arg) \
		ARRAY_FOREACH ( i, m_tParams._arg ) \
			if ( m_tParams._arg[i].IsEmpty() ) \
				m_tParams._arg.Remove ( i-- );
	LOC_FIX_QARRAY ( m_dQueryPre );
	LOC_FIX_QARRAY ( m_dQueryPost );
	LOC_FIX_QARRAY ( m_dQueryPostIndex );
	#undef LOC_FIX_QARRAY

	// build and store default DSN for error reporting
	m_sSqlDSN.SetSprintf ( "sql://%s:***@%s:%d/%s",
		m_tParams.m_sUser.cstr(), m_tParams.m_sHost.cstr(),
		m_tParams.m_uPort, m_tParams.m_sDB.cstr() );

	if ( m_tParams.m_iMaxFileBufferSize > 0 )
		m_iMaxFileBufferSize = m_tParams.m_iMaxFileBufferSize;
	m_eOnFileFieldError = m_tParams.m_eOnFileFieldError;

	return true;
}

const char * SubstituteParams ( const char * sQuery, const char * const * dMacroses, const char ** dValues, int iMcount )
{
	// OPTIMIZE? things can be precalculated
	const char * sCur = sQuery;
	int iLen = 0;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<iMcount; i++ )
				if ( strncmp ( dMacroses[i], sCur, strlen ( dMacroses[i] ) )==0 )
				{
					sCur += strlen ( dMacroses[i] );
					iLen += strlen ( dValues[i] );
					break;
				}
			if ( i<iMcount )
				continue;
		}

		sCur++;
		iLen++;
	}
	iLen++; // trailing zero

	// do interpolation
	auto * sRes = new char [ iLen ];
	sCur = sQuery;

	char * sDst = sRes;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<iMcount; i++ )
				if ( strncmp ( dMacroses[i], sCur, strlen ( dMacroses[i] ) )==0 )
				{
					strcpy ( sDst, dValues[i] ); // NOLINT
					sCur += strlen ( dMacroses[i] );
					sDst += strlen ( dValues[i] );
					break;
				}
			if ( i<iMcount )
				continue;
		}
		*sDst++ = *sCur++;
	}
	*sDst++ = '\0';
	assert ( sDst-sRes==iLen );
	return sRes;
}


bool CSphSource_SQL::RunQueryStep ( const char * sQuery, CSphString & sError )
{
	sError = "";

	if ( m_tParams.m_iRangeStep<=0 )
		return false;
	if ( m_tCurrentID>m_tMaxID )
		return false;

	static const int iBufSize = 32;
	const char * sRes = nullptr;

	sphSleepMsec ( m_tParams.m_iRangedThrottle );

	//////////////////////////////////////////////
	// range query with $start/$end interpolation
	//////////////////////////////////////////////

	assert ( m_tMinID>0 );
	assert ( m_tMaxID>0 );
	assert ( m_tMinID<=m_tMaxID );
	assert ( sQuery );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	const char * pValues [ MACRO_COUNT ];
	DocID_t tNextID = Min ( m_tCurrentID + (DocID_t)m_tParams.m_iRangeStep - 1, m_tMaxID );
	snprintf ( sValues[0], iBufSize, INT64_FMT, m_tCurrentID );
	snprintf ( sValues[1], iBufSize, INT64_FMT, tNextID );
	pValues[0] = sValues[0];
	pValues[1] = sValues[1];
	g_iIndexerCurrentRangeMin = m_tCurrentID;
	g_iIndexerCurrentRangeMax = tNextID;
	m_tCurrentID = 1 + tNextID;

	sRes = SubstituteParams ( sQuery, MACRO_VALUES, pValues, MACRO_COUNT );

	// run query
	SqlDismissResult ();
	bool bRes = SqlQuery ( sRes );

	if ( !bRes )
		sError.SetSprintf ( "sql_range_query: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );

	SafeDeleteArray ( sRes );
	return bRes;
}

static bool HookConnect ( const char* szCommand )
{
	FILE * pPipe = popen ( szCommand, "r" );
	if ( !pPipe )
		return false;
	pclose ( pPipe );
	return true;
}

inline static const char* skipspace ( const char* pBuf, const char* pBufEnd )
{
	assert ( pBuf );
	assert ( pBufEnd );

	while ( (pBuf<pBufEnd) && isspace ( *pBuf ) )
		++pBuf;
	return pBuf;
}

inline static const char* scannumber ( const char* pBuf, const char* pBufEnd, DocID_t * pRes )
{
	assert ( pBuf );
	assert ( pBufEnd );
	assert ( pRes );

	if ( pBuf<pBufEnd )
	{
		*pRes = 0;
		// FIXME! could check for overflow
		while ( isdigit ( *pBuf ) && pBuf<pBufEnd )
			(*pRes) = 10*(*pRes) + (int)( (*pBuf++)-'0' );
	}
	return pBuf;
}

static bool HookQueryRange ( const char* szCommand, DocID_t* pMin, DocID_t* pMax )
{
	FILE * pPipe = popen ( szCommand, "r" );
	if ( !pPipe )
		return false;

	const int MAX_BUF_SIZE = 1024;
	char dBuf [MAX_BUF_SIZE];
	auto iRead = (int)fread ( dBuf, 1, MAX_BUF_SIZE, pPipe );
	pclose ( pPipe );
	const char* pStart = dBuf;
	const char* pEnd = pStart + iRead;
	// leading whitespace and 1-st number
	pStart = skipspace ( pStart, pEnd );
	pStart = scannumber ( pStart, pEnd, pMin );
	// whitespace and 2-nd number
	pStart = skipspace ( pStart, pEnd );
	scannumber ( pStart, pEnd, pMax );
	return true;
}

static bool HookPostIndex ( const char* szCommand, DocID_t tLastIndexed )
{
	const char * sMacro = "$maxid";
	char sValue[32];
	const char* pValue = sValue;
	snprintf ( sValue, sizeof(sValue), INT64_FMT, tLastIndexed );

	const char * pCmd = SubstituteParams ( szCommand, &sMacro, &pValue, 1 );

	FILE * pPipe = popen ( pCmd, "r" );
	SafeDeleteArray ( pCmd );
	if ( !pPipe )
		return false;
	pclose ( pPipe );
	return true;
}

/// connect to SQL server
bool CSphSource_SQL::Connect ( CSphString & sError )
{
	// do not connect twice
	if ( m_bSqlConnected )
		return true;

	// try to connect
	if ( !SqlConnect() )
	{
		sError.SetSprintf ( "sql_connect: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	m_tHits.m_dData.Reserve ( m_iMaxHits );

	// all good
	m_bSqlConnected = true;
	if ( !m_tParams.m_sHookConnect.IsEmpty() && !HookConnect ( m_tParams.m_sHookConnect.cstr() ) )
	{
		sError.SetSprintf ( "hook_connect: runtime error %s when running external hook", strerrorm(errno) );
		return false;
	}
	return true;
}


#define LOC_ERROR(_msg,_arg)			{ sError.SetSprintf ( _msg, _arg ); return false; }
#define LOC_ERROR2(_msg,_arg,_arg2)		{ sError.SetSprintf ( _msg, _arg, _arg2 ); return false; }

/// setup them ranges (called both for document range-queries and MVA range-queries)
bool CSphSource_SQL::SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError, ERangesReason iReason )
{
	// check step
	if ( m_tParams.m_iRangeStep<=0 )
		LOC_ERROR ( "sql_range_step=" INT64_FMT ": must be non-zero positive", m_tParams.m_iRangeStep );

	if ( m_tParams.m_iRangeStep<128 )
		sphWarn ( "sql_range_step=" INT64_FMT ": too small; might hurt indexing performance!", m_tParams.m_iRangeStep );

	// check query for macros
	for ( int i=0; i<MACRO_COUNT; i++ )
		if ( !strstr ( sQuery, MACRO_VALUES[i] ) )
			LOC_ERROR2 ( "%s: macro '%s' not found in match fetch query", sPrefix, MACRO_VALUES[i] );

	// run query
	if ( !SqlQuery ( sRangeQuery ) )
	{
		sError.SetSprintf ( "%s: range-query failed: %s (DSN=%s)", sPrefix, SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	// fetch min/max
	int iCols = SqlNumFields ();
	if ( iCols!=2 )
		LOC_ERROR2 ( "%s: expected 2 columns (min_id/max_id), got %d", sPrefix, iCols );

	if ( !SqlFetchRow() )
	{
		sError.SetSprintf ( "%s: range-query fetch failed: %s (DSN=%s)", sPrefix, SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	if ( ( SqlColumn(0)==NULL || !SqlColumn(0)[0] ) && ( SqlColumn(1)==NULL || !SqlColumn(1)[0] ) )
	{
		// the source seems to be empty; workaround
		m_tMinID = 1;
		m_tMaxID = 1;

	} else
	{
		// get and check min/max id
		const char * sCol0 = SqlColumn(0);
		const char * sCol1 = SqlColumn(1);
		m_tMinID = sphToInt64 ( sCol0 );
		m_tMaxID = sphToInt64 ( sCol1 );
		if ( !sCol0 ) sCol0 = "(null)";
		if ( !sCol1 ) sCol1 = "(null)";

		if ( m_tMinID<=0 )
			LOC_ERROR ( "sql_query_range: min_id='%s': must be positive 32/64-bit unsigned integer", sCol0 );
		if ( m_tMaxID<=0 )
			LOC_ERROR ( "sql_query_range: max_id='%s': must be positive 32/64-bit unsigned integer", sCol1 );
		if ( m_tMinID>m_tMaxID )
			LOC_ERROR2 ( "sql_query_range: min_id='%s', max_id='%s': min_id must be less than max_id", sCol0, sCol1 );
	}

	SqlDismissResult ();

	if ( iReason==SRE_DOCS && ( !m_tParams.m_sHookQueryRange.IsEmpty() ) )
	{
		if ( !HookQueryRange ( m_tParams.m_sHookQueryRange.cstr(), &m_tMinID, &m_tMaxID ) )
			LOC_ERROR ( "hook_query_range: runtime error %s when running external hook", strerror(errno) );
		if ( m_tMinID<=0 )
			LOC_ERROR ( "hook_query_range: min_id=" INT64_FMT ": must be positive 32/64-bit unsigned integer", m_tMinID );
		if ( m_tMaxID<=0 )
			LOC_ERROR ( "hook_query_range: max_id=" INT64_FMT ": must be positive 32/64-bit unsigned integer", m_tMaxID );
		if ( m_tMinID>m_tMaxID )
			LOC_ERROR2 ( "hook_query_range: min_id=" INT64_FMT ", max_id=" INT64_FMT ": min_id must be less than max_id", m_tMinID, m_tMaxID );
	}

	return true;
}


/// issue main rows fetch query
bool CSphSource_SQL::IterateStart ( CSphString & sError )
{
	assert ( m_bSqlConnected );

	// run pre-queries
	ARRAY_FOREACH ( i, m_tParams.m_dQueryPre )
	{
		if ( !SqlQuery ( m_tParams.m_dQueryPre[i].cstr() ) )
		{
			sError.SetSprintf ( "sql_query_pre[%d]: %s (DSN=%s)", i, SqlError(), m_sSqlDSN.cstr() );
			SqlDisconnect ();
			return false;
		}

		SqlDismissResult ();
	}

	while (true)
	{
		m_tParams.m_iRangeStep = 0;

		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;
			// run range-query; setup ranges
			if ( !SetupRanges ( m_tParams.m_sQueryRange.cstr(), m_tParams.m_sQuery.cstr(), "sql_query_range: ", sError, SRE_DOCS ) )
				return false;

			// issue query
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( m_tParams.m_sQuery.cstr(), sError ) )
				return false;
		} else
		{
			// normal query; just issue
			if ( !SqlQuery ( m_tParams.m_sQuery.cstr() ) )
			{
				sError.SetSprintf ( "sql_query: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );
				return false;
			}
		}
		break;
	}

	// some post-query setup
	m_tSchema.Reset();

	for ( int i=0; i<SPH_MAX_FIELDS; i++ )
		m_dUnpack[i] = SPH_UNPACK_NONE;

	m_iSqlFields = SqlNumFields(); // for rowdump

	CSphVector<bool> dFound;
	dFound.Resize ( m_tParams.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, dFound )
		dFound[i] = false;

	const bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// map plain attrs from SQL
	for ( int i=0; i<m_iSqlFields; i++ )
	{
		const char * sName = SqlFieldName(i);
		if ( !sName )
			LOC_ERROR ( "column number %d has no name", i+1 );

		CSphColumnInfo tCol ( sName );
		ARRAY_FOREACH ( j, m_tParams.m_dAttrs )
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dAttrs[j].m_sName.cstr() ) )
			{
				const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[j];

				tCol.m_eAttrType = tAttr.m_eAttrType;
				assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );

				if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
					LOC_ERROR ( "multi-valued attribute '%s' of wrong source-type found in query; must be 'field'", tAttr.m_sName.cstr() );

				tCol = tAttr;
				dFound[j] = true;
				break;
			}

		if ( !i && tCol.m_eAttrType==SPH_ATTR_NONE )
		{
			// id column coming from sql may have another name
			tCol.m_sName = sphGetDocidName();
			tCol.m_eAttrType = SPH_ATTR_BIGINT;
		}

		ARRAY_FOREACH ( j, m_tParams.m_dFileFields )
		{
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dFileFields[j].cstr() ) )
				tCol.m_bFilename = true;
		}

		tCol.m_iIndex = i;
		tCol.m_eWordpart = GetWordpart ( tCol.m_sName.cstr(), bWordDict );

		if ( tCol.m_eAttrType==SPH_ATTR_NONE || tCol.m_bIndexed )
		{
			m_tSchema.AddField ( tCol );
			ARRAY_FOREACH ( k, m_tParams.m_dUnpack )
			{
				CSphUnpackInfo & tUnpack = m_tParams.m_dUnpack[k];
				if ( tUnpack.m_sName==tCol.m_sName )
				{
					if ( !m_bCanUnpack )
					{
						sError.SetSprintf ( "this source does not support column unpacking" );
						return false;
					}
					int iIndex = m_tSchema.GetFieldsCount() - 1;
					if ( iIndex < SPH_MAX_FIELDS )
					{
						m_dUnpack[iIndex] = tUnpack.m_eFormat;
						m_dUnpackBuffers[iIndex].Resize ( SPH_UNPACK_BUFFER_SIZE );
					}
					break;
				}
			}
		}

		if ( tCol.m_eAttrType!=SPH_ATTR_NONE )
		{
			if ( CSphSchema::IsReserved ( tCol.m_sName.cstr() ) )
				LOC_ERROR ( "%s is not a valid attribute name", tCol.m_sName.cstr() );

			m_tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
		}
	}

	// map multi-valued attrs
	ARRAY_FOREACH ( i, m_tParams.m_dAttrs )
	{
		const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[i];
		if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			CSphColumnInfo tMva;
			tMva = tAttr;
			tMva.m_iIndex = m_tSchema.GetAttrsCount();

			if ( CSphSchema::IsReserved ( tMva.m_sName.cstr() ) )
				LOC_ERROR ( "%s is not a valid attribute name", tMva.m_sName.cstr() );

			m_tSchema.AddAttr ( tMva, true ); // all attributes are dynamic at indexing time
			dFound[i] = true;
		}
	}

	// warn if some attrs went unmapped
	ARRAY_FOREACH ( i, dFound )
		if ( !dFound[i] )
			sphWarn ( "attribute '%s' not found - IGNORING", m_tParams.m_dAttrs[i].m_sName.cstr() );

	// joined fields
	m_iPlainFieldsLength = m_tSchema.GetFieldsCount();

	ARRAY_FOREACH ( i, m_tParams.m_dJoinedFields )
	{
		CSphColumnInfo tCol;
		tCol.m_iIndex = -1;
		tCol.m_sName = m_tParams.m_dJoinedFields[i].m_sName;
		tCol.m_sQuery = m_tParams.m_dJoinedFields[i].m_sQuery;
		tCol.m_bPayload = m_tParams.m_dJoinedFields[i].m_bPayload;
		tCol.m_sQueryRange = m_tParams.m_dJoinedFields[i].m_sRanged;
		tCol.m_eWordpart = GetWordpart ( tCol.m_sName.cstr(), bWordDict );

		tCol.m_eSrc = ( !m_tParams.m_dJoinedFields[i].m_bRangedMain ? SPH_ATTRSRC_QUERY : SPH_ATTRSRC_RANGEDMAINQUERY );
		if ( !m_tParams.m_dJoinedFields[i].m_sRanged.IsEmpty() )
			tCol.m_eSrc = SPH_ATTRSRC_RANGEDQUERY;

		m_tSchema.AddField ( tCol );
	}

	// auto-computed length attributes
	if ( !AddAutoAttrs ( sError ) )
		return false;

	// check it
	if ( m_tSchema.GetFieldsCount()>SPH_MAX_FIELDS )
		LOC_ERROR2 ( "too many fields (fields=%d, max=%d)", m_tSchema.GetFieldsCount(), SPH_MAX_FIELDS );

	// alloc storage
	AllocDocinfo();

	// log it
	if ( m_fpDumpRows )
	{
		const char * sTable = m_tSchema.GetName();

		time_t iNow = time ( NULL );
		fprintf ( m_fpDumpRows, "#\n# === source %s ts %d\n# %s#\n", sTable, (int)iNow, ctime ( &iNow ) );
		for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
			fprintf ( m_fpDumpRows, "# field %d: %s\n", i, m_tSchema.GetFieldName(i) );

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(i);
			fprintf ( m_fpDumpRows, "# %s = %s # attr %d\n", sphTypeDirective ( tCol.m_eAttrType ), tCol.m_sName.cstr(), i );
		}

		fprintf ( m_fpDumpRows, "#\n\nDROP TABLE IF EXISTS rows_%s;\nCREATE TABLE rows_%s (\n  id VARCHAR(32) NOT NULL,\n",
			sTable, sTable );
		for ( int i=1; i<m_iSqlFields; i++ )
			fprintf ( m_fpDumpRows, "  %s VARCHAR(4096) NOT NULL,\n", SqlFieldName(i) );
		fprintf ( m_fpDumpRows, "  KEY(id) );\n\n" );
	}

	return true;
}

#undef LOC_ERROR
#undef LOC_ERROR2
#undef LOC_SQL_ERROR


void CSphSource_SQL::Disconnect ()
{
	SafeDeleteArray ( m_pReadFileBuffer );
	m_tHits.m_dData.Reset();

	if ( m_bSqlConnected )
		SqlDisconnect ();

	m_bSqlConnected = false;
}


BYTE ** CSphSource_SQL::NextDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_bSqlConnected );

	// try to get next row
	bool bGotRow = SqlFetchRow ();

	bEOF = false;

	// when the party's over...
	while ( !bGotRow )
	{
		// is that an error?
		if ( SqlIsError() )
		{
			sError.SetSprintf ( "sql_fetch_row: %s", SqlError() );
			return nullptr;
		}

		// maybe we can do next step yet?
		if ( !RunQueryStep ( m_tParams.m_sQuery.cstr(), sError ) )
		{
			// if there's a message, there's an error
			// otherwise, we're just over
			if ( !sError.IsEmpty() )
				return nullptr;

		} else
		{
			// step went fine; try to fetch
			bGotRow = SqlFetchRow ();
			continue;
		}

		SqlDismissResult();

		// ok, we're over
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPost )
		{
			if ( !SqlQuery ( m_tParams.m_dQueryPost[i].cstr() ) )
			{
				sphWarn ( "sql_query_post[%d]: error=%s, query=%s",	i, SqlError(), m_tParams.m_dQueryPost[i].cstr() );
				break;
			}

			SqlDismissResult();
		}

		bEOF = true;
		return nullptr;
	}

	m_tMaxFetchedID = Max ( m_tMaxFetchedID, sphToInt64 ( SqlColumn(0) ) );

	// cleanup attrs
	for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
		m_tDocInfo.m_pDynamic[i] = 0;

	// split columns into fields and attrs
	for ( int i=0; i<m_iPlainFieldsLength; i++ )
	{
		// get that field
		#if USE_ZLIB
		if ( m_dUnpack[i]!=SPH_UNPACK_NONE )
		{
			DWORD uUnpackedLen = 0;
			m_dFields[i] = (BYTE*) SqlUnpackColumn ( i, uUnpackedLen, m_dUnpack[i] );
			m_dFieldLengths[i] = (int)uUnpackedLen;
			continue;
		}
		#endif
		m_dFields[i] = (BYTE*) SqlColumn ( m_tSchema.GetField(i).m_iIndex );
		m_dFieldLengths[i] = SqlColumnLength ( m_tSchema.GetField(i).m_iIndex );
	}

	m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
	for ( auto & i : m_dMvas )
		i.Resize(0);

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i); // shortcut

		switch ( tAttr.m_eAttrType )
		{
			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
				// memorize string, fixup NULLs
				m_dStrAttrs[i] = SqlColumn ( tAttr.m_iIndex );
				if ( !m_dStrAttrs[i].cstr() )
					m_dStrAttrs[i] = "";

				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				break;

			case SPH_ATTR_FLOAT:
				m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, sphToFloat ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
				break;

			case SPH_ATTR_BIGINT:
				if ( tAttr.m_iIndex<0 )
				{
					assert ( tAttr.m_sName==sphGetBlobLocatorName() );
				} else
				{
					CSphString sWarn;
					m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToInt64 ( SqlColumn ( tAttr.m_iIndex ), &sWarn ) );
					if ( !sWarn.IsEmpty() )
						sphWarn ( "%s", sWarn.cstr() );
				}
				break;

			case SPH_ATTR_TOKENCOUNT:
				// reset, and the value will be filled by IterateHits()
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				break;

			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
				if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
					ParseFieldMVA ( i, SqlColumn ( tAttr.m_iIndex ) );
				break;

			default:
				// just store as uint by default
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToDword ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
				break;
		}
	}

	// log it
	if ( m_fpDumpRows )
	{
		fprintf ( m_fpDumpRows, "INSERT INTO rows_%s VALUES (", m_tSchema.GetName() );
		for ( int i=0; i<m_iSqlFields; i++ )
		{
			if ( i )
				fprintf ( m_fpDumpRows, ", " );
			FormatEscaped ( m_fpDumpRows, SqlColumn(i) );
		}
		fprintf ( m_fpDumpRows, ");\n" );
	}

	return m_dFields;
}


const int * CSphSource_SQL::GetFieldLengths() const
{
	return m_dFieldLengths;
}


void CSphSource_SQL::PostIndex ()
{
	if ( ( !m_tParams.m_dQueryPostIndex.GetLength() ) && m_tParams.m_sHookPostIndex.IsEmpty() )
		return;

	assert ( !m_bSqlConnected );

	const char * sSqlError = NULL;
	if ( m_tParams.m_dQueryPostIndex.GetLength() )
	{
#define LOC_SQL_ERROR(_msg) { sSqlError = _msg; break; }

		while (true)
		{
			if ( !SqlConnect () )
				LOC_SQL_ERROR ( "mysql_real_connect" );

			ARRAY_FOREACH ( i, m_tParams.m_dQueryPostIndex )
			{
				char * sQuery = sphStrMacro ( m_tParams.m_dQueryPostIndex[i].cstr(), "$maxid", m_tMaxFetchedID );
				bool bRes = SqlQuery ( sQuery );
				delete [] sQuery;

				if ( !bRes )
					LOC_SQL_ERROR ( "sql_query_post_index" );

				SqlDismissResult ();
			}

			break;
		}

		if ( sSqlError )
			sphWarn ( "%s: %s (DSN=%s)", sSqlError, SqlError(), m_sSqlDSN.cstr() );

#undef LOC_SQL_ERROR

		SqlDisconnect ();
	}
	if ( !m_tParams.m_sHookPostIndex.IsEmpty() && !HookPostIndex ( m_tParams.m_sHookPostIndex.cstr(), m_tMaxFetchedID ) )
	{
		sphWarn ( "hook_post_index: runtime error %s when running external hook", strerrorm(errno) );
	}
}


bool CSphSource_SQL::IterateMultivaluedStart ( int iAttr, CSphString & sError )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	m_iMultiAttr = iAttr;
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);

	if ( !(tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) )
		return false;

	CSphString sPrefix;
	switch ( tAttr.m_eSrc )
	{
	case SPH_ATTRSRC_FIELD:
		return false;

	case SPH_ATTRSRC_QUERY:
		// run simple query
		if ( !SqlQuery ( tAttr.m_sQuery.cstr() ) )
		{
			sError.SetSprintf ( "multi-valued attr '%s' query failed: %s", tAttr.m_sName.cstr(), SqlError() );
			return false;
		}
		break;

	case SPH_ATTRSRC_RANGEDQUERY:
			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

			// setup ranges
			sPrefix.SetSprintf ( "multi-valued attr '%s' ranged query: ", tAttr.m_sName.cstr() );
			if ( !SetupRanges ( tAttr.m_sQueryRange.cstr(), tAttr.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_MVA ) )
				return false;

			// run first step (in order to report errors)
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sError ) )
				return false;

			break;

	case SPH_ATTRSRC_RANGEDMAINQUERY:
			if ( m_tParams.m_sQueryRange.IsEmpty() )
			{
				sError.SetSprintf ( "multi-valued attr '%s': empty main range query", tAttr.m_sName.cstr() );
				return false;
			}

			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

			// setup ranges
			sPrefix.SetSprintf ( "multi-valued attr '%s' ranged query: ", tAttr.m_sName.cstr() );
			if ( !SetupRanges ( m_tParams.m_sQueryRange.cstr(), tAttr.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_MVA ) )
				return false;

			// run first step (in order to report errors)
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sError ) )
				return false;

			break;

	default:
		sError.SetSprintf ( "INTERNAL ERROR: unknown multi-valued attr source type %d", tAttr.m_eSrc );
		return false;
	}

	// check fields count
	if ( SqlNumFields()!=2 )
	{
		sError.SetSprintf ( "multi-valued attr '%s' query returned %d fields (expected 2)", tAttr.m_sName.cstr(), SqlNumFields() );
		SqlDismissResult ();
		return false;
	}
	return true;
}


bool CSphSource_SQL::IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue )
{
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iMultiAttr );

	assert ( m_bSqlConnected );
	assert ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET );

	// fetch next row
	bool bGotRow = SqlFetchRow ();
	while ( !bGotRow )
	{
		if ( SqlIsError() )
			sphDie ( "sql_fetch_row: %s", SqlError() ); // FIXME! this should be reported

		if ( tAttr.m_eSrc!=SPH_ATTRSRC_RANGEDQUERY &&  tAttr.m_eSrc!=SPH_ATTRSRC_RANGEDMAINQUERY )
		{
			SqlDismissResult();
			return false;
		}

		CSphString sTmp;
		if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sTmp ) ) // FIXME! this should be reported
			return false;

		bGotRow = SqlFetchRow ();
		continue;
	}

	// return that tuple or offset to storage for MVA64 value
	iDocID = sphToInt64 ( SqlColumn(0) );
	iMvaValue = sphToInt64 ( SqlColumn(1) );

	return true;
}


bool CSphSource_SQL::IterateKillListStart ( CSphString & sError )
{
	if ( m_tParams.m_sQueryKilllist.IsEmpty () )
		return false;

	if ( !SqlQuery ( m_tParams.m_sQueryKilllist.cstr () ) )
	{
		sError.SetSprintf ( "killlist query failed: %s", SqlError() );
		return false;
	}

	return true;
}


bool CSphSource_SQL::IterateKillListNext ( DocID_t & tDocID )
{
	if ( SqlFetchRow () )
		tDocID = sphToInt64 ( SqlColumn(0) );
	else
	{
		if ( SqlIsError() )
			sphDie ( "sql_query_killlist: %s", SqlError() ); // FIXME! this should be reported
		else
		{
			SqlDismissResult ();
			return false;
		}
	}

	return true;
}


void CSphSource_SQL::ReportUnpackError ( int iIndex, int iError )
{
	if ( !m_bUnpackFailed )
	{
		m_bUnpackFailed = true;
		sphWarn ( "failed to unpack column '%s', error=%d, rowid=%u", SqlFieldName(iIndex), iError, m_tDocInfo.m_tRowID );
	}
}


#if !USE_ZLIB

const char * CSphSource_SQL::SqlUnpackColumn ( int iFieldIndex, DWORD & uUnpackedLen, ESphUnpackFormat )
{
	int iIndex = m_tSchema.GetField(iFieldIndex).m_iIndex;
	uUnpackedLen = SqlColumnLength(iIndex);
	return SqlColumn(iIndex);
}

#else

const char * CSphSource_SQL::SqlUnpackColumn ( int iFieldIndex, DWORD & uUnpackedLen, ESphUnpackFormat eFormat )
{
	int iIndex = m_tSchema.GetField(iFieldIndex).m_iIndex;
	const char * pData = SqlColumn(iIndex);

	if ( pData==NULL )
		return NULL;

	int iPackedLen = SqlColumnLength(iIndex);
	if ( iPackedLen<=0 )
		return NULL;


	CSphVector<char> & tBuffer = m_dUnpackBuffers[iFieldIndex];
	switch ( eFormat )
	{
		case SPH_UNPACK_MYSQL_COMPRESS:
		{
			if ( iPackedLen<=4 )
			{
				if ( !m_bUnpackFailed )
				{
					m_bUnpackFailed = true;
					sphWarn ( "failed to unpack '%s', invalid column size (size=%d), rowid=%u", SqlFieldName(iIndex), iPackedLen, m_tDocInfo.m_tRowID );
				}
				return NULL;
			}

			unsigned long uSize = 0;
			for ( int i=0; i<4; i++ )
				uSize += ((unsigned long)((BYTE)pData[i])) << ( 8*i );
			uSize &= 0x3FFFFFFF;

			if ( uSize > m_tParams.m_uUnpackMemoryLimit )
			{
				if ( !m_bUnpackOverflow )
				{
					m_bUnpackOverflow = true;
					sphWarn ( "failed to unpack '%s', column size limit exceeded (size=%d), rowid=%u", SqlFieldName(iIndex), (int)uSize, m_tDocInfo.m_tRowID );
				}
				return NULL;
			}

			int iResult;
			tBuffer.Resize ( uSize + 1 );
			unsigned long uLen = iPackedLen-4;
			iResult = uncompress ( (Bytef *)tBuffer.Begin(), &uSize, (Bytef *)pData + 4, uLen );
			if ( iResult==Z_OK )
			{
				uUnpackedLen = uSize;
				tBuffer[uSize] = 0;
				return &tBuffer[0];
			} else
				ReportUnpackError ( iIndex, iResult );
			return NULL;
		}

		case SPH_UNPACK_ZLIB:
		{
			char * sResult = 0;
			int iBufferOffset = 0;
			int iResult;

			z_stream tStream;
			tStream.zalloc = Z_NULL;
			tStream.zfree = Z_NULL;
			tStream.opaque = Z_NULL;
			tStream.avail_in = iPackedLen;
			tStream.next_in = (Bytef *)SqlColumn(iIndex);

			iResult = inflateInit ( &tStream );
			if ( iResult!=Z_OK )
				return NULL;

			while (true)
			{
				tStream.next_out = (Bytef *)&tBuffer[iBufferOffset];
				tStream.avail_out = tBuffer.GetLength() - iBufferOffset - 1;

				iResult = inflate ( &tStream, Z_NO_FLUSH );
				if ( iResult==Z_STREAM_END )
				{
					tBuffer [ tStream.total_out ] = 0;
					uUnpackedLen = tStream.total_out;
					sResult = &tBuffer[0];
					break;
				} else if ( iResult==Z_OK )
				{
					assert ( tStream.avail_out==0 );

					tBuffer.Resize ( tBuffer.GetLength()*2 );
					iBufferOffset = tStream.total_out;
				} else
				{
					ReportUnpackError ( iIndex, iResult );
					break;
				}
			}

			inflateEnd ( &tStream );
			return sResult;
		}

		case SPH_UNPACK_NONE:
			return pData;
	}
	return NULL;
}
#endif // USE_ZLIB


struct CmpPairs_fn
{
	bool IsLess ( const IDPair_t & tA, const IDPair_t & tB ) const
	{
		if ( tA.m_tDocID < tB.m_tDocID )
			return true;
		else if ( tA.m_tDocID > tB.m_tDocID )
			return false;

		return tA.m_tRowID < tB.m_tRowID;
	}

	bool IsEq ( const IDPair_t & tA, const IDPair_t & tB ) const
	{
		return tA.m_tDocID==tB.m_tDocID;
	}
};


ISphHits * CSphSource_SQL::IterateJoinedHits ( CSphString & sError )
{
	// iterating of joined hits happens after iterating hits from main query
	// so we may be sure at this moment no new IDs will be put in m_dAllIds
	if ( !m_bIdsSorted )
	{
		// sorted by docids, but we may have duplicates
		m_dAllIds.Sort ( CmpPairs_fn() );
		IDPair_t * pStart = m_dAllIds.Begin();
		int iLeft = sphUniq ( pStart, m_dAllIds.GetLength(), CmpPairs_fn() );
		m_dAllIds.Resize(iLeft);
		m_bIdsSorted = true;
	}

	m_tHits.m_dData.Resize(0);

	// eof check
	if ( m_iJoinedHitField>=m_tSchema.GetFieldsCount() )
	{
		m_tDocInfo.m_tRowID = INVALID_ROWID;
		return &m_tHits;
	}

	bool bProcessingRanged = true;

	// my fetch loop
	while ( m_iJoinedHitField<m_tSchema.GetFieldsCount() )
	{
		if ( m_tState.m_bProcessingHits || SqlFetchRow() )
		{
			// next row
			DocID_t tDocId = sphToInt64 ( SqlColumn(0) ); // FIXME! handle conversion errors and zero/max values?

			// lets skip joined document totally if there was no such document ID returned by main query
			const IDPair_t * pIdPair = m_dAllIds.BinarySearch ( bind ( &IDPair_t::m_tDocID ), tDocId );
			if ( !pIdPair )
				continue;

			// field start? restart ids
			if ( !m_iJoinedHitID )
				m_iJoinedHitID = tDocId;

			// docid asc requirement violated? report an error
			if ( m_iJoinedHitID>tDocId )
			{
				sError.SetSprintf ( "joined field '%s': query MUST return document IDs in ASC order", m_tSchema.GetFieldName(m_iJoinedHitField) );
				return NULL;
			}

			// next document? update tracker, reset position
			if ( m_iJoinedHitID<tDocId )
			{
				m_iJoinedHitID = tDocId;
				m_iJoinedHitPos = 0;
			}

			if ( !m_tState.m_bProcessingHits )
			{
				m_tState = CSphBuildHitsState_t();
				m_tState.m_iField = m_iJoinedHitField;
				m_tState.m_iStartField = m_iJoinedHitField;
				m_tState.m_iEndField = m_iJoinedHitField+1;

				if ( m_tSchema.GetField(m_iJoinedHitField).m_bPayload )
					m_tState.m_iStartPos = sphToDword ( SqlColumn(2) );
				else
					m_tState.m_iStartPos = m_iJoinedHitPos;
			}

			// build those hits
			BYTE * dText[] = { (BYTE *)SqlColumn(1) };
			m_tState.m_dFields = dText;
			m_tState.m_dFieldLengths.Resize(1);
			m_tState.m_dFieldLengths[0] = SqlColumnLength(1);

			m_tDocInfo.m_tRowID = pIdPair->m_tRowID;
			BuildHits ( sError, true );

			// update current position
			if ( !m_tSchema.GetField(m_iJoinedHitField).m_bPayload && !m_tState.m_bProcessingHits && m_tHits.Length() )
				m_iJoinedHitPos = HITMAN::GetPos ( m_tHits.Last()->m_uWordPos );

			if ( m_tState.m_bProcessingHits )
				break;
		} else if ( SqlIsError() )
		{
			// error while fetching row
			sError = SqlError();
			return NULL;

		} else
		{
			int iLastField = m_iJoinedHitField;
			bool bRanged = ( m_iJoinedHitField>=m_iPlainFieldsLength && m_iJoinedHitField<m_tSchema.GetFieldsCount()
				&& ( m_tSchema.GetField(m_iJoinedHitField).m_eSrc==SPH_ATTRSRC_RANGEDQUERY || m_tSchema.GetField(m_iJoinedHitField).m_eSrc==SPH_ATTRSRC_RANGEDMAINQUERY ) );

			// current field is over, continue to next field
			if ( m_iJoinedHitField<0 )
				m_iJoinedHitField = m_iPlainFieldsLength;
			else if ( !bRanged || !bProcessingRanged )
				m_iJoinedHitField++;

			// eof check
			if ( m_iJoinedHitField>=m_tSchema.GetFieldsCount() )
			{
				m_tDocInfo.m_tRowID = ( m_tHits.Length() ? 0 : INVALID_ROWID ); // to eof or not to eof
				SqlDismissResult ();
				return &m_tHits;
			}

			SqlDismissResult ();

			bProcessingRanged = false;
			bool bCheckNumFields = true;
			const CSphColumnInfo & tJoined = m_tSchema.GetField(m_iJoinedHitField);

			bool bJoinedRanged = ( tJoined.m_eSrc==SPH_ATTRSRC_RANGEDQUERY || tJoined.m_eSrc==SPH_ATTRSRC_RANGEDMAINQUERY );
			// start fetching next field
			if ( !bJoinedRanged )
			{
				if ( !SqlQuery ( tJoined.m_sQuery.cstr() ) )
				{
					sError = SqlError();
					return NULL;
				}
			} else
			{
				m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

				// setup ranges for next field
				if ( iLastField!=m_iJoinedHitField )
				{
					const CSphString & sRange = ( tJoined.m_eSrc==SPH_ATTRSRC_RANGEDQUERY ? tJoined.m_sQueryRange : m_tParams.m_sQueryRange );

					CSphString sPrefix;
					sPrefix.SetSprintf ( "joined field '%s' ranged query: ", tJoined.m_sName.cstr() );
					if ( !SetupRanges ( sRange.cstr(), tJoined.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_JOINEDHITS ) )
						return NULL;

					m_tCurrentID = m_tMinID;
				}

				// run first step (in order to report errors)
				bool bRes = RunQueryStep ( tJoined.m_sQuery.cstr(), sError );
				bProcessingRanged = bRes; // select next documents in range or loop once to process next field
				bCheckNumFields = bRes;

				if ( !sError.IsEmpty() )
					return NULL;
			}

			const int iExpected = m_tSchema.GetField(m_iJoinedHitField).m_bPayload ? 3 : 2;
			if ( bCheckNumFields && SqlNumFields()!=iExpected )
			{
				const char * sName = m_tSchema.GetField(m_iJoinedHitField).m_sName.cstr();
				sError.SetSprintf ( "joined field '%s': query MUST return exactly %d columns, got %d", sName, iExpected, SqlNumFields() );
				return NULL;
			}

			m_iJoinedHitID = 0;
			m_iJoinedHitPos = 0;
		}
	}

	return &m_tHits;
}

// a little staff for using static/dynamic libraries.
// for dynamic we declare the type and define the originally nullptr pointer.
// for static we define const pointer as alias to target function.

#define F_DL(name) static decltype(&name) sph_##name = nullptr
#define F_DR(name) static decltype(&name) sph_##name = &name

#if DL_MYSQL
	#define MYSQL_F(name) F_DL(name)
#else
	#define MYSQL_F(name) F_DR(name)
#endif

#if DL_EXPAT
	#define EXPAT_F(name) F_DL(name)
#else
	#define EXPAT_F(name) F_DR(name)
#endif

#if DL_PGSQL
	#define PGSQL_F(name) F_DL(name)
#else
	#define PGSQL_F(name) F_DR(name)
#endif


#if DL_UNIXODBC
	#define ODBC_F(name) F_DL(name)
#else
	#define ODBC_F( name ) F_DR(name)
#endif


/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////
#if USE_MYSQL

	MYSQL_F ( mysql_free_result );
	MYSQL_F ( mysql_next_result );
	MYSQL_F ( mysql_use_result );
	MYSQL_F ( mysql_num_rows );
	MYSQL_F ( mysql_query );
	MYSQL_F ( mysql_errno );
	MYSQL_F ( mysql_error );
	MYSQL_F ( mysql_init );
	MYSQL_F ( mysql_ssl_set );
	MYSQL_F ( mysql_real_connect );
	MYSQL_F ( mysql_close );
	MYSQL_F ( mysql_num_fields );
	MYSQL_F ( mysql_fetch_row );
	MYSQL_F ( mysql_fetch_fields );
	MYSQL_F ( mysql_fetch_lengths );

	#ifndef MYSQL_LIB
		#define MYSQL_LIB "no_mysql"
	#endif

	#if DL_MYSQL

		bool InitDynamicMysql()
		{
			const char * sFuncs[] = { "mysql_free_result", "mysql_next_result", "mysql_use_result"
				, "mysql_num_rows", "mysql_query", "mysql_errno", "mysql_error"
				, "mysql_init", "mysql_ssl_set", "mysql_real_connect", "mysql_close"
				, "mysql_num_fields", "mysql_fetch_row", "mysql_fetch_fields"
				, "mysql_fetch_lengths" };

			void ** pFuncs[] = { (void **) &sph_mysql_free_result, (void **) &sph_mysql_next_result
				, (void **) &sph_mysql_use_result, (void **) &sph_mysql_num_rows, (void **) &sph_mysql_query
				, (void **) &sph_mysql_errno, (void **) &sph_mysql_error, (void **) &sph_mysql_init
				, (void **) &sph_mysql_ssl_set, (void **) &sph_mysql_real_connect, (void **) &sph_mysql_close
				, (void **) &sph_mysql_num_fields, (void **) &sph_mysql_fetch_row
				, (void **) &sph_mysql_fetch_fields, (void **) &sph_mysql_fetch_lengths };

			static CSphDynamicLibrary dLib ( MYSQL_LIB );
			if ( !dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) ) )
				return false;
			return true;
		}
	#else
		#define InitDynamicMysql() (true)
	#endif

CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
	: m_iFlags ( 0 )
{
	m_uPort = 3306;
}


CSphSource_MySQL::CSphSource_MySQL ( const char * sName )
	: CSphSource_SQL	( sName )
{
	m_bCanUnpack = true;
	memset ( &m_tMysqlDriver, 0, sizeof ( m_tMysqlDriver ) );
}


void CSphSource_MySQL::SqlDismissResult ()
{
	if ( !m_pMysqlResult )
		return;

	while ( m_pMysqlResult )
	{
		sph_mysql_free_result ( m_pMysqlResult );
		m_pMysqlResult = NULL;

		// stored procedures might return multiple result sets
		// FIXME? we might want to index all of them
		// but for now, let's simply dismiss additional result sets
		if ( sph_mysql_next_result ( &m_tMysqlDriver )==0 )
		{
			m_pMysqlResult = sph_mysql_use_result ( &m_tMysqlDriver );

			static bool bOnce = false;
			if ( !bOnce && m_pMysqlResult && sph_mysql_num_rows ( m_pMysqlResult ) )
			{
				sphWarn ( "indexing of multiple result sets is not supported yet; some results sets were dismissed!" );
				bOnce = true;
			}
		}
	}

	m_pMysqlFields = nullptr;
	m_pMysqlLengths = nullptr;
}


bool CSphSource_MySQL::SqlQuery ( const char * sQuery )
{
	if ( sph_mysql_query ( &m_tMysqlDriver, sQuery ) )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL\n", sQuery );
		return false;
	}
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-QUERY: %s: ok\n", sQuery );

	m_pMysqlResult = sph_mysql_use_result ( &m_tMysqlDriver );
	m_pMysqlFields = nullptr;
	return true;
}


bool CSphSource_MySQL::SqlIsError ()
{
	return sph_mysql_errno ( &m_tMysqlDriver )!=0;
}


const char * CSphSource_MySQL::SqlError ()
{
	if ( sph_mysql_error!=nullptr )
		return sph_mysql_error ( &m_tMysqlDriver );
	return "MySQL source wasn't initialized. Wrong name in dlopen?";
}


bool CSphSource_MySQL::SqlConnect ()
{
	if_const ( !InitDynamicMysql() )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL (NO MYSQL CLIENT LIB, tried " MYSQL_LIB ")\n" );
		return false;
	}

	sph_mysql_init ( &m_tMysqlDriver );
	if ( !m_sSslKey.IsEmpty() || !m_sSslCert.IsEmpty() || !m_sSslCA.IsEmpty() )
		sph_mysql_ssl_set ( &m_tMysqlDriver, m_sSslKey.cstr(), m_sSslCert.cstr(), m_sSslCA.cstr(), NULL, NULL );

	m_iMysqlConnectFlags |= CLIENT_MULTI_RESULTS; // we now know how to handle this
	bool bRes = ( nullptr!=sph_mysql_real_connect ( &m_tMysqlDriver,
		m_tParams.m_sHost.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr(),
		m_tParams.m_sDB.cstr(), m_tParams.m_uPort, m_sMysqlUsock.cstr(), m_iMysqlConnectFlags ) );
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, bRes ? "SQL-CONNECT: ok\n" : "SQL-CONNECT: FAIL\n" );
	return bRes;
}


void CSphSource_MySQL::SqlDisconnect ()
{
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-DISCONNECT\n" );

	sph_mysql_close ( &m_tMysqlDriver );
}


int CSphSource_MySQL::SqlNumFields ()
{
	if ( !m_pMysqlResult )
		return -1;

	return sph_mysql_num_fields ( m_pMysqlResult );
}


bool CSphSource_MySQL::SqlFetchRow ()
{
	if ( !m_pMysqlResult )
		return false;

	m_tMysqlRow = sph_mysql_fetch_row ( m_pMysqlResult );
	return m_tMysqlRow!=NULL;
}


const char * CSphSource_MySQL::SqlColumn ( int iIndex )
{
	if ( !m_pMysqlResult )
		return NULL;

	return m_tMysqlRow[iIndex];
}


const char * CSphSource_MySQL::SqlFieldName ( int iIndex )
{
	if ( !m_pMysqlResult )
		return NULL;

	if ( !m_pMysqlFields )
		m_pMysqlFields = sph_mysql_fetch_fields ( m_pMysqlResult );

	return m_pMysqlFields[iIndex].name;
}


DWORD CSphSource_MySQL::SqlColumnLength ( int iIndex )
{
	if ( !m_pMysqlResult )
		return 0;

	if ( !m_pMysqlLengths )
		m_pMysqlLengths = sph_mysql_fetch_lengths ( m_pMysqlResult );

	return m_pMysqlLengths[iIndex];
}


bool CSphSource_MySQL::Setup ( const CSphSourceParams_MySQL & tParams )
{
	if ( !CSphSource_SQL::Setup ( tParams ) )
		return false;

	m_sMysqlUsock = tParams.m_sUsock;
	m_iMysqlConnectFlags = tParams.m_iFlags;
	m_sSslKey = tParams.m_sSslKey;
	m_sSslCert = tParams.m_sSslCert;
	m_sSslCA = tParams.m_sSslCA;

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "mysql%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}

#endif // USE_MYSQL

/////////////////////////////////////////////////////////////////////////////
// PGSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_PGSQL

	PGSQL_F ( PQgetvalue );
	PGSQL_F ( PQgetlength );
	PGSQL_F ( PQclear );
	PGSQL_F ( PQsetdbLogin );
	PGSQL_F ( PQstatus );
	PGSQL_F ( PQsetClientEncoding );
	PGSQL_F ( PQexec );
	PGSQL_F ( PQresultStatus );
	PGSQL_F ( PQntuples );
	PGSQL_F ( PQfname );
	PGSQL_F ( PQnfields );
	PGSQL_F ( PQfinish );
	PGSQL_F ( PQerrorMessage );

	#ifndef PGSQL_LIB
			#define PGSQL_LIB "no_pgsql"
	#endif
	#if DL_PGSQL
		bool InitDynamicPosgresql ()
		{
			const char * sFuncs[] = {"PQgetvalue", "PQgetlength", "PQclear",
					"PQsetdbLogin", "PQstatus", "PQsetClientEncoding", "PQexec",
					"PQresultStatus", "PQntuples", "PQfname", "PQnfields",
					"PQfinish", "PQerrorMessage" };

			void ** pFuncs[] = {(void**)&sph_PQgetvalue, (void**)&sph_PQgetlength, (void**)&sph_PQclear,
					(void**)&sph_PQsetdbLogin, (void**)&sph_PQstatus, (void**)&sph_PQsetClientEncoding,
					(void**)&sph_PQexec, (void**)&sph_PQresultStatus, (void**)&sph_PQntuples,
					(void**)&sph_PQfname, (void**)&sph_PQnfields, (void**)&sph_PQfinish,
					(void**)&sph_PQerrorMessage};

			static CSphDynamicLibrary dLib ( PGSQL_LIB );
			if ( !dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) ) )
				return false;
			return true;
		}

	#else
		#define InitDynamicPosgresql() (true)
	#endif

CSphSourceParams_PgSQL::CSphSourceParams_PgSQL ()
{
	m_iRangeStep = 1024;
	m_uPort = 5432;
}


CSphSource_PgSQL::CSphSource_PgSQL ( const char * sName )
	: CSphSource_SQL	( sName )
{}


bool CSphSource_PgSQL::SqlIsError ()
{
	return ( m_iPgRow<m_iPgRows ); // if we're over, it's just last row
}


const char * CSphSource_PgSQL::SqlError ()
{
	if ( sph_PQerrorMessage!=nullptr )
		return sph_PQerrorMessage ( m_tPgDriver );
	return "PgSQL source wasn't initialized. Wrong name in dlopen?";
}


bool CSphSource_PgSQL::Setup ( const CSphSourceParams_PgSQL & tParams )
{
	// checks
	CSphSource_SQL::Setup ( tParams );

	m_sPgClientEncoding = tParams.m_sClientEncoding;
	if ( !m_sPgClientEncoding.cstr() )
		m_sPgClientEncoding = "";

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "pgsql%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}


bool CSphSource_PgSQL::IterateStart ( CSphString & sError )
{
	bool bResult = CSphSource_SQL::IterateStart ( sError );
	if ( !bResult )
		return false;

	int iMaxIndex = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.GetAttr(i).m_iIndex );

	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.GetField(i).m_iIndex );

	m_dIsColumnBool.Resize ( iMaxIndex + 1 );
	ARRAY_FOREACH ( i, m_dIsColumnBool )
		m_dIsColumnBool[i] = false;

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		m_dIsColumnBool [ m_tSchema.GetAttr(i).m_iIndex ] = ( m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_BOOL );

	return true;
}


bool CSphSource_PgSQL::SqlConnect ()
{
	if ( !InitDynamicPosgresql() )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL (NO POSGRES CLIENT LIB, tried " PGSQL_LIB ")\n" );
		return false;
	}

	char sPort[64];
	snprintf ( sPort, sizeof(sPort), "%d", m_tParams.m_uPort );
	m_tPgDriver = sph_PQsetdbLogin ( m_tParams.m_sHost.cstr(), sPort, NULL, NULL,
		m_tParams.m_sDB.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr() );

	if ( sph_PQstatus ( m_tPgDriver )==CONNECTION_BAD )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	// set client encoding
	if ( !m_sPgClientEncoding.IsEmpty() )
		if ( -1==sph_PQsetClientEncoding ( m_tPgDriver, m_sPgClientEncoding.cstr() ) )
	{
		SqlDisconnect ();
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-CONNECT: ok\n" );
	return true;
}


void CSphSource_PgSQL::SqlDisconnect ()
{
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-DISCONNECT\n" );

	sph_PQfinish ( m_tPgDriver );
}


bool CSphSource_PgSQL::SqlQuery ( const char * sQuery )
{
	m_iPgRow = -1;
	m_iPgRows = 0;

	m_pPgResult = sph_PQexec ( m_tPgDriver, sQuery );

	ExecStatusType eRes = sph_PQresultStatus ( m_pPgResult );
	if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL\n", sQuery );
		return false;
	}
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-QUERY: %s: ok\n", sQuery );

	m_iPgRows = sph_PQntuples ( m_pPgResult );
	return true;
}


void CSphSource_PgSQL::SqlDismissResult ()
{
	if ( !m_pPgResult )
		return;

	sph_PQclear ( m_pPgResult );
	m_pPgResult = NULL;
}


int CSphSource_PgSQL::SqlNumFields ()
{
	if ( !m_pPgResult )
		return -1;

	return sph_PQnfields ( m_pPgResult );
}


const char * CSphSource_PgSQL::SqlColumn ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	const char * szValue = sph_PQgetvalue ( m_pPgResult, m_iPgRow, iIndex );
	if ( m_dIsColumnBool.GetLength() && m_dIsColumnBool[iIndex] && szValue[0]=='t' && !szValue[1] )
		return "1";

	return szValue;
}


const char * CSphSource_PgSQL::SqlFieldName ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	return sph_PQfname ( m_pPgResult, iIndex );
}


bool CSphSource_PgSQL::SqlFetchRow ()
{
	if ( !m_pPgResult )
		return false;
	return ( ++m_iPgRow<m_iPgRows );
}


DWORD CSphSource_PgSQL::SqlColumnLength ( int iIndex )
{
	return sph_PQgetlength ( m_pPgResult, m_iPgRow, iIndex );
}

#endif // USE_PGSQL

/////////////////////////////////////////////////////////////////////////////
// XMLPIPE (v2)
/////////////////////////////////////////////////////////////////////////////

template < typename T >
struct CSphSchemaConfigurator
{
	bool ConfigureAttrs ( const CSphVariant * pHead, ESphAttr eAttrType, CSphSchema & tSchema, CSphString & sError ) const
	{
		for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
		{
			CSphColumnInfo tCol ( pCur->strval().cstr(), eAttrType );
			char * pColon = strchr ( const_cast<char*> ( tCol.m_sName.cstr() ), ':' );
			if ( pColon )
			{
				*pColon = '\0';

				if ( eAttrType==SPH_ATTR_INTEGER )
				{
					int iBits = strtol ( pColon+1, NULL, 10 );
					if ( iBits<=0 || iBits>ROWITEM_BITS )
					{
						sphWarn ( "%s", ((T*)this)->DecorateMessage ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), iBits ) );
						iBits = -1;
					}

					tCol.m_tLocator.m_iBitCount = iBits;
				} else
				{
					sphWarn ( "%s", ((T*)this)->DecorateMessage ( "attribute '%s': bitcount is only supported for integer types", tCol.m_sName.cstr() ) );
				}
			}

			tCol.m_iIndex = tSchema.GetAttrsCount ();

			if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET )
			{
				tCol.m_eAttrType = eAttrType;
				tCol.m_eSrc = SPH_ATTRSRC_FIELD;
			}

			if ( tCol.m_sName.IsEmpty() )
			{
				sError.SetSprintf ( "column number %d has no name", tCol.m_iIndex );
				return false;
			}

			if ( CSphSchema::IsReserved ( tCol.m_sName.cstr() ) )
			{
				sError.SetSprintf ( "%s is not a valid attribute name", tCol.m_sName.cstr() );
				return false;
			}

			tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
		}

		return true;
	}

	void ConfigureFields ( const CSphVariant * pHead, bool bWordDict, CSphSchema & tSchema ) const
	{
		for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
		{
			const char * sFieldName = pCur->strval().cstr();

			bool bFound = false;
			for ( int i = 0; i < tSchema.GetFieldsCount() && !bFound; i++ )
				bFound = !strcmp ( tSchema.GetFieldName(i), sFieldName );

			if ( bFound )
				sphWarn ( "%s", ((T*)this)->DecorateMessage ( "duplicate field '%s'", sFieldName ) );
			else
				AddFieldToSchema ( sFieldName, bWordDict, tSchema );
		}
	}

	void AddFieldToSchema ( const char * sFieldName, bool bWordDict, CSphSchema & tSchema ) const
	{
		CSphColumnInfo tCol ( sFieldName );
		tCol.m_eWordpart = ((T*)this)->GetWordpart ( tCol.m_sName.cstr(), bWordDict );
		tSchema.AddField ( tCol );
	}
};


static bool SourceCheckSchema ( const CSphSchema & tSchema, CSphString & sError )
{
	SmallStringHash_T<int> hAttrs;
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr ( i );
		bool bUniq = hAttrs.Add ( 1, tAttr.m_sName );

		if ( !bUniq )
		{
			sError.SetSprintf ( "attribute %s declared multiple times", tAttr.m_sName.cstr() );
			return false;
		}
	}

	return true;
}


#if USE_LIBEXPAT

	EXPAT_F ( XML_ParserFree );
	EXPAT_F ( XML_Parse );
	EXPAT_F ( XML_GetCurrentColumnNumber );
	EXPAT_F ( XML_GetCurrentLineNumber );
	EXPAT_F ( XML_GetErrorCode );
	EXPAT_F ( XML_ErrorString );
	EXPAT_F ( XML_ParserCreate );
	EXPAT_F ( XML_SetUserData );
	EXPAT_F ( XML_SetElementHandler );
	EXPAT_F ( XML_SetCharacterDataHandler );
	EXPAT_F ( XML_SetUnknownEncodingHandler );

	#ifndef EXPAT_LIB
		#define EXPAT_LIB "no_expat"
	#endif

	#if DL_EXPAT
		bool InitDynamicExpat ()
		{
			const char * sFuncs[] = { "XML_ParserFree", "XML_Parse",
					"XML_GetCurrentColumnNumber", "XML_GetCurrentLineNumber", "XML_GetErrorCode", "XML_ErrorString",
					"XML_ParserCreate", "XML_SetUserData", "XML_SetElementHandler", "XML_SetCharacterDataHandler",
					"XML_SetUnknownEncodingHandler" };

			void ** pFuncs[] = { (void **) & sph_XML_ParserFree, (void **) & sph_XML_Parse,
					(void **) & sph_XML_GetCurrentColumnNumber, (void **) & sph_XML_GetCurrentLineNumber,
					(void **) & sph_XML_GetErrorCode, (void **) & sph_XML_ErrorString,
					(void **) & sph_XML_ParserCreate, (void **) & sph_XML_SetUserData,
					(void **) & sph_XML_SetElementHandler, (void **) & sph_XML_SetCharacterDataHandler,
					(void **) & sph_XML_SetUnknownEncodingHandler };

			static CSphDynamicLibrary dLib ( EXPAT_LIB );
			if ( !dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) ) )
				return false;
			return true;
		}

	#else
		#define InitDynamicExpat() (true)
	#endif

/// XML pipe source implementation (v2)
class CSphSource_XMLPipe2 : public CSphSource_Document, public CSphSchemaConfigurator<CSphSource_XMLPipe2>
{
public:
	explicit			CSphSource_XMLPipe2 ( const char * sName );
					~CSphSource_XMLPipe2 ();

	bool			Setup ( int iFieldBufferMax, bool bFixupUTF8, FILE * pPipe, const CSphConfigSection & hSource, CSphString & sError );			///< memorize the command
	virtual bool	Connect ( CSphString & sError );			///< run the command and open the pipe
	virtual void	Disconnect ();								///< close the pipe

	virtual bool	IterateStart ( CSphString & ) { m_iPlainFieldsLength = m_tSchema.GetFieldsCount(); return true; }	///< Connect() starts getting documents automatically, so this one is empty
	virtual BYTE **	NextDocument ( bool & bEOF, CSphString & sError );			///< parse incoming chunk and emit some hits
	virtual const int *	GetFieldLengths () const { return m_dFieldLengths.Begin(); }

	virtual bool	IterateMultivaluedStart ( int, CSphString & )	{ return false; }
	virtual bool	IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue )	{ return false; }
	virtual bool	IterateKillListStart ( CSphString & );
	virtual bool	IterateKillListNext ( DocID_t & tDocId );

	void			StartElement ( const char * szName, const char ** pAttrs );
	void			EndElement ( const char * pName );
	void			Characters ( const char * pCharacters, int iLen );

	void			Error ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	const char *	DecorateMessage ( const char * sTemplate, ... ) const __attribute__ ( ( format ( printf, 2, 3 ) ) );
	const char *	DecorateMessageVA ( const char * sTemplate, va_list ap ) const;

private:
	struct Document_t
	{
		DocID_t							m_tDocID;
		CSphVector < CSphVector<BYTE> >	m_dFields;
		StrVec_t						m_dAttrs;
	};

	Document_t *				m_pCurDocument;
	CSphVector<Document_t *>	m_dParsedDocuments;

	FILE *			m_pPipe;			///< incoming stream
	CSphString		m_sError;
	StrVec_t		m_dDefaultAttrs;
	StrVec_t		m_dInvalid;
	StrVec_t		m_dWarned;
	int				m_iElementDepth;

	BYTE *			m_pBuffer;
	int				m_iBufferSize;

	CSphVector<BYTE*>m_dFieldPtrs;
	CSphVector<int>	m_dFieldLengths;
	bool			m_bRemoveParsed;

	bool			m_bInDocset;
	bool			m_bInSchema;
	bool			m_bInDocument;
	bool			m_bInKillList;
	bool			m_bInId;
	bool			m_bInIgnoredTag;
	bool			m_bFirstTagAfterDocset;

	int				m_iKillListIterator;
	CSphVector<DocID_t> m_dKillList;

	int				m_iCurField;
	int				m_iCurAttr;

	XML_Parser		m_pParser;

	int				m_iFieldBufferMax;
	BYTE * 			m_pFieldBuffer;
	int				m_iFieldBufferLen;

	bool			m_bFixupUTF8;		///< whether to replace invalid utf-8 codepoints with spaces
	int				m_iReparseStart;	///< utf-8 fixerupper might need to postpone a few bytes, starting at this offset
	int				m_iReparseLen;		///< and this much bytes (under 4)

	void			UnexpectedCharaters ( const char * pCharacters, int iLen, const char * szComment );

	bool			ParseNextChunk ( int iBufferLen, CSphString & sError );

	void DocumentError ( const char * sWhere )
	{
		Error ( "malformed source, <sphinx:document> found inside %s", sWhere );

		// Ideally I'd like to display a notice on the next line that
		// would say where exactly it's allowed. E.g.:
		//
		// <sphinx:document> must be contained in <sphinx:docset>
	}
};


// callbacks
static void XMLCALL xmlStartElement ( void * user_data, const XML_Char * name, const XML_Char ** attrs )
{
	CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->StartElement ( name, attrs );
}


static void XMLCALL xmlEndElement ( void * user_data, const XML_Char * name )
{
	CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->EndElement ( name );
}


static void XMLCALL xmlCharacters ( void * user_data, const XML_Char * ch, int len )
{
	CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->Characters ( ch, len );
}

#if USE_LIBICONV
static int XMLCALL xmlUnknownEncoding ( void *, const XML_Char * name, XML_Encoding * info )
{
	iconv_t pDesc = iconv_open ( "UTF-16", name );
	if ( !pDesc )
		return XML_STATUS_ERROR;

	for ( size_t i = 0; i < 256; i++ )
	{
		char cIn = (char) i;
		char dOut[4];
		memset ( dOut, 0, sizeof ( dOut ) );
#if ICONV_INBUF_CONST
		const char * pInbuf = &cIn;
#else
		char * pInbuf = &cIn;
#endif
		char * pOutbuf = dOut;
		size_t iInBytesLeft = 1;
		size_t iOutBytesLeft = 4;

		if ( iconv ( pDesc, &pInbuf, &iInBytesLeft, &pOutbuf, &iOutBytesLeft )!=size_t(-1) )
			info->map[i] = int ( BYTE ( dOut[0] ) ) << 8 | int ( BYTE ( dOut[1] ) );
		else
			info->map[i] = 0;
	}

	iconv_close ( pDesc );

	return XML_STATUS_OK;
}
#endif

CSphSource_XMLPipe2::CSphSource_XMLPipe2 ( const char * sName )
	: CSphSource_Document ( sName )
	, m_pCurDocument	( NULL )
	, m_pPipe			( NULL )
	, m_iElementDepth	( 0 )
	, m_pBuffer			( NULL )
	, m_iBufferSize		( 1048576 )
	, m_bRemoveParsed	( false )
	, m_bInDocset		( false )
	, m_bInSchema		( false )
	, m_bInDocument		( false )
	, m_bInKillList		( false )
	, m_bInId			( false )
	, m_bInIgnoredTag	( false )
	, m_bFirstTagAfterDocset	( false )
	, m_iKillListIterator		( 0 )
	, m_iCurField		( -1 )
	, m_iCurAttr		( -1 )
	, m_pParser			( NULL )
	, m_iFieldBufferMax	( 65536 )
	, m_pFieldBuffer	( NULL )
	, m_iFieldBufferLen	( 0 )
	, m_bFixupUTF8		( false )
	, m_iReparseStart	( 0 )
	, m_iReparseLen		( 0 )
{
}


CSphSource_XMLPipe2::~CSphSource_XMLPipe2 ()
{
	Disconnect ();
	SafeDeleteArray ( m_pBuffer );
	SafeDeleteArray ( m_pFieldBuffer );
	ARRAY_FOREACH ( i, m_dParsedDocuments )
		SafeDelete ( m_dParsedDocuments[i] );
}


void CSphSource_XMLPipe2::Disconnect ()
{
	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}

	if ( m_pParser )
	{
		sph_XML_ParserFree ( m_pParser );
		m_pParser = NULL;
	}

	m_tHits.m_dData.Reset();
}


void CSphSource_XMLPipe2::Error ( const char * sTemplate, ... )
{
	if ( !m_sError.IsEmpty() )
		return;

	va_list ap;
	va_start ( ap, sTemplate );
	m_sError = DecorateMessageVA ( sTemplate, ap );
	va_end ( ap );
}


const char * CSphSource_XMLPipe2::DecorateMessage ( const char * sTemplate, ... ) const
{
	va_list ap;
	va_start ( ap, sTemplate );
	const char * sRes = DecorateMessageVA ( sTemplate, ap );
	va_end ( ap );
	return sRes;
}


const char * CSphSource_XMLPipe2::DecorateMessageVA ( const char * sTemplate, va_list ap ) const
{
	static char sBuf[1024];

	snprintf ( sBuf, sizeof(sBuf), "source '%s': ", m_tSchema.GetName() );
	int iBufLen = strlen ( sBuf );
	int iLeft = sizeof(sBuf) - iBufLen;
	char * szBufStart = sBuf + iBufLen;

	vsnprintf ( szBufStart, iLeft, sTemplate, ap );
	iBufLen = strlen ( sBuf );
	iLeft = sizeof(sBuf) - iBufLen;
	szBufStart = sBuf + iBufLen;

	if ( m_pParser )
	{
		DocID_t tFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			tFailedID = m_dParsedDocuments.Last()->m_tDocID;

		snprintf ( szBufStart, iLeft, " (line=%d, pos=%d, docid=" INT64_FMT ")",
			(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
			tFailedID );
	}

	return sBuf;
}


bool CSphSource_XMLPipe2::Setup ( int iFieldBufferMax, bool bFixupUTF8, FILE * pPipe, const CSphConfigSection & hSource, CSphString & sError )
{
	assert ( !m_pBuffer && !m_pFieldBuffer && !m_pPipe );

	m_pBuffer = new BYTE [m_iBufferSize];
	m_iFieldBufferMax = Max ( iFieldBufferMax, 65536 );
	m_pFieldBuffer = new BYTE [ m_iFieldBufferMax ];
	m_bFixupUTF8 = bFixupUTF8;
	m_pPipe = pPipe;
	m_tSchema.Reset ();
	bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );

	bOk &= ConfigureAttrs ( hSource("xmlpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	if ( !SourceCheckSchema ( m_tSchema, sError ) )
		return false;

	ConfigureFields ( hSource("xmlpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("xmlpipe_field_string"), bWordDict, m_tSchema );

	AllocDocinfo();
	return true;
}


bool CSphSource_XMLPipe2::Connect ( CSphString & sError )
{
	assert ( m_pBuffer && m_pFieldBuffer );

	// source settings have been updated after ::Setup
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		ESphWordpart eWordpart = GetWordpart ( m_tSchema.GetFieldName(i), m_pDict && m_pDict->GetSettings().m_bWordDict );
		m_tSchema.SetFieldWordpart ( i, eWordpart );
	}

	if_const ( !InitDynamicExpat() )
	{
		sError.SetSprintf ( "xmlpipe: failed to load libexpat library (tried "  EXPAT_LIB ")" );
		return false;
	}

	if ( !AddAutoAttrs ( sError ) )
		return false;

	AllocDocinfo();

	m_pParser = sph_XML_ParserCreate(NULL);
	if ( !m_pParser )
	{
		sError.SetSprintf ( "xmlpipe: failed to create XML parser" );
		return false;
	}

	sph_XML_SetUserData ( m_pParser, this );
	sph_XML_SetElementHandler ( m_pParser, xmlStartElement, xmlEndElement );
	sph_XML_SetCharacterDataHandler ( m_pParser, xmlCharacters );

#if USE_LIBICONV
	sph_XML_SetUnknownEncodingHandler ( m_pParser, xmlUnknownEncoding, NULL );
#endif

	m_dKillList.Reserve ( 1024 );
	m_dKillList.Resize ( 0 );

	m_bRemoveParsed = false;
	m_bInDocset = false;
	m_bInSchema = false;
	m_bInDocument = false;
	m_bInKillList = false;
	m_bInId = false;
	m_bFirstTagAfterDocset = false;
	m_iCurField = -1;
	m_iCurAttr = -1;
	m_iElementDepth = 0;

	m_dParsedDocuments.Reset ();
	m_dDefaultAttrs.Reset ();
	m_dInvalid.Reset ();
	m_dWarned.Reset ();

	m_dParsedDocuments.Reserve ( 1024 );
	m_dParsedDocuments.Resize ( 0 );

	m_iKillListIterator = 0;

	m_sError = "";

	int iBytesRead = fread ( m_pBuffer, 1, m_iBufferSize, m_pPipe );

	if ( !ParseNextChunk ( iBytesRead, sError ) )
		return false;

	m_tHits.m_dData.Reserve ( m_iMaxHits );

	return true;
}


bool CSphSource_XMLPipe2::ParseNextChunk ( int iBufferLen, CSphString & sError )
{
	if ( !iBufferLen )
		return true;

	bool bLast = ( iBufferLen!=m_iBufferSize );

	m_iReparseLen = 0;
	if ( m_bFixupUTF8 )
	{
		BYTE * p = m_pBuffer;
		BYTE * pMax = m_pBuffer + iBufferLen;

		while ( p<pMax )
		{
			BYTE v = *p;

			// fix control codes
			if ( v<0x20 && v!=0x0D && v!=0x0A )
			{
				*p++ = ' ';
				continue;
			}

			// accept ascii7 codes
			if ( v<128 )
			{
				p++;
				continue;
			}

			// remove invalid start bytes
			if ( v<0xC2 )
			{
				*p++ = ' ';
				continue;
			}

			// get and check byte count
			int iBytes = 0;
			while ( v & 0x80 )
			{
				iBytes++;
				v <<= 1;
			}
			if ( iBytes<2 || iBytes>3 )
			{
				*p++ = ' ';
				continue;
			}

			// if we're on a boundary, save these few bytes for the future
			if ( p+iBytes>pMax )
			{
				m_iReparseStart = (int)(p-m_pBuffer);
				m_iReparseLen = (int)(pMax-p);
				iBufferLen -= m_iReparseLen;
				break;
			}

			// otherwise (not a boundary), check them all
			int i = 1;
			int iVal = ( v >> iBytes );
			for ( ; i<iBytes; i++ )
			{
				if ( ( p[i] & 0xC0 )!=0x80 )
					break;
				iVal = ( iVal<<6 ) + ( p[i] & 0x3f );
			}

			if ( i!=iBytes // remove invalid sequences
				|| ( iVal>=0xd800 && iVal<=0xdfff ) // and utf-16 surrogate pairs
				|| ( iBytes==3 && iVal<0x800 ) // and overlong 3-byte codes
				|| ( iVal>=0xfff0 && iVal<=0xffff ) ) // and kinda-valid specials expat chokes on anyway
			{
				iBytes = i;
				for ( i=0; i<iBytes; i++ )
					p[i] = ' ';
			}

			// only move forward by the amount of succesfully processed bytes!
			p += i;
		}
	}

	if ( sph_XML_Parse ( m_pParser, (const char*) m_pBuffer, iBufferLen, bLast )!=XML_STATUS_OK )
	{
		DocID_t tFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			tFailedID = m_dParsedDocuments.Last()->m_tDocID;

		if ( !m_sError.IsEmpty () )
			sError = m_sError;
		else
		{
			sError.SetSprintf ( "source '%s': XML parse error: %s (line=%d, pos=%d, docid=" INT64_FMT ")",
				m_tSchema.GetName(), sph_XML_ErrorString ( sph_XML_GetErrorCode ( m_pParser ) ),
				(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
				tFailedID );
		}

		m_tDocInfo.m_tRowID = 0;
		return false;
	}

	if ( !m_sError.IsEmpty () )
	{
		sError = m_sError;
		m_tDocInfo.m_tRowID = 0;
		return false;
	}

	return true;
}


BYTE **	CSphSource_XMLPipe2::NextDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_pBuffer && m_pFieldBuffer );

	bEOF = true;

	if ( m_bRemoveParsed )
	{
		SafeDelete ( m_dParsedDocuments[0] );
		m_dParsedDocuments.RemoveFast ( 0 );
		m_bRemoveParsed = false;
	}

	// ok by default
	m_tDocInfo.m_tRowID = INVALID_ROWID;

	int iReadResult = 0;

	while ( m_dParsedDocuments.GetLength()==0 )
	{
		// saved bytes to the front!
		if ( m_iReparseLen )
			memmove ( m_pBuffer, m_pBuffer+m_iReparseStart, m_iReparseLen );

		// read more data
		iReadResult = fread ( m_pBuffer+m_iReparseLen, 1, m_iBufferSize-m_iReparseLen, m_pPipe );
		if ( iReadResult==0 )
			break;

		// and parse it
		if ( !ParseNextChunk ( iReadResult+m_iReparseLen, sError ) )
			return NULL;
	}

	while ( m_dParsedDocuments.GetLength()!=0 )
	{
		Document_t * pDocument = m_dParsedDocuments[0];
		int nAttrs = m_tSchema.GetAttrsCount ();

		int iFirstFieldLenAttr = m_tSchema.GetAttrId_FirstFieldLen();
		int iLastFieldLenAttr = m_tSchema.GetAttrId_LastFieldLen();

		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & i : m_dMvas )
			i.Resize(0);

		// attributes
		for ( int i = 0; i < nAttrs; i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( i );

			// docid comes separate from other attrs
			if ( !i )
			{
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, pDocument->m_tDocID );
				continue;
			}

			// reset, and the value will be filled by IterateHits()
			if ( i>=iFirstFieldLenAttr && i<=iLastFieldLenAttr )
			{
				assert ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT );
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				continue;
			}

			const CSphString & sAttrValue = pDocument->m_dAttrs[i].IsEmpty () && m_dDefaultAttrs.GetLength()
				? m_dDefaultAttrs[i]
				: pDocument->m_dAttrs[i];

			switch ( tAttr.m_eAttrType )
			{
				case SPH_ATTR_STRING:
				case SPH_ATTR_JSON:
					m_dStrAttrs[i] = sAttrValue.cstr();
					if ( !m_dStrAttrs[i].cstr() )
						m_dStrAttrs[i] = "";

					m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
					break;

				case SPH_ATTR_FLOAT:
					m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, sphToFloat ( sAttrValue.cstr () ) );
					break;

				case SPH_ATTR_BIGINT:
					m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToInt64 ( sAttrValue.cstr () ) );
					break;

				case SPH_ATTR_UINT32SET:
				case SPH_ATTR_INT64SET:
					if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
						ParseFieldMVA ( i, sAttrValue.cstr() );
					break;

				default:
					m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToDword ( sAttrValue.cstr () ) );
					break;
			}
		}

		m_bRemoveParsed = true;

		int nFields = m_tSchema.GetFieldsCount();
		if ( !nFields )
			return nullptr;

		m_dFieldPtrs.Resize ( nFields );
		m_dFieldLengths.Resize ( nFields );
		for ( int i = 0; i < nFields; ++i )
		{
			m_dFieldPtrs[i] = pDocument->m_dFields[i].Begin();
			m_dFieldLengths[i] = pDocument->m_dFields[i].GetLength();

			// skip trailing zero
			if ( m_dFieldLengths[i] && !m_dFieldPtrs[i][m_dFieldLengths[i]-1] )
				m_dFieldLengths[i]--;
		}

		bEOF = false;
		return (BYTE **)&( m_dFieldPtrs[0] );
	}

	return nullptr;
}


bool CSphSource_XMLPipe2::IterateKillListStart ( CSphString & )
{
	m_iKillListIterator = 0;
	return true;
}


bool CSphSource_XMLPipe2::IterateKillListNext ( DocID_t & tDocID )
{
	if ( m_iKillListIterator>=m_dKillList.GetLength () )
		return false;

	tDocID = m_dKillList [ m_iKillListIterator++ ];
	return true;
}

enum EXMLElem
{
	ELEM_DOCSET,
	ELEM_SCHEMA,
	ELEM_FIELD,
	ELEM_ATTR,
	ELEM_DOCUMENT,
	ELEM_KLIST,
	ELEM_NONE
};

static EXMLElem LookupElement ( const char * szName )
{
	if ( szName[0]!='s' )
		return ELEM_NONE;

	int iLen = strlen(szName);
	if ( iLen>=11 && iLen<=15 )
	{
		char iHash = (char)( ( iLen + szName[7] ) & 15 );
		switch ( iHash )
		{
		case 1:		if ( !strcmp ( szName, "sphinx:docset" ) )		return ELEM_DOCSET; break;
		case 0:		if ( !strcmp ( szName, "sphinx:schema" ) )		return ELEM_SCHEMA;	break;
		case 2:		if ( !strcmp ( szName, "sphinx:field" ) )		return ELEM_FIELD;	break;
		case 12:	if ( !strcmp ( szName, "sphinx:attr" ) )		return ELEM_ATTR;	break;
		case 3:		if ( !strcmp ( szName, "sphinx:document" ) )	return ELEM_DOCUMENT;break;
		case 10:	if ( !strcmp ( szName, "sphinx:killlist" ) )	return ELEM_KLIST;	break;
		}
	}

	return ELEM_NONE;
}

void CSphSource_XMLPipe2::StartElement ( const char * szName, const char ** pAttrs )
{
	EXMLElem ePos = LookupElement ( szName );

	switch ( ePos )
	{
	case ELEM_DOCSET:
		m_bInDocset = true;
		m_bFirstTagAfterDocset = true;
		return;

	case ELEM_SCHEMA:
	{
		if ( !m_bInDocset || !m_bFirstTagAfterDocset )
		{
			Error ( "<sphinx:schema> is allowed immediately after <sphinx:docset> only" );
			return;
		}

		if ( m_tSchema.GetFieldsCount() > 0 || m_tSchema.GetAttrsCount () > 1 )
			sphWarn ( "%s", DecorateMessage ( "both embedded and configured schemas found; using embedded" ) );

		m_tSchema.Reset();
		CSphMatch tDocInfo;
		Swap ( m_tDocInfo, tDocInfo );
		m_dDefaultAttrs.Reset();

		m_bFirstTagAfterDocset = false;
		m_bInSchema = true;
	}
	return;

	case ELEM_FIELD:
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:field> is allowed inside <sphinx:schema> only" );
			return;
		}

		const char ** dAttrs = pAttrs;
		CSphColumnInfo Info;
		CSphString sDefault;
		bool bIsAttr = false;
		bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
			{
				AddFieldToSchema ( dAttrs[1], bWordDict, m_tSchema );
				Info.m_sName = dAttrs[1];
			} else if ( !strcmp ( *dAttrs, "attr" ) )
			{
				bIsAttr = true;
				if ( !strcmp ( dAttrs[1], "string" ) )
					Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( dAttrs[1], "json" ) )
					Info.m_eAttrType = SPH_ATTR_JSON;

			} else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];

			dAttrs += 2;
		}

		if ( bIsAttr )
		{
			if ( Info.m_sName.IsEmpty() || CSphSchema::IsReserved ( Info.m_sName.cstr() ) )
			{
				Error ( "%s is not a valid attribute name", Info.m_sName.cstr() );
				return;
			}

			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
		}
	}
	return;

	case ELEM_ATTR:
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:attr> is allowed inside <sphinx:schema> only" );
			return;
		}

		bool bError = false;
		CSphString sDefault;

		CSphColumnInfo Info;
		Info.m_eAttrType = SPH_ATTR_INTEGER;

		const char ** dAttrs = pAttrs;

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] && !bError )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
				Info.m_sName = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "bits" ) )
				Info.m_tLocator.m_iBitCount = strtol ( dAttrs[1], NULL, 10 );
			else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "type" ) )
			{
				const char * szType = dAttrs[1];
				if ( !strcmp ( szType, "int" ) )				Info.m_eAttrType = SPH_ATTR_INTEGER;
				else if ( !strcmp ( szType, "timestamp" ) )		Info.m_eAttrType = SPH_ATTR_TIMESTAMP;
				else if ( !strcmp ( szType, "bool" ) )			Info.m_eAttrType = SPH_ATTR_BOOL;
				else if ( !strcmp ( szType, "float" ) )			Info.m_eAttrType = SPH_ATTR_FLOAT;
				else if ( !strcmp ( szType, "bigint" ) )		Info.m_eAttrType = SPH_ATTR_BIGINT;
				else if ( !strcmp ( szType, "string" ) )		Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( szType, "json" ) )			Info.m_eAttrType = SPH_ATTR_JSON;
				else if ( !strcmp ( szType, "multi" ) )
				{
					Info.m_eAttrType = SPH_ATTR_UINT32SET;
					Info.m_eSrc = SPH_ATTRSRC_FIELD;
				} else if ( !strcmp ( szType, "multi_64" ) )
				{
					Info.m_eAttrType = SPH_ATTR_INT64SET;
					Info.m_eSrc = SPH_ATTRSRC_FIELD;
				} else
				{
					Error ( "unknown column type '%s'", szType );
					bError = true;
				}
			}

			dAttrs += 2;
		}

		if ( !bError )
		{
			if ( Info.m_sName.IsEmpty() || CSphSchema::IsReserved ( Info.m_sName.cstr() ) )
			{
				Error ( "%s is not a valid attribute name", Info.m_sName.cstr() );
				return;
			}

			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
		}
	}
	return;

	case ELEM_DOCUMENT:
	{
		if ( !m_bInDocset || m_bInSchema )
			return DocumentError ( "<sphinx:schema>" );

		if ( m_bInKillList )
			return DocumentError ( "<sphinx:killlist>" );

		if ( m_bInDocument )
			return DocumentError ( "<sphinx:document>" );

		if ( m_tSchema.GetFieldsCount()==0 && m_tSchema.GetAttrsCount()==0 )
		{
			Error ( "no schema configured, and no embedded schema found" );
			return;
		}

		m_bInDocument = true;

		assert ( !m_pCurDocument );
		m_pCurDocument = new Document_t;

		m_pCurDocument->m_tDocID = INT64_MAX;
		m_pCurDocument->m_dFields.Resize ( m_tSchema.GetFieldsCount() );
		// for safety
		ARRAY_FOREACH ( i, m_pCurDocument->m_dFields )
			m_pCurDocument->m_dFields[i].Add ( '\0' );
		m_pCurDocument->m_dAttrs.Resize ( m_tSchema.GetAttrsCount () );

		if ( pAttrs[0] && pAttrs[1] && pAttrs[0][0] && pAttrs[1][0] )
			if ( !strcmp ( pAttrs[0], "id" ) )
				m_pCurDocument->m_tDocID = sphToInt64 ( pAttrs[1] );

		if ( m_pCurDocument->m_tDocID==0 )
			Error ( "attribute 'id' required in <sphinx:document>" );
	}
	return;

	case ELEM_KLIST:
	{
		if ( !m_bInDocset || m_bInDocument || m_bInSchema )
		{
			Error ( "<sphinx:killlist> is not allowed inside <sphinx:schema> or <sphinx:document>" );
			return;
		}

		m_bInKillList = true;
	}
	return;

	case ELEM_NONE: break; // avoid warning
	}

	if ( m_bInKillList )
	{
		if ( m_bInId )
		{
			m_iElementDepth++;
			return;
		}

		if ( strcmp ( szName, "id" ) )
		{
			Error ( "only 'id' is allowed inside <sphinx:killlist>" );
			return;
		}

		m_bInId = true;

	} else if ( m_bInDocument )
	{
		if ( m_iCurField!=-1 || m_iCurAttr!=-1 )
		{
			m_iElementDepth++;
			return;
		}

		m_iCurField = m_tSchema.GetFieldIndex ( szName );
		m_iCurAttr = m_tSchema.GetAttrIndex ( szName );

		if ( m_iCurAttr!=-1 || m_iCurField!=-1 )
			return;

		m_bInIgnoredTag = true;

		bool bInvalidFound = false;
		for ( int i = 0; i < m_dInvalid.GetLength () && !bInvalidFound; i++ )
			bInvalidFound = m_dInvalid[i]==szName;

		if ( !bInvalidFound )
		{
			sphWarn ( "%s", DecorateMessage ( "unknown field/attribute '%s'; ignored", szName ) );
			m_dInvalid.Add ( szName );
		}
	}
}


void CSphSource_XMLPipe2::EndElement ( const char * szName )
{
	m_bInIgnoredTag = false;

	EXMLElem ePos = LookupElement ( szName );

	switch ( ePos )
	{
	case ELEM_DOCSET:
		m_bInDocset = false;
		return;

	case ELEM_SCHEMA:
		m_bInSchema = false;
		AddAutoAttrs ( m_sError, &m_dDefaultAttrs );
		AllocDocinfo();
		return;

	case ELEM_DOCUMENT:
		m_bInDocument = false;
		if ( m_pCurDocument )
			m_dParsedDocuments.Add ( m_pCurDocument );
		m_pCurDocument = NULL;
		return;

	case ELEM_KLIST:
		m_bInKillList = false;
		return;

	case ELEM_FIELD: // avoid warnings
	case ELEM_ATTR:
	case ELEM_NONE: break;
	}

	if ( m_bInKillList )
	{
		if ( m_iElementDepth!=0 )
		{
			m_iElementDepth--;
			return;
		}

		if ( m_bInId )
		{
			m_pFieldBuffer [ Min ( m_iFieldBufferLen, m_iFieldBufferMax-1 ) ] = '\0';
			m_dKillList.Add ( sphToInt64 ( (const char *)m_pFieldBuffer ) );
			m_iFieldBufferLen = 0;
			m_bInId = false;
		}

	} else if ( m_bInDocument && ( m_iCurAttr!=-1 || m_iCurField!=-1 ) )
	{
		if ( m_iElementDepth!=0 )
		{
			m_iElementDepth--;
			return;
		}

		if ( m_iCurField!=-1 )
		{
			assert ( m_pCurDocument );
			CSphVector<BYTE> & dBuf = m_pCurDocument->m_dFields [ m_iCurField ];

			dBuf.Last() = ' ';
			dBuf.Reserve ( dBuf.GetLength() + m_iFieldBufferLen + 6 ); // 6 is a safety gap
			dBuf.Append( m_pFieldBuffer, m_iFieldBufferLen );
			dBuf.Add ( '\0' );
		}
		if ( m_iCurAttr!=-1 )
		{
			assert ( m_pCurDocument );
			if ( !m_pCurDocument->m_dAttrs [ m_iCurAttr ].IsEmpty () )
				sphWarn ( "duplicate attribute node <%s> - using first value", m_tSchema.GetAttr ( m_iCurAttr ).m_sName.cstr() );
			else
				m_pCurDocument->m_dAttrs [ m_iCurAttr ].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
		}

		m_iFieldBufferLen = 0;

		m_iCurAttr = -1;
		m_iCurField = -1;
	}
}


void CSphSource_XMLPipe2::UnexpectedCharaters ( const char * pCharacters, int iLen, const char * szComment )
{
	const int MAX_WARNING_LENGTH = 64;

	bool bSpaces = true;
	for ( int i = 0; i < iLen && bSpaces; i++ )
		if ( !sphIsSpace ( pCharacters[i] ) )
			bSpaces = false;

	if ( !bSpaces )
	{
		CSphString sWarning;
		sWarning.SetBinary ( pCharacters, Min ( iLen, MAX_WARNING_LENGTH ) );
		sphWarn ( "source '%s': unexpected string '%s' (line=%d, pos=%d) %s",
			m_tSchema.GetName(), sWarning.cstr (),
			(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ), szComment );
	}
}


void CSphSource_XMLPipe2::Characters ( const char * pCharacters, int iLen )
{
	if ( m_bInIgnoredTag )
		return;

	if ( !m_bInDocset )
	{
		UnexpectedCharaters ( pCharacters, iLen, "outside of <sphinx:docset>" );
		return;
	}

	if ( !m_bInSchema && !m_bInDocument && !m_bInKillList )
	{
		UnexpectedCharaters ( pCharacters, iLen, "outside of <sphinx:schema> and <sphinx:document>" );
		return;
	}

	if ( m_iCurAttr==-1 && m_iCurField==-1 && !m_bInKillList )
	{
		UnexpectedCharaters ( pCharacters, iLen, m_bInDocument ? "inside <sphinx:document>" : ( m_bInSchema ? "inside <sphinx:schema>" : "" ) );
		return;
	}

	if ( iLen + m_iFieldBufferLen < m_iFieldBufferMax )
	{
		memcpy ( m_pFieldBuffer + m_iFieldBufferLen, pCharacters, iLen );
		m_iFieldBufferLen += iLen;

	} else
	{
		const char * szName = nullptr;
		if ( m_iCurField!=-1 )
			szName = m_tSchema.GetFieldName ( m_iCurField );
		else if ( m_iCurAttr!=-1 )
			szName = m_tSchema.GetAttr(m_iCurAttr).m_sName.cstr();

		assert ( szName );

		bool bWarned = false;
		for ( int i = 0; i < m_dWarned.GetLength () && !bWarned; i++ )
			bWarned = m_dWarned[i]==szName;

		if ( !bWarned )
		{
			sphWarn ( "source '%s': field/attribute '%s' length exceeds max length (line=%d, pos=%d, docid=" INT64_FMT ")",
				m_tSchema.GetName(), szName,
				(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
				m_pCurDocument->m_tDocID );

			m_dWarned.Add ( szName );
		}
	}
}

CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, const char * szSourceName, int iMaxFieldLen, bool bProxy, CSphString & sError )
{
	bool bUTF8 = pSource->GetInt ( "xmlpipe_fixup_utf8", 0 )!=0;

	CSphSource_XMLPipe2 * pXMLPipe = CreateSourceWithProxy<CSphSource_XMLPipe2> ( szSourceName, bProxy );
	if ( !pXMLPipe->Setup ( iMaxFieldLen, bUTF8, pPipe, *pSource, sError ) )
		SafeDelete ( pXMLPipe );

	return pXMLPipe;
}

#endif

#if USE_ODBC

	ODBC_F ( SQLFreeHandle );
	ODBC_F ( SQLDisconnect );
	ODBC_F ( SQLCloseCursor );
	ODBC_F ( SQLGetDiagRec );
	ODBC_F ( SQLSetEnvAttr );
	ODBC_F ( SQLAllocHandle );
	ODBC_F ( SQLFetch );
	ODBC_F ( SQLExecDirect );
	ODBC_F ( SQLNumResultCols );
	ODBC_F ( SQLDescribeCol );
	ODBC_F ( SQLBindCol );
	ODBC_F ( SQLDrivers );
	ODBC_F ( SQLDriverConnect );

	#ifndef UNIXODBC_LIB
		#define UNIXODBC_LIB "no_unixodbc"
	#endif

	#if DL_UNIXODBC

		bool InitDynamicOdbc ()
		{
			const char * sFuncs[] = {"SQLFreeHandle", "SQLDisconnect",
					"SQLCloseCursor", "SQLGetDiagRec", "SQLSetEnvAttr", "SQLAllocHandle",
					"SQLFetch", "SQLExecDirect", "SQLNumResultCols", "SQLDescribeCol",
					"SQLBindCol", "SQLDrivers", "SQLDriverConnect" };

			void ** pFuncs[] = {(void**)&sph_SQLFreeHandle, (void**)&sph_SQLDisconnect,
					(void**)&sph_SQLCloseCursor, (void**)&sph_SQLGetDiagRec, (void**)&sph_SQLSetEnvAttr,
					(void**)&sph_SQLAllocHandle, (void**)&sph_SQLFetch, (void**)&sph_SQLExecDirect,
					(void**)&sph_SQLNumResultCols, (void**)&sph_SQLDescribeCol, (void**)&sph_SQLBindCol,
					(void**)&sph_SQLDrivers, (void**)&sph_SQLDriverConnect };

			static CSphDynamicLibrary dLib ( UNIXODBC_LIB );
			if ( !dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) ) )
				return false;
			return true;
		}

	#else
		#define InitDynamicOdbc() (true)
	#endif

CSphSourceParams_ODBC::CSphSourceParams_ODBC ()
	: m_bWinAuth	( false )
{
}


CSphSource_ODBC::CSphSource_ODBC ( const char * sName )
	: CSphSource_SQL	( sName )
	, m_bWinAuth		( false )
	, m_bUnicode		( false )
	, m_hEnv			( NULL )
	, m_hDBC			( NULL )
	, m_hStmt			( NULL )
	, m_nResultCols		( 0 )
{
}


void CSphSource_ODBC::SqlDismissResult ()
{
	if ( m_hStmt )
	{
		sph_SQLCloseCursor ( m_hStmt );
		sph_SQLFreeHandle ( SQL_HANDLE_STMT, m_hStmt );
		m_hStmt = NULL;
	}
}


#define MS_SQL_BUFFER_GAP 16


bool CSphSource_ODBC::SqlQuery ( const char * sQuery )
{
	if ( sph_SQLAllocHandle ( SQL_HANDLE_STMT, m_hDBC, &m_hStmt )==SQL_ERROR )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL (SQLAllocHandle failed)\n", sQuery );
		return false;
	}

	if ( sph_SQLExecDirect ( m_hStmt, (SQLCHAR *)sQuery, SQL_NTS )==SQL_ERROR )
	{
		GetSqlError ( SQL_HANDLE_STMT, m_hStmt );
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL\n", sQuery );
		return false;
	}
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-QUERY: %s: ok\n", sQuery );

	SQLSMALLINT nCols = 0;
	m_nResultCols = 0;
	if ( sph_SQLNumResultCols ( m_hStmt, &nCols )==SQL_ERROR )
		return false;

	m_nResultCols = nCols;

	const int MAX_NAME_LEN = 512;
	char szColumnName[MAX_NAME_LEN];

	m_dColumns.Resize ( m_nResultCols );
	int iTotalBuffer = 0;
	ARRAY_FOREACH ( i, m_dColumns )
	{
		QueryColumn_t & tCol = m_dColumns[i];

		SQLULEN uColSize = 0;
		SQLSMALLINT iNameLen = 0;
		SQLSMALLINT iDataType = 0;
		if ( sph_SQLDescribeCol ( m_hStmt, (SQLUSMALLINT)(i+1), (SQLCHAR*)szColumnName,
			MAX_NAME_LEN, &iNameLen, &iDataType, &uColSize, NULL, NULL )==SQL_ERROR )
				return false;

		tCol.m_sName = szColumnName;
		tCol.m_sName.ToLower();

		// deduce buffer size
		// use a small buffer by default, and a bigger one for varchars
		int iBuffLen = DEFAULT_COL_SIZE;
		if ( iDataType==SQL_WCHAR || iDataType==SQL_WVARCHAR || iDataType==SQL_WLONGVARCHAR|| iDataType==SQL_VARCHAR )
			iBuffLen = VARCHAR_COL_SIZE;

		if ( m_hColBuffers ( tCol.m_sName ) )
			iBuffLen = m_hColBuffers [ tCol.m_sName ]; // got explicit user override
		else if ( uColSize )
			iBuffLen = Min ( uColSize+1, (SQLULEN) MAX_COL_SIZE ); // got data from driver

		tCol.m_dContents.Resize ( iBuffLen + MS_SQL_BUFFER_GAP );
		tCol.m_dRaw.Resize ( iBuffLen + MS_SQL_BUFFER_GAP );
		tCol.m_iInd = 0;
		tCol.m_iBytes = 0;
		tCol.m_iBufferSize = iBuffLen;
		tCol.m_bUCS2 = m_bUnicode && ( iDataType==SQL_WCHAR || iDataType==SQL_WVARCHAR || iDataType==SQL_WLONGVARCHAR );
		tCol.m_bTruncated = false;
		iTotalBuffer += iBuffLen;

		if ( sph_SQLBindCol ( m_hStmt, (SQLUSMALLINT)(i+1),
			tCol.m_bUCS2 ? SQL_UNICODE : SQL_C_CHAR,
			tCol.m_bUCS2 ? tCol.m_dRaw.Begin() : tCol.m_dContents.Begin(),
			iBuffLen, &(tCol.m_iInd) )==SQL_ERROR )
				return false;
	}

	if ( iTotalBuffer>WARN_ROW_SIZE )
		sphWarn ( "row buffer is over %d bytes; consider revising sql_column_buffers", iTotalBuffer );

	return true;
}


bool CSphSource_ODBC::SqlIsError ()
{
	return !m_sError.IsEmpty ();
}


const char * CSphSource_ODBC::SqlError ()
{
	return m_sError.cstr();
}


bool CSphSource_ODBC::SqlConnect ()
{
	if_const ( !InitDynamicOdbc() )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL (NO ODBC CLIENT LIB, tried " UNIXODBC_LIB ")\n" );
		return false;
	}

	if ( sph_SQLAllocHandle ( SQL_HANDLE_ENV, NULL, &m_hEnv )==SQL_ERROR )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	sph_SQLSetEnvAttr ( m_hEnv, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, SQL_IS_INTEGER );

	if ( sph_SQLAllocHandle ( SQL_HANDLE_DBC, m_hEnv, &m_hDBC )==SQL_ERROR )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	OdbcPostConnect ();

	char szOutConn [2048];
	SQLSMALLINT iOutConn = 0;
	if ( sph_SQLDriverConnect ( m_hDBC, NULL, (SQLTCHAR*) m_sOdbcDSN.cstr(), SQL_NTS,
		(SQLCHAR*)szOutConn, sizeof(szOutConn), &iOutConn, SQL_DRIVER_NOPROMPT )==SQL_ERROR )
	{
		GetSqlError ( SQL_HANDLE_DBC, m_hDBC );
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-CONNECT: ok\n" );
	return true;
}


void CSphSource_ODBC::SqlDisconnect ()
{
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-DISCONNECT\n" );

	if ( m_hStmt!=NULL )
		sph_SQLFreeHandle ( SQL_HANDLE_STMT, m_hStmt );

	if ( m_hDBC )
	{
		sph_SQLDisconnect ( m_hDBC );
		sph_SQLFreeHandle ( SQL_HANDLE_DBC, m_hDBC );
	}

	if ( m_hEnv )
		sph_SQLFreeHandle ( SQL_HANDLE_ENV, m_hEnv );
}


int CSphSource_ODBC::SqlNumFields ()
{
	if ( !m_hStmt )
		return -1;

	return m_nResultCols;
}


bool CSphSource_ODBC::SqlFetchRow ()
{
	if ( !m_hStmt )
		return false;

	SQLRETURN iRet = sph_SQLFetch ( m_hStmt );
	if ( iRet==SQL_ERROR || iRet==SQL_INVALID_HANDLE || iRet==SQL_NO_DATA )
	{
		GetSqlError ( SQL_HANDLE_STMT, m_hStmt );
		return false;
	}

	ARRAY_FOREACH ( i, m_dColumns )
	{
		QueryColumn_t & tCol = m_dColumns[i];
		switch ( tCol.m_iInd )
		{
			case SQL_NULL_DATA:
				tCol.m_dContents[0] = '\0';
				tCol.m_iBytes = 0;
				break;

			default:
#if USE_WINDOWS // FIXME! support UCS-2 columns on Unix too
				if ( tCol.m_bUCS2 )
				{
					// WideCharToMultiByte should get NULL terminated string
					memset ( tCol.m_dRaw.Begin()+tCol.m_iBufferSize, 0, MS_SQL_BUFFER_GAP );

					int iConv = WideCharToMultiByte ( CP_UTF8, 0, LPCWSTR ( tCol.m_dRaw.Begin() ), tCol.m_iInd/sizeof(WCHAR),
						LPSTR ( tCol.m_dContents.Begin() ), tCol.m_iBufferSize-1, NULL, NULL );

					if ( iConv==0 )
						if ( GetLastError()==ERROR_INSUFFICIENT_BUFFER )
							iConv = tCol.m_iBufferSize-1;

					tCol.m_dContents[iConv] = '\0';
					tCol.m_iBytes = iConv;

				} else
#endif
				{
					if ( tCol.m_iInd>=0 && tCol.m_iInd<tCol.m_iBufferSize )
					{
						// data fetched ok; add trailing zero
						tCol.m_dContents[tCol.m_iInd] = '\0';
						tCol.m_iBytes = tCol.m_iInd;

					} else if ( tCol.m_iInd>=tCol.m_iBufferSize && !tCol.m_bTruncated )
					{
						// out of buffer; warn about that (once)
						tCol.m_bTruncated = true;
						sphWarn ( "'%s' column truncated (buffer=%d, got=%d); consider revising sql_column_buffers",
							tCol.m_sName.cstr(), tCol.m_iBufferSize-1, (int) tCol.m_iInd );
						tCol.m_iBytes = tCol.m_iBufferSize;
					}
				}
			break;
		}
	}

	return iRet!=SQL_NO_DATA;
}


const char * CSphSource_ODBC::SqlColumn ( int iIndex )
{
	if ( !m_hStmt )
		return NULL;

	return &(m_dColumns [iIndex].m_dContents[0]);
}


const char * CSphSource_ODBC::SqlFieldName ( int iIndex )
{
	return m_dColumns[iIndex].m_sName.cstr();
}


DWORD CSphSource_ODBC::SqlColumnLength ( int iIndex )
{
	return m_dColumns[iIndex].m_iBytes;
}


bool CSphSource_ODBC::Setup ( const CSphSourceParams_ODBC & tParams )
{
	if ( !CSphSource_SQL::Setup ( tParams ) )
		return false;

	// parse column buffers spec, if any
	if ( !tParams.m_sColBuffers.IsEmpty() )
	{
		const char * p = tParams.m_sColBuffers.cstr();
		while ( *p )
		{
			// skip space
			while ( sphIsSpace(*p) )
				p++;

			// expect eof or ident
			if ( !*p )
				break;
			if ( !sphIsAlpha(*p) )
			{
				m_sError.SetSprintf ( "identifier expected in sql_column_buffers near '%s'", p );
				return false;
			}

			// get ident
			CSphString sCol;
			const char * pIdent = p;
			while ( sphIsAlpha(*p) )
				p++;
			sCol.SetBinary ( pIdent, p-pIdent );

			// skip space
			while ( sphIsSpace(*p) )
				p++;

			// expect assignment
			if ( *p!='=' )
			{
				m_sError.SetSprintf ( "'=' expected in sql_column_buffers near '%s'", p );
				return false;
			}
			p++;

			// skip space
			while ( sphIsSpace(*p) )
				p++;

			// expect number
			if (!( *p>='0' && *p<='9' ))
			{
				m_sError.SetSprintf ( "number expected in sql_column_buffers near '%s'", p );
				return false;
			}

			// get value
			int iSize = 0;
			while ( *p>='0' && *p<='9' )
			{
				iSize = 10*iSize + ( *p-'0' );
				p++;
			}
			if ( *p=='K' )
			{
				iSize *= 1024;
				p++;
			} else if ( *p=='M' )
			{
				iSize *= 1048576;
				p++;
			}

			// hash value
			sCol.ToLower();
			m_hColBuffers.Add ( iSize, sCol );

			// skip space
			while ( sphIsSpace(*p) )
				p++;

			// expect eof or comma
			if ( !*p )
				break;
			if ( *p!=',' )
			{
				m_sError.SetSprintf ( "comma expected in sql_column_buffers near '%s'", p );
				return false;
			}
			p++;
		}
	}

	// ODBC specific params
	m_sOdbcDSN = tParams.m_sOdbcDSN;
	m_bWinAuth = tParams.m_bWinAuth;

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "odbc%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}


void CSphSource_ODBC::GetSqlError ( SQLSMALLINT iHandleType, SQLHANDLE hHandle )
{
	if ( !hHandle )
	{
		m_sError.SetSprintf ( "invalid handle" );
		return;
	}

	char szState[16] = "";
	char szMessageText[1024] = "";
	SQLINTEGER iError;
	SQLSMALLINT iLen;
	sph_SQLGetDiagRec ( iHandleType, hHandle, 1, (SQLCHAR*)szState, &iError, (SQLCHAR*)szMessageText, 1024, &iLen );
	m_sError = szMessageText;
}

//////////////////////////////////////////////////////////////////////////

void CSphSource_MSSQL::OdbcPostConnect ()
{
	if ( !m_sOdbcDSN.IsEmpty() )
		return;

	const int MAX_LEN = 1024;
	char szDriver[MAX_LEN];
	char szDriverAttrs[MAX_LEN];
	SQLSMALLINT iDescLen = 0;
	SQLSMALLINT iAttrLen = 0;
	SQLSMALLINT iDir = SQL_FETCH_FIRST;

	CSphString sDriver;
	while (true)
	{
		SQLRETURN iRet = sph_SQLDrivers ( m_hEnv, iDir, (SQLCHAR*)szDriver, MAX_LEN, &iDescLen, (SQLCHAR*)szDriverAttrs, MAX_LEN, &iAttrLen );
		if ( iRet==SQL_NO_DATA )
			break;

		iDir = SQL_FETCH_NEXT;
		if ( !strcmp ( szDriver, "SQL Native Client" )
			|| !strncmp ( szDriver, "SQL Server Native Client", strlen("SQL Server Native Client") ) )
		{
			sDriver = szDriver;
			break;
		}
	}

	if ( sDriver.IsEmpty() )
		sDriver = "SQL Server";

	if ( m_bWinAuth && m_tParams.m_sUser.IsEmpty () )
	{
		m_sOdbcDSN.SetSprintf ( "DRIVER={%s};SERVER={%s};Database={%s};Trusted_Connection=yes",
			sDriver.cstr (), m_tParams.m_sHost.cstr (), m_tParams.m_sDB.cstr () );

	} else if ( m_bWinAuth )
	{
		m_sOdbcDSN.SetSprintf ( "DRIVER={%s};SERVER={%s};UID={%s};PWD={%s};Database={%s};Trusted_Connection=yes",
			sDriver.cstr (), m_tParams.m_sHost.cstr (), m_tParams.m_sUser.cstr (), m_tParams.m_sPass.cstr (), m_tParams.m_sDB.cstr () );
	} else
	{
		m_sOdbcDSN.SetSprintf ( "DRIVER={%s};SERVER={%s};UID={%s};PWD={%s};Database={%s}",
			sDriver.cstr (), m_tParams.m_sHost.cstr (), m_tParams.m_sUser.cstr (), m_tParams.m_sPass.cstr (), m_tParams.m_sDB.cstr () );
	}
}

#endif


struct RemapXSV_t
{
	int m_iAttr {-1};
	int m_iField {-1};
	int m_iTag {-1};
};


class CSphSource_BaseSV : public CSphSource_Document, public CSphSchemaConfigurator<CSphSource_BaseSV>
{
public:
	explicit		CSphSource_BaseSV ( const char * sName );
					~CSphSource_BaseSV () override;

	bool	Connect ( CSphString & sError ) override;			///< run the command and open the pipe
	void	Disconnect () override;								///< close the pipe
	const char *	DecorateMessage ( const char * sTemplate, ... ) const __attribute__ ( ( format ( printf, 2, 3 ) ) );

	bool	IterateStart ( CSphString & ) override;				///< Connect() starts getting documents automatically, so this one is empty
	BYTE **	NextDocument ( bool & bEOF, CSphString & ) override;				///< parse incoming chunk and emit some hits
	const int *	GetFieldLengths () const override { return m_dFieldLengths.Begin(); }

	bool	IterateMultivaluedStart ( int, CSphString & ) override	{ return false; }
	bool	IterateMultivaluedNext ( int64_t &, int64_t & ) override{ return false; }
	bool	IterateKillListStart ( CSphString & ) override			{ return false; }
	bool	IterateKillListNext ( DocID_t & ) override				{ return false; }

	bool	Setup ( const CSphConfigSection & hSource, FILE * pPipe, CSphString & sError );

protected:
	enum ESphParseResult
	{
		PARSING_FAILED,
		GOT_DOCUMENT,
		DATA_OVER
	};

	BYTE **					ReportDocumentError();
	virtual bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) = 0;
	virtual ESphParseResult	SplitColumns ( CSphString & ) = 0;

	CSphVector<BYTE>			m_dBuf;
	CSphFixedVector<char>		m_dError {1024};
	CSphFixedVector<int>		m_dColumnsLen {0};
	CSphVector<RemapXSV_t>		m_dRemap;

	// output
	CSphFixedVector<BYTE *>		m_dFields {0};
	CSphFixedVector<int>		m_dFieldLengths {0};

	FILE *						m_pFP = nullptr;
	int							m_iDataStart = 0;	///< where the next line to parse starts in m_dBuf
	int							m_iDocStart = 0;	///< where the last parsed document stats in m_dBuf
	int							m_iBufUsed = 0;		///< bytes [0,m_iBufUsed) are actually currently used; the rest of m_dBuf is free
	int							m_iLine = 0;
};


class CSphSource_TSV : public CSphSource_BaseSV
{
public:
	explicit				CSphSource_TSV ( const char * sName ) : CSphSource_BaseSV ( sName ) {}
	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	 bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
};


class CSphSource_CSV : public CSphSource_BaseSV
{
public:
	explicit				CSphSource_CSV ( const char * sName );
	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
	void					SetDelimiter ( const char * sDelimiter );

private:
	BYTE			m_iDelimiter;
};


CSphSource * sphCreateSourceTSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName, bool bProxy )
{
	CSphString sError;
	auto * pTSV = CreateSourceWithProxy<CSphSource_TSV> ( sSourceName, bProxy );
	if ( !pTSV->Setup ( *pSource, pPipe, sError ) )
	{
		SafeDelete ( pTSV );
		fprintf ( stdout, "ERROR: tsvpipe: %s", sError.cstr() );
	}

	return pTSV;
}


CSphSource * sphCreateSourceCSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName, bool bProxy )
{
	CSphString sError;
	const char * sDelimiter = pSource->GetStr ( "csvpipe_delimiter", "" );
	auto * pCSV = CreateSourceWithProxy<CSphSource_CSV> ( sSourceName, bProxy );
	pCSV->SetDelimiter ( sDelimiter );
	if ( !pCSV->Setup ( *pSource, pPipe, sError ) )
	{
		SafeDelete ( pCSV );
		fprintf ( stdout, "ERROR: csvpipe: %s", sError.cstr() );
	}

	return pCSV;
}


CSphSource_BaseSV::CSphSource_BaseSV ( const char * sName )
	: CSphSource_Document ( sName )
	{}



CSphSource_BaseSV::~CSphSource_BaseSV ()
{
	Disconnect();
}


bool CSphSource_BaseSV::Setup ( const CSphConfigSection & hSource, FILE * pPipe, CSphString & sError )
{
	m_pFP = pPipe;
	m_tSchema.Reset ();
	bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );

	if ( !SetupSchema ( hSource, bWordDict, sError ) )
		return false;

	if ( !SourceCheckSchema ( m_tSchema, sError ) )
		return false;

	if ( !AddAutoAttrs ( sError ) )
		return false;

	int nFields = m_tSchema.GetFieldsCount();
	m_dFields.Reset ( nFields );
	m_dFieldLengths.Reset ( nFields );

	// build hash from schema names
	SmallStringHash_T<RemapXSV_t> hSchema;

	for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		RemapXSV_t tField;
		tField.m_iField = i;
		hSchema.Add ( tField, m_tSchema.GetFieldName(i) );
	}

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphString & sAttrName = m_tSchema.GetAttr(i).m_sName;
		RemapXSV_t * pRemap = hSchema ( sAttrName );
		if ( pRemap )
			pRemap->m_iAttr = i;
		else
		{
			RemapXSV_t tAttr;
			tAttr.m_iAttr = i;
			hSchema.Add ( tAttr, sAttrName );
		}
	}

	// restore order for declared columns
	CSphString sColumn;
	hSource.IterateStart();
	while ( hSource.IterateNext() )
	{
		const CSphVariant * pVal = &hSource.IterateGet();
		while ( pVal )
		{
			sColumn = pVal->strval();
			// uint attribute might have bit count that should by cut off from name
			const char * pColon = strchr ( sColumn.cstr(), ':' );
			if ( pColon )
			{
				int iColon = pColon-sColumn.cstr();
				CSphString sTmp;
				sTmp.SetBinary ( sColumn.cstr(), iColon );
				sColumn.Swap ( sTmp );
			}

			// let's handle different char cases
			sColumn.ToLower();

			RemapXSV_t * pColumn = hSchema ( sColumn );
			assert ( !pColumn || pColumn->m_iAttr>=0 || pColumn->m_iField>=0 );
			assert ( !pColumn || pColumn->m_iTag==-1 );
			if ( pColumn )
				pColumn->m_iTag = pVal->m_iTag;

			pVal = pVal->m_pNext;
		}
	}

	RemapXSV_t * pIdCol = hSchema ( sphGetDocidName() );
	assert ( pIdCol && pIdCol->m_iTag==-1 && pIdCol->m_iAttr==0 );
	pIdCol->m_iTag = 0;

	hSchema.IterateStart();
	for ( int i=0; hSchema.IterateNext(); i++ )
		if ( hSchema.IterateGet().m_iTag>=0 )
			m_dRemap.Add ( hSchema.IterateGet() );

	m_dColumnsLen.Reset ( m_dRemap.GetLength() );

	sphSort ( m_dRemap.Begin(), m_dRemap.GetLength(), bind ( &RemapXSV_t::m_iTag ) );

	return true;
}


bool CSphSource_BaseSV::Connect ( CSphString & sError )
{
	// source settings have been updated after ::Setup
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		ESphWordpart eWordpart = GetWordpart ( m_tSchema.GetFieldName(i), m_pDict && m_pDict->GetSettings().m_bWordDict );
		m_tSchema.SetFieldWordpart ( i, eWordpart );
	}

	AllocDocinfo();

	m_tHits.m_dData.Reserve ( m_iMaxHits );
	m_dBuf.Resize ( DEFAULT_READ_BUFFER );

	return true;
}


void CSphSource_BaseSV::Disconnect()
{
	if ( m_pFP )
	{
		fclose ( m_pFP );
		m_pFP = nullptr;
	}

	m_tHits.m_dData.Reset();
}


const char * CSphSource_BaseSV::DecorateMessage ( const char * sTemplate, ... ) const
{
	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( m_dError.Begin (), m_dError.GetLength (), sTemplate, ap );
	va_end ( ap );
	return m_dError.Begin();
}

static const BYTE g_dBOM[] = { 0xEF, 0xBB, 0xBF };

bool CSphSource_BaseSV::IterateStart ( CSphString & sError )
{
	if ( !m_tSchema.GetFieldsCount() )
	{
		sError.SetSprintf ( "No fields in schema - will not index" );
		return false;
	}

	m_iLine = 0;
	m_iDataStart = 0;

	// initial buffer update
	m_iBufUsed = fread ( m_dBuf.Begin(), 1, m_dBuf.GetLength(), m_pFP );
	if ( !m_iBufUsed )
	{
		sError.SetSprintf ( "source '%s': read error '%s'", m_tSchema.GetName(), strerrorm(errno) );
		return false;
	}
	m_iPlainFieldsLength = m_tSchema.GetFieldsCount();

	// space out BOM like xml-pipe does
	if ( m_iBufUsed>(int)sizeof(g_dBOM) && memcmp ( m_dBuf.Begin(), g_dBOM, sizeof ( g_dBOM ) )==0 )
		memset ( m_dBuf.Begin(), ' ', sizeof(g_dBOM) );
	return true;
}

BYTE ** CSphSource_BaseSV::ReportDocumentError ()
{
	m_tDocInfo.m_tRowID = 0; // INVALID_ROWID means legal eof
	m_iDataStart = 0;
	m_iBufUsed = 0;
	return NULL;
}


BYTE **	CSphSource_BaseSV::NextDocument ( bool & bEOF, CSphString & sError )
{
	bEOF = false;

	ESphParseResult eRes = SplitColumns ( sError );
	if ( eRes==PARSING_FAILED )
		return ReportDocumentError();
	else if ( eRes==DATA_OVER )
	{
		bEOF = true;
		return nullptr;
	}

	assert ( eRes==GOT_DOCUMENT );

	m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
	for ( auto & i : m_dMvas )
		i.Resize(0);

	int iOff = m_iDocStart;
	ARRAY_FOREACH ( iCol, m_dRemap )
	{
		// if+if for field-string attribute case
		const RemapXSV_t & tRemap = m_dRemap[iCol];

		// field column
		if ( tRemap.m_iField!=-1 )
		{
			m_dFields[tRemap.m_iField] = m_dBuf.Begin() + iOff;
			m_dFieldLengths[tRemap.m_iField] = strlen ( (char *)m_dFields[tRemap.m_iField] );
		}

		// attribute column
		if ( tRemap.m_iAttr!=-1 )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( tRemap.m_iAttr );
			const char * sVal = (const char *)m_dBuf.Begin() + iOff;

			switch ( tAttr.m_eAttrType )
			{
			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
				m_dStrAttrs[tRemap.m_iAttr] = sVal;
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				break;

			case SPH_ATTR_FLOAT:
				m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, sphToFloat ( sVal ) );
				break;

			case SPH_ATTR_BIGINT:
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToInt64 ( sVal ) );
				break;

			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
				ParseFieldMVA ( tRemap.m_iAttr, sVal );
				break;

			case SPH_ATTR_TOKENCOUNT:
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				break;

			default:
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToDword ( sVal ) );
				break;
			}
		}

		iOff += m_dColumnsLen[iCol] + 1; // length of value plus null-terminator
	}

	m_iLine++;
	return m_dFields.Begin();
}


CSphSource_BaseSV::ESphParseResult CSphSource_TSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	BYTE * pData = m_dBuf.Begin() + m_iDataStart;
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed;
	m_iDocStart = m_iDataStart;

	while (true)
	{
		if ( iCol>=iColumns )
		{
			sError.SetSprintf ( "source '%s': too many columns found (found=%d, declared=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}

		// move to next control symbol
		while ( pData<pEnd && *pData && *pData!='\t' && *pData!='\r' && *pData!='\n' )
			pData++;

		if ( pData<pEnd )
		{
			assert ( *pData=='\t' || !*pData || *pData=='\r' || *pData=='\n' );
			bool bNull = !*pData;
			bool bEOL = ( *pData=='\r' || *pData=='\n' );

			int iLen = pData - m_dBuf.Begin() - iColumnStart;
			assert ( iLen>=0 );
			m_dColumnsLen[iCol] = iLen;
			*pData++ = '\0';
			iCol++;

			if ( bNull )
			{
				// null terminated string found
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL )
			{
				// end of document found
				// skip all EOL characters
				while ( pData<pEnd && *pData && ( *pData=='\r' || *pData=='\n' ) )
					pData++;
				break;
			}

			// column separator found
			iColumnStart = pData - m_dBuf.Begin();
			continue;
		}

		int iOff = pData - m_dBuf.Begin();

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		int iGot = fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			// error in case no data left in middle of data stream
			sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restored pointers after buffer resize
		pData = m_dBuf.Begin() + iOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( pData<=pEnd );
	m_iDataStart = pData - m_dBuf.Begin();
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_TSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	ConfigureFields ( hSource("tsvpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("tsvpipe_field_string"), bWordDict, m_tSchema );

	return true;
}


CSphSource_CSV::CSphSource_CSV ( const char * sName )
	: CSphSource_BaseSV ( sName )
{
	m_iDelimiter = BYTE ( ',' );
}


CSphSource_BaseSV::ESphParseResult CSphSource_CSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	int iQuotPrev = -1;
	int	iEscapeStart = -1;
	const BYTE * s = m_dBuf.Begin() + m_iDataStart; // parse this line
	BYTE * d = m_dBuf.Begin() + m_iDataStart; // do parsing in place
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed; // until we reach the end of current buffer
	m_iDocStart = m_iDataStart;
	bool bOnlySpace = true;
	bool bQuoted = false;
	bool bHasQuot = false;

	while (true)
	{
		assert ( d<=s );

		// move to next control symbol
		while ( s<pEnd && *s && *s!=m_iDelimiter && *s!='"' && *s!='\\' && *s!='\r' && *s!='\n' )
		{
			bOnlySpace &= sphIsSpace ( *s );
			*d++ = *s++;
		}

		if ( s<pEnd )
		{
			assert ( !*s || *s==m_iDelimiter || *s=='"' || *s=='\\' || *s=='\r' || *s=='\n' );
			bool bNull = !*s;
			bool bEOL = ( *s=='\r' || *s=='\n' );
			bool bDelimiter = ( *s==m_iDelimiter );
			bool bQuot = ( *s=='"' );
			bool bEscape = ( *s=='\\' );
			int iOff = s - m_dBuf.Begin();
			bool bEscaped = ( iEscapeStart>=0 && iEscapeStart+1==iOff );

			// escape symbol outside double quotation
			if ( !bQuoted && !bDelimiter && ( bEscape || bEscaped ) )
			{
				if ( bEscaped ) // next to escape symbol proceed as regular
				{
					*d++ = *s++;
				} else // escape just started
				{
					iEscapeStart = iOff;
					s++;
				}
				continue;
			}

			// double quote processing
			// [ " ... " ]
			// [ " ... "" ... " ]
			// [ " ... """ ]
			// [ " ... """" ... " ]
			// any symbol inside double quote proceed as regular
			// but quoted quote proceed as regular symbol
			if ( bQuot )
			{
				if ( bOnlySpace && iQuotPrev==-1 )
				{
					// enable double quote
					bQuoted = true;
					bHasQuot = true;
				} else if ( bQuoted )
				{
					// close double quote on 2st quote symbol
					bQuoted = false;
				} else if ( bHasQuot && iQuotPrev!=-1 && iQuotPrev+1==iOff )
				{
					// escaped quote found, re-enable double quote and copy symbol itself
					bQuoted = true;
					*d++ = '"';
				} else
				{
					*d++ = *s;
				}

				s++;
				iQuotPrev = iOff;
				continue;
			}

			if ( bQuoted )
			{
				*d++ = *s++;
				continue;
			}

			int iLen = d - m_dBuf.Begin() - iColumnStart;
			assert ( iLen>=0 );
			if ( iCol<m_dColumnsLen.GetLength() )
				m_dColumnsLen[iCol] = iLen;
			*d++ = '\0';
			s++;
			iCol++;

			if ( bNull ) // null terminated string found
			{
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL ) // end of document found
			{
				// skip all EOL characters
				while ( s<pEnd && *s && ( *s=='\r' || *s=='\n' ) )
					s++;
				break;
			}

			assert ( bDelimiter );
			// column separator found
			iColumnStart = d - m_dBuf.Begin();
			bOnlySpace = true;
			bQuoted = false;
			bHasQuot = false;
			iQuotPrev = -1;
			continue;
		}

		/////////////////////
		// read in more data
		/////////////////////

		int iDstOff = s - m_dBuf.Begin();
		int iSrcOff = d - m_dBuf.Begin();

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iDstOff -= m_iDataStart;
			iSrcOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			if ( iQuotPrev!=-1 )
				iQuotPrev -= m_iDataStart;
			iEscapeStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		int iGot = fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			if ( iCol!=iColumns )
				sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d, error='%s')",	m_tSchema.GetName(), iCol, iColumns, m_iLine, strerror(errno) );
			else
			{
				// error in case no data left in middle of data stream
				sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			}
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restore pointers because of the resize
		s = m_dBuf.Begin() + iDstOff;
		d = m_dBuf.Begin() + iSrcOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( s<=pEnd );
	m_iDataStart = s - m_dBuf.Begin();
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_CSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	ConfigureFields ( hSource("csvpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("csvpipe_field_string"), bWordDict, m_tSchema );

	return true;
}


void CSphSource_CSV::SetDelimiter ( const char * sDelimiter )
{
	if ( sDelimiter && *sDelimiter )
		m_iDelimiter = *sDelimiter;
}


/////////////////////////////////////////////////////////////////////////////


void sphSetJsonOptions ( bool bStrict, bool bAutoconvNumbers, bool bKeynamesToLowercase )
{
	g_bJsonStrict = bStrict;
	g_bJsonAutoconvNumbers = bAutoconvNumbers;
	g_bJsonKeynamesToLowercase = bKeynamesToLowercase;
}


static inline float GetPercent ( int64_t a, int64_t b )
{
	if ( b==0 )
		return 100.0f;

	int64_t r = a*100000/b;
	return float(r)/1000;
}


const char * CSphIndexProgress::BuildMessage() const
{
	static char sBuf[256];
	switch ( m_ePhase )
	{
		case PHASE_COLLECT:
			snprintf ( sBuf, sizeof(sBuf), "collected " INT64_FMT " docs, %.1f MB", m_iDocuments, float(m_iBytes)/1000000.0f );
			break;

		case PHASE_SORT:
			snprintf ( sBuf, sizeof(sBuf), "sorted %.1f Mhits, %.1f%% done", float(m_iHits)/1000000, GetPercent ( m_iHits, m_iHitsTotal ) );
			break;

		case PHASE_MERGE:
			snprintf ( sBuf, sizeof(sBuf), "merged %.1f Kwords", float(m_iWords)/1000 );
			break;

		case PHASE_LOOKUP:
			snprintf ( sBuf, sizeof(sBuf), "creating lookup: %.1f Kdocs, %.1f%% done", float(m_iDocids)/1000, GetPercent ( m_iDocids, m_iDocidsTotal ) );
			break;

		case PHASE_HISTOGRAMS:
			snprintf ( sBuf, sizeof(sBuf), "creating histograms: %.1f Kdocs, %.1f%% done", float(m_iDocids)/1000, GetPercent ( m_iDocids, m_iDocidsTotal ) );
			break;

		default:
			assert ( 0 && "internal error: unhandled progress phase" );
			snprintf ( sBuf, sizeof(sBuf), "(progress-phase-%d)", m_ePhase );
			break;
	}

	sBuf[sizeof(sBuf)-1] = '\0';
	return sBuf;
}


void CSphIndexProgress::Show ( bool bPhaseEnd ) const
{
	if ( m_fnProgress )
		m_fnProgress ( this, bPhaseEnd );
}

//////////////////////////////////////////////////////////////////////////

uint64_t sphCalcLocatorHash ( const CSphAttrLocator & tLoc, uint64_t uPrevHash )
{
	uint64_t uHash = sphFNV64 ( &tLoc.m_bDynamic, sizeof(tLoc.m_bDynamic), uPrevHash );
	uHash = sphFNV64 ( &tLoc.m_iBitCount, sizeof(tLoc.m_iBitCount), uHash );
	return sphFNV64 ( &tLoc.m_iBitOffset, sizeof(tLoc.m_iBitOffset), uHash );
}

uint64_t sphCalcExprDepHash ( const char * szTag, ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	uint64_t uHash = sphFNV64 ( szTag, strlen(szTag), uPrevHash );
	return sphCalcExprDepHash ( pExpr, tSorterSchema, uHash, bDisable );
}

uint64_t sphCalcExprDepHash ( ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	CSphVector<int> dCols;
	pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCols );

	uint64_t uHash = uPrevHash;
	ARRAY_FOREACH ( i, dCols )
	{
		const CSphColumnInfo & tCol = tSorterSchema.GetAttr ( dCols[i] );
		if ( tCol.m_pExpr )
		{
			// one more expression
			uHash = tCol.m_pExpr->GetHash ( tSorterSchema, uHash, bDisable );
			if ( bDisable )
				return 0;
		} else
		{
			uHash = sphCalcLocatorHash ( tCol.m_tLocator, uHash ); // plain column, add locator to hash
		}
	}

	return uHash;
}

/////////////////////////////////////////////////////////////////////////////

int sphDictCmp ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 )
{
	assert ( pStr1 && pStr2 );
	assert ( iLen1 && iLen2 );
	const int iCmpLen = Min ( iLen1, iLen2 );
	return memcmp ( pStr1, pStr2, iCmpLen );
}

int sphDictCmpStrictly ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 )
{
	assert ( pStr1 && pStr2 );
	assert ( iLen1 && iLen2 );
	const int iCmpLen = Min ( iLen1, iLen2 );
	const int iCmpRes = memcmp ( pStr1, pStr2, iCmpLen );
	return iCmpRes==0 ? iLen1-iLen2 : iCmpRes;
}


CWordlist::~CWordlist ()
{
	Reset();
}


void CWordlist::Reset ()
{
	m_tBuf.Reset ();
	m_dCheckpoints.Reset ( 0 );
	m_pWords.Reset ( 0 );
	SafeDeleteArray ( m_pInfixBlocksWords );
	m_tMapedCpReader.Reset();
}


class CheckpointReader_c : public ISphCheckpointReader
{
public:
	CheckpointReader_c()
	{
		m_iSrcStride = 0;
		m_iSrcStride += 2*sizeof(SphOffset_t);
	}

	const BYTE * ReadEntry ( const BYTE * pBuf, CSphWordlistCheckpoint & tCP ) const
	{
		tCP.m_uWordID = (SphWordID_t)sphUnalignedRead ( *(SphOffset_t *)pBuf );
		pBuf += sizeof(SphOffset_t);

		tCP.m_iWordlistOffset = sphUnalignedRead ( *(SphOffset_t *)pBuf );
		pBuf += sizeof(SphOffset_t);

		return pBuf;
	}
};


struct MappedCheckpoint_fn : public ISphNoncopyable
{
	const CSphWordlistCheckpoint *	m_pDstStart;
	const BYTE *					m_pSrcStart;
	const ISphCheckpointReader *	m_pReader;

	MappedCheckpoint_fn ( const CSphWordlistCheckpoint * pDstStart, const BYTE * pSrcStart, const ISphCheckpointReader * pReader )
		: m_pDstStart ( pDstStart )
		, m_pSrcStart ( pSrcStart )
		, m_pReader ( pReader )
	{}

	CSphWordlistCheckpoint operator() ( const CSphWordlistCheckpoint * pCP ) const
	{
		assert ( m_pDstStart<=pCP );
		const BYTE * pCur = ( pCP - m_pDstStart ) * m_pReader->m_iSrcStride + m_pSrcStart;
		CSphWordlistCheckpoint tEntry;
		m_pReader->ReadEntry ( pCur, tEntry );
		return tEntry;
	}
};


bool CWordlist::Preread ( const CSphString & sName, bool bWordDict, int iSkiplistBlockSize, CSphString & sError )
{
	assert ( m_iDictCheckpointsOffset>0 );

	m_bWordDict = bWordDict;
	m_iWordsEnd = m_iDictCheckpointsOffset; // set wordlist end
	m_iSkiplistBlockSize = iSkiplistBlockSize;

	////////////////////////////
	// preload word checkpoints
	////////////////////////////

	////////////////////////////
	// fast path for CRC checkpoints - just maps data and use inplace CP reader
	if ( !bWordDict )
	{
		if ( !m_tBuf.Setup ( sName, sError ) )
			return false;

		m_tMapedCpReader = new CheckpointReader_c;
		return true;
	}

	////////////////////////////
	// regular path that loads checkpoints data

	CSphAutoreader tReader;
	if ( !tReader.Open ( sName, sError ) )
		return false;

	int64_t iFileSize = tReader.GetFilesize();

	int iCheckpointOnlySize = (int)(iFileSize-m_iDictCheckpointsOffset);
	if ( m_iInfixCodepointBytes && m_iInfixBlocksOffset )
		iCheckpointOnlySize = (int)(m_iInfixBlocksOffset - strlen ( g_sTagInfixBlocks ) - m_iDictCheckpointsOffset);

	if ( iFileSize-m_iDictCheckpointsOffset>=UINT_MAX )
	{
		sError.SetSprintf ( "dictionary meta overflow: meta size=" INT64_FMT ", total size=" INT64_FMT ", meta offset=" INT64_FMT,
			iFileSize-m_iDictCheckpointsOffset, iFileSize, (int64_t)m_iDictCheckpointsOffset );
		return false;
	}

	tReader.SeekTo ( m_iDictCheckpointsOffset, iCheckpointOnlySize );

	assert ( m_bWordDict );
	int iArenaSize = iCheckpointOnlySize
		- (sizeof(DWORD)+sizeof(SphOffset_t))*m_dCheckpoints.GetLength()
		+ sizeof(BYTE)*m_dCheckpoints.GetLength();
	assert ( iArenaSize>=0 );
	m_pWords.Reset ( iArenaSize );

	BYTE * pWord = m_pWords.Begin();
	for ( auto & dCheckpoint : m_dCheckpoints )
	{
		dCheckpoint.m_sWord = (char *)pWord;

		const int iLen = tReader.GetDword();
		assert ( iLen>0 );
		assert ( iLen + 1 + ( pWord - m_pWords.Begin() )<=iArenaSize );
		tReader.GetBytes ( pWord, iLen );
		pWord[iLen] = '\0';
		pWord += iLen+1;

		dCheckpoint.m_iWordlistOffset = tReader.GetOffset();
	}

	////////////////////////
	// preload infix blocks
	////////////////////////

	if ( m_iInfixCodepointBytes && m_iInfixBlocksOffset )
	{
		// reading to vector as old version doesn't store total infix words length
		CSphTightVector<BYTE> dInfixWords;
		dInfixWords.Reserve ( (int)m_iInfixBlocksWordsSize );

		tReader.SeekTo ( m_iInfixBlocksOffset, (int)(iFileSize-m_iInfixBlocksOffset) );
		m_dInfixBlocks.Resize ( tReader.UnzipInt() );
		for ( auto & dInfixBlock : m_dInfixBlocks )
		{
			int iBytes = tReader.UnzipInt();

			int iOff = dInfixWords.GetLength();
			dInfixBlock.m_iInfixOffset = (DWORD) iOff; /// FIXME! name convention of m_iInfixOffset
			dInfixWords.Resize ( iOff+iBytes+1 );

			tReader.GetBytes ( dInfixWords.Begin()+iOff, iBytes );
			dInfixWords[iOff+iBytes] = '\0';

			dInfixBlock.m_iOffset = tReader.UnzipInt();
		}

		// fix-up offset to pointer
		m_pInfixBlocksWords = dInfixWords.LeakData();
		ARRAY_FOREACH ( i, m_dInfixBlocks )
			m_dInfixBlocks[i].m_sInfix = (const char *)m_pInfixBlocksWords + m_dInfixBlocks[i].m_iInfixOffset;

		// FIXME!!! store and load that explicitly
		if ( m_dInfixBlocks.GetLength() )
			m_iWordsEnd = m_dInfixBlocks.Begin()->m_iOffset - strlen ( g_sTagInfixEntries );
		else
			m_iWordsEnd -= strlen ( g_sTagInfixEntries );
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	tReader.Close();

	// mapping up only wordlist without meta (checkpoints, infixes, etc)
	if ( !m_tBuf.Setup ( sName, sError ) )
		return false;

	return true;
}


void CWordlist::DebugPopulateCheckpoints()
{
	if ( !m_tMapedCpReader.Ptr() )
		return;

	const BYTE * pCur = m_tBuf.GetWritePtr() + m_iDictCheckpointsOffset;
	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		pCur = m_tMapedCpReader->ReadEntry ( pCur, m_dCheckpoints[i] );
	}

	m_tMapedCpReader.Reset();
}


const CSphWordlistCheckpoint * CWordlist::FindCheckpoint ( const char * sWord, int iWordLen, SphWordID_t iWordID, bool bStarMode ) const
{
	if ( m_tMapedCpReader.Ptr() ) // FIXME!!! fall to regular checkpoints after data got read
	{
		MappedCheckpoint_fn tPred ( m_dCheckpoints.Begin(), m_tBuf.GetWritePtr() + m_iDictCheckpointsOffset, m_tMapedCpReader.Ptr() );
		return sphSearchCheckpoint ( sWord, iWordLen, iWordID, bStarMode, m_bWordDict, m_dCheckpoints.Begin(), &m_dCheckpoints.Last(), tPred );
	} else
	{
		return sphSearchCheckpoint ( sWord, iWordLen, iWordID, bStarMode, m_bWordDict, m_dCheckpoints.Begin(), &m_dCheckpoints.Last() );
	}
}


KeywordsBlockReader_c::KeywordsBlockReader_c ( const BYTE * pBuf, int iSkiplistBlockSize )
	: m_iSkiplistBlockSize ( iSkiplistBlockSize )
{
	Reset ( pBuf );
}


void KeywordsBlockReader_c::Reset ( const BYTE * pBuf )
{
	m_pBuf = pBuf;
	m_sWord[0] = '\0';
	m_iLen = 0;
	m_sKeyword = m_sWord;
}


bool KeywordsBlockReader_c::UnpackWord()
{
	if ( !m_pBuf )
		return false;

	assert ( m_iSkiplistBlockSize>0 );

	// unpack next word
	// must be in sync with DictEnd()!
	BYTE uPack = *m_pBuf++;
	if ( !uPack )
	{
		// ok, this block is over
		m_pBuf = NULL;
		m_iLen = 0;
		return false;
	}

	int iMatch, iDelta;
	if ( uPack & 0x80 )
	{
		iDelta = ( ( uPack>>4 ) & 7 ) + 1;
		iMatch = uPack & 15;
	} else
	{
		iDelta = uPack & 127;
		iMatch = *m_pBuf++;
	}

	assert ( iMatch+iDelta<(int)sizeof(m_sWord)-1 );
	assert ( iMatch<=(int)strlen ( (char *)m_sWord ) );

	memcpy ( m_sWord + iMatch, m_pBuf, iDelta );
	m_pBuf += iDelta;

	m_iLen = iMatch + iDelta;
	m_sWord[m_iLen] = '\0';

	m_iDoclistOffset = sphUnzipOffset ( m_pBuf );
	m_iDocs = sphUnzipInt ( m_pBuf );
	m_iHits = sphUnzipInt ( m_pBuf );
	m_uHint = ( m_iDocs>=DOCLIST_HINT_THRESH ) ? *m_pBuf++ : 0;
	m_iDoclistHint = DoclistHintUnpack ( m_iDocs, m_uHint );
	if ( m_iDocs > m_iSkiplistBlockSize )
		m_iSkiplistOffset = sphUnzipInt ( m_pBuf );
	else
		m_iSkiplistOffset = 0;

	assert ( m_iLen>0 );
	return true;
}


bool CWordlist::GetWord ( const BYTE * pBuf, SphWordID_t iWordID, CSphDictEntry & tWord ) const
{
	SphWordID_t iLastID = 0;
	SphOffset_t uLastOff = 0;

	while (true)
	{
		// unpack next word ID
		const SphWordID_t iDeltaWord = sphUnzipWordid ( pBuf ); // FIXME! slow with 32bit wordids

		if ( iDeltaWord==0 ) // wordlist chunk is over
			return false;

		iLastID += iDeltaWord;

		// list is sorted, so if there was no match, there's no such word
		if ( iLastID>iWordID )
			return false;

		// unpack next offset
		const SphOffset_t iDeltaOffset = sphUnzipOffset ( pBuf );
		uLastOff += iDeltaOffset;

		// unpack doc/hit count
		const int iDocs = sphUnzipInt ( pBuf );
		const int iHits = sphUnzipInt ( pBuf );
		SphOffset_t iSkiplistPos = 0;
		if ( iDocs > m_iSkiplistBlockSize )
			iSkiplistPos = sphUnzipOffset ( pBuf );

		assert ( iDeltaOffset );
		assert ( iDocs );
		assert ( iHits );

		// it matches?!
		if ( iLastID==iWordID )
		{
			sphUnzipWordid ( pBuf ); // might be 0 at checkpoint
			const SphOffset_t iDoclistLen = sphUnzipOffset ( pBuf );

			tWord.m_iDoclistOffset = uLastOff;
			tWord.m_iDocs = iDocs;
			tWord.m_iHits = iHits;
			tWord.m_iDoclistHint = (int)iDoclistLen;
			tWord.m_iSkiplistOffset = iSkiplistPos;
			return true;
		}
	}
}

const BYTE * CWordlist::AcquireDict ( const CSphWordlistCheckpoint * pCheckpoint ) const
{
	assert ( pCheckpoint );
	assert ( m_dCheckpoints.GetLength() );
	assert ( pCheckpoint>=m_dCheckpoints.Begin() && pCheckpoint<=&m_dCheckpoints.Last() );

	SphOffset_t iOff = pCheckpoint->m_iWordlistOffset;
	if ( m_tMapedCpReader.Ptr() )
	{
		MappedCheckpoint_fn tPred ( m_dCheckpoints.Begin(), m_tBuf.GetWritePtr() + m_iDictCheckpointsOffset, m_tMapedCpReader.Ptr() );
		iOff = tPred ( pCheckpoint ).m_iWordlistOffset;
	}

	assert ( !m_tBuf.IsEmpty() );
	assert ( iOff>0 && iOff<(int64_t)m_tBuf.GetLengthBytes() );

	return m_tBuf.GetWritePtr()+iOff;
}


ISphWordlist::Args_t::Args_t ( bool bPayload, int iExpansionLimit, bool bHasExactForms, ESphHitless eHitless, const void * pIndexData )
	: m_bPayload ( bPayload )
	, m_iExpansionLimit ( iExpansionLimit )
	, m_bHasExactForms ( bHasExactForms )
	, m_eHitless ( eHitless )
	, m_pIndexData ( pIndexData )
{
	m_sBuf.Reserve ( 2048 * SPH_MAX_WORD_LEN * 3 );
	m_dExpanded.Reserve ( 2048 );
	m_pPayload = NULL;
	m_iTotalDocs = 0;
	m_iTotalHits = 0;
}


ISphWordlist::Args_t::~Args_t ()
{
	SafeDelete ( m_pPayload );
}


void ISphWordlist::Args_t::AddExpanded ( const BYTE * sName, int iLen, int iDocs, int iHits )
{
	SphExpanded_t & tExpanded = m_dExpanded.Add();
	tExpanded.m_iDocs = iDocs;
	tExpanded.m_iHits = iHits;
	int iOff = m_sBuf.GetLength();
	tExpanded.m_iNameOff = iOff;

	m_sBuf.Resize ( iOff + iLen + 1 );
	memcpy ( m_sBuf.Begin()+iOff, sName, iLen );
	m_sBuf[iOff+iLen] = '\0';
}


const char * ISphWordlist::Args_t::GetWordExpanded ( int iIndex ) const
{
	assert ( m_dExpanded[iIndex].m_iNameOff<m_sBuf.GetLength() );
	return (const char *)m_sBuf.Begin() + m_dExpanded[iIndex].m_iNameOff;
}


struct DiskExpandedEntry_t
{
	int		m_iNameOff;
	int		m_iDocs;
	int		m_iHits;
};

struct DiskExpandedPayload_t
{
	int			m_iDocs;
	int			m_iHits;
	uint64_t	m_uDoclistOff;
	int			m_iDoclistHint;
};


struct DictEntryDiskPayload_t
{
	explicit DictEntryDiskPayload_t ( bool bPayload, ESphHitless eHitless )
	{
		m_bPayload = bPayload;
		m_eHitless = eHitless;
		if ( bPayload )
			m_dWordPayload.Reserve ( 1000 );

		m_dWordExpand.Reserve ( 1000 );
		m_dWordBuf.Reserve ( 8096 );
	}

	void Add ( const CSphDictEntry & tWord, int iWordLen )
	{
		if ( !m_bPayload || !sphIsExpandedPayload ( tWord.m_iDocs, tWord.m_iHits ) ||
			m_eHitless==SPH_HITLESS_ALL || ( m_eHitless==SPH_HITLESS_SOME && ( tWord.m_iDocs & HITLESS_DOC_FLAG )!=0 ) ) // FIXME!!! do we need hitless=some as payloads?
		{
			DiskExpandedEntry_t & tExpand = m_dWordExpand.Add();

			int iOff = m_dWordBuf.GetLength();
			tExpand.m_iNameOff = iOff;
			tExpand.m_iDocs = tWord.m_iDocs;
			tExpand.m_iHits = tWord.m_iHits;
			m_dWordBuf.Resize ( iOff + iWordLen + 1 );
			memcpy ( m_dWordBuf.Begin() + iOff + 1, tWord.m_sKeyword, iWordLen );
			m_dWordBuf[iOff] = (BYTE)iWordLen;

		} else
		{
			DiskExpandedPayload_t & tExpand = m_dWordPayload.Add();
			tExpand.m_iDocs = tWord.m_iDocs;
			tExpand.m_iHits = tWord.m_iHits;
			tExpand.m_uDoclistOff = tWord.m_iDoclistOffset;
			tExpand.m_iDoclistHint = tWord.m_iDoclistHint;
		}
	}

	void Convert ( ISphWordlist::Args_t & tArgs )
	{
		if ( !m_dWordExpand.GetLength() && !m_dWordPayload.GetLength() )
			return;

		int iTotalDocs = 0;
		int iTotalHits = 0;
		if ( m_dWordExpand.GetLength() )
		{
			LimitExpanded ( tArgs.m_iExpansionLimit, m_dWordExpand );

			const BYTE * sBase = m_dWordBuf.Begin();
			ARRAY_FOREACH ( i, m_dWordExpand )
			{
				const DiskExpandedEntry_t & tCur = m_dWordExpand[i];
				int iDocs = tCur.m_iDocs;

				if ( m_eHitless==SPH_HITLESS_SOME )
					iDocs = ( tCur.m_iDocs & HITLESS_DOC_MASK );

				tArgs.AddExpanded ( sBase + tCur.m_iNameOff + 1, sBase[tCur.m_iNameOff], iDocs, tCur.m_iHits );

				iTotalDocs += iDocs;
				iTotalHits += tCur.m_iHits;
			}
		}

		if ( m_dWordPayload.GetLength() )
		{
			LimitExpanded ( tArgs.m_iExpansionLimit, m_dWordPayload );

			DiskSubstringPayload_t * pPayload = new DiskSubstringPayload_t ( m_dWordPayload.GetLength() );
			// sorting by ascending doc-list offset gives some (15%) speed-up too
			sphSort ( m_dWordPayload.Begin(), m_dWordPayload.GetLength(), bind ( &DiskExpandedPayload_t::m_uDoclistOff ) );

			ARRAY_FOREACH ( i, m_dWordPayload )
			{
				const DiskExpandedPayload_t & tCur = m_dWordPayload[i];
				assert ( m_eHitless==SPH_HITLESS_NONE || ( m_eHitless==SPH_HITLESS_SOME && ( tCur.m_iDocs & HITLESS_DOC_FLAG )==0 ) );

				iTotalDocs += tCur.m_iDocs;
				iTotalHits += tCur.m_iHits;
				pPayload->m_dDoclist[i].m_uOff = tCur.m_uDoclistOff;
				pPayload->m_dDoclist[i].m_iLen = tCur.m_iDoclistHint;
			}

			pPayload->m_iTotalDocs = iTotalDocs;
			pPayload->m_iTotalHits = iTotalHits;
			tArgs.m_pPayload = pPayload;
		}
		tArgs.m_iTotalDocs = iTotalDocs;
		tArgs.m_iTotalHits = iTotalHits;
	}

	// sort expansions by frequency desc
	// clip the less frequent ones if needed, as they are likely misspellings
	template < typename T >
	void LimitExpanded ( int iExpansionLimit, CSphVector<T> & dVec ) const
	{
		if ( !iExpansionLimit || dVec.GetLength()<=iExpansionLimit )
			return;

		sphSort ( dVec.Begin(), dVec.GetLength(), ExpandedOrderDesc_T<T>() );
		dVec.Resize ( iExpansionLimit );
	}

	bool								m_bPayload;
	ESphHitless							m_eHitless;
	CSphVector<DiskExpandedEntry_t>		m_dWordExpand;
	CSphVector<DiskExpandedPayload_t>	m_dWordPayload;
	CSphVector<BYTE>					m_dWordBuf;
};


void CWordlist::GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	assert ( sSubstring && *sSubstring && iSubLen>0 );

	// empty index?
	if ( !m_dCheckpoints.GetLength() )
		return;

	DictEntryDiskPayload_t tDict2Payload ( tArgs.m_bPayload, tArgs.m_eHitless );

	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

	const CSphWordlistCheckpoint * pCheckpoint = FindCheckpoint ( sSubstring, iSubLen, 0, true );
	const int iSkipMagic = ( BYTE(*sSubstring)<0x20 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	while ( pCheckpoint )
	{
		// decode wordlist chunk
		KeywordsBlockReader_c tDictReader ( AcquireDict ( pCheckpoint ), m_iSkiplistBlockSize );
		while ( tDictReader.UnpackWord() )
		{
			// block is sorted
			// so once keywords are greater than the prefix, no more matches
			int iCmp = sphDictCmp ( sSubstring, iSubLen, (const char *)tDictReader.m_sKeyword, tDictReader.GetWordLen() );
			if ( iCmp<0 )
				break;

			if ( sphInterrupted() )
				break;

			// does it match the prefix *and* the entire wildcard?
			if ( iCmp==0 && sphWildcardMatch ( (const char *)tDictReader.m_sKeyword + iSkipMagic, sWildcard, pWildcard ) )
				tDict2Payload.Add ( tDictReader, tDictReader.GetWordLen() );
		}

		if ( sphInterrupted () )
			break;

		pCheckpoint++;
		if ( pCheckpoint > &m_dCheckpoints.Last() )
			break;

		if ( sphDictCmp ( sSubstring, iSubLen, pCheckpoint->m_sWord, strlen ( pCheckpoint->m_sWord ) )<0 )
			break;
	}

	tDict2Payload.Convert ( tArgs );
}

bool operator < ( const InfixBlock_t & a, const char * b )
{
	return strcmp ( a.m_sInfix, b )<0;
}

bool operator == ( const InfixBlock_t & a, const char * b )
{
	return strcmp ( a.m_sInfix, b )==0;
}

bool operator < ( const char * a, const InfixBlock_t & b )
{
	return strcmp ( a, b.m_sInfix )<0;
}


bool sphLookupInfixCheckpoints ( const char * sInfix, int iBytes, const BYTE * pInfixes, const CSphVector<InfixBlock_t> & dInfixBlocks, int iInfixCodepointBytes, CSphVector<DWORD> & dCheckpoints )
{
	assert ( pInfixes );

	char dInfixBuf[3*SPH_MAX_WORD_LEN+4];
	memcpy ( dInfixBuf, sInfix, iBytes );
	dInfixBuf[iBytes] = '\0';

	// lookup block
	int iBlock = FindSpan ( dInfixBlocks, dInfixBuf );
	if ( iBlock<0 )
		return false;
	const BYTE * pBlock = pInfixes + dInfixBlocks[iBlock].m_iOffset;

	// decode block and check for exact infix match
	// block entry is { byte edit_code, byte[] key_append, zint data_len, zint data_deltas[] }
	// zero edit_code marks block end
	BYTE sKey[32];
	while (true)
	{
		// unpack next key
		int iCode = *pBlock++;
		if ( !iCode )
			break;

		BYTE * pOut = sKey;
		if ( iInfixCodepointBytes==1 )
		{
			pOut = sKey + ( iCode>>4 );
			iCode &= 15;
			while ( iCode-- )
				*pOut++ = *pBlock++;
		} else
		{
			int iKeep = ( iCode>>4 );
			while ( iKeep-- )
				pOut += sphUtf8CharBytes ( *pOut ); ///< wtf? *pOut (=sKey) is NOT initialized?
			assert ( pOut-sKey<=(int)sizeof(sKey) );
			iCode &= 15;
			while ( iCode-- )
			{
				int i = sphUtf8CharBytes ( *pBlock );
				while ( i-- )
					*pOut++ = *pBlock++;
			}
			assert ( pOut-sKey<=(int)sizeof(sKey) );
		}
		assert ( pOut-sKey<(int)sizeof(sKey) );
#ifndef NDEBUG
		*pOut = '\0'; // handy for debugging, but not used for real matching
#endif

		if ( pOut==sKey+iBytes && memcmp ( sKey, dInfixBuf, iBytes )==0 )
		{
			// found you! decompress the data
			int iLast = 0;
			int iPackedLen = sphUnzipInt ( pBlock );
			const BYTE * pMax = pBlock + iPackedLen;
			while ( pBlock<pMax )
			{
				iLast += sphUnzipInt ( pBlock );
				dCheckpoints.Add ( (DWORD)iLast );
			}
			return true;
		}

		int iSkip = sphUnzipInt ( pBlock );
		pBlock += iSkip;
	}
	return false;
}


// calculate length, upto iInfixCodepointBytes chars from infix start
int sphGetInfixLength ( const char * sInfix, int iBytes, int iInfixCodepointBytes )
{
	int iBytes1 = Min ( 6, iBytes );
	if ( iInfixCodepointBytes!=1 )
	{
		int iCharsLeft = 6;
		const char * s = sInfix;
		const char * sMax = sInfix + iBytes;
		while ( iCharsLeft-- && s<sMax )
			s += sphUtf8CharBytes(*s);
		iBytes1 = (int)( s - sInfix );
	}

	return iBytes1;
}


void CWordlist::GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	// dict must be of keywords type, and fully cached
	// mmap()ed in the worst case, should we ever banish it to disk again
	if ( m_tBuf.IsEmpty() || !m_dCheckpoints.GetLength() )
		return;

	assert ( !m_tMapedCpReader.Ptr() );

	// extract key1, upto 6 chars from infix start
	int iBytes1 = sphGetInfixLength ( sSubstring, iSubLen, m_iInfixCodepointBytes );

	// lookup key1
	// OPTIMIZE? maybe lookup key2 and reduce checkpoint set size, if possible?
	CSphVector<DWORD> dPoints;
	if ( !sphLookupInfixCheckpoints ( sSubstring, iBytes1, m_tBuf.GetWritePtr(), m_dInfixBlocks, m_iInfixCodepointBytes, dPoints ) )
		return;

	DictEntryDiskPayload_t tDict2Payload ( tArgs.m_bPayload, tArgs.m_eHitless );
	const int iSkipMagic = ( tArgs.m_bHasExactForms ? 1 : 0 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker

	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

	// walk those checkpoints, check all their words
	ARRAY_FOREACH ( i, dPoints )
	{
		// OPTIMIZE? add a quicker path than a generic wildcard for "*infix*" case?
		KeywordsBlockReader_c tDictReader ( m_tBuf.GetWritePtr() + m_dCheckpoints[dPoints[i]-1].m_iWordlistOffset, m_iSkiplistBlockSize );
		while ( tDictReader.UnpackWord() )
		{
			if ( sphInterrupted () )
				break;

			// stemmed terms should not match suffixes
			if ( tArgs.m_bHasExactForms && *tDictReader.m_sKeyword!=MAGIC_WORD_HEAD_NONSTEMMED )
				continue;

			if ( sphWildcardMatch ( (const char *)tDictReader.m_sKeyword+iSkipMagic, sWildcard, pWildcard ) )
				tDict2Payload.Add ( tDictReader, tDictReader.GetWordLen() );
		}

		if ( sphInterrupted () )
			break;
	}

	tDict2Payload.Convert ( tArgs );
}

static int BuildUtf8Offsets ( const char * sWord, int iLen, int * pOff, int DEBUGARG ( iBufSize ) )
{
	const BYTE * s = (const BYTE *)sWord;
	const BYTE * sEnd = s + iLen;
	int * pStartOff = pOff;
	*pOff = 0;
	pOff++;
	while ( s<sEnd )
	{
		sphUTF8Decode ( s );
		*pOff = s-(const BYTE *)sWord;
		pOff++;
	}
	assert ( pOff-pStartOff<iBufSize );
	return pOff - pStartOff - 1;
}

void sphBuildNGrams ( const char * sWord, int iLen, char cDelimiter, CSphVector<char> & dNGrams )
{
	int dOff[SPH_MAX_WORD_LEN+1];
	int iCodepoints = BuildUtf8Offsets ( sWord, iLen, dOff, sizeof ( dOff ) );
	if ( iCodepoints<3 )
		return;

	dNGrams.Reserve ( iLen*3 );
	for ( int iChar=0; iChar<=iCodepoints-3; iChar++ )
	{
		int iStart = dOff[iChar];
		int iEnd = dOff[iChar+3];
		int iGramLen = iEnd - iStart;

		char * sDst = dNGrams.AddN ( iGramLen + 1 );
		memcpy ( sDst, sWord+iStart, iGramLen );
		sDst[iGramLen] = cDelimiter;
	}
	// n-grams split by delimiter
	// however it's still null terminated
	dNGrams.Last() = '\0';
}

template <typename T>
int sphLevenshtein ( const T * sWord1, int iLen1, const T * sWord2, int iLen2 )
{
	if ( !iLen1 )
		return iLen2;
	if ( !iLen2 )
		return iLen1;

	int dTmp [ 3*SPH_MAX_WORD_LEN+1 ]; // FIXME!!! remove extra length after utf8->codepoints conversion

	for ( int i=0; i<=iLen2; i++ )
		dTmp[i] = i;

	for ( int i=0; i<iLen1; i++ )
	{
		dTmp[0] = i+1;
		int iWord1 = sWord1[i];
		int iDist = i;

		for ( int j=0; j<iLen2; j++ )
		{
			int iDistNext = dTmp[j+1];
			dTmp[j+1] = ( iWord1==sWord2[j] ? iDist : ( 1 + Min ( Min ( iDist, iDistNext ), dTmp[j] ) ) );
			iDist = iDistNext;
		}
	}

	return dTmp[iLen2];
}

int sphLevenshtein ( const char * sWord1, int iLen1, const char * sWord2, int iLen2 )
{
	return sphLevenshtein<char> ( sWord1, iLen1, sWord2, iLen2 );
}

int sphLevenshtein ( const int * sWord1, int iLen1, const int * sWord2, int iLen2 )
{
	return sphLevenshtein<int> ( sWord1, iLen1, sWord2, iLen2 );
}

// sort by distance(uLen) desc, checkpoint index(uOff) asc
struct CmpHistogram_fn
{
	inline bool IsLess ( const Slice_t & a, const Slice_t & b ) const
	{
		return ( a.m_uLen>b.m_uLen || ( a.m_uLen==b.m_uLen && a.m_uOff<b.m_uOff ) );
	}
};

// convert utf8 to unicode string
int DecodeUtf8 ( const BYTE * sWord, int * pBuf )
{
	if ( !sWord )
		return 0;

	int * pCur = pBuf;
	while ( *sWord )
	{
		*pCur = sphUTF8Decode ( sWord );
		pCur++;
	}
	return pCur - pBuf;
}


bool SuggestResult_t::SetWord ( const char * sWord, const ISphTokenizer * pTok, bool bUseLastWord )
{
	ISphTokenizerRefPtr_c pTokenizer ( pTok->Clone ( SPH_CLONE_QUERY_LIGHTWEIGHT ) );
	pTokenizer->SetBuffer ( (BYTE *)sWord, strlen ( sWord ) );

	const BYTE * pToken = pTokenizer->GetToken();
	for ( ; pToken!=NULL; )
	{
		m_sWord = (const char *)pToken;
		if ( !bUseLastWord )
			break;

		if ( pTokenizer->TokenIsBlended() )
			pTokenizer->SkipBlended();
		pToken = pTokenizer->GetToken();
	}


	m_iLen = m_sWord.Length();
	m_iCodepoints = DecodeUtf8 ( (const BYTE *)m_sWord.cstr(), m_dCodepoints );
	m_bUtf8 = ( m_iCodepoints!=m_iLen );

	bool bValidWord = ( m_iCodepoints>=3 );
	if ( bValidWord )
		sphBuildNGrams ( m_sWord.cstr(), m_iLen, '\0', m_dTrigrams );

	return bValidWord;
}

void SuggestResult_t::Flattern ( int iLimit )
{
	int iCount = Min ( m_dMatched.GetLength(), iLimit );
	m_dMatched.Resize ( iCount );
}

struct SliceInt_t
{
	int		m_iOff;
	int		m_iEnd;
};

static void SuggestGetChekpoints ( const ISphWordlistSuggest * pWordlist, int iInfixCodepointBytes, const CSphVector<char> & dTrigrams, CSphVector<Slice_t> & dCheckpoints, SuggestResult_t & tStats )
{
	CSphVector<DWORD> dWordCp; // FIXME!!! add mask that trigram matched
	// v1 - current index, v2 - end index
	CSphVector<SliceInt_t> dMergeIters;

	int iReserveLen = 0;
	int iLastLen = 0;
	const char * sTrigram = dTrigrams.Begin();
	const char * sTrigramEnd = sTrigram + dTrigrams.GetLength();
	while (true)
	{
		int iTrigramLen = strlen ( sTrigram );
		int iInfixLen = sphGetInfixLength ( sTrigram, iTrigramLen, iInfixCodepointBytes );

		// count how many checkpoint we will get
		iReserveLen = Max ( iReserveLen, dWordCp.GetLength () - iLastLen );
		iLastLen = dWordCp.GetLength();

		dMergeIters.Add().m_iOff = dWordCp.GetLength();
		pWordlist->SuffixGetChekpoints ( tStats, sTrigram, iInfixLen, dWordCp );

		sTrigram += iTrigramLen + 1;
		if ( sTrigram>=sTrigramEnd )
			break;

		if ( sphInterrupted() )
			return;
	}
	if ( !dWordCp.GetLength() )
		return;

	for ( int i=0; i<dMergeIters.GetLength()-1; i++ )
	{
		dMergeIters[i].m_iEnd = dMergeIters[i+1].m_iOff;
	}
	dMergeIters.Last().m_iEnd = dWordCp.GetLength();

	// v1 - checkpoint index, v2 - checkpoint count
	dCheckpoints.Reserve ( iReserveLen );
	dCheckpoints.Resize ( 0 );

	// merge sorting of already ordered checkpoints
	while (true)
	{
		DWORD iMinCP = UINT_MAX;
		DWORD iMinIndex = UINT_MAX;
		ARRAY_FOREACH ( i, dMergeIters )
		{
			const SliceInt_t & tElem = dMergeIters[i];
			if ( tElem.m_iOff<tElem.m_iEnd && dWordCp[tElem.m_iOff]<iMinCP )
			{
				iMinIndex = i;
				iMinCP = dWordCp[tElem.m_iOff];
			}
		}

		if ( iMinIndex==UINT_MAX )
			break;

		if ( dCheckpoints.GetLength()==0 || iMinCP!=dCheckpoints.Last().m_uOff )
		{
			dCheckpoints.Add().m_uOff = iMinCP;
			dCheckpoints.Last().m_uLen = 1;
		} else
		{
			dCheckpoints.Last().m_uLen++;
		}

		assert ( iMinIndex!=UINT_MAX && iMinCP!=UINT_MAX );
		assert ( dMergeIters[iMinIndex].m_iOff<dMergeIters[iMinIndex].m_iEnd );
		dMergeIters[iMinIndex].m_iOff++;
	}
	dCheckpoints.Sort ( CmpHistogram_fn() );
}


void CWordlist::SuffixGetChekpoints ( const SuggestResult_t & , const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const
{
	sphLookupInfixCheckpoints ( sSuffix, iLen, m_tBuf.GetWritePtr(), m_dInfixBlocks, m_iInfixCodepointBytes, dCheckpoints );
}

void CWordlist::SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const
{
	assert ( tRes.m_pWordReader );
	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	pReader->Reset ( m_tBuf.GetWritePtr() + m_dCheckpoints[iCP-1].m_iWordlistOffset );
}

bool CWordlist::ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const
{
	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	if ( !pReader->UnpackWord() )
		return false;

	tWord.m_sWord = pReader->GetWord();
	tWord.m_iLen = pReader->GetWordLen();
	tWord.m_iDocs = pReader->m_iDocs;
	return true;
}

struct CmpSuggestOrder_fn
{
	bool IsLess ( const SuggestWord_t & a, const SuggestWord_t & b ) const
	{
		if ( a.m_iDistance==b.m_iDistance )
			return a.m_iDocs>b.m_iDocs;

		return a.m_iDistance<b.m_iDistance;
	}
};

void SuggestMergeDocs ( CSphVector<SuggestWord_t> & dMatched )
{
	if ( !dMatched.GetLength() )
		return;

	dMatched.Sort ( bind ( &SuggestWord_t::m_iNameHash ) );

	int iSrc = 1;
	int iDst = 1;
	while ( iSrc<dMatched.GetLength() )
	{
		if ( dMatched[iDst-1].m_iNameHash==dMatched[iSrc].m_iNameHash )
		{
			dMatched[iDst-1].m_iDocs += dMatched[iSrc].m_iDocs;
			iSrc++;
		} else
		{
			dMatched[iDst++] = dMatched[iSrc++];
		}
	}

	dMatched.Resize ( iDst );
}

template <bool SINGLE_BYTE_CHAR>
void SuggestMatchWords ( const ISphWordlistSuggest * pWordlist, const CSphVector<Slice_t> & dCheckpoints, const SuggestArgs_t & tArgs, SuggestResult_t & tRes )
{
	// walk those checkpoints, check all their words

	const int iMinWordLen = ( tArgs.m_iDeltaLen>0 ? Max ( 0, tRes.m_iCodepoints - tArgs.m_iDeltaLen ) : -1 );
	const int iMaxWordLen = ( tArgs.m_iDeltaLen>0 ? tRes.m_iCodepoints + tArgs.m_iDeltaLen : INT_MAX );

	OpenHash_T<int, int64_t, HashFunc_Int64_t> dHashTrigrams;
	const char * sBuf = tRes.m_dTrigrams.Begin ();
	const char * sEnd = sBuf + tRes.m_dTrigrams.GetLength();
	while ( sBuf<sEnd )
	{
		dHashTrigrams.Add ( sphCRC32 ( sBuf ), 1 );
		while ( *sBuf ) sBuf++;
		sBuf++;
	}
	int dCharOffset[SPH_MAX_WORD_LEN+1];
	int dDictWordCodepoints[SPH_MAX_WORD_LEN];

	const int iQLen = Max ( tArgs.m_iQueueLen, tArgs.m_iLimit );
	const int iRejectThr = tArgs.m_iRejectThr;
	int iQueueRejected = 0;
	int iLastBad = 0;
	bool bSorted = true;
	const bool bMergeWords = tRes.m_bMergeWords;
	const bool bHasExactDict = tRes.m_bHasExactDict;
	const int iMaxEdits = tArgs.m_iMaxEdits;
	const bool bNonCharAllowed = tArgs.m_bNonCharAllowed;
	tRes.m_dMatched.Reserve ( iQLen * 2 );
	CmpSuggestOrder_fn fnCmp;

	ARRAY_FOREACH ( i, dCheckpoints )
	{
		DWORD iCP = dCheckpoints[i].m_uOff;
		pWordlist->SetCheckpoint ( tRes, iCP );

		ISphWordlistSuggest::DictWord_t tWord;
		while ( pWordlist->ReadNextWord ( tRes, tWord ) )
		{
			const char * sDictWord = tWord.m_sWord;
			int iDictWordLen = tWord.m_iLen;
			int iDictCodepoints = iDictWordLen;

			// for stemmer \ lematizer suggest should match only original words
			if ( bHasExactDict && sDictWord[0]!=MAGIC_WORD_HEAD_NONSTEMMED )
				continue;

			if ( bHasExactDict )
			{
				// skip head MAGIC_WORD_HEAD_NONSTEMMED char
				sDictWord++;
				iDictWordLen--;
				iDictCodepoints--;
			}

			if_const ( SINGLE_BYTE_CHAR )
			{
				if ( iDictWordLen<=iMinWordLen || iDictWordLen>=iMaxWordLen )
					continue;
			}

			int iChars = 0;

			const BYTE * s = (const BYTE *)sDictWord;
			const BYTE * sDictWordEnd = s + iDictWordLen;
			bool bGotNonChar = false;
			while ( !bGotNonChar && s<sDictWordEnd )
			{
				dCharOffset[iChars] = s - (const BYTE *)sDictWord;
				int iCode = sphUTF8Decode ( s );
				if ( !bNonCharAllowed )
					bGotNonChar = ( iCode<'A' || ( iCode>'Z' && iCode<'a' ) ); // skip words with any numbers or special characters

				if_const ( !SINGLE_BYTE_CHAR )
				{
					dDictWordCodepoints[iChars] = iCode;
				}
				iChars++;
			}
			dCharOffset[iChars] = s - (const BYTE *)sDictWord;
			iDictCodepoints = iChars;

			if_const ( !SINGLE_BYTE_CHAR )
			{
				if ( iDictCodepoints<=iMinWordLen || iDictCodepoints>=iMaxWordLen )
					continue;
			}

			// skip word in case of non char symbol found
			if ( bGotNonChar )
				continue;

			// FIXME!!! should we skip in such cases
			// utf8 reference word			!=	single byte dictionary word
			// single byte reference word	!=	utf8 dictionary word

			bool bGotMatch = false;
			for ( int iChar=0; iChar<=iDictCodepoints-3 && !bGotMatch; iChar++ )
			{
				int iStart = dCharOffset[iChar];
				int iEnd = dCharOffset[iChar+3];
				bGotMatch = ( dHashTrigrams.Find ( sphCRC32 ( sDictWord + iStart, iEnd - iStart ) )!=NULL );
			}

			// skip word in case of no trigrams matched
			if ( !bGotMatch )
				continue;

			int iDist = INT_MAX;
			if_const ( SINGLE_BYTE_CHAR )
				iDist = sphLevenshtein ( tRes.m_sWord.cstr(), tRes.m_iLen, sDictWord, iDictWordLen );
			else
				iDist = sphLevenshtein ( tRes.m_dCodepoints, tRes.m_iCodepoints, dDictWordCodepoints, iDictCodepoints );

			// skip word in case of too many edits
			if ( iDist>iMaxEdits )
				continue;

			SuggestWord_t tElem;
			tElem.m_iNameOff = tRes.m_dBuf.GetLength();
			tElem.m_iLen = iDictWordLen;
			tElem.m_iDistance = iDist;
			tElem.m_iDocs = tWord.m_iDocs;

			// store in k-buffer up to 2*QLen words
			if ( !iLastBad || fnCmp.IsLess ( tElem, tRes.m_dMatched[iLastBad] ) )
			{
				if ( bMergeWords )
					tElem.m_iNameHash = sphCRC32 ( sDictWord, iDictWordLen );

				tRes.m_dMatched.Add ( tElem );
				BYTE * sWord = tRes.m_dBuf.AddN ( iDictWordLen+1 );
				memcpy ( sWord, sDictWord, iDictWordLen );
				sWord[iDictWordLen] = '\0';
				iQueueRejected = 0;
				bSorted = false;
			} else
			{
				iQueueRejected++;
			}

			// sort k-buffer in case of threshold overflow
			if ( tRes.m_dMatched.GetLength()>iQLen*2 )
			{
				if ( bMergeWords )
					SuggestMergeDocs ( tRes.m_dMatched );
				int iTotal = tRes.m_dMatched.GetLength();
				tRes.m_dMatched.Sort ( CmpSuggestOrder_fn() );
				bSorted = true;

				// there might be less than necessary elements after merge operation
				if ( iTotal>iQLen )
				{
					iQueueRejected += iTotal - iQLen;
					tRes.m_dMatched.Resize ( iQLen );
				}
				iLastBad = tRes.m_dMatched.GetLength()-1;
			}
		}

		if ( sphInterrupted () )
			break;

		// stop dictionary unpacking in case queue rejects a lot of matched words
		if ( iQueueRejected && iQueueRejected>iQLen*iRejectThr )
			break;
	}

	// sort at least once or any unsorted
	if ( !bSorted )
	{
		if ( bMergeWords )
			SuggestMergeDocs ( tRes.m_dMatched );
		tRes.m_dMatched.Sort ( CmpSuggestOrder_fn() );
	}
}


void sphGetSuggest ( const ISphWordlistSuggest * pWordlist, int iInfixCodepointBytes, const SuggestArgs_t & tArgs, SuggestResult_t & tRes )
{
	assert ( pWordlist );

	CSphVector<Slice_t> dCheckpoints;
	SuggestGetChekpoints ( pWordlist, iInfixCodepointBytes, tRes.m_dTrigrams, dCheckpoints, tRes );
	if ( !dCheckpoints.GetLength() )
		return;

	if ( tRes.m_bUtf8 )
		SuggestMatchWords<false> ( pWordlist, dCheckpoints, tArgs, tRes );
	else
		SuggestMatchWords<true> ( pWordlist, dCheckpoints, tArgs, tRes );

	if ( sphInterrupted() )
		return;

	tRes.Flattern ( tArgs.m_iLimit );
}


// all indexes should produce same terms for same query
void SphWordStatChecker_t::Set ( const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hStat )
{
	m_dSrcWords.Reserve ( hStat.GetLength() );
	hStat.IterateStart();
	while ( hStat.IterateNext() )
	{
		m_dSrcWords.Add ( sphFNV64 ( hStat.IterateGetKey().cstr() ) );
	}
	m_dSrcWords.Sort();
}


void SphWordStatChecker_t::DumpDiffer ( const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hStat,
	const char * sIndex, CSphString & sWarning ) const
{
	if ( !m_dSrcWords.GetLength() )
		return;

	StringBuilder_c tWarningBuilder;
	hStat.IterateStart();
	while ( hStat.IterateNext() )
	{
		uint64_t uHash = sphFNV64 ( hStat.IterateGetKey().cstr() );
		if ( !m_dSrcWords.BinarySearch ( uHash ) )
		{
			if ( tWarningBuilder.IsEmpty () )
			{
				if ( sIndex )
					tWarningBuilder.Appendf ( "index '%s': ", sIndex );

				tWarningBuilder.Appendf ( "query word(s) mismatch: %s", hStat.IterateGetKey().cstr() );
			} else
				tWarningBuilder << ", " << hStat.IterateGetKey();
		}
	}

	if ( tWarningBuilder.GetLength() )
		tWarningBuilder.MoveTo ( sWarning );
}

//////////////////////////////////////////////////////////////////////////
// CSphQueryResultMeta
//////////////////////////////////////////////////////////////////////////

void RemoveDictSpecials ( CSphString & sWord )
{
	if ( sWord.cstr()[0]==MAGIC_WORD_HEAD )
	{
		*(char *)( sWord.cstr() ) = '*';
	} else if ( sWord.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
	{
		*(char *)( sWord.cstr() ) = '=';
	} else
	{
		const char * p = strchr ( sWord.cstr(), MAGIC_WORD_BIGRAM );
		if ( p )
		{
			*(char *)p = ' ';
		}
	}
}

const CSphString & RemoveDictSpecials ( const CSphString & sWord, CSphString & sFixed )
{
	const CSphString * pFixed = &sWord;
	if ( sWord.cstr()[0]==MAGIC_WORD_HEAD )
	{
		sFixed = sWord;
		*(char *)( sFixed.cstr() ) = '*';
		pFixed = &sFixed;
	} else if ( sWord.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
	{
		sFixed = sWord;
		*(char *)( sFixed.cstr() ) = '=';
		pFixed = &sFixed;
	} else
	{
		const char * p = strchr ( sWord.cstr(), MAGIC_WORD_BIGRAM );
		if ( p )
		{
			sFixed.SetSprintf ( "\"%s\"", sWord.cstr() );
			*( (char*)sFixed.cstr() + ( p - sWord.cstr() ) + 1 ) = ' ';
			pFixed = &sFixed;
		}
	}

	return *pFixed;
}

void CSphQueryResultMeta::AddStat ( const CSphString & sWord, int64_t iDocs, int64_t iHits )
{
	CSphString sBuf;
	const CSphString & tFixed = RemoveDictSpecials ( sWord, sBuf );
	WordStat_t & tStats = m_hWordStats.AddUnique ( tFixed );
	tStats.m_iDocs += iDocs;
	tStats.m_iHits += iHits;
}


bool CSphGlobalIDF::Touch ( const CSphString & sFilename )
{
	// update m_uMTime, return true if modified
	struct_stat tStat = {0};
	if ( stat ( sFilename.cstr(), &tStat ) < 0 )
		tStat.st_mtime = 0;
	bool bModified = ( m_uMTime!=tStat.st_mtime );
	m_uMTime = tStat.st_mtime;
	return bModified;
}


bool CSphGlobalIDF::Preread ( const CSphString & sFilename, CSphString & sError )
{
	Touch ( sFilename );

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFilename, sError ) )
		return false;

	m_iTotalDocuments = tReader.GetOffset ();
	const SphOffset_t iSize = tReader.GetFilesize () - sizeof(SphOffset_t);
	m_iTotalWords = iSize/sizeof(IDFWord_t);

	// allocate words cache
	CSphString sWarning;
	if ( !m_pWords.Alloc ( m_iTotalWords, sError ) )
		return false;

	// allocate lookup table if needed
	int iHashSize = (int)( U64C(1) << HASH_BITS );
	if ( m_iTotalWords > iHashSize*8 )
	{
		if ( !m_pHash.Alloc ( iHashSize+2, sError ) )
			return false;
	}

	// read file into memory (may exceed 2GB)
	const int iBlockSize = 10485760; // 10M block
	for ( SphOffset_t iRead=0; iRead<iSize && !sphInterrupted(); iRead+=iBlockSize )
		tReader.GetBytes ( (BYTE*)m_pWords.GetWritePtr()+iRead, iRead+iBlockSize>iSize ? (int)( iSize-iRead ) : iBlockSize );

	if ( sphInterrupted() )
		return false;

	// build lookup table
	if ( m_pHash.GetLengthBytes () )
	{
		int64_t * pHash = m_pHash.GetWritePtr();

		uint64_t uFirst = m_pWords[0].m_uWordID;
		uint64_t uRange = m_pWords[m_iTotalWords-1].m_uWordID - uFirst;

		DWORD iShift = 0;
		while ( uRange>=( U64C(1) << HASH_BITS ) )
		{
			iShift++;
			uRange >>= 1;
		}

		pHash[0] = iShift;
		pHash[1] = 0;
		DWORD uLastHash = 0;

		for ( int64_t i=1; i<m_iTotalWords; i++ )
		{
			// check for interrupt (throttled for speed)
			if ( ( i & 0xffff )==0 && sphInterrupted() )
				return false;

			DWORD uHash = (DWORD)( ( m_pWords[i].m_uWordID-uFirst ) >> iShift );

			if ( uHash==uLastHash )
				continue;

			while ( uLastHash<uHash )
				pHash [ ++uLastHash+1 ] = i;

			uLastHash = uHash;
		}
		pHash [ ++uLastHash+1 ] = m_iTotalWords;
	}
	return true;
}


DWORD CSphGlobalIDF::GetDocs ( const CSphString & sWord ) const
{
	const char * s = sWord.cstr();

	// replace = to MAGIC_WORD_HEAD_NONSTEMMED for exact terms
	char sBuf [ 3*SPH_MAX_WORD_LEN+4 ];
	if ( *s && *s=='=' )
	{
		strncpy ( sBuf, sWord.cstr(), sizeof(sBuf)-1 );
		sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
		s = sBuf;
	}

	uint64_t uWordID = sphFNV64(s);

	int64_t iStart = 0;
	int64_t iEnd = m_iTotalWords-1;

	const IDFWord_t * pWords = (IDFWord_t *)m_pWords.GetWritePtr ();

	if ( m_pHash.GetLengthBytes () )
	{
		uint64_t uFirst = pWords[0].m_uWordID;
		DWORD uHash = (DWORD)( ( uWordID-uFirst ) >> m_pHash[0] );
		if ( uHash > ( U64C(1) << HASH_BITS ) )
			return 0;

		iStart = m_pHash [ uHash+1 ];
		iEnd = m_pHash [ uHash+2 ] - 1;
	}

	const IDFWord_t * pWord = sphBinarySearch ( pWords+iStart, pWords+iEnd, bind ( &IDFWord_t::m_uWordID ), uWordID );
	return pWord ? pWord->m_iDocs : 0;
}


float CSphGlobalIDF::GetIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF )
{
	const int64_t iDocs = Max ( iDocsLocal, (int64_t)GetDocs ( sWord ) );
	const int64_t iTotalClamped = Max ( m_iTotalDocuments, iDocs );

	if ( !iDocs )
		return 0.0f;

	if ( bPlainIDF )
	{
		float fLogTotal = logf ( float ( 1+iTotalClamped ) );
		return logf ( float ( iTotalClamped-iDocs+1 ) / float ( iDocs ) )
			/ ( 2*fLogTotal );
	} else
	{
		float fLogTotal = logf ( float ( 1+iTotalClamped ) );
		return logf ( float ( iTotalClamped ) / float ( iDocs ) )
			/ ( 2*fLogTotal );
	}
}


bool sphPrereadGlobalIDF ( const CSphString & sPath, CSphString & sError )
{
	g_tGlobalIDFLock.Lock ();

	CSphGlobalIDF ** ppGlobalIDF = g_hGlobalIDFs ( sPath );
	bool bExpired = ( ppGlobalIDF && *ppGlobalIDF && (*ppGlobalIDF)->Touch ( sPath ) );

	if ( !ppGlobalIDF || bExpired )
	{
		if ( bExpired )
			sphLogDebug ( "Reloading global IDF (%s)", sPath.cstr() );
		else
			sphLogDebug ( "Loading global IDF (%s)", sPath.cstr() );

		// unlock while prereading
		g_tGlobalIDFLock.Unlock ();

		CSphGlobalIDF * pGlobalIDF = new CSphGlobalIDF ();
		if ( !pGlobalIDF->Preread ( sPath, sError ) )
		{
			SafeDelete ( pGlobalIDF );
			return false;
		}

		// lock while updating
		g_tGlobalIDFLock.Lock ();

		if ( bExpired )
		{
			ppGlobalIDF = g_hGlobalIDFs ( sPath );
			if ( ppGlobalIDF )
			{
				CSphGlobalIDF * pOld = *ppGlobalIDF;
				*ppGlobalIDF = pGlobalIDF;
				SafeDelete ( pOld );
			}
		} else
		{
			if ( !g_hGlobalIDFs.Add ( pGlobalIDF, sPath ) )
				SafeDelete ( pGlobalIDF );
		}
	}

	g_tGlobalIDFLock.Unlock ();

	return true;
}


void sphUpdateGlobalIDFs ( const StrVec_t & dFiles )
{
	// delete unlisted entries
	g_tGlobalIDFLock.Lock ();
	g_hGlobalIDFs.IterateStart ();
	while ( g_hGlobalIDFs.IterateNext () )
	{
		const CSphString & sKey = g_hGlobalIDFs.IterateGetKey ();
		if ( !dFiles.Contains ( sKey ) )
		{
			sphLogDebug ( "Unloading global IDF (%s)", sKey.cstr() );
			SafeDelete ( g_hGlobalIDFs.IterateGet () );
			g_hGlobalIDFs.Delete ( sKey );
		}
	}
	g_tGlobalIDFLock.Unlock ();

	// load/rotate remaining entries
	CSphString sError;
	ARRAY_FOREACH ( i, dFiles )
	{
		CSphString sPath = dFiles[i];
		if ( !sphPrereadGlobalIDF ( sPath, sError ) )
			sphLogDebug ( "Could not load global IDF (%s): %s", sPath.cstr(), sError.cstr() );
	}
}


void sphShutdownGlobalIDFs ()
{
	StrVec_t dEmptyFiles;
	sphUpdateGlobalIDFs ( dEmptyFiles );
}

//////////////////////////////////////////////////////////////////////////

void SorterSchemas ( ISphMatchSorter ** ppSorters, int iCount, int iSkipSorter, CSphVector<const ISphSchema *> & dSchemas )
{
	if ( iCount<2 )
		return;

	dSchemas.Reserve ( iCount - 1 );
	for ( int i=0; i<iCount; i++ )
	{
		if ( i==iSkipSorter || !ppSorters[i] )
			continue;

		const ISphSchema * pSchema = ppSorters[i]->GetSchema();
		dSchemas.Add ( pSchema );
	}
}

//////////////////////////////////////////////////////////////////////////
CSphString IndexFiles_c::FatalMsg ( const char * sMsg )
{
	CSphString sFatalMsg;
	if ( sMsg )
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s: %s", sMsg, ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "%s index '%s': %s", sMsg, m_sIndexName.cstr(), ErrorMsg() );
	} else
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s", ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "index '%s': %s", m_sIndexName.cstr (), ErrorMsg () );
	}
	return sFatalMsg;
}

CSphString IndexFiles_c::FullPath ( const char * sExt, const char* sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix, sExt );
	return sResult;
}

CSphString IndexFiles_c::MakePath ( const char * sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix );
	return sResult;
}

bool IndexFiles_c::HasAllFiles ( const char * sType )
{
	for ( const auto & dExt : g_dIndexFilesExts )
	{
		if ( m_uVersion<dExt.m_uMinVer || dExt.m_bOptional )
			continue;

		if ( !sphIsReadable ( FullPath ( dExt.m_szExt, sType ) ) )
			return false;
	}
	return true;
}

void IndexFiles_c::Unlink ( const char * sType )
{
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFile = FullPath ( dExt.m_szExt, sType );
		if ( ::unlink ( sFile.cstr() ) && !dExt.m_bOptional )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFile.cstr (), strerrorm ( errno ) );
	}
}

bool IndexFiles_c::Rename ( const char * sFromSz, const char * sToSz )  // move files between different bases
{
	m_bFatal = false;
	bool bRenamed[SPH_EXT_TOTAL] = { false };
	bool bAllOk = true;
	for ( int i = 0; i<SPH_EXT_TOTAL; i++ )
	{
		const auto & dExt = g_dIndexFilesExts[i];
		if ( m_uVersion<dExt.m_uMinVer || !dExt.m_bCopy )
			continue;

		auto sFrom = FullPath ( dExt.m_szExt, "", sFromSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sToSz );

#if USE_WINDOWS
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif

		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			// this is no reason to fail if not necessary files missed.
			if ( dExt.m_bOptional )
				continue;

			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm ( errno ) );
			bAllOk = false;
			break;
		}
		bRenamed[i] = true;
	}

	if ( bAllOk )
		return true;

	for ( int i = 0; i<SPH_EXT_TOTAL; ++i )
	{
		if ( !bRenamed[i] )
			continue;

		const auto & dExt = g_dIndexFilesExts[i];
		auto sFrom = FullPath ( dExt.m_szExt, "", sToSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sFromSz );
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback failure when renaming %s to %s", sFrom.cstr (), sTo.cstr () );
			m_bFatal = true;
		}
	}

	return false;
}

bool IndexFiles_c::RenameLock ( const char * sToSz, int &iLockFD )
{
	if ( iLockFD<0 ) // no lock, no renaming need
		return true;

	m_bFatal = false;
	auto sFrom = FullPath ( sphGetExt(SPH_EXT_SPL).cstr() );
	auto sTo = FullPath ( sphGetExt(SPH_EXT_SPL).cstr(), "", sToSz );

#if !USE_WINDOWS
	if ( !sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		return true;

	m_sLastError.SetSprintf ("failed to rename lock %s to %s, fd=%d, error %s (%d); ", sFrom.cstr(),
				   sTo.cstr(), iLockFD, strerrorm ( errno ), errno );

	// that is renaming of only 1 file failed; no need to rollback.
	m_bFatal = true;
	return false;

#else
	// on Windows - no direct rename. Lock new instead, release previous.
	int iNewLock=-1;
	if ( !RawFileLock ( sTo, iNewLock, m_sLastError ) )
		return false;
	auto iOldLock = iLockFD;
	iLockFD = iNewLock;
	RawFileUnLock ( sFrom, iOldLock );
	return true;
#endif
}

// move from backup to path using full (long) paths; fail is fatal
bool IndexFiles_c::Rollback ( const char * sBackup, const char * sPath )
{
	m_bFatal = false;
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFrom = FullPath ( dExt.m_szExt, "", sBackup );
		auto sTo = FullPath ( dExt.m_szExt, "", sPath );

		if ( !sphIsReadable ( sFrom ) )
		{
			::unlink ( sTo.cstr () );
			continue;
		}
#if USE_WINDOWS
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm (	errno ) );
			m_bFatal = true;
			return false;
		}
	}
	return true;
}

// move everything except not intended for copying.
bool IndexFiles_c::RenameSuffix ( const char * sFromSuffix, const char * sToSuffix )
{
	return Rename ( FullPath ( "", sFromSuffix ).cstr (), FullPath ( "", sToSuffix ).cstr () );
}

bool IndexFiles_c::RenameBase ( const char * sToBase )  // move files to different base
{
	return Rename ( FullPath ( "" ).cstr (), sToBase );
}

bool IndexFiles_c::RelocateToNew ( const char * sNewBase )
{
	return Rollback( FullPath ( "", "", sNewBase ).cstr(), FullPath ( "", ".new" ).cstr() );
}

bool IndexFiles_c::RollbackSuff ( const char * sBackupSuffix, const char * sActiveSuffix )
{
	return Rollback ( FullPath ( "", sBackupSuffix ).cstr (), FullPath ( "", sActiveSuffix ).cstr () );
}

bool IndexFiles_c::ReadVersion ( const char * sType )
{
	auto sPath = FullPath ( sphGetExt(SPH_EXT_SPH).cstr(), sType );
	BYTE dBuffer[8];

	CSphAutoreader rdHeader ( dBuffer, sizeof ( dBuffer ) );
	if ( !rdHeader.Open ( sPath, m_sLastError ) )
		return false;

	// check magic header
	const char * sMsg = CheckFmtMagic ( rdHeader.GetDword () );
	if ( sMsg )
	{
		m_sLastError.SetSprintf ( sMsg, sPath.cstr() );
		return false;
	}

	// get version
	DWORD uVersion = rdHeader.GetDword ();
	if ( uVersion==0 || uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sPath.cstr(), uVersion, INDEX_FORMAT_VERSION );
		return false;
	}
	m_uVersion = uVersion;
	return true;
}


bool IndexFiles_c::ReadKlistTargets ( StrVec_t & dTargets, const char * szType )
{
	CSphString sPath = FullPath ( sphGetExt(SPH_EXT_SPK).cstr(), szType );
	if ( !sphIsReadable(sPath) )
		return true;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sPath, sError ) )
		return false;

	DWORD nIndexes = tReader.GetDword();
	dTargets.Resize ( nIndexes );
	for ( auto & i : dTargets )
	{
		i = tReader.GetString();
		tReader.GetDword();	// skip flags
	}

	return true;
}


void IndexFiles_c::InitFrom ( const CSphIndex* pIndex )
{
	if ( !pIndex )
		return;
	m_sIndexName = pIndex->GetName ();
	m_sFilename = pIndex->GetFilename ();
	ReadVersion ();
}

volatile bool& sphGetShutdown ()
{
	static bool bShutdown = false;
	return bShutdown;
}

volatile int &sphGetTFO ()
{
	static int iTFO = 0;
	return iTFO;
}