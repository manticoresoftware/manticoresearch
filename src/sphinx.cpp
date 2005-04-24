//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxstem.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#if USE_WINDOWS
	#include <io.h> // for open()

	// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
	#define snprintf	_snprintf
	#define sphSeek		_lseeki64
#else
	#include <unistd.h>
	#include <sys/time.h>

	#define sphSeek		lseek
#endif

#if USE_MYSQL
#include <mysql/mysql.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

/// compile time error struct
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

namespace Private
{
	template<int x>		struct static_assert_test{};
	template<bool x>	struct SizeError;
	template<>			struct SizeError<true>{};
}

#define STATIC_SIZE_ASSERT(_Type,_ExpSize)	\
	typedef ::Private::static_assert_test<sizeof(::Private::SizeError \
	< (bool) (sizeof(_Type) == (_ExpSize)) >)> static_assert_typedef_

/////////////////////////////////////////////////////////////////////////////

STATIC_SIZE_ASSERT ( SphOffset_t, 8 );
STATIC_SIZE_ASSERT ( CSphHit, 16 );

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
	}

	void Start ()
	{
		m_fStamp -= sphLongTimer ();
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
	g_dTimers[0].Stop ();
}


void sphProfilerShow ( int iTimer=0, int iLevel=0 )
{
	assert ( g_iTimers==0 );
	assert ( g_iTimer==0 );

	if ( iTimer==0 )
		fprintf ( stderr, "--- PROFILE ---\n" );

	CSphTimer & tTimer = g_dTimers[iTimer];
	int iChild;

	// calc me
	int iChildren = 0;
	float fSelf = tTimer.m_fStamp;
	for ( iChild=tTimer.m_iChild; iChild>0; iChild=g_dTimers[iChild].m_iNext, iChildren++ )
		fSelf -= g_dTimers[iChild].m_fStamp;

	// dump me
	for ( int i=0; i<iLevel; i++ )
		fprintf ( stderr, "  " );
	fprintf ( stderr, "%s: %.2f", g_dTimerNames [ tTimer.m_eTimer ], tTimer.m_fStamp );
	if ( iChildren )
		fprintf ( stderr, ", self: %.2f", fSelf );
	fprintf ( stderr, "\n" );

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
		fprintf ( stderr, "---------------\n" );
}


class CSphEasyTimer
{
public:
	CSphEasyTimer ( ESphTimer eTimer )
		: m_eTimer ( eTimer )
	{
		sphProfilerPush ( m_eTimer );
	}

	~CSphEasyTimer ()
	{
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
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

/// generic tokenizer
class CSphTokenizer
{
public:
	/// max token size
	static const int	TOKEN_SIZE		= 64;

public:
	/// create empty tokenizer
						CSphTokenizer ();

	/// pass next buffer
	void				SetBuffer ( BYTE * sBuffer, int iLength );

	/// get next token
	BYTE *				GetToken ();

protected:
	BYTE *				m_pBuffer;					///< my buffer
	BYTE *				m_pBufferMax;				///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;						///< current position
	BYTE				m_sAccum [ 4+TOKEN_SIZE ];	///< boundary token accumulator
	int					m_iAccum;					///< boundary token size
};

/////////////////////////////////////////////////////////////////////////////

/// possible bin states
enum ESphBinState
{
	BIN_ERR_READ	= -2,	///< bin read error
	BIN_ERR_END		= -1,	///< bin end
	BIN_POS			= 0,	///< bin is in "expects pos delta" state
	BIN_DOC			= 1,	///< bin is in "expects doc delta" state
	BIN_WORD		= 2		///< bin is in "expects word delta" state
};


struct CSphBin
{
	static const int	MIN_SIZE = 8192;
	int					m_iSize;

	BYTE *data, *pData;
	int left, done, state;
	DWORD lastGroupID, lastDocID, lastWordID, lastPos; // FIXME! make it a hit

	SphOffset_t		m_iFilePos;		///< bin offset in raw hits log
	int				m_iFileLeft;	///< how much data is still unread from raw log via this bin

	CSphBin ()
		: m_iFilePos ( 0 )
		, m_iFileLeft ( 0 )
	{
		data = pData = NULL;
		left = done = 0;
		lastDocID = lastWordID = lastPos = 0;
		state = BIN_POS;
	}


	void Init ( int iSize )
	{
		assert ( !data );
		assert ( iSize>=MIN_SIZE );

		m_iSize = iSize;
		data = new BYTE [ iSize ];
		pData = data; 
	}


	~CSphBin ()
	{
		SafeDeleteArray ( data );
	}
};


/// VLN index header
struct CSphIndexHeader_VLN
{
	DWORD		m_iMinDocID;
	DWORD		m_iMinGroupID;
	DWORD		m_iGroupBits;
	DWORD		m_iFieldCount;
};


class CSphWriter_VLN
{
public:
	char *		m_sName;
	SphOffset_t	m_iPos;

public:
				CSphWriter_VLN ( char * sName );
	virtual		~CSphWriter_VLN ();

	int			OpenFile ();
	void		PutBytes ( void *data, int size );
	void		PutRawBytes ( void * pData, int iSize );
	void		CloseFile ();
	void		SeekTo ( SphOffset_t pos );

	void		ZipInt ( DWORD uValue );
	void		ZipOffset ( SphOffset_t uValue );
	void		ZipOffsets ( CSphVector<SphOffset_t> * pData );

private:
	int			m_iFD;
	int			m_iPoolUsed;
	int			m_iPoolOdd;
	BYTE		m_dPool [ SPH_CACHE_WRITE ];
	BYTE *		m_pPool;

	void		PutNibble ( int data );
	void		Flush ();
};


class CSphReader_VLN
{
public:

				CSphReader_VLN ();
	virtual		~CSphReader_VLN ();

	void		SetFile ( int iFD );

	void		SeekTo ( SphOffset_t iPos );

	void		GetRawBytes ( void * pData, int iSize );
	void		GetBytes ( void * data, int size );

	DWORD		UnzipInt ();
	SphOffset_t	UnzipOffset ();
	void		UnzipInts ( CSphVector<DWORD> * pData );
	int			UnzipHits ( CSphVector<DWORD> * pHits );

	const CSphReader_VLN &	operator = ( const CSphReader_VLN & rhs );

private:

	int			m_iFD;
	SphOffset_t	m_iPos;

	int			m_iBufPos;
	int			m_iBufOdd;
	int			m_iBufUsed;
	int			m_iBufSize;
	BYTE *		m_pBuf;

private:
	int			GetNibble ();
	void		UpdateCache ();
};


/// query word from the searcher's point of view
class CSphQueryWord
{
public:
	char *			m_sWord;		///< my copy of word
	int				m_iQueryPos;	///< word position, from query
	DWORD			m_iWordID;		///< word ID, from dictionary

	int				m_iDocs;		///< document count, from wordlist
	int				m_iHits;		///< hit count, from wordlist

	DWORD			m_iDoc;			///< current document, from doclist
	DWORD			m_iHitPos;		///< current hit postition, from hitlist

	SphOffset_t		m_iHitlistPos;	///< current position in hitlist, from doclist

	CSphReader_VLN	m_rdDoclist;	///< my doclist reader
	CSphReader_VLN	m_rdHitlist;	///< my hitlist reader

public:
	CSphQueryWord ()
		: m_sWord ( NULL )
		, m_iQueryPos ( -1 )
		, m_iWordID ( 0 )
		, m_iDocs ( 0 )
		, m_iHits ( 0 )
		, m_iDoc ( 0 )
		, m_iHitPos ( 0 )
		, m_iHitlistPos ( 0 )
	{
	}

	~CSphQueryWord ()
	{	
		sphFree ( m_sWord );
	}

	const CSphQueryWord & operator = ( const CSphQueryWord & rhs )
	{
		m_sWord			= sphDup ( rhs.m_sWord );
		m_iQueryPos		= rhs.m_iQueryPos;
		m_iWordID		= rhs.m_iWordID;
		m_iDocs			= rhs.m_iDocs;
		m_iHits			= rhs.m_iHits;
		m_iDoc			= rhs.m_iDoc;
		m_iHitPos		= rhs.m_iHitPos;
		m_iHitlistPos	= rhs.m_iHitlistPos;
		m_rdDoclist		= rhs.m_rdDoclist;
		m_rdHitlist		= rhs.m_rdHitlist;
		return *this;
	}

	void GetDoclistEntry ()
	{
		DWORD iDeltaDoc = m_rdDoclist.UnzipInt ();
		if ( iDeltaDoc )
		{
			m_iDoc += iDeltaDoc;

			SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset ();
			assert ( iDeltaPos>0 );

			m_iHitlistPos += iDeltaPos;
			m_rdHitlist.SeekTo ( m_iHitlistPos );

		} else
		{
			m_iDoc = 0;
		}
	}

	void GetHitlistEntry ()
	{
		DWORD iDelta = m_rdHitlist.UnzipInt ();
		if ( iDelta )
			m_iHitPos += iDelta;
		else
			m_iHitPos = 0;
	}
};


/// this is my actual VLN-compressed phrase index implementation
struct CSphIndex_VLN : CSphIndex
{
								CSphIndex_VLN ( const char * filename );
	virtual						~CSphIndex_VLN ();

	virtual int					build ( CSphDict * dict, CSphSource * source, int iMemoryLimit );
	virtual CSphQueryResult *	query ( CSphDict * dict, CSphQuery * pQuery );

private:
	char *						m_sFilename;
	int							fdRaw;
	SphOffset_t					m_iFilePos;
	CSphWriter_VLN *			fdIndex;
	CSphWriter_VLN *			fdData;
	CSphVector<CSphBin *>		bins;

	SphOffset_t					cidxPagesDir [ SPH_CLOG_DIR_PAGES ];
	CSphVector<SphOffset_t>		m_dDoclist;

	CSphIndexHeader_VLN			m_tHeader;

	int							OpenFile ( char * ext, int mode );

	int							binsInit ();
	void						binsDone ();
	DWORD						binsReadVLB ( int b );
	int							binsReadByte ( int b );
	int							binsRead ( int b, CSphHit * e );

	int							cidxCreate ();
	int							cidxWriteRawVLB ( int fd, CSphHit * hit, int count );
	void						cidxHit ( CSphHit * hit );
	void						cidxDone ();
};

/////////////////////////////////////////////////////////////////////////////

static BYTE sphLT_cp1251[] =
{
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 0-10
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 10-20
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 20-30
	0x30,0x31,0x32,0x33, 0x34,0x35,0x36,0x37, 0x38,0x39,0,0, 0,0,0,0, // 30-40
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f, // 40-50
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0, 0,0,0,0, // 50-60
	0x00,0x61,0x62,0x63, 0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b, 0x6c,0x6d,0x6e,0x6f, // 60-70
	0x70,0x71,0x72,0x73, 0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0, 0,0,0,0, // 70-80
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 80-90
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 90-a0
	0,0,0,0, 0,0,0,0, 0xb8,0,0,0, 0,0,0,0, // a0-b0
	0,0,0,0, 0,0,0,0, 0xb8,0,0,0, 0,0,0,0, // b0-c0
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef, // c0-d0
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff, // d0-e0
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef, // e0-f0
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff // f0-ff
};

/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

void sphDie(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);
	exit(1);
}


void * sphMalloc ( size_t size )
{
	void * result = ::malloc ( size );
	if ( !result )
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", size );
	return result;
}


void * sphRealloc ( void * ptr, size_t size )
{
	void * result = ::realloc ( ptr, size );
	if ( !result )
		sphDie ( "FATAL: out of memory (unable to reallocate %d bytes).\n", size );
	return result;
}


void sphFree ( void * ptr )
{
	if ( ptr )
		::free ( ptr );
}


void * operator new ( size_t iSize )
{
	return sphMalloc ( iSize );
}


void * operator new [] ( size_t iSize )
{
	return sphMalloc ( iSize );
}


/// time, in seconds
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


char * sphDup ( const char * s )
{
	char * r;
	if ( s )
	{
		r = (char*)sphMalloc ( 1+strlen(s) );
		strcpy ( r, s );
		return r;
	} else
	{
		return NULL;
	}
}


int sphWrite ( int iFD, const void * pBuf, size_t iCount, const char * sName )
{
	int iWritten = ::write ( iFD, pBuf, iCount );
	if ( iWritten!=(int)iCount )
	{
		if ( iWritten<0 )
		{
			fprintf ( stderr, "ERROR: %s: write error: %s.\n",
				sName, strerror(errno) );
		} else
		{
			fprintf ( stderr, "ERROR: %s: write error: %d of %d bytes written.\n",
				sName, iWritten, iCount );
		}
	}
	return iWritten;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC TOKENIZER
/////////////////////////////////////////////////////////////////////////////

CSphTokenizer::CSphTokenizer ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
{
}


void CSphTokenizer::SetBuffer ( BYTE * sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( m_pCur>=m_pBufferMax );
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;

	// do inplace case and non-char removal
	BYTE * p = sBuffer;
	while ( p<m_pBufferMax )
	{
		*p = sphLT_cp1251[*p];
		p++;
	}
}


BYTE * CSphTokenizer::GetToken ()
{
	// flush whatever accumulated from that last buffer
	if ( m_iAccum )
	{
		// if it's not EOF
		if ( m_pCur<m_pBufferMax )
		{
			// accumulate as much extra chars as possible
			while ( m_pCur<m_pBufferMax && *m_pCur && m_iAccum<TOKEN_SIZE )
				m_sAccum [ m_iAccum++ ] = *m_pCur++;

			// through away everything which is over the token size limit
			while ( m_pCur<m_pBufferMax && *m_pCur )
				m_pCur++;

			// if buffer is now over (wow, that's REAL big token),
			// we need to get another bufer
			if ( m_pCur>=m_pBufferMax )
				return NULL;
		}

		// clear and return the accumulated token
		m_sAccum [ m_iAccum ] = '\0';
		m_iAccum = 0;
		return m_sAccum;
	}

	// skip whitespace
	while ( m_pCur<m_pBufferMax && !*m_pCur )
		m_pCur++;
	BYTE * pToken = m_pCur;

	// skip non-whitespace
	while ( m_pCur<m_pBufferMax && *m_pCur )
		m_pCur++;

	// if buffer's not over, we have a full token now
	if ( m_pCur<m_pBufferMax )
	{
		if ( m_pCur-pToken>TOKEN_SIZE )
			pToken [ TOKEN_SIZE ] = '\0';
		return pToken;
	}

	// buffer's over, so we need to accumulate
	m_iAccum = Min ( TOKEN_SIZE, m_pCur-pToken );
	memcpy ( m_sAccum, pToken, m_iAccum );
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////

CSphQuery::CSphQuery ()
	: m_sQuery		( NULL )
	, m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_bAll		( true )
	, m_iGroup		( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE OUTPUT
///////////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_BINARY O_BINARY
#else
#define SPH_BINARY 0
#endif

CSphWriter_VLN::CSphWriter_VLN ( char * sName )
{
	assert ( sName );
	m_sName = sphDup ( sName );

	m_pPool = m_dPool;
	m_iFD = 0;
	m_iPos = 0;
}


CSphWriter_VLN::~CSphWriter_VLN()
{
//	close();
	sphFree ( m_sName );
}


int CSphWriter_VLN::OpenFile ()
{
	if ( m_iFD )
		return 1;
	m_iFD = ::open ( m_sName, O_CREAT | O_RDWR | O_TRUNC | SPH_BINARY, 0644 );
	m_iPoolUsed = 0;
	m_iPoolOdd = 0;
	m_iPos = 0;
	return m_iFD;
}


void CSphWriter_VLN::CloseFile() 
{
	if ( m_iFD )
	{
		Flush ();
		::close ( m_iFD );
		m_iFD = 0;
	}
}


void CSphWriter_VLN::PutNibble ( int data )
{
	data &= 0x0f;
	if ( m_iPoolOdd )
	{
		m_iPoolOdd = 0;
		*m_pPool++ |= data;
		if ( m_iPoolUsed==SPH_CACHE_WRITE )
			Flush ();
	} else
	{
		m_iPoolOdd = 1;
		*m_pPool = (BYTE)(data<<4);
		m_iPoolUsed++;
	}
	m_iPos++;
}


void CSphWriter_VLN::PutBytes ( void * data, int size )
{
	BYTE * b = (BYTE*) data;

	while ( size-->0 )
	{
		PutNibble ( (*b) >> 4 );
		PutNibble ( (*b) & 0x0f );
		b++;
	}
}


void CSphWriter_VLN::PutRawBytes ( void * pData, int iSize )
{
	assert ( !m_iPoolOdd );

	if ( m_iPoolUsed+iSize>SPH_CACHE_WRITE )
		Flush ();
	assert ( m_iPoolUsed+iSize<=SPH_CACHE_WRITE );

	memcpy ( m_pPool, pData, iSize );
	m_pPool += iSize;
	m_iPoolUsed += iSize;
	m_iPos += 2*iSize;
}


void CSphWriter_VLN::ZipInt ( DWORD uValue )
{
	do
	{
		int b = (int)(uValue & 0x07);
		uValue >>= 3;
		if ( uValue )
			b |= 0x08;
		PutNibble ( b );
	} while ( uValue );
}


void CSphWriter_VLN::ZipOffset ( SphOffset_t uValue )
{
	do
	{
		int b = (int)(uValue & 0x07);
		uValue >>= 3;
		if ( uValue )
			b |= 0x08;
		PutNibble ( b );
	} while ( uValue );
}


void CSphWriter_VLN::ZipOffsets ( CSphVector<SphOffset_t> * pData )
{
	assert ( pData );

	SphOffset_t * pValue = &((*pData)[0]);
	int n = pData->GetLength ();

	while ( n-->0 )
	{
		SphOffset_t uValue = *pValue++;
		do
		{
			int b = (int)(uValue & 0x07);
			uValue >>= 3;
			if ( uValue )
				b |= 0x08;
			PutNibble ( b );
		} while ( uValue );
	}
}


void CSphWriter_VLN::Flush ()
{
	PROFILE ( write_hits );

	sphWrite ( m_iFD, m_dPool, m_iPoolUsed, m_sName ); // FIXME! should return fail-code

	if ( m_iPoolOdd )
		m_iPos++;
	m_iPoolUsed = 0;
	m_iPoolOdd = 0;
	m_pPool = m_dPool;
}


void CSphWriter_VLN::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );
	BYTE b;

	Flush ();
	sphSeek ( m_iFD, iPos>>1, SEEK_SET );
	if ( iPos&1 )
	{
		::read ( m_iFD, &b, 1 );
		sphSeek ( m_iFD, iPos>>1, SEEK_SET );
		PutNibble ( b & 0x0f );
	}
	m_iPos = iPos;
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE INPUT
///////////////////////////////////////////////////////////////////////////////

CSphReader_VLN::CSphReader_VLN ()
	: m_iFD ( 0 )
	, m_iPos ( 0 )
	, m_iBufPos ( 0 )
	, m_iBufOdd ( 0 )
	, m_iBufUsed ( 0 )
{
	m_iBufSize = 4096; // FIXME
	m_pBuf = new BYTE [ m_iBufSize ];
}


CSphReader_VLN::~CSphReader_VLN ()
{
	SafeDeleteArray ( m_pBuf );
}


void CSphReader_VLN::SetFile ( int iFD )
{
	assert ( iFD>0 );
	m_iFD = iFD;
	m_iPos = 0;
	m_iBufPos = 0;
	m_iBufOdd = 0;
	m_iBufUsed = 0;
}


void CSphReader_VLN::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );
	if ( iPos>=m_iPos && iPos<m_iPos+2*m_iBufUsed )
	{
		m_iBufPos = (int)( ( iPos>>1 ) - ( m_iPos>>1 ) ); // reposition to proper byte
		m_iBufOdd = ( iPos & 1 ) ? 1 : 0; // reposition to proper nibble
		assert ( m_iBufPos<m_iBufUsed );
	} else
	{
		m_iPos = iPos;
		m_iBufUsed = 0;
	}
}


void CSphReader_VLN::UpdateCache ()
{
	assert ( m_iFD );

	// stream position could be changed externally
	// so let's just hope that the OS optimizes redundant seeks
	SphOffset_t iNewPos = ( m_iPos>>1 ) + m_iBufUsed;
	sphSeek ( m_iFD, iNewPos, SEEK_SET );
	
	m_iBufPos = 0;
	m_iBufUsed = ::read ( m_iFD, m_pBuf, m_iBufSize );
	m_iBufOdd = ( m_iPos & 1 ) ? 1 : 0;
	m_iPos = iNewPos<<1;
}


int CSphReader_VLN::GetNibble ()
{
	if ( m_iBufPos>=m_iBufUsed )
		UpdateCache ();
	assert ( m_iBufPos<m_iBufUsed );

	if ( m_iBufOdd )
	{
		m_iBufOdd = 0;
		return ( m_pBuf [ m_iBufPos++ ] & 0x0f );
	} else
	{
		m_iBufOdd = 1;
		return ( m_pBuf [ m_iBufPos ] >> 4 );
	}
}


void CSphReader_VLN::GetRawBytes ( void * pData, int iSize )
{
	assert ( !m_iBufOdd );

	if ( m_iBufPos+iSize>m_iBufUsed )
		UpdateCache ();
	assert ( (m_iBufPos+iSize)<m_iBufUsed );

	memcpy ( pData, m_pBuf+m_iBufPos, iSize );
	m_iBufPos += iSize;
}


void CSphReader_VLN::GetBytes ( void *data, int size )
{
	BYTE * b = (BYTE*) data;

	while ( size-->0 )
		*b++ = (BYTE)( (GetNibble()<<4) + GetNibble() );
}


DWORD CSphReader_VLN::UnzipInt ()
{
	register int offset = 0;
	register DWORD b, v = 0;

	do
	{
		b = GetNibble ();
		v += ( (b&0x07) << offset );
		offset += 3;
	} while ( b&0x08 );
	return v;
}


SphOffset_t CSphReader_VLN::UnzipOffset ()
{
	register int offset = 0;
	register DWORD b = 0;
	register SphOffset_t v = 0;

	do
	{
		b = GetNibble ();
		v += ( SphOffset_t(b&0x07) << offset );
		offset += 3;
	} while ( b&0x08 );
	return v;
}


void CSphReader_VLN::UnzipInts ( CSphVector<DWORD> * pData )
{
	register DWORD i = 0;
	while (( i=UnzipInt() ))
		pData->Add ( i );
}


int CSphReader_VLN::UnzipHits ( CSphVector<DWORD> * pList )
{
	register int i, v = 0;
	int iStart = pList->GetLength ();

	while (( i=UnzipInt() ))
	{
		v += i;
		pList->Add ( v );
	}

	return pList->GetLength() - iStart;
}


const CSphReader_VLN & CSphReader_VLN::operator = ( const CSphReader_VLN & rhs )
{
	SetFile ( rhs.m_iFD );
	m_iPos = rhs.m_iPos;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// QUERY RESULT
/////////////////////////////////////////////////////////////////////////////

CSphQueryResult::CSphQueryResult ()
{
	for ( int i=0; i<SPH_MAX_QUERY_WORDS; i++ )
	{
		m_tWordStats[i].m_sWord = NULL;
		m_tWordStats[i].m_iDocs = 0;
		m_tWordStats[i].m_iHits = 0;
	}

	m_iNumWords = 0;
	m_fQueryTime = 0.0f;
}


CSphQueryResult::~CSphQueryResult ()
{
	for ( int i=0; i<SPH_MAX_QUERY_WORDS; i++ )
		sphFree ( m_tWordStats[i].m_sWord );
}

/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphIndex * sphCreateIndexPhrase ( const char * sFilename )
{
	return new CSphIndex_VLN ( sFilename );
}


CSphIndex_VLN::CSphIndex_VLN ( const char * sName )
{
	m_sFilename = sphDup ( sName );
	fdIndex = 0;
	fdData = 0;
	fdRaw = 0;
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	sphFree ( m_sFilename );
}


#define SPH_CMPHIT_LESS(a,b) \
	(a.m_iWordID < b.m_iWordID || \
	(a.m_iWordID == b.m_iWordID && a.m_iDocID < b.m_iDocID) || \
	(a.m_iWordID == b.m_iWordID && a.m_iDocID == b.m_iDocID && a.m_iWordPos < b.m_iWordPos))


struct CmpMatch_fn
{
	inline int operator () ( const CSphMatch & a, const CSphMatch & b )
	{
		return a.m_iWeight - b.m_iWeight;
	}
};


/// sort hits by word/document/position
void sphSortHits ( CSphHit * s, int n )
{
	int st0[32], st1[32];
	int a, b, k, i, j;
	CSphHit t;
	DWORD x_word, x_doc, x_pos;

	k = 1;
	st0[0] = 0;
	st1[0] = n - 1;
	while (k != 0) {
		k--;
		i = a = st0[k];
		j = b = st1[k];
		x_word = s[(a+b) / 2].m_iWordID;
		x_doc = s[(a+b) / 2].m_iDocID;
		x_pos = s[(a+b) / 2].m_iWordPos;
//		x = s[(a+b) / 2];
		while (a < b) {
			while (i <= j) {
//				while (cmpHit(&s[i], &x) < 0) i++;
				while (s[i].m_iWordID < x_word ||
					(s[i].m_iWordID == x_word && s[i].m_iDocID < x_doc) ||
					(s[i].m_iWordID == x_word && s[i].m_iDocID == x_doc && s[i].m_iWordPos < x_pos))
						i++;
//				while (cmpHit(&x, &s[j]) < 0) j--;
				while (x_word < s[j].m_iWordID ||
					(x_word == s[j].m_iWordID && x_doc < s[j].m_iDocID) ||
					(x_word == s[j].m_iWordID && x_doc == s[j].m_iDocID && x_pos < s[j].m_iWordPos))
						j--;
				if (i <= j) {
					t = s[i]; s[i] = s[j]; s[j] = t;
					i++; j--;
				}
			}
			if (j - a >= b - i) {
				if (a < j) {
					st0[k] = a;
					st1[k] = j;
					k++;
				}
				a = i;
			} else {
				if (i < b) {
					st0[k] = i;
					st1[k] = b;
					k++;
				}
				b = j;
			}
		}
	}
}

int CSphIndex_VLN::OpenFile ( char * ext, int mode )
{
	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s.%s", m_sFilename, ext );
	return ::open ( sBuf, mode | SPH_BINARY, 0644 );
}

int CSphIndex_VLN::binsInit ()
{
	sphSeek ( fdRaw, 0, SEEK_SET ); // FIXME
	m_iFilePos = 0;
	return 1;
}

void CSphIndex_VLN::binsDone ()
{
	bins.Reset ();
}

int CSphIndex_VLN::binsReadByte(int b)
{
	BYTE r;

	if ( !bins[b]->left )
	{
		PROFILE ( read_hits );
		if ( m_iFilePos!=bins[b]->m_iFilePos )
		{
			sphSeek ( fdRaw, bins[b]->m_iFilePos, SEEK_SET );
			m_iFilePos = bins[b]->m_iFilePos;
		}

		int n = bins[b]->m_iFileLeft > bins[b]->m_iSize
			? bins[b]->m_iSize
			: (int)bins[b]->m_iFileLeft;
		if (n == 0)
		{
			bins[b]->done = 1;
			bins[b]->left = 1;
		} else
		{
			assert ( bins[b]->data );

			if ( ::read ( fdRaw, bins[b]->data, n )!=n )
				return -2;
			bins[b]->left = n;

			bins[b]->m_iFilePos += n;
			bins[b]->m_iFileLeft -= n;
			bins[b]->pData = bins[b]->data;
			m_iFilePos += n;
		}
	}
	if ( bins[b]->done )
		return -1;

	bins[b]->left--;
	r = *(bins[b]->pData);
	bins[b]->pData++;
	return r;
}

DWORD CSphIndex_VLN::binsReadVLB(int b)
{
	DWORD v = 0, o = 0;
	int t;

	do {
		if ((t = binsReadByte(b)) < 0) return 0xffffffff;
		v += ((t & 0x7f) << o);
		o += 7;
	} while (t & 0x80);
	return v;
}

int CSphIndex_VLN::binsRead ( int b, CSphHit * e )
{
	DWORD r;

	// expected EOB
	if ( bins[b]->done )
	{
		e->m_iWordID = 0;
		return 1;
	}

	for ( ;; )
	{
		// unexpected EOB
		if ( (r = binsReadVLB(b)) == 0xffffffffUL )
		{
			return 0;
		}

		if ( r )
		{
			switch ( bins[b]->state )
			{
				case BIN_WORD:
					bins[b]->lastWordID += r;
					bins[b]->lastGroupID = bins[b]->lastDocID = bins[b]->lastPos = 0;
					bins[b]->state = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					bins[b]->lastDocID += r;
					bins[b]->lastPos = 0;
					bins[b]->state = BIN_POS;
					if ( (bins[b]->lastGroupID = binsReadVLB(b)) == 0xffffffffUL )
					{
						return 0; // read unexpected EOB
					}
					break;

				case BIN_POS: 
					bins[b]->lastPos += r;
					e->m_iGroupID = bins[b]->lastGroupID;
					e->m_iDocID = bins[b]->lastDocID;
					e->m_iWordID = bins[b]->lastWordID;
					e->m_iWordPos = bins[b]->lastPos;
					return 1;
			}
		} else
		{
			switch ( bins[b]->state )
			{
				case BIN_POS:	bins[b]->state = BIN_DOC; break;
				case BIN_DOC:	bins[b]->state = BIN_WORD; break;
				case BIN_WORD:
					bins[b]->done = 1;
					e->m_iWordID = 0;
					return 1;
			}
		}
	}

	return 1;
}


int CSphIndex_VLN::cidxCreate ()
{
	char sBuf [ SPH_MAX_FILENAME_LEN ];

	sprintf ( sBuf, "%s.spi", m_sFilename );
	fdIndex = new CSphWriter_VLN ( sBuf );
	if ( !fdIndex->OpenFile() )
		return 0;

	sprintf ( sBuf, "%s.spd", m_sFilename );
	fdData = new CSphWriter_VLN ( sBuf );
	if ( !fdData->OpenFile() )
	{
		SafeDelete ( fdIndex );
		return 0;
	}

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	fdData->PutBytes ( &bDummy, 1 );

	for ( int i=0; i<SPH_CLOG_DIR_PAGES; i++ )
		cidxPagesDir[i] = -1;

	fdIndex->PutRawBytes ( &m_tHeader, sizeof(m_tHeader) );
	fdIndex->PutBytes ( cidxPagesDir, sizeof(cidxPagesDir) );
	return 1;
}


void CSphIndex_VLN::cidxHit ( CSphHit * hit )
{
	static SphOffset_t	iLastDoclistPos = 0;
	static SphOffset_t	iLastHitlistPos = 0;
	static DWORD		iLastDocID = 0;
	static DWORD		iLastWordID = 0;
	static DWORD		iLastPageID = 0xffffffff;
	static DWORD		iLastHitPos = 0;
	static int			iLastWordDocs = 0;
	static int			iLastWordHits = 0;

	assert (
		( hit->m_iWordID && hit->m_iWordPos && hit->m_iDocID ) || // it's either ok hit
		( !hit->m_iWordID && !hit->m_iWordPos ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	if ( iLastWordID!=hit->m_iWordID )
	{
		// close prev hitlist, if any
		if ( iLastHitPos )
		{
			fdData->ZipInt ( 0 );
			iLastHitPos = 0;
		}

		// flush prev doclist, if any
		if ( m_dDoclist.GetLength() )
		{
			// finish writing wordlist entry
			assert ( fdData->m_iPos > iLastDoclistPos );
			assert ( iLastWordDocs );
			assert ( iLastWordHits );

			fdIndex->ZipOffset ( fdData->m_iPos - iLastDoclistPos );
			fdIndex->ZipInt ( iLastWordDocs );
			fdIndex->ZipInt ( iLastWordHits );

			iLastDoclistPos = fdData->m_iPos;
			iLastWordDocs = 0;
			iLastWordHits = 0;

			// write doclist
			fdData->ZipOffsets ( &m_dDoclist );
			fdData->ZipInt ( 0 );
			m_dDoclist.Reset ();

			// restart doclist deltas
			iLastDocID = 0;
			iLastHitlistPos = 0;
		}

		// flush last wordlist
		if ( !hit->m_iWordPos )
		{
			// close wordlist
			fdIndex->ZipInt ( 0 );
			return;
		}

		// close prev wordlist, if next wordlist map page reached
		DWORD iPageID = hit->m_iWordID >> SPH_CLOG_BITS_PAGE;
		if ( iLastPageID!=iPageID )
		{
			// close wordlist
			fdIndex->ZipInt ( 0 );

			// restart wordlist deltas
			iLastWordID = 0; 
			iLastDoclistPos = 0;

			// next map page
			iLastPageID = iPageID;
			cidxPagesDir [ iPageID ] = fdIndex->m_iPos;
		}

		// begin writing wordlist entry
		assert ( hit->m_iWordID > iLastWordID );
		fdIndex->ZipInt ( hit->m_iWordID - iLastWordID );
		iLastWordID = hit->m_iWordID;
	}

	////////////
	// next doc
	////////////

	if ( iLastDocID!=hit->m_iDocID )
	{
		// close prev hitlist, if any
		if ( iLastHitPos )
		{
			fdData->ZipInt ( 0 );
			iLastHitPos = 0;
		}

		// add new doclist entry for new doc id
		assert ( hit->m_iDocID > iLastDocID );
		assert ( fdData->m_iPos > iLastHitlistPos );

		m_dDoclist.Add ( hit->m_iDocID - iLastDocID );
		m_dDoclist.Add ( fdData->m_iPos - iLastHitlistPos );

		iLastDocID = hit->m_iDocID;
		iLastHitlistPos = fdData->m_iPos;

		// update per-word stats
		iLastWordDocs++;
	}

	///////////
	// the hit
	///////////

	// add hit delta
	assert ( hit->m_iWordPos > iLastHitPos );
	fdData->ZipInt ( hit->m_iWordPos - iLastHitPos );
	iLastHitPos = hit->m_iWordPos;
	iLastWordHits++;
}


void CSphIndex_VLN::cidxDone()
{
	fdIndex->SeekTo ( 0 );
	fdIndex->PutRawBytes ( &m_tHeader, sizeof(m_tHeader) );
	fdIndex->PutBytes ( cidxPagesDir, sizeof(cidxPagesDir) );
	
	fdIndex->CloseFile ();
	fdData->CloseFile ();

	SafeDelete ( fdIndex );
	SafeDelete ( fdData );
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

int CSphIndex_VLN::cidxWriteRawVLB ( int fd, CSphHit * hit, int count )
{
	assert ( hit );

	BYTE buf[65536+1024], *pBuf, *maxP;
	int n = 0, w;
	DWORD d1, d2, d3, l1=0, l2=0, l3=0;

	pBuf = &buf[0];
	maxP = &buf[65536-1];
	while ( count-- )
	{
		// calc deltas
		d1 = hit->m_iWordID - l1;
		d2 = hit->m_iDocID - l2;
		d3 = hit->m_iWordPos - l3;

		// non-zero delta restarts all the fields after it
		// because their deltas might now be negative
		if ( d1 ) d2 = hit->m_iDocID;
		if ( d2 ) d3 = hit->m_iWordPos;

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 ) pBuf += encodeVLB ( pBuf, 0 );

		// encode deltas
		if ( d1 ) pBuf += encodeVLB ( pBuf, d1 ); // encode word delta
		if ( d2 )
		{
			pBuf += encodeVLB ( pBuf, d2 ); // encode doc id (whole or delta)
			pBuf += encodeVLB ( pBuf, hit->m_iGroupID ); // encode group id
		}

		assert ( d3 );
		pBuf += encodeVLB ( pBuf, d3 );

		// update current state
		l1 = hit->m_iWordID;
		l2 = hit->m_iDocID;
		l3 = hit->m_iWordPos;

		hit++;

		if ( pBuf>maxP )
		{
			w = (int)(pBuf - buf);
			if ( sphWrite ( fd, buf, w, "rawlog" )!=w )
				return -1;
			n += w;
			pBuf = buf;
		}
	}
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	w = (int)(pBuf - buf);
	if ( sphWrite ( fd, buf, w, "rawlog" )!=w )
		return -1;
	n += w;

	return n;
}

/////////////////////////////////////////////////////////////////////////////

static int iLog2 ( int iMin, int iMax )
{
	if ( iMin<iMax )
	{
		int iBits = 0;
		DWORD iRange = iMax-iMin;
		while ( iRange )
		{
			iBits++;
			iRange >>= 1;
		}
		return iBits;
	} else
	{
		return 0;
	}
}


/// hit priority queue entry
struct CSphHitQueueEntry : public CSphHit
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
	void Push ( CSphHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		m_pData [ m_iUsed ].m_iDocID = tHit.m_iDocID;
		m_pData [ m_iUsed ].m_iGroupID = tHit.m_iGroupID;
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


int CSphIndex_VLN::build ( CSphDict * pDict, CSphSource * pSource, int iMemoryLimit )
{
	PROFILER_INIT ();

	/////////////////
	// build raw log
	/////////////////

	// set dictionary
	pSource->setDict ( pDict );

	// create raw log
	fdRaw = OpenFile ( "spr", O_CREAT | O_RDWR | O_TRUNC );
	if ( !fdRaw )
		return 0;

	// adjust memory requirements
	const int MIN_MEM_LIMIT = sizeof(CSphHit)*1048576;
	bool bRelimit = false;
	int iOldLimit = 0;

	if ( iMemoryLimit<MIN_MEM_LIMIT )
	{
		iOldLimit = iMemoryLimit;
		iMemoryLimit = MIN_MEM_LIMIT;
		bRelimit = true;
	}

	iMemoryLimit = ( ( (iMemoryLimit+32768) >> 16 ) << 16 ); // round to 64K
	int iRawBlockSize = iMemoryLimit / sizeof(CSphHit);

	if ( bRelimit && iOldLimit )
	{
		fprintf ( stderr, "WARNING: collect_hits: mem_limit=%d kb too low, increasing to %d kb.\n",
			iOldLimit/1024, iMemoryLimit/1024 );
	}

	// allocate raw block
	int iRawBlockUsed = 0;
	SphOffset_t iRawHits = 0;
	CSphHit * dRawBlock = new CSphHit [ iRawBlockSize ];
	CSphHit * pRawBlock = dRawBlock;
	assert ( dRawBlock );

	// accumulate group IDs range
	DWORD iMinDocID = INT_MAX;
	DWORD iMaxDocID = 0;
	DWORD iMinGroupID = INT_MAX;
	DWORD iMaxGroupID = 0;

	// build raw log
	PROFILE_BEGIN ( collect_hits );

	CSphIndexProgress tProgress;
	tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;

	DWORD iDocID;
	while (( iDocID = pSource->next() ))
	{
		if ( m_pProgress
			&& ( ( pSource->GetStats()->m_iTotalDocuments % 1000 )==0 ) )
		{
			tProgress.m_iDocuments = pSource->GetStats()->m_iTotalDocuments;
			tProgress.m_iBytes = pSource->GetStats()->m_iTotalBytes;
			m_pProgress ( &tProgress );
		}

		int iHitCount = pSource->hits.GetLength ();
		if ( iHitCount<=0 )
			continue;

		CSphHit * pHit = &pSource->hits[0];
		while ( iHitCount-- )
		{
			assert ( pHit->m_iGroupID );
			assert ( pHit->m_iDocID );
			assert ( pHit->m_iWordID );
			assert ( pHit->m_iWordPos );

			iMinGroupID = Min ( iMinGroupID, pHit->m_iGroupID );
			iMaxGroupID = Max ( iMaxGroupID, pHit->m_iGroupID );
			iMinDocID = Min ( iMinDocID, pHit->m_iDocID );
			iMaxDocID = Max ( iMaxDocID, pHit->m_iDocID );

			*pRawBlock++ = *pHit++;
			iRawBlockUsed++;
			iRawHits++;

			if ( iRawBlockUsed==iRawBlockSize )
			{
				PROFILE_BEGIN ( sort_hits );
				sphSortHits ( dRawBlock, iRawBlockUsed );
				PROFILE_END ( sort_hits );

				PROFILE_BEGIN ( write_hits );
				bins.Add ( new CSphBin() );
				CSphBin * pBin = bins.Last ();
				pBin->m_iFileLeft = cidxWriteRawVLB ( fdRaw, dRawBlock, iRawBlockUsed );
				if ( pBin->m_iFileLeft<0 )
				{
					fprintf ( stderr, "ERROR: write() failed, out of disk space?\n" );
					return 0;
				}
				PROFILE_END ( write_hits );

				iRawBlockUsed = 0;
				pRawBlock = dRawBlock;
			}
		}
	}

	if ( iRawBlockUsed )
	{
		PROFILE_BEGIN ( sort_hits );
		sphSortHits ( dRawBlock, iRawBlockUsed );
		PROFILE_END ( sort_hits );

		PROFILE_BEGIN ( write_hits );
		bins.Add ( new CSphBin() );
		CSphBin * pBin = bins.Last ();
		pBin->m_iFileLeft = cidxWriteRawVLB ( fdRaw, dRawBlock, iRawBlockUsed );
		if ( pBin->m_iFileLeft<0 )
		{
			fprintf ( stderr, "ERROR: write() failed, out of disk space?\n" );
			return 0;
		}
		PROFILE_END ( write_hits );
	}

	if ( m_pProgress )
	{
		tProgress.m_iDocuments = pSource->GetStats()->m_iTotalDocuments;
		tProgress.m_iBytes = pSource->GetStats()->m_iTotalBytes;
		m_pProgress ( &tProgress );

		tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT_END;
		m_pProgress ( &tProgress );
	}

	PROFILE_END ( collect_hits );
	PROFILE_BEGIN ( invert_hits );

	// calc bin positions from their lengths
	ARRAY_FOREACH ( i, bins )
	{
		bins[i]->m_iFilePos = 0;
		if ( i )
			bins[i]->m_iFilePos = bins[i-1]->m_iFilePos + bins[i-1]->m_iFileLeft;
	}

	// deallocate raw block
	SafeDeleteArray ( dRawBlock );

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	// reopen raw log as read-only
	::close ( fdRaw );
	fdRaw = OpenFile ( "spr", O_RDONLY );
	if ( !fdRaw )
		return 0;

	// create new compressed index
	if ( !cidxCreate() )
		return 0;

	// calculate how much bits do we need for encoding gids
	int iGidBits = iLog2 ( iMinGroupID, iMaxGroupID );
	int iDocBits = iLog2 ( iMinDocID, iMaxDocID );
	assert ( (iGidBits+iDocBits)<=32 );

	// fill header
	m_tHeader.m_iMinDocID = iMinDocID;
	m_tHeader.m_iMinGroupID = iMinGroupID;
	m_tHeader.m_iGroupBits = iGidBits;
	m_tHeader.m_iFieldCount = pSource->GetFieldCount ();

	// initialize sorting
	int iRawBlocks = bins.GetLength();
	if ( iRawBlocks>16 )
	{
		fprintf ( stderr, "WARNING: sort_hits: merge_blocks=%d too high, increasing mem_limit may improve performance.\n",
			bins.GetLength() );
	}

	int iBinSize = ( ( iMemoryLimit/iRawBlocks + 2048 ) >> 12 ) << 12; // round to 4k
	if ( iBinSize<CSphBin::MIN_SIZE )
	{
		iBinSize = CSphBin::MIN_SIZE;
		fprintf ( stderr, "WARNING: sort_hits: mem_limit=%d kb too low, increasing to %d kb.\n",
			iMemoryLimit/1024, (iBinSize*iRawBlocks)/1024 );
	}

	ARRAY_FOREACH ( i, bins )
		bins[i]->Init ( iBinSize );

	//////////////
	// final sort
	//////////////

	CSphHitQueue tQueue ( iRawBlocks );
	CSphHit tHit;

	// initial fill
	int * bActive = new int [ iRawBlocks ];
	for ( int i=0; i<iRawBlocks; i++ )
	{
		binsRead ( i, &tHit );
		bActive[i] = tHit.m_iWordID;
		if ( bActive[i] )
			tQueue.Push ( tHit, i );
	}

	// init progress meter
	tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT;
	tProgress.m_iHits = 0;
	tProgress.m_iHitsTotal = iRawHits;

	// while the queue has data for us
	// FIXME! analyze binsRead return code
	int iHitsSorted = 0;
	while ( tQueue.m_iUsed )
	{
		// pack and emit queue root
		tQueue.m_pData->m_iDocID = 1 +
			( ( tQueue.m_pData->m_iDocID - iMinDocID ) << iGidBits ) +
			( tQueue.m_pData->m_iGroupID - iMinGroupID );
		cidxHit ( tQueue.m_pData );

		// pop queue root and push next hit from popped bin
		int iBin = tQueue.m_pData->m_iBin;
		tQueue.Pop ();
		if ( bActive[iBin] )
		{
			binsRead ( iBin, &tHit );
			bActive[iBin] = tHit.m_iWordID;
			if ( bActive[iBin] )
				tQueue.Push ( tHit, iBin );
		}

		// progress
		if ( m_pProgress && ++iHitsSorted==1000000 )
		{
			tProgress.m_iHits += iHitsSorted;
			m_pProgress ( &tProgress );
			iHitsSorted = 0;
		}
	}

	if ( m_pProgress )
	{
		tProgress.m_iHits += iHitsSorted;
		m_pProgress ( &tProgress );

		tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT_END;
		m_pProgress ( &tProgress );
	}

	// cleanup
	SafeDeleteArray ( bActive );

	binsDone ();

	CSphHit tFlush;
	tFlush.m_iDocID = 0;
	tFlush.m_iGroupID = 0;
	tFlush.m_iWordID = 0;
	tFlush.m_iWordPos =0;
	cidxHit ( &tFlush );
	cidxDone ();

	// unlink raw log
	char sBuf [ SPH_MAX_FILENAME_LEN+1 ];
	snprintf ( sBuf, sizeof(sBuf), "%s.spr", m_sFilename );
	unlink ( sBuf );

	PROFILE_END ( invert_hits );
	PROFILER_DONE ();
	PROFILE_SHOW ();
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

struct CSphQueryParser : CSphSource_Text
{
	char *query;
	char *words[SPH_MAX_QUERY_WORDS];
	int numWords;

	CSphQueryParser(CSphDict *dict, const char *query)
	{
		int i;

		for (i = 0; i < SPH_MAX_QUERY_WORDS; i++) words[i] = NULL;
		this->numWords = 0;
		this->query = sphDup(query);
		this->dict = dict;
		m_bCallWordCallback = true;
		this->next();
	}

	~CSphQueryParser()
	{
		int i;

		for (i = 0; i < SPH_MAX_QUERY_WORDS; i++)
			if (words[i]) sphFree(words[i]);
	}

	void wordCallback(char *word)
	{
		if (numWords < SPH_MAX_QUERY_WORDS)
			this->words[numWords++] = sphDup(word);
	}

	BYTE * NextText()
	{
		m_iLastID = 1;
		return (BYTE*)this->query;
	}
};


int cmpQueryWord ( const void * a, const void * b )
{
	return ((CSphQueryWord*)a)->m_iDocs - ((CSphQueryWord*)b)->m_iDocs;
}


/// file which closes automatically when going out of scope
class CSphAutofile
{
public:
	/// my file descriptior
	int m_iFD;

	/// open
	CSphAutofile ( const char * sName )
	{
		m_iFD = ::open ( sName, O_RDONLY | SPH_BINARY );
	}

	/// close
	~CSphAutofile ()
	{
		if ( m_iFD>=0 )
			::close ( m_iFD );
	}
};


CSphQueryResult * CSphIndex_VLN::query ( CSphDict * dict, CSphQuery * pQuery )
{
	assert ( dict );
	assert ( pQuery );

	CSphQueryParser *qp;
	CSphQueryWord qwords[SPH_MAX_QUERY_WORDS];
	int i, j, nwords, weights [ SPH_MAX_FIELD_COUNT ], imin, nweights;
	DWORD wordID, docID, pmin, k;

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	// create result and start timing
	CSphQueryResult * pResult = new CSphQueryResult();
	pResult->m_fQueryTime = -sphLongTimer ();

	// split query into words
	qp = new CSphQueryParser ( dict, pQuery->m_sQuery );
	nwords = qp->hits.GetLength ();
	if (nwords > SPH_MAX_QUERY_WORDS) nwords = SPH_MAX_QUERY_WORDS; // FIXME
	for ( i=0; i<nwords; i++ )
	{
		qwords[i].m_sWord = sphDup ( qp->words[i] );
		qwords[i].m_iWordID = qp->hits[i].m_iWordID;
		qwords[i].m_iQueryPos = 1+i;
	}
	delete qp;

	if ( !nwords )
	{
		pResult->m_fQueryTime += sphLongTimer ();
		return pResult;
	}

	// open files
	char sTmp [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sTmp, sizeof(sTmp), "%s.spi", m_sFilename );
	CSphAutofile tWordlist ( sTmp );

	snprintf ( sTmp, sizeof(sTmp), "%s.spd", m_sFilename );
	CSphAutofile tDoclist ( sTmp );

	if ( tWordlist.m_iFD<0 || tDoclist.m_iFD<0 )
		return NULL;

	PROFILE_END ( query_init );
	PROFILE_BEGIN ( query_load_dir );

	// load index pages directory
	CSphReader_VLN rdIndex;
	rdIndex.SetFile ( tWordlist.m_iFD );
	rdIndex.GetRawBytes ( &m_tHeader, sizeof(m_tHeader) );
	rdIndex.GetBytes ( cidxPagesDir, sizeof(cidxPagesDir) );
	DWORD iGroupMask = (1<<m_tHeader.m_iGroupBits) - 1;

	PROFILE_END ( query_load_dir );

	//////////////////////////////
	// decode words from wordlist
	//////////////////////////////

	PROFILE_BEGIN ( query_load_words );

	for ( i=0; i<nwords; i++ )
	{
		// if the word was already loaded, just link to it
		for ( j=0; j<i; j++ )
			if ( qwords[i].m_iWordID == qwords[j].m_iWordID )
		{
			qwords[i] = qwords[j];
			qwords[i].m_iQueryPos = i;
			break;
		}
		if ( j<i )
			continue;

		// lookup this wordlist page
		// offset might be -1 if page is totally empty
		SphOffset_t iWordlistOffset = cidxPagesDir [ qwords[i].m_iWordID >> SPH_CLOG_BITS_PAGE ];
		if ( iWordlistOffset>0 )
		{
			// set doclist files
			qwords[i].m_rdDoclist.SetFile ( tDoclist.m_iFD );
			qwords[i].m_rdHitlist.SetFile ( tDoclist.m_iFD );

			// read wordlist
			rdIndex.SeekTo ( iWordlistOffset );

			// restart delta decoding
			wordID = 0;
			SphOffset_t iDoclistOffset = 0;
			for ( ;; )
			{
				// unpack next word ID
				DWORD iDeltaWord = rdIndex.UnzipInt();
				if ( !iDeltaWord ) // wordlist chunk is over
					break;
				wordID += iDeltaWord;

				// unpack next offset
				SphOffset_t iDeltaOffset = rdIndex.UnzipOffset ();
				iDoclistOffset += iDeltaOffset;
				assert ( iDeltaOffset );

				// unpack doc/hit count
				int iDocs = rdIndex.UnzipInt ();
				int iHits = rdIndex.UnzipInt ();
				assert ( iDocs );
				assert ( iHits );

				// break on match or list end
				if ( wordID>=qwords[i].m_iWordID )
				{
					if ( wordID==qwords[i].m_iWordID )
					{
						qwords[i].m_rdDoclist.SeekTo ( iDoclistOffset );
						qwords[i].m_iDocs = iDocs;
						qwords[i].m_iHits = iHits;
					}
					break;
				}
			}
		}
	}

	// build word stats
	pResult->m_iNumWords = nwords;
	for ( i=0; i<nwords; i++ )
	{
		pResult->m_tWordStats[i].m_sWord = sphDup ( qwords[i].m_sWord );
		pResult->m_tWordStats[i].m_iDocs = qwords[i].m_iDocs;
		pResult->m_tWordStats[i].m_iHits = qwords[i].m_iHits;
	}

	// reorder hit lists
	qsort ( qwords, nwords, sizeof(CSphQueryWord), cmpQueryWord );

	PROFILE_END ( query_load_words );

	PROFILE_BEGIN ( query_match );

	// build weights
	nweights = m_tHeader.m_iFieldCount;
	for ( i=0; i<nweights; i++ ) // defaults
		weights[i] = 1;
	if ( pQuery->m_pWeights ) // user-supplied
		for ( int i=0; i<Min ( nweights, pQuery->m_iWeights ); i++ )
			weights[i] = pQuery->m_pWeights[i];

	// find and proximity-weight matching documents
	i = docID = 0;
	if ( pQuery->m_bAll )
	{
		///////////////////
		// match all words
		///////////////////

		if ( !qwords[0].m_iDocs )
			return pResult;

		// preload doclist entries
		for ( i=0; i<nwords; i++ )
			qwords[i].GetDoclistEntry ();
		i = 0;

		for ( ;; )
		{
			// scan lists until *all* the ids match
			while ( qwords[i].m_iDoc && docID>qwords[i].m_iDoc )
				qwords[i].GetDoclistEntry ();
			if ( !qwords[i].m_iDoc )
				break;

			if ( docID<qwords[i].m_iDoc )
			{
				docID = qwords[i].m_iDoc;
				i = 0;
				continue;
			}
			if (++i != nwords)
				continue;

			// preload hitlist entries
			for ( i=0; i<nwords; i++ )
				qwords[i].GetHitlistEntry ();

			// early reject by group id
			if ( pQuery->m_iGroup )
				if ( pQuery->m_iGroup != int( ((docID-1) & iGroupMask) + m_tHeader.m_iMinGroupID ) )
			{
				docID++;
				i = 0;
				continue;
			}

			// init weighting
			BYTE curPhraseWeight [ SPH_MAX_FIELD_COUNT ];
			BYTE phraseWeight [ SPH_MAX_FIELD_COUNT ];
			BYTE matchWeight [ SPH_MAX_FIELD_COUNT ];
			for ( i=0; i<nweights; i++ )
			{
				curPhraseWeight[i] = 0;
				phraseWeight[i] = 0;
				matchWeight[i] = 0;
			}

			k = INT_MAX;
			for ( ;; )
			{
				// scan until next hit in this document
				pmin = INT_MAX;
				imin = -1;
				for ( i=0; i<nwords; i++ )
				{
					if ( !qwords[i].m_iHitPos )
						continue;
					if ( pmin>qwords[i].m_iHitPos )
					{
						pmin = qwords[i].m_iHitPos;
						imin = i;
					}
				}
				if ( imin<0 )
					break;
				qwords[imin].GetHitlistEntry ();

				// get field number and mark a simple match
				j = pmin >> 24;
				matchWeight[j] = 1;

				// find max proximity relevance
				if ( qwords[imin].m_iQueryPos - pmin == k )
				{
					curPhraseWeight[j]++;
					if ( phraseWeight[j] < curPhraseWeight[j] )
						phraseWeight[j] = curPhraseWeight[j];
				} else
				{
					curPhraseWeight[j] = 0;
				}
				k = qwords[imin].m_iQueryPos - pmin;
			}

			// sum simple match weights and phrase match weights
			for ( i=0, j=0; i<nweights; i++ )
				j += weights[i] * ( matchWeight[i] + phraseWeight[i] );

			// add match
			CSphMatch & tMatch = pResult->m_dMatches.Add ();
			tMatch.m_iGroupID = ((docID-1) & iGroupMask) + m_tHeader.m_iMinGroupID; // unpack group id
			tMatch.m_iDocID = ((docID-1) >> m_tHeader.m_iGroupBits) + m_tHeader.m_iMinDocID; // unpack document id
			tMatch.m_iWeight = j; // set weight

			// continue looking for next matches
			i = 0;
			docID++;
		}

	} else
	{
		//////////////////
		// match any word
		//////////////////

		int iActive = nwords; // total number of words still active
		DWORD iDocID = 0; // FIXME! make a macro like INVALID_DOCUMENT_ID or something

		for ( ;; )
		{
			// update active pointers to document lists, kill empty ones,
			// and get min current document id
			DWORD iNewID = UINT_MAX;
			for ( i=0; i<iActive; i++ )
			{
				// move to next document
				if ( qwords[i].m_iDoc==iDocID )
				{
					qwords[i].GetDoclistEntry ();
					qwords[i].GetHitlistEntry ();
				}
				assert ( qwords[i].m_iDoc!=iDocID );

				// remove emptied words
				if ( !qwords[i].m_iDoc )
				{
					qwords[i] = qwords[iActive-1];
					i--;
					iActive--;
					continue;
				}

				// get new min id
				if ( qwords[i].m_iDoc<iNewID )
					iNewID = qwords[i].m_iDoc;
			}
			if ( iActive==0 )
				break;
			iDocID = iNewID;

			assert ( iDocID!=0 );
			assert ( iDocID!=UINT_MAX );

			// early reject by group id
			if ( pQuery->m_iGroup )
				if ( pQuery->m_iGroup != int( ((iDocID-1) & iGroupMask) + m_tHeader.m_iMinGroupID ) )
					continue;

			// get the words we're matching current document against (let's call them "terms")
			CSphQueryWord * pHit [ SPH_MAX_QUERY_WORDS ];
			int dPos [ SPH_MAX_QUERY_WORDS ];
			int iTerms = 0;

			for ( i=0; i<iActive; i++ )
				if ( qwords[i].m_iDoc==iDocID )
			{
				pHit[iTerms] = &qwords[i];
				dPos[iTerms] = qwords[i].m_iQueryPos;
				iTerms++;
			}
			assert ( iTerms>0 );

			// init weighting
			struct
			{
				int		m_iCurPhrase;	// current phrase-match weight
				int		m_iMaxPhrase;	// max phrase-match weight
				int		m_iMatch;		// max simple match weight
			} dWeights [ SPH_MAX_FIELD_COUNT ];
			memset ( dWeights, 0, sizeof(dWeights) ); // OPTIMIZE: only zero out nweights, not total

			// do matching
			// OPTIMIZE: implement simplified matching for iTerms==1 case
			k = INT_MAX;
			for ( ;; )
			{
				// get next good hit in this document
				pmin = INT_MAX;
				imin = -1;
				for ( i=0; i<iTerms; i++ )
				{
					// if hit list for this term is no longer active, remove it
					if ( !pHit[i]->m_iHitPos )
					{
						pHit[i] = pHit[iTerms-1];
						dPos[i] = dPos[iTerms-1];
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
				dWeights[iField].m_iMatch = 1; // FIXME! count matching words, maybe?

				// find max proximity relevance
				if ( dPos[imin] - pmin == k )
				{
					dWeights[iField].m_iCurPhrase++;
					if ( dWeights[iField].m_iMaxPhrase < dWeights[iField].m_iCurPhrase )
						dWeights[iField].m_iMaxPhrase = dWeights[iField].m_iCurPhrase;
				} else
				{
					dWeights[iField].m_iCurPhrase = 0;
				}
				k = dPos[imin] - pmin;
			}

			// sum simple match weights and phrase match weights
			for ( i=0, j=0; i<nweights; i++ )
				j += weights[i] * ( dWeights[i].m_iMatch + dWeights[i].m_iMaxPhrase );

			// add match
			CSphMatch & tMatch = pResult->m_dMatches.Add ();
			tMatch.m_iGroupID = ((iDocID-1) & iGroupMask) + m_tHeader.m_iMinGroupID; // unpack group id
			tMatch.m_iDocID = ((iDocID-1) >> m_tHeader.m_iGroupBits) + m_tHeader.m_iMinDocID; // unpack document id
			tMatch.m_iWeight = j; // set weight
		}
	}

	PROFILE_END ( query_match );
	PROFILE_BEGIN ( query_sort );

	pResult->m_dMatches.Sort ( CmpMatch_fn() );

	PROFILE_END ( query_sort );

	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_fQueryTime += sphLongTimer ();
	return pResult;
}

/////////////////////////////////////////////////////////////////////////////
// CRC32 DICTIONARY
/////////////////////////////////////////////////////////////////////////////

DWORD sphCRC32 ( const BYTE * pString )
{
	static const DWORD crc32tab [ 256 ] =
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

	// calc CRC
	DWORD crc = ~((DWORD)0);
	for ( const BYTE * p=pString; *p; p++ )
		crc = (crc >> 8) ^ crc32tab[(crc ^ (*p)) & 0xff];
	return ~crc;
}

CSphDict_CRC32::CSphDict_CRC32 ( DWORD iMorph )
	: m_iMorph		( iMorph )
	, m_iStopwords	( 0 )
	, m_pStopwords	( NULL )
{
	if ( m_iMorph & SPH_MORPH_STEM_RU )
		stem_ru_init ();
}


DWORD CSphDict_CRC32::GetWordID ( BYTE * pWord )
{
	// apply morphology
	if ( m_iMorph & SPH_MORPH_STEM_EN )
		stem_en ( pWord );
	if ( m_iMorph & SPH_MORPH_STEM_RU )
		stem_ru ( pWord );
	
	DWORD crc = sphCRC32 ( pWord );

	// apply stopwords
	// OPTIMIZE: binary search is not too good, do some hashing instead
	if ( m_iStopwords )
	{
		DWORD * pStart = m_pStopwords;
		DWORD * pEnd = m_pStopwords + m_iStopwords - 1;

		do
		{
			if ( crc==*pStart || crc==*pEnd )
				return 0;
			if ( crc<*pStart || crc>*pEnd )
				break;

			DWORD * pMid = pStart + (pEnd-pStart)/2;
			if ( crc==*pMid )
				return 0;
			if ( crc<*pMid )
				pEnd = pMid;
			else
				pStart = pMid;

		} while ( pEnd-pStart>1 );
	}

	// done
	return crc;
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


void CSphDict_CRC32::LoadStopwords ( const char * sFiles )
{
	static BYTE sBuffer [ 65536 ];

	// tokenize file list
	if ( !sFiles )
		return;
	char * sList = sphDup ( sFiles );
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

		// open file
		FILE * fp = fopen ( sName, "rb" );
		if ( !fp )
		{
			fprintf ( stderr, "WARNING: failed to load stopwords from '%s'.\n", sName );
			continue;
		}

		// tokenize file
		CSphTokenizer tTokenizer;
		CSphVector<DWORD> dStop;
		int iLength;
		do
		{
			iLength = (int)fread ( sBuffer, 1, sizeof(sBuffer), fp );
			tTokenizer.SetBuffer ( sBuffer, iLength );

			BYTE * pToken;
			while (( pToken = tTokenizer.GetToken() ))
				dStop.Add ( GetWordID ( pToken ) );
		} while ( iLength );

		// sort stopwords
		dStop.Sort ( DwordCmp_fn() );

		// store IDs
		m_iStopwords = dStop.GetLength ();
		m_pStopwords = new DWORD [ m_iStopwords ];
		memcpy ( m_pStopwords, &dStop[0], sizeof(DWORD)*m_iStopwords );
		fprintf ( stderr, "- loaded %d stopwords from '%s'\n",
			m_iStopwords, sName ); // FIXME! do loglevels

		// close file
		fclose ( fp );
	}

	sphFree ( sList );
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource() :
	dict ( NULL )
{
}


void CSphSource::setDict ( CSphDict * pDict )
{
	this->dict = pDict;
}


const CSphSourceStats * CSphSource::GetStats ()
{
	return &m_iStats;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

int CSphSource_Document::next()
{
	PROFILE ( src_document );

	BYTE **fields = NextDocument(), *data, *pData, *pWord;
	int pos, i, len;

	if (!fields || (m_iLastID <= 0)) return 0;

	m_iStats.m_iTotalDocuments++;
	hits.Reset ();
	for ( int j=0; j<m_iFieldCount; j++ )
	{
		data = fields[j];
		if ( !data )
			continue;

		i = len = (int)strlen((char*)data);
		m_iStats.m_iTotalBytes += len;

		pData = data;
		while (i-- > 0) { *pData = sphLT_cp1251[*pData]; pData++; }

		i = 0;
		pos = 1;
		pData = data;
		while (i < len) {
			while (!(*pData) && i < len) { pData++; i++; }
			if (i >= len) break;
			pWord = pData;
			while ((*pData) && i < len) { pData++; i++; }

			DWORD iWord = dict->GetWordID ( pWord );
			if ( iWord )
			{
				CSphHit & tHit = hits.Add ();
				tHit.m_iGroupID = m_iLastGroupID;
				tHit.m_iDocID = m_iLastID;
				tHit.m_iWordID = iWord;
				tHit.m_iWordPos = (j << 24) | pos++; // FIXME! add groups support here
				if ( m_bCallWordCallback )
					wordCallback ( (char*) pWord );
			}
		}
	}

	return m_iLastID;
}

int CSphSource_Document::GetFieldCount ()
{
	return m_iFieldCount;
}

/////////////////////////////////////////////////////////////////////////////
// PLAIN TEXT SOURCE
/////////////////////////////////////////////////////////////////////////////

BYTE **CSphSource_Text::NextDocument()
{
	static BYTE * t = NextText ();
	return t ? &t : NULL;
};

/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_MYSQL

CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
	: m_sQuery			( NULL )
	, m_sQueryPre		( NULL )
	, m_sQueryPost		( NULL )
	, m_sQueryRange		( NULL )
	, m_sGroupColumn	( NULL )
	, m_iRangeStep		( 1024 )

	, m_sHost			( NULL )
	, m_sUser			( NULL )
	, m_sPass			( NULL )
	, m_sDB				( NULL )
	, m_iPort			( 3306 )
	, m_sUsock			( NULL )
{
}

/////////////////////////////////////////////////////////////////////////////

const char * const CSphSource_MySQL::MACRO_VALUES [ CSphSource_MySQL::MACRO_COUNT ] =
{
	"$start",
	"$end"
};


CSphSource_MySQL::CSphSource_MySQL ()
	: m_pSqlResult		( NULL )
	, m_tSqlRow			( NULL )
	, m_sSqlDSN			( NULL )

	, m_sQuery			( NULL )
	, m_sQueryPost		( NULL )
	, m_iGroupColumn	( 0 )

	, m_iRangeStep		( 0 )
	, m_iMinID			( 0 )
	, m_iMaxID			( 0 )
	, m_iCurrentID		( 0 )
{
}


CSphSource_MySQL::~CSphSource_MySQL ()
{
	sphFree ( m_sSqlDSN );
	sphFree ( m_sQuery );
	sphFree ( m_sQueryPost );
}


bool CSphSource_MySQL::RunQueryStep ()
{
	if ( m_iRangeStep<=0 )
		return false;
	if ( m_iCurrentID>m_iMaxID )
		return false;

	static const int iBufSize = 16;
	char * sRes = NULL;

	//////////////////////////////////////////////
	// range query with $start/$end interpolation
	//////////////////////////////////////////////

	assert ( m_iMinID>0 );
	assert ( m_iMaxID>0 );
	assert ( m_iMinID<=m_iMaxID );
	assert ( m_sQuery );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	snprintf ( sValues[0], iBufSize, "%d", m_iCurrentID );
	snprintf ( sValues[1], iBufSize, "%d", m_iCurrentID+m_iRangeStep-1 );
	m_iCurrentID += m_iRangeStep;

	// OPTIMIZE? things can be precalculated
	char * sCur = m_sQuery;
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
	sCur = m_sQuery;

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
	const char * sError = NULL;
	do
	{
		if ( m_pSqlResult )
		{
			mysql_free_result ( m_pSqlResult );
			m_pSqlResult = NULL;
		}
		if ( mysql_query ( &m_tSqlDriver, sRes ) )
		{
			sError = "mysql_query";
			break;
		}
		if (!( m_pSqlResult = mysql_use_result ( &m_tSqlDriver ) ))
		{
			sError = "mysql_use_result";
			break;
		}
	} while ( false );

	SafeDeleteArray ( sRes );

	// report errors, if any
	if ( sError )
	{
		fprintf ( stderr, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN );
		return false;
	}

	// all ok
	return true;
}


bool CSphSource_MySQL::Init ( CSphSourceParams_MySQL * pParams )
{
	#define LOC_FIX_NULL(_arg) if ( !pParams->_arg ) pParams->_arg = "";
	#define LOC_ERROR(_msg,_arg) { fprintf ( stderr, _msg, _arg ); return 0; }
	#define LOC_ERROR2(_msg,_arg,_arg2) { fprintf ( stderr, _msg, _arg, _arg2 ); return 0; }
	#define LOC_MYSQL_ERROR(_msg) { sError = _msg; break; }

	const char * sError = NULL;

	// checks
	assert ( pParams );
	assert ( pParams->m_sQuery );

	// defaults
	LOC_FIX_NULL ( m_sQueryPre );
	LOC_FIX_NULL ( m_sQueryPost );
	LOC_FIX_NULL ( m_sGroupColumn );
	LOC_FIX_NULL ( m_sHost );
	LOC_FIX_NULL ( m_sUser );
	LOC_FIX_NULL ( m_sPass );
	LOC_FIX_NULL ( m_sDB );

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "mysql://%s:***@%s:%d/%s",
		pParams->m_sUser, pParams->m_sHost, pParams->m_iPort,
		pParams->m_sUsock
			? ( pParams->m_sUsock + ( pParams->m_sUsock[0]=='/' ? 1 : 0 ) )
			: "" );
	m_sSqlDSN = sphDup ( sBuf );

	// connect
	do
	{
		// initialize mysql lib
		mysql_init ( &m_tSqlDriver );

		// do connect
		if ( !mysql_real_connect ( &m_tSqlDriver,
			pParams->m_sHost,
			pParams->m_sUser,
			pParams->m_sPass,
			pParams->m_sDB,
			pParams->m_iPort,
			pParams->m_sUsock, 0 ) )
		{
			LOC_MYSQL_ERROR ( "mysql_real_connect" );
		}

		// run pre-query
		if ( pParams->m_sQueryPre && strlen(pParams->m_sQueryPre) )
		{
			if ( mysql_query ( &m_tSqlDriver, pParams->m_sQueryPre ) )
				LOC_MYSQL_ERROR ( "mysql_query_pre" );

			if ( (m_pSqlResult = mysql_use_result ( &m_tSqlDriver )) )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
		}

		// issue first fetch query
		if ( pParams->m_sQueryRange )
		{
			///////////////
			// range query
			///////////////

			m_iRangeStep = pParams->m_iRangeStep;

			// check step
			if ( m_iRangeStep<=0 )
				LOC_ERROR ( "ERROR: mysql_range_step=%d: must be positive.\n", m_iRangeStep );
			if ( m_iRangeStep<128 )
				fprintf ( stderr, "WARNING: mysql_range_step=%d: too small, increase recommended.\n", m_iRangeStep );

			// check query for macros
			bool bError = false;
			for ( int i=0; i<MACRO_COUNT; i++ )
			{
				if ( !strstr ( pParams->m_sQuery, MACRO_VALUES[i] ) )
				{
					fprintf ( stderr, "ERROR: mysql_ranged_query: macro '%s' not found.\n",
						MACRO_VALUES[i] );
					bError = true;
				}
			}
			if ( bError )
				return 0;

		    // run query
			if ( mysql_query ( &m_tSqlDriver, pParams->m_sQueryRange ) )
				LOC_MYSQL_ERROR ( "mysql_query_range" );

			// fetch min/max
			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( !m_pSqlResult )
				LOC_MYSQL_ERROR ( "mysql_use_result_range" );
			if ( m_pSqlResult->field_count!=2 )
				LOC_ERROR ( "ERROR: mysql_query_range: got %d columns, must be 2 (min_id/max_id).\n", m_pSqlResult->field_count );

			m_tSqlRow = mysql_fetch_row ( m_pSqlResult );
			if ( !m_tSqlRow )
				LOC_MYSQL_ERROR ( "mysql_fetch_row_range" );

			m_iMinID = atoi ( m_tSqlRow[0] );
			m_iMaxID = atoi ( m_tSqlRow[1] );
			if ( m_iMinID<=0 )
				LOC_ERROR ( "ERROR: mysql_query_range: min_id=%d: must be positive.\n", m_iMinID );
			if ( m_iMaxID<=0 )
				LOC_ERROR ( "ERROR: mysql_query_range: max_id=%d: must be positive.\n", m_iMaxID );
			if ( m_iMinID>m_iMaxID )
				LOC_ERROR2 ( "ERROR: mysql_query_range: min_id=%d must not be greater than max_id=%d.\n", m_iMinID, m_iMaxID );

			mysql_free_result ( m_pSqlResult );
			m_pSqlResult = NULL;

			m_iCurrentID = m_iMinID;
			m_sQuery = sphDup ( pParams->m_sQuery );

			// issue query
			if ( !RunQueryStep () )
				return 0;

		} else
		{
			////////////////
			// normal query
			////////////////

			assert ( !m_pSqlResult );
			if ( mysql_query ( &m_tSqlDriver, pParams->m_sQuery ) )
				LOC_MYSQL_ERROR ( "mysql_query" );
			if (!( m_pSqlResult = mysql_use_result ( &m_tSqlDriver ) ))
				LOC_MYSQL_ERROR ( "mysql_use_result" );
		}

	} while ( false );

	// errors, anyone?
	if ( sError )
	{
		fprintf ( stderr, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN );
		return 0;
	}

	// some post-query setup
	m_iFieldCount = mysql_num_fields ( m_pSqlResult ) - 1;
	m_sQueryPost = sphDup ( pParams->m_sQueryPost );

	// group column
	m_iGroupColumn = 0;
	if ( strlen(pParams->m_sGroupColumn) )
	{
		m_iGroupColumn = atoi ( pParams->m_sGroupColumn );

		if ( !m_iGroupColumn )
		{
			// if it's string, match by name
			m_iGroupColumn = -1;
			for ( int i=1; i<(int)mysql_num_fields(m_pSqlResult); i++ )
				if ( strcasecmp ( m_pSqlResult->fields[i].name, pParams->m_sGroupColumn )==0 )
			{
				m_iGroupColumn = i;
				break;
			}
		} else
		{
			// if it's number, reindex from base 1 to base 0
			m_iGroupColumn--;
		}

		if ( m_iGroupColumn<=0 || m_iGroupColumn>=(int)mysql_num_fields(m_pSqlResult) )
		{
			fprintf ( stderr, "WARNING: bad group column index (name='%s', index=%d), GROUPS DISABLED.\n",
				pParams->m_sGroupColumn, m_iGroupColumn );
			m_iGroupColumn = 0;
		} else
		{
			m_iFieldCount--;
			assert ( m_iFieldCount>0 );
		}
	}

	return 1;

	#undef LOC_FIX_NULL
	#undef LOC_ERROR
	#undef LOC_MYSQL_ERROR
}


BYTE ** CSphSource_MySQL::NextDocument ()
{
	PROFILE ( src_mysql );

	m_tSqlRow = mysql_fetch_row ( m_pSqlResult );

	// when the party's over...
	while ( !m_tSqlRow )
	{
		// maybe we can do next step yet?
		if ( RunQueryStep () )
		{
			m_tSqlRow = mysql_fetch_row ( m_pSqlResult );
			continue;
		}

		// ok, we're over
		while ( strlen(m_sQueryPost) )
		{
			if ( mysql_query ( &m_tSqlDriver, m_sQueryPost ) )
			{
				fprintf ( stderr, "WARNING: mysql_query_post: %s\n", mysql_error ( &m_tSqlDriver ) );
				break;
			}
			if ( (m_pSqlResult = mysql_use_result ( &m_tSqlDriver )) )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
			break;
		}

		return NULL;
	}

	// get him!
	m_iLastID = atoi ( m_tSqlRow[0] );
	if ( m_iGroupColumn )
	{
		// there's group column, need to extract group ID and reorder
		// OPTIMIZE: can prebuild field ID array once
		m_iLastGroupID = atoi ( m_tSqlRow [ m_iGroupColumn ] );

		memcpy ( &m_dFields[0], &m_tSqlRow[1], sizeof(BYTE*)*(m_iGroupColumn-1) );
		memcpy ( &m_dFields[m_iGroupColumn-1], &m_tSqlRow[m_iGroupColumn+1],
			sizeof(BYTE*)*(m_iFieldCount-m_iGroupColumn+1) );
		return m_dFields;

	} else
	{
		// no groups in this query
		m_iLastGroupID = 1;
		return (BYTE**)( &m_tSqlRow[1] );
	}
}

#endif // USE_MYSQL

/////////////////////////////////////////////////////////////////////////////
// HASH
/////////////////////////////////////////////////////////////////////////////

CSphHash::CSphHash()
{
	max = 32;
	count = 0;
	keys = (char**)sphMalloc(max * sizeof(char*));
	values = (char**)sphMalloc(max * sizeof(char*));
}

CSphHash::~CSphHash()
{
	sphFree(keys);
	sphFree(values);
}

void CSphHash::add(char *key, char *value)
{
	if (max == count) {
		max *= 2;
		keys = (char**)sphRealloc(keys, max * sizeof(char*));
		values = (char**)sphRealloc(values, max * sizeof(char*));
	}
	keys[count] = sphDup(key);
	values[count] = sphDup(value);
	count++;
}

char *CSphHash::get(char *key)
{
	int i;

	// NOTE: linear search is ok here, because these hashes
	// are intended to be rather tiny (config sections)
	if (key) for (i = 0; i < count; i++)
		if (strcmp(key, keys[i]) == 0) return values[i];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CONFIG FILE PARSER
/////////////////////////////////////////////////////////////////////////////

CSphConfig::CSphConfig ()
{
	fp = NULL;
	m_sFileName = NULL;
}


CSphConfig::~CSphConfig ()
{
	if ( fp )
	{
		fclose ( fp );
		fp = NULL;
	}
	sphFree ( m_sFileName );
}


int CSphConfig::open ( const char * file )
{
	fp = fopen ( file, "r" );
	if ( fp )
	{
		m_sFileName = sphDup ( file );
		return 1;
	} else
	{
		return 0;
	}
}


static char * ltrim ( char * sLine )
{
	while ( *sLine && isspace(*sLine) )
		sLine++;
	return sLine;
}


static char * rtrim ( char * sLine )
{
	char * p = sLine + strlen(sLine) - 1;
	while ( p>=sLine && isspace(*p) )
		p--;
	p[1] = '\0';
	return sLine;
}


static char * trim ( char * sLine )
{
	return ltrim ( rtrim ( sLine ) );
}


static char * sphCat ( char * sDst, const char * sSrc )
{
	sDst = (char *) sphRealloc ( sDst, strlen(sDst) + strlen(sSrc) + 1 );
	strcat ( sDst, sSrc );
	return sDst;
}


bool CSphConfig::ValidateKey ( const char * sKey, const char ** dKnownKeys )
{
	// no validation requested
	if ( dKnownKeys==NULL )
		return true;

	while ( *dKnownKeys )
	{
		if ( strcmp ( *dKnownKeys, sKey )==0 )
			return true;
		dKnownKeys++;
	}

	fprintf ( stderr, "WARNING: error in %s:%d, unknown key '%s' in section [%s]\n",
			m_sFileName, m_iLine, sKey, m_sSection );
	return false;
}


CSphHash * CSphConfig::loadSection ( const char * sSection, const char ** dKnownKeys )
{
	char buf[2048], *p, *pp, *sKey = NULL, *sValue = NULL;
	int l;
	CSphHash * pRes;

	// alloc result set
	pRes = new CSphHash ();
	if ( fseek ( fp, 0, SEEK_SET )<0 )
		 return pRes;

	m_iLine = 0;
	m_sSection = sSection;

	#define CLEAN_CONFIG_LINE() \
		p = trim ( buf ); \
		l = strlen ( p ); \
		pp = strchr ( p, '#' ); \
		if ( pp ) \
		{ \
			*pp = '\0'; \
			l = pp - p; \
		}

	// skip until we find the section
	while (( p = fgets ( buf, sizeof(buf), fp ) ))
	{
		m_iLine++;
		CLEAN_CONFIG_LINE ();
		if ( p[0]=='['
			&& p[l-1] == ']'
			&& strncmp ( p+1, sSection, strlen(sSection) )==0
			&& 2+(int)strlen(sSection)==l )
		{
			break;
		}
	}
	if ( !p )
	{
		m_sSection = NULL;
		return pRes;
	}

	// load all the config lines until next section or EOF
	while (( p = fgets ( buf, sizeof(buf), fp ) ))
	{
		m_iLine++;
		CLEAN_CONFIG_LINE ();

		// handle empty strings
		if ( !l )
			continue;

		// next section, bail out
		// FIXME! add more validation
		if ( p[0]=='[')
			break;

		// handle split strings
		if ( p[l-1]=='\\' )
		{
			if ( sKey )
			{
				p[l-1] = ' '; // insert space at the end of the split
				sValue = sphCat ( sValue, p );
				continue;

			} else
			{
				pp = strchr ( p, '=' );
				if ( pp )
				{
					*pp++ = '\0';
					sKey = rtrim ( p );
					if ( ValidateKey ( sKey, dKnownKeys ) )
					{
						p[l-1] = ' '; // insert space at the end of the split
						sKey = sphDup ( p );
						sValue = sphDup ( ltrim ( pp ) );
					} else
					{
						sKey = NULL;
					}
					continue;

				} else
				{
					fprintf ( stderr, "WARNING: error in %s:%d, stray line '%s' in section [%s]\n",
						m_sFileName, m_iLine, p, m_sSection );
					continue;
				}
			}

		} else if ( sKey )
		{
			// previous split-string just ended, so add it to result
			sValue = sphCat ( sValue, p );
			pRes->add ( sKey, sValue );

			sphFree ( sKey );
			sphFree ( sValue );
			sKey = NULL;
			continue;
		}

		// handle one-line pairs
		pp = strchr ( p, '=' );
		if ( pp )
		{
			*pp++ = '\0';
			p = rtrim ( p );
			if ( ValidateKey ( p, dKnownKeys ) )
			{
				pRes->add ( p, ltrim ( pp ) );
				sKey = NULL;
			}
			continue;
		}

		fprintf ( stderr, "WARNING: error in %s:%d, stray line '%s' in section [%s]\n",
			m_sFileName, m_iLine, p, m_sSection );
	}

	// return
	m_sSection = NULL;
	return pRes;
}

/////////////////////////////////////////////////////////////////////////////

CSphSource_XMLPipe::CSphSource_XMLPipe ()
{
	m_bBody			= false;
	m_pTag			= NULL;
	m_iTagLength	= 0;
	m_pPipe			= NULL;
	m_pBuffer		= NULL;
	m_pBufferEnd	= NULL;
}


CSphSource_XMLPipe::~CSphSource_XMLPipe ()
{
	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}
}


bool CSphSource_XMLPipe::Init ( const char * sCommand )
{
	assert ( sCommand );

	m_pPipe = popen ( sCommand, "r" );
	m_bBody = false;
	return ( m_pPipe!=NULL );
}


int CSphSource_XMLPipe::next ()
{
	PROFILE ( src_xmlpipe );
	char sTitle [ 1024 ]; // FIXME?

	assert ( m_pPipe );
	hits.Reset ();

	/////////////////////////
	// parse document header
	/////////////////////////

	if ( !m_bBody )
	{
		// scan for opening '<document>' tag if necessary
		SetTag ( "document" );
		if ( !SkipTag ( true, false ) )
			return 0;

		if ( !ScanInt ( "id", &m_iDocID ) )
			return 0;
		m_iStats.m_iTotalDocuments++;

		if ( !ScanInt ( "group", &m_iGroupID ) )
			return 0;

		if ( !ScanStr ( "title", sTitle, sizeof ( sTitle ) ) )
			return 0;

		// index title
		{
			int i, iLen, iPos=1;
			i = iLen = (int)strlen((char*)sTitle);

			// tolower, remove non-chars
			BYTE * pData = (BYTE*)sTitle;
			while (i-- > 0) { *pData = sphLT_cp1251[*pData]; pData++; }

			i = 0;
			pData = (BYTE*)sTitle;
			while ( i<iLen )
			{
				// skip whitespace
				while ( !(*pData) && i<iLen ) { pData++; i++; }
				if ( i>=iLen ) break;
				BYTE * pWord = pData;

				// skip non-whitespace
				while ( (*pData) && i<iLen ) { pData++; i++; }
		
				// add hit
				DWORD iWID = dict->GetWordID ( pWord );
				if ( iWID )
				{
					CSphHit & tHit = hits.Add ();
					tHit.m_iGroupID = m_iGroupID;
					tHit.m_iDocID = m_iDocID;
					tHit.m_iWordID = iWID;
					tHit.m_iWordPos = iPos++;
				}
			}
		}

		SetTag ( "body" );
		if ( !SkipTag ( true ) )
			return 0;

		m_bBody = true;
		m_iWordPos = 0;
	}

	/////////////////////////////
	// parse body chunk by chunk
	/////////////////////////////

	assert ( m_bBody );
	bool bBodyEnd = false;

	while ( hits.GetLength()<1024 ) // FIXME!
	{
		// skip whitespace
		for ( ;; )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",
					m_iDocID );
				return 0;
			}

			// skip whitespace
			while ( (m_pBuffer<m_pBufferEnd) && (*m_pBuffer)!='<' && !sphLT_cp1251 [ *m_pBuffer ] )
				m_pBuffer++;

			if ( m_pBuffer<m_pBufferEnd )
				break; // we have found it
		}
		if ( (*m_pBuffer)=='<' )
		{
			bBodyEnd = true;
			break;
		}

		// the word
		BYTE sWord [ SPH_MAX_WORD_LEN+1 ];
		BYTE sWord2 [ SPH_MAX_WORD_LEN+1 ];
		BYTE * pWord = &sWord [ 0 ];
		BYTE * pWordEnd = &sWord [ SPH_MAX_WORD_LEN ];

		for ( ;; )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",
					m_iDocID );
				return 0;
			}

			// collect word
			while ( pWord<pWordEnd && sphLT_cp1251 [ *m_pBuffer ] && m_pBuffer<m_pBufferEnd )
				*pWord++ = sphLT_cp1251 [ *m_pBuffer++ ];

			// enough?
			if ( pWord==pWordEnd || m_pBuffer!=m_pBufferEnd )
				break;
		}
		*pWord++ = '\0';

		// if the word is too long, skip all the remaining non-whitespace
		if ( pWord==pWordEnd )
			for ( ;; )
		{
			// suck in some data if needed
			if ( m_pBuffer>=m_pBufferEnd )
				if ( !UpdateBuffer() )
			{
				fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body starting at '%s'.\n",
					m_iDocID, sWord );
				return 0;
			}

			// skip non-whitespace
			while ( (m_pBuffer<m_pBufferEnd) && sphLT_cp1251 [ *m_pBuffer ] )
				m_pBuffer++;

			if ( m_pBuffer!=m_pBufferEnd )
				break; // we have found it
		}

		// we found it, yes we did!
		strcpy ( (char*)sWord2, (char*)sWord );
		DWORD iWID = dict->GetWordID ( sWord );

		// FIXME! do zero CRC warns in dict, in debug mode
		if ( iWID )
		{
			CSphHit & tHit = hits.Add ();
			tHit.m_iGroupID = m_iGroupID;
			tHit.m_iDocID = m_iDocID;
			tHit.m_iWordID = iWID;
			tHit.m_iWordPos = (1<<24) | (++m_iWordPos); // field_id | (iPos++)
		}
	}

	// some tag was found
	if ( bBodyEnd )
	{
		// let's check if it's '</body>' which is the only allowed tag at this point
		SetTag ( "body" );
		if ( !SkipTag ( false ) )
			return 0;

		// well, it is
		m_bBody = false;

		// let's check if it's '</document>' which is the only allowed tag at this point
		SetTag ( "document" );
		if ( !SkipTag ( false ) )
			return 0;
	}

	// if it was all correct, we have to flush our hits
	return m_iDocID;
}


void CSphSource_XMLPipe::SetTag ( const char * sTag )
{
	m_pTag = sTag;
	m_iTagLength = (int)strlen ( sTag );
}


bool CSphSource_XMLPipe::UpdateBuffer ()
{
	assert ( m_pBuffer!=&m_sBuffer[0] );

	int iLeft = Max ( m_pBufferEnd-m_pBuffer, 0 );
	if ( iLeft>0 )
		memmove ( m_sBuffer, m_pBuffer, iLeft );

	size_t iLen = fread ( &m_sBuffer [ iLeft ], 1, sizeof(m_sBuffer)-iLeft, m_pPipe );
	m_iStats.m_iTotalBytes += iLen;

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


bool CSphSource_XMLPipe::CheckTag ( bool bOpen )
{
	int iAdd = bOpen ? 2 : 3;

	// if case the tag is at buffer boundary, try to suck in some more data
	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
		UpdateBuffer ();

	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
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

		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got '%s'.\n",
			bOpen ? "" : "/", m_pTag, sGot );
		return false;
	}

	// got tag
	m_pBuffer += iAdd+m_iTagLength;
	assert ( m_pBuffer<=m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::SkipTag ( bool bOpen, bool bWarnOnEOF )
{
	if ( !SkipWhitespace() )
	{
		if ( bWarnOnEOF )
			fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
				bOpen ? "" : "/", m_pTag );
		return false;
	}

	return CheckTag ( bOpen );
}


bool CSphSource_XMLPipe::ScanInt ( const char * sTag, int * pRes )
{
	assert ( sTag );
	assert ( pRes );

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true ) )
		return false;

	if ( !SkipWhitespace() )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
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
	if ( !SkipTag ( false ) )
		return false;

	return true;
}


bool CSphSource_XMLPipe::ScanStr ( const char * sTag, char * pRes, int iMaxLength )
{
	assert ( sTag );
	assert ( pRes );
	
	char * pEnd = pRes+iMaxLength-1;

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true ) )
		return false;

	if ( !SkipWhitespace() )
	{
		fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
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
	if ( !SkipTag ( false ) )
		return false;

	return true;
}


int CSphSource_XMLPipe::GetFieldCount ()
{
	return 2;
}

//
// $Id$
//
