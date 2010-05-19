//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
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

#if USE_ZLIB
#include <zlib.h>
#endif

#if USE_ODBC
#include <sql.h>
#endif

#if USE_WINDOWS
	#include <io.h> // for open()

	// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
	#define snprintf	_snprintf
	#define sphSeek		_lseeki64

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

	#define sphSeek		lseek

	#define struct_stat		struct stat
#endif

#if ( USE_WINDOWS && USE_MYSQL )
	#pragma comment(linker, "/defaultlib:libmysql.lib")
	#pragma message("Automatically linking with libmysql.lib")
#endif

#if ( USE_WINDOWS && USE_PGSQL )
	#pragma comment(linker, "/defaultlib:libpq.lib")
	#pragma message("Automatically linking with libpq.lib")
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

// forward decl
void sphWarn ( const char * sTemplate, ... );
size_t sphReadThrottled ( int iFD, void * pBuf, size_t iCount );
static bool sphTruncate ( int iFD );

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *	SPHINX_DEFAULT_SBCS_TABLE	= "0..9, A..Z->a..z, _, a..z, U+A8->U+B8, U+B8, U+C0..U+DF->U+E0..U+FF, U+E0..U+FF";
const char *	SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F";

const char		MAGIC_WORD_HEAD				= 1;
const char		MAGIC_WORD_TAIL				= 1;
const char		MAGIC_WORD_HEAD_NONSTEMMED	= 2;
const char		MAGIC_SYNONYM_WHITESPACE	= 1;

static const int	DEFAULT_READ_BUFFER		= 262144;
static const int	DEFAULT_READ_UNHINTED	= 32768;
static const int	MIN_READ_BUFFER			= 8192;
static const int	MIN_READ_UNHINTED		= 1024;


static bool						g_bSphQuiet					= false;
static SphErrorCallback_fn		g_pInternalErrorCallback	= NULL;
static SphWarningCallback_fn	g_pInternalWarningCallback	= NULL;

static int					g_iReadBuffer				= DEFAULT_READ_BUFFER;
static int					g_iReadUnhinted				= DEFAULT_READ_UNHINTED;

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
	#include "sphinxtimers.h" // NOLINT
	#undef DECLARE_TIMER
};


struct CSphTimer
{
	int64_t			m_iMicroSec;		///< time as clocked raw
	int				m_iCalls;			///< number of times this timer was called

	int				m_iChildrenCalls;	///< number of times all subtimers (children, grandchildren etc) of this timer were called
	int64_t			m_iMicroSecAdj;		///< guessed (!) time after timer costs adjustment, including subtimer costs
	int64_t			m_iMicroSecSelf;	///< guessed (!) self time

	ESphTimer		m_eTimer;
	int				m_iParent;
	int				m_iChild;
	int				m_iNext;
	int				m_iPrev;

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
		m_iMicroSec = 0;
		m_iMicroSecAdj = 0;
		m_iCalls = 0;
		m_iChildrenCalls = 0;
	}

	void Start ()
	{
		m_iMicroSec -= sphMicroTimer ();
		m_iCalls++;
	}

	void Stop ()
	{
		m_iMicroSec += sphMicroTimer ();
	}
};

static const int	SPH_MAX_TIMERS					= 128;
static const int	SPH_TIMER_TRIALS				= 16384;

static int			g_iTimer						= -1;
static int			g_iTimers						= 0;
static CSphTimer	g_dTimers [ SPH_MAX_TIMERS ];
static int64_t		g_iTimerTrialsWall				= 0;

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
	for ( iTimer=g_dTimers[g_iTimer].m_iChild; iTimer>0; iTimer=g_dTimers[iTimer].m_iNext )
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


static void sphProfilerAdjust ( int iTimer )
{
	CSphTimer & tTimer = g_dTimers[iTimer];
	tTimer.m_iChildrenCalls = 0;

	// adjust all my children first
	// count the subtimer call totals along the way, too
	for ( int iChild=tTimer.m_iChild; iChild>0; iChild=g_dTimers[iChild].m_iNext )
	{
		sphProfilerAdjust ( iChild );
		tTimer.m_iChildrenCalls += g_dTimers[iChild].m_iCalls + g_dTimers[iChild].m_iChildrenCalls;
	}

	// adjust my raw time, remove all the timer costs from it
	// my own costs are 1x sphMicroTimer() call per start/stop cycle
	// subtimer costs are 2x sphMicroTimer() calls per start/stop cycle
	tTimer.m_iMicroSecAdj = tTimer.m_iMicroSec - ( ( tTimer.m_iCalls + 2*tTimer.m_iChildrenCalls )*g_iTimerTrialsWall / SPH_TIMER_TRIALS );

	// now calculate self time
	// as adjusted time (all subtimer costs removed) minus all subtimer self time
	tTimer.m_iMicroSecSelf = tTimer.m_iMicroSecAdj;
	for ( int iChild=tTimer.m_iChild; iChild>0; iChild=g_dTimers[iChild].m_iNext )
		tTimer.m_iMicroSecSelf -= g_dTimers[iChild].m_iMicroSecSelf;
}


void sphProfilerDone ()
{
	assert ( g_iTimers>0 );
	assert ( g_iTimer==0 );

	// stop root timer
	g_iTimers = 0;
	g_iTimer = -1;
	g_dTimers[0].Stop ();

	// bench adjustments
	for ( int iRun=0; iRun<3; iRun++ )
	{
		int64_t iTrial = sphMicroTimer();
		for ( int i=0; i<SPH_TIMER_TRIALS-1; i++ )
			sphMicroTimer();
		iTrial = sphMicroTimer()-iTrial;

		if ( iRun!=0 )
			g_iTimerTrialsWall = Min ( g_iTimerTrialsWall, iTrial );
		else
			g_iTimerTrialsWall = iTrial;
	}

	// apply those adjustments
	sphProfilerAdjust ( 0 );
}


void sphProfilerShow ( int iTimer=0, int iLevel=0 )
{
	assert ( g_iTimers==0 );
	assert ( g_iTimer==-1 );

	if ( iTimer==0 )
		fprintf ( stdout, "--- PROFILE ---\n" );

	// show this timer
	CSphTimer & tTimer = g_dTimers[iTimer];
	if ( tTimer.m_iMicroSec<50 )
		return;

	char sName[32];
	for ( int i=0; i<iLevel; i++ )
		sName[2*i] = sName[2*i+1] = ' ';
	sName[2*iLevel] = '\0';
	strncat ( sName, g_dTimerNames [ tTimer.m_eTimer ], sizeof(sName) );

	fprintf ( stdout, "%-32s | %6d.%02d ms | %6d.%02d ms self | %d calls\n",
		sName,
		(int)(tTimer.m_iMicroSecAdj/1000), (int)(tTimer.m_iMicroSecAdj%1000)/10,
		(int)(tTimer.m_iMicroSecSelf/1000), (int)(tTimer.m_iMicroSecSelf%1000)/10,
		tTimer.m_iCalls );

	// dump my children
	int iChild = tTimer.m_iChild;
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
	explicit CSphEasyTimer ( ESphTimer eTimer )
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
	return __rdtsc();
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
	explicit CSphClocker ( INT64 * pTarget )
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

#else

bool g_bHeadProcess = true;

void sphSetProcessInfo ( bool bHead )
{
	g_bHeadProcess = bHead;
}

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


static size_t sphRead ( int iFD, void * pBuf, size_t iCount )
{
	int64_t tmStart = 0;
	if ( g_bIOStats )
		tmStart = sphMicroTimer();

	size_t uRead = (size_t) ::read ( iFD, pBuf, iCount );

	if ( g_bIOStats )
	{
		g_IOStats.m_iReadTime += sphMicroTimer() - tmStart;
		g_IOStats.m_iReadOps++;
		g_IOStats.m_iReadBytes += iCount;
	}

	return uRead;
}


static void GetFileStats ( const char * szFilename, CSphSavedFile & tInfo );

/////////////////////////////////////////////////////////////////////////////
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
protected:
	int			m_iFD;			///< my file descriptior
	CSphString	m_sFilename;	///< my file name
	bool		m_bTemporary;	///< whether to unlink this file on Close()

	CSphIndex::ProgressCallback_t *		m_pProgress; ///< for displaying progress
	CSphIndexProgress *					m_pStat;

public:
	CSphAutofile ()
		: m_iFD ( -1 )
		, m_bTemporary ( false )
		, m_pProgress ( NULL )
		, m_pStat ( NULL )
	{
	}

	CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false )
		: m_iFD ( -1 )
		, m_bTemporary ( false )
		, m_pProgress ( NULL )
		, m_pStat ( NULL )
	{
		Open ( sName, iMode, sError, bTemp );
	}

	~CSphAutofile ()
	{
		Close ();
	}

	int Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false )
	{
		assert ( m_iFD==-1 && m_sFilename.IsEmpty () );
		assert ( !sName.IsEmpty() );

		m_iFD = ::open ( sName.cstr(), iMode, 0644 );
		m_sFilename = sName; // not exactly sure why is this uncoditional. for error reporting later, i suppose

		if ( m_iFD<0 )
			sError.SetSprintf ( "failed to open %s: %s", sName.cstr(), strerror(errno) );
		else
			m_bTemporary = bTemp; // only if we managed to actually open it

		return m_iFD;
	}

	void Close ()
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
		struct_stat st;
		if ( stat ( GetFilename(), &st )<0 )
		{
			sError.SetSprintf ( "failed to stat %s: %s", GetFilename(), strerror(errno) );
			return -1;
		}
		if ( st.st_size<iMinSize )
		{
			sError.SetSprintf ( "failed to load %s: bad size "INT64_FMT" (at least "INT64_FMT" bytes expected)",
				GetFilename(), (int64_t)st.st_size, (int64_t)iMinSize );
			return -1;
		}
		if ( bCheckSizeT )
		{
			size_t sCheck = (size_t)st.st_size;
			if ( st.st_size!=SphOffset_t(sCheck) )
			{
				sError.SetSprintf ( "failed to load %s: bad size "INT64_FMT" (out of size_t; 4 GB limit on 32-bit machine hit?)",
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
		int64_t iCount = (int64_t) uCount;
		int64_t iToRead = iCount;
		BYTE * pCur = (BYTE *)pBuf;
		while ( iToRead>0 )
		{
			int64_t iToReadOnce = ( m_pProgress && m_pStat )
				? Min ( SPH_READ_PROGRESS_CHUNK, iToRead )
				: Min ( SPH_READ_NOPROGRESS_CHUNK, iToRead );
			int64_t iGot = (int64_t) sphRead ( GetFD(), pCur, (size_t)iToReadOnce );
			if ( iGot<=0 )
				break;

			iToRead -= iGot;
			pCur += iGot;

			if ( m_pProgress && m_pStat )
			{
				m_pStat->m_iBytes += iGot;
				m_pProgress ( m_pStat, false );
			}
		}

		if ( iToRead!=0 )
		{
			sError.SetSprintf ( "read error in %s; "INT64_FMT" of "INT64_FMT" bytes read",
				GetFilename(), iCount-iToRead, iCount );
			return false;
		}
		return true;
	}

	void SetProgressCallback ( CSphIndex::ProgressCallback_t * pfnProgress, CSphIndexProgress * pStat )
	{
		m_pProgress = pfnProgress;
		m_pStat = pStat;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// array pointer which self-destructs when going out of scope, or on demand
template < typename T > class CSphAutoArray
{
protected:
	T *		m_pData;

public:
	explicit	CSphAutoArray ( int iCount )	{ m_pData = ( iCount>0 ) ? new T [ iCount ] : NULL; }
				~CSphAutoArray ()				{ Reset (); }

	void		Reset ()						{ SafeDeleteArray ( m_pData ); }

	const CSphAutoArray & operator = ( const CSphAutoArray & )		{ assert(0); return *this; }
	operator T * ()													{ return m_pData; }
};

/////////////////////////////////////////////////////////////////////////////

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
	explicit CSphQueue ( int iSize )
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
	BIN_READ_OK,			///< bin read ok
	BIN_READ_EOF,			///< bin end
	BIN_READ_ERROR,			///< bin read error
	BIN_PRECACHE_OK,		///< precache ok
	BIN_PRECACHE_ERROR		///< precache failed
};


/// aggregated hit info
struct CSphAggregateHit
{
	SphDocID_t		m_iDocID;		///< document ID
	SphWordID_t		m_iWordID;		///< word ID in current dictionary
	union
	{
		DWORD		m_iWordPos;		///< word position in current document
		DWORD		m_uHitCount;	///< number of hits in current document
	};
	DWORD			m_uFieldMask;	///< mask of fields containing this word
};


/// bin, block input buffer
struct CSphBin
{
	static const int	MIN_SIZE	= 8192;
	static const int	WARN_SIZE	= 262144;

protected:
	ESphHitless			m_eMode;
	int					m_iSize;

	BYTE *				m_dBuffer;
	BYTE *				m_pCurrent;
	int					m_iLeft;
	int					m_iDone;
	ESphBinState		m_eState;
	CSphAggregateHit	m_tLastHit;		///< last decoded hit

	int					m_iFile;		///< my file
	SphOffset_t *		m_pFilePos;		///< shared current offset in file

public:
	SphOffset_t			m_iFilePos;		///< my current offset in file
	int					m_iFileLeft;	///< how much data is still unread from the file

public:
	explicit 			CSphBin ( ESphHitless eMode = SPH_HITLESS_NONE );
						~CSphBin ();

	static int			CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn = true );
	void				Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize );

	SphWordID_t			ReadVLB ();
	int					ReadByte ();
	ESphBinRead			ReadBytes ( void * pDest, int iBytes );
	int					ReadHit ( CSphAggregateHit * pHit, int iRowitems, CSphRowitem * pRowitems );

	bool				IsEOF () const;
	ESphBinRead			Precache ();
};

/////////////////////////////////////////////////////////////////////////////

#define READ_NO_SIZE_HINT 0

/////////////////////////////////////////////////////////////////////////////

/// search filter attribute types
enum ESphFilterAttr
{
	SPH_FILTERATTR_ATTR		= 0,
	SPH_FILTERATTR_ID		= 1,
	SPH_FILTERATTR_WEIGHT	= 2
};

class CSphIndex_VLN;

/// everything required to setup search term
class DiskIndexQwordSetup_c : public ISphQwordSetup
{
public:
	const CSphAutofile &	m_tDoclist;
	const CSphAutofile &	m_tHitlist;
	const CSphAutofile &	m_tWordlist;
	bool					m_bSetupReaders;

public:
	DiskIndexQwordSetup_c ( const CSphAutofile & tDoclist, const CSphAutofile & tHitlist, const CSphAutofile & tWordlist )
		: m_tDoclist ( tDoclist )
		, m_tHitlist ( tHitlist )
		, m_tWordlist ( tWordlist )
		, m_bSetupReaders ( false )
	{}

	virtual ISphQword *					QwordSpawn ( const XQKeyword_t & ) const;
	virtual bool						QwordSetup ( ISphQword * ) const;

protected:
	template < class T >	bool		Setup ( ISphQword * ) const;
};


#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif


/// query word from the searcher's point of view
class DiskIndexQwordTraits_c : public ISphQword
{
public:
	SphOffset_t		m_uHitPosition;
	DWORD			m_uInlinedHit;
	DWORD			m_uHitState;

	bool			m_bDupe;		///< whether the word occurs only once in current query

	CSphMatch		m_tDoc;			///< current match (partial)
	DWORD			m_iHitPos;		///< current hit postition, from hitlist

	CSphReader	m_rdDoclist;	///< my doclist reader
	CSphReader	m_rdHitlist;	///< my hitlist reader

	SphDocID_t		m_iMinID;		///< min ID to fixup
	int				m_iInlineAttrs;	///< inline attributes count
	CSphRowitem *	m_pInlineFixup;	///< inline attributes fixup (POINTER TO EXTERNAL DATA, NOT MANAGED BY THIS CLASS!)

#ifndef NDEBUG
	bool			m_bHitlistOver;
#endif

public:
	DiskIndexQwordTraits_c ()
		: m_uHitPosition ( 0 )
		, m_uHitState ( 0 )
		, m_bDupe ( false )
		, m_iHitPos ( 0 )
		, m_iMinID ( 0 )
		, m_iInlineAttrs ( 0 )
		, m_pInlineFixup ( NULL )
#ifndef NDEBUG
		, m_bHitlistOver ( true )
#endif
	{}
};


/// query word from the searcher's point of view
template < bool INLINE_HITS, bool INLINE_DOCINFO, bool DISABLE_HITLIST_SEEK >
class DiskIndexQword_c : public DiskIndexQwordTraits_c
{
public:
	virtual void Reset ()
	{
		m_uHitPosition = 0;
		m_uHitState = 0;
		m_rdDoclist.Reset ();
		m_rdHitlist.Reset ();
		ISphQword::Reset();
		m_iHitPos = 0;
		m_iInlineAttrs = 0;
	}

	void GetHitlistEntry ()
	{
		assert ( !m_bHitlistOver );
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

	virtual const CSphMatch & GetNextDoc ( DWORD * pDocinfo )
	{
		SphDocID_t iDelta = m_rdDoclist.UnzipDocid();
		if ( iDelta )
		{
			m_tDoc.m_iDocID += iDelta;
			if ( INLINE_DOCINFO )
			{
				assert ( pDocinfo );
				for ( int i=0; i<m_iInlineAttrs; i++ )
					pDocinfo[i] = m_rdDoclist.UnzipInt() + m_pInlineFixup[i];
			}

			if ( INLINE_HITS )
			{
				m_uMatchHits = m_rdDoclist.UnzipInt();
				const DWORD uFirst = m_rdDoclist.UnzipInt();
				if ( m_uMatchHits==1 )
				{
					const DWORD uField = m_rdDoclist.UnzipInt(); // field and end marker
					m_iHitlistPos = uFirst | ( uField << 23 ) | ( U64C(1)<<63 );
					m_uFields = 1 << ( uField >> 1 );
				} else
				{
					m_uFields = uFirst;
					m_uHitPosition += m_rdDoclist.UnzipOffset();
					m_iHitlistPos = m_uHitPosition;
				}
			} else
			{
				SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset();
				assert ( iDeltaPos>=0 );

				m_iHitlistPos += iDeltaPos;

				m_uFields = m_rdDoclist.UnzipInt();
				m_uMatchHits = m_rdDoclist.UnzipInt();
			}
		} else
		{
			m_tDoc.m_iDocID = 0;
		}
		return m_tDoc;
	}

	virtual void SeekHitlist ( SphOffset_t uOff )
	{
		if ( uOff >> 63 )
		{
			m_uHitState = 1;
			m_uInlinedHit = (DWORD)uOff; // truncate high dword
		} else
		{
			m_uHitState = 0;
			m_iHitPos = 0;
			if ( DISABLE_HITLIST_SEEK )
				assert ( m_rdHitlist.GetPos()==uOff ); // make sure we're where caller thinks we are.
			else
				m_rdHitlist.SeekTo ( uOff, READ_NO_SIZE_HINT );
		}
#ifndef NDEBUG
		m_bHitlistOver = false;
#endif
	}

	virtual DWORD GetNextHit ()
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
				return 0;
		}
		sphDie ( "INTERNAL ERROR: impossible hit emitter state" );
		return 0;
	}
};

#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const for MSVC
#endif

//////////////////////////////////////////////////////////////////////////////

#define WITH_QWORD(INDEX, NO_SEEK, NAME, ACTION)													\
	{																								\
	CSphIndex_VLN * pIndex = (CSphIndex_VLN *)INDEX;												\
	DWORD uInlineHits = pIndex->m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE;					\
	DWORD uInlineDocinfo = pIndex->m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE;						\
																									\
	switch ( ( uInlineHits<<1 ) | uInlineDocinfo )													\
	{																								\
		case 0: { typedef DiskIndexQword_c < false, false, NO_SEEK > NAME; ACTION; break; }			\
		case 1: { typedef DiskIndexQword_c < false, true, NO_SEEK > NAME; ACTION; break; }			\
		case 2: { typedef DiskIndexQword_c < true, false, NO_SEEK > NAME; ACTION; break; }			\
		case 3: { typedef DiskIndexQword_c < true, true, NO_SEEK > NAME; ACTION; break; }			\
		default:																					\
			sphDie ( "INTERNAL ERROR: impossible qword settings" );									\
	}																								\
}

/////////////////////////////////////////////////////////////////////////////

struct CSphWordlistCheckpoint
{
	SphWordID_t			m_iWordID;
	SphOffset_t			m_iWordlistOffset;
};

// pre-v11 wordlist checkpoint
struct CSphWordlistCheckpoint_v10
{
	SphWordID_t			m_iWordID;
	DWORD				m_iWordlistOffset;
};

struct CSphDocMVA
{
	SphDocID_t							m_iDocID;
	CSphVector < CSphVector<DWORD> >	m_dMVA;
	CSphVector < DWORD >				m_dOffsets;

	explicit CSphDocMVA ( int iSize )
		: m_iDocID ( 0 )
	{
		m_dMVA.Resize ( iSize );
		m_dOffsets.Resize ( iSize );
	}

	void	Read ( CSphReader & tReader );
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
	Swap ( a.m_iTag, b.m_iTag );
}

//////////////////////////////////////////////////////////////////////////
static void ReadFileInfo ( CSphReader & tReader, const char * szFilename, CSphString & sWarning )
{
	SphOffset_t uSize = tReader.GetOffset ();
	SphOffset_t uCTime = tReader.GetOffset ();
	SphOffset_t uMTime = tReader.GetOffset ();
	DWORD uCRC32 = tReader.GetDword ();

	if ( szFilename && *szFilename )
	{
		struct_stat tFileInfo;
		if ( stat ( szFilename, &tFileInfo ) < 0 )
			sWarning.SetSprintf ( "failed to stat %s: %s", szFilename, strerror(errno) );
		else
		{
			DWORD uMyCRC32 = 0;
			if ( !sphCalcFileCRC32 ( szFilename, uMyCRC32 ) )
				sWarning.SetSprintf ( "failed to calculate CRC32 for %s", szFilename );
			else
				if ( uMyCRC32!=uCRC32 || tFileInfo.st_size!=uSize || tFileInfo.st_ctime!=uCTime || tFileInfo.st_mtime!=uMTime )
						sWarning.SetSprintf ( "'%s' differs from the original", szFilename );
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


class AttrIndexBuilder_c;

/// this is my actual VLN-compressed phrase index implementation
class CSphIndex_VLN : public CSphIndex
{
	friend class DiskIndexQwordSetup_c;
	friend class CSphMerger;
	friend class AttrIndexBuilder_c;

public:
	explicit					CSphIndex_VLN ( const char * sFilename );
								~CSphIndex_VLN ();

	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer );

	virtual bool				LoadHeader ( const char * sHeaderName, CSphString & sWarning );
	virtual bool				WriteHeader ( CSphWriter & fdInfo, SphOffset_t iCheckpointsPos, DWORD iCheckpointCount );

	virtual void				DebugDumpHeader ( FILE * fp, const char * sHeaderName );
	virtual void				DebugDumpDocids ( FILE * fp );
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID );
	virtual int					DebugCheck ( FILE * fp );
	template <class Qword> void	DumpHitlist ( FILE * fp, const char * sKeyword, bool bID );

	virtual bool				Prealloc ( bool bMlock, CSphString & sWarning );
	virtual bool				Mlock ();
	virtual void				Dealloc ();

	virtual bool				Preread ();
	template<typename T> bool	PrereadSharedBuffer ( CSphSharedBuffer<T> & pBuffer, const char * sExt, size_t uExpected=0 );

	virtual void				SetBase ( const char * sNewBase );
	virtual bool				Rename ( const char * sNewBase );

	virtual bool				Lock ();
	virtual void				Unlock ();

	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const;
	template <class Qword> bool	DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const;

	virtual bool				Merge ( CSphIndex * pSource, CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists );
	template <class QWORDDST, class QWORDSRC> bool MergeWords ( CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter );

	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError );
	virtual bool				SaveAttributes ();

	bool						EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const;

	virtual SphAttr_t *			GetKillList () const;
	virtual int					GetKillListSize () const { return m_iKillListSize; }
	virtual bool				HasDocid ( SphDocID_t uDocid ) const;

	virtual const CSphSourceStats &		GetStats () const { return m_tStats; }

private:
	static const int			WORDLIST_CHECKPOINT		= 1024;		///< wordlist checkpoints frequency

	static const int			MIN_WRITE_BUFFER		= 262144;	///< min write buffer size
	static const int			DEFAULT_WRITE_BUFFER	= 1048576;	///< deafult write buffer size

	static const DWORD			INDEX_MAGIC_HEADER		= 0x58485053;	///< my magic 'SPHX' header
	static const DWORD			INDEX_FORMAT_VERSION	= 20;			///< my format version

private:
	// common stuff
	CSphString					m_sFilename;
	int							m_iLockFD;

	CSphMatch					m_tMin;				///< min attribute values tracker
	CSphSourceStats				m_tStats;			///< my stats
	SphDocID_t					m_iMergeInfinum;	///< minimal docid-1 for merging

	CSphVector<CSphWordlistCheckpoint>	m_dWordlistCheckpoints;	///< wordlist checkpoint offsets

private:
	// indexing-only
	BYTE *						m_pWriteBuffer;		///< my write buffer (for temp files)
	int							m_iWriteBuffer;		///< my write buffer size

	CSphWordHit					m_tLastHit;				///< hitlist entry

	SphOffset_t					m_iLastHitlistPos;		///< doclist entry
	SphOffset_t					m_iLastHitlistDelta;	///< doclist entry
	DWORD						m_uLastDocFields;		///< doclist entry
	DWORD						m_uLastDocHits;			///< doclist entry

	SphOffset_t					m_iLastDoclistPos;		///< wordlist entry
	int							m_iLastWordDocs;		///< wordlist entry
	int							m_iLastWordHits;		///< wordlist entry

	int							m_iWordlistEntries;		///< wordlist entries written since last checkpoint

	CSphWriter					m_wrWordlist;	///< wordlist writer
	CSphWriter					m_wrDoclist;	///< wordlist writer
	CSphWriter					m_wrHitlist;	///< hitlist writer

	CSphIndexProgress			m_tProgress;

	CSphVector<SphWordID_t>		m_dHitlessWords;

	bool						LoadHitlessWords ();

private:
	// searching-only, per-index
	static const int			DOCINFO_HASH_BITS	= 18;	// FIXME! make this configurable
	static const int			DOCINFO_INDEX_FREQ	= 128;	// FIXME! make this configurable

	CSphSharedBuffer<DWORD>		m_pDocinfo;				///< my docinfo cache
	DWORD						m_uDocinfo;				///< my docinfo cache size
	CSphSharedBuffer<DWORD>		m_pDocinfoHash;			///< hashed ids, to accelerate lookups
	DWORD						m_uDocinfoIndex;		///< docinfo "index" entries count (each entry is 2x docinfo rows, for min/max)
	DWORD *						m_pDocinfoIndex;		///< docinfo "index", to accelerate filtering during full-scan (2x rows for each block, and 2x rows for the whole index, 1+m_uDocinfoIndex entries)

	CSphSharedBuffer<DWORD>		m_pMva;					///< my multi-valued attrs cache
	CSphSharedBuffer<BYTE>		m_pStrings;				///< my in-RAM strings cache

	SphOffset_t					m_iCheckpointsPos;		///< wordlist checkpoints offset
	CSphSharedBuffer<BYTE>		m_pWordlist;			///< my wordlist cache

	CSphSharedBuffer<SphAttr_t>	m_pKillList;			///< killlist
	DWORD						m_iKillListSize;		///< killlist size (in elements)

	DWORD						m_uMinMaxIndex;			///< stored min/max cache offset (counted in DWORDs)

	SphOffset_t					m_iWordlistSize;		///< wordlist file size
	BYTE *						m_pWordlistChunkBuf;	///< buffer for wordlist chunks

	CSphAutofile				m_tDoclistFile;			///< doclist file
	CSphAutofile				m_tHitlistFile;			///< hitlist file
	CSphAutofile				m_tWordlistFile;		///< wordlist file

	bool						m_bPreallocated;		///< are we ready to preread
	CSphSharedBuffer<BYTE>		m_bPreread;				///< are we ready to search
	DWORD						m_uVersion;				///< data files version
	bool						m_bUse64;				///< whether the header is id64

	int							m_iIndexTag;			///< my ids for MVA updates pool
	static int					m_iIndexTagSeq;			///< static ids sequence

private:
	CSphString					GetIndexFileName ( const char * sExt ) const;

	int							AdjustMemoryLimit ( int iMemoryLimit );

	int							cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits, DWORD * pDocinfo, int Docinfos, int iStride );
	void						cidxHit ( CSphAggregateHit * pHit, CSphRowitem * pDocinfos );
	bool						cidxDone ( const char * sHeaderExtension );

	void						WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol );
	void						ReadSchemaColumn ( CSphReader & rdInfo, CSphColumnInfo & tCol );

	bool						ParsedMultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const XQNode_t * pRoot, CSphDict * pDict, const CSphVector<CSphFilterSettings> * pExtraFilters, CSphQueryNodeCache * pNodeCache, int iTag ) const;
	bool						MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const;
	bool						MatchExtended ( CSphQueryContext * pCtx, const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters, ISphRanker * pRanker, int iTag ) const;

	const DWORD *				FindDocinfo ( SphDocID_t uDocID ) const;
	void						CopyDocinfo ( CSphQueryContext * pCtx, CSphMatch & tMatch, const DWORD * pFound ) const;

	bool						BuildMVA ( const CSphVector<CSphSource*> & dSources, CSphAutoArray<CSphWordHit> & dHits, int iArenaSize, int iFieldFD, int nFieldMVAs, int iFieldMVAInPool );

	CSphDict *					SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, ISphTokenizer & tTokenizer ) const;
	CSphDict *					SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer & tTokenizer ) const;

	void						LoadSettings ( CSphReader & tReader );
	void						SaveSettings ( CSphWriter & tWriter );

	bool						RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize, SphOffset_t * pFileSize, CSphBin * pMinBin, SphOffset_t * pSharedOffset );
	bool						PrecomputeMinMax();

private:
	static const int MAX_ORDINAL_STR_LEN	= 4096;	///< maximum ordinal string length in bytes
	static const int ORDINAL_READ_SIZE		= 262144;	///< sorted ordinal id read buffer size in bytes

	ESphBinRead					ReadOrdinal ( CSphBin & Reader, Ordinal_t & Ordinal );
	SphOffset_t					DumpOrdinals ( CSphWriter & Writer, CSphVector<Ordinal_t> & dOrdinals );
	bool						SortOrdinals ( const char * szToFile, int iFromFD, int iArenaSize, int iOrdinalsInPool, CSphVector< CSphVector<SphOffset_t> > & dOrdBlockSize, bool bWarnOfMem );
	bool						SortOrdinalIds ( const char * szToFile, int iFromFD, int iArenaSize, CSphVector < CSphVector < SphOffset_t > > & dOrdBlockSize, bool bWarnOfMem );

	const DWORD *				GetMVAPool () const { return m_pMva.GetNumEntries() ? &m_pMva[0] : NULL; }
};

int CSphIndex_VLN::m_iIndexTagSeq = 0;

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

/// timer initializer
static int64_t g_tmInitTimer = sphMicroTimer();

/// time since startup, in microseconds
int64_t sphMicroTimer()
{
#if USE_WINDOWS
	// Windows time query
	static int64_t iStart = 0;
	static int64_t iFreq = 0;

	LARGE_INTEGER iLarge;
	if ( !iFreq )
	{
		QueryPerformanceFrequency ( &iLarge ); iFreq = iLarge.QuadPart;
		QueryPerformanceCounter ( &iLarge ); iStart = iLarge.QuadPart;
	}

	QueryPerformanceCounter ( &iLarge );
	return ( iLarge.QuadPart-iStart )*1000000/iFreq;

#else
	// UNIX time query
	static int s_sec = -1, s_usec = -1;
	struct timeval tv;

	if ( s_sec==-1 )
	{
		gettimeofday ( &tv, NULL );
		s_sec = tv.tv_sec;
		s_usec = tv.tv_usec;
	}

	gettimeofday ( &tv, NULL );
	return (int64_t)(tv.tv_sec-s_sec)*int64_t(1000000) + (int64_t)(tv.tv_usec-s_usec);
#endif // USE_WINDOWS
}

//////////////////////////////////////////////////////////////////////////

static int		g_iMaxIOps		= 0;
static int		g_iMaxIOSize	= 0;
static int64_t	g_tmLastIOTime	= 0;


void sphSetThrottling ( int iMaxIOps, int iMaxIOSize )
{
	g_iMaxIOps = iMaxIOps;
	g_iMaxIOSize = iMaxIOSize;
}


static inline void sphThrottleSleep ()
{
	if ( g_iMaxIOps>0 )
	{
		int64_t tmTimer = sphMicroTimer();
		int64_t tmSleep = Max ( 0, g_tmLastIOTime + 1000000/g_iMaxIOps - tmTimer );
		sphSleepMsec ( (int)(tmSleep/1000) );
		g_tmLastIOTime = tmTimer + tmSleep;
	}
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

	// while there's data, write it chunk by chunk
	const BYTE * p = (const BYTE*) pBuf;
	while ( iCount>0 )
	{
		// wait for a timely occasion
		sphThrottleSleep ();

		// write (and maybe time)
		int64_t tmTimer = 0;
		if ( g_bIOStats )
			tmTimer = sphMicroTimer();

		int iToWrite = iChunkSize;
		if ( iCount<iChunkSize )
			iToWrite = (int)iCount;

		int iWritten = ::write ( iFD, p, iToWrite );

		if ( g_bIOStats )
		{
			g_IOStats.m_iWriteTime += sphMicroTimer() - tmTimer;
			g_IOStats.m_iWriteOps++;
			g_IOStats.m_iWriteBytes += iToWrite;
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
			sError.SetSprintf ( "%s: write error: %s", sName, strerror(errno) );
		else
			sError.SetSprintf ( "%s: write error: %d of %d bytes written", sName, iWritten, iToWrite );
		return false;
	}
	return true;
}


size_t sphReadThrottled ( int iFD, void * pBuf, size_t iCount )
{
	if ( g_iMaxIOSize && int(iCount) > g_iMaxIOSize )
	{
		size_t nChunks = iCount / g_iMaxIOSize;
		size_t nBytesLeft = iCount % g_iMaxIOSize;

		size_t nBytesRead = 0;
		size_t iRead = 0;

		for ( size_t i=0; i<nChunks; i++ )
		{
			iRead = sphReadThrottled ( iFD, (char *)pBuf + i*g_iMaxIOSize, g_iMaxIOSize );
			nBytesRead += iRead;
			if ( iRead!=(size_t)g_iMaxIOSize )
				return nBytesRead;
		}

		if ( nBytesLeft > 0 )
		{
			iRead = sphReadThrottled ( iFD, (char *)pBuf + nChunks*g_iMaxIOSize, nBytesLeft );
			nBytesRead += iRead;
			if ( iRead!=nBytesLeft )
				return nBytesRead;
		}

		return nBytesRead;
	}

	sphThrottleSleep ();
	return sphRead ( iFD, pBuf, iCount );
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


char * sphStrMacro ( const char * sTemplate, const char * sMacro, SphDocID_t uValue )
{
	// expand macro
	char sExp[32];
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
		strcpy ( sOut, sExp ); sOut += iExp; // NOLINT
		sCur += iMacro;
		sLast = sCur;
	}

	if ( *sLast )
		strcpy ( sOut, sLast ); // NOLINT

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


uint64_t sphToUint64 ( const char * s )
{
	if ( !s ) return 0;
	return strtoull ( s, NULL, 10 );
}


int64_t sphToInt64 ( const char * s )
{
	if ( !s ) return 0;
	return strtoll ( s, NULL, 10 );
}


#if USE_64BIT
#define sphToDocid sphToUint64
#else
#define sphToDocid sphToDword
#endif


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

	int iCmd = bWait ? F_SETLKW : F_SETLK; // FIXME! check for HAVE_F_SETLKW?
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


void sphSleepMsec ( int iMsec )
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


void sphSetInternalErrorCallback ( void ( *fnCallback ) ( const char * ) )
{
	g_pInternalErrorCallback = fnCallback;
}

void sphSetWarningCallback ( SphWarningCallback_fn fnCallback )
{
	g_pInternalWarningCallback = fnCallback;
}

void sphCallWarningCallback ( const char * sFmt, ... )
{
	if ( !g_pInternalWarningCallback )
		return;

	char sBuf [ 1024 ];
	va_list ap;
	va_start ( ap, sFmt );
	vsnprintf ( sBuf, sizeof(sBuf), sFmt, ap );
	va_end ( ap );

	g_pInternalWarningCallback ( sBuf );
}


void sphSetReadBuffers ( int iReadBuffer, int iReadUnhinted )
{
	if ( iReadBuffer<=0 )
		iReadBuffer = DEFAULT_READ_BUFFER;
	g_iReadBuffer = Max ( iReadBuffer, MIN_READ_BUFFER );

	if ( iReadUnhinted<=0 )
		iReadUnhinted = DEFAULT_READ_UNHINTED;
	g_iReadUnhinted = Max ( iReadUnhinted, MIN_READ_UNHINTED );
}

//////////////////////////////////////////////////////////////////////////
// DOCINFO
//////////////////////////////////////////////////////////////////////////

#define MVA_DOWNSIZE		DWORD			// MVA offset type
#define MVA_OFFSET_MASK		0x7fffffffUL	// MVA offset mask
#define MVA_ARENA_FLAG		0x80000000UL	// MVA global-arena flag


static DWORD *				g_pMvaArena = NULL;		///< initialized by sphArenaInit()


// OPTIMIZE! try to inline or otherwise simplify maybe
const DWORD * CSphMatch::GetAttrMVA ( const CSphAttrLocator & tLoc, const DWORD * pPool ) const
{
	DWORD uIndex = MVA_DOWNSIZE ( GetAttr ( tLoc ) );
	if ( !uIndex )
		return NULL;

	if ( uIndex & MVA_ARENA_FLAG )
		return g_pMvaArena + ( uIndex & MVA_OFFSET_MASK );

	assert ( pPool );
	return pPool + uIndex;
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif
inline int sphUTF8Decode ( BYTE * & pBuf ); // forward ref for GCC
inline int sphUTF8Encode ( BYTE * pBuf, int iCode ); // forward ref for GCC


/// synonym list entry
struct CSphSynonym
{
	CSphString	m_sFrom;	///< specially packed list of map-from tokens
	CSphString	m_sTo;		///< map-to string
	int			m_iFromLen;	///< cached m_sFrom length
	int			m_iToLen;	///< cached m_sTo length

	inline bool operator < ( const CSphSynonym & rhs ) const
	{
		return strcmp ( m_sFrom.cstr(), rhs.m_sFrom.cstr() ) < 0;
	}
};


/// tokenizer implementation traits
template < bool IS_UTF8 >
class CSphTokenizerTraits : public ISphTokenizer
{
public:
	CSphTokenizerTraits ();

	virtual bool			SetCaseFolding ( const char * sConfig, CSphString & sError );
	virtual bool			LoadSynonyms ( const char * sFilename, CSphString & sError );
	virtual void			CloneBase ( const CSphTokenizerTraits<IS_UTF8> * pFrom, bool bEscaped );

	virtual const char *	GetTokenStart () const		{ return (const char *) m_pTokenStart; }
	virtual const char *	GetTokenEnd () const		{ return (const char *) m_pTokenEnd; }
	virtual const char *	GetBufferPtr () const		{ return (const char *) m_pCur; }
	virtual const char *	GetBufferEnd () const		{ return (const char *) m_pBufferMax; }
	virtual void			SetBufferPtr ( const char * sNewPtr );
	virtual int				SkipBlended ();

protected:
	BYTE *	GetTokenSyn ();
	void	BlendAdjust ( BYTE * pPosition );
	int		CodepointArbitration ( int iCodepoint, int iLastCodepoint, bool bSpaceAhead );

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
				: int ( *m_pCur++ );
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
	BYTE *				m_pTokenStart;						///< last token start point
	BYTE *				m_pTokenEnd;						///< last token end point

	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< folded token accumulator
	BYTE *				m_pAccum;							///< current accumulator position
	int					m_iAccum;							///< boundary token size

	CSphVector<CSphSynonym>			m_dSynonyms;				///< active synonyms
	CSphVector<int>					m_dSynStart;				///< map 1st byte to candidate range start
	CSphVector<int>					m_dSynEnd;					///< map 1st byte to candidate range end

	BYTE *	m_pBlendStart;
	BYTE *	m_pBlendEnd;
};


/// single-byte charset tokenizer
class CSphTokenizer_SBCS : public CSphTokenizerTraits<false>
{
public:
								CSphTokenizer_SBCS ();

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength );
	virtual BYTE *				GetToken ();
	virtual ISphTokenizer *		Clone ( bool bEscaped ) const;
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
	virtual ISphTokenizer *		Clone ( bool bEscaped ) const;
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
	CSphString			m_sNgramCharsStr;
};


struct CSphMultiform
{
	CSphString				m_sNormalForm;
	int						m_iNormalTokenLen;
	CSphVector<CSphString>	m_dTokens;
};


struct CSphMultiforms
{
	int						m_iMinTokens;
	int						m_iMaxTokens;
	CSphVector<CSphMultiform*> m_dWordforms;
};


struct CSphMultiformContainer
{
							CSphMultiformContainer () : m_iMaxTokens ( 0 ) {}

	int						m_iMaxTokens;
	typedef CSphOrderedHash < CSphMultiforms *, CSphString, CSphStrHashFunc, 131072, 117 > CSphMultiformHash;
	CSphMultiformHash	m_Hash;
};


/// Token filter
class CSphTokenizer_Filter : public ISphTokenizer
{
public:
									CSphTokenizer_Filter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer );
									~CSphTokenizer_Filter ();

	virtual bool					SetCaseFolding ( const char * sConfig, CSphString & sError )	{ return m_pTokenizer->SetCaseFolding ( sConfig, sError ); }
	virtual void					AddCaseFolding ( CSphRemapRange & tRange )						{ m_pTokenizer->AddCaseFolding ( tRange ); }
	virtual void					AddSpecials ( const char * sSpecials )							{ m_pTokenizer->AddSpecials ( sSpecials ); }
	virtual bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError )	{ return m_pTokenizer->SetIgnoreChars ( sIgnored, sError ); }
	virtual bool					SetNgramChars ( const char * sConfig, CSphString & sError )		{ return m_pTokenizer->SetNgramChars ( sConfig, sError ); }
	virtual void					SetNgramLen ( int iLen )										{ m_pTokenizer->SetNgramLen ( iLen ); }
	virtual bool					LoadSynonyms ( const char * sFilename, CSphString & sError )	{ return m_pTokenizer->LoadSynonyms ( sFilename, sError ); }
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError )		{ return m_pTokenizer->SetBoundary ( sConfig, sError ); }
	virtual void					Setup ( const CSphTokenizerSettings & tSettings )				{ m_pTokenizer->Setup ( tSettings ); }
	virtual const CSphTokenizerSettings &	GetSettings () const									{ return m_pTokenizer->GetSettings (); }
	virtual const CSphSavedFile &	GetSynFileInfo () const											{ return m_pTokenizer->GetSynFileInfo (); }

public:
	virtual void					SetBuffer ( BYTE * sBuffer, int iLength )	{ m_pTokenizer->SetBuffer ( sBuffer, iLength ); }
	virtual BYTE *					GetToken ();
	virtual int						GetCodepointLength ( int iCode ) const		{ return m_pTokenizer->GetCodepointLength ( iCode ); }
	virtual void					EnableQueryParserMode ( bool bEnable )		{ m_pTokenizer->EnableQueryParserMode ( bEnable ); }
	virtual int						GetLastTokenLen () const					{ return m_pLastToken->m_iTokenLen; }
	virtual bool					GetBoundary ()								{ return m_pLastToken->m_bBoundary; }
	virtual bool					WasTokenSpecial ()							{ return m_pLastToken->m_bSpecial; }
	virtual int						GetOvershortCount ()						{ return m_pLastToken->m_iOvershortCount; }

public:
	virtual ISphTokenizer *			Clone ( bool bEscaped ) const;
	virtual bool					IsUtf8 () const				{ return m_pTokenizer->IsUtf8 (); }
	virtual const char *			GetTokenStart () const		{ return m_pLastToken->m_szTokenStart; }
	virtual const char *			GetTokenEnd () const		{ return m_pLastToken->m_szTokenEnd; }
	virtual const char *			GetBufferPtr () const		{ return m_pTokenizer->GetBufferPtr (); }
	virtual const char *			GetBufferEnd () const		{ return m_pTokenizer->GetBufferEnd (); }
	virtual void					SetBufferPtr ( const char * sNewPtr );

private:
	ISphTokenizer *					m_pTokenizer;
	const CSphMultiformContainer *	m_pMultiWordforms;
	int								m_iStoredStart;
	int								m_iStoredLen;

	struct StoredToken_t
	{
		BYTE			m_sToken [3*SPH_MAX_WORD_LEN+4];
		int				m_iTokenLen;
		bool			m_bBoundary;
		bool			m_bSpecial;
		int				m_iOvershortCount;
		const char *	m_szTokenStart;
		const char *	m_szTokenEnd;
	};

	CSphVector<StoredToken_t>		m_dStoredTokens;
	StoredToken_t					m_tLastToken;
	StoredToken_t *					m_pLastToken;

	void							FillTokenInfo ( StoredToken_t * pToken );
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

enum
{
	MASK_CODEPOINT			= 0x00ffffffUL,	// mask off codepoint flags
	MASK_FLAGS				= 0xff000000UL, // mask off codepoint value
	FLAG_CODEPOINT_SPECIAL	= 0x01000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL		= 0x02000000UL,	// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM	= 0x04000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_SYNONYM	= 0x08000000UL,	// this codepoint is used in synonym tokens only
	FLAG_CODEPOINT_BOUNDARY	= 0x10000000UL,	// this codepoint is phrase boundary
	FLAG_CODEPOINT_IGNORE	= 0x20000000UL,	// this codepoint is ignored
	FLAG_CODEPOINT_BLEND	= 0x40000000UL	// this codepoint is "blended" (indexed both as a character, and as a separator)
};


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
			bool bWordPart = ( iCodepoint & MASK_CODEPOINT ) && !( iCodepoint & FLAG_CODEPOINT_SYNONYM );
			int iNew = iRemapped | uFlags | ( iCodepoint & MASK_FLAGS );
			iCodepoint = bWordPart ? ( iNew | FLAG_CODEPOINT_DUAL ) : iNew;
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

const CSphLowercaser & CSphLowercaser::operator = ( const CSphLowercaser & rhs )
{
	SetRemap ( &rhs );
	return * this;
}

/////////////////////////////////////////////////////////////////////////////

/// parser to build lowercaser from textual config
class CSphCharsetDefinitionParser
{
public:
						CSphCharsetDefinitionParser () : m_bError ( false ) {}
	bool				Parse ( const char * sConfig, CSphVector<CSphRemapRange> & dRanges );
	const char *		GetLastError ();

protected:
	bool				m_bError;
	char				m_sError [ 1024 ];
	const char *		m_pCurrent;

	bool				Error ( const char * sMessage );
	void				SkipSpaces ();
	bool				IsEof ();
	bool				CheckEof ();
	int					HexDigit ( int c );
	int					ParseCharsetCode ();
};


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
		if ( ( iRemapEnd-iRemapStart )!=( iEnd-iStart ) )
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

//////////////////////////////////////////////////////////////////////////

bool sphParseCharset ( const char * sCharset, CSphVector<CSphRemapRange> & dRemaps )
{
	CSphCharsetDefinitionParser tParser;
	return tParser.Parse ( sCharset, dRemaps );
}

/////////////////////////////////////////////////////////////////////////////

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

	int iCode = ( v >> iBytes );
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


int sphUTF8Len ( const char * pStr )
{
	BYTE * pBuf = (BYTE*) pStr;
	int iRes = 0, iCode;

	while ( ( iCode = sphUTF8Decode(pBuf) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}


CSphSavedFile::CSphSavedFile ()
	: m_uSize	( 0 )
	, m_uCTime	( 0 )
	, m_uMTime	( 0 )
	, m_uCRC32	( 0 )
{
}


CSphTokenizerSettings::CSphTokenizerSettings ()
	: m_iType				( TOKENIZER_SBCS )
	, m_iMinWordLen			( 1 )
	, m_iNgramLen			( 0 )
{
}


static void LoadTokenizerSettings ( CSphReader & tReader, CSphTokenizerSettings & tSettings, DWORD uVersion, CSphString & sWarning )
{
	if ( uVersion<9 )
		return;

	tSettings.m_iType = tReader.GetByte ();
	tSettings.m_sCaseFolding = tReader.GetString ();
	tSettings.m_iMinWordLen = tReader.GetDword ();
	tSettings.m_sSynonymsFile = tReader.GetString ();
	ReadFileInfo ( tReader, tSettings.m_sSynonymsFile.cstr (), sWarning );
	tSettings.m_sBoundary = tReader.GetString ();
	tSettings.m_sIgnoreChars = tReader.GetString ();
	tSettings.m_iNgramLen = tReader.GetDword ();
	tSettings.m_sNgramChars = tReader.GetString ();
	if ( uVersion>=15 )
		tSettings.m_sBlendChars = tReader.GetString ();
}


static void SaveTokenizerSettings ( CSphWriter & tWriter, ISphTokenizer * pTokenizer )
{
	assert ( pTokenizer );

	const CSphTokenizerSettings & tSettings = pTokenizer->GetSettings ();
	tWriter.PutByte ( tSettings.m_iType );
	tWriter.PutString ( tSettings.m_sCaseFolding.cstr () );
	tWriter.PutDword ( tSettings.m_iMinWordLen );
	tWriter.PutString ( tSettings.m_sSynonymsFile.cstr () );
	WriteFileInfo ( tWriter, pTokenizer->GetSynFileInfo () );
	tWriter.PutString ( tSettings.m_sBoundary.cstr () );
	tWriter.PutString ( tSettings.m_sIgnoreChars.cstr () );
	tWriter.PutDword ( tSettings.m_iNgramLen );
	tWriter.PutString ( tSettings.m_sNgramChars.cstr () );
	tWriter.PutString ( tSettings.m_sBlendChars.cstr () );
}


void LoadDictionarySettings ( CSphReader & tReader, CSphDictSettings & tSettings, DWORD uVersion, CSphString & sWarning )
{
	if ( uVersion<9 )
		return;

	tSettings.m_sMorphology = tReader.GetString ();
	tSettings.m_sStopwords = tReader.GetString ();
	int nFiles = tReader.GetDword ();

	CSphString sFile;
	for ( int i = 0; i < nFiles; i++ )
	{
		sFile = tReader.GetString ();
		ReadFileInfo ( tReader, sFile.cstr (), sWarning );
	}

	tSettings.m_sWordforms = tReader.GetString ();
	ReadFileInfo ( tReader, tSettings.m_sWordforms.cstr (), sWarning );

	if ( uVersion>=13 )
		tSettings.m_iMinStemmingLen = tReader.GetDword ();
}


void SaveDictionarySettings ( CSphWriter & tWriter, CSphDict * pDict )
{
	const CSphDictSettings & tSettings = pDict->GetSettings ();

	tWriter.PutString ( tSettings.m_sMorphology.cstr () );
	tWriter.PutString ( tSettings.m_sStopwords.cstr () );
	const CSphVector <CSphSavedFile> & dSWFileInfos = pDict->GetStopwordsFileInfos ();
	tWriter.PutDword ( dSWFileInfos.GetLength () );
	ARRAY_FOREACH ( i, dSWFileInfos )
	{
		tWriter.PutString ( dSWFileInfos[i].m_sFilename.cstr () );
		WriteFileInfo ( tWriter, dSWFileInfos[i] );
	}

	const CSphSavedFile & tWFFileInfo = pDict->GetWordformsFileInfo ();

	tWriter.PutString ( tSettings.m_sWordforms.cstr () );
	WriteFileInfo ( tWriter, tWFFileInfo );

	tWriter.PutDword ( tSettings.m_iMinStemmingLen );
}


static inline bool ShortTokenFilter ( BYTE * pToken, int iLen )
{
	return pToken[0]=='*' || ( iLen > 0 && pToken[iLen-1]=='*' );
}

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer::ISphTokenizer ()
	: m_iLastTokenLen ( 0 )
	, m_bTokenBoundary ( false )
	, m_bBoundary ( false )
	, m_bWasSpecial ( false )
	, m_bEscaped ( false )
	, m_iOvershortCount ( 0 )
	, m_bShortTokenFilter ( false )
	, m_bQueryMode ( false )
	, m_bPhrase ( false )
{}


bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	m_tLC.Reset ();
	m_tLC.AddRemaps ( dRemaps, 0 );
	return true;
}


void ISphTokenizer::AddCaseFolding ( CSphRemapRange & tRange )
{
	CSphVector<CSphRemapRange> dTmp;
	dTmp.Add ( tRange );
	m_tLC.AddRemaps ( dTmp, 0 );
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


void ISphTokenizer::Setup ( const CSphTokenizerSettings & tSettings )
{
	m_tSettings = tSettings;
	m_tSettings.m_iMinWordLen = Max ( tSettings.m_iMinWordLen, 1 );
}


ISphTokenizer * ISphTokenizer::Create ( const CSphTokenizerSettings & tSettings, CSphString & sError )
{
	CSphScopedPtr<ISphTokenizer> pTokenizer ( NULL );

	switch ( tSettings.m_iType )
	{
		case TOKENIZER_SBCS:	pTokenizer = sphCreateSBCSTokenizer (); break;
		case TOKENIZER_UTF8:	pTokenizer = sphCreateUTF8Tokenizer (); break;
		case TOKENIZER_NGRAM:	pTokenizer = sphCreateUTF8NgramTokenizer (); break;
		default:
			sError.SetSprintf ( "failed to create tokenizer (unknown charset type '%d')", tSettings.m_iType );
			return NULL;
	}

	pTokenizer->Setup ( tSettings );

	if ( !tSettings.m_sCaseFolding.IsEmpty () && !pTokenizer->SetCaseFolding ( tSettings.m_sCaseFolding.cstr (), sError ) )
	{
		sError.SetSprintf ( "'charset_table': %s", sError.cstr() );
		return NULL;
	}

	if ( !tSettings.m_sSynonymsFile.IsEmpty () && !pTokenizer->LoadSynonyms ( tSettings.m_sSynonymsFile.cstr (), sError ) )
	{
		sError.SetSprintf ( "'synonyms': %s", sError.cstr() );
		return NULL;
	}

	if ( !tSettings.m_sBoundary.IsEmpty () && !pTokenizer->SetBoundary ( tSettings.m_sBoundary.cstr (), sError ) )
	{
		sError.SetSprintf ( "'phrase_boundary': %s", sError.cstr() );
		return NULL;
	}

	if ( !tSettings.m_sIgnoreChars.IsEmpty () && !pTokenizer->SetIgnoreChars ( tSettings.m_sIgnoreChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ignore_chars': %s", sError.cstr() );
		return NULL;
	}

	if ( !tSettings.m_sBlendChars.IsEmpty () && !pTokenizer->SetBlendChars ( tSettings.m_sBlendChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_chars': %s", sError.cstr() );
		return NULL;
	}

	pTokenizer->SetNgramLen ( tSettings.m_iNgramLen );

	if ( !tSettings.m_sNgramChars.IsEmpty () && !pTokenizer->SetNgramChars ( tSettings.m_sNgramChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ngram_chars': %s", sError.cstr() );
		return NULL;
	}

	return pTokenizer.LeakPtr ();
}


ISphTokenizer * ISphTokenizer::CreateTokenFilter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
{
	if ( !pContainer )
		return NULL;

	return new CSphTokenizer_Filter ( pTokenizer, pContainer );
}

//////////////////////////////////////////////////////////////////////////

template < bool IS_UTF8 >
CSphTokenizerTraits<IS_UTF8>::CSphTokenizerTraits ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_pTokenStart ( NULL )
	, m_pTokenEnd	( NULL )
	, m_iAccum		( 0 )
	, m_pBlendStart		( NULL )
	, m_pBlendEnd		( NULL )
{
	m_pAccum = m_sAccum;
}


template < bool IS_UTF8 >
bool CSphTokenizerTraits<IS_UTF8>::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	if ( m_dSynonyms.GetLength() )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}
	return ISphTokenizer::SetCaseFolding ( sConfig, sError );
}


template < bool IS_UTF8 >
bool CSphTokenizerTraits<IS_UTF8>::LoadSynonyms ( const char * sFilename, CSphString & sError )
{
	m_dSynonyms.Reset ();

	if ( !sFilename || !*sFilename )
		return true;

	GetFileStats ( sFilename, m_tSynFileInfo );

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
			iFromLen += strlen ( dFrom[i].cstr() ) + 1;

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
		int iCh = *(BYTE*)( m_dSynonyms[i].m_sFrom.cstr() );
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

	m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_SYNONYM );
	return true;
}


template < bool IS_UTF8 >
void CSphTokenizerTraits<IS_UTF8>::CloneBase ( const CSphTokenizerTraits<IS_UTF8> * pFrom, bool bEscaped )
{
	m_tLC = pFrom->m_tLC;
	m_dSynonyms = pFrom->m_dSynonyms;
	m_dSynStart = pFrom->m_dSynStart;
	m_dSynEnd = pFrom->m_dSynEnd;
	m_tSettings = pFrom->m_tSettings;
	m_bEscaped = bEscaped;

	if ( bEscaped )
	{
		CSphVector<CSphRemapRange> dRemaps;
		CSphRemapRange Range;
		Range.m_iStart = Range.m_iEnd = Range.m_iRemapStart = '\\';
		dRemaps.Add ( Range );
		m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_SPECIAL );
	}
}


template < bool IS_UTF8 >
void CSphTokenizerTraits<IS_UTF8>::SetBufferPtr ( const char * sNewPtr )
{
	assert ( (BYTE*)sNewPtr>=m_pBuffer && (BYTE*)sNewPtr<=m_pBufferMax );
	m_pCur = Min ( m_pBufferMax, Max ( m_pBuffer, (BYTE*)sNewPtr ) );
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}

template < bool IS_UTF8 >
int CSphTokenizerTraits<IS_UTF8>::SkipBlended()
{
	if ( m_pBlendEnd )
	{
		int iBlended = 0;
		bool bQueryMode = m_bQueryMode;
		m_bQueryMode = false;
		while ( m_pCur < m_pBlendEnd && GetToken() )
			iBlended++;
		m_bQueryMode = bQueryMode;
		return iBlended;
	}
	return 0;
}

template < bool IS_UTF8 >
void CSphTokenizerTraits<IS_UTF8>::BlendAdjust ( BYTE * pCur )
{
	// if all we got is a bunch of blended characters, let's pretend they
	// weren't really blended to make things easier for the caller
	m_bBlended = m_bBlended && m_bNonBlended;

	// adjust buffer pointers
	if ( m_bBlended && m_pBlendStart )
	{
		m_pCur = m_pBlendStart;
		m_pBlendEnd = pCur;
		m_pBlendStart = NULL;

	} else if ( pCur>=m_pBlendEnd )
	{
		m_pBlendEnd = NULL;
		m_pBlendStart = NULL;
	}

	m_bNonBlended = false;
}


static bool IsModifier ( int iSymbol )
{
	return iSymbol=='^' || iSymbol=='$' || iSymbol=='=' || iSymbol=='*';
}


template < bool IS_UTF8 >
int CSphTokenizerTraits<IS_UTF8>::CodepointArbitration ( int iCode, int iLastCodepoint, bool bSpaceAhead )
{
	if ( !m_bQueryMode )
		return iCode;

	if ( iCode & FLAG_CODEPOINT_NGRAM )
		return iCode; // ngrams are handled elsewhere

	int iSymbol = iCode & MASK_CODEPOINT;

	// codepoints can't be blended and special at the same time
	if ( ( iCode & FLAG_CODEPOINT_BLEND ) && ( iCode & FLAG_CODEPOINT_SPECIAL ) )
	{
		bool bBlend =
			( iLastCodepoint=='\\' ) || // escaped characters should always act as blended
			( m_bPhrase && !IsModifier ( iSymbol ) ) || // non-modifier special inside phrase
			( m_iAccum && ( iSymbol=='@' || iSymbol=='/' || iSymbol=='-' ) ); // some specials in the middle of a token

		// clear special or blend flags
		iCode &= bBlend
			? ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_SPECIAL )
			: ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_BLEND );
	}

	// escaped specials are not special
	// dash and dollar inside the word are not special
	// non-modifier specials within phrase are not special
	if ( iCode & FLAG_CODEPOINT_SPECIAL )
		if ( iLastCodepoint=='\\'
			|| ( m_iAccum && iSymbol=='-' )
			|| ( m_iAccum && iSymbol=='$' && !bSpaceAhead )
			|| ( m_bPhrase && iSymbol!='"' && !IsModifier ( iSymbol ) ) )
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

	assert ( sphBitCount ( iCode & MASK_FLAGS )<=1 ); // all conflicts must be resolved here
	return iCode;
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
	const BYTE * sCand = ( (const BYTE*)tCandidate.m_sFrom.cstr() ) + iOff;

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
	if ( iFolded<0 || ( iFolded & MASK_CODEPOINT )==0 )
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


template < bool IS_UTF8 >
BYTE * CSphTokenizerTraits<IS_UTF8>::GetTokenSyn ()
{
	assert ( m_dSynonyms.GetLength() );

	bool bEscaped = m_bEscaped;
	BYTE * pCur;

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

		int iLastCodepoint = 0;
		int iLastFolded = 0;
		BYTE * pRescan = NULL;

		int iExact = -1;
		BYTE * pExact = NULL;

		// main refinement loop
		for ( ;; )
		{
			// store current position (to be able to restart from it on folded boundary)
			pCur = m_pCur;

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

			if ( bEscaped )
			{
				if ( iCode=='\\' && iLastCodepoint!='\\' )
				{
					iLastCodepoint = iCode;
					continue;
				} else if ( iLastCodepoint=='\\' && !Special2Simple ( iFolded ) )
				{
					iLastCodepoint = 0;
					continue;
				}

				iLastCodepoint = iCode;
			}

			iLastFolded = iFolded;

			// handle specials at the very word start
			if ( ( iFolded & FLAG_CODEPOINT_SPECIAL ) && m_iAccum==0 )
			{
				m_bWasSpecial = !( iFolded & FLAG_CODEPOINT_NGRAM );

				AccumCodepoint ( iFolded & MASK_CODEPOINT );
				*m_pAccum = '\0';

				m_iLastTokenLen = 1;
				m_pTokenStart = pCur;
				m_pTokenEnd = m_pCur;
				return m_sAccum;
			}

			// handle specials
			bool bJustSpecial = ( iFolded & FLAG_CODEPOINT_SPECIAL )
				&& !( iFolded & FLAG_CODEPOINT_DUAL ) // OPTIMIZE?
				&& !( iFolded & FLAG_CODEPOINT_SYNONYM ); // OPTIMIZE?

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
				{
					if ( m_iAccum==0 )
						m_pTokenStart = pCur;

					AccumCodepoint ( iFolded & MASK_CODEPOINT );
				}
			}

			// accumulate next raw synonym symbol to refine
			// note that we need a special check for whitespace here, to avoid "MS*DOS" being treated as "MS DOS" synonym
			BYTE sTest[4];
			int iTest;

			int iMasked = ( iCode & MASK_CODEPOINT );
			if ( iFolded<=0 || bJustSpecial )
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
				m_pTokenEnd = pCur; \
				if ( bJustSpecial ) m_pCur = pCur; \
				strncpy ( (char*)m_sAccum, m_dSynonyms[_idx].m_sTo.cstr(), sizeof(m_sAccum) ); \
				m_iLastTokenLen = m_dSynonyms[_idx].m_iToLen; \
				return m_sAccum; \
			}

			#define LOC_REFINE_BREAK() \
			{ \
				if ( iExact>=0 ) { m_pCur = pCur = pExact; LOC_RETURN_SYNONYM ( iExact ); } \
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
			bool bMaybeSeparator = ( iFolded & FLAG_CODEPOINT_SYNONYM )!=0 || ( iFolded<0 );

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
				if ( eL==SYNCHECK_EXACT && iL==iSynStart )	LOC_RETURN_SYNONYM ( iL );

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
			int iLast = 0;

			// if there was none, scan until found
			for ( ;; )
			{
				pCur = m_pCur;
				int iCode = *pCur;
				int iFolded = m_tLC.ToLower ( GetCodepoint() );
				if ( iFolded<0 )
					break; // eof

				if ( bEscaped )
				{
					if ( iCode=='\\' && iLast!='\\' )
					{
						iLast = iCode;
						continue;
					}

					if ( iLast=='\\' && !Special2Simple ( iFolded ) )
						break;

					iLast = iCode;
				}

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
			pCur = m_pCur;
		}

		// return accumulated token
		if ( m_iAccum<m_tSettings.m_iMinWordLen )
		{
			if ( m_bShortTokenFilter )
			{
				*m_pAccum = '\0';

				if ( ShortTokenFilter ( m_sAccum, m_iAccum ) )
				{
					m_iLastTokenLen = m_iAccum;
					m_pTokenEnd = pCur;
					m_iAccum = 0;
					return m_sAccum;
				}
			}

			if ( m_iAccum )
				m_iOvershortCount++;

			m_iAccum = 0;
			continue;
		}

		*m_pAccum = '\0';
		m_iLastTokenLen = m_iAccum;
		m_pTokenEnd = pCur;
		return m_sAccum;
	}
}

bool ISphTokenizer::RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, CSphString & sError )
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
			sError.SetSprintf ( "%s characters must not be remapped (map-from=U+%x, map-to=U+%x)",
				sSource, dRemaps[i].m_iStart, dRemaps[i].m_iRemapStart );
			return false;
		}

		for ( int j=dRemaps[i].m_iStart; j<=dRemaps[i].m_iEnd; j++ )
			if ( m_tLC.ToLower(j) )
		{
			sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
			return false;
		}
	}

	// add mapping
	m_tLC.AddRemaps ( dRemaps, uFlags );
	return true;
}

bool ISphTokenizer::SetBoundary ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_BOUNDARY, "phrase boundary", sError );
}

bool ISphTokenizer::SetIgnoreChars ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_IGNORE, "ignored", sError );
}

bool ISphTokenizer::SetBlendChars ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_BLEND, "blend", sError );
}

//////////////////////////////////////////////////////////////////////////

static inline bool IsWhitespace ( BYTE c )
{
	return ( c=='\0' || c==' ' || c=='\t' || c=='\r' || c=='\n' );
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

	m_iOvershortCount = 0;
	m_bBoundary = m_bTokenBoundary = false;
}


BYTE * CSphTokenizer_SBCS::GetToken ()
{
	m_bWasSpecial = false;
	m_bBlended = false;
	m_iOvershortCount = 0;

	if ( m_dSynonyms.GetLength() )
		return GetTokenSyn ();

	bool bEscaped = m_bEscaped;
	int iCodepoint = 0;
	int iLastCodepoint = 0;
	BYTE * pCur = m_pCur;
	m_bTokenBoundary = false;
	for ( ;; )
	{
		// get next codepoint
		int iCode = 0;
		if ( m_pCur>=m_pBufferMax )
		{
			pCur = m_pCur;
			if ( m_iAccum<m_tSettings.m_iMinWordLen )
			{
				bool bShortToken = false;
				if ( m_bShortTokenFilter )
				{
					m_sAccum [m_iAccum] = '\0';
					if ( ShortTokenFilter ( m_sAccum, m_iAccum ) )
						bShortToken = true;
				}

				if ( !bShortToken )
				{
					m_iAccum = 0;
					m_iLastTokenLen = 0;
					return NULL;
				}
			}
		} else
		{
			pCur = m_pCur;
			iCodepoint = *m_pCur++;
			iCode = m_tLC.ToLower ( iCodepoint );
		}

		iCode = CodepointArbitration ( iCode, iLastCodepoint, IsWhitespace ( *m_pCur ) );

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle blended characters
		if ( iCode & FLAG_CODEPOINT_BLEND )
		{
			if ( m_pBlendEnd )
				iCode = 0;
			else
			{
				m_bBlended = true;
				m_pBlendStart = m_iAccum ? m_pTokenStart : pCur;
			}
		}

		if ( bEscaped )
		{
			if ( iCodepoint=='\\' && iLastCodepoint!='\\' )
			{
				iLastCodepoint = iCodepoint;
				continue;
			}

			if ( iLastCodepoint=='\\' && !Special2Simple ( iCode ) )
				iCode = 0;

			iLastCodepoint = iCodepoint;
		}

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) )
		{
			m_bTokenBoundary = true;
			m_iBoundaryOffset = pCur - m_pBuffer - 1;
		}
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;
		if ( iCode==0 || m_bBoundary )
		{
			if ( m_iAccum<m_tSettings.m_iMinWordLen )
			{
				bool bShortToken = false;
				if ( m_bShortTokenFilter )
				{
					m_sAccum[m_iAccum] = '\0';
					if ( ShortTokenFilter ( m_sAccum, m_iAccum ) )
						bShortToken = true;
				}

				if ( !bShortToken )
				{
					if ( m_iAccum )
						m_iOvershortCount++;

					m_iAccum = 0;
					BlendAdjust ( pCur );
					continue;
				}
			}

			m_iLastTokenLen = m_iAccum;
			m_sAccum[m_iAccum] = '\0';
			m_iAccum = 0;
			m_pTokenEnd = pCur>=m_pBufferMax ? m_pCur : pCur;
			BlendAdjust ( pCur );
			return m_sAccum;
		}

		// handle specials
		bool bSpecial = ( iCode & FLAG_CODEPOINT_SPECIAL )!=0;
		iCode &= MASK_CODEPOINT;
		if ( bSpecial )
		{
			// skip short words
			if ( m_iAccum<m_tSettings.m_iMinWordLen )
			{
				if ( m_iAccum )
					m_iOvershortCount++;

				bool bShortToken = false;
				if ( m_bShortTokenFilter )
				{
					m_sAccum[m_iAccum] = '\0';
					if ( ShortTokenFilter ( m_sAccum, m_iAccum ) )
						bShortToken = true;
				}

				if ( !bShortToken )
				{
					if ( m_iAccum )
						m_iOvershortCount++;

					m_iAccum = 0;
				}
			}

			m_pTokenEnd = m_pCur;

			if ( m_iAccum==0 )
			{
				// nice standalone special
				m_iLastTokenLen = 1;
				m_sAccum[0] = (BYTE)iCode;
				m_sAccum[1] = '\0';
				m_pTokenStart = pCur;
				m_bWasSpecial = true;

			} else
			{
				// flush prev accum and redo this special
				m_iLastTokenLen = m_iAccum;
				m_sAccum[m_iAccum] = '\0';
				m_pCur--;
			}

			m_iAccum = 0;
			BlendAdjust ( pCur );
			return m_sAccum;
		}

		// just accumulate
		assert ( iCode>0 );
		if ( m_iAccum<SPH_MAX_WORD_LEN )
		{
			if ( m_iAccum==0 )
				m_pTokenStart = pCur;

			m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );
			m_sAccum[m_iAccum++] = (BYTE)iCode;
		}
	}
}

ISphTokenizer * CSphTokenizer_SBCS::Clone ( bool bEscaped ) const
{
	CSphTokenizer_SBCS * pClone = new CSphTokenizer_SBCS ();
	pClone->CloneBase ( this, bEscaped );
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

	m_iOvershortCount = 0;
	m_bBoundary = m_bTokenBoundary = false;
}

BYTE * CSphTokenizer_UTF8::GetToken ()
{
	m_bBlended = false;
	m_bWasSpecial = false;
	m_iOvershortCount = 0;

	if ( m_dSynonyms.GetLength() )
		return GetTokenSyn ();

	bool bEscaped = m_bEscaped;
	int iLastCodepoint = 0;
	m_bTokenBoundary = false;
	for ( ;; )
	{
		// get next codepoint
		BYTE * pCur = m_pCur; // to redo special char, if there's a token already
		int iCodePoint = GetCodepoint(); // advances m_pCur
		int iCode = m_tLC.ToLower ( iCodePoint );

		// handle eof
		if ( iCode<0 )
		{
			// skip trailing short word
			FlushAccum ();
			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen )
			{
				if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) )
				{
					m_iLastTokenLen = 0;
					return NULL;
				}
			}

			// return trailing word
			BlendAdjust ( pCur );
			m_pTokenEnd = m_pCur;
			return m_sAccum;
		}

		iCode = CodepointArbitration ( iCode, iLastCodepoint, IsWhitespace ( *m_pCur ) );

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle blended characters
		if ( iCode & FLAG_CODEPOINT_BLEND )
		{
			if ( m_pBlendEnd )
				iCode = 0;
			else
			{
				m_bBlended = true;
				m_pBlendStart = m_iAccum ? m_pTokenStart : pCur;
			}
		}

		if ( bEscaped )
		{
			if ( iCodePoint=='\\' && iLastCodepoint!='\\' )
			{
				iLastCodepoint = iCodePoint;
				continue;
			}

			if ( iLastCodepoint=='\\' && !Special2Simple ( iCode ) )
				iCode = 0;

			iLastCodepoint = iCodePoint;
		}

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) )
		{
			m_bTokenBoundary = true;
			m_iBoundaryOffset = pCur - m_pBuffer - 1;
		}
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;
		if ( iCode==0 || m_bBoundary )
		{
			FlushAccum ();
			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen )
			{
				if ( m_bShortTokenFilter && ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) )
				{
					BlendAdjust ( pCur );
					m_pTokenEnd = pCur;
					return m_sAccum;
				} else
				{
					if ( m_iLastTokenLen )
						m_iOvershortCount++;

					BlendAdjust ( pCur );
					continue;
				}
			} else
			{
				BlendAdjust ( pCur );
				m_pTokenEnd = pCur;
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
				m_bWasSpecial = !( iCode & FLAG_CODEPOINT_NGRAM );
				m_pTokenStart = pCur;
				m_pTokenEnd = m_pCur;
				m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );
				AccumCodepoint ( iCode & MASK_CODEPOINT ); // handle special as a standalone token
			} else
			{
				m_pCur = pCur; // we need to flush current accum and then redo special char again
				m_pTokenEnd = pCur;
			}

			FlushAccum ();
			BlendAdjust ( pCur );
			return m_sAccum;
		}

		if ( m_iAccum==0 )
			m_pTokenStart = pCur;

		// just accumulate
		m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );
		AccumCodepoint ( iCode & MASK_CODEPOINT );
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


ISphTokenizer * CSphTokenizer_UTF8::Clone ( bool bEscaped ) const
{
	CSphTokenizer_UTF8 * pClone = new CSphTokenizer_UTF8 ();
	pClone->CloneBase ( this, bEscaped );
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

	m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL ); // !COMMIT support other n-gram lengths than 1
	m_sNgramCharsStr = sConfig;
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

//////////////////////////////////////////////////////////////////////////

CSphTokenizer_Filter::CSphTokenizer_Filter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
	: m_pTokenizer		( pTokenizer )
	, m_pMultiWordforms ( pContainer )
	, m_iStoredStart	( 0 )
	, m_iStoredLen		( 0 )
	, m_pLastToken		( NULL )
{
	assert ( pTokenizer && pContainer );
	m_dStoredTokens.Resize ( pContainer->m_iMaxTokens + 1 );
}


CSphTokenizer_Filter::~CSphTokenizer_Filter ()
{
	SafeDelete ( m_pTokenizer );
}


void CSphTokenizer_Filter::FillTokenInfo ( StoredToken_t * pToken )
{
	pToken->m_bBoundary = m_pTokenizer->GetBoundary ();
	pToken->m_bSpecial = m_pTokenizer->WasTokenSpecial ();
	pToken->m_iOvershortCount = m_pTokenizer->GetOvershortCount ();
	pToken->m_iTokenLen = m_pTokenizer->GetLastTokenLen ();
	pToken->m_szTokenStart = m_pTokenizer->GetTokenStart ();
	pToken->m_szTokenEnd = m_pTokenizer->GetTokenEnd ();
}


BYTE * CSphTokenizer_Filter::GetToken ()
{
	BYTE * pToken = ( m_iStoredLen>0 )
		? m_dStoredTokens [m_iStoredStart].m_sToken
		: m_pTokenizer->GetToken ();

	if ( !pToken )
	{
		memset ( &m_tLastToken, 0, sizeof ( m_tLastToken ) );
		m_pLastToken = &m_tLastToken;
		return NULL;
	}

	int iSize = m_dStoredTokens.GetLength ();

	CSphMultiforms ** pWordforms = m_pMultiWordforms->m_Hash ( (const char *)pToken );
	if ( !pWordforms )
	{
		if ( m_iStoredLen )
		{
			m_pLastToken = &(m_dStoredTokens[m_iStoredStart]);
			m_iStoredLen--;
			m_iStoredStart = (m_iStoredStart + 1) % iSize;
		} else
		{
			FillTokenInfo ( &m_tLastToken );
			m_pLastToken = &m_tLastToken;
		}

		return pToken;
	}

	if ( !m_iStoredLen )
	{
		FillTokenInfo ( &m_dStoredTokens[m_iStoredStart] );
		strcpy ( (char *)m_dStoredTokens[m_iStoredStart].m_sToken, (const char *)pToken ); // NOLINT
		m_iStoredLen++;
	}

	int iTokensNeeded = (*pWordforms)->m_iMaxTokens - m_iStoredLen + 1;
	for ( int i = 0; i < iTokensNeeded; i++ )
	{
		pToken = m_pTokenizer->GetToken ();

		if ( !pToken )
			break;

		int iIndex = (m_iStoredStart+m_iStoredLen) % iSize;
		FillTokenInfo ( &(m_dStoredTokens[iIndex]) );
		strcpy ( (char *)m_dStoredTokens[iIndex].m_sToken, (const char *)pToken ); // NOLINT
		m_iStoredLen++;
	}

	if ( !m_iStoredLen )
		return NULL;

	if ( m_iStoredLen<=(*pWordforms)->m_iMinTokens )
	{
		m_pLastToken = &(m_dStoredTokens [m_iStoredStart]);
		m_iStoredLen--;
		m_iStoredStart = (m_iStoredStart + 1) % iSize;
		return m_pLastToken->m_sToken;
	}

	ARRAY_FOREACH ( i, (*pWordforms)->m_dWordforms )
	{
		CSphMultiform * pCurForm = (*pWordforms)->m_dWordforms[i];

		if ( m_iStoredLen<=pCurForm->m_dTokens.GetLength () )
			continue;

		bool bFound = true;
		for ( int j = 0; j < pCurForm->m_dTokens.GetLength (); j++ )
		{
			int iIndex = ( m_iStoredStart + j + 1 ) % iSize;
			const char * szStored = (const char*)m_dStoredTokens[iIndex].m_sToken;
			const char * szNormal = pCurForm->m_dTokens[j].cstr ();

			if ( *szNormal!=*szStored || strcasecmp ( szNormal, szStored ) )
			{
				bFound = false;
				break;
			}
		}

		if ( bFound )
		{
			int iTokensPerForm = 1+pCurForm->m_dTokens.GetLength();

			m_tLastToken.m_bBoundary = false;
			m_tLastToken.m_bSpecial = false;
			m_tLastToken.m_iOvershortCount = m_dStoredTokens[m_iStoredStart].m_iOvershortCount;
			m_tLastToken.m_iTokenLen = pCurForm->m_iNormalTokenLen;
			m_tLastToken.m_szTokenStart = m_dStoredTokens[m_iStoredStart].m_szTokenStart;
			m_tLastToken.m_szTokenEnd = m_dStoredTokens[ ( m_iStoredStart+iTokensPerForm-1 ) % iSize ].m_szTokenEnd;
			m_pLastToken = &m_tLastToken;

			m_iStoredStart = ( m_iStoredStart+iTokensPerForm ) % iSize;
			m_iStoredLen -= iTokensPerForm;

			assert ( m_iStoredLen>=0 );
			return (BYTE*)pCurForm->m_sNormalForm.cstr ();
		}
	}

	pToken = m_dStoredTokens[m_iStoredStart].m_sToken;
	m_pLastToken = &(m_dStoredTokens[m_iStoredStart]);

	m_iStoredStart = (m_iStoredStart + 1) % iSize;
	m_iStoredLen--;

	return pToken;
}


ISphTokenizer * CSphTokenizer_Filter::Clone ( bool bEscaped ) const
{
	ISphTokenizer * pClone = m_pTokenizer->Clone ( bEscaped );
	return CreateTokenFilter ( pClone, m_pMultiWordforms );
}


void CSphTokenizer_Filter::SetBufferPtr ( const char * sNewPtr )
{
	m_iStoredLen = 0;
	m_iStoredStart = 0;
	m_pTokenizer->SetBufferPtr ( sNewPtr );
}


/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////

CSphFilterSettings::CSphFilterSettings ()
	: m_sAttrName	( "" )
	, m_bExclude	( false )
	, m_uMinValue	( 0 )
	, m_uMaxValue	( UINT_MAX )
	, m_pValues		( NULL )
	, m_nValues		( 0 )
{}


CSphFilterSettings::CSphFilterSettings ( const CSphFilterSettings & rhs )
{
	assert ( 0 );
	(*this) = rhs;
}


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
	: m_sIndexes	( "*" )
	, m_sQuery		( "" )
	, m_sRawQuery	( "" )
	, m_iOffset		( 0 )
	, m_iLimit		( 20 )
	, m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_eMode		( SPH_MATCH_ALL )
	, m_eRanker		( SPH_RANK_DEFAULT )
	, m_eSort		( SPH_SORT_RELEVANCE )
	, m_iMaxMatches	( 1000 )
	, m_eGroupFunc		( SPH_GROUPBY_ATTR )
	, m_sGroupSortBy	( "@groupby desc" )
	, m_sGroupDistinct	( "" )
	, m_iCutoff			( 0 )
	, m_iRetryCount		( 0 )
	, m_iRetryDelay		( 0 )
	, m_bGeoAnchor		( false )
	, m_fGeoLatitude	( 0.0f )
	, m_fGeoLongitude	( 0.0f )
	, m_uMaxQueryMsec	( 0 )
	, m_sComment		( "" )
	, m_sSelect			( "" )

	, m_iOldVersion		( 0 )
	, m_iOldGroups		( 0 )
	, m_pOldGroups		( NULL )
	, m_iOldMinTS		( 0 )
	, m_iOldMaxTS		( UINT_MAX )
	, m_iOldMinGID		( 0 )
	, m_iOldMaxGID		( UINT_MAX )
{}


CSphQuery::~CSphQuery ()
{
}


int CSphQuery::GetIndexWeight ( const char * sName ) const
{
	ARRAY_FOREACH ( i, m_dIndexWeights )
		if ( m_dIndexWeights[i].m_sName==sName )
			return m_dIndexWeights[i].m_iValue;
	return 1;
}

//////////////////////////////////////////////////////////////////////////

struct SelectBounds_t
{
	int		m_iStart;
	int		m_iEnd;
};
#define YYSTYPE SelectBounds_t
#include "yysphinxselect.h"


class SelectParser_t
{
public:
	int				GetToken ( YYSTYPE * lvalp );
	void			AddItem ( YYSTYPE * pExpr, YYSTYPE * pAlias, ESphAggrFunc eAggrFunc=SPH_AGGR_NONE );

public:
	CSphString		m_sParserError;
	const char *	m_pLastTokenStart;

	const char *	m_pStart;
	const char *	m_pCur;

	CSphQuery *		m_pQuery;
};

int yylex ( YYSTYPE * lvalp, SelectParser_t * pParser )				{ return pParser->GetToken ( lvalp );}
void yyerror ( SelectParser_t * pParser, const char * sMessage )	{ pParser->m_sParserError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart ); }
#include "yysphinxselect.c"


int SelectParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace ( *m_pCur ) ) m_pCur++;
	if ( !*m_pCur ) return 0;

	m_pLastTokenStart = m_pCur;
	lvalp->m_iStart = m_pCur-m_pStart;

	// check for constant
	if ( isdigit ( *m_pCur ) )
	{
		char * pEnd = NULL;
		strtod ( m_pCur, &pEnd );

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

		LOC_CHECK ( "AS", 2, SEL_AS );
		LOC_CHECK ( "OR", 2, TOK_OR );
		LOC_CHECK ( "AND", 3, TOK_AND );
		LOC_CHECK ( "NOT", 3, TOK_NOT );
		LOC_CHECK ( "AVG", 3, SEL_AVG );
		LOC_CHECK ( "MIN", 3, SEL_MIN );
		LOC_CHECK ( "MAX", 3, SEL_MAX );
		LOC_CHECK ( "SUM", 3, SEL_SUM );

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
	}

	// return char as a token
	return *m_pCur++;
}


void SelectParser_t::AddItem ( YYSTYPE * pExpr, YYSTYPE * pAlias, ESphAggrFunc eAggrFunc )
{
	CSphQueryItem tItem;
	tItem.m_sExpr.SetBinary ( m_pStart + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	if ( pAlias )
		tItem.m_sAlias.SetBinary ( m_pStart + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
	tItem.m_eAggrFunc = eAggrFunc;

	m_pQuery->m_dItems.Add ( tItem );
}


bool CSphQuery::ParseSelectList ( CSphString & sError )
{
	m_dItems.Reset ();
	if ( m_sSelect.IsEmpty() )
		return true; // empty is ok; will just return everything

	SelectParser_t tParser;
	tParser.m_pStart = m_sSelect.cstr();
	tParser.m_pCur = m_sSelect.cstr();
	tParser.m_pQuery = this;

	yyparse ( &tParser );

	sError = tParser.m_sParserError;
	return sError.IsEmpty ();
}

/////////////////////////////////////////////////////////////////////////////
// SCHEMA
/////////////////////////////////////////////////////////////////////////////

static CSphString sphDumpAttr ( const CSphColumnInfo & tAttr )
{
	char sTypeBuf[32];
	snprintf ( sTypeBuf, sizeof(sTypeBuf), "unknown-%d", tAttr.m_eAttrType );

	const char * sType = sTypeBuf;
	switch ( tAttr.m_eAttrType )
	{
		case SPH_ATTR_NONE:							sType = "none"; break;
		case SPH_ATTR_INTEGER:						sType = "integer"; break;
		case SPH_ATTR_TIMESTAMP:					sType = "timestamp"; break;
		case SPH_ATTR_ORDINAL:						sType = "ordinal"; break;
		case SPH_ATTR_BOOL:							sType = "bool"; break;
		case SPH_ATTR_FLOAT:						sType = "float"; break;
		case SPH_ATTR_BIGINT:						sType = "bigint"; break;
		case SPH_ATTR_INTEGER | SPH_ATTR_MULTI:		sType = "mva"; break;
	}

	CSphString sRes;
	sRes.SetSprintf ( "%s %s:%d@%d", sType, tAttr.m_sName.cstr(), tAttr.m_tLocator.m_iBitCount, tAttr.m_tLocator.m_iBitOffset );
	return sRes;
}


bool CSphSchema::CompareTo ( const CSphSchema & rhs, CSphString & sError ) const
{
	// check attr count
	if ( GetAttrsCount()!=rhs.GetAttrsCount() )
	{
		sError.SetSprintf ( "attribute count mismatch (me=%s, in=%s, myattrs=%d, inattrs=%d)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			GetAttrsCount(), rhs.GetAttrsCount() );
		return false;
	}

	// check attrs
	ARRAY_FOREACH ( i, m_dAttrs )
		if (!( rhs.m_dAttrs[i]==m_dAttrs[i] ))
	{
		sError.SetSprintf ( "attribute mismatch (me=%s, in=%s, idx=%d, myattr=%s, inattr=%s)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			i, sphDumpAttr ( m_dAttrs[i] ).cstr(), sphDumpAttr ( rhs.m_dAttrs[i] ).cstr() );
		return false;
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
	m_dFields.Reset();
	ResetAttrs ();
}


void CSphSchema::ResetAttrs ()
{
	m_dAttrs.Reset();
	m_dStaticUsed.Reset();
	m_dDynamicUsed.Reset();
	m_iStaticSize = 0;
}


void CSphSchema::AddAttr ( const CSphColumnInfo & tCol, bool bDynamic )
{
	assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
	if ( tCol.m_eAttrType==SPH_ATTR_NONE )
		return;

	m_dAttrs.Add ( tCol );
	CSphAttrLocator & tLoc = m_dAttrs.Last().m_tLocator;

	int iBits = ROWITEM_BITS;
	if ( tCol.m_tLocator.m_iBitCount>0 )		iBits = tCol.m_tLocator.m_iBitCount;
	if ( tCol.m_eAttrType==SPH_ATTR_BOOL )		iBits = 1;
	if ( tCol.m_eAttrType==SPH_ATTR_BIGINT )	iBits = 64;
	tLoc.m_iBitCount = iBits;
	tLoc.m_bDynamic = bDynamic;

	CSphVector<int> & dUsed = bDynamic ? m_dDynamicUsed : m_dStaticUsed;
	if ( iBits>=ROWITEM_BITS )
	{
		tLoc.m_iBitOffset = dUsed.GetLength()*ROWITEM_BITS;

		int iItems = (iBits+ROWITEM_BITS-1) / ROWITEM_BITS;
		for ( int i=0; i<iItems; i++ )
		{
			dUsed.Add ( ROWITEM_BITS );
			if ( !bDynamic )
				m_iStaticSize++;
		}

	} else
	{
		int iItem;
		for ( iItem=0; iItem<dUsed.GetLength(); iItem++ )
			if ( dUsed[iItem]+iBits<=ROWITEM_BITS )
				break;
		if ( iItem==dUsed.GetLength() )
		{
			dUsed.Add ( 0 );
			if ( !bDynamic )
				m_iStaticSize++;
		}

		tLoc.m_iBitOffset = iItem*ROWITEM_BITS + dUsed[iItem];
		dUsed[iItem] += iBits;
	}
}


void CSphSchema::RemoveAttr ( int iIndex )
{
	// adjust size
	CSphAttrLocator & tLoc = m_dAttrs[iIndex].m_tLocator;
	assert ( !tLoc.m_bDynamic );

	int iItem = tLoc.m_iBitOffset / ROWITEM_BITS;
	if ( tLoc.m_iBitCount>=ROWITEM_BITS )
	{
		for ( int i=0; i<tLoc.m_iBitCount/ROWITEM_BITS; i++ )
		{
			m_dStaticUsed[i+iItem] = 0;
			m_iStaticSize--;
		}
	} else
	{
		m_dStaticUsed[iItem] -= tLoc.m_iBitCount;
		assert ( m_dStaticUsed[iItem]>=0 );

		if ( m_dStaticUsed[iItem]<=0 )
			m_iStaticSize--;
	}

	// do remove
	m_dAttrs.Remove ( iIndex );
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE OUTPUT
///////////////////////////////////////////////////////////////////////////////

CSphWriter::CSphWriter ()
	: m_sName ( "" )
	, m_iPos ( -1 )
	, m_iWritten ( 0 )

	, m_iFD ( -1 )
	, m_iPoolUsed ( 0 )
	, m_pBuffer ( NULL )
	, m_pPool ( NULL )
	, m_bOwnFile ( false )
	, m_pSharedOffset ( NULL )
	, m_iBufferSize	( 262144 )

	, m_bError ( false )
	, m_pError ( NULL )
{
}


void CSphWriter::SetBufferSize ( int iBufferSize )
{
	if ( iBufferSize!=m_iBufferSize )
	{
		m_iBufferSize = Max ( iBufferSize, 262144 );
		if ( m_pBuffer )
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
	m_iWritten = 0;
	m_bError = ( m_iFD<0 );

	if ( m_bError )
		m_pError->SetSprintf ( "failed to create %s: %s" , sName.cstr(), strerror(errno) );

	return !m_bError;
}


void CSphWriter::SetFile ( int iFD, SphOffset_t * pSharedOffset )
{
	assert ( m_iFD<0 && "already open" );
	m_bOwnFile = false;

	if ( !m_pBuffer )
		m_pBuffer = new BYTE [ m_iBufferSize ];

	m_iFD = iFD;
	m_pPool = m_pBuffer;
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_iWritten = 0;
	m_pSharedOffset = pSharedOffset;
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


void CSphWriter::PutByte ( int data )
{
	if ( m_iPoolUsed==m_iBufferSize )
		Flush ();
	*m_pPool++ = BYTE ( data & 0xff );
	m_iPoolUsed++;
	m_iPos++;
}


void CSphWriter::PutBytes ( const void * pData, int iSize )
{
	const BYTE * pBuf = (const BYTE *) pData;
	while ( iSize>0 )
	{
		int iPut = Min ( iSize, m_iBufferSize );
		if ( m_iPoolUsed+iPut>m_iBufferSize )
			Flush ();
		assert ( m_iPoolUsed+iPut<=m_iBufferSize );

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
			( 0x7f & ( uValue >> (7*iBytes) ) )
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
			( 0x7f & (DWORD)( uValue >> (7*iBytes) ) )
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
				( 0x7f & (DWORD)( uValue >> (7*iBytes) ) )
				| ( iBytes ? 0x80 : 0 ) );
	}
}


void CSphWriter::Flush ()
{
	PROFILE ( write_hits );

	if ( m_pSharedOffset && *m_pSharedOffset!=m_iWritten )
		sphSeek ( m_iFD, m_iWritten, SEEK_SET );

	if ( !sphWriteThrottled ( m_iFD, m_pBuffer, m_iPoolUsed, m_sName.cstr(), *m_pError ) )
		m_bError = true;

	m_iWritten += m_iPoolUsed;
	m_iPoolUsed = 0;
	m_pPool = m_pBuffer;

	if ( m_pSharedOffset )
		*m_pSharedOffset = m_iWritten;
}


void CSphWriter::PutString ( const char * szString )
{
	int iLen = szString ? strlen ( szString ) : 0;
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void CSphWriter::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );

	if ( iPos>=m_iWritten && iPos<=( m_iWritten + m_iPoolUsed ) )
	{
		// seeking inside the buffer
		m_iPoolUsed = (int)( iPos - m_iWritten );
		m_pPool = m_pBuffer + m_iPoolUsed;
	} else
	{
		assert ( iPos<m_iWritten ); // seeking forward in a writer, we don't support it
		sphSeek ( m_iFD, iPos, SEEK_SET );

		// seeking outside the buffer; so the buffer must be discarded
		// also, current write position must be adjusted
		m_pPool = m_pBuffer;
		m_iPoolUsed = 0;
		m_iWritten = iPos;
	}
	m_iPos = iPos;
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE INPUT
///////////////////////////////////////////////////////////////////////////////

CSphReader::CSphReader ( BYTE * pBuf, int iSize )
	: m_iFD ( -1 )
	, m_iPos ( 0 )
	, m_iBuffPos ( 0 )
	, m_iBuffUsed ( 0 )
	, m_pBuff ( pBuf )
	, m_iSizeHint ( 0 )
	, m_iBufSize ( iSize )
	, m_bBufOwned ( false )
	, m_iReadUnhinted ( DEFAULT_READ_UNHINTED )
	, m_bError ( false )
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


void CSphReader::SeekTo ( SphOffset_t iPos, int iSizeHint )
{
	assert ( iPos>=0 );

#ifndef NDEBUG
	struct_stat tStat;
	fstat ( m_iFD, &tStat );
	if ( iPos > tStat.st_size )
		sphDie ( "INTERNAL ERROR: seeking past the end of file" );
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
		m_iSizeHint = iSizeHint;
	}
}


void CSphReader::SkipBytes ( int iCount )
{
	SeekTo ( m_iPos+m_iBuffPos+iCount, m_iSizeHint-m_iBuffPos-iCount );
}


#if USE_WINDOWS

// atomic seek+read for Windows
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( iBytes==0 )
		return 0;

	int64_t tmStart = 0;
	if ( g_bIOStats )
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

	if ( g_bIOStats )
	{
		g_IOStats.m_iReadTime += sphMicroTimer() - tmStart;
		g_IOStats.m_iReadOps++;
		g_IOStats.m_iReadBytes += iBytes;
	}

	return uRes;
}

#else
#if HAVE_PREAD

// atomic seek+read for non-Windows systems with pread() call
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( !g_bIOStats )
		return ::pread ( iFD, pBuf, iBytes, iOffset );

	int64_t tmStart = sphMicroTimer();
	int iRes = (int) ::pread ( iFD, pBuf, iBytes, iOffset );
	g_IOStats.m_iReadTime += sphMicroTimer() - tmStart;
	g_IOStats.m_iReadOps++;
	g_IOStats.m_iReadBytes += iBytes;
	return iRes;
}

#else

// generic fallback; prone to races between seek and read
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( sphSeek ( iFD, iOffset, SEEK_SET )==-1 )
		return -1;

	return sphReadThrottled ( iFD, pBuf, iBytes );
}

#endif // HAVE_PREAD
#endif // USE_WINDOWS


void CSphReader::UpdateCache ()
{
	PROFILE ( read_hits );
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
		m_sError.SetSprintf ( "pread error in %s: pos="INT64_FMT", len=%d, code=%d, msg=%s",
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
		m_iSizeHint = iSize; // FIXME!

		if ( iSize>0 )
		{
			UpdateCache ();
			if ( !m_iBuffUsed )
				return; // unexpected io failure
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

		m_iSizeHint = iSize - m_iBuffUsed + m_iBuffPos; // FIXME!
		UpdateCache ();
		if ( m_iBuffPos+iSize>m_iBuffUsed )
		{
			memset ( pOut, 0, iSize ); // unexpected io failure
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

DWORD CSphReader::UnzipInt ()			{ SPH_UNZIP_IMPL ( DWORD, GetByte() ); }
SphOffset_t CSphReader::UnzipOffset ()	{ SPH_UNZIP_IMPL ( SphOffset_t, GetByte() ); }


#if USE_64BIT
#define sphUnzipWordid sphUnzipOffset
#else
#define sphUnzipWordid sphUnzipInt
#endif

/////////////////////////////////////////////////////////////////////////////

const CSphReader & CSphReader::operator = ( const CSphReader & rhs )
{
	SetFile ( rhs.m_iFD, rhs.m_sFilename.cstr() );
	SeekTo ( rhs.m_iPos + rhs.m_iBuffPos, rhs.m_iSizeHint );
	return *this;
}


DWORD CSphReader::GetDword ()
{
	DWORD uRes;
	GetBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


SphOffset_t CSphReader::GetOffset ()
{
	SphOffset_t uRes;
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

//////////////////////////////////////////////////////////////////////////

CSphAutoreader::~CSphAutoreader ()
{
	Close ();
}


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


SphOffset_t CSphAutoreader::GetFilesize ()
{
	assert ( m_iFD>=0 );

	struct_stat st;
	if ( m_iFD<0 || fstat ( m_iFD, &st )<0 )
		return -1;

	return st.st_size;
}

/////////////////////////////////////////////////////////////////////////////
// QUERY RESULT
/////////////////////////////////////////////////////////////////////////////

CSphQueryResult::CSphQueryResult ()
	: m_tSchema ( "query_result" )
{
	m_iQueryTime = 0;
	m_iCpuTime = 0;
	m_iMultiplier = 1;
	m_iTotalMatches = 0;
	m_pMva = NULL;
	m_pStrings = NULL;
	m_pAttrs = NULL;
	m_iOffset = 0;
	m_iCount = 0;
	m_iSuccesses = 0;
}


CSphQueryResult::~CSphQueryResult ()
{
	SafeDeleteArray ( m_pAttrs );
}

/////////////////////////////////////////////////////////////////////////////
// CHUNK READER
/////////////////////////////////////////////////////////////////////////////

CSphBin::CSphBin ( ESphHitless eMode )
	: m_eMode ( eMode )
	, m_dBuffer ( NULL )
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
	m_tLastHit.m_uFieldMask = 0;
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
		if ( n==0 )
		{
			m_iDone = 1;
			m_iLeft = 1;
		} else
		{
			assert ( m_dBuffer );

			if ( sphReadThrottled ( m_iFile, m_dBuffer, n )!=(size_t)n )
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

		if ( sphReadThrottled ( m_iFile, m_dBuffer + m_iLeft, n )!=(size_t)n )
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


int CSphBin::ReadHit ( CSphAggregateHit * pOut, int iRowitems, CSphRowitem * pRowitems )
{
	// expected EOB
	if ( m_iDone )
	{
		pOut->m_iWordID = 0;
		return 1;
	}

	CSphAggregateHit & tHit = m_tLastHit; // shortcut
	for ( ;; )
	{
		SphWordID_t uDelta = ReadVLB();
		if ( uDelta )
		{
			switch ( m_eState )
			{
				case BIN_WORD:
					tHit.m_iWordID += uDelta;
					tHit.m_iDocID = 0;
					tHit.m_iWordPos = 0;
					tHit.m_uFieldMask = 0;
					m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					m_eState = BIN_POS;
					tHit.m_iDocID += uDelta;
					tHit.m_iWordPos = 0;
					for ( int i=0; i<iRowitems; i++, pRowitems++ )
						*pRowitems = (DWORD)ReadVLB(); // FIXME? check range?
					break;

				case BIN_POS:
					if ( m_eMode==SPH_HITLESS_ALL )
					{
						tHit.m_uFieldMask = (DWORD)ReadVLB();
						m_eState = BIN_DOC;

					} else if ( m_eMode==SPH_HITLESS_SOME )
					{
						if ( uDelta & 1 )
						{
							tHit.m_uFieldMask = (DWORD)ReadVLB();
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
				case BIN_WORD:	m_iDone = 1; pOut->m_iWordID = 0; return 1;
				default:		sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		}
	}
}


bool CSphBin::IsEOF () const
{
	return m_iDone!=0 || m_iFileLeft<=0;
}


ESphBinRead CSphBin::Precache ()
{
	if ( m_iFileLeft > m_iSize-m_iLeft )
		return BIN_PRECACHE_ERROR;

	if ( !m_iFileLeft )
		return BIN_PRECACHE_OK;

	if ( *m_pFilePos!=m_iFilePos )
	{
		sphSeek ( m_iFile, m_iFilePos, SEEK_SET );
		*m_pFilePos = m_iFilePos;
	}

	assert ( m_dBuffer );
	memmove ( m_dBuffer, m_pCurrent, m_iLeft );

	if ( sphReadThrottled ( m_iFile, m_dBuffer+m_iLeft, m_iFileLeft )!=(size_t)m_iFileLeft )
		return BIN_READ_ERROR;

	m_iLeft += m_iFileLeft;
	m_iFilePos += m_iFileLeft;
	m_iFileLeft -= m_iFileLeft;
	m_pCurrent = m_dBuffer;
	*m_pFilePos += m_iFileLeft;

	return BIN_PRECACHE_OK;
}


//////////////////////////////////////////////////////////////////////////
// INDEX SETTINGS
//////////////////////////////////////////////////////////////////////////

CSphIndexSettings::CSphIndexSettings ()
	: m_eDocinfo			( SPH_DOCINFO_NONE )
	, m_eHitFormat			( SPH_HIT_FORMAT_PLAIN )
	, m_bHtmlStrip			( false )
	, m_eHitless			( SPH_HITLESS_NONE )
{
}

//////////////////////////////////////////////////////////////////////////
// PROCESS-SHARED MUTEX
//////////////////////////////////////////////////////////////////////////

/// scoped mutex lock
class CSphScopedMutexLock : ISphNoncopyable
{
public:
	/// lock on creation
	explicit CSphScopedMutexLock ( CSphProcessSharedMutex & tMutex )
		: m_tMutexRef ( tMutex )
	{
		m_tMutexRef.Lock();
	}

	/// unlock on going out of scope
	~CSphScopedMutexLock ()
	{
		m_tMutexRef.Unlock ();
	}

protected:
	CSphProcessSharedMutex &	m_tMutexRef;
};

//////////////////////////////////////////////////////////////////////////
// GLOBAL MVA STORAGE ARENA
//////////////////////////////////////////////////////////////////////////

/// shared-memory arena allocator
/// manages small tagged dword strings, upto 4096 bytes in size
class CSphArena
{
public:
							CSphArena ();
							~CSphArena ();

	bool					Init ( int uMaxBytes );
	DWORD *					GetBasePtr () const { return m_pBasePtr; }

	int						TaggedAlloc ( int iTag, int iBytes );
	void					TaggedFreeIndex ( int iTag, int iIndex );
	void					TaggedFreeTag ( int iTag );

protected:
	static const int		MIN_BITS	= 4;
	static const int		MAX_BITS	= 12;
	static const int		NUM_SIZES	= MAX_BITS-MIN_BITS+2;	///< one for 0 (empty pages), and one for each size from min to max

	static const int		PAGE_SIZE	= 1<<MAX_BITS;
	static const int		PAGE_ALLOCS	= 1<<( MAX_BITS-MIN_BITS);
	static const int		PAGE_BITMAP	= ( PAGE_ALLOCS+8*sizeof(DWORD)-1 )/( 8*sizeof(DWORD) );

	static const int		MAX_TAGS		= 1024;
	static const int		MAX_LOGENTRIES	= 29;

	///< page descriptor
	struct PageDesc_t
	{
		int					m_iSizeBits;			///< alloc size
		int					m_iPrev;				///< prev free page of this size
		int					m_iNext;				///< next free page of this size
		int					m_iUsed;				///< usage count
		DWORD				m_uBitmap[PAGE_BITMAP];	///< usage bitmap
	};

	///< tag descriptor
	struct TagDesc_t
	{
		int					m_iTag;					///< tag value
		int					m_iAllocs;				///< active allocs
		int					m_iLogHead;				///< pointer to head allocs log entry
	};

	///< allocs log entry
	struct AllocsLogEntry_t
	{
		int					m_iUsed;
		int					m_iNext;
		int					m_dEntries[MAX_LOGENTRIES];
	};
	STATIC_SIZE_ASSERT ( AllocsLogEntry_t, 124 );

protected:
	int						RawAlloc ( int iBytes );
	void					RawFree ( int iIndex );
	void					RemoveTag ( TagDesc_t * pTag );

protected:
	CSphProcessSharedMutex	m_tMutex;

	int						m_iPages;			///< max pages count
	CSphSharedBuffer<DWORD>	m_pArena;			///< arena that stores everything (all other pointers point here)

	PageDesc_t *			m_pPages;			///< page descriptors
	int *					m_pFreelistHeads;	///< free-list heads
	int *					m_pTagCount;
	TagDesc_t *				m_pTags;

	DWORD *					m_pBasePtr;			///< base data storage pointer

#if ARENADEBUG
protected:
	int *					m_pTotalAllocs;
	int *					m_pTotalBytes;

public:
	void					CheckFreelists ();
#else
	inline void				CheckFreelists () {}
#endif // ARENADEBUG
};

//////////////////////////////////////////////////////////////////////////

CSphArena::CSphArena ()
	: m_iPages ( 0 )
{
}


CSphArena::~CSphArena ()
{
	// notify callers that arena no longer exists
	g_pMvaArena = NULL;
}


bool CSphArena::Init ( int uMaxBytes )
{
	m_iPages = ( uMaxBytes+PAGE_SIZE-1 ) / PAGE_SIZE;

	int iData = m_iPages*PAGE_SIZE; // data size, bytes
	int iMyTaglist = sizeof(int) + MAX_TAGS*sizeof(TagDesc_t); // int length, TagDesc_t[] tags; NOLINT
	int iMy = m_iPages*sizeof(PageDesc_t) + NUM_SIZES*sizeof(int) + iMyTaglist; // my internal structures size, bytes; NOLINT
#if ARENADEBUG
	iMy += 2*sizeof(int); // debugging counters; NOLINT
#endif

	assert ( iData%sizeof(DWORD)==0 );
	assert ( iMy%sizeof(DWORD)==0 );

	CSphString sError, sWarning;
	if ( !m_pArena.Alloc ( (iData+iMy)/sizeof(DWORD), sError, sWarning ) )
	{
		m_iPages = 0;
		return false;
	}

	// setup internal pointers
	DWORD * pCur = m_pArena.GetWritePtr();

	m_pPages = (PageDesc_t*) pCur;
	pCur += sizeof(PageDesc_t)*m_iPages/sizeof(DWORD);

	m_pFreelistHeads = (int*) pCur;
	pCur += NUM_SIZES; // one for each size, and one extra for zero

	m_pTagCount = (int*) pCur++;
	m_pTags = (TagDesc_t*) pCur;
	pCur += sizeof(TagDesc_t)*MAX_TAGS/sizeof(DWORD);

#if ARENADEBUG
	m_pTotalAllocs = (int*) pCur++;
	m_pTotalBytes = (int*) pCur++;
	*m_pTotalAllocs = 0;
	*m_pTotalBytes = 0;
#endif

	m_pBasePtr = m_pArena.GetWritePtr() + iMy/sizeof(DWORD);
	assert ( m_pBasePtr==pCur );

	// setup initial state
	for ( int i=0; i<m_iPages; i++ )
	{
		m_pPages[i].m_iSizeBits = 0; // fully empty
		m_pPages[i].m_iPrev = ( i>0 ) ? i-1 : -1;
		m_pPages[i].m_iNext = ( i<m_iPages-1 ) ? i+1 : -1;
	}

	m_pFreelistHeads[0] = 0;
	for ( int i=1; i<NUM_SIZES; i++ )
		m_pFreelistHeads[i] = -1;

	*m_pTagCount = 0;

	return true;
}


static inline int FindBit ( DWORD uValue )
{
	DWORD uMask = 0xffff;
	int iIdx = 0;
	int iBits = 16;

	// we negate bits to compare with 0
	// this makes MSVC emit 'test' instead of 'cmp'
	uValue ^= 0xffffffff;
	for ( int t=0; t<5; t++ )
	{
		if ( ( uValue & uMask )==0 )
		{
			iIdx += iBits;
			uValue >>= iBits;
		}
		iBits >>= 1;
		uMask >>= iBits;
	}
	return iIdx;
}


int CSphArena::RawAlloc ( int iBytes )
{
	CheckFreelists ();

	if ( iBytes<=0 || iBytes>( ( 1 << MAX_BITS ) - (int)sizeof(int) ) )
		return -1;

	int iSizeBits = sphLog2 ( iBytes+sizeof(int)-1 ); // always reserve sizeof(int) for the tag; NOLINT
	iSizeBits = Max ( iSizeBits, MIN_BITS );
	assert ( iSizeBits>=MIN_BITS && iSizeBits<=MAX_BITS );

	int iSizeSlot = iSizeBits-MIN_BITS+1;
	assert ( iSizeSlot>=1 && iSizeSlot<NUM_SIZES );

	// get semi-free page for this size
	PageDesc_t * pPage = NULL;
	if ( m_pFreelistHeads[iSizeSlot]>=0 )
	{
		// got something in the free-list
		pPage = m_pPages + m_pFreelistHeads[iSizeSlot];

	} else
	{
		// nothing in free-list, alloc next empty one
		if ( m_pFreelistHeads[0]<0 )
			return -1; // out of memory

		// update the page
		pPage = m_pPages + m_pFreelistHeads[0];
		assert ( pPage->m_iPrev==-1 );

		m_pFreelistHeads[iSizeSlot] = m_pFreelistHeads[0];
		m_pFreelistHeads[0] = pPage->m_iNext;
		if ( pPage->m_iNext>=0 )
			m_pPages[pPage->m_iNext].m_iPrev = -1;

		pPage->m_iSizeBits = iSizeBits;
		pPage->m_iUsed = 0;
		pPage->m_iNext = -1;

		CheckFreelists ();

		// setup bitmap
		int iUsedBits = ( 1<<(MAX_BITS-iSizeBits) ); // max-used-bits = page-size/alloc-size = ( 1<<page-bitsize )/( 1<<alloc-bitsize )
		assert ( iUsedBits>0 && iUsedBits<=(PAGE_BITMAP<<5) );

		for ( int i=0; i<PAGE_BITMAP; i++ )
			pPage->m_uBitmap[i] = ( ( i<<5 )>=iUsedBits ) ? 0xffffffffUL : 0;

		if ( iUsedBits<32 )
			pPage->m_uBitmap[0] = ( 0xffffffffUL<<iUsedBits );
	}

	// get free alloc slot and use it
	assert ( pPage );
	assert ( pPage->m_iSizeBits==iSizeBits );

	for ( int i=0; i<PAGE_BITMAP; i++ ) // FIXME! optimize, can scan less
	{
		if ( pPage->m_uBitmap[i]==0xffffffffUL )
			continue;

		int iFree = FindBit ( pPage->m_uBitmap[i] );
		pPage->m_uBitmap[i] |= ( 1<<iFree );

		pPage->m_iUsed++;
		if ( pPage->m_iUsed==( PAGE_SIZE >> pPage->m_iSizeBits ) )
		{
			// this page is full now, unchain from the free-list
			assert ( m_pFreelistHeads[iSizeSlot]==pPage-m_pPages );
			m_pFreelistHeads[iSizeSlot] = pPage->m_iNext;
			if ( pPage->m_iNext>=0 )
			{
				assert ( m_pPages[pPage->m_iNext].m_iPrev==pPage-m_pPages );
				m_pPages[pPage->m_iNext].m_iPrev = -1;
			}
			pPage->m_iNext = -1;
		}

#if ARENADEBUG
		(*m_pTotalAllocs)++;
		(*m_pTotalBytes) += ( 1<<iSizeBits );
#endif

		CheckFreelists ();

		int iOffset = ( pPage-m_pPages )*PAGE_SIZE + ( i*32+iFree )*( 1<<iSizeBits ); // raw internal byte offset (FIXME! optimize with shifts?)
		int iIndex = 1 + ( iOffset/sizeof(DWORD) ); // dword index with tag fixup

		m_pBasePtr[iIndex-1] = DWORD(-1); // untagged by default
		return iIndex;
	}

	assert ( 0 && "internal error, no free slots in free page" );
	return -1;
}


void CSphArena::RawFree ( int iIndex )
{
	CheckFreelists ();

	int iOffset = (iIndex-1)*sizeof(DWORD); // remove tag fixup, and go to raw internal byte offset
	int iPage = iOffset / PAGE_SIZE;

	if ( iPage<0 || iPage>m_iPages )
	{
		assert ( 0 && "internal error, freed index out of arena" );
		return;
	}

	PageDesc_t * pPage = m_pPages + iPage;
	int iBit = ( iOffset % PAGE_SIZE ) >> pPage->m_iSizeBits;
	assert ( ( iOffset % PAGE_SIZE )==( iBit << pPage->m_iSizeBits ) && "internal error, freed offset is unaligned" );

	if (!( pPage->m_uBitmap[iBit>>5] & ( 1UL<<(iBit & 31) ) ))
	{
		assert ( 0 && "internal error, freed index already freed" );
		return;
	}

	pPage->m_uBitmap[iBit>>5] &= ~( 1UL << ( iBit & 31 ) );
	pPage->m_iUsed--;

#if ARENADEBUG
	(*m_pTotalAllocs)--;
	(*m_pTotalBytes) -= ( 1<<pPage->m_iSizeBits );
#endif

	CheckFreelists ();

	int iSizeSlot = pPage->m_iSizeBits-MIN_BITS+1;

	if ( pPage->m_iUsed==( PAGE_SIZE >> pPage->m_iSizeBits )-1 )
	{
		// this page was full, but it's semi-free now
		// chain to free-list
		assert ( pPage->m_iPrev==-1 ); // full pages must not be in any list
		assert ( pPage->m_iNext==-1 );

		pPage->m_iNext = m_pFreelistHeads[iSizeSlot];
		if ( pPage->m_iNext>=0 )
		{
			assert ( m_pPages[pPage->m_iNext].m_iPrev==-1 );
			assert ( m_pPages[pPage->m_iNext].m_iSizeBits==pPage->m_iSizeBits );
			m_pPages[pPage->m_iNext].m_iPrev = iPage;
		}
		m_pFreelistHeads[iSizeSlot] = iPage;
	}

	if ( pPage->m_iUsed==0 )
	{
		// this page is empty now
		// unchain from free-list
		if ( pPage->m_iPrev>=0 )
		{
			// non-head page
			assert ( m_pPages[pPage->m_iPrev].m_iNext==iPage );
			m_pPages[pPage->m_iPrev].m_iNext = pPage->m_iNext;

			if ( pPage->m_iNext>=0 )
			{
				assert ( m_pPages[pPage->m_iNext].m_iPrev==iPage );
				m_pPages[pPage->m_iNext].m_iPrev = pPage->m_iPrev;
			}

		} else
		{
			// head page
			assert ( m_pFreelistHeads[iSizeSlot]==iPage );
			assert ( pPage->m_iPrev==-1 );

			if ( pPage->m_iNext>=0 )
			{
				assert ( m_pPages[pPage->m_iNext].m_iPrev==iPage );
				m_pPages[pPage->m_iNext].m_iPrev = -1;
			}
			m_pFreelistHeads[iSizeSlot] = pPage->m_iNext;
		}

		pPage->m_iSizeBits = 0;
		pPage->m_iPrev = -1;
		pPage->m_iNext = m_pFreelistHeads[0];
		if ( pPage->m_iNext>=0 )
		{
			assert ( m_pPages[pPage->m_iNext].m_iPrev==-1 );
			assert ( m_pPages[pPage->m_iNext].m_iSizeBits==0 );
			m_pPages[pPage->m_iNext].m_iPrev = iPage;
		}
		m_pFreelistHeads[0] = iPage;
	}

	CheckFreelists ();
}


int CSphArena::TaggedAlloc ( int iTag, int iBytes )
{
	if ( !m_iPages )
		return -1; // uninitialized

	assert ( iTag>=0 );
	CSphScopedMutexLock tLock ( m_tMutex );

	// find that tag first
	TagDesc_t * pTag = sphBinarySearch ( m_pTags, m_pTags+(*m_pTagCount)-1, bind ( &TagDesc_t::m_iTag ), iTag );
	if ( !pTag )
	{
		if ( *m_pTagCount==MAX_TAGS )
			return -1; // out of tags

		int iLogHead = RawAlloc ( sizeof(AllocsLogEntry_t) );
		if ( iLogHead<0 )
			return -1; // out of memory

		AllocsLogEntry_t * pLog = (AllocsLogEntry_t*) ( m_pBasePtr + iLogHead );
		pLog->m_iUsed = 0;
		pLog->m_iNext = -1;

		// add new tag
		pTag = m_pTags + (*m_pTagCount)++;
		pTag->m_iTag = iTag;
		pTag->m_iAllocs = 0;
		pTag->m_iLogHead = iLogHead;

		// re-sort
		// OPTIMIZE! full-blown sort is overkill here
		sphSort ( m_pTags, *m_pTagCount, sphMemberLess ( &TagDesc_t::m_iTag ) );

		// we must be able to find it now
		pTag = sphBinarySearch ( m_pTags, m_pTags+(*m_pTagCount)-1, bind ( &TagDesc_t::m_iTag ), iTag );
		assert ( pTag && "internal error, fresh tag not found in TaggedAlloc()" );

		if ( !pTag )
			return -1; // internal error
	}

	// grow the log if needed
	AllocsLogEntry_t * pLog = (AllocsLogEntry_t*) ( m_pBasePtr + pTag->m_iLogHead );
	if ( pLog->m_iUsed==MAX_LOGENTRIES )
	{
		int iNewEntry = RawAlloc ( sizeof(AllocsLogEntry_t) );
		if ( iNewEntry<0 )
			return -1; // out of memory

		AllocsLogEntry_t * pNew = (AllocsLogEntry_t*) ( m_pBasePtr + iNewEntry );
		pNew->m_iUsed = 0;
		pNew->m_iNext = pTag->m_iLogHead;
		pTag->m_iLogHead = iNewEntry;
		pLog = pNew;
	}

	// do the alloc itself
	int iIndex = RawAlloc ( iBytes );
	if ( iIndex<0 )
		return -1; // out of memory

	// tag it
	m_pBasePtr[iIndex-1] = iTag;

	// log it
	assert ( pLog->m_iUsed<MAX_LOGENTRIES );
	pLog->m_dEntries [ pLog->m_iUsed++ ] = iIndex;
	pTag->m_iAllocs++;

	// and we're done
	return iIndex;
}


void CSphArena::TaggedFreeIndex ( int iTag, int iIndex )
{
	if ( !m_iPages )
		return; // uninitialized

	assert ( iTag>=0 );
	CSphScopedMutexLock tLock ( m_tMutex );

	// find that tag
	TagDesc_t * pTag = sphBinarySearch ( m_pTags, m_pTags+(*m_pTagCount)-1, bind ( &TagDesc_t::m_iTag ), iTag );
	assert ( pTag && "internal error, unknown tag in TaggedFreeIndex()" );
	assert ( m_pBasePtr[iIndex-1]==DWORD(iTag) && "internal error, tag mismatch in TaggedFreeIndex()" );

	// defence against internal errors
	if ( !pTag )
		return;

	// untag it
	m_pBasePtr[iIndex-1] = DWORD(-1);

	// free it
	RawFree ( iIndex );

	// update the tag decsriptor
	pTag->m_iAllocs--;
	assert ( pTag->m_iAllocs>=0 );

	// remove the descriptor if its empty now
	if ( pTag->m_iAllocs==0 )
		RemoveTag ( pTag );
}


void CSphArena::TaggedFreeTag ( int iTag )
{
	if ( !m_iPages )
		return; // uninitialized

	assert ( iTag>=0 );
	CSphScopedMutexLock tLock ( m_tMutex );

	// find that tag
	TagDesc_t * pTag = sphBinarySearch ( m_pTags, m_pTags+(*m_pTagCount)-1, bind ( &TagDesc_t::m_iTag ), iTag );
	if ( !pTag )
		return;

	// walk the log and free it
	int iLog = pTag->m_iLogHead;
	while ( iLog>=0 )
	{
		AllocsLogEntry_t * pLog = (AllocsLogEntry_t*) ( m_pBasePtr + iLog );
		iLog = pLog->m_iNext;

		// free each alloc if tag still matches
		for ( int i=0; i<pLog->m_iUsed; i++ )
		{
			int iIndex = pLog->m_dEntries[i];
			if ( m_pBasePtr[iIndex-1]==DWORD(iTag) )
			{
				m_pBasePtr[iIndex-1] = DWORD(-1); // avoid double free
				RawFree ( iIndex );
				pTag->m_iAllocs--;
			}
		}
	}

	// check for mismatches
	assert ( pTag->m_iAllocs==0 );

	// remove the descriptor
	RemoveTag ( pTag );
}


void CSphArena::RemoveTag ( TagDesc_t * pTag )
{
	assert ( pTag );
	assert ( pTag->m_iAllocs==0 );

	// dealloc log chain
	int iLog = pTag->m_iLogHead;
	while ( iLog>=0 )
	{
		AllocsLogEntry_t * pLog = (AllocsLogEntry_t*) ( m_pBasePtr + iLog );
		int iNext = pLog->m_iNext;

		RawFree ( iLog );
		iLog = iNext;
	}

	// remove tag from the list
	int iTail = m_pTags + (*m_pTagCount) - pTag - 1;
	memmove ( pTag, pTag+1, iTail*sizeof(TagDesc_t) );
	(*m_pTagCount)--;
}


#if ARENADEBUG
void CSphArena::CheckFreelists ()
{
	assert ( m_pFreelistHeads[0]==-1 || m_pPages[m_pFreelistHeads[0]].m_iSizeBits==0 );
	for ( int iSizeSlot=1; iSizeSlot<NUM_SIZES; iSizeSlot++ )
		assert ( m_pFreelistHeads[iSizeSlot]==-1 || m_pPages[m_pFreelistHeads[iSizeSlot]].m_iSizeBits-MIN_BITS+1==iSizeSlot );
}
#endif // ARENADEBUG

//////////////////////////////////////////////////////////////////////////

static CSphArena g_MvaArena; // global mega-arena

DWORD * sphArenaInit ( int iMaxBytes )
{
	if ( g_pMvaArena )
		return g_pMvaArena; // already initialized

	if ( !g_MvaArena.Init ( iMaxBytes ) )
		return NULL; // tried but failed

	g_pMvaArena = g_MvaArena.GetBasePtr ();
	return g_pMvaArena; // all good
}

/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphIndex::CSphIndex ( const char * sName )
	: m_uAttrsStatus ( 0 )
	, m_bEnableStar ( false )
	, m_bExpandKeywords ( false )
	, m_pProgress ( NULL )
	, m_tSchema ( sName )
	, m_sLastError ( "(no error message)" )
	, m_bInplaceSettings ( false )
	, m_iHitGap ( 0 )
	, m_iDocinfoGap ( 0 )
	, m_fRelocFactor ( 0.0f )
	, m_fWriteFactor ( 0.0f )
	, m_bKeepFilesOpen ( false )
	, m_bPreloadWordlist ( true )
	, m_bStripperInited ( true )
	, m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_iMaxCachedDocs ( 0 )
	, m_iMaxCachedHits ( 0 )
{
}


CSphIndex::~CSphIndex ()
{
	SafeDelete ( m_pTokenizer );
	SafeDelete ( m_pDict );
}


void CSphIndex::SetInplaceSettings ( int iHitGap, int iDocinfoGap, float fRelocFactor, float fWriteFactor )
{
	m_iHitGap = iHitGap;
	m_iDocinfoGap = iDocinfoGap;
	m_fRelocFactor = fRelocFactor;
	m_fWriteFactor = fWriteFactor;
	m_bInplaceSettings = true;
}


void CSphIndex::SetTokenizer ( ISphTokenizer * pTokenizer )
{
	if ( m_pTokenizer!=pTokenizer )
		SafeDelete ( m_pTokenizer );
	m_pTokenizer = pTokenizer;
}


ISphTokenizer *	CSphIndex::LeakTokenizer ()
{
	ISphTokenizer * pTokenizer = m_pTokenizer;
	m_pTokenizer = NULL;
	return pTokenizer;
}


void CSphIndex::SetDictionary ( CSphDict * pDict )
{
	if ( m_pDict!=pDict )
		SafeDelete ( m_pDict );

	m_pDict = pDict;
}


CSphDict * CSphIndex::LeakDictionary ()
{
	CSphDict * pDict = m_pDict;
	m_pDict = NULL;
	return pDict;
}


void CSphIndex::Setup ( const CSphIndexSettings & tSettings )
{
	m_bStripperInited = true;
	m_tSettings = tSettings;
}

void CSphIndex::SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits )
{
	m_iMaxCachedDocs = iMaxCachedDocs;
	m_iMaxCachedHits = iMaxCachedHits;
}

/////////////////////////////////////////////////////////////////////////////

CSphIndex * sphCreateIndexPhrase ( const char * sFilename )
{
	return new CSphIndex_VLN ( sFilename );
}


CSphIndex_VLN::CSphIndex_VLN ( const char * sFilename )
	: CSphIndex ( sFilename )
	, m_iLockFD ( -1 )
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
	m_iWordlistSize = 0;

	m_uDocinfo = 0;
	m_uDocinfoIndex = 0;
	m_pDocinfoIndex = NULL;

	m_bPreallocated = false;
	m_uVersion = INDEX_FORMAT_VERSION;

	m_pWordlistChunkBuf = NULL;

	m_iKillListSize = 0;
	m_uMinMaxIndex = 0;

	m_iIndexTag = -1;
	m_iMergeInfinum = 0;
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	SafeDeleteArray ( m_pWriteBuffer );

#if USE_WINDOWS
	if ( m_iIndexTag>=0 && g_pMvaArena )
#else
	if ( m_iIndexTag>=0 && g_bHeadProcess && g_pMvaArena )
#endif
		g_MvaArena.TaggedFreeTag ( m_iIndexTag );

#if !USE_WINDOWS
	if ( g_bHeadProcess )
#endif
	Unlock();
}


/////////////////////////////////////////////////////////////////////////////

int CSphIndex_VLN::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError )
{
	// check if we can
	if ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		sError.SetSprintf ( "docinfo=extern required for updates" );
		return -1;
	}

	// check if we have to
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRowOffset.GetLength() );
	if ( !m_uDocinfo || !tUpd.m_dDocids.GetLength() )
		return 0;

	// remap update schema to index schema
	CSphVector<CSphAttrLocator> dLocators;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		int iIndex = m_tSchema.GetAttrIndex ( tUpd.m_dAttrs[i].m_sName.cstr() );
		if ( iIndex<0 )
		{
			sError.SetSprintf ( "attribute '%s' not found", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}

		// forbid updates on non-int columns
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(iIndex);
		if (!( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP
			|| tCol.m_eAttrType==( SPH_ATTR_INTEGER | SPH_ATTR_MULTI ) ))
		{
			sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, timestamp, or MVA)", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}

		// forbid updates on MVA columns if there's no arena
		if ( ( tCol.m_eAttrType & SPH_ATTR_MULTI ) && !g_pMvaArena )
		{
			sError.SetSprintf ( "MVA attribute '%s' can not be updated (MVA arena not initialized)" );
			return -1;
		}

		dLocators.Add ( tCol.m_tLocator );
	}
	assert ( dLocators.GetLength()==tUpd.m_dAttrs.GetLength() );

	// FIXME! FIXME! FIXME! overwriting just-freed blocks might hurt concurrent searchers;
	// should implement a simplistic MVCC-style delayed-free to avoid that

	// do the update
	const int iFirst = ( iIndex<0 ) ? 0 : iIndex;
	const int iLast = ( iIndex<0 ) ? tUpd.m_dDocids.GetLength() : iIndex+1;

	// row update must leave it in cosistent state; so let's preallocate all the needed MVA
	// storage upfront to avoid suddenly having to rollback if allocation fails later
	int iNumMVA = 0;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
		if ( tUpd.m_dAttrs[i].m_eAttrType & SPH_ATTR_MULTI )
			iNumMVA++;

	// OPTIMIZE! execute the code below conditionally
	CSphVector<DWORD*> dRowPtrs;
	CSphVector<int> dMvaPtrs;

	dRowPtrs.Resize ( tUpd.m_dDocids.GetLength() );
	dMvaPtrs.Resize ( tUpd.m_dDocids.GetLength()*iNumMVA );
	dMvaPtrs.Fill ( -1 );

	// preallocate
	bool bFailed = false;
	for ( int iUpd=iFirst; iUpd<iLast && !bFailed; iUpd++ )
	{
		dRowPtrs[iUpd] = const_cast < DWORD * > ( FindDocinfo ( tUpd.m_dDocids[iUpd] ) );
		if ( !dRowPtrs[iUpd] )
			continue; // no such id

		int iPoolPos = tUpd.m_dRowOffset[iUpd];
		int iMvaPtr = iUpd*iNumMVA;
		ARRAY_FOREACH_COND ( iCol, tUpd.m_dAttrs, !bFailed )
		{
			if (!( tUpd.m_dAttrs[iCol].m_eAttrType & SPH_ATTR_MULTI )) // FIXME! optimize using a prebuilt dword mask?
			{
				iPoolPos++;
				continue;
			}

			// get the requested new count
			int iNewCount = (int)tUpd.m_dPool[iPoolPos++];
			iPoolPos += iNewCount;

			// try to alloc
			int iAlloc = -1;
			if ( iNewCount )
			{
				iAlloc = g_MvaArena.TaggedAlloc ( m_iIndexTag, (1+iNewCount)*sizeof(DWORD) );
				if ( iAlloc<0 )
					bFailed = true;
			}

			// whatever the outcome, move the pointer
			dMvaPtrs[iMvaPtr++] = iAlloc;
		}
	}

	// if there were any allocation failures, rollback everything
	if ( bFailed )
	{
		ARRAY_FOREACH ( i, dMvaPtrs )
			if ( dMvaPtrs[i]>=0 )
				g_MvaArena.TaggedFreeIndex ( m_iIndexTag, dMvaPtrs[i] );

		sError.SetSprintf ( "out of pool memory on MVA update" );
		return -1;
	}

	// preallocation went OK; do the actual update
	int iRowStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	int iUpdated = 0;
	DWORD uUpdateMask = 0;

	for ( int iUpd=iFirst; iUpd<iLast; iUpd++ )
	{
		DWORD * pEntry = dRowPtrs[iUpd];
		if ( !pEntry )
			continue; // no such id

		int iBlock = ( pEntry-m_pDocinfo.GetWritePtr() ) / ( iRowStride*DOCINFO_INDEX_FREQ );
		DWORD * pBlockRanges = const_cast < DWORD * > ( &m_pDocinfoIndex[2*iBlock*iRowStride] );
		DWORD * pIndexRanges = const_cast < DWORD * > ( &m_pDocinfoIndex[2*m_uDocinfoIndex*iRowStride] );
		assert ( iBlock>=0 && iBlock<(int)m_uDocinfoIndex );

		assert ( DOCINFO2ID(pEntry)==tUpd.m_dDocids[iUpd] );
		pEntry = DOCINFO2ATTRS(pEntry);

		int iPos = tUpd.m_dRowOffset[iUpd];
		int iMvaPtr = iUpd*iNumMVA;
		ARRAY_FOREACH ( iCol, tUpd.m_dAttrs )
		{
			if (!( tUpd.m_dAttrs[iCol].m_eAttrType & SPH_ATTR_MULTI )) // FIXME! optimize using a prebuilt dword mask?
			{
				// plain update
				SphAttr_t uValue = tUpd.m_dPool[iPos];
				sphSetRowAttr ( pEntry, dLocators[iCol], uValue );

				// update block and index ranges
				for ( int i=0; i<2; i++ )
				{
					DWORD * pBlock = i ? pBlockRanges : pIndexRanges;
					SphAttr_t uMin = sphGetRowAttr ( DOCINFO2ATTRS ( pBlock ), dLocators[iCol] );
					SphAttr_t uMax = sphGetRowAttr ( DOCINFO2ATTRS ( pBlock+iRowStride ) , dLocators[iCol] );
					if ( uValue<uMin || uValue>uMax )
					{
						sphSetRowAttr ( DOCINFO2ATTRS ( pBlock ), dLocators[iCol], Min ( uMin, uValue ) );
						sphSetRowAttr ( DOCINFO2ATTRS ( pBlock+iRowStride ), dLocators[iCol], Max ( uMax, uValue ) );
					}
				}

				iPos++;
				uUpdateMask |= ATTRS_UPDATED;
				continue;
			}

			// MVA update
			DWORD uOldIndex = MVA_DOWNSIZE ( sphGetRowAttr ( pEntry, dLocators[iCol] ) );

			// get new count, store new data if needed
			DWORD uNew = tUpd.m_dPool[iPos++], uNewMin = UINT_MAX, uNewMax = 0;
			int iNewIndex = dMvaPtrs[iMvaPtr++];

			if ( uNew )
			{
				assert ( iNewIndex>=0 );
				DWORD * uPtr = g_pMvaArena + iNewIndex;

				// copy values list
				*uPtr++ = uNew;
				DWORD * uFirst = uPtr, uCount = uNew;

				while ( uNew-- )
				{
					register DWORD uValue = tUpd.m_dPool[iPos++];
					*uPtr++ = uValue;
					uNewMin = Min ( uNewMin, uValue );
					uNewMax = Max ( uNewMax, uValue );
				}

				// sort values list
				if ( uCount>1 )
					sphSort ( uFirst, uCount );

				// setup new value (flagged index) to store within row
				uNew = DWORD(iNewIndex) | MVA_ARENA_FLAG;
			}

			// store new value
			sphSetRowAttr ( pEntry, dLocators[iCol], uNew );

			// update block and index ranges
			if ( uNew )
				for ( int i=0; i<2; i++ )
			{
				DWORD * pBlock = i ? pBlockRanges : pIndexRanges;
				SphAttr_t uMin = sphGetRowAttr ( DOCINFO2ATTRS ( pBlock ), dLocators[iCol] );
				SphAttr_t uMax = sphGetRowAttr ( DOCINFO2ATTRS ( pBlock+iRowStride ), dLocators[iCol] );
				if ( uNewMin<uMin || uNewMax>uMax )
				{
					sphSetRowAttr ( DOCINFO2ATTRS ( pBlock ), dLocators[iCol], Min ( uMin, uNewMin ) );
					sphSetRowAttr ( DOCINFO2ATTRS ( pBlock+iRowStride ), dLocators[iCol], Max ( uMax, uNewMax ) );
				}
			}

			// free old storage if needed
			if ( uOldIndex & MVA_ARENA_FLAG )
				g_MvaArena.TaggedFreeIndex ( m_iIndexTag, uOldIndex & MVA_OFFSET_MASK );

			uUpdateMask |= ATTRS_MVA_UPDATED;
		}

		iUpdated++;
	}

	m_uAttrsStatus |= uUpdateMask; // FIXME! add lock/atomic?
	return iUpdated;
}

class AttrIndexBuilder_c : ISphNoncopyable
{
private:
	CSphVector<CSphAttrLocator>	m_dIntAttrs;
	CSphVector<CSphAttrLocator>	m_dFloatAttrs;
	CSphVector<CSphAttrLocator>	m_dMvaAttrs;
	CSphVector<SphAttr_t>		m_dIntMin;
	CSphVector<SphAttr_t>		m_dIntMax;
	CSphVector<SphAttr_t>		m_dIntIndexMin;
	CSphVector<SphAttr_t>		m_dIntIndexMax;
	CSphVector<float>			m_dFloatMin;
	CSphVector<float>			m_dFloatMax;
	CSphVector<float>			m_dFloatIndexMin;
	CSphVector<float>			m_dFloatIndexMax;
	CSphVector<DWORD>			m_dMvaMin;
	CSphVector<DWORD>			m_dMvaMax;
	CSphVector<DWORD>			m_dMvaIndexMin;
	CSphVector<DWORD>			m_dMvaIndexMax;
	DWORD						m_uStride;		// size of attribute's chunk (in DWORDs)
	DWORD						m_uElements;	// counts total number of collected min/max pairs
	int							m_iLoop;		// loop inside one set
	DWORD *						m_pOutBuffer;	// storage for collected min/max
	DWORD *						m_pOutMax;		// storage max for bound checking
	SphDocID_t					m_uStart;		// first and last docids of current chunk
	SphDocID_t					m_uLast;
	SphDocID_t					m_uIndexStart;	// first and last docids of whole index
	SphDocID_t					m_uIndexLast;

private:
	void ResetLocal()
	{
		ARRAY_FOREACH ( i, m_dIntMin )
		{
			m_dIntMin[i] = UINT_MAX;
			m_dIntMax[i] = 0;
		}
		ARRAY_FOREACH ( i, m_dFloatMin )
		{
			m_dFloatMin[i] = FLT_MAX;
			m_dFloatMax[i] = -FLT_MAX;
		}
		ARRAY_FOREACH ( i, m_dMvaMin )
		{
			m_dMvaMin[i] = UINT_MAX;
			m_dMvaMax[i] = 0;
		}
		m_uStart = m_uLast = 0;
		m_iLoop = 0;
	}

	void FlushComputed ( bool bUseAttrs, bool bUseMvas )
	{
		assert ( m_pOutBuffer );
		DWORD * pMinEntry = m_pOutBuffer + 2 * m_uElements * m_uStride;
		DWORD * pMinAttrs = DOCINFO2ATTRS ( pMinEntry );
		DWORD * pMaxEntry = pMinEntry + m_uStride;
		DWORD * pMaxAttrs = pMinAttrs + m_uStride;

		assert ( pMaxEntry+m_uStride-1 < m_pOutMax );
		assert ( pMaxAttrs+m_uStride-1 < m_pOutMax );

		m_uIndexLast = m_uLast;

		DOCINFOSETID ( pMinEntry, m_uStart );
		DOCINFOSETID ( pMaxEntry, m_uLast );

		if ( bUseAttrs )
		{
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
		}
		if ( bUseMvas )
			ARRAY_FOREACH ( i, m_dMvaAttrs )
			{
				m_dMvaIndexMin[i] = Min ( m_dMvaIndexMin[i], m_dMvaMin[i] );
				m_dMvaIndexMax[i] = Max ( m_dMvaIndexMax[i], m_dMvaMax[i] );
				sphSetRowAttr ( pMinAttrs, m_dMvaAttrs[i], m_dMvaMin[i] );
				sphSetRowAttr ( pMaxAttrs, m_dMvaAttrs[i], m_dMvaMax[i] );
			}
		m_uElements++;
		ResetLocal();
	}

	void UpdateMinMaxDocids ( SphDocID_t uDocID )
	{
		if ( !m_uStart )
			m_uStart = uDocID;
		if ( !m_uIndexStart )
			m_uIndexStart = uDocID;
		m_uLast = uDocID;
	}

public:
	explicit AttrIndexBuilder_c ( const CSphSchema & tSchema )
		: m_uStride ( DOCINFO_IDSIZE + tSchema.GetRowSize() )
		, m_uElements ( 0 )
		, m_iLoop ( 0 )
		, m_pOutBuffer ( NULL )
		, m_pOutMax ( NULL )
		, m_uStart ( 0 )
		, m_uLast ( 0 )
		, m_uIndexStart ( 0 )
		, m_uIndexLast ( 0 )
	{
		for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr(i);

			if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
			{
				m_dMvaAttrs.Add ( tCol.m_tLocator );
				continue;
			}

			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_BIGINT:
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
		m_dIntIndexMin.Resize ( m_dIntAttrs.GetLength() );
		m_dIntIndexMax.Resize ( m_dIntAttrs.GetLength() );
		m_dFloatMin.Resize ( m_dFloatAttrs.GetLength() );
		m_dFloatMax.Resize ( m_dFloatAttrs.GetLength() );
		m_dFloatIndexMin.Resize ( m_dFloatAttrs.GetLength() );
		m_dFloatIndexMax.Resize ( m_dFloatAttrs.GetLength() );
		m_dMvaMin.Resize ( m_dMvaAttrs.GetLength() );
		m_dMvaMax.Resize ( m_dMvaAttrs.GetLength() );
		m_dMvaIndexMin.Resize ( m_dMvaAttrs.GetLength() );
		m_dMvaIndexMax.Resize ( m_dMvaAttrs.GetLength() );
	}

	void Prepare ( DWORD * pOutBuffer, DWORD * pOutMax )
	{
		m_pOutBuffer = pOutBuffer;
		m_pOutMax = pOutMax;
		m_uElements = 0;
		m_uIndexStart = m_uIndexLast = 0;
		ARRAY_FOREACH ( i, m_dIntIndexMin )
		{
			m_dIntIndexMin[i] = UINT_MAX;
			m_dIntIndexMax[i] = 0;
		}
		ARRAY_FOREACH ( i, m_dFloatIndexMin )
		{
			m_dFloatIndexMin[i] = FLT_MAX;
			m_dFloatIndexMax[i] = -FLT_MAX;
		}
		ARRAY_FOREACH ( i, m_dMvaIndexMin )
		{
			m_dMvaIndexMin[i] = UINT_MAX;
			m_dMvaIndexMax[i] = 0;
		}
		ResetLocal();
	}

	void CollectWithoutMvas ( const DWORD * pCur, bool bUseMvas )
	{
		// check if it is time to flush already collected values
		if ( m_iLoop>=CSphIndex_VLN::DOCINFO_INDEX_FREQ )
			FlushComputed ( true, bUseMvas );
		const DWORD * pRow = DOCINFO2ATTRS(pCur);
		UpdateMinMaxDocids ( DOCINFO2ID(pCur) );
		m_iLoop++;

		// ints
		ARRAY_FOREACH ( i, m_dIntAttrs )
		{
			SphAttr_t uVal = sphGetRowAttr ( pRow, m_dIntAttrs[i] );
			m_dIntMin[i] = Min ( m_dIntMin[i], uVal );
			m_dIntMax[i] = Max ( m_dIntMax[i], uVal );
		}

		// floats
		ARRAY_FOREACH ( i, m_dFloatAttrs )
		{
			float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, m_dFloatAttrs[i] ) );
			m_dFloatMin[i] = Min ( m_dFloatMin[i], fVal );
			m_dFloatMax[i] = Max ( m_dFloatMax[i], fVal );
		}
	}

	bool Collect ( const DWORD * pCur, const CSphSharedBuffer<DWORD> & pMvas, CSphString & sError )
	{
		CollectWithoutMvas ( pCur, true );

		const DWORD * pRow = DOCINFO2ATTRS(pCur);
		SphDocID_t uDocID = DOCINFO2ID(pCur);

		// MVAs
		ARRAY_FOREACH ( i, m_dMvaAttrs )
		{
			SphAttr_t uOff = sphGetRowAttr ( pRow, m_dMvaAttrs[i] );
			if ( !uOff )
				continue;

			// sanity checks
			if ( uOff>=pMvas.GetNumEntries() )
			{
				sError.SetSprintf ( "broken index: mva offset out of bounds, id=" DOCID_FMT, uDocID );
				return false;
			}

			const DWORD * pMva = &pMvas [ MVA_DOWNSIZE ( uOff ) ]; // don't care about updates at this point

			if ( i==0 && DOCINFO2ID ( pMva-DOCINFO_IDSIZE )!=uDocID )
			{
				sError.SetSprintf ( "broken index: mva docid verification failed, id=" DOCID_FMT, uDocID );
				return false;
			}
			if ( uOff+pMva[0]>=pMvas.GetNumEntries() )
			{
				sError.SetSprintf ( "broken index: mva list out of bounds, id=" DOCID_FMT, uDocID );
				return false;
			}

			// walk and calc
			for ( DWORD uCount = *pMva++; uCount>0; uCount--, pMva++ )
			{
				m_dMvaMin[i] = Min ( m_dMvaMin[i], *pMva );
				m_dMvaMax[i] = Max ( m_dMvaMax[i], *pMva );
			}
		}
		return true;
	}

	void Collect ( const DWORD * pCur, const CSphDocMVA & dMvas )
	{
		CollectWithoutMvas ( pCur, true );
		ARRAY_FOREACH ( i, m_dMvaAttrs )
			ARRAY_FOREACH ( j, dMvas.m_dMVA[i] )
			{
				m_dMvaMin[i] = Min ( m_dMvaMin[i], dMvas.m_dMVA[i][j] );
				m_dMvaMax[i] = Max ( m_dMvaMax[i], dMvas.m_dMVA[i][j] );
			}
	}

	void CollectMVA ( SphDocID_t uDocID, const CSphVector< CSphVector<DWORD> > & dCurInfo )
	{
		// check if it is time to flush already collected values
		if ( m_iLoop>=CSphIndex_VLN::DOCINFO_INDEX_FREQ )
			FlushComputed ( false, true );
		UpdateMinMaxDocids(uDocID);
		m_iLoop++;
		ARRAY_FOREACH ( i, dCurInfo )
			ARRAY_FOREACH ( j, dCurInfo[i] )
		{
			m_dMvaMin[i] = Min ( m_dMvaMin[i], dCurInfo[i][j] );
			m_dMvaMax[i] = Max ( m_dMvaMax[i], dCurInfo[i][j] );
		}
	}

	void FinishCollect ( bool bMvaOnly = false )
	{
		assert ( m_pOutBuffer );
		if ( m_iLoop )
			FlushComputed ( !bMvaOnly, true );
		DWORD * pMinEntry = m_pOutBuffer + 2 * m_uElements * m_uStride;
		DWORD * pMaxEntry = pMinEntry + m_uStride;
		CSphRowitem * pMinAttrs = DOCINFO2ATTRS ( pMinEntry );
		CSphRowitem * pMaxAttrs = DOCINFO2ATTRS ( pMaxEntry );

		assert ( pMaxEntry < m_pOutMax );
		assert ( pMaxAttrs < m_pOutMax );

		DOCINFOSETID ( pMinEntry, m_uIndexStart );
		DOCINFOSETID ( pMaxEntry, m_uIndexLast );

		ARRAY_FOREACH ( i, m_dMvaAttrs )
		{
			sphSetRowAttr ( pMinAttrs, m_dMvaAttrs[i], m_dMvaIndexMin[i] );
			sphSetRowAttr ( pMaxAttrs, m_dMvaAttrs[i], m_dMvaIndexMax[i] );
		}

		if ( !bMvaOnly )
		{
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
			m_uElements++;
		} else
			m_uElements = 0; // rewind back for collecting the rest of attributes.
	}
	// actually used part of output buffer - used only in merge() because
	// we just reserve the sum of both index, but might use not all of reserved.
	inline DWORD GetActualSize() const { return 2 * m_uElements * m_uStride; }

	// just calculate the number of DWORDs wee need
	// to store our data for our schema and given number of documents
	inline DWORD GetExpectedSize ( DWORD uMaxDocs ) const
	{
		DWORD uDocinfoIndex = ( uMaxDocs + CSphIndex_VLN::DOCINFO_INDEX_FREQ - 1 )
			/ CSphIndex_VLN::DOCINFO_INDEX_FREQ;
		return 2 * ( 1 + uDocinfoIndex ) * m_uStride;
	}
};


bool CSphIndex_VLN::PrecomputeMinMax()
{
	if ( !m_uDocinfo )
		return true;

	AttrIndexBuilder_c tBuilder ( m_tSchema );
	tBuilder.Prepare ( m_pDocinfoIndex, m_pDocinfoIndex + 2*( 1+m_uDocinfoIndex )*( DOCINFO_IDSIZE + m_tSchema.GetRowSize() ) );

	DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	DWORD uProgressEntry = 0;
	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_PRECOMPUTE;
	m_tProgress.m_iDone = 0;
	m_uMinMaxIndex = 0;

	for ( DWORD uIndexEntry=0; uIndexEntry<m_uDocinfo; uIndexEntry++ )
	{
		if ( !tBuilder.Collect ( &m_pDocinfo[uIndexEntry*uStride], m_pMva, m_sLastError ) )
			return false;
		m_uMinMaxIndex += uStride;
		// show progress
		if ( uIndexEntry==uProgressEntry )
		{
			uProgressEntry = Min ( uIndexEntry+1000, m_uDocinfoIndex-1 );
			if ( m_pProgress )
			{
				m_tProgress.m_iDone = (uIndexEntry+1)*1000/m_uDocinfoIndex;
				m_pProgress ( &m_tProgress, m_tProgress.m_iDone==1000 );
			}
		}
	}

	tBuilder.FinishCollect();
	return true;
}


bool CSphIndex_VLN::SaveAttributes ()
{
	if ( !m_uAttrsStatus || !m_uDocinfo )
		return true;

	if ( m_uAttrsStatus & ATTRS_MVA_UPDATED )
	{
		m_sLastError.SetSprintf ( "MVA updates are RAM-based only, saving is not (yet) possible" );
		return false;
	}

	assert ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && m_uDocinfo && m_pDocinfo.GetWritePtr() );

	// save current state
	CSphAutofile fdTmpnew ( GetIndexFileName("spa.tmpnew"), SPH_O_NEW, m_sLastError );
	if ( fdTmpnew.GetFD()<0 )
		return false;

	size_t uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	size_t uSize = uStride*size_t(m_uDocinfo)*sizeof(DWORD);
	if ( m_uVersion>=20 )
		uSize += 2*(1+m_uDocinfoIndex)*uStride*sizeof(CSphRowitem);

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

	m_uAttrsStatus = 0;
	return true;
}

/////////////////////////////////////////////////////////////////////////////

#define SPH_CMPHIT_LESS(a,b) \
	( a.m_iWordID<b.m_iWordID || \
	( a.m_iWordID==b.m_iWordID && a.m_iDocID<b.m_iDocID ) || \
	( a.m_iWordID==b.m_iWordID && a.m_iDocID==b.m_iDocID && a.m_iWordPos<b.m_iWordPos ) )


struct CmpHit_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b ) const
	{
		return SPH_CMPHIT_LESS ( a, b );
	}
};


/// sort baked docinfos by document ID
struct DocinfoSort_fn
{
	typedef SphDocID_t MEDIAN_TYPE;
	typedef DWORD * PTR_TYPE;

	int m_iStride;

	explicit DocinfoSort_fn ( int iStride )
		: m_iStride ( iStride )
	{}

	SphDocID_t Key ( DWORD * pData ) const
	{
		return DOCINFO2ID(pData);
	}

	void CopyKey ( SphDocID_t * pMed, DWORD * pVal ) const
	{
		*pMed = Key(pVal);
	}

	bool IsLess ( SphDocID_t a, SphDocID_t b ) const
	{
		return a < b;
	}

	void Swap ( DWORD * a, DWORD * b ) const
	{
		for ( int i=0; i<m_iStride; i++ )
			::Swap ( a[i], b[i] );
	}

	DWORD * Add ( DWORD * p, int i ) const
	{
		return p+i*m_iStride;
	}

	int Sub ( DWORD * b, DWORD * a ) const
	{
		return (int)((b-a)/m_iStride);
	}
};


void sphSortDocinfos ( DWORD * pBuf, int iCount, int iStride )
{
	DocinfoSort_fn fnSort ( iStride );
	sphSort ( pBuf, iCount, fnSort, fnSort );
}


CSphString CSphIndex_VLN::GetIndexFileName ( const char * sExt ) const
{
	CSphString sRes;
	sRes.SetSprintf ( "%s.%s", m_sFilename.cstr(), sExt );
	return sRes;
}


void CSphIndex_VLN::cidxHit ( CSphAggregateHit * hit, CSphRowitem * pAttrs )
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
		DWORD uLastPos = m_tLastHit.m_iWordPos;
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

			if ( m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )
			{
				bool bIgnoreHits =
					( m_tSettings.m_eHitless==SPH_HITLESS_ALL ) ||
					( m_tSettings.m_eHitless==SPH_HITLESS_SOME && ( m_iLastWordDocs & 0x80000000 ) );

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
					m_wrDoclist.ZipInt ( m_uLastDocFields );
					m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
				}

			} else // plain format - finish doclist entry
			{
				assert ( m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_PLAIN );
				m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
				m_wrDoclist.ZipInt ( m_uLastDocFields );
				m_wrDoclist.ZipInt ( m_uLastDocHits );
			}
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
			tCheckpoint.m_iWordlistOffset = m_wrWordlist.GetPos();

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
		DWORD uLastPos = m_tLastHit.m_iWordPos;
		if ( m_tLastHit.m_iWordPos )
		{
			m_wrHitlist.ZipInt ( 0 );
			m_tLastHit.m_iWordPos = 0;
		}

		// close prev doc, if any
		if ( m_tLastHit.m_iDocID )
		{
			if ( m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )
			{
				bool bIgnoreHits =
					( m_tSettings.m_eHitless==SPH_HITLESS_ALL ) ||
					( m_tSettings.m_eHitless==SPH_HITLESS_SOME && ( m_iLastWordDocs & 0x80000000 ) );

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
					m_wrDoclist.ZipInt ( m_uLastDocFields );
					m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
				}

			} else // plain format - finish doclist entry
			{
				assert ( m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_PLAIN );
				m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
				m_wrDoclist.ZipInt ( m_uLastDocFields );
				m_wrDoclist.ZipInt ( m_uLastDocHits );
			}
			m_uLastDocFields = 0;
			m_uLastDocHits = 0;
		}

		// add new doclist entry for new doc id
		assert ( hit->m_iDocID>m_tLastHit.m_iDocID );
		assert ( m_wrHitlist.GetPos()>=m_iLastHitlistPos );

		m_wrDoclist.ZipOffset ( hit->m_iDocID - m_tLastHit.m_iDocID );
		if ( pAttrs )
		{
			for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
				m_wrDoclist.ZipInt ( pAttrs[i] - m_tMin.m_pDynamic[i] );
		}
		m_iLastHitlistDelta = m_wrHitlist.GetPos() - m_iLastHitlistPos;

		m_tLastHit.m_iDocID = hit->m_iDocID;
		m_iLastHitlistPos = m_wrHitlist.GetPos();

		// update per-word stats
		m_iLastWordDocs++;
	}

	///////////
	// the hit
	///////////

	if ( hit->m_uFieldMask ) // merge aggregate hits into the current hit
	{
		assert ( m_tSettings.m_eHitless );
		assert ( hit->m_uHitCount );
		assert ( hit->m_uFieldMask );

		m_uLastDocHits += hit->m_uHitCount;
		m_uLastDocFields |= hit->m_uFieldMask;
		m_iLastWordHits += hit->m_uHitCount;

		if ( m_tSettings.m_eHitless==SPH_HITLESS_SOME )
			m_iLastWordDocs |= 0x80000000;

	} else // handle normal hits
	{
		// add hit delta
		if ( hit->m_iWordPos==m_tLastHit.m_iWordPos )
			return;

		assert ( hit->m_iWordPos > m_tLastHit.m_iWordPos );
		m_wrHitlist.ZipInt ( hit->m_iWordPos - m_tLastHit.m_iWordPos );
		m_tLastHit.m_iWordPos = hit->m_iWordPos;
		m_iLastWordHits++;

		// update matched fields mask
		m_uLastDocFields |= ( 1UL << HIT2FIELD ( hit->m_iWordPos ) );
		m_uLastDocHits++;
	}
}


void CSphIndex_VLN::WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol )
{
	int iLen = strlen ( tCol.m_sName.cstr() );
	fdInfo.PutDword ( iLen );
	fdInfo.PutBytes ( tCol.m_sName.cstr(), iLen );

	DWORD eAttrType = tCol.m_eAttrType;
	if ( eAttrType==SPH_ATTR_WORDCOUNT )
		eAttrType = SPH_ATTR_INTEGER;
	fdInfo.PutDword ( eAttrType );

	fdInfo.PutDword ( tCol.m_tLocator.CalcRowitem() ); // for backwards compatibility
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitOffset );
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitCount );

	fdInfo.PutByte ( tCol.m_bPayload );
}


void CSphIndex_VLN::ReadSchemaColumn ( CSphReader & rdInfo, CSphColumnInfo & tCol )
{
	tCol.m_sName = rdInfo.GetString ();
	if ( tCol.m_sName.IsEmpty () )
		tCol.m_sName = "@emptyname";

	tCol.m_sName.ToLower ();
	tCol.m_eAttrType = rdInfo.GetDword ();

	if ( m_uVersion>=5 )
	{
		rdInfo.GetDword (); // ignore rowitem
		tCol.m_tLocator.m_iBitOffset = rdInfo.GetDword ();
		tCol.m_tLocator.m_iBitCount = rdInfo.GetDword ();
	} else
	{
		tCol.m_tLocator.m_iBitOffset = -1;
		tCol.m_tLocator.m_iBitCount = -1;
	}

	if ( m_uVersion>=16 )
		tCol.m_bPayload = ( rdInfo.GetByte()!=0 );
}


bool CSphIndex_VLN::WriteHeader ( CSphWriter & fdInfo, SphOffset_t iCheckpointsPos, DWORD iCheckpointCount )
{
	// version
	fdInfo.PutDword ( INDEX_MAGIC_HEADER );
	fdInfo.PutDword ( INDEX_FORMAT_VERSION );

	// bits
	fdInfo.PutDword ( USE_64BIT );

	// docinfo
	fdInfo.PutDword ( m_tSettings.m_eDocinfo );

	// schema
	fdInfo.PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		WriteSchemaColumn ( fdInfo, m_tSchema.m_dFields[i] );

	fdInfo.PutDword ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( fdInfo, m_tSchema.GetAttr(i) );

	// min doc
	fdInfo.PutOffset ( m_tMin.m_iDocID ); // was dword in v.1
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
		fdInfo.PutBytes ( m_tMin.m_pDynamic, m_tSchema.GetRowSize()*sizeof(CSphRowitem) );

	// wordlist checkpoints
	fdInfo.PutBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	fdInfo.PutDword ( iCheckpointCount );

	// index stats
	fdInfo.PutDword ( m_tStats.m_iTotalDocuments );
	fdInfo.PutOffset ( m_tStats.m_iTotalBytes );

	// index settings
	SaveSettings ( fdInfo );

	// tokenizer info
	assert ( m_pTokenizer );
	SaveTokenizerSettings ( fdInfo, m_pTokenizer );

	// dictionary info
	assert ( m_pDict );
	SaveDictionarySettings ( fdInfo, m_pDict );

	fdInfo.PutDword ( m_iKillListSize );
	fdInfo.PutDword ( m_uMinMaxIndex );

	return true;
}


bool CSphIndex_VLN::cidxDone ( const char * sHeaderExtension )
{
	// flush wordlist checkpoints
	SphOffset_t iCheckpointsPos = m_wrWordlist.GetPos();
	ARRAY_FOREACH ( i, m_dWordlistCheckpoints )
	{
		m_wrWordlist.PutOffset ( m_dWordlistCheckpoints[i].m_iWordID );
		m_wrWordlist.PutOffset ( m_dWordlistCheckpoints[i].m_iWordlistOffset );
	}

	/////////////////
	// create header
	/////////////////

	CSphWriter fdInfo;
	fdInfo.OpenFile ( GetIndexFileName ( sHeaderExtension ), m_sLastError );
	if ( fdInfo.IsError() )
		return false;

	if ( !WriteHeader ( fdInfo, iCheckpointsPos, m_dWordlistCheckpoints.GetLength() ) )
		return false;

	////////////////////////
	// close all data files
	////////////////////////

	fdInfo.CloseFile ();
	m_wrWordlist.CloseFile ();
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ( true );

	if ( fdInfo.IsError() || m_wrWordlist.IsError() || m_wrDoclist.IsError() || m_wrHitlist.IsError() )
		return false;

	return true;
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


inline int encodeVLB8 ( BYTE * buf, uint64_t v )
{
	register BYTE b;
	register int n = 0;

	do {
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if ( v )
			b |= 0x80;
		*buf++ = b;
		n++;
	} while ( v );
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
	static const int HSIZE = ( 1 << HBITS );

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
			int iHash = (int)( ( DOCINFO2ID ( pDocinfo+i*iStride ) - iStartID ) >> iShift );
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
	SphWordID_t d1, l1 = 0;
	SphDocID_t d2, l2 = 0;
	DWORD d3, l3 = 0;

	pBuf = m_pWriteBuffer;
	maxP = m_pWriteBuffer + m_iWriteBuffer - 128;

	SphDocID_t iAttrID = 0; // current doc id
	DWORD * pAttrs = NULL; // current doc attrs

	// hit aggregation state
	DWORD uHitCount = 0;
	DWORD uHitFieldMask = 0;

	const int iPositionShift = m_tSettings.m_eHitless==SPH_HITLESS_SOME ? 1 : 0;

	while ( iHits-- )
	{
		// find attributes by id
		if ( pDocinfo && iAttrID!=pHit->m_iDocID )
		{
			int iHash = (int)( ( pHit->m_iDocID - iStartID ) >> iShift );
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
			assert ( DOCINFO2ID ( pAttrs - DOCINFO_IDSIZE )==pHit->m_iDocID );
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

		// when we moved to the next word or document
		bool bFlushed = false;
		if ( d1 || d2 )
		{
			// flush previous aggregate hit
			if ( uHitCount )
			{
				// we either skip all hits or the high bit must be available for marking
				// failing that, we can't produce a consistent index
				assert ( m_tSettings.m_eHitless!=SPH_HITLESS_NONE );
				assert ( m_tSettings.m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

				if ( m_tSettings.m_eHitless!=SPH_HITLESS_ALL )
					uHitCount = ( uHitCount << 1 ) | 1;
				pBuf += encodeVLB ( pBuf, uHitCount );
				pBuf += encodeVLB ( pBuf, uHitFieldMask );

				uHitCount = 0;
				uHitFieldMask = 0;

				bFlushed = true;
			}

			// start aggregating if we're skipping all hits or this word is in a list of ignored words
			if ( ( m_tSettings.m_eHitless==SPH_HITLESS_ALL ) ||
				( m_tSettings.m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.BinarySearch ( pHit->m_iWordID ) ) )
			{
				uHitCount = 1;
				uHitFieldMask |= 1 << HIT2FIELD ( pHit->m_iWordPos );
			}

		} else if ( uHitCount ) // next hit for the same word/doc pair, update state if we need it
		{
			uHitCount++;
			uHitFieldMask |= 1 << HIT2FIELD ( pHit->m_iWordPos );
		}

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 && !bFlushed ) pBuf += encodeVLB ( pBuf, 0 );

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
			for ( int i=0; i<iStride-DOCINFO_IDSIZE; i++ )
				pBuf += encodeVLB ( pBuf, pAttrs[i] );
		}

		assert ( d3 );
		if ( !uHitCount ) // encode position delta, unless accumulating hits
			pBuf += encodeVLB ( pBuf, d3 << iPositionShift );

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

	// flush last aggregate
	if ( uHitCount )
	{
		assert ( m_tSettings.m_eHitless!=SPH_HITLESS_NONE );
		assert ( m_tSettings.m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

		if ( m_tSettings.m_eHitless!=SPH_HITLESS_ALL )
			uHitCount = ( uHitCount << 1 ) | 1;
		pBuf += encodeVLB ( pBuf, uHitCount );
		pBuf += encodeVLB ( pBuf, uHitFieldMask );
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

	/// destory queue
	~CSphHitQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	void Push ( CSphAggregateHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		m_pData [ m_iUsed ].m_iDocID = tHit.m_iDocID;
		m_pData [ m_iUsed ].m_iWordID = tHit.m_iWordID;
		m_pData [ m_iUsed ].m_iWordPos = tHit.m_iWordPos;
		m_pData [ m_iUsed ].m_uFieldMask = tHit.m_uFieldMask;
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


bool CSphIndex_VLN::BuildMVA ( const CSphVector<CSphSource*> & dSources,
		CSphAutoArray<CSphWordHit> & dHits, int iArenaSize, int iFieldFD,
		int nFieldMVAs, int iFieldMVAInPool )
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
			if ( tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
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
	CSphAutofile fdTmpMva ( GetIndexFileName("tmp3"), SPH_O_NEW, m_sLastError, true );
	if ( fdTmpMva.GetFD()<0 )
		return false;

	//////////////////////////////
	// collect and partially sort
	//////////////////////////////

	CSphVector<int> dBlockLens;
	dBlockLens.Reserve ( 1024 );

	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT_MVA;

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

				if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
					continue;

				if ( !pSource->IterateMultivaluedStart ( iAttr, m_sLastError ) )
					return false;

				while ( pSource->IterateMultivaluedNext () )
				{
					pMva->m_uDocID = pSource->m_tDocInfo.m_iDocID;
					pMva->m_iAttr = i;
					pMva->m_uValue = MVA_DOWNSIZE ( pSource->m_tDocInfo.GetAttr ( tAttr.m_tLocator ) );

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

		dBins[iBin]->m_iFileLeft = ( i==nFieldBlocks-1 ? ( nLastBlockFieldMVAs ? nLastBlockFieldMVAs : iFieldMVAInPool ): iFieldMVAInPool ) * sizeof(MvaEntry_t);
		dBins[iBin]->m_iFilePos = uStart;
		dBins[iBin]->Init ( iFieldFD, &iSharedFieldOffset, iBinSize );

		uStart += dBins [iBin]->m_iFileLeft;
	}

	// do the sort
	CSphQueue < MvaEntryTag_t, MvaEntryCmp_fn > qMva ( Max ( 1, dBins.GetLength() ) );
	ARRAY_FOREACH ( i, dBins )
	{
		MvaEntryTag_t tEntry;
		if ( dBins[i]->ReadBytes ( (MvaEntry_t*) &tEntry, sizeof(MvaEntry_t) )!=BIN_READ_OK )
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
		assert ( dCurInfo [ qMva.Root().m_iAttr ].GetLength()==0
			|| dCurInfo [ qMva.Root().m_iAttr ].Last()<=qMva.Root().m_uValue );
#endif
		dCurInfo [ qMva.Root().m_iAttr ].AddUnique ( qMva.Root().m_uValue );

		// get next entry
		int iBin = qMva.Root().m_iTag;
		qMva.Pop ();

		MvaEntryTag_t tEntry;
		ESphBinRead iRes = dBins[iBin]->ReadBytes ( (MvaEntry_t*)&tEntry, sizeof(MvaEntry_t) );
		tEntry.m_iTag = iBin;

		if ( iRes==BIN_READ_OK )
			qMva.Push ( tEntry );

		if ( iRes==BIN_READ_ERROR )
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

	return true;
}


struct CmpOrdinalsValue_fn
{
	inline bool IsLess ( const Ordinal_t & a, const Ordinal_t & b ) const
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
	inline bool IsLess ( const OrdinalId_t & a, const OrdinalId_t & b ) const
	{
		return a.m_uDocID < b.m_uDocID;
	}
};


struct CmpMvaEntries_fn
{
	inline bool IsLess ( const MvaEntry_t & a, const MvaEntry_t & b ) const
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
		Ordinal_t & Ord = dOrdinals[i];

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
	if ( eRes!=BIN_READ_OK )
		return eRes;

	DWORD uStrLen;
	eRes = Reader.ReadBytes ( &uStrLen, sizeof ( DWORD ) );
	if ( eRes!=BIN_READ_OK )
		return eRes;

	if ( uStrLen>=(DWORD)MAX_ORDINAL_STR_LEN )
		return BIN_READ_ERROR;

	char dBuffer [MAX_ORDINAL_STR_LEN];

	if ( uStrLen > 0 )
	{
		eRes = Reader.ReadBytes ( dBuffer, uStrLen );
		if ( eRes!=BIN_READ_OK )
			return eRes;
	}

	dBuffer [uStrLen] = '\0';
	Ordinal.m_sValue = dBuffer;

	return BIN_READ_OK;
}


bool CSphIndex_VLN::SortOrdinals ( const char * szToFile, int iFromFD, int iArenaSize, int iOrdinalsInPool, CSphVector < CSphVector < SphOffset_t > > & dOrdBlockSize, bool bWarnOfMem )
{
	int nAttrs = dOrdBlockSize.GetLength ();
	int nBlocks = dOrdBlockSize[0].GetLength ();

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
		dStarts[i].Resize ( nBlocks );

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
			dBins[i].m_iFileLeft = (int)dOrdBlockSize[iAttr][i];
			dBins[i].m_iFilePos = dStarts[iAttr][i];
			dBins[i].Init ( iFromFD, &iSharedOffset, iBinSize );
		}

		dOrdBlockSize [iAttr].Resize ( 0 );

		for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
		{
			if ( ReadOrdinal ( dBins [iBlock], tOrdinalEntry )!=BIN_READ_OK )
			{
				m_sLastError = "sort_ordinals: warmup failed (io error?)";
				return false;
			}

			tOrdinalEntry.m_iTag = iBlock;
			qOrdinals.Push ( tOrdinalEntry );
		}

		SphDocID_t uCurID = 0;

		CSphString sLastOrdValue;
		int iMyBlock = 0;

		for ( ;; )
		{
			if ( !qOrdinals.GetLength () || qOrdinals.Root ().m_uDocID!=uCurID )
			{
				if ( uCurID )
				{
					OrdinalId_t tId;
					tId.m_uDocID = uCurID;
					tId.m_uId = uOrdinalId;
					dOrdinalIdPool.Add ( tId );

					if ( qOrdinals.GetLength () > 0 )
					{
						if ( sLastOrdValue.cstr()[0]!=qOrdinals.Root ().m_sValue.cstr()[0] )
							uOrdinalId++;
						else
							if ( strcmp ( sLastOrdValue.cstr (), qOrdinals.Root ().m_sValue.cstr () ) )
								uOrdinalId++;
					}

					if ( dOrdinalIdPool.GetLength()==iOrdinalsInPool )
					{
						dOrdinalIdPool.Sort ( CmpOrdinalsDocid_fn () );
						Writer.PutBytes ( &dOrdinalIdPool[0], sizeof(OrdinalId_t)*dOrdinalIdPool.GetLength() );
						if ( Writer.IsError () )
						{
							m_sLastError = "sort_ordinals: io error";
							return false;
						}

						dOrdBlockSize [iAttr].Add ( dOrdinalIdPool.GetLength () * sizeof ( OrdinalId_t ) );
						dOrdinalIdPool.Resize ( 0 );
					}
				}

				if ( !qOrdinals.GetLength () )
					break;

				uCurID = qOrdinals.Root().m_uDocID;
				const_cast < CSphString & > ( qOrdinals.Root ().m_sValue ).Swap ( sLastOrdValue );
			}

			// get next entry
			iMyBlock = qOrdinals.Root().m_iTag;
			qOrdinals.Pop ();

			ESphBinRead eRes = ReadOrdinal ( dBins [iMyBlock], tOrdinalEntry );
			tOrdinalEntry.m_iTag = iMyBlock;
			if ( eRes==BIN_READ_OK )
				qOrdinals.Push ( tOrdinalEntry );

			if ( eRes==BIN_READ_ERROR )
			{
				m_sLastError = "sort_ordinals: read error";
				return false;
			}
		}

		// flush last ordinal ids
		if ( dOrdinalIdPool.GetLength () )
		{
			dOrdinalIdPool.Sort ( CmpOrdinalsDocid_fn () );
			Writer.PutBytes ( &dOrdinalIdPool[0], sizeof(OrdinalId_t)*dOrdinalIdPool.GetLength () );
			if ( Writer.IsError () )
			{
				m_sLastError = "sort_ordinals: io error";
				return false;
			}

			dOrdBlockSize [iAttr].Add ( dOrdinalIdPool.GetLength()*sizeof(OrdinalId_t) );
			dOrdinalIdPool.Resize ( 0 );
		}
	}

	Writer.CloseFile ();
	if ( Writer.IsError () )
		return false;

	return true;
}


bool CSphIndex_VLN::SortOrdinalIds ( const char * szToFile, int iFromFD, int iArenaSize, CSphVector < CSphVector < SphOffset_t > > & dOrdBlockSize, bool bWarnOfMem )
{
	int nAttrs = dOrdBlockSize.GetLength ();
	int nMaxBlocks = 0;
	ARRAY_FOREACH ( i, dOrdBlockSize )
		if ( dOrdBlockSize[i].GetLength () > nMaxBlocks )
			nMaxBlocks = dOrdBlockSize[i].GetLength ();

	CSphWriter Writer;
	if ( !Writer.OpenFile ( szToFile, m_sLastError ) )
		return false;

	int iBinSize = CSphBin::CalcBinSize ( iArenaSize, nMaxBlocks, "ordinals", bWarnOfMem );

	SphOffset_t uStart = 0;
	OrdinalIdEntry_t tOrdinalIdEntry;
	OrdinalId_t tOrdinalId;

	for ( int iAttr = 0; iAttr < nAttrs; ++iAttr )
	{
		int nBlocks = dOrdBlockSize [iAttr].GetLength ();
		CSphQueue < OrdinalIdEntry_t, CmpOrdinalIdEntry_fn > qOrdinalIds ( Max ( 1, nBlocks ) );
		CSphVector < CSphBin > dBins;
		dBins.Resize ( nBlocks );

		SphOffset_t iSharedOffset = -1;

		ARRAY_FOREACH ( i, dBins )
		{
			dBins[i].m_iFileLeft = (int)dOrdBlockSize [iAttr][i];
			dBins[i].m_iFilePos = uStart;
			dBins[i].Init ( iFromFD, &iSharedOffset, iBinSize );

			uStart += dBins[i].m_iFileLeft;
		}

		for ( int iBlock = 0; iBlock < nBlocks; iBlock++ )
		{
			if ( dBins[iBlock].ReadBytes ( &tOrdinalId, sizeof ( tOrdinalId ) )!=BIN_READ_OK )
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

		SphOffset_t uResultSize = 0;

		for ( ;; )
		{
			if ( !qOrdinalIds.GetLength () || qOrdinalIds.Root ().m_uDocID!=tCachedId.m_uDocID )
			{
				if ( tCachedId.m_uDocID )
				{
					uResultSize += sizeof ( OrdinalId_t );
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
			if ( eRes==BIN_READ_OK )
				qOrdinalIds.Push ( tOrdinalIdEntry );

			if ( eRes==BIN_READ_ERROR )
			{
				m_sLastError = "sort_ordinals: read error";
				return false;
			}
		}

		dOrdBlockSize [iAttr].Resize ( 0 );
		dOrdBlockSize [iAttr].Add ( uResultSize );
	}

	return true;
}

struct FieldMVARedirect_t
{
	int					m_iAttr;
	int					m_iMVAAttr;
	CSphAttrLocator		m_tLocator;
};


bool CSphIndex_VLN::RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize, SphOffset_t * pFileSize, CSphBin * pMinBin, SphOffset_t * pSharedOffset )
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
		sphSeek ( iFile, iBlockStart + uTotalRead, SEEK_SET );

		int iToRead = i==nTransfers-1 ? (int)( iBlockLeft % iRelocationSize ) : iRelocationSize;
		size_t iRead = sphReadThrottled ( iFile, pBuffer, iToRead );
		if ( iRead!=size_t(iToRead) )
		{
			m_sLastError.SetSprintf ( "block relocation: read error (%d of %d bytes read): %s", iRead, iToRead, strerror(errno) );
			return false;
		}

		sphSeek ( iFile, *pFileSize, SEEK_SET );
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


static int CountWords ( const CSphString & sData, ISphTokenizer * pTokenizer )
{
	BYTE * sField = (BYTE*) sData.cstr();
	if ( !sField )
		return 0;

	int iCount = 0;
	pTokenizer->SetBuffer ( sField, (int)strlen ( (char*)sField ) );
	while ( pTokenizer->GetToken() )
		iCount++;
	return iCount;
}

bool CSphIndex_VLN::LoadHitlessWords ()
{
	assert ( m_dHitlessWords.GetLength()==0 );

	if ( m_tSettings.m_sHitlessFile.IsEmpty() )
		return true;

	CSphAutofile tFile ( m_tSettings.m_sHitlessFile.cstr(), SPH_O_READ, m_sLastError );
	if ( tFile.GetFD()==-1 )
		return false;

	CSphVector<BYTE> dBuffer ( (int)tFile.GetSize() );
	if ( !tFile.Read ( &dBuffer[0], dBuffer.GetLength(), m_sLastError ) )
		return false;

	m_pTokenizer->SetBuffer ( &dBuffer[0], dBuffer.GetLength() );
	while ( BYTE * sToken = m_pTokenizer->GetToken() )
		m_dHitlessWords.Add ( m_pDict->GetWordID ( sToken ) );

	m_dHitlessWords.Sort();
	return true;
}


static bool sphTruncate ( int iFD )
{
#if USE_WINDOWS
	return SetEndOfFile ( (HANDLE) _get_osfhandle(iFD) )!=0;
#else
	return ::ftruncate ( iFD, ::lseek ( iFD, 0, SEEK_CUR ) )==0;
#endif
}


int CSphIndex_VLN::Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer )
{
	PROFILER_INIT ();

	assert ( dSources.GetLength() );

	if ( !LoadHitlessWords() )
		return 0;

	m_iWriteBuffer = ( iWriteBuffer>0 )
		? Max ( iWriteBuffer, MIN_WRITE_BUFFER )
		: DEFAULT_WRITE_BUFFER;

	if ( !m_pWriteBuffer )
		m_pWriteBuffer = new BYTE [ m_iWriteBuffer ];

	// vars shared between phases
	CSphVector<CSphBin*> dBins;
	SphOffset_t iSharedOffset = -1;

	// setup sources
	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		pSource->SetDict ( m_pDict );
		pSource->Setup ( m_tSettings );
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
		m_tSettings.m_eDocinfo = SPH_DOCINFO_NONE;

	if ( m_tSchema.GetAttrsCount()>0 && m_tSettings.m_eDocinfo==SPH_DOCINFO_NONE )
	{
		m_sLastError.SetSprintf ( "got attributes, but docinfo is 'none' (fix your config file)" );
		return 0;
	}

	bool bHaveFieldMVAs = false;
	CSphVector<int> dMvaIndexes;
	CSphVector<CSphAttrLocator> dMvaLocators;

	// ordinals and strings storage
	CSphVector<int> dOrdinalAttrs;
	CSphVector<int> dStringAttrs;
	CSphVector<int> dWordcountAttrs;

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(i);
		DWORD eAttrType = tCol.m_eAttrType;

		if ( eAttrType & SPH_ATTR_MULTI )
		{
			if ( tCol.m_eSrc==SPH_ATTRSRC_FIELD )
				bHaveFieldMVAs = true;

			dMvaIndexes.Add ( i );
			dMvaLocators.Add ( tCol.m_tLocator );
		}
		switch ( eAttrType )
		{
		case SPH_ATTR_ORDINAL:
			if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN )
				dOrdinalAttrs.Add ( i );
			break;
		case SPH_ATTR_STRING:
			dStringAttrs.Add ( i );
			break;
		case SPH_ATTR_WORDCOUNT:
			dWordcountAttrs.Add ( i );
			break;
		default:
			break;
		}
	}

	bool bGotMVA = ( dMvaIndexes.GetLength()!=0 );
	if ( bGotMVA && m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		m_sLastError.SetSprintf ( "multi-valued attributes require docinfo=extern (fix your config file)" );
		return 0;
	}

	////////////////////////////////////////////////
	// collect and partially sort hits and docinfos
	////////////////////////////////////////////////

	// killlist storage
	CSphVector <SphAttr_t> dKillList;

	// adjust memory requirements
	// ensure there's enough memory to hold 1M hits and 64K docinfos
	// alloc 1/16 of memory (but not less than 64K entries) for docinfos
	iMemoryLimit = AdjustMemoryLimit ( iMemoryLimit );

	const int iDocinfoStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	int iDocinfoMax = Max ( 65536, iMemoryLimit/16/iDocinfoStride/sizeof(DWORD) );
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_NONE )
		iDocinfoMax = 1;

	int iOrdinalPoolSize = Max ( 32768, iMemoryLimit/8 );
	if ( dOrdinalAttrs.GetLength()==0 )
		iOrdinalPoolSize = 0;

	int iFieldMVAPoolSize = Max ( 32768, iMemoryLimit/16 );
	if ( bHaveFieldMVAs==0 )
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
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_NONE )
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
		dOrdinals[i].Reserve ( 65536 );

	CSphVector < CSphVector<SphOffset_t> > dOrdBlockSize;
	dOrdBlockSize.Resize ( dOrdinalAttrs.GetLength () );
	ARRAY_FOREACH ( i, dOrdBlockSize )
		dOrdBlockSize[i].Reserve ( 8192 );

	int iMaxOrdLen = 0;

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
	CSphAutofile fdLock ( GetIndexFileName("tmp0"), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdHits ( GetIndexFileName ( m_bInplaceSettings ? "spp" : "tmp1" ), SPH_O_NEW, m_sLastError, !m_bInplaceSettings );
	CSphAutofile fdDocinfos ( GetIndexFileName ( m_bInplaceSettings ? "spa" : "tmp2" ), SPH_O_NEW, m_sLastError, !m_bInplaceSettings );
	CSphAutofile fdTmpFieldMVAs ( GetIndexFileName("tmp7"), SPH_O_NEW, m_sLastError, true );
	CSphWriter tOrdWriter;
	CSphWriter tStrWriter;

	CSphString sRawOrdinalsFile = GetIndexFileName("tmp4");
	if ( bHaveOrdinals && !tOrdWriter.OpenFile ( sRawOrdinalsFile.cstr (), m_sLastError ) )
		return 0;

	if ( !tStrWriter.OpenFile ( GetIndexFileName("sps"), m_sLastError ) )
		return 0;
	tStrWriter.PutByte ( 0 ); // dummy byte, to reserve magic zero offset

	if ( fdLock.GetFD()<0 || fdHits.GetFD()<0 || fdDocinfos.GetFD()<0 || fdTmpFieldMVAs.GetFD ()<0 )
		return 0;

	SphOffset_t iHitsGap = 0;
	SphOffset_t iDocinfosGap = 0;

	if ( m_bInplaceSettings )
	{
		const int HIT_SIZE_AVG = 4;
		const float HIT_BLOCK_FACTOR = 1.0f;
		const float DOCINFO_BLOCK_FACTOR = 1.0f;

		if ( m_iHitGap )
			iHitsGap = (SphOffset_t) m_iHitGap;
		else
			iHitsGap = (SphOffset_t)( iHitsMax*HIT_BLOCK_FACTOR*HIT_SIZE_AVG );

		iHitsGap = Max ( iHitsGap, 1 );
		sphSeek ( fdHits.GetFD (), iHitsGap, SEEK_SET );

		if ( m_iDocinfoGap )
			iDocinfosGap = (SphOffset_t) m_iDocinfoGap;
		else
			iDocinfosGap = (SphOffset_t)( iDocinfoMax*DOCINFO_BLOCK_FACTOR*iDocinfoStride*sizeof(DWORD) );

		iDocinfosGap = Max ( iDocinfosGap, 1 );
		sphSeek ( fdDocinfos.GetFD (), iDocinfosGap, SEEK_SET );
	}

	if ( !sphLockEx ( fdLock.GetFD(), false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock '%s': another indexer running?", fdLock.GetFilename() );
		return 0;
	}

	// setup accumulating docinfo IDs range
	m_tMin.Reset ( m_tSchema.GetRowSize() );

	for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
		m_tMin.m_pDynamic[i] = ROWITEM_MAX;
	m_tMin.m_iDocID = DOCID_MAX;

	// build raw log
	PROFILE_BEGIN ( collect_hits );

	m_tStats.Reset ();
	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;
	m_tProgress.m_iAttrs = 0;

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
			int iAttr = dMvaIndexes[i];
			if ( m_tSchema.GetAttr(iAttr).m_eSrc==SPH_ATTRSRC_FIELD )
			{
				FieldMVARedirect_t & tRedirect = dFieldMvaIndexes.Add();
				tRedirect.m_iAttr = iAttr;
				tRedirect.m_iMVAAttr = i;
				tRedirect.m_tLocator = m_tSchema.GetAttr ( iAttr ).m_tLocator;
			}
		}

		// joined filter
		bool bGotJoined = ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_INLINE ) && pSource->HasJoinedFields();
		CSphVector<SphDocID_t> dAllIds; // FIXME! unlimited RAM use..

		// fetch documents
		for ( ;; )
		{
			// get next doc, and handle errors
			if ( !pSource->IterateHitsNext ( m_sLastError ) )
				return 0;

			// ensure docid is sane
			if ( pSource->m_tDocInfo.m_iDocID==DOCID_MAX )
			{
				m_sLastError.SetSprintf ( "docid==DOCID_MAX (source broken?)" );
				return 0;
			}

			// check for eof
			if ( !pSource->m_tDocInfo.m_iDocID )
				break;

			if ( bGotJoined )
				dAllIds.Add ( pSource->m_tDocInfo.m_iDocID );

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
					int iAttr = dFieldMvaIndexes[i].m_iAttr;
					int iMVA = dFieldMvaIndexes[i].m_iMVAAttr;
					CSphAttrLocator tLoc = dFieldMvaIndexes[i].m_tLocator;

					// store per-document MVAs
					if ( pSource->IterateFieldMVAStart ( iAttr, m_sLastError ) )
						while ( pSource->IterateFieldMVANext () )
						{
							MvaEntry_t & tMva = dFieldMVAs.Add();
							tMva.m_uDocID = pSource->m_tDocInfo.m_iDocID;
							tMva.m_iAttr = iMVA;
							tMva.m_uValue = MVA_DOWNSIZE ( pSource->m_tDocInfo.GetAttr ( tLoc ) );
							m_tProgress.m_iAttrs++;

							int iLength = dFieldMVAs.GetLength ();
							if ( iLength==iMaxPoolFieldMVAs )
							{
								dFieldMVAs.Sort ( CmpMvaEntries_fn () );
								if ( !sphWriteThrottled ( fdTmpFieldMVAs.GetFD (), &dFieldMVAs[0], iLength*sizeof(MvaEntry_t), "temp_field_mva", m_sLastError ) )
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
				dCol.Add();

				Ordinal_t & tLastOrd = dCol.Last();
				tLastOrd.m_uDocID = pSource->m_tDocInfo.m_iDocID;
				Swap ( tLastOrd.m_sValue, pSource->m_dStrAttrs[dOrdinalAttrs[i]] );
				int iOrdStrLen = strlen ( tLastOrd.m_sValue.cstr () );
				if ( iOrdStrLen > MAX_ORDINAL_STR_LEN )
				{
					iMaxOrdLen = iOrdStrLen;

					// truncate
					iOrdStrLen = MAX_ORDINAL_STR_LEN;
					tLastOrd.m_sValue = tLastOrd.m_sValue.SubString ( 0, iOrdStrLen - 1 );
				}

				iCurrentBlockSize += iOrdStrLen;
			}

			if ( bHaveOrdinals )
			{
				if ( iCurrentBlockSize>=iOrdinalPoolSize )
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

						dOrdBlockSize[i].Add ( uSize );
						dCol.Resize ( 0 );
					}
				}
			}

			// store strings
			ARRAY_FOREACH ( i, dStringAttrs )
			{
				// FIXME! optimize locators etc?
				// FIXME! support binary strings w/embedded zeroes?
				// get data, calc length
				const char * sData = pSource->m_dStrAttrs[dStringAttrs[i]].cstr();
				int iLen = sData ? strlen ( sData ) : 0;

				if ( iLen )
				{
					// calc offset, do sanity checks
					SphOffset_t uOff = tStrWriter.GetPos();
					if ( uint64_t(uOff)>>32 )
					{
						m_sLastError.SetSprintf ( "too many string attributes (current index format allows up to 4 GB)" );
						return 0;
					}
					pSource->m_tDocInfo.SetAttr ( m_tSchema.GetAttr ( dStringAttrs[i] ).m_tLocator, DWORD(uOff) );

					// pack length, emit it, emit data
					BYTE dPackedLen[4];
					int iLenLen = sphPackStrlen ( dPackedLen, iLen );
					tStrWriter.PutBytes ( &dPackedLen, iLenLen );
					tStrWriter.PutBytes ( sData, iLen );
				} else
				{
					// no data
					pSource->m_tDocInfo.SetAttr ( m_tSchema.GetAttr ( dStringAttrs[i] ).m_tLocator, 0 );
				}
			}

			// count words
			ARRAY_FOREACH ( i, dWordcountAttrs )
			{
				int iAttr = dWordcountAttrs[i];
				int iNumWords = CountWords ( pSource->m_dStrAttrs[iAttr], m_pTokenizer );
				pSource->m_tDocInfo.SetAttr ( m_tSchema.GetAttr(iAttr).m_tLocator, iNumWords );
			}

			// update min docinfo
			assert ( pSource->m_tDocInfo.m_iDocID );
			m_tMin.m_iDocID = Min ( m_tMin.m_iDocID, pSource->m_tDocInfo.m_iDocID );
			if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
				for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
					m_tMin.m_pDynamic[i] = Min ( m_tMin.m_pDynamic[i], pSource->m_tDocInfo.m_pDynamic[i] );

			// store docinfo
			if ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_NONE )
			{
				// store next entry
				DOCINFOSETID ( pDocinfo, pSource->m_tDocInfo.m_iDocID );
				memcpy ( DOCINFO2ATTRS ( pDocinfo ), pSource->m_tDocInfo.m_pDynamic, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );
				pDocinfo += iDocinfoStride;

				// if not inlining, flush buffer if it's full
				// (if inlining, it will flushed later, along with the hits)
				if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>=pDocinfoMax )
				{
					assert ( pDocinfo==pDocinfoMax );
					int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);

					sphSortDocinfos ( dDocinfos, iDocinfoMax, iDocinfoStride );
					if ( !sphWriteThrottled ( fdDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
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
				if ( pHits<pHitsMax && !( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE && pDocinfo>=pDocinfoMax ) )
					continue;

				// sort hits
				int iHits = pHits - dHits;
				{
					PROFILE ( sort_hits );
					sphSort ( &dHits[0], iHits, CmpHit_fn() );
				}
				pHits = dHits;

				if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
				{
					// we're inlining, so let's flush both hits and docs
					int iDocs = ( pDocinfo - dDocinfos ) / iDocinfoStride;
					pDocinfo = dDocinfos;

					sphSortDocinfos ( pDocinfo, iDocs, iDocinfoStride );

					dHitBlocks.Add ( cidxWriteRawVLB ( fdHits.GetFD(), dHits, iHits,
						dDocinfos, iDocs, iDocinfoStride ) );

					// we are inlining, so if there are more hits in this document,
					// we'll need to know it's info next flush
					if ( iDocHits )
					{
						DOCINFOSETID ( pDocinfo, pSource->m_tDocInfo.m_iDocID );
						memcpy ( DOCINFO2ATTRS ( pDocinfo ), pSource->m_tDocInfo.m_pDynamic, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );
						pDocinfo += iDocinfoStride;
					}
				} else
				{
					// we're not inlining, so only flush hits, docs are flushed independently
					dHitBlocks.Add ( cidxWriteRawVLB ( fdHits.GetFD(), dHits, iHits,
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

		// FIXME! uncontrolled memory usage; add checks and/or diskbased sort in the future?
		if ( pSource->IterateKillListStart ( m_sLastError ) )
		{
			SphDocID_t tDocId;
			while ( pSource->IterateKillListNext ( tDocId ) )
				dKillList.Add ( tDocId );
		}

		// fetch joined fields
		if ( bGotJoined )
		{
			dAllIds.Uniq();

			SphDocID_t uLastID = 0;
			bool bLastFound = 0;

			for ( ;; )
			{
				// get next doc, and handle errors
				if ( !pSource->IterateJoinedHits ( m_sLastError ) )
					return 0;

				// ensure docid is sane
				if ( pSource->m_tDocInfo.m_iDocID==DOCID_MAX )
				{
					m_sLastError.SetSprintf ( "joined_docid==DOCID_MAX (source broken?)" );
					return 0;
				}

				// check for eof
				if ( !pSource->m_tDocInfo.m_iDocID )
					break;

				// filter and store hits
				ARRAY_FOREACH ( i, pSource->m_dHits )
				{
					// flush if needed
					if ( pHits>=pHitsMax )
					{
						// sort hits
						int iHits = pHits - dHits;
						{
							PROFILE ( sort_hits );
							sphSort ( &dHits[0], iHits, CmpHit_fn() );
						}
						pHits = dHits;

						// we're not inlining, so only flush hits, docs are flushed independently
						dHitBlocks.Add ( cidxWriteRawVLB ( fdHits.GetFD(), dHits, iHits,
							NULL, 0, 0 ) );

						if ( dHitBlocks.Last()<0 )
							return 0;
					}

					// filter
					SphDocID_t uHitID = pSource->m_dHits[i].m_iDocID;
					if ( uHitID!=uLastID )
					{
						uLastID = uHitID;
						bLastFound = ( dAllIds.BinarySearch ( uHitID )!=NULL );
					}

					// copy next hit
					if ( bLastFound )
						*pHits++ = pSource->m_dHits[i];
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
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>dDocinfos )
	{
		iDocinfoLastBlockSize = ( pDocinfo - dDocinfos ) / iDocinfoStride;
		assert ( pDocinfo==( dDocinfos + iDocinfoLastBlockSize*iDocinfoStride ) );

		int iLen = iDocinfoLastBlockSize*iDocinfoStride*sizeof(DWORD);
		sphSortDocinfos ( dDocinfos, iDocinfoLastBlockSize, iDocinfoStride );
		if ( !sphWriteThrottled ( fdDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
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

		if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
		{
			int iDocs = ( pDocinfo - dDocinfos ) / iDocinfoStride;
			sphSortDocinfos ( dDocinfos, iDocs, iDocinfoStride );
			dHitBlocks.Add ( cidxWriteRawVLB ( fdHits.GetFD(), dHits, iHits,
				dDocinfos, iDocs, iDocinfoStride ) );
		} else
		{
			dHitBlocks.Add ( cidxWriteRawVLB ( fdHits.GetFD(), dHits, iHits, NULL, 0, 0 ) );
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
		if ( !sphWriteThrottled ( fdTmpFieldMVAs.GetFD (), &dFieldMVAs[0], iLength*sizeof(MvaEntry_t), "temp_field_mva", m_sLastError ) )
			return 0;

		dFieldMVAs.Reset ();
	}

	// flush last ordinals block
	if ( bHaveOrdinals && dOrdinals[0].GetLength () )
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

			dOrdBlockSize[i].Add ( uSize );
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

	if ( !BuildMVA ( dSources, dHits, iHitsMax*sizeof(CSphWordHit),
		fdTmpFieldMVAs.GetFD (), nFieldMVAs, iMaxPoolFieldMVAs ) )
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
	if ( bHaveOrdinals && !dOrdBlockSize[0].GetLength () )
	{
		bHaveOrdinals = false;
		::unlink ( sRawOrdinalsFile.cstr () );
	}

	if ( bHaveOrdinals )
	{
		if ( iMaxOrdLen > MAX_ORDINAL_STR_LEN )
			sphWarn ( "some ordinal attributes are too long (len=%d,max=%d)", iMaxOrdLen, MAX_ORDINAL_STR_LEN );

		CSphString sUnsortedIdFile = GetIndexFileName("tmp5");

		CSphAutofile fdRawOrdinals ( sRawOrdinalsFile.cstr (), SPH_O_READ, m_sLastError, true );
		if ( fdRawOrdinals.GetFD () < 0 )
			return 0;

		const float ARENA_PERCENT = 0.5f;
		int nBlocks = dOrdBlockSize[0].GetLength ();

		SphOffset_t uMemNeededForReaders = SphOffset_t ( nBlocks ) * uMaxOrdinalAttrBlockSize;
		SphOffset_t uMemNeededForSorting = sizeof ( OrdinalId_t ) * nOrdinals;

		int iArenaSize = (int) Min ( SphOffset_t ( iMemoryLimit * ARENA_PERCENT ), uMemNeededForReaders );
		iArenaSize = Max ( CSphBin::MIN_SIZE * nBlocks, iArenaSize );

		int iOrdinalsInPool = (int) Min ( SphOffset_t ( iMemoryLimit * ( 1.0f - ARENA_PERCENT ) ), uMemNeededForSorting ) / sizeof ( OrdinalId_t );

		if ( !SortOrdinals ( sUnsortedIdFile.cstr (), fdRawOrdinals.GetFD (), iArenaSize, iOrdinalsInPool, dOrdBlockSize, iArenaSize < uMemNeededForReaders ) )
			return 0;

		CSphAutofile fdUnsortedId ( sUnsortedIdFile.cstr (), SPH_O_READ, m_sLastError, true );
		if ( fdUnsortedId.GetFD () < 0 )
			return 0;

		iArenaSize = Min ( iMemoryLimit, (int)uMemNeededForSorting );
		iArenaSize = Max ( CSphBin::MIN_SIZE * ( nOrdinals / iOrdinalsInPool + 1 ), iArenaSize );

		if ( !SortOrdinalIds ( sSortedOrdinalIdFile.cstr (), fdUnsortedId.GetFD (), iArenaSize, dOrdBlockSize, iArenaSize < uMemNeededForSorting ) )
			return 0;
	}

	// initialize MVA reader
	CSphAutoreader rdMva;
	if ( !rdMva.Open ( GetIndexFileName("spm"), m_sLastError ) )
		return false;

	SphDocID_t uMvaID = rdMva.GetDocid();

	// initialize writer
	int iDocinfoFD = -1;
	SphOffset_t iDocinfoWritePos = 0;
	CSphScopedPtr<CSphAutofile> pfdDocinfoFinal ( NULL );

	if ( m_bInplaceSettings )
		iDocinfoFD = fdDocinfos.GetFD ();
	else
	{
		pfdDocinfoFinal = new CSphAutofile ( GetIndexFileName("spa"), SPH_O_NEW, m_sLastError );
		iDocinfoFD = pfdDocinfoFinal->GetFD();
		if ( iDocinfoFD < 0 )
			return 0;
	}

	int iDupes = 0;
	int iMinBlock = -1;

	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && dHitBlocks.GetLength() )
	{
		// initialize readers
		assert ( dBins.GetLength()==0 );
		dBins.Reserve ( iDocinfoBlocks );

		float fReadFactor = 1.0f;
		float fRelocFactor = 0.0f;
		if ( m_bInplaceSettings )
		{
			assert ( m_fRelocFactor > 0.005f && m_fRelocFactor < 0.95f );
			fRelocFactor = m_fRelocFactor;
			fReadFactor -= fRelocFactor;
		}

		int iBinSize = CSphBin::CalcBinSize ( int ( iMemoryLimit * fReadFactor ), iDocinfoBlocks, "sort_docinfos" );
		int iRelocationSize = m_bInplaceSettings ? int ( iMemoryLimit * fRelocFactor ) : 0;
		CSphAutoArray <BYTE> pRelocationBuffer ( iRelocationSize );
		iSharedOffset = -1;

		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			dBins.Add ( new CSphBin() );
			dBins[i]->m_iFileLeft = ( ( i==iDocinfoBlocks-1 ) ? iDocinfoLastBlockSize : iDocinfoMax )*iDocinfoStride*sizeof(DWORD);
			dBins[i]->m_iFilePos = ( i==0 ) ? iDocinfosGap : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
			dBins[i]->Init ( fdDocinfos.GetFD(), &iSharedOffset, iBinSize );
		}

		SphOffset_t iDocinfoFileSize = 0;
		if ( iDocinfoBlocks )
			iDocinfoFileSize = dBins [iDocinfoBlocks-1]->m_iFilePos + dBins [iDocinfoBlocks-1]->m_iFileLeft;

		// docinfo queue
		CSphAutoArray<DWORD> dDocinfoQueue ( iDocinfoBlocks*iDocinfoStride );
		CSphQueue < int, CmpQueuedDocinfo_fn > qDocinfo ( iDocinfoBlocks );

		CmpQueuedDocinfo_fn::m_pStorage = dDocinfoQueue;
		CmpQueuedDocinfo_fn::m_iStride = iDocinfoStride;

		pDocinfo = dDocinfoQueue;
		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			if ( dBins[i]->ReadBytes ( pDocinfo, iDocinfoStride*sizeof(DWORD) )!=BIN_READ_OK )
			{
				m_sLastError.SetSprintf ( "sort_docinfos: warmup failed (io error?)" );
				return 0;
			}
			pDocinfo += iDocinfoStride;
			qDocinfo.Push ( i );
		}

		CSphVector < CSphBin > dOrdReaders;
		SphOffset_t iSharedOrdOffset = -1;

		CSphAutofile fdTmpSortedIds ( sSortedOrdinalIdFile.cstr (), SPH_O_READ, m_sLastError, true );

		if ( bHaveOrdinals )
		{
			if ( fdTmpSortedIds.GetFD () < 0 )
				return 0;

			dOrdReaders.Resize ( dOrdinalAttrs.GetLength () );
			SphOffset_t uStart = 0;
			ARRAY_FOREACH ( i, dOrdReaders )
			{
				dOrdReaders[i].m_iFileLeft = (int)dOrdBlockSize [i][0];
				dOrdReaders[i].m_iFilePos = uStart;
				dOrdReaders[i].Init ( fdTmpSortedIds.GetFD(), &iSharedOrdOffset, ORDINAL_READ_SIZE );
				uStart += dOrdReaders[i].m_iFileLeft;
			}
		}

		// while the queue has data for us
		int iOrd = 0;
		pDocinfo = dDocinfos;
		SphDocID_t uLastId = 0;
		m_uMinMaxIndex = 0;

		// prepare the collector for min/max of attributes
		AttrIndexBuilder_c tMinMax ( m_tSchema );
		CSphVector<DWORD> dMinMaxBuffer ( tMinMax.GetExpectedSize ( m_tStats.m_iTotalDocuments ) );
		CSphDocMVA tCurInfo ( dMvaIndexes.GetLength() );
		tMinMax.Prepare ( &dMinMaxBuffer[0], &dMinMaxBuffer[0] + dMinMaxBuffer.GetLength() );

		SphDocID_t uLastDupe = 0;
		while ( qDocinfo.GetLength() )
		{
			// obtain bin index and next entry
			int iBin = qDocinfo.Root();
			DWORD * pEntry = dDocinfoQueue + iBin*iDocinfoStride;

			// skip duplicates
			if ( DOCINFO2ID ( pEntry )==uLastId )
			{
				// dupe, report it
				if ( m_tSettings.m_bVerbose && uLastDupe!=uLastId )
					sphWarn ( "duplicated document id="DOCID_FMT, uLastId );

				uLastDupe = uLastId;
				iDupes++;

			} else
			{
				// new unique document, handle it
				// update ordinals
				ARRAY_FOREACH ( i, dOrdinalAttrs )
				{
					OrdinalId_t Id;
					if ( dOrdReaders[i].ReadBytes ( &Id, sizeof(Id) )!=BIN_READ_OK )
					{
						m_sLastError = "update ordinals: io error";
						return 0;
					}

					assert ( Id.m_uDocID==DOCINFO2ID(pEntry) );
					sphSetRowAttr ( DOCINFO2ATTRS(pEntry), m_tSchema.GetAttr(dOrdinalAttrs[i]).m_tLocator, Id.m_uId );
				}
				iOrd++;
				m_uMinMaxIndex += iDocinfoStride;

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
							sphSetRowAttr ( DOCINFO2ATTRS(pEntry), dMvaLocators[i], SphAttr_t(rdMva.GetPos()/sizeof(DWORD)) ); // intentional clamp; we'll check for 32bit overflow later

							DWORD iMvaCount = rdMva.GetDword();
							tCurInfo.m_dMVA[i].Reserve ( iMvaCount );
							while ( iMvaCount-- )
								tCurInfo.m_dMVA[i].Add ( rdMva.GetDword() );
						}

						uMvaID = rdMva.GetDocid();
						if ( !uMvaID )
							uMvaID = DOCID_MAX;
					}
				}

				tMinMax.Collect ( pEntry, tCurInfo );

				ARRAY_FOREACH ( i, tCurInfo.m_dMVA )
					tCurInfo.m_dMVA[i].Resize ( 0 );

				// emit it
				memcpy ( pDocinfo, pEntry, iDocinfoStride*sizeof(DWORD) );
				pDocinfo += iDocinfoStride;

				uLastId = DOCINFO2ID(pEntry);

				if ( pDocinfo>=pDocinfoMax )
				{
					int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);

					if ( m_bInplaceSettings )
					{
						if ( iMinBlock==-1 || dBins[iMinBlock]->IsEOF () )
						{
							iMinBlock = -1;
							ARRAY_FOREACH ( i, dBins )
								if ( !dBins[i]->IsEOF () && ( iMinBlock==-1 || dBins [i]->m_iFilePos<dBins [iMinBlock]->m_iFilePos ) )
									iMinBlock = i;
						}

						if ( iMinBlock!=-1 && ( iDocinfoWritePos + iLen ) > dBins[iMinBlock]->m_iFilePos )
						{
							if ( !RelocateBlock ( iDocinfoFD, (BYTE*)pRelocationBuffer, iRelocationSize, &iDocinfoFileSize, dBins[iMinBlock], &iSharedOffset ) )
								return 0;

							iMinBlock = (iMinBlock+1) % dBins.GetLength ();
						}

						sphSeek ( iDocinfoFD, iDocinfoWritePos, SEEK_SET );
					}

					if ( !sphWriteThrottled ( iDocinfoFD, dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
						return 0;

					iDocinfoWritePos += iLen;
					pDocinfo = dDocinfos;
				}
			}

			// pop its index, update it, push its index again
			qDocinfo.Pop ();
			ESphBinRead eRes = dBins[iBin]->ReadBytes ( pEntry, iDocinfoStride*sizeof(DWORD) );
			if ( eRes==BIN_READ_ERROR )
			{
				m_sLastError.SetSprintf ( "sort_docinfo: failed to read entry" );
				return 0;
			}
			if ( eRes==BIN_READ_OK )
				qDocinfo.Push ( iBin );
		}

		if ( pDocinfo>dDocinfos )
		{
			assert ( 0==( pDocinfo-dDocinfos ) % iDocinfoStride );
			int iLen = ( pDocinfo - dDocinfos )*sizeof(DWORD);

			if ( m_bInplaceSettings )
				sphSeek ( iDocinfoFD, iDocinfoWritePos, SEEK_SET );

			if ( !sphWriteThrottled ( iDocinfoFD, dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
				return 0;

			if ( m_bInplaceSettings )
				if ( !sphTruncate ( iDocinfoFD ) )
					sphWarn ( "failed to truncate %s", fdDocinfos.GetFilename() );
		}
		tMinMax.FinishCollect();
		sphWriteThrottled ( iDocinfoFD, &dMinMaxBuffer[0],
			sizeof(DWORD) * tMinMax.GetActualSize(), "minmax_docinfo", m_sLastError );

		// clean up readers
		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );

		dBins.Reset ();
	}

	dDocinfos.Reset ();
	pDocinfo = NULL;

	// it might be zero-length, but it must exist
	if ( m_bInplaceSettings )
		fdDocinfos.Close ();
	else
	{
		assert ( pfdDocinfoFinal.Ptr () );
		pfdDocinfoFinal->Close ();
	}

	// dump killlist
	CSphAutofile fdKillList ( GetIndexFileName("spk"), SPH_O_NEW, m_sLastError );
	if ( fdKillList.GetFD()<0 )
		return 0;

	if ( dKillList.GetLength () )
	{
		dKillList.Uniq ();

		m_iKillListSize = dKillList.GetLength ();

		if ( !sphWriteThrottled ( fdKillList.GetFD (), &dKillList[0], m_iKillListSize*sizeof(SphAttr_t), "kill list", m_sLastError ) )
			return 0;
	}

	fdKillList.Close ();

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	PROFILE_BEGIN ( invert_hits );

	// initialize readers
	assert ( dBins.GetLength()==0 );
	dBins.Reserve ( dHitBlocks.GetLength() );

	iSharedOffset = -1;

	float fReadFactor = 1.0f;
	int iRelocationSize = 0;
	iWriteBuffer = m_iWriteBuffer;

	if ( m_bInplaceSettings )
	{
		assert ( m_fRelocFactor > 0.005f && m_fRelocFactor < 0.95f );
		assert ( m_fWriteFactor > 0.005f && m_fWriteFactor < 0.95f );
		assert ( m_fWriteFactor+m_fRelocFactor < 1.0f );

		fReadFactor -= m_fRelocFactor + m_fWriteFactor;

		iRelocationSize = int ( iMemoryLimit * m_fRelocFactor );
		iWriteBuffer = int ( iMemoryLimit * m_fWriteFactor );
	}

	int iBinSize = CSphBin::CalcBinSize ( int ( iMemoryLimit * fReadFactor ), dHitBlocks.GetLength(), "sort_hits" );

	CSphAutoArray <BYTE> pRelocationBuffer ( iRelocationSize );

	ARRAY_FOREACH ( i, dHitBlocks )
	{
		dBins.Add ( new CSphBin ( m_tSettings.m_eHitless ) );
		dBins[i]->m_iFileLeft = dHitBlocks[i];
		dBins[i]->m_iFilePos = ( i==0 ) ? iHitsGap : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( fdHits.GetFD(), &iSharedOffset, iBinSize );
	}

	// if there were no hits, create zero-length index files
	int iRawBlocks = dBins.GetLength();

	//////////////////////////////
	// create new index files set
	//////////////////////////////

	m_wrWordlist.CloseFile ();
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ();

	m_wrWordlist.SetBufferSize ( m_iWriteBuffer );
	m_wrDoclist.SetBufferSize ( m_iWriteBuffer );
	m_wrHitlist.SetBufferSize ( m_bInplaceSettings ? iWriteBuffer : m_iWriteBuffer );

	if (
		!m_wrWordlist.OpenFile ( GetIndexFileName("spi"), m_sLastError ) ||
		!m_wrDoclist.OpenFile ( GetIndexFileName("spd"), m_sLastError ) )
	{
		return 0;
	}

	if ( m_bInplaceSettings )
	{
		sphSeek ( fdHits.GetFD(), 0, SEEK_SET );
		m_wrHitlist.SetFile ( fdHits.GetFD(), &iSharedOffset );
	} else
		if ( !m_wrHitlist.OpenFile ( GetIndexFileName("spp"), m_sLastError ) )
			return 0;

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	m_wrWordlist.PutBytes ( &bDummy, 1 );
	m_wrDoclist.PutBytes ( &bDummy, 1 );
	m_wrHitlist.PutBytes ( &bDummy, 1 );

	// adjust min IDs, and fill header
	assert ( m_tMin.m_iDocID>0 );
	m_tMin.m_iDocID--;
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
		for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
			m_tMin.m_pDynamic[i]--;

	//////////////
	// final sort
	//////////////

	if ( iRawBlocks )
	{
		int iLastBin = dBins.GetLength () - 1;
		SphOffset_t iHitFileSize = dBins[iLastBin]->m_iFilePos + dBins [iLastBin]->m_iFileLeft;

		CSphHitQueue tQueue ( iRawBlocks );
		CSphAggregateHit tHit;

		m_tLastHit.m_iDocID = 0;
		m_tLastHit.m_iWordID = 0;
		m_tLastHit.m_iWordPos = 0;

		// initial fill
		int iRowitems = ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE ) ? m_tSchema.GetRowSize() : 0;
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
		iMinBlock = -1;
		while ( tQueue.m_iUsed )
		{
			int iBin = tQueue.m_pData->m_iBin;

			// pack and emit queue root
			tQueue.m_pData->m_iDocID -= m_tMin.m_iDocID;

			if ( m_bInplaceSettings )
			{
				if ( iMinBlock==-1 || dBins[iMinBlock]->IsEOF () || !bActive[iMinBlock] )
				{
					iMinBlock = -1;
					ARRAY_FOREACH ( i, dBins )
						if ( !dBins[i]->IsEOF () && bActive[i] && ( iMinBlock==-1 || dBins[i]->m_iFilePos < dBins[iMinBlock]->m_iFilePos ) )
							iMinBlock = i;
				}

				int iToWriteMax = 3*sizeof(DWORD);
				if ( iMinBlock!=-1 && ( m_wrHitlist.GetPos () + iToWriteMax ) > dBins[iMinBlock]->m_iFilePos )
				{
					if ( !RelocateBlock ( fdHits.GetFD (), (BYTE*)pRelocationBuffer, iRelocationSize, &iHitFileSize, dBins[iMinBlock], &iSharedOffset ) )
						return 0;

					iMinBlock = (iMinBlock+1) % dBins.GetLength ();
				}
			}

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
			m_tProgress.m_iHits = m_tProgress.m_iHitsTotal; // sum might be less than total because of dupes!
			m_pProgress ( &m_tProgress, true );
		}

		// cleanup
		SafeDeleteArray ( bActive );

		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );
		dBins.Reset ();

		CSphAggregateHit tFlush;
		tFlush.m_iDocID = 0;
		tFlush.m_iWordID = 0;
		tFlush.m_iWordPos = 0;
		tFlush.m_uFieldMask = 0;
		cidxHit ( &tFlush, NULL );

		if ( m_bInplaceSettings )
		{
			m_wrHitlist.CloseFile ();
			if ( !sphTruncate ( fdHits.GetFD () ) )
				sphWarn ( "failed to truncate %s", fdHits.GetFilename() );
		}
	}

	if ( iDupes )
		sphWarn ( "%d duplicate document id pairs found", iDupes );

	PROFILE_END ( invert_hits );

	// we're done
	if ( !cidxDone("sph") )
		return 0;

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	PROFILER_DONE ();
	PROFILE_SHOW ();
	return 1;
} // NOLINT function length


static bool CopyFile ( const char * sSrc, const char * sDst, CSphString & sErrStr )
{
	assert ( sSrc );
	assert ( sDst );

	const DWORD iMaxBufSize = 1024 * 1024;

	CSphAutofile tSrcFile ( sSrc, SPH_O_READ, sErrStr );
	CSphAutofile tDstFile ( sDst, SPH_O_NEW, sErrStr );

	if ( tSrcFile.GetFD()<0 || tDstFile.GetFD()<0 )
		return false;

	SphOffset_t iFileSize = tSrcFile.GetSize();
	DWORD iBufSize = (DWORD) Min ( iFileSize, (SphOffset_t)iMaxBufSize );

	if ( iFileSize )
	{
		BYTE * pData = new BYTE[iBufSize];

		if ( !pData )
		{
			sErrStr.SetSprintf ( "memory allocation error" );
			return false;
		}

		bool bError = true;

		while ( iFileSize > 0 )
		{
			DWORD iSize = (DWORD) Min ( iFileSize, (SphOffset_t)iBufSize );

			if ( !tSrcFile.Read ( pData, iSize, sErrStr ) )
				break;

			if ( !sphWriteThrottled ( tDstFile.GetFD(), pData, iSize, "CopyFile", sErrStr ) )
				break;

			iFileSize -= iSize;

			if ( !iFileSize )
				bError = false;
		}

		SafeDeleteArray ( pData );
		return ( bError==false );
	}

	return true;
}


SphAttr_t CopyStringAttr ( CSphWriter & wrTo, CSphReader & rdFrom, SphAttr_t uOffset )
{
	// magic offset? do nothing
	if ( !uOffset )
		return 0;

	// aim
	rdFrom.SeekTo ( uOffset, 0 );

	// read and decode length
	// MUST be in sync with sphUnpackStr
	int iLen = rdFrom.GetByte ();
	if ( iLen & 0x80 )
	{
		if ( iLen & 0x40 )
		{
			iLen = ( (int)( iLen & 0x3f )<<16 ) + ( rdFrom.GetByte()<<8 );
			iLen += rdFrom.GetByte(); // MUST be separate statement; cf. sequence point
		} else
		{
			iLen = ( (int)( iLen & 0x3f )<<8 ) + rdFrom.GetByte();
		}
	}

	// no data? do nothing
	if ( !iLen )
		return 0;

	// copy bytes
	uOffset = (SphAttr_t) wrTo.GetPos(); // FIXME! check bounds?

	BYTE dLen[4];
	wrTo.PutBytes ( dLen, sphPackStrlen ( dLen, iLen ) );

	while ( iLen>0 )
	{
		const BYTE * pBuf = NULL;
		int iChunk = rdFrom.GetBytesZerocopy ( &pBuf, iLen );
		wrTo.PutBytes ( pBuf, iChunk );
		iLen -= iChunk;
	}

	return uOffset;
}


class CSphDictReader
{
public:
	// current word
	SphWordID_t		m_iWordID;
	SphOffset_t		m_iDoclistOffset;
	int				m_iDocs;
	int				m_iHits;
	bool			m_bHasHitlist;

private:
	int				m_iEntries;

	ESphHitless		m_eHitless;
	CSphReader	m_tReader;
	CSphAutofile	m_tFile;
	SphOffset_t		m_iMaxPos;

public:
	CSphDictReader()
		: m_iWordID ( 0 )
		, m_iDoclistOffset ( 0 )
		, m_iEntries ( 0 )
		, m_iMaxPos ( 0 )
	{}

	void Setup ( const CSphString & sFilename, SphOffset_t iMaxPos, ESphHitless eHitless, CSphString & sError )
	{
		m_iMaxPos = iMaxPos;
		m_tFile.Open ( sFilename, SPH_O_READ, sError );
		m_tReader.SetFile ( m_tFile );
		m_tReader.SeekTo ( 1, READ_NO_SIZE_HINT );
		m_eHitless = eHitless;
	}

	bool Read()
	{
		if ( m_tReader.GetPos()>=m_iMaxPos )
			return false;

		SphWordID_t iDelta = m_tReader.UnzipWordid();
		if ( !iDelta ) // checkpoint
		{
			m_tReader.UnzipOffset();

			m_iEntries = 0;
			m_iWordID = 0;
			m_iDoclistOffset = 0;

			if ( m_tReader.GetPos()>=m_iMaxPos )
				return false;

			iDelta = m_tReader.UnzipWordid(); // get next word delta
		}

		if ( iDelta )
		{
			m_iWordID += iDelta;
			m_iDoclistOffset += m_tReader.UnzipOffset();

			m_iDocs = m_tReader.UnzipInt();
			m_iHits = m_tReader.UnzipInt();

			m_bHasHitlist =
				( m_eHitless==SPH_HITLESS_NONE ) ||
				( m_eHitless==SPH_HITLESS_SOME && !( m_iDocs & 0x80000000 ) );
			m_iDocs = m_eHitless==SPH_HITLESS_SOME ? ( m_iDocs & 0x7FFFFFFF ) : m_iDocs;
			m_iEntries++;
		}
		return iDelta!=0;
	}
};

static ISphFilter * CreateMergeFilters ( CSphVector<CSphFilterSettings> & dSettings, const CSphSchema & tSchema, const DWORD * pMvaPool )
{
	CSphString sError;
	ISphFilter * pResult = NULL;
	ARRAY_FOREACH ( i, dSettings )
	{
		ISphFilter * pFilter = sphCreateFilter ( dSettings[i], tSchema, pMvaPool, sError );
		if ( pFilter )
			pResult = sphJoinFilters ( pResult, pFilter );
	}
	return pResult;
}

class CSphMerger
{
private:
	CSphIndex_VLN * m_pOutputIndex;

public:
	explicit CSphMerger ( CSphIndex_VLN * pOutputIndex )
		: m_pOutputIndex ( pOutputIndex )
	{}
	template < typename QWORD > static inline
	void PrepareQword ( QWORD & tQword, const CSphDictReader & tReader, int iDynamic, SphDocID_t iMinID )
	{
		tQword.m_tDoc.Reset ( iDynamic );
		tQword.m_iMinID = iMinID;
		tQword.m_tDoc.m_iDocID = iMinID;

		tQword.m_iDocs = tReader.m_iDocs;
		tQword.m_iHits = tReader.m_iHits;
		tQword.m_bHasHitlist = tReader.m_bHasHitlist;

		tQword.m_uHitPosition = 0;
		tQword.m_iHitlistPos = 0;
	}

	template < typename QWORD >
	static inline bool NextDocument ( QWORD & tQword, CSphIndex_VLN * pSourceIndex, CSphRowitem * pInline, ISphFilter * pFilter )
	{
		for ( ;; )
		{
			tQword.GetNextDoc ( pInline );
			if ( tQword.m_tDoc.m_iDocID )
			{
				tQword.SeekHitlist ( tQword.m_iHitlistPos );
				if ( pFilter )
				{
					CSphMatch tMatch;
					tMatch.m_iDocID = tQword.m_tDoc.m_iDocID;
					if ( pFilter->UsesAttrs() )
						if ( pInline )
							tMatch.m_pDynamic = pInline;
						else
						{
							const DWORD * pInfo = pSourceIndex->FindDocinfo ( tQword.m_tDoc.m_iDocID );
							tMatch.m_pStatic = pInfo?DOCINFO2ATTRS ( pInfo ):NULL;
						}
					bool bResult = pFilter->Eval ( tMatch );
					tMatch.m_pDynamic = NULL;
					if ( !bResult )
					{
						while ( tQword.m_bHasHitlist && tQword.GetNextHit() );
						continue;
					}
				}
				return true;
			} else
				return false;
		}
	}

	template < typename QWORD >
	inline void TransferData ( QWORD & tQword, SphWordID_t iWordID, CSphIndex_VLN * pSourceIndex, CSphRowitem * pInline, ISphFilter * pFilter )
	{
		CSphAggregateHit tHit;
		tHit.m_iWordID = iWordID;
		tHit.m_uFieldMask = 0;

		while ( CSphMerger::NextDocument ( tQword, pSourceIndex, pInline, pFilter ) )
		{
			if ( tQword.m_bHasHitlist )
				TransferHits ( tQword, pInline, tHit );
			else
			{
				// convert to aggregate if there is no hit-list
				tHit.m_iDocID = tQword.m_tDoc.m_iDocID - m_pOutputIndex->m_tMin.m_iDocID;
				tHit.m_uFieldMask = tQword.m_uFields;
				tHit.m_uHitCount = tQword.m_uMatchHits;
				m_pOutputIndex->cidxHit ( &tHit, pInline );
			}
		}
	}

	template < typename QWORD >
	inline void TransferHits ( QWORD & tQword, CSphRowitem * pInline, CSphAggregateHit & tHit )
	{
		assert ( tQword.m_bHasHitlist );
		tHit.m_iDocID = tQword.m_tDoc.m_iDocID - m_pOutputIndex->m_tMin.m_iDocID;
		while ( DWORD uHit = tQword.GetNextHit() )
		{
			tHit.m_iWordPos = uHit;
			m_pOutputIndex->cidxHit ( &tHit, pInline );
		}
	}

	template < typename QWORD >
	static inline void ConfigureQword ( QWORD & tQword, CSphAutofile & tHits, CSphAutofile & tDocs, CSphIndex_VLN * pIndex )
	{
		bool bInline = pIndex->m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE;

		tQword.m_iInlineAttrs = bInline ? pIndex->m_tSchema.GetAttrsCount() : 0;
		tQword.m_pInlineFixup = bInline ? pIndex->m_tMin.m_pDynamic : NULL;

		tQword.m_rdHitlist.SetFile ( tHits );
		tQword.m_rdHitlist.GetByte();

		tQword.m_rdDoclist.SetFile ( tDocs );
		tQword.m_rdDoclist.GetByte();
	}
};


template < typename QWORDDST, typename QWORDSRC >
bool CSphIndex_VLN::MergeWords ( CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter )
{
	/// setup writers

	m_wrDoclist.OpenFile ( GetIndexFileName("spd.tmp"), m_sLastError );
	m_wrWordlist.OpenFile ( GetIndexFileName("spi.tmp"), m_sLastError );
	m_wrHitlist.OpenFile ( GetIndexFileName("spp.tmp"), m_sLastError );

	if ( !m_sLastError.IsEmpty() )
		return false;

	BYTE bDummy = 1;
	m_wrDoclist.PutBytes ( &bDummy, 1 );
	m_wrWordlist.PutBytes ( &bDummy, 1 );
	m_wrHitlist.PutBytes ( &bDummy, 1 );

	CSphDictReader tDstReader;
	CSphDictReader tSrcReader;

	tDstReader.Setup ( GetIndexFileName("spi"), m_iCheckpointsPos,
		m_tSettings.m_eHitless, m_sLastError );
	tSrcReader.Setup ( pSrcIndex->GetIndexFileName("spi"), pSrcIndex->m_iCheckpointsPos,
		pSrcIndex->m_tSettings.m_eHitless, m_sLastError );

	if ( !m_sLastError.IsEmpty() )
		return false;

	/// prepare for indexing

	m_tLastHit.m_iDocID = 0;
	m_tLastHit.m_iWordID = 0;
	m_tLastHit.m_iWordPos = 0;

	const SphDocID_t iDstMinID = m_tMin.m_iDocID;
	const SphDocID_t iSrcMinID = pSrcIndex->m_tMin.m_iDocID;

	// correct infinum might be already set during spa merging.
	if ( !m_iMergeInfinum )
		m_tMin.m_iDocID = Min ( iDstMinID, iSrcMinID );
	else
		m_tMin.m_iDocID = m_iMergeInfinum;

	m_dWordlistCheckpoints.Reset();

	const int iDstDynamic = m_tSchema.GetDynamicSize();
	const int iSrcDynamic = pSrcIndex->m_tSchema.GetDynamicSize();

	/// setup qwords

	QWORDDST tDstQword;
	QWORDSRC tSrcQword;

	CSphAutofile fSrcDocs, fSrcHits;
	fSrcDocs.Open ( pSrcIndex->GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	fSrcHits.Open ( pSrcIndex->GetIndexFileName("spp"), SPH_O_READ, m_sLastError );

	CSphAutofile fDstDocs, fDstHits;
	fDstDocs.Open ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	fDstHits.Open ( GetIndexFileName("spp"), SPH_O_READ, m_sLastError );

	if ( !m_sLastError.IsEmpty() )
		return false;

	CSphMerger::ConfigureQword<QWORDDST> ( tDstQword, fDstHits, fDstDocs, this );
	CSphMerger::ConfigureQword<QWORDSRC> ( tSrcQword, fSrcHits, fSrcDocs, pSrcIndex );

	int iDstInlineSize = m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE ? m_tSchema.GetRowSize() : 0;
	int iSrcInlineSize = pSrcIndex->m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE ? pSrcIndex->m_tSchema.GetRowSize() : 0;

	CSphAutoArray<CSphRowitem> dDstInline ( iDstInlineSize );
	CSphAutoArray<CSphRowitem> dSrcInline ( iSrcInlineSize );

	/// merge

	CSphMerger tMerge(this);

	bool bDstWord = tDstReader.Read();
	bool bSrcWord = tSrcReader.Read();

	if ( m_pProgress )
	{
		m_tProgress.m_ePhase = CSphIndexProgress::PHASE_MERGE;
		m_pProgress ( &m_tProgress, false );
	}

	int iWords = 0;
	int iHitlistsDiscarded = 0;
	while ( bDstWord || bSrcWord )
	{
		if ( iWords==1000 )
		{
			m_tProgress.m_iWords += 1000;
			iWords = 0;
			m_pProgress ( &m_tProgress, false );
		}

		if ( !bSrcWord || ( bDstWord && ( tDstReader.m_iWordID < tSrcReader.m_iWordID ) ) )
		{
			// transfer documents and hits from destination
			CSphMerger::PrepareQword<QWORDDST> ( tDstQword, tDstReader, iDstDynamic, iDstMinID );
			tMerge.TransferData<QWORDDST> ( tDstQword, tDstReader.m_iWordID, this, dDstInline, pFilter );
			iWords++;
			bDstWord = tDstReader.Read();

		} else if ( !bDstWord || ( bSrcWord && ( tDstReader.m_iWordID > tSrcReader.m_iWordID ) ) )
		{
			// transfer documents and hits from source
			CSphMerger::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader, iSrcDynamic, iSrcMinID );
			tMerge.TransferData<QWORDSRC> ( tSrcQword, tSrcReader.m_iWordID, pSrcIndex, dSrcInline, NULL );
			iWords++;
			bSrcWord = tSrcReader.Read();

		} else // merge documents and hits inside the word
		{
			assert ( tDstReader.m_iWordID==tSrcReader.m_iWordID );

			bool bHitless = !tDstReader.m_bHasHitlist;
			if ( tDstReader.m_bHasHitlist!=tSrcReader.m_bHasHitlist )
			{
				iHitlistsDiscarded++;
				bHitless = true;
			}

			CSphMerger::PrepareQword<QWORDDST> ( tDstQword, tDstReader, iDstDynamic, iDstMinID );
			CSphMerger::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader, iSrcDynamic, iSrcMinID );

			CSphAggregateHit tHit;
			tHit.m_iWordID = tDstReader.m_iWordID;
			tHit.m_uFieldMask = 0;

			bool bDstDocs = CSphMerger::NextDocument ( tDstQword, this, dDstInline, pFilter );
			bool bSrcDocs = true;

			tSrcQword.GetNextDoc ( dSrcInline );
			tSrcQword.SeekHitlist ( tSrcQword.m_iHitlistPos );

			while ( bDstDocs || bSrcDocs )
			{
				if ( !bSrcDocs || ( bDstDocs && tDstQword.m_tDoc.m_iDocID < tSrcQword.m_tDoc.m_iDocID ) )
				{
					// transfer hits from destination
					if ( bHitless )
					{
						while ( tDstQword.m_bHasHitlist && tDstQword.GetNextHit() );

						tHit.m_iDocID = tDstQword.m_tDoc.m_iDocID - m_tMin.m_iDocID;
						tHit.m_uFieldMask = tDstQword.m_uFields;
						tHit.m_uHitCount = tDstQword.m_uMatchHits;
						cidxHit ( &tHit, dSrcInline );
					} else
						tMerge.TransferHits ( tDstQword, dDstInline, tHit );
					bDstDocs = CSphMerger::NextDocument ( tDstQword, this, dDstInline, pFilter );

				} else if ( !bDstDocs || ( bSrcDocs && tDstQword.m_tDoc.m_iDocID > tSrcQword.m_tDoc.m_iDocID ) )
				{
					// transfer hits from source
					if ( bHitless )
					{
						while ( tSrcQword.m_bHasHitlist && tSrcQword.GetNextHit() );

						tHit.m_iDocID = tSrcQword.m_tDoc.m_iDocID - m_tMin.m_iDocID;
						tHit.m_uFieldMask = tSrcQword.m_uFields;
						tHit.m_uHitCount = tSrcQword.m_uMatchHits;
						cidxHit ( &tHit, dSrcInline );
					} else
						tMerge.TransferHits ( tSrcQword, dSrcInline, tHit );
					bSrcDocs = CSphMerger::NextDocument ( tSrcQword, pSrcIndex, dSrcInline, NULL );

				} else
				{
					// merge hits inside the document
					assert ( bDstDocs );
					assert ( bSrcDocs );
					assert ( tDstQword.m_tDoc.m_iDocID==tSrcQword.m_tDoc.m_iDocID );

					tHit.m_iDocID = tDstQword.m_tDoc.m_iDocID - m_tMin.m_iDocID;

					if ( bHitless )
					{
						while ( tDstQword.m_bHasHitlist && tDstQword.GetNextHit() );
						while ( tSrcQword.m_bHasHitlist && tSrcQword.GetNextHit() );

						tHit.m_uFieldMask = tDstQword.m_uFields | tSrcQword.m_uFields;
						tHit.m_uHitCount = tDstQword.m_uMatchHits + tSrcQword.m_uMatchHits;
						cidxHit ( &tHit, dSrcInline );

					} else
					{
						DWORD uDstHit = tDstQword.GetNextHit();
						DWORD uSrcHit = tSrcQword.GetNextHit();

						while ( uDstHit || uSrcHit )
						{
							if ( !uSrcHit || ( uDstHit && uDstHit<uSrcHit ) )
							{
								tHit.m_iWordPos = uDstHit;
								cidxHit ( &tHit, dSrcInline );
								uDstHit = tDstQword.GetNextHit();

							} else if ( !uDstHit || ( uSrcHit && uSrcHit<uDstHit ) )
							{
								tHit.m_iWordPos = uSrcHit;
								cidxHit ( &tHit, dSrcInline );
								uSrcHit = tSrcQword.GetNextHit();

							} else
							{
								assert ( uDstHit==uSrcHit );

								tHit.m_iWordPos = uDstHit;
								cidxHit ( &tHit, dSrcInline );

								uDstHit = tDstQword.GetNextHit();
								uSrcHit = tSrcQword.GetNextHit();
							}
						}
					}

					// next document
					bDstDocs = CSphMerger::NextDocument ( tDstQword, this, dDstInline, pFilter );
					bSrcDocs = CSphMerger::NextDocument ( tSrcQword, pSrcIndex, dSrcInline, NULL );
				}
			}
			// next word
			bDstWord = tDstReader.Read();
			bSrcWord = tSrcReader.Read();
			iWords++;
		}
	}

	m_tStats.m_iTotalDocuments += pSrcIndex->m_tStats.m_iTotalDocuments;
	m_tStats.m_iTotalBytes += pSrcIndex->m_tStats.m_iTotalBytes;

	m_tProgress.m_iWords += iWords;
	m_pProgress ( &m_tProgress, false );

	if ( iHitlistsDiscarded )
		m_sLastWarning.SetSprintf ( "discarded hitlists for %u words", iHitlistsDiscarded );

	return true;
}

bool CSphIndex_VLN::Merge ( CSphIndex * pSource, CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists )
{
	assert ( pSource );

	CSphIndex_VLN * pSrcIndex = dynamic_cast< CSphIndex_VLN * >( pSource );
	assert ( pSrcIndex );

	CSphString sWarning;
	if ( !Prealloc ( false, sWarning ) || !Preread() )
		return false;
	if ( !pSrcIndex->Prealloc ( false, sWarning ) || !pSrcIndex->Preread() )
	{
		m_sLastError.SetSprintf ( "source index preload failed: %s", pSrcIndex->GetLastError().cstr() );
		return false;
	}

	const CSphSchema & tDstSchema = m_tSchema;
	const CSphSchema & tSrcSchema = pSrcIndex->m_tSchema;
	if ( !tDstSchema.CompareTo ( tSrcSchema, m_sLastError ) )
		return false;

	if ( m_tSettings.m_eHitless!=pSrcIndex->m_tSettings.m_eHitless )
	{
		m_sLastError = "hitless settings must be the same on merged indices";
		return false;
	}

	// FIXME!
	if ( m_tSettings.m_eDocinfo!=pSrcIndex->m_tSettings.m_eDocinfo && !( !m_tStats.m_iTotalDocuments || !pSrcIndex->m_tStats.m_iTotalDocuments ) )
	{
		m_sLastError.SetSprintf ( "docinfo storage on non-empty indexes must be the same (dst docinfo %d, docs %d, src docinfo %d, docs %d",
			m_tSettings.m_eDocinfo, m_tStats.m_iTotalDocuments, pSrcIndex->m_tSettings.m_eDocinfo, pSrcIndex->m_tStats.m_iTotalDocuments );
		return false;
	}

	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	// create filters
	ISphFilter * pFilter = CreateMergeFilters ( dFilters, m_tSchema, GetMVAPool() );
	if ( !bMergeKillLists )
	{
		DWORD nKillListSize = pSrcIndex->GetKillListSize ();
		if ( nKillListSize )
		{
			CSphFilterSettings tKillListFilter;
			SphAttr_t * pKillList = pSrcIndex->GetKillList ();

			tKillListFilter.m_bExclude = true;
			tKillListFilter.m_eType = SPH_FILTER_VALUES;
			tKillListFilter.m_uMinValue = pKillList[0];
			tKillListFilter.m_uMaxValue = pKillList[nKillListSize -1];
			tKillListFilter.m_sAttrName = "@id";
			tKillListFilter.SetExternalValues ( pKillList, nKillListSize );

			ISphFilter * pKillListFilter =
				sphCreateFilter ( tKillListFilter, m_tSchema, GetMVAPool(), m_sLastError );
			pFilter = sphJoinFilters ( pFilter, pKillListFilter );
		}
	}

	/////////////////////////////////////////
	// merging attributes (.spa, .spm, .sps)
	/////////////////////////////////////////

	CSphAutoreader tDstSPM, tSrcSPM, tDstSPS, tSrcSPS;
	if ( !tDstSPM.Open ( GetIndexFileName("spm"), m_sLastError )
		|| !tSrcSPM.Open ( pSrcIndex->GetIndexFileName("spm"), m_sLastError )
		|| !tDstSPS.Open ( GetIndexFileName("sps"), m_sLastError )
		|| !tSrcSPS.Open ( pSrcIndex->GetIndexFileName("sps"), m_sLastError ) )
	{
		return false;
	}

	CSphWriter tSPMWriter, tSPSWriter;
	if ( !tSPMWriter.OpenFile ( GetIndexFileName("spm.tmp"), m_sLastError )
		|| !tSPSWriter.OpenFile ( GetIndexFileName("sps.tmp"), m_sLastError ) )
	{
		return false;
	}
	tSPSWriter.PutByte ( 0 ); // dummy byte, to reserve magic zero offset

	/// merging
	CSphVector<CSphAttrLocator> dMvaLocators;
	CSphVector<CSphAttrLocator> dStringLocators;
	for ( int i=0; i<tDstSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tInfo = tDstSchema.GetAttr(i);
		if ( tInfo.m_eAttrType & SPH_ATTR_MULTI )
			dMvaLocators.Add ( tInfo.m_tLocator );
		if ( tInfo.m_eAttrType==SPH_ATTR_STRING )
			dStringLocators.Add ( tInfo.m_tLocator );
	}

	CSphDocMVA	tDstMVA ( dMvaLocators.GetLength() ), tSrcMVA ( dMvaLocators.GetLength() );
	CSphVector<SphAttr_t> dPhantomKiller;

	int iTotalDocuments = 0;
	bool bNeedInfinum = true;
	m_iMergeInfinum = 0;

	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && pSrcIndex->m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN )
	{
		CSphWriter wrRows;
		if ( !wrRows.OpenFile ( GetIndexFileName("spa.tmp"), m_sLastError ) )
			return false;

		AttrIndexBuilder_c tMinMax ( m_tSchema );
		CSphVector<DWORD> dMinMaxBuffer ( tMinMax.GetExpectedSize (
			m_tStats.m_iTotalDocuments + pSrcIndex->GetStats().m_iTotalDocuments ) );
		tMinMax.Prepare ( &dMinMaxBuffer[0], &dMinMaxBuffer[0] + dMinMaxBuffer.GetLength() );
		m_uMinMaxIndex = 0;

		DWORD * pSrcRow = pSrcIndex->m_pDocinfo.GetWritePtr(); // they *can* be null if the respective index is empty
		DWORD * pDstRow = m_pDocinfo.GetWritePtr();

		DWORD iSrcCount = 0;
		DWORD iDstCount = 0;

		tDstMVA.Read ( tDstSPM );
		tSrcMVA.Read ( tSrcSPM );

		CSphMatch tMatch;
		while ( iSrcCount < pSrcIndex->m_uDocinfo || iDstCount < m_uDocinfo )
		{
			SphDocID_t iDstDocID, iSrcDocID;

			if ( iDstCount < m_uDocinfo )
			{
				iDstDocID = DOCINFO2ID ( pDstRow );
				if ( pFilter )
				{
					tMatch.m_iDocID = iDstDocID;
					tMatch.m_pStatic = reinterpret_cast<CSphRowitem *> ( DOCINFO2ATTRS ( pDstRow ) );
					tMatch.m_pDynamic = NULL;
					if ( !pFilter->Eval ( tMatch ) )
					{
						pDstRow += iStride;
						iDstCount++;
						continue;
					}
				}
			} else
				iDstDocID = 0;

			if ( iSrcCount < pSrcIndex->m_uDocinfo )
				iSrcDocID = DOCINFO2ID ( pSrcRow );
			else
				iSrcDocID = 0;

			if ( ( iDstDocID && iDstDocID < iSrcDocID ) || ( iDstDocID && !iSrcDocID ) )
			{
				while ( tDstMVA.m_iDocID && tDstMVA.m_iDocID<iDstDocID )
					tDstMVA.Read ( tDstSPM );

				if ( tDstMVA.m_iDocID==iDstDocID )
				{
					tDstMVA.Write ( tSPMWriter );
					ARRAY_FOREACH ( i, tDstMVA.m_dMVA )
						sphSetRowAttr ( DOCINFO2ATTRS ( pDstRow ), dMvaLocators[i], tDstMVA.m_dOffsets[i] );
				}

				ARRAY_FOREACH ( i, dStringLocators )
					sphSetRowAttr ( DOCINFO2ATTRS ( pDstRow ), dStringLocators[i],
						CopyStringAttr ( tSPSWriter, tDstSPS, sphGetRowAttr ( DOCINFO2ATTRS ( pDstRow ), dStringLocators[i] ) ) );

				wrRows.PutBytes ( pDstRow, sizeof(DWORD)*iStride );
				tMinMax.Collect ( pDstRow, tDstMVA );
				m_uMinMaxIndex += iStride;
				pDstRow += iStride;
				iDstCount++;
				iTotalDocuments++;
				if ( bNeedInfinum )
				{
					bNeedInfinum = false;
					m_iMergeInfinum = iDstDocID - 1;
				}

			} else if ( iSrcDocID )
			{
				// iSrcDocID<=iDstDocID; in both cases, its src attr values that must win
				while ( tSrcMVA.m_iDocID && tSrcMVA.m_iDocID<iSrcDocID )
					tSrcMVA.Read ( tSrcSPM );

				if ( tSrcMVA.m_iDocID==iSrcDocID )
				{
					tSrcMVA.Write ( tSPMWriter );
					ARRAY_FOREACH ( i, tSrcMVA.m_dMVA )
						sphSetRowAttr ( DOCINFO2ATTRS ( pSrcRow ), dMvaLocators[i], tSrcMVA.m_dOffsets[i] );
				}

				ARRAY_FOREACH ( i, dStringLocators )
					sphSetRowAttr ( DOCINFO2ATTRS ( pSrcRow ), dStringLocators[i],
						CopyStringAttr ( tSPSWriter, tSrcSPS, sphGetRowAttr ( DOCINFO2ATTRS ( pSrcRow ), dStringLocators[i] ) ) );

				wrRows.PutBytes ( pSrcRow, sizeof(DWORD)*iStride );
				tMinMax.Collect ( pSrcRow, tSrcMVA );
				m_uMinMaxIndex += iStride;
				pSrcRow += iStride;
				iSrcCount++;
				iTotalDocuments++;
				if ( bNeedInfinum )
				{
					bNeedInfinum = false;
					m_iMergeInfinum = iSrcDocID - 1;
				}

				if ( iDstDocID==iSrcDocID )
				{
					dPhantomKiller.Add ( iSrcDocID );
					pDstRow += iStride;
					iDstCount++;
				}
			}
		}

		tMinMax.FinishCollect();
		wrRows.PutBytes ( &dMinMaxBuffer[0], sizeof(DWORD) * tMinMax.GetActualSize() );
		wrRows.CloseFile();
		if ( wrRows.IsError() )
			return false;

	} else if ( !m_tStats.m_iTotalDocuments || !pSrcIndex->m_tStats.m_iTotalDocuments )
	{
		// one of the indexes has no documents; copy the .spa file from the other one
		CSphString sSrc = m_tStats.m_iTotalDocuments ? GetIndexFileName("spa") : pSrcIndex->GetIndexFileName("spa");
		CSphString sDst = GetIndexFileName("spa.tmp");

		if ( !CopyFile ( sSrc.cstr(), sDst.cstr(), m_sLastError ) )
			return false;

	} else
	{
		// storage is not extern; create dummy .spa file
		CSphAutofile fdSpa ( GetIndexFileName("spa.tmp"), SPH_O_NEW, m_sLastError );
		fdSpa.Close();
	}

	// create phantom killlist filter
	if ( dPhantomKiller.GetLength() )
	{
		CSphFilterSettings tKLF;
		tKLF.m_bExclude = true;
		tKLF.m_eType = SPH_FILTER_VALUES;
		tKLF.m_uMinValue = dPhantomKiller[0];
		tKLF.m_uMaxValue = dPhantomKiller.Last();
		tKLF.m_sAttrName = "@id";
		tKLF.SetExternalValues ( &dPhantomKiller[0], dPhantomKiller.GetLength() );
		ISphFilter * pSpaFilter = sphCreateFilter ( tKLF, m_tSchema, GetMVAPool(), m_sLastError );
		pFilter = sphJoinFilters ( pFilter, pSpaFilter );
	}
	CSphScopedPtr<ISphFilter> pScopedFilter ( pFilter );

	// merge dictionaries, doclists and hitlists

	WITH_QWORD ( this, true, QwordDst,
		WITH_QWORD ( pSrcIndex, true, QwordSrc,
		{
			if ( !MergeWords < QwordDst, QwordSrc > ( pSrcIndex, pFilter ) )
				return false;
		} ) );

	if ( iTotalDocuments )
		m_tStats.m_iTotalDocuments = iTotalDocuments;
	// merge kill-lists
	CSphAutofile fdKillList ( GetIndexFileName("spk.tmp"), SPH_O_NEW, m_sLastError );
	if ( fdKillList.GetFD () < 0 )
		return false;

	if ( bMergeKillLists )
	{
		// merge spk
		CSphVector<SphAttr_t> dKillList;
		dKillList.Reserve ( 1024 );
		for ( int i = 0; i < pSrcIndex->GetKillListSize (); i++ )
			dKillList.Add ( pSrcIndex->GetKillList () [i] );

		for ( int i = 0; i < GetKillListSize (); i++ )
			dKillList.Add ( GetKillList () [i] );

		dKillList.Uniq ();

		m_iKillListSize = dKillList.GetLength ();

		if ( dKillList.GetLength() )
		{
			if ( !sphWriteThrottled ( fdKillList.GetFD(), &dKillList[0], dKillList.GetLength()*sizeof(SphAttr_t), "kill_list", m_sLastError ) )
				return false;
		}
	}

	fdKillList.Close ();

	// finalize
	CSphAggregateHit tFlush;
	tFlush.m_iDocID = 0;
	tFlush.m_iWordID = 0;
	tFlush.m_iWordPos = 0;
	tFlush.m_uFieldMask = 0;
	cidxHit ( &tFlush, NULL );
	cidxDone ( "sph.tmp" );

	// we're done
	if ( m_pProgress )
		m_pProgress ( &m_tProgress, true );

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

/// dict traits
class CSphDictTraits : public CSphDict
{
public:
	explicit			CSphDictTraits ( CSphDict * pDict ) : m_pDict ( pDict ) { assert ( m_pDict ); }

	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer ) { m_pDict->LoadStopwords ( sFiles, pTokenizer ); }
	virtual bool		LoadWordforms ( const char * sFile, ISphTokenizer * pTokenizer ) { return m_pDict->LoadWordforms ( sFile, pTokenizer ); }
	virtual bool		SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError ) { return m_pDict->SetMorphology ( szMorph, bUseUTF8, sError ); }

	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) { return m_pDict->GetWordID ( pWord, iLen, bFilterStops ); }

	virtual void		Setup ( const CSphDictSettings & ) {}
	virtual const CSphDictSettings & GetSettings () const { return m_pDict->GetSettings (); }
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () { return m_pDict->GetStopwordsFileInfos (); }
	virtual const CSphSavedFile & GetWordformsFileInfo () { return m_pDict->GetWordformsFileInfo (); }
	virtual const CSphMultiformContainer * GetMultiWordforms () const { return m_pDict->GetMultiWordforms (); }

	virtual bool IsStopWord ( const BYTE * pWord ) const { return m_pDict->IsStopWord ( pWord ); }

protected:
	CSphDict *			m_pDict;
};


/// dict wrapper for star-syntax support in prefix-indexes
class CSphDictStar : public CSphDictTraits
{
public:
	explicit			CSphDictStar ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}

	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord );
};


SphWordID_t CSphDictStar::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];
	assert ( strlen ( (const char*)pWord ) < 16+3*SPH_MAX_WORD_LEN );

	m_pDict->ApplyStemmers ( pWord );

	int iLen = strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 1 );

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


SphWordID_t	CSphDictStar::GetWordIDNonStemmed ( BYTE * pWord )
{
	return m_pDict->GetWordIDNonStemmed ( pWord );
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
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	bool bHeadStar = ( pWord[0]=='*' );
	bool bTailStar = ( pWord[iLen-1]=='*' );

	if ( !bHeadStar && !bTailStar )
	{
		m_pDict->ApplyStemmers ( pWord );
		if ( IsStopWord ( pWord ) )
			return 0;
	}

	iLen = strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 2 );

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
	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !bHeadStar && !bTailStar );
}


/// dict wrapper for exact-word syntax
class CSphDictExact : public CSphDictTraits
{
public:
	explicit CSphDictExact ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}
	virtual SphWordID_t	GetWordID ( BYTE * pWord );
};


SphWordID_t CSphDictExact::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iLen = strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	if ( pWord[0]=='=' )
	{
		memcpy ( sBuf+1, pWord+1, iLen );
		sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
		return m_pDict->GetWordIDNonStemmed ( (BYTE*)sBuf );
	}

	memcpy ( sBuf, pWord, iLen+1 );
	return m_pDict->GetWordID ( (BYTE*)sBuf );
}


/////////////////////////////////////////////////////////////////////////////

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
	// might be needed even when we do not have a filter
	if ( pCtx->m_bLookupFilter )
		CopyDocinfo ( pCtx, tMatch, FindDocinfo ( tMatch.m_iDocID ) );
	pCtx->CalcFilter ( tMatch );

	return pCtx->m_pFilter ? !pCtx->m_pFilter->Eval ( tMatch ) : false;
}


SphAttr_t * CSphIndex_VLN::GetKillList () const
{
	return m_pKillList.GetWritePtr ();
}


bool CSphIndex_VLN::HasDocid ( SphDocID_t uDocid ) const
{
	return FindDocinfo ( uDocid )!=NULL;
}


const DWORD * CSphIndex_VLN::FindDocinfo ( SphDocID_t uDocID ) const
{
	if ( m_uDocinfo<=0 )
		return NULL;

	assert ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN );
	assert ( !m_pDocinfo.IsEmpty() );
	assert ( m_tSchema.GetAttrsCount() );

	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	int iStart = 0;
	int iEnd = m_uDocinfo-1;

	if ( m_pDocinfoHash.GetLength() )
	{
		SphDocID_t uFirst = DOCINFO2ID ( &m_pDocinfo[0] );
		SphDocID_t uLast = DOCINFO2ID ( &m_pDocinfo[(m_uDocinfo-1)*iStride] );
		if ( uDocID<uFirst || uDocID>uLast )
			return NULL;

		DWORD uHash = (DWORD)( ( uDocID - uFirst ) >> m_pDocinfoHash[0] );
		if ( uHash > ( 1 << DOCINFO_HASH_BITS ) ) // possible in case of broken data, for instance
			return NULL;

		iStart = m_pDocinfoHash [ uHash+1 ];
		iEnd = m_pDocinfoHash [ uHash+2 ] - 1;
	}

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

void CSphIndex_VLN::CopyDocinfo ( CSphQueryContext * pCtx, CSphMatch & tMatch, const DWORD * pFound ) const
{
	if ( !pFound )
		return;

	// setup static pointer
	assert ( DOCINFO2ID(pFound)==tMatch.m_iDocID );
	tMatch.m_pStatic = DOCINFO2ATTRS(pFound);

	// patch if necessary
	if ( pCtx->m_pOverrides )
		ARRAY_FOREACH ( i, (*pCtx->m_pOverrides) )
	{
		const CSphAttrOverride & tOverride = (*pCtx->m_pOverrides)[i]; // shortcut
		const CSphAttrOverride::IdValuePair_t * pEntry = tOverride.m_dValues.BinarySearch ( bind ( &CSphAttrOverride::IdValuePair_t::m_uDocID ), tMatch.m_iDocID );
		tMatch.SetAttr ( pCtx->m_dOverrideOut[i], pEntry
			? pEntry->m_uValue
			: sphGetRowAttr ( tMatch.m_pStatic, pCtx->m_dOverrideIn[i] ) );
	}
}


static inline void CalcContextItems ( CSphMatch & tMatch, const CSphVector<CSphQueryContext::CalcItem_t> & dItems )
{
	ARRAY_FOREACH ( i, dItems )
	{
		const CSphQueryContext::CalcItem_t & tCalc = dItems[i];
		if ( tCalc.m_uType==SPH_ATTR_INTEGER )
			tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->IntEval(tMatch) );
		else if ( tCalc.m_uType==SPH_ATTR_BIGINT )
			tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->Int64Eval(tMatch) );
		else
			tMatch.SetAttrFloat ( tCalc.m_tLoc, tCalc.m_pExpr->Eval(tMatch) );
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


bool CSphIndex_VLN::MatchExtended ( CSphQueryContext * pCtx, const CSphQuery * pQuery, int iSorters, ISphMatchSorter ** ppSorters, ISphRanker * pRanker, int iTag ) const
{
	int iCutoff = pQuery->m_iCutoff;
	if ( iCutoff<=0 )
		iCutoff = -1;

	// do searching
	CSphMatch * pMatch = pRanker->GetMatchesBuffer();
	for ( ;; )
	{
		int iMatches = pRanker->GetMatches ( pCtx->m_iWeights, pCtx->m_dWeights );
		if ( iMatches<=0 )
			break;

		for ( int i=0; i<iMatches; i++ )
		{
			if ( pCtx->m_bLookupSort )
				CopyDocinfo ( pCtx, pMatch[i], FindDocinfo ( pMatch[i].m_iDocID ) );
			pCtx->CalcSort ( pMatch[i] );

			if ( pCtx->m_pWeightFilter && !pCtx->m_pWeightFilter->Eval ( pMatch[i] ) )
				continue;

			pMatch[i].m_iTag = iTag;

			bool bRand = false;
			bool bNewMatch = false;
			for ( int iSorter=0; iSorter<iSorters; iSorter++ )
			{
				// all non-random sorters are in the beginning,
				// so we can avoid the simple 'first-element' assertion
				if ( !bRand && ppSorters[iSorter]->m_bRandomize )
				{
					bRand = true;
					pMatch[i].m_iWeight = ( sphRand() & 0xffff );

					if ( pCtx->m_pWeightFilter && !pCtx->m_pWeightFilter->Eval ( pMatch[i] ) )
						break;
				}
				bNewMatch |= ppSorters[iSorter]->Push ( pMatch[i] );
			}

			if ( bNewMatch )
				if ( --iCutoff==0 )
					break;
		}

		if ( iCutoff==0 )
			break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const
{
	assert ( pQuery->m_sQuery.IsEmpty() );

	// check if index is ready
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
	{
		pResult->m_sError = "index not preread";
		return false;
	}

	// check if index supports scans
	if ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN || !m_tSchema.GetAttrsCount() )
	{
		pResult->m_sError = "fullscan requires extern docinfo";
		return false;
	}

	// check if index has data
	if ( !m_tStats.m_iTotalDocuments || m_uDocinfo<=0 || m_pDocinfo.IsEmpty() )
		return true;

	// start counting
	int64_t tmQueryStart = sphMicroTimer();

	// select the sorter with max schema
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i]->GetSchema().GetRowSize() > iMaxSchemaSize )
		{
			iMaxSchemaSize = ppSorters[i]->GetSchema().GetRowSize();
			iMaxSchemaIndex = i;
		}

	// setup calculations and result schema
	CSphQueryContext tCtx;
	if ( !tCtx.SetupCalc ( pResult, ppSorters[iMaxSchemaIndex]->GetSchema(), m_tSchema, GetMVAPool() ) )
		return false;

	// setup filters
	if ( !tCtx.CreateFilters ( true, &pQuery->m_dFilters, pResult->m_tSchema, GetMVAPool(), pResult->m_sError ) )
		return false;
	if ( !tCtx.CreateFilters ( true, pExtraFilters, pResult->m_tSchema, GetMVAPool(), pResult->m_sError ) )
		return false;

	// check if we can early reject the whole index
	if ( tCtx.m_pFilter && m_uDocinfoIndex )
	{
		DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ 2*m_uDocinfoIndex*uStride ] );
		DWORD * pMaxEntry = pMinEntry + uStride;

		if ( !tCtx.m_pFilter->EvalBlock ( pMinEntry, pMaxEntry ) )
		{
			pResult->m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
			return true;
		}
	}

	// setup lookup
	tCtx.m_bLookupFilter = false;
	tCtx.m_bLookupSort = true;

	// setup sorters vs. MVA
	for ( int i=0; i<iSorters; i++ )
		(ppSorters[i])->SetMVAPool ( m_pMva.GetWritePtr() );

	// setup overrides
	if ( !tCtx.SetupOverrides ( pQuery, pResult, m_tSchema ) )
		return false;

	// do scan
	bool bRandomize = ppSorters[0]->m_bRandomize;
	int iCutoff = pQuery->m_iCutoff;
	if ( iCutoff<=0 )
		iCutoff = -1;

	CSphMatch tMatch;
	tMatch.Reset ( pResult->m_tSchema.GetDynamicSize() );
	tMatch.m_iWeight = 1;

	DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	for ( DWORD uIndexEntry=0; uIndexEntry<m_uDocinfoIndex; uIndexEntry++ )
	{
		/////////////////////////
		// block-level filtering
		/////////////////////////

		const DWORD * pMin = &m_pDocinfoIndex[2*uIndexEntry*uStride];
		const DWORD * pMax = pMin + uStride;

		// check applicable filters
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->EvalBlock ( pMin, pMax ) )
			continue;

		///////////////////////
		// row-level filtering
		///////////////////////

		const DWORD * pBlockStart = &m_pDocinfo [ uStride*uIndexEntry*DOCINFO_INDEX_FREQ ];
		const DWORD * pBlockEnd = &m_pDocinfo [ uStride*( Min ( (uIndexEntry+1)*DOCINFO_INDEX_FREQ, m_uDocinfo ) - 1 ) ];

		for ( const DWORD * pDocinfo=pBlockStart; pDocinfo<=pBlockEnd; pDocinfo+=uStride )
		{
			tMatch.m_iDocID = DOCINFO2ID ( pDocinfo );
			CopyDocinfo ( &tCtx, tMatch, pDocinfo );

			// early filter only (no late filters in full-scan because of no @weight)
			tCtx.CalcFilter ( tMatch );
			if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
				continue;

			// submit match to sorters
			tCtx.CalcSort ( tMatch );
			if ( bRandomize )
				tMatch.m_iWeight = ( sphRand() & 0xffff );

			tMatch.m_iTag = iTag;

			bool bNewMatch = false;
			for ( int iSorter=0; iSorter<iSorters; iSorter++ )
				bNewMatch |= ppSorters[iSorter]->Push ( tMatch );

			// handle cutoff
			if ( bNewMatch )
				if ( --iCutoff==0 )
				{
					uIndexEntry = m_uDocinfoIndex; // outer break
					break;
				}
		}
		if ( iCutoff==0 )
			break;
	}

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength() )
		for ( int iSorter=0; iSorter<iSorters; iSorter++ )
	{
		ISphMatchSorter * pTop = ppSorters[iSorter];
		const int iCount = pTop->GetLength ();
		if ( !iCount )
			continue;

		CSphMatch * const pHead = pTop->First();
		CSphMatch * const pTail = pHead + iCount;
		for ( CSphMatch * pCur=pHead; pCur<pTail; pCur++ )
			tCtx.CalcFinal ( *pCur );
	}

	// done
	pResult->m_pMva = m_pMva.GetWritePtr();
	pResult->m_pStrings = m_pStrings.GetWritePtr();
	pResult->m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
	return true;
}

//////////////////////////////////////////////////////////////////////////////

ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & ) const
{
	WITH_QWORD ( m_pIndex, false, Qword, return new Qword() );
	return NULL;
}


bool DiskIndexQwordSetup_c::QwordSetup ( ISphQword * pWord ) const
{
	WITH_QWORD ( m_pIndex, false, Qword, return Setup<Qword> ( pWord ) );
	return false;
}


template < class Qword >
bool DiskIndexQwordSetup_c::Setup ( ISphQword * pWord ) const
{
	Qword * pMyWord = dynamic_cast<Qword*> ( pWord );

	if ( !pMyWord )
		return false;

	Qword & tWord = *pMyWord;

	// setup attrs
	tWord.m_tDoc.Reset ( m_iDynamicRowitems );
	tWord.m_iMinID = m_tMin.m_iDocID;
	tWord.m_tDoc.m_iDocID = m_tMin.m_iDocID;

	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
	{
		tWord.m_iInlineAttrs = m_iInlineRowitems;
		tWord.m_pInlineFixup = m_tMin.m_pDynamic;
	} else
	{
		tWord.m_iInlineAttrs = 0;
		tWord.m_pInlineFixup = NULL;
	}

	// setup stats
	tWord.m_iDocs = 0;
	tWord.m_iHits = 0;

	CSphIndex_VLN * pIndex = (CSphIndex_VLN *)m_pIndex;

	// binary search through checkpoints for a one whose range matches word ID
	assert ( pIndex->m_bPreread[0] );
	assert ( !pIndex->m_bPreloadWordlist || !pIndex->m_pWordlist.IsEmpty() );

	// empty index?
	if ( !pIndex->m_dWordlistCheckpoints.GetLength() )
		return false;

	SphOffset_t uWordlistOffset = 0;
	const CSphWordlistCheckpoint * pStart = &pIndex->m_dWordlistCheckpoints[0];
	const CSphWordlistCheckpoint * pEnd = &pIndex->m_dWordlistCheckpoints.Last();

	if ( tWord.m_iWordID<pStart->m_iWordID )
		return false;

	bool bLastChunk = false;
	if ( tWord.m_iWordID>=pEnd->m_iWordID )
	{
		bLastChunk = true;
		uWordlistOffset = pEnd->m_iWordlistOffset;
	} else
	{
		while ( pEnd-pStart>1 )
		{
			const CSphWordlistCheckpoint * pMid = pStart + (pEnd-pStart)/2;

			if ( tWord.m_iWordID==pMid->m_iWordID )
			{
				pStart = pEnd = pMid;
				break;

			} else if ( tWord.m_iWordID<pMid->m_iWordID )
			{
				pEnd = pMid;

			} else
			{
				pStart = pMid;
			}
		}

		assert ( pStart>=&pIndex->m_dWordlistCheckpoints[0] );
		assert ( pStart<&pIndex->m_dWordlistCheckpoints.Last() );
		assert ( tWord.m_iWordID>=pStart->m_iWordID && tWord.m_iWordID<pStart[1].m_iWordID );
		uWordlistOffset = pStart->m_iWordlistOffset;
	}

	// decode wordlist chunk
	const BYTE * pBuf = NULL;

	if ( pIndex->m_bPreloadWordlist )
	{
		assert ( uWordlistOffset>0 && uWordlistOffset<=(SphOffset_t)UINT_MAX );
		pBuf = &pIndex->m_pWordlist[(DWORD)uWordlistOffset];
	} else
	{
		assert ( pIndex->m_pWordlistChunkBuf );
		SphOffset_t iChunkLength = 0;

		// not the end?
		if ( !bLastChunk && pStart < &pIndex->m_dWordlistCheckpoints.Last() )
			iChunkLength = pStart[1].m_iWordlistOffset - uWordlistOffset;
		else
			iChunkLength = pIndex->m_iWordlistSize - uWordlistOffset;

		if ( sphPread ( m_tWordlist.GetFD (), pIndex->m_pWordlistChunkBuf, (size_t)iChunkLength, uWordlistOffset )!=iChunkLength )
			return false;

		pBuf = pIndex->m_pWordlistChunkBuf;
	}

	assert ( pBuf );

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
			const ESphHitless eMode = pIndex->m_tSettings.m_eHitless;;
			tWord.m_iDocs = eMode==SPH_HITLESS_SOME ? ( iDocs & 0x7FFFFFFF ) : iDocs;
			tWord.m_iHits = iHits;
			tWord.m_bHasHitlist =
				( eMode==SPH_HITLESS_NONE ) ||
				( eMode==SPH_HITLESS_SOME && !( iDocs & 0x80000000 ) );

			if ( m_bSetupReaders )
			{
				// unpack next word ID and offset delta (ie. hitlist length)
				sphUnzipWordid ( pBuf ); // might be 0 at checkpoint
				SphOffset_t iDoclistLen = sphUnzipOffset ( pBuf );

				tWord.m_rdDoclist.SetBuffers ( g_iReadBuffer, g_iReadUnhinted );
				tWord.m_rdDoclist.SetFile ( m_tDoclist );
				tWord.m_rdDoclist.SeekTo ( iDoclistOffset, (int)iDoclistLen );

				tWord.m_rdHitlist.SetBuffers ( g_iReadBuffer, g_iReadUnhinted );
				tWord.m_rdHitlist.SetFile ( m_tHitlist );
			}
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::Lock ()
{
	CSphString sName = GetIndexFileName("spl");
	sphLogDebug ( "Locking the index via file %s", sName.cstr() );

	if ( m_iLockFD<0 )
	{
		m_iLockFD = ::open ( sName.cstr(), SPH_O_NEW, 0644 );
		if ( m_iLockFD<0 )
		{
			m_sLastError.SetSprintf ( "failed to open %s: %s", sName.cstr(), strerror(errno) );
			sphLogDebug ( "failed to open %s: %s", sName.cstr(), strerror(errno) );
			return false;
		}
	}

	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock %s: %s", sName.cstr(), strerror(errno) );
		::close ( m_iLockFD );
		m_iLockFD = -1;
		return false;
	}
	sphLogDebug ( "lock %s success", sName.cstr() );
	return true;
}


void CSphIndex_VLN::Unlock()
{
	CSphString sName = GetIndexFileName("spl");
	sphLogDebug ( "Unlocking the index (lock %s)", sName.cstr() );
	if ( m_iLockFD>=0 )
	{
		sphLogDebug ( "File ID ok, closing lock FD %d, unlinking %s", m_iLockFD, sName.cstr() );
		::close ( m_iLockFD );
		::unlink ( sName.cstr() );
		m_iLockFD = -1;
	}
}


bool CSphIndex_VLN::Mlock ()
{
	bool bRes = true;
	bRes &= m_pDocinfo.Mlock ( "docinfo", m_sLastError );

	if ( m_bPreloadWordlist )
		bRes &= m_pWordlist.Mlock ( "wordlist", m_sLastError );

	bRes &= m_pMva.Mlock ( "mva", m_sLastError );
	bRes &= m_pStrings.Mlock ( "strings", m_sLastError );
	return bRes;
}


void CSphIndex_VLN::Dealloc ()
{
	if ( !m_bPreallocated )
		return;

	m_tDoclistFile.Close ();
	m_tHitlistFile.Close ();
	m_tWordlistFile.Close ();
	m_pDocinfo.Reset ();
	m_pDocinfoHash.Reset ();
	m_pWordlist.Reset ();
	m_pMva.Reset ();
	m_pStrings.Reset ();
	m_pKillList.Reset ();
	m_dWordlistCheckpoints.Reset ();

	m_uDocinfo = 0;
	m_tSettings.m_eDocinfo = SPH_DOCINFO_NONE;
	m_uAttrsStatus = 0;

	m_bPreallocated = false;
	SafeDelete ( m_pTokenizer );
	SafeDelete ( m_pDict );
	SafeDeleteArray ( m_pWordlistChunkBuf );

	if ( m_iIndexTag>=0 && g_pMvaArena )
		g_MvaArena.TaggedFreeTag ( m_iIndexTag );
	m_iIndexTag = -1;

#ifndef NDEBUG
	m_bPreread.Reset ();
#endif
}


bool CSphIndex_VLN::LoadHeader ( const char * sHeaderName, CSphString & sWarning )
{
	const int MAX_HEADER_SIZE = 32768;
	CSphAutoArray<BYTE> dCacheInfo ( MAX_HEADER_SIZE );

	CSphAutoreader rdInfo ( dCacheInfo, MAX_HEADER_SIZE ); // to avoid mallocs
	if ( !rdInfo.Open ( sHeaderName, m_sLastError ) )
		return false;

	// version
	DWORD uHeader = rdInfo.GetDword ();
	if ( uHeader!=INDEX_MAGIC_HEADER )
	{
		m_sLastError.SetSprintf ( "%s is invalid header file (too old index version?)", sHeaderName );
		return false;
	}

	m_uVersion = rdInfo.GetDword();
	if ( m_uVersion==0 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sHeaderName, m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	// bits
	m_bUse64 = false;
	if ( m_uVersion>=2 )
		m_bUse64 = ( rdInfo.GetDword ()!=0 );

	// docinfo
	m_tSettings.m_eDocinfo = (ESphDocinfo) rdInfo.GetDword();

	// schema
	m_tSchema.Reset ();

	m_tSchema.m_dFields.Resize ( rdInfo.GetDword() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		ReadSchemaColumn ( rdInfo, m_tSchema.m_dFields[i] );

	int iNumAttrs = rdInfo.GetDword();
	bool bDynamic = ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE ); // inline attributes need be dynamic in searching time too

	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		m_tSchema.AddAttr ( tCol, bDynamic );

		// warn on dups
		for ( int k = 0; k < m_tSchema.GetAttrsCount() - 1; k++ )
			if ( m_tSchema.GetAttr(k).m_sName==tCol.m_sName )
				sWarning.SetSprintf ( "duplicate attribute name: %s", tCol.m_sName.cstr() );
	}

	// min doc
	m_tMin.Reset ( m_tSchema.GetRowSize() );
	if ( m_uVersion>=2 )
		m_tMin.m_iDocID = (SphDocID_t) rdInfo.GetOffset (); // v2+; losing high bits when !USE_64 is intentional, check is performed on bUse64 above
	else
		m_tMin.m_iDocID = rdInfo.GetDword(); // v1
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
		rdInfo.GetBytes ( m_tMin.m_pDynamic, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );

	// wordlist checkpoints
	rdInfo.GetBytes ( &m_iCheckpointsPos, sizeof(SphOffset_t) );
	m_dWordlistCheckpoints.Resize ( rdInfo.GetDword() );

	// index stats
	m_tStats.m_iTotalDocuments = rdInfo.GetDword ();
	m_tStats.m_iTotalBytes = rdInfo.GetOffset ();

	LoadSettings ( rdInfo );

	if ( m_uVersion>=9 )
	{
		// tokenizer stuff
		CSphTokenizerSettings tSettings;
		LoadTokenizerSettings ( rdInfo, tSettings, m_uVersion, sWarning );
		ISphTokenizer * pTokenizer = ISphTokenizer::Create ( tSettings, m_sLastError );
		if ( !pTokenizer )
			return false;

		// dictionary stuff
		CSphDictSettings tDictSettings;
		LoadDictionarySettings ( rdInfo, tDictSettings, m_uVersion, sWarning );
		CSphDict * pDict = sphCreateDictionaryCRC ( tDictSettings, pTokenizer, m_sLastError );
		if ( !pDict )
			return false;

		SetDictionary ( pDict );

		ISphTokenizer * pTokenFilter = ISphTokenizer::CreateTokenFilter ( pTokenizer, pDict->GetMultiWordforms () );
		SetTokenizer ( pTokenFilter ? pTokenFilter : pTokenizer );
	}

	if ( m_uVersion>=10 )
		m_iKillListSize = rdInfo.GetDword ();

	if ( m_uVersion>=20 )
		m_uMinMaxIndex = rdInfo.GetDword ();

	if ( rdInfo.GetErrorFlag() )
		m_sLastError.SetSprintf ( "%s: failed to parse header (unexpected eof)", sHeaderName );

	return !rdInfo.GetErrorFlag();
}


void CSphIndex_VLN::DebugDumpHeader ( FILE * fp, const char * sHeaderName )
{
	CSphString sWarning;
	if ( !LoadHeader ( sHeaderName, sWarning ) )
	{
		fprintf ( fp, "FATAL: failed to load header: %s.\n", m_sLastError.cstr() );
		return;
	}

	if ( !sWarning.IsEmpty () )
		fprintf ( fp, "WARNING: %s\n", sWarning.cstr () );

	fprintf ( fp, "version: %d\n",			m_uVersion );
	fprintf ( fp, "idbits: %d\n",			m_bUse64 ? 64 : 32 );
	fprintf ( fp, "docinfo: " );
	switch ( m_tSettings.m_eDocinfo )
	{
		case SPH_DOCINFO_NONE:		fprintf ( fp, "none\n" ); break;
		case SPH_DOCINFO_INLINE:	fprintf ( fp, "inline\n" ); break;
		case SPH_DOCINFO_EXTERN:	fprintf ( fp, "extern\n" ); break;
		default:					fprintf ( fp, "unknown (value=%d)\n", m_tSettings.m_eDocinfo ); break;
	}

	fprintf ( fp, "fields: %d\n", m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		fprintf ( fp, "  field %d: %s\n", i, m_tSchema.m_dFields[i].m_sName.cstr() );

	fprintf ( fp, "attrs: %d\n", m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		fprintf ( fp, "  attr %d: %s, ", i, tAttr.m_sName.cstr() );
		switch ( tAttr.m_eAttrType )
		{
			case SPH_ATTR_INTEGER:						fprintf ( fp, "uint, bits %d", tAttr.m_tLocator.m_iBitCount ); break;
			case SPH_ATTR_TIMESTAMP:					fprintf ( fp, "timestamp" ); break;
			case SPH_ATTR_ORDINAL:						fprintf ( fp, "ordinal" ); break;
			case SPH_ATTR_BOOL:							fprintf ( fp, "boolean" ); break;
			case SPH_ATTR_FLOAT:						fprintf ( fp, "float" ); break;
			case SPH_ATTR_BIGINT:						fprintf ( fp, "bigint" ); break;
			case SPH_ATTR_MULTI | SPH_ATTR_INTEGER:		fprintf ( fp, "mva" ); break;
			default:									fprintf ( fp, "unknown-%d, bits %d", tAttr.m_eAttrType, tAttr.m_tLocator.m_iBitCount ); break;
		}
		fprintf ( fp, ", bitoff %d\n", tAttr.m_tLocator.m_iBitOffset );
	}

	// skipped min doc, wordlist checkpoints
	fprintf ( fp, "total-documents: %d\n", m_tStats.m_iTotalDocuments );
	fprintf ( fp, "total-bytes: "INT64_FMT"\n", int64_t(m_tStats.m_iTotalBytes) );

	fprintf ( fp, "min-prefix-len: %d\n", m_tSettings.m_iMinPrefixLen );
	fprintf ( fp, "min-infix-len: %d\n", m_tSettings.m_iMinInfixLen );
	fprintf ( fp, "exact-words: %d\n", m_tSettings.m_bIndexExactWords ? 1 : 0 );
	fprintf ( fp, "html-strip: %d\n", m_tSettings.m_bHtmlStrip ? 1 : 0 );
	fprintf ( fp, "html-index-attrs: %s\n", m_tSettings.m_sHtmlIndexAttrs.cstr () );
	fprintf ( fp, "html-remove-elements: %s\n", m_tSettings.m_sHtmlRemoveElements.cstr () );

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
	}

	if ( m_pDict )
	{
		const CSphDictSettings & tSettings = m_pDict->GetSettings ();
		fprintf ( fp, "dictionary-morphology: %s\n", tSettings.m_sMorphology.cstr () );
		fprintf ( fp, "dictionary-stopwords: %s\n", tSettings.m_sStopwords.cstr () );
		fprintf ( fp, "dictionary-wordforms: %s\n", tSettings.m_sWordforms.cstr () );
		fprintf ( fp, "min-stemming-len: %d\n", tSettings.m_iMinStemmingLen );
	}

	fprintf ( fp, "killlist-size: %d\n", m_iKillListSize );
}


void CSphIndex_VLN::DebugDumpDocids ( FILE * fp )
{
	if ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		fprintf ( fp, "FATAL: docids dump only supported for docinfo=extern\n" );
		return;
	}

	const int iRowStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	const DWORD uNumMinMaxRow = ( m_uVersion>=20 ) ? ( 2*(1+m_uDocinfoIndex)*iRowStride ) : 0;
	const DWORD uNumRows = (m_pDocinfo.GetNumEntries()-uNumMinMaxRow) / iRowStride; // all 32bit, as we don't expect 2 billion documents per single physical index

	const uint64_t uDocinfoSize = iRowStride*size_t(m_uDocinfo)*sizeof(DWORD);
	const uint64_t uMinmaxSize = uNumMinMaxRow*sizeof(CSphRowitem);

	fprintf ( fp, "docinfo-bytes: docinfo="UINT64_FMT", min-max="UINT64_FMT", total="UINT64_FMT"\n"
		, uDocinfoSize, uMinmaxSize, (uint64_t)m_pDocinfo.GetLength() );
	fprintf ( fp, "docinfo-stride: %d\n", (int)(iRowStride*sizeof(DWORD)) );
	fprintf ( fp, "docinfo-rows: %d\n", uNumRows );

	if ( !m_pDocinfo.GetNumEntries() )
		return;

	DWORD * pDocinfo = m_pDocinfo.GetWritePtr();
	for ( DWORD uRow=0; uRow<uNumRows; uRow++, pDocinfo+=iRowStride )
		printf ( "%u. id=" DOCID_FMT "\n", uRow+1, DOCINFO2ID ( pDocinfo ) );
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
	if ( !bID )
	{
		CSphString sBuf ( sKeyword );

		m_pTokenizer->SetBuffer ( (BYTE*)sBuf.cstr(), strlen ( sBuf.cstr() ) );
		BYTE * sTok = m_pTokenizer->GetToken();

		if ( !sTok )
			sphDie ( "keyword=%s, no token (too short?)", sKeyword );

		uWordID = m_pDict->GetWordID ( sTok );
		if ( !uWordID )
			sphDie ( "keyword=%s, tok=%s, no wordid (stopped?)", sKeyword, sTok );

		fprintf ( fp, "keyword=%s, tok=%s, wordid="UINT64_FMT"\n", sKeyword, sTok, uint64_t(uWordID) );

	} else
	{
		uWordID = (SphWordID_t) strtoull ( sKeyword, NULL, 10 );
		if ( !uWordID )
			sphDie ( "failed to convert keyword=%d to id (must be integer)", sKeyword );

		fprintf ( fp, "wordid="UINT64_FMT"\n", uint64_t(uWordID) );
	}

	// open files
	CSphAutofile tDoclist, tHitlist, tWordlist;
	if ( tDoclist.Open ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError ) < 0 )
		sphDie ( "failed to open doclist: %s", m_sLastError.cstr() );

	if ( tHitlist.Open ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError ) < 0 )
		sphDie ( "failed to open hitlist: %s", m_sLastError.cstr() );

	if ( tWordlist.Open ( GetIndexFileName ( "spi" ), SPH_O_READ, m_sLastError ) < 0 )
		sphDie ( "failed to open wordlist: %s", m_sLastError.cstr() );

	// aim
	DiskIndexQwordSetup_c tTermSetup ( tDoclist, tHitlist, tWordlist );
	tTermSetup.m_pDict = m_pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	tTermSetup.m_tMin.Clone ( m_tMin, m_tSchema.GetRowSize() );
	tTermSetup.m_bSetupReaders = true;

	Qword tKeyword;
	tKeyword.m_tDoc.m_iDocID = m_tMin.m_iDocID;
	tKeyword.m_iWordID = uWordID;
	if ( !tTermSetup.QwordSetup ( &tKeyword ) )
		sphDie ( "failed to setup keyword" );

	int iSize = m_tSchema.GetRowSize();
	CSphVector<CSphRowitem> dAttrs ( iSize );

	// press play on tape
	for ( ;; )
	{
		tKeyword.GetNextDoc ( iSize ? &dAttrs[0] : NULL );
		if ( !tKeyword.m_tDoc.m_iDocID )
			break;
		tKeyword.SeekHitlist ( tKeyword.m_iHitlistPos );

		int iHits = 0;
		if ( tKeyword.m_bHasHitlist )
			while ( DWORD uHit = tKeyword.GetNextHit() )
			{
				fprintf ( fp, "doc="DOCID_FMT", hit=0x%08x\n", tKeyword.m_tDoc.m_iDocID, uHit );
				iHits++;
			}

		if ( !iHits )
		{
			uint64_t uOff = tKeyword.m_iHitlistPos;
			fprintf ( fp, "doc="DOCID_FMT", NO HITS, inline=%d, off="UINT64_FMT"\n",
				tKeyword.m_tDoc.m_iDocID, (int)(uOff>>63), (uOff<<1)>>1 );
		}
	}
}


bool CSphIndex_VLN::Prealloc ( bool bMlock, CSphString & sWarning )
{
	MEMORY ( SPH_MEM_IDX_DISK );

	// reset
	Dealloc ();

	// always keep preread flag
	if ( m_bPreread.IsEmpty() )
	{
		if ( !m_bPreread.Alloc ( 1, m_sLastError, sWarning ) )
			return false;
	}
	m_bPreread.GetWritePtr()[0] = 0;

	// set new locking flag
	m_pDocinfo.SetMlock ( bMlock );
	m_pWordlist.SetMlock ( bMlock );
	m_pMva.SetMlock ( bMlock );
	m_pStrings.SetMlock ( bMlock );
	m_pKillList.SetMlock ( bMlock );

	// preload schema
	if ( !LoadHeader ( GetIndexFileName("sph").cstr(), sWarning ) )
		return false;

	if ( m_bUse64!=USE_64BIT )
	{
		m_sLastError.SetSprintf ( "'%s' is id%d, and this binary is id%d",
			GetIndexFileName("sph").cstr(),
			m_bUse64 ? 64 : 32, USE_64BIT ? 64 : 32 );
		return false;
	}

	// verify that data files are readable
	if ( !sphIsReadable ( GetIndexFileName("spd").cstr(), &m_sLastError ) )
		return false;

	if ( m_uVersion>=3 && !sphIsReadable ( GetIndexFileName("spp").cstr(), &m_sLastError ) )
		return false;

	/////////////////////
	// prealloc docinfos
	/////////////////////

	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && m_tStats.m_iTotalDocuments )
	{
		/////////////
		// attr data
		/////////////

		int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		int iEntrySize = sizeof(DWORD)*iStride;

		CSphAutofile tDocinfo ( GetIndexFileName("spa"), SPH_O_READ, m_sLastError );
		if ( tDocinfo.GetFD()<0 )
			return false;

		DWORD iDocinfoSize = DWORD ( tDocinfo.GetSize ( iEntrySize, true, m_sLastError )
			/ sizeof(DWORD) );
		if ( iDocinfoSize<0 )
			return false;

		DWORD iRealDocinfoSize = m_uMinMaxIndex ? m_uMinMaxIndex : iDocinfoSize;

		// intentionally losing data; we don't support more than 4B documents per instance yet
		m_uDocinfo = (DWORD)( iRealDocinfoSize / iStride );
		if ( iRealDocinfoSize!=m_uDocinfo*iStride )
		{
			m_sLastError.SetSprintf ( "docinfo size check mismatch (4B document limit hit?)" );
			return false;
		}

		if ( m_uVersion < 20 )
		{
			m_uDocinfoIndex = ( m_uDocinfo+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;

			// prealloc docinfo
			if ( !m_pDocinfo.Alloc ( iDocinfoSize + 2*(1+m_uDocinfoIndex)*iStride, m_sLastError, sWarning ) )
				return false;

			m_pDocinfoIndex = const_cast < DWORD * > ( &m_pDocinfo [ iDocinfoSize ] );
		} else
		{
			if ( iDocinfoSize < iRealDocinfoSize )
			{
				m_sLastError.SetSprintf ( "precomputed chunk size check mismatch" );
				return false;
			}

			// prealloc docinfo
			if ( !m_pDocinfo.Alloc ( iDocinfoSize, m_sLastError, sWarning ) )
				return false;

			m_uDocinfoIndex = ( ( iDocinfoSize - iRealDocinfoSize ) / iStride / 2 ) - 1;

#if PARANOID
			DWORD uDocinfoIndex = ( m_uDocinfo+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;
			assert ( uDocinfoIndex==m_uDocinfoIndex );
#endif

			m_pDocinfoIndex = const_cast < DWORD * > ( &m_pDocinfo [ m_uMinMaxIndex ] );
		}

		// prealloc docinfo hash but only if docinfo is big enough (in other words if hash is 8x+ less in size)
		if ( m_pDocinfoHash.IsEmpty() && m_pDocinfo.GetLength() > ( 32 << DOCINFO_HASH_BITS ) )
			if ( !m_pDocinfoHash.Alloc ( ( 1 << DOCINFO_HASH_BITS )+4, m_sLastError, sWarning ) )
				return false;

		////////////
		// MVA data
		////////////

		if ( m_uVersion>=4 )
		{
			// if index is v4, .spm must always exist, even though length could be 0
			CSphAutofile fdMva ( GetIndexFileName("spm"), SPH_O_READ, m_sLastError );
			if ( fdMva.GetFD()<0 )
				return false;

			SphOffset_t iMvaSize = fdMva.GetSize ( 0, true, m_sLastError );
			if ( iMvaSize<0 )
				return false;

			// prealloc
			if ( iMvaSize>0 )
				if ( !m_pMva.Alloc ( DWORD(iMvaSize/sizeof(DWORD)), m_sLastError, sWarning ) )
					return false;
		}

		///////////////
		// string data
		///////////////

		if ( m_uVersion>=17 )
		{
			CSphAutofile fdStrings ( GetIndexFileName("sps"), SPH_O_READ, m_sLastError );
			if ( fdStrings.GetFD()<0 )
				return false;

			SphOffset_t iStringsSize = fdStrings.GetSize ( 0, true, m_sLastError );
			if ( iStringsSize<0 )
				return false;

			// prealloc
			if ( iStringsSize>0 )
				if ( !m_pStrings.Alloc ( DWORD(iStringsSize), m_sLastError, sWarning ) )
					return false;
		}
	}

	/////////////////////
	// prealloc wordlist
	/////////////////////

	// try to open wordlist file in all cases
	CSphAutofile tWordlist ( GetIndexFileName("spi"), SPH_O_READ, m_sLastError );
	if ( tWordlist.GetFD()<0 )
		return false;

	m_iWordlistSize = tWordlist.GetSize ( 1, true, m_sLastError );
	if ( m_iWordlistSize<0 )
		return false;

	// make sure checkpoints are loadable
	// pre-11 indices use different offset type (this is fixed up later during the loading)
	assert ( m_iCheckpointsPos>0 );

	int64_t iCheckpointSize = 2*sizeof(int64_t);
	if ( m_uVersion<14 )
		iCheckpointSize = sizeof(CSphWordlistCheckpoint);
	if ( m_uVersion<11 )
		iCheckpointSize = sizeof(CSphWordlistCheckpoint_v10);

	if ( ( (int64_t)m_iWordlistSize - (int64_t)m_iCheckpointsPos )!=( (int64_t)m_dWordlistCheckpoints.GetLength() * iCheckpointSize ) )
	{
		m_sLastError = "checkpoint segment size mismatch (rebuild the index)";
		return false;
	}

	// prealloc wordlist
	if ( m_bPreloadWordlist )
		if ( !m_pWordlist.Alloc ( DWORD(m_iWordlistSize), m_sLastError, sWarning ) )
			return false;

	// preopen
	if ( m_bKeepFilesOpen )
	{
		if ( m_tDoclistFile.Open ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError ) < 0 )
			return false;

		if ( m_tHitlistFile.Open ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError ) < 0 )
			return false;

		if ( !m_bPreloadWordlist && m_tWordlistFile.Open ( GetIndexFileName("spi"), SPH_O_READ, m_sLastError ) < 0 )
			return false;
	}

	// prealloc killlist
	if ( m_uVersion>=10 )
	{
		CSphAutofile fdKillList ( GetIndexFileName("spk"), SPH_O_READ, m_sLastError );
		if ( fdKillList.GetFD()<0 )
			return false;

		SphOffset_t iSize = fdKillList.GetSize ( 0, true, m_sLastError );
		if ( iSize<0 )
			return false;

		if ( iSize!=(SphOffset_t)( m_iKillListSize*sizeof(SphAttr_t) ) )
		{
			m_sLastError.SetSprintf ( "header killlist size (%d) does not match with spk file size (%d)", m_iKillListSize * sizeof(SphAttr_t), iSize );
			return false;
		}

		// prealloc
		if ( iSize>0 && !m_pKillList.Alloc ( m_iKillListSize, m_sLastError, sWarning ) )
			return false;
	}

	// preload checkpoints (must be done here as they are not shared)
	assert ( m_iCheckpointsPos>0 );
	CSphReader tCheckpointReader;
	tCheckpointReader.SetFile ( tWordlist );
	tCheckpointReader.SeekTo ( m_iCheckpointsPos, READ_NO_SIZE_HINT );

	if ( m_uVersion>=14 )
	{
		// read v.14 checkpoints
		ARRAY_FOREACH ( i, m_dWordlistCheckpoints )
		{
			m_dWordlistCheckpoints[i].m_iWordID = (SphWordID_t) tCheckpointReader.GetOffset();
			m_dWordlistCheckpoints[i].m_iWordlistOffset = tCheckpointReader.GetOffset();
		}
	} else if ( m_uVersion>=11 )
	{
		// convert v.11 checkpoints
		CSphWordlistCheckpoint tCheckpoint;
		ARRAY_FOREACH ( i, m_dWordlistCheckpoints )
		{
			tCheckpointReader.GetBytes ( &tCheckpoint, sizeof(tCheckpoint) );
			m_dWordlistCheckpoints[i] = tCheckpoint;
		}
	} else
	{
		// convert v.10 checkpoints
		CSphWordlistCheckpoint_v10 tCheckpoint;
		ARRAY_FOREACH ( i, m_dWordlistCheckpoints )
		{
			tCheckpointReader.GetBytes ( &tCheckpoint, sizeof(tCheckpoint) );
			m_dWordlistCheckpoints[i].m_iWordID = tCheckpoint.m_iWordID;
			m_dWordlistCheckpoints[i].m_iWordlistOffset = tCheckpoint.m_iWordlistOffset;
		}
	}

	if ( tCheckpointReader.GetErrorFlag() )
	{
		m_sLastError.SetSprintf ( "failed to read %s: %s", GetIndexFileName("spi").cstr(), tCheckpointReader.GetErrorMessage().cstr () );
		return false;
	}

	SphOffset_t uMaxChunk = 0;
	ARRAY_FOREACH ( i, m_dWordlistCheckpoints )
	{
		SphOffset_t uNextOffset = ( i+1 )==m_dWordlistCheckpoints.GetLength()
			? m_iWordlistSize
			: m_dWordlistCheckpoints[i+1].m_iWordlistOffset;
		uMaxChunk = Max ( uMaxChunk, uNextOffset - m_dWordlistCheckpoints[i].m_iWordlistOffset );
	}
	m_pWordlistChunkBuf = new BYTE [ (size_t)uMaxChunk ];

	// all done
	m_bPreallocated = true;
	m_iIndexTag = ++m_iIndexTagSeq;
	return true;
}


template < typename T > bool CSphIndex_VLN::PrereadSharedBuffer ( CSphSharedBuffer<T> & pBuffer, const char * sExt, size_t uExpected )
{
	if ( !pBuffer.GetLength() )
		return true;

	CSphAutofile fdBuf ( GetIndexFileName(sExt), SPH_O_READ, m_sLastError );
	if ( fdBuf.GetFD()<0 )
		return false;

	fdBuf.SetProgressCallback ( m_pProgress, &m_tProgress );
	if ( uExpected==0 )
		uExpected = size_t ( pBuffer.GetLength() );
	return fdBuf.Read ( pBuffer.GetWritePtr(), uExpected, m_sLastError );
}


bool CSphIndex_VLN::Preread ()
{
	MEMORY ( SPH_MEM_IDX_DISK );

	sphLogDebug ( "CSphIndex_VLN::Preread invoked" );
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

	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_PREREAD;
	m_tProgress.m_iBytes = 0;
	m_tProgress.m_iBytesTotal = m_pDocinfo.GetLength() + m_pMva.GetLength() + m_pStrings.GetLength() + m_pKillList.GetLength();
	if ( m_bPreloadWordlist )
		m_tProgress.m_iBytesTotal += m_pWordlist.GetLength();

	sphLogDebug ( "Prereading .spa" );
	if ( !PrereadSharedBuffer ( m_pDocinfo, "spa",
		( m_uVersion<20 )? m_uDocinfo * ( DOCINFO_IDSIZE + m_tSchema.GetRowSize() ) * sizeof(DWORD) : 0 ) )
		return false;

	sphLogDebug ( "Prereading .spm" );
	if ( !PrereadSharedBuffer ( m_pMva, "spm" ) )
		return false;

	sphLogDebug ( "Prereading .sps" );
	if ( !PrereadSharedBuffer ( m_pStrings, "sps" ) )
		return false;

	sphLogDebug ( "Prereading .spk" );
	if ( !PrereadSharedBuffer ( m_pKillList, "spk" ) )
		return false;

#if PARANOID
	for ( int i = 1; i < (int)m_iKillListSize; i++ )
		assert ( m_pKillList[i-1] < m_pKillList[i] );
#endif

	// preload wordlist
	// FIXME! OPTIMIZE! can skip checkpoints
	if ( m_bPreloadWordlist )
	{
		sphLogDebug ( "Prereading .spi" );
		if ( !PrereadSharedBuffer ( m_pWordlist, "spi" ) )
			return false;
	}

	if ( m_pProgress )
		m_pProgress ( &m_tProgress, true );

	//////////////////////
	// precalc everything
	//////////////////////

	// build attributes hash
	if ( m_pDocinfo.GetLength() && m_pDocinfoHash.GetLength() )
	{
		int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		SphDocID_t uFirst = DOCINFO2ID ( &m_pDocinfo[0] );
		SphDocID_t uRange = DOCINFO2ID ( &m_pDocinfo[(m_uDocinfo-1)*iStride] ) - uFirst;
		DWORD iShift = 0;
		while ( uRange>=( 1 << DOCINFO_HASH_BITS ) )
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
	if ( m_uVersion < 20 && !PrecomputeMinMax() )
		return false;

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
	sphLogDebug ( "Preread successfully finished" );
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

	const int EXT_COUNT = 9;
	const char * sExts[EXT_COUNT] = { "spa", "spd", "sph", "spi", "spl", "spm", "spp", "spk", "sps" };
	DWORD uMask = 0;

	int iExt;
	for ( iExt=0; iExt<EXT_COUNT; iExt++ )
	{
		const char * sExt = sExts[iExt];
		if ( !strcmp ( sExt, "spp" ) && m_uVersion<3 ) // .spp files are v3+
			continue;
		if ( !strcmp ( sExt, "spm" ) && m_uVersion<4 ) // .spm files are v4+
			continue;
		if ( !strcmp ( sExt, "spk" ) && m_uVersion<10 ) // .spk files are v10+
			continue;
		if ( !strcmp ( sExt, "sps" ) && m_uVersion<17 ) // .spk files are v17+
			continue;

#if !USE_WINDOWS
		if ( !strcmp ( sExt, "spl" ) && m_iLockFD<0 ) // .spl files are locks
			continue;
#else
		if ( !strcmp ( sExt, "spl" ) )
		{
			if ( m_iLockFD>=0 )
			{
				::close ( m_iLockFD );
				::unlink ( GetIndexFileName("spl").cstr() );
				sphLogDebug ( "lock %s unlinked, file with ID %d closed", GetIndexFileName("spl").cstr(), m_iLockFD );
				m_iLockFD = -1;
			}
			continue;
		}
#endif

		snprintf ( sFrom, sizeof(sFrom), "%s.%s", m_sFilename.cstr(), sExt );
		snprintf ( sTo, sizeof(sTo), "%s.%s", sNewBase, sExt );

#if USE_WINDOWS
		::unlink ( sTo );
		sphLogDebug ( "%s unlinked", sTo );
#endif

		if ( ::rename ( sFrom, sTo ) )
		{
			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFrom, sTo, strerror(errno) );
			// this is no reason to fail if spl is missing, since it is only lock and no data.
			if ( strcmp ( sExt, "spl" ) )
				break;
		}
		uMask |= ( 1UL << iExt );
	}

	// are we good?
	if ( iExt==EXT_COUNT )
	{
		SetBase ( sNewBase );
		sphLogDebug ( "Base set to %s", sNewBase );
		return true;
	}

	// if there were errors, rollback
	for ( iExt=0; iExt<EXT_COUNT; iExt++ )
	{
		if (!( uMask & ( 1UL << iExt ) ))
			continue;

		const char * sExt = sExts[iExt];
		snprintf ( sFrom, sizeof(sFrom), "%s.%s", sNewBase, sExt );
		snprintf ( sTo, sizeof(sTo), "%s.%s", m_sFilename.cstr(), sExt );
		if ( ::rename ( sFrom, sTo ) )
		{
			sphLogDebug ( "Rollback failure when renaming %s to %s", sFrom, sTo );
			// !COMMIT should handle rollback failures somehow
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryContext::CSphQueryContext ()
{
	m_iWeights = 0;
	m_bLookupFilter = false;
	m_bLookupSort = false;
	m_pFilter = false;
	m_pWeightFilter = false;
	m_pIndexData = NULL;
}

CSphQueryContext::~CSphQueryContext ()
{
	SafeDelete ( m_pFilter );
	SafeDelete ( m_pWeightFilter );
}

void CSphQueryContext::BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema )
{
	const int MIN_WEIGHT = 1;

	// defaults
	m_iWeights = Min ( tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );
	for ( int i=0; i<m_iWeights; i++ )
		m_dWeights[i] = MIN_WEIGHT;

	// name-bound weights
	if ( pQuery->m_dFieldWeights.GetLength() )
	{
		ARRAY_FOREACH ( i, pQuery->m_dFieldWeights )
		{
			int j = tSchema.GetFieldIndex ( pQuery->m_dFieldWeights[i].m_sName.cstr() );
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


bool CSphQueryContext::SetupCalc ( CSphQueryResult * pResult, const CSphSchema & tInSchema, const CSphSchema & tSchema, const DWORD * pMvaPool )
{
	m_dCalcFilter.Resize ( 0 );
	m_dCalcSort.Resize ( 0 );
	m_dCalcFinal.Resize ( 0 );

	// quickly verify that all my real attributes can be stashed there
	if ( tInSchema.GetAttrsCount() < tSchema.GetAttrsCount() )
	{
		pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (incount=%d, mycount=%d)", tInSchema.GetAttrsCount(), tSchema.GetAttrsCount() );
		return false;
	}

	// now match everyone
	for ( int iIn=0; iIn<tInSchema.GetAttrsCount(); iIn++ )
	{
		const CSphColumnInfo & tIn = tInSchema.GetAttr(iIn);
		switch ( tIn.m_eStage )
		{
			case SPH_EVAL_STATIC:
			case SPH_EVAL_OVERRIDE:
			{
				const CSphColumnInfo * pMy = tSchema.GetAttr ( tIn.m_sName.cstr() );
				if ( !pMy )
				{
					pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema attr missing from index-schema (in=%s)",
						sphDumpAttr(tIn).cstr() );
					return false;
				}

				if ( tIn.m_eStage==SPH_EVAL_OVERRIDE )
				{
					// override; check for type/size match and dynamic part
					if ( tIn.m_eAttrType!=pMy->m_eAttrType
						|| tIn.m_tLocator.m_iBitCount!=pMy->m_tLocator.m_iBitCount
						|| !tIn.m_tLocator.m_bDynamic )
					{
						pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema override mismatch (in=%s, my=%s)",
							sphDumpAttr(tIn).cstr(), sphDumpAttr(*pMy).cstr() );
						return false;
					}
				} else
				{
					// static; check for full match
					if (!( tIn==*pMy ))
					{
						pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (in=%s, my=%s)",
							sphDumpAttr(tIn).cstr(), sphDumpAttr(*pMy).cstr() );
						return false;
					}
				}
				break;
			}

			case SPH_EVAL_PREFILTER:
			case SPH_EVAL_PRESORT:
			case SPH_EVAL_FINAL:
			{
				if ( !tIn.m_pExpr.Ptr() )
				{
					pResult->m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema expression missing evaluator (stage=%d, in=%s)",
						(int)tIn.m_eStage, sphDumpAttr(tIn).cstr() );
					return false;
				}

				// an expression that index/searcher should compute
				CalcItem_t tCalc;
				tCalc.m_uType = tIn.m_eAttrType;
				tCalc.m_tLoc = tIn.m_tLocator;
				tCalc.m_pExpr = tIn.m_pExpr.Ptr();
				tCalc.m_pExpr->SetMVAPool ( pMvaPool );

				switch ( tIn.m_eStage )
				{
					case SPH_EVAL_PREFILTER:	m_dCalcFilter.Add ( tCalc ); break;
					case SPH_EVAL_PRESORT:		m_dCalcSort.Add ( tCalc ); break;
					case SPH_EVAL_FINAL:		m_dCalcFinal.Add ( tCalc ); break;
				}
				break;
			}

			case SPH_EVAL_SORTER:
				// sorter tells it will compute itself; so just skip it
				break;

			default:
				pResult->m_sError.SetSprintf ( "INTERNAL ERROR: unhandled eval stage=%d", (int)tIn.m_eStage );
				return false;
		}
	}

	// ok, we can emit matches in this schema (incoming for sorter, outgoing for index/searcher)
	pResult->m_tSchema = tInSchema;
	return true;
}


CSphDict * CSphIndex_VLN::SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, ISphTokenizer & tTokenizer ) const
{
	// setup proper dict
	bool bUseStarDict = false;
	if (
		( m_uVersion>=7 && ( m_tSettings.m_iMinPrefixLen>0 || m_tSettings.m_iMinInfixLen>0 ) && m_bEnableStar ) || // v.7 added mangling to infixes
		( m_uVersion==6 && ( m_tSettings.m_iMinPrefixLen>0 ) && m_bEnableStar ) ) // v.6 added mangling to prefixes
	{
		bUseStarDict = true;
	}

	// no star? just return the original one
	if ( !bUseStarDict )
		return m_pDict;

	// spawn wrapper, and put it in the box
	// wrapper type depends on version; v.8 introduced new mangling rules
	if ( m_uVersion>=8 )
		tContainer = new CSphDictStarV8 ( m_pDict, m_tSettings.m_iMinPrefixLen>0, m_tSettings.m_iMinInfixLen>0 );
	else
		tContainer = new CSphDictStar ( m_pDict );

	CSphRemapRange tStar ( '*', '*', '*' ); // FIXME? check and warn if star was already there
	tTokenizer.AddCaseFolding ( tStar );

	return tContainer.Ptr();
}


CSphDict * CSphIndex_VLN::SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer & tTokenizer ) const
{
	if ( m_uVersion<12 || !m_tSettings.m_bIndexExactWords )
		return pPrevDict;

	tContainer = new CSphDictExact ( pPrevDict );

	CSphRemapRange tStar ( '=', '=', '=' ); // FIXME? check and warn if star was already there
	tTokenizer.AddCaseFolding ( tStar );

	return tContainer.Ptr();
}


void CSphIndex_VLN::LoadSettings ( CSphReader & tReader )
{
	if ( m_uVersion>=8 )
	{
		m_tSettings.m_iMinPrefixLen = tReader.GetDword ();
		m_tSettings.m_iMinInfixLen = tReader.GetDword ();

	} else if ( m_uVersion>=6 )
	{
		bool bPrefixesOnly = ( tReader.GetByte ()!=0 );
		m_tSettings.m_iMinPrefixLen = tReader.GetDword ();
		m_tSettings.m_iMinInfixLen = 0;
		if ( !bPrefixesOnly )
			Swap ( m_tSettings.m_iMinPrefixLen, m_tSettings.m_iMinInfixLen );
	}

	if ( m_uVersion>=9 )
	{
		m_tSettings.m_bHtmlStrip = !!tReader.GetByte ();
		m_tSettings.m_sHtmlIndexAttrs = tReader.GetString ();
		m_tSettings.m_sHtmlRemoveElements = tReader.GetString ();
	} else
		m_bStripperInited = false;

	if ( m_uVersion>=12 )
		m_tSettings.m_bIndexExactWords = !!tReader.GetByte ();

	if ( m_uVersion>=18 )
		m_tSettings.m_eHitless = (ESphHitless)tReader.GetDword();

	if ( m_uVersion>=19 )
		m_tSettings.m_eHitFormat = (ESphHitFormat)tReader.GetDword();
	else // force plain format for old indices
		m_tSettings.m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
}


void CSphIndex_VLN::SaveSettings ( CSphWriter & tWriter )
{
	tWriter.PutDword ( m_tSettings.m_iMinPrefixLen );
	tWriter.PutDword ( m_tSettings.m_iMinInfixLen );
	tWriter.PutByte ( m_tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( m_tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( m_tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( m_tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( m_tSettings.m_eHitless );
	tWriter.PutDword ( m_tSettings.m_eHitFormat );
}


bool CSphIndex_VLN::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const
{
	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, szQuery, bGetStats, sError ) );
	return false;
}


template < class Qword >
bool CSphIndex_VLN::DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const
{
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
	{
		sError = "index not preread";
		return false;
	}

	CSphScopedPtr <CSphAutofile> pDoclist ( NULL );
	CSphScopedPtr <CSphAutofile> pHitlist ( NULL );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( false ) ); // avoid race

	CSphScopedPtr<CSphDict> tDict ( NULL );
	CSphDict * pDict = SetupStarDict ( tDict, *pTokenizer.Ptr() );

	CSphScopedPtr<CSphDict> tDict2 ( NULL );
	pDict = SetupExactDict ( tDict2, pDict, *pTokenizer.Ptr() );

	// prepare for setup
	CSphAutofile tDummy1, tDummy2, tDummy3, tWordlist;

	if ( !m_bKeepFilesOpen )
		if ( tWordlist.Open ( GetIndexFileName ( "spi" ), SPH_O_READ, sError ) < 0 )
			return false;

	DiskIndexQwordSetup_c tTermSetup ( tDummy1, tDummy2, m_bPreloadWordlist ? tDummy3 : ( m_bKeepFilesOpen ? m_tWordlistFile : tWordlist ) );
	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	dKeywords.Resize ( 0 );

	Qword QueryWord;
	CSphString sTokenized;
	BYTE * sWord;
	int nWords = 0;

	CSphString sQbuf ( szQuery );
	pTokenizer->SetBuffer ( (BYTE*)sQbuf.cstr(), strlen(szQuery) );

	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		sTokenized = (const char*)sWord;

		SphWordID_t iWord = pDict->GetWordID ( sWord );
		if ( iWord )
		{
			if ( bGetStats )
			{
				QueryWord.Reset ();
				QueryWord.m_sWord = (const char*)sWord;
				QueryWord.m_iWordID = iWord;
				tTermSetup.QwordSetup ( &QueryWord );
			}

			CSphKeywordInfo & tInfo = dKeywords.Add();
			Swap ( tInfo.m_sTokenized, sTokenized );
			tInfo.m_sNormalized = (const char*)sWord;
			tInfo.m_iDocs = bGetStats ? QueryWord.m_iDocs : 0;
			tInfo.m_iHits = bGetStats ? QueryWord.m_iHits : 0;
			++nWords;
		}
	}

	return true;
}

// fix MSVC 2005 fuckup, template DoGetKeywords() just above somehow resets forScope
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif


bool CSphQueryContext::CreateFilters ( bool bFullscan, const CSphVector<CSphFilterSettings> * pdFilters, const CSphSchema & tSchema, const DWORD * pMvaPool, CSphString & sError )
{
	if ( !pdFilters )
		return true;
	ARRAY_FOREACH ( i, (*pdFilters) )
	{
		const CSphFilterSettings & tFilter = (*pdFilters)[i];
		if ( tFilter.m_sAttrName.IsEmpty() )
			continue;

		if ( bFullscan && tFilter.m_sAttrName=="@weight" )
			continue; // @weight is not avaiable in fullscan mode

		ISphFilter * pFilter = sphCreateFilter ( tFilter, tSchema, pMvaPool, sError );
		if ( !pFilter )
			return false;

		ISphFilter ** pGroup = tFilter.m_sAttrName=="@weight" ? &m_pWeightFilter : &m_pFilter;
		*pGroup = sphJoinFilters ( *pGroup, pFilter );
	}
	return true;
}


bool CSphQueryContext::SetupOverrides ( const CSphQuery * pQuery, CSphQueryResult * pResult, const CSphSchema & tIndexSchema )
{
	m_pOverrides = NULL;
	m_dOverrideIn.Resize ( pQuery->m_dOverrides.GetLength() );
	m_dOverrideOut.Resize ( pQuery->m_dOverrides.GetLength() );

	ARRAY_FOREACH ( i, pQuery->m_dOverrides )
	{
		const char * sAttr = pQuery->m_dOverrides[i].m_sAttr.cstr(); // shortcut
		const CSphColumnInfo * pCol = tIndexSchema.GetAttr ( sAttr );
		if ( !pCol )
		{
			pResult->m_sError.SetSprintf ( "attribute override: unknown attribute name '%s'", sAttr );
			return false;
		}

		if ( pCol->m_eAttrType!=pQuery->m_dOverrides[i].m_uAttrType )
		{
			pResult->m_sError.SetSprintf ( "attribute override: attribute '%s' type mismatch (index=%d, query=%d)",
				sAttr, pCol->m_eAttrType, pQuery->m_dOverrides[i].m_uAttrType );
			return false;
		}

		const CSphColumnInfo * pOutCol = pResult->m_tSchema.GetAttr ( pQuery->m_dOverrides[i].m_sAttr.cstr() );
		if ( !pOutCol )
		{
			pResult->m_sError.SetSprintf ( "attribute override: unknown attribute name '%s' in outgoing schema", sAttr );
			return false;
		}

		m_dOverrideIn[i] = pCol->m_tLocator;
		m_dOverrideOut[i] = pOutCol->m_tLocator;

#ifndef NDEBUG
		// check that the values are actually sorted
		const CSphVector<CSphAttrOverride::IdValuePair_t> & dValues = pQuery->m_dOverrides[i].m_dValues;
		for ( int j=1; j<dValues.GetLength(); j++ )
			assert ( dValues[j-1] < dValues[j] );
#endif
	}

	if ( pQuery->m_dOverrides.GetLength() )
		m_pOverrides = &pQuery->m_dOverrides;
	return true;
}

void sphDoStatsOrder ( const XQNode_t * pNode, CSphQueryResultMeta & tResult )
{
	if ( !pNode )
		return;

	ARRAY_FOREACH ( i, pNode->m_dWords )
		tResult.AddStat ( pNode->m_dWords[i].m_sWord, 0, 0, true );

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		sphDoStatsOrder ( pNode->m_dChildren[i], tResult );
}

static XQNode_t * CloneKeyword ( const XQNode_t * pNode )
{
	assert ( pNode );
	assert ( pNode->m_dWords.GetLength()==1 );

	XQNode_t * pRes = new XQNode_t;
	pRes->m_bFieldSpec = pNode->m_bFieldSpec;
	pRes->m_uFieldMask = pNode->m_uFieldMask;
	pRes->m_iFieldMaxPos = pNode->m_iFieldMaxPos;
	pRes->m_dWords = pNode->m_dWords;
	return pRes;
}


static XQNode_t * ExpandKeyword ( XQNode_t * pNode, const CSphIndexSettings & tSettings )
{
	XQNode_t * pExpand = new XQNode_t;
	pExpand->SetOp ( SPH_QUERY_OR, pNode );

	if ( tSettings.m_iMinInfixLen>0 )
	{
		XQNode_t * pInfix = CloneKeyword ( pNode );
		pInfix->m_dWords[0].m_sWord.SetSprintf ( "*%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pInfix->m_dWords[0].m_uStarPosition = STAR_BOTH;
		pExpand->m_dChildren.Add ( pInfix );
	}

	if ( tSettings.m_bIndexExactWords )
	{
		XQNode_t * pExact = CloneKeyword ( pNode );
		pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		pExpand->m_dChildren.Add ( pExact );
	}

	return pExpand;
}

static XQNode_t * ExpandKeywords ( XQNode_t * pNode, const CSphIndexSettings & tSettings )
{
	// only if expansion makes sense at all
	if ( tSettings.m_iMinInfixLen<=0 && !tSettings.m_bIndexExactWords )
		return pNode;

	// process children for composite nodes
	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			pNode->m_dChildren[i] = ExpandKeywords ( pNode->m_dChildren[i], tSettings );
		return pNode;
	}

	// if that's a phrase/proximity node, create a very special, magic phrase/proximity node
	if ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM )
	{
		assert ( pNode->m_dWords.GetLength()>1 );
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pWord = new XQNode_t;
			pWord->m_dWords.Add ( pNode->m_dWords[i] );
			pNode->m_dChildren.Add ( ExpandKeyword ( pWord, tSettings ) );
			pNode->m_dChildren.Last()->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
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
	if ( tKeyword.m_uStarPosition!=STAR_NONE
		|| tKeyword.m_sWord.Begins("=")
		|| tKeyword.m_sWord.Begins("*")
		|| tKeyword.m_sWord.Ends("*") )
	{
		return pNode;
	}

	// do the expansion
	return ExpandKeyword ( pNode, tSettings );
}

// transform the "one two three"/1 quorum into one|two|three (~40% faster)
static void TransformQuorum ( XQNode_t ** ppNode )
{
	XQNode_t *& pNode = *ppNode;
	if ( pNode->GetOp()!=SPH_QUERY_QUORUM || pNode->m_iOpArg!=1 )
		return;

	assert ( pNode->m_dChildren.GetLength()==0 );
	CSphVector<XQNode_t*> dArgs;
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		XQNode_t * pAnd = new XQNode_t();
		pAnd->m_dWords.Add ( pNode->m_dWords[i] );
		dArgs.Add ( pAnd );
	}
	pNode->m_dWords.Reset();
	pNode->SetOp ( SPH_QUERY_OR, dArgs );
}

struct CmpPSortersByRandom_fn
{
	inline bool IsLess ( const ISphMatchSorter * a, const ISphMatchSorter * b ) const
	{
		assert ( a );
		assert ( b );
		return a->m_bRandomize < b->m_bRandomize;
	}
};


/// one regular query vs many sorters
bool CSphIndex_VLN::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const
{
	assert ( pQuery );

	MEMORY ( SPH_MEM_IDX_DISK_MULTY_QUERY );

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

	// fast path for scans
	if ( pQuery->m_sQuery.IsEmpty() )
		return MultiScan ( pQuery, pResult, iSorters, &dSorters[0], pExtraFilters, iTag );

	ISphTokenizer * pTokenizer = m_pTokenizer->Clone ( false );

	CSphScopedPtr<CSphDict> tDict ( NULL );
	CSphDict * pDict = SetupStarDict ( tDict, *pTokenizer );

	CSphScopedPtr<CSphDict> tDict2 ( NULL );
	pDict = SetupExactDict ( tDict2, pDict, *pTokenizer );

	// parse query
	XQQuery_t tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), pTokenizer, &m_tSchema, pDict ) )
	{
		pResult->m_sError = tParsed.m_sParseError;
		SafeDelete ( pTokenizer );
		return false;
	}

	// fixup stat's order
	sphDoStatsOrder ( tParsed.m_pRoot, *pResult );

	// transform query if needed (quorum transform, keyword expansion, etc.)
	TransformQuorum ( &tParsed.m_pRoot );
	if ( m_bExpandKeywords )
		tParsed.m_pRoot = ExpandKeywords ( tParsed.m_pRoot, m_tSettings );

	// flag common subtrees
	CSphVector<XQNode_t*> dTrees;
	dTrees.Add ( tParsed.m_pRoot );
	int iCommonSubtrees = sphMarkCommonSubtrees ( dTrees );

	CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
	bool bResult = ParsedMultiQuery ( pQuery, pResult, iSorters, &dSorters[0], tParsed.m_pRoot, pDict, pExtraFilters, &tNodeCache, iTag );

	SafeDelete ( pTokenizer );

	return bResult;
}


/// many regular queries with one sorter attached to each query.
/// returns true if at least one query succeeded. The failed queries indicated with pResult->m_iMultiplier==-1
bool CSphIndex_VLN::MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const
{
	// ensure we have multiple queries
	if ( iQueries==1 )
		return MultiQuery ( pQueries, ppResults[0], 1, ppSorters, pExtraFilters, iTag );

	MEMORY ( SPH_MEM_IDX_DISK_MULTY_QUERY_EX );

	assert ( pQueries );
	assert ( ppResults );
	assert ( ppSorters );

	ISphTokenizer * pTokenizer = m_pTokenizer->Clone ( false );

	CSphScopedPtr<CSphDict> tDict ( NULL );
	CSphDict * pDict = SetupStarDict ( tDict, *pTokenizer );

	CSphScopedPtr<CSphDict> tDict2 ( NULL );
	pDict = SetupExactDict ( tDict2, pDict, *pTokenizer );

	CSphVector<XQNode_t*> dTrees;
	dTrees.Reserve ( iQueries );

	bool bResult = false;
	bool bResultScan = false;
	for ( int i=0; i<iQueries; i++ )
	{
		// nothing to do without a sorter
		if ( !ppSorters[i] )
		{
			ppResults[i]->m_iMultiplier = -1; ///< show that this particular query failed
			dTrees.Add ( NULL );
			continue;
		}

		// fast path for scans
		if ( pQueries[i].m_sQuery.IsEmpty() )
		{
			if ( MultiScan ( pQueries + i, ppResults[i], 1, &ppSorters[i], pExtraFilters, iTag ) )
				bResultScan = true;
			else
				ppResults[i]->m_iMultiplier = -1; ///< show that this particular query failed

			dTrees.Add ( NULL );
			continue;
		}

		// parse query
		XQQuery_t tParsed;
		if ( sphParseExtendedQuery ( tParsed, pQueries[i].m_sQuery.cstr(), pTokenizer, &m_tSchema, pDict ) )
		{
			// fixup stat's order
			sphDoStatsOrder ( tParsed.m_pRoot, *ppResults[i] );

			// transform query if needed (quorum transform, keyword expansion, etc.)
			TransformQuorum ( &tParsed.m_pRoot );
			if ( m_bExpandKeywords )
				tParsed.m_pRoot = ExpandKeywords ( tParsed.m_pRoot, m_tSettings );

			dTrees.Add ( tParsed.m_pRoot );
			tParsed.m_pRoot = NULL;
			bResult = true;
		} else
		{
			ppResults[i]->m_sError = tParsed.m_sParseError;
			ppResults[i]->m_iMultiplier = -1;
		}
	}

	// continue only if we have at least one non-failed
	if ( bResult )
	{
		int iCommonSubtrees = sphMarkCommonSubtrees ( dTrees );

		CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
		bResult = false;
		ARRAY_FOREACH ( j, dTrees )
			if ( dTrees[j] && ppSorters[j]
					&& ParsedMultiQuery ( &pQueries[j], ppResults[j], 1, &ppSorters[j], dTrees[j], pDict, pExtraFilters, &tNodeCache, iTag ) )
			{
				bResult = true;
				ppResults[j]->m_iMultiplier = iCommonSubtrees ? iQueries : 1;
			} else
				ppResults[j]->m_iMultiplier = -1;
	}

	ARRAY_FOREACH ( k, dTrees )
		SafeDelete ( dTrees[k] );

	SafeDelete ( pTokenizer );

	return bResult | bResultScan;
}

bool CSphIndex_VLN::ParsedMultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const XQNode_t * pRoot, CSphDict * pDict, const CSphVector<CSphFilterSettings> * pExtraFilters, CSphQueryNodeCache * pNodeCache, int iTag ) const
{
	assert ( pQuery );
	assert ( pResult );
	assert ( ppSorters );
	assert ( !pQuery->m_sQuery.IsEmpty() && pQuery->m_eMode!=SPH_MATCH_FULLSCAN ); // scans must go through MultiScan()

	// start counting
	int64_t tmQueryStart = sphMicroTimer();

	///////////////////
	// setup searching
	///////////////////

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	// non-ready index, empty response!
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
	{
		pResult->m_sError = "index not preread";
		return false;
	}

	// setup calculations and result schema
	CSphQueryContext tCtx;
	if ( !tCtx.SetupCalc ( pResult, ppSorters[0]->GetSchema(), m_tSchema, GetMVAPool() ) )
		return false;

	// open files
	CSphAutofile tDoclist, tHitlist, tWordlist, tDummy;
	if ( !m_bKeepFilesOpen )
	{
		if ( tDoclist.Open ( GetIndexFileName("spd"), SPH_O_READ, pResult->m_sError ) < 0 )
			return false;

		if ( tHitlist.Open ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, pResult->m_sError ) < 0 )
			return false;

		if ( tWordlist.Open ( GetIndexFileName ( "spi" ), SPH_O_READ, pResult->m_sError ) < 0 )
			return false;
	}

	// setup search terms
	DiskIndexQwordSetup_c tTermSetup ( m_bKeepFilesOpen ? m_tDoclistFile : tDoclist,
		m_bKeepFilesOpen ? m_tHitlistFile : tHitlist,
		m_bPreloadWordlist ? tDummy : ( m_bKeepFilesOpen ? m_tWordlistFile : tWordlist ) );

	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	tTermSetup.m_tMin.m_iDocID = m_tMin.m_iDocID;
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
	{
		tTermSetup.m_tMin.Clone ( m_tMin, m_tSchema.GetRowSize() );
		tTermSetup.m_iInlineRowitems = m_tSchema.GetRowSize();
	}
	tTermSetup.m_iDynamicRowitems = pResult->m_tSchema.GetDynamicSize();

	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time
	tTermSetup.m_pWarning = &pResult->m_sWarning;
	tTermSetup.m_bSetupReaders = true;
	tTermSetup.m_pCtx = &tCtx;
	tTermSetup.m_pNodeCache = pNodeCache;

	// bind weights
	tCtx.BindWeights ( pQuery, m_tSchema );

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( pRoot, pQuery->m_eRanker, pResult, tTermSetup ) );
	if ( !pRanker.Ptr() )
		return false;

	// empty index, empty response!
	if ( !m_tStats.m_iTotalDocuments )
		return true;
	assert ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN || !m_pDocinfo.IsEmpty() ); // check that docinfo is preloaded

	// setup filters
	if ( !tCtx.CreateFilters ( pQuery->m_sQuery.IsEmpty(), &pQuery->m_dFilters, pResult->m_tSchema, GetMVAPool(), pResult->m_sError ) )
		return false;
	if ( !tCtx.CreateFilters ( pQuery->m_sQuery.IsEmpty(), pExtraFilters, pResult->m_tSchema, GetMVAPool(), pResult->m_sError ) )
		return false;

	// check if we can early reject the whole index
	if ( tCtx.m_pFilter && m_uDocinfoIndex )
	{
		DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ 2*m_uDocinfoIndex*uStride ] );
		DWORD * pMaxEntry = pMinEntry + uStride;

		if ( !tCtx.m_pFilter->EvalBlock ( pMinEntry, pMaxEntry ) )
			return true;
	}

	// setup lookup
	tCtx.m_bLookupFilter = ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	if ( tCtx.m_dCalcFilter.GetLength() )
		tCtx.m_bLookupFilter = true; // suboptimal in case of attr-independent expressions, but we don't care

	tCtx.m_bLookupSort = false;
	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && !tCtx.m_bLookupFilter )
		for ( int iSorter=0; iSorter<iSorters && !tCtx.m_bLookupSort; iSorter++ )
			if ( ppSorters[iSorter]->UsesAttrs() )
				tCtx.m_bLookupSort = true;
	if ( tCtx.m_dCalcSort.GetLength() )
		tCtx.m_bLookupSort = true; // suboptimal in case of attr-independent expressions, but we don't care

	// setup sorters vs. MVA
	for ( int i=0; i<iSorters; i++ )
		(ppSorters[i])->SetMVAPool ( m_pMva.GetWritePtr() );

	// setup overrides
	if ( !tCtx.SetupOverrides ( pQuery, pResult, m_tSchema ) )
		return false;

	PROFILE_END ( query_init );

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	PROFILE_BEGIN ( query_match );
	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:
		case SPH_MATCH_PHRASE:
		case SPH_MATCH_ANY:
		case SPH_MATCH_EXTENDED:
		case SPH_MATCH_EXTENDED2:
		case SPH_MATCH_BOOLEAN:
			if ( !MatchExtended ( &tCtx, pQuery, iSorters, ppSorters, pRanker.Ptr(), iTag ) )
				return false;
			break;

		default:
			sphDie ( "INTERNAL ERROR: unknown matching mode (mode=%d)", pQuery->m_eMode );
	}
	PROFILE_END ( query_match );

	////////////////////
	// cook result sets
	////////////////////

	// adjust result sets
	for ( int iSorter=0; iSorter<iSorters; iSorter++ )
	{
		ISphMatchSorter * pTop = ppSorters[iSorter];

		// final lookup and/or calc
		bool bFinalLookup = !tCtx.m_bLookupFilter && !tCtx.m_bLookupSort;
		if ( pTop->GetLength() && ( bFinalLookup || tCtx.m_dCalcFinal.GetLength() ) )
		{
			const int iCount = pTop->GetLength ();
			CSphMatch * const pHead = pTop->First();
			CSphMatch * const pTail = pHead + iCount;

			for ( CSphMatch * pCur=pHead; pCur<pTail; pCur++ )
			{
				if ( bFinalLookup )
					CopyDocinfo ( &tCtx, *pCur, FindDocinfo ( pCur->m_iDocID ) );
				tCtx.CalcFinal ( *pCur );
			}
		}

		// mva and string pools ptrs
		pResult->m_pMva = m_pMva.GetWritePtr();
		pResult->m_pStrings = m_pStrings.GetWritePtr();
	}

	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// INDEX CHECKING
//////////////////////////////////////////////////////////////////////////

#define LOC_FAIL(_args) \
	if ( ++iFails<=FAILS_THRESH ) \
	{ \
		fprintf ( fp, "FAILED, " ); \
		fprintf _args; \
		fprintf ( fp, "\n" ); \
		iFailsPrinted++; \
		\
		if ( iFails==FAILS_THRESH ) \
			fprintf ( fp, "(threshold reached; suppressing further output)\n" ); \
	}


int CSphIndex_VLN::DebugCheck ( FILE * fp )
{
	int64_t tmCheck = sphMicroTimer();
	int iFails = 0;
	int iFailsPrinted = 0;
	const int FAILS_THRESH = 100;

	// check if index is ready
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
		LOC_FAIL(( fp, "index not preread" ));

	bool bProgress = isatty ( fileno ( fp ) )!=0;

	//////////////
	// open files
	//////////////

	CSphString sError;
	CSphAutoreader rdDict, rdDocs, rdHits;

	if ( !rdDict.Open ( GetIndexFileName("spi"), sError ) )
		LOC_FAIL(( fp, "unable to open dictionary: %s", sError.cstr() ));

	if ( !rdDocs.Open ( GetIndexFileName("spd"), sError ) )
		LOC_FAIL(( fp, "unable to open doclist: %s", sError.cstr() ));

	if ( !rdHits.Open ( GetIndexFileName("spp"), sError ) )
		LOC_FAIL(( fp, "unable to open hitlist: %s", sError.cstr() ));

	////////////////////
	// check dictionary
	////////////////////

	fprintf ( fp, "checking dictionary...\n" );
	SphWordID_t uWordid = 0;
	SphOffset_t iDoclistOffset = 0;

	int iWordsTotal = 0;
	int iWordsSinceCheckpoint = 0;
	CSphVector<CSphWordlistCheckpoint> dCheckpoints;

	rdDict.SeekTo ( 1, READ_NO_SIZE_HINT );
	for ( ;; )
	{
		// sanity checks
		if ( rdDict.GetPos()>=m_iCheckpointsPos )
		{
			LOC_FAIL(( fp, "reading past checkpoints" ));
			break;
		}

		// store current entry pos (for checkpointing later), read next delta
		SphOffset_t iDictPos = rdDict.GetPos();
		SphWordID_t iDelta = rdDict.UnzipWordid();

		// checkpoint encountered, handle it
		if ( !iDelta )
		{
			rdDict.UnzipOffset();

			if ( rdDict.GetPos()==m_iCheckpointsPos )
				break; // this is the end, my beautiful friend

			if ( iWordsSinceCheckpoint!=WORDLIST_CHECKPOINT )
				LOC_FAIL(( fp, "unexpected checkpoint (pos="INT64_FMT", words=%d)",
					int64_t(iDictPos), iWordsSinceCheckpoint ));

			uWordid = 0;
			iDoclistOffset = 0;
			iWordsSinceCheckpoint = 0;
			continue;
		}

		// finish reading the entire entry
		SphWordID_t uNewWordid = uWordid + iDelta;
		SphOffset_t iNewDoclistOffset = iDoclistOffset + rdDict.UnzipOffset();
		int iDocs = rdDict.UnzipInt();
		int iHits = rdDict.UnzipInt();

		// update stats, add checkpoint
		if ( iWordsSinceCheckpoint==0 )
		{
			dCheckpoints.Add();
			dCheckpoints.Last().m_iWordID = uNewWordid;
			dCheckpoints.Last().m_iWordlistOffset = iDictPos;
		}
		iWordsTotal++;
		iWordsSinceCheckpoint++;

		// do the checks
		if ( uNewWordid<=uWordid )
			LOC_FAIL(( fp, "wordid decreased (pos="INT64_FMT", wordid="UINT64_FMT", previd="UINT64_FMT")",
				(int64_t)iDictPos, (uint64_t)uNewWordid, (uint64_t)uWordid ));

		if ( iNewDoclistOffset<=iDoclistOffset )
			LOC_FAIL(( fp, "doclist offset decreased (pos="INT64_FMT", wordid="UINT64_FMT")",
				(int64_t)iDictPos, (uint64_t)uNewWordid ));

		if ( iDocs<=0 || iHits<=0 || iHits<iDocs )
			LOC_FAIL(( fp, "invalid docs/hits (pos="INT64_FMT", wordid="UINT64_FMT", docs=%d, hits=%d)",
				(int64_t)iDictPos, (uint64_t)uNewWordid, iDocs, iHits ));

		uWordid = uNewWordid;
		iDoclistOffset = iNewDoclistOffset;
	}

	// check the checkpoints
	if ( dCheckpoints.GetLength()!=m_dWordlistCheckpoints.GetLength() )
		LOC_FAIL(( fp, "checkpoint count mismatch (read=%d, calc=%d)",
			m_dWordlistCheckpoints.GetLength(), dCheckpoints.GetLength() ));

	for ( int i=0; i < Min ( dCheckpoints.GetLength(), m_dWordlistCheckpoints.GetLength() ); i++ )
		if ( dCheckpoints[i].m_iWordID!=m_dWordlistCheckpoints[i].m_iWordID
			|| dCheckpoints[i].m_iWordlistOffset!=m_dWordlistCheckpoints[i].m_iWordlistOffset )
	{
		LOC_FAIL(( fp, "checkpoint %d differs (readid="UINT64_FMT", readpos="INT64_FMT", calcid="UINT64_FMT", calcpos="INT64_FMT")",
			i,
			(uint64_t)m_dWordlistCheckpoints[i].m_iWordID,
			(int64_t)m_dWordlistCheckpoints[i].m_iWordlistOffset,
			(uint64_t)dCheckpoints[i].m_iWordID,
			(int64_t)dCheckpoints[i].m_iWordlistOffset ));
	}

	dCheckpoints.Reset ();

	///////////////////////
	// check docs and hits
	///////////////////////

	fprintf ( fp, "checking data...\n" );

	rdDict.SeekTo ( 1, READ_NO_SIZE_HINT );
	rdDocs.SeekTo ( 1, READ_NO_SIZE_HINT );
	rdHits.SeekTo ( 1, READ_NO_SIZE_HINT );

	uWordid = 0;
	iDoclistOffset = 0;

	int iWordsChecked = 0;
	for ( ;; )
	{
		SphWordID_t iDelta = rdDict.UnzipWordid();
		if ( !iDelta )
		{
			rdDict.UnzipOffset();
			if ( rdDict.GetPos()==m_iCheckpointsPos )
				break; // this is the end, my beautiful friend

			uWordid = 0;
			iDoclistOffset = 0;
			continue;
		}

		// finish reading the entire entry
		uWordid += iDelta;
		iDoclistOffset += rdDict.UnzipOffset();
		int iDictDocs = rdDict.UnzipInt();
		int iDictHits = rdDict.UnzipInt();

		// check whether the offset is as expected
		if ( iDoclistOffset!=rdDocs.GetPos() )
		{
			LOC_FAIL(( fp, "unexpected doclist offset (wordid="UINT64_FMT", dictpos="INT64_FMT", doclistpos="INT64_FMT")",
				(uint64_t)uWordid, (int64_t)iDoclistOffset, (int64_t)rdDocs.GetPos() ));
			rdDocs.SeekTo ( iDoclistOffset, READ_NO_SIZE_HINT );
		}

		// create and manually setup doclist reader
		DiskIndexQwordTraits_c * pQword = NULL;
		WITH_QWORD ( this, false, T, pQword = new T() );

		pQword->m_tDoc.Reset ( m_tSchema.GetDynamicSize() );
		pQword->m_iMinID = m_tMin.m_iDocID;
		pQword->m_tDoc.m_iDocID = m_tMin.m_iDocID;
		if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
		{
			pQword->m_iInlineAttrs = m_tSchema.GetDynamicSize();
			pQword->m_pInlineFixup = m_tMin.m_pDynamic;
		} else
		{
			pQword->m_iInlineAttrs = 0;
			pQword->m_pInlineFixup = NULL;
		}
		pQword->m_iDocs = 0;
		pQword->m_iHits = 0;
		pQword->m_rdDoclist.SetFile ( rdDocs.GetFD(), rdDocs.GetFilename().cstr() );
		pQword->m_rdDoclist.SeekTo ( rdDocs.GetPos(), READ_NO_SIZE_HINT );
		pQword->m_rdHitlist.SetFile ( rdHits.GetFD(), rdHits.GetFilename().cstr() );
		pQword->m_rdHitlist.SeekTo ( rdHits.GetPos(), READ_NO_SIZE_HINT );

		CSphRowitem * pInlineStorage = NULL;
		if ( pQword->m_iInlineAttrs )
			pInlineStorage = new CSphRowitem [ pQword->m_iInlineAttrs ];

		// loop the doclist
		SphDocID_t uLastDocid = 0;
		int iDoclistDocs = 0;
		int iDoclistHits = 0;
		int iHitlistHits = 0;

		for ( ;; )
		{
			const CSphMatch & tDoc = pQword->GetNextDoc ( pInlineStorage );
			if ( !tDoc.m_iDocID )
				break;

			// checks!
			if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN )
			{
				const CSphRowitem * pFound = FindDocinfo ( tDoc.m_iDocID );
				if ( !pFound )
					LOC_FAIL(( fp, "row not found (wordid="UINT64_FMT", docid="DOCID_FMT")",
						uint64_t(uWordid), tDoc.m_iDocID ));

				if ( pFound )
					if ( tDoc.m_iDocID!=DOCINFO2ID(pFound) )
						LOC_FAIL(( fp, "row found but id mismatches (wordid="UINT64_FMT", docid="DOCID_FMT", found="DOCID_FMT")",
							uint64_t(uWordid), tDoc.m_iDocID, DOCINFO2ID(pFound) ));
			}

			if ( tDoc.m_iDocID<=uLastDocid )
				LOC_FAIL(( fp, "docid decreased (wordid="UINT64_FMT", docid="DOCID_FMT", lastid="DOCID_FMT")",
					uint64_t(uWordid), tDoc.m_iDocID, uLastDocid ));

			uLastDocid = tDoc.m_iDocID;
			iDoclistDocs++;
			iDoclistHits += pQword->m_uMatchHits;

			// check position in case of regular (not-inline) hit
			if (!( pQword->m_iHitlistPos>>63 ))
			{
				if ( pQword->m_iHitlistPos!=pQword->m_rdHitlist.GetPos() )
					LOC_FAIL(( fp, "unexpected hitlist offset (wordid="UINT64_FMT", docid="DOCID_FMT", expected="INT64_FMT", actual="INT64_FMT")",
						(uint64_t)uWordid, pQword->m_tDoc.m_iDocID,
						(int64_t)pQword->m_iHitlistPos, (int64_t)pQword->m_rdHitlist.GetPos() ));
			}

			// aim
			pQword->SeekHitlist ( pQword->m_iHitlistPos );

			// loop the hitlist
			int iDocHits = 0;
			DWORD uFieldMask = 0;
			DWORD uLastHit = 0;

			for ( ;; )
			{
				DWORD uHit = pQword->GetNextHit();
				if ( !uHit )
					break;

				if ( uHit<=uLastHit )
					LOC_FAIL(( fp, "hit decreased (wordid="UINT64_FMT", docid="DOCID_FMT", hit=%u, last=%u)",
						(uint64_t)uWordid, pQword->m_tDoc.m_iDocID, uHit, uLastHit ));
				uLastHit = uHit;

				int iField = HIT2FIELD(uHit);
				if ( iField<0 || iField>=SPH_MAX_FIELDS )
				{
					LOC_FAIL(( fp, "hit field out of bounds (wordid="UINT64_FMT", docid="DOCID_FMT", field=%d)",
						(uint64_t)uWordid, pQword->m_tDoc.m_iDocID, iField ));

				} else if ( iField>=m_tSchema.m_dFields.GetLength() )
				{
					LOC_FAIL(( fp, "hit field out of schema (wordid="UINT64_FMT", docid="DOCID_FMT", field=%d)",
						(uint64_t)uWordid, pQword->m_tDoc.m_iDocID, iField ));
				}

				uFieldMask |= ( 1UL<<iField );
				iDocHits++; // to check doclist entry
				iHitlistHits++; // to check dictionary entry
			}

			// check hit count
			if ( iDocHits!=(int)pQword->m_uMatchHits )
				LOC_FAIL(( fp, "doc hit count mismatch (wordid="UINT64_FMT", docid="DOCID_FMT", doclist=%d, hitlist=%d)",
					(uint64_t)uWordid, pQword->m_tDoc.m_iDocID, pQword->m_uMatchHits, iDocHits ));

			// check the mask
			if ( uFieldMask!=pQword->m_uFields )
				LOC_FAIL(( fp, "field mask mismatch (wordid="UINT64_FMT", docid="DOCID_FMT", doclist=0x%x, hitlist=0x%x)",
					(uint64_t)uWordid, pQword->m_tDoc.m_iDocID, pQword->m_uFields, uFieldMask ));

			// update my hitlist reader
			rdHits.SeekTo ( pQword->m_rdHitlist.GetPos(), READ_NO_SIZE_HINT );
		}

		// do checks
		if ( iDictDocs!=iDoclistDocs )
			LOC_FAIL(( fp, "doc count mismatch (wordid="UINT64_FMT", dict=%d, doclist=%d)",
				uint64_t(uWordid), iDictDocs, iDoclistDocs ));

		if ( iDictHits!=iDoclistHits || iDictHits!=iHitlistHits )
			LOC_FAIL(( fp, "hit count mismatch (wordid="UINT64_FMT", dict=%d, doclist=%d, hitlist=%d)",
				uint64_t(uWordid), iDictHits, iDoclistHits, iHitlistHits ));

		// move my reader instance forward too
		rdDocs.SeekTo ( pQword->m_rdDoclist.GetPos(), READ_NO_SIZE_HINT );

		// cleanup
		SafeDelete ( pInlineStorage );
		SafeDelete ( pQword );

		// progress bar
		if ( (++iWordsChecked)%1000==0 && bProgress )
		{
			fprintf ( fp, "%d/%d\r", iWordsChecked, iWordsTotal );
			fflush ( fp );
		}
	}

	///////////////////////////
	// check rows (attributes)
	///////////////////////////

	if ( m_tSettings.m_eDocinfo==SPH_DOCINFO_EXTERN && !m_pDocinfo.IsEmpty() )
	{
		fprintf ( fp, "checking rows...\n" );

		// sizes and counts
		DWORD uRowsTotal = m_uDocinfo;
		DWORD uStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

		DWORD uAllRowsTotal = uRowsTotal;
		uAllRowsTotal += 2*(1+m_uDocinfoIndex); // should had been fixed up to v.20 by the loader

		if ( uAllRowsTotal*uStride!=m_pDocinfo.GetNumEntries() )
			LOC_FAIL(( fp, "rowitems count mismatch (expected=%u, loaded=%u)",
				uAllRowsTotal*uStride, m_pDocinfo.GetNumEntries() ));

		// extract rowitem indexes for MVAs etc
		// (ie. attr types that we can and will run additional checks on)
		CSphVector<int> dMvaItems;
		CSphVector<CSphAttrLocator> dFloatItems;
		CSphVector<CSphAttrLocator> dStrItems;
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
			{
				if ( tAttr.m_eAttrType!=( SPH_ATTR_MULTI | SPH_ATTR_INTEGER ))
				{
					LOC_FAIL(( fp, "unknown MVA type (attr=%d, type=0x%x)", i, tAttr.m_eAttrType ));
					continue;
				}
				if ( tAttr.m_tLocator.m_iBitCount!=ROWITEM_BITS )
				{
					LOC_FAIL(( fp, "unexpected MVA bitcount (attr=%d, expected=%d, got=%d)",
						i, ROWITEM_BITS, tAttr.m_tLocator.m_iBitCount ));
					continue;
				}
				if ( ( tAttr.m_tLocator.m_iBitOffset % ROWITEM_BITS )!=0 )
				{
					LOC_FAIL(( fp, "unaligned MVA bitoffset (attr=%d, bitoffset=%d)",
						i, tAttr.m_tLocator.m_iBitOffset ));
					continue;
				}
				dMvaItems.Add ( tAttr.m_tLocator.m_iBitOffset/ROWITEM_BITS );
			} else if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
				dFloatItems.Add	( tAttr.m_tLocator );
			else if ( tAttr.m_eAttrType==SPH_ATTR_STRING )
				dStrItems.Add ( tAttr.m_tLocator );
		}

		// loop the rows
		const CSphRowitem * pRow = m_pDocinfo.GetWritePtr();
		const DWORD * pMvaBase = m_pMva.GetWritePtr();
		const DWORD * pMvaMax = pMvaBase + m_pMva.GetNumEntries();
		const DWORD * pMva = pMvaBase;
		const BYTE * pStrLast = NULL;

		int iOrphan = 0;
		SphDocID_t uLastID = 0;

		for ( DWORD uRow=0; uRow<uRowsTotal; uRow++, pRow+=uStride )
		{
			// check that ids are ascending
			bool bIsSpaValid = uLastID < DOCINFO2ID(pRow);
			if ( !bIsSpaValid )
				LOC_FAIL(( fp, "docid decreased (row=%u, id="DOCID_FMT", lastid="DOCID_FMT")",
					uRow, DOCINFO2ID(pRow), uLastID ));

			uLastID = DOCINFO2ID(pRow);

			///////////////////////////
			// check MVAs
			///////////////////////////

			if ( dMvaItems.GetLength() )
			{
				const DWORD * pMvaSpaFixed = NULL;
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
				bool bHasValues = false;
				ARRAY_FOREACH ( iItem, dMvaItems )
				{
					const DWORD uOffset = pAttrs[dMvaItems[iItem]];
					bHasValues |= uOffset!=0;

					if ( pMvaBase+uOffset>=pMvaMax )
					{
						bIsSpaValid = false;
						LOC_FAIL(( fp, "MVA index out of bounds (row=%u, mvaattr=%d, docid="DOCID_FMT", index=%u)",
							uRow, iItem, uLastID, uOffset ));
					}

					if ( uOffset && pMvaBase+uOffset<pMvaMax && !pMvaSpaFixed )
						pMvaSpaFixed = pMvaBase + uOffset - sizeof(SphDocID_t) / sizeof(DWORD);
				}

				// MVAs ptr recovery from previous errors only if current spa record is valid
				if ( pMva!=pMvaSpaFixed && bIsSpaValid && pMvaSpaFixed )
					pMva = pMvaSpaFixed;

				bool bLastIDChecked = false;

				SphDocID_t uLastMvaID = 0;
				while ( pMva<pMvaMax && DOCINFO2ID(pMva)<=uLastID )
				{
					const SphDocID_t uMvaID = DOCINFO2ID(pMva);
					pMva = DOCINFO2ATTRS(pMva);

					if ( bLastIDChecked && uLastID==uMvaID )
						LOC_FAIL(( fp, "duplicate docid found (row=%u, docid expected="DOCID_FMT", got="DOCID_FMT", index=%u)",
							uRow, uLastID, uMvaID, (DWORD)(pMva-pMvaBase) ));

					if ( uMvaID<uLastMvaID )
						LOC_FAIL(( fp, "MVA docid decreased (row=%u, spa docid="DOCID_FMT", last MVA docid="DOCID_FMT", MVA docid="DOCID_FMT", index=%u)",
							uRow, uLastID, uLastMvaID, uMvaID, (DWORD)(pMva-pMvaBase) ));

					bool bIsMvaCorrect = uLastMvaID<=uMvaID && uMvaID<=uLastID;
					uLastMvaID = uMvaID;

					// loop MVAs
					ARRAY_FOREACH_COND ( iItem, dMvaItems, bIsMvaCorrect )
					{
						const DWORD uSpaOffset = pAttrs[dMvaItems[iItem]];

						// check offset (index)
						if ( uMvaID==uLastID && uSpaOffset && bIsSpaValid && pMva!=pMvaBase+uSpaOffset )
						{
							LOC_FAIL(( fp, "unexpected MVA docid (row=%u, mvaattr=%d, docid expected="DOCID_FMT", got="DOCID_FMT", expected=%u, got=%u)",
								uRow, iItem, uLastID, uMvaID, (DWORD)(pMva-pMvaBase), uSpaOffset ));
							// it's unexpected but it's our best guess
							// but do fix up only once, to prevent infinite loop
							if ( !bLastIDChecked )
								pMva = pMvaBase+uSpaOffset;
						}

						if ( pMva>=pMvaMax )
						{
							LOC_FAIL(( fp, "MVA index out of bounds (row=%u, mvaattr=%d, docid expected="DOCID_FMT", got="DOCID_FMT", index=%u)",
								uRow, iItem, uLastID, uMvaID, (DWORD)(pMva-pMvaBase) ));
							bIsMvaCorrect = false;
							continue;
						}

						// check values
						const DWORD uValues = *pMva++;
						if ( pMva+uValues-1>=pMvaMax )
						{
							LOC_FAIL(( fp, "MVA count out of bounds (row=%u, mvaattr=%d, docid expected="DOCID_FMT", got="DOCID_FMT", count=%u)",
								uRow, iItem, uLastID, uMvaID, uValues ));
							pMva += uValues;
							bIsMvaCorrect = false;
							continue;
						}
						// check that values are ascending
						for ( DWORD uVal=1; uVal<uValues && bIsMvaCorrect; uVal++ )
							if ( pMva[uVal]<=pMva[uVal-1] )
							{
								LOC_FAIL(( fp, "unsorted MVA values (row=%u, mvaattr=%d, docid expected="DOCID_FMT", got="DOCID_FMT", val[%u]=%u, val[%u]=%u)",
									uRow, iItem, uLastID, uMvaID, uVal-1, pMva[uVal-1], uVal, pMva[uVal] ));
								bIsMvaCorrect = false;
							}
						pMva += uValues;
					}

					if ( !bIsMvaCorrect )
						break;

					// orphan only ON no errors && ( not matched ids || ids matched multiply times )
					if ( bIsMvaCorrect && ( uMvaID!=uLastID || ( uMvaID==uLastID && bLastIDChecked ) ) )
						iOrphan++;

					bLastIDChecked |= uLastID==uMvaID;
				}

				if ( !bLastIDChecked && bHasValues )
					LOC_FAIL(( fp, "missed or damaged MVA (row=%u, docid expected="DOCID_FMT")",
						uRow, uLastID ));
			}

			///////////////////////////
			// check floats
			///////////////////////////

			ARRAY_FOREACH ( iItem, dFloatItems )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
				const DWORD uValue = (DWORD)sphGetRowAttr ( pAttrs, dFloatItems[ iItem ] );
				const DWORD uExp = ( uValue >> 23 ) & 0xff;
				const DWORD uMantissa = uValue & 0x003fffff;

				// check normalized
				if ( uExp==0 && uMantissa!=0 )
					LOC_FAIL(( fp, "float attribute value is unnormalized (row=%u, attr=%d, id="DOCID_FMT", raw=0x%x, value=%f)",
						uRow, iItem, uLastID, uValue, sphDW2F ( uValue ) ));

				// check +-inf
				if ( uExp==0xff && uMantissa==0 )
					LOC_FAIL(( fp, "float attribute is infinity (row=%u, attr=%d, id="DOCID_FMT", raw=0x%x, value=%f)",
						uRow, iItem, uLastID, uValue, sphDW2F ( uValue ) ));
			}

			///////////////////////////
			// check strings
			///////////////////////////

			ARRAY_FOREACH ( iItem, dStrItems )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);

				const DWORD uOffset = (DWORD)sphGetRowAttr ( pAttrs, dStrItems[ iItem ] );
				if ( uOffset>=m_pStrings.GetNumEntries() )
					LOC_FAIL(( fp, "string offset out of bounds (row=%u, stringattr=%d, docid="DOCID_FMT", index=%u)",
						uRow, iItem, uLastID, uOffset ));


				const BYTE * pStr = NULL;
				const int iLen = sphUnpackStr ( m_pStrings.GetWritePtr() + uOffset, &pStr );

				if ( pStr+iLen-1>=m_pStrings.GetWritePtr()+m_pStrings.GetLength() )
					LOC_FAIL(( fp, "string length out of bounds (row=%u, stringattr=%d, docid="DOCID_FMT", index=%u)",
						uRow, iItem, uLastID, pStr-m_pStrings.GetWritePtr()+iLen-1 ));

				if ( pStrLast>=pStr )
					LOC_FAIL(( fp, "overlapping string values (row=%u, stringattr=%d, docid="DOCID_FMT", last_end=%u, cur_start=%u)",
						uRow, iItem, uLastID, pStrLast-m_pStrings.GetWritePtr(), pStr-m_pStrings.GetWritePtr() ));

				pStrLast = pStr + iLen;
			}

			// progress bar
			if ( uRow%1000==0 && bProgress )
			{
				fprintf ( fp, "%d/%d\r", uRow, uRowsTotal );
				fflush ( fp );
			}
		}

		if ( iOrphan )
			fprintf ( fp, "WARNING: %d orphaned MVA entries were found\n", iOrphan );

		///////////////////////////
		// check blocks index
		///////////////////////////

		fprintf ( fp, "checking attribute blocks index...\n" );

		// check size
		const DWORD uTempDocinfoIndex = ( m_uDocinfo+DOCINFO_INDEX_FREQ-1 ) / DOCINFO_INDEX_FREQ;
		if ( uTempDocinfoIndex!=m_uDocinfoIndex )
			LOC_FAIL(( fp, "block count differs (expected=%d, got=%d)",
				uTempDocinfoIndex, m_uDocinfoIndex ));

		const DWORD uMinMaxStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
		const DWORD * pDocinfoIndexMax = m_pDocinfoIndex + 2*( 1+m_uDocinfoIndex )*uMinMaxStride;

		for ( DWORD uIndexEntry=0; uIndexEntry<m_uDocinfo; uIndexEntry++ )
		{
			const DWORD uBlock = uIndexEntry / DOCINFO_INDEX_FREQ;

			// we have to do some checks in border cases, for example: when move from 1st to 2nd block
			const DWORD uPrevEntryBlock = ( uIndexEntry-1 )/DOCINFO_INDEX_FREQ;
			const bool bIsBordersCheckTime = uPrevEntryBlock!=uBlock;

			const DWORD * pAttr = m_pDocinfo.GetWritePtr() + uIndexEntry * uMinMaxStride;
			const SphDocID_t uDocID = DOCINFO2ID(pAttr);

			const DWORD * pMinEntry = m_pDocinfoIndex + 2 * uBlock * uMinMaxStride;
			const DWORD * pMaxEntry = pMinEntry + uMinMaxStride;
			const DWORD * pMinAttrs = DOCINFO2ATTRS ( pMinEntry );
			const DWORD * pMaxAttrs = pMinAttrs + uMinMaxStride;

			// check docid vs global range
			if ( pMaxEntry+uMinMaxStride > pDocinfoIndexMax )
				LOC_FAIL(( fp, "unexpected block index end (row=%u, docid="DOCID_FMT", block=%d, max=%u, cur=%u)",
					uIndexEntry, uDocID, uBlock, (DWORD)(pDocinfoIndexMax-m_pDocinfoIndex), (DWORD)(pMaxEntry+uMinMaxStride-m_pDocinfoIndex) ));

			// check attribute location vs global range
			if ( pMaxAttrs+uMinMaxStride > pDocinfoIndexMax )
				LOC_FAIL(( fp, "attribute position out of blocks index (row=%u, docid="DOCID_FMT", block=%u, expected<%u, got=%u)",
					uIndexEntry, uDocID, uBlock, (DWORD)(pDocinfoIndexMax-m_pDocinfoIndex), (DWORD)(pMaxAttrs+uMinMaxStride-m_pDocinfoIndex) ));

			const SphDocID_t uMinDocID = *pMinEntry;
			const SphDocID_t uMaxDocID = *pMaxEntry;

			// checks is docid min max range valid
			if ( uMinDocID > uMaxDocID && bIsBordersCheckTime )
				LOC_FAIL(( fp, "invalid docid range (row=%u, block=%d, min="DOCID_FMT", max="DOCID_FMT")",
					uIndexEntry, uBlock, uMinDocID, uMaxDocID ));

			// checks docid vs blocks range
			if ( uDocID < uMinDocID || uDocID > uMaxDocID )
				LOC_FAIL(( fp, "unexpected docid range (row=%u, docid="DOCID_FMT", block=%d, min="DOCID_FMT", max="DOCID_FMT")",
					uIndexEntry, uDocID, uBlock, uMinDocID, uMaxDocID ));

			bool bIsFirstMva = true;

			// check values vs blocks range
			const DWORD * pSpaRow = DOCINFO2ATTRS(pAttr);
			for ( int iItem=0; iItem<m_tSchema.GetAttrsCount(); iItem++ )
			{
				const CSphColumnInfo & tCol = m_tSchema.GetAttr(iItem);

				switch ( tCol.m_eAttrType )
				{
				case SPH_ATTR_INTEGER:
				case SPH_ATTR_TIMESTAMP:
				case SPH_ATTR_BOOL:
				case SPH_ATTR_BIGINT:
					{
						const SphAttr_t uVal = sphGetRowAttr ( pSpaRow, tCol.m_tLocator );
						const SphAttr_t uMin = sphGetRowAttr ( pMinAttrs, tCol.m_tLocator );
						const SphAttr_t uMax = sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator );

						// checks is attribute min max range valid
						if ( uMin > uMax && bIsBordersCheckTime )
							LOC_FAIL(( fp, "invalid attribute range (row=%u, block=%d, min="INT64_FMT", max="INT64_FMT")",
								uIndexEntry, uBlock, uMin, uMax ));

						if ( uVal < uMin || uVal > uMax )
							LOC_FAIL(( fp, "unexpected attribute value (row=%u, attr=%u, docid="DOCID_FMT", block=%d, value=0x%x, min=0x%x, max=0x%x)",
								uIndexEntry, iItem, uDocID, uBlock, (DWORD)uVal, (DWORD)uMin, (DWORD)uMax ));
					}
					break;

				case SPH_ATTR_FLOAT:
					{
						const float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pSpaRow, tCol.m_tLocator ) );
						const float fMin = sphDW2F ( (DWORD)sphGetRowAttr ( pMinAttrs, tCol.m_tLocator ) );
						const float fMax = sphDW2F ( (DWORD)sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator ) );

						// checks is attribute min max range valid
						if ( fMin > fMax && bIsBordersCheckTime )
							LOC_FAIL(( fp, "invalid attribute range (row=%u, block=%d, min=%f, max=%f)",
								uIndexEntry, uBlock, fMin, fMax ));

						if ( fVal < fMin || fVal > fMax )
							LOC_FAIL(( fp, "unexpected attribute value (row=%u, attr=%u, docid="DOCID_FMT", block=%d, value=%f, min=%f, max=%f)",
								uIndexEntry, iItem, uDocID, uBlock, fVal, fMin, fMax ));
					}
					break;

				case SPH_ATTR_INTEGER | SPH_ATTR_MULTI:
					{
						const DWORD uMin = (DWORD)sphGetRowAttr ( pMinAttrs, tCol.m_tLocator );
						const DWORD uMax = (DWORD)sphGetRowAttr ( pMaxAttrs, tCol.m_tLocator );

						// checks is MVA attribute min max range valid
						if ( uMin > uMax && bIsBordersCheckTime && uMin!=0xffffffff && uMax!=0 )
							LOC_FAIL(( fp, "invalid MVA range (row=%u, block=%d, min=0x%x, max=0x%x)",
							uIndexEntry, uBlock, uMin, uMax ));

						SphAttr_t uOff = sphGetRowAttr ( pSpaRow, tCol.m_tLocator );
						if ( !uOff )
							break;

						const DWORD * pMva = m_pMva.GetWritePtr() + uOff;
						const DWORD * pMvaDocID = bIsFirstMva ? ( pMva - sizeof(SphDocID_t) / sizeof(DWORD) ) : NULL;
						bIsFirstMva = false;

						if ( uOff>=m_pMva.GetNumEntries() )
							break;

						if ( pMvaDocID && DOCINFO2ID ( pMvaDocID )!=uDocID )
						{
							LOC_FAIL(( fp, "unexpected MVA docid (row=%u, mvaattr=%d, expected="DOCID_FMT", got="DOCID_FMT", block=%d, index=%u)",
								uIndexEntry, iItem, uDocID, DOCINFO2ID ( pMvaDocID ), uBlock, (DWORD)uOff ));
							break;
						}

						// check values
						const DWORD uValues = *pMva++;
						if ( uOff+uValues>m_pMva.GetNumEntries() )
							break;

						for ( DWORD iVal=0; iVal<uValues; iVal++ )
						{
							const DWORD uVal = *pMva++;
							if ( uVal < uMin || uVal > uMax )
								LOC_FAIL(( fp, "unexpected MVA value (row=%u, attr=%u, docid="DOCID_FMT", block=%d, index=%u, value=0x%x, min=0x%x, max=0x%x)",
								uIndexEntry, iItem, uDocID, uBlock, iVal, (DWORD)uVal, (DWORD)uMin, (DWORD)uMax ));
						}
					}
					break;

				default:
					break;
				}
			}

			// progress bar
			if ( uIndexEntry%1000==0 && bProgress )
			{
				fprintf ( fp, "%d/%d\r", uIndexEntry, m_uDocinfo );
				fflush ( fp );
			}
		}
	}

	///////////////////////////
	// check kill-list
	///////////////////////////

	fprintf ( fp, "checking kill-list...\n" );

	// check size
	if ( m_pKillList.GetNumEntries()!=m_iKillListSize )
		LOC_FAIL(( fp, "kill-list size differs (expected=%d, got=%d)",
			m_iKillListSize, m_pKillList.GetNumEntries() ));

	// check that ids are ascending
	for ( DWORD uID=1; uID<m_pKillList.GetNumEntries(); uID++ )
		if ( m_pKillList[uID]<=m_pKillList[uID-1] )
			LOC_FAIL(( fp, "unsorted kill-list values (val[%d]=%d, val[%d]=%d)",
				uID-1, (DWORD)m_pKillList[uID-1], uID, (DWORD)m_pKillList[uID] ));

	///////////////////////////
	// all finished
	///////////////////////////

	// well, no known kinds of failures, maybe some unknown ones
	tmCheck = sphMicroTimer() - tmCheck;
	if ( !iFails )
		fprintf ( fp, "check passed" );
	else if ( iFails!=iFailsPrinted )
		fprintf ( fp, "check FAILED, %d of %d failures reported", iFailsPrinted, iFails );
	else
		fprintf ( fp, "check FAILED, %d failures reported", iFails );
	fprintf ( fp, ", %d.%d sec elapsed\n", (int)(tmCheck/1000000), (int)((tmCheck/100000)%10) );

	return Min ( iFails, 255 ); // this is the exitcode; so cap it
} // NOLINT function length


//////////////////////////////////////////////////////////////////////////

/// morphology
enum
{
	SPH_MORPH_STEM_EN,
	SPH_MORPH_STEM_RU_CP1251,
	SPH_MORPH_STEM_RU_UTF8,
	SPH_MORPH_STEM_CZ,
	SPH_MORPH_SOUNDEX,
	SPH_MORPH_METAPHONE_SBCS,
	SPH_MORPH_METAPHONE_UTF8,
	SPH_MORPH_LIBSTEMMER_FIRST,
	SPH_MORPH_LIBSTEMMER_LAST = SPH_MORPH_LIBSTEMMER_FIRST + 64
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
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops );
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer );
	virtual bool		LoadWordforms ( const char * szFile, ISphTokenizer * pTokenizer );
	virtual bool		SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError );
	virtual SphWordID_t	GetWordIDWithMarkers ( BYTE * pWord );
	virtual SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord );
	virtual void		ApplyStemmers ( BYTE * pWord );

	virtual void		Setup ( const CSphDictSettings & tSettings ) { m_tSettings = tSettings; }
	virtual const CSphDictSettings & GetSettings () const { return m_tSettings; }
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () { return m_dSWFileInfos; }
	virtual const CSphSavedFile & GetWordformsFileInfo () { return m_tWFFileInfo; }
	virtual const CSphMultiformContainer * GetMultiWordforms () const { return m_pWordforms ? m_pWordforms->m_pMultiWordforms : NULL; }

	static void			SweepWordformContainers ( const char * szFile, DWORD uCRC32 );

	virtual bool IsStopWord ( const BYTE * pWord ) const;

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
	SphWordID_t			FilterStopword ( SphWordID_t uID ) const;	///< filter ID against stopwords list

private:
	typedef CSphOrderedHash < int, CSphString, CSphStrHashFunc, 1048576, 117 > CWordHash;

	struct WordformContainer
	{
		int							m_iRefCount;
		CSphString					m_sFilename;
		struct_stat					m_tStat;
		DWORD						m_uCRC32;
		CSphVector <CSphString>		m_dNormalForms;
		CSphMultiformContainer * m_pMultiWordforms;
		CWordHash					m_dHash;

									WordformContainer ();
									~WordformContainer ();

		bool						IsEqual ( const char * szFile, DWORD uCRC32 );
	};

	WordformContainer *	m_pWordforms;
	CSphVector <CSphSavedFile> m_dSWFileInfos;
	CSphSavedFile		m_tWFFileInfo;
	CSphDictSettings	m_tSettings;

	static CSphVector <WordformContainer*> m_dWordformContainers;

	static WordformContainer *	GetWordformContainer ( const char * szFile, DWORD uCRC32, const ISphTokenizer * pTokenizer );
	static WordformContainer *	LoadWordformContainer ( const char * szFile, DWORD uCRC32, const ISphTokenizer * pTokenizer );

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


static void GetFileStats ( const char * szFilename, CSphSavedFile & tInfo )
{
	if ( !szFilename )
	{
		memset ( &tInfo, 0, sizeof ( tInfo ) );
		return;
	}

	tInfo.m_sFilename = szFilename;

	struct_stat tStat;
	memset ( &tStat, 0, sizeof ( tStat ) );
	if ( stat ( szFilename, &tStat ) < 0 )
		memset ( &tStat, 0, sizeof ( tStat ) );

	tInfo.m_uSize = tStat.st_size;
	tInfo.m_uCTime = tStat.st_ctime;
	tInfo.m_uMTime = tStat.st_mtime;

	DWORD uCRC32 = 0;
	sphCalcFileCRC32 ( szFilename, uCRC32 );

	tInfo.m_uCRC32 = uCRC32;
}

/////////////////////////////////////////////////////////////////////////////

CSphDictCRC::WordformContainer::WordformContainer ()
	: m_iRefCount 		( 0 )
	, m_pMultiWordforms	( NULL )
{
}


CSphDictCRC::WordformContainer::~WordformContainer ()
{
	if ( m_pMultiWordforms )
	{
		m_pMultiWordforms->m_Hash.IterateStart ();
		while ( m_pMultiWordforms->m_Hash.IterateNext () )
		{
			CSphMultiforms * pWordforms = m_pMultiWordforms->m_Hash.IterateGet ();
			ARRAY_FOREACH ( i, pWordforms->m_dWordforms )
				SafeDelete ( pWordforms->m_dWordforms[i] );

			SafeDelete ( pWordforms );
		}

		SafeDelete ( m_pMultiWordforms );
	}
}


bool CSphDictCRC::WordformContainer::IsEqual ( const char * szFile, DWORD uCRC32 )
{
	if ( !szFile )
		return false;

	struct_stat FileStat;
	if ( stat ( szFile, &FileStat ) < 0 )
		return false;

	return m_sFilename==szFile && m_tStat.st_ctime==FileStat.st_ctime
		&& m_tStat.st_mtime==FileStat.st_mtime && m_tStat.st_size==FileStat.st_size && m_uCRC32==uCRC32;
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
		sb_stemmer_delete ( m_dStemmers[i] );
#endif

	if ( m_pWordforms )
		--m_pWordforms->m_iRefCount;
}


SphWordID_t CSphDictCRC::FilterStopword ( SphWordID_t uID ) const
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
	if ( !m_pWordforms )
		return false;

	int * pIndex = m_pWordforms->m_dHash ( (char *)pWord );
	if ( !pIndex )
		return false;

	if ( *pIndex<0 || *pIndex>=m_pWordforms->m_dNormalForms.GetLength () )
		return false;

	if ( m_pWordforms->m_dNormalForms [*pIndex].IsEmpty () )
		return false;

	strcpy ( (char *)pWord, m_pWordforms->m_dNormalForms[*pIndex].cstr() ); // NOLINT
	return true;
}

bool CSphDictCRC::ParseMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError )
{
	const char * szStart = szMorph;

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
			if ( !InitMorph ( szWordStart, szStart - szWordStart, bUseUTF8, sError ) )
				return false;
		}
	}

	return true;
}


bool CSphDictCRC::InitMorph ( const char * szMorph, int iLength, bool bUseUTF8, CSphString & sError )
{
	if ( iLength==0 )
		return true;

	if ( iLength==4 && !strncmp ( szMorph, "none", iLength ) )
		return true;

	if ( iLength==7 && !strncmp ( szMorph, "stem_en", iLength ) )
	{
		stem_en_init ();
		return AddMorph ( SPH_MORPH_STEM_EN );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_ru", iLength ) )
	{
		stem_ru_init ();
		return AddMorph ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_cz", iLength ) )
	{
		stem_cz_init ();
		return AddMorph ( SPH_MORPH_STEM_CZ );
	}

	if ( iLength==9 && !strncmp ( szMorph, "stem_enru", iLength ) )
	{
		stem_en_init ();
		stem_ru_init ();

		if ( !AddMorph ( SPH_MORPH_STEM_EN ) )
			return false;

		return AddMorph ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
	}

	if ( iLength==7 && !strncmp ( szMorph, "soundex", iLength ) )
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
			if ( m_dStemmers[i]==pStemmer )
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
		if ( m_dMorph[i]==iMorph )
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
	// try wordforms
	if ( ToNormalForm ( pWord ) )
		return;

	// check length
	if ( m_tSettings.m_iMinStemmingLen>1 )
		if ( sphUTF8Len ( (const char*)pWord )<m_tSettings.m_iMinStemmingLen )
			return;

	// try stemmers
	ARRAY_FOREACH ( i, m_dMorph )
		if ( StemById ( pWord, m_dMorph[i] ) )
			break;
}


SphWordID_t CSphDictCRC::GetWordID ( BYTE * pWord )
{
	ApplyStemmers ( pWord );
	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord ) );
}


SphWordID_t CSphDictCRC::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = sphCRCWord<SphWordID_t> ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


SphWordID_t	CSphDictCRC::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	SphWordID_t uWordId = sphCRCWord<SphWordID_t> ( pWord + 1 );
	int iLength = strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? sphCRCWord<SphWordID_t> ( pWord ) : 0;
}


SphWordID_t	CSphDictCRC::GetWordIDNonStemmed ( BYTE * pWord )
{
	SphWordID_t uWordId = sphCRCWord<SphWordID_t> ( pWord + 1 );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return sphCRCWord<SphWordID_t> ( pWord );
}


struct DwordCmp_fn
{
	inline int operator () ( DWORD b, DWORD a )
	{
		if ( a<b )
			return -1;
		else if ( a==b )
			return 0;
		else
			return 1;
	}
};


void CSphDictCRC::LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer )
{
	assert ( !m_pStopwords );
	assert ( !m_iStopwords );

	// tokenize file list
	if ( !sFiles || !*sFiles )
		return;

	m_dSWFileInfos.Resize ( 0 );

	char * sList = new char [ 1+strlen(sFiles) ];
	strcpy ( sList, sFiles ); // NOLINT

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

		CSphSavedFile tInfo;
		tInfo.m_sFilename = sName;
		GetFileStats ( sName, tInfo );
		m_dSWFileInfos.Add ( tInfo );

		// open file
		struct_stat st;
		if ( stat ( sName, &st )==0 )
			pBuffer = new BYTE [(size_t)st.st_size];
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
		int iLength = (int)fread ( pBuffer, 1, (size_t)st.st_size, fp );

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


void CSphDictCRC::SweepWordformContainers ( const char * szFile, DWORD uCRC32 )
{
	for ( int i = 0; i < m_dWordformContainers.GetLength (); )
	{
		WordformContainer * WC = m_dWordformContainers[i];
		if ( WC->m_iRefCount==0 && !WC->IsEqual ( szFile, uCRC32 ) )
		{
			delete WC;
			m_dWordformContainers.Remove ( i );
		} else
			++i;
	}
}

bool CSphDictCRC::IsStopWord ( const BYTE * pWord ) const
{
	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord ) )==0;
}

CSphDictCRC::WordformContainer * CSphDictCRC::GetWordformContainer ( const char * szFile, DWORD uCRC32, const ISphTokenizer * pTokenizer )
{
	ARRAY_FOREACH ( i, m_dWordformContainers )
		if ( m_dWordformContainers[i]->IsEqual ( szFile, uCRC32 ) )
			return m_dWordformContainers[i];

	WordformContainer * pContainer = LoadWordformContainer ( szFile, uCRC32, pTokenizer );
	if ( pContainer )
		m_dWordformContainers.Add ( pContainer );

	return pContainer;
}


CSphDictCRC::WordformContainer * CSphDictCRC::LoadWordformContainer ( const char * szFile, DWORD uCRC32, const ISphTokenizer * pTokenizer )
{
	// stat it; we'll store stats for later checks
	struct_stat FileStat;
	if ( !szFile || !*szFile || stat ( szFile, &FileStat )<0 )
		return NULL;

	// allocate it
	WordformContainer * pContainer = new WordformContainer;
	if ( !pContainer )
		return NULL;
	pContainer->m_sFilename = szFile;
	pContainer->m_tStat = FileStat;
	pContainer->m_uCRC32 = uCRC32;

	// open it
	CSphString sError;
	CSphAutoreader rdWordforms;
	if ( !rdWordforms.Open ( szFile, sError ) )
		return NULL;

	// my tokenizer
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( false ) );
	pMyTokenizer->AddSpecials ( ">" );

	// scan it line by line
	char sBuffer [ 6*SPH_MAX_WORD_LEN + 512 ]; // enough to hold 2 UTF-8 words, plus some whitespace overhead
	int iLen;
	bool bSeparatorFound = false;
	CSphString sFrom;
	while ( ( iLen = rdWordforms.GetLine ( sBuffer, sizeof(sBuffer) ) )>=0 )
	{
		// parse the line
		pMyTokenizer->SetBuffer ( (BYTE*)sBuffer, iLen );

		CSphMultiform * pMultiWordform = NULL;
		CSphString sKey;

		BYTE * pFrom = NULL;
		while ( ( pFrom = pMyTokenizer->GetToken () )!=NULL )
		{
			const BYTE * pCur = (const BYTE *) pMyTokenizer->GetBufferPtr ();

			while ( isspace(*pCur) ) pCur++;
			if ( *pCur=='>' )
			{
				sFrom = (const char*)pFrom;
				bSeparatorFound = true;
				pMyTokenizer->SetBufferPtr ( (const char*) pCur+1 );
				break;
			} else
			{
				if ( !pMultiWordform )
				{
					pMultiWordform = new CSphMultiform;
					sKey = (const char*)pFrom;
				} else
					pMultiWordform->m_dTokens.Add ( (const char*)pFrom );
			}
		}

		if ( !pFrom ) continue; // FIXME! report parsing error
		if ( !bSeparatorFound ) continue; // FIXME! report parsing error

		BYTE * pTo = pMyTokenizer->GetToken ();
		if ( !pTo ) continue; // FIXME! report parsing error

		if ( !pMultiWordform )
		{
			pContainer->m_dNormalForms.AddUnique ( (const char*)pTo );
			pContainer->m_dHash.Add ( pContainer->m_dNormalForms.GetLength()-1, sFrom.cstr() );
		} else
		{
			pMultiWordform->m_sNormalForm = (const char*)pTo;
			pMultiWordform->m_iNormalTokenLen = pMyTokenizer->GetLastTokenLen ();
			pMultiWordform->m_dTokens.Add ( sFrom );
			if ( !pContainer->m_pMultiWordforms )
				pContainer->m_pMultiWordforms = new CSphMultiformContainer;

			CSphMultiforms ** pWordforms = pContainer->m_pMultiWordforms->m_Hash ( sKey );
			if ( pWordforms )
			{
				(*pWordforms)->m_dWordforms.Add ( pMultiWordform );
				(*pWordforms)->m_iMinTokens = Min ( (*pWordforms)->m_iMinTokens, pMultiWordform->m_dTokens.GetLength () );
				(*pWordforms)->m_iMaxTokens = Max ( (*pWordforms)->m_iMaxTokens, pMultiWordform->m_dTokens.GetLength () );
				pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, (*pWordforms)->m_iMaxTokens );
			} else
			{
				CSphMultiforms * pNewWordforms = new CSphMultiforms;
				pNewWordforms->m_dWordforms.Add ( pMultiWordform );
				pNewWordforms->m_iMinTokens = pMultiWordform->m_dTokens.GetLength ();
				pNewWordforms->m_iMaxTokens = pMultiWordform->m_dTokens.GetLength ();
				pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pNewWordforms->m_iMaxTokens );
				pContainer->m_pMultiWordforms->m_Hash.Add ( pNewWordforms, sKey );
			}
		}
	}

	return pContainer;
}


bool CSphDictCRC::LoadWordforms ( const char * szFile, ISphTokenizer * pTokenizer )
{
	GetFileStats ( szFile, m_tWFFileInfo );

	DWORD uCRC32 = m_tWFFileInfo.m_uCRC32;

	SweepWordformContainers ( szFile, uCRC32 );
	m_pWordforms = GetWordformContainer ( szFile, uCRC32, pTokenizer );
	if ( m_pWordforms )
		m_pWordforms->m_iRefCount++;

	return !!m_pWordforms;
}


bool CSphDictCRC::SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError )
{
	m_dMorph.Reset ();

#if USE_LIBSTEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers[i] );

	m_dStemmers.Reset ();
#endif

	if ( !szMorph )
		return true;

	CSphString sOption = szMorph;
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
	strncpy ( szBuf, (char *)pWord, sizeof(szBuf) );

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

	case SPH_MORPH_STEM_CZ:
		stem_cz ( pWord );
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

CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, ISphTokenizer * pTokenizer, CSphString & sError )
{
	assert ( pTokenizer );

	CSphDict * pDict = new CSphDictCRC ();
	if ( !pDict )
		return NULL;

	pDict->Setup ( tSettings );

	if ( pDict->SetMorphology ( tSettings.m_sMorphology.cstr (), pTokenizer->IsUtf8(), sError ) )
		sError = "";

	pDict->LoadStopwords ( tSettings.m_sStopwords.cstr (), pTokenizer );
	pDict->LoadWordforms ( tSettings.m_sWordforms.cstr (), pTokenizer );

	return pDict;
}


void sphShutdownWordforms ()
{
	CSphDictCRC::SweepWordformContainers ( NULL, 0 );
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
	if ( !sConfig )
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
	if ( !sConfig )
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
				s += 2; // skip opening '<?'
				while ( *s )
				{
					if ( s[0]=='?' && s[1]=='>' )
						break;
					s++;
				}
				if ( !*s )
					break;
				s += 2; // skip closing '?>'
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
				if ( iCmp==0 && !sphIsTag ( sTagName[iLen] ) )
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
				if ( *s!='>' )
					s++;
			}
			if ( !sphIsTag(*s) )
			{
				if ( *s && *s!='>' )
					s++;
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
				if ( s[iLen]=='=' || isspace ( s[iLen] ) )
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
				*d++ = ' ';
				if ( *s==cEnd ) s++;
				continue;
			}

			// handle unquoted value
			while ( *s && !isspace(*s) && *s!='>' ) *d++ = *s++;
			*d++ = ' ';
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
			if ( !sphIsTag ( s[tTag.m_iTagLen] ) )
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

CSphSourceSettings::CSphSourceSettings ()
	: m_iMinPrefixLen ( 0 )
	, m_iMinInfixLen ( 0 )
	, m_iBoundaryStep ( 0 )
	, m_bIndexExactWords ( false )
	, m_iOvershortStep ( 1 )
	, m_iStopwordStep ( 1 )
{}

//////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource ( const char * sName )
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_tSchema ( sName )
	, m_bStripHTML ( false )
	, m_iNullIds ( 0 )
	, m_iMaxIds ( 0 )
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


void CSphSource::SetTokenizer ( ISphTokenizer * pTokenizer )
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
	return m_tSchema.CompareTo ( *pInfo, sError );
}


void CSphSource::Setup ( const CSphSourceSettings & tSettings )
{
	m_iMinPrefixLen = Max ( tSettings.m_iMinPrefixLen, 0 );
	m_iMinInfixLen = Max ( tSettings.m_iMinInfixLen, 0 );
	m_iBoundaryStep = Max ( tSettings.m_iBoundaryStep, -1 );
	m_bIndexExactWords = tSettings.m_bIndexExactWords;
	m_iOvershortStep = Min ( Max ( tSettings.m_iOvershortStep, 0 ), 1 );
	m_iStopwordStep = Min ( Max ( tSettings.m_iStopwordStep, 0 ), 1 );
}


SphDocID_t CSphSource::VerifyID ( SphDocID_t uID )
{
	if ( uID==0 )
	{
		m_iNullIds++;
		return 0;
	}

	if ( uID==DOCID_MAX )
	{
		m_iMaxIds++;
		return 0;
	}

	return uID;
}


void CSphSource::AddHitFor ( SphWordID_t iWordID, DWORD iWordPos )
{
	if ( !iWordID )
		return;
	CSphWordHit & tHit = m_dHits.Add();
	tHit.m_iDocID = m_tDocInfo.m_iDocID;
	tHit.m_iWordID = iWordID;
	tHit.m_iWordPos = iWordPos;
}


bool CSphSource::IterateJoinedHits ( CSphString & )
{
	m_tDocInfo.m_iDocID = 0; // pretend that's an eof
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

static void FormatEscaped ( FILE * fp, const char * sLine )
{
	// handle empty lines
	if ( !sLine || !*sLine )
	{
		fprintf ( fp, ", ''" );
		return;
	}

	// pass one, count the needed buffer size
	int iLen = strlen(sLine);
	int iOut = 0;
	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\r':
		case '\n':
		case '\t':
		case '\'':
			iOut += 2;
			break;

		default:
			iOut++;
			break;
	}
	iOut += 4; // comma, spaces, quotes

	// allocate the buffer
	char sMinibuffer[8192];
	char * sMaxibuffer = NULL;
	char * sBuffer = sMinibuffer;

	if ( iOut>sizeof(sMinibuffer) )
	{
		sMaxibuffer = new char [ iOut+4 ]; // 4 is just my safety gap
		sBuffer = sMaxibuffer;
	}

	// pass two, escape it
	char * sOut = sBuffer;

	strncpy ( sOut, ", '", 3 );
	sOut += 3;

	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\r':	*sOut++ = '\\'; *sOut++ = 'r'; break;
		case '\n':	*sOut++ = '\\'; *sOut++ = 'n'; break;
		case '\t':	*sOut++ = '\\'; *sOut++ = 't'; break;
		case '\'':	*sOut++ = '\\'; *sOut++ = '\''; break;
		default:	*sOut++ = sLine[i];
	}
	*sOut++ = '\'';

	// print!
	assert ( sOut==sBuffer+iOut );
	fwrite ( sBuffer, 1, iOut, fp );

	// cleanup
	SafeDeleteArray ( sMaxibuffer );
}


bool CSphSource_Document::IterateHitsNext ( CSphString & sError )
{
	assert ( m_pTokenizer );
	PROFILE ( src_document );

	BYTE ** dFields = NextDocument ( sError );
	if ( m_tDocInfo.m_iDocID==0 )
		return true;
	if ( !dFields )
		return false;

	if ( m_fpDumpRows )
	{
		fprintf ( m_fpDumpRows, "INSERT INTO rows VALUES (" DOCID_FMT, m_tDocInfo.m_iDocID );
		for ( int iAttr=0; iAttr<m_tSchema.GetAttrsCount(); iAttr++ )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iAttr);
			switch ( tCol.m_eAttrType )
			{
				case SPH_ATTR_INTEGER | SPH_ATTR_MULTI:
					fprintf ( m_fpDumpRows, ", ''" );
					break;

				case SPH_ATTR_STRING:
					FormatEscaped ( m_fpDumpRows, m_dStrAttrs[iAttr].cstr() );
					break;

				case SPH_ATTR_BIGINT:
					fprintf ( m_fpDumpRows, ", " INT64_FMT, m_tDocInfo.GetAttr ( tCol.m_tLocator ) );
					break;

				default:
					fprintf ( m_fpDumpRows, ", %u", (DWORD)m_tDocInfo.GetAttr ( tCol.m_tLocator ) );
					break;
			}
		}

		ARRAY_FOREACH ( i, m_tSchema.m_dFields )
			FormatEscaped ( m_fpDumpRows, (const char*) dFields[i] );

		fprintf ( m_fpDumpRows, ");\n" );
	}

	m_tStats.m_iTotalDocuments++;
	m_dHits.Reserve ( 1024 );
	m_dHits.Resize ( 0 );

	if ( !BuildHits ( dFields, -1, 0, sError ) )
		return false;

	return true;
}


bool CSphSource_Document::BuildHits ( BYTE ** dFields, int iFieldIndex, int iStartPos, CSphString & sError )
{
	bool bGlobalPartialMatch = m_iMinPrefixLen > 0 || m_iMinInfixLen > 0;

	int iStartField = 0;
	int iEndField = m_tSchema.m_iBaseFields;
	if ( !iEndField )
		iEndField = m_tSchema.m_dFields.GetLength();

	if ( iFieldIndex>=0 )
	{
		iStartField = iFieldIndex;
		iEndField = iFieldIndex+1;
	}

	CSphAutofile	tFileSource;
	for ( int iField=iStartField; iField<iEndField; iField++ )
	{
		BYTE * sField = dFields[iField-iStartField];
		if ( !sField || !(*sField) )
			continue;

		int iFieldBytes = 0;

		// get data from file source
		if ( m_tSchema.m_dFields[iField].m_bFilename )
		{
			if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
				return false;
			else
			{
				iFieldBytes = (int)tFileSource.GetSize();

				if ( iFieldBytes > m_iMaxFileBufferSize )
				{
					sError.SetSprintf ( "file '%s' too big for a field (size="INT64_FMT", max_file_field_buffer="INT64_FMT")", (const char *)sField, (int64_t)iFieldBytes, (int64_t)m_iMaxFileBufferSize );
					return false;
				}

				int iBufSize = m_iReadFileBufferSize;

				while ( iBufSize < iFieldBytes ) iBufSize <<= 1;

				if ( m_iReadFileBufferSize < iBufSize )
					SafeDeleteArray ( m_pReadFileBuffer );

				if ( !m_pReadFileBuffer )
				{
					m_pReadFileBuffer = new char[iBufSize];
					m_iReadFileBufferSize = iBufSize;
				}

				assert ( m_pReadFileBuffer );

				if ( tFileSource.Read ( m_pReadFileBuffer, iFieldBytes, sError ) )
				{
					sField = (BYTE*)m_pReadFileBuffer;
					tFileSource.Close();
				} else
				{
					sError.SetSprintf ( "failed to read file '%s'", (const char *)sField );
					return false;
				}
			}
		} else
		{
			if ( m_bStripHTML )
				m_pStripper->Strip ( sField );

			iFieldBytes = (int) strlen ( (char*)sField );
		}

		m_tStats.m_iTotalBytes += iFieldBytes;

		m_pTokenizer->SetBuffer ( sField, iFieldBytes );

		BYTE * sWord;
		int iPos = HIT_PACK ( iField, iStartPos );
		int iLastStep = 1;

		bool bPrefixField = m_tSchema.m_dFields[iField].m_eWordpart==SPH_WORDPART_PREFIX;
		bool bInfixMode = m_iMinInfixLen > 0;

		BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

		if ( m_tSchema.m_dFields[iField].m_eWordpart!=SPH_WORDPART_WHOLE )
		{
			int iMinInfixLen = bPrefixField ? m_iMinPrefixLen : m_iMinInfixLen;

			// index all infixes
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				if ( !m_tSchema.m_dFields[iField].m_bPayload )
				{
					iPos += iLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep;
					if ( m_pTokenizer->GetBoundary() )
						iPos = Max ( iPos+m_iBoundaryStep, 1 );
					iLastStep = 1;
				}

				int iLen = m_pTokenizer->GetLastTokenLen ();

				// always index full word (with magic head/tail marker(s))
				int iBytes = strlen ( (const char*)sWord );

				if ( m_bIndexExactWords )
				{
					int iBytes = strlen ( (const char*)sWord );
					memcpy ( sBuf + 1, sWord, iBytes );
					sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
					sBuf[iBytes+1] = '\0';
					AddHitFor ( m_pDict->GetWordIDNonStemmed ( sBuf ), iPos );
				}

				memcpy ( sBuf + 1, sWord, iBytes );
				sBuf[0] = MAGIC_WORD_HEAD;
				sBuf[iBytes+1] = '\0';

				// stemmed word w/markers
				SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
				if ( iWord )
					AddHitFor ( iWord, iPos );
				else
				{
					iLastStep = m_iStopwordStep;
					continue;
				}

				// restore stemmed word
				int iStemmedLen = strlen ( ( const char *)sBuf );
				sBuf [iStemmedLen - 1] = '\0';

				// stemmed word w/o markers
				if ( strcmp ( (const char *)sBuf + 1, (const char *)sWord ) )
					AddHitFor ( m_pDict->GetWordID ( sBuf + 1, iStemmedLen - 2, true ), iPos );

				// restore word
				memcpy ( sBuf + 1, sWord, iBytes );
				sBuf[iBytes+1] = MAGIC_WORD_TAIL;
				sBuf[iBytes+2] = '\0';

				// if there are no infixes, that's it
				if ( iMinInfixLen > iLen )
				{
					// index full word
					AddHitFor ( m_pDict->GetWordID ( sWord ), iPos );
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
						AddHitFor ( m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix, false ), iPos );

						// word start: add magic head
						if ( bInfixMode && iStart==0 )
							AddHitFor ( m_pDict->GetWordID ( sInfix - 1, sInfixEnd-sInfix + 1, false ), iPos );

						// word end: add magic tail
						if ( bInfixMode && i==iLen-iStart )
							AddHitFor ( m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix+1, false ), iPos );

						sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );
					}

					sInfix += m_pTokenizer->GetCodepointLength ( *sInfix );
				}
			}

			// mark trailing hits
			if ( m_dHits.GetLength() )
			{
				DWORD uRefPos = m_dHits.Last().m_iWordPos;
				for	( int i=m_dHits.GetLength()-1; i>=0 && m_dHits[i].m_iWordPos==uRefPos; i-- )
					m_dHits[i].m_iWordPos |= HIT_FIELD_END;
			}

		} else
		{
			// index words only
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				if ( !m_tSchema.m_dFields[iField].m_bPayload )
				{
					iPos += iLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep;
					if ( m_pTokenizer->GetBoundary() )
						iPos = Max ( iPos+m_iBoundaryStep, 1 );
				}

				if ( bGlobalPartialMatch )
				{
					int iBytes = strlen ( (const char*)sWord );
					memcpy ( sBuf + 1, sWord, iBytes );
					sBuf[0] = MAGIC_WORD_HEAD;
					sBuf[iBytes+1] = '\0';
					AddHitFor ( m_pDict->GetWordIDWithMarkers ( sBuf ), iPos );
				}

				if ( m_bIndexExactWords )
				{
					int iBytes = strlen ( (const char*)sWord );
					memcpy ( sBuf + 1, sWord, iBytes );
					sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
					sBuf[iBytes+1] = '\0';
					AddHitFor ( m_pDict->GetWordIDNonStemmed ( sBuf ), iPos );
				}

				SphWordID_t iWord = m_pDict->GetWordID ( sWord );
				if ( iWord )
				{
					AddHitFor ( iWord, iPos );
					iLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
				} else
					iLastStep = m_iStopwordStep;
			}
		}

		// mark trailing hit
		if ( m_dHits.GetLength() )
			m_dHits.Last().m_iWordPos |= HIT_FIELD_END;
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


void CSphSource_Document::ParseFieldMVA ( CSphVector < CSphVector < DWORD > > & dFieldMVAs, int iFieldMVA, const char * szValue )
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
		if ( *pPtr>='0' && *pPtr<='9' )
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
	if ( !szString || szString[0]=='\0' )
		return true;

	if ( !szField || szField[0]=='\0' )
		return false;

	const char * szPos = strstr ( szString, szField );
	if ( !szPos )
		return false;

	int iFieldLen = strlen ( szField );
	bool bStartOk = szPos==szString || !sphIsAlpha ( *(szPos - 1) );
	bool bEndOk = !*(szPos + iFieldLen) || !sphIsAlpha ( *(szPos + iFieldLen) );

	return bStartOk && bEndOk;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SQL SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSourceParams_SQL::CSphSourceParams_SQL ()
	: m_iRangeStep ( 1024 )
	, m_iRangedThrottle ( 0 )
	, m_iMaxFileBufferSize ( 0 )
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
	, m_bCanUnpack			( false )
	, m_bUnpackFailed		( false )
	, m_bUnpackOverflow		( false )
	, m_iJoinedHitField		( -1 )
	, m_iJoinedHitID		( 0 )
	, m_iJoinedHitPos		( 0 )
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

	if ( m_tParams.m_iMaxFileBufferSize > 0 )
		m_iMaxFileBufferSize = m_tParams.m_iMaxFileBufferSize;

	return true;
}


bool CSphSource_SQL::RunQueryStep ( const char * sQuery, CSphString & sError )
{
	sError = "";

	if ( m_tParams.m_iRangeStep<=0 )
		return false;
	if ( m_uCurrentID>m_uMaxID )
		return false;

	static const int iBufSize = 32;
	char * sRes = NULL;

	sphSleepMsec ( m_tParams.m_iRangedThrottle );

	//////////////////////////////////////////////
	// range query with $start/$end interpolation
	//////////////////////////////////////////////

	assert ( m_uMinID>0 );
	assert ( m_uMaxID>0 );
	assert ( m_uMinID<=m_uMaxID );
	assert ( sQuery );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	SphDocID_t uNextID = Min ( m_uCurrentID + m_tParams.m_iRangeStep - 1, m_uMaxID );
	snprintf ( sValues[0], iBufSize, DOCID_FMT, m_uCurrentID );
	snprintf ( sValues[1], iBufSize, DOCID_FMT, uNextID );
	m_uCurrentID = 1 + uNextID;

	// OPTIMIZE? things can be precalculated
	const char * sCur = sQuery;
	int iLen = 0;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<MACRO_COUNT; i++ )
				if ( strncmp ( MACRO_VALUES[i], sCur, strlen ( MACRO_VALUES[i] ) )==0 )
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
	sCur = sQuery;

	char * sDst = sRes;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<MACRO_COUNT; i++ )
				if ( strncmp ( MACRO_VALUES[i], sCur, strlen ( MACRO_VALUES[i] ) )==0 )
			{
				strcpy ( sDst, sValues[i] ); // NOLINT
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

/// setup them ranges (called both for document range-queries and MVA range-queries)
bool CSphSource_SQL::SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError )
{
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
			LOC_ERROR ( "sql_query_range: min_id='%s': must be positive 32/64-bit unsigned integer", sCol0 );
		if ( m_uMaxID<=0 )
			LOC_ERROR ( "sql_query_range: max_id='%s': must be positive 32/64-bit unsigned integer", sCol1 );
		if ( m_uMinID>m_uMaxID )
			LOC_ERROR2 ( "sql_query_range: min_id='%s', max_id='%s': min_id must be less than max_id", sCol0, sCol1 );
	}

	SqlDismissResult ();
	return true;
}


/// issue main rows fetch query
bool CSphSource_SQL::IterateHitsStart ( CSphString & sError )
{
	assert ( m_bSqlConnected );

	m_iNullIds = false;
	m_iMaxIds = false;

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

	for ( ;; )
	{
		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			// run range-query; setup ranges
			if ( !SetupRanges ( m_tParams.m_sQueryRange.cstr(), m_tParams.m_sQuery.cstr(), "sql_query_range: ", sError ) )
				return false;

			// issue query
			m_uCurrentID = m_uMinID;
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
			m_tParams.m_iRangeStep = 0;
		}
		break;
	}

	// some post-query setup
	m_tSchema.Reset();

	for ( int i=0; i<SPH_MAX_FIELDS; i++ )
		m_dUnpack[i] = SPH_UNPACK_NONE;

	int iCols = SqlNumFields() - 1; // skip column 0, which must be the id

	CSphVector<bool> dFound;
	dFound.Resize ( m_tParams.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, dFound )
		dFound[i] = false;

	// map plain attrs from SQL
	for ( int i=0; i<iCols; i++ )
	{
		const char * sName = SqlFieldName ( i+1 );
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

		ARRAY_FOREACH ( j, m_tParams.m_dFileFields )
		{
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dFileFields[j].cstr() ) )
				tCol.m_bFilename = true;
		}

		tCol.m_iIndex = i+1;
		tCol.m_eWordpart = SPH_WORDPART_WHOLE;

		bool bPrefix = m_iMinPrefixLen > 0 && IsPrefixMatch ( tCol.m_sName.cstr () );
		bool bInfix = m_iMinInfixLen > 0 && IsInfixMatch ( tCol.m_sName.cstr () );

		if ( bPrefix && m_iMinPrefixLen > 0 && bInfix && m_iMinInfixLen > 0 )
			LOC_ERROR ( "field '%s' is marked for both infix and prefix indexing", tCol.m_sName.cstr() );

		if ( bPrefix )
			tCol.m_eWordpart = SPH_WORDPART_PREFIX;

		if ( bInfix )
			tCol.m_eWordpart = SPH_WORDPART_INFIX;

		if ( tCol.m_eAttrType==SPH_ATTR_NONE || tCol.m_bIndexed )
		{
			m_tSchema.m_dFields.Add ( tCol );
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
					int iIndex = m_tSchema.m_dFields.GetLength() - 1;
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
			m_tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
	}

	// map multi-valued attrs
	ARRAY_FOREACH ( i, m_tParams.m_dAttrs )
	{
		const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[i];
		if ( ( tAttr.m_eAttrType & SPH_ATTR_MULTI ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			m_tSchema.AddAttr ( tAttr, true ); // all attributes are dynamic at indexing time
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
		if ( ( tCol.m_eAttrType & SPH_ATTR_MULTI ) && tCol.m_eSrc==SPH_ATTRSRC_FIELD )
			m_dAttrToFieldMVA.Add ( iFieldMVA++ );
		else
			m_dAttrToFieldMVA.Add ( -1 );
	}

	m_dFieldMVAs.Resize ( iFieldMVA );
	ARRAY_FOREACH ( i, m_dFieldMVAs )
		m_dFieldMVAs[i].Reserve ( 16 );

	// joined fields
	m_tSchema.m_iBaseFields = m_tSchema.m_dFields.GetLength();

	CSphColumnInfo tCol;
	tCol.m_iIndex = -1;
	ARRAY_FOREACH ( i, m_tParams.m_dJoinedFields )
	{
		tCol.m_sName = m_tParams.m_dJoinedFields[i].m_sName;
		tCol.m_sQuery = m_tParams.m_dJoinedFields[i].m_sQuery;
		tCol.m_bPayload = m_tParams.m_dJoinedFields[i].m_bPayload;
		m_tSchema.m_dFields.Add ( tCol );
	}

	// alloc storage
	m_tDocInfo.Reset ( m_tSchema.GetRowSize() );
	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );

	// check it
	if ( m_tSchema.m_dFields.GetLength()>SPH_MAX_FIELDS )
		LOC_ERROR2 ( "too many fields (fields=%d, max=%d)",
			m_tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );

	return true;
}

#undef LOC_ERROR
#undef LOC_ERROR2
#undef LOC_SQL_ERROR


void CSphSource_SQL::Disconnect ()
{
	if ( m_iNullIds )
		sphWarn ( "source %s: skipped %d document(s) with zero/NULL ids", m_tSchema.m_sName.cstr(), m_iNullIds );

	if ( m_iMaxIds )
		sphWarn ( "source %s: skipped %d document(s) with DOCID_MAX ids", m_tSchema.m_sName.cstr(), m_iMaxIds );

	m_iNullIds = 0;
	m_iMaxIds = 0;

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
			if ( !RunQueryStep ( m_tParams.m_sQuery.cstr(), sError ) )
			{
				// if there's a message, there's an error
				// otherwise, we're just over
				if ( !sError.IsEmpty() )
				{
					m_tDocInfo.m_iDocID = 1; // 0 means legal eof
					return NULL;
				}

			} else
			{
				// step went fine; try to fetch
				bGotRow = SqlFetchRow ();
				continue;
			}

			SqlDismissResult ();

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
		m_tDocInfo.m_iDocID = VerifyID ( sphToDocid ( SqlColumn(0) ) );
		m_uMaxFetchedID = Max ( m_uMaxFetchedID, m_tDocInfo.m_iDocID );
	} while ( !m_tDocInfo.m_iDocID );

	// cleanup attrs
	for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
		m_tDocInfo.m_pDynamic[i] = 0;

	// split columns into fields and attrs
	for ( int i=0; i<m_tSchema.m_iBaseFields; i++ )
	{
		// get that field
		#if USE_ZLIB
		if ( m_dUnpack[i]!=SPH_UNPACK_NONE )
		{
			m_dFields[i] = (BYTE*) SqlUnpackColumn ( i, m_dUnpack[i] );
			continue;
		}
		#endif
		m_dFields[i] = (BYTE*) SqlColumn ( m_tSchema.m_dFields[i].m_iIndex );
	}

	int iFieldMVA = 0;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i); // shortcut

		if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
		{
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
			if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
				ParseFieldMVA ( m_dFieldMVAs, iFieldMVA++, SqlColumn ( tAttr.m_iIndex ) );

			continue;
		}

		switch ( tAttr.m_eAttrType )
		{
			case SPH_ATTR_ORDINAL:
			case SPH_ATTR_STRING:
			case SPH_ATTR_WORDCOUNT:
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
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToInt64 ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
				break;

			default:
				// just store as uint by default
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToDword ( SqlColumn ( tAttr.m_iIndex ) ) ); // FIXME? report conversion errors maybe?
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

	#define LOC_SQL_ERROR(_msg) { sSqlError = _msg; break; }

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
			// setup ranges
			sPrefix.SetSprintf ( "multi-valued attr '%s' ranged query: ", tAttr.m_sName.cstr() );
			if ( !SetupRanges ( tAttr.m_sQueryRange.cstr(), tAttr.m_sQuery.cstr(), sPrefix.cstr(), sError ) )
				return false;

			// run first step (in order to report errors)
			m_uCurrentID = m_uMinID;
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


bool CSphSource_SQL::IterateMultivaluedNext ()
{
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iMultiAttr );

	assert ( m_bSqlConnected );
	assert ( tAttr.m_eAttrType & SPH_ATTR_MULTI );

	// fetch next row
	bool bGotRow = SqlFetchRow ();
	while ( !bGotRow )
	{
		if ( SqlIsError() )
			sphDie ( "sql_fetch_row: %s", SqlError() ); // FIXME! this should be reported

		if ( tAttr.m_eSrc!=SPH_ATTRSRC_RANGEDQUERY )
			return false;

		CSphString sTmp;
		if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sTmp ) ) // FIXME! this should be reported
			return false;

		bGotRow = SqlFetchRow ();
		continue;
	}

	// return that tuple
	const CSphAttrLocator & tLoc = m_tSchema.GetAttr ( m_iMultiAttr ).m_tLocator;
	m_tDocInfo.m_iDocID = sphToDocid ( SqlColumn(0) );
	m_tDocInfo.SetAttr ( tLoc, sphToDword ( SqlColumn(1) ) );
	return true;
}


bool CSphSource_SQL::IterateFieldMVAStart ( int iAttr, CSphString & )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	if ( m_dAttrToFieldMVA[iAttr]==-1 )
		return false;

	m_iFieldMVA = iAttr;
	m_iFieldMVAIterator = 0;

	return true;
}


bool CSphSource_SQL::IterateFieldMVANext ()
{
	int iFieldMVA = m_dAttrToFieldMVA [m_iFieldMVA];
	if ( m_iFieldMVAIterator>=m_dFieldMVAs[iFieldMVA].GetLength() )
		return false;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iFieldMVA );
	m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dFieldMVAs[iFieldMVA][m_iFieldMVAIterator] );

	++m_iFieldMVAIterator;

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


bool CSphSource_SQL::IterateKillListNext ( SphDocID_t & tDocId )
{
	if ( SqlFetchRow () )
		tDocId = sphToDocid ( SqlColumn(0) );
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
		sphWarn ( "failed to unpack column '%s', error=%d, docid=" DOCID_FMT, SqlFieldName(iIndex), iError, m_tDocInfo.m_iDocID );
	}
}


#if !USE_ZLIB

const char * CSphSource_SQL::SqlUnpackColumn ( int iFieldIndex, ESphUnpackFormat )
{
	return SqlColumn ( m_tSchema.m_dFields[iFieldIndex].m_iIndex );
}

#else

const char * CSphSource_SQL::SqlUnpackColumn ( int iFieldIndex, ESphUnpackFormat eFormat )
{
	int iIndex = m_tSchema.m_dFields[iFieldIndex].m_iIndex;
	const char * pData = SqlColumn(iIndex);

	if ( pData==NULL || pData[0]==0 )
		return pData;

	CSphVector<char> & tBuffer = m_dUnpackBuffers[iFieldIndex];
	switch ( eFormat )
	{
		case SPH_UNPACK_MYSQL_COMPRESS:
		{
			unsigned long uSize = 0;
			for ( int i=0; i<4; i++ )
				uSize += (unsigned long)pData[i] << ( 8*i );
			uSize &= 0x3FFFFFFF;

			if ( uSize > m_tParams.m_uUnpackMemoryLimit )
			{
				if ( !m_bUnpackOverflow )
				{
					m_bUnpackOverflow = true;
					sphWarn ( "failed to unpack '%s', column size limit exceeded. size=%u", SqlFieldName ( iIndex ), uSize );
				}
				return NULL;
			}

			int iResult;
			tBuffer.Resize ( uSize + 1 );
			iResult = uncompress ( (Bytef *)&tBuffer[0], &uSize, (Bytef *)pData + 4, SqlColumnLength(iIndex) );
			if ( iResult==Z_OK )
			{
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
			tStream.avail_in = SqlColumnLength(iIndex);
			tStream.next_in = (Bytef *)SqlColumn(iIndex);

			iResult = inflateInit ( &tStream );
			if ( iResult!=Z_OK )
				return NULL;

			for ( ;; )
			{
				tStream.next_out = (Bytef *)&tBuffer[iBufferOffset];
				tStream.avail_out = tBuffer.GetLength() - iBufferOffset - 1;

				iResult = inflate ( &tStream, Z_NO_FLUSH );
				if ( iResult==Z_STREAM_END )
				{
					tBuffer [ tStream.total_out ] = 0;
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


bool CSphSource_SQL::IterateJoinedHits ( CSphString & sError )
{
	// eof check
	if ( m_iJoinedHitField>=m_tSchema.m_dFields.GetLength() )
	{
		m_tDocInfo.m_iDocID = 0;
		return true;
	}

	// my buffer
	const int HITS_THRESH = 1048576/sizeof(CSphWordHit);
	m_dHits.Reserve ( 1024 );
	m_dHits.Resize ( 0 );

	// my fetch loop
	while ( m_dHits.GetLength()<HITS_THRESH )
	{
		if ( SqlFetchRow() )
		{
			// next row
			m_tDocInfo.m_iDocID = sphToDocid ( SqlColumn(0) ); // FIXME! handle conversion errors and zero/max values?

			// field start? restart ids
			if ( !m_iJoinedHitID )
				m_iJoinedHitID = m_tDocInfo.m_iDocID;

			// docid asc requirement violated? report an error
			if ( m_iJoinedHitID>m_tDocInfo.m_iDocID )
			{
				sError.SetSprintf ( "joined field '%s': query MUST return document IDs in ASC order",
					m_tSchema.m_dFields[m_iJoinedHitField].m_sName.cstr() );
				return false;
			}

			// next document? update tracker, reset position
			if ( m_iJoinedHitID<m_tDocInfo.m_iDocID )
			{
				m_iJoinedHitID = m_tDocInfo.m_iDocID;
				m_iJoinedHitPos = 0;
			}

			// build those hits
			BYTE * pText = (BYTE *) SqlColumn(1);
			if ( m_tSchema.m_dFields[m_iJoinedHitField].m_bPayload )
			{
				DWORD uPosition = sphToDword ( SqlColumn(2) );
				if ( !BuildHits ( &pText, m_iJoinedHitField, uPosition, sError ) )
					return false;
			} else
			{
				if ( !BuildHits ( &pText, m_iJoinedHitField, m_iJoinedHitPos, sError ) )
					return false;

				// update current position
				if ( m_dHits.GetLength() )
					m_iJoinedHitPos = HIT2POS ( m_dHits.Last().m_iWordPos );
			}
		} else if ( SqlIsError() )
		{
			// error while fetching row
			sError = SqlError();
			return false;

		} else
		{
			// current field is over, continue to next field
			if ( m_iJoinedHitField<0 )
				m_iJoinedHitField = m_tSchema.m_iBaseFields;
			else
				m_iJoinedHitField++;

			// eof check
			if ( m_iJoinedHitField>=m_tSchema.m_dFields.GetLength() )
			{
				m_tDocInfo.m_iDocID = ( m_dHits.GetLength() ? 1 : 0 ); // to eof or not to eof
				return true;
			}

			// start fetching next field
			SqlDismissResult ();

			if ( !SqlQuery ( m_tSchema.m_dFields[m_iJoinedHitField].m_sQuery.cstr() ) )
			{
				sError = SqlError();
				return false;
			}

			const int iExpected = m_tSchema.m_dFields[m_iJoinedHitField].m_bPayload ? 3 : 2;
			if ( SqlNumFields()!=iExpected )
			{
				const char * sName = m_tSchema.m_dFields[m_iJoinedHitField].m_sName.cstr();
				sError.SetSprintf ( "joined field '%s': query MUST return exactly %d columns, got %d", sName, iExpected, SqlNumFields() );
				return false;
			}

			m_iJoinedHitID = 0;
			m_iJoinedHitPos = 0;
		}
	}

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
	: CSphSource_SQL	( sName )
	, m_pMysqlResult	( NULL )
	, m_pMysqlFields	( NULL )
	, m_tMysqlRow		( NULL )
	, m_pMysqlLengths	( NULL )
{
	m_bCanUnpack = true;
}


void CSphSource_MySQL::SqlDismissResult ()
{
	if ( !m_pMysqlResult )
		return;

	mysql_free_result ( m_pMysqlResult );
	m_pMysqlResult = NULL;
	m_pMysqlFields = NULL;
	m_pMysqlLengths = NULL;
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

	if ( !m_sSslKey.IsEmpty() || !m_sSslCert.IsEmpty() || !m_sSslCA.IsEmpty() )
		mysql_ssl_set ( &m_tMysqlDriver, m_sSslKey.cstr(), m_sSslCert.cstr(), m_sSslCA.cstr(), NULL, NULL );

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


DWORD CSphSource_MySQL::SqlColumnLength ( int iIndex )
{
	if ( !m_pMysqlResult )
		return 0;

	if ( !m_pMysqlLengths )
		m_pMysqlLengths = mysql_fetch_lengths ( m_pMysqlResult );

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


bool CSphSource_PgSQL::IterateHitsStart ( CSphString & sError )
{
	bool bResult = CSphSource_SQL::IterateHitsStart ( sError );
	if ( !bResult )
		return false;

	int iMaxIndex = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.GetAttr(i).m_iIndex );

	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.m_dFields[i].m_iIndex );

	m_dIsColumnBool.Resize ( iMaxIndex + 1 );
	ARRAY_FOREACH ( i, m_dIsColumnBool )
		m_dIsColumnBool[i] = false;

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		m_dIsColumnBool [ m_tSchema.GetAttr(i).m_iIndex ] = m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_BOOL;

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

	const char * szValue = PQgetvalue ( m_pPgResult, m_iPgRow, iIndex );
	if ( m_dIsColumnBool.GetLength() && m_dIsColumnBool[iIndex] && szValue[0]=='t' && !szValue[1] )
		return "1";

	return szValue;
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


DWORD CSphSource_PgSQL::SqlColumnLength ( int iIndex )
{
	return 0;
}

#endif // USE_PGSQL

/////////////////////////////////////////////////////////////////////////////
// XMLPIPE
/////////////////////////////////////////////////////////////////////////////

CSphSource_XMLPipe::CSphSource_XMLPipe ( BYTE * dInitialBuf, int iBufLen, const char * sName )
	: CSphSource	( sName )
	, m_iBufferSize	( 1048576 )
	, m_bEOF		( false )
	, m_bWarned		( false )
	, m_iInitialBufLen ( iBufLen )
{
	assert ( m_iBufferSize > iBufLen );

	m_pTag = NULL;
	m_iTagLength = 0;
	m_pPipe = NULL;
	m_pBuffer = NULL;
	m_pBufferEnd = NULL;
	m_sBuffer = new BYTE [m_iBufferSize];

	if ( iBufLen )
		memcpy ( m_sBuffer, dInitialBuf, iBufLen );
}


CSphSource_XMLPipe::~CSphSource_XMLPipe ()
{
	Disconnect ();
	SafeDeleteArray ( m_sBuffer );
}


void CSphSource_XMLPipe::Disconnect ()
{
	m_iInitialBufLen = 0;

	m_tSchema.Reset ();

	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}
}


bool CSphSource_XMLPipe::Setup ( FILE * pPipe, const char * sCommand )
{
	assert ( sCommand );
	m_pPipe = pPipe;
	m_sCommand = sCommand;
	return true;
}


bool CSphSource_XMLPipe::Connect ( CSphString & )
{
	m_bEOF = false;
	m_bWarned = false;

	m_tSchema.m_dFields.Reset ();
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "title" ) );
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "body" ) );

	CSphColumnInfo tGid ( "gid", SPH_ATTR_INTEGER );
	CSphColumnInfo tTs ( "ts", SPH_ATTR_TIMESTAMP );
	m_tSchema.AddAttr ( tGid, true ); // all attributes are dynamic at indexing time
	m_tSchema.AddAttr ( tTs, true ); // all attributes are dynamic at indexing time

	m_tDocInfo.Reset ( m_tSchema.GetRowSize() );

	m_pBuffer = m_iInitialBufLen > 0 ? m_sBuffer : NULL;
	m_pBufferEnd = m_pBuffer ? m_pBuffer + m_iInitialBufLen : NULL;

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "xmlpipe(%s)", m_sCommand.cstr() );
	m_tSchema.m_sName = sBuf;

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

	SphAttr_t uVal;
	if ( !ScanInt ( "group", &uVal, sError ) ) uVal = 1; m_tDocInfo.SetAttr ( m_tSchema.GetAttr(0).m_tLocator, uVal );
	if ( !ScanInt ( "timestamp", &uVal, sError ) ) uVal = 1; m_tDocInfo.SetAttr ( m_tSchema.GetAttr(1).m_tLocator, uVal );

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
			AddHitFor ( m_pDict->GetWordID ( sWord ), iPos );
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

	while ( !bBodyEnd )
	{
		p = m_pBuffer;
		while ( p<m_pBufferEnd && !bBodyEnd )
		{
			BYTE * pBufTemp = p;
			BYTE * pEndTemp = (BYTE *)szBodyEnd;
			while ( pBufTemp < m_pBufferEnd && *pEndTemp && *pBufTemp==*pEndTemp )
			{
				++pBufTemp;
				++pEndTemp;
			}

			if ( !*pEndTemp )
				bBodyEnd = true;
			else
				p++;
		}

		if ( !bFirstPass )
			break;

		bFirstPass = false;

		if ( !bBodyEnd )
			UpdateBuffer ();
	}

	if ( !bBodyEnd )
	{
		if ( !m_bWarned )
		{
			sphWarn ( "xmlpipe: encountered body larger than %d bytes while scanning docid=" DOCID_FMT " body", m_iBufferSize, m_tDocInfo.m_iDocID );
			m_bWarned = true;
		}
	}

	m_pTokenizer->SetBuffer ( m_pBuffer, p-m_pBuffer );

	// tokenize
	BYTE * sWord;
	while ( ( sWord = m_pTokenizer->GetToken () )!=NULL )
		AddHitFor ( m_pDict->GetWordID ( sWord ), HIT_PACK ( 1, ++m_iWordPos ) );

	m_pBuffer = p;

	SetTag ( "body" );

	// some tag was found
	if ( bBodyEnd )
	{
		// let's check if it's '</body>' which is the only allowed tag at this point
		if ( !SkipTag ( false, sError ) )
			return false;
	} else
	{
		// search for '</body>' tag
		bool bFound = false;

		while ( !bFound )
		{
			while ( m_pBuffer < m_pBufferEnd && *m_pBuffer!='<' )
				++m_pBuffer;

			BYTE * pBufferTmp = m_pBuffer;
			if ( m_pBuffer < m_pBufferEnd )
			{
				if ( !SkipTag ( false, sError ) )
				{
					if ( m_bEOF )
						return false;
					else
					{
						if ( m_pBuffer==pBufferTmp )
							m_pBuffer = pBufferTmp + 1;
					}
				} else
					bFound = true;
			} else
				if ( !UpdateBuffer () )
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
		? ( ( m_pBuffer[0]=='<' )
			&& ( m_pBuffer[m_iTagLength+1]=='>' )
			&& strncmp ( (char*)(m_pBuffer+1), m_pTag, m_iTagLength )==0 )
		: ( ( m_pBuffer[0]=='<' )
			&& ( m_pBuffer[1]=='/' )
			&& ( m_pBuffer[m_iTagLength+2]=='>' )
			&& strncmp ( (char*)(m_pBuffer+2), m_pTag, m_iTagLength )==0 );
	if ( !bOk )
	{
		char sGot[64];
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
		while ( isdigit ( *m_pBuffer ) && m_pBuffer<m_pBufferEnd )
			(*pRes) = 10*(*pRes) + (int)( (*m_pBuffer++)-'0' );

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
					CSphSource_XMLPipe2 ( BYTE * dInitialBuf, int iBufLen, const char * sName, int iFieldBufferMax, bool bFixupUTF8 );
					~CSphSource_XMLPipe2 ();

	bool			Setup ( FILE * pPipe, const CSphConfigSection & hSource );			///< memorize the command
	virtual bool	Connect ( CSphString & sError );			///< run the command and open the pipe
	virtual void	Disconnect ();								///< close the pipe

	virtual bool	IterateHitsStart ( CSphString & ) { return true; }	///< Connect() starts getting documents automatically, so this one is empty
	virtual BYTE **	NextDocument ( CSphString & sError );			///< parse incoming chunk and emit some hits

	virtual bool	HasAttrsConfigured ()							{ return true; }	///< xmlpipe always has some attrs for now
	virtual bool	IterateMultivaluedStart ( int, CSphString & )	{ return false; }
	virtual bool	IterateMultivaluedNext ()						{ return false; }
	virtual bool	IterateFieldMVAStart ( int iAttr, CSphString & sError );
	virtual bool	IterateFieldMVANext ();
	virtual bool	IterateKillListStart ( CSphString & );
	virtual bool	IterateKillListNext ( SphDocID_t & tDocId );


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
	bool			m_bInKillList;
	bool			m_bInId;
	bool			m_bFirstTagAfterDocset;

	int				m_iKillListIterator;
	CSphVector < SphDocID_t > m_dKillList;

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

	int				m_iInitialBufSize;

	int				m_iFieldBufferMax;
	BYTE * 			m_pFieldBuffer;
	int				m_iFieldBufferLen;

	bool			m_bFixupUTF8;		///< whether to replace invalid utf-8 codepoints with spaces
	int				m_iReparseStart;	///< utf-8 fixerupper might need to postpone a few bytes, starting at this offset
	int				m_iReparseLen;		///< and this much bytes (under 4)

	const char *	DecorateMessage ( const char * sTemplate, ... );
	const char *	DecorateMessageVA ( const char * sTemplate, va_list ap );

	void			ConfigureAttrs ( const CSphVariant * pHead, DWORD uAttrType );
	void			ConfigureFields ( const CSphVariant * pHead );
	void			AddFieldToSchema ( const char * szName );
	void			SetupFieldMatch ( CSphColumnInfo & tCol );
	void			UnexpectedCharaters ( const char * pCharacters, int iLen, const char * szComment );

#if USE_LIBEXPAT
	bool			ParseNextChunk ( int iBufferLen, CSphString & sError );
#endif

#if USE_LIBXML
	int				ParseNextChunk ( CSphString & sError );
#endif

	void DocumentError ( const char * sWhere )
	{
		Error ( "malformed source, <sphinx:document> found inside %s", sWhere );

		// Ideally I'd like to display a notice on the next line that
		// would say where exactly it's allowed. E.g.:
		//
		// <sphinx:document> must be contained in <sphinx:docset>
	}
};


#if USE_LIBEXPAT
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

#endif

#if USE_LIBXML
int	xmlReadBuffers ( void * context, char * buffer, int len )
{
	CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) context;
	return pSource->ReadBuffer ( (BYTE*)buffer, len );
}

void xmlErrorHandler ( void * arg, const char * msg, xmlParserSeverities severity, xmlTextReaderLocatorPtr locator )
{
	if ( severity==XML_PARSER_SEVERITY_ERROR )
	{
		int iLine = xmlTextReaderLocatorLineNumber ( locator );
		CSphSource_XMLPipe2 * pSource = (CSphSource_XMLPipe2 *) arg;
		pSource->Error ( "%s (line=%d)", msg, iLine );
	}
}
#endif


CSphSource_XMLPipe2::CSphSource_XMLPipe2 ( BYTE * dInitialBuf, int iBufLen, const char * sName, int iFieldBufferMax, bool bFixupUTF8 )
	: CSphSource_Document ( sName )
	, m_pCurDocument	( NULL )
	, m_pPipe			( NULL )
	, m_iElementDepth	( 0 )
	, m_iBufferSize		( 1048576 )
	, m_bRemoveParsed	( false )
	, m_bInDocset		( false )
	, m_bInSchema		( false )
	, m_bInDocument		( false )
	, m_bInKillList		( false )
	, m_bInId			( false )
	, m_bFirstTagAfterDocset	( false )
	, m_iKillListIterator		( 0 )
	, m_iMVA			( 0 )
	, m_iMVAIterator	( 0 )
	, m_iCurField		( -1 )
	, m_iCurAttr		( -1 )
	, m_pParser			( NULL )
#if USE_LIBXML
	, m_pBufferPtr			( NULL )
	, m_pBufferEnd			( NULL )
	, m_bPassedBufferEnd	( false )
#endif
	, m_iInitialBufSize	( iBufLen )
	, m_iFieldBufferLen	( 0 )
	, m_bFixupUTF8		( bFixupUTF8 )
	, m_iReparseStart	( 0 )
	, m_iReparseLen		( 0 )
{
	assert ( m_iBufferSize > iBufLen );

	m_pBuffer = new BYTE [m_iBufferSize];
	m_iFieldBufferMax = Max ( iFieldBufferMax, 65536 );
	m_pFieldBuffer = new BYTE [ m_iFieldBufferMax ];

	if ( iBufLen )
		memcpy ( m_pBuffer, dInitialBuf, iBufLen );

	m_iInitialBufSize = iBufLen;
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

	m_iInitialBufSize = 0;
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

	snprintf ( sBuf, sizeof(sBuf), "source '%s': ", m_tSchema.m_sName.cstr() );
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

		snprintf ( szBufStart, iLeft, " (line=%d, pos=%d, docid=" DOCID_FMT ")",
			(int)XML_GetCurrentLineNumber ( m_pParser ), (int)XML_GetCurrentColumnNumber ( m_pParser ),
			uFailedID );
	}
#endif

#if USE_LIBXML
	if ( m_pParser )
	{
		SphDocID_t uFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			uFailedID = m_dParsedDocuments.Last()->m_iDocID;

		snprintf ( szBufStart, iLeft, " (docid=" DOCID_FMT ")", uFailedID );
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
		char * pColon = strchr ( const_cast<char*> ( tCol.m_sName.cstr() ), ':' );
		if ( pColon )
		{
			*pColon = '\0';

			if ( uAttrType==SPH_ATTR_INTEGER )
			{
				int iBits = strtol ( pColon+1, NULL, 10 );
				if ( iBits<=0 || iBits>ROWITEM_BITS )
				{
					sphWarn ( "%s", DecorateMessage ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), iBits ) );
					iBits = -1;
				}

				tCol.m_tLocator.m_iBitCount = iBits;
			} else
				sphWarn ( "%s", DecorateMessage ( "attribute '%s': bitcount is only supported for integer types", tCol.m_sName.cstr() ) );
		}

		tCol.m_iIndex = m_tSchema.GetAttrsCount ();

		if ( uAttrType==SPH_ATTR_MULTI )
		{
			tCol.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
			tCol.m_eSrc = SPH_ATTRSRC_FIELD;
		}

		m_tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
	}
}


void CSphSource_XMLPipe2::ConfigureFields ( const CSphVariant * pHead )
{
	for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
	{
		CSphString sFieldName = pCur->cstr ();

		bool bFound = false;
		for ( int i = 0; i < m_tSchema.m_dFields.GetLength () && !bFound; i++ )
			bFound = m_tSchema.m_dFields[i].m_sName==sFieldName;

		if ( bFound )
			sphWarn ( "%s", DecorateMessage ( "duplicate field '%s'", sFieldName.cstr () ) );
		else
			AddFieldToSchema ( sFieldName.cstr () );
	}
}


bool CSphSource_XMLPipe2::Setup ( FILE * pPipe, const CSphConfigSection & hSource )
{
	m_pPipe = pPipe;

	m_tSchema.Reset ();

	m_sCommand = hSource["xmlpipe_command"].cstr ();

	ConfigureAttrs ( hSource("xmlpipe_attr_uint"),			SPH_ATTR_INTEGER );
	ConfigureAttrs ( hSource("xmlpipe_attr_timestamp"),		SPH_ATTR_TIMESTAMP );
	ConfigureAttrs ( hSource("xmlpipe_attr_str2ordinal"),	SPH_ATTR_ORDINAL );
	ConfigureAttrs ( hSource("xmlpipe_attr_bool"),			SPH_ATTR_BOOL );
	ConfigureAttrs ( hSource("xmlpipe_attr_float"),			SPH_ATTR_FLOAT );
	ConfigureAttrs ( hSource("xmlpipe_attr_bigint"),		SPH_ATTR_BIGINT );
	ConfigureAttrs ( hSource("xmlpipe_attr_multi"),			SPH_ATTR_MULTI );
	ConfigureAttrs ( hSource("xmlpipe_attr_string"),		SPH_ATTR_STRING );
	ConfigureAttrs ( hSource("xmlpipe_attr_wordcount"),		SPH_ATTR_WORDCOUNT );
	ConfigureAttrs ( hSource("xmlpipe_field_string"),		SPH_ATTR_STRING );
	ConfigureAttrs ( hSource("xmlpipe_field_wordcount"),	SPH_ATTR_WORDCOUNT );

	m_tDocInfo.Reset ( m_tSchema.GetRowSize () );

	ConfigureFields ( hSource("xmlpipe_field") );
	ConfigureFields ( hSource("xmlpipe_field_string") );
	ConfigureFields ( hSource("xmlpipe_field_wordcount") );

	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );

	return true;
}


void CSphSource_XMLPipe2::SetupFieldMatch ( CSphColumnInfo & tCol )
{
	bool bPrefix = m_iMinPrefixLen > 0 && IsPrefixMatch ( tCol.m_sName.cstr () );
	bool bInfix = m_iMinInfixLen > 0 && IsInfixMatch ( tCol.m_sName.cstr () );

	if ( bPrefix && m_iMinPrefixLen > 0 && bInfix && m_iMinInfixLen > 0 )
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
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		SetupFieldMatch ( m_tSchema.m_dFields[i] );

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
	m_pBufferPtr = m_pBuffer;
	m_pBufferEnd = m_pBuffer + m_iInitialBufSize;
	m_bPassedBufferEnd = false;

	m_dAttrs.Reserve ( 16 );
	m_dAttrs.Resize ( 0 );

	m_pParser = xmlReaderForIO ( (xmlInputReadCallback)xmlReadBuffers, NULL, this, NULL, NULL, 0 );
	if ( !m_pParser )
	{
		sError.SetSprintf ( "xmlpipe: failed to create XML parser" );
		return false;
	}

	xmlTextReaderSetErrorHandler ( m_pParser, xmlErrorHandler, this );
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

	m_iMVA = 0;
	m_iMVAIterator = 0;

	m_sError = "";

#if USE_LIBEXPAT
	int iBytesRead = m_iInitialBufSize;
	iBytesRead += fread ( m_pBuffer + m_iInitialBufSize, 1, m_iBufferSize - m_iInitialBufSize, m_pPipe );

	if ( !ParseNextChunk ( iBytesRead, sError ) )
		return false;
#endif

#if USE_LIBXML
	if ( ParseNextChunk ( sError )==-1 )
		return false;
#endif

	m_dAttrToMVA.Resize ( 0 );

	int iFieldMVA = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount (); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr ( i );
		if ( ( tCol.m_eAttrType & SPH_ATTR_MULTI ) && tCol.m_eSrc==SPH_ATTRSRC_FIELD )
			m_dAttrToMVA.Add ( iFieldMVA++ );
		else
			m_dAttrToMVA.Add ( -1 );
	}

	m_dFieldMVAs.Resize ( iFieldMVA );
	ARRAY_FOREACH ( i, m_dFieldMVAs )
		m_dFieldMVAs[i].Reserve ( 16 );

	return true;
}


#if USE_LIBXML
int CSphSource_XMLPipe2::ParseNextChunk ( CSphString & sError )
{
	int iRet = xmlTextReaderRead ( m_pParser );
	while ( iRet==1 )
	{
		ProcessNode ( m_pParser );
		if ( !m_sError.IsEmpty () )
		{
			sError = m_sError;
			m_tDocInfo.m_iDocID = 1;
			return false;
		}

		if ( m_bPassedBufferEnd )
			break;

		iRet = xmlTextReaderRead ( m_pParser );
	}

	m_bPassedBufferEnd = false;

	if ( !m_sError.IsEmpty () || iRet==-1 )
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
				for ( i=0; i<iBytes; i++ )
					p[i] = ' ';
			}

			// only move forward by the amount of succesfully processed bytes!
			p += i;
		}
	}

	if ( XML_Parse ( m_pParser, (const char*) m_pBuffer, iBufferLen, bLast )!=XML_STATUS_OK )
	{
		SphDocID_t uFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			uFailedID = m_dParsedDocuments.Last()->m_iDocID;

		sError.SetSprintf ( "source '%s': XML parse error: %s (line=%d, pos=%d, docid=" DOCID_FMT ")",
			m_tSchema.m_sName.cstr(), XML_ErrorString ( XML_GetErrorCode ( m_pParser ) ),
			XML_GetCurrentLineNumber ( m_pParser ), XML_GetCurrentColumnNumber ( m_pParser ),
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
		SafeDelete ( m_dParsedDocuments[0] );
		m_dParsedDocuments.RemoveFast ( 0 );
		m_bRemoveParsed = false;
	}

	int iReadResult = 0;

#if USE_LIBEXPAT
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
#endif

#if USE_LIBXML
	while ( m_dParsedDocuments.GetLength()==0 && ( iReadResult = ParseNextChunk ( sError ) )==1 );
#endif

	while ( m_dParsedDocuments.GetLength()!=0 )
	{
		Document_t * pDocument = m_dParsedDocuments[0];
		int nAttrs = m_tSchema.GetAttrsCount ();

		// docid
		m_tDocInfo.m_iDocID = VerifyID ( pDocument->m_iDocID );
		if ( m_tDocInfo.m_iDocID==0 )
		{
			SafeDelete ( m_dParsedDocuments[0] );
			m_dParsedDocuments.RemoveFast ( 0 );
			continue;
		}

		// attributes
		int iFieldMVA = 0;
		for ( int i = 0; i < nAttrs; i++ )
		{
			const CSphString & sAttrValue = pDocument->m_dAttrs[i].IsEmpty () && m_dDefaultAttrs.GetLength () ? m_dDefaultAttrs[i] : pDocument->m_dAttrs[i];
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( i );

			if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
			{
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				ParseFieldMVA ( m_dFieldMVAs, iFieldMVA++, sAttrValue.cstr () );
				continue;
			}

			switch ( tAttr.m_eAttrType )
			{
				case SPH_ATTR_ORDINAL:
				case SPH_ATTR_STRING:
				case SPH_ATTR_WORDCOUNT:
					m_dStrAttrs[i] = sAttrValue.cstr ();
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

				default:
					m_tDocInfo.SetAttr ( tAttr.m_tLocator, sphToDword ( sAttrValue.cstr () ) );
					break;
			}
		}

		m_bRemoveParsed = true;

		int nFields = m_tSchema.m_dFields.GetLength ();
		if ( !nFields )
		{
			m_tDocInfo.m_iDocID = 0;
			return NULL;
		}

		m_dFieldPtrs.Resize ( nFields );
		for ( int i = 0; i < nFields; ++i )
			m_dFieldPtrs[i] = (BYTE*)( pDocument->m_dFields [i].cstr() );

		return (BYTE **)&( m_dFieldPtrs[0] );
	}

	if ( !iReadResult )
		m_tDocInfo.m_iDocID = 0;

	return NULL;
}


bool CSphSource_XMLPipe2::IterateFieldMVAStart ( int iAttr, CSphString & )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	if ( m_dAttrToMVA[iAttr]==-1 )
		return false;

	m_iMVA = iAttr;
	m_iMVAIterator = 0;

	return true;
}


bool CSphSource_XMLPipe2::IterateFieldMVANext ()
{
	int iFieldMVA = m_dAttrToMVA [m_iMVA];
	if ( m_iMVAIterator>=m_dFieldMVAs[iFieldMVA].GetLength() )
		return false;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iMVA );
	m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dFieldMVAs [iFieldMVA][m_iMVAIterator] );

	++m_iMVAIterator;

	return true;
}


bool CSphSource_XMLPipe2::IterateKillListStart ( CSphString & )
{
	m_iKillListIterator = 0;
	return true;
}


bool CSphSource_XMLPipe2::IterateKillListNext ( SphDocID_t & tDocId )
{
	if ( m_iKillListIterator>=m_dKillList.GetLength () )
		return false;

	tDocId = m_dKillList [ m_iKillListIterator++ ];
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
		CSphColumnInfo Info;
		CSphString sDefault;
		bool bIsAttr = false;

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
			{
				AddFieldToSchema ( dAttrs[1] );
				Info.m_sName = dAttrs[1];
			} else if ( !strcmp ( *dAttrs, "attr" ) )
			{
				bIsAttr = true;
				if ( !strcmp ( dAttrs[1], "string" ) )
					Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( dAttrs[1], "wordcount" ) )
					Info.m_eAttrType = SPH_ATTR_WORDCOUNT;

			} else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];

			dAttrs += 2;
		}

		if ( bIsAttr )
		{
			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
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
				Info.m_tLocator.m_iBitCount = strtol ( dAttrs[1], NULL, 10 );
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
				else if ( !strcmp ( szType, "bigint" ) )		Info.m_eAttrType = SPH_ATTR_BIGINT;
				else if ( !strcmp ( szType, "string" ) )		Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( szType, "wordcount" ) )		Info.m_eAttrType = SPH_ATTR_WORDCOUNT;
				else if ( !strcmp ( szType, "multi" ) )
				{
					Info.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
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
			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
		}

		return;
	}

	if ( !strcmp ( szName, "sphinx:document" ) )
	{
		if ( !m_bInDocset || m_bInSchema )
			return DocumentError ( "<sphinx:schema>" );

		if ( m_bInKillList )
			return DocumentError ( "<sphinx:killlist>" );

		if ( m_bInDocument )
			return DocumentError ( "<sphinx:document>" );

		if ( m_tSchema.m_dFields.GetLength()==0 && m_tSchema.GetAttrsCount()==0 )
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

		if ( m_pCurDocument->m_iDocID==0 )
			Error ( "attribute 'id' required in <sphinx:document>" );

		return;
	}

	if ( !strcmp ( szName, "sphinx:killlist" ) )
	{
		if ( !m_bInDocset || m_bInDocument || m_bInSchema )
		{
			Error ( "<sphinx:killlist> is not allowed inside <sphinx:schema> or <sphinx:document>" );
			return;
		}

		m_bInKillList = true;
		return;
	}

	if ( m_bInKillList )
	{
		if ( !m_bInId )
		{
			if ( strcmp ( szName, "id" ) )
			{
				Error ( "only 'id' is allowed inside <sphinx:killlist>" );
				return;
			}

			m_bInId = true;
		} else
			++m_iElementDepth;
	}

	if ( m_bInDocument )
	{
		if ( m_iCurField==-1 && m_iCurAttr==-1 )
		{
			for ( int i = 0; i < m_tSchema.m_dFields.GetLength () && m_iCurField==-1; i++ )
				if ( m_tSchema.m_dFields[i].m_sName==szName )
					m_iCurField = i;

			for ( int i = 0; i < m_tSchema.GetAttrsCount () && m_iCurAttr==-1; i++ )
				if ( m_tSchema.GetAttr(i).m_sName==szName )
					m_iCurAttr = i;

			if ( m_iCurAttr==-1 && m_iCurField==-1 )
			{
				bool bInvalidFound = false;
				for ( int i = 0; i < m_dInvalid.GetLength () && !bInvalidFound; i++ )
					bInvalidFound = m_dInvalid[i]==szName;

				if ( !bInvalidFound )
				{
					sphWarn ( "%s", DecorateMessage ( "unknown field/attribute '%s'; ignored", szName ) );
					m_dInvalid.Add ( szName );
				}
			}
		} else
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

	} else if ( !strcmp ( szName, "sphinx:document" ) )
	{
		m_bInDocument = false;
		if ( m_pCurDocument )
			m_dParsedDocuments.Add ( m_pCurDocument );
		m_pCurDocument = NULL;

	} else if ( !strcmp ( szName, "sphinx:killlist" ) )
	{
		m_bInKillList = false;

	} else if ( m_bInKillList )
	{
		if ( m_iElementDepth==0 )
		{
			if ( m_bInId )
			{
				m_pFieldBuffer [ Min ( m_iFieldBufferLen, m_iFieldBufferMax-1 ) ] = '\0';
				m_dKillList.Add ( sphToDocid ( (const char *)m_pFieldBuffer ) );
				m_iFieldBufferLen = 0;
				m_bInId = false;
			}
		} else
			m_iElementDepth--;

	} else if ( m_bInDocument && ( m_iCurAttr!=-1 || m_iCurField!=-1 ) )
	{
		if ( m_iElementDepth==0 )
		{
			if ( m_iCurField!=-1 )
			{
				assert ( m_pCurDocument );
				m_pCurDocument->m_dFields [m_iCurField].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
			}
			if ( m_iCurAttr!=-1 )
			{
				assert ( m_pCurDocument );
				m_pCurDocument->m_dAttrs [m_iCurAttr].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
			}

			m_iFieldBufferLen = 0;

			m_iCurAttr = -1;
			m_iCurField = -1;
		} else
			m_iElementDepth--;
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
#if USE_LIBEXPAT
		sWarning.SetBinary ( pCharacters, Min ( iLen, MAX_WARNING_LENGTH ) );
		sphWarn ( "source '%s': unexpected string '%s' (line=%d, pos=%d) %s",
			m_tSchema.m_sName.cstr(), sWarning.cstr (),
			XML_GetCurrentLineNumber ( m_pParser ), XML_GetCurrentColumnNumber ( m_pParser ), szComment );
#endif

#if USE_LIBXML
		int i = 0;
		for ( i=0; i<iLen && sphIsSpace ( pCharacters[i] ); i++ );
		sWarning.SetBinary ( pCharacters + i, Min ( iLen - i, MAX_WARNING_LENGTH ) );
		for ( i=iLen-i-1; i>=0 && sphIsSpace ( sWarning.cstr()[i] ); i-- );
		if ( i>=0 )
			( (char *)sWarning.cstr() )[i+1] = '\0';

		sphWarn ( "source '%s': unexpected string '%s' %s", m_tSchema.m_sName.cstr(), sWarning.cstr(), szComment );
#endif
	}
}


void CSphSource_XMLPipe2::Characters ( const char * pCharacters, int iLen )
{
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
		const CSphString & sName = ( m_iCurField!=-1 ) ? m_tSchema.m_dFields[m_iCurField].m_sName : m_tSchema.GetAttr ( m_iCurAttr ).m_sName;

		bool bWarned = false;
		for ( int i = 0; i < m_dWarned.GetLength () && !bWarned; i++ )
			bWarned = m_dWarned[i]==sName;

		if ( !bWarned )
		{
#if USE_LIBEXPAT
			sphWarn ( "source '%s': field/attribute '%s' length exceeds max length (line=%d, pos=%d, docid=" DOCID_FMT ")",
				m_tSchema.m_sName.cstr(), sName.cstr(),
				XML_GetCurrentLineNumber ( m_pParser ), XML_GetCurrentColumnNumber ( m_pParser ),
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
		memmove ( m_pBuffer, m_pBufferPtr, iLeft );
		size_t iRead = fread ( m_pBuffer + iLeft, 1, m_iBufferSize - iLeft, m_pPipe );

		m_bPassedBufferEnd = ( ( m_iBufferSize - iLeft )==int(iRead) );

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
				if ( xmlTextReaderMoveToFirstAttribute ( pReader )!=1 )
					return;

				do
				{
					int iLen = m_dAttrs.GetLength ();
					m_dAttrs.Resize ( iLen + 2 );
					m_dAttrs[iLen] = (char*)xmlTextReaderName ( pReader );
					m_dAttrs[iLen+1] = (char*)xmlTextReaderValue ( pReader );
				}
				while ( xmlTextReaderMoveToNextAttribute ( pReader )==1 );
			}

			int iLen = m_dAttrs.GetLength ();
			m_dAttrs.Resize ( iLen + 2 );
			m_dAttrs[iLen] = NULL;
			m_dAttrs[iLen+1] = NULL;

			StartElement ( szName, &m_dAttrs[0] );
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

CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, BYTE * dInitialBuf, int iBufLen, const char * szSourceName, int iMaxFieldLen )
{
	CSphSource_XMLPipe2 * pXMLPipe = new CSphSource_XMLPipe2 ( dInitialBuf, iBufLen, szSourceName, iMaxFieldLen, pSource->GetInt ( "xmlpipe_fixup_utf8", 0 )!=0 );
	if ( !pXMLPipe->Setup ( pPipe, *pSource ) )
		SafeDelete ( pXMLPipe );

	return pXMLPipe;
}

#endif


FILE * sphDetectXMLPipe ( const char * szCommand, BYTE * dBuf, int & iBufSize, int iMaxBufSize, bool & bUsePipe2 )
{
	bUsePipe2 = true; // default is xmlpipe2

	FILE * pPipe = popen ( szCommand, "r" );
	if ( !pPipe )
		return NULL;

	BYTE * pStart = dBuf;
	iBufSize = (int)fread ( dBuf, 1, iMaxBufSize, pPipe );
	BYTE * pEnd = pStart + iBufSize;

	// BOM
	if ( iBufSize>=3 )
		if ( !strncmp ( (char*)pStart, "\xEF\xBB\xBF", 3 ) )
			pStart += 3;

	while ( isspace ( *pStart ) && pStart < pEnd )
		pStart++;

	if ( ( pEnd - pStart)>=5 )
		bUsePipe2 = !strncasecmp ( (char *)pStart, "<?xml", 5 );

	return pPipe;
}


#if USE_ODBC

CSphSourceParams_ODBC::CSphSourceParams_ODBC ()
	: m_bWinAuth	( false )
	, m_bUnicode	( false )
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
		SQLCloseCursor ( m_hStmt );
		SQLFreeHandle ( SQL_HANDLE_STMT, m_hStmt );
		m_hStmt = NULL;
	}
}


bool CSphSource_ODBC::SqlQuery ( const char * sQuery )
{
	if ( SQLAllocHandle ( SQL_HANDLE_STMT, m_hDBC, &m_hStmt )==SQL_ERROR )
		return false;

	if ( SQLExecDirect ( m_hStmt, (SQLCHAR *)sQuery, SQL_NTS )==SQL_ERROR )
		return false;

	SQLSMALLINT nCols = 0;
	m_nResultCols = 0;
	if ( SQLNumResultCols ( m_hStmt, &nCols )==SQL_ERROR )
		return false;

	m_nResultCols = nCols;

	const int MAX_NAME_LEN = 512;
	char szColumnName[MAX_NAME_LEN];

	m_dColumns.Resize ( m_nResultCols );
	for ( int i = 0; i<m_nResultCols; ++i )
	{
		QueryColumn_t & tCol = m_dColumns[i];

		SQLULEN uColSize = 0;
		SQLSMALLINT iNameLen = 0;
		SQLSMALLINT iDataType = 0;
		if ( SQLDescribeCol ( m_hStmt, (SQLUSMALLINT)(i+1), (SQLCHAR*)szColumnName, MAX_NAME_LEN, &iNameLen, &iDataType, &uColSize, NULL, NULL )==SQL_ERROR )
			return false;

		int iBuffLen = DEFAULT_COL_SIZE;
		if ( uColSize && (int)uColSize<MAX_COL_SIZE )
			iBuffLen = uColSize+1;

		tCol.m_dContents.Resize ( iBuffLen );
		tCol.m_dRaw.Resize ( iBuffLen );
		tCol.m_sName = szColumnName;
		tCol.m_iInd = 0;
		tCol.m_iBufferSize = iBuffLen;
		tCol.m_bUnicode = m_bUnicode && ( iDataType==SQL_WCHAR || iDataType==SQL_WVARCHAR || iDataType==SQL_WLONGVARCHAR );

		if ( SQLBindCol ( m_hStmt, (SQLUSMALLINT)(i+1),
			tCol.m_bUnicode ? SQL_UNICODE : SQL_C_CHAR,
			tCol.m_bUnicode ? &(tCol.m_dRaw[0]) : &(tCol.m_dContents[0]),
			iBuffLen, &(tCol.m_iInd) )==SQL_ERROR )
				return false;
	}

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
	if ( SQLAllocHandle ( SQL_HANDLE_ENV, NULL, &m_hEnv )==SQL_ERROR )
		return false;

	SQLSetEnvAttr ( m_hEnv, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, SQL_IS_INTEGER );

	if ( SQLAllocHandle ( SQL_HANDLE_DBC, m_hEnv, &m_hDBC )==SQL_ERROR )
		return false;

	OdbcPostConnect ();

	char szOutConn [2048];
	SQLSMALLINT iOutConn = 0;
	if ( SQLDriverConnect ( m_hDBC, NULL, (SQLTCHAR*) m_sOdbcDSN.cstr(), SQL_NTS, (SQLCHAR*)szOutConn, sizeof(szOutConn), &iOutConn, SQL_DRIVER_NOPROMPT )==SQL_ERROR )
	{
		GetSqlError ( SQL_HANDLE_DBC, m_hDBC );
		return false;
	}

	return true;
}


void CSphSource_ODBC::SqlDisconnect ()
{
	if ( m_hStmt!=NULL )
		SQLFreeHandle ( SQL_HANDLE_STMT, m_hStmt );

	if ( m_hDBC )
	{
		SQLDisconnect ( m_hDBC );
		SQLFreeHandle ( SQL_HANDLE_DBC, m_hDBC );
	}

	if ( m_hEnv )
		SQLFreeHandle ( SQL_HANDLE_ENV, m_hEnv );
}


int CSphSource_ODBC::SqlNumFields ()
{
	if ( !m_hStmt )
		return -1;

	return m_nResultCols;
}


bool CSphSource_ODBC::SqlFetchRow ()
{
	SQLRETURN iRet = SQLFetch ( m_hStmt );
	if ( iRet==SQL_ERROR )
	{
		GetSqlError ( SQL_HANDLE_STMT, m_hStmt );
		return false;
	}

	ARRAY_FOREACH ( i, m_dColumns )
	{
		QueryColumn_t & tCol = m_dColumns[i];
		switch ( tCol.m_iInd )
		{
			case SQL_NO_DATA:
			case SQL_NULL_DATA:
				tCol.m_dContents[0] = '\0';
				tCol.m_dContents[0] = '\0';
				break;

			default:
#if USE_WINDOWS // FIXME! support UCS-2 columns on Unix too
				if ( tCol.m_bUnicode )
				{
					int iConv = WideCharToMultiByte ( CP_UTF8, 0, LPCWSTR ( &tCol.m_dRaw[0] ), tCol.m_iInd/sizeof(WCHAR),
						LPSTR ( &tCol.m_dContents[0] ), tCol.m_iBufferSize-1, NULL, NULL );

					if ( iConv==0 )
						if ( GetLastError()==ERROR_INSUFFICIENT_BUFFER )
							iConv = tCol.m_iBufferSize-1;

					tCol.m_dContents[iConv] = '\0';

				} else
#endif
				{
					tCol.m_dContents[tCol.m_iInd] = '\0';
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


DWORD CSphSource_ODBC::SqlColumnLength ( int )
{
	return 0;
}


bool CSphSource_ODBC::Setup ( const CSphSourceParams_ODBC & tParams )
{
	if ( !CSphSource_SQL::Setup ( tParams ) )
		return false;

	m_sOdbcDSN = tParams.m_sOdbcDSN;
	m_bWinAuth = tParams.m_bWinAuth;
	m_bUnicode = tParams.m_bUnicode;

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "odbc%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}


void CSphSource_ODBC::GetSqlError ( SQLSMALLINT iHandleType, SQLHANDLE hHandle )
{
	char szState[16];
	char szMessageText[1024];
	SQLINTEGER iError;
	SQLSMALLINT iLen;
	SQLGetDiagRec ( iHandleType, hHandle, 1, (SQLCHAR*)szState, &iError, (SQLCHAR*)szMessageText, 1024, &iLen );
	m_sError = szMessageText;
}

//////////////////////////////////////////////////////////////////////////

void CSphSource_MSSQL::OdbcPostConnect ()
{
	const int MAX_LEN = 1024;
	char szDriver[MAX_LEN];
	char szDriverAttrs[MAX_LEN];
	SQLSMALLINT iDescLen = 0;
	SQLSMALLINT iAttrLen = 0;
	SQLSMALLINT iDir = SQL_FETCH_FIRST;

	CSphString sDriver;
	for ( ;; )
	{
		SQLRETURN iRet = SQLDrivers ( m_hEnv, iDir, (SQLCHAR*)szDriver, MAX_LEN, &iDescLen, (SQLCHAR*)szDriverAttrs, MAX_LEN, &iAttrLen );
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

/////////////////////////////////////////////////////////////////////////////
// MERGER HELPERS
/////////////////////////////////////////////////////////////////////////////

void CSphDocMVA::Read ( CSphReader & tReader )
{
	m_iDocID = tReader.GetDocid();
	if ( m_iDocID )
	{
		ARRAY_FOREACH ( i, m_dMVA )
		{
			DWORD iValues = tReader.GetDword();
			m_dMVA[i].Resize ( iValues );
			if ( iValues )
				tReader.GetBytes ( &m_dMVA[i][0], iValues*sizeof(DWORD) );
		}
	}
}

void CSphDocMVA::Write ( CSphWriter & tWriter )
{
	tWriter.PutDocid ( m_iDocID );
	ARRAY_FOREACH ( i, m_dMVA )
	{
		m_dOffsets[i] = ( DWORD )tWriter.GetPos() / sizeof( DWORD );

		int iValues = m_dMVA[i].GetLength();
		tWriter.PutDword ( iValues );
		if ( iValues )
			tWriter.PutBytes ( &m_dMVA[i][0], iValues*sizeof(DWORD) );
	}
}

/////////////////////////////////////////////////////////////////////////////

void sphSetQuiet ( bool bQuiet )
{
	g_bSphQuiet = bQuiet;
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
			snprintf ( sBuf, sizeof(sBuf), "collected %d docs, %.1f MB", m_iDocuments,
				float(m_iBytes)/1000000.0f );
			break;

		case PHASE_SORT:
			snprintf ( sBuf, sizeof(sBuf), "sorted %.1f Mhits, %.1f%% done", float(m_iHits)/1000000,
				GetPercent ( m_iHits, m_iHitsTotal ) );
			break;

		case PHASE_COLLECT_MVA:
			snprintf ( sBuf, sizeof(sBuf), "collected "INT64_FMT" attr values", m_iAttrs );
			break;

		case PHASE_SORT_MVA:
			snprintf ( sBuf, sizeof(sBuf), "sorted %.1f Mvalues, %.1f%% done", float(m_iAttrs)/1000000,
				GetPercent ( m_iAttrs, m_iAttrsTotal ) );
			break;

		case PHASE_MERGE:
			snprintf ( sBuf, sizeof(sBuf), "merged %.1f Kwords", float(m_iWords)/1000 );
			break;

		case PHASE_PREREAD:
			snprintf ( sBuf, sizeof(sBuf), "read %.1f of %.1f MB, %.1f%% done",
				float(m_iBytes)/1000000.0f, float(m_iBytesTotal)/1000000.0f,
				GetPercent ( m_iBytes, m_iBytesTotal ) );
			break;

		case PHASE_PRECOMPUTE:
			snprintf ( sBuf, sizeof(sBuf), "indexing attributes, %d.%d%% done", m_iDone/10, m_iDone%10 );
			break;

		default:
			assert ( 0 && "internal error: unhandled progress phase" );
			snprintf ( sBuf, sizeof(sBuf), "(progress-phase-%d)", m_ePhase );
			break;
	}

	sBuf[sizeof(sBuf)-1] = '\0';
	return sBuf;
}

int CSphStrHashFunc::Hash ( const CSphString & sKey )
{
	return sKey.IsEmpty() ? 0 : sphCRC32 ( (const BYTE *)sKey.cstr() );
}


void CSphQueryResultMeta::AddStat ( const CSphString & sWord, int iDocs, int iHits, bool bUntouched )
{
	WordStat_t * pStats = m_hWordStats ( sWord );
	if ( !pStats )
	{
		CSphQueryResult::WordStat_t tStats;
		tStats.m_iDocs = iDocs;
		tStats.m_iHits = iHits;
		tStats.m_bUntouched = bUntouched;
		m_hWordStats.Add ( tStats, sWord );
	} else
	{
		pStats->m_iDocs += iDocs;
		pStats->m_iHits += iHits;
		pStats->m_bUntouched = bUntouched;
	}
}

//
// $Id$
//
