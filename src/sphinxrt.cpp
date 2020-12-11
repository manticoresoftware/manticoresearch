//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
#include "sphinxint.h"
#include "sphinxrt.h"
#include "sphinxpq.h"
#include "sphinxsearch.h"
#include "sphinxutils.h"
#include "fileutils.h"
#include "sphinxjson.h"
#include "sphinxplugin.h"
#include "icu.h"
#include "sphinxqcache.h"
#include "attribute.h"
#include "killlist.h"
#include "secondaryindex.h"
#include "accumulator.h"
#include "indexcheck.h"
#include "indexsettings.h"
#include "indexformat.h"
#include "coroutine.h"
#include "mini_timer.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <atomic>

#if USE_WINDOWS
#include <errno.h>
#else
#include <unistd.h>
#include <sys/time.h>

#endif

//////////////////////////////////////////////////////////////////////////

#define BINLOG_WRITE_BUFFER		256*1024
#define BINLOG_AUTO_FLUSH		1000000 // 1 sec

#define RTDICT_CHECKPOINT_V5			48
#define SPH_RT_DOUBLE_BUFFER_PERCENT	10

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
static auto&	g_bRTChangesAllowed		= RTChangesAllowed ();

// optimize mode for disk chunks merge
static bool g_bProgressiveMerge = true;

//////////////////////////////////////////////////////////////////////////
volatile bool &RTChangesAllowed ()
{
	static bool bRTChangesAllowed = false;
	return bRTChangesAllowed;
}

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

/* // note difference: sphZipValue packs in BE order (most significant bits cames first)
 template<typename T, typename WRITER>
inline int sphZipValue ( WRITER fnPut, T tValue )
{
	int nBytes = sphCalcZippedLen ( tValue );
	for ( int i = nBytes-1; i>=0; --i )
		fnPut ( ( 0x7f & ( tValue >> ( 7 * i ) ) ) | ( i ? 0x80 : 0 ) );

	return nBytes;
}
 */

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );


// Variable Length Byte (VLB) decoding
template < typename T >
static inline const BYTE * UnzipT ( T * pValue, const BYTE * pIn )
{
	T uValue = 0;
	BYTE uIn;
	int iOff = 0;

	do
	{
		uIn = *pIn++;
		uValue += ( T ( uIn & 0x7FU ) ) << iOff;
		iOff += 7;
	} while ( uIn & 0x80U );

	*pValue = uValue;
	return pIn;
}

/* // Note difference: in code below zipped bytes came in BE order (most significant first)
 * #define SPH_VARINT_DECODE(_type,_getexpr) \
	register DWORD b = _getexpr; \
	register _type res = 0; \
	while ( b & 0x80 ) \
	{ \
		res = ( res<<7 ) + ( b & 0x7f ); \
		b = _getexpr; \
	} \
	res = ( res<<7 ) + b; \
	return res;

 */

// Variable Length Byte (VLB) skipping (BE/LE agnostic)
static inline void SkipZipped ( const BYTE *& pIn )
{
	while ( *pIn & 0x80U )
		++pIn;
	++pIn; // jump over last one
}

#define ZipDword ZipT<DWORD>
#define ZipQword ZipT<uint64_t>
#define UnzipDword UnzipT<DWORD>
#define UnzipQword UnzipT<uint64_t>

#define ZipDocid ZipQword
#define ZipWordid ZipQword
#define UnzipWordid UnzipQword

//////////////////////////////////////////////////////////////////////////

struct CmpHitPlain_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b ) const
	{
		return 	( a.m_uWordID<b.m_uWordID ) ||
			( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) ||
			( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && a.m_uWordPos<b.m_uWordPos );
	}
};


struct CmpHitKeywords_fn
{
	const BYTE * m_pBase;
	explicit CmpHitKeywords_fn ( const BYTE * pBase ) : m_pBase ( pBase ) {}
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b ) const
	{
		const BYTE * pPackedA = m_pBase + a.m_uWordID;
		const BYTE * pPackedB = m_pBase + b.m_uWordID;
		int iCmp = sphDictCmpStrictly ( (const char *)pPackedA+1, *pPackedA, (const char *)pPackedB+1, *pPackedB );
		return 	( iCmp<0 ) ||
			( iCmp==0 && a.m_tRowID<b.m_tRowID ) ||
			( iCmp==0 && a.m_tRowID==b.m_tRowID && a.m_uWordPos<b.m_uWordPos );
	}
};


RtSegment_t::RtSegment_t ( DWORD uDocs )
	: m_tDeadRowMap ( uDocs )
{
}

RtSegment_t::~RtSegment_t ()
{
	if ( m_pRAMCounter )
		FixupRAMCounter ( -GetUsedRam() );
}


void RtSegment_t::UpdateUsedRam()
{
	FixupRAMCounter ( -m_iUsedRam );

	m_iUsedRam = 0;
	m_iUsedRam += m_dWords.AllocatedBytes();
	m_iUsedRam += m_dDocs.AllocatedBytes();
	m_iUsedRam += m_dHits.AllocatedBytes();
	m_iUsedRam += m_dBlobs.AllocatedBytes();
	m_iUsedRam += m_dKeywordCheckpoints.AllocatedBytes();
	m_iUsedRam += m_dRows.AllocatedBytes();
	m_iUsedRam += m_dInfixFilterCP.AllocatedBytes();
	m_iUsedRam += m_pDocstore.Ptr() ? m_pDocstore->AllocatedBytes() : 0;

	FixupRAMCounter ( m_iUsedRam );
}


int64_t RtSegment_t::GetUsedRam()
{
	if ( !m_iUsedRam )
		UpdateUsedRam();

	return m_iUsedRam;
}


void RtSegment_t::FixupRAMCounter ( int64_t iDelta ) const
{
	if ( !m_pRAMCounter )
		return;

	m_pRAMCounter->fetch_add ( iDelta, std::memory_order_relaxed );
}

DWORD RtSegment_t::GetMergeFactor() const
{
	return m_uRows;
}

int RtSegment_t::GetStride () const
{
	return ( m_dRows.GetLength() / m_uRows );
}



const CSphRowitem * RtSegment_t::FindAliveRow ( DocID_t tDocid ) const
{
	RowID_t tRowID = GetRowidByDocid(tDocid);
	if ( tRowID==INVALID_ROWID || m_tDeadRowMap.IsSet(tRowID) )
		return nullptr;

	return GetDocinfoByRowID(tRowID);
}


const CSphRowitem * RtSegment_t::GetDocinfoByRowID ( RowID_t tRowID ) const
{
	return &m_dRows[tRowID*GetStride()];
}


RowID_t RtSegment_t::GetRowidByDocid ( DocID_t tDocID ) const
{
	RowID_t * pRowID = m_tDocIDtoRowID.Find(tDocID);
	return pRowID ? *pRowID : INVALID_ROWID;
}


int RtSegment_t::Kill ( DocID_t tDocID )
{
	if ( m_tDeadRowMap.Set ( GetRowidByDocid ( tDocID ) ) )
	{
		assert ( m_tAliveRows>0 );
		m_tAliveRows.fetch_sub ( 1, std::memory_order_relaxed );
		return 1;
	}

	return 0;
}


int	RtSegment_t::KillMulti ( const VecTraits_T<DocID_t> & dKlist )
{
	int iTotalKilled = 0;

	// fixme: implement more efficient batch killer
	for ( auto iDocID : dKlist )
		iTotalKilled += Kill ( iDocID );

	return iTotalKilled;
}


void RtSegment_t::SetupDocstore ( const CSphSchema * pSchema )
{
	assert ( !m_pDocstore.Ptr() );
	m_pDocstore = CreateDocstoreRT();
	assert ( m_pDocstore.Ptr() );

	if ( pSchema )
		SetupDocstoreFields ( *m_pDocstore.Ptr(), *pSchema );
}


void RtSegment_t::BuildDocID2RowIDMap()
{
	m_tDocIDtoRowID.Reset(m_uRows);

	int iStride = GetStride();
	RowID_t tRowID = 0;
	for ( int i=0; i<m_dRows.GetLength(); i+=iStride )
		m_tDocIDtoRowID.Add ( sphGetDocID ( &m_dRows[i] ), tRowID++ );
}


//////////////////////////////////////////////////////////////////////////

struct RtDocWriter_t
{
	CSphTightVector<BYTE> *		m_pDocs;
	RowID_t						m_tLastRowID {INVALID_ROWID};

	explicit RtDocWriter_t ( RtSegment_t * pSeg )
		: m_pDocs ( &pSeg->m_dDocs )
	{}

	void ZipDoc ( const RtDoc_t & tDoc )
	{
		CSphTightVector<BYTE> * pDocs = m_pDocs;
		BYTE * pEnd = pDocs->AddN ( 12*sizeof(DWORD) );
		const BYTE * pBegin = pDocs->Begin();

		ZipDword ( pEnd, tDoc.m_tRowID - m_tLastRowID );
		m_tLastRowID = tDoc.m_tRowID;
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
		m_tLastRowID = INVALID_ROWID;
	}
};


RtDocReader_t::RtDocReader_t ( const RtSegment_t * pSeg, const RtWord_t & tWord )
{
	m_pDocs = ( pSeg->m_dDocs.Begin() ? pSeg->m_dDocs.Begin() + tWord.m_uDoc : NULL );
	m_iLeft = tWord.m_uDocs;
	m_tDoc.m_tRowID = INVALID_ROWID;
}

const RtDoc_t * RtDocReader_t::UnzipDoc ()
{
	if ( !m_iLeft || !m_pDocs )
		return NULL;

	const BYTE * pIn = m_pDocs;
	RowID_t tDeltaID;
	pIn = UnzipDword ( &tDeltaID, pIn );
	m_tDoc.m_tRowID += tDeltaID;
	pIn = UnzipDword ( &m_tDoc.m_uDocFields, pIn );
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
				tCheckpoint.m_uWordID = tWord.m_uWordID;
			else
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

		BYTE * pEnd = pWords->AddN ( 4*sizeof(DWORD) );
		const BYTE * pBegin = pWords->Begin();

		DWORD uDocs = tWord.m_uDocs;
		if ( !tWord.m_bHasHitlist )
			uDocs |= HITLESS_DOC_FLAG;

		ZipDword ( pEnd, uDocs );
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

RtWordReader_t::RtWordReader_t ( const RtSegment_t * pSeg, bool bWordDict, int iWordsCheckpoint, ESphHitless eHitlessMode )
	: m_bWordDict ( bWordDict )
	, m_iWordsCheckpoint ( iWordsCheckpoint )
	, m_eHitlessMode ( eHitlessMode )
{
	m_tWord.m_uWordID = 0;
	Reset ( pSeg );
	if ( bWordDict )
		m_tWord.m_sWord = m_tPackedWord;
}

void RtWordReader_t::Reset ( const RtSegment_t * pSeg )
{
	m_pCur = pSeg->m_dWords.Begin();
	m_pMax = m_pCur + pSeg->m_dWords.GetLength();

	m_tWord.m_uDoc = 0;
	m_iWords = 0;
}

const RtWord_t * RtWordReader_t::UnzipWord ()
{
	if ( ++m_iWords==m_iWordsCheckpoint )
	{
		m_tWord.m_uDoc = 0;
		m_iWords = 1;
		++m_iCheckpoint;
		if ( !m_bWordDict )
			m_tWord.m_uWordID = 0;
	}
	if ( m_pCur>=m_pMax )
		return nullptr;

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
		m_tWord.m_uWordID += uDeltaID;
	}
	pIn = UnzipDword ( &m_tWord.m_uDocs, pIn );
	pIn = UnzipDword ( &m_tWord.m_uHits, pIn );
	pIn = UnzipDword ( &uDeltaDoc, pIn );
	m_pCur = pIn;

	m_tWord.m_bHasHitlist = ( m_eHitlessMode==SPH_HITLESS_NONE || ( m_eHitlessMode==SPH_HITLESS_SOME && !( m_tWord.m_uDocs & HITLESS_DOC_FLAG ) ) );
	m_tWord.m_uDocs = ( m_eHitlessMode==SPH_HITLESS_NONE ? m_tWord.m_uDocs : ( m_tWord.m_uDocs & HITLESS_DOC_MASK ) );

	m_tWord.m_uDoc += uDeltaDoc;
	return &m_tWord;
}


struct RtHitWriter_t
{
	CSphTightVector<BYTE> *		m_pHits;
	DWORD						m_uLastHit = 0;

	explicit RtHitWriter_t ( RtSegment_t * pSeg )
		: m_pHits ( &pSeg->m_dHits )
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

RtHitReader_t::RtHitReader_t ( const RtSegment_t * pSeg, const RtDoc_t * pDoc )
{
	m_pCur = pSeg->m_dHits.Begin() + pDoc->m_uHit;
	m_iLeft = pDoc->m_uHits;
	m_uLast = 0;
}

DWORD RtHitReader_t::UnzipHit ()
{
	if ( !m_iLeft )
		return 0;

	DWORD uValue;
	m_pCur = UnzipDword ( &uValue, m_pCur );
	m_uLast += uValue;
	--m_iLeft;
	return m_uLast;
}

ByteBlob_t RtHitReader_t::GetHitsBlob () const
{
	const BYTE * pEnd = m_pCur;
	for ( auto i = 0U; i<m_iLeft; ++i )
		SkipZipped (pEnd);
	return { m_pCur, pEnd-m_pCur };
}

void RtHitReader2_t::Seek ( SphOffset_t uOff, int iHits )
{
	m_pCur = m_pBase + uOff;
	m_iLeft = iHits;
	m_uLast = 0;
}

//////////////////////////////////////////////////////////////////////////

static const int PQ_META_VERSION_MAX = 255;

uint64_t MemoryReader_c::UnzipOffset()
{
	assert ( m_pCur );
	assert ( m_pCur<m_pData+m_iLen );
	uint64_t uVal = 0;
	m_pCur = UnzipQword ( &uVal, m_pCur );
	return uVal;
}

DWORD MemoryReader_c::UnzipInt()
{
	assert ( m_pCur );
	assert ( m_pCur<m_pData+m_iLen );
	DWORD uVal = 0;
	m_pCur = UnzipDword ( &uVal, m_pCur );
	return uVal;
}

void MemoryWriter_c::ZipOffset ( uint64_t uVal )
{
	ZipQword ( &m_dBuf, uVal );
}

void MemoryWriter_c::ZipInt ( DWORD uVal )
{
	ZipDword ( &m_dBuf, uVal );
}

void LoadStoredQueryV6 ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader )
{
	if ( uVersion>=3 )
		tQuery.m_iQUID = tReader.GetOffset();
	if ( uVersion>=4 )
		tQuery.m_bQL = ( tReader.GetDword()!=0 );

	tQuery.m_sQuery = tReader.GetString();
	if ( uVersion==1 )
		return;

	tQuery.m_sTags = tReader.GetString();

	tQuery.m_dFilters.Resize ( tReader.GetDword() );
	tQuery.m_dFilterTree.Resize ( tReader.GetDword() );
	for ( auto& tFilter : tQuery.m_dFilters )
	{
		tFilter.m_sAttrName = tReader.GetString();
		tFilter.m_bExclude = ( tReader.GetDword()!=0 );
		tFilter.m_bHasEqualMin = ( tReader.GetDword()!=0 );
		tFilter.m_bHasEqualMax = ( tReader.GetDword()!=0 );
		tFilter.m_eType = (ESphFilter)tReader.GetDword();
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReader.GetDword ();
		tReader.GetBytes ( &tFilter.m_iMinValue, sizeof(tFilter.m_iMinValue) );
		tReader.GetBytes ( &tFilter.m_iMaxValue, sizeof(tFilter.m_iMaxValue) );
		tFilter.m_dValues.Resize ( tReader.GetDword() );
		tFilter.m_dStrings.Resize ( tReader.GetDword() );
		for ( auto& dValue : tFilter.m_dValues )
			tReader.GetBytes ( &dValue, sizeof ( dValue ) );
		for ( auto& dString : tFilter.m_dStrings )
			dString = tReader.GetString ();
	}
	for ( auto & tItem : tQuery.m_dFilterTree )
	{
		tItem.m_iLeft = tReader.GetDword();
		tItem.m_iRight = tReader.GetDword();
		tItem.m_iFilterItem = tReader.GetDword();
		tItem.m_bOr = ( tReader.GetDword()!=0 );
	}
}

template<typename READER>
void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, READER & tReader )
{
	assert ( uVersion>=7 );
	tQuery.m_iQUID = tReader.UnzipOffset();
	tQuery.m_bQL = ( tReader.UnzipInt()!=0 );
	tQuery.m_sQuery = tReader.GetString();
	tQuery.m_sTags = tReader.GetString();

	tQuery.m_dFilters.Resize ( tReader.UnzipInt() );
	tQuery.m_dFilterTree.Resize ( tReader.UnzipInt() );
	for ( auto& tFilter : tQuery.m_dFilters )
	{
		tFilter.m_sAttrName = tReader.GetString();
		tFilter.m_bExclude = ( tReader.UnzipInt()!=0 );
		tFilter.m_bHasEqualMin = ( tReader.UnzipInt()!=0 );
		tFilter.m_bHasEqualMax = ( tReader.UnzipInt()!=0 );
		tFilter.m_bOpenLeft = ( tReader.UnzipInt()!=0 );
		tFilter.m_bOpenRight = ( tReader.UnzipInt()!=0 );
		tFilter.m_bIsNull = ( tReader.UnzipInt()!=0 );
		tFilter.m_eType = (ESphFilter)tReader.UnzipInt();
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReader.UnzipInt ();
		tFilter.m_iMinValue = tReader.UnzipOffset();
		tFilter.m_iMaxValue = tReader.UnzipOffset();
		tFilter.m_dValues.Resize ( tReader.UnzipInt() );
		tFilter.m_dStrings.Resize ( tReader.UnzipInt() );
		for ( auto& dValue : tFilter.m_dValues )
			dValue = tReader.UnzipOffset ();
		for ( auto& dString : tFilter.m_dStrings )
			dString = tReader.GetString ();
	}
	for ( auto& tItem : tQuery.m_dFilterTree )
	{
		tItem.m_iLeft = tReader.UnzipInt();
		tItem.m_iRight = tReader.UnzipInt();
		tItem.m_iFilterItem = tReader.UnzipInt();
		tItem.m_bOr = ( tReader.UnzipInt()!=0 );
	}
}

template<typename WRITER>
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, WRITER & tWriter )
{
	tWriter.ZipOffset ( tQuery.m_iQUID );
	tWriter.ZipInt ( tQuery.m_bQL );
	tWriter.PutString ( tQuery.m_sQuery );
	tWriter.PutString ( tQuery.m_sTags );
	tWriter.ZipInt ( tQuery.m_dFilters.GetLength() );
	tWriter.ZipInt ( tQuery.m_dFilterTree.GetLength() );
	ARRAY_FOREACH ( iFilter, tQuery.m_dFilters )
	{
		const CSphFilterSettings & tFilter = tQuery.m_dFilters[iFilter];
		tWriter.PutString ( tFilter.m_sAttrName );
		tWriter.ZipInt ( tFilter.m_bExclude );
		tWriter.ZipInt ( tFilter.m_bHasEqualMin );
		tWriter.ZipInt ( tFilter.m_bHasEqualMax );
		tWriter.ZipInt ( tFilter.m_bOpenLeft );
		tWriter.ZipInt ( tFilter.m_bOpenRight );
		tWriter.ZipInt ( tFilter.m_bIsNull );
		tWriter.ZipInt ( tFilter.m_eType );
		tWriter.ZipInt ( tFilter.m_eMvaFunc );
		tWriter.ZipOffset ( tFilter.m_iMinValue );
		tWriter.ZipOffset ( tFilter.m_iMaxValue );
		tWriter.ZipInt ( tFilter.m_dValues.GetLength() );
		tWriter.ZipInt ( tFilter.m_dStrings.GetLength() );
		ARRAY_FOREACH ( j, tFilter.m_dValues )
			tWriter.ZipOffset ( tFilter.m_dValues[j] );
		ARRAY_FOREACH ( j, tFilter.m_dStrings )
			tWriter.PutString ( tFilter.m_dStrings[j] );
	}
	ARRAY_FOREACH ( iTree, tQuery.m_dFilterTree )
	{
		const FilterTreeItem_t & tItem = tQuery.m_dFilterTree[iTree];
		tWriter.ZipInt ( tItem.m_iLeft );
		tWriter.ZipInt ( tItem.m_iRight );
		tWriter.ZipInt ( tItem.m_iFilterItem );
		tWriter.ZipInt ( tItem.m_bOr );
	}
}

void LoadStoredQuery ( const BYTE * pData, int iLen, StoredQueryDesc_t & tQuery )
{
	MemoryReader_c tReader ( pData, iLen );
	LoadStoredQuery ( PQ_META_VERSION_MAX, tQuery, tReader );
}

void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader )
{
	LoadStoredQuery<CSphReader> ( uVersion, tQuery, tReader );
}

void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphVector<BYTE> & dOut )
{
	MemoryWriter_c tWriter ( dOut );
	SaveStoredQuery ( tQuery, tWriter );
}

void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphWriter & tWriter )
{
	SaveStoredQuery<CSphWriter> ( tQuery, tWriter );
}

template<typename READER>
void LoadDeleteQuery ( CSphVector<int64_t> & dQueries, CSphString & sTags, READER & tReader )
{
	dQueries.Resize ( tReader.UnzipInt() );
	ARRAY_FOREACH ( i, dQueries )
		dQueries[i] = tReader.UnzipOffset();

	sTags = tReader.GetString();
}

void LoadDeleteQuery ( const BYTE * pData, int iLen, CSphVector<int64_t> & dQueries, CSphString & sTags )
{
	MemoryReader_c tReader ( pData, iLen );
	LoadDeleteQuery ( dQueries, sTags, tReader );
}

template<typename WRITER>
void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char* sTags, WRITER& tWriter )
{
	tWriter.ZipInt ( dQueries.GetLength () );
	for ( int64_t iQuery : dQueries )
		tWriter.ZipOffset ( iQuery );

	tWriter.PutString ( sTags );
}

void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char* sTags, CSphVector<BYTE>& dOut )
{
	MemoryWriter_c tWriter ( dOut );
	SaveDeleteQuery ( dQueries, sTags, tWriter );
}


//////////////////////////////////////////////////////////////////////////

/// forward ref
class RtIndex_c;

/// TLS indexing accumulator (we disallow two uncommitted adds within one thread; and so need at most one)
thread_local RtAccum_t* g_pTlsAccum;

/// binlog file view of the index
/// everything that a given log file needs to know about an index
struct BinlogIndexInfo_t
{
	CSphString	m_sName;				///< index name
	int64_t		m_iMinTID = INT64_MAX;	///< min TID logged by this file
	int64_t		m_iMaxTID = 0;			///< max TID logged by this file
	int64_t		m_iFlushedTID = 0;		///< last flushed TID
	int64_t		m_tmMin = INT64_MAX;	///< min TID timestamp
	int64_t		m_tmMax = 0;			///< max TID timestamp

	CSphIndex *	m_pIndex = nullptr;		///< replay only; associated index (might be NULL if we don't serve it anymore!)
	RtIndex_c *	m_pRT = nullptr;		///< replay only; RT index handle (might be NULL if N/A or non-RT)
	PercolateIndex_i *	m_pPQ = nullptr;		///< replay only; PQ index handle (might be NULL if N/A or non-PQ)
	int64_t		m_iPreReplayTID = 0;	///< replay only; index TID at the beginning of this file replay
};

/// binlog file descriptor
/// file id (aka extension), plus a list of associated index infos
struct BinlogFileDesc_t
{
	int								m_iExt = 0;
	CSphVector<BinlogIndexInfo_t>	m_dIndexInfos;
};

/// Bin Log Operation
enum Blop_e
{
	BLOP_COMMIT			= 1,
	BLOP_UPDATE_ATTRS	= 2,
	BLOP_ADD_INDEX		= 3,
	BLOP_ADD_CACHE		= 4,
	BLOP_RECONFIGURE	= 5,
	BLOP_PQ_ADD			= 6,
	BLOP_PQ_DELETE		= 7,

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
	void			UpdateCache () override;
	void			HashCollected ();
};


class RtBinlog_c : public ISphBinlog
{
public:
	RtBinlog_c ();
	~RtBinlog_c ();

	void	BinlogCommit ( int64_t * pTID, const char * sIndexName, const RtSegment_t * pSeg, const CSphVector<DocID_t> & dKlist, bool bKeywordDict );
	void	BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd ) final;
	void	BinlogReconfigure ( int64_t * pTID, const char * sIndexName, const CSphReconfigureSetup & tSetup ) override;
	void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown ) final;
	void	BinlogPqAdd ( int64_t * pTID, const char * sIndexName, const StoredQueryDesc_t & tStored ) override;
	void	BinlogPqDelete ( int64_t* pTID, const char* sIndexName, const VecTraits_T<int64_t>& dQueries, const char* sTags ) override;

	void	Configure ( const CSphConfigSection & hSearchd, bool bTestMode );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback );

	bool	IsActive () override	{ return !m_bDisabled; }
	void	CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode );

	bool	IsFlushingEnabled() const;
	void	DoFlush ();
	int64_t	NextFlushingTime() const;

private:
	struct BlopStartEnd_t
	{
		RtBinlog_c & m_tBinlog;

		BlopStartEnd_t ( RtBinlog_c & tBinlog, int64_t * pTID, Blop_e eBlop, const char * szIndexName );
		~BlopStartEnd_t();
	};

	static const DWORD		BINLOG_VERSION = 9;

	static const DWORD		BINLOG_HEADER_MAGIC = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
	static const DWORD		BLOP_MAGIC = 0x214e5854;			/// magic 'TXN!' header that marks binlog entry
	static const DWORD		BINLOG_META_MAGIC = 0x494c5053;		/// magic 'SPLI' header that marks binlog meta

	volatile int64_t		m_iLastFlushed = 0;
	volatile int64_t		m_iFlushPeriod = BINLOG_AUTO_FLUSH;

	enum OnCommitAction_e
	{
		ACTION_NONE,
		ACTION_FSYNC,
		ACTION_WRITE
	};
	OnCommitAction_e		m_eOnCommit { ACTION_NONE };

	CSphMutex				m_tWriteLock; // lock on operation

	int						m_iLockFD = -1;
	CSphString				m_sWriterError;
	BinlogWriter_c			m_tWriter;

	mutable CSphVector<BinlogFileDesc_t>	m_dLogFiles; // active log files

	CSphString				m_sLogPath;

	bool					m_bReplayMode = false; // replay mode indicator
	bool					m_bDisabled = true;

	int						m_iRestartSize = 268435456; // binlog size restart threshold, 256M

	// replay stats
	mutable int				m_iReplayedRows=0;

private:

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
	bool					ReplayPqAdd ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const;
	bool					ReplayPqDelete ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const;

	bool	PreOp ( Blop_e eOp, int64_t * pTID, const char * sIndexName ) ACQUIRE (m_tWriteLock);
	void	PostOp () RELEASE (m_tWriteLock);
	bool	CheckCrc ( const char * sOp, const CSphString & sIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader ) const;
	void	CheckTid ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos ) const;

	template<typename T>
	void	CheckTidSeq ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, T * pIndexTID, int64_t iTxnPos ) const;
	void	CheckTime ( BinlogIndexInfo_t & tIndex, const char * sOp, int64_t tmStamp, int64_t iTID, int64_t iTxnPos, DWORD uReplayFlags ) const;
	bool	PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, int64_t tmStamp, DWORD uReplayFlags, BinlogReader_c & tReader ) const;
	void	UpdateIndexInfo ( BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t tmStamp ) const;

	void	WriteBlopHeader ( int64_t * pTID, Blop_e eBlop, const char * szIndexName );
};

ISphBinlog * GetBinlog() { return g_pRtBinlog; }

// RAII locks ram segments and kill lists
struct SCOPED_CAPABILITY SphChunkGuard_t : public ISphNoncopyable
{
	CSphFixedVector<RtSegmentRefPtf_t>		m_dRamChunks { 0 };
	CSphFixedVector<const CSphIndex *>		m_dDiskChunks { 0 };
	CSphRwlock *							m_pReading = nullptr;

	~SphChunkGuard_t() RELEASE();

	// moving makes possible return by value
	explicit SphChunkGuard_t ( CSphRwlock * pReading ) ACQUIRE_SHARED ( *pReading );
	SphChunkGuard_t ( SphChunkGuard_t && rhs ) noexcept ACQUIRE ( *rhs.m_pReading );
	SphChunkGuard_t & operator= ( SphChunkGuard_t && rhs ) noexcept RELEASE() ACQUIRE ( *rhs.m_pReading );
	void Swap ( SphChunkGuard_t& rhs) noexcept;
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

template<typename IDX>
CSphFixedVector<int> GetChunkNames ( const VecTraits_T<IDX> & dIndexes )
{
	int iChunks = dIndexes.GetLength();
	CSphFixedVector<int> dNames ( iChunks );
	for ( int i=0; i<iChunks; ++i)
		dNames[i] = dIndexes[i]->m_iChunk;

	return dNames;
}

/// RAM based index
struct RtQword_t;
class RtRowIterator_c;
struct SaveDiskDataContext_t;
using Selector_t = std::function<bool (int*,int*)>;

/* fixme! That zoo of locks in rtindex looks brain-screwing
 *

	mutable CSphRwlock			m_tChunkLock;		// protect both RAM segments and disk chunks
	mutable CSphRwlock			m_tReading;			// reading op over disk chunk. w-lock on drop and merge chunks
 	CSphMutex					m_tWriting;			// commit, drop, save disk chunk, merge uses it
 	CSphMutex					m_tFlushLock;		// when saving ram chunk to .ram
	CSphMutex					m_tOptimizingLock;	// when optimizing is in game
 	CSphMutex					m_tSaveFinished;	// fixed #1137, tricky ping-pong with m_tWritting and double buffer
 *
*/

class RtIndex_c final : public RtIndex_i, public ISphNoncopyable, public ISphWordlist, public ISphWordlistSuggest,
		public IndexUpdateHelper_c, public IndexAlterHelper_c, public DebugCheckHelper_c
{
public:
	explicit			RtIndex_c ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );
						~RtIndex_c () final;

	bool				AddDocument ( const VecTraits_T<VecTraits_T<const char >> &dFields,
		CSphMatch & tDoc, bool bReplace, const CSphString & sTokenFilterOptions, const char ** ppStr,
		const VecTraits_T<int64_t> & dMvas, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt ) override;
	virtual bool		AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const VecTraits_T<int64_t> & dMvas,
		const DocstoreBuilder_i::Doc_t * pStoredDoc, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt );
	bool				DeleteDocument ( const VecTraits_T<DocID_t> & dDocs, CSphString & sError, RtAccum_t * pAccExt ) final;
	bool				Commit ( int * pDeleted, RtAccum_t * pAccExt ) final;
	void				RollBack ( RtAccum_t * pAccExt ) final;
	bool				CommitReplayable ( RtSegment_t * pNewSeg, const CSphVector<DocID_t> & dAccKlist, int * pTotalKilled, bool bForceDump ) EXCLUDES (m_tChunkLock ); // FIXME? protect?
	void				ForceRamFlush ( bool bPeriodic=false ) final;
	bool				IsFlushNeed() const final;
	bool				ForceDiskChunk() final;
	bool				AttachDiskIndex ( CSphIndex * pIndex, bool bTruncate, bool & bFatal, CSphString & sError ) 			final  EXCLUDES (m_tReading );
	bool				Truncate ( CSphString & sError ) final;
	void				Optimize ( int iCutoff, int iFrom, int iTo ) final;
	void				CommonMerge ( Selector_t&& fnSelector );
	void				DropDiskChunk ( int iChunk ) REQUIRES ( m_tOptimizingLock );
	CSphIndex *			GetDiskChunk ( int iChunk ) final { return m_dDiskChunks.GetLength()>iChunk ? m_dDiskChunks[iChunk] : nullptr; }
	ISphTokenizer *		CloneIndexingTokenizer() const final { return m_pTokenizerIndexing->Clone ( SPH_CLONE_INDEX ); }

public:
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4100)
#endif

	int					Kill ( DocID_t tDocID ) final;
	int					KillMulti ( const VecTraits_T<DocID_t> & dKlist ) final;
	bool				IsAlive ( DocID_t tDocID ) const final;

	int					Build ( const CSphVector<CSphSource*> & , int , int ) final { return 0; }
	bool				Merge ( CSphIndex * , const CSphVector<CSphFilterSettings> &, bool ) final { return false; }

	bool				Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder ) final;
	void				Dealloc () final {}
	void				Preread () final;
	void				SetMemorySettings ( const FileAccessSettings_t & tFileAccessSettings ) final;
	const FileAccessSettings_t & GetMemorySettings() const final { return m_tFiles; }
	void				SetBase ( const char * ) final {}
	bool				Rename ( const char * ) final { return true; }
	bool				Lock () final { return true; }
	void				Unlock () final {}
	void				PostSetup() final;
	bool				IsRT() const final { return true; }

	int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, bool & bCritical, CSphString & sError, CSphString & sWarning ) final;
	bool				SaveAttributes ( CSphString & sError ) const final;
	DWORD				GetAttributeStatus () const final { return m_uDiskAttrStatus; }
	bool				AddRemoveAttribute ( bool bAdd, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError ) final;

	void				DebugDumpHeader ( FILE * , const char * , bool ) final {}
	void				DebugDumpDocids ( FILE * ) final {}
	void				DebugDumpHitlist ( FILE * , const char * , bool ) final {}
	void				DebugDumpDict ( FILE * ) final {}
	int					DebugCheck ( FILE * fp ) final;
#if USE_WINDOWS
#pragma warning(pop)
#endif

	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & ) const final;
	const CSphSourceStats &		GetStats () const final { return m_tStats; }
	int64_t *			GetFieldLens() const final { return m_tSettings.m_bIndexFieldLens ? m_dFieldLens.Begin() : nullptr; }
	void				GetStatus ( CSphIndexStatus* ) const final;

	bool				MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const final;
	bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const final;
	void				DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError, const SphChunkGuard_t & tGuard ) const;
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const final;
	bool				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const final;
	void				AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, CSphDict * pDict, bool bGetStats, int iQpos, RtQword_t * pQueryWord, CSphVector <CSphKeywordInfo> & dKeywords, const SphChunkGuard_t & tGuard ) const;

	bool				RtQwordSetup ( RtQword_t * pQword, int iSeg, const SphChunkGuard_t & tGuard ) const;
	static bool			RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint, const CSphIndexSettings & tSettings );

	bool				IsWordDict () const { return m_bKeywordDict; }
	int					GetWordCheckoint() const { return m_iWordsCheckpoint; }
	int					GetMaxCodepointLength() const { return m_iMaxCodepointLength; }

	// TODO: implement me
	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) final {}

	bool				IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const final;
	bool				Reconfigure ( CSphReconfigureSetup & tSetup ) final;
	int64_t				GetLastFlushTimestamp() const final;
	void				IndexDeleted() final { m_bIndexDeleted = true; }
	bool				CopyExternalFiles ( int iPostfix, StrVec_t & dCopied ) final;
	void				CollectFiles ( StrVec_t & dFiles, StrVec_t & dExt ) const final;
	void				ProhibitSave() final;
	void				EnableSave() final;
	void				LockFileState ( CSphVector<CSphString> & dFiles ) final;

	void				SetDebugCheck ( bool bCheckIdDups, int iCheckChunk ) final;

	void				CreateReader ( int64_t iSessionId ) const final;
	bool				GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const final;
	int					GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const final;
	virtual				uint64_t GetSchemaHash () const final { return m_uSchemaHash; }

protected:
	CSphSourceStats		m_tStats;
	bool				m_bDebugCheck = false;
	bool				m_bCheckIdDups = false;
	int					m_iCheckChunk = -1;
	CSphFixedVector<int>	m_dChunkNames { 0 };
	mutable int			m_iMaxChunkName = -1;

private:
	static const DWORD			META_HEADER_MAGIC	= 0x54525053;	///< my magic 'SPRT' header
	static const DWORD			META_VERSION		= 17;			///< current version fixme! Also change version in indextool.cpp, and support the changes!

	int							m_iStride;
	uint64_t					m_uSchemaHash = 0;

	LazyVector_T<RtSegmentRefPtf_t>	m_dRamChunks GUARDED_BY ( m_tChunkLock );
	std::atomic<int64_t>		m_iRamChunksAllocatedRAM;

	CSphMutex					m_tWriting;
	mutable CSphRwlock			m_tChunkLock;
	mutable CSphRwlock			m_tReading;

	CSphVector<DocID_t>			m_dKillsWhileSaving GUARDED_BY ( m_tWriting );	///< documents killed in ram chunks while we were saving disk chunks (double-buffered)
	CSphVector<DocID_t>			m_dKillsWhileOptimizing GUARDED_BY ( m_tOptimizingLock );

	/// double buffer stuff (allows to work with RAM chunk while future disk is being saved)
	/// m_dSegments consists of two parts
	/// segments with indexes < m_iDoubleBuffer are being saved now as a disk chunk
	/// segments with indexes >= m_iDoubleBuffer are RAM chunk
	CSphMutex					m_tFlushLock;
	CSphMutex					m_tOptimizingLock;
	int							m_iDoubleBuffer = 0;
	CSphMutex					m_tSaveFinished;
	volatile bool				m_bDoubleDump = false;

	int64_t						m_iSoftRamLimit;
	int64_t						m_iDoubleBufferLimit;
	CSphString					m_sPath;
	bool						m_bPathStripped = false;
	CSphVector<CSphIndex*>		m_dDiskChunks GUARDED_BY ( m_tChunkLock );
	int							m_iLockFD = -1;
	volatile bool				m_bOptimizing = false;
	volatile bool				m_bOptimizeStop = false;
	bool						m_bIndexDeleted = false;

	int64_t						m_iSavedTID;
	int64_t						m_tmSaved;
	mutable DWORD				m_uDiskAttrStatus = 0;

	bool						m_bKeywordDict;
	int							m_iWordsCheckpoint = RTDICT_CHECKPOINT_V5;
	int							m_iMaxCodepointLength = 0;
	TokenizerRefPtr_c			m_pTokenizerIndexing;
	bool						m_bLoadRamPassedOk = true;
	bool						m_bSaveDisabled = false;
	bool						m_bHasFiles = false;

	FileAccessSettings_t		m_tFiles;

	CSphFixedVector<int64_t>	m_dFieldLens { SPH_MAX_FIELDS };	///< total field lengths over entire index
	CSphFixedVector<int64_t>	m_dFieldLensRam { SPH_MAX_FIELDS };	///< field lengths summed over current RAM chunk
	CSphFixedVector<int64_t>	m_dFieldLensDisk { SPH_MAX_FIELDS };	///< field lengths summed over all disk chunks
	CSphBitvec					m_tMorphFields;
	CSphVector<SphWordID_t>		m_dHitlessWords;

	CSphScopedPtr<DocstoreFields_i> m_pDocstoreFields {nullptr};	// rt index doesn't have its own docstore, but it must keep all fields to get their ids for GetDoc

	RtAccum_t *					CreateAccum ( RtAccum_t * pAccExt, CSphString & sError ) final;

	int							CompareWords ( const RtWord_t * pWord1, const RtWord_t * pWord2 ) const;
	void						MergeAttributes ( RtRowIterator_c & tIt, RtSegment_t * pDestSeg, const RtSegment_t * pSrcSeg, int nBlobs, CSphVector<RowID_t> & dRowMap, RowID_t & tNextRowID ) const;
	void						MergeKeywords ( RtSegment_t & tSeg, const RtSegment_t & tSeg1, const RtSegment_t & tSeg2, const CSphVector<RowID_t> & dRowMap1, const CSphVector<RowID_t> & dRowMap2 ) const;
	RtSegment_t *				MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, bool bHasMorphology ) const;
	static void					CopyWord ( RtSegment_t & tDst, const RtSegment_t & tSrc, RtDocWriter_t & tOutDoc, RtDocReader_t & tInDoc, RtWord_t & tWord, const CSphVector<RowID_t> & tRowMap );

	bool						LoadMeta ( FilenameBuilder_i * pFilenameBuilder, bool bStripPath, DWORD & uVersion, bool & bRebuildInfixes );
	bool						PreallocDiskChunks ( FilenameBuilder_i * pFilenameBuilder );
	void						SaveMeta ( int64_t iTID, bool bSaveChunks=true );
	void						SaveMetaSpecial ( int64_t iTID, const VecTraits_T<int> & dChunkNames );
	void						SaveDiskHeader ( SaveDiskDataContext_t & tCtx, const ChunkStats_t & tStats ) const;
	void						SaveDiskData ( const char * sFilename, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats ) const;
	bool						SaveDiskChunk ( int64_t iTID, const SphChunkGuard_t & tGuard,
			const ChunkStats_t & tStats, bool bForced, int * pSavedChunkId )
			EXCLUDES ( m_tWriting ) EXCLUDES ( m_tChunkLock );
	CSphIndex *					PreallocDiskChunk ( const char * sChunk, int iChunk, FilenameBuilder_i * pFilenameBuilder, CSphString & sError, const char * sName=nullptr ) const;
	bool						LoadRamChunk ( DWORD uVersion, bool bRebuildInfixes );
	bool						SaveRamChunk ( const VecTraits_T<const RtSegmentRefPtf_t>& dSegments );

	bool						WriteAttributes ( SaveDiskDataContext_t & tCtx, CSphString & sError ) const;
	bool						WriteDocs ( SaveDiskDataContext_t & tCtx, CSphWriter & tWriterDict, CSphString & sError ) const;
	void						WriteCheckpoints ( SaveDiskDataContext_t & tCtx, CSphWriter & tWriterDict ) const;
	bool						WriteDeadRowMap ( SaveDiskDataContext_t & tCtx, CSphString & sError ) const;

	void						GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const final;
	void						GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const final;
	void						GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const final;

	void						SuffixGetChekpoints ( const SuggestResult_t & tRes, const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const final;
	void						SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const final;
	bool						ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const final;

	SphChunkGuard_t				GetReaderChunks() const ACQUIRE_SHARED (m_tReading );
	SphChunkGuard_t				GetReaderChunks ( const VecTraits_T<int64_t> & dChunks ) const ACQUIRE_SHARED ( m_tReading );

	RtSegmentRefPtf_t			AdoptSegment ( RtSegment_t * pNewSeg );

	int							ApplyKillList ( const CSphVector<DocID_t> & dAccKlist );

	void						Update_CollectRowPtrs ( UpdateContext_t & tCtx, const SphChunkGuard_t & tGuard );
	bool						Update_WriteBlobRow ( UpdateContext_t & tCtx, int iUpd, CSphRowitem * pDocinfo, const BYTE * pBlob, int iLength, int nBlobAttrs, bool & bCritical, CSphString & sError ) override;
	bool						Update_DiskChunks ( UpdateContext_t & tCtx, const SphChunkGuard_t & tGuard,	int & iUpdated, CSphString & sError );

	void						GetIndexFiles ( CSphVector<CSphString> & dFiles, const FilenameBuilder_i * pParentBuilder ) const override;
	DocstoreBuilder_i::Doc_t *	FetchDocFields ( DocstoreBuilder_i::Doc_t & tStoredDoc, CSphSource_StringVector & tSrc ) const;

	bool						MergeSegments ( CSphVector<RtSegmentRefPtf_t> & dSegments, bool bForceDump, int64_t iMemLimit, bool bHasNewSegment );
	RtSegmentRefPtf_t			MergeDoubleBufSegments ( CSphVector<RtSegmentRefPtf_t> & dSegments ) const;
	bool						NeedStoreWordID () const override;
	int64_t						GetMemLimit() const final { return m_iSoftRamLimit; }
	int							GetNextChunkName() REQUIRES_SHARED ( m_tChunkLock );

	void						DebugCheckRam ( DebugCheckError_c & tReporter );
	int							DebugCheckDisk ( DebugCheckError_c & tReporter, FILE * fp );

	void						SetSchema ( const CSphSchema & tSchema );
};


RtIndex_c::RtIndex_c ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict )
	: RtIndex_i ( sIndexName, sPath )
	, m_iSoftRamLimit ( iRamSize )
	, m_sPath ( sPath )
	, m_iSavedTID ( m_iTID )
	, m_tmSaved ( sphMicroTimer() )
	, m_bKeywordDict ( bKeywordDict )
{
	MEMORY ( MEM_INDEX_RT );

	SetSchema ( tSchema );

	m_iDoubleBufferLimit = ( m_iSoftRamLimit * SPH_RT_DOUBLE_BUFFER_PERCENT ) / 100;

	Verify ( m_tChunkLock.Init() );
	Verify ( m_tReading.Init() );
}


RtIndex_c::~RtIndex_c ()
{
	int64_t tmSave = sphMicroTimer();
	bool bValid = m_pTokenizer && m_pDict && m_bLoadRamPassedOk;

	if ( bValid && !m_bSaveDisabled )
	{
		SaveRamChunk ( m_dRamChunks );
		SaveMeta ( m_iTID );
	}

	Verify ( m_tReading.Done() );
	Verify ( m_tChunkLock.Done() );

	for ( auto & dDiskChunk : m_dDiskChunks )
		SafeDelete ( dDiskChunk );

	if ( m_iLockFD>=0 )
		::close ( m_iLockFD );

	// might be NULL during startup
	if ( g_pBinlog )
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, sphInterrupted () );

	if ( m_bIndexDeleted )
	{
		CSphString sFile;
		sFile.SetSprintf ( "%s.meta", m_sPath.cstr() );
		::unlink ( sFile.cstr() );
		sFile.SetSprintf ( "%s.ram", m_sPath.cstr() );
		::unlink ( sFile.cstr() );
	}

	tmSave = sphMicroTimer() - tmSave;
	if ( tmSave>=1000 && bValid )
	{
		sphInfo ( "rt: index %s: ramchunk saved in %d.%03d sec",
			m_sIndexName.cstr(), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
	}
}

bool RtIndex_c::IsFlushNeed() const
{
	// m_iTID get managed by binlog that is why wo binlog there is no need to compare it 
	if ( g_pBinlog && g_pBinlog->IsActive() && m_iTID<=m_iSavedTID )
		return false;

	if ( m_bSaveDisabled )
		return false;

	return true;
}

static int64_t SegmentsGetUsedRam ( const VecTraits_T<RtSegmentRefPtf_t> & dSegments )
{
	int64_t iTotal = 0;
	for ( RtSegment_t * pSeg : dSegments )
		iTotal += pSeg->GetUsedRam();

	return iTotal;
}

static int64_t SegmentsGetRamLeft ( const VecTraits_T<RtSegmentRefPtf_t> & dSegments, int64_t iMemLimit )
{
	iMemLimit -= SegmentsGetUsedRam ( dSegments );
	if ( iMemLimit<0 )
		iMemLimit = 0;
	return iMemLimit;
}

static int64_t SegmentsGetDeadRows ( const VecTraits_T<RtSegmentRefPtf_t> & dSegments )
{
	int64_t iTotal = 0;
	for ( RtSegment_t * pSeg : dSegments )
		iTotal += pSeg->m_tDeadRowMap.GetNumDeads();

	return iTotal;
}

void RtIndex_c::ForceRamFlush ( bool bPeriodic ) REQUIRES (!this->m_tFlushLock)
{
	if ( !IsFlushNeed() )
		return;

	int64_t tmSave = sphMicroTimer();

	// need this lock as could get here at same time either ways:
	// and via HandleMysqlFlushRtindex
	ScopedMutex_t tLock ( m_tFlushLock );

	int64_t iUsedRam = 0;
	int64_t iSavedTID = m_iTID;
	{
		SphChunkGuard_t tGuard = GetReaderChunks ();
		iUsedRam = SegmentsGetUsedRam ( tGuard.m_dRamChunks );

		if ( !SaveRamChunk ( tGuard.m_dRamChunks ) )
		{
			sphWarning ( "rt: index %s: ramchunk save FAILED! (error=%s)", m_sIndexName.cstr(), m_sLastError.cstr() );
			return;
		}
		SaveMetaSpecial ( iSavedTID, GetChunkNames ( tGuard.m_dDiskChunks ) );
		for ( auto pChunk : tGuard.m_dDiskChunks )
			pChunk->FlushDeadRowMap(true);
	}
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), iSavedTID, false );

	int64_t iWasTID = m_iSavedTID;
	int64_t tmDelta = sphMicroTimer() - m_tmSaved;
	m_iSavedTID = iSavedTID;
	m_tmSaved = sphMicroTimer();

	tmSave = sphMicroTimer() - tmSave;
	sphInfo ( "rt: index %s: ramchunk saved ok (mode=%s, last TID=" INT64_FMT ", current TID=" INT64_FMT ", "
		"ram=%d.%03d Mb, time delta=%d sec, took=%d.%03d sec)"
		, m_sIndexName.cstr(), bPeriodic ? "periodic" : "forced"
		, iWasTID, m_iTID, (int)(iUsedRam/1024/1024), (int)((iUsedRam/1024)%1000)
		, (int) (tmDelta/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}


int64_t RtIndex_c::GetLastFlushTimestamp() const
{
	return m_tmSaved;
}


//////////////////////////////////////////////////////////////////////////
// INDEXING
//////////////////////////////////////////////////////////////////////////
CSphSource_StringVector::CSphSource_StringVector ( const VecTraits_T<const char *> &dFields, const CSphSchema & tSchema )
	: CSphSource_Document ( "$stringvector" )
{
	m_tSchema = tSchema;
	m_dFieldLengths.Reserve ( dFields.GetLength () );
	m_dFields.Reserve ( dFields.GetLength() + 1 );
	for ( const char* sField : dFields )
	{
		m_dFields.Add ( (BYTE*) sField );
		m_dFieldLengths.Add ( (int) strlen ( sField ) );
		assert ( sField );
	}
	m_dFields.Add (nullptr);

	m_iMaxHits = 0; // force all hits build
}

CSphSource_StringVector::CSphSource_StringVector ( const VecTraits_T<VecTraits_T<const char >> &dFields, const CSphSchema & tSchema )
	: CSphSource_Document ( "$blobvector" )
{
	m_tSchema = tSchema;
	m_dFieldLengths.Reserve ( dFields.GetLength () );
	m_dFields.Reserve ( dFields.GetLength() + 1 );
	for ( const auto& dField : dFields )
	{
		m_dFields.Add ( (BYTE*) dField.begin() );
		m_dFieldLengths.Add ( dField.GetLength () );
		assert ( dField.begin() || dField.IsEmpty () );
	}
	m_dFields.Add (nullptr);

	m_iMaxHits = 0; // force all hits build
}

bool CSphSource_StringVector::Connect ( CSphString & )
{
	// no AddAutoAttrs() here; they should already be in the schema
	m_tHits.Reserve ( 1024 );
	return true;
}

void CSphSource_StringVector::Disconnect ()
{
	m_tHits.Reset();
}


DocstoreBuilder_i::Doc_t * RtIndex_c::FetchDocFields ( DocstoreBuilder_i::Doc_t & tStoredDoc, CSphSource_StringVector & tSrc ) const
{
	if ( !m_tSchema.HasStoredFields() )
		return nullptr;

	tSrc.GetDocFields ( tStoredDoc.m_dFields );
	assert ( tStoredDoc.m_dFields.GetLength()==m_tSchema.GetFieldsCount() );

	// filter out non-hl fields (should already be null)
	int iField = 0;
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		if ( !m_tSchema.IsFieldStored(i) )
			tStoredDoc.m_dFields.Remove(iField);
		else
			iField++;
	}

	return &tStoredDoc;
}


bool RtIndex_c::AddDocument ( const VecTraits_T<VecTraits_T<const char >> &dFields,
	CSphMatch & tDoc,	bool bReplace, const CSphString & sTokenFilterOptions, const char ** ppStr,
	const VecTraits_T<int64_t> & dMvas, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt )
{
	assert ( g_bRTChangesAllowed );
	assert ( m_tSchema.GetAttrIndex ( sphGetDocidName() )==0 );
	assert ( m_tSchema.GetAttr ( sphGetDocidName() )->m_eAttrType==SPH_ATTR_BIGINT );

	DocID_t tDocID = sphGetDocID ( tDoc.m_pDynamic );
	const bool bAutoID = ( tDocID==0 );
	if ( bAutoID )
	{
		SphChunkGuard_t tGuard = GetReaderChunks();
		// get uuid_short and check it will not collide with already provided document
		bool bDuplicate = true;
		while ( bDuplicate )
		{
			tDocID = UidShort();
			bDuplicate = false;
			ARRAY_FOREACH_COND ( i, tGuard.m_dRamChunks, !bDuplicate )
			{
				bDuplicate = ( tGuard.m_dRamChunks[i]->FindAliveRow ( tDocID )!=nullptr );
			}
		}

		CSphAttrLocator tLocID = m_tSchema.GetAttr ( sphGetDocidName() )->m_tLocator;
		tLocID.m_bDynamic = true;
		tDoc.SetAttr ( tLocID, tDocID );
	}

	TokenizerRefPtr_c tTokenizer { CloneIndexingTokenizer() };

	if (!tTokenizer)
	{
		sError.SetSprintf ( "internal error: no indexing tokenizer available" );
		return false;
	}

	MEMORY ( MEM_INDEX_RT );

	if ( !bReplace && !bAutoID )
	{
		SphChunkGuard_t tGuard = GetReaderChunks();
		for ( const auto & pSegment : tGuard.m_dRamChunks )
		{
			if ( pSegment->FindAliveRow ( tDocID ) )
			{
				sError.SetSprintf ( "duplicate id '" INT64_FMT "'", tDocID );
				return false; // already exists and not deleted; INSERT fails
			}
		}
		for ( const auto & pChunk : tGuard.m_dDiskChunks )
		{
			if ( pChunk->IsAlive ( tDocID ) )
			{
				sError.SetSprintf ( "duplicate id '" INT64_FMT "'", tDocID );
				return false; // already exists and not deleted; INSERT fails
			}
		}

	}

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt, m_bKeywordDict, true, &sError );
	if ( !pAcc )
		return false;

	tDoc.m_tRowID = pAcc->GenerateRowID();

	// OPTIMIZE? do not create filter on each(!) INSERT
	if ( !m_tSettings.m_sIndexTokenFilter.IsEmpty() )
	{
		tTokenizer = ISphTokenizer::CreatePluginFilter ( tTokenizer, m_tSettings.m_sIndexTokenFilter, sError );
		if ( !tTokenizer )
			return false;
		if ( !tTokenizer->SetFilterSchema ( m_tSchema, sError ) )
			return false;
		if ( !sTokenFilterOptions.IsEmpty() )
			if ( !tTokenizer->SetFilterOptions ( sTokenFilterOptions.cstr(), sError ) )
				return false;
	}

	// OPTIMIZE? do not create filter on each(!) INSERT
	if ( m_tSettings.m_uAotFilterMask )
		tTokenizer = sphAotCreateFilter ( tTokenizer, m_pDict, m_tSettings.m_bIndexExactWords, m_tSettings.m_uAotFilterMask );

	CSphSource_StringVector tSrc ( dFields, m_tSchema );

	// SPZ setup
	if ( m_tSettings.m_bIndexSP && !tTokenizer->EnableSentenceIndexing ( sError ) )
		return false;

	if ( !m_tSettings.m_sZones.IsEmpty() && !tTokenizer->EnableZoneIndexing ( sError ) )
		return false;

	if ( m_tSettings.m_bHtmlStrip && !tSrc.SetStripHTML ( m_tSettings.m_sHtmlIndexAttrs.cstr(), m_tSettings.m_sHtmlRemoveElements.cstr(),
			m_tSettings.m_bIndexSP, m_tSettings.m_sZones.cstr(), sError ) )
		return false;

	// OPTIMIZE? do not clone filters on each INSERT
	FieldFilterRefPtr_c pFieldFilter;
	if ( m_pFieldFilter )
		pFieldFilter = m_pFieldFilter->Clone();

	tSrc.Setup ( m_tSettings );
	tSrc.SetTokenizer ( tTokenizer );
	tSrc.SetDict ( pAcc->m_pDict );
	tSrc.SetFieldFilter ( pFieldFilter );
	tSrc.SetMorphFields ( m_tMorphFields );
	if ( !tSrc.Connect ( m_sLastError ) )
		return false;

	m_tSchema.CloneWholeMatch ( tSrc.m_tDocInfo, tDoc );

	bool bEOF = false;
	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( bEOF, sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );
	pAcc->GrabLastWarning ( sWarning );

	DocstoreBuilder_i::Doc_t tStoredDoc;
	DocstoreBuilder_i::Doc_t * pStoredDoc = FetchDocFields ( tStoredDoc, tSrc );
	if ( !AddDocument ( pHits, tDoc, bReplace, ppStr, dMvas, pStoredDoc, sError, sWarning, pAcc ) )
		return false;

	m_tStats.m_iTotalBytes += tSrc.GetStats().m_iTotalBytes;

	return true;
}


RtAccum_t * RtIndex_i::AcquireAccum ( CSphDict * pDict, RtAccum_t * pAcc,
	bool bWordDict, bool bSetTLS, CSphString* sError )
{
	if ( !pAcc )
		pAcc = g_pTlsAccum;

	if ( pAcc && pAcc->GetIndex() && pAcc->GetIndex()!=this )
	{
		if ( sError )
			sError->SetSprintf ( "current txn is working with another index ('%s')", pAcc->GetIndex()->GetName() );
		return nullptr;
	}

	if ( pAcc && pAcc->GetIndex() && pAcc->GetSchemaHash()!=GetSchemaHash() )
	{
		if ( sError )
			sError->SetSprintf ( "current txn is working with index's another schema ('%s'), restart session", pAcc->GetIndex()->GetName() );
		return nullptr;
	}


	if ( !pAcc )
	{
		pAcc = new RtAccum_t ( bWordDict );
		if ( bSetTLS )
		{
			g_pTlsAccum = pAcc;
			Threads::OnExitThread( [pAcc] { delete pAcc; } );
		}
	}

	assert ( pAcc->GetIndex()==nullptr || pAcc->GetIndex()==this );
	pAcc->SetIndex ( this );
	pAcc->SetupDict ( this, pDict, bWordDict );
	return pAcc;
}

RtAccum_t * RtIndex_c::CreateAccum ( RtAccum_t * pAccExt, CSphString & sError )
{
	return AcquireAccum ( m_pDict, pAccExt, m_bKeywordDict, false, &sError);
}


bool RtIndex_c::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const VecTraits_T<int64_t> & dMvas,
	const DocstoreBuilder_i::Doc_t * pStoredDoc, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = (RtAccum_t *)pAccExt;

	if ( pAcc )
		pAcc->AddDocument ( pHits, tDoc, bReplace, m_tSchema.GetRowSize(), ppStr, dMvas, pStoredDoc );

	return ( pAcc!=NULL );
}


RtAccum_t::RtAccum_t ( bool bKeywordDict )
	: m_bKeywordDict ( bKeywordDict )
{}

RtAccum_t::~RtAccum_t()
{
	SafeDelete ( m_pBlobWriter );
	ARRAY_FOREACH ( i, m_dCmd )
		SafeDelete ( m_dCmd[i] );
}

void RtAccum_t::SetupDict ( const RtIndex_i * pIndex, CSphDict * pDict, bool bKeywordDict )
{
	if ( pIndex!=m_pIndex || pDict!=m_pRefDict || bKeywordDict!=m_bKeywordDict )
	{
		m_bKeywordDict = bKeywordDict;
		m_pRefDict = pDict;
		m_pDict = GetStatelessDict ( pDict );
		if ( m_bKeywordDict )
		{
			m_pDict = m_pDictRt = sphCreateRtKeywordsDictionaryWrapper ( m_pDict, pIndex->NeedStoreWordID() );
			SafeAddRef ( m_pDict ); // since m_pDict and m_pDictRt are DIFFERENT types, = works via CsphDict*
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
	m_dPackedKeywords.Reset ( 0 );
}

const BYTE * RtAccum_t::GetPackedKeywords () const
{
	if ( m_dPackedKeywords.GetLength() )
		return m_dPackedKeywords.Begin();
	else
		return m_pDictRt->GetPackedKeywords();
}

int RtAccum_t::GetPackedLen () const
{
	if ( m_dPackedKeywords.GetLength() )
		return m_dPackedKeywords.GetLength();
	else
		return m_pDictRt->GetPackedLen();
}

void RtAccum_t::Sort ()
{
	if ( !m_bKeywordDict )
		m_dAccum.Sort ( CmpHitPlain_fn() );
	else
	{
		assert ( m_pDictRt );
		const BYTE * pPackedKeywords = GetPackedKeywords();
		m_dAccum.Sort ( CmpHitKeywords_fn ( pPackedKeywords ) );
	}
}

void RtAccum_t::CleanupPart()
{
	m_dAccumRows.Resize(0);
	m_dBlobs.Resize(0);
	m_dPerDocHitsCount.Resize (0);
	m_dAccum.Resize(0);
	m_pDocstore.Reset();

	ResetDict();
	ResetRowID();
}

void RtAccum_t::Cleanup()
{
	CleanupPart();

	SetIndex ( nullptr );
	m_uAccumDocs = 0;
	m_dAccumKlist.Reset ();
	
	ARRAY_FOREACH ( i, m_dCmd )
		SafeDelete ( m_dCmd[i] );

	m_dCmd.Reset();
}


void RtAccum_t::SetupDocstore()
{
	if ( m_pDocstore.Ptr() )
		return;

	m_pDocstore = CreateDocstoreRT();
	assert ( m_pDocstore.Ptr() );
	SetupDocstoreFields ( *m_pDocstore.Ptr(), m_pIndex->GetInternalSchema() );
}

bool RtAccum_t::SetupDocstore ( RtIndex_i & tIndex, CSphString & sError )
{
	const CSphSchema & tSchema = tIndex.GetInternalSchema();
	if ( !m_pDocstore.Ptr() && !tSchema.HasStoredFields() )
		return true;

	// might be a case when replicated trx was wo docstore but index has docstore
	if ( !m_pDocstore.Ptr() )
		m_pDocstore = CreateDocstoreRT();

	assert ( m_pDocstore.Ptr() );
	SetupDocstoreFields ( *m_pDocstore.Ptr(), tSchema );
	return m_pDocstore->CheckFieldsLoaded ( sError );
}


void RtAccum_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, bool bReplace, int iRowSize, const char ** ppStr, const VecTraits_T<int64_t> & dMvas, const DocstoreBuilder_i::Doc_t * pStoredDoc )
{
	MEMORY ( MEM_RT_ACCUM );

	// FIXME? what happens on mixed insert/replace?
	m_bReplace = bReplace;

	DocID_t tDocID = sphGetDocID ( tDoc.m_pDynamic );

	// schedule existing copies for deletion
	m_dAccumKlist.Add ( tDocID );

	// reserve some hit space on first use
	if ( pHits && pHits->GetLength() && !m_dAccum.GetLength() )
		m_dAccum.Reserve ( 128*1024 );

	// accumulate row data; expect fully dynamic rows
	assert ( !tDoc.m_pStatic );
	assert (!( !tDoc.m_pDynamic && iRowSize!=0 ));
	assert (!( tDoc.m_pDynamic && (int)tDoc.m_pDynamic[-1]!=iRowSize ));

	m_dAccumRows.Append ( tDoc.m_pDynamic, iRowSize );
	CSphRowitem * pRow = &m_dAccumRows [ m_dAccumRows.GetLength() - iRowSize ];
	CSphString sError;

	int iStrAttr = 0;
	int iBlobAttr = 0;
	int iMva = 0;

	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tColumn = tSchema.GetAttr(i);

		if ( tColumn.m_eAttrType==SPH_ATTR_STRING || tColumn.m_eAttrType==SPH_ATTR_JSON )
		{
			const BYTE * pStr = ppStr ? (const BYTE *) ppStr[iStrAttr++] : nullptr;
			ByteBlob_t dStr;
			if ( tColumn.m_eAttrType==SPH_ATTR_STRING )
				dStr = {pStr, pStr ? (int) strlen ((const char *) pStr ) : 0};
			else // SPH_ATTR_JSON - packed len + data
				dStr = sphUnpackPtrAttr ( pStr );

			assert ( m_pBlobWriter );
			m_pBlobWriter->SetAttr ( iBlobAttr++, dStr.first, dStr.second, sError );
		} else if ( tColumn.m_eAttrType==SPH_ATTR_UINT32SET || tColumn.m_eAttrType==SPH_ATTR_INT64SET )
		{
			assert ( m_pBlobWriter );
			const int64_t * pMva = &dMvas[iMva];
			int nValues = *pMva++;
			iMva += nValues+1;

			m_pBlobWriter->SetAttr ( iBlobAttr++, (const BYTE*)pMva, nValues*sizeof(int64_t), sError );
		}
	}

	if ( m_pBlobWriter )
	{
		const CSphColumnInfo * pBlobLoc = tSchema.GetAttr ( sphGetBlobLocatorName() );
		assert ( pBlobLoc );

		sphSetRowAttr ( pRow, pBlobLoc->m_tLocator, m_pBlobWriter->Flush() );
	}

	// handle index_field_lengths
	DWORD * pFieldLens = NULL;
	if ( m_pIndex->GetSettings().m_bIndexFieldLens )
	{
		int iFirst = tSchema.GetAttrId_FirstFieldLen();
		assert ( tSchema.GetAttr ( iFirst ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		assert ( tSchema.GetAttr ( iFirst+tSchema.GetFieldsCount()-1 ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		pFieldLens = pRow + ( tSchema.GetAttr ( iFirst ).m_tLocator.m_iBitOffset / 32 );
		memset ( pFieldLens, 0, sizeof(int)*tSchema.GetFieldsCount() ); // NOLINT
	}

	// accumulate hits
	int iHits = 0;
	if ( pHits && pHits->GetLength() )
	{
		CSphWordHit tLastHit;
		tLastHit.m_tRowID = INVALID_ROWID;
		tLastHit.m_uWordID = 0;
		tLastHit.m_uWordPos = 0;

		m_dAccum.ReserveGap ( pHits->GetLength() );
		iHits = 0;
		for ( CSphWordHit * pHit = pHits->Begin(); pHit<pHits->End(); ++pHit )
		{
			// ignore duplicate hits
			if ( *pHit==tLastHit )
				continue;

			// update field lengths
			if ( pFieldLens && HITMAN::GetField ( pHit->m_uWordPos )!=HITMAN::GetField ( tLastHit.m_uWordPos ) )
				pFieldLens [ HITMAN::GetField ( tLastHit.m_uWordPos ) ] = HITMAN::GetPos ( tLastHit.m_uWordPos );

			// need original hit for duplicate removal
			tLastHit = *pHit;
			// reset field end for not very last position
			if ( HITMAN::IsEnd ( pHit->m_uWordPos ) && pHit!=&pHits->Last() &&
				pHit->m_tRowID==pHit[1].m_tRowID && pHit->m_uWordID==pHit[1].m_uWordID && HITMAN::IsEnd ( pHit[1].m_uWordPos ) )
				pHit->m_uWordPos = HITMAN::GetPosWithField ( pHit->m_uWordPos );

			// accumulate
			m_dAccum.Add ( *pHit );
			++iHits;
		}
		if ( pFieldLens )
			pFieldLens [ HITMAN::GetField ( tLastHit.m_uWordPos ) ] = HITMAN::GetPos ( tLastHit.m_uWordPos );
	}
	// make sure to get real count without duplicated hits
	m_dPerDocHitsCount.Add ( iHits );

	if ( pStoredDoc )
	{
		SetupDocstore();
		RowID_t tRowID = m_dAccumRows.GetLength()/iRowSize - 1;
		m_pDocstore->AddDoc ( tRowID, *pStoredDoc );
	}

	++m_uAccumDocs;
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
	for ( auto & dCheckpoint : pSeg->m_dWordCheckpoints )
		dCheckpoint.m_sWord = pBase + dCheckpoint.m_uWordID;
}


RtSegment_t * RtAccum_t::CreateSegment ( int iRowSize, int iWordsCheckpoint, ESphHitless eHitless, const VecTraits_T<SphWordID_t> & dHitlessWords )
{
	if ( !m_uAccumDocs )
		return nullptr;

	MEMORY ( MEM_RT_ACCUM );
	auto * pSeg = new RtSegment_t ( m_uAccumDocs );

	CSphWordHit tClosingHit;
	tClosingHit.m_uWordID = WORDID_MAX;
	tClosingHit.m_tRowID = INVALID_ROWID;
	tClosingHit.m_uWordPos = EMPTY_HIT;
	m_dAccum.Add ( tClosingHit );

	RtDoc_t tDoc;
	tDoc.m_tRowID = INVALID_ROWID;
	tDoc.m_uDocFields = 0;
	tDoc.m_uHits = 0;
	tDoc.m_uHit = 0;

	RtWord_t tWord;
	RtDocWriter_t tOutDoc ( pSeg );
	RtWordWriter_t tOutWord ( pSeg, m_bKeywordDict, iWordsCheckpoint );
	RtHitWriter_t tOutHit ( pSeg );

	const BYTE * pPacketBase = m_bKeywordDict ? GetPackedKeywords() : nullptr;

	Hitpos_t uEmbeddedHit = EMPTY_HIT;
	Hitpos_t uPrevHit = EMPTY_HIT;

	for ( const CSphWordHit & tHit : m_dAccum )
	{
		// new keyword or doc; flush current doc
		if ( tHit.m_uWordID!=tWord.m_uWordID || tHit.m_tRowID!=tDoc.m_tRowID )
		{
			if ( tDoc.m_tRowID!=INVALID_ROWID )
			{
				++tWord.m_uDocs;
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

			tDoc.m_tRowID = tHit.m_tRowID;
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
					assert ( pPackedWord[0] && pPackedWord[0]+1<GetPackedLen() );
					tWord.m_sWord = pPackedWord;
				}
				tOutWord.ZipWord ( tWord );
			}

			tWord.m_uWordID = tHit.m_uWordID;
			tWord.m_uDocs = 0;
			tWord.m_uHits = 0;
			tWord.m_uDoc = tOutDoc.ZipDocPtr();
			uPrevHit = EMPTY_HIT;
			if ( eHitless==SPH_HITLESS_NONE || eHitless==SPH_HITLESS_ALL || !tWord.m_uWordID || tHit.m_uWordPos==EMPTY_HIT )
			{
				tWord.m_bHasHitlist = ( eHitless==SPH_HITLESS_NONE || !tWord.m_uWordID );
			} else
			{
				SphWordID_t tWordID = tWord.m_uWordID;
				if ( m_bKeywordDict && dHitlessWords.GetLength() )
				{
					const BYTE * pPackedWord = pPacketBase + tWord.m_uWordID;
					DWORD uLen = pPackedWord[0];
					assert ( uLen && (int)uLen+1<GetPackedLen() );
					memcpy ( &tWordID, pPackedWord + uLen + 1, sizeof ( tWordID ) );
				}
				tWord.m_bHasHitlist = ( dHitlessWords.BinarySearch ( tWordID )==nullptr );
			}
		}

		// might be a duplicate
		if ( uPrevHit==tHit.m_uWordPos )
			continue;

		// just a new hit
		if ( !tWord.m_bHasHitlist )
		{
			if ( !tDoc.m_uHits )
				uEmbeddedHit = tHit.m_uWordPos;
			tDoc.m_uHits = 1; // FIXME!!! hitless hit-count always 1
		} else if ( !tDoc.m_uHits )
		{
			uEmbeddedHit = tHit.m_uWordPos;
			++tDoc.m_uHits;
		} else
		{
			if ( uEmbeddedHit )
			{
				tOutHit.ZipHit ( uEmbeddedHit );
				uEmbeddedHit = 0;
			}

			tOutHit.ZipHit ( tHit.m_uWordPos );
			++tDoc.m_uHits;
		}
		uPrevHit = tHit.m_uWordPos;

		const int iField = HITMAN::GetField ( tHit.m_uWordPos );
		if ( iField<32 )
			tDoc.m_uDocFields |= ( 1UL<<iField );
	}

	if ( m_bKeywordDict )
		FixupSegmentCheckpoints ( pSeg );

	pSeg->m_uRows = m_uAccumDocs;
	pSeg->m_tAliveRows.store ( m_uAccumDocs, std::memory_order_relaxed );

	pSeg->m_dRows.SwapData(m_dAccumRows);
	pSeg->m_dBlobs.SwapData(m_dBlobs);
	pSeg->m_pDocstore.Swap(m_pDocstore);

	pSeg->BuildDocID2RowIDMap();

	m_tNextRowID = 0;

	// done
	return pSeg;
}


struct AccumDocHits_t
{
	DocID_t	m_tDocID;
	int		m_iDocIndex;
	int		m_iHitIndex;
	int		m_iHitCount;
};


struct CmpDocHitIndex_t
{
	inline bool IsLess ( const AccumDocHits_t & a, const AccumDocHits_t & b ) const
	{
		return ( a.m_tDocID<b.m_tDocID || ( a.m_tDocID==b.m_tDocID && a.m_iDocIndex<b.m_iDocIndex ) );
	}
};


void RtAccum_t::CleanupDuplicates ( int iRowSize )
{
	if ( m_uAccumDocs<=1 )
		return;

	assert ( m_uAccumDocs==(DWORD)m_dPerDocHitsCount.GetLength() );
	CSphVector<AccumDocHits_t> dDocHits ( m_dPerDocHitsCount.GetLength() );

	int iHitIndex = 0;
	CSphRowitem * pRow = m_dAccumRows.Begin();
	for ( DWORD i=0; i<m_uAccumDocs; i++, pRow+=iRowSize )
	{
		AccumDocHits_t & tElem = dDocHits[i];
		tElem.m_tDocID = sphGetDocID ( pRow );
		tElem.m_iDocIndex = i;
		tElem.m_iHitIndex = iHitIndex;
		tElem.m_iHitCount = m_dPerDocHitsCount[i];
		iHitIndex += m_dPerDocHitsCount[i];
	}

	dDocHits.Sort ( CmpDocHitIndex_t() );

	DocID_t uPrev = 0;
	if ( !dDocHits.any_of ( [&] ( const AccumDocHits_t &dDoc ) {
			bool bRes = dDoc.m_tDocID==uPrev;
			uPrev = dDoc.m_tDocID;
			return bRes;
		} ) )
		return;

	CSphFixedVector<RowID_t> dRowMap(m_uAccumDocs);
	for ( auto & i : dRowMap )
		i = 0;

	// identify duplicates to kill
	if ( m_bReplace )
	{
		// replace mode, last value wins, precending values are duplicate
		for ( DWORD i=0; i<m_uAccumDocs-1; ++i )
			if ( dDocHits[i].m_tDocID==dDocHits[i+1].m_tDocID )
				dRowMap[dDocHits[i].m_iDocIndex] = INVALID_ROWID;
	} else
	{
		// insert mode, first value wins, subsequent values are duplicates
		for ( DWORD i=1; i<m_uAccumDocs; ++i )
			if ( dDocHits[i].m_tDocID==dDocHits[i-1].m_tDocID )
				dRowMap[dDocHits[i].m_iDocIndex] = INVALID_ROWID;
	}

	RowID_t tNextRowID = 0;
	for ( auto & i : dRowMap )
		if ( i!=INVALID_ROWID )
			i = tNextRowID++;

	// remove duplicate hits
	int iDstRow = 0;
	for ( int i=0, iLen=m_dAccum.GetLength(); i<iLen; ++i )
	{
		const auto & dSrcHit = m_dAccum[i];
		RowID_t tSrcRowID = dRowMap[dSrcHit.m_tRowID];
		if ( tSrcRowID!=INVALID_ROWID )
		{
			CSphWordHit & tDstHit = m_dAccum[iDstRow];
			if ( i!=iDstRow )
			{
				tDstHit = dSrcHit;
				tDstHit.m_tRowID = tSrcRowID;
			}
			++iDstRow;
		}
	}

	m_dAccum.Resize( iDstRow);

	// remove duplicates
	iDstRow = 0;
	ARRAY_FOREACH ( i, dRowMap )
		if ( dRowMap[i]!=INVALID_ROWID )
		{
			if ( i!=iDstRow )
			{
				// remove duplicate docinfo
				memcpy ( &m_dAccumRows[iDstRow * iRowSize], &m_dAccumRows[i * iRowSize], iRowSize * sizeof ( CSphRowitem ) );

				// remove duplicate docstore
				if ( m_pDocstore )
					m_pDocstore->SwapRows ( iDstRow, i );
			}
			++iDstRow;
		}

	m_dAccumRows.Resize ( iDstRow * iRowSize );
	m_uAccumDocs = iDstRow;
	if ( m_pDocstore )
		m_pDocstore->DropTail ( iDstRow );
}


void RtAccum_t::GrabLastWarning ( CSphString & sWarning )
{
	if ( m_pDictRt && m_pDictRt->GetLastWarning() )
	{
		sWarning = m_pDictRt->GetLastWarning();
		m_pDictRt->ResetWarning();
	}
}


void RtAccum_t::SetIndex ( RtIndex_i * pIndex )
{
	m_pIndex = pIndex;

	SafeDelete ( m_pBlobWriter );
	if ( pIndex && pIndex->GetInternalSchema().HasBlobAttrs() )
		m_pBlobWriter = sphCreateBlobRowBuilder ( pIndex->GetInternalSchema(), m_dBlobs );

	if ( pIndex )
		m_uSchemaHash = pIndex->GetSchemaHash();
}


RowID_t RtAccum_t::GenerateRowID()
{
	return m_tNextRowID++;
}


void RtAccum_t::ResetRowID()
{
	m_tNextRowID=0;
}

ReplicationCommand_t * RtAccum_t::AddCommand ( ReplicationCommand_e eCmd )
{
	return AddCommand ( eCmd, CSphString(), CSphString() );
}

ReplicationCommand_t * RtAccum_t::AddCommand ( ReplicationCommand_e eCmd, const CSphString & sCluster, const CSphString & sIndex )
{
	// all writes to RT index go as single command to serialize accumulator
	if ( eCmd==ReplicationCommand_e::RT_TRX && m_dCmd.GetLength() && m_dCmd.Last()->m_eCommand==ReplicationCommand_e::RT_TRX )
		return m_dCmd.Last();

	auto* pCmd = new ReplicationCommand_t ();
	m_dCmd.Add ( pCmd );
	pCmd->m_eCommand = eCmd;
	pCmd->m_sCluster = sCluster;
	pCmd->m_sIndex = sIndex;

	return pCmd;
}

void RtIndex_c::CopyWord ( RtSegment_t & tDst, const RtSegment_t & tSrc, RtDocWriter_t & tOutDoc, RtDocReader_t & tInDoc,
		RtWord_t & tWord, const CSphVector<RowID_t> & dRowMap )
{
	// copy docs
	while (true)
	{
		const RtDoc_t * pDoc = tInDoc.UnzipDoc();
		if ( !pDoc )
			break;

		RowID_t tNewRowID = dRowMap[pDoc->m_tRowID];
		if ( tNewRowID==INVALID_ROWID )
			continue;

		RtDoc_t tDoc = *pDoc;
		tDoc.m_tRowID = tNewRowID;

		++tWord.m_uDocs;
		tWord.m_uHits += pDoc->m_uHits;

		if ( pDoc->m_uHits!=1 )
		{
			RtHitReader_t tInHit ( &tSrc, pDoc );

			tDoc.m_uHit = tDst.m_dHits.GetLength ();
			tDst.m_dHits.Append ( tInHit.GetHitsBlob () );

			// this is reference of what append (hitsblob) above does.
//			RtHitWriter_t tOutHit ( &tDst );
//			tDoc.m_uHit = tOutHit.ZipHitPtr();
//			for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
//				tOutHit.ZipHit ( uValue );
		}

		// copy doc
		tOutDoc.ZipDoc ( tDoc );
	}
}


class RtRowIterator_c : public ISphNoncopyable
{
public:
	RtRowIterator_c ( const RtSegment_t * pSeg, int iStride )
		: m_pRow ( pSeg->m_dRows.Begin() )
		, m_pRowMax ( pSeg->m_dRows.Begin() + pSeg->m_dRows.GetLength() )
		, m_iStride ( iStride )
		, m_tDeadRowMap ( pSeg->m_tDeadRowMap )
	{}

	const CSphRowitem * GetNextAliveRow()
	{
		// while there are rows and k-list entries
		while ( m_pRow<m_pRowMax )
		{
			if ( !m_tDeadRowMap.IsSet(m_tRowID) )
				break;

			m_pRow += m_iStride;
			m_tRowID++;
		}

		if ( m_pRow>=m_pRowMax )
			return nullptr;

		// got it, and it's alive!
		m_tRowID++;
		m_pRow += m_iStride;
		return m_pRow-m_iStride;
	}

	RowID_t GetRowID() const
	{
		return m_tRowID-1;
	}

protected:
	RowID_t						m_tRowID {0};
	const CSphRowitem *			m_pRow {nullptr};
	const CSphRowitem *			m_pRowMax {nullptr};
	const int					m_iStride {0};
	const DeadRowMap_Ram_c &	m_tDeadRowMap;
};

template <typename BLOOM_TRAITS>
inline bool BuildBloom_T ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8, int iKeyValCount, BLOOM_TRAITS & tBloom )
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
	for ( int i=0; i<=iCodes-iInfixCodepointCount && tBloom.IterateNext(); i++ )
	{
		int iFrom = dOffsets[i];
		int iTo = dOffsets[i+iInfixCodepointCount];
		uint64_t uHash64 = sphFNV64 ( sWord+iFrom, iTo-iFrom );

		uHash64 = ( uHash64>>32 ) ^ ( (DWORD)uHash64 );
		int iByte = (int)( uHash64 % iKeyBytes );
		int iPos = iByte/64;
		uint64_t uVal = U64C(1) << ( iByte % 64 );

		tBloom.Set ( iPos, uVal );
	}
	return true;
}

// explicit instantiations
bool BuildBloom ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8, int iKeyValCount
				  , BloomCheckTraits_t &tBloom )
{
	return BuildBloom_T ( sWord, iLen, iInfixCodepointCount, bUtf8, iKeyValCount, tBloom );
}

bool BuildBloom ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8, int iKeyValCount
				  , BloomGenTraits_t &tBloom )
{
	return BuildBloom_T ( sWord, iLen, iInfixCodepointCount, bUtf8, iKeyValCount, tBloom );
}

void BuildSegmentInfixes ( RtSegment_t * pSeg, bool bHasMorphology, bool bKeywordDict, int iMinInfixLen, int iWordsCheckpoint, bool bUtf8, ESphHitless eHitlessMode )
{
	if ( !pSeg || !bKeywordDict || !iMinInfixLen )
		return;

	int iBloomSize = ( pSeg->m_dWordCheckpoints.GetLength()+1 ) * BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT;
	pSeg->m_dInfixFilterCP.Resize ( iBloomSize );
	// reset filters
	pSeg->m_dInfixFilterCP.Fill ( 0 );

	uint64_t * pRough = pSeg->m_dInfixFilterCP.Begin();
	const RtWord_t * pWord = nullptr;
	RtWordReader_t rdDictRough ( pSeg, true, iWordsCheckpoint, eHitlessMode );
	while ( ( pWord = rdDictRough.UnzipWord () )!=nullptr )
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
		BloomGenTraits_t tBloom0 ( pVal );
		BloomGenTraits_t tBloom1 ( pVal+BLOOM_PER_ENTRY_VALS_COUNT );
		BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_0, bUtf8, BLOOM_PER_ENTRY_VALS_COUNT, tBloom0 );
		BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_1, bUtf8, BLOOM_PER_ENTRY_VALS_COUNT, tBloom1 );
	}
}


void RtIndex_c::MergeAttributes ( RtRowIterator_c & tIt, RtSegment_t * pDestSeg, const RtSegment_t * pSrcSeg, int nBlobs,
		CSphVector<RowID_t> & dRowMap, RowID_t & tNextRowID ) const
{
	const CSphRowitem * pRow;
	while ( !!(pRow=tIt.GetNextAliveRow()) )
	{
		auto pNewRow = pDestSeg->m_dRows.AddN ( m_iStride );
		memcpy ( pNewRow, pRow, m_iStride*sizeof(CSphRowitem) );

		if ( nBlobs )
		{
			int64_t iOldOffset = sphGetBlobRowOffset ( pRow );
			int64_t iNewOffset = sphCopyBlobRow ( pDestSeg->m_dBlobs, pSrcSeg->m_dBlobs, iOldOffset, nBlobs );
			sphSetBlobRowOffset ( pNewRow, iNewOffset );
		}

		if ( pDestSeg->m_pDocstore )
			pDestSeg->m_pDocstore->AddPackedDoc ( tNextRowID, pSrcSeg->m_pDocstore.Ptr(), tIt.GetRowID() );

		dRowMap[tIt.GetRowID()] = tNextRowID++;
	}
}


int RtIndex_c::CompareWords ( const RtWord_t * pWord1, const RtWord_t * pWord2 ) const
{
	if ( !pWord1 )
		return pWord2 ? 1 : 0;

	if ( !pWord2 )
		return -1;

	if ( m_bKeywordDict )
		return sphDictCmpStrictly ( (const char *)pWord1->m_sWord+1, *pWord1->m_sWord, (const char *)pWord2->m_sWord+1, *pWord2->m_sWord );

	if ( pWord1->m_uWordID<pWord2->m_uWordID )
		return -1;

	if ( pWord1->m_uWordID>pWord2->m_uWordID )
		return 1;

	return 0;
}


void RtIndex_c::MergeKeywords ( RtSegment_t & tSeg, const RtSegment_t & tSeg1, const RtSegment_t & tSeg2,
		const CSphVector<RowID_t> & dRowMap1, const CSphVector<RowID_t> & dRowMap2 ) const
{
	tSeg.m_dWords.Reserve ( Max ( tSeg1.m_dWords.GetLength(), tSeg2.m_dWords.GetLength() ) );
	tSeg.m_dDocs.Reserve ( Max ( tSeg1.m_dDocs.GetLength(), tSeg2.m_dDocs.GetLength() ) );
	tSeg.m_dHits.Reserve ( Max ( tSeg1.m_dHits.GetLength(), tSeg2.m_dHits.GetLength() ) );

	RtWordWriter_t tOut ( &tSeg, m_bKeywordDict, m_iWordsCheckpoint );
	RtWordReader_t tIn1 ( &tSeg1, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings.m_eHitless );
	RtWordReader_t tIn2 ( &tSeg2, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings.m_eHitless );

	const RtWord_t * pWords1 = tIn1.UnzipWord ();
	const RtWord_t * pWords2 = tIn2.UnzipWord ();

	while ( pWords1 || pWords2 )
	{
		int iCmp = CompareWords ( pWords1, pWords2 );

		RtDocWriter_t tOutDoc ( &tSeg );

		RtWord_t tWord = iCmp<=0 ? *pWords1 : *pWords2;
		tWord.m_uDocs = 0;
		tWord.m_uHits = 0;
		tWord.m_uDoc = tOutDoc.ZipDocPtr();

		// if words are equal, copy both
		if ( iCmp<=0 )
		{
			RtDocReader_t tInDoc ( &tSeg1, *pWords1 );
			CopyWord ( tSeg, tSeg1, tOutDoc, tInDoc, tWord, dRowMap1 );
		}

		if ( iCmp>=0 )
		{
			RtDocReader_t tInDoc ( &tSeg2, *pWords2 );
			CopyWord ( tSeg, tSeg2, tOutDoc, tInDoc, tWord, dRowMap2 );
		}

		// append word to the dictionary
		if ( tWord.m_uDocs )
			tOut.ZipWord ( tWord );

		// move forward (both readers if words are equal)
		if ( iCmp<=0 )
			pWords1 = tIn1.UnzipWord();

		if ( iCmp>=0 )
			pWords2 = tIn2.UnzipWord();
	}
}


RtSegment_t * RtIndex_c::MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, bool bHasMorphology ) const
{

	auto * pSeg = new RtSegment_t(0);

	if ( m_tSchema.HasStoredFields() )
		pSeg->SetupDocstore ( &m_tSchema );

	////////////////////
	// merge attributes
	////////////////////

	// just a shortcut
	CSphTightVector<CSphRowitem> & dRows = pSeg->m_dRows;
	CSphTightVector<BYTE> & dBlobs = pSeg->m_dBlobs;

	// we might need less because of dupes, but we can not know yet
	dRows.Reserve ( Max ( pSeg1->m_dRows.GetLength(), pSeg2->m_dRows.GetLength() ) );
	dBlobs.Reserve ( Max ( pSeg1->m_dBlobs.GetLength(), pSeg2->m_dBlobs.GetLength() ) );

	CSphVector<RowID_t> dRowMap1(pSeg1->m_uRows);
	CSphVector<RowID_t> dRowMap2(pSeg2->m_uRows);

	int nBlobAttrs = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		if ( sphIsBlobAttr ( m_tSchema.GetAttr(i).m_eAttrType ) )
			++nBlobAttrs;

	RowID_t tNextRowID = 0;

	{
		for ( auto & i : dRowMap1 )
			i = INVALID_ROWID;

		RtRowIterator_c tIt ( pSeg1, m_iStride );
		MergeAttributes ( tIt, pSeg, pSeg1, nBlobAttrs, dRowMap1, tNextRowID );
	}

	{
		for ( auto & i : dRowMap2 )
			i = INVALID_ROWID;

		RtRowIterator_c tIt ( pSeg2, m_iStride );
		MergeAttributes ( tIt, pSeg, pSeg2, nBlobAttrs, dRowMap2, tNextRowID );
	}

	assert ( tNextRowID<=INT_MAX );
	pSeg->m_uRows = tNextRowID;
	pSeg->m_tAliveRows.store ( pSeg->m_uRows, std::memory_order_relaxed );
	pSeg->m_tDeadRowMap.Reset ( pSeg->m_uRows );

	assert ( pSeg->m_uRows*m_iStride==(DWORD)pSeg->m_dRows.GetLength() );

	// merged segment might be completely killed by committed data
	if ( !pSeg->m_uRows )
	{
		SafeRelease ( pSeg );
		return pSeg;
	}

	MergeKeywords ( *pSeg, *pSeg1, *pSeg2, dRowMap1, dRowMap2 );

	if ( m_bKeywordDict )
		FixupSegmentCheckpoints ( pSeg );

	BuildSegmentInfixes ( pSeg, bHasMorphology, m_bKeywordDict, m_tSettings.m_iMinInfixLen, m_iWordsCheckpoint, ( m_iMaxCodepointLength>1 ), m_tSettings.m_eHitless );

	pSeg->BuildDocID2RowIDMap();

	assert ( pSeg->m_dRows.GetLength() );
	assert ( pSeg->m_uRows );
	assert ( pSeg->m_tAliveRows==pSeg->m_uRows );

	return pSeg;
}


bool RtIndex_c::Commit ( int * pDeleted, RtAccum_t * pAccExt )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( MEM_INDEX_RT );

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt, m_bKeywordDict );
	if ( !pAcc )
		return true;

	// empty txn, just ignore
	if ( !pAcc->m_uAccumDocs && !pAcc->m_dAccumKlist.GetLength() )
	{
		pAcc->Cleanup();
		return true;
	}

	// phase 0, build a new segment
	// accum and segment are thread local; so no locking needed yet
	// segment might be NULL if we're only killing rows this txn
	pAcc->CleanupDuplicates ( m_tSchema.GetRowSize() );
	pAcc->Sort();

	RtSegmentRefPtf_t pNewSeg { pAcc->CreateSegment ( m_tSchema.GetRowSize(), m_iWordsCheckpoint, m_tSettings.m_eHitless, m_dHitlessWords ) };
	assert ( !pNewSeg || pNewSeg->m_uRows>0 );
	assert ( !pNewSeg || pNewSeg->m_tAliveRows>0 );

	BuildSegmentInfixes ( pNewSeg, m_pDict->HasMorphology(), m_bKeywordDict, m_tSettings.m_iMinInfixLen, m_iWordsCheckpoint, ( m_iMaxCodepointLength>1 ), m_tSettings.m_eHitless );

	// clean up parts we no longer need
	pAcc->CleanupPart();

	// sort accum klist, too
	pAcc->m_dAccumKlist.Uniq ();

	// now on to the stuff that needs locking and recovery
	bool bOk = CommitReplayable ( pNewSeg, pAcc->m_dAccumKlist, pDeleted, false );

	// done; cleanup accum
	pAcc->Cleanup();

	if ( !bOk )
		return false;

	// reset accumulated warnings
	CSphString sWarning;
	pAcc->GrabLastWarning ( sWarning );

	return true;
}

RtSegmentRefPtf_t RtIndex_c::AdoptSegment ( RtSegment_t * pNewSeg )
{
	RtSegmentRefPtf_t tResult {pNewSeg};
	if ( pNewSeg )
	{
		pNewSeg->AddRef ();
		pNewSeg->m_pRAMCounter = &m_iRamChunksAllocatedRAM;
		pNewSeg->UpdateUsedRam();
	}
	return tResult;
}


int RtIndex_c::ApplyKillList ( const CSphVector<DocID_t> & dAccKlist )
{
	if ( !dAccKlist.GetLength() )
		return 0;

	int iKilled = 0;

	if ( m_iDoubleBuffer )
		m_dKillsWhileSaving.Append ( dAccKlist );

	// should be appended once not for each disk chunk
	if ( m_bOptimizing )
		m_dKillsWhileOptimizing.Append ( dAccKlist );

	// chunks got checked under m_tWriting lock - should be safe to check chunks vectors itself
	for ( auto & pChunk : m_dDiskChunks )
		iKilled += pChunk->KillMulti ( dAccKlist );

	// don't touch the chunks that are being saved
	for ( int iChunk = m_iDoubleBuffer; iChunk < m_dRamChunks.GetLength(); iChunk++ )
		iKilled += m_dRamChunks[iChunk]->KillMulti ( dAccKlist );

	return iKilled;
}

static void RemoveEmptySegments ( CSphVector<RtSegmentRefPtf_t> & dSegments )
{
	ARRAY_FOREACH ( i, dSegments )
	{
		auto & pSeg = dSegments[i];
		if ( !pSeg->m_tAliveRows.load ( std::memory_order_relaxed ) )
		{
			pSeg = nullptr; // release it
			dSegments.RemoveFast ( i );
			--i;
		}
	}
}

// returns bool meaning 'can't merge, need to flush ramchunk'.
bool RtIndex_c::MergeSegments ( CSphVector<RtSegmentRefPtf_t> & dSegments, bool bForceDump, int64_t iMemLimit, bool bHasNewSegment )
{
	// enforce RAM usage limit
	int64_t iRamLeft = SegmentsGetRamLeft ( dSegments, iMemLimit );

	// skip merging if no rows were added or no memory left
	bool bDump = ( iRamLeft==0 || bForceDump );

	// no need to merge segments in these cases:
	// - remove of document(s) from existed segments
	// - no free ram left - segments will be saved on disk
	// - forced flush of segments on disk
	// - double buffer active - lets just collect new segments but do not merge them till save finished
	if ( !bHasNewSegment || !iRamLeft || bDump || ( m_iDoubleBuffer>0 ) )
	{
		RemoveEmptySegments ( dSegments );
		return bDump;
	}

	bool bHasMorphology = m_pDict->HasMorphology();

	const int MAX_SEGMENTS = 32;
	const int MAX_PROGRESSION_SEGMENT = 8;
	const int64_t MAX_SEGMENT_VECTOR_LEN = INT_MAX;
	while ( iRamLeft>0 )
	{
		// segments sort order: large first, smallest last
		// merge last smallest segments
		dSegments.Sort ( Lesser (
				[] ( RtSegment_t * a, RtSegment_t * b ) { return a->GetMergeFactor ()>b->GetMergeFactor (); } ) );

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
#define LOC_ESTIMATE1(_seg,_vec) (int64_t)( ( (int64_t)(_seg)->_vec.GetLength() ) * (_seg)->m_tAliveRows / (_seg)->m_uRows )
#define LOC_ESTIMATE0(_vec) ( LOC_ESTIMATE1 ( dSegments[iLen-1], _vec ) + LOC_ESTIMATE1 ( dSegments[iLen-2], _vec ) )
#define LOC_ESTIMATE(_vec) ( dSegments[iLen-1]->_vec.Relimit( 0, LOC_ESTIMATE0 ( _vec ) ) )

		using namespace sph;
		int64_t iWordsRelimit =	LOC_ESTIMATE ( m_dWords );
		int64_t iDocsRelimit =	LOC_ESTIMATE ( m_dDocs );
		int64_t iHitsRelimit =	LOC_ESTIMATE ( m_dHits );
		int64_t iBlobsRelimit = LOC_ESTIMATE ( m_dBlobs );
		int64_t iKeywordsRelimit = LOC_ESTIMATE ( m_dKeywordCheckpoints );
		int64_t iRowsRelimit =	LOC_ESTIMATE ( m_dRows );

#undef LOC_ESTIMATE
#undef LOC_ESTIMATE0
#undef LOC_ESTIMATE1

		int64_t iEstimate = iWordsRelimit + iDocsRelimit + iHitsRelimit + iBlobsRelimit + iKeywordsRelimit + iRowsRelimit;
		if ( iEstimate>iRamLeft )
		{
			// dump case: can't merge any more AND segments count limit's reached
			bDump = ( iRamLeft<=iEstimate ) && ( iLen>=MAX_SEGMENTS );
			break;
		}

		// we have to dump if we can't merge even smallest segments without breaking vector constrain ( len<INT_MAX )
		// split this way to avoid superlong string after macro expansion that kills gcov
		int64_t iMaxLen = Max (
			Max ( iWordsRelimit, iDocsRelimit ),
			Max ( iHitsRelimit, iBlobsRelimit ) );
		iMaxLen = Max (
			Max ( iRowsRelimit, iKeywordsRelimit ),
			iMaxLen );

		if ( MAX_SEGMENT_VECTOR_LEN<iMaxLen )
		{
			bDump = true;
			break;
		}

		// do it
		RtSegmentRefPtf_t & pA = dSegments.Pop();
		RtSegmentRefPtf_t & pB = dSegments.Pop();
		RtSegmentRefPtf_t pMerged { MergeSegments ( pA, pB, bHasMorphology ) };
		pA = nullptr; // force releasing of parent segments
		pB = nullptr;
		if ( pMerged )
			dSegments.Add ( AdoptSegment ( pMerged ));
		iRamLeft = SegmentsGetRamLeft ( dSegments, iMemLimit );
	}

	RemoveEmptySegments ( dSegments );

	return bDump;
}

bool RtIndex_c::CommitReplayable ( RtSegment_t * pNewSeg, const CSphVector<DocID_t> & dAccKlist, int * pTotalKilled, bool bForceDump )
{
	// store statistics, because pNewSeg just might get merged
	int iNewDocs = pNewSeg ? pNewSeg->m_uRows : 0;

	CSphVector<int64_t> dLens;
	int iFirstFieldLenAttr = m_tSchema.GetAttrId_FirstFieldLen();
	if ( pNewSeg && iFirstFieldLenAttr>=0 )
	{
		assert ( pNewSeg->GetStride()==m_iStride );
		int iFields = m_tSchema.GetFieldsCount(); // shortcut
		dLens.Resize ( iFields );
		dLens.Fill ( 0 );
		for ( DWORD i=0; i<pNewSeg->m_uRows; ++i )
			for ( int j=0; j<iFields; ++j )
				dLens[j] += sphGetRowAttr ( pNewSeg->GetDocinfoByRowID(i), m_tSchema.GetAttr ( j+iFirstFieldLenAttr ).m_tLocator );
	}

	// phase 1, lock out other writers (but not readers yet)
	// concurrent readers are ok during merges, as existing segments won't be modified yet
	// however, concurrent writers are not
	Verify ( m_tWriting.Lock() );

	// in case no ram left even for double buffer
	// need to hold clients to prevent commits till save finished
	if ( m_iDoubleBuffer>0 && m_bDoubleDump )
	{
		Verify ( m_tWriting.Unlock() );
		bool bSaving = m_bDoubleDump;
		while ( bSaving )
		{
			ScopedMutex_t tLock ( m_tSaveFinished );
			bSaving = m_bDoubleDump;
		}

		Verify ( m_tWriting.Lock() );
	}

	// first of all, binlog txn data for recovery
	g_pRtBinlog->BinlogCommit ( &m_iTID, m_sIndexName.cstr(), pNewSeg, dAccKlist, m_bKeywordDict );
	int64_t iTID = m_iTID;

	auto dDoubleChunks = m_dRamChunks.Slice ( m_iDoubleBuffer ); // shortcut to slice on doublebuffer
	int64_t iMemLimit = ( m_iDoubleBuffer ? m_iDoubleBufferLimit : m_iSoftRamLimit );

	// prepare new segments vector
	// create more new segments by merging as needed
	// do not (!) kill processed old segments just yet, as readers might still need them
	CSphVector<RtSegmentRefPtf_t> dSegments;

	dSegments.Reserve ( dDoubleChunks.GetLength ()+1 );
	for ( auto & pChunk : dDoubleChunks )
		dSegments.Add ( pChunk );
	if ( pNewSeg )
		dSegments.Add ( AdoptSegment ( pNewSeg ) );

	int iTotalKilled = ApplyKillList ( dAccKlist );

	bool bDump = MergeSegments ( dSegments, bForceDump, iMemLimit, ( pNewSeg!=nullptr ) );

	// wipe out readers - now we are only using RAM segments
	m_tChunkLock.WriteLock ();

	// go live!
	// got rid of 'old' double-buffer segments then add 'new' onces
	m_dRamChunks.Resize ( m_iDoubleBuffer );
	m_dRamChunks.Append ( dSegments );
	m_dRamChunks.ZeroTail ();
	// release temporary vector of collected segments
	// as no need to keep them here after this point
	// but vector default release at function exit cause segments to live after SaveDiskChunk
	dSegments.Reset();

	// phase 3, enable readers again
	// we might need to dump data to disk now
	// but during the dump, readers can still use RAM chunk data
	Verify ( m_tChunkLock.Unlock() );

	// update stats
	m_tStats.m_iTotalDocuments += iNewDocs - iTotalKilled;

	if ( dLens.GetLength() )
		for ( int i = 0; i < m_tSchema.GetFieldsCount(); ++i )
		{
			m_dFieldLensRam[i] += dLens[i];
			m_dFieldLens[i] = m_dFieldLensRam[i] + m_dFieldLensDisk[i];
		}

	// tell about DELETE affected_rows
	if ( pTotalKilled )
		*pTotalKilled = iTotalKilled;

	// double buffer writer stands still till save done
	// all writers waiting double buffer done
	// no need to dump or waiting for some writer
	if ( !bDump || m_iDoubleBuffer>0 )
	{
		if ( bDump && m_iDoubleBuffer>0 )
			m_bDoubleDump = true;

		// all done, enable other writers
		Verify ( m_tWriting.Unlock() );
		return true;
	}

	// scope for guard for released segments to clean up
	bool bSavedOk = false;
	while ( true )
	{
		// copy stats for disk chunk
		SphChunkGuard_t tGuard = GetReaderChunks ();

		ChunkStats_t tStat2Dump ( m_tStats, m_dFieldLensRam );
		m_iDoubleBuffer = m_dRamChunks.GetLength();
		// can not use tGuard.m_dDiskChunks in SaveDiskChunk after tGuard.m_pReading unlocked below
		// due to race with Optimize that modified disk chunks
		// need release m_tReading lock to prevent deadlock - commit vs SaveDiskChunk
		// chunks will keep till this scope end
		tGuard.m_pReading->Unlock();
		tGuard.m_pReading = nullptr;

		Verify ( m_tWriting.Unlock() );

		// create pit-stop for further clients these overflows double-buffer
		// but need release them only after ram chunk guard will free segments
		// otherwise there will be another save of tiny disk chunk
		// as segments these released still count as used memory till readers release them
		m_tSaveFinished.Lock();

		int iSavedChunkId = -1;
		if ( !SaveDiskChunk ( iTID, tGuard, tStat2Dump, bForceDump, &iSavedChunkId ) )
		{
			bSavedOk = false;
			break;
		}

		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), iTID, false );

		// notify the disk chunk that we were saving of the documents that were killed while we were saving it
		ScopedMutex_t tWriteLock ( m_tWriting );
		ScWL_t tChunksWLock { m_tChunkLock };

		CSphIndex * pDiskChunk = nullptr;
		for ( auto pChunk : m_dDiskChunks )
			if ( pChunk->GetIndexId()==iSavedChunkId )
			{
				pDiskChunk = pChunk;
				break;
			}

		if ( pDiskChunk )
		{
			// kill-list might have multiple accumulators - need to sort by id asc and remove duplicates
			m_dKillsWhileSaving.Uniq();
			pDiskChunk->KillMulti ( m_dKillsWhileSaving );
		}

		m_dKillsWhileSaving.Reset();
		bSavedOk = true;
		break;
	}

	// release all clients waiting commit
	m_bDoubleDump = false;
	m_tSaveFinished.Unlock();

	return bSavedOk;
}

void RtIndex_c::RollBack ( RtAccum_t * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt, m_bKeywordDict );
	if ( !pAcc )
		return;

	pAcc->Cleanup ();
}

bool RtIndex_c::DeleteDocument ( const VecTraits_T<DocID_t> & dDocs, CSphString & sError, RtAccum_t * pAccExt )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( MEM_RT_ACCUM );

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt, m_bKeywordDict, true, &sError );
	if ( !pAcc )
		return false;

	if ( dDocs.IsEmpty() )
		return true;

	// !COMMIT should handle case when uDoc what inserted in current txn here
	pAcc->m_dAccumKlist.Append ( dDocs );

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


bool RtIndex_c::ForceDiskChunk()
{
	MEMORY ( MEM_INDEX_RT );

	if ( m_dRamChunks.IsEmpty() || m_bSaveDisabled )
		return true;

	CSphVector<DocID_t> dTmp;
	return CommitReplayable ( nullptr, dTmp, nullptr, true );
}


struct SaveDiskDataContext_t
{
	SphOffset_t						m_tDocsOffset {0};
	SphOffset_t						m_tLastDocPos {0};
	SphOffset_t						m_tCheckpointsPosition {0};
	SphOffset_t						m_tMinMaxPos {0};
	DWORD							m_uRows {0};
	int64_t							m_iDocinfoIndex {0};
	int64_t							m_iTotalDocs {0};
	int64_t							m_iInfixBlockOffset {0};
	int								m_iInfixCheckpointWordsSize {0};
	ISphInfixBuilder *				m_pInfixer {nullptr};
	CSphVector<Checkpoint_t>		m_dCheckpoints;
	CSphVector<BYTE>				m_dKeywordCheckpoints;
	CSphVector<CSphVector<RowID_t>>	m_dRowMaps;
	const char *					m_szFilename;
	const SphChunkGuard_t &			m_tGuard;

	SaveDiskDataContext_t ( const char * szFilename, const SphChunkGuard_t & tGuard )
		: m_szFilename ( szFilename )
		, m_tGuard ( tGuard )
	{
		m_dRowMaps.Resize ( tGuard.m_dRamChunks.GetLength() );

		ARRAY_FOREACH ( i, m_dRowMaps )
		{
			m_dRowMaps[i].Resize ( tGuard.m_dRamChunks[i]->m_uRows );
			for ( auto & j : m_dRowMaps[i] )
				j = INVALID_ROWID;
		}
	}

	~SaveDiskDataContext_t()
	{
		delete m_pInfixer;
	}
};


bool RtIndex_c::WriteAttributes ( SaveDiskDataContext_t & tCtx, CSphString & sError ) const
{
	CSphString sSPA, sSPB, sSPT, sSPHI, sSPDS;
	CSphWriter tWriterSPA;

	sSPA.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPA).cstr() );
	sSPB.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPB).cstr() );
	sSPT.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPT).cstr() );
	sSPHI.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPHI).cstr() );
	sSPDS.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPDS).cstr() );

	if ( !tWriterSPA.OpenFile ( sSPA.cstr(), sError ) )
		return false;

	const CSphColumnInfo * pBlobLocatorAttr = m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	AttrIndexBuilder_c tMinMaxBuilder(m_tSchema);

	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder(nullptr);
	if ( pBlobLocatorAttr )
	{
		pBlobRowBuilder = sphCreateBlobRowBuilder ( m_tSchema, sSPB, m_tSettings.m_tBlobUpdateSpace, sError );
		if ( !pBlobRowBuilder.Ptr() )
			return false;
	}

	CSphScopedPtr<DocstoreBuilder_i> pDocstoreBuilder(nullptr);
	if ( m_tSchema.HasStoredFields() )
	{
		pDocstoreBuilder = CreateDocstoreBuilder ( sSPDS, m_tSettings, sError );
		if ( !pDocstoreBuilder.Ptr() )
			return false;

		SetupDocstoreFields ( *pDocstoreBuilder.Ptr(), m_tSchema );
	}

	tCtx.m_iTotalDocs = 0;
	for ( const auto & i : tCtx.m_tGuard.m_dRamChunks )
		tCtx.m_iTotalDocs += i->m_tAliveRows.load ( std::memory_order_relaxed );

	CSphFixedVector<DocidRowidPair_t> dLookup ( tCtx.m_iTotalDocs );

	RowID_t tNextRowID = 0;
	int iStride = m_tSchema.GetRowSize();
	CSphFixedVector<CSphRowitem> dRow ( iStride );
	CSphRowitem * pNewRow = dRow.Begin();
	ARRAY_FOREACH ( i, tCtx.m_tGuard.m_dRamChunks )
	{
		const RtSegment_t * pSeg = tCtx.m_tGuard.m_dRamChunks[i];
		assert ( pSeg );

		RtRowIterator_c tIt ( pSeg, iStride );
		const CSphRowitem * pRow;
		while ( !!(pRow=tIt.GetNextAliveRow()) )
		{
			tMinMaxBuilder.Collect(pRow);
			if ( pBlobLocatorAttr )
			{
				SphAttr_t tBlobOffset = sphGetRowAttr ( pRow, pBlobLocatorAttr->m_tLocator );
				SphOffset_t tOffset = pBlobRowBuilder->Flush ( pSeg->m_dBlobs.Begin() + tBlobOffset );

				memcpy ( pNewRow, pRow, iStride*sizeof(CSphRowitem) );
				sphSetRowAttr ( pNewRow, pBlobLocatorAttr->m_tLocator, tOffset );
				tWriterSPA.PutBytes ( pNewRow, iStride*sizeof(CSphRowitem) );
			} else
				tWriterSPA.PutBytes ( pRow, iStride*sizeof(CSphRowitem) );

			dLookup[tNextRowID].m_tDocID = sphGetDocID(pRow);
			dLookup[tNextRowID].m_tRowID = tNextRowID;

			if ( pDocstoreBuilder.Ptr() )
			{
				assert ( pSeg->m_pDocstore.Ptr() );
				pDocstoreBuilder->AddDoc ( tNextRowID, pSeg->m_pDocstore->GetDoc ( tIt.GetRowID(), nullptr, -1, false ) );
			}

			tCtx.m_dRowMaps[i][tIt.GetRowID()] = tNextRowID++;
		}
	}

	if ( pBlobRowBuilder.Ptr() && !pBlobRowBuilder->Done(sError) )
		return false;

	if ( pDocstoreBuilder.Ptr() )
		pDocstoreBuilder->Finalize();

	dLookup.Sort ( bind ( &DocidRowidPair_t::m_tDocID ) );

	if ( !WriteDocidLookup ( sSPT, dLookup, sError ) )
		return false;

	dLookup.Reset(0);

	tMinMaxBuilder.FinishCollect();
	const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMaxBuilder.GetCollected();
	const CSphRowitem * pMinRow = dMinMaxRows.Begin()+dMinMaxRows.GetLength()-iStride*2;
	const CSphRowitem * pMaxRow = pMinRow+iStride;

	// create the histograms
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

	// iterate one more time to collect histogram data
	ARRAY_FOREACH ( i, tCtx.m_tGuard.m_dRamChunks )
	{
		const RtSegment_t * pSeg = tCtx.m_tGuard.m_dRamChunks[i];
		assert ( pSeg );

		RtRowIterator_c tIt ( pSeg, iStride );
		const CSphRowitem * pRow;
		while ( !!(pRow=tIt.GetNextAliveRow()) )
		{
			ARRAY_FOREACH ( i, dHistograms )
				dHistograms[i]->Insert ( sphGetRowAttr ( pRow,  dPOD[i].m_tLocator ) );
		}
	}

	if ( !tHistogramContainer.Save ( sSPHI, sError ) )
		return false;

	tCtx.m_tMinMaxPos = tWriterSPA.GetPos();
	tCtx.m_uRows = tNextRowID;
	tCtx.m_iDocinfoIndex = ( dMinMaxRows.GetLength() / m_tSchema.GetRowSize() / 2 ) - 1;
	tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLength()*sizeof(CSphRowitem) );

	return true;
}


bool RtIndex_c::WriteDocs ( SaveDiskDataContext_t & tCtx, CSphWriter & tWriterDict, CSphString & sError ) const
{
	CSphWriter tWriterHits, tWriterDocs, tWriterSkips;

	CSphString sName;
	sName.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPP).cstr() );
	if ( !tWriterHits.OpenFile ( sName.cstr(), sError ) )
		return false;

	sName.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPD).cstr() );
	if ( !tWriterDocs.OpenFile ( sName.cstr(), sError ) )
		return false;

	sName.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPE).cstr() );
	if ( !tWriterSkips.OpenFile ( sName.cstr(), sError ) )
		return false;

	tWriterHits.PutByte(1);
	tWriterDocs.PutByte(1);
	tWriterSkips.PutByte(1);

	int iSegments = tCtx.m_tGuard.m_dRamChunks.GetLength();

	CSphVector<RtWordReader_t*> dWordReaders(iSegments);
	CSphVector<const RtWord_t*> dWords(iSegments);

	ARRAY_FOREACH ( i, dWordReaders )
		dWordReaders[i] = new RtWordReader_t ( tCtx.m_tGuard.m_dRamChunks[i], m_bKeywordDict, m_iWordsCheckpoint, m_tSettings.m_eHitless );

	ARRAY_FOREACH ( i, dWords )
		dWords[i] = dWordReaders[i]->UnzipWord();

	// loop keywords
	int iWords = 0;
	CSphKeywordDeltaWriter tLastWord;
	SphWordID_t uLastWordID = 0;
	CSphVector<SkiplistEntry_t> dSkiplist;

	tCtx.m_tLastDocPos = 0;

	bool bHasMorphology = m_pDict->HasMorphology();
	int iSkiplistBlockSize = m_tSettings.m_iSkiplistBlockSize;
	assert ( iSkiplistBlockSize>0 );

	while (true)
	{
		// find keyword with min id
		const RtWord_t * pWord = nullptr;
		for ( auto & i : dWords )
			if ( CompareWords ( i, pWord ) < 0 )
				pWord = i;

		if ( !pWord )
			break;

		SphOffset_t uDocpos = tWriterDocs.GetPos();
		SphOffset_t uLastHitpos = 0;
		RowID_t tLastRowID = INVALID_ROWID;
		RowID_t tSkiplistRowID = INVALID_ROWID;
		int iDocs = 0;
		int iHits = 0;
		dSkiplist.Resize(0);

		// loop all segments that have this keyword
		CSphBitvec tSegsWithWord ( iSegments );

		ARRAY_FOREACH ( iSegment, dWords )
		{
			if ( !CompareWords ( dWords[iSegment], pWord ) )
				tSegsWithWord.BitSet(iSegment);
			else
				continue;

			RtDocReader_t tDocReader ( tCtx.m_tGuard.m_dRamChunks[iSegment], *dWords[iSegment] );

			const RtDoc_t * pDoc;
			while ( !!( pDoc = tDocReader.UnzipDoc() ) )
			{
				RowID_t tRowID = tCtx.m_dRowMaps[iSegment][pDoc->m_tRowID];
				if ( tRowID==INVALID_ROWID )
					continue;

				// build skiplist, aka save decoder state as needed
				if ( ( iDocs & ( iSkiplistBlockSize-1 ) )==0 )
				{
					SkiplistEntry_t & t = dSkiplist.Add();
					t.m_tBaseRowIDPlus1 = tSkiplistRowID+1;
					t.m_iOffset = tWriterDocs.GetPos();
					t.m_iBaseHitlistPos = uLastHitpos;
				}

				iDocs++;
				iHits += pDoc->m_uHits;
				tSkiplistRowID = tRowID;

				tWriterDocs.ZipOffset ( tRowID - tLastRowID );
				tWriterDocs.ZipInt ( pDoc->m_uHits );
				if ( pDoc->m_uHits==1 )
				{
					tWriterDocs.ZipInt ( pDoc->m_uHit & 0x7FFFFFUL );
					tWriterDocs.ZipInt ( pDoc->m_uHit >> 23 );
				} else
				{
					tWriterDocs.ZipInt ( pDoc->m_uDocFields );
					tWriterDocs.ZipOffset ( tWriterHits.GetPos() - uLastHitpos );
					uLastHitpos = tWriterHits.GetPos();
				}

				tLastRowID = tRowID;

				// loop hits from current segment
				if ( pDoc->m_uHits>1 )
				{
					DWORD uLastHit = 0;
					RtHitReader_t tInHit ( tCtx.m_tGuard.m_dRamChunks[iSegment], pDoc );
					for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
					{
						tWriterHits.ZipInt ( uValue - uLastHit );
						uLastHit = uValue;
					}

					tWriterHits.ZipInt(0);
				}
			}
		}

		// write skiplist
		int64_t iSkiplistOff = tWriterSkips.GetPos();
		for ( int i=1; i<dSkiplist.GetLength(); i++ )
		{
			const SkiplistEntry_t & tPrev = dSkiplist[i-1];
			const SkiplistEntry_t & tCur = dSkiplist[i];
			assert ( tCur.m_tBaseRowIDPlus1 - tPrev.m_tBaseRowIDPlus1>=(DWORD)iSkiplistBlockSize );
			assert ( tCur.m_iOffset - tPrev.m_iOffset>=4*iSkiplistBlockSize );
			tWriterSkips.ZipInt ( tCur.m_tBaseRowIDPlus1 - tPrev.m_tBaseRowIDPlus1 - iSkiplistBlockSize );
			tWriterSkips.ZipOffset ( tCur.m_iOffset - tPrev.m_iOffset - 4*iSkiplistBlockSize );
			tWriterSkips.ZipOffset ( tCur.m_iBaseHitlistPos - tPrev.m_iBaseHitlistPos );
		}

		// write dict entry if necessary
		if ( tWriterDocs.GetPos()!=uDocpos )
		{
			tWriterDocs.ZipInt ( 0 ); // docs over

			if ( ( iWords%SPH_WORDLIST_CHECKPOINT )==0 )
			{
				if ( iWords )
				{
					SphOffset_t uOff = m_bKeywordDict ? 0 : uDocpos - tCtx.m_tLastDocPos;
					tWriterDict.ZipInt ( 0 );
					tWriterDict.ZipOffset ( uOff ); // store last hitlist length
				}

				// restart delta coding, once per SPH_WORDLIST_CHECKPOINT entries
				tCtx.m_tLastDocPos = 0;
				uLastWordID = 0;
				tLastWord.Reset();

				// begin new wordlist entry
				Checkpoint_t & tChk = tCtx.m_dCheckpoints.Add();
				tChk.m_uOffset = tWriterDict.GetPos();
				if ( m_bKeywordDict )
					tChk.m_uWord = sphPutBytes ( &tCtx.m_dKeywordCheckpoints, pWord->m_sWord, pWord->m_sWord[0]+1 ); // copy word len + word itself to checkpoint storage
				else
					tChk.m_uWord = pWord->m_uWordID;
			}

			iWords++;

			if ( m_bKeywordDict )
			{
				tLastWord.PutDelta ( tWriterDict, pWord->m_sWord+1, pWord->m_sWord[0] );
				tWriterDict.ZipOffset ( uDocpos );
			} else
			{
				assert ( pWord->m_uWordID!=uLastWordID );
				tWriterDict.ZipOffset ( pWord->m_uWordID - uLastWordID );
				uLastWordID = pWord->m_uWordID;
				assert ( uDocpos>tCtx.m_tLastDocPos );
				tWriterDict.ZipOffset ( uDocpos - tCtx.m_tLastDocPos );
			}

			DWORD iDocsCount = iDocs;
			if ( !pWord->m_bHasHitlist )
				iDocsCount |= HITLESS_DOC_FLAG;

			tWriterDict.ZipInt ( iDocsCount );
			tWriterDict.ZipInt ( iHits );

			if ( m_bKeywordDict )
			{
				BYTE uHint = sphDoclistHintPack ( iDocs, tWriterDocs.GetPos()-tCtx.m_tLastDocPos );
				if ( uHint )
					tWriterDict.PutByte ( uHint );

				// build infixes
				if ( tCtx.m_pInfixer )
					tCtx.m_pInfixer->AddWord ( pWord->m_sWord+1, pWord->m_sWord[0], tCtx.m_dCheckpoints.GetLength(), bHasMorphology );
			}

			// emit skiplist pointer
			if ( iDocs>iSkiplistBlockSize )
				tWriterDict.ZipOffset ( iSkiplistOff );

			tCtx.m_tLastDocPos = uDocpos;
		}

		// read next words
		for ( int i = 0; i < tSegsWithWord.GetBits(); i++ )
			if ( tSegsWithWord.BitGet(i) )
				dWords[i] = dWordReaders[i]->UnzipWord();
	}

	tCtx.m_tDocsOffset = tWriterDocs.GetPos();

	for ( auto i : dWordReaders )
		delete i;

	return true;
}


void RtIndex_c::WriteCheckpoints ( SaveDiskDataContext_t & tCtx, CSphWriter & tWriterDict ) const
{
	// write checkpoints
	SphOffset_t uOff = m_bKeywordDict ? 0 : tCtx.m_tDocsOffset - tCtx.m_tLastDocPos;

	// FIXME!!! don't write to wrDict if iWords==0
	// however plain index becomes m_bIsEmpty and full scan does not work there
	// we'll get partly working RT ( RAM chunk works and disk chunks give empty result set )
	tWriterDict.ZipInt ( 0 ); // indicate checkpoint
	tWriterDict.ZipOffset ( uOff ); // store last doclist length

	// flush infix hash entries, if any
	if ( tCtx.m_pInfixer )
		tCtx.m_pInfixer->SaveEntries ( tWriterDict );

	tCtx.m_tCheckpointsPosition = tWriterDict.GetPos();
	if ( m_bKeywordDict )
	{
		const char * pCheckpoints = (const char *)tCtx.m_dKeywordCheckpoints.Begin();
		for ( const auto & i : tCtx.m_dCheckpoints )
		{
			const char * pPacked = pCheckpoints + i.m_uWord;
			int iLen = *pPacked;
			assert ( iLen && (int)i.m_uWord+1+iLen<=tCtx.m_dKeywordCheckpoints.GetLength() );
			tWriterDict.PutDword ( iLen );
			tWriterDict.PutBytes ( pPacked+1, iLen );
			tWriterDict.PutOffset ( i.m_uOffset );
		}
	} else
	{
		for ( const auto & i : tCtx.m_dCheckpoints )
		{
			tWriterDict.PutOffset ( i.m_uWord );
			tWriterDict.PutOffset ( i.m_uOffset );
		}
	}

	// flush infix hash blocks
	if ( tCtx.m_pInfixer )
	{
		tCtx.m_iInfixBlockOffset = tCtx.m_pInfixer->SaveEntryBlocks ( tWriterDict );
		tCtx.m_iInfixCheckpointWordsSize = tCtx.m_pInfixer->GetBlocksWordsSize();

		if ( tCtx.m_iInfixBlockOffset>UINT_MAX )
			sphWarning ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", tCtx.m_iInfixBlockOffset );
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	tWriterDict.PutBytes ( "dict-header", 11 );
	tWriterDict.ZipInt ( tCtx.m_dCheckpoints.GetLength() );
	tWriterDict.ZipOffset ( tCtx.m_tCheckpointsPosition );
	tWriterDict.ZipInt ( m_pTokenizer->GetMaxCodepointLength() );
	tWriterDict.ZipInt ( (DWORD)tCtx.m_iInfixBlockOffset );
}


bool RtIndex_c::WriteDeadRowMap ( SaveDiskDataContext_t & tContext, CSphString & sError ) const
{
	CSphString sName;
	sName.SetSprintf ( "%s%s", tContext.m_szFilename, sphGetExt(SPH_EXT_SPM).cstr() );

	return ::WriteDeadRowMap ( sName, tContext.m_uRows, sError );
}


void RtIndex_c::SaveDiskData ( const char * szFilename, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats ) const
{
	CSphString sName, sError; // FIXME!!! report collected (sError) errors

	CSphWriter tWriterDict;

	sName.SetSprintf ( "%s%s", szFilename, sphGetExt(SPH_EXT_SPI).cstr() );
	tWriterDict.OpenFile ( sName.cstr(), sError );
	tWriterDict.PutByte(1);

	SaveDiskDataContext_t tCtx ( szFilename, tGuard );

	CSphScopedPtr<ISphInfixBuilder> pInfixer(nullptr);
	if ( m_tSettings.m_iMinInfixLen && m_pDict->GetSettings().m_bWordDict )
		tCtx.m_pInfixer = sphCreateInfixBuilder ( m_pTokenizer->GetMaxCodepointLength(), &sError );

	// fixme: handle errors
	WriteAttributes ( tCtx, sError );
	WriteDocs ( tCtx, tWriterDict, sError );
	WriteCheckpoints ( tCtx, tWriterDict );
	WriteDeadRowMap ( tCtx, sError );

	SaveDiskHeader ( tCtx, tStats );
}


static void FixupIndexSettings ( CSphIndexSettings & tSettings )
{
	tSettings.m_eHitFormat = SPH_HIT_FORMAT_INLINE;
	tSettings.m_iBoundaryStep = 0;
	tSettings.m_iStopwordStep = 1;
	tSettings.m_iOvershortStep = 1;
}


void RtIndex_c::SaveDiskHeader ( SaveDiskDataContext_t & tCtx, const ChunkStats_t & tStats ) const
{
	CSphWriter tWriter;
	CSphString sName, sError;
	sName.SetSprintf ( "%s%s", tCtx.m_szFilename, sphGetExt(SPH_EXT_SPH).cstr() );
	tWriter.OpenFile ( sName.cstr(), sError );

	// format
	tWriter.PutDword ( INDEX_MAGIC_HEADER );
	tWriter.PutDword ( INDEX_FORMAT_VERSION );

	// schema
	WriteSchema ( tWriter, m_tSchema );

	// wordlist checkpoints
	tWriter.PutOffset ( tCtx.m_tCheckpointsPosition );
	tWriter.PutDword ( tCtx.m_dCheckpoints.GetLength() );

	int iInfixCodepointBytes = ( m_tSettings.m_iMinInfixLen && m_pDict->GetSettings().m_bWordDict ? m_pTokenizer->GetMaxCodepointLength() : 0 );
	tWriter.PutByte ( iInfixCodepointBytes ); // m_iInfixCodepointBytes, v.27+
	tWriter.PutDword ( tCtx.m_iInfixBlockOffset ); // m_iInfixBlocksOffset, v.27+
	tWriter.PutDword ( tCtx.m_iInfixCheckpointWordsSize ); // m_iInfixCheckpointWordsSize, v.34+

	// stats
	tWriter.PutDword ( (DWORD)tCtx.m_iTotalDocs ); // FIXME? we don't expect over 4G docs per just 1 local index
	tWriter.PutOffset ( tStats.m_Stats.m_iTotalBytes );

	// index settings
	// FIXME: remove this?
	CSphIndexSettings tSettings = m_tSettings;
	FixupIndexSettings ( tSettings );
	SaveIndexSettings ( tWriter, tSettings );

	// tokenizer
	SaveTokenizerSettings ( tWriter, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );

	// dictionary
	// can not use embedding as stopwords id differs between RT and plain dictionaries
	SaveDictionarySettings ( tWriter, m_pDict, m_bKeywordDict, 0 );

	// min-max count
	tWriter.PutOffset ( tCtx.m_uRows );
	tWriter.PutOffset ( tCtx.m_iDocinfoIndex );
	tWriter.PutOffset ( tCtx.m_tMinMaxPos/sizeof(CSphRowitem) );		// min-max count

	// field filter
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( m_pFieldFilter.Ptr() )
		m_pFieldFilter->GetSettings(tFieldFilterSettings);
	tFieldFilterSettings.Save(tWriter);

	// field lengths
	if ( m_tSettings.m_bIndexFieldLens )
		for ( int i=0; i <m_tSchema.GetFieldsCount(); i++ )
			tWriter.PutOffset ( tStats.m_dFieldLens[i] );

	// done
	tWriter.CloseFile ();
}

void RtIndex_c::SaveMetaSpecial ( int64_t iTID, const VecTraits_T<int> & dChunkNames )
{
	if ( m_bSaveDisabled )
		return;

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
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( m_pFieldFilter.Ptr() )
		m_pFieldFilter->GetSettings(tFieldFilterSettings);
	tFieldFilterSettings.Save(wrMeta);

	// meta v.12
	wrMeta.PutDword ( dChunkNames.GetLength () );
	wrMeta.PutBytes ( dChunkNames.Begin(), dChunkNames.GetLengthBytes() );

	// meta v.17+
	wrMeta.PutOffset(m_iSoftRamLimit);

	wrMeta.CloseFile();

	// no need to remove old but good meta in case new meta failed to save
	if ( wrMeta.IsError() )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	// rename
	if ( sph::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMetaNew.cstr(), sMeta.cstr(), errno, strerrorm(errno) ); // !COMMIT handle this gracefully
}

void RtIndex_c::SaveMeta ( int64_t iTID, bool bSaveChunks )
{
	if ( bSaveChunks )
	{
		CSphFixedVector<int> dChunkNames = GetChunkNames ( m_dDiskChunks );
		SaveMetaSpecial ( iTID, dChunkNames );
	} else
		SaveMetaSpecial ( iTID, CSphFixedVector<int> ( 0 ) );
}


RtSegmentRefPtf_t RtIndex_c::MergeDoubleBufSegments ( CSphVector<RtSegmentRefPtf_t> & dSegments ) const
{
	RtSegmentRefPtf_t tLast;
	if ( !dSegments.GetLength() )
		return tLast;

	// lets also merge all double buffer segments into single one
	// as these were not merged during save
	tLast = dSegments[0];

	bool bHasMorphology = m_pDict->HasMorphology();

	for ( int i=1; i<dSegments.GetLength(); i++ )
	{
		RtSegmentRefPtf_t & tNext = dSegments[i];
		RtSegmentRefPtf_t tMerged { MergeSegments ( tLast, tNext, bHasMorphology ) };

		// force releasing of parent segments
		tLast = nullptr;
		tNext = nullptr;
		tLast = tMerged;
	}

	return tLast;
}

int RtIndex_c::GetNextChunkName ()
{
	if ( m_iMaxChunkName < 0 )
	{
		int& iCh = m_iMaxChunkName;
		m_dDiskChunks.for_each ( [&iCh] ( const CSphIndex * pIdx ) { iCh = Max ( iCh, pIdx->m_iChunk ); });
	}
	++m_iMaxChunkName;
	return m_iMaxChunkName;
}


bool RtIndex_c::SaveDiskChunk ( int64_t iTID, const SphChunkGuard_t & tGuard, const ChunkStats_t & tStats, bool bForce, int * pSavedChunkId )
{
	if ( tGuard.m_dRamChunks.IsEmpty() || m_bSaveDisabled )
	{
		if ( pSavedChunkId )
			*pSavedChunkId = -1;
		return true;
	}

	int64_t tmSave = sphMicroTimer ();

	MEMORY ( MEM_INDEX_RT );

	// dump it
	int iChunk = GetNextChunkName();
	CSphString sNewChunk;
	sNewChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunk );
	SaveDiskData ( sNewChunk.cstr(), tGuard, tStats );

	// bring new disk chunk online
	CreateFilenameBuilder_fn fnCreateFilenameBuilder = GetIndexFilenameBuilder();
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( fnCreateFilenameBuilder ? fnCreateFilenameBuilder ( m_sIndexName.cstr() ) : nullptr );
	CSphIndex * pDiskChunk = PreallocDiskChunk ( sNewChunk.cstr(), iChunk, pFilenameBuilder.Ptr(), m_sLastError );
	if ( !pDiskChunk )
	{
		sphWarning ( "rt: index %s failed to load disk chunk after RAM save: %s", m_sIndexName.cstr(), m_sLastError.cstr() );
		return false;
	}

	// get exclusive lock again, gotta reset RAM chunk now
	Verify ( m_tWriting.Lock() );

	Verify ( m_tChunkLock.ReadLock() );
	CSphVector<RtSegmentRefPtf_t> dSegments;
	if ( m_iDoubleBuffer && m_iDoubleBuffer!=m_dRamChunks.GetLength() )
	{
		dSegments.Reserve ( m_dRamChunks.GetLength()-m_iDoubleBuffer );
		for ( int i=m_iDoubleBuffer; i<m_dRamChunks.GetLength(); i++ )
			dSegments.Add ( m_dRamChunks[i] );
	}
	Verify ( m_tChunkLock.Unlock() );
	// merge all double buffer segments into single one as these were not merged during save
	RtSegmentRefPtf_t tLast = MergeDoubleBufSegments ( dSegments );

	Verify ( m_tChunkLock.WriteLock() );

	m_dDiskChunks.Add ( pDiskChunk );
	// save updated meta
	SaveMeta ( iTID );
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// swap double buffer data
	m_dRamChunks.Resize ( 0 );
	m_dRamChunks.ZeroTail();
	if ( tLast.Ptr() )
		m_dRamChunks.Add ( AdoptSegment ( tLast ) );

	int iSavedChunkId = pDiskChunk->GetIndexId();

	// update field lengths
	if ( m_tSchema.GetAttrId_FirstFieldLen()>=0 )
	{
		ARRAY_FOREACH ( i, m_dFieldLensRam )
			m_dFieldLensRam[i] -= tStats.m_dFieldLens[i];
		ARRAY_FOREACH ( i, m_dFieldLensDisk )
			m_dFieldLensDisk[i] += tStats.m_dFieldLens[i];
	}

	Verify ( m_tChunkLock.Unlock() );

	// abandon .ram file
	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	if ( sphIsReadable ( sChunk.cstr() ) && ::unlink ( sChunk.cstr() ) )
		sphWarning ( "failed to unlink ram chunk (file=%s, errno=%d, error=%s)", sChunk.cstr(), errno, strerrorm(errno) );

	m_iDoubleBuffer = 0;
	m_iSavedTID = iTID;
	m_tmSaved = sphMicroTimer();

	Verify ( m_tWriting.Unlock() );

	tmSave = sphMicroTimer () - tmSave;
	const char* sReason = bForce ? "forcibly saved" : "saved";
	sphInfo ( "rt: index %s: diskchunk %d(%d), segments %d %s in %d.%03d sec",
			  m_sIndexName.cstr (), iSavedChunkId, m_dDiskChunks.GetLength(), tGuard.m_dRamChunks.GetLength(), sReason,
			  ( int ) ( tmSave / 1000000 ), ( int ) (( tmSave / 1000 ) % 1000 ));

	if ( pSavedChunkId )
		*pSavedChunkId = iSavedChunkId;
	return true;
}


CSphIndex * RtIndex_c::PreallocDiskChunk ( const char * sChunk, int iChunk, FilenameBuilder_i * pFilenameBuilder, CSphString & sError, const char * sName ) const
{
	MEMORY ( MEM_INDEX_DISK );

	// !COMMIT handle errors gracefully instead of dying
	CSphIndex * pDiskChunk = sphCreateIndexPhrase ( ( sName ? sName : sChunk ), sChunk );
	if ( !pDiskChunk )
	{
		sError.SetSprintf ( "disk chunk %s: alloc failed", sChunk );
		return NULL;
	}

	pDiskChunk->m_iExpansionLimit = m_iExpansionLimit;
	pDiskChunk->m_iExpandKeywords = m_iExpandKeywords;
	pDiskChunk->SetBinlog ( false );
	pDiskChunk->SetMemorySettings ( m_tFiles );
	pDiskChunk->m_iChunk = iChunk;

	if ( m_bDebugCheck )
		pDiskChunk->SetDebugCheck ( m_bCheckIdDups, -1 );

	if ( !pDiskChunk->Prealloc ( m_bPathStripped, pFilenameBuilder ) )
	{
		sError.SetSprintf ( "disk chunk %s: prealloc failed: %s", sChunk, pDiskChunk->GetLastError().cstr() );
		SafeDelete ( pDiskChunk );
		return NULL;
	}

	return pDiskChunk;
}


bool RtIndex_c::LoadMeta ( FilenameBuilder_i * pFilenameBuilder, bool bStripPath, DWORD & uVersion, bool & bRebuildInfixes )
{
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

	uVersion = rdMeta.GetDword();
	if ( uVersion==0 || uVersion>META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sMeta.cstr(), uVersion, META_VERSION );
		return false;
	}

	DWORD uMinFormatVer = 14;
	if ( uVersion<uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "indexes with meta prior to v.%d are no longer supported (use index_converter tool); %s is v.%d", uMinFormatVer, sMeta.cstr(), uVersion );
		return false;
	}

	m_tStats.m_iTotalDocuments = rdMeta.GetDword();
	m_tStats.m_iTotalBytes = rdMeta.GetOffset();
	m_iTID = rdMeta.GetOffset();

	// tricky bit
	// we started saving settings into .meta from v.4 and up only
	// and those reuse disk format version, aka INDEX_FORMAT_VERSION
	// anyway, starting v.4, serialized settings take precedence over config
	// so different chunks can't have different settings any more
	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;
	CSphEmbeddedFiles tEmbeddedFiles;
	CSphString sWarning;

	// load them settings
	DWORD uSettingsVer = rdMeta.GetDword();
	CSphSchema tSchema;
	ReadSchema ( rdMeta, tSchema, uSettingsVer );
	SetSchema ( tSchema );
	LoadIndexSettings ( m_tSettings, rdMeta, uSettingsVer );
	if ( !tTokenizerSettings.Load ( pFilenameBuilder, rdMeta, tEmbeddedFiles, m_sLastError ) )
		return false;

	tDictSettings.Load ( rdMeta, tEmbeddedFiles, sWarning );

	m_bKeywordDict = tDictSettings.m_bWordDict;

	// initialize AOT if needed
	DWORD uPrevAot = m_tSettings.m_uAotFilterMask;
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );
	if ( m_tSettings.m_uAotFilterMask!=uPrevAot )
		sphWarning ( "index '%s': morphology option changed from config has no effect, ignoring", m_sIndexName.cstr() );

	if ( bStripPath )
	{
		StripPath ( tTokenizerSettings.m_sSynonymsFile );
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	// recreate tokenizer
	StrVec_t dWarnings;
	m_pTokenizer = ISphTokenizer::Create ( tTokenizerSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError );
	if ( !m_pTokenizer )
		return false;

	// recreate dictionary
	m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, m_sIndexName.cstr(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError );
	if ( !m_sLastError.IsEmpty() )
		m_sLastError.SetSprintf ( "index '%s': %s", m_sIndexName.cstr(), m_sLastError.cstr() );

	if ( !m_pDict )
		return false;

	if ( !m_sLastError.IsEmpty() )
		sphWarning ( "%s", m_sLastError.cstr() );

	m_pTokenizer = ISphTokenizer::CreateMultiformFilter ( m_pTokenizer, m_pDict->GetMultiWordforms () );

	m_iWordsCheckpoint = rdMeta.GetDword();

	// check that infixes definition changed - going to rebuild infixes
	m_iMaxCodepointLength = rdMeta.GetDword();
	int iBloomKeyLen = rdMeta.GetByte();
	int iBloomHashesCount = rdMeta.GetByte();
	bRebuildInfixes = ( iBloomKeyLen!=BLOOM_PER_ENTRY_VALS_COUNT || iBloomHashesCount!=BLOOM_HASHES_COUNT );

	if ( bRebuildInfixes )
		sphWarning ( "infix definition changed (from len=%d, hashes=%d to len=%d, hashes=%d) - rebuilding...",
			(int)BLOOM_PER_ENTRY_VALS_COUNT, (int)BLOOM_HASHES_COUNT, iBloomKeyLen, iBloomHashesCount );

	FieldFilterRefPtr_c pFieldFilter;
	CSphFieldFilterSettings tFieldFilterSettings;
	tFieldFilterSettings.Load(rdMeta);
	if ( tFieldFilterSettings.m_dRegexps.GetLength() )
		pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

	if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokenizerSettings, sMeta.cstr(), m_sLastError ) )
		return false;

	SetFieldFilter ( pFieldFilter );

	int iLen = (int)rdMeta.GetDword();
	m_dChunkNames.Reset ( iLen );
	rdMeta.GetBytes ( m_dChunkNames.Begin(), iLen*sizeof(int) );

	if ( uVersion>=17 )
		m_iSoftRamLimit = rdMeta.GetOffset();

	return true;
}


bool RtIndex_c::PreallocDiskChunks ( FilenameBuilder_i * pFilenameBuilder )
{
	// load disk chunks, if any
	ARRAY_FOREACH ( iName, m_dChunkNames )
	{
		int iChunkIndex = m_dChunkNames[iName];
		CSphString sChunk;
		sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunkIndex );
		CSphIndex * pIndex = PreallocDiskChunk ( sChunk.cstr(), iChunkIndex, pFilenameBuilder, m_sLastError );
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
				for ( int i=0; i < pIndex->GetMatchSchema().GetFieldsCount(); i++ )
					m_dFieldLensDisk[i] += pLens[i];
		}
	}

	m_dChunkNames.Reset(0);
	return true;
}


bool RtIndex_c::Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder )
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
		m_sLastError.SetSprintf ( "failed to open %s: %s", sLock.cstr(), strerrorm(errno) );
		return false;
	}

	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		if ( !m_bDebugCheck )
		{
			m_sLastError.SetSprintf ( "failed to lock %s: %s", sLock.cstr(), strerrorm(errno) );
			SafeClose ( m_iLockFD );
			return false;
		} else
		{
			SafeClose ( m_iLockFD );
		}
	}

	DWORD uVersion = 0;
	bool bRebuildInfixes = false;
	if ( !LoadMeta ( pFilenameBuilder, bStripPath, uVersion, bRebuildInfixes ) )
		return false;

	if ( m_bDebugCheck )
		return true;

	m_bPathStripped = bStripPath;

	if ( !PreallocDiskChunks ( pFilenameBuilder ) )
		return false;

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


void RtIndex_c::Preread ()
{
	ScRL_t tChunkRLock ( m_tChunkLock );
	for ( auto & dDiskChunk : m_dDiskChunks )
		dDiskChunk->Preread();
}


void RtIndex_c::SetMemorySettings ( const FileAccessSettings_t & tFileAccessSettings )
{
	m_tFiles = tFileAccessSettings;
}

static bool CheckVectorLength ( int iLen, int64_t iSaneLen, const char * sAt, CSphString & sError )
{
	if ( iLen>=0 && iLen<iSaneLen )
		return true;

	sError.SetSprintf ( "broken index, %s length overflow (len=%d, max=" INT64_FMT ")", sAt, iLen, iSaneLen );
	return false;
}

template < typename T >
static void SaveVector ( CSphWriter & tWriter, const VecTraits_T < T > & tVector )
{
	STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE(T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.PutDword ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLengthBytes() );
}


template < typename T, typename P >
static bool LoadVector ( CSphReader & tReader, CSphVector < T, P > & tVector,
	int64_t iSaneLen, const char * sAt, CSphString & sError )
{
	STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE(T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
	int iSize = tReader.GetDword();
	if ( !CheckVectorLength ( iSize, iSaneLen, sAt, sError ) )
		return false;

	tVector.Resize ( iSize );
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), (int) tVector.GetLengthBytes() );

	return true;
}


template < typename T, typename P >
static void SaveVector ( BinlogWriter_c &tWriter, const CSphVector<T, P> &tVector )
{
	STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE (T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.ZipOffset ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLengthBytes() );
}


template < typename T, typename P >
static bool LoadVector ( BinlogReader_c & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE(T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( (int) tReader.UnzipOffset() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), (int) tVector.GetLengthBytes() );
	return !tReader.GetErrorFlag();
}


bool RtIndex_c::SaveRamChunk ( const VecTraits_T<const RtSegmentRefPtf_t> &dSegments )
{
	if ( m_bSaveDisabled )
		return true;

	MEMORY ( MEM_INDEX_RT );

	CSphString sChunk, sNewChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	sNewChunk.SetSprintf ( "%s.ram.new", m_sPath.cstr() );

	CSphWriter wrChunk;
	if ( !wrChunk.OpenFile ( sNewChunk, m_sLastError ) )
		return false;

	wrChunk.PutDword ( 0 );
	wrChunk.PutDword ( dSegments.GetLength() );

	// no locks here, because it's only intended to be called from dtor
	for ( const RtSegment_t * pSeg : dSegments )
	{
		wrChunk.PutDword ( pSeg->m_uRows );
		wrChunk.PutDword ( pSeg->m_tAliveRows.load ( std::memory_order_relaxed ) );
		wrChunk.PutDword ( 0 );
		SaveVector ( wrChunk, pSeg->m_dWords );
		if ( m_bKeywordDict )
			SaveVector ( wrChunk, pSeg->m_dKeywordCheckpoints );

		auto pCheckpoints = (const char *)pSeg->m_dKeywordCheckpoints.Begin();
		wrChunk.PutDword ( pSeg->m_dWordCheckpoints.GetLength() );
		for ( const auto& dWordCheckpoint : pSeg->m_dWordCheckpoints )
		{
			wrChunk.PutOffset ( dWordCheckpoint.m_iOffset );
			if ( m_bKeywordDict )
				wrChunk.PutOffset ( dWordCheckpoint.m_sWord-pCheckpoints );
			else
				wrChunk.PutOffset ( dWordCheckpoint.m_uWordID );
		}

		SaveVector ( wrChunk, pSeg->m_dDocs );
		SaveVector ( wrChunk, pSeg->m_dHits );
		SaveVector ( wrChunk, pSeg->m_dRows );
		pSeg->m_tDeadRowMap.Save ( wrChunk );
		SaveVector ( wrChunk, pSeg->m_dBlobs );

		if ( pSeg->m_pDocstore.Ptr() )
			pSeg->m_pDocstore->Save(wrChunk);

		// infixes
		SaveVector ( wrChunk, pSeg->m_dInfixFilterCP );
	}

	// field lengths
	wrChunk.PutDword ( m_tSchema.GetFieldsCount() );
	for ( int i=0; i < m_tSchema.GetFieldsCount(); ++i )
		wrChunk.PutOffset ( m_dFieldLensRam[i] );

	wrChunk.CloseFile();
	if ( wrChunk.IsError() )
		return false;

	// rename
	if ( sph::rename ( sNewChunk.cstr(), sChunk.cstr() ) )
		sphDie ( "failed to rename ram chunk (src=%s, dst=%s, errno=%d, error=%s)",
			sNewChunk.cstr(), sChunk.cstr(), errno, strerrorm(errno) ); // !COMMIT handle this gracefully

	return true;
}


bool RtIndex_c::LoadRamChunk ( DWORD uVersion, bool bRebuildInfixes )
{
	MEMORY ( MEM_INDEX_RT );

	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );

	if ( !sphIsReadable ( sChunk.cstr(), &m_sLastError ) )
		return true;

	m_bLoadRamPassedOk = false;
	m_bHasFiles = true;

	CSphAutoreader rdChunk;
	if ( !rdChunk.Open ( sChunk, m_sLastError ) )
		return false;

	int64_t iFileSize = rdChunk.GetFilesize();
	int64_t iSaneVecSize = Min ( iFileSize, INT_MAX / 2 );
	int64_t iSaneTightVecSize = Min ( iFileSize, int ( INT_MAX / 1.2f ) );

	bool bHasMorphology = ( m_pDict && m_pDict->HasMorphology() ); // fresh and old-format index still has no dictionary at this point
	rdChunk.GetDword ();

	int iSegmentCount = rdChunk.GetDword();
	if ( !CheckVectorLength ( iSegmentCount, iSaneVecSize, "ram-chunks", m_sLastError ) )
		return false;

	m_dRamChunks.Resize ( iSegmentCount );

	for ( auto & dRamChunk : m_dRamChunks )
	{

		DWORD uRows = rdChunk.GetDword();

		RtSegmentRefPtf_t pSeg {new RtSegment_t ( uRows )};
		pSeg->m_uRows = uRows;
		pSeg->m_tAliveRows.store ( rdChunk.GetDword (), std::memory_order_relaxed );

		rdChunk.GetDword ();
		if ( !LoadVector ( rdChunk, pSeg->m_dWords, iSaneTightVecSize, "ram-words", m_sLastError ) )
			return false;

		if ( m_bKeywordDict && !LoadVector ( rdChunk, pSeg->m_dKeywordCheckpoints, iSaneVecSize, "ram-checkpoints", m_sLastError ) )
			return false;

		auto * pCheckpoints = (const char *)pSeg->m_dKeywordCheckpoints.Begin();

		int iCheckpointCount = rdChunk.GetDword();
		if ( !CheckVectorLength ( iCheckpointCount, iSaneVecSize, "ram-checkpoints", m_sLastError ) )
			return false;

		pSeg->m_dWordCheckpoints.Resize ( iCheckpointCount );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int)rdChunk.GetOffset();
			SphOffset_t uOff = rdChunk.GetOffset();
			if ( m_bKeywordDict )
				pSeg->m_dWordCheckpoints[i].m_sWord = pCheckpoints + uOff;
			else
				pSeg->m_dWordCheckpoints[i].m_uWordID = (SphWordID_t)uOff;
		}

		if ( !LoadVector ( rdChunk, pSeg->m_dDocs, iSaneTightVecSize, "ram-doclist", m_sLastError ) )
			return false;

		if ( !LoadVector ( rdChunk, pSeg->m_dHits, iSaneTightVecSize, "ram-hitlist", m_sLastError ) )
			return false;

		if ( !LoadVector ( rdChunk, pSeg->m_dRows, iSaneTightVecSize, "ram-attributes", m_sLastError ) )
			return false;

		pSeg->m_tDeadRowMap.Load ( uRows, rdChunk, m_sLastError );

		if ( !LoadVector ( rdChunk, pSeg->m_dBlobs, iSaneTightVecSize, "ram-blobs", m_sLastError ) )
			return false;

		if ( uVersion>=15 && m_tSchema.HasStoredFields() )
		{
			pSeg->m_pDocstore = CreateDocstoreRT();
			SetupDocstoreFields ( *pSeg->m_pDocstore.Ptr(), m_tSchema );
			assert ( pSeg->m_pDocstore.Ptr() );
			if ( !pSeg->m_pDocstore->Load ( rdChunk ) )
				return false;
		}

		// infixes
		if ( !LoadVector ( rdChunk, pSeg->m_dInfixFilterCP, iSaneTightVecSize, "ram-infixes", m_sLastError ) )
			return false;

		if ( bRebuildInfixes )
				BuildSegmentInfixes ( pSeg, bHasMorphology, m_bKeywordDict, m_tSettings.m_iMinInfixLen, m_iWordsCheckpoint, ( m_iMaxCodepointLength>1 ), m_tSettings.m_eHitless );

		pSeg->BuildDocID2RowIDMap();
		dRamChunk = AdoptSegment ( pSeg );
	}

	// field lengths
	int iFields = rdChunk.GetDword();
	assert ( iFields==m_tSchema.GetFieldsCount() );

	for ( int i=0; i<iFields; i++ )
		m_dFieldLensRam[i] = rdChunk.GetOffset();

	// all done
	if ( rdChunk.GetErrorFlag() )
		return false;

	m_bLoadRamPassedOk = true;
	return true;
}


void RtIndex_c::PostSetup()
{
	RtIndex_i::PostSetup();

	m_pDocstoreFields.Reset();
	if ( m_tSchema.HasStoredFields() )
	{
		m_pDocstoreFields = CreateDocstoreFields();
		SetupDocstoreFields ( *m_pDocstoreFields.Ptr(), m_tSchema );
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
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	TokenizerRefPtr_c pIndexing { ISphTokenizer::CreateBigramFilter ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError ) };
	if ( pIndexing )
		m_pTokenizerIndexing = pIndexing;

	const CSphDictSettings & tDictSettings = m_pDict->GetSettings();
	if ( !ParseMorphFields ( tDictSettings.m_sMorphology, tDictSettings.m_sMorphFields, m_tSchema.GetFields(), m_tMorphFields, m_sLastError ) )
		sphWarning ( "index '%s': %s", m_sIndexName.cstr(), m_sLastError.cstr() );

	// hitless
	CSphString sHitlessFiles = m_tSettings.m_sHitlessFiles;
	if ( GetIndexFilenameBuilder() )
	{
		CSphScopedPtr<const FilenameBuilder_i> pFilenameBuilder ( GetIndexFilenameBuilder() ( m_sIndexName.cstr() ) );
		if ( pFilenameBuilder.Ptr() )
			sHitlessFiles = pFilenameBuilder->GetFullPath ( sHitlessFiles );
	}

	if ( !LoadHitlessWords ( sHitlessFiles, m_pTokenizerIndexing, m_pDict, m_dHitlessWords, m_sLastError ) )
		sphWarning ( "index '%s': %s", m_sIndexName.cstr(), m_sLastError.cstr() );

	// still need index files for index just created from config
	if ( !m_bHasFiles )
	{
		SaveRamChunk ( m_dRamChunks );
		SaveMeta ( m_iTID );
	}
}

struct MemoryDebugCheckReader_c : public DebugCheckReader_i
{
	MemoryDebugCheckReader_c ( const BYTE * pData, const BYTE * pDataEnd )
		: m_pData ( pData )
		, m_pDataEnd ( pDataEnd )
		, m_pCur ( pData )
	{}
	virtual ~MemoryDebugCheckReader_c () override
	{}
	virtual int64_t GetLengthBytes () override
	{
		return ( m_pDataEnd - m_pData );
	}
	virtual bool GetBytes ( void * pData, int iSize ) override
	{
		if ( m_pCur && m_pCur+iSize<=m_pDataEnd )
		{
			memcpy ( pData, m_pCur, iSize );
			m_pCur += iSize;
			return true;
		} else
		{
			return false;
		}
	}

	bool SeekTo ( int64_t iOff, int iHint ) final
	{
		if ( m_pData && m_pData+iOff<m_pDataEnd )
		{
			m_pCur = m_pData + iOff;
			return true;
		} else
		{
			return false;
		}
	}

	const BYTE * m_pData = nullptr;
	const BYTE * m_pDataEnd = nullptr;
	const BYTE * m_pCur = nullptr;
};

int RtIndex_c::DebugCheck ( FILE * fp )
{
	// FIXME! remove copypasted code from CSphIndex_VLN::DebugCheck

	DebugCheckError_c tReporter(fp);

	if ( m_iLockFD<0 && m_iCheckChunk!=-1 )
		sphWarning ( "failed to load RAM chunks, checking only %d disk chunks", m_dChunkNames.GetLength() );

	if ( m_iStride!=m_tSchema.GetRowSize() )
		tReporter.Fail ( "wrong attribute stride (current=%d, should_be=%d)", m_iStride, m_tSchema.GetRowSize() );

	if ( m_iSoftRamLimit<=0 )
		tReporter.Fail ( "wrong RAM limit (current=" INT64_FMT ")", m_iSoftRamLimit );

	if ( m_iTID<0 )
		tReporter.Fail ( "index TID < 0 (current=" INT64_FMT ")", m_iTID );

	if ( m_iSavedTID<0 )
		tReporter.Fail ( "index saved TID < 0 (current=" INT64_FMT ")", m_iSavedTID );

	if ( m_iTID<m_iSavedTID )
		tReporter.Fail ( "index TID < index saved TID (current=" INT64_FMT ", saved=" INT64_FMT ")", m_iTID, m_iSavedTID );

	if ( m_iWordsCheckpoint!=RTDICT_CHECKPOINT_V5 )
		tReporter.Fail ( "unexpected number of words per checkpoint (expected 48, got %d)", m_iWordsCheckpoint );

	tReporter.Msg ( "checking schema..." );
	DebugCheckSchema ( m_tSchema, tReporter );

	if ( m_iCheckChunk!=-1 )
		DebugCheckRam ( tReporter );

	int iFailsPlain = DebugCheckDisk ( tReporter, fp );

	tReporter.Done();

	return tReporter.GetNumFails() + iFailsPlain;
}

void RtIndex_c::DebugCheckRam ( DebugCheckError_c & tReporter )
{
	ARRAY_FOREACH ( iSegment, m_dRamChunks )
	{
		tReporter.Msg ( "checking RT segment %d(%d)...", iSegment, m_dRamChunks.GetLength() );

		if ( !m_dRamChunks[iSegment] )
		{
			tReporter.Fail ( "missing RT segment (segment=%d)", iSegment );
			continue;
		}

		RtSegment_t & tSegment = *m_dRamChunks[iSegment];
		if ( !tSegment.m_uRows )
		{
			tReporter.Fail ( "empty RT segment (segment=%d)", iSegment );
			continue;
		}

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
					tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );
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
						tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );
						break;
					}

					if ( iDelta<=8 && iMatch<=15 )
					{
						sLastWord[sizeof(sLastWord)-1] = '\0';
						tReporter.Fail ( "wrong word-delta (segment=%d, word=%d, last_word=%s, last_len=%d, match=%d, delta=%d)",
							iSegment, nWordsRead, sLastWord+1, iLastWordLen, iMatch, iDelta );
					}
				}

				if ( iMatch+iDelta>=(int)sizeof(sWord)-2 || iMatch>iLastWordLen )
				{
					sLastWord[sizeof(sLastWord)-1] = '\0';
					tReporter.Fail ( "wrong word-delta (segment=%d, word=%d, last_word=%s, last_len=%d, match=%d, delta=%d)",
						iSegment, nWordsRead, sLastWord+1, iLastWordLen, iMatch, iDelta );

					pIn += iDelta;
					if ( pIn>=pMaxWord )
					{
						tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );
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
						tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );
						break;
					}
				}

				auto iCalcWordLen = (int) strlen ( (const char *)sWord+1 );
				if ( iWordLen!=iCalcWordLen )
				{
					sWord[sizeof(sWord)-1] = '\0';
					tReporter.Fail ( "word length mismatch (segment=%d, word=%d, read_word=%s, read_len=%d, calc_len=%d)", iSegment, nWordsRead, sWord+1, iWordLen, iCalcWordLen );
				}

				if ( !iWordLen )
					tReporter.Fail ( "empty word in word list (segment=%d, word=%d)", iSegment, nWordsRead );

				const BYTE * pStr = sWord+1;
				const BYTE * pStringStart = pStr;
				while ( pStringStart-pStr < iWordLen )
				{
					if ( !*pStringStart )
					{
						CSphString sErrorStr;
						sErrorStr.SetBinary ( (const char*)pStr, iWordLen );
						tReporter.Fail ( "embedded zero in a word list string (segment=%d, offset=%u, string=%s)", iSegment, (DWORD)(pStringStart-pStr), sErrorStr.cstr() );
					}

					pStringStart++;
				}

				if ( iLastWordLen && iWordLen )
				{
					if ( sphDictCmpStrictly ( (const char *)sWord+1, iWordLen, (const char *)sLastWord+1, iLastWordLen )<=0 )
					{
						sWord[sizeof(sWord)-1] = '\0';
						sLastWord[sizeof(sLastWord)-1] = '\0';
						tReporter.Fail ( "word order decreased (segment=%d, word=%d, read_word=%s, last_word=%s)", iSegment, nWordsRead, sWord+1, sLastWord+1 );
					}
				}

				memcpy ( sLastWord, sWord, iWordLen+2 );
				iLastWordLen = iWordLen;
			} else
			{
				SphWordID_t uDeltaID;
				pIn = UnzipWordid ( &uDeltaID, pIn );
				if ( pIn>=pMaxWord )
					tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );

				tWord.m_uWordID += uDeltaID;

				if ( tWord.m_uWordID<=uPrevWordID )
				{
					tReporter.Fail ( "wordid decreased (segment=%d, word=%d, wordid=" UINT64_FMT ", previd=" UINT64_FMT ")", iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, (uint64_t)uPrevWordID );
				}

				uPrevWordID = tWord.m_uWordID;
			}

			pIn = UnzipDword ( &tWord.m_uDocs, pIn );
			if ( pIn>=pMaxWord )
			{
				sWord[sizeof(sWord)-1] = '\0';
				tReporter.Fail ( "invalid docs/hits (segment=%d, word=%d, read_word=%s, docs=%u, hits=%u)", iSegment, nWordsRead, sWord+1, tWord.m_uDocs, tWord.m_uHits );
			}

			pIn = UnzipDword ( &tWord.m_uHits, pIn );
			if ( pIn>=pMaxWord )
				tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );

			pIn = UnzipDword ( &uDeltaDoc, pIn );
			if ( pIn>pMaxWord )
				tReporter.Fail ( "reading past wordlist end (segment=%d, word=%d)", iSegment, nWordsRead );

			pCurWord = pIn;
			tWord.m_uDoc += uDeltaDoc;

			if ( !tWord.m_uDocs || !tWord.m_uHits || tWord.m_uHits<tWord.m_uDocs )
			{
				sWord[sizeof(sWord)-1] = '\0';
				tReporter.Fail ( "invalid docs/hits (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, docs=%u, hits=%u)",
					iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, tWord.m_uDocs, tWord.m_uHits );
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
				tReporter.Fail ( "doclist offset decreased (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, prev_doclist_offset=%u)",
					iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, tWord.m_uDoc, uPrevDocOffset );

			// read doclist
			DWORD uDocOffset = pCurDoc-tSegment.m_dDocs.Begin();
			if ( tWord.m_uDoc!=uDocOffset )
			{
				tReporter.Fail ( "unexpected doclist offset (wordid=" UINT64_FMT "(%s)(%d), doclist_offset=%u, expected_offset=%u)",
					(uint64_t)tWord.m_uWordID, sWord+1, nWordsRead, tWord.m_uDoc, uDocOffset );

				if ( uDocOffset>=(DWORD)tSegment.m_dDocs.GetLength() )
				{
					tReporter.Fail ( "doclist offset pointing past doclist (segment=%d, word=%d, read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );

					nWordsRead++;
					continue;
				} else
					pCurDoc = tSegment.m_dDocs.Begin()+uDocOffset;
			}

			// read all docs from doclist
			RtDoc_t tDoc;
			RowID_t tPrevRowID = INVALID_ROWID;

			for ( DWORD uDoc=0; uDoc<tWord.m_uDocs && pCurDoc<pMaxDoc; uDoc++ )
			{
				bool bEmbeddedHit = false;
				pIn = pCurDoc;
				RowID_t tDeltaRowID;
				pIn = UnzipDword ( &tDeltaRowID, pIn );

				if ( pIn>=pMaxDoc )
				{
					tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
					break;
				}

				tDoc.m_tRowID += tDeltaRowID;
				DWORD uDocField;
				pIn = UnzipDword ( &uDocField, pIn );

				if ( pIn>=pMaxDoc )
				{
					tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
					break;
				}

				tDoc.m_uDocFields = uDocField;
				pIn = UnzipDword ( &tDoc.m_uHits, pIn );
				if ( pIn>=pMaxDoc )
				{
					tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
					break;
				}

				if ( tDoc.m_uHits==1 )
				{
					bEmbeddedHit = true;

					DWORD a, b;
					pIn = UnzipDword ( &a, pIn );
					if ( pIn>=pMaxDoc )
					{
						tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
						break;
					}

					pIn = UnzipDword ( &b, pIn );
					if ( pIn>pMaxDoc )
					{
						tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
						break;
					}

					tDoc.m_uHit = HITMAN::Create ( b, a );
				} else
				{
					pIn = UnzipDword ( &tDoc.m_uHit, pIn );
					if ( pIn>pMaxDoc )
					{
						tReporter.Fail ( "reading past doclist end (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, doclist_offset=%u, doclist_size=%d)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, uDocOffset, tSegment.m_dDocs.GetLength() );
						break;
					}
				}

				pCurDoc = pIn;

				if ( uDoc && tDoc.m_tRowID<=tPrevRowID )
				{
					tReporter.Fail ( "rowid decreased (segment=%d, word=%d, read_wordid=" UINT64_FMT ", read_word=%s, rowid=%u, prev_rowid=%u)",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, sWord+1, tDoc.m_tRowID, tPrevRowID );
				}

				if ( tDoc.m_tRowID>=tSegment.m_uRows )
					tReporter.Fail ( "invalid rowid (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u)", iSegment, nWordsRead, tWord.m_uWordID, tDoc.m_tRowID );

				if ( bEmbeddedHit )
				{
					DWORD uFieldId = HITMAN::GetField ( tDoc.m_uHit );
					DWORD uFieldMask = tDoc.m_uDocFields;
					int iCounter = 0;
					for ( ; uFieldMask; iCounter++ )
						uFieldMask &= uFieldMask - 1;

					if ( iCounter!=1 || tDoc.m_uHits!=1 )
					{
						tReporter.Fail ( "embedded hit with multiple occurences in a document found (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID );
					}

					if ( (int)uFieldId>m_tSchema.GetFieldsCount() || uFieldId>SPH_MAX_FIELDS )
					{
						tReporter.Fail ( "invalid field id in an embedded hit (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, field_id=%u, total_fields=%d)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, uFieldId, m_tSchema.GetFieldsCount() );
					}

					if ( !( tDoc.m_uDocFields & ( 1 << uFieldId ) ) )
					{
						tReporter.Fail ( "invalid field id: not in doclist mask (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, field_id=%u, field_mask=%u)",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, uFieldId, tDoc.m_uDocFields );
					}
				} else
				{
					DWORD uExpectedHitOffset = pCurHit-tSegment.m_dHits.Begin();
					if ( tDoc.m_uHit!=uExpectedHitOffset )
					{
						tReporter.Fail ( "unexpected hitlist offset (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, offset=%u, expected_offset=%u",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, tDoc.m_uHit, uExpectedHitOffset );
					}

					if ( tDoc.m_uHit && tDoc.m_uHit<=uPrevHitOffset )
					{
						tReporter.Fail ( "hitlist offset decreased (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, offset=%u, prev_offset=%u",
							iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, tDoc.m_uHit, uPrevHitOffset );
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
							tReporter.Fail ( "reading past hitlist end (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u)", iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID );
							break;
						}

						uHitlistEntry += uValue;

						DWORD uPosInField = HITMAN::GetPos ( uHitlistEntry );
						bool bLastInField = HITMAN::IsEnd ( uHitlistEntry );
						DWORD uFieldId = HITMAN::GetField ( uHitlistEntry );

						if ( (int)uFieldId>m_tSchema.GetFieldsCount() || uFieldId>SPH_MAX_FIELDS )
						{
							tReporter.Fail ( "invalid field id in a hitlist (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, field_id=%u, total_fields=%d)",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, uFieldId, m_tSchema.GetFieldsCount() );
						}

						if ( !( tDoc.m_uDocFields & ( 1 << uFieldId ) ) )
						{
							tReporter.Fail ( "invalid field id: not in doclist mask (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, field_id=%u, field_mask=%u)",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, uFieldId, tDoc.m_uDocFields );
						}

						if ( uLastFieldId!=uFieldId )
						{
							bLastInFieldFound = false;
							uLastPosInField = 0;
						}

						if ( uLastPosInField && uPosInField<=uLastPosInField )
						{
							tReporter.Fail ( "hit position in field decreased (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, pos=%u, last_pos=%u)",
								iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, uPosInField, uLastPosInField );
						}

						if ( bLastInField && bLastInFieldFound )
							tReporter.Fail ( "duplicate last-in-field hit found (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u)", iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID );

						uLastPosInField = uPosInField;
						uLastFieldId = uFieldId;
						bLastInFieldFound |= bLastInField;
					}

					uPrevHitOffset = tDoc.m_uHit;
				}

				DWORD uAvailFieldMask = ( 1 << m_tSchema.GetFieldsCount() ) - 1;
				if ( tDoc.m_uDocFields & ~uAvailFieldMask )
				{
					tReporter.Fail ( "wrong document field mask (segment=%d, word=%d, wordid=" UINT64_FMT ", rowid=%u, mask=%u, total_fields=%d",
						iSegment, nWordsRead, (uint64_t)tWord.m_uWordID, tDoc.m_tRowID, tDoc.m_uDocFields, m_tSchema.GetFieldsCount() );
				}

				tPrevRowID = tDoc.m_tRowID;
			}

			uPrevDocOffset = tWord.m_uDoc;
			nWordsRead++;
		}

		if ( pCurDoc!=pMaxDoc )
			tReporter.Fail ( "unused doclist entries found (segment=%d, doclist_size=%d)", iSegment, tSegment.m_dDocs.GetLength() );

		if ( pCurHit!=pMaxHit )
			tReporter.Fail ( "unused hitlist entries found (segment=%d, hitlist_size=%d)", iSegment, tSegment.m_dHits.GetLength() );

		if ( dRefCheckpoints.GetLength()!=tSegment.m_dWordCheckpoints.GetLength() )
			tReporter.Fail ( "word checkpoint count mismatch (read=%d, calc=%d)", tSegment.m_dWordCheckpoints.GetLength(), dRefCheckpoints.GetLength() );

		for ( int i=0; i < Min ( dRefCheckpoints.GetLength(), tSegment.m_dWordCheckpoints.GetLength() ); i++ )
		{
			const RtWordCheckpoint_t & tRefCP = dRefCheckpoints[i];
			const RtWordCheckpoint_t & tCP = tSegment.m_dWordCheckpoints[i];
			const int iLen = m_bKeywordDict ? (const int) strlen ( tCP.m_sWord ) : 0;
			if ( m_bKeywordDict && ( !tCP.m_sWord || ( !strlen ( tRefCP.m_sWord ) || !strlen ( tCP.m_sWord ) ) ) )
			{
				tReporter.Fail ( "empty word checkpoint %d ((segment=%d, read_word=%s, read_len=%u, readpos=%d, calc_word=%s, calc_len=%u, calcpos=%d)",
					i, iSegment, tCP.m_sWord, (DWORD)strlen ( tCP.m_sWord ), tCP.m_iOffset,
					tRefCP.m_sWord, (DWORD)strlen ( tRefCP.m_sWord ), tRefCP.m_iOffset );
			} else if ( sphCheckpointCmpStrictly ( tCP.m_sWord, iLen, tCP.m_uWordID, m_bKeywordDict, tRefCP ) || tRefCP.m_iOffset!=tCP.m_iOffset )
			{
				if ( m_bKeywordDict )
				{
					tReporter.Fail ( "word checkpoint %d differs (segment=%d, read_word=%s, readpos=%d, calc_word=%s, calcpos=%d)",
						i, iSegment, tCP.m_sWord, tCP.m_iOffset, tRefCP.m_sWord, tRefCP.m_iOffset );
				} else
				{
					tReporter.Fail ( "word checkpoint %d differs (segment=%d, readid=" UINT64_FMT ", readpos=%d, calcid=" UINT64_FMT ", calcpos=%d)",
						i, iSegment, (uint64_t)tCP.m_uWordID, tCP.m_iOffset, (int64_t)tRefCP.m_uWordID, tRefCP.m_iOffset );
				}
			}
		}

		if ( m_bKeywordDict )
			ARRAY_FOREACH ( i, dRefCheckpoints )
				SafeDeleteArray ( dRefCheckpoints[i].m_sWord );

		dRefCheckpoints.Reset ();

		MemoryDebugCheckReader_c tAttrs ( (const BYTE *)tSegment.m_dRows.begin(), (const BYTE *)tSegment.m_dRows.end() );
		MemoryDebugCheckReader_c tBlobs ( tSegment.m_dBlobs.begin(), tSegment.m_dBlobs.end() );
		DebugCheck_Attributes ( tAttrs, tBlobs, tSegment.m_uRows, 0, m_tSchema, tReporter );
		DebugCheck_DeadRowMap ( tSegment.m_tDeadRowMap.GetLengthBytes(), tSegment.m_uRows, tReporter );

		DWORD uCalcAliveRows = tSegment.m_tDeadRowMap.GetNumAlive();
		if ( tSegment.m_tAliveRows.load(std::memory_order_relaxed)!=uCalcAliveRows )
			tReporter.Fail ( "alive row count mismatch (segment=%d, expected=%u, current=%u)", iSegment, uCalcAliveRows,
					tSegment.m_tAliveRows.load ( std::memory_order_relaxed ) );
	}

} // NOLINT function length

int RtIndex_c::DebugCheckDisk ( DebugCheckError_c & tReporter, FILE * fp )
{
	CreateFilenameBuilder_fn fnCreateFilenameBuilder = GetIndexFilenameBuilder();
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( fnCreateFilenameBuilder ? fnCreateFilenameBuilder ( m_sIndexName.cstr() ) : nullptr );

	VecTraits_T<int> dChunks = m_dChunkNames.Slice();
	if ( m_iCheckChunk!=-1 )
	{
		int iChunk = dChunks.GetFirst (  [&] ( int & v ) { return m_iCheckChunk==v; }  );
		if ( iChunk==-1 )
		{
			tReporter.Fail ( "failed to find disk chunk %s.%d, disk chunks total %d", m_sPath.cstr(), m_iCheckChunk, m_dChunkNames.GetLength() );
			return 1;
		}

		dChunks = m_dChunkNames.Slice ( iChunk, 1 );
	}

	int iFailsPlain = 0;
	ARRAY_FOREACH ( i, dChunks )
	{
		int iChunk = dChunks[i];
		CSphString sChunk;
		sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunk );
		tReporter.Msg ( "checking disk chunk, extension %d, %d(%d)...", dChunks[i], i, m_dChunkNames.GetLength() );

		CSphScopedPtr<CSphIndex> pIndex ( PreallocDiskChunk ( sChunk.cstr(), iChunk, pFilenameBuilder.Ptr(), m_sLastError ) );
		if ( pIndex.Ptr() )
		{
			iFailsPlain += pIndex->DebugCheck ( fp );
		} else
		{
			tReporter.Fail ( "%s", m_sLastError.cstr() );
			m_sLastError = "";
			iFailsPlain++;
		}
	}

	return iFailsPlain;
}

void RtIndex_c::SetDebugCheck ( bool bCheckIdDups, int iCheckChunk )
{
	m_bDebugCheck = true;
	m_bCheckIdDups = bCheckIdDups;
	m_iCheckChunk = iCheckChunk;
	ProhibitSave();
}

//////////////////////////////////////////////////////////////////////////
// SEARCHING
//////////////////////////////////////////////////////////////////////////


struct RtQword_t final : public ISphQword
{
public:
	RtQword_t ()
	{
		m_tMatch.Reset ( 0 );
	}

	virtual ~RtQword_t ()
	{
	}

	const CSphMatch & GetNextDoc() final
	{
		while (true)
		{
			const RtDoc_t * pDoc = m_tDocReader.UnzipDoc();
			if ( !pDoc )
			{
				m_tMatch.m_tRowID = INVALID_ROWID;
				return m_tMatch;
			}

			if ( m_pSeg->m_tDeadRowMap.IsSet ( pDoc->m_tRowID ) )
				continue;

			m_tMatch.m_tRowID = pDoc->m_tRowID;
			m_dQwordFields.Assign32 ( pDoc->m_uDocFields );
			m_uMatchHits = pDoc->m_uHits;
			m_iHitlistPos = (uint64_t(pDoc->m_uHits)<<32) + pDoc->m_uHit;
			m_bAllFieldsKnown = false;
			return m_tMatch;
		}
	}

	void SeekHitlist ( SphOffset_t uOff ) final
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

	Hitpos_t GetNextHit () final
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

	bool SetupScan ( const RtIndex_c * pIndex, int iSegment, const SphChunkGuard_t & tGuard ) final
	{
		return pIndex->RtQwordSetup ( this, iSegment, tGuard );
	}

	void SetupReader ( const RtSegment_t * pSeg, const RtWord_t & tWord )
	{
		m_pSeg = pSeg;
		m_tDocReader = RtDocReader_t ( pSeg, tWord );
		m_tHitReader.m_pBase = pSeg->m_dHits.Begin();
	}

private:
	RtDocReader_t		m_tDocReader;
	CSphMatch			m_tMatch;

	DWORD				m_uNextHit {0};
	RtHitReader2_t		m_tHitReader;

	const RtSegment_t * m_pSeg {nullptr};
};


//////////////////////////////////////////////////////////////////////////


struct RtSubstringPayload_t : public ISphSubstringPayload
{
	RtSubstringPayload_t ( int iSegmentCount, int iDoclists )
		: m_dSegment2Doclists ( iSegmentCount )
		, m_dDoclist ( iDoclists )
	{}
	CSphFixedVector<Slice_t>	m_dSegment2Doclists;
	CSphFixedVector<Slice_t>	m_dDoclist;
};


struct RtQwordPayload_t final : public ISphQword
{
public:
	explicit RtQwordPayload_t ( const RtSubstringPayload_t * pPayload )
		: m_pPayload ( pPayload )
	{
		m_tMatch.Reset ( 0 );
		m_iDocs = m_pPayload->m_iTotalDocs;
		m_iHits = m_pPayload->m_iTotalHits;

		m_uDoclist = 0;
		m_uDoclistLeft = 0;
		m_pSegment = NULL;
		m_uHitEmbeded = EMPTY_HIT;
	}

	~RtQwordPayload_t () final
	{}

	const CSphMatch & GetNextDoc() final
	{
		m_iHits = 0;
		while (true)
		{
			const RtDoc_t * pDoc = m_tDocReader.UnzipDoc();
			if ( !pDoc && !m_uDoclistLeft )
			{
				m_tMatch.m_tRowID = INVALID_ROWID;
				return m_tMatch;
			}

			if ( !pDoc && m_uDoclistLeft )
			{
				SetupReader();
				pDoc = m_tDocReader.UnzipDoc();
				assert ( pDoc );
			}

			if ( m_pSegment->m_tDeadRowMap.IsSet ( pDoc->m_tRowID ) )
				continue;

			m_tMatch.m_tRowID = pDoc->m_tRowID;
			m_dQwordFields.Assign32 ( pDoc->m_uDocFields );
			m_bAllFieldsKnown = false;

			m_iHits = pDoc->m_uHits;
			m_uHitEmbeded = pDoc->m_uHit;
			m_tHitReader = RtHitReader_t ( m_pSegment, pDoc );

			return m_tMatch;
		}
	}

	void SeekHitlist ( SphOffset_t ) final
	{}

	Hitpos_t GetNextHit () final
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

	bool SetupScan ( const RtIndex_c *, int iSegment, const SphChunkGuard_t & tGuard ) final
	{
		m_uDoclist = 0;
		m_uDoclistLeft = 0;
		m_tDocReader = RtDocReader_t();
		m_pSegment = NULL;

		if ( iSegment<0 )
			return false;

		m_pSegment = tGuard.m_dRamChunks[iSegment];
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

	DWORD						m_uDoclist;
	DWORD						m_uDoclistLeft;
	DWORD						m_uHitEmbeded;
};

struct RtScanQword_t final : public QwordScan_c
{
public:
	explicit RtScanQword_t ( int iRowsTotal )
		: QwordScan_c ( iRowsTotal )
	{
	}

	bool SetupScan ( const RtIndex_c * pIndex, int iSegment, const SphChunkGuard_t & tGuard ) final
	{
		m_tDoc.Reset ( 0 );

		if ( iSegment<0 )
			return false;

		RtSegment_t * pSegment = tGuard.m_dRamChunks[iSegment];

		m_iRowsCount = pSegment->m_uRows;
		m_iDocs = m_iRowsCount;
		m_bDone = ( m_iRowsCount==0 );
		m_dQwordFields.SetAll();

		return ( pSegment->m_tAliveRows.load(std::memory_order_relaxed)>0 );
	}
};


//////////////////////////////////////////////////////////////////////////

class RtQwordSetup_t final : public ISphQwordSetup
{
public:
	explicit RtQwordSetup_t ( const SphChunkGuard_t & tGuard );
	ISphQword *	QwordSpawn ( const XQKeyword_t & ) const final;
	bool		QwordSetup ( ISphQword * pQword ) const final;
	void				SetSegment ( int iSegment ) { m_iSeg = iSegment; }
	ISphQword *			ScanSpawn() const final;

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
	const RtIndex_c * pIndex = (const RtIndex_c *)m_pIndex;
	return pQword->SetupScan ( pIndex, m_iSeg, m_tGuard );
}


bool RtIndex_c::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	auto pSegment = (RtSegment_t*)pCtx->m_pIndexData;
	tMatch.m_pStatic = pSegment->GetDocinfoByRowID ( tMatch.m_tRowID );

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

ISphQword * RtQwordSetup_t::ScanSpawn () const
{
	return new RtScanQword_t ( m_pIndex->GetStats().m_iTotalDocuments );
}


// WARNING, setup is pretty tricky
// for RT queries, we setup qwords several times
// first pass (with NULL segment arg) should sum all stats over all segments
// others passes (with non-NULL segments) should setup specific segment (including local stats)
bool RtIndex_c::RtQwordSetupSegment ( RtQword_t * pQword, const RtSegment_t * pCurSeg, bool bSetup, bool bWordDict, int iWordsCheckpoint, const CSphIndexSettings & tSettings )
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
	if ( bPrefix && ( ( tSettings.GetMinPrefixLen ( bWordDict ) && iWordLen<tSettings.GetMinPrefixLen ( bWordDict ) )
		|| ( tSettings.m_iMinInfixLen && iWordLen<tSettings.m_iMinInfixLen ) ) )
		return false;

	// no checkpoints - check all words
	// no checkpoints matched - check only words prior to 1st checkpoint
	// checkpoint found - check words at that checkpoint
	RtWordReader_t tReader ( pCurSeg, bWordDict, iWordsCheckpoint, tSettings.m_eHitless );

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
			pQword->m_bHasHitlist &= pWord->m_bHasHitlist;
			if ( bSetup )
				pQword->SetupReader ( pCurSeg, *pWord );

			return true;

		} else if ( iCmp>0 )
		{
			return false;
		}
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
		m_iSegExpansionLimit = iSegments;
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
			m_dWordBuf.Append ( pWord->m_sWord, iWordLen );
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
		if ( !m_dWordExpand.IsEmpty() )
		{
			int iRtExpansionLimit = tArgs.m_iExpansionLimit * m_iSegExpansionLimit;
			if ( tArgs.m_iExpansionLimit && m_dWordExpand.GetLength()>iRtExpansionLimit )
			{
				// sort expansions by frequency desc
				// clip the less frequent ones if needed, as they are likely misspellings
				sphSort ( m_dWordExpand.Begin(), m_dWordExpand.GetLength(), ExpandedOrderDesc_T<RtExpandedEntry_t>() );
				m_dWordExpand.Resize ( iRtExpansionLimit );
			}

			// lets merge statistics for same words from different segments
			// as hash produce a lot tiny allocations here
			const BYTE * sBase = m_dWordBuf.Begin();
			RtExpandedTraits_fn fnCmp ( sBase );
			sphSort ( m_dWordExpand.Begin(), m_dWordExpand.GetLength(), fnCmp );

			const RtExpandedEntry_t * pLast = m_dWordExpand.Begin();
			tArgs.AddExpanded ( sBase+pLast->m_iNameOff+1, sBase[pLast->m_iNameOff], pLast->m_iDocs, pLast->m_iHits );
			iTotalDocs += pLast->m_iDocs;
			iTotalHits += pLast->m_iHits;
			for ( int i=1; i<m_dWordExpand.GetLength(); ++i )
			{
				const RtExpandedEntry_t * pCur = m_dWordExpand.Begin() + i;

				if ( fnCmp.IsEqual ( pLast, pCur ) )
				{
					tArgs.m_dExpanded.Last().m_iDocs += pCur->m_iDocs;
					tArgs.m_dExpanded.Last().m_iHits += pCur->m_iHits;
				} else
				{
					tArgs.AddExpanded ( sBase + pCur->m_iNameOff + 1, sBase[pCur->m_iNameOff],
							pCur->m_iDocs, pCur->m_iHits );
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
			for ( auto& tSeg: m_dSeg )
			{
				// reverse per segment offset to payload doc-list as offset was the end instead of start
				assert ( tSeg.m_uOff>=tSeg.m_uLen );
				tSeg.m_uOff = tSeg.m_uOff - tSeg.m_uLen;

				// per segment expansion limit clip
				if ( uExpansionLimit && tSeg.m_uLen>uExpansionLimit )
				{
					// sort expansions by frequency desc
					// per segment clip the less frequent ones if needed, as they are likely misspellings
					sphSort ( m_dWordPayload.Begin()+tSeg.m_uOff, tSeg.m_uLen,
							ExpandedOrderDesc_T<RtExpandedPayload_t>() );
					tSeg.m_uLen = uExpansionLimit;
				}

				iPayloads += tSeg.m_uLen;
				// sort by ascending doc-list offset
				sphSort ( m_dWordPayload.Begin()+tSeg.m_uOff, tSeg.m_uLen,
						bind ( &RtExpandedPayload_t::m_uDoclistOff ) );
			}

			auto * pPayload = new RtSubstringPayload_t ( m_dSeg.GetLength(), iPayloads );

			Slice_t * pDst = pPayload->m_dDoclist.Begin();
			ARRAY_FOREACH ( i, m_dSeg )
			{
				const Slice_t & tSeg = m_dSeg[i];
				const RtExpandedPayload_t * pSrc = m_dWordPayload.Begin() + tSeg.m_uOff;
				const RtExpandedPayload_t * pEnd = pSrc + tSeg.m_uLen;
				pPayload->m_dSegment2Doclists[i].m_uOff = pPayload->m_dDoclist.Idx(pDst);
				pPayload->m_dSegment2Doclists[i].m_uLen = tSeg.m_uLen;
				while ( pSrc!=pEnd )
				{
					pDst->m_uOff = pSrc->m_uDoclistOff;
					pDst->m_uLen = pSrc->m_iDocs;
					iTotalDocs += pSrc->m_iDocs;
					iTotalHits += pSrc->m_iHits;
					++pDst;
					++pSrc;
				}
			}
			pPayload->m_iTotalDocs = iTotalDocs;
			pPayload->m_iTotalHits = iTotalHits;
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
	int								m_iSegExpansionLimit = 0;
};


void RtIndex_c::GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

	const CSphFixedVector<RtSegment_t*> & dSegments = *((CSphFixedVector<RtSegment_t*> *)tArgs.m_pIndexData);
	DictEntryRtPayload_t tDict2Payload ( tArgs.m_bPayload, dSegments.GetLength() );
	const int iSkipMagic = ( BYTE(*sSubstring)<0x20 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	ARRAY_FOREACH ( iSeg, dSegments )
	{
		const RtSegment_t * pCurSeg = dSegments[iSeg];
		RtWordReader_t tReader ( pCurSeg, true, m_iWordsCheckpoint, m_tSettings.m_eHitless );

		// find initial checkpoint or check words prior to 1st checkpoint
		if ( pCurSeg->m_dWordCheckpoints.GetLength() )
		{
			const RtWordCheckpoint_t * pCurCheckpoint = sphSearchCheckpoint ( sSubstring, iSubLen, 0, true, true
				, pCurSeg->m_dWordCheckpoints.Begin(), &pCurSeg->m_dWordCheckpoints.Last() );

			if ( pCurCheckpoint )
			{
				// there could be valid data prior 1st checkpoint that should be unpacked and checked
				auto iCheckpointNameLen = (int) strlen ( pCurCheckpoint->m_sWord );
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
			} else if ( iCmp==0 && iSubLen<=pWord->m_sWord[0]
				&& sphWildcardMatch ( (const char *)pWord->m_sWord+1+iSkipMagic, sWildcard, pWildcard ) )
			{
				tDict2Payload.Add ( pWord, iSeg );
			}
			// FIXME!!! same case 'boxi*' matches 'box' document at plain index
			// but masked by a checkpoint search
		}
	}

	tDict2Payload.Convert ( tArgs );
}


bool ExtractInfixCheckpoints ( const char * sInfix, int iBytes, int iMaxCodepointLength, int iDictCpCount, const CSphTightVector<uint64_t> & dFilter, CSphVector<DWORD> & dCheckpoints )
{
	if ( !dFilter.GetLength() )
		return false;

	int iStart = dCheckpoints.GetLength();

	uint64_t dVals[ BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT ];
	memset ( dVals, 0, sizeof(dVals) );

	BloomGenTraits_t tBloom0 ( dVals );
	BloomGenTraits_t tBloom1 ( dVals+BLOOM_PER_ENTRY_VALS_COUNT );
	if ( !BuildBloom ( (const BYTE *)sInfix, iBytes, BLOOM_NGRAM_0, ( iMaxCodepointLength>1 ), BLOOM_PER_ENTRY_VALS_COUNT, tBloom0 ) )
		return false;
	BuildBloom ( (const BYTE *)sInfix, iBytes, BLOOM_NGRAM_1, ( iMaxCodepointLength>1 ), BLOOM_PER_ENTRY_VALS_COUNT, tBloom1 );

	for ( int iDictCp=0; iDictCp<iDictCpCount+1; iDictCp++ )
	{
		const uint64_t * pCP = dFilter.Begin() + iDictCp * BLOOM_PER_ENTRY_VALS_COUNT * BLOOM_HASHES_COUNT;
		const uint64_t * pSuffix = dVals;

		bool bMatched = true;
		for ( int iElem=0; iElem<BLOOM_PER_ENTRY_VALS_COUNT*BLOOM_HASHES_COUNT; iElem++ )
		{
			uint64_t uFilter = *pCP++;
			uint64_t uSuffix = *pSuffix++;
			if ( ( uFilter & uSuffix )!=uSuffix )
			{
				bMatched = false;
				break;
			}
		}

		if ( bMatched )
			dCheckpoints.Add ( (DWORD)iDictCp );
	}

	return ( dCheckpoints.GetLength()!=iStart );
}


void RtIndex_c::GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	// sanity checks
	if ( !sSubstring || iSubLen<=0 )
		return;

	// find those prefixes
	CSphVector<DWORD> dPoints;
	const int iSkipMagic = ( tArgs.m_bHasExactForms ? 1 : 0 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
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

		int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
		int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

		// walk those checkpoints, check all their words
		ARRAY_FOREACH ( i, dPoints )
		{
			int iNext = (int)dPoints[i];
			int iCur = iNext-1;
			RtWordReader_t tReader ( pSeg, true, m_iWordsCheckpoint, m_tSettings.m_eHitless );
			if ( iCur>0 )
				tReader.m_pCur = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iCur].m_iOffset;
			if ( iNext<pSeg->m_dWordCheckpoints.GetLength() )
				tReader.m_pMax = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iNext].m_iOffset;

			const RtWord_t * pWord = NULL;
			while ( ( pWord = tReader.UnzipWord() )!=NULL )
			{
				if ( tArgs.m_bHasExactForms && pWord->m_sWord[1]!=MAGIC_WORD_HEAD_NONSTEMMED )
					continue;

				// check it
				if ( !sphWildcardMatch ( (const char*)pWord->m_sWord+1+iSkipMagic, sWildcard, pWildcard ) )
					continue;

				// matched, lets add
				tDict2Payload.Add ( pWord, iSeg );
			}
		}
	}

	tDict2Payload.Convert ( tArgs );
}

void RtIndex_c::GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const
{
	SphChunkGuard_t tGuard = GetReaderChunks ();

	const CSphFixedVector<RtSegmentRefPtf_t> & dSegments = tGuard.m_dRamChunks;

	// segments and disk chunks dictionaries produce duplicated entries
	tRes.m_bMergeWords = true;

	if ( dSegments.GetLength() )
	{
		assert ( !tRes.m_pWordReader && !tRes.m_pSegments );
		tRes.m_pWordReader = new RtWordReader_t ( dSegments[0], true, m_iWordsCheckpoint, m_tSettings.m_eHitless );
		tRes.m_pSegments = &tGuard.m_dRamChunks;
		tRes.m_bHasExactDict = m_tSettings.m_bIndexExactWords;

		// FIXME!!! cache InfixCodepointBytes as it is slow - GetMaxCodepointLength is charset_table traverse
		sphGetSuggest ( this, m_pTokenizer->GetMaxCodepointLength(), tArgs, tRes );

		auto pReader = ( RtWordReader_t * ) tRes.m_pWordReader;
		SafeDelete ( pReader );
		tRes.m_pWordReader = NULL;
		tRes.m_pSegments = NULL;
	}

	int iWorstCount = 0;
	// check disk chunks from recent to oldest
	for ( int i=tGuard.m_dDiskChunks.GetLength()-1; i>=0; i-- )
	{
		int iWorstDist = 0;
		int iWorstDocs = 0;
		if ( tRes.m_dMatched.GetLength() )
		{
			iWorstDist = tRes.m_dMatched.Last().m_iDistance;
			iWorstDocs = tRes.m_dMatched.Last().m_iDocs;
		}

		tGuard.m_dDiskChunks[i]->GetSuggest ( tArgs, tRes );

		// stop checking in case worst element is same several times during loop
		if ( tRes.m_dMatched.GetLength() && iWorstDist==tRes.m_dMatched.Last().m_iDistance && iWorstDocs==tRes.m_dMatched.Last().m_iDocs )
		{
			iWorstCount++;
			if ( iWorstCount>2 )
				break;
		} else
		{
			iWorstCount = 0;
		}
	}
}

void RtIndex_c::SuffixGetChekpoints ( const SuggestResult_t & tRes, const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const
{
	const CSphFixedVector<const RtSegment_t*> & dSegments = *( (const CSphFixedVector<const RtSegment_t*> *)tRes.m_pSegments );
	assert ( dSegments.GetLength()<0xFF );

	ARRAY_FOREACH ( iSeg, dSegments )
	{
		const RtSegment_t * pSeg = dSegments[iSeg];
		if ( !pSeg->m_dWords.GetLength () )
			continue;

		int iStart = dCheckpoints.GetLength();
		if ( !ExtractInfixCheckpoints ( sSuffix, iLen, m_iMaxCodepointLength, pSeg->m_dWordCheckpoints.GetLength(), pSeg->m_dInfixFilterCP, dCheckpoints ) )
			continue;

		DWORD iSegPacked = (DWORD)iSeg<<24;
		for ( int i=iStart; i<dCheckpoints.GetLength(); i++ )
		{
			assert ( ( dCheckpoints[i] & 0xFFFFFF )==dCheckpoints[i] );
			dCheckpoints[i] |= iSegPacked;
		}
	}
}

void RtIndex_c::SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const
{
	assert ( tRes.m_pWordReader && tRes.m_pSegments );
	const CSphFixedVector<const RtSegment_t*> & dSegments = *( (const CSphFixedVector<const RtSegment_t*> *)tRes.m_pSegments );
	RtWordReader_t * pReader = (RtWordReader_t *)tRes.m_pWordReader;

	int iSeg = iCP>>24;
	assert ( iSeg>=0 && iSeg<dSegments.GetLength() );
	const RtSegment_t * pSeg = dSegments[iSeg];
	pReader->Reset ( pSeg );

	int iNext = (int)( iCP & 0xFFFFFF );
	int iCur = iNext-1;

	if ( iCur>0 )
		pReader->m_pCur = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iCur].m_iOffset;
	if ( iNext<pSeg->m_dWordCheckpoints.GetLength() )
		pReader->m_pMax = pSeg->m_dWords.Begin() + pSeg->m_dWordCheckpoints[iNext].m_iOffset;
}

bool RtIndex_c::ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const
{
	assert ( tRes.m_pWordReader );
	RtWordReader_t * pReader = (RtWordReader_t *)tRes.m_pWordReader;

	const RtWord_t * pWord = pReader->UnzipWord();

	if ( !pWord )
		return false;

	tWord.m_sWord = (const char *)( pWord->m_sWord + 1 );
	tWord.m_iLen = pWord->m_sWord[0];
	tWord.m_iDocs = pWord->m_uDocs;
	return true;
}


bool RtIndex_c::RtQwordSetup ( RtQword_t * pQword, int iSeg, const SphChunkGuard_t & tGuard ) const
{
	// segment-specific setup pass
	if ( iSeg>=0 )
		return RtQwordSetupSegment ( pQword, tGuard.m_dRamChunks[iSeg], true, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings );

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
		bFound |= RtQwordSetupSegment ( pQword, tGuard.m_dRamChunks[i], false, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings );

	// sanity check
	assert (!( bFound==true && pQword->m_iDocs==0 ) );
	return bFound;
}


void SetupExactDict ( DictRefPtr_c &pDict, ISphTokenizer * pTokenizer, bool bAddSpecial )
{
	assert ( pTokenizer );
	pTokenizer->AddPlainChar ( '=' );
	if ( bAddSpecial )
		pTokenizer->AddSpecials ( "=" );
	pDict = new CSphDictExact ( pDict );
}


void SetupStarDict ( DictRefPtr_c& pDict, ISphTokenizer * pTokenizer )
{
	assert ( pTokenizer );
	pTokenizer->AddPlainChar ( '*' );
	pDict = new CSphDictStarV8 ( pDict, true );
}

class SphRtFinalMatchCalc_c : public ISphMatchProcessor, ISphNoncopyable
{
private:
	const CSphQueryContext &	m_tCtx;
	int							m_iSeg = 0;
	int							m_iSegments;
	// count per segments matches
	// to skip iteration of matches at sorter and pool setup for segment without matches at sorter
	CSphBitvec					m_dSegments;

public:
	SphRtFinalMatchCalc_c ( int iSegments, const CSphQueryContext & tCtx )
		: m_tCtx ( tCtx )
		, m_iSegments ( iSegments )
	{
		m_dSegments.Init ( iSegments );
	}

	bool NextSegment ( int iSeg )
	{
		m_iSeg = iSeg;

		bool bSegmentGotRows = m_dSegments.BitGet ( iSeg );

		// clear current row
		m_dSegments.BitClear ( iSeg );

		// also clear 0 segment as it got forced to process
		m_dSegments.BitClear ( 0 );

		// also force to process 0 segment to mark all used segments
		return ( iSeg==0 || bSegmentGotRows );
	}

	bool HasSegments () const
	{
		return ( m_iSeg==0 || m_dSegments.BitCount()>0 );
	}

	void Process ( CSphMatch * pMatch ) final
	{
		int iMatchSegment = pMatch->m_iTag-1;
		if ( iMatchSegment==m_iSeg && pMatch->m_pStatic )
			m_tCtx.CalcFinal ( *pMatch );

		// count all used segments at 0 pass
		if ( m_iSeg==0 && iMatchSegment<m_iSegments )
			m_dSegments.BitSet ( iMatchSegment );
	}
};

static void TransformSorterSchema ( ISphMatchSorter * pSorter, const SphChunkGuard_t & tGuard, const CSphVector<const BYTE *> & dDiskBlobPools )
{
	assert ( pSorter );
	pSorter->TransformPooled2StandalonePtrs([&] ( const CSphMatch * pMatch )
	{
		int nRamChunks = tGuard.m_dRamChunks.GetLength ();
		int iChunkId = pMatch->m_iTag-1;
		if ( iChunkId<nRamChunks )
			return (const BYTE *) tGuard.m_dRamChunks[iChunkId]->m_dBlobs.Begin ();

		return dDiskBlobPools[iChunkId-nRamChunks];
	});
}

SphChunkGuard_t RtIndex_c::GetReaderChunks () const
{
	SphChunkGuard_t tGuard ( &m_tReading );

	ScRL_t tChunkRLock ( m_tChunkLock );
	if ( m_dRamChunks.IsEmpty() && m_dDiskChunks.IsEmpty() )
		return tGuard;

	tGuard.m_dRamChunks.CopyFrom ( m_dRamChunks );
	tGuard.m_dDiskChunks.CopyFrom ( m_dDiskChunks );
	return tGuard;
}

SphChunkGuard_t RtIndex_c::GetReaderChunks ( const VecTraits_T<int64_t>& dChunks ) const
{
	if ( dChunks.IsEmpty() )
		return GetReaderChunks();

	SphChunkGuard_t tGuard ( &m_tReading );

	ScRL_t tChunkRLock ( m_tChunkLock );
	if ( m_dRamChunks.IsEmpty() && m_dDiskChunks.IsEmpty() )
		return tGuard;

	CSphVector<int> dOrderedChunks;
	dChunks.for_each ( [&dOrderedChunks] ( int64_t iVal ) { dOrderedChunks.Add ( (int) iVal ); } );
	dOrderedChunks.Uniq(); // implies also Sort()

	auto iDiskBound = m_dDiskChunks.GetLength();
	auto iAllBound = iDiskBound + m_dRamChunks.GetLength();
	int iDiskSelected = 0;
	int iRamSelected = 0;
	dOrderedChunks.any_of ( [&] ( int iVal )
	{
		if ( iVal<iDiskBound )
		{
			++iDiskSelected;
			return false;
		}
		if ( iVal<iAllBound )
		{
			++iRamSelected;
			return false;
		}
		return true;
	});

	tGuard.m_dDiskChunks.Reset ( iDiskSelected );
	for ( int i = 0; i<iDiskSelected; ++i )
		tGuard.m_dDiskChunks[i] = m_dDiskChunks[dOrderedChunks[i]];

	tGuard.m_dRamChunks.Reset ( iRamSelected );
	for ( int i = 0; i<iRamSelected; ++i )
		tGuard.m_dRamChunks[i] = m_dRamChunks[dOrderedChunks[iDiskSelected+i]-iDiskBound];

	return tGuard;
}

SphChunkGuard_t::SphChunkGuard_t ( CSphRwlock * pReading ) : m_pReading ( pReading )
{
	if ( pReading )
		pReading->ReadLock ();
}

void SphChunkGuard_t::Swap ( SphChunkGuard_t & rhs ) noexcept
{
	m_dRamChunks.SwapData (rhs.m_dRamChunks);
	m_dDiskChunks.SwapData (rhs.m_dDiskChunks);
	::Swap ( m_pReading, rhs.m_pReading);
}

SphChunkGuard_t::SphChunkGuard_t ( SphChunkGuard_t && rhs ) noexcept
{
	Swap(rhs);
}

SphChunkGuard_t & SphChunkGuard_t::operator= ( SphChunkGuard_t && rhs ) noexcept
{
	Swap(rhs);
	return *this;
}


SphChunkGuard_t::~SphChunkGuard_t () RELEASE()
{
	if ( m_pReading )
		m_pReading->Unlock ();
}

namespace { // nameless namespace instead of 'static' modifier

struct DiskChunkSearcherCtx_t
{
	using Sorters_t = VecTraits_T<ISphMatchSorter *>;

	Sorters_t&			m_dSorters;
	CSphQueryResultMeta&	m_tMeta;

	DiskChunkSearcherCtx_t ( Sorters_t & dSorters, CSphQueryResultMeta & tMeta )
		: m_dSorters ( dSorters )
		, m_tMeta ( tMeta )
	{}

	// called from finalize.
	void MergeChild ( DiskChunkSearcherCtx_t dChild ) const
	{
		// sorting results
		ARRAY_FOREACH ( i, m_dSorters )
			if ( dChild.m_dSorters[i] )
				dChild.m_dSorters[i]->MoveTo ( m_dSorters[i] );

		auto& dChildRes = dChild.m_tMeta;

		// word statistics
		if ( m_tMeta.m_hWordStats.GetLength ())
			for ( auto & tStat : m_tMeta.m_hWordStats )
			{
				const auto * pDstStat = dChildRes.m_hWordStats ( tStat.first );
				if ( pDstStat )
					m_tMeta.AddStat ( tStat.first, pDstStat->first, pDstStat->second );
			}
		else
			m_tMeta.m_hWordStats = dChildRes.m_hWordStats;

		// other data (warnings, errors, etc.)
		// errors
		if ( !dChildRes.m_sError.IsEmpty ())
			m_tMeta.m_sError = dChildRes.m_sError;

		// warnings
		if ( !dChildRes.m_sWarning.IsEmpty ())
			m_tMeta.m_sWarning = dChildRes.m_sWarning;

		// prediction counters
		if ( m_tMeta.m_bHasPrediction )
			m_tMeta.m_tStats.Add ( dChildRes.m_tStats );

		// profiling
		if ( dChildRes.m_pProfile )
			m_tMeta.m_pProfile->AddMetric ( *dChildRes.m_pProfile );
	}

	inline bool IsClonable () const
	{
		return m_dSorters.all_of ( [] ( const ISphMatchSorter * p ) { return p->CanBeCloned (); } );
	}
};

struct DiskChunkSearcherCloneCtx_t
{
	CSphVector<ISphMatchSorter *>	m_dSorters;
	CSphQueryResultMeta				m_tMeta;

	explicit DiskChunkSearcherCloneCtx_t ( const DiskChunkSearcherCtx_t& dParent )
	{
		m_dSorters.Resize ( dParent.m_dSorters.GetLength ());
		ARRAY_FOREACH ( i, m_dSorters )
			m_dSorters[i] = dParent.m_dSorters[i]->Clone ();

		m_tMeta.m_bHasPrediction = dParent.m_tMeta.m_bHasPrediction;
		if ( dParent.m_tMeta.m_pProfile )
			m_tMeta.m_pProfile = new QueryProfile_c;
	}

	~DiskChunkSearcherCloneCtx_t()
	{
		m_dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete ( pSorter ); } );
		SafeDelete ( m_tMeta.m_pProfile );
	}

	explicit operator DiskChunkSearcherCtx_t()
	{
		return { m_dSorters, m_tMeta };
	}
};

void QueryDiskChunks ( const CSphQuery & tQuery,
		CSphQueryResultMeta& tResult,
		const CSphMultiQueryArgs & tArgs,
		SphChunkGuard_t& tGuard,
		VecTraits_T<ISphMatchSorter *>& dSorters,
		QueryProfile_c * pProfiler,
		bool bGotLocalDF,
		const SmallStringHash_T<int64_t> * pLocalDocs,
		int64_t iTotalDocs,
		const char * szIndexName,
		VecTraits_T<const BYTE*>& dDiskBlobPools,
		int64_t tmMaxTimer
		)
{
	if ( tGuard.m_dDiskChunks.IsEmpty() )
		return;

	assert ( !dSorters.IsEmpty () );

	// counter of tasks we will issue now
	int iJobs = tGuard.m_dDiskChunks.GetLength ();

	// the context
	ClonableCtx_T<DiskChunkSearcherCtx_t, DiskChunkSearcherCloneCtx_t> dCtx { dSorters, tResult };

	auto iConcurrency = tQuery.m_iCouncurrency;
	if ( !iConcurrency )
		iConcurrency = GetEffectiveDistThreads ();
	dCtx.LimitConcurrency ( iConcurrency );

	auto iStart = sphMicroTimer ();
	sphLogDebugv ( "Started: " INT64_FMT, sphMicroTimer()-iStart );

	// because disk chunk search within the loop will switch the profiler state
	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	std::atomic<bool> bInterrupt {false};
	std::atomic<int32_t> iCurChunk { iJobs-1 };
	CoExecuteN ( dCtx.Concurrency ( iJobs ), [&]
	{
		auto iChunk = iCurChunk.fetch_sub ( 1, std::memory_order_acq_rel );
		if ( iChunk<0 || bInterrupt )
			return; // already nothing to do, early finish.

		auto tCtx = dCtx.CloneNewContext ();
		Threads::CoThrottler_c tThrottler ( myinfo::ThrottlingPeriodMS () );
		int iTick=1; // num of times coro rescheduled by throttler
		while ( !bInterrupt ) // some earlier job met error; abort.
		{
			myinfo::SetThreadInfo ( "%d ch %d:", iTick, iChunk );
			auto & dLocalSorters = tCtx.m_dSorters;
			CSphQueryResultMeta tChunkMeta;
			CSphQueryResult tChunkResult;
			tChunkResult.m_pMeta = &tChunkMeta;
			CSphQueryResultMeta& tThMeta = tCtx.m_tMeta;
			tChunkMeta.m_pProfile = tThMeta.m_pProfile;

			CSphMultiQueryArgs tMultiArgs ( tArgs.m_iIndexWeight );
			// storing index in matches tag for finding strings attrs offset later, biased against default zero and segments
			tMultiArgs.m_iTag = tGuard.m_dRamChunks.GetLength ()+iChunk+1;
			tMultiArgs.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
			tMultiArgs.m_bLocalDF = bGotLocalDF;
			tMultiArgs.m_pLocalDocs = pLocalDocs;
			tMultiArgs.m_iTotalDocs = iTotalDocs;

			// we use sorters in both disk chunks and ram chunks,
			// that's why we don't want to move to a new schema before we searched ram chunks
			tMultiArgs.m_bModifySorterSchemas = false;

			bInterrupt = !tGuard.m_dDiskChunks[iChunk]->MultiQuery ( tChunkResult, tQuery, dLocalSorters, tMultiArgs );

			// check terms inconsistency among disk chunks
			tCtx.m_tMeta.MergeWordStats ( tChunkMeta );

			dDiskBlobPools[iChunk] = tChunkResult.m_pBlobPool;

			if ( tThMeta.m_bHasPrediction )
				tThMeta.m_tStats.Add ( tChunkMeta.m_tStats );

			if ( iChunk && tmMaxTimer>0 && sph::TimeExceeded ( tmMaxTimer ) )
			{
				tThMeta.m_sWarning = "query time exceeded max_query_time";
				bInterrupt = true;
			}

			if ( bInterrupt && !tChunkMeta.m_sError.IsEmpty ())
				// FIXME? maybe handle this more gracefully (convert to a warning)?
				tThMeta.m_sError = tChunkMeta.m_sError;

			iChunk = iCurChunk.fetch_sub ( 1, std::memory_order_acq_rel );
			if ( iChunk<0 || bInterrupt )
				return; // all is done

			// yield and reschedule every quant of time. It gives work to other tasks
			if ( tThrottler.ThrottleAndKeepCrashQuery () )
				// report current disk chunk processing
				++iTick;
		}
	});
	dCtx.Finalize();
}

void FinalExpressionCalculation( CSphQueryContext& tCtx,
		const VecTraits_T<RtSegmentRefPtf_t>& dRamChunks,
		VecTraits_T<ISphMatchSorter *>& dSorters )
{
	if ( tCtx.m_dCalcFinal.IsEmpty () )
		return;

	const int iSegmentsTotal = dRamChunks.GetLength ();

	// at 0 pass processor also fills bitmask of segments these has matches at sorter
	// then skip sorter processing for these 'empty' segments
	SphRtFinalMatchCalc_c tFinal ( iSegmentsTotal, tCtx );

	ARRAY_FOREACH_COND ( iSeg, dRamChunks, tFinal.HasSegments() )
	{
		if ( !tFinal.NextSegment ( iSeg ) )
			continue;

		// set blob pool for string on_sort expression fix up
		tCtx.SetBlobPool ( dRamChunks[iSeg]->m_dBlobs.Begin() );
		dSorters.Apply ( [&tFinal] ( ISphMatchSorter * pTop ) { pTop->Finalize ( tFinal, false ); } );
	}
}

// perform initial query transformations and expansion.
int PrepareFTSearch ( const RtIndex_c * pThis,
		bool bIsStarDict,
		bool bKeywordDict,
		int iExpandKeywords,
		int iExpansionLimit,
		const char * sModifiedQuery,
		const CSphIndexSettings& tSettings,
		const QueryParser_i * pQueryParser,
		const CSphQuery& tQuery,
		const CSphSchema & tSchema,
		void * pIndexData,
		ISphTokenizer * pTokenizer,
		ISphTokenizer * pQueryTokenizer,
		CSphDict* pDict,
		CSphQueryResultMeta& tMeta,
		QueryProfile_c* pProfiler,
		CSphScopedPayload* pPayloads,
		XQQuery_t & tParsed )
{
	// OPTIMIZE! make a lightweight clone here? and/or remove double clone?
	TokenizerRefPtr_c pQueryTokenizerJson { pTokenizer };
	sphSetupQueryTokenizer ( pQueryTokenizerJson, bIsStarDict, tSettings.m_bIndexExactWords, true );

	if ( !pQueryParser->ParseQuery ( tParsed, sModifiedQuery, &tQuery, pQueryTokenizer
									 , pQueryTokenizerJson, &tSchema, pDict, tSettings ) )
	{
		tMeta.m_sError = tParsed.m_sParseError;
		return 0;
	}

	if ( !tParsed.m_sParseWarning.IsEmpty () )
		tMeta.m_sWarning = tParsed.m_sParseWarning;

	// transform query if needed (quorum transform, etc.)
	SwitchProfile ( pProfiler, SPH_QSTATE_TRANSFORMS );

	// FIXME!!! provide segments list instead index
	sphTransformExtendedQuery ( &tParsed.m_pRoot, tSettings, tQuery.m_bSimplify, pThis );

	int iExpandKw = ExpandKeywords ( iExpandKeywords, tQuery.m_eExpandKeywords, tSettings, bKeywordDict );
	if ( iExpandKw!=KWE_DISABLED )
	{
		sphQueryExpandKeywords ( &tParsed.m_pRoot, tSettings, iExpandKw, bKeywordDict );
		tParsed.m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	if ( tSettings.m_uAotFilterMask )
		TransformAotFilter ( tParsed.m_pRoot, pDict->GetWordforms (), tSettings );

	// expanding prefix in word dictionary case
	if ( bKeywordDict && bIsStarDict )
	{
		ExpansionContext_t tExpCtx;
		tExpCtx.m_pWordlist = pThis;
		tExpCtx.m_pBuf = nullptr;
		tExpCtx.m_pResult = &tMeta;
		tExpCtx.m_iMinPrefixLen = tSettings.GetMinPrefixLen ( bKeywordDict );
		tExpCtx.m_iMinInfixLen = tSettings.m_iMinInfixLen;
		tExpCtx.m_iExpansionLimit = iExpansionLimit;
		tExpCtx.m_bHasExactForms = ( pDict->HasMorphology () || tSettings.m_bIndexExactWords );
		tExpCtx.m_bMergeSingles = ( tQuery.m_uDebugFlags & QUERY_DEBUG_NO_PAYLOAD )==0;
		tExpCtx.m_pPayloads = pPayloads;
		tExpCtx.m_pIndexData = pIndexData;

		tParsed.m_pRoot = sphExpandXQNode ( tParsed.m_pRoot, tExpCtx ); // here magics happens
	}
	return ConsiderStack ( tParsed.m_pRoot, tMeta.m_sError );
}

// setup filters
bool SetupFilters ( const CSphQuery & tQuery,
		const ISphSchema* pSchema,
		bool bFullscan,
		CSphQueryContext & tCtx,
		CSphString& sError,
		CSphString& sWarning )
{
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &tQuery.m_dFilters;
	tFlx.m_pFilterTree = &tQuery.m_dFilterTree;
	tFlx.m_pSchema = pSchema;
	tFlx.m_eCollation = tQuery.m_eCollation;
	tFlx.m_bScan = bFullscan;
	return tCtx.CreateFilters ( tFlx, sError, sWarning );
}

void PerformFullScan ( const VecTraits_T<RtSegmentRefPtf_t> & dRamChunks,
		int iMaxDynamicSize,
		int iIndexWeight,
		int iStride,
		int iCutoff,
		int64_t tmMaxTimer,
		QueryProfile_c* pProfiler,
		CSphQueryContext& tCtx,
		VecTraits_T<ISphMatchSorter*>& dSorters,
		CSphString& sWarning )
{
	bool bRandomize = dSorters[0]->m_bRandomize;

	SwitchProfile ( pProfiler, SPH_QSTATE_FULLSCAN );

	// full scan
	// FIXME? OPTIMIZE? add shortcuts here too?
	CSphMatch tMatch;
	tMatch.Reset ( iMaxDynamicSize );
	tMatch.m_iWeight = iIndexWeight;

	ARRAY_FOREACH ( iSeg, dRamChunks )
	{
		// set string pool for string on_sort expression fix up
		tCtx.SetBlobPool ( dRamChunks[iSeg]->m_dBlobs.Begin() );
		for ( auto* pSorter: dSorters )
			pSorter->SetBlobPool ( dRamChunks[iSeg]->m_dBlobs.Begin() );

		RtRowIterator_c tIt ( dRamChunks[iSeg], iStride );
		while (true)
		{
			const CSphRowitem * pRow = tIt.GetNextAliveRow();
			if ( !pRow )
				break;

			tMatch.m_tRowID = tIt.GetRowID();
			tMatch.m_pStatic = pRow;

			tCtx.CalcFilter ( tMatch );
			if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
			{
				tCtx.FreeDataFilter ( tMatch );
				continue;
			}

			if ( bRandomize )
				tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

			tCtx.CalcSort ( tMatch );

			// storing segment in matches tag for finding strings attrs offset later, biased against default zero
			tMatch.m_iTag = iSeg+1;

			bool bNewMatch = false;
			for ( auto* pSorter: dSorters )
				bNewMatch |= pSorter->Push ( tMatch );

			// stringptr expressions should be duplicated (or taken over) at this point
			tCtx.FreeDataFilter ( tMatch );
			tCtx.FreeDataSort ( tMatch );

			// handle cutoff
			if ( bNewMatch )
				if ( --iCutoff==0 )
					break;

			// handle timer
			if ( tmMaxTimer && sph::TimeExceeded ( tmMaxTimer ) )
			{
				sWarning = "query time exceeded max_query_time";
				iSeg = dRamChunks.GetLength() - 1;	// outer break
				break;
			}
		}

		if ( !iCutoff )
			break;
	}
}

bool DoFullScanQuery ( const VecTraits_T<RtSegmentRefPtf_t> & dRamChunks,
		const ISphSchema& tMaxSorterSchema,
		const CSphQuery& tQuery,
		int iIndexWeight,
		int iStride,
		int64_t tmMaxTimer,
		QueryProfile_c* pProfiler,
		CSphQueryContext& tCtx,
		VecTraits_T<ISphMatchSorter*>& dSorters,
		CSphQueryResultMeta& tMeta )
{
	// probably redundant, but just in case
	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	// search segments no looking to max_query_time
	// FIXME!!! move searching at segments before disk chunks as result set is safe with kill-lists
	if ( !dRamChunks.IsEmpty () )
	{
		if ( !SetupFilters ( tQuery, &tMaxSorterSchema, true, tCtx, tMeta.m_sError, tMeta.m_sWarning ) )
			return false;
		// FIXME! OPTIMIZE! check if we can early reject the whole index

		// do searching
		int iCutoff = tQuery.m_iCutoff;
		if ( iCutoff<=0 )
			iCutoff = -1;
		PerformFullScan ( dRamChunks, tMaxSorterSchema.GetDynamicSize (), iIndexWeight, iStride
						  , iCutoff, tmMaxTimer, pProfiler, tCtx, dSorters, tMeta.m_sWarning );
	}

	FinalExpressionCalculation ( tCtx, dRamChunks, dSorters );
	return true;
}

void PerformFullTextSearch ( const VecTraits_T<RtSegmentRefPtf_t> & dRamChunks,
		RtQwordSetup_t& tTermSetup,
		ISphRanker* pRanker,
		int iIndexWeight,
		int iCutoff,
		QueryProfile_c* pProfiler,
		CSphQueryContext& tCtx,
		VecTraits_T<ISphMatchSorter*>& dSorters )
{
	bool bRandomize = dSorters[0]->m_bRandomize;
	// query matching
	ARRAY_FOREACH ( iSeg, dRamChunks )
	{
		const RtSegment_t * pSeg = dRamChunks[iSeg];

		SwitchProfile ( pProfiler, SPH_QSTATE_INIT_SEGMENT );

		tTermSetup.SetSegment ( iSeg );
		pRanker->Reset ( tTermSetup );

		// for lookups to work
		tCtx.m_pIndexData = pSeg;
		tCtx.m_pIndexSegment = pSeg;

		// set blob pool for string on_sort expression fix up
		const BYTE * pBlobPool = pSeg->m_dBlobs.Begin ();
		tCtx.SetBlobPool ( pBlobPool );
		for ( auto * pSorter : dSorters )
			pSorter->SetBlobPool ( pBlobPool );

		pRanker->ExtraData ( EXTRA_SET_BLOBPOOL, (void**)&pBlobPool );

		CSphMatch * pMatch = pRanker->GetMatchesBuffer();
		while (true)
		{
			// ranker does profile switches internally in GetMatches()
			int iMatches = pRanker->GetMatches();
			if ( iMatches<=0 )
				break;

			SwitchProfile ( pProfiler, SPH_QSTATE_SORT );

			for ( int i=0; i<iMatches; i++ )
			{
				pMatch[i].m_pStatic = pSeg->GetDocinfoByRowID ( pMatch[i].m_tRowID );
				pMatch[i].m_iWeight *= iIndexWeight;
				if ( bRandomize )
					pMatch[i].m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

				tCtx.CalcSort ( pMatch[i] );

				if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( pMatch[i] ) )
				{
					tCtx.FreeDataSort ( pMatch[i] );
					continue;
				}

				// storing segment in matches tag for finding strings attrs offset later, biased against default zero
				pMatch[i].m_iTag = iSeg+1;

				bool bNewMatch = false;
				for ( auto* pSorter : dSorters )
				{
					bNewMatch |= pSorter->Push ( pMatch[i] );

					if ( tCtx.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
					{
						pRanker->ExtraData ( EXTRA_SET_MATCHPUSHED, (void**)&(pSorter->m_iJustPushed) );
						pRanker->ExtraData ( EXTRA_SET_MATCHPOPPED, (void**)&(pSorter->m_dJustPopped) );
					}
				}

				// stringptr expressions should be duplicated (or taken over) at this point
				tCtx.FreeDataFilter ( pMatch[i] );
				tCtx.FreeDataSort ( pMatch[i] );

				if ( bNewMatch )
					if ( --iCutoff==0 )
						break;
			}

			if ( iCutoff==0 )
			{
				iSeg = dRamChunks.GetLength();
				break;
			}
		}
	}
}

bool DoFullTextSearch ( const VecTraits_T<RtSegmentRefPtf_t> & dRamChunks,
		const ISphSchema& tMaxSorterSchema,
		const CSphQuery& tQuery,
		const char* szIndexName,
		int iIndexWeight,
		int iMatchPoolSize,
		int iStackNeed,
		RtQwordSetup_t& tTermSetup,
		QueryProfile_c* pProfiler,
		CSphQueryContext& tCtx,
		VecTraits_T<ISphMatchSorter*>& dSorters,
		XQQuery_t& tParsed,
		CSphQueryResultMeta& tMeta,
		ISphMatchSorter* pSorter )
{
	// set zonespanlist settings
	tParsed.m_bNeedSZlist = tQuery.m_bZSlist;

	return Threads::CoContinueBool ( iStackNeed, [&] {

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( nullptr );
	pRanker = sphCreateRanker ( tParsed, tQuery, tMeta, tTermSetup, tCtx, tMaxSorterSchema );
	if ( !pRanker.Ptr () )
		return false;

	tCtx.SetupExtraData ( pRanker.Ptr (), pSorter );

	pRanker->ExtraData ( EXTRA_SET_POOL_CAPACITY, (void **) &iMatchPoolSize );

	// check for the possible integer overflow in m_dPool.Resize
	int64_t iPoolSize = 0;
	if ( pRanker->ExtraData ( EXTRA_GET_POOL_SIZE, (void **) &iPoolSize ) && iPoolSize>INT_MAX )
	{
		tMeta.m_sError.SetSprintf ( "ranking factors pool too big (%d Mb), reduce max_matches",
				(int) ( iPoolSize / 1024 / 1024 ) );
		return false;
	}

	// probably redundant, but just in case
	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	// search segments no looking to max_query_time
	// FIXME!!! move searching at segments before disk chunks as result set is safe with kill-lists
	if ( !dRamChunks.IsEmpty () )
	{
		if ( !SetupFilters ( tQuery, &tMaxSorterSchema, false, tCtx, tMeta.m_sError, tMeta.m_sWarning ) )
			return false;
		// FIXME! OPTIMIZE! check if we can early reject the whole index

		// do searching
		int iCutoff = tQuery.m_iCutoff;
		if ( iCutoff<=0 )
			iCutoff = -1;
		PerformFullTextSearch ( dRamChunks, tTermSetup, pRanker.Ptr (), iIndexWeight, iCutoff, pProfiler, tCtx, dSorters );
	}

	FinalExpressionCalculation ( tCtx, dRamChunks, dSorters );

	//////////////////////
	// copying match's attributes to external storage in result set
	//////////////////////

	SwitchProfile ( pProfiler, SPH_QSTATE_FINALIZE );
	pRanker->FinalizeCache ( tMaxSorterSchema );
	return true;
	});
}

} // nameless namespace

// FIXME! missing MVA, index_exact_words support
// FIXME? any chance to factor out common backend agnostic code?
// FIXME? do we need to support pExtraFilters?
bool RtIndex_c::MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery,
		const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// to avoid the checking of a ppSorters's element for NULL on every next step,
	// just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p) { if ( p ) dSorters.Add(p); });
	auto& tMeta = *tResult.m_pMeta;

	// if we have anything to work with
	if ( dSorters.IsEmpty() )
	{
		tMeta.m_iQueryTime = 0;
		return false;
	}

	assert ( tArgs.m_iTag==0 );

	MEMORY ( MEM_RT_QUERY );

	// start counting
	tMeta.m_iQueryTime = 0;
	int64_t tmQueryStart = sphMicroTimer();
	auto tmCpuQueryStart = sphTaskCpuTimer();
	QueryProfile_c * pProfiler = tMeta.m_pProfile;
	CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_DICT_SETUP );

	// force ext2 mode for them
	// FIXME! eliminate this const breakage
	const_cast<CSphQuery*> ( &tQuery )->m_eMode = SPH_MATCH_EXTENDED2;

	SphChunkGuard_t tGuard = GetReaderChunks ( tQuery.m_dIntSubkeys );

	// debug hack (don't use ram chunk in debug modeling mode)
	if_const( MODELING )
		tGuard.m_dRamChunks.Reset(0);

	// wrappers
	TokenizerRefPtr_c pQueryTokenizer { m_pTokenizer->Clone ( SPH_CLONE_QUERY ) };
	sphSetupQueryTokenizer ( pQueryTokenizer, IsStarDict ( m_bKeywordDict ), m_tSettings.m_bIndexExactWords, false );

	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( m_bKeywordDict && IsStarDict ( m_bKeywordDict ) )
		SetupStarDict ( pDict, pQueryTokenizer );

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict, pQueryTokenizer );

	// calculate local idf for RT with disk chunks
	// in case of local_idf set but no external hash no full-scan query and RT has disk chunks
	const SmallStringHash_T<int64_t> * pLocalDocs = tArgs.m_pLocalDocs;
	SmallStringHash_T<int64_t> hLocalDocs;
	int64_t iTotalDocs = ( tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments );
	bool bGotLocalDF = tArgs.m_bLocalDF;
	if ( tArgs.m_bLocalDF && !tArgs.m_pLocalDocs && !tQuery.m_sQuery.IsEmpty() && tGuard.m_dDiskChunks.GetLength() )
	{
		SwitchProfile ( pProfiler, SPH_QSTATE_LOCAL_DF );
		GetKeywordsSettings_t tSettings;
		tSettings.m_bStats = true;
		CSphVector < CSphKeywordInfo > dKeywords;
		DoGetKeywords ( dKeywords, tQuery.m_sQuery.cstr(), tSettings, false, nullptr, tGuard );
		for ( auto & tKw : dKeywords )
			if ( !hLocalDocs.Exists ( tKw.m_sNormalized ) ) // skip dupes
				hLocalDocs.Add ( tKw.m_iDocs, tKw.m_sNormalized );

		pLocalDocs = &hLocalDocs;
		iTotalDocs = GetStats().m_iTotalDocuments;
		bGotLocalDF = true;
	}

	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	// FIXME! each result will point to its own MVA and string pools

	//////////////////////
	// search disk chunks
	//////////////////////

	tMeta.m_bHasPrediction = tQuery.m_iMaxPredictedMsec>0;

	int64_t tmMaxTimer = 0;
	sph::MiniTimer_c dTimerGuard;
	if ( tQuery.m_uMaxQueryMsec>0 )
		tmMaxTimer = dTimerGuard.MiniTimerEngage ( tQuery.m_uMaxQueryMsec ); // max_query_time

	CSphVector<const BYTE *> dDiskBlobPools ( tGuard.m_dDiskChunks.GetLength() );

	if ( !tGuard.m_dDiskChunks.IsEmpty () )
		QueryDiskChunks ( tQuery,tMeta,tArgs,tGuard,dSorters,pProfiler,bGotLocalDF,pLocalDocs,iTotalDocs,
				m_sIndexName.cstr(), dDiskBlobPools, tmMaxTimer);

	////////////////////
	// search RAM chunk
	////////////////////

	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaIndex, iMatchPoolSize;
	std::tie ( iMaxSchemaIndex, iMatchPoolSize ) = GetMaxSchemaIndexAndMatchCapacity ( dSorters );

	if ( iMaxSchemaIndex==-1 )
		return false;

	const ISphSchema & tMaxSorterSchema = *( dSorters[iMaxSchemaIndex]->GetSchema ());
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex );

	// setup calculations and result schema
	CSphQueryContext tCtx ( tQuery );
	tCtx.m_pProfile = pProfiler;
	tCtx.m_pLocalDocs = pLocalDocs;
	tCtx.m_iTotalDocs = iTotalDocs;
	tCtx.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;

	if ( !tCtx.SetupCalc ( tMeta, tMaxSorterSchema, m_tSchema, nullptr, dSorterSchemas ) )
		return false;

	// setup search terms
	RtQwordSetup_t tTermSetup ( tGuard );
	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;
	tTermSetup.m_iDynamicRowitems = tMaxSorterSchema.GetDynamicSize();
	if ( tQuery.m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = dTimerGuard.MiniTimerEngage ( tQuery.m_uMaxQueryMsec ); // max_query_time
	tTermSetup.m_pWarning = &tMeta.m_sWarning;
	tTermSetup.SetSegment ( -1 );
	tTermSetup.m_pCtx = &tCtx;
	tTermSetup.m_bHasWideFields = ( m_tSchema.GetFieldsCount()>32 );

	// setup prediction constrain
	CSphQueryStats tQueryStats;
	int64_t iNanoBudget = (int64_t)(tQuery.m_iMaxPredictedMsec) * 1000000; // from milliseconds to nanoseconds
	tQueryStats.m_pNanoBudget = &iNanoBudget;
	if ( tMeta.m_bHasPrediction )
		tTermSetup.m_pStats = &tQueryStats;

	// bind weights
	tCtx.BindWeights ( tQuery, m_tSchema, tMeta.m_sWarning );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (BYTE *)tQuery.m_sQuery.cstr();

	FieldFilterRefPtr_c pFieldFilter;
	if ( m_pFieldFilter && sModifiedQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// parse query
	SwitchProfile ( pProfiler, SPH_QSTATE_PARSE );

	XQQuery_t tParsed;
	// FIXME!!! provide segments list instead index to tTermSetup.m_pIndex

	const QueryParser_i * pQueryParser = tQuery.m_pQueryParser;
	assert ( pQueryParser );

	CSphScopedPayload tPayloads;

	// FIXME!!! add proper
	// - qcache invalidation after INSERT \ DELETE \ UPDATE and for plain index afte UPDATE #256
	// - qcache duplicates removal from killed document at segment #263
	tCtx.m_bSkipQCache = true;

	int iStackNeed = -1;
	bool bFullscan = pQueryParser->IsFullscan ( tQuery ); // use this
	// no need to create ranker, etc if there's no query
	if ( !bFullscan )
		iStackNeed = PrepareFTSearch (this, IsStarDict ( m_bKeywordDict ), m_bKeywordDict, m_iExpandKeywords, m_iExpansionLimit,
			(const char *) sModifiedQuery, m_tSettings, pQueryParser, tQuery, m_tSchema, &tGuard.m_dRamChunks,
			m_pTokenizer->Clone ( SPH_CLONE_QUERY ), pQueryTokenizer, pDict, tMeta, pProfiler,  &tPayloads, tParsed );

	// empty index, empty result. Must be AFTER PrepareFTSearch, since it prepares list of words
	if ( tGuard.m_dRamChunks.IsEmpty () )
	{
		for ( auto dSorter : dSorters )
			TransformSorterSchema ( dSorter, tGuard, dDiskBlobPools );

		tResult.m_pDocstore = m_tSchema.HasStoredFields () ? this : nullptr;
		tMeta.m_iQueryTime = 0;
		return true;
	}

	if ( !iStackNeed )
		return false;

	bool bResult;
	if ( bFullscan || pQueryParser->IsFullscan ( tParsed ) )
		bResult = DoFullScanQuery ( tGuard.m_dRamChunks, tMaxSorterSchema, tQuery, tArgs.m_iIndexWeight, m_iStride,
				tmMaxTimer, pProfiler, tCtx, dSorters, tMeta );
	else
		bResult = DoFullTextSearch ( tGuard.m_dRamChunks, tMaxSorterSchema, tQuery, m_sIndexName.cstr (),
				tArgs.m_iIndexWeight, iMatchPoolSize, iStackNeed, tTermSetup, pProfiler, tCtx, dSorters,
				tParsed, tMeta, dSorters.GetLength()==1 ? dSorters[0] : nullptr );

	if (!bResult)
		return false;

	MEMORY ( MEM_RT_RES_STRINGS );

	SwitchProfile ( pProfiler, SPH_QSTATE_DYNAMIC );

	// create new standalone schema for sorters (independent of any external indexes/pools/storages)
	// modify matches inside the sorters to work with the new schema
	for ( auto i : dSorters )
		TransformSorterSchema ( i, tGuard, dDiskBlobPools );

	if ( tMeta.m_bHasPrediction )
		tMeta.m_tStats.Add ( tQueryStats );

	tResult.m_pDocstore = m_tSchema.HasStoredFields() ? this : nullptr;
	tMeta.m_iQueryTime = int ( ( sphMicroTimer()-tmQueryStart )/1000 );
	tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;
	return true;
}

bool RtIndex_c::MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// FIXME! OPTIMIZE! implement common subtree cache here
	bool bResult = false;
	for ( int i=0; i<iQueries; ++i )
		if ( MultiQuery ( pResults[i], pQueries[i], { ppSorters+i, 1}, tArgs ) )
			bResult = true;
		else
			pResults[i].m_pMeta->m_iMultiplier = -1; // -1 means 'error'

	return bResult;
}


void RtIndex_c::AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, CSphDict * pDict, bool bGetStats, int iQpos, RtQword_t * pQueryWord, CSphVector <CSphKeywordInfo> & dKeywords, const SphChunkGuard_t & tGuard ) const
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
			RtQwordSetupSegment ( pQueryWord, tGuard.m_dRamChunks[iSeg], false, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings );
	}

	CSphKeywordInfo & tInfo = dKeywords.Add();
	tInfo.m_sTokenized = (const char *)sTokenized;
	tInfo.m_sNormalized = (const char*)sWord;
	tInfo.m_iDocs = bGetStats ? pQueryWord->m_iDocs : 0;
	tInfo.m_iHits = bGetStats ? pQueryWord->m_iHits : 0;
	tInfo.m_iQpos = iQpos;

	RemoveDictSpecials ( tInfo.m_sNormalized );
}


struct CSphRtQueryFilter : public ISphQueryFilter, public ISphNoncopyable
{
	const RtIndex_c *	m_pIndex;
	RtQword_t *			m_pQword;
	bool				m_bGetStats = false;
	const SphChunkGuard_t & m_tGuard;

	CSphRtQueryFilter ( const RtIndex_c * pIndex, RtQword_t * pQword, const SphChunkGuard_t & tGuard )
		: m_pIndex ( pIndex )
		, m_pQword ( pQword )
		, m_tGuard ( tGuard )
	{}

	void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) final
	{
		assert ( m_pIndex && m_pQword );
		m_pIndex->AddKeywordStats ( sWord, sTokenized, m_pDict, m_tFoldSettings.m_bStats, iQpos, m_pQword, dKeywords, m_tGuard );
	}
};

static void HashKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, SmallStringHash_T<CSphKeywordInfo> & hKeywords )
{
	for ( CSphKeywordInfo & tSrc : dKeywords )
	{
		CSphKeywordInfo & tDst = hKeywords.AddUnique ( tSrc.m_sNormalized );
		tDst.m_sTokenized = std::move ( tSrc.m_sTokenized );
		tDst.m_sNormalized = std::move ( tSrc.m_sNormalized );
		tDst.m_iQpos = tSrc.m_iQpos;
		tDst.m_iDocs += tSrc.m_iDocs;
		tDst.m_iHits += tSrc.m_iHits;
	}
}

void RtIndex_c::DoGetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError, const SphChunkGuard_t & tGuard ) const
{
	if ( !bFillOnly )
		dKeywords.Resize ( 0 );

	if ( ( bFillOnly && !dKeywords.GetLength() ) || ( !bFillOnly && ( !sQuery || !sQuery[0] ) ) )
		return;

	RtQword_t tQword;

	TokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_INDEX ) };
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually
	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( IsStarDict ( m_bKeywordDict ) )
	{
		if ( m_bKeywordDict )
			SetupStarDict ( pDict, pTokenizer );
		else
			pTokenizer->AddPlainChar ( '*' );
	}

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict, pTokenizer, false );

	CSphVector<BYTE> dFiltered;
	FieldFilterRefPtr_c pFieldFilter;
	const BYTE * sModifiedQuery = (const BYTE *)sQuery;
	if ( m_pFieldFilter && sQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// FIXME!!! missing bigram

	if ( !bFillOnly )
	{
		ExpansionContext_t tExpCtx;

		// query defined options
		tExpCtx.m_iExpansionLimit = tSettings.m_iExpansionLimit ? tSettings.m_iExpansionLimit : m_iExpansionLimit;
		bool bExpandWildcards = ( m_bKeywordDict && IsStarDict ( m_bKeywordDict ) && !tSettings.m_bFoldWildcards );

		CSphRtQueryFilter tAotFilter ( this, &tQword, tGuard );
		tAotFilter.m_pTokenizer = pTokenizer;
		tAotFilter.m_pDict = pDict;
		tAotFilter.m_pSettings = &m_tSettings;
		tAotFilter.m_tFoldSettings = tSettings;
		tAotFilter.m_tFoldSettings.m_bFoldWildcards = !bExpandWildcards;

		tExpCtx.m_pWordlist = this;
		tExpCtx.m_iMinPrefixLen = m_tSettings.GetMinPrefixLen ( m_bKeywordDict );
		tExpCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
		tExpCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
		tExpCtx.m_bMergeSingles = false;
		tExpCtx.m_pIndexData = &tGuard.m_dRamChunks;

		pTokenizer->SetBuffer ( sModifiedQuery, (int) strlen ( (const char*)sModifiedQuery ) );
		tAotFilter.GetKeywords ( dKeywords, tExpCtx );
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
					RtQwordSetupSegment ( &tQword, tGuard.m_dRamChunks[iSeg], false, m_bKeywordDict, m_iWordsCheckpoint, m_tSettings );

				tInfo.m_iDocs += tQword.m_iDocs;
				tInfo.m_iHits += tQword.m_iHits;
			}
		}
	}

	// get stats from disk chunks too
	if ( !tSettings.m_bStats )
		return;

	if ( bFillOnly )
	{
		for ( auto& pChunk : tGuard.m_dDiskChunks )
			pChunk->FillKeywords ( dKeywords );
	} else
	{
		// bigram and expanded might differs need to merge infos
		CSphVector<CSphKeywordInfo> dChunkKeywords;
		SmallStringHash_T<CSphKeywordInfo> hKeywords;
		for ( auto& pChunk: tGuard.m_dDiskChunks )
		{
			pChunk->GetKeywords ( dChunkKeywords, (const char*)sModifiedQuery, tSettings, pError );
			HashKeywords ( dChunkKeywords, hKeywords );
			dChunkKeywords.Resize ( 0 );
		}

		if ( hKeywords.GetLength() )
		{
			// merge keywords from RAM parts with disk keywords into hash
			HashKeywords ( dKeywords, hKeywords );
			dKeywords.Resize ( 0 );
			dKeywords.Reserve ( hKeywords.GetLength() );

			hKeywords.IterateStart();
			while ( hKeywords.IterateNext() )
			{
				const CSphKeywordInfo & tSrc = hKeywords.IterateGet();
				dKeywords.Add ( tSrc );
			}
			sphSort ( dKeywords.Begin(), dKeywords.GetLength(), bind ( &CSphKeywordInfo::m_iQpos ) );
		}
	}
}


bool RtIndex_c::GetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const
{
	DoGetKeywords ( dKeywords, sQuery, tSettings, false, pError, GetReaderChunks () );
	return true;
}


bool RtIndex_c::FillKeywords ( CSphVector<CSphKeywordInfo> & dKeywords ) const
{
	DoGetKeywords ( dKeywords, nullptr, GetKeywordsSettings_t(), true, nullptr, GetReaderChunks () );
	return true;
}


static RtSegment_t * UpdateFindSegment ( const SphChunkGuard_t & tGuard, CSphRowitem * & pRow, DocID_t uDocID )
{
	assert ( uDocID );

	for ( const auto& pChunk : tGuard.m_dRamChunks )
	{
		pRow = const_cast<CSphRowitem *> ( pChunk->FindAliveRow ( uDocID ) );
		if ( pRow )
			return pChunk;
	}

	return nullptr;
}


void RtIndex_c::Update_CollectRowPtrs ( UpdateContext_t & tCtx, const SphChunkGuard_t & tGuard )
{
	for ( int iUpd = tCtx.m_iFirst; iUpd<tCtx.m_iLast; iUpd++ )
	{
		DocID_t uDocid = tCtx.m_tUpd.m_dDocids[iUpd];

		CSphRowitem * pRow = nullptr;
		RtSegment_t * pSegment = UpdateFindSegment ( tGuard, pRow, uDocid );

		UpdatedRowData_t & tNew = tCtx.GetRowData(iUpd);
		tNew.m_pRow = pRow;
		tNew.m_pAttrPool = ( pSegment && pSegment->m_dRows.GetLength() ) ? &pSegment->m_dRows[0] : nullptr;
		tNew.m_pBlobPool = ( pSegment && pSegment->m_dBlobs.GetLength() ) ? &pSegment->m_dBlobs[0] : nullptr;
		tNew.m_pSegment = pSegment;
	}
}


bool RtIndex_c::Update_DiskChunks ( UpdateContext_t & tCtx, const SphChunkGuard_t & tGuard, int & iUpdated, CSphString & sError )
{
	for ( int iUpd=tCtx.m_iFirst; iUpd<tCtx.m_iLast; iUpd++ )
	{
		if ( tCtx.GetRowData(iUpd).m_bUpdated )
			continue;

		for ( int iChunk = tGuard.m_dDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
		{
			// run just this update
			// FIXME! might be inefficient in case of big batches (redundant allocs in disk update)
			bool bCritical = false;
			CSphString sWarning;
			int iRes = const_cast<CSphIndex *>( tGuard.m_dDiskChunks[iChunk] )->UpdateAttributes ( tCtx.m_tUpd, iUpd, bCritical, sError, sWarning );

			// FIXME! need to handle critical failures here (chunk is unusable at this point)
			assert ( !bCritical );

			// FIXME! maybe emit a warning to client as well?
			if ( iRes<0 )
				return false;

			// update stats
			iUpdated += iRes;
			m_uDiskAttrStatus |= tGuard.m_dDiskChunks[iChunk]->GetAttributeStatus();

			// we only need to update the most fresh chunk
			if ( iRes>0 )
				break;
		}
	}

	return true;
}


bool RtIndex_c::Update_WriteBlobRow ( UpdateContext_t & tCtx, int iUpd, CSphRowitem * pDocinfo, const BYTE * pBlob, int iLength, int nBlobAttrs, bool & bCritical, CSphString & sError )
{
	IndexSegment_c * pSegment = tCtx.GetRowData(iUpd).m_pSegment;
	assert ( pSegment );

	bCritical = false;

	CSphTightVector<BYTE> & dBlobPool = ((RtSegment_t*)pSegment)->m_dBlobs;

	BYTE * pExistingBlob = dBlobPool.Begin() + sphGetBlobRowOffset(pDocinfo);
	DWORD uExistingBlobLen = sphGetBlobTotalLen ( pExistingBlob, nBlobAttrs );

	// overwrite old record
	if ( (DWORD)iLength<=uExistingBlobLen )
	{
		memcpy ( pExistingBlob, pBlob, iLength );
		return true;
	}

	int iPoolSize = dBlobPool.GetLength();
	dBlobPool.Resize ( iPoolSize+iLength );
	memcpy ( dBlobPool.Begin()+iPoolSize, pBlob, iLength );

	sphSetBlobRowOffset ( pDocinfo, iPoolSize );

	// update blob pool ptrs since they could have changed after the resize
	for ( auto & i : tCtx.m_dUpdatedRows )
		if ( i.m_pSegment==pSegment )
			i.m_pBlobPool = &dBlobPool[0];

	return true;
}


// FIXME! might be inconsistent in case disk chunk update fails
int RtIndex_c::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, bool & bCritical, CSphString & sError, CSphString & sWarning )
{
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRowOffset.GetLength() );
	DWORD uRows = tUpd.m_dDocids.GetLength();

	if ( ( !m_dRamChunks.GetLength() && !m_dDiskChunks.GetLength() ) || !uRows )
		return 0;

	UpdateContext_t tCtx ( tUpd, m_tSchema, nullptr, ( iIndex<0 ) ? 0 : iIndex, ( iIndex<0 ) ? uRows : iIndex+1 );

	// FIXME!!! grab Writer lock to prevent segments retirement during commit(merge)
	SphChunkGuard_t tGuard = GetReaderChunks ();

	Update_CollectRowPtrs ( tCtx, tGuard );
	if ( !Update_FixupData ( tCtx, sError ) )
		return -1;

	if ( tUpd.m_bStrict )
		if ( !Update_InplaceJson ( tCtx, sError, true ) )
			return -1;

	// second pass
	tCtx.m_iJsonWarnings = 0;
	Update_InplaceJson ( tCtx, sError, false );

	if ( !Update_Blobs ( tCtx, bCritical, sError ) )
		return -1;

	Update_Plain ( tCtx );

	int iUpdated = 0;
	for ( const auto & i : tCtx.m_dUpdatedRows )
		if ( i.m_bUpdated )
			iUpdated++;

	if ( !Update_DiskChunks ( tCtx, tGuard, iUpdated, sError ) )
		sphWarn ( "INTERNAL ERROR: index %s update failure: %s", m_sIndexName.cstr(), sError.cstr() );

	// bump the counter, binlog the update!
	assert ( iIndex<0 );
	g_pBinlog->BinlogUpdateAttributes ( &m_iTID, m_sIndexName.cstr(), tUpd );

	if ( !Update_HandleJsonWarnings ( tCtx, iUpdated, sWarning, sError ) )
		return -1;

	// all done
	return iUpdated;
}


bool RtIndex_c::SaveAttributes ( CSphString & sError ) const
{
	if ( !m_dDiskChunks.GetLength() || m_bSaveDisabled )
		return true;

	DWORD uStatus = m_uDiskAttrStatus;
	bool bAllSaved = true;

	SphChunkGuard_t tGuard = GetReaderChunks ();

	ARRAY_FOREACH ( i, tGuard.m_dDiskChunks )
		bAllSaved &= tGuard.m_dDiskChunks[i]->SaveAttributes ( sError );

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


template<typename T>
class WriteWrapper_Mem_c : public WriteWrapper_c
{
public:
	WriteWrapper_Mem_c ( CSphTightVector<T> & tBuffer )
		: m_tBuffer ( tBuffer )
	{}

	void PutBytes ( const BYTE * pData, int iSize ) override
	{
		assert ( iSize % sizeof(T) == 0 );
		T * pNew = m_tBuffer.AddN ( iSize/sizeof(T) );
		memcpy ( pNew, pData, iSize );
	}

	SphOffset_t	GetPos() const override
	{
		return m_tBuffer.GetLength()*sizeof(T);
	}

	bool IsError() const override
	{
		return false;
	}

protected:
	CSphTightVector<T> & m_tBuffer;
};


bool RtIndex_c::AddRemoveAttribute ( bool bAdd, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError )
{
	if ( m_dDiskChunks.GetLength() && !m_tSchema.GetAttrsCount() )
	{
		sError = "index must already have attributes";
		return false;
	}

	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop ); // got write-locked at daemon

	CSphSchema tOldSchema = m_tSchema;
	CSphSchema tNewSchema = m_tSchema;
	if ( !Alter_AddRemoveFromSchema ( tNewSchema, sAttrName, eAttrType, bAdd, sError ) )
		return false;

	m_tSchema = tNewSchema;

	int iOldStride = m_iStride;
	m_iStride = m_tSchema.GetRowSize();

	CSphFixedVector<int> dChunkNames = GetChunkNames ( m_dDiskChunks );

	// modify the in-memory data of disk chunks
	// fixme: we can't rollback in-memory changes, so we just show errors here for now
	ARRAY_FOREACH ( iDiskChunk, m_dDiskChunks )
		if ( !m_dDiskChunks[iDiskChunk]->AddRemoveAttribute ( bAdd, sAttrName, eAttrType, sError ) )
			sphWarning ( "%s attribute to %s.%d: %s", bAdd ? "adding" : "removing", m_sPath.cstr(), dChunkNames[iDiskChunk], sError.cstr() );

	bool bHadBlobs = false;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		bHadBlobs |= sphIsBlobAttr ( m_tSchema.GetAttr(i).m_eAttrType );

	bool bHaveBlobs = false;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		bHaveBlobs |= sphIsBlobAttr ( tNewSchema.GetAttr(i).m_eAttrType );

	bool bBlob = sphIsBlobAttr ( eAttrType );
	bool bBlobsModified = bBlob && ( bAdd || bHaveBlobs==bHadBlobs );

	// now modify the ramchunk
	for ( RtSegment_t * pSeg : m_dRamChunks )
	{
		assert ( pSeg );

		const CSphRowitem * pDocinfo = pSeg->m_dRows.Begin();
		const CSphRowitem * pDocinfoMax = pDocinfo+pSeg->m_dRows.GetLength();

		CSphTightVector<CSphRowitem> dSPA;
		dSPA.Reserve ( pSeg->m_dRows.GetLength() / iOldStride * m_iStride );

		CSphTightVector<BYTE> dSPB;
		dSPB.Reserve ( pSeg->m_dBlobs.GetLength() / 2 );		// reserve half of our current blobs, just in case

		WriteWrapper_Mem_c<CSphRowitem> tSPAWriteWrapper(dSPA);
		WriteWrapper_Mem_c<BYTE> tSPBWriteWrapper(dSPB);

		if ( !Alter_AddRemoveAttr ( tOldSchema, tNewSchema, pDocinfo, pDocinfoMax, pSeg->m_dBlobs.Begin(), tSPAWriteWrapper, tSPBWriteWrapper, bAdd, sAttrName ) )
			sphWarning ( "%s attribute to %s: %s", bAdd ? "adding" : "removing", m_sPath.cstr(), sError.cstr() );

		pSeg->m_dRows.SwapData(dSPA);
		if ( bBlob || bBlobsModified )
			pSeg->m_dBlobs.SwapData(dSPB);

		pSeg->UpdateUsedRam();
	}

	// fixme: we can't rollback at this point
	Verify ( SaveRamChunk ( m_dRamChunks ) );

	SaveMeta ( m_iTID );

	// fixme: notify that it was ALTER that caused the flush
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// MAGIC CONVERSIONS
//////////////////////////////////////////////////////////////////////////

bool RtIndex_c::AttachDiskIndex ( CSphIndex * pIndex, bool bTruncate, bool & bFatal, CSphString & sError )
{
	bFatal = false;
	bool bEmptyRT = ( ( !m_dRamChunks.GetLength() && !m_dDiskChunks.GetLength() ) || bTruncate );

	// safeguards
	// we do not support some of the disk index features in RT just yet
#define LOC_ERROR(_arg) { sError = _arg; return false; }
	const CSphIndexSettings & tSettings = pIndex->GetSettings();
	if ( tSettings.m_iStopwordStep!=1 )
		LOC_ERROR ( "ATTACH currently requires stopword_step=1 in disk index (RT-side support not implemented yet)" );
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

	if ( bTruncate && !Truncate ( sError ) )
		return false;

	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop ); // got write-locked at daemon

	int iTotalKilled = 0;
	if ( !bEmptyRT )
	{
		CSphFixedVector<SphAttr_t> dIndexDocList = pIndex->BuildDocList();
		if ( TlsMsg::HasErr () )
		{
			sError.SetSprintf ( "ATTACH failed, %s", TlsMsg::szError () );
			return false;
		}

		ChunkStats_t tStats ( m_tStats, m_dFieldLensRam );
		{	// scope for SphChunkGuard_t. Fixme! Check if it is necessary in current context
			SphChunkGuard_t tGuard = GetReaderChunks ();
			if ( !SaveDiskChunk ( m_iTID, tGuard, tStats, true, nullptr ) )
			{
				bFatal = true;
				return false;
			}
		}

		// docs ids could be not asc
		// should sort and remove duplicates
		int iDocsCount = sphUniq ( dIndexDocList.Begin(), dIndexDocList.GetLength() );
		VecTraits_T<SphAttr_t> dDocsSorted ( dIndexDocList.Begin(), iDocsCount );

		if ( m_bOptimizing )
			m_dKillsWhileOptimizing.Append ( dDocsSorted );

		for ( const auto & pChunk : m_dDiskChunks )
			iTotalKilled += pChunk->KillMulti ( dDocsSorted );

	}

//	CSphFixedVector<int> dChunkNames = GetIndexNames ( m_dDiskChunks, true );

	// rename that source index to our last chunk
	int iChunk = GetNextChunkName ();
	CSphString sChunk;
	sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunk );
	if ( !pIndex->Rename ( sChunk.cstr() ) )
	{
		sError.SetSprintf ( "ATTACH failed, %s", pIndex->GetLastError().cstr() );
		return false;
	}

	// copy schema from new index
	SetSchema ( pIndex->GetMatchSchema() );
	m_tStats.m_iTotalBytes += pIndex->GetStats().m_iTotalBytes;
	m_tStats.m_iTotalDocuments += pIndex->GetStats().m_iTotalDocuments-iTotalKilled;

	// copy tokenizer, dict etc settings from new index
	m_tSettings = pIndex->GetSettings();
	m_tSettings.m_dBigramWords.Reset();

	m_pTokenizer = pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX );
	m_pDict = pIndex->GetDictionary()->Clone ();
	PostSetup();
	CSphString sName;
	sName.SetSprintf ( "%s_%d", m_sIndexName.cstr(), m_dDiskChunks.GetLength() );
	pIndex->SetName ( sName.cstr() );
	pIndex->SetBinlog ( false );
	pIndex->m_iChunk = iChunk;

	// FIXME? what about copying m_TID etc?

	// recreate disk chunk list, resave header file
	m_dDiskChunks.Add ( pIndex );
	SaveMeta ( m_iTID );

	// FIXME? do something about binlog too?
	// g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// all done, reset cache
	QcacheDeleteIndex ( GetIndexId() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// TRUNCATE
//////////////////////////////////////////////////////////////////////////

bool RtIndex_c::Truncate ( CSphString & )
{
	// TRUNCATE needs an exclusive lock, should be write-locked at daemon, conflicts only with optimize
	SphOptimizeGuard_t tStopOptimize ( m_tOptimizingLock, m_bOptimizeStop );

	// update and save meta
	// indicate 0 disk chunks, we are about to kill them anyway
	// current TID will be saved, so replay will properly skip preceding txns
	m_tStats.Reset();
	SaveMeta ( m_iTID, false ); // false means 'save without disk chunks'

	// allow binlog to unlink now-redundant data files
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	// kill RAM chunk file
	CSphString sFile;
	sFile.SetSprintf ( "%s.ram", m_sPath.cstr() );
	if ( ::unlink ( sFile.cstr() ) )
		if ( errno!=ENOENT )
			sphWarning ( "rt: truncate failed to unlink %s: %s", sFile.cstr(), strerrorm(errno) );

	// kill all disk chunks files
	for ( auto & i : m_dDiskChunks )
	{
		StrVec_t v;
		const char * sChunkFilename = i->GetFilename();
		i->Dealloc();
		sphSplit ( v, sChunkFilename, "." ); // split something like "rt.1"
		const char * sChunkNumber = v.Last().cstr();
		sFile.SetSprintf ( "%s.%s", m_sPath.cstr(), sChunkNumber );
		sphUnlinkIndex ( sFile.cstr(), false );
	}

	// kill in-memory data, reset stats
	ARRAY_FOREACH ( i, m_dDiskChunks )
		SafeDelete ( m_dDiskChunks[i] );
	m_dDiskChunks.Reset();

	m_dRamChunks.Reset();

	// reset cache
	QcacheDeleteIndex ( GetIndexId() );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// OPTIMIZE
//////////////////////////////////////////////////////////////////////////

static int64_t GetEffectiveSize (const CSphIndexStatus& tStatus, int64_t iTotalDocs )
{
	if ( !tStatus.m_iDead ) // no docs killed
		return tStatus.m_iDiskUse;

	if ( tStatus.m_iDead==iTotalDocs ) // all docs killed
		return 0;

	// has killed docs: use naive formulae, sumple part of used disk proportional to partial of alive docs
	auto fPart = double ( tStatus.m_iDead ) / double ( iTotalDocs );
	return int64_t ( tStatus.m_iDiskUse * ( 1.0f-fPart ) );
}

static int64_t GetChunkSize ( const CSphVector<CSphIndex*> & dDiskChunks, int iIndex )
{
	if (iIndex<0)
		return 0;
	CSphIndexStatus tDisk;
	dDiskChunks[iIndex]->GetStatus(&tDisk);
	return GetEffectiveSize ( tDisk, dDiskChunks[iIndex]->GetStats ().m_iTotalDocuments );
}


static std::pair<int, int64_t> GetNextSmallestChunk ( const CSphVector<CSphIndex*> & dDiskChunks, int iIndex )
{
	int iRes = -1;
	int64_t iLastSize = INT64_MAX;
	ARRAY_FOREACH ( i, dDiskChunks )
	{
		int64_t iSize = GetChunkSize ( dDiskChunks, i );
		if ( iSize<iLastSize && iIndex!=i )
		{
			iLastSize = iSize;
			iRes = i;
		}
	}
	return { iRes, iLastSize };
}

void RtIndex_c::DropDiskChunk( int iChunk )
{
	sphLogDebug( "rt optimize: index %s: drop disk chunk %d",  m_sIndexName.cstr(), iChunk );

	CSphString sDeleted;

	{	ScopedMutex_t _ ( m_tWriting );

		Verify ( m_tChunkLock.WriteLock () );
		auto pToDelete = m_dDiskChunks[iChunk];
		sDeleted = pToDelete->GetFilename ();
		m_dDiskChunks.Remove ( iChunk );
		SaveMeta ( m_iTID );
		Verify ( m_tChunkLock.Unlock () );
		ScWL_t ReaderWlock ( m_tReading );
		SafeDelete ( pToDelete );
	}

	// we might remove index files
	sphUnlinkIndex ( sDeleted.cstr(), true );
}

void RtIndex_c::CommonMerge( Selector_t&& fnSelector )
{
	int64_t tmStart = sphMicroTimer();
	sphLogDebug( "rt optimize: index %s: optimization started",  m_sIndexName.cstr() );

	ScopedMutex_t tOptimizing ( m_tOptimizingLock );
	m_bOptimizing = true;

	CreateFilenameBuilder_fn fnCreateFilenameBuilder = GetIndexFilenameBuilder();
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( fnCreateFilenameBuilder ? fnCreateFilenameBuilder ( m_sIndexName.cstr() ) : nullptr );

	int iChunks = 0;
	CSphString sError;

	int iA = -1;
	int iB = -1;

	while ( fnSelector ( &iA, &iB ) && !sphInterrupted () && !m_bOptimizeStop && !m_bSaveDisabled )
	{
		const CSphIndex * pOldest = nullptr;
		const CSphIndex * pOlder = nullptr;

		// got zero chunk (iA), just remove it quickly
		if ( iB<0 )
		{
			DropDiskChunk ( iA );
			continue;
		}

		{ // m_tChunkLock scoped Readlock
			CSphScopedRLock RChunkLock { m_tChunkLock };
			// merge 'older'(pSrc) to 'oldest'(pDst) and get 'merged' that names like 'oldest'+.tmp
			// however 'merged' got placed at 'older' position and 'merged' renamed to 'older' name
			pOldest = m_dDiskChunks[iA];
			pOlder = m_dDiskChunks[iB];
		} // m_tChunkLock scoped Readlock

		sphLogDebug ( "common merge - merging %d (%d kb) with %d (%d kb)",
				iA, (int)(GetChunkSize ( m_dDiskChunks, iA )/1024),
				iB, (int)(GetChunkSize ( m_dDiskChunks, iB )/1024) );

		CSphString sOlder, sOldest, sRename, sMerged;
		sOlder.SetSprintf ( "%s", pOlder->GetFilename() );
		sOldest.SetSprintf ( "%s", pOldest->GetFilename() );
		sRename.SetSprintf ( "%s.old", pOlder->GetFilename() );
		sMerged.SetSprintf ( "%s.tmp", pOldest->GetFilename() );

		// check forced exit after long operation
		if ( m_bOptimizeStop || m_bSaveDisabled || sphInterrupted ())
			break;

		// need klist for merged chunk only after we got disk chunks and going to merge them
		Verify ( m_tWriting.Lock() );
		m_dKillsWhileOptimizing.Reset();
		Verify ( m_tWriting.Unlock() );

		// merge data to disk ( data is constant during that phase )
		CSphIndexProgress tProgress;
		bool bMerged = sphMerge ( pOldest, pOlder, sError, tProgress, &m_bOptimizeStop, true );
		if ( !bMerged )
		{
			sphWarning ( "rt optimize: index %s: failed to merge %s to %s (error %s)",
				m_sIndexName.cstr(), sOlder.cstr(), sOldest.cstr(), sError.cstr() );
			break;
		}

		// check forced exit after long operation
		if ( m_bOptimizeStop || m_bSaveDisabled || sphInterrupted ())
			break;

		CSphScopedPtr<CSphIndex> pMerged ( PreallocDiskChunk ( sMerged.cstr(), pOlder->m_iChunk, pFilenameBuilder.Ptr(), sError, pOlder->GetName() ) );
		if ( !pMerged.Ptr() )
		{
			sphWarning ( "rt optimize: index %s: failed to load merged chunk (error %s)",
				m_sIndexName.cstr(), sError.cstr() );
			break;
		}
		// check forced exit after long operation
		if ( m_bOptimizeStop || m_bSaveDisabled || sphInterrupted ())
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
				sphWarning ( "rt optimize: index %s: old to cur rename failed (error %s)",
					m_sIndexName.cstr(), pOlder->GetLastError().cstr() );
			break;
		}

		if ( m_bOptimizeStop || sphInterrupted ()) // protection
			break;

		// merged replaces recent chunk
		// oldest chunk got deleted

		// Writing lock - to wipe out writers
		// Chunk wlock - to lock chunks as we are going to modify the chunk vector
		// order same as GetReaderChunks and SaveDiskChunk to prevent deadlock

		Verify ( m_tWriting.Lock() );
		Verify ( m_tChunkLock.WriteLock() );

		// apply killlists that were collected while we were merging segments
		m_dKillsWhileOptimizing.Uniq();
		int iKilled = pMerged->KillMulti ( m_dKillsWhileOptimizing );
		m_dKillsWhileOptimizing.Reset();

		sphLogDebug ( "optimized a=%s, b=%s, new=%s, killed=%d",
				pOldest->GetFilename(), pOlder->GetFilename(), pMerged->GetFilename(), iKilled );

		m_dDiskChunks[iB] = pMerged.LeakPtr();
		m_dDiskChunks.Remove ( iA );
		SaveMeta ( m_iTID );

		Verify ( m_tChunkLock.Unlock() );
		Verify ( m_tWriting.Unlock() );

		++iChunks;

		if ( m_bOptimizeStop || sphInterrupted () )
		{
			sphWarning ( "rt optimize: index %s: forced to shutdown, remove old index files manually '%s', '%s'",
				m_sIndexName.cstr(), sRename.cstr(), sOldest.cstr() );
			break;
		}

		// exclusive reader (to make sure that disk chunks not used any more) and writer lock here
		// write lock goes first as with commit
		Verify ( m_tWriting.Lock() );
		Verify ( m_tReading.WriteLock() );

		SafeDelete ( pOlder );
		SafeDelete ( pOldest );

		Verify ( m_tReading.Unlock() );
		Verify ( m_tWriting.Unlock() );

		// we might remove old index files
		sphUnlinkIndex ( sRename.cstr(), true );
		sphUnlinkIndex ( sOldest.cstr(), true );
		// FIXEME: wipe out 'merged' index files in case of error
	}

	m_bOptimizing = false;
	int64_t tmPass = sphMicroTimer() - tmStart;

	if ( sphInterrupted () )
	{
		sphWarning ( "rt: index %s: optimization terminated chunk(s) %d ( left %d ) in %d.%03d sec",
			m_sIndexName.cstr(), iChunks, m_dDiskChunks.GetLength(), (int)(tmPass/1000000), (int)((tmPass/1000)%1000) );
	} else
	{
		sphInfo ( "rt: index %s: optimized %s chunk(s) %d ( left %d ) in %d.%03d sec",
			m_sIndexName.cstr(), g_bProgressiveMerge ? "progressive" : "regular", iChunks, m_dDiskChunks.GetLength(), (int)(tmPass/1000000), (int)((tmPass/1000)%1000) );
	}
}

void RtIndex_c::Optimize( int iCutoff, int iFrom, int iTo ) REQUIRES ( m_tChunkLock )
{
	int iChunks = m_dDiskChunks.GetLength ();

	// manual case: iFrom and iTo provided from outside, iCutoff is not in game
	if ( iFrom>=0 && iTo>=0 )
	{
		CommonMerge( [&iFrom,&iTo,iChunks] (int* piA, int* piB) -> bool
		{
			if (iFrom<0||iFrom>=iChunks||iTo<0||iTo>=iChunks)
				return false;
			if (iFrom==iTo) // fool protect
				return false;
			assert ( piA );
			assert ( piB );
			*piA = iFrom;
			*piB = iTo;
			iFrom=iTo=-1;
			return true;
		});
		return;
	}

	if ( g_bProgressiveMerge )
	{
		// How does this work:
		// In order to minimize IO operations we merge chunks in order from the smallest to the largest to build a progression
		// the timeline is: [older chunks], ..., A, A+1, ..., B, ..., [younger chunks]
		// this also needs meta v.12 (chunk list with possible skips, instead of a base chunk + length as in meta v.11)
		if ( !iCutoff )
			iCutoff = sphCpuThreadsCount () * 2;

		CommonMerge ( [this, iCutoff] (int* piA, int* piB) REQUIRES ( m_tChunkLock ) -> bool
		{
			if ( m_dDiskChunks.IsEmpty() )
				return false;

			// merge 'smallest' to 'smaller' and get 'merged' that names like 'A'+.tmp
			// however 'merged' got placed at 'B' position and 'merged' renamed to 'B' name

			auto chA = GetNextSmallestChunk ( m_dDiskChunks, -1 );
			if ( !chA.second ) // empty chunk - just remove
			{
				assert ( piA );
				assert ( piB );
				*piA = chA.first;
				*piB = -1;
				return true;
			}

			// stop on cutoff for non-empty chunks
			if ( m_dDiskChunks.GetLength ()<=iCutoff )
				return false;

			auto chB = GetNextSmallestChunk ( m_dDiskChunks, chA.first );

			if ( chA.first<0 || chB.first<0 )
			{
				sphWarning ( "Couldn't find smallest chunk" );
				return false;
			}

			// we need to make sure that A is the oldest one
			// indexes go from oldest to newest so A must go before B (A is always older than B)
			// this is not required by bitmap killlists, but by some other stuff (like ALTER RECONFIGURE)
			if ( chA.first>chB.first )
				Swap ( chB, chA );

			assert ( piA );
			assert ( piB );
			*piA = chA.first;
			*piB = chB.first;
			return true;
		});
		return;
	}

	CommonMerge ( [this] (int* piA, int* piB) REQUIRES ( m_tChunkLock ) -> bool
	{
		if ( m_dDiskChunks.GetLength ()<2 )
			return false;

		assert ( piA );
		assert ( piB );
		*piA = 0;
		*piB = 1;
		return true;
	});
}


//////////////////////////////////////////////////////////////////////////
// STATUS
//////////////////////////////////////////////////////////////////////////

void RtIndex_c::GetStatus ( CSphIndexStatus * pRes ) const
{
	assert ( pRes );
	if ( !pRes )
		return;

	SphChunkGuard_t tGuard = GetReaderChunks();

	int64_t iUsedRam = SegmentsGetUsedRam ( tGuard.m_dRamChunks );
	pRes->m_iDead = SegmentsGetDeadRows ( tGuard.m_dRamChunks );

	pRes->m_iRamChunkSize = iUsedRam + tGuard.m_dRamChunks.GetLength()*int(sizeof(RtSegment_t));
	pRes->m_iRamUse = sizeof( RtIndex_c ) + pRes->m_iRamChunkSize;
	pRes->m_iRamRetired = m_iRamChunksAllocatedRAM.load(std::memory_order_relaxed) - iUsedRam;

	pRes->m_iMemLimit = m_iSoftRamLimit;

	CSphString sError;
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sFiles[] = { ".meta", ".ram" };
	for ( const char * sName : sFiles )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", m_sFilename.cstr(), sName );
		CSphAutofile fdRT ( sFile, SPH_O_READ, sError );
		int64_t iFileSize = fdRT.GetSize();
		if ( iFileSize>0 )
			pRes->m_iDiskUse += iFileSize; // that uses disk, but not occupies
	}
	CSphIndexStatus tDisk;
	ARRAY_FOREACH ( i, tGuard.m_dDiskChunks )
	{
		tGuard.m_dDiskChunks[i]->GetStatus ( &tDisk );
		pRes->m_iRamUse += tDisk.m_iRamUse;
		pRes->m_iDiskUse += tDisk.m_iDiskUse;
		pRes->m_iMapped += tDisk.m_iMapped;
		pRes->m_iMappedResident += tDisk.m_iMappedResident;
		pRes->m_iMappedDocs += tDisk.m_iMappedDocs;
		pRes->m_iMappedResidentDocs += tDisk.m_iMappedResidentDocs;
		pRes->m_iMappedHits += tDisk.m_iMappedHits;
		pRes->m_iMappedResidentHits += tDisk.m_iMappedResidentHits;
		pRes->m_iDead += tDisk.m_iDead;
	}

	pRes->m_iNumRamChunks = tGuard.m_dRamChunks.GetLength();
	pRes->m_iNumChunks = tGuard.m_dDiskChunks.GetLength();

	pRes->m_iTID = m_iTID;
	pRes->m_iSavedTID = m_iSavedTID;
//	sphWarning ( "Chunks: %d, RAM: %d, DISK: %d", pRes->m_iNumChunks, (int) pRes->m_iRamUse, (int) pRes->m_iDiskUse );
}

//////////////////////////////////////////////////////////////////////////
// RECONFIGURE
//////////////////////////////////////////////////////////////////////////

bool CreateReconfigure ( const CSphString & sIndexName, bool bIsStarDict, const ISphFieldFilter * pFieldFilter,
	const CSphIndexSettings & tIndexSettings, uint64_t uTokHash, uint64_t uDictHash, int iMaxCodepointLength, int64_t iMemLimit,
	bool bSame, CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError )
{
	CreateFilenameBuilder_fn fnCreateFilenameBuilder = GetIndexFilenameBuilder();
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( fnCreateFilenameBuilder ? fnCreateFilenameBuilder ( sIndexName.cstr() ) : nullptr );

	// FIXME!!! check missed embedded files
	TokenizerRefPtr_c pTokenizer { ISphTokenizer::Create ( tSettings.m_tTokenizer, nullptr, pFilenameBuilder.Ptr(), dWarnings, sError ) };
	if ( !pTokenizer )
	{
		sError.SetSprintf ( "'%s' failed to create tokenizer, error '%s'", sIndexName.cstr(), sError.cstr() );
		return true;
	}

	// dict setup second
	DictRefPtr_c tDict { sphCreateDictionaryCRC ( tSettings.m_tDict, NULL, pTokenizer, sIndexName.cstr(), false, tIndexSettings.m_iSkiplistBlockSize, pFilenameBuilder.Ptr(), sError ) };
	if ( !tDict )
	{
		sError.SetSprintf ( "'%s' failed to create dictionary, error '%s'", sIndexName.cstr(), sError.cstr() );
		return true;
	}

	// multiforms right after dict
	pTokenizer = ISphTokenizer::CreateMultiformFilter ( pTokenizer, tDict->GetMultiWordforms() );

	// bigram filter
	if ( tSettings.m_tIndex.m_eBigramIndex!=SPH_BIGRAM_NONE && tSettings.m_tIndex.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		pTokenizer->SetBuffer ( (BYTE*)tSettings.m_tIndex.m_sBigramWords.cstr(), tSettings.m_tIndex.m_sBigramWords.Length() );

		BYTE * pTok = NULL;
		while ( ( pTok = pTokenizer->GetToken() )!=NULL )
			tSettings.m_tIndex.m_dBigramWords.Add() = (const char*)pTok;

		tSettings.m_tIndex.m_dBigramWords.Sort();
	}

	bool bNeedExact = ( tDict->HasMorphology() || tDict->GetWordformsFileInfos().GetLength() );
	if ( tSettings.m_tIndex.m_bIndexExactWords && !bNeedExact )
		tSettings.m_tIndex.m_bIndexExactWords = false;

	if ( tDict->GetSettings().m_bWordDict && tDict->HasMorphology() && bIsStarDict && !tSettings.m_tIndex.m_bIndexExactWords )
		tSettings.m_tIndex.m_bIndexExactWords = true;

	// field filter
	FieldFilterRefPtr_c tFieldFilter;

	// re filter
	bool bReFilterSame = true;
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( pFieldFilter )
		pFieldFilter->GetSettings ( tFieldFilterSettings );
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
		if ( !tFieldFilter )
		{
			sError.SetSprintf ( "'%s' failed to create field filter, error '%s'", sIndexName.cstr (), sError.cstr () );
			return true;
		}
	}

	// icu filter
	bool bIcuSame = ( tIndexSettings.m_ePreprocessor==tSettings.m_tIndex.m_ePreprocessor );
	if ( !bIcuSame )
	{
		if ( !sphSpawnFilterICU ( tFieldFilter, tSettings.m_tIndex, tSettings.m_tTokenizer, sIndexName.cstr (), sError ) )
		{
			sError.SetSprintf ( "'%s' failed to create field filter, error '%s'", sIndexName.cstr (), sError.cstr () );
			return true;
		}
	}

	// compare options
	if ( !bSame || uTokHash!=pTokenizer->GetSettingsFNV() || uDictHash!=tDict->GetSettingsFNV() ||
		iMaxCodepointLength!=pTokenizer->GetMaxCodepointLength() || sphGetSettingsFNV ( tIndexSettings )!=sphGetSettingsFNV ( tSettings.m_tIndex ) ||
		!bReFilterSame || !bIcuSame || iMemLimit!=tSettings.m_iMemLimit )
	{
		tSetup.m_pTokenizer = pTokenizer.Leak();
		tSetup.m_pDict = tDict.Leak();
		tSetup.m_tIndex = tSettings.m_tIndex;
		tSetup.m_pFieldFilter = tFieldFilter.Leak();
		tSetup.m_iMemLimit = tSettings.m_iMemLimit;
		return false;
	}

	return true;
}


bool RtIndex_c::IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const
{
	bool bSame = true;
	if ( tSettings.m_bChangeSchema && m_uSchemaHash!=SchemaFNV ( tSettings.m_tSchema ) )
	{
		tSetup.m_tSchema = tSettings.m_tSchema;
		tSetup.m_bChangeSchema = true;
		bSame = false;
	}

	return CreateReconfigure ( m_sIndexName, IsStarDict ( m_bKeywordDict ), m_pFieldFilter, m_tSettings, m_pTokenizer->GetSettingsFNV(), m_pDict->GetSettingsFNV(), m_pTokenizer->GetMaxCodepointLength(),
		GetMemLimit(), bSame, tSettings, tSetup, dWarnings, sError );
}

bool RtIndex_c::Reconfigure ( CSphReconfigureSetup & tSetup )
{
	if ( !ForceDiskChunk() )
		return false;

	if ( tSetup.m_bChangeSchema )
		SetSchema ( tSetup.m_tSchema );

	m_iSoftRamLimit = tSetup.m_iMemLimit;

	Setup ( tSetup.m_tIndex );
	SetTokenizer ( tSetup.m_pTokenizer );
	SetDictionary ( tSetup.m_pDict );
	SetFieldFilter ( tSetup.m_pFieldFilter );

	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();
	SetupQueryTokenizer();

	// FIXME!!! handle error
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	TokenizerRefPtr_c pIndexing { ISphTokenizer::CreateBigramFilter ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError ) };
	if ( pIndexing )
		m_pTokenizerIndexing = pIndexing;

	return true;
}


int	RtIndex_c::Kill ( DocID_t tDocID )
{
	assert ( 0 && "No external kills for RT");
	return 0;
}


int RtIndex_c::KillMulti ( const VecTraits_T<DocID_t> & dKlist )
{
	assert ( 0 && "No external kills for RT");
	return 0;
}

bool RtIndex_c::IsAlive ( DocID_t tDocID ) const
{
	assert ( 0 && "No external kills for RT");
	return false;
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

	int iMinPrefixLen = tSettings.RawMinPrefixLen();

	uHash = sphFNV64 ( &tSettings.m_eHitFormat, sizeof(tSettings.m_eHitFormat), uHash );
	uHash = sphFNV64 ( tSettings.m_sHtmlIndexAttrs.cstr(), tSettings.m_sHtmlIndexAttrs.Length(), uHash );
	uHash = sphFNV64 ( tSettings.m_sHtmlRemoveElements.cstr(), tSettings.m_sHtmlRemoveElements.Length(), uHash );
	uHash = sphFNV64 ( tSettings.m_sZones.cstr(), tSettings.m_sZones.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_eHitless, sizeof(tSettings.m_eHitless), uHash );
	uHash = sphFNV64 ( tSettings.m_sHitlessFiles.cstr(), tSettings.m_sHitlessFiles.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_eBigramIndex, sizeof(tSettings.m_eBigramIndex), uHash );
	uHash = sphFNV64 ( tSettings.m_sBigramWords.cstr(), tSettings.m_sBigramWords.Length(), uHash );
	uHash = sphFNV64 ( &tSettings.m_uAotFilterMask, sizeof(tSettings.m_uAotFilterMask), uHash );
	uHash = sphFNV64 ( &tSettings.m_ePreprocessor, sizeof(tSettings.m_ePreprocessor), uHash );
	uHash = sphFNV64 ( tSettings.m_sIndexTokenFilter.cstr(), tSettings.m_sIndexTokenFilter.Length(), uHash );
	uHash = sphFNV64 ( &iMinPrefixLen, sizeof(iMinPrefixLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iMinInfixLen, sizeof(tSettings.m_iMinInfixLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iMaxSubstringLen, sizeof(tSettings.m_iMaxSubstringLen), uHash );
	uHash = sphFNV64 ( &tSettings.m_iBoundaryStep, sizeof(tSettings.m_iBoundaryStep), uHash );
	uHash = sphFNV64 ( &tSettings.m_iOvershortStep, sizeof(tSettings.m_iOvershortStep), uHash );
	uHash = sphFNV64 ( &tSettings.m_iStopwordStep, sizeof(tSettings.m_iStopwordStep), uHash );

	return uHash;
}

void RtIndex_c::GetIndexFiles ( CSphVector<CSphString> & dFiles, const FilenameBuilder_i * pParentBuilder ) const
{
	CSphString & sMeta = dFiles.Add();
	sMeta.SetSprintf ( "%s.meta", m_sPath.cstr() );

	CSphString & sRam = dFiles.Add();
	sRam.SetSprintf ( "%s.ram", m_sPath.cstr() );
	// RT index might be with flushed RAM into disk chunk
	CSphString sTmpError;
	if ( !sphIsReadable ( sRam.cstr(), &sTmpError ) )
		dFiles.Pop();

	CSphScopedPtr<const FilenameBuilder_i> pFilenameBuilder ( nullptr );
	if ( !pParentBuilder && GetIndexFilenameBuilder() )
	{
		pFilenameBuilder = GetIndexFilenameBuilder() ( m_sIndexName.cstr() );
		pParentBuilder = pFilenameBuilder.Ptr();
	}

	GetSettingsFiles ( m_pTokenizer, m_pDict, GetSettings(), pParentBuilder, dFiles );

	auto tGuard = GetReaderChunks ();
	for ( const CSphIndex * pIndex : tGuard.m_dDiskChunks )
		pIndex->GetIndexFiles ( dFiles, pParentBuilder );

	dFiles.Uniq(); // might be duplicates of tok \ dict files from disk chunks
}


bool RtIndex_c::CopyExternalFiles ( int /*iPostfix*/, StrVec_t & dCopied )
{
	CSphVector<std::pair<CSphString,CSphString>> dExtFiles;
	if ( m_pTokenizer && !m_pTokenizer->GetSettings().m_sSynonymsFile.IsEmpty() )
	{
		const char * szRenameTo = "exceptions.txt";
		dExtFiles.Add ( { m_pTokenizer->GetSettings().m_sSynonymsFile, szRenameTo } );
		const_cast<CSphTokenizerSettings &>(m_pTokenizer->GetSettings()).m_sSynonymsFile = szRenameTo;
	}

	if ( m_pDict )
	{
		const CSphString & sStopwords = m_pDict->GetSettings().m_sStopwords;
		if ( !sStopwords.IsEmpty() )
		{
			StringBuilder_c sNewStopwords(" ");
			StrVec_t dStops = sphSplit ( sStopwords.cstr(), sStopwords.Length(), " \t," );
			ARRAY_FOREACH ( i, dStops )
			{
				CSphString sTmp;
				sTmp.SetSprintf ( "stopwords_%d.txt", i );
				dExtFiles.Add ( { dStops[i], sTmp } );

				sNewStopwords << sTmp;
			}

			const_cast<CSphDictSettings &>(m_pDict->GetSettings()).m_sStopwords = sNewStopwords.cstr();
		}

		StrVec_t dNewWordforms;
		ARRAY_FOREACH ( i, m_pDict->GetSettings().m_dWordforms )
		{
			CSphString sTmp;
			sTmp.SetSprintf ( "wordforms_%d.txt", i );
			dExtFiles.Add( { m_pDict->GetSettings().m_dWordforms[i], sTmp } );
			dNewWordforms.Add(sTmp);
		}

		const_cast<CSphDictSettings &>(m_pDict->GetSettings()).m_dWordforms = dNewWordforms;
	}

	CSphString sPathOnly = GetPathOnly(m_sPath);
	for ( const auto & i : dExtFiles )
	{
		CSphString sDest;
		sDest.SetSprintf ( "%s%s", sPathOnly.cstr(), i.second.cstr() );
		if ( !CopyFile ( i.first, sDest, m_sLastError ) )
			return false;

		dCopied.Add(sDest);
	}

	SaveMeta ( m_iTID );

	ARRAY_FOREACH ( i, m_dDiskChunks )
		if ( !m_dDiskChunks[i]->CopyExternalFiles ( i, dCopied ) )
		{
			m_sLastError = m_dDiskChunks[i]->GetLastError();
			return false;
		}

	return true;
}

void RtIndex_c::CollectFiles ( StrVec_t & dFiles, StrVec_t & dExt ) const
{
	if ( m_pTokenizer && !m_pTokenizer->GetSettings().m_sSynonymsFile.IsEmpty() )
		dExt.Add ( m_pTokenizer->GetSettings ().m_sSynonymsFile );

	if ( m_pDict )
	{
		const CSphString & sStopwords = m_pDict->GetSettings().m_sStopwords;
		if ( !sStopwords.IsEmpty() )
			sphSplit ( dExt, sStopwords.cstr(), sStopwords.Length(), " \t," );

		m_pDict->GetSettings ().m_dWordforms.Apply ( [&dExt] ( const CSphString & a ) { dExt.Add ( a ); } );
	}
	GetReaderChunks ().m_dDiskChunks.Apply ( [&] ( const CSphIndex * a ) { a->CollectFiles ( dFiles, dExt ); } );

	CSphString sPath;
	sPath.SetSprintf ( "%s.meta", m_sPath.cstr () );
	if ( sphIsReadable ( sPath ) )
		dFiles.Add ( sPath );
	sPath.SetSprintf ( "%s.ram", m_sPath.cstr () );
	if ( sphIsReadable ( sPath ) )
		dFiles.Add ( sPath );
}

void RtIndex_c::ProhibitSave()
{
	m_bOptimizeStop = true;
	// just wait optimize finished
	m_tOptimizingLock.Lock();
	m_bSaveDisabled = true;
	m_tOptimizingLock.Unlock();
}

void RtIndex_c::EnableSave()
{
	m_bSaveDisabled = false;
	m_bOptimizeStop = false;
}

void RtIndex_c::LockFileState ( CSphVector<CSphString> & dFiles )
{
	m_bOptimizeStop = true;
	// just wait optimize finished
	m_tOptimizingLock.Lock();
	m_tOptimizingLock.Unlock();

	ForceRamFlush ( false );
	m_bSaveDisabled = true;

	GetIndexFiles ( dFiles, nullptr );
}


void RtIndex_c::CreateReader ( int64_t iSessionId ) const
{
	// rt docstore doesn't need buffered readers
	// but disk chunks need them
	SphChunkGuard_t tGuard = GetReaderChunks();

	for ( const auto & i : tGuard.m_dDiskChunks )
	{
		assert(i);
		i->CreateReader(iSessionId);
	}
}


bool RtIndex_c::GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
	SphChunkGuard_t tGuard = GetReaderChunks();

	for ( const auto & i : tGuard.m_dRamChunks )
	{
		assert ( i && i->m_pDocstore.Ptr() );
		RowID_t tRowID = i->GetRowidByDocid(tDocID);
		if ( tRowID==INVALID_ROWID || i->m_tDeadRowMap.IsSet(tRowID) )
			continue;

		tDoc = i->m_pDocstore->GetDoc ( tRowID, pFieldIds, iSessionId, bPack );
		return true;
	}

	for ( const auto & i : tGuard.m_dDiskChunks )
	{
		assert(i);
		if ( i->GetDoc ( tDoc, tDocID, pFieldIds, iSessionId, bPack ) )
			return true;
	}

	return false;
}


int RtIndex_c::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	return m_pDocstoreFields.Ptr() ? m_pDocstoreFields->GetFieldId ( sName, eType ) : -1;
}

Bson_t RtIndex_c::ExplainQuery ( const CSphString & sQuery ) const
{
	ExplainQueryArgs_t tArgs ( sQuery );
	tArgs.m_pSchema = &GetMatchSchema();

	TokenizerRefPtr_c pQueryTokenizer { m_pTokenizer->Clone ( SPH_CLONE_QUERY ) };
	sphSetupQueryTokenizer ( pQueryTokenizer, IsStarDict ( m_bKeywordDict ), m_tSettings.m_bIndexExactWords, false );

	tArgs.m_pDict = GetStatelessDict ( m_pDict );
	SetupStarDict ( tArgs.m_pDict, pQueryTokenizer );
	SetupExactDict ( tArgs.m_pDict, pQueryTokenizer );
	if ( m_pFieldFilter )
		tArgs.m_pFieldFilter = m_pFieldFilter->Clone();
	tArgs.m_pSettings = &m_tSettings;
	tArgs.m_pWordlist = this;
	tArgs.m_pQueryTokenizer = pQueryTokenizer;
	tArgs.m_iExpandKeywords = m_iExpandKeywords;
	tArgs.m_iExpansionLimit = m_iExpansionLimit;
	tArgs.m_bExpandPrefix = ( m_pDict->GetSettings().m_bWordDict && IsStarDict ( m_bKeywordDict ) );

	SphChunkGuard_t tGuard = GetReaderChunks ();
	tArgs.m_pIndexData = &tGuard.m_dRamChunks;

	return Explain ( tArgs );
}

bool RtIndex_c::NeedStoreWordID () const
{
	return ( m_tSettings.m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.GetLength() );
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
		m_pError->SetSprintf ( "failed to sync %s: %s" , m_sName.cstr(), strerrorm(errno) );

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
		sphWarning ( "binlog: %s: CRC mismatch (index=%s, tid=" INT64_FMT ", pos=" INT64_FMT ")", sOp, sIndexName ? sIndexName : "", iTid, iTxnPos );
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

RtBinlog_c::BlopStartEnd_t::BlopStartEnd_t ( RtBinlog_c & tBinlog, int64_t * pTID, Blop_e eBlop, const char * szIndexName )
	: m_tBinlog ( tBinlog )
{
	m_tBinlog.WriteBlopHeader ( pTID, eBlop, szIndexName );
}


RtBinlog_c::BlopStartEnd_t::~BlopStartEnd_t()
{
	// checksum
	m_tBinlog.m_tWriter.WriteCrc ();

	// finalize
	m_tBinlog.CheckDoFlush();
	m_tBinlog.CheckDoRestart();
}

//////////////////////////////////////////////////////////////////////////

RtBinlog_c::RtBinlog_c ()
{
	MEMORY ( MEM_BINLOG );

	m_tWriter.SetBufferSize ( BINLOG_WRITE_BUFFER );
}

RtBinlog_c::~RtBinlog_c ()
{
	if ( !m_bDisabled )
	{
		DoCacheWrite();
		m_tWriter.CloseFile();
		LockFile ( false );
	}
}

void RtBinlog_c::BinlogCommit ( int64_t * pTID, const char * szIndexName, const RtSegment_t * pSeg, const CSphVector<DocID_t> & dKlist, bool bKeywordDict )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( MEM_BINLOG );
	ScopedMutex_t tWriteLock ( m_tWriteLock );
	BlopStartEnd_t tStartEnd ( *this, pTID, BLOP_COMMIT, szIndexName );

	// save txn data
	if ( !pSeg || !pSeg->m_uRows )
		m_tWriter.ZipOffset ( 0 );
	else
	{
		m_tWriter.ZipOffset ( pSeg->m_uRows );
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
		SaveVector ( m_tWriter, pSeg->m_dBlobs );
		SaveVector ( m_tWriter, pSeg->m_dKeywordCheckpoints );

		if ( pSeg->m_pDocstore.Ptr() )
		{
			m_tWriter.PutByte(1);
			pSeg->m_pDocstore->Save ( m_tWriter );
		}
		else
			m_tWriter.PutByte(0);
	}

	SaveVector ( m_tWriter, dKlist );
}


void RtBinlog_c::BinlogUpdateAttributes ( int64_t * pTID, const char * szIndexName, const CSphAttrUpdate & tUpd )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( MEM_BINLOG );
	ScopedMutex_t tWriteLock ( m_tWriteLock );
	BlopStartEnd_t tStartEnd ( *this, pTID, BLOP_UPDATE_ATTRS, szIndexName );

	// update data
	m_tWriter.ZipOffset ( tUpd.m_dAttributes.GetLength() );
	for ( const auto & i : tUpd.m_dAttributes )
	{
		m_tWriter.PutString ( i.m_sName );
		m_tWriter.ZipOffset ( i.m_eType );
	}

	// POD vectors
	SaveVector ( m_tWriter, tUpd.m_dPool );
	SaveVector ( m_tWriter, tUpd.m_dDocids );
	SaveVector ( m_tWriter, tUpd.m_dRowOffset );
}


void RtBinlog_c::BinlogReconfigure ( int64_t * pTID, const char * szIndexName, const CSphReconfigureSetup & tSetup )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( MEM_BINLOG );
	ScopedMutex_t tWriteLock ( m_tWriteLock );
	BlopStartEnd_t tStartEnd ( *this, pTID, BLOP_RECONFIGURE, szIndexName );

	// reconfigure data
	SaveIndexSettings ( m_tWriter, tSetup.m_tIndex );
	SaveTokenizerSettings ( m_tWriter, tSetup.m_pTokenizer, 0 );
	SaveDictionarySettings ( m_tWriter, tSetup.m_pDict, false, 0 );

	CSphFieldFilterSettings tFieldFilterSettings;
	if ( tSetup.m_pFieldFilter.Ptr() )
		tSetup.m_pFieldFilter->GetSettings(tFieldFilterSettings);
	tFieldFilterSettings.Save(m_tWriter);
}


void RtBinlog_c::WriteBlopHeader ( int64_t * pTID, Blop_e eBlop, const char * szIndexName )
{
	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( szIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipOffset ( eBlop );
	m_tWriter.ZipOffset ( uIndex );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );
}


// here's been going binlogs with ALL closed indices removing
void RtBinlog_c::NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown )
{
	if ( m_bReplayMode )
		sphInfo ( "index '%s': ramchunk saved. TID=" INT64_FMT "", sIndexName, iTID );

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
			sphWarning ( "binlog: failed to unlink %s: %s (remove it manually)", sLog.cstr(), strerrorm(errno) );

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

#ifndef LOCALDATADIR
#define LOCALDATADIR "."
#endif

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : LOCALDATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	m_iRestartSize = hSearchd.GetSize ( "binlog_max_log_size", m_iRestartSize );

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LoadMeta();
	}
}

void RtBinlog_c::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback )
{
	if ( m_bDisabled )
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


bool RtBinlog_c::IsFlushingEnabled() const
{
	return ( !m_bDisabled && m_eOnCommit!=ACTION_FSYNC );
}


void RtBinlog_c::DoFlush ()
{
	assert ( !m_bDisabled );
	MEMORY ( MEM_BINLOG );

	if ( m_eOnCommit==ACTION_NONE || m_tWriter.HasUnwrittenData() )
	{
		ScopedMutex_t LockWriter( m_tWriteLock);
		m_tWriter.Flush();
	}

	if ( m_tWriter.HasUnsyncedData() )
		m_tWriter.Fsync();

	m_iLastFlushed = sphMicroTimer ();
}

int64_t RtBinlog_c::NextFlushingTime () const
{
	if ( !m_iLastFlushed )
		return sphMicroTimer () + m_iFlushPeriod;
	return m_iLastFlushed + m_iFlushPeriod;
}

bool sphFlushBinlogEnabled ()
{
	if ( !g_pRtBinlog )
		return false;

	return g_pRtBinlog->IsFlushingEnabled ();
}

void sphFlushBinlog ()
{
	if ( !g_pRtBinlog )
		return;

	g_pRtBinlog->DoFlush ();
}

int64_t sphNextFlushTimestamp ()
{
	if ( !g_pRtBinlog )
		return -1;

	return g_pRtBinlog->NextFlushingTime ();
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

	if ( !bLoaded64bit )
		sphDie ( "indexes with 32-bit docids are no longer supported; recovery requires previous binary version" );

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
	wrMeta.PutByte ( 1 ); // was USE_64BIT

	// save list of active log files
	wrMeta.ZipInt ( m_dLogFiles.GetLength() );
	ARRAY_FOREACH ( i, m_dLogFiles )
		wrMeta.ZipInt ( m_dLogFiles[i].m_iExt ); // everything else is saved in logs themselves

	wrMeta.CloseFile();

	if ( wrMeta.IsError() )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	if ( sph::rename ( sMeta.cstr(), sMetaOld.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMeta.cstr(), sMetaOld.cstr(), errno, strerrorm(errno) ); // !COMMIT handle this gracefully
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
			sphDie ( "failed to open '%s': %u '%s'", sName.cstr(), errno, strerrorm(errno) );

		if ( !sphLockEx ( iLockFD, false ) )
			sphDie ( "failed to lock '%s': %u '%s'", sName.cstr(), errno, strerrorm(errno) );

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

	// update meta first then only remove binlog file
	SaveMeta();

	// create file
	CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );

	if ( !iLastState ) // reuse the last binlog since it is empty or useless.
		::unlink ( sLog.cstr() );

	if ( !m_tWriter.OpenFile ( sLog.cstr(), m_sWriterError ) )
		sphDie ( "failed to create %s: errno=%d, error=%s", sLog.cstr(), errno, strerrorm(errno) );

	// emit header
	m_tWriter.PutDword ( BINLOG_HEADER_MAGIC );
	m_tWriter.PutDword ( BINLOG_VERSION );
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
			sphDie ( "binlog: log missing txn marker at pos=" INT64_FMT " (corrupted?)", iPos );
			break;
		}

		tReader.ResetCrc ();
		const uint64_t uOp = tReader.UnzipOffset ();

		if ( uOp<=0 || uOp>=BLOP_TOTAL )
			sphDie ( "binlog: unexpected entry (blop=" UINT64_FMT ", pos=" INT64_FMT ")", uOp, iPos );

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

			case BLOP_PQ_ADD:
				bReplayOK = ReplayPqAdd ( iBinlog, uReplayFlags, tReader );
				break;

			case BLOP_PQ_DELETE:
				bReplayOK = ReplayPqDelete ( iBinlog, uReplayFlags, tReader );
				break;

			default:
				sphDie ( "binlog: internal error, unhandled entry (blop=%d)", (int)uOp );
		}

		dTotal [ uOp ] += bReplayOK ? 1 : 0;
		dTotal [ BLOP_TOTAL ]++;
	}

	tmReplay = sphMicroTimer() - tmReplay;

	if ( tReader.GetErrorFlag() )
		sphWarning ( "binlog: log io error at pos=" INT64_FMT ": %s", iPos, sError.cstr() );

	if ( !bReplayOK )
		sphWarning ( "binlog: replay error at pos=" INT64_FMT ")", iPos );

	// show additional replay statistics
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		const BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( tIndex.m_iPreReplayTID < tIndex.m_iMaxTID )
		{
			sphInfo ( "binlog: index %s: recovered from tid " INT64_FMT " to tid " INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );

		} else
		{
			sphInfo ( "binlog: index %s: skipped at tid " INT64_FMT " and max binlog tid " INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );
		}
	}

	sphInfo ( "binlog: replay stats: %d rows in %d commits; %d updates, %d reconfigure; %d pq-add; %d pq-delete; %d indexes",
		m_iReplayedRows, dTotal[BLOP_COMMIT], dTotal[BLOP_UPDATE_ATTRS], dTotal[BLOP_RECONFIGURE], dTotal[BLOP_PQ_ADD], dTotal[BLOP_PQ_DELETE], dTotal[BLOP_ADD_INDEX] );
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
		sphDie ( "binlog: %s: unexpected index id (id=%d, max=%d, pos=" INT64_FMT ")",
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
	auto iTID = (int64_t) tReader.UnzipOffset();
	auto tmStamp = (int64_t) tReader.UnzipOffset();

	CSphRefcountedPtr<RtSegment_t> pSeg;
	CSphVector<DocID_t> dKlist;

	DWORD uRows = tReader.UnzipOffset();
	if ( uRows )
	{
		pSeg = new RtSegment_t(uRows);
		pSeg->m_uRows = uRows;
		pSeg->m_tAliveRows.store ( uRows, std::memory_order_relaxed );
		m_iReplayedRows += (int)uRows;

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
		LoadVector ( tReader, pSeg->m_dBlobs );
		LoadVector ( tReader, pSeg->m_dKeywordCheckpoints );

		bool bHaveDocstore = !!tReader.GetByte();
		if ( bHaveDocstore )
		{
			assert ( tIndex.m_pRT );
			pSeg->SetupDocstore ( tIndex.m_pRT ? &(tIndex.m_pRT->GetInternalSchema()) : nullptr );
			assert ( pSeg->m_pDocstore.Ptr() );
			CSphString sError;
			if ( !pSeg->m_pDocstore->Load ( tReader ) )
				sphWarning ( "binlog: commit: docstore load error: %s (index=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
					tReader.GetErrorMessage().cstr(), tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos );
		}

		pSeg->BuildDocID2RowIDMap();
	}

	LoadVector ( tReader, dKlist );

	if ( !PerformChecks ( "commit", tIndex, iTID, iTxnPos, tmStamp, uReplayFlags, tReader ) )
		return false;

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pRT && iTID > tIndex.m_pRT->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		CheckTidSeq ( "commit", tIndex, iTID, tIndex.m_pRT, iTxnPos );

		// in case dict=keywords
		// + cook checkpoint
		// + build infixes
		if ( tIndex.m_pRT->IsWordDict() && pSeg )
		{
			FixupSegmentCheckpoints ( pSeg );
			BuildSegmentInfixes ( pSeg, tIndex.m_pRT->GetDictionary()->HasMorphology(),
				tIndex.m_pRT->IsWordDict(), tIndex.m_pRT->GetSettings().m_iMinInfixLen, tIndex.m_pRT->GetWordCheckoint(), ( tIndex.m_pRT->GetMaxCodepointLength()>1 ), tIndex.m_pRT->GetSettings().m_eHitless );
		}

		// actually replay
		if ( !tIndex.m_pRT->CommitReplayable ( pSeg, dKlist, NULL, false ) )
			return false;

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pRT->m_iTID = iTID;
	}

	UpdateIndexInfo ( tIndex, iTID, tmStamp );

	return true;
}


bool RtBinlog_c::ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const
{
	// load and check index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	uint64_t uVal = tReader.UnzipOffset();
	if ( (int)uVal!=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: indexadd: unexpected index id (id=" UINT64_FMT ", expected=%d, pos=" INT64_FMT ")",
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
			sphDie ( "binlog: duplicate index name (name=%s, dupeid=%d, pos=" INT64_FMT ")",
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
			tIndex.m_pRT = (RtIndex_c*)pIndex;

		if ( pIndex->IsPQ() )
			tIndex.m_pPQ = (PercolateIndex_i *)pIndex;

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
	tUpd.m_dAttributes.Resize ( iAttrs ); // FIXME! sanity check
	for ( auto & i : tUpd.m_dAttributes )
	{
		i.m_sName = tReader.GetString();
		i.m_eType = (ESphAttr) tReader.UnzipOffset(); // safe, we'll crc check later
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
		sphDie ( "binlog: update: descending tid (index=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
	if ( tmStamp<tIndex.m_tmMax )
		sphDie ( "binlog: update: descending time (index=%s, lasttime=" INT64_FMT ", logtime=" INT64_FMT ", pos=" INT64_FMT ")",
			tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

	if ( tIndex.m_pIndex && iTID > tIndex.m_pIndex->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pIndex->m_iTID+1 )
			sphWarning ( "binlog: update: unexpected tid (index=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
				tIndex.m_sName.cstr(), tIndex.m_pIndex->m_iTID, iTID, iTxnPos );

		CSphString sError, sWarning;
		bool bCritical = false;
		tIndex.m_pIndex->UpdateAttributes ( tUpd, -1, bCritical, sError, sWarning ); // FIXME! check for errors
		assert ( !bCritical ); // fixme! handle this

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
				"(cached " INT64_FMT " to " INT64_FMT ", replayed " INT64_FMT " to " INT64_FMT ")",
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
	CSphScopedPtr<const FilenameBuilder_i> pFilenameBuilder ( nullptr );
	if ( GetIndexFilenameBuilder() )
		pFilenameBuilder = GetIndexFilenameBuilder() ( tIndex.m_sName.cstr() );

	CSphReconfigureSettings tSettings;
	LoadIndexSettings ( tSettings.m_tIndex, tReader, INDEX_FORMAT_VERSION );
	if ( !tSettings.m_tTokenizer.Load ( pFilenameBuilder.Ptr(), tReader, tEmbeddedFiles, sError ) )
		sphDie ( "binlog: reconfigure: failed to load settings (index=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ", error=%s)",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos, sError.cstr() );

	tSettings.m_tDict.Load ( tReader, tEmbeddedFiles, sError );
	tSettings.m_tFieldFilter.Load(tReader);

	if ( !PerformChecks ( "reconfigure", tIndex, iTID, iTxnPos, tmStamp, uReplayFlags, tReader ) )
		return false;

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pRT && iTID > tIndex.m_pRT->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pRT->m_iTID+1 )
			sphWarning ( "binlog: reconfigure: unexpected tid (index=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos );

		sError = "";
		CSphReconfigureSetup tSetup;
		StrVec_t dWarnings;
		bool bSame = tIndex.m_pRT->IsSameSettings ( tSettings, tSetup, dWarnings, sError );

		if ( !sError.IsEmpty() )
			sphWarning ( "binlog: reconfigure: wrong settings (index=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ", error=%s)",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos, sError.cstr() );

		if ( !bSame )
		{
			if ( !tIndex.m_pRT->Reconfigure ( tSetup ) )
				return false;
		}

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pRT->m_iTID = iTID;
	}

	UpdateIndexInfo ( tIndex, iTID, tmStamp );

	return true;
}


void RtBinlog_c::CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode )
{
#ifndef LOCALDATADIR
#define LOCALDATADIR "."
#endif

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : LOCALDATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LockFile ( false );
	}
}

bool RtBinlog_c::CheckCrc ( const char * sOp, const CSphString & sIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader ) const
{
	return !tReader.GetErrorFlag() && tReader.CheckCrc ( sOp, sIndex.cstr(), iTID, iTxnPos );
}

void RtBinlog_c::CheckTid ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos ) const
{
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: %s: descending tid (index=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			sOp, tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
}


template <typename T>
void RtBinlog_c::CheckTidSeq ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, T * pIndexTID, int64_t iTxnPos ) const
{
	if ( pIndexTID && iTID!=pIndexTID->m_iTID+1 )
		sphWarning ( "binlog: %s: unexpected tid (index=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			sOp, tIndex.m_sName.cstr(), pIndexTID->m_iTID, iTID, iTxnPos );
}

void RtBinlog_c::CheckTime ( BinlogIndexInfo_t & tIndex, const char * sOp, int64_t tmStamp, int64_t iTID, int64_t iTxnPos, DWORD uReplayFlags ) const
{
	if ( tmStamp<tIndex.m_tmMax )
	{
		if (!( uReplayFlags & SPH_REPLAY_ACCEPT_DESC_TIMESTAMP ))
			sphDie ( "binlog: %s: descending time (index=%s, lasttime=" INT64_FMT ", logtime=" INT64_FMT ", pos=" INT64_FMT ")",
				sOp, tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

		sphWarning ( "binlog: %s: replaying txn despite descending time "
			"(index=%s, logtid=" INT64_FMT ", lasttime=" INT64_FMT ", logtime=" INT64_FMT ", pos=" INT64_FMT ")",
			sOp, tIndex.m_sName.cstr(), iTID, tIndex.m_tmMax, tmStamp, iTxnPos );
		tIndex.m_tmMax = tmStamp;
	}
}


bool RtBinlog_c::PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, int64_t tmStamp, DWORD uReplayFlags, BinlogReader_c & tReader ) const
{
	// checksum
	if ( !CheckCrc ( "commit", tIndex.m_sName, iTID, iTxnPos, tReader ) )
		return false;

	// check TID
	CheckTid ( "commit", tIndex, iTID, iTxnPos );

	// check timestamp
	CheckTime ( tIndex, "commit", tmStamp, iTID, iTxnPos, uReplayFlags );

	return true;
}


void RtBinlog_c::UpdateIndexInfo ( BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t tmStamp ) const
{
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
}

bool RtBinlog_c::PreOp ( Blop_e eOp, int64_t * pTID, const char * sIndexName )
{
	if ( m_bReplayMode || m_bDisabled )
	{
		// still need to advance TID as index flush according to it
		if ( m_bDisabled )
		{
			ScopedMutex_t tLock ( m_tWriteLock );
			(*pTID)++;
		}
		return false;
	}

	Verify ( m_tWriteLock.Lock() );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipOffset ( eOp );
	m_tWriter.ZipOffset ( uIndex );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );

	return true;
}

void RtBinlog_c::PostOp()
{
	// checksum
	m_tWriter.WriteCrc();

	// finalize
	CheckDoFlush();
	CheckDoRestart();
	Verify ( m_tWriteLock.Unlock() );
}

void RtBinlog_c::BinlogPqAdd ( int64_t * pTID, const char * sIndexName, const StoredQueryDesc_t & tStored )
{
	MEMORY ( MEM_BINLOG );
	if ( !PreOp ( BLOP_PQ_ADD, pTID, sIndexName ) )
		return;

	// save txn data
	SaveStoredQuery ( tStored, m_tWriter );

	PostOp();
}

bool RtBinlog_c::ReplayPqAdd ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "pq-add" );

	// load transaction data
	const int64_t iTID = (int64_t) tReader.UnzipOffset();
	const int64_t tmStamp = (int64_t) tReader.UnzipOffset();

	StoredQueryDesc_t tStored;
	LoadStoredQuery ( PQ_META_VERSION_MAX, tStored, tReader );

	if ( !PerformChecks ( "pq-add", tIndex, iTID, iTxnPos, tmStamp, uReplayFlags, tReader ) )
		return false;

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pPQ && iTID>tIndex.m_pPQ->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		CheckTidSeq ( "pq-add", tIndex, iTID, tIndex.m_pPQ, iTxnPos );

		CSphString sError;
		PercolateQueryArgs_t tArgs ( tStored );
		// at binlog query already passed replace checks
		tArgs.m_bReplace = true;

		// actually replay
		StoredQuery_i * pQuery = tIndex.m_pPQ->CreateQuery ( tArgs, sError );
		if ( !pQuery )
			sphDie ( "binlog: pq-add: apply error (index=%s, lasttime=" INT64_FMT ", logtime=" INT64_FMT ", pos=" INT64_FMT ", '%s')",
				tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos, sError.cstr() );

		tIndex.m_pPQ->ReplayCommit ( pQuery );

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pPQ->m_iTID = iTID;
	}

	// update info
	UpdateIndexInfo ( tIndex, iTID, tmStamp );

	return true;
}

void RtBinlog_c::BinlogPqDelete ( int64_t* pTID, const char* sIndexName, const VecTraits_T<int64_t>& dQueries, const char* sTags )
{
	MEMORY ( MEM_BINLOG );
	if ( !PreOp ( BLOP_PQ_DELETE, pTID, sIndexName ) )
		return;

	// save txn data
	SaveDeleteQuery ( dQueries, sTags, m_tWriter );

	PostOp();
}

bool RtBinlog_c::ReplayPqDelete ( int iBinlog, DWORD uReplayFlags, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "pq-delete" );

	// load transaction data
	const int64_t iTID = (int64_t) tReader.UnzipOffset();
	const int64_t tmStamp = (int64_t) tReader.UnzipOffset();

	CSphVector<int64_t> dQueries;
	CSphString sTags;
	LoadDeleteQuery ( dQueries, sTags, tReader );

	if ( !PerformChecks ( "pq-delete", tIndex, iTID, iTxnPos, tmStamp, uReplayFlags, tReader ) )
		return false;

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pPQ && iTID>tIndex.m_pPQ->m_iTID )
	{
		CheckTidSeq ( "pq-delete", tIndex, iTID, tIndex.m_pPQ, iTxnPos );

		// actually replay
		if ( dQueries.GetLength() )
			tIndex.m_pPQ->ReplayDeleteQueries ( dQueries );
		else
			tIndex.m_pPQ->ReplayDeleteQueries ( sTags.cstr() );

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pPQ->m_iTID = iTID;
	}

	// update info
	UpdateIndexInfo ( tIndex, iTID, tmStamp );

	return true;
}


//////////////////////////////////////////////////////////////////////////

RtIndex_i * sphGetCurrentIndexRT()
{
	RtAccum_t * pAcc = g_pTlsAccum;
	if ( pAcc )
		return pAcc->GetIndex();
	return nullptr;
}

RtIndex_i * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName,
	int64_t iRamSize, const char * sPath, bool bKeywordDict )
{
	MEMORY ( MEM_INDEX_RT );
	return new RtIndex_c ( tSchema, sIndexName, iRamSize, sPath, bKeywordDict );
}

void sphRTInit ( const CSphConfigSection & hSearchd, bool bTestMode, const CSphConfigSection * pCommon )
{
	MEMORY ( MEM_BINLOG );

	g_bRTChangesAllowed = false;
	g_pRtBinlog = new RtBinlog_c();
	if ( !g_pRtBinlog )
		sphDie ( "binlog: failed to create binlog" );
	g_pBinlog = g_pRtBinlog;

	// check binlog path before detaching from the console
	g_pRtBinlog->CheckPath ( hSearchd, bTestMode );

	if ( pCommon )
		g_bProgressiveMerge = pCommon->GetBool ( "progressive_merge", true );
}

void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	assert ( g_pBinlog );
	g_pRtBinlog->Configure ( hSearchd, bTestMode );
}


void sphRTDone ()
{
	// its valid for "searchd --stop" case
	SafeDelete ( g_pBinlog );
}


void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( MEM_BINLOG );
	g_pRtBinlog->Replay ( hIndexes, uReplayFlags, pfnProgressCallback );
	g_bRTChangesAllowed = true;
}

static bool g_bTestMode = false;

void sphRTSetTestMode ()
{
	g_bTestMode = true;
}

bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema & tSchema, CSphString & sError, bool bSkipValidation )
{
	// fields
	SmallStringHash_T<BYTE> hFields;
	for ( CSphVariant * v=hIndex("rt_field"); v; v=v->m_pNext )
	{
		CSphString sFieldName = v->cstr();
		sFieldName.ToLower();
		tSchema.AddField ( sFieldName.cstr() );
		hFields.Add ( 1, sFieldName );
	}

	if ( !tSchema.GetFieldsCount() && !bSkipValidation )
	{
		sError.SetSprintf ( "no fields configured (use rt_field directive)" );
		return false;
	}

	if ( tSchema.GetFieldsCount()>SPH_MAX_FIELDS )
	{
		sError.SetSprintf ( "too many fields (fields=%d, max=%d)", tSchema.GetFieldsCount(), SPH_MAX_FIELDS );
		return false;
	}

	// add id column
	CSphColumnInfo tCol ( sphGetDocidName() );
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSchema.AddAttr ( tCol, false );

	// attrs
	const int iNumTypes = 9;
	const char * sTypes[iNumTypes] = { "rt_attr_uint", "rt_attr_bigint", "rt_attr_timestamp", "rt_attr_bool", "rt_attr_float", "rt_attr_string", "rt_attr_json", "rt_attr_multi", "rt_attr_multi_64" };
	const ESphAttr iTypes[iNumTypes] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_TIMESTAMP, SPH_ATTR_BOOL, SPH_ATTR_FLOAT, SPH_ATTR_STRING, SPH_ATTR_JSON, SPH_ATTR_UINT32SET, SPH_ATTR_INT64SET };

	for ( int iType=0; iType<iNumTypes; ++iType )
	{
		for ( CSphVariant * v = hIndex ( sTypes[iType] ); v; v = v->m_pNext )
		{
			StrVec_t dNameParts;
			sphSplit ( dNameParts, v->cstr(), ":");
			CSphColumnInfo tCol ( dNameParts[0].cstr(), iTypes[iType]);
			tCol.m_sName.ToLower();

			// bitcount
			tCol.m_tLocator = CSphAttrLocator();
			if ( dNameParts.GetLength ()>1 )
			{
				if ( tCol.m_eAttrType==SPH_ATTR_INTEGER )
				{
					auto iBits = strtol ( dNameParts[1].cstr(), NULL, 10 );
					if ( iBits>0 && iBits<=ROWITEM_BITS )
						tCol.m_tLocator.m_iBitCount = (int)iBits;
					else
						sError.SetSprintf ( "attribute '%s': invalid bitcount=%d (bitcount ignored)", tCol.m_sName.cstr(), (int)iBits );

				} else
					sError.SetSprintf ( "attribute '%s': bitcount is only supported for integer types (bitcount ignored)", tCol.m_sName.cstr() );
			}

			if ( !SchemaConfigureCheckAttribute ( tSchema, tCol, sError ) )
				return false;

			tSchema.AddAttr ( tCol, false );

			if ( tCol.m_eAttrType!=SPH_ATTR_STRING && hFields.Exists ( tCol.m_sName ) && !bSkipValidation )
			{
				sError.SetSprintf ( "can not add attribute that shadows '%s' field", tCol.m_sName.cstr () );
				return false;
			}
		}
	}

	// add blob attr locator
	if ( tSchema.HasBlobAttrs() )
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

	// should be id and at least one attribute
	if ( tSchema.GetAttrsCount()<2 && !g_bTestMode && !bSkipValidation )
	{
		sError.SetSprintf ( "no attribute configured (use rt_attr directive)" );
		return false;
	}

	return true;
}

void RtAccum_t::LoadRtTrx ( const BYTE * pData, int iLen )
{
	MemoryReader_c tReader ( pData, iLen );
	m_bReplace = !!tReader.GetByte();
	m_uAccumDocs = tReader.GetDword();

	// insert and replace
	m_dAccum.Resize ( tReader.GetDword() );
	for ( CSphWordHit & tHit : m_dAccum )
	{
		// such manual serialization is necessary because CSphWordHit is internally aligned by 8,
		// and it's size is 3*8, however actually we have 4+8+4 bytes in members.
		// Sending raw unitialized bytes is not ok, since it may influent crc checking.
		tReader.GetBytes ( &tHit.m_tRowID, (int) sizeof ( tHit.m_tRowID ) );
		tReader.GetBytes ( &tHit.m_uWordID, (int) sizeof ( tHit.m_uWordID ) );
		tReader.GetBytes ( &tHit.m_uWordPos, (int) sizeof ( tHit.m_uWordPos ) );
	}
	m_dAccumRows.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dAccumRows.Begin(), (int) m_dAccumRows.GetLengthBytes() );
	m_dBlobs.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dBlobs.Begin(), (int) m_dBlobs.GetLengthBytes() );
	m_dPerDocHitsCount.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dPerDocHitsCount.Begin(), (int) m_dPerDocHitsCount.GetLengthBytes() );
	m_dPackedKeywords.Reset ( tReader.GetDword() );
	tReader.GetBytes ( m_dPackedKeywords.Begin(), (int) m_dPackedKeywords.GetLengthBytes() );
	if ( !!tReader.GetByte() )
	{
		if ( !m_pDocstore.Ptr() )
			m_pDocstore = CreateDocstoreRT();
		assert ( m_pDocstore.Ptr() );
		m_pDocstore->Load ( tReader );
	}

	// delete
	m_dAccumKlist.Resize ( tReader.GetDword() );
	tReader.GetBytes ( m_dAccumKlist.Begin(), (int) m_dAccumKlist.GetLengthBytes() );
}

void RtAccum_t::SaveRtTrx ( MemoryWriter_c & tWriter ) const
{
	tWriter.PutByte ( m_bReplace ); // this need only for data sort on commit
	tWriter.PutDword ( m_uAccumDocs );

	// insert and replace
	tWriter.PutDword ( m_dAccum.GetLength() );
	for ( const CSphWordHit& tHit : m_dAccum )
	{
		tWriter.PutBytes ( &tHit.m_tRowID, sizeof ( tHit.m_tRowID ) );
		tWriter.PutBytes ( &tHit.m_uWordID, sizeof ( tHit.m_uWordID ) );
		tWriter.PutBytes ( &tHit.m_uWordPos, sizeof ( tHit.m_uWordPos ) );
	}
	tWriter.PutDword ( m_dAccumRows.GetLength() );
	tWriter.PutBytes ( m_dAccumRows.Begin(), (int) m_dAccumRows.GetLengthBytes() );
	tWriter.PutDword ( m_dBlobs.GetLength() );
	tWriter.PutBytes ( m_dBlobs.Begin(), (int) m_dBlobs.GetLengthBytes() );
	tWriter.PutDword ( m_dPerDocHitsCount.GetLength() );
	tWriter.PutBytes ( m_dPerDocHitsCount.Begin(), (int) m_dPerDocHitsCount.GetLengthBytes() );
	// packed keywords default length is 1 no need to pass that
	int iLen = ( m_bKeywordDict && m_pDictRt->GetPackedLen()>1 ? (int) m_pDictRt->GetPackedLen() : 0 );
	tWriter.PutDword ( iLen );
	if ( iLen )
		tWriter.PutBytes ( m_pDictRt->GetPackedKeywords(), iLen );
	tWriter.PutByte ( m_pDocstore.Ptr()!=0 );
	if ( m_pDocstore.Ptr() )
		m_pDocstore->Save ( tWriter );

	// delete
	tWriter.PutDword ( m_dAccumKlist.GetLength() );
	tWriter.PutBytes ( m_dAccumKlist.Begin(), (int) m_dAccumKlist.GetLengthBytes() );
}

void RtIndex_c::SetSchema ( const CSphSchema & tSchema )
{
	m_tSchema = tSchema;
	m_iStride = m_tSchema.GetRowSize();
	m_uSchemaHash = SchemaFNV ( m_tSchema );

#ifndef NDEBUG
	// check that index cols are static
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		assert ( !m_tSchema.GetAttr(i).m_tLocator.m_bDynamic );
#endif

	if ( m_tSchema.HasStoredFields() )
	{
		m_pDocstoreFields = CreateDocstoreFields();
		SetupDocstoreFields ( *m_pDocstoreFields.Ptr(), m_tSchema );
	}

	ARRAY_FOREACH ( i, m_dFieldLens )
	{
		m_dFieldLens[i] = 0;
		m_dFieldLensRam[i] = 0;
		m_dFieldLensDisk[i] = 0;
	}
}

uint64_t SchemaFNV ( const ISphSchema & tSchema )
{
	uint64_t uHash = SPH_FNV64_SEED;

	// attrs
	int iAttrsCount = tSchema.GetAttrsCount();
	for ( int i=0; i<iAttrsCount; i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr ( i );
		uHash = sphFNV64cont ( tAttr.m_sName.cstr(), uHash );
		uHash = sphFNV64 ( &tAttr.m_eAttrType, sizeof( tAttr.m_eAttrType ), uHash );
		uHash = sphFNV64 ( &tAttr.m_tLocator, sizeof( tAttr.m_tLocator ), uHash );
	}

	// fulltext fields
	int iFieldsCount = tSchema.GetFieldsCount();
	for ( int i=0; i<iFieldsCount; i++ )
	{
		const CSphColumnInfo & tField = tSchema.GetField ( i );
		uHash = sphFNV64cont ( tField.m_sName.cstr(), uHash );
		uHash = sphFNV64 ( &tField.m_uFieldFlags, sizeof( tField.m_uFieldFlags ), uHash );
	}

	return uHash;
}