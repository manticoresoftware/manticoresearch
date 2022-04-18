//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "secondaryindex.h"

#include <boost/preprocessor/repetition/repeat.hpp>

#include "histogram.h"
#include "sphinxint.h"
#include "killlist.h"
#include "attribute.h"

#include "util/util.h"
#include "secondarylib.h"


static bool HaveIndex ( const CSphString & sAttr )
{
	return sAttr=="id";
}

//////////////////////////////////////////////////////////////////////////

bool ReturnIteratorResult ( RowID_t * pRowID, RowID_t * pRowIdStart, RowIdBlock_t & dRowIdBlock )
{
	if ( pRowID==pRowIdStart )
		return false;

	dRowIdBlock = RowIdBlock_t(pRowIdStart, pRowID-pRowIdStart);
	return true;
}

//////////////////////////////////////////////////////////////////////////
class SecondaryIndexIterator_c : public RowidIterator_i
{
protected:
	static const int MAX_COLLECTED = 128;
	CSphFixedVector<RowID_t> m_dCollected {MAX_COLLECTED};
};

template <bool BITMAP>
class CachedIterator_T : public SecondaryIndexIterator_c
{
public:
				CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs );

	bool		HintRowID ( RowID_t tRowID ) override;

protected:
	CSphTightVector<RowID_t> m_dRowIDs;
	int			m_iId = 0;

	CSphBitvec	m_tBitmap;
	RowID_t		m_tRowID = 0;
	RowID_t		m_tMinRowID = INVALID_ROWID;
	RowID_t		m_tMaxRowID = INVALID_ROWID;

	void		Add ( RowID_t tRowID );
	bool		Finalize();
	bool		ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock );
};

template <>
CachedIterator_T<true>::CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs )
{
	m_tBitmap.Init(uTotalDocs);
}

template <>
CachedIterator_T<false>::CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs )
{
	m_dRowIDs.Reserve(iRsetEstimate);
}

template <>
bool CachedIterator_T<true>::HintRowID  ( RowID_t tRowID )
{
	if ( tRowID<m_tRowID )
		return true;

	m_tRowID = tRowID;
	while ( m_tRowID<=m_tMaxRowID )
	{
		if ( m_tBitmap.BitGet(m_tRowID) )
			return true;

		m_tRowID++;
	}
	
	return false;
}

template <>
bool CachedIterator_T<false>::HintRowID  ( RowID_t tRowID )
{
	RowID_t * pRowID = m_dRowIDs.Begin() + m_iId;
	RowID_t * pRowIdMax = m_dRowIDs.End();

	if ( m_dRowIDs.IsEmpty() || pRowID>=pRowIdMax )
		return false;

	const int64_t LINEAR_THRESH = 256;
	if ( tRowID - *pRowID < LINEAR_THRESH )
	{
		const RowID_t * pRowIdStart = m_dRowIDs.Begin();

		while ( pRowID<pRowIdMax && *pRowID<tRowID )
			pRowID++;

		m_iId = pRowID-pRowIdStart;
		return pRowID<pRowIdMax;
	}
	else
	{
		// we assume we are never rewinding backwards
		const RowID_t * pFound = sphBinarySearchFirst ( pRowID, pRowIdMax-1, SphIdentityFunctor_T<RowID_t>(), tRowID );
		assert(pFound);

		if ( *pFound < tRowID )
			return false;

		m_iId = pFound-m_dRowIDs.Begin();
		return true;
	}
}

template <>
void CachedIterator_T<true>::Add ( RowID_t tRowID )
{
	if ( m_tMinRowID==INVALID_ROWID )
	{
		m_tMinRowID = tRowID;
		m_tMaxRowID = tRowID;
	}
	else
	{
		m_tMinRowID = Min ( m_tMinRowID, tRowID );
		m_tMaxRowID = Max ( m_tMaxRowID, tRowID );
	}

	m_tBitmap.BitSet(tRowID);
}

template<>
void CachedIterator_T<false>::Add ( RowID_t tRowID )
{
	m_dRowIDs.Add(tRowID);
}

template<>
bool CachedIterator_T<true>::Finalize()
{
	m_tRowID = m_tMinRowID;
	return m_tMinRowID!=INVALID_ROWID;
}

template<>
bool CachedIterator_T<false>::Finalize()
{
	m_dRowIDs.Sort();
	return !m_dRowIDs.IsEmpty();
}

template <>
bool CachedIterator_T<true>::ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + m_dCollected.GetLength();
	RowID_t * pRowID = pRowIdStart;

	while ( m_tRowID<=m_tMaxRowID && pRowID<pRowIdMax )
	{
		if ( m_tBitmap.BitGet(m_tRowID) )
			*pRowID++ = m_tRowID;

		m_tRowID++;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <>
bool CachedIterator_T<false>::ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dRowIDs.Begin() + m_iId;
	RowID_t * pRowID = Min ( pRowIdStart+m_dCollected.GetLength(), m_dRowIDs.End() );
	m_iId += pRowID-pRowIdStart;

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupValues_T : public CachedIterator_T<BITMAP>
{
	using BASE = CachedIterator_T<BITMAP>;

public:
						RowidIterator_LookupValues_T ( const SphAttr_t * pValues, int nValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }

private:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocidListReader_c	m_tFilterReader;
	bool				m_bFirstTime = true;

	bool				Fill();
};

template <bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupValues_T<ROWID_LIMITS, BITMAP>::RowidIterator_LookupValues_T ( const SphAttr_t * pValues, int nValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_tLookupReader ( pDocidLookup )
	, m_tFilterReader ( pValues, nValues )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bFirstTime )
	{
		m_bFirstTime = false;
		if ( !Fill() )
			return false;
	}

	return BASE::ReturnRowIdChunk(dRowIdBlock);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t	tLookupDocID = 0;
	DocID_t	tFilterDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;
	bool bHaveFilterDocs = m_tFilterReader.ReadDocID(tFilterDocID);
	bool bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
	m_iProcessed += bHaveFilterDocs ? 1 : 0;
	m_iProcessed += bHaveLookupDocs ? 1 : 0;

	while ( bHaveFilterDocs && bHaveLookupDocs )
	{
		if ( tFilterDocID < tLookupDocID )
		{
			m_tFilterReader.HintDocID(tLookupDocID);
			bHaveFilterDocs = m_tFilterReader.ReadDocID ( tFilterDocID );
		}
		else if ( tFilterDocID > tLookupDocID )
		{
			m_tLookupReader.HintDocID(tFilterDocID);
			bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
		}
		else
		{
			// lookup reader can have duplicates; filter reader can't have duplicates
			// advance only the lookup reader
			if ( ROWID_LIMITS )
			{
				if ( tLookupRowID>=m_tBoundaries.m_tMinRowID && tLookupRowID<=m_tBoundaries.m_tMaxRowID )
					BASE::Add(tLookupRowID);
			}
			else
				BASE::Add(tLookupRowID);

			bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
		}

		m_iProcessed++;
	}

	return BASE::Finalize();
}

//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRange_T : public CachedIterator_T<BITMAP>
{
	using BASE = CachedIterator_T<BITMAP>;

public:
						RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }

protected:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocID_t				m_tMinValue {0};
	DocID_t				m_tMaxValue {0};
	bool				m_bFirstTime = true;

private:
	bool				Fill();
};

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_tLookupReader ( pDocidLookup )
	, m_tMinValue ( tMinValue )
	, m_tMaxValue ( tMaxValue )
{
	if_const ( !OPEN_LEFT )
		m_tLookupReader.HintDocID ( tMinValue );

	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bFirstTime )
	{
		m_bFirstTime = false;
		if ( !Fill() )
			return false;
	}

	return BASE::ReturnRowIdChunk(dRowIdBlock);
}

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		m_iProcessed++;

		if ( !OPEN_LEFT && ( tLookupDocID < m_tMinValue || ( !HAS_EQUAL_MIN && tLookupDocID==m_tMinValue ) ) )
			continue;

		if ( !OPEN_RIGHT && ( tLookupDocID > m_tMaxValue || ( !HAS_EQUAL_MAX && tLookupDocID==m_tMaxValue ) ) )
			break;

		if ( ROWID_LIMITS )
		{
			if ( tLookupRowID >= m_tBoundaries.m_tMinRowID && tLookupRowID <= m_tBoundaries.m_tMaxRowID )
				BASE::Add(tLookupRowID);
		}
		else
			BASE::Add(tLookupRowID);
	}

	return BASE::Finalize();
}

//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRangeExclude_T : public RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>
{
	using BASE = RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>;

public:
			RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries );

	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;

private:
	bool	m_bLeft {true};
};


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP> ( tMinValue, tMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries )
{
	if ( OPEN_LEFT && !OPEN_RIGHT )
		this->m_tLookupReader.HintDocID ( this->m_tMaxValue );

	m_bLeft = OPEN_RIGHT || ( !OPEN_LEFT && !OPEN_RIGHT ); 
}


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if_const ( OPEN_LEFT && OPEN_RIGHT )
		return false;

	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && BASE::m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		BASE::m_iProcessed++;

		if ( m_bLeft )
		{
			// use everything ending with m_tMinValue
			if ( tLookupDocID > BASE::m_tMinValue || ( HAS_EQUAL_MIN && tLookupDocID==BASE::m_tMinValue ) )
			{
				// switch to right interval
				if_const ( !OPEN_LEFT && !OPEN_RIGHT )
				{
					m_bLeft = false;
					this->m_tLookupReader.HintDocID ( this->m_tMaxValue );
					continue;
				}

				return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
			}
		}
		else
		{
			// use everything starting from m_tMaxValue
			if ( tLookupDocID < BASE::m_tMaxValue || ( HAS_EQUAL_MAX && tLookupDocID==BASE::m_tMaxValue ) )
				continue;
		}

		if ( ROWID_LIMITS )
		{
			if ( tLookupRowID>=BASE::m_tBoundaries.m_tMinRowID && tLookupRowID<=BASE::m_tBoundaries.m_tMaxRowID )
				*pRowID++ = tLookupRowID;
		}
		else
			*pRowID++ = tLookupRowID;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

//////////////////////////////////////////////////////////////////////////

static RowIdBlock_t DoRowIdFiltering ( const RowIdBlock_t & dRowIdBlock, const RowIdBoundaries_t & tBoundaries, CSphVector<RowID_t> & dCollected )
{
	RowID_t tMinSpanRowID = dRowIdBlock.First();
	RowID_t tMaxSpanRowID = dRowIdBlock.Last();

	if ( tMaxSpanRowID < tBoundaries.m_tMinRowID || tMinSpanRowID > tBoundaries.m_tMaxRowID )
		return {};

	dCollected.Resize ( dRowIdBlock.GetLength() );

	RowID_t * pRowIdStart = dCollected.Begin();
	RowID_t * pRowID = pRowIdStart;
	for ( auto i : dRowIdBlock )
	{
		if ( i>=tBoundaries.m_tMinRowID && i<=tBoundaries.m_tMaxRowID )
			*pRowID++ = i;
	}

	return { pRowIdStart, pRowID-pRowIdStart };
}

//////////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Intersect_T : public SecondaryIndexIterator_c
{
public:
				RowidIterator_Intersect_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries = nullptr );
				~RowidIterator_Intersect_T() override;

	bool		HintRowID ( RowID_t tRowID ) override;
	bool		GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t		GetNumProcessed() const override;

private:
	struct IteratorState_t
	{
		T *					m_pIterator = nullptr;

		const RowID_t *		m_pRowID = nullptr;
		const RowID_t *		m_pRowIDMax = nullptr;

		RowIdBoundaries_t	m_tBoundaries;
		CSphVector<RowID_t> m_dCollected;

		FORCE_INLINE bool	RewindTo  ( RowID_t tRowID );
		FORCE_INLINE bool	WarmupDocs();
		FORCE_INLINE bool	WarmupDocs ( RowID_t tRowID );
	};

	CSphFixedVector<IteratorState_t>	m_dIterators {0};

	FORCE_INLINE bool	AdvanceIterators();
	FORCE_INLINE bool	Advance ( int iIterator, RowID_t tRowID );
};

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::IteratorState_t::WarmupDocs()
{
	assert(m_pIterator);
	assert ( !ROWID_LIMITS );	// we assume that underlying iterators do the filtering

	RowIdBlock_t dRowIdBlock;
	if ( !m_pIterator->GetNextRowIdBlock(dRowIdBlock) )
	{
		m_pRowID = m_pRowIDMax = nullptr;
		return false;
	}

	m_pRowID = dRowIdBlock.Begin();
	m_pRowIDMax = m_pRowID+dRowIdBlock.GetLength();

	return true;
}

template <>
bool RowidIterator_Intersect_T<columnar::BlockIterator_i,true>::IteratorState_t::WarmupDocs()
{
	assert(m_pIterator);

	columnar::Span_T<uint32_t> dSpan;
	if ( !m_pIterator->GetNextRowIdBlock(dSpan) )
	{
		m_pRowID = m_pRowIDMax = nullptr;
		return false;
	}

	RowID_t tMinSpanRowID = dSpan.front();
	RowID_t tMaxSpanRowID = dSpan.back();

	RowIdBlock_t dRowIdBlock = { (RowID_t *)dSpan.begin(), (int64_t)dSpan.size() };

	// we need additional filtering only on first and last blocks
	// per-block filtering is performed inside MCL
	if ( tMinSpanRowID < m_tBoundaries.m_tMinRowID || tMaxSpanRowID > m_tBoundaries.m_tMaxRowID )
		dRowIdBlock = DoRowIdFiltering ( dRowIdBlock, m_tBoundaries, m_dCollected );

	m_pRowID = (const RowID_t *)dRowIdBlock.begin();
	m_pRowIDMax = (const RowID_t *)dRowIdBlock.end();

	return true;
}

template <>
bool RowidIterator_Intersect_T<columnar::BlockIterator_i,false>::IteratorState_t::WarmupDocs()
{
	assert(m_pIterator);

	columnar::Span_T<uint32_t> dRowIdBlock;
	if ( !m_pIterator->GetNextRowIdBlock(dRowIdBlock) )
	{
		m_pRowID = m_pRowIDMax = nullptr;
		return false;
	}

	m_pRowID = (const RowID_t *)dRowIdBlock.begin();
	m_pRowIDMax = (const RowID_t *)dRowIdBlock.end();

	return true;
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::IteratorState_t::WarmupDocs ( RowID_t tRowID )
{
	if ( !m_pIterator->HintRowID(tRowID) )
		return false;

	return WarmupDocs();
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::IteratorState_t::RewindTo ( RowID_t tRowID )
{
	if ( m_pRowID>=m_pRowIDMax || tRowID>*(m_pRowIDMax-1) )
	{
		if ( !WarmupDocs(tRowID) )
			return false;
	}

	const RowID_t * pRowID = m_pRowID;

	while ( true )
	{
		while ( pRowID < m_pRowIDMax && *pRowID < tRowID )
			pRowID++;

		if ( pRowID<m_pRowIDMax )
			break;

		if ( !WarmupDocs() )
			return false;

		pRowID = m_pRowID;
	}

	m_pRowID = pRowID;

	return true;
}

template <typename T, bool ROWID_LIMITS>
RowidIterator_Intersect_T<T,ROWID_LIMITS>::RowidIterator_Intersect_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries )
	: m_dIterators(iNumIterators)
{
	ARRAY_FOREACH ( i, m_dIterators )
	{
		auto & tIterator = m_dIterators[i];
		tIterator.m_pIterator = ppIterators[i];
		if ( pBoundaries )
			tIterator.m_tBoundaries = *pBoundaries;
	}

	m_dIterators[0].WarmupDocs();
}

template <typename T, bool ROWID_LIMITS>
RowidIterator_Intersect_T<T,ROWID_LIMITS>::~RowidIterator_Intersect_T()
{
	for ( auto & i : m_dIterators )
		SafeDelete ( i.m_pIterator );
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::HintRowID ( RowID_t tRowID )
{
	return m_dIterators[0].RewindTo(tRowID);
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	IteratorState_t & tFirst = m_dIterators[0];

	// we assume that iterators are sorted from most selective to least selective
	while ( pRowID<pRowIdMax )
	{
		if ( !tFirst.m_pRowID )
			break;

		if ( !AdvanceIterators() )
		{
			tFirst.m_pRowID = nullptr;
			break;
		}

		*pRowID++ = *tFirst.m_pRowID;

		tFirst.m_pRowID++;
		if ( tFirst.m_pRowID>=tFirst.m_pRowIDMax && !tFirst.WarmupDocs() )
		{
			tFirst.m_pRowID = nullptr;
			break;
		}
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::AdvanceIterators()
{
	IteratorState_t & tFirst = m_dIterators[0];
	RowID_t tMaxRowID = *tFirst.m_pRowID;
	for ( int i=1; i < m_dIterators.GetLength(); i++ )
	{
		auto & tState = m_dIterators[i];
		if ( !tState.m_pRowID && !tState.WarmupDocs(tMaxRowID) )
			return false;

		if ( *tState.m_pRowID==tMaxRowID )
			continue;

		if ( !tState.RewindTo(tMaxRowID) )
			return false;

		if ( *tState.m_pRowID>tMaxRowID )
		{
			if ( !tFirst.RewindTo ( *tState.m_pRowID ) )
				return false;

			tMaxRowID = *tFirst.m_pRowID;
			i = 0;
		}
	}

	return true;
}

template <typename T, bool ROWID_LIMITS>
int64_t RowidIterator_Intersect_T<T,ROWID_LIMITS>::GetNumProcessed() const
{
	int64_t iTotal = 0;
	for ( auto i : m_dIterators )
		iTotal += i.m_pIterator->GetNumProcessed();

	return iTotal;
}

/////////////////////////////////////////////////////////////////////

template <bool ROWID_LIMITS>
class RowidIterator_Wrapper_T : public RowidIterator_i
{
public:
			RowidIterator_Wrapper_T ( columnar::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool	HintRowID ( RowID_t tRowID ) override { return m_pIterator->HintRowID(tRowID); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t	GetNumProcessed() const override { return m_pIterator->GetNumProcessed(); }

private:
	std::unique_ptr<columnar::BlockIterator_i> m_pIterator;
	RowIdBoundaries_t	m_tBoundaries;
	CSphVector<RowID_t>	m_dCollected;
};

template <bool ROWID_LIMITS>
RowidIterator_Wrapper_T<ROWID_LIMITS>::RowidIterator_Wrapper_T ( columnar::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries )
	: m_pIterator ( pIterator )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <>
bool RowidIterator_Wrapper_T<true>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	columnar::Span_T<uint32_t> dSpan;
	if ( !m_pIterator->GetNextRowIdBlock(dSpan) )
		return false;

	dRowIdBlock = { (RowID_t *)dSpan.begin(), (int64_t)dSpan.size() };
	if ( !dSpan.size() )
		return true;

	RowID_t tMinSpanRowID = dSpan.front();
	RowID_t tMaxSpanRowID = dSpan.back();

	// we need additional filtering only on first and last blocks
	// per-block filtering is performed inside MCL
	if ( tMinSpanRowID < m_tBoundaries.m_tMinRowID || tMaxSpanRowID > m_tBoundaries.m_tMaxRowID )
		dRowIdBlock = DoRowIdFiltering ( dRowIdBlock, m_tBoundaries, m_dCollected );

	return true;
}

template <>
bool RowidIterator_Wrapper_T<false>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	columnar::Span_T<uint32_t> dSpan;
	if ( !m_pIterator->GetNextRowIdBlock(dSpan) )
		return false;

	dRowIdBlock = { (RowID_t *)dSpan.begin(), (int64_t)dSpan.size() };
	return true;
}

//////////////////////////////////////////////////////////////////////////
static bool NeedBitmapStorage ( int64_t iRsetSize, DWORD uTotalDocs )
{
	return float(iRsetSize)/uTotalDocs > 0.05f;
}

#define DECL_CREATEVALUES( _, n, params ) case n: return new RowidIterator_LookupValues_T<!!( n & 2 ), !!( n & 1 )> params;
#define DECL_CREATERANGEEX( _, n, params ) case n: return new RowidIterator_LookupRangeExclude_T<!!( n & 32 ), !!( n & 16 ), !!( n & 8 ), !!( n & 4 ), !!( n & 2 ), !!( n & 1 )> params;
#define DECL_CREATERANGE( _, n, params ) case n: return new RowidIterator_LookupRange_T<!!( n & 32 ), !!( n & 16 ), !!( n & 8 ), !!( n & 4 ), !!( n & 2 ), !!( n & 1 )> params;

static RowidIterator_i * CreateIterator ( const CSphFilterSettings & tFilter, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
{
	if ( !HaveIndex ( tFilter.m_sAttrName ) )
		return nullptr;

	bool bBitmap = NeedBitmapStorage ( iRsetEstimate, uTotalDocs );

	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
		{
			int iIndex = !!pBoundaries * 2 + bBitmap;
			switch ( iIndex )
			{
				BOOST_PP_REPEAT ( 4, DECL_CREATEVALUES, ( tFilter.GetValueArray(), tFilter.GetNumValues(), iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
				default: assert ( 0 && "Internal error" ); return nullptr;
			}
		}
		break;

	case SPH_FILTER_RANGE:
		{
			int iIndex = tFilter.m_bHasEqualMin * 32 + tFilter.m_bHasEqualMax * 16 + tFilter.m_bOpenLeft * 8 + tFilter.m_bOpenRight * 4 + !!pBoundaries * 2 + bBitmap;
			if ( tFilter.m_bExclude )
				switch ( iIndex )
				{
					BOOST_PP_REPEAT ( 64, DECL_CREATERANGEEX, ( tFilter.m_iMinValue, tFilter.m_iMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
					default: assert ( 0 && "Internal error" ); return nullptr;
				}
			else
				switch ( iIndex )
				{
					BOOST_PP_REPEAT ( 64, DECL_CREATERANGE, ( tFilter.m_iMinValue, tFilter.m_iMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
					default: assert ( 0 && "Internal error" ); return nullptr;
				}
		}
		break;

	default:
		break;
	}

	return nullptr;
}

#undef DECL_CREATEVALUES
#undef DECL_CREATERANGEEX
#undef DECL_CREATERANGE

struct IndexWithEstimate_t : public SecondaryIndexInfo_t
{
	bool	m_bEnabled {false};
	bool	m_bForce {false};
};


static bool NextSet ( CSphBitvec & dSet, const CSphVector<IndexWithEstimate_t> & dSecondaryIndexes )
{
	for ( int i = 0; i < dSet.GetBits(); i++ )
	{
		if ( dSecondaryIndexes[i].m_bForce )
			continue;

		bool bState = dSet.BitGet(i);
		if ( bState )
			dSet.BitClear(i);
		else
		{
			dSet.BitSet(i);
			return true;
		}
	}

	return false;
}


class CostEstimate_c
{
public:
	float LookupRead ( int64_t iDocs ) const
	{
		return m_fLookupRead*float(iDocs)*SCALE;
	}

	float Filter ( int64_t iDocs, int nFilters ) const
	{
		return m_fFilter*nFilters*float(iDocs)*SCALE;
	}

	float Fullscan ( int64_t iDocs ) const
	{
		return m_fFullscan*float(iDocs)*SCALE;
	}

	float IndexIntersect ( int64_t iDocs ) const
	{
		return m_fIndexIntersect*float(iDocs)*SCALE;
	}

	float IndexFilter ( int64_t iDocs, int nFilters ) const
	{
		return m_fIndexFilter*nFilters*float(iDocs)*SCALE;
	}

private:
	static constexpr float SCALE = 1.0f/1000000.0f;
	static constexpr float m_fLookupRead = 100.0f;
	static constexpr float m_fFilter = 5.0f;
	static constexpr float m_fFullscan = 5.0f;
	static constexpr float m_fIndexIntersect = 8.0f;
	static constexpr float m_fIndexFilter = 150.0f;
};


static float CalcQueryCost ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<IndexWithEstimate_t> & dSecondaryIndexes, const CostEstimate_c & tCost, int64_t iTotalDocs )
{
	float fCost = 0.0f;
	int64_t iDocsProcessedByIndexes = 0;
	float fTotalIndexProbability = 1.0f;

	int nEnabled = 0;
	for ( const auto & tIndex : dSecondaryIndexes )
	{
		if ( !tIndex.m_bEnabled )
			continue;

		nEnabled++;
		iDocsProcessedByIndexes += tIndex.m_iRsetEstimate;

		float fIndexProbability = float(tIndex.m_iRsetEstimate) / iTotalDocs;
		fTotalIndexProbability *= fIndexProbability;
	}

	if ( !nEnabled )
	{
		fCost += tCost.Fullscan ( iTotalDocs );
		fCost += tCost.Filter ( iTotalDocs, dFilters.GetLength() );
	}
	else
	{
		fCost += tCost.LookupRead ( iDocsProcessedByIndexes );

		if ( nEnabled>1 )
		{
			fCost += tCost.IndexIntersect ( iDocsProcessedByIndexes );
			fCost += tCost.IndexFilter ( uint64_t(fTotalIndexProbability*iTotalDocs), dFilters.GetLength()-nEnabled+1 );

		} else
		{
			fCost += tCost.IndexFilter ( iDocsProcessedByIndexes, dFilters.GetLength()-nEnabled+1 );
		}
	}

	return fCost;
}

static bool g_bEnabledSI = false;
void SetSecondaryIndexDefault ( bool bEnabled )
{
	g_bEnabledSI = bEnabled;
}

bool GetSecondaryIndexDefault ()
{
	return g_bEnabledSI;
}

static CSphVector<SecondaryIndexInfo_t> SelectIterators ( const CSphVector<CSphFilterSettings> & dFilters, bool bFilterTreeEmpty, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c * pHistograms, const SelectSI_c * pCheck, float & fBestCost )
{
	fBestCost = FLT_MAX;

	// no iterators with OR queries
	CSphVector<SecondaryIndexInfo_t> dEnabledIndexes;
	if ( !bFilterTreeEmpty || !pHistograms )
		return dEnabledIndexes;

	bool bHaveUseHint = false;
	ARRAY_FOREACH_COND ( i, dHints, !bHaveUseHint )
		bHaveUseHint = dHints[i].m_eHint==INDEX_HINT_USE;
	
	CSphVector<IndexWithEstimate_t> dSecondaryIndexes;
	ARRAY_FOREACH ( i, dFilters )
	{
		const CSphFilterSettings & tFilter = dFilters[i];

		if ( !pCheck->IsEnabled ( tFilter ) )
			continue;

		const Histogram_i * pHistogram = pHistograms->Get ( tFilter.m_sAttrName );
		if ( !pHistogram )
			continue;

		IndexHint_e * pHint = nullptr;
		ARRAY_FOREACH_COND ( iHint, dHints, !pHint )
			if ( dHints[iHint].m_sIndex==tFilter.m_sAttrName )
				pHint = &dHints[iHint].m_eHint;

		if ( pHint && *pHint==INDEX_HINT_IGNORE )
			continue;

		if ( bHaveUseHint && !pHint )
			continue;

		// default secondary index is disabled
		// but FORCE INDEX (name) enables it
		// set global secondary_indexes=1 allows to use optimizer
		if ( !g_bEnabledSI && ( !pHint || ( pHint && *pHint!=INDEX_HINT_FORCE ) ) )
			continue;

		IndexWithEstimate_t tNewIndex;

		HistogramRset_t tEstimate;
		bool bUsable = pHistogram->EstimateRsetSize ( tFilter, tEstimate );
		if ( bUsable )
		{
			tNewIndex.m_iRsetEstimate = tEstimate.m_iTotal;
			tNewIndex.m_iFilterId = i;
			tNewIndex.m_bForce = pHint && *pHint==INDEX_HINT_FORCE;

			dSecondaryIndexes.Add ( tNewIndex );

			//sphInfo ( "filter %d, est " INT64_FMT " (%u), %.3f, count %d, range %.3f, size %d", tNewIndex.m_iFilterId, tNewIndex.m_iRsetEstimate, pHistograms->GetNumValues(), (float)tNewIndex.m_iRsetEstimate/pHistograms->GetNumValues(), tEstimate.m_iCount, tEstimate.m_fRangeSize, pHistogram->GetSize() ); // !COMMIT
		}
	}

	int nAvailableIndexes = dSecondaryIndexes.GetLength();
	if ( !nAvailableIndexes )
		return dEnabledIndexes;

	CostEstimate_c tCost;
	//sphInfo ( "scan cost: %.3f", CalcQueryCost ( dFilters, CSphVector<IndexWithEstimate_t>(), tCost, pHistograms->GetNumValues() ) );  // !COMMIT

	CSphBitvec dEnabled ( nAvailableIndexes );
	CSphBitvec dBestEnabled ( nAvailableIndexes );

	for ( int i = 0; i < dEnabled.GetBits(); i++ )
		if ( dSecondaryIndexes[i].m_bForce )
			dEnabled.BitSet(i);

	const int MAX_TRIES = 1024;
	for ( int iTry = 0; iTry < MAX_TRIES; iTry++ )
	{
		for ( int i = 0; i < dEnabled.GetBits(); i++ )
			dSecondaryIndexes[i].m_bEnabled = dEnabled.BitGet(i);

		float fCost = CalcQueryCost ( dFilters, dSecondaryIndexes, tCost, pHistograms->GetNumValues() );
		//sphInfo ( "filter %x, cost %.3f(%.3f)", *dEnabled.Begin(), fCost, ( fBestCost==FLT_MAX ? 0.0 : fBestCost ) ); // !COMMIT
		if ( fCost < fBestCost )
		{
			dBestEnabled = dEnabled;
			fBestCost = fCost;
		}

		if ( !NextSet ( dEnabled, dSecondaryIndexes ) )
			break;
	}

	for ( int i = 0; i < dBestEnabled.GetBits(); i++ )
	{
		if ( dBestEnabled.BitGet(i) )
		{
			SecondaryIndexInfo_t & tInfo = dEnabledIndexes.Add();
			tInfo.m_iFilterId = dSecondaryIndexes[i].m_iFilterId;
			tInfo.m_iRsetEstimate = dSecondaryIndexes[i].m_iRsetEstimate;
		}
	}

	dEnabledIndexes.Sort ( bind ( &SecondaryIndexInfo_t::m_iFilterId ) );

	return dEnabledIndexes;
}

CSphVector<SecondaryIndexInfo_t> SelectIterators ( const CSphVector<CSphFilterSettings> & dFilters, bool bFilterTreeEmpty, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c * pHistograms, const SelectSI_c * pCheck )
{
	float fCost = FLT_MAX;
	return SelectIterators ( dFilters, bFilterTreeEmpty, dHints, pHistograms, pCheck, fCost );
}

static bool UpdateModifiedFilters ( const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, const CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphFilterSettings * pRowIdFilter )
{
	dModifiedFilters.Resize(0);
	ARRAY_FOREACH ( i, dFilters )
	{
		// if we have a rowid filter, we assume that the iterator handles it. so we no longer need it
		if ( !dEnabledIndexes.any_of ( [i] ( const SecondaryIndexInfo_t & tInfo ) { return tInfo.m_iFilterId==i; } ) && &dFilters[i]!=pRowIdFilter )
			dModifiedFilters.Add ( dFilters[i] );
	}

	return dFilters.GetLength()!=dModifiedFilters.GetLength();
}

float GetEnabledSecondaryIndexes ( CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c & tHistograms, const SelectSI_c * pCheck )
{
	float fBestCost = FLT_MAX;
	dEnabledIndexes = SelectIterators ( dFilters, dFilterTree.IsEmpty(), dHints, &tHistograms, pCheck, fBestCost );
	return fBestCost;
}

static const CSphFilterSettings * GetRowIdFilter ( const CSphVector<CSphFilterSettings> & dFilters, RowID_t uTotalDocs, RowIdBoundaries_t & tRowidBounds )
{
	const CSphFilterSettings * pRowIdFilter = nullptr;
	for ( const auto & tFilter : dFilters )
	{
		if ( tFilter.m_sAttrName=="@rowid" )
		{
			pRowIdFilter = &tFilter;
			break;
		}
	}

	if ( pRowIdFilter )
		tRowidBounds = GetFilterRowIdBoundaries ( *pRowIdFilter, uTotalDocs );

	return pRowIdFilter;
}

RowidIterator_i * CreateFilteredIterator ( const CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const BYTE * pDocidLookup, uint32_t uTotalDocs )
{
	bFiltersChanged = false;
	if ( dEnabledIndexes.IsEmpty() )
		return nullptr;

	RowIdBoundaries_t tBoundaries;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, uTotalDocs, tBoundaries );

	CSphVector<RowidIterator_i *> dIterators;
	for ( auto & i : dEnabledIndexes )
	{
		RowidIterator_i * pIterator = CreateIterator ( dFilters[i.m_iFilterId], i.m_iRsetEstimate, uTotalDocs, pDocidLookup, pRowIdFilter ? &tBoundaries : nullptr );
		if ( pIterator )
			dIterators.Add ( pIterator );
	}

	if ( !dIterators.GetLength() )
		return nullptr;

	bFiltersChanged = UpdateModifiedFilters ( dFilters, dModifiedFilters, dEnabledIndexes, pRowIdFilter );

	if ( dIterators.GetLength()==1 )
		return dIterators[0];

	return new RowidIterator_Intersect_T<RowidIterator_i,false> ( dIterators.Begin(), dIterators.GetLength() );
}


RowidIterator_i * CreateIteratorIntersect ( CSphVector<RowidIterator_i*> & dIterators, const RowIdBoundaries_t * pBoundaries )
{
	if ( pBoundaries )
		return new RowidIterator_Intersect_T<RowidIterator_i,true> ( dIterators.Begin(), dIterators.GetLength(), pBoundaries );
	else
		return new RowidIterator_Intersect_T<RowidIterator_i,false> ( dIterators.Begin(), dIterators.GetLength() );
}


RowidIterator_i * CreateIteratorWrapper ( columnar::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries )
{
	if ( pBoundaries )
		return new RowidIterator_Wrapper_T<true> ( pIterator, pBoundaries );
	else
		return new RowidIterator_Wrapper_T<false> ( pIterator );
}


RowidIterator_i * CreateIteratorIntersect ( std::vector<columnar::BlockIterator_i *> & dIterators, const RowIdBoundaries_t * pBoundaries )
{
	if ( pBoundaries )
		return new RowidIterator_Intersect_T<columnar::BlockIterator_i, true> ( &dIterators[0], (int)dIterators.size(), pBoundaries );
	else
		return new RowidIterator_Intersect_T<columnar::BlockIterator_i, false> ( &dIterators[0], (int)dIterators.size() );
}

//////////////////////////////////////////////////////////////////////////

DocidLookupWriter_c::DocidLookupWriter_c ( DWORD nDocs )
	: m_nDocs ( nDocs )
{}


DocidLookupWriter_c::~DocidLookupWriter_c()
{
	SafeDelete ( m_pWriter );
}


bool DocidLookupWriter_c::Open ( const CSphString & sFilename, CSphString & sError )
{
	assert ( !m_pWriter );
	m_pWriter = new CSphWriter;

	if ( !m_pWriter->OpenFile ( sFilename, sError ) )
		return false;

	m_pWriter->PutDword ( m_nDocs );
	m_pWriter->PutDword ( DOCS_PER_LOOKUP_CHECKPOINT );

	m_tCheckpointStart = m_pWriter->GetPos();
	m_pWriter->PutOffset ( 0 );	// reserve space for max docid

	int nCheckpoints = (m_nDocs+DOCS_PER_LOOKUP_CHECKPOINT-1)/DOCS_PER_LOOKUP_CHECKPOINT;
	m_dCheckpoints.Reset ( nCheckpoints );
	for ( int i = 0; i < nCheckpoints; i++ )
	{
		// reserve space for checkpoints
		m_pWriter->PutOffset(0);
		m_pWriter->PutOffset(0);
	}

	return true;
}


void DocidLookupWriter_c::AddPair ( const DocidRowidPair_t & tPair )
{
	assert ( tPair.m_tDocID>=m_tLastDocID );

	if ( !(m_iProcessed % DOCS_PER_LOOKUP_CHECKPOINT) )
	{
		m_dCheckpoints[m_iCheckpoint].m_tBaseDocID = tPair.m_tDocID;
		m_dCheckpoints[m_iCheckpoint].m_tOffset = m_pWriter->GetPos();
		m_iCheckpoint++;

		// no need to store docid for 1st entry
		m_pWriter->PutDword ( tPair.m_tRowID );
	}
	else
	{
		m_pWriter->ZipOffset ( tPair.m_tDocID-m_tLastDocID );
		m_pWriter->PutDword ( tPair.m_tRowID );
	}

	m_tLastDocID = tPair.m_tDocID;
	m_iProcessed++;
}


bool DocidLookupWriter_c::Finalize ( CSphString & sError )
{
	m_pWriter->Flush();
	m_pWriter->SeekTo ( m_tCheckpointStart );
	m_pWriter->PutOffset ( m_tLastDocID );
	for ( const auto & i : m_dCheckpoints )
	{
		m_pWriter->PutOffset ( i.m_tBaseDocID );
		m_pWriter->PutOffset ( i.m_tOffset );
	}

	m_pWriter->CloseFile();
	if ( m_pWriter->IsError() )
	{
		sError = "error writing .SPT";
		return false;
	}

	return true;
}


CSphWriter & DocidLookupWriter_c::GetWriter()
{
	assert ( m_pWriter );
	return *m_pWriter;
}


bool WriteDocidLookup ( const CSphString & sFilename, const VecTraits_T<DocidRowidPair_t> & dLookup, CSphString & sError )
{
	DocidLookupWriter_c tWriter ( dLookup.GetLength() );
	if ( !tWriter.Open ( sFilename, sError ) )
		return false;

	for ( const auto & i : dLookup )
		tWriter.AddPair(i);

	if ( !tWriter.Finalize ( sError ) )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////

LookupReader_c::LookupReader_c ( const BYTE * pData )
{
	SetData ( pData );
}


void LookupReader_c::SetData ( const BYTE * pData )
{
	m_pData = pData;

	if ( !pData )
		return;

	const BYTE * p = pData;
	m_nDocs = *(const DWORD*)p;
	p += sizeof(DWORD);
	m_nDocsPerCheckpoint = *(const DWORD*)p;
	p += sizeof(DWORD);
	m_tMaxDocID = *(const DocID_t*)p;
	p += sizeof(DocID_t);

	m_nCheckpoints = (m_nDocs+m_nDocsPerCheckpoint-1)/m_nDocsPerCheckpoint;
	m_pCheckpoints = (const DocidLookupCheckpoint_t *)p;
}

//////////////////////////////////////////////////////////////////////////

LookupReaderIterator_c::LookupReaderIterator_c ( const BYTE * pData )
{
	SetData(pData);
}


void LookupReaderIterator_c::SetData ( const BYTE * pData )
{
	LookupReader_c::SetData(pData);
	SetCheckpoint ( m_pCheckpoints );
}


class RowidEmptyIterator_c : public RowidIterator_i
{
public:
	RowidEmptyIterator_c () = default;
	bool	HintRowID ( RowID_t tRowID ) { return false; }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) { return false; }
	int64_t	GetNumProcessed() const { return 0; }
};

class RowidIterator_c : public RowidIterator_i
{
public:
	RowidIterator_c ( SI::RowidIterator_i * pIterator )
		: m_pIterator ( pIterator )
	{}

	bool	HintRowID ( RowID_t tRowID ) { return m_pIterator->HintRowID ( tRowID ); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	int64_t	GetNumProcessed() const { return m_pIterator->GetNumProcessed(); }

private:
	std::unique_ptr<SI::RowidIterator_i> m_pIterator;
};

bool RowidIterator_c::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	columnar::Span_T<uint32_t> dRows;
	if ( !m_pIterator->GetNextRowIdBlock ( dRows ) )
		return false;

	dRowIdBlock = RowIdBlock_t ( (RowID_t *)dRows.data(), (int64_t)dRows.size() );
	return true;
}

#define CIDX_ROWS 128

class RowidIteratorNot_c : public RowidIterator_i
{
public:
	RowidIteratorNot_c ( RowidIterator_i * pIterator, RowID_t uRowsCount )
		: m_uRowsCount ( uRowsCount )
		, m_pExcludedIt ( pIterator )
	{
		m_dRows.Reserve ( CIDX_ROWS );
	}

	bool	HintRowID ( RowID_t tRowID ) { return m_pExcludedIt->HintRowID ( tRowID ); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	int64_t	GetNumProcessed() const { return m_pExcludedIt->GetNumProcessed(); }

private:
	RowID_t m_uRowsCount { 0 };
	RowID_t m_uCurRow { 0 };
	bool m_bStopped = false;
	bool m_bPass = false;

	bool m_bExcludedStarted = false;
	bool m_bExcludedStopped = false;
	int m_iExcludedCur = 0;
	RowIdBlock_t m_dExcludedRows;
	std::unique_ptr<RowidIterator_i> m_pExcludedIt;

	CSphVector<RowID_t> m_dRows;

	void CopyTail ();
};

void RowidIteratorNot_c::CopyTail()
{
	int iCount = Min ( m_uRowsCount-m_uCurRow, CIDX_ROWS-m_dRows.GetLength() );
	RowID_t * pRow = m_dRows.AddN ( iCount );

	while ( iCount>0 )
	{
		*pRow++ = m_uCurRow++;
		iCount--;
	}
}

bool RowidIteratorNot_c::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bStopped )
		return false;

	m_dRows.Resize ( 0 );

	if ( !m_bPass )
	{
		// warmup
		if ( !m_bExcludedStarted )
		{
			m_bExcludedStopped = !m_pExcludedIt->GetNextRowIdBlock ( m_dExcludedRows );
			m_bExcludedStarted = true;
			m_iExcludedCur = 0;
		}

		for ( ; m_uCurRow<m_uRowsCount && m_dRows.GetLength()<CIDX_ROWS; )
		{
			// cases if filter values is over
			if ( m_iExcludedCur>=m_dExcludedRows.GetLength() || m_bExcludedStopped )
			{
				// fetch more values
				if ( !m_bExcludedStopped )
				{
					m_bExcludedStopped = !m_pExcludedIt->GetNextRowIdBlock ( m_dExcludedRows );
					m_iExcludedCur = 0;
				}

				// copy tail rows
				if ( m_bExcludedStopped )
				{
					m_bPass = true;
					break;
				}
			}

			assert ( m_dExcludedRows.GetLength() && m_iExcludedCur<m_dExcludedRows.GetLength() );
			RowID_t uNotRowid = m_dExcludedRows[m_iExcludedCur];

			// copy accepted with filter by excluded
			while ( m_uCurRow<uNotRowid && m_uCurRow<m_uRowsCount && m_dRows.GetLength()<CIDX_ROWS )
					m_dRows.Add ( m_uCurRow++ );

			// result vector full
			if ( m_uCurRow>=m_uRowsCount || m_dRows.GetLength()==CIDX_ROWS )
				break;

			// skip rejected till min accepted
			while ( m_iExcludedCur<m_dExcludedRows.GetLength() && m_dExcludedRows[m_iExcludedCur]<m_uCurRow )
				m_iExcludedCur++;

			// skip both while matched
			while ( m_iExcludedCur<m_dExcludedRows.GetLength() && m_uCurRow<m_uRowsCount && m_dExcludedRows[m_iExcludedCur]==m_uCurRow )
			{
				m_iExcludedCur++;
				m_uCurRow++;
			}
		}
	}

	if ( m_bPass )
		CopyTail();

	dRowIdBlock = m_dRows;

	if ( !m_dRows.GetLength() || m_uCurRow==m_uRowsCount )
		m_bStopped = true;

	return ( m_dRows.GetLength() );
}

struct IterQueueItem_t
{
	IterQueueItem_t() = default;

	static inline bool IsLess ( const IterQueueItem_t & tA, const IterQueueItem_t & tB )
	{
		return ( tA.m_dSrcRows[tA.m_iCur]<tB.m_dSrcRows[tB.m_iCur] );
	}

	RowID_t GetRow() const { return m_dSrcRows[m_iCur]; }

	int m_iCur = 0;
	columnar::Span_T<uint32_t> m_dSrcRows;
	SI::RowidIterator_i * m_pIt;
};


class RowsIterator_c : public RowidIterator_i
{
public:
	RowsIterator_c ( std::vector<SI::RowidIterator_i *> & dIt )
		: m_dIt ( std::move ( dIt ) )
		, m_tMerge ( m_dIt.size() )
	{
		m_dRows.Reserve ( CIDX_ROWS );
	}

	~RowsIterator_c()
	{
		for ( SI::RowidIterator_i * pIt : m_dIt )
			SafeDelete ( pIt );
	}

	bool	HintRowID ( RowID_t tRowID ) { return ( m_dIt.size()>0 ); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	int64_t	GetNumProcessed() const
	{
		uint64_t uCount = 0;
		for ( const auto * pIt : m_dIt )
			uCount += pIt->GetNumProcessed();

		return uCount;
	}

private:
	std::vector<SI::RowidIterator_i *> m_dIt;
	bool m_bStarted = false;

	CSphVector<RowID_t> m_dRows;

	CSphQueue<IterQueueItem_t, IterQueueItem_t> m_tMerge;
};

static void AdvanceQueue ( CSphQueue<IterQueueItem_t, IterQueueItem_t> & tMerge, IterQueueItem_t tTop )
{
	tTop.m_iCur++;
	if ( tTop.m_iCur<tTop.m_dSrcRows.size() )
	{
		tMerge.Push ( tTop );
		return;
	}

	bool bGotRows = tTop.m_pIt->GetNextRowIdBlock ( tTop.m_dSrcRows );
	if ( !bGotRows )
		return;

	tTop.m_iCur = 0;
	tMerge.Push ( tTop );
}

bool RowsIterator_c::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bStarted && !m_tMerge.GetLength() )
		return false;

	m_dRows.Resize ( 0 );

	// warmup
	if ( !m_bStarted )
	{
		m_bStarted = true;
		auto tIt = m_dIt.begin();
		while ( tIt!=m_dIt.end() )
		{
			IterQueueItem_t tItem;
			bool bGotRows = (*tIt)->GetNextRowIdBlock ( tItem.m_dSrcRows );
			if ( bGotRows )
			{
				tItem.m_pIt = *tIt;
				m_tMerge.Push ( tItem );
				tIt++;
			} else
			{
				tIt = m_dIt.erase ( tIt );
			}
		}
	}

	while ( m_tMerge.GetLength() && m_dRows.GetLength()<CIDX_ROWS )
	{
		IterQueueItem_t tTop = m_tMerge.Root();
		m_tMerge.Pop();

		RowID_t tCurRowID = tTop.GetRow();
		m_dRows.Add ( tCurRowID );

		// advance top item
		AdvanceQueue ( m_tMerge, tTop );

		// skip all items with the same row-id
		while ( m_tMerge.GetLength() && m_tMerge.Root().GetRow()==tCurRowID )
		{
			IterQueueItem_t tMatched = m_tMerge.Root();
			m_tMerge.Pop();
			AdvanceQueue ( m_tMerge, tMatched );
		}
	}

	dRowIdBlock = m_dRows;
	return ( m_dRows.GetLength() );
}

struct AndItem_t
{
	AndItem_t() = default;
	RowID_t GetRow() const { return m_dSrcRows[m_iCur]; }

	int m_iCur = 0;
	RowIdBlock_t m_dSrcRows;
	RowidIterator_i * m_pIt { nullptr };
};


class AndRowsIterator_c : public RowidIterator_i
{
public:
	AndRowsIterator_c ( std::vector<RowidIterator_i *> & dIt )
		: m_dSrc ( std::move ( dIt ) )
	{
		m_dRows.Reserve ( CIDX_ROWS );
	}

	~AndRowsIterator_c()
	{
		for ( RowidIterator_i * pIt : m_dSrc )
			SafeDelete ( pIt );
	}

	bool	HintRowID ( RowID_t tRowID ) { return ( m_dItems.size()>0 ); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	int64_t	GetNumProcessed() const
	{
		uint64_t uCount = 0;
		for ( const auto & tItem : m_dItems )
			uCount += tItem.m_pIt->GetNumProcessed();

		return uCount;
	}

private:
	std::vector<RowidIterator_i *> m_dSrc;
	std::vector<AndItem_t> m_dItems;
	bool m_bStarted = false;

	CSphVector<RowID_t> m_dRows;
};

static SI::FilterRange_t Convert ( const CSphFilterSettings & tFilter, const CommonFilterSettings_t & tFilterVals, const SI::ColumnInfo_t & tCol )
{
	SI::FilterRange_t tItem;
	tItem.m_iMin = tFilterVals.m_iMinValue;
	tItem.m_iMax = tFilterVals.m_iMaxValue;
	tItem.m_fMin = (float)tFilterVals.m_fMinValue;
	tItem.m_fMax = (float)tFilterVals.m_fMaxValue;
	tItem.m_bHasEqualMin = tFilter.m_bHasEqualMin;
	tItem.m_bHasEqualMax = tFilter.m_bHasEqualMax;
	tItem.m_bOpenLeft = tFilter.m_bOpenLeft;
	tItem.m_bOpenRight = tFilter.m_bOpenRight;

	if ( tFilterVals.m_eType==SPH_FILTER_RANGE )
	{
		tItem.m_fMin = (float)tFilterVals.m_iMinValue;
		tItem.m_fMax = (float)tFilterVals.m_iMaxValue;
	}

	// clamp to min and max values from a wider type
	if ( tCol.m_eType==SI::AttrType_e::UINT32 || tCol.m_eType==SI::AttrType_e::UINT32SET || tCol.m_eType==SI::AttrType_e::TIMESTAMP )
	{
		if ( tFilterVals.m_iMinValue<0 )
		{
			tItem.m_iMin = 0;
			tItem.m_bHasEqualMin = true;
		}
		if ( tFilterVals.m_iMaxValue>UINT_MAX )
		{
			tItem.m_iMax = UINT_MAX;
			tItem.m_bHasEqualMax = true;
		}
	}

	return tItem;
}

static SI::Collation_e ConvertCollation ( ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_LIBC_CS: return SI::Collation_e::LIBC_CS;
	case SPH_COLLATION_UTF8_GENERAL_CI: return SI::Collation_e::UTF8_GENERAL_CI;
	case SPH_COLLATION_BINARY: return SI::Collation_e::BINARY;

	case SPH_COLLATION_LIBC_CI:
	default:
		return SI::Collation_e::LIBC_CI;
	}
}

RowidIterator_i * CreateSecondaryIndexIterator ( const SI::Index_i * pCidx, CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged )
{
	if ( dEnabledIndexes.IsEmpty() )
		return nullptr;

	CSphVector<SecondaryIndexInfo_t> dNewEnabledIndexes;
	CSphVector<SecondaryIndexInfo_t> dFilterChanged;
	std::vector<SI::RowidIterator_i *> dFilterIt;
	std::vector<RowidIterator_i *> dRes;
	std::unique_ptr<SI::FilterContext_i> pFilterCtx { pCidx->CreateFilterContext() };
	RowIdBoundaries_t tRowidBounds;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, uRowsCount, tRowidBounds );

	for ( int iFilter=0; iFilter<dEnabledIndexes.GetLength(); iFilter++ )
	{
		int iFilterId = dEnabledIndexes[iFilter].m_iFilterId;
		dNewEnabledIndexes.Add().m_iFilterId = iFilterId;

		const CSphFilterSettings & tFilter = dFilters[iFilterId];

		SI::FilterArgs_t tArgs;
		tArgs.m_tBounds.m_bHasRange = ( pRowIdFilter!=nullptr );
		tArgs.m_tBounds.m_uMin = tRowidBounds.m_tMinRowID;
		tArgs.m_tBounds.m_uMax = tRowidBounds.m_tMaxRowID;

		tArgs.m_tCol = pCidx->GetColumn ( tFilter.m_sAttrName.cstr() );

		CommonFilterSettings_t tFixedSettings = tFilter;
		if ( tArgs.m_tCol.m_eType==SI::AttrType_e::FLOAT )
			FixupFilterSettings ( tFilter, SPH_ATTR_FLOAT, tFixedSettings );

		 // !COMMIT
		//StringBuilder_c sTmp;
		//FormatFilterQL ( tFilter, sTmp, 1000 );
		//sphInfo ( "filter %d: %s", iFilterId, sTmp.cstr() );

		int iCount = Max ( tFilter.m_dStrings.GetLength(), tFilter.GetNumValues() );
		CSphVector<uint64_t> dFilterVals ( iCount );
		for ( int i=0; i<iCount; i++ )
		{
			switch ( tFixedSettings.m_eType )
			{
			case SPH_FILTER_VALUES:
				dFilterVals[i] = tFilter.GetValue ( i );
				break;

			case SPH_FILTER_STRING:
			case SPH_FILTER_STRING_LIST:
				dFilterVals[i] = pCidx->GetHash ( tFilter.m_dStrings[i].cstr() );
				break;

			default: break;
			}
		}

		// need ascending order wo duplicates
		dFilterVals.Uniq();

		bool bCreated = false;
		std::string sError;
		CSphString sErrorBuf;

		switch ( tFixedSettings.m_eType )
		{
		case SPH_FILTER_VALUES:
		case SPH_FILTER_STRING:
		case SPH_FILTER_STRING_LIST:
			tArgs.m_dVals = columnar::Span_T<uint64_t> ( dFilterVals.Begin(), dFilterVals.GetLength() );
			bCreated = pCidx->GetValsRows ( tArgs, sError, pFilterCtx.get(), dFilterIt );
			break;

		case SPH_FILTER_RANGE:
		case SPH_FILTER_FLOATRANGE:
			tArgs.m_tRange = Convert ( tFilter, tFixedSettings, tArgs.m_tCol );
			bCreated = pCidx->GetRangeRows ( tArgs, sError, pFilterCtx.get(), dFilterIt );
			break;

		default:
			sErrorBuf.SetSprintf ( "unsupported filter selected for secondary index %s(%d), type %d", tFilter.m_sAttrName.cstr(), tArgs.m_tCol.m_iSrcAttr, tFixedSettings.m_eType );
			sError = sErrorBuf.cstr();
			break;
		}

		if ( !bCreated )
		{
			// FIXME!!! return as query warning
			sphWarning ( "%s", sError.c_str() );
			for ( auto * pIt : dFilterIt ) { SafeDelete ( pIt ); }
			dFilterIt.resize ( 0 );
			continue;
		}

		if ( !sError.empty() )
		{
			// FIXME!!! return as query warning
			sphWarning ( "secondary index %s(%d):%s", tFilter.m_sAttrName.cstr(), tArgs.m_tCol.m_iSrcAttr, sError.c_str() );
			sError.clear();
		}

		// !COMMIT
		//int dCounts[4] { 0, 0, 0, 0 };
		//for ( const auto pIt : dFilterIt )
		//{
		//	int iType = pIt->GetPacking();
		//	dCounts[iType]++;
		//}
		//sphInfo ( "iterators %d: row %d, block %d, list %d", (int)dFilterIt.size(), dCounts[0], dCounts[1], dCounts[2] );

		dNewEnabledIndexes.Pop();
		dFilterChanged.Add().m_iFilterId = iFilterId;

		bool bExclude = tFilter.m_bExclude;
		
		RowidIterator_i * pIt = nullptr;
		if ( !dFilterIt.size() )
			pIt = new RowidEmptyIterator_c();
		else if ( dFilterIt.size()==1 )
			pIt = new RowidIterator_c ( dFilterIt[0] );
		else
			pIt = new RowsIterator_c ( dFilterIt );

		if ( bExclude )
			pIt = new RowidIteratorNot_c ( pIt, uRowsCount );

		dRes.emplace_back ( pIt );
		dFilterIt.resize ( 0 );
	}

	if ( dFilterChanged.GetLength() )
	{
		bFiltersChanged = UpdateModifiedFilters ( dFilters, dModifiedFilters, dFilterChanged, nullptr );
		dEnabledIndexes.SwapData ( dNewEnabledIndexes );

		// remove "@rowid" filter in case iterator created and no other unhandled filters left
		if ( pRowIdFilter && dModifiedFilters.GetLength()==1 && dModifiedFilters.Begin()==pRowIdFilter )
		{
			dModifiedFilters.Resize ( 0 );
			dEnabledIndexes.Resize ( 0 );
			bFiltersChanged = true;
		}
	}

	if ( !dRes.size() )
		return nullptr;
	else if ( dRes.size()==1 )
		return dRes[0];
	else
		return new AndRowsIterator_c ( dRes );
}

static SI::AttrType_e GetType ( ESphAttr eAttrType )
{
	switch ( eAttrType )
	{
		case SPH_ATTR_INTEGER: return SI::AttrType_e::UINT32;
		case SPH_ATTR_TIMESTAMP: return SI::AttrType_e::TIMESTAMP;
		case SPH_ATTR_BIGINT: return SI::AttrType_e::INT64;
		case SPH_ATTR_BOOL: return SI::AttrType_e::BOOLEAN;
		case SPH_ATTR_FLOAT: return SI::AttrType_e::FLOAT;
		case SPH_ATTR_STRING: return SI::AttrType_e::STRING;
		case SPH_ATTR_UINT32SET: return SI::AttrType_e::UINT32SET;
		case SPH_ATTR_INT64SET: return SI::AttrType_e::INT64SET;

		default: return SI::AttrType_e::NONE;
	}
}

static std::vector<SI::SourceAttrTrait_t> ConvertSchema ( const CSphSchema & tSchema )
{
	std::vector<SI::SourceAttrTrait_t> dAttrs;
	for ( int iAttr=0; iAttr<tSchema.GetAttrsCount(); iAttr++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( iAttr );
		// skip special / iternal attributes
		if ( sphIsInternalAttr ( tCol.m_sName ) )
			continue;

		if ( tCol.m_eAttrType==SPH_ATTR_JSON )
			continue;

		SI::SourceAttrTrait_t tSrcAttr;
		tSrcAttr.m_eType = GetType ( tCol.m_eAttrType );
		tSrcAttr.m_iAttr = iAttr;
		tSrcAttr.m_sName = tCol.m_sName.cstr();
		dAttrs.emplace_back ( tSrcAttr );
	}

	return dAttrs;
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, const char * sFile, CSphString & sError )
{
	std::vector<SI::SourceAttrTrait_t> dAttrs = ConvertSchema ( tSchema );
	return CreateSecondaryIndexBuilder ( dAttrs, iMemoryLimit, ConvertCollation ( GlobalCollation() ), sFile, sError );
}

static void GetAttrsProxy ( const ISphSchema & tSchema, const std::vector<SI::SourceAttrTrait_t> & dSrcAttrs, CSphVector<PlainOrColumnar_t> & dDstAttrs )
{
	int iColumnar = 0;
	for ( const SI::SourceAttrTrait_t & tSrcAttr : dSrcAttrs )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr ( tSrcAttr.m_iAttr );

		PlainOrColumnar_t & tDstAttr = dDstAttrs.Add();
		tDstAttr.m_eType = tAttr.m_eAttrType;
		tDstAttr.m_iSchemaAttr = tSrcAttr.m_iAttr;

		if ( tAttr.IsColumnar() )
			tDstAttr.m_iColumnarId = iColumnar++;
		else
			tDstAttr.m_tLocator = tAttr.m_tLocator;
	}
}

std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, const char * sFile, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError )
{
	std::vector<SI::SourceAttrTrait_t> dSrcAttrs = ConvertSchema ( tSchema );
	GetAttrsProxy ( tSchema, dSrcAttrs, dAttrs );

	return CreateSecondaryIndexBuilder ( dSrcAttrs, iMemoryLimit, ConvertCollation ( GlobalCollation() ), sFile, sError );
}

void BuilderStoreAttrs ( const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, SI::Builder_i * pBuilder, CSphVector<int64_t> & dTmp )
{
	for ( const PlainOrColumnar_t & tSrc : dAttrs )
	{
		switch ( tSrc.m_eType )
		{
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
		{
			const BYTE * pSrc = nullptr;
			int iBytes = tSrc.Get ( pRow, pPool, dIterators, pSrc );
			int iValues = iBytes / ( tSrc.m_eType==SPH_ATTR_UINT32SET ? sizeof(DWORD) : sizeof(int64_t) );
			if ( tSrc.m_eType==SPH_ATTR_UINT32SET )
			{
				// need a 64-bit array as input. so we need to convert our 32-bit array to 64-bit entries
				dTmp.Resize ( iValues );
				ARRAY_FOREACH ( i, dTmp )
					dTmp[i] = ((DWORD*)pSrc)[i];

				pBuilder->SetAttr ( tSrc.m_iSchemaAttr, dTmp.Begin(), iValues );
			}
			else
				pBuilder->SetAttr ( tSrc.m_iSchemaAttr, (const int64_t*)pSrc, iValues );
		}
		break;

		case SPH_ATTR_STRING:
		{
			const BYTE * pSrc = nullptr;
			int iBytes = tSrc.Get ( pRow, pPool, dIterators, pSrc );
			pBuilder->SetAttr ( tSrc.m_iSchemaAttr, (const uint8_t*)pSrc, iBytes );
		}
		break;

		default:
			pBuilder->SetAttr ( tSrc.m_iSchemaAttr, tSrc.Get ( pRow, dIterators ) );
			break;
		}
	}
}

RowID_t GetMin ( const std::vector<AndItem_t> & dItems )
{
	RowID_t tMin = INVALID_ROWID;
	for ( const auto & tIt : dItems )
	{
		tMin = std::min ( tMin, tIt.GetRow() );
	}

	return tMin;
}

void Warmup ( const std::vector<RowidIterator_i *> & dSrc, std::vector<AndItem_t> & dItems )
{
	for ( auto tIt : dSrc )
	{
		AndItem_t tItem;
		bool bGotRows = tIt->GetNextRowIdBlock ( tItem.m_dSrcRows );
		if ( !bGotRows )
		{
			dItems.resize ( 0 );
			return;
		}

		tItem.m_pIt = tIt;
		dItems.push_back ( tItem );
		tIt++;
	}
}

int Advance ( std::vector<AndItem_t> & dItems, RowID_t tCurRowID )
{
	int iCount = 0;
	for ( auto & tIt : dItems )
	{
		if ( tIt.GetRow()==tCurRowID )
		{
			iCount++;
			continue;
		}

		// rewind till the row
		while ( tIt.GetRow()<tCurRowID )
		{
			tIt.m_iCur++;
			if ( tIt.m_iCur>=tIt.m_dSrcRows.GetLength() )
			{
				tIt.m_iCur = 0;
				if ( !tIt.m_pIt->GetNextRowIdBlock ( tIt.m_dSrcRows ) )
				{
					// no more common rows
					dItems.resize ( 0 );
					return 0;
				}
			}
		}

		if ( tIt.GetRow()==tCurRowID )
			iCount++;
		else
			iCount = 0;

	}

	return iCount;
}

bool AndRowsIterator_c::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bStarted && !m_dItems.size() )
		return false;

	m_dRows.Resize ( 0 );

	// warmup
	if ( !m_bStarted )
	{
		m_bStarted = true;
		Warmup ( m_dSrc, m_dItems );
	}

	while ( m_dItems.size() && m_dRows.GetLength()<CIDX_ROWS )
	{
		RowID_t tCurRowID = GetMin ( m_dItems );

		int iCount = Advance ( m_dItems, tCurRowID );

		if ( m_dItems.size() && iCount==m_dItems.size() )
			m_dRows.Add ( tCurRowID );

		// rewind all iterators to the next row-id
		tCurRowID++;
		Advance ( m_dItems, tCurRowID );
	}

	dRowIdBlock = m_dRows;
	return ( m_dRows.GetLength() );
}

class SelectProxy_c : public SelectSI_c
{
public:
	explicit SelectProxy_c ( const SI::Index_i * pSidx, const ISphSchema & tSchema, ESphCollation eCollation )
		: m_pSidx ( pSidx )
		, m_tSchema ( tSchema )
		, m_eCollation ( eCollation )
	{
	}

	virtual ~SelectProxy_c() {};

	bool IsEnabled ( const CSphFilterSettings & tFilter ) const
	{
		assert ( m_pSidx );

		if ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_STRING_LIST && tFilter.m_eType!=SPH_FILTER_RANGE && tFilter.m_eType!=SPH_FILTER_FLOATRANGE )
			return false;
		
		// all(mva\string) need to scan whole row
		if ( tFilter.m_eMvaFunc==SPH_MVAFUNC_ALL )
			return false;

		// need to handle only plain or columnar attr but not dynamic \ expressions
		const CSphColumnInfo * pCol = m_tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
		if ( !pCol )
			return false;
		if ( pCol->m_pExpr.Ptr() && !pCol->IsColumnarExpr() )
			return false;

		SI::ColumnInfo_t tColSI = m_pSidx->GetColumn (  tFilter.m_sAttrName.cstr() );
		if ( tColSI.m_eType==SI::AttrType_e::NONE || !tColSI.m_bEnabled )
			return false;

		// FIXME!!! warn in case force index used but index was skipped
		if ( tColSI.m_eType==SI::AttrType_e::STRING && m_pSidx->GetCollation()!=ConvertCollation ( m_eCollation ) )
			return false;

		return true;
	}

	const SI::Index_i * m_pSidx { nullptr };
	const ISphSchema & m_tSchema;
	ESphCollation m_eCollation { SPH_COLLATION_DEFAULT };
};

SelectSI_c * GetSelectIteratorsCond ( const SI::Index_i * pSidx, const ISphSchema & tSchema, ESphCollation eCollation )
{
	if ( pSidx )
		return new SelectProxy_c ( pSidx, tSchema, eCollation );
	else
		return new SelectSI_c();
}

bool SelectSI_c::IsEnabled ( const CSphFilterSettings & tFilter ) const
{
	return HaveIndex ( tFilter.m_sAttrName.cstr() );
}

