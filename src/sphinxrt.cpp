//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
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
#include "sphinxjson.h"
#include "sphinxplugin.h"
#include "sphinxrlp.h"
#include "sphinxqcache.h"

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

#define RTDICT_CHECKPOINT_V3			1024
#define RTDICT_CHECKPOINT_V5			48
#define SPH_RT_DOUBLE_BUFFER_PERCENT	10

#if USE_64BIT
#define WORDID_MAX				U64C(0xffffffffffffffff)
#else
#define	WORDID_MAX				0xffffffffUL
#endif

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

// Variable Length Byte (VLB) encoding
// store int variable in as much bytes as actually needed to represent it
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

template < typename T >
static inline void ZipT ( BYTE * & pOut, T uValue )
{
	do
	{
		BYTE bOut = (BYTE)( uValue & 0x7f );
		uValue >>= 7;
		if ( uValue )
			bOut |= 0x80;
		*pOut++ = bOut;
	} while ( uValue );
}

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );


// Variable Length Byte (VLB) decoding
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
		return 	( a.m_uWordID<b.m_uWordID ) ||
			( a.m_uWordID==b.m_uWordID && a.m_uDocID<b.m_uDocID ) ||
			( a.m_uWordID==b.m_uWordID && a.m_uDocID==b.m_uDocID && a.m_uWordPos<b.m_uWordPos );
	}
};


struct CmpHitKeywords_fn
{
	const BYTE * m_pBase;
	explicit CmpHitKeywords_fn ( const BYTE * pBase ) : m_pBase ( pBase ) {}
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b )
	{
		const BYTE * pPackedA = m_pBase + a.m_uWordID;
		const BYTE * pPackedB = m_pBase + b.m_uWordID;
		int iCmp = sphDictCmpStrictly ( (const char *)pPackedA+1, *pPackedA, (const char *)pPackedB+1, *pPackedB );
		return 	( iCmp<0 ) ||
			( iCmp==0 && a.m_uDocID<b.m_uDocID ) ||
			( iCmp==0 && a.m_uDocID==b.m_uDocID && a.m_uWordPos<b.m_uWordPos );
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
		SphWordID_t					m_uWordID;
		const char *				m_sWord;
	};
	int							m_iOffset;
};


struct Slice_t
{
	DWORD				m_uOff;
	DWORD				m_uLen;
};


// More than just sorted vector.
// OrderedHash is for fast (without sorting potentially big vector) inserts.
class CSphKilllist : public ISphNoncopyable
{
private:
	static const int				MAX_SMALL_SIZE = 512;
	CSphVector<SphDocID_t>			m_dLargeKlist;
	CSphOrderedHash < bool, SphDocID_t, IdentityHash_fn, MAX_SMALL_SIZE >	m_hSmallKlist;
	CSphRwlock						m_tLock;

public:

	CSphKilllist()
	{
		m_tLock.Init();
	}

	virtual ~CSphKilllist()
	{
		m_tLock.Done();
	}

	void Flush ( CSphVector<SphDocID_t> & dKlist )
	{
		m_tLock.ReadLock();

		bool bGotHash = ( m_hSmallKlist.GetLength()>0 );
		if ( !bGotHash )
			NakedCopy ( dKlist );

		m_tLock.Unlock();

		if ( !bGotHash )
			return;

		m_tLock.WriteLock();
		NakedFlush ( NULL, 0 );
		NakedCopy ( dKlist );
		m_tLock.Unlock();
	}

	inline void Add ( SphDocID_t * pDocs, int iCount )
	{
		if ( !iCount )
			return;

		m_tLock.WriteLock();
		if ( m_hSmallKlist.GetLength()+iCount>=MAX_SMALL_SIZE )
		{
			NakedFlush ( pDocs, iCount );
		} else
		{
			while ( iCount-- )
				m_hSmallKlist.Add ( true, *pDocs++ );
		}
		m_tLock.Unlock();
	}

	bool Exists ( SphDocID_t uDoc )
	{
		m_tLock.ReadLock();
		bool bGot = ( m_hSmallKlist.Exists ( uDoc ) || m_dLargeKlist.BinarySearch ( uDoc )!=NULL );
		m_tLock.Unlock();
		return bGot;
	}

	void Reset ( SphDocID_t * pDocs, int iCount )
	{
		m_tLock.WriteLock();
		m_dLargeKlist.Reset();
		m_hSmallKlist.Reset();

		NakedFlush ( pDocs, iCount );

		m_tLock.Unlock();
	}

	void LoadFromFile ( const char * sFilename );
	void SaveToFile ( const char * sFilename );

private:

	void NakedCopy ( CSphVector<SphDocID_t> & dKlist )
	{
		assert ( m_hSmallKlist.GetLength()==0 );
		if ( !m_dLargeKlist.GetLength() )
			return;

		int iOff = dKlist.GetLength();
		dKlist.Resize ( m_dLargeKlist.GetLength()+iOff );
		memcpy ( dKlist.Begin()+iOff, m_dLargeKlist.Begin(), sizeof(m_dLargeKlist[0]) * m_dLargeKlist.GetLength() );
	}

	void NakedFlush ( SphDocID_t * pDocs, int iCount )
	{
		if ( m_hSmallKlist.GetLength()==0 && iCount==0 )
			return;

		m_dLargeKlist.Reserve ( m_dLargeKlist.GetLength()+m_hSmallKlist.GetLength()+iCount );
		m_hSmallKlist.IterateStart();
		while ( m_hSmallKlist.IterateNext() )
			m_dLargeKlist.Add ( m_hSmallKlist.IterateGetKey() );
		if ( pDocs && iCount )
		{
			int iOff = m_dLargeKlist.GetLength();
			m_dLargeKlist.Resize ( iOff+iCount );
			memcpy ( m_dLargeKlist.Begin()+iOff, pDocs, sizeof(m_dLargeKlist[0]) * iCount );
		}
		m_dLargeKlist.Uniq();
		m_hSmallKlist.Reset();
	}
};

// is already id32<>id64 safe
void CSphKilllist::LoadFromFile ( const char * sFilename )
{
	Reset ( NULL, 0 );

	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	if ( !sphIsReadable ( sName.cstr(), &sError ) )
		return;

	CSphAutoreader tKlistReader;
	if ( !tKlistReader.Open ( sName, sError ) )
		return;

	// FIXME!!! got rid of locks here
	m_tLock.WriteLock();
	m_dLargeKlist.Resize ( tKlistReader.GetDword() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		uLastDocID += ( SphDocID_t ) tKlistReader.UnzipOffset();
		m_dLargeKlist[i] = uLastDocID;
	};
	m_tLock.Unlock();
}

void CSphKilllist::SaveToFile ( const char * sFilename )
{
	// FIXME!!! got rid of locks here
	m_tLock.WriteLock();
	NakedFlush ( NULL, 0 );

	CSphWriter tKlistWriter;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	tKlistWriter.OpenFile ( sName.cstr(), sError );

	tKlistWriter.PutDword ( m_dLargeKlist.GetLength() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		tKlistWriter.ZipOffset ( m_dLargeKlist[i] - uLastDocID );
		uLastDocID = ( SphDocID_t ) m_dLargeKlist[i];
	};
	m_tLock.Unlock();
	tKlistWriter.CloseFile();
}


struct KlistRefcounted_t
{
	KlistRefcounted_t ()
		: m_dKilled ( 0 )
		, m_tRefCount ( 1 )
	{}
	CSphFixedVector<SphDocID_t>		m_dKilled;
	CSphAtomic						m_tRefCount;
};


// this is what actually stores index data
// RAM chunk consists of such segments
struct RtSegment_t : ISphNoncopyable
{
protected:
	static const int			KLIST_ACCUM_THRESH	= 32;

public:
	static CSphMutex			m_tSegmentSeq;
	static int					m_iSegments;	///< age tag sequence generator (guarded by m_tSegmentSeq)
	int							m_iTag;			///< segment age tag

	CSphTightVector<BYTE>			m_dWords;
	CSphVector<RtWordCheckpoint_t>	m_dWordCheckpoints;
	CSphTightVector<uint64_t>		m_dInfixFilterCP;
	CSphTightVector<BYTE>		m_dDocs;
	CSphTightVector<BYTE>		m_dHits;

	int							m_iRows;		///< number of actually allocated rows
	int							m_iAliveRows;	///< number of alive (non-killed) rows
	CSphTightVector<CSphRowitem>		m_dRows;		///< row data storage
	KlistRefcounted_t *			m_pKlist;
	bool						m_bTlsKlist;	///< whether to apply TLS K-list during merge (must only be used by writer during Commit())
	CSphTightVector<BYTE>		m_dStrings;		///< strings storage
	CSphTightVector<DWORD>		m_dMvas;		///< MVAs storage
	CSphVector<BYTE>			m_dKeywordCheckpoints;
	mutable CSphAtomic			m_tRefCount;

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
		m_pKlist = new KlistRefcounted_t();
	}

	~RtSegment_t ()
	{
		SafeDelete ( m_pKlist );
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

	int GetStride () const
	{
		return ( m_dRows.GetLength() / m_iRows );
	}

	const CSphFixedVector<SphDocID_t> & GetKlist() const { return m_pKlist->m_dKilled; }

	const CSphRowitem *		FindRow ( SphDocID_t uDocid ) const;
	const CSphRowitem *		FindAliveRow ( SphDocID_t uDocid ) const;
};

int RtSegment_t::m_iSegments = 0;
CSphMutex RtSegment_t::m_tSegmentSeq;


const CSphRowitem * RtSegment_t::FindRow ( SphDocID_t uDocid ) const
{
	// binary search through the rows
	int iStride = GetStride();
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
	if ( m_pKlist->m_dKilled.BinarySearch ( uDocid ) )
		return NULL;
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
		BYTE * pEnd = pDocs->AddN ( 5*sizeof(DWORD) );
		const BYTE * pBegin = pDocs->Begin();

		ZipDocid ( pEnd, tDoc.m_uDocID - m_uLastDocID );
		m_uLastDocID = tDoc.m_uDocID;
		ZipDword ( pEnd, tDoc.m_uDocFields );
		ZipDword ( pEnd, tDoc.m_uHits );
		if ( tDoc.m_uHits==1 )
		{
			ZipDword ( pEnd, tDoc.m_uHit & 0xffffffUL );
			ZipDword ( pEnd, tDoc.m_uHit>>24 );
		} else
			ZipDword ( pEnd, tDoc.m_uHit );

		pDocs->Resize ( pEnd-pBegin );
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
	RtDocReader_T ( const RtSegment_t * pSeg, const RTWORD & tWord )
	{
		m_pDocs = ( pSeg->m_dDocs.Begin() ? pSeg->m_dDocs.Begin() + tWord.m_uDoc : NULL );
		m_iLeft = tWord.m_uDocs;
		m_tDoc.m_uDocID = 0;
	}

	RtDocReader_T ()
	{
		m_pDocs = NULL;
		m_iLeft = 0;
		m_tDoc.m_uDocID = 0;
	}

	const RTDOC * UnzipDoc ()
	{
		if ( !m_iLeft || !m_pDocs )
			return NULL;

		const BYTE * pIn = m_pDocs;
		SphDocID_t uDeltaID;
		pIn = UnzipDocid ( &uDeltaID, pIn );
		m_tDoc.m_uDocID += (DOCID) uDeltaID;
		DWORD uField;
		pIn = UnzipDword ( &uField, pIn );
		m_tDoc.m_uDocFields = uField;
		pIn = UnzipDword ( &m_tDoc.m_uHits, pIn );
		if ( m_tDoc.m_uHits==1 )
		{
			DWORD a, b;
			pIn = UnzipDword ( &a, pIn );
			pIn = UnzipDword ( &b, pIn );
			m_tDoc.m_uHit = a + ( b<<24 );
		} else
			pIn = UnzipDword ( &m_tDoc.m_uHit, pIn );
		m_pDocs = pIn;

		m_iLeft--;
		return &m_tDoc;
	}
};

typedef RtDocReader_T<> RtDocReader_t;

template < typename VECTOR >
static int sphPutBytes ( VECTOR * pOut, const void * pData, int iLen )
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
	CSphVector<BYTE> *					m_pKeywordCheckpoints;

	CSphKeywordDeltaWriter				m_tLastKeyword;
	SphWordID_t							m_uLastWordID;
	DWORD								m_uLastDoc;
	int									m_iWords;

	bool								m_bKeywordDict;
	int									m_iWordsCheckpoint;

	RtWordWriter_t ( RtSegment_t * pSeg, bool bKeywordDict, int iWordsCheckpoint )
		: m_pWords ( &pSeg->m_dWords )
		, m_pCheckpoints ( &pSeg->m_dWordCheckpoints )
		, m_pKeywordCheckpoints ( &pSeg->m_dKeywordCheckpoints )
		, m_uLastWordID ( 0 )
		, m_uLastDoc ( 0 )
		, m_iWords ( 0 )
		, m_bKeywordDict ( bKeywordDict )
		, m_iWordsCheckpoint ( iWordsCheckpoint )
	{
		assert ( !m_pWords->GetLength() );
		assert ( !m_pCheckpoints->GetLength() );
		assert ( !m_pKeywordCheckpoints->GetLength() );
	}

	void ZipWord ( const RtWord_t & tWord )
	{
		CSphTightVector<BYTE> * pWords = m_pWords;
		if ( ++m_iWords==m_iWordsCheckpoint )
		{
			RtWordCheckpoint_t & tCheckpoint = m_pCheckpoints->Add();
			if ( !m_bKeywordDict )
			{
				tCheckpoint.m_uWordID = tWord.m_uWordID;
			} else
			{
				int iLen = tWord.m_sWord[0];
				assert ( iLen && iLen-1<SPH_MAX_KEYWORD_LEN );
				tCheckpoint.m_uWordID = sphPutBytes ( m_pKeywordCheckpoints, tWord.m_sWord+1, iLen+1 );
				m_pKeywordCheckpoints->Last() = '\0'; // checkpoint is NULL terminating string

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

		BYTE * pEnd = pWords->AddN ( 3*sizeof(DWORD) );
		const BYTE * pBegin = pWords->Begin();

		ZipDword ( pEnd, tWord.m_uDocs );
		ZipDword ( pEnd, tWord.m_uHits );
		ZipDword ( pEnd, tWord.m_uDoc - m_uLastDoc );

		pWords->Resize ( pEnd-pBegin );

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
		m_pCur = pSeg->m_dHits.Begin() + pDoc->m_uHit;
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

struct JSONAttr_t
{
	BYTE *	m_pData;
	int		m_iLen;
};

/// indexing accumulator
class RtAccum_t : public ISphRtAccum
{
public:
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
	CSphDict *					m_pDictCloned;
	ISphRtDictWraper *			m_pDictRt;
	bool						m_bReplace;		///< insert or replace mode (affects CleanupDuplicates() behavior)

public:
					explicit RtAccum_t ( bool bKeywordDict );
					~RtAccum_t();

	void			SetupDict ( const RtIndex_t * pIndex, CSphDict * pDict, bool bKeywordDict );
	void			ResetDict ();
	void			Sort ();

	void			AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas, const CSphVector<JSONAttr_t> & dJson );
	RtSegment_t *	CreateSegment ( int iRowSize, int iWordsCheckpoint );
	void			CleanupDuplicates ( int iRowSize );
	void			GrabLastWarning ( CSphString & sWarning );
	void			SetIndex ( ISphRtIndex * pIndex ) { m_pIndex = pIndex; }
};

/// TLS indexing accumulator (we disallow two uncommitted adds within one thread; and so need at most one)
static SphThreadKey_t g_tTlsAccumKey;

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
	BLOP_RECONFIGURE	= 5,

	BLOP_TOTAL
};

// forward declaration
class BufferReader_t;
class RtBinlog_c;


class BinlogWriter_c : public CSphWriter
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


private:
	int64_t			m_iLastWritePos;
	int64_t			m_iLastFsyncPos;
	int				m_iLastCrcPos;

	DWORD			m_uCRC;
	void			HashCollected ();
};


class BinlogReader_c : public CSphAutoreader
{
public:
					BinlogReader_c ();

	void			ResetCrc ();
	bool			CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos );

private:
	DWORD			m_uCRC;
	int				m_iLastCrcPos;
	virtual void	UpdateCache ();
	void			HashCollected ();
};


class RtBinlog_c : public ISphBinlog
{
public:
	RtBinlog_c ();
	~RtBinlog_c ();

	void	BinlogCommit ( int64_t * pTID, const char * sIndexName, const RtSegment_t * pSeg, const CSphVector<SphDocID_t> & dKlist, bool bKeywordDict );
	void	BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd );
	void	BinlogReconfigure ( int64_t * pTID, const char * sIndexName, const CSphReconfigureSetup & tSetup );
	void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown );

	void	Configure ( const CSphConfigSection & hSearchd, bool bTestMode );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback );

	void	CreateTimerThread ();
	bool	IsActive ()			{ return !m_bDisabled; }
	void	CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode );

private:
	static const DWORD		BINLOG_VERSION = 6;

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
	bool					ReplayReconfigure ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const;
};


struct SphChunkGuard_t
{
	CSphFixedVector<const RtSegment_t *>	m_dRamChunks;
	CSphFixedVector<const CSphIndex *>		m_dDiskChunks;
	CSphFixedVector<const KlistRefcounted_t *>		m_dKill;
	CSphRwlock *							m_pReading;
	SphChunkGuard_t ()
		: m_dRamChunks ( 0 )
		, m_dDiskChunks ( 0 )
		, m_dKill ( 0 )
		, m_pReading ( NULL )
	{
	}
	~SphChunkGuard_t();
};


struct ChunkStats_t
{
	CSphSourceStats				m_Stats;
	CSphFixedVector<int64_t>	m_dFieldLens;

	explicit ChunkStats_t ( const CSphSourceStats & s, const CSphFixedVector<int64_t> & dLens )
		: m_dFieldLens ( dLens.GetLength() )
	{
		m_Stats = s;
		ARRAY_FOREACH ( i, dLens )
			m_dFieldLens[i] = dLens[i];
	}
};


/// RAM based index
struct RtQword_t;
struct RtIndex_t : public ISphRtIndex, public ISphNoncopyable, public ISphWordlist
{
private:
	static const DWORD			META_HEADER_MAGIC	= 0x54525053;	///< my magic 'SPRT' header
	static const DWORD			META_VERSION		= 11;			///< current version

private:
	int							m_iStride;
	CSphVector<RtSegment_t*>	m_dRamChunks;
	CSphVector<const RtSegment_t*>	m_dRetired;

	CSphMutex					m_tWriting;
	mutable CSphRwlock			m_tChunkLock;
	mutable CSphRwlock			m_tReading;

	/// double buffer stuff (allows to work with RAM chunk while future disk is being saved)
	/// m_dSegments consists of two parts
	/// segments with indexes < m_iDoubleBuffer are being saved now as a disk chunk
	/// segments with indexes >= m_iDoubleBuffer are RAM chunk
	CSphMutex					m_tFlushLock;
	CSphMutex					m_tOptimizingLock;
	int							m_iDoubleBuffer;
	CSphVector<SphDocID_t>		m_dNewSegmentKlist;					///< raw docid container
	CSphVector<SphDocID_t>		m_dDiskChunkKlist;					///< ordered SphDocID_t kill list

	int64_t						m_iSoftRamLimit;
	int64_t						m_iDoubleBufferLimit;
	CSphString					m_sPath;
	bool						m_bPathStripped;
	CSphVector<CSphIndex*>		m_dDiskChunks;
	int							m_iLockFD;
	mutable CSphKilllist		m_tKlist;							///< kill list for disk chunks and saved chunks
	int							m_iDiskBase;
	volatile bool				m_bOptimizing;
	volatile bool				m_bOptimizeStop;

	int64_t						m_iSavedTID;
	int64_t						m_tmSaved;
	mutable DWORD				m_uDiskAttrStatus;

	bool						m_bKeywordDict;
	int							m_iWordsCheckpoint;
	int							m_iMaxCodepointLength;
	ISphTokenizer *				m_pTokenizerIndexing;
	bool						m_bLoadRamPassedOk;

	bool						m_bMlock;
	bool						m_bOndiskAllAttr;
	bool						m_bOndiskPoolAttr;

	CSphFixedVector<int64_t>	m_dFieldLens;						///< total field lengths over entire index
	CSphFixedVector<int64_t>	m_dFieldLensRam;					///< field lengths summed over current RAM chunk
	CSphFixedVector<int64_t>	m_dFieldLensDisk;					///< field lengths summed over all disk chunks

public:
	explicit					RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );
	virtual						~RtIndex_t ();

	virtual bool				AddDocument ( ISphTokenizer * pTokenizer, int iFields, const char ** ppFields, const CSphMatch & tDoc, bool bReplace, const CSphString & sTokenFilterOptions, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt );
	virtual bool				AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt );
	virtual bool				DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError, ISphRtAccum * pAccExt );
	virtual void				Commit ( int * pDeleted, ISphRtAccum * pAccExt );
	virtual void				RollBack ( ISphRtAccum * pAccExt );
	void						CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist, int * pTotalKilled ); // FIXME? protect?
	virtual void				CheckRamFlush ();
	virtual void				ForceRamFlush ( bool bPeriodic=false );
	virtual void				ForceDiskChunk ();
	virtual bool				AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError );
	virtual bool				Truncate ( CSphString & sError );
	virtual void				Optimize ( volatile bool * pForceTerminate, ThrottleState_t * pThrottle );
	CSphIndex *					GetDiskChunk ( int iChunk ) { return m_dDiskChunks.GetLength()>iChunk ? m_dDiskChunks[iChunk] : NULL; }
	virtual ISphTokenizer *		CloneIndexingTokenizer() const { return m_pTokenizerIndexing->Clone ( SPH_CLONE_INDEX ); }

private:
	/// acquire thread-local indexing accumulator
	/// returns NULL if another index already uses it in an open txn
	RtAccum_t *					AcquireAccum ( CSphString * sError, ISphRtAccum * pAccExt, bool bSetTLS );
	virtual ISphRtAccum *		CreateAccum ( CSphString & sError );

	RtSegment_t *				MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist, bool bHasMorphology );
	const RtWord_t *			CopyWord ( RtSegment_t * pDst, RtWordWriter_t & tOutWord, const RtSegment_t * pSrc, const RtWord_t * pWord, RtWordReader_t & tInWord, const CSphVector<SphDocID_t> * pAccKlist );
	void						MergeWord ( RtSegment_t * pDst, const RtSegment_t * pSrc1, const RtWord_t * pWord1, const RtSegment_t * pSrc2, const RtWord_t * pWord2, RtWordWriter_t & tOut, const CSphVector<SphDocID_t> * pAccKlist );
	void						CopyDoc ( RtSegment_t * pSeg, RtDocWriter_t & tOutDoc, RtWord_t * pWord, const RtSegment_t * pSrc, const RtDoc_t * pDoc );

	void						SaveMeta ( int iDiskChunks, int64_t iTID );
	void						SaveDiskHeader ( const char * sFilename, SphDocID_t iMinDocID, int iCheckpoints, SphOffset_t iCheckpointsPosition, DWORD iInfixBlocksOffset, int iInfixCheckpointWordsSize, DWORD uKillListSize, uint64_t uMinMaxSize, const ChunkStats_t & tStats ) const;
	void						SaveDiskDataImpl ( const char * sFilename, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats ) const;
	void						SaveDiskChunk ( int64_t iTID, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats );
	CSphIndex *					LoadDiskChunk ( const char * sChunk, CSphString & sError ) const;
	bool						LoadRamChunk ( DWORD uVersion, bool bRebuildInfixes );
	bool						SaveRamChunk ();

	virtual void				GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const;
	virtual void				GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const;

public:
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4100)
#endif
	virtual SphDocID_t *		GetKillList () const				{ return NULL; }
	virtual int					GetKillListSize () const			{ return 0; }
	virtual bool				HasDocid ( SphDocID_t ) const		{ assert ( 0 ); return false; }

	virtual int					Build ( const CSphVector<CSphSource*> & , int , int ) { return 0; }
	virtual bool				Merge ( CSphIndex * , const CSphVector<CSphFilterSettings> & , bool ) { return false; }

	virtual bool				Prealloc ( bool bStripPath );
	virtual void				Dealloc () {}
	virtual void				Preread ();
	virtual void				SetMemorySettings ( bool bMlock, bool bOndiskAttrs, bool bOndiskPool );
	virtual void				SetBase ( const char * ) {}
	virtual bool				Rename ( const char * ) { return true; }
	virtual bool				Lock () { return true; }
	virtual void				Unlock () {}
	virtual void				PostSetup();
	virtual bool				IsRT() const { return true; }

	virtual void				Setup ( const CSphIndexSettings & tSettings );

	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError, CSphString & sWarning );
	virtual bool				SaveAttributes ( CSphString & sError ) const;
	virtual DWORD				GetAttributeStatus () const { return m_uDiskAttrStatus; }
	virtual bool				AddRemoveAttribute ( bool bAdd, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError );

	virtual void				DebugDumpHeader ( FILE * , const char * , bool ) {}
	virtual void				DebugDumpDocids ( FILE * ) {}
	virtual void				DebugDumpHitlist ( FILE * , const char * , bool ) {}
	virtual void				DebugDumpDict ( FILE * ) {}
	virtual int					DebugCheck ( FILE * fp );
#if USE_WINDOWS
#pragma warning(pop)
#endif

public:
	virtual bool						EarlyReject ( CSphQueryContext * pCtx, CSphMatch & ) const;
	virtual const CSphSourceStats &		GetStats () const { return m_tStats; }
	virtual int64_t *					GetFieldLens() const { return m_tSettings.m_bIndexFieldLens ? m_dFieldLens.Begin() : NULL; }
	virtual void				GetStatus ( CSphIndexStatus* ) const;

	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const;
	bool						DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, bool bFillOnly, CSphString * pError, const SphChunkGuard_t & tGuard ) const;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString * pError ) const;
	virtual bool				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const;
	void						AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, CSphDict * pDict, bool bGetStats, int iQpos, RtQword_t * pQueryWord, CSphVector <CSphKeywordInfo> & dKeywords, const SphChunkGuard_t & tGuard ) const;

	void						CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const;
	const CSphRowitem *			FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID ) const;

	bool						RtQwordSetup ( RtQword_t * pQword, int iSeg, const SphChunkGuard_t & tGuard ) const;
	static bool					RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint, const CSphFixedVector<SphDocID_t> & dKill, const CSphIndexSettings & tSettings );

	virtual bool				IsStarDict() const;
	CSphDict *					SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer, bool bAddSpecial ) const;
	CSphDict *					SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const;

	virtual const CSphSchema &	GetMatchSchema () const { return m_tSchema; }
	virtual const CSphSchema &	GetInternalSchema () const { return m_tSchema; }
	int64_t						GetUsedRam () const;

	bool						IsWordDict () const { return m_bKeywordDict; }
	void						BuildSegmentInfixes ( RtSegment_t * pSeg, bool bHasMorphology ) const;

	// TODO: implement me
	virtual	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) {}

	virtual bool				IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError ) const;
	virtual void				Reconfigure ( CSphReconfigureSetup & tSetup );

protected:
	CSphSourceStats				m_tStats;

private:

	void						GetReaderChunks ( SphChunkGuard_t & tGuard ) const;
	void						FreeRetired();
};


RtIndex_t::RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict )

	: ISphRtIndex ( sIndexName, sPath )
	, m_dDiskChunkKlist ( 0 )
	, m_iSoftRamLimit ( iRamSize )
	, m_sPath ( sPath )
	, m_bPathStripped ( false )
	, m_iLockFD ( -1 )
	, m_iDiskBase ( 0 )
	, m_bOptimizing ( false )
	, m_bOptimizeStop ( false )
	, m_iSavedTID ( m_iTID )
	, m_tmSaved ( sphMicroTimer() )
	, m_uDiskAttrStatus ( 0 )
	, m_bKeywordDict ( bKeywordDict )
	, m_iWordsCheckpoint ( RTDICT_CHECKPOINT_V5 )
	, m_pTokenizerIndexing ( NULL )
	, m_dFieldLens ( SPH_MAX_FIELDS )
	, m_dFieldLensRam ( SPH_MAX_FIELDS )
	, m_dFieldLensDisk ( SPH_MAX_FIELDS )
{
	MEMORY ( MEM_INDEX_RT );

	m_tSchema = tSchema;
	m_iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	m_iDoubleBufferLimit = ( m_iSoftRamLimit * SPH_RT_DOUBLE_BUFFER_PERCENT ) / 100;
	m_iDoubleBuffer = 0;
	m_bMlock = false;
	m_bOndiskAllAttr = false;
	m_bOndiskPoolAttr = false;
	m_bLoadRamPassedOk = true;

#ifndef NDEBUG
	// check that index cols are static
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		assert ( !m_tSchema.GetAttr(i).m_tLocator.m_bDynamic );
#endif

	Verify ( m_tChunkLock.Init() );
	Verify ( m_tReading.Init() );

	ARRAY_FOREACH ( i, m_dFieldLens )
	{
		m_dFieldLens[i] = 0;
		m_dFieldLensRam[i] = 0;
		m_dFieldLensDisk[i] = 0;
	}
}


RtIndex_t::~RtIndex_t ()
{
	int64_t tmSave = sphMicroTimer();
	bool bValid = m_pTokenizer && m_pDict && m_bLoadRamPassedOk;

	if ( bValid )
	{
		SaveRamChunk ();
		SaveMeta ( m_dDiskChunks.GetLength(), m_iTID );
	}

	Verify ( m_tReading.Done() );
	Verify ( m_tChunkLock.Done() );

	ARRAY_FOREACH ( i, m_dRamChunks )
		SafeDelete ( m_dRamChunks[i] );

	m_dRetired.Uniq();
	ARRAY_FOREACH ( i, m_dRetired )
		SafeDelete ( m_dRetired[i] );

	ARRAY_FOREACH ( i, m_dDiskChunks )
		SafeDelete ( m_dDiskChunks[i] );

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


static int64_t g_iRtFlushPeriod = 10*60*60; // default period is 10 hours

void RtIndex_t::CheckRamFlush ()
{
	if ( ( sphMicroTimer()-m_tmSaved )/1000000<g_iRtFlushPeriod )
		return;
	if ( g_pRtBinlog->IsActive() && m_iTID<=m_iSavedTID )
		return;

	ForceRamFlush ( true );
}


void RtIndex_t::ForceRamFlush ( bool bPeriodic )
{
	int64_t tmSave = sphMicroTimer();

	// need this lock as could get here at same time either ways:
	// via RtFlushThreadFunc->RtIndex_t::CheckRamFlush
	// and via HandleMysqlFlushRtindex
	CSphScopedLock<CSphMutex> tLock ( m_tFlushLock );

	if ( g_pRtBinlog->IsActive() && m_iTID<=m_iSavedTID )
		return;

	Verify ( m_tWriting.Lock() );

	int64_t iUsedRam = GetUsedRam();
	if ( !SaveRamChunk () )
	{
		sphWarning ( "rt: index %s: ramchunk save FAILED! (error=%s)", m_sIndexName.cstr(), m_sLastError.cstr() );
		Verify ( m_tWriting.Unlock() );
		return;
	}
	SaveMeta ( m_dDiskChunks.GetLength(), m_iTID );
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	int64_t iWasTID = m_iSavedTID;
	int64_t tmDelta = sphMicroTimer() - m_tmSaved;
	m_iSavedTID = m_iTID;
	m_tmSaved = sphMicroTimer();

	Verify ( m_tWriting.Unlock() );

	tmSave = sphMicroTimer() - tmSave;
	sphInfo ( "rt: index %s: ramchunk saved ok (mode=%s, last TID="INT64_FMT", current TID="INT64_FMT", "
		"ram=%d.%03d Mb, time delta=%d sec, took=%d.%03d sec)"
		, m_sIndexName.cstr(), bPeriodic ? "periodic" : "forced"
		, iWasTID, m_iTID, (int)(iUsedRam/1024/1024), (int)((iUsedRam/1024)%1000)
		, (int) (tmDelta/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}


int64_t RtIndex_t::GetUsedRam () const
{
	int64_t iTotal = 0;
	ARRAY_FOREACH ( i, m_dRamChunks )
		iTotal += m_dRamChunks[i]->GetUsedRam();

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
	virtual const int *	GetFieldLengths () const { return m_dFieldLengths.Begin(); }

protected:
	CSphVector<BYTE *>			m_dFields;
	CSphVector<int>				m_dFieldLengths;
};


CSphSource_StringVector::CSphSource_StringVector ( int iFields, const char ** ppFields, const CSphSchema & tSchema )
	: CSphSource_Document ( "$stringvector" )
{
	m_tSchema = tSchema;

	m_dFields.Resize ( 1+iFields );
	m_dFieldLengths.Resize ( iFields );
	for ( int i=0; i<iFields; i++ )
	{
		m_dFields[i] = (BYTE*) ppFields[i];
		m_dFieldLengths[i] = strlen ( ppFields[i] );
		assert ( m_dFields[i] );
	}
	m_dFields [ iFields ] = NULL;

	m_iMaxHits = 0; // force all hits build
}

bool CSphSource_StringVector::Connect ( CSphString & )
{
	// no AddAutoAttrs() here; they should already be in the schema
	m_tHits.m_dData.Reserve ( 1024 );
	return true;
}

void CSphSource_StringVector::Disconnect ()
{
	m_tHits.m_dData.Reset();
}

bool RtIndex_t::AddDocument ( ISphTokenizer * pTokenizer, int iFields, const char ** ppFields, const CSphMatch & tDoc,
	bool bReplace, const CSphString & sTokenFilterOptions,
	const char ** ppStr, const CSphVector<DWORD> & dMvas,
	CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	CSphScopedPtr<ISphTokenizer> tTokenizer ( pTokenizer );

	if ( !tDoc.m_uDocID )
		return true;

	MEMORY ( MEM_INDEX_RT );

	if ( !bReplace )
	{
		m_tChunkLock.ReadLock ();
		bool bGotID = ARRAY_ANY ( bGotID, m_dRamChunks, ( m_dRamChunks[_any]->FindAliveRow ( tDoc.m_uDocID )!=NULL ) );
		m_tChunkLock.Unlock ();

		if ( bGotID )
		{
			sError.SetSprintf ( "duplicate id '"UINT64_FMT"'", (uint64_t)tDoc.m_uDocID );
			return false; // already exists and not deleted; INSERT fails
		}
	}

	RtAccum_t * pAcc = AcquireAccum ( &sError, pAccExt, true );
	if ( !pAcc )
		return false;

	// OPTIMIZE? do not create filter on each(!) INSERT
	if ( !m_tSettings.m_sIndexTokenFilter.IsEmpty() )
	{
		tTokenizer.ReplacePtr ( ISphTokenizer::CreatePluginFilter ( tTokenizer.Ptr(), m_tSettings.m_sIndexTokenFilter, sError ) );
		if ( !tTokenizer.Ptr() )
			return false;
		if ( !tTokenizer->SetFilterSchema ( m_tSchema, sError ) )
			return false;
		if ( !sTokenFilterOptions.IsEmpty() )
			if ( !tTokenizer->SetFilterOptions ( sTokenFilterOptions.cstr(), sError ) )
				return false;
	}

	// OPTIMIZE? do not create filter on each(!) INSERT
	if ( m_tSettings.m_uAotFilterMask )
		tTokenizer.ReplacePtr ( sphAotCreateFilter ( tTokenizer.Ptr(), m_pDict, m_tSettings.m_bIndexExactWords, m_tSettings.m_uAotFilterMask ) );

	CSphSource_StringVector tSrc ( iFields, ppFields, m_tSchema );

	// SPZ setup
	if ( m_tSettings.m_bIndexSP && !tTokenizer->EnableSentenceIndexing ( sError ) )
		return false;

	if ( !m_tSettings.m_sZones.IsEmpty() && !tTokenizer->EnableZoneIndexing ( sError ) )
		return false;

	if ( m_tSettings.m_bHtmlStrip && !tSrc.SetStripHTML ( m_tSettings.m_sHtmlIndexAttrs.cstr(), m_tSettings.m_sHtmlRemoveElements.cstr(),
			m_tSettings.m_bIndexSP, m_tSettings.m_sZones.cstr(), sError ) )
		return false;

	// OPTIMIZE? do not clone filters on each INSERT
	CSphScopedPtr<ISphFieldFilter> pFieldFilter ( NULL );
	if ( m_pFieldFilter )
		pFieldFilter = m_pFieldFilter->Clone();

	tSrc.Setup ( m_tSettings );
	tSrc.SetTokenizer ( tTokenizer.Ptr() );
	tSrc.SetDict ( pAcc->m_pDict );
	tSrc.SetFieldFilter ( pFieldFilter.Ptr() );
	if ( !tSrc.Connect ( m_sLastError ) )
		return false;

	m_tSchema.CloneWholeMatch ( &tSrc.m_tDocInfo, tDoc );

	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );
	pAcc->GrabLastWarning ( sWarning );

	if ( !AddDocument ( pHits, tDoc, bReplace, ppStr, dMvas, sError, sWarning, pAcc ) )
		return false;

	m_tStats.m_iTotalBytes += tSrc.GetStats().m_iTotalBytes;

	return true;
}


void AccumCleanup ( void * pArg )
{
	RtAccum_t * pAcc = (RtAccum_t *) pArg;
	SafeDelete ( pAcc );
}


RtAccum_t * RtIndex_t::AcquireAccum ( CSphString * sError, ISphRtAccum * pAccExt, bool bSetTLS )
{
	RtAccum_t * pAcc = NULL;
	//assert ( pAccExt && !bSetTLS );

	// check that no other index is holding the acc
	if ( !pAccExt )
		pAcc = (RtAccum_t*) sphThreadGet ( g_tTlsAccumKey );
	else
		pAcc = (RtAccum_t *)pAccExt;

	if ( pAcc && pAcc->GetIndex()!=NULL && pAcc->GetIndex()!=this )
	{
		if ( sError )
			sError->SetSprintf ( "current txn is working with another index ('%s')", pAcc->GetIndex()->GetName() );
		return NULL;
	}

	if ( !pAcc )
	{
		pAcc = new RtAccum_t ( m_bKeywordDict );
		if ( bSetTLS )
		{
			sphThreadSet ( g_tTlsAccumKey, pAcc );
			sphThreadOnExit ( AccumCleanup, pAcc );
		}
	}

	assert ( pAcc->GetIndex()==NULL || pAcc->GetIndex()==this );
	pAcc->SetIndex ( this );
	pAcc->SetupDict ( this, m_pDict, m_bKeywordDict );
	return pAcc;
}

ISphRtAccum * RtIndex_t::CreateAccum ( CSphString & sError )
{
	return AcquireAccum ( &sError, NULL, false );
}


bool RtIndex_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas,
	CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = (RtAccum_t *)pAccExt;

	if ( pAcc )
	{
		CSphVector<JSONAttr_t> dJsonData;

		const CSphSchema & tSchema = GetInternalSchema();
		int iAttr = 0;

		for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tColumn = tSchema.GetAttr(i);
			if ( tColumn.m_eAttrType==SPH_ATTR_JSON )
			{
				const char * pStr = ppStr ? ppStr[iAttr] : NULL;
				int iLen = pStr ? strlen ( pStr ) : 0;

				if ( pStr && iLen )
				{
					// pStr originates as CSphString, so we DO have space for an extra '\0'
					char * pData = const_cast<char*>(pStr);
					pData[iLen+1] = '\0';

					CSphVector<BYTE> dBuf;
					if ( !sphJsonParse ( dBuf, pData, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase, sError ) )
					{
						sError.SetSprintf ( "column %s: JSON error: %s", tColumn.m_sName.cstr(), sError.cstr() );

						if ( g_bJsonStrict )
						{
							ARRAY_FOREACH ( j, dJsonData )
								SafeDeleteArray ( dJsonData[j].m_pData );

							return false;
						}

						if ( sWarning.IsEmpty() )
							sWarning = sError;
						else
							sWarning.SetSprintf ( "%s; %s", sWarning.cstr(), sError.cstr() );

						sError = "";
					}

					JSONAttr_t & tAttr = dJsonData.Add();
					tAttr.m_iLen = dBuf.GetLength();
					tAttr.m_pData = dBuf.LeakData();
				}
			}

			iAttr += ( tColumn.m_eAttrType==SPH_ATTR_STRING || tColumn.m_eAttrType==SPH_ATTR_JSON ) ? 1 : 0;
		}

		pAcc->AddDocument ( pHits, tDoc, bReplace, m_tSchema.GetRowSize(), ppStr, dMvas, dJsonData );
	}

	return ( pAcc!=NULL );
}


RtAccum_t::RtAccum_t ( bool bKeywordDict )
	: m_iAccumDocs ( 0 )
	, m_bKeywordDict ( bKeywordDict )
	, m_pDict ( NULL )
	, m_pRefDict ( NULL )
	, m_pDictCloned ( NULL )
	, m_pDictRt ( NULL )
	, m_bReplace ( false )
{
	m_pIndex = NULL;
	m_dStrings.Add ( 0 );
	m_dMvas.Add ( 0 );
}

RtAccum_t::~RtAccum_t()
{
	SafeDelete ( m_pDictCloned );
	SafeDelete ( m_pDictRt );
}

void RtAccum_t::SetupDict ( const RtIndex_t * pIndex, CSphDict * pDict, bool bKeywordDict )
{
	if ( pIndex!=m_pIndex || pDict!=m_pRefDict || bKeywordDict!=m_bKeywordDict )
	{
		SafeDelete ( m_pDictCloned );
		SafeDelete ( m_pDictRt );
		m_pDict = NULL;
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

void RtAccum_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas, const CSphVector<JSONAttr_t> & dJson )
{
	MEMORY ( MEM_RT_ACCUM );

	// FIXME? what happens on mixed insert/replace?
	m_bReplace = bReplace;

	// schedule existing copies for deletion
	m_dAccumKlist.Add ( tDoc.m_uDocID );

	// reserve some hit space on first use
	if ( pHits && pHits->Length() && !m_dAccum.GetLength() )
		m_dAccum.Reserve ( 128*1024 );

	// accumulate row data; expect fully dynamic rows
	assert ( !tDoc.m_pStatic );
	assert (!( !tDoc.m_pDynamic && iRowSize!=0 ));
	assert (!( tDoc.m_pDynamic && (int)tDoc.m_pDynamic[-1]!=iRowSize ));

	m_dAccumRows.Resize ( m_dAccumRows.GetLength() + DOCINFO_IDSIZE + iRowSize );
	CSphRowitem * pRow = &m_dAccumRows [ m_dAccumRows.GetLength() - DOCINFO_IDSIZE - iRowSize ];
	DOCINFOSETID ( pRow, tDoc.m_uDocID );

	CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
	for ( int i=0; i<iRowSize; i++ )
		pAttrs[i] = tDoc.m_pDynamic[i];

	int iMva = 0;

	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();
	int iAttr = 0;
	int iJsonAttr = 0;
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		bool bJsonCleanup = false;
		const CSphColumnInfo & tColumn = tSchema.GetAttr(i);
		if ( tColumn.m_eAttrType==SPH_ATTR_STRING || tColumn.m_eAttrType==SPH_ATTR_JSON )
		{
			const char * pStr = ppStr ? ppStr[iAttr++] : NULL;
			int iLen = pStr ? strlen ( pStr ) : 0;

			CSphVector<BYTE> dBuf;
			if ( pStr && iLen && tColumn.m_eAttrType==SPH_ATTR_JSON )
			{
				pStr = (const char*)dJson[iJsonAttr].m_pData;
				iLen = dJson[iJsonAttr].m_iLen;
				bJsonCleanup = true;
			}

			if ( pStr && iLen )
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

			if ( bJsonCleanup )
				delete [] dJson[iJsonAttr++].m_pData;
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

	// handle index_field_lengths
	DWORD * pFieldLens = NULL;
	if ( m_pIndex->GetSettings().m_bIndexFieldLens )
	{
		int iFirst = tSchema.GetAttrId_FirstFieldLen();
		assert ( tSchema.GetAttr ( iFirst ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		assert ( tSchema.GetAttr ( iFirst+tSchema.m_dFields.GetLength()-1 ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		pFieldLens = pAttrs + ( tSchema.GetAttr ( iFirst ).m_tLocator.m_iBitOffset / 32 );
		memset ( pFieldLens, 0, sizeof(int)*tSchema.m_dFields.GetLength() ); // NOLINT
	}

	// accumulate hits
	int iHits = 0;
	if ( pHits && pHits->Length() )
	{
		CSphWordHit tLastHit;
		tLastHit.m_uDocID = 0;
		tLastHit.m_uWordID = 0;
		tLastHit.m_uWordPos = 0;

		iHits = pHits->Length();
		m_dAccum.Reserve ( m_dAccum.GetLength()+iHits );
		for ( const CSphWordHit * pHit = pHits->First(); pHit<=pHits->Last(); pHit++ )
		{
			// ignore duplicate hits
			if ( pHit->m_uDocID==tLastHit.m_uDocID && pHit->m_uWordID==tLastHit.m_uWordID && pHit->m_uWordPos==tLastHit.m_uWordPos )
				continue;

			// update field lengths
			if ( pFieldLens && HITMAN::GetField ( pHit->m_uWordPos )!=HITMAN::GetField ( tLastHit.m_uWordPos ) )
				pFieldLens [ HITMAN::GetField ( tLastHit.m_uWordPos ) ] = HITMAN::GetPos ( tLastHit.m_uWordPos );

			// accumulate
			m_dAccum.Add ( *pHit );
			tLastHit = *pHit;
		}
		if ( pFieldLens )
			pFieldLens [ HITMAN::GetField ( tLastHit.m_uWordPos ) ] = HITMAN::GetPos ( tLastHit.m_uWordPos );
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
		const char * sWord = pBase + pSeg->m_dWordCheckpoints[i].m_uWordID;
		pSeg->m_dWordCheckpoints[i].m_sWord = sWord;
	}
}


RtSegment_t * RtAccum_t::CreateSegment ( int iRowSize, int iWordsCheckpoint )
{
	if ( !m_iAccumDocs )
		return NULL;

	MEMORY ( MEM_RT_ACCUM );

	RtSegment_t * pSeg = new RtSegment_t ();

	CSphWordHit tClosingHit;
	tClosingHit.m_uWordID = WORDID_MAX;
	tClosingHit.m_uDocID = DOCID_MAX;
	tClosingHit.m_uWordPos = EMPTY_HIT;
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
	Hitpos_t uPrevHit = EMPTY_HIT;
	ARRAY_FOREACH ( i, m_dAccum )
	{
		const CSphWordHit & tHit = m_dAccum[i];

		// new keyword or doc; flush current doc
		if ( tHit.m_uWordID!=tWord.m_uWordID || tHit.m_uDocID!=tDoc.m_uDocID )
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

			tDoc.m_uDocID = tHit.m_uDocID;
			tOutHit.ZipRestart ();
			uEmbeddedHit = EMPTY_HIT;
			uPrevHit = EMPTY_HIT;
		}

		// new keyword; flush current keyword
		if ( tHit.m_uWordID!=tWord.m_uWordID )
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

			tWord.m_uWordID = tHit.m_uWordID;
			tWord.m_uDocs = 0;
			tWord.m_uHits = 0;
			tWord.m_uDoc = tOutDoc.ZipDocPtr();
			uPrevHit = EMPTY_HIT;
		}

		// might be a duplicate
		if ( uPrevHit==tHit.m_uWordPos )
			continue;

		// just a new hit
		if ( !tDoc.m_uHits )
		{
			uEmbeddedHit = tHit.m_uWordPos;
		} else
		{
			if ( uEmbeddedHit )
			{
				tOutHit.ZipHit ( uEmbeddedHit );
				uEmbeddedHit = 0;
			}

			tOutHit.ZipHit ( tHit.m_uWordPos );
		}
		uPrevHit = tHit.m_uWordPos;

		const int iField = HITMAN::GetField ( tHit.m_uWordPos );
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


void RtAccum_t::CleanupDuplicates ( int iRowSize )
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

	bool bHasDups = ARRAY_ANY ( bHasDups, dDocHits, ( _any>0 ) && ( dDocHits[_any-1].m_uDocid==dDocHits[_any].m_uDocid ) );
	if ( !bHasDups )
		return;

	// identify duplicates to kill, and store them in dDocHits
	int iDst = 0;
	if ( m_bReplace )
	{
		// replace mode, last value wins, precending values are duplicate
		for ( int iSrc=0; iSrc<dDocHits.GetLength()-1; iSrc++ )
			if ( dDocHits[iSrc].m_uDocid==dDocHits[iSrc+1].m_uDocid ) // if my next value has the same docid, i am dupe
				dDocHits[iDst++] = dDocHits[iSrc];
	} else
	{
		// insert mode, first value wins, subsequent values are duplicates
		for ( int iSrc=1; iSrc<dDocHits.GetLength(); iSrc++ )
			if ( dDocHits[iSrc].m_uDocid==dDocHits[iSrc-1].m_uDocid ) // if my prev value has the same docid, i am a dupe
				dDocHits[iDst++] = dDocHits[iSrc];
	}
	dDocHits.Resize ( iDst );
	assert ( dDocHits.GetLength() );

	// sort by hit index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iHitIndex ) );

	// clean up hits of duplicates
	int iSrc;
	for ( int iHit = dDocHits.GetLength()-1; iHit>=0; iHit-- )
	{
		if ( !dDocHits[iHit].m_iHitCount )
			continue;

		int iFrom = dDocHits[iHit].m_iHitIndex;
		int iCount = dDocHits[iHit].m_iHitCount;
		if ( iFrom+iCount<m_dAccum.GetLength() )
		{
			for ( iDst=iFrom, iSrc=iFrom+iCount; iSrc<m_dAccum.GetLength(); iSrc++, iDst++ )
				m_dAccum[iDst] = m_dAccum[iSrc];
		}
		m_dAccum.Resize ( m_dAccum.GetLength()-iCount );
	}

	// sort by docid index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iDocIndex ) );

	// clean up docinfos of duplicates
	for ( int iDoc = dDocHits.GetLength()-1; iDoc>=0; iDoc-- )
	{
		iDst = dDocHits[iDoc].m_iDocIndex*iStride;
		iSrc = iDst+iStride;
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
		bool bKill = ( pSrc->GetKlist().BinarySearch ( pDoc->m_uDocID )!=NULL );
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
			if ( !pSrc2->GetKlist().BinarySearch ( pDoc2->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pSrc2->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc2->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc2, pDoc2 );
			pDoc1 = tIn1.UnzipDoc();
			pDoc2 = tIn2.UnzipDoc();

		} else if ( pDoc1 && ( !pDoc2 || pDoc1->m_uDocID < pDoc2->m_uDocID ) )
		{
			// winner from the first segment
			if ( !pSrc1->GetKlist().BinarySearch ( pDoc1->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc1->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc1, pDoc1 );
			pDoc1 = tIn1.UnzipDoc();

		} else
		{
			// winner from the second segment
			assert ( pDoc2 && ( !pDoc1 || pDoc2->m_uDocID < pDoc1->m_uDocID ) );
			if ( !pSrc2->GetKlist().BinarySearch ( pDoc2->m_uDocID )
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
	const DOCID * m_pTlsKlist;
	const DOCID * m_pTlsKlistMax;
	const int m_iStride;

	const SphDocID_t *	m_pKlist;
	const SphDocID_t *	m_pKlistMax;

public:
	explicit RtRowIterator_T ( const RtSegment_t * pSeg, int iStride, bool bWriter, const CSphVector<DOCID> * pAccKlist, const CSphFixedVector<SphDocID_t> & tKill )
		: m_pRow ( pSeg->m_dRows.Begin() )
		, m_pRowMax ( pSeg->m_dRows.Begin() + pSeg->m_dRows.GetLength() )
		, m_pTlsKlist ( NULL )
		, m_pTlsKlistMax ( NULL )
		, m_iStride ( iStride )
		, m_pKlist ( NULL )
		, m_pKlistMax ( NULL )
	{
		if ( tKill.GetLength() )
		{
			m_pKlist = tKill.Begin();
			m_pKlistMax = tKill.Begin() + tKill.GetLength();
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
void VerifyEmptyStrings ( const CSphTightVector<BYTE> & dStorage, const CSphSchema & tSchema, const CSphRowitem * pRow )
{
	if ( dStorage.GetLength()>1 )
		return;

	const DWORD * pAttr = DOCINFO2ATTRS(pRow);
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
	// plain and rt indexes have different formats for storing empty MVA values
	// plain stores legal offset in attribute and zero in MVA pool
	// rt stores 0 as offset in attribute and non a single byte in MVA pool
	// we should handle here cases where plain was ATTACHed to rt like this
	if ( !uCount )
		return 0;

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
		return CopyMva ( pSrc, m_dDst );
	}
};


template <typename STORAGE, typename SRC>
void CopyFixupStorageAttrs ( const CSphTightVector<SRC> & dSrc, STORAGE & tStorage, CSphRowitem * pRow )
{
	const CSphVector<CSphAttrLocator> & dLocators = tStorage.GetLocators();
	if ( !dLocators.GetLength() )
		return;

	// store string\mva attr for this row
	SphDocID_t uDocid = DOCINFO2ID ( pRow );
	DWORD * pAttr = DOCINFO2ATTRS( pRow );
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
	assert ( iLen<=SPH_MAX_WORD_LEN || ( bUtf8 && iLen<=SPH_MAX_WORD_LEN*3 ) );
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


void RtIndex_t::BuildSegmentInfixes ( RtSegment_t * pSeg, bool bHasMorphology ) const
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
		const BYTE * pDictWord = pWord->m_sWord+1;
		if ( bHasMorphology && *pDictWord!=MAGIC_WORD_HEAD_NONSTEMMED )
			continue;

		int iLen = pWord->m_sWord[0];
		if ( *pDictWord<0x20 ) // anyway skip heading magic chars in the prefix, like NONSTEMMED maker
		{
			pDictWord++;
			iLen--;
		}

		uint64_t * pVal = pRough + rdDictRough.m_iCheckpoint * BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT;
		BuildBloom ( pDictWord, iLen, 2, ( m_iMaxCodepointLength>1 ), pVal+BLOOM_PER_ENTRY_VALS_COUNT*0, BLOOM_PER_ENTRY_VALS_COUNT );
		BuildBloom ( pDictWord, iLen, 4, ( m_iMaxCodepointLength>1 ), pVal+BLOOM_PER_ENTRY_VALS_COUNT*1, BLOOM_PER_ENTRY_VALS_COUNT );
	}
}


RtSegment_t * RtIndex_t::MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist, bool bHasMorphology )
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

	RtRowIterator_t tIt1 ( pSeg1, m_iStride, true, pAccKlist, pSeg1->GetKlist() );
	RtRowIterator_t tIt2 ( pSeg2, m_iStride, true, pAccKlist, pSeg2->GetKlist() );

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
			CopyFixupStorageAttrs ( pSeg1->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs ( pSeg1->m_dMvas, tStorageMva, pDstRow );
			pRow1 = tIt1.GetNextAliveRow();
		} else
		{
			assert ( pRow2 );
			assert ( !pRow1 || ( DOCINFO2ID(pRow1)!=DOCINFO2ID(pRow2) ) ); // all dupes must be killed and skipped by the iterator
			for ( int i=0; i<m_iStride; i++ )
				dRows.Add ( *pRow2++ );
			CSphRowitem * pDstRow = dRows.Begin() + dRows.GetLength() - m_iStride;
			CopyFixupStorageAttrs ( pSeg2->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs ( pSeg2->m_dMvas, tStorageMva, pDstRow );
			pRow2 = tIt2.GetNextAliveRow();
		}
		pSeg->m_iRows++;
		pSeg->m_iAliveRows++;
	}

	assert ( pSeg->m_iRows*m_iStride==pSeg->m_dRows.GetLength() );
#if PARANOID
	CheckSegmentRows ( pSeg, m_iStride );
#endif

	// merged segment might be completely killed by committed data
	if ( !pSeg->m_iRows )
	{
		SafeDelete ( pSeg );
		return NULL;
	}

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

	BuildSegmentInfixes ( pSeg, bHasMorphology );

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


void RtIndex_t::Commit ( int * pDeleted, ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( MEM_INDEX_RT );

	RtAccum_t * pAcc = AcquireAccum ( NULL, pAccExt, true );
	if ( !pAcc )
		return;

	// empty txn, just ignore
	if ( !pAcc->m_iAccumDocs && !pAcc->m_dAccumKlist.GetLength() )
	{
		pAcc->SetIndex ( NULL );
		pAcc->m_dAccumRows.Resize ( 0 );
		pAcc->m_dStrings.Resize ( 1 );
		pAcc->m_dMvas.Resize ( 1 );
		pAcc->m_dPerDocHitsCount.Resize ( 0 );
		pAcc->ResetDict();
		return;
	}

	// phase 0, build a new segment
	// accum and segment are thread local; so no locking needed yet
	// segment might be NULL if we're only killing rows this txn
	pAcc->CleanupDuplicates ( m_tSchema.GetRowSize() );
	pAcc->Sort();

	RtSegment_t * pNewSeg = pAcc->CreateSegment ( m_tSchema.GetRowSize(), m_iWordsCheckpoint );
	assert ( !pNewSeg || pNewSeg->m_iRows>0 );
	assert ( !pNewSeg || pNewSeg->m_iAliveRows>0 );
	assert ( !pNewSeg || pNewSeg->m_bTlsKlist==false );

	BuildSegmentInfixes ( pNewSeg, m_pDict->HasMorphology() );

#if PARANOID
	if ( pNewSeg )
		CheckSegmentRows ( pNewSeg, m_iStride );
#endif

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );
	pAcc->m_dStrings.Resize ( 1 ); // handle dummy zero offset
	pAcc->m_dMvas.Resize ( 1 );
	pAcc->m_dPerDocHitsCount.Resize ( 0 );
	pAcc->ResetDict();

	// sort accum klist, too
	pAcc->m_dAccumKlist.Uniq ();

	// now on to the stuff that needs locking and recovery
	CommitReplayable ( pNewSeg, pAcc->m_dAccumKlist, pDeleted );

	// done; cleanup accum
	pAcc->SetIndex ( NULL );
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
	// reset accumulated warnings
	CSphString sWarning;
	pAcc->GrabLastWarning ( sWarning );
}

void RtIndex_t::CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist, int * pTotalKilled )
{
	// store statistics, because pNewSeg just might get merged
	int iNewDocs = pNewSeg ? pNewSeg->m_iRows : 0;

	CSphVector<int64_t> dLens;
	int iFirstFieldLenAttr = m_tSchema.GetAttrId_FirstFieldLen();
	if ( pNewSeg && iFirstFieldLenAttr>=0 )
	{
		assert ( pNewSeg->GetStride()==m_iStride );
		int iFields = m_tSchema.m_dFields.GetLength(); // shortcut
		dLens.Resize ( iFields );
		dLens.Fill ( 0 );
		for ( int i=0; i<pNewSeg->m_iRows; i++ )
			for ( int j=0; j<iFields; j++ )
				dLens[j] += sphGetRowAttr ( &pNewSeg->m_dRows [ i*m_iStride+DOCINFO_IDSIZE ], m_tSchema.GetAttr ( j+iFirstFieldLenAttr ).m_tLocator );
	}

	// phase 1, lock out other writers (but not readers yet)
	// concurrent readers are ok during merges, as existing segments won't be modified yet
	// however, concurrent writers are not
	Verify ( m_tWriting.Lock() );

	// first of all, binlog txn data for recovery
	g_pRtBinlog->BinlogCommit ( &m_iTID, m_sIndexName.cstr(), pNewSeg, dAccKlist, m_bKeywordDict );
	int64_t iTID = m_iTID;

	// let merger know that existing segments are subject to additional, TLS K-list filter
	// safe despite the readers, flag must only be used by writer
	if ( dAccKlist.GetLength() )
		for ( int i=m_iDoubleBuffer; i<m_dRamChunks.GetLength(); i++ )
		{
			// OPTIMIZE? only need to set the flag if TLS K-list *actually* affects segment
			assert ( m_dRamChunks[i]->m_bTlsKlist==false );
			m_dRamChunks[i]->m_bTlsKlist = true;
		}

	// prepare new segments vector
	// create more new segments by merging as needed
	// do not (!) kill processed old segments just yet, as readers might still need them
	CSphVector<RtSegment_t*> dSegments;

	dSegments.Reserve ( m_dRamChunks.GetLength() - m_iDoubleBuffer + 1 );
	for ( int i=m_iDoubleBuffer; i<m_dRamChunks.GetLength(); i++ )
	{
		dSegments.Add ( m_dRamChunks[i] );
	}
	if ( pNewSeg )
		dSegments.Add ( pNewSeg );

	int64_t iRamFreed = 0;
	bool bHasMorphology = m_pDict->HasMorphology();
	FreeRetired();

	// enforce RAM usage limit
	int64_t iRamLeft = m_iDoubleBuffer ? m_iDoubleBufferLimit : m_iSoftRamLimit;
	ARRAY_FOREACH ( i, dSegments )
		iRamLeft = Max ( iRamLeft - dSegments[i]->GetUsedRam(), 0 );
	ARRAY_FOREACH ( i, m_dRetired )
		iRamLeft = Max ( iRamLeft - m_dRetired[i]->GetUsedRam(), 0 );

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
		RtSegment_t * pMerged = MergeSegments ( pA, pB, &dAccKlist, bHasMorphology );
		if ( pMerged )
		{
			int64_t iMerged = pMerged->GetUsedRam();
			iRamLeft -= Min ( iRamLeft, iMerged );
			dSegments.Add ( pMerged );
		}
		m_dRetired.Add ( pA );
		m_dRetired.Add ( pB );

		iRamFreed += pA->GetUsedRam() + pB->GetUsedRam();
	}

	// phase 2, obtain exclusive writer lock
	// we now have to update K-lists in (some of) the survived segments
	// and also swap in new segment list

	// adjust for an incoming accumulator K-list
	int iTotalKilled = 0;
	int iDiskLiveKLen = 0;
	if ( dAccKlist.GetLength() )
	{
		// update totals
		// work the original (!) segments, and before (!) updating their K-lists
		iDiskLiveKLen = dAccKlist.GetLength();
		for ( int i=0; i<iDiskLiveKLen; i++ )
		{
			SphDocID_t uDocid = dAccKlist[i];

			// the most recent part of RT index is its RAM chunk, so first search it
			// then search chunk which is saving right now
			// after that search disk chunks in order from younger to older ones
			// if doc is killed in younger index part then it's really killed - no need to search older parts
			bool bRamAlive = false;
			bool bSavedOrDiskAlive = false;
			bool bAlreadyKilled = false;
			for ( ;; )
			{
				for ( int j=m_dRamChunks.GetLength()-1; j>=m_iDoubleBuffer && !bRamAlive; --j )
					bRamAlive = !!m_dRamChunks[j]->FindAliveRow ( uDocid );
				if ( bRamAlive )
					break;

				// killed in saved or one of disk chunks? but not during double buffer work
				if ( !m_iDoubleBuffer && m_tKlist.Exists ( uDocid ) )
				{
					bAlreadyKilled = true;
					break;
				}

				for ( int j=m_iDoubleBuffer-1; j>=0 && !bSavedOrDiskAlive; --j )
					bSavedOrDiskAlive = !!m_dRamChunks[j]->FindAliveRow ( uDocid );
				if ( bSavedOrDiskAlive )
					break;

				// killed in one of disk chunks?
				if ( m_dDiskChunkKlist.BinarySearch ( uDocid )!=NULL )
					break;

				for ( int j=m_dDiskChunks.GetLength()-1; j>=0; --j )
				{
					bSavedOrDiskAlive = m_dDiskChunks[j]->HasDocid ( uDocid );
					if ( bSavedOrDiskAlive )
						break;
					// killed in previous disk chunks?
					if ( sphBinarySearch ( m_dDiskChunks[j]->GetKillList(), m_dDiskChunks[j]->GetKillList()+m_dDiskChunks[j]->GetKillListSize()-1, uDocid ) )
						break;
				}

				break;
			}

			if ( bRamAlive || bSavedOrDiskAlive )
				iTotalKilled++;

			if ( bAlreadyKilled || !bSavedOrDiskAlive )
			{
				// we can't just RemoveFast() elements from vector
				// because we'll use its values with indexes >=iDiskLiveKLen in segments kill lists just below
				Swap ( dAccKlist[i], dAccKlist[iDiskLiveKLen-1] );
				iDiskLiveKLen--;
				i--;
			}
		}

		CSphVector<SphDocID_t> dSegmentKlist;

		// update K-lists on survivors
		ARRAY_FOREACH ( iSeg, dSegments )
		{
			RtSegment_t * pSeg = dSegments[iSeg];
			if ( !pSeg->m_bTlsKlist )
				continue; // should be fresh enough

			dSegmentKlist.Resize ( 0 );
			ARRAY_FOREACH ( j, dAccKlist )
			{
				SphDocID_t uDocid = dAccKlist[j];
				if ( pSeg->FindAliveRow ( uDocid ) )
					dSegmentKlist.Add ( uDocid );
			}

			// now actually update it
			if ( dSegmentKlist.GetLength() )
			{
				int iAdded = dSegmentKlist.GetLength();
				dSegmentKlist.Resize ( pSeg->GetKlist().GetLength() + iAdded );
				memcpy ( dSegmentKlist.Begin() + iAdded, pSeg->GetKlist().Begin(), sizeof(dSegmentKlist[0]) * pSeg->GetKlist().GetLength() );
				dSegmentKlist.Uniq();

				KlistRefcounted_t * pKlist = new KlistRefcounted_t();
				pKlist->m_dKilled.Reset ( dSegmentKlist.GetLength() );
				memcpy ( pKlist->m_dKilled.Begin(), dSegmentKlist.Begin(), sizeof(dSegmentKlist[0]) * dSegmentKlist.GetLength() );

				// swap data, update counters
				m_tChunkLock.WriteLock();

				uint64_t uRefs = pSeg->m_pKlist->m_tRefCount.Dec();
				Swap ( pSeg->m_pKlist, pKlist ); // hold swapped kill-list for postponed delete
				pSeg->m_iAliveRows -= iAdded;
				assert ( pSeg->m_iAliveRows>=0 );

				m_tChunkLock.Unlock();

				if ( uRefs==1 ) // 1 means we only owner when decrement event occurred
					SafeDelete ( pKlist );
			}

			// mark as good
			pSeg->m_bTlsKlist = false;
		}

		// collect kill-list for new segments
		if ( m_iDoubleBuffer )
		{
			int iOff = m_dNewSegmentKlist.GetLength();
			m_dNewSegmentKlist.Resize ( iOff + iDiskLiveKLen );
			memcpy ( m_dNewSegmentKlist.Begin()+iOff, dAccKlist.Begin(), sizeof(SphDocID_t)*iDiskLiveKLen );
		}
	}

	// update saved chunk and disk chunks kill list
	// after iDiskLiveKLen IDs are already killed or don't exist - just skip them
	if ( iDiskLiveKLen )
		m_tKlist.Add ( dAccKlist.Begin(), iDiskLiveKLen );

	ARRAY_FOREACH ( i, dSegments )
	{
		RtSegment_t * pSeg = dSegments[i];
		if ( pSeg->m_iAliveRows==0 )
		{
			m_dRetired.Add ( pSeg );
			dSegments.RemoveFast ( i );
			i--;
		}
	}

	// wipe out readers - now we are only using RAM segments
	m_tChunkLock.WriteLock ();

	// go live!
	// got rid of 'old' double-buffer segments then add 'new' onces
	m_dRamChunks.Resize ( m_iDoubleBuffer + dSegments.GetLength() );
	memcpy ( m_dRamChunks.Begin() + m_iDoubleBuffer, dSegments.Begin(), sizeof(dSegments[0]) * dSegments.GetLength() );

	// phase 3, enable readers again
	// we might need to dump data to disk now
	// but during the dump, readers can still use RAM chunk data
	Verify ( m_tChunkLock.Unlock() );

	// update stats
	m_tStats.m_iTotalDocuments += iNewDocs - iTotalKilled;

	if ( m_tSchema.GetAttrId_FirstFieldLen()>=0 )
		ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		{
			m_dFieldLensRam[i] += dLens[i];
			m_dFieldLens[i] = m_dFieldLensRam[i] + m_dFieldLensDisk[i];
		}

	// get flag of double-buffer prior mutex unlock
	bool bDoubleBufferActive = ( m_iDoubleBuffer>0 );

	// tell about DELETE affected_rows
	if ( pTotalKilled )
		*pTotalKilled = iTotalKilled;

	// we can kill retired segments now
	FreeRetired();

	// double buffer writer stands still till save done
	// all writers waiting double buffer done
	// no need to dump or waiting for some writer
	if ( !bDump || bDoubleBufferActive )
	{
		// all done, enable other writers
		Verify ( m_tWriting.Unlock() );
		return;
	}

	// scope for guard then retired clean up
	{
		// copy stats for disk chunk
		SphChunkGuard_t tGuard;
		GetReaderChunks ( tGuard );

		ChunkStats_t tStat2Dump ( m_tStats, m_dFieldLensRam );
		m_iDoubleBuffer = m_dRamChunks.GetLength();

		m_dDiskChunkKlist.Resize ( 0 );
		m_tKlist.Flush ( m_dDiskChunkKlist );

		Verify ( m_tWriting.Unlock() );

		SaveDiskChunk ( iTID, tGuard, tStat2Dump );
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), iTID, false );
	}
}


void RtIndex_t::FreeRetired()
{
	m_dRetired.Uniq();
	ARRAY_FOREACH ( i, m_dRetired )
	{
		if ( m_dRetired[i]->m_tRefCount.GetValue()==0 )
		{
			SafeDelete ( m_dRetired[i] );
			m_dRetired.RemoveFast ( i );
			i--;
		}
	}
}


void RtIndex_t::RollBack ( ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = AcquireAccum ( NULL, pAccExt, true );
	if ( !pAcc )
		return;

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );
	pAcc->m_dStrings.Resize ( 1 ); // handle dummy zero offset
	pAcc->m_dMvas.Resize ( 1 );
	pAcc->m_dPerDocHitsCount.Resize ( 0 );
	pAcc->ResetDict();

	// finish cleaning up and release accumulator
	pAcc->SetIndex ( NULL );
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
}

bool RtIndex_t::DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError, ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( MEM_RT_ACCUM );

	RtAccum_t * pAcc = AcquireAccum ( &sError, pAccExt, true );
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
	MEMORY ( MEM_INDEX_RT );

	if ( !m_dRamChunks.GetLength() )
		return;

	Verify ( m_tWriting.Lock() );

	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );

	m_dDiskChunkKlist.Resize ( 0 );
	m_tKlist.Flush ( m_dDiskChunkKlist );
	Verify ( m_tWriting.Unlock() );

	ChunkStats_t s ( m_tStats, m_dFieldLensRam );
	SaveDiskChunk ( m_iTID, tGuard, s );
}


struct SaveSegment_t
{
	const RtSegment_t *		m_pSeg;
	const CSphFixedVector<SphDocID_t> * m_pKill;
};


void RtIndex_t::SaveDiskDataImpl ( const char * sFilename, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats ) const
{
	typedef RtDoc_T<SphDocID_t> RTDOC;
	typedef RtWord_T<SphWordID_t> RTWORD;

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
	CSphVector<RtWordReader_T<SphWordID_t>*> pWordReaders;
	CSphVector<RtDocReader_T<SphDocID_t>*> pDocReaders;
	CSphVector<SaveSegment_t> pSegments;
	CSphVector<const RTWORD*> pWords;
	CSphVector<const RTDOC*> pDocs;

	int iSegments = tGuard.m_dRamChunks.GetLength();

	pWordReaders.Reserve ( iSegments );
	pDocReaders.Reserve ( iSegments );
	pSegments.Reserve ( iSegments );
	pWords.Reserve ( iSegments );
	pDocs.Reserve ( iSegments );

	////////////////////
	// write attributes
	////////////////////

	// the new, template-param aligned iStride instead of index-wide
	int iStride = DWSIZEOF(SphDocID_t) + m_tSchema.GetRowSize();
	CSphFixedVector<RtRowIterator_T<SphDocID_t>*> pRowIterators ( iSegments );
	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		pRowIterators[i] = new RtRowIterator_T<SphDocID_t> ( tGuard.m_dRamChunks[i], iStride, false, NULL, tGuard.m_dKill[i]->m_dKilled );

	CSphVector<const CSphRowitem*> pRows ( iSegments );
	ARRAY_FOREACH ( i, pRowIterators )
		pRows[i] = pRowIterators[i]->GetNextAliveRow();

	// prepare to build min-max index for attributes too
	int iTotalDocs = 0;
	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		iTotalDocs += tGuard.m_dRamChunks[i]->m_iAliveRows;

	AttrIndexBuilder_t<SphDocID_t> tMinMaxBuilder ( m_tSchema );
	CSphVector<DWORD> dMinMaxBuffer ( int ( tMinMaxBuilder.GetExpectedSize ( iTotalDocs ) ) ); // RT index doesn't support over 4Gb .spa
	tMinMaxBuilder.Prepare ( dMinMaxBuffer.Begin(), dMinMaxBuffer.Begin() + dMinMaxBuffer.GetLength() );

	sName.SetSprintf ( "%s.sps", sFilename );
	CSphWriter tStrWriter;
	tStrWriter.OpenFile ( sName.cstr(), sError );
	tStrWriter.PutByte ( 0 ); // dummy byte, to reserve magic zero offset

	sName.SetSprintf ( "%s.spm", sFilename );
	CSphWriter tMvaWriter;
	tMvaWriter.OpenFile ( sName.cstr(), sError );
	tMvaWriter.PutDword ( 0 ); // dummy dword, to reserve magic zero offset

	SphDocID_t iMinDocID = DOCID_MAX;
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
				if ( iMinRow<0 || DOCINFO2ID ( pRows[i] ) < DOCINFO2ID ( pRows[iMinRow] ) )
					iMinRow = i;
		if ( iMinRow<0 )
			break;

#ifndef NDEBUG
		// verify that it's unique
		int iDupes = 0;
		ARRAY_FOREACH ( i, pRows )
			if ( pRows[i] )
				if ( DOCINFO2ID ( pRows[i] )==DOCINFO2ID ( pRows[iMinRow] ) )
					iDupes++;
		assert ( iDupes==1 );
#endif

		const CSphRowitem * pRow = pRows[iMinRow];

		// strings storage for stored row
		assert ( iMinRow<iSegments );
		const RtSegment_t * pSegment = tGuard.m_dRamChunks[iMinRow];

#ifdef PARANOID // sanity check in PARANOID mode
		VerifyEmptyStrings ( pSegment->m_dStrings, m_tSchema, pRow );
#endif

		// collect min-max data
		Verify ( tMinMaxBuilder.Collect ( pRow, pSegment->m_dMvas.Begin(), pSegment->m_dMvas.GetLength(), sError, false ) );

		if ( iMinDocID==DOCID_MAX )
			iMinDocID = DOCINFO2ID ( pRows[iMinRow] );

		if ( pSegment->m_dStrings.GetLength()>1 || pSegment->m_dMvas.GetLength()>1 ) // should be more then dummy zero elements
		{
			// copy row content as we'll fix up its attrs ( string offset for now )
			memcpy ( pFixedRow, pRow, iStride*sizeof(CSphRowitem) );
			pRow = pFixedRow;

			CopyFixupStorageAttrs ( pSegment->m_dStrings, tStorageString, pFixedRow );
			CopyFixupStorageAttrs ( pSegment->m_dMvas, tStorageMva, pFixedRow );
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
	SphOffset_t uMinMaxOff = wrRows.GetPos() / sizeof(CSphRowitem);
	if ( tMinMaxBuilder.GetActualSize() )
		wrRows.PutBytes ( dMinMaxBuffer.Begin(), tMinMaxBuilder.GetActualSize()*sizeof(DWORD) );

	tMvaWriter.CloseFile();
	tStrWriter.CloseFile ();

	////////////////////
	// write docs & hits
	////////////////////

	assert ( iMinDocID>0 );
	iMinDocID--;

	// OPTIMIZE? somehow avoid new on iterators maybe?
	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		pWordReaders.Add ( new RtWordReader_T<SphWordID_t> ( tGuard.m_dRamChunks[i], m_bKeywordDict, m_iWordsCheckpoint ) );

	ARRAY_FOREACH ( i, pWordReaders )
		pWords.Add ( pWordReaders[i]->UnzipWord() );

	// loop keywords
	CSphVector<Checkpoint_t> dCheckpoints;
	CSphVector<BYTE> dKeywordCheckpoints;
	int iWords = 0;
	CSphKeywordDeltaWriter tLastWord;
	SphWordID_t uLastWordID = 0;
	SphOffset_t uLastDocpos = 0;
	CSphVector<SkiplistEntry_t> dSkiplist;

	bool bHasMorphology = m_pDict->HasMorphology();

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
				pSegments.Add ();
				pSegments.Last().m_pSeg = tGuard.m_dRamChunks[i];
				pSegments.Last().m_pKill = &tGuard.m_dKill[i]->m_dKilled;
				pDocReaders.Add ( new RtDocReader_T<SphDocID_t> ( tGuard.m_dRamChunks[i], *pWords[i] ) );

				const RTDOC * pDoc = pDocReaders.Last()->UnzipDoc();
				while ( pDoc && tGuard.m_dKill[i]->m_dKilled.BinarySearch ( pDoc->m_uDocID ) )
					pDoc = pDocReaders.Last()->UnzipDoc();

				pDocs.Add ( pDoc );
			}

		// loop documents
		SphOffset_t uDocpos = wrDocs.GetPos();
		SphDocID_t uLastDoc = 0;
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

				assert ( !pSegments[i].m_pKill->BinarySearch ( pDocs[i]->m_uDocID ) );
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
				RtHitReader_t tInHit ( pSegments[iMinReader].m_pSeg, pDoc );
				for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
				{
					wrHits.ZipInt ( uValue - uLastHit );
					uLastHit = uValue;
				}
				wrHits.ZipInt ( 0 );
			}

			// fast forward readers
			SphDocID_t uMinID = pDocs[iMinReader]->m_uDocID;
			ARRAY_FOREACH ( i, pDocs )
				while ( pDocs[i] && ( pDocs[i]->m_uDocID<=uMinID || pSegments[i].m_pKill->BinarySearch ( pDocs[i]->m_uDocID ) ) )
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
					pInfixer->AddWord ( pWord->m_sWord+1, pWord->m_sWord[0], dCheckpoints.GetLength(), bHasMorphology );
			}

			// emit skiplist pointer
			if ( iDocs>SPH_SKIPLIST_BLOCK )
				wrDict.ZipInt ( iSkiplistOff );

			uLastDocpos = uDocpos;
		}

		// move words forward
		// because pWord contents will move forward too!
		SphWordID_t uMinID = pWord->m_uWordID;
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

	int64_t iInfixBlockOffset = 0;
	int iInfixCheckpointWordsSize = 0;
	// flush infix hash blocks
	if ( pInfixer.Ptr() )
	{
		iInfixBlockOffset = pInfixer->SaveEntryBlocks ( wrDict );
		iInfixCheckpointWordsSize = pInfixer->GetBlocksWordsSize();

		if ( iInfixBlockOffset>UINT_MAX )
			sphWarning ( "INTERNAL ERROR: dictionary size "INT64_FMT" overflow at infix save", iInfixBlockOffset );
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	wrDict.PutBytes ( "dict-header", 11 );
	wrDict.ZipInt ( dCheckpoints.GetLength() );
	wrDict.ZipOffset ( iCheckpointsPosition );
	wrDict.ZipInt ( m_pTokenizer->GetMaxCodepointLength() );
	wrDict.ZipInt ( (DWORD)iInfixBlockOffset );

	// write dummy kill-list files
	CSphWriter wrDummy;
	// dump killlist
	sName.SetSprintf ( "%s.spk", sFilename );
	wrDummy.OpenFile ( sName.cstr(), sError );
	if ( m_dDiskChunkKlist.GetLength() )
		wrDummy.PutBytes ( m_dDiskChunkKlist.Begin(), m_dDiskChunkKlist.GetLength()*sizeof ( SphDocID_t ) );
	wrDummy.CloseFile ();

	// header
	SaveDiskHeader ( sFilename, iMinDocID, dCheckpoints.GetLength(), iCheckpointsPosition, (DWORD)iInfixBlockOffset, iInfixCheckpointWordsSize,
		m_dDiskChunkKlist.GetLength(), uMinMaxOff, tStats );

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


void RtIndex_t::SaveDiskHeader ( const char * sFilename, SphDocID_t iMinDocID, int iCheckpoints,
	SphOffset_t iCheckpointsPosition, DWORD iInfixBlocksOffset, int iInfixCheckpointWordsSize, DWORD uKillListSize, uint64_t uMinMaxSize,
	const ChunkStats_t & tStats ) const
{
	static const DWORD INDEX_FORMAT_VERSION	= 39;			///< my format version

	CSphWriter tWriter;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.sph", sFilename );
	tWriter.OpenFile ( sName.cstr(), sError );

	// format
	tWriter.PutDword ( INDEX_MAGIC_HEADER );
	tWriter.PutDword ( INDEX_FORMAT_VERSION );

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
	tWriter.PutDword ( (DWORD)tStats.m_Stats.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	tWriter.PutOffset ( tStats.m_Stats.m_iTotalBytes );

	// index settings
	tWriter.PutDword ( m_tSettings.m_iMinPrefixLen );
	tWriter.PutDword ( m_tSettings.m_iMinInfixLen );
	tWriter.PutDword ( m_tSettings.m_iMaxSubstringLen );
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
	tWriter.PutByte ( m_tSettings.m_eChineseRLP ); // v. 39+
	tWriter.PutString ( m_tSettings.m_sRLPContext ); // v. 39+

	// tokenizer
	SaveTokenizerSettings ( tWriter, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );

	// dictionary
	SaveDictionarySettings ( tWriter, m_pDict, m_bKeywordDict, 0 );

	// kill-list size
	tWriter.PutDword ( uKillListSize );

	// min-max count
	tWriter.PutOffset ( uMinMaxSize );

	// field filter
	SaveFieldFilterSettings ( tWriter, m_pFieldFilter );

	// field lengths
	if ( m_tSettings.m_bIndexFieldLens )
		ARRAY_FOREACH ( i, m_tSchema.m_dFields )
			tWriter.PutOffset ( tStats.m_dFieldLens[i] );

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

	// meta v.11
	SaveFieldFilterSettings ( wrMeta, m_pFieldFilter );

	wrMeta.CloseFile(); // FIXME? handle errors?

	// rename
	if ( ::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMetaNew.cstr(), sMeta.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully
}


void RtIndex_t::SaveDiskChunk ( int64_t iTID, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats )
{
	if ( !tGuard.m_dRamChunks.GetLength() )
		return;

	MEMORY ( MEM_INDEX_RT );

	// dump it
	CSphString sNewChunk;
	sNewChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), tGuard.m_dDiskChunks.GetLength()+m_iDiskBase );
	SaveDiskDataImpl ( sNewChunk.cstr(), tGuard, tStats );

	// bring new disk chunk online
	CSphIndex * pDiskChunk = LoadDiskChunk ( sNewChunk.cstr(), m_sLastError );
	if ( !pDiskChunk )
		sphDie ( "%s", m_sLastError.cstr() );

	// FIXME! add binlog cleanup here once we have binlogs

	// get exclusive lock again, gotta reset RAM chunk now
	Verify ( m_tWriting.Lock() );
	Verify ( m_tChunkLock.WriteLock() );

	// save updated meta
	SaveMeta ( tGuard.m_dDiskChunks.GetLength()+1, iTID );
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// swap double buffer data
	int iNewSegmentsCount = ( m_iDoubleBuffer ? m_dRamChunks.GetLength() - m_iDoubleBuffer : 0 );
	for ( int i=0; i<iNewSegmentsCount; i++ )
		m_dRamChunks[i] = m_dRamChunks[i+m_iDoubleBuffer];
	m_dRamChunks.Resize ( iNewSegmentsCount );

	m_dDiskChunks.Add ( pDiskChunk );

	// update field lengths
	if ( m_tSchema.GetAttrId_FirstFieldLen()>=0 )
	{
		ARRAY_FOREACH ( i, m_dFieldLensRam )
			m_dFieldLensRam[i] -= tStats.m_dFieldLens[i];
		ARRAY_FOREACH ( i, m_dFieldLensDisk )
			m_dFieldLensDisk[i] += tStats.m_dFieldLens[i];
	}

	// move up kill-list
	m_tKlist.Reset ( m_dNewSegmentKlist.Begin(), m_dNewSegmentKlist.GetLength() );
	m_dNewSegmentKlist.Reset();
	m_dDiskChunkKlist.Reset();

	Verify ( m_tChunkLock.Unlock() );

	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		m_dRetired.Add ( tGuard.m_dRamChunks[i] );

	// abandon .ram file
	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	if ( sphIsReadable ( sChunk.cstr() ) && ::unlink ( sChunk.cstr() ) )
		sphWarning ( "failed to unlink ram chunk (file=%s, errno=%d, error=%s)", sChunk.cstr(), errno, strerror(errno) );

	FreeRetired();

	m_iDoubleBuffer = 0;
	m_iSavedTID = iTID;
	m_tmSaved = sphMicroTimer();

	Verify ( m_tWriting.Unlock() );
}


CSphIndex * RtIndex_t::LoadDiskChunk ( const char * sChunk, CSphString & sError ) const
{
	MEMORY ( MEM_INDEX_DISK );

	// !COMMIT handle errors gracefully instead of dying
	CSphIndex * pDiskChunk = sphCreateIndexPhrase ( sChunk, sChunk );
	if ( !pDiskChunk )
	{
		sError.SetSprintf ( "disk chunk %s: alloc failed", sChunk );
		return NULL;
	}

	pDiskChunk->m_iExpansionLimit = m_iExpansionLimit;
	pDiskChunk->m_bExpandKeywords = m_bExpandKeywords;
	pDiskChunk->SetBinlog ( false );
	pDiskChunk->SetMemorySettings ( m_bMlock, m_bOndiskAllAttr, m_bOndiskPoolAttr );

	if ( !pDiskChunk->Prealloc ( m_bPathStripped ) )
	{
		sError.SetSprintf ( "disk chunk %s: prealloc failed: %s", sChunk, pDiskChunk->GetLastError().cstr() );
		SafeDelete ( pDiskChunk );
		return NULL;
	}
	pDiskChunk->Preread();

	return pDiskChunk;
}


bool RtIndex_t::Prealloc ( bool bStripPath )
{
	MEMORY ( MEM_INDEX_RT );

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
	CSphTokenizerSettings tTokenizerSettings;
	if ( uVersion>=4 )
	{
		CSphDictSettings tDictSettings;
		CSphEmbeddedFiles tEmbeddedFiles;
		CSphString sWarning;

		// load them settings
		DWORD uSettingsVer = rdMeta.GetDword();
		ReadSchema ( rdMeta, m_tSchema, uSettingsVer, false );
		LoadIndexSettings ( m_tSettings, rdMeta, uSettingsVer );
		if ( !LoadTokenizerSettings ( rdMeta, tTokenizerSettings, tEmbeddedFiles, uSettingsVer, m_sLastError ) )
			return false;
		LoadDictionarySettings ( rdMeta, tDictSettings, tEmbeddedFiles, uSettingsVer, sWarning );

		// meta v.5 dictionary
		if ( uVersion>=5 )
			m_bKeywordDict = tDictSettings.m_bWordDict;

		// initialize AOT if needed
		DWORD uPrevAot = m_tSettings.m_uAotFilterMask;
		m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );
		if ( m_tSettings.m_uAotFilterMask!=uPrevAot )
			sphWarning ( "index '%s': morphology option changed from config has no effect, ignoring", m_sIndexName.cstr() );

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

		// recreate dictionary
		SafeDelete ( m_pDict );
		m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, m_sIndexName.cstr(), m_sLastError );
		if ( !m_pDict )
		{
			m_sLastError.SetSprintf ( "index '%s': %s", m_sIndexName.cstr(), m_sLastError.cstr() );
			return false;
		}

		m_pTokenizer = ISphTokenizer::CreateMultiformFilter ( m_pTokenizer, m_pDict->GetMultiWordforms () );

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

	if ( uVersion>=11 )
	{
		ISphFieldFilter * pFieldFilter = NULL;
		CSphFieldFilterSettings tFieldFilterSettings;
		LoadFieldFilterSettings ( rdMeta, tFieldFilterSettings );
		if ( tFieldFilterSettings.m_dRegexps.GetLength() )
			pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

		if ( !sphSpawnRLPFilter ( pFieldFilter, m_tSettings, tTokenizerSettings, sMeta.cstr(), m_sLastError ) )
		{
			SafeDelete ( pFieldFilter );
			return false;
		}

		SetFieldFilter ( pFieldFilter );
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

		m_dDiskChunks.Add ( pIndex );

		// tricky bit
		// outgoing match schema on disk chunk should be identical to our internal (!) schema
		if ( !m_tSchema.CompareTo ( pIndex->GetMatchSchema(), m_sLastError ) )
			return false;

		// update field lengths
		if ( m_tSchema.GetAttrId_FirstFieldLen()>=0 )
		{
			int64_t * pLens = pIndex->GetFieldLens();
			if ( pLens )
				ARRAY_FOREACH ( i, pIndex->GetMatchSchema().m_dFields )
					m_dFieldLensDisk[i] += pLens[i];
		}
	}

	// load ram chunk
	bool bRamLoaded = LoadRamChunk ( uVersion, bRebuildInfixes );

	// field lengths
	ARRAY_FOREACH ( i, m_dFieldLens )
		m_dFieldLens[i] = m_dFieldLensDisk[i] + m_dFieldLensRam[i];

	// set up values for on timer save
	m_iSavedTID = m_iTID;
	m_tmSaved = sphMicroTimer();

	return bRamLoaded;
}


void RtIndex_t::Preread ()
{
	// !COMMIT move disk chunks prereading here
}


void RtIndex_t::SetMemorySettings ( bool bMlock, bool bOndiskAttrs, bool bOndiskPool )
{
	m_bMlock = bMlock;
	m_bOndiskAllAttr = bOndiskAttrs;
	m_bOndiskPoolAttr = ( bOndiskAttrs || bOndiskPool );
}


static bool CheckVectorLength ( int iLen, int64_t iSaneLen, const char * sAt, CSphString & sError )
{
	if ( iLen>=0 && iLen<iSaneLen )
		return true;

	sError.SetSprintf ( "broken index, %s length overflow (len=%d, max="INT64_FMT")", sAt, iLen, iSaneLen );
	return false;
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
static bool LoadVector ( CSphReader & tReader, CSphVector < T, P > & tVector, int64_t iSaneLen, const char * sAt, CSphString & sError )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	int iSize = tReader.GetDword();
	if ( !CheckVectorLength ( iSize, iSaneLen, sAt, sError ) )
		return false;

	tVector.Resize ( iSize );
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );

	return true;
}


template < typename T, typename P >
static void SaveVector ( BinlogWriter_c & tWriter, const CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.ZipOffset ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static bool LoadVector ( BinlogReader_c & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( (int) tReader.UnzipOffset() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
	return !tReader.GetErrorFlag();
}


bool RtIndex_t::SaveRamChunk ()
{
	MEMORY ( MEM_INDEX_RT );

	CSphString sChunk, sNewChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	sNewChunk.SetSprintf ( "%s.ram.new", m_sPath.cstr() );
	m_tKlist.SaveToFile ( m_sPath.cstr() );

	CSphWriter wrChunk;
	if ( !wrChunk.OpenFile ( sNewChunk, m_sLastError ) )
		return false;

	wrChunk.PutDword ( USE_64BIT );
	RtSegment_t::m_tSegmentSeq.Lock();
	wrChunk.PutDword ( RtSegment_t::m_iSegments );
	RtSegment_t::m_tSegmentSeq.Unlock();
	wrChunk.PutDword ( m_dRamChunks.GetLength() );

	// no locks here, because it's only intended to be called from dtor
	ARRAY_FOREACH ( iSeg, m_dRamChunks )
	{
		const RtSegment_t * pSeg = m_dRamChunks[iSeg];
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
				wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_uWordID );
			}
		}
		SaveVector ( wrChunk, pSeg->m_dDocs );
		SaveVector ( wrChunk, pSeg->m_dHits );
		wrChunk.PutDword ( pSeg->m_iRows );
		wrChunk.PutDword ( pSeg->m_iAliveRows );
		SaveVector ( wrChunk, pSeg->m_dRows );

		wrChunk.PutDword ( pSeg->GetKlist().GetLength() );
		if ( pSeg->GetKlist().GetLength() )
			wrChunk.PutBytes ( pSeg->GetKlist().Begin(), sizeof(pSeg->GetKlist()[0]) * pSeg->GetKlist().GetLength() );

		SaveVector ( wrChunk, pSeg->m_dStrings );
		SaveVector ( wrChunk, pSeg->m_dMvas );

		// infixes
		SaveVector ( wrChunk, pSeg->m_dInfixFilterCP );
	}

	// field lengths
	wrChunk.PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		wrChunk.PutOffset ( m_dFieldLensRam[i] );

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
	MEMORY ( MEM_INDEX_RT );

	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );

	if ( !sphIsReadable ( sChunk.cstr(), &m_sLastError ) )
		return true;

	m_bLoadRamPassedOk = false;
	m_tKlist.LoadFromFile ( m_sPath.cstr() );

	CSphAutoreader rdChunk;
	if ( !rdChunk.Open ( sChunk, m_sLastError ) )
		return false;

	bool bId64 = ( rdChunk.GetDword()!=0 );
	if ( bId64!=USE_64BIT )
	{
		m_sLastError.SetSprintf ( "ram chunk dumped by %s binary; this binary is %s",
			bId64 ? "id64" : "id32",
			USE_64BIT ? "id64" : "id32" );
		return false;
	}

	int64_t iFileSize = rdChunk.GetFilesize();
	int64_t iSaneVecSize = Min ( iFileSize, INT_MAX / 2 );
	int64_t iSaneTightVecSize = Min ( iFileSize, int( INT_MAX / 1.2f ) );

	bool bHasMorphology = ( m_pDict && m_pDict->HasMorphology() ); // fresh and old-format index still has no dictionary at this point
	int iSegmentSeq = rdChunk.GetDword();

	int iSegmentCount = rdChunk.GetDword();
	if ( !CheckVectorLength ( iSegmentCount, iSaneVecSize, "ram-chunks", m_sLastError ) )
		return false;
	m_dRamChunks.Resize ( iSegmentCount );
	m_dRamChunks.Fill ( NULL );

	ARRAY_FOREACH ( iSeg, m_dRamChunks )
	{
		RtSegment_t * pSeg = new RtSegment_t ();
		m_dRamChunks[iSeg] = pSeg;

		pSeg->m_iTag = rdChunk.GetDword ();
		if ( !LoadVector ( rdChunk, pSeg->m_dWords, iSaneTightVecSize, "ram-words", m_sLastError ) )
			return false;

		if ( uVersion>=5 && m_bKeywordDict && !LoadVector ( rdChunk, pSeg->m_dKeywordCheckpoints, iSaneVecSize, "ram-checkpoints", m_sLastError ) )
				return false;

		const char * pCheckpoints = (const char *)pSeg->m_dKeywordCheckpoints.Begin();

		int iCheckpointCount = rdChunk.GetDword();
		if ( !CheckVectorLength ( iCheckpointCount, iSaneVecSize, "ram-checkpoints", m_sLastError ) )
			return false;

		pSeg->m_dWordCheckpoints.Resize ( iCheckpointCount );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int)rdChunk.GetOffset();
			SphOffset_t uOff = rdChunk.GetOffset();
			if ( m_bKeywordDict )
			{
				pSeg->m_dWordCheckpoints[i].m_sWord = pCheckpoints + uOff;
			} else
			{
				pSeg->m_dWordCheckpoints[i].m_uWordID = (SphWordID_t)uOff;
			}
		}
		if ( !LoadVector ( rdChunk, pSeg->m_dDocs, iSaneTightVecSize, "ram-doclist", m_sLastError ) )
			return false;

		if ( !LoadVector ( rdChunk, pSeg->m_dHits, iSaneTightVecSize, "ram-hitlist", m_sLastError ) )
			return false;

		pSeg->m_iRows = rdChunk.GetDword();
		pSeg->m_iAliveRows = rdChunk.GetDword();
		// warning! m_dRows saved in id32 is NOT consistent for id64!
		// (the Stride for id32 is 1 DWORD shorter than for id64)
		// the only usage of this BLOB is to save id32 disk-chunk.
		if ( !LoadVector ( rdChunk, pSeg->m_dRows, iSaneTightVecSize, "ram-attributes", m_sLastError ) )
			return false;

		if ( uVersion>=9 )
		{
			int iLen = rdChunk.GetDword();
			if ( !CheckVectorLength ( iLen, Min ( iFileSize, INT_MAX ), "ram-killlist", m_sLastError ) )
				return false;

			if ( iLen )
			{
				pSeg->m_pKlist->m_dKilled.Reset ( iLen );
				rdChunk.GetBytes ( pSeg->m_pKlist->m_dKilled.Begin(), sizeof(pSeg->m_pKlist->m_dKilled[0]) * iLen );
			}
		} else
		{
			// legacy path - all types of kill-list loading either with conversion or not
			if ( uVersion==8 )
				rdChunk.GetDword(); // Hash.used

			int iLen = rdChunk.GetDword();
			if ( !CheckVectorLength ( iLen, iSaneVecSize, "ram-killlist", m_sLastError ) )
				return false;

			CSphVector<SphDocID_t> dLegacy;
			for ( int i=0; i<iLen; i++ )
			{
				SphDocID_t uDocid = USE_64BIT ? rdChunk.GetDword() : rdChunk.GetOffset();
				if ( uDocid ) // hash might have 0
					dLegacy.Add ( uDocid );
			}

			if ( dLegacy.GetLength() )
			{
				dLegacy.Uniq(); // hash was unordered
				pSeg->m_pKlist->m_dKilled.Reset ( dLegacy.GetLength() );
				memcpy ( pSeg->m_pKlist->m_dKilled.Begin(), dLegacy.Begin(), sizeof(pSeg->m_pKlist->m_dKilled[0]) * dLegacy.GetLength() );
			}
		}

		if ( !LoadVector ( rdChunk, pSeg->m_dStrings, iSaneTightVecSize, "ram-strings", m_sLastError ) )
			return false;
		if ( uVersion>=3 && !LoadVector ( rdChunk, pSeg->m_dMvas, iSaneTightVecSize, "ram-mva", m_sLastError ) )
			return false;

		// infixes
		if ( uVersion>=7 )
		{
			if ( !LoadVector ( rdChunk, pSeg->m_dInfixFilterCP, iSaneTightVecSize, "ram-infixes", m_sLastError ) )
				return false;
			if ( bRebuildInfixes )
				BuildSegmentInfixes ( pSeg, bHasMorphology );
		}
	}

	// field lengths
	if ( uVersion>=10 )
	{
		int iFields = rdChunk.GetDword();
		assert ( iFields==m_tSchema.m_dFields.GetLength() );

		for ( int i=0; i<iFields; i++ )
			m_dFieldLensRam[i] = rdChunk.GetOffset();
	}

	// all done
	RtSegment_t::m_tSegmentSeq.Lock();
	RtSegment_t::m_iSegments = iSegmentSeq;
	RtSegment_t::m_tSegmentSeq.Unlock();
	if ( rdChunk.GetErrorFlag() )
		return false;
	m_bLoadRamPassedOk = true;
	return true;
}


void RtIndex_t::Setup ( const CSphIndexSettings & tSettings )
{
	m_bStripperInited = true;
	m_tSettings = tSettings;
}


void RtIndex_t::PostSetup()
{
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
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
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

	ARRAY_FOREACH ( iSegment, m_dRamChunks )
	{
		fprintf ( fp, "checking RT segment %d(%d)...\n", iSegment, m_dRamChunks.GetLength() );

		if ( !m_dRamChunks[iSegment] )
		{
			LOC_FAIL(( fp, "missing RT segment (segment=%d)", iSegment ));
			continue;
		}

		RtSegment_t & tSegment = *m_dRamChunks[iSegment];
		if ( tSegment.m_bTlsKlist )
			LOC_FAIL(( fp, "TLS k-list flag on: index is being commited (segment=%d)", iSegment ));

		const BYTE * pCurWord = tSegment.m_dWords.Begin();
		const BYTE * pMaxWord = pCurWord+tSegment.m_dWords.GetLength();
		const BYTE * pCurDoc = tSegment.m_dDocs.Begin();
		const BYTE * pMaxDoc = pCurDoc+tSegment.m_dDocs.GetLength();
		const BYTE * pCurHit = tSegment.m_dHits.Begin();
		const BYTE * pMaxHit = pCurHit+tSegment.m_dHits.GetLength();

		CSphVector<RtWordCheckpoint_t> dRefCheckpoints;
		int nWordsRead = 0;
		int nCheckpointWords = 0;
		int iCheckpointOffset = 0;
		SphWordID_t uPrevWordID = 0;
		DWORD uPrevDocOffset = 0;
		DWORD uPrevHitOffset = 0;

		CSphVector<SphDocID_t> dUsedKListEntries;

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
					tCP.m_uWordID = tWord.m_uWordID;
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
				pIn = pCurDoc;
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

				if ( tSegment.GetKlist().BinarySearch ( tDoc.m_uDocID ) )
					dUsedKListEntries.Add ( tDoc.m_uDocID );

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
			} else if ( sphCheckpointCmpStrictly ( tCP.m_sWord, iLen, tCP.m_uWordID, m_bKeywordDict, tRefCP ) || tRefCP.m_iOffset!=tCP.m_iOffset )
			{
				if ( m_bKeywordDict )
				{
					LOC_FAIL(( fp, "word checkpoint %d differs (segment=%d, read_word=%s, readpos=%d, calc_word=%s, calcpos=%d)",
						i, iSegment, tCP.m_sWord, tCP.m_iOffset, tRefCP.m_sWord, tRefCP.m_iOffset ));
				} else
				{
					LOC_FAIL(( fp, "word checkpoint %d differs (segment=%d, readid="UINT64_FMT", readpos=%d, calcid="UINT64_FMT", calcpos=%d)",
						i, iSegment, (uint64_t)tCP.m_uWordID, tCP.m_iOffset, (int64_t)tRefCP.m_uWordID, tRefCP.m_iOffset ));
				}
			}
		}

		if ( m_bKeywordDict )
			ARRAY_FOREACH ( i, dRefCheckpoints )
				SafeDeleteArray ( dRefCheckpoints[i].m_sWord );

		dRefCheckpoints.Reset ();

		// check killlists
		dUsedKListEntries.Uniq();
		int nUsedKListEntries = dUsedKListEntries.GetLength();

		if ( nUsedKListEntries!=tSegment.GetKlist().GetLength() )
		{
			LOC_FAIL(( fp, "used killlist entries mismatch (segment=%d, klist_entries=%d, used_entries=%d)",
				iSegment, tSegment.GetKlist().GetLength(), nUsedKListEntries ));
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
		CSphVector<CSphAttrLocator> dJsonItems;
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
			else if ( tAttr.m_eAttrType==SPH_ATTR_JSON )
				dJsonItems.Add ( tAttr.m_tLocator );
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
		int nUsedJsons = 0;

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
						int64_t iPrev, iCur;
						if ( iItem>=iMva64 )
						{
							iPrev = MVA_UPSIZE ( pMvaCur+uVal-2 );
							iCur = MVA_UPSIZE ( pMvaCur+uVal );
							uVal += 2;
						} else
						{
							iPrev = pMvaCur[uVal-1];
							iCur = pMvaCur[uVal];
							uVal++;
						}

						if ( iCur<=iPrev )
						{
							LOC_FAIL(( fp, "unsorted MVA values (segment=%d, row=%u, mvaattr=%d, docid="DOCID_FMT", val[%u]="INT64_FMT", val[%u]="INT64_FMT")",
								iSegment, uRow, iItem, uLastID, ( iItem>=iMva64 ? uVal-2 : uVal-1 ), iPrev, uVal, iCur ));
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

				bool bLastOff4UpdatedJson = ( iItem==0 && dJsonItems.GetLength () );
				if ( uLastStrOffset>=uOffset && !bLastOff4UpdatedJson )
					LOC_FAIL(( fp, "string offset decreased (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u, last_offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset, uLastStrOffset ));

				if ( !dStringOffsets.BinarySearch ( uOffset ) )
				{
					LOC_FAIL(( fp, "string offset is not a string start (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset ));
				} else
					nUsedStrings++;

				const BYTE * pStr = NULL;
				int iLen = sphUnpackStr ( tSegment.m_dStrings.Begin()+uOffset, &pStr );
				const BYTE * pStringStart = pStr;
				while ( pStringStart-pStr < iLen )
				{
					if ( !*pStringStart )
					{
						CSphString sErrorStr;
						sErrorStr.SetBinary ( (const char*)pStr, iLen );
						LOC_FAIL(( fp, "embedded zero in a string (segment=%d, offset=%u, string=%s)",
									iSegment, uOffset, sErrorStr.cstr() ));
					}

					pStringStart++;
				}

				uLastStrOffset = uOffset;
			}

			/////////////////////////////
			// check JSON attributes
			/////////////////////////////

			ARRAY_FOREACH ( iItem, dJsonItems )
			{
				const CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);

				const DWORD uOffset = (DWORD)sphGetRowAttr ( pAttrs, dJsonItems[iItem] );
				if ( uOffset>=(DWORD)tSegment.m_dStrings.GetLength() )
				{
					LOC_FAIL(( fp, "string(JSON) offset out of bounds (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", index=%u)",
						iSegment, uRow, iItem, uLastID, uOffset ));
					continue;
				}

				if ( !uOffset )
					continue;

				if ( uLastStrOffset>=uOffset )
					LOC_FAIL(( fp, "string(JSON) offset decreased (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u, last_offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset, uLastStrOffset ));

				if ( !dStringOffsets.BinarySearch ( uOffset ) )
				{
					LOC_FAIL(( fp, "string(JSON) offset is not a string start (segment=%d, row=%u, stringattr=%d, docid="DOCID_FMT", offset=%u)",
						iSegment, uRow, iItem, uLastID, uOffset ));
				} else
					nUsedJsons++;

				const BYTE * pData = NULL;
				int iBlobLen = sphUnpackStr ( tSegment.m_dStrings.Begin()+uOffset, &pData );

				const BYTE * p = pData+4;
				CSphVector<ESphJsonType> dStateStack;
				if ( pData[0] | pData[1]<<8 | pData[2]<<16 | pData[3]<<24 )
					dStateStack.Add ( JSON_OBJECT );

				do
				{
					ESphJsonType eType = (ESphJsonType)*p++;

					if ( dStateStack.GetLength() && dStateStack.Last()==JSON_OBJECT && eType!=JSON_EOF )
					{
						int iKeyLen = sphJsonUnpackInt ( &p );
						p += iKeyLen;
					}

					if ( dStateStack.GetLength() && dStateStack.Last()==JSON_MIXED_VECTOR )
						dStateStack.Pop();

					switch ( eType )
					{
					case JSON_EOF:
					{
						if ( dStateStack.GetLength() && dStateStack.Last()==JSON_OBJECT )
							dStateStack.Pop();
						break;
					}

					case JSON_INT32:		sphJsonLoadInt ( &p ); break;
					case JSON_INT64:		sphJsonLoadBigint ( &p ); break;
					case JSON_DOUBLE:		sphJsonLoadBigint ( &p ); break;
					case JSON_TRUE:			break;
					case JSON_FALSE:		break;
					case JSON_NULL:			break;

					case JSON_STRING:
					{
						int iStrLen = sphJsonUnpackInt ( &p );
						p += iStrLen;
						break;
					}

					case JSON_OBJECT:
					{
						dStateStack.Add ( JSON_OBJECT );
						sphJsonUnpackInt ( &p );
						p += 4; // bloom mask
						break;
					}

					case JSON_MIXED_VECTOR:
					{
						sphJsonUnpackInt ( &p );
						for ( int iLen=sphJsonUnpackInt ( &p ); iLen; iLen-- )
							dStateStack.Add ( JSON_MIXED_VECTOR );
						break;
					}

					case JSON_STRING_VECTOR:
					{
						int iTotalLen = sphJsonUnpackInt ( &p );
						p += iTotalLen;
						break;
					}

					case JSON_INT32_VECTOR:
					{
						for ( int iLen=sphJsonUnpackInt ( &p ); iLen; iLen-- )
							sphJsonLoadInt ( &p );
						break;
					}

					case JSON_INT64_VECTOR:
					case JSON_DOUBLE_VECTOR:
					{
						for ( int iLen=sphJsonUnpackInt ( &p ); iLen; iLen-- )
							sphJsonLoadBigint ( &p );
						break;
					}

					default:
						LOC_FAIL(( fp, "incorrect type in JSON blob (type=%d", eType ));
						break;
					}
				} while ( p<( pData+iBlobLen ));

				if ( dStateStack.GetLength() )
					LOC_FAIL(( fp, "JSON blob nested arrays/objects mismatch"));

				if ( iBlobLen!=( p-pData ))
					LOC_FAIL(( fp, "JSON blob length mismatch (stored=%d, actual=%d)", iBlobLen, int( p-pData ) ));

				uLastStrOffset = uOffset;
			}

			nCalcRows++;
			if ( !tSegment.GetKlist().BinarySearch ( uLastID ) )
				nCalcAliveRows++;
		}

		if ( ( nUsedStrings+nUsedJsons )!=dStringOffsets.GetLength() )
			LOC_FAIL(( fp, "unused string/JSON entries found (segment=%d)", iSegment ));

		if ( dMvaItems.GetLength() && pMvaCur!=pMvaMax )
			LOC_FAIL(( fp, "unused MVA entries found (segment=%d)", iSegment ));

		if ( tSegment.m_iRows!=nCalcRows )
			LOC_FAIL(( fp, "row count mismatch (segment=%d, expected=%d, current=%d)",
				iSegment, nCalcRows, tSegment.m_iRows ));

		if ( tSegment.m_iAliveRows!=nCalcAliveRows )
			LOC_FAIL(( fp, "alive row count mismatch (segment=%d, expected=%d, current=%d)",
				iSegment, nCalcAliveRows, tSegment.m_iAliveRows ));
	}

	ARRAY_FOREACH ( i, m_dDiskChunks )
	{
		fprintf ( fp, "checking disk chunk %d(%d)...\n", i, m_dDiskChunks.GetLength() );
		iFailsPlain += m_dDiskChunks[i]->DebugCheck ( fp );
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

//////////////////////////////////////////////////////////////////////////
// SEARCHING
//////////////////////////////////////////////////////////////////////////


struct RtQwordTraits_t : public ISphQword
{
public:
	virtual bool Setup ( const RtIndex_t * pIndex, int iSegment, const SphChunkGuard_t & tGuard ) = 0;
};


//////////////////////////////////////////////////////////////////////////

struct RtQword_t : public RtQwordTraits_t
{
public:
	RtQword_t ()
		: m_uNextHit ( 0 )
		, m_pKill ( NULL )
		, m_pKillEnd ( NULL )
	{
		m_tMatch.Reset ( 0 );
	}

	virtual ~RtQword_t ()
	{
	}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		for ( ;; )
		{
			const RtDoc_t * pDoc = m_tDocReader.UnzipDoc();
			if ( !pDoc )
			{
				m_tMatch.m_uDocID = 0;
				return m_tMatch;
			}

			if ( sphBinarySearch ( m_pKill, m_pKillEnd, pDoc->m_uDocID )!=NULL )
				continue;

			m_tMatch.m_uDocID = pDoc->m_uDocID;
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

	virtual bool Setup ( const RtIndex_t * pIndex, int iSegment, const SphChunkGuard_t & tGuard )
	{
		return pIndex->RtQwordSetup ( this, iSegment, tGuard );
	}

	void SetupReader ( const RtSegment_t * pSeg, const RtWord_t & tWord, const CSphFixedVector<SphDocID_t> & dKill )
	{
		m_tDocReader = RtDocReader_t ( pSeg, tWord );
		m_tHitReader.m_pBase = pSeg->m_dHits.Begin();
		m_pKill = m_pKillEnd = NULL;
		if ( dKill.GetLength()>0 )
		{
			m_pKill = dKill.Begin();
			m_pKillEnd = m_pKill + dKill.GetLength() - 1;
		}
	}

private:
	RtDocReader_t		m_tDocReader;
	CSphMatch			m_tMatch;

	DWORD				m_uNextHit;
	RtHitReader2_t		m_tHitReader;

	const SphDocID_t *	m_pKill;
	const SphDocID_t *	m_pKillEnd;
};


//////////////////////////////////////////////////////////////////////////


struct RtSubstringPayload_t : public ISphSubstringPayload
{
	RtSubstringPayload_t ( int iSegmentCount, int iDoclists )
		: m_dSegment2Doclists ( iSegmentCount )
		, m_dDoclist ( iDoclists )
		, m_iDocsTotal ( 0 )
		, m_iHitsTotal ( 0 )
	{}
	CSphFixedVector<Slice_t>	m_dSegment2Doclists;
	CSphFixedVector<Slice_t>	m_dDoclist;
	int							m_iDocsTotal;
	int							m_iHitsTotal;
};


struct RtQwordPayload_t : public RtQwordTraits_t
{
public:
	explicit RtQwordPayload_t ( const RtSubstringPayload_t * pPayload )
		: m_pPayload ( pPayload )
	{
		m_tMatch.Reset ( 0 );
		m_iDocs = m_pPayload->m_iDocsTotal;
		m_iHits = m_pPayload->m_iHitsTotal;

		m_uDoclist = 0;
		m_uDoclistLeft = 0;
		m_pSegment = NULL;
		m_uHitEmbeded = EMPTY_HIT;
		m_pKill = m_pKillEnd = NULL;
	}

	virtual ~RtQwordPayload_t ()
	{}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		m_iHits = 0;
		for ( ;; )
		{
			const RtDoc_t * pDoc = m_tDocReader.UnzipDoc();
			if ( !pDoc && !m_uDoclistLeft )
			{
				m_tMatch.m_uDocID = 0;
				return m_tMatch;
			}

			if ( !pDoc && m_uDoclistLeft )
			{
				SetupReader();
				pDoc = m_tDocReader.UnzipDoc();
				assert ( pDoc );
			}

			if ( sphBinarySearch ( m_pKill, m_pKillEnd, pDoc->m_uDocID )!=NULL )
				continue;

			m_tMatch.m_uDocID = pDoc->m_uDocID;
			m_dQwordFields.Assign32 ( pDoc->m_uDocFields );
			m_bAllFieldsKnown = false;

			m_iHits = pDoc->m_uHits;
			m_uHitEmbeded = pDoc->m_uHit;
			m_tHitReader = RtHitReader_t ( m_pSegment, pDoc );

			return m_tMatch;
		}
	}

	virtual void SeekHitlist ( SphOffset_t )
	{}

	virtual Hitpos_t GetNextHit ()
	{
		if ( m_iHits>1 )
			return Hitpos_t ( m_tHitReader.UnzipHit() );
		else if ( m_iHits==1 )
		{
			Hitpos_t tHit ( m_uHitEmbeded );
			m_uHitEmbeded = EMPTY_HIT;
			return tHit;
		} else
		{
			return EMPTY_HIT;
		}
	}

	virtual bool Setup ( const RtIndex_t *, int iSegment, const SphChunkGuard_t & tGuard )
	{
		m_uDoclist = 0;
		m_uDoclistLeft = 0;
		m_tDocReader = RtDocReader_t();
		m_pSegment = NULL;
		m_pKill = m_pKillEnd = NULL;

		if ( iSegment<0 )
			return false;

		m_pSegment = tGuard.m_dRamChunks[iSegment];
		const KlistRefcounted_t * pKill = tGuard.m_dKill[iSegment];
		if ( pKill->m_dKilled.GetLength() )
		{
			m_pKill = pKill->m_dKilled.Begin();
			m_pKillEnd = m_pKill + pKill->m_dKilled.GetLength() - 1;
		}

		m_uDoclist = m_pPayload->m_dSegment2Doclists[iSegment].m_uOff;
		m_uDoclistLeft = m_pPayload->m_dSegment2Doclists[iSegment].m_uLen;

		if ( !m_uDoclistLeft )
			return false;

		SetupReader();
		return true;
	}

private:
	void SetupReader ()
	{
		assert ( m_uDoclistLeft );
		RtWord_t tWord;
		tWord.m_uDoc = m_pPayload->m_dDoclist[m_uDoclist].m_uOff;
		tWord.m_uDocs = m_pPayload->m_dDoclist[m_uDoclist].m_uLen;
		m_tDocReader = RtDocReader_t ( m_pSegment, tWord );
		m_uDoclist++;
		m_uDoclistLeft--;
	}

	const RtSubstringPayload_t *	m_pPayload;
	CSphMatch					m_tMatch;
	RtDocReader_t				m_tDocReader;
	RtHitReader_t				m_tHitReader;
	const RtSegment_t *			m_pSegment;
	const SphDocID_t *			m_pKill;
	const SphDocID_t *			m_pKillEnd;

	DWORD						m_uDoclist;
	DWORD						m_uDoclistLeft;
	DWORD						m_uHitEmbeded;
};


//////////////////////////////////////////////////////////////////////////

class RtQwordSetup_t : public ISphQwordSetup
{
public:
	explicit RtQwordSetup_t ( const SphChunkGuard_t & tGuard );
	virtual ISphQword *	QwordSpawn ( const XQKeyword_t & ) const;
	virtual bool		QwordSetup ( ISphQword * pQword ) const;
	void				SetSegment ( int iSegment )
	{
		m_iSeg = iSegment;
	}

private:
	const SphChunkGuard_t & m_tGuard;
	int					m_iSeg;
};


RtQwordSetup_t::RtQwordSetup_t ( const SphChunkGuard_t & tGuard )
	: m_tGuard ( tGuard )
	, m_iSeg ( -1 )
{ }


ISphQword * RtQwordSetup_t::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( !tWord.m_pPayload )
		return new RtQword_t ();
	else
		return new RtQwordPayload_t ( (const RtSubstringPayload_t *)tWord.m_pPayload );
}


bool RtQwordSetup_t::QwordSetup ( ISphQword * pQword ) const
{
	// there was two dynamic_casts here once but they're not necessary
	// maybe it's worth to rewrite class hierarchy to avoid c-casts here?
	RtQwordTraits_t * pMyWord = (RtQwordTraits_t*)pQword;
	const RtIndex_t * pIndex = (const RtIndex_t *)m_pIndex;
	return pMyWord->Setup ( pIndex, m_iSeg, m_tGuard );
}


bool RtIndex_t::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	// might be needed even when we do not have a filter!
	if ( pCtx->m_bLookupFilter || pCtx->m_bLookupSort )
	{
		const CSphRowitem * pRow = FindDocinfo ( (RtSegment_t*)pCtx->m_pIndexData, tMatch.m_uDocID );
		if ( !pRow )
		{
			pCtx->m_iBadRows++;
			return true;
		}
		CopyDocinfo ( tMatch, pRow );
	}

	pCtx->CalcFilter ( tMatch ); // FIXME!!! leak of filtered STRING_PTR
	return pCtx->m_pFilter ? !pCtx->m_pFilter->Eval ( tMatch ) : false;
}


void RtIndex_t::CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const
{
	if ( !pFound )
		return;

	// setup static pointer
	assert ( DOCINFO2ID(pFound)==tMatch.m_uDocID );
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
bool RtIndex_t::RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pCurSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint, const CSphFixedVector<SphDocID_t> & dKill, const CSphIndexSettings & tSettings )
{
	if ( !pCurSeg )
		return false;

	SphWordID_t uWordID = pQword->m_uWordID;
	const char * sWord = pQword->m_sDictWord.cstr();
	int iWordLen = pQword->m_sDictWord.Length();
	bool bPrefix = false;
	if ( bWordDict && iWordLen && sWord[iWordLen-1]=='*' ) // crc star search emulation
	{
		iWordLen = iWordLen-1;
		bPrefix = true;
	}

	if ( !iWordLen )
		return false;

	// prevent prefix matching for explicitly setting prohibited by config, to be on pair with plain index (or CRC kind of index)
	if ( bPrefix && ( ( tSettings.m_iMinPrefixLen && iWordLen<tSettings.m_iMinPrefixLen ) || ( tSettings.m_iMinInfixLen && iWordLen<tSettings.m_iMinInfixLen ) ) )
		return false;

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
				pQword->SetupReader ( pCurSeg, *pWord, dKill );

			return true;

		} else if ( iCmp>0 )
			return false;
	}
	return false;
}

struct RtExpandedEntry_t
{
	DWORD	m_uHash;
	int		m_iNameOff;
	int		m_iDocs;
	int		m_iHits;
};

struct RtExpandedPayload_t
{
	int		m_iDocs;
	int		m_iHits;
	DWORD	m_uDoclistOff;
};

struct RtExpandedTraits_fn
{
	inline bool IsLess ( const RtExpandedEntry_t & a, const RtExpandedEntry_t & b ) const
	{
		assert ( m_sBase );
		if ( a.m_uHash!=b.m_uHash )
		{
			return a.m_uHash<b.m_uHash;
		} else
		{
			const BYTE * pA = m_sBase + a.m_iNameOff;
			const BYTE * pB = m_sBase + b.m_iNameOff;
			if ( pA[0]!=pB[0] )
				return pA[0]<pB[0];

			return ( sphDictCmp ( (const char *)pA+1, pA[0], (const char *)pB+1, pB[0] )<0 );
		}
	}

	inline bool IsEqual ( const RtExpandedEntry_t * a, const RtExpandedEntry_t * b ) const
	{
		assert ( m_sBase );
		if ( a->m_uHash!=b->m_uHash )
			return false;

		const BYTE * pA = m_sBase + a->m_iNameOff;
		const BYTE * pB = m_sBase + b->m_iNameOff;
		if ( pA[0]!=pB[0] )
			return false;

		return ( sphDictCmp ( (const char *)pA+1, pA[0], (const char *)pB+1, pB[0] )==0 );
	}

	explicit RtExpandedTraits_fn ( const BYTE * sBase )
		: m_sBase ( sBase )
	{ }
	const BYTE * m_sBase;
};


struct DictEntryRtPayload_t
{
	DictEntryRtPayload_t ( bool bPayload, int iSegments )
	{
		m_bPayload = bPayload;
		if ( bPayload )
		{
			m_dWordPayload.Reserve ( 1000 );
			m_dSeg.Resize ( iSegments );
			ARRAY_FOREACH ( i, m_dSeg )
			{
				m_dSeg[i].m_uOff = 0;
				m_dSeg[i].m_uLen = 0;
			}
		}

		m_dWordExpand.Reserve ( 1000 );
		m_dWordBuf.Reserve ( 8096 );
	}

	void Add ( const RtWord_t * pWord, int iSegment )
	{
		if ( !m_bPayload || !sphIsExpandedPayload ( pWord->m_uDocs, pWord->m_uHits ) )
		{
			RtExpandedEntry_t & tExpand = m_dWordExpand.Add();

			int iOff = m_dWordBuf.GetLength();
			int iWordLen = pWord->m_sWord[0] + 1;
			tExpand.m_uHash = sphCRC32 ( pWord->m_sWord, iWordLen );
			tExpand.m_iNameOff = iOff;
			tExpand.m_iDocs = pWord->m_uDocs;
			tExpand.m_iHits = pWord->m_uHits;
			m_dWordBuf.Resize ( iOff + iWordLen );
			memcpy ( m_dWordBuf.Begin() + iOff, pWord->m_sWord, iWordLen );
		} else
		{
			RtExpandedPayload_t & tExpand = m_dWordPayload.Add();
			tExpand.m_iDocs = pWord->m_uDocs;
			tExpand.m_iHits = pWord->m_uHits;
			tExpand.m_uDoclistOff = pWord->m_uDoc;

			m_dSeg[iSegment].m_uOff = m_dWordPayload.GetLength();
			m_dSeg[iSegment].m_uLen++;
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
			int iRtExpansionLimit = tArgs.m_iExpansionLimit * m_dSeg.GetLength();
			if ( tArgs.m_iExpansionLimit && m_dWordExpand.GetLength()>iRtExpansionLimit )
			{
				// sort expansions by frequency desc
				// clip the less frequent ones if needed, as they are likely misspellings
				sphSort ( m_dWordExpand.Begin(), m_dWordExpand.GetLength(), ExpandedOrderDesc_T<RtExpandedEntry_t>() );
				m_dWordExpand.Resize ( iRtExpansionLimit );
			}

			// lets merge statistics for same words from different segments as hash produce a lot tiny allocations here
			const BYTE * sBase = m_dWordBuf.Begin();
			RtExpandedTraits_fn fnCmp ( sBase );
			sphSort ( m_dWordExpand.Begin(), m_dWordExpand.GetLength(), fnCmp );

			const RtExpandedEntry_t * pLast = m_dWordExpand.Begin();
			tArgs.AddExpanded ( sBase+pLast->m_iNameOff+1, sBase[pLast->m_iNameOff], pLast->m_iDocs, pLast->m_iHits );
			for ( int i=1; i<m_dWordExpand.GetLength(); i++ )
			{
				const RtExpandedEntry_t * pCur = m_dWordExpand.Begin() + i;

				if ( fnCmp.IsEqual ( pLast, pCur ) )
				{
					tArgs.m_dExpanded.Last().m_iDocs += pCur->m_iDocs;
					tArgs.m_dExpanded.Last().m_iHits += pCur->m_iHits;
				} else
				{
					tArgs.AddExpanded ( sBase + pCur->m_iNameOff + 1, sBase[pCur->m_iNameOff], pCur->m_iDocs, pCur->m_iHits );
					pLast = pCur;
				}
				iTotalDocs += pCur->m_iDocs;
				iTotalHits += pCur->m_iHits;
			}
		}

		if ( m_dWordPayload.GetLength() )
		{
			DWORD uExpansionLimit = tArgs.m_iExpansionLimit;
			int iPayloads = 0;
			ARRAY_FOREACH ( i, m_dSeg )
			{
				Slice_t & tSeg = m_dSeg[i];

				// reverse per segment offset to payload doc-list as offset was the end instead of start
				assert ( tSeg.m_uOff>=tSeg.m_uLen );
				tSeg.m_uOff = tSeg.m_uOff - tSeg.m_uLen;

				// per segment expansion limit clip
				if ( uExpansionLimit && tSeg.m_uLen>uExpansionLimit )
				{
					// sort expansions by frequency desc
					// per segment clip the less frequent ones if needed, as they are likely misspellings
					sphSort ( m_dWordPayload.Begin()+tSeg.m_uOff, tSeg.m_uLen, ExpandedOrderDesc_T<RtExpandedPayload_t>() );
					tSeg.m_uLen = uExpansionLimit;
				}

				iPayloads += tSeg.m_uLen;
				// sort by ascending doc-list offset
				sphSort ( m_dWordPayload.Begin()+tSeg.m_uOff, tSeg.m_uLen, bind ( &RtExpandedPayload_t::m_uDoclistOff ) );
			}

			RtSubstringPayload_t * pPayload = new RtSubstringPayload_t ( m_dSeg.GetLength(), iPayloads );

			Slice_t * pDst = pPayload->m_dDoclist.Begin();
			ARRAY_FOREACH ( i, m_dSeg )
			{
				const Slice_t & tSeg = m_dSeg[i];
				const RtExpandedPayload_t * pSrc = m_dWordPayload.Begin() + tSeg.m_uOff;
				const RtExpandedPayload_t * pEnd = pSrc + tSeg.m_uLen;
				pPayload->m_dSegment2Doclists[i].m_uOff = pDst - pPayload->m_dDoclist.Begin();
				pPayload->m_dSegment2Doclists[i].m_uLen = tSeg.m_uLen;
				while ( pSrc!=pEnd )
				{
					pDst->m_uOff = pSrc->m_uDoclistOff;
					pDst->m_uLen = pSrc->m_iDocs;
					iTotalDocs += pSrc->m_iDocs;
					iTotalHits += pSrc->m_iHits;
					pDst++;
					pSrc++;
				}
			}
			pPayload->m_iDocsTotal = iTotalDocs;
			pPayload->m_iHitsTotal = iTotalHits;
			tArgs.m_pPayload = pPayload;
		}

		tArgs.m_iTotalDocs = iTotalDocs;
		tArgs.m_iTotalHits = iTotalHits;
	}

	bool							m_bPayload;
	CSphVector<RtExpandedEntry_t>	m_dWordExpand;
	CSphVector<RtExpandedPayload_t>	m_dWordPayload;
	CSphVector<BYTE>				m_dWordBuf;
	CSphVector<Slice_t>				m_dSeg;
};


void RtIndex_t::GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	const CSphFixedVector<RtSegment_t*> & dSegments = *((CSphFixedVector<RtSegment_t*> *)tArgs.m_pIndexData);
	DictEntryRtPayload_t tDict2Payload ( tArgs.m_bPayload, dSegments.GetLength() );
	const int iSkipMagic = ( BYTE(*sSubstring)<0x20 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	ARRAY_FOREACH ( iSeg, dSegments )
	{
		const RtSegment_t * pCurSeg = dSegments[iSeg];
		RtWordReader_t tReader ( pCurSeg, true, m_iWordsCheckpoint );

		// find initial checkpoint or check words prior to 1st checkpoint
		if ( pCurSeg->m_dWordCheckpoints.GetLength() )
		{
			const RtWordCheckpoint_t * pCurCheckpoint = sphSearchCheckpoint ( sSubstring, iSubLen, 0, true, true
				, pCurSeg->m_dWordCheckpoints.Begin(), &pCurSeg->m_dWordCheckpoints.Last() );

			if ( pCurCheckpoint )
			{
				// there could be valid data prior 1st checkpoint that should be unpacked and checked
				int iCheckpointNameLen = strlen ( pCurCheckpoint->m_sWord );
				if ( pCurCheckpoint!=pCurSeg->m_dWordCheckpoints.Begin()
					|| ( sphDictCmp ( sSubstring, iSubLen, pCurCheckpoint->m_sWord, iCheckpointNameLen )==0 && iSubLen==iCheckpointNameLen ) )
				{
					tReader.m_pCur = pCurSeg->m_dWords.Begin() + pCurCheckpoint->m_iOffset;
				}
			}
		}

		// find the word between checkpoints
		const RtWord_t * pWord = NULL;
		while ( ( pWord = tReader.UnzipWord() )!=NULL )
		{
			int iCmp = sphDictCmp ( sSubstring, iSubLen, (const char *)pWord->m_sWord+1, pWord->m_sWord[0] );
			if ( iCmp<0 )
			{
				break;
			} else if ( iCmp==0 && iSubLen<=pWord->m_sWord[0] && sphWildcardMatch ( (const char *)pWord->m_sWord+1+iSkipMagic, sWildcard ) )
			{
				tDict2Payload.Add ( pWord, iSeg );
			}
			// FIXME!!! same case 'boxi*' matches 'box' document at plain index
			// but masked by a checkpoint search
		}
	}

	tDict2Payload.Convert ( tArgs );
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
	for ( int i=0; i<iHashes; i++ )
		iMatched += ( dMatches[i]>0 );

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


void RtIndex_t::GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	// sanity checks
	if ( !sSubstring || iSubLen<=0 )
		return;

	// find those prefixes
	CSphVector<int> dPoints;
	const int iSkipMagic = ( tArgs.m_bHasMorphology ? 1 : 0 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	const CSphFixedVector<RtSegment_t*> & dSegments = *((CSphFixedVector<RtSegment_t*> *)tArgs.m_pIndexData);

	DictEntryRtPayload_t tDict2Payload ( tArgs.m_bPayload, dSegments.GetLength() );
	ARRAY_FOREACH ( iSeg, dSegments )
	{
		const RtSegment_t * pSeg = dSegments[iSeg];
		if ( !pSeg->m_dWords.GetLength() )
			continue;

		dPoints.Resize ( 0 );
		if ( !ExtractInfixCheckpoints ( sSubstring, iSubLen, m_iMaxCodepointLength, pSeg->m_dWordCheckpoints.GetLength(), pSeg->m_dInfixFilterCP, dPoints ) )
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

			const RtWord_t * pWord = NULL;
			while ( ( pWord = tReader.UnzipWord() )!=NULL )
			{
				if ( tArgs.m_bHasMorphology && pWord->m_sWord[1]!=MAGIC_WORD_HEAD_NONSTEMMED )
					continue;

				// check it
				if ( !sphWildcardMatch ( (const char*)pWord->m_sWord+1+iSkipMagic, sWildcard ) )
					continue;

				// matched, lets add
				tDict2Payload.Add ( pWord, iSeg );
			}
		}
	}

	tDict2Payload.Convert ( tArgs );
}


bool RtIndex_t::RtQwordSetup ( RtQword_t * pQword, int iSeg, const SphChunkGuard_t & tGuard ) const
{
	// segment-specific setup pass
	if ( iSeg>=0 )
		return RtQwordSetupSegment ( pQword, tGuard.m_dRamChunks[iSeg], true, m_bKeywordDict, m_iWordsCheckpoint, tGuard.m_dKill[iSeg]->m_dKilled, m_tSettings );

	// stat-only pass
	// loop all segments, gather stats, do not setup anything
	pQword->m_iDocs = 0;
	pQword->m_iHits = 0;
	if ( !tGuard.m_dRamChunks.GetLength() )
		return true;

	// we care about the results anyway though
	// because if all (!) segments miss this word, we must notify the caller, right?
	bool bFound = false;
	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		bFound |= RtQwordSetupSegment ( pQword, tGuard.m_dRamChunks[i], false, m_bKeywordDict, m_iWordsCheckpoint, tGuard.m_dKill[i]->m_dKilled, m_tSettings );

	// sanity check
	assert (!( bFound==true && pQword->m_iDocs==0 ) );
	return bFound;
}


bool RtIndex_t::IsStarDict() const
{
	return m_tSettings.m_iMinPrefixLen>0 || m_tSettings.m_iMinInfixLen>0;
}


CSphDict * RtIndex_t::SetupExactDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer, bool bAddSpecial ) const
{
	assert ( pTokenizer );

	if ( !m_tSettings.m_bIndexExactWords )
		return pPrevDict;

	tContainer = new CSphDictExact ( pPrevDict );
	pTokenizer->AddPlainChar ( '=' );
	if ( bAddSpecial )
		pTokenizer->AddSpecials ( "=" );
	return tContainer.Ptr();
}


CSphDict * RtIndex_t::SetupStarDict ( CSphScopedPtr<CSphDict> & tContainer, CSphDict * pPrevDict, ISphTokenizer * pTokenizer ) const
{
	assert ( pTokenizer );
	if ( !m_bKeywordDict )
		return pPrevDict;

	if ( !IsStarDict() )
		return pPrevDict;

	tContainer = new CSphDictStarV8 ( pPrevDict, false, true );
	pTokenizer->AddPlainChar ( '*' );
	return tContainer.Ptr();
}

struct CSphAttrTypedLocator : public CSphAttrLocator
{
	ESphAttr m_eAttrType;
	CSphAttrTypedLocator()
		: m_eAttrType ( SPH_ATTR_NONE )
	{}
	inline void Set ( const CSphAttrLocator& tLoc, ESphAttr eAttrType )
	{
		m_bDynamic = tLoc.m_bDynamic;
		m_iBitCount = tLoc.m_iBitCount;
		m_iBitOffset = tLoc.m_iBitOffset;
		m_eAttrType = eAttrType;
	}
};

struct SphFinalMatchCounter_t : ISphMatchProcessor
{
	int		m_iCount;
	int		m_iSegments;

	explicit SphFinalMatchCounter_t ( int iSegments )
		: m_iCount ( 0 )
		, m_iSegments ( iSegments )
	{ }

	virtual void Process ( CSphMatch * pMatch )
	{
		int iMatchSegment = pMatch->m_iTag-1;
		if ( iMatchSegment>=0 && iMatchSegment<m_iSegments && pMatch->m_pStatic )
			m_iCount++;
	}
};

struct SphFinalMatchCopy_t : ISphMatchProcessor
{
	CSphRowitem *	m_pStorage;
	int				m_iSegments;
	int				m_iStaticSize;
	bool			m_bForce;
#ifndef NDEBUG
	CSphRowitem * m_pEnd;
#endif

	SphFinalMatchCopy_t ( CSphRowitem * pStorage, int iSegments, int iStaticSize, bool bForce )
		: m_pStorage ( pStorage )
		, m_iSegments ( iSegments )
		, m_iStaticSize ( iStaticSize )
		, m_bForce ( bForce )
	{ }

	virtual void Process ( CSphMatch * pMatch )
	{
		const int iMatchSegment = pMatch->m_iTag-1;
		if ( pMatch->m_pStatic && ( m_bForce || ( iMatchSegment>=0 && iMatchSegment<m_iSegments ) ) )
		{
			assert ( m_pStorage+m_iStaticSize<=m_pEnd );

			memcpy ( m_pStorage, STATIC2DOCINFO ( pMatch->m_pStatic ), sizeof(CSphRowitem)*m_iStaticSize );
			pMatch->m_pStatic = DOCINFO2ATTRS ( m_pStorage );
			DOCINFOSETID ( m_pStorage, (SphDocID_t)0 ); // the zero docid will show that the data was copied
			m_pStorage += m_iStaticSize;
		}
	}
};

struct SphFinalArenaCopy_t : ISphMatchProcessor
{
	const CSphFixedVector<const RtSegment_t *> & m_dSegments;
	const CSphVector<const BYTE *> & m_dDiskStrings;
	const CSphVector<const DWORD *> & m_dDiskMva;
	const CSphVector<CSphAttrTypedLocator> & m_dGetLoc;
	const CSphVector<CSphAttrLocator> & m_dSetLoc;

	CSphFixedVector<int> m_dJsonAssoc;
	CSphVector<DWORD> m_dOriginalJson;
	CSphVector<DWORD> m_dMovedJson;

	CSphTightVector<BYTE> m_dStorageString;
	CSphTightVector<DWORD> m_dStorageMva;
	const CSphBitvec & m_dMvaArenaFlag;

	SphFinalArenaCopy_t ( const CSphFixedVector<const RtSegment_t *> & dSegments, const CSphVector<const BYTE *> & dDiskStrings,
		const CSphVector<const DWORD *> & dDiskMva, const CSphVector<CSphAttrTypedLocator> & dGetLoc, const CSphVector<CSphAttrLocator> & dSetLoc,
		int iJsonFields, CSphBitvec & dMvaArenaFlag )
		: m_dSegments ( dSegments )
		, m_dDiskStrings ( dDiskStrings )
		, m_dDiskMva ( dDiskMva )
		, m_dGetLoc ( dGetLoc )
		, m_dSetLoc ( dSetLoc )
		, m_dJsonAssoc ( iJsonFields )
		, m_dMvaArenaFlag ( dMvaArenaFlag )
	{
		m_dStorageString.Add ( 0 );
		m_dStorageMva.Add ( 0 );
		ARRAY_FOREACH ( i, m_dJsonAssoc )
			m_dJsonAssoc[i] = -1;
	}

	virtual void Process ( CSphMatch * pMatch )
	{
		assert ( pMatch->m_iTag>=1 && pMatch->m_iTag<m_dSegments.GetLength()+m_dDiskStrings.GetLength()+1 );

		if ( !pMatch->m_pStatic )
			return;

		const int iSegCount = m_dSegments.GetLength();
		const int iStorageSrc = pMatch->m_iTag-1;
		bool bSegmentMatch = ( iStorageSrc<iSegCount );
		const BYTE * pBaseString = bSegmentMatch ? m_dSegments[iStorageSrc]->m_dStrings.Begin() : m_dDiskStrings[ iStorageSrc-iSegCount ];
		const DWORD * pBaseMva = bSegmentMatch ? m_dSegments[iStorageSrc]->m_dMvas.Begin() : m_dDiskMva[ iStorageSrc-iSegCount ];
		bool bArenaProhibit = bSegmentMatch ? true : m_dMvaArenaFlag.BitGet ( iStorageSrc-iSegCount );

		int iJson = 0;
		m_dOriginalJson.Resize ( 0 );
		m_dMovedJson.Resize ( 0 );

		ARRAY_FOREACH ( i, m_dGetLoc )
		{
			int64_t iAttr = 0;
			const CSphAttrTypedLocator& tLoc = m_dGetLoc[i];

			switch ( tLoc.m_eAttrType )
			{
			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
			{
				const SphAttr_t uOff = pMatch->GetAttr ( tLoc );
				DWORD uRebased = 0;
				if ( uOff>0 )
				{
					assert ( uOff<( I64C(1)<<32 ) ); // should be 32 bit offset
					assert ( !bSegmentMatch || (int)uOff<m_dSegments[iStorageSrc]->m_dStrings.GetLength() );
					uRebased = CopyPackedString ( pBaseString + uOff, m_dStorageString );
					iAttr = uRebased;
				}

				// store the map of full jsons in order to map json fields
				// note that m_dJsonAssoc mapping is calculated only once (see the m_dJsonAssoc[iJson]<0 condition)
				// we have to consider empty fields too (if any) otherwise mapping will be incorrect or out of bounds
				if ( tLoc.m_eAttrType==SPH_ATTR_JSON && m_dJsonAssoc.GetLength() )
				{
					m_dOriginalJson.Add ( (DWORD)uOff );
					m_dMovedJson.Add ( uRebased );
				}
			}
			break;
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
			{
				const DWORD * pMva = pMatch->GetAttrMVA ( tLoc, pBaseMva, bArenaProhibit );
				// have to fix up only existed attribute
				if ( pMva )
				{
					assert ( ( pMatch->GetAttr ( tLoc ) & MVA_ARENA_FLAG )<( I64C(1)<<32 ) ); // should be 32 bit offset
					assert ( !bSegmentMatch || (int)pMatch->GetAttr ( tLoc )<m_dSegments[iStorageSrc]->m_dMvas.GetLength() );
					iAttr = CopyMva ( pMva, m_dStorageMva );
				}
			}
			break;
			case SPH_ATTR_JSON_FIELD:
			{
				iAttr = pMatch->GetAttr ( tLoc );
				if ( iAttr )
				{
					ESphJsonType eJson = ESphJsonType ( iAttr>>32 );
					DWORD uOff = (DWORD)iAttr;
					if ( m_dJsonAssoc[iJson]<0 )
					{
						// tricky part. We have packed json field, but it points somewhere into original json.
						// since all jsons already relocated, we have to find the original (source) and recalculate the locator
						int k = -1;
						int iDistance = -1;
						ARRAY_FOREACH ( j, m_dOriginalJson )
							if ( iDistance<0 || ( uOff>=m_dOriginalJson[j] && uOff<( m_dOriginalJson[j]+iDistance ) ) )
							{
								iDistance = uOff - m_dOriginalJson[j];
								k = j;
							}
						assert ( k>=0 );
						m_dJsonAssoc[iJson] = k;
					}
					DWORD uNew = m_dMovedJson [ m_dJsonAssoc[iJson] ] - m_dOriginalJson [ m_dJsonAssoc[iJson] ] + uOff;
					++iJson;
					iAttr = ( ( (int64_t)uNew ) | ( ( (int64_t)eJson )<<32 ) );
				}
			}
			break;
			default:
				break;
			}

			const CSphAttrLocator & tSet = m_dSetLoc[i];
			assert ( !tSet.m_bDynamic || tSet.GetMaxRowitem() < (int)pMatch->m_pDynamic[-1] );
			sphSetRowAttr ( tSet.m_bDynamic ? pMatch->m_pDynamic : const_cast<CSphRowitem*>( pMatch->m_pStatic ), tSet, iAttr );
		}
	}
};


struct SphRtFinalMatchCalc_t : ISphMatchProcessor, ISphNoncopyable
{
	const CSphQueryContext &	m_tCtx;
	int							m_iSegments;

	SphRtFinalMatchCalc_t ( int iSegments, const CSphQueryContext & tCtx )
		: m_tCtx ( tCtx )
		, m_iSegments ( iSegments )
	{}

	virtual void Process ( CSphMatch * pMatch )
	{
		int iMatchSegment = pMatch->m_iTag-1;
		if ( iMatchSegment>=0 && iMatchSegment<m_iSegments && pMatch->m_pStatic )
			m_tCtx.CalcPostAggregate ( *pMatch );
	}
};


void RtIndex_t::GetReaderChunks ( SphChunkGuard_t & tGuard ) const
{
	if ( !m_dRamChunks.GetLength() && !m_dDiskChunks.GetLength() )
		return;

	m_tReading.ReadLock();
	tGuard.m_pReading = &m_tReading;

	m_tChunkLock.ReadLock ();

	tGuard.m_dRamChunks.Reset ( m_dRamChunks.GetLength() );
	tGuard.m_dKill.Reset ( m_dRamChunks.GetLength() );
	tGuard.m_dDiskChunks.Reset ( m_dDiskChunks.GetLength() );

	memcpy ( tGuard.m_dRamChunks.Begin(), m_dRamChunks.Begin(), sizeof(m_dRamChunks[0]) * m_dRamChunks.GetLength() );
	memcpy ( tGuard.m_dDiskChunks.Begin(), m_dDiskChunks.Begin(), sizeof(m_dDiskChunks[0]) * m_dDiskChunks.GetLength() );

	ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
	{
		KlistRefcounted_t * pKlist = tGuard.m_dRamChunks[i]->m_pKlist;
		pKlist->m_tRefCount.Inc();
		tGuard.m_dKill[i] = pKlist;

		assert ( tGuard.m_dRamChunks[i]->m_tRefCount.GetValue()>=0 );
		tGuard.m_dRamChunks[i]->m_tRefCount.Inc();
	}

	m_tChunkLock.Unlock ();
}


SphChunkGuard_t::~SphChunkGuard_t()
{
	if ( m_pReading )
		m_pReading->Unlock();

	if ( !m_dRamChunks.GetLength() )
		return;

	ARRAY_FOREACH ( i, m_dRamChunks )
	{
		assert ( m_dRamChunks[i]->m_tRefCount.GetValue()>=1 );

		KlistRefcounted_t * pKlist = const_cast<KlistRefcounted_t *> ( m_dKill[i] );
		uint64_t uRefs = pKlist->m_tRefCount.Dec();
		if ( uRefs==1 ) // 1 means we only owner when decrement event occurred
			SafeDelete ( pKlist );

		m_dRamChunks[i]->m_tRefCount.Dec();
	}
}


// FIXME! missing MVA, index_exact_words support
// FIXME? any chance to factor out common backend agnostic code?
// FIXME? do we need to support pExtraFilters?
bool RtIndex_t::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters,
	ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	assert ( ppSorters );
	assert ( pResult );

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

	assert ( pQuery );
	assert ( tArgs.m_iTag==0 );

	MEMORY ( MEM_RT_QUERY );

	// start counting
	pResult->m_iQueryTime = 0;
	int64_t tmQueryStart = sphMicroTimer();
	CSphQueryProfile * pProfiler = pResult->m_pProfile;

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_DICT_SETUP );

	// force ext2 mode for them
	// FIXME! eliminate this const breakage
	const_cast<CSphQuery*> ( pQuery )->m_eMode = SPH_MATCH_EXTENDED2;

	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );

	// wrappers
	// OPTIMIZE! make a lightweight clone here? and/or remove double clone?
	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( SPH_CLONE_QUERY ) );
	sphSetupQueryTokenizer ( pTokenizer.Ptr(), IsStarDict(), m_tSettings.m_bIndexExactWords );

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDict = m_pDict;
	if ( pDict->HasState() )
	{
		tDictCloned = pDict = pDict->Clone();
	}

	CSphScopedPtr<CSphDict> tDictStar ( NULL );
	pDict = SetupStarDict ( tDictStar, pDict, pTokenizer.Ptr() );

	CSphScopedPtr<CSphDict> tDictExact ( NULL );
	pDict = SetupExactDict ( tDictExact, pDict, pTokenizer.Ptr(), true );

	// calculate local idf for RT with disk chunks
	// in case of local_idf set but no external hash no full-scan query and RT has disk chunks
	const SmallStringHash_T<int64_t> * pLocalDocs = tArgs.m_pLocalDocs;
	SmallStringHash_T<int64_t> hLocalDocs;
	int64_t iTotalDocs = tArgs.m_iTotalDocs;
	bool bGotLocalDF = tArgs.m_bLocalDF;
	if ( tArgs.m_bLocalDF && !tArgs.m_pLocalDocs && !pQuery->m_sQuery.IsEmpty() && tGuard.m_dDiskChunks.GetLength() )
	{
		if ( pProfiler )
			pProfiler->Switch ( SPH_QSTATE_LOCAL_DF );

		CSphVector < CSphKeywordInfo > dKeywords;
		DoGetKeywords ( dKeywords, pQuery->m_sQuery.cstr(), true, false, NULL, tGuard );
		ARRAY_FOREACH ( i, dKeywords )
		{
			const CSphKeywordInfo & tKw = dKeywords[i];
			if ( !hLocalDocs.Exists ( tKw.m_sNormalized ) ) // skip dupes
				hLocalDocs.Add ( tKw.m_iDocs, tKw.m_sNormalized );
		}

		pLocalDocs = &hLocalDocs;
		iTotalDocs = GetStats().m_iTotalDocuments;
		bGotLocalDF = true;
	}

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_INIT );

	// FIXME! each result will point to its own MVA and string pools

	//////////////////////
	// search disk chunks
	//////////////////////

	pResult->m_bHasPrediction = pQuery->m_iMaxPredictedMsec>0;

	SphWordStatChecker_t tDiskStat;
	SphWordStatChecker_t tStat;
	tStat.Set ( pResult->m_hWordStats );

	int64_t tmMaxTimer = 0;
	if ( pQuery->m_uMaxQueryMsec>0 )
		tmMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time

	CSphVector<SphDocID_t> dCumulativeKList;
	KillListVector dMergedKillist;
	CSphVector<const BYTE *> dDiskStrings ( tGuard.m_dDiskChunks.GetLength() );
	CSphVector<const DWORD *> dDiskMva ( tGuard.m_dDiskChunks.GetLength() );
	CSphBitvec tMvaArenaFlag ( tGuard.m_dDiskChunks.GetLength() );
	if ( tGuard.m_dDiskChunks.GetLength() )
	{
		m_tKlist.Flush ( dCumulativeKList );
	}

	for ( int iChunk = tGuard.m_dDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
	{
		// because disk chunk search within the loop will switch the profiler state
		if ( pProfiler )
			pProfiler->Switch ( SPH_QSTATE_INIT );

		// collect & sort cumulative killlist for current chunk
		if ( iChunk<tGuard.m_dDiskChunks.GetLength()-1 )
		{
			const CSphIndex * pNewerChunk = tGuard.m_dDiskChunks [ iChunk+1 ];
			int iKlistEntries = pNewerChunk->GetKillListSize();
			if ( iKlistEntries )
			{
				// merging two kill lists, assuming they have sorted data
				const SphDocID_t * pSrc1 = dCumulativeKList.Begin();
				const SphDocID_t * pSrc2 = pNewerChunk->GetKillList();
				const SphDocID_t * pEnd1 = pSrc1 + dCumulativeKList.GetLength();
				const SphDocID_t * pEnd2 = pSrc2 + iKlistEntries;
				CSphVector<SphDocID_t> dNewCumulative ( ( pEnd1-pSrc1 )+( pEnd2-pSrc2 ) );
				SphDocID_t * pDst = dNewCumulative.Begin();

				while ( pSrc1!=pEnd1 && pSrc2!=pEnd2 )
				{
					if ( *pSrc1<*pSrc2 )
						*pDst = *pSrc1++;
					else if ( *pSrc2<*pSrc1 )
						*pDst = *pSrc2++;
					else
					{
						*pDst = *pSrc1++;
						// handle duplicates
						while ( *pDst==*pSrc1 ) pSrc1++;
						while ( *pDst==*pSrc2 ) pSrc2++;
					}
					pDst++;
				}
				while ( pSrc1!=pEnd1 ) *pDst++ = *pSrc1++;
				while ( pSrc2!=pEnd2 ) *pDst++ = *pSrc2++;

				assert ( pDst<=( dNewCumulative.Begin()+dNewCumulative.GetLength() ) );
				dNewCumulative.Resize ( pDst-dNewCumulative.Begin() );
				dNewCumulative.SwapData ( dCumulativeKList );
			}
		}

		dMergedKillist.Resize ( 0 );
		if ( dCumulativeKList.GetLength() )
		{
			dMergedKillist.Resize ( 1 );
			dMergedKillist.Last().m_pBegin = dCumulativeKList.Begin();
			dMergedKillist.Last().m_iLen = dCumulativeKList.GetLength();
		}

		CSphQueryResult tChunkResult;
		tChunkResult.m_pProfile = pResult->m_pProfile;
		CSphMultiQueryArgs tMultiArgs ( dMergedKillist, tArgs.m_iIndexWeight );
		// storing index in matches tag for finding strings attrs offset later, biased against default zero and segments
		tMultiArgs.m_iTag = tGuard.m_dRamChunks.GetLength()+iChunk+1;
		tMultiArgs.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
		tMultiArgs.m_bLocalDF = bGotLocalDF;
		tMultiArgs.m_pLocalDocs = pLocalDocs;
		tMultiArgs.m_iTotalDocs = iTotalDocs;

		if ( !tGuard.m_dDiskChunks[iChunk]->MultiQuery ( pQuery, &tChunkResult, iSorters, ppSorters, tMultiArgs ) )
		{
			// FIXME? maybe handle this more gracefully (convert to a warning)?
			pResult->m_sError = tChunkResult.m_sError;
			return false;
		}

		// check terms inconsistency among disk chunks
		const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hDstStats = tChunkResult.m_hWordStats;
		tStat.DumpDiffer ( hDstStats, m_sIndexName.cstr(), pResult->m_sWarning );
		if ( pResult->m_hWordStats.GetLength() )
		{
			pResult->m_hWordStats.IterateStart();
			while ( pResult->m_hWordStats.IterateNext() )
			{
				const CSphQueryResultMeta::WordStat_t * pDstStat = hDstStats ( pResult->m_hWordStats.IterateGetKey() );
				if ( pDstStat )
					pResult->AddStat ( pResult->m_hWordStats.IterateGetKey(), pDstStat->m_iDocs, pDstStat->m_iHits );
			}
		} else
		{
			pResult->m_hWordStats = hDstStats;
		}
		// keep last chunk statistics to check vs rt settings
		if ( iChunk==tGuard.m_dDiskChunks.GetLength()-1 )
			tDiskStat.Set ( hDstStats );
		if ( !iChunk )
			tStat.Set ( hDstStats );

		dDiskStrings[iChunk] = tChunkResult.m_pStrings;
		dDiskMva[iChunk] = tChunkResult.m_pMva;
		if ( tChunkResult.m_bArenaProhibit )
			tMvaArenaFlag.BitSet ( iChunk );
		pResult->m_iBadRows += tChunkResult.m_iBadRows;

		if ( pResult->m_bHasPrediction )
		{
			pResult->m_tStats.m_iFetchedDocs += tChunkResult.m_tStats.m_iFetchedDocs;
			pResult->m_tStats.m_iFetchedHits += tChunkResult.m_tStats.m_iFetchedHits;
			pResult->m_tStats.m_iSkips += tChunkResult.m_tStats.m_iSkips;
		}

		if ( iChunk && tmMaxTimer>0 && sphMicroTimer()>=tmMaxTimer )
		{
			pResult->m_sWarning = "query time exceeded max_query_time";
			break;
		}
	}

	////////////////////
	// search RAM chunk
	////////////////////

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_INIT );

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	int iMatchPoolSize = 0;
	ARRAY_FOREACH ( i, dSorters )
	{
		iMatchPoolSize += dSorters[i]->m_iMatchCapacity;
		if ( dSorters[i]->GetSchema ().GetAttrsCount ()>iMaxSchemaSize )
		{
			iMaxSchemaSize = dSorters[i]->GetSchema ().GetAttrsCount ();
			iMaxSchemaIndex = i;
		}
	}

	// setup calculations and result schema
	CSphQueryContext tCtx ( *pQuery );
	tCtx.m_pProfile = pProfiler;
	if ( !tCtx.SetupCalc ( pResult, dSorters[iMaxSchemaIndex]->GetSchema(), m_tSchema, NULL, false, true ) )
		return false;

	tCtx.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
	tCtx.m_pLocalDocs = pLocalDocs;
	tCtx.m_iTotalDocs = iTotalDocs;

	// setup search terms
	RtQwordSetup_t tTermSetup ( tGuard );
	tTermSetup.m_pDict = pDict;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	tTermSetup.m_iDynamicRowitems = dSorters[iMaxSchemaIndex]->GetSchema().GetDynamicSize();
	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time
	tTermSetup.m_pWarning = &pResult->m_sWarning;
	tTermSetup.SetSegment ( -1 );
	tTermSetup.m_pCtx = &tCtx;

	// setup prediction constrain
	CSphQueryStats tQueryStats;
	int64_t iNanoBudget = (int64_t)(pQuery->m_iMaxPredictedMsec) * 1000000; // from milliseconds to nanoseconds
	tQueryStats.m_pNanoBudget = &iNanoBudget;
	if ( pResult->m_bHasPrediction )
		tTermSetup.m_pStats = &tQueryStats;

	// bind weights
	tCtx.BindWeights ( pQuery, m_tSchema, pResult->m_sWarning );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (BYTE *)pQuery->m_sQuery.cstr();

	CSphScopedPtr<ISphFieldFilter> pFieldFilter ( NULL );
	if ( m_pFieldFilter )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter->Apply ( sModifiedQuery, strlen ( (char*)sModifiedQuery ), dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// parse query
	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_PARSE );

	XQQuery_t tParsed;
	// FIXME!!! provide segments list instead index to tTermSetup.m_pIndex
	bool bParsed = sphParseExtendedQuery ( tParsed, (const char *)sModifiedQuery, pQuery, pTokenizer.Ptr(), &m_tSchema, pDict, m_tSettings );

	if ( !bParsed )
	{
		pResult->m_sError = tParsed.m_sParseError;
		return false;
	}
	if ( !tParsed.m_sParseWarning.IsEmpty() )
		pResult->m_sWarning = tParsed.m_sParseWarning;

	// transform query if needed (quorum transform, etc.)
	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_TRANSFORMS );

	// FIXME!!! provide segments list instead index
	sphTransformExtendedQuery ( &tParsed.m_pRoot, m_tSettings, pQuery->m_bSimplify, this );

	if ( m_bExpandKeywords )
	{
		tParsed.m_pRoot = sphQueryExpandKeywords ( tParsed.m_pRoot, m_tSettings );
		tParsed.m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	if ( m_tSettings.m_uAotFilterMask )
		TransformAotFilter ( tParsed.m_pRoot, pDict->GetWordforms(), m_tSettings );

	// expanding prefix in word dictionary case
	CSphScopedPayload tPayloads;
	if ( m_bKeywordDict && IsStarDict() )
	{
		ExpansionContext_t tExpCtx;
		tExpCtx.m_pWordlist = this;
		tExpCtx.m_pBuf = NULL;
		tExpCtx.m_pResult = pResult;
		tExpCtx.m_iMinPrefixLen = m_tSettings.m_iMinPrefixLen;
		tExpCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
		tExpCtx.m_iExpansionLimit = m_iExpansionLimit;
		tExpCtx.m_bHasMorphology = m_pDict->HasMorphology();
		tExpCtx.m_bMergeSingles = ( m_tSettings.m_eDocinfo!=SPH_DOCINFO_INLINE && ( pQuery->m_uDebugFlags & QUERY_DEBUG_NO_PAYLOAD )==0 );
		tExpCtx.m_pPayloads = &tPayloads;
		tExpCtx.m_pIndexData = &tGuard.m_dRamChunks;

		tParsed.m_pRoot = sphExpandXQNode ( tParsed.m_pRoot, tExpCtx );
	}

	if ( !sphCheckQueryHeight ( tParsed.m_pRoot, pResult->m_sError ) )
		return false;

	// set zonespanlist settings
	tParsed.m_bNeedSZlist = pQuery->m_bZSlist;

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_INIT );

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( tParsed, pQuery, pResult, tTermSetup, tCtx ) );

	if ( !pRanker.Ptr() )
		return false;

	tCtx.SetupExtraData ( pRanker.Ptr(), iSorters==1 ? ppSorters[0] : NULL );

	// check terms inconsistency disk chunks vs rt vs previous indexes
	tDiskStat.DumpDiffer ( pResult->m_hWordStats, m_sIndexName.cstr(), pResult->m_sWarning );
	tStat.DumpDiffer ( pResult->m_hWordStats, m_sIndexName.cstr(), pResult->m_sWarning );

	// empty index, empty result
	if ( !tGuard.m_dRamChunks.GetLength() && !tGuard.m_dDiskChunks.GetLength() )
	{
		pResult->m_iQueryTime = 0;
		return true;
	}

	pRanker->ExtraData ( EXTRA_SET_POOL_CAPACITY, (void**)&iMatchPoolSize );

	// check for the possible integer overflow in m_dPool.Resize
	int64_t iPoolSize = 0;
	if ( pRanker->ExtraData ( EXTRA_GET_POOL_SIZE, (void**)&iPoolSize ) && iPoolSize>INT_MAX )
	{
		pResult->m_sError.SetSprintf ( "ranking factors pool too big (%d Mb), reduce max_matches", (int)( iPoolSize/1024/1024 ) );
		return false;
	}

	// probably redundant, but just in case
	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_INIT );

	// search segments no looking to max_query_time
	// FIXME!!! move searching at segments before disk chunks as result set is safe with kill-lists
	if ( tGuard.m_dRamChunks.GetLength() )
	{
		// setup filters
		// FIXME! setup filters MVA pool
		bool bFullscan = ( pQuery->m_eMode==SPH_MATCH_FULLSCAN || pQuery->m_sQuery.IsEmpty() );
		if ( !tCtx.CreateFilters ( bFullscan, &pQuery->m_dFilters, dSorters[iMaxSchemaIndex]->GetSchema(), NULL, NULL, pResult->m_sError, pResult->m_sWarning, pQuery->m_eCollation, false, KillListVector() ) )
			return false;

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
			if ( pProfiler )
				pProfiler->Switch ( SPH_QSTATE_FULLSCAN );

			// full scan
			// FIXME? OPTIMIZE? add shortcuts here too?
			CSphMatch tMatch;
			tMatch.Reset ( dSorters[iMaxSchemaIndex]->GetSchema().GetDynamicSize() );
			tMatch.m_iWeight = tArgs.m_iIndexWeight;

			ARRAY_FOREACH ( iSeg, tGuard.m_dRamChunks )
			{
				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( tGuard.m_dRamChunks[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( tGuard.m_dRamChunks[iSeg]->m_dMvas.Begin(), false );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( tGuard.m_dRamChunks[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( tGuard.m_dRamChunks[iSeg]->m_dMvas.Begin(), false );
				}

				RtRowIterator_t tIt ( tGuard.m_dRamChunks[iSeg], m_iStride, false, NULL, tGuard.m_dKill[iSeg]->m_dKilled );
				for ( ;; )
				{
					const CSphRowitem * pRow = tIt.GetNextAliveRow();
					if ( !pRow )
						break;

					tMatch.m_uDocID = DOCINFO2ID(pRow);
					tMatch.m_pStatic = DOCINFO2ATTRS(pRow); // FIXME! overrides

					tCtx.CalcFilter ( tMatch );
					if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
					{
						tCtx.FreeStrFilter ( tMatch );
						continue;
					}

					if ( bRandomize )
						tMatch.m_iWeight = ( sphRand() & 0xffff ) * tArgs.m_iIndexWeight;

					tCtx.CalcSort ( tMatch );
					tCtx.CalcFinal ( tMatch ); // OPTIMIZE? could be possibly done later

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
			ARRAY_FOREACH ( iSeg, tGuard.m_dRamChunks )
			{
				if ( pProfiler )
					pProfiler->Switch ( SPH_QSTATE_INIT_SEGMENT );

				tTermSetup.SetSegment ( iSeg );
				pRanker->Reset ( tTermSetup );

				// for lookups to work
				tCtx.m_pIndexData = tGuard.m_dRamChunks[iSeg];

				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( tGuard.m_dRamChunks[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( tGuard.m_dRamChunks[iSeg]->m_dMvas.Begin(), false );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( tGuard.m_dRamChunks[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( tGuard.m_dRamChunks[iSeg]->m_dMvas.Begin(), false );
				}
				PoolPtrs_t tMva;
				tMva.m_pMva = tGuard.m_dRamChunks[iSeg]->m_dMvas.Begin();
				tMva.m_bArenaProhibit = false;
				pRanker->ExtraData ( EXTRA_SET_MVAPOOL, (void**)&tMva );
				pRanker->ExtraData ( EXTRA_SET_STRINGPOOL, (void**)tGuard.m_dRamChunks[iSeg]->m_dStrings.Begin() );

				CSphMatch * pMatch = pRanker->GetMatchesBuffer();
				for ( ;; )
				{
					// ranker does profile switches internally in GetMatches()
					int iMatches = pRanker->GetMatches();
					if ( iMatches<=0 )
						break;

					if ( pProfiler )
						pProfiler->Switch ( SPH_QSTATE_SORT );
					for ( int i=0; i<iMatches; i++ )
					{
						if ( tCtx.m_bLookupSort )
						{
							// tricky bit
							// that kills two birds with one stone, sort of
							// first, broken indexes MIGHT yield nonexistent docids at this point
							// second, query cache ranker WILL return nonexistent docids (from other segments)
							// because the cached query only stores docids, not segment ids (which change all the time anyway)
							// to catch broken indexes or other bugs in debug, we have that assert
							// but release builds will simply ignore nonexistent docids, whatever the reason they do not exist
							const CSphRowitem * pRow = FindDocinfo ( tGuard.m_dRamChunks[iSeg], pMatch[i].m_uDocID );
							assert ( pRanker->IsCache() || pRow );
							if ( !pRow )
							{
								tCtx.m_iBadRows++;
								continue;
							}
							CopyDocinfo ( pMatch[i], pRow );
						}

						pMatch[i].m_iWeight *= tArgs.m_iIndexWeight;
						if ( bRandomize )
							pMatch[i].m_iWeight = ( sphRand() & 0xffff ) * tArgs.m_iIndexWeight;

						tCtx.CalcSort ( pMatch[i] );
						tCtx.CalcFinal ( pMatch[i] ); // OPTIMIZE? could be possibly done later

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
						iSeg = tGuard.m_dRamChunks.GetLength();
						break;
					}
				}
			}
		}
	}

	// do final expression calculations
	if ( tCtx.m_dCalcPostAggregate.GetLength () )
	{
		const int iSegmentsTotal = tGuard.m_dRamChunks.GetLength ();
		SphRtFinalMatchCalc_t tFinal ( iSegmentsTotal, tCtx );
		for ( int iSorter = 0; iSorter<iSorters; iSorter++ )
		{
			ISphMatchSorter * pTop = ppSorters[iSorter];
			pTop->Finalize ( tFinal, false );
		}
	}


	//////////////////////
	// coping match's attributes to external storage in result set
	//////////////////////

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_FINALIZE );

	CSphVector<CSphAttrTypedLocator> dGetLoc;
	CSphVector<CSphAttrLocator> dSetLoc;
	int iJsonFields = 0;
	for ( int i=0; i<dSorters[iMaxSchemaIndex]->GetSchema().GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tSetInfo = dSorters[iMaxSchemaIndex]->GetSchema().GetAttr(i);
		if ( tSetInfo.m_eAttrType==SPH_ATTR_STRING || tSetInfo.m_eAttrType==SPH_ATTR_JSON
			|| tSetInfo.m_eAttrType==SPH_ATTR_UINT32SET || tSetInfo.m_eAttrType==SPH_ATTR_INT64SET )
		{
			const CSphColumnInfo * pCol = m_tSchema.GetAttr ( tSetInfo.m_sName.cstr() );
			if ( !pCol && ( tSetInfo.m_eAttrType==SPH_ATTR_UINT32SET || tSetInfo.m_eAttrType==SPH_ATTR_INT64SET ) )
			{
				pCol = &tSetInfo;
			}
			assert ( pCol );

			dGetLoc.Add().Set ( pCol->m_tLocator, tSetInfo.m_eAttrType );
			dSetLoc.Add ( tSetInfo.m_tLocator );
		}
		iJsonFields += ( tSetInfo.m_eAttrType==SPH_ATTR_JSON_FIELD );
	}

	// put the json fields attrs at the very end (surely after all json attrs)
	if ( iJsonFields )
	{
		for ( int i=0; i<dSorters[iMaxSchemaIndex]->GetSchema().GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tSetInfo = dSorters[iMaxSchemaIndex]->GetSchema().GetAttr(i);
			if ( tSetInfo.m_eAttrType==SPH_ATTR_JSON_FIELD )
			{
				const int iInLocator = dSorters[iMaxSchemaIndex]->GetSchema().GetAttrIndex ( tSetInfo.m_sName.cstr() );
				assert ( iInLocator>=0 );

				dGetLoc.Add().Set ( dSorters[iMaxSchemaIndex]->GetSchema().GetAttr ( iInLocator ).m_tLocator, SPH_ATTR_JSON_FIELD );
				dSetLoc.Add ( tSetInfo.m_tLocator );
			}
		}
	}

	// we're copying docinfo from RT segments to result set (segments got merged in RAM)
	// also static part of docinfo:
	// during optimize process (disk chunks are merged and removed from RT index)
	// result set has arena attributes (STRING and MVA) (all these attrs should be at one pool)
	bool bOptimizing = m_bOptimizing;
	bool bHasArenaAttrs = ( dSetLoc.GetLength()>0 );
	const int iSegmentsTotal = tGuard.m_dRamChunks.GetLength();
	bool bSegmentMatchesFixup = ( m_tSchema.GetStaticSize()>0 && iSegmentsTotal>0 );
	if ( bSegmentMatchesFixup || bHasArenaAttrs || bOptimizing )
	{
		MEMORY ( MEM_RT_RES_MATCHES );

		// we need to count matches for allocating arena
		// as we are going to fix match's m_pStatic pointers later
		// and copy real match's data to arena

		int iFixupCount = 0;
		SphFinalMatchCounter_t fnCounter ( iSegmentsTotal );

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];
			const int iMatchesCount = pSorter->GetLength();

			if ( bHasArenaAttrs || bOptimizing )
			{
				iFixupCount += iMatchesCount;
				continue;
			} else
			{
				// copying only RT segments docinfo (no need to copy docinfo from disk chunks)
				pSorter->Finalize ( fnCounter, false );
			}
		}
		iFixupCount += fnCounter.m_iCount;

		if ( iFixupCount>0 || bHasArenaAttrs || bOptimizing )
		{
			const int iStaticSize = m_tSchema.GetStaticSize() + DWSIZEOF ( SphDocID_t );
			CSphRowitem * pAttr = new CSphRowitem [ iFixupCount * iStaticSize ];
			pResult->m_dStorage2Free.Add ( (BYTE*)pAttr );

			SphFinalMatchCopy_t fnCopy ( pAttr, iSegmentsTotal, iStaticSize, ( bHasArenaAttrs || bOptimizing ) );
#ifndef NDEBUG
			fnCopy.m_pEnd = pAttr + iFixupCount * iStaticSize;
#endif

			ARRAY_FOREACH ( iSorter, dSorters )
			{
				ISphMatchSorter * pSorter = dSorters[iSorter];
				pSorter->Finalize ( fnCopy, false );
			}
		}
	}


	//////////////////////
	// fixing string offset and data in resulting matches
	//////////////////////

	MEMORY ( MEM_RT_RES_STRINGS );

	if ( bHasArenaAttrs || bOptimizing )
	{
		assert ( !pResult->m_pStrings && !pResult->m_pMva );
		SphFinalArenaCopy_t fnArena ( tGuard.m_dRamChunks, dDiskStrings, dDiskMva, dGetLoc, dSetLoc, iJsonFields, tMvaArenaFlag );

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];
			pSorter->Finalize ( fnArena, false );
		}

		if ( fnArena.m_dStorageString.GetLength()>1 )
		{
			BYTE * pStrings = fnArena.m_dStorageString.LeakData ();
			pResult->m_dStorage2Free.Add ( pStrings );
			pResult->m_pStrings = pStrings;
		}
		if ( fnArena.m_dStorageMva.GetLength()>1 )
		{
			DWORD * pMva = fnArena.m_dStorageMva.LeakData();
			pResult->m_dStorage2Free.Add ( (BYTE*)pMva );
			pResult->m_pMva = pMva;
		}
	}

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_UNKNOWN );

	if ( pResult->m_bHasPrediction )
	{
		pResult->m_tStats.m_iFetchedDocs += tQueryStats.m_iFetchedDocs;
		pResult->m_tStats.m_iFetchedHits += tQueryStats.m_iFetchedHits;
		pResult->m_tStats.m_iSkips += tQueryStats.m_iSkips;
	}

	// query timer
	pResult->m_iQueryTime = int ( ( sphMicroTimer()-tmQueryStart )/1000 );
	return true;
}

bool RtIndex_t::MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults,
								ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// FIXME! OPTIMIZE! implement common subtree cache here
	bool bResult = false;
	for ( int i=0; i<iQueries; i++ )
		if ( MultiQuery ( &ppQueries[i], ppResults[i], 1, &ppSorters[i], tArgs ) )
			bResult = true;
		else
			ppResults[i]->m_iMultiplier = -1;

	return bResult;
}


void RtIndex_t::AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, CSphDict * pDict, bool bGetStats, int iQpos, RtQword_t * pQueryWord, CSphVector <CSphKeywordInfo> & dKeywords, const SphChunkGuard_t & tGuard ) const
{
	assert ( !bGetStats || pQueryWord );

	SphWordID_t iWord = pDict->GetWordID ( sWord );
	if ( !iWord )
		return;

	if ( bGetStats )
	{
		pQueryWord->Reset();
		pQueryWord->m_uWordID = iWord;
		pQueryWord->m_sWord = (const char *)sTokenized;
		pQueryWord->m_sDictWord = (const char *)sWord;
		ARRAY_FOREACH ( iSeg, tGuard.m_dRamChunks )
			RtQwordSetupSegment ( pQueryWord, tGuard.m_dRamChunks[iSeg], false, m_bKeywordDict, m_iWordsCheckpoint, tGuard.m_dKill[iSeg]->m_dKilled, m_tSettings );
	}

	CSphKeywordInfo & tInfo = dKeywords.Add();
	tInfo.m_sTokenized = (const char *)sTokenized;
	tInfo.m_sNormalized = (const char*)sWord;
	tInfo.m_iDocs = bGetStats ? pQueryWord->m_iDocs : 0;
	tInfo.m_iHits = bGetStats ? pQueryWord->m_iHits : 0;
	tInfo.m_iQpos = iQpos;

	if ( tInfo.m_sNormalized.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
		*(char *)tInfo.m_sNormalized.cstr() = '=';
}


struct CSphRtQueryFilter : public ISphQueryFilter, public ISphNoncopyable
{
	const RtIndex_t *	m_pIndex;
	RtQword_t *			m_pQword;
	bool				m_bGetStats;
	const SphChunkGuard_t & m_tGuard;

	CSphRtQueryFilter ( const RtIndex_t * pIndex, RtQword_t * pQword, bool bGetStats, const SphChunkGuard_t & tGuard )
		: m_pIndex ( pIndex )
		, m_pQword ( pQword )
		, m_bGetStats ( bGetStats )
		, m_tGuard ( tGuard )
	{}

	virtual void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords )
	{
		assert ( m_pIndex && m_pQword );
		m_pIndex->AddKeywordStats ( sWord, sTokenized, m_pDict, m_bGetStats, iQpos, m_pQword, dKeywords, m_tGuard );
	}
};


bool RtIndex_t::DoGetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, bool bGetStats, bool bFillOnly, CSphString * , const SphChunkGuard_t & tGuard ) const
{
	if ( !bFillOnly )
		dKeywords.Resize ( 0 );

	if ( ( bFillOnly && !dKeywords.GetLength() ) || ( !bFillOnly && ( !sQuery || !sQuery[0] ) ) )
		return true;

	RtQword_t tQword;

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( SPH_CLONE_INDEX ) ); // avoid race
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually
	if ( IsStarDict() )
		pTokenizer->AddPlainChar ( '*' );

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDictBase = m_pDict;
	if ( pDictBase->HasState() )
	{
		tDictCloned = pDictBase = pDictBase->Clone();
	}

	CSphScopedPtr<CSphDict> tDict ( NULL );
	CSphDict * pDict = SetupStarDict ( tDict, pDictBase, pTokenizer.Ptr() );

	CSphScopedPtr<CSphDict> tDict2 ( NULL );
	pDict = SetupExactDict ( tDict2, pDict, pTokenizer.Ptr(), false );

	// FIXME!!! missed bigram, FieldFilter
	CSphRtQueryFilter tAotFilter ( this, &tQword, bGetStats, tGuard );
	tAotFilter.m_pTokenizer = pTokenizer.Ptr();
	tAotFilter.m_pDict = pDict;
	tAotFilter.m_pSettings = &m_tSettings;

	if ( !bFillOnly )
	{
		pTokenizer->SetBuffer ( (BYTE *)sQuery, strlen ( sQuery ) );

		tAotFilter.GetKeywords ( dKeywords );
	} else
	{
		BYTE sWord[SPH_MAX_KEYWORD_LEN];

		ARRAY_FOREACH ( i, dKeywords )
		{
			CSphKeywordInfo & tInfo = dKeywords[i];
			int iLen = tInfo.m_sTokenized.Length();
			memcpy ( sWord, tInfo.m_sTokenized.cstr(), iLen );
			sWord[iLen] = '\0';

			SphWordID_t iWord = pDict->GetWordID ( sWord );
			if ( iWord )
			{
				tQword.Reset();
				tQword.m_uWordID = iWord;
				tQword.m_sWord = tInfo.m_sTokenized;
				tQword.m_sDictWord = (const char *)sWord;
				ARRAY_FOREACH ( iSeg, tGuard.m_dRamChunks )
					RtQwordSetupSegment ( &tQword, tGuard.m_dRamChunks[iSeg], false, m_bKeywordDict, m_iWordsCheckpoint, tGuard.m_dKill[iSeg]->m_dKilled, m_tSettings );

				tInfo.m_iDocs += tQword.m_iDocs;
				tInfo.m_iHits += tQword.m_iHits;
			}
		}
	}

	// get stats from disk chunks too
	if ( !bGetStats )
		return true;

	ARRAY_FOREACH ( iChunk, tGuard.m_dDiskChunks )
		tGuard.m_dDiskChunks[iChunk]->FillKeywords ( dKeywords );

	return true;
}


bool RtIndex_t::GetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, bool bGetStats, CSphString * pError ) const
{
	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );
	bool bGot = DoGetKeywords ( dKeywords, sQuery, bGetStats, false, pError, tGuard );
	return bGot;
}


bool RtIndex_t::FillKeywords ( CSphVector<CSphKeywordInfo> & dKeywords ) const
{
	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );
	bool bGot = DoGetKeywords ( dKeywords, NULL, true, true, NULL, tGuard );
	return bGot;
}


static const RtSegment_t * UpdateFindSegment ( const SphChunkGuard_t & tGuard, const CSphRowitem ** ppRow, SphDocID_t uDocID )
{
	assert ( ppRow && ( ( *ppRow!=NULL ) ^ ( uDocID!=0 ) ) );

	const CSphRowitem * pRow = *ppRow;
	*ppRow = NULL;

	if ( uDocID )
	{
		ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		{
			bool bKilled = ( tGuard.m_dKill[i]->m_dKilled.BinarySearch ( uDocID )!=NULL );
			if ( bKilled )
				continue;

			pRow = const_cast<CSphRowitem *> ( tGuard.m_dRamChunks[i]->FindRow ( uDocID ) );
			if ( !pRow )
				continue;

			*ppRow = pRow;
			return tGuard.m_dRamChunks[i];
		}
	} else
	{
		ARRAY_FOREACH ( i, tGuard.m_dRamChunks )
		{
			const CSphTightVector<CSphRowitem> & dRows = tGuard.m_dRamChunks[i]->m_dRows;
			if ( dRows.Begin()<=pRow && pRow<dRows.Begin()+ dRows.GetLength() )
			{
				*ppRow = pRow;
				return tGuard.m_dRamChunks[i];
			}
		}
	}

	return NULL;
}


// FIXME! might be inconsistent in case disk chunk update fails
int RtIndex_t::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError, CSphString & sWarning )
{
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRows.GetLength() );
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRowOffset.GetLength() );
	int iRows = tUpd.m_dDocids.GetLength();
	bool bHasMva = false;

	if ( !iRows )
		return 0;

	// remap update schema to index schema
	int iUpdLen = tUpd.m_dAttrs.GetLength();
	CSphVector<CSphAttrLocator> dLocators ( iUpdLen );
	CSphBitvec dBigints ( iUpdLen );
	CSphBitvec dDoubles ( iUpdLen );
	CSphBitvec dJsonFields ( iUpdLen );
	CSphBitvec dBigint2Float ( iUpdLen );
	CSphBitvec dFloat2Bigint ( iUpdLen );
	CSphVector<int64_t> dValues ( iUpdLen );
	CSphVector < CSphRefcountedPtr<ISphExpr> > dExpr ( iUpdLen );
	memset ( dLocators.Begin(), 0, dLocators.GetSizeBytes() );
	memset ( dValues.Begin(), 0, dValues.GetSizeBytes() );

	uint64_t uDst64 = 0;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		int iIdx = m_tSchema.GetAttrIndex ( tUpd.m_dAttrs[i] );

		if ( iIdx<0 )
		{
			CSphString sJsonCol, sJsonKey;
			if ( sphJsonNameSplit ( tUpd.m_dAttrs[i], &sJsonCol, &sJsonKey ) )
			{
				iIdx = m_tSchema.GetAttrIndex ( sJsonCol.cstr() );
				if ( iIdx>=0 )
					dExpr[i] = sphExprParse ( tUpd.m_dAttrs[i], m_tSchema, NULL, NULL, sError, NULL );
			}
		}

		if ( iIdx>=0 )
		{
			// forbid updates on non-int columns
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iIdx);
			if ( !( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP
				|| tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET
				|| tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_FLOAT || tCol.m_eAttrType==SPH_ATTR_JSON ))
			{
				sError.SetSprintf ( "attribute '%s' can not be updated "
					"(must be boolean, integer, bigint, float, timestamp, MVA or JSON)",
					tUpd.m_dAttrs[i] );
				return -1;
			}

			bool bSrcMva = ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );
			bool bDstMva = ( tUpd.m_dTypes[i]==SPH_ATTR_UINT32SET || tUpd.m_dTypes[i]==SPH_ATTR_INT64SET );
			if ( bSrcMva!=bDstMva )
			{
				sError.SetSprintf ( "attribute '%s' MVA flag mismatch", tUpd.m_dAttrs[i] );
				return -1;
			}

			if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET && tUpd.m_dTypes[i]==SPH_ATTR_INT64SET )
			{
				sError.SetSprintf ( "attribute '%s' MVA bits (dst=%d, src=%d) mismatch", tUpd.m_dAttrs[i],
					tCol.m_eAttrType, tUpd.m_dTypes[i] );
				return -1;
			}

			if ( tCol.m_eAttrType==SPH_ATTR_INT64SET )
				uDst64 |= ( U64C(1)<<i );

			if ( tCol.m_eAttrType==SPH_ATTR_FLOAT )
			{
				if ( tUpd.m_dTypes[i]==SPH_ATTR_BIGINT )
					dBigint2Float.BitSet(i);
			} else if ( tCol.m_eAttrType==SPH_ATTR_JSON )
				dJsonFields.BitSet(i);
			else if ( tCol.m_eAttrType==SPH_ATTR_BIGINT )
			{
				if ( tUpd.m_dTypes[i]==SPH_ATTR_FLOAT )
					dFloat2Bigint.BitSet(i);
			}

			dLocators[i] = tCol.m_tLocator;
			bHasMva |= ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );
		} else if ( tUpd.m_bIgnoreNonexistent )
		{
			continue;
		} else
		{
			sError.SetSprintf ( "attribute '%s' not found", tUpd.m_dAttrs[i] );
			return -1;
		}

		// this is a hack
		// Query parser tries to detect an attribute type. And this is wrong because, we should
		// take attribute type from schema. Probably we'll rewrite updates in future but
		// for now this fix just works.
		// Fixes cases like UPDATE float_attr=1 WHERE id=1;
		assert ( iIdx>=0 );
		if ( tUpd.m_dTypes[i]==SPH_ATTR_INTEGER && m_tSchema.GetAttr(iIdx).m_eAttrType==SPH_ATTR_FLOAT )
		{
			const_cast<CSphAttrUpdate &>(tUpd).m_dTypes[i] = SPH_ATTR_FLOAT;
			const_cast<CSphAttrUpdate &>(tUpd).m_dPool[i] = sphF2DW ( (float)tUpd.m_dPool[i] );
		}

		if ( tUpd.m_dTypes[i]==SPH_ATTR_BIGINT )
			dBigints.BitSet(i);
		else if ( tUpd.m_dTypes[i]==SPH_ATTR_FLOAT )
			dDoubles.BitSet(i);
		switch ( tUpd.m_dTypes[i] )
		{
			case SPH_ATTR_FLOAT:	dValues[i] = sphD2QW ( (double)sphDW2F ( tUpd.m_dPool[i] ) ); break;
			case SPH_ATTR_BIGINT:	dValues[i] = MVA_UPSIZE ( &tUpd.m_dPool[i] ); break;
			default:				dValues[i] = tUpd.m_dPool[i]; break;
		}
	}

	// check if we are empty
	if ( !m_dRamChunks.GetLength() && !m_dDiskChunks.GetLength() )
	{
		return true;
	}

	// FIXME!!! grab Writer lock to prevent segments retirement during commit(merge)
	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );

	// do the update
	int iUpdated = 0;
	DWORD uUpdateMask = 0;
	int iJsonWarnings = 0;

	// bRaw do only one pass as it has pointers to actual data at segments
	// MVA && bRaw should find appropriate segment to update storage there

	int iFirst = ( iIndex<0 ) ? 0 : iIndex;
	int iLast = ( iIndex<0 ) ? iRows : iIndex+1;

	// first pass, if needed
	if ( tUpd.m_bStrict )
	{
		for ( int iUpd=iFirst; iUpd<iLast; iUpd++ )
		{
			const CSphRowitem * pRow = tUpd.m_dRows[iUpd];
			SphDocID_t uDocid = tUpd.m_dDocids[iUpd];

			const RtSegment_t * pSegment = UpdateFindSegment ( tGuard, &pRow, uDocid );
			if ( !pRow )
				continue;

			assert ( !uDocid || DOCINFO2ID(pRow)==uDocid );
			pRow = DOCINFO2ATTRS(pRow);

			int iPos = tUpd.m_dRowOffset[iUpd];
			ARRAY_FOREACH ( iCol, tUpd.m_dAttrs )
			{
				if ( !dJsonFields.BitGet ( iCol ) )
					continue;

				ESphJsonType eType = dDoubles.BitGet ( iCol )
					? JSON_DOUBLE
					: ( dBigints.BitGet ( iCol ) ? JSON_INT64 : JSON_INT32 );

				if ( !sphJsonInplaceUpdate ( eType, dValues[iCol], dExpr[iCol].Ptr(), (BYTE *)pSegment->m_dStrings.Begin(), pRow, false ) )
				{
					sError.SetSprintf ( "attribute '%s' can not be updated (not found or incompatible types)", tUpd.m_dAttrs[iCol] );
					return -1;
				}

				iPos += dBigints.BitGet ( iCol ) ? 2 : 1;
			}
		}
	}

	CSphVector<SphDocID_t> dKilled;
	m_tKlist.Flush ( dKilled );

	for ( int iUpd=iFirst; iUpd<iLast; iUpd++ )
	{
		// search segments first
		bool bUpdated = false;
		for ( ;; )
		{
			const CSphRowitem * pRow = tUpd.m_dRows[iUpd];
			SphDocID_t uDocid = tUpd.m_dDocids[iUpd];

			RtSegment_t * pSegment = const_cast<RtSegment_t *> ( UpdateFindSegment ( tGuard, &pRow, uDocid ) );
			if ( !pRow )
				break;

			assert ( pSegment );
			assert ( !uDocid || ( DOCINFO2ID(pRow)==uDocid ) );
			pRow = DOCINFO2ATTRS(pRow);

			int iPos = tUpd.m_dRowOffset[iUpd];
			ARRAY_FOREACH ( iCol, tUpd.m_dAttrs )
			{
				if ( dJsonFields.BitGet ( iCol ) )
				{
					ESphJsonType eType = dDoubles.BitGet ( iCol )
						? JSON_DOUBLE
						: ( dBigints.BitGet ( iCol ) ? JSON_INT64 : JSON_INT32 );

					if ( sphJsonInplaceUpdate ( eType, dValues[iCol], dExpr[iCol].Ptr(), pSegment->m_dStrings.Begin(), pRow, true ) )
					{
						bUpdated = true;
						uUpdateMask |= ATTRS_STRINGS_UPDATED;

					} else
						iJsonWarnings++;

					iPos += dBigints.BitGet ( iCol ) ? 2 : 1;
					continue;
				}

				if ( !( tUpd.m_dTypes[iCol]==SPH_ATTR_UINT32SET || tUpd.m_dTypes[iCol]==SPH_ATTR_INT64SET ) )
				{
					// plain update
					bUpdated = true;
					uUpdateMask |= ATTRS_UPDATED;

					SphAttr_t uValue = dBigints.BitGet ( iCol ) ? MVA_UPSIZE ( &tUpd.m_dPool[iPos] ) : tUpd.m_dPool[iPos];
					if ( dBigint2Float.BitGet(iCol) ) // handle bigint(-1) -> float attr updates
						uValue = sphF2DW ( float((int64_t)uValue) );
					else if ( dFloat2Bigint.BitGet(iCol) ) // handle float(1.0) -> bigint attr updates
						uValue = (int64_t)sphDW2F((DWORD)uValue);

					sphSetRowAttr ( const_cast<CSphRowitem *>( pRow ), dLocators[iCol], uValue );

					iPos += dBigints.BitGet ( iCol ) ? 2 : 1;
				} else
				{
					const DWORD * pSrc = tUpd.m_dPool.Begin()+iPos;
					DWORD iLen = *pSrc;
					iPos += iLen+1;

					// MVA update
					bUpdated = true;
					uUpdateMask |= ATTRS_MVA_UPDATED;

					if ( !iLen )
					{
						sphSetRowAttr ( const_cast<CSphRowitem *>( pRow ), dLocators[iCol], 0 );
						continue;
					}

					bool bDst64 = ( ( uDst64 & ( U64C(1) << iCol ) )!=0 );
					assert ( ( iLen%2 )==0 );
					DWORD uCount = ( bDst64 ? iLen : iLen/2 );

					CSphTightVector<DWORD> & dStorageMVA = pSegment->m_dMvas;
					DWORD uMvaOff = MVA_DOWNSIZE ( sphGetRowAttr ( pRow, dLocators[iCol] ) );
					assert ( uMvaOff<(DWORD)dStorageMVA.GetLength() );
					DWORD * pDst = dStorageMVA.Begin() + uMvaOff;
					if ( uCount>(*pDst) )
					{
						uMvaOff = dStorageMVA.GetLength();
						dStorageMVA.Resize ( uMvaOff+uCount+1 );
						pDst = dStorageMVA.Begin()+uMvaOff;
						sphSetRowAttr ( const_cast<CSphRowitem *>( pRow ), dLocators[iCol], uMvaOff );
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

			if ( bUpdated )
				iUpdated++;

			break;
		}
		if ( bUpdated )
			continue;

		// check disk K-list now
		const SphDocID_t uRef = ( tUpd.m_dRows[iUpd] ? DOCINFO2ID ( tUpd.m_dRows[iUpd] ) : tUpd.m_dDocids[iUpd] );
		if ( dKilled.BinarySearch ( uRef )!=NULL )
			continue;

		// finally, try disk chunks
		for ( int iChunk = tGuard.m_dDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
		{
			// run just this update
			// FIXME! might be inefficient in case of big batches (redundant allocs in disk update)
			int iRes = const_cast<CSphIndex *>( tGuard.m_dDiskChunks[iChunk] )->UpdateAttributes ( tUpd, iUpd, sError, sWarning );

			// errors are highly unlikely at this point
			// FIXME! maybe emit a warning to client as well?
			if ( iRes<0 )
			{
				sphWarn ( "INTERNAL ERROR: index %s chunk %d update failure: %s", m_sIndexName.cstr(), iChunk, sError.cstr() );
				continue;
			}

			// update stats
			iUpdated += iRes;
			m_uDiskAttrStatus |= tGuard.m_dDiskChunks[iChunk]->GetAttributeStatus();

			// we only need to update the most fresh chunk
			if ( iRes>0 )
				break;
		}
	}

	// bump the counter, binlog the update!
	assert ( iIndex<0 );
	g_pBinlog->BinlogUpdateAttributes ( &m_iTID, m_sIndexName.cstr(), tUpd );

	if ( iJsonWarnings>0 )
	{
		sWarning.SetSprintf ( "%d attribute(s) can not be updated (not found or incompatible types)", iJsonWarnings );
		if ( iUpdated==0 )
		{
			sError = sWarning;
			return -1;
		}
	}

	// all done
	return iUpdated;
}


bool RtIndex_t::SaveAttributes ( CSphString & sError ) const
{
	if ( !m_dDiskChunks.GetLength() )
		return true;

	DWORD uStatus = m_uDiskAttrStatus;
	bool bAllSaved = true;

	SphChunkGuard_t tGuard;
	GetReaderChunks ( tGuard );

	ARRAY_FOREACH ( i, tGuard.m_dDiskChunks )
	{
		bAllSaved &= tGuard.m_dDiskChunks[i]->SaveAttributes ( sError );
	}

	if ( uStatus==m_uDiskAttrStatus )
		m_uDiskAttrStatus = 0;

	return bAllSaved;
}


struct SphOptimizeGuard_t : ISphNoncopyable
{
	CSphMutex &			m_tLock;
	volatile bool &		m_bOptimizeStop;

	SphOptimizeGuard_t ( CSphMutex & tLock, volatile bool & bOptimizeStop )
		: m_tLock ( tLock )
		, m_bOptimizeStop ( bOptimizeStop )
	{
		bOptimizeStop = true;
		m_tLock.Lock();
	}

	~SphOptimizeGuard_t ()
	{
		m_bOptimizeStop = false;
		m_tLock.Unlock();
	}
};


bool RtIndex_t::AddRemoveAttribute ( bool bAdd, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError )
{
	if ( m_dDiskChunks.GetLength() && !m_tSchema.GetAttrsCount() )
	{
		sError = "index must already have attributes";
		return false;
	}

	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop ); // got write-locked at daemon

	int iOldStride = m_iStride;
	const CSphColumnInfo * pNewAttr = NULL;
	CSphSchema tOldSchema = m_tSchema;

	if ( bAdd )
	{
		CSphColumnInfo tInfo ( sAttrName.cstr(), eAttrType );
		m_tSchema.AddAttr ( tInfo, false );
		pNewAttr = m_tSchema.GetAttr ( sAttrName.cstr() );
	} else
		m_tSchema.RemoveAttr ( sAttrName.cstr(), false );

	m_iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();

	// modify the in-memory data of disk chunks
	// fixme: we can't rollback in-memory changes, so we just show errors here for now
	ARRAY_FOREACH ( iDiskChunk, m_dDiskChunks )
		if ( !m_dDiskChunks[iDiskChunk]->AddRemoveAttribute ( bAdd, sAttrName, eAttrType, sError ) )
			sphWarning ( "%s attribute to %s.%d: %s", bAdd ? "adding" : "removing", m_sPath.cstr(), iDiskChunk+m_iDiskBase, sError.cstr() );

	// now modify the ramchunk
	ARRAY_FOREACH ( iSegment, m_dRamChunks )
	{
		RtSegment_t * pSeg = m_dRamChunks[iSegment];
		assert ( pSeg );
		CSphTightVector<CSphRowitem> dNewRows;
		dNewRows.Resize ( pSeg->m_dRows.GetLength() / iOldStride * m_iStride );
		CSphRowitem * pOldDocinfo = pSeg->m_dRows.Begin();
		CSphRowitem * pOldDocinfoEnd = pOldDocinfo+pSeg->m_dRows.GetLength();
		CSphRowitem * pNewDocinfo = dNewRows.Begin();

		if ( bAdd )
		{
			while ( pOldDocinfo < pOldDocinfoEnd )
			{
				SphDocID_t uDocId = DOCINFO2ID ( pOldDocinfo );
				DWORD * pAttrs = DOCINFO2ATTRS ( pOldDocinfo );
				memcpy ( DOCINFO2ATTRS ( pNewDocinfo ), pAttrs, m_tSchema.GetRowSize()*sizeof(CSphRowitem) );
				sphSetRowAttr ( DOCINFO2ATTRS ( pNewDocinfo ), pNewAttr->m_tLocator, 0 );
				DOCINFOSETID ( pNewDocinfo, uDocId );
				pOldDocinfo += iOldStride;
				pNewDocinfo += m_iStride;
			}
		} else
		{
			int iAttrToRemove = tOldSchema.GetAttrIndex ( sAttrName.cstr() );

			CSphVector<int> dAttrMap;
			dAttrMap.Resize ( tOldSchema.GetAttrsCount() );
			for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
				if ( iAttr!=iAttrToRemove )
				{
					dAttrMap[iAttr] = m_tSchema.GetAttrIndex ( tOldSchema.GetAttr ( iAttr ).m_sName.cstr() );
					assert ( dAttrMap[iAttr]>=0 );
				} else
					dAttrMap[iAttr] = -1;

			while ( pOldDocinfo < pOldDocinfoEnd )
			{
				DWORD * pOldAttrs = DOCINFO2ATTRS ( pOldDocinfo );
				DWORD * pNewAttrs = DOCINFO2ATTRS ( pNewDocinfo );

				for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
					if ( iAttr!=iAttrToRemove )
					{
						SphAttr_t tValue = sphGetRowAttr ( pOldAttrs, tOldSchema.GetAttr ( iAttr ).m_tLocator );
						sphSetRowAttr ( pNewAttrs, m_tSchema.GetAttr ( dAttrMap[iAttr] ).m_tLocator, tValue );
					}

				DOCINFOSETID ( pNewDocinfo, DOCINFO2ID ( pOldDocinfo ) );

				pOldDocinfo += iOldStride;
				pNewDocinfo += m_iStride;
			}
		}

		pSeg->m_dRows.SwapData ( dNewRows );
	}

	// fixme: we can't rollback at this point
	Verify ( SaveRamChunk () );

	SaveMeta ( m_dDiskChunks.GetLength(), m_iTID );

	// fixme: notify that it was ALTER that caused the flush
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// MAGIC CONVERSIONS
//////////////////////////////////////////////////////////////////////////

bool RtIndex_t::AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError )
{
	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop ); // got write-locked at daemon

	bool bEmptyRT = ( !m_dRamChunks.GetLength() && !m_dDiskChunks.GetLength() );

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
	// ATTACH to exist index require these checks
	if ( !bEmptyRT )
	{
		if ( m_pTokenizer->GetSettingsFNV()!=pIndex->GetTokenizer()->GetSettingsFNV() )
			LOC_ERROR ( "ATTACH currently requires same tokenizer settings (RT-side support not implemented yet)" );
		if ( m_pDict->GetSettingsFNV()!=pIndex->GetDictionary()->GetSettingsFNV() )
			LOC_ERROR ( "ATTACH currently requires same dictionary settings (RT-side support not implemented yet)" );
		if ( !GetMatchSchema().CompareTo ( pIndex->GetMatchSchema(), sError, true ) )
			LOC_ERROR ( "ATTACH currently requires same attributes declaration (RT-side support not implemented yet)" );
	}
#undef LOC_ERROR

	if ( !bEmptyRT )
	{
		SphAttr_t * pIndexDocList = NULL;
		int64_t iCount = 0;
		if ( !pIndex->BuildDocList ( &pIndexDocList, &iCount, &sError ) )
		{
			sError.SetSprintf ( "ATTACH failed, %s", sError.cstr() );
			return false;
		}

		// new[] might fail on 32bit here
		// sphSort is 32bit too
		int64_t iSizeMax = (size_t)( iCount + pIndex->GetKillListSize() );
		if ( iCount + pIndex->GetKillListSize()!=iSizeMax )
		{
			SafeDeleteArray ( pIndexDocList );
			sError.SetSprintf ( "ATTACH failed, documents overflow (count="INT64_FMT", size max="INT64_FMT")", iCount + pIndex->GetKillListSize(), iSizeMax );
			return false;
		}

		SphDocID_t * pCombined = new SphDocID_t[(size_t)( iCount + pIndex->GetKillListSize() )];
		memcpy ( pCombined, pIndexDocList, (size_t)( sizeof(SphDocID_t) * iCount ) );
		memcpy ( pCombined+iCount, pIndex->GetKillList(), sizeof(SphDocID_t) * pIndex->GetKillListSize() );
		iCount += pIndex->GetKillListSize();
		SafeDeleteArray ( pIndexDocList );

		m_dDiskChunkKlist.Resize ( 0 );
		m_tKlist.Flush ( m_dDiskChunkKlist );
		SphChunkGuard_t tGuard;
		GetReaderChunks ( tGuard );

		ChunkStats_t s ( m_tStats, m_dFieldLensRam );
		SaveDiskChunk ( m_iTID, tGuard, s );

		// kill-list drying up
		for ( int iIndex=m_dDiskChunks.GetLength()-1; iIndex>=0 && iCount; iIndex-- )
		{
			const CSphIndex * pDiskIndex = m_dDiskChunks[iIndex];
			for ( int i=0; i<iCount; i++ )
			{
				SphDocID_t uDocid = pCombined[i];
				if ( !pDiskIndex->HasDocid ( (SphDocID_t)uDocid ) )
					continue;

				// we just found the most recent chunk with our suspect docid
				// let's check whether it's already killed by subsequent chunks, or gets killed now
				bool bKeep = true;
				for ( int k=i+1; k<m_dDiskChunks.GetLength() && bKeep; k++ )
				{
					const CSphIndex * pKilled = m_dDiskChunks[k];
					bKeep = ( sphBinarySearch ( pKilled->GetKillList(), pKilled->GetKillList() + pKilled->GetKillListSize() - 1, uDocid )==NULL );
				}

				if ( !bKeep )
				{
					// RemoveFast
					pCombined[i] = pCombined[iCount-1];
					iCount--;
				}
			}
		}

		// sort by id and got rid of duplicates
		sphSort ( pCombined, (int)iCount );
		iCount = sphUniq ( pCombined, iCount );

		iSizeMax = (size_t)iCount;
		if ( iCount!=iSizeMax )
		{
			SafeDeleteArray ( pCombined );
			sError.SetSprintf ( "ATTACH failed, kill-list overflow (size="INT64_FMT", size max="INT64_FMT")", iCount, iSizeMax );
			return false;
		}

		bool bKillListDone = pIndex->ReplaceKillList ( pCombined, (int)iCount );
		SafeDeleteArray ( pCombined );

		if ( !bKillListDone )
		{
			sError.SetSprintf ( "ATTACH failed, kill-list replacement error (error='%s', warning='%s'", pIndex->GetLastError().cstr(), pIndex->GetLastWarning().cstr() );
			return false;
		}
	}

	// rename that source index to our last chunk
	CSphString sChunk;
	sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), m_dDiskChunks.GetLength()+m_iDiskBase );
	if ( !pIndex->Rename ( sChunk.cstr() ) )
	{
		sError.SetSprintf ( "ATTACH failed, %s", pIndex->GetLastError().cstr() );
		return false;
	}

	// copy schema from new index
	m_tSchema = pIndex->GetMatchSchema();
	m_iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	m_tStats.m_iTotalBytes += pIndex->GetStats().m_iTotalBytes;
	m_tStats.m_iTotalDocuments += pIndex->GetStats().m_iTotalDocuments;

	// copy tokenizer, dict etc settings from new index
	SafeDelete ( m_pTokenizer );
	SafeDelete ( m_pDict );

	m_tSettings = pIndex->GetSettings();
	m_tSettings.m_dBigramWords.Reset();
	m_tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;

	m_pTokenizer = pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX );
	m_pDict = pIndex->GetDictionary()->Clone ();
	PostSetup();
	CSphString sName;
	sName.SetSprintf ( "%s_%d", m_sIndexName.cstr(), m_dDiskChunks.GetLength() );
	pIndex->SetName ( sName.cstr() );
	pIndex->SetBinlog ( false );

	// FIXME? what about copying m_TID etc?

	// recreate disk chunk list, resave header file
	m_dDiskChunks.Add ( pIndex );
	SaveMeta ( m_dDiskChunks.GetLength(), m_iTID );

	// FIXME? do something about binlog too?
	// g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// all done, reset cache
	QcacheDeleteIndex ( GetIndexId() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// TRUNCATE
//////////////////////////////////////////////////////////////////////////

bool RtIndex_t::Truncate ( CSphString & )
{
	// TRUNCATE needs an exclusive lock, should be write-locked at daemon, conflicts only with optimize
	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop );

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
	ARRAY_FOREACH ( i, m_dDiskChunks )
	{
		CSphVector<CSphString> v;
		const char * sChunkFilename = m_dDiskChunks[i]->GetFilename();
		sphSplit ( v, sChunkFilename, "." ); // split something like "rt.1"
		const char * sChunkNumber = v.Last().cstr();
		sFile.SetSprintf ( "%s.%s", m_sPath.cstr(), sChunkNumber );
		sphUnlinkIndex ( sFile.cstr(), false );
	}

	// kill in-memory data, reset stats
	ARRAY_FOREACH ( i, m_dDiskChunks )
		SafeDelete ( m_dDiskChunks[i] );
	m_dDiskChunks.Reset();

	ARRAY_FOREACH ( i, m_dRamChunks )
		SafeDelete ( m_dRamChunks[i] );
	m_dRamChunks.Reset();

	// we don't want kill list to work if we perform ATTACH right after this TRUNCATE
	m_tKlist.Reset ( NULL, 0 );

	// reset cache
	QcacheDeleteIndex ( GetIndexId() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// OPTIMIZE
//////////////////////////////////////////////////////////////////////////

void RtIndex_t::Optimize ( volatile bool * pForceTerminate, ThrottleState_t * pThrottle )
{
	assert ( pForceTerminate && pThrottle );
	int64_t tmStart = sphMicroTimer();

	CSphScopedLock<CSphMutex> tOptimizing ( m_tOptimizingLock );
	m_bOptimizing = true;

	int iChunks = m_dDiskChunks.GetLength();
	CSphSchema tSchema = m_tSchema;
	CSphString sError;

	while ( m_dDiskChunks.GetLength()>1 && !*pForceTerminate && !m_bOptimizeStop )
	{
		CSphVector<SphDocID_t> dKlist;

		// make kill-list
		// initially add RAM kill-list
		dKlist.Resize ( 0 );
		m_tKlist.Flush ( dKlist );

		Verify ( m_tChunkLock.ReadLock () );

		// merge 'older'(pSrc) to 'oldest'(pDst) and get 'merged' that names like 'oldest'+.tmp
		// to got rid of keeping actual kill-list
		// however 'merged' got placed at 'older' position and 'merged' renamed to 'older' name
		const CSphIndex * pOldest = m_dDiskChunks[0];
		const CSphIndex * pOlder = m_dDiskChunks[1];

		// add disk chunks kill-lists
		for ( int iChunk=1; iChunk<m_dDiskChunks.GetLength(); iChunk++ )
		{
			if ( *pForceTerminate || m_bOptimizeStop )
				break;

			const CSphIndex * pIndex = m_dDiskChunks[iChunk];
			if ( !pIndex->GetKillListSize() )
				continue;

			int iOff = dKlist.GetLength();
			dKlist.Resize ( iOff+pIndex->GetKillListSize() );
			memcpy ( dKlist.Begin()+iOff, pIndex->GetKillList(), sizeof(SphDocID_t)*pIndex->GetKillListSize() );
		}
		Verify ( m_tChunkLock.Unlock() );

		dKlist.Add ( 0 );
		dKlist.Add ( DOCID_MAX );
		dKlist.Uniq();

		CSphString sOlder, sOldest, sRename, sMerged;
		sOlder.SetSprintf ( "%s", pOlder->GetFilename() );
		sOldest.SetSprintf ( "%s", pOldest->GetFilename() );
		sRename.SetSprintf ( "%s.old", pOlder->GetFilename() );
		sMerged.SetSprintf ( "%s.tmp", pOldest->GetFilename() );

		// check forced exit after long operation
		if ( *pForceTerminate || m_bOptimizeStop )
			break;

		// merge data to disk ( data is constant during that phase )
		CSphIndexProgress tProgress;
		bool bMerged = sphMerge ( pOldest, pOlder, dKlist, sError, tProgress, pThrottle, pForceTerminate, &m_bOptimizeStop );
		if ( !bMerged )
		{
			sphWarning ( "rt optimize: index %s: failed to merge %s to %s (error %s)",
				m_sIndexName.cstr(), sOlder.cstr(), sOldest.cstr(), sError.cstr() );
			break;
		}
		// check forced exit after long operation
		if ( *pForceTerminate || m_bOptimizeStop )
			break;

		CSphScopedPtr<CSphIndex> pMerged ( LoadDiskChunk ( sMerged.cstr(), sError ) );
		if ( !pMerged.Ptr() )
		{
			sphWarning ( "rt optimize: index %s: failed to load merged chunk (error %s)",
				m_sIndexName.cstr(), sError.cstr() );
			break;
		}
		// check forced exit after long operation
		if ( *pForceTerminate || m_bOptimizeStop )
			break;

		// lets rotate indexes

		// rename older disk chunk to 'old'
		if ( !const_cast<CSphIndex *>( pOlder )->Rename ( sRename.cstr() ) )
		{
			sphWarning ( "rt optimize: index %s: cur to old rename failed (error %s)",
				m_sIndexName.cstr(), pOlder->GetLastError().cstr() );
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
			break;
		}

		if ( *pForceTerminate || m_bOptimizeStop ) // protection
			break;

		Verify ( m_tWriting.Lock() );
		Verify ( m_tChunkLock.WriteLock() );

		m_dDiskChunks[1] = pMerged.LeakPtr();
		m_dDiskChunks.Remove ( 0 );
		m_iDiskBase++;
		int iDiskChunksCount = m_dDiskChunks.GetLength();

		Verify ( m_tChunkLock.Unlock() );
		SaveMeta ( iDiskChunksCount, m_iTID );
		Verify ( m_tWriting.Unlock() );

		if ( *pForceTerminate || m_bOptimizeStop )
		{
			sphWarning ( "rt optimize: index %s: forced to shutdown, remove old index files manually '%s', '%s'",
				m_sIndexName.cstr(), sRename.cstr(), sOldest.cstr() );
			break;
		}

		// exclusive reader (to make sure that disk chunks not used any more) and writer lock here
		Verify ( m_tReading.WriteLock() );
		Verify ( m_tWriting.Lock() );

		SafeDelete ( pOlder );
		SafeDelete ( pOldest );

		Verify ( m_tWriting.Unlock() );
		Verify ( m_tReading.Unlock() );

		// we might remove old index files
		sphUnlinkIndex ( sRename.cstr(), true );
		sphUnlinkIndex ( sOldest.cstr(), true );
		// FIXEME: wipe out 'merged' index files in case of error
	}

	m_bOptimizing = false;
	int64_t tmPass = sphMicroTimer() - tmStart;

	if ( *pForceTerminate )
	{
		sphWarning ( "rt: index %s: optimization terminated chunk(s) %d ( of %d ) in %d.%03d sec",
			m_sIndexName.cstr(), iChunks-m_dDiskChunks.GetLength(), iChunks, (int)(tmPass/1000000), (int)((tmPass/1000)%1000) );
	} else
	{
		sphInfo ( "rt: index %s: optimized chunk(s) %d ( of %d ) in %d.%03d sec",
			m_sIndexName.cstr(), iChunks-m_dDiskChunks.GetLength(), iChunks, (int)(tmPass/1000000), (int)((tmPass/1000)%1000) );
	}
}


//////////////////////////////////////////////////////////////////////////
// STATUS
//////////////////////////////////////////////////////////////////////////

void RtIndex_t::GetStatus ( CSphIndexStatus * pRes ) const
{
	assert ( pRes );
	if ( !pRes )
		return;

	Verify ( m_tChunkLock.ReadLock() );

	pRes->m_iRamChunkSize = GetUsedRam()
		+ m_dRamChunks.GetSizeBytes()
		+ m_dRamChunks.GetLength()*int(sizeof(RtSegment_t))
		+ m_dNewSegmentKlist.GetSizeBytes();

	pRes->m_iRamUse = sizeof(RtIndex_t)
		+ m_dDiskChunkKlist.GetSizeBytes()
		+ m_dDiskChunks.GetSizeBytes()
		+ pRes->m_iRamChunkSize;

	pRes->m_iMemLimit = m_iSoftRamLimit;
	pRes->m_iDiskUse = 0;

	CSphString sError;
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sFiles[] = { ".meta", ".kill", ".ram" };
	for ( int i=0; i<int(sizeof(sFiles)/sizeof(sFiles[0])); i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", m_sFilename.cstr(), sFiles[i] );
		CSphAutofile fdRT ( sFile, SPH_O_READ, sError );
		int64_t iFileSize = fdRT.GetSize();
		if ( iFileSize>0 )
			pRes->m_iDiskUse += iFileSize;
	}
	CSphIndexStatus tDisk;
	ARRAY_FOREACH ( i, m_dDiskChunks )
	{
		m_dDiskChunks[i]->GetStatus(&tDisk);
		pRes->m_iRamUse += tDisk.m_iRamUse;
		pRes->m_iDiskUse += tDisk.m_iDiskUse;
	}

	pRes->m_iNumChunks = m_dDiskChunks.GetLength();

	Verify ( m_tChunkLock.Unlock() );
}

//////////////////////////////////////////////////////////////////////////
// RECONFIGURE
//////////////////////////////////////////////////////////////////////////

bool RtIndex_t::IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError ) const
{
	// FIXME!!! check missed embedded files
	CSphScopedPtr<ISphTokenizer> tTokenizer ( ISphTokenizer::Create ( tSettings.m_tTokenizer, NULL, sError ) );
	if ( !tTokenizer.Ptr() )
	{
		sError.SetSprintf ( "'%s' failed to create tokenizer, error '%s'", m_sIndexName.cstr(), sError.cstr() );
		return true;
	}

	// multiforms
	tTokenizer = ISphTokenizer::CreateMultiformFilter ( tTokenizer.LeakPtr(), m_pDict->GetMultiWordforms() );

	// bigram filter
	if ( tSettings.m_tIndex.m_eBigramIndex!=SPH_BIGRAM_NONE && tSettings.m_tIndex.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		tTokenizer->SetBuffer ( (BYTE*)tSettings.m_tIndex.m_sBigramWords.cstr(), tSettings.m_tIndex.m_sBigramWords.Length() );

		BYTE * pTok = NULL;
		while ( ( pTok = tTokenizer->GetToken() )!=NULL )
			tSettings.m_tIndex.m_dBigramWords.Add() = (const char*)pTok;

		tSettings.m_tIndex.m_dBigramWords.Sort();
	}

	// FIXME!!! check missed embedded files
	CSphScopedPtr<CSphDict> tDict ( sphCreateDictionaryCRC ( tSettings.m_tDict, NULL, tTokenizer.Ptr(), m_sIndexName.cstr(), sError ) );
	if ( !tDict.Ptr() )
	{
		sError.SetSprintf ( "'%s' failed to create dictionary, error '%s'", m_sIndexName.cstr(), sError.cstr() );
		return true;
	}

	bool bNeedExact = ( tDict->HasMorphology() || tDict->GetWordformsFileInfos().GetLength() );
	if ( tSettings.m_tIndex.m_bIndexExactWords && !bNeedExact )
		tSettings.m_tIndex.m_bIndexExactWords = false;

	if ( tDict->GetSettings().m_bWordDict && tDict->HasMorphology() && IsStarDict() && !tSettings.m_tIndex.m_bIndexExactWords )
		tSettings.m_tIndex.m_bIndexExactWords = true;

	// field filter
	CSphScopedPtr<ISphFieldFilter> tFieldFilter ( NULL );

	// re filter
	bool bReFilterSame = true;
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( m_pFieldFilter )
		m_pFieldFilter->GetSettings ( tFieldFilterSettings );
	if ( tFieldFilterSettings.m_dRegexps.GetLength()!=tSettings.m_tFieldFilter.m_dRegexps.GetLength() )
	{
		bReFilterSame = false;
	} else
	{
		CSphVector<uint64_t> dFieldFilter;
		ARRAY_FOREACH ( i, tFieldFilterSettings.m_dRegexps )
			dFieldFilter.Add ( sphFNV64 ( tFieldFilterSettings.m_dRegexps[i].cstr() ) );
		dFieldFilter.Uniq();
		uint64_t uMyFF = sphFNV64 ( dFieldFilter.Begin(), sizeof(dFieldFilter[0]) * dFieldFilter.GetLength() );

		dFieldFilter.Resize ( 0 );
		ARRAY_FOREACH ( i, tSettings.m_tFieldFilter.m_dRegexps )
			dFieldFilter.Add ( sphFNV64 ( tSettings.m_tFieldFilter.m_dRegexps[i].cstr() ) );
		dFieldFilter.Uniq();
		uint64_t uNewFF = sphFNV64 ( dFieldFilter.Begin(), sizeof(dFieldFilter[0]) * dFieldFilter.GetLength() );

		bReFilterSame = ( uMyFF==uNewFF );
	}

	if ( !bReFilterSame && tSettings.m_tFieldFilter.m_dRegexps.GetLength () )
	{
		tFieldFilter = sphCreateRegexpFilter ( tSettings.m_tFieldFilter, sError );
		if ( !tFieldFilter.Ptr() )
		{
			sError.SetSprintf ( "'%s' failed to create field filter, error '%s'", m_sIndexName.cstr (), sError.cstr () );
			return true;
		}
	}

	// rlp filter
	bool bRlpSame = ( m_tSettings.m_eChineseRLP==tSettings.m_tIndex.m_eChineseRLP );
	if ( !bRlpSame )
	{
		ISphFieldFilter * pRlpFilter = tFieldFilter.Ptr();
		bool bOk = sphSpawnRLPFilter ( pRlpFilter, tSettings.m_tIndex, tSettings.m_tTokenizer, m_sIndexName.cstr(), sError );
		if ( !bOk )
		{
			sError.SetSprintf ( "'%s' failed to create field filter, error '%s'", m_sIndexName.cstr (), sError.cstr () );
			return true;
		}

		tFieldFilter = pRlpFilter;
	}

	// compare options
	if ( m_pTokenizer->GetSettingsFNV()!=tTokenizer->GetSettingsFNV() || m_pDict->GetSettingsFNV()!=tDict->GetSettingsFNV() ||
		m_pTokenizer->GetMaxCodepointLength()!=tTokenizer->GetMaxCodepointLength() || sphGetSettingsFNV ( m_tSettings )!=sphGetSettingsFNV ( tSettings.m_tIndex ) ||
		!bReFilterSame || !bRlpSame )
	{
		tSetup.m_pTokenizer = tTokenizer.LeakPtr();
		tSetup.m_pDict = tDict.LeakPtr();
		tSetup.m_tIndex = tSettings.m_tIndex;
		tSetup.m_pFieldFilter = tFieldFilter.LeakPtr();
		return false;
	} else
	{
		return true;
	}
}

void RtIndex_t::Reconfigure ( CSphReconfigureSetup & tSetup )
{
	ForceDiskChunk();

	Setup ( tSetup.m_tIndex );
	SetTokenizer ( tSetup.m_pTokenizer );
	SetDictionary ( tSetup.m_pDict );
	SetFieldFilter ( tSetup.m_pFieldFilter );

	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();
	SetupQueryTokenizer();

	// FIXME!!! handle error
	SafeDelete ( m_pTokenizerIndexing );
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	ISphTokenizer * pIndexing = ISphTokenizer::CreateBigramFilter ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError );
	if ( pIndexing )
		m_pTokenizerIndexing = pIndexing;

	g_pRtBinlog->BinlogReconfigure ( &m_iTID, m_sIndexName.cstr(), tSetup );
	// clean-up
	tSetup.m_pTokenizer = NULL;
	tSetup.m_pDict = NULL;
	tSetup.m_pFieldFilter = NULL;
}

uint64_t sphGetSettingsFNV ( const CSphIndexSettings & tSettings )
{
	uint64_t uHash = 0;

	DWORD uFlags = 0;
	if ( tSettings.m_bHtmlStrip )
		uFlags |= 1<<1;
	if ( tSettings.m_bIndexExactWords )
		uFlags |= 1<<2;
	if ( tSettings.m_bIndexFieldLens )
		uFlags |= 1<<3;
	if ( tSettings.m_bIndexSP )
		uFlags |= 1<<4;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );

	uHash = sphFNV64 ( &tSettings.m_eHitFormat, sizeof(tSettings.m_eHitFormat), uHash );
	uHash = sphFNV64 ( tSettings.m_sHtmlIndexAttrs.cstr(), tSettings.m_sHtmlIndexAttrs.Length(), uHash );
	uHash = sphFNV64 ( tSettings.m_sHtmlRemoveElements.cstr(), tSettings.m_sHtmlRemoveElements.Length(), uHash );
	uHash = sphFNV64 ( tSettings.m_sZones.cstr(), tSettings.m_sZones.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_eHitless, sizeof(tSettings.m_eHitless), uHash );
	uHash = sphFNV64 ( tSettings.m_sHitlessFiles.cstr(), tSettings.m_sHitlessFiles.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_eBigramIndex, sizeof(tSettings.m_eBigramIndex), uHash );
	uHash = sphFNV64 ( tSettings.m_sBigramWords.cstr(), tSettings.m_sBigramWords.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_uAotFilterMask, sizeof(tSettings.m_uAotFilterMask), uHash );
	uHash = sphFNV64 ( &tSettings.m_eChineseRLP, sizeof(tSettings.m_eChineseRLP), uHash );
	uHash = sphFNV64 ( tSettings.m_sRLPContext.cstr(), tSettings.m_sRLPContext.Length(), uHash );
	uHash = sphFNV64 ( tSettings.m_sIndexTokenFilter.cstr(), tSettings.m_sIndexTokenFilter.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_iMinPrefixLen, sizeof(tSettings.m_iMinPrefixLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iMinInfixLen, sizeof(tSettings.m_iMinInfixLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iMaxSubstringLen, sizeof(tSettings.m_iMaxSubstringLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iBoundaryStep, sizeof(tSettings.m_iBoundaryStep), uHash );
	uHash = sphFNV64 ( &tSettings.m_iOvershortStep, sizeof(tSettings.m_iOvershortStep), uHash );
	uHash = sphFNV64 ( &tSettings.m_iStopwordStep, sizeof(tSettings.m_iStopwordStep), uHash );

	return uHash;
}

CSphReconfigureSetup::CSphReconfigureSetup ()
	: m_pTokenizer ( NULL )
	, m_pDict ( NULL )
	, m_pFieldFilter ( NULL )
{}


CSphReconfigureSetup::~CSphReconfigureSetup()
{
	SafeDelete ( m_pTokenizer );
	SafeDelete ( m_pDict );
	SafeDelete ( m_pFieldFilter );
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
	m_iLastCrcPos = 0;
	ResetCrc();
}


void BinlogWriter_c::ResetCrc ()
{
	m_uCRC = ~((DWORD)0);
	m_iLastCrcPos = m_iPoolUsed;
}


void BinlogWriter_c::HashCollected ()
{
	assert ( m_iLastCrcPos<=m_iPoolUsed );

	const BYTE * b = m_pBuffer + m_iLastCrcPos;
	int iSize = m_iPoolUsed - m_iLastCrcPos;
	DWORD uCRC = m_uCRC;

	for ( int i=0; i<iSize; i++ )
		uCRC = (uCRC >> 8) ^ g_dSphinxCRC32 [ (uCRC ^ *b++) & 0xff ];

	m_iLastCrcPos = m_iPoolUsed;
	m_uCRC = uCRC;
}


void BinlogWriter_c::WriteCrc ()
{
	HashCollected();
	m_uCRC = ~m_uCRC;
	CSphWriter::PutDword ( m_uCRC );
	ResetCrc();
}


void BinlogWriter_c::Flush ()
{
	Write();
	Fsync();
	m_iLastCrcPos = m_iPoolUsed;
}


void BinlogWriter_c::Write ()
{
	if ( m_iPoolUsed<=0 )
		return;

	HashCollected();
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

BinlogReader_c::BinlogReader_c()
{
	ResetCrc ();
}

void BinlogReader_c::ResetCrc ()
{
	m_uCRC = ~(DWORD(0));
	m_iLastCrcPos = m_iBuffPos;
}


bool BinlogReader_c::CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos )
{
	HashCollected ();
	DWORD uCRC = ~m_uCRC;
	DWORD uRef = CSphAutoreader::GetDword();
	ResetCrc();
	bool bPassed = ( uRef==uCRC );
	if ( !bPassed )
		sphWarning ( "binlog: %s: CRC mismatch (index=%s, tid="INT64_FMT", pos="INT64_FMT")", sOp, sIndexName ? sIndexName : "", iTid, iTxnPos );
	return bPassed;
}


void BinlogReader_c::UpdateCache ()
{
	HashCollected();
	CSphAutoreader::UpdateCache();
	m_iLastCrcPos = m_iBuffPos;
}

void BinlogReader_c::HashCollected ()
{
	assert ( m_iLastCrcPos<=m_iBuffPos );

	const BYTE * b = m_pBuff + m_iLastCrcPos;
	int iSize = m_iBuffPos - m_iLastCrcPos;
	DWORD uCRC = m_uCRC;

	for ( int i=0; i<iSize; i++ )
		uCRC = (uCRC >> 8) ^ g_dSphinxCRC32 [ (uCRC ^ *b++) & 0xff ];

	m_iLastCrcPos = m_iBuffPos;
	m_uCRC = uCRC;
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
	MEMORY ( MEM_BINLOG );

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
}


void RtBinlog_c::BinlogCommit ( int64_t * pTID, const char * sIndexName, const RtSegment_t * pSeg,
	const CSphVector<SphDocID_t> & dKlist, bool bKeywordDict )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipOffset ( BLOP_COMMIT );
	m_tWriter.ZipOffset ( uIndex );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );

	// save txn data
	if ( !pSeg || !pSeg->m_iRows )
	{
		m_tWriter.ZipOffset ( 0 );
	} else
	{
		m_tWriter.ZipOffset ( pSeg->m_iRows );
		SaveVector ( m_tWriter, pSeg->m_dWords );
		m_tWriter.ZipOffset ( pSeg->m_dWordCheckpoints.GetLength() );
		if ( !bKeywordDict )
		{
			ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
			{
				m_tWriter.ZipOffset ( pSeg->m_dWordCheckpoints[i].m_iOffset );
				m_tWriter.ZipOffset ( pSeg->m_dWordCheckpoints[i].m_uWordID );
			}
		} else
		{
			const char * pBase = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
			ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
			{
				m_tWriter.ZipOffset ( pSeg->m_dWordCheckpoints[i].m_iOffset );
				m_tWriter.ZipOffset ( pSeg->m_dWordCheckpoints[i].m_sWord - pBase );
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

	MEMORY ( MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipOffset ( BLOP_UPDATE_ATTRS );
	m_tWriter.ZipOffset ( uIndex );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );

	// update data
	m_tWriter.ZipOffset ( tUpd.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		m_tWriter.PutString ( tUpd.m_dAttrs[i] );
		m_tWriter.ZipOffset ( tUpd.m_dTypes[i] );
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

void RtBinlog_c::BinlogReconfigure ( int64_t * pTID, const char * sIndexName, const CSphReconfigureSetup & tSetup )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipOffset ( BLOP_RECONFIGURE );
	m_tWriter.ZipOffset ( uIndex );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );

	// reconfigure data
	SaveIndexSettings ( m_tWriter, tSetup.m_tIndex );
	SaveTokenizerSettings ( m_tWriter, tSetup.m_pTokenizer, 0 );
	SaveDictionarySettings ( m_tWriter, tSetup.m_pDict, false, 0 );
	SaveFieldFilterSettings ( m_tWriter, tSetup.m_pFieldFilter );

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

	MEMORY ( MEM_BINLOG );
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
	MEMORY ( MEM_BINLOG );

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
			MEMORY ( MEM_BINLOG );

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
	MEMORY ( MEM_BINLOG );
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

	m_tWriter.ZipOffset ( BLOP_ADD_INDEX );
	m_tWriter.ZipOffset ( iID );
	m_tWriter.PutString ( sName );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );
	m_tWriter.WriteCrc ();

	// return the index
	return iID;
}

void RtBinlog_c::LoadMeta ()
{
	MEMORY ( MEM_BINLOG );

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
	MEMORY ( MEM_BINLOG );

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
	MEMORY ( MEM_BINLOG );

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

	m_tWriter.ZipOffset ( BLOP_ADD_CACHE );
	m_tWriter.ZipOffset ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
	{
		m_tWriter.PutString ( dIndexes[i].m_sName.cstr() );
		m_tWriter.ZipOffset ( dIndexes[i].m_iMinTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_iMaxTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_iFlushedTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_tmMin );
		m_tWriter.ZipOffset ( dIndexes[i].m_tmMax );
	}
	m_tWriter.WriteCrc ();
}

void RtBinlog_c::CheckDoRestart ()
{
	// restart on exceed file size limit
	if ( m_iRestartSize>0 && m_tWriter.GetPos()>m_iRestartSize )
	{
		MEMORY ( MEM_BINLOG );

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
	{
		if ( ( uReplayFlags & SPH_REPLAY_IGNORE_OPEN_ERROR )!=0 )
		{
			sphWarning ( "binlog: log open error: %s", sError.cstr() );
			return 0;
		}
		sphDie ( "binlog: log open error: %s", sError.cstr() );
	}

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
		const uint64_t uOp = tReader.UnzipOffset ();

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

			case BLOP_RECONFIGURE:
				bReplayOK = ReplayReconfigure ( iBinlog, uReplayFlags, tReader );
				break;

			default:
				sphDie ( "binlog: internal error, unhandled entry (blop=%d)", (int)uOp );
		}

		dTotal [ uOp ] += bReplayOK ? 1 : 0;
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

	sphInfo ( "binlog: replay stats: %d rows in %d commits; %d updates, %d reconfigure; %d indexes",
		m_iReplayedRows, dTotal[BLOP_COMMIT], dTotal[BLOP_UPDATE_ATTRS], dTotal[BLOP_RECONFIGURE], dTotal[BLOP_ADD_INDEX] );
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
	const int iVal = (int)tReader.UnzipOffset();

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
	const int64_t iTID = (int64_t) tReader.UnzipOffset();
	const int64_t tmStamp = (int64_t) tReader.UnzipOffset();

	CSphScopedPtr<RtSegment_t> pSeg ( NULL );
	CSphVector<SphDocID_t> dKlist;

	int iRows = (int)tReader.UnzipOffset();
	if ( iRows )
	{
		pSeg = new RtSegment_t();
		pSeg->m_iRows = pSeg->m_iAliveRows = iRows;
		m_iReplayedRows += iRows;

		LoadVector ( tReader, pSeg->m_dWords );
		pSeg->m_dWordCheckpoints.Resize ( (int) tReader.UnzipOffset() ); // FIXME! sanity check
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int) tReader.UnzipOffset();
			pSeg->m_dWordCheckpoints[i].m_uWordID = (SphWordID_t )tReader.UnzipOffset();
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
			tIndex.m_pRT->BuildSegmentInfixes ( pSeg.Ptr(), tIndex.m_pRT->GetDictionary()->HasMorphology() );
		}

		// actually replay
		tIndex.m_pRT->CommitReplayable ( pSeg.LeakPtr(), dKlist, NULL );

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

	uint64_t uVal = tReader.UnzipOffset();
	if ( (int)uVal!=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: indexadd: unexpected index id (id="UINT64_FMT", expected=%d, pos="INT64_FMT")",
			uVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );

	// load data
	CSphString sName = tReader.GetString();

	// FIXME? use this for double checking?
	tReader.UnzipOffset (); // TID
	tReader.UnzipOffset (); // time

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

	int64_t iTID = (int64_t) tReader.UnzipOffset();
	int64_t tmStamp = (int64_t) tReader.UnzipOffset();

	int iAttrs = (int)tReader.UnzipOffset();
	tUpd.m_dAttrs.Resize ( iAttrs ); // FIXME! sanity check
	tUpd.m_dTypes.Resize ( iAttrs ); // FIXME! sanity check
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		tUpd.m_dAttrs[i] = tReader.GetString().Leak();
		tUpd.m_dTypes[i] = (ESphAttr) tReader.UnzipOffset(); // safe, we'll crc check later
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

		tUpd.m_dRows.Resize ( tUpd.m_dDocids.GetLength() );
		ARRAY_FOREACH ( i, tUpd.m_dRows ) tUpd.m_dRows[i] = NULL;

		CSphString sError, sWarning;
		tIndex.m_pIndex->UpdateAttributes ( tUpd, -1, sError, sWarning ); // FIXME! check for errors

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
	dCache.Resize ( (int) tReader.UnzipOffset() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, dCache )
	{
		dCache[i].m_sName = tReader.GetString();
		dCache[i].m_iMinTID = tReader.UnzipOffset();
		dCache[i].m_iMaxTID = tReader.UnzipOffset();
		dCache[i].m_iFlushedTID = tReader.UnzipOffset();
		dCache[i].m_tmMin = tReader.UnzipOffset();
		dCache[i].m_tmMax = tReader.UnzipOffset();
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

bool RtBinlog_c::ReplayReconfigure ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "reconfigure" );

	// load transaction data
	const int64_t iTID = (int64_t) tReader.UnzipOffset();
	const int64_t tmStamp = (int64_t) tReader.UnzipOffset();

	CSphString sError;
	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;
	CSphEmbeddedFiles tEmbeddedFiles;

	CSphReconfigureSettings tSettings;
	LoadIndexSettings ( tSettings.m_tIndex, tReader, INDEX_FORMAT_VERSION );
	if ( !LoadTokenizerSettings ( tReader, tSettings.m_tTokenizer, tEmbeddedFiles, INDEX_FORMAT_VERSION, sError ) )
		sphDie ( "binlog: reconfigure: failed to load settings (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT", error=%s)",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos, sError.cstr() );
	LoadDictionarySettings ( tReader, tSettings.m_tDict, tEmbeddedFiles, INDEX_FORMAT_VERSION, sError );
	LoadFieldFilterSettings ( tReader, tSettings.m_tFieldFilter );

	// checksum
	if ( tReader.GetErrorFlag() || !tReader.CheckCrc ( "reconfigure", tIndex.m_sName.cstr(), iTID, iTxnPos ) )
		return false;

	// check TID
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: reconfigure: descending tid (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );

	// check timestamp
	if ( tmStamp<tIndex.m_tmMax )
	{
		if (!( uReplayFlags & SPH_REPLAY_ACCEPT_DESC_TIMESTAMP ))
			sphDie ( "binlog: reconfigure: descending time (index=%s, lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

		sphWarning ( "binlog: reconfigure: replaying txn despite descending time "
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
			sphWarning ( "binlog: reconfigure: unexpected tid (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos );

		sError = "";
		CSphReconfigureSetup tSetup;
		bool bSame = tIndex.m_pRT->IsSameSettings ( tSettings, tSetup, sError );

		if ( !sError.IsEmpty() )
			sphWarning ( "binlog: reconfigure: wrong settings (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT", error=%s)",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos, sError.cstr() );

		if ( !bSame )
			tIndex.m_pRT->Reconfigure ( tSetup );

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

void RtBinlog_c::CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode )
{
#ifndef DATADIR
#define DATADIR "."
#endif

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : DATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LockFile ( false );
	}
}

//////////////////////////////////////////////////////////////////////////

ISphRtIndex * sphGetCurrentIndexRT()
{
	ISphRtAccum * pAcc = (ISphRtAccum*) sphThreadGet ( g_tTlsAccumKey );
	if ( pAcc )
		return pAcc->GetIndex();
	return NULL;
}

ISphRtIndex * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName,
	int64_t iRamSize, const char * sPath, bool bKeywordDict )
{
	MEMORY ( MEM_INDEX_RT );
	return new RtIndex_t ( tSchema, sIndexName, iRamSize, sPath, bKeywordDict );
}


void sphRTInit ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	MEMORY ( MEM_BINLOG );

	g_bRTChangesAllowed = false;
	Verify ( sphThreadKeyCreate ( &g_tTlsAccumKey ) );

	g_pRtBinlog = new RtBinlog_c();
	if ( !g_pRtBinlog )
		sphDie ( "binlog: failed to create binlog" );
	g_pBinlog = g_pRtBinlog;

	// check binlog path before detaching from the console
	g_pRtBinlog->CheckPath ( hSearchd, bTestMode );
}


void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	assert ( g_pBinlog );
	g_pRtBinlog->Configure ( hSearchd, bTestMode );
	g_iRtFlushPeriod = hSearchd.GetInt ( "rt_flush_period", (int)g_iRtFlushPeriod );
	g_iRtFlushPeriod = Max ( g_iRtFlushPeriod, 10 );
}


void sphRTDone ()
{
	sphThreadKeyDelete ( g_tTlsAccumKey );
	// its valid for "searchd --stop" case
	SafeDelete ( g_pBinlog );
}


void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( MEM_BINLOG );
	g_pRtBinlog->Replay ( hIndexes, uReplayFlags, pfnProgressCallback );
	g_pRtBinlog->CreateTimerThread();
	g_bRTChangesAllowed = true;
}


bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema * pSchema, CSphString * pError )
{
	assert ( pSchema && pError );

	CSphColumnInfo tCol;

	// fields
	SmallStringHash_T<BYTE> hFields;
	for ( CSphVariant * v=hIndex("rt_field"); v; v=v->m_pNext )
	{
		tCol.m_sName = v->cstr();
		tCol.m_sName.ToLower();
		pSchema->m_dFields.Add ( tCol );
		hFields.Add ( 1, tCol.m_sName );
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
	const int iNumTypes = 9;
	const char * sTypes[iNumTypes] = { "rt_attr_uint", "rt_attr_bigint", "rt_attr_float", "rt_attr_timestamp", "rt_attr_string", "rt_attr_multi", "rt_attr_multi_64", "rt_attr_json", "rt_attr_bool" };
	const ESphAttr iTypes[iNumTypes] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_FLOAT, SPH_ATTR_TIMESTAMP, SPH_ATTR_STRING, SPH_ATTR_UINT32SET, SPH_ATTR_INT64SET, SPH_ATTR_JSON, SPH_ATTR_BOOL };

	for ( int iType=0; iType<iNumTypes; iType++ )
	{
		for ( CSphVariant * v = hIndex ( sTypes[iType] ); v; v = v->m_pNext )
		{
			tCol.m_sName = v->cstr();
			tCol.m_sName.ToLower();
			tCol.m_eAttrType = iTypes[iType];

			// bitcount
			tCol.m_tLocator = CSphAttrLocator();
			const char * pColon = strchr ( const_cast<char*> ( tCol.m_sName.cstr() ), ':' );
			if ( pColon )
			{
				if ( tCol.m_eAttrType==SPH_ATTR_INTEGER )
				{
					int iBits = strtol ( pColon+1, NULL, 10 );
					if ( iBits<=0 || iBits>ROWITEM_BITS )
					{
						pError->SetSprintf ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), iBits );
					} else
					{
						tCol.m_tLocator.m_iBitCount = iBits;
					}

				} else
				{
					pError->SetSprintf ( "attribute '%s': bitcount is only supported for integer types (bitcount ignored)", tCol.m_sName.cstr() );
				}

				// trim bitcount from name
				CSphString sName ( tCol.m_sName.cstr(), pColon-tCol.m_sName.cstr() );
				tCol.m_sName.Swap ( sName );
			}

			pSchema->AddAttr ( tCol, false );

			if ( tCol.m_eAttrType!=SPH_ATTR_STRING && hFields.Exists ( tCol.m_sName ) )
			{
				pError->SetSprintf ( "can not add attribute that shadows '%s' field", tCol.m_sName.cstr () );
				return false;
			}
		}
	}

	return true;
}

//
// $Id$
//
