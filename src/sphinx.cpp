//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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
		fprintf ( stdout, "%s", sBuffer );
		exit ( 1 );
	}
}

#endif // !NDEBUG
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *	SPHINX_DEFAULT_SBCS_TABLE	= "0..9, A..Z->a..z, _, a..z, U+A8->U+B8, U+B8, U+C0..U+DF->U+E0..U+FF, U+E0..U+FF";
const char *	SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F";

/////////////////////////////////////////////////////////////////////////////

static bool		g_bSphQuiet		= false;

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
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_BINARY O_BINARY
#else
#define SPH_BINARY 0
#endif

/// file which closes automatically when going out of scope
class CSphAutofile
{
protected:
	int		m_iFD;		///< my file descriptior

public:
			CSphAutofile ( const char * sName )	{ m_iFD = ::open ( sName, O_RDONLY | SPH_BINARY ); }
			CSphAutofile ( int iFD )			{ m_iFD = iFD; }
			~CSphAutofile ()					{ Close (); }

	int		GetFD () const						{ return m_iFD; }
	void	SetFD ( int iFD )					{ Close (); m_iFD = iFD; }
	void	Close ()							{ if ( m_iFD>=0 ) ::close ( m_iFD ); m_iFD = -1; }

	const CSphAutofile & operator = ( const CSphAutofile & )	{ assert(0); return *this; }
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

/// additional comparator parameters
struct CSphMatchComparatorState
{
	int		m_iAttr;
	DWORD	m_iNow;
};


/// my match-sorting priority queue
template < typename T, typename COMP > class CSphQueue
	: public ISphQueue<T>
{
protected:
	T *		m_pData;
	int		m_iUsed;
	int		m_iSize;
	CSphMatchComparatorState	m_tState;

public:
	/// ctor
	CSphQueue ( int iSize, bool bUsesAttrs )
		: ISphQueue<T> ( bUsesAttrs )
		, m_iUsed ( 0 )
		, m_iSize ( iSize )
	{
		assert ( iSize>0 );
		m_pData = new T [ iSize ];
		assert ( m_pData );

		m_tState.m_iAttr = 0;
		m_tState.m_iNow = (DWORD) time ( NULL );
	}

	/// dtor
	~CSphQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	virtual void Push ( const T & tEntry )
	{
		if ( m_iUsed==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, m_pData[0], m_tState ) )
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
			if ( !COMP::IsLess ( m_pData[iEntry], m_pData[iParent], m_tState ) )
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

	/// set attr
	virtual void SetAttr ( int iAttr )
	{
		m_tState.m_iAttr = iAttr;
	}

	/// get attr
	virtual int GetAttr ()
	{
		return m_tState.m_iAttr;
	}
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
	void	Init ( int iFD, SphOffset_t * pSharedOffset, const char * sPhase, int iMemoryLimit, int iBlocks );

	DWORD	ReadVLB ();
	int		ReadByte ();
	int		ReadBytes ( void * pDest, int iBytes );
	int		ReadHit ( CSphWordHit * e, int iAttrs, DWORD * pAttrs );
};

/////////////////////////////////////////////////////////////////////////////

class CSphWriter_VLN
{
public:
	CSphString	m_sName;
	SphOffset_t	m_iPos;

public:
				CSphWriter_VLN ( char * sName );

	int			OpenFile ();
	void		PutBytes ( const void * pData, int iSize );
	void		PutRawBytes ( const void * pData, int iSize );
	void		PutDword ( DWORD uValue ) { PutRawBytes ( &uValue, sizeof(DWORD) ); }
	void		CloseFile ();
	void		SeekTo ( SphOffset_t pos );

	void		ZipInt ( DWORD uValue );
	void		ZipOffset ( SphOffset_t uValue );
	void		ZipOffsets ( CSphVector<SphOffset_t> * pData );

	bool		IsError ();

private:
	int			m_iFD;
	int			m_iPoolUsed;
	BYTE		m_dPool [ SPH_CACHE_WRITE ];
	BYTE *		m_pPool;
	bool		m_bError;

	void		PutByte ( int data );
	void		Flush ();
};

/////////////////////////////////////////////////////////////////////////////

class CSphReader_VLN
{
public:

				CSphReader_VLN ();
	virtual		~CSphReader_VLN ();

	void		SetFile ( int iFD );
	void		Reset ();

	void		SeekTo ( SphOffset_t iPos );

	void		GetRawBytes ( void * pData, int iSize );
	void		GetBytes ( void * data, int size );
	DWORD		GetDword ();
	CSphString	GetString ();

	DWORD		UnzipInt ();
	SphOffset_t	UnzipOffset ();

	const CSphReader_VLN &	operator = ( const CSphReader_VLN & rhs );

private:

	int			m_iFD;
	SphOffset_t	m_iPos;

	int			m_iBuffPos;
	int			m_iBuffUsed;
	int			m_iBufSize;
	BYTE *		m_pBuff;

private:
	int			GetByte ();
	void		UpdateCache ();
};

/////////////////////////////////////////////////////////////////////////////

/// query word from the searcher's point of view
class CSphQueryWord
{
public:
	CSphString		m_sWord;		///< my copy of word
	int				m_iQueryPos;	///< word position, from query
	DWORD			m_iWordID;		///< word ID, from dictionary

	int				m_iDocs;		///< document count, from wordlist
	int				m_iHits;		///< hit count, from wordlist

	CSphMatch		m_tDoc;			///< current match (partial)
	DWORD			m_uFields;		///< current match fields
	DWORD			m_iHitPos;		///< current hit postition, from hitlist

	SphOffset_t		m_iHitlistPos;	///< current position in hitlist, from doclist

	CSphReader_VLN	m_rdDoclist;	///< my doclist reader
	CSphReader_VLN	m_rdHitlist;	///< my hitlist reader

	DWORD			m_iMinID;		///< min ID to fixup
	int				m_iInlineAttrs;	///< inline attributes count
	DWORD *			m_pInlineFixup;	///< inline attributes fixup (POINTER TO EXTERNAL DATA, NOT MANAGED BY THIS CLASS!)

public:
	CSphQueryWord ()
		: m_sWord ( NULL )
		, m_iQueryPos ( -1 )
		, m_iWordID ( 0 )
		, m_iDocs ( 0 )
		, m_iHits ( 0 )
		, m_uFields ( 0 )
		, m_iHitPos ( 0 )
		, m_iHitlistPos ( 0 )
		, m_iMinID ( 0 )
		, m_iInlineAttrs ( 0 )
		, m_pInlineFixup ( NULL )
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
		m_tDoc.Reset ();
		m_uFields = 0;
	}

	void GetDoclistEntry ()
	{
		DWORD iDeltaDoc = m_rdDoclist.UnzipInt ();
		if ( iDeltaDoc )
		{
			if ( !m_tDoc.m_iDocID )
				m_tDoc.m_iDocID = m_iMinID;
			m_tDoc.m_iDocID += iDeltaDoc;

			for ( int i=0; i<m_iInlineAttrs; i++ )
				m_tDoc.m_pAttrs[i] = m_rdDoclist.UnzipInt () + m_pInlineFixup[i];

			SphOffset_t iDeltaPos = m_rdDoclist.UnzipOffset ();
			assert ( iDeltaPos>0 );

			m_iHitlistPos += iDeltaPos;
			m_rdHitlist.SeekTo ( m_iHitlistPos );
			m_iHitPos = 0;

			// FIXME? incompatible with 096
			m_uFields = m_rdDoclist.UnzipInt ();

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

struct CSphWordlistCheckpoint
{
	DWORD	m_iWordID;
	DWORD	m_iWordlistOffset;
};

/// this is my actual VLN-compressed phrase index implementation
struct CSphIndex_VLN : CSphIndex
{
								CSphIndex_VLN ( const char * sFilename );
								~CSphIndex_VLN ();

	virtual int					Build ( CSphDict * pDict, const CSphVector < CSphSource * > & dSources, int iMemoryLimit, ESphDocinfo eDocinfo );
	virtual CSphQueryResult *	Query ( CSphDict * pDict, CSphQuery * pQuery);
	virtual bool				QueryEx ( CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchQueue * pTop );
	virtual const CSphSchema *	Preload ();

private:
	static const int			WORDLIST_CHECKPOINT		= 1024;		///< wordlist checkpoints frequency
	static const int			WRITE_BUFFER_SIZE		= 262144;	///< my write buffer size

	CSphString					m_sFilename;
	SphOffset_t					m_iFilePos;
	CSphWriter_VLN *			m_fdWordlist;
	CSphWriter_VLN *			fdData;

	CSphVector<SphOffset_t>		m_dDoclist;

	ESphDocinfo					m_eDocinfo;
	CSphDocInfo					m_tMin;				///< min attribute values tracker

	BYTE *						m_pWriteBuffer;					///< my write buffer

	CSphWordHit					m_tLastHit;			///< hitlist entry
	SphOffset_t					m_iLastHitlistPos;	///< doclist entry
	DWORD						m_uFields;			///< doclist entry
	SphOffset_t					m_iLastDoclistPos;	///< wordlist entry
	int							m_iLastWordDocs;	///< wordlist entry
	int							m_iLastWordHits;	///< wordlist entry

	int									m_iWordlistEntries;		///< wordlist entries written since last checkpoint
	CSphVector<CSphWordlistCheckpoint>	m_dWordlistCheckpoints;	///< wordlist checkpoint offsets

	CSphVector<CSphQueryWord,8>	m_dQueryWords;		///< search query words for "simple" query types (ie. match all/any/phrase)

	int							m_iWeights;						///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights count

private:
	DWORD *						m_pDocinfo;				///< my docinfo cache
	int							m_iDocinfo;				///< my docinfo cache size

	static const int			DOCINFO_HASH_BITS = 18;	// FIXME! make this configurable
	DWORD *						m_pDocinfoHash;
	int							m_iDocinfoIdShift;

	BYTE *						m_pWordlist;			///< my wordlist cache
	CSphWordlistCheckpoint *	m_pWordlistCheckpoints;	///< my wordlist cache
	int							m_iWordlistCheckpoints;	///< my wordlist cache

	bool						m_bPreloaded;			///< if schema/docinfos are preloaded

private:
	int							OpenFile ( char * ext, int mode );
	int							AdjustMemoryLimit ( int iMemoryLimit );

	int							cidxCreate ();
	int							cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits, DWORD * pDocinfo, int Docinfos, int iStride );
	void						cidxHit ( CSphWordHit * pHit, DWORD * pDocinfos );
	bool						cidxDone ();

	void						WriteSchemaColumn ( CSphWriter_VLN * fdInfo, const CSphColumnInfo & tCol );
	void						ReadSchemaColumn ( CSphReader_VLN & rdInfo, CSphColumnInfo & tCol );

	void						MatchAll ( const CSphQuery * pQuery, ISphMatchQueue * pTop, CSphQueryResult * pResult );
	void						MatchAny ( const CSphQuery * pQuery, ISphMatchQueue * pTop, CSphQueryResult * pResult );
	void						MatchBoolean ( const CSphQuery * pQuery, CSphDict * pTree, ISphMatchQueue * pTop, CSphQueryResult * pResult, int iDoclistFD );

	void						LookupDocinfo ( CSphDocInfo & tMatch );

public:
	// FIXME! this needs to be protected, and refactored as well
	bool						SetupQueryWord ( CSphQueryWord & tWord, int iFD );
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
// MEMORY MANAGEMENT
/////////////////////////////////////////////////////////////////////////////

void * operator new ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void * operator new [] ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void operator delete ( void * pPtr )
{
	if ( pPtr )
		::free ( pPtr );
}


void operator delete [] ( void * pPtr )
{
	if ( pPtr )
		::free ( pPtr );
}

/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

void sphDie(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vfprintf(stdout, message, ap);
	va_end(ap);
	exit(1);
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


int sphWrite ( int iFD, const void * pBuf, size_t iCount, const char * sName )
{
	int iWritten = ::write ( iFD, pBuf, iCount );
	if ( iWritten!=(int)iCount )
	{
		if ( iWritten<0 )
		{
			fprintf ( stdout, "ERROR: %s: write error: %s.\n",
				sName, strerror(errno) );
		} else
		{
			fprintf ( stdout, "ERROR: %s: write error: %d of %d bytes written.\n",
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


char * sphStrMacro ( const char * sTemplate, const char * sMacro, int iValue )
{
	// expand macro
	char sExp [ 16 ];
	snprintf ( sExp, sizeof(sExp), "%d", iValue );

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


int myatoi ( const char * nptr )
{
	return nptr ? atoi ( nptr ) : 0;
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

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

	virtual void				SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );
	virtual BYTE *				GetToken ();
	virtual ISphTokenizer *		Clone () const;

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
	virtual ISphTokenizer *		Clone () const;

protected:
	int					GetCodePoint ();
	void				AccumCodePoint ( int iCode );
	void				FlushAccum ();

protected:
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
	: m_pData ( NULL )
{
}


CSphLowercaser::~CSphLowercaser ()
{
	SafeDeleteArray ( m_pData );
}


void CSphLowercaser::SetRemap ( const CSphLowercaser * pLC )
{
	if ( !pLC )
		return;

	m_iChunks = pLC->m_iChunks;

	SafeDeleteArray ( m_pData );
	m_pData = new int [ m_iChunks*CHUNK_SIZE ];
	memcpy ( m_pData, pLC->m_pData, sizeof(int)*m_iChunks*CHUNK_SIZE );

	for ( int i=0; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = pLC->m_pChunk[i]
			? pLC->m_pChunk[i] - pLC->m_pData + m_pData
			: NULL;
}


void CSphLowercaser::SetRemap ( const CSphRemapRange * pRemaps, int iRemaps )
{
	SafeDeleteArray ( m_pData );

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

	m_iChunks = 0;
	for ( int i=0; i<CHUNK_COUNT; i++ )
		if ( dUsed[i] )
			m_iChunks++;

	#undef LOC_CHECK_RANGE

	// alloc
	assert ( m_iChunks );
	m_pData = new int [ m_iChunks*CHUNK_SIZE ];
	memset ( m_pData, 0, sizeof(int)*m_iChunks*CHUNK_SIZE );

	memset ( m_pChunk, 0, sizeof(m_pChunk) );

	// build chunks hash
	int * pChunk = m_pData;
	for ( int i=0; i<CHUNK_COUNT; i++ )
		if ( dUsed[i] )
	{
		m_pChunk[i] = pChunk;
		pChunk += CHUNK_SIZE;
	}

	// fill the actual remapping table chunks
	pRemap = pRemaps;
	for ( int i=0; i<iRemaps; i++, pRemap++ )
	{
		int iRemapped = pRemap->m_iRemapStart;
		for ( int j = pRemap->m_iStart; j <= pRemap->m_iEnd; j++, iRemapped++ )
		{
			assert ( m_pChunk [ j>>CHUNK_BITS ] );
			m_pChunk [ j>>CHUNK_BITS ] [ j & CHUNK_MASK ] = iRemapped;
		}
	}
}


bool CSphLowercaser::SetRemap ( const char * sConfig )
{
	CSphCharsetDefinitionParser tParser;
	if ( tParser.Parse ( sConfig, *this )!=0 )
	{
		fprintf ( stdout, "ERROR: %s", tParser.GetLastError() );
		return false;
	}
	return true;
}


void CSphLowercaser::AddSpecials ( const char * sSpecials )
{
	assert ( sSpecials );

	// build new chunks map
	// 0 means "was unused"
	// 1 means "was used"
	// 2 means "is used now"
	int dUsed [ CHUNK_COUNT ];
	for ( int i=0; i<CHUNK_COUNT; i++ )
		dUsed[i] = m_pChunk[i] ? 1 : 0;

	int iNewChunks = m_iChunks;
	for ( const BYTE * c = (const BYTE *) sSpecials; *c; c++ )
	{
		int iChunk = ((int)(*c)) >> CHUNK_BITS;

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
		assert ( pChunk-m_pData==iNewChunks*CHUNK_SIZE );

		SafeDeleteArray ( m_pData );
		m_pData = pData;
		m_iChunks = iNewChunks;
	}

	// fill new stuff
	for ( const BYTE * c = (const BYTE *) sSpecials; *c; c++ )
	{
		int j = (int)(*c);
		assert ( m_pChunk [ j>>CHUNK_BITS ] );
		m_pChunk [ j>>CHUNK_BITS ] [ j & CHUNK_MASK ] = -j;
	}
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

bool ISphTokenizer::SetCaseFolding ( const char * sConfig )
{
	CSphCharsetDefinitionParser tParser;
	if ( tParser.Parse ( sConfig, m_tLC )!=0 )
	{
		fprintf ( stdout, "ERROR: %s", tParser.GetLastError() );
		return false;
	}
	return true;
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	m_tLC.AddSpecials ( sSpecials );
}

/////////////////////////////////////////////////////////////////////////////

CSphTokenizer_SBCS::CSphTokenizer_SBCS ()
	: m_pBuffer		( NULL )
	, m_pBufferMax	( NULL )
	, m_pCur		( NULL )
	, m_iAccum		( 0 )
	, m_bLast		( false )
{
	SetCaseFolding ( SPHINX_DEFAULT_SBCS_TABLE );
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
				return NULL;
		} else
		{
			iCode = m_tLC.ToLower ( *m_pCur++ );
		}

		// skip whitespace and short words
		if ( m_iAccum<m_iMinWordLen && iCode==0 )
		{
			m_iAccum = 0;
			continue;
		}

		// handle whitespace and specials
		if ( iCode<=0 )
		{
			// skip short words
			if ( m_iAccum<m_iMinWordLen )
				m_iAccum = 0;

			if ( m_iAccum==0 )
			{
				// handle specials
				assert ( iCode<0 && iCode>=-255 );
				m_sAccum[0] = (BYTE)(-iCode);
				m_sAccum[1] = '\0';
			} else
			{
				// flush accum in any case
				m_sAccum[m_iAccum] = '\0';

				// we'll need to redo special char again
				if ( iCode<0 )
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
	return pClone;
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
	SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE );
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
	for ( ;; )
	{
		// get next codepoint
		int iCode = 0;
		BYTE * pCur = m_pCur; // to redo special char, if there's a token already
		if ( m_pCur>=m_pBufferMax )
		{
			if ( !m_bLast || m_iAccum<m_iMinWordLen )
				return NULL;
		} else
		{
			iCode = GetCodePoint(); // advances m_pCur
		}

		// skip whitespaces and short words
		if ( m_iAccum<m_iMinWordLen && iCode==0 )
		{
			FlushAccum ();
			continue;
		}

		// handle whitespace and specials
		if ( iCode<=0 )
		{
			// skip short words
			if ( m_iAccum<m_iMinWordLen )
				FlushAccum ();

			if ( m_iAccum==0 )
			{
				assert ( iCode<0 );
				AccumCodePoint ( -iCode ); // handle special as a standalone token
			} else
			{
				if ( iCode<0 )
					m_pCur = pCur; // we need to flush current accum and then redo special char again
			}
			FlushAccum ();
			return m_sAccum;
		}

		// just accumulate
		AccumCodePoint ( iCode );
	}
}


int CSphTokenizer_UTF8::GetCodePoint ()
{
	if ( m_pCur>=m_pBufferMax )
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
			m_pCur++;
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


void CSphTokenizer_UTF8::FlushAccum ()
{
	assert ( m_pAccum-m_sAccum < (int)sizeof(m_sAccum) );
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}


ISphTokenizer * CSphTokenizer_UTF8::Clone () const
{
	CSphTokenizer_UTF8 * pClone = new CSphTokenizer_UTF8 ();
	pClone->m_tLC = m_tLC;
	return pClone;
}

/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////

CSphFilter::CSphFilter ()
	: m_sAttrName	( "" )
	, m_iAttrIndex	( -1 )
	, m_uMinValue	( 0 )
	, m_uMaxValue	( UINT_MAX )
	, m_iValues		( 0 )
	, m_pValues		( NULL )
{}


CSphFilter::~CSphFilter ()
{
	SafeDeleteArray ( m_pValues );
}


const CSphFilter & CSphFilter::operator = ( const CSphFilter & rhs )
{
	m_sAttrName		= rhs.m_sAttrName;
	m_iAttrIndex	= rhs.m_iAttrIndex;
	m_uMinValue		= rhs.m_uMinValue;
	m_uMaxValue		= rhs.m_uMaxValue;
	m_iValues		= rhs.m_iValues;
	SafeDeleteArray ( m_pValues );
	if ( m_iValues )
	{
		m_pValues = new DWORD [ m_iValues ];
		memcpy ( m_pValues, rhs.m_pValues, sizeof(DWORD)*m_iValues );
	}
	return *this;
}


static int cmpDword ( const void * a, const void * b )
{
	return *((DWORD*)a) - *((DWORD*)b);
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
	: m_pWeights	( NULL )
	, m_iWeights	( 0 )
	, m_eMode		( SPH_MATCH_ALL )
	, m_eSort		( SPH_SORT_RELEVANCE )
	, m_pTokenizer	( NULL )
	, m_iMaxMatches	( 1000 )
	, m_iMinID		( 0 )
	, m_iMaxID		( UINT_MAX )
{}

/////////////////////////////////////////////////////////////////////////////
// SCHEMA
/////////////////////////////////////////////////////////////////////////////

bool CSphSchema::IsEqual ( const CSphSchema & rhs, CSphString & sError ) const
{
	char sTmp [ 1024 ];

	if ( rhs.m_dFields.GetLength()!=m_dFields.GetLength() )
	{
		snprintf ( sTmp, sizeof(sTmp), "fulltext fields count mismatch: %d in schema '%s', %d in schema '%s'",
			m_dFields.GetLength(), m_sName.cstr(),
			rhs.m_dFields.GetLength(), rhs.m_sName.cstr() );
		sError = sTmp;
		return false;
	}

	if ( rhs.m_dAttrs.GetLength()!=m_dAttrs.GetLength() )
	{
		snprintf ( sTmp, sizeof(sTmp), "attributes count mismatch: %d in schema '%s', %d in schema '%s'",
			m_dAttrs.GetLength(), m_sName.cstr(),
			rhs.m_dAttrs.GetLength(), rhs.m_sName.cstr() );
		sError = sTmp;
		return false;
	}

	ARRAY_FOREACH ( i, rhs.m_dFields )
		if ( rhs.m_dFields[i].m_sName!=m_dFields[i].m_sName )
	{
		snprintf ( sTmp, sizeof(sTmp), "field %d/%d name mismatch: '%s' in schema '%s', '%s' in schema '%s'",
			i, m_dFields.GetLength(),
			m_dFields[i].m_sName.cstr(), m_sName.cstr(),
			rhs.m_dFields[i].m_sName.cstr(), rhs.m_sName.cstr() );
		sError = sTmp;
		return false;
	}

	ARRAY_FOREACH ( i, rhs.m_dAttrs )
		if ( rhs.m_dAttrs[i].m_sName!=m_dAttrs[i].m_sName )
	{
		snprintf ( sTmp, sizeof(sTmp), "attribute %d/%d name mismatch: '%s' in schema '%s', '%s' in schema '%s'",
			i, m_dAttrs.GetLength(),
			m_dAttrs[i].m_sName.cstr(), m_sName.cstr(),
			rhs.m_dAttrs[i].m_sName.cstr(), rhs.m_sName.cstr() );
		sError = sTmp;
		return false;
	}

	return true;
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

///////////////////////////////////////////////////////////////////////////////
// BIT-ENCODED FILE OUTPUT
///////////////////////////////////////////////////////////////////////////////

CSphWriter_VLN::CSphWriter_VLN ( char * sName )
{
	assert ( sName );
	m_sName = sName;

	m_pPool = m_dPool;
	m_iFD = -1;
	m_iPos = 0;
	m_bError = false;
}


int CSphWriter_VLN::OpenFile ()
{
	if ( m_iFD>=0 )
		return m_iFD;
	m_iFD = ::open ( m_sName.cstr(), O_CREAT | O_RDWR | O_TRUNC | SPH_BINARY, 0644 );
	m_iPoolUsed = 0;
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


void CSphWriter_VLN::PutByte ( int data )
{
	*m_pPool++ = BYTE( data & 0xff );
	m_iPoolUsed++;
	if ( m_iPoolUsed==SPH_CACHE_WRITE )
		Flush ();
	m_iPos++;
}


void CSphWriter_VLN::PutBytes ( const void * pData, int iSize )
{
	const BYTE * b = (BYTE*) pData;
	while ( iSize-->0 )
		PutByte ( *b++ );
}


void CSphWriter_VLN::PutRawBytes ( const void * pData, int iSize )
{
	if ( m_iPoolUsed+iSize>SPH_CACHE_WRITE )
		Flush ();
	assert ( m_iPoolUsed+iSize<=SPH_CACHE_WRITE );

	memcpy ( m_pPool, pData, iSize );
	m_pPool += iSize;
	m_iPoolUsed += iSize;
	m_iPos += iSize;
}


void CSphWriter_VLN::ZipInt ( DWORD uValue )
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


void CSphWriter_VLN::ZipOffset ( SphOffset_t uValue )
{
	int iBytes = 1;

	SphOffset_t u = ( uValue>>7 );
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


void CSphWriter_VLN::ZipOffsets ( CSphVector<SphOffset_t> * pData )
{
	assert ( pData );

	SphOffset_t * pValue = &((*pData)[0]);
	int n = pData->GetLength ();

	while ( n-->0 )
	{
		SphOffset_t uValue = *pValue++;

		int iBytes = 1;

		SphOffset_t u = ( uValue>>7 );
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


void CSphWriter_VLN::Flush ()
{
	PROFILE ( write_hits );

	if ( sphWrite ( m_iFD, m_dPool, m_iPoolUsed, m_sName.cstr() )!=m_iPoolUsed )
		m_bError = true;

	m_iPoolUsed = 0;
	m_pPool = m_dPool;
}


void CSphWriter_VLN::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );

	Flush ();
	sphSeek ( m_iFD, iPos, SEEK_SET );
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
	, m_iBuffPos ( 0 )
	, m_iBuffUsed ( 0 )
	, m_iBufSize ( 262144 ) // FIXME!
	, m_pBuff ( NULL )
{
}


CSphReader_VLN::~CSphReader_VLN ()
{
	SafeDeleteArray ( m_pBuff );
}


void CSphReader_VLN::SetFile ( int iFD )
{
	m_iFD = iFD;
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
}


void CSphReader_VLN::Reset ()
{
	SetFile ( -1 );
}


void CSphReader_VLN::SeekTo ( SphOffset_t iPos )
{
	assert ( iPos>=0 );
	if ( iPos>=m_iPos && iPos<m_iPos+m_iBuffUsed )
	{
		m_iBuffPos = (int)( iPos-m_iPos ); // reposition to proper byte
		assert ( m_iBuffPos<m_iBuffUsed );
	} else
	{
		m_iPos = iPos;
		m_iBuffUsed = 0;
	}
}


void CSphReader_VLN::UpdateCache ()
{
	PROFILE ( read_hits );
	assert ( m_iFD>=0 );

	// alloc buf on first actual read
	if ( !m_pBuff )
		m_pBuff = new BYTE [ m_iBufSize ];

	// stream position could be changed externally
	// so let's just hope that the OS optimizes redundant seeks
	SphOffset_t iNewPos = m_iPos + m_iBuffUsed;
	sphSeek ( m_iFD, iNewPos, SEEK_SET );
	
	m_iBuffPos = 0;
	m_iBuffUsed = ::read ( m_iFD, m_pBuff, m_iBufSize );
	m_iPos = iNewPos;
}


int CSphReader_VLN::GetByte ()
{
	if ( m_iBuffPos>=m_iBuffUsed )
		UpdateCache ();
	assert ( m_iBuffPos<m_iBuffUsed );

	return m_pBuff [ m_iBuffPos++ ];
}


void CSphReader_VLN::GetRawBytes ( void * pData, int iSize )
{
	if ( m_iBuffPos+iSize>m_iBuffUsed )
		UpdateCache ();
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


DWORD sphUnzipInt ( BYTE * & pBuf )
{
	register DWORD b, v = 0;
	do
	{
		b = *pBuf++;
		v = ( v<<7 ) + ( b&0x7f );
	} while ( b&0x80 );
	return v;
}


DWORD CSphReader_VLN::UnzipInt ()
{
	register DWORD b, v = 0;
	do
	{
		b = GetByte ();
		v = ( v<<7 ) + ( b&0x7f );
	} while ( b&0x80 );
	return v;
}


SphOffset_t sphUnzipOffset ( BYTE * & pBuf )
{
	register DWORD b = 0;
	register SphOffset_t v = 0;
	do
	{
		b = *pBuf++;
		v = ( v<<7 ) + ( b&0x7f );
	} while ( b&0x80 );
	return v;
}


SphOffset_t CSphReader_VLN::UnzipOffset ()
{
	register DWORD b;
	register SphOffset_t v = 0;

	do
	{
		b = GetByte ();
		v = ( v<<7 ) + ( SphOffset_t(b&0x7f) );
	} while ( b&0x80 );
	return v;
}

const CSphReader_VLN & CSphReader_VLN::operator = ( const CSphReader_VLN & rhs )
{
	SetFile ( rhs.m_iFD );
	SeekTo ( rhs.m_iPos + rhs.m_iBuffPos );
	return *this;
}


DWORD CSphReader_VLN::GetDword ()
{
	DWORD uRes;
	GetRawBytes ( &uRes, sizeof(DWORD) );
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
}


CSphQueryResult::~CSphQueryResult ()
{
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


void CSphBin::Init ( int iFD, SphOffset_t * pSharedOffset, const char * sPhase, int iMemoryLimit, int iBlocks )
{
	assert ( !m_dBuffer );

	int iBinSize = ( ( iMemoryLimit/iBlocks + 2048 ) >> 12 ) << 12; // round to 4k
	if ( iBinSize<CSphBin::MIN_SIZE )
	{
		iBinSize = CSphBin::MIN_SIZE;
		fprintf ( stdout, "WARNING: %s: mem_limit=%d kb too low, increasing to %d kb.\n",
			sPhase, iMemoryLimit/1024, iBinSize*iBlocks/1024 );
	}
	if ( iBinSize<CSphBin::WARN_SIZE )
	{
		fprintf ( stdout, "WARNING: %s: merge_block_size=%d kb too low, increasing mem_limit may improve performance.\n",
			sPhase, iBinSize/1024 );
	}

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


DWORD CSphBin::ReadVLB ()
{
	DWORD v = 0, o = 0;
	int t;
	do
	{
		if ( ( t = ReadByte() )<0 )
			return 0xffffffff; // FIXME! replace with some other errcode
		v += ( (t & 0x7f ) << o );
		o += 7;
	} while ( t & 0x80 );
	return v;
}


int CSphBin::ReadHit ( CSphWordHit * e, int iAttrs, DWORD * pAttrs )
{
	DWORD r;

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
		r = ReadVLB();
		if ( r==0xffffffffUL )
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

					for ( int i=0; i<iAttrs; i++, pAttrs++ )
					{
						*pAttrs = ReadVLB ();
						if ( *pAttrs==0xffffffffUL )
							return 0; // read unexpected EOB
					}
					break;

				case BIN_POS: 
					tHit.m_iWordPos += r;
					*e = tHit;
					return 1;

				default:
					sphDie ( "FATAL: INTERNAL ERROR: unknown bin state (state=%d).\n", m_eState );
			}
		} else
		{
			switch ( m_eState )
			{
				case BIN_POS:	m_eState = BIN_DOC; break;
				case BIN_DOC:	m_eState = BIN_WORD; break;
				case BIN_WORD:	m_iDone = 1; e->m_iWordID = 0; return 1;
				default:		sphDie ( "FATAL: INTERNAL ERROR: unknown bin state (state=%d).\n", m_eState );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphIndex * sphCreateIndexPhrase ( const char * sFilename )
{
	return new CSphIndex_VLN ( sFilename );
}


CSphIndex_VLN::CSphIndex_VLN ( const char * sFilename )
	: CSphIndex ( sFilename )
{
	m_sFilename = sFilename;

	m_fdWordlist = NULL;
	fdData = NULL;

	m_pWriteBuffer = NULL;

	m_tLastHit.m_iDocID = 0;
	m_tLastHit.m_iWordID = 0;
	m_tLastHit.m_iWordPos = 0;
	m_iLastHitlistPos = 0;
	m_uFields = 0;
	m_iLastDoclistPos = 0;
	m_iLastWordDocs = 0;
	m_iLastWordHits = 0;
	m_iWordlistEntries = 0;

	m_eDocinfo = SPH_DOCINFO_NONE;
	m_pDocinfo = NULL;
	m_pDocinfoHash = NULL;
	m_iDocinfoIdShift = 0;
	m_pWordlist = NULL;
	m_bPreloaded = false;
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	SafeDeleteArray ( m_pDocinfo );
	SafeDeleteArray ( m_pWordlist );
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
	PROFILE ( sort_hits );

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
		DWORD iMedian = pBuf [ ((a+b)/2) * iStride ];
		while ( a<b )
		{
			while ( i<=j )
			{
				while ( pBuf[i*iStride] < iMedian ) i++;
				while ( iMedian < pBuf[j*iStride] ) j--;
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


int CSphIndex_VLN::OpenFile ( char * ext, int mode )
{
	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s.%s", m_sFilename.cstr(), ext );

	int iFD = ::open ( sBuf, mode | SPH_BINARY, 0644 );
	if ( iFD<0 )
		fprintf ( stdout, "ERROR: failed to open '%s': %s.\n", sBuf, strerror(errno) );

	return iFD;
}


int CSphIndex_VLN::cidxCreate ()
{
	char sBuf [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sBuf, sizeof(sBuf), "%s.spi", m_sFilename.cstr() );
	m_fdWordlist = new CSphWriter_VLN ( sBuf );
	if ( m_fdWordlist->OpenFile()<0 )
	{
		SafeDelete ( m_fdWordlist );
		return 0;
	}

	snprintf ( sBuf, sizeof(sBuf), "%s.spd", m_sFilename.cstr() );
	fdData = new CSphWriter_VLN ( sBuf );
	if ( fdData->OpenFile()<0 )
	{
		SafeDelete ( m_fdWordlist );
		SafeDelete ( fdData );
		return 0;
	}

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	fdData->PutBytes ( &bDummy, 1 );
	m_fdWordlist->PutBytes ( &bDummy, 1 );

	return 1;
}


void CSphIndex_VLN::cidxHit ( CSphWordHit * hit, DWORD * pAttrs )
{
	assert (
		( hit->m_iWordID && hit->m_iWordPos && hit->m_iDocID ) || // it's either ok hit
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
			// flush matched fields mask
			m_dDoclist.Add ( m_uFields );
			m_uFields = 0;

			// flush wordlist entry
			assert ( fdData->m_iPos > m_iLastDoclistPos );
			assert ( m_iLastWordDocs );
			assert ( m_iLastWordHits );

			m_fdWordlist->ZipOffset ( fdData->m_iPos - m_iLastDoclistPos );
			m_fdWordlist->ZipInt ( m_iLastWordDocs );
			m_fdWordlist->ZipInt ( m_iLastWordHits );

			m_iLastDoclistPos = fdData->m_iPos;
			m_iLastWordDocs = 0;
			m_iLastWordHits = 0;

			m_iWordlistEntries++;

			// flush doclist
			fdData->ZipOffsets ( &m_dDoclist );
			fdData->ZipInt ( 0 );
			m_dDoclist.Reset ();

			m_tLastHit.m_iDocID = 0;
			m_iLastHitlistPos = 0;
		}

		// flush wordlist, if this is the end
		if ( !hit->m_iWordPos )
		{
			m_fdWordlist->ZipInt ( 0 ); // indicate checkpoint
			return;
		}

		// insert wordlist checkpoint (ie. restart delta coding) once per WORDLIST_CHECKPOINT entries
		if ( m_iWordlistEntries==WORDLIST_CHECKPOINT )
		{
			m_fdWordlist->ZipInt ( 0 ); // indicate checkpoint
			m_tLastHit.m_iWordID = 0;
			m_iLastDoclistPos = 0;
			m_iLastWordDocs = 0;
			m_iLastWordHits = 0;

			m_iWordlistEntries = 0;
		}

		// begin new wordlist entry
		if ( m_iWordlistEntries==0 )
		{
			assert ( m_fdWordlist->m_iPos<=UINT_MAX );

			CSphWordlistCheckpoint tCheckpoint;
			tCheckpoint.m_iWordID = hit->m_iWordID;
			tCheckpoint.m_iWordlistOffset = (DWORD) m_fdWordlist->m_iPos;

			m_dWordlistCheckpoints.Add ( tCheckpoint );
		}

		assert ( hit->m_iWordID > m_tLastHit.m_iWordID );
		m_fdWordlist->ZipInt ( hit->m_iWordID - m_tLastHit.m_iWordID );
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

		// close prev doc, if any
		if ( m_dDoclist.GetLength() )
		{
			// flush matched fields mask
			m_dDoclist.Add ( m_uFields );
			m_uFields = 0;
		}

		// add new doclist entry for new doc id
		assert ( hit->m_iDocID > m_tLastHit.m_iDocID );
		assert ( fdData->m_iPos > m_iLastHitlistPos );

		m_dDoclist.Add ( hit->m_iDocID - m_tLastHit.m_iDocID );
		if ( pAttrs )
			for ( int i=0; i<m_tSchema.m_dAttrs.GetLength(); i++ )
				m_dDoclist.Add ( pAttrs[i] - m_tMin.m_pAttrs[i] );
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

	// update matched fields mask
	m_uFields |= ( 1UL << (hit->m_iWordPos>>24) );
}


void CSphIndex_VLN::WriteSchemaColumn ( CSphWriter_VLN * fdInfo, const CSphColumnInfo & tCol )
{
	int iLen = strlen ( tCol.m_sName.cstr() );
	fdInfo->PutDword ( iLen );
	fdInfo->PutRawBytes ( tCol.m_sName.cstr(), iLen );
	fdInfo->PutDword ( tCol.m_eAttrType );
}


void CSphIndex_VLN::ReadSchemaColumn ( CSphReader_VLN & rdInfo, CSphColumnInfo & tCol )
{
	tCol.m_sName = rdInfo.GetString ();
	tCol.m_eAttrType = (ESphAttrType) rdInfo.GetDword ();
}


bool CSphIndex_VLN::cidxDone()
{
	// flush wordlist checkpoints
	SphOffset_t iCheckpointsPos = m_fdWordlist->m_iPos;
	if ( m_dWordlistCheckpoints.GetLength() )
		m_fdWordlist->PutRawBytes ( &m_dWordlistCheckpoints[0], m_dWordlistCheckpoints.GetLength()*sizeof(CSphWordlistCheckpoint) );

	/////////////////
	// create header
	/////////////////

	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s.sph", m_sFilename.cstr() );

	CSphWriter_VLN * fdInfo = new CSphWriter_VLN ( sBuf );
	if ( fdInfo->OpenFile()<0 )
	{
		SafeDelete ( fdInfo );
		return false;
	}

	// docinfo
	fdInfo->PutDword ( m_eDocinfo );

	// schema
	fdInfo->PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		WriteSchemaColumn ( fdInfo, m_tSchema.m_dFields[i] );
	fdInfo->PutDword ( m_tSchema.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dAttrs )
		WriteSchemaColumn ( fdInfo, m_tSchema.m_dAttrs[i] );

	// min doc
	fdInfo->PutDword ( m_tMin.m_iDocID );
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		fdInfo->PutRawBytes ( m_tMin.m_pAttrs, m_tMin.m_iAttrs*sizeof(DWORD) );

	// wordlist checkpoints
	fdInfo->PutRawBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	fdInfo->PutDword ( m_dWordlistCheckpoints.GetLength() );

	////////////////////////
	// close all data files
	////////////////////////

	fdInfo->CloseFile ();
	fdData->CloseFile ();
	m_fdWordlist->CloseFile ();

	if ( m_fdWordlist->IsError() || fdData->IsError() )
		fprintf ( stdout, "ERROR: write() failed, out of disk space?\n" );

	SafeDelete ( m_fdWordlist );
	SafeDelete ( fdData );
	SafeDelete ( fdInfo );
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

	DWORD iStartID = 0;
	int dHash [ HSIZE+1 ];
	int iShift = 0;

	if ( pDocinfo )
	{
		iStartID = *pDocinfo;
		int iBits = iLog2 ( pDocinfo [ (iDocinfos-1)*iStride ] - iStartID );
		iShift = ( iBits<HBITS ) ? 0 : ( iBits-HBITS );

		#ifndef NDEBUG
		for ( int i=0; i<=HSIZE; i++ )
			dHash[i] = -1;
		#endif

		dHash[0] = 0;
		int iHashed = 0;
		for ( int i=0; i<iDocinfos; i++ )
		{
			int iHash = ( pDocinfo [ i*iStride ] - iStartID ) >> iShift;
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
	DWORD d1, d2, d3, l1=0, l2=0, l3=0;

	pBuf = m_pWriteBuffer;
	maxP = m_pWriteBuffer + WRITE_BUFFER_SIZE - 128;

	DWORD iAttrID = 0; // current doc id
	DWORD * pAttrs = NULL; // current doc attrs

	while ( iHits-- )
	{
		// find attributes by id
		if ( pDocinfo && iAttrID!=pHit->m_iDocID )
		{
			int iHash = ( pHit->m_iDocID - iStartID ) >> iShift;
			assert ( iHash>=0 && iHash<HSIZE );

			int iStart = dHash[iHash];
			int iEnd = dHash[iHash+1];

			if ( pHit->m_iDocID==pDocinfo [ iStart*iStride ] )
			{
				pAttrs = pDocinfo + iStart*iStride + 1;

			} else if ( pHit->m_iDocID==pDocinfo [ iEnd*iStride ])
			{
				pAttrs = pDocinfo + iEnd*iStride + 1;

			} else
			{
				pAttrs = NULL;
				while ( iEnd-iStart>1 )
				{
					// check if nothing found
					if (
						pHit->m_iDocID < pDocinfo [ iStart*iStride ] ||
						pHit->m_iDocID > pDocinfo [ iEnd*iStride ] )
							break;
					assert ( pHit->m_iDocID > pDocinfo [ iStart*iStride ] );
					assert ( pHit->m_iDocID < pDocinfo [ iEnd*iStride ] );

					int iMid = iStart + (iEnd-iStart)/2;
					if ( pHit->m_iDocID==pDocinfo [ iMid*iStride ] )
					{
						pAttrs = pDocinfo + iMid*iStride + 1;
						break;
					}
					if ( pHit->m_iDocID<pDocinfo [ iMid*iStride ] )
						iEnd = iMid;
					else
						iStart = iMid;
				}
			}

			assert ( pAttrs );
			assert ( pAttrs[-1]==pHit->m_iDocID );
			iAttrID = pHit->m_iDocID;
		}

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
		if ( d2 ) pBuf += encodeVLB ( pBuf, d2 ); // encode doc id (whole or delta)

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
			if ( sphWrite ( fd, m_pWriteBuffer, w, "raw_hits" )!=w )
				return -1;
			n += w;
			pBuf = m_pWriteBuffer;
		}
	}
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	w = (int)(pBuf - m_pWriteBuffer);
	if ( sphWrite ( fd, m_pWriteBuffer, w, "raw_hits" )!=w )
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

	static inline bool IsLess ( const int a, const int b, const CSphMatchComparatorState & )
	{
		return m_pStorage [ a*m_iStride ] < m_pStorage [ b*m_iStride ];
	};
};
DWORD *		CmpQueuedDocinfo_fn::m_pStorage		= NULL;
int			CmpQueuedDocinfo_fn::m_iStride		= 1;


int CSphIndex_VLN::AdjustMemoryLimit ( int iMemoryLimit )
{
	const int MIN_MEM_LIMIT = sizeof(CSphWordHit)*1048576 + sizeof(DWORD)*( 1+m_tSchema.m_dAttrs.GetLength() )*65536;

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
		fprintf ( stdout, "WARNING: collect_hits: mem_limit=%d kb too low, increasing to %d kb.\n",
			iOldLimit/1024, iMemoryLimit/1024 );
	}

	return iMemoryLimit;
}


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
		if ( !pSource->UpdateSchema ( &m_tSchema ) )
			return 0;
	}

	if ( m_tSchema.m_dAttrs.GetLength()==0 )
		m_eDocinfo = SPH_DOCINFO_NONE;

	////////////////////////////////////////////////
	// collect and partially sort hits and docinfos
	////////////////////////////////////////////////

	// adjust memory requirements
	// ensure there's enough memory to hold 1M hits and 64K docinfos
	// alloc 1/16 of memory (but not less than 64K entries) for docinfos
	iMemoryLimit = AdjustMemoryLimit ( iMemoryLimit );

	const int iDocinfoStride = 1 + m_tSchema.m_dAttrs.GetLength();
	int iDocinfoMax = Max ( 65536, iMemoryLimit/16/iDocinfoStride/sizeof(DWORD) );
	int iHitsMax = ( iMemoryLimit - iDocinfoMax*iDocinfoStride*sizeof(DWORD) ) / sizeof(CSphWordHit);

	// allocate raw hits block
	CSphAutoArray<CSphWordHit> dHits ( iHitsMax );
	CSphWordHit * pHits = dHits;
	CSphWordHit * pHitsMax = dHits + iHitsMax;

	// allocate docinfos buffer
	CSphAutoArray<DWORD> dDocinfos ( iDocinfoMax*iDocinfoStride );
	DWORD * pDocinfo = dDocinfos;
	const DWORD * pDocinfoMax = dDocinfos + iDocinfoMax*iDocinfoStride;

	// create temp hits file
	CSphAutofile fdTmpHits ( OpenFile ( "tmp1", O_CREAT | O_RDWR | O_TRUNC ) );
	if ( fdTmpHits.GetFD()<0 )
		return 0;

	// create temp docinfos file
	CSphAutofile fdTmpDocinfos ( OpenFile ( "tmp2", O_CREAT | O_RDWR | O_TRUNC ) );
	if ( fdTmpDocinfos.GetFD()<0 )
		return 0;

	// setup accumulating docinfo IDs range
	m_tMin.Reset ();
	m_tMin.m_iAttrs = m_tSchema.m_dAttrs.GetLength();
	m_tMin.m_pAttrs = new DWORD [ m_tSchema.m_dAttrs.GetLength() ];
	for ( int i=0; i<m_tSchema.m_dAttrs.GetLength(); i++ )
		m_tMin.m_pAttrs[i] = UINT_MAX;
	m_tMin.m_iDocID = UINT_MAX;

	// build raw log
	PROFILE_BEGIN ( collect_hits );

	CSphSourceStats tComplete; // total stats for all the completed sources

	CSphIndexProgress tProgress; // current progress
	tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT;
	tProgress.m_iHitsTotal = 0;

	CSphVector<int> dHitBlocks;
	int iDocinfoBlocks = 0;

	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		// FIXME! sql_query for sources 2+ might timeout by this time
		DWORD iDocID;
		while ( ( iDocID = pSource->Next() )!=0 )
		{
			assert ( iDocID==pSource->m_tDocInfo.m_iDocID );

			// show progress bar
			if ( m_pProgress
				&& ( ( pSource->GetStats().m_iTotalDocuments % 1000 )==0 ) )
			{
				tProgress.m_iDocuments = tComplete.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				tProgress.m_iBytes = tComplete.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				m_pProgress ( &tProgress );
			}

			// make a shortcut for hit count
			int iDocHits = pSource->m_dHits.GetLength ();
			if ( iDocHits<=0 )
				continue;

			// update min docinfo
			assert ( pSource->m_tDocInfo.m_iDocID );
			m_tMin.m_iDocID = Min ( m_tMin.m_iDocID, pSource->m_tDocInfo.m_iDocID );
			if ( m_eDocinfo==SPH_DOCINFO_INLINE )
				for ( int i=0; i<m_tSchema.m_dAttrs.GetLength(); i++ )
			{
				assert ( pSource->m_tDocInfo.m_pAttrs[i] );
				m_tMin.m_pAttrs[i] = Min ( m_tMin.m_pAttrs[i], pSource->m_tDocInfo.m_pAttrs[i] );
			}

			// store docinfo
			if ( pDocinfo==dDocinfos || *pDocinfo!=pSource->m_tDocInfo.m_iDocID )
			{
				// store next entry
				*pDocinfo = pSource->m_tDocInfo.m_iDocID;
				memcpy ( pDocinfo+1, pSource->m_tDocInfo.m_pAttrs, sizeof(DWORD)*m_tSchema.m_dAttrs.GetLength() );
				pDocinfo += iDocinfoStride;

				// if not inlining, flush buffer if it's full
				// (if inlining, it will flushed later, along with the hits)
				if ( m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>=pDocinfoMax )
				{
					assert ( pDocinfo==pDocinfoMax );
					int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);

					sphSortDocinfos ( dDocinfos, iDocinfoMax, iDocinfoStride );
					if ( sphWrite ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos" )!=iLen )
						return 0;

					pDocinfo = dDocinfos;
					iDocinfoBlocks++;
				}
			}

			// store hits
			CSphWordHit * pDocHits = &pSource->m_dHits[0];

#if PARANOID
			for ( int i=0; i<DocHits; i++ )
			{
				assert ( pDocHits[i]->m_iDocID==pSource->m_tDocInfo.m_iDocID );
				assert ( pDocHits[i]->m_iWordID );
				assert ( pDocHits[i]->m_iWordPos );
			}
#endif

			// while there's not enough space in buffers
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
				sphSortHits ( dHits, iHits );
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
						*pDocinfo = pSource->m_tDocInfo.m_iDocID;
						memcpy ( pDocinfo+1, pSource->m_tDocInfo.m_pAttrs, sizeof(DWORD)*m_tSchema.m_dAttrs.GetLength() );
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
				tProgress.m_iHitsTotal += iHits;
				if ( m_pProgress )
				{
					tProgress.m_iDocuments = tComplete.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
					tProgress.m_iBytes = tComplete.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
					m_pProgress ( &tProgress );
				}
			}
		}

		// this source is over, update overall stats
		tComplete.m_iTotalDocuments += pSource->GetStats().m_iTotalDocuments;
		tComplete.m_iTotalBytes += pSource->GetStats().m_iTotalBytes;
	}

	// flush last docinfo block
	int iDocinfoLastBlockSize = 0;
	if ( m_eDocinfo==SPH_DOCINFO_EXTERN && pDocinfo>dDocinfos )
	{
		iDocinfoLastBlockSize = ( pDocinfo - dDocinfos ) / iDocinfoStride;
		assert ( pDocinfo == dDocinfos + iDocinfoLastBlockSize*iDocinfoStride );

		int iLen = iDocinfoLastBlockSize*iDocinfoStride*sizeof(DWORD);
		sphSortDocinfos ( dDocinfos, iDocinfoLastBlockSize, iDocinfoStride );
		if ( sphWrite ( fdTmpDocinfos.GetFD(), dDocinfos, iLen, "raw_docinfos" )!=iLen )
			return 0;

		iDocinfoBlocks++;
	}

	// flush last hit block
	if ( pHits>dHits )
	{
		int iHits = pHits - dHits;
		sphSortHits ( dHits, iHits );
		tProgress.m_iHitsTotal += iHits;

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

	dHits.Reset ();

	if ( m_pProgress )
	{
		tProgress.m_iDocuments = tComplete.m_iTotalDocuments;
		tProgress.m_iBytes = tComplete.m_iTotalBytes;
		m_pProgress ( &tProgress );

		tProgress.m_ePhase = CSphIndexProgress::PHASE_COLLECT_END;
		m_pProgress ( &tProgress );
	}

	PROFILE_END ( collect_hits );

	/////////////////
	// sort docinfos
	/////////////////

	if ( m_eDocinfo==SPH_DOCINFO_EXTERN && dHitBlocks.GetLength() )
	{
		// initialize writer
		CSphAutofile fdDocinfo ( OpenFile ( "spa", O_CREAT | O_RDWR | O_TRUNC ) );
		if ( fdDocinfo.GetFD()<0 )
			return 0;

		// initialize readers
		assert ( dBins.GetLength()==0 );
		dBins.Grow ( iDocinfoBlocks );

		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			dBins.Add ( new CSphBin() );
			dBins[i]->m_iFileLeft = ( ( i==iDocinfoBlocks-1 ) ? iDocinfoLastBlockSize : iDocinfoMax )*iDocinfoStride*sizeof(DWORD);
			dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
		}

		iSharedOffset = -1;
		ARRAY_FOREACH ( i, dBins )
			dBins[i]->Init ( fdTmpDocinfos.GetFD(), &iSharedOffset, "sort_docinfos", iMemoryLimit, iDocinfoBlocks );

		// docinfo queue
		CSphAutoArray<DWORD> dDocinfoQueue ( iDocinfoBlocks*iDocinfoStride );
		CSphQueue < int,CmpQueuedDocinfo_fn > qDocinfo ( iDocinfoBlocks, false );

		CmpQueuedDocinfo_fn::m_pStorage = dDocinfoQueue;
		CmpQueuedDocinfo_fn::m_iStride = iDocinfoStride;

		pDocinfo = dDocinfoQueue;
		for ( int i=0; i<iDocinfoBlocks; i++ )
		{
			if ( dBins[i]->ReadBytes  ( pDocinfo, iDocinfoStride*sizeof(DWORD) )<=0 )
			{
				fprintf ( stdout, "ERROR: sort_docinfos: warmup failed; I/O error?\n" );
				return 0;
			}
			pDocinfo += iDocinfoStride;
			qDocinfo.Push ( i );
		}

		// while the queue has data for us
		pDocinfo = dDocinfos;
		while ( qDocinfo.GetLength() )
		{
			// obtain bin index and next entry
			int iBin = qDocinfo.Root();
			DWORD * pEntry = dDocinfoQueue + iBin*iDocinfoStride;

			// emit it
			memcpy ( pDocinfo, pEntry, iDocinfoStride*sizeof(DWORD) );
			pDocinfo += iDocinfoStride;
			if ( pDocinfo>=pDocinfoMax )
			{
				int iLen = iDocinfoMax*iDocinfoStride*sizeof(DWORD);
				if ( sphWrite ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo" )!=iLen )
					return 0;
				pDocinfo = dDocinfos;
			}

			// pop its index, update it, push its index again
			qDocinfo.Pop ();
			int iRes = dBins[iBin]->ReadBytes ( pEntry, iDocinfoStride*sizeof(DWORD) );
			if ( iRes<0 )
			{
				fprintf ( stdout, "ERROR: sort_docinfo: failed to read entry.\n" );
				return 0;
			}
			if ( iRes>0 )
				qDocinfo.Push ( iBin );
		}
		if ( pDocinfo>dDocinfos )
		{
			assert ( 0 == ( pDocinfo-dDocinfos ) % iDocinfoStride );
			int iLen = ( pDocinfo - dDocinfos )*sizeof(DWORD);
			if ( sphWrite ( fdDocinfo.GetFD(), dDocinfos, iLen, "sort_docinfo" )!=iLen )
				return 0;
		}

		// clean up readers
		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );
		dBins.Reset ();
	}

	dDocinfos.Reset ();
	pDocinfo = NULL;

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	PROFILE_BEGIN ( invert_hits );

	// calc bin positions from their lengths
	assert ( dBins.GetLength()==0 );
	dBins.Grow ( dHitBlocks.GetLength() );

	ARRAY_FOREACH ( i, dHitBlocks )
	{
		dBins.Add ( new CSphBin() );
		dBins[i]->m_iFileLeft = dHitBlocks[i];
		dBins[i]->m_iFilePos = ( i==0 ) ? 0 : dBins[i-1]->m_iFilePos + dBins[i-1]->m_iFileLeft;
	}

	// if there were no hits, create zero-length index files
	int iRawBlocks = dBins.GetLength();
	if ( iRawBlocks==0 )
	{
		// FIXME! should also unlink old .spa here
		m_eDocinfo = SPH_DOCINFO_INLINE;
	}

	// initialize readers
	fdTmpHits.Close ();
	fdTmpHits.SetFD ( OpenFile ( "tmp1", O_RDONLY ) );
	iSharedOffset = -1;
	ARRAY_FOREACH ( i, dBins )
		dBins[i]->Init ( fdTmpHits.GetFD(), &iSharedOffset, "sort_hits", iMemoryLimit, iRawBlocks );

	// create new compressed index
	if ( !cidxCreate() )
		return 0;

	// adjust min IDs, and fill header
	assert ( m_tMin.m_iDocID>0 );
	m_tMin.m_iDocID--;
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		for ( int i=0; i<m_tSchema.m_dAttrs.GetLength(); i++ )
	{
		assert ( m_tMin.m_pAttrs[i]>0 );
		m_tMin.m_pAttrs[i]--;
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
		int iAttrs = ( m_eDocinfo==SPH_DOCINFO_INLINE ) ? m_tSchema.m_dAttrs.GetLength() : 0;
		CSphAutoArray<DWORD> dInlineAttrs ( iRawBlocks*iAttrs );

		int * bActive = new int [ iRawBlocks ];
		for ( int i=0; i<iRawBlocks; i++ )
		{
			if ( !dBins[i]->ReadHit ( &tHit, iAttrs, dInlineAttrs+i*iAttrs ) )
			{
				fprintf ( stdout, "ERROR: sort_hits: warmup failed; I/O error?\n" );
				return 0;
			}
			bActive[i] = tHit.m_iWordID;
			if ( bActive[i] )
				tQueue.Push ( tHit, i );
		}

		// init progress meter
		tProgress.m_ePhase = CSphIndexProgress::PHASE_SORT;
		tProgress.m_iHits = 0;

		// while the queue has data for us
		// FIXME! analyze binsRead return code
		int iHitsSorted = 0;
		while ( tQueue.m_iUsed )
		{
			int iBin = tQueue.m_pData->m_iBin;

			// pack and emit queue root
			tQueue.m_pData->m_iDocID -= m_tMin.m_iDocID;
			cidxHit ( tQueue.m_pData, iAttrs ? dInlineAttrs+iBin*iAttrs : NULL );
			if ( m_fdWordlist->IsError() || fdData->IsError() )
			{
				fprintf ( stdout, "ERROR: write() failed, out of disk space?\n" );
				return 0;
			}

			// pop queue root and push next hit from popped bin

			tQueue.Pop ();
			if ( bActive[iBin] )
			{
				dBins[iBin]->ReadHit ( &tHit, iAttrs, dInlineAttrs+iBin*iAttrs );
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

		ARRAY_FOREACH ( i, dBins )
			SafeDelete ( dBins[i] );
		dBins.Reset ();

		CSphWordHit tFlush;
		tFlush.m_iDocID = 0;
		tFlush.m_iWordID = 0;
		tFlush.m_iWordPos = 0;
		cidxHit ( &tFlush, NULL );
	}

	if ( !cidxDone () )
		return 0;

	// close and unlink temp files
	// FIXME! should be unlinked on early-exit too
	fdTmpHits.Close ();
	fdTmpDocinfos.Close ();

	char sBuf [ SPH_MAX_FILENAME_LEN+1 ];
	snprintf ( sBuf, sizeof(sBuf), "%s.tmp1", m_sFilename.cstr() ); unlink ( sBuf );
	snprintf ( sBuf, sizeof(sBuf), "%s.tmp2", m_sFilename.cstr() ); unlink ( sBuf );

	PROFILE_END ( invert_hits );

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	PROFILER_DONE ();
	PROFILE_SHOW ();
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

/// my simple query parser
struct CSphQueryParser : CSphSource_Text
{
	CSphString	m_sQuery;
	CSphString	m_sWords [ SPH_MAX_QUERY_WORDS ];
	int			m_iNumWords;

	CSphQueryParser ( CSphDict * pDict, const char * sQuery, ISphTokenizer * pTokenizer )
		: CSphSource_Text ( "CSphQueryParser" )
	{
		SetTokenizer ( pTokenizer );

		m_iNumWords = 0;
		m_sQuery = sQuery;
		m_pDict = pDict;
		m_bCallWordCallback = true;
		m_tSchema.m_dFields.Add ( CSphColumnInfo ( "word", SPH_ATTR_NONE ) );
		Next ();
	}

	virtual void WordCallback ( char * sWord )
	{
		if ( m_iNumWords<SPH_MAX_QUERY_WORDS )
			m_sWords [ m_iNumWords++ ] = sWord;
	}

	virtual BYTE * NextText()
	{
		m_tDocInfo.m_iDocID = 1;
		return (BYTE*)m_sQuery.cstr();
	}
};


/// match sorter
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
struct MatchAttrLt_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( a.m_pAttrs[t.m_iAttr]!=b.m_pAttrs[t.m_iAttr] )
			return a.m_pAttrs[t.m_iAttr] < b.m_pAttrs[t.m_iAttr];

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
struct MatchAttrGt_fn
{
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( a.m_pAttrs[t.m_iAttr]!=b.m_pAttrs[t.m_iAttr] )
			return a.m_pAttrs[t.m_iAttr] > b.m_pAttrs[t.m_iAttr];

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
struct MatchTimeSegments_fn
{
public:
	/// comparator
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		int iA = GetSegment ( a.m_pAttrs[t.m_iAttr], t.m_iNow );
		int iB = GetSegment ( b.m_pAttrs[t.m_iAttr], t.m_iNow );
		if ( iA!=iB )
			return iA > iB;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		if ( a.m_pAttrs[t.m_iAttr]!=b.m_pAttrs[t.m_iAttr] )
			return a.m_pAttrs[t.m_iAttr] < b.m_pAttrs[t.m_iAttr];

		return a.m_iDocID > b.m_iDocID;
	};

protected:
	/// calc time segment
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
		case SPH_SORT_ATTR_DESC:	pTop = new CSphQueue < CSphMatch, MatchAttrLt_fn > ( pQuery->m_iMaxMatches, true ); break;
		case SPH_SORT_ATTR_ASC:		pTop = new CSphQueue < CSphMatch, MatchAttrGt_fn > ( pQuery->m_iMaxMatches, true ); break;
		case SPH_SORT_TIME_SEGMENTS:pTop = new CSphQueue < CSphMatch, MatchTimeSegments_fn > ( pQuery->m_iMaxMatches, true ); break;
		case SPH_SORT_RELEVANCE:	pTop = new CSphQueue < CSphMatch, MatchRelevanceLt_fn > ( pQuery->m_iMaxMatches, false ); break;
		default:
			pTop = new CSphQueue < CSphMatch, MatchRelevanceLt_fn > ( pQuery->m_iMaxMatches, false );
			fprintf ( stdout, "WARNING: unknown sorting mode '%d', using SPH_SORT_RELEVANCE\n", pQuery->m_eSort );
			break;
	}

	assert ( pTop );
	return pTop;
}


void sphFlattenQueue ( ISphMatchQueue * pQueue, CSphQueryResult * pResult )
{
	int iOffset = pResult->m_dMatches.GetLength ();
	pResult->m_dMatches.Resize ( iOffset + pQueue->GetLength() );

	int iDest = pQueue->GetLength();
	while ( iDest-- )
	{
		pResult->m_dMatches [ iDest+iOffset ] = pQueue->Root ();
		pQueue->Pop ();
	}
}


CSphQueryResult * CSphIndex_VLN::Query ( CSphDict * pDict, CSphQuery * pQuery )
{
	// create result
	CSphQueryResult * pResult = new CSphQueryResult();

	// create queue
	int iAttr = m_tSchema.GetAttrIndex ( pQuery->m_sSortBy.cstr() );
	if ( iAttr<0 )
	{
		if ( pQuery->m_eSort!=SPH_SORT_RELEVANCE )
		{
			fprintf ( stdout, "WARNING: CSphIndex_VLN::Query(): no sort-by attribute '%s', sorting by relevance.\n",
				pQuery->m_sSortBy.cstr() );
		}
		pQuery->m_eSort = SPH_SORT_RELEVANCE;
		iAttr = 0;
	}

	ISphMatchQueue * pTop = sphCreateQueue ( pQuery );
	pTop->SetAttr ( iAttr );

	// run query
	QueryEx ( pDict, pQuery, pResult, pTop );

	// convert results and return
	pResult->m_dMatches.Reset ();
	sphFlattenQueue ( pTop, pResult );
	SafeDelete ( pTop );
	return pResult;
}


static inline bool sphMatchEarlyReject ( const CSphMatch & tMatch, const CSphQuery * pQuery )
{
	if ( tMatch.m_iDocID < pQuery->m_iMinID || tMatch.m_iDocID > pQuery->m_iMaxID )
		return true;

	if ( !pQuery->m_dFilters.GetLength() )
		return false;

	ARRAY_FOREACH ( i, pQuery->m_dFilters )
	{
		const CSphFilter & tFilter = pQuery->m_dFilters[i];
		if ( tFilter.m_iAttrIndex<0 )
			continue;

		DWORD uValue = tMatch.m_pAttrs [ tFilter.m_iAttrIndex ];
		if ( tFilter.m_iValues )
		{
			if ( !sphGroupMatch ( uValue, tFilter.m_pValues, tFilter.m_iValues ) )
				return true;
		} else
		{
			if ( uValue<tFilter.m_uMinValue || uValue>tFilter.m_uMaxValue )
				return true;
		}
	}

	return false;
}


void CSphIndex_VLN::LookupDocinfo ( CSphDocInfo & tMatch )
{
	if ( m_iDocinfo<=0 )
		return;

	assert ( m_eDocinfo==SPH_DOCINFO_EXTERN );
	assert ( m_pDocinfo );
	assert ( m_tSchema.m_dAttrs.GetLength() );
	assert ( tMatch.m_pAttrs );
	assert ( tMatch.m_iAttrs==m_tSchema.m_dAttrs.GetLength() );

	DWORD uHash = ( tMatch.m_iDocID - m_pDocinfo[0] ) >> m_iDocinfoIdShift;
	int iStart = m_pDocinfoHash [ uHash ];
	int iEnd = m_pDocinfoHash [ uHash+1 ] - 1;

	int iStride = 1 + m_tSchema.m_dAttrs.GetLength();
	DWORD * pFound = NULL;

	if ( tMatch.m_iDocID==m_pDocinfo [ iStart*iStride ] )
	{
		pFound = m_pDocinfo;

	} else if ( tMatch.m_iDocID==m_pDocinfo [ iEnd*iStride ] )
	{
		pFound = m_pDocinfo + iEnd*iStride;

	} else
	{
		while ( iEnd-iStart>1 )
		{
			// check if nothing found
			if (
				tMatch.m_iDocID < m_pDocinfo [ iStart*iStride ] ||
				tMatch.m_iDocID > m_pDocinfo [ iEnd*iStride ] )
					break;
			assert ( tMatch.m_iDocID > m_pDocinfo [ iStart*iStride ] );
			assert ( tMatch.m_iDocID < m_pDocinfo [ iEnd*iStride ] );

			int iMid = iStart + (iEnd-iStart)/2;
			if ( tMatch.m_iDocID==m_pDocinfo [ iMid*iStride ] )
			{
				pFound = m_pDocinfo + iMid*iStride;
				break;
			}
			if ( tMatch.m_iDocID<m_pDocinfo [ iMid*iStride ] )
				iEnd = iMid;
			else
				iStart = iMid;
		}
	}

	assert ( pFound );
	memcpy ( tMatch.m_pAttrs, pFound+1, tMatch.m_iAttrs*sizeof(DWORD) );
}


void CSphIndex_VLN::MatchAll ( const CSphQuery * pQuery, ISphMatchQueue * pTop, CSphQueryResult * pResult )
{
	///////////////////
	// match all words
	///////////////////

	if ( !m_dQueryWords[0].m_iDocs )
		return;

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

	// preload doclist entries
	int i;
	for ( i=0; i<m_dQueryWords.GetLength(); i++ )
		m_dQueryWords[i].GetDoclistEntry ();

	i = 0;
	DWORD docID = 0;
	for ( ;; )
	{
		/////////////////////
		// obtain next match
		/////////////////////

		// scan lists until *all* the ids match
		while ( m_dQueryWords[i].m_tDoc.m_iDocID && docID>m_dQueryWords[i].m_tDoc.m_iDocID )
			m_dQueryWords[i].GetDoclistEntry ();
		if ( !m_dQueryWords[i].m_tDoc.m_iDocID )
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
		if ( sphMatchEarlyReject ( tMatch, pQuery ) )
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
					curPhraseWeight[j]++;
					if ( phraseWeight[j] < curPhraseWeight[j] )
						phraseWeight[j] = curPhraseWeight[j];
				} else
				{
					curPhraseWeight[j] = 0;
				}
				k = m_dQueryWords[imin].m_iQueryPos - pmin;
			}

			// check if there was a perfect match
			if ( pQuery->m_eMode==SPH_MATCH_PHRASE && m_dQueryWords.GetLength()>1 )
			{
				for ( i=0; i<m_iWeights; i++ )
				{
					if ( phraseWeight[i]==m_dQueryWords.GetLength()-1 )
						break;
				}
				if ( i==m_iWeights )
				{
					// there was not. continue
					i = 0;
					docID++;
					continue;
				}
			}

			// sum simple match weights and phrase match weights
			tMatch.m_iWeight = 0;
			for ( i=0; i<m_iWeights; i++ )
				tMatch.m_iWeight += m_dWeights[i] * ( matchWeight[i] + phraseWeight[i] );
		}

		////////////////
		// submit match
		////////////////

		// lookup externally stored docinfo
		if ( bLateLookup )
			LookupDocinfo ( tMatch );

		pTop->Push ( tMatch );
		pResult->m_iTotalMatches++;

		// continue looking for next matches
		i = 0;
		docID++;
	}
}


void CSphIndex_VLN::MatchAny ( const CSphQuery * pQuery, ISphMatchQueue * pTop, CSphQueryResult * pResult )
{
	//////////////////
	// match any word
	//////////////////

	int iActive = m_dQueryWords.GetLength(); // total number of words still active
	DWORD iLastMatchID = 0;

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
		DWORD iMinID = UINT_MAX;
		int iMinIndex = -1;

		for ( i=0; i<iActive; i++ )
		{
			// move to next document
			if ( m_dQueryWords[i].m_tDoc.m_iDocID==iLastMatchID && iLastMatchID )
			{
				m_dQueryWords[i].GetDoclistEntry ();
				m_dQueryWords[i].GetHitlistEntry ();
			}

			// remove emptied words
			if ( !m_dQueryWords[i].m_tDoc.m_iDocID )
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
		assert ( iLastMatchID!=0 );
		assert ( iLastMatchID!=UINT_MAX );

		// early reject by group id, doc id or timestamp
		CSphMatch & tMatch = m_dQueryWords[iMinIndex].m_tDoc;
		if ( bEarlyLookup )
			LookupDocinfo ( tMatch );
		if ( sphMatchEarlyReject ( tMatch, pQuery ) )
			continue;

		// get the words we're matching current document against (let's call them "terms")
		CSphQueryWord * pHit [ SPH_MAX_QUERY_WORDS ];
		int iTerms = 0;

		for ( i=0; i<iActive; i++ )
			if ( m_dQueryWords[i].m_tDoc.m_iDocID==iLastMatchID )
				pHit[iTerms++] = &m_dQueryWords[i];
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

		// lookup externally stored docinfo
		if ( bLateLookup )
			LookupDocinfo ( tMatch );

		pTop->Push ( tMatch );
		pResult->m_iTotalMatches++;
	}
}


struct SphEvalNode_t : public CSphQueryWord
{
	SphQueryExpr_e		m_eType;	///< this node's type

	SphEvalNode_t *		m_pExpr;	///< subexperssion
	SphEvalNode_t *		m_pParent;	///< subexperssion's parent

	SphEvalNode_t *		m_pPrev;	///< prev expression at this level
	SphEvalNode_t *		m_pNext;	///< next expression at this level

	CSphMatch *			m_pLast;	///< last matched document at this node (may point to a subexpression or sibling); we only need to store this for NODE_OR type

	bool				m_bInvert;		///< whether to invert the matching logic
	bool				m_bEvaluable;	///< whether this node is for matching or for filtering

public:
						SphEvalNode_t ( const SphQueryExpr_t * pNode, CSphDict * pDict );
						~SphEvalNode_t ();

	void				SetFile ( CSphIndex_VLN * pIndex, int iFD );
	CSphMatch *			MatchNode ( DWORD iMinID );		///< get next match at this node, with ID greater than iMinID
	CSphMatch *			MatchLevel ( DWORD iMinID );	///< get next match at this level, with ID greater than iMinID
	bool				IsRejected ( DWORD iID );		///< check if this match should be rejected
};


SphEvalNode_t::SphEvalNode_t ( const SphQueryExpr_t * pNode, CSphDict * pDict )
{
	assert ( pNode );
	assert ( pDict );

	m_eType = pNode->m_eType;
	m_bInvert = pNode->m_bInvert;
	m_bEvaluable = pNode->m_bEvaluable;

	m_sWord = pNode->m_sWord;
	m_iWordID = 0;

	m_pExpr = pNode->m_pExpr ? new SphEvalNode_t ( pNode->m_pExpr, pDict ) : NULL;
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

	m_pNext = pNode->m_pNext ? new SphEvalNode_t ( pNode->m_pNext, pDict ) : NULL;
	if ( m_pNext )
		m_pNext->m_pPrev = this;

	m_pParent = NULL;

	m_tDoc.m_iDocID = 0;
}


SphEvalNode_t::~SphEvalNode_t ()
{
	SafeDelete ( m_pExpr );
	SafeDelete ( m_pNext );
}


void SphEvalNode_t::SetFile ( CSphIndex_VLN * pIndex, int iFD )
{
	// setup self
	if ( pIndex->SetupQueryWord ( *this, iFD ) )
		m_pLast = &m_tDoc;

	// setup children
	if ( m_pExpr )
		m_pExpr->SetFile ( pIndex, iFD );

	// setup siblings
	for ( SphEvalNode_t * pNode = m_pNext; pNode; pNode = pNode->m_pNext )
		pNode->SetFile ( pIndex, iFD );
}


bool SphEvalNode_t::IsRejected ( DWORD iID )
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
		if ( !m_tDoc.m_iDocID )
		{
			m_pLast = NULL;
			return false;
		}
	}
	return ( iID==m_tDoc.m_iDocID );
}


CSphMatch * SphEvalNode_t::MatchNode ( DWORD iMinID )
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


CSphMatch * SphEvalNode_t::MatchLevel ( DWORD iMinID )
{
	assert ( m_bEvaluable );
	if ( m_eType==NODE_AND )
	{
		// match all siblings
		// search for min equal match ID through all the siblings
		CSphMatch * pCur = NULL;
		for ( SphEvalNode_t * pNode = this; pNode; )
		{
			if ( pNode->m_bEvaluable )
			{
				// as all evaluatable siblings matched last time,
				// we are positive that it is time to advance the pointer
				// and fetch the next id
				CSphMatch * pCandidate = pNode->MatchNode ( iMinID );
				if ( !pCandidate )
					return NULL;

				DWORD iCandidate = pCandidate->m_iDocID;
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
					iMinID = 1+pCur->m_iDocID;
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
		DWORD iMinMatch = UINT_MAX; // best match ID found so far

		for ( SphEvalNode_t * pNode = this; pNode; pNode = pNode->m_pNext )
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
			DWORD iCandidate = pNode->m_pLast->m_iDocID;
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


void CSphIndex_VLN::MatchBoolean ( const CSphQuery * pQuery, CSphDict * pDict, ISphMatchQueue * pTop, CSphQueryResult * pResult, int iDoclistFD )
{
	/////////////////////////
	// match in boolean mode
	/////////////////////////

	// parse query
	SphQueryExpr_t * pTree = sphParseQuery ( pQuery->m_sQuery.cstr(), pQuery->m_pTokenizer );
	if ( !pTree )
		return;

	// let's build our own tree! with doclists! and hits!
	SphEvalNode_t tTree ( pTree, pDict );
	tTree.SetFile ( this, iDoclistFD );

	bool bEarlyLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && pQuery->m_dFilters.GetLength();
	bool bLateLookup = ( m_eDocinfo==SPH_DOCINFO_EXTERN ) && !bEarlyLookup && pTop->UsesAttrs();

	// do matching
	DWORD iMinID = 1;
	for ( ;; )
	{
		CSphMatch * pMatch = tTree.MatchLevel ( iMinID );
		if ( !pMatch )
			break;
		iMinID = 1+pMatch->m_iDocID;

		// early reject by group id, doc id or timestamp
		if ( bEarlyLookup )
			LookupDocinfo ( *pMatch );
		if ( sphMatchEarlyReject ( *pMatch, pQuery ) )
			continue;

		// set weight and push it to the queue
		pMatch->m_iWeight = 1; // set weight

		// lookup externally stored docinfo
		if ( bLateLookup )
			LookupDocinfo ( *pMatch );

		pTop->Push ( *pMatch );
		pResult->m_iTotalMatches++;
	}

	// all done
	SafeDelete ( pTree );
}


bool CSphIndex_VLN::SetupQueryWord ( CSphQueryWord & tWord, int iFD )
{
	// binary search through checkpoints for a one whose range matches word ID
	assert ( m_bPreloaded && m_pWordlist );

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

		assert ( pStart > m_pWordlistCheckpoints );
		assert ( pStart < m_pWordlistCheckpoints+m_iWordlistCheckpoints-1 );
		assert ( tWord.m_iWordID >= pStart[0].m_iWordID && tWord.m_iWordID < pStart[1].m_iWordID );
		iWordlistOffset = pStart->m_iWordlistOffset;
	}
	assert ( iWordlistOffset>=0 );

	// decode wordlist chunk
	BYTE * pBuf = m_pWordlist + iWordlistOffset;

	DWORD iWordID = 0;
	SphOffset_t iDoclistOffset = 0;

	for ( ;; )
	{
		// unpack next word ID
		DWORD iDeltaWord = sphUnzipInt ( pBuf );

		if ( !iDeltaWord ) // wordlist chunk is over
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
			tWord.m_rdDoclist.SetFile ( iFD );
			tWord.m_rdDoclist.SeekTo ( iDoclistOffset );
			tWord.m_rdHitlist.SetFile ( iFD );
			tWord.m_iDocs = iDocs;
			tWord.m_iHits = iHits;
			return true;
		}
	}

	return false;
}


const CSphSchema * CSphIndex_VLN::Preload ()
{
	m_bPreloaded = false;

	//////////////////
	// preload schema
	//////////////////

	char sTmp [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sTmp, sizeof(sTmp), "%s.sph", m_sFilename.cstr() );

	CSphAutofile tIndexInfo ( sTmp );
	if ( tIndexInfo.GetFD()<0 )
		return false;

	CSphReader_VLN rdInfo;
	rdInfo.SetFile ( tIndexInfo.GetFD() );

	// docinfo
	m_eDocinfo = (ESphDocinfo) rdInfo.GetDword ();

	// schema
	m_tSchema.m_dFields.Resize ( rdInfo.GetDword() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		ReadSchemaColumn ( rdInfo, m_tSchema.m_dFields[i] );
	m_tSchema.m_dAttrs.Resize ( rdInfo.GetDword() );
	ARRAY_FOREACH ( i, m_tSchema.m_dAttrs )
		ReadSchemaColumn ( rdInfo, m_tSchema.m_dAttrs[i] );

	// min doc
	m_tMin.m_iDocID = rdInfo.GetDword();
	m_tMin.m_iAttrs = m_tSchema.m_dAttrs.GetLength();
	m_tMin.m_pAttrs = new DWORD [ m_tSchema.m_dAttrs.GetLength() ];
	if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		rdInfo.GetRawBytes ( m_tMin.m_pAttrs, sizeof(DWORD)*m_tSchema.m_dAttrs.GetLength() );

	// wordlist checkpoints
	SphOffset_t iCheckpointsPos;
	rdInfo.GetRawBytes ( &iCheckpointsPos, sizeof(SphOffset_t) );
	m_iWordlistCheckpoints = rdInfo.GetDword ();

	////////////////////
	// preload docinfos
	////////////////////

	if ( m_eDocinfo==SPH_DOCINFO_EXTERN )
	{
		SafeDeleteArray ( m_pDocinfo );
		int iStride = 1+m_tSchema.m_dAttrs.GetLength();
		int iEntrySize = sizeof(DWORD)*iStride;

		snprintf ( sTmp, sizeof(sTmp), "%s.spa", m_sFilename.cstr() );
		CSphAutofile tDocinfo ( sTmp );

		struct stat stDocinfo;
		if ( tDocinfo.GetFD()<0 || stat ( sTmp, &stDocinfo)<0 || stDocinfo.st_size<iEntrySize )
			return NULL;

		m_iDocinfo = stDocinfo.st_size / iEntrySize;
		m_pDocinfo = new DWORD [ m_iDocinfo*iStride ]; // OPTIMIZE? maybe use mmap

		if ( ::read ( tDocinfo.GetFD(), m_pDocinfo, m_iDocinfo*iEntrySize )!=m_iDocinfo*iEntrySize )
		{
			SafeDeleteArray ( m_pDocinfo );
			return NULL;
		}

		// build hash
		m_pDocinfoHash = new DWORD [ (1<<DOCINFO_HASH_BITS) + 1 ];

		DWORD uRange = m_pDocinfo [ (m_iDocinfo-1)*iStride ] - m_pDocinfo[0];
		m_iDocinfoIdShift = 0;
		while ( uRange >= (1<<DOCINFO_HASH_BITS) )
		{
			m_iDocinfoIdShift++;
			uRange >>= 1;
		}

		DWORD uLastHash = 0;
		m_pDocinfoHash[0] = 0;

		for ( int i=1; i<m_iDocinfo; i++ )
		{
			DWORD uHash = ( m_pDocinfo [ i*iStride ] - m_pDocinfo[0] ) >> m_iDocinfoIdShift;
			if ( uHash==uLastHash )
				continue;

			while ( uLastHash<uHash )
				m_pDocinfoHash [ ++uLastHash ] = i;

			uLastHash = uHash;
		}
		m_pDocinfoHash [ ++uLastHash ] = m_iDocinfo;
	}

	////////////////////
	// preload wordlist
	////////////////////

	snprintf ( sTmp, sizeof(sTmp), "%s.spi", m_sFilename.cstr() );
	CSphAutofile tWordlist ( sTmp );

	struct stat stWordlist;
	if ( tWordlist.GetFD()<0 || stat ( sTmp, &stWordlist)<0 || stWordlist.st_size<=0 )
	{
		SafeDeleteArray ( m_pDocinfo );
		return NULL;
	}

	m_pWordlist = new BYTE [ stWordlist.st_size ];
	if ( ::read ( tWordlist.GetFD(), m_pWordlist, stWordlist.st_size )!=stWordlist.st_size )
	{
		SafeDeleteArray ( m_pDocinfo );
		SafeDeleteArray ( m_pWordlist );
		return NULL;
	}

	m_pWordlistCheckpoints = (CSphWordlistCheckpoint *)( m_pWordlist + iCheckpointsPos );

	// all done
	m_bPreloaded = true;
	return &m_tSchema;
}


bool CSphIndex_VLN::QueryEx ( CSphDict * pDict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchQueue * pTop )
{
	assert ( pDict );
	assert ( pQuery );
	assert ( pQuery->m_pTokenizer );
	assert ( pResult );
	assert ( pTop );
	assert ( pTop->GetAttr()>=0 );

	PROFILER_INIT ();
	PROFILE_BEGIN ( query_init );

	// create result and start timing
	float tmQueryStart = sphLongTimer ();

	// open files
	if ( !m_bPreloaded )
		return false;
	pResult->m_tSchema = m_tSchema;

	char sTmp [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sTmp, sizeof(sTmp), "%s.spd", m_sFilename.cstr() );
	CSphAutofile tDoclist ( sTmp );
	if ( tDoclist.GetFD()<0 )
		return false;

	PROFILE_END ( query_init );

	// check that docinfo is preloaded
	assert ( m_eDocinfo!=SPH_DOCINFO_EXTERN || m_pDocinfo );

	// split query into words
	CSphQueryParser * pQueryParser = new CSphQueryParser ( pDict, pQuery->m_sQuery.cstr(),
		pQuery->m_pTokenizer );
	assert ( pQueryParser );

	int iQueryWords = Min ( pQueryParser->m_dHits.GetLength (), SPH_MAX_QUERY_WORDS );
	m_dQueryWords.Resize ( iQueryWords );
	ARRAY_FOREACH ( i, m_dQueryWords )
	{
		m_dQueryWords[i].Reset ();
		m_dQueryWords[i].m_sWord = pQueryParser->m_sWords[i];
		m_dQueryWords[i].m_iWordID = pQueryParser->m_dHits[i].m_iWordID;
		m_dQueryWords[i].m_iQueryPos = 1+i;

		m_dQueryWords[i].m_tDoc.Reset ();
		m_dQueryWords[i].m_tDoc.m_iAttrs = m_tSchema.m_dAttrs.GetLength();
		if ( m_tSchema.m_dAttrs.GetLength() )
			m_dQueryWords[i].m_tDoc.m_pAttrs = new DWORD [ m_tSchema.m_dAttrs.GetLength() ];

		if ( m_eDocinfo==SPH_DOCINFO_INLINE )
		{
			m_dQueryWords[i].m_iInlineAttrs = m_tMin.m_iAttrs;
			m_dQueryWords[i].m_pInlineFixup = m_tMin.m_pAttrs;
		} else
		{
			m_dQueryWords[i].m_iInlineAttrs = 0;
			m_dQueryWords[i].m_pInlineFixup = NULL;
		}
		m_dQueryWords[i].m_iMinID = m_tMin.m_iDocID;
		m_dQueryWords[i].m_tDoc.m_iDocID = m_tMin.m_iDocID;
	}
	SafeDelete ( pQueryParser );

	if ( !iQueryWords )
	{
		pResult->m_iQueryTime += int ( 1000.0f*( sphLongTimer() - tmQueryStart ) );
		return true;
	}

	//////////////////////////////
	// decode words from wordlist
	//////////////////////////////

	PROFILE_BEGIN ( query_load_words );

	ARRAY_FOREACH ( i, m_dQueryWords )
	{
		// if the word was already loaded, just link to it
		int j;
		for ( j=0; j<i; j++ )
			if ( m_dQueryWords[i].m_iWordID == m_dQueryWords[j].m_iWordID )
		{
			m_dQueryWords[i] = m_dQueryWords[j];
			m_dQueryWords[i].m_iQueryPos = i;
			break;
		}
		if ( j<i )
			continue;

		// lookup this wordlist page
		// offset might be -1 if page is totally empty
		SetupQueryWord ( m_dQueryWords[i], tDoclist.GetFD() );
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

	// reorder attr set values and lookup indexes
	ARRAY_FOREACH ( i, pQuery->m_dFilters )
	{
		pQuery->m_dFilters[i].SortValues ();

		// OPTIMIZE! could do hash lookup here
		pQuery->m_dFilters[i].m_iAttrIndex = -1;
		ARRAY_FOREACH ( j, m_tSchema.m_dAttrs )
			if ( pQuery->m_dFilters[i].m_sAttrName==m_tSchema.m_dAttrs[j].m_sName )
		{
			pQuery->m_dFilters[i].m_iAttrIndex = j;
			break;
		}

		// FIXME! should at least warn about bad filter name
	}

	PROFILE_END ( query_load_words );

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
	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:
		case SPH_MATCH_PHRASE:
			MatchAll ( pQuery, pTop, pResult );
			break;

		case SPH_MATCH_ANY:
			MatchAny ( pQuery, pTop, pResult );
			break;

		case SPH_MATCH_BOOLEAN:
			MatchBoolean ( pQuery, pDict, pTop, pResult, tDoclist.GetFD() );
			break;

		default:
			sphDie ( "FATAL: INTERNAL ERROR: unknown matching mode (mode=%d).\n", pQuery->m_eMode );
	}
	PROFILE_END ( query_match );

	if ( !pTop->UsesAttrs() && pTop->GetLength() )
	{
		// FIXME! hack
		CSphMatch * pCur = const_cast < CSphMatch * > ( &pTop->Root() );
		int iCount = pTop->GetLength ();
		while ( iCount-- )
			LookupDocinfo ( *pCur++ );
	}


	PROFILER_DONE ();
	PROFILE_SHOW ();

	// query timer
	pResult->m_iQueryTime += int ( 1000.0f*( sphLongTimer() - tmQueryStart ) );
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
	assert (!( ( m_iMorph & SPH_MORPH_STEM_RU_CP1251 ) && ( m_iMorph & SPH_MORPH_STEM_RU_UTF8 ) ));
	if ( ( m_iMorph & SPH_MORPH_STEM_RU_CP1251 ) || ( m_iMorph & SPH_MORPH_STEM_RU_UTF8 ) )
		stem_ru_init ();
}


DWORD CSphDict_CRC32::GetWordID ( BYTE * pWord )
{
	// apply morphology
	if ( m_iMorph & SPH_MORPH_STEM_EN )
		stem_en ( pWord );
	if ( m_iMorph & SPH_MORPH_STEM_RU_CP1251 )
		stem_ru_cp1251 ( pWord );
	if ( m_iMorph & SPH_MORPH_STEM_RU_UTF8 )
		stem_ru_utf8 ( (WORD*)pWord );
	if ( m_iMorph & SPH_MORPH_SOUNDEX )
		stem_soundex ( pWord );

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


void CSphDict_CRC32::LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer )
{
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
			fprintf ( stdout, "WARNING: failed to load stopwords from '%s'.\n", sName );
			continue;
		}

		// tokenize file
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

	SafeDeleteArray ( sList );
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
			iLen = (int)strlen(pTag->m_sTag); // OPTIMIZE! cache strlen() result
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
				iLen2 = (int)strlen(pAttr->m_sAttr); // OPTIMIZE! cache strlen() result
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

CSphSource::CSphSource ( const char * sName )
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_tSchema ( sName )
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


bool CSphSource::UpdateSchema ( CSphSchema * pInfo )
{
	assert ( pInfo );
	assert ( m_tSchema.m_dFields.GetLength()>0 );
	assert ( m_tSchema.m_dAttrs.GetLength()>=0 );

	// fill it
	if ( pInfo->m_dFields.GetLength()==0 && pInfo->m_dAttrs.GetLength()==0 )
	{
		*pInfo = m_tSchema;
		return true;
	}

	// check it
	CSphString sError;
	if ( !m_tSchema.IsEqual ( *pInfo, sError ) )
	{
		fprintf ( stdout, "ERROR: schema mismatch: %s", sError.cstr() );
		return false;
	}

	return true;
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

	m_tStats.m_iTotalDocuments++;
	m_dHits.Reset ();

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

/////////////////////////////////////////////////////////////////////////////
// PLAIN TEXT SOURCE
/////////////////////////////////////////////////////////////////////////////

BYTE ** CSphSource_Text::NextDocument()
{
	static BYTE * t;
	t = NextText ();
	return t ? &t : NULL;
};



/////////////////////////////////////////////////////////////////////////////
// PGSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_PGSQL

CSphSourceParams_PgSQL::CSphSourceParams_PgSQL ()
	: m_sQuery			( NULL )
	, m_sQueryRange		( NULL )
	, m_iRangeStep		( 1024 )

	, m_sHost			( NULL )
	, m_sUser			( NULL )
	, m_sPass			( NULL )
	, m_sDB				( NULL )
	, m_sPort			( "5432" )
{
}

/////////////////////////////////////////////////////////////////////////////

const char * const CSphSource_PgSQL::MACRO_VALUES [ CSphSource_PgSQL::MACRO_COUNT ] =
{
	"$start",
	"$end"
};


CSphSource_PgSQL::CSphSource_PgSQL ( const char * sName )
	: CSphSource_Document	( sName )
	, m_pSqlResult		( NULL )

	, m_iSqlRows		( 0 )
	, m_iSqlRow			( 0 )

	, m_iMinID			( 0 )
	, m_iMaxID			( 0 )
	, m_iCurrentID		( 0 )

	, m_iMaxFetchedID	( 0 )

	, m_bSqlConnected	( false )
{
}


bool CSphSource_PgSQL::RunQueryStep ()
{
	if ( m_tParams.m_iRangeStep<=0 )
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

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	snprintf ( sValues[0], iBufSize, "%d", m_iCurrentID );
	snprintf ( sValues[1], iBufSize, "%d", m_iCurrentID+m_tParams.m_iRangeStep-1 );
	m_iCurrentID += m_tParams.m_iRangeStep;

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
	const char * sError = NULL;
	for ( ;; )
	{
		if ( m_pSqlResult )
		{
			PQclear ( m_pSqlResult );
			m_pSqlResult = NULL;
		}

		m_pSqlResult = PQexec ( m_tSqlDriver, sRes );
		ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
		if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
		{
			sError = "pgsql_query";
			break;
		}
		break;
	}

	SafeDeleteArray ( sRes );

	// report errors, if any
	if ( sError )
	{
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, PQerrorMessage ( m_tSqlDriver ), m_sSqlDSN.cstr() );
		return false;
	}

	// all ok
	m_iSqlRow = 0;
	m_iSqlRows = PQntuples ( m_pSqlResult );
	return true;
}


bool CSphSource_PgSQL::Init ( const CSphSourceParams_PgSQL & tParams )
{
	#define LOC_ERROR(_msg,_arg) { fprintf ( stdout, _msg, _arg ); return 0; }
	#define LOC_ERROR2(_msg,_arg,_arg2) { fprintf ( stdout, _msg, _arg, _arg2 ); return 0; }
	#define LOC_PGSQL_ERROR(_msg) { sError = _msg; break; }

	const char * sError = NULL;

	// checks
	assert ( !tParams.m_sQuery.IsEmpty() );
	m_tParams = tParams;

	// defaults
	#define LOC_FIX_NULL(_arg) if ( !m_tParams._arg.cstr() ) m_tParams._arg = "";
	LOC_FIX_NULL ( m_sHost );
	LOC_FIX_NULL ( m_sUser );
	LOC_FIX_NULL ( m_sPass );
	LOC_FIX_NULL ( m_sDB );
	LOC_FIX_NULL ( m_sClientEncoding );
	#undef LOC_FIX_NULL

	#define LOC_FIX_QARRAY(_arg) \
		ARRAY_FOREACH ( i, m_tParams._arg ) \
			if ( m_tParams._arg[i].IsEmpty() ) \
				m_tParams._arg.Remove ( i-- );

	LOC_FIX_QARRAY ( m_dQueryPre );
	LOC_FIX_QARRAY ( m_dQueryPost );
	LOC_FIX_QARRAY ( m_dQueryPostIndex );

	#undef LOC_FIX_QARRAY

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "pgsql://%s:***@%s:%s/%s",
		m_tParams.m_sUser.cstr(), m_tParams.m_sHost.cstr(), m_tParams.m_sPort.cstr(), m_tParams.m_sDB.cstr() );
	m_sSqlDSN = sBuf;

	// connect
	for ( ;; )
	{
		// do connect to database
		m_tSqlDriver = PQsetdbLogin ( m_tParams.m_sHost.cstr(), m_tParams.m_sPort.cstr(), NULL, NULL,
			m_tParams.m_sDB.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr() );

		// get connection status
		if ( PQstatus ( m_tSqlDriver )==CONNECTION_BAD )
			LOC_PGSQL_ERROR ( "PQsetdbLogin" ); // could not connect

		m_bSqlConnected = true;		

		// set client encoding
		if ( !m_tParams.m_sClientEncoding.IsEmpty() )
			if ( -1==PQsetClientEncoding ( m_tSqlDriver, m_tParams.m_sClientEncoding.cstr() ) )
				LOC_PGSQL_ERROR ( "PQsetClientEncoding" );

		// run pre-query
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPre )
		{
			m_pSqlResult = PQexec ( m_tSqlDriver, m_tParams.m_dQueryPre[i].cstr() );

			ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
			if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
				LOC_PGSQL_ERROR ( "PQexec_pre" );

			PQclear ( m_pSqlResult );
			m_pSqlResult = NULL;
		}

		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			///////////////
			// range query
			///////////////

			// check step
			if ( m_tParams.m_iRangeStep<=0 )
				LOC_ERROR ( "ERROR: sql_range_step=%d: must be positive.\n", m_tParams.m_iRangeStep );
			if ( m_tParams.m_iRangeStep<128 )
			{
				fprintf ( stdout, "WARNING: sql_range_step=%d: too small; increased to 128.\n", m_tParams.m_iRangeStep );
				m_tParams.m_iRangeStep = 128;
			}

			// check query for macros
			bool bError = false;
			for ( int i=0; i<MACRO_COUNT; i++ )
			{
				if ( !strstr ( m_tParams.m_sQuery.cstr(), MACRO_VALUES[i] ) )
				{
					fprintf ( stdout, "ERROR: sql_query_range: macro '%s' not found.\n",
						MACRO_VALUES[i] );
					bError = true;
				}
			}
			if ( bError )
				return 0;

			// execute
			m_pSqlResult = PQexec ( m_tSqlDriver, m_tParams.m_sQueryRange.cstr() );
			ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
			if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
				LOC_PGSQL_ERROR ( "sql_query_range" );

			// check number of fields (must be exactly 2)
			if ( PQnfields ( m_pSqlResult ) !=2 )
				LOC_ERROR ( "ERROR: sql_query_range: got %d columns, must be 2 (min_id/max_id).\n", PQnfields(m_pSqlResult) );

			// fetch min/max
			m_iMinID = myatoi ( PQgetvalue ( m_pSqlResult, 0, 0 ) );
			m_iMaxID = myatoi ( PQgetvalue ( m_pSqlResult, 0, 1 ) );
			
			// check the data integrity
			if ( m_iMinID<=0 )
				LOC_ERROR ( "ERROR: sql_query_range: min_id=%d: must be positive.\n", m_iMinID );
			if ( m_iMaxID<=0 )
				LOC_ERROR ( "ERROR: sql_query_range: max_id=%d: must be positive.\n", m_iMaxID );
			if ( m_iMinID>m_iMaxID )
				LOC_ERROR2 ( "ERROR: sql_query_range: min_id=%d must not be greater than max_id=%d.\n", m_iMinID, m_iMaxID );

			// free result
			PQclear ( m_pSqlResult );
			m_pSqlResult = NULL;

			m_iCurrentID = m_iMinID;

			// issue query
			if ( !RunQueryStep () )
				return 0;

		} else
		{
			////////////////
			// normal query
			////////////////

			assert ( !m_pSqlResult );
			
			m_pSqlResult = PQexec ( m_tSqlDriver, m_tParams.m_sQuery.cstr() );
			
			ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
			if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
				LOC_PGSQL_ERROR ( "sql_query" );

			m_iSqlRow = 0;
			m_iSqlRows = PQntuples ( m_pSqlResult );

			m_tParams.m_iRangeStep = 0;
		}
		break;
	}

	// errors, anyone?
	if ( sError )
	{
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, PQerrorMessage ( m_tSqlDriver ), m_sSqlDSN.cstr() );
		return 0;
	}

	// some post-query setup
	m_tSchema.m_dColumns.Reset ();
	m_tSchema.m_dFields.GetLength() = 0;
	m_tSchema.m_dAttrs.GetLength() = 0;

	int iCols = PQnfields(m_pSqlResult)-1; // skip column 0, which must be the id
	for ( int i=0; i<iCols; i++ )
	{
		const char * sName = PQfname ( m_pSqlResult, i+1 );

		if ( !sName )
		{
			fprintf ( stdout, "ERROR: column number %d has no name.\n", i+1 );
			return 0;
		}

		CSphColumnInfo tCol ( sName );
		tCol.m_iIndex = i+1;

		ARRAY_FOREACH ( j, m_tParams.m_dAttrs )
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dAttrs[j].m_sName.cstr() ) )
		{
			tCol.m_eAttrType  = m_tParams.m_dAttrs[j].m_eAttrType;
			assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
			break;
		}

		if ( tCol.m_eAttrType==SPH_ATTR_NONE )
			m_tSchema.m_dFields.Add ( tCol );
		else
			m_tSchema.m_dAttrs.Add ( tCol );
	}

	m_tDocInfo.m_iAttrs = m_tSchema.m_dAttrs.GetLength();
	SafeDeleteArray ( m_tDocInfo.m_pAttrs );
	if ( m_tSchema.m_dAttrs.GetLength() )
		m_tDocInfo.m_pAttrs = new DWORD [ m_tSchema.m_dAttrs.GetLength() ];

	// check it
	if ( m_tSchema.m_dFields.GetLength()>SPH_MAX_FIELDS )
	{
		fprintf ( stdout, "ERROR: too many fields (fields=%d, max=%d), please increase SPH_MAX_FIELDS in sphinx.h and recompile.\n",
			m_tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );
		return 0;
	}

	#undef LOC_ERROR
	#undef LOC_PGSQL_ERROR

	return 1;
}


BYTE ** CSphSource_PgSQL::NextDocument ()
{
	PROFILE ( src_PgSQL );
	assert ( m_bSqlConnected );

	// check if we're finished	
	while ( m_iSqlRow>=m_iSqlRows )
	{
		// maybe we can do next step yet?
		if ( RunQueryStep () )
			continue;

		// ok, we're over
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPost )
		{
			m_pSqlResult = PQexec ( m_tSqlDriver, m_tParams.m_dQueryPost[i].cstr() );

			ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
			if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
			{
				fprintf ( stdout, "WARNING: pgsql_query_post(%d): error=%s, query=%s\n",
					i, PQerrorMessage ( m_tSqlDriver ), m_tParams.m_dQueryPost[i].cstr() );
				break;
			}

			PQclear ( m_pSqlResult );
			m_pSqlResult = NULL;
		}

		// close connection and return
		PQfinish ( m_tSqlDriver );
		m_bSqlConnected = false;

		return NULL;
	}

	// get him!
	m_tDocInfo.m_iDocID = myatoi ( PQgetvalue ( m_pSqlResult, m_iSqlRow, 0 ) );
	m_iMaxFetchedID = Max ( m_iMaxFetchedID, m_tDocInfo.m_iDocID );

	if ( !m_tDocInfo.m_iDocID )
		fprintf ( stdout, "WARNING: zero/NULL document_id, aborting indexing\n" );

	// split columns into fields and attrs
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		m_dFields[i] = (BYTE*) PQgetvalue ( m_pSqlResult, m_iSqlRow, m_tSchema.m_dFields[i].m_iIndex );

	ARRAY_FOREACH ( i, m_tSchema.m_dAttrs )
	{
		DWORD * pAttrs = m_tDocInfo.m_pAttrs; // shortcut
		pAttrs[i] = myatoi ( PQgetvalue ( m_pSqlResult, m_iSqlRow, m_tSchema.m_dAttrs[i].m_iIndex ) );
		if ( !pAttrs[i] )
		{
			pAttrs[i] = 1;
			fprintf ( stdout, "WARNING: zero/NULL attribute '%s' for document_id=%d, fixed up to 1\n",
				m_tSchema.m_dAttrs[i].m_sName.cstr(), m_tDocInfo.m_iDocID );
		}
	}

	// now, move to next row
	m_iSqlRow++;
	return m_dFields;
}


void CSphSource_PgSQL::PostIndex ()
{
	if ( !m_tParams.m_dQueryPostIndex.GetLength() || !m_iMaxFetchedID )
		return;

	assert ( !m_bSqlConnected );

	#define LOC_PGSQL_ERROR(_msg) { sError = _msg; break; }

	const char * sError = NULL;
	for ( ;; )
	{		
		// do connect to database
		m_tSqlDriver = PQsetdbLogin ( m_tParams.m_sHost.cstr(), m_tParams.m_sPort.cstr(), NULL, NULL,
			m_tParams.m_sDB.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr() );

		// get connection status
		if ( PQstatus ( m_tSqlDriver )==CONNECTION_BAD )
			LOC_PGSQL_ERROR ( "PQsetdbLogin" );

		m_bSqlConnected = true;
		
		// do execute query
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPostIndex )
		{
			char * sQuery = sphStrMacro ( m_tParams.m_dQueryPostIndex[i].cstr(), "$maxid", m_iMaxFetchedID );	
			m_pSqlResult = PQexec ( m_tSqlDriver, sQuery );
			delete [] sQuery;

			ExecStatusType eRes = PQresultStatus ( m_pSqlResult );
			if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
				LOC_PGSQL_ERROR ( "PQexec_post_index" );

			PQclear ( m_pSqlResult );
			m_pSqlResult = NULL;
		}
		break;
	}
	
	if ( sError )
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, PQerrorMessage ( m_tSqlDriver ), m_sSqlDSN.cstr() );

	#undef LOC_PGSQL_ERROR

	PQfinish ( m_tSqlDriver );
	m_bSqlConnected = false;
}

#endif // USE_PGSQL

/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#if USE_MYSQL

CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
	: m_sQuery			( NULL )
	, m_sQueryRange		( NULL )
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


CSphSource_MySQL::CSphSource_MySQL ( const char * sName )
	: CSphSource_Document	( sName )
	, m_pSqlResult		( NULL )
	, m_tSqlRow			( NULL )

	, m_iMinID			( 0 )
	, m_iMaxID			( 0 )
	, m_iCurrentID		( 0 )

	, m_iMaxFetchedID	( 0 )

	, m_bSqlConnected	( false )
{
}


bool CSphSource_MySQL::RunQueryStep ()
{
	if ( m_tParams.m_iRangeStep<=0 )
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
	assert ( m_tParams.m_sQuery.cstr() );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	snprintf ( sValues[0], iBufSize, "%d", m_iCurrentID );
	snprintf ( sValues[1], iBufSize, "%d", m_iCurrentID+m_tParams.m_iRangeStep-1 );
	m_iCurrentID += m_tParams.m_iRangeStep;

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
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN.cstr() );
		return false;
	}

	// all ok
	return true;
}


bool CSphSource_MySQL::Init ( const CSphSourceParams_MySQL & tParams )
{
	#define LOC_ERROR(_msg,_arg) { fprintf ( stdout, _msg, _arg ); return 0; }
	#define LOC_ERROR2(_msg,_arg,_arg2) { fprintf ( stdout, _msg, _arg, _arg2 ); return 0; }
	#define LOC_MYSQL_ERROR(_msg) { sError = _msg; break; }

	const char * sError = NULL;

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

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "mysql://%s:***@%s:%d/%s",
		m_tParams.m_sUser.cstr(),
		m_tParams.m_sHost.cstr(),
		m_tParams.m_iPort,
		m_tParams.m_sUsock.cstr()
			? ( m_tParams.m_sUsock.cstr() + ( m_tParams.m_sUsock.cstr()[0]=='/' ? 1 : 0 ) )
			: "" );
	m_sSqlDSN = sBuf;

	// connect
	for ( ;; )
	{
		// initialize mysql lib
		mysql_init ( &m_tSqlDriver );

		// do connect
		if ( !mysql_real_connect ( &m_tSqlDriver,
			m_tParams.m_sHost.cstr(),
			m_tParams.m_sUser.cstr(),
			m_tParams.m_sPass.cstr(),
			m_tParams.m_sDB.cstr(),
			m_tParams.m_iPort,
			m_tParams.m_sUsock.cstr(), 0 ) )
		{
			LOC_MYSQL_ERROR ( "mysql_real_connect" );
		}
		m_bSqlConnected = true;

		// run pre-query
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPre )
		{
			if ( mysql_query ( &m_tSqlDriver, m_tParams.m_dQueryPre[i].cstr() ) )
				LOC_MYSQL_ERROR ( "mysql_query_pre" );

			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( m_pSqlResult )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
		}

		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			///////////////
			// range query
			///////////////

			// check step
			if ( m_tParams.m_iRangeStep<=0 )
				LOC_ERROR ( "ERROR: mysql_range_step=%d: must be positive.\n", m_tParams.m_iRangeStep );
			if ( m_tParams.m_iRangeStep<128 )
			{
				fprintf ( stdout, "WARNING: mysql_range_step=%d: too small; increased to 128.\n", m_tParams.m_iRangeStep );
				m_tParams.m_iRangeStep = 128;
			}

			// check query for macros
			bool bError = false;
			for ( int i=0; i<MACRO_COUNT; i++ )
			{
				if ( !strstr ( m_tParams.m_sQuery.cstr(), MACRO_VALUES[i] ) )
				{
					fprintf ( stdout, "ERROR: mysql_ranged_query: macro '%s' not found.\n",
						MACRO_VALUES[i] );
					bError = true;
				}
			}
			if ( bError )
				return 0;

			// run query
			if ( mysql_query ( &m_tSqlDriver, m_tParams.m_sQueryRange.cstr() ) )
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

			m_iMinID = myatoi ( m_tSqlRow[0] );
			m_iMaxID = myatoi ( m_tSqlRow[1] );
			if ( m_iMinID<=0 )
				LOC_ERROR ( "ERROR: mysql_query_range: min_id=%d: must be positive.\n", m_iMinID );
			if ( m_iMaxID<=0 )
				LOC_ERROR ( "ERROR: mysql_query_range: max_id=%d: must be positive.\n", m_iMaxID );
			if ( m_iMinID>m_iMaxID )
				LOC_ERROR2 ( "ERROR: mysql_query_range: min_id=%d must not be greater than max_id=%d.\n", m_iMinID, m_iMaxID );

			mysql_free_result ( m_pSqlResult );
			m_pSqlResult = NULL;

			m_iCurrentID = m_iMinID;

			// issue query
			if ( !RunQueryStep () )
				return 0;

		} else
		{
			////////////////
			// normal query
			////////////////

			assert ( !m_pSqlResult );
			if ( mysql_query ( &m_tSqlDriver, m_tParams.m_sQuery.cstr() ) )
				LOC_MYSQL_ERROR ( "mysql_query" );
			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( !m_pSqlResult )
				LOC_MYSQL_ERROR ( "mysql_use_result" );
		
			m_tParams.m_iRangeStep = 0;
		}
		break;
	}

	// errors, anyone?
	if ( sError )
	{
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN.cstr() );
		return 0;
	}

	// some post-query setup
	m_tSchema.m_dFields.Reset ();
	m_tSchema.m_dAttrs.Reset ();

	int iCols = mysql_num_fields(m_pSqlResult)-1; // skip column 0, which must be the id
	MYSQL_FIELD * pFields = mysql_fetch_fields ( m_pSqlResult );
	for ( int i=0; i<iCols; i++ )
	{
		const char * sName = pFields[i+1].name;

		if ( !sName )
		{
			fprintf ( stdout, "ERROR: column number %d has no name.\n", i+1 );
			return 0;
		}

		CSphColumnInfo tCol ( sName );
		tCol.m_iIndex = i+1;

		ARRAY_FOREACH ( j, m_tParams.m_dAttrs )
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dAttrs[j].m_sName.cstr() ) )
		{
			tCol.m_eAttrType  = m_tParams.m_dAttrs[j].m_eAttrType;
			assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
			break;
		}

		if ( tCol.m_eAttrType==SPH_ATTR_NONE )
			m_tSchema.m_dFields.Add ( tCol );
		else
			m_tSchema.m_dAttrs.Add ( tCol );
	}
	// FIXME! should warn if some attrs went unmapped

	m_tDocInfo.m_iAttrs = m_tSchema.m_dAttrs.GetLength();
	SafeDeleteArray ( m_tDocInfo.m_pAttrs );
	if ( m_tSchema.m_dAttrs.GetLength() )
		m_tDocInfo.m_pAttrs = new DWORD [ m_tSchema.m_dAttrs.GetLength() ];

	// check it
	if ( m_tSchema.m_dFields.GetLength()>SPH_MAX_FIELDS )
	{
		fprintf ( stdout, "ERROR: too many fields (fields=%d, max=%d), please increase SPH_MAX_FIELDS in sphinx.h and recompile.\n",
			m_tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );
		return 0;
	}

	#undef LOC_ERROR
	#undef LOC_MYSQL_ERROR

	return 1;
}


BYTE ** CSphSource_MySQL::NextDocument ()
{
	PROFILE ( src_mysql );
	assert ( m_bSqlConnected );

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
		ARRAY_FOREACH ( i, m_tParams.m_dQueryPost )
		{
			if ( mysql_query ( &m_tSqlDriver, m_tParams.m_dQueryPost[i].cstr() ) )
			{
				fprintf ( stdout, "WARNING: mysql_query_post(%d): error=%s, query=%s\n",
					i, mysql_error ( &m_tSqlDriver ), m_tParams.m_dQueryPost[i].cstr() );
				break;
			}
			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( m_pSqlResult )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
		}

		// close connection and return
		mysql_close ( &m_tSqlDriver );
		m_bSqlConnected = false;

		return NULL;
	}

	// get him!
	m_tDocInfo.m_iDocID = myatoi ( m_tSqlRow[0] );
	m_iMaxFetchedID = Max ( m_iMaxFetchedID, m_tDocInfo.m_iDocID );

	if ( !m_tDocInfo.m_iDocID )
		fprintf ( stdout, "WARNING: zero/NULL document_id, aborting indexing\n" );

	// split columns into fields and attrs
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		m_dFields[i] = (BYTE*) m_tSqlRow [ m_tSchema.m_dFields[i].m_iIndex ];

	ARRAY_FOREACH ( i, m_tSchema.m_dAttrs )
	{
		DWORD * pAttrs = m_tDocInfo.m_pAttrs; // shortcut
		pAttrs[i] = myatoi ( m_tSqlRow [ m_tSchema.m_dAttrs[i].m_iIndex ] );
		if ( !pAttrs[i] )
		{
			pAttrs[i] = 1;
			fprintf ( stdout, "WARNING: zero/NULL attribute '%s' for document_id=%d, fixed up to 1\n",
				m_tSchema.m_dAttrs[i].m_sName.cstr(), m_tDocInfo.m_iDocID );
		}
	}

	return m_dFields;
}


void CSphSource_MySQL::PostIndex ()
{
	if ( !m_tParams.m_dQueryPostIndex.GetLength() || !m_iMaxFetchedID )
		return;

	assert ( !m_bSqlConnected );

	#define LOC_MYSQL_ERROR(_msg) { sError = _msg; break; }

	const char * sError = NULL;
	for ( ;; )
	{
		mysql_init ( &m_tSqlDriver );
		if ( !mysql_real_connect ( &m_tSqlDriver,
			m_tParams.m_sHost.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr(), m_tParams.m_sDB.cstr(),
			m_tParams.m_iPort, m_tParams.m_sUsock.cstr(), 0 ) )
		{
			LOC_MYSQL_ERROR ( "mysql_real_connect" );
		}

		ARRAY_FOREACH ( i, m_tParams.m_dQueryPostIndex )
		{
			char * sQuery = sphStrMacro ( m_tParams.m_dQueryPostIndex[i].cstr(), "$maxid", m_iMaxFetchedID );
			int iRes = mysql_query ( &m_tSqlDriver, sQuery );
			delete [] sQuery;

			if ( iRes )
				LOC_MYSQL_ERROR ( "mysql_query_post_index" );

			m_pSqlResult = mysql_use_result ( &m_tSqlDriver );
			if ( m_pSqlResult )
			{
				mysql_free_result ( m_pSqlResult );
				m_pSqlResult = NULL;
			}
		}
		break;
	}
	
	if ( sError )
		fprintf ( stdout, "ERROR: %s: %s (DSN=%s).\n",
			sError, mysql_error ( &m_tSqlDriver ), m_sSqlDSN.cstr() );

	#undef LOC_MYSQL_ERROR

	mysql_close ( &m_tSqlDriver );
	m_bSqlConnected = false;
}

#endif // USE_MYSQL

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

	m_tDocInfo.Reset ();
	m_tDocInfo.m_iAttrs = 2;
	m_tDocInfo.m_pAttrs = new DWORD [ 2];

	m_tSchema.m_dFields.Reset ();
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "title" ) );
	m_tSchema.m_dFields.Add ( CSphColumnInfo ( "body" ) );

	m_tSchema.m_dAttrs.Reset ();
	m_tSchema.m_dAttrs.Add ( CSphColumnInfo ( "gid", SPH_ATTR_INTEGER ) );
	m_tSchema.m_dAttrs.Add ( CSphColumnInfo ( "ts", SPH_ATTR_TIMESTAMP ) );

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "xmlpipe(%s)", sCommand );
	m_tSchema.m_sName = sBuf;

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
		m_tStats.m_iTotalDocuments++;

		if ( !ScanInt ( "group", &m_tDocInfo.m_pAttrs[0] ) )
			m_tDocInfo.m_pAttrs[0] = 1;

		if ( !ScanInt ( "timestamp", &m_tDocInfo.m_pAttrs[1], false ) )
			m_tDocInfo.m_pAttrs[1] = 1;

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
			fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): unexpected EOF while scanning doc '%d' body.\n",
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


bool CSphSource_XMLPipe::CheckTag ( bool bOpen, bool bStrict )
{
	int iAdd = bOpen ? 2 : 3;

	// if case the tag is at buffer boundary, try to suck in some more data
	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
		UpdateBuffer ();

	if ( m_pBufferEnd-m_pBuffer < m_iTagLength+iAdd )
	{
		if ( bStrict )
			fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
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
			fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got '%s'.\n",
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
			fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): expected '<%s%s>', got EOF.\n",
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
		fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
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
		fprintf ( stdout, "WARNING: CSphSource_XMLPipe(): expected <%s> data, got EOF.\n", m_pTag );
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

/////////////////////////////////////////////////////////////////////////////

void sphSetQuiet ( bool bQuiet )
{
	g_bSphQuiet = bQuiet;
}

//
// $Id$
//
