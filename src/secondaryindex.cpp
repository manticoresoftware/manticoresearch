//
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
#include <algorithm>

#include "histogram.h"
#include "sphinxint.h"
#include "killlist.h"
#include "attribute.h"
#include "columnarfilter.h"
#include <queue>

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
	static const int MAX_COLLECTED = 512;
	CSphFixedVector<RowID_t> m_dCollected {MAX_COLLECTED};
};

template <bool BITMAP>
class CachedIterator_T : public SecondaryIndexIterator_c
{
public:
				CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs );

	bool		HintRowID ( RowID_t tRowID ) override;
	void		SetCutoff ( int iCutoff ) override {}	// fixme! add real cutoff
	bool		WasCutoffHit() const override { return false; }

protected:
	CSphTightVector<RowID_t> m_dRowIDs;
	int			m_iId = 0;

	BitVec_T<uint64_t> m_tBitmap;
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
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "lookup" } ); }

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
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "lookup" } ); }

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

template <typename T,  bool ROWID_LIMITS>
struct IteratorState_T
{
	std::unique_ptr<T>	m_pIterator;

	const RowID_t *		m_pRowID = nullptr;
	const RowID_t *		m_pRowIDMax = nullptr;

	RowIdBoundaries_t	m_tBoundaries;
	CSphVector<RowID_t> m_dCollected;

						IteratorState_T() = default;
						IteratorState_T ( T * pIterator ) : m_pIterator(pIterator) {}

	FORCE_INLINE bool	RewindTo  ( RowID_t tRowID );
	FORCE_INLINE bool	WarmupDocs();
	FORCE_INLINE bool	WarmupDocs ( RowID_t tRowID );

	static inline bool	IsLess ( const IteratorState_T * pA, const IteratorState_T * pB ) { return ( *pA->m_pRowID < *pB->m_pRowID ); }
};

template <typename T, bool ROWID_LIMITS>
bool IteratorState_T<T, ROWID_LIMITS>::WarmupDocs()
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
bool IteratorState_T<common::BlockIterator_i,true>::WarmupDocs()
{
	assert(m_pIterator);

	RowIdBlock_t dRowIdBlock;

	do
	{
		util::Span_T<uint32_t> dSpan;
		if ( !m_pIterator->GetNextRowIdBlock(dSpan) )
		{
			m_pRowID = m_pRowIDMax = nullptr;
			return false;
		}

		RowID_t tMinSpanRowID = dSpan.front();
		RowID_t tMaxSpanRowID = dSpan.back();

		dRowIdBlock = { (RowID_t *)dSpan.begin(), (int64_t)dSpan.size() };

		// we need additional filtering only on first and last blocks
		// per-block filtering is performed inside MCL
		if ( tMinSpanRowID < m_tBoundaries.m_tMinRowID || tMaxSpanRowID > m_tBoundaries.m_tMaxRowID )
			dRowIdBlock = DoRowIdFiltering ( dRowIdBlock, m_tBoundaries, m_dCollected );
	}
	while ( !dRowIdBlock.GetLength() );

	m_pRowID = (const RowID_t *)dRowIdBlock.begin();
	m_pRowIDMax = (const RowID_t *)dRowIdBlock.end();

	return true;
}

template <>
bool IteratorState_T<common::BlockIterator_i,false>::WarmupDocs()
{
	assert(m_pIterator);

	util::Span_T<uint32_t> dRowIdBlock;
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
bool IteratorState_T<T,ROWID_LIMITS>::WarmupDocs ( RowID_t tRowID )
{
	if ( !m_pIterator->HintRowID(tRowID) )
		return false;

	return WarmupDocs();
}

template <typename T, bool ROWID_LIMITS>
bool IteratorState_T<T,ROWID_LIMITS>::RewindTo ( RowID_t tRowID )
{
	if ( tRowID>*(m_pRowIDMax-1) && !WarmupDocs(tRowID) )
	{
		m_pRowID = m_pRowIDMax = nullptr;
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

//////////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Base_T : public SecondaryIndexIterator_c
{
public:
				RowidIterator_Base_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries );

	int64_t		GetNumProcessed() const override;
	void		AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override;

protected:
	using IteratorState_t = IteratorState_T<T,ROWID_LIMITS>;

	CSphFixedVector<IteratorState_t> m_dIterators;
};

template <typename T, bool ROWID_LIMITS>
RowidIterator_Base_T<T,ROWID_LIMITS>::RowidIterator_Base_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries )
	: m_dIterators ( iNumIterators )
{
	for ( int i = 0; i < iNumIterators; i++ )
	{
		m_dIterators[i] = ppIterators[i];
		if ( pBoundaries )
			m_dIterators[i].m_tBoundaries = *pBoundaries;
	}
}

template <typename T, bool ROWID_LIMITS>
int64_t RowidIterator_Base_T<T,ROWID_LIMITS>::GetNumProcessed() const
{
	int64_t iTotal = 0;
	for ( auto & i : m_dIterators )
		iTotal += i.m_pIterator->GetNumProcessed();

	return iTotal;
}

template <typename T, bool ROWID_LIMITS>
void RowidIterator_Base_T<T,ROWID_LIMITS>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	std::vector<common::IteratorDesc_t> dIteratorDesc;
	for ( const auto & i : m_dIterators )
		i.m_pIterator->AddDesc(dIteratorDesc);

	for ( const auto & i : dIteratorDesc )
		dDesc.Add ( { i.m_sAttr.c_str(), i.m_sType.c_str() } );
}

template <>
void RowidIterator_Base_T<RowidIterator_i,true>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	for ( const auto & i : m_dIterators )
		i.m_pIterator->AddDesc(dDesc);
}

template <>
void RowidIterator_Base_T<RowidIterator_i,false>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	for ( const auto & i : m_dIterators )
		i.m_pIterator->AddDesc(dDesc);
}

//////////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Intersect_T : public RowidIterator_Base_T<T,ROWID_LIMITS>
{
	using BASE = RowidIterator_Base_T<T,ROWID_LIMITS>;

public:
				RowidIterator_Intersect_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool		HintRowID ( RowID_t tRowID ) override { return BASE::m_dIterators[0].RewindTo(tRowID); }
	bool		GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	void		SetCutoff ( int iCutoff ) override { m_iRowsLeft = iCutoff; }
	bool		WasCutoffHit() const override { return !m_iRowsLeft; }

private:
	int			m_iRowsLeft = INT_MAX;

	FORCE_INLINE bool	AdvanceIterators();
	FORCE_INLINE bool	Advance ( int iIterator, RowID_t tRowID );
};

template <typename T, bool ROWID_LIMITS>
RowidIterator_Intersect_T<T,ROWID_LIMITS>::RowidIterator_Intersect_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries )
	: BASE ( ppIterators, iNumIterators, pBoundaries )
{
	BASE::m_dIterators[0].WarmupDocs();
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + Min ( BASE::m_dCollected.GetLength()-1, m_iRowsLeft );
	RowID_t * pRowID = pRowIdStart;

	auto & tFirst = BASE::m_dIterators[0];

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
			break;
	}

	if ( m_iRowsLeft!=INT_MAX )
	{
		m_iRowsLeft -= pRowID-pRowIdStart;
		assert ( m_iRowsLeft>=0 );
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::AdvanceIterators()
{
	auto & tFirst = BASE::m_dIterators[0];
	RowID_t tMaxRowID = *tFirst.m_pRowID;
	for ( int i=1; i < BASE::m_dIterators.GetLength(); i++ )
	{
		auto & tState = BASE::m_dIterators[i];
		if ( !tState.m_pRowID && !tState.WarmupDocs(tMaxRowID) )
			return false;

		if ( *tState.m_pRowID==tMaxRowID )
			continue;

		if ( !tState.RewindTo(tMaxRowID) )
			return false;

		if ( *tState.m_pRowID>tMaxRowID )
		{
			if ( !tFirst.RewindTo( *tState.m_pRowID ) )
				return false;

			tMaxRowID = *tFirst.m_pRowID;
			i = 0;
		}
	}

	return true;
}

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Union_T : public RowidIterator_Base_T<T,ROWID_LIMITS>
{
	using BASE = RowidIterator_Base_T<T,ROWID_LIMITS>;

public:
				RowidIterator_Union_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool		HintRowID ( RowID_t tRowID ) override;
	bool		GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	void		SetCutoff ( int iCutoff ) override { m_iRowsLeft = iCutoff; }
	bool		WasCutoffHit() const override { return !m_iRowsLeft; }

private:
	CSphQueue<typename BASE::IteratorState_t*, typename BASE::IteratorState_t> m_tMerge;
	int			m_iRowsLeft = INT_MAX;

	FORCE_INLINE void AdvanceQueue ( typename BASE::IteratorState_t * pState );
};

template <typename T, bool ROWID_LIMITS>
RowidIterator_Union_T<T,ROWID_LIMITS>::RowidIterator_Union_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries )
	: BASE ( ppIterators, iNumIterators, pBoundaries )
	, m_tMerge ( iNumIterators )
{
	for ( auto & i : BASE::m_dIterators )
		if ( i.WarmupDocs() )
			m_tMerge.Push(&i);
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Union_T<T,ROWID_LIMITS>::HintRowID ( RowID_t tRowID )
{
	if ( !m_tMerge.GetLength() )
		return false;

	if ( tRowID<=*m_tMerge.Root()->m_pRowID + m_tMerge.GetLength() )
		return true;

	m_tMerge.Clear();
	for ( auto & i : BASE::m_dIterators )
		if ( i.m_pRowID && i.RewindTo(tRowID) )
			m_tMerge.Push(&i);

	return m_tMerge.GetLength()>0;
}

template <typename T, bool ROWID_LIMITS>
void RowidIterator_Union_T<T,ROWID_LIMITS>::AdvanceQueue ( typename BASE::IteratorState_t * pState )
{
	pState->m_pRowID++;
	if ( pState->m_pRowID<pState->m_pRowIDMax || pState->WarmupDocs() )
		m_tMerge.Push(pState);
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_Union_T<T,ROWID_LIMITS>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + Min ( BASE::m_dCollected.GetLength()-1, m_iRowsLeft );
	RowID_t * pRowID = pRowIdStart;

	RowID_t tLastRowID = INVALID_ROWID;
	while ( pRowID<pRowIdMax && m_tMerge.GetLength()>1 )
	{
		auto pState = m_tMerge.Root();
		m_tMerge.Pop();

		RowID_t tCurRowID = *pState->m_pRowID;
		if ( tCurRowID!=tLastRowID ) // skip all items with the same row-id
			*pRowID++ = tCurRowID;

		tLastRowID = tCurRowID;
		AdvanceQueue(pState);
	}

	if ( m_tMerge.GetLength()==1 )
	{
		auto pState = m_tMerge.Root();
		do
		{
			while ( pRowID<pRowIdMax && pState->m_pRowID<pState->m_pRowIDMax )
				*pRowID++ = *pState->m_pRowID++;
		}
		while ( pState->m_pRowID>=pState->m_pRowIDMax && pState->WarmupDocs() );
	}

	if ( m_iRowsLeft!=INT_MAX )
	{
		m_iRowsLeft -= pRowID-pRowIdStart;
		assert ( m_iRowsLeft>=0 );
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

/////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_UnionBitmap_T : public RowidIterator_Base_T<T, ROWID_LIMITS>
{
	using BASE = RowidIterator_Base_T<T, ROWID_LIMITS>;

public:
				RowidIterator_UnionBitmap_T ( T ** ppIterators, int iNumIterators, RowID_t tNumDocs, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool		HintRowID ( RowID_t tRowID ) override { m_tBit = tRowID; return true; }
	bool		GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t		GetNumProcessed() const override { return m_iTotalProcessed; }
	void		SetCutoff ( int iCutoff ) override {} // bitmap can't do cutoff by design
	bool		WasCutoffHit() const override { return false; }
	void		AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override;

private:
	BitVec_T<uint64_t> m_tBitmap;
	CSphVector<IteratorDesc_t> m_dDesc;
	RowID_t		m_tBit = 0;
	RowID_t		m_tMinRowID = 0;
	int64_t		m_iTotalProcessed = 0;

	void		StoreIteratorDesc();
};

template <typename T, bool ROWID_LIMITS>
RowidIterator_UnionBitmap_T<T, ROWID_LIMITS>::RowidIterator_UnionBitmap_T ( T ** ppIterators, int iNumIterators, RowID_t tNumDocs, const RowIdBoundaries_t * pBoundaries )
	: BASE ( ppIterators, iNumIterators, pBoundaries )
{
	if ( pBoundaries )
	{
		m_tBitmap.Init ( pBoundaries->m_tMaxRowID-pBoundaries->m_tMinRowID+1 );
		m_tMinRowID = pBoundaries->m_tMinRowID;
	}
	else
		m_tBitmap.Init(tNumDocs);

	for ( auto & i : BASE::m_dIterators )
		while ( i.WarmupDocs() )
			while ( i.m_pRowID < i.m_pRowIDMax )
			{
				m_tBitmap.BitSet ( *i.m_pRowID - m_tMinRowID );
				i.m_pRowID++;
			}

	StoreIteratorDesc();

	for ( auto & i : BASE::m_dIterators )
	{
		m_iTotalProcessed += i.m_pIterator->GetNumProcessed();
		i.m_pIterator.reset();
	}
}

template <typename T, bool ROWID_LIMITS>
bool RowidIterator_UnionBitmap_T<T, ROWID_LIMITS>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;
	int iTotal = m_tBitmap.GetSize();

	while ( pRowID<pRowIdMax && m_tBit<iTotal )
	{
		m_tBit = m_tBitmap.Scan(m_tBit);
		if ( m_tBit>=iTotal )
			break;

		*pRowID++ = m_tBit + m_tMinRowID;
		m_tBit++;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <typename T, bool ROWID_LIMITS>
void RowidIterator_UnionBitmap_T<T, ROWID_LIMITS>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	for ( const auto & i : m_dDesc )
		dDesc.Add(i);
}

template <typename T, bool ROWID_LIMITS>
void RowidIterator_UnionBitmap_T<T, ROWID_LIMITS>::StoreIteratorDesc()
{
	std::vector<common::IteratorDesc_t> dDesc;
	for ( const auto & i : BASE::m_dIterators )
		i.m_pIterator->AddDesc(dDesc);

	for ( const auto & i : dDesc )
		m_dDesc.Add ( { i.m_sAttr.c_str(), i.m_sType.c_str() } );
}

template <>
void RowidIterator_UnionBitmap_T<RowidIterator_i, true>::StoreIteratorDesc()
{
	for ( const auto & i : BASE::m_dIterators )
		i.m_pIterator->AddDesc(m_dDesc);
}

template <>
void RowidIterator_UnionBitmap_T<RowidIterator_i, false>::StoreIteratorDesc()
{
	for ( const auto & i : BASE::m_dIterators )
		i.m_pIterator->AddDesc(m_dDesc);
}

/////////////////////////////////////////////////////////////////////

template <bool ROWID_LIMITS>
class RowidIterator_Wrapper_T : public RowidIterator_i
{
public:
			RowidIterator_Wrapper_T ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool	HintRowID ( RowID_t tRowID ) override { return m_pIterator->HintRowID(tRowID); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t	GetNumProcessed() const override { return m_pIterator->GetNumProcessed(); }
	void	SetCutoff ( int iCutoff ) override {}
	bool	WasCutoffHit() const override { return false; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override;

private:
	std::unique_ptr<common::BlockIterator_i> m_pIterator;
	RowIdBoundaries_t	m_tBoundaries;
	CSphVector<RowID_t>	m_dCollected;
};

template <bool ROWID_LIMITS>
RowidIterator_Wrapper_T<ROWID_LIMITS>::RowidIterator_Wrapper_T ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries )
	: m_pIterator ( pIterator )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <>
bool RowidIterator_Wrapper_T<true>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	do
	{
		util::Span_T<uint32_t> dSpan;
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
	}
	while ( !dRowIdBlock.GetLength() );

	return true;
}

template <>
bool RowidIterator_Wrapper_T<false>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	util::Span_T<uint32_t> dSpan;
	if ( !m_pIterator->GetNextRowIdBlock(dSpan) )
		return false;

	dRowIdBlock = { (RowID_t *)dSpan.begin(), (int64_t)dSpan.size() };
	return true;
}

template <bool ROWID_LIMITS>
void RowidIterator_Wrapper_T<ROWID_LIMITS>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	assert(m_pIterator);

	std::vector<common::IteratorDesc_t> dIteratorDesc;
	m_pIterator->AddDesc(dIteratorDesc);

	for ( const auto & i : dIteratorDesc )
		dDesc.Add ( { i.m_sAttr.c_str(), i.m_sType.c_str() } );
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
	for ( int i = 0; i < dSet.GetSize(); i++ )
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

	for ( int i = 0; i < dEnabled.GetSize(); i++ )
		if ( dSecondaryIndexes[i].m_bForce )
			dEnabled.BitSet(i);

	const int MAX_TRIES = 1024;
	for ( int iTry = 0; iTry < MAX_TRIES; iTry++ )
	{
		for ( int i = 0; i < dEnabled.GetSize(); i++ )
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

	for ( int i = 0; i < dBestEnabled.GetSize(); i++ )
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


RowidIterator_i * CreateIteratorWrapper ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries )
{
	if ( pBoundaries )
		return new RowidIterator_Wrapper_T<true> ( pIterator, pBoundaries );
	else
		return new RowidIterator_Wrapper_T<false> ( pIterator );
}


RowidIterator_i * CreateIteratorIntersect ( std::vector<common::BlockIterator_i *> & dIterators, const RowIdBoundaries_t * pBoundaries )
{
	if ( pBoundaries )
		return new RowidIterator_Intersect_T<common::BlockIterator_i, true> ( &dIterators[0], (int)dIterators.size(), pBoundaries );
	else
		return new RowidIterator_Intersect_T<common::BlockIterator_i, false> ( &dIterators[0], (int)dIterators.size() );
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

	bool	HintRowID ( RowID_t tRowID ) override { return false; }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override { return false; }
	int64_t	GetNumProcessed() const override { return 0; }
	void	SetCutoff ( int iCutoff ) override {}
	bool	WasCutoffHit() const override { return false; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override {}
};

static const int CIDX_ROWS = 512;

class RowidIteratorNot_c : public RowidIterator_i
{
public:
			RowidIteratorNot_c ( RowidIterator_i * pIterator, RowID_t uRowsCount );

	bool	HintRowID ( RowID_t tRowID ) override { return m_pExcludedIt->HintRowID ( tRowID ); }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override ;
	int64_t	GetNumProcessed() const override { return m_pExcludedIt->GetNumProcessed(); }
	void	SetCutoff ( int iCutoff ) override {}		// fixme! implement cutoff
	bool	WasCutoffHit() const override { return false; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { m_pExcludedIt->AddDesc(dDesc); }

private:
	RowID_t		m_uRowsCount { 0 };
	RowID_t		m_uCurRow { 0 };
	bool		m_bStopped = false;
	bool		m_bPass = false;

	bool		m_bExcludedStarted = false;
	bool		m_bExcludedStopped = false;
	int			m_iExcludedCur = 0;
	RowIdBlock_t m_dExcludedRows;
	std::unique_ptr<RowidIterator_i> m_pExcludedIt;

	CSphVector<RowID_t> m_dRows;

	void CopyTail ();
};


RowidIteratorNot_c::RowidIteratorNot_c ( RowidIterator_i * pIterator, RowID_t uRowsCount )
	: m_uRowsCount ( uRowsCount )
	, m_pExcludedIt ( pIterator )
{
	m_dRows.Reserve ( CIDX_ROWS );
}


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

/////////////////////////////////////////////////////////////////////

RowidIterator_i * CreateSecondaryIndexIterator ( const SI::Index_i * pCidx, CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, int iCutoff )
{
	if ( dEnabledIndexes.IsEmpty() )
		return nullptr;

	CSphVector<SecondaryIndexInfo_t> dNewEnabledIndexes;
	CSphVector<SecondaryIndexInfo_t> dFilterChanged;
	std::vector<common::BlockIterator_i *> dFilterIt;
	std::vector<std::pair<RowidIterator_i *, int64_t>> dRes;
	RowIdBoundaries_t tRowidBounds;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, uRowsCount, tRowidBounds );
	common::RowidRange_t tRange { tRowidBounds.m_tMinRowID, tRowidBounds.m_tMaxRowID };

	for ( int iFilter=0; iFilter<dEnabledIndexes.GetLength(); iFilter++ )
	{
		int iFilterId = dEnabledIndexes[iFilter].m_iFilterId;
		dNewEnabledIndexes.Add().m_iFilterId = iFilterId;

		const CSphFilterSettings & tFilter = dFilters[iFilterId];
		if ( &tFilter==pRowIdFilter )
			break;

		bool bCreated = false;
		common::Filter_t tColumnarFilter;
		CSphString sWarning;
		std::string sError;
		if ( ToColumnarFilter ( tColumnarFilter, tFilter, eCollation, tSchema, sWarning ) )
			bCreated = pCidx->CreateIterators ( dFilterIt, tColumnarFilter, pRowIdFilter ? &tRange : nullptr, sError );
		else
			sphWarning ( "secondary index %s: %s", tFilter.m_sAttrName.cstr(), sWarning.cstr() );

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
			sphWarning ( "secondary index %s:%s", tFilter.m_sAttrName.cstr(), sError.c_str() );
			sError.clear();
		}

		dNewEnabledIndexes.Pop();
		dFilterChanged.Add().m_iFilterId = iFilterId;

		bool bExclude = tFilter.m_bExclude;
		
		RowidIterator_i * pIt = nullptr;
		if ( !dFilterIt.size() )
			pIt = new RowidEmptyIterator_c();
		else if ( dFilterIt.size()==1 )
		{
			if ( pRowIdFilter )
				pIt = new RowidIterator_Wrapper_T<true> ( dFilterIt[0], &tRowidBounds );
			else
				pIt = new RowidIterator_Wrapper_T<false> ( dFilterIt[0] );
		}
		else
		{
			const size_t BITMAP_ITERATOR_THRESH = 16;
			const float	BITMAP_RATIO_THRESH = 0.002;

			int64_t iRsetSize = dEnabledIndexes[iFilter].m_iRsetEstimate;
			if ( iCutoff>=0 )
				iRsetSize = Min ( iRsetSize, iCutoff );

			/// fixme! account for cutoff here
			float fRsetRatio = float ( iRsetSize ) / uRowsCount;
			if ( dFilterIt.size()>BITMAP_ITERATOR_THRESH && fRsetRatio >= BITMAP_RATIO_THRESH )
			{
				if ( pRowIdFilter )
					pIt = new RowidIterator_UnionBitmap_T<common::BlockIterator_i,true> ( &dFilterIt[0], (int)dFilterIt.size(), uRowsCount, &tRowidBounds );
				else
					pIt = new RowidIterator_UnionBitmap_T<common::BlockIterator_i,false> ( &dFilterIt[0], (int)dFilterIt.size(), uRowsCount );
			}
			else
			{
				if ( pRowIdFilter )
					pIt = new RowidIterator_Union_T<common::BlockIterator_i,true> ( &dFilterIt[0], (int)dFilterIt.size(), &tRowidBounds );
				else
					pIt = new RowidIterator_Union_T<common::BlockIterator_i,false> ( &dFilterIt[0], (int)dFilterIt.size() );
			}
		}

		if ( bExclude )
			pIt = new RowidIteratorNot_c ( pIt, uRowsCount );

		dRes.push_back ( { pIt, dEnabledIndexes[iFilter].m_iRsetEstimate } );
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
	
	if ( dRes.size()==1 )
		return dRes[0].first;

	std::sort ( dRes.begin(), dRes.end(), []( auto tA, auto tB ) { return tA.second < tB.second; } );
	std::vector<RowidIterator_i*> dIterators;
	for ( auto & i : dRes )
		dIterators.push_back ( i.first );

	return new RowidIterator_Intersect_T<RowidIterator_i, false> ( &dIterators[0], (int)dIterators.size() );
}


static void ConvertSchema ( const CSphSchema & tSchema, common::Schema_t & tSISchema, CSphBitvec & tSIAttrs )
{
	for ( int iAttr=0; iAttr<tSchema.GetAttrsCount(); iAttr++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( iAttr );
		// skip special / iternal attributes
		if ( sphIsInternalAttr ( tCol.m_sName ) )
			continue;

		if ( tCol.m_eAttrType==SPH_ATTR_JSON )
			continue;

		common::StringHash_fn fnStringCalcHash = nullptr;
		common::AttrType_e eAttrType = ToColumnarType ( tCol.m_eAttrType, tCol.m_tLocator.m_iBitCount );

		// fixme! make default collation configurable
		if ( eAttrType==common::AttrType_e::STRING )
			fnStringCalcHash = LibcCIHash_fn::Hash;

		tSISchema.push_back ( { tCol.m_sName.cstr(), eAttrType, fnStringCalcHash } );
		tSIAttrs.BitSet(iAttr);
	}
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, CSphBitvec & tSIAttrs, const char * sFile, CSphString & sError )
{
	common::Schema_t tSISchema;
	ConvertSchema ( tSchema, tSISchema, tSIAttrs );
	return CreateSecondaryIndexBuilder ( tSISchema, iMemoryLimit, sFile, sError );
}


static void GetAttrsProxy ( const ISphSchema & tSchema, common::Schema_t & tSISchema, CSphVector<PlainOrColumnar_t> & dDstAttrs )
{
	int iColumnar = 0;
	for ( const auto & i : tSISchema )
	{
		const CSphColumnInfo * pAttr = tSchema.GetAttr ( i.m_sName.c_str() );
		assert(pAttr);

		PlainOrColumnar_t & tDstAttr = dDstAttrs.Add();
		tDstAttr.m_eType = pAttr->m_eAttrType;

		if ( pAttr->IsColumnar() )
			tDstAttr.m_iColumnarId = iColumnar++;
		else
			tDstAttr.m_tLocator = pAttr->m_tLocator;
	}
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, const char * sFile, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError )
{
	common::Schema_t tSISchema;
	CSphBitvec tSIAttrs ( tSchema.GetAttrsCount() );
	ConvertSchema ( tSchema, tSISchema, tSIAttrs );
	GetAttrsProxy ( tSchema, tSISchema, dAttrs );

	return CreateSecondaryIndexBuilder ( tSISchema, iMemoryLimit, sFile, sError );
}


void BuilderStoreAttrs ( const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, SI::Builder_i * pBuilder, CSphVector<int64_t> & dTmp )
{
	for ( int i = 0; i < dAttrs.GetLength(); i++ )
	{
		const PlainOrColumnar_t & tSrc = dAttrs[i];

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

				pBuilder->SetAttr ( i, dTmp.Begin(), iValues );
			}
			else
				pBuilder->SetAttr ( i, (const int64_t*)pSrc, iValues );
		}
		break;

		case SPH_ATTR_STRING:
		{
			const BYTE * pSrc = nullptr;
			int iBytes = tSrc.Get ( pRow, pPool, dIterators, pSrc );
			pBuilder->SetAttr ( i, (const uint8_t*)pSrc, iBytes );
		}
		break;

		default:
			pBuilder->SetAttr ( i, tSrc.Get ( pRow, dIterators ) );
			break;
		}
	}
}


class SelectProxy_c : public SelectSI_c
{
public:
			SelectProxy_c ( const SI::Index_i * pSidx, const ISphSchema & tSchema, ESphCollation eCollation );

	bool	IsEnabled ( const CSphFilterSettings & tFilter ) const;

private:
	const SI::Index_i *		m_pSidx = nullptr;
	const ISphSchema &		m_tSchema;
	ESphCollation			m_eCollation = SPH_COLLATION_DEFAULT;
};


SelectProxy_c::SelectProxy_c ( const SI::Index_i * pSidx, const ISphSchema & tSchema, ESphCollation eCollation )
	: m_pSidx ( pSidx )
	, m_tSchema ( tSchema )
	, m_eCollation ( eCollation )
{}


bool SelectProxy_c::IsEnabled ( const CSphFilterSettings & tFilter ) const
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

	// FIXME!!! warn in case force index used but index was skipped
	if ( pCol->m_eAttrType==SPH_ATTR_STRING && m_eCollation!=SPH_COLLATION_DEFAULT )
		return false;

	return m_pSidx->IsEnabled( tFilter.m_sAttrName.cstr() );
}


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

