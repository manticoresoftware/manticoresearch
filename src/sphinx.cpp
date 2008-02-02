//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
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

#include <ctype.h>
#include <errno.h>
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

#if USE_LIBSTEMMER
#include "libstemmer.h"
#endif

#if USE_LIBEXPAT
#define XMLIMPORT
#include "expat.h"

// workaround for expat versions prior to 1.95.7
#ifndef XMLCALL
#define XMLCALL
#endif
#endif

#if USE_LIBXML
#include <libxml/xmlreader.h>
#endif

#if USE_LIBICONV
#include "iconv.h"
#endif

#if USE_WINDOWS
	#include <io.h> // for open()

	// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
	#define snprintf	_snprintf
	#define sphSeek		_lseeki64
	#define strtoull	_strtoui64
	#define stat		_stat

	#define ICONV_INBUF_CONST	1
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/mman.h>

	#define sphSeek		lseek
#endif

#if ( USE_WINDOWS && USE_MYSQL )
	#pragma comment(linker, "/defaultlib:libmysql.lib")
	#pragma message("Automatically linking with libmysql.lib")
#endif

#if ( USE_WINDOWS && USE_LIBSTEMMER )
	#pragma comment(linker, "/defaultlib:libstemmer_c.lib")
	#pragma message("Automatically linking with libstemmer_c.lib")
#endif

#if ( USE_WINDOWS && USE_LIBEXPAT )
	#pragma comment(linker, "/defaultlib:libexpat.lib")
	#pragma message("Automatically linking with libexpat.lib")
#endif

#if ( USE_WINDOWS && USE_LIBICONV )
	#pragma comment(linker, "/defaultlib:iconv.lib")	
	#pragma message("Automatically linking with iconv.lib")
#endif

#if ( USE_WINDOWS && USE_LIBXML )
	#pragma comment(linker, "/defaultlib:libxml.lib")
	#pragma message("Automatically linking with libxml.lib")
#endif

/////////////////////////////////////////////////////////////////////////////

// logf() is not there sometimes (eg. Solaris 9)
#if !USE_WINDOWS && !HAVE_LOGF
static inline float logf ( float v )
{
	return (float) log ( v );
}
#endif

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#ifndef NDEBUG

void sphAssert ( const char * sExpr, const char * sFile, int iLine )
{
	char sBuffer [ 1024 ];
	_snprintf ( sBuffer, sizeof(sBuffer), "%s(%d): assertion %s failed\n", sFile, iLine, sExpr );

	if ( MessageBox ( NULL, sBuffer, "Assert failed! Cancel to debug.", 
		MB_OKCANCEL | MB_TOPMOST | MB_SYSTEMMODAL | MB_ICONEXCLAMATION ) != IDOK )
	{
		__debugbreak ();
	} else
	{
		fprintf ( stdout, "%s", sBuffer );
		exit ( 1 );
	}
}

#endif // !NDEBUG
#endif // USE_WINDOWS

// forward decl
void sphWarn ( const char * sTemplate, ... );
int sphReadThrottled ( int iFD, void * pBuf, size_t iCount );

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *	SPHINX_DEFAULT_SBCS_TABLE	= "0..9, A..Z->a..z, _, a..z, U+A8->U+B8, U+B8, U+C0..U+DF->U+E0..U+FF, U+E0..U+FF";
const char *	SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F";

const char		MAGIC_WORD_HEAD				= 1;
const char		MAGIC_WORD_TAIL				= 1;
const char		MAGIC_SYNONYM_WHITESPACE	= 1;

/////////////////////////////////////////////////////////////////////////////

static bool					g_bSphQuiet					= false;
static SphErrorCallback_fn	g_pInternalErrorCallback	= NULL;

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

STATIC_SIZE_ASSERT ( SphOffset_t, 8 );

/////////////////////////////////////////////////////////////////////////////
// INTERNAL PROFILER
/////////////////////////////////////////////////////////////////////////////

#define SPH_INTERNAL_PROFILER 0

#if SPH_INTERNAL_PROFILER

enum ESphTimer
{
	TIMER_root = 0,

	#define DECLARE_TIMER(_arg) TIMER_##_arg,
	#include "sphinxtimers.h"
	#undef DECLARE_TIMER

	TIMERS_TOTAL
};


static const char * const g_dTimerNames [ TIMERS_TOTAL ] =
{
	"root",

	#define DECLARE_TIMER(_arg) #_arg,
	#include "sphinxtimers.h"
	#undef DECLARE_TIMER
};


struct CSphTimer
{
	float			m_fStamp;
	ESphTimer		m_eTimer;
	int				m_iParent;
	int				m_iChild;
	int				m_iNext;
	int				m_iPrev;
	int				m_iCalls;

	CSphTimer ()
	{
		Alloc ( TIMER_root, -1 );
	}

	void Alloc ( ESphTimer eTimer, int iParent )
	{
		m_iParent = iParent;
		m_iChild = -1;
		m_iNext = -1;
		m_iPrev = -1;
		m_eTimer = eTimer;
		m_fStamp = 0;
		m_iCalls = 0;
	}

	void Start ()
	{
		m_fStamp -= sphLongTimer ();
		m_iCalls++;
	}

	void Stop ()
	{
		m_fStamp += sphLongTimer ();
	}
};

static const int	SPH_MAX_TIMERS					= 128;
static int			g_iTimer						= -1;
static int			g_iTimers						= 0;
static CSphTimer	g_dTimers [ SPH_MAX_TIMERS ];


void sphProfilerInit ()
{
	assert ( g_iTimers==0 );
	assert ( g_iTimer==-1 );

	// start root timer
	g_iTimers = 1;
	g_iTimer = 0;
	g_dTimers[g_iTimer].Alloc ( TIMER_root, -1 );
	g_dTimers[g_iTimer].Start ();
}


void sphProfilerPush ( ESphTimer eTimer )
{
	assert ( g_iTimer>=0 && g_iTimer<SPH_MAX_TIMERS );
	assert ( eTimer!=TIMER_root );

	// search for match timer in current timer's children list
	int iTimer;
	for ( iTimer=g_dTimers[g_iTimer].m_iChild;
		iTimer>0;
		iTimer=g_dTimers[iTimer].m_iNext )
	{
		if ( g_dTimers[iTimer].m_eTimer==eTimer )
			break;
	}

	// not found? let's alloc
	if ( iTimer<0 )
	{
		assert ( g_iTimers<SPH_MAX_TIMERS );
		iTimer = g_iTimers++;

		// create child and make current timer it's parent
		g_dTimers[iTimer].Alloc ( eTimer, g_iTimer );

		// make it new children list head
		g_dTimers[iTimer].m_iNext = g_dTimers[g_iTimer].m_iChild;
		if ( g_dTimers[g_iTimer].m_iChild>=0 )
			g_dTimers [ g_dTimers[g_iTimer].m_iChild ].m_iPrev = iTimer;
		g_dTimers[g_iTimer].m_iChild = iTimer;
	}

	// make it new current one
	assert ( iTimer>0 );
	g_dTimers[iTimer].Start ();
	g_iTimer = iTimer;
}


void sphProfilerPop ( ESphTimer eTimer )
{
	assert ( g_iTimer>0 && g_iTimer<SPH_MAX_TIMERS );
	assert ( g_dTimers[g_iTimer].m_eTimer==eTimer );

	g_dTimers[g_iTimer].Stop ();
	g_iTimer = g_dTimers[g_iTimer].m_iParent;
	assert ( g_iTimer>=0 && g_iTimer<SPH_MAX_TIMERS );
}


void sphProfilerDone ()
{
	assert ( g_iTimers>0 );
	assert ( g_iTimer==0 );

	// stop root timer
	g_iTimers = 0;
	g_iTimer = -1;
	g_dTimers[0].Stop ();
}


void sphProfilerShow ( int iTimer=0, int iLevel=0 )
{
	assert ( g_iTimers==0 );
	assert ( g_iTimer==-1 );

	if ( iTimer==0 )
		fprintf ( stdout, "--- PROFILE ---\n" );

	CSphTimer & tTimer = g_dTimers[iTimer];
	int iChild;

	// calc me
	int iChildren = 0;
	float fSelf = tTimer.m_fStamp;
	for ( iChild=tTimer.m_iChild; iChild>0; iChild=g_dTimers[iChild].m_iNext, iChildren++ )
		fSelf -= g_dTimers[iChild].m_fStamp;

	// dump me
	if ( tTimer.m_fStamp<0.00005f )
		return;

	char sName[32];
	for ( int i=0; i<iLevel; i++ )
		sName[2*i] = sName[2*i+1] = ' ';
	sName[2*iLevel] = '\0';
	strncat ( sName, g_dTimerNames [ tTimer.m_eTimer ], sizeof(sName) );

	fprintf ( stdout, "%-32s | %7.2f ms | %7.2f ms self | %d calls\n",
		sName, 1000.0f*tTimer.m_fStamp, 1000.0f*fSelf, tTimer.m_iCalls );

	// dump my children
	iChild = tTimer.m_iChild;
	while ( iChild>0 && g_dTimers[iChild].m_iNext>0 )
		iChild = g_dTimers[iChild].m_iNext;

	while ( iChild>0 )
	{
		sphProfilerShow ( iChild, 1+iLevel );
		iChild = g_dTimers[iChild].m_iPrev;
	}

	if ( iTimer==0 )
		fprintf ( stdout, "---------------\n" );
}


class CSphEasyTimer
{
public:
	CSphEasyTimer ( ESphTimer eTimer )
		: m_eTimer ( eTimer )
	{
		if ( g_iTimer>=0 )
			sphProfilerPush ( m_eTimer );
	}

	~CSphEasyTimer ()
	{
		if ( g_iTimer>=0 )
			sphProfilerPop ( m_eTimer );
	}

protected:
	ESphTimer		m_eTimer;
};


#define PROFILER_INIT() sphProfilerInit()
#define PROFILER_DONE() sphProfilerDone()
#define PROFILE_BEGIN(_arg) sphProfilerPush(TIMER_##_arg)
#define PROFILE_END(_arg) sphProfilerPop(TIMER_##_arg)
#define PROFILE_SHOW() sphProfilerShow()
#define PROFILE(_arg) CSphEasyTimer __t_##_arg ( TIMER_##_arg );

#else

#define PROFILER_INIT()
#define PROFILER_DONE()
#define PROFILE_BEGIN(_arg)
#define PROFILE_END(_arg)
#define PROFILE_SHOW()
#define PROFILE(_arg)

#endif // SPH_INTERNAL_PROFILER

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

INT64 sphClocks ()
{
	DWORD tLo, tHi;
	_asm
	{
		rdtsc
			mov		tLo, eax
			mov		tHi, edx
	}
	return ( INT64(tHi)<<32 ) | INT64(tLo);
}


INT64 sphClocksSec ()
{
	static INT64 tSEC = 0;
	if ( tSEC )
		return tSEC;

	clock_t t, tt;

	// wait til the begining of a new second
	t = clock();
	do {
		tt = clock();
	} while ( t==tt );

	// measure one
	tSEC = sphClocks();
	while ( clock()-tt<CLK_TCK );
	tSEC = sphClocks() - tSEC;

	return tSEC;
}


class CSphClocker
{
public:
	CSphClocker ( INT64 * pTarget )
		: m_pTarget ( pTarget )
	{
		*m_pTarget -= sphClocks();
	}

	~CSphClocker ()
	{
		*m_pTarget += sphClocks();
	}

protected:
	INT64 *		m_pTarget;
};

#endif // USE_WINDOWS

static bool g_bIOStats = false;
static CSphIOStats g_IOStats;


void sphStartIOStats ()
{
	g_bIOStats = true;
	memset ( &g_IOStats, 0, sizeof ( g_IOStats ) );
}


const CSphIOStats & sphStopIOStats ()
{
	g_bIOStats = false;
	return g_IOStats;
}


int sphRead ( int iFD, void * pBuf, size_t iCount )
{
	float fTimer = 0.0f;
	if ( g_bIOStats )
		fTimer = sphLongTimer ();

	size_t uRead = (size_t) ::read ( iFD, pBuf, iCount );

	if ( g_bIOStats )
	{
		g_IOStats.m_fReadTime += sphLongTimer () - fTimer;
		g_IOStats.m_iReadOps++;
		g_IOStats.m_fReadKBytes += float ( iCount ) / 1024.0f;
	}

	return uRead;
}

/////////////////////////////////////////////////////////////////////////////
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_O_BINARY O_BINARY
#else
#define SPH_O_BINARY 0
#endif

#define SPH_O_READ	( O_RDONLY | SPH_O_BINARY )
#define SPH_O_NEW	( O_CREAT | O_RDWR | O_TRUNC | SPH_O_BINARY )


/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
protected:
	int			m_iFD;			///< my file descriptior
	CSphString	m_sFilename;	///< my file name

public:
	CSphAutofile ( const char * sName, int iMode, CSphString & sError )
	{
		assert ( sName );
		m_iFD = ::open ( sName, iMode, 0644 );
		m_sFilename = sName;

		if ( m_iFD<0 )
			sError.SetSprintf ( "failed to open %s: %s", sName, strerror(errno) );
	}

	~CSphAutofile ()
	{
		Close ();
	}

	void Close ()
	{
		if ( m_iFD>=0 ) 
			::close ( m_iFD );
		m_iFD = -1;
		m_sFilename = "";
	}

public:
	int GetFD () const
	{
		return m_iFD;
	}

	const char * GetFilename () const
	{
		assert ( m_sFilename.cstr() );
		return m_sFilename.cstr();
	}

	SphOffset_t GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError )
	{
		struct stat st;
		if ( stat ( GetFilename(), &st )<0 )
		{
			sError.SetSprintf ( "failed to stat %s: %s", GetFilename(), strerror(errno) );
			return -1;
		}
		if ( st.st_size<iMinSize )
		{
			sError.SetSprintf ( "failed to load %s: bad size %"PRIi64" (at least %"PRIi64" bytes expected)",
				GetFilename(), (int64_t)st.st_size, (int64_t)iMinSize );
			return -1;
		}
		if ( bCheckSizeT )
		{
			size_t sCheck = (size_t)st.st_size;
			if ( st.st_size!=SphOffset_t(sCheck) )
			{
				sError.SetSprintf ( "failed to load %s: bad size %"PRIi64" (out of size_t; 4 GB limit on 32-bit machine hit?)",
					GetFilename(), (int64_t)st.st_size );
				return -1;
			}
		}
		return st.st_size;
	}

	SphOffset_t GetSize ()
	{
		CSphString sTmp;
		return GetSize ( 0, false, sTmp );
	}

	bool Read ( void * pBuf, size_t uCount, CSphString & sError )
	{
		size_t uRead = (size_t) sphRead ( GetFD(), pBuf, uCount );

		if ( uRead!=uCount )
		{
			sError.SetSprintf ( "read error in %s; %u of %u bytes read",
				GetFilename(), uRead, uCount );
			return false;
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// array pointer which self-destructs when going out of scope, or on demand
template < typename T > class CSphAutoArray
{
protected:
	T *		m_pData;

public:
			CSphAutoArray ( int iCount )	{ m_pData = ( iCount>0 ) ? new T [ iCount ] : NULL; }
			~CSphAutoArray ()				{ Reset (); }

	void	Reset ()						{ SafeDeleteArray ( m_pData ); }

	const CSphAutoArray & operator = ( const CSphAutoArray & )		{ assert(0); return *this; }
	operator T * ()													{ return m_pData; }
};

/////////////////////////////////////////////////////////////////////////////

/// in-memory buffer shared between processes
template < typename T > class CSphSharedBuffer
{
public:
	/// ctor
	CSphSharedBuffer ()
		: m_pData ( NULL )
		, m_iLength ( 0 )
		, m_iEntries ( 0 )
		, m_bMlock ( false )
	{}

	/// dtor
	~CSphSharedBuffer ()
	{
		Reset ();
	}

	/// set locking mode for subsequent Alloc()s
	void SetMlock ( bool bMlock )
	{
		m_bMlock = bMlock;
	}

public:
	/// allocate storage
	bool Alloc ( DWORD iEntries, CSphString & sError, CSphString * sWarning )
	{
		assert ( !m_pData );

		SphOffset_t uCheck = sizeof(T);
		uCheck *= (SphOffset_t)iEntries;

		m_iLength = (size_t)uCheck;
		if ( uCheck!=(SphOffset_t)m_iLength )
		{
			sError.SetSprintf ( "impossible to mmap() over 4 GB on 32-bit system" );
			m_iLength = 0;
			return false;
		}

		if ( sWarning )
			*sWarning = ""; // avoid-a-warn

#if USE_WINDOWS
		m_pData = new T [ iEntries ];
#else
		m_pData = (T *) mmap ( NULL, m_iLength, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0 );
		if ( m_pData==MAP_FAILED )
		{
			if ( m_iLength>0x7fffffffUL )
				sError.SetSprintf ( "mmap() failed: %s (length=%"PRIi64" is over 2GB, impossible on some 32-bit systems)", strerror(errno), (int64_t)m_iLength );
			else
				sError.SetSprintf ( "mmap() failed: %s (length=%"PRIi64")", strerror(errno), (int64_t)m_iLength );
			m_iLength = 0;
			return false;
		}

		if ( m_bMlock )
			if ( -1==mlock ( m_pData, m_iLength ) )
				sWarning->SetSprintf ( "mlock() failed: %s", strerror(errno) );
#endif // USE_WINDOWS

		assert ( m_pData );
		m_iEntries = iEntries;
		return true;
	}


	/// relock again (for daemonization only)
#if USE_WINDOWS
	bool Mlock ( const char *, CSphString & )
	{
		return true;
	}
#else
	bool Mlock ( const char * sPrefix, CSphString & sError )
	{
		if ( !m_bMlock )
			return true;

		if ( mlock ( m_pData, m_iLength )!=-1 )
			return true;

		if ( sError.IsEmpty() )
			sError.SetSprintf ( "%s mlock() failed: bytes=%"PRIu64", error=%s", sPrefix, (uint64_t)m_iLength, strerror(errno) );
		else
			sError.SetSprintf ( "%s; %s mlock() failed: bytes=%"PRIu64", error=%s", sError.cstr(), sPrefix, (uint64_t)m_iLength, strerror(errno) );
		return false;
	}
#endif


	/// deallocate storage
	void Reset ()
	{
		if ( !m_pData )
			return;

#if USE_WINDOWS
		delete [] m_pData;
#else
		int iRes = munmap ( m_pData, m_iLength );
		if ( iRes )
			sphWarn ( "munmap() failed: %s", strerror(errno) );
#endif // USE_WINDOWS

		m_pData = NULL;
		m_iLength = 0;
		m_iEntries = 0;
	}

public:
	/// accessor
	inline const T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<(int)m_iEntries );
		return m_pData[iIndex];
	}

	/// get write address
	T * GetWritePtr () const
	{
		return m_pData;
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		return m_pData==NULL;
	}

	/// get length
	size_t GetLength () const
	{
		return m_iLength;
	}

protected:
	T *					m_pData;	///< data storage
	size_t				m_iLength;	///< data length, bytes
	DWORD				m_iEntries;	///< data length, entries
	bool				m_bMlock;	///< whether to lock data in RAM
};

/////////////////////////////////////////////////////////////////////////////

/// generic stateless priority queue
template < typename T, typename COMP > class CSphQueue
{
protected:
	T *		m_pData;
	int		m_iUsed;
	int		m_iSize;

public:
	/// ctor
	CSphQueue ( int iSize )
		: m_iUsed ( 0 )
		, m_iSize ( iSize )
	{
		assert ( iSize>0 );
		m_pData = new T [ iSize ];
		assert ( m_pData );
	}

	/// dtor
	virtual ~CSphQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	virtual bool Push ( const T & tEntry )
	{
		if ( m_iUsed==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, m_pData[0] ) )
				return true;
			else
				Pop ();
		}

		// do add
		m_pData [ m_iUsed ] = tEntry;
		int iEntry = m_iUsed++;

		// sift up if needed, so that worst (lesser) ones float to the top
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) >> 1;
			if ( !COMP::IsLess ( m_pData[iEntry], m_pData[iParent] ) )
				break;

			// entry is less than parent, should float to the top
			Swap ( m_pData[iEntry], m_pData[iParent] );
			iEntry = iParent;
		}

		return true;
	}

	/// remove root (ie. top priority) entry
	virtual void Pop ()
	{
		assert ( m_iUsed );
		if ( !(--m_iUsed) ) // empty queue? just return
			return;

		// make the last entry my new root
		m_pData[0] = m_pData[m_iUsed];

		// sift down if needed
		int iEntry = 0;
		for ( ;; )
		{
			// select child
			int iChild = (iEntry<<1) + 1;
			if ( iChild>=m_iUsed )
				break;

			// select smallest child
			if ( iChild+1<m_iUsed )
				if ( COMP::IsLess ( m_pData[iChild+1], m_pData[iChild] ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( COMP::IsLess ( m_pData[iChild], m_pData[iEntry] ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}

	/// get entries count
	inline int GetLength () const
	{
		return m_iUsed;
	};

	/// get current root
	inline const T & Root () const
	{
		assert ( m_iUsed );
		return m_pData[0];
	}
};

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
	 BIN_READ_OK				///< bin read ok
	,BIN_READ_EOF				///< bin end
	,BIN_READ_ERROR			///< bin read error
};


/// bin, block input buffer
struct CSphBin
{
	static const int	MIN_SIZE	= 8192;
	static const int	WARN_SIZE	= 262144;

protected:
	int					m_iSize;

	BYTE *				m_dBuffer;
	BYTE *				m_pCurrent;
	int					m_iLeft;
	int					m_iDone;
	ESphBinState		m_eState;
	CSphWordHit			m_tLastHit;		///< last decoded hit

	int					m_iFile;		///< my file
	SphOffset_t *		m_pFilePos;		///< shared current offset in file

public:
	SphOffset_t			m_iFilePos;		///< my current offset in file
	int					m_iFileLeft;	///< how much data is still unread from the file

public:
						CSphBin ();
						~CSphBin ();

	static int			CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn = true );
	void				Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize );

	int					ReadByte ();
	ESphBinRead			ReadBytes ( void * pDest, int iBytes );
	int					ReadHit ( CSphWordHit * e, int iRowitems, CSphRowitem * pRowitems );
};

/////////////////////////////////////////////////////////////////////////////

class CSphWriter : ISphNoncopyable
{
public:
				CSphWriter ();
				~CSphWriter ();

	bool		OpenFile ( const char * sName, CSphString & sErrorBuffer );
	void		CloseFile (); ///< note: calls Flush(), ie. IsError() might get true after this call

	void		PutByte ( int uValue );
	void		PutBytes ( const void * pData, int iSize );
	void		PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void		PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }

	void		SeekTo ( SphOffset_t pos );

#if USE_64BIT
	void		PutDocid ( SphDocID_t uValue ) { PutOffset ( uValue ); }
#else
	void		PutDocid ( SphDocID_t uValue ) { PutDword ( uValue ); }
#endif

	void		ZipInt ( DWORD uValue );
	void		ZipOffset ( SphOffset_t uValue );
	void		ZipOffsets ( CSphVector<SphOffset_t> * pData );

	inline bool			IsError () const	{ return m_bError; }
	inline SphOffset_t	GetPos () const		{ return m_iPos; }

private:
	CSphString	m_sName;
	SphOffset_t	m_iPos;

	int			m_iFD;
	int			m_iPoolUsed;
	BYTE *		m_pBuffer;
	BYTE *		m_pPool;

	bool			m_bError;
	CSphString *	m_pError;

	void		Flush ();
};

/////////////////////////////////////////////////////////////////////////////

class CSphReader_VLN
{
public:
				CSphReader_VLN ( BYTE * pBuf=NULL, int iSize=0 );
	virtual		~CSphReader_VLN ();

	void		SetFile ( int iFD, const char * sFilename );
	void		SetFile ( const CSphAutofile & tFile );
	void		Reset ();

	void		SeekTo ( SphOffset_t iPos, int iSizeHint );

	void		SkipBytes ( int iCount );
	SphOffset_t	GetPos () const { return m_iPos+m_iBuffPos; }

	void		GetRawBytes ( void * pData, int iSize );
	void		GetBytes ( void * data, int size );

	int			GetByte ();
	DWORD		GetDword ();
	SphOffset_t	GetOffset ();
	CSphString	GetString ();
	int			GetLine ( char * sBuffer, int iMaxLen );

	DWORD		UnzipInt ();
	SphOffset_t	UnzipOffset ();

	SphOffset_t				Tell () const				{ return m_iPos + m_iBuffPos; }
	bool					GetErrorFlag () const		{ return m_bError; }
	const CSphString &		GetErrorMessage () const	{ return m_sError; }
	const CSphString &		GetFilename() const			{ return m_sFilename; }

#if USE_64BIT
	SphDocID_t	GetDocid ()		{ return GetOffset(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipOffset(); }
#else
	SphDocID_t	GetDocid ()		{ return GetDword(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipInt(); }
#endif

	const CSphReader_VLN &	operator = ( const CSphReader_VLN & rhs );

private:

	int			m_iFD;
	SphOffset_t	m_iPos;

	int			m_iBuffPos;
	int			m_iBuffUsed;
	BYTE *		m_pBuff;
	int			m_iSizeHint;	///< how much do we expect to read

	int			m_iBufSize;
	bool		m_bBufOwned;

	bool		m_bError;
	CSphString	m_sError;
	CSphString	m_sFilename;

	static const int	READ_BUFFER_SIZE		= 262144;
	static const int	READ_DEFAULT_BLOCK		= 32768;

private:
	void		UpdateCache ();
};

#define READ_NO_SIZE_HINT 0

/////////////////////////////////////////////////////////////////////////////

struct CSphIndex_VLN;

/// everything required to setup search term
struct CSphTermSetup : ISphNoncopyable
{
	ISphTokenizer *			m_pTokenizer;
	CSphDict *				m_pDict;
	CSphIndex_VLN *			m_pIndex;
	ESphDocinfo				m_eDocinfo;
	CSphDocInfo				m_tMin;
	int						m_iToCalc;
	const CSphAutofile &	m_tDoclist;
	const CSphAutofile &	m_tHitlist;
	DWORD					m_uMaxStamp;

	CSphTermSetup ( const CSphAutofile & tDoclist, const CSphAutofile & tHitlist )
		: m_pTokenizer ( NULL )
		, m_pDict ( NULL )
		, m_pIndex ( NULL )
		, m_eDocinfo ( SPH_DOCINFO_NONE )
		, m_iToCalc ( 0 )
		, m_tDoclist ( tDoclist )
		, m_tHitlist ( tHitlist )
		, m_uMaxStamp ( 0 )
	{}
};


/// query word from the searcher's point of view
class CSphQueryWord
{
public:
	CSphString		m_sWord;		///< my copy of word
	int				m_iQueryPos;	///< word position, from query (WARNING! reused for index into TF array in extended mode)
	SphWordID_t		m_iWordID;		///< word ID, from dictionary
	bool			m_bDupe;		///< whether the word occurs only once in current query

	int				m_iDocs;		///< document count, from wordlist
	int				m_iHits;		///< hit count, from wordlist

	CSphMatch		m_tDoc;			///< current match (partial)
	DWORD			m_uFields;		///< current match fields
	DWORD			m_uMatchHits;	///< current match hits count
	DWORD			m_iHitPos;		///< current hit postition, from hitlist

	SphOffset_t		m_iHitlistPos;	///< current position in hitlist, from doclist

	CSphReader_VLN	m_rdDoclist;	///< my doclist reader
	CSphReader_VLN	m_rdHitlist;	///< my hitlist reader

	SphDocID_t		m_iMinID;		///< min ID to fixup
	int				m_iInlineAttrs;	///< inline attributes count
	CSphRowitem *	m_pInlineFixup;	///< inline attributes fixup (POINTER TO EXTERNAL DATA, NOT MANAGED BY THIS CLASS!)

#ifndef NDEBUG
	bool			m_bHitlistOver;
#endif

public:
	CSphQueryWord ()
		: m_sWord ( NULL )
		, m_iQueryPos ( -1 )
		, m_iWordID ( 0 )
		, m_bDupe ( false )
		, m_iDocs ( 0 )
		, m_iHits ( 0 )
		, m_uFields ( 0 )
		, m_uMatchHits ( 0 )
		, m_iHitPos ( 0 )
		, m_iHitlistPos ( 0 )
		, m_iMinID ( 0 )
		, m_iInlineAttrs ( 0 )
		, m_pInlineFixup ( NULL )
#ifndef NDEBUG
		, m_bHitlistOver ( true )
#endif
	{
	}

	void Reset ()
	{
		m_rdDoclist.Reset ();
		m_rdHitlist.Reset ();
		m_iDocs = 0;
		m_iHits = 0;
		m_iHitPos = 0;
		m_iHitlistPos = 0;
		m_iInlineAttrs = 0;
		m_tDoc.Reset ( 0 );
		m_uFields = 0;
		m_uMatchHits = 0;
	}

	void GetDoclistEntry ()
	{
		SphDocID_t iDeltaDoc = m_rdDoclist.UnzipDocid ();
		if ( iDeltaDoc )
		{
			if ( m_tDoc.m_iDocID==0 )
				m_tDoc.m_iDocID = m_iMinID;
			m_tDoc.m_iDocID += iDeltaDoc;

			for ( int i=0; i<m_iInlineAttrs; i++ )
				m_tDoc.m_pRowitems[i] = m_rdDoclist.UnzipInt () + m_pInlineFixup[i];

			SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset ();
			assert ( iDeltaPos>0 );

			m_iHitlistPos += iDeltaPos;
			m_rdHitlist.SeekTo ( m_iHitlistPos, READ_NO_SIZE_HINT );
			m_iHitPos = 0;
#ifndef NDEBUG
			m_bHitlistOver = false;
#endif

			m_uFields = m_rdDoclist.UnzipInt ();
			m_uMatchHits = m_rdDoclist.UnzipInt ();

		} else
		{
			m_tDoc.m_iDocID = 0;
		}
	}

	void GetDoclistEntryOnly ()
	{
		SphDocID_t iDeltaDoc = m_rdDoclist.UnzipDocid ();
		if ( iDeltaDoc )
		{
			if ( m_tDoc.m_iDocID==0 )
				m_tDoc.m_iDocID = m_iMinID;
			m_tDoc.m_iDocID += iDeltaDoc;

			for ( int i=0; i<m_iInlineAttrs; i++ )
				m_tDoc.m_pRowitems[i] = m_rdDoclist.UnzipInt () + m_pInlineFixup[i];

			SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset ();
			assert ( iDeltaPos>0 );

			m_iHitlistPos += iDeltaPos;

			m_uFields = m_rdDoclist.UnzipInt ();
			m_uMatchHits = m_rdDoclist.UnzipInt ();

		} else
		{
			m_tDoc.m_iDocID = 0;
		}
	}

	void GetHitlistEntry ()
	{
#ifndef NDEBUG
		assert ( !m_bHitlistOver );
#endif

		DWORD iDelta = m_rdHitlist.UnzipInt ();
		if ( iDelta )
		{
			m_iHitPos += iDelta;
		} else
		{
			m_iHitPos = 0;
#ifndef NDEBUG
			m_bHitlistOver = true;
#endif
		}
	}

	void SetupAttrs ( const CSphTermSetup & tSetup )
	{
		m_tDoc.Reset ( tSetup.m_tMin.m_iRowitems + tSetup.m_iToCalc );

		m_iMinID = tSetup.m_tMin.m_iDocID;
		if ( tSetup.m_eDocinfo==SPH_DOCINFO_INLINE )
		{
			m_iInlineAttrs = tSetup.m_tMin.m_iRowitems;
			m_pInlineFixup = tSetup.m_tMin.m_pRowitems;
		} else
		{
			m_iInlineAttrs = 0;
			m_pInlineFixup = NULL;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////

struct CSphWordlistCheckpoint
{
	SphWordID_t			m_iWordID;
	DWORD				m_iWordlistOffset;
};


struct CSphMergeSource
{	
	const BYTE *		m_pWordlist;
	const BYTE *		m_pWordlistEnd;
	int					m_iRowitems;
	SphDocID_t			m_iLastDocID;
	SphDocID_t			m_iMinDocID;
	CSphMatch			m_tMatch;
	bool				m_bForceDocinfo;
	CSphIndex_VLN *		m_pIndex;
	CSphReader_VLN *	m_pDoclistReader;
	CSphReader_VLN *	m_pHitlistReader;
	CSphRowitem *		m_pMinAttrs;
	int					m_iWordlistEntries;
	int					m_iMaxWordlistEntries;

	CSphMergeSource()
		: m_pWordlist ( NULL )
		, m_pWordlistEnd ( NULL )
		, m_iRowitems ( 0 )
		, m_iLastDocID ( 0 )
		, m_iMinDocID ( 0 )
		, m_bForceDocinfo ( false )
		, m_pIndex ( NULL )
		, m_pDoclistReader ( NULL )
		, m_pHitlistReader ( NULL )
		, m_pMinAttrs ( NULL )
		, m_iWordlistEntries ( 0 )
		, m_iMaxWordlistEntries ( 0 )
	{}

	bool Check ()
	{
		return ( m_pWordlist && m_pDoclistReader && m_pHitlistReader );
	}

	DWORD	GetWordlistSize() const
	{
		assert ( m_pWordlist && m_pWordlistEnd );
		
		return ( m_pWordlistEnd - m_pWordlist );
	}
};

struct CSphMergeData
{
	CSphWriter *		m_pIndexWriter;
	CSphWriter *		m_pDataWriter;
	CSphWriter *		m_pHitlistWriter;

	SphOffset_t			m_iLastDoclistPos;
	SphOffset_t			m_iDoclistPos;
	SphOffset_t			m_iLastHitlistPos;
	SphOffset_t			m_iWordlistOffset;

	SphWordID_t			m_iLastWordID;
	SphDocID_t			m_iLastDocID;
	SphDocID_t			m_iMinDocID;

	CSphPurgeData *		m_pPurgeData;
	CSphSourceStats		m_tStats;
	DWORD *				m_pMinAttrs;
	ESphDocinfo			m_eDocinfo;

	CSphMergeData ()
		: m_pIndexWriter ( NULL )
		, m_pDataWriter ( NULL )
		, m_pHitlistWriter ( NULL )

		, m_iLastDoclistPos ( 0 )
		, m_iDoclistPos ( 0 )
		, m_iLastHitlistPos ( 0 )
		, m_iWordlistOffset ( 0 )

		, m_iLastWordID ( 0 )
		, m_iLastDocID ( 0 )
		, m_iMinDocID ( 0 )

		, m_pPurgeData ( NULL )
		, m_pMinAttrs ( NULL )
		, m_eDocinfo ( SPH_DOCINFO_NONE )
	{}

	virtual ~CSphMergeData ()
	{
		SafeDeleteArray ( m_pMinAttrs );
	}
};

/// doclist record
struct CSphDoclistRecord
{
	SphDocID_t		m_iDocID;
	CSphRowitem *	m_pRowitems;
	SphOffset_t		m_iPos;
	DWORD			m_uFields;
	DWORD			m_uMatchHits;
	SphOffset_t		m_iLeadingZero;	
	int				m_iRowitems;

					CSphDoclistRecord ()
						: m_iDocID ( 0 )
						, m_pRowitems( NULL )
						, m_iPos ( 0 )
						, m_uFields ( 0 )
						, m_uMatchHits ( 0 )
						, m_iLeadingZero ( 0 )
						, m_iRowitems ( 0 )					
					{}

	virtual			~CSphDoclistRecord ();

	void			Inititalize ( int iAttrNum );
	void			Read ( CSphMergeSource * pSource );
	void			Write ( CSphMergeData * pData );

	const CSphDoclistRecord & operator = ( const CSphDoclistRecord & rhs );
};

struct CSphWordIndexRecord
{
	SphWordID_t		m_iWordID;
	SphOffset_t		m_iDoclistPos;
	int				m_iDocNum;
	int				m_iHitNum;

	CSphWordIndexRecord()
		: m_iWordID ( 0 )
		, m_iDoclistPos ( 0 )
		, m_iDocNum ( 0 )
		, m_iHitNum ( 0 )
	{}

	bool Read ( const BYTE * & pSource, CSphMergeSource * pMergeSource );
	void Write ( CSphWriter * pWriter, CSphMergeData * pMergeData );

	bool operator < ( const CSphWordIndexRecord & rhs ) const
	{
		return m_iWordID < rhs.m_iWordID;
	}

	bool operator > ( const CSphWordIndexRecord & rhs ) const
	{
		return m_iWordID > rhs.m_iWordID;
	}

	bool operator == ( const CSphWordIndexRecord & rhs ) const
	{
		return m_iWordID == rhs.m_iWordID;
	}
};

struct CSphWordDataRecord
{
	SphWordID_t							m_iWordID;
	int									m_iRowitems;
	CSphVector<DWORD>					m_dWordPos;
	CSphVector<CSphDoclistRecord>		m_dDoclist;
	DWORD								m_iLeadingZero;

	CSphWordDataRecord()
		: m_iWordID ( 0 )
		, m_iRowitems ( 0 )
		, m_iLeadingZero ( 0 )
	{}

	bool operator < ( const CSphWordDataRecord & rhs )
	{
		return m_iWordID < rhs.m_iWordID;
	}

	bool operator > ( const CSphWordDataRecord & rhs )
	{
		return m_iWordID > rhs.m_iWordID;
	}

	bool operator == ( const CSphWordDataRecord & rhs ) const
	{
		return m_iWordID == rhs.m_iWordID;
	}

	void Read ( CSphMergeSource * pSource, CSphMergeData * pMergeData, int iDocNum );
	void Write ( CSphMergeData * pMergeData );

	bool IsEmpty() const
	{
		return ( m_dDoclist.GetLength() == 0 );
	}
};

struct CSphWordRecord
{
	CSphWordIndexRecord		m_tWordIndex;
	CSphWordDataRecord		m_tWordData;
	CSphMergeSource	*		m_pMergeSource;
	CSphMergeData *			m_pMergeData;

	CSphWordRecord ( CSphMergeSource * pSource, CSphMergeData * pData )
		: m_pMergeSource ( pSource )
		, m_pMergeData ( pData )
	{}

	bool Read ();
	void Write ();
	bool IsEmpty() const
	{
		return m_tWordData.IsEmpty();
	}
};

struct CSphDocMVA
{
	SphDocID_t							m_iDocID;
	CSphVector < CSphVector<DWORD> >	m_dMVA;
	CSphVector < DWORD >				m_dOffsets;

	CSphDocMVA( int iSize )
		: m_iDocID ( 0 )
	{
		m_dMVA.Resize ( iSize );
		m_dOffsets.Resize ( iSize );
	}

	void	Read ( CSphReader_VLN & tReader );
	void	Write ( CSphWriter & tWriter );
};

/////////////////////////////////////////////////////////////////////////////

/// ordinals accumulation and sorting
struct Ordinal_t
{
	SphDocID_t	m_uDocID;	///< doc id
	CSphString	m_sValue;	///< string value
};


struct OrdinalEntry_t : public Ordinal_t
{
	int	m_iTag;
};


struct OrdinalId_t
{
	SphDocID_t	m_uDocID;
	DWORD		m_uId;
};


struct OrdinalIdEntry_t : public OrdinalId_t
{
	int	m_iTag;
};

void Swap ( Ordinal_t & a, Ordinal_t & b )
{
	Swap ( a.m_uDocID, b.m_uDocID );
	Swap ( a.m_sValue, b.m_sValue );
}

void Swap ( OrdinalEntry_t & a, OrdinalEntry_t & b )
{
	Swap ( a.m_uDocID, b.m_uDocID );
	Swap ( a.m_sValue, b.m_sValue );
	Swap ( a.m_iTag,   b.m_iTag );
}

/// this is my actual VLN-compressed phrase index implementation
struct CSphIndex_VLN : CSphIndex
{
	friend struct CSphDoclistRecord;

								CSphIndex_VLN ( const char * sFilename, bool bEnableStar );
								~CSphIndex_VLN ();

	virtual int					Build ( CSphDict * pDict, const CSphVector<CSphSource*> & dSources, int iMemoryLimit, ESphDocinfo eDocinfo );

	virtual const CSphSchema *	Prealloc ( bool bMlock, CSphString * sWarning );
	virtual bool				Mlock ();
	virtual void				Dealloc ();

	virtual bool				Preread ();
	template<typename T> bool	PrereadSharedBuffer ( CSphSharedBuffer<T> & pBuffer, const char * sExt );

	virtual void				SetBase ( const char * sNewBase );
	virtual bool				Rename ( const char * sNewBase );

	virtual bool				Lock ();
	virtual void				Unlock ();

	void						BindWeights ( const CSphQuery * pQuery );
	bool						SetupCalc ( CSphQueryResult * pResult, const CSphQuery * pQuery );

	virtual CSphQueryResult *	Query ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery );
	virtual bool				QueryEx ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchSorter * pTop );
	virtual bool				MultiQuery ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters );

	virtual bool				Merge( CSphIndex * pSource, CSphPurgeData & tPurgeData );
	void						MergeWordData ( CSphWordRecord & tDstWord, CSphWordRecord & tSrcWord );

	virtual int					UpdateAttributes ( const CSphAttrUpdate_t & tUpd );
	virtual bool				SaveAttributes ();

private:
	static const int			WORDLIST_CHECKPOINT		= 1024;		///< wordlist checkpoints frequency
	static const int			WRITE_BUFFER_SIZE		= 262144;	///< my write buffer size

	static const DWORD			INDEX_MAGIC_HEADER		= 0x58485053;	///< my magic 'SPHX' header
	static const DWORD			INDEX_FORMAT_VERSION	= 8;			///< my format version

private:
	// common stuff
	CSphString					m_sFilename;
	int							m_iLockFD;

	ESphDocinfo					m_eDocinfo;
	CSphDocInfo					m_tMin;				///< min attribute values tracker
	CSphSourceStats				m_tStats;			///< my stats

private:
	// indexing-only
	BYTE *						m_pWriteBuffer;		///< my write buffer

	CSphWordHit					m_tLastHit;			///< hitlist entry

	SphOffset_t					m_iLastHitlistPos;	///< doclist entry
	DWORD						m_uLastDocFields;	///< doclist entry
	DWORD						m_uLastDocHits;		///< doclist entry

	SphOffset_t					m_iLastDoclistPos;	///< wordlist entry
	int							m_iLastWordDocs;	///< wordlist entry
	int							m_iLastWordHits;	///< wordlist entry

	int									m_iWordlistEntries;		///< wordlist entries written since last checkpoint
	CSphVector<CSphWordlistCheckpoint>	m_dWordlistCheckpoints;	///< wordlist checkpoint offsets

	CSphWriter					m_wrWordlist;	///< wordlist writer
	CSphWriter					m_wrDoclist;	///< wordlist writer
	CSphWriter					m_wrHitlist;	///< hitlist writer

	CSphIndexProgress			m_tProgress;

private:
	// searching-only
	CSphVector<CSphQueryWord>	m_dQueryWords;			///< search query words for "simple" query types (ie. match all/any/phrase)

	int							m_iWeights;						///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights

	static const int			DOCINFO_HASH_BITS	= 18;	// FIXME! make this configurable
	static const int			DOCINFO_INDEX_FREQ	= 128;	// FIXME! make this configurable

	CSphSharedBuffer<DWORD>		m_pDocinfo;				///< my docinfo cache
	DWORD						m_uDocinfo;				///< my docinfo cache size
	CSphSharedBuffer<DWORD>		m_pDocinfoHash;			///< hashed ids, to accelerate lookups
	DWORD						m_uDocinfoIndex;		///< docinfo "index" entries count (each entry is 2x docinfo rows, for min/max)
	CSphSharedBuffer<DWORD>		m_pDocinfoIndex;		///< docinfo "index", to accelerate filtering during full-scan

	CSphSharedBuffer<DWORD>		m_pMva;					///< my multi-valued attrs cache

	CSphSharedBuffer<BYTE>		m_pWordlist;			///< my wordlist cache
	CSphWordlistCheckpoint *	m_pWordlistCheckpoints;	///< my wordlist cache checkpoints
	int							m_iWordlistCheckpoints;	///< my wordlist cache checkpoints count

	bool						m_bPreallocated;		///< are we ready to preread
	CSphSharedBuffer<BYTE>		m_bPreread;				///< are we ready to search
	DWORD						m_uVersion;				///< data files version

	bool						m_bEnableStar;			///< enable star-syntax

	bool						m_bEarlyLookup;			///< whether early attr value lookup is needed
	bool						m_bLateLookup;			///< whether late attr value lookup is needed

	int							m_iCalcGeodist;			///< compute geodistance to this rowitem (-1 means do not compute)
	int							m_iGeoLatRowitem;		///< latitude rowitem
	int							m_iGeoLongRowitem;		///< longitude rowitem
	float						m_fGeoAnchorLat;		///< anchor latitude
	float						m_fGeoAnchorLong;		///< anchor longitude

	int							m_iCalcExpr;			///< compute sorting expr to this rowitem (-1 means do not compute)
	CSphExpr					m_tCalcExpr;			///< expression opcodes

private:
	const char *				GetIndexFileName ( const char * sExt ) const;	///< WARNING, non-reenterable, static buffer!
	int							AdjustMemoryLimit ( int iMemoryLimit );

	int							cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits, DWORD * pDocinfo, int Docinfos, int iStride );
	void						cidxHit ( CSphWordHit * pHit, CSphRowitem * pDocinfos );
	bool						cidxDone ();

	void						WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol );
	void						ReadSchemaColumn ( CSphReader_VLN & rdInfo, CSphColumnInfo & tCol );

	void						MatchAll ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters );
	void						MatchAny ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters );
	bool						MatchBoolean ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup );
	bool						MatchExtended ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup );
	bool						MatchExtendedV1 ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup );
	void						MatchFullScan ( const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup );

	const DWORD *				FindDocinfo ( SphDocID_t uDocID );
	void						CalcDocinfo ( CSphMatch & tMatch, const DWORD * pDocinfo );
	void						LookupDocinfo ( CSphMatch & tMatch );

	bool						BuildMVA ( const CSphVector<CSphSource*> & dSources, CSphAutoArray<CSphWordHit> & dHits, int iArenaSize, int iFieldFD, int nFieldMVAs, int iFieldMVAInPool );

	void						CheckQueryWord ( const char * szWord, CSphQueryResult * pResult );
	void						CheckExtendedQuery ( const CSphExtendedQueryNode * pNode, CSphQueryResult * pResult );
	void						CheckBooleanQuery ( const CSphBooleanQueryExpr * pExpr, CSphQueryResult * pResult );

private:
	static const int MAX_ORDINAL_STR_LEN	= 1024;		///< maximum ordinal string length in bytes
	static const int ORDINAL_READ_SIZE		= 262144;	///< sorted ordinal id read buffer size in bytes

	ESphBinRead					ReadOrdinal ( CSphBin & Reader, Ordinal_t & Ordinal );
	SphOffset_t					DumpOrdinals ( CSphWriter & Writer, CSphVector<Ordinal_t> & dOrdinals );
	bool						SortOrdinals ( const char * szToFile, int iFromFD, int iArenaSize, int iOrdinalsInPool, const CSphVector< CSphVector<SphOffset_t> > & dOrdBlockSize, bool bWarnOfMem );
	bool						SortOrdinalIds ( const char * szToFile, int iFromFD, int nAttrs, int nOrdinals, int iArenaSize, int iNumPoolOrdinals, bool bWarnOfMem );

public:
	// FIXME! this needs to be protected, and refactored as well
	bool						SetupQueryWord ( CSphQueryWord & tWord, const CSphTermSetup & tTermSetup );
};

/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

void sphWarn ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	fprintf ( stdout, "WARNING: " );
	vfprintf ( stdout, sTemplate, ap );
	fprintf ( stdout, "\n" );
	va_end ( ap );
}


/// time since start, in seconds
float sphLongTimer ()
{
#if USE_WINDOWS
	// Windows time query
	static float fFreq;
	static INT64 iStart;
	static bool bFirst = true;

	LARGE_INTEGER iLarge;
	if ( bFirst )
	{
		QueryPerformanceFrequency ( &iLarge );
		fFreq = 1.0f/iLarge.QuadPart;

		QueryPerformanceCounter ( &iLarge );
		iStart = iLarge.QuadPart;

		bFirst = false;
	}

	QueryPerformanceCounter ( &iLarge);
	return ( iLarge.QuadPart-iStart )*fFreq;

#else
	// UNIX time query
	static int s_sec = -1, s_usec = -1;
	struct timeval tv;

	if ( s_sec == -1 )
	{
		gettimeofday ( &tv, NULL );
		s_sec = tv.tv_sec;
		s_usec = tv.tv_usec;
	}
	gettimeofday ( &tv, NULL );
	return float(tv.tv_sec-s_sec) + float(tv.tv_usec-s_usec)/1000000.0f;

#endif // USE_WINDOWS
}


/// time since start, in milliseconds
DWORD sphTimerMsec ()
{
	return (DWORD)( sphLongTimer()*1000.0f );
}


bool sphWrite ( int iFD, const void * pBuf, size_t iCount, const char * sName, CSphString & sError )
{
	if ( iCount<=0 )
		return true;

	int iWritten = ::write ( iFD, pBuf, iCount );
	if ( iWritten==(int)iCount )
		return true;

	if ( iWritten<0 )
		sError.SetSprintf ( "%s: write error: %s", sName, strerror(errno) );
	else
		sError.SetSprintf ( "%s: write error: %d of %d bytes written", sName, iWritten, int(iCount) );
	return false;
}


static int		g_iMaxIOps		= 0;
static int		g_iMaxIOSize	= 0;
static float	g_fLastIOTime	= 0.0f;

void sphSetThrottling ( int iMaxIOps, int iMaxIOSize )
{
	g_iMaxIOps	 = iMaxIOps;
	g_iMaxIOSize = iMaxIOSize;
}


bool sphWriteThrottled ( int iFD, const void * pBuf, size_t iCount, const char * sName, CSphString & sError )
{
	if ( g_iMaxIOSize && int (iCount) > g_iMaxIOSize )
	{
		int nChunks		= iCount / g_iMaxIOSize;
		int nBytesLeft	= iCount % g_iMaxIOSize;

		for ( int i = 0; i < nChunks; ++i )
		{
			if ( !sphWriteThrottled ( iFD, (const char *)pBuf + i*g_iMaxIOSize, g_iMaxIOSize, sName, sError ) )
				return false;
		}

		if ( nBytesLeft > 0 )
			if ( !sphWriteThrottled ( iFD, (const char *)pBuf + nChunks*g_iMaxIOSize, nBytesLeft, sName, sError ) )
				return false;

		return true;
	}

	if ( g_iMaxIOps > 0 )
	{
		float fTime = sphLongTimer ();
		float fSleep = Max ( 0.0f, g_fLastIOTime + 1.0f / g_iMaxIOps - fTime );
		sphUsleep ( int ( fSleep * 1000.0f ) );
		g_fLastIOTime = fTime + fSleep;
	}

	return sphWrite ( iFD, pBuf, iCount, sName, sError );
}


int sphReadThrottled ( int iFD, void * pBuf, size_t iCount )
{
	if ( g_iMaxIOSize && int (iCount) > g_iMaxIOSize )
	{
		int nChunks		= iCount / g_iMaxIOSize;
		int nBytesLeft	= iCount % g_iMaxIOSize;

		int nBytesRead = 0;
		int iRead = 0;

		for ( int i = 0; i < nChunks; ++i )
		{
			iRead = sphReadThrottled ( iFD, (char *)pBuf + i*g_iMaxIOSize, g_iMaxIOSize );
			nBytesRead += iRead;
			if ( iRead != g_iMaxIOSize )
				return nBytesRead;
		}

		if ( nBytesLeft > 0 )
		{
			iRead = sphReadThrottled ( iFD, (char *)pBuf + nChunks*g_iMaxIOSize, nBytesLeft );
			nBytesRead += iRead;
			if ( iRead != nBytesLeft )
				return nBytesRead;
		}

		return nBytesRead;
	}

	if ( g_iMaxIOps > 0 )
	{
		float fTime = sphLongTimer ();
		float fSleep = Max ( 0.0f, g_fLastIOTime + 1.0f / g_iMaxIOps - fTime );
		sphUsleep ( int ( fSleep * 1000.0f ) );
		g_fLastIOTime = fTime + fSleep;
	}

	return sphRead ( iFD, pBuf, iCount );
}


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


char * sphStrMacro ( const char * sTemplate, const char * sMacro, SphDocID_t uValue )
{
	// expand macro
	char sExp [ 32 ];
	snprintf ( sExp, sizeof(sExp), DOCID_FMT, uValue );

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
		strcpy ( sOut, sExp ); sOut += iExp;
		sCur += iMacro;
		sLast = sCur;
	}

	if ( *sLast )
		strcpy ( sOut, sLast );

	assert ( (int)strlen(sRes)==iRes );
	return sRes;
}


float sphToFloat ( const char * s )
{
	if ( !s ) return 0.0f;
	return (float)strtod ( s, NULL );
}


DWORD sphToDword ( const char * s )
{
	if ( !s ) return 0;
	return strtoul ( s, NULL, 10 );
}


uint64_t sphToQword ( const char * s )
{
	if ( !s ) return 0;
	return strtoull ( s, NULL, 10 );
}


#if USE_64BIT
#define sphToDocid sphToQword
#else
#define sphToDocid sphToDword
#endif


#if USE_WINDOWS || !HAVE_F_SETLKW

bool sphLockEx ( int, bool )	{ return true; }
void sphLockUn ( int )			{}

#else

bool sphLockEx ( int iFile, bool bWait )
{
	struct flock tLock;
	tLock.l_type = F_WRLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;

	int iCmd = bWait ? F_SETLKW : F_SETLK;
	return ( fcntl ( iFile, iCmd, &tLock )!=-1 );
}


void sphLockUn ( int iFile )
{
	struct flock tLock;
	tLock.l_type = F_UNLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;

	fcntl ( iFile, F_SETLK, &tLock );
}
#endif


void sphUsleep ( int iMsec )
{
	if ( iMsec<=0 )
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
	int iFD = ::open ( sPath, O_RDONLY );

	if ( iFD<0 )
	{
		if ( pError )
			pError->SetSprintf ( "%s unreadable: %s", sPath, strerror(errno) );
		return false;
	}

	close ( iFD );
	return true;
}


void sphInternalError ( const char * sTemplate, ... )
{
	if ( !g_pInternalErrorCallback )
		return;

	char sBuf [ 1024 ];
	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
	va_end ( ap );

	g_pInternalErrorCallback ( sBuf );
}


void sphSetInternalErrorCallback ( void (*fnCallback) ( const char * ) )
{
	g_pInternalErrorCallback = fnCallback;
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////


#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif
inline int sphUTF8Decode ( BYTE * & pBuf ); // forward ref for GCC
inline int sphUTF8Encode ( BYTE * pBuf, int iCode ); // forward ref for GCC


/// tokenizer implementation traits
template < bool IS_UTF8 > 
class CSphTokenizerTraits : public ISphTokenizer
{
public:
	CSphTokenizerTraits ();

	virtual const BYTE *	GetBufferPtr () const;
	virtual const BYTE *	GetBufferEnd () const;
	virtual void			AdvanceBufferPtr ( int iOffset );

protected:
	BYTE * GetTokenSyn ();

protected:
	/// get codepoint
	inline int GetCodepoint ()
	{
		if ( IS_UTF8 )
		{
			while ( m_pCur<m_pBufferMax )
			{
				int iCode = sphUTF8Decode ( m_pCur );
				if ( iCode>=0 )
					return iCode; // succesful decode
			}
			return -1; // eof
		} else
		{
			return m_pCur>=m_pBufferMax
				? -1
				: int(*m_pCur++);
		}
	}

	/// accum codepoint
	inline void AccumCodepoint ( int iCode )
	{
		assert ( iCode>0 );
		assert ( m_iAccum>=0 );

		// throw away everything which is over the token size
		if ( m_iAccum<SPH_MAX_WORD_LEN )
		{
			if ( IS_UTF8 )
				m_pAccum += sphUTF8Encode ( m_pAccum, iCode );
			else
				*m_pAccum++ = BYTE(iCode);
			assert ( m_pAccum>=m_sAccum && m_pAccum<m_sAccum+sizeof(m_sAccum) );

			m_iAccum++;
		}
	}

protected:
	BYTE *				m_pBuffer;							///< my buffer
	BYTE *				m_pBufferMax;						///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;								///< current position

	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< folded token accumulator
	BYTE *				m_pAccum;							///< current accumulator position
	int					m_iAccum;							///< boundary token size
};


/// single-byte charset tokenizer
class CSphTokenizer_SBCS : public CSphTokenizerTraits<false>
{
public:
								CSphTokenizer_SBCS ();

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength );
	virtual BYTE *				GetToken ();
	virtual ISphTokenizer *		Clone () const;
	virtual bool				IsUtf8 () const { return false; }
	virtual int					GetCodepointLength ( int ) const { return 1; }
};


/// UTF-8 tokenizer
class CSphTokenizer_UTF8 : public CSphTokenizerTraits<true>
{
public:
								CSphTokenizer_UTF8 ();

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength );
	virtual BYTE *				GetToken ();
	virtual ISphTokenizer *		Clone () const;
	virtual bool				IsUtf8 () const { return true; }
	virtual int					GetCodepointLength ( int iCode ) const;

protected:
	void						FlushAccum ();
};


/// UTF-8 tokenizer with n-grams
class CSphTokenizer_UTF8Ngram : public CSphTokenizer_UTF8
{
public:
						CSphTokenizer_UTF8Ngram () : m_iNgramLen ( 1 ) {}

public:
	virtual bool		SetNgramChars ( const char * sConfig, CSphString & sError );
	virtual void		SetNgramLen ( int iLen );
	virtual BYTE *		GetToken ();

protected:
	int					m_iNgramLen;
};


#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const
#endif

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphCreateSBCSTokenizer ()
{
	return new CSphTokenizer_SBCS ();
}


ISphTokenizer * sphCreateUTF8Tokenizer ()
{
	return new CSphTokenizer_UTF8 ();
}

ISphTokenizer * sphCreateUTF8NgramTokenizer ()
{
	return new CSphTokenizer_UTF8Ngram ();
}

/////////////////////////////////////////////////////////////////////////////

CSphLowercaser::CSphLowercaser ()
	: m_pData ( NULL )
{
}


void CSphLowercaser::Reset()
{
	m_iChunks = 0;
	for ( int i=0; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = NULL;
	SafeDeleteArray ( m_pData );
}


CSphLowercaser::~CSphLowercaser ()
{
	Reset ();
}


void CSphLowercaser::SetRemap ( const CSphLowercaser * pLC )
{
	if ( !pLC )
		return;

	Reset ();

	m_iChunks = pLC->m_iChunks;
	m_pData = new int [ m_iChunks*CHUNK_SIZE ];
	memcpy ( m_pData, pLC->m_pData, sizeof(int)*m_iChunks*CHUNK_SIZE );

	for ( int i=0; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = pLC->m_pChunk[i]
			? pLC->m_pChunk[i] - pLC->m_pData + m_pData
			: NULL;
}


void CSphLowercaser::AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags, DWORD uFlagsIfExists )
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

		for ( int iChunk = tRemap.m_iStart>>CHUNK_BITS; iChunk <= tRemap.m_iEnd>>CHUNK_BITS; iChunk++ )
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
		memset ( pData, 0, sizeof(int)*iNewChunks*CHUNK_SIZE );

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
				memcpy ( m_pChunk[i], pOldChunk, sizeof(int)*CHUNK_SIZE );
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
		for ( int j = tRemap.m_iStart; j <= tRemap.m_iEnd; j++, iRemapped++ )
		{
			assert ( m_pChunk [ j>>CHUNK_BITS ] );
			int & iCodepoint = m_pChunk [ j>>CHUNK_BITS ] [ j & CHUNK_MASK ];
			iCodepoint = ( iCodepoint ? uFlagsIfExists : uFlags ) | iRemapped;
		}
	}
}


enum
{
	MASK_CODEPOINT			= 0x00ffffffUL,	// mask off codepoint flags
	MASK_FLAGS				= 0xff000000UL, // mask off codepoint value
	FLAG_CODEPOINT_SPECIAL	= 0x01000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL		= 0x02000000UL,	// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM	= 0x04000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_SYNONYM	= 0x08000000UL,	// this codepoint is used in synonym tokens only
	FLAG_CODEPOINT_BOUNDARY	= 0x10000000UL,	// this codepoint is phrase boundary
	FLAG_CODEPOINT_IGNORE	= 0x20000000UL	// this codepoint is ignored
};


void CSphLowercaser::AddSpecials ( const char * sSpecials )
{
	assert ( sSpecials );
	int iSpecials = strlen(sSpecials);

	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Resize ( iSpecials );
	ARRAY_FOREACH ( i, dRemaps )
		dRemaps[i].m_iStart = dRemaps[i].m_iEnd = dRemaps[i].m_iRemapStart = sSpecials[i];

	AddRemaps ( dRemaps,
		FLAG_CODEPOINT_SPECIAL,
		FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
}

const CSphLowercaser & CSphLowercaser::operator = ( const CSphLowercaser & rhs )
{
	SetRemap ( &rhs );
	return * this;
}

/////////////////////////////////////////////////////////////////////////////

CSphCharsetDefinitionParser::CSphCharsetDefinitionParser ()
	: m_bError ( false )
{
}


const char * CSphCharsetDefinitionParser::GetLastError ()
{
	return m_bError ? m_sError : NULL;
}


bool CSphCharsetDefinitionParser::IsEof ()
{
	return (*m_pCurrent)==0;
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
	while ( (*m_pCurrent) && isspace((BYTE)*m_pCurrent) )
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
			iCode = iCode*16 + HexDigit(*p++);
//			if ( iCode>255 )
//				return Error ( "invalid hex code (exceeds byte range)" );
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

		// parse char code
		const char * pStart = m_pCurrent;
		int iStart = ParseCharsetCode();
		if ( iStart<0 )
			return false;

		// stray char?
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			// stray char
			dRanges.Add ( CSphRemapRange ( iStart, iStart, iStart ) );
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
			dRanges.Add ( CSphRemapRange ( iStart, iStart, iDest ) );

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
			dRanges.Add ( CSphRemapRange ( iStart, iEnd, iStart ) );
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
				dRanges.Add ( CSphRemapRange ( i, i, i+1 ) );
				dRanges.Add ( CSphRemapRange ( i+1, i+1, i+1 ) );
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
		if ( iRemapEnd-iRemapStart != iEnd-iStart )
		{
			m_pCurrent = pStart;
			return Error ( "dest range length must match src range length" );
		}

		// remapped ok
		dRanges.Add ( CSphRemapRange ( iStart, iEnd, iRemapStart ) );
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

/////////////////////////////////////////////////////////////////////////////

/// check if the code is whitespace
inline bool sphIsSpace ( int iCode )
{
	return iCode==' ' || iCode=='\t' || iCode=='\n' || iCode=='\r';
}


/// UTF-8 decode codepoint
/// advances buffer ptr in all cases but end of buffer
///
/// returns -1 on failure
/// returns 0 on end of buffer
/// returns codepoint on success
inline int sphUTF8Decode ( BYTE * & pBuf )
{
	BYTE v = *pBuf;
	if ( !v )
		return 0;
	pBuf++;

	// check for 7-bit case
	if ( v<128 )
		return v;

	// get number of bytes
	int iBytes = 0;
	while ( v & 0x80 )
	{
		iBytes++;
		v <<= 1;
	}

	// check for valid number of bytes
	if ( iBytes<2 || iBytes>4 )
		return -1;

	int iCode = ( v>>iBytes );
	iBytes--;
	do
	{
		if ( !(*pBuf) )
			return 0; // unexpected eof

		if ( ((*pBuf) & 0xC0)!=0x80 )
			return -1; // invalid code

		iCode = ( iCode<<6 ) + ( (*pBuf) & 0x3F );
		iBytes--;
		pBuf++;
	} while ( iBytes );

	// all good
	return iCode;
}


/// UTF-8 encode codepoint to buffer
/// returns number of bytes used
inline int sphUTF8Encode ( BYTE * pBuf, int iCode )
{
	if ( iCode<0x80 )
	{
		pBuf[0] = (BYTE)( iCode & 0x7F );
		return 1;

	} else if ( iCode<0x800 )
	{
		pBuf[0] = (BYTE)( ( (iCode>>6) & 0x1F ) | 0xC0 );
		pBuf[1] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 2;

	} else
	{
		pBuf[0] = (BYTE)( ( (iCode>>12) & 0x0F ) | 0xE0 );
		pBuf[1] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		pBuf[2] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 3;
	}
}


static int sphUTF8Len ( const char * pStr )
{
	BYTE * pBuf = (BYTE*) pStr;
	int iRes = 0, iCode;

	while ( ( iCode=sphUTF8Decode(pBuf) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer::ISphTokenizer ()
	: m_iMinWordLen ( 1 )
	, m_iLastTokenLen ( 0 )
	, m_bTokenBoundary ( false )
	, m_bBoundary ( false )
{}


bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	if ( m_dSynonyms.GetLength() )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}

	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	m_tLC.Reset ();
	m_tLC.AddRemaps ( dRemaps, 0, 0 );
	return true;
}


void ISphTokenizer::AddCaseFolding ( CSphRemapRange & tRange )
{
	CSphVector<CSphRemapRange> dTmp;
	dTmp.Add ( tRange );
	m_tLC.AddRemaps ( dTmp, 0, 0 );
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	m_tLC.AddSpecials ( sSpecials );
}


static int TokenizeOnWhitespace ( CSphVector<CSphString> & dTokens, BYTE * sFrom, bool bUtf8 )
{
	BYTE sAccum [ 3*SPH_MAX_WORD_LEN+16 ];
	BYTE * pAccum = sAccum;
	int iAccum = 0;

	for ( ;; )
	{
		int iCode = bUtf8 ? sphUTF8Decode(sFrom) : *sFrom++;

		// eof or whitespace?
		if ( !iCode || sphIsSpace(iCode) )
		{
			// flush accum
			if ( iAccum )
			{
				*pAccum = '\0';
				dTokens.Add ( (char*)sAccum );

				pAccum = sAccum;
				iAccum = 0;
			}

			// break on eof
			if ( !iCode )
				break;
		} else
		{
			// accumulate everything else
			if ( iAccum<SPH_MAX_WORD_LEN )
			{
				if ( bUtf8 )
				{
					pAccum += sphUTF8Encode ( pAccum, iCode );
					iAccum++;
				} else
				{
					*pAccum++ = BYTE(iCode);
					iAccum++;
				}
			}
		}
	}

	return dTokens.GetLength();
}


static BYTE * sphTrim ( BYTE * s )
{
	// skip to first non-whitespace from start
	while ( *s && sphIsSpace(*s) )
		s++;
	if ( !*s )
		return s;

	// find the end
	BYTE * sEnd = s;
	while ( *sEnd )
		sEnd++;
	sEnd--;

	// skip to first non-whitespace from end
	while ( sEnd>s && sphIsSpace(*sEnd) )
		sEnd--;

	*++sEnd = '\0';
	return s;
}


struct IdentityHash_fn
{
	static inline uint64_t	Hash ( uint64_t iValue )	{ return iValue; }
	static inline DWORD		Hash ( DWORD iValue )		{ return iValue; }
	static inline int		Hash ( int iValue )			{ return iValue; }
};


bool ISphTokenizer::LoadSynonyms ( const char * sFilename, CSphString & sError )
{
	m_dSynonyms.Reset ();

	if ( !sFilename || !*sFilename )
		return true;

	FILE * fp = fopen ( sFilename, "r" );
	if ( !fp )
	{
		sError.SetSprintf ( "failed to open '%s'", sFilename );
		return false;
	}

	int iLine = 0;
	char sBuffer[1024];

	CSphOrderedHash < int, int, IdentityHash_fn, 4096, 117 > hSynonymOnly;
	CSphVector<CSphString> dFrom;

	bool bOK = false;
	for ( ;; )
	{
		char * sGot = fgets ( sBuffer, sizeof(sBuffer), fp );
		if ( !sGot )
		{
			if ( feof(fp) )
				bOK = true;
			break;
		}		

		iLine++;
		dFrom.Resize ( 0 );

		// extract map-from and map-to parts
		char * sSplit = strstr ( sBuffer, "=>" );
		if ( !sSplit )
		{
			sError.SetSprintf ( "%s line %d: mapping token (=>) not found", sFilename, iLine );
			break;
		}

		BYTE * sFrom = (BYTE *) sBuffer;
		BYTE * sTo = (BYTE *)( sSplit + strlen ( "=>" ) );
		*sSplit = '\0';

		// tokenize map-from
		if ( !TokenizeOnWhitespace ( dFrom, sFrom, IsUtf8() ) )
		{
			sError.SetSprintf ( "%s line %d: empty map-from part", sFilename, iLine );
			break;
		}

		// trim map-to
		sTo = sphTrim ( sTo );
		if ( !*sTo )
		{
			sError.SetSprintf ( "%s line %d: empty map-to part", sFilename, iLine );
			break;
		}

		// check lengths
		ARRAY_FOREACH ( i, dFrom )
		{
			int iFromLen = IsUtf8() ? sphUTF8Len ( dFrom[i].cstr() ) : strlen ( dFrom[i].cstr() );
			if ( iFromLen>SPH_MAX_WORD_LEN )
			{
				sError.SetSprintf ( "%s line %d: map-from token too long (over %d bytes)", sFilename, iLine, SPH_MAX_WORD_LEN );
				break;
			}
		}

		int iToLen = IsUtf8() ? sphUTF8Len ( (const char*)sTo ) : strlen ( (const char*)sTo );
		if ( iToLen>SPH_MAX_WORD_LEN )
		{
			sError.SetSprintf ( "%s line %d: map-to token too long (over %d bytes)", sFilename, iLine, SPH_MAX_WORD_LEN );
			break;
		}

		// pack and store it
		int iFromLen = 1;
		ARRAY_FOREACH ( i, dFrom )
			iFromLen += strlen(dFrom[i].cstr()) + 1;

		if ( iFromLen>MAX_SYNONYM_LEN )
		{
			sError.SetSprintf ( "%s line %d: map-from part too long (over %d bytes)", sFilename, iLine, MAX_SYNONYM_LEN );
			break;
		}

		CSphSynonym & tSyn = m_dSynonyms.Add ();
		tSyn.m_sFrom.Reserve ( iFromLen );
		tSyn.m_iFromLen = iFromLen;
		tSyn.m_sTo = (char*)sTo;
		tSyn.m_iToLen = iToLen;

		char * sCur = const_cast<char*> ( tSyn.m_sFrom.cstr() );
		ARRAY_FOREACH ( i, dFrom )
		{
			int iLen = strlen ( dFrom[i].cstr() );
			memcpy ( sCur, dFrom[i].cstr(), iLen );

			sCur[iLen] = MAGIC_SYNONYM_WHITESPACE;
			sCur += iLen+1;
		}
		*sCur++ = '\0';
		assert ( sCur-tSyn.m_sFrom.cstr()==iFromLen );

		// track synonym-only codepoints in map-from
		for ( ;; )
		{
			int iCode = IsUtf8() ? sphUTF8Decode(sFrom) : *sFrom++;
			if ( !iCode )
				break;
			if ( iCode>0 && !sphIsSpace(iCode) && !m_tLC.ToLower(iCode) )
				hSynonymOnly.Add ( 1, iCode );
		}
	}
	fclose ( fp );

	if ( !bOK )
	{
		m_dSynonyms.Reset ();
		return false;
	}

	// sort the list
	m_dSynonyms.Sort ();

	// build simple lookup table
	m_dSynStart.Resize ( 256 );
	m_dSynEnd.Resize ( 256 );
	for ( int i=0; i<256; i++ )
	{
		m_dSynStart[i] = INT_MAX;
		m_dSynEnd[i] = -INT_MAX;
	}
	ARRAY_FOREACH ( i, m_dSynonyms )
	{
		int iCh = *(BYTE*)(m_dSynonyms[i].m_sFrom.cstr());
		m_dSynStart[iCh] = Min ( m_dSynStart[iCh], i );
		m_dSynEnd[iCh] = Max ( m_dSynEnd[iCh], i );
	}

	// add synonym-only remaps
	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Reserve ( hSynonymOnly.GetLength() );

	hSynonymOnly.IterateStart ();
	while ( hSynonymOnly.IterateNext() )
	{
		CSphRemapRange & tRange = dRemaps.Add ();
		tRange.m_iStart = tRange.m_iEnd = tRange.m_iRemapStart = hSynonymOnly.IterateGetKey();
	}

	m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_SYNONYM, 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////

template < bool IS_UTF8 >
CSphTokenizerTraits<IS_UTF8>::CSphTokenizerTraits ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
{
	m_pAccum = m_sAccum;
}

template < bool IS_UTF8 >
const BYTE * CSphTokenizerTraits<IS_UTF8>::GetBufferPtr () const
{
	return m_pCur;
}

template < bool IS_UTF8 >
const BYTE * CSphTokenizerTraits<IS_UTF8>::GetBufferEnd () const
{
	return m_pBufferMax;
}

template < bool IS_UTF8 >
void CSphTokenizerTraits<IS_UTF8>::AdvanceBufferPtr ( int iOffset )
{
	assert ( iOffset >= 0 );
	m_pCur = Min ( m_pBufferMax, m_pCur + iOffset );
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}


enum SynCheck_e
{
	SYNCHECK_LESS,
	SYNCHECK_PARTIAL,
	SYNCHECK_EXACT,
	SYNCHECK_GREATER
};


static inline SynCheck_e SynCheckPrefix ( const CSphSynonym & tCandidate, int iOff, const BYTE * sCur, int iBytes, bool bMaybeSeparator )
{
	const BYTE * sCand = ((const BYTE*)tCandidate.m_sFrom.cstr()) + iOff;

	while ( iBytes-->0 )
	{
		if ( *sCand!=*sCur )
		{
			// incoming synonym-only char vs. ending sequence (eg. 2nd slash in "OS/2/3"); we actually have a match
			if ( bMaybeSeparator && sCand[0]==MAGIC_SYNONYM_WHITESPACE && sCand[1]=='\0' )
				return SYNCHECK_EXACT;

			// otherwise, it is a mismatch
			return ( *sCand<*sCur ) ? SYNCHECK_LESS : SYNCHECK_GREATER;
		}
		sCand++;
		sCur++;
	}

	// full match after a full separator
	if ( sCand[0]=='\0' )
		return SYNCHECK_EXACT;

	// full match after my last synonym-only char
	if ( bMaybeSeparator && sCand[0]==MAGIC_SYNONYM_WHITESPACE && sCand[1]=='\0' )
		return SYNCHECK_EXACT;

	// otherwise, partial match so far
	return SYNCHECK_PARTIAL;
}


static inline bool IsSeparator ( int iFolded, bool bFirst )
{
	// eternal separator
	if ( ( iFolded & MASK_CODEPOINT )==0 )
		return true;

	// just a codepoint
	if (!( iFolded & MASK_FLAGS ))
		return false;

	// any magic flag, besides dual
	if (!( iFolded & FLAG_CODEPOINT_DUAL ))
		return true;

	// dual depends on position
	return bFirst;
}


template < bool IS_UTF8 >
BYTE * CSphTokenizerTraits<IS_UTF8>::GetTokenSyn ()
{
	assert ( m_dSynonyms.GetLength() );

	m_bTokenBoundary = false;
	for ( ;; )
	{
		// initialize accumulators and range
		BYTE * pFirstSeparator = NULL;

		m_iAccum = 0;
		m_pAccum = m_sAccum;

		int iSynStart = 0;
		int iSynEnd = m_dSynonyms.GetLength()-1;
		int iSynOff = 0;

		int iLastFolded = 0;
		BYTE * pRescan = NULL;

		int iExact = -1;
		BYTE * pExact = NULL;

		// main refinement loop
		for ( ;; )
		{
			// store current position (to be able to restart from it on folded boundary)
			BYTE * pCur = m_pCur;

			// get next codepoint
			int iCode = GetCodepoint();
			
			// handle early-out
			if ( iCode<0 )
			{
				// eof at token start? we're done
				if ( iSynOff==0 )
					return NULL;

				// eof after whitespace? we already checked the candidate last time, so break
				if ( iLastFolded==0 )
					break;
			}

			// fold codepoint (and lookup flags!)
			int iFolded = m_tLC.ToLower ( iCode );

			// handle boundaries
			if ( m_bBoundary && ( iFolded==0 ) ) m_bTokenBoundary = true;
			m_bBoundary = ( iFolded & FLAG_CODEPOINT_BOUNDARY )!=0;

			// skip continuous whitespace
			if ( iLastFolded==0 && iFolded==0 )
				continue;
			iLastFolded = iFolded;

			// handle specials at the very word start
			if ( ( iFolded & FLAG_CODEPOINT_SPECIAL ) && m_iAccum==0 )
			{
				AccumCodepoint ( iFolded & MASK_CODEPOINT );
				*m_pAccum = '\0';

				m_iLastTokenLen = 1;
				return m_sAccum;
			}

			// if candidate starts with something special, and turns out to be not a synonym,
			// we will need to rescan from current position later
			if ( iSynOff==0 )
				pRescan = IsSeparator ( iFolded, true ) ? m_pCur : NULL;

			// accumulate folded token
			if ( !pFirstSeparator )
			{
				if ( IsSeparator ( iFolded, m_iAccum==0 ) )
				{
					if ( m_iAccum )
						pFirstSeparator = pCur;

				} else
					AccumCodepoint ( iFolded & MASK_CODEPOINT );
			}

			// accumulate next raw synonym symbol to refine
			// note that we need a special check for whitespace here, to avoid "MS*DOS" being treated as "MS DOS" synonym
			BYTE sTest[4];
			int iTest;

			int iMasked = ( iCode & MASK_CODEPOINT );
			if ( iFolded==0 )
			{
				sTest[0] = MAGIC_SYNONYM_WHITESPACE;
				iTest = 1;

				if (!( iMasked==' ' || iMasked=='\t' ))
				{
					sTest[1] = '\0';
					iTest = 2;
				}
			} else
			{
				if ( IsUtf8() )
				{
					iTest = sphUTF8Encode ( sTest, iMasked );
				} else
				{
					iTest = 1;
					sTest[0] = BYTE(iMasked);
				}
			}

			// refine synonyms range
			#define LOC_RETURN_SYNONYM(_idx) \
			{ \
				strcpy ( (char*)m_sAccum, m_dSynonyms[_idx].m_sTo.cstr() ); \
				m_iLastTokenLen = m_dSynonyms[_idx].m_iToLen; \
				return m_sAccum; \
			}

			#define LOC_REFINE_BREAK() \
			{ \
				if ( iExact>=0 ) { m_pCur = pExact; LOC_RETURN_SYNONYM ( iExact ); } \
				break; \
			}

			// if this is the first symbol, use prebuilt lookup table to speedup initial range search
			if ( iSynOff==0 )
			{
				iSynStart = m_dSynStart[sTest[0]];
				iSynEnd = m_dSynEnd[sTest[0]];
				if ( iSynStart>iSynEnd )
					break;
			}

			// this is to catch intermediate separators (eg. "OS/2/3")
			bool bMaybeSeparator = ( iFolded & FLAG_CODEPOINT_SYNONYM )!=0;

			SynCheck_e eStart = SynCheckPrefix ( m_dSynonyms[iSynStart], iSynOff, sTest, iTest, bMaybeSeparator );
			if ( eStart==SYNCHECK_EXACT )
			{
				if ( iSynStart==iSynEnd ) LOC_RETURN_SYNONYM ( iSynStart );
				iExact = iSynStart;
				pExact = pCur;
			}
			if ( eStart==SYNCHECK_GREATER || ( iSynStart==iSynEnd && eStart!=SYNCHECK_PARTIAL ) )
				LOC_REFINE_BREAK();

			SynCheck_e eEnd = SynCheckPrefix ( m_dSynonyms[iSynEnd], iSynOff, sTest, iTest, bMaybeSeparator );
			if ( eEnd==SYNCHECK_LESS )
				LOC_REFINE_BREAK();
			if ( eEnd==SYNCHECK_EXACT )
			{
				iExact = iSynEnd;
				pExact = pCur;
			}


			// refine left boundary
			if ( eStart!=SYNCHECK_PARTIAL && eStart!=SYNCHECK_EXACT )
			{
				assert ( eStart==SYNCHECK_LESS );

				int iL = iSynStart;
				int iR = iSynEnd;
				SynCheck_e eL = eStart;
				SynCheck_e eR = eEnd;

				while ( iR-iL>1 )
				{
					int iM = iL + (iR-iL)/2;
					SynCheck_e eMid = SynCheckPrefix ( m_dSynonyms[iM], iSynOff, sTest, iTest, bMaybeSeparator );

					if ( eMid==SYNCHECK_LESS )
					{
						iL = iM;
						eL = eMid;
					} else
					{
						iR = iM;
						eR = eMid;
					}
				}

				assert ( eL==SYNCHECK_LESS );
				assert ( eR!=SYNCHECK_LESS );
				assert ( iR-iL==1 );

				if ( eR==SYNCHECK_GREATER )					LOC_REFINE_BREAK();
				if ( eR==SYNCHECK_EXACT && iR==iSynEnd )	LOC_RETURN_SYNONYM ( iR );

				assert ( eR==SYNCHECK_PARTIAL || eR==SYNCHECK_EXACT );
				iSynStart = iR;
				eStart = eR;
			}

			// refine right boundary
			if ( eEnd!=SYNCHECK_PARTIAL && eEnd!=SYNCHECK_EXACT )
			{
				assert ( eEnd==SYNCHECK_GREATER );

				int iL = iSynStart;
				int iR = iSynEnd;
				SynCheck_e eL = eStart;
				SynCheck_e eR = eEnd;

				while ( iR-iL>1 )
				{
					int iM = iL + (iR-iL)/2;
					SynCheck_e eMid = SynCheckPrefix ( m_dSynonyms[iM], iSynOff, sTest, iTest, bMaybeSeparator );

					if ( eMid==SYNCHECK_GREATER )
					{
						iR = iM;
						eR = eMid;
					} else
					{
						iL = iM;
						eL = eMid;
					}
				}

				assert ( eR==SYNCHECK_GREATER );
				assert ( eL!=SYNCHECK_GREATER );
				assert ( iR-iL==1 );

				if ( eL==SYNCHECK_LESS )					LOC_REFINE_BREAK();
				if ( eL==SYNCHECK_EXACT && iL==iSynStart)	LOC_RETURN_SYNONYM ( iL );

				assert ( eL==SYNCHECK_PARTIAL || eL==SYNCHECK_EXACT );
				iSynEnd = iL;
				eEnd = eL;
			}

			// handle eof
			if ( iCode<0 )
				break;

			// we still have a partial synonym match, continue;
			iSynOff += iTest;
		}

		// at this point, that was not a synonym
		if ( pRescan )
		{
			m_pCur = pRescan;
			continue;
		}			

		// at this point, it also started with a valid char
		assert ( m_iAccum>0 );

		// find the proper separator
		if ( !pFirstSeparator )
		{
			// if there was none, scan until found
			for ( ;; )
			{
				BYTE * pCur = m_pCur;
				int iFolded = m_tLC.ToLower ( GetCodepoint() );
				if ( iFolded<0 )
					break; // eof

				if ( IsSeparator ( iFolded, false ) )
				{
					if ( iFolded!=0 )
						m_pCur = pCur; // force rescan
					break;
				}

				AccumCodepoint ( iFolded & MASK_CODEPOINT );
			}
		} else
		{
			// if there was, token is ready but we should restart from that separator
			m_pCur = pFirstSeparator;
		}

		// return accumulated token
		if ( m_iAccum<m_iMinWordLen )
			continue;

		*m_pAccum = '\0';
		m_iLastTokenLen = m_iAccum;
		return m_sAccum;
	}
}


bool ISphTokenizer::SetBoundary ( const char * sConfig, CSphString & sError )
{
	// parse
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	// check
	ARRAY_FOREACH ( i, dRemaps )
	{
		if ( dRemaps[i].m_iStart!=dRemaps[i].m_iRemapStart )
		{
			sError.SetSprintf ( "phrase boundary characters must not be remapped (map-from=U+%x, map-to=U+%x)",
				dRemaps[i].m_iStart, dRemaps[i].m_iRemapStart );
			return false;
		}

		for ( int j=dRemaps[i].m_iStart; j<=dRemaps[i].m_iEnd; j++ )
			if ( m_tLC.ToLower(j) )
		{
			sError.SetSprintf ( "phrase boundary characters must not be referenced anywhere else (code=U+%x)", j );
			return false;
		}
	}

	// add mapping
	m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_BOUNDARY, 0 );
	return true;
}


// FIXME! refactor and merge with SetBoundary()
bool ISphTokenizer::SetIgnoreChars ( const char * sConfig, CSphString & sError )
{
	// parse
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	// check
	ARRAY_FOREACH ( i, dRemaps )
	{
		if ( dRemaps[i].m_iStart!=dRemaps[i].m_iRemapStart )
		{
			sError.SetSprintf ( "ignored characters must not be remapped (map-from=U+%x, map-to=U+%x)",
				dRemaps[i].m_iStart, dRemaps[i].m_iRemapStart );
			return false;
		}

		for ( int j=dRemaps[i].m_iStart; j<=dRemaps[i].m_iEnd; j++ )
			if ( m_tLC.ToLower(j) )
		{
			sError.SetSprintf ( "ignored characters must not be referenced anywhere else (code=U+%x)", j );
			return false;
		}
	}

	// add mapping
	m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_IGNORE, 0 );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_SBCS::CSphTokenizer_SBCS ()
{
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_SBCS_TABLE, sTmp );
}


void CSphTokenizer_SBCS::SetBuffer ( BYTE * sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
}


BYTE * CSphTokenizer_SBCS::GetToken ()
{
	if ( m_dSynonyms.GetLength() )
		return GetTokenSyn ();

	m_bTokenBoundary = false;
	for ( ;; )
	{
		// get next codepoint
		int iCode = 0;
		if ( m_pCur>=m_pBufferMax )
		{
			if ( m_iAccum<m_iMinWordLen )
			{
				m_bBoundary = m_bTokenBoundary = false;
				m_iAccum = 0;
				m_iLastTokenLen = 0;
				return NULL;
			}
		} else
		{
			iCode = m_tLC.ToLower ( *m_pCur++ );
		}

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) ) m_bTokenBoundary = true;
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;
		if ( iCode==0 || m_bBoundary )
		{
			if ( m_iAccum<m_iMinWordLen )
			{
				m_iAccum = 0;
				continue;
			}

			m_iLastTokenLen = m_iAccum;
			m_sAccum[m_iAccum] = '\0';
			m_iAccum = 0;
			return m_sAccum;
		}

		// handle specials
		// duals in the middle of the word are handled as if they were just plain codepoints
		bool bSpecial =
			( iCode & FLAG_CODEPOINT_SPECIAL ) &&
			!( ( iCode & FLAG_CODEPOINT_DUAL ) && m_iAccum );
		iCode &= MASK_CODEPOINT;

		if ( bSpecial )
		{
			// skip short words
			if ( m_iAccum<m_iMinWordLen )
				m_iAccum = 0;

			if ( m_iAccum==0 )
			{
				// nice standalone special
				m_iLastTokenLen = 1;
				m_sAccum[0] = (BYTE)iCode;
				m_sAccum[1] = '\0';
			} else
			{
				// flush prev accum and redo this special
				m_iLastTokenLen = m_iAccum;
				m_sAccum[m_iAccum] = '\0';
				m_pCur--;
			}

			m_iAccum = 0;
			return m_sAccum;
		}

		// just accumulate
		assert ( iCode>0 );
		if ( m_iAccum<SPH_MAX_WORD_LEN )
			m_sAccum[m_iAccum++] = (BYTE)iCode;
	}
}

ISphTokenizer * CSphTokenizer_SBCS::Clone () const
{
	CSphTokenizer_SBCS * pClone = new CSphTokenizer_SBCS ();
	pClone->m_tLC = m_tLC;
	pClone->m_dSynonyms = m_dSynonyms;
	pClone->m_dSynStart = m_dSynStart;
	pClone->m_dSynEnd = m_dSynEnd;
	return pClone;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_UTF8::CSphTokenizer_UTF8 ()
{
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE, sTmp );
}


void CSphTokenizer_UTF8::SetBuffer ( BYTE * sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;

	// fixup embedded zeroes with spaces
	for ( BYTE * p = m_pBuffer; p < m_pBufferMax; p++ )
		if ( !*p )
			*p = ' ';
}


BYTE * CSphTokenizer_UTF8::GetToken ()
{
	if ( m_dSynonyms.GetLength() )
		return GetTokenSyn ();

	m_bTokenBoundary = false;
	for ( ;; )
	{
		// get next codepoint
		BYTE * pCur = m_pCur; // to redo special char, if there's a token already
		int iCode = m_tLC.ToLower ( GetCodepoint() ); // advances m_pCur

		// handle eof
		if ( iCode<0 )
		{
			m_bBoundary = m_bTokenBoundary = false;

			// skip trailing short word
			FlushAccum ();
			if ( m_iLastTokenLen<m_iMinWordLen )
			{
				m_iLastTokenLen = 0;
				return NULL;
			}

			// return trailing word
			return m_sAccum;
		}

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) ) m_bTokenBoundary = true;
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;
		if ( iCode==0 || m_bBoundary )
		{
			FlushAccum ();
			if ( m_iLastTokenLen<m_iMinWordLen )
				continue;
			else
				return m_sAccum;
		}

		// handle specials
		// duals in the middle of the word are handled as if they were just plain codepoints
		bool bSpecial =
			( iCode & FLAG_CODEPOINT_SPECIAL ) &&
			!( ( iCode & FLAG_CODEPOINT_DUAL ) && m_iAccum );
		iCode &= MASK_CODEPOINT;

		if ( bSpecial )
		{
			// skip short words preceding specials
			if ( m_iAccum<m_iMinWordLen )
				FlushAccum ();

			if ( m_iAccum==0 )
			{
				AccumCodepoint ( iCode ); // handle special as a standalone token
			} else
			{
				m_pCur = pCur; // we need to flush current accum and then redo special char again
			}

			FlushAccum ();
			return m_sAccum;
		}

		// just accumulate
		AccumCodepoint ( iCode );
	}
}


void CSphTokenizer_UTF8::FlushAccum ()
{
	assert ( m_pAccum-m_sAccum < (int)sizeof(m_sAccum) );
	m_iLastTokenLen = m_iAccum;
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}


ISphTokenizer * CSphTokenizer_UTF8::Clone () const
{
	CSphTokenizer_UTF8 * pClone = new CSphTokenizer_UTF8 ();
	pClone->m_tLC = m_tLC;
	pClone->m_dSynonyms = m_dSynonyms;
	pClone->m_dSynStart = m_dSynStart;
	pClone->m_dSynEnd = m_dSynEnd;
	return pClone;
}


int CSphTokenizer_UTF8::GetCodepointLength ( int iCode ) const
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

bool CSphTokenizer_UTF8Ngram::SetNgramChars ( const char * sConfig, CSphString & sError )
{
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	m_tLC.AddRemaps ( dRemaps,
		FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL,
		FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL ); // !COMMIT support other n-gram lengths than 1
	return true;
}


void CSphTokenizer_UTF8Ngram::SetNgramLen ( int iLen )
{
	assert ( iLen>0 );
	m_iNgramLen = iLen;
}


BYTE * CSphTokenizer_UTF8Ngram::GetToken ()
{
	// !COMMIT support other n-gram lengths than 1
	assert ( m_iNgramLen==1 );
	return CSphTokenizer_UTF8::GetToken ();
}

/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////

CSphFilter::CSphFilter ()
	: m_sAttrName	( "" )
	, m_bExclude	( false )
	, m_uMinValue	( 0 )
	, m_uMaxValue	( UINT_MAX )
	, m_bMva		( false )
	, m_iBitOffset	( -1 )
	, m_iBitCount	( -1 )
{}


CSphFilter::CSphFilter ( const CSphFilter & rhs )
{
	assert ( 0 );
	(*this) = rhs;
}


bool CSphFilter::operator == ( const CSphFilter & rhs ) const
{
	// check name, mode, type
	if ( m_sAttrName!=rhs.m_sAttrName || m_bExclude!=rhs.m_bExclude || m_eType==rhs.m_eType )
		return false;

	switch ( m_eType )
	{
		case SPH_FILTER_RANGE:
			return m_uMinValue==rhs.m_uMinValue && m_uMaxValue==rhs.m_uMaxValue;

		case SPH_FILTER_VALUES:
			if ( m_dValues.GetLength()!=rhs.m_dValues.GetLength() )
				return false;

			ARRAY_FOREACH ( i, m_dValues )
				if ( m_dValues[i]!=rhs.m_dValues[i] )
					return false;

			return true;

		default:
			assert ( 0 && "internal error: unhandled filter type in comparison" );
			return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////

CSphQuery::CSphQuery ()
	: m_iOffset		( 0 )
	, m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_eMode		( SPH_MATCH_ALL )
	, m_eRanker		( SPH_RANK_DEFAULT )
	, m_eSort		( SPH_SORT_RELEVANCE )
	, m_iMaxMatches	( 1000 )
	, m_iMinID		( 0 )
	, m_iMaxID		( DOCID_MAX )
	, m_sGroupSortBy	( "@group desc" )
	, m_sGroupDistinct	( "" )
	, m_iCutoff			( 0 )
	, m_iRetryCount		( 0 )
	, m_iRetryDelay		( 0 )
	, m_bGeoAnchor		( false )
	, m_fGeoLatitude	( 0.0f )
	, m_fGeoLongitude	( 0.0f )
	, m_uMaxQueryMsec	( 0 )

	, m_bCalcGeodist	( false )
	, m_iPresortRowitems( -1 )
	, m_iGroupbyOffset	( -1 )
	, m_iGroupbyCount	( -1 )
	, m_iDistinctOffset	( -1 )
	, m_iDistinctCount	( -1 )

	, m_iOldVersion		( 0 )
	, m_iOldGroups		( 0 )
	, m_pOldGroups		( NULL )
	, m_iOldMinTS		( 0 )
	, m_iOldMaxTS		( UINT_MAX )
	, m_iOldMinGID		( 0 )
	, m_iOldMaxGID		( UINT_MAX )
{}


int CSphQuery::GetIndexWeight ( const char * sName ) const
{
	ARRAY_FOREACH ( i, m_dIndexWeights )
		if ( m_dIndexWeights[i].m_sName==sName )
			return m_dIndexWeights[i].m_iValue;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// SCHEMA
/////////////////////////////////////////////////////////////////////////////

ESphSchemaCompare CSphSchema::CompareTo ( const CSphSchema & rhs, CSphString & sError ) const
{
	/////////////////////////
	// incompatibility tests
	/////////////////////////

	// check attrs count
	if ( GetAttrsCount()!=rhs.GetAttrsCount() )
	{
		sError.SetSprintf ( "attribute count mismatch: %d in schema '%s', %d in schema '%s'",
			GetAttrsCount(), m_sName.cstr(), rhs.GetAttrsCount(), rhs.m_sName.cstr() );
		return SPH_SCHEMAS_INCOMPATIBLE;
	}

	// check attr types
	ARRAY_FOREACH ( i, m_dAttrs )
		if ( rhs.m_dAttrs[i].m_eAttrType!=m_dAttrs[i].m_eAttrType )
	{
		sError.SetSprintf ( "attribute %d/%d type mismatch: name='%s', type=%d in schema '%s'; name='%s', type=%d in schema '%s'",
			i, m_dAttrs.GetLength(),
			m_dAttrs[i].m_sName.cstr(), m_dAttrs[i].m_eAttrType, m_sName.cstr(),
			rhs.m_dAttrs[i].m_sName.cstr(), rhs.m_dAttrs[i].m_eAttrType, rhs.m_sName.cstr() );
		return SPH_SCHEMAS_INCOMPATIBLE;
	}

	//////////////////
	// equality tests
	//////////////////

	// check fulltext fields count
	if ( rhs.m_dFields.GetLength()!=m_dFields.GetLength() )
	{
		sError.SetSprintf ( "fulltext fields count mismatch: %d in schema '%s', %d in schema '%s'",
			m_dFields.GetLength(), m_sName.cstr(),
			rhs.m_dFields.GetLength(), rhs.m_sName.cstr() );
		return SPH_SCHEMAS_COMPATIBLE;
	}

	// check fulltext field names
	ARRAY_FOREACH ( i, rhs.m_dFields )
		if ( rhs.m_dFields[i].m_sName!=m_dFields[i].m_sName )
	{
		sError.SetSprintf ( "fulltext field %d/%d name mismatch: name='%s' in schema '%s'; name='%s' in schema '%s'",
			i, m_dFields.GetLength(),
			m_dFields[i].m_sName.cstr(), m_sName.cstr(),
			rhs.m_dFields[i].m_sName.cstr(), rhs.m_sName.cstr() );
		return SPH_SCHEMAS_COMPATIBLE;
	}

	// check attr names
	ARRAY_FOREACH ( i, m_dAttrs )
		if ( rhs.m_dAttrs[i].m_sName!=m_dAttrs[i].m_sName )
	{
		sError.SetSprintf ( "attribute %d/%d name mismatch: name='%s' in schema '%s', name='%s' in schema '%s'",
			i, m_dAttrs.GetLength(),
			m_dAttrs[i].m_sName.cstr(), m_sName.cstr(),
			rhs.m_dAttrs[i].m_sName.cstr(), rhs.m_sName.cstr() );
		return SPH_SCHEMAS_COMPATIBLE;
	}

	return SPH_SCHEMAS_EQUAL;
}


int CSphSchema::GetFieldIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;
	ARRAY_FOREACH ( i, m_dFields )
		if ( m_dFields[i].m_sName==sName )
			return i;
	return -1;
}


int CSphSchema::GetAttrIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;
	ARRAY_FOREACH ( i, m_dAttrs )
		if ( m_dAttrs[i].m_sName==sName )
			return i;
	return -1;
}


void CSphSchema::Reset ()
{
	m_dFields.Reset();
	ResetAttrs ();
}


void CSphSchema::ResetAttrs ()
{
	m_dAttrs.Reset();
	m_dRowUsed.Reset();
}


void CSphSchema::AddAttr ( const CSphColumnInfo & tCol )
{
	assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
	if ( tCol.m_eAttrType==SPH_ATTR_NONE )
		return;

	m_dAttrs.Add ( tCol );

	int iBits = ROWITEM_BITS;
	if ( tCol.m_iBitCount>0 )
		iBits = tCol.m_iBitCount;
	if ( tCol.m_eAttrType==SPH_ATTR_BOOL )
		iBits = 1;

	m_dAttrs.Last().m_iBitCount = iBits;

	if ( iBits==ROWITEM_BITS )
	{
		m_dAttrs.Last().m_iRowitem = m_dRowUsed.GetLength();
		m_dAttrs.Last().m_iBitOffset = m_dRowUsed.GetLength()*ROWITEM_BITS;
		m_dRowUsed.Add ( ROWITEM_BITS );

	} else
	{
		int iItem;
		for ( iItem=0; iItem<m_dRowUsed.GetLength(); iItem++ )
			if ( m_dRowUsed[iItem]+iBits<=ROWITEM_BITS )
				break;
		if ( iItem==m_dRowUsed.GetLength() )
			m_dRowUsed.Add(0);

		m_dAttrs.Last().m_iRowitem = -1;
		m_dAttrs.Last().m_iBitOffset = iItem*ROWITEM_BITS + m_dRowUsed[iItem];
		m_dRowUsed[iItem] += iBits;
	}
}


void CSphSchema::BuildResultSchema ( const CSphQuery * pQuery )
{
	if ( pQuery->m_iGroupbyOffset<0 )
		return;

	CSphColumnInfo tGroupby ( "@groupby", SPH_ATTR_INTEGER );
	CSphColumnInfo tCount ( "@count", SPH_ATTR_INTEGER );
	CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );

	AddAttr ( tGroupby );
	AddAttr ( tCount );
	if ( pQuery->m_iDistinctOffset>=0 )
		AddAttr ( tDistinct );
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE OUTPUT
///////////////////////////////////////////////////////////////////////////////

CSphWriter::CSphWriter ()
	: m_sName ( "" )
	, m_iPos ( -1 )

	, m_iFD ( -1 )
	, m_iPoolUsed ( 0 )
	, m_pBuffer ( NULL )
	, m_pPool ( NULL )

	, m_bError ( false )
	, m_pError ( NULL )
{
}


bool CSphWriter::OpenFile ( const char * sName, CSphString & sErrorBuffer )
{
	assert ( sName );
	assert ( m_iFD<0 && "already open" );

	m_sName = sName;
	m_pError = &sErrorBuffer;

	if ( !m_pBuffer )
		m_pBuffer = new BYTE [ SPH_CACHE_WRITE ];

	m_iFD = ::open ( m_sName.cstr(), SPH_O_NEW, 0644 );
	m_pPool = m_pBuffer;
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_bError = ( m_iFD<0 );

	if ( m_bError )
		m_pError->SetSprintf ( "failed to create %s: %s" , sName, strerror(errno) );

	return !m_bError;
}


CSphWriter::~CSphWriter ()
{
	CloseFile ();
	SafeDeleteArray ( m_pBuffer );
}


void CSphWriter::CloseFile () 
{
	if ( m_iFD>=0 )
	{
		Flush ();
		::close ( m_iFD );
		m_iFD = -1;
	}
}


void CSphWriter::PutByte ( int data )
{
	*m_pPool++ = BYTE( data & 0xff );
	m_iPoolUsed++;
	if ( m_iPoolUsed==SPH_CACHE_WRITE )
		Flush ();
	m_iPos++;
}


void CSphWriter::PutBytes ( const void * pData, int iSize )
{
	const BYTE * pBuf = (const BYTE *) pData;
	while ( iSize>0 )
	{
		int iPut = Min ( iSize, SPH_CACHE_WRITE );
		if ( m_iPoolUsed+iPut>SPH_CACHE_WRITE )
			Flush ();
		assert ( m_iPoolUsed+iPut<=SPH_CACHE_WRITE );

		memcpy ( m_pPool, pBuf, iPut );
		m_pPool += iPut;
		m_iPoolUsed += iPut;
		m_iPos += iPut;

		pBuf += iPut;
		iSize -= iPut;
	}
}


void CSphWriter::ZipInt ( DWORD uValue )
{
	int iBytes = 1;

	DWORD u = ( uValue>>7 );
	while ( u )
	{
		u >>= 7;
		iBytes++;
	}

	while ( iBytes-- )
		PutByte (
			( 0x7f & ( uValue>>(7*iBytes) ) )
			| ( iBytes ? 0x80 : 0 ) );
}


void CSphWriter::ZipOffset ( SphOffset_t uValue )
{
	int iBytes = 1;

	uint64_t u = ((uint64_t)uValue)>>7;
	while ( u )
	{
		u >>= 7;
		iBytes++;
	}

	while ( iBytes-- )
		PutByte (
			( 0x7f & (DWORD)( uValue>>(7*iBytes) ) )
			| ( iBytes ? 0x80 : 0 ) );
}


void CSphWriter::ZipOffsets ( CSphVector<SphOffset_t> * pData )
{
	assert ( pData );

	SphOffset_t * pValue = &((*pData)[0]);
	int n = pData->GetLength ();

	while ( n-->0 )
	{
		SphOffset_t uValue = *pValue++;

		int iBytes = 1;

		uint64_t u = ((uint64_t)uValue)>>7;
		while ( u )
		{
			u >>= 7;
			iBytes++;
		}

		while ( iBytes-- )
			PutByte (
				( 0x7f & (DWORD)( uValue>>(7*iBytes) ) )
				| ( iBytes ? 0x80 : 0 ) );
	}
}


void CSphWriter::Flush ()
{
	PROFILE ( write_hits );

	if ( !sphWriteThrottled ( m_iFD, m_pBuffer, m_iPoolUsed, m_sName.cstr(), *m_pError ) )
		m_bError = true;

	m_iPoolUsed = 0;
	m_pPool = m_pBuffer;
}


void CSphWriter::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );

	Flush ();
	sphSeek ( m_iFD, iPos, SEEK_SET );
	m_iPos = iPos;
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE INPUT
///////////////////////////////////////////////////////////////////////////////

CSphReader_VLN::CSphReader_VLN ( BYTE * pBuf, int iSize )
	: m_iFD ( -1 )
	, m_iPos ( 0 )
	, m_iBuffPos ( 0 )
	, m_iBuffUsed ( 0 )
	, m_pBuff ( pBuf )
	, m_iSizeHint ( 0 )
	, m_iBufSize ( iSize )
	, m_bBufOwned ( false )
	, m_bError ( false )
{
	assert ( pBuf==NULL || iSize>0 );
}


CSphReader_VLN::~CSphReader_VLN ()
{
	if ( m_bBufOwned )
		SafeDeleteArray ( m_pBuff );
}


void CSphReader_VLN::SetFile ( int iFD, const char * sFilename )
{
	m_iFD = iFD;
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
	m_sFilename = sFilename;
}


void CSphReader_VLN::SetFile ( const CSphAutofile & tFile )
{
	SetFile ( tFile.GetFD(), tFile.GetFilename() );
}


void CSphReader_VLN::Reset ()
{
	SetFile ( -1, "" );
}


void CSphReader_VLN::SeekTo ( SphOffset_t iPos, int iSizeHint )
{
	assert ( iPos>=0 );
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
		m_iSizeHint = iSizeHint;
	}
}


void CSphReader_VLN::SkipBytes ( int iCount )
{
	SeekTo ( m_iPos+m_iBuffPos+iCount, m_iSizeHint-m_iBuffPos-iCount );
}


void CSphReader_VLN::UpdateCache ()
{
	PROFILE ( read_hits );
	assert ( m_iFD>=0 );

	// alloc buf on first actual read
	if ( !m_pBuff )
	{
		m_bBufOwned = true;
		m_iBufSize = READ_BUFFER_SIZE;
		m_pBuff = new BYTE [ m_iBufSize ];
	}

	// stream position could be changed externally
	// so let's just hope that the OS optimizes redundant seeks
	SphOffset_t iNewPos = m_iPos + Min ( m_iBuffPos, m_iBuffUsed );
	if ( sphSeek ( m_iFD, iNewPos, SEEK_SET )==-1 )
	{
		// unexpected io failure
		m_iBuffPos = 0;
		m_iBuffUsed = 0;

		m_bError = true;
		m_sError.SetSprintf ( "seek error in %s: pos=%"PRIi64", code=%d, msg=%s",
			m_sFilename.cstr(), (int64_t)iNewPos, errno, strerror(errno) );
		return;
	}

	if ( m_iSizeHint<=0 )
		m_iSizeHint = READ_DEFAULT_BLOCK;

	int iReadLen = Min ( m_iSizeHint, m_iBufSize );
	m_iBuffPos = 0;
	m_iBuffUsed = sphReadThrottled ( m_iFD, m_pBuff, iReadLen );
	m_iPos = iNewPos;

	if ( m_iBuffUsed>0 )
	{
		// all fine, adjust hint
		m_iSizeHint -= m_iBuffUsed;
	} else
	{
		// unexpected io failure
		m_bError = true;
		m_sError.SetSprintf ( "read error in %s: pos=%"PRIi64", len=%d, code=%d, msg=%s",
			m_sFilename.cstr(), (int64_t)m_iPos, iReadLen, errno, strerror(errno) );
	}
}


int CSphReader_VLN::GetByte ()
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


void CSphReader_VLN::GetRawBytes ( void * pData, int iSize )
{
	if ( m_iBuffPos+iSize>m_iBuffUsed )
	{
		UpdateCache ();
		if ( m_iBuffPos+iSize>m_iBuffUsed )
		{
			memset ( pData, 0, iSize ); // unexpected io failure
			return;
		}
	}

	assert ( (m_iBuffPos+iSize)<=m_iBuffUsed );
	memcpy ( pData, m_pBuff+m_iBuffPos, iSize );
	m_iBuffPos += iSize;
}


void CSphReader_VLN::GetBytes ( void *data, int size )
{
	BYTE * b = (BYTE*) data;

	while ( size-->0 )
		*b++ = (BYTE) GetByte ();
}


int CSphReader_VLN::GetLine ( char * sBuffer, int iMaxLen )
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
	for ( ;; )
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

/////////////////////////////////////////////////////////////////////////////

#if PARANOID

#define SPH_UNZIP_IMPL(_type,_getexpr) \
	register DWORD b = 0; \
	register _type v = 0; \
	int it = 0; \
	do { b = _getexpr; v = ( v<<7 ) + ( b&0x7f ); it++; } while ( b&0x80 ); \
	assert ( (it-1)*7<=sizeof(_type)*8 ); \
	return v;

#else

#define SPH_UNZIP_IMPL(_type,_getexpr) \
	register DWORD b = 0; \
	register _type v = 0; \
	do { b = _getexpr; v = ( v<<7 ) + ( b&0x7f ); } while ( b&0x80 ); \
	return v;

#endif // PARANOID

DWORD sphUnzipInt ( const BYTE * & pBuf )			{ SPH_UNZIP_IMPL ( DWORD, *pBuf++ ); }
SphOffset_t sphUnzipOffset ( const BYTE * & pBuf )	{ SPH_UNZIP_IMPL ( SphOffset_t, *pBuf++ ); }

DWORD CSphReader_VLN::UnzipInt ()			{ SPH_UNZIP_IMPL ( DWORD, GetByte() ); }
SphOffset_t CSphReader_VLN::UnzipOffset ()	{ SPH_UNZIP_IMPL ( SphOffset_t, GetByte() ); }


#if USE_64BIT
#define sphUnzipWordid sphUnzipOffset
#else
#define sphUnzipWordid sphUnzipInt
#endif

/////////////////////////////////////////////////////////////////////////////

const CSphReader_VLN & CSphReader_VLN::operator = ( const CSphReader_VLN & rhs )
{
	SetFile ( rhs.m_iFD, rhs.m_sFilename.cstr() );
	SeekTo ( rhs.m_iPos + rhs.m_iBuffPos, rhs.m_iSizeHint );
	return *this;
}


DWORD CSphReader_VLN::GetDword ()
{
	DWORD uRes;
	GetRawBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


SphOffset_t CSphReader_VLN::GetOffset ()
{
	SphOffset_t uRes;
	GetRawBytes ( &uRes, sizeof(SphOffset_t) );
	return uRes;
}


CSphString CSphReader_VLN::GetString ()
{
	CSphString sRes;

	DWORD iLen = GetDword ();
	if ( iLen )
	{
		char * sBuf = new char [ iLen ];
		GetRawBytes ( sBuf, iLen );
		sRes.SetBinary ( sBuf, iLen );
		SafeDeleteArray ( sBuf );
	} else
	{
		sRes = "@emptyname";
	}

	return sRes;
}

/////////////////////////////////////////////////////////////////////////////
// QUERY RESULT
/////////////////////////////////////////////////////////////////////////////

CSphQueryResult::CSphQueryResult ()
	: m_tSchema ( "query_result" )
{
	for ( int i=0; i<SPH_MAX_QUERY_WORDS; i++ )
	{
		m_tWordStats[i].m_iDocs = 0;
		m_tWordStats[i].m_iHits = 0;
	}

	m_iNumWords = 0;
	m_iQueryTime = 0;
	m_iTotalMatches = 0;
	m_pMva = NULL;
	m_iOffset = 0;
	m_iCount = 0;
	m_iSuccesses = 0;
}


CSphQueryResult::~CSphQueryResult ()
{
}

/////////////////////////////////////////////////////////////////////////////
// ATTR UPDATE
/////////////////////////////////////////////////////////////////////////////

CSphAttrUpdate_t::CSphAttrUpdate_t ()
	: m_iUpdates ( 0 )
	, m_pUpdates ( NULL )
{}


CSphAttrUpdate_t::~CSphAttrUpdate_t ()
{
	SafeDeleteArray ( m_pUpdates );
}

/////////////////////////////////////////////////////////////////////////////
// CHUNK READER
/////////////////////////////////////////////////////////////////////////////

CSphBin::CSphBin ()
	: m_dBuffer ( NULL )
	, m_pCurrent ( NULL )
	, m_iLeft ( 0 )
	, m_iDone ( 0 )
	, m_eState ( BIN_POS )
	, m_iFile ( -1 )
	, m_pFilePos ( NULL )
	, m_iFilePos ( 0 )
	, m_iFileLeft ( 0 )
{
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

	m_tLastHit.m_iDocID = 0;
	m_tLastHit.m_iWordID = 0;
	m_tLastHit.m_iWordPos = 0;
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
		PROFILE ( read_hits );
		if ( *m_pFilePos!=m_iFilePos )
		{
			sphSeek ( m_iFile, m_iFilePos, SEEK_SET );
			*m_pFilePos = m_iFilePos;
		}

		int n = m_iFileLeft > m_iSize
			? m_iSize
			: (int)m_iFileLeft;
		if (n == 0)
		{
			m_iDone = 1;
			m_iLeft = 1;
		} else
		{
			assert ( m_dBuffer );

			if ( sphReadThrottled ( m_iFile, m_dBuffer, n )!=n )
				return -2;
			m_iLeft = n;

			m_iFilePos += n;
			m_iFileLeft -= n;
			m_pCurrent = m_dBuffer;
			*m_pFilePos += n;
		}
	}
	if ( m_iDone )
		return -1;

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
			sphSeek ( m_iFile, m_iFilePos, SEEK_SET );
			*m_pFilePos = m_iFilePos;
		}

		int n = Min ( m_iFileLeft, m_iSize - m_iLeft );
		if ( n==0 )
		{
			m_iDone = 1;
			return BIN_READ_EOF;
		}

		assert ( m_dBuffer );
		memmove ( m_dBuffer, m_pCurrent, m_iLeft );

		if ( sphReadThrottled ( m_iFile, m_dBuffer + m_iLeft, n )!=n )
			return BIN_READ_ERROR;

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


int CSphBin::ReadHit ( CSphWordHit * e, int iRowitems, CSphRowitem * pRowitems )
{
	// expected EOB
	if ( m_iDone )
	{
		e->m_iWordID = 0;
		return 1;
	}

	CSphWordHit & tHit = m_tLastHit; // shortcut
	for ( ;; )
	{
		SphWordID_t uDelta;
		int iOff, iTmp;

		// read VLB-encoded delta
		uDelta = 0;
		iOff = 0;
		do
		{
			if ( ( iTmp = ReadByte() )<0 )
				return 0;
			uDelta += ( (SphWordID_t(iTmp & 0x7f)) << iOff );
			iOff += 7;
		} while ( iTmp & 0x80 );

		if ( uDelta )
		{
			switch ( m_eState )
			{
				case BIN_WORD:
					tHit.m_iWordID += uDelta;
					tHit.m_iDocID = 0;
					tHit.m_iWordPos = 0;
					m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					m_eState = BIN_POS;
					tHit.m_iDocID += uDelta;
					tHit.m_iWordPos = 0;

					for ( int i=0; i<iRowitems; i++, pRowitems++ )
					{
						// read VLB-encoded rowitem
						uDelta = 0;
						iOff = 0;
						do
						{
							if ( ( iTmp = ReadByte() )<0 )
								return 0;
							uDelta += ( (SphWordID_t(iTmp & 0x7f)) << iOff );
							iOff += 7;
						} while ( iTmp & 0x80 );

						// emit it
						*pRowitems = (DWORD)uDelta; // FIXME? check range?
					}
					break;

				case BIN_POS: 
					tHit.m_iWordPos += (DWORD)uDelta; // FIXME? check range?
					*e = tHit;
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
				case BIN_WORD:	m_iDone = 1; e->m_iWordID = 0; return 1;
				default:		sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphIndex::CSphIndex ( const char * sName )
	: m_pProgress ( NULL )
	, m_tSchema ( sName )
	, m_sLastError ( "(no error message)" )
	, m_iMinPrefixLen ( 0 )
	, m_iMinInfixLen ( 0 )
	, m_iBoundaryStep ( 0 )
	, m_bAttrsUpdated ( false )
{
}


void CSphIndex::SetInfixIndexing ( int iPrefixLen, int iInfixLen )
{
	m_iMinPrefixLen = Max ( iPrefixLen, 0 );
	m_iMinInfixLen = Max ( iInfixLen, 0 );
}


void CSphIndex::SetBoundaryStep ( int iBoundaryStep )
{
	m_iBoundaryStep = Max ( iBoundaryStep, 0 );
}

/////////////////////////////////////////////////////////////////////////////

CSphIndex * sphCreateIndexPhrase ( const char * sFilename, bool bEnableStar )
{
	return new CSphIndex_VLN ( sFilename, bEnableStar );
}


CSphIndex_VLN::CSphIndex_VLN ( const char * sFilename, bool bEnableStar )
	: CSphIndex ( sFilename )
	, m_iLockFD ( -1 )
	, m_bEnableStar ( bEnableStar )
{
	m_sFilename = sFilename;

	m_pWriteBuffer = NULL;

	m_tLastHit.m_iDocID = 0;
	m_tLastHit.m_iWordID = 0;
	m_tLastHit.m_iWordPos = 0;
	m_iLastHitlistPos = 0;
	m_uLastDocFields = 0;
	m_uLastDocHits = 0;
	m_iLastDoclistPos = 0;
	m_iLastWordDocs = 0;
	m_iLastWordHits = 0;
	m_iWordlistEntries = 0;

	m_uDocinfo = 0;
	m_eDocinfo = SPH_DOCINFO_NONE;

	m_bPreallocated = false;
	m_uVersion = INDEX_FORMAT_VERSION;
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	SafeDeleteArray ( m_pWriteBuffer );
}


/////////////////////////////////////////////////////////////////////////////

int CSphIndex_VLN::UpdateAttributes ( const CSphAttrUpdate_t & tUpd )
{
	// check if we can
	if ( m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		m_sLastError.SetSprintf ( "docinfo=extern required for updates" );
		return -1;
	}

	// check if we have to
	if ( !m_uDocinfo || !tUpd.m_iUpdates )
		return 0;

	// check if we have what
	assert ( tUpd.m_pUpdates );

	// remap update schema to index schema
	CSphVector<int> dAttrIndex;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		int iIndex = m_tSchema.GetAttrIndex ( tUpd.m_dAttrs[i].m_sName.cstr() );
		if ( iIndex<0 )
		{
			m_sLastError.SetSprintf ( "attribute '%s' not found", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}
		dAttrIndex.Add ( iIndex );
	}
	assert ( dAttrIndex.GetLength()==tUpd.m_dAttrs.GetLength() );

	// do update
	int iUpdated = 0;
	int iStride = DOCINFO_IDSIZE + dAttrIndex.GetLength();
	const DWORD * pUpdate = tUpd.m_pUpdates;

	for ( int iUpd=0; iUpd<tUpd.m_iUpdates; iUpd++, pUpdate+=iStride )
	{
		DWORD * pEntry = const_cast < DWORD * > ( FindDocinfo ( DOCINFO2ID(pUpdate ) ) );
		if ( !pEntry )
			continue;

		assert ( DOCINFO2ID(pEntry)==DOCINFO2ID(pUpdate) );
		pEntry = DOCINFO2ATTRS(pEntry);

		ARRAY_FOREACH ( i, dAttrIndex )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr ( dAttrIndex[i] );
			if ( tCol.m_iRowitem>=0 )
				pEntry [ tCol.m_iRowitem ] = pUpdate [ DOCINFO_IDSIZE+i ];
			else
				sphSetRowAttr ( pEntry, tCol.m_iBitOffset, tCol.m_iBitCount, pUpdate [ DOCINFO_IDSIZE+i ] );
		}

		iUpdated++;
	}

	if ( iUpdated>0 )
		m_bAttrsUpdated = true;

	return iUpdated;
}


bool CSphIndex_VLN::SaveAttributes ()
{
	if ( !m_bAttrsUpdated || !m_uDocinfo )
		return true;
	assert ( m_eDocinfo==SPH_DOCINFO_EXTERN && m_uDocinfo && m_pDocinfo.GetWritePtr() );

	// save current state
	CSphAutofile fdTmpnew ( GetIndexFileName("spa.tmpnew"), SPH_O_NEW, m_sLastError );
	if ( fdTmpnew.GetFD()<0 )
		return false;

	size_t uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	size_t uSize = uStride*size_t(m_uDocinfo)*sizeof(DWORD);

	if ( !sphWriteThrottled ( fdTmpnew.GetFD(), m_pDocinfo.GetWritePtr(), uSize, "docinfo", m_sLastError ) )
		return false;

	fdTmpnew.Close ();

	// do some juggling
	CSphString sSpa = GetIndexFileName ( "spa" );
	CSphString sSpaNew = GetIndexFileName ( "spa.tmpnew" );
	CSphString sSpaOld = GetIndexFileName ( "spa.tmpold" );

	if ( ::rename ( sSpa.cstr(), sSpaOld.cstr() ) )
	{
		m_sLastError.SetSprintf ( "rename '%s' to '%s' failed: %s",
			sSpa.cstr(), sSpaOld.cstr(), strerror(errno) );
		return false;
	}

	if ( ::rename ( sSpaNew.cstr(), sSpa.cstr() ) )
	{
		if ( ::rename ( sSpaOld.cstr(), sSpa.cstr() ) )
		{
			// rollback failed too!
			m_sLastError.SetSprintf ( "rollback rename to '%s' failed: %s; INDEX UNUSABLE; FIX FILE NAMES MANUALLY",
				sSpa.cstr(), strerror(errno) );
		} else
		{
			// rollback went ok
			m_sLastError.SetSprintf ( "rename '%s' to '%s' failed: %s",
				sSpaNew.cstr(), sSpa.cstr(), strerror(errno) );
		}
		return false;
	}

	// all done
	::unlink ( sSpaOld.cstr() );

	m_bAttrsUpdated = false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////

#define SPH_CMPHIT_LESS(a,b) \
	(a.m_iWordID < b.m_iWordID || \
	(a.m_iWordID == b.m_iWordID && a.m_iDocID < b.m_iDocID) || \
	(a.m_iWordID == b.m_iWordID && a.m_iDocID == b.m_iDocID && a.m_iWordPos < b.m_iWordPos))


struct CmpHit_fn
{
	inline int operator () ( const CSphWordHit & a, const CSphWordHit & b )
	{
		return SPH_CMPHIT_LESS ( a, b );
	}
};


/// sort baked docinfos by document ID
void sphSortDocinfos ( DWORD * pBuf, int iCount, int iStride )
{
	int st0[32], st1[32];
	int a, b, k, i, j;

	k = 1;
	st0[0] = 0;
	st1[0] = iCount-1;
	DWORD * pTmp = new DWORD [ iStride ];
	while ( k!=0 )
	{
		k--;
		i = a = st0[k];
		j = b = st1[k];
		SphDocID_t iMedian = DOCINFO2ID ( pBuf + ((a+b)/2)*iStride );
		while ( a<b )
		{
			while ( i<=j )
			{
				while ( DOCINFO2ID ( pBuf + i*iStride ) < iMedian ) i++;
				while ( iMedian < DOCINFO2ID ( pBuf + j*iStride ) ) j--;
				if ( i<=j )
				{
					memcpy ( pTmp, pBuf+i*iStride, iStride*sizeof(DWORD) );
					memcpy ( pBuf+i*iStride, pBuf+j*iStride, iStride*sizeof(DWORD) );
					memcpy ( pBuf+j*iStride, pTmp, iStride*sizeof(DWORD) );
					i++;
					j--;
				}
			}
			if ( j-a>=b-i )
			{
				if ( a<j ) { st0[k] = a; st1[k] = j; k++; }
				a = i;
			} else
			{
				if ( i<b ) { st0[k] = i; st1[k] = b; k++; }
				b = j;
			}
		}
	}
	SafeDeleteArray ( pTmp );
}


const char * CSphIndex_VLN::GetIndexFileName ( const char * sExt ) const
{
	static char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s.%s", m_sFilename.cstr(), sExt );
	return sBuf;
}


void CSphIndex_VLN::cidxHit ( CSphWordHit * hit, CSphRowitem * pAttrs )
{
	assert (
		( hit->m_iWordID!=0 && hit->m_iWordPos!=0 && hit->m_iDocID!=0 ) || // it's either ok hit
		( hit->m_iWordID==0 && hit->m_iWordPos==0 ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	if ( m_tLastHit.m_iWordID!=hit->m_iWordID )
	{
		// close prev hitlist, if any
		if ( m_tLastHit.m_iWordPos )
		{
			m_wrHitlist.ZipInt ( 0 );
			m_tLastHit.m_iWordPos = 0;
		}

		// flush prev doclist, if any
		if ( m_tLastHit.m_iDocID )
		{
			// finish wordlist entry
			assert ( m_iLastWordDocs );
			assert ( m_iLastWordHits );
			m_wrWordlist.ZipInt ( m_iLastWordDocs );
			m_wrWordlist.ZipInt ( m_iLastWordHits );

			// finish doclist entry
			m_wrDoclist.ZipInt ( m_uLastDocFields );
			m_wrDoclist.ZipInt ( m_uLastDocHits );
			m_uLastDocFields = 0;
			m_uLastDocHits = 0;

			// emit end-of-doclist marker
			m_wrDoclist.ZipInt ( 0 );

			// reset trackers
			m_iLastWordDocs = 0;
			m_iLastWordHits = 0;

			m_tLastHit.m_iDocID = 0;
			m_iLastHitlistPos = 0;
			m_iWordlistEntries++;
		}

		// flush wordlist, if this is the end
		if ( !hit->m_iWordPos )
		{
			assert ( m_wrDoclist.GetPos()>=m_iLastDoclistPos );
			m_wrWordlist.ZipInt ( 0 ); // indicate checkpoint
			m_wrWordlist.ZipOffset ( m_wrDoclist.GetPos() - m_iLastDoclistPos ); // store last doclist length
			return;
		}

		// insert wordlist checkpoint (ie. restart delta coding) once per WORDLIST_CHECKPOINT entries
		if ( m_iWordlistEntries==WORDLIST_CHECKPOINT )
		{
			assert ( m_wrDoclist.GetPos() > m_iLastDoclistPos );
			m_wrWordlist.ZipInt ( 0 ); // indicate checkpoint
			m_wrWordlist.ZipOffset ( m_wrDoclist.GetPos() - m_iLastDoclistPos ); // store last hitlist length
			m_tLastHit.m_iWordID = 0;
			m_iLastDoclistPos = 0;
			m_iLastWordDocs = 0;
			m_iLastWordHits = 0;

			m_iWordlistEntries = 0;
		}

		// begin new wordlist entry
		if ( m_iWordlistEntries==0 )
		{
			assert ( m_wrWordlist.GetPos()<=UINT_MAX );

			CSphWordlistCheckpoint tCheckpoint;
			tCheckpoint.m_iWordID = hit->m_iWordID;
			tCheckpoint.m_iWordlistOffset = (DWORD) m_wrWordlist.GetPos();

			m_dWordlistCheckpoints.Add ( tCheckpoint );
		}

		assert ( hit->m_iWordID > m_tLastHit.m_iWordID );
		assert ( m_wrDoclist.GetPos() > m_iLastDoclistPos );
		m_wrWordlist.ZipOffset ( hit->m_iWordID - m_tLastHit.m_iWordID ); // FIXME! slow with 32bit wordids
		m_wrWordlist.ZipOffset ( m_wrDoclist.GetPos() - m_iLastDoclistPos );
		m_tLastHit.m_iWordID = hit->m_iWordID;
		m_iLastDoclistPos = m_wrDoclist.GetPos();
	}

	////////////
	// next doc
	////////////

	if ( m_tLastHit.m_iDocID!=hit->m_iDocID )
	{
		// close prev hitlist, if any
		if ( m_tLastHit.m_iWordPos )
		{
			m_wrHitlist.ZipInt ( 0 );
			m_tLastHit.m_iWordPos = 0;
		}

		// close prev doc, if any
		if ( m_tLastHit.m_iDocID )
		{
			// flush matched fields mask
			m_wrDoclist.ZipInt ( m_uLastDocFields );
			m_wrDoclist.ZipInt ( m_uLastDocHits );
			m_uLastDocFields = 0;
			m_uLastDocHits = 0;
		}

		// add new doclist entry for new doc id
		assert ( hit->m_iDocID > m_tLastHit.m_iDocID );
		assert ( m_wrHitlist.GetPos() > m_iLastHitlistPos );

		m_wrDoclist.ZipOffset ( hit->m_iDocID - m_tLastHit.m_iDocID );
		if ( pAttrs )
			for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
				m_wrDoclist.ZipInt ( pAttrs[i] - m_tMin.m_pRowitems[i] );
		m_wrDoclist.ZipOffset ( m_wrHitlist.GetPos() - m_iLastHitlistPos );

		m_tLastHit.m_iDocID = hit->m_iDocID;
		m_iLastHitlistPos = m_wrHitlist.GetPos();

		// update per-word stats
		m_iLastWordDocs++;
	}

	///////////
	// the hit
	///////////

	// add hit delta
	if ( hit->m_iWordPos==m_tLastHit.m_iWordPos )
		return;

	assert ( hit->m_iWordPos > m_tLastHit.m_iWordPos );	
	m_wrHitlist.ZipInt ( hit->m_iWordPos - m_tLastHit.m_iWordPos );
	m_tLastHit.m_iWordPos = hit->m_iWordPos;
	m_iLastWordHits++;	

	// update matched fields mask
	m_uLastDocFields |= ( 1UL << (hit->m_iWordPos>>24) );
	m_uLastDocHits++;
}


void CSphIndex_VLN::WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol )
{
	int iLen = strlen ( tCol.m_sName.cstr() );
	fdInfo.PutDword ( iLen );
	fdInfo.PutBytes ( tCol.m_sName.cstr(), iLen );
	fdInfo.PutDword ( tCol.m_eAttrType );

	fdInfo.PutDword ( tCol.m_iRowitem );
	fdInfo.PutDword ( tCol.m_iBitOffset );
	fdInfo.PutDword ( tCol.m_iBitCount );
}


void CSphIndex_VLN::ReadSchemaColumn ( CSphReader_VLN & rdInfo, CSphColumnInfo & tCol )
{
	tCol.m_sName = rdInfo.GetString ();
	tCol.m_sName.ToLower ();
	tCol.m_eAttrType = rdInfo.GetDword ();

	if ( m_uVersion>=5 )
	{
		tCol.m_iRowitem = rdInfo.GetDword ();
		tCol.m_iBitOffset = rdInfo.GetDword ();
		tCol.m_iBitCount = rdInfo.GetDword ();
	} else
	{
		tCol.m_iRowitem = -1;
		tCol.m_iBitOffset = -1;
		tCol.m_iBitCount = -1;
	}
}


bool CSphIndex_VLN::cidxDone ()
{
	// flush wordlist checkpoints
	SphOffset_t iCheckpointsPos = m_wrWordlist.GetPos();
	if ( m_dWordlistCheckpoints.GetLength() )
		m_wrWordlist.PutBytes ( &m_dWordlistCheckpoints[0], m_dWordlistCheckpoints.GetLength()*sizeof(CSphWordlistCheckpoint) );

	/////////////////
	// create header
	/////////////////

	CSphWriter fdInfo;
	fdInfo.OpenFile ( GetIndexFileName("sph"), m_sLastError );
	if ( fdInfo.IsError() )
		return false;

	// version
	fdInfo.PutDword ( INDEX_MAGIC_HEADER );
	fdInfo.PutDword ( INDEX_FORMAT_VERSION );

	// bits
	fdInfo.PutDword ( USE_64BIT );

	// docinfo
	fdInfo.PutDword ( m_eDocinfo );

	// schema
	fdInfo.PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		WriteSchemaColumn ( fdInfo, m_tSchema.m_dFields[i] );

	fdInfo.PutDword ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( fdInfo, m_tSchema.GetAttr(i) );

	// min doc
	fdInfo.PutOffset ( m_tMin.m_iDocID ); // was dword in v.1
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		fdInfo.PutBytes ( m_tMin.m_pRowitems, m_tMin.m_iRowitems*sizeof(CSphRowitem) );

	// wordlist checkpoints
	fdInfo.PutBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	fdInfo.PutDword ( m_dWordlistCheckpoints.GetLength() );

	// index stats
	fdInfo.PutDword ( m_tStats.m_iTotalDocuments );
	fdInfo.PutOffset ( m_tStats.m_iTotalBytes );

	// prefix/infix info
	fdInfo.PutDword ( m_iMinPrefixLen );
	fdInfo.PutDword ( m_iMinInfixLen );

	////////////////////////
	// close all data files
	////////////////////////

	fdInfo.CloseFile ();
	m_wrWordlist.CloseFile ();
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ();

	if ( fdInfo.IsError() || m_wrWordlist.IsError() || m_wrDoclist.IsError() || m_wrHitlist.IsError() )
		return false;

	return true;
}


inline int encodeVLB(BYTE *buf, DWORD v)
{
	register BYTE b;
	register int n = 0;

	do {
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if (v) b |= 0x80;
		*buf++ = b;
		n++;
	} while (v);
	return n;
}


inline int encodeVLB8 ( BYTE * buf, uint64_t v )
{
	register BYTE b;
	register int n = 0;

	do {
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if (v) b |= 0x80;
		*buf++ = b;
		n++;
	} while (v);
	return n;
}


int CSphIndex_VLN::cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits, DWORD * pDocinfo, int iDocinfos, int iStride )
{
	PROFILE ( write_hits );

	assert ( pHit );
	assert ( iHits>0 );

	/////////////////////////////
	// do simple bitwise hashing
	/////////////////////////////

	static const int HBITS = 11;
	static const int HSIZE = (1<<HBITS);

	SphDocID_t iStartID = 0;
	int dHash [ HSIZE+1 ];
	int iShift = 0;

	if ( pDocinfo )
	{
		iStartID = DOCINFO2ID ( pDocinfo );
		int iBits = sphLog2 ( DOCINFO2ID ( pDocinfo + (iDocinfos-1)*iStride ) - iStartID );
		iShift = ( iBits<HBITS ) ? 0 : ( iBits-HBITS );

		#ifndef NDEBUG
		for ( int i=0; i<=HSIZE; i++ )
			dHash[i] = -1;
		#endif

		dHash[0] = 0;
		int iHashed = 0;
		for ( int i=0; i<iDocinfos; i++ )
		{
			int iHash = (int)( ( DOCINFO2ID(pDocinfo+i*iStride)-iStartID )>>iShift );
			assert ( iHash>=0 && iHash<HSIZE );

			if ( iHash>iHashed )
			{
				dHash [ iHashed+1 ] = i-1; // right boundary for prev hash value
				dHash [ iHash ] = i; // left boundary for next hash value
				iHashed = iHash;
			}
		}
		dHash [ iHashed+1 ] = iDocinfos-1; // right boundary for last hash value
	}

	///////////////////////////////////////
	// encode through a small write buffer
	///////////////////////////////////////

	BYTE *pBuf, *maxP;
	int n = 0, w;
	SphWordID_t d1, l1=0;
	SphDocID_t d2, l2=0;
	DWORD d3, l3=0;

	pBuf = m_pWriteBuffer;
	maxP = m_pWriteBuffer + WRITE_BUFFER_SIZE - 128;

	SphDocID_t iAttrID = 0; // current doc id
	DWORD * pAttrs = NULL; // current doc attrs

	while ( iHits-- )
	{
		// find attributes by id
		if ( pDocinfo && iAttrID!=pHit->m_iDocID )
		{
			int iHash = (int)( ( pHit->m_iDocID-iStartID )>>iShift );
			assert ( iHash>=0 && iHash<HSIZE );

			int iStart = dHash[iHash];
			int iEnd = dHash[iHash+1];

			if ( pHit->m_iDocID==DOCINFO2ID ( pDocinfo + iStart*iStride ) )
			{
				pAttrs = DOCINFO2ATTRS ( pDocinfo + iStart*iStride );

			} else if ( pHit->m_iDocID==DOCINFO2ID ( pDocinfo + iEnd*iStride ) )
			{
				pAttrs = DOCINFO2ATTRS ( pDocinfo + iEnd*iStride );

			} else
			{
				pAttrs = NULL;
				while ( iEnd-iStart>1 )
				{
					// check if nothing found
					if (
						pHit->m_iDocID < DOCINFO2ID ( pDocinfo + iStart*iStride ) ||
						pHit->m_iDocID > DOCINFO2ID ( pDocinfo + iEnd*iStride ) )
							break;
					assert ( pHit->m_iDocID > DOCINFO2ID ( pDocinfo + iStart*iStride ) );
					assert ( pHit->m_iDocID < DOCINFO2ID ( pDocinfo + iEnd*iStride ) );

					int iMid = iStart + (iEnd-iStart)/2;
					if ( pHit->m_iDocID==DOCINFO2ID ( pDocinfo + iMid*iStride ) )
					{
						pAttrs = DOCINFO2ATTRS ( pDocinfo + iMid*iStride );
						break;
					}
					if ( pHit->m_iDocID<DOCINFO2ID ( pDocinfo + iMid*iStride ) )
						iEnd = iMid;
					else
						iStart = iMid;
				}
			}

			assert ( pAttrs );
			assert ( DOCINFO2ID(pAttrs-DOCINFO_IDSIZE )==pHit->m_iDocID );
			iAttrID = pHit->m_iDocID;
		}

		// calc deltas
		d1 = pHit->m_iWordID - l1;
		d2 = pHit->m_iDocID - l2;
		d3 = pHit->m_iWordPos - l3;

		// ignore duplicate hits
		if ( d1==0 && d2==0 && d3==0 ) // OPTIMIZE? check if ( 0==(d1|d2|d3) ) is faster
		{
			pHit++;
			continue;
		}

		// non-zero delta restarts all the fields after it
		// because their deltas might now be negative
		if ( d1 ) d2 = pHit->m_iDocID;
		if ( d2 ) d3 = pHit->m_iWordPos;

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 ) pBuf += encodeVLB ( pBuf, 0 );

		// encode deltas
#if USE_64BIT
		if ( d1 ) pBuf += encodeVLB8 ( pBuf, d1 ); // encode word delta
		if ( d2 ) pBuf += encodeVLB8 ( pBuf, d2 ); // encode doc id (whole or delta)
#else
		if ( d1 ) pBuf += encodeVLB ( pBuf, d1 ); // encode word delta
		if ( d2 ) pBuf += encodeVLB ( pBuf, d2 ); // encode doc id (whole or delta)
#endif

		// encode attrs
		if ( d2 && pAttrs )
		{
			for ( int i=0; i<iStride-1; i++ )
				pBuf += encodeVLB ( pBuf, pAttrs[i] );
		}

		assert ( d3 );
		pBuf += encodeVLB ( pBuf, d3 );

		// update current state
		l1 = pHit->m_iWordID;
		l2 = pHit->m_iDocID;
		l3 = pHit->m_iWordPos;

		pHit++;

		if ( pBuf>maxP )
		{
			w = (int)(pBuf - m_pWriteBuffer);
			if ( !sphWriteThrottled ( fd, m_pWriteBuffer, w, "raw_hits", m_sLastError ) )
				return -1;
			n += w;
			pBuf = m_pWriteBuffer;
		}
	}
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	w = (int)(pBuf - m_pWriteBuffer);
	if ( !sphWriteThrottled ( fd, m_pWriteBuffer, w, "raw_hits", m_sLastError ) )
		return -1;
	n += w;

	return n;
}

/////////////////////////////////////////////////////////////////////////////

/// hit priority queue entry
struct CSphHitQueueEntry : public CSphWordHit
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
	CSphHitQueue ( int iSize )
	{
		assert ( iSize>0 );
		m_iSize = iSize;
		m_iUsed = 0;
		m_pData = new CSphHitQueueEntry [ iSize ];
	}

	/// destory queue
	~CSphHitQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	void Push ( CSphWordHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		m_pData [ m_iUsed ].m_iDocID = tHit.m_iDocID;
		m_pData [ m_iUsed ].m_iWordID = tHit.m_iWordID;
		m_pData [ m_iUsed ].m_iWordPos = tHit.m_iWordPos;
		m_pData [ m_iUsed ].m_iBin = iBin;

		int iEntry = m_iUsed++;

		// sift up if needed
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) >> 1;
			if ( SPH_CMPHIT_LESS ( m_pData[iEntry], m_pData[iParent] ) )
			{
				// entry is less than parent, should float to the top
				Swap ( m_pData[iEntry], m_pData[iParent] );
				iEntry = iParent;
			} else
			{
				break;
			}
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
		for ( ;; )
		{
			// select child
			int iChild = (iEntry<<1) + 1;
			if ( iChild>=m_iUsed )
				break;

			// select smallest child
			if ( iChild+1<m_iUsed )
				if ( SPH_CMPHIT_LESS ( m_pData[iChild+1], m_pData[iChild] ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( SPH_CMPHIT_LESS ( m_pData[iChild], m_pData[iEntry] ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}
};


struct CmpQueuedDocinfo_fn
{
	static DWORD *	m_pStorage;
	static int		m_iStride;

	static inline bool IsLess ( const int a, const int b )
	{
		return DOCINFO2ID ( m_pStorage + a*m_iStride ) < DOCINFO2ID ( m_pStorage + b*m_iStride );
	};
};
DWORD *		CmpQueuedDocinfo_fn::m_pStorage		= NULL;
int			CmpQueuedDocinfo_fn::m_iStride		= 1;


int CSphIndex_VLN::AdjustMemoryLimit ( int iMemoryLimit )
{
	const int MIN_MEM_LIMIT = sizeof(CSphWordHit)*1048576 + sizeof(DWORD)*( 1+m_tSchema.GetRowSize() )*65536;

	bool bRelimit = false;
	int iOldLimit = 0;

	if ( iMemoryLimit<MIN_MEM_LIMIT )
	{
		iOldLimit = iMemoryLimit;
		iMemoryLimit = MIN_MEM_LIMIT;
		bRelimit = true;
	}

	iMemoryLimit = ( ( (iMemoryLimit+32768) >> 16 ) << 16 ); // round to 64K

	if ( bRelimit && iOldLimit )
	{
		sphWarn ( "collect_hits: mem_limit=%d kb too low, increasing to %d kb",
			iOldLimit/1024, iMemoryLimit/1024 );
	}

	return iMemoryLimit;
}

/////////////////////////////////////////////////////////////////////////////

struct MvaEntry_t
{
	SphDocID_t	m_uDocID;
	int			m_iAttr;
	DWORD		m_uValue;

	inline bool operator < ( const MvaEntry_t & rhs ) const
	{
		if ( m_uDocID!=rhs.m_uDocID ) return m_uDocID<rhs.m_uDocID;
		if ( m_iAttr!=rhs.m_iAttr ) return m_iAttr<rhs.m_iAttr;
		return m_uValue<rhs.m_uValue;
	}
};


struct MvaEntryTag_t : public MvaEntry_t
{
	int			m_iTag;
};


struct MvaEntryCmp_fn
{
	static inline bool IsLess ( const MvaEntry_t & a, const MvaEntry_t & b )
	{
		return a<b;
	};
};


bool CSphIndex_VLN::BuildMVA ( const CSphVector<CSphSource*> & dSources, CSphAutoArray<CSphWordHit> & dHits, int iArenaSize,
							  int iFieldFD, int nFieldMVAs, int iFieldMVAInPool )
{
	// initialize writer (data file must always exist)
	CSphWriter wrMva;
	if ( !wrMva.OpenFile ( GetIndexFileName("spm"), m_sLastError ) )
		return false;

	// calcs and checks
	bool bOnlyFieldMVAs = true;
	CSphVector<int> dMvaIndexes;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
		{
			dMvaIndexes.Add ( i );
			if ( tAttr.m_eSrc != SPH_ATTRSRC_FIELD )
				bOnlyFieldMVAs = false;
		}
	}

	if ( dMvaIndexes.GetLength()<=0 )
		return true;

	// reuse hits pool
	CSphWordHit * pArena = dHits;
	MvaEntry_t * pMvaPool = (MvaEntry_t*) pArena;
	MvaEntry_t * pMvaMax = pMvaPool + ( iArenaSize/sizeof(MvaEntry_t) );
	MvaEntry_t * pMva = pMvaPool;

	// create temp file
	CSphAutofile fdTmpMva ( GetIndexFileName("tmp3"), SPH_O_NEW, m_sLastError );
	if ( fdTmpMva.GetFD()<0 )
		return false;

	//////////////////////////////
	// collect and partially sort
	//////////////////////////////

	CSphVector<int> dBlockLens;
	dBlockLens.Reserve ( 1024 );

	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT_MVA;
	m_tProgress.m_iAttrs = 0;

	if ( !bOnlyFieldMVAs )
	{
		ARRAY_FOREACH ( iSource, dSources )
		{
			CSphSource * pSource = dSources[iSource];
			if ( !pSource->Connect ( m_sLastError ) )
				return false;

			ARRAY_FOREACH ( i, dMvaIndexes )
			{
				int iAttr = dMvaIndexes[i];
				const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);

				if ( tAttr.m_eSrc == SPH_ATTRSRC_FIELD )
					continue;

				int iRowitem = tAttr.m_iRowitem;
				assert ( iRowitem>=0 );

				if ( !pSource->IterateMultivaluedStart ( iAttr, m_sLastError ) )
					return false;

				while ( pSource->IterateMultivaluedNext () )
				{
					pMva->m_uDocID = pSource->m_tDocInfo.m_iDocID;
					pMva->m_iAttr = i;
					pMva->m_uValue = pSource->m_tDocInfo.m_pRowitems[iRowitem];

					if ( ++pMva>=pMvaMax )
					{
						sphSort ( pMvaPool, pMva-pMvaPool );
						if ( !sphWriteThrottled ( fdTmpMva.GetFD(), pMvaPool, (pMva-pMvaPool)*sizeof(MvaEntry_t), "temp_mva", m_sLastError ) )
							return false;

						dBlockLens.Add ( pMva-pMvaPool );
						m_tProgress.m_iAttrs += pMva-pMvaPool;
						pMva = pMvaPool;

						if ( m_pProgress )
							m_pProgress ( &m_tProgress, false );
					}
				}
			}

			pSource->Disconnect ();
		}

		if ( pMva>pMvaPool )
		{
			sphSort ( pMvaPool, pMva-pMvaPool );
			if ( !sphWriteThrottled ( fdTmpMva.GetFD(), pMvaPool, (pMva-pMvaPool)*sizeof(MvaEntry_t), "temp_mva", m_sLastError ) )
				return false;

			dBlockLens.Add ( pMva-pMvaPool );
			m_tProgress.m_iAttrs += pMva-pMvaPool;
			pMva = pMvaPool;
		}
	}

	if ( m_pProgress )
		m_pProgress ( &m_tProgress, true );

	///////////////////////////
	// free memory for sorting
	///////////////////////////

	dHits.Reset ();

	//////////////
	// fully sort
	//////////////

	if ( m_pProgress )
	{
		m_tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT_MVA;
		m_tProgress.m_iAttrs = m_tProgress.m_iAttrs + nFieldMVAs;
		m_tProgress.m_iAttrsTotal = m_tProgress.m_iAttrs;
		m_pProgress ( &m_tProgress, false );
	}

	int	nLastBlockFieldMVAs = iFieldMVAInPool ? ( nFieldMVAs % iFieldMVAInPool ) : 0;
	int nFieldBlocks = iFieldMVAInPool ? ( nFieldMVAs / iFieldMVAInPool + ( nLastBlockFieldMVAs ? 1 : 0 ) ) : 0;

	// initialize readers
	CSphVector<CSphBin*> dBins;
	dBins.Reserve ( dBlockLens.GetLength() + nFieldBlocks );

	int iBinSize = CSphBin::CalcBinSize ( iArenaSize, dBlockLens.GetLength() + nFieldBlocks, "sort_mva" );
	SphOffset_t iSharedOffset = -1;

	ARRAY_FOREACH ( i, dBlockLens )
	{
		dBins.Add ( new CSphBin() );
		dBins[i]->m_iFileLeft = dBlockLens[i]*sizeof(MvaEntry_t);
		dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( fdTmpMva.GetFD(), &iSharedOffset, iBinSize );
	}

	SphOffset_t iSharedFieldOffset = -1;
	SphOffset_t uStart = 0;
	for ( int i = 0; i < nFieldBlocks; i++ )
	{
		dBins.Add ( new CSphBin() );
		int iBin = dBins.GetLength () - 1;

		dBins[iBin]->m_iFileLeft = ( i == nFieldBlocks - 1 ? ( nLastBlockFieldMVAs ? nLastBlockFieldMVAs : iFieldMVAInPool ): iFieldMVAInPool ) * sizeof ( MvaEntry_t );
		dBins[iBin]->m_iFilePos = uStart;
		dBins[iBin]->Init ( iFieldFD, &iSharedFieldOffset, iBinSize );

		uStart += dBins [iBin]->m_iFileLeft;
	}

	// do the sort
	CSphQueue < MvaEntryTag_t, MvaEntryCmp_fn > qMva ( Max ( 1, dBins.GetLength() ) );
	ARRAY_FOREACH ( i, dBins )
	{
		MvaEntryTag_t tEntry;
		if ( dBins[i]->ReadBytes ( (MvaEntry_t*) &tEntry, sizeof(MvaEntry_t) ) != BIN_READ_OK )
		{
			m_sLastError.SetSprintf ( "sort_mva: warmup failed (io error?)" );
			return false;
		}

		tEntry.m_iTag = i;
		qMva.Push ( tEntry );
	}

	// spm-file := info-list [ 0+ ]
	// info-list := docid, values-list [ index.schema.mva-count ]
	// values-list := values-count, value [ values-count ]
	SphDocID_t uCurID = 0;
	CSphVector < CSphVector<DWORD> > dCurInfo;
	dCurInfo.Resize ( dMvaIndexes.GetLength() );

	for ( ;; )
	{
		// flush previous per-document info-list
		if ( !qMva.GetLength() || qMva.Root().m_uDocID!=uCurID )
		{
			if ( uCurID )
			{
				wrMva.PutDocid ( uCurID );
				ARRAY_FOREACH ( i, dCurInfo )
				{
					int iValues = dCurInfo[i].GetLength();
					wrMva.PutDword ( iValues );
					if ( iValues )
						wrMva.PutBytes ( &dCurInfo[i][0], iValues*sizeof(DWORD) );
				}
			}

			if ( !qMva.GetLength() )
				break;

			uCurID = qMva.Root().m_uDocID;
			ARRAY_FOREACH ( i, dCurInfo )
				dCurInfo[i].Resize ( 0 );
		}

		// accumulate this entry
#if PARANOID
		assert ( dCurInfo[ qMva.Root().m_iAttr ].GetLength()==0
			|| dCurInfo [ qMva.Root().m_iAttr ].Last() <= qMva.Root().m_uValue );
#endif
		dCurInfo [ qMva.Root().m_iAttr ].AddUnique ( qMva.Root().m_uValue );

		// get next entry
		int iBin = qMva.Root().m_iTag;
		qMva.Pop ();

		MvaEntryTag_t tEntry;
		ESphBinRead iRes = dBins[iBin]->ReadBytes ( (MvaEntry_t*)&tEntry, sizeof(MvaEntry_t) );
		tEntry.m_iTag = iBin;

		if ( iRes == BIN_READ_OK )
			qMva.Push ( tEntry );

		if ( iRes == BIN_READ_ERROR )
		{
			m_sLastError.SetSprintf ( "sort_mva: read error" );
			return false;
		}
	}

	// clean up readers
	ARRAY_FOREACH ( i, dBins )
		SafeDelete ( dBins[i] );

	wrMva.CloseFile ();
	if ( wrMva.IsError() )
		return false;

	if ( m_pProgress )
		m_pProgress ( &m_tProgress, true );

	fdTmpMva.Close ();
	unlink ( GetIndexFileName("tmp3") );
	return true;
}


struct CmpOrdinalsValue_fn
{
	inline int operator () ( const Ordinal_t & a, const Ordinal_t & b )
	{
		return strcmp ( a.m_sValue.cstr(), b.m_sValue.cstr() )<0;
	}
};

struct CmpOrdinalsEntry_fn
{
	static inline bool IsLess ( const OrdinalEntry_t & a, const OrdinalEntry_t & b )
	{
		return strcmp ( a.m_sValue.cstr(), b.m_sValue.cstr() )<0;
	}
};

struct CmpOrdinalsDocid_fn
{
	inline int operator () ( const OrdinalId_t & a, const OrdinalId_t & b )
	{
		return a.m_uDocID < b.m_uDocID;
	}
};


struct CmpMvaEntries_fn
{
	inline int operator () ( const MvaEntry_t & a, const MvaEntry_t & b )
	{
		return a<b;
	}
};

struct CmpOrdinalIdEntry_fn
{
	static inline bool IsLess ( const OrdinalIdEntry_t & a, const OrdinalIdEntry_t & b )
	{
		return a.m_uDocID < b.m_uDocID;
	}
};


SphOffset_t CSphIndex_VLN::DumpOrdinals ( CSphWriter & Writer, CSphVector<Ordinal_t> & dOrdinals )
{
	SphOffset_t uSize = ( sizeof ( SphDocID_t ) + sizeof ( DWORD ) ) * dOrdinals.GetLength ();

	ARRAY_FOREACH ( i, dOrdinals )
	{
		Ordinal_t & Ord = dOrdinals [i];

		DWORD uValueLen = Ord.m_sValue.cstr () ? strlen ( Ord.m_sValue.cstr () ) : 0;
		Writer.PutBytes ( &(Ord.m_uDocID), sizeof ( Ord.m_uDocID ) );
		Writer.PutBytes ( &uValueLen, sizeof ( uValueLen ) );
		Writer.PutBytes ( Ord.m_sValue.cstr (), uValueLen );
		uSize += uValueLen;

		if ( Writer.IsError () )
			return 0;
	}

	return uSize;
}


ESphBinRead CSphIndex_VLN::ReadOrdinal ( CSphBin & Reader, Ordinal_t & Ordinal )
{
	ESphBinRead eRes = Reader.ReadBytes ( &Ordinal.m_uDocID, sizeof ( Ordinal.m_uDocID ) );
	if ( eRes != BIN_READ_OK )
		return eRes;

	DWORD uStrLen;
	eRes = Reader.ReadBytes ( &uStrLen, sizeof ( DWORD ) );
	if ( eRes != BIN_READ_OK )
		return eRes;

	if ( uStrLen >= (DWORD)MAX_ORDINAL_STR_LEN )
		return BIN_READ_ERROR;

	char dBuffer [MAX_ORDINAL_STR_LEN];

	if ( uStrLen > 0 )
	{
		eRes = Reader.ReadBytes ( dBuffer, uStrLen );
		if ( eRes != BIN_READ_OK )
			return eRes;
	}

	dBuffer [uStrLen] = '\0';
	Ordinal.m_sValue = dBuffer;

	return BIN_READ_OK;
}


bool CSphIndex_VLN::SortOrdinals ( const char * szToFile, int iFromFD, int iArenaSize, int iOrdinalsInPool, const CSphVector < CSphVector < SphOffset_t > > & dOrdBlockSize, bool bWarnOfMem )
{
	int nAttrs = dOrdBlockSize.GetLength ();
	int nBlocks = dOrdBlockSize [0].GetLength ();

	CSphWriter Writer;
	if ( !Writer.OpenFile ( szToFile, m_sLastError ) )
		return false;

	int iBinSize = CSphBin::CalcBinSize ( iArenaSize, nBlocks, "ordinals", bWarnOfMem );
	SphOffset_t iSharedOffset = -1;

	CSphQueue < OrdinalEntry_t, CmpOrdinalsEntry_fn > qOrdinals ( Max ( 1, nBlocks ) );
	OrdinalEntry_t tOrdinalEntry;
	DWORD uOrdinalId = 0;

	CSphVector < OrdinalId_t > dOrdinalIdPool;
	dOrdinalIdPool.Reserve ( nBlocks );

	CSphVector < CSphVector < SphOffset_t > > dStarts;
	dStarts.Resize ( nAttrs );
	ARRAY_FOREACH ( i, dStarts )
		dStarts [i].Resize ( nBlocks );

	SphOffset_t uStart = 0;
	for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
		for ( int iAttr = 0; iAttr < nAttrs; iAttr++ )
		{
			dStarts [iAttr][iBlock] = uStart;
			uStart += dOrdBlockSize [iAttr][iBlock];
		}

	for ( int iAttr = 0; iAttr < nAttrs; iAttr++ )
	{
		CSphVector < CSphBin > dBins;
		dBins.Resize ( nBlocks );

		ARRAY_FOREACH ( i, dBins )
		{
			dBins [i].m_iFileLeft = (int)dOrdBlockSize [iAttr][i];
			dBins [i].m_iFilePos = dStarts [iAttr][i];
			dBins [i].Init ( iFromFD, &iSharedOffset, iBinSize );
		}

		for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
		{
			if ( ReadOrdinal ( dBins [iBlock], tOrdinalEntry ) != BIN_READ_OK )
			{
				m_sLastError = "sort_ordinals: warmup failed (io error?)";
				return false;
			}

			tOrdinalEntry.m_iTag = iBlock;
			qOrdinals.Push ( tOrdinalEntry );
		}

		SphDocID_t uCurID = 0;

		CSphString sLastOrdValue;
		int iBlock = 0;

		for ( ;; )
		{
			if ( !qOrdinals.GetLength () || qOrdinals.Root ().m_uDocID != uCurID )
			{
				if ( uCurID )
				{
					OrdinalId_t tId;
					tId.m_uDocID = uCurID;
					tId.m_uId = uOrdinalId;
					dOrdinalIdPool.Add ( tId );

					if ( qOrdinals.GetLength () > 0 )
					{
						if ( sLastOrdValue.cstr () [0] != qOrdinals.Root ().m_sValue.cstr () [0] )
							uOrdinalId++;
						else
							if ( strcmp ( sLastOrdValue.cstr (), qOrdinals.Root ().m_sValue.cstr () ) )
								uOrdinalId++;
					}

					if ( dOrdinalIdPool.GetLength () == iOrdinalsInPool )
					{
						dOrdinalIdPool.Sort ( CmpOrdinalsDocid_fn () );
						Writer.PutBytes ( &(dOrdinalIdPool [0]), sizeof ( OrdinalId_t ) * dOrdinalIdPool.GetLength () );
						if ( Writer.IsError () )
						{
							m_sLastError = "sort_ordinals: io error";
							return false;
						}

						dOrdinalIdPool.Resize ( 0 );
					}
				}

				if ( !qOrdinals.GetLength () )
					break;

				uCurID = qOrdinals.Root().m_uDocID;
				const_cast < CSphString & > ( qOrdinals.Root ().m_sValue ).Swap ( sLastOrdValue );
			}

			// get next entry
			iBlock = qOrdinals.Root().m_iTag;
			qOrdinals.Pop ();

			ESphBinRead eRes = ReadOrdinal ( dBins [iBlock], tOrdinalEntry );
			tOrdinalEntry.m_iTag = iBlock;
			if ( eRes == BIN_READ_OK )
				qOrdinals.Push ( tOrdinalEntry );

			if ( eRes == BIN_READ_ERROR )
			{
				m_sLastError = "sort_ordinals: read error";
				return false;
			}
		}

		// flush last ordinal ids
		if ( dOrdinalIdPool.GetLength () )
		{
			dOrdinalIdPool.Sort ( CmpOrdinalsDocid_fn () );
			Writer.PutBytes ( &(dOrdinalIdPool [0]), sizeof ( OrdinalId_t ) * dOrdinalIdPool.GetLength () );
			if ( Writer.IsError () )
			{
				m_sLastError = "sort_ordinals: io error";
				return false;
			}

			dOrdinalIdPool.Resize ( 0 );
		}
	}

	Writer.CloseFile ();
	if ( Writer.IsError () )
		return false;

	return true;
}


bool CSphIndex_VLN::SortOrdinalIds ( const char * szToFile, int iFromFD, int nAttrs, int nOrdinals, int iArenaSize, int iOrdinalsInPool, bool bWarnOfMem )
{
	int nLastBlockIds = nOrdinals % iOrdinalsInPool;
	int nBlocks = nOrdinals / iOrdinalsInPool + ( nLastBlockIds ? 1 : 0 );

	CSphWriter Writer;
	if ( !Writer.OpenFile ( szToFile, m_sLastError ) )
		return false;

	SphOffset_t iSharedOffset = -1;
	int iBinSize = CSphBin::CalcBinSize ( iArenaSize, nBlocks, "ordinals", bWarnOfMem );

	CSphQueue < OrdinalIdEntry_t, CmpOrdinalIdEntry_fn > qOrdinalIds ( Max ( 1, nBlocks ) );

	SphOffset_t uStart = 0;
	OrdinalIdEntry_t tOrdinalIdEntry;
	OrdinalId_t tOrdinalId;

	for ( int iAttr = 0; iAttr < nAttrs; ++iAttr )
	{
		CSphVector < CSphBin > dBins;
		dBins.Resize ( nBlocks );

		ARRAY_FOREACH ( i, dBins )
		{
			dBins [i].m_iFileLeft = ( i == nBlocks - 1 ? ( nLastBlockIds ? nLastBlockIds : iOrdinalsInPool ): iOrdinalsInPool ) * sizeof ( OrdinalId_t );
			dBins [i].m_iFilePos = uStart;
			dBins [i].Init ( iFromFD, &iSharedOffset, iBinSize );

			uStart += dBins [i].m_iFileLeft;
		}

		for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
		{
			if ( dBins [iBlock].ReadBytes ( &tOrdinalId, sizeof ( tOrdinalId ) ) != BIN_READ_OK )
			{
				m_sLastError = "sort_ordinals: warmup failed (io error?)";
				return false;
			}

			tOrdinalIdEntry.m_uDocID = tOrdinalId.m_uDocID;
			tOrdinalIdEntry.m_uId = tOrdinalId.m_uId;
			tOrdinalIdEntry.m_iTag = iBlock;
			qOrdinalIds.Push ( tOrdinalIdEntry );
		}

		OrdinalId_t tCachedId;
		tCachedId.m_uDocID = 0;

		for ( ;; )
		{
			if ( !qOrdinalIds.GetLength () || qOrdinalIds.Root ().m_uDocID != tCachedId.m_uDocID )
			{
				if ( tCachedId.m_uDocID )
				{
					Writer.PutBytes ( &tCachedId, sizeof ( OrdinalId_t ) );
					if ( Writer.IsError () )
					{
						m_sLastError = "sort_ordinals: io error";
						return false;
					}
				}

				if ( !qOrdinalIds.GetLength () )
					break;

				tCachedId.m_uDocID = qOrdinalIds.Root().m_uDocID;
				tCachedId.m_uId = qOrdinalIds.Root ().m_uId;
			}

			// get next entry
			int iBlock = qOrdinalIds.Root().m_iTag;
			qOrdinalIds.Pop ();

			ESphBinRead eRes = dBins [iBlock].ReadBytes ( &tOrdinalId, sizeof ( tOrdinalId ) );
			tOrdinalIdEntry.m_uDocID = tOrdinalId.m_uDocID;
			tOrdinalIdEntry.m_uId = tOrdinalId.m_uId;
			tOrdinalIdEntry.m_iTag = iBlock;
			if ( eRes == BIN_READ_OK )
				qOrdinalIds.Push ( tOrdinalIdEntry );

			if ( eRes == BIN_READ_ERROR )
			{
				m_sLastError = "sort_ordinals: read error";
				return false;
			}
		}
	}

	return true;
}

struct FieldMVARedirect_t
{
	int		m_iAttr;
	int		m_iMVAAttr;
	int		m_iRowItem;
};


int CSphIndex_VLN::Build ( CSphDict * pDict, const CSphVector<CSphSource*> & dSources, int iMemoryLimit, ESphDocinfo eDocinfo )
{
	PROFILER_INIT ();

	assert ( pDict );
	assert ( dSources.GetLength() );

	if ( !m_pWriteBuffer )
		m_pWriteBuffer = new BYTE [ WRITE_BUFFER_SIZE ];

	m_eDocinfo = eDocinfo;

	// vars shared between phases
	CSphVector<CSphBin*> dBins;
	SphOffset_t iSharedOffset = -1;

	// setup sources
	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		pSource->SetDict ( pDict );
		pSource->SetEmitInfixes ( m_iMinPrefixLen, m_iMinInfixLen );
		pSource->SetBoundaryStep ( m_iBoundaryStep );
	}

	// connect 1st source and fetch its schema
	if ( !dSources[0]->Connect ( m_sLastError )
		|| !dSources[0]->IterateHitsStart ( m_sLastError )
		|| !dSources[0]->UpdateSchema ( &m_tSchema, m_sLastError ) )
	{
		return 0;
	}

	// check docinfo
	if ( m_tSchema.GetAttrsCount()==0 )
		m_eDocinfo = SPH_DOCINFO_NONE;

	if ( m_tSchema.GetAttrsCount()>0 && m_eDocinfo==SPH_DOCINFO_NONE )
	{
		m_sLastError.SetSprintf ( "got attributes, but docinfo is 'none' (fix your config file)" );
		return 0;
	}

	bool bHaveFieldMVAs = false;
	CSphVector<int> dMvaIndexes, dMvaRowitem;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(i);
		if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
		{
			if ( tCol.m_eSrc == SPH_ATTRSRC_FIELD )
				bHaveFieldMVAs = true;

			dMvaIndexes.Add ( i );
			dMvaRowitem.Add ( tCol.m_iRowitem );
		}
	}

	bool bGotMVA = ( dMvaIndexes.GetLength()!=0 );
	if ( bGotMVA && m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		m_sLastError.SetSprintf ( "multi-valued attributes are only supported if but docinfo is 'extern' (fix your config file)" );
		return 0;
	}

	////////////////////////////////////////////////
	// collect and partially sort hits and docinfos
	////////////////////////////////////////////////

	// ordinals storage
	CSphVector<int> dOrdinalAttrs;
	if ( m_eDocinfo==SPH_DOCINFO_EXTERN )
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
			if ( m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_ORDINAL )
				dOrdinalAttrs.Add ( i );

	// adjust memory requirements
	// ensure there's enough memory to hold 1M hits and 64K docinfos
	// alloc 1/16 of memory (but not less than 64K entries) for docinfos
	iMemoryLimit = AdjustMemoryLimit ( iMemoryLimit );

	const int iDocinfoStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	int iDocinfoMax = Max ( 65536, iMemoryLimit/16/iDocinfoStride/sizeof(DWORD) );
	if ( m_eDocinfo==SPH_DOCINFO_NONE )
		iDocinfoMax = 1;

	int iOrdinalPoolSize = Max ( 32768, iMemoryLimit/8 );
	if ( dOrdinalAttrs.GetLength () == 0 )
		iOrdinalPoolSize = 0;

	int iFieldMVAPoolSize = Max ( 32768, iMemoryLimit/16 );
	if ( bHaveFieldMVAs  == 0 )
		iFieldMVAPoolSize = 0;

	int iHitsMax = ( iMemoryLimit - iDocinfoMax*iDocinfoStride*sizeof(DWORD) - iOrdinalPoolSize - iFieldMVAPoolSize ) / sizeof(CSphWordHit);

	// allocate raw hits block
	CSphAutoArray<CSphWordHit> dHits ( iHitsMax );
	CSphWordHit * pHits = dHits;
	CSphWordHit * pHitsMax = dHits + iHitsMax;

	// allocate docinfos buffer
	CSphAutoArray<DWORD> dDocinfos ( iDocinfoMax*iDocinfoStride );
	DWORD * pDocinfo = dDocinfos;
	const DWORD * pDocinfoMax = dDocinfos + iDocinfoMax*iDocinfoStride;
	if ( m_eDocinfo==SPH_DOCINFO_NONE )
	{
		pDocinfo = NULL;
		pDocinfoMax = NULL;
	}

	int nOrdinals = 0;
	SphOffset_t uMaxOrdinalAttrBlockSize = 0;
	int iCurrentBlockSize = 0;
	bool bHaveOrdinals = dOrdinalAttrs.GetLength() > 0;

	CSphVector < CSphVector < Ordinal_t > > dOrdinals;
	dOrdinals.Resize ( dOrdinalAttrs.GetLength() );
	ARRAY_FOREACH ( i, dOrdinals )
		dOrdinals [i].Reserve ( 65536 );

	CSphVector < CSphVector<SphOffset_t> > dOrdBlockSize;
	dOrdBlockSize.Resize ( dOrdinalAttrs.GetLength () );
	ARRAY_FOREACH ( i, dOrdBlockSize )
		dOrdBlockSize [i].Reserve ( 8192 );

	CSphVector < MvaEntry_t > dFieldMVAs;
	dFieldMVAs.Reserve ( 16384 );

	CSphVector < SphOffset_t > dFieldMVABlocks;
	dFieldMVABlocks.Reserve ( 4096 );

	CSphVector < FieldMVARedirect_t > dFieldMvaIndexes;

	if ( bHaveFieldMVAs )
		dFieldMvaIndexes.Reserve ( 8 );

	int iMaxPoolFieldMVAs = iFieldMVAPoolSize / sizeof ( MvaEntry_t );
	int nFieldMVAs = 0;

	// create temp files
	CSphAutofile fdLock ( GetIndexFileName("tmp0"), SPH_O_NEW, m_sLastError );
	CSphAutofile fdTmpHits ( GetIndexFileName("tmp1"), SPH_O_NEW, m_sLastError );
	CSphAutofile fdTmpDocinfos ( GetIndexFileName("tmp2"), SPH_O_NEW, m_sLastError );
	CSphAutofile fdTmpFieldMVAs ( GetIndexFileName("tmp7"), SPH_O_NEW, m_sLastError );
	CSphWriter tOrdWriter;

	CSphString sRawOrdinalsFile = GetIndexFileName("tmp4");
	if ( bHaveOrdinals && !tOrdWriter.OpenFile ( sRawOrdinalsFile.cstr (), m_sLastError ) )
		return 0;

	if ( fdLock.GetFD()<0 || fdTmpHits.GetFD()<0 || fdTmpDocinfos.GetFD()<0 || fdTmpFieldMVAs.GetFD ()<0 )
		return 0;

	if ( !sphLockEx ( fdLock.GetFD(), false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock '%s': another indexer running?", fdLock.GetFilename() );
		return 0;
	}

	// setup accumulating docinfo IDs range
	m_tMin.Reset ( m_tSchema.GetRowSize() );

	for ( int i=0; i<m_tMin.m_iRowitems; i++ )
		m_tMin.m_pRowitems[i] = ROWITEM_MAX;
	m_tMin.m_iDocID = DOCID_MAX;

	// build raw log
	PROFILE_BEGIN ( collect_hits );

	m_tStats.Reset ();
	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;

	CSphVector<int> dHitBlocks;
	dHitBlocks.Reserve ( 1024 );

	int iDocinfoBlocks = 0;

	ARRAY_FOREACH ( iSource, dSources )
	{
		// connect and check schema, if it's not the first one
		CSphSource * pSource = dSources[iSource];

		if ( iSource )
			if ( !pSource->Connect ( m_sLastError )
				|| !pSource->IterateHitsStart ( m_sLastError )
				|| !pSource->UpdateSchema ( &m_tSchema, m_sLastError ) )
			{
				return 0;
			}

		dFieldMvaIndexes.Resize ( 0 );

		ARRAY_FOREACH ( i, dMvaIndexes )
		{
			int iAttr = dMvaIndexes [i];

			if ( m_tSchema.GetAttr ( iAttr ).m_eSrc == SPH_ATTRSRC_FIELD )
			{
				dFieldMvaIndexes.Resize ( dFieldMvaIndexes.GetLength() + 1 );
				FieldMVARedirect_t & tRedirect = dFieldMvaIndexes.Last ();
				tRedirect.m_iAttr	 = iAttr; 
				tRedirect.m_iMVAAttr = i;
				tRedirect.m_iRowItem = m_tSchema.GetAttr ( iAttr ).m_iRowitem;
			}
		}

		// fetch documents
		for ( ;; )
		{
			// get next doc, and handle errors
			if ( !pSource->IterateHitsNext ( m_sLastError ) )
				return 0;

			// check for eof
			if ( !pSource->m_tDocInfo.m_iDocID )
				break;

			// show progress bar
			if ( m_pProgress
				&& ( ( pSource->GetStats().m_iTotalDocuments % 1000 )==0 ) )
			{
				m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				m_tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				m_pProgress ( &m_tProgress, false );
			}

			// make a shortcut for hit count
			int iDocHits = pSource->m_dHits.GetLength ();
			if ( iDocHits<=0 )
				continue;

			// store field MVAs
			if ( bHaveFieldMVAs )
			{
				ARRAY_FOREACH ( i, dFieldMvaIndexes )
				{
					int iAttr	= dFieldMvaIndexes [i].m_iAttr;
					int iMVA	= dFieldMvaIndexes [i].m_iMVAAttr;
					int iRowitem= dFieldMvaIndexes [i].m_iRowItem;

					// store per-document MVAs
					if ( pSource->IterateFieldMVAStart ( iAttr, m_sLastError ) )
						while ( pSource->IterateFieldMVANext () )
						{
							dFieldMVAs.Resize ( dFieldMVAs.GetLength() + 1 );

							dFieldMVAs.Last ().m_uDocID = pSource->m_tDocInfo.m_iDocID;
							dFieldMVAs.Last ().m_iAttr = iMVA;
							dFieldMVAs.Last ().m_uValue = pSource->m_tDocInfo.m_pRowitems[iRowitem];
							
							int iLength = dFieldMVAs.GetLength ();
							if ( iLength == iMaxPoolFieldMVAs )
							{
								dFieldMVAs.Sort ( CmpMvaEntries_fn () );
								if ( !sphWriteThrottled ( fdTmpFieldMVAs.GetFD (), &(dFieldMVAs [0]), iLength*sizeof(MvaEntry_t), "temp_field_mva", m_sLastError ) )
									return 0;

								dFieldMVAs.Resize ( 0 );

								nFieldMVAs += iMaxPoolFieldMVAs;
							}
						}
				}
			}

			// store ordinals
			iCurrentBlockSize += ( sizeof ( SphOffset_t ) + sizeof ( DWORD ) ) * dOrdinalAttrs.GetLength ();

			ARRAY_FOREACH ( i, dOrdinalAttrs )
			{
				CSphVector<Ordinal_t> & dCol = dOrdinals[i];
				dCol.Resize ( 1+dCol.GetLength() );

				dCol.Last().m_uDocID = pSource->m_tDocInfo.m_iDocID;
				Swap ( dCol.Last().m_sValue, pSource->m_dStrAttrs[dOrdinalAttrs[i]] );

				iCurrentBlockSize += strlen ( dCol.Last ().m_sValue.cstr () );
			}

			if ( bHaveOrdinals )
			{
				if ( iCurrentBlockSize >= iOrdinalPoolSize )
				{
					iCurrentBlockSize = 0;

					nOrdinals += dOrdinals[0].GetLength ();

					ARRAY_FOREACH ( i, dOrdinalAttrs )
					{
						CSphVector<Ordinal_t> & dCol = dOrdinals[i];
						dCol.Sort ( CmpOrdinalsValue_fn() );
						SphOffset_t uSize = DumpOrdinals ( tOrdWriter, dCol );
						if ( !uSize )
						{
							m_sLastError = "dump ordinals: io error";
							return 0;
						}

						if ( uSize > uMaxOrdinalAttrBlockSize )
							uMaxOrdinalAttrBlockSize = uSize;

						dOrdBlockSize [i].Add ( uSize );
						dCol.Resize ( 0 );
					}
				}
			}

			// update min docinfo
			assert ( pSource->m_tDocInfo.m_iDocID );
			m_tMin.m_iDocID = Min ( m_tMin.m_iDocID, pSource->m_tDocInfo.m_iDocID );
			if ( m_eDocinfo==SPH_DOCINFO_INLINE )
				for ( int i=0; i<m_tMin.m_iRowitems; i++ )
					m_tMin.m_pRowitems[i] = Min ( m_tMin.m_pRowitems[i], pSource->m_tDocInfo.m_pRowitems[i] );

			// store docinfo
			if ( m_eDocinfo!=SPH_DOCINFO_NONE )
				if ( pDocinfo==dDocinfos || DOCINFO2ID(pDocinfo)!=pSource->m_tDocInfo.m_iDocID )
			{
				// store next entry
				DOCINFO2ID(pDocinfo) = pSource->m_tDocInfo.m_iDocID;
				memcpy ( DOCINFO2ATTRS(pDocinfo), pSource->m_tDocInfo.m_pRowitems, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );
				pDocinfo += iDocinfoStride;

				// if not inlining, flush buffer if it's full
				// (if inlining, it will flushed later, along with the hits)
				if ( m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>=pDocinfoMax )
				{
					assert ( pDocinfo==pDocinfoMax );
					int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);

					sphSortDocinfos ( dDocinfos, iDocinfoMax, iDocinfoStride );
					if ( !sphWriteThrottled ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
						return 0;

					pDocinfo = dDocinfos;
					iDocinfoBlocks++;
				}
			}

			// store hits
			CSphWordHit * pDocHits = &pSource->m_dHits[0];

#if PARANOID
			for ( int i=0; i<iDocHits; i++ )
			{
				assert ( pDocHits[i].m_iDocID==pSource->m_tDocInfo.m_iDocID );
				assert ( pDocHits[i].m_iWordID );
				assert ( pDocHits[i].m_iWordPos );
			}
#endif

			while ( iDocHits )
			{
				// copy as much hits as we can
				int iToCopy = Min ( iDocHits, pHitsMax - pHits ); 

				memcpy ( pHits, pDocHits, iToCopy*sizeof(CSphWordHit) );
				pHits += iToCopy;
				pDocHits += iToCopy;
				iDocHits -= iToCopy;

				assert ( iDocHits>=0 );
				assert ( pHits<=pHitsMax );

				// check if we need to flush
				if ( pHits<pHitsMax && !( m_eDocinfo==SPH_DOCINFO_INLINE && pDocinfo>=pDocinfoMax ) )
					continue;

				// sort hits
				int iHits = pHits - dHits;
				{
					PROFILE ( sort_hits );
					sphSort ( &dHits[0], iHits, CmpHit_fn() );
				}
				pHits = dHits;

				if ( m_eDocinfo==SPH_DOCINFO_INLINE )
				{
					// we're inlining, so let's flush both hits and docs
					int iDocs = ( pDocinfo - dDocinfos ) / iDocinfoStride;
					pDocinfo = dDocinfos;

					sphSortDocinfos ( pDocinfo, iDocs, iDocinfoStride );

					dHitBlocks.Add ( cidxWriteRawVLB ( fdTmpHits.GetFD(), dHits, iHits,
						dDocinfos, iDocs, iDocinfoStride ) );

					// we are inlining, so if there are more hits in this document,
					// we'll need to know it's info next flush
					if ( iDocHits )
					{
						DOCINFO2ID(pDocinfo) = pSource->m_tDocInfo.m_iDocID;
						memcpy ( DOCINFO2ATTRS(pDocinfo), pSource->m_tDocInfo.m_pRowitems, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );
						pDocinfo += iDocinfoStride;
					}
				} else
				{
					// we're not inlining, so only flush hits, docs are flushed independently
					dHitBlocks.Add ( cidxWriteRawVLB ( fdTmpHits.GetFD(), dHits, iHits,
						NULL, 0, 0 ) );
				}

				if ( dHitBlocks.Last()<0 )
					return 0;

				// progress bar
				m_tProgress.m_iHitsTotal += iHits;
				if ( m_pProgress )
				{
					m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
					m_tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
					m_pProgress ( &m_tProgress, false );
				}
			}
		}

		// this source is over, disconnect and update stats
		pSource->Disconnect ();

		m_tStats.m_iTotalDocuments += pSource->GetStats().m_iTotalDocuments;
		m_tStats.m_iTotalBytes += pSource->GetStats().m_iTotalBytes;
	}

	// flush last docinfo block
	int iDocinfoLastBlockSize = 0;
	if ( m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>dDocinfos )
	{
		iDocinfoLastBlockSize = ( pDocinfo - dDocinfos ) / iDocinfoStride;
		assert ( pDocinfo == dDocinfos + iDocinfoLastBlockSize*iDocinfoStride );

		int iLen = iDocinfoLastBlockSize*iDocinfoStride*sizeof(DWORD);
		sphSortDocinfos ( dDocinfos, iDocinfoLastBlockSize, iDocinfoStride );
		if ( !sphWriteThrottled ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
			return 0;

		iDocinfoBlocks++;
	}

	// flush last hit block
	if ( pHits>dHits )
	{
		int iHits = pHits - dHits;
		{
			PROFILE ( sort_hits );
			sphSort ( &dHits[0], iHits, CmpHit_fn() );
		}
		m_tProgress.m_iHitsTotal += iHits;

		if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		{
			int iDocs = ( pDocinfo - dDocinfos ) / iDocinfoStride;
			sphSortDocinfos ( dDocinfos, iDocs, iDocinfoStride );
			dHitBlocks.Add ( cidxWriteRawVLB ( fdTmpHits.GetFD(), dHits, iHits,
				dDocinfos, iDocs, iDocinfoStride ) );
		} else
		{
			dHitBlocks.Add ( cidxWriteRawVLB ( fdTmpHits.GetFD(), dHits, iHits, NULL, 0, 0 ) );
		}

		if ( dHitBlocks.Last()<0 )
			return 0;
	}

	// flush last field MVA block
	if ( bHaveFieldMVAs && dFieldMVAs.GetLength () )
	{
		int iLength = dFieldMVAs.GetLength ();
		nFieldMVAs += iLength;

		dFieldMVAs.Sort ( CmpMvaEntries_fn () );
		if ( !sphWriteThrottled ( fdTmpFieldMVAs.GetFD (), &(dFieldMVAs [0]), iLength*sizeof(MvaEntry_t), "temp_field_mva", m_sLastError ) )
			return 0;

		dFieldMVAs.Reset ();
	}

	// flush last ordinals block
	if ( bHaveOrdinals && dOrdinals [0].GetLength () )
	{
		nOrdinals += dOrdinals[0].GetLength ();

		ARRAY_FOREACH ( i, dOrdinalAttrs )
		{
			CSphVector<Ordinal_t> & dCol = dOrdinals[i];
			dCol.Sort ( CmpOrdinalsValue_fn() );

			SphOffset_t uSize = DumpOrdinals ( tOrdWriter, dCol );
			if ( !uSize )
			{
				m_sLastError = "dump ordinals: io error";
				return 0;
			}

			if ( uSize > uMaxOrdinalAttrBlockSize )
				uMaxOrdinalAttrBlockSize = uSize;

			dOrdBlockSize [i].Add ( uSize );
			dCol.Reset ();
		}
	}

	if ( m_pProgress )
	{
		m_tProgress.m_iDocuments = m_tStats.m_iTotalDocuments;
		m_tProgress.m_iBytes = m_tStats.m_iTotalBytes;
		m_pProgress ( &m_tProgress, true );
	}

	PROFILE_END ( collect_hits );

	///////////////////////////////////////
	// collect and sort multi-valued attrs
	///////////////////////////////////////

	if ( !BuildMVA ( dSources, dHits, iHitsMax*sizeof(CSphWordHit), fdTmpFieldMVAs.GetFD (), nFieldMVAs, iMaxPoolFieldMVAs ) )
		return 0;

	// reset hits pool
	dHits.Reset ();

	CSphString sFieldMVAFile = fdTmpFieldMVAs.GetFilename ();
	fdTmpFieldMVAs.Close ();
	::unlink ( sFieldMVAFile.cstr () );

	/////////////////
	// sort docinfos
	/////////////////

	tOrdWriter.CloseFile ();
	if ( tOrdWriter.IsError () )
		return 0;

	CSphString sSortedOrdinalIdFile = GetIndexFileName("tmp6");

	// sort ordinals
	if ( bHaveOrdinals && !dOrdBlockSize [0].GetLength () )
	{
		bHaveOrdinals = false;
		::unlink ( sRawOrdinalsFile.cstr () );
	}

	if ( bHaveOrdinals )
	{
		CSphString sUnsortedIdFile = GetIndexFileName("tmp5");

		CSphAutofile fdRawOrdinals ( sRawOrdinalsFile.cstr (), SPH_O_READ, m_sLastError );
		if ( fdRawOrdinals.GetFD () < 0 )
			return 0;

		const float ARENA_PERCENT = 0.5f;
		int nBlocks = dOrdBlockSize [0].GetLength ();

		SphOffset_t uMemNeededForReaders = SphOffset_t ( nBlocks ) * uMaxOrdinalAttrBlockSize;
		SphOffset_t uMemNeededForSorting = sizeof ( OrdinalId_t ) * nOrdinals;

		int iArenaSize =	  (int) Min ( SphOffset_t ( iMemoryLimit * ARENA_PERCENT ), uMemNeededForReaders );
		iArenaSize =				Max ( CSphBin::MIN_SIZE * nBlocks, iArenaSize );

		int iOrdinalsInPool = (int) Min ( SphOffset_t ( iMemoryLimit * ( 1.0f - ARENA_PERCENT ) ), uMemNeededForSorting ) / sizeof ( OrdinalId_t );

		SortOrdinals ( sUnsortedIdFile.cstr (), fdRawOrdinals.GetFD (), iArenaSize, iOrdinalsInPool, dOrdBlockSize, iArenaSize < uMemNeededForReaders );

		CSphAutofile fdUnsortedId ( sUnsortedIdFile.cstr (), SPH_O_READ, m_sLastError );
		if ( fdUnsortedId.GetFD () < 0 )
			return 0;

		iArenaSize = Min ( iMemoryLimit, (int)uMemNeededForSorting );
		iArenaSize = Max ( CSphBin::MIN_SIZE * ( nOrdinals / iOrdinalsInPool + 1 ), iArenaSize );

		SortOrdinalIds ( sSortedOrdinalIdFile.cstr (), fdUnsortedId.GetFD (), dOrdinalAttrs.GetLength (), nOrdinals, iArenaSize, iOrdinalsInPool, iArenaSize < uMemNeededForSorting );

		fdRawOrdinals.Close ();
		fdUnsortedId.Close ();

		::unlink ( sRawOrdinalsFile.cstr () );
		::unlink ( sUnsortedIdFile.cstr () );
	}

	// initialize MVA reader
	CSphAutofile fdMva ( GetIndexFileName("spm"), SPH_O_BINARY, m_sLastError );
	if ( fdMva.GetFD()<0 )
		return false;

	CSphReader_VLN rdMva;
	rdMva.SetFile ( fdMva );

	SphDocID_t uMvaID = rdMva.GetDocid();

	// initialize writer
	CSphAutofile fdDocinfo ( GetIndexFileName("spa"), SPH_O_NEW, m_sLastError );
	if ( fdDocinfo.GetFD()<0 )
		return 0;

	if ( m_eDocinfo==SPH_DOCINFO_EXTERN && dHitBlocks.GetLength() )
	{
		// initialize readers
		assert ( dBins.GetLength()==0 );
		dBins.Reserve ( iDocinfoBlocks );

		int iBinSize = CSphBin::CalcBinSize ( iMemoryLimit, iDocinfoBlocks, "sort_docinfos" );
		iSharedOffset = -1;

		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			dBins.Add ( new CSphBin() );
			dBins[i]->m_iFileLeft = ( ( i==iDocinfoBlocks-1 ) ? iDocinfoLastBlockSize : iDocinfoMax )*iDocinfoStride*sizeof(DWORD);
			dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
			dBins[i]->Init ( fdTmpDocinfos.GetFD(), &iSharedOffset, iBinSize );
		}

		// docinfo queue
		CSphAutoArray<DWORD> dDocinfoQueue ( iDocinfoBlocks*iDocinfoStride );
		CSphQueue < int,CmpQueuedDocinfo_fn > qDocinfo ( iDocinfoBlocks );

		CmpQueuedDocinfo_fn::m_pStorage = dDocinfoQueue;
		CmpQueuedDocinfo_fn::m_iStride = iDocinfoStride;

		pDocinfo = dDocinfoQueue;
		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			if ( dBins[i]->ReadBytes ( pDocinfo, iDocinfoStride*sizeof(DWORD) ) != BIN_READ_OK )
			{
				m_sLastError.SetSprintf ( "sort_docinfos: warmup failed (io error?)" );
				return 0;
			}
			pDocinfo += iDocinfoStride;
			qDocinfo.Push ( i );
		}

		CSphVector < CSphBin > dOrdReaders;
		SphOffset_t iSharedOffset = -1;

		CSphAutofile fdTmpSortedIds ( sSortedOrdinalIdFile.cstr (), SPH_O_READ, m_sLastError );

		if ( bHaveOrdinals )
		{
			if ( fdTmpSortedIds.GetFD () < 0 )
				return 0;

			dOrdReaders.Resize ( dOrdinalAttrs.GetLength () );
			ARRAY_FOREACH ( i, dOrdReaders )
			{
				dOrdReaders [i].m_iFileLeft = nOrdinals * sizeof ( OrdinalId_t );
				dOrdReaders [i].m_iFilePos = ( i==0 ) ? 0 : dOrdReaders[i-1].m_iFilePos + dOrdReaders[i-1].m_iFileLeft;
				dOrdReaders [i].Init ( fdTmpSortedIds.GetFD(), &iSharedOffset, ORDINAL_READ_SIZE );
			}
		}

		// while the queue has data for us
		int iOrd = 0;
		pDocinfo = dDocinfos;
		while ( qDocinfo.GetLength() )
		{
			// obtain bin index and next entry
			int iBin = qDocinfo.Root();
			DWORD * pEntry = dDocinfoQueue + iBin*iDocinfoStride;

			// update ordinals
			ARRAY_FOREACH ( i, dOrdinalAttrs )
			{
				OrdinalId_t Id;
				if ( dOrdReaders [i].ReadBytes ( &Id, sizeof ( Id ) ) != BIN_READ_OK )
				{
					m_sLastError = "update ordinals: io error";
					return 0;
				}

				assert ( Id.m_uDocID == DOCINFO2ID(pEntry) );
				DOCINFO2ATTRS(pEntry) [ dOrdinalAttrs[i] ] = Id.m_uId;
			}
			iOrd++;

			// update MVA
			if ( bGotMVA )
			{
				// go to next id
				while ( uMvaID<DOCINFO2ID(pEntry) )
				{
					ARRAY_FOREACH ( i, dMvaIndexes )
						rdMva.SkipBytes ( rdMva.GetDword()*sizeof(DWORD) );

					uMvaID = rdMva.GetDocid();
					if ( !uMvaID )
						uMvaID = DOCID_MAX;
				}

				assert ( uMvaID>=DOCINFO2ID(pEntry) );
				if ( uMvaID==DOCINFO2ID(pEntry) )
				{
					ARRAY_FOREACH ( i, dMvaIndexes )
					{
						DOCINFO2ATTRS(pEntry) [ dMvaRowitem[i] ] = (DWORD)( rdMva.GetPos()/sizeof(DWORD) ); // intentional clamp; we'll check for 32bit overflow later
						rdMva.SkipBytes ( rdMva.GetDword()*sizeof(DWORD) );
					}

					uMvaID = rdMva.GetDocid();
					if ( !uMvaID )
						uMvaID = DOCID_MAX;
				}
			}

			// emit it
			memcpy ( pDocinfo, pEntry, iDocinfoStride*sizeof(DWORD) );
			pDocinfo += iDocinfoStride;

			if ( pDocinfo>=pDocinfoMax )
			{
				int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);
				if ( !sphWriteThrottled ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
					return 0;

				pDocinfo = dDocinfos;
			}

			// pop its index, update it, push its index again
			qDocinfo.Pop ();
			ESphBinRead eRes = dBins[iBin]->ReadBytes ( pEntry, iDocinfoStride*sizeof(DWORD) );
			if ( eRes == BIN_READ_ERROR )
			{
				m_sLastError.SetSprintf ( "sort_docinfo: failed to read entry" );
				return 0;
			}
			if ( eRes == BIN_READ_OK )
				qDocinfo.Push ( iBin );
		}

		if ( pDocinfo>dDocinfos )
		{
			assert ( 0 == ( pDocinfo-dDocinfos ) % iDocinfoStride );
			int iLen = ( pDocinfo - dDocinfos )*sizeof(DWORD);
			if ( !sphWriteThrottled ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
				return 0;
		}

		// clean up readers
		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );

		if ( bHaveOrdinals )
		{
			fdTmpSortedIds.Close ();
			::unlink ( sSortedOrdinalIdFile.cstr () );
		}

		dBins.Reset ();
	}

	dDocinfos.Reset ();
	pDocinfo = NULL;

	fdDocinfo.Close (); // it might be zero-length, but it must exist

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	PROFILE_BEGIN ( invert_hits );

	// initialize readers
	assert ( dBins.GetLength()==0 );
	dBins.Reserve ( dHitBlocks.GetLength() );

	iSharedOffset = -1;
	int iBinSize = CSphBin::CalcBinSize ( iMemoryLimit, dHitBlocks.GetLength(), "sort_hits" );

	ARRAY_FOREACH ( i, dHitBlocks )
	{
		dBins.Add ( new CSphBin() );
		dBins[i]->m_iFileLeft = dHitBlocks[i];
		dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( fdTmpHits.GetFD(), &iSharedOffset, iBinSize );
	}

	// if there were no hits, create zero-length index files
	int iRawBlocks = dBins.GetLength();
	if ( iRawBlocks==0 )
		m_eDocinfo = SPH_DOCINFO_INLINE;

	//////////////////////////////
	// create new index files set
	//////////////////////////////

	m_wrWordlist.CloseFile ();
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ();
	if (
		!m_wrWordlist.OpenFile ( GetIndexFileName("spi"), m_sLastError ) ||
		!m_wrDoclist.OpenFile ( GetIndexFileName("spd"), m_sLastError ) ||
		!m_wrHitlist.OpenFile ( GetIndexFileName("spp"), m_sLastError ) )
	{
		return 0;
	}

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	m_wrWordlist.PutBytes ( &bDummy, 1 );
	m_wrDoclist.PutBytes ( &bDummy, 1 );
	m_wrHitlist.PutBytes ( &bDummy, 1 );

	// adjust min IDs, and fill header
	assert ( m_tMin.m_iDocID>0 );
	m_tMin.m_iDocID--;
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		for ( int i=0; i<m_tMin.m_iRowitems; i++ )
			m_tMin.m_pRowitems[i]--;

	//////////////
	// final sort
	//////////////

	if ( iRawBlocks )
	{
		CSphHitQueue tQueue ( iRawBlocks );
		CSphWordHit tHit;

		m_tLastHit.m_iDocID = 0;
		m_tLastHit.m_iWordID = 0;
		m_tLastHit.m_iWordPos = 0;

		// initial fill
		int iRowitems = ( m_eDocinfo==SPH_DOCINFO_INLINE ) ? m_tSchema.GetRowSize() : 0;
		CSphAutoArray<CSphRowitem> dInlineAttrs ( iRawBlocks*iRowitems );

		int * bActive = new int [ iRawBlocks ];
		for ( int i=0; i<iRawBlocks; i++ )
		{
			if ( !dBins[i]->ReadHit ( &tHit, iRowitems, dInlineAttrs+i*iRowitems ) )
			{
				m_sLastError.SetSprintf ( "sort_hits: warmup failed (io error?)" );
				return 0;
			}
			bActive[i] = ( tHit.m_iWordID!=0 );
			if ( bActive[i] )
				tQueue.Push ( tHit, i );
		}

		// init progress meter
		m_tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT;
		m_tProgress.m_iHits = 0;

		// while the queue has data for us
		// FIXME! analyze binsRead return code
		int iHitsSorted = 0;
		while ( tQueue.m_iUsed )
		{
			int iBin = tQueue.m_pData->m_iBin;

			// pack and emit queue root
			tQueue.m_pData->m_iDocID -= m_tMin.m_iDocID;
			cidxHit ( tQueue.m_pData, iRowitems ? dInlineAttrs+iBin*iRowitems : NULL );
			if ( m_wrWordlist.IsError() || m_wrDoclist.IsError() || m_wrHitlist.IsError() )
				return 0;

			// pop queue root and push next hit from popped bin
			tQueue.Pop ();
			if ( bActive[iBin] )
			{
				dBins[iBin]->ReadHit ( &tHit, iRowitems, dInlineAttrs+iBin*iRowitems );
				bActive[iBin] = ( tHit.m_iWordID!=0 );
				if ( bActive[iBin] )
					tQueue.Push ( tHit, iBin );
			}

			// progress
			if ( m_pProgress && ++iHitsSorted==1000000 )
			{
				m_tProgress.m_iHits += iHitsSorted;
				m_pProgress ( &m_tProgress, false );
				iHitsSorted = 0;
			}
		}

		if ( m_pProgress )
		{
			m_tProgress.m_iHits += iHitsSorted;
			m_pProgress ( &m_tProgress, true );
		}

		// cleanup
		SafeDeleteArray ( bActive );

		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );
		dBins.Reset ();

		CSphWordHit tFlush;
		tFlush.m_iDocID = 0;
		tFlush.m_iWordID = 0;
		tFlush.m_iWordPos = 0;
		cidxHit ( &tFlush, NULL );
	}

	PROFILE_END ( invert_hits );

	// close and unlink temp files
	// FIXME! should be unlinked on early-exit too
	fdTmpHits.Close ();
	fdTmpDocinfos.Close ();
	fdLock.Close ();

	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s.tmp0", m_sFilename.cstr() ); unlink ( sBuf );
	snprintf ( sBuf, sizeof(sBuf), "%s.tmp1", m_sFilename.cstr() ); unlink ( sBuf );
	snprintf ( sBuf, sizeof(sBuf), "%s.tmp2", m_sFilename.cstr() ); unlink ( sBuf );

	// we're done
	if ( !cidxDone() )
		return 0;

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	PROFILER_DONE ();
	PROFILE_SHOW ();
	return 1;
}


bool CSphIndex_VLN::Merge( CSphIndex * pSource, CSphPurgeData & tPurgeData )
{
	assert( pSource );

	CSphIndex_VLN * pSrcIndex = dynamic_cast< CSphIndex_VLN * >( pSource );
	assert( pSrcIndex );

	const CSphSchema * pDstSchema = Prealloc ( false, NULL );
	if ( !pDstSchema || !Preread() )
		return false;

	const CSphSchema * pSrcSchema = pSrcIndex->Prealloc ( false, NULL );
	if ( !pSrcSchema || !pSrcIndex->Preread() )
	{
		m_sLastError.SetSprintf ( "source index preload failed: %s", pSrcIndex->GetLastError().cstr() );
		return false;
	}
	if ( pDstSchema->CompareTo ( *pSrcSchema, m_sLastError ) != SPH_SCHEMAS_EQUAL )
	{
		m_sLastError.SetSprintf ( "schemas must be the same" );
		return false;
	}
	 
	// FIXME!
	if ( m_eDocinfo!=pSrcIndex->m_eDocinfo )
	{
		m_sLastError.SetSprintf ( "docinfo storage must be the same" );
		return false;
	}

	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	/////////////////
	/// merging .spa, .spm
	/////////////////

	CSphReader_VLN	tDstSPM, tSrcSPM;
	CSphWriter		tSPMWriter;

	/// preparing files
	CSphAutofile tDstSPMFile ( GetIndexFileName("spm"), SPH_O_READ, m_sLastError );
	if ( tDstSPMFile.GetFD()<0 )
		return false;

	CSphAutofile tSrcSPMFile ( pSrcIndex->GetIndexFileName("spm"), SPH_O_READ, m_sLastError );
	if ( tSrcSPMFile.GetFD()<0 )
		return false;

	tDstSPM.SetFile ( tDstSPMFile.GetFD(), tDstSPMFile.GetFilename() );		
	tSrcSPM.SetFile ( tSrcSPMFile.GetFD(), tSrcSPMFile.GetFilename() );	

	if ( !tSPMWriter.OpenFile ( GetIndexFileName("spm.tmp"), m_sLastError ) )
		return false;

	/// merging
	int iAttrNum = pDstSchema->GetAttrsCount();
	int iMVANum = 0;
	CSphVector<DWORD> dRowItemOffset;
	for( int i = 0; i < iAttrNum; ++i )
	{
		const CSphColumnInfo & tInfo = pDstSchema->GetAttr( i );
		if ( tInfo.m_eAttrType & SPH_ATTR_MULTI )
		{
			assert ( tInfo.m_iRowitem >= 0 );
			assert ( tInfo.m_iRowitem < iStride );

			dRowItemOffset.Add( tInfo.m_iRowitem );
			iMVANum++;
		}
	}

	CSphDocMVA	tDstMVA( iMVANum ), tSrcMVA( iMVANum );

	int iDeltaTotalDocs = 0;
	if ( m_eDocinfo == SPH_DOCINFO_EXTERN && pSrcIndex->m_eDocinfo == SPH_DOCINFO_EXTERN )
	{
		CSphAutofile fdSpa ( GetIndexFileName("spa.tmp"), SPH_O_NEW, m_sLastError );
		if ( fdSpa.GetFD()<0 )
			return false;

		DWORD * pSrcRow = pSrcIndex->m_pDocinfo.GetWritePtr();
		assert( pSrcRow );

		DWORD * pDstRow = m_pDocinfo.GetWritePtr();
		assert( pDstRow );

		DWORD iSrcCount = 0;
		DWORD iDstCount = 0;

		tDstMVA.Read( tDstSPM );
		tSrcMVA.Read( tSrcSPM );

		while( iSrcCount < pSrcIndex->m_uDocinfo || iDstCount < m_uDocinfo )
		{
			SphDocID_t iDstDocID, iSrcDocID;

			if ( iDstCount < m_uDocinfo )
				iDstDocID = DOCINFO2ID(pDstRow);
			else
				iDstDocID = 0;

			if ( iSrcCount < pSrcIndex->m_uDocinfo )
				iSrcDocID = DOCINFO2ID(pSrcRow);
			else
				iSrcDocID = 0;
			
			if ( ( iDstDocID && iDstDocID < iSrcDocID ) || ( iDstDocID && !iSrcDocID ) )
			{
				while ( tDstMVA.m_iDocID && tDstMVA.m_iDocID<iDstDocID )
					tDstMVA.Read ( tDstSPM );

				if ( tDstMVA.m_iDocID==iDstDocID )
				{
					tDstMVA.Write( tSPMWriter );
					DWORD * pAttrs = DOCINFO2ATTRS(pDstRow);
					ARRAY_FOREACH ( i, tDstMVA.m_dMVA )
						pAttrs [ dRowItemOffset[i] ] = tDstMVA.m_dOffsets[i];
				}

				sphWriteThrottled ( fdSpa.GetFD(), pDstRow, sizeof(DWORD)*iStride, "doc_attr", m_sLastError );
				pDstRow += iStride;
				iDstCount++;

			} else if ( iSrcDocID )
			{
				// iSrcDocID<=iDstDocID; in both cases, its src attr values that must win
				while ( tSrcMVA.m_iDocID && tSrcMVA.m_iDocID<iSrcDocID )
					tSrcMVA.Read ( tSrcSPM );
				
				if ( tSrcMVA.m_iDocID==iSrcDocID )
				{
					tSrcMVA.Write ( tSPMWriter );
					DWORD * pAttrs = DOCINFO2ATTRS(pSrcRow);
					ARRAY_FOREACH ( i, tSrcMVA.m_dMVA )
						pAttrs [ dRowItemOffset[i] ] = tSrcMVA.m_dOffsets[i];
				}

				sphWriteThrottled( fdSpa.GetFD(), pSrcRow, sizeof( DWORD ) * iStride, "doc_attr", m_sLastError );
				pSrcRow += iStride;
				iSrcCount++;

				if ( iDstDocID==iSrcDocID )
				{
					iDeltaTotalDocs++;
					pDstRow += iStride;
					iDstCount++;
				}
			}
		}
	}
	else
	{
		CSphAutofile fdSpa ( GetIndexFileName("spa.tmp"), SPH_O_NEW, m_sLastError );
		fdSpa.Close();
	}
	
	/////////////////
	/// merging .spd
	/////////////////

	CSphAutofile tDstData ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	if ( tDstData.GetFD()<0 )
		return false;

	CSphAutofile tDstHitlist ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError );
	if ( tDstHitlist.GetFD()<0 )
		return false;

	CSphAutofile tSrcData ( pSrcIndex->GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	if ( tSrcData.GetFD()<0 )
		return false;

	CSphAutofile tSrcHitlist ( pSrcIndex->GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError );
	if ( tSrcHitlist.GetFD()<0 )
		return false;

	CSphReader_VLN rdDstData;
	CSphReader_VLN rdDstHitlist;
	CSphReader_VLN rdSrcData;
	CSphReader_VLN rdSrcHitlist;
	CSphWriter wrDstData;
	CSphWriter wrDstIndex;
	CSphWriter wrDstHitlist;
	
	bool bSrcEmpty = false;
	bool bDstEmpty = false;

	rdDstData.SetFile ( tDstData.GetFD(), tDstData.GetFilename() );		
	rdDstHitlist.SetFile ( tDstHitlist.GetFD(), tDstHitlist.GetFilename() );	

	rdSrcData.SetFile ( tSrcData.GetFD(), tSrcData.GetFilename() );
	rdSrcHitlist.SetFile ( tSrcHitlist.GetFD(), tSrcHitlist.GetFilename() );	

	rdDstData.SeekTo( 1, 0 );
	rdDstHitlist.SeekTo( 1, 0 );

	if ( rdDstData.Tell() >= tDstData.GetSize() || rdDstHitlist.Tell() >= tDstHitlist.GetSize() )
		bDstEmpty = true;

	rdSrcData.SeekTo( 1, 0 );
	rdSrcHitlist.SeekTo( 1, 0 );

	if ( rdSrcData.Tell() >= tSrcData.GetSize() || rdSrcHitlist.Tell() >= tSrcHitlist.GetSize() )
		bSrcEmpty = true;

	if ( !wrDstData.OpenFile ( GetIndexFileName("spd.tmp"), m_sLastError ) )
		return false;	
	if ( !wrDstHitlist.OpenFile ( GetIndexFileName("spp.tmp"), m_sLastError ) )
		return false;	
	if ( !wrDstIndex.OpenFile ( GetIndexFileName("spi.tmp"), m_sLastError ) )
		return false;

	BYTE bDummy = 1;
	wrDstData.PutBytes ( &bDummy, 1 );
	wrDstIndex.PutBytes ( &bDummy, 1 );
	wrDstHitlist.PutBytes ( &bDummy, 1 );

	CSphVector<CSphWordlistCheckpoint> dWordlistCheckpoints;
	dWordlistCheckpoints.Reserve ( 1024 );

	int iWordListEntries = 0;
	CSphMergeSource tDstSource, tSrcSource;

	tDstSource.m_pWordlist = !bDstEmpty? &m_pWordlist[1] : NULL;
	tDstSource.m_pWordlistEnd = ( const BYTE * )m_pWordlistCheckpoints;
	tDstSource.m_pDoclistReader = &rdDstData;	
	tDstSource.m_pHitlistReader = &rdDstHitlist;
	tDstSource.m_iRowitems = ( m_eDocinfo == SPH_DOCINFO_INLINE )? m_tSchema.GetRowSize() : 0;
	tDstSource.m_iLastDocID = m_tMin.m_iDocID;
	tDstSource.m_iMinDocID = m_tMin.m_iDocID;
	tDstSource.m_pMinAttrs = m_tMin.m_pRowitems;
	tDstSource.m_pIndex = this;
	tDstSource.m_iMaxWordlistEntries = WORDLIST_CHECKPOINT;
	
	tSrcSource.m_pWordlist = !bSrcEmpty? &pSrcIndex->m_pWordlist[1] : NULL;
	tSrcSource.m_pWordlistEnd = ( const BYTE * )pSrcIndex->m_pWordlistCheckpoints;
	tSrcSource.m_pDoclistReader = &rdSrcData;
	tSrcSource.m_pHitlistReader = &rdSrcHitlist;
	tSrcSource.m_iRowitems = ( pSrcIndex->m_eDocinfo == SPH_DOCINFO_INLINE )? pSrcIndex->m_tSchema.GetRowSize() : 0;
	tSrcSource.m_iLastDocID = pSrcIndex->m_tMin.m_iDocID;
	tSrcSource.m_iMinDocID = pSrcIndex->m_tMin.m_iDocID;
	tSrcSource.m_pMinAttrs = pSrcIndex->m_tMin.m_pRowitems;
	tSrcSource.m_pIndex = pSrcIndex;
	tSrcSource.m_iMaxWordlistEntries = WORDLIST_CHECKPOINT;

	if ( m_pProgress )
	{
		m_tProgress.m_ePhase = CSphIndexProgress::PHASE_MERGE;
		m_pProgress ( &m_tProgress, false );
	}	
		
	if ( m_eDocinfo == SPH_DOCINFO_INLINE && pSrcIndex->m_eDocinfo == SPH_DOCINFO_EXTERN )
	{
		tSrcSource.m_iRowitems = pSrcIndex->m_tSchema.GetRowSize();
		tSrcSource.m_tMatch.Reset ( tSrcSource.m_iRowitems );
		tSrcSource.m_bForceDocinfo = true;
	}

	CSphMergeData tMerge;
	tPurgeData.m_iAttrIndex = m_tSchema.GetAttrIndex( tPurgeData.m_sKey.cstr() );
	tMerge.m_pPurgeData = &tPurgeData;
	tMerge.m_iMinDocID = Min( m_tMin.m_iDocID, pSrcIndex->m_tMin.m_iDocID );
	tMerge.m_iLastDocID = tMerge.m_iMinDocID;
	tMerge.m_pIndexWriter = &wrDstIndex;
	tMerge.m_pDataWriter = &wrDstData;
	tMerge.m_pHitlistWriter = &wrDstHitlist;
	tMerge.m_eDocinfo = m_eDocinfo;

	int iMinAttrSize = m_tSchema.GetRowSize();
	assert ( iMinAttrSize );

	tMerge.m_pMinAttrs = new CSphRowitem [ iMinAttrSize ];
	for( int i = 0; i < iMinAttrSize; i++ )
	{
		if ( bDstEmpty )
			tMerge.m_pMinAttrs[i] = pSrcIndex->m_tMin.m_pRowitems[i];
		else if ( bSrcEmpty )
			tMerge.m_pMinAttrs[i] = m_tMin.m_pRowitems[i];
		else
			tMerge.m_pMinAttrs[i] = Min ( m_tMin.m_pRowitems[i], pSrcIndex->m_tMin.m_pRowitems[i] );
	}

	CSphWordRecord		tDstWord ( &tDstSource, &tMerge );
	CSphWordRecord		tSrcWord ( &tSrcSource, &tMerge );
	
	DWORD uProgress = 0x03;

	if ( !bDstEmpty )
	{
		if ( !tDstWord.Read() )
			uProgress &= ~0x01;
	}
	else
		uProgress &= ~0x01;

	if ( !bSrcEmpty )
	{
		if ( !tSrcWord.Read() )
			uProgress &= ~0x02;
	}
	else
		uProgress &= ~0x02;

	tMerge.m_iWordlistOffset = wrDstIndex.GetPos();
	
	while ( uProgress ) 
	{
		if ( iWordListEntries == WORDLIST_CHECKPOINT )
		{	
			wrDstIndex.ZipInt ( 0 );
			wrDstIndex.ZipOffset ( wrDstData.GetPos() - tMerge.m_iLastDoclistPos );
			tMerge.m_iLastWordID = 0;
			tMerge.m_iLastDoclistPos = 0;
			iWordListEntries = 0;
		}

		if ( ( ( uProgress & 0x01 ) && tDstWord.m_tWordIndex < tSrcWord.m_tWordIndex ) || !( uProgress & 0x02 ) )
		{
			if ( !tDstWord.IsEmpty() )
			{
				tDstWord.Write();
				m_tProgress.m_iWords++;
			}
			
			if ( !tDstWord.Read() )
				uProgress &= ~0x01;
		}
		else if ( ( ( uProgress & 0x02 ) && tDstWord.m_tWordIndex > tSrcWord.m_tWordIndex ) || !( uProgress & 0x01 ) )
		{
			if ( !tSrcWord.IsEmpty() )
			{
				tSrcWord.Write();
				m_tProgress.m_iWords++;
			}

			if ( !tSrcWord.Read() )
				uProgress &= ~0x02;
		}
		else
		{
			assert ( uProgress & 0x03 );

			MergeWordData ( tDstWord, tSrcWord );
			
			tDstWord.Write();
			m_tProgress.m_iWords++;

			if ( !tDstWord.Read() )
				uProgress &= ~0x01;

			if ( !tSrcWord.Read() )
				uProgress &= ~0x02;
		}

		if ( iWordListEntries == 0 )
		{
			CSphWordlistCheckpoint tCheckpoint;
			tCheckpoint.m_iWordID = tMerge.m_iLastWordID;
			tCheckpoint.m_iWordlistOffset = ( DWORD ) tMerge.m_iWordlistOffset;

			dWordlistCheckpoints.Add ( tCheckpoint );
		}

		iWordListEntries++;

		if ( m_pProgress && !(m_tProgress.m_iWords % 500) )
			m_pProgress ( &m_tProgress, false );
	}

	wrDstIndex.ZipInt ( 0 );
	wrDstIndex.ZipOffset ( wrDstData.GetPos() - tMerge.m_iDoclistPos );	

	SphOffset_t iCheckpointsPos = wrDstIndex.GetPos();

	if ( dWordlistCheckpoints.GetLength() )
		wrDstIndex.PutBytes ( &dWordlistCheckpoints[0], dWordlistCheckpoints.GetLength() * sizeof ( CSphWordlistCheckpoint ) );

	CSphWriter fdInfo;
	if ( !fdInfo.OpenFile ( GetIndexFileName("sph.tmp"), m_sLastError ) )
		return false;

	// version
	fdInfo.PutDword ( INDEX_MAGIC_HEADER );
	fdInfo.PutDword ( INDEX_FORMAT_VERSION );

	// bits
	fdInfo.PutDword ( USE_64BIT );

	// docinfo
	fdInfo.PutDword ( m_eDocinfo );

	// schema
	fdInfo.PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		WriteSchemaColumn ( fdInfo, m_tSchema.m_dFields[i] );

	fdInfo.PutDword ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( fdInfo, m_tSchema.GetAttr(i) );

	// min doc
	fdInfo.PutOffset ( tMerge.m_iMinDocID );

	for ( int i = 0; i < m_tMin.m_iRowitems; i++ )
		m_tMin.m_pRowitems[i] = tMerge.m_pMinAttrs[i];

	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		fdInfo.PutBytes ( m_tMin.m_pRowitems, m_tMin.m_iRowitems*sizeof(CSphRowitem) );

	// wordlist checkpoints
	fdInfo.PutBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	fdInfo.PutDword ( dWordlistCheckpoints.GetLength() );

	int iTotalDocs = m_tStats.m_iTotalDocuments + pSrcIndex->m_tStats.m_iTotalDocuments - iDeltaTotalDocs;
	SphOffset_t iTotalBytes = m_tStats.m_iTotalBytes + pSrcIndex->m_tStats.m_iTotalBytes;

	// index stats
	fdInfo.PutDword ( iTotalDocs );
	fdInfo.PutOffset ( iTotalBytes );

	fdInfo.PutDword ( m_iMinPrefixLen );
	fdInfo.PutDword ( m_iMinInfixLen );

	if ( m_pProgress )
		m_pProgress ( &m_tProgress, true );

	return true;
}


void CSphIndex_VLN::MergeWordData ( CSphWordRecord & tDstWord, CSphWordRecord & tSrcWord )
{
	assert ( tDstWord.m_pMergeSource->Check() );
	assert ( tSrcWord.m_pMergeSource->Check() );
	assert ( tDstWord.m_tWordIndex == tSrcWord.m_tWordIndex );

	static CSphVector<DWORD> dWordPos;
	dWordPos.Reserve ( 1024 );

	static CSphVector<CSphDoclistRecord> dDoclist;
	dDoclist.Reserve ( 1024 );

	CSphWordDataRecord *					pDstData = &tDstWord.m_tWordData;
	CSphWordDataRecord *					pSrcData = &tSrcWord.m_tWordData;
	
	int iDstWordPos = 0;
	int iSrcWordPos = 0;

	int iDstDocNum = pDstData->m_dDoclist.GetLength();
	int iSrcDocNum = pSrcData->m_dDoclist.GetLength();

	assert ( iDstDocNum );
	assert ( iSrcDocNum );

	int iTotalHits = 0;
	int iTotalDocs = 0;

	SphDocID_t iDstDocID = pDstData->m_dDoclist[0].m_iDocID;
	SphDocID_t iSrcDocID = pSrcData->m_dDoclist[0].m_iDocID;

	int iDstCount = 0;
	int iSrcCount = 0;

	while ( iDstDocID || iSrcDocID )
	{
		if ( ( iDstDocID && iDstDocID<iSrcDocID ) || iSrcDocID==0 )
		{
			DWORD iPos = 0;
			do 
			{
				iPos = pDstData->m_dWordPos[iDstWordPos++];
				dWordPos.Add ( iPos );
				if ( iPos )
					iTotalHits++;
			}
			while ( iPos );
			
			dDoclist.Add ( pDstData->m_dDoclist[iDstCount] );
			
			iTotalDocs++;			
			iDstCount++;

			if ( iDstCount < iDstDocNum )
				iDstDocID = pDstData->m_dDoclist[iDstCount].m_iDocID;
			else
				iDstDocID = 0;
		}
		else if ( ( iSrcDocID && iDstDocID>iSrcDocID ) || iDstDocID==0 )
		{
			DWORD iPos = 0;
			do 
			{
				iPos = pSrcData->m_dWordPos[iSrcWordPos++];
				dWordPos.Add ( iPos );
				if ( iPos )
					iTotalHits++;
			}
			while ( iPos );

			dDoclist.Add ( pSrcData->m_dDoclist[iSrcCount] );

			iTotalDocs++;
			iSrcCount++;

			if ( iSrcCount < iSrcDocNum )
				iSrcDocID = pSrcData->m_dDoclist[iSrcCount].m_iDocID;
			else
				iSrcDocID = 0;
		}
		else
		{
			assert ( iDstDocID );
			assert ( iSrcDocID );

			DWORD iDstPos = 0;
			DWORD iSrcPos = 0;

			DWORD uCompleted = 0;

			while ( uCompleted != 0x03 ) 
			{
				if ( !( uCompleted & 0x01 ) )
				{
					iDstPos = pDstData->m_dWordPos[iDstWordPos++];
					if ( !iDstPos )
						uCompleted |= 0x01;
				}

				if ( !( uCompleted & 0x02 ) )
				{
					iSrcPos = pSrcData->m_dWordPos[iSrcWordPos++];
					if ( !iSrcPos )
						uCompleted |= 0x02;
				}

				if ( uCompleted != 0x03 )
				{
					if ( iDstPos <= iSrcPos )
						dWordPos.Add ( iDstPos );
					else if ( iDstPos > iSrcPos )
						dWordPos.Add ( iSrcPos );

					iTotalHits++;
				}				
			}

			dWordPos.Add ( 0 );	
			dDoclist.Add ( pDstData->m_dDoclist[iDstCount] );

			iTotalDocs++;
			iDstCount++;
			iSrcCount++;

			if ( iDstCount < iDstDocNum )
				iDstDocID += pDstData->m_dDoclist[iDstCount].m_iDocID;
			else
				iDstDocID = 0;

			if ( iSrcCount < iSrcDocNum )
				iSrcDocID += pSrcData->m_dDoclist[iSrcCount].m_iDocID;
			else
				iSrcDocID = 0;
		}
	}

	tDstWord.m_tWordData.m_dWordPos.SwapData ( dWordPos );
	tDstWord.m_tWordData.m_dDoclist.SwapData ( dDoclist );
	dWordPos.Resize ( 0 );
	dDoclist.Resize ( 0 );

	tDstWord.m_tWordIndex.m_iHitNum = iTotalHits;
	tDstWord.m_tWordIndex.m_iDocNum = iTotalDocs;	
}

/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

/// dict wrapper for star-syntax support in prefix-indexes
class CSphDictStar : public CSphDict
{
public:
						CSphDictStar ( CSphDict * pDict ) : m_pDict ( pDict ) { assert ( m_pDict ); }

	void				LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer ) { m_pDict->LoadStopwords ( sFiles, pTokenizer ); }
	bool				LoadWordforms ( const char * sFile, ISphTokenizer * pTokenizer ) { return m_pDict->LoadWordforms ( sFile, pTokenizer ); }
	bool				SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError ) { return m_pDict->SetMorphology ( sMorph, bUseUTF8, sError ); }

	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen ) { return m_pDict->GetWordID ( pWord, iLen ); }

protected:
	CSphDict *			m_pDict;
};


SphWordID_t CSphDictStar::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	m_pDict->ApplyStemmers ( pWord );

	int iLen = strlen ( (const char*)pWord );
	memcpy ( sBuf, pWord, iLen+1 );

	if ( iLen )
	{
		if ( sBuf[iLen-1]=='*' )
		{
			sBuf[iLen-1] = '\0';
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL;
			sBuf[iLen+1] = '\0';
		}
	}

	return m_pDict->GetWordID ( (BYTE*)sBuf );
}

//////////////////////////////////////////////////////////////////////////

/// star dict for index v.8+
class CSphDictStarV8 : public CSphDictStar
{
public:
						CSphDictStarV8 ( CSphDict * pDict, bool bPrefixes, bool bInfixes );

	virtual SphWordID_t	GetWordID ( BYTE * pWord );

private:
	bool				m_bPrefixes;
	bool				m_bInfixes;
};


CSphDictStarV8::CSphDictStarV8 ( CSphDict * pDict, bool bPrefixes, bool bInfixes )
	: CSphDictStar	( pDict )
	, m_bPrefixes	( bPrefixes )
	, m_bInfixes	( bInfixes )
{
}


SphWordID_t	CSphDictStarV8::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iLen = strlen ( (const char*)pWord );
	if ( !iLen )
		return 0;

	bool bHeadStar = ( pWord[0]=='*' );
	bool bTailStar = ( pWord[iLen-1]=='*' );

	if ( !bHeadStar && !bTailStar )
		m_pDict->ApplyStemmers ( pWord );

	iLen = strlen ( (const char*)pWord );
	if ( !iLen )
		return 0;

	if ( m_bInfixes )
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

		assert ( m_bPrefixes );

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
	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen );
}

/////////////////////////////////////////////////////////////////////////////

/// my simple query parser
struct CSphSimpleQueryParser
{
	struct
	{
		CSphString		m_sWord;
		SphWordID_t		m_uWordID;
		int				m_iQueryPos;
	} m_dWords [ SPH_MAX_QUERY_WORDS ];

	CSphSimpleQueryParser () {}

	int Parse ( const char * sQuery, ISphTokenizer * pTokenizer, CSphDict * pDict )
	{
		assert ( sQuery );
		assert ( pTokenizer );
		assert ( pDict );

		CSphString sQbuf ( sQuery );
		pTokenizer->SetBuffer ( (BYTE*)sQbuf.cstr(), strlen(sQuery) );

		int iWords = 0;
		int iPos = 0;

		BYTE * sWord;
		while ( ( sWord = pTokenizer->GetToken() )!=NULL && iWords<SPH_MAX_QUERY_WORDS )
		{
			SphWordID_t iWord = pDict->GetWordID ( sWord );
			if ( iWord )
			{
				m_dWords[iWords].m_sWord = (const char*)sWord;
				m_dWords[iWords].m_uWordID = iWord;
				m_dWords[iWords].m_iQueryPos = ++iPos;
				++iWords;
			} else
			{
				if ( iPos )
					++iPos;
			}
		}

		return iWords;
	}
};


/// query-word comparator
bool operator < ( const CSphQueryWord & a, const CSphQueryWord & b )
{
	return ( a.m_iDocs==b.m_iDocs )
		? ( a.m_iQueryPos < b.m_iQueryPos ) // this is tricky. with "xxx yyy xxx" queries, we need first query word to emit positions first
		: ( a.m_iDocs < b.m_iDocs );
}


inline int sphBitCount ( DWORD n )
{
	// MIT HACKMEM count
	// works for 32-bit numbers only
	// fix last line for 64-bit numbers
	register DWORD tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}


inline bool sphGroupMatch ( DWORD iGroup, const DWORD * pGroups, int iGroups )
{
	if ( !pGroups ) return true;
	const DWORD * pA = pGroups;
	const DWORD * pB = pGroups+iGroups-1;
	if ( iGroup==*pA || iGroup==*pB ) return true;
	if ( iGroup<(*pA) || iGroup>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		const DWORD * pM = pA + ((pB-pA)/2);
		if ( iGroup==(*pM) )
			return true;
		if ( iGroup<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}


CSphQueryResult * CSphIndex_VLN::Query ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery )
{
	// create sorter
	CSphString sError;
	ISphMatchSorter * pTop = sphCreateQueue ( pQuery, m_tSchema, sError );
	if ( !pTop )
	{
		m_sLastError.SetSprintf ( "failed to create sorting queue: %s", sError.cstr() );
		return NULL;
	}

	// create result
	CSphQueryResult * pResult = new CSphQueryResult();

	// run query
	if ( QueryEx ( pTokenizer, pDict, pQuery, pResult, pTop ) )
	{
		// convert results and return
		pResult->m_dMatches.Reset ();
		sphFlattenQueue ( pTop, pResult, 0 );
	} else
	{
		SafeDelete ( pResult );
	}

	SafeDelete ( pTop );
	return pResult;
}


static inline bool sphMatchEarlyReject ( const CSphMatch & tMatch, const CSphQuery * pQuery, CSphSharedBuffer<DWORD> & pMvaStorage )
{
	if ( tMatch.m_iDocID < pQuery->m_iMinID || tMatch.m_iDocID > pQuery->m_iMaxID )
		return true;

	if ( !pQuery->m_dFilters.GetLength() )
		return false;

	ARRAY_FOREACH ( i, pQuery->m_dFilters )
	{
		const CSphFilter & tFilter = pQuery->m_dFilters[i];
		if ( tFilter.m_iBitOffset<0 )
			continue;

		if ( tFilter.m_bMva )
		{
			// multiple values
			CSphRowitem uOff = tMatch.GetAttr ( tFilter.m_iRowitem );

			const DWORD * pMva = NULL;
			const DWORD * pMvaMax = NULL;
			if ( uOff )
			{
				pMva = &pMvaStorage [ uOff ];
				pMvaMax = pMva + (*pMva) + 1;
				pMva++;
			}

			// filter matches if any of multiple values match (ie. are in the set, or in the range)
			bool bOK = false;
			if ( uOff )
				switch ( tFilter.m_eType )
			{
				case SPH_FILTER_VALUES:
				{
					// walk both attr values and filter values lists, and ok match if there's any intersection
					const DWORD * pFilter = &tFilter.m_dValues[0];
					const DWORD * pFilterMax = pFilter + tFilter.m_dValues.GetLength();

					while ( pMva<pMvaMax && pFilter<pFilterMax && pMva[0]!=pFilter[0] )
					{
						while ( pMva<pMvaMax && pMva[0]<pFilter[0] ) pMva++;
						if ( pMva>=pMvaMax ) break;

						while ( pFilter<pFilterMax && pFilter[0]<pMva[0] ) pFilter++;
						if ( pFilter>=pFilterMax ) break;

					}

					bOK = ( pMva<pMvaMax && pFilter<pFilterMax );
					assert ( bOK==false || pMva[0]==pFilter[0] );

					// FIXME! should we report matched-value somehow?
					// if ( bOK ) tMatch.SetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount, pFilter[0] );
					break;
				}

				case SPH_FILTER_RANGE:
					// walk attr list, and ok match if any of values is within range
					while ( pMva<pMvaMax )
					{
						if ( pMva[0]>=tFilter.m_uMinValue && pMva[0]<=tFilter.m_uMaxValue )
						{
							bOK = true;
							// FIXME! should we report matched-value somehow?
							// tMatch.SetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount, pMva[0] );
							break;
						}
						pMva++;
					}
					break;

				default:
					assert ( 0 && "internal error: unhandled MVA filter type" );
					break;
			}

			if ( !( tFilter.m_bExclude ^ bOK ) )
				return true;

		} else
		{
			// single value
			DWORD uValue;
			float fValue;
			switch ( tFilter.m_eType )
			{
				case SPH_FILTER_VALUES:
					uValue = tMatch.GetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount );
					if ( !( tFilter.m_bExclude ^ sphGroupMatch ( uValue, &tFilter.m_dValues[0], tFilter.m_dValues.GetLength() ) ) )
						return true;
					break;

				case SPH_FILTER_RANGE:
					uValue = tMatch.GetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount );
					if ( tFilter.m_bExclude ^ ( uValue<tFilter.m_uMinValue || uValue>tFilter.m_uMaxValue ) )
						return true;
					break;

				case SPH_FILTER_FLOATRANGE:
					fValue = tMatch.GetAttrFloat ( tFilter.m_iRowitem );
					if ( tFilter.m_bExclude ^ ( fValue<tFilter.m_fMinValue || fValue>tFilter.m_fMaxValue ) )
						return true;
					break;

				default:
					assert ( 0 && "internal error: unhandled filter type" );
					break;
			}
		}
	}

	return false;
}


const DWORD * CSphIndex_VLN::FindDocinfo ( SphDocID_t uDocID )
{
	if ( m_uDocinfo<=0 )
		return NULL;

	assert ( m_eDocinfo==SPH_DOCINFO_EXTERN );
	assert ( !m_pDocinfo.IsEmpty() );
	assert ( m_tSchema.GetAttrsCount() );

	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	SphDocID_t uFirst = DOCINFO2ID ( &m_pDocinfo[0] );
	SphDocID_t uLast = DOCINFO2ID ( &m_pDocinfo[(m_uDocinfo-1)*iStride] );
	if ( uDocID<uFirst || uDocID>uLast )
		return NULL;

	DWORD uHash = (DWORD)( ( uDocID - uFirst ) >> m_pDocinfoHash[0] );
	if ( uHash>(1<<DOCINFO_HASH_BITS) ) // possible in case of broken data, for instance
		return NULL;

	int iStart = m_pDocinfoHash [ uHash+1 ];
	int iEnd = m_pDocinfoHash [ uHash+2 ] - 1;

	const DWORD * pFound = NULL;

	if ( uDocID==DOCINFO2ID ( &m_pDocinfo [ iStart*iStride ] ) )
	{
		pFound = &m_pDocinfo [ iStart*iStride ];

	} else if ( uDocID==DOCINFO2ID ( &m_pDocinfo [ iEnd*iStride ] ) )
	{
		pFound = &m_pDocinfo [ iEnd*iStride ];

	} else
	{
		while ( iEnd-iStart>1 )
		{
			// check if nothing found
			if (
				uDocID < DOCINFO2ID ( &m_pDocinfo [ iStart*iStride ] ) ||
				uDocID > DOCINFO2ID ( &m_pDocinfo [ iEnd*iStride ] ) )
					break;
			assert ( uDocID > DOCINFO2ID ( &m_pDocinfo [ iStart*iStride ] ) );
			assert ( uDocID < DOCINFO2ID ( &m_pDocinfo [ iEnd*iStride ] ) );

			int iMid = iStart + (iEnd-iStart)/2;
			if ( uDocID==DOCINFO2ID ( &m_pDocinfo [ iMid*iStride ] ) )
			{
				pFound = &m_pDocinfo [ iMid*iStride ];
				break;
			}
			if ( uDocID<DOCINFO2ID ( &m_pDocinfo [ iMid*iStride ] ) )
				iEnd = iMid;
			else
				iStart = iMid;
		}
	}

	return pFound;
}


static inline double sphSqr ( double v )
{
	return v*v;
}


void CSphIndex_VLN::CalcDocinfo ( CSphMatch & tMatch, const DWORD * pInfo )
{
	assert ( tMatch.m_pRowitems );
//	assert ( tMatch.m_iRowitems==m_tSchema.GetRowSize() + ( m_bCalcGeodist ? 1 : 0 ) );
	assert ( DOCINFO2ID(pInfo)==tMatch.m_iDocID );
	memcpy ( tMatch.m_pRowitems, DOCINFO2ATTRS(pInfo), m_tSchema.GetRowSize()*sizeof(CSphRowitem) );

	if ( m_iCalcGeodist>=0 )
	{
		const double R = 6384000;
		float plat = tMatch.GetAttrFloat ( m_iGeoLatRowitem );
		float plon = tMatch.GetAttrFloat ( m_iGeoLongRowitem );
		double dlat = plat - m_fGeoAnchorLat;
		double dlon = plon - m_fGeoAnchorLong;
		double a = sphSqr(sin(dlat/2)) + cos(plat)*cos(m_fGeoAnchorLat)*sphSqr(sin(dlon/2));
		double c = 2*asin ( Min ( 1, sqrt(a) ) );
		tMatch.SetAttrFloat ( m_iCalcGeodist, float(R*c) );
	}

	if ( m_iCalcExpr>=0 )
	{
		float fRes = m_tCalcExpr.Eval ( tMatch );
		tMatch.SetAttrFloat ( m_iCalcExpr, fRes );
	}
}


void CSphIndex_VLN::LookupDocinfo ( CSphMatch & tMatch )
{
	const DWORD * pFound = FindDocinfo ( tMatch.m_iDocID );
	if ( pFound )
		CalcDocinfo ( tMatch, pFound );
}


#define SPH_SUBMIT_MATCH(_match) \
	if ( m_bLateLookup ) \
		LookupDocinfo ( _match ); \
	\
	if ( bRandomize ) \
		(_match).m_iWeight = rand(); \
	\
	bool bNewMatch = false; \
	for ( int iSorter=0; iSorter<iSorters; iSorter++ ) \
		bNewMatch |= ppSorters[iSorter]->Push ( _match ); \
	\
	if ( bNewMatch ) \
		if ( --iCutoff==0 ) \
			break;


// check for dupes
struct WordDupeCheck_t
{
	SphWordID_t		m_uWordID;
	int				m_iIndex;

	bool operator < ( const WordDupeCheck_t & rhs ) const
	{
		return m_uWordID < rhs.m_uWordID;
	}
};


void CSphIndex_VLN::MatchAll ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters )
{
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;

	///////////////////
	// match all words
	///////////////////

	if ( !m_dQueryWords[0].m_iDocs )
		return;

	// preload doclist entries, and calc max qpos
	int i, iMaxQpos = 0;
	for ( i=0; i<m_dQueryWords.GetLength(); i++ )
	{
		m_dQueryWords[i].GetDoclistEntry ();
		iMaxQpos = Max ( iMaxQpos, m_dQueryWords[i].m_iQueryPos );
	}
	iMaxQpos--; // because we'll check base-0 count

	// check for dupes
	CSphVector<WordDupeCheck_t> dCheck;
	dCheck.Resize ( m_dQueryWords.GetLength() );

	for ( i=0; i<m_dQueryWords.GetLength(); i++ )
	{
		m_dQueryWords[i].m_bDupe = false;
		dCheck[i].m_uWordID = m_dQueryWords[i].m_iWordID;
		dCheck[i].m_iIndex = i;
	}

	dCheck.Sort ();
	for ( i=1; i<dCheck.GetLength(); i++ )
		if ( dCheck[i].m_uWordID==dCheck[i-1].m_uWordID )
	{
		m_dQueryWords [ dCheck[i].m_iIndex ].m_bDupe = true;
		m_dQueryWords [ dCheck[i-1].m_iIndex ].m_bDupe = true;
	}

	// max_query_time
	DWORD uMaxStamp = 0;
	if ( pQuery->m_uMaxQueryMsec )
		uMaxStamp = sphTimerMsec () + pQuery->m_uMaxQueryMsec;
	int iScannedEntries = 0;

	// main loop
	i = 0;
	SphDocID_t docID = 0;
	for ( ;; )
	{
		/////////////////////
		// obtain next match
		/////////////////////

		// scan lists until *all* the ids match
		while ( m_dQueryWords[i].m_tDoc.m_iDocID && docID>m_dQueryWords[i].m_tDoc.m_iDocID )
		{
			m_dQueryWords[i].GetDoclistEntry ();

			// max_query_time
			if ( uMaxStamp && ++iScannedEntries==1000 )
			{
				iScannedEntries = 0;
				if ( sphTimerMsec()>=uMaxStamp )
				{
					m_dQueryWords[i].m_tDoc.m_iDocID = 0;
					pResult->m_sWarning = "query time exceeded max_query_time";
					break;
				}
			}
		}
		if ( m_dQueryWords[i].m_tDoc.m_iDocID==0 )
			break;

		if ( docID<m_dQueryWords[i].m_tDoc.m_iDocID )
		{
			docID = m_dQueryWords[i].m_tDoc.m_iDocID;
			i = 0;
			continue;
		}
		if ( ++i != m_dQueryWords.GetLength() )
			continue;

		// this is my match
		CSphMatch & tMatch = m_dQueryWords[0].m_tDoc;

		// early reject by group id, doc id or timestamp
		if ( m_bEarlyLookup )
			LookupDocinfo ( tMatch );
		if ( sphMatchEarlyReject ( tMatch, pQuery, m_pMva ) )
		{
			docID++;
			i = 0;
			continue;
		}

		//////////////
		// rank match
		//////////////

		if ( m_dQueryWords.GetLength()==1 )
		{
			// sum simple match weights and phrase match weights
			tMatch.m_iWeight = 0;

			// FIXME? incompatible with 096
			DWORD uFields = m_dQueryWords[0].m_uFields;
			for ( int iField = 0; uFields; iField++ )
			{
				if ( uFields & 1 )
					tMatch.m_iWeight += m_dWeights[iField];
				uFields >>= 1;
			}

		} else
		{
			// preload hitlist entries
			for ( i=0; i<m_dQueryWords.GetLength(); i++ )
				m_dQueryWords[i].GetHitlistEntry ();

			// init weighting
			BYTE curPhraseWeight [ SPH_MAX_FIELDS ];
			BYTE phraseWeight [ SPH_MAX_FIELDS ];
			BYTE matchWeight [ SPH_MAX_FIELDS ];
			for ( i=0; i<m_iWeights; i++ )
			{
				curPhraseWeight[i] = 0;
				phraseWeight[i] = 0;
				matchWeight[i] = 0;
			}

			DWORD uSpanStart = UINT_MAX;
			DWORD uSpanEnd = UINT_MAX;
			bool bPhraseMatch = false;

			DWORD k = INT_MAX;
			for ( ;; )
			{
				// scan until next hit in this document
				DWORD pmin = INT_MAX;
				int imin = -1;
				for ( i=0; i<m_dQueryWords.GetLength(); i++ )
				{
					if ( !m_dQueryWords[i].m_iHitPos )
						continue;
					if ( pmin>m_dQueryWords[i].m_iHitPos )
					{
						pmin = m_dQueryWords[i].m_iHitPos;
						imin = i;
					}
				}
				if ( imin<0 )
					break;

				if ( m_dQueryWords[imin].m_bDupe )
				{
					for ( i=imin; i<m_dQueryWords.GetLength(); i++ )
						if ( m_dQueryWords[i].m_iHitPos==pmin )
					{
						if ( m_dQueryWords[i].m_iQueryPos-pmin==k )
							imin = i;
						m_dQueryWords[i].GetHitlistEntry ();
					}

				} else
				{
					m_dQueryWords[imin].GetHitlistEntry ();
				}

				// get field number and mark a simple match
				int j = pmin >> 24;
				assert ( j>=0 && j<m_tSchema.m_dFields.GetLength() );
				matchWeight[j] = 1;

				// find max proximity relevance
				if ( m_dQueryWords[imin].m_iQueryPos - pmin == k )
				{
					uSpanStart = Min ( uSpanStart, pmin );
					uSpanEnd = Max ( uSpanEnd, pmin );

					curPhraseWeight[j]++;
					if ( phraseWeight[j] < curPhraseWeight[j] )
					{
						phraseWeight[j] = curPhraseWeight[j];

						if ( (int)(uSpanEnd-uSpanStart)==iMaxQpos )
							if ( curPhraseWeight[j]==m_dQueryWords.GetLength()-1 )
								bPhraseMatch = true;
					}

				} else
				{
					curPhraseWeight[j] = 0;
					uSpanStart = uSpanEnd = pmin;
				}

				k = m_dQueryWords[imin].m_iQueryPos - pmin;
			}

			// check if there was a perfect match
			if ( pQuery->m_eMode==SPH_MATCH_PHRASE && m_dQueryWords.GetLength()>1 && !bPhraseMatch )
			{
				// there was not. continue
				i = 0;
				docID++;
				continue;
			}

			// sum simple match weights and phrase match weights
			tMatch.m_iWeight = 0;
			for ( i=0; i<m_iWeights; i++ )
				tMatch.m_iWeight += m_dWeights[i] * ( matchWeight[i] + phraseWeight[i] );
		}

		// submit
		SPH_SUBMIT_MATCH ( tMatch );

		// continue looking for next matches
		i = 0;
		docID++;
	}
}


void CSphIndex_VLN::MatchAny ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters )
{
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;

	//////////////////
	// match any word
	//////////////////

	int iActive = m_dQueryWords.GetLength(); // total number of words still active
	SphDocID_t iLastMatchID = 0;

	// preload entries
	int i;
	for ( i=0; i<m_dQueryWords.GetLength(); i++ )
		if ( m_dQueryWords[i].m_iDocs )
			m_dQueryWords[i].GetDoclistEntry ();

	// find a multiplier for phrase match
	// so that it will weight more than any word match
	int iPhraseK = 0;
	for ( i=0; i<m_iWeights; i++ )
		iPhraseK += m_dWeights[i] * m_dQueryWords.GetLength();

	// max_query_time
	DWORD uMaxStamp = 0;
	if ( pQuery->m_uMaxQueryMsec )
		uMaxStamp = sphTimerMsec () + pQuery->m_uMaxQueryMsec;
	int iScannedEntries = 0;

	for ( ;; )
	{
		// update active pointers to document lists, kill empty ones,
		// and get min current document id
		SphDocID_t iMinID = DOCID_MAX;
		int iMinIndex = -1;

		for ( i=0; i<iActive; i++ )
		{
			// move to next document
			if ( m_dQueryWords[i].m_tDoc.m_iDocID==iLastMatchID && iLastMatchID )
				m_dQueryWords[i].GetDoclistEntry ();

			// remove emptied words
			if ( m_dQueryWords[i].m_tDoc.m_iDocID==0 )
			{
				m_dQueryWords[i] = m_dQueryWords[iActive-1];
				i--;
				iActive--;
				continue;
			}

			// get new min id
			assert ( m_dQueryWords[i].m_tDoc.m_iDocID > iLastMatchID );
			if ( m_dQueryWords[i].m_tDoc.m_iDocID < iMinID )
			{
				iMinID = m_dQueryWords[i].m_tDoc.m_iDocID;
				iMinIndex = i;
			}
		}
		if ( iActive==0 )
			break;

		iLastMatchID = iMinID;
		assert ( iMinIndex>=0 );
		assert ( iLastMatchID );
		assert ( iLastMatchID!=DOCID_MAX );

		// max_query_time
		if ( uMaxStamp && ++iScannedEntries==1000 )
		{
			iScannedEntries = 0;
			if ( sphTimerMsec()>=uMaxStamp )
			{
				pResult->m_sWarning = "query time exceeded max_query_time";
				break;
			}
		}

		// early reject by group id, doc id or timestamp
		CSphMatch & tMatch = m_dQueryWords[iMinIndex].m_tDoc;
		if ( m_bEarlyLookup )
			LookupDocinfo ( tMatch );
		if ( sphMatchEarlyReject ( tMatch, pQuery, m_pMva ) )
			continue;

		// get the words we're matching current document against (let's call them "terms")
		CSphQueryWord * pHit [ SPH_MAX_QUERY_WORDS ];
		int iTerms = 0;

		for ( i=0; i<iActive; i++ )
			if ( m_dQueryWords[i].m_tDoc.m_iDocID==iLastMatchID )
		{
			pHit[iTerms++] = &m_dQueryWords[i];
			m_dQueryWords[i].GetHitlistEntry ();
		}
		assert ( iTerms>0 );

		// init weighting
		struct
		{
			int		m_iCurPhrase;	// current phrase-match weight
			int		m_iMaxPhrase;	// max phrase-match weight
			DWORD	m_uMatch;		// matching terms bitmap
		} dWeights [ SPH_MAX_FIELDS ];
		memset ( dWeights, 0, sizeof(dWeights) ); // OPTIMIZE: only zero out m_iWeights, not total

		// do matching
		// OPTIMIZE: implement simplified matching for iTerms==1 case
		DWORD k = INT_MAX;
		for ( ;; )
		{
			// get next good hit in this document
			DWORD pmin = INT_MAX;
			int imin = -1;
			for ( i=0; i<iTerms; i++ )
			{
				// if hit list for this term is no longer active, remove it
				if ( !pHit[i]->m_iHitPos )
				{
					pHit[i] = pHit[iTerms-1];
					i--;
					iTerms--;
					continue;
				}

				// my current best match
				if ( pmin>pHit[i]->m_iHitPos )
				{
					pmin = pHit[i]->m_iHitPos;
					imin = i;
				}
			}
			if ( imin<0 )
				break;
			pHit[imin]->GetHitlistEntry ();

			// get field number and mark a simple match
			int iField = pmin >> 24;
			dWeights[iField].m_uMatch |= ( 1<<pHit[imin]->m_iQueryPos );

			// find max proximity relevance
			if ( pHit[imin]->m_iQueryPos - pmin == k )
			{
				dWeights[iField].m_iCurPhrase++;
				if ( dWeights[iField].m_iMaxPhrase < dWeights[iField].m_iCurPhrase )
					dWeights[iField].m_iMaxPhrase = dWeights[iField].m_iCurPhrase;
			} else
			{
				dWeights[iField].m_iCurPhrase = 0;
			}
			k = pHit[imin]->m_iQueryPos - pmin;
		}

		// sum simple match weights and phrase match weights
		tMatch.m_iWeight = 0;
		for ( i=0; i<m_iWeights; i++ )
			tMatch.m_iWeight += m_dWeights[i] * ( sphBitCount(dWeights[i].m_uMatch) + dWeights[i].m_iMaxPhrase*iPhraseK );

		// submit
		SPH_SUBMIT_MATCH ( tMatch );
	}
}


struct CSphBooleanEvalNode : public CSphQueryWord
{
	ESphBooleanQueryExpr	m_eType;	///< this node's type

	CSphBooleanEvalNode *	m_pExpr;	///< subexperssion
	CSphBooleanEvalNode *	m_pParent;	///< subexperssion's parent

	CSphBooleanEvalNode *	m_pPrev;	///< prev expression at this level
	CSphBooleanEvalNode *	m_pNext;	///< next expression at this level

	CSphMatch *				m_pLast;	///< last matched document at this node (may point to a subexpression or sibling); we only need to store this for NODE_OR type

	bool					m_bInvert;		///< whether to invert the matching logic
	bool					m_bEvaluable;	///< whether this node is for matching or for filtering

public:
							CSphBooleanEvalNode ( const CSphBooleanQueryExpr * pNode, const CSphTermSetup & tTermSetup );
							~CSphBooleanEvalNode ();

	void					SetFile ( CSphIndex_VLN * pIndex, const CSphTermSetup & tTermSetup );
	CSphMatch *				MatchNode ( SphDocID_t iMinID, CSphString & sWarning );	///< get next match at this node, with ID greater than iMinID
	CSphMatch *				MatchLevel ( SphDocID_t iMinID, CSphString & sWarning );	///< get next match at this level, with ID greater than iMinID
	bool					IsRejected ( SphDocID_t iID, CSphString & sWarning );		///< check if this match should be rejected

protected:
	DWORD					m_uMaxStamp;
	int						m_iScannedEntries;
};


CSphBooleanEvalNode::CSphBooleanEvalNode ( const CSphBooleanQueryExpr * pNode, const CSphTermSetup & tTermSetup )
{
	assert ( pNode );
	assert ( tTermSetup.m_pDict );

	m_eType = pNode->m_eType;
	m_bInvert = pNode->m_bInvert;
	m_bEvaluable = pNode->m_bEvaluable;

	m_sWord = pNode->m_sWord;
	m_iWordID = 0;

	m_pExpr = pNode->m_pExpr ? new CSphBooleanEvalNode ( pNode->m_pExpr, tTermSetup ) : NULL;
	if ( m_pExpr )
	{
		m_pExpr->m_pParent = this;
		m_pLast = &m_tDoc;
	} else
	{
		// GetWordID() may modify the word in-place; so we alloc a tempbuffer
		CSphString sBuf ( m_sWord );
		m_iWordID = tTermSetup.m_pDict->GetWordID ( (BYTE*)sBuf.cstr() );
		m_pLast = NULL;
	}

	m_pNext = pNode->m_pNext ? new CSphBooleanEvalNode ( pNode->m_pNext, tTermSetup ) : NULL;
	m_pPrev = NULL;
	if ( m_pNext )
		m_pNext->m_pPrev = this;

	m_pParent = NULL;

	SetupAttrs ( tTermSetup );

	m_uMaxStamp = tTermSetup.m_uMaxStamp;
	m_iScannedEntries = 0;
}


CSphBooleanEvalNode::~CSphBooleanEvalNode ()
{
	SafeDelete ( m_pExpr );
	SafeDelete ( m_pNext );
}


void CSphBooleanEvalNode::SetFile ( CSphIndex_VLN * pIndex, const CSphTermSetup & tTermSetup )
{
	// setup self
	if ( pIndex->SetupQueryWord ( *this, tTermSetup ) )
		m_pLast = &m_tDoc;

	// setup children
	if ( m_pExpr )
		m_pExpr->SetFile ( pIndex, tTermSetup );

	// setup siblings
	if ( !m_pPrev )
	{
		for ( CSphBooleanEvalNode * pNode = m_pNext; pNode; pNode = pNode->m_pNext )
			pNode->SetFile ( pIndex, tTermSetup );
	}
}


bool CSphBooleanEvalNode::IsRejected ( SphDocID_t iID, CSphString & sWarning )
{
	assert ( !m_bEvaluable );
	assert ( m_pPrev ); // filter node can't start a level

	// "done" mark
	if ( !m_pLast )
		return false;

	// subexpr case
	if ( m_pExpr )
	{
		if ( m_bInvert )
		{
			// subexpr is directly evaluable
			m_pLast = m_pExpr->MatchNode ( iID, sWarning );
			return ( m_pLast && m_pLast->m_iDocID==iID );
		} else
		{
			// subexpr is not evaluable as well
			return m_pExpr->IsRejected ( iID, sWarning );
		}
	}

	// plain word case
	assert ( m_bInvert );
	while ( iID > m_tDoc.m_iDocID )
	{
		GetDoclistEntry ();
		if ( m_tDoc.m_iDocID==0 )
		{
			m_pLast = NULL;
			return false;
		}
	}
	return ( iID==m_tDoc.m_iDocID );
}


CSphMatch * CSphBooleanEvalNode::MatchNode ( SphDocID_t iMinID, CSphString & sWarning )
{
	assert ( m_bEvaluable );

	// "done" mark
	if ( !m_pLast )
		return NULL;

	// subexpr case
	if ( m_pExpr )
		return m_pExpr->MatchLevel ( iMinID, sWarning );

	// plain word case
	while ( iMinID > m_tDoc.m_iDocID )
	{
		GetDoclistEntry ();
		if ( !m_tDoc.m_iDocID )
			return NULL;

		// max_query_time
		if ( m_uMaxStamp && ++m_iScannedEntries==1000 )
		{
			if ( sphTimerMsec()>=m_uMaxStamp )
			{
				sWarning = "query time exceeded max_query_time";
				m_pLast = NULL;
				return NULL;
			}
			m_iScannedEntries = 0;
		}
	}
	return &m_tDoc;
}


CSphMatch * CSphBooleanEvalNode::MatchLevel ( SphDocID_t iMinID, CSphString & sWarning )
{
	assert ( m_bEvaluable );
	if ( m_eType==NODE_AND )
	{
		// match all siblings
		// search for min equal match ID through all the siblings
		CSphMatch * pCur = NULL;
		for ( CSphBooleanEvalNode * pNode = this; pNode; )
		{
			if ( pNode->m_bEvaluable )
			{
				// as all evaluatable siblings matched last time,
				// we are positive that it is time to advance the pointer
				// and fetch the next id
				CSphMatch * pCandidate = pNode->MatchNode ( iMinID, sWarning );
				if ( !pCandidate )
					return NULL;

				SphDocID_t iCandidate = pCandidate->m_iDocID;
				assert ( iCandidate );

				if ( iCandidate>=iMinID )
				{
					if ( iCandidate>iMinID )
						pNode = this;
					else
						pNode = pNode->m_pNext;

					iMinID = iCandidate;
					pCur = pCandidate;
				}
			} else
			{
				// if this node is a filter, well, we need some value to pass through it!
				assert ( pCur );
				if ( pNode->IsRejected ( pCur->m_iDocID, sWarning ) )
				{
					// oh, the doc's rejected. restart scanning
					iMinID = 1 + pCur->m_iDocID;
					pNode = this;
					pCur = NULL;
				} else
				{
					pNode = pNode->m_pNext;
				}
			}
		}

		assert ( pCur );
		return pCur;

	} else
	{
		// match any sibling
		// search for min match ID through all the siblings

		CSphMatch * pCur = NULL;
		SphDocID_t iMinMatch = DOCID_MAX; // best match ID found so far

		for ( CSphBooleanEvalNode * pNode = this; pNode; pNode = pNode->m_pNext )
		{
			assert ( pNode->m_bEvaluable );

			// if the node match is not OK already, ask for next one
			if ( pNode->m_pLast )
			{
				while ( pNode->m_pLast->m_iDocID < iMinID )
				{
					pNode->m_pLast = pNode->MatchNode ( iMinID, sWarning );
					if ( !pNode->m_pLast )
						break;
				}
			}
			if ( !pNode->m_pLast ) // OPTIMIZE! remove this node from the tree
				continue;

			// check if this match is any better than the current one
			SphDocID_t iCandidate = pNode->m_pLast->m_iDocID;
			assert ( iCandidate>=iMinID );

			if ( iCandidate<=iMinMatch )
			{
				iMinMatch = iCandidate;
				pCur = pNode->m_pLast;
			}
		}

		return pCur;
	}
}


void CSphIndex_VLN::CheckBooleanQuery ( const CSphBooleanQueryExpr * pExpr, CSphQueryResult * pResult )
{
	const CSphBooleanQueryExpr * pTestExpr = pExpr;

	while ( pTestExpr )
	{
		CheckQueryWord ( pTestExpr->m_sWord.cstr (), pResult );
		if ( pTestExpr->m_pExpr )
			CheckBooleanQuery ( pTestExpr->m_pExpr, pResult );

		pTestExpr = pTestExpr->m_pNext;
	}
}

void sphGatherBooleanWordStats ( CSphBooleanEvalNode * pTree, CSphQueryResult * pResult )
{
	CSphBooleanEvalNode * pNode = pTree;

	while ( pNode )
	{
		if ( pResult->m_iNumWords >= SPH_MAX_QUERY_WORDS )
			return;

		if ( pNode->m_pExpr )
			sphGatherBooleanWordStats ( pNode->m_pExpr, pResult );
		else
		{
			CSphQueryResult::WordStat_t & tStats = pResult->m_tWordStats [ pResult->m_iNumWords++ ];

			if ( tStats.m_sWord.cstr() )
			{
				assert ( tStats.m_sWord==pNode->m_sWord );

				tStats.m_iDocs += pNode->m_iDocs;
				tStats.m_iHits += pNode->m_iHits;
			} else
			{
				tStats.m_sWord = pNode->m_sWord;
				tStats.m_iDocs = pNode->m_iDocs;
				tStats.m_iHits = pNode->m_iHits;
			}
		}

		pNode = pNode->m_pNext;
	}
}


bool CSphIndex_VLN::MatchBoolean ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup )
{
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;

	/////////////////////////
	// match in boolean mode
	/////////////////////////

	// parse query
	CSphBooleanQuery tParsed;
	if ( !sphParseBooleanQuery ( tParsed, pQuery->m_sQuery.cstr(), tTermSetup.m_pTokenizer ) )
	{
		m_sLastError = tParsed.m_sParseError;
		return false;
	}

	CheckBooleanQuery ( tParsed.m_pTree, pResult );

	// let's build our own tree! with doclists! and hits!
	assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );
	
	if ( !tParsed.m_pTree )
		return true;

	CSphBooleanEvalNode tTree ( tParsed.m_pTree, tTermSetup );
	tTree.SetFile ( this, tTermSetup );

	// do matching
	SphDocID_t iMinID = 1;
	for ( ;; )
	{
		CSphMatch * pMatch = tTree.MatchLevel ( iMinID, pResult->m_sWarning );
		if ( !pMatch )
			break;
		iMinID = 1 + pMatch->m_iDocID;

		// early reject by group id, doc id or timestamp
		if ( m_bEarlyLookup )
			LookupDocinfo ( *pMatch );
		if ( sphMatchEarlyReject ( *pMatch, pQuery, m_pMva ) )
			continue;

		// set weight and push it to the queue
		pMatch->m_iWeight = 1; // set weight

		// submit
		SPH_SUBMIT_MATCH ( *pMatch );
	}

	sphGatherBooleanWordStats ( &tTree, pResult );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V1
//////////////////////////////////////////////////////////////////////////

struct CSphLCSState
{
	DWORD	m_uNextPos;						///< next pos expected
	DWORD	m_dCurLCS [ SPH_MAX_FIELDS ];	///< current LCS length
	DWORD	m_dMaxLCS [ SPH_MAX_FIELDS ];	///< max LCS length

	CSphLCSState ( int iFields )
	{
		for ( int i=0; i<iFields; i++ )
			m_dMaxLCS[i] = 0;
	}

	void CleanCurrent ( int iFields )
	{
		m_uNextPos = 0;
		for ( int i=0; i<iFields; i++ )
			m_dCurLCS[i] = 0;
	}
};


class CSphExtendedEvalAtom : public CSphExtendedQueryAtom
{
public:
						CSphExtendedEvalAtom ( const CSphExtendedQueryAtom & tAtom, const CSphTermSetup & tSetup );
	CSphMatch *			GetNextDoc ( SphDocID_t iMinID, CSphString & sWarning );
	void				GetNextHit ( DWORD iMinPos );
	void				GetLastTF ( DWORD * pTF ) const;
	inline CSphMatch *	GetLastMatch () const { return m_pLast; }
	void				UpdateLCS ( CSphLCSState & tState );

public:
	DWORD				m_uLastHitPos;

	CSphVector<CSphQueryWord>	m_dTerms;	///< query term readers
	CSphMatch *					m_pLast;	///< term with the highest position. NULL if this atom has no more matches

	DWORD				m_uMaxStamp;
	DWORD				m_uMinHitpos;
	DWORD				m_uMaxHitpos;
	int					m_iScannedEntries;
};


CSphExtendedEvalAtom::CSphExtendedEvalAtom ( const CSphExtendedQueryAtom & tAtom, const CSphTermSetup & tSetup )
	: m_pLast ( NULL )
	, m_uMaxStamp ( tSetup.m_uMaxStamp )
	, m_iScannedEntries ( 0 )
{
	assert ( tSetup.m_pDict );

	// atom assignment
	m_dWords = tAtom.m_dWords;
	m_uFields = tAtom.m_uFields;
	m_iMaxDistance = tAtom.m_iMaxDistance;

	int iMin = 0;
	int iMax = 0;
	for ( int i = 0; i < 32; ++i )
		if ( m_uFields & ( 1 << i ) )
		{
			if ( iMin == -1 )
				iMin = i;
			iMax = i;
		}

	if ( iMin == -1 )
		iMin = 0;

	m_uMinHitpos = ( iMin << 24 ) + 1;
	m_uMaxHitpos = ( 1 + iMax ) << 24;

	// setup words
	m_dTerms.Resize ( m_dWords.GetLength() );
	if ( m_dWords.GetLength() )
		m_pLast = &m_dTerms[0].m_tDoc;

	ARRAY_FOREACH ( iTerm, m_dTerms )
	{
		CSphQueryWord & tTerm = m_dTerms[iTerm];
		tTerm.m_sWord = m_dWords[iTerm].m_sWord;

		// GetWordID() may modify the word in-place; so we alloc a tempbuffer
		CSphString sBuf ( tTerm.m_sWord );
		tTerm.m_iWordID = tSetup.m_pDict->GetWordID ( (BYTE*)sBuf.cstr() );
		tTerm.SetupAttrs ( tSetup );

		if ( !tSetup.m_pIndex->SetupQueryWord ( tTerm, tSetup ) )
			m_pLast = NULL;
	}
}


void CSphExtendedEvalAtom::GetLastTF ( DWORD * pTF ) const
{
	ARRAY_FOREACH ( i, m_dTerms )
	{
		assert ( m_dTerms[i].m_iQueryPos>=0 ); // FIXME? verify max range as well
		pTF [ m_dTerms[i].m_iQueryPos ] = m_dTerms[i].m_uMatchHits;
	}
}


void CSphExtendedEvalAtom::GetNextHit ( DWORD iMinPos )
{
#ifndef NDEBUG
	ARRAY_FOREACH ( i, m_dTerms )
		assert ( m_dTerms[i].m_iHitPos );
#endif

	// if we just bail out without filling the hit,
	// it means that nothing was found
	m_uLastHitPos = 0;

	if ( m_uMaxHitpos <= iMinPos )
		return;

	// skip to necessary min pos
	ARRAY_FOREACH ( i, m_dTerms )
	{
		while ( m_dTerms[i].m_iHitPos && m_dTerms[i].m_iHitPos<iMinPos ) // OPTIMIZE? can remove check for 0
			m_dTerms[i].GetHitlistEntry ();

		if ( !m_dTerms[i].m_iHitPos || m_dTerms[i].m_iHitPos>=m_uMaxHitpos )
			return;
	}

	if ( m_iMaxDistance==-1 )
	{
		//////////////////
		// next plain hit
		//////////////////

		assert ( m_dTerms.GetLength()==1 ); // OPTIMIZE R&D single-atom leaves; should profile
		assert ( m_dTerms[0].m_iHitPos );
		assert ( m_dTerms[0].m_iHitPos>=m_uMinHitpos );

		m_uLastHitPos = m_dTerms[0].m_iHitPos;
		return;
	}

	if ( m_iMaxDistance>0 )
	{
		//////////////////////
		// next proximity hit
		//////////////////////

		for ( ;; )
		{
			// scan until next hit in this document
			DWORD uMin = UINT_MAX;
			DWORD uMax = 0;
			int iMin = -1;

			// calc min/max pos
			ARRAY_FOREACH ( i, m_dTerms )
			{
				assert ( m_dTerms[i].m_iHitPos>=m_uMinHitpos && m_dTerms[i].m_iHitPos<m_uMaxHitpos );
				if ( !( ( 1 << (m_dTerms[i].m_iHitPos>>24) ) & m_uFields ) )
					continue;

				DWORD uPos = m_dTerms[i].m_iHitPos;
				if ( uMin>uPos )
				{
					uMin = uPos;
					iMin = i;
				}
				if ( uMax<uPos )
					uMax = uPos;
			}

			// if proximity is ok, we have a match
			// !COMMIT weighting needs to be redone
			// !COMMIT fix "cat a cat" query vs. "a cat" text; should NOT match!
			assert ( uMax>uMin );
			int iWeight = m_iMaxDistance-uMax+uMin+m_dTerms.GetLength()-1;
			if ( iWeight>0 )
			{
				m_uLastHitPos = uMin;
				return;
			}

			// next one
			// if one hitlist is over, document is over, because *all* words must be within specified proximity
			m_dTerms[iMin].GetHitlistEntry ();
			if ( !m_dTerms[iMin].m_iHitPos || m_dTerms[iMin].m_iHitPos>=m_uMaxHitpos )
				break;
		}

	} else
	{
		/////////////////////////
		// next exact phrase hit
		/////////////////////////

		assert ( m_iMaxDistance==0 );

		DWORD uCandidate = Max ( m_dTerms[0].m_iHitPos, iMinPos ); // next candidate start pos

		int i;
		for ( i=0; i<m_dTerms.GetLength(); i++ )
		{
			const DWORD & uPos = m_dTerms[i].m_iHitPos;
			assert ( uPos>=m_uMinHitpos && uPos<m_uMaxHitpos );

			if ( !( ( 1 << (m_dTerms[i].m_iHitPos>>24) ) & m_uFields ) )
				continue;

			// scan forward until this hit is not too early
			const DWORD uRequiredPos = uCandidate + m_dWords[i].m_iAtomPos;
			while ( uPos < uRequiredPos )
			{
				// if one hitlist is over, document is over, because all words must be in the phrase
				m_dTerms[i].GetHitlistEntry ();
				if ( !uPos || uPos>=m_uMaxHitpos )
				{
					i = -1;
					break;
				}
			}
			if ( i==-1 )
				break;

			// if this his is too late, update candidate and scan again
			if ( uPos > uRequiredPos )
			{
				uCandidate = uPos-i;
				i = -1;
				continue;
			}

			// this hit is a perfect match; continue to next term
			assert ( uPos==uRequiredPos );
		}

		// so do we have a match?
		if ( i==m_dTerms.GetLength() )
		{
			assert ( uCandidate>=m_uMinHitpos && uCandidate<m_uMaxHitpos );
			assert ( uCandidate>=iMinPos );
			m_uLastHitPos = uCandidate;
		}
	}
}


CSphMatch * CSphExtendedEvalAtom::GetNextDoc ( SphDocID_t iMinID, CSphString & sWarning )
{
	if ( !m_pLast )
		return NULL;

	if ( m_pLast->m_iDocID>=iMinID )
	{
		assert ( m_uLastHitPos );
		return m_pLast;
	}

	// OPTIMIZE?
	for ( ;; )
	{
		/////////////////
		// scan doclists
		/////////////////

		ARRAY_FOREACH ( i, m_dTerms ) // OPTIMIZE? PQ?
		{
			// scan doclist until first ok id
			CSphQueryWord & tTerm = m_dTerms[i];
			while ( tTerm.m_tDoc.m_iDocID<iMinID )
			{
				tTerm.GetDoclistEntry ();
				if ( !tTerm.m_tDoc.m_iDocID )
				{
					m_pLast = NULL;
					return NULL;
				}

				// max_query_time
				if ( m_uMaxStamp && ++m_iScannedEntries==1000 )
				{
					if ( sphTimerMsec()>=m_uMaxStamp )
					{
						sWarning = "query time exceeded max_query_time";
						m_pLast = NULL;
						return NULL;
					}
					m_iScannedEntries = 0;
				}
			}

			// check fields
			if (!( tTerm.m_uFields & m_uFields ))
			{
				i = -1;
				iMinID = 1 + tTerm.m_tDoc.m_iDocID;
				continue;
			}

			// adjust and rescan, if necessary
			if ( tTerm.m_tDoc.m_iDocID>iMinID )
			{
				i = -1;
				iMinID = tTerm.m_tDoc.m_iDocID;
				continue;
			}
		}

		// at this point, all terms share the same doc id
		#if PARANOID
		assert ( iMinID );
		ARRAY_FOREACH ( i, m_dTerms )
			assert ( m_dTerms[i].m_tDoc.m_iDocID==iMinID );
		#endif

		// preload hitlists
		// !COMMIT OPTIMIZE 1-word query case
		ARRAY_FOREACH ( i, m_dTerms )
		{
			// do preload
			assert ( !m_dTerms[i].m_iHitPos );
			while ( m_dTerms[i].m_iHitPos<m_uMinHitpos )
			{
				m_dTerms[i].GetHitlistEntry ();

				// broken hitlist protection
				if ( !m_dTerms[i].m_iHitPos )
				{
					if ( m_dTerms[i].m_rdHitlist.GetErrorFlag() )
						sphInternalError ( "%s", m_dTerms[i].m_rdHitlist.GetErrorMessage().cstr() );
					else
						sphInternalError ( "unexpected hitlist end (word=%s, hitlist=%s, docid=" DOCID_FMT ")",
							m_dTerms[i].m_sWord.cstr(), m_dTerms[i].m_rdHitlist.GetFilename().cstr(),
							m_dTerms[i].m_tDoc.m_iDocID );
					return NULL; 
				}
			}

			// according to fields mask, there should be at least one hit, so check it
			assert ( m_dTerms[i].m_iHitPos>=m_uMinHitpos && m_dTerms[i].m_iHitPos<m_uMaxHitpos );
		}		

		// if we just needed to match all words, we're done
		if ( m_iMaxDistance==-1 )
		{
			assert ( m_dTerms.GetLength()==1 ); // OPTIMIZE R&D single-atom leaves; should profile
			assert ( m_dTerms[0].m_iHitPos );

			m_uLastHitPos = m_dTerms[0].m_iHitPos;
			return m_pLast;
		}

		// scan hitlists to enforce proximity and/or exact phrase constraints
		assert ( m_iMaxDistance>=0 );
		GetNextHit ( m_uMinHitpos );

		if ( m_uLastHitPos )
			return m_pLast;

		// proximity constraints were not met; continue scanning for documents
		iMinID++;
	}
}


void CSphExtendedEvalAtom::UpdateLCS ( CSphLCSState & tState )
{
	if ( !m_uLastHitPos )
	{
		tState.m_uNextPos = 0; // no need to zero m_dCurLCS; it'll be auto-zeroed next time now
		return;
	}

	DWORD uField = m_uLastHitPos>>24;

	if ( m_iMaxDistance>0 )
	{
		// proximity atom
		if ( m_uLastHitPos!=tState.m_uNextPos )
		{
			tState.m_dCurLCS[uField] = 0;
			tState.m_uNextPos = m_uLastHitPos;
		}

		// calc head LCS
		int i = 0;
		while ( i<m_dTerms.GetLength() && tState.m_uNextPos==m_dTerms[i].m_iHitPos )
		{
			tState.m_dCurLCS[uField]++;
			tState.m_uNextPos++;
			i++;
		}
		tState.m_dMaxLCS[uField] = Max ( tState.m_dMaxLCS[uField], tState.m_dCurLCS[uField] );

		if ( i!=m_dTerms.GetLength() )
		{
			// !COMMIT calc tail LCS here...
			tState.m_uNextPos = 0;
		} else
		{
			assert ( tState.m_uNextPos==m_dTerms.Last().m_iHitPos+1 );
		}

	} else
	{
		// plain 1-term atom or phrase atom
		assert ( m_iMaxDistance==0 || m_dTerms.GetLength()==1 );

		if ( m_uLastHitPos!=tState.m_uNextPos )
			tState.m_dCurLCS[uField] = 0;

		tState.m_dCurLCS[uField] += m_dTerms.GetLength ();

		tState.m_dMaxLCS[uField] = Max ( tState.m_dMaxLCS[uField], tState.m_dCurLCS[uField] );
		tState.m_uNextPos = m_uLastHitPos + m_dTerms.GetLength ();
	}
}

//////////////////////////////////////////////////////////////////////////////

#define SPH_BM25_MAX_TERMS		256
#define SPH_BM25_K1				1.2f
#define SPH_BM25_SCALE			1000


struct QwordsHash_fn
{
	static inline int Hash ( const CSphString & sKey )
	{
		return sphCRC32 ( (const BYTE *)sKey.cstr() );
	}
};


typedef CSphOrderedHash < CSphQueryWord, CSphString, QwordsHash_fn, 256, 13 > CSphQwordsHash;


class CSphExtendedEvalNode
{
public:
						CSphExtendedEvalNode ( const CSphExtendedQueryNode * pNode, const CSphTermSetup & tSetup );
						~CSphExtendedEvalNode ();

public:
	void				CollectQwords ( CSphQwordsHash & dHash, int & iCount );	///< collects all unique query words into dHash
	CSphMatch *			GetNextMatch ( SphDocID_t iMinID, int iTerms, float * pIDF, int iFields, int * pWeights, CSphString & sWarning );	///< iTF==0 means that no weighting is required

protected:
	CSphMatch *			GetNextDoc ( SphDocID_t iMinID, CSphString & sWarning );
	DWORD				GetNextHitNode ( DWORD iMinPos );
	void				GetLastTF ( DWORD * pTF ) const;
	inline CSphMatch *	GetLastDoc () const { return m_pLast; }
	void				UpdateLCS ( CSphLCSState & tState );

public:
	DWORD				m_uLastHitPos;
	int					m_iLastHitChild;

protected:
	CSphExtendedEvalAtom				m_tAtom;		///< plain node atom
	bool								m_bAny;			///< whether to match any or all children (ie. OR or AND)
	CSphVector<CSphExtendedEvalNode*>	m_dChildren;	///< non-plain node children
	bool								m_bDone;		///< whether this node has any more matches
	CSphMatch *							m_pLast;		///< my last match
};


CSphExtendedEvalNode::CSphExtendedEvalNode ( const CSphExtendedQueryNode * pNode, const CSphTermSetup & tSetup )
	: m_iLastHitChild ( -1 )
	, m_tAtom ( pNode->m_tAtom, tSetup )
	, m_bAny ( pNode->m_bAny )
	, m_bDone ( false )
	, m_pLast ( NULL )
{
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		m_dChildren.Add ( new CSphExtendedEvalNode ( pNode->m_dChildren[i], tSetup ) );

	if ( pNode->IsEmpty() )
		m_bDone = true;
}


CSphExtendedEvalNode::~CSphExtendedEvalNode ()
{
	ARRAY_FOREACH ( i, m_dChildren )
		SafeDelete ( m_dChildren[i] );
}


DWORD CSphExtendedEvalNode::GetNextHitNode ( DWORD uMinPos )
{
	assert ( !m_bDone );

	// plain node
	if ( !m_dChildren.GetLength() )
	{
		assert ( !m_bAny );
		assert ( !m_tAtom.IsEmpty() );

		m_tAtom.GetNextHit ( uMinPos );
		m_uLastHitPos = m_tAtom.m_uLastHitPos;
		return m_uLastHitPos;
	}

	// expression node
	DWORD uChildPos = UINT_MAX;
	int iMinChild = -1;

	if ( m_bAny )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			if ( m_dChildren[i]->m_uLastHitPos && m_dChildren[i]->m_pLast->m_iDocID==m_pLast->m_iDocID )
		{
			CSphExtendedEvalNode * pChild = m_dChildren[i];
			do
			{
				pChild->GetNextHitNode ( uMinPos );
			} while ( pChild->m_uLastHitPos && pChild->m_uLastHitPos<uMinPos );

			if ( pChild->m_uLastHitPos && pChild->m_uLastHitPos<uChildPos )
			{
				uChildPos = pChild->m_uLastHitPos;
				iMinChild = i;
			}
		}

		assert ( iMinChild<0 || m_dChildren[iMinChild]->m_uLastHitPos==uChildPos );

	} else
	{
		ARRAY_FOREACH ( i, m_dChildren )
		{
			CSphExtendedEvalNode * pChild = m_dChildren[i];

			DWORD uPos = pChild->m_uLastHitPos;
			if ( !uPos )
				continue;

			if ( i==m_iLastHitChild )
				uPos = pChild->GetNextHitNode ( uMinPos );

			if ( uPos && uPos<uChildPos )
			{
				uChildPos = uPos;
				iMinChild = i;
			}
		}
	}

	if ( iMinChild<0 )
		uChildPos = 0;
	m_uLastHitPos = uChildPos;
	m_iLastHitChild = iMinChild;
	return uChildPos;
}


CSphMatch * CSphExtendedEvalNode::GetNextDoc ( SphDocID_t iMinID, CSphString & sWarning )
{
	if ( m_bDone )
		return NULL;

	////////////////////
	// match plain node
	////////////////////

	if ( !m_dChildren.GetLength() )
	{
		assert ( !m_bAny );
		assert ( !m_tAtom.IsEmpty() );

		m_pLast = m_tAtom.GetNextDoc ( iMinID, sWarning );
		m_uLastHitPos = m_tAtom.m_uLastHitPos;
		if ( !m_pLast )
			m_bDone = true;
		return m_pLast;
	}

	/////////////////////////
	// match expression node
	/////////////////////////

	CSphMatch * pMatch = NULL;

	if ( m_bAny )
	{
		////////////////////////
		// match any child node
		////////////////////////

		ARRAY_FOREACH ( i, m_dChildren )
		{
			// get next candidate
			CSphMatch * pCur = m_dChildren[i]->GetNextDoc ( iMinID, sWarning );
			assert ( pCur==m_dChildren[i]->GetLastDoc() );

			if ( !pCur )
			{
				SafeDelete ( m_dChildren[i] );
				m_dChildren.RemoveFast ( i-- );
				if ( !m_dChildren.GetLength() )
				{
					m_bDone = true;
					return false;
				}
				continue;
			}

			// if it's better than current, make it current match
			if ( !pMatch || pCur->m_iDocID < pMatch->m_iDocID )
			{
				pMatch = pCur;
				m_uLastHitPos = m_dChildren[i]->m_uLastHitPos;
				m_iLastHitChild = i;
			}
		}

	} else
	{
		////////////////////////////
		// match all children nodes
		////////////////////////////

		// find a match
		ARRAY_FOREACH ( i, m_dChildren )
		{
			CSphExtendedEvalNode * pChild = m_dChildren[i];
			pMatch = pChild->GetNextDoc ( iMinID, sWarning );
			if ( !pMatch )
			{
				m_bDone = true;
				m_pLast = NULL;
				return NULL;
			}

			if ( pMatch->m_iDocID > iMinID )
				i = -1;

			iMinID = pMatch->m_iDocID;
		}

		assert ( pMatch );

		m_uLastHitPos = UINT_MAX;
		ARRAY_FOREACH ( i, m_dChildren )
		{
			DWORD uPos = m_dChildren[i]->m_uLastHitPos;
			assert ( uPos );

			if ( uPos<m_uLastHitPos )
			{
				m_uLastHitPos = uPos;
				m_iLastHitChild = i;
			}
		}
	}

	m_pLast = pMatch;
	return pMatch;
}


void CSphExtendedEvalNode::UpdateLCS ( CSphLCSState & tState )
{
	if ( m_dChildren.GetLength() )
	{
		if ( !m_bAny )
		{
			ARRAY_FOREACH ( i, m_dChildren )
				if ( m_dChildren[i]->m_uLastHitPos )
					m_dChildren[i]->UpdateLCS ( tState );
		} else
		{
			assert ( m_dChildren[m_iLastHitChild]->m_uLastHitPos==m_uLastHitPos );
			m_dChildren[m_iLastHitChild]->UpdateLCS ( tState );
		}
	} else
	{
		m_tAtom.UpdateLCS ( tState );
	}
}


CSphMatch * CSphExtendedEvalNode::GetNextMatch ( SphDocID_t iMinID, int iTerms, float * pIDF,
	int iFields, int * pWeights, CSphString & sWarning )
{
	CSphMatch * pMatch = GetNextDoc ( iMinID, sWarning );
	m_pLast = pMatch;

	if ( !pMatch || !iTerms ) // if there's no match or no need to rank, we're done
		return pMatch;

	////////////////////
	// weight the match
	////////////////////

#ifndef NDEBUG
	// check that every actually matching child has its hitlist ready
	assert ( m_uLastHitPos );
	ARRAY_FOREACH ( i, m_dChildren )
	{
		assert ( m_dChildren[i]->GetLastDoc() );
		if ( m_bAny )
		{
			assert ( m_dChildren[i]->GetLastDoc()->m_iDocID>=pMatch->m_iDocID );
			if ( m_dChildren[i]->GetLastDoc()->m_iDocID==pMatch->m_iDocID )
				assert ( m_dChildren[i]->m_uLastHitPos );
		} else
		{
			assert ( m_dChildren[i]->GetLastDoc()->m_iDocID==pMatch->m_iDocID );
			assert ( m_dChildren[i]->m_uLastHitPos );
		}
	}
#endif

	// calc LCS rank
	CSphLCSState tState ( iFields );
	do 
	{
		tState.CleanCurrent ( iFields );
		UpdateLCS ( tState );
		GetNextHitNode ( 1+m_uLastHitPos );
	} while ( m_uLastHitPos );

	int iMaxLCS = 0;
	for ( int i=0; i<iFields; i++ )
		iMaxLCS += tState.m_dMaxLCS[i] * pWeights[i];

	// calc simplified BM25 rank
	DWORD uTF [ SPH_BM25_MAX_TERMS ]; // FIXME? maybe a dynamic array
	assert ( iTerms<SPH_BM25_MAX_TERMS );

	for ( int i=0; i<iTerms; i++ ) // FIXME? could be avoided if GetLastTF() returns a mask..
		uTF[i] = 0;
	GetLastTF ( uTF );

	float fBM25 = 0.0f;
	for ( int i=0; i<iTerms; i++ )
		fBM25 += float(uTF[i]) / float(uTF[i]+SPH_BM25_K1) * pIDF[i]; // TF*IDF

	// calc combined rank
	// !COMMIT must account for field weights everywhere, document length in BM25, etc
	int iBM25 = int ( ( fBM25 + 0.5f )*SPH_BM25_SCALE );
	assert ( iBM25>=0 && iBM25<SPH_BM25_SCALE );

	pMatch->m_iWeight = iMaxLCS*SPH_BM25_SCALE + iBM25;
	return pMatch;
}


void CSphExtendedEvalNode::GetLastTF ( DWORD * pTF ) const
{
	if ( m_bDone )
		return;

	assert ( m_pLast );

	if ( !m_dChildren.GetLength() )
	{
		m_tAtom.GetLastTF ( pTF );
		return;
	}

	SphDocID_t uID = m_pLast->m_iDocID;
	if ( m_bAny )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			if ( m_dChildren[i]->GetLastDoc()->m_iDocID==uID )
				m_dChildren[i]->GetLastTF ( pTF );
	} else
	{
		ARRAY_FOREACH ( i, m_dChildren )
		{
			assert ( m_dChildren[i]->GetLastDoc()->m_iDocID==uID );
			m_dChildren[i]->GetLastTF ( pTF );
		}
	}
}


void CSphExtendedEvalNode::CollectQwords ( CSphQwordsHash & dHash, int & iCount )
{
	if ( m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			m_dChildren[i]->CollectQwords ( dHash, iCount );
	} else
	{
		ARRAY_FOREACH ( i, m_tAtom.m_dTerms )
		{
			bool bAdded = dHash.Add ( m_tAtom.m_dTerms[i], m_tAtom.m_dTerms[i].m_sWord );

			CSphQueryWord * pWord = dHash ( m_tAtom.m_dTerms[i].m_sWord );
			assert ( pWord );

			if ( bAdded )
				pWord->m_iQueryPos = iCount++;

			m_tAtom.m_dTerms[i].m_iQueryPos = pWord->m_iQueryPos;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::MatchExtendedV1 ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup )
{
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;

	assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );

	//////////////////////////
	// match in extended mode
	//////////////////////////

	// parse query
	CSphExtendedQuery tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), tTermSetup.m_pTokenizer,
		&m_tSchema, tTermSetup.m_pDict ) )
	{
		m_sLastError = tParsed.m_sParseError;
		return false;
	}

	pResult->m_sWarning = tParsed.m_sParseWarning;

	CheckExtendedQuery ( tParsed.m_pAccept, pResult );
	CheckExtendedQuery ( tParsed.m_pReject, pResult );

	CSphExtendedEvalNode tAccept ( tParsed.m_pAccept, tTermSetup );
	CSphExtendedEvalNode tReject ( tParsed.m_pReject, tTermSetup );

	////////////////////
	// build word stats
	////////////////////

	CSphQwordsHash hQwords;
	int iQwords = 0;
	tAccept.CollectQwords ( hQwords, iQwords );

	if ( iQwords >= SPH_BM25_MAX_TERMS )
	{
		m_sLastError.SetSprintf ( "too many keywords (keywords=%d, max=%d)", iQwords, SPH_BM25_MAX_TERMS );
		return false;
	}

	int iIDF = 0;
	float dIDF [ SPH_BM25_MAX_TERMS ];

	pResult->m_iNumWords = 0;

	hQwords.IterateStart ();
	while ( hQwords.IterateNext() )
	{
		CSphQueryWord & tWord = hQwords.IterateGet ();

		// build IDF
		float fIDF = 0.0f;
		if ( tWord.m_iDocs )
		{
			float fLogTotal = logf ( float(1+m_tStats.m_iTotalDocuments) );
			fIDF = logf ( float(m_tStats.m_iTotalDocuments-tWord.m_iDocs+1)/float(tWord.m_iDocs) )
				/ ( 2*iQwords*fLogTotal );
		}
		dIDF[iIDF++] = fIDF;

		// update word stats
		if ( pResult->m_iNumWords<SPH_MAX_QUERY_WORDS )
		{
			CSphQueryResult::WordStat_t & tStats = pResult->m_tWordStats [ pResult->m_iNumWords++ ];
			if ( tStats.m_sWord.cstr() )
			{
				assert ( tStats.m_sWord==tWord.m_sWord );
				tStats.m_iDocs += tWord.m_iDocs;
				tStats.m_iHits += tWord.m_iHits;
			} else
			{
				tStats.m_sWord = tWord.m_sWord;
				tStats.m_iDocs = tWord.m_iDocs;
				tStats.m_iHits = tWord.m_iHits;
			}
		}
	}

	//////////////////////////
	// do that matching thing
	//////////////////////////

	// randomizing trick. setting iQwords to 0 prevents weighting
	if ( bRandomize )
		iQwords = 0;

	SphDocID_t iMinID = 1;
	for ( ;; )
	{
		CSphMatch * pAccept = tAccept.GetNextMatch ( iMinID, iQwords, dIDF, m_iWeights, m_dWeights, pResult->m_sWarning );
		if ( !pAccept )
			break;
		iMinID = 1 + pAccept->m_iDocID;

		CSphMatch * pReject = tReject.GetNextMatch ( pAccept->m_iDocID, 0, NULL, 0, NULL, pResult->m_sWarning );
		if ( pReject && pReject->m_iDocID==pAccept->m_iDocID )
			continue;

		// early reject by group id, doc id or timestamp
		if ( m_bEarlyLookup )
			LookupDocinfo ( *pAccept );

		if ( sphMatchEarlyReject ( *pAccept, pQuery, m_pMva ) )
			continue;

		// submit
		SPH_SUBMIT_MATCH ( *pAccept );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V2
//////////////////////////////////////////////////////////////////////////

/// match in the stream
struct ExtDoc_t
{
	SphDocID_t	m_uDocid;
	SphOffset_t	m_uHitlistOffset;
	DWORD		m_uFields;
	float		m_fTFIDF;
};


/// hit in the stream
struct ExtHit_t
{
	SphDocID_t	m_uDocid;
	DWORD		m_uHitpos;
	DWORD		m_uQuerypos;
	DWORD		m_uSpanlen;
};


/// word in the query
struct ExtQword_t
{
	CSphString	m_sWord;	///< word
	int			m_iDocs;	///< matching documents
	int			m_iHits;	///< matching hits
	float		m_fIDF;		///< IDF value
};


/// query words set
typedef CSphOrderedHash < ExtQword_t, CSphString, QwordsHash_fn, 256, 13 > ExtQwordsHash_t;


/// generic match streamer
class ExtNode_i
{
public:
								ExtNode_i ();
	virtual						~ExtNode_i () {}
	static ExtNode_i *			Create ( const CSphExtendedQueryNode * pNode, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning );

	virtual const ExtDoc_t *	GetDocsChunk () = 0;
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs ) = 0;

	virtual void				GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;

public:
	static const int			MAX_DOCS = 512;
	static const int			MAX_HITS = 512;

	ExtDoc_t					m_dDocs[MAX_DOCS];
	ExtHit_t					m_dHits[MAX_HITS];
};


/// single keyword streamer
class ExtTerm_c : public ExtNode_i
{
public:
								ExtTerm_c ( const CSphString & sTerm, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning );
	virtual const ExtDoc_t *	GetDocsChunk ();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

protected:
	CSphQueryWord				m_tQword;
	DWORD						m_uQuerypos;
	ExtDoc_t *					m_pHitDoc;		///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	SphDocID_t					m_uHitsOverFor;	///< there are no more hits for matches block starting with this ID
	DWORD						m_uFields;		///< accepted fields mask
	float						m_fIDF;			///< IDF for this term (might be 0.0f for non-1st occurences in query)
	DWORD						m_uMaxStamp;	///< work until this timestamp 
	CSphString *				m_pWarning;
};


/// multi-node binary-operation streamer traits
class ExtTwofer_c : public ExtNode_i
{
public:
								ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond );
								~ExtTwofer_c ();
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

protected:
	ExtNode_i *					m_pChildren[2];
	const ExtDoc_t *			m_pCurDoc[2];
	const ExtHit_t *			m_pCurHit[2];
	SphDocID_t					m_uMatchedDocid;
};


/// A-and-B streamer
class ExtAnd_c : public ExtTwofer_c
{
public:
								ExtAnd_c ( ExtNode_i * pFirst, ExtNode_i * pSecond ) : ExtTwofer_c ( pFirst, pSecond ) {}
	virtual const ExtDoc_t *	GetDocsChunk ();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
};


/// A-or-B streamer
class ExtOr_c : public ExtTwofer_c
{
public:
								ExtOr_c ( ExtNode_i * pFirst, ExtNode_i * pSecond ) : ExtTwofer_c ( pFirst, pSecond ) {}
	virtual const ExtDoc_t *	GetDocsChunk ();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
};


/// A-and-not-B streamer
class ExtAndNot_c : public ExtTwofer_c
{
public:
								ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond );
	virtual const ExtDoc_t *	GetDocsChunk ();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );

protected:
	bool						m_bPassthrough;
};


/// exact phrase streamer
class ExtPhrase_c : public ExtNode_i
{
public:
								ExtPhrase_c ( const CSphExtendedQueryNode * pNode, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning );
								~ExtPhrase_c ();
	virtual const ExtDoc_t *	GetDocsChunk ();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

protected:
	ExtNode_i *					m_pNode;				///< my and-node for all the terms
	DWORD						m_uFields;				///< what fields is the search restricted to
	SphDocID_t					m_uLastDocID;			///< last emitted hit
	SphDocID_t					m_uExpID;
	DWORD						m_uExpPos;
	DWORD						m_uExpQpos;
	DWORD						m_uMinQpos;				///< min qpos through my keywords
	DWORD						m_uMaxQpos;				///< max qpos through my keywords
	CSphVector<int>				m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
	ExtHit_t					m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	SphDocID_t					m_uMatchedDocid;
	SphDocID_t					m_uHitsOverFor;			///< there are no more hits for matches block starting with this ID
};


/// proximity streamer
class ExtProximity_c : public ExtPhrase_c
{
public:
								ExtProximity_c ( const CSphExtendedQueryNode * pNode, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning );
	virtual const ExtDoc_t *	GetDocsChunk ();

protected:
	int							m_iMaxDistance;
	int							m_iNumWords;
};


/// ranker interface
/// ranker folds incoming hitstream into simple match chunks, and computes relevance rank
class ExtRanker_c
{
public:
								ExtRanker_c ( const CSphExtendedQueryNode * pAccept, const CSphExtendedQueryNode * pReject, const CSphTermSetup & tSetup, CSphString * pWarning );
	virtual						~ExtRanker_c () { SafeDelete ( m_pRoot ); }

	virtual int					GetMatches ( int iFields, const int * pWeights ) = 0;

	void						GetQwords ( ExtQwordsHash_t & hQwords )				{ if ( m_pRoot ) m_pRoot->GetQwords ( hQwords ); }
	void						SetQwordsIDF ( const ExtQwordsHash_t & hQwords )	{ if ( m_pRoot ) m_pRoot->SetQwordsIDF ( hQwords ); }

public:
	CSphMatch					m_dMatches[ExtNode_i::MAX_DOCS];

protected:
	ExtNode_i *					m_pRoot;
	const ExtDoc_t *			m_pDoclist;
	const ExtHit_t *			m_pHitlist;
};


#define DECLARE_RANKER(_name) \
	class ExtRanker_##_name##_c : public ExtRanker_c \
	{ \
	public: \
		ExtRanker_##_name##_c ( const CSphExtendedQueryNode * pAccept, const CSphExtendedQueryNode * pReject, const CSphTermSetup & tSetup, CSphString * pWarning ) \
			: ExtRanker_c ( pAccept, pReject, tSetup, pWarning ) \
		{} \
	\
		virtual int GetMatches ( int iFields, const int * pWeights ); \
	};

DECLARE_RANKER ( ProximityBM25 )
DECLARE_RANKER ( BM25 )
DECLARE_RANKER ( None )
DECLARE_RANKER ( Wordcount )

//////////////////////////////////////////////////////////////////////////

ExtNode_i::ExtNode_i ()
{
	m_dDocs[0].m_uDocid = DOCID_MAX;
	m_dHits[0].m_uDocid = DOCID_MAX;
}

ExtNode_i * ExtNode_i::Create ( const CSphExtendedQueryNode * pNode, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning )
{
	if ( pNode->IsPlain() )
	{
		const CSphExtendedQueryAtom & tAtom = pNode->m_tAtom;
		const int iWords = tAtom.m_dWords.GetLength();
		
		if ( iWords<=0 )
			return NULL; // empty reject node

		DWORD uFields = tAtom.m_uFields;

		if ( iWords==1 )
			return new ExtTerm_c ( tAtom.m_dWords[0].m_sWord, uFields, tSetup, uQuerypos, pWarning );

		assert ( tAtom.m_iMaxDistance>=0 );
		if ( tAtom.m_iMaxDistance==0 )
			return new ExtPhrase_c ( pNode, uFields, tSetup, uQuerypos, pWarning );
		else
			return new ExtProximity_c ( pNode, uFields, tSetup, uQuerypos, pWarning );

	} else
	{
		int iChildren = pNode->m_dChildren.GetLength ();
		assert ( iChildren>0 );

		ExtNode_i * pCur = ExtNode_i::Create ( pNode->m_dChildren[0], tSetup, uQuerypos, pWarning );
		for ( int i=1; i<iChildren; i++ )
		{
			if ( pNode->m_bAny )
				pCur = new ExtOr_c ( pCur, ExtNode_i::Create ( pNode->m_dChildren[i], tSetup, uQuerypos, pWarning ) );
			else
				pCur = new ExtAnd_c ( pCur, ExtNode_i::Create ( pNode->m_dChildren[i], tSetup, uQuerypos+i, pWarning ) );
		}
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

ExtTerm_c::ExtTerm_c ( const CSphString & sTerm, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning )
	: m_pWarning ( pWarning )
{
	CSphString sBuf ( sTerm );
	m_tQword.m_sWord = sBuf;
	m_tQword.m_iWordID = tSetup.m_pDict->GetWordID ( (BYTE*)sBuf.cstr() );
	m_tQword.SetupAttrs ( tSetup );
	tSetup.m_pIndex->SetupQueryWord ( m_tQword, tSetup );

	m_uQuerypos = uQuerypos;
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_uFields = uFields;
	m_uMaxStamp = tSetup.m_uMaxStamp;
}


const ExtDoc_t * ExtTerm_c::GetDocsChunk ()
{
	if ( !m_tQword.m_iDocs )
		return NULL;

	// max_query_time
	if ( m_uMaxStamp>0 && sphTimerMsec()>=m_uMaxStamp )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	int iDoc = 0;
	while ( iDoc<MAX_DOCS-1 )
	{
		m_tQword.GetDoclistEntryOnly();
		if ( !m_tQword.m_tDoc.m_iDocID )
		{
			m_tQword.m_iDocs = 0;
			break;
		}
		if (!( m_tQword.m_uFields & m_uFields ))
			continue;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_uDocid = m_tQword.m_tDoc.m_iDocID;
		tDoc.m_uHitlistOffset = m_tQword.m_iHitlistPos;
		tDoc.m_uFields = m_tQword.m_uFields; // OPTIMIZE: only needed for phrase node
		tDoc.m_fTFIDF = float(m_tQword.m_uMatchHits) / float(m_tQword.m_uMatchHits+SPH_BM25_K1) * m_fIDF;
	}

	m_pHitDoc = NULL;

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

const ExtHit_t * ExtTerm_c::GetHitsChunk ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return NULL;
	SphDocID_t uFirstMatch = pMatched->m_uDocid;

	// aim to the right document
	ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( uFirstMatch==m_uHitsOverFor )
			return NULL;

		// find match
		pDoc = m_dDocs;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			if ( pDoc->m_uDocid==DOCID_MAX ) return NULL; // matched docs block is over for me, gimme another one

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX ) return NULL; // matched doc block did not yet begin for me, gimme another one

		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		// setup hitlist reader
		m_tQword.m_rdHitlist.SeekTo ( pDoc->m_uHitlistOffset, READ_NO_SIZE_HINT );
		m_tQword.m_iHitPos = 0;
		#ifndef NDEBUG
		m_tQword.m_bHitlistOver = false;
		#endif
	}

	// hit emission
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// get next hit
		m_tQword.GetHitlistEntry ();
		if ( !m_tQword.m_iHitPos )
		{
			// no more hits; get next acceptable document
			pDoc++;
			do
			{
				while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
				if ( pDoc->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched docs block is over for me, gimme another one

				while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
				if ( pMatched->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched doc block did not yet begin for me, gimme another one

			} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

			if ( !pDoc )
				break;
			assert ( pDoc->m_uDocid==pMatched->m_uDocid );

			// setup hitlist reader
			m_tQword.m_rdHitlist.SeekTo ( pDoc->m_uHitlistOffset, READ_NO_SIZE_HINT );
			m_tQword.m_iHitPos = 0;
			#ifndef NDEBUG
			m_tQword.m_bHitlistOver = false;
			#endif

			continue;
		}

		if (!( m_uFields & ( 1UL<<(m_tQword.m_iHitPos>>24) ) ))
			continue;

		ExtHit_t & tHit = m_dHits[iHit++];
		tHit.m_uDocid = pDoc->m_uDocid;
		tHit.m_uHitpos = m_tQword.m_iHitPos;
		tHit.m_uQuerypos = m_uQuerypos;
		tHit.m_uSpanlen = 1;
	}

	m_pHitDoc = pDoc;
	if ( iHit==0 || iHit<MAX_HITS-1 )
		m_uHitsOverFor = uFirstMatch;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;

	return ( iHit!=0 ) ? m_dHits : NULL;
}

void ExtTerm_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_fIDF = 0.0f;
	if ( hQwords.Exists ( m_tQword.m_sWord ) )
		return;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_tQword.m_sWord;
	tInfo.m_iDocs = m_tQword.m_iDocs;
	tInfo.m_iHits = m_tQword.m_iHits;
	hQwords.Add (  tInfo, m_tQword.m_sWord );
}

void ExtTerm_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	if ( m_fIDF<0.0f )
	{
		assert ( hQwords(m_tQword.m_sWord) );
		m_fIDF = hQwords(m_tQword.m_sWord)->m_fIDF;
	}
}

//////////////////////////////////////////////////////////////////////////

ExtTwofer_c::ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
{
	m_pChildren[0] = pFirst;
	m_pChildren[1] = pSecond;
	m_pCurHit[0] = NULL;
	m_pCurHit[1] = NULL;
	m_pCurDoc[0] = NULL;
	m_pCurDoc[1] = NULL;
	m_uMatchedDocid = 0;
}

ExtTwofer_c::~ExtTwofer_c ()
{
	SafeDelete ( m_pChildren[0] );
	SafeDelete ( m_pChildren[1] );
}

void ExtTwofer_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_pChildren[0]->GetQwords ( hQwords );
	m_pChildren[1]->GetQwords ( hQwords );
}

void ExtTwofer_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pChildren[0]->SetQwordsIDF ( hQwords );
	m_pChildren[1]->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtAnd_c::GetDocsChunk ()
{
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	int iDoc = 0;
	for ( ;; )
	{
		// if any of the pointers is empty, *and* there is no data yet, process next child chunk
		// if there is data, we can't advance, because child hitlist offsets would be lost
		if ( !pCur0 || !pCur1 )
		{
			if ( iDoc!=0 )
				break;

			if ( !pCur0 ) pCur0 = m_pChildren[0]->GetDocsChunk ();
			if ( !pCur1 ) pCur1 = m_pChildren[1]->GetDocsChunk ();
			if ( !pCur0 || !pCur1 )
			{
				m_pCurDoc[0] = NULL;
				m_pCurDoc[1] = NULL;
				return NULL;
			}
		}

		// find common matches
		assert ( pCur0 && pCur1 );
		while ( iDoc<MAX_DOCS-1 )
		{
			// find next matching docid
			while ( pCur0->m_uDocid < pCur1->m_uDocid ) pCur0++;
			if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0=NULL; break; }

			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1=NULL; break; }

			if ( pCur0->m_uDocid!=pCur1->m_uDocid ) continue;

			// emit it
			ExtDoc_t & tDoc = m_dDocs[iDoc++];	
			tDoc.m_uDocid = pCur0->m_uDocid;
			tDoc.m_uFields = pCur0->m_uFields | pCur1->m_uFields;
			tDoc.m_uHitlistOffset = -1;
			tDoc.m_fTFIDF = pCur0->m_fTFIDF + pCur1->m_fTFIDF;

			// skip it
			pCur0++; if ( pCur0->m_uDocid==DOCID_MAX ) pCur0 = NULL;
			pCur1++; if ( pCur1->m_uDocid==DOCID_MAX ) pCur1 = NULL;
			if ( !pCur0 || !pCur1 ) break;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

const ExtHit_t * ExtAnd_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	const ExtHit_t * pCur0 = m_pCurHit[0];
	const ExtHit_t * pCur1 = m_pCurHit[1];

	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// emit hits, while possible
		if ( m_uMatchedDocid!=0
			&& m_uMatchedDocid!=DOCID_MAX
			&& ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) ) )
		{
			// merge, while possible
			if ( pCur0 && pCur1 && pCur0->m_uDocid==m_uMatchedDocid && pCur1->m_uDocid==m_uMatchedDocid )
				while ( iHit<MAX_HITS-1 )
			{
				if ( pCur0->m_uHitpos < pCur1->m_uHitpos )
				{
					m_dHits[iHit++] = *pCur0++;
					if ( pCur0->m_uDocid!=m_uMatchedDocid )
						break;
				} else
				{
					m_dHits[iHit++] = *pCur1++;
					if ( pCur1->m_uDocid!=m_uMatchedDocid )
						break;
				}
			}

			// copy tail, while possible
			if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid )
			{
				while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur0++;
			} else
			{
				assert ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid );
				while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur1++;
			}
		}

		// move on
		if ( ( pCur0 && pCur0->m_uDocid!=m_uMatchedDocid ) && ( pCur1 && pCur1->m_uDocid!=m_uMatchedDocid ) )
			m_uMatchedDocid = 0;

		// warmup if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
		if ( !pCur0 ) break;

		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );
		if ( !pCur1 ) break;

		assert ( pCur0 && pCur1 );

		// find matching doc
		while ( !m_uMatchedDocid )
		{
			while ( pCur0->m_uDocid < pCur1->m_uDocid ) pCur0++;
			if ( pCur0->m_uDocid==DOCID_MAX ) break;

			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) break;

			if ( pCur0->m_uDocid==pCur1->m_uDocid ) m_uMatchedDocid = pCur0->m_uDocid;
		}
	}

	m_pCurHit[0] = pCur0;
	m_pCurHit[1] = pCur1;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;

	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtOr_c::GetDocsChunk ()
{
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	DWORD uTouched = 0;
	int iDoc = 0;
	while ( iDoc<MAX_DOCS-1 )
	{
		// if any of the pointers is empty, and not touched yet, advance
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		{
			if ( uTouched&1 ) break; // it was touched, so we can't advance, because child hitlist offsets would be lost
			pCur0 = m_pChildren[0]->GetDocsChunk ();
		}
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
		{
			if ( uTouched&2 ) break; // it was touched, so we can't advance, because child hitlist offsets would be lost
			pCur1 = m_pChildren[1]->GetDocsChunk ();
		}

		// check if we're over
		if ( !pCur0 && !pCur1 ) break;

		// merge lists while we can, copy tail while if we can not
		if ( pCur0 && pCur1 )
		{
			// merge lists if we have both of them
			while ( iDoc<MAX_DOCS-1 )
			{			
				// copy min docids from 1st child
				while ( pCur0->m_uDocid < pCur1->m_uDocid && iDoc<MAX_DOCS-1 )
				{
					m_dDocs[iDoc++] = *pCur0++;
					uTouched |= 1;
				}
				if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0 = NULL; break; }

				// copy min docids from 2nd child
				while ( pCur1->m_uDocid < pCur0->m_uDocid && iDoc<MAX_DOCS-1 )
				{
					m_dDocs[iDoc++] = *pCur1++;
					uTouched |= 2;
				}
				if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1 = NULL; break; }

				// copy min docids from both children
				assert ( pCur0->m_uDocid && pCur0->m_uDocid!=DOCID_MAX );
				assert ( pCur1->m_uDocid && pCur1->m_uDocid!=DOCID_MAX );

				while ( pCur0->m_uDocid==pCur1->m_uDocid && pCur0->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
				{
					m_dDocs[iDoc] = *pCur0;
					m_dDocs[iDoc].m_uFields = pCur0->m_uFields | pCur1->m_uFields;
					m_dDocs[iDoc].m_fTFIDF = pCur0->m_fTFIDF + pCur1->m_fTFIDF;
					iDoc++;
					pCur0++;
					pCur1++;
					uTouched |= 3;
				}
				if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0 = NULL; break; }
				if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1 = NULL; break; }
			}
		} else
		{
			// copy tail if we don't have both lists
			const ExtDoc_t * pList = pCur0 ? pCur0 : pCur1;
			if ( pList->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
			{
				while ( pList->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
					m_dDocs[iDoc++] = *pList++;
				uTouched |= pCur0 ? 1 : 2;
			}

			if ( pList->m_uDocid==DOCID_MAX ) pList = NULL;
			if ( pCur0 ) pCur0 = pList; else pCur1 = pList;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

const ExtHit_t * ExtOr_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	const ExtHit_t * pCur0 = m_pCurHit[0];
	const ExtHit_t * pCur1 = m_pCurHit[1];

	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// emit hits, while possible
		if ( m_uMatchedDocid!=0
			&& m_uMatchedDocid!=DOCID_MAX
			&& ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) ) )
		{
			// merge, while possible
			if ( pCur0 && pCur1 && pCur0->m_uDocid==m_uMatchedDocid && pCur1->m_uDocid==m_uMatchedDocid )
				while ( iHit<MAX_HITS-1 )
			{
				if ( pCur0->m_uHitpos < pCur1->m_uHitpos )
				{
					m_dHits[iHit++] = *pCur0++;
					if ( pCur0->m_uDocid!=m_uMatchedDocid )
						break;
				} else
				{
					m_dHits[iHit++] = *pCur1++;
					if ( pCur1->m_uDocid!=m_uMatchedDocid )
						break;
				}
			}

			// copy tail, while possible
			if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid )
			{
				while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur0++;
			} else
			{
				assert ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid );
				while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur1++;
			}
		}

		// move on
		if ( ( pCur0 && pCur0->m_uDocid!=m_uMatchedDocid ) && ( pCur1 && pCur1->m_uDocid!=m_uMatchedDocid ) )
			m_uMatchedDocid = 0;

		// warmup if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );

		if ( !pCur0 && !pCur1 ) break;
		m_uMatchedDocid = ( pCur0 && pCur1 )
			? Min ( pCur0->m_uDocid, pCur1->m_uDocid )
			: ( pCur0 ? pCur0->m_uDocid : pCur1->m_uDocid );
	}

	m_pCurHit[0] = pCur0;
	m_pCurHit[1] = pCur1;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtAndNot_c::ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
	: ExtTwofer_c ( pFirst, pSecond )
	, m_bPassthrough ( false )
{
}

const ExtDoc_t * ExtAndNot_c::GetDocsChunk ()
{
	// if reject-list is over, simply pass through to accept-list
	if ( m_bPassthrough )
		return m_pChildren[0]->GetDocsChunk ();

	// otherwise, do some removals
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	int iDoc = 0;
	while ( iDoc<MAX_DOCS-1 )
	{
		// pull more docs from accept, if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		{
			// there were matches; we can not pull more because that'd fuckup hitlists
			if ( iDoc )
				break;

			// no matches so far; go pull
			pCur0 = m_pChildren[0]->GetDocsChunk();
			if ( !pCur0 )
				break;
		}

		// pull more docs from reject, if nedeed
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
			pCur1 = m_pChildren[1]->GetDocsChunk();

		// if there's nothing to filter against, simply copy leftovers
		if ( !pCur1 )
		{
			assert ( pCur0 );
			while ( pCur0->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
				m_dDocs[iDoc++] = *pCur0++;

			if ( pCur0->m_uDocid==DOCID_MAX )
				m_bPassthrough = true;

			break;
		}

		// perform filtering
		assert ( pCur0 );
		assert ( pCur1 );
		for ( ;; )
		{
			assert ( iDoc<MAX_DOCS-1 );
			assert ( pCur0->m_uDocid!=DOCID_MAX );
			assert ( pCur1->m_uDocid!=DOCID_MAX );

			// copy accepted until min rejected id
			while ( pCur0->m_uDocid < pCur1->m_uDocid && iDoc<MAX_DOCS-1 ) m_dDocs[iDoc++] = *pCur0++;
			if ( pCur0->m_uDocid==DOCID_MAX || iDoc==MAX_DOCS-1 ) break;

			// skip rejected until min accepted id
			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) break;

			// skip both while ids match
			while ( pCur0->m_uDocid==pCur1->m_uDocid && pCur0->m_uDocid!=DOCID_MAX )
			{
				pCur0++;
				pCur1++;
			}
			if ( pCur0->m_uDocid==DOCID_MAX || pCur1->m_uDocid==DOCID_MAX ) break;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

const ExtHit_t * ExtAndNot_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	return m_pChildren[0]->GetHitsChunk ( pDocs );
};

//////////////////////////////////////////////////////////////////////////

ExtPhrase_c::ExtPhrase_c ( const CSphExtendedQueryNode * pNode, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning )
	: m_uFields ( uFields )
	, m_uLastDocID ( 0 )
	, m_uExpID ( 0 )
	, m_uExpPos ( 0 )
	, m_uExpQpos ( 0 )
	, m_uMatchedDocid ( 0 )
	, m_uHitsOverFor ( 0 )
{
	assert ( pNode );
	assert ( pNode->IsPlain() );
	assert ( pNode->m_tAtom.m_iMaxDistance>=0 );

	m_uFields = uFields;

	const CSphVector<CSphExtendedQueryAtomWord> & dWords = pNode->m_tAtom.m_dWords;
	int iWords = dWords.GetLength();
	assert ( iWords>1 );

	m_uMinQpos = uQuerypos+dWords[0].m_iAtomPos;
	m_uMaxQpos = uQuerypos+dWords.Last().m_iAtomPos;

	m_dQposDelta.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;

	ExtNode_i * pCur = new ExtTerm_c ( dWords[0].m_sWord, uFields, tSetup, uQuerypos+dWords[0].m_iAtomPos, pWarning );
	for ( int i=1; i<iWords; i++ )
	{
		pCur = new ExtAnd_c ( pCur, new ExtTerm_c ( dWords[i].m_sWord, uFields, tSetup, uQuerypos+dWords[i].m_iAtomPos, pWarning ) );
		m_dQposDelta [ dWords[i-1].m_iAtomPos - dWords[0].m_iAtomPos ] = dWords[i].m_iAtomPos - dWords[i-1].m_iAtomPos;
	}
	m_pNode = pCur;
}

ExtPhrase_c::~ExtPhrase_c ()
{
	SafeDelete ( m_pNode );
}

const ExtDoc_t * ExtPhrase_c::GetDocsChunk ()
{
	const ExtDoc_t * pDocs = NULL;
	const ExtHit_t * pHits = NULL;

	if ( !pDocs ) pDocs = m_pNode->GetDocsChunk ();
	if ( !pDocs ) return NULL;

	if ( !pHits ) pHits = m_pNode->GetHitsChunk ( pDocs );
	assert ( pHits );

	const ExtHit_t * pHit = pHits;
	const ExtDoc_t * pDoc = pDocs;

	int iDoc = 0;
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		if ( pHit->m_uDocid==DOCID_MAX )
		{
			m_uExpID = m_uExpPos = m_uExpQpos = 0;

			pHits = m_pNode->GetHitsChunk ( pDocs );
			if ( pHits )
			{
				pHit = pHits;
				continue;
			}

			pDoc = pDocs = m_pNode->GetDocsChunk ();
			if ( !pDocs )
				break;

			pHit = pHits = m_pNode->GetHitsChunk ( pDocs );
			assert ( pHit );
			continue;
		}

		if ( pHit->m_uDocid==m_uExpID && pHit->m_uHitpos==m_uExpPos )
		{
			// stream position is as expected; let's check query position
			if ( pHit->m_uQuerypos!=m_uExpQpos )
			{
				// unexpected query position
				// do nothing; there might be other words in same (!) expected position following
				// (eg. if the query words are repeated)

			} else if ( m_uExpQpos==m_uMaxQpos )
			{
				// expected position which concludes the phrase; emit next match
				assert ( pHit->m_uQuerypos==m_uExpQpos );

				if ( pHit->m_uDocid!=m_uLastDocID )
				{
					assert ( pDoc->m_uDocid<=pHit->m_uDocid );
					while ( pDoc->m_uDocid < pHit->m_uDocid ) pDoc++;
					assert ( pDoc->m_uDocid==pHit->m_uDocid );

					m_dDocs[iDoc].m_uDocid = pHit->m_uDocid;
					m_dDocs[iDoc].m_uFields = ( 1UL<<(pHit->m_uHitpos>>24) );
					m_dDocs[iDoc].m_uHitlistOffset = -1;
					m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
					iDoc++;
					m_uLastDocID = pHit->m_uDocid;
				}

				DWORD uSpanlen = m_uMaxQpos - m_uMinQpos;
				m_dMyHits[iHit].m_uDocid = pHit->m_uDocid;
				m_dMyHits[iHit].m_uHitpos = pHit->m_uHitpos - uSpanlen;
				m_dMyHits[iHit].m_uQuerypos = m_uMinQpos;
				m_dMyHits[iHit].m_uSpanlen = uSpanlen + 1;
				iHit++;

				m_uExpID = m_uExpPos = m_uExpQpos = 0;
			} else
			{
				// intermediate expected position; keep looking
				assert ( pHit->m_uQuerypos==m_uExpQpos );
				int iDelta = m_dQposDelta [ pHit->m_uQuerypos - m_uMinQpos ];
				m_uExpPos += iDelta;
				m_uExpQpos += iDelta;
			}

		} else
		{
			// stream position out of sequence; reset expected positions
			if ( pHit->m_uQuerypos==m_uMinQpos )
			{
				m_uExpID = pHit->m_uDocid;
				m_uExpPos = pHit->m_uHitpos + m_dQposDelta[0];
				m_uExpQpos = pHit->m_uQuerypos + m_dQposDelta[0];
			} else
			{
				m_uExpID = m_uExpPos = m_uExpQpos = 0;
			}
		}
		pHit++;
	}

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dMyHits[iHit].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

const ExtHit_t * ExtPhrase_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	// if we already emitted hits for this matches block, do not do that again
	SphDocID_t uFirstMatch = pDocs->m_uDocid;
	if ( uFirstMatch==m_uHitsOverFor )
		return NULL;

	const ExtDoc_t * pMyDoc = m_dDocs;
	const ExtHit_t * pMyHit = m_dMyHits;

	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// pull next doc if needed
		if ( !m_uMatchedDocid )
		{
			do
			{
				while ( pMyDoc->m_uDocid < pDocs->m_uDocid ) pMyDoc++;
				if ( pMyDoc->m_uDocid==DOCID_MAX ) break;

				while ( pDocs->m_uDocid < pMyDoc->m_uDocid ) pDocs++;
				if ( pDocs->m_uDocid==DOCID_MAX ) break;

			} while ( pDocs->m_uDocid!=pMyDoc->m_uDocid );

			if ( pDocs->m_uDocid!=pMyDoc->m_uDocid )
			{
				assert ( pMyDoc->m_uDocid==DOCID_MAX || pDocs->m_uDocid==DOCID_MAX );
				break;
			}

			assert ( pDocs->m_uDocid==pMyDoc->m_uDocid );
			assert ( pDocs->m_uDocid!=0 );
			assert ( pDocs->m_uDocid!=DOCID_MAX );

			m_uMatchedDocid = pDocs->m_uDocid;
		}

		// skip until we have to
		while ( pMyHit->m_uDocid < m_uMatchedDocid ) pMyHit++;
		assert ( pMyHit->m_uDocid==m_uMatchedDocid );

		// copy while we can
		assert ( m_uMatchedDocid!=0 && m_uMatchedDocid!=DOCID_MAX );
		while ( pMyHit->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
			m_dHits[iHit++] = *pMyHit++;

		// if this hitlist is over, tell the doclist scanner to pull next doc
		if ( pMyHit->m_uDocid!=m_uMatchedDocid )
		{
			m_uMatchedDocid = 0;
			pMyDoc++;
		}
	}

	if ( iHit==0 || iHit<MAX_HITS-1 )
		m_uHitsOverFor = uFirstMatch;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX; // end marker
	return iHit ? m_dHits : NULL;
}

void ExtPhrase_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->GetQwords ( hQwords );
}

void ExtPhrase_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

ExtProximity_c::ExtProximity_c ( const CSphExtendedQueryNode * pNode, DWORD uFields, const CSphTermSetup & tSetup, DWORD uQuerypos, CSphString * pWarning )
	: ExtPhrase_c ( pNode, uFields, tSetup, uQuerypos, pWarning )
	, m_iMaxDistance ( pNode->m_tAtom.m_iMaxDistance )
	, m_iNumWords ( pNode->m_tAtom.m_dWords.GetLength() )
{
	assert ( pNode->IsPlain() );
	assert ( m_iMaxDistance>0 );
}

const ExtDoc_t * ExtProximity_c::GetDocsChunk ()
{
	const ExtDoc_t * pDocs = NULL;
	const ExtHit_t * pHits = NULL;

	if ( !pDocs ) pDocs = m_pNode->GetDocsChunk ();
	if ( !pDocs ) return NULL;

	if ( !pHits ) pHits = m_pNode->GetHitsChunk ( pDocs );
	assert ( pHits );

	const ExtHit_t * pHit = pHits;
	const ExtDoc_t * pDoc = pDocs;

	CSphVector<DWORD> dProx; // proximity hit position for i-th word
	int iProxWords = 0;
	int iProxMinEntry = -1;

	dProx.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, dProx )
		dProx[i] = UINT_MAX;

	int iDoc = 0;
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// update hitlist
		if ( pHit->m_uDocid==DOCID_MAX )
		{
			m_uExpID = m_uExpPos = m_uExpQpos = 0;

			pHits = m_pNode->GetHitsChunk ( pDocs );
			if ( pHits )
			{
				pHit = pHits;
				continue;
			}

			pDoc = pDocs = m_pNode->GetDocsChunk ();
			if ( !pDocs )
				break;

			pHit = pHits = m_pNode->GetHitsChunk ( pDocs );
			assert ( pHit );
			continue;
		}

		// walk through the hitlist and update context
		int iEntry = pHit->m_uQuerypos - m_uMinQpos;

		// check if the incoming hit is out of bounds, or affects min pos
		if (
			!( pHit->m_uDocid==m_uExpID && pHit->m_uHitpos<m_uExpPos ) // out of expected bounds
			|| iEntry==iProxMinEntry ) // or simply affects min pos
		{
			if ( pHit->m_uDocid!=m_uExpID )
			{
				ARRAY_FOREACH ( i, dProx ) dProx[i] = UINT_MAX;
				m_uExpID = pHit->m_uDocid;

				dProx[iEntry] = pHit->m_uHitpos;
				iProxMinEntry = iEntry;
				iProxWords = 1;

			} else
			{
				// update and recompute for old document
				if ( dProx[iEntry]==UINT_MAX )
					iProxWords++;
				dProx[iEntry] = pHit->m_uHitpos;
				iProxMinEntry = iEntry;

				int iMinPos = pHit->m_uHitpos - ( m_uMaxQpos-m_uMinQpos ) - m_iMaxDistance;
				DWORD uMin = pHit->m_uHitpos;
				ARRAY_FOREACH ( i, dProx )
					if ( dProx[i]!=UINT_MAX )
				{
					if ( int(dProx[i])<=iMinPos )
					{
						dProx[i] = UINT_MAX;
						iProxWords--;
						continue;
					}
					if ( dProx[i]<uMin )
					{
						iProxMinEntry = i;
						uMin = dProx[i];
					}
				}
			}

			m_uExpPos = dProx[iProxMinEntry] + ( m_uMaxQpos-m_uMinQpos ) + m_iMaxDistance;

		} else
		{
			// incoming hit within context bounds; check if the word is new
			if ( dProx[iEntry]==UINT_MAX )
				iProxWords++;

			// update the context
			dProx[iEntry] = pHit->m_uHitpos;
		}

		// all words were found within given distance?
		if ( iProxWords==m_iNumWords )
		{
			// emit document, if it's new
			if ( pHit->m_uDocid!=m_uLastDocID )
			{
				assert ( pDoc->m_uDocid<=pHit->m_uDocid );
				while ( pDoc->m_uDocid < pHit->m_uDocid ) pDoc++;
				assert ( pDoc->m_uDocid==pHit->m_uDocid );

				m_dDocs[iDoc].m_uDocid = pHit->m_uDocid;
				m_dDocs[iDoc].m_uFields = ( 1UL<<(pHit->m_uHitpos>>24) );
				m_dDocs[iDoc].m_uHitlistOffset = -1;
				m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
				iDoc++;
				m_uLastDocID = pHit->m_uDocid;
			}

			// compute phrase weight
			//
			// FIXME! should also account for proximity factor, which is in 1 to maxdistance range:
			// m_iMaxDistance - ( pHit->m_uHitpos - dProx[iProxMinEntry] - ( m_uMaxQpos-m_uMinQpos ) )
			CSphVector<int> dDeltas;
			dDeltas.Resize ( m_uMaxQpos-m_uMinQpos+1 );

			ARRAY_FOREACH ( i, dProx )
				dDeltas[i] = dProx[i] - i;
			dDeltas.Sort ();

			DWORD uWeight = 0;
			int iLast = -INT_MAX;
			ARRAY_FOREACH ( i, dDeltas )
			{
				if ( dDeltas[i]==iLast ) uWeight++; else uWeight = 1;
				iLast = dDeltas[i];
			}

			// emit hit
			m_dMyHits[iHit].m_uDocid = pHit->m_uDocid;
			m_dMyHits[iHit].m_uHitpos = dProx[iProxMinEntry];
			m_dMyHits[iHit].m_uQuerypos = m_uMinQpos;
			m_dMyHits[iHit].m_uSpanlen = uWeight;
			iHit++;

			// remove current min, and force recompue
			dProx[iProxMinEntry] = UINT_MAX;
			iProxMinEntry = -1;
			iProxWords--;
			m_uExpPos = 0;
		}

		// go on
		pHit++;
	}

	assert ( iDoc>=0 && iDoc<MAX_DOCS );
	m_dDocs[iDoc].m_uDocid = DOCID_MAX; // end marker

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dMyHits[iHit].m_uDocid = DOCID_MAX; // end marker
	return iDoc ? m_dDocs : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtRanker_c::ExtRanker_c ( const CSphExtendedQueryNode * pAccept, const CSphExtendedQueryNode * pReject, const CSphTermSetup & tSetup, CSphString * pWarning )
{
	for ( int i=0; i<ExtNode_i::MAX_DOCS; i++ )
		m_dMatches[i].Reset ( tSetup.m_tMin.m_iRowitems + tSetup.m_iToCalc );

	assert ( pAccept );
	m_pRoot = ExtNode_i::Create ( pAccept, tSetup, 1, pWarning );

	if ( pReject )
	{
		ExtNode_i * pRej = ExtNode_i::Create ( pReject, tSetup, 1, pWarning );
		if ( pRej )
			m_pRoot = new ExtAndNot_c ( m_pRoot, pRej );
	}

	m_pDoclist = NULL;
	m_pHitlist = NULL;
}

//////////////////////////////////////////////////////////////////////////

int ExtRanker_ProximityBM25_c::GetMatches ( int iFields, const int * pWeights )
{
	if ( !m_pRoot )
		return 0;

	BYTE uLCS[SPH_MAX_FIELDS];
	memset ( uLCS, 0, sizeof(uLCS) );

	BYTE uCurLCS = 0;
	int iExpDelta = -INT_MAX;

	int iMatches = 0;
	const ExtHit_t * pHlist = m_pHitlist;
	const ExtDoc_t * pDocs = m_pDoclist;

	// warmup if necessary
	if ( !pHlist )
	{
		if ( !pDocs ) pDocs = m_pRoot->GetDocsChunk ();
		if ( !pDocs ) return iMatches;

		pHlist = m_pRoot->GetHitsChunk ( pDocs );
		if ( !pHlist ) return iMatches;
	}

	// main matching loop
	const ExtDoc_t * pDoc = pDocs;
	float uCurTFIDF = 0.0f;

	for ( SphDocID_t uCurDocid=0; iMatches<ExtNode_i::MAX_DOCS; )
	{
		assert ( pHlist );

		// next match (or block end)? compute final weight, and flush prev one
		if ( pHlist->m_uDocid!=uCurDocid )
		{
			// if hits block is over, get next block, but do *not* flush current doc
			if ( pHlist->m_uDocid==DOCID_MAX )
			{
				assert ( pDocs );
				pHlist = m_pRoot->GetHitsChunk ( pDocs );
				if ( pHlist )
					continue;
			}

			// otherwise (new match or no next hits block), flush current doc
			if ( uCurDocid )
			{
				uCurLCS = 0;
				DWORD uRank = 0;
				for ( int i=0; i<iFields; i++ )
				{
					uRank += uLCS[i]*pWeights[i];
					uLCS[i] = 0;
				}
				m_dMatches[iMatches].m_iDocID = uCurDocid;
				m_dMatches[iMatches].m_iWeight = uRank*SPH_BM25_SCALE + int( (uCurTFIDF+0.5f)*SPH_BM25_SCALE );
				iMatches++;
				iExpDelta = -1;
			}

			// boundary checks
			if ( !pHlist )
			{
				// there are no more hits for current docs block; do we have a next one?
				assert ( pDocs );
				pDoc = pDocs = m_pRoot->GetDocsChunk ();

				// we don't, so bail out
				if ( !pDocs )
					break;

				// we do, get some hits
				pHlist = m_pRoot->GetHitsChunk ( pDocs );
				assert ( pHlist ); // fresh docs block, must have hits
			}

			// carry on
			assert ( pDoc->m_uDocid<=pHlist->m_uDocid );
			while ( pDoc->m_uDocid<pHlist->m_uDocid ) pDoc++;
			assert ( pDoc->m_uDocid==pHlist->m_uDocid );

			uCurDocid = pHlist->m_uDocid;
			uCurTFIDF = pDoc->m_fTFIDF;

			continue; // we might had flushed the match; need to check the limit
		}

		// upd LCS
		int iDelta = pHlist->m_uHitpos - pHlist->m_uQuerypos;
		if ( iDelta==iExpDelta )
			uCurLCS = uCurLCS + BYTE(pHlist->m_uSpanlen);
		else
			uCurLCS = BYTE(pHlist->m_uSpanlen);

		DWORD uField = pHlist->m_uHitpos >> 24;
		if ( uCurLCS>uLCS[uField] )
			uLCS[uField] = uCurLCS;

		iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
		pHlist++;
	}

	m_pDoclist = pDocs;
	m_pHitlist = pHlist;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

int ExtRanker_BM25_c::GetMatches ( int iFields, const int * pWeights )
{
	if ( !m_pRoot )
		return 0;

	const ExtDoc_t * pDoc = m_pDoclist;
	int iMatches = 0;

	while ( iMatches<ExtNode_i::MAX_DOCS )
	{
		if ( !pDoc || pDoc->m_uDocid==DOCID_MAX ) pDoc = m_pRoot->GetDocsChunk ();
		if ( !pDoc ) { m_pDoclist = NULL; return iMatches; }

		DWORD uRank = 0;
		for ( int i=0; i<iFields; i++ )
			uRank += ( (pDoc->m_uFields>>i)&1 )*pWeights[i];

		m_dMatches[iMatches].m_iDocID = pDoc->m_uDocid;
		m_dMatches[iMatches].m_iWeight = uRank*SPH_BM25_SCALE + int( (pDoc->m_fTFIDF+0.5f)*SPH_BM25_SCALE );
		iMatches++;

		pDoc++;
	}

	m_pDoclist = pDoc;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

int ExtRanker_None_c::GetMatches ( int, const int * )
{
	if ( !m_pRoot )
		return 0;

	const ExtDoc_t * pDoc = m_pDoclist;
	int iMatches = 0;

	while ( iMatches<ExtNode_i::MAX_DOCS )
	{
		if ( !pDoc || pDoc->m_uDocid==DOCID_MAX ) pDoc = m_pRoot->GetDocsChunk ();
		if ( !pDoc ) { m_pDoclist = NULL; return iMatches; }

		m_dMatches[iMatches].m_iDocID = pDoc->m_uDocid;
		m_dMatches[iMatches].m_iWeight = 1;
		iMatches++;

		pDoc++;
	}

	m_pDoclist = pDoc;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

int ExtRanker_Wordcount_c::GetMatches ( int, const int * pWeights )
{
	if ( !m_pRoot )
		return 0;

	int iMatches = 0;
	const ExtHit_t * pHlist = m_pHitlist;
	const ExtDoc_t * pDocs = m_pDoclist;

	// warmup if necessary
	if ( !pHlist )
	{
		if ( !pDocs ) pDocs = m_pRoot->GetDocsChunk ();
		if ( !pDocs ) return iMatches;

		pHlist = m_pRoot->GetHitsChunk ( pDocs );
		if ( !pHlist ) return iMatches;
	}

	// main matching loop
	const ExtDoc_t * pDoc = pDocs;
	DWORD uRank = 0;
	for ( SphDocID_t uCurDocid=0; iMatches<ExtNode_i::MAX_DOCS; )
	{
		assert ( pHlist );

		// next match (or block end)? compute final weight, and flush prev one
		if ( pHlist->m_uDocid!=uCurDocid )
		{
			// if hits block is over, get next block, but do *not* flush current doc
			if ( pHlist->m_uDocid==DOCID_MAX )
			{
				assert ( pDocs );
				pHlist = m_pRoot->GetHitsChunk ( pDocs );
				if ( pHlist )
					continue;
			}

			// otherwise (new match or no next hits block), flush current doc
			if ( uCurDocid )
			{
				m_dMatches[iMatches].m_iDocID = uCurDocid;
				m_dMatches[iMatches].m_iWeight = uRank;
				iMatches++;
				uRank = 0;
			}

			// boundary checks
			if ( !pHlist )
			{
				// there are no more hits for current docs block; do we have a next one?
				assert ( pDocs );
				pDoc = pDocs = m_pRoot->GetDocsChunk ();

				// we don't, so bail out
				if ( !pDocs )
					break;

				// we do, get some hits
				pHlist = m_pRoot->GetHitsChunk ( pDocs );
				assert ( pHlist ); // fresh docs block, must have hits
			}

			// carry on
			assert ( pDoc->m_uDocid<=pHlist->m_uDocid );
			while ( pDoc->m_uDocid<pHlist->m_uDocid ) pDoc++;
			assert ( pDoc->m_uDocid==pHlist->m_uDocid );

			uCurDocid = pHlist->m_uDocid;
			continue; // we might had flushed the match; need to check the limit
		}

		// upd rank
		uRank += pWeights [ pHlist->m_uHitpos>>24 ]; // FIXME! boundary check
		pHlist++;
	}

	m_pDoclist = pDocs;
	m_pHitlist = pHlist;
	return iMatches;	
}

//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::CheckExtendedQuery ( const CSphExtendedQueryNode * pNode, CSphQueryResult * pResult )
{
	ARRAY_FOREACH ( i, pNode->m_tAtom.m_dWords )
		CheckQueryWord ( pNode->m_tAtom.m_dWords [i].m_sWord.cstr (), pResult );

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		CheckExtendedQuery ( pNode->m_dChildren [i], pResult );
}


bool CSphIndex_VLN::MatchExtended ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tTermSetup )
{
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;
	assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );

	// parse query
	CSphExtendedQuery tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), tTermSetup.m_pTokenizer,
		&m_tSchema, tTermSetup.m_pDict ) )
	{
		m_sLastError = tParsed.m_sParseError;
		return false;
	}

	CheckExtendedQuery ( tParsed.m_pAccept, pResult );
	CheckExtendedQuery ( tParsed.m_pReject, pResult );

	// setup eval-tree
	CSphScopedPtr<ExtRanker_c> pRoot ( NULL );
	switch ( pQuery->m_eRanker )
	{
		case SPH_RANK_PROXIMITY_BM25:	pRoot = new ExtRanker_ProximityBM25_c ( tParsed.m_pAccept, tParsed.m_pReject, tTermSetup, &(pResult->m_sWarning) ); break;
		case SPH_RANK_BM25:				pRoot = new ExtRanker_BM25_c ( tParsed.m_pAccept, tParsed.m_pReject, tTermSetup, &(pResult->m_sWarning) ); break;
		case SPH_RANK_NONE:				pRoot = new ExtRanker_None_c ( tParsed.m_pAccept, tParsed.m_pReject, tTermSetup, &(pResult->m_sWarning) ); break;
		case SPH_RANK_WORDCOUNT:		pRoot = new ExtRanker_Wordcount_c ( tParsed.m_pAccept, tParsed.m_pReject, tTermSetup, &(pResult->m_sWarning) ); break;

		default:
			pResult->m_sWarning.SetSprintf ( "unknown ranking mode %d; using default", (int)pQuery->m_eRanker );
			pRoot = new ExtRanker_ProximityBM25_c ( tParsed.m_pAccept, tParsed.m_pReject, tTermSetup, &(pResult->m_sWarning) );
			break;
	}
	assert ( pRoot.Ptr() );

	// setup word stats and IDFs
	ExtQwordsHash_t hQwords;
	pRoot->GetQwords ( hQwords );

	pResult->m_iNumWords = 0;
	const int iQwords = hQwords.GetLength ();

	hQwords.IterateStart ();
	while ( hQwords.IterateNext() )
	{
		ExtQword_t & tWord = hQwords.IterateGet ();

		// build IDF
		float fIDF = 0.0f;
		if ( tWord.m_iDocs )
		{
			float fLogTotal = logf ( float(1+m_tStats.m_iTotalDocuments) );
			fIDF = logf ( float(m_tStats.m_iTotalDocuments-tWord.m_iDocs+1)/float(tWord.m_iDocs) )
				/ ( 2*iQwords*fLogTotal );
		}
		tWord.m_fIDF = fIDF;

		// update word stats
		if ( pResult->m_iNumWords<SPH_MAX_QUERY_WORDS )
		{
			CSphQueryResult::WordStat_t & tStats = pResult->m_tWordStats [ pResult->m_iNumWords++ ];
			if ( tStats.m_sWord.cstr() )
			{
				assert ( tStats.m_sWord==tWord.m_sWord );
				tStats.m_iDocs += tWord.m_iDocs;
				tStats.m_iHits += tWord.m_iHits;
			} else
			{
				tStats.m_sWord = tWord.m_sWord;
				tStats.m_iDocs = tWord.m_iDocs;
				tStats.m_iHits = tWord.m_iHits;
			}
		}
	}

	pRoot->SetQwordsIDF ( hQwords );

	// do searching
	for ( ;; )
	{
		int iMatches = pRoot->GetMatches ( m_iWeights, m_dWeights );
		if ( iMatches<=0 )
			break;

		for ( int i=0; i<iMatches; i++ )
		{
			CSphMatch & tMatch = pRoot->m_dMatches[i];

			// early reject by group id, doc id or timestamp
			if ( m_bEarlyLookup )
				LookupDocinfo ( tMatch );

			if ( sphMatchEarlyReject ( tMatch, pQuery, m_pMva ) )
				continue;

			// submit
			SPH_SUBMIT_MATCH ( tMatch );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::MatchFullScan ( const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters, const CSphTermSetup & tSetup )
{
	if ( m_uDocinfo<=0 || m_eDocinfo!=SPH_DOCINFO_EXTERN || m_pDocinfo.IsEmpty() || !m_tSchema.GetAttrsCount() )
		return; // FIXME? produce error/warning, at least

	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;

	CSphMatch tMatch;
	tMatch.Reset ( tSetup.m_tMin.m_iRowitems + tSetup.m_iToCalc );
	tMatch.m_iWeight = 1;

	DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	for ( DWORD uIndexEntry=0; uIndexEntry<m_uDocinfoIndex; uIndexEntry++ )
	{
		/////////////////////////
		// block-level filtering
		/////////////////////////

		const DWORD * pMin = &m_pDocinfoIndex[2*uIndexEntry*uStride];
		const DWORD * pMax = pMin + uStride;

		// check id ranges
		if ( DOCINFO2ID(pMax)<pQuery->m_iMinID )
			continue;

		// if we are already over max-id, we are done
		if ( DOCINFO2ID(pMin)>pQuery->m_iMaxID )
			break;

		// check applicable filters
		bool bReject = false;
		for ( int iFilter=0; iFilter<pQuery->m_dFilters.GetLength() && !bReject; iFilter++ )
		{
			const CSphFilter & tFilter = pQuery->m_dFilters[iFilter];
			if ( tFilter.m_iBitOffset<0 )
				continue;

			if ( tFilter.m_iBitOffset>=m_tSchema.GetRowSize()*ROWITEM_BITS ) // index schema only contains non-computed attrs
				continue; // no early reject for computed attrs

			if ( tFilter.m_eType==SPH_FILTER_VALUES )
			{
				if ( tFilter.m_bExclude )
					continue; // no early reject for this case yet

				// check all acceptable values against the range
				DWORD uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS(pMin), tFilter.m_iBitOffset, tFilter.m_iBitCount );
				DWORD uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS(pMax), tFilter.m_iBitOffset, tFilter.m_iBitCount );
				bool bBlockMatches = false;

				ARRAY_FOREACH ( i, tFilter.m_dValues )
					if ( tFilter.m_dValues[i]>=uBlockMin && tFilter.m_dValues[i]<=uBlockMax )
				{
					bBlockMatches = true;
					break;
				}

				// if all acceptable values are out of block range, we can safely reject it
				if ( !bBlockMatches )
					bReject = true;

			} else if ( tFilter.m_eType==SPH_FILTER_RANGE )
			{
				DWORD uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS(pMin), tFilter.m_iBitOffset, tFilter.m_iBitCount );
				DWORD uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS(pMax), tFilter.m_iBitOffset, tFilter.m_iBitCount );

				if ( !tFilter.m_bExclude )
				{
					// include-filter, non-overlapping block-range and include-range
					if ( uBlockMin>tFilter.m_uMaxValue || uBlockMax<tFilter.m_uMinValue )
						bReject = true;
				} else
				{
					// exclude-filter, block range fully inside exclude-range
					if ( uBlockMin>=tFilter.m_uMinValue && uBlockMax<=tFilter.m_uMaxValue )
						bReject = true;
				}

			} else if ( tFilter.m_eType==SPH_FILTER_FLOATRANGE )
			{
				float fBlockMin = *( reinterpret_cast<const float*> ( DOCINFO2ATTRS(pMin)+tFilter.m_iRowitem ) );
				float fBlockMax = *( reinterpret_cast<const float*> ( DOCINFO2ATTRS(pMax)+tFilter.m_iRowitem ) );

				if ( !tFilter.m_bExclude )
				{
					// include-filter, non-overlapping block-range and include-range
					if ( fBlockMin>tFilter.m_fMaxValue || fBlockMax<tFilter.m_fMinValue )
						bReject = true;
				} else
				{
					// exclude-filter, block range fully inside exclude-range
					if ( fBlockMin>=tFilter.m_fMinValue && fBlockMax<=tFilter.m_fMaxValue )
						bReject = true;
				}
			}
		}

		if ( bReject )
			continue;

		///////////////////////
		// row-level filtering
		///////////////////////

		const DWORD * pBlockStart = &m_pDocinfo [ uStride*uIndexEntry*DOCINFO_INDEX_FREQ ];
		const DWORD * pBlockEnd = &m_pDocinfo [ uStride*( Min ( (uIndexEntry+1)*DOCINFO_INDEX_FREQ, m_uDocinfo ) - 1 ) ];

		for ( const DWORD * pDocinfo=pBlockStart; pDocinfo<=pBlockEnd; pDocinfo+=uStride )
		{
			tMatch.m_iDocID = DOCINFO2ID(pDocinfo);
			CalcDocinfo ( tMatch, pDocinfo );

			if ( sphMatchEarlyReject ( tMatch, pQuery, m_pMva ) )
				continue;

			SPH_SUBMIT_MATCH ( tMatch );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::CheckQueryWord ( const char * szWord, CSphQueryResult * pResult )
{
	if ( ( !m_iMinPrefixLen && !m_iMinInfixLen ) || !m_bEnableStar || !szWord )
		return;

	int iLen = strlen ( szWord );
	bool bHeadStar = szWord [0] == '*';
	bool bTailStar = szWord [iLen - 1] == '*';
	int iLenWOStars = iLen - ( bHeadStar ? 1 : 0 ) - ( bTailStar ? 1 : 0 );
	if ( bHeadStar || bTailStar )
	{
		if ( m_iMinInfixLen > 0 && iLenWOStars < m_iMinInfixLen )
			pResult->m_sWarning.SetSprintf ( "Query word length is less than min infix length. word: '%s' ", szWord );
		else
			if ( m_iMinPrefixLen > 0 && iLenWOStars < m_iMinPrefixLen )
				pResult->m_sWarning.SetSprintf ( "Query word length is less than min prefix length. word: '%s' ", szWord );
	}
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::SetupQueryWord ( CSphQueryWord & tWord, const CSphTermSetup & tTermSetup )
{
	tWord.m_iDocs = 0;
	tWord.m_iHits = 0;

	// binary search through checkpoints for a one whose range matches word ID
	assert ( m_bPreread[0] && !m_pWordlist.IsEmpty() );

	// empty index?
	if ( !m_iWordlistCheckpoints )
		return false;

	int iWordlistOffset = -1;
	CSphWordlistCheckpoint * pStart = m_pWordlistCheckpoints;
	CSphWordlistCheckpoint * pEnd = m_pWordlistCheckpoints + m_iWordlistCheckpoints - 1;

	if ( tWord.m_iWordID < sphUnalignedRead ( pStart->m_iWordID ) )
		return false;

	if ( tWord.m_iWordID >= sphUnalignedRead ( pEnd->m_iWordID ) )
	{
		iWordlistOffset = sphUnalignedRead ( pEnd->m_iWordlistOffset );
	} else
	{
		while ( pEnd-pStart>1 )
		{
			CSphWordlistCheckpoint * pMid = pStart + (pEnd-pStart)/2;

			if ( tWord.m_iWordID == sphUnalignedRead ( pMid->m_iWordID ) )
			{
				pStart = pEnd = pMid;
				break;

			} else if ( tWord.m_iWordID < sphUnalignedRead ( pMid->m_iWordID ) )
			{
				pEnd = pMid;

			} else
			{
				pStart = pMid;
			}
		}

		assert ( pStart >= m_pWordlistCheckpoints );
		assert ( pStart <= m_pWordlistCheckpoints+m_iWordlistCheckpoints-1 );
		assert ( tWord.m_iWordID >= sphUnalignedRead(pStart->m_iWordID) && tWord.m_iWordID < sphUnalignedRead((pStart+1)->m_iWordID) );
		iWordlistOffset = sphUnalignedRead ( pStart->m_iWordlistOffset );
	}
	assert ( iWordlistOffset>=0 );

	// decode wordlist chunk
	const BYTE * pBuf = &m_pWordlist[iWordlistOffset];

	SphWordID_t iWordID = 0;
	SphOffset_t iDoclistOffset = 0;

	for ( ;; )
	{
		// unpack next word ID
		SphWordID_t iDeltaWord = sphUnzipWordid ( pBuf ); // FIXME! slow with 32bit wordids

		if ( iDeltaWord==0 ) // wordlist chunk is over
			break;
		iWordID += iDeltaWord;

		// list is sorted, so if there was no match, there's no such word
		if ( iWordID>tWord.m_iWordID )
			return false;

		// unpack next offset
		SphOffset_t iDeltaOffset = sphUnzipOffset ( pBuf );
		iDoclistOffset += iDeltaOffset;
		assert ( iDeltaOffset );

		// unpack doc/hit count
		int iDocs = sphUnzipInt ( pBuf );
		int iHits = sphUnzipInt ( pBuf );
		assert ( iDocs );
		assert ( iHits );

		// it matches?!
		if ( iWordID==tWord.m_iWordID )
		{
			// unpack next word ID and offset delta (ie. hitlist length)
			sphUnzipWordid ( pBuf ); // might be 0 at checkpoint
			SphOffset_t iDoclistLen = sphUnzipOffset ( pBuf );

			tWord.m_rdDoclist.SetFile ( tTermSetup.m_tDoclist );
			tWord.m_rdDoclist.SeekTo ( iDoclistOffset, (int)iDoclistLen );
			tWord.m_rdHitlist.SetFile ( tTermSetup.m_tHitlist );
			tWord.m_iDocs = iDocs;
			tWord.m_iHits = iHits;
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::Lock ()
{
	const char * sName = GetIndexFileName ( "spl" );

	if ( m_iLockFD<0 )
	{
		m_iLockFD = ::open ( sName, SPH_O_NEW, 0644 );
		if ( m_iLockFD<0 )
		{
			m_sLastError.SetSprintf ( "failed to open %s: %s", sName, strerror(errno) );
			return false;
		}
	}

	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock %s: %s", sName, strerror(errno) );
		::close ( m_iLockFD );
		return false;
	}

	return true;
}


void CSphIndex_VLN::Unlock() 
{
	if ( m_iLockFD>=0 )
	{
		::close ( m_iLockFD );
		::unlink ( GetIndexFileName ( "spl" ) );
		m_iLockFD = -1;
	}
}


bool CSphIndex_VLN::Mlock ()
{
	bool bRes = true;
	bRes &= m_pDocinfo.Mlock ( "docinfo", m_sLastError );
	bRes &= m_pWordlist.Mlock ( "wordlist", m_sLastError );
	bRes &= m_pMva.Mlock ( "mva", m_sLastError );
	return bRes;
}


void CSphIndex_VLN::Dealloc ()
{
	if ( !m_bPreallocated )
		return;

	m_pDocinfo.Reset ();
	m_pDocinfoHash.Reset ();
	m_pWordlist.Reset ();
	m_pMva.Reset ();
	m_pDocinfoIndex.Reset ();

	m_uDocinfo = 0;
	m_eDocinfo = SPH_DOCINFO_NONE;
	m_bAttrsUpdated = false;

	m_bPreallocated = false;
}


const CSphSchema * CSphIndex_VLN::Prealloc ( bool bMlock, CSphString * sWarning )
{
	// reset
	Dealloc ();

	// always keep preread flag
	if ( m_bPreread.IsEmpty() )
	{
		if ( !m_bPreread.Alloc ( 1, m_sLastError, NULL ) )
			return NULL;
	}
	m_bPreread.GetWritePtr()[0] = 0;

	// set new locking flag
	m_pDocinfo.SetMlock ( bMlock );
	m_pWordlist.SetMlock ( bMlock );
	m_pMva.SetMlock ( bMlock );

	//////////////////
	// preload schema
	//////////////////

	CSphAutofile tIndexInfo ( GetIndexFileName("sph"), SPH_O_READ, m_sLastError );
	if ( tIndexInfo.GetFD()<0 )
		return NULL;

	BYTE dCacheInfo [ 32768 ];
	SphOffset_t iHeaderSize = tIndexInfo.GetSize ();
	if ( size_t (iHeaderSize) > sizeof(dCacheInfo) )
	{
		m_sLastError.SetSprintf ( "unexpectedly large header file (size=%d, max=%d)", (int)iHeaderSize, sizeof(dCacheInfo) );
		return NULL;
	}

	CSphReader_VLN rdInfo ( dCacheInfo, sizeof(dCacheInfo) ); // to avoid mallocs
	rdInfo.SetFile ( tIndexInfo );

	// version
	DWORD uHeader = rdInfo.GetDword ();
	if ( uHeader!=INDEX_MAGIC_HEADER )
	{
		m_sLastError.SetSprintf ( "'%s': invalid schema file (old index version?)",
			GetIndexFileName ( "sph" ) );
		return NULL;
	}

	m_uVersion = rdInfo.GetDword();
	if ( m_uVersion==0 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", GetIndexFileName("sph"),
			m_uVersion, INDEX_FORMAT_VERSION );
		return NULL;
	}

	// bits
	DWORD bUse64 = false;
	if ( m_uVersion>=2 )
		bUse64 = rdInfo.GetDword ();
	if ( bUse64!=USE_64BIT )
	{
		m_sLastError.SetSprintf ( "'%s' is id%d, and this binary is id%d",
			GetIndexFileName ( "sph" ), bUse64 ? 64 : 32, USE_64BIT ? 64 : 32 );
		return NULL;
	}

	// docinfo
	m_eDocinfo = (ESphDocinfo) rdInfo.GetDword();

	// schema
	m_tSchema.Reset ();

	m_tSchema.m_dFields.Resize ( rdInfo.GetDword() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		ReadSchemaColumn ( rdInfo, m_tSchema.m_dFields[i] );

	int iNumAttrs = rdInfo.GetDword();
	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		m_tSchema.AddAttr ( tCol );
	}

	// min doc
	m_tMin.Reset ( m_tSchema.GetRowSize() );
	if ( m_uVersion>=2 )
		m_tMin.m_iDocID = (SphDocID_t) rdInfo.GetOffset (); // v2+; losing high bits when !USE_64 is intentional, check is performed on bUse64 above
	else
		m_tMin.m_iDocID = rdInfo.GetDword(); // v1
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		rdInfo.GetRawBytes ( m_tMin.m_pRowitems, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );

	// wordlist checkpoints
	SphOffset_t iCheckpointsPos;
	rdInfo.GetRawBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	m_iWordlistCheckpoints = rdInfo.GetDword ();

	// index stats
	m_tStats.m_iTotalDocuments = rdInfo.GetDword ();
	m_tStats.m_iTotalBytes = rdInfo.GetOffset ();

	// infix stuff
	if ( m_uVersion>=8 )
	{
		m_iMinPrefixLen = rdInfo.GetDword ();
		m_iMinInfixLen = rdInfo.GetDword ();

	} else if ( m_uVersion>=6 )
	{
		bool bPrefixesOnly = ( rdInfo.GetByte ()!=0 );
		m_iMinPrefixLen = rdInfo.GetDword ();
		m_iMinInfixLen = 0;
		if ( !bPrefixesOnly )
			Swap ( m_iMinPrefixLen, m_iMinInfixLen );
	}

	///////////////////////////////////////
	// verify that data files are readable
	///////////////////////////////////////

	if ( !sphIsReadable ( GetIndexFileName("spd"), &m_sLastError ) )
		return NULL;

	if ( m_uVersion>=3 && !sphIsReadable ( GetIndexFileName("spp"), &m_sLastError ) )
		return NULL;

	/////////////////////
	// prealloc docinfos
	/////////////////////

	if ( m_eDocinfo==SPH_DOCINFO_EXTERN )
	{
		/////////////
		// attr data
		/////////////

		int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		int iEntrySize = sizeof(DWORD)*iStride;

		CSphAutofile tDocinfo ( GetIndexFileName("spa"), SPH_O_READ, m_sLastError );
		if ( tDocinfo.GetFD()<0 )
			return NULL;

		SphOffset_t iDocinfoSize = tDocinfo.GetSize ( iEntrySize, true, m_sLastError );
		if ( iDocinfoSize<0 )
			return NULL;

		// intentionally losing data; we don't support more than 4B documents per instance yet
		m_uDocinfo = (DWORD)( iDocinfoSize / iEntrySize );
		if ( iDocinfoSize!=SphOffset_t(m_uDocinfo)*SphOffset_t(iEntrySize) )
		{
			m_sLastError.SetSprintf ( "docinfo size check mismatch (4B document limit hit?)" );
			return NULL;
		}

		// prealloc
		if ( !m_pDocinfo.Alloc ( DWORD(iDocinfoSize/sizeof(DWORD)), m_sLastError, sWarning ) )
			return NULL;

		if ( m_pDocinfoHash.IsEmpty() )
			if ( !m_pDocinfoHash.Alloc ( (1<<DOCINFO_HASH_BITS)+4, m_sLastError, sWarning ) )
				return NULL;

		m_uDocinfoIndex = ( m_uDocinfo+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;
		if ( !m_pDocinfoIndex.Alloc ( 2*m_uDocinfoIndex*iStride, m_sLastError, sWarning ) ) // 2x because we store min/max
			return NULL;

		////////////
		// MVA data
		////////////

		if ( m_uVersion>=4 )
		{
			// if index is v4, .spm must always exist, even though length could be 0
			CSphAutofile fdMva ( GetIndexFileName("spm"), SPH_O_READ, m_sLastError );
			if ( fdMva.GetFD()<0 )
				return NULL;

			SphOffset_t iMvaSize = fdMva.GetSize ( 0, true, m_sLastError );
			if ( iMvaSize<0 )
				return NULL;

			// prealloc
			if ( iMvaSize>0 )
				if ( !m_pMva.Alloc ( DWORD(iMvaSize/sizeof(DWORD)), m_sLastError, sWarning ) )
					return NULL;
		}
	}

	/////////////////////
	// prealloc wordlist
	/////////////////////

	CSphAutofile tWordlist ( GetIndexFileName("spi"), SPH_O_READ, m_sLastError );
	if ( tWordlist.GetFD()<0 )
		return NULL;

	SphOffset_t iWordlistLen = tWordlist.GetSize ( 1, true, m_sLastError );
	if ( iWordlistLen<0 )
		return NULL;

	if ( !m_pWordlist.Alloc ( DWORD(iWordlistLen), m_sLastError, sWarning ) )
		return NULL;

	assert ( iCheckpointsPos>0 && iCheckpointsPos<INT_MAX );
	m_pWordlistCheckpoints = ( iCheckpointsPos>=iWordlistLen )
		? NULL
		: (CSphWordlistCheckpoint *)( &m_pWordlist[int(iCheckpointsPos)] );

	// all done
	m_bPreallocated = true;
	return &m_tSchema;
}


template < typename T > bool CSphIndex_VLN::PrereadSharedBuffer ( CSphSharedBuffer<T> & pBuffer, const char * sExt )
{
	if ( !pBuffer.GetLength() )
		return true;

	CSphAutofile fdBuf ( GetIndexFileName(sExt), SPH_O_READ, m_sLastError );
	if ( fdBuf.GetFD()<0 )
		return false;

	if ( !fdBuf.Read ( pBuffer.GetWritePtr(), size_t(pBuffer.GetLength()), m_sLastError ) )
		return false;

	return true;
}


/// attribute value location in the row
struct AttrLocator_t
{
	int m_iRowitem;
	int m_iBitOffset;
	int m_iBitCount;

	AttrLocator_t ()
		: m_iRowitem ( -1 )
		, m_iBitOffset ( -1 )
		, m_iBitCount ( -1 )
	{}

	AttrLocator_t ( const CSphColumnInfo & tCol )
		: m_iRowitem	( tCol.m_iRowitem )
		, m_iBitOffset	( tCol.m_iBitOffset )
		, m_iBitCount	( tCol.m_iBitCount )
	{}
};


bool CSphIndex_VLN::Preread ()
{
	if ( !m_bPreallocated )
	{
		m_sLastError = "INTERNAL ERROR: not preallocated";
		return false;
	}
	if ( m_bPreread[0] )
	{
		m_sLastError = "INTERNAL ERROR: already preread";
		return false;
	}

	///////////////////
	// read everything
	///////////////////

	if ( !PrereadSharedBuffer ( m_pDocinfo, "spa" ) )
		return false;

	if ( !PrereadSharedBuffer ( m_pMva, "spm" ) )
		return false;

	if ( !PrereadSharedBuffer ( m_pWordlist, "spi" ) )
		return false;

	//////////////////////
	// precalc everything
	//////////////////////

	// build attributes hash
	if ( m_pDocinfo.GetLength() )
	{
		int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		SphDocID_t uFirst = DOCINFO2ID ( &m_pDocinfo[0] );
		SphDocID_t uRange = DOCINFO2ID ( &m_pDocinfo[(m_uDocinfo-1)*iStride] ) - uFirst;
		DWORD iShift = 0;
		while ( uRange >= (1<<DOCINFO_HASH_BITS) )
		{
			iShift++;
			uRange >>= 1;
		}

		DWORD * pHash = m_pDocinfoHash.GetWritePtr();
		*pHash++ = iShift;
		*pHash = 0;
		DWORD uLastHash = 0;

		for ( DWORD i=1; i<m_uDocinfo; i++ )
		{
			DWORD uHash = (DWORD)( ( DOCINFO2ID ( &m_pDocinfo[i*iStride] ) - uFirst ) >> iShift );
			if ( uHash==uLastHash )
				continue;

			while ( uLastHash<uHash )
				pHash [ ++uLastHash ] = i;

			uLastHash = uHash;
		}
		pHash [ ++uLastHash ] = m_uDocinfo;
	}

	// build "indexes" for full-scan
	if ( m_pDocinfoIndex.GetLength() )
	{
		// int attrs to the left, float attrs to the right
		CSphVector<AttrLocator_t> dIntAttrs;
		CSphVector<int> dFloatAttrs, dMvaAttrs;

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(i);

			if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
			{
				dMvaAttrs.Add ( tCol.m_iRowitem );
				continue;
			}

			switch ( tCol.m_eAttrType )
			{
				case SPH_ATTR_INTEGER:
				case SPH_ATTR_TIMESTAMP:
				case SPH_ATTR_BOOL:
					dIntAttrs.Add ( tCol );
					break;

				case SPH_ATTR_FLOAT:
					dFloatAttrs.Add ( tCol.m_iRowitem );
					break;

				default:
					break;
			}
		}

		// min/max storage
		CSphVector<CSphRowitem> dIntMin ( dIntAttrs.GetLength() ), dIntMax ( dIntAttrs.GetLength() );
		CSphVector<float> dFloatMin ( dFloatAttrs.GetLength() ), dFloatMax ( dFloatAttrs.GetLength() );
		CSphVector<DWORD> dMvaMin ( dMvaAttrs.GetLength() ), dMvaMax ( dMvaAttrs.GetLength() );

		// scan everyone
		DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		assert ( m_pDocinfoIndex.GetLength()==2*size_t(m_uDocinfoIndex)*uStride*sizeof(DWORD) );

		for ( DWORD uIndexEntry=0; uIndexEntry<m_uDocinfoIndex; uIndexEntry++ )
		{
			// reset
			ARRAY_FOREACH ( i, dIntMin )
			{
				dIntMin[i] = UINT_MAX;
				dIntMax[i] = 0;
			}
			ARRAY_FOREACH ( i, dFloatMin )
			{
				dFloatMin[i] = FLT_MAX;
				dFloatMax[i] = -FLT_MAX;
			}
			ARRAY_FOREACH ( i, dMvaMin )
			{
				dMvaMin[i] = UINT_MAX;
				dMvaMax[i] = 0;
			}

			// aim
			DWORD uStart = uIndexEntry*DOCINFO_INDEX_FREQ;
			DWORD uCount = Min ( (DWORD)DOCINFO_INDEX_FREQ, m_uDocinfo-uStart );

			const DWORD * const pDocinfoStart = &m_pDocinfo [ uStart*uStride ];
			const DWORD * const pDocinfoMax = pDocinfoStart + uCount*uStride;

			// scan everything
			for ( const DWORD * pCur=pDocinfoStart; pCur<pDocinfoMax; pCur+=uStride )
			{
				const DWORD * pRow = DOCINFO2ATTRS(pCur);

				// ints
				ARRAY_FOREACH ( i, dIntAttrs )
				{
					int uRowitem = dIntAttrs[i].m_iRowitem;
					DWORD uVal = ( uRowitem<0 )
						? sphGetRowAttr ( pRow, dIntAttrs[i].m_iBitOffset, dIntAttrs[i].m_iBitCount )
						: pRow [ uRowitem ];
					dIntMin[i] = Min ( dIntMin[i], uVal );
					dIntMax[i] = Max ( dIntMax[i], uVal );
				}

				// floats
				ARRAY_FOREACH ( i, dFloatAttrs )
				{
					float fVal = *( reinterpret_cast<const float*> ( pRow + dFloatAttrs[i] ) );
					dFloatMin[i] = Min ( dFloatMin[i], fVal );
					dFloatMax[i] = Max ( dFloatMax[i], fVal );
				}

				// MVAs
				ARRAY_FOREACH ( i, dMvaAttrs )
				{
					DWORD uOff = pRow [ dMvaAttrs[i] ];
					if ( !uOff )
						continue;

					const DWORD * pMva = &m_pMva [ uOff ];
					for ( DWORD uCount = *pMva++; uCount>0; uCount--, pMva++ )
					{
						dMvaMin[i] = Min ( dMvaMin[i], *pMva );
						dMvaMax[i] = Max ( dMvaMax[i], *pMva );
					}
				}
			}

			// pack and emit
			DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ 2*uIndexEntry*uStride ] );
			DWORD * pMinAttrs = DOCINFO2ATTRS(pMinEntry);
			DWORD * pMaxEntry = pMinEntry + uStride;
			DWORD * pMaxAttrs = pMinAttrs + uStride;

			DOCINFO2ID(pMinEntry) = DOCINFO2ID(pDocinfoStart);
			DOCINFO2ID(pMaxEntry) = DOCINFO2ID(pDocinfoMax-uStride);

			ARRAY_FOREACH ( i, dIntAttrs )
			{
				sphSetRowAttr ( pMinAttrs, dIntAttrs[i].m_iBitOffset, dIntAttrs[i].m_iBitCount, dIntMin[i] );
				sphSetRowAttr ( pMaxAttrs, dIntAttrs[i].m_iBitOffset, dIntAttrs[i].m_iBitCount, dIntMax[i] );
			}
			ARRAY_FOREACH ( i, dFloatAttrs )
			{
				*( reinterpret_cast<float*> ( pMinAttrs+dFloatAttrs[i] ) ) = dFloatMin[i];
				*( reinterpret_cast<float*> ( pMaxAttrs+dFloatAttrs[i] ) ) = dFloatMax[i];
			}
			ARRAY_FOREACH ( i, dMvaAttrs )
			{
				pMinAttrs [ dMvaAttrs[i] ] = dMvaMin[i];
				pMaxAttrs [ dMvaAttrs[i] ] = dMvaMax[i];
			}
		}
	}

	// paranoid MVA verification
	#if PARANOID
	// find out what attrs are MVA
	CSphVector<int> dMvaRowitem;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).m_eAttrType & SPH_ATTR_MULTI )
			dMvaRowitem.Add ( m_tSchema.GetAttr(i).m_iRowitem );

	// for each docinfo entry, verify that MVA attrs point to right storage location
	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	for ( DWORD iDoc=0; iDoc<m_uDocinfo; iDoc++ )
	{
		DWORD * pEntry = (DWORD *)&m_pDocinfo[iDoc*iStride];
		SphDocID_t uDocID = DOCINFO2ID(pEntry);

		DWORD uOff = DOCINFO2ATTRS(pEntry) [ dMvaRowitem[0] ];
		if ( !uOff )
		{
			// its either all or nothing
			ARRAY_FOREACH ( i, dMvaRowitem )
				assert ( DOCINFO2ATTRS(pEntry) [ dMvaRowitem[i] ]==0 );
		} else
		{
			// check id
			DWORD * pMva = (DWORD *)&m_pMva[uOff-DOCINFO_IDSIZE];
			SphDocID_t uMvaID = DOCINFO2ID(pMva);
			assert ( uDocID==uMvaID );

			// walk the trail
			ARRAY_FOREACH ( i, dMvaRowitem )
			{
				assert ( DOCINFO2ATTRS(pEntry) [ dMvaRowitem[i] ]==uOff );
				int iCount = m_pMva[uOff];
				uOff += 1+iCount;
			}
		}
	}
	#endif // PARANOID

	m_bPreread.GetWritePtr()[0] = 1;
	return true;
}


void CSphIndex_VLN::SetBase ( const char * sNewBase )
{
	m_sFilename = sNewBase;
}


bool CSphIndex_VLN::Rename ( const char * sNewBase )
{
	if ( m_sFilename==sNewBase )
		return true;

	// try to rename everything
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];

	const int EXT_COUNT = 7;
	const char * sExts[EXT_COUNT] = { "spa", "spd", "sph", "spi", "spl", "spm", "spp" };
	DWORD uMask = 0;

	int iExt;
	for ( iExt=0; iExt<EXT_COUNT; iExt++ )
	{
		const char * sExt = sExts[iExt];
		if ( !strcmp ( sExt, "spp" ) && m_uVersion<3 ) // .spp files are v3+
			continue;
		if ( !strcmp ( sExt, "spm" ) && m_uVersion<4 ) // .spm files are v4+
			continue;
		if ( !strcmp ( sExt, "spl" ) && m_iLockFD<0 ) // .spl files are locks
			continue;

		snprintf ( sFrom, sizeof(sFrom), "%s.%s", m_sFilename.cstr(), sExt );
		snprintf ( sTo, sizeof(sTo), "%s.%s", sNewBase, sExt );
		if ( ::rename ( sFrom, sTo ) )
		{
			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFrom, sTo, strerror(errno) );
			break;
		}
		uMask |= (1UL<<iExt);
	}

	// are we good?
	if ( iExt==EXT_COUNT )
	{
		SetBase ( sNewBase );
		return true;
	}

	// if there were errors, rollback
	for ( iExt=0; iExt<EXT_COUNT; iExt++ )
	{
		if (!( uMask & (1UL<<iExt) ))
			continue;

		const char * sExt = sExts[iExt];
		snprintf ( sFrom, sizeof(sFrom), "%s.%s", sNewBase, sExt );
		snprintf ( sTo, sizeof(sTo), "%s.%s", m_sFilename.cstr(), sExt );
		if ( ::rename ( sFrom, sTo ) )
		{
			// !COMMIT should handle rollback failures somehow
		}
	}
	return false;
}


bool CSphIndex_VLN::QueryEx ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchSorter * pTop )
{
	bool bRes = MultiQuery ( pTokenizer, pDict, pQuery, pResult, 1, &pTop );
	pResult->m_iTotalMatches += bRes ? pTop->GetTotalCount () : 0;
	pResult->m_tSchema.BuildResultSchema ( pQuery ); 
	return bRes;
}


void CSphIndex_VLN::BindWeights ( const CSphQuery * pQuery )
{
	const int MIN_WEIGHT = 1;

	// defaults
	m_iWeights = Min ( m_tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );
	for ( int i=0; i<m_iWeights; i++ )
		m_dWeights[i] = MIN_WEIGHT;

	// name-bound weights
	if ( pQuery->m_dFieldWeights.GetLength() )
	{
		ARRAY_FOREACH ( i, pQuery->m_dFieldWeights )
		{
			int j = m_tSchema.GetFieldIndex ( pQuery->m_dFieldWeights[i].m_sName.cstr() );
			if ( j>=0 && j<SPH_MAX_FIELDS )
				m_dWeights[j] = Max ( MIN_WEIGHT, pQuery->m_dFieldWeights[i].m_iValue );
		}
		return;
	}

	// order-bound weights
	if ( pQuery->m_pWeights )
	{
		for ( int i=0; i<Min ( m_iWeights, pQuery->m_iWeights ); i++ )
			m_dWeights[i] = Max ( MIN_WEIGHT, (int)pQuery->m_pWeights[i] );
	}
}


bool CSphIndex_VLN::SetupCalc ( CSphQueryResult * pResult, const CSphQuery * pQuery )
{
	// base schema
	pResult->m_tSchema = m_tSchema;

	// geodist
	m_iCalcGeodist = -1;
	if ( pQuery->m_bCalcGeodist )
	{
		if ( !pQuery->m_bGeoAnchor )
		{
			m_sLastError.SetSprintf ( "no geo-anchor point specified in search query, @geodist not available" );
			return false;
		}

		int iLat = m_tSchema.GetAttrIndex ( pQuery->m_sGeoLatAttr.cstr() );
		if ( iLat<0 )
		{
			m_sLastError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLatAttr.cstr() );
			return false;
		}

		int iLong = m_tSchema.GetAttrIndex ( pQuery->m_sGeoLongAttr.cstr() );
		if ( iLong<0 )
		{
			m_sLastError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLongAttr.cstr() );
			return false;
		}

		CSphColumnInfo tGeodist ( "@geodist", SPH_ATTR_FLOAT );
		pResult->m_tSchema.AddAttr ( tGeodist );

		int iAttr = pResult->m_tSchema.GetAttrIndex ( "@geodist" );
		m_iCalcGeodist = pResult->m_tSchema.GetAttr ( iAttr ).m_iRowitem;
		assert ( m_iCalcGeodist>=0 );

		m_iGeoLatRowitem = m_tSchema.GetAttr(iLat).m_iRowitem;
		m_iGeoLongRowitem = m_tSchema.GetAttr(iLong).m_iRowitem;
		m_fGeoAnchorLat = pQuery->m_fGeoLatitude;
		m_fGeoAnchorLong = pQuery->m_fGeoLongitude;
	}

	// sorting expression
	m_iCalcExpr = -1;
	if ( pQuery->m_eSort==SPH_SORT_EXPR )
	{
		CSphColumnInfo tExpr ( "@expr", SPH_ATTR_FLOAT );
		pResult->m_tSchema.AddAttr ( tExpr );

		int iAttr = pResult->m_tSchema.GetAttrIndex ( "@expr" );
		m_iCalcExpr = pResult->m_tSchema.GetAttr ( iAttr ).m_iRowitem;
		assert ( m_iCalcExpr>=0 );

		m_tCalcExpr = pQuery->m_tCalcExpr;
	}

	return true;
}


bool CSphIndex_VLN::MultiQuery ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters )
{
	assert ( pTokenizer );
	assert ( pDict );
	assert ( pQuery );
	assert ( pResult );
	assert ( ppSorters );

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	float tmQueryStart = sphLongTimer ();

	// open files
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
	{
		m_sLastError = "index not preread";
		return false;
	}

	CSphAutofile tDoclist ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	if ( tDoclist.GetFD()<0 )
		return false;

	CSphAutofile tHitlist ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError );
	if ( tHitlist.GetFD()<0 )
		return false;

	// setup proper dict
	bool bUseStarDict = false;
	if (
		( m_uVersion>=7 && ( m_iMinPrefixLen>0 || m_iMinInfixLen>0 ) && m_bEnableStar ) || // v.7 added mangling to infixes
		( m_uVersion==6 && ( m_iMinPrefixLen>0 ) && m_bEnableStar ) ) // v.6 added mangling to prefixes
	{
		bUseStarDict = true;
	}

	CSphDictStar tDictStar ( pDict );
	CSphDictStarV8 tDictStarV8 ( pDict, m_iMinPrefixLen>0, m_iMinInfixLen>0 );
	if ( bUseStarDict )
	{
		pDict = ( m_uVersion>=8 ) ? &tDictStarV8 : &tDictStar; // v.8 introduced new mangling rules

		CSphRemapRange tStar ( '*', '*', '*' ); // FIXME? check and warn if star was already there
		pTokenizer->AddCaseFolding ( tStar );
	}

	// setup calculations and result schema
	if ( !SetupCalc ( pResult, pQuery ) )
		return false;

	// prepare for setup
	CSphTermSetup tTermSetup ( tDoclist, tHitlist );
	tTermSetup.m_pTokenizer = pTokenizer;
	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_eDocinfo;
	tTermSetup.m_tMin = m_tMin;
	tTermSetup.m_iToCalc = pResult->m_tSchema.GetRowSize() - m_tSchema.GetRowSize();
	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_uMaxStamp = sphTimerMsec() + pQuery->m_uMaxQueryMsec; // max_query_time

	PROFILE_END ( query_init );

	// check that docinfo is preloaded
	assert ( m_eDocinfo!=SPH_DOCINFO_EXTERN || !m_pDocinfo.IsEmpty() );

	// fixup "empty query" at low level
	if ( pQuery->m_sQuery.IsEmpty() )
		pQuery->m_eMode = SPH_MATCH_FULLSCAN;

	// split query into words
	bool bSimpleQuery = ( pQuery->m_eMode==SPH_MATCH_ALL
		|| pQuery->m_eMode==SPH_MATCH_ANY
		|| pQuery->m_eMode==SPH_MATCH_PHRASE
		|| pQuery->m_eMode==SPH_MATCH_FULLSCAN );
	if ( bSimpleQuery && pQuery->m_eMode!=SPH_MATCH_FULLSCAN )
	{
		CSphSimpleQueryParser qp;
		int iRealWords = qp.Parse ( pQuery->m_sQuery.cstr(), pTokenizer, pDict );
		if ( !iRealWords )
		{
			pResult->m_iQueryTime += int ( 1000.0f*( sphLongTimer() - tmQueryStart ) );
			return true;
		}

		assert ( iRealWords<=SPH_MAX_QUERY_WORDS );
		m_dQueryWords.Resize ( iRealWords );
		ARRAY_FOREACH ( i, m_dQueryWords )
		{
			CheckQueryWord ( qp.m_dWords[i].m_sWord.cstr (), pResult );

			m_dQueryWords[i].Reset ();
			m_dQueryWords[i].m_sWord = qp.m_dWords[i].m_sWord;
			m_dQueryWords[i].m_iWordID = qp.m_dWords[i].m_uWordID;
			m_dQueryWords[i].m_iQueryPos = qp.m_dWords[i].m_iQueryPos;

			assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );
			m_dQueryWords[i].SetupAttrs ( tTermSetup );
		}

		// setup words from the wordlist
		PROFILE_BEGIN ( query_load_words );

		ARRAY_FOREACH ( i, m_dQueryWords )
			SetupQueryWord ( m_dQueryWords[i], tTermSetup );

		// build word stats
		pResult->m_iNumWords = m_dQueryWords.GetLength();
		ARRAY_FOREACH ( i, m_dQueryWords )
		{
			CSphQueryResult::WordStat_t & tWordStats = pResult->m_tWordStats[i];
			if ( tWordStats.m_sWord.cstr() )
			{
				tWordStats.m_iDocs += m_dQueryWords[i].m_iDocs;
				tWordStats.m_iHits += m_dQueryWords[i].m_iHits;
			} else
			{
				tWordStats.m_sWord = m_dQueryWords[i].m_sWord;
				tWordStats.m_iDocs = m_dQueryWords[i].m_iDocs;
				tWordStats.m_iHits = m_dQueryWords[i].m_iHits;
			}
		}

		// reorder hit lists
		m_dQueryWords.Sort ();

		PROFILE_END ( query_load_words );
	}

	// reorder attr set values and lookup indexes
	ARRAY_FOREACH ( i, pQuery->m_dFilters )
	{
		CSphFilter & tFilter = pQuery->m_dFilters[i];
		tFilter.m_dValues.Sort ();
		tFilter.m_iBitOffset = -1; // filters with negative bit-offset will be ignored
		tFilter.m_iRowitem = INT_MAX;

		// lookup it
		int iAttr = pResult->m_tSchema.GetAttrIndex ( tFilter.m_sAttrName.cstr() );
		if ( iAttr<0 )
			continue; // FIXME! simply continue? should at least warn about bad attr name

		const CSphColumnInfo & tAttr = pResult->m_tSchema.GetAttr(iAttr);
		tFilter.m_iRowitem = tAttr.m_iRowitem;
		tFilter.m_iBitOffset = tAttr.m_iBitOffset;
		tFilter.m_iBitCount = tAttr.m_iBitCount;
		tFilter.m_bMva = ( ( tAttr.m_eAttrType & SPH_ATTR_MULTI )!=0 );
	}

	// bind weights
	BindWeights ( pQuery );

	// setup lookup
	m_bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	m_bLateLookup = false;
	if ( ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !m_bEarlyLookup )
		for ( int iSorter=0; iSorter<iSorters && !m_bLateLookup; iSorter++ )
			if ( ppSorters[iSorter]->UsesAttrs() )
				m_bLateLookup = true;

	// setup sorters vs. MVA
	for ( int i=0; i<iSorters; i++ )
		(ppSorters[i])->SetMVAPool ( m_pMva.GetWritePtr() );

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	PROFILE_BEGIN ( query_match );
	bool bMatch = true;
	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:			MatchAll ( pQuery, pResult, iSorters, ppSorters ); break;
		case SPH_MATCH_PHRASE:		MatchAll ( pQuery, pResult, iSorters, ppSorters ); break;
		case SPH_MATCH_ANY:			MatchAny ( pQuery, pResult, iSorters, ppSorters ); break;
		case SPH_MATCH_BOOLEAN:		bMatch = MatchBoolean ( pQuery, pResult, iSorters, ppSorters, tTermSetup ); break;
		case SPH_MATCH_EXTENDED:	bMatch = MatchExtendedV1 ( pQuery, pResult, iSorters, ppSorters, tTermSetup ); break;
		case SPH_MATCH_EXTENDED2:	bMatch = MatchExtended ( pQuery, pResult, iSorters, ppSorters, tTermSetup ); break;
		case SPH_MATCH_FULLSCAN:	MatchFullScan ( pQuery, iSorters, ppSorters, tTermSetup ); break;
		default:					sphDie ( "INTERNAL ERROR: unknown matching mode (mode=%d)", pQuery->m_eMode );
	}
	PROFILE_END ( query_match );

	// check if there was error while matching (boolean or extended query parsing error, for one)
	if ( !bMatch )
		return false;

	if ( bSimpleQuery )
		ARRAY_FOREACH ( i, m_dQueryWords )
	{
		if ( m_dQueryWords[i].m_rdDoclist.GetErrorFlag() )
		{
			m_sLastError = m_dQueryWords[i].m_rdDoclist.GetErrorMessage ();
			return false;
		}
		if ( m_dQueryWords[i].m_rdHitlist.GetErrorFlag() )
		{
			m_sLastError = m_dQueryWords[i].m_rdHitlist.GetErrorMessage ();
			return false;
		}
	}

	m_dQueryWords.Reset ();

	////////////////////
	// cook result sets
	////////////////////

	// adjust result sets
	for ( int iSorter=0; iSorter<iSorters; iSorter++ )
	{
		ISphMatchSorter * pTop = ppSorters[iSorter];

		// final lookup
		if ( pTop->GetLength() && !m_bEarlyLookup && !m_bLateLookup )
		{
			const int iCount = pTop->GetLength ();
			CSphMatch * const pHead = pTop->First();
			CSphMatch * const pTail = pHead + iCount;

			for ( CSphMatch * pCur=pHead; pCur<pTail; pCur++ )
				LookupDocinfo ( *pCur );
		}

		// mva ptr
		pResult->m_pMva = m_pMva.GetWritePtr();
	}

	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_iQueryTime += int ( 1000.0f*( sphLongTimer() - tmQueryStart ) );
	return true;
}


/// morphology
enum
{
	 SPH_MORPH_STEM_EN
	,SPH_MORPH_STEM_RU_CP1251
	,SPH_MORPH_STEM_RU_UTF8
	,SPH_MORPH_SOUNDEX
	,SPH_MORPH_METAPHONE_SBCS
	,SPH_MORPH_METAPHONE_UTF8
	,SPH_MORPH_LIBSTEMMER_FIRST
	,SPH_MORPH_LIBSTEMMER_LAST = SPH_MORPH_LIBSTEMMER_FIRST + 64
};


/////////////////////////////////////////////////////////////////////////////
// CRC32/64 DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

/// CRC32/64 dictionary
struct CSphDictCRC : CSphDict
{
						CSphDictCRC ();
	virtual				~CSphDictCRC ();

	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen );
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer );
	virtual bool		LoadWordforms ( const char * szFile, ISphTokenizer * pTokenizer );
	virtual bool		SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError );
	virtual SphWordID_t	GetWordIDWithMarkers ( BYTE * pWord );
	virtual void		ApplyStemmers ( BYTE * pWord );

	static void			SweepWordformContainers ( const char * szFile );

protected:
	CSphVector < int >	m_dMorph;
#if USE_LIBSTEMMER
	CSphVector < sb_stemmer * >	m_dStemmers;
#endif

	int					m_iStopwords;	///< stopwords count
	SphWordID_t *		m_pStopwords;	///< stopwords ID list

protected:
	bool				ToNormalForm ( BYTE * pWord );
	bool				ParseMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError );
	SphWordID_t			FilterStopword ( SphWordID_t uID );	///< filter ID against stopwords list

private:
	typedef CSphOrderedHash < int, CSphString, CSphStrHashFunc, 1048576, 117 > CWordHash;

	struct WordformContainer
	{
		int							m_iRefCount;
		CSphString					m_sFilename;
		struct stat				m_Stat;
		CSphVector <CSphString>		m_dNormalForms;
		CWordHash					m_dHash;

									WordformContainer ();

		bool						IsEqual ( const char * szFile );
	};

	WordformContainer *	m_pWordforms;

	static CSphVector <WordformContainer*> m_dWordformContainers;

	static WordformContainer *	GetWordformContainer ( const char * szFile, const ISphTokenizer * pTokenizer );
	static WordformContainer *	LoadWordformContainer ( const char * szFile, const ISphTokenizer * pTokenizer );

	bool				InitMorph ( const char * szMorph, int iLength, bool bUseUTF8, CSphString & sError );
	bool				AddMorph ( int iMorph );
	bool				StemById ( BYTE * pWord, int iStemmer );
};

CSphVector <CSphDictCRC::WordformContainer*> CSphDictCRC::m_dWordformContainers;

/////////////////////////////////////////////////////////////////////////////

static const DWORD g_dSphinxCRC32 [ 256 ] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


DWORD sphCRC32 ( const BYTE * pString )
{
	// calc CRC
	DWORD crc = ~((DWORD)0);
	for ( const BYTE * p=pString; *p; p++ )
		crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ (*p)) & 0xff ];
	return ~crc;
}


DWORD sphCRC32 ( const BYTE * pString, int iLen )
{
	// calc CRC
	DWORD crc = ~((DWORD)0);
	for ( int i=0; i<iLen; i++ )
		crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ pString[i]) & 0xff ];
	return ~crc;
}

/////////////////////////////////////////////////////////////////////////////

uint64_t sphFNV64 ( const BYTE * s )
{
	uint64_t hval = 0xcbf29ce484222325ULL;
	while ( *s )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*s++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}


uint64_t sphFNV64 ( const BYTE * s, int iLen )
{
	uint64_t hval = 0xcbf29ce484222325ULL;
	for ( ; iLen>0; iLen-- )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*s++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}

/////////////////////////////////////////////////////////////////////////////

CSphDictCRC::WordformContainer::WordformContainer ()
	: m_iRefCount ( 0 )
{
}


bool CSphDictCRC::WordformContainer::IsEqual ( const char * szFile )
{
	if ( ! szFile )
		return false;

	struct stat FileStat;
	if ( stat ( szFile, &FileStat ) == -1 )
		return false;

	return m_sFilename == szFile && m_Stat.st_ctime == FileStat.st_ctime
		&& m_Stat.st_mtime == FileStat.st_mtime && m_Stat.st_size == FileStat.st_size;
}

/////////////////////////////////////////////////////////////////////////////

CSphDictCRC::CSphDictCRC ()
	: m_iStopwords	( 0 )
	, m_pStopwords	( NULL )
	, m_pWordforms	( NULL )
{
}


CSphDictCRC::~CSphDictCRC ()
{
#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers [i] );
#endif

	if ( m_pWordforms )
		--m_pWordforms->m_iRefCount;
}


SphWordID_t CSphDictCRC::FilterStopword ( SphWordID_t uID )
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


bool CSphDictCRC::ToNormalForm ( BYTE * pWord )
{
	if ( ! m_pWordforms )
		return false;

	if ( m_pWordforms )
	{
		int * pIndex = m_pWordforms->m_dHash ( (char *)pWord );
		if ( ! pIndex )
			return false;

		if ( *pIndex < 0 || *pIndex >= m_pWordforms->m_dNormalForms.GetLength () )
			return false;

		if ( m_pWordforms->m_dNormalForms [*pIndex].IsEmpty () )
			return false;

		strcpy ( (char *)pWord, m_pWordforms->m_dNormalForms [*pIndex].cstr () );

		return true;
	}

	return false;
}

bool CSphDictCRC::ParseMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError )
{
	const char * szStart = szMorph;

	while ( *szStart )
	{
		while ( *szStart && ( sphIsSpace ( *szStart ) || *szStart == ',' ) )
			++szStart;

		if ( !*szStart )
			break;

		const char * szWordStart = szStart;

		while ( *szStart && !sphIsSpace ( *szStart ) && *szStart != ',' )
			++szStart;

		if ( szStart - szWordStart > 0 )
		{
			if ( !InitMorph ( szWordStart, szStart - szWordStart, bUseUTF8, sError ) )
				return false;
		}
	}

	return true;
}


bool CSphDictCRC::InitMorph ( const char * szMorph, int iLength, bool bUseUTF8, CSphString & sError )
{
	if ( iLength == 0 )
		return true;

	if ( iLength == 4 && !strncmp ( szMorph, "none", iLength ) )
		return true;

	if ( iLength == 7 && !strncmp ( szMorph, "stem_en", iLength ) )
	{
		stem_en_init ();
		return AddMorph ( SPH_MORPH_STEM_EN );
	}
	
	if ( iLength == 7 && !strncmp ( szMorph, "stem_ru", iLength ) )
	{
		stem_ru_init ();
		return AddMorph ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
	}

	if ( iLength == 9 && !strncmp ( szMorph, "stem_enru", iLength ) )
	{
		stem_en_init ();
		stem_ru_init ();

		if ( ! AddMorph ( SPH_MORPH_STEM_EN ) )
			return false;

		return AddMorph ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
	}
	
	if ( iLength == 7 && !strncmp ( szMorph, "soundex", iLength ) )
		return AddMorph ( SPH_MORPH_SOUNDEX );

	if ( iLength==9 && !strncmp ( szMorph, "metaphone", iLength ) )
		return AddMorph ( bUseUTF8 ? SPH_MORPH_METAPHONE_UTF8 : SPH_MORPH_METAPHONE_SBCS );

	sError = "";

#if USE_LIBSTEMMER
	const int LIBSTEMMER_LEN = 11;
	const int MAX_ALGO_LENGTH = 64;
	if ( iLength > LIBSTEMMER_LEN && iLength - LIBSTEMMER_LEN < MAX_ALGO_LENGTH && !strncmp ( szMorph, "libstemmer_", LIBSTEMMER_LEN ) )
	{
		char szAlgo [MAX_ALGO_LENGTH];

		strncpy ( szAlgo, szMorph + LIBSTEMMER_LEN, iLength - LIBSTEMMER_LEN );
		szAlgo [iLength - LIBSTEMMER_LEN] = '\0';

		sb_stemmer * pStemmer = NULL;

		if ( bUseUTF8 )
			pStemmer = sb_stemmer_new ( szAlgo, "UTF_8" );
		else
		{
			pStemmer = sb_stemmer_new ( szAlgo, "ISO_8859_1" );

			if ( !pStemmer )
				pStemmer = sb_stemmer_new ( szAlgo, "ISO_8859_2" );

			if ( !pStemmer )
				pStemmer = sb_stemmer_new ( szAlgo, "KOI8_R" );
		}

		if ( !pStemmer )
		{
			sError.SetSprintf ( "libstemmer morphology algorithm '%s' not available for %s encoding - IGNORED",
				szAlgo, bUseUTF8 ? "UTF-8" : "SBCS" );
			return false;
		}

		AddMorph ( SPH_MORPH_LIBSTEMMER_FIRST + m_dStemmers.GetLength () );
		ARRAY_FOREACH ( i, m_dStemmers )
			if ( m_dStemmers [i] == pStemmer )
				return false;

		m_dStemmers.Add ( pStemmer );
		return true;
	}
#endif

	return false;
}


bool CSphDictCRC::AddMorph ( int iMorph )
{
	ARRAY_FOREACH ( i, m_dMorph )
		if ( m_dMorph [i] == iMorph )
			return false;

	m_dMorph.Add ( iMorph );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

template < typename T > T sphCRCWord ( const BYTE * pWord );
template<> uint64_t sphCRCWord ( const BYTE * pWord ) { return sphFNV64 ( pWord ); }
template<> DWORD sphCRCWord ( const BYTE * pWord ) { return sphCRC32 ( pWord ); }

template < typename T > T sphCRCWord ( const BYTE * pWord, int iLen );
template<> uint64_t sphCRCWord ( const BYTE * pWord, int iLen ) { return sphFNV64 ( pWord, iLen ); }
template<> DWORD sphCRCWord ( const BYTE * pWord, int iLen ) { return sphCRC32 ( pWord, iLen ); }


void CSphDictCRC::ApplyStemmers ( BYTE * pWord )
{
	if ( ! ToNormalForm ( pWord ) )
	{
		for ( int i = 0; i < m_dMorph.GetLength (); ++i )
			if ( StemById ( pWord, m_dMorph [i] ) )
				break;
	}
}


SphWordID_t CSphDictCRC::GetWordID ( BYTE * pWord )
{
	ApplyStemmers ( pWord );
	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord ) );
}


SphWordID_t CSphDictCRC::GetWordID ( const BYTE * pWord, int iLen )
{
	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord, iLen ) );
}


SphWordID_t	CSphDictCRC::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	SphWordID_t uWordId = sphCRCWord<SphWordID_t> ( pWord + 1 );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	int iLength = strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return sphCRCWord<SphWordID_t> ( pWord );
}


struct DwordCmp_fn
{
	inline int operator () ( DWORD b, DWORD a )
	{
		if ( a<b ) return -1;
		else if ( a==b ) return 0;
		else return 1;
	}
};


void CSphDictCRC::LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer )
{
	assert ( !m_pStopwords );
	assert ( !m_iStopwords );

	// tokenize file list
	if ( !sFiles || !*sFiles )
		return;

	char * sList = new char [ 1+strlen(sFiles) ];
	strcpy ( sList, sFiles );

	char * pCur = sList;
	char * sName = NULL;

	for ( ;; )
	{
		// find next name start
		while ( *pCur && isspace(*pCur) ) pCur++;
		if ( !*pCur ) break;
		sName = pCur;

		// find next name end
		while ( *pCur && !isspace(*pCur) ) pCur++;
		if ( *pCur ) *pCur++ = '\0';

		BYTE * pBuffer = NULL;

		// open file
		struct stat st;
		if ( stat ( sName, &st ) == 0 )
			pBuffer = new BYTE [st.st_size];
		else
		{
			sphWarn ( "stopwords: failed to get file size for '%s'", sName );
			continue;
		}

		FILE * fp = fopen ( sName, "rb" );
		if ( !fp )
		{
			sphWarn ( "failed to load stopwords from '%s'", sName );
			SafeDeleteArray ( pBuffer );
			continue;
		}

		// tokenize file
		CSphVector<SphWordID_t> dStop;
		int iLength = (int)fread ( pBuffer, 1, st.st_size, fp );

		BYTE * pToken;
		pTokenizer->SetBuffer ( pBuffer, iLength );
		while ( ( pToken = pTokenizer->GetToken() )!=NULL )
			dStop.Add ( GetWordID ( pToken ) );

		// sort stopwords
		dStop.Sort();

		// store IDs
		if ( dStop.GetLength() )
		{
			m_iStopwords = dStop.GetLength ();
			m_pStopwords = new SphWordID_t [ m_iStopwords ];
			memcpy ( m_pStopwords, &dStop[0], sizeof(SphWordID_t)*m_iStopwords );
		}

		// close file
		fclose ( fp );

		SafeDeleteArray ( pBuffer );
	}

	SafeDeleteArray ( sList );
}


void CSphDictCRC::SweepWordformContainers ( const char * szFile )
{
	for ( int i = 0; i < m_dWordformContainers.GetLength (); )
	{
		WordformContainer * WC = m_dWordformContainers [i];
		if ( WC->m_iRefCount == 0 && !WC->IsEqual ( szFile ) )
		{
			delete WC;
			m_dWordformContainers.Remove ( i );
		}
		else
			++i;
	}
}


CSphDictCRC::WordformContainer * CSphDictCRC::GetWordformContainer ( const char * szFile, const ISphTokenizer * pTokenizer )
{
	ARRAY_FOREACH ( i, m_dWordformContainers )
		if ( m_dWordformContainers [i]->IsEqual ( szFile ) )
			return m_dWordformContainers [i];

	WordformContainer * pContainer = LoadWordformContainer ( szFile, pTokenizer );
	if ( pContainer )
		m_dWordformContainers.Add ( pContainer );

	return pContainer;
}


CSphDictCRC::WordformContainer * CSphDictCRC::LoadWordformContainer ( const char * szFile, const ISphTokenizer * pTokenizer )
{
	// stat it; we'll store stats for later checks
	struct stat FileStat;
	if ( !szFile || !*szFile || stat ( szFile, &FileStat )<0 )
		return NULL;

	// allocate it
	WordformContainer * pContainer = new WordformContainer;
	if ( !pContainer )
		return NULL;
	pContainer->m_sFilename = szFile;
	pContainer->m_Stat = FileStat;

	// open it
	CSphString sError;
	CSphAutofile fdWordforms ( szFile, SPH_O_READ, sError );
	if ( fdWordforms.GetFD()<0 )
		return NULL;

	CSphReader_VLN rdWordforms;
	rdWordforms.SetFile ( fdWordforms );

	// my tokenizer
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone() );
	pMyTokenizer->AddSpecials ( ">" );

	// scan it line by line
	char sBuffer [ 6*SPH_MAX_WORD_LEN + 512 ]; // enough to hold 2 UTF-8 words, plus some whitespace overhead
	int iLen;
	while ( ( iLen=rdWordforms.GetLine ( sBuffer, sizeof(sBuffer) ) )>=0 )
	{
		// parse the line
		pMyTokenizer->SetBuffer ( (BYTE*)sBuffer, iLen );

		BYTE * pFrom = pMyTokenizer->GetToken ();
		if ( !pFrom ) continue; // FIXME! report parsing error

		CSphString sFrom ( (const char*)pFrom );
		const BYTE * pStart = pMyTokenizer->GetBufferPtr ();
		const BYTE * pCur = pStart;
		while ( isspace(*pCur) ) pCur++;
		if ( *pCur!='>' ) continue; // FIXME! report parsing error
		pMyTokenizer->AdvanceBufferPtr ( pCur+1-pStart ); // FIXME! replace with SetBufferPtr()

		BYTE * pTo = pMyTokenizer->GetToken ();
		if ( !pTo ) continue; // FIXME! report parsing error

		pContainer->m_dNormalForms.AddUnique ( (const char*)pTo );
		pContainer->m_dHash.Add ( pContainer->m_dNormalForms.GetLength()-1, sFrom.cstr() );
	}

	return pContainer;
}



bool CSphDictCRC::LoadWordforms ( const char * szFile, ISphTokenizer * pTokenizer )
{
	SweepWordformContainers ( szFile );
	m_pWordforms = GetWordformContainer ( szFile, pTokenizer );
	if ( m_pWordforms )
		m_pWordforms->m_iRefCount++;

	return !!m_pWordforms;
}


bool CSphDictCRC::SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError )
{
	m_dMorph.Reset ();

#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers [i] );

	m_dStemmers.Reset ();
#endif

	if ( !sMorph )
		return true;

	CSphString sOption = *sMorph;
	sOption.ToLower ();

	sError = "";
	if ( !ParseMorphology ( sOption.cstr (), bUseUTF8, sError ) )
	{
		m_dMorph.Reset ();

		if ( sError.IsEmpty () )
			sError.SetSprintf ( "invalid morphology option '%s' - IGNORED", sOption.cstr() );

		return false;
	}

	return true;
}

/// common id-based stemmer
bool CSphDictCRC::StemById ( BYTE * pWord, int iStemmer )
{
	char szBuf [4*SPH_MAX_WORD_LEN];
	strcpy ( szBuf, (char *)pWord );

	switch ( iStemmer )
	{
	case SPH_MORPH_STEM_EN:
		stem_en ( pWord );
		break;

	case SPH_MORPH_STEM_RU_CP1251:
		stem_ru_cp1251 ( pWord );
		break;

	case SPH_MORPH_STEM_RU_UTF8:
		stem_ru_utf8 ( (WORD*)pWord );
		break;

	case SPH_MORPH_SOUNDEX:
		stem_soundex ( pWord );
		break;

	case SPH_MORPH_METAPHONE_SBCS:
		stem_dmetaphone ( pWord, false );
		break;

	case SPH_MORPH_METAPHONE_UTF8:
		stem_dmetaphone ( pWord, true );
		break;

	default:
#if USE_LIBSTEMMER
		if ( iStemmer >= SPH_MORPH_LIBSTEMMER_FIRST && iStemmer < SPH_MORPH_LIBSTEMMER_LAST )
		{
			sb_stemmer * pStemmer = m_dStemmers [iStemmer - SPH_MORPH_LIBSTEMMER_FIRST];
			assert ( pStemmer );
			
			const sb_symbol * sStemmed = sb_stemmer_stem ( pStemmer, (sb_symbol*)pWord, strlen((const char*)pWord) );
			int iLen = sb_stemmer_length ( pStemmer );

			memcpy ( pWord, sStemmed, iLen );
			pWord[iLen] = '\0';
		}
		else
			return false;

	break;
#else
		return false;
#endif
	}

	return strcmp ( (char *)pWord, szBuf ) != 0;
}


CSphDict * sphCreateDictionaryCRC ( const CSphVariant * pMorph, const char * szStopwords, const char * szWordforms, ISphTokenizer * pTokenizer, CSphString & sError )
{
	assert ( pTokenizer );

	CSphDict * pDict = new CSphDictCRC ();
	if ( ! pDict  )
		return NULL;

	if ( pDict->SetMorphology ( pMorph, pTokenizer->IsUtf8(), sError ) )
		sError = "";

	pDict->LoadStopwords ( szStopwords, pTokenizer );
	pDict->LoadWordforms ( szWordforms, pTokenizer );

	return pDict;
}


void sphShutdownWordforms ()
{
	CSphDictCRC::SweepWordformContainers ( NULL );
}

/////////////////////////////////////////////////////////////////////////////
// HTML STRIPPER
/////////////////////////////////////////////////////////////////////////////

static inline int sphIsTag ( int c )
{
	return sphIsAlpha(c) || c=='.' || c==':';
}


CSphHTMLStripper::CSphHTMLStripper ()
{
	// known inline tags
	const char * dKnown[] =
	{
		"a", "b", "i", "s", "u",
		"basefont", "big", "em", "font", "img",
		"label", "small", "span", "strike", "strong",
		"sub", "sup", "tt"
	};

	m_dTags.Resize ( sizeof(dKnown)/sizeof(dKnown[0]) );
	ARRAY_FOREACH ( i, m_dTags )
	{
		m_dTags[i].m_sTag = dKnown[i];
		m_dTags[i].m_iTagLen = strlen(dKnown[i]);
		m_dTags[i].m_bInline = true;
	}

	UpdateTags ();
}


int CSphHTMLStripper::GetCharIndex ( int iCh )
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
			m_dTags.Resize ( m_dTags.GetLength()+1 );
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = strlen ( sTag );
			iIndexTag = m_dTags.GetLength()-1;
		}

		m_dTags[iIndexTag].m_bIndexAttrs = true;
		CSphVector<CSphString> & dAttrs = m_dTags[iIndexTag].m_dAttrs;

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
			m_dTags.Resize ( m_dTags.GetLength()+1 );
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = strlen ( sTag.cstr() );
			m_dTags.Last().m_bRemove = true;
		}
	}

	UpdateTags ();
	return true;
}


const BYTE * SkipQuoted ( const BYTE * p )
{
	BYTE cEnd = *p++; // either apostrophe or quote
	while ( *p && *p!=cEnd )
		p++;
	if ( *p==cEnd )
		p++;
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
		421, 421, 421, 421, 421, 421, 421, 421, 421,   4,
		  6,  22,   1, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 170,  48,   0,   5,  44,
		  0,  10,  10,  86, 421,   7,   0,   1,  42,  93,
		 41, 421,   0,   5,   8,  14, 421, 421,   5,  11,
		  8, 421, 421, 421, 421, 421, 421,   1,  25,  27,
		  9,   2, 113,  82,  14,   3, 179,   1,  81,  91,
		 12,   0,   1, 180,  56,  17,   5,  31,  60,   7,
		  3, 161,   2,   3, 421, 421, 421, 421, 421, 421,
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
		{""}, {""}, {""}, {""}, {""}, {""},
		{"Rho", 929},
		{""}, {""}, {""}, {""}, {""},
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
		{""},
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
		{""}, {""}, {""}, {""},
		{"scaron", 353},
		{""},
		{"notin", 8713},
		{"ndash", 8211},
		{""},
		{"acute", 180},
		{"otilde", 245},
		{"atilde", 227},
		{"Phi", 934},
		{""},
		{"Psi", 936},
		{"pound", 163},
		{"cap", 8745},
		{""},
		{"otimes", 8855},
		{""},
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
		{""},
		{"Uacute", 218},
		{"Eta", 919},
		{"ETH", 208},
		{"sup", 8835},
		{""},
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
		{""}, {""},
		{"sube", 8838},
		{"Eacute", 201},
		{"thetasym", 977},
		{""}, {""}, {""},
		{"Omega", 937},
		{"Ecirc", 202},
		{""},
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
		{""},
		{"Nu", 925},
		{""}, {""},
		{"ograve", 242},
		{"agrave", 224},
		{"egrave", 232},
		{"igrave", 236},
		{"frac14", 188},
		{"ordf", 170},
		{"Otilde", 213},
		{"infin", 8734},
		{""},
		{"frac12", 189},
		{"beta", 946},
		{"radic", 8730},
		{"darr", 8595},
		{"Iacute", 205},
		{"Ugrave", 217},
		{""}, {""},
		{"harr", 8596},
		{"hearts", 9829},
		{"Icirc", 206},
		{"Oacute", 211},
		{""},
		{"frac34", 190},
		{"cent", 162},
		{"crarr", 8629},
		{"curren", 164},
		{"Ocirc", 212},
		{"brvbar", 166},
		{"sect", 167},
		{""},
		{"ang", 8736},
		{"ugrave", 249},
		{""},
		{"Beta", 914},
		{"uarr", 8593},
		{"dArr", 8659},
		{"asymp", 8776},
		{"perp", 8869},
		{"Dagger", 8225},
		{""},
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
		{""}, {""}, {""},
		{"copy", 169},
		{"uArr", 8657},
		{"ni", 8715},
		{"rarr", 8594},
		{"le", 8804},
		{"ge", 8805},
		{"zwnj", 8204},
		{""},
		{"apos", 39},
		{"macr", 175},
		{"lang", 9001},
		{"gamma", 947},
		{"Delta", 916},
		{""},
		{"uml", 168},
		{"alefsym", 8501},
		{"delta", 948},
		{""},
		{"bdquo", 8222},
		{"lambda", 955},
		{"equiv", 8801},
		{""},
		{"Oslash", 216},
		{""},
		{"hellip", 8230},
		{""},
		{"rArr", 8658},
		{"Atilde", 195},
		{"larr", 8592},
		{"spades", 9824},
		{"Igrave", 204},
		{"Pi", 928},
		{"yacute", 253},
		{""},
		{"diams", 9830},
		{"sbquo", 8218},
		{"fnof", 402},
		{"Ograve", 210},
		{"plusmn", 177},
		{""},
		{"rceil", 8969},
		{"Aacute", 193},
		{"ouml", 246},
		{"auml", 228},
		{"euml", 235},
		{"iuml", 239},
		{""},
		{"Acirc", 194},
		{""},
		{"rdquo", 8221},
		{""},
		{"lArr", 8656},
		{"rsquo", 8217},
		{"Yuml", 376},
		{""},
		{"quot", 34},
		{"Uuml", 220},
		{"bull", 8226},
		{""}, {""}, {""},
		{"real", 8476},
		{""}, {""}, {""},
		{"lceil", 8968},
		{"permil", 8240},
		{"upsih", 978},
		{"sum", 8721},
		{""}, {""},
		{"divide", 247},
		{"raquo", 187},
		{"uuml", 252},
		{"ldquo", 8220},
		{"Alpha", 913},
		{"szlig", 223},
		{"lsquo", 8216},
		{""},
		{"Sigma", 931},
		{"tilde", 732},
		{""},
		{"THORN", 222},
		{""}, {""}, {""},
		{"Euml", 203},
		{"rfloor", 8971},
		{""},
		{"lrm", 8206},
		{""},
		{"sigma", 963},
		{"iexcl", 161},
		{""}, {""},
		{"deg", 176},
		{"middot", 183},
		{"laquo", 171},
		{""},
		{"circ", 710},
		{""}, {""}, {""}, {""},
		{"frasl", 8260},
		{"epsilon", 949},
		{"oplus", 8853},
		{"yen", 165},
		{"micro", 181},
		{"piv", 982},
		{""}, {""},
		{"lfloor", 8970},
		{""},
		{"Agrave", 192},
		{""}, {""},
		{"Upsilon", 933},
		{""}, {""},
		{"times", 215},
		{""},
		{"cedil", 184},
		{""}, {""}, {""}, {""},
		{"minus", 8722},
		{"Iuml", 207},
		{""}, {""}, {""}, {""}, {""},
		{"upsilon", 965},
		{"Ouml", 214},
		{""}, {""},
		{"rlm", 8207},
		{""}, {""}, {""},
		{"reg", 174},
		{""},
		{"forall", 8704},
		{""}, {""},
		{"Epsilon", 917},
		{"empty", 8709},
		{"OElig", 338},
		{""},
		{"shy", 173},
		{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
		{""}, {""}, {""}, {""},
		{"Aring", 197},
		{""}, {""}, {""},
		{"oelig", 339},
		{"aelig", 230},
		{""},
		{"zwj", 8205},
		{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
		{""}, {""}, {""}, {""}, {""},
		{"sim", 8764},
		{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
		{""}, {""}, {""}, {""}, {""}, {""},
		{"yuml", 255},
		{"sigmaf", 962},
		{""}, {""}, {""}, {""}, {""}, {""}, {""},
		{"Auml", 196},
		{""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
		{""}, {""}, {""}, {""},
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


void CSphHTMLStripper::Strip ( BYTE * sData )
{
	const BYTE * s = sData;
	BYTE * d = sData;
	for ( ;; )
	{
		// scan until eof, or tag, or entity
		while ( *s && *s!='<' && *s!='&' )
			*d++ = *s++;
		if ( !*s )
			break;

		// handle entity
		if ( *s=='&' )
		{
			if ( s[1]=='#' )
			{
				// handle "&#number;" form
				int iCode = 0;
				s += 2;
				while ( isdigit(*s) )
					iCode = iCode*10 + (*s++) - '0';

				if ( iCode==0 || *s!=';' )
					continue;

				d += sphUTF8Encode ( d, iCode );
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

		// handle tag
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
				// check if it's valid comment
				if (!( s[2]=='-' && s[3]=='-' ))
				{
					*d++ = *s++;
					continue;
				};

				// scan until comment end
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

			} else
			{
				// simply malformed
				*d++ = *s++;
				continue;
			}
		}
		s++; // skip '<'

		// lookup this tag in known tags list
		int iTag = -1;
		const BYTE * sTagName = ( s[0]=='/' ) ? s+1 : s;

		int iIdx = GetCharIndex ( sTagName[0] );
		assert ( iIdx>=0 && iIdx<MAX_CHAR_INDEX );

		if ( m_dEnd[iIdx]>=0 )
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
				if ( iCmp==0 && !sphIsTag(sTagName[iLen]) )
				{
					iTag = i;
					s = sTagName + iLen; // skip tag name
					assert ( !sphIsTag(*s) );
					break;
				}
			}
		}

		// skip unknown or do-not-index-attrs tag
		if ( iTag<0 || !m_dTags[iTag].m_bIndexAttrs )
		{
			// we just scan until EOLN or tag end
			while ( *s && *s!='>' )
			{
				if ( *s=='\'' || *s=='"' )
					s = SkipQuoted ( s );
				else
					s++;
			}
			if ( *s )
				s++;

			// unknown tag is done; others might require a bit more work
			if ( iTag<0 )
			{
				*d++ = ' '; // unknown tags are *not* inline by default
				continue;
			}
		}

		// work this known tag
		assert ( iTag>=0 );
		const StripperTag_t & tTag = m_dTags[iTag];

		// handle index-attrs
		if ( tTag.m_bIndexAttrs )
			while ( *s && *s!='>' )
		{
			// skip non-alphas
			while ( *s && *s!='>' )
			{
				if ( *s=='\'' || *s=='"' )
				{
					s = SkipQuoted ( s );
					while ( isspace(*s) ) s++;
				}
				if ( sphIsTag(*s) )
					break;
				s++;
			}
			if ( !sphIsTag(*s) )
			{
				if ( *s ) s++;
				break;
			}

			// check attribute name
			// OPTIMIZE! remove linear search
			int iAttr;
			for ( iAttr=0; iAttr<tTag.m_dAttrs.GetLength(); iAttr++ )
			{
				int iLen = strlen ( tTag.m_dAttrs[iAttr].cstr() );
				if ( strncasecmp ( tTag.m_dAttrs[iAttr].cstr(), (const char*)s, iLen ) )
					continue;
				if ( s[iLen]=='=' || isspace(s[iLen]) )
				{
					s += iLen;
					break;
				}
			}

			// if attribute is unknown or malformed, we just skip all alphas and rescan
			if ( iAttr==tTag.m_dAttrs.GetLength() )
			{
				while ( sphIsTag(*s) ) s++;
				continue;
			}

			// skip spaces, check for '=', and skip spaces again
			while ( isspace(*s) ) s++; if ( !*s ) break;
			if ( *s++!='=' ) break;
			while ( isspace(*s) ) s++;

			// handle quoted value
			if ( *s=='\'' || *s=='"' )
			{
				char cEnd = *s;
				s++;
				while ( *s && *s!=cEnd ) *d++ = *s++;
				if ( *s==cEnd ) s++;
				continue;
			}

			// handle unquoted value
			while ( *s && !isspace(*s) && *s!='>' ) *d++ = *s++;
		}
		if ( *s=='>' ) s++;

		// in all cases, the tag must be fully processed at this point
		// not a remove-tag? we're done
		if ( !tTag.m_bRemove )
		{
			if ( !tTag.m_bInline ) *d++ = ' ';
			continue;
		}

		// sudden eof? bail out
		if ( !*s )
			break;

		// must be a proper remove-tag end, then
		assert ( tTag.m_bRemove && s[-1]=='>' );

		// short-form? we're done
		if ( s[-2]=='/' )
			continue;

		// skip everything until the closing tag
		// FIXME! should we handle insane cases with quoted closing tag within tag?
		for ( ;; )
		{
			while ( *s && ( s[0]!='<' || s[1]!='/' ) ) s++;
			if ( !*s ) break;

			s += 2; // skip </
			if ( strncasecmp ( tTag.m_sTag.cstr(), (const char*)s, tTag.m_iTagLen )!=0 ) continue;
			if ( !sphIsTag(s[tTag.m_iTagLen]) )
			{
				s += tTag.m_iTagLen; // skip tag
				if ( *s=='>' ) s++;
				break;
			}
		}

		if ( !tTag.m_bInline ) *d++ = ' ';
	}

	*d++ = '\0';
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource ( const char * sName )
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_tSchema ( sName )
	, m_bStripHTML ( false )
	, m_iMinPrefixLen ( 0 )
	, m_iMinInfixLen ( 0 )
	, m_iBoundaryStep ( 0 )
{
	m_pStripper = new CSphHTMLStripper ();
}


CSphSource::~CSphSource()
{
	delete m_pStripper;
}


void CSphSource::SetDict ( CSphDict * pDict )
{
	assert ( pDict );
	m_pDict = pDict;
}


const CSphSourceStats & CSphSource::GetStats ()
{
	return m_tStats;
}


bool CSphSource::SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements, CSphString & sError )
{
	m_bStripHTML = ( sExtractAttrs!=NULL );
	if ( !m_bStripHTML )
		return false;

	return m_pStripper->SetIndexedAttrs ( sExtractAttrs, sError )
		&& m_pStripper->SetRemovedElements ( sRemoveElements, sError );
}


void CSphSource::SetTokenizer ( ISphTokenizer * pTokenizer)
{
	assert ( pTokenizer );
	m_pTokenizer = pTokenizer;
}


bool CSphSource::UpdateSchema ( CSphSchema * pInfo, CSphString & sError )
{
	assert ( pInfo );

	// fill it
	if ( pInfo->m_dFields.GetLength()==0 && pInfo->GetAttrsCount()==0 )
	{
		*pInfo = m_tSchema;
		return true;
	}

	// check it
	CSphString sCompError;
	ESphSchemaCompare eComp = m_tSchema.CompareTo ( *pInfo, sCompError );
	if ( eComp==SPH_SCHEMAS_INCOMPATIBLE )
	{
		sError.SetSprintf ( "incompatible schemas: %s", sCompError.cstr() );
		return false;
	}

	// FIXME!!! warn if schemas are compatible but not equal!
	return true;
}


void CSphSource::SetEmitInfixes ( int iMinPrefixLen, int iMinInfixLen )
{
	m_iMinPrefixLen = Max ( iMinPrefixLen, 0 );
	m_iMinInfixLen = Max ( iMinInfixLen, 0 );
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

bool CSphSource_Document::IterateHitsNext ( CSphString & sError )
{
	assert ( m_pTokenizer );
	PROFILE ( src_document );

	BYTE ** dFields = NextDocument ( sError );
	if ( m_tDocInfo.m_iDocID==0 )
		return true;
	if ( !dFields )
		return false;

	m_tStats.m_iTotalDocuments++;
	m_dHits.Reserve ( 1024 );
	m_dHits.Resize ( 0 );

	bool bGlobalPartialMatch = m_iMinPrefixLen > 0 || m_iMinInfixLen > 0;

	for ( int j=0; j<m_tSchema.m_dFields.GetLength(); j++ )
	{
		BYTE * sField = dFields[j];
		if ( !sField )
			continue;

		if ( m_bStripHTML )
			m_pStripper->Strip ( sField );

		int iFieldBytes = (int) strlen ( (char*)sField );
		m_tStats.m_iTotalBytes += iFieldBytes;

		m_pTokenizer->SetBuffer ( sField, iFieldBytes );

		BYTE * sWord;
		int iPos = ( j<<24 ) + 1;

		bool bPrefixField = m_tSchema.m_dFields [j].m_eWordpart == SPH_WORDPART_PREFIX;
		bool bInfixMode = m_iMinInfixLen > 0;

		BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

		if ( m_tSchema.m_dFields [j].m_eWordpart != SPH_WORDPART_WHOLE )
		{
			int iMinInfixLen = bPrefixField ? m_iMinPrefixLen : m_iMinInfixLen;

			// index all infixes
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				if ( m_pTokenizer->GetBoundary() )
					iPos += m_iBoundaryStep;

				int iLen = m_pTokenizer->GetLastTokenLen ();

				// always index full word (with magic head/tail marker(s))
				int iBytes = strlen ( (const char*)sWord );
				memcpy ( sBuf + 1, sWord, iBytes );
				sBuf [0]			= MAGIC_WORD_HEAD;
				sBuf [iBytes + 1]	= '\0';

				// stemmed word w/markers
				SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
				if ( iWord )
				{
					CSphWordHit & tHit = m_dHits.Add ();
					tHit.m_iDocID = m_tDocInfo.m_iDocID;
					tHit.m_iWordID = iWord;
					tHit.m_iWordPos = iPos;
				}

				// restore stemmed word
				int iStemmedLen = strlen ( ( const char *)sBuf );
				sBuf [iStemmedLen - 1] = '\0';

				// stemmed word w/o markers
				if ( strcmp ( (const char *)sBuf + 1, (const char *)sWord ) )
				{
					SphWordID_t iWord = m_pDict->GetWordID ( sBuf + 1, iStemmedLen - 2 );
					if ( iWord )
					{
						CSphWordHit & tHit = m_dHits.Add ();
						tHit.m_iDocID = m_tDocInfo.m_iDocID;
						tHit.m_iWordID = iWord;
						tHit.m_iWordPos = iPos;
					}
				}

				// restore word
				memcpy ( sBuf + 1, sWord, iBytes );
				sBuf [iBytes + 1]	= MAGIC_WORD_TAIL;
				sBuf [iBytes + 2]	= '\0';

				// if there are no infixes, that's it
				if ( iMinInfixLen > iLen )
				{
					// index full word
					SphWordID_t iWord = m_pDict->GetWordID ( sWord );
					if ( iWord )
					{
						CSphWordHit & tHit = m_dHits.Add ();
						tHit.m_iDocID = m_tDocInfo.m_iDocID;
						tHit.m_iWordID = iWord;
						tHit.m_iWordPos = iPos;
					}

					iPos++;
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

					for ( int i=iMinInfixLen; i<=iLen-iStart; i++ )
					{
						SphWordID_t iWord = m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix );

						if ( iWord )
						{
							CSphWordHit & tHit = m_dHits.Add ();
							tHit.m_iDocID = m_tDocInfo.m_iDocID;
							tHit.m_iWordID = iWord;
							tHit.m_iWordPos = iPos;
						}

						// word start: add magic head
						if ( bInfixMode && iStart==0 )
						{
							iWord = m_pDict->GetWordID ( sInfix - 1, sInfixEnd-sInfix + 1 );
							if ( iWord )
							{
								CSphWordHit & tHit = m_dHits.Add ();
								tHit.m_iDocID = m_tDocInfo.m_iDocID;
								tHit.m_iWordID = iWord;
								tHit.m_iWordPos = iPos;
							}
						}

						// word end: add magic tail
						if ( bInfixMode && i==iLen-iStart )
						{
							iWord = m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix+1 );
							if ( iWord )
							{
								CSphWordHit & tHit = m_dHits.Add ();
								tHit.m_iDocID = m_tDocInfo.m_iDocID;
								tHit.m_iWordID = iWord;
								tHit.m_iWordPos = iPos;
							}
						}

						sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );
					}

					sInfix += m_pTokenizer->GetCodepointLength ( *sInfix );
				}

				// move on
				iPos++; 
			}
		} else
		{
			// index words only
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				if ( m_pTokenizer->GetBoundary() )
					iPos += m_iBoundaryStep;

				if ( bGlobalPartialMatch )
				{
					int iBytes = strlen ( (const char*)sWord );
					memcpy ( sBuf + 1, sWord, iBytes );
					sBuf [0]			= MAGIC_WORD_HEAD;
					sBuf [iBytes + 1]	= '\0';

					SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
					if ( iWord )
					{
						CSphWordHit & tHit = m_dHits.Add ();
						tHit.m_iDocID = m_tDocInfo.m_iDocID;
						tHit.m_iWordID = iWord;
						tHit.m_iWordPos = iPos;
					}
				}

				SphWordID_t iWord = m_pDict->GetWordID ( sWord );
				if ( iWord )
				{
					CSphWordHit & tHit = m_dHits.Add ();
					tHit.m_iDocID = m_tDocInfo.m_iDocID;
					tHit.m_iWordID = iWord;
					tHit.m_iWordPos = iPos;
				}
				iPos++;
			}
		}
	}

	return true;
}


void CSphSource_Document::SetupFieldMatch ( const char * szPrefixFields, const char * szInfixFields )
{
	m_sPrefixFields = szPrefixFields;
	m_sInfixFields = szInfixFields;

	m_sPrefixFields.ToLower ();
	m_sInfixFields.ToLower ();
}


bool CSphSource_Document::IsPrefixMatch ( const char * szField ) const
{
	return IsFieldInStr ( szField, m_sPrefixFields.cstr () );
}


bool CSphSource_Document::IsInfixMatch ( const char * szField ) const
{
	return IsFieldInStr ( szField, m_sInfixFields.cstr () );
}


void CSphSource_Document::ParseFieldMVA ( CSphVector < CSphVector < DWORD > >  & dFieldMVAs, int iFieldMVA, const char * szValue )
{
	dFieldMVAs [iFieldMVA].Resize ( 0 );

	if ( !szValue )
		return;

	const char * pPtr = szValue;
	const char * pDigit = NULL;
	const int MAX_NUMBER_LEN = 64;
	char szBuf [MAX_NUMBER_LEN];

	while ( *pPtr )
	{
		if ( *pPtr >= '0' && *pPtr <= '9' )
		{
			if ( !pDigit )
				pDigit = pPtr;
		}
		else
		{
			if ( pDigit )
			{
				if ( pPtr - pDigit < MAX_NUMBER_LEN )
				{
					strncpy ( szBuf, pDigit, pPtr - pDigit );
					szBuf [pPtr - pDigit] = '\0';
					dFieldMVAs [iFieldMVA].Add ( sphToDword ( szBuf ) );
				}

				pDigit = NULL;
			}
		}
	
		pPtr++;
	}

	if ( pDigit )
		dFieldMVAs [iFieldMVA].Add ( sphToDword ( pDigit ) );
}


bool CSphSource_Document::IsFieldInStr ( const char * szField, const char * szString ) const
{
	if ( !szString || szString [0] == '\0' )
		return true;

	if  ( !szField || szField [0] == '\0' )
		return false;

	const char * szPos = strstr ( szString, szField );
	if ( ! szPos )
		return false;

	int iFieldLen = strlen ( szField );
	bool bStartOk = szPos == szString || ! sphIsAlpha ( *(szPos - 1) );
	bool bEndOk = !*(szPos + iFieldLen) || ! sphIsAlpha ( *(szPos + iFieldLen) );

	return bStartOk && bEndOk;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SQL SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSourceParams_SQL::CSphSourceParams_SQL ()
	: m_iRangeStep ( 1024 )
	, m_iRangedThrottle ( 0 )
	, m_iPort ( 0 )
{
}


const char * const CSphSource_SQL::MACRO_VALUES [ CSphSource_SQL::MACRO_COUNT ] =
{
	"$start",
	"$end"
};


CSphSource_SQL::CSphSource_SQL ( const char * sName )
	: CSphSource_Document	( sName )
	, m_bSqlConnected		( false )
	, m_uMinID				( 0 )
	, m_uMaxID				( 0 )
	, m_uCurrentID			( 0 )
	, m_uMaxFetchedID		( 0 )
	, m_iMultiAttr			( -1 )
	, m_iFieldMVA			( 0 )
	, m_iFieldMVAIterator	( 0 )
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
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "sql://%s:***@%s:%d/%s",
		m_tParams.m_sUser.cstr(), m_tParams.m_sHost.cstr(),
		m_tParams.m_iPort, m_tParams.m_sDB.cstr() );
	m_sSqlDSN = sBuf;

	return true;
}


bool CSphSource_SQL::RunQueryStep ( CSphString & sError )
{
	if ( m_tParams.m_iRangeStep<=0 )
		return false;
	if ( m_uCurrentID>m_uMaxID )
		return false;

	static const int iBufSize = 32;
	char * sRes = NULL;

	sphUsleep ( m_tParams.m_iRangedThrottle );

	//////////////////////////////////////////////
	// range query with $start/$end interpolation
	//////////////////////////////////////////////

	assert ( m_uMinID>0 );
	assert ( m_uMaxID>0 );
	assert ( m_uMinID<=m_uMaxID );
	assert ( m_tParams.m_sQuery.cstr() );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	SphDocID_t uNextID = Min ( m_uCurrentID + m_tParams.m_iRangeStep - 1, m_uMaxID );
	snprintf ( sValues[0], iBufSize, DOCID_FMT, m_uCurrentID );
	snprintf ( sValues[1], iBufSize, DOCID_FMT, uNextID );
	m_uCurrentID = 1 + uNextID;

	// OPTIMIZE? things can be precalculated
	const char * sCur = m_tParams.m_sQuery.cstr();
	int iLen = 0;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<MACRO_COUNT; i++ )
				if ( strncmp ( MACRO_VALUES[i], sCur, strlen(MACRO_VALUES[i]) )==0 )
			{
				sCur += strlen ( MACRO_VALUES[i] );
				iLen += strlen ( sValues[i] );
				break;
			}
			if ( i<MACRO_COUNT )
				continue;
		}

		sCur++;
		iLen++;
	}
	iLen++; // trailing zero

	// do interpolation
	sRes = new char [ iLen ];
	sCur = m_tParams.m_sQuery.cstr();

	char * sDst = sRes;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<MACRO_COUNT; i++ )
				if ( strncmp ( MACRO_VALUES[i], sCur, strlen(MACRO_VALUES[i]) )==0 )
			{
				strcpy ( sDst, sValues[i] );
				sCur += strlen ( MACRO_VALUES[i] );
				sDst += strlen ( sValues[i] );
				break;
			}
			if ( i<MACRO_COUNT )
				continue;
		}
		*sDst++ = *sCur++;
	}
	*sDst++ = '\0';
	assert ( sDst-sRes==iLen );

	// run query
	SqlDismissResult ();
	bool bRes = SqlQuery ( sRes );

	if ( !bRes )
		sError.SetSprintf ( "sql_range_query: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );

	SafeDeleteArray ( sRes );
	return bRes;
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

	// all good
	m_bSqlConnected = true;
	return true;
}


#define LOC_ERROR(_msg,_arg)			{ sError.SetSprintf ( _msg, _arg ); return false; }
#define LOC_ERROR2(_msg,_arg,_arg2)		{ sError.SetSprintf ( _msg, _arg, _arg2 ); return false; }
#define LOC_SQL_ERROR(_msg)				{ sSqlError = _msg; break; }

/// issue main rows fetch query
bool CSphSource_SQL::IterateHitsStart ( CSphString & sError )
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

	const char * sSqlError = NULL;
	for ( ;; )
	{
		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			///////////////
			// range query
			///////////////

			// check step
			if ( m_tParams.m_iRangeStep<=0 )
				LOC_ERROR ( "sql_range_step=%d: must be positive", m_tParams.m_iRangeStep );

			if ( m_tParams.m_iRangeStep<128 )
			{
				sphWarn ( "sql_range_step=%d: too small; increased to 128", m_tParams.m_iRangeStep );
				m_tParams.m_iRangeStep = 128;
			}

			// check query for macros
			for ( int i=0; i<MACRO_COUNT; i++ )
				if ( !strstr ( m_tParams.m_sQuery.cstr(), MACRO_VALUES[i] ) )
					LOC_ERROR ( "sql_query: macro '%s' not found", MACRO_VALUES[i] );

			// run query
			if ( !SqlQuery ( m_tParams.m_sQueryRange.cstr() ) )
				LOC_SQL_ERROR ( "sql_query_range" );

			// fetch min/max
			int iCols = SqlNumFields ();
			if ( iCols!=2 )
				LOC_ERROR ( "sql_query_range: expected 2 columns (min_id/max_id), got %d", iCols );

			if ( !SqlFetchRow() )
				LOC_SQL_ERROR ( "sql_query_range: fetch_row" );

			if ( SqlColumn(0)==NULL && SqlColumn(1)==NULL )
			{
				// the source seems to be empty; workaround
				m_uMinID = 1;
				m_uMaxID = 1;

			} else
			{
				// get and check min/max id
				const char * sCol0 = SqlColumn(0);
				const char * sCol1 = SqlColumn(1);
				m_uMinID = sphToDocid ( sCol0 );
				m_uMaxID = sphToDocid ( sCol1 );
				if ( !sCol0 ) sCol0 = "(null)";
				if ( !sCol1 ) sCol1 = "(null)";

				if ( m_uMinID<=0 )
					LOC_ERROR ( "sql_query_range: min_id='%s': must be positive 32-bit unsigned integer", sCol0 );
				if ( m_uMaxID<=0 )
					LOC_ERROR ( "sql_query_range: max_id='%s': must be positive 32-bit unsigned integer", sCol1 );
				if ( m_uMinID>m_uMaxID )
					LOC_ERROR2 ( "sql_query_range: min_id='%s', max_id='%s': min_id must be less than max_id", sCol0, sCol1 );
			}

			SqlDismissResult ();

			// issue query
			m_uCurrentID = m_uMinID;
			if ( !RunQueryStep ( sError ) )
				return false;

		} else
		{
			////////////////
			// normal query
			////////////////

			if ( !SqlQuery ( m_tParams.m_sQuery.cstr() ) )
				LOC_SQL_ERROR ( "sql_query" );

			m_tParams.m_iRangeStep = 0;
		}
		break;
	}

	// errors, anyone?
	if ( sSqlError )
	{
		sError.SetSprintf ( "%s: %s (DSN=%s)", sSqlError, SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	// some post-query setup
	m_tSchema.Reset();

	int iCols = SqlNumFields() - 1; // skip column 0, which must be the id

	CSphVector<bool> dFound;
	dFound.Resize ( m_tParams.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, dFound )
		dFound[i] = false;

	// map plain attrs from SQL
	for ( int i=0; i<iCols; i++ )
	{
		const char * sName = SqlFieldName(i+1);
		if ( !sName )
			LOC_ERROR ( "column number %d has no name", i+1 );

		CSphColumnInfo tCol ( sName );
		ARRAY_FOREACH ( j, m_tParams.m_dAttrs )
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dAttrs[j].m_sName.cstr() ) )
		{
			const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[j];

			tCol.m_eAttrType = tAttr.m_eAttrType;
			assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );

			if ( ( tAttr.m_eAttrType & SPH_ATTR_MULTI ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
				LOC_ERROR ( "multi-valued attribute '%s' of wrong source-type found in query; must be 'field'", tAttr.m_sName.cstr() );

			tCol = tAttr;
			dFound[j] = true;
			break;
		}

		tCol.m_iIndex = i+1;
		tCol.m_eWordpart = SPH_WORDPART_WHOLE;

		bool bPrefix = m_iMinPrefixLen > 0 && IsPrefixMatch ( tCol.m_sName.cstr () );
		bool bInfix =  m_iMinInfixLen > 0  && IsInfixMatch ( tCol.m_sName.cstr () );

		if ( bPrefix && m_iMinPrefixLen > 0 && bInfix && m_iMinInfixLen > 0)
			LOC_ERROR ( "field '%s' is marked for both infix and prefix indexing", tCol.m_sName.cstr() );
			
		if ( bPrefix )
			tCol.m_eWordpart = SPH_WORDPART_PREFIX;

		if ( bInfix )
			tCol.m_eWordpart = SPH_WORDPART_INFIX;

		if ( tCol.m_eAttrType==SPH_ATTR_NONE )
			m_tSchema.m_dFields.Add ( tCol );
		else
			m_tSchema.AddAttr ( tCol );
	}

	// map multi-valued attrs
	ARRAY_FOREACH ( i, m_tParams.m_dAttrs )
	{
		const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[i];
		if ( ( tAttr.m_eAttrType & SPH_ATTR_MULTI ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			m_tSchema.AddAttr ( tAttr );
			dFound[i] = true;
		}
	}

	// warn if some attrs went unmapped
	ARRAY_FOREACH ( i, dFound )
		if ( !dFound[i] )
			sphWarn ( "attribute '%s' not found - IGNORING", m_tParams.m_dAttrs[i].m_sName.cstr() );

	m_iFieldMVA = 0;
	m_iFieldMVAIterator = 0;
	m_dAttrToFieldMVA.Resize ( 0 );

	int iFieldMVA = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount (); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr ( i );
		if ( ( tCol.m_eAttrType & SPH_ATTR_MULTI ) && tCol.m_eSrc == SPH_ATTRSRC_FIELD )
			m_dAttrToFieldMVA.Add ( iFieldMVA++ );
		else
			m_dAttrToFieldMVA.Add ( -1 );
	}

	m_dFieldMVAs.Resize ( iFieldMVA );
	ARRAY_FOREACH ( i, m_dFieldMVAs )
		m_dFieldMVAs [i].Reserve ( 16 );

	// alloc storage
	m_tDocInfo.Reset ( m_tSchema.GetRowSize() );
	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );

	// check it
	if ( m_tSchema.m_dFields.GetLength()>SPH_MAX_FIELDS )
		LOC_ERROR2 ( "too many fields (fields=%d, max=%d); raise SPH_MAX_FIELDS in sphinx.h and rebuild",
			m_tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );

	return true;
}

#undef LOC_ERROR
#undef LOC_ERROR2
#undef LOC_SQL_ERROR


void CSphSource_SQL::Disconnect ()
{
	if ( m_bSqlConnected )
		SqlDisconnect ();
	m_bSqlConnected = false;
}


BYTE ** CSphSource_SQL::NextDocument ( CSphString & sError )
{
	PROFILE ( src_sql );
	assert ( m_bSqlConnected );

	// get next non-zero-id row
	do
	{
		// try to get next row
		bool bGotRow = SqlFetchRow ();

		// when the party's over...
		while ( !bGotRow )
		{
			// is that an error?
			if ( SqlIsError() )
			{
				sError.SetSprintf ( "sql_fetch_row: %s", SqlError() );
				m_tDocInfo.m_iDocID = 1; // 0 means legal eof
				return NULL;
			}

			// maybe we can do next step yet?
			if ( RunQueryStep ( sError ) )
			{
				bGotRow = SqlFetchRow ();
				continue;
			}

			// ok, we're over
			ARRAY_FOREACH ( i, m_tParams.m_dQueryPost )
			{
				if ( !SqlQuery ( m_tParams.m_dQueryPost[i].cstr() ) )
				{
					sphWarn ( "sql_query_post[%d]: error=%s, query=%s",
						i, SqlError(), m_tParams.m_dQueryPost[i].cstr() );
					break;
				}
				SqlDismissResult ();
			}

			m_tDocInfo.m_iDocID = 0; // 0 means legal eof
			return NULL;
		}

		// get him!
		m_tDocInfo.m_iDocID = sphToDocid ( SqlColumn(0) );
		m_uMaxFetchedID = Max ( m_uMaxFetchedID, m_tDocInfo.m_iDocID );

		if ( !m_tDocInfo.m_iDocID )
			sphWarn ( "zero/NULL document_id, skipping" );

	} while ( !m_tDocInfo.m_iDocID );

	// cleanup attrs
	for ( int i=0; i<m_tDocInfo.m_iRowitems; i++ )
		m_tDocInfo.m_pRowitems[i] = 0;

	// split columns into fields and attrs
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		m_dFields[i] = (BYTE*) SqlColumn ( m_tSchema.m_dFields[i].m_iIndex );

	int iFieldMVA = 0;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i); // shortcut

		if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
		{
			m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, 0 );
			if ( tAttr.m_eSrc == SPH_ATTRSRC_FIELD )
				ParseFieldMVA ( m_dFieldMVAs, iFieldMVA++, SqlColumn ( tAttr.m_iIndex ) );

			continue;
		}

		switch ( tAttr.m_eAttrType )
		{
			case SPH_ATTR_ORDINAL:
				// memorize string, fixup NULLs
				m_dStrAttrs[i] = SqlColumn ( tAttr.m_iIndex );
				if ( !m_dStrAttrs[i].cstr() )
					m_dStrAttrs[i] = "";

				m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, 0 );
				break;

			case SPH_ATTR_FLOAT:
				m_tDocInfo.SetAttrFloat ( tAttr.m_iRowitem,
					sphToFloat ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
				break;

			default:
				// just store as uint by default
				m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount,
					sphToDword ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
				break;
		}
	}

	return m_dFields;
}


void CSphSource_SQL::PostIndex ()
{
	if ( !m_tParams.m_dQueryPostIndex.GetLength() || !m_uMaxFetchedID )
		return;

	assert ( !m_bSqlConnected );

	#define LOC_SQL_ERROR(_msg) { sSqlError= _msg; break; }

	const char * sSqlError = NULL;
	for ( ;; )
	{
		if ( !SqlConnect () )
			LOC_SQL_ERROR ( "mysql_real_connect" );

		ARRAY_FOREACH ( i, m_tParams.m_dQueryPostIndex )
		{
			char * sQuery = sphStrMacro ( m_tParams.m_dQueryPostIndex[i].cstr(), "$maxid", m_uMaxFetchedID );
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


bool CSphSource_SQL::IterateMultivaluedStart ( int iAttr, CSphString & sError )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	m_iMultiAttr = iAttr;
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);

	if ( !(tAttr.m_eAttrType & SPH_ATTR_MULTI) )
		return false;

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
		if ( SqlNumFields()!=2 )
		{
			sError.SetSprintf ( "multi-valued attr '%s' query returned %d fields (expected 2)", tAttr.m_sName.cstr(), SqlNumFields() );
			SqlDismissResult ();
			return false;
		}
		return true;

	case SPH_ATTRSRC_RANGEDQUERY:
		// !COMMIT support ranged as well
		return false;

	default:
		sError.SetSprintf ( "INTERNAL ERROR: unknown multi-valued attr source type %d", tAttr.m_eSrc );
		return false;
	}
}


bool CSphSource_SQL::IterateMultivaluedNext ()
{
	assert ( m_bSqlConnected );
	assert ( m_tSchema.GetAttr(m_iMultiAttr).m_eAttrType & SPH_ATTR_MULTI );

	// fetch next row
	bool bGotRow = SqlFetchRow ();
	while ( !bGotRow )
	{
		if ( SqlIsError() )
			sphDie ( "sql_fetch_row: %s", SqlError() );

		// !COMMIT run next ranged step here
		return false;
	}

	// return that tuple
	int iRowitem = m_tSchema.GetAttr(m_iMultiAttr).m_iRowitem;
	m_tDocInfo.m_iDocID = sphToDocid ( SqlColumn(0) );
	m_tDocInfo.m_pRowitems[iRowitem] = sphToDword ( SqlColumn(1) );
	return true;
}


bool CSphSource_SQL::IterateFieldMVAStart ( int iAttr, CSphString & )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	if ( m_dAttrToFieldMVA [iAttr] == -1 )
		return false;

	m_iFieldMVA = iAttr;
	m_iFieldMVAIterator = 0;

	return true;
}


bool CSphSource_SQL::IterateFieldMVANext ()
{
	int iFieldMVA = m_dAttrToFieldMVA [m_iFieldMVA];
	if ( m_iFieldMVAIterator >= m_dFieldMVAs [iFieldMVA].GetLength () )
		return false;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iFieldMVA );
	m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, m_dFieldMVAs [iFieldMVA][m_iFieldMVAIterator] );

	++m_iFieldMVAIterator;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_MYSQL

CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
	: m_iFlags ( 0 )
{
	m_iPort = 3306;
}


CSphSource_MySQL::CSphSource_MySQL ( const char * sName )
	: CSphSource_SQL ( sName )
	, m_pMysqlResult( NULL )
	, m_pMysqlFields( NULL )
	, m_tMysqlRow	( NULL )
{
}


void CSphSource_MySQL::SqlDismissResult ()
{
	if ( !m_pMysqlResult )
		return;

	mysql_free_result ( m_pMysqlResult );
	m_pMysqlResult = NULL;
	m_pMysqlFields = NULL;
}


bool CSphSource_MySQL::SqlQuery ( const char * sQuery )
{
	if ( mysql_query ( &m_tMysqlDriver, sQuery ) )
		return false;

	m_pMysqlResult = mysql_use_result ( &m_tMysqlDriver );
	m_pMysqlFields = NULL;
	return true;
}


bool CSphSource_MySQL::SqlIsError ()
{
	return mysql_errno ( &m_tMysqlDriver )!=0;
}


const char * CSphSource_MySQL::SqlError ()
{
	return mysql_error ( &m_tMysqlDriver );
}


bool CSphSource_MySQL::SqlConnect ()
{
	mysql_init ( &m_tMysqlDriver );
	return NULL!=mysql_real_connect ( &m_tMysqlDriver,
		m_tParams.m_sHost.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr(),
		m_tParams.m_sDB.cstr(), m_tParams.m_iPort, m_sMysqlUsock.cstr(), m_iMysqlConnectFlags );
}


void CSphSource_MySQL::SqlDisconnect ()
{
	mysql_close ( &m_tMysqlDriver );
}


int CSphSource_MySQL::SqlNumFields ()
{
	if ( !m_pMysqlResult )
		return -1;

	return mysql_num_fields ( m_pMysqlResult );
}


bool CSphSource_MySQL::SqlFetchRow ()
{
	if ( !m_pMysqlResult )
		return false;

	m_tMysqlRow = mysql_fetch_row ( m_pMysqlResult );
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
		m_pMysqlFields = mysql_fetch_fields ( m_pMysqlResult );

	return m_pMysqlFields[iIndex].name;
}


bool CSphSource_MySQL::Setup ( const CSphSourceParams_MySQL & tParams )
{
	if ( !CSphSource_SQL::Setup ( tParams ) )
		return false;

	m_sMysqlUsock = tParams.m_sUsock;
	m_iMysqlConnectFlags = tParams.m_iFlags;

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

CSphSourceParams_PgSQL::CSphSourceParams_PgSQL ()
{
	m_iRangeStep = 1024;
	m_iPort = 5432;
}


CSphSource_PgSQL::CSphSource_PgSQL ( const char * sName )
	: CSphSource_SQL	( sName )
	, m_pPgResult		( NULL )
	, m_iPgRows			( 0 )
	, m_iPgRow			( 0 )
{
}


bool CSphSource_PgSQL::SqlIsError ()
{
	return ( m_iPgRow<m_iPgRows ); // if we're over, it's just last row
}


const char * CSphSource_PgSQL::SqlError ()
{
	return PQerrorMessage ( m_tPgDriver );
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


bool CSphSource_PgSQL::SqlConnect ()
{
	char sPort[64];
	snprintf ( sPort, sizeof(sPort), "%d", m_tParams.m_iPort );
	m_tPgDriver = PQsetdbLogin ( m_tParams.m_sHost.cstr(), sPort, NULL, NULL,
		m_tParams.m_sDB.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr() );

	if ( PQstatus ( m_tPgDriver )==CONNECTION_BAD )
		return false;

	// set client encoding
	if ( !m_sPgClientEncoding.IsEmpty() )
		if ( -1==PQsetClientEncoding ( m_tPgDriver, m_sPgClientEncoding.cstr() ) )
	{
		SqlDisconnect ();
		return false;
	}

	return true;
}


void CSphSource_PgSQL::SqlDisconnect ()	
{
	PQfinish ( m_tPgDriver );
}


bool CSphSource_PgSQL::SqlQuery ( const char * sQuery )
{
	m_iPgRow = -1;
	m_iPgRows = 0;

	m_pPgResult = PQexec ( m_tPgDriver, sQuery );

	ExecStatusType eRes = PQresultStatus ( m_pPgResult );
	if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
		return false;

	m_iPgRows = PQntuples ( m_pPgResult );
	return true;
}


void CSphSource_PgSQL::SqlDismissResult ()
{
	if ( !m_pPgResult )
		return;

	PQclear ( m_pPgResult );
	m_pPgResult = NULL;
}


int CSphSource_PgSQL::SqlNumFields ()
{
	if ( !m_pPgResult )
		return -1;

	return PQnfields ( m_pPgResult );
}


const char * CSphSource_PgSQL::SqlColumn ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	return PQgetvalue ( m_pPgResult, m_iPgRow, iIndex );
}


const char * CSphSource_PgSQL::SqlFieldName ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	return PQfname ( m_pPgResult, iIndex );
}


bool CSphSource_PgSQL::SqlFetchRow ()
{
	if ( !m_pPgResult )
		return false;
	return ( ++m_iPgRow<m_iPgRows );
}

#endif // USE_PGSQL

/////////////////////////////////////////////////////////////////////////////
// XMLPIPE
/////////////////////////////////////////////////////////////////////////////

CSphSource_XMLPipe::CSphSource_XMLPipe ( const char * sName)
	: CSphSource	( sName )
	, m_iBufferSize	( 1048576 )
	, m_bEOF		( false )
	, m_bWarned		( false )
{
	m_pTag			= NULL;
	m_iTagLength	= 0;
	m_pPipe			= NULL;
	m_pBuffer		= NULL;
	m_pBufferEnd	= NULL;
	m_sBuffer		= new BYTE [m_iBufferSize];
}


CSphSource_XMLPipe::~CSphSource_XMLPipe ()
{
	Disconnect ();
	SafeDeleteArray ( m_sBuffer );
}


void CSphSource_XMLPipe::Disconnect ()
{
	m_tSchema.Reset ();

	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}
}


bool CSphSource_XMLPipe::Setup ( const char * sCommand )
{
	assert ( sCommand );
	m_sCommand = sCommand;
	return true;
}


bool CSphSource_XMLPipe::Connect ( CSphString & sError )
{
	m_pPipe = popen ( m_sCommand.cstr(), "r" );
	
	m_bEOF		= false;
	m_bWarned	= false;

	m_tSchema.m_dFields.Reset ();
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "title" ) );
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "body" ) );

	CSphColumnInfo tGid ( "gid", SPH_ATTR_INTEGER ); 
	CSphColumnInfo tTs ( "ts", SPH_ATTR_TIMESTAMP );
	m_tSchema.AddAttr ( tGid );
	m_tSchema.AddAttr ( tTs );

	m_tDocInfo.Reset ( m_tSchema.GetRowSize() );

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "xmlpipe(%s)", m_sCommand.cstr() );
	m_tSchema.m_sName = sBuf;

	if ( !m_pPipe )
	{
		sError.SetSprintf ( "xmlpipe: failed to popen '%s'", m_sCommand.cstr() );
		return false;
	}

	return true;
}


bool CSphSource_XMLPipe::IterateHitsNext ( CSphString & sError )
{
	PROFILE ( src_xmlpipe );
	char sTitle [ 1024 ]; // FIXME?

	assert ( m_pPipe );
	assert ( m_pTokenizer );

	m_dHits.Reserve ( 1024 );
	m_dHits.Resize ( 0 );

	/////////////////////////
	// parse document header
	/////////////////////////

	// check for eof
	if ( !SkipWhitespace() )
	{
		m_tDocInfo.m_iDocID = 0;
		return true;
	}

	// look for opening '<document>' tag
	SetTag ( "document" );
	if ( !SkipTag ( true, sError ) )
		return false;

	if ( !ScanInt ( "id", &m_tDocInfo.m_iDocID, sError ) )
		return false;
	m_tStats.m_iTotalDocuments++;

	if ( !ScanInt ( "group", &m_tDocInfo.m_pRowitems[0], sError ) )
		m_tDocInfo.m_pRowitems[0] = 1;

	if ( !ScanInt ( "timestamp", &m_tDocInfo.m_pRowitems[1], sError ) )
		m_tDocInfo.m_pRowitems[1] = 1;

	if ( !ScanStr ( "title", sTitle, sizeof(sTitle), sError ) )
		return false;

	// index title
	{
		int iLen = (int)strlen ( sTitle );
		int iPos = 1;
		BYTE * sWord;

		m_pTokenizer->SetBuffer ( (BYTE*)sTitle, iLen );
		while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
		{
			SphWordID_t iWID = m_pDict->GetWordID ( sWord );
			if ( iWID )
			{
				CSphWordHit & tHit = m_dHits.Add ();
				tHit.m_iDocID = m_tDocInfo.m_iDocID;
				tHit.m_iWordID = iWID;
				tHit.m_iWordPos = iPos;
			}
			iPos++;
		}
	}

	SetTag ( "body" );
	if ( !SkipTag ( true, sError ) )
		return false;

	m_iWordPos = 0;

	/////////////////////////////
	// parse body chunk by chunk
	/////////////////////////////

	// check for body tag end in this buffer
	const char * szBodyEnd = "</body>";

	bool bFirstPass = true;
	bool bBodyEnd = false;
	BYTE * p = m_pBuffer;

	while ( ! bBodyEnd )
	{
		p = m_pBuffer;
		while ( p<m_pBufferEnd && ! bBodyEnd )
		{
			BYTE * pBufTemp = p;
			BYTE * pEndTemp = (BYTE *)szBodyEnd;
			while ( pBufTemp < m_pBufferEnd && *pEndTemp && *pBufTemp == *pEndTemp )
			{
				++pBufTemp;
				++pEndTemp;
			}

			if ( !*pEndTemp )
				bBodyEnd = true;
			else
				p++;
		}

		if ( ! bFirstPass )
			break;

		bFirstPass = false;

		if ( ! bBodyEnd )
			UpdateBuffer ();
	}

	if ( ! bBodyEnd )
	{
		if ( ! m_bWarned )
		{
			sphWarn ( "xmlpipe: encountered body larger than %d bytes while scanning docid=" DOCID_FMT " body", m_iBufferSize, m_tDocInfo.m_iDocID );
			m_bWarned = true;
		}
	}

	m_pTokenizer->SetBuffer ( m_pBuffer, p-m_pBuffer );

	// tokenize
	BYTE * sWord;
	while ( ( sWord = m_pTokenizer->GetToken () )!=NULL )
	{
		SphWordID_t iWID = m_pDict->GetWordID ( sWord );
		++m_iWordPos;
		if ( iWID )
		{
			CSphWordHit & tHit = m_dHits.Add ();
			tHit.m_iDocID = m_tDocInfo.m_iDocID;
			tHit.m_iWordID = iWID;
			tHit.m_iWordPos = (1<<24) | m_iWordPos; // field_id | iPos
		}
	}

	m_pBuffer = p;

	SetTag ( "body" );

	// some tag was found
	if ( bBodyEnd )
	{
		// let's check if it's '</body>' which is the only allowed tag at this point
		if ( !SkipTag ( false, sError ) )
			return false;
	}
	else
	{
		// search for '</body>' tag
		bool bFound = false;

		while ( ! bFound )
		{
			while ( m_pBuffer < m_pBufferEnd && *m_pBuffer != '<' )
				++m_pBuffer;

			BYTE * pBufferTmp = m_pBuffer;
			if ( m_pBuffer < m_pBufferEnd )
			{
				if ( ! SkipTag ( false, sError ) )
				{
					if ( m_bEOF )
						return false;
					else
					{
						if ( m_pBuffer == pBufferTmp )
							m_pBuffer = pBufferTmp + 1;
					}
				}
				else
					bFound = true;
			}
			else
				if ( ! UpdateBuffer () )
					return false;
		}
	}


	// let's check if it's '</document>' which is the only allowed tag at this point
	SetTag ( "document" );
	if ( !SkipTag ( false, sError ) )
		return false;

	// if it was all correct, we have to flush our hits
	return true;
}


void CSphSource_XMLPipe::SetTag ( const char * sTag )
{
	m_pTag = sTag;
	m_iTagLength = (int)strlen ( sTag );
}


bool CSphSource_XMLPipe::UpdateBuffer ()
{
	assert ( m_pBuffer!=m_sBuffer );

	int iLeft = Max ( m_pBufferEnd-m_pBuffer, 0 );
	if ( iLeft>0 )
		memmove ( m_sBuffer, m_pBuffer, iLeft );

	size_t iLen = fread ( &m_sBuffer [ iLeft ], 1, m_iBufferSize-iLeft, m_pPipe );
	m_tStats.m_iTotalBytes += iLen;

	m_pBuffer = m_sBuffer;
	m_pBufferEnd = m_pBuffer+iLeft+iLen;

	return ( iLen!=0 );
}


bool CSphSource_XMLPipe::SkipWhitespace ()
{
	for ( ;; )
	{
		// suck in some data if needed
		if ( m_pBuffer>=m_pBufferEnd )
			if ( !UpdateBuffer() )
				return false;

		// skip whitespace
		while ( (m_pBuffer<m_pBufferEnd) && isspace ( *m_pBuffer ) )
			m_pBuffer++;

		// did we anything non-whitspace?
		if ( m_pBuffer<m_pBufferEnd )
			break;
	}

	assert ( m_pBuffer<m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::CheckTag ( bool bOpen, CSphString & sError )
{
	int iAdd = bOpen ? 2 : 3;

	// if case the tag is at buffer boundary, try to suck in some more data
	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
		UpdateBuffer ();

	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
	{
		m_bEOF = true;
		sError.SetSprintf ( "xmlpipe: expected '<%s%s>', got EOF",
			bOpen ? "" : "/", m_pTag );
		return false;
	}

	// check tag
	bool bOk = bOpen
		? ( ( m_pBuffer[0] == '<' )
			&& ( m_pBuffer[m_iTagLength+1] == '>' )
			&& strncmp ( (char*)(m_pBuffer+1), m_pTag, m_iTagLength ) == 0 )
		: ( ( m_pBuffer[0] == '<' )
			&& ( m_pBuffer[1] == '/' )
			&& ( m_pBuffer[m_iTagLength+2] == '>' )
			&& strncmp ( (char*)(m_pBuffer+2), m_pTag, m_iTagLength ) == 0 );
	if ( !bOk )
	{
		char sGot [ 64 ];
		int iCopy = Min ( m_pBufferEnd-m_pBuffer, (int)sizeof(sGot)-1 );

		strncpy ( sGot, (char*)m_pBuffer, iCopy );
		sGot [ iCopy ] = '\0';

		sError.SetSprintf ( "xmlpipe: expected '<%s%s>', got '%s'",
			bOpen ? "" : "/", m_pTag, sGot );
		return false;
	}

	// got tag
	m_pBuffer += iAdd+m_iTagLength;
	assert ( m_pBuffer<=m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::SkipTag ( bool bOpen, CSphString & sError )
{
	if ( !SkipWhitespace() )
	{
		m_bEOF = true;
		sError.SetSprintf ( "xmlpipe: expected '<%s%s>', got EOF",
			bOpen ? "" : "/", m_pTag );
		return false;
	}

	return CheckTag ( bOpen, sError );
}


bool CSphSource_XMLPipe::ScanInt ( const char * sTag, DWORD * pRes, CSphString & sError )
{
	uint64_t uRes;
	if ( !ScanInt ( sTag, &uRes, sError ) )
		return false;

	(*pRes) = (DWORD)uRes;
	return true;
}


bool CSphSource_XMLPipe::ScanInt ( const char * sTag, uint64_t * pRes, CSphString & sError )
{
	assert ( sTag );
	assert ( pRes );

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true, sError ) )
		return false;

	if ( !SkipWhitespace() )
	{
		sError.SetSprintf ( "xmlpipe: expected <%s> data, got EOF", m_pTag );
		return false;
	}

	*pRes = 0;
	while ( m_pBuffer<m_pBufferEnd )
	{
		// FIXME! could check for overflow
		while ( isdigit(*m_pBuffer) && m_pBuffer<m_pBufferEnd )
			(*pRes) = 10*(*pRes) + int( (*m_pBuffer++)-'0' );

		if ( m_pBuffer<m_pBufferEnd )
			break;
		else
			UpdateBuffer ();
	}

	// scan for </sTag>
	if ( !SkipTag ( false, sError ) )
		return false;

	return true;
}


bool CSphSource_XMLPipe::ScanStr ( const char * sTag, char * pRes, int iMaxLength, CSphString & sError )
{
	assert ( sTag );
	assert ( pRes );
	
	char * pEnd = pRes+iMaxLength-1;

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true, sError ) )
		return false;

	if ( !SkipWhitespace() )
	{
		sError.SetSprintf ( "xmlpipe: expected <%s> data, got EOF", m_pTag );
		return false;
	}

	while ( m_pBuffer<m_pBufferEnd )
	{
		while ( (*m_pBuffer)!='<' && pRes<pEnd && m_pBuffer<m_pBufferEnd )
			*pRes++ = *m_pBuffer++;

		if ( m_pBuffer<m_pBufferEnd )
			break;
		else
			UpdateBuffer ();
	}
	*pRes++ = '\0';

	// scan for </sTag>
	if ( !SkipTag ( false, sError ) )
		return false;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// XMLPIPE (v2)
/////////////////////////////////////////////////////////////////////////////

#if USE_LIBEXPAT || USE_LIBXML

/// XML pipe source implementation (v2)
class CSphSource_XMLPipe2 : public CSphSource_Document
{
public:
					CSphSource_XMLPipe2 ( const char * sName );	///< ctor
					~CSphSource_XMLPipe2 ();					///< dtor

	bool			Setup ( const CSphConfigSection & hSource );			///< memorize the command
	virtual bool	Connect ( CSphString & sError );			///< run the command and open the pipe
	virtual void	Disconnect ();								///< close the pipe

	virtual bool	IterateHitsStart ( CSphString & ) { return true; }	///< Connect() starts getting documents automatically, so this one is empty
	virtual BYTE **	NextDocument ( CSphString & sError );			///< parse incoming chunk and emit some hits

	virtual bool	HasAttrsConfigured ()							{ return true; }	///< xmlpipe always has some attrs for now
	virtual bool	IterateMultivaluedStart ( int, CSphString & )	{ return false; }
	virtual bool	IterateMultivaluedNext ()						{ return false; }
	virtual bool	IterateFieldMVAStart ( int iAttr, CSphString & sError );
	virtual bool	IterateFieldMVANext ();


	void			StartElement ( const char * szName, const char ** pAttrs );
	void			EndElement ( const char * pName );
	void			Characters ( const char * pCharacters, int iLen );

#if USE_LIBXML
	int				ReadBuffer ( BYTE * pBuffer, int iLen );
	void			ProcessNode ( xmlTextReaderPtr pReader );
#endif

	void			Error ( const char * sTemplate, ... );

private:
	struct Document_t
	{
		SphDocID_t				m_iDocID;
		CSphVector<CSphString>	m_dFields;
		CSphVector<CSphString>	m_dAttrs;
	};

	Document_t *				m_pCurDocument;
	CSphVector<Document_t *>	m_dParsedDocuments;

	FILE *			m_pPipe;			///< incoming stream
	CSphString		m_sCommand;			///< my command
	CSphString		m_sError;
	CSphVector<CSphString> m_dDefaultAttrs;
	CSphVector<CSphString> m_dInvalid;
	CSphVector<CSphString> m_dWarned;
	int				m_iElementDepth;

	BYTE *			m_pBuffer;
	int				m_iBufferSize;

	CSphVector<BYTE*>m_dFieldPtrs;
	bool			m_bRemoveParsed;

	bool			m_bInDocset;
	bool			m_bInSchema;
	bool			m_bInDocument;
	bool			m_bFirstTagAfterDocset;

	int				m_iMVA;
	int				m_iMVAIterator;
	CSphVector < CSphVector <DWORD> > m_dFieldMVAs;
	CSphVector < int > m_dAttrToMVA;

	int				m_iCurField;
	int				m_iCurAttr;

#if USE_LIBEXPAT
	XML_Parser		m_pParser;
#endif

#if USE_LIBXML
	xmlTextReaderPtr m_pParser;

	BYTE *			m_pBufferPtr;
	BYTE *			m_pBufferEnd;
	bool			m_bPassedBufferEnd;
	CSphVector <const char *> m_dAttrs;
#endif

	static const int MAX_FIELD_LENGTH = 2097152;
	BYTE * 			m_pFieldBuffer;
	int				m_iFieldBufferLen;

	const char *	DecorateMessage ( const char * sTemplate, ... );
	const char *	DecorateMessageVA ( const char * sTemplate, va_list ap );

	void			ConfigureAttrs ( const CSphVariant * pHead, DWORD uAttrType );
	void			ConfigureFields ( const CSphVariant * pHead );
	void			AddFieldToSchema ( const char * szName );
	void			SetupFieldMatch ( CSphColumnInfo & tCol );

#if USE_LIBEXPAT
	bool			ParseNextChunk ( int iBufferLen, CSphString & sError );
#endif

#if USE_LIBXML
	int				ParseNextChunk ( CSphString & sError );
#endif
};


#if USE_LIBEXPAT
// callbacks
static void XMLCALL xmlStartElement ( void * user_data,  const XML_Char * name, const XML_Char ** attrs )
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
		char dOut [4];
		memset ( dOut, 0, sizeof ( dOut ) );
#if ICONV_INBUF_CONST
		const char * pInbuf = &cIn;
#else
		char * pInbuf = &cIn;
#endif
		char * pOutbuf = dOut;
		size_t iInBytesLeft = 1;
		size_t iOutBytesLeft = 4;

		if ( iconv ( pDesc, &pInbuf, &iInBytesLeft, &pOutbuf, &iOutBytesLeft ) != size_t (-1) )
			info->map [i] = int ( BYTE ( dOut [0] ) ) << 8 | int ( BYTE ( dOut [1] ) );
		else
			info->map [i] = 0;
	}

	iconv_close ( pDesc );

	return XML_STATUS_OK;

}
#endif

#endif

#if USE_LIBXML
int	xmlReadBuffers (void * context, char * buffer, int len )
{
	CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) context;
	return pSource->ReadBuffer ( (BYTE*)buffer, len );
}

void xmlErrorHandler ( void * arg, const char * msg, xmlParserSeverities severity, xmlTextReaderLocatorPtr locator)
{
	if ( severity == XML_PARSER_SEVERITY_ERROR )
	{
		int iLine = xmlTextReaderLocatorLineNumber ( locator );
		CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) arg;
		pSource->Error ( "%s (line=%d)", msg, iLine );
	}
}
#endif


CSphSource_XMLPipe2::CSphSource_XMLPipe2 ( const char * sName )
	: CSphSource_Document ( sName )
	, m_pCurDocument	( NULL )
	, m_pPipe			( NULL )
	, m_iElementDepth	( 0 )
	, m_iBufferSize		( 1048576 )
	, m_bRemoveParsed	( false )
	, m_bInDocset		( false )
	, m_bInSchema		( false )
	, m_bInDocument		( false )
	, m_bFirstTagAfterDocset( false )
	, m_iMVA			( 0 )
	, m_iMVAIterator	( 0 )
	, m_iCurField		( -1 )
	, m_iCurAttr		( -1 )
	, m_pParser			( NULL )
	, m_iFieldBufferLen	( 0 )
#if USE_LIBXML
	, m_pBufferPtr		( NULL )
	, m_pBufferEnd		( NULL )
	, m_bPassedBufferEnd( false )
#endif
{
	m_pBuffer = new BYTE [m_iBufferSize];
	m_pFieldBuffer = new BYTE [MAX_FIELD_LENGTH];
}


CSphSource_XMLPipe2::~CSphSource_XMLPipe2 ()
{
	Disconnect ();
	SafeDeleteArray ( m_pBuffer );
	SafeDeleteArray ( m_pFieldBuffer );
	ARRAY_FOREACH ( i, m_dParsedDocuments )
		SafeDelete ( m_dParsedDocuments [i] );
}


void CSphSource_XMLPipe2::Disconnect ()
{
	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}

#if USE_LIBEXPAT
	if ( m_pParser )
	{
		XML_ParserFree ( m_pParser );
		m_pParser = NULL;
	}
#endif

#if USE_LIBXML
	if ( m_pParser )
	{
		xmlFreeTextReader ( m_pParser );
		m_pParser = NULL;
	}
#endif
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


const char * CSphSource_XMLPipe2::DecorateMessage ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	const char * sRes = DecorateMessageVA ( sTemplate, ap );
	va_end ( ap );
	return sRes;
}


const char * CSphSource_XMLPipe2::DecorateMessageVA ( const char * sTemplate, va_list ap )
{
	static char sBuf[1024];

	snprintf ( sBuf, sizeof(sBuf), "source '%s': ",m_tSchema.m_sName.cstr() );
	int iBufLen = strlen ( sBuf );
	int iLeft = sizeof(sBuf) - iBufLen;
	char * szBufStart = sBuf + iBufLen;

	vsnprintf ( szBufStart, iLeft, sTemplate, ap );
	iBufLen = strlen ( sBuf );
	iLeft = sizeof(sBuf) - iBufLen;
	szBufStart = sBuf + iBufLen;

#if USE_LIBEXPAT
	if ( m_pParser )
	{
		SphDocID_t uFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			uFailedID = m_dParsedDocuments.Last()->m_iDocID;

		snprintf ( szBufStart, iLeft,  " (line=%d, pos=%d, docid=" DOCID_FMT ")",
			(int)XML_GetCurrentLineNumber(m_pParser), (int)XML_GetCurrentColumnNumber(m_pParser),
			uFailedID );
	}
#endif

#if USE_LIBXML
	if ( m_pParser )
	{
		SphDocID_t uFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			uFailedID = m_dParsedDocuments.Last()->m_iDocID;

		snprintf ( szBufStart, iLeft,  " (docid=" DOCID_FMT ")", uFailedID );
	}
#endif

	return sBuf;
}


void CSphSource_XMLPipe2::AddFieldToSchema ( const char * szName )
{
	CSphColumnInfo tCol ( szName );
	SetupFieldMatch ( tCol );
	m_tSchema.m_dFields.Add ( tCol );
}


void CSphSource_XMLPipe2::ConfigureAttrs ( const CSphVariant * pHead, DWORD uAttrType )
{
	for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
	{
		CSphColumnInfo tCol ( pCur->cstr(), uAttrType );
		char * pColon = strchr ( const_cast<char*>(tCol.m_sName.cstr()), ':' );
		if ( pColon )
		{
			*pColon = '\0';

			if ( uAttrType == SPH_ATTR_INTEGER )
			{
				int iBits = strtol ( pColon+1, NULL, 10 );
				if ( iBits<=0 || iBits>ROWITEM_BITS )
				{
					sphWarn ( "%s", DecorateMessage ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), iBits ) );
					iBits = -1;
				}

				tCol.m_iBitCount = iBits;
			}
			else
				sphWarn ( "%s", DecorateMessage ( "attribute '%s': bitcount is only supported for integer types", tCol.m_sName.cstr() ) );
		}

		tCol.m_iIndex = m_tSchema.GetAttrsCount ();

		if ( uAttrType == SPH_ATTR_MULTI )
		{
			tCol.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
			tCol.m_eSrc = SPH_ATTRSRC_FIELD;
		}

		m_tSchema.AddAttr ( tCol );
	}
}


void CSphSource_XMLPipe2::ConfigureFields ( const CSphVariant * pHead )
{
	for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
	{
		CSphString sFieldName = pCur->cstr ();

		bool bFound = false;
		for ( int i = 0; i < m_tSchema.m_dFields.GetLength () && !bFound; i++ )
			bFound = m_tSchema.m_dFields [i].m_sName == sFieldName;
		
		if ( bFound )
			sphWarn ( "%s", DecorateMessage ( "duplicate field '%s'", sFieldName.cstr () ) );
		else
			AddFieldToSchema ( sFieldName.cstr () );
	}
}


bool CSphSource_XMLPipe2::Setup ( const CSphConfigSection & hSource )
{
	m_tSchema.Reset ();

	m_sCommand = hSource["xmlpipe_command"].cstr ();

	ConfigureAttrs ( hSource("xmlpipe_attr_uint"),			SPH_ATTR_INTEGER );
	ConfigureAttrs ( hSource("xmlpipe_attr_timestamp"),		SPH_ATTR_TIMESTAMP );
	ConfigureAttrs ( hSource("xmlpipe_attr_str2ordinal"),	SPH_ATTR_ORDINAL );
	ConfigureAttrs ( hSource("xmlpipe_attr_bool"),			SPH_ATTR_BOOL );
	ConfigureAttrs ( hSource("xmlpipe_attr_float"),			SPH_ATTR_FLOAT );
	ConfigureAttrs ( hSource("xmlpipe_attr_multi"),			SPH_ATTR_MULTI );

	m_tDocInfo.Reset ( m_tSchema.GetRowSize () );

	ConfigureFields ( hSource("xmlpipe_field") );

	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );

	return true;
}


void CSphSource_XMLPipe2::SetupFieldMatch ( CSphColumnInfo & tCol )
{
	bool bPrefix = m_iMinPrefixLen > 0 && IsPrefixMatch ( tCol.m_sName.cstr () );
	bool bInfix =  m_iMinInfixLen > 0  && IsInfixMatch ( tCol.m_sName.cstr () );

	if ( bPrefix && m_iMinPrefixLen > 0 && bInfix && m_iMinInfixLen > 0)
	{
		Error ( "field '%s' is marked for both infix and prefix indexing", tCol.m_sName.cstr() );
		return;
	}

	if ( bPrefix )
		tCol.m_eWordpart = SPH_WORDPART_PREFIX;

	if ( bInfix )
		tCol.m_eWordpart = SPH_WORDPART_INFIX;
}


bool CSphSource_XMLPipe2::Connect ( CSphString & sError )
{
	m_pPipe = popen ( m_sCommand.cstr(), "r" );
	if ( !m_pPipe )
	{
		sError.SetSprintf ( "xmlpipe: failed to popen '%s'", m_sCommand.cstr() );
		return false;
	}

	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		SetupFieldMatch ( m_tSchema.m_dFields [i] );

#if USE_LIBEXPAT
	m_pParser = XML_ParserCreate(NULL);
	if ( !m_pParser )
	{
		sError.SetSprintf ( "xmlpipe: failed to create XML parser" );
		return false;
	}

	XML_SetUserData ( m_pParser, this );	
	XML_SetElementHandler ( m_pParser, xmlStartElement, xmlEndElement );
	XML_SetCharacterDataHandler ( m_pParser, xmlCharacters );

#if USE_LIBICONV
	XML_SetUnknownEncodingHandler ( m_pParser, xmlUnknownEncoding, NULL );
#endif

#endif

#if USE_LIBXML
	m_pBufferPtr = NULL;
	m_pBufferEnd = NULL;
	m_bPassedBufferEnd = false;
	m_dAttrs.Reserve ( 16 );
	m_dAttrs.Resize ( 0 );

	m_pParser = xmlReaderForIO ( (xmlInputReadCallback)xmlReadBuffers, NULL, this, NULL, NULL, 0 );
	if ( !m_pParser )
	{
		sError.SetSprintf ( "xmlpipe: failed to create XML parser" );
		return false;
	}

	xmlTextReaderSetErrorHandler ( m_pParser, xmlErrorHandler,  this );
#endif

	m_bRemoveParsed = false;
	m_bInDocset	= false;
	m_bInSchema	= false;
	m_bInDocument = false;
	m_bFirstTagAfterDocset = false;
	m_iCurField	= -1;
	m_iCurAttr	= -1;
	m_iElementDepth = 0;

	m_dParsedDocuments.Reset ();
	m_dDefaultAttrs.Reset ();
	m_dInvalid.Reset ();
	m_dWarned.Reset ();

	m_dParsedDocuments.Reserve ( 1024 );
	m_dParsedDocuments.Resize ( 0 );

	m_iMVA = 0;
	m_iMVAIterator = 0;

	m_sError = "";

#if USE_LIBEXPAT
	int iBytesRead = fread ( m_pBuffer, 1, m_iBufferSize, m_pPipe );
	if ( !ParseNextChunk ( iBytesRead, sError ) )
		return false;
#endif

#if USE_LIBXML
	if ( ParseNextChunk ( sError ) == -1 )
		return false;
#endif

	m_dAttrToMVA.Resize ( 0 );

	int iFieldMVA = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount (); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr ( i );
		if ( ( tCol.m_eAttrType & SPH_ATTR_MULTI ) && tCol.m_eSrc == SPH_ATTRSRC_FIELD )
			m_dAttrToMVA.Add ( iFieldMVA++ );
		else
			m_dAttrToMVA.Add ( -1 );
	}

	m_dFieldMVAs.Resize ( iFieldMVA );
	ARRAY_FOREACH ( i, m_dFieldMVAs )
		m_dFieldMVAs [i].Reserve ( 16 );

	return true;
}


#if USE_LIBXML
int CSphSource_XMLPipe2::ParseNextChunk ( CSphString & sError )
{
	int iRet = xmlTextReaderRead ( m_pParser );
	while ( iRet == 1 && !m_bPassedBufferEnd )
	{
		ProcessNode ( m_pParser );
		if ( !m_sError.IsEmpty () )
		{
			sError = m_sError;
			m_tDocInfo.m_iDocID = 1;
			return false;
		}

		iRet = xmlTextReaderRead ( m_pParser );
	}

	m_bPassedBufferEnd = false;

	if ( !m_sError.IsEmpty () || iRet == -1 )
	{
		sError = m_sError;
		m_tDocInfo.m_iDocID = 1;
		return -1;
	}

	return iRet;
}
#endif


#if USE_LIBEXPAT
bool CSphSource_XMLPipe2::ParseNextChunk ( int iBufferLen, CSphString & sError )
{
	if ( !iBufferLen )
		return true;

	if ( XML_Parse ( m_pParser, (const char*) m_pBuffer, iBufferLen, iBufferLen != m_iBufferSize ) != XML_STATUS_OK )
	{
		SphDocID_t uFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			uFailedID = m_dParsedDocuments.Last()->m_iDocID;

		sError.SetSprintf ( "source '%s': XML parse error: %s (line=%d, pos=%d, docid=" DOCID_FMT ")",
			m_tSchema.m_sName.cstr(),  XML_ErrorString ( XML_GetErrorCode(m_pParser) ),
			XML_GetCurrentLineNumber(m_pParser), XML_GetCurrentColumnNumber(m_pParser),
			uFailedID );
		m_tDocInfo.m_iDocID = 1;
		return false;
	}

	if ( !m_sError.IsEmpty () )
	{
		sError = m_sError;
		m_tDocInfo.m_iDocID = 1;
		return false;
	}

	return true;
}
#endif


BYTE **	CSphSource_XMLPipe2::NextDocument ( CSphString & sError )
{
	if ( m_bRemoveParsed )
	{
		SafeDelete ( m_dParsedDocuments [0] );
		m_dParsedDocuments.RemoveFast ( 0 );
		m_bRemoveParsed = false;
	}

	int iReadResult = 0;

#if USE_LIBEXPAT
	while ( m_dParsedDocuments.GetLength () == 0 && ( iReadResult = fread ( m_pBuffer, 1, m_iBufferSize, m_pPipe ) ) != 0 )
		if ( !ParseNextChunk ( iReadResult, sError ) )
			return NULL;
#endif

#if USE_LIBXML
	while ( m_dParsedDocuments.GetLength () == 0 && ( iReadResult = ParseNextChunk ( sError ) ) == 1 );
#endif
		
	if ( m_dParsedDocuments.GetLength () != 0 )
	{
		Document_t * pDocument = m_dParsedDocuments [0];
		int nAttrs = m_tSchema.GetAttrsCount ();

		// docid
		m_tDocInfo.m_iDocID = pDocument->m_iDocID;

		// attributes
		int iFieldMVA = 0;
		for ( int i = 0; i < nAttrs; i++ )
		{
			const CSphString & sAttrValue = pDocument->m_dAttrs [i].IsEmpty () && m_dDefaultAttrs.GetLength () ? m_dDefaultAttrs [i] : pDocument->m_dAttrs [i];
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( i );

			if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
			{
				m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, 0 );
				ParseFieldMVA ( m_dFieldMVAs, iFieldMVA++, sAttrValue.cstr () );
				continue;
			}

			switch ( tAttr.m_eAttrType )
			{
				case SPH_ATTR_ORDINAL:
					m_dStrAttrs [i] = sAttrValue.cstr ();
					if ( !m_dStrAttrs[i].cstr() )
						m_dStrAttrs[i] = "";

					m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, 0 );
					break;

				case SPH_ATTR_FLOAT:
					m_tDocInfo.SetAttrFloat ( tAttr.m_iRowitem, sphToFloat ( sAttrValue.cstr () ) );
					break;

				default:
					m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount,	sphToDword ( sAttrValue.cstr () ) );
					break;
			}
		}

		m_bRemoveParsed = true;

		int nFields = m_tSchema.m_dFields.GetLength ();
		m_dFieldPtrs.Resize ( nFields );
		for ( int i = 0; i < nFields; ++i )
			m_dFieldPtrs [i] = (BYTE*) ( pDocument->m_dFields [i].cstr () );

		return (BYTE **)&( m_dFieldPtrs [0]);
	}

	if ( !iReadResult )
		m_tDocInfo.m_iDocID = 0;

	return NULL;
}


bool CSphSource_XMLPipe2::IterateFieldMVAStart ( int iAttr, CSphString & )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	if ( m_dAttrToMVA [iAttr] == -1 )
		return false;

	m_iMVA = iAttr;
	m_iMVAIterator = 0;

	return true;
}


bool CSphSource_XMLPipe2::IterateFieldMVANext ()
{
	int iFieldMVA = m_dAttrToMVA [m_iMVA];
	if ( m_iMVAIterator >= m_dFieldMVAs [iFieldMVA].GetLength () )
		return false;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iMVA );
	m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, m_dFieldMVAs [iFieldMVA][m_iMVAIterator] );

	++m_iMVAIterator;

	return true;
}


void CSphSource_XMLPipe2::StartElement ( const char * szName, const char ** pAttrs )
{
	if ( !strcmp ( szName, "sphinx:docset" ) )
	{
		m_bInDocset = true;
		m_bFirstTagAfterDocset = true;
		return;
	}

	if ( !strcmp ( szName, "sphinx:schema" ) )
	{
		if ( !m_bInDocset || !m_bFirstTagAfterDocset )
		{
			Error ( "<sphinx:schema> is allowed immediately after <sphinx:docset> only" );
			return;
		}

		if ( m_tSchema.m_dFields.GetLength () > 0 || m_tSchema.GetAttrsCount () > 0 )
		{
			sphWarn ( "%s", DecorateMessage ( "both embedded and configured schemas found; using embedded" ) );
			m_tSchema.Reset ();
		}

		m_bFirstTagAfterDocset = false;
		m_bInSchema = true;
		return;
	}

	if ( !strcmp ( szName, "sphinx:field" ) )
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:field> is allowed inside <sphinx:schema> only" );
			return;
		}

		const char ** dAttrs = pAttrs;

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] )
		{
			if ( !strcmp ( dAttrs[0], "name" ) )
				AddFieldToSchema ( dAttrs[1] );

			dAttrs += 2;
		}
		return;
	}

	if ( !strcmp ( szName, "sphinx:attr" ) )
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:attr> is allowed inside <sphinx:schema> only" );
			return;
		}

		bool bError = false;
		CSphColumnInfo Info;
		CSphString sDefault;

		const char ** dAttrs = pAttrs;

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] && !bError )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
				Info.m_sName = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "bits" ) )
				Info.m_iBitCount = strtol ( dAttrs[1], NULL, 10 );
			else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "type" ) )
			{
				const char * szType = dAttrs[1];
				if ( !strcmp ( szType, "int" ) )				Info.m_eAttrType = SPH_ATTR_INTEGER;
				else if ( !strcmp ( szType, "timestamp" ) )		Info.m_eAttrType = SPH_ATTR_TIMESTAMP;
				else if ( !strcmp ( szType, "str2ordinal" ) )	Info.m_eAttrType = SPH_ATTR_ORDINAL;
				else if ( !strcmp ( szType, "bool" ) )			Info.m_eAttrType = SPH_ATTR_BOOL;
				else if ( !strcmp ( szType, "float" ) )			Info.m_eAttrType = SPH_ATTR_FLOAT;
				else if ( !strcmp ( szType, "multi" ) )
				{
					Info.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
					Info.m_eSrc = SPH_ATTRSRC_FIELD;
				}
				else
				{
					Error ( "unknown column type '%s'", szType );
					bError = true;
				}
			}

			dAttrs += 2;
		}

		if ( !bError )
		{
			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info );
			m_dDefaultAttrs.Add ( sDefault );
		}

		return;
	}

	if ( !strcmp ( szName, "sphinx:document" ) )
	{
		if ( !m_bInDocset || m_bInSchema )
		{
			Error ( "<sphinx:document> is not allowed inside <sphinx:schema>" );
			return;
		}

		if ( m_tSchema.m_dFields.GetLength () == 0 && m_tSchema.GetAttrsCount () == 0 )
		{
			Error ( "no schema configured, and no embedded schema found" );
			return;
		}

		m_bInDocument = true;

		assert ( !m_pCurDocument );
		m_pCurDocument = new Document_t;

		m_pCurDocument->m_iDocID = 0;
		m_pCurDocument->m_dFields.Resize ( m_tSchema.m_dFields.GetLength () );
		m_pCurDocument->m_dAttrs.Resize ( m_tSchema.GetAttrsCount () );

		if ( pAttrs[0] && pAttrs[1] && pAttrs[0][0] && pAttrs[1][0] )
			if ( !strcmp ( pAttrs[0], "id" ) )
				m_pCurDocument->m_iDocID = sphToDocid ( pAttrs[1] );

		if ( m_pCurDocument->m_iDocID == 0 )
			Error ( "attribute 'id' required in <sphinx:document>" );

		return;
	}

	if ( m_bInDocument )
	{
		if ( m_iCurField == -1 && m_iCurAttr == -1 )
		{
			for ( int i = 0; i < m_tSchema.m_dFields.GetLength () && m_iCurField == -1; i++ )
				if ( m_tSchema.m_dFields [i].m_sName == szName )
					m_iCurField = i;

			for ( int i = 0; i < m_tSchema.GetAttrsCount () && m_iCurAttr == -1 && m_iCurField == -1; i++ )
				if ( m_tSchema.GetAttr ( i ).m_sName == szName )
					m_iCurAttr = i;

			if ( m_iCurAttr == -1 && m_iCurField == -1 )
			{
				bool bInvalidFound = false;
				for ( int i = 0; i < m_dInvalid.GetLength () && !bInvalidFound; i++ )
					bInvalidFound = m_dInvalid [i] == szName;

				if ( !bInvalidFound )
				{
					sphWarn ( "%s", DecorateMessage ( "unknown field/attribute '%s'; ignored", szName ) );
					m_dInvalid.Add ( szName );
				}
			}
		}
		else
			m_iElementDepth++;
	}
}


void CSphSource_XMLPipe2::EndElement ( const char * szName )
{
	if ( !strcmp ( szName, "sphinx:docset" ) )
		m_bInDocset = false;
	else if ( !strcmp ( szName, "sphinx:schema" ) )
	{
		m_bInSchema = false;
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );
	}
	else if ( !strcmp ( szName, "sphinx:document" ) )
	{
		m_bInDocument = false;
		if ( m_pCurDocument )
			m_dParsedDocuments.Add ( m_pCurDocument );
		m_pCurDocument = NULL;
	}
	else if ( m_bInDocument && ( m_iCurAttr != -1 || m_iCurField != -1 ) )
	{
		if ( m_iElementDepth == 0 )
		{
			if ( m_iCurField != -1 )
			{
				assert ( m_pCurDocument );
				m_pCurDocument->m_dFields [m_iCurField].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
			}
			else if ( m_iCurAttr != -1 )
			{
				assert ( m_pCurDocument );
				m_pCurDocument->m_dAttrs [m_iCurAttr].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
			}

			m_iFieldBufferLen = 0;

			m_iCurAttr = -1;
			m_iCurField = -1;
		}
		else
			m_iElementDepth--;
	}
}


void CSphSource_XMLPipe2::Characters ( const char * pCharacters, int iLen )
{
	if ( m_iCurAttr == -1 && m_iCurField == -1 )
		return;
	
	if ( iLen + m_iFieldBufferLen < MAX_FIELD_LENGTH )
	{
		memcpy ( m_pFieldBuffer + m_iFieldBufferLen, pCharacters, iLen );
		m_iFieldBufferLen += iLen;
	}
	else
	{
		const CSphString & sName = ( m_iCurField != -1 ) ? m_tSchema.m_dFields [m_iCurField].m_sName : m_tSchema.GetAttr ( m_iCurAttr ).m_sName;

		bool bWarned = false;
		for ( int i = 0; i < m_dWarned.GetLength () && !bWarned; i++ )
			bWarned = m_dWarned [i] == sName;

		if ( !bWarned )
		{
#if USE_LIBEXPAT
			sphWarn ( "source '%s': field/attribute '%s' length exceeds max length (line=%d, pos=%d, docid=" DOCID_FMT ")",
				m_tSchema.m_sName.cstr(), sName.cstr(),
				XML_GetCurrentLineNumber(m_pParser), XML_GetCurrentColumnNumber(m_pParser),
				m_pCurDocument->m_iDocID );
#endif

#if USE_LIBXML
			sphWarn ( "source '%s': field/attribute '%s' length exceeds max length (docid=" DOCID_FMT ")",
				m_tSchema.m_sName.cstr(), sName.cstr(), m_pCurDocument->m_iDocID );
#endif
			m_dWarned.Add ( sName );
		}
	}
}


#if USE_LIBXML
int CSphSource_XMLPipe2::ReadBuffer ( BYTE * pBuffer, int iLen )
{
	int iLeft = Max ( m_pBufferEnd - m_pBufferPtr, 0 );
	if ( iLeft < iLen )
	{
		m_bPassedBufferEnd = !!m_pBufferEnd;

		memmove ( m_pBuffer, m_pBufferPtr, iLeft );
		size_t iRead = fread ( m_pBuffer + iLeft, 1, m_iBufferSize - iLeft, m_pPipe );
		m_pBufferPtr = m_pBuffer;
		m_pBufferEnd = m_pBuffer + iLeft + iRead;

		iLeft = Max ( m_pBufferEnd - m_pBuffer, 0 );
	}

	int iToCopy = Min ( iLen, iLeft );
	memcpy ( pBuffer, m_pBufferPtr, iToCopy );
	m_pBufferPtr += iToCopy;

	return iToCopy;
}


void CSphSource_XMLPipe2::ProcessNode ( xmlTextReaderPtr pReader )
{
	int iType = xmlTextReaderNodeType ( pReader );
	switch ( iType )
	{
	case XML_READER_TYPE_ELEMENT:
		{
			const char * szName = (char*)xmlTextReaderName ( pReader );

			m_dAttrs.Resize ( 0 );

			if ( xmlTextReaderHasAttributes ( pReader ) )
			{
				if ( xmlTextReaderMoveToFirstAttribute ( pReader ) != 1 )
					return;

				do
				{
					int iLen = m_dAttrs.GetLength ();
					m_dAttrs.Resize ( iLen + 2 );
					m_dAttrs [iLen  ] = (char*)xmlTextReaderName ( pReader );
					m_dAttrs [iLen+1] = (char*)xmlTextReaderValue ( pReader );
				}
				while ( xmlTextReaderMoveToNextAttribute ( pReader ) == 1 );
			}

			int iLen = m_dAttrs.GetLength ();
			m_dAttrs.Resize ( iLen + 2 );
			m_dAttrs [iLen  ] = NULL;
			m_dAttrs [iLen+1] = NULL;

			StartElement ( szName, &(m_dAttrs [0]) );
		}
		break;
	case XML_READER_TYPE_END_ELEMENT:
		EndElement ( (char*)xmlTextReaderName ( pReader ) );
		break;	
	case XML_TEXT_NODE:
		{
			const char * szText = (char*)xmlTextReaderValue	( pReader );
			Characters ( szText, strlen ( szText ) );
		}
		break;
	}	
}
#endif

CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, const char * szSourceName )
{
	CSphSource_XMLPipe2 * pPipe = new CSphSource_XMLPipe2 ( szSourceName );
	if ( !pPipe->Setup ( *pSource ) )
		SafeDelete ( pPipe );

	return pPipe;
}

#endif

/////////////////////////////////////////////////////////////////////////////
// MERGER HELPERS
/////////////////////////////////////////////////////////////////////////////

CSphDoclistRecord::~CSphDoclistRecord()
{
	SafeDeleteArray ( m_pRowitems );
}


void CSphDoclistRecord::Inititalize ( int iRowitems )
{
	assert ( !m_pRowitems );
	m_iRowitems = iRowitems;
	if ( m_iRowitems>0 )
		m_pRowitems = new CSphRowitem [ m_iRowitems ];
}


void CSphDoclistRecord::Read( CSphMergeSource * pSource )
{	
	assert ( pSource );
	 
	CSphReader_VLN * pReader = pSource->m_pDoclistReader;

	assert ( pReader );

	m_iDocID = pSource->m_iLastDocID + pReader->UnzipDocid();
	pSource->m_iLastDocID = m_iDocID;

	if ( pSource->m_bForceDocinfo )
	{	
		pSource->m_tMatch.m_iDocID = m_iDocID;
		pSource->m_pIndex->LookupDocinfo ( pSource->m_tMatch );
	}

	for ( int i=0; i<m_iRowitems; i++ )
	{
		assert ( pSource->m_pMinAttrs );
		if ( pSource->m_bForceDocinfo )
			m_pRowitems[i] = pSource->m_tMatch.m_pRowitems[i];
		else
			m_pRowitems[i] = pReader->UnzipInt() + pSource->m_pMinAttrs[i];
	}

	m_iPos = pReader->UnzipOffset();
	m_uFields = pReader->UnzipInt();
	m_uMatchHits = pReader->UnzipInt ();
}

void CSphDoclistRecord::Write ( CSphMergeData * pData )
{
	assert ( pData );

	CSphWriter * pWriter = pData->m_pDataWriter;
	assert ( pWriter );
	
	pWriter->ZipOffset ( m_iDocID - pData->m_iLastDocID );
	pData->m_iLastDocID = m_iDocID;

	assert ( pData->m_pMinAttrs );

	if ( m_iRowitems )
	{
		if ( pData->m_eDocinfo == SPH_DOCINFO_INLINE )
		{
			for ( int i=0; i<m_iRowitems; i++ )
				pWriter->ZipInt ( m_pRowitems[i] - pData->m_pMinAttrs[i] );
		}
		else if ( pData->m_eDocinfo == SPH_DOCINFO_EXTERN )
		{
			for ( int i=0; i<m_iRowitems; i++ )
				pWriter->ZipInt ( m_pRowitems[i] );
		}
	}

	pWriter->ZipOffset( m_iPos );
	pWriter->ZipInt( m_uFields );
	pWriter->ZipInt( m_uMatchHits );
}

const CSphDoclistRecord & CSphDoclistRecord::operator = ( const CSphDoclistRecord & rhs )
{
	m_iDocID = rhs.m_iDocID;
	m_iPos = rhs.m_iPos;
	m_uFields = rhs.m_uFields;
	m_uMatchHits = rhs.m_uMatchHits;
	m_iLeadingZero = rhs.m_iLeadingZero;

	if ( m_iRowitems < rhs.m_iRowitems )
	{
		SafeDeleteArray ( m_pRowitems );
		m_pRowitems = new CSphRowitem [ rhs.m_iRowitems ];
	}
	m_iRowitems = rhs.m_iRowitems;

	for ( int i=0; i<m_iRowitems; i++ )
		m_pRowitems[i] = rhs.m_pRowitems[i];

	return *this;
}

void CSphWordDataRecord::Read( CSphMergeSource * pSource, CSphMergeData * pMergeData, int iDocNum )
{
	assert ( pSource );
	assert ( pMergeData );
	assert ( pMergeData->m_pPurgeData );
	
	CSphVector<DWORD> dWordPosIndex;
	dWordPosIndex.Reserve ( 1024 );

	DWORD iWordPos = 0;
	CSphReader_VLN * pReader = pSource->m_pDoclistReader;
	CSphReader_VLN * pHitlistReader = pSource->m_pHitlistReader;
	assert ( pReader && pHitlistReader );

	m_dWordPos.Reserve ( 1024 );
	m_dWordPos.Resize ( 0 );
	m_dDoclist.Reserve ( 1024 );
	m_dDoclist.Resize ( 0 );

	for( int i = 0; i < iDocNum; i++ )
	{
		dWordPosIndex.Add( m_dWordPos.GetLength() );
		do 
		{
			iWordPos = pHitlistReader->UnzipInt ();
			m_dWordPos.Add ( iWordPos );
			if ( iWordPos == 0 )
				break;
		} while ( iWordPos );
	}

	CSphDoclistRecord tDoc;

	for( int i = 0; i < iDocNum; i++ )
	{
		tDoc.Inititalize ( m_iRowitems);
		tDoc.Read ( pSource );

		if ( pMergeData->m_pPurgeData->IsEnabled() )
		{
			if ( !pMergeData->m_pPurgeData->IsShouldPurge( ( const DWORD * ) tDoc.m_pRowitems ) )
				m_dDoclist.Add( tDoc );
			else
			{
				// remove word posistions
				assert ( i < dWordPosIndex.GetLength() );
				DWORD iStartIndex = dWordPosIndex[i];

				if ( i == ( dWordPosIndex.GetLength() - 1 ) )
				{
					if ( iStartIndex > 0)
						m_dWordPos.Resize( iStartIndex + 1 );
					else
						m_dWordPos.Reset();
				}
				else
				{
					DWORD iLastIndex = dWordPosIndex[i + 1];
					int iRemoveSize = iLastIndex - iStartIndex;
					int iNewSize = m_dWordPos.GetLength() - iRemoveSize;

					for ( int iWordPos = iStartIndex; iWordPos < iNewSize; iWordPos++ )
						m_dWordPos[iWordPos] = m_dWordPos[iWordPos+iRemoveSize];
					m_dWordPos.Resize( iNewSize );
				}				
			}
		}
		else
			m_dDoclist.Add( tDoc );
	}
	
	m_iLeadingZero = pReader->UnzipInt ();
	assert( m_iLeadingZero == 0 );
}

void CSphWordDataRecord::Write ( CSphMergeData * pMergeData )
{
	assert ( pMergeData );
	assert ( m_dWordPos.GetLength() );
	assert ( m_dDoclist.GetLength() );

	CSphWriter * pWriter = pMergeData->m_pDataWriter;
	CSphWriter * pHitlistWriter = pMergeData->m_pHitlistWriter;
	assert ( pWriter && pHitlistWriter );

	int iDocCount = 0;

	pMergeData->m_iLastHitlistPos = 0;
	m_dDoclist[iDocCount++].m_iPos = pHitlistWriter->GetPos() - pMergeData->m_iLastHitlistPos;
	pMergeData->m_iLastHitlistPos = pHitlistWriter->GetPos();

	ARRAY_FOREACH ( i, m_dWordPos )
	{
		pHitlistWriter->ZipInt ( m_dWordPos[i] );
		if ( m_dWordPos[i] == 0 )
		{
			if ( iDocCount < m_dDoclist.GetLength() )
				m_dDoclist[iDocCount++].m_iPos = pHitlistWriter->GetPos() - pMergeData->m_iLastHitlistPos;
			pMergeData->m_iLastHitlistPos = pHitlistWriter->GetPos();
		}
	}

	pMergeData->m_iDoclistPos = pWriter->GetPos();

	ARRAY_FOREACH ( i, m_dDoclist )
		m_dDoclist[i].Write ( pMergeData );

	pWriter->ZipInt ( 0 );

	pMergeData->m_tStats.m_iTotalDocuments += m_dDoclist.GetLength();
}

bool CSphWordIndexRecord::Read ( const BYTE * & pSource, CSphMergeSource * pMergeSource )
{
	assert ( pSource );
	assert ( pMergeSource );
	assert ( pMergeSource->m_iMaxWordlistEntries );

	if ( pMergeSource->m_iWordlistEntries == pMergeSource->m_iMaxWordlistEntries )
	{
		sphUnzipInt( pSource ); /// reading '0'
		sphUnzipOffset( pSource ); /// reading doclist length
		m_iWordID = 0;
		pMergeSource->m_iWordlistEntries = 0;

		if ( pMergeSource->GetWordlistSize() == 0 )
			return false;
	}

	SphWordID_t iWordID = sphUnzipWordid( pSource );
	
	if ( !iWordID )
	{
		m_iDoclistPos = sphUnzipOffset( pSource );		
		m_iWordID = 0;

		return false;
	}
	else
	{
		m_iWordID += iWordID;
		m_iDoclistPos = sphUnzipOffset( pSource );
		m_iDocNum = sphUnzipInt( pSource );
		assert ( m_iDocNum );

		m_iHitNum = sphUnzipInt( pSource );		
		assert ( m_iHitNum );
	}

	return true;
}

void CSphWordIndexRecord::Write ( CSphWriter * pWriter, CSphMergeData * pMergeData )
{
	assert ( pWriter );
	assert ( pMergeData );

	pMergeData->m_iWordlistOffset = pWriter->GetPos();

	pWriter->ZipOffset ( m_iWordID - pMergeData->m_iLastWordID ); // FIXME! slow with 32bit wordids
	pWriter->ZipOffset ( m_iDoclistPos );
	pWriter->ZipInt ( m_iDocNum );
	pWriter->ZipInt ( m_iHitNum );

	pMergeData->m_iLastWordID = m_iWordID;
}

bool CSphWordRecord::Read ()
{
	assert ( m_pMergeSource );
	assert ( m_pMergeData );
	assert ( m_pMergeSource->Check() );

	// read word index data
	if ( m_tWordIndex.Read( m_pMergeSource->m_pWordlist, m_pMergeSource ) )
	{
		m_pMergeSource->m_iWordlistEntries++;

		// setup
		m_tWordData.m_iWordID = m_tWordIndex.m_iWordID;
		m_tWordData.m_iRowitems = m_pMergeSource->m_iRowitems;
		m_pMergeSource->m_iLastDocID = m_pMergeSource->m_iMinDocID;

		// read doclist data
		m_tWordData.Read( m_pMergeSource, m_pMergeData, m_tWordIndex.m_iDocNum );

		// update docs counter for "purge" reason
		m_tWordIndex.m_iDocNum = m_tWordData.m_dDoclist.GetLength();

		return true;
	}
	else
		return false;
}

void CSphWordRecord::Write ()
{
	assert ( m_pMergeSource );
	assert ( m_pMergeData );
	assert ( m_pMergeSource->Check() );
	
	m_tWordData.Write ( m_pMergeData );

	m_tWordIndex.m_iDoclistPos = m_pMergeData->m_iDoclistPos - m_pMergeData->m_iLastDoclistPos;
	m_pMergeData->m_iLastDoclistPos = m_pMergeData->m_iDoclistPos;
	
	m_tWordIndex.Write ( m_pMergeData->m_pIndexWriter, m_pMergeData );

	m_pMergeData->m_iLastDocID = m_pMergeData->m_iMinDocID;
}

void CSphDocMVA::Read( CSphReader_VLN & tReader )
{
	m_iDocID = tReader.GetDocid();
	if ( m_iDocID )
	{
		ARRAY_FOREACH( i, m_dMVA )
		{
			DWORD iValues = tReader.GetDword();
			m_dMVA[i].Resize( iValues );
			tReader.GetBytes( &m_dMVA[i][0], iValues * sizeof(DWORD) );
		}
	}	
}

void CSphDocMVA::Write( CSphWriter & tWriter )
{
	tWriter.PutDocid ( m_iDocID );
	ARRAY_FOREACH ( i, m_dMVA )
	{
		m_dOffsets[i] = ( DWORD )tWriter.GetPos() / sizeof( DWORD );

		int iValues = m_dMVA[i].GetLength();
		tWriter.PutDword ( iValues );
		tWriter.PutBytes ( &m_dMVA[i][0], iValues*sizeof(DWORD) );
	}
}

/////////////////////////////////////////////////////////////////////////////

void sphSetQuiet ( bool bQuiet )
{
	g_bSphQuiet = bQuiet;
}

//
// $Id$
//
