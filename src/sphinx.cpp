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
#include <time.h>

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

#if ( USE_WINDOWS && USE_MYSQL )
	#pragma comment(linker, "/defaultlib:libmysql.lib")
	#pragma message("Automatically linking with libmysql.lib")
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
		fprintf ( stderr, "%s", sBuffer );
		exit ( 1 );
	}
}

#undef assert
#define assert(_expr) { if ( !(_expr) ) sphAssert ( #_expr, __FILE__, __LINE__ ); }

#endif // !NDEBUG
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

static bool		g_bSphQuiet		= false;

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
	if ( tTimer.m_fStamp<0.005f )
		return;

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

/// fat hit, which is actually stored in VLN index
struct CSphFatHit
{
	DWORD	m_iDocID;		///< document ID
	DWORD	m_iGroupID;		///< documents group ID
	DWORD	m_iTimestamp;	///< document timestamp
	DWORD	m_iWordID;		///< word ID in current dictionary
	DWORD	m_iWordPos;		///< word position in current document
};
STATIC_SIZE_ASSERT ( CSphFatHit, 20 );

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

/////////////////////////////////////////////////////////////////////////////

struct CSphBin
{
	static const int	MIN_SIZE = 8192;
	int					m_iSize;

	BYTE *				m_dBuffer;
	BYTE *				m_pCurrent;
	int					m_iLeft;
	int					m_iDone;
	ESphBinState		m_eState;

	CSphFatHit			m_tLastHit;		///< last decoded hit
	SphOffset_t			m_iFilePos;		///< bin offset in raw hits log
	int					m_iFileLeft;	///< how much data is still unread from raw log via this bin

	CSphBin ()
		: m_dBuffer ( NULL )
		, m_pCurrent ( NULL )
		, m_iLeft ( 0 )
		, m_iDone ( 0 )
		, m_eState ( BIN_POS )
		, m_iFilePos ( 0 )
		, m_iFileLeft ( 0 )
	{
		m_tLastHit.m_iDocID = 0;
		m_tLastHit.m_iGroupID = 0;
		m_tLastHit.m_iTimestamp = 0;
		m_tLastHit.m_iWordID = 0;
		m_tLastHit.m_iWordPos = 0;
	}


	void Init ( int iSize )
	{
		assert ( !m_dBuffer );
		assert ( iSize>=MIN_SIZE );

		m_iSize = iSize;
		m_dBuffer = new BYTE [ iSize ];
		m_pCurrent = m_dBuffer; 
	}


	~CSphBin ()
	{
		SafeDeleteArray ( m_dBuffer );
	}
};

/////////////////////////////////////////////////////////////////////////////

/// VLN index header
struct CSphIndexHeader_VLN
{
	DWORD		m_iFieldCount;	///< document fields count
	CSphDocInfo	m_tMin;			///< mind id/group/timestamp
};

/////////////////////////////////////////////////////////////////////////////

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

	bool		IsError ();

private:
	int			m_iFD;
	int			m_iPoolUsed;
	int			m_iPoolOdd;
	BYTE		m_dPool [ SPH_CACHE_WRITE ];
	BYTE *		m_pPool;
	bool		m_bError;

	void		PutNibble ( int data );
	void		Flush ();
};

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

/// query word from the searcher's point of view
class CSphQueryWord
{
public:
	char *			m_sWord;		///< my copy of word
	int				m_iQueryPos;	///< word position, from query
	DWORD			m_iWordID;		///< word ID, from dictionary

	int				m_iDocs;		///< document count, from wordlist
	int				m_iHits;		///< hit count, from wordlist

	CSphDocInfo		m_tDoc;			///< current document info, from doclist
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
		, m_iHitPos ( 0 )
		, m_iHitlistPos ( 0 )
	{
		m_tDoc.m_iDocID = 0;
		m_tDoc.m_iGroupID = 0;
		m_tDoc.m_iTimestamp = 0;
	}

	~CSphQueryWord ()
	{
		sphFree ( m_sWord );
	}

	const CSphQueryWord & operator = ( const CSphQueryWord & rhs )
	{
		sphFree ( rhs.m_sWord );
		m_sWord			= sphDup ( rhs.m_sWord );
		m_iQueryPos		= rhs.m_iQueryPos;
		m_iWordID		= rhs.m_iWordID;
		m_iDocs			= rhs.m_iDocs;
		m_iHits			= rhs.m_iHits;
		m_iHitPos		= rhs.m_iHitPos;
		m_iHitlistPos	= rhs.m_iHitlistPos;
		m_rdDoclist		= rhs.m_rdDoclist;
		m_rdHitlist		= rhs.m_rdHitlist;
		m_tDoc			= rhs.m_tDoc;
		return *this;
	}

	void GetDoclistEntry ()
	{
		DWORD iDeltaDoc = m_rdDoclist.UnzipInt ();
		if ( iDeltaDoc )
		{
			m_tDoc.m_iDocID += iDeltaDoc;

			m_tDoc.m_iGroupID = m_rdDoclist.UnzipInt ();
			m_tDoc.m_iTimestamp = m_rdDoclist.UnzipInt ();
			assert ( m_tDoc.m_iGroupID );
			assert ( m_tDoc.m_iTimestamp );

			SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset ();
			assert ( iDeltaPos>0 );

			m_iHitlistPos += iDeltaPos;
			m_rdHitlist.SeekTo ( m_iHitlistPos );
			m_iHitPos = 0;

		} else
		{
			m_tDoc.m_iDocID = 0;
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

/////////////////////////////////////////////////////////////////////////////

/// this is my actual VLN-compressed phrase index implementation
struct CSphIndex_VLN : CSphIndex
{
								CSphIndex_VLN ( const char * filename );
	virtual						~CSphIndex_VLN ();

	virtual int					Build ( CSphDict * dict, CSphSource * source, int iMemoryLimit );
	virtual CSphQueryResult *	Query ( CSphDict * dict, CSphQuery * pQuery );
	virtual bool				QueryEx ( CSphDict * dict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchQueue * pTop );

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

	CSphVector<CSphDocInfo>		m_dDocinfos;		///< current raw block docinfos

	CSphFatHit					m_tLastHit;			///< VLN writer state
	SphOffset_t					m_iLastDoclistPos;	///< VLN writer state
	SphOffset_t					m_iLastHitlistPos;	///< VLN writer state
	DWORD						m_iLastPageID;		///< VLN writer state
	int							m_iLastWordDocs;	///< VLN writer state
	int							m_iLastWordHits;	///< VLN writer state

	int							OpenFile ( char * ext, int mode );

	int							binsInit ();
	void						binsDone ();
	DWORD						binsReadVLB ( int b );
	int							binsReadByte ( int b );
	int							binsRead ( int b, CSphFatHit * e );

	int							cidxCreate ();
	int							cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iCount );
	void						cidxHit ( CSphFatHit * pHit );
	void						cidxDone ();
};

/////////////////////////////////////////////////////////////////////////////

class CSphHTMLStripper
{
public:
						CSphHTMLStripper ();
						~CSphHTMLStripper ();
	void				AddAttr ( const char * sTag, const char * sAttr );
	const char *		SetAttrs ( const char * sConfig );
	BYTE *				Strip ( BYTE * sData );

protected:
	struct AttrLink_t
	{
		char *			m_sAttr;
		AttrLink_t *	m_pNext;

		AttrLink_t ( const char * sAttr )
		{
			m_sAttr = new char [ 1+strlen(sAttr) ];
			m_pNext = NULL;
			strcpy ( m_sAttr, sAttr );
		}

		~AttrLink_t ()
		{
			SafeDeleteArray ( m_sAttr );
		}
	};

	struct TagLink_t
	{
		char *			m_sTag;
		TagLink_t *		m_pNext;
		AttrLink_t *	m_pAttrs;

		TagLink_t ( const char * sTag )
		{
			m_sTag = new char [ 1+strlen(sTag) ];
			m_pNext = NULL;
			m_pAttrs = NULL;
			strcpy ( m_sTag, sTag );
		}

		~TagLink_t ()
		{
			SafeDeleteArray ( m_sTag );

			while ( m_pAttrs )
			{
				AttrLink_t * pKill = m_pAttrs;
				m_pAttrs = m_pAttrs->m_pNext;
				SafeDelete ( pKill );
			}
		}
	};

	TagLink_t *			m_pTags;
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
				sName, iWritten, int(iCount) );
		}
	}
	return iWritten;
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

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

/// lowercaser remap range
struct CSphRemapRange
{
	int			m_iStart;
	int			m_iEnd;
	int			m_iRemapStart;

	CSphRemapRange ()
		: m_iStart		( -1 )
		, m_iEnd		( -1 )
		, m_iRemapStart	( -1 )
	{}

	CSphRemapRange ( int iStart, int iEnd, int iRemapStart )
		: m_iStart		( iStart )
		, m_iEnd		( iEnd )
		, m_iRemapStart	( iRemapStart )
	{}

	inline bool operator < ( const CSphRemapRange & b )
	{
		return m_iStart < b.m_iStart;
	}
};


/// lowercaser
class CSphLowercaser
{
public:
				CSphLowercaser ();
				~CSphLowercaser ();

	void		SetRemap ( const CSphRemapRange * pRemaps, int iRemaps );

public:
	inline int	ToLower ( int iCode )
	{
		assert ( iCode>=0 );
		if ( iCode>=MAX_CODE )
			return 0;
		register int * pChunk = m_ppTable [ iCode>>CHUNK_BITS ];
		if ( pChunk )
			return pChunk [ iCode & CHUNK_MASK ];
		return 0;
	}

protected:
	int **				m_ppTable;
	int *				m_pTable;

	static const int	CHUNK_COUNT	= 0x200;
	static const int	CHUNK_BITS	= 8;

	static const int	CHUNK_SIZE	= 1 << CHUNK_BITS;
	static const int	CHUNK_MASK	= CHUNK_SIZE - 1;
	static const int	MAX_CODE	= CHUNK_COUNT * CHUNK_SIZE;
};


/// parser to build lowercaser from textual config
class CSphCharsetDefinitionParser
{
public:
						CSphCharsetDefinitionParser ();
	int					Parse ( const char * sConfig, CSphLowercaser & tLC );
	const char *		GetLastError ();

protected:
	bool				m_bError;
	char				m_sError [ 1024 ];
	const char *		m_pCurrent;

	int					Error ( const char * sMessage );
	void				SkipSpaces ();
	bool				IsEof ();
	bool				CheckEof ();
	int					HexDigit ( int c );
	int					ParseCharsetCode ();
};


/// single-byte charset tokenizer
class CSphTokenizer_SBCS : public ISphTokenizer
{
public:
						CSphTokenizer_SBCS ();
	virtual				~CSphTokenizer_SBCS ();

	virtual void		SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );
	virtual BYTE *		GetToken ();
	virtual bool		SetCaseFolding ( const char * sConfig );

protected:
	CSphLowercaser		m_tLC;								///< my lowercaser
	BYTE *				m_pBuffer;							///< my buffer
	BYTE *				m_pBufferMax;						///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;								///< current position
	BYTE				m_sAccum [ 4+SPH_MAX_WORD_LEN ];	///< boundary token accumulator
	int					m_iAccum;							///< boundary token size
	bool				m_bLast;							///< is this buffer the last one
};


/// UTF-8 tokenizer
class CSphTokenizer_UTF8 : public ISphTokenizer
{
public:
						CSphTokenizer_UTF8 ();
	virtual				~CSphTokenizer_UTF8 ();

	virtual void		SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );
	virtual BYTE *		GetToken ();
	virtual bool		SetCaseFolding ( const char * sConfig );

protected:
	inline int			IsEmpty () { return m_pCur>=m_pBufferMax; }
	int					GetCodePoint ();
	void				AccumCodePoint ( int iCode );
	BYTE *				FlushAccum ();

protected:
	CSphLowercaser		m_tLC;								///< my lowercaser
	BYTE *				m_pBuffer;							///< my buffer
	BYTE *				m_pBufferMax;						///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;								///< current position
	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< boundary token accumulator
	BYTE *				m_pAccum;							///< current accumulator position
	int					m_iAccum;							///< boundary token size
	bool				m_bLast;							///< is this buffer the last one
};

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphCreateSBCSTokenizer ()
{
	return new CSphTokenizer_SBCS ();
}


ISphTokenizer * sphCreateUTF8Tokenizer ()
{
	return new CSphTokenizer_UTF8 ();
}

/////////////////////////////////////////////////////////////////////////////

CSphLowercaser::CSphLowercaser ()
	: m_ppTable ( NULL )
	, m_pTable ( NULL )
{
}


void CSphLowercaser::SetRemap ( const CSphRemapRange * pRemaps, int iRemaps )
{
	SafeDeleteArray ( m_ppTable );
	SafeDeleteArray ( m_pTable );

	#define LOC_CHECK_RANGE(_a) assert ( (_a)>=0 && (_a)<MAX_CODE );

	// find out chunk count and build chunk usage bitmap
	int dUsed [ CHUNK_COUNT ];
	memset ( dUsed, 0, sizeof(dUsed) );

	const CSphRemapRange * pRemap = pRemaps;
	for ( int i=0; i<iRemaps; i++, pRemap++ )
	{
		LOC_CHECK_RANGE ( pRemap->m_iStart );
		LOC_CHECK_RANGE ( pRemap->m_iEnd );
		LOC_CHECK_RANGE ( pRemap->m_iRemapStart );
		LOC_CHECK_RANGE ( pRemap->m_iRemapStart + pRemap->m_iEnd + pRemap->m_iStart );

		for ( int j = pRemap->m_iStart>>CHUNK_BITS; j <= pRemap->m_iEnd>>CHUNK_BITS; j++ )
			dUsed[j] = 1;
	}

	int iUsed = 0;
	for ( int i=0; i<CHUNK_COUNT; i++ )
		if ( dUsed[i] ) iUsed++;

	#undef LOC_CHECK_RANGE

	// alloc
	assert ( iUsed );

	m_ppTable = new int * [ CHUNK_COUNT ];
	m_pTable = new int [ iUsed*CHUNK_SIZE ];
	memset ( m_ppTable, 0, sizeof(int*)*CHUNK_COUNT );
	memset ( m_pTable, 0, sizeof(int)*iUsed*CHUNK_SIZE );

	// build chunks hash
	int * pChunk = m_pTable;
	for ( int i=0; i<CHUNK_COUNT; i++ )
		if ( dUsed[i] )
	{
		m_ppTable[i] = pChunk;
		pChunk += CHUNK_SIZE;
	}

	// fill the actual remapping table chunks
	pRemap = pRemaps;
	for ( int i=0; i<iRemaps; i++, pRemap++ )
	{
		int iRemapped = pRemap->m_iRemapStart;
		for ( int j = pRemap->m_iStart; j <= pRemap->m_iEnd; j++, iRemapped++ )
		{
			assert ( m_ppTable [ j>>CHUNK_BITS ] );
			m_ppTable [ j>>CHUNK_BITS ] [ j & CHUNK_MASK ] = iRemapped;
		}
	}
}


CSphLowercaser::~CSphLowercaser ()
{
	SafeDeleteArray ( m_ppTable );
	SafeDeleteArray ( m_pTable );
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


int CSphCharsetDefinitionParser::Error ( const char * sMessage )
{
	char sErrorBuffer[32];
	strncpy ( sErrorBuffer, m_pCurrent, sizeof(sErrorBuffer) );
	sErrorBuffer [ sizeof(sErrorBuffer)-1 ] = '\0';

	snprintf ( m_sError, sizeof(m_sError), "CharsetDefinitionParser(): %s near '%s'.",
		sMessage, sErrorBuffer );
	m_sError [ sizeof(m_sError)-1 ] = '\0';

	m_bError = true;
	return -1;
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
	while ( (*m_pCurrent) && isspace(*m_pCurrent) )
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
			return Error ( "non-ASCII characters not allowed, use 'U+00AB' syntax" );

		iCode = *p++;
		while ( isspace(*p) )
			p++;
	}

	m_pCurrent = p;
	return iCode;
}


int CSphCharsetDefinitionParser::Parse ( const char * sConfig, CSphLowercaser & tLC )
{
	m_pCurrent = sConfig;

	CSphVector<CSphRemapRange> dRanges;

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
		int iStart = ParseCharsetCode ();
		if ( iStart<0 )
			return -1;

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
			m_pCurrent += 2;
			int iDest = ParseCharsetCode ();
			if ( iDest<0 )
				return -1;

			dRanges.Add ( CSphRemapRange ( iStart, iStart, iDest ) );
			if ( *m_pCurrent!=',' )
				return Error ( "syntax error" );
			m_pCurrent++;
			continue;
		}

		// range start?
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "syntax error" );
		m_pCurrent += 2;
		
		SkipSpaces ();
		if ( CheckEof () )
			return -1;

		// parse range end char code
		int iEnd = ParseCharsetCode ();
		if ( iEnd<0 )
			return -1;
		if ( iStart>iEnd )
		{
			m_pCurrent = pStart;
			return Error ( "range end less than range start" );
		}

		// stray range
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			dRanges.Add ( CSphRemapRange ( iStart, iEnd, iStart ) );
			if ( IsEof () )
				break;
			m_pCurrent++;
			continue;
		}

		// remapped range?
		if (!( m_pCurrent[0]=='-' && m_pCurrent[1]=='>' ))
			return Error ( "expected end of line, ',' or '-><char>'" );
		m_pCurrent += 2;

		SkipSpaces ();
		if ( CheckEof () )
			return -1;

		// parse dest start
		const char * pRemapStart = m_pCurrent;
		int iRemapStart = ParseCharsetCode ();
		if ( iRemapStart<0 )
			return -1;

		// expect '..'
		if ( CheckEof () )
			return -1;
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "expected '..'" );
		m_pCurrent += 2;

		// parse dest end
		int iRemapEnd = ParseCharsetCode ();
		if ( iRemapEnd<0 )
			return -1;

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

	tLC.SetRemap ( &dRanges[0], dRanges.GetLength() );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_SBCS::CSphTokenizer_SBCS ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
	, m_bLast		( false )
{
	SetCaseFolding (
		"0..9, A..Z->a..z, _, a..z, "
		"U+a8->U+b8, U+b8, U+c0..U+df->U+e0..U+ff, U+e0..U+ff" );
}


CSphTokenizer_SBCS::~CSphTokenizer_SBCS ()
{
}

void CSphTokenizer_SBCS::SetBuffer ( BYTE * sBuffer, int iLength, bool bLast )
{
	// check that old one is over and that new length is sane
	assert ( m_pCur>=m_pBufferMax );
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
	m_bLast = bLast;

	// do inplace case and non-char removal
	BYTE * p = sBuffer;
	while ( p<m_pBufferMax )
	{
		*p = (BYTE) m_tLC.ToLower ( *p ); // FIXME! !COMMIT
		p++;
	}
}


BYTE * CSphTokenizer_SBCS::GetToken ()
{
	// flush whatever accumulated from that last buffer
	if ( m_iAccum )
	{
		// if it's not EOF
		if ( m_pCur<m_pBufferMax )
		{
			// accumulate as much extra chars as possible
			while ( m_pCur<m_pBufferMax && *m_pCur && m_iAccum<SPH_MAX_WORD_LEN )
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

	// check for buffer overflow
	if ( m_pCur>=m_pBufferMax )
		return NULL;

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
		if ( m_pCur-pToken>SPH_MAX_WORD_LEN )
			pToken [ SPH_MAX_WORD_LEN ] = '\0';
		return pToken;
	}

	// buffer's over, so we need to accumulate
	m_iAccum = Min ( SPH_MAX_WORD_LEN, m_pCur-pToken );
	memcpy ( m_sAccum, pToken, m_iAccum );

	if ( m_bLast && m_iAccum )
	{
		m_sAccum [ m_iAccum ] = '\0';
		m_iAccum = 0;
		return m_sAccum;
	} else
	{
		return NULL;
	}
}


bool CSphTokenizer_SBCS::SetCaseFolding ( const char * sConfig )
{
	CSphCharsetDefinitionParser tParser;
	if ( tParser.Parse ( sConfig, m_tLC )!=0 )
	{
		fprintf ( stderr, "ERROR: %s", tParser.GetLastError() );
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_UTF8::CSphTokenizer_UTF8 ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
	, m_bLast		( false )
{
	m_pAccum = m_sAccum;
	SetCaseFolding ( "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F" );
}


CSphTokenizer_UTF8::~CSphTokenizer_UTF8 ()
{
}


void CSphTokenizer_UTF8::SetBuffer ( BYTE * sBuffer, int iLength, bool bLast )
{
	// check that old one is over and that new length is sane
	assert ( m_pCur>=m_pBufferMax );
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
	m_bLast = bLast;

	// fixup embedded zeroes with spaces
	for ( BYTE * p = m_pBuffer; p < m_pBufferMax; p++ )
		if ( !*p )
			*p = ' ';
}


BYTE * CSphTokenizer_UTF8::GetToken ()
{
	// flush whatever accumulated from that last buffer
	if ( m_iAccum )
	{
		// if it's not EOF
		if ( !IsEmpty() )
		{
			// accumulate as much extra chars as possible
			// AccumCodePoint will through away everything which is over the token size limit
			for ( int iCode=GetCodePoint(); iCode; iCode=GetCodePoint() )
				AccumCodePoint ( iCode );

			// if buffer is now over (wow, that's REAL big token),
			// we need to get another bufer
			if ( IsEmpty() )
				return NULL;
		}

		// clear and return the accumulated token
		return FlushAccum ();
	}

	// check for buffer overflow
	if ( IsEmpty() )
		return NULL;

	// skip whitespace
	int iCode;
	while ( ( iCode = GetCodePoint() )==0 && !IsEmpty() );
	if ( IsEmpty() )
		return NULL;

	// accumulate non-whitespace
	FlushAccum ();
	AccumCodePoint ( iCode );

	while ( ( iCode=GetCodePoint() )!=0 )
		AccumCodePoint ( iCode );

	// if buffer's not over,
	// or if buffer's over but it's the last one,
	// we have a full token now
	if ( !IsEmpty() || ( m_bLast && m_iAccum ) )
		return FlushAccum ();
	return NULL;
}


int CSphTokenizer_UTF8::GetCodePoint ()
{
	if ( IsEmpty() )
		return 0;

	for ( ;; )
	{
		// check for eof
		BYTE v = *m_pCur;
		if ( !v )
			return 0;
		m_pCur++;

		// check for 7-bit case
		if ( v<128 )
			return m_tLC.ToLower ( v );

		// get number of bytes
		int iBytes = 0;
		while ( v & 0x80 )
		{
			iBytes++;
			v <<= 1;
		}

		// check for valid number of bytes
		if ( iBytes<2 || iBytes>4 )
			continue;

		int iCode = ( v>>iBytes );
		iBytes--;
		do
		{
			if ( !(*m_pCur) )
				break;
			if ( ((*m_pCur) & 0xC0)!=0x80 )
				break;

			iCode = ( iCode<<6 ) + ( (*m_pCur) & 0x3F );
			iBytes--;
		} while ( iBytes );

		// return code point if there were no errors
		// ignore and continue scanning otherwise
		if ( !iBytes )
			return m_tLC.ToLower ( iCode );
	}
}


void CSphTokenizer_UTF8::AccumCodePoint ( int iCode )
{
	assert ( iCode>0 );
	assert ( m_iAccum>=0 );

	// throw away everything which is over the token size
	if ( m_iAccum>SPH_MAX_WORD_LEN )
		return;

	// do UTF-8 encoding here
	if ( iCode<0x80 )
	{
		*m_pAccum++ = (BYTE)( iCode & 0x7F );

	} else if ( iCode<0x800 )
	{
		*m_pAccum++ = (BYTE)( ( (iCode>>6) & 0x1F ) | 0xC0 );
		*m_pAccum++ = (BYTE)( ( iCode & 0x3F ) | 0x80 );

	} else
	{
		*m_pAccum++ = (BYTE)( ( (iCode>>12) & 0x0F ) | 0xC0 );
		*m_pAccum++ = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		*m_pAccum++ = (BYTE)( ( iCode & 0x3F ) | 0x80 );
	}

	assert ( m_pAccum>=m_sAccum && m_pAccum<m_sAccum+sizeof(m_sAccum) );
	m_iAccum++;
}


BYTE * CSphTokenizer_UTF8::FlushAccum ()
{
	BYTE * pRes = m_sAccum;

	assert ( m_pAccum-m_sAccum < (int)sizeof(m_sAccum) );
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;

	return pRes;
}


bool CSphTokenizer_UTF8::SetCaseFolding ( const char * sConfig )
{
	CSphCharsetDefinitionParser tParser;
	if ( tParser.Parse ( sConfig, m_tLC )!=0 )
	{
		fprintf ( stderr, "ERROR: %s", tParser.GetLastError() );
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////

CSphQuery::CSphQuery ()
	: m_sQuery		( NULL )
	, m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_eMode		( SPH_MATCH_ALL )
	, m_pGroups		( NULL )
	, m_iGroups		( 0 )
	, m_eSort		( SPH_SORT_RELEVANCE )
	, m_pTokenizer	( NULL )
{
}


CSphQuery::~CSphQuery ()
{
	SafeDeleteArray ( m_pGroups );
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
	m_iFD = -1;
	m_iPos = 0;
	m_bError = false;
}


CSphWriter_VLN::~CSphWriter_VLN()
{
//	close();
	sphFree ( m_sName );
}


int CSphWriter_VLN::OpenFile ()
{
	if ( m_iFD>=0 )
		return m_iFD;
	m_iFD = ::open ( m_sName, O_CREAT | O_RDWR | O_TRUNC | SPH_BINARY, 0644 );
	m_iPoolUsed = 0;
	m_iPoolOdd = 0;
	m_iPos = 0;
	return m_iFD;
}


void CSphWriter_VLN::CloseFile() 
{
	if ( m_iFD>=0 )
	{
		Flush ();
		::close ( m_iFD );
		m_iFD = -1;
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

	if ( sphWrite ( m_iFD, m_dPool, m_iPoolUsed, m_sName )!=m_iPoolUsed )
		m_bError = true;

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


bool CSphWriter_VLN::IsError ()
{
	return m_bError;
}

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE INPUT
///////////////////////////////////////////////////////////////////////////////

CSphReader_VLN::CSphReader_VLN ()
	: m_iFD ( -1 )
	, m_iPos ( 0 )
	, m_iBufPos ( 0 )
	, m_iBufOdd ( 0 )
	, m_iBufUsed ( 0 )
{
	m_iBufSize = 262144; // FIXME
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
	PROFILE ( read_hits );
	assert ( m_iFD>=0 );

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
	{
		register int t = GetNibble()<<4;
		t += GetNibble ();
		*b++ = (BYTE)t;
	}
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

const CSphReader_VLN & CSphReader_VLN::operator = ( const CSphReader_VLN & rhs )
{
	SetFile ( rhs.m_iFD );
	SeekTo ( rhs.m_iPos + rhs.m_iBufPos*2 + rhs.m_iBufOdd );
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
	
	fdIndex = NULL;
	fdData = NULL;
	fdRaw = 0;

	m_iLastDoclistPos = 0;
	m_iLastHitlistPos = 0;
	m_iLastWordDocs = 0;
	m_iLastWordHits = 0;
	m_iLastPageID = 0xffffffff;
	m_tLastHit.m_iDocID = 0;
	m_tLastHit.m_iWordID = 0;
	m_tLastHit.m_iWordPos = 0;
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
void sphSortHits ( CSphWordHit * s, int n )
{
	int st0[32], st1[32];
	int a, b, k, i, j;
	CSphWordHit t;
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

	int iFD = ::open ( sBuf, mode | SPH_BINARY, 0644 );
	if ( iFD<0 )
		fprintf ( stderr, "ERROR: failed to open '%s': %s.\n", sBuf, strerror(errno) );

	return iFD;
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

	if ( !bins[b]->m_iLeft )
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
			bins[b]->m_iDone = 1;
			bins[b]->m_iLeft = 1;
		} else
		{
			assert ( bins[b]->m_dBuffer );

			if ( ::read ( fdRaw, bins[b]->m_dBuffer, n )!=n )
				return -2;
			bins[b]->m_iLeft = n;

			bins[b]->m_iFilePos += n;
			bins[b]->m_iFileLeft -= n;
			bins[b]->m_pCurrent = bins[b]->m_dBuffer;
			m_iFilePos += n;
		}
	}
	if ( bins[b]->m_iDone )
		return -1;

	bins[b]->m_iLeft--;
	r = *(bins[b]->m_pCurrent);
	bins[b]->m_pCurrent++;
	return r;
}

DWORD CSphIndex_VLN::binsReadVLB(int b)
{
	DWORD v = 0, o = 0;
	int t;

	do {
		if ((t = binsReadByte(b)) < 0) return 0xffffffff; // FIXME! replace with some other errcode
		v += ((t & 0x7f) << o);
		o += 7;
	} while (t & 0x80);
	return v;
}

int CSphIndex_VLN::binsRead ( int b, CSphFatHit * e )
{
	DWORD r;

	// expected EOB
	if ( bins[b]->m_iDone )
	{
		e->m_iWordID = 0;
		return 1;
	}

	CSphFatHit & tHit = bins[b]->m_tLastHit; // shortcut
	for ( ;; )
	{
		// unexpected EOB
		if ( (r = binsReadVLB(b)) == 0xffffffffUL )
		{
			return 0;
		}

		if ( r )
		{
			switch ( bins[b]->m_eState )
			{
				case BIN_WORD:
					tHit.m_iWordID += r;
					tHit.m_iGroupID = 0;
					tHit.m_iDocID = 0;
					tHit.m_iWordPos = 0;
					tHit.m_iTimestamp = 0;
					bins[b]->m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					bins[b]->m_eState = BIN_POS;
					tHit.m_iDocID += r;
					tHit.m_iWordPos = 0;
					tHit.m_iGroupID = binsReadVLB ( b );
					tHit.m_iTimestamp = binsReadVLB ( b );
					if ( tHit.m_iGroupID==0xffffffffUL || tHit.m_iGroupID==0xffffffffUL )
						return 0; // read unexpected EOB
					break;

				case BIN_POS: 
					tHit.m_iWordPos += r;
					*e = tHit;
					return 1;

				default:
					sphDie ( "FATAL: INTERNAL ERROR: unknown bin state (state=%d).\n", bins[b]->m_eState );
			}
		} else
		{
			switch ( bins[b]->m_eState )
			{
				case BIN_POS:	bins[b]->m_eState = BIN_DOC; break;
				case BIN_DOC:	bins[b]->m_eState = BIN_WORD; break;
				case BIN_WORD:	bins[b]->m_iDone = 1; e->m_iWordID = 0; return 1;
				default:		sphDie ( "FATAL: INTERNAL ERROR: unknown bin state (state=%d).\n", bins[b]->m_eState );
			}
		}
	}
}


int CSphIndex_VLN::cidxCreate ()
{
	char sBuf [ SPH_MAX_FILENAME_LEN ];

	sprintf ( sBuf, "%s.spi", m_sFilename );
	fdIndex = new CSphWriter_VLN ( sBuf );
	if ( fdIndex->OpenFile()<0 )
		return 0;

	sprintf ( sBuf, "%s.spd", m_sFilename );
	fdData = new CSphWriter_VLN ( sBuf );
	if ( fdData->OpenFile()<0 )
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


void CSphIndex_VLN::cidxHit ( CSphFatHit * hit )
{
	assert (
		( hit->m_iWordID && hit->m_iWordPos && hit->m_iDocID && hit->m_iGroupID ) || // it's either ok hit
		( !hit->m_iWordID && !hit->m_iWordPos ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	if ( m_tLastHit.m_iWordID!=hit->m_iWordID )
	{
		// close prev hitlist, if any
		if ( m_tLastHit.m_iWordPos )
		{
			fdData->ZipInt ( 0 );
			m_tLastHit.m_iWordPos = 0;
		}

		// flush prev doclist, if any
		if ( m_dDoclist.GetLength() )
		{
			// finish writing wordlist entry
			assert ( fdData->m_iPos > m_iLastDoclistPos );
			assert ( m_iLastWordDocs );
			assert ( m_iLastWordHits );

			fdIndex->ZipOffset ( fdData->m_iPos - m_iLastDoclistPos );
			fdIndex->ZipInt ( m_iLastWordDocs );
			fdIndex->ZipInt ( m_iLastWordHits );

			m_iLastDoclistPos = fdData->m_iPos;
			m_iLastWordDocs = 0;
			m_iLastWordHits = 0;

			// write doclist
			fdData->ZipOffsets ( &m_dDoclist );
			fdData->ZipInt ( 0 );
			m_dDoclist.Reset ();

			// restart doclist deltas
			m_tLastHit.m_iDocID = 0;
			m_iLastHitlistPos = 0;
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
		if ( m_iLastPageID!=iPageID )
		{
			// close wordlist
			fdIndex->ZipInt ( 0 );

			// restart wordlist deltas
			m_tLastHit.m_iWordID = 0; 
			m_iLastDoclistPos = 0;

			// next map page
			m_iLastPageID = iPageID;
			cidxPagesDir [ iPageID ] = fdIndex->m_iPos;
		}

		// begin writing wordlist entry
		assert ( hit->m_iWordID > m_tLastHit.m_iWordID );
		fdIndex->ZipInt ( hit->m_iWordID - m_tLastHit.m_iWordID );
		m_tLastHit.m_iWordID = hit->m_iWordID;
	}

	////////////
	// next doc
	////////////

	if ( m_tLastHit.m_iDocID!=hit->m_iDocID )
	{
		// close prev hitlist, if any
		if ( m_tLastHit.m_iWordPos )
		{
			fdData->ZipInt ( 0 );
			m_tLastHit.m_iWordPos = 0;
		}

		// add new doclist entry for new doc id
		assert ( hit->m_iDocID > m_tLastHit.m_iDocID );
		assert ( fdData->m_iPos > m_iLastHitlistPos );

		m_dDoclist.Add ( hit->m_iDocID - m_tLastHit.m_iDocID );
		m_dDoclist.Add ( hit->m_iGroupID ); // R&D: maybe some delta-coding would help here too
		m_dDoclist.Add ( hit->m_iTimestamp );
		m_dDoclist.Add ( fdData->m_iPos - m_iLastHitlistPos );

		m_tLastHit.m_iDocID = hit->m_iDocID;
		m_iLastHitlistPos = fdData->m_iPos;

		// update per-word stats
		m_iLastWordDocs++;
	}

	///////////
	// the hit
	///////////

	// add hit delta
	assert ( hit->m_iWordPos > m_tLastHit.m_iWordPos );
	fdData->ZipInt ( hit->m_iWordPos - m_tLastHit.m_iWordPos );
	m_tLastHit.m_iWordPos = hit->m_iWordPos;
	m_iLastWordHits++;
}


void CSphIndex_VLN::cidxDone()
{
	fdIndex->SeekTo ( 0 );
	fdIndex->PutRawBytes ( &m_tHeader, sizeof(m_tHeader) );
	fdIndex->PutBytes ( cidxPagesDir, sizeof(cidxPagesDir) );
	
	fdIndex->CloseFile ();
	fdData->CloseFile ();

	if ( fdIndex->IsError() || fdData->IsError() )
		fprintf ( stderr, "ERROR: write() failed, out of disk space?\n" );

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


static inline int iLog2 ( DWORD iValue )
{
	int iBits = 0;
	while ( iValue )
	{
		iValue >>= 1;
		iBits++;
	}
	return iBits;
}


inline bool operator < ( const CSphDocInfo & a, const CSphDocInfo & b )
{
	return a.m_iDocID < b.m_iDocID;
};


int CSphIndex_VLN::cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iCount )
{
	assert ( pHit );
	assert ( iCount>0 );

	/////////////////////////////
	// do simple bitwise hashing
	/////////////////////////////

	m_dDocinfos.Sort();
	DWORD iStartID = m_dDocinfos[0].m_iDocID;

	static const int HBITS = 11;
	static const int HSIZE = (1<<HBITS);

	int dHash [ HSIZE+1 ];
	int iBits = iLog2 ( m_dDocinfos.Last().m_iDocID - iStartID );
	int iShift = ( iBits<HBITS ) ? 0 : ( iBits-HBITS );

	#ifndef NDEBUG
	for ( int i=0; i<=HSIZE; i++ )
		dHash[i] = -1;
	#endif

	dHash[0] = 0;
	int iHashed = 0;
	ARRAY_FOREACH ( i, m_dDocinfos )
	{
		int iHash = ( m_dDocinfos[i].m_iDocID - iStartID ) >> iShift;
		assert ( iHash>=0 && iHash<HSIZE );

		if ( iHash>iHashed )
		{
			dHash [ iHashed+1 ] = i-1; // right boundary for prev hash value
			dHash [ iHash ] = i; // left boundary for next hash value
			iHashed = iHash;
		}
	}
	dHash [ iHashed+1 ] = m_dDocinfos.GetLength()-1; // right boundary for last hash value

	///////////////////////////////////////
	// encode through a small write buffer
	///////////////////////////////////////

	BYTE buf[65536+1024], *pBuf, *maxP;
	int n = 0, w;
	DWORD d1, d2, d3, l1=0, l2=0, l3=0;

	pBuf = &buf[0];
	maxP = &buf[65536-1];
	CSphDocInfo * pDoc = &m_dDocinfos[0];
	while ( iCount-- )
	{
		if ( pDoc->m_iDocID!=pHit->m_iDocID )
		{
			int iHash = ( pHit->m_iDocID - iStartID ) >> iShift;
			assert ( iHash>=0 && iHash<HSIZE );

			CSphDocInfo * pStart = &m_dDocinfos [ dHash[iHash] ];
			CSphDocInfo * pEnd = &m_dDocinfos [ dHash[iHash+1] ];

			if ( pHit->m_iDocID==pStart->m_iDocID )
			{
				pDoc = pStart;
			} else if ( pHit->m_iDocID==pEnd->m_iDocID )
			{
				pDoc = pEnd;
			} else
			{
				pDoc = NULL;
				while ( pEnd-pStart>1 )
				{
					// check if nothing found
					if ( pHit->m_iDocID<pStart->m_iDocID || pHit->m_iDocID>pEnd->m_iDocID )
						break;
					assert ( pHit->m_iDocID>pStart->m_iDocID );
					assert ( pHit->m_iDocID<pEnd->m_iDocID );

					CSphDocInfo * pMid = pStart + (pEnd-pStart)/2;
					if ( pHit->m_iDocID==pMid->m_iDocID )
					{
						pDoc = pMid;
						break;
					}
					if ( pHit->m_iDocID<pMid->m_iDocID )
						pEnd = pMid;
					else
						pStart = pMid;
				}
			}
		}
		assert ( pDoc );
		assert ( pDoc->m_iDocID==pHit->m_iDocID );
		assert ( pDoc->m_iGroupID );
		assert ( pDoc->m_iTimestamp );

		// calc deltas
		d1 = pHit->m_iWordID - l1;
		d2 = pHit->m_iDocID - l2;
		d3 = pHit->m_iWordPos - l3;

		// non-zero delta restarts all the fields after it
		// because their deltas might now be negative
		if ( d1 ) d2 = pHit->m_iDocID;
		if ( d2 ) d3 = pHit->m_iWordPos;

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 ) pBuf += encodeVLB ( pBuf, 0 );

		// encode deltas
		if ( d1 ) pBuf += encodeVLB ( pBuf, d1 ); // encode word delta
		if ( d2 )
		{
			pBuf += encodeVLB ( pBuf, d2 ); // encode doc id (whole or delta)
			pBuf += encodeVLB ( pBuf, pDoc->m_iGroupID ); // encode group id
			pBuf += encodeVLB ( pBuf, pDoc->m_iTimestamp ); // encode timestamp
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
			w = (int)(pBuf - buf);
			if ( sphWrite ( fd, buf, w, "rawlog" )!=w )
			{
				m_dDocinfos.Reset ();
				return -1;
			}
			n += w;
			pBuf = buf;
		}
	}
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	w = (int)(pBuf - buf);
	if ( sphWrite ( fd, buf, w, "rawlog" )!=w )
	{
		m_dDocinfos.Reset ();
		return -1;
	}
	n += w;

	m_dDocinfos.Reset ();
	return n;
}

/////////////////////////////////////////////////////////////////////////////

/// hit priority queue entry
struct CSphHitQueueEntry : public CSphFatHit
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
	void Push ( CSphFatHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		m_pData [ m_iUsed ].m_iDocID = tHit.m_iDocID;
		m_pData [ m_iUsed ].m_iGroupID = tHit.m_iGroupID;
		m_pData [ m_iUsed ].m_iTimestamp = tHit.m_iTimestamp;
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


int CSphIndex_VLN::Build ( CSphDict * pDict, CSphSource * pSource, int iMemoryLimit )
{
	PROFILER_INIT ();

	/////////////////
	// build raw log
	/////////////////

	// set dictionary
	pSource->SetDict ( pDict );

	// create raw log
	fdRaw = OpenFile ( "spr", O_CREAT | O_RDWR | O_TRUNC );
	if ( fdRaw<0 )
		return 0;

	// adjust memory requirements
	const int MIN_MEM_LIMIT = sizeof(CSphWordHit)*1048576;
	bool bRelimit = false;
	int iOldLimit = 0;

	if ( iMemoryLimit<MIN_MEM_LIMIT )
	{
		iOldLimit = iMemoryLimit;
		iMemoryLimit = MIN_MEM_LIMIT;
		bRelimit = true;
	}

	iMemoryLimit = ( ( (iMemoryLimit+32768) >> 16 ) << 16 ); // round to 64K
	int iRawBlockSize = iMemoryLimit / sizeof(CSphWordHit);

	if ( bRelimit && iOldLimit )
	{
		fprintf ( stderr, "WARNING: collect_hits: mem_limit=%d kb too low, increasing to %d kb.\n",
			iOldLimit/1024, iMemoryLimit/1024 );
	}

	// allocate raw block
	int iRawBlockUsed = 0;
	SphOffset_t iRawHits = 0;
	CSphWordHit * dRawBlock = new CSphWordHit [ iRawBlockSize ];
	CSphWordHit * pRawBlock = dRawBlock;
	assert ( dRawBlock );

	// accumulate docinfo IDs range
	m_tHeader.m_tMin.m_iDocID = UINT_MAX;
	m_tHeader.m_tMin.m_iGroupID = UINT_MAX;
	m_tHeader.m_tMin.m_iTimestamp = UINT_MAX;

	// build raw log
	PROFILE_BEGIN ( collect_hits );

	CSphIndexProgress tProgress;
	tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;

	DWORD iDocID;
	while ( ( iDocID = pSource->Next() )!=0 )
	{
		// progress bar
		if ( m_pProgress
			&& ( ( pSource->GetStats()->m_iTotalDocuments % 1000 )==0 ) )
		{
			tProgress.m_iDocuments = pSource->GetStats()->m_iTotalDocuments;
			tProgress.m_iBytes = pSource->GetStats()->m_iTotalBytes;
			m_pProgress ( &tProgress );
		}

		// check/store docinfo
		int iHitCount = pSource->m_dHits.GetLength ();
		if ( iHitCount<=0 )
			continue;
		if ( !m_dDocinfos.GetLength() || m_dDocinfos.Last().m_iDocID!=pSource->m_tDocInfo.m_iDocID )
			m_dDocinfos.Add ( pSource->m_tDocInfo );
	
		// store hits
		assert ( pSource->m_tDocInfo.m_iDocID );
		assert ( pSource->m_tDocInfo.m_iGroupID );
		assert ( pSource->m_tDocInfo.m_iTimestamp );
		m_tHeader.m_tMin.m_iDocID = Min ( m_tHeader.m_tMin.m_iDocID, pSource->m_tDocInfo.m_iDocID );
		m_tHeader.m_tMin.m_iGroupID = Min ( m_tHeader.m_tMin.m_iGroupID, pSource->m_tDocInfo.m_iGroupID );
		m_tHeader.m_tMin.m_iTimestamp = Min ( m_tHeader.m_tMin.m_iTimestamp, pSource->m_tDocInfo.m_iTimestamp );

		CSphWordHit * pHit = &pSource->m_dHits[0];
		while ( iHitCount-- )
		{
			assert ( pHit->m_iDocID==pSource->m_tDocInfo.m_iDocID );
			assert ( pHit->m_iWordID );
			assert ( pHit->m_iWordPos );

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
				m_dDocinfos.Reset ();
				m_dDocinfos.Add ( pSource->m_tDocInfo );

				// progress bar
				if ( m_pProgress )
				{
					tProgress.m_iDocuments = pSource->GetStats()->m_iTotalDocuments;
					tProgress.m_iBytes = pSource->GetStats()->m_iTotalBytes;
					m_pProgress ( &tProgress );
				}
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
	m_dDocinfos.Reset ();

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
	if ( fdRaw<0 )
		return 0;

	// create new compressed index
	if ( !cidxCreate() )
		return 0;

	// adjust min IDs, and fill header
	m_tHeader.m_iFieldCount = pSource->GetFieldCount ();

	assert ( m_tHeader.m_tMin.m_iDocID>0 );
	assert ( m_tHeader.m_tMin.m_iGroupID>0 );
	assert ( m_tHeader.m_tMin.m_iTimestamp>0 );
	m_tHeader.m_tMin.m_iDocID--;
	m_tHeader.m_tMin.m_iGroupID--;
	m_tHeader.m_tMin.m_iTimestamp--;

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
	CSphFatHit tHit;

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
		tQueue.m_pData->m_iDocID -= m_tHeader.m_tMin.m_iDocID;
		tQueue.m_pData->m_iGroupID -= m_tHeader.m_tMin.m_iGroupID;
		tQueue.m_pData->m_iTimestamp -= m_tHeader.m_tMin.m_iTimestamp;
		cidxHit ( tQueue.m_pData );
		if ( fdIndex->IsError() || fdData->IsError() )
		{
			fprintf ( stderr, "ERROR: write() failed, out of disk space?\n" );
			return 0;
		}

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

	CSphFatHit tFlush;
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

/// generic comparison functor
template< typename T > struct Less_T
{
	inline bool operator() ( const T & x, const T & y ) const
	{
		return x<y;
	}
};


/// generic comparison functor instantiation
template< typename T> Less_T<T> Less_fn ( T & )
{
	return Less_T<T> ();
}


/// match-sorting priority min-queue
template< typename T, int SIZE, class COMP > class CSphQueue : public ISphQueue<T, SIZE>
{
protected:
	T		m_pData [ SIZE ];
	int		m_iUsed;

public:
	/// ctor
	CSphQueue () : m_iUsed ( 0 ) {}

	/// add entry to the queue
	virtual void Push ( const T & tEntry )
	{
		if ( m_iUsed==SIZE )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP() ( tEntry, m_pData[0] ) )
				return;
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
			if ( !COMP() ( m_pData[iEntry], m_pData[iParent] ) )
				break;

			// entry is less than parent, should float to the top
			Swap ( m_pData[iEntry], m_pData[iParent] );
			iEntry = iParent;
		}
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
				if ( COMP() ( m_pData[iChild+1], m_pData[iChild] ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( COMP() ( m_pData[iChild], m_pData[iEntry] ) )
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


/// my simple query parser
struct CSphQueryParser : CSphSource_Text
{
	char *query;
	char *words[SPH_MAX_QUERY_WORDS];
	int numWords;

	CSphQueryParser ( CSphDict * pDict, const char * sQuery, ISphTokenizer * pTokenizer )
	{
		for ( int i=0; i<SPH_MAX_QUERY_WORDS; i++ )
			words[i] = NULL;

		SetTokenizer ( pTokenizer );

		numWords = 0;
		query = sphDup ( sQuery );
		m_pDict = pDict;
		m_bCallWordCallback = true;
		Next ();
	}

	~CSphQueryParser()
	{
		int i;

		for (i = 0; i < SPH_MAX_QUERY_WORDS; i++)
			if (words[i]) sphFree(words[i]);
	}

	virtual void WordCallback ( char * word )
	{
		if (numWords < SPH_MAX_QUERY_WORDS)
			this->words[numWords++] = sphDup(word);
	}

	BYTE * NextText()
	{
		m_tDocInfo.m_iDocID = 1;
		m_tDocInfo.m_iGroupID = 1;
		m_tDocInfo.m_iTimestamp = 1;
		return (BYTE*)this->query;
	}
};


/// match sorter
struct MatchRelevanceLt_fn
{
	inline bool operator() ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return ( a.m_iWeight==b.m_iWeight )
			? ( a.m_iTimestamp < b.m_iTimestamp )
			: ( a.m_iWeight < b.m_iWeight );
	};
};


/// match sorter
struct MatchDateLt_fn
{
	inline bool operator() ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return ( a.m_iTimestamp==b.m_iTimestamp )
			? ( a.m_iWeight < b.m_iWeight )
			: ( a.m_iTimestamp < b.m_iTimestamp );
	};
};


/// match sorter
struct MatchDateGt_fn
{
	inline bool operator() ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return ( a.m_iTimestamp==b.m_iTimestamp )
			? ( a.m_iWeight < b.m_iWeight )
			: ( a.m_iTimestamp > b.m_iTimestamp );
	};
};


/// match sorter
struct MatchTimeSegments_fn
{
public:
	static DWORD m_iNow;

public:
	/// comparator
	inline bool operator() ( const CSphMatch & a, const CSphMatch & b ) const
	{
		int iA = GetSegment ( a.m_iTimestamp );
		int iB = GetSegment ( b.m_iTimestamp );
		if ( iA==iB )
		{
			if ( a.m_iWeight==b.m_iWeight )
				return ( a.m_iTimestamp < b.m_iTimestamp );
			else
				return ( a.m_iWeight < b.m_iWeight );
		} else
		{
			return ( iA > iB );
		}
	};

protected:
	/// calc time segment
	inline int GetSegment ( DWORD iStamp ) const
	{
		if ( iStamp>=m_iNow-3600 ) return 0; // last hour
		if ( iStamp>=m_iNow-24*3600 ) return 1; // last day
		if ( iStamp>=m_iNow-7*24*3600 ) return 2; // last week
		if ( iStamp>=m_iNow-30*24*3600 ) return 3; // last month
		if ( iStamp>=m_iNow-90*24*3600 ) return 4; // last 3 months
		return 5; // everything else
	}
};
DWORD MatchTimeSegments_fn::m_iNow = time ( NULL );


/// qsort query-word comparator
int cmpQueryWord ( const void * a, const void * b )
{
	return ((CSphQueryWord*)a)->m_iDocs - ((CSphQueryWord*)b)->m_iDocs;
}


/// qsort dword comparator
int cmpDword ( const void * a, const void * b )
{
	return *((DWORD*)a) - *((DWORD*)b);
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


inline int sphBitCount ( DWORD n )
{
	// MIT HACKMEM count
	// works for 32-bit numbers only
	// fix last line for 64-bit numbers
	register DWORD tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}


inline int sphGroupMatch ( DWORD iGroup, DWORD * pGroups, int iGroups )
{
	if ( !pGroups ) return 1;

	DWORD * pA = pGroups;
	DWORD * pB = pGroups+iGroups-1;
	if ( iGroup==*pA || iGroup==*pB ) return 1;
	if ( iGroup<(*pA) || iGroup>(*pB) ) return 0;

	while ( pB-pA>1 )
	{
		DWORD * pM = pA + ((pB-pA)/2);
		if ( iGroup==(*pM) )
			return 1;
		if ( iGroup<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return 0;
}


ISphMatchQueue * sphCreateQueue ( CSphQuery * pQuery )
{
	ISphMatchQueue * pTop = NULL;
	switch ( pQuery->m_eSort )
	{
		case SPH_SORT_DATE_DESC:	pTop = new CSphQueue < CSphMatch, CSphQueryResult::MAX_MATCHES, MatchDateLt_fn > (); break;
		case SPH_SORT_DATE_ASC:		pTop = new CSphQueue < CSphMatch, CSphQueryResult::MAX_MATCHES, MatchDateGt_fn > (); break;
		case SPH_SORT_TIME_SEGMENTS:pTop = new CSphQueue < CSphMatch, CSphQueryResult::MAX_MATCHES, MatchTimeSegments_fn > (); break;
		case SPH_SORT_RELEVANCE:	pTop = new CSphQueue < CSphMatch, CSphQueryResult::MAX_MATCHES, MatchRelevanceLt_fn > (); break;
		default:
			pTop = new CSphQueue < CSphMatch, CSphQueryResult::MAX_MATCHES, MatchRelevanceLt_fn > ();
			fprintf ( stderr, "WARNING: unknown sorting mode '%d', using SPH_SORT_RELEVANCE\n", pQuery->m_eSort );
			break;
	}

	assert ( pTop );
	return pTop;
}


void sphFlattenQueue ( ISphMatchQueue * pQueue, CSphQueryResult * pResult )
{
	pResult->m_dMatches.Resize ( pQueue->GetLength() );
	int iDest = pQueue->GetLength();
	while ( iDest-- )
	{
		pResult->m_dMatches [ iDest ] = pQueue->Root ();
		pQueue->Pop ();
	}
}


CSphQueryResult * CSphIndex_VLN::Query ( CSphDict * pDict, CSphQuery * pQuery )
{
	// create result and queue
	CSphQueryResult * pResult = new CSphQueryResult();
	ISphMatchQueue * pTop = sphCreateQueue ( pQuery );

	// run query
	QueryEx ( pDict, pQuery, pResult, pTop );

	// convert results and return
	sphFlattenQueue ( pTop, pResult );
	SafeDelete ( pTop );
	return pResult;
}


bool CSphIndex_VLN::QueryEx ( CSphDict * dict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchQueue * pTop )
{
	assert ( dict );
	assert ( pQuery );
	assert ( pQuery->m_pTokenizer );
	assert ( pResult );
	assert ( pTop );

	CSphQueryWord qwords[SPH_MAX_QUERY_WORDS];
	int i, j, nwords, weights [ SPH_MAX_FIELD_COUNT ], imin, nweights;
	DWORD wordID, docID, pmin, k;

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	// create result and start timing
	pResult->m_fQueryTime -= sphLongTimer ();

	// split query into words
	CSphQueryParser * pQueryParser = new CSphQueryParser ( dict, pQuery->m_sQuery,
		pQuery->m_pTokenizer );
	assert ( pQueryParser );

	nwords = Min ( pQueryParser->m_dHits.GetLength (), SPH_MAX_QUERY_WORDS );
	for ( i=0; i<nwords; i++ )
	{
		qwords[i].m_sWord = sphDup ( pQueryParser->words[i] );
		qwords[i].m_iWordID = pQueryParser->m_dHits[i].m_iWordID;
		qwords[i].m_iQueryPos = 1+i;
	}
	SafeDelete ( pQueryParser );

	if ( !nwords )
	{
		pResult->m_fQueryTime += sphLongTimer ();
		return true;
	}

	// open files
	char sTmp [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sTmp, sizeof(sTmp), "%s.spi", m_sFilename );
	CSphAutofile tWordlist ( sTmp );

	snprintf ( sTmp, sizeof(sTmp), "%s.spd", m_sFilename );
	CSphAutofile tDoclist ( sTmp );

	if ( tWordlist.m_iFD<0 || tDoclist.m_iFD<0 )
		return false;

	PROFILE_END ( query_init );
	PROFILE_BEGIN ( query_load_dir );

	// load index pages directory
	CSphReader_VLN rdIndex;
	rdIndex.SetFile ( tWordlist.m_iFD );
	rdIndex.GetRawBytes ( &m_tHeader, sizeof(m_tHeader) );
	rdIndex.GetBytes ( cidxPagesDir, sizeof(cidxPagesDir) );

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

	// reorder groups
	if ( pQuery->m_pGroups )
		qsort ( pQuery->m_pGroups, pQuery->m_iGroups, sizeof(DWORD), cmpDword );

	PROFILE_END ( query_load_words );

	// build weights
	nweights = m_tHeader.m_iFieldCount;
	for ( i=0; i<nweights; i++ ) // defaults
		weights[i] = 1;
	if ( pQuery->m_pWeights ) // user-supplied
		for ( int i=0; i<Min ( nweights, pQuery->m_iWeights ); i++ )
			weights[i] = Max ( 1, pQuery->m_pWeights[i] );

	// find and proximity-weight matching documents
	i = docID = 0;
	pResult->m_iTotalMatches = 0;

	PROFILE_BEGIN ( query_match );
	if ( pQuery->m_eMode==SPH_MATCH_ALL || pQuery->m_eMode==SPH_MATCH_PHRASE )
	{
		///////////////////
		// match all words
		///////////////////

		if ( !qwords[0].m_iDocs )
		{
			pResult->m_fQueryTime += sphLongTimer ();
			return true;
		}

		// preload doclist entries
		for ( i=0; i<nwords; i++ )
			qwords[i].GetDoclistEntry ();
		i = 0;

		for ( ;; )
		{
			// scan lists until *all* the ids match
			while ( qwords[i].m_tDoc.m_iDocID && docID>qwords[i].m_tDoc.m_iDocID )
				qwords[i].GetDoclistEntry ();
			if ( !qwords[i].m_tDoc.m_iDocID )
				break;

			if ( docID<qwords[i].m_tDoc.m_iDocID )
			{
				docID = qwords[i].m_tDoc.m_iDocID;
				i = 0;
				continue;
			}
			if (++i != nwords)
				continue;

			// preload hitlist entries
			for ( i=0; i<nwords; i++ )
				qwords[i].GetHitlistEntry ();

			// early reject by group id
			if ( !sphGroupMatch ( qwords[0].m_tDoc.m_iGroupID + m_tHeader.m_tMin.m_iGroupID,
				pQuery->m_pGroups, pQuery->m_iGroups ) )
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
				assert ( j>=0 && j<(int)m_tHeader.m_iFieldCount );
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

			// check if there was a perfect match
			if ( pQuery->m_eMode==SPH_MATCH_PHRASE && nwords>1 )
			{
				for ( i=0; i<nweights; i++ )
				{
					if ( phraseWeight[i]==nwords-1 )
						break;
				}
				if ( i==nweights )
				{
					// there was not. continue
					i = 0;
					docID++;
					continue;
				}
			}

			// sum simple match weights and phrase match weights
			for ( i=0, j=0; i<nweights; i++ )
				j += weights[i] * ( matchWeight[i] + phraseWeight[i] );

			// unpack match, set weight and push it to the queue
			CSphMatch tMatch;
			tMatch.m_iDocID = docID + m_tHeader.m_tMin.m_iDocID;
			tMatch.m_iGroupID = qwords[0].m_tDoc.m_iGroupID + m_tHeader.m_tMin.m_iGroupID;
			tMatch.m_iTimestamp = qwords[0].m_tDoc.m_iTimestamp + m_tHeader.m_tMin.m_iTimestamp;
			tMatch.m_iWeight = j; // set weight
			pTop->Push ( tMatch );
			pResult->m_iTotalMatches++;

			// continue looking for next matches
			i = 0;
			docID++;
		}

	} else if ( pQuery->m_eMode==SPH_MATCH_ANY )
	{
		//////////////////
		// match any word
		//////////////////

		int iActive = nwords; // total number of words still active
		DWORD iLastMatchID = 0;

		// preload entries
		for ( i=0; i<nwords; i++ )
			qwords[i].GetDoclistEntry ();

		// find a multiplier for phrase match
		// so that it will weight more than any word match
		int iPhraseK = 0;
		for ( i=0; i<nweights; i++ )
			iPhraseK += weights[i] * nwords;

		for ( ;; )
		{
			// update active pointers to document lists, kill empty ones,
			// and get min current document id
			CSphDocInfo tMatchDoc;
			tMatchDoc.m_iDocID = UINT_MAX;
			tMatchDoc.m_iGroupID = 0;
			tMatchDoc.m_iTimestamp = 0;

			for ( i=0; i<iActive; i++ )
			{
				// move to next document
				if ( qwords[i].m_tDoc.m_iDocID==iLastMatchID && iLastMatchID )
				{
					qwords[i].GetDoclistEntry ();
					qwords[i].GetHitlistEntry ();
				}

				// remove emptied words
				if ( !qwords[i].m_tDoc.m_iDocID )
				{
					qwords[i] = qwords[iActive-1];
					i--;
					iActive--;
					continue;
				}

				// get new min id
				assert ( qwords[i].m_tDoc.m_iDocID>iLastMatchID );
				if ( qwords[i].m_tDoc.m_iDocID<tMatchDoc.m_iDocID )
					tMatchDoc = qwords[i].m_tDoc;
			}
			if ( iActive==0 )
				break;

			iLastMatchID = tMatchDoc.m_iDocID;
			assert ( iLastMatchID!=0 );
			assert ( iLastMatchID!=UINT_MAX );

			// early reject by group id
			tMatchDoc.m_iGroupID += m_tHeader.m_tMin.m_iGroupID;
			if ( !sphGroupMatch ( tMatchDoc.m_iGroupID, pQuery->m_pGroups, pQuery->m_iGroups ) )
				continue;

			// get the words we're matching current document against (let's call them "terms")
			CSphQueryWord * pHit [ SPH_MAX_QUERY_WORDS ];
			int iTerms = 0;

			for ( i=0; i<iActive; i++ )
				if ( qwords[i].m_tDoc.m_iDocID==iLastMatchID )
					pHit[iTerms++] = &qwords[i];
			assert ( iTerms>0 );

			// init weighting
			struct
			{
				int		m_iCurPhrase;	// current phrase-match weight
				int		m_iMaxPhrase;	// max phrase-match weight
				DWORD	m_uMatch;		// matching terms bitmap
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
			for ( i=0, j=0; i<nweights; i++ )
				j += weights[i] * ( sphBitCount(dWeights[i].m_uMatch) + dWeights[i].m_iMaxPhrase*iPhraseK );

			// unpack match, set weight and push it to the queue
			CSphMatch tMatch;
			tMatch.m_iDocID = iLastMatchID + m_tHeader.m_tMin.m_iDocID;
			tMatch.m_iGroupID = tMatchDoc.m_iGroupID; // group id already unpacked
			tMatch.m_iTimestamp = tMatchDoc.m_iTimestamp + m_tHeader.m_tMin.m_iTimestamp;
			tMatch.m_iWeight = j; // set weight
			pTop->Push ( tMatch );
			pResult->m_iTotalMatches++;
		}
	} else
	{
		sphDie ( "FATAL: INTERNAL ERROR: unknown matching mode (mode=%d).\n", pQuery->m_eMode );
	}
	PROFILE_END ( query_match );

	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_fQueryTime += sphLongTimer ();
	return true;
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
		stem_ru_cp1251 ( pWord ); // FIXME! !COMMIT

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
		ISphTokenizer * pTokenizer = new CSphTokenizer_SBCS (); // FIXME! !COMMIT
		CSphVector<DWORD> dStop;
		int iLength;
		do
		{
			iLength = (int)fread ( sBuffer, 1, sizeof(sBuffer), fp );
			pTokenizer->SetBuffer ( sBuffer, iLength, false );

			BYTE * pToken;
			while ( ( pToken = pTokenizer->GetToken() )!=NULL )
				dStop.Add ( GetWordID ( pToken ) );
		} while ( iLength );

		// sort stopwords
		dStop.Sort();

		// store IDs
		m_iStopwords = dStop.GetLength ();
		m_pStopwords = new DWORD [ m_iStopwords ];
		memcpy ( m_pStopwords, &dStop[0], sizeof(DWORD)*m_iStopwords );
		if ( !g_bSphQuiet )
			fprintf ( stdout, "- loaded %d stopwords from '%s'\n",
				m_iStopwords, sName ); // FIXME! do loglevels

		// close file
		fclose ( fp );
	}

	sphFree ( sList );
}

/////////////////////////////////////////////////////////////////////////////
// HTML STRIPPER
/////////////////////////////////////////////////////////////////////////////

CSphHTMLStripper::CSphHTMLStripper ()
	: m_pTags ( NULL )
{
}


CSphHTMLStripper::~CSphHTMLStripper ()
{
	// kill tags list
	while ( m_pTags )
	{
		TagLink_t * pKill = m_pTags;
		m_pTags = m_pTags->m_pNext;
		SafeDelete ( pKill );
	}
}


void CSphHTMLStripper::AddAttr ( const char * sTag, const char * sAttr )
{
	// find matching tag in list
	TagLink_t * pTag = m_pTags;
	TagLink_t * pPrev = NULL;
	while ( pTag && strcasecmp ( pTag->m_sTag, sTag ) )
	{
		pPrev = pTag;
		pTag = pTag->m_pNext;
	}

	// chain to the list if not found
	if ( !pTag )
	{
		pTag = new TagLink_t ( sTag );
		if ( pPrev )
			pPrev->m_pNext = pTag;
		else
			m_pTags = pTag;
	}

	// find matching attr in list
	AttrLink_t * pAttr = pTag->m_pAttrs;
	AttrLink_t * pPrevA = NULL;
	while ( pAttr && strcasecmp ( pAttr->m_sAttr, sAttr ) )
	{
		pPrevA = pAttr;
		pAttr = pAttr->m_pNext;
	}

	// chain to the list if not found
	if ( !pAttr )
	{
		pAttr = new AttrLink_t ( sAttr );
		if ( pPrevA )
			pPrevA->m_pNext = pAttr;
		else
			pTag->m_pAttrs = pAttr;
	}
}


const char * CSphHTMLStripper::SetAttrs ( const char * sConfig )
{
	char sTag[256], sAttr[256];

	const char * p = sConfig, * s;
	while ( *p )
	{
		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( !*p ) break;

		// check tag name
		s = p; while ( isalpha(*p) ) p++;
		if ( s==p ) return s; // error: non-alphas in tag name

		// get tag name
		if ( p-s>=(int)sizeof(sTag) ) return s; // error: tag name too long
		strncpy ( sTag, s, p-s );
		sTag[p-s] = '\0';

		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( !*p ) break;

		// check '='
		if ( *p++!='=' ) return p-1; // error: '=' expected

		// scan attributes
		while ( *p )
		{
			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check attr name
			s = p; while ( isalpha(*p) ) p++;
			if ( s==p ) return s; // error: non-alphas in attr name

			// get attr name
			if ( p-s>=(int)sizeof(sAttr) ) return s; // error: attr name too long
			strncpy ( sAttr, s, p-s );
			sAttr[p-s] = '\0';

			// add it
			strlwr ( sTag );
			strlwr ( sAttr );
			AddAttr ( sTag, sAttr );

			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check if there's next attr or tag
			if ( *p==',' ) { p++; continue; } // next attr
			if ( *p==';' ) { p++; break; } // next tag
			return p; // error: ',' or ';' or line end expected
		}
	}
	return NULL;
}


BYTE * RemoveQuoted ( BYTE * p )
{
	BYTE cEnd = *p;
	*p++ = ' ';
	while ( *p && *p!=cEnd )
		*p++ = ' ';
	if ( *p==cEnd )
		*p++ = ' ';
	return p;
}


BYTE * CSphHTMLStripper::Strip ( BYTE * sData )
{
	BYTE * p = sData;
	for ( ;; )
	{
		// scan for tag beginning
		while ( *p && *p!='<' ) p++;
		if ( !*p ) break;

		if ( !isalpha(p[1]) && p[1]!='/' && p[1]!='!' )
		{
			p++;
			continue;
		}
		*p++ = ' '; // remove '<'

		// check if it's a comment
		if ( p[0]=='!' && p[1]=='-' && p[2]=='-' )
		{
			// scan until comment end
			p[0] = p[1] = p[2] = ' '; p += 3; // clear closing '!--'
			while ( *p )
			{
				if ( p[0]=='-' && p[1]=='-' && p[2]=='>' )
					break;
				*p++ = ' ';
			}
			if ( !*p ) break;
			p[0] = p[1] = p[2] = ' '; p += 3; // clear closing '-->'
			continue;
		}

		// check if it's known tag or what
		TagLink_t * pTag;
		int iLen = 0;
		for ( pTag=m_pTags; pTag; pTag=pTag->m_pNext )
		{
			iLen = (int)strlen(pTag->m_sTag); // !COMMIT: OPTIMIZE
			if ( strncasecmp ( pTag->m_sTag, (char*)p, iLen ) )
				continue;
			if ( !isalpha ( p[iLen] ) )
				break;
		}

		// processing unknown tags is somewhat easy
		if ( !pTag )
		{
			// we just scan until EOLN or tag end
			while ( *p && *p!='>' )
			{
				if ( *p=='\'' || *p=='"' )
					p = RemoveQuoted ( p );
				else
					*p++ = ' ';
			}
			if ( *p ) *p++ = ' ';
			continue;;
		}

		// remove tag name
		while ( iLen-- ) *p++ = ' ';
		assert ( !isalpha(*p) );

		while ( *p && *p!='>' )
		{
			// skip non-alphas
			while ( *p && *p!='>' )
			{
				if ( *p=='\'' || *p=='"' )
				{
					p = RemoveQuoted ( p );
					while ( isspace(*p) ) p++;
				}
				if ( isalpha(*p) )
					break;
				*p++ = ' ';
			}
			if ( !isalpha(*p) )
			{
				if ( *p ) *p++ = ' ';
				break;
			}

			// check attribute name
			AttrLink_t * pAttr;
			int iLen2 = 0;
			for ( pAttr=pTag->m_pAttrs; pAttr; pAttr=pAttr->m_pNext )
			{
				iLen2 = (int)strlen(pAttr->m_sAttr); // !COMMIT: OPTIMIZE
				if ( strncasecmp ( pAttr->m_sAttr, (char*)p, iLen2 ) )
					continue;
				if ( p[iLen2]=='=' || isspace(p[iLen2]) )
					break;
			}

			// if attribute is unknown or malformed, we just skip all alphas and rescan
			if ( !pAttr )
			{
				while ( isalpha(*p) ) *p++ = ' ';
				continue;
			}

			// attribute is known
			while ( iLen2-- ) *p++ = ' ';

			// skip spaces, check for '=', and skip spaces again
			while ( isspace(*p) ) p++; if ( !*p ) break;
			if ( *p!='=' )
			{
				*p++ = ' ';
				break;
			}
			*p++ = ' ';
			while ( isspace(*p) ) p++;

			// handle quoted value
			if ( *p=='\'' || *p=='"' )
			{
				char cEnd = *p;
				*p++ = ' ';
				while ( *p && *p!=cEnd ) p++;
				if ( *p==cEnd ) *p++ = ' ';
				continue;
			}

			// handle unquoted value
			while ( *p && !isspace(*p) && *p!='>' ) p++;
		}
		if ( *p=='>' ) *p++ = ' ';
	}

	return sData;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource()
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_bStripHTML ( false )
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


const CSphSourceStats * CSphSource::GetStats ()
{
	return &m_iStats;
}


const char * CSphSource::SetStripHTML ( const char * sExtractAttrs )
{
	m_bStripHTML = ( sExtractAttrs!=NULL );
	if ( m_bStripHTML )
		return m_pStripper->SetAttrs ( sExtractAttrs );
	else
		return NULL;
}


void CSphSource::SetTokenizer ( ISphTokenizer * pTokenizer)
{
	assert ( pTokenizer );
	m_pTokenizer = pTokenizer;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////

int CSphSource_Document::Next()
{
	assert ( m_pTokenizer );
	PROFILE ( src_document );

	BYTE ** dFields = NextDocument();

	if ( !dFields || m_tDocInfo.m_iDocID<=0 )
		return 0;

	m_iStats.m_iTotalDocuments++;
	m_dHits.Reset ();

	for ( int j=0; j<m_iFieldCount; j++ )
	{
		BYTE * sField = dFields[j];
		if ( !sField )
			continue;

		if ( m_bStripHTML )
			m_pStripper->Strip ( sField );

		int iLen = (int) strlen ( (char*)sField );
		m_iStats.m_iTotalBytes += iLen;

		m_pTokenizer->SetBuffer ( sField, iLen, true );

		BYTE * sWord;
		int iPos = 1;

		while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
		{
			DWORD iWord = m_pDict->GetWordID ( sWord );
			if ( iWord )
			{
				CSphWordHit & tHit = m_dHits.Add ();
				tHit.m_iDocID = m_tDocInfo.m_iDocID;
				tHit.m_iWordID = iWord;
				tHit.m_iWordPos = (j << 24) | iPos++;
				if ( m_bCallWordCallback )
					WordCallback ( (char*) sWord );
			}
		}
	}

	return m_tDocInfo.m_iDocID;
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
	, m_sDateColumn		( NULL )
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
	, m_iDateColumn		( 0 )

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
	for ( ;; )
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
		m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
		if ( !m_pSqlResult )
		{
			sError = "mysql_use_result";
			break;
		}
		break;
	}

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


int CSphSource_MySQL::GetColumnIndex ( const char * sColumn )
{
	if ( !strlen(sColumn) )
		return 0;

	int iRes = atoi ( sColumn );
	if ( !iRes )
	{
		// if it's string, match by name
		iRes = -1;
		for ( int i=1; i<(int)mysql_num_fields(m_pSqlResult); i++ )
			if ( strcasecmp ( m_pSqlResult->fields[i].name, sColumn )==0 )
		{
			iRes = i;
			break;
		}
	} else
	{
		// if it's number, reindex from base 1 to base 0
		iRes--;
	}

	if ( iRes<=0 || iRes>=(int)mysql_num_fields(m_pSqlResult) )
	{
		fprintf ( stderr, "WARNING: bad column index (name='%s', index=%d), SUPPORT DISABLED.\n",
			sColumn, iRes );
		iRes = 0;
	} else
	{
		m_iFieldCount--;
		assert ( m_iFieldCount>0 );
	}
	return iRes;
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
	LOC_FIX_NULL ( m_sDateColumn );
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
	for ( ;; )
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

			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( m_pSqlResult )
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
			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( !m_pSqlResult )
				LOC_MYSQL_ERROR ( "mysql_use_result" );
		}
		break;
	}

	// errors, anyone?
	if ( sError )
	{
		fprintf ( stderr, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN );
		return 0;
	}

	// some post-query setup
	int iFields = mysql_num_fields ( m_pSqlResult ) - 1;
	m_iFieldCount = iFields; // will be changed by GetColumnIndex below
	m_sQueryPost = sphDup ( pParams->m_sQueryPost );

	// group/date columns
	m_iGroupColumn = GetColumnIndex ( pParams->m_sGroupColumn );
	m_iDateColumn = GetColumnIndex ( pParams->m_sDateColumn );

	// check it
	if ( m_iFieldCount>SPH_MAX_FIELD_COUNT )
	{
		fprintf ( stderr, "ERROR: too many fields (fields=%d, max=%d), please increase SPH_MAX_FIELD_COUNT in sphinx.h and recompile.\n",
			m_iFieldCount, SPH_MAX_FIELD_COUNT );
		return 0;
	}

	// build field remap table
	int j = 0;
	for ( int i=1; i<=iFields; i++ )
	{
		if ( i!=m_iGroupColumn && i!=m_iDateColumn )
			m_dRemapFields [ j++ ] = i;
	}
	assert ( j==m_iFieldCount );

	#undef LOC_FIX_NULL
	#undef LOC_ERROR
	#undef LOC_MYSQL_ERROR

	return 1;
}


BYTE ** CSphSource_MySQL::NextDocument ()
{
	PROFILE ( src_mysql );

	m_tSqlRow = mysql_fetch_row ( m_pSqlResult );

	// when the party's over...
	while ( !m_tSqlRow )
	{
		// is that an error?
		if ( mysql_errno ( &m_tSqlDriver ) )
			sphDie ( "FATAL: mysql_fetch_row: %s.\n", mysql_error ( &m_tSqlDriver ) );

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
			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( m_pSqlResult )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
			break;
		}

		return NULL;
	}

	// get him!
	m_tDocInfo.m_iDocID = atoi ( m_tSqlRow[0] );
	m_tDocInfo.m_iGroupID = m_iGroupColumn ? atoi ( m_tSqlRow [ m_iGroupColumn ] ) : 1;
	m_tDocInfo.m_iTimestamp = m_iDateColumn ? atoi ( m_tSqlRow [ m_iDateColumn ] ) : 1;

	if ( !m_tDocInfo.m_iDocID )
		fprintf ( stderr, "WARNING: zero/NULL document_id, aborting indexing\n" );

	if ( !m_tDocInfo.m_iGroupID )
	{
		m_tDocInfo.m_iGroupID = 1;
		fprintf ( stderr, "WARNING: zero/NULL group for document_id=%d, fixed up to 1\n", m_tDocInfo.m_iDocID );
	}

	if ( !m_tDocInfo.m_iTimestamp )
	{
		m_tDocInfo.m_iTimestamp = 1;
		fprintf ( stderr, "WARNING: zero/NULL timestamp for document_id=%d, fixed up to 1\n", m_tDocInfo.m_iDocID );
	}

	// remap and return
	for ( int i=0; i<m_iFieldCount; i++ )
		m_dFields[i] = (BYTE*) m_tSqlRow [ m_dRemapFields[i] ];
	return m_dFields;
}

#endif // USE_MYSQL

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


int CSphSource_XMLPipe::Next ()
{
	PROFILE ( src_xmlpipe );
	char sTitle [ 1024 ]; // FIXME?

	assert ( m_pPipe );
	assert ( m_pTokenizer );
	m_dHits.Reset ();

	/////////////////////////
	// parse document header
	/////////////////////////

	if ( !m_bBody )
	{
		// scan for opening '<document>' tag if necessary
		SetTag ( "document" );
		if ( !SkipTag ( true, false ) )
			return 0;

		if ( !ScanInt ( "id", &m_tDocInfo.m_iDocID ) )
			return 0;
		m_iStats.m_iTotalDocuments++;

		if ( !ScanInt ( "group", &m_tDocInfo.m_iGroupID ) )
			return 0;

		if ( !ScanInt ( "timestamp", &m_tDocInfo.m_iTimestamp, false ) )
			m_tDocInfo.m_iTimestamp = 1;

		if ( !ScanStr ( "title", sTitle, sizeof ( sTitle ) ) )
			return 0;

		// index title
		{
			int iLen = (int)strlen ( sTitle );
			int iPos = 1;
			BYTE * sWord;

			m_pTokenizer->SetBuffer ( (BYTE*)sTitle, iLen, true );
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				DWORD iWID = m_pDict->GetWordID ( sWord );
				if ( iWID )
				{
					CSphWordHit & tHit = m_dHits.Add ();
					tHit.m_iDocID = m_tDocInfo.m_iDocID;
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
	while ( m_dHits.GetLength()<1024 && !bBodyEnd ) // FIXME!
	{
		// suck in some data if needed
		if ( m_pBuffer>=m_pBufferEnd )
			if ( !UpdateBuffer() )
		{
			fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",
				m_tDocInfo.m_iDocID );
			return 0;
		}

		// check for body tag end in this buffer
		BYTE * p = m_pBuffer;
		while ( p<m_pBufferEnd && *p!='<' )
			p++;
		if ( p<m_pBufferEnd && *p=='<' )
			bBodyEnd = true;

		// set proper buffer part
		m_pTokenizer->SetBuffer ( m_pBuffer, p-m_pBuffer, bBodyEnd );
		m_pBuffer = p;

		// tokenize
		BYTE * sWord;
		while ( ( sWord = m_pTokenizer->GetToken () )!=NULL )
		{
			DWORD iWID = m_pDict->GetWordID ( sWord );
			if ( iWID )
			{
				CSphWordHit & tHit = m_dHits.Add ();
				tHit.m_iDocID = m_tDocInfo.m_iDocID;
				tHit.m_iWordID = iWID;
				tHit.m_iWordPos = (1<<24) | (++m_iWordPos); // field_id | (iPos++)
			}
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
	return m_tDocInfo.m_iDocID;
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


bool CSphSource_XMLPipe::CheckTag ( bool bOpen, bool bStrict )
{
	int iAdd = bOpen ? 2 : 3;

	// if case the tag is at buffer boundary, try to suck in some more data
	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
		UpdateBuffer ();

	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
	{
		if ( bStrict )
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

		if ( bStrict )
			fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got '%s'.\n",
				bOpen ? "" : "/", m_pTag, sGot );
		return false;
	}

	// got tag
	m_pBuffer += iAdd+m_iTagLength;
	assert ( m_pBuffer<=m_pBufferEnd );
	return true;
}


bool CSphSource_XMLPipe::SkipTag ( bool bOpen, bool bWarnOnEOF, bool bStrict )
{
	if ( !SkipWhitespace() )
	{
		if ( bWarnOnEOF )
			fprintf ( stderr, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
				bOpen ? "" : "/", m_pTag );
		return false;
	}

	return CheckTag ( bOpen, bStrict );
}


bool CSphSource_XMLPipe::ScanInt ( const char * sTag, DWORD * pRes, bool bStrict )
{
	assert ( sTag );
	assert ( pRes );

	// scan for <sTag>
	SetTag ( sTag );
	if ( !SkipTag ( true, true, bStrict ) )
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

/////////////////////////////////////////////////////////////////////////////

void sphSetQuiet ( bool bQuiet )
{
	g_bSphQuiet = bQuiet;
}

//
// $Id$
//
