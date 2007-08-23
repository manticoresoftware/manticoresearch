//
// $Id$
//

//
// Copyright (c) 2001-2007, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxstem.h"
#include "sphinxquery.h"

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

#if USE_LIBSTEMMER
#include "libstemmer.h"
#endif

#if USE_WINDOWS
	#include <io.h> // for open()

	// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
	#define snprintf	_snprintf
	#define sphSeek		_lseeki64
	#define strtoull	_strtoui64

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
void sphWarn ( char * sTemplate, ... );

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *	SPHINX_DEFAULT_SBCS_TABLE	= "0..9, A..Z->a..z, _, a..z, U+A8->U+B8, U+B8, U+C0..U+DF->U+E0..U+FF, U+E0..U+FF";
const char *	SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F";

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

/////////////////////////////////////////////////////////////////////////////
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

/// virtual attributes
enum
{
	SPH_VATTR_ID			= -1,	///< tells match sorter to use doc id
	SPH_VATTR_RELEVANCE		= -2,	///< tells match sorter to use match weight

	SPH_VATTR_GROUP			= 0,	///< @group offset after normal attrs
	SPH_VATTR_COUNT			= 1,	///< @count offset after normal attrs
	SPH_VATTR_DISTINCT		= 2,	///< @distinct offset after normal attrs

	SPH_VATTRTOTAL_GROUP	= 2,
	SPH_VATTRTOTAL_DISTINCT	= 3
};

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
		size_t uRead = (size_t) ::read ( GetFD(), pBuf, uCount );
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

/////////////////////////////////////////////////////////////////////////////

/// match-sorting priority queue traits
class CSphMatchQueueTraits : public ISphMatchSorter
{
protected:
	CSphMatch *					m_pData;
	int							m_iUsed;
	int							m_iSize;

	CSphMatchComparatorState	m_tState;
	bool						m_bUsesAttrs;

public:
	/// ctor
	CSphMatchQueueTraits ( int iSize, bool bUsesAttrs )
		: m_iUsed ( 0 )
		, m_iSize ( iSize )
		, m_bUsesAttrs ( bUsesAttrs )
	{
		assert ( iSize>0 );
		m_pData = new CSphMatch [ iSize ];
		assert ( m_pData );

		m_tState.m_iNow = (DWORD) time ( NULL );
	}

	/// dtor
	~CSphMatchQueueTraits ()
	{
		SafeDeleteArray ( m_pData );
	}

public:
	int			GetLength () const										{ return m_iUsed; }
	void		SetState ( const CSphMatchComparatorState & tState )	{ m_tState = tState; m_tState.m_iNow = (DWORD) time ( NULL ); }
	bool		UsesAttrs ()											{ return m_bUsesAttrs; }
	CSphMatch *	First ()												{ return m_pData; }
};


/// normal match-sorting priority queue
template < typename COMP > class CSphMatchQueue : public CSphMatchQueueTraits
{
public:
	/// ctor
	CSphMatchQueue ( int iSize, bool bUsesAttrs )
		: CSphMatchQueueTraits ( iSize, bUsesAttrs )
	{}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		if ( m_iUsed==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, m_pData[0], m_tState ) )
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
			if ( !COMP::IsLess ( m_pData[iEntry], m_pData[iParent], m_tState ) )
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
				if ( COMP::IsLess ( m_pData[iChild+1], m_pData[iChild], m_tState ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( COMP::IsLess ( m_pData[iChild], m_pData[iEntry], m_tState ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch * pTo, int iTag )
	{
		assert ( m_iUsed>=0 );
		pTo += m_iUsed;
		while ( m_iUsed>0 )
		{
			--pTo;
			pTo[0] = m_pData[0]; // OPTIMIZE? reset dst + swap?
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			Pop ();
		}
	}
};

//////////////////////////////////////////////////////////////////////////////

static inline int iLog2 ( uint64_t iValue )
{
	int iBits = 0;
	while ( iValue )
	{
		iValue >>= 1;
		iBits++;
	}
	return iBits;
}


/// simple fixed-size hash
/// doesn't keep the order
template < typename T, typename KEY, typename HASHFUNC >
class CSphFixedHash : ISphNoncopyable
{
protected:
	static const int			HASH_LIST_END	= -1;
	static const int			HASH_DELETED	= -2;

	struct HashEntry_t
	{
		KEY		m_tKey;
		T		m_tValue;
		int		m_iNext;
	};

protected:
	CSphVector<HashEntry_t>		m_dEntries;		///< key-value pairs storage pool
	CSphVector<int>				m_dHash;		///< hash into m_dEntries pool

	int							m_iFree;		///< free pairs count
	CSphVector<int>				m_dFree;		///< free pair indexes

public:
	/// ctor
	CSphFixedHash ( int iLength )
	{
		int iBuckets = 2<<iLog2(iLength-1); // less than 50% bucket usage guaranteed
		assert ( iLength>0 );
		assert ( iLength<=iBuckets );

		m_dEntries.Resize ( iLength );
		m_dHash.Resize ( iBuckets );
		m_dFree.Resize ( iLength );

		Reset ();
	}

	/// cleanup
	void Reset ()
	{
		ARRAY_FOREACH ( i, m_dEntries )
			m_dEntries[i].m_iNext = HASH_DELETED;

		ARRAY_FOREACH ( i, m_dHash )
			m_dHash[i] = HASH_LIST_END;

		m_iFree = m_dFree.GetLength();
		ARRAY_FOREACH ( i, m_dFree )
			m_dFree[i] = i;
	}

	/// add new entry
	/// returns NULL on success
	/// returns pointer to value if already hashed
	T * Add ( const T & tValue, const KEY & tKey )
	{
		assert ( m_iFree>0 && "hash overflow" );

		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = -1, iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iPrev=iEntry, iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return &m_dEntries[iEntry].m_tValue;
		assert ( iEntry!=HASH_DELETED );

		// if it's not, do add
		int iNew = m_dFree [ --m_iFree ];

		HashEntry_t & tNew = m_dEntries[iNew];
		assert ( tNew.m_iNext==HASH_DELETED );

		tNew.m_tKey = tKey;
		tNew.m_tValue = tValue;
		tNew.m_iNext = HASH_LIST_END;

		if ( iPrev>=0 )
		{
			assert ( m_dEntries[iPrev].m_iNext==HASH_LIST_END );
			m_dEntries[iPrev].m_iNext = iNew;
		} else
		{
			assert ( m_dHash[uHash]==HASH_LIST_END );
			m_dHash[uHash] = iNew;
		}
		return NULL;
	}

	/// remove entry from hash
	void Remove ( const KEY & tKey )
	{
		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = -1, iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iPrev=iEntry, iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
		{
			// found, remove it
			assert ( m_dEntries[iEntry].m_iNext!=HASH_DELETED );
			if ( iPrev>=0 )
				m_dEntries[iPrev].m_iNext = m_dEntries[iEntry].m_iNext;
			else
				m_dHash[uHash] = m_dEntries[iEntry].m_iNext;

#ifndef NDEBUG
			m_dEntries[iEntry].m_iNext = HASH_DELETED;
#endif

			m_dFree [ m_iFree++ ] = iEntry;
			return;
		}
		assert ( iEntry!=HASH_DELETED );
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return (T*)&m_dEntries[iEntry].m_tValue;

		assert ( iEntry!=HASH_DELETED );
		return NULL;
	}
};


struct IdentityHash_fn
{
	static inline uint64_t	Hash ( uint64_t iValue )	{ return iValue; }
	static inline DWORD		Hash ( DWORD iValue )		{ return iValue; }
	static inline int		Hash ( int iValue )			{ return iValue; }
};

/////////////////////////////////////////////////////////////////////////////

typedef uint64_t	SphGroupKey_t;

/// (group,attrvalue) pair
struct SphGroupedValue_t
{
public:
	SphGroupKey_t	m_uGroup;
	DWORD			m_uValue;

public:
	SphGroupedValue_t ()
	{}

	SphGroupedValue_t ( SphGroupKey_t uGroup, DWORD uValue )
		: m_uGroup ( uGroup )
		, m_uValue ( uValue )
	{}

	inline bool operator < ( const SphGroupedValue_t & rhs ) const
	{
		if ( m_uGroup<rhs.m_uGroup ) return true;
		if ( m_uGroup>rhs.m_uGroup ) return false;
		return m_uValue<rhs.m_uValue;
	}

	inline bool operator == ( const SphGroupedValue_t & rhs ) const
	{
		return m_uGroup==rhs.m_uGroup && m_uValue==rhs.m_uValue;
	}
};


/// unique values counter
/// used for COUNT(DISTINCT xxx) GROUP BY yyy queries
class CSphUniqounter : public CSphVector<SphGroupedValue_t,16384>
{
public:
#ifndef NDEBUG
					CSphUniqounter () : m_iCountPos ( 0 ), m_bSorted ( true ) {}
	void			Add ( const SphGroupedValue_t & tValue )	{ CSphVector<SphGroupedValue_t,16384>::Add ( tValue ); m_bSorted = false; }
	void			Sort ()										{ CSphVector<SphGroupedValue_t,16384>::Sort (); m_bSorted = true; }

#else
					CSphUniqounter () : m_iCountPos ( 0 ) {}
#endif

public:
	int				CountStart ( SphGroupKey_t * pOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t * pOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups );

protected:
	int				m_iCountPos;

#ifndef NDEBUG
	bool			m_bSorted;
#endif
};


int CSphUniqounter::CountStart ( SphGroupKey_t * pOutGroup )
{
	m_iCountPos = 0;
	return CountNext ( pOutGroup );
}


int CSphUniqounter::CountNext ( SphGroupKey_t * pOutGroup )
{
	assert ( m_bSorted );
	if ( m_iCountPos>=m_iLength )
		return 0;

	SphGroupKey_t uGroup = m_pData[m_iCountPos].m_uGroup;
	DWORD uValue = m_pData[m_iCountPos].m_uValue;
	*pOutGroup = uGroup;

	int iCount = 1;
	while ( m_pData[m_iCountPos].m_uGroup==uGroup )
	{
		if ( m_pData[m_iCountPos].m_uValue!=uValue )
			iCount++;
		uValue = m_pData[m_iCountPos].m_uValue;
		m_iCountPos++;
	}
	return iCount;
}


void CSphUniqounter::Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups )
{
	assert ( m_bSorted );
	if ( !m_iLength )
		return;

	sphSort ( pRemoveGroups, iRemoveGroups );

	SphGroupedValue_t * pSrc = m_pData;
	SphGroupedValue_t * pDst = m_pData;
	SphGroupedValue_t * pEnd = m_pData + m_iLength;

	// skip remove-groups which are not in my list
	while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
	{
		pRemoveGroups++;
		iRemoveGroups--;
	}

	for ( ; pSrc<pEnd; pSrc++ )
	{
		// check if this entry needs to be removed
		while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
		{
			pRemoveGroups++;
			iRemoveGroups--;
		}
		if ( iRemoveGroups && pSrc->m_uGroup==*pRemoveGroups )
			continue;

		// check if it's a dupe
		if ( pDst>m_pData && pDst[-1]==pSrc[0] )
			continue;

		*pDst++ = *pSrc;
	}

	assert ( pDst-m_pData<=m_iLength );
	m_iLength = pDst-m_pData;
}

/////////////////////////////////////////////////////////////////////////////

SphGroupKey_t sphCalcGroupKey ( const CSphMatch & tMatch, ESphGroupBy eGroupBy, int iAttrOffset, int iAttrBits )
{
	if ( eGroupBy==SPH_GROUPBY_ATTRPAIR )
	{
		int iItem = iAttrOffset/ROWITEM_BITS;
		return *(SphGroupKey_t*)( tMatch.m_pRowitems+iItem );
	}

	CSphRowitem iAttr = tMatch.GetAttr ( iAttrOffset, iAttrBits );
	if ( eGroupBy==SPH_GROUPBY_ATTR )
		return iAttr;

	time_t tStamp = iAttr;
	struct tm * pSplit = localtime ( &tStamp ); // FIXME! use _r on UNIX

	switch ( eGroupBy )
	{
		case SPH_GROUPBY_DAY:	return (pSplit->tm_year+1900)*10000 + (1+pSplit->tm_mon)*100 + pSplit->tm_mday;
		case SPH_GROUPBY_WEEK:	return (pSplit->tm_year+1900)*1000 + (1+pSplit->tm_yday) - pSplit->tm_wday;
		case SPH_GROUPBY_MONTH:	return (pSplit->tm_year+1900)*100 + (1+pSplit->tm_mon);
		case SPH_GROUPBY_YEAR:	return (pSplit->tm_year+1900);
		default:				assert ( 0 ); return 0;
	}
}


/// match sorter with k-buffering and group-by
#if USE_WINDOWS
#pragma warning(disable:4127)
#endif

template < typename COMPMATCH, typename COMPGROUP, bool DISTINCT >
class CSphKBufferGroupSorter : public CSphMatchQueueTraits, public ISphNoncopyable
{
protected:
	const int		m_iRowitems;		///< normal match rowitems count (to distinguish already grouped matches)

	ESphGroupBy		m_eGroupBy;			///< group-by function
	int				m_iGroupbyOffset;	///< group-by attr bit offset
	int				m_iGroupbyCount;	///< group-by attr bit count

	CSphFixedHash < CSphMatch *, SphGroupKey_t, IdentityHash_fn >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved

	CSphUniqounter	m_tUniq;
	int				m_iDistinctOffset;
	int				m_iDistinctCount;
	bool			m_bSortByDistinct;

	CSphMatchComparatorState	m_tStateGroup;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	CSphKBufferGroupSorter ( const CSphQuery * pQuery ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( pQuery->m_iMaxMatches*GROUPBY_FACTOR, true )
		, m_iRowitems		( pQuery->m_iRealRowitems )
		, m_eGroupBy		( pQuery->m_eGroupFunc )
		, m_iGroupbyOffset	( pQuery->m_iGroupbyOffset )
		, m_iGroupbyCount	( pQuery->m_iGroupbyCount )
		, m_hGroup2Match	( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, m_iLimit			( pQuery->m_iMaxMatches )
		, m_iDistinctOffset	( pQuery->m_iDistinctOffset )
		, m_iDistinctCount	( pQuery->m_iDistinctCount )
		, m_bSortByDistinct	( false )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || m_iDistinctOffset>=0 );
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		const int VATTR_TOTAL = DISTINCT ? SPH_VATTRTOTAL_DISTINCT : SPH_VATTRTOTAL_GROUP;
		assert ( tEntry.m_iRowitems==m_iRowitems || tEntry.m_iRowitems==m_iRowitems+VATTR_TOTAL );

		bool bGrouped = ( tEntry.m_iRowitems!=m_iRowitems );
		SphGroupKey_t uGroupKey = sphCalcGroupKey ( tEntry, m_eGroupBy, m_iGroupbyOffset, m_iGroupbyCount );

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( m_eGroupBy==SPH_GROUPBY_ATTRPAIR || pMatch->m_pRowitems [ m_iRowitems+SPH_VATTR_GROUP ]==uGroupKey );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				assert ( pMatch->m_iRowitems==tEntry.m_iRowitems );
				pMatch->m_pRowitems [ m_iRowitems+SPH_VATTR_COUNT ] += tEntry.m_pRowitems [ m_iRowitems+SPH_VATTR_COUNT ];
				if ( DISTINCT )
					pMatch->m_pRowitems [ m_iRowitems+SPH_VATTR_DISTINCT ] += tEntry.m_pRowitems [ m_iRowitems+SPH_VATTR_DISTINCT ];

			} else
			{
				// it's a simple match
				// increase grouped matches count
				assert ( pMatch->m_iRowitems==tEntry.m_iRowitems+VATTR_TOTAL );
				pMatch->m_pRowitems [ m_iRowitems+SPH_VATTR_COUNT ]++;
			}

			// if new entry is more relevant, update from it
			if ( COMPMATCH::IsLess ( *pMatch, tEntry, m_tState ) )
			{
				pMatch->m_iDocID = tEntry.m_iDocID;
				pMatch->m_iWeight = tEntry.m_iWeight;
				pMatch->m_iTag = tEntry.m_iTag;

				for ( int i=0; i<pMatch->m_iRowitems-VATTR_TOTAL; i++ )
					pMatch->m_pRowitems[i] = tEntry.m_pRowitems[i];
			}
		}

		// submit actual distinct value in all cases
		if ( DISTINCT && !bGrouped )
			m_tUniq.Add ( SphGroupedValue_t ( uGroupKey, tEntry.GetAttr ( m_iDistinctOffset, m_iDistinctCount ) ) );

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
			CutWorst ();

		// do add
		assert ( m_iUsed<m_iSize );
		CSphMatch & tNew = m_pData [ m_iUsed++ ];

		int iNewSize = tEntry.m_iRowitems + ( bGrouped ? 0 : VATTR_TOTAL );
		assert ( tNew.m_iRowitems==0 || tNew.m_iRowitems==iNewSize );

		tNew.m_iDocID = tEntry.m_iDocID;
		tNew.m_iWeight = tEntry.m_iWeight;
		tNew.m_iTag = tEntry.m_iTag;
		if ( !tNew.m_iRowitems )
		{
			tNew.m_iRowitems = iNewSize;
			tNew.m_pRowitems = new CSphRowitem [ iNewSize ];
		}
		memcpy ( tNew.m_pRowitems, tEntry.m_pRowitems, tEntry.m_iRowitems*sizeof(CSphRowitem) );
		if ( !bGrouped )
		{
			tNew.m_pRowitems [ m_iRowitems+SPH_VATTR_GROUP ] = (DWORD)uGroupKey; // intentionally truncate
			tNew.m_pRowitems [ m_iRowitems+SPH_VATTR_COUNT ] = 1;
			if ( DISTINCT )
				tNew.m_pRowitems [ m_iRowitems+SPH_VATTR_DISTINCT ] = 0;
		}

		m_hGroup2Match.Add ( &tNew, uGroupKey );
		return true;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch * pTo, int iTag )
	{
		CountDistinct ();
		SortGroups ();

		int iLen = GetLength ();
		for ( int i=0; i<iLen; i++, pTo++ )
		{
			pTo[0] = m_pData[i];
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
		}

		m_iUsed = 0;
	}

	/// get entries count
	int GetLength () const
	{
		return Min ( m_iUsed, m_iLimit );
	}

	/// set group comparator state
	void SetGroupState ( const CSphMatchComparatorState & tState )
	{
		m_tStateGroup = tState;

		if ( DISTINCT && m_iDistinctOffset>=0 )
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
				if ( m_tStateGroup.m_iBitOffset[i]==m_iDistinctOffset )
			{
				m_bSortByDistinct = true;
				break;
			}
	}

protected:
	/// count distinct values if necessary
	void CountDistinct ()
	{
		if ( DISTINCT )
		{
			m_tUniq.Sort ();
			SphGroupKey_t uGroup;
			for ( int iCount = m_tUniq.CountStart(&uGroup); iCount; iCount = m_tUniq.CountNext(&uGroup) )
			{
				CSphMatch ** ppMatch = m_hGroup2Match(uGroup);
				if ( ppMatch )
					(*ppMatch)->m_pRowitems[m_iRowitems+SPH_VATTR_DISTINCT] = iCount;
			}
		}
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ()
	{
		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();
		SortGroups ();

		// cut groups
		int iCut = m_iLimit * (int)(GROUPBY_FACTOR/2);
		m_iUsed -= iCut;

		// cleanup unused distinct stuff
		if ( DISTINCT )
		{
			// build kill-list
			CSphVector<SphGroupKey_t> dRemove;
			dRemove.Resize ( iCut );
			if ( m_eGroupBy==SPH_GROUPBY_ATTRPAIR )
			{
				for ( int i=0; i<iCut; i++ )
					dRemove[i] = sphCalcGroupKey ( m_pData[m_iUsed+i], m_eGroupBy, m_iGroupbyOffset, m_iGroupbyCount );
			} else
			{
				for ( int i=0; i<iCut; i++ )
					dRemove[i] = m_pData[m_iUsed+i].m_pRowitems[m_iRowitems+SPH_VATTR_GROUP];
			}

			// sort and compact
			if ( !m_bSortByDistinct )
				m_tUniq.Sort ();
			m_tUniq.Compact ( &dRemove[0], iCut );
		}

		// rehash
		m_hGroup2Match.Reset ();
		if ( m_eGroupBy==SPH_GROUPBY_ATTRPAIR )
		{
			for ( int i=0; i<m_iUsed; i++ )
			{
				SphGroupKey_t uKey = sphCalcGroupKey ( m_pData[i], m_eGroupBy, m_iGroupbyOffset, m_iGroupbyCount );
				m_hGroup2Match.Add ( &m_pData[i], uKey );
			}
		} else
		{
			for ( int i=0; i<m_iUsed; i++ )
				m_hGroup2Match.Add ( m_pData+i, m_pData[i].m_pRowitems[ m_iRowitems+SPH_VATTR_GROUP ] );
		}
	}

	/// sort groups buffer
	void SortGroups ()
	{
		CSphMatch * pData = m_pData;
		int iCount = m_iUsed;

		int st0[32], st1[32], a, b, k, i, j;
		CSphMatch x;

		k = 1;
		st0[0] = 0;
		st1[0] = iCount-1;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{

					while ( COMPGROUP::IsLess ( x, pData[i], m_tStateGroup ) ) i++;
					while ( COMPGROUP::IsLess ( pData[j], x, m_tStateGroup ) ) j--;
					if (i <= j) { Swap ( pData[i], pData[j] ); i++; j--; }
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
	}
};

#if USE_WINDOWS
#pragma warning(default:4127)
#endif

//////////////////////////////////////////////////////////////////////////////
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

	static int			CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase );
	void				Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize );

	SphWordID_t			ReadVLB ();		// FIXME? should actually be widest type of both 
	int					ReadByte ();
	int					ReadBytes ( void * pDest, int iBytes );
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

/// query word from the searcher's point of view
class CSphQueryWord
{
public:
	CSphString		m_sWord;		///< my copy of word
	int				m_iQueryPos;	///< word position, from query (WARNING! reused for index into TF array in extended mode)
	SphWordID_t		m_iWordID;		///< word ID, from dictionary

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

private:
#ifndef NDEBUG
	bool			m_bHitlistOver;
#endif

public:
	CSphQueryWord ()
		: m_sWord ( NULL )
		, m_iQueryPos ( -1 )
		, m_iWordID ( 0 )
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

	void SetupAttrs ( ESphDocinfo eDocinfo, const CSphDocInfo & m_tMin )
	{
		m_tDoc.Reset ( m_tMin.m_iRowitems );

		m_iMinID = m_tMin.m_iDocID;
		if ( eDocinfo==SPH_DOCINFO_INLINE )
		{
			m_iInlineAttrs = m_tMin.m_iRowitems;
			m_pInlineFixup = m_tMin.m_pRowitems;
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

struct CSphIndex_VLN;

struct CSphMergeSource
{	
	const BYTE *		m_pWordlist;
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
	CSphVector< DWORD >					m_dWordPos;
	CSphVector< CSphDoclistRecord >		m_dDoclist;
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

	void Read ();
	void Write ();
	bool IsEmpty() const
	{
		return m_tWordData.IsEmpty();
	}
};

struct CSphDocMVA
{
	SphDocID_t								m_iDocID;
	CSphVector < CSphVector<DWORD,16>, 16 > m_dMVA;
	CSphVector < DWORD,16 >					m_dOffsets;

	CSphDocMVA( int iSize )
		: m_iDocID ( 0 )
	{
		m_dMVA.Resize( iSize );
		m_dOffsets.Resize( iSize );
	}

	void	Read( CSphReader_VLN & tReader );
	void	Write( CSphWriter & tWriter );
};

/////////////////////////////////////////////////////////////////////////////

/// everything required to setup search term
struct CSphTermSetup : ISphNoncopyable
{
	ISphTokenizer *			m_pTokenizer;
	CSphDict *				m_pDict;
	CSphIndex_VLN *			m_pIndex;
	ESphDocinfo				m_eDocinfo;
	CSphDocInfo				m_tMin;
	const CSphAutofile &	m_tDoclist;
	const CSphAutofile &	m_tHitlist;

	CSphTermSetup ( const CSphAutofile & tDoclist, const CSphAutofile & tHitlist )
		: m_pTokenizer ( NULL )
		, m_pDict ( NULL )
		, m_pIndex ( NULL )
		, m_tDoclist ( tDoclist )
		, m_tHitlist ( tHitlist )
	{}
};


/// this is my actual VLN-compressed phrase index implementation
struct CSphIndex_VLN : CSphIndex
{
	friend struct CSphDoclistRecord;

								CSphIndex_VLN ( const char * sFilename );

	virtual int					Build ( CSphDict * pDict, const CSphVector < CSphSource * > & dSources, int iMemoryLimit, ESphDocinfo eDocinfo );

	virtual const CSphSchema *	Prealloc ( bool bMlock, CSphString * sWarning );
	virtual bool				Mlock ();
	virtual void				Dealloc ();

	virtual bool				Preread ();
	template<typename T> bool	PrereadSharedBuffer ( CSphSharedBuffer<T> & pBuffer, const char * sExt );

	virtual void				SetBase ( const char * sNewBase );
	virtual bool				Rename ( const char * sNewBase );

	virtual bool				Lock ();
	virtual void				Unlock ();

	virtual CSphQueryResult *	Query ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery );
	virtual bool				QueryEx ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchSorter * pTop );

	virtual bool				Merge( CSphIndex * pSource, CSphPurgeData & tPurgeData );
	void						MergeWordData ( CSphWordRecord & tDstWord, CSphWordRecord & tSrcWord );

	virtual int					UpdateAttributes ( const CSphAttrUpdate_t & tUpd );
	virtual bool				SaveAttributes ();

private:
	static const int			WORDLIST_CHECKPOINT		= 1024;		///< wordlist checkpoints frequency
	static const int			WRITE_BUFFER_SIZE		= 262144;	///< my write buffer size

	static const DWORD			INDEX_MAGIC_HEADER		= 0x58485053;	///< my magic 'SPHX' header
	static const DWORD			INDEX_FORMAT_VERSION	= 6;			///< my format version

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
	CSphVector<CSphQueryWord,8>			m_dQueryWords;			///< search query words for "simple" query types (ie. match all/any/phrase)

	int							m_iWeights;						///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights count

	CSphSharedBuffer<DWORD>		m_pDocinfo;				///< my docinfo cache
	DWORD						m_uDocinfo;				///< my docinfo cache size
	CSphSharedBuffer<DWORD>		m_pMva;					///< my multi-valued attrs cache

	static const int			DOCINFO_HASH_BITS = 18;	// FIXME! make this configurable
	CSphSharedBuffer<DWORD>		m_pDocinfoHash;

	CSphSharedBuffer<BYTE>		m_pWordlist;			///< my wordlist cache
	CSphWordlistCheckpoint *	m_pWordlistCheckpoints;	///< my wordlist cache checkpoints
	int							m_iWordlistCheckpoints;	///< my wordlist cache checkpoints count

	bool						m_bPreallocated;		///< are we ready to preread
	CSphSharedBuffer<BYTE>		m_bPreread;				///< are we ready to search
	DWORD						m_uVersion;				///< data files version

private:
	const char *				GetIndexFileName ( const char * sExt ) const;	///< WARNING, non-reenterable, static buffer!
	int							AdjustMemoryLimit ( int iMemoryLimit );

	int							cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits, DWORD * pDocinfo, int Docinfos, int iStride );
	void						cidxHit ( CSphWordHit * pHit, CSphRowitem * pDocinfos );
	bool						cidxDone ();

	void						WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol );
	void						ReadSchemaColumn ( CSphReader_VLN & rdInfo, CSphColumnInfo & tCol );

	void						MatchAll ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult );
	void						MatchAny ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult );
	bool						MatchBoolean ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult, const CSphTermSetup & tTermSetup );
	bool						MatchExtended ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult, const CSphTermSetup & tTermSetup );

	const DWORD *				FindDocinfo ( SphDocID_t uDocID );
	void						LookupDocinfo ( CSphDocInfo & tMatch );

	bool						BuildMVA ( const CSphVector<CSphSource*> & dSources, CSphAutoArray<CSphWordHit> & dHits, int iArenaSize );

public:
	// FIXME! this needs to be protected, and refactored as well
	bool						SetupQueryWord ( CSphQueryWord & tWord, const CSphTermSetup & tTermSetup );
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

void sphDie ( char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	fprintf ( stdout, "FATAL: " );
	vfprintf ( stdout, sTemplate, ap );
	fprintf ( stdout, "\n" );
	va_end ( ap );
	exit ( 1 );
}


void sphWarn ( char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	fprintf ( stdout, "WARNING: " );
	vfprintf ( stdout, sTemplate, ap );
	fprintf ( stdout, "\n" );
	va_end ( ap );
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

/// parser to build lowercaser from textual config
class CSphCharsetDefinitionParser
{
public:
						CSphCharsetDefinitionParser ();
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


/// single-byte charset tokenizer
class CSphTokenizer_SBCS : public ISphTokenizer
{
public:
						CSphTokenizer_SBCS ();
	virtual				~CSphTokenizer_SBCS ();

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );
	virtual BYTE *				GetToken ();
	virtual ISphTokenizer *		Clone () const;
	virtual bool				IsUtf8 () const { return false; }
	virtual int					GetCodepointLength ( int ) const { return 1; }

protected:
	BYTE *				m_pBuffer;							///< my buffer
	BYTE *				m_pBufferMax;						///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;								///< current position
	BYTE				m_sAccum [ 4+SPH_MAX_WORD_LEN ];	///< token accumulator
	int					m_iAccum;							///< token size
	bool				m_bLast;							///< is this buffer the last one
};


/// UTF-8 tokenizer
class CSphTokenizer_UTF8 : public ISphTokenizer
{
public:
						CSphTokenizer_UTF8 ();
	virtual				~CSphTokenizer_UTF8 ();

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );
	virtual BYTE *				GetToken ();
	virtual BYTE *				GetTokenSyn ();
	virtual ISphTokenizer *		Clone () const;
	virtual bool				IsUtf8 () const { return true; }
	virtual int					GetCodepointLength ( int iCode ) const;

protected:
	int					GetFoldedCodePoint ();
	void				AccumCodePoint ( int iCode );
	void				FlushAccum ();

	void				SynReset ();

protected:
	BYTE *				m_pBuffer;							///< my buffer
	BYTE *				m_pBufferMax;						///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;								///< current position
	bool				m_bLast;							///< is this buffer the last one

	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< boundary token accumulator
	BYTE *				m_pAccum;							///< current accumulator position
	int					m_iAccum;							///< boundary token size

protected:
	int					m_iSynFlushing;						///< are we flushing folded tokens

	BYTE				m_sSyn [ MAX_SYNONYM_LEN+4 ];		///< candidate buffer
	int					m_iSynBytes;						///< candidate buffer length, bytes
	int					m_iSynCodes;						///< candidate buffer length, codepoints

	BYTE				m_sFolded [ MAX_SYNONYM_LEN+4 ];	///< folded tokens bufer
	int					m_iFolded;							///< folded tokens buffer length, bytes
	int					m_iFoldedLastTok;					///< last folded token start offset, bytes
	int					m_iFoldedLastCodes;					///< last folded token length, codepoints
	int					m_iFoldedLast;						///< last folded codepoint
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


void CSphLowercaser::AddRemaps ( const CSphRemapRange * pRemaps, int iRemaps, DWORD uFlags, DWORD uFlagsIfExists )
{
	// build new chunks map
	// 0 means "was unused"
	// 1 means "was used"
	// 2 means "is used now"
	int dUsed [ CHUNK_COUNT ];
	for ( int i=0; i<CHUNK_COUNT; i++ )
		dUsed[i] = m_pChunk[i] ? 1 : 0;

	int iNewChunks = m_iChunks;

	for ( int i=0; i<iRemaps; i++ )
	{
		const CSphRemapRange * pRemap = pRemaps+i;

		#define LOC_CHECK_RANGE(_a) assert ( (_a)>=0 && (_a)<MAX_CODE );
		LOC_CHECK_RANGE ( pRemap->m_iStart );
		LOC_CHECK_RANGE ( pRemap->m_iEnd );
		LOC_CHECK_RANGE ( pRemap->m_iRemapStart );
		LOC_CHECK_RANGE ( pRemap->m_iRemapStart + pRemap->m_iEnd - pRemap->m_iStart );
		#undef LOC_CHECK_RANGE

		for ( int iChunk = pRemap->m_iStart>>CHUNK_BITS; iChunk <= pRemap->m_iEnd>>CHUNK_BITS; iChunk++ )
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
	for ( int i=0; i<iRemaps; i++ )
	{
		const CSphRemapRange * pRemap = pRemaps+i;

		DWORD iRemapped = pRemap->m_iRemapStart;
		for ( int j = pRemap->m_iStart; j <= pRemap->m_iEnd; j++, iRemapped++ )
		{
			assert ( m_pChunk [ j>>CHUNK_BITS ] );
			int & iCodepoint = m_pChunk [ j>>CHUNK_BITS ] [ j & CHUNK_MASK ];
			iCodepoint =  ( iCodepoint ? uFlagsIfExists : uFlags ) | iRemapped;
		}
	}
}


enum
{
	MASK_CODEPOINT			= 0x0ffffffUL,	// mask off codepoint flags
	FLAG_CODEPOINT_SPECIAL	= 0x1000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL		= 0x2000000UL,	// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM	= 0x4000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_SYNONYM	= 0x8000000UL	// this codepoint is used in synonym tokens only
};


void CSphLowercaser::AddSpecials ( const char * sSpecials )
{
	assert ( sSpecials );
	int iSpecials = strlen(sSpecials);

	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Resize ( iSpecials );
	ARRAY_FOREACH ( i, dRemaps )
		dRemaps[i].m_iStart = dRemaps[i].m_iEnd = dRemaps[i].m_iRemapStart = sSpecials[i];

	AddRemaps ( &dRemaps[0], iSpecials,
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


static int sphUTF8Len ( const char * pStr  )
{
	BYTE * pBuf = (BYTE*) pStr;
	int iRes = 0, iCode;

	while ( ( iCode=sphUTF8Decode(pBuf) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}

/////////////////////////////////////////////////////////////////////////////

bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	if ( m_dSynonyms.GetLength() )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}

	CSphCharsetDefinitionParser tParser;
	CSphVector<CSphRemapRange> dRemaps;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	m_tLC.Reset ();
	m_tLC.AddRemaps ( &dRemaps[0], dRemaps.GetLength(), 0, 0 );
	return true;
}


void ISphTokenizer::AddCaseFolding ( CSphRemapRange & tRange )
{
	m_tLC.AddRemaps ( &tRange, 1, 0, 0 );
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	m_tLC.AddSpecials ( sSpecials );
}


static int TokenizeOnWhitespace ( CSphVector<CSphString,8> & dTokens, BYTE * sFrom, bool bUtf8 )
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
				pAccum += sphUTF8Encode ( pAccum, iCode );
				iAccum++;
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


bool ISphTokenizer::LoadSynonyms ( const char * sFilename, CSphString & sError )
{
	m_dSynonyms.Reset ();

	FILE * fp = fopen ( sFilename, "r" );
	if ( !fp )
	{
		sError.SetSprintf ( "failed to open '%s'", sFilename );
		return false;
	}

	int iLine = 0;
	char sBuffer[1024];

	CSphOrderedHash < int, int, IdentityHash_fn, 4096, 117 > hSynonymOnly;
	CSphVector<CSphString,8> dFrom;

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
			if ( sphUTF8Len ( dFrom[i].cstr() )>SPH_MAX_WORD_LEN )
		{
			sError.SetSprintf ( "%s line %d: map-from token too long (over %d bytes)", sFilename, iLine, SPH_MAX_WORD_LEN );
			break;
		}
		if ( sphUTF8Len((const char*)sTo)>SPH_MAX_WORD_LEN )
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
		tSyn.m_iToLen = strlen((char*)sTo);

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
		return false;

	// sort the list
	m_dSynonyms.Sort ();

	// add synonym-only remaps
	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Grow ( hSynonymOnly.GetLength() );

	hSynonymOnly.IterateStart ();
	while ( hSynonymOnly.IterateNext() )
	{
		CSphRemapRange & tRange = dRemaps.Add ();
		tRange.m_iStart = tRange.m_iEnd = tRange.m_iRemapStart = hSynonymOnly.IterateGetKey();
	}

	m_tLC.AddRemaps ( &dRemaps[0], dRemaps.GetLength(), FLAG_CODEPOINT_SYNONYM, 0 );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_SBCS::CSphTokenizer_SBCS ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
	, m_bLast		( false )
{
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_SBCS_TABLE, sTmp );
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
}


BYTE * CSphTokenizer_SBCS::GetToken ()
{
	for ( ;; )
	{
		// get next codepoint
		int iCode = 0;
		if ( m_pCur>=m_pBufferMax )
		{
			if ( !m_bLast || m_iAccum<m_iMinWordLen )
			{
				if ( m_bLast ) // if this is the last buffer, flush accumulator contents
					m_iAccum = 0;

				m_iLastTokenLen = 0;
				return NULL;
			}
		} else
		{
			iCode = m_tLC.ToLower ( *m_pCur++ );
		}

		// handle whitespace
		if ( iCode==0 )
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
	return pClone;
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_UTF8::CSphTokenizer_UTF8 ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_bLast		( false )
	, m_iAccum		( 0 )

	, m_iSynFlushing		( -1 )
	, m_iSynBytes			( 0 )
	, m_iSynCodes			( 0 )

	, m_iFolded				( 0 )
	, m_iFoldedLastTok		( 0 )
	, m_iFoldedLastCodes	( 0 )
	, m_iFoldedLast			( 0 )
{
	m_pAccum = m_sAccum;
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE, sTmp );
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
	if ( m_dSynonyms.GetLength() )
		return GetTokenSyn ();

	for ( ;; )
	{
		// get next codepoint
		BYTE * pCur = m_pCur; // to redo special char, if there's a token already
		int iCode = GetFoldedCodePoint (); // advances m_pCur

		// handle eof
		if ( iCode<0 )
		{
			// if it was not the last buffer, just keep the accumulator
			if ( !m_bLast )
			{
				m_iLastTokenLen = 0;
				return NULL;
			}

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

		// handle whitespace
		if ( iCode==0 )
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
				AccumCodePoint ( iCode ); // handle special as a standalone token
			} else
			{
				m_pCur = pCur; // we need to flush current accum and then redo special char again
			}

			FlushAccum ();
			return m_sAccum;
		}

		// just accumulate
		AccumCodePoint ( iCode );
	}
}


enum SynCheck_e
{
	SYNCHECK_LESS,
	SYNCHECK_PARTIAL,
	SYNCHECK_EXACT,
	SYNCHECK_GREATER
};


static inline SynCheck_e SynCheckPrefix ( const CSphSynonym & tCandidate, const BYTE * sPrefix, int iPrefixLen )
{
	const BYTE * sCand = (const BYTE*) tCandidate.m_sFrom.cstr();
	const BYTE * sCur = sPrefix;
	const BYTE * sMax = sPrefix + iPrefixLen;

	while ( sCur<sMax )
	{
		if ( *sCand < *sCur ) return SYNCHECK_LESS;
		if ( *sCand > *sCur ) return SYNCHECK_GREATER;
		sCand++;
		sCur++;
	}
	return *sCand ? SYNCHECK_PARTIAL : SYNCHECK_EXACT;
}


void CSphTokenizer_UTF8::SynReset ()
{
	m_iSynBytes = 0;
	m_iSynCodes = 0;

	m_iFolded = 0;
	m_iFoldedLastCodes = 0;
	m_iFoldedLastTok = 0;
	m_iFoldedLast = 0;
}


BYTE * CSphTokenizer_UTF8::GetTokenSyn ()
{
	assert ( m_dSynonyms.GetLength() );

	for ( ;; )
	{
		////////////////////////////////////////////////////////////
		// accumulate incoming raw stream and check it for synonyms
		////////////////////////////////////////////////////////////

		bool bBreakOnFolded = false;
		for ( ; m_iSynFlushing<0; )
		{
			assert ( m_iSynBytes<(int)sizeof(m_sSyn) );
			assert ( m_iFolded<(int)sizeof(m_sFolded) );

			// get raw and folded codepoints
			int iCode = -1;
			while ( m_pCur<m_pBufferMax && iCode<0 )
				iCode = sphUTF8Decode ( m_pCur );

			// handle eof
			int iFolded = 0;
			if ( iCode<0 )
			{
				if ( !m_bLast || m_iSynBytes==0 )
				{
					// if it's not the last one, or if the acc is empty, just bail out
					m_iLastTokenLen = 0;
					return NULL;
				}
			} else
			{
				iFolded = m_tLC.ToLower ( iCode );
			}

			// handle raw non-whitespace
			if ( iFolded>0 )
			{
				// check for specials
				bool bRawSpecial =
					( iFolded & FLAG_CODEPOINT_SPECIAL ) &&
					!( ( iFolded & FLAG_CODEPOINT_DUAL ) && m_iSynBytes && m_sSyn[m_iSynBytes-1]!=MAGIC_SYNONYM_WHITESPACE );

				bool bFoldedSpecial =
					( iFolded & FLAG_CODEPOINT_SPECIAL ) &&
					!( ( iFolded & FLAG_CODEPOINT_DUAL ) && m_iFoldedLast );

				// fixup folded code
				if ( iFolded & FLAG_CODEPOINT_SYNONYM )
					iFolded = 0;
				iFolded &= MASK_CODEPOINT;

				// skip short folded tokens
				if ( !iFolded || bFoldedSpecial )
				{
					if ( m_iFoldedLastCodes<m_iMinWordLen )
						m_iFolded = m_iFoldedLastTok;
				}

				if ( bRawSpecial )
				{
					// it's special for raw token (and therefore for folded too); detach it and flush
					if ( m_iFoldedLast ) m_sFolded[m_iFolded++] = 0;
					m_iFolded += sphUTF8Encode ( m_sFolded+m_iFolded, iFolded & MASK_CODEPOINT );
					m_sFolded[m_iFolded++] = 0;
					m_iFoldedLast = 0;
					m_iSynFlushing = 0;

				} else if ( bFoldedSpecial )
				{
					// it's either non-special or dual for raw token; but special for folded
					if ( m_iSynCodes<SPH_MAX_WORD_LEN )
					{
						m_iSynBytes += sphUTF8Encode ( m_sSyn+m_iSynBytes, iCode );
						m_iSynCodes++;
					} else
					{
						bBreakOnFolded = true;
					}

					if ( m_iFoldedLast ) m_sFolded[m_iFolded++] = 0;
					m_iFolded += sphUTF8Encode ( m_sFolded+m_iFolded, iFolded );
					m_sFolded[m_iFolded++] = 0;
					m_iFoldedLast = 0;

				} else
				{
					// it's non-special or dual for both raw and folded tokens
					if ( m_iSynCodes<SPH_MAX_WORD_LEN )
					{
						m_iSynBytes += sphUTF8Encode ( m_sSyn+m_iSynBytes, iCode );
						m_iSynCodes++;
					} else
					{
						bBreakOnFolded = true;
					}

					if ( !iFolded )
					{
						if ( m_iFoldedLast )
							m_sFolded[m_iFolded++] = 0;
					} else if ( m_iFoldedLastCodes<SPH_MAX_WORD_LEN )
					{
						m_iFolded += sphUTF8Encode ( m_sFolded+m_iFolded, iFolded );
					}
					m_iFoldedLast = iFolded;
				}

				if ( !m_iFoldedLast )
				{
					m_iFoldedLastTok = m_iFolded;
					m_iFoldedLastCodes = 0;
					if ( bBreakOnFolded )
					{
						m_iSynFlushing = 0;
						break;
					}
				} else
				{
					m_iFoldedLastCodes++;
				}
				continue;
			}
			assert ( iFolded==0 );

			// handle raw whitespace
			if ( iCode>=0 && ( !m_iSynBytes || m_sSyn[m_iSynBytes-1]==MAGIC_SYNONYM_WHITESPACE ) ) // ignore heading and trailing whitespace
				continue;

			// skip short folded tokens
			if ( m_iFoldedLastCodes<m_iMinWordLen )
				m_iFolded = m_iFoldedLastTok;

			m_sSyn[m_iSynBytes++] = MAGIC_SYNONYM_WHITESPACE;
			m_sFolded[m_iFolded++] = 0;

			m_iFoldedLastTok = m_iFolded;
			m_iFoldedLastCodes = 0;

			// check current prefix against synonyms list
			// FIXME! optimize this
			bool bPartial = false;
			ARRAY_FOREACH ( i, m_dSynonyms )
			{
				SynCheck_e eCheck = SynCheckPrefix ( m_dSynonyms[i], m_sSyn, m_iSynBytes );
				if ( eCheck==SYNCHECK_EXACT )
				{
					SynReset ();

					m_iLastTokenLen = m_dSynonyms[i].m_iToLen;
					strcpy ( (char*)m_sAccum, m_dSynonyms[i].m_sTo.cstr() );

					return m_sAccum;
				}
				if ( eCheck==SYNCHECK_PARTIAL )
					bPartial = true;
			}

			// if there's no partial matches or if it's eof, let's flush
			if ( !bPartial || iCode<0 )
				m_iSynFlushing = 0;
		}

		///////////////////////////////////////////////////////////////
		// flush everything accumulated during potential synonym check
		///////////////////////////////////////////////////////////////

		// skip folded whitespace
		assert ( m_iSynFlushing>=0 );
		int iPos = m_iSynFlushing;

		while ( iPos<m_iFolded && !m_sFolded[iPos] )
			iPos++;

		// check if we're over
		if ( iPos>=m_iFolded )
		{
			SynReset ();
			m_iSynFlushing = -1;
			continue;
		}

		// got something; memorize it and skip until whitespace
		int iRes = iPos;
		while ( iPos<m_iFolded && m_sFolded[iPos] )
			iPos++;
		m_iLastTokenLen = iPos - iRes;
		m_iSynFlushing = iPos;

		return m_sFolded + iRes;
	}
}


int CSphTokenizer_UTF8::GetFoldedCodePoint ()
{
	while ( m_pCur<m_pBufferMax )
	{
		// decode next one (returns -1 on error)
		int iCode = sphUTF8Decode ( m_pCur );

		// on succesful decode, fold and return
		if ( iCode>=0 )
			return m_tLC.ToLower ( iCode );
	}
	return -1; // eof
}


void CSphTokenizer_UTF8::AccumCodePoint ( int iCode )
{
	assert ( iCode>0 );
	assert ( m_iAccum>=0 );

	// throw away everything which is over the token size
	if ( m_iAccum>=SPH_MAX_WORD_LEN )
		return;

	m_pAccum += sphUTF8Encode ( m_pAccum, iCode );

	assert ( m_pAccum>=m_sAccum && m_pAccum<m_sAccum+sizeof(m_sAccum) );
	m_iAccum++;
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
	CSphCharsetDefinitionParser tParser;
	CSphVector<CSphRemapRange> dRemaps;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	m_tLC.AddRemaps ( &dRemaps[0], dRemaps.GetLength(),
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
	, m_uMinValue	( 0 )
	, m_uMaxValue	( UINT_MAX )
	, m_iValues		( 0 )
	, m_pValues		( NULL )
	, m_bExclude	( false )
	, m_bMva		( false )
	, m_iAttr		( -1 )
	, m_iBitOffset	( -1 )
	, m_iBitCount	( -1 )
{}


CSphFilter::CSphFilter ( const CSphFilter &  rhs )
{
	assert ( 0 );
	m_pValues = NULL;
	(*this) = rhs;
}


CSphFilter::~CSphFilter ()
{
	SafeDeleteArray ( m_pValues );
}


const CSphFilter & CSphFilter::operator = ( const CSphFilter & rhs )
{
	m_sAttrName		= rhs.m_sAttrName;
	m_uMinValue		= rhs.m_uMinValue;
	m_uMaxValue		= rhs.m_uMaxValue;
	m_iValues		= rhs.m_iValues;
	SafeDeleteArray ( m_pValues );
	if ( m_iValues )
	{
		m_pValues = new DWORD [ m_iValues ];
		memcpy ( m_pValues, rhs.m_pValues, sizeof(DWORD)*m_iValues );
	}
	m_bExclude		= rhs.m_bExclude;
	m_bMva			= rhs.m_bMva;
	m_iBitOffset	= rhs.m_iBitOffset;
	m_iBitCount		= rhs.m_iBitCount;
	return *this;
}


static int cmpDword ( const void * a, const void * b )
{
	DWORD aa = *((DWORD*)a);
	DWORD bb = *((DWORD*)b);
	if ( aa==bb )
		return 0;
	return ( aa<bb ) ? -1 : 1;
}


void CSphFilter::SortValues ()
{
	if ( m_pValues )
		qsort ( m_pValues, m_iValues, sizeof(DWORD), cmpDword );
}

/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////

CSphQuery::CSphQuery ()
	: m_iOffset		( 0 )
	, m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_eMode		( SPH_MATCH_ALL )
	, m_eSort		( SPH_SORT_RELEVANCE )
	, m_iMaxMatches	( 1000 )
	, m_iMinID		( 0 )
	, m_iMaxID		( DOCID_MAX )
	, m_sGroupSortBy	( "@group desc" )
	, m_sGroupDistinct	( "" )
	, m_iCutoff		( 0 )
	, m_iRetryCount	( 0 )
	, m_iRetryDelay	( 0 )

	, m_iRealRowitems	( -1 )
	, m_iGroupbyOffset	( -1 )
	, m_iGroupbyCount	( -1 )
	, m_iDistinctOffset	( -1 )
	, m_iDistinctCount	( -1 )
{}


CSphQuery::~CSphQuery ()
{
	SafeDeleteArray ( m_pWeights );
}


bool CSphQuery::SetSchema ( const CSphSchema & tSchema, CSphString & sError )
{
	m_iRealRowitems = tSchema.GetRealRowSize();
	m_iGroupbyOffset = -1;
	m_iDistinctOffset = -1;

	if ( m_sGroupBy.IsEmpty() )
		return true;

	// setup gropuby attr
	int iGroupBy = tSchema.GetAttrIndex ( m_sGroupBy.cstr() );
	if ( iGroupBy<0 )
	{
		sError.SetSprintf ( "group-by attribute '%s' not found", m_sGroupBy.cstr() );
		return false;
	}
	if ( m_eGroupFunc==SPH_GROUPBY_ATTRPAIR && iGroupBy+1>=tSchema.GetAttrsCount() )
	{
		sError.SetSprintf ( "group-by attribute '%s' must not be last in ATTRPAIR grouping mode", m_sGroupBy.cstr() );
		return false;
	}
	m_iGroupbyOffset = tSchema.GetAttr(iGroupBy).m_iBitOffset;
	m_iGroupbyCount = tSchema.GetAttr(iGroupBy).m_iBitCount;

	// setup distinct attr
	if ( !m_sGroupDistinct.IsEmpty() )
	{
		int iDistinct = tSchema.GetAttrIndex ( m_sGroupDistinct.cstr() );
		if ( iDistinct<0 )
		{
			sError.SetSprintf ( "group-count-distinct attribute '%s' not found", m_sGroupDistinct.cstr() );
			return false;
		}

		m_iDistinctOffset = tSchema.GetAttr(iDistinct).m_iBitOffset;
		m_iDistinctCount = tSchema.GetAttr(iDistinct).m_iBitCount;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// SCHEMA
/////////////////////////////////////////////////////////////////////////////

int CSphSchema::GetRealAttrsCount () const
{
	// FIXME! add some bool instead of magic attr naming?
	int iGroupby = GetAttrIndex ( "@groupby" );
	return ( iGroupby>=0 ) ? iGroupby : GetAttrsCount();
}


int CSphSchema::GetRealRowSize () const
{
	// FIXME! add some bool instead of magic attr naming?
	int iGroupby = GetAttrIndex ( "@groupby" );
	return ( iGroupby>=0 ) ? GetAttr(iGroupby).m_iRowitem : GetRowSize();
}


ESphSchemaCompare CSphSchema::CompareTo ( const CSphSchema & rhs, CSphString & sError ) const
{
	char sTmp [ 1024 ];
	sError = "";

	/////////////////////////
	// incompatibility tests
	/////////////////////////

	// check attrs count
	int iRealAttrs = GetRealAttrsCount();
	if ( iRealAttrs!=rhs.GetRealAttrsCount() )
	{
		snprintf ( sTmp, sizeof(sTmp), "non-virtual attributes count mismatch: %d in schema '%s', %d in schema '%s'",
			m_dAttrs.GetLength(), m_sName.cstr(),
			rhs.m_dAttrs.GetLength(), rhs.m_sName.cstr() );
		sError = sTmp;
		return SPH_SCHEMAS_INCOMPATIBLE;
	}

	// check attr types
	for ( int i=0; i<iRealAttrs; i++ )
		if ( rhs.m_dAttrs[i].m_eAttrType!=m_dAttrs[i].m_eAttrType )
	{
		snprintf ( sTmp, sizeof(sTmp), "attribute %d/%d type mismatch: name='%s', type=%d in schema '%s'; name='%s', type=%d in schema '%s'",
			i, iRealAttrs,
			m_dAttrs[i].m_sName.cstr(), m_dAttrs[i].m_eAttrType, m_sName.cstr(),
			rhs.m_dAttrs[i].m_sName.cstr(), rhs.m_dAttrs[i].m_eAttrType, rhs.m_sName.cstr() );
		sError = sTmp;
		return SPH_SCHEMAS_INCOMPATIBLE;
	}

	//////////////////
	// equality tests
	//////////////////

	// check fulltext fields count
	if ( rhs.m_dFields.GetLength()!=m_dFields.GetLength() )
	{
		snprintf ( sTmp, sizeof(sTmp), "fulltext fields count mismatch: %d in schema '%s', %d in schema '%s'",
			m_dFields.GetLength(), m_sName.cstr(),
			rhs.m_dFields.GetLength(), rhs.m_sName.cstr() );
		sError = sTmp;
		return SPH_SCHEMAS_COMPATIBLE;
	}

	// check fulltext field names
	ARRAY_FOREACH ( i, rhs.m_dFields )
		if ( rhs.m_dFields[i].m_sName!=m_dFields[i].m_sName )
	{
		snprintf ( sTmp, sizeof(sTmp), "fulltext field %d/%d name mismatch: name='%s' in schema '%s'; name='%s' in schema '%s'",
			i, m_dFields.GetLength(),
			m_dFields[i].m_sName.cstr(), m_sName.cstr(),
			rhs.m_dFields[i].m_sName.cstr(), rhs.m_sName.cstr() );
		sError = sTmp;
		return SPH_SCHEMAS_COMPATIBLE;
	}

	// check attr names
	for ( int i=0; i<iRealAttrs; i++ )
		if ( rhs.m_dAttrs[i].m_sName!=m_dAttrs[i].m_sName )
	{
		snprintf ( sTmp, sizeof(sTmp), "attribute %d/%d name mismatch: name='%s' in schema '%s', name='%s' in schema '%s'",
			i, iRealAttrs,
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
	if ( m_iPoolUsed+iSize>SPH_CACHE_WRITE )
		Flush ();
	assert ( m_iPoolUsed+iSize<=SPH_CACHE_WRITE );

	memcpy ( m_pPool, pData, iSize );
	m_pPool += iSize;
	m_iPoolUsed += iSize;
	m_iPos += iSize;
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

	if ( !sphWrite ( m_iFD, m_pBuffer, m_iPoolUsed, m_sName.cstr(), *m_pError ) )
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
	SphOffset_t iNewPos = m_iPos + m_iBuffUsed;
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
	m_iBuffUsed = ::read ( m_iFD, m_pBuff, iReadLen );
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


int CSphBin::CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase )
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

	if ( iBinSize<CSphBin::WARN_SIZE )
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

			if ( ::read ( m_iFile, m_dBuffer, n )!=n )
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


int CSphBin::ReadBytes ( void * pDest, int iBytes )
{
	assert ( iBytes>0 );
	assert ( iBytes<=m_iSize );

	if ( m_iDone )
		return 0;

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
			return 0;
		}

		assert ( m_dBuffer );
		memmove ( m_dBuffer, m_pCurrent, m_iLeft );

		if ( ::read ( m_iFile, m_dBuffer + m_iLeft, n )!=n )
			return -2;

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

	return 1;
}


#if USE_64BIT
#define READVLB_FAILED U64C(0xffffffffffffffff)
#else
#define READVLB_FAILED 0xffffffffUL
#endif


SphWordID_t CSphBin::ReadVLB ()
{
	SphWordID_t v = 0;
	int o = 0, t;
	do
	{
		if ( ( t = ReadByte() )<0 )
			return READVLB_FAILED; // FIXME! replace with some other errcode
		v += ( (SphWordID_t(t & 0x7f)) << o );
		o += 7;
	} while ( t & 0x80 );
	return v;
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
		// unexpected EOB
		SphWordID_t r = ReadVLB ();
		if ( r==READVLB_FAILED )
			return 0;

		if ( r )
		{
			switch ( m_eState )
			{
				case BIN_WORD:
					tHit.m_iWordID += r;
					tHit.m_iDocID = 0;
					tHit.m_iWordPos = 0;
					m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					m_eState = BIN_POS;
					tHit.m_iDocID += r;
					tHit.m_iWordPos = 0;

					for ( int i=0; i<iRowitems; i++, pRowitems++ )
					{
						SphWordID_t uTmp = ReadVLB (); // FIXME? check range?
						*pRowitems = (DWORD)uTmp;
						if ( uTmp==READVLB_FAILED )
							return 0; // read unexpected EOB
					}
					break;

				case BIN_POS: 
					tHit.m_iWordPos += (DWORD)r; // FIXME? check range?
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
	, m_iMinInfixLen ( 0 )
	, m_bPrefixesOnly ( false )
	, m_bAttrsUpdated ( false )
{
}


void CSphIndex::SetInfixIndexing ( bool bPrefixesOnly, int iMinLength )
{
	m_iMinInfixLen = Max ( iMinLength, 0 );
	m_bPrefixesOnly = bPrefixesOnly;
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

	m_uDocinfo = 0;
	m_eDocinfo = SPH_DOCINFO_NONE;

	m_bPreallocated = false;
	m_uVersion = INDEX_FORMAT_VERSION;
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
	CSphVector<int,8> dAttrIndex;
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
	size_t uWrote = ::write ( fdTmpnew.GetFD(), m_pDocinfo.GetWritePtr(), uSize );

	if ( uWrote!=uSize )
	{
		m_sLastError.SetSprintf ( "write error: %s", strerror(errno) );
		return false;
	}
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
	fdInfo.PutByte ( m_bPrefixesOnly );
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


inline int encodeVLB8(BYTE *buf, SphOffset_t v)
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
		int iBits = iLog2 ( DOCINFO2ID ( pDocinfo + (iDocinfos-1)*iStride ) - iStartID  );
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
			if ( !sphWrite ( fd, m_pWriteBuffer, w, "raw_hits", m_sLastError ) )
				return -1;
			n += w;
			pBuf = m_pWriteBuffer;
		}
	}
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	w = (int)(pBuf - m_pWriteBuffer);
	if ( !sphWrite ( fd, m_pWriteBuffer, w, "raw_hits", m_sLastError ) )
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


bool CSphIndex_VLN::BuildMVA ( const CSphVector<CSphSource*> & dSources, CSphAutoArray<CSphWordHit> & dHits, int iArenaSize )
{
	// initialize writer (data file must always exist)
	CSphWriter wrMva;
	if ( !wrMva.OpenFile ( GetIndexFileName("spm"), m_sLastError ) )
		return false;

	// calcs and checks
	CSphVector<int,32> dMvaIndexes;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).m_eAttrType & SPH_ATTR_MULTI )
			dMvaIndexes.Add ( i );

	if ( dMvaIndexes.GetLength()<=0 )
		return false;

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

	m_tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT_MVA;
	m_tProgress.m_iAttrs = 0;

	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		if ( !pSource->Connect ( m_sLastError ) )
			return 0;

		ARRAY_FOREACH ( i, dMvaIndexes )
		{
			int iAttr = dMvaIndexes[i];

			int iRowitem = m_tSchema.GetAttr(iAttr).m_iRowitem;
			assert ( iRowitem>=0 );

			if ( !pSource->IterateMultivaluedStart ( iAttr, m_sLastError ) )
				return 0;

			while ( pSource->IterateMultivaluedNext () )
			{
				pMva->m_uDocID = pSource->m_tDocInfo.m_iDocID;
				pMva->m_iAttr = i;
				pMva->m_uValue = pSource->m_tDocInfo.m_pRowitems[iRowitem];

				if ( ++pMva>=pMvaMax )
				{
					sphSort ( pMvaPool, pMva-pMvaPool );
					if ( !sphWrite ( fdTmpMva.GetFD(), pMvaPool, (pMva-pMvaPool)*sizeof(MvaEntry_t), "temp_mva", m_sLastError ) )
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
		if ( !sphWrite ( fdTmpMva.GetFD(), pMvaPool, (pMva-pMvaPool)*sizeof(MvaEntry_t), "temp_mva", m_sLastError ) )
			return false;

		dBlockLens.Add ( pMva-pMvaPool );
		m_tProgress.m_iAttrs += pMva-pMvaPool;
		pMva = pMvaPool;
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
		m_tProgress.m_iAttrsTotal = m_tProgress.m_iAttrs;
		m_pProgress ( &m_tProgress, false );
	}

	// initialize readers
	CSphVector<CSphBin *> dBins;
	dBins.Grow ( dBlockLens.GetLength() );

	int iBinSize = CSphBin::CalcBinSize ( iArenaSize, dBlockLens.GetLength(), "sort_mva" );
	SphOffset_t iSharedOffset = -1;

	ARRAY_FOREACH ( i, dBlockLens )
	{
		dBins.Add ( new CSphBin() );
		dBins[i]->m_iFileLeft = dBlockLens[i]*sizeof(MvaEntry_t);
		dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		dBins[i]->Init ( fdTmpMva.GetFD(), &iSharedOffset, iBinSize );
	}

	// do the sort
	CSphQueue < MvaEntryTag_t, MvaEntryCmp_fn > qMva ( Max ( 1, dBlockLens.GetLength() ) );
	ARRAY_FOREACH ( i, dBlockLens )
	{
		MvaEntryTag_t tEntry;
		if ( dBins[i]->ReadBytes ( (MvaEntry_t*) &tEntry, sizeof(MvaEntry_t) )<=0 )
		{
			m_sLastError.SetSprintf ( "sort_mva: warmup failed (io error?)" );
			return 0;
		}

		tEntry.m_iTag = i;
		qMva.Push ( tEntry );
	}

	// spm-file := info-list [ 0+ ]
	// info-list := docid, values-list [ index.schema.mva-count ]
	// values-list := values-count, value [ values-count ]
	SphDocID_t uCurID = 0;
	CSphVector < CSphVector<DWORD,16>, 16 > dCurInfo;
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
		int iRes = dBins[iBin]->ReadBytes ( (MvaEntry_t*)&tEntry, sizeof(MvaEntry_t) );
		tEntry.m_iTag = iBin;
		if ( iRes>0 )
			qMva.Push ( tEntry );

		if ( iRes<0 )
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


/// in-memory ordinals accumulation and sorting
struct CSphOrdinal
{
	SphDocID_t	m_uDocID;	///< doc id
	DWORD		m_uValue;	///< ordinal value
	CSphString	m_sValue;	///< string value
};

struct CmpOrdinalsValue_fn
{
	inline int operator () ( const CSphOrdinal & a, const CSphOrdinal & b )
	{
		return strcmp ( a.m_sValue.cstr(), b.m_sValue.cstr() )==-1;
	}
};

struct CmpOrdinalsDocid_fn
{
	inline int operator () ( const CSphOrdinal & a, const CSphOrdinal & b )
	{
		return a.m_uDocID < b.m_uDocID;
	}
};


int CSphIndex_VLN::Build ( CSphDict * pDict, const CSphVector < CSphSource * > & dSources, int iMemoryLimit, ESphDocinfo eDocinfo )
{
	PROFILER_INIT ();

	assert ( pDict );
	assert ( dSources.GetLength() );

	if ( !m_pWriteBuffer )
		m_pWriteBuffer = new BYTE [ WRITE_BUFFER_SIZE ];

	m_eDocinfo = eDocinfo;

	// vars shared between phases
	CSphVector<CSphBin *> dBins;
	SphOffset_t iSharedOffset = -1;

	// setup sources
	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		pSource->SetDict ( pDict );
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

	CSphVector<int,32> dMvaIndexes;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).m_eAttrType & SPH_ATTR_MULTI )
			dMvaIndexes.Add ( i );

	bool bGotMVA = ( dMvaIndexes.GetLength()!=0 );
	if ( bGotMVA && m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		m_sLastError.SetSprintf ( "multi-valued attributes are only supported if but docinfo is 'extern' (fix your config file)" );
		return 0;
	}

	////////////////////////////////////////////////
	// collect and partially sort hits and docinfos
	////////////////////////////////////////////////

	// adjust memory requirements
	// ensure there's enough memory to hold 1M hits and 64K docinfos
	// alloc 1/16 of memory (but not less than 64K entries) for docinfos
	iMemoryLimit = AdjustMemoryLimit ( iMemoryLimit );

	const int iDocinfoStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	int iDocinfoMax = Max ( 65536, iMemoryLimit/16/iDocinfoStride/sizeof(DWORD) );
	if ( m_eDocinfo==SPH_DOCINFO_NONE )
		iDocinfoMax = 1;

	int iHitsMax = ( iMemoryLimit - iDocinfoMax*iDocinfoStride*sizeof(DWORD) ) / sizeof(CSphWordHit);

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

	// ordinals storage
	CSphVector<int,SPH_MAX_FIELDS> dOrdinalAttrs;
	if ( m_eDocinfo==SPH_DOCINFO_EXTERN )
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
			if ( m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_ORDINAL )
				dOrdinalAttrs.Add ( i );

	CSphVector < CSphVector<CSphOrdinal> > dOrdinals;
	dOrdinals.Resize ( dOrdinalAttrs.GetLength() );

	// create temp files
	CSphAutofile fdLock ( GetIndexFileName("tmp0"), SPH_O_NEW, m_sLastError );
	CSphAutofile fdTmpHits ( GetIndexFileName("tmp1"), SPH_O_NEW, m_sLastError );
	CSphAutofile fdTmpDocinfos ( GetIndexFileName("tmp2"), SPH_O_NEW, m_sLastError );

	if ( fdLock.GetFD()<0 || fdTmpHits.GetFD()<0 || fdTmpDocinfos.GetFD()<0 )
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

		// setup infix indexing
		pSource->SetEmitInfixes ( m_bPrefixesOnly, m_iMinInfixLen );

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

			// store ordinals
			ARRAY_FOREACH ( i, dOrdinalAttrs )
			{
				CSphVector<CSphOrdinal> & dCol = dOrdinals[i];
				dCol.Resize ( 1+dCol.GetLength() );

				dCol.Last().m_uDocID = pSource->m_tDocInfo.m_iDocID;
				dCol.Last().m_uValue = 0;
				Swap ( dCol.Last().m_sValue, pSource->m_dStrAttrs[dOrdinalAttrs[i]] );
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
					if ( !sphWrite ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
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
		if ( !sphWrite ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos", m_sLastError ) )
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

	BuildMVA ( dSources, dHits, iHitsMax*sizeof(CSphWordHit) );

	// reset hits pool
	dHits.Reset ();

	/////////////////
	// sort docinfos
	/////////////////

	// sort ordinals
	ARRAY_FOREACH ( i, dOrdinalAttrs )
	{
		DWORD uVal = 0;
		dOrdinals[i].Sort ( CmpOrdinalsValue_fn() );
		ARRAY_FOREACH ( j, dOrdinals[i] )
			dOrdinals[i][j].m_uValue = ++uVal;
		dOrdinals[i].Sort ( CmpOrdinalsDocid_fn() );
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
		dBins.Grow ( iDocinfoBlocks );

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
			if ( dBins[i]->ReadBytes  ( pDocinfo, iDocinfoStride*sizeof(DWORD) )<=0 )
			{
				m_sLastError.SetSprintf ( "sort_docinfos: warmup failed (io error?)" );
				return 0;
			}
			pDocinfo += iDocinfoStride;
			qDocinfo.Push ( i );
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
				assert ( dOrdinals[i][iOrd].m_uDocID == DOCINFO2ID(pEntry) );
				DOCINFO2ATTRS(pEntry) [ dOrdinalAttrs[i] ] = dOrdinals[i][iOrd].m_uValue;
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
						DOCINFO2ATTRS(pEntry) [ dMvaIndexes[i] ] = (DWORD)( rdMva.GetPos()/sizeof(DWORD) ); // intentional clamp; we'll check for 32bit overflow later
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
				if ( !sphWrite ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
					return 0;
				pDocinfo = dDocinfos;
			}

			// pop its index, update it, push its index again
			qDocinfo.Pop ();
			int iRes = dBins[iBin]->ReadBytes ( pEntry, iDocinfoStride*sizeof(DWORD) );
			if ( iRes<0 )
			{
				m_sLastError.SetSprintf ( "sort_docinfo: failed to read entry" );
				return 0;
			}
			if ( iRes>0 )
				qDocinfo.Push ( iBin );
		}
		if ( pDocinfo>dDocinfos )
		{
			assert ( 0 == ( pDocinfo-dDocinfos ) % iDocinfoStride );
			int iLen = ( pDocinfo - dDocinfos )*sizeof(DWORD);
			if ( !sphWrite ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo", m_sLastError ) )
				return 0;
		}

		// clean up readers
		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );
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
	dBins.Grow ( dHitBlocks.GetLength() );

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
	{
		assert ( m_tMin.m_pRowitems[i]>0 ); // FIXME? is this necessary now?
		m_tMin.m_pRowitems[i]--;
	}

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

	const CSphSchema * pSrcSchema = pSrcIndex->Prealloc(  false, NULL );
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
	CSphVector < DWORD, 16 >  dRowItemOffset;
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

			if ( ( iDstDocID && iDstDocID < iSrcDocID ) || iDstDocID )
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

				sphWrite ( fdSpa.GetFD(), pDstRow, sizeof(DWORD)*iStride, "doc_attr", m_sLastError );
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

				sphWrite( fdSpa.GetFD(), pSrcRow, sizeof( DWORD ) * iStride, "doc_attr", m_sLastError );
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

	const BYTE * pDstWordlist = &m_pWordlist[1];
	const BYTE * pSrcWordlist = &pSrcIndex->m_pWordlist[1];

	CSphVector<CSphWordlistCheckpoint>	dWordlistCheckpoints;
	int									iWordListEntries = 0;

	CSphMergeSource		tDstSource;
	CSphMergeSource		tSrcSource;

	tDstSource.m_pWordlist = pDstWordlist;
	tDstSource.m_pDoclistReader = &rdDstData;	
	tDstSource.m_pHitlistReader = &rdDstHitlist;
	tDstSource.m_iRowitems = ( m_eDocinfo == SPH_DOCINFO_INLINE )? m_tSchema.GetRowSize() : 0;
	tDstSource.m_iLastDocID = m_tMin.m_iDocID;
	tDstSource.m_iMinDocID = m_tMin.m_iDocID;
	tDstSource.m_pMinAttrs = m_tMin.m_pRowitems;
	tDstSource.m_pIndex = this;
	tDstSource.m_iMaxWordlistEntries = WORDLIST_CHECKPOINT;
	
	tSrcSource.m_pWordlist = pSrcWordlist;
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
		tDstWord.Read();
	else
		uProgress &= ~0x01;

	if ( !bSrcEmpty )
		tSrcWord.Read();
	else
		uProgress &= ~0x02;

	DWORD iDstPos = DWORD ( (BYTE*)m_pWordlistCheckpoints - pDstWordlist );
	DWORD iSrcPos = DWORD ( (BYTE*)pSrcIndex->m_pWordlistCheckpoints - pSrcWordlist );

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
			
			if ( iDstPos > 2 )
				tDstWord.Read();
			else
				uProgress &= ~0x01;
		}
		else if ( ( ( uProgress & 0x02 ) && tDstWord.m_tWordIndex > tSrcWord.m_tWordIndex ) || !( uProgress & 0x01 ) )
		{
			if ( !tSrcWord.IsEmpty() )
			{
				tSrcWord.Write();
				m_tProgress.m_iWords++;
			}

			if ( iSrcPos > 2 )
				tSrcWord.Read();		
			else
				uProgress &= ~0x02;
		}
		else
		{
			assert ( uProgress & 0x03 );

			MergeWordData ( tDstWord, tSrcWord );
			
			tDstWord.Write();
			m_tProgress.m_iWords++;

			if ( iDstPos > 2 )
				tDstWord.Read();
			else
				uProgress &= ~0x01;

			if ( iSrcPos > 2 )
				tSrcWord.Read();		
			else
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

		iDstPos = DWORD ( (BYTE*)m_pWordlistCheckpoints - tDstSource.m_pWordlist );
		iSrcPos = DWORD ( (BYTE*)pSrcIndex->m_pWordlistCheckpoints - tSrcSource.m_pWordlist );

		if ( iDstPos == 0 )
			uProgress &= ~0x01;

		if ( iSrcPos == 0 )
			uProgress &= ~0x02;

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

	fdInfo.PutByte ( m_bPrefixesOnly );
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

	static CSphVector<DWORD>				dWordPos;
	static CSphVector<CSphDoclistRecord>	dDoclist;

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
				if ( ! ( uCompleted & 0x01 ) )
				{
					iDstPos = pDstData->m_dWordPos[iDstWordPos++];
					if ( !iDstPos )
						uCompleted |= 0x01;
				}

				if ( ! ( uCompleted & 0x02 ) )
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

	void		LoadStopwords ( const char * sStops, ISphTokenizer * pTok );
	bool		SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError );

	SphWordID_t	GetWordID ( BYTE * pWord );
	SphWordID_t	GetWordID ( const BYTE * pWord, int iLen );

protected:
	CSphDict *	m_pDict;
};


void CSphDictStar::LoadStopwords ( const char * sStops, ISphTokenizer * pTok )
{
	m_pDict->LoadStopwords ( sStops, pTok );
}


bool CSphDictStar::SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError )
{
	return m_pDict->SetMorphology ( sMorph, bUseUTF8, sError );
}


SphWordID_t CSphDictStar::GetWordID ( const BYTE * pWord, int iLen )
{
	return m_pDict->GetWordID ( pWord, iLen );
}


SphWordID_t CSphDictStar::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

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
		pTokenizer->SetBuffer ( (BYTE*)sQbuf.cstr(), strlen(sQuery), true );

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
	return a.m_iDocs < b.m_iDocs;
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


inline bool sphGroupMatch ( DWORD iGroup, DWORD * pGroups, int iGroups )
{
	if ( !pGroups ) return true;
	DWORD * pA = pGroups;
	DWORD * pB = pGroups+iGroups-1;
	if ( iGroup==*pA || iGroup==*pB ) return true;
	if ( iGroup<(*pA) || iGroup>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		DWORD * pM = pA + ((pB-pA)/2);
		if ( iGroup==(*pM) )
			return true;
		if ( iGroup<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

/// match sorter
template < bool BITS >
struct MatchRelevanceLt_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & )
	{
		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
template < bool BITS >
struct MatchAttrLt_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		CSphRowitem aa = t.GetAttr<BITS>(a,0);
		CSphRowitem bb = t.GetAttr<BITS>(b,0);
		if ( aa!=bb  )
			return aa<bb;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
template < bool BITS >
struct MatchAttrGt_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		CSphRowitem aa = t.GetAttr<BITS>(a,0);
		CSphRowitem bb = t.GetAttr<BITS>(b,0);
		if ( aa!=bb  )
			return aa>bb;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
template < bool BITS >
struct MatchTimeSegments_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		CSphRowitem aa = t.GetAttr<BITS>(a,0);
		CSphRowitem bb = t.GetAttr<BITS>(b,0);
		int iA = GetSegment ( aa, t.m_iNow );
		int iB = GetSegment ( bb, t.m_iNow );
		if ( iA!=iB )
			return iA > iB;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		if ( aa!=bb )
			return aa<bb;

		return a.m_iDocID > b.m_iDocID;
	};

protected:
	static inline int GetSegment ( DWORD iStamp, DWORD iNow )
	{
		if ( iStamp>=iNow-3600 ) return 0; // last hour
		if ( iStamp>=iNow-24*3600 ) return 1; // last day
		if ( iStamp>=iNow-7*24*3600 ) return 2; // last week
		if ( iStamp>=iNow-30*24*3600 ) return 3; // last month
		if ( iStamp>=iNow-90*24*3600 ) return 4; // last 3 months
		return 5; // everything else
	}
};

/////////////////////////////////////////////////////////////////////////////

template < bool BITS >
static inline SphDocID_t MatchGetVattr ( const CSphMatch & m, const CSphMatchComparatorState & t, int iIndex )
{
	if ( t.m_iAttr[iIndex]==SPH_VATTR_ID )
		return m.m_iDocID;

	if ( t.m_iAttr[iIndex]==SPH_VATTR_RELEVANCE )
		return m.m_iWeight;

	return t.GetAttr<BITS> ( m, iIndex );
}


#define SPH_TEST_KEYPART(_idx) \
	aa = MatchGetVattr<BITS> ( a, t, _idx ); \
	bb = MatchGetVattr<BITS> ( b, t, _idx ); \
	if ( aa!=bb ) \
		return ( (t.m_uAttrDesc>>(_idx))&1 ) ^ ( aa>bb );


#define SPH_TEST_LASTKEYPART(_idx) \
	aa = MatchGetVattr<BITS> ( a, t, _idx ); \
	bb = MatchGetVattr<BITS> ( b, t, _idx ); \
	if ( aa==bb ) \
		return false; \
	return ((t.m_uAttrDesc>>(_idx))&1) ^ ( aa>bb );


template < bool BITS >
struct MatchGeneric2_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphDocID_t aa, bb;
		SPH_TEST_KEYPART(0);
		SPH_TEST_LASTKEYPART(1);
	};
};


template < bool BITS >
struct MatchGeneric3_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphDocID_t aa, bb;
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_LASTKEYPART(2);
	};
};


template < bool BITS >
struct MatchGeneric4_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphDocID_t aa, bb;
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_LASTKEYPART(3);
	};
};


template < bool BITS >
struct MatchGeneric5_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphDocID_t aa, bb;
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		SPH_TEST_LASTKEYPART(4);
	};
};

//////////////////////////////////////////////////////////////////////////

template < bool BITS >
struct MatchCustom_fn
{
	// setup sorting state
	static bool SetupAttr ( const CSphSchema & tSchema, CSphMatchComparatorState & tState, CSphString & sError, int iIdx, const char * sAttr )
	{
		if ( iIdx>=CSphMatchComparatorState::MAX_ATTRS )
		{
			sError.SetSprintf ( "custom sort: too many attributes declared" );
			return false;
		}

		int iAttr = tSchema.GetAttrIndex(sAttr);
		if ( iAttr<0 )
		{
			sError.SetSprintf ( "custom sort: attr '%s' not found in schema", sAttr );
			return false;
		}

		const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);
		tState.m_iAttr[iIdx] = iAttr;
		tState.m_iBitOffset[iIdx] = tAttr.m_iBitOffset;
		tState.m_iBitCount[iIdx] = tAttr.m_iBitCount;
		tState.m_iRowitem[iIdx] = tAttr.m_iRowitem;
		return true;
	}

	// setup sorting state
	static bool Setup ( const CSphSchema & tSchema, CSphMatchComparatorState & tState, CSphString & sError )
	{
		float fTmp;
		int iAttr = 0;

#define MATCH_FUNCTION				fTmp
#define MATCH_WEIGHT				1.0f
#define MATCH_NOW					1.0f
#define MATCH_ATTR(_idx)			1.0f
#define MATCH_DECLARE_ATTR(_name)	if ( !SetupAttr ( tSchema, tState, sError, iAttr++, _name ) ) return false;
#include "sphinxcustomsort.inl"
;

		return true;
	}

	// calc function and compare matches
	// OPTIMIZE? could calc once per match on submit
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
#undef MATCH_DECLARE_ATTR
#undef MATCH_WEIGHT
#undef MATCH_NOW
#undef MATCH_ATTR
#define MATCH_DECLARE_ATTR(_name)	;
#define MATCH_WEIGHT				float(MATCH_VAR.m_iWeight)
#define MATCH_NOW					float(t.m_iNow)
#define MATCH_ATTR(_idx)			float(t.GetAttr<BITS>(MATCH_VAR,_idx))

		float aa, bb;

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION aa
#define MATCH_VAR a
#include "sphinxcustomsort.inl"
;

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION bb
#define MATCH_VAR b
#include "sphinxcustomsort.inl"
;

		return aa<bb;
	}
};

//////////////////////////////////////////////////////////////////////////

static const int MAX_SORT_FIELDS = 5; // MUST be in sync with CSphMatchComparatorState::m_iAttr


enum ESphSortFunc
{
	FUNC_REL_DESC,
	FUNC_ATTR_DESC,
	FUNC_ATTR_ASC,
	FUNC_TIMESEGS,
	FUNC_GENERIC2,
	FUNC_GENERIC3,
	FUNC_GENERIC4,
	FUNC_GENERIC5,
	FUNC_CUSTOM
};


enum ESortClauseParseResult
{
	SORT_CLAUSE_OK,
	SORT_CLAUSE_ERROR,
	SORT_CLAUSE_RANDOM
};


static ESortClauseParseResult sphParseSortClause ( const char * sClause, const CSphQuery * pQuery,
	const CSphSchema & tSchema, bool bGroupClause,
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, CSphString & sError )
{
	// mini parser
	CSphString sTmp;
	CSphTokenizer_SBCS tTokenizer;
	tTokenizer.SetCaseFolding ( "0..9, A..Z->a..z, _, a..z, @", sTmp );

	CSphString sSortClause ( sClause );
	tTokenizer.SetBuffer ( (BYTE*)sSortClause.cstr(), strlen(sSortClause.cstr()), true );

	bool bField = false; // whether i'm expecting field name or sort order
	int iField = 0;

	for ( const char * pTok=(char*)tTokenizer.GetToken(); pTok; pTok=(char*)tTokenizer.GetToken() )
	{
		bField = !bField;

		// special case, sort by random
		if ( iField==0 && bField && strcmp ( pTok, "@random" )==0 )
			return SORT_CLAUSE_RANDOM;

		// special case, sort by custom
		if ( iField==0 && bField && strcmp ( pTok, "@custom" )==0 )
		{
			eFunc = FUNC_CUSTOM;
			return MatchCustom_fn<false>::Setup ( tSchema, tState, sError ) ? SORT_CLAUSE_OK : SORT_CLAUSE_ERROR;
		}

		// handle sort order
		if ( !bField )
		{
			// check
			if ( strcmp ( pTok, "desc" ) && strcmp ( pTok, "asc" ) )
			{
				sError.SetSprintf ( "invalid sorting order '%s'", pTok );
				return SORT_CLAUSE_ERROR;
			}

			// set
			if ( !strcmp ( pTok, "desc" ) )
				tState.m_uAttrDesc |= ( 1<<iField );

			iField++;
			continue;
		}

		// handle field name
		if ( iField==MAX_SORT_FIELDS )
		{
			sError.SetSprintf ( "too much sort-by fields; maximum count is %d", MAX_SORT_FIELDS );
			return SORT_CLAUSE_ERROR;
		}

		if ( !strcasecmp ( pTok, "@relevance" )
			|| !strcasecmp ( pTok, "@rank" )
			|| !strcasecmp ( pTok, "@weight" ) )
		{
			tState.m_iAttr[iField] = SPH_VATTR_RELEVANCE;

		} else if ( !strcasecmp ( pTok, "@id" ) )
		{
			tState.m_iAttr[iField] = SPH_VATTR_ID;

		} else if ( !strcasecmp ( pTok, "@count" ) && bGroupClause )
		{
			if ( pQuery->m_iGroupbyOffset<0 )
			{
				sError.SetSprintf ( "no group-by attribute; can not sort by @count" );
				return SORT_CLAUSE_ERROR;
			}

			tState.m_iAttr[iField] = tSchema.GetRealAttrsCount() + SPH_VATTR_COUNT;
			tState.m_iRowitem[iField] = tSchema.GetRealRowSize() + SPH_VATTR_COUNT;
			tState.m_iBitOffset[iField] = ( tSchema.GetRealRowSize()+SPH_VATTR_COUNT )*ROWITEM_BITS;
			tState.m_iBitCount[iField] = ROWITEM_BITS;

		} else if ( !strcasecmp ( pTok, "@group" ) && bGroupClause )
		{
			if ( pQuery->m_iGroupbyOffset<0 )
			{
				sError.SetSprintf ( "no group-by attribute; can not sort by @group" );
				return SORT_CLAUSE_ERROR;
			}

			tState.m_iAttr[iField] = tSchema.GetRealAttrsCount() + SPH_VATTR_GROUP;
			tState.m_iRowitem[iField] = tSchema.GetRealRowSize() + SPH_VATTR_GROUP;
			tState.m_iBitOffset[iField] = ( tSchema.GetRealRowSize()+SPH_VATTR_GROUP )*ROWITEM_BITS;
			tState.m_iBitCount[iField] = ROWITEM_BITS;

		} else if ( !strcasecmp ( pTok, "@distinct" ) && bGroupClause )
		{
			if ( pQuery->m_iDistinctOffset<0 )
			{
				sError.SetSprintf ( "no count-distinct attribute; can not sort by @distinct" );
				return SORT_CLAUSE_ERROR;
			}

			tState.m_iAttr[iField] = tSchema.GetRealAttrsCount() + SPH_VATTR_DISTINCT;
			tState.m_iRowitem[iField] = tSchema.GetRealRowSize() + SPH_VATTR_DISTINCT;
			tState.m_iBitOffset[iField] = ( tSchema.GetRealRowSize()+SPH_VATTR_DISTINCT )*ROWITEM_BITS;
			tState.m_iBitCount[iField] = ROWITEM_BITS;

		} else
		{
			int iAttr = tSchema.GetAttrIndex ( pTok );
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}

			tState.m_iAttr[iField] = iAttr;
			tState.m_iRowitem[iField] = tSchema.GetAttr(iAttr).m_iRowitem;
			tState.m_iBitOffset[iField] = tSchema.GetAttr(iAttr).m_iBitOffset;
			tState.m_iBitCount[iField] = tSchema.GetAttr(iAttr).m_iBitCount;
		}
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	if ( iField==1 )
		tState.m_iAttr[iField++] = SPH_VATTR_ID; // add "id ASC"

	switch ( iField )
	{
		case 2:		eFunc = FUNC_GENERIC2; break;
		case 3:		eFunc = FUNC_GENERIC3; break;
		case 4:		eFunc = FUNC_GENERIC4; break;
		case 5:		eFunc = FUNC_GENERIC5; break;
		default:	sError.SetSprintf ( "INTERNAL ERROR: %d fields in sphParseSortClause()", iField ); return SORT_CLAUSE_ERROR;
	}
	return SORT_CLAUSE_OK;
}

/////////////////////////////////////////////////////////////////////////////

template < typename COMPMATCH, typename COMPGROUP, typename ARG >
ISphMatchSorter * sphCreateSorter3rd  ( bool bDistinct, ARG arg )
{
	if ( bDistinct==true )
		return new CSphKBufferGroupSorter<COMPMATCH,COMPGROUP,true> ( arg );
	else
		return new CSphKBufferGroupSorter<COMPMATCH,COMPGROUP,false> ( arg );
}


template < typename COMPMATCH, typename ARG >
ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, bool bGroupBits, bool bDistinct, ARG arg )
{
	if ( bGroupBits )
	{
		switch ( eGroupFunc )
		{
			case FUNC_GENERIC2:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric2_fn<true> >	( bDistinct, arg ); break;
			case FUNC_GENERIC3:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric3_fn<true> >	( bDistinct, arg ); break;
			case FUNC_GENERIC4:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric4_fn<true> >	( bDistinct, arg ); break;
			case FUNC_GENERIC5:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric5_fn<true> >	( bDistinct, arg ); break;
			default:				return NULL;
		}
	} else
	{
		switch ( eGroupFunc )
		{
			case FUNC_GENERIC2:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric2_fn<false> >	( bDistinct, arg ); break;
			case FUNC_GENERIC3:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric3_fn<false> >	( bDistinct, arg ); break;
			case FUNC_GENERIC4:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric4_fn<false> >	( bDistinct, arg ); break;
			case FUNC_GENERIC5:		return sphCreateSorter3rd<COMPMATCH,MatchGeneric5_fn<false> >	( bDistinct, arg ); break;
			default:				return NULL;
		}
	}
}


template < typename ARG >
ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, bool bMatchBits, ESphSortFunc eGroupFunc, bool bGroupBits, bool bDistinct, ARG arg )
{
	if ( bMatchBits )
	{
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		return sphCreateSorter2nd<MatchRelevanceLt_fn<true> >	( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_ATTR_DESC:	return sphCreateSorter2nd<MatchAttrLt_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_ATTR_ASC:		return sphCreateSorter2nd<MatchAttrGt_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_TIMESEGS:		return sphCreateSorter2nd<MatchTimeSegments_fn<true> >	( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC2:		return sphCreateSorter2nd<MatchGeneric2_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC3:		return sphCreateSorter2nd<MatchGeneric3_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC4:		return sphCreateSorter2nd<MatchGeneric4_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC5:		return sphCreateSorter2nd<MatchGeneric5_fn<true> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			default:				return NULL;
		}
	} else
	{
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		return sphCreateSorter2nd<MatchRelevanceLt_fn<false> >	( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_ATTR_DESC:	return sphCreateSorter2nd<MatchAttrLt_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_ATTR_ASC:		return sphCreateSorter2nd<MatchAttrGt_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_TIMESEGS:		return sphCreateSorter2nd<MatchTimeSegments_fn<false> >	( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC2:		return sphCreateSorter2nd<MatchGeneric2_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC3:		return sphCreateSorter2nd<MatchGeneric3_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC4:		return sphCreateSorter2nd<MatchGeneric4_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			case FUNC_GENERIC5:		return sphCreateSorter2nd<MatchGeneric5_fn<false> >		( eGroupFunc, bGroupBits, bDistinct, arg ); break;
			default:				return NULL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

ISphMatchSorter * sphCreateQueue ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError )
{
	assert ( pQuery->m_sGroupBy.IsEmpty() || pQuery->m_iGroupbyOffset>=0 );

	ISphMatchSorter * pTop = NULL;
	CSphMatchComparatorState tStateMatch, tStateGroup;

	sError = "";

	////////////////////////////////////
	// choose and setup sorting functor
	////////////////////////////////////

	ESphSortFunc eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc eGroupFunc = FUNC_REL_DESC;
	bool bUsesAttrs = false;
	bool bRandomize = false;

	// matches sorting function
	if ( pQuery->m_eSort==SPH_SORT_EXTENDED )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery->m_sSortBy.cstr(), pQuery, tSchema,
			false, eMatchFunc, tStateMatch, sError );

		if ( eRes==SORT_CLAUSE_ERROR )
			return false;

		if ( eRes==SORT_CLAUSE_RANDOM )
			bRandomize = true;

		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
			if ( tStateMatch.m_iAttr[i]>=0 )
				bUsesAttrs = true;

	} else
	{
		// check sort-by attribute
		if ( pQuery->m_eSort!=SPH_SORT_RELEVANCE )
		{
			tStateMatch.m_iAttr[0] = tSchema.GetAttrIndex ( pQuery->m_sSortBy.cstr() );
			if ( tStateMatch.m_iAttr[0]<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pQuery->m_sSortBy.cstr() );
				return false;
			}

			const CSphColumnInfo & tAttr = tSchema.GetAttr ( tStateMatch.m_iAttr[0] );
			tStateMatch.m_iRowitem[0] = tAttr.m_iRowitem;
			tStateMatch.m_iBitOffset[0] = tAttr.m_iBitOffset;
			tStateMatch.m_iBitCount[0] = tAttr.m_iBitCount;
		}
	
		// find out what function to use and whether it needs attributes
		bUsesAttrs = true;
		switch ( pQuery->m_eSort )
		{
			case SPH_SORT_ATTR_DESC:		eMatchFunc = FUNC_ATTR_DESC; break;
			case SPH_SORT_ATTR_ASC:			eMatchFunc = FUNC_ATTR_ASC; break;
			case SPH_SORT_TIME_SEGMENTS:	eMatchFunc = FUNC_TIMESEGS; break;
			case SPH_SORT_RELEVANCE:		eMatchFunc = FUNC_REL_DESC; bUsesAttrs = false; break;
			default:
				sError.SetSprintf ( "unknown sorting mode %d", pQuery->m_eSort );
				return false;
		}
	}

	// groups sorting function
	if ( pQuery->m_iGroupbyOffset>=0 )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery->m_sGroupSortBy.cstr(), pQuery, tSchema,
			true, eGroupFunc, tStateGroup, sError );

		if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
		{
			if ( eRes==SORT_CLAUSE_RANDOM )
				sError.SetSprintf ( "groups can not be sorted by @random" );
			return false;
		}
	}

	///////////////////
	// spawn the queue
	///////////////////

	bool bMatchBits = tStateMatch.UsesBitfields ();
	bool bGroupBits = tStateGroup.UsesBitfields ();

	if ( pQuery->m_iGroupbyOffset<0 )
	{
		if ( bMatchBits )
		{
			switch ( eMatchFunc )
			{
				case FUNC_REL_DESC:	pTop = new CSphMatchQueue<MatchRelevanceLt_fn<true> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_DESC:pTop = new CSphMatchQueue<MatchAttrLt_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_ASC:	pTop = new CSphMatchQueue<MatchAttrGt_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_TIMESEGS:	pTop = new CSphMatchQueue<MatchTimeSegments_fn<true> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC2:	pTop = new CSphMatchQueue<MatchGeneric2_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC3:	pTop = new CSphMatchQueue<MatchGeneric3_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC4:	pTop = new CSphMatchQueue<MatchGeneric4_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC5:	pTop = new CSphMatchQueue<MatchGeneric5_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_CUSTOM:	pTop = new CSphMatchQueue<MatchCustom_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				default:			pTop = NULL;
			}
		} else
		{
			switch ( eMatchFunc )
			{
				case FUNC_REL_DESC:	pTop = new CSphMatchQueue<MatchRelevanceLt_fn<false> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_DESC:pTop = new CSphMatchQueue<MatchAttrLt_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_ASC:	pTop = new CSphMatchQueue<MatchAttrGt_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_TIMESEGS:	pTop = new CSphMatchQueue<MatchTimeSegments_fn<false> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC2:	pTop = new CSphMatchQueue<MatchGeneric2_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC3:	pTop = new CSphMatchQueue<MatchGeneric3_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC4:	pTop = new CSphMatchQueue<MatchGeneric4_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC5:	pTop = new CSphMatchQueue<MatchGeneric5_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_CUSTOM:	pTop = new CSphMatchQueue<MatchCustom_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				default:			pTop = NULL;
			}
		}

	} else
	{
		pTop = sphCreateSorter1st ( eMatchFunc, bMatchBits, eGroupFunc, bGroupBits, pQuery->m_iDistinctOffset>=0, pQuery );
	}

	if ( !pTop )
	{
		sError.SetSprintf ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)",
			pQuery->m_iGroupbyOffset>=0, eMatchFunc, eGroupFunc );
		return false;
	}

	assert ( pTop );
	pTop->SetState ( tStateMatch );
	pTop->SetGroupState ( tStateGroup );
	pTop->m_bRandomize = bRandomize;
	return pTop;
}


void sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag )
{
	if ( pQueue && pQueue->GetLength() )
	{
		int iOffset = pResult->m_dMatches.GetLength ();
		pResult->m_dMatches.Resize ( iOffset + pQueue->GetLength() );
		pQueue->Flatten ( &pResult->m_dMatches[iOffset], iTag );
	}
}


CSphQueryResult * CSphIndex_VLN::Query ( ISphTokenizer * pTokenizer, CSphDict * pDict, CSphQuery * pQuery )
{
	// create result
	CSphQueryResult * pResult = new CSphQueryResult();

	CSphString sError;
	ISphMatchSorter * pTop = sphCreateQueue ( pQuery, m_tSchema, sError );
	if ( !pTop )
	{
		m_sLastError.SetSprintf ( "failed to create sorting queue: %s", sError.cstr() );
		return pResult;
	}

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
			const DWORD * pMva = &pMvaStorage [ tMatch.GetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount ) ];
			const DWORD * pMvaMax = pMva + (*pMva) + 1;
			pMva++;

			// filter matches if any of multiple values match (ie. are in the set, or in the range)
			bool bOK = false;
			if ( tFilter.m_iValues )
			{
				// walk both attr values and filter values lists, and ok match if there's any intersection
				const DWORD * pFilter = tFilter.m_pValues;
				const DWORD * pFilterMax = pFilter + tFilter.m_iValues;

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
//				if ( bOK )
//					tMatch.SetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount, pFilter[0] );

			} else
			{
				// walk attr list, and ok match if any of values is within range
				while ( pMva<pMvaMax )
				{
					if ( pMva[0]>=tFilter.m_uMinValue && pMva[0]<=tFilter.m_uMaxValue )
					{
						bOK = true;
// FIXME! should we report matched-value somehow?
//						tMatch.SetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount, pMva[0] );
						break;
					}
					pMva++;
				}
			}

			return ( !( tFilter.m_bExclude ^ bOK ) );

		} else
		{
			// single value
			DWORD uValue = tMatch.GetAttr ( tFilter.m_iBitOffset, tFilter.m_iBitCount );
			if ( tFilter.m_iValues )
			{
				if ( !( tFilter.m_bExclude ^ sphGroupMatch ( uValue, tFilter.m_pValues, tFilter.m_iValues ) ) )
					return true;
			} else
			{
				if ( tFilter.m_bExclude ^ ( uValue<tFilter.m_uMinValue || uValue>tFilter.m_uMaxValue ) )
					return true;
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


void CSphIndex_VLN::LookupDocinfo ( CSphDocInfo & tMatch )
{
	DWORD * pFound = const_cast < DWORD * > ( FindDocinfo ( tMatch.m_iDocID ) );
	if ( pFound )
	{
		assert ( tMatch.m_pRowitems );
		assert ( tMatch.m_iRowitems==m_tSchema.GetRowSize() );
		assert ( DOCINFO2ID(pFound)==tMatch.m_iDocID );
		memcpy ( tMatch.m_pRowitems, DOCINFO2ATTRS(pFound), tMatch.m_iRowitems*sizeof(CSphRowitem) );
	}
}


#define SPH_SUBMIT_MATCH(_match) \
	if ( bLateLookup ) \
		LookupDocinfo ( _match ); \
	\
	if ( pTop->m_bRandomize ) \
		(_match).m_iWeight = rand(); \
	\
	if ( pTop->Push ( _match ) ) \
		pResult->m_iTotalMatches++; \
	\
	if ( pQuery->m_iCutoff>0 && pResult->m_iTotalMatches>=pQuery->m_iCutoff ) \
		break;


void CSphIndex_VLN::MatchAll ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult )
{
	///////////////////
	// match all words
	///////////////////

	if ( !m_dQueryWords[0].m_iDocs )
		return;

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

	// preload doclist entries, and calc max qpos
	int i, iMaxQpos = 0;
	for ( i=0; i<m_dQueryWords.GetLength(); i++ )
	{
		m_dQueryWords[i].GetDoclistEntry ();
		iMaxQpos = Max ( iMaxQpos, m_dQueryWords[i].m_iQueryPos );
	}
	iMaxQpos--; // because we'll check base-0 count

	i = 0;
	SphDocID_t docID = 0;
	for ( ;; )
	{
		/////////////////////
		// obtain next match
		/////////////////////

		// scan lists until *all* the ids match
		while ( m_dQueryWords[i].m_tDoc.m_iDocID && docID>m_dQueryWords[i].m_tDoc.m_iDocID )
			m_dQueryWords[i].GetDoclistEntry ();
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
		if ( bEarlyLookup )
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
				m_dQueryWords[imin].GetHitlistEntry ();

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


void CSphIndex_VLN::MatchAny ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult )
{
	//////////////////
	// match any word
	//////////////////

	int iActive = m_dQueryWords.GetLength(); // total number of words still active
	SphDocID_t iLastMatchID = 0;

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

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

		// early reject by group id, doc id or timestamp
		CSphMatch & tMatch = m_dQueryWords[iMinIndex].m_tDoc;
		if ( bEarlyLookup )
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
							CSphBooleanEvalNode ( const CSphBooleanQueryExpr * pNode, CSphDict * pDict, ESphDocinfo eDocinfo, const CSphDocInfo & tMin );
							~CSphBooleanEvalNode ();

	void					SetFile ( CSphIndex_VLN * pIndex, const CSphTermSetup & tTermSetup );
	CSphMatch *				MatchNode ( SphDocID_t iMinID );	///< get next match at this node, with ID greater than iMinID
	CSphMatch *				MatchLevel ( SphDocID_t iMinID );	///< get next match at this level, with ID greater than iMinID
	bool					IsRejected ( SphDocID_t iID );		///< check if this match should be rejected
};


CSphBooleanEvalNode::CSphBooleanEvalNode ( const CSphBooleanQueryExpr * pNode, CSphDict * pDict, ESphDocinfo eDocinfo, const CSphDocInfo & tMin )
{
	assert ( pNode );
	assert ( pDict );

	m_eType = pNode->m_eType;
	m_bInvert = pNode->m_bInvert;
	m_bEvaluable = pNode->m_bEvaluable;

	m_sWord = pNode->m_sWord;
	m_iWordID = 0;

	m_pExpr = pNode->m_pExpr ? new CSphBooleanEvalNode ( pNode->m_pExpr, pDict, eDocinfo, tMin ) : NULL;
	if ( m_pExpr )
	{
		m_pExpr->m_pParent = this;
		m_pLast = &m_tDoc;
	} else
	{
		// GetWordID() may modify the word in-place; so we alloc a tempbuffer
		CSphString sBuf ( m_sWord );
		m_iWordID = pDict->GetWordID ( (BYTE*)sBuf.cstr() );
		m_pLast = NULL;
	}

	m_pNext = pNode->m_pNext ? new CSphBooleanEvalNode ( pNode->m_pNext, pDict, eDocinfo, tMin ) : NULL;
	m_pPrev = NULL;
	if ( m_pNext )
		m_pNext->m_pPrev = this;

	m_pParent = NULL;

	SetupAttrs ( eDocinfo, tMin );
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


bool CSphBooleanEvalNode::IsRejected ( SphDocID_t iID )
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
			m_pLast = m_pExpr->MatchNode ( iID );
			return ( m_pLast && m_pLast->m_iDocID==iID );
		} else
		{
			// subexpr is not evaluable as well
			return m_pExpr->IsRejected ( iID );
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


CSphMatch * CSphBooleanEvalNode::MatchNode ( SphDocID_t iMinID )
{
	assert ( m_bEvaluable );

	// "done" mark
	if ( !m_pLast )
		return NULL;

	// subexpr case
	if ( m_pExpr )
		return m_pExpr->MatchLevel ( iMinID );

	// plain word case
	while ( iMinID > m_tDoc.m_iDocID )
	{
		GetDoclistEntry ();
		if ( !m_tDoc.m_iDocID )
			return NULL;
	}
	return &m_tDoc;
}


CSphMatch * CSphBooleanEvalNode::MatchLevel ( SphDocID_t iMinID )
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
				CSphMatch * pCandidate = pNode->MatchNode ( iMinID );
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
				if ( pNode->IsRejected ( pCur->m_iDocID ) )
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
					pNode->m_pLast = pNode->MatchNode ( iMinID );
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


bool CSphIndex_VLN::MatchBoolean ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult, const CSphTermSetup & tTermSetup )
{
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

	// let's build our own tree! with doclists! and hits!
	assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );
	CSphBooleanEvalNode tTree ( tParsed.m_pTree, tTermSetup.m_pDict, m_eDocinfo, m_tMin );
	tTree.SetFile ( this, tTermSetup );

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

	// do matching
	SphDocID_t iMinID = 1;
	for ( ;; )
	{
		CSphMatch * pMatch = tTree.MatchLevel ( iMinID );
		if ( !pMatch )
			break;
		iMinID = 1 + pMatch->m_iDocID;

		// early reject by group id, doc id or timestamp
		if ( bEarlyLookup )
			LookupDocinfo ( *pMatch );
		if ( sphMatchEarlyReject ( *pMatch, pQuery, m_pMva ) )
			continue;

		// set weight and push it to the queue
		pMatch->m_iWeight = 1; // set weight

		// submit
		SPH_SUBMIT_MATCH ( *pMatch );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

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
	CSphMatch *			GetNextDoc ( SphDocID_t iMinID );
	void				GetNextHit ( DWORD iMinPos );
	void				GetLastTF ( DWORD * pTF ) const;
	inline CSphMatch *	GetLastMatch () const { return m_pLast; }
	void				UpdateLCS ( CSphLCSState & tState );

public:
	DWORD				m_uLastHitPos;

	CSphVector<CSphQueryWord,8>	m_dTerms;	///< query term readers
	CSphMatch *					m_pLast;	///< term with the highest position. NULL if this atom has no more matches
};


CSphExtendedEvalAtom::CSphExtendedEvalAtom ( const CSphExtendedQueryAtom & tAtom, const CSphTermSetup & tSetup )
	: m_pLast ( NULL )
{
	assert ( tSetup.m_pDict );

	// atom assignment
	m_dWords = tAtom.m_dWords;
	m_iField = tAtom.m_iField;
	m_iMaxDistance = tAtom.m_iMaxDistance;

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
		tTerm.SetupAttrs ( tSetup.m_eDocinfo, tSetup.m_tMin );

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

	// OPTIMIZE? these are invariant during evaluation
	assert ( m_iField<0 || m_iField<255 );
#ifndef NDEBUG
	const DWORD uMinHitpos = ( m_iField<0 ) ? 1 : ( m_iField<<24 )+1;
#endif
	const DWORD uMaxHitpos = ( m_iField<0 ) ? UINT_MAX : (1+m_iField)<<24;
	if ( uMaxHitpos<=iMinPos )
		return;

	// skip to necessary min pos
	ARRAY_FOREACH ( i, m_dTerms )
	{
		while ( m_dTerms[i].m_iHitPos && m_dTerms[i].m_iHitPos<iMinPos ) // OPTIMIZE? can remove check for 0
			m_dTerms[i].GetHitlistEntry ();

		if ( !m_dTerms[i].m_iHitPos || m_dTerms[i].m_iHitPos>=uMaxHitpos )
			return;
	}

	if ( m_iMaxDistance==-1 )
	{
		//////////////////
		// next plain hit
		//////////////////

		assert ( m_dTerms.GetLength()==1 ); // OPTIMIZE R&D single-atom leaves; should profile
		assert ( m_dTerms[0].m_iHitPos );
		assert ( m_dTerms[0].m_iHitPos>=uMinHitpos );

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
				assert ( m_dTerms[i].m_iHitPos>=uMinHitpos && m_dTerms[i].m_iHitPos<uMaxHitpos );
				assert ( m_iField<0 || int(m_dTerms[i].m_iHitPos>>24)==m_iField );

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
			if ( !m_dTerms[iMin].m_iHitPos || m_dTerms[iMin].m_iHitPos>=uMaxHitpos )
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
			assert ( uPos>=uMinHitpos && uPos<uMaxHitpos );
			assert ( m_iField<0 || int(uPos>>24)==m_iField );

			// scan forward until this hit is not too early
			const DWORD uRequiredPos = uCandidate + m_dWords[i].m_iAtomPos;
			while ( uPos < uRequiredPos )
			{
				// if one hitlist is over, document is over, because all words must be in the phrase
				m_dTerms[i].GetHitlistEntry ();
				if ( !uPos || uPos>=uMaxHitpos )
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
			assert ( uCandidate>=uMinHitpos && uCandidate<uMaxHitpos );
			assert ( uCandidate>=iMinPos );
			m_uLastHitPos = uCandidate;
		}
	}
}


CSphMatch * CSphExtendedEvalAtom::GetNextDoc ( SphDocID_t iMinID )
{
	if ( !m_pLast )
		return NULL;

	if ( m_pLast->m_iDocID>=iMinID )
	{
		assert ( m_uLastHitPos );
		return m_pLast;
	}

	// OPTIMIZE?
	const DWORD uField = ( m_iField>=0 ) ? ( 1UL<<m_iField ) : 0xffffffffUL;
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
			}

			// check fields
			if (!( tTerm.m_uFields & uField ))
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
		assert ( m_iField<0 || m_iField<255 );
		const DWORD uMinHitpos = ( m_iField<0 ) ? 1 : ( m_iField<<24 )+1;
#ifndef NDEBUG
		const DWORD uMaxHitpos = ( m_iField<0 ) ? UINT_MAX : (1+m_iField)<<24;
#endif

		ARRAY_FOREACH ( i, m_dTerms )
		{
			// do preload
			assert ( !m_dTerms[i].m_iHitPos );
			while ( m_dTerms[i].m_iHitPos<uMinHitpos )
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
			assert ( m_dTerms[i].m_iHitPos>=uMinHitpos && m_dTerms[i].m_iHitPos<uMaxHitpos );
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
		GetNextHit ( uMinHitpos );

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
		// plain 1-term atom  or phrase atom
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
	CSphMatch *			GetNextMatch ( SphDocID_t iMinID, int iTerms, float * pIDF, int iFields, int * pWeights );	///< iTF==0 means that no weighting is required

protected:
	CSphMatch *			GetNextDoc ( SphDocID_t iMinID );
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
	CSphVector<CSphExtendedEvalNode*,8>	m_dChildren;	///< non-plain node children
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


CSphMatch * CSphExtendedEvalNode::GetNextDoc ( SphDocID_t iMinID )
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

		m_pLast = m_tAtom.GetNextDoc ( iMinID );
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
			CSphMatch * pCur = m_dChildren[i]->GetNextDoc ( iMinID );
			assert ( pCur==m_dChildren[i]->GetLastDoc() );

			if ( !pCur )
			{
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
			pMatch = pChild->GetNextDoc ( iMinID );
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
	int iFields, int * pWeights )
{
	CSphMatch * pMatch = GetNextDoc ( iMinID );
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

bool CSphIndex_VLN::MatchExtended ( const CSphQuery * pQuery, ISphMatchSorter * pTop, CSphQueryResult * pResult, const CSphTermSetup & tTermSetup )
{
	assert ( pTop );
	assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );

	//////////////////////////
	// match in extended mode
	//////////////////////////

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

	// parse query
	CSphExtendedQuery tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), tTermSetup.m_pTokenizer,
		&m_tSchema, tTermSetup.m_pDict ) )
	{
		m_sLastError = tParsed.m_sParseError;
		return false;
	}

	CSphExtendedEvalNode tAccept ( tParsed.m_pAccept, tTermSetup );
	CSphExtendedEvalNode tReject ( tParsed.m_pReject, tTermSetup );

	////////////////////
	// build word stats
	////////////////////

	CSphQwordsHash hQwords;
	int iQwords = 0;
	tAccept.CollectQwords ( hQwords, iQwords );

	assert ( iQwords<SPH_BM25_MAX_TERMS );
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
	if ( pTop->m_bRandomize )
		iQwords = 0;

	SphDocID_t iMinID = 1;
	for ( ;; )
	{
		CSphMatch * pAccept = tAccept.GetNextMatch ( iMinID, iQwords, dIDF, m_iWeights, m_dWeights );
		if ( !pAccept )
			break;
		iMinID = 1 + pAccept->m_iDocID;

		CSphMatch * pReject = tReject.GetNextMatch ( pAccept->m_iDocID, 0, NULL, 0, NULL );
		if ( pReject && pReject->m_iDocID==pAccept->m_iDocID )
			continue;

		// early reject by group id, doc id or timestamp
		if ( bEarlyLookup )
			LookupDocinfo ( *pAccept );

		if ( sphMatchEarlyReject ( *pAccept, pQuery, m_pMva ) )
			continue;

		// submit
		SPH_SUBMIT_MATCH ( *pAccept );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::SetupQueryWord ( CSphQueryWord & tWord, const CSphTermSetup & tTermSetup )
{
	// binary search through checkpoints for a one whose range matches word ID
	assert ( m_bPreread[0] && !m_pWordlist.IsEmpty() );

	// empty index?
	if ( !m_iWordlistCheckpoints )
		return false;

	int iWordlistOffset = -1;
	CSphWordlistCheckpoint * pStart = m_pWordlistCheckpoints;
	CSphWordlistCheckpoint * pEnd = m_pWordlistCheckpoints + m_iWordlistCheckpoints - 1;

	if ( tWord.m_iWordID<pStart->m_iWordID )
		return false;

	if ( tWord.m_iWordID>=pEnd->m_iWordID )
	{
		iWordlistOffset = pEnd->m_iWordlistOffset;
	} else
	{
		while ( pEnd-pStart>1 )
		{
			CSphWordlistCheckpoint * pMid = pStart + (pEnd-pStart)/2;

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

		assert ( pStart >= m_pWordlistCheckpoints );
		assert ( pStart <= m_pWordlistCheckpoints+m_iWordlistCheckpoints-1 );
		assert ( tWord.m_iWordID >= pStart[0].m_iWordID && tWord.m_iWordID < pStart[1].m_iWordID );
		iWordlistOffset = pStart->m_iWordlistOffset;
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
	m_pWordlist.Reset ();
	m_pMva.Reset ();

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

	BYTE dCacheInfo [ 8192 ];
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
	if ( m_uVersion>=6 )
	{
		m_bPrefixesOnly = ( rdInfo.GetByte ()!=0 );
		m_iMinInfixLen = rdInfo.GetDword ();
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

	// paranoid MVA verification
	#if PARANOID
	// find out what attrs are MVA
	CSphVector<int,32> dMvaIndexes;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).m_eAttrType & SPH_ATTR_MULTI )
			dMvaIndexes.Add ( i );

	// for each docinfo entry, verify that MVA attrs point to right storage location
	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	for ( DWORD iDoc=0; iDoc<m_uDocinfo; iDoc++ )
	{
		DWORD * pEntry = (DWORD *)&m_pDocinfo[iDoc*iStride];
		SphDocID_t uDocID = DOCINFO2ID(pEntry);

		DWORD uOff = DOCINFO2ATTRS(pEntry) [ dMvaIndexes[0] ];
		if ( !uOff )
		{
			// its either all or nothing
			ARRAY_FOREACH ( i, dMvaIndexes )
				assert ( DOCINFO2ATTRS(pEntry) [ dMvaIndexes[i] ]==0 );
		} else
		{
			// check id
			DWORD * pMva = (DWORD *)&m_pMva[uOff-DOCINFO_IDSIZE];
			SphDocID_t uMvaID = DOCINFO2ID(pMva);
			assert ( uDocID==uMvaID );

			// walk the trail
			ARRAY_FOREACH ( i, dMvaIndexes )
			{
				assert ( DOCINFO2ATTRS(pEntry) [ dMvaIndexes[i] ]==uOff );
				int iCount = m_pMva[uOff];
				uOff += iCount;
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
	assert ( pTokenizer );
	assert ( pDict );
	assert ( pQuery );
	assert ( pResult );
	assert ( pTop );

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	// create result and start timing
	float tmQueryStart = sphLongTimer ();

	// open files
	if ( m_bPreread.IsEmpty() || !m_bPreread[0] )
	{
		m_sLastError = "index not preread";
		return false;
	}
	pResult->m_tSchema = m_tSchema;

	CSphAutofile tDoclist ( GetIndexFileName("spd"), SPH_O_READ, m_sLastError );
	if ( tDoclist.GetFD()<0 )
		return false;

	CSphAutofile tHitlist ( GetIndexFileName ( m_uVersion>=3 ? "spp" : "spd" ), SPH_O_READ, m_sLastError );
	if ( tHitlist.GetFD()<0 )
		return false;

	// prepare for setup
	bool bUseStar = ( m_bPrefixesOnly==true && m_iMinInfixLen>0 );

	CSphDictStar tDictStar ( pDict );
	if ( bUseStar )
	{
		pDict = &tDictStar;

		CSphRemapRange tStar ( '*', '*', '*' ); // FIXME? check and warn if star was already there
		pTokenizer->AddCaseFolding ( tStar );
	}

	CSphTermSetup tTermSetup ( tDoclist, tHitlist );
	tTermSetup.m_pTokenizer = pTokenizer;
	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_eDocinfo;
	tTermSetup.m_tMin = m_tMin;

	PROFILE_END ( query_init );

	// check that docinfo is preloaded
	assert ( m_eDocinfo!=SPH_DOCINFO_EXTERN || !m_pDocinfo.IsEmpty() );

	// split query into words
	bool bSimpleQuery = ( pQuery->m_eMode==SPH_MATCH_ALL
		|| pQuery->m_eMode==SPH_MATCH_ANY
		|| pQuery->m_eMode==SPH_MATCH_PHRASE );
	if ( bSimpleQuery )
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
			m_dQueryWords[i].Reset ();
			m_dQueryWords[i].m_sWord = qp.m_dWords[i].m_sWord;
			m_dQueryWords[i].m_iWordID = qp.m_dWords[i].m_uWordID;
			m_dQueryWords[i].m_iQueryPos = qp.m_dWords[i].m_iQueryPos;

			assert ( m_tMin.m_iRowitems==m_tSchema.GetRowSize() );
			m_dQueryWords[i].SetupAttrs ( m_eDocinfo, m_tMin );
		}


		// lookup group-by attribute index
		if ( !pQuery->SetSchema ( m_tSchema, m_sLastError ) )
			return false;

		// setup words from the wordlist
		PROFILE_BEGIN ( query_load_words );

		ARRAY_FOREACH ( i, m_dQueryWords )
		{
			// if the word was already loaded, just link to it
			int j;
			for ( j=0; j<i; j++ )
				if ( m_dQueryWords[i].m_iWordID == m_dQueryWords[j].m_iWordID )
			{
				m_dQueryWords[i] = m_dQueryWords[j];
				m_dQueryWords[i].m_iQueryPos = 1+i;
				break;
			}
			if ( j<i )
				continue;

			// lookup this wordlist page
			// offset might be -1 if page is totally empty
			SetupQueryWord ( m_dQueryWords[i], tTermSetup );
		}

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

		tFilter.SortValues ();
		tFilter.m_iAttr = -1;
		tFilter.m_iBitOffset = -1;

		for ( int j=0; j<m_tSchema.GetAttrsCount(); j++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(j);
			if ( tFilter.m_sAttrName==tAttr.m_sName )
			{
				tFilter.m_iAttr = j;
				tFilter.m_iBitOffset = tAttr.m_iBitOffset;
				tFilter.m_iBitCount = tAttr.m_iBitCount;
				tFilter.m_bMva = ( ( tAttr.m_eAttrType & SPH_ATTR_MULTI )!=0 );
				break;
			}
		}

		// FIXME! should at least warn about bad filter name
	}

	// build weights
	m_iWeights = m_tSchema.m_dFields.GetLength();
	for ( int i=0; i<m_iWeights; i++ ) // defaults
		m_dWeights[i] = 1;
	if ( pQuery->m_pWeights ) // user-supplied
		for ( int i=0; i<Min ( m_iWeights, pQuery->m_iWeights ); i++ )
			m_dWeights[i] = Max ( 1, pQuery->m_pWeights[i] );

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	PROFILE_BEGIN ( query_match );
	bool bMatch = true;
	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:			MatchAll ( pQuery, pTop, pResult ); break;
		case SPH_MATCH_PHRASE:		MatchAll ( pQuery, pTop, pResult ); break;
		case SPH_MATCH_ANY:			MatchAny ( pQuery, pTop, pResult ); break;
		case SPH_MATCH_BOOLEAN:		bMatch = MatchBoolean ( pQuery, pTop, pResult, tTermSetup ); break;
		case SPH_MATCH_EXTENDED:	bMatch = MatchExtended ( pQuery, pTop, pResult, tTermSetup ); break;
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

	///////////////////
	// cook result set
	///////////////////

	// adjust results
	if ( pTop->GetLength() )
	{
		const int iCount = pTop->GetLength ();
		CSphMatch * const pHead = pTop->First();
		CSphMatch * const pTail = pHead + iCount;

		// lookup attributes if necessary
		if ( m_eDocinfo==SPH_DOCINFO_EXTERN && !pTop->UsesAttrs() && !pQuery->m_dFilters.GetLength() )
			for ( CSphMatch * pCur=pHead; pCur<pTail; pCur++ )
				LookupDocinfo ( *pCur );
	}
	pResult->m_pMva = m_pMva.GetWritePtr();

	// adjust schema
	if ( pQuery->m_iGroupbyOffset>=0 )
	{
		CSphColumnInfo tGroupby ( "@groupby", SPH_ATTR_INTEGER );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_INTEGER );
		CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );

		pResult->m_tSchema.AddAttr ( tGroupby );
		pResult->m_tSchema.AddAttr ( tCount );
		if ( pQuery->m_iDistinctOffset>=0 )
			pResult->m_tSchema.AddAttr ( tDistinct );
	}

	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_iQueryTime += int ( 1000.0f*( sphLongTimer() - tmQueryStart ) );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CRC32/64 DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

/// morphology flags
enum
{
	SPH_MORPH_STEM_EN			= (1UL<<1),
	SPH_MORPH_STEM_RU_CP1251	= (1UL<<2),
	SPH_MORPH_STEM_RU_UTF8		= (1UL<<3),
	SPH_MORPH_SOUNDEX			= (1UL<<4),
	SPH_MORPH_LIBSTEMMER		= (1UL<<5)
};


/// CRC32/64 dictionary
struct CSphDictCRC : CSphDict
{
						CSphDictCRC ();
	virtual				~CSphDictCRC ();

	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen );
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer );
	virtual bool		SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError );

protected:
	DWORD				m_iMorph;		///< morphology flags
#if USE_LIBSTEMMER
	sb_stemmer *		m_pStemmer;
#endif

	int					m_iStopwords;	///< stopwords count
	SphWordID_t *		m_pStopwords;	///< stopwords ID list

protected:
	SphWordID_t			FilterStopword ( SphWordID_t uID );	///< filter ID against stopwords list
};

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

SphOffset_t sphFNV64 ( const BYTE * s )
{
	SphOffset_t hval = 0xcbf29ce484222325ULL;
	while ( *s )
	{
		// xor the bottom with the current octet
		hval ^= (SphOffset_t)*s++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}


SphOffset_t sphFNV64 ( const BYTE * s, int iLen )
{
	SphOffset_t hval = 0xcbf29ce484222325ULL;
	for ( ; iLen>0; iLen-- )
	{
		// xor the bottom with the current octet
		hval ^= (SphOffset_t)*s++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}

/////////////////////////////////////////////////////////////////////////////

CSphDictCRC::CSphDictCRC ()
	: m_iMorph		( 0 )
#if USE_LIBSTEMMER
	, m_pStemmer	( NULL )
#endif

	, m_iStopwords	( 0 )
	, m_pStopwords	( NULL )
{
}


CSphDictCRC::~CSphDictCRC ()
{
#if USE_LIBSTEMMER
	sb_stemmer_delete ( m_pStemmer );
#endif
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

/////////////////////////////////////////////////////////////////////////////

template < typename T > T sphCRCWord ( const BYTE * pWord );
template<> uint64_t sphCRCWord ( const BYTE * pWord ) { return sphFNV64 ( pWord ); }
template<> DWORD sphCRCWord ( const BYTE * pWord ) { return sphCRC32 ( pWord ); }

template < typename T > T sphCRCWord ( const BYTE * pWord, int iLen );
template<> uint64_t sphCRCWord ( const BYTE * pWord, int iLen ) { return sphFNV64 ( pWord, iLen ); }
template<> DWORD sphCRCWord ( const BYTE * pWord, int iLen ) { return sphCRC32 ( pWord, iLen ); }


SphWordID_t CSphDictCRC::GetWordID ( BYTE * pWord )
{
	if ( m_iMorph & SPH_MORPH_STEM_EN )
		stem_en ( pWord );

	if ( m_iMorph & SPH_MORPH_STEM_RU_CP1251 )
		stem_ru_cp1251 ( pWord );

	if ( m_iMorph & SPH_MORPH_STEM_RU_UTF8 )
		stem_ru_utf8 ( (WORD*)pWord );

	if ( m_iMorph & SPH_MORPH_SOUNDEX )
		stem_soundex ( pWord );

#if USE_LIBSTEMMER
	if ( m_iMorph & SPH_MORPH_LIBSTEMMER )
	{
		assert ( m_pStemmer );

		const sb_symbol * sStemmed = sb_stemmer_stem ( m_pStemmer, (sb_symbol*)pWord, strlen((const char*)pWord) );
		int iLen = sb_stemmer_length ( m_pStemmer );

		memcpy ( pWord, sStemmed, iLen );
		pWord[iLen] = '\0';		
	}
#endif

	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord ) );
}


SphWordID_t CSphDictCRC::GetWordID ( const BYTE * pWord, int iLen )
{
	return FilterStopword ( sphCRCWord<SphWordID_t> ( pWord, iLen ) );
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

	static BYTE sBuffer [ 65536 ];

	// tokenize file list
	if ( !sFiles )
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

		// open file
		FILE * fp = fopen ( sName, "rb" );
		if ( !fp )
		{
			sphWarn ( "failed to load stopwords from '%s'", sName );
			continue;
		}

		// tokenize file
		CSphVector<SphWordID_t> dStop;
		int iLength;
		do
		{
			iLength = (int)fread ( sBuffer, 1, sizeof(sBuffer), fp );
			pTokenizer->SetBuffer ( sBuffer, iLength, iLength!=0 );

			BYTE * pToken;
			while ( ( pToken = pTokenizer->GetToken() )!=NULL )
				dStop.Add ( GetWordID ( pToken ) );
		} while ( iLength );

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
	}

	SafeDeleteArray ( sList );
}


bool CSphDictCRC::SetMorphology ( const CSphVariant * sMorph, bool bUseUTF8, CSphString & sError )
{
	m_iMorph = 0;
#if USE_LIBSTEMMER
	sb_stemmer_delete ( m_pStemmer );
	m_pStemmer = NULL;
#endif

	if ( !sMorph )
		return true;

	DWORD iStemRu = ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
	CSphString sOption = *sMorph;
	sOption.ToLower ();

	if ( sOption.IsEmpty() || sOption=="none" )
	{
		return true;

	} else if ( sOption=="stem_en" )
	{
		m_iMorph = SPH_MORPH_STEM_EN;

	} else if ( sOption=="stem_ru" )
	{
		m_iMorph = iStemRu;
		stem_ru_init ();

	} else if ( sOption=="stem_enru" )
	{
		m_iMorph = iStemRu | SPH_MORPH_STEM_EN;
		stem_ru_init ();

	} else if ( sOption=="soundex" )
	{
		m_iMorph = SPH_MORPH_SOUNDEX;

	}
	
#if USE_LIBSTEMMER
	else if ( sOption.Begins ( "libstemmer_" ) )
	{
		const char * sAlgo = sOption.cstr() + strlen ( "libstemmer_" );
		if ( bUseUTF8 )
		{
			m_pStemmer = sb_stemmer_new ( sAlgo, "UTF_8" );
		} else
		{
			m_pStemmer = sb_stemmer_new ( sAlgo, "ISO_8859_1" );
			if ( !m_pStemmer )
				m_pStemmer = sb_stemmer_new ( sAlgo, "ISO_8859_2" );
			if ( !m_pStemmer )
				m_pStemmer = sb_stemmer_new ( sAlgo, "KOI8_R" );
		}

		if ( !m_pStemmer )
		{
			sError.SetSprintf (  "libstemmer morphology algorithm '%s' not available for %s encoding - IGNORED",
				sAlgo, bUseUTF8 ? "UTF-8" : "SBCS" );
			return false;
		}

		m_iMorph = SPH_MORPH_LIBSTEMMER;

	}
#endif

	else
	{
		sError.SetSprintf ( "unknown morphology option '%s' - IGNORED", sOption.cstr() );
		return false;
	}
	return true;
}


CSphDict * sphCreateDictionaryCRC ()
{
	return new CSphDictCRC ();
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
	const char *	m_sEntity;
	int				m_iLen;
	BYTE			m_cStripped;
};


static const HtmlEntity_t g_dHtmlEntities[] =
{
	{ "&amp;",		5,	'&' },
	{ "&lt;",		4,	'<' },
	{ "&gt;",		4,	'>' },
	{ "&quot;",		6,	'"' },
	{ "&apos;",		6,	'\'' }
};
static const int g_iHtmlEntities = sizeof(g_dHtmlEntities)/sizeof(HtmlEntity_t);


BYTE * CSphHTMLStripper::Strip ( BYTE * sData )
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
				// handle known list
				// FIXME! add bigger table and hashed search here
				int i;
				for ( i=0; i<g_iHtmlEntities; i++ )
					if ( strncmp ( (const char*)s, g_dHtmlEntities[i].m_sEntity, g_dHtmlEntities[i].m_iLen )==0 )
				{
					*d++ = g_dHtmlEntities[i].m_cStripped;
					s += g_dHtmlEntities[i].m_iLen;
					break;
				}
				if ( i==g_iHtmlEntities )
				{
					// unknown entity
					*d++ = *s++;
				}
			}
			continue;
		}

		// handle tag
		assert ( *s=='<' );
		if ( !isalpha(s[1]) && s[1]!='/' && s[1]!='!' )
		{
			*d++ = *s++;
			continue;
		}
		s++; // skip '<'

		// check if it's a comment
		if ( s[0]=='!' && s[1]=='-' && s[2]=='-' )
		{
			// scan until comment end
			s += 3; // skip opening '!--'
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
		}

		// check if it's known tag or what
		TagLink_t * pTag;
		int iLen = 0;
		for ( pTag=m_pTags; pTag; pTag=pTag->m_pNext )
		{
			iLen = (int)strlen(pTag->m_sTag); // OPTIMIZE! cache strlen() result
			if ( strncasecmp ( pTag->m_sTag, (const char*)s, iLen ) )
				continue;
			if ( !isalpha ( s[iLen] ) )
				break;
		}

		// processing unknown tags is somewhat easy
		if ( !pTag )
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
			continue;
		}

		// remove tag name
		while ( iLen-- ) s++;
		assert ( !isalpha(*s) );

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
				if ( isalpha(*s) )
					break;
				s++;
			}
			if ( !isalpha(*s) )
			{
				if ( *s ) s++;
				break;
			}

			// check attribute name
			AttrLink_t * pAttr;
			int iLen2 = 0;
			for ( pAttr=pTag->m_pAttrs; pAttr; pAttr=pAttr->m_pNext )
			{
				iLen2 = (int)strlen(pAttr->m_sAttr); // OPTIMIZE! cache strlen() result
				if ( strncasecmp ( pAttr->m_sAttr, (const char*)s, iLen2 ) )
					continue;
				if ( s[iLen2]=='=' || isspace(s[iLen2]) )
					break;
			}

			// if attribute is unknown or malformed, we just skip all alphas and rescan
			if ( !pAttr )
			{
				while ( isalpha(*s) ) s++;
				continue;
			}

			// attribute is known
			while ( iLen2-- ) s++;

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
	}

	*d++ = '\0';
	return sData;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

CSphSource::CSphSource ( const char * sName )
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_tSchema ( sName )
	, m_bStripHTML ( false )
	, m_iMinInfixLen ( 0 )
	, m_bPrefixesOnly ( false )
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


bool CSphSource::UpdateSchema ( CSphSchema * pInfo, CSphString & sError )
{
	assert ( pInfo );
	assert ( m_tSchema.m_dFields.GetLength()>0 );

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


void CSphSource::SetEmitInfixes ( bool bPrefixesOnly, int iMinLength )
{
	m_iMinInfixLen = Max ( iMinLength, 0 );
	m_bPrefixesOnly = bPrefixesOnly;
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
	m_dHits.Resize ( 0 );

	for ( int j=0; j<m_tSchema.m_dFields.GetLength(); j++ )
	{
		BYTE * sField = dFields[j];
		if ( !sField )
			continue;

		if ( m_bStripHTML )
			m_pStripper->Strip ( sField );

		int iLen = (int) strlen ( (char*)sField );
		m_tStats.m_iTotalBytes += iLen;

		m_pTokenizer->SetBuffer ( sField, iLen, true );

		BYTE * sWord;
		int iPos = ( j<<24 ) + 1;

		if ( m_iMinInfixLen )
		{
			// index all infixes
			while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
			{
				int iLen = m_pTokenizer->GetLastTokenLen ();

				// always index full word (maybe with magic tail marker)
				if ( m_bPrefixesOnly )
				{
					int iBytes = strlen ( (const char*)sWord );
					sWord[iBytes] = MAGIC_WORD_TAIL;
					sWord[iBytes+1] = '\0';
				}

				SphWordID_t iWord = m_pDict->GetWordID ( sWord );
				if ( iWord )
				{
					CSphWordHit & tHit = m_dHits.Add ();
					tHit.m_iDocID = m_tDocInfo.m_iDocID;
					tHit.m_iWordID = iWord;
					tHit.m_iWordPos = iPos;
				}

				// if there are no infixes, that's it
				if ( m_iMinInfixLen>=iLen )
				{
					iPos++;
					continue;
				}

				// process all infixes
				int iMaxStart = m_bPrefixesOnly ? 0 : ( iLen-m_iMinInfixLen );

				BYTE * sInfix = sWord;
				for ( int iStart=0; iStart<=iMaxStart; iStart++ )
				{
					BYTE * sInfixEnd = sInfix;
					for ( int i=0; i<m_iMinInfixLen; i++ )
						sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );

					for ( int i=m_iMinInfixLen; i<=iLen-iStart; i++ )
					{
						SphWordID_t iWord = m_pDict->GetWordID ( sInfix, sInfixEnd-sInfix );
						sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );

						if ( iWord )
						{
							CSphWordHit & tHit = m_dHits.Add ();
							tHit.m_iDocID = m_tDocInfo.m_iDocID;
							tHit.m_iWordID = iWord;
							tHit.m_iWordPos = iPos;
						}
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

	CSphVector<bool,256> dFound;
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

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		// shortcuts
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);

		DWORD uValue = 0;
		if ( tAttr.m_eAttrType==SPH_ATTR_ORDINAL )
		{
			// memorize string for ordinals; fixup NULLs
			m_dStrAttrs[i] = SqlColumn ( tAttr.m_iIndex );
			if ( !m_dStrAttrs[i].cstr() )
				m_dStrAttrs[i] = "";

		} else if (!( tAttr.m_eAttrType & SPH_ATTR_MULTI ))
		{
			// just store as uint by default
			uValue = sphToDword ( SqlColumn ( tAttr.m_iIndex ) ); // FIXME? report conversion errors maybe?
		}

		m_tDocInfo.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, uValue );
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

/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_MYSQL

CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
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
		m_tParams.m_sDB.cstr(), m_tParams.m_iPort, m_sMysqlUsock.cstr(), 0 );
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
	: CSphSource ( sName )
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
	Disconnect ();
}


void CSphSource_XMLPipe::Disconnect ()
{
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
	m_bBody = false;

	m_tSchema.Reset ();

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
		sError.SetSprintf ( "xmlpipe: failed to popen '%s'", m_sCommand.cstr() );

	return ( m_pPipe!=NULL );
}


bool CSphSource_XMLPipe::IterateHitsNext ( CSphString & sError )
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

			m_pTokenizer->SetBuffer ( (BYTE*)sTitle, iLen, true );
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
			sError.SetSprintf ( "xmlpipe: unexpected EOF while scanning docid=" DOCID_FMT " body",
				m_tDocInfo.m_iDocID );
			return false;
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
	}

	// some tag was found
	if ( bBodyEnd )
	{
		// let's check if it's '</body>' which is the only allowed tag at this point
		SetTag ( "body" );
		if ( !SkipTag ( false, sError ) )
			return false;

		// well, it is
		m_bBody = false;

		// let's check if it's '</document>' which is the only allowed tag at this point
		SetTag ( "document" );
		if ( !SkipTag ( false, sError ) )
			return false;
	}

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
	assert ( m_pBuffer!=&m_sBuffer[0] );

	int iLeft = Max ( m_pBufferEnd-m_pBuffer, 0 );
	if ( iLeft>0 )
		memmove ( m_sBuffer, m_pBuffer, iLeft );

	size_t iLen = fread ( &m_sBuffer [ iLeft ], 1, sizeof(m_sBuffer)-iLeft, m_pPipe );
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
	
	DWORD iWordPos = 0;
	CSphVector< DWORD > dWordPosIndex;
	CSphReader_VLN * pReader = pSource->m_pDoclistReader;
	CSphReader_VLN * pHitlistReader = pSource->m_pHitlistReader;
	assert ( pReader && pHitlistReader );
	
	m_dWordPos.Reset();
	m_dDoclist.Reset();

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

	for( int i = 0; i < m_dWordPos.GetLength(); i++ )
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

	for( int i = 0; i < m_dDoclist.GetLength(); i++ )
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
		m_iHitNum = sphUnzipInt( pSource );
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

void CSphWordRecord::Read ()
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
	}	
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
