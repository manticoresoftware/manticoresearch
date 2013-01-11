//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxrt.h"
#include "sphinxsearch.h"
#include "sphinxutils.h"

#include <sys/stat.h>
#include <fcntl.h>

#if USE_WINDOWS
#include <io.h> // for open(), close()
#include <errno.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////

#define BINLOG_WRITE_BUFFER		256*1024
#define BINLOG_AUTO_FLUSH		1000000
#define BINLOG_RESTART_SIZE		128*1024*1024

#define RTDICT_CHECKPOINT_V3			1024
#define RTDICT_CHECKPOINT_V5			48
#define SPH_RT_DOUBLE_BUFFER_PERCENT	10

#if USE_64BIT
#define WORDID_MAX				U64C(0xffffffffffffffff)
#else
#define	WORDID_MAX				0xffffffffUL
#endif

// RT hitman
typedef Hitman_c<8> HITMAN;

//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
#define Verify(_expr) assert(_expr)
#else
#define Verify(_expr) _expr
#endif

//////////////////////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////////////////////

/// publicly exposed binlog interface
ISphBinlog *			g_pBinlog				= NULL;

/// actual binlog implementation
class RtBinlog_c;
static RtBinlog_c *		g_pRtBinlog				= NULL;

/// protection from concurrent changes during binlog replay
static bool				g_bRTChangesAllowed		= false;

//////////////////////////////////////////////////////////////////////////

// !COMMIT cleanup extern ref to sphinx.cpp
extern void sphSortDocinfos ( DWORD * pBuf, int iCount, int iStride );

// !COMMIT yes i am when debugging
#ifndef NDEBUG
#define PARANOID 1
#endif

//////////////////////////////////////////////////////////////////////////

template < typename T, typename P >
static inline void ZipT ( CSphVector < BYTE, P > * pOut, T uValue )
{
	do
	{
		BYTE bOut = (BYTE)( uValue & 0x7f );
		uValue >>= 7;
		if ( uValue )
			bOut |= 0x80;
		pOut->Add ( bOut );
	} while ( uValue );
}

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );


template < typename T >
static inline const BYTE * UnzipT ( T * pValue, const BYTE * pIn )
{
	T uValue = 0;
	BYTE bIn;
	int iOff = 0;

	do
	{
		bIn = *pIn++;
		uValue += ( T ( bIn & 0x7f ) ) << iOff;
		iOff += 7;
	} while ( bIn & 0x80 );

	*pValue = uValue;
	return pIn;
}

#define ZipDword ZipT<DWORD>
#define ZipQword ZipT<uint64_t>
#define UnzipDword UnzipT<DWORD>
#define UnzipQword UnzipT<uint64_t>

#if USE_64BIT
#define ZipDocid ZipQword
#define ZipWordid ZipQword
#define UnzipDocid UnzipQword
#define UnzipWordid UnzipQword
#else
#define ZipDocid ZipDword
#define ZipWordid ZipDword
#define UnzipDocid UnzipDword
#define UnzipWordid UnzipDword
#endif

//////////////////////////////////////////////////////////////////////////

struct CmpHitPlain_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b )
	{
		return 	( a.m_iWordID<b.m_iWordID ) ||
			( a.m_iWordID==b.m_iWordID && a.m_iDocID<b.m_iDocID ) ||
			( a.m_iWordID==b.m_iWordID && a.m_iDocID==b.m_iDocID && a.m_iWordPos<b.m_iWordPos );
	}
};


struct CmpHitKeywords_fn
{
	const BYTE * m_pBase;
	explicit CmpHitKeywords_fn ( const BYTE * pBase ) : m_pBase ( pBase ) {}
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b )
	{
		const BYTE * pPackedA = m_pBase + a.m_iWordID;
		const BYTE * pPackedB = m_pBase + b.m_iWordID;
		int iCmp = sphDictCmpStrictly ( (const char *)pPackedA+1, *pPackedA, (const char *)pPackedB+1, *pPackedB );
		return 	( iCmp<0 ) ||
			( iCmp==0 && a.m_iDocID<b.m_iDocID ) ||
			( iCmp==0 && a.m_iDocID==b.m_iDocID && a.m_iWordPos<b.m_iWordPos );
	}
};


template < typename DOCID = SphDocID_t >
struct RtDoc_T
{
	DOCID						m_uDocID;	///< my document id
	DWORD						m_uDocFields;	///< fields mask
	DWORD						m_uHits;	///< hit count
	DWORD						m_uHit;		///< either index into segment hits, or the only hit itself (if hit count is 1)
};

template < typename WORDID=SphWordID_t >
struct RtWord_T
{
	union
	{
		WORDID					m_uWordID;	///< my keyword id
		const BYTE *			m_sWord;
	};
	DWORD						m_uDocs;	///< document count (for stats and/or BM25)
	DWORD						m_uHits;	///< hit count (for stats and/or BM25)
	DWORD						m_uDoc;		///< index into segment docs
};

typedef RtDoc_T<> RtDoc_t;
typedef RtWord_T<> RtWord_t;

struct RtWordCheckpoint_t
{
	union
	{
		SphWordID_t					m_iWordID;
		const char *				m_sWord;
	};
	int							m_iOffset;
};

class RtDiskKlist_t : public ISphNoncopyable
{
private:
	static const int				MAX_SMALL_SIZE = 512;
	CSphVector < SphAttr_t >		m_dLargeKlist;
	CSphOrderedHash < bool, SphDocID_t, IdentityHash_fn, MAX_SMALL_SIZE >	m_hSmallKlist;
	CSphMutex						m_tLock;

	void NakedFlush();				// flush without lockers

public:
	RtDiskKlist_t() { m_tLock.Init(); }
	virtual ~RtDiskKlist_t() { m_tLock.Done(); }
	void Reset ();
	void Flush()
	{
		if ( m_hSmallKlist.GetLength()==0 )
			return;

		m_tLock.Lock();
		NakedFlush();
		m_tLock.Unlock();
	}
	void LoadFromFile ( const char * sFilename );
	void SaveToFile ( const char * sFilename );
	inline void Delete ( SphDocID_t * pDocs, int iCount )
	{
		if ( !iCount )
			return;

		if ( m_hSmallKlist.GetLength()+iCount>=MAX_SMALL_SIZE )
		{
			int iOff = m_dLargeKlist.GetLength();
			// 1st resize for new data with hashed data
			// 2nd resize for new data copying
			m_dLargeKlist.Resize ( iOff + iCount + m_hSmallKlist.GetLength() );
			m_dLargeKlist.Resize ( iOff + iCount );

			// can not just use memcpy as SphAttr_t(always 64bits) != SphDocID_t(might be 32 or 64 bits)
			SphAttr_t * pDst = m_dLargeKlist.Begin() + iOff;
			while ( iCount-- )
				*pDst++ = *pDocs++;

			NakedFlush();
		} else
		{
			while ( iCount-- )
				m_hSmallKlist.Add ( true, *pDocs++ );
		}
	}
	inline const SphAttr_t * GetKillList () const { return m_dLargeKlist.Begin(); }
	inline int	GetKillListSize () const { return m_dLargeKlist.GetLength(); }
	bool Exists ( SphDocID_t uDoc )
	{
		return ( m_hSmallKlist.Exists ( uDoc ) || m_dLargeKlist.BinarySearch ( SphAttr_t(uDoc))!=NULL );
	}
};

void RtDiskKlist_t::Reset()
{
	m_dLargeKlist.Reset();
	m_hSmallKlist.Reset();
}

void RtDiskKlist_t::NakedFlush()
{
	if ( m_hSmallKlist.GetLength()==0 )
		return;

	m_dLargeKlist.Reserve ( m_dLargeKlist.GetLength()+m_hSmallKlist.GetLength() );
	m_hSmallKlist.IterateStart();
	while ( m_hSmallKlist.IterateNext() )
		m_dLargeKlist.Add ( m_hSmallKlist.IterateGetKey() );
	m_dLargeKlist.Uniq();
	m_hSmallKlist.Reset();
}

// is already id32<>id64 safe
void RtDiskKlist_t::LoadFromFile ( const char * sFilename )
{
	// FIXME!!! got rid of locks here
	m_tLock.Lock();
	Reset();
	m_tLock.Unlock();

	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	if ( !sphIsReadable ( sName.cstr(), &sError ) )
		return;

	CSphAutoreader rdKlist;
	if ( !rdKlist.Open ( sName, sError ) )
		return;

	// FIXME!!! got rid of locks here
	m_tLock.Lock();
	m_dLargeKlist.Resize ( rdKlist.GetDword() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		uLastDocID += ( SphDocID_t ) rdKlist.UnzipOffset();
		m_dLargeKlist[i] = uLastDocID;
	};
	m_tLock.Unlock();
}

void RtDiskKlist_t::SaveToFile ( const char * sFilename )
{
	// FIXME!!! got rid of locks here
	m_tLock.Lock();
	NakedFlush();

	CSphWriter wrKlist;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	wrKlist.OpenFile ( sName.cstr(), sError );

	wrKlist.PutDword ( m_dLargeKlist.GetLength() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		wrKlist.ZipOffset ( m_dLargeKlist[i] - uLastDocID );
		uLastDocID = ( SphDocID_t ) m_dLargeKlist[i];
	};
	m_tLock.Unlock();
	wrKlist.CloseFile ();
}

struct RtSegment_t
{
protected:
	static const int			KLIST_ACCUM_THRESH	= 32;

public:
	static CSphStaticMutex		m_tSegmentSeq;
	static int					m_iSegments;	///< age tag sequence generator
	int							m_iTag;			///< segment age tag

	CSphTightVector<BYTE>			m_dWords;
	CSphVector<RtWordCheckpoint_t>	m_dWordCheckpoints;
	CSphTightVector<uint64_t>		m_dInfixFilterCP;
	CSphTightVector<BYTE>		m_dDocs;
	CSphTightVector<BYTE>		m_dHits;

	int							m_iRows;		///< number of actually allocated rows
	int							m_iAliveRows;	///< number of alive (non-killed) rows
	CSphTightVector<CSphRowitem>		m_dRows;		///< row data storage
	CSphVector<SphDocID_t>		m_dKlist;		///< sorted K-list
	bool						m_bTlsKlist;	///< whether to apply TLS K-list during merge (must only be used by writer during Commit())
	CSphTightVector<BYTE>		m_dStrings;		///< strings storage
	CSphTightVector<DWORD>		m_dMvas;		///< MVAs storage
	CSphVector<BYTE>			m_dKeywordCheckpoints;

	RtSegment_t ()
	{
		m_tSegmentSeq.Lock ();
		m_iTag = m_iSegments++;
		m_tSegmentSeq.Unlock ();
		m_iRows = 0;
		m_iAliveRows = 0;
		m_bTlsKlist = false;
		m_dStrings.Add ( 0 ); // dummy zero offset
		m_dMvas.Add ( 0 ); // dummy zero offset
	}

	int64_t GetUsedRam () const
	{
		// FIXME! gonna break on vectors over 2GB
		return
			( (int64_t)m_dWords.GetLimit() )*sizeof(m_dWords[0]) +
			( (int64_t)m_dDocs.GetLimit() )*sizeof(m_dDocs[0]) +
			( (int64_t)m_dHits.GetLimit() )*sizeof(m_dHits[0]) +
			( (int64_t)m_dStrings.GetLimit() )*sizeof(m_dStrings[0]) +
			( (int64_t)m_dMvas.GetLimit() )*sizeof(m_dMvas[0]) +
			( (int64_t)m_dKeywordCheckpoints.GetLimit() )*sizeof(m_dKeywordCheckpoints[0])+
			( (int64_t)m_dRows.GetLimit() )*sizeof(m_dRows[0]) +
			( (int64_t)m_dInfixFilterCP.GetLength()*sizeof(m_dInfixFilterCP[0]) );
	}

	int GetMergeFactor () const
	{
		return m_iRows;
	}

	const CSphRowitem *		FindRow ( SphDocID_t uDocid ) const;
	const CSphRowitem *		FindAliveRow ( SphDocID_t uDocid ) const;
};

int RtSegment_t::m_iSegments = 0;
CSphStaticMutex RtSegment_t::m_tSegmentSeq;


const CSphRowitem * RtSegment_t::FindRow ( SphDocID_t uDocid ) const
{
	// binary search through the rows
	int iStride = m_dRows.GetLength() / m_iRows;
	SphDocID_t uL = DOCINFO2ID ( m_dRows.Begin() );
	SphDocID_t uR = DOCINFO2ID ( &m_dRows[m_dRows.GetLength()-iStride] );

	if ( uDocid==uL )
		return m_dRows.Begin();

	if ( uDocid==uR )
		return &m_dRows[m_dRows.GetLength()-iStride];

	if ( uDocid<uL || uDocid>uR )
		return NULL;

	int iL = 0;
	int iR = m_iRows-1;
	while ( iR-iL>1 )
	{
		int iM = iL + (iR-iL)/2;
		SphDocID_t uM = DOCINFO2ID ( &m_dRows[iM*iStride] );

		if ( uDocid==uM )
			return &m_dRows[iM*iStride];
		else if ( uDocid>uM )
			iL = iM;
		else
			iR = iM;
	}
	return NULL;
}


const CSphRowitem * RtSegment_t::FindAliveRow ( SphDocID_t uDocid ) const
{
	if ( m_dKlist.BinarySearch ( uDocid ) )
		return NULL;
	else
		return FindRow ( uDocid );
}

//////////////////////////////////////////////////////////////////////////

struct RtDocWriter_t
{
	CSphTightVector<BYTE> *		m_pDocs;
	SphDocID_t					m_uLastDocID;

	explicit RtDocWriter_t ( RtSegment_t * pSeg )
		: m_pDocs ( &pSeg->m_dDocs )
		, m_uLastDocID ( 0 )
	{}

	void ZipDoc ( const RtDoc_t & tDoc )
	{
		CSphTightVector<BYTE> * pDocs = m_pDocs;
		ZipDocid ( pDocs, tDoc.m_uDocID - m_uLastDocID );
		m_uLastDocID = tDoc.m_uDocID;
		ZipDword ( pDocs, tDoc.m_uDocFields );
		ZipDword ( pDocs, tDoc.m_uHits );
		if ( tDoc.m_uHits==1 )
		{
			ZipDword ( pDocs, tDoc.m_uHit & 0xffffffUL );
			ZipDword ( pDocs, tDoc.m_uHit>>24 );
		} else
			ZipDword ( pDocs, tDoc.m_uHit );
	}

	DWORD ZipDocPtr () const
	{
		return m_pDocs->GetLength();
	}

	void ZipRestart ()
	{
		m_uLastDocID = 0;
	}
};

template < typename DOCID = SphDocID_t >
struct RtDocReader_T
{
	typedef RtDoc_T<DOCID>	RTDOC;
	const BYTE *	m_pDocs;
	int				m_iLeft;
	RTDOC			m_tDoc;

	template < typename RTWORD >
	explicit RtDocReader_T ( const RtSegment_t * pSeg, const RTWORD & tWord )
	{
		m_pDocs = ( pSeg->m_dDocs.Begin() ? pSeg->m_dDocs.Begin() + tWord.m_uDoc : NULL );
		m_iLeft = tWord.m_uDocs;
		m_tDoc.m_uDocID = 0;
	}

	const RTDOC * UnzipDoc ()
	{
		if ( !m_iLeft || !m_pDocs )
			return NULL;

		const BYTE * pIn = m_pDocs;
		SphDocID_t uDeltaID;
		pIn = UnzipDocid ( &uDeltaID, pIn );
		RTDOC & mtDoc = *(RTDOC*)&m_tDoc;
		mtDoc.m_uDocID += (DOCID) uDeltaID;
		DWORD uField;
		pIn = UnzipDword ( &uField, pIn );
		m_tDoc.m_uDocFields = uField;
		pIn = UnzipDword ( &mtDoc.m_uHits, pIn );
		if ( mtDoc.m_uHits==1 )
		{
			DWORD a, b;
			pIn = UnzipDword ( &a, pIn );
			pIn = UnzipDword ( &b, pIn );
			mtDoc.m_uHit = a + ( b<<24 );
		} else
			pIn = UnzipDword ( &mtDoc.m_uHit, pIn );
		m_pDocs = pIn;

		m_iLeft--;
		return &mtDoc;
	}
};

typedef RtDocReader_T<> RtDocReader_t;

template < typename VECTOR >
int sphPutBytes ( VECTOR * pOut, const void * pData, int iLen )
{
	int iOff = pOut->GetLength();
	pOut->Resize ( iOff + iLen );
	memcpy ( pOut->Begin()+iOff, pData, iLen );
	return iOff;
}


struct RtWordWriter_t
{
	CSphTightVector<BYTE> *				m_pWords;
	CSphVector<RtWordCheckpoint_t> *	m_pCheckpoints;
	CSphVector<BYTE> *					m_dKeywordCheckpoints;

	CSphKeywordDeltaWriter				m_tLastKeyword;
	SphWordID_t							m_uLastWordID;
	DWORD								m_uLastDoc;
	int									m_iWords;

	bool								m_bKeywordDict;
	int									m_iWordsCheckpoint;

	RtWordWriter_t ( RtSegment_t * pSeg, bool bKeywordDict, int iWordsCheckpoint )
		: m_pWords ( &pSeg->m_dWords )
		, m_pCheckpoints ( &pSeg->m_dWordCheckpoints )
		, m_dKeywordCheckpoints ( &pSeg->m_dKeywordCheckpoints )
		, m_uLastWordID ( 0 )
		, m_uLastDoc ( 0 )
		, m_iWords ( 0 )
		, m_bKeywordDict ( bKeywordDict )
		, m_iWordsCheckpoint ( iWordsCheckpoint )
	{
		assert ( !m_pWords->GetLength() );
		assert ( !m_pCheckpoints->GetLength() );
		assert ( !m_dKeywordCheckpoints->GetLength() );
	}

	void ZipWord ( const RtWord_t & tWord )
	{
		CSphTightVector<BYTE> * pWords = m_pWords;
		if ( ++m_iWords==m_iWordsCheckpoint )
		{
			RtWordCheckpoint_t & tCheckpoint = m_pCheckpoints->Add();
			if ( !m_bKeywordDict )
			{
				tCheckpoint.m_iWordID = tWord.m_uWordID;
			} else
			{
				int iLen = tWord.m_sWord[0];
				assert ( iLen && iLen-1<SPH_MAX_KEYWORD_LEN );
				tCheckpoint.m_iWordID = sphPutBytes ( m_dKeywordCheckpoints, tWord.m_sWord+1, iLen+1 );
				m_dKeywordCheckpoints->Last() = '\0'; // checkpoint is NULL terminating string

				// reset keywords delta encoding
				m_tLastKeyword.Reset();
			}
			tCheckpoint.m_iOffset = pWords->GetLength();

			m_uLastWordID = 0;
			m_uLastDoc = 0;
			m_iWords = 1;
		}

		if ( !m_bKeywordDict )
		{
			ZipWordid ( pWords, tWord.m_uWordID - m_uLastWordID );
		} else
		{
			m_tLastKeyword.PutDelta ( *this, tWord.m_sWord+1, tWord.m_sWord[0] );
		}

		ZipDword ( pWords, tWord.m_uDocs );
		ZipDword ( pWords, tWord.m_uHits );
		ZipDword ( pWords, tWord.m_uDoc - m_uLastDoc );
		m_uLastWordID = tWord.m_uWordID;
		m_uLastDoc = tWord.m_uDoc;
	}

	void PutBytes ( const BYTE * pData, int iLen )
	{
		sphPutBytes ( m_pWords, pData, iLen );
	}
};

template < typename WORDID = SphWordID_t >
struct RtWordReader_T
{
	typedef RtWord_T<WORDID> RTWORD;
	BYTE			m_tPackedWord[SPH_MAX_KEYWORD_LEN+1];
	const BYTE *	m_pCur;
	const BYTE *	m_pMax;
	RTWORD			m_tWord;
	int				m_iWords;

	bool			m_bWordDict;
	int				m_iWordsCheckpoint;
	int				m_iCheckpoint;

	RtWordReader_T ( const RtSegment_t * pSeg, bool bWordDict, int iWordsCheckpoint )
		: m_iWords ( 0 )
		, m_bWordDict ( bWordDict )
		, m_iWordsCheckpoint ( iWordsCheckpoint )
		, m_iCheckpoint ( 0 )
	{
		m_pCur = pSeg->m_dWords.Begin();
		m_pMax = m_pCur + pSeg->m_dWords.GetLength();

		m_tWord.m_uWordID = 0;
		m_tWord.m_uDoc = 0;
		if ( bWordDict )
			m_tWord.m_sWord = m_tPackedWord;
	}

	const RTWORD * UnzipWord ()
	{
		if ( ++m_iWords==m_iWordsCheckpoint )
		{
			m_tWord.m_uDoc = 0;
			m_iWords = 1;
			m_iCheckpoint++;
			if ( !m_bWordDict )
				m_tWord.m_uWordID = 0;
		}
		if ( m_pCur>=m_pMax )
			return NULL;

		const BYTE * pIn = m_pCur;
		DWORD uDeltaDoc;
		if ( m_bWordDict )
		{
			BYTE iMatch, iDelta, uPacked;
			uPacked = *pIn++;
			if ( uPacked & 0x80 )
			{
				iDelta = ( ( uPacked>>4 ) & 7 ) + 1;
				iMatch = uPacked & 15;
			} else
			{
				iDelta = uPacked & 127;
				iMatch = *pIn++;
			}
			m_tPackedWord[0] = iMatch+iDelta;
			memcpy ( m_tPackedWord+1+iMatch, pIn, iDelta );
			m_tPackedWord[1+m_tPackedWord[0]] = 0;
			pIn += iDelta;
		} else
		{
			SphWordID_t uDeltaID;
			pIn = UnzipWordid ( &uDeltaID, pIn );
			m_tWord.m_uWordID += (WORDID) uDeltaID;
		}
		pIn = UnzipDword ( &m_tWord.m_uDocs, pIn );
		pIn = UnzipDword ( &m_tWord.m_uHits, pIn );
		pIn = UnzipDword ( &uDeltaDoc, pIn );
		m_pCur = pIn;

		m_tWord.m_uDoc += uDeltaDoc;
		return &m_tWord;
	}
};

typedef RtWordReader_T<SphWordID_t> RtWordReader_t;

struct RtHitWriter_t
{
	CSphTightVector<BYTE> *		m_pHits;
	DWORD						m_uLastHit;

	explicit RtHitWriter_t ( RtSegment_t * pSeg )
		: m_pHits ( &pSeg->m_dHits )
		, m_uLastHit ( 0 )
	{}

	void ZipHit ( DWORD uValue )
	{
		ZipDword ( m_pHits, uValue - m_uLastHit );
		m_uLastHit = uValue;
	}

	void ZipRestart ()
	{
		m_uLastHit = 0;
	}

	DWORD ZipHitPtr () const
	{
		return m_pHits->GetLength();
	}
};


struct RtHitReader_t
{
	const BYTE *	m_pCur;
	DWORD			m_iLeft;
	DWORD			m_uLast;

	RtHitReader_t ()
		: m_pCur ( NULL )
		, m_iLeft ( 0 )
		, m_uLast ( 0 )
	{}

	template < typename RTDOC >
	explicit RtHitReader_t ( const RtSegment_t * pSeg, const RTDOC * pDoc )
	{
		m_pCur = &pSeg->m_dHits [ pDoc->m_uHit ];
		m_iLeft = pDoc->m_uHits;
		m_uLast = 0;
	}

	DWORD UnzipHit ()
	{
		if ( !m_iLeft )
			return 0;

		DWORD uValue;
		m_pCur = UnzipDword ( &uValue, m_pCur );
		m_uLast += uValue;
		m_iLeft--;
		return m_uLast;
	}
};


struct RtHitReader2_t : public RtHitReader_t
{
	const BYTE * m_pBase;

	RtHitReader2_t ()
		: m_pBase ( NULL )
	{}

	void Seek ( SphOffset_t uOff, int iHits )
	{
		m_pCur = m_pBase + uOff;
		m_iLeft = iHits;
		m_uLast = 0;
	}
};

//////////////////////////////////////////////////////////////////////////

/// forward ref
struct RtIndex_t;

struct AccDocDup_t
{
	SphDocID_t m_uDocid;
	int m_iDupCount;
};


/// indexing accumulator
class RtAccum_t
{
public:
	RtIndex_t *					m_pIndex;		///< my current owner in this thread
	int							m_iAccumDocs;
	CSphTightVector<CSphWordHit>	m_dAccum;
	CSphTightVector<CSphRowitem>	m_dAccumRows;
	CSphVector<SphDocID_t>		m_dAccumKlist;
	CSphTightVector<BYTE>		m_dStrings;
	CSphTightVector<DWORD>		m_dMvas;
	CSphVector<DWORD>			m_dPerDocHitsCount;

	bool						m_bKeywordDict;
	CSphDict *					m_pDict;

private:
	CSphDict *					m_pRefDict;
	RtIndex_t *					m_pRefIndex;
	CSphDict *					m_pDictCloned;
	ISphRtDictWraper *			m_pDictRt;

public:
					explicit RtAccum_t ( bool bKeywordDict );
					~RtAccum_t();

	void			SetupDict ( RtIndex_t * pIndex, CSphDict * pDict, bool bKeywordDict );
	void			ResetDict ();
	void			Sort ();

	void			AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas );
	RtSegment_t *	CreateSegment ( int iRowSize, int iWordsCheckpoint );
	void			CleanupDuplacates ( int iRowSize );
	void			GrabLastWarning ( CSphString & sWarning );
};

/// TLS indexing accumulator (we disallow two uncommitted adds within one thread; and so need at most one)
SphThreadKey_t g_tTlsAccumKey;

/// binlog file view of the index
/// everything that a given log file needs to know about an index
struct BinlogIndexInfo_t
{
	CSphString	m_sName;			///< index name
	int64_t		m_iMinTID;			///< min TID logged by this file
	int64_t		m_iMaxTID;			///< max TID logged by this file
	int64_t		m_iFlushedTID;		///< last flushed TID
	int64_t		m_tmMin;			///< min TID timestamp
	int64_t		m_tmMax;			///< max TID timestamp

	CSphIndex *	m_pIndex;			///< replay only; associated index (might be NULL if we don't serve it anymore!)
	RtIndex_t *	m_pRT;				///< replay only; RT index handle (might be NULL if N/A or non-RT)
	int64_t		m_iPreReplayTID;	///< replay only; index TID at the beginning of this file replay

	BinlogIndexInfo_t ()
		: m_iMinTID ( INT64_MAX )
		, m_iMaxTID ( 0 )
		, m_iFlushedTID ( 0 )
		, m_tmMin ( INT64_MAX )
		, m_tmMax ( 0 )
		, m_pIndex ( NULL )
		, m_pRT ( NULL )
		, m_iPreReplayTID ( 0 )
	{}
};

/// binlog file descriptor
/// file id (aka extension), plus a list of associated index infos
struct BinlogFileDesc_t
{
	int								m_iExt;
	CSphVector<BinlogIndexInfo_t>	m_dIndexInfos;

	BinlogFileDesc_t ()
		: m_iExt ( 0 )
	{}
};

/// Bin Log Operation
enum Blop_e
{
	BLOP_COMMIT			= 1,
	BLOP_UPDATE_ATTRS	= 2,
	BLOP_ADD_INDEX		= 3,
	BLOP_ADD_CACHE		= 4,

	BLOP_TOTAL
};

// forward declaration
class BufferReader_t;
class RtBinlog_c;
struct RtIndex_t;


class BinlogWriter_c : protected CSphWriter
{
public:
					BinlogWriter_c ();
	virtual			~BinlogWriter_c () {}

	virtual	void	Flush ();
	void			Write ();
	void			Fsync ();
	bool			HasUnwrittenData () const { return m_iPoolUsed>0; }
	bool			HasUnsyncedData () const { return m_iLastFsyncPos!=m_iLastWritePos; }

	void			ResetCrc ();	///< restart checksumming
	void			WriteCrc ();	///< finalize and write current checksum to output stream

	void			SetBufferSize ( int iBufferSize )									{ CSphWriter::SetBufferSize ( iBufferSize ); }
	bool			OpenFile ( const CSphString & sName, CSphString & sErrorBuffer )	{ return CSphWriter::OpenFile ( sName, sErrorBuffer ); }
	void			CloseFile ( bool bTruncate=false )									{ CSphWriter::CloseFile ( bTruncate ); }
	SphOffset_t		GetPos () const														{ return m_iPos; }

	void			PutBytes ( const void * pData, int iSize );
	void			PutString ( const char * szString );
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			ZipValue ( uint64_t uValue );

private:
	int64_t			m_iLastWritePos;
	int64_t			m_iLastFsyncPos;

	DWORD			m_uCRC;
};


class BinlogReader_c : protected CSphAutoreader
{
public:
	bool			Open ( const CSphString & sFilename, CSphString & sError )		{ return CSphAutoreader::Open ( sFilename, sError ); }
	void			Close ()														{ CSphAutoreader::Close(); }
	SphOffset_t		GetFilesize ()													{ return CSphAutoreader::GetFilesize(); }

	void			GetBytes ( void * pData, int iSize );
	CSphString		GetString ();
	DWORD			GetDword ();
	uint64_t		UnzipValue ();

	bool			GetErrorFlag ()													{ return CSphAutoreader::GetErrorFlag(); }
	SphOffset_t		GetPos ()														{ return CSphAutoreader::GetPos(); }

	void			ResetCrc ();
	bool			CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos );

private:
	DWORD			m_uCRC;
};


class RtBinlog_c : public ISphBinlog
{
public:
	RtBinlog_c ();
	~RtBinlog_c ();

	void	BinlogCommit ( int64_t * pTID, const char * sIndexName, const RtSegment_t * pSeg, const CSphVector<SphDocID_t> & dKlist, bool bKeywordDict );
	void	BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd );
	void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown );

	void	Configure ( const CSphConfigSection & hSearchd, bool bTestMode );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback );

	void	CreateTimerThread ();

private:
	static const DWORD		BINLOG_VERSION = 4;

	static const DWORD		BINLOG_HEADER_MAGIC = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
	static const DWORD		BLOP_MAGIC = 0x214e5854;			/// magic 'TXN!' header that marks binlog entry
	static const DWORD		BINLOG_META_MAGIC = 0x494c5053;		/// magic 'SPLI' header that marks binlog meta

	int64_t					m_iFlushTimeLeft;
	volatile int			m_iFlushPeriod;

	enum OnCommitAction_e
	{
		ACTION_NONE,
		ACTION_FSYNC,
		ACTION_WRITE
	};
	OnCommitAction_e		m_eOnCommit;

	CSphMutex				m_tWriteLock; // lock on operation

	int						m_iLockFD;
	CSphString				m_sWriterError;
	BinlogWriter_c			m_tWriter;

	mutable CSphVector<BinlogFileDesc_t>	m_dLogFiles; // active log files

	CSphString				m_sLogPath;

	SphThread_t				m_tUpdateTread;
	bool					m_bReplayMode; // replay mode indicator
	bool					m_bDisabled;

	int						m_iRestartSize; // binlog size restart threshold

	// replay stats
	mutable int				m_iReplayedRows;

private:
	static void				DoAutoFlush ( void * pBinlog );
	int 					GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow );
	void					LoadMeta ();
	void					SaveMeta ();
	void					LockFile ( bool bLock );
	void					DoCacheWrite ();
	void					CheckDoRestart ();
	void					CheckDoFlush ();
	void					OpenNewLog ( int iLastState=0 );

	int						ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, int iBinlog );
	bool					ReplayCommit ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const;
	bool					ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const;
	bool					ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const;
	bool					ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const;
};


/// RAM based index
struct RtQword_t;
struct RtIndex_t : public ISphRtIndex, public ISphNoncopyable, public ISphWordlist
{
private:
	static const DWORD			META_HEADER_MAGIC	= 0x54525053;	///< my magic 'SPRT' header
	static const DWORD			META_VERSION		= 7;			///< current version

private:
	int							m_iStride;
	CSphVector<RtSegment_t*>	m_pSegments;

	CSphMutex					m_tWriterMutex;
	mutable CSphRwlock			m_tRwlock;

	/// double buffer
	CSphMutex					m_tSaveInnerMutex;
	CSphMutex					m_tSaveOuterMutex;
	int							m_iDoubleBuffer;
	CSphVector<SphDocID_t>		m_dNewSegmentKlist;					///< raw doc-id container
	CSphFixedVector<SphAttr_t>	m_dDiskChunkKlist;					///< well ordered SphAttr_t kill-list

	int64_t						m_iSoftRamLimit;
	int64_t						m_iDoubleBufferLimit;
	CSphString					m_sPath;
	bool						m_bPathStripped;
	CSphVector<CSphIndex*>		m_pDiskChunks;
	int							m_iLockFD;
	mutable RtDiskKlist_t		m_tKlist;
	int							m_iDiskBase;
	bool						m_bOptimizing;

	int64_t						m_iSavedTID;
	int64_t						m_iSavedRam;
	int64_t						m_tmSaved;

	bool						m_bKeywordDict;
	int							m_iWordsCheckpoint;
	int							m_iMaxCodepointLength;
	ISphTokenizer *				m_pTokenizerIndexing;

public:
	explicit					RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );
	virtual						~RtIndex_t ();

	virtual bool				AddDocument ( int iFields, const char ** ppFields, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError, CSphString & sWarning );
	virtual bool				AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError );
	virtual bool				DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError );
	virtual void				Commit ();
	virtual void				RollBack ();
	void						CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist ); // FIXME? protect?
	virtual void				CheckRamFlush ();
	virtual void				ForceRamFlush ( bool bPeriodic=false );
	virtual void				ForceDiskChunk ();
	virtual bool				AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError );
	virtual bool				Truncate ( CSphString & sError );
	virtual void				Optimize ( volatile bool * pForceTerminate, ThrottleState_t * pThrottle );

private:
	/// acquire thread-local indexing accumulator
	/// returns NULL if another index already uses it in an open txn
	RtAccum_t *					AcquireAccum ( CSphString * sError=NULL );

	RtSegment_t *				MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist );
	const RtWord_t *			CopyWord ( RtSegment_t * pDst, RtWordWriter_t & tOutWord, const RtSegment_t * pSrc, const RtWord_t * pWord, RtWordReader_t & tInWord, const CSphVector<SphDocID_t> * pAccKlist );
	void						MergeWord ( RtSegment_t * pDst, const RtSegment_t * pSrc1, const RtWord_t * pWord1, const RtSegment_t * pSrc2, const RtWord_t * pWord2, RtWordWriter_t & tOut, const CSphVector<SphDocID_t> * pAccKlist );
	void						CopyDoc ( RtSegment_t * pSeg, RtDocWriter_t & tOutDoc, RtWord_t * pWord, const RtSegment_t * pSrc, const RtDoc_t * pDoc );

	void						SaveMeta ( int iDiskChunks, int64_t iTID );
	template < typename DOCID >
	void						SaveDiskHeader ( const char * sFilename, DOCID iMinDocID, int iCheckpoints, SphOffset_t iCheckpointsPosition, int iInfixBlocksOffset, int iInfixCheckpointWordsSize, DWORD uKillListSize, uint64_t uMinMaxSize, const CSphSourceStats & tStats, bool bForceID32=false ) const;
	void						SaveDiskData ( const char * sFilename, const CSphVector<RtSegment_t *> & dSegments, const CSphSourceStats & tStats ) const;
	template < typename DOCID, typename WORDID >
	void						SaveDiskDataImpl ( const char * sFilename, const CSphVector<RtSegment_t *> & dSegments, const CSphSourceStats & tStats ) const;
	void						SaveDiskChunk ( int64_t iTID, const CSphVector<RtSegment_t *> & dSegments, const CSphSourceStats & tStats );
	CSphIndex *					LoadDiskChunk ( const char * sChunk, CSphString & sError ) const;
	bool						LoadRamChunk ( DWORD uVersion, bool bRebuildInfixes );
	bool						SaveRamChunk ();

	virtual void				GetPrefixedWords ( const char * sPrefix, int iPrefix, const char * sWildcard, CSphVector<CSphNamedInt> & dPrefixedWords, BYTE * pDictBuf, int iFD ) const;
	virtual void				GetInfixedWords ( const char * sInfix, int iInfix, const char * sWildcard, CSphVector<CSphNamedInt> & dPrefixedWords ) const;

public:
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4100)
#endif
	virtual SphAttr_t *			GetKillList () const				{ return NULL; }
	virtual int					GetKillListSize () const			{ return 0; }
	virtual bool				HasDocid ( SphDocID_t ) const		{ assert ( 0 ); return false; }

	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) { return 0; }
	virtual bool				Merge ( CSphIndex * pSource, const CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists ) { return false; }

	virtual bool				Prealloc ( bool bMlock, bool bStripPath, CSphString & sWarning );
	virtual void				Dealloc () {}
	virtual bool				Preread ();
	virtual void				SetBase ( const char * sNewBase ) {}
	virtual bool				Rename ( const char * sNewBase ) { return true; }
	virtual bool				Lock () { return true; }
	virtual void				Unlock () {}
	virtual bool				Mlock () { return true; }
	virtual void				PostSetup();
	virtual bool				IsRT() const { return true; }

	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError );
	virtual bool				SaveAttributes ( CSphString & sError ) const { return true; }
	virtual DWORD				GetAttributeStatus () const { return 0; }

	virtual void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) {}
	virtual void				DebugDumpDocids ( FILE * fp ) {}
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) {}
	virtual void				DebugDumpDict ( FILE * fp ) {}
	virtual int					DebugCheck ( FILE * fp );
#if USE_WINDOWS
#pragma warning(pop)
#endif

public:
	virtual bool						EarlyReject ( CSphQueryContext * pCtx, CSphMatch & ) const;
	virtual const CSphSourceStats &		GetStats () const { return m_tStats; }

	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag, bool bFactors ) const;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag, bool bFactors ) const;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const;

	void						CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const;
	const CSphRowitem *			FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID ) const;

	bool						RtQwordSetup ( RtQword_t * pQword, const RtSegment_t * pSeg ) const;
	static bool					RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint );

	CSphDict *					SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const;
	CSphDict *					SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const;

	virtual const CSphSchema &	GetMatchSchema () const { return m_tSchema; }
	virtual const CSphSchema &	GetInternalSchema () const { return m_tSchema; }
	int64_t						GetUsedRam () const;

	virtual void				SetEnableStar ( bool bEnableStar );
	bool						IsWordDict () const { return m_bKeywordDict; }
	void						BuildSegmentInfixes ( RtSegment_t * pSeg ) const;

	// TODO: implement me
	virtual	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) {}

protected:
	CSphSourceStats				m_tStats;
};


RtIndex_t::RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict )

	: ISphRtIndex ( sIndexName, "rtindex" )
	, m_iStride ( DOCINFO_IDSIZE + tSchema.GetRowSize() )
	, m_dDiskChunkKlist ( 0 )
	, m_iSoftRamLimit ( iRamSize )
	, m_sPath ( sPath )
	, m_bPathStripped ( false )
	, m_iLockFD ( -1 )
	, m_iDiskBase ( 0 )
	, m_bOptimizing ( false )
	, m_iSavedTID ( m_iTID )
	, m_iSavedRam ( 0 )
	, m_tmSaved ( sphMicroTimer() )
	, m_bKeywordDict ( bKeywordDict )
	, m_iWordsCheckpoint ( RTDICT_CHECKPOINT_V5 )
	, m_pTokenizerIndexing ( NULL )
{
	MEMORY ( SPH_MEM_IDX_RT );

	m_tSchema = tSchema;
	m_iDoubleBufferLimit = ( m_iSoftRamLimit * SPH_RT_DOUBLE_BUFFER_PERCENT ) / 100;
	m_iDoubleBuffer = 0;

#ifndef NDEBUG
	// check that index cols are static
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		assert ( !m_tSchema.GetAttr(i).m_tLocator.m_bDynamic );
#endif

	Verify ( m_tWriterMutex.Init() );
	Verify ( m_tRwlock.Init() );
	Verify ( m_tSaveOuterMutex.Init() );
	Verify ( m_tSaveInnerMutex.Init() );
}


RtIndex_t::~RtIndex_t ()
{
	int64_t tmSave = sphMicroTimer();
	bool bValid = m_pTokenizer && m_pDict;

	if ( bValid )
	{
		SaveRamChunk ();
		SaveMeta ( m_pDiskChunks.GetLength(), m_iTID );
	}

	Verify ( m_tSaveInnerMutex.Done() );
	Verify ( m_tSaveOuterMutex.Done() );
	Verify ( m_tRwlock.Done() );
	Verify ( m_tWriterMutex.Done() );

	ARRAY_FOREACH ( i, m_pSegments )
		SafeDelete ( m_pSegments[i] );

	ARRAY_FOREACH ( i, m_pDiskChunks )
		SafeDelete ( m_pDiskChunks[i] );

	SafeDelete ( m_pTokenizerIndexing );

	if ( m_iLockFD>=0 )
		::close ( m_iLockFD );

	// might be NULL during startup
	if ( g_pBinlog )
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, true );

	tmSave = sphMicroTimer() - tmSave;
	if ( tmSave>=1000 && bValid )
	{
		sphInfo ( "rt: index %s: ramchunk saved in %d.%03d sec",
			m_sIndexName.cstr(), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
	}
}

#define SPH_THRESHOLD_SAVE_RAM ( 64*1024*1024 )
static int64_t g_iRtFlushPeriod = 10*60*60; // default period is 10 hours


void RtIndex_t::CheckRamFlush ()
{
	int64_t tmSave = sphMicroTimer();
	if ( m_iTID<=m_iSavedTID || ( tmSave-m_tmSaved )/1000000<g_iRtFlushPeriod )
		return;

	Verify ( m_tRwlock.ReadLock() );
	int64_t iUsedRam = GetUsedRam();
	int64_t iDeltaRam = iUsedRam-m_iSavedRam;
	Verify ( m_tRwlock.Unlock() );

	// save if delta-ram runs over maximum value of ram-threshold or 1/3 of index size
	if ( iDeltaRam < Max ( m_iSoftRamLimit/3, SPH_THRESHOLD_SAVE_RAM ) )
		return;

	ForceRamFlush ( true );
}


void RtIndex_t::ForceRamFlush ( bool bPeriodic )
{
	int64_t tmSave = sphMicroTimer();
	if ( m_iTID<=m_iSavedTID )
		return;

	Verify ( m_tRwlock.ReadLock() );

	int64_t iUsedRam = GetUsedRam();
	if ( !SaveRamChunk () )
	{
		sphWarning ( "rt: index %s: ramchunk save FAILED! (error=%s)", m_sIndexName.cstr(), m_sLastError.cstr() );
		Verify ( m_tRwlock.Unlock() );
		return;
	}
	SaveMeta ( m_pDiskChunks.GetLength(), m_iTID );
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	int64_t iWasTID = m_iSavedTID;
	int64_t iWasRam = m_iSavedRam;
	int64_t tmDelta = sphMicroTimer() - m_tmSaved;
	m_iSavedTID = m_iTID;
	m_iSavedRam = iUsedRam;
	m_tmSaved = sphMicroTimer();

	Verify ( m_tRwlock.Unlock() );

	tmSave = sphMicroTimer() - tmSave;
	sphInfo ( "rt: index %s: ramchunk saved ok (mode=%s, last TID="INT64_FMT", current TID="INT64_FMT", "
		"last ram=%d.%03d Mb, current ram=%d.%03d Mb, time delta=%d sec, took=%d.%03d sec)"
		, m_sIndexName.cstr(), bPeriodic ? "periodic" : "forced"
		, iWasTID, m_iTID, (int)(iWasRam/1024/1024), (int)((iWasRam/1024)%1000)
		, (int)(m_iSavedRam/1024/1024), (int)((m_iSavedRam/1024)%1000)
		, (int) (tmDelta/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}


int64_t RtIndex_t::GetUsedRam () const
{
	int64_t iTotal = 0;
	ARRAY_FOREACH ( i, m_pSegments )
		iTotal += m_pSegments[i]->GetUsedRam();

	return iTotal;
}

//////////////////////////////////////////////////////////////////////////
// INDEXING
//////////////////////////////////////////////////////////////////////////

class CSphSource_StringVector : public CSphSource_Document
{
public:
	explicit			CSphSource_StringVector ( int iFields, const char ** ppFields, const CSphSchema & tSchema );
	virtual				~CSphSource_StringVector () {}

	virtual bool		Connect ( CSphString & );
	virtual void		Disconnect ();

	virtual bool		HasAttrsConfigured () { return false; }
	virtual bool		IterateStart ( CSphString & ) { m_iPlainFieldsLength = m_tSchema.m_dFields.GetLength(); return true; }

	virtual bool		IterateMultivaluedStart ( int, CSphString & ) { return false; }
	virtual bool		IterateMultivaluedNext () { return false; }

	virtual bool		IterateFieldMVAStart ( int, CSphString & ) { return false; }
	virtual bool		IterateFieldMVANext () { return false; }

	virtual bool		IterateKillListStart ( CSphString & ) { return false; }
	virtual bool		IterateKillListNext ( SphDocID_t & ) { return false; }

	virtual BYTE **		NextDocument ( CSphString & ) { return m_dFields.Begin(); }

protected:
	CSphVector<BYTE *>			m_dFields;
	CSphVector<CSphWordHit>		m_dHits;
};


CSphSource_StringVector::CSphSource_StringVector ( int iFields, const char ** ppFields, const CSphSchema & tSchema )
	: CSphSource_Document ( "$stringvector" )
{
	m_tSchema = tSchema;

	m_dFields.Resize ( 1+iFields );
	for ( int i=0; i<iFields; i++ )
	{
		m_dFields[i] = (BYTE*) ppFields[i];
		assert ( m_dFields[i] );
	}
	m_dFields [ iFields ] = NULL;

	m_iMaxHits = 0; // force all hits build
}

bool CSphSource_StringVector::Connect ( CSphString & )
{
	m_tHits.m_dData.Reserve ( 1024 );
	return true;
}

void CSphSource_StringVector::Disconnect ()
{
	m_tHits.m_dData.Reset();
}

bool RtIndex_t::AddDocument ( int iFields, const char ** ppFields, const CSphMatch & tDoc,
	bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas,
	CSphString & sError, CSphString & sWarning )
{
	assert ( g_bRTChangesAllowed );

	if ( !tDoc.m_iDocID )
		return true;

	MEMORY ( SPH_MEM_IDX_RT );

	if ( !bReplace )
	{
		m_tRwlock.ReadLock ();
		ARRAY_FOREACH ( i, m_pSegments )
			if ( FindDocinfo ( m_pSegments[i], tDoc.m_iDocID )
				&& !m_pSegments[i]->m_dKlist.BinarySearch ( tDoc.m_iDocID ) )
		{
			m_tRwlock.Unlock ();
			sError.SetSprintf ( "duplicate id '"UINT64_FMT"'", (uint64_t)tDoc.m_iDocID );
			return false; // already exists and not deleted; INSERT fails
		}
		m_tRwlock.Unlock ();
	}

	RtAccum_t * pAcc = AcquireAccum ( &sError );
	if ( !pAcc )
		return false;

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizerIndexing->Clone ( false ) ); // avoid race
	CSphSource_StringVector tSrc ( iFields, ppFields, m_tSchema );

	// SPZ setup
	if ( m_tSettings.m_bIndexSP && !pTokenizer->EnableSentenceIndexing ( sError ) )
		return false;

	if ( !m_tSettings.m_sZones.IsEmpty() && !pTokenizer->EnableZoneIndexing ( sError ) )
		return false;

	if ( m_tSettings.m_bHtmlStrip && !tSrc.SetStripHTML ( m_tSettings.m_sHtmlIndexAttrs.cstr(), m_tSettings.m_sHtmlRemoveElements.cstr(),
			m_tSettings.m_bIndexSP, m_tSettings.m_sZones.cstr(), sError ) )
		return false;

	tSrc.Setup ( m_tSettings );
	tSrc.SetTokenizer ( pTokenizer.Ptr() );
	tSrc.SetDict ( pAcc->m_pDict );
	if ( !tSrc.Connect ( m_sLastError ) )
		return false;

	m_tSchema.CloneWholeMatch ( &tSrc.m_tDocInfo, tDoc );

	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );
	pAcc->GrabLastWarning ( sWarning );
	return AddDocument ( pHits, tDoc, ppStr, dMvas, sError );
}


void AccumCleanup ( void * pArg )
{
	RtAccum_t * pAcc = (RtAccum_t *) pArg;
	SafeDelete ( pAcc );
}


RtAccum_t * RtIndex_t::AcquireAccum ( CSphString * sError )
{
	RtAccum_t * pAcc = NULL;

	// check that no other index is holding the acc
	pAcc = (RtAccum_t*) sphThreadGet ( g_tTlsAccumKey );
	if ( pAcc && pAcc->m_pIndex!=NULL && pAcc->m_pIndex!=this )
	{
		if ( sError )
			sError->SetSprintf ( "current txn is working with another index ('%s')", pAcc->m_pIndex->m_tSchema.m_sName.cstr() );
		return NULL;
	}

	if ( !pAcc )
	{
		pAcc = new RtAccum_t ( m_bKeywordDict );
		sphThreadSet ( g_tTlsAccumKey, pAcc );
		sphThreadOnExit ( AccumCleanup, pAcc );
	}

	assert ( pAcc->m_pIndex==NULL || pAcc->m_pIndex==this );
	pAcc->m_pIndex = this;
	pAcc->SetupDict ( this, m_pDict, m_bKeywordDict );
	return pAcc;
}

bool RtIndex_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, const char ** ppStr, const CSphVector<DWORD> & dMvas,
	CSphString & sError )
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = AcquireAccum ( &sError );
	if ( pAcc )
		pAcc->AddDocument ( pHits, tDoc, m_tSchema.GetRowSize(), ppStr, dMvas );

	return ( pAcc!=NULL );
}


RtAccum_t::RtAccum_t ( bool bKeywordDict )
	: m_pIndex ( NULL )
	, m_iAccumDocs ( 0 )
	, m_bKeywordDict ( bKeywordDict )
	, m_pDict ( NULL )
	, m_pRefDict ( NULL )
	, m_pRefIndex ( NULL )
	, m_pDictCloned ( NULL )
	, m_pDictRt ( NULL )
{
	m_dStrings.Add ( 0 );
	m_dMvas.Add ( 0 );
}

RtAccum_t::~RtAccum_t()
{
	SafeDelete ( m_pDictCloned );
	SafeDelete ( m_pDictRt );
}

void RtAccum_t::SetupDict ( RtIndex_t * pIndex, CSphDict * pDict, bool bKeywordDict )
{
	if ( pIndex!=m_pRefIndex || pDict!=m_pRefDict || bKeywordDict!=m_bKeywordDict )
	{
		SafeDelete ( m_pDictCloned );
		SafeDelete ( m_pDictRt );
		m_pDict = NULL;
		m_pRefIndex = pIndex;
		m_pRefDict = pDict;
		m_bKeywordDict = bKeywordDict;
	}

	if ( !m_pDict )
	{
		m_pDict = m_pRefDict;
		if ( m_pRefDict->HasState() )
		{
			m_pDict = m_pDictCloned = m_pRefDict->Clone();
		}

		if ( m_bKeywordDict )
		{
			m_pDict = m_pDictRt = sphCreateRtKeywordsDictionaryWrapper ( m_pDict );
		}
	}
}

void RtAccum_t::ResetDict ()
{
	assert ( !m_bKeywordDict || m_pDictRt );
	if ( m_pDictRt )
	{
		m_pDictRt->ResetKeywords();
	}
}

void RtAccum_t::Sort ()
{
	if ( !m_bKeywordDict )
	{
		m_dAccum.Sort ( CmpHitPlain_fn() );
	} else
	{
		assert ( m_pDictRt );
		const BYTE * pPackedKeywords = m_pDictRt->GetPackedKeywords();
		m_dAccum.Sort ( CmpHitKeywords_fn ( pPackedKeywords ) );
	}
}

void RtAccum_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas )
{
	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	// schedule existing copies for deletion
	m_dAccumKlist.Add ( tDoc.m_iDocID );

	// reserve some hit space on first use
	if ( pHits && pHits->Length() && !m_dAccum.GetLength() )
		m_dAccum.Reserve ( 128*1024 );

	// accumulate row data; expect fully dynamic rows
	assert ( !tDoc.m_pStatic );
	assert (!( !tDoc.m_pDynamic && iRowSize!=0 ));
	assert (!( tDoc.m_pDynamic && (int)tDoc.m_pDynamic[-1]!=iRowSize ));

	m_dAccumRows.Resize ( m_dAccumRows.GetLength() + DOCINFO_IDSIZE + iRowSize );
	CSphRowitem * pRow = &m_dAccumRows [ m_dAccumRows.GetLength() - DOCINFO_IDSIZE - iRowSize ];
	DOCINFOSETID ( pRow, tDoc.m_iDocID );

	CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
	for ( int i=0; i<iRowSize; i++ )
		pAttrs[i] = tDoc.m_pDynamic[i];

	int iMva = 0;

	const CSphSchema & pSchema = m_pIndex->GetInternalSchema();
	int iAttr = 0;
	for ( int i=0; i<pSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tColumn = pSchema.GetAttr(i);
		if ( tColumn.m_eAttrType==SPH_ATTR_STRING || tColumn.m_eAttrType==SPH_ATTR_JSON )
		{
			const char * pStr = ppStr ? ppStr[iAttr++] : NULL;
			const int iLen = pStr ? strlen ( pStr ) : 0;

			// FIXME! might need to add some json conversion magic here,
			// but how would be go about reporting errors?

			if ( iLen )
			{
				BYTE dLen[3];
				const int iLenPacked = sphPackStrlen ( dLen, iLen );
				const int iOff = m_dStrings.GetLength();
				assert ( iOff>=1 );
				m_dStrings.Resize ( iOff + iLenPacked + iLen );
				memcpy ( &m_dStrings[iOff], dLen, iLenPacked );
				memcpy ( &m_dStrings[iOff+iLenPacked], pStr, iLen );
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, iOff );
			} else
			{
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, 0 );
			}
		} else if ( tColumn.m_eAttrType==SPH_ATTR_UINT32SET || tColumn.m_eAttrType==SPH_ATTR_INT64SET )
		{
			assert ( m_dMvas.GetLength() );
			int iCount = dMvas[iMva];
			if ( iCount )
			{
				int iDst = m_dMvas.GetLength();
				m_dMvas.Resize ( iDst+iCount+1 );
				memcpy ( m_dMvas.Begin()+iDst, dMvas.Begin()+iMva, (iCount+1)*sizeof(dMvas[0]) );
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, iDst );
			} else
			{
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, 0 );
			}

			iMva += iCount+1;
		}
	}

	// accumulate hits
	int iHits = 0;
	if ( pHits && pHits->Length() )
	{
		CSphWordHit tLastHit;
		tLastHit.m_iDocID = 0;
		tLastHit.m_iWordID = 0;
		tLastHit.m_iWordPos = 0;

		iHits = pHits->Length();
		m_dAccum.Reserve ( m_dAccum.GetLength()+iHits );
		for ( const CSphWordHit * pHit = pHits->First(); pHit<=pHits->Last(); pHit++ )
		{
			// ignore duplicate hits
			if ( pHit->m_iDocID==tLastHit.m_iDocID && pHit->m_iWordID==tLastHit.m_iWordID && pHit->m_iWordPos==tLastHit.m_iWordPos )
				continue;

			m_dAccum.Add ( *pHit );
			tLastHit = *pHit;
		}
	}
	m_dPerDocHitsCount.Add ( iHits );

	m_iAccumDocs++;
}


// cook checkpoints - make NULL terminating strings from offsets
static void FixupSegmentCheckpoints ( RtSegment_t * pSeg )
{
	assert ( pSeg &&
		( !pSeg->m_dWordCheckpoints.GetLength() || pSeg->m_dKeywordCheckpoints.GetLength() ) );
	if ( !pSeg->m_dWordCheckpoints.GetLength() )
		return;

	const char * pBase = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
	assert ( pBase );
	ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
	{
		const char * sWord = pBase + pSeg->m_dWordCheckpoints[i].m_iWordID;
		pSeg->m_dWordCheckpoints[i].m_sWord = sWord;
	}
}


RtSegment_t * RtAccum_t::CreateSegment ( int iRowSize, int iWordsCheckpoint )
{
	if ( !m_iAccumDocs )
		return NULL;

	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	RtSegment_t * pSeg = new RtSegment_t ();

	CSphWordHit tClosingHit;
	tClosingHit.m_iWordID = WORDID_MAX;
	tClosingHit.m_iDocID = DOCID_MAX;
	tClosingHit.m_iWordPos = EMPTY_HIT;
	m_dAccum.Add ( tClosingHit );

	RtDoc_t tDoc;
	tDoc.m_uDocID = 0;
	tDoc.m_uDocFields = 0;
	tDoc.m_uHits = 0;
	tDoc.m_uHit = 0;

	RtWord_t tWord;
	tWord.m_uWordID = 0;
	tWord.m_uDocs = 0;
	tWord.m_uHits = 0;
	tWord.m_uDoc = 0;

	RtDocWriter_t tOutDoc ( pSeg );
	RtWordWriter_t tOutWord ( pSeg, m_bKeywordDict, iWordsCheckpoint );
	RtHitWriter_t tOutHit ( pSeg );

	const BYTE * pPacketBase = NULL;
	if ( m_bKeywordDict )
		pPacketBase = m_pDictRt->GetPackedKeywords();

	Hitpos_t uEmbeddedHit = EMPTY_HIT;
	ARRAY_FOREACH ( i, m_dAccum )
	{
		const CSphWordHit & tHit = m_dAccum[i];

		// new keyword or doc; flush current doc
		if ( tHit.m_iWordID!=tWord.m_uWordID || tHit.m_iDocID!=tDoc.m_uDocID )
		{
			if ( tDoc.m_uDocID )
			{
				tWord.m_uDocs++;
				tWord.m_uHits += tDoc.m_uHits;

				if ( uEmbeddedHit )
				{
					assert ( tDoc.m_uHits==1 );
					tDoc.m_uHit = uEmbeddedHit;
				}

				tOutDoc.ZipDoc ( tDoc );
				tDoc.m_uDocFields = 0;
				tDoc.m_uHits = 0;
				tDoc.m_uHit = tOutHit.ZipHitPtr();
			}

			tDoc.m_uDocID = tHit.m_iDocID;
			tOutHit.ZipRestart ();
			uEmbeddedHit = 0;
		}

		// new keyword; flush current keyword
		if ( tHit.m_iWordID!=tWord.m_uWordID )
		{
			tOutDoc.ZipRestart ();
			if ( tWord.m_uWordID )
			{
				if ( m_bKeywordDict )
				{
					const BYTE * pPackedWord = pPacketBase + tWord.m_uWordID;
					assert ( pPackedWord[0] && pPackedWord[0]+1<m_pDictRt->GetPackedLen() );
					tWord.m_sWord = pPackedWord;
				}
				tOutWord.ZipWord ( tWord );
			}

			tWord.m_uWordID = tHit.m_iWordID;
			tWord.m_uDocs = 0;
			tWord.m_uHits = 0;
			tWord.m_uDoc = tOutDoc.ZipDocPtr();
		}

		// just a new hit
		if ( !tDoc.m_uHits )
		{
			uEmbeddedHit = tHit.m_iWordPos;
		} else
		{
			if ( uEmbeddedHit )
			{
				tOutHit.ZipHit ( uEmbeddedHit );
				uEmbeddedHit = 0;
			}

			tOutHit.ZipHit ( tHit.m_iWordPos );
		}

		const int iField = HITMAN::GetField ( tHit.m_iWordPos );
		if ( iField<32 )
			tDoc.m_uDocFields |= ( 1UL<<iField );
		tDoc.m_uHits++;
	}

	if ( m_bKeywordDict )
		FixupSegmentCheckpoints ( pSeg );

	pSeg->m_iRows = m_iAccumDocs;
	pSeg->m_iAliveRows = m_iAccumDocs;

	// copy and sort attributes
	int iStride = DOCINFO_IDSIZE + iRowSize;
	pSeg->m_dRows.SwapData ( m_dAccumRows );
	pSeg->m_dStrings.SwapData ( m_dStrings );
	pSeg->m_dMvas.SwapData ( m_dMvas );
	sphSortDocinfos ( pSeg->m_dRows.Begin(), pSeg->m_dRows.GetLength()/iStride, iStride );

	// done
	return pSeg;
}


struct AccumDocHits_t
{
	SphDocID_t m_uDocid;
	int m_iDocIndex;
	int m_iHitIndex;
	int m_iHitCount;
};


struct CmpDocHitIndex_t
{
	inline bool IsLess ( const AccumDocHits_t & a, const AccumDocHits_t & b ) const
	{
		return ( a.m_uDocid<b.m_uDocid || ( a.m_uDocid==b.m_uDocid && a.m_iDocIndex<b.m_iDocIndex ) );
	}
};


void RtAccum_t::CleanupDuplacates ( int iRowSize )
{
	if ( m_iAccumDocs<=1 )
		return;

	assert ( m_iAccumDocs==m_dPerDocHitsCount.GetLength() );
	CSphVector<AccumDocHits_t> dDocHits ( m_dPerDocHitsCount.GetLength() );
	int iStride = DOCINFO_IDSIZE + iRowSize;

	int iHitIndex = 0;
	CSphRowitem * pRow = m_dAccumRows.Begin();
	for ( int i=0; i<m_iAccumDocs; i++, pRow+=iStride )
	{
		AccumDocHits_t & tElem = dDocHits[i];
		tElem.m_uDocid = DOCINFO2ID ( pRow );
		tElem.m_iDocIndex = i;
		tElem.m_iHitIndex = iHitIndex;
		tElem.m_iHitCount = m_dPerDocHitsCount[i];
		iHitIndex += m_dPerDocHitsCount[i];
	}

	dDocHits.Sort ( CmpDocHitIndex_t() );

	bool bHasDups = false;;
	for ( int i=0; i<dDocHits.GetLength()-1 && !bHasDups; i++ )
		bHasDups = ( dDocHits[i].m_uDocid==dDocHits[i+1].m_uDocid );

	if ( !bHasDups )
		return;

	// filter out unique - keep duplicates, but not last one
	int iDst = 0;
	int iSrc = 1;
	while ( iSrc<dDocHits.GetLength() )
	{
		bool bDup = ( dDocHits[iDst].m_uDocid==dDocHits[iSrc].m_uDocid );
		iDst += bDup;
		dDocHits[iDst] = dDocHits[iSrc++];
	}
	dDocHits.Resize ( iDst );
	assert ( dDocHits.GetLength() );

	// sort by hit index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iHitIndex ) );

	// clean up hits of duplicates
	for ( int iHit = dDocHits.GetLength()-1; iHit>=0; iHit-- )
	{
		if ( !dDocHits[iHit].m_iHitCount )
			continue;

		int iFrom = dDocHits[iHit].m_iHitIndex;
		int iCount = dDocHits[iHit].m_iHitCount;
		if ( iFrom+iCount<m_dAccum.GetLength() )
		{
			for ( int iDst=iFrom, iSrc=iFrom+iCount; iSrc<m_dAccum.GetLength(); iSrc++, iDst++ )
				m_dAccum[iDst] = m_dAccum[iSrc];
		}
		m_dAccum.Resize ( m_dAccum.GetLength()-iCount );
	}

	// sort by docid index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iDocIndex ) );

	// clean up docinfos of duplicates
	for ( int iDoc = dDocHits.GetLength()-1; iDoc>=0; iDoc-- )
	{
		int iDst = dDocHits[iDoc].m_iDocIndex*iStride;
		int iSrc = iDst+iStride;
		while ( iSrc<m_dAccumRows.GetLength() )
		{
			m_dAccumRows[iDst++] = m_dAccumRows[iSrc++];
		}
		m_iAccumDocs--;
		m_dAccumRows.Resize ( m_iAccumDocs*iStride );
	}
}


void RtAccum_t::GrabLastWarning ( CSphString & sWarning )
{
	if ( m_pDictRt && m_pDictRt->GetLastWarning() )
	{
		sWarning = m_pDictRt->GetLastWarning();
		m_pDictRt->ResetWarning();
	}
}


const RtWord_t * RtIndex_t::CopyWord ( RtSegment_t * pDst, RtWordWriter_t & tOutWord,
	const RtSegment_t * pSrc, const RtWord_t * pWord, RtWordReader_t & tInWord,
	const CSphVector<SphDocID_t> * pAccKlist )
{
	RtDocReader_t tInDoc ( pSrc, *pWord );
	RtDocWriter_t tOutDoc ( pDst );

	RtWord_t tNewWord = *pWord;
	tNewWord.m_uDoc = tOutDoc.ZipDocPtr();

	// if flag is there, acc must be there
	// however, NOT vice versa (newly created segments are unaffected by TLS klist)
	assert (!( pSrc->m_bTlsKlist && !pAccKlist ));
#if 0
	// index *must* be holding acc during merge
	assert ( !pAcc || pAcc->m_pIndex==this );
#endif

	// copy docs
	for ( ;; )
	{
		const RtDoc_t * pDoc = tInDoc.UnzipDoc();
		if ( !pDoc )
			break;

		// apply klist
		bool bKill = ( pSrc->m_dKlist.BinarySearch ( pDoc->m_uDocID )!=NULL );
		if ( !bKill && pSrc->m_bTlsKlist )
			bKill = ( pAccKlist->BinarySearch ( pDoc->m_uDocID )!=NULL );

		if ( bKill )
		{
			tNewWord.m_uDocs--;
			tNewWord.m_uHits -= pDoc->m_uHits;
			continue;
		}

		// short route, single embedded hit
		if ( pDoc->m_uHits==1 )
		{
			tOutDoc.ZipDoc ( *pDoc );
			continue;
		}

		// long route, copy hits
		RtHitWriter_t tOutHit ( pDst );
		RtHitReader_t tInHit ( pSrc, pDoc );

		RtDoc_t tDoc = *pDoc;
		tDoc.m_uHit = tOutHit.ZipHitPtr();

		// OPTIMIZE? decode+memcpy?
		for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
			tOutHit.ZipHit ( uValue );

		// copy doc
		tOutDoc.ZipDoc ( tDoc );
	}

	// append word to the dictionary
	if ( tNewWord.m_uDocs )
		tOutWord.ZipWord ( tNewWord );

	// move forward
	return tInWord.UnzipWord ();
}


void RtIndex_t::CopyDoc ( RtSegment_t * pSeg, RtDocWriter_t & tOutDoc, RtWord_t * pWord, const RtSegment_t * pSrc, const RtDoc_t * pDoc )
{
	pWord->m_uDocs++;
	pWord->m_uHits += pDoc->m_uHits;

	if ( pDoc->m_uHits==1 )
	{
		tOutDoc.ZipDoc ( *pDoc );
		return;
	}

	RtHitWriter_t tOutHit ( pSeg );
	RtHitReader_t tInHit ( pSrc, pDoc );

	RtDoc_t tDoc = *pDoc;
	tDoc.m_uHit = tOutHit.ZipHitPtr();
	tOutDoc.ZipDoc ( tDoc );

	// OPTIMIZE? decode+memcpy?
	for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
		tOutHit.ZipHit ( uValue );
}


void RtIndex_t::MergeWord ( RtSegment_t * pSeg, const RtSegment_t * pSrc1, const RtWord_t * pWord1,
	const RtSegment_t * pSrc2, const RtWord_t * pWord2, RtWordWriter_t & tOut,
	const CSphVector<SphDocID_t> * pAccKlist )
{
	assert ( ( !m_bKeywordDict && pWord1->m_uWordID==pWord2->m_uWordID )
		|| ( m_bKeywordDict && sphDictCmpStrictly ( (const char *)pWord1->m_sWord+1, *pWord1->m_sWord, (const char *)pWord2->m_sWord+1, *pWord2->m_sWord )==0 ) );

	RtDocWriter_t tOutDoc ( pSeg );

	RtWord_t tWord;
	if ( !m_bKeywordDict )
		tWord.m_uWordID = pWord1->m_uWordID;
	else
		tWord.m_sWord = pWord1->m_sWord;
	tWord.m_uDocs = 0;
	tWord.m_uHits = 0;
	tWord.m_uDoc = tOutDoc.ZipDocPtr();

	RtDocReader_t tIn1 ( pSrc1, *pWord1 );
	RtDocReader_t tIn2 ( pSrc2, *pWord2 );
	const RtDoc_t * pDoc1 = tIn1.UnzipDoc();
	const RtDoc_t * pDoc2 = tIn2.UnzipDoc();

	while ( pDoc1 || pDoc2 )
	{
		if ( pDoc1 && pDoc2 && pDoc1->m_uDocID==pDoc2->m_uDocID )
		{
			// dupe, must (!) be killed in the first segment, might be in both
#if 0
			assert ( pSrc1->m_dKlist.BinarySearch ( pDoc1->m_uDocID )
				|| ( pSrc1->m_bTlsKlist && pAcc && pAcc->m_dAccumKlist.BinarySearch ( pDoc1->m_uDocID ) ) );
#endif
			if ( !pSrc2->m_dKlist.BinarySearch ( pDoc2->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pSrc2->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc2->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc2, pDoc2 );
			pDoc1 = tIn1.UnzipDoc();
			pDoc2 = tIn2.UnzipDoc();

		} else if ( pDoc1 && ( !pDoc2 || pDoc1->m_uDocID < pDoc2->m_uDocID ) )
		{
			// winner from the first segment
			if ( !pSrc1->m_dKlist.BinarySearch ( pDoc1->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc1->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc1, pDoc1 );
			pDoc1 = tIn1.UnzipDoc();

		} else
		{
			// winner from the second segment
			assert ( pDoc2 && ( !pDoc1 || pDoc2->m_uDocID < pDoc1->m_uDocID ) );
			if ( !pSrc2->m_dKlist.BinarySearch ( pDoc2->m_uDocID )
				&& ( !pSrc2->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc2->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc2, pDoc2 );
			pDoc2 = tIn2.UnzipDoc();
		}
	}

	if ( tWord.m_uDocs )
		tOut.ZipWord ( tWord );
}


#if PARANOID
static void CheckSegmentRows ( const RtSegment_t * pSeg, int iStride )
{
	const CSphTightVector<CSphRowitem> & dRows = pSeg->m_dRows; // shortcut
	for ( int i=iStride; i<dRows.GetLength(); i+=iStride )
		assert ( DOCINFO2ID ( &dRows[i] ) > DOCINFO2ID ( &dRows[i-iStride] ) );
}
#endif

template < typename DOCID = SphDocID_t >
struct RtRowIterator_T : public ISphNoncopyable
{
protected:
	const CSphRowitem * m_pRow;
	const CSphRowitem * m_pRowMax;
	const DOCID * m_pKlist;
	const DOCID * m_pKlistMax;
	const DOCID * m_pTlsKlist;
	const DOCID * m_pTlsKlistMax;
	const int m_iStride;

public:
	explicit RtRowIterator_T ( const RtSegment_t * pSeg, int iStride, bool bWriter, const CSphVector<DOCID> * pAccKlist )
		: m_pRow ( pSeg->m_dRows.Begin() )
		, m_pRowMax ( pSeg->m_dRows.Begin() + pSeg->m_dRows.GetLength() )
		, m_pKlist ( NULL )
		, m_pKlistMax ( NULL )
		, m_pTlsKlist ( NULL )
		, m_pTlsKlistMax ( NULL )
		, m_iStride ( iStride )
	{
		if ( pSeg->m_dKlist.GetLength() )
		{
			m_pKlist = ( const DOCID* ) pSeg->m_dKlist.Begin();
			m_pKlistMax = m_pKlist + pSeg->m_dKlist.GetLength();
		}

		// FIXME? OPTIMIZE? must not scan tls (open txn) in readers; can implement lighter iterator
		// FIXME? OPTIMIZE? maybe we should just rely on the segment order and don't scan tls klist here
		if ( bWriter && pSeg->m_bTlsKlist && pAccKlist && pAccKlist->GetLength() )
		{
			m_pTlsKlist = pAccKlist->Begin();
			m_pTlsKlistMax = m_pTlsKlist + pAccKlist->GetLength();
		}
	}

	const CSphRowitem * GetNextAliveRow ()
	{
		// while there are rows and k-list entries
		while ( m_pRow<m_pRowMax && ( m_pKlist<m_pKlistMax || m_pTlsKlist<m_pTlsKlistMax ) )
		{
			// get next candidate id
			DOCID uID = DOCINFO2ID_T<DOCID>(m_pRow);

			// check if segment k-list kills it
			while ( m_pKlist<m_pKlistMax && *m_pKlist<uID )
				m_pKlist++;

			if ( m_pKlist<m_pKlistMax && *m_pKlist==uID )
			{
				m_pKlist++;
				m_pRow += m_iStride;
				continue;
			}

			// check if txn k-list kills it
			while ( m_pTlsKlist<m_pTlsKlistMax && *m_pTlsKlist<uID )
				m_pTlsKlist++;

			if ( m_pTlsKlist<m_pTlsKlistMax && *m_pTlsKlist==uID )
			{
				m_pTlsKlist++;
				m_pRow += m_iStride;
				continue;
			}

			// oh, so nobody kills it
			break;
		}

		// oops, out of rows
		if ( m_pRow>=m_pRowMax )
			return NULL;

		// got it, and it's alive!
		m_pRow += m_iStride;
		return m_pRow-m_iStride;
	}
};

typedef RtRowIterator_T<> RtRowIterator_t;

#ifdef PARANOID // sanity check in PARANOID mode
template <typename DOCID>
void VerifyEmptyStrings ( const CSphTightVector<BYTE> & dStorage, const CSphSchema & tSchema, const CSphRowitem * pRow )
{
	if ( dStorage.GetLength()>1 )
		return;

	const DWORD * pAttr = DOCINFO2ATTRS_T<DOCID>(pRow);
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		assert ( tCol.m_eAttrType!=SPH_ATTR_STRING
		|| ( tCol.m_eAttrType==SPH_ATTR_STRING && sphGetRowAttr ( pAttr, tCol.m_tLocator )==0 ) );
	}
}
#endif

static DWORD CopyPackedString ( const BYTE * pSrc, CSphTightVector<BYTE> & dDst )
{
	assert ( pSrc );
	assert ( dDst.GetLength()>=1 );
	const BYTE * pStr = NULL;
	const int iLen = sphUnpackStr ( pSrc, &pStr );
	assert ( iLen>0 );
	assert ( pStr );

	const DWORD uOff = dDst.GetLength();
	const DWORD uWriteLen = iLen + ( pStr - pSrc ); // actual length = strings content length + packed length of string
	dDst.Resize ( uOff + uWriteLen );
	memcpy ( dDst.Begin() + uOff, pSrc, uWriteLen );
	return uOff;
}

static DWORD CopyMva ( const DWORD * pSrc, CSphTightVector<DWORD> & dDst )
{
	assert ( pSrc );
	assert ( dDst.GetLength()>=1 );

	DWORD uCount = *pSrc;
	assert ( uCount );

	DWORD iLen = dDst.GetLength();
	dDst.Resize ( iLen+uCount+1 );
	memcpy ( dDst.Begin()+iLen, pSrc, ( uCount+1 )*sizeof(DWORD) );
	return iLen;
}

static void ExtractLocators ( const CSphSchema & tSchema, ESphAttr eAttrType, CSphVector<CSphAttrLocator> & dLocators )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tColumn = tSchema.GetAttr(i);
		if ( tColumn.m_eAttrType==eAttrType )
			dLocators.Add ( tColumn.m_tLocator );
	}
}


class StorageStringWriter_t : ISphNoncopyable
{
private:
	CSphWriter &					m_tDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageStringWriter_t ( const CSphSchema & tSchema, CSphWriter & tDst )
		: m_tDst ( tDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_STRING, m_dLocators );
		ExtractLocators ( tSchema, SPH_ATTR_JSON, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }
	void SetDocid ( SphDocID_t ) {}

	DWORD CopyAttr ( const BYTE * pSrc )
	{
		assert ( m_tDst.GetPos()>0 && m_tDst.GetPos()<( I64C(1)<<32 ) ); // should be 32 bit offset

		const BYTE * pStr = NULL;
		const int iLen = sphUnpackStr ( pSrc, &pStr );
		assert ( iLen && pStr );

		DWORD uAttr = (DWORD)m_tDst.GetPos();
		const int iWriteLen = iLen + ( pStr - pSrc );
		m_tDst.PutBytes ( pSrc, iWriteLen );
		return uAttr;
	}
};


class StorageStringVector_t : ISphNoncopyable
{
private:
	CSphTightVector<BYTE> &			m_dDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageStringVector_t ( const CSphSchema & tSchema, CSphTightVector<BYTE> & dDst )
		: m_dDst ( dDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_STRING, m_dLocators );
		ExtractLocators ( tSchema, SPH_ATTR_JSON, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }
	void SetDocid ( SphDocID_t ) {}

	DWORD CopyAttr ( const BYTE * pSrc )
	{
		assert ( m_dDst.GetLength()>0 && m_dDst.GetLength()<( I64C(1)<<32 ) ); // should be 32 bit offset
		return CopyPackedString ( pSrc, m_dDst );
	}
};


class StorageMvaWriter_t : ISphNoncopyable
{
private:
	CSphWriter &					m_tDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageMvaWriter_t ( const CSphSchema & tSchema, CSphWriter & tDst )
		: m_tDst ( tDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_UINT32SET, m_dLocators );
		ExtractLocators ( tSchema, SPH_ATTR_INT64SET, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }

	void SetDocid ( SphDocID_t uDocid )
	{
		m_tDst.PutDocid ( uDocid );
	}

	DWORD CopyAttr ( const DWORD * pSrc )
	{
		assert ( m_tDst.GetPos()>0 && m_tDst.GetPos()<( I64C(1)<<32 ) ); // should be 32 bit offset

		DWORD uCount = *pSrc;
		assert ( uCount );

		SphOffset_t uOff = m_tDst.GetPos();
		assert ( ( uOff%sizeof(DWORD) )==0 );
		m_tDst.PutBytes ( pSrc, ( uCount+1 )*sizeof(DWORD) );

		return MVA_DOWNSIZE ( uOff/sizeof(DWORD) );
	}
};


class StorageMvaVector_t : ISphNoncopyable
{
private:
	CSphTightVector<DWORD> &		m_dDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageMvaVector_t ( const CSphSchema & tSchema, CSphTightVector<DWORD> & dDst )
		: m_dDst ( dDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_UINT32SET, m_dLocators );
		ExtractLocators ( tSchema, SPH_ATTR_INT64SET, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }

	void SetDocid ( SphDocID_t ) {}

	DWORD CopyAttr ( const DWORD * pSrc )
	{
		assert ( m_dDst.GetLength()>0 && m_dDst.GetLength()<( I64C(1)<<32 ) ); // should be 32 bit offset
		return CopyMva ( pSrc, m_dDst );
	}
};


template <typename DOCID, typename STORAGE, typename SRC>
void CopyFixupStorageAttrs ( const CSphTightVector<SRC> & dSrc, STORAGE & tStorage, CSphRowitem * pRow )
{
	const CSphVector<CSphAttrLocator> & dLocators = tStorage.GetLocators();
	if ( !dLocators.GetLength() )
		return;

	// store string\mva attr for this row
	SphDocID_t uDocid = DOCINFO2ID ( pRow );
	DWORD * pAttr = DOCINFO2ATTRS_T<DOCID>( pRow );
	bool bIdSet = false;
	ARRAY_FOREACH ( i, dLocators )
	{
		const SphAttr_t uOff = sphGetRowAttr ( pAttr, dLocators[i] );
		if ( !uOff )
			continue;

		assert ( uOff && uOff<dSrc.GetLength() );

		if ( !bIdSet ) // setting docid only on saving MVA to disk for plain index comparability
		{
			tStorage.SetDocid ( uDocid );
			bIdSet = true;
		}

		DWORD uAttr = tStorage.CopyAttr ( dSrc.Begin() + uOff );

		sphSetRowAttr ( pAttr, dLocators[i], uAttr );
	}
}


#define BLOOM_PER_ENTRY_VALS_COUNT 8
#define BLOOM_HASHES_COUNT 2

static bool BuildBloom ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8, uint64_t * pBloom, int iKeyValCount )
{
	if ( iLen<iInfixCodepointCount )
		return false;
	// byte offset for each codepoints
	BYTE dOffsets [ SPH_MAX_WORD_LEN+1 ] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42 };
	int iCodes = iLen;
	if ( bUtf8 )
	{
		// build an offsets table into the bytestring
		iCodes = 0;
		const BYTE * s = sWord;
		const BYTE * sEnd = sWord + iLen;
		while ( s<sEnd )
		{
			int iCodepoints = sphUtf8CharBytes ( *s );
			assert ( iCodepoints>=1 && iCodepoints<=4 );
			dOffsets[iCodes+1] = dOffsets[iCodes] + (BYTE)iCodepoints;
			s += iCodepoints;
			iCodes++;
		}
	}
	if ( iCodes<iInfixCodepointCount )
		return false;

	int iKeyBytes = iKeyValCount * 64;
	for ( int i=0; i<=iCodes-iInfixCodepointCount; i++ )
	{
		int iFrom = dOffsets[i];
		int iTo = dOffsets[i+iInfixCodepointCount];
		uint64_t uHash64 = sphFNV64 ( sWord+iFrom, iTo-iFrom );

		uHash64 = ( uHash64>>32 ) ^ ( (DWORD)uHash64 );
		int iByte = (int)( uHash64 % iKeyBytes );
		int iPos = iByte/64;
		uint64_t uVal = U64C(1) << ( iByte % 64 );

		pBloom[iPos] |= uVal;
	}
	return true;
}


void RtIndex_t::BuildSegmentInfixes ( RtSegment_t * pSeg ) const
{
	if ( !pSeg || !m_bKeywordDict || !m_tSettings.m_iMinInfixLen )
		return;

	int iBloomSize = ( pSeg->m_dWordCheckpoints.GetLength()+1 ) * BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT;
	pSeg->m_dInfixFilterCP.Resize ( iBloomSize );
	// reset filters
	memset ( pSeg->m_dInfixFilterCP.Begin(), 0, pSeg->m_dInfixFilterCP.GetLength() * sizeof ( pSeg->m_dInfixFilterCP[0] ) );

	uint64_t * pRough = pSeg->m_dInfixFilterCP.Begin();
	const RtWord_t * pWord = NULL;
	RtWordReader_t rdDictRough ( pSeg, true, m_iWordsCheckpoint );
	while ( ( pWord = rdDictRough.UnzipWord () )!=NULL )
	{
		uint64_t * pVal = pRough + rdDictRough.m_iCheckpoint * BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT;
		BuildBloom ( pWord->m_sWord+1, pWord->m_sWord[0], 2, ( m_iMaxCodepointLength>1 ), pVal+BLOOM_PER_ENTRY_VALS_COUNT*0, BLOOM_PER_ENTRY_VALS_COUNT );
		BuildBloom ( pWord->m_sWord+1, pWord->m_sWord[0], 4, ( m_iMaxCodepointLength>1 ), pVal+BLOOM_PER_ENTRY_VALS_COUNT*1, BLOOM_PER_ENTRY_VALS_COUNT );
	}
}


RtSegment_t * RtIndex_t::MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist )
{
	if ( pSeg1->m_iTag > pSeg2->m_iTag )
		Swap ( pSeg1, pSeg2 );

	RtSegment_t * pSeg = new RtSegment_t ();

	////////////////////
	// merge attributes
	////////////////////

	// check that all the IDs are in proper asc order
#if PARANOID
	CheckSegmentRows ( pSeg1, m_iStride );
	CheckSegmentRows ( pSeg2, m_iStride );
#endif

	// just a shortcut
	CSphTightVector<CSphRowitem> & dRows = pSeg->m_dRows;
	CSphTightVector<BYTE> & dStrings = pSeg->m_dStrings;
	CSphTightVector<DWORD> & dMvas = pSeg->m_dMvas;

	// we might need less because of dupes, but we can not know yet
	dRows.Reserve ( Max ( pSeg1->m_dRows.GetLength(), pSeg2->m_dRows.GetLength() ) );

	// as each segment has dummy zero we reserve less
	assert ( pSeg1->m_dStrings.GetLength() + pSeg2->m_dStrings.GetLength()>=2 );
	dStrings.Reserve ( Max ( pSeg1->m_dStrings.GetLength(), pSeg2->m_dStrings.GetLength() ) );
	assert ( pSeg1->m_dMvas.GetLength() + pSeg2->m_dMvas.GetLength()>=2 );
	dMvas.Reserve ( Max ( pSeg1->m_dMvas.GetLength(), pSeg2->m_dMvas.GetLength() ) );

	StorageStringVector_t tStorageString ( m_tSchema, dStrings );
	StorageMvaVector_t tStorageMva ( m_tSchema, dMvas );

	RtRowIterator_t tIt1 ( pSeg1, m_iStride, true, pAccKlist );
	RtRowIterator_t tIt2 ( pSeg2, m_iStride, true, pAccKlist );

	const CSphRowitem * pRow1 = tIt1.GetNextAliveRow();
	const CSphRowitem * pRow2 = tIt2.GetNextAliveRow();

	while ( pRow1 || pRow2 )
	{
		if ( !pRow2 || ( pRow1 && pRow2 && DOCINFO2ID(pRow1)<DOCINFO2ID(pRow2) ) )
		{
			assert ( pRow1 );
			for ( int i=0; i<m_iStride; i++ )
				dRows.Add ( *pRow1++ );
			CSphRowitem * pDstRow = dRows.Begin() + dRows.GetLength() - m_iStride;
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg1->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg1->m_dMvas, tStorageMva, pDstRow );
			pRow1 = tIt1.GetNextAliveRow();
		} else
		{
			assert ( pRow2 );
			assert ( !pRow1 || ( DOCINFO2ID(pRow1)!=DOCINFO2ID(pRow2) ) ); // all dupes must be killed and skipped by the iterator
			for ( int i=0; i<m_iStride; i++ )
				dRows.Add ( *pRow2++ );
			CSphRowitem * pDstRow = dRows.Begin() + dRows.GetLength() - m_iStride;
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg2->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg2->m_dMvas, tStorageMva, pDstRow );
			pRow2 = tIt2.GetNextAliveRow();
		}
		pSeg->m_iRows++;
		pSeg->m_iAliveRows++;
	}

	assert ( pSeg->m_iRows*m_iStride==pSeg->m_dRows.GetLength() );
#if PARANOID
	CheckSegmentRows ( pSeg, m_iStride );
#endif

	//////////////////
	// merge keywords
	//////////////////

	pSeg->m_dWords.Reserve ( Max ( pSeg1->m_dWords.GetLength(), pSeg2->m_dWords.GetLength() ) );
	pSeg->m_dDocs.Reserve ( Max ( pSeg1->m_dDocs.GetLength(), pSeg2->m_dDocs.GetLength() ) );
	pSeg->m_dHits.Reserve ( Max ( pSeg1->m_dHits.GetLength(), pSeg2->m_dHits.GetLength() ) );

	RtWordWriter_t tOut ( pSeg, m_bKeywordDict, m_iWordsCheckpoint );
	RtWordReader_t tIn1 ( pSeg1, m_bKeywordDict, m_iWordsCheckpoint );
	RtWordReader_t tIn2 ( pSeg2, m_bKeywordDict, m_iWordsCheckpoint );
	const RtWord_t * pWords1 = tIn1.UnzipWord ();
	const RtWord_t * pWords2 = tIn2.UnzipWord ();

	// merge while there are common words
	for ( ;; )
	{
		while ( pWords1 && pWords2 )
		{
			int iCmp = 0;
			if ( m_bKeywordDict )
			{
				iCmp = sphDictCmpStrictly ( (const char *)pWords1->m_sWord+1, *pWords1->m_sWord, (const char *)pWords2->m_sWord+1, *pWords2->m_sWord );
			} else
			{
				if ( pWords1->m_uWordID<pWords2->m_uWordID )
					iCmp = -1;
				else if ( pWords1->m_uWordID>pWords2->m_uWordID )
					iCmp = 1;
			}

			if ( iCmp==0 )
				break;

			if ( iCmp<0 )
				pWords1 = CopyWord ( pSeg, tOut, pSeg1, pWords1, tIn1, pAccKlist );
			else
				pWords2 = CopyWord ( pSeg, tOut, pSeg2, pWords2, tIn2, pAccKlist );
		}

		if ( !pWords1 || !pWords2 )
			break;

		assert ( pWords1 && pWords2 &&
			( ( !m_bKeywordDict && pWords1->m_uWordID==pWords2->m_uWordID )
			|| ( m_bKeywordDict && sphDictCmpStrictly ( (const char *)pWords1->m_sWord+1, *pWords1->m_sWord, (const char *)pWords2->m_sWord+1, *pWords2->m_sWord )==0 ) ) );
		MergeWord ( pSeg, pSeg1, pWords1, pSeg2, pWords2, tOut, pAccKlist );
		pWords1 = tIn1.UnzipWord();
		pWords2 = tIn2.UnzipWord();
	}

	// copy tails
	while ( pWords1 ) pWords1 = CopyWord ( pSeg, tOut, pSeg1, pWords1, tIn1, pAccKlist );
	while ( pWords2 ) pWords2 = CopyWord ( pSeg, tOut, pSeg2, pWords2, tIn2, pAccKlist );

	if ( m_bKeywordDict )
		FixupSegmentCheckpoints ( pSeg );

	BuildSegmentInfixes ( pSeg );

	assert ( pSeg->m_dRows.GetLength() );
	assert ( pSeg->m_iRows );
	assert ( pSeg->m_iAliveRows==pSeg->m_iRows );
	return pSeg;
}


struct CmpSegments_fn
{
	inline bool IsLess ( const RtSegment_t * a, const RtSegment_t * b )
	{
		return a->GetMergeFactor() > b->GetMergeFactor();
	}
};


void RtIndex_t::Commit ()
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( SPH_MEM_IDX_RT );

	RtAccum_t * pAcc = AcquireAccum();
	if ( !pAcc )
		return;

	// empty txn, just ignore
	if ( !pAcc->m_iAccumDocs && !pAcc->m_dAccumKlist.GetLength() )
	{
		pAcc->m_pIndex = NULL;
		pAcc->m_iAccumDocs = 0;
		pAcc->m_dAccumRows.Resize ( 0 );
		pAcc->m_dStrings.Resize ( 1 );
		pAcc->m_dPerDocHitsCount.Resize ( 0 );
		pAcc->ResetDict();
		return;
	}

	// phase 0, build a new segment
	// accum and segment are thread local; so no locking needed yet
	// segment might be NULL if we're only killing rows this txn
	pAcc->CleanupDuplacates ( m_tSchema.GetRowSize() );
	pAcc->Sort();

	RtSegment_t * pNewSeg = pAcc->CreateSegment ( m_tSchema.GetRowSize(), m_iWordsCheckpoint );
	assert ( !pNewSeg || pNewSeg->m_iRows>0 );
	assert ( !pNewSeg || pNewSeg->m_iAliveRows>0 );
	assert ( !pNewSeg || pNewSeg->m_bTlsKlist==false );

	BuildSegmentInfixes ( pNewSeg );

#if PARANOID
	if ( pNewSeg )
		CheckSegmentRows ( pNewSeg, m_iStride );
#endif

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );
	pAcc->m_dStrings.Resize ( 1 ); // handle dummy zero offset
	pAcc->m_dPerDocHitsCount.Resize ( 0 );
	pAcc->ResetDict();

	// sort accum klist, too
	pAcc->m_dAccumKlist.Uniq ();

	// now on to the stuff that needs locking and recovery
	CommitReplayable ( pNewSeg, pAcc->m_dAccumKlist );

	// done; cleanup accum
	pAcc->m_pIndex = NULL;
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
	// reset accumulated warnings
	CSphString sWarning;
	pAcc->GrabLastWarning ( sWarning );
}

void RtIndex_t::CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist )
{
	int iNewDocs = pNewSeg ? pNewSeg->m_iRows : 0;

	// phase 1, lock out other writers (but not readers yet)
	// concurrent readers are ok during merges, as existing segments won't be modified yet
	// however, concurrent writers are not
	Verify ( m_tWriterMutex.Lock() );
	Verify ( m_tSaveInnerMutex.Lock() );

	// first of all, binlog txn data for recovery
	g_pRtBinlog->BinlogCommit ( &m_iTID, m_sIndexName.cstr(), pNewSeg, dAccKlist, m_bKeywordDict );
	int64_t iTID = m_iTID;

	// let merger know that existing segments are subject to additional, TLS K-list filter
	// safe despite the readers, flag must only be used by writer
	if ( dAccKlist.GetLength() )
		for ( int i=m_iDoubleBuffer; i<m_pSegments.GetLength(); i++ )
	{
		// OPTIMIZE? only need to set the flag if TLS K-list *actually* affects segment
		assert ( m_pSegments[i]->m_bTlsKlist==false );
		m_pSegments[i]->m_bTlsKlist = true;
	}

	// prepare new segments vector
	// create more new segments by merging as needed
	// do not (!) kill processed old segments just yet, as readers might still need them
	CSphVector<RtSegment_t*> dSegments;
	CSphVector<RtSegment_t*> dToKill;

	dSegments.Reserve ( m_pSegments.GetLength() - m_iDoubleBuffer + 1 );
	for ( int i=m_iDoubleBuffer; i<m_pSegments.GetLength(); i++ )
	{
		dSegments.Add ( m_pSegments[i] );
	}
	if ( pNewSeg )
		dSegments.Add ( pNewSeg );

	int64_t iRamFreed = 0;

	// enforce RAM usage limit
	int64_t iRamLeft = m_iDoubleBuffer ? m_iDoubleBufferLimit : m_iSoftRamLimit;
	ARRAY_FOREACH ( i, dSegments )
		iRamLeft = Max ( iRamLeft - dSegments[i]->GetUsedRam(), 0 );

	// skip merging if no rows were added or no memory left
	bool bDump = ( iRamLeft==0 );
	const int MAX_SEGMENTS = 32;
	const int MAX_PROGRESSION_SEGMENT = 8;
	const int64_t MAX_SEGMENT_VECTOR_LEN = INT_MAX;
	while ( pNewSeg && iRamLeft>0 )
	{
		// segments sort order: large first, smallest last
		// merge last smallest segments
		dSegments.Sort ( CmpSegments_fn() );

		// unconditionally merge if there's too much segments now
		// conditionally merge if smallest segment has grown too large
		// otherwise, we're done
		const int iLen = dSegments.GetLength();
		if ( iLen < ( MAX_SEGMENTS - MAX_PROGRESSION_SEGMENT ) )
			break;
		assert ( iLen>=2 );
		// exit if progression is kept AND lesser MAX_SEGMENTS limit
		if ( dSegments[iLen-2]->GetMergeFactor() > dSegments[iLen-1]->GetMergeFactor()*2 && iLen < MAX_SEGMENTS )
			break;

		// check whether we have enough RAM
#define LOC_ESTIMATE1(_seg,_vec) \
	(int)( ( (int64_t)_seg->_vec.GetLength() ) * _seg->m_iAliveRows / _seg->m_iRows )

#define LOC_ESTIMATE(_vec) \
	( LOC_ESTIMATE1 ( dSegments[iLen-1], _vec ) + LOC_ESTIMATE1 ( dSegments[iLen-2], _vec ) )

		int64_t iWordsRelimit = CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dWords ) );
		int64_t iDocsRelimit = CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dDocs ) );
		int64_t iHitsRelimit = CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dHits ) );
		int64_t iStringsRelimit = CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dStrings ) );
		int64_t iMvasRelimit = CSphTightVectorPolicy<DWORD>::Relimit ( 0, LOC_ESTIMATE ( m_dMvas ) );
		int64_t iKeywordsRelimit = CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dKeywordCheckpoints ) );
		int64_t iRowsRelimit = CSphTightVectorPolicy<SphDocID_t>::Relimit ( 0, LOC_ESTIMATE ( m_dRows ) );

#undef LOC_ESTIMATE
#undef LOC_ESTIMATE1

		int64_t iEstimate = iWordsRelimit + iDocsRelimit + iHitsRelimit + iStringsRelimit + iMvasRelimit + iKeywordsRelimit + iRowsRelimit;
		if ( iEstimate>iRamLeft )
		{
			// dump case: can't merge any more AND segments count limit's reached
			bDump = ( ( iRamLeft + iRamFreed )<=iEstimate ) && ( iLen>=MAX_SEGMENTS );
			break;
		}

		// we have to dump if we can't merge even smallest segments without breaking vector constrain ( len<INT_MAX )
		// split this way to avoid superlong string after macro expansion that kills gcov
		int64_t iMaxLen = Max (
			Max ( iWordsRelimit, iDocsRelimit ),
			Max ( iHitsRelimit, iStringsRelimit ) );
		iMaxLen = Max (
			Max ( iMvasRelimit, iKeywordsRelimit ),
			Max ( iMaxLen, iRowsRelimit ) );

		if ( MAX_SEGMENT_VECTOR_LEN<iMaxLen )
		{
			bDump = true;
			break;
		}

		// do it
		RtSegment_t * pA = dSegments.Pop();
		RtSegment_t * pB = dSegments.Pop();
		dSegments.Add ( MergeSegments ( pA, pB, &dAccKlist ) );
		dToKill.Add ( pA );
		dToKill.Add ( pB );

		iRamFreed += pA->GetUsedRam() + pB->GetUsedRam();

		int64_t iMerged = dSegments.Last()->GetUsedRam();
		iRamLeft -= Min ( iRamLeft, iMerged );
	}

	// phase 2, obtain exclusive writer lock
	// we now have to update K-lists in (some of) the survived segments
	// and also swap in new segment list

	// adjust for an incoming accumulator K-list
	int iTotalKilled = 0;
	if ( dAccKlist.GetLength() )
	{
#ifndef NDEBUG
#if PARANOID
		// check that klist is sorted and unique
		for ( int i=1; i<dAccKlist.GetLength(); i++ )
			assert ( dAccKlist[i-1] < dAccKlist[i] );
#endif
#endif

		// update totals
		// work the original (!) segments, and before (!) updating their K-lists
		int iDiskLiveKLen = dAccKlist.GetLength();
		for ( int i=0; i<iDiskLiveKLen; i++ )
		{
			const SphDocID_t uDocid = dAccKlist[i];

			// check RAM chunk
			bool bRamKilled = false;
			for ( int j=m_iDoubleBuffer; j<m_pSegments.GetLength() && !bRamKilled; j++ )
				bRamKilled = ( m_pSegments[j]->FindAliveRow ( uDocid )!=NULL );

			bool bDiskKilled = false;
			if ( !m_iDoubleBuffer )
				bDiskKilled = m_tKlist.Exists ( uDocid );

			// check disk chunks
			bool bKeep = false;
			if ( !bRamKilled || !bDiskKilled )
			{
				// check saving segments first (will be recent disk chunk)
				for ( int i=0; i<m_iDoubleBuffer && !bKeep; i++ )
				{
					bKeep = ( m_pSegments[i]->FindAliveRow ( uDocid )!=NULL );
				}

				// then disk chunks
				for ( int j=m_pDiskChunks.GetLength()-1; j>=0 && !bKeep; j-- )
				{
					if ( m_pDiskChunks[j]->HasDocid ( uDocid ) )
					{
						// we just found the most recent chunk with our suspect docid
						// let's check whether it's already killed by subsequent chunks, or gets killed now
						SphAttr_t uRef = uDocid;
						bKeep = true;
						for ( int k=j+1; k<m_pDiskChunks.GetLength() && bKeep; k++ )
						{
							const CSphIndex * pIndex = m_pDiskChunks[k];
							bKeep = ( sphBinarySearch ( pIndex->GetKillList(), pIndex->GetKillList() + pIndex->GetKillListSize() - 1, uRef )==NULL );
						}

						// might be last disk chunk that saving now
						if ( bKeep && m_iDoubleBuffer )
							bKeep = ( sphBinarySearch ( m_dDiskChunkKlist.Begin(), m_dDiskChunkKlist.Begin() + m_dDiskChunkKlist.GetLength() - 1, uRef )==NULL );
					}
				}
			}

			if ( bRamKilled || bKeep )
				iTotalKilled++;

			if ( bDiskKilled || !bKeep )
			{
				Swap ( dAccKlist[i], dAccKlist[iDiskLiveKLen-1] );
				iDiskLiveKLen--;
				i--;
			}
		}

		// wipe out readers - now we are only using RAM segments
		m_tRwlock.WriteLock ();

		// update K-lists on survivors
		ARRAY_FOREACH ( iSeg, dSegments )
		{
			RtSegment_t * pSeg = dSegments[iSeg];
			if ( !pSeg->m_bTlsKlist )
				continue; // should be fresh enough

			// this segment was not created by this txn
			// so we need to merge additional K-list from current txn into it
			CSphVector<SphDocID_t> dKlistAddon;
			ARRAY_FOREACH ( j, dAccKlist )
			{
				// tricky bit!
				// we can NOT append ids to segment k-list directly
				// because FindAliveRow() will binary search it
				// and it will expect a sorted list
				SphDocID_t uDocid = dAccKlist[j];
				if ( pSeg->FindAliveRow ( uDocid ) )
					dKlistAddon.Add ( uDocid );
			}

			// now actually update it
			if ( dKlistAddon.GetLength() )
			{
				// copy data, update counters
				ARRAY_FOREACH ( i, dKlistAddon )
					pSeg->m_dKlist.Add ( dKlistAddon[i] );

				pSeg->m_iAliveRows -= dKlistAddon.GetLength();
				assert ( pSeg->m_iAliveRows>=0 );

				// we did not check for existence in K-list, only in segment
				// so need to use Uniq(), not just Sort()
				pSeg->m_dKlist.Uniq ();
			}

			// mark as good
			pSeg->m_bTlsKlist = false;
		}

		// update disk K-list
		// after iDiskLiveKLen are ids already stored on disk - just skip them
		m_tKlist.Delete ( dAccKlist.Begin(), iDiskLiveKLen );

		// collect kill-list for new segments
		if ( m_iDoubleBuffer )
		{
			int iOff = m_dNewSegmentKlist.GetLength();
			m_dNewSegmentKlist.Resize ( iOff + iDiskLiveKLen );
			memcpy ( m_dNewSegmentKlist.Begin()+iOff, dAccKlist.Begin(), sizeof(SphDocID_t)*iDiskLiveKLen );
		}
	} else
	{
		// wipe out readers - now we are only using RAM segments
		m_tRwlock.WriteLock ();
	}

	ARRAY_FOREACH ( i, dSegments )
	{
		RtSegment_t * pSeg = dSegments[i];
		if ( pSeg->m_iAliveRows==0 )
		{
			dToKill.Add ( pSeg );
			dSegments.RemoveFast ( i );
			i--;
		}
	}

	// go live!
	// got rid of 'old' double-buffer segments then add 'new' onces
	m_pSegments.Resize ( m_iDoubleBuffer );
	ARRAY_FOREACH ( i, dSegments )
		m_pSegments.Add ( dSegments[i] );

	// update stats
	m_tStats.m_iTotalDocuments += iNewDocs - iTotalKilled;

	// get flag of double-buffer prior mutex unlock
	bool bDoubleBufferActive = ( m_iDoubleBuffer>0 );

	// phase 3, enable readers again
	// we might need to dump data to disk now
	// but during the dump, readers can still use RAM chunk data
	Verify ( m_tRwlock.Unlock() );

	// we can kill retired segments now
	ARRAY_FOREACH ( i, dToKill )
		SafeDelete ( dToKill[i] );

	Verify ( m_tSaveInnerMutex.Unlock() );

	// double buffer writer stands still till save done
	// all writers waiting double buffer done
	CSphVector<RtSegment_t *> dSegments2Dump;
	CSphSourceStats tStat2Dump;
	if ( bDump )
	{
		Verify ( m_tSaveOuterMutex.Lock() );

		// wait writer that saved data
		if ( bDoubleBufferActive )
		{
			bDump = false;
			m_tSaveOuterMutex.Unlock();
		} else
		{
			// copy stats for disk chunk
			dSegments2Dump = m_pSegments;
			tStat2Dump = m_tStats;
			m_iDoubleBuffer = m_pSegments.GetLength();

			m_tKlist.Flush();
			m_dDiskChunkKlist.Reset ( m_tKlist.GetKillListSize() );
			if ( m_tKlist.GetKillListSize() )
			{
				memcpy ( m_dDiskChunkKlist.Begin(), m_tKlist.GetKillList(), sizeof(SphAttr_t)*m_tKlist.GetKillListSize() );
			}
		}
	}

	// all done, enable other writers
	Verify ( m_tWriterMutex.Unlock() );

	if ( bDump )
	{
		SaveDiskChunk ( iTID, dSegments2Dump, tStat2Dump );
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), iTID, false );

		m_tSaveOuterMutex.Unlock();
	}
}

void RtIndex_t::RollBack ()
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = AcquireAccum();
	if ( !pAcc )
		return;

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );

	// finish cleaning up and release accumulator
	pAcc->m_pIndex = NULL;
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
}

bool RtIndex_t::DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	RtAccum_t * pAcc = AcquireAccum ( &sError );
	if ( !pAcc )
		return false;

	if ( !iDocs )
		return true;

	assert ( pDocs && iDocs );

	// !COMMIT should handle case when uDoc what inserted in current txn here
	while ( iDocs-- )
		pAcc->m_dAccumKlist.Add ( *pDocs++ );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// LOAD/SAVE
//////////////////////////////////////////////////////////////////////////

struct Checkpoint_t
{
	uint64_t m_uWord;
	uint64_t m_uOffset;
};


void RtIndex_t::ForceDiskChunk ()
{
	MEMORY ( SPH_MEM_IDX_RT );

	if ( !m_pSegments.GetLength() )
		return;

	Verify ( m_tWriterMutex.Lock() );
	Verify ( m_tSaveOuterMutex.Lock() );

	m_tKlist.Flush();
	m_dDiskChunkKlist.Reset ( m_tKlist.GetKillListSize() );
	if ( m_tKlist.GetKillListSize() )
	{
		memcpy ( m_dDiskChunkKlist.Begin(), m_tKlist.GetKillList(), sizeof(SphAttr_t)*m_tKlist.GetKillListSize() );
	}
	SaveDiskChunk ( m_iTID, m_pSegments, m_tStats );

	Verify ( m_tSaveOuterMutex.Unlock() );
	Verify ( m_tWriterMutex.Unlock() );
}


// Here is the devil of saving id32 chunk from id64 binary daemon
template < typename DOCID, typename WORDID >
void RtIndex_t::SaveDiskDataImpl ( const char * sFilename, const CSphVector<RtSegment_t *> & dSegments,
	const CSphSourceStats & tStats ) const
{
	typedef RtDoc_T<DOCID> RTDOC;
	typedef RtWord_T<WORDID> RTWORD;

	CSphString sName, sError; // FIXME!!! report collected (sError) errors

	CSphWriter wrHits, wrDocs, wrDict, wrRows, wrSkips;
	sName.SetSprintf ( "%s.spp", sFilename ); wrHits.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spd", sFilename ); wrDocs.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spi", sFilename ); wrDict.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spa", sFilename ); wrRows.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spe", sFilename ); wrSkips.OpenFile ( sName.cstr(), sError );


	wrDict.PutByte ( 1 );
	wrDocs.PutByte ( 1 );
	wrHits.PutByte ( 1 );
	wrSkips.PutByte ( 1 );

	// we don't have enough RAM to create new merged segments
	// and have to do N-way merge kinda in-place
	CSphVector<RtWordReader_T<WORDID>*> pWordReaders;
	CSphVector<RtDocReader_T<DOCID>*> pDocReaders;
	CSphVector<RtSegment_t*> pSegments;
	CSphVector<const RTWORD*> pWords;
	CSphVector<const RTDOC*> pDocs;

	pWordReaders.Reserve ( dSegments.GetLength() );
	pDocReaders.Reserve ( dSegments.GetLength() );
	pSegments.Reserve ( dSegments.GetLength() );
	pWords.Reserve ( dSegments.GetLength() );
	pDocs.Reserve ( dSegments.GetLength() );

	////////////////////
	// write attributes
	////////////////////

	// the new, template-param aligned iStride instead of index-wide
	int iStride = DWSIZEOF(DOCID) + m_tSchema.GetRowSize();
	CSphVector<RtRowIterator_T<DOCID>*> pRowIterators ( dSegments.GetLength() );
	ARRAY_FOREACH ( i, dSegments )
		pRowIterators[i] = new RtRowIterator_T<DOCID> ( dSegments[i], iStride, false, NULL );

	CSphVector<const CSphRowitem*> pRows ( dSegments.GetLength() );
	ARRAY_FOREACH ( i, pRowIterators )
		pRows[i] = pRowIterators[i]->GetNextAliveRow();

	// prepare to build min-max index for attributes too
	int iTotalDocs = 0;
	ARRAY_FOREACH ( i, dSegments )
		iTotalDocs += dSegments[i]->m_iAliveRows;

	AttrIndexBuilder_t<DOCID> tMinMaxBuilder ( m_tSchema );
	CSphVector<DWORD> dMinMaxBuffer ( tMinMaxBuilder.GetExpectedSize ( iTotalDocs ) );
	tMinMaxBuilder.Prepare ( dMinMaxBuffer.Begin(), dMinMaxBuffer.Begin() + dMinMaxBuffer.GetLength() );

	sName.SetSprintf ( "%s.sps", sFilename );
	CSphWriter tStrWriter;
	tStrWriter.OpenFile ( sName.cstr(), sError );
	tStrWriter.PutByte ( 0 ); // dummy byte, to reserve magic zero offset

	sName.SetSprintf ( "%s.spm", sFilename );
	CSphWriter tMvaWriter;
	tMvaWriter.OpenFile ( sName.cstr(), sError );
	tMvaWriter.PutDword ( 0 ); // dummy dword, to reserve magic zero offset

	DOCID iMinDocID = DOCID_MAX;
	CSphRowitem * pFixedRow = new CSphRowitem[iStride];

#ifndef NDEBUG
	int iStoredDocs = 0;
#endif

	StorageStringWriter_t tStorageString ( m_tSchema, tStrWriter );
	StorageMvaWriter_t tStorageMva ( m_tSchema, tMvaWriter );

	for ( ;; )
	{
		// find min row
		int iMinRow = -1;
		ARRAY_FOREACH ( i, pRows )
			if ( pRows[i] )
				if ( iMinRow<0 || DOCINFO2ID_T<DOCID> ( pRows[i] ) < DOCINFO2ID_T<DOCID> ( pRows[iMinRow] ) )
					iMinRow = i;
		if ( iMinRow<0 )
			break;

#ifndef NDEBUG
		// verify that it's unique
		int iDupes = 0;
		ARRAY_FOREACH ( i, pRows )
			if ( pRows[i] )
				if ( DOCINFO2ID_T<DOCID> ( pRows[i] )==DOCINFO2ID_T<DOCID> ( pRows[iMinRow] ) )
					iDupes++;
		assert ( iDupes==1 );
#endif

		const CSphRowitem * pRow = pRows[iMinRow];

		// strings storage for stored row
		assert ( iMinRow<dSegments.GetLength() );
		const RtSegment_t * pSegment = dSegments[iMinRow];

#ifdef PARANOID // sanity check in PARANOID mode
		VerifyEmptyStrings<DOCID> ( pSegment->m_dStrings, m_tSchema, pRow );
#endif

		// collect min-max data
		Verify ( tMinMaxBuilder.Collect ( pRow, pSegment->m_dMvas.Begin(), pSegment->m_dMvas.GetLength(), sError, false ) );

		if ( iMinDocID==DOCID_MAX )
			iMinDocID = DOCINFO2ID_T<DOCID> ( pRows[iMinRow] );

		if ( pSegment->m_dStrings.GetLength()>1 || pSegment->m_dMvas.GetLength()>1 ) // should be more then dummy zero elements
		{
			// copy row content as we'll fix up its attrs ( string offset for now )
			memcpy ( pFixedRow, pRow, iStride*sizeof(CSphRowitem) );
			pRow = pFixedRow;

			CopyFixupStorageAttrs<DOCID> ( pSegment->m_dStrings, tStorageString, pFixedRow );
			CopyFixupStorageAttrs<DOCID> ( pSegment->m_dMvas, tStorageMva, pFixedRow );
		}

		// emit it
		wrRows.PutBytes ( pRow, iStride*sizeof(CSphRowitem) );

		// fast forward
		pRows[iMinRow] = pRowIterators[iMinRow]->GetNextAliveRow();
#ifndef NDEBUG
		iStoredDocs++;
#endif
	}

	SafeDeleteArray ( pFixedRow );

	assert ( iStoredDocs==iTotalDocs );

	tMinMaxBuilder.FinishCollect ();
	if ( tMinMaxBuilder.GetActualSize() )
		wrRows.PutBytes ( dMinMaxBuffer.Begin(), sizeof(DWORD) * tMinMaxBuilder.GetActualSize() );

	tMvaWriter.CloseFile();
	tStrWriter.CloseFile ();

	////////////////////
	// write docs & hits
	////////////////////

	assert ( iMinDocID>0 );
	iMinDocID--;

	// OPTIMIZE? somehow avoid new on iterators maybe?
	ARRAY_FOREACH ( i, dSegments )
		pWordReaders.Add ( new RtWordReader_T<WORDID> ( dSegments[i], m_bKeywordDict, m_iWordsCheckpoint ) );

	ARRAY_FOREACH ( i, pWordReaders )
		pWords.Add ( pWordReaders[i]->UnzipWord() );

	// loop keywords
	CSphVector<Checkpoint_t> dCheckpoints;
	CSphVector<BYTE> dKeywordCheckpoints;
	int iWords = 0;
	CSphKeywordDeltaWriter tLastWord;
	WORDID uLastWordID = 0;
	SphOffset_t uLastDocpos = 0;
	CSphVector<SkiplistEntry_t> dSkiplist;

	CSphScopedPtr<ISphInfixBuilder> pInfixer ( NULL );
	if ( m_tSettings.m_iMinInfixLen && m_pDict->GetSettings().m_bWordDict )
		pInfixer = sphCreateInfixBuilder ( m_pTokenizer->GetMaxCodepointLength(), &sError );

	for ( ;; )
	{
		// find keyword with min id
		const RTWORD * pWord = NULL;
		ARRAY_FOREACH ( i, pWords ) // OPTIMIZE? PQ or at least nulls removal here?!
		{
			if ( pWords[i] )
			{
				if ( !pWord
					|| ( !m_bKeywordDict && pWords[i]->m_uWordID<pWord->m_uWordID )
					|| ( m_bKeywordDict &&
						sphDictCmpStrictly ( (const char *)pWords[i]->m_sWord+1, *pWords[i]->m_sWord, (const char *)pWord->m_sWord+1, *pWord->m_sWord )<0 ) )
				{
					pWord = pWords[i];
				}
			}
		}

		if ( !pWord )
			break;

		// loop all segments that have this keyword
		assert ( pSegments.GetLength()==0 );
		assert ( pDocReaders.GetLength()==0 );
		assert ( pDocs.GetLength()==0 );

		ARRAY_FOREACH ( i, pWords )
			if ( pWords[i] &&
				( ( !m_bKeywordDict && pWords[i]->m_uWordID==pWord->m_uWordID )
				|| ( m_bKeywordDict &&
				sphDictCmpStrictly ( (const char *)pWords[i]->m_sWord+1, *pWords[i]->m_sWord, (const char *)pWord->m_sWord+1, *pWord->m_sWord )==0 ) ) )
		{
			pSegments.Add ( dSegments[i] );
			pDocReaders.Add ( new RtDocReader_T<DOCID> ( dSegments[i], *pWords[i] ) );

			const RTDOC * pDoc = pDocReaders.Last()->UnzipDoc();
			while ( pDoc && dSegments[i]->m_dKlist.BinarySearch ( pDoc->m_uDocID ) )
				pDoc = pDocReaders.Last()->UnzipDoc();

			pDocs.Add ( pDoc );
		}

		// loop documents
		SphOffset_t uDocpos = wrDocs.GetPos();
		DOCID uLastDoc = 0;
		SphOffset_t uLastHitpos = 0;
		SphDocID_t uSkiplistDocID = iMinDocID;
		int iDocs = 0;
		int iHits = 0;
		dSkiplist.Resize ( 0 );
		for ( ;; )
		{
			// find alive doc with min id
			int iMinReader = -1;
			ARRAY_FOREACH ( i, pDocs ) // OPTIMIZE?
			{
				if ( !pDocs[i] )
					continue;

				assert ( !pSegments[i]->m_dKlist.BinarySearch ( pDocs[i]->m_uDocID ) );
				if ( iMinReader<0 || pDocs[i]->m_uDocID < pDocs[iMinReader]->m_uDocID )
					iMinReader = i;
			}
			if ( iMinReader<0 )
				break;

			// write doclist entry
			const RTDOC * pDoc = pDocs[iMinReader]; // shortcut
			// build skiplist, aka save decoder state as needed
			if ( ( iDocs & ( SPH_SKIPLIST_BLOCK-1 ) )==0 )
			{
				SkiplistEntry_t & t = dSkiplist.Add();
				t.m_iBaseDocid = uSkiplistDocID;
				t.m_iOffset = wrDocs.GetPos();
				t.m_iBaseHitlistPos = uLastHitpos;
			}
			iDocs++;
			iHits += pDoc->m_uHits;
			uSkiplistDocID = pDoc->m_uDocID;

			wrDocs.ZipOffset ( pDoc->m_uDocID - uLastDoc - iMinDocID );
			wrDocs.ZipInt ( pDoc->m_uHits );
			if ( pDoc->m_uHits==1 )
			{
				wrDocs.ZipInt ( pDoc->m_uHit & 0x7FFFFFUL );
				wrDocs.ZipInt ( pDoc->m_uHit >> 23 );
			} else
			{
				wrDocs.ZipInt ( pDoc->m_uDocFields );
				wrDocs.ZipOffset ( wrHits.GetPos() - uLastHitpos );
				uLastHitpos = wrHits.GetPos();
			}

			uLastDoc = pDoc->m_uDocID - iMinDocID;

			// loop hits from most current segment
			if ( pDoc->m_uHits>1 )
			{
				DWORD uLastHit = 0;
				RtHitReader_t tInHit ( pSegments[iMinReader], pDoc );
				for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
				{
					wrHits.ZipInt ( uValue - uLastHit );
					uLastHit = uValue;
				}
				wrHits.ZipInt ( 0 );
			}

			// fast forward readers
			DOCID uMinID = pDocs[iMinReader]->m_uDocID;
			ARRAY_FOREACH ( i, pDocs )
				while ( pDocs[i] && ( pDocs[i]->m_uDocID<=uMinID || pSegments[i]->m_dKlist.BinarySearch ( pDocs[i]->m_uDocID ) ) )
					pDocs[i] = pDocReaders[i]->UnzipDoc();
		}

		// write skiplist
		int iSkiplistOff = (int)wrSkips.GetPos();
		for ( int i=1; i<dSkiplist.GetLength(); i++ )
		{
			const SkiplistEntry_t & tPrev = dSkiplist[i-1];
			const SkiplistEntry_t & tCur = dSkiplist[i];
			assert ( tCur.m_iBaseDocid - tPrev.m_iBaseDocid>=SPH_SKIPLIST_BLOCK );
			assert ( tCur.m_iOffset - tPrev.m_iOffset>=4*SPH_SKIPLIST_BLOCK );
			wrSkips.ZipOffset ( tCur.m_iBaseDocid - tPrev.m_iBaseDocid - SPH_SKIPLIST_BLOCK );
			wrSkips.ZipOffset ( tCur.m_iOffset - tPrev.m_iOffset - 4*SPH_SKIPLIST_BLOCK );
			wrSkips.ZipOffset ( tCur.m_iBaseHitlistPos - tPrev.m_iBaseHitlistPos );
		}

		// write dict entry if necessary
		if ( wrDocs.GetPos()!=uDocpos )
		{
			wrDocs.ZipInt ( 0 ); // docs over

			if ( ( iWords%SPH_WORDLIST_CHECKPOINT )==0 )
			{
				if ( iWords )
				{
					SphOffset_t uOff = m_bKeywordDict ? 0 : uDocpos - uLastDocpos;
					wrDict.ZipInt ( 0 );
					wrDict.ZipOffset ( uOff ); // store last hitlist length
				}

				// restart delta coding, once per SPH_WORDLIST_CHECKPOINT entries
				uLastDocpos = 0;
				uLastWordID = 0;
				tLastWord.Reset();

				// begin new wordlist entry
				Checkpoint_t & tChk = dCheckpoints.Add ();
				tChk.m_uOffset = wrDict.GetPos();
				if ( m_bKeywordDict )
				{
					// copy word len + word itself to checkpoint storage
					tChk.m_uWord = sphPutBytes ( &dKeywordCheckpoints, pWord->m_sWord, pWord->m_sWord[0]+1 );
				} else
				{
					tChk.m_uWord = pWord->m_uWordID;
				}
			}
			iWords++;

			if ( m_bKeywordDict )
			{
				tLastWord.PutDelta ( wrDict, pWord->m_sWord+1, pWord->m_sWord[0] );
				wrDict.ZipOffset ( uDocpos );
			} else
			{
				assert ( pWord->m_uWordID!=uLastWordID );
				wrDict.ZipOffset ( pWord->m_uWordID - uLastWordID );
				uLastWordID = pWord->m_uWordID;
				assert ( uDocpos>uLastDocpos );
				wrDict.ZipOffset ( uDocpos - uLastDocpos );
			}
			wrDict.ZipInt ( iDocs );
			wrDict.ZipInt ( iHits );
			if ( m_bKeywordDict )
			{
				BYTE uHint = sphDoclistHintPack ( iDocs, wrDocs.GetPos()-uLastDocpos );
				if ( uHint )
					wrDict.PutByte ( uHint );

				// build infixes
				if ( pInfixer.Ptr() )
					pInfixer->AddWord ( pWord->m_sWord+1, pWord->m_sWord[0], dCheckpoints.GetLength() );
			}

			// emit skiplist pointer
			if ( iDocs>SPH_SKIPLIST_BLOCK )
				wrDict.ZipInt ( iSkiplistOff );

			uLastDocpos = uDocpos;
		}

		// move words forward
		// because pWord contents will move forward too!
		WORDID uMinID = pWord->m_uWordID;
		char sMinWord[SPH_MAX_KEYWORD_LEN];
		int iMinWordLen = 0;
		if ( m_bKeywordDict )
		{
			iMinWordLen = pWord->m_sWord[0];
			assert ( iMinWordLen<SPH_MAX_KEYWORD_LEN );
			memcpy ( sMinWord, pWord->m_sWord+1, iMinWordLen );
		}

		ARRAY_FOREACH ( i, pWords )
		{
			if ( pWords[i] &&
				( ( !m_bKeywordDict && pWords[i]->m_uWordID==uMinID )
				|| ( m_bKeywordDict && sphDictCmpStrictly ( (const char *)pWords[i]->m_sWord+1, pWords[i]->m_sWord[0], sMinWord, iMinWordLen )==0 ) ) )
			{
				pWords[i] = pWordReaders[i]->UnzipWord();
			}
		}

		// cleanup
		ARRAY_FOREACH ( i, pDocReaders )
			SafeDelete ( pDocReaders[i] );
		pSegments.Resize ( 0 );
		pDocReaders.Resize ( 0 );
		pDocs.Resize ( 0 );
	}

	// write checkpoints
	SphOffset_t uOff = m_bKeywordDict ? 0 : wrDocs.GetPos() - uLastDocpos;
	// FIXME!!! don't write to wrDict if iWords==0
	// however plain index becomes m_bIsEmpty and full scan does not work there
	// we'll get partly working RT ( RAM chunk works and disk chunks give empty result set )
	wrDict.ZipInt ( 0 ); // indicate checkpoint
	wrDict.ZipOffset ( uOff ); // store last doclist length

	// flush infix hash entries, if any
	if ( pInfixer.Ptr() )
		pInfixer->SaveEntries ( wrDict );

	SphOffset_t iCheckpointsPosition = wrDict.GetPos();
	if ( m_bKeywordDict )
	{
		const char * pCheckpoints = (const char *)dKeywordCheckpoints.Begin();
		ARRAY_FOREACH ( i, dCheckpoints )
		{
			const char * pPacked = pCheckpoints + dCheckpoints[i].m_uWord;
			int iLen = *pPacked;
			assert ( iLen && (int)dCheckpoints[i].m_uWord+1+iLen<=dKeywordCheckpoints.GetLength() );
			wrDict.PutDword ( iLen );
			wrDict.PutBytes ( pPacked+1, iLen );
			wrDict.PutOffset ( dCheckpoints[i].m_uOffset );
		}
	} else
	{
		ARRAY_FOREACH ( i, dCheckpoints )
		{
			wrDict.PutOffset ( dCheckpoints[i].m_uWord );
			wrDict.PutOffset ( dCheckpoints[i].m_uOffset );
		}
	}

	int iInfixBlockOffset = 0;
	int iInfixCheckpointWordsSize = 0;
	// flush infix hash blocks
	if ( pInfixer.Ptr() )
	{
		iInfixBlockOffset = pInfixer->SaveEntryBlocks ( wrDict );
		iInfixCheckpointWordsSize = pInfixer->GetBlocksWordsSize();
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	wrDict.PutBytes ( "dict-header", 11 );
	wrDict.ZipInt ( dCheckpoints.GetLength() );
	wrDict.ZipOffset ( iCheckpointsPosition );
	wrDict.ZipInt ( m_pTokenizer->GetMaxCodepointLength() );
	wrDict.ZipInt ( iInfixBlockOffset );

	// write dummy kill-list files
	CSphWriter wrDummy;
	// dump killlist
	sName.SetSprintf ( "%s.spk", sFilename );
	wrDummy.OpenFile ( sName.cstr(), sError );
	if ( m_dDiskChunkKlist.GetLength() )
		wrDummy.PutBytes ( m_dDiskChunkKlist.Begin(), m_dDiskChunkKlist.GetLength()*sizeof ( SphAttr_t ) );
	wrDummy.CloseFile ();

	uint64_t uMinMax = (uint64_t)iTotalDocs * iStride;
	// header
	SaveDiskHeader ( sFilename, iMinDocID, dCheckpoints.GetLength(), iCheckpointsPosition, iInfixBlockOffset, iInfixCheckpointWordsSize,
		m_dDiskChunkKlist.GetLength(), uMinMax, tStats, m_bId32to64 );

	// cleanup
	ARRAY_FOREACH ( i, pWordReaders )
		SafeDelete ( pWordReaders[i] );
	ARRAY_FOREACH ( i, pDocReaders )
		SafeDelete ( pDocReaders[i] );
	ARRAY_FOREACH ( i, pRowIterators )
		SafeDelete ( pRowIterators[i] );

	// done
	wrSkips.CloseFile ();
	wrHits.CloseFile ();
	wrDocs.CloseFile ();
	wrDict.CloseFile ();
	wrRows.CloseFile ();
}


void RtIndex_t::SaveDiskData ( const char * sFilename, const CSphVector<RtSegment_t *> & dSegments, const CSphSourceStats & tStats ) const
{
	if ( m_bId32to64 )
		return SaveDiskDataImpl<DWORD,DWORD> ( sFilename, dSegments, tStats );
	else
		return SaveDiskDataImpl<SphDocID_t,SphWordID_t> ( sFilename, dSegments, tStats );
}


template < typename DOCID >
void RtIndex_t::SaveDiskHeader ( const char * sFilename, DOCID iMinDocID, int iCheckpoints,
	SphOffset_t iCheckpointsPosition, int iInfixBlocksOffset, int iInfixCheckpointWordsSize, DWORD uKillListSize, uint64_t uMinMaxSize,
	const CSphSourceStats & tStats, bool bForceID32 ) const
{
	static const DWORD INDEX_MAGIC_HEADER	= 0x58485053;	///< my magic 'SPHX' header
	static const DWORD INDEX_FORMAT_VERSION	= 36;			///< my format version

	CSphWriter tWriter;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.sph", sFilename );
	tWriter.OpenFile ( sName.cstr(), sError );

	// format
	tWriter.PutDword ( INDEX_MAGIC_HEADER );
	tWriter.PutDword ( INDEX_FORMAT_VERSION );

	if ( bForceID32 )
		tWriter.PutDword ( 0 ); // use-32bit
	else
		tWriter.PutDword ( USE_64BIT ); // use-64bit
	tWriter.PutDword ( SPH_DOCINFO_EXTERN );

	// schema
	WriteSchema ( tWriter, m_tSchema );

	// min docid
	tWriter.PutOffset ( iMinDocID );

	// wordlist checkpoints
	tWriter.PutOffset ( iCheckpointsPosition );
	tWriter.PutDword ( iCheckpoints );

	int iInfixCodepointBytes = ( m_tSettings.m_iMinInfixLen && m_pDict->GetSettings().m_bWordDict ? m_pTokenizer->GetMaxCodepointLength() : 0 );
	tWriter.PutByte ( iInfixCodepointBytes ); // m_iInfixCodepointBytes, v.27+
	tWriter.PutDword ( iInfixBlocksOffset ); // m_iInfixBlocksOffset, v.27+
	tWriter.PutDword ( iInfixCheckpointWordsSize ); // m_iInfixCheckpointWordsSize, v.34+

	// stats
	tWriter.PutDword ( (DWORD)tStats.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	tWriter.PutOffset ( tStats.m_iTotalBytes );

	// index settings
	tWriter.PutDword ( m_tSettings.m_iMinPrefixLen );
	tWriter.PutDword ( m_tSettings.m_iMinInfixLen );
	tWriter.PutByte ( m_tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( m_tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( m_tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( m_tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( m_tSettings.m_eHitless );
	tWriter.PutDword ( SPH_HIT_FORMAT_INLINE );
	tWriter.PutByte ( m_tSettings.m_bIndexSP ? 1 : 0 ); // m_bIndexSP, v.21+
	tWriter.PutString ( m_tSettings.m_sZones ); // m_sZonePrefix, v.22+
	tWriter.PutDword ( 0 ); // m_iBoundaryStep, v.23+
	tWriter.PutDword ( 1 ); // m_iStopwordStep, v.23+
	tWriter.PutDword ( 1 );	// m_iOvershortStep
	tWriter.PutDword ( m_tSettings.m_iEmbeddedLimit );	// v.30+
	tWriter.PutByte ( m_tSettings.m_eBigramIndex ); // v.32+
	tWriter.PutString ( m_tSettings.m_sBigramWords ); // v.32+
	tWriter.PutByte ( m_tSettings.m_bIndexFieldLens ); // v. 35+

	// tokenizer
	SaveTokenizerSettings ( tWriter, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );

	// dictionary
	SaveDictionarySettings ( tWriter, m_pDict, m_bKeywordDict, m_tSettings.m_iEmbeddedLimit );

	// kill-list size
	tWriter.PutDword ( uKillListSize );

	// min-max count
	tWriter.PutOffset ( uMinMaxSize );

	// field filter
	SaveFieldFilterSettings ( tWriter, m_pFieldFilter );

	// done
	tWriter.CloseFile ();
}


#if USE_WINDOWS
#undef rename
int rename ( const char * sOld, const char * sNew )
{
	if ( MoveFileEx ( sOld, sNew, MOVEFILE_REPLACE_EXISTING ) )
		return 0;
	errno = GetLastError();
	return -1;
}
#endif


void RtIndex_t::SaveMeta ( int iDiskChunks, int64_t iTID )
{
	// sanity check
	if ( m_iLockFD<0 )
		return;

	// write new meta
	CSphString sMeta, sMetaNew;
	sMeta.SetSprintf ( "%s.meta", m_sPath.cstr() );
	sMetaNew.SetSprintf ( "%s.meta.new", m_sPath.cstr() );

	CSphString sError;
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMetaNew, sError ) )
		sphDie ( "failed to serialize meta: %s", sError.cstr() ); // !COMMIT handle this gracefully
	wrMeta.PutDword ( META_HEADER_MAGIC );
	wrMeta.PutDword ( META_VERSION );
	wrMeta.PutDword ( iDiskChunks );
	wrMeta.PutDword ( m_iDiskBase );
	wrMeta.PutDword ( (DWORD)m_tStats.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	wrMeta.PutOffset ( m_tStats.m_iTotalBytes ); // FIXME? need PutQword ideally
	wrMeta.PutOffset ( iTID );

	// meta v.4, save disk index format and settings, too
	wrMeta.PutDword ( INDEX_FORMAT_VERSION );
	WriteSchema ( wrMeta, m_tSchema );
	SaveIndexSettings ( wrMeta, m_tSettings );
	SaveTokenizerSettings ( wrMeta, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );
	SaveDictionarySettings ( wrMeta, m_pDict, m_bKeywordDict, m_tSettings.m_iEmbeddedLimit );

	// meta v.5
	wrMeta.PutDword ( m_iWordsCheckpoint );

	// meta v.7
	wrMeta.PutDword ( m_iMaxCodepointLength );
	wrMeta.PutByte ( BLOOM_PER_ENTRY_VALS_COUNT );
	wrMeta.PutByte ( BLOOM_HASHES_COUNT );

	wrMeta.CloseFile(); // FIXME? handle errors?

	// rename
	if ( ::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMetaNew.cstr(), sMeta.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully
}


void RtIndex_t::SaveDiskChunk ( int64_t iTID, const CSphVector<RtSegment_t *> & dSegments, const CSphSourceStats & tStats )
{
	if ( !dSegments.GetLength() )
		return;

	MEMORY ( SPH_MEM_IDX_RT );

	// dump it
	CSphString sNewChunk;
	sNewChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), m_pDiskChunks.GetLength()+m_iDiskBase );
	SaveDiskData ( sNewChunk.cstr(), dSegments, tStats );

	// bring new disk chunk online
	CSphIndex * pDiskChunk = LoadDiskChunk ( sNewChunk.cstr(), m_sLastError );
	if ( !pDiskChunk )
		sphDie ( "%s", m_sLastError.cstr() );

	// save updated meta
	SaveMeta ( m_pDiskChunks.GetLength()+1, iTID );

	// FIXME! add binlog cleanup here once we have binlogs

	// get exclusive lock again, gotta reset RAM chunk now
	Verify ( m_tSaveInnerMutex.Lock() );
	Verify ( m_tRwlock.WriteLock() );

	int iOldSegmentsCount = ( m_iDoubleBuffer ? m_iDoubleBuffer : m_pSegments.GetLength() );
	for ( int i=0; i<iOldSegmentsCount; i++ )
		SafeDelete ( m_pSegments[i] );

	// swap double buffer data
	int iNewSegmentsCount = ( m_iDoubleBuffer ? m_pSegments.GetLength() - m_iDoubleBuffer : 0 );
	for ( int i=0; i<iNewSegmentsCount; i++ )
		m_pSegments[i] = m_pSegments[i+m_iDoubleBuffer];
	m_pSegments.Resize ( iNewSegmentsCount );

	// clean up kill-list
	m_tKlist.Reset();
	m_tKlist.Delete ( m_dNewSegmentKlist.Begin(), m_dNewSegmentKlist.GetLength() );
	m_dNewSegmentKlist.Reset();
	m_dDiskChunkKlist.Reset ( 0 );

	m_iDoubleBuffer = 0;
	m_iSavedTID = iTID;
	m_iSavedRam = 0;
	m_tmSaved = sphMicroTimer();

	m_pDiskChunks.Add ( pDiskChunk );

	Verify ( m_tRwlock.Unlock() );
	Verify ( m_tSaveInnerMutex.Unlock() );
}


CSphIndex * RtIndex_t::LoadDiskChunk ( const char * sChunk, CSphString & sError ) const
{
	MEMORY ( SPH_MEM_IDX_DISK );

	// !COMMIT handle errors gracefully instead of dying
	CSphIndex * pDiskChunk = sphCreateIndexPhrase ( m_sIndexName.cstr(), sChunk );
	if ( !pDiskChunk )
	{
		sError.SetSprintf ( "disk chunk %s: alloc failed", sChunk );
		return NULL;
	}

	pDiskChunk->SetWordlistPreload ( m_bPreloadWordlist );
	pDiskChunk->m_iExpansionLimit = m_iExpansionLimit;
	pDiskChunk->SetEnableStar ( m_bEnableStar );

	CSphString sWarning;
	if ( !pDiskChunk->Prealloc ( false, m_bPathStripped, sWarning ) )
	{
		sError.SetSprintf ( "disk chunk %s: prealloc failed: %s", sChunk, pDiskChunk->GetLastError().cstr() );
		SafeDelete ( pDiskChunk );
		return NULL;
	}

	if ( !pDiskChunk->Preread() )
	{
		sError.SetSprintf ( "disk chunk %s: preread failed: %s", sChunk, pDiskChunk->GetLastError().cstr() );
		SafeDelete ( pDiskChunk );
		return NULL;
	}

	return pDiskChunk;
}


bool RtIndex_t::Prealloc ( bool, bool bStripPath, CSphString & )
{
	MEMORY ( SPH_MEM_IDX_RT );

	// locking uber alles
	// in RT backend case, we just must be multi-threaded
	// so we simply lock here, and ignore Lock/Unlock hassle caused by forks
	assert ( m_iLockFD<0 );

	CSphString sLock;
	sLock.SetSprintf ( "%s.lock", m_sPath.cstr() );
	m_iLockFD = ::open ( sLock.cstr(), SPH_O_NEW, 0644 );
	if ( m_iLockFD<0 )
	{
		m_sLastError.SetSprintf ( "failed to open %s: %s", sLock.cstr(), strerror(errno) );
		return false;
	}
	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock %s: %s", sLock.cstr(), strerror(errno) );
		::close ( m_iLockFD );
		return false;
	}

	/////////////
	// load meta
	/////////////

	// check if we have a meta file (kinda-header)
	CSphString sMeta;
	sMeta.SetSprintf ( "%s.meta", m_sPath.cstr() );

	// no readable meta? no disk part yet
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return true;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, m_sLastError ) )
		return false;

	if ( rdMeta.GetDword()!=META_HEADER_MAGIC )
	{
		m_sLastError.SetSprintf ( "invalid meta file %s", sMeta.cstr() );
		return false;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==0 || uVersion>META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sMeta.cstr(), uVersion, META_VERSION );
		return false;
	}
	const int iDiskChunks = rdMeta.GetDword();
	if ( uVersion>=6 )
		m_iDiskBase = rdMeta.GetDword();
	m_tStats.m_iTotalDocuments = rdMeta.GetDword();
	m_tStats.m_iTotalBytes = rdMeta.GetOffset();
	if ( uVersion>=2 )
		m_iTID = rdMeta.GetOffset();

	// tricky bit
	// we started saving settings into .meta from v.4 and up only
	// and those reuse disk format version, aka INDEX_FORMAT_VERSION
	// anyway, starting v.4, serialized settings take precedence over config
	// so different chunks can't have different settings any more
	if ( uVersion>=4 )
	{
		CSphTokenizerSettings tTokenizerSettings;
		CSphDictSettings tDictSettings;
		CSphEmbeddedFiles tEmbeddedFiles;
		CSphString sWarning;

		// load them settings
		DWORD uSettingsVer = rdMeta.GetDword();
		ReadSchema ( rdMeta, m_tSchema, uSettingsVer, false );
		LoadIndexSettings ( m_tSettings, rdMeta, uSettingsVer );
		LoadTokenizerSettings ( rdMeta, tTokenizerSettings, tEmbeddedFiles, uSettingsVer, sWarning );
		LoadDictionarySettings ( rdMeta, tDictSettings, tEmbeddedFiles, uSettingsVer, sWarning );

		// meta v.5 dictionary
		if ( uVersion>=5 )
			m_bKeywordDict = tDictSettings.m_bWordDict;

		// fixup them settings
		if ( m_bId32to64 )
			tDictSettings.m_bCrc32 = true;

		if ( bStripPath )
		{
			StripPath ( tTokenizerSettings.m_sSynonymsFile );
			StripPath ( tDictSettings.m_sStopwords );
			ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
				StripPath ( tDictSettings.m_dWordforms[i] );
		}

		// recreate tokenizer
		SafeDelete ( m_pTokenizer );
		m_pTokenizer = ISphTokenizer::Create ( tTokenizerSettings, &tEmbeddedFiles, m_sLastError );
		if ( !m_pTokenizer )
			return false;

		// !COMMIT implement support for multiforms, eh?
		// ISphTokenizer * pTokenFilter = ISphTokenizer::CreateTokenFilter ( pTokenizer, pDict->GetMultiWordforms () );
		// SetTokenizer ( pTokenFilter ? pTokenFilter : pTokenizer );

		// recreate dictionary
		SafeDelete ( m_pDict );
		m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, m_sIndexName.cstr() );
		if ( !m_pDict )
		{
			m_sLastError.SetSprintf ( "index '%s': unable to create dictionary", m_sIndexName.cstr() );
			return false;
		}

		// update schema
		m_iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	}

	// meta v.5 checkpoint freq
	m_iWordsCheckpoint = ( uVersion<5 ? RTDICT_CHECKPOINT_V3 : RTDICT_CHECKPOINT_V5 );
	if ( uVersion>=5 )
	{
		m_iWordsCheckpoint = rdMeta.GetDword();
	}

	// check that infixes definition changed - going to rebuild infixes
	bool bRebuildInfixes = false;
	if ( uVersion>=7 )
	{
		m_iMaxCodepointLength = rdMeta.GetDword();
		int iBloomKeyLen = rdMeta.GetByte();
		int iBloomHashesCount = rdMeta.GetByte();
		bRebuildInfixes = ( iBloomKeyLen!=BLOOM_PER_ENTRY_VALS_COUNT || iBloomHashesCount!=BLOOM_HASHES_COUNT );

		if ( bRebuildInfixes )
			sphWarning ( "infix definition changed (from len=%d, hashes=%d to len=%d, hashes=%d) - rebuilding...",
						(int)BLOOM_PER_ENTRY_VALS_COUNT, (int)BLOOM_HASHES_COUNT, iBloomKeyLen, iBloomHashesCount );
	}

	///////////////
	// load chunks
	///////////////

	m_bPathStripped = bStripPath;

	// load disk chunks, if any
	for ( int iChunk=0; iChunk<iDiskChunks; iChunk++ )
	{
		CSphString sChunk;
		sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunk+m_iDiskBase );
		CSphIndex * pIndex = LoadDiskChunk ( sChunk.cstr(), m_sLastError );
		if ( !pIndex )
			sphDie ( "%s", m_sLastError.cstr() );

		m_pDiskChunks.Add ( pIndex );

		// tricky bit
		// outgoing match schema on disk chunk should be identical to our internal (!) schema
		if ( !m_tSchema.CompareTo ( pIndex->GetMatchSchema(), m_sLastError ) )
			return false;
	}

	// load ram chunk
	bool bRamLoaded = LoadRamChunk ( uVersion, bRebuildInfixes );

	// set up values for on timer save
	m_iSavedTID = m_iTID;
	m_iSavedRam = GetUsedRam();
	m_tmSaved = sphMicroTimer();

	return bRamLoaded;
}


bool RtIndex_t::Preread ()
{
	// !COMMIT move disk chunks prereading here
	return true;
}

template < typename T > struct IsPodType { enum { Value = false }; };
template<> struct IsPodType<char> { enum { Value = true }; };
template<> struct IsPodType<BYTE> { enum { Value = true }; };
template<> struct IsPodType<int> { enum { Value = true }; };
template<> struct IsPodType<DWORD> { enum { Value = true }; };
template<> struct IsPodType<uint64_t> { enum { Value = true }; };
template<> struct IsPodType<float> { enum { Value = true }; };


template < typename T, typename P >
static void SaveVector ( CSphWriter & tWriter, const CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.PutDword ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static void LoadVector ( CSphReader & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( tReader.GetDword() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static void SaveVector ( BinlogWriter_c & tWriter, const CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.ZipValue ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static bool LoadVector ( BinlogReader_c & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( (int) tReader.UnzipValue() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
	return !tReader.GetErrorFlag();
}


bool RtIndex_t::SaveRamChunk ()
{
	MEMORY ( SPH_MEM_IDX_RT );

	CSphString sChunk, sNewChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	sNewChunk.SetSprintf ( "%s.ram.new", m_sPath.cstr() );
	m_tKlist.SaveToFile ( m_sPath.cstr() );

	CSphWriter wrChunk;
	if ( !wrChunk.OpenFile ( sNewChunk, m_sLastError ) )
		return false;

	wrChunk.PutDword ( USE_64BIT );
	wrChunk.PutDword ( RtSegment_t::m_iSegments );
	wrChunk.PutDword ( m_pSegments.GetLength() );

	// no locks here, because it's only intended to be called from dtor
	ARRAY_FOREACH ( iSeg, m_pSegments )
	{
		const RtSegment_t * pSeg = m_pSegments[iSeg];
		wrChunk.PutDword ( pSeg->m_iTag );
		SaveVector ( wrChunk, pSeg->m_dWords );
		if ( m_bKeywordDict )
		{
			SaveVector ( wrChunk, pSeg->m_dKeywordCheckpoints );
		}

		const char * pCheckpoints = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
		wrChunk.PutDword ( pSeg->m_dWordCheckpoints.GetLength() );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_iOffset );
			if ( m_bKeywordDict )
			{
				wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_sWord-pCheckpoints );
			} else
			{
				wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_iWordID );
			}
		}
		SaveVector ( wrChunk, pSeg->m_dDocs );
		SaveVector ( wrChunk, pSeg->m_dHits );
		wrChunk.PutDword ( pSeg->m_iRows );
		wrChunk.PutDword ( pSeg->m_iAliveRows );
		SaveVector ( wrChunk, pSeg->m_dRows );
		SaveVector ( wrChunk, pSeg->m_dKlist );
		SaveVector ( wrChunk, pSeg->m_dStrings );
		SaveVector ( wrChunk, pSeg->m_dMvas );

		// infixes
		SaveVector ( wrChunk, pSeg->m_dInfixFilterCP );
	}

	wrChunk.CloseFile();
	if ( wrChunk.IsError() )
		return false;

	// rename
	if ( ::rename ( sNewChunk.cstr(), sChunk.cstr() ) )
		sphDie ( "failed to rename ram chunk (src=%s, dst=%s, errno=%d, error=%s)",
			sNewChunk.cstr(), sChunk.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully

	return true;
}


bool RtIndex_t::LoadRamChunk ( DWORD uVersion, bool bRebuildInfixes )
{
	MEMORY ( SPH_MEM_IDX_RT );

	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );

	if ( !sphIsReadable ( sChunk.cstr(), &m_sLastError ) )
		return true;

	m_tKlist.LoadFromFile ( m_sPath.cstr() );

	CSphAutoreader rdChunk;
	if ( !rdChunk.Open ( sChunk, m_sLastError ) )
		return false;

	bool bId64 = ( rdChunk.GetDword()!=0 );
	if ( bId64!=USE_64BIT )
	{
#if USE_64BIT
// #if 0
		// TODO: may be do this param conditional and push it into the config?
		m_bId32to64 = true;
#else
		m_sLastError.SetSprintf ( "ram chunk dumped by %s binary; this binary is %s",
			bId64 ? "id64" : "id32",
			USE_64BIT ? "id64" : "id32" );
		return false;
#endif
	}

	int iSegmentSeq = rdChunk.GetDword();
	m_pSegments.Resize ( rdChunk.GetDword() ); // FIXME? sanitize

	ARRAY_FOREACH ( iSeg, m_pSegments )
	{
		RtSegment_t * pSeg = new RtSegment_t ();
		m_pSegments[iSeg] = pSeg;

		pSeg->m_iTag = rdChunk.GetDword ();
		LoadVector ( rdChunk, pSeg->m_dWords );
		if ( uVersion>=5 && m_bKeywordDict )
		{
			LoadVector ( rdChunk, pSeg->m_dKeywordCheckpoints );
		}

		const char * pCheckpoints = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
		pSeg->m_dWordCheckpoints.Resize ( rdChunk.GetDword() );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int)rdChunk.GetOffset();
			SphOffset_t uOff = rdChunk.GetOffset();
			if ( m_bKeywordDict )
			{
				pSeg->m_dWordCheckpoints[i].m_sWord = pCheckpoints + uOff;
			} else
			{
				pSeg->m_dWordCheckpoints[i].m_iWordID = (SphWordID_t)uOff;
			}
		}
		LoadVector ( rdChunk, pSeg->m_dDocs );
		LoadVector ( rdChunk, pSeg->m_dHits );
		pSeg->m_iRows = rdChunk.GetDword();
		pSeg->m_iAliveRows = rdChunk.GetDword();
		LoadVector ( rdChunk, pSeg->m_dRows );
		LoadVector ( rdChunk, pSeg->m_dKlist );
		LoadVector ( rdChunk, pSeg->m_dStrings );
		if ( uVersion>=3 )
			LoadVector ( rdChunk, pSeg->m_dMvas );

		// infixes
		if ( uVersion>=7 )
		{
			LoadVector ( rdChunk, pSeg->m_dInfixFilterCP );
			if ( bRebuildInfixes )
				BuildSegmentInfixes ( pSeg );
		}
	}

	RtSegment_t::m_iSegments = iSegmentSeq;
	if ( rdChunk.GetErrorFlag() )
		return false;
	return true;
}

void RtIndex_t::PostSetup()
{
	if ( m_bId32to64 )
	{
		m_tKlist.Flush();
		m_dDiskChunkKlist.Reset ( m_tKlist.GetKillListSize() );
		if ( m_tKlist.GetKillListSize() )
		{
			memcpy ( m_dDiskChunkKlist.Begin(), m_tKlist.GetKillList(), sizeof(SphAttr_t)*m_tKlist.GetKillListSize() );
		}
		SaveDiskChunk ( m_iTID, m_pSegments, m_tStats );
		// since the RAM chunk is just stored as id32, we are no more in compat mode
		m_bId32to64 = false;
	}

	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();

	// bigram filter
	if ( m_tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE && m_tSettings.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		m_pTokenizer->SetBuffer ( (BYTE*)m_tSettings.m_sBigramWords.cstr(), m_tSettings.m_sBigramWords.Length() );

		BYTE * pTok = NULL;
		while ( ( pTok = m_pTokenizer->GetToken() )!=NULL )
			m_tSettings.m_dBigramWords.Add() = (const char*)pTok;

		m_tSettings.m_dBigramWords.Sort();
	}
	// FIXME!!! handle error
	m_pTokenizerIndexing = m_pTokenizer->Clone ( false );
	ISphTokenizer * pIndexing = ISphTokenizer::CreateBigramFilter ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError );
	if ( pIndexing )
		m_pTokenizerIndexing = pIndexing;
}


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

int RtIndex_t::DebugCheck ( FILE * fp )
{
	const int FAILS_THRESH = 100;
	int iFails = 0;
	int iFailsPrinted = 0;
	int iFailsPlain = 0;

	int64_t tmCheck = sphMicroTimer();

	if ( m_iStride!=DOCINFO_IDSIZE+m_tSchema.GetRowSize() )
		LOC_FAIL(( fp, "wrong attribute stride (current=%d, should_be=%d)", m_iStride, DOCINFO_IDSIZE+m_tSchema.GetRowSize() ));

	if ( m_iSoftRamLimit<=0 )
		LOC_FAIL(( fp, "wrong RAM limit (current="INT64_FMT")", m_iSoftRamLimit ));

	if ( m_iLockFD<0 )
		LOC_FAIL(( fp, "index lock file id < 0" ));

	if ( m_iDiskBase<0 )
		LOC_FAIL(( fp, "disk chunk base < 0" ));

	if ( m_iTID<0 )
		LOC_FAIL(( fp, "index TID < 0 (current="INT64_FMT")", m_iTID ));

	if ( m_iSavedTID<0 )
		LOC_FAIL(( fp, "index saved TID < 0 (current="INT64_FMT")", m_iSavedTID ));

	if ( m_iTID<m_iSavedTID )
		LOC_FAIL(( fp, "index TID < index saved TID (current="INT64_FMT", saved="INT64_FMT")", m_iTID, m_iSavedTID ));

	if ( m_iWordsCheckpoint!=RTDICT_CHECKPOINT_V3 && m_iWordsCheckpoint!=RTDICT_CHECKPOINT_V5 )
		LOC_FAIL(( fp, "unexpected number of words per checkpoint (expected 1024 or 48, got %d)", m_iWordsCheckpoint ));

	ARRAY_FOREACH ( iSegment, m_pSegments )
	{
		fprintf ( fp, "checking RT segment %d(%d)...\n", iSegment, m_pSegments.GetLength() );

		if ( !m_pSegments[iSegment] )
		{
			LOC_FAIL(( fp, "missing RT segment (segment=%d)", iSegment ));
			continue;
		}

		RtSegment_t & tSegment = *m_pSegments[iSegment];
		if ( tSegment.m_bTlsKlist )
			LOC_FAIL(( fp, "TLS k-list flag on: index is being commited (segment=%d)", iSegment ));

		const BYTE * pCurWord = tSegment.m_dWords.Begin();
		const BYTE * pMaxWord = pCurWord+tSegment.m_dWords.GetLength();
		const BYTE * pCurDoc = tSegment.m_dDocs.Begin();
		const BYTE * pMaxDoc = pCurDoc+tSegment.m_dDocs.GetLength();
		const BYTE * pCurHit = tSegment.m_dHits.Begin();
		const BYTE * pMaxHit = pCurHit+tSegment.m_dHits.GetLength();

		if ( !pCurWord )
			LOC_FAIL(( fp, "empty dictionary detected (segment=%d)", iSegment ));

		if ( !pCurDoc )
			LOC_FAIL(( fp, "empty doclist detected (segment=%d)", iSegment ));

		CSphVector<RtWordCheckpoint_t> dRefCheckpoints;
		int nWordsRead = 0;
		int nCheckpointWords = 0;
		int iCheckpointOffset = 0;
		SphWordID_t uPrevWordID = 0;
		DWORD uPrevDocOffset = 0;
		DWORD uPrevHitOffset = 0;

		CSphVector<bool> dUsedKListEntries;
		dUsedKListEntries.Resize ( tSegment.m_dKlist.GetLength() );
		ARRAY_FOREACH ( i, dUsedKListEntries )
			dUsedKListEntries[i] = false;

		RtWord_t tWord;
		memset ( &tWord, 0, sizeof(tWord) );

		BYTE sWord[SPH_MAX_KEYWORD_LEN+2], sLastWord[SPH_MAX_KEYWORD_LEN+2];
		memset ( sWord, 0, sizeof(sWord) );
		memset ( sLastWord, 0, sizeof(sLastWord) );

		int iLastWordLen = 0, iWordLen = 0;

		while ( pCurWord && pCurWord<pMaxWord )
		{
			bool bCheckpoint = ++nCheckpointWords==m_iWordsCheckpoint;
			if ( bCheckpoint )
			{
				nCheckpointWords = 1;
				iCheckpointOffset = pCurWord - tSegment.m_dWords.Begin();
				tWord.m_uDoc = 0;
				if ( !m_bKeywordDict )
					tWord.m_uWordID = 0;
			}

			const BYTE * pIn = pCurWord;
			DWORD uDeltaDoc;
			if ( m_bKeywordDict )
			{
				BYTE iMatch, iDelta, uPacked;
				uPacked = *pIn++;

				if ( pIn>=pMaxWord )
				{
					LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));
					break;
				}

				if ( uPacked & 0x80 )
				{
					iDelta = ( ( uPacked>>4 ) & 7 ) + 1;
					iMatch = uPacked & 15;
				} else
				{
					iDelta = uPacked & 127;
					iMatch = *pIn++;
					if ( pIn>=pMaxWord )
					{
						LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));
						break;
					}

					if ( iDelta<=8 && iMatch<=15 )
					{
						sLastWord[sizeof(sLastWord)-1] = '\0';
						LOC_FAIL(( fp, "wrong word-delta (segment=%d, word=%d, last_word=%s, last_len=%d, match=%d, delta=%d)",
							iSegment, nWordsRead, sLastWord+1, iLastWordLen, iMatch, iDelta ));
					}
				}

				if ( iMatch+iDelta>=(int)sizeof(sWord)-2 || iMatch>iLastWordLen )
				{
					sLastWord[sizeof(sLastWord)-1] = '\0';
					LOC_FAIL(( fp, "wrong word-delta (segment=%d, word=%d, last_word=%s, last_len=%d, match=%d, delta=%d)",
						iSegment, nWordsRead, sLastWord+1, iLastWordLen, iMatch, iDelta ));

					pIn += iDelta;
					if ( pIn>=pMaxWord )
					{
						LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));
						break;
					}
				} else
				{
					iWordLen = iMatch+iDelta;
					sWord[0] = (BYTE)iWordLen;
					memcpy ( sWord+1+iMatch, pIn, iDelta );
					sWord[1+iWordLen] = 0;
					pIn += iDelta;
					if ( pIn>=pMaxWord )
					{
						LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));
						break;
					}
				}

				int iCalcWordLen = strlen ( (const char *)sWord+1 );
				if ( iWordLen!=iCalcWordLen )
				{
					sWord[sizeof(sWord)-1] = '\0';
					LOC_FAIL(( fp, "word length mismatch (segment=%d, word=%d, read_word=%s, read_len=%d, calc_len=%d)",
						iSegment, nWordsRead, sWord+1, iWordLen, iCalcWordLen ));
				}

				if ( !iWordLen )
					LOC_FAIL(( fp, "empty word in word list (segment=%d, word=%d)",	iSegment, nWordsRead ));

				const BYTE * pStr = sWord+1;
				const BYTE * pStringStart = pStr;
				while ( pStringStart-pStr < iWordLen )
				{
					if ( !*pStringStart )
					{
						CSphString sErrorStr;
						sErrorStr.SetBinary ( (const char*)pStr, iWordLen );
						LOC_FAIL(( fp, "embedded zero in a word list string (segment=%d, offset=%u, string=%s)",
							iSegment, (DWORD)(pStringStart-pStr), sErrorStr.cstr() ));
					}

					pStringStart++;
				}

				if ( iLastWordLen && iWordLen )
				{
					if ( sphDictCmpStrictly ( (const char *)sWord+1, iWordLen, (const char *)sLastWord+1, iLastWordLen )<=0 )
					{
						sWord[sizeof(sWord)-1] = '\0';
						sLastWord[sizeof(sLastWord)-1] = '\0';
						LOC_FAIL(( fp, "word order decreased (segment=%d, word=%d, read_word=%s, last_word=%s)",
							iSegment, nWordsRead, sWord+1, sLastWord+1 ));
					}
				}

				memcpy ( sLastWord, sWord, iWordLen+2 );
				iLastWordLen = iWordLen;
			} else
			{
				SphWordID_t uDeltaID;
				pIn = UnzipWordid ( &uDeltaID, pIn );
				if ( pIn>=pMaxWord )
					LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));

				tWord.m_uWordID += uDeltaID;

				if ( tWord.m_uWordID<=uPrevWordID )
				{
					LOC_FAIL(( fp, "wordid decreased (segment=%d, word=%d, wordid="UINT64_FMT", previd="UINT64_FMT")",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)uPrevWordID ));
				}

				uPrevWordID = tWord.m_uWordID;
			}

			pIn = UnzipDword ( &tWord.m_uDocs, pIn );
			if ( pIn>=pMaxWord )
			{
				sWord[sizeof(sWord)-1] = '\0';
				LOC_FAIL(( fp, "invalid docs/hits (segment=%d, word=%d, read_word=%s, docs=%u, hits=%u)", iSegment, nWordsRead,
					sWord+1, tWord.m_uDocs, tWord.m_uHits ));
			}

			pIn = UnzipDword ( &tWord.m_uHits, pIn );
			if ( pIn>=pMaxWord )
				LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));

			pIn = UnzipDword ( &uDeltaDoc, pIn );
			if ( pIn>pMaxWord )
				LOC_FAIL(( fp, "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead ));

			pCurWord = pIn;
			tWord.m_uDoc += uDeltaDoc;

			if ( !tWord.m_uDocs || !tWord.m_uHits || tWord.m_uHits<tWord.m_uDocs )
			{
				sWord[sizeof(sWord)-1] = '\0';
				LOC_FAIL(( fp, "invalid docs/hits (segment=%d, word=%d, read_wordid="UINT64_FMT
					", read_word=%s, docs=%u, hits=%u)",
					iSegment, nWordsRead, (uint64_t)tWord.m_uWordID,
					sWord+1, tWord.m_uDocs, tWord.m_uHits ));
			}

			if ( bCheckpoint )
			{
				RtWordCheckpoint_t & tCP = dRefCheckpoints.Add();
				tCP.m_iOffset = iCheckpointOffset;

				if ( m_bKeywordDict )
				{
					tCP.m_sWord = new char [sWord[0]+1];
					memcpy ( (void *)tCP.m_sWord, sWord+1, sWord[0]+1 );
				} else
					tCP.m_iWordID = tWord.m_uWordID;
			}

			sWord[sizeof(sWord)-1] = '\0';

			if ( uPrevDocOffset && tWord.m_uDoc<=uPrevDocOffset )
				LOC_FAIL(( fp, "doclist offset decreased (segment=%d, word=%d, "
					"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, prev_doclist_offset=%u)",
					iSegment, nWordsRead,
					(uint64_t)tWord.m_uWordID, sWord+1, tWord.m_uDoc, uPrevDocOffset ));

			// read doclist
			DWORD uDocOffset = pCurDoc-tSegment.m_dDocs.Begin();
			if ( tWord.m_uDoc!=uDocOffset )
			{
				LOC_FAIL(( fp, "unexpected doclist offset (wordid="UINT64_FMT"(%s)(%d), "
					"doclist_offset=%u, expected_offset=%u)",
					(uint64_t)tWord.m_uWordID, sWord+1, nWordsRead,
					tWord.m_uDoc, uDocOffset ));

				if ( uDocOffset>=(DWORD)tSegment.m_dDocs.GetLength() )
				{
					LOC_FAIL(( fp, "doclist offset pointing past doclist (segment=%d, word=%d, "
						"read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead,
						sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));

					nWordsRead++;
					continue;
				} else
					pCurDoc = tSegment.m_dDocs.Begin()+uDocOffset;
			}

			// read all docs from doclist
			RtDoc_t tDoc;
			memset ( &tDoc, 0, sizeof(tDoc) );
			SphDocID_t uPrevDocID = 0;

			for ( DWORD uDoc=0; uDoc<tWord.m_uDocs && pCurDoc<pMaxDoc; uDoc++ )
			{
				bool bEmbeddedHit = false;
				const BYTE * pIn = pCurDoc;
				SphDocID_t uDeltaID;
				pIn = UnzipDocid ( &uDeltaID, pIn );

				if ( pIn>=pMaxDoc )
				{
					LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
						"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead,
						(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
					break;
				}

				tDoc.m_uDocID += uDeltaID;
				DWORD uField;
				pIn = UnzipDword ( &uField, pIn );
				if ( pIn>=pMaxDoc )
				{
					LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
						"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead,
						(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
					break;
				}

				tDoc.m_uDocFields = uField;
				pIn = UnzipDword ( &tDoc.m_uHits, pIn );
				if ( pIn>=pMaxDoc )
				{
					LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
						"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead,
						(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
					break;
				}

				if ( tDoc.m_uHits==1 )
				{
					bEmbeddedHit = true;

					DWORD a, b;
					pIn = UnzipDword ( &a, pIn );
					if ( pIn>=pMaxDoc )
					{
						LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
							"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
						break;
					}

					pIn = UnzipDword ( &b, pIn );
					if ( pIn>pMaxDoc )
					{
						LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
							"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
						break;
					}

					tDoc.m_uHit = HITMAN::Create ( b, a );
				} else
				{
					pIn = UnzipDword ( &tDoc.m_uHit, pIn );
					if ( pIn>pMaxDoc )
					{
						LOC_FAIL(( fp, "reading past doclist end (segment=%d, word=%d, "
							"read_wordid="UINT64_FMT", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() ));
						break;
					}
				}
				pCurDoc = pIn;

				if ( tDoc.m_uDocID<=uPrevDocID )
				{
					LOC_FAIL(( fp, "docid decreased (segment=%d, word=%d, "
						"read_wordid="UINT64_FMT", read_word=%s, docid="UINT64_FMT", prev_docid="UINT64_FMT")",
						iSegment, nWordsRead,
						(uint64_t)tWord.m_uWordID, sWord+1, (uint64_t)tDoc.m_uDocID, (uint64_t)uPrevDocID ));
				}

				if ( !tSegment.FindRow ( tDoc.m_uDocID ) )
					LOC_FAIL(( fp, "no attributes found (segment=%d, word=%d, "
						"wordid="UINT64_FMT", docid="UINT64_FMT")",
						iSegment, nWordsRead,
						(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID ));

				if ( bEmbeddedHit )
				{
					DWORD uFieldId = HITMAN::GetField ( tDoc.m_uHit );
					DWORD uFieldMask = tDoc.m_uDocFields;
					int iCounter = 0;
					for ( ; uFieldMask; iCounter++ )
						uFieldMask &= uFieldMask - 1;

					if ( iCounter!=1 || tDoc.m_uHits!=1 )
					{
						LOC_FAIL(( fp, "embedded hit with multiple occurences in a document found "
							"(segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT")",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID ));
					}

					if ( (int)uFieldId>m_tSchema.m_dFields.GetLength() || uFieldId>SPH_MAX_FIELDS )
					{
						LOC_FAIL(( fp, "invalid field id in an embedded hit (segment=%d, word=%d, "
							"wordid="UINT64_FMT", docid="UINT64_FMT", field_id=%u, total_fields=%d)",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, uFieldId, m_tSchema.m_dFields.GetLength() ));
					}

					if ( !( tDoc.m_uDocFields & ( 1 << uFieldId ) ) )
					{
						LOC_FAIL(( fp, "invalid field id: not in doclist mask (segment=%d, word=%d, "
							"wordid="UINT64_FMT", docid="UINT64_FMT", field_id=%u, field_mask=%u)",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, uFieldId, tDoc.m_uDocFields ));
					}
				} else
				{
					DWORD uExpectedHitOffset = pCurHit-tSegment.m_dHits.Begin();
					if ( tDoc.m_uHit!=uExpectedHitOffset )
					{
						LOC_FAIL(( fp, "unexpected hitlist offset (segment=%d, word=%d, "
							"wordid="UINT64_FMT", docid="UINT64_FMT", offset=%u, expected_offset=%u",
							iSegment, nWordsRead,
							(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, tDoc.m_uHit, uExpectedHitOffset ));
					}

					if ( tDoc.m_uHit && tDoc.m_uHit<=uPrevHitOffset )
					{
						LOC_FAIL(( fp, "hitlist offset decreased (segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT", offset=%u, prev_offset=%u",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, tDoc.m_uHit, uPrevHitOffset ));
					}

					// check hitlist
					DWORD uHitlistEntry = 0;
					DWORD uLastPosInField = 0;
					DWORD uLastFieldId = 0;
					bool bLastInFieldFound = false;

					for ( DWORD uHit = 0; uHit < tDoc.m_uHits && pCurHit; uHit++ )
					{
						DWORD uValue = 0;
						pCurHit = UnzipDword ( &uValue, pCurHit );
						if ( pCurHit>pMaxHit )
						{
							LOC_FAIL(( fp, "reading past hitlist end (segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT")",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID ));
							break;
						}

						uHitlistEntry += uValue;

						DWORD uPosInField = HITMAN::GetPos ( uHitlistEntry );
						bool bLastInField = HITMAN::IsEnd ( uHitlistEntry );
						DWORD uFieldId = HITMAN::GetField ( uHitlistEntry );

						if ( (int)uFieldId>m_tSchema.m_dFields.GetLength() || uFieldId>SPH_MAX_FIELDS )
						{
							LOC_FAIL(( fp, "invalid field id in a hitlist (segment=%d, word=%d, "
								"wordid="UINT64_FMT", docid="UINT64_FMT", field_id=%u, total_fields=%d)",
								iSegment, nWordsRead,
								(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, uFieldId, m_tSchema.m_dFields.GetLength() ));
						}

						if ( !( tDoc.m_uDocFields & ( 1 << uFieldId ) ) )
						{
							LOC_FAIL(( fp, "invalid field id: not in doclist mask (segment=%d, word=%d, "
								"wordid="UINT64_FMT", docid="UINT64_FMT", field_id=%u, field_mask=%u)",
								iSegment, nWordsRead,
								(uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, uFieldId, tDoc.m_uDocFields ));
						}

						if ( uLastFieldId!=uFieldId )
						{
							bLastInFieldFound = false;
							uLastPosInField = 0;
						}

						if ( uLastPosInField && uPosInField<=uLastPosInField )
						{
							LOC_FAIL(( fp, "hit position in field decreased (segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT", pos=%u, last_pos=%u)",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, uPosInField, uLastPosInField ));
						}

						if ( bLastInField && bLastInFieldFound )
						{
							LOC_FAIL(( fp, "duplicate last-in-field hit found (segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT")",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID ));
						}

						uLastPosInField = uPosInField;
						uLastFieldId = uField;
						bLastInFieldFound |= bLastInField;
					}

					uPrevHitOffset = tDoc.m_uHit;
				}

				DWORD uAvailFieldMask = ( 1 << m_tSchema.m_dFields.GetLength() ) - 1;
				if ( tDoc.m_uDocFields & ~uAvailFieldMask )
				{
					LOC_FAIL(( fp, "wrong document field mask (segment=%d, word=%d, wordid="UINT64_FMT", docid="UINT64_FMT", mask=%u, total_fields=%d",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)tDoc.m_uDocID, tDoc.m_uDocFields, m_tSchema.m_dFields.GetLength() ));
				}

				const SphDocID_t * pKEntry = tSegment.m_dKlist.BinarySearch ( tDoc.m_uDocID );
				if ( pKEntry )
					dUsedKListEntries [pKEntry-tSegment.m_dKlist.Begin()] = true;

				uPrevDocID = tDoc.m_uDocID;
			}

			uPrevDocOffset = tWord.m_uDoc;
			nWordsRead++;
		}

		if ( pCurDoc!=pMaxDoc )
			LOC_FAIL(( fp, "unused doclist entries found (segment=%d, doclist_size=%d)",
				iSegment, tSegment.m_dDocs.GetLength() ));

		if ( pCurHit!=pMaxHit )
			LOC_FAIL(( fp, "unused hitlist entries found (segment=%d, hitlist_size=%d)",
				iSegment, tSegment.m_dHits.GetLength() ));

		if ( dRefCheckpoints.GetLength()!=tSegment.m_dWordCheckpoints.GetLength() )
			LOC_FAIL(( fp, "word checkpoint count mismatch (read=%d, calc=%d)",
				tSegment.m_dWordCheckpoints.GetLength(), dRefCheckpoints.GetLength() ));

		for ( int i=0; i < Min ( dRefCheckpoints.GetLength(), tSegment.m_dWordCheckpoints.GetLength() ); i++ )
		{
			const RtWordCheckpoint_t & tRefCP = dRefCheckpoints[i];
			const RtWordCheckpoint_t & tCP = tSegment.m_dWordCheckpoints[i];
			const int iLen = m_bKeywordDict ? strlen ( tCP.m_sWord ) : 0;
			if ( m_bKeywordDict && ( !tCP.m_sWord || ( !strlen ( tRefCP.m_sWord ) || !strlen ( tCP.m_sWord ) ) ) )
			{
				LOC_FAIL(( fp, "empty word checkpoint %d ((segment=%d, read_word=%s, read_len=%u, readpos=%d, calc_word=%s, calc_len=%u, calcpos=%d)",
					i, iSegment, tCP.m_sWord, (DWORD)strlen ( tCP.m_sWord ), tCP.m_iOffset,
					tRefCP.m_sWord, (DWORD)strlen ( tRefCP.m_sWord ), tRefCP.m_iOffset ));
			} else if ( sphCheckpointCmpStrictly ( tCP.m_sWord, iLen, tCP.m_iWordID, m_bKeywordDict, tRefCP ) || tRefCP.m_iOffset!=tCP.m_iOffset )
			{
				if ( m_bKeywordDict )
				{
					LOC_FAIL(( fp, "word checkpoint %d differs (segment=%d, read_word=%s, readpos=%d, calc_word=%s, calcpos=%d)",
						i, iSegment, tCP.m_sWord, tCP.m_iOffset, tRefCP.m_sWord, tRefCP.m_iOffset ));
				} else
				{
					LOC_FAIL(( fp, "word checkpoint %d differs (segment=%d, readid="UINT64_FMT", readpos=%d, calcid="UINT64_FMT", calcpos=%d)",
						i, iSegment, (uint64_t)tCP.m_iWordID, tCP.m_iOffset, (int64_t)tRefCP.m_iWordID, tRefCP.m_iOffset ));
				}
			}
		}

		if ( m_bKeywordDict )
			ARRAY_FOREACH ( i, dRefCheckpoints )
				SafeDeleteArray ( dRefCheckpoints[i].m_sWord );

		dRefCheckpoints.Reset ();

		// check killlists
		int nUsedKListEntries = 0;
		ARRAY_FOREACH ( i, dUsedKListEntries )
			if ( dUsedKListEntries[i] )
				nUsedKListEntries++;

		if ( nUsedKListEntries!=tSegment.m_dKlist.GetLength() )
		{
			LOC_FAIL(( fp, "used killlist entries mismatch (segment=%d, klist_entries=%d, used_entries=%d)",
				iSegment, tSegment.m_dKlist.GetLength(), nUsedKListEntries ));
		}

		for ( int iKListEntry = 1; iKListEntry<tSegment.m_dKlist.GetLength(); iKListEntry++ )
		{
			if ( tSegment.m_dKlist[iKListEntry-1]>=tSegment.m_dKlist[iKListEntry] )
				LOC_FAIL(( fp, "unsorted kill-list values (segment=%d, val[%d]="DOCID_FMT", val[%d]="DOCID_FMT")",
					iSegment, iKListEntry-1, tSegment.m_dKlist[iKListEntry-1], iKListEntry, tSegment.m_dKlist[iKListEntry] ));
		}

		// check attributes
		if ( tSegment.m_iRows*m_iStride!=tSegment.m_dRows.GetLength() )
		{
			LOC_FAIL(( fp, "rowitems count mismatch (segment=%d, expected=%d, current=%d)",
				iSegment, tSegment.m_iRows*m_iStride, tSegment.m_dRows.GetLength() ));
		}

		CSphVector<int> dMvaItems;
		CSphVector<CSphAttrLocator> dFloatItems;
		CSphVector<CSphAttrLocator> dStrItems;
		for ( int iAttr=0; iAttr<m_tSchema.GetAttrsCount(); iAttr++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);
			if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET )
			{
				if ( tAttr.m_tLocator.m_iBitCount!=ROWITEM_BITS )
				{
					LOC_FAIL(( fp, "unexpected MVA bitcount (segment=%d, attr=%d, expected=%d, got=%d)",
						iSegment, iAttr, ROWITEM_BITS, tAttr.m_tLocator.m_iBitCount ));
					continue;
				}
				if ( ( tAttr.m_tLocator.m_iBitOffset % ROWITEM_BITS )!=0 )
				{
					LOC_FAIL(( fp, "unaligned MVA bitoffset (segment=%d, attr=%d, bitoffset=%d)",
						iSegment, iAttr, tAttr.m_tLocator.m_iBitOffset ));
					continue;
				}
				if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET )
					dMvaItems.Add ( tAttr.m_tLocator.m_iBitOffset/ROWITEM_BITS );
			} else if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
				dFloatItems.Add	( tAttr.m_tLocator );
			else if ( tAttr.m_eAttrType==SPH_ATTR_STRING )
				dStrItems.Add ( tAttr.m_tLocator );
		}
		int iMva64 = dMvaItems.GetLength();
		for ( int iAttr=0; iAttr<m_tSchema.GetAttrsCount(); iAttr++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);
			if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
				dMvaItems.Add ( tAttr.m_tLocator.m_iBitOffset/ROWITEM_BITS );
		}

		CSphVector<DWORD> dStringOffsets;
		if ( tSegment.m_dStrings.GetLength() > 1 )
		{
			const BYTE * pBaseStr = tSegment.m_dStrings.Begin();
			const BYTE * pCurStr = pBaseStr + 1;
			const BYTE * pMaxStr = pBaseStr + tSegment.m_dStrings.GetLength();
			while ( pCurStr<pMaxStr )
			{
				const BYTE * pStr = NULL;
				const int iLen = sphUnpackStr ( pCurStr, &pStr );

				if ( !iLen )
					LOC_FAIL(( fp, "empty attribute string found (segment=%d, offset=%u)", iSegment, (DWORD)(pCurStr-pBaseStr) ));

				if ( pStr+iLen>pMaxStr || pStr<pCurStr || pStr>pCurStr+4 )
				{
					LOC_FAIL(( fp, "string length out of bounds (segment=%d, offset=%u, len=%d)", iSegment, (DWORD)(pCurStr-pBaseStr), iLen ));
					break;
				}

				dStringOffsets.Add ( (DWORD)(pCurStr-pBaseStr) );

				const BYTE * pStringStart = pStr;
				while ( pStringStart-pStr < iLen )
				{
					if ( !*pStringStart )
					{
						CSphString sErrorStr;
						sErrorStr.SetBinary ( (const char*)pStr, iLen );
						LOC_FAIL(( fp, "embedded zero in a string (segment=%d, offset=%u, string=%s)",
							iSegment, (DWORD)(pStringStart-pBaseStr), sErrorStr.cstr() ));
					}

					pStringStart++;
				}

				pCurStr = pStr + iLen;
			}
		}

		// loop the rows
		const CSphRowitem * pRow = tSegment.m_dRows.Begin();
		const CSphRowitem * pRowMax = pRow + tSegment.m_dRows.GetLength();
		const DWORD * pMvaBase = tSegment.m_dMvas.Begin();
		const DWORD * pMvaMax = pMvaBase + tSegment.m_dMvas.GetLength();
		const DWORD * pMvaCur = pMvaBase + 1;

		SphDocID_t uLastID = 0;
		DWORD uLastStrOffset = 0;
		int nCalcAliveRows = 0;
		int nCalcRows = 0;
		int nUsedStrings = 0;

		for ( DWORD uRow=0; pRow<pRowMax; uRow++, pRow+=m_iStride )
		{
			if ( uLastID>=DOCINFO2ID(pRow) )
				LOC_FAIL(( fp, "docid decreased in attributes (segment=%d, row=%u, docid="DOCID_FMT", lastid="DOCID_FMT")",
					iSegment, uRow, DOCINFO2ID(pRow), uLastID ));

			uLastID = DOCINFO2ID(pRow);

			///////////////////////////
			// check MVAs
			///////////////////////////

			if ( dMvaItems.GetLength() )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);

				ARRAY_FOREACH ( iItem, dMvaItems )
				{
					const DWORD uOffset = pAttrs[dMvaItems[iItem]];
					if ( !uOffset )
						continue;

					if ( pMvaBase+uOffset>=pMvaMax )
					{
						LOC_FAIL(( fp, "MVA index out of bounds (segment=%d, row=%u, mvaattr=%d, docid="DOCID_FMT", index=%u)",
							iSegment, uRow, iItem, uLastID, uOffset ));
						continue;
					}

					if ( pMvaCur!=pMvaBase+uOffset )
					{
						LOC_FAIL(( fp, "wrong MVA offset (segment=%d, row=%u, mvaattr=%d, docid="DOCID_FMT", expected=%u, got=%u)",
							iSegment, uRow, iItem, uLastID, (DWORD)(pMvaCur-pMvaBase), uOffset ));

						pMvaCur = pMvaBase+uOffset;
					}

					// check values
					DWORD uValues = *pMvaCur++;

					if ( pMvaCur+uValues-1>=pMvaMax )
					{
						LOC_FAIL(( fp, "MVA count out of bounds (segment=%d, row=%u, mvaattr=%d, docid="DOCID_FMT", count=%u)",
							iSegment, uRow, iItem, uLastID, uValues ));
						pMvaCur += uValues;
						continue;
					}

					// check that values are ascending
					for ( DWORD uVal=(iItem>=iMva64 ? 2 : 1); uVal<uValues; )
					{
						uint64_t uPrev, uCur;
						if ( iItem>=iMva64 )
						{
							uPrev = MVA_UPSIZE ( pMvaCur+uVal-2 );
							uCur = MVA_UPSIZE ( pMvaCur+uVal );
							uVal += 2;
						} else
						{
							uPrev = pMvaCur[uVal-1];
							uCur = pMvaCur[uVal];
							uVal++;
						}

						if ( uCur<=uPrev )
						{
							LOC_FAIL(( fp, "unsorted MVA values (segment=%d, row=%u, mvaattr=%d, docid="DOCID_FMT", val[%u]="UINT64_FMT", val[%u]="UINT64_FMT")",
								iSegment, uRow, iItem, uLastID, ( iItem>=iMva64 ? uVal-2 : uVal-1 ), uPrev, uVal, uCur ));
						}

						uVal += ( iItem>=iMva64 ? 2 : 1 );
					}

					pMvaCur += uValues;
				}
			}

			///////////////////////////
			// check floats
			///////////////////////////

			ARRAY_FOREACH ( iItem, dFloatItems )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
				const DWORD uValue = (DWORD)sphGetRowAttr ( pAttrs, dFloatItems[iItem] );
				const DWORD uExp = ( uValue >> 23 ) & 0xff;
				const DWORD uMantissa = uValue & 0x003fffff;

				// check normalized
				if ( uExp==0 && uMantissa!=0 )
					LOC_FAIL(( fp, "float attribute value is unnormalized (segment=%d, row=%u, attr=%d, id="DOCID_FMT", raw=0x%x, value=%f)",
						iSegment, uRow, iItem, uLastID, uValue, sphDW2F ( uValue ) ));

				// check +-inf
				if ( uExp==0xff && uMantissa==0 )
					LOC_FAIL(( fp, "float attribute is infinity (segment=%d, row=%u, attr=%d, id="DOCID_FMT", raw=0x%x, value=%f)",
						iSegment, uRow, iItem, uLastID, uValue, sphDW2F ( uValue ) ));
			}

			/////////////////
			// check strings
			/////////////////

			ARRAY_FOREACH ( iItem, dStrItems )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);

				const DWORD uOffset = (DWORD)sphGetRowAttr ( pAttrs, dStrItems[iItem] );
				if ( uOffset>=(DWORD)tSegment.m_dStrings.GetLength() )
				{
					LOC_FAIL(( fp, "string offset out of bounds (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", index=%u)",
						iSegment, uRow, iItem, uLastID, uOffset ));
					continue;
				}

				if ( !uOffset )
					continue;

				if ( uLastStrOffset>=uOffset )
					LOC_FAIL(( fp, "string offset decreased (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u, last_offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset, uLastStrOffset ));

				if ( !dStringOffsets.BinarySearch ( uOffset ) )
				{
					LOC_FAIL(( fp, "string offset is not a string start (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset ));
				} else
					nUsedStrings++;

				uLastStrOffset = uOffset;
			}

			nCalcRows++;
			if ( !tSegment.m_dKlist.BinarySearch ( uLastID ) )
				nCalcAliveRows++;
		}

		if ( nUsedStrings!=dStringOffsets.GetLength() )
			LOC_FAIL(( fp, "unused string entries found (segment=%d)", iSegment ));

		if ( dMvaItems.GetLength() && pMvaCur!=pMvaMax )
			LOC_FAIL(( fp, "unused MVA entries found (segment=%d)", iSegment ));

		if ( tSegment.m_iRows!=nCalcRows )
			LOC_FAIL(( fp, "row count mismatch (segment=%d, expected=%d, current=%d)",
				iSegment, nCalcRows, tSegment.m_iRows ));

		if ( tSegment.m_iAliveRows!=nCalcAliveRows )
			LOC_FAIL(( fp, "alive row count mismatch (segment=%d, expected=%d, current=%d)",
				iSegment, nCalcAliveRows, tSegment.m_iAliveRows ));
	}

	ARRAY_FOREACH ( i, m_pDiskChunks )
	{
		fprintf ( fp, "checking disk chunk %d(%d)...\n", i, m_pDiskChunks.GetLength() );
		iFailsPlain += m_pDiskChunks[i]->DebugCheck ( fp );
	}

	tmCheck = sphMicroTimer() - tmCheck;
	if ( ( iFails+iFailsPlain )==0 )
		fprintf ( fp, "check passed" );
	else if ( iFails!=iFailsPrinted )
		fprintf ( fp, "check FAILED, %d of %d failures reported", iFailsPrinted, iFails+iFailsPlain );
	else
		fprintf ( fp, "check FAILED, %d failures reported", iFails+iFailsPlain );

	fprintf ( fp, ", %d.%d sec elapsed\n", (int)(tmCheck/1000000), (int)((tmCheck/100000)%10) );

	return iFails + iFailsPlain;
} // NOLINT function length

void RtIndex_t::SetEnableStar ( bool bEnableStar )
{
	m_bEnableStar = bEnableStar;
	ARRAY_FOREACH ( i, m_pDiskChunks )
		m_pDiskChunks[i]->SetEnableStar ( bEnableStar );
}

//////////////////////////////////////////////////////////////////////////
// SEARCHING
//////////////////////////////////////////////////////////////////////////

struct RtQword_t : public ISphQword
{
	friend struct RtIndex_t;
	friend struct RtQwordSetup_t;

protected:
	RtDocReader_t *		m_pDocReader;
	CSphMatch			m_tMatch;

	DWORD				m_uNextHit;
	RtHitReader2_t		m_tHitReader;

	const RtSegment_t *	m_pSeg;

public:
	RtQword_t ()
		: m_pDocReader ( NULL )
		, m_uNextHit ( 0 )
		, m_pSeg ( NULL )
	{
		m_tMatch.Reset ( 0 );
	}

	virtual ~RtQword_t ()
	{
		SafeDelete ( m_pDocReader );
	}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		if ( !m_pDocReader )
		{
			m_tMatch.m_iDocID = 0;
			return m_tMatch;
		}

		for ( ;; )
		{
			const RtDoc_t * pDoc = m_pDocReader->UnzipDoc();
			if ( !pDoc )
			{
				m_tMatch.m_iDocID = 0;
				return m_tMatch;
			}

			if ( m_pSeg->m_dKlist.BinarySearch ( pDoc->m_uDocID ) )
				continue;

			m_tMatch.m_iDocID = pDoc->m_uDocID;
			m_dQwordFields.Assign32 ( pDoc->m_uDocFields );
			m_uMatchHits = pDoc->m_uHits;
			m_iHitlistPos = (uint64_t(pDoc->m_uHits)<<32) + pDoc->m_uHit;
			m_bAllFieldsKnown = false;
			return m_tMatch;
		}
	}

	virtual void SeekHitlist ( SphOffset_t uOff )
	{
		int iHits = (int)(uOff>>32);
		if ( iHits==1 )
		{
			m_uNextHit = DWORD(uOff);
		} else
		{
			m_uNextHit = 0;
			m_tHitReader.Seek ( DWORD(uOff), iHits );
		}
	}

	virtual Hitpos_t GetNextHit ()
	{
		if ( m_uNextHit==0 )
		{
			return Hitpos_t ( m_tHitReader.UnzipHit() );

		} else if ( m_uNextHit==0xffffffffUL )
		{
			return EMPTY_HIT;

		} else
		{
			DWORD uRes = m_uNextHit;
			m_uNextHit = 0xffffffffUL;
			return Hitpos_t ( uRes );
		}
	}
};


struct RtQwordSetup_t : ISphQwordSetup
{
	RtSegment_t *		m_pSeg;

	virtual ISphQword *	QwordSpawn ( const XQKeyword_t & ) const;
	virtual bool		QwordSetup ( ISphQword * pQword ) const;
};


ISphQword * RtQwordSetup_t::QwordSpawn ( const XQKeyword_t & ) const
{
	return new RtQword_t ();
}


bool RtQwordSetup_t::QwordSetup ( ISphQword * pQword ) const
{
	RtQword_t * pMyWord = dynamic_cast<RtQword_t*> ( pQword );
	if ( !pMyWord )
		return false;

	const RtIndex_t * pIndex = dynamic_cast< const RtIndex_t * > ( m_pIndex );
	if ( !pIndex )
		return false;

	return pIndex->RtQwordSetup ( pMyWord, m_pSeg );
}


bool RtIndex_t::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	// might be needed even when we do not have a filter!
	if ( pCtx->m_bLookupFilter || pCtx->m_bLookupSort )
		CopyDocinfo ( tMatch, FindDocinfo ( (RtSegment_t*)pCtx->m_pIndexData, tMatch.m_iDocID ) );

	pCtx->CalcFilter ( tMatch ); // FIXME!!! leak of filtered STRING_PTR
	return pCtx->m_pFilter ? !pCtx->m_pFilter->Eval ( tMatch ) : false;
}


void RtIndex_t::CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const
{
	if ( !pFound )
		return;

	// setup static pointer
	assert ( DOCINFO2ID(pFound)==tMatch.m_iDocID );
	tMatch.m_pStatic = DOCINFO2ATTRS(pFound);

	// FIXME? implement overrides
}


const CSphRowitem * RtIndex_t::FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID ) const
{
	// FIXME! move to CSphIndex, and implement hashing
	if ( pSeg->m_dRows.GetLength()==0 )
		return NULL;

	int iStride = m_iStride;
	int iStart = 0;
	int iEnd = pSeg->m_iRows-1;
	assert ( iStride==( DOCINFO_IDSIZE + m_tSchema.GetRowSize() ) );

	const CSphRowitem * pStorage = pSeg->m_dRows.Begin();
	const CSphRowitem * pFound = NULL;

	if ( uDocID==DOCINFO2ID ( &pStorage [ iStart*iStride ] ) )
	{
		pFound = &pStorage [ iStart*iStride ];

	} else if ( uDocID==DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) )
	{
		pFound = &pStorage [ iEnd*iStride ];

	} else
	{
		while ( iEnd-iStart>1 )
		{
			// check if nothing found
			if (
				uDocID < DOCINFO2ID ( &pStorage [ iStart*iStride ] ) ||
				uDocID > DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) )
				break;
			assert ( uDocID > DOCINFO2ID ( &pStorage [ iStart*iStride ] ) );
			assert ( uDocID < DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) );

			int iMid = iStart + (iEnd-iStart)/2;
			if ( uDocID==DOCINFO2ID ( &pStorage [ iMid*iStride ] ) )
			{
				pFound = &pStorage [ iMid*iStride ];
				break;
			}
			if ( uDocID<DOCINFO2ID ( &pStorage [ iMid*iStride ] ) )
				iEnd = iMid;
			else
				iStart = iMid;
		}
	}

	return pFound;
}

// WARNING, setup is pretty tricky
// for RT queries, we setup qwords several times
// first pass (with NULL segment arg) should sum all stats over all segments
// others passes (with non-NULL segments) should setup specific segment (including local stats)
bool RtIndex_t::RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pCurSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint )
{
	if ( !pCurSeg )
		return false;

	SphWordID_t uWordID = pQword->m_iWordID;
	const char * sWord = pQword->m_sDictWord.cstr();
	int iWordLen = pQword->m_sDictWord.Length();
	if ( bWordDict && iWordLen && sWord[iWordLen-1]=='*' ) // crc star search emulation
	{
		iWordLen = iWordLen-1;
	}

	// no checkpoints - check all words
	// no checkpoints matched - check only words prior to 1st checkpoint
	// checkpoint found - check words at that checkpoint
	RtWordReader_t tReader ( pCurSeg, bWordDict, iWordsCheckpoint );

	if ( pCurSeg->m_dWordCheckpoints.GetLength() )
	{
		const RtWordCheckpoint_t * pCp = sphSearchCheckpoint ( sWord, iWordLen, uWordID, false, bWordDict
			, pCurSeg->m_dWordCheckpoints.Begin(), &pCurSeg->m_dWordCheckpoints.Last() );

		const BYTE * pWords = pCurSeg->m_dWords.Begin();

		if ( !pCp )
		{
			tReader.m_pMax = pWords + pCurSeg->m_dWordCheckpoints.Begin()->m_iOffset;
		} else
		{
			tReader.m_pCur = pWords + pCp->m_iOffset;
			// if next checkpoint exists setup reader range
			if ( ( pCp+1 )<= ( &pCurSeg->m_dWordCheckpoints.Last() ) )
				tReader.m_pMax = pWords + pCp[1].m_iOffset;
		}
	}

	// find the word between checkpoints
	const RtWord_t * pWord = NULL;
	while ( ( pWord = tReader.UnzipWord() )!=NULL )
	{
		int iCmp = 0;
		if ( bWordDict )
		{
			iCmp = sphDictCmpStrictly ( (const char *)pWord->m_sWord+1, pWord->m_sWord[0], sWord, iWordLen );
		} else
		{
			if ( pWord->m_uWordID<uWordID )
				iCmp = -1;
			else if ( pWord->m_uWordID>uWordID )
				iCmp = 1;
		}

		if ( iCmp==0 )
		{
			pQword->m_iDocs += pWord->m_uDocs;
			pQword->m_iHits += pWord->m_uHits;
			if ( bSetup )
			{
				SafeDelete ( pQword->m_pDocReader );
				pQword->m_pDocReader = new RtDocReader_t ( pCurSeg, *pWord );
				pQword->m_tHitReader.m_pBase = NULL;
				if ( pCurSeg->m_dHits.GetLength() )
					pQword->m_tHitReader.m_pBase = pCurSeg->m_dHits.Begin();
				pQword->m_pSeg = pCurSeg;
			}
			return true;

		} else if ( iCmp>0 )
			return false;
	}
	return false;
}


struct DocHitPair_t
{
	int m_iDocs;
	int m_iHits;
};


void RtIndex_t::GetPrefixedWords ( const char * sWord, int iWordLen, const char * sWildcard,
	CSphVector<CSphNamedInt> & dPrefixedWords, BYTE *, int ) const
{
	const int iSkipMagic = ( BYTE(*sWord)<0x20 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	SmallStringHash_T<DocHitPair_t> hPrefixedWords;
	ARRAY_FOREACH ( i, m_pSegments )
	{
		RtSegment_t * pCurSeg = m_pSegments[i];
		RtWordReader_t tReader ( pCurSeg, true, m_iWordsCheckpoint );

		// find initial checkpoint or check words prior to 1st checkpoint
		if ( pCurSeg->m_dWordCheckpoints.GetLength() )
		{
			const RtWordCheckpoint_t * pCurCheckpoint = sphSearchCheckpoint ( sWord, iWordLen, 0, true, true
				, pCurSeg->m_dWordCheckpoints.Begin(), &pCurSeg->m_dWordCheckpoints.Last() );

			if ( pCurCheckpoint )
			{
				// there could be valid data prior 1st checkpoint that should be unpacked and checked
				int iCheckpointNameLen = strlen ( pCurCheckpoint->m_sWord );
				if ( pCurCheckpoint!=pCurSeg->m_dWordCheckpoints.Begin()
					|| ( sphDictCmp ( sWord, iWordLen, pCurCheckpoint->m_sWord, iCheckpointNameLen )==0 && iWordLen==iCheckpointNameLen ) )
				{
					tReader.m_pCur = pCurSeg->m_dWords.Begin() + pCurCheckpoint->m_iOffset;
				}
			}
		}

		// find the word between checkpoints
		const RtWord_t * pWord = NULL;
		while ( ( pWord = tReader.UnzipWord() )!=NULL )
		{
			int iCmp = sphDictCmp ( sWord, iWordLen, (const char *)pWord->m_sWord+1, pWord->m_sWord[0] );
			if ( iCmp<0 )
			{
				break;
			} else if ( iCmp==0 && iWordLen<=pWord->m_sWord[0] && sphWildcardMatch ( (const char *)pWord->m_sWord+1+iSkipMagic, sWildcard ) )
			{
				CSphString sExpandedWord;
				sExpandedWord.SetBinary ( (const char *)pWord->m_sWord+1, pWord->m_sWord[0] );
				DocHitPair_t * pPair = hPrefixedWords ( sExpandedWord );
				if ( pPair )
				{
					// FIXME!!! in case of REPLACED data docs\hits grows a lot and ExtCached caches less words
					// keep track of alive docs?
					pPair->m_iDocs += pWord->m_uDocs;
					pPair->m_iHits += pWord->m_uHits;
				} else
				{
					DocHitPair_t tPair;
					tPair.m_iDocs = pWord->m_uDocs;
					tPair.m_iHits = pWord->m_uHits;
					hPrefixedWords.Add ( tPair, sExpandedWord );
				}
			}
			// FIXME!!! same case 'boxi*' matches 'box' document at plain index
			// but masked by a checkpoint search
		}
	}

	dPrefixedWords.Reserve ( dPrefixedWords.GetLength() + hPrefixedWords.GetLength() );
	hPrefixedWords.IterateStart();
	while ( hPrefixedWords.IterateNext() )
	{
		CSphNamedInt & tExpanded = dPrefixedWords.Add ();
		tExpanded.m_sName = hPrefixedWords.IterateGetKey();
		const DocHitPair_t & tPair = hPrefixedWords.IterateGet();
		tExpanded.m_iValue = sphGetExpansionMagic ( tPair.m_iDocs, tPair.m_iHits );
	}
}


static bool MatchBloomCheckpoint ( const uint64_t * pBloom, const uint64_t * pVals, int iWordsStride, int iCP, int iHashes )
{
	int dMatches[ BLOOM_HASHES_COUNT ];
	memset ( dMatches, 0, sizeof(dMatches) );
	int iMatch = 0;

	for ( int j=0; j<iWordsStride*iHashes*BLOOM_PER_ENTRY_VALS_COUNT; j++ )
	{
		int iVal = j % ( BLOOM_PER_ENTRY_VALS_COUNT * iHashes );
		uint64_t uInfix = pVals[iVal];
		uint64_t uFilter = pBloom[ j + iCP * iWordsStride * iHashes * BLOOM_PER_ENTRY_VALS_COUNT ];
		iMatch += ( ( uInfix & uFilter )==uInfix );

		if ( j%BLOOM_PER_ENTRY_VALS_COUNT==BLOOM_PER_ENTRY_VALS_COUNT-1 )
		{
			dMatches[ ( j/BLOOM_PER_ENTRY_VALS_COUNT ) % iHashes ] += ( iMatch==BLOOM_PER_ENTRY_VALS_COUNT );
			iMatch = 0;
		}
	}

	int iMatched = 0;
	for ( int iMatch=0; iMatch<iHashes; iMatch++ )
	{
		iMatched += ( dMatches[iMatch]>0 );
	}

	return ( iMatched==iHashes );
}


static bool ExtractInfixCheckpoints ( const char * sInfix, int iBytes, int iMaxCodepointLength, int iCPs, const CSphTightVector<uint64_t> & dFilter, CSphVector<int> & dCheckpoints )
{
	dCheckpoints.Resize ( 0 );
	if ( !dFilter.GetLength() )
		return false;

	uint64_t dVals[ BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT ];
	memset ( dVals, 0, sizeof(dVals) );

	if ( !BuildBloom ( (const BYTE *)sInfix, iBytes, 2, ( iMaxCodepointLength>1 ), dVals+BLOOM_PER_ENTRY_VALS_COUNT*0, BLOOM_PER_ENTRY_VALS_COUNT ) )
		return false;
	BuildBloom ( (const BYTE *)sInfix, iBytes, 4, ( iMaxCodepointLength>1 ), dVals+BLOOM_PER_ENTRY_VALS_COUNT*1, BLOOM_PER_ENTRY_VALS_COUNT );

	const uint64_t * pRough = dFilter.Begin();
	for ( int i=0; i<iCPs+1; i++ )
	{
		if ( MatchBloomCheckpoint ( pRough, dVals, 1, i, BLOOM_HASHES_COUNT ) )
			dCheckpoints.Add ( i );
	}

	return ( dCheckpoints.GetLength()>0 );
}


void RtIndex_t::GetInfixedWords ( const char * sInfix, int iBytes, const char * sWildcard, CSphVector<CSphNamedInt> & dExpanded ) const
{
	// sanity checks
	if ( !sInfix || iBytes<=0 )
		return;

	// find those prefixes
	CSphVector<int> dPoints;

	SmallStringHash_T<DocHitPair_t> hWords;
	ARRAY_FOREACH ( iSeg, m_pSegments )
	{
		RtSegment_t * pSeg = m_pSegments[iSeg];
		if ( !pSeg->m_dWords.GetLength() )
			continue;

		dPoints.Resize ( 0 );
		if ( !ExtractInfixCheckpoints ( sInfix, iBytes, m_iMaxCodepointLength, pSeg->m_dWordCheckpoints.GetLength(), pSeg->m_dInfixFilterCP, dPoints ) )
			continue;

		// walk those checkpoints, check all their words
		ARRAY_FOREACH ( i, dPoints )
		{
			int iNext = dPoints[i];
			int iCur = iNext-1;
			RtWordReader_t tReader ( pSeg, true, m_iWordsCheckpoint );
			if ( iCur>0 )
				tReader.m_pCur = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iCur].m_iOffset;
			if ( iNext<pSeg->m_dWordCheckpoints.GetLength() )
				tReader.m_pMax = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iNext].m_iOffset;

			int iMatches = 0;
			const RtWord_t * pWord = NULL;
			while ( ( pWord = tReader.UnzipWord() )!=NULL )
			{
				// check it
				if ( !sphWildcardMatch ( (const char*)pWord->m_sWord+1, sWildcard ) )
					continue;

				iMatches++;
				// matched, lets add
				CSphString sWord ( (const char*)pWord->m_sWord+1, pWord->m_sWord[0] );
				DocHitPair_t * pPair = hWords ( sWord );
				if ( pPair )
				{
					pPair->m_iDocs += pWord->m_uDocs;
					pPair->m_iHits += pWord->m_uHits;
				} else
				{
					DocHitPair_t tPair;
					tPair.m_iDocs = pWord->m_uDocs;
					tPair.m_iHits = pWord->m_uHits;
					hWords.Add ( tPair, sWord );
				}
			}
		}
	}

	dExpanded.Reserve ( dExpanded.GetLength() + hWords.GetLength() );
	hWords.IterateStart();
	while ( hWords.IterateNext() )
	{
		CSphNamedInt & tExpanded = dExpanded.Add ();
		tExpanded.m_sName = hWords.IterateGetKey();
		DocHitPair_t & tPair = hWords.IterateGet();
		tExpanded.m_iValue = sphGetExpansionMagic ( tPair.m_iDocs, tPair.m_iHits );
	}
}


bool RtIndex_t::RtQwordSetup ( RtQword_t * pQword, const RtSegment_t * pSeg ) const
{
	// segment-specific setup pass
	if ( pSeg )
		return RtQwordSetupSegment ( pQword, pSeg, true, m_bKeywordDict, m_iWordsCheckpoint );

	// stat-only pass
	// loop all segments, gather stats, do not setup anything
	assert ( !pSeg );
	pQword->m_iDocs = 0;
	pQword->m_iHits = 0;
	if ( !m_pSegments.GetLength() )
		return true;

	// we care about the results anyway though
	// because if all (!) segments miss this word, we must notify the caller, right?
	bool bFound = false;
	ARRAY_FOREACH ( i, m_pSegments )
		bFound |= RtQwordSetupSegment ( pQword, m_pSegments[i], false, m_bKeywordDict, m_iWordsCheckpoint );

	// sanity check
	assert (!( bFound==true && pQword->m_iDocs==0 ) );
	return bFound;
}

static void AddKillListFilter ( CSphFilterSettings * pFilter, const SphAttr_t * pKillList, int nEntries )
{
	assert ( nEntries && pKillList && pFilter );
	pFilter->m_bExclude = true;
	pFilter->m_eType = SPH_FILTER_VALUES;
	pFilter->m_iMinValue = pKillList[0];
	pFilter->m_iMaxValue = pKillList[nEntries-1];
	pFilter->m_sAttrName = "@id";
	pFilter->SetExternalValues ( pKillList, nEntries );
}


CSphDict * RtIndex_t::SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const
{
	assert ( pTokenizer );

	if ( !m_tSettings.m_bIndexExactWords )
		return pPrevDict;

	tContainer = new CSphDictExact ( pPrevDict );
	pTokenizer->AddPlainChar ( '=' );
	return tContainer.Ptr();
}


CSphDict * RtIndex_t::SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const
{
	assert ( pTokenizer );
	if ( !m_bEnableStar || !m_bKeywordDict )
		return pPrevDict;

	tContainer = new CSphDictStarV8 ( pPrevDict, false, true );
	pTokenizer->AddPlainChar ( '*' );
	return tContainer.Ptr();
}


// FIXME! missing MVA, index_exact_words support
// FIXME? missing enable_star, legacy match modes support
// FIXME? any chance to factor out common backend agnostic code?
// FIXME? do we need to support pExtraFilters?
bool RtIndex_t::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters,
	ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> *, int iTag, bool bFactors ) const
{
	assert ( ppSorters );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( iSorters );
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i] )
			dSorters.Add ( ppSorters[i] );

	// if we have anything to work with
	if ( dSorters.GetLength()==0 )
	{
		pResult->m_iQueryTime = 0;
		return false;
	}

	// FIXME! too early (how low can you go?)
	m_tRwlock.ReadLock ();

	assert ( pQuery );
	assert ( pResult );
	assert ( iTag==0 );
	iTag = 0; // just to avoid a compiler warning

	MEMORY ( SPH_MEM_IDX_RT_MULTY_QUERY );

	// start counting
	pResult->m_iQueryTime = 0;
	int64_t tmQueryStart = sphMicroTimer();

	// force ext2 mode for them
	// FIXME! eliminate this const breakage
	const_cast<CSphQuery*> ( pQuery )->m_eMode = SPH_MATCH_EXTENDED2;

	// wrappers
	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( false ) );

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDict = m_pDict;
	if ( pDict->HasState() )
	{
		tDictCloned = pDict = pDict->Clone();
	}

	CSphScopedPtr<CSphDict> tDictStar ( NULL );
	pDict = SetupStarDict ( tDictStar, pDict, pTokenizer.Ptr() );

	CSphScopedPtr<CSphDict> tDictExact ( NULL );
	pDict = SetupExactDict ( tDictExact, pDict, pTokenizer.Ptr() );

	// FIXME! slow disk searches could lock out concurrent writes for too long
	// FIXME! each result will point to its own MVA and string pools
	// !COMMIT need to setup disk K-list here

	//////////////////////
	// search disk chunks
	//////////////////////

	m_tKlist.Flush();

	CSphVector<CSphString> dWrongWords;
	SmallStringHash_T<CSphQueryResultMeta::WordStat_t> hDiskStats;
	SmallStringHash_T<CSphQueryResultMeta::WordStat_t> hPrevWordStat = pResult->m_hWordStats;
	int64_t tmMaxTimer = 0;
	if ( pQuery->m_uMaxQueryMsec>0 )
		tmMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time

	CSphVector<SphAttr_t> dCumulativeKList;
	CSphVector<const BYTE *> dDiskStrings ( m_pDiskChunks.GetLength() );
	CSphVector<const DWORD *> dDiskMva ( m_pDiskChunks.GetLength() );
	for ( int iChunk = m_pDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
	{
		// collect & sort cumulative killlist for this chunk
		const SphAttr_t * pKlist = NULL;
		int iKlistEntries = 0;
		if ( iChunk==m_pDiskChunks.GetLength()-1 )
		{
			pKlist = m_tKlist.GetKillList();
			iKlistEntries = m_tKlist.GetKillListSize();
		} else
		{
			const CSphIndex * pDiskChunk = m_pDiskChunks[iChunk+1];
			pKlist = pDiskChunk->GetKillList();
			iKlistEntries = pDiskChunk->GetKillListSize();
		}

		if ( iKlistEntries )
		{
			int iKListLength = dCumulativeKList.GetLength();
			dCumulativeKList.Resize ( iKListLength+iKlistEntries );
			for ( int i = 0; i < iKlistEntries; i++ )
				dCumulativeKList[iKListLength+i] = pKlist[i];

			dCumulativeKList.Sort();
		}

		CSphVector<CSphFilterSettings> dKListFilter;
		if ( dCumulativeKList.GetLength() )
		{
			CSphFilterSettings & tKListFilter = dKListFilter.Add();
			AddKillListFilter ( &tKListFilter, dCumulativeKList.Begin(), dCumulativeKList.GetLength() );
		}

		CSphQueryResult tChunkResult;
		// storing index in matches tag for finding strings attrs offset later, biased against default zero and segments
		const int iTag = m_pSegments.GetLength()+iChunk+1;
		if ( !m_pDiskChunks[iChunk]->MultiQuery ( pQuery, &tChunkResult, iSorters, ppSorters, dCumulativeKList.GetLength() ? &dKListFilter : NULL, iTag, bFactors ) )
		{
			// FIXME? maybe handle this more gracefully (convert to a warning)?
			pResult->m_sError = tChunkResult.m_sError;
			m_tRwlock.Unlock ();
			return false;
		}

		// check terms inconsistency amongs disk chunks
		const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hDstStats = tChunkResult.m_hWordStats;
		if ( pResult->m_hWordStats.GetLength() )
		{
			pResult->m_hWordStats.IterateStart();
			while ( pResult->m_hWordStats.IterateNext() )
			{
				const CSphQueryResultMeta::WordStat_t * pDstStat = hDstStats ( pResult->m_hWordStats.IterateGetKey() );
				const CSphQueryResultMeta::WordStat_t & tSrcStat = pResult->m_hWordStats.IterateGet();

				// all indexes should produce same words from the query
				if ( !pDstStat && !tSrcStat.m_bExpanded )
				{
					dWrongWords.Add ( pResult->m_hWordStats.IterateGetKey() );
				}

				if ( pDstStat )
					pResult->AddStat ( pResult->m_hWordStats.IterateGetKey(), pDstStat->m_iDocs, pDstStat->m_iHits, pDstStat->m_bExpanded );
			}
		} else
		{
			pResult->m_hWordStats = hDstStats;
		}

		dDiskStrings[iChunk] = tChunkResult.m_pStrings;
		dDiskMva[iChunk] = tChunkResult.m_pMva;

		// keep last chunk statistics to check vs rt settings
		if ( !iChunk )
			hDiskStats = hDstStats;

		if ( iChunk && tmMaxTimer>0 && sphMicroTimer()>=tmMaxTimer )
		{
			pResult->m_sWarning = "query time exceeded max_query_time";
			break;
		}
	}

	////////////////////
	// search RAM chunk
	////////////////////

	// select the sorter with max schema
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	ARRAY_FOREACH ( i, dSorters )
		if ( dSorters[i]->GetSchema().GetRowSize() > iMaxSchemaSize )
		{
			iMaxSchemaSize = dSorters[i]->GetSchema().GetRowSize();
			iMaxSchemaIndex = i;
		}

	// setup calculations and result schema
	CSphQueryContext tCtx;
	if ( !tCtx.SetupCalc ( pResult, dSorters[iMaxSchemaIndex]->GetSchema(), m_tSchema, NULL ) )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	tCtx.m_bPackedFactors = bFactors;

	// setup search terms
	RtQwordSetup_t tTermSetup;
	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	tTermSetup.m_iDynamicRowitems = pResult->m_tSchema.GetDynamicSize();
	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time
	tTermSetup.m_pWarning = &pResult->m_sWarning;
	tTermSetup.m_pSeg = NULL;
	tTermSetup.m_pCtx = &tCtx;

	int iIndexWeight = pQuery->GetIndexWeight ( m_sIndexName.cstr() );

	// bind weights
	tCtx.BindWeights ( pQuery, m_tSchema, iIndexWeight );

	// parse query
	XQQuery_t tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), pTokenizer.Ptr(), &m_tSchema, pDict, m_tSettings ) )
	{
		pResult->m_sError = tParsed.m_sParseError;
		m_tRwlock.Unlock ();
		return false;
	}

	// transform query if needed (quorum transform, keyword expansion, etc.)
	sphTransformExtendedQuery ( &tParsed.m_pRoot, m_tSettings, pQuery->m_bSimplify, this );

	// adjust stars in keywords for dict=keywords, enable_star=0 case
	if ( pDict->GetSettings().m_bWordDict && !m_bEnableStar && ( m_tSettings.m_iMinPrefixLen>0 || m_tSettings.m_iMinInfixLen>0 ) )
		sphQueryAdjustStars ( tParsed.m_pRoot, m_tSettings );

	if ( m_bExpandKeywords )
	{
		tParsed.m_pRoot = sphQueryExpandKeywords ( tParsed.m_pRoot, m_tSettings, m_bEnableStar );
		tParsed.m_pRoot->Check ( true );
	}

	// expanding prefix in word dictionary case
	if ( m_bEnableStar && m_bKeywordDict )
	{
		ExpansionContext_t tCtx;
		tCtx.m_pWordlist = this;
		tCtx.m_pBuf = NULL;
		tCtx.m_pResult = pResult;
		tCtx.m_iFD = -1;
		tCtx.m_iMinPrefixLen = m_tSettings.m_iMinPrefixLen;
		tCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
		tCtx.m_iExpansionLimit = m_iExpansionLimit;
		tCtx.m_bHasMorphology = m_pDict->HasMorphology();
		tParsed.m_pRoot = sphExpandXQNode ( tParsed.m_pRoot, tCtx );
	}

	if ( !sphCheckQueryHeight ( tParsed.m_pRoot, pResult->m_sError ) )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( tParsed, pQuery, pResult, tTermSetup, tCtx ) );
	if ( !pRanker.Ptr() )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	tCtx.SetupExtraData ( pRanker.Ptr() );

	// check terms inconsistency disk chunks vs rt vs previous indexes
	sphCheckWordStats ( hDiskStats, pResult->m_hWordStats, m_sIndexName.cstr(), pResult->m_sWarning );
	sphCheckWordStats ( hPrevWordStat, pResult->m_hWordStats, m_sIndexName.cstr(), pResult->m_sWarning );

	// make warning on terms inconsistency
	if ( dWrongWords.GetLength() )
	{
		dWrongWords.Uniq();
		pResult->m_sWarning.SetSprintf ( "index '%s': query word(s) mismatch: %s", m_sIndexName.cstr(), dWrongWords.Begin()->cstr() );
		for ( int i=1; i<dWrongWords.GetLength(); i++ )
			pResult->m_sWarning.SetSprintf ( "%s, %s", pResult->m_sWarning.cstr(), dWrongWords[i].cstr() );
	}

	// empty index, empty result
	if ( !m_pSegments.GetLength() && !m_pDiskChunks.GetLength() )
	{
		pResult->m_iQueryTime = 0;
		m_tRwlock.Unlock ();
		return true;
	}

	// search segments no looking to max_query_time
	// FIXME!!! move searching at segments before disk chunks as result set is safe with kill-lists
	if ( m_pSegments.GetLength() )
	{
		// setup filters
		// FIXME! setup filters MVA pool
		bool bFullscan = ( pQuery->m_eMode==SPH_MATCH_FULLSCAN || pQuery->m_sQuery.IsEmpty() );
		if ( !tCtx.CreateFilters ( bFullscan, &pQuery->m_dFilters, pResult->m_tSchema, NULL, NULL, pResult->m_sError ) )
		{
			m_tRwlock.Unlock ();
			return false;
		}

		// FIXME! OPTIMIZE! check if we can early reject the whole index

		// setup lookup
		// do pre-filter lookup as needed
		// do pre-sort lookup in all cases
		// post-sort lookup is complicated (because of many segments)
		// pre-sort lookup is cheap now anyway, and almost always anyway
		// (except maybe by stupid relevance-sorting-only benchmarks!!)
		tCtx.m_bLookupFilter = ( pQuery->m_dFilters.GetLength() || tCtx.m_dCalcFilter.GetLength() );
		tCtx.m_bLookupSort = true;

		// FIXME! setup overrides

		// do searching
		bool bRandomize = dSorters[0]->m_bRandomize;
		int iCutoff = pQuery->m_iCutoff;
		if ( iCutoff<=0 )
			iCutoff = -1;

		if ( bFullscan )
		{
			// full scan
			// FIXME? OPTIMIZE? add shortcuts here too?
			CSphMatch tMatch;
			tMatch.Reset ( pResult->m_tSchema.GetDynamicSize() );
			tMatch.m_iWeight = pQuery->GetIndexWeight ( m_sIndexName.cstr() );

			int iCutoff = pQuery->m_iCutoff;
			if ( iCutoff<=0 )
				iCutoff = -1;

			ARRAY_FOREACH ( iSeg, m_pSegments )
			{
				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				}

				RtRowIterator_t tIt ( m_pSegments[iSeg], m_iStride, false, NULL );
				for ( ;; )
				{
					const CSphRowitem * pRow = tIt.GetNextAliveRow();
					if ( !pRow )
						break;

					tMatch.m_iDocID = DOCINFO2ID(pRow);
					tMatch.m_pStatic = DOCINFO2ATTRS(pRow); // FIXME! overrides

					tCtx.CalcFilter ( tMatch );
					if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
					{
						tCtx.FreeStrFilter ( tMatch );
						continue;
					}

					tCtx.CalcSort ( tMatch );
					tCtx.CalcFinal ( tMatch ); // OPTIMIZE? could be possibly done later

					if ( bRandomize )
						tMatch.m_iWeight = ( sphRand() & 0xffff );

					// storing segment in matches tag for finding strings attrs offset later, biased against default zero
					tMatch.m_iTag = iSeg+1;

					bool bNewMatch = false;
					ARRAY_FOREACH ( iSorter, dSorters )
						bNewMatch |= dSorters[iSorter]->Push ( tMatch );

					// stringptr expressions should be duplicated (or taken over) at this point
					tCtx.FreeStrFilter ( tMatch );
					tCtx.FreeStrSort ( tMatch );
					tCtx.FreeStrFinal ( tMatch );

					// handle cutoff
					if ( bNewMatch )
						if ( --iCutoff==0 )
							break;
				}

				if ( iCutoff==0 )
					break;
			}

		} else
		{
			// query matching
			ARRAY_FOREACH ( iSeg, m_pSegments )
			{
				tTermSetup.m_pSeg = m_pSegments[iSeg];
				pRanker->Reset ( tTermSetup );

				// for lookups to work
				tCtx.m_pIndexData = m_pSegments[iSeg];

				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				}
				pRanker->ExtraData ( EXTRA_SET_MVAPOOL, (void**)m_pSegments[iSeg]->m_dMvas.Begin() );
				pRanker->ExtraData ( EXTRA_SET_STRINGPOOL, (void**)m_pSegments[iSeg]->m_dStrings.Begin() );

				CSphMatch * pMatch = pRanker->GetMatchesBuffer();
				for ( ;; )
				{
					int iMatches = pRanker->GetMatches();
					if ( iMatches<=0 )
						break;

					for ( int i=0; i<iMatches; i++ )
					{
						assert ( !tCtx.m_bLookupSort || FindDocinfo ( m_pSegments[iSeg], pMatch[i].m_iDocID ) );

						if ( tCtx.m_bLookupSort )
							CopyDocinfo ( pMatch[i], FindDocinfo ( m_pSegments[iSeg], pMatch[i].m_iDocID ) );

						tCtx.CalcSort ( pMatch[i] );
						tCtx.CalcFinal ( pMatch[i] ); // OPTIMIZE? could be possibly done later

						if ( bRandomize )
							pMatch[i].m_iWeight = ( sphRand() & 0xffff );

						if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( pMatch[i] ) )
						{
							tCtx.FreeStrSort ( pMatch[i] );
							tCtx.FreeStrFinal ( pMatch[i] );
							continue;
						}

						// storing segment in matches tag for finding strings attrs offset later, biased against default zero
						pMatch[i].m_iTag = iSeg+1;

						bool bNewMatch = false;
						ARRAY_FOREACH ( iSorter, dSorters )
							bNewMatch |= dSorters[iSorter]->Push ( pMatch[i] );

						// stringptr expressions should be duplicated (or taken over) at this point
						tCtx.FreeStrSort ( pMatch[i] );
						tCtx.FreeStrFinal ( pMatch[i] );

						if ( bNewMatch )
							if ( --iCutoff==0 )
								break;
					}

					if ( iCutoff==0 )
					{
						iSeg = m_pSegments.GetLength();
						break;
					}
				}
			}
		}
	}

	//////////////////////
	// coping match's attributes to external storage in result set
	//////////////////////

	CSphVector<CSphAttrLocator> dStringGetLoc;
	CSphVector<CSphAttrLocator> dStringSetLoc;
	CSphVector<CSphAttrLocator> dMvaGetLoc;
	CSphVector<CSphAttrLocator> dMvaSetLoc;
	for ( int i=0; i<pResult->m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tSetInfo = pResult->m_tSchema.GetAttr(i);
		if ( tSetInfo.m_eAttrType==SPH_ATTR_STRING )
		{
			const int iInLocator = m_tSchema.GetAttrIndex ( tSetInfo.m_sName.cstr() );
			assert ( iInLocator>=0 );

			dStringGetLoc.Add ( m_tSchema.GetAttr ( iInLocator ).m_tLocator );
			dStringSetLoc.Add ( tSetInfo.m_tLocator );
		} else if ( tSetInfo.m_eAttrType==SPH_ATTR_UINT32SET || tSetInfo.m_eAttrType==SPH_ATTR_INT64SET )
		{
			const int iInLocator = m_tSchema.GetAttrIndex ( tSetInfo.m_sName.cstr() );
			assert ( iInLocator>=0 );

			dMvaGetLoc.Add ( m_tSchema.GetAttr ( iInLocator ).m_tLocator );
			dMvaSetLoc.Add ( tSetInfo.m_tLocator );
		}
	}

	// we're copying docinfo from RT segments to result set (segments got merged in RAM)
	// also static part of docinfo:
	// during optimize process (disk chunks are merged and removed from RT index)
	// result set has arena attributes (STRING and MVA) (all these attrs should be at one pool)
	bool bOptimizing = m_bOptimizing;
	bool bHasArenaAttrs = ( dStringSetLoc.GetLength()>0 || dMvaSetLoc.GetLength()>0 );
	const int iSegmentsTotal = m_pSegments.GetLength();
	bool bSegmentMatchesFixup = ( m_tSchema.GetStaticSize()>0 && iSegmentsTotal>0 );
	if ( bSegmentMatchesFixup || bHasArenaAttrs || bOptimizing )
	{
		MEMORY ( SPH_MEM_IDX_RT_RES_MATCHES );

		// we need to count matches for allocating arena
		// as we are going to fix match's m_pStatic pointers later
		// and copy real match's data to arena

		int iFixupCount = 0;

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];

			const CSphMatch * pMatches = pSorter->Finalize();
			const int iMatchesCount = pSorter->GetLength();

			if ( bHasArenaAttrs || bOptimizing )
			{
				iFixupCount += iMatchesCount;
				continue;
			}

			// copying only RT segments docinfo (no need to copy docinfo from disk chunks)
			for ( int i=0; i<iMatchesCount; i++ )
			{
				const int iMatchSegment = pMatches[i].m_iTag-1;
				if ( iMatchSegment>=0 && iMatchSegment< iSegmentsTotal )
					iFixupCount++;
			}
		}

		if ( iFixupCount>0 || bHasArenaAttrs || bOptimizing )
		{
			const int iStaticSize = m_tSchema.GetStaticSize() + DWSIZEOF ( SphDocID_t );
			CSphRowitem * pAttr = new CSphRowitem [ iFixupCount * iStaticSize ];
			pResult->m_dStorage2Free.Add ( (BYTE*)pAttr );
#ifndef NDEBUG
			CSphRowitem * pEnd = pAttr + iFixupCount * iStaticSize;
#endif

			ARRAY_FOREACH ( iSorter, dSorters )
			{
				ISphMatchSorter * pSorter = dSorters[iSorter];

				CSphMatch * pMatches = pSorter->Finalize();
				const int iMatchesCount = pSorter->GetLength();

				for ( int i=0; i<iMatchesCount; i++ )
				{
					const int iMatchSegment = pMatches[i].m_iTag-1;
					if ( ( iMatchSegment>=0 && iMatchSegment< iSegmentsTotal ) || bHasArenaAttrs || bOptimizing )
					{
						assert ( pAttr+iStaticSize<=pEnd );

						memcpy ( pAttr, STATIC2DOCINFO ( pMatches[i].m_pStatic ), sizeof(CSphRowitem)*iStaticSize );
						pMatches[i].m_pStatic = DOCINFO2ATTRS ( pAttr );
						DOCINFOSETID ( pAttr, (SphDocID_t)0 ); // the zero docid will show that the data was copied
						pAttr += iStaticSize;
					}
				}
			}
		}
	}


	//////////////////////
	// fixing string offset and data in resulting matches
	//////////////////////

	MEMORY ( SPH_MEM_IDX_RT_RES_STRINGS );

	if ( bHasArenaAttrs || bOptimizing )
	{
		assert ( !pResult->m_pStrings && !pResult->m_pMva );
		CSphTightVector<BYTE> dStorageString;
		CSphTightVector<DWORD> dStorageMva;
		dStorageString.Add ( 0 );
		dStorageMva.Add ( 0 );

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];

			CSphMatch * pMatches = pSorter->Finalize();
			const int iMatchesCount = pSorter->GetLength();

			for ( int i=0; i<iMatchesCount; i++ )
			{
				CSphMatch & tMatch = pMatches[i];

				const int iSegCount = m_pSegments.GetLength();
				assert ( tMatch.m_iTag>=1 && tMatch.m_iTag<iSegCount+dDiskStrings.GetLength()+1 );

				const int iStorageSrc = tMatch.m_iTag-1;
				bool bSegmentMatch = ( iStorageSrc < iSegCount );
				const BYTE * pBaseString = bSegmentMatch ? m_pSegments[iStorageSrc]->m_dStrings.Begin() : dDiskStrings[ iStorageSrc-iSegCount ];
				const DWORD * pBaseMva = bSegmentMatch ? m_pSegments[iStorageSrc]->m_dMvas.Begin() : dDiskMva[ iStorageSrc-iSegCount ];

				ARRAY_FOREACH ( i, dStringGetLoc )
				{
					DWORD uAttr = 0;
					const SphAttr_t uOff = tMatch.GetAttr ( dStringGetLoc[i] );
					if ( uOff>0 ) // have to fix up only existed attribute
					{
						assert ( uOff<( I64C(1)<<32 ) ); // should be 32 bit offset
						assert ( !bSegmentMatch || (int)uOff<m_pSegments[iStorageSrc]->m_dStrings.GetLength() );

						uAttr = CopyPackedString ( pBaseString + uOff, dStorageString );
					}

					const CSphAttrLocator & tSet = dStringSetLoc[i];
					assert ( !tSet.m_bDynamic || tSet.GetMaxRowitem() < (int)tMatch.m_pDynamic[-1] );
					sphSetRowAttr ( tSet.m_bDynamic ? tMatch.m_pDynamic : const_cast<CSphRowitem*>( tMatch.m_pStatic ), tSet, uAttr );
				}

				ARRAY_FOREACH ( i, dMvaGetLoc )
				{
					DWORD uAttr = 0;
					const DWORD * pMva = tMatch.GetAttrMVA ( dMvaGetLoc[i], pBaseMva );
					// have to fix up only existed attribute
					if ( pMva )
					{
						assert ( ( tMatch.GetAttr ( dMvaGetLoc[i] ) & MVA_ARENA_FLAG )<( I64C(1)<<32 ) ); // should be 32 bit offset
						assert ( !bSegmentMatch || (int)tMatch.GetAttr ( dMvaGetLoc[i] )<m_pSegments[iStorageSrc]->m_dMvas.GetLength() );

						uAttr = CopyMva ( pMva, dStorageMva );
					}

					const CSphAttrLocator & tSet = dMvaSetLoc[i];
					assert ( !tSet.m_bDynamic || tSet.GetMaxRowitem() < (int)tMatch.m_pDynamic[-1] );
					sphSetRowAttr ( tSet.m_bDynamic ? tMatch.m_pDynamic : const_cast<CSphRowitem*>( tMatch.m_pStatic ), tSet, uAttr );
				}
			}
		}

		if ( dStorageString.GetLength()>1 )
		{
			BYTE * pStrings = dStorageString.LeakData ();
			pResult->m_dStorage2Free.Add ( pStrings );
			pResult->m_pStrings = pStrings;
		}
		if ( dStorageMva.GetLength()>1 )
		{
			DWORD * pMva = dStorageMva.LeakData();
			pResult->m_dStorage2Free.Add ( (BYTE*)pMva );
			pResult->m_pMva = pMva;
		}
	}

	// query timer
	pResult->m_iQueryTime = int ( ( sphMicroTimer()-tmQueryStart )/1000 );
	m_tRwlock.Unlock ();
	return true;
}

bool RtIndex_t::MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults,
	ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag, bool bFactors ) const
{
	// FIXME! OPTIMIZE! implement common subtree cache here
	bool bResult = false;
	for ( int i=0; i<iQueries; i++ )
		if ( MultiQuery ( &ppQueries[i], ppResults[i], 1, &ppSorters[i], pExtraFilters, iTag, bFactors ) )
			bResult = true;
		else
			ppResults[i]->m_iMultiplier = -1;

	return bResult;
}

bool RtIndex_t::GetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, bool bGetStats, CSphString & sError ) const
{
	m_tRwlock.ReadLock(); // this is actually needed only if they want stats

	RtQword_t tQword;
	CSphString sBuffer ( sQuery );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( false ) ); // avoid race
	pTokenizer->SetBuffer ( (BYTE *)sBuffer.cstr(), sBuffer.Length() );

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDictBase = m_pDict;
	if ( pDictBase->HasState() )
	{
		tDictCloned = pDictBase = pDictBase->Clone();
	}

	CSphScopedPtr<CSphDict> tDict ( NULL );
	CSphDict * pDict = SetupStarDict ( tDict, pDictBase, pTokenizer.Ptr() );

	CSphScopedPtr<CSphDict> tDict2 ( NULL );
	pDict = SetupExactDict ( tDict2, pDict, pTokenizer.Ptr() );

	while ( BYTE * pToken = pTokenizer->GetToken() )
	{
		// keep tokenized form
		CSphString sTokenized = ( const char *)pToken;
		SphWordID_t iWord = pDict->GetWordID ( pToken );
		if ( iWord )
		{
			CSphKeywordInfo & tInfo = dKeywords.Add();
			Swap ( tInfo.m_sTokenized, sTokenized );
			tInfo.m_sNormalized = (const char *)pToken;
			tInfo.m_iDocs = 0;
			tInfo.m_iHits = 0;

			if ( tInfo.m_sNormalized.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
				*(char *)tInfo.m_sNormalized.cstr() = '=';

			if ( !bGetStats )
				continue;

			tQword.Reset();
			tQword.m_iWordID = iWord;
			tQword.m_sWord = tInfo.m_sTokenized;
			tQword.m_sDictWord = tInfo.m_sNormalized;
			ARRAY_FOREACH ( iSeg, m_pSegments )
				RtQwordSetupSegment ( &tQword, m_pSegments[iSeg], false, m_bKeywordDict, m_iWordsCheckpoint );

			tInfo.m_iDocs = tQword.m_iDocs;
			tInfo.m_iHits = tQword.m_iHits;
		}
	}

	// get stats from disk chunks too
	if ( bGetStats )
		ARRAY_FOREACH ( iChunk, m_pDiskChunks )
	{
		CSphVector<CSphKeywordInfo> dKeywords2;
		if ( !m_pDiskChunks[iChunk]->GetKeywords ( dKeywords2, sQuery, bGetStats, sError ) )
		{
			m_tRwlock.Unlock();
			return false;
		}

		if ( dKeywords.GetLength()!=dKeywords2.GetLength() )
		{
			sError.SetSprintf ( "INTERNAL ERROR: keyword count mismatch (ram=%d, disk[%d]=%d)",
				dKeywords.GetLength(), iChunk, dKeywords2.GetLength() );
			m_tRwlock.Unlock ();
			break;
		}

		ARRAY_FOREACH ( i, dKeywords )
		{
			if ( dKeywords[i].m_sTokenized!=dKeywords2[i].m_sTokenized )
			{
				sError.SetSprintf ( "INTERNAL ERROR: tokenized keyword mismatch (n=%d, ram=%s, disk[%d]=%s)",
					i, dKeywords[i].m_sTokenized.cstr(), iChunk, dKeywords2[i].m_sTokenized.cstr() );
				m_tRwlock.Unlock ();
				break;
			}

			if ( dKeywords[i].m_sNormalized!=dKeywords2[i].m_sNormalized )
			{
				sError.SetSprintf ( "INTERNAL ERROR: normalized keyword mismatch (n=%d, ram=%s, disk[%d]=%s)",
					i, dKeywords[i].m_sTokenized.cstr(), iChunk, dKeywords2[i].m_sTokenized.cstr() );
				m_tRwlock.Unlock ();
				break;
			}

			dKeywords[i].m_iDocs += dKeywords2[i].m_iDocs;
			dKeywords[i].m_iHits += dKeywords2[i].m_iHits;
		}
	}

	m_tRwlock.Unlock();
	return true;
}

// FIXME! might be inconsistent in case disk chunk update fails
int RtIndex_t::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError )
{
	// check if we have to

	assert ( tUpd.m_dDocids.GetLength()==0 || tUpd.m_dRows.GetLength()==0 );
	int iRows = Max ( tUpd.m_dDocids.GetLength(), tUpd.m_dRows.GetLength() );
	bool bRaw = tUpd.m_dDocids.GetLength()==0;
	bool bHasMva = false;

	assert ( iRows==(int)tUpd.m_dRowOffset.GetLength() );
	if ( !iRows )
		return 0;

	// remap update schema to index schema
	CSphVector<CSphAttrLocator> dLocators;
	CSphVector<int> dIndexes;
	CSphVector<bool> dFloats;
	CSphVector<bool> dBigints;
	dLocators.Reserve ( tUpd.m_dAttrs.GetLength() );
	dIndexes.Reserve ( tUpd.m_dAttrs.GetLength() );
	dFloats.Reserve ( tUpd.m_dAttrs.GetLength() );
	dBigints.Reserve ( tUpd.m_dAttrs.GetLength() ); // bigint flags for *source* schema.

	uint64_t uDst64 = 0;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		int iIndex = m_tSchema.GetAttrIndex ( tUpd.m_dAttrs[i].m_sName.cstr() );
		if ( iIndex>=0 )
		{
			// forbid updates on non-int columns
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iIndex);
			if ( !( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP
				|| tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET
				|| tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_FLOAT ))
			{
				sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, "
					"bigint, float or timestamp or MVA)", tUpd.m_dAttrs[i].m_sName.cstr() );
				return -1;
			}

			bool bSrcMva = ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );
			bool bDstMva = ( tUpd.m_dAttrs[i].m_eAttrType==SPH_ATTR_UINT32SET || tUpd.m_dAttrs[i].m_eAttrType==SPH_ATTR_INT64SET );
			if ( bSrcMva!=bDstMva )
			{
				sError.SetSprintf ( "attribute '%s' MVA flag mismatch", tUpd.m_dAttrs[i].m_sName.cstr() );
				return -1;
			}

			if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET && tUpd.m_dAttrs[i].m_eAttrType==SPH_ATTR_INT64SET )
			{
				sError.SetSprintf ( "attribute '%s' MVA bits (dst=%d, src=%d) mismatch", tUpd.m_dAttrs[i].m_sName.cstr(),
					tCol.m_eAttrType, tUpd.m_dAttrs[i].m_eAttrType );
				return -1;
			}

			if ( tCol.m_eAttrType==SPH_ATTR_INT64SET )
				uDst64 |= ( U64C(1)<<i );

			dFloats.Add ( tCol.m_eAttrType==SPH_ATTR_FLOAT );
			dLocators.Add ( tCol.m_tLocator );
			bHasMva |= ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );

		} else if ( !tUpd.m_bIgnoreNonexistent )
		{
			sError.SetSprintf ( "attribute '%s' not found", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}

		dBigints.Add ( tUpd.m_dAttrs[i].m_eAttrType==SPH_ATTR_BIGINT );

		// find dupes to optimize
		ARRAY_FOREACH ( i, dIndexes )
			if ( dIndexes[i]==iIndex )
			{
				dIndexes[i] = -1;
				break;
			}
		dIndexes.Add ( iIndex );
	}
	assert ( tUpd.m_bIgnoreNonexistent || ( dLocators.GetLength()==tUpd.m_dAttrs.GetLength() ) );

	// check if we are empty
	if ( !m_pSegments.GetLength() && !m_pDiskChunks.GetLength() )
	{
		return true;
	}

	m_tRwlock.ReadLock();

	// do the update
	int iUpdated = 0;
	DWORD uUpdateMask = 0;

	// bRaw do only one pass as it has pointers to actual data at segments
	// MVA && bRaw should find appropriate segment to update storage there

	int iFirst = ( iIndex<0 ) ? 0 : iIndex;
	int iLast = ( iIndex<0 ) ? iRows : iIndex+1;
	for ( int iUpd=iFirst; iUpd<iLast; iUpd++ )
	{
		// search segments first
		bool bUpdated = false;
		for ( int iSeg=0; iSeg<m_pSegments.GetLength() && ( !bRaw || !iSeg ); iSeg++ )
		{
			CSphRowitem * pRow = const_cast<CSphRowitem*> ( bRaw? tUpd.m_dRows[iUpd] : m_pSegments[iSeg]->FindAliveRow ( tUpd.m_dDocids[iUpd] ) );
			if ( !pRow )
				continue;

			assert ( bRaw || ( DOCINFO2ID(pRow)==tUpd.m_dDocids[iUpd] ) );
			pRow = DOCINFO2ATTRS(pRow);

			CSphTightVector<DWORD> * pStorageMVA = NULL;
			if ( bHasMva )
			{
				if ( !bRaw )
				{
					pStorageMVA = &m_pSegments[iSeg]->m_dMvas;
				} else
				{
					ARRAY_FOREACH ( iMva, m_pSegments )
					{
						const CSphTightVector<CSphRowitem> & dSegRows = m_pSegments[iMva]->m_dRows;
						if ( dSegRows.Begin()<=pRow && pRow<dSegRows.Begin()+dSegRows.GetLength() )
						{
							pStorageMVA = &m_pSegments[iMva]->m_dMvas;
							break;
						}
					}
				}
			}
			assert ( !bHasMva || pStorageMVA );

			int iPos = tUpd.m_dRowOffset[iUpd];
			ARRAY_FOREACH ( iCol, tUpd.m_dAttrs )
			{
				if ( !( tUpd.m_dAttrs[iCol].m_eAttrType==SPH_ATTR_UINT32SET || tUpd.m_dAttrs[iCol].m_eAttrType==SPH_ATTR_INT64SET ) )
				{
					if ( dIndexes[iCol]>=0 )
					{
						// plain update
						uUpdateMask |= ATTRS_UPDATED;

						SphAttr_t uValue = dBigints[iCol] ? MVA_UPSIZE ( &tUpd.m_dPool[iPos] ) : tUpd.m_dPool[iPos];
						sphSetRowAttr ( pRow, dLocators[iCol], uValue );
						iPos += dBigints[iCol]?2:1;
					}
				} else
				{
					const DWORD * pSrc = tUpd.m_dPool.Begin()+iPos;
					DWORD iLen = *pSrc;
					iPos += iLen+1;

					if ( dIndexes[iCol]>=0 )
					{
						// MVA update
						uUpdateMask |= ATTRS_MVA_UPDATED;

						if ( !iLen )
						{
							sphSetRowAttr ( pRow, dLocators[iCol], 0 );
							continue;
						}

						bool bDst64 = ( ( uDst64 & ( U64C(1) << iCol ) )!=0 );
						assert ( ( iLen%2 )==0 );
						DWORD uCount = ( bDst64 ? iLen : iLen/2 );

						DWORD uMvaOff = MVA_DOWNSIZE ( sphGetRowAttr ( pRow, dLocators[iCol] ) );
						assert ( uMvaOff<(DWORD)pStorageMVA->GetLength() );
						DWORD * pDst = pStorageMVA->Begin() + uMvaOff;
						if ( uCount>(*pDst) )
						{
							uMvaOff = pStorageMVA->GetLength();
							pStorageMVA->Resize ( uMvaOff+uCount+1 );
							pDst = pStorageMVA->Begin()+uMvaOff;
							sphSetRowAttr ( pRow, dLocators[iCol], uMvaOff );
						}

						if ( bDst64 )
						{
							memcpy ( pDst, pSrc, sizeof(DWORD)*(uCount+1) );
						} else
						{
							*pDst++ = uCount; // MVA values counter first
							pSrc++;
							while ( uCount-- )
							{
								*pDst = *pSrc;
								pDst++;
								pSrc+=2;
							}
						}
					}
				}
			}

			bUpdated = true;
			iUpdated++;
		}
		if ( bUpdated )
			continue;

		// check disk K-list now
		if ( iUpdated==1 )
		{
			m_tKlist.Flush(); // no need to lock here as it got protected here by writer locks
		}
		const SphAttr_t uRef = bRaw ? DOCINFO2ID ( tUpd.m_dRows[iUpd] ) : tUpd.m_dDocids[iUpd];
		bUpdated = ( sphBinarySearch ( m_tKlist.GetKillList(), m_tKlist.GetKillList() + m_tKlist.GetKillListSize() - 1, uRef )!=NULL );
		if ( bUpdated )
			continue;

		// finally, try disk chunks
		for ( int iChunk = m_pDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
		{
			// run just this update
			// FIXME! might be inefficient in case of big batches (redundant allocs in disk update)
			int iRes = m_pDiskChunks[iChunk]->UpdateAttributes ( tUpd, iUpd, sError );

			// errors are highly unlikely at this point
			// FIXME! maybe emit a warning to client as well?
			if ( iRes<0 )
			{
				sphWarn ( "INTERNAL ERROR: index %s chunk %d update failure: %s", m_sIndexName.cstr(), iChunk, sError.cstr() );
				continue;
			}

			// update stats
			iUpdated += iRes;

			// we only need to update the most fresh chunk
			if ( iRes>0 )
				break;
		}
	}

	// bump the counter, binlog the update!
	assert ( iIndex<0 );
	g_pBinlog->BinlogUpdateAttributes ( &m_iTID, m_sIndexName.cstr(), tUpd );

	m_tRwlock.Unlock();

	// all done
	return iUpdated;
}

//////////////////////////////////////////////////////////////////////////
// MAGIC CONVERSIONS
//////////////////////////////////////////////////////////////////////////

bool RtIndex_t::AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError )
{
	// safeguards
	// we do not support some of the disk index features in RT just yet
#define LOC_ERROR(_arg) { sError = _arg; return false; }
	const CSphIndexSettings & tSettings = pIndex->GetSettings();
	if ( tSettings.m_iBoundaryStep!=0 )
		LOC_ERROR ( "ATTACH currently requires boundary_step=0 in disk index (RT-side support not implemented yet)" );
	if ( tSettings.m_iStopwordStep!=1 )
		LOC_ERROR ( "ATTACH currently requires stopword_step=1 in disk index (RT-side support not implemented yet)" );
	if ( tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN )
		LOC_ERROR ( "ATTACH currently requires docinfo=extern in disk index (RT-side support not implemented yet)" );
#undef LOC_ERROR

	// ATTACH needs an exclusive global lock on both indexes
	// source disk index must come in locked internally
	// target RT index lock is acquired here
	Verify ( m_tWriterMutex.Lock() );
	Verify ( m_tSaveOuterMutex.Lock() );
	Verify ( m_tRwlock.WriteLock() );

	// for now, let's do the simplest possible thing
	// and attach new data to empty RT indexes only
	bool bHasData = ( m_pDiskChunks.GetLength()!=0 );
	ARRAY_FOREACH_COND ( i, m_pSegments, !bHasData )
		bHasData = ( m_pSegments[i]->m_iAliveRows!=0 );

	if ( bHasData )
	{
		Verify ( m_tRwlock.Unlock() );
		Verify ( m_tSaveOuterMutex.Unlock() );
		Verify ( m_tWriterMutex.Unlock() );
		sError.SetSprintf ( "ATTACH currently supports empty target RT indexes only" );
		return false;
	}

	// rename that source index to our chunk0
	CSphString sChunk;
	sChunk.SetSprintf ( "%s.0", m_sPath.cstr() );
	if ( !pIndex->Rename ( sChunk.cstr() ) )
	{
		Verify ( m_tRwlock.Unlock() );
		Verify ( m_tSaveOuterMutex.Unlock() );
		Verify ( m_tWriterMutex.Unlock() );
		sError.SetSprintf ( "ATTACH failed: %s", pIndex->GetLastError().cstr() );
		return false;
	}

	// copy schema from chunk0 schema
	m_tSchema = pIndex->GetMatchSchema();
	m_tStats = pIndex->GetStats();
	m_iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	// copy tokenizer, dict etc settings from chunk0
	SafeDelete ( m_pTokenizer );
	SafeDelete ( m_pDict );

	m_tSettings = pIndex->GetSettings();
	m_tSettings.m_dBigramWords.Reset();
	m_tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;

	m_pTokenizer = pIndex->GetTokenizer()->Clone ( false );
	m_pDict = pIndex->GetDictionary()->Clone ();
	PostSetup();

	// FIXME? what about copying m_TID etc?

	// recreate disk chunk list, resave header file
	m_iDiskBase = 0;
	m_pDiskChunks.Add ( pIndex );
	SaveMeta ( m_pDiskChunks.GetLength(), m_iTID );

	// FIXME? do something about binlog too?
	// g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// all done
	Verify ( m_tRwlock.Unlock() );
	Verify ( m_tSaveOuterMutex.Unlock() );
	Verify ( m_tWriterMutex.Unlock() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// TRUNCATE
//////////////////////////////////////////////////////////////////////////

bool RtIndex_t::Truncate ( CSphString & )
{
	// TRUNCATE needs an exclusive lock, so get it
	Verify ( m_tWriterMutex.Lock() );
	Verify ( m_tSaveOuterMutex.Lock() );
	Verify ( m_tRwlock.WriteLock() );

	// update and save meta
	// indicate 0 disk chunks, we are about to kill them anyway
	// current TID will be saved, so replay will properly skip preceding txns
	m_iDiskBase = 0;
	m_tStats.Reset();
	SaveMeta ( 0, m_iTID );

	// allow binlog to unlink now-redundant data files
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// kill RAM chunk file
	CSphString sFile;
	sFile.SetSprintf ( "%s.ram", m_sPath.cstr() );
	if ( ::unlink ( sFile.cstr() ) )
		if ( errno!=ENOENT )
			sphWarning ( "rt: truncate failed to unlink %s: %s", sFile.cstr(), strerror(errno) );

	// kill all disk chunks files
	ARRAY_FOREACH ( i, m_pDiskChunks )
	{
		sFile.SetSprintf ( "%s.%d", m_sPath.cstr(), i );
		sphUnlinkIndex ( sFile.cstr(), false );
	}

	// kill in-memory data, reset stats
	ARRAY_FOREACH ( i, m_pDiskChunks )
		SafeDelete ( m_pDiskChunks[i] );
	m_pDiskChunks.Reset();

	ARRAY_FOREACH ( i, m_pSegments )
		SafeDelete ( m_pSegments[i] );
	m_pSegments.Reset();

	// done, unlock
	Verify ( m_tRwlock.Unlock() );
	Verify ( m_tSaveOuterMutex.Unlock() );
	Verify ( m_tWriterMutex.Unlock() );

	return true;
}


//////////////////////////////////////////////////////////////////////////
// OPTIMIZE
//////////////////////////////////////////////////////////////////////////

void RtIndex_t::Optimize ( volatile bool * pForceTerminate, ThrottleState_t * pThrottle )
{
	assert ( pForceTerminate && pThrottle );
	int64_t tmStart = sphMicroTimer();
	int iChunks = m_pDiskChunks.GetLength();
	CSphSchema tSchema = m_tSchema;
	CSphString sError;

	while ( m_pDiskChunks.GetLength()>1 && !*pForceTerminate )
	{
		m_bOptimizing = true;
		CSphTightVector<SphAttr_t> dKlist;
		Verify ( m_tRwlock.ReadLock () );

		// make kill-list
		// initially add RAM kill-list
		m_tKlist.Flush();
		dKlist.Resize ( m_tKlist.GetKillListSize() );
		memcpy ( dKlist.Begin(), m_tKlist.GetKillList(), sizeof(SphAttr_t)*m_tKlist.GetKillListSize() );

		// add disk chunks kill-lists
		for ( int iChunk=1; iChunk<m_pDiskChunks.GetLength(); iChunk++ )
		{
			const CSphIndex * pIndex = m_pDiskChunks[iChunk];
			if ( !pIndex->GetKillListSize() )
				continue;

			int iOff = dKlist.GetLength();
			dKlist.Resize ( iOff+pIndex->GetKillListSize() );
			memcpy ( dKlist.Begin()+iOff, pIndex->GetKillList(), sizeof(SphAttr_t)*pIndex->GetKillListSize() );

			// get rid of duplicates on each iteration to keep memory consumption low
			dKlist.Uniq();
		}

		// merge 'older'(pSrc) to 'oldest'(pDst) and get 'merged' that names like 'oldest'+.tmp
		// to got rid of keeping actual kill-list
		// however 'merged' got placed at 'older' position and 'merged' renamed to 'older' name

		const CSphIndex * pOldest = m_pDiskChunks[0];
		const CSphIndex * pOlder = m_pDiskChunks[1];

		CSphString sOlder, sOldest, sRename, sMerged;
		sOlder.SetSprintf ( "%s", pOlder->GetFilename() );
		sOldest.SetSprintf ( "%s", pOldest->GetFilename() );
		sRename.SetSprintf ( "%s.old", pOlder->GetFilename() );
		sMerged.SetSprintf ( "%s.tmp", pOldest->GetFilename() );

		Verify ( m_tRwlock.Unlock() );

		// check forced exit after long operation
		if ( *pForceTerminate )
			break;

		// create filter from kill-list
		CSphScopedPtr<ISphFilter> pFilter ( NULL );
		if ( dKlist.GetLength() )
		{
			CSphFilterSettings tFilterSettings;
			tFilterSettings.m_bExclude = true;
			tFilterSettings.m_eType = SPH_FILTER_VALUES;
			tFilterSettings.m_iMinValue = dKlist[0];
			tFilterSettings.m_iMaxValue = dKlist.Last();
			tFilterSettings.m_sAttrName = "@id";
			tFilterSettings.SetExternalValues ( dKlist.Begin(), dKlist.GetLength() );
			pFilter = sphCreateFilter ( tFilterSettings, tSchema, NULL, NULL, sError );
		}

		// merge data to disk ( data is constant during that phase )
		CSphIndexProgress tProgress;
		bool bMerged = sphMerge ( pOldest, pOlder, pFilter.Ptr(), sError, tProgress, pThrottle );
		if ( !bMerged )
		{
			sphWarning ( "rt optimize: index %s: failed to merge %s to %s (error %s)",
				m_sIndexName.cstr(), sOlder.cstr(), sOldest.cstr(), sError.cstr() );
			break;
		}
		// check forced exit after long operation
		if ( *pForceTerminate )
			break;

		CSphScopedPtr<CSphIndex> pMerged ( LoadDiskChunk ( sMerged.cstr(), sError ) );
		if ( !pMerged.Ptr() )
		{
			sphWarning ( "rt optimize: index %s: failed to load merged chunk (error %s)",
				m_sIndexName.cstr(), sError.cstr() );
			break;
		}
		// check forced exit after long operation
		if ( *pForceTerminate )
			break;

		// lets rotate indexes
		Verify ( m_tWriterMutex.Lock() );
		Verify ( m_tSaveOuterMutex.Lock() );
		Verify ( m_tRwlock.WriteLock() );

		// rename older disk chunk to 'old'
		if ( !const_cast<CSphIndex *>( pOlder )->Rename ( sRename.cstr() ) )
		{
			sphWarning ( "rt optimize: index %s: cur to old rename failed (error %s)",
				m_sIndexName.cstr(), pOlder->GetLastError().cstr() );
			Verify ( m_tRwlock.Unlock() );
			Verify ( m_tSaveOuterMutex.Unlock() );
			Verify ( m_tWriterMutex.Unlock() );
			break;
		}
		// rename merged disk chunk to 0
		if ( !pMerged->Rename ( sOlder.cstr() ) )
		{
			sphWarning ( "rt optimize: index %s: merged to cur rename failed (error %s)",
				m_sIndexName.cstr(), pMerged->GetLastError().cstr() );
			if ( !const_cast<CSphIndex *>( pOlder )->Rename ( sOlder.cstr() ) )
			{
				sphWarning ( "rt optimize: index %s: old to cur rename failed (error %s)",
					m_sIndexName.cstr(), pOlder->GetLastError().cstr() );
			}
			Verify ( m_tRwlock.Unlock() );
			Verify ( m_tSaveOuterMutex.Unlock() );
			Verify ( m_tWriterMutex.Unlock() );
			break;
		}

		m_pDiskChunks[1] = pMerged.LeakPtr();
		m_pDiskChunks.Remove ( 0 );
		m_iDiskBase++;

		SaveMeta ( m_pDiskChunks.GetLength(), m_iTID );

		Verify ( m_tRwlock.Unlock() );
		Verify ( m_tSaveOuterMutex.Unlock() );
		Verify ( m_tWriterMutex.Unlock() );

		if ( *pForceTerminate )
		{
			sphWarning ( "rt optimize: index %s: forced to shutdown, remove old index files manually '%s', '%s'",
				m_sIndexName.cstr(), sRename.cstr(), sOldest.cstr() );
			break;
		}

		SafeDelete ( pOlder );
		SafeDelete ( pOldest );

		// we might remove old index files
		sphUnlinkIndex ( sRename.cstr(), true );
		sphUnlinkIndex ( sOldest.cstr(), true );
		// FIXEME: wipe out 'merged' index files in case of error
	}

	m_bOptimizing = false;
	int64_t tmPass = sphMicroTimer() - tmStart;

	sphInfo ( "rt: index %s: optimized chunk(s) %d ( of %d ) in %d.%03d sec",
		m_sIndexName.cstr(), iChunks-m_pDiskChunks.GetLength(), iChunks, (int)(tmPass/1000000), (int)((tmPass/1000)%1000) );
}

//////////////////////////////////////////////////////////////////////////
// BINLOG
//////////////////////////////////////////////////////////////////////////

extern DWORD g_dSphinxCRC32 [ 256 ];


static CSphString MakeBinlogName ( const char * sPath, int iExt )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.%03d", sPath, iExt );
	return sName;
}


BinlogWriter_c::BinlogWriter_c ()
{
	m_iLastWritePos = 0;
	m_iLastFsyncPos = 0;
	ResetCrc();
}


void BinlogWriter_c::ResetCrc ()
{
	m_uCRC = ~((DWORD)0);
}


void BinlogWriter_c::PutBytes ( const void * pData, int iSize )
{
	BYTE * b = (BYTE*) pData;
	for ( int i=0; i<iSize; i++ )
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ *b++) & 0xff ];
	CSphWriter::PutBytes ( pData, iSize );
}


void BinlogWriter_c::PutString ( const char * szString )
{
	int iLen = szString ? strlen ( szString ) : 0;
	ZipValue ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void BinlogWriter_c::ZipValue ( uint64_t uValue )
{
	BYTE uBuf[16];
	int iLen = 0;

	while ( uValue>=0x80 )
	{
		uBuf[iLen++] = (BYTE)( 0x80 | ( uValue & 0x7f ) );
		uValue >>= 7;
	}
	uBuf[iLen++] = (BYTE)uValue;

	PutBytes ( uBuf, iLen );
}


void BinlogWriter_c::WriteCrc ()
{
	m_uCRC = ~m_uCRC;
	CSphWriter::PutDword ( m_uCRC );
	m_uCRC = ~((DWORD)0);
}


void BinlogWriter_c::Flush ()
{
	Write();
	Fsync();
}


void BinlogWriter_c::Write ()
{
	if ( m_iPoolUsed<=0 )
		return;

	CSphWriter::Flush();
	m_iLastWritePos = GetPos();
}


#if USE_WINDOWS
int fsync ( int iFD )
{
	// map fd to handle
	HANDLE h = (HANDLE) _get_osfhandle ( iFD );
	if ( h==INVALID_HANDLE_VALUE )
	{
		errno = EBADF;
		return -1;
	}

	// do flush
	if ( FlushFileBuffers(h) )
		return 0;

	// error handling
	errno = EIO;
	if ( GetLastError()==ERROR_INVALID_HANDLE )
		errno = EINVAL;
	return -1;
}
#endif


void BinlogWriter_c::Fsync ()
{
	if ( !HasUnsyncedData() )
		return;

	m_bError = ( fsync ( m_iFD )!=0 );
	if ( m_bError && m_pError )
		m_pError->SetSprintf ( "failed to sync %s: %s" , m_sName.cstr(), strerror(errno) );

	m_iLastFsyncPos = GetPos();
}

//////////////////////////////////////////////////////////////////////////

void BinlogReader_c::ResetCrc ()
{
	m_uCRC = ~(DWORD(0));
}


void BinlogReader_c::GetBytes ( void * pData, int iSize )
{
	CSphReader::GetBytes ( pData, iSize );
	BYTE * b = (BYTE*) pData;
	for ( int i=0; i<iSize; i++ )
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ *b++) & 0xff ];
}


DWORD BinlogReader_c::GetDword ()
{
	DWORD uRes;
	GetBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


CSphString BinlogReader_c::GetString ()
{
	CSphString sRes;
	int iLen = (int) UnzipValue();
	if ( iLen )
	{
		sRes.Reserve ( iLen );
		GetBytes ( (BYTE*)sRes.cstr(), iLen );
	}
	return sRes;
}


uint64_t BinlogReader_c::UnzipValue ()
{
	uint64_t uRes = 0;
	int iOff = 0, iByte;
	do
	{
		iByte = CSphReader::GetByte();
		uRes += ( (uint64_t)( iByte & 0x7f ) << iOff );
		iOff += 7;
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ (BYTE)iByte) & 0xff ];
	} while ( iByte>=128 );

	return uRes;
}


bool BinlogReader_c::CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos )
{
	DWORD uRef = CSphAutoreader::GetDword();
	m_uCRC = ~m_uCRC;
	if ( uRef!=m_uCRC )
		sphWarning ( "binlog: %s: CRC mismatch (index=%s, tid="INT64_FMT", pos="INT64_FMT")", sOp, sIndexName ? sIndexName : "", iTid, iTxnPos );
	return uRef==m_uCRC;
}

//////////////////////////////////////////////////////////////////////////

RtBinlog_c::RtBinlog_c ()
	: m_iFlushTimeLeft ( 0 )
	, m_iFlushPeriod ( BINLOG_AUTO_FLUSH )
	, m_eOnCommit ( ACTION_NONE )
	, m_iLockFD ( -1 )
	, m_bReplayMode ( false )
	, m_bDisabled ( true )
	, m_iRestartSize ( 0 )
{
	MEMORY ( SPH_MEM_BINLOG );

	Verify ( m_tWriteLock.Init() );

	m_tWriter.SetBufferSize ( BINLOG_WRITE_BUFFER );
}

RtBinlog_c::~RtBinlog_c ()
{
	if ( !m_bDisabled )
	{
		m_iFlushPeriod = 0;
		if ( m_eOnCommit!=ACTION_FSYNC )
			sphThreadJoin ( &m_tUpdateTread );

		DoCacheWrite();
		m_tWriter.CloseFile();
		LockFile ( false );
	}

	Verify ( m_tWriteLock.Done() );
}


void RtBinlog_c::BinlogCommit ( int64_t * pTID, const char * sIndexName, const RtSegment_t * pSeg,
	const CSphVector<SphDocID_t> & dKlist, bool bKeywordDict )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_COMMIT );
	m_tWriter.ZipValue ( uIndex );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );

	// save txn data
	if ( !pSeg || !pSeg->m_iRows )
	{
		m_tWriter.ZipValue ( 0 );
	} else
	{
		m_tWriter.ZipValue ( pSeg->m_iRows );
		SaveVector ( m_tWriter, pSeg->m_dWords );
		m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints.GetLength() );
		if ( !bKeywordDict )
		{
			ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
			{
				m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_iOffset );
				m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_iWordID );
			}
		} else
		{
			const char * pBase = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
			ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
			{
				m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_iOffset );
				m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_sWord - pBase );
			}
		}
		SaveVector ( m_tWriter, pSeg->m_dDocs );
		SaveVector ( m_tWriter, pSeg->m_dHits );
		SaveVector ( m_tWriter, pSeg->m_dRows );
		SaveVector ( m_tWriter, pSeg->m_dStrings );
		SaveVector ( m_tWriter, pSeg->m_dMvas );
		SaveVector ( m_tWriter, pSeg->m_dKeywordCheckpoints );
	}
	SaveVector ( m_tWriter, dKlist );

	// checksum
	m_tWriter.WriteCrc ();

	// finalize
	CheckDoFlush();
	CheckDoRestart();
	Verify ( m_tWriteLock.Unlock() );
}

void RtBinlog_c::BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_UPDATE_ATTRS );
	m_tWriter.ZipValue ( uIndex );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );

	// update data
	m_tWriter.ZipValue ( tUpd.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		m_tWriter.PutString ( tUpd.m_dAttrs[i].m_sName.cstr() );
		m_tWriter.ZipValue ( tUpd.m_dAttrs[i].m_eAttrType );
	}

	CSphVector<SphDocID_t> dActiveDocids;
	bool bUseRaw = false;
	if ( tUpd.m_dDocids.GetLength()==0 && tUpd.m_dRows.GetLength()!=0 )
	{
		bUseRaw = true;
		dActiveDocids.Resize ( tUpd.m_dRows.GetLength() );
		ARRAY_FOREACH ( i, tUpd.m_dRows )
			dActiveDocids[i] = DOCINFO2ID ( tUpd.m_dRows[i] );
	}
	const CSphVector<SphDocID_t> & dBinlogDocids = bUseRaw ? dActiveDocids : tUpd.m_dDocids;

	// POD vectors
	SaveVector ( m_tWriter, tUpd.m_dPool );
	SaveVector ( m_tWriter, dBinlogDocids );
	dActiveDocids.Reset();
	SaveVector ( m_tWriter, tUpd.m_dRowOffset );



	// checksum
	m_tWriter.WriteCrc ();

	// finalize
	CheckDoFlush();
	CheckDoRestart();
	Verify ( m_tWriteLock.Unlock() );
}

// here's been going binlogs with ALL closed indices removing
void RtBinlog_c::NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown )
{
	if ( m_bReplayMode )
		sphInfo ( "index '%s': ramchunk saved. TID="INT64_FMT"", sIndexName, iTID );

	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	assert ( bShutdown || m_dLogFiles.GetLength() );

	Verify ( m_tWriteLock.Lock() );

	bool bCurrentLogShut = false;
	const int iPreflushFiles = m_dLogFiles.GetLength();

	// loop through all log files, and check if we can unlink any
	ARRAY_FOREACH ( iLog, m_dLogFiles )
	{
		BinlogFileDesc_t & tLog = m_dLogFiles[iLog];
		bool bUsed = false;

		// update index info for this log file
		ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
		{
			BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

			// this index was just flushed, update flushed TID
			if ( tIndex.m_sName==sIndexName )
			{
				assert ( iTID>=tIndex.m_iFlushedTID );
				tIndex.m_iFlushedTID = Max ( tIndex.m_iFlushedTID, iTID );
			}

			// if max logged TID is greater than last flushed TID, log file still has needed recovery data
			if ( tIndex.m_iFlushedTID < tIndex.m_iMaxTID )
				bUsed = true;
		}

		// it's needed, keep looking
		if ( bUsed )
			continue;

		// hooray, we can remove this log!
		// if this is our current log, we have to close it first
		if ( iLog==m_dLogFiles.GetLength()-1 )
		{
			m_tWriter.CloseFile ();
			bCurrentLogShut = true;
		}

		// do unlink
		CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );
		if ( ::unlink ( sLog.cstr() ) )
			sphWarning ( "binlog: failed to unlink %s: %s (remove it manually)", sLog.cstr(), strerror(errno) );

		// we need to reset it, otherwise there might be leftover data after last Remove()
		m_dLogFiles[iLog] = BinlogFileDesc_t();
		// quit tracking it
		m_dLogFiles.Remove ( iLog-- );
	}

	if ( bCurrentLogShut && !bShutdown )
	{
		// if current log was closed, we need a new one (it will automatically save meta, too)
		OpenNewLog ();

	} else if ( iPreflushFiles!=m_dLogFiles.GetLength() )
	{
		// if we unlinked any logs, we need to save meta, too
		SaveMeta ();
	}

	Verify ( m_tWriteLock.Unlock() );
}

void RtBinlog_c::Configure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	MEMORY ( SPH_MEM_BINLOG );

	const int iMode = hSearchd.GetInt ( "binlog_flush", 2 );
	switch ( iMode )
	{
		case 0:		m_eOnCommit = ACTION_NONE; break;
		case 1:		m_eOnCommit = ACTION_FSYNC; break;
		case 2:		m_eOnCommit = ACTION_WRITE; break;
		default:	sphDie ( "unknown binlog flush mode %d (must be 0, 1, or 2)\n", iMode );
	}

#ifndef DATADIR
#define DATADIR "."
#endif

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : DATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	m_iRestartSize = hSearchd.GetSize ( "binlog_max_log_size", m_iRestartSize );

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LoadMeta();
	}
}

void RtBinlog_c::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags,
	ProgressCallbackSimple_t * pfnProgressCallback )
{
	if ( m_bDisabled || !hIndexes.GetLength() )
		return;

	// on replay started
	if ( pfnProgressCallback )
		pfnProgressCallback();

	int64_t tmReplay = sphMicroTimer();
	// do replay
	m_bReplayMode = true;
	int iLastLogState = 0;
	ARRAY_FOREACH ( i, m_dLogFiles )
	{
		iLastLogState = ReplayBinlog ( hIndexes, uReplayFlags, i );
		if ( pfnProgressCallback ) // on each replayed binlog
			pfnProgressCallback();
	}

	if ( m_dLogFiles.GetLength()>0 )
	{
		tmReplay = sphMicroTimer() - tmReplay;
		sphInfo ( "binlog: finished replaying total %d in %d.%03d sec",
			m_dLogFiles.GetLength(),
			(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );
	}

	// FIXME?
	// in some cases, indexes might had been flushed during replay
	// and we might therefore want to update m_iFlushedTID everywhere
	// but for now, let's just wait until next flush for simplicity

	// resume normal operation
	m_bReplayMode = false;
	OpenNewLog ( iLastLogState );
}

void RtBinlog_c::CreateTimerThread ()
{
	if ( !m_bDisabled && m_eOnCommit!=ACTION_FSYNC )
	{
		m_iFlushTimeLeft = sphMicroTimer() + m_iFlushPeriod;
		sphThreadCreate ( &m_tUpdateTread, RtBinlog_c::DoAutoFlush, this );
	}
}

void RtBinlog_c::DoAutoFlush ( void * pBinlog )
{
	assert ( pBinlog );
	RtBinlog_c * pLog = (RtBinlog_c *)pBinlog;
	assert ( !pLog->m_bDisabled );

	while ( pLog->m_iFlushPeriod>0 )
	{
		if ( pLog->m_iFlushTimeLeft < sphMicroTimer() )
		{
			MEMORY ( SPH_MEM_BINLOG );

			pLog->m_iFlushTimeLeft = sphMicroTimer() + pLog->m_iFlushPeriod;

			if ( pLog->m_eOnCommit==ACTION_NONE || pLog->m_tWriter.HasUnwrittenData() )
			{
				Verify ( pLog->m_tWriteLock.Lock() );
				pLog->m_tWriter.Flush();
				Verify ( pLog->m_tWriteLock.Unlock() );
			}

			if ( pLog->m_tWriter.HasUnsyncedData() )
				pLog->m_tWriter.Fsync();
		}

		// sleep N msec before next iter or terminate because of shutdown
		sphSleepMsec ( 100 );
	}
}

int RtBinlog_c::GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow )
{
	MEMORY ( SPH_MEM_BINLOG );
	assert ( m_dLogFiles.GetLength() );

	// OPTIMIZE? maybe hash them?
	BinlogFileDesc_t & tLog = m_dLogFiles.Last();
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( tIndex.m_sName==sName )
		{
			tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
			tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmNow );
			return i;
		}
	}

	// create a new entry
	int iID = tLog.m_dIndexInfos.GetLength();
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add(); // caller must hold a wlock
	tIndex.m_sName = sName;
	tIndex.m_iMinTID = iTID;
	tIndex.m_iMaxTID = iTID;
	tIndex.m_iFlushedTID = 0;
	tIndex.m_tmMin = tmNow;
	tIndex.m_tmMax = tmNow;

	// log this new entry
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_ADD_INDEX );
	m_tWriter.ZipValue ( iID );
	m_tWriter.PutString ( sName );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );
	m_tWriter.WriteCrc ();

	// return the index
	return iID;
}

void RtBinlog_c::LoadMeta ()
{
	MEMORY ( SPH_MEM_BINLOG );

	CSphString sMeta;
	sMeta.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return;

	CSphString sError;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, sError ) )
		sphDie ( "%s error: %s", sMeta.cstr(), sError.cstr() );

	if ( rdMeta.GetDword()!=BINLOG_META_MAGIC )
		sphDie ( "invalid meta file %s", sMeta.cstr() );

	// binlog meta v1 was dev only, crippled, and we don't like it anymore
	// binlog metas v2 upto current v4 (and likely up) share the same simplistic format
	// so let's support empty (!) binlogs w/ known versions and compatible metas
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==1 || uVersion>BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version",
			sMeta.cstr(), uVersion, BINLOG_VERSION );

	const bool bLoaded64bit = ( rdMeta.GetByte()==1 );
	m_dLogFiles.Resize ( rdMeta.UnzipInt() ); // FIXME! sanity check

	if ( !m_dLogFiles.GetLength() )
		return;

	// ok, so there is actual recovery data
	// let's require that exact version and bitness, then
	if ( uVersion!=BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version",
			sMeta.cstr(), uVersion, BINLOG_VERSION );

	if ( bLoaded64bit!=USE_64BIT )
		sphDie ( "USE_64BIT inconsistency (binary=%d, binlog=%d); recovery requires previous binary version",
			USE_64BIT, bLoaded64bit );

	// load list of active log files
	ARRAY_FOREACH ( i, m_dLogFiles )
		m_dLogFiles[i].m_iExt = rdMeta.UnzipInt(); // everything else is saved in logs themselves
}

void RtBinlog_c::SaveMeta ()
{
	MEMORY ( SPH_MEM_BINLOG );

	CSphString sMeta, sMetaOld;
	sMeta.SetSprintf ( "%s/binlog.meta.new", m_sLogPath.cstr() );
	sMetaOld.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );

	CSphString sError;

	// opened and locked, lets write
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMeta, sError ) )
		sphDie ( "failed to open '%s': '%s'", sMeta.cstr(), sError.cstr() );

	wrMeta.PutDword ( BINLOG_META_MAGIC );
	wrMeta.PutDword ( BINLOG_VERSION );
	wrMeta.PutByte ( USE_64BIT );

	// save list of active log files
	wrMeta.ZipInt ( m_dLogFiles.GetLength() );
	ARRAY_FOREACH ( i, m_dLogFiles )
		wrMeta.ZipInt ( m_dLogFiles[i].m_iExt ); // everything else is saved in logs themselves

	wrMeta.CloseFile();

	if ( ::rename ( sMeta.cstr(), sMetaOld.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMeta.cstr(), sMetaOld.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully
	sphLogDebug ( "SaveMeta: Done." );
}

void RtBinlog_c::LockFile ( bool bLock )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.lock", m_sLogPath.cstr() );

	if ( bLock )
	{
		assert ( m_iLockFD==-1 );
		const int iLockFD = ::open ( sName.cstr(), SPH_O_NEW, 0644 );

		if ( iLockFD<0 )
			sphDie ( "failed to open '%s': %u '%s'", sName.cstr(), errno, strerror(errno) );

		if ( !sphLockEx ( iLockFD, false ) )
			sphDie ( "failed to lock '%s': %u '%s'", sName.cstr(), errno, strerror(errno) );

		m_iLockFD = iLockFD;
	} else
	{
		if ( m_iLockFD>=0 )
			sphLockUn ( m_iLockFD );
		SafeClose ( m_iLockFD );
		::unlink ( sName.cstr()	);
	}
}

void RtBinlog_c::OpenNewLog ( int iLastState )
{
	MEMORY ( SPH_MEM_BINLOG );

	// calc new ext
	int iExt = 1;
	if ( m_dLogFiles.GetLength() )
	{
		iExt = m_dLogFiles.Last().m_iExt;
		if ( !iLastState )
			iExt++;
	}

	// create entry
	// we need to reset it, otherwise there might be leftover data after last Remove()
	BinlogFileDesc_t tLog;
	tLog.m_iExt = iExt;
	m_dLogFiles.Add ( tLog );

	// create file
	CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );

	if ( !iLastState ) // reuse the last binlog since it is empty or useless.
		::unlink ( sLog.cstr() );

	if ( !m_tWriter.OpenFile ( sLog.cstr(), m_sWriterError ) )
		sphDie ( "failed to create %s: errno=%d, error=%s", sLog.cstr(), errno, strerror(errno) );

	// emit header
	m_tWriter.PutDword ( BINLOG_HEADER_MAGIC );
	m_tWriter.PutDword ( BINLOG_VERSION );

	// update meta
	SaveMeta();
}

void RtBinlog_c::DoCacheWrite ()
{
	if ( !m_dLogFiles.GetLength() )
		return;
	const CSphVector<BinlogIndexInfo_t> & dIndexes = m_dLogFiles.Last().m_dIndexInfos;

	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_ADD_CACHE );
	m_tWriter.ZipValue ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
	{
		m_tWriter.PutString ( dIndexes[i].m_sName.cstr() );
		m_tWriter.ZipValue ( dIndexes[i].m_iMinTID );
		m_tWriter.ZipValue ( dIndexes[i].m_iMaxTID );
		m_tWriter.ZipValue ( dIndexes[i].m_iFlushedTID );
		m_tWriter.ZipValue ( dIndexes[i].m_tmMin );
		m_tWriter.ZipValue ( dIndexes[i].m_tmMax );
	}
	m_tWriter.WriteCrc ();
}

void RtBinlog_c::CheckDoRestart ()
{
	// restart on exceed file size limit
	if ( m_iRestartSize>0 && m_tWriter.GetPos()>m_iRestartSize )
	{
		MEMORY ( SPH_MEM_BINLOG );

		assert ( m_dLogFiles.GetLength() );

		DoCacheWrite();
		m_tWriter.CloseFile();
		OpenNewLog();
	}
}

void RtBinlog_c::CheckDoFlush ()
{
	if ( m_eOnCommit==ACTION_NONE )
		return;

	if ( m_eOnCommit==ACTION_WRITE && m_tWriter.HasUnwrittenData() )
		m_tWriter.Write();

	if ( m_eOnCommit==ACTION_FSYNC && m_tWriter.HasUnsyncedData() )
	{
		if ( m_tWriter.HasUnwrittenData() )
			m_tWriter.Write();

		m_tWriter.Fsync();
	}
}

int RtBinlog_c::ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, int iBinlog )
{
	assert ( iBinlog>=0 && iBinlog<m_dLogFiles.GetLength() );
	CSphString sError;

	const CSphString sLog ( MakeBinlogName ( m_sLogPath.cstr(), m_dLogFiles[iBinlog].m_iExt ) );
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// open, check, play
	sphInfo ( "binlog: replaying log %s", sLog.cstr() );

	BinlogReader_c tReader;
	if ( !tReader.Open ( sLog, sError ) )
		sphDie ( "binlog: log open error: %s", sError.cstr() );

	const SphOffset_t iFileSize = tReader.GetFilesize();

	if ( !iFileSize )
	{
		sphWarning ( "binlog: empty binlog %s detected, skipping", sLog.cstr() );
		return -1;
	}

	if ( tReader.GetDword()!=BINLOG_HEADER_MAGIC )
		sphDie ( "binlog: log %s missing magic header (corrupted?)", sLog.cstr() );

	DWORD uVersion = tReader.GetDword();
	if ( uVersion!=BINLOG_VERSION || tReader.GetErrorFlag() )
		sphDie ( "binlog: log %s is v.%d, binary is v.%d; recovery requires previous binary version", sLog.cstr(), uVersion, BINLOG_VERSION );

	/////////////
	// do replay
	/////////////

	int dTotal [ BLOP_TOTAL+1 ];
	memset ( dTotal, 0, sizeof(dTotal) );

	// !COMMIT
	// instead of simply replaying everything, we should check whether this binlog is clean
	// by loading and checking the cache stored at its very end
	tLog.m_dIndexInfos.Reset();

	bool bReplayOK = true;
	bool bHaveCacheOp = false;
	int64_t iPos = -1;

	m_iReplayedRows = 0;
	int64_t tmReplay = sphMicroTimer();

	while ( iFileSize!=tReader.GetPos() && !tReader.GetErrorFlag() && bReplayOK )
	{
		iPos = tReader.GetPos();
		if ( tReader.GetDword()!=BLOP_MAGIC )
		{
			sphDie ( "binlog: log missing txn marker at pos="INT64_FMT" (corrupted?)", iPos );
			break;
		}

		tReader.ResetCrc ();
		const uint64_t uOp = tReader.UnzipValue ();

		if ( uOp<=0 || uOp>=BLOP_TOTAL )
			sphDie ( "binlog: unexpected entry (blop="UINT64_FMT", pos="INT64_FMT")", uOp, iPos );

		// FIXME! blop might be OK but skipped (eg. index that is no longer)
		switch ( uOp )
		{
			case BLOP_COMMIT:
				bReplayOK = ReplayCommit ( iBinlog, uReplayFlags, tReader );
				break;

			case BLOP_UPDATE_ATTRS:
				bReplayOK = ReplayUpdateAttributes ( iBinlog, tReader );
				break;

			case BLOP_ADD_INDEX:
				bReplayOK = ReplayIndexAdd ( iBinlog, hIndexes, tReader );
				break;

			case BLOP_ADD_CACHE:
				if ( bHaveCacheOp )
					sphDie ( "binlog: internal error, second BLOP_ADD_CACHE detected (corruption?)" );
				bHaveCacheOp = true;
				bReplayOK = ReplayCacheAdd ( iBinlog, tReader );
				break;

			default:
				sphDie ( "binlog: internal error, unhandled entry (blop=%d)", (int)uOp );
		}

		dTotal [ uOp ] += bReplayOK?1:0;
		dTotal [ BLOP_TOTAL ]++;
	}

	tmReplay = sphMicroTimer() - tmReplay;

	if ( tReader.GetErrorFlag() )
		sphWarning ( "binlog: log io error at pos="INT64_FMT": %s", iPos, sError.cstr() );

	if ( !bReplayOK )
		sphWarning ( "binlog: replay error at pos="INT64_FMT")", iPos );

	// show additional replay statistics
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		const BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( !hIndexes ( tIndex.m_sName.cstr() ) )
		{
			sphWarning ( "binlog: index %s: missing; tids "INT64_FMT" to "INT64_FMT" skipped!",
				tIndex.m_sName.cstr(), tIndex.m_iMinTID, tIndex.m_iMaxTID );

		} else if ( tIndex.m_iPreReplayTID < tIndex.m_iMaxTID )
		{
			sphInfo ( "binlog: index %s: recovered from tid "INT64_FMT" to tid "INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );

		} else
		{
			sphInfo ( "binlog: index %s: skipped at tid "INT64_FMT" and max binlog tid "INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );
		}
	}

	sphInfo ( "binlog: replay stats: %d rows in %d commits; %d updates; %d indexes",
		m_iReplayedRows, dTotal[BLOP_COMMIT], dTotal[BLOP_UPDATE_ATTRS], dTotal[BLOP_ADD_INDEX] );
	sphInfo ( "binlog: finished replaying %s; %d.%d MB in %d.%03d sec",
		sLog.cstr(),
		(int)(iFileSize/1048576), (int)((iFileSize*10/1048576)%10),
		(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );

	if ( bHaveCacheOp && dTotal[BLOP_TOTAL]==1 ) // only one operation, that is Add Cache - by the fact, empty binlog
		return 1;

	return 0;
}


static BinlogIndexInfo_t & ReplayIndexID ( BinlogReader_c & tReader, BinlogFileDesc_t & tLog, const char * sPlace )
{
	const int64_t iTxnPos = tReader.GetPos();
	const int iVal = (int)tReader.UnzipValue();

	if ( iVal<0 || iVal>=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: %s: unexpected index id (id=%d, max=%d, pos="INT64_FMT")",
			sPlace, iVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );

	return tLog.m_dIndexInfos[iVal];
}


bool RtBinlog_c::ReplayCommit ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "commit" );

	// load transaction data
	const int64_t iTID = (int64_t) tReader.UnzipValue();
	const int64_t tmStamp = (int64_t) tReader.UnzipValue();

	CSphScopedPtr<RtSegment_t> pSeg ( NULL );
	CSphVector<SphDocID_t> dKlist;

	int iRows = (int)tReader.UnzipValue();
	if ( iRows )
	{
		pSeg = new RtSegment_t();
		pSeg->m_iRows = pSeg->m_iAliveRows = iRows;
		m_iReplayedRows += iRows;

		LoadVector ( tReader, pSeg->m_dWords );
		pSeg->m_dWordCheckpoints.Resize ( (int) tReader.UnzipValue() ); // FIXME! sanity check
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int) tReader.UnzipValue();
			pSeg->m_dWordCheckpoints[i].m_iWordID = (SphWordID_t )tReader.UnzipValue();
		}
		LoadVector ( tReader, pSeg->m_dDocs );
		LoadVector ( tReader, pSeg->m_dHits );
		LoadVector ( tReader, pSeg->m_dRows );
		LoadVector ( tReader, pSeg->m_dStrings );
		LoadVector ( tReader, pSeg->m_dMvas );
		LoadVector ( tReader, pSeg->m_dKeywordCheckpoints );
	}
	LoadVector ( tReader, dKlist );

	// checksum
	if ( tReader.GetErrorFlag() || !tReader.CheckCrc ( "commit", tIndex.m_sName.cstr(), iTID, iTxnPos ) )
		return false;

	// check TID
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: commit: descending tid (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );

	// check timestamp
	if ( tmStamp<tIndex.m_tmMax )
	{
		if (!( uReplayFlags & SPH_REPLAY_ACCEPT_DESC_TIMESTAMP ))
			sphDie ( "binlog: commit: descending time (index=%s, lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

		sphWarning ( "binlog: commit: replaying txn despite descending time "
			"(index=%s, logtid="INT64_FMT", lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), iTID, tIndex.m_tmMax, tmStamp, iTxnPos );
		tIndex.m_tmMax = tmStamp;
	}

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pRT && iTID > tIndex.m_pRT->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pRT->m_iTID+1 )
			sphWarning ( "binlog: commit: unexpected tid (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos );

		// in case dict=keywords
		// + cook checkpoint
		// + build infixes
		if ( tIndex.m_pRT->IsWordDict() && pSeg.Ptr() )
		{
			FixupSegmentCheckpoints ( pSeg.Ptr() );
			tIndex.m_pRT->BuildSegmentInfixes ( pSeg.Ptr() );
		}

		// actually replay
		tIndex.m_pRT->CommitReplayable ( pSeg.LeakPtr(), dKlist );

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pRT->m_iTID = iTID;
	}

	// update info
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
	return true;
}

bool RtBinlog_c::ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const
{
	// load and check index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	uint64_t uVal = tReader.UnzipValue();
	if ( (int)uVal!=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: indexadd: unexpected index id (id="UINT64_FMT", expected=%d, pos="INT64_FMT")",
			uVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );

	// load data
	CSphString sName = tReader.GetString();

	// FIXME? use this for double checking?
	tReader.UnzipValue (); // TID
	tReader.UnzipValue (); // time

	if ( !tReader.CheckCrc ( "indexadd", sName.cstr(), 0, iTxnPos ) )
		return false;

	// check for index name dupes
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
		if ( tLog.m_dIndexInfos[i].m_sName==sName )
			sphDie ( "binlog: duplicate index name (name=%s, dupeid=%d, pos="INT64_FMT")",
				sName.cstr(), i, iTxnPos );

	// not a dupe, lets add
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add();
	tIndex.m_sName = sName;

	// lookup index in the list of currently served ones
	CSphIndex ** ppIndex = hIndexes ( sName.cstr() );
	CSphIndex * pIndex = ppIndex ? (*ppIndex) : NULL;
	if ( pIndex )
	{
		tIndex.m_pIndex = pIndex;
		if ( pIndex->IsRT() )
			tIndex.m_pRT = (RtIndex_t*)pIndex;
		tIndex.m_iPreReplayTID = pIndex->m_iTID;
		tIndex.m_iFlushedTID = pIndex->m_iTID;
	}

	// all ok
	// TID ranges will be now recomputed as we replay
	return true;
}

bool RtBinlog_c::ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "update" );

	// load transaction data
	CSphAttrUpdate tUpd;
	tUpd.m_bIgnoreNonexistent = true;

	int64_t iTID = (int64_t) tReader.UnzipValue();
	int64_t tmStamp = (int64_t) tReader.UnzipValue();

	tUpd.m_dAttrs.Resize ( (DWORD) tReader.UnzipValue() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		tUpd.m_dAttrs[i].m_sName = tReader.GetString();
		tUpd.m_dAttrs[i].m_eAttrType = (ESphAttr) tReader.UnzipValue(); // safe, we'll crc check later
	}
	if ( tReader.GetErrorFlag()
		|| !LoadVector ( tReader, tUpd.m_dPool )
		|| !LoadVector ( tReader, tUpd.m_dDocids )
		|| !LoadVector ( tReader, tUpd.m_dRowOffset )
		|| !tReader.CheckCrc ( "update", tIndex.m_sName.cstr(), iTID, iTxnPos ) )
	{
		return false;
	}

	// check TID, time order in log
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: update: descending tid (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
	if ( tmStamp<tIndex.m_tmMax )
		sphDie ( "binlog: update: descending time (index=%s, lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

	if ( tIndex.m_pIndex && iTID > tIndex.m_pIndex->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pIndex->m_iTID+1 )
			sphWarning ( "binlog: update: unexpected tid (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_pIndex->m_iTID, iTID, iTxnPos );

		CSphString sError;
		tIndex.m_pIndex->UpdateAttributes ( tUpd, -1, sError ); // FIXME! check for errors

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pIndex->m_iTID = iTID;
	}

	// update info
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
	return true;
}

bool RtBinlog_c::ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const
{
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// load data
	CSphVector<BinlogIndexInfo_t> dCache;
	dCache.Resize ( (int) tReader.UnzipValue() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, dCache )
	{
		dCache[i].m_sName = tReader.GetString();
		dCache[i].m_iMinTID = tReader.UnzipValue();
		dCache[i].m_iMaxTID = tReader.UnzipValue();
		dCache[i].m_iFlushedTID = tReader.UnzipValue();
		dCache[i].m_tmMin = tReader.UnzipValue();
		dCache[i].m_tmMax = tReader.UnzipValue();
	}
	if ( !tReader.CheckCrc ( "cache", "", 0, iTxnPos ) )
		return false;

	// if we arrived here by replay, let's verify everything
	// note that cached infos just passed checksumming, so the file is supposed to be clean!
	// in any case, broken log or not, we probably managed to replay something
	// so let's just report differences as warnings

	if ( dCache.GetLength()!=tLog.m_dIndexInfos.GetLength() )
	{
		sphWarning ( "binlog: cache mismatch: %d indexes cached, %d replayed",
			dCache.GetLength(), tLog.m_dIndexInfos.GetLength() );
		return true;
	}

	ARRAY_FOREACH ( i, dCache )
	{
		BinlogIndexInfo_t & tCache = dCache[i];
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

		if ( tCache.m_sName!=tIndex.m_sName )
		{
			sphWarning ( "binlog: cache mismatch: index %d name mismatch (%s cached, %s replayed)",
				i, tCache.m_sName.cstr(), tIndex.m_sName.cstr() );
			continue;
		}

		if ( tCache.m_iMinTID!=tIndex.m_iMinTID || tCache.m_iMaxTID!=tIndex.m_iMaxTID )
		{
			sphWarning ( "binlog: cache mismatch: index %s tid ranges mismatch "
				"(cached "INT64_FMT" to "INT64_FMT", replayed "INT64_FMT" to "INT64_FMT")",
				tCache.m_sName.cstr(),
				tCache.m_iMinTID, tCache.m_iMaxTID, tIndex.m_iMinTID, tIndex.m_iMaxTID );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

ISphRtIndex * sphGetCurrentIndexRT()
{
	RtAccum_t * pAcc = (RtAccum_t*) sphThreadGet ( g_tTlsAccumKey );
	if ( pAcc )
		return pAcc->m_pIndex;
	return NULL;
}

ISphRtIndex * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName,
	int64_t iRamSize, const char * sPath, bool bKeywordDict )
{
	MEMORY ( SPH_MEM_IDX_RT );
	return new RtIndex_t ( tSchema, sIndexName, iRamSize, sPath, bKeywordDict );
}


void sphRTInit ()
{
	MEMORY ( SPH_MEM_BINLOG );

	g_bRTChangesAllowed = false;
	Verify ( RtSegment_t::m_tSegmentSeq.Init() );
	Verify ( sphThreadKeyCreate ( &g_tTlsAccumKey ) );

	g_pRtBinlog = new RtBinlog_c();
	if ( !g_pRtBinlog )
		sphDie ( "binlog: failed to create binlog" );
	g_pBinlog = g_pRtBinlog;
}


void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	assert ( g_pBinlog );
	g_pRtBinlog->Configure ( hSearchd, bTestMode );
	g_iRtFlushPeriod = hSearchd.GetInt ( "rt_flush_period", (int)g_iRtFlushPeriod );

	// clip period to range ( 10 sec, million years )
	g_iRtFlushPeriod = Max ( g_iRtFlushPeriod, 10 );
	g_iRtFlushPeriod = Min ( g_iRtFlushPeriod, INT64_MAX );
}


void sphRTDone ()
{
	sphThreadKeyDelete ( g_tTlsAccumKey );
	Verify ( RtSegment_t::m_tSegmentSeq.Done() );
	// its valid for "searchd --stop" case
	SafeDelete ( g_pBinlog );
}


void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( SPH_MEM_BINLOG );
	g_pRtBinlog->Replay ( hIndexes, uReplayFlags, pfnProgressCallback );
	g_pRtBinlog->CreateTimerThread();
	g_bRTChangesAllowed = true;
}


bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema * pSchema, CSphString * pError )
{
	assert ( pSchema && pError );

	CSphColumnInfo tCol;

	// fields
	for ( CSphVariant * v=hIndex("rt_field"); v; v=v->m_pNext )
	{
		tCol.m_sName = v->cstr();
		tCol.m_sName.ToLower();
		pSchema->m_dFields.Add ( tCol );
	}
	if ( !pSchema->m_dFields.GetLength() )
	{
		pError->SetSprintf ( "no fields configured (use rt_field directive)" );
		return false;
	}

	if ( pSchema->m_dFields.GetLength()>SPH_MAX_FIELDS )
	{
		pError->SetSprintf ( "too many fields (fields=%d, max=%d)", pSchema->m_dFields.GetLength(), SPH_MAX_FIELDS );
		return false;
	}

	// attrs
	const int iNumTypes = 8;
	const char * sTypes[iNumTypes] = { "rt_attr_uint", "rt_attr_bigint", "rt_attr_float", "rt_attr_timestamp", "rt_attr_string", "rt_attr_multi", "rt_attr_multi_64", "rt_attr_json" };
	const ESphAttr iTypes[iNumTypes] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_FLOAT, SPH_ATTR_TIMESTAMP, SPH_ATTR_STRING, SPH_ATTR_UINT32SET, SPH_ATTR_INT64SET, SPH_ATTR_JSON };

	for ( int iType=0; iType<iNumTypes; iType++ )
	{
		for ( CSphVariant * v = hIndex ( sTypes[iType] ); v; v = v->m_pNext )
		{
			tCol.m_sName = v->cstr();
			tCol.m_sName.ToLower();
			tCol.m_eAttrType = iTypes[iType];
			pSchema->AddAttr ( tCol, false );
		}
	}

	return true;
}

//
// $Id$
//
