//
//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "secondaryindex.h"

#include <algorithm>

#include "histogram.h"
#include "sphinxint.h"
#include "killlist.h"
#include "attribute.h"
#include "columnarfilter.h"
#include <queue>

#include "util/util.h"
#include "secondarylib.h"


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
	static const int MAX_COLLECTED = 1024;
	CSphFixedVector<RowID_t> m_dCollected {MAX_COLLECTED};
};

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
class IteratorState_T
{
public:
	std::unique_ptr<T>	m_pIterator;

	const RowID_t *		m_pRowID = nullptr;
	const RowID_t *		m_pRowIDMax = nullptr;

	RowIdBoundaries_t	m_tBoundaries;

						IteratorState_T() = default;
						IteratorState_T ( T * pIterator ) : m_pIterator(pIterator) {}

	FORCE_INLINE bool	RewindTo  ( RowID_t tRowID );
	FORCE_INLINE bool	WarmupDocs();
	FORCE_INLINE bool	WarmupDocs ( RowID_t tRowID );
	void				Stop();
	FORCE_INLINE bool	IsStopped() const { return m_bStopped; }

	static inline bool	IsLess ( const IteratorState_T * pA, const IteratorState_T * pB ) { return ( *pA->m_pRowID < *pB->m_pRowID ); }

private:
	bool				m_bStopped = false;
	CSphVector<RowID_t> m_dCollected;
};

template <typename T, bool ROWID_LIMITS>
bool IteratorState_T<T, ROWID_LIMITS>::WarmupDocs()
{
	assert(m_pIterator);
	assert ( !ROWID_LIMITS );	// we assume that underlying iterators do the filtering

	RowIdBlock_t dRowIdBlock;
	if ( !m_pIterator->GetNextRowIdBlock(dRowIdBlock) )
	{
		Stop();
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
			Stop();
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
		Stop();
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
	{
		Stop();
		return false;
	}

	return WarmupDocs();
}

template <typename T, bool ROWID_LIMITS>
void IteratorState_T<T,ROWID_LIMITS>::Stop()
{
	m_pRowID = m_pRowIDMax = nullptr;
	m_bStopped = true;
}

template <typename T, bool ROWID_LIMITS>
bool IteratorState_T<T,ROWID_LIMITS>::RewindTo ( RowID_t tRowID )
{
	assert ( !m_bStopped );

	while ( tRowID>*(m_pRowIDMax-1) )
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
	assert(pRowID);
	return true;
}

//////////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Base_T : public SecondaryIndexIterator_c
{
public:
				RowidIterator_Base_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries );

	int64_t		GetNumProcessed() const override;

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

//////////////////////////////////////////////////////////////////////////

template <typename T, bool ROWID_LIMITS>
class RowidIterator_Intersect_T : public RowidIterator_Base_T<T,ROWID_LIMITS>
{
	using BASE = RowidIterator_Base_T<T,ROWID_LIMITS>;

public:
				RowidIterator_Intersect_T ( T ** ppIterators, int iNumIterators, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool		HintRowID ( RowID_t tRowID ) override;
	bool		GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	void		SetCutoff ( int iCutoff ) override { m_iRowsLeft = iCutoff; }
	bool		WasCutoffHit() const override { return !m_iRowsLeft; }
	void		AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override;

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
bool RowidIterator_Intersect_T<T,ROWID_LIMITS>::HintRowID ( RowID_t tRowID )
{
	if ( BASE::m_dIterators[0].IsStopped() )
		return false;

	return BASE::m_dIterators[0].RewindTo(tRowID);
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
			tFirst.Stop();
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
void RowidIterator_Intersect_T<T,ROWID_LIMITS>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	for ( const auto & i : BASE::m_dIterators )
		i.m_pIterator->AddDesc(dDesc);
}

/////////////////////////////////////////////////////////////////////

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
	void		AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override;

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

	if ( tRowID<=*m_tMerge.Last()->m_pRowID )
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

		if ( !pState->m_pRowID )
			m_tMerge.Pop();
	}

	if ( m_iRowsLeft!=INT_MAX )
	{
		m_iRowsLeft -= pRowID-pRowIdStart;
		assert ( m_iRowsLeft>=0 );
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <typename T, bool ROWID_LIMITS>
void RowidIterator_Union_T<T,ROWID_LIMITS>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	std::vector<common::IteratorDesc_t> dIteratorDesc;
	BASE::m_dIterators[0].m_pIterator->AddDesc(dIteratorDesc);
	dDesc.Add ( { dIteratorDesc[0].m_sAttr.c_str(), dIteratorDesc[0].m_sType.c_str() } );
}

template <>
void RowidIterator_Union_T<RowidIterator_i,true>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	BASE::m_dIterators[0].m_pIterator->AddDesc(dDesc);
}

template <>
void RowidIterator_Union_T<RowidIterator_i,false>::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	BASE::m_dIterators[0].m_pIterator->AddDesc(dDesc);
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
	void	SetCutoff ( int iCutoff ) override { m_pIterator->SetCutoff(iCutoff); }
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

static bool NextSet ( CSphVector<int> & dSet, const CSphVector<SecondaryIndexInfo_t> & dSecondaryIndexes )
{
	for ( int i = 0; i < dSet.GetLength(); i++ )
	{
		int iNumCapabilities = dSecondaryIndexes[i].m_dCapabilities.GetLength();
		if ( !iNumCapabilities )
			continue;

		dSet[i]++;
		if ( dSet[i] >= iNumCapabilities )
			dSet[i] = 0;
		else
			return true;
	}

	return false;
}

static SIDefault_e g_eSIState = SIDefault_e::ENABLED;
void SetSecondaryIndexDefault ( SIDefault_e eState )
{
	g_eSIState = eState;
}

SIDefault_e GetSecondaryIndexDefault ()
{
	return g_eSIState;
}


static bool CheckIndexHint ( const CSphFilterSettings & tFilter, const CSphVector<IndexHint_t> & dHints, SecondaryIndexType_e eType, bool & bForce )
{
	bForce = false;

	for ( const auto & i : dHints )
		if ( i.m_sIndex==tFilter.m_sAttrName && i.m_eType==eType )
		{
			bForce = i.m_bForce;
			return bForce;
		}

	return true;
}


static bool HaveSI ( const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx, bool & bForce )
{
	if ( !tCtx.IsEnabled_SI(tFilter) )
		return false;

	if ( !CheckIndexHint ( tFilter, tCtx.m_tQuery.m_dIndexHints, SecondaryIndexType_e::INDEX, bForce ) )
		return false;

	if ( !IsSecondaryLibLoaded() || GetSecondaryIndexDefault()==SIDefault_e::DISABLED )
		return false;

	// force secondary indexes from non-default files (that would be JSON SI); at least for now
	// the idea is that they should always be faster
	auto * pAttr = tCtx.m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	if ( !pAttr || pAttr->m_eAttrType==SPH_ATTR_JSON )
		bForce = true;

	return true;
}


static bool HaveAnalyzer ( const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx, bool & bForce )
{
	if ( !tCtx.IsEnabled_Analyzer(tFilter) )
		return false;

	return CheckIndexHint ( tFilter, tCtx.m_tQuery.m_dIndexHints, SecondaryIndexType_e::ANALYZER, bForce );
}


static bool HaveLookup ( const CSphFilterSettings & tFilter, const CSphVector<IndexHint_t> & dHints, bool & bForce )
{
	if ( tFilter.m_sAttrName!=sphGetDocidName() )
		return false;

	return CheckIndexHint ( tFilter, dHints, SecondaryIndexType_e::LOOKUP, bForce );
}


static void FetchHistogramInfo ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	if ( !tCtx.m_pHistograms )
		return;

	ARRAY_FOREACH ( i, tCtx.m_dFilters )
	{
		const CSphFilterSettings & tFilter = tCtx.m_dFilters[i];
		const Histogram_i * pHistogram = tCtx.m_pHistograms->Get ( tFilter.m_sAttrName );
		auto & tSIInfo = dSIInfo[i];
		if ( !pHistogram )
		{
			auto * pAttr = tCtx.m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
			tSIInfo.m_iTotalValues = tCtx.m_iTotalDocs;
			tSIInfo.m_iRsetEstimate = tCtx.m_iTotalDocs;
			tSIInfo.m_bUsable = !pAttr || pAttr->m_eAttrType==SPH_ATTR_JSON;
			continue;
		}

		HistogramRset_t tEstimate;
		tSIInfo.m_bUsable = pHistogram->EstimateRsetSize ( tFilter, tEstimate );
		tSIInfo.m_iTotalValues = pHistogram->GetNumValues();
		tSIInfo.m_iRsetEstimate = tSIInfo.m_bUsable ? tEstimate.m_iTotal : tSIInfo.m_iTotalValues;
		tSIInfo.m_bHasHistograms = true;
	}
}


static void MarkAvailableLookup ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, tCtx.m_dFilters )
	{
		if ( !dSIInfo[i].m_bUsable )
			continue;

		bool bForce = false;
		if ( !HaveLookup( tCtx.m_dFilters[i], tCtx.m_tQuery.m_dIndexHints, bForce ) )
			continue;

		dSIInfo[i].m_dCapabilities.Add ( SecondaryIndexType_e::LOOKUP );
		if ( bForce )
			dSIInfo[i].m_eForce = SecondaryIndexType_e::LOOKUP;
	}
}


static void MarkAvailableSI ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	if ( !tCtx.m_pHistograms )
		return;

	ARRAY_FOREACH ( i, tCtx.m_dFilters )
	{
		if ( !dSIInfo[i].m_bUsable )
			continue;

		bool bForce = false;
		if ( !HaveSI ( tCtx.m_dFilters[i], tCtx, bForce ) )
			continue;

		if ( bForce )
			dSIInfo[i].m_eForce = SecondaryIndexType_e::INDEX;

		dSIInfo[i].m_dCapabilities.Add ( SecondaryIndexType_e::INDEX );
	}
}


static void MarkAvailableAnalyzers ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, tCtx.m_dFilters )
	{
		if ( !dSIInfo[i].m_bUsable )
			continue;

		bool bForce = false;
		if ( !HaveAnalyzer ( tCtx.m_dFilters[i], tCtx, bForce ) )
			continue;

		if ( bForce )
			dSIInfo[i].m_eForce = SecondaryIndexType_e::ANALYZER;

		dSIInfo[i].m_dCapabilities.Add ( SecondaryIndexType_e::ANALYZER );

		// this belongs in the CBO, but for now let's just remove the option to evaluate FILTER if ANALYZER is present
		// as ANALYZERs are always faster than FILTERs
		dSIInfo[i].m_dCapabilities.RemoveValue ( SecondaryIndexType_e::FILTER );
		if ( dSIInfo[i].m_eType==SecondaryIndexType_e::FILTER )
			dSIInfo[i].m_eType = SecondaryIndexType_e::ANALYZER;
	}
}


static void MarkAvailableOptional ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, tCtx.m_dFilters )
		if ( tCtx.m_dFilters[i].m_bOptional )
			dSIInfo[i].m_dCapabilities.Add ( SecondaryIndexType_e::NONE );
}


static void RemoveOptionalColumnar ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	// if we have a columnar attribute and it's capabilities are limited to only "filter" and "none"
	// then we disabled columnarscan and secondaryindex via index hints
	// such filters need to be removed
	ARRAY_FOREACH ( i, tCtx.m_dFilters )
	{
		auto & tSIInfo = dSIInfo[i];
		auto & tFilter = tCtx.m_dFilters[i];

		if ( tSIInfo.m_dCapabilities.GetLength()==2 && tSIInfo.m_dCapabilities[0]==SecondaryIndexType_e::FILTER && tSIInfo.m_dCapabilities[1]==SecondaryIndexType_e::NONE && tCtx.IsEnabled_Analyzer(tFilter) )
		{
			tSIInfo.m_dCapabilities.RemoveFast(0);
			tSIInfo.m_eType = SecondaryIndexType_e::NONE;
		}
	}
}


static void ForceSI ( CSphVector<SecondaryIndexInfo_t> & dSIInfo )
{
	for ( auto & i : dSIInfo )
		if ( i.m_eForce!=SecondaryIndexType_e::NONE )
		{
			i.m_dCapabilities.Resize(0);
			i.m_dCapabilities.Add ( i.m_eForce );
			i.m_eType = i.m_eForce;
		}
}


static void DisableRowidFilters ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, dSIInfo )
		if ( tCtx.m_dFilters[i].m_sAttrName=="@rowid" )
			dSIInfo[i].m_dCapabilities.Resize(0);
}


static void FetchPartialColumnarMinMax ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, dSIInfo )
	{
		auto & tSIInfo = dSIInfo[i];
		auto & tFilter = tCtx.m_dFilters[i];

		bool bHaveAnalyzers = tSIInfo.m_dCapabilities.any_of ( []( auto eCapability ){ return eCapability==SecondaryIndexType_e::ANALYZER; } );
		bool bHaveSI		= tSIInfo.m_dCapabilities.any_of ( []( auto eCapability ){ return eCapability==SecondaryIndexType_e::INDEX; } );
		bool bHaveLookups	= tSIInfo.m_dCapabilities.any_of ( []( auto eCapability ){ return eCapability==SecondaryIndexType_e::LOOKUP; } );

		if ( bHaveAnalyzers && ( bHaveSI || bHaveLookups ) )
		{
			// create a single filter and run it through partial columnar minmax
			VecTraits_T<CSphFilterSettings> dFilter = { &tFilter, 1 };

			CreateFilterContext_t tCFCtx;
			tCFCtx.m_pFilters	= &dFilter;
			tCFCtx.m_pMatchSchema = &tCtx.m_tSorterSchema;
			tCFCtx.m_pIndexSchema = &tCtx.m_tIndexSchema;
			tCFCtx.m_pColumnar	= tCtx.m_pColumnar;
			tCFCtx.m_eCollation	= tCtx.m_tQuery.m_eCollation;
			tCFCtx.m_bScan		= true;
			tCFCtx.m_pHistograms= tCtx.m_pHistograms;
			tCFCtx.m_iTotalDocs	= tCtx.m_iTotalDocs;

			CSphString sError, sWarning;
			if ( !sphCreateFilters ( tCFCtx, sError, sWarning ) )
				continue;

			common::Filter_t tColumnarFilter;
			if ( !ToColumnarFilter ( tColumnarFilter, tFilter, tCtx.m_tQuery.m_eCollation, tCtx.m_tIndexSchema, sWarning ) )
				continue;

			tSIInfo.m_iPartialColumnarMinMax = tCtx.m_pColumnar->EstimateMinMax ( tColumnarFilter, *tCFCtx.m_pFilter );
		}
	}
}


static uint32_t CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs, const SelectIteratorCtx_t & tCtx )
{
	uint32_t uNumIterators = 1;
	if ( tCtx.m_tSI.IsEmpty() )
		return uNumIterators;

	common::Filter_t tColumnarFilter;
	CSphString sWarning;
	if ( !ToColumnarFilter ( tColumnarFilter, tFilter, tCtx.m_tQuery.m_eCollation, tCtx.m_tIndexSchema, sWarning ) )
		return 0;

	return tCtx.m_tSI.GetNumIterators(tColumnarFilter);
}


static void FetchNumSIIterators ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	ARRAY_FOREACH ( i, dSIInfo )
	{
		auto & tSIInfo = dSIInfo[i];
		tSIInfo.m_uNumSIIterators = CalcNumSIIterators ( tCtx.m_dFilters[i], tSIInfo.m_iRsetEstimate, tCtx );
	}
}


static void CheckHint ( const IndexHint_t & tHint, const CSphFilterSettings & tFilter, const SecondaryIndexInfo_t & tSIInfo, const SelectIteratorCtx_t & tCtx, StrVec_t & dWarnings )
{
	CSphString sWarning;
	const auto * pAttr = tCtx.m_tIndexSchema.GetAttr ( tHint.m_sIndex.cstr() );
	if ( !pAttr && !sphJsonNameSplit ( tHint.m_sIndex.cstr() ) )
	{
		sWarning.SetSprintf ( "hint error: '%s' attribute not found", tHint.m_sIndex.cstr() );
		dWarnings.Add (sWarning);
		return;
	}

	if ( !tSIInfo.m_bHasHistograms && !tSIInfo.m_bUsable )
	{
		sWarning.SetSprintf ( "hint error: histogram not found for attribute '%s'", tHint.m_sIndex.cstr() );
		dWarnings.Add (sWarning);
	}
	else if ( !tSIInfo.m_bUsable )
	{
		sWarning.SetSprintf ( "hint error: histogram unusable for attribute '%s'", tHint.m_sIndex.cstr() );
		dWarnings.Add (sWarning);
	}

	switch ( tHint.m_eType )
	{
	case SecondaryIndexType_e::LOOKUP:
		if ( tHint.m_sIndex!=sphGetDocidName() )
			dWarnings.Add ( "hint error: DocidIndex can only be applied to 'id' attribute" );
		break;

	case SecondaryIndexType_e::ANALYZER:
		if ( tHint.m_bForce )
		{
			if ( !IsColumnarLibLoaded() )
				dWarnings.Add ( "hint error: columnar library not loaded" );
			else if ( !tCtx.m_pColumnar )
				dWarnings.Add ( "hint error: no columnar storage" );
			else if ( pAttr->m_eAttrType==SPH_ATTR_STRING && tCtx.m_tQuery.m_eCollation!=SPH_COLLATION_DEFAULT )
			{
				sWarning.SetSprintf ( "hint error: unsupported collation; ColumnarScan might be slow for '%s'", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
			else
			{
				const auto * pAttr = tCtx.m_tIndexSchema.GetAttr ( tHint.m_sIndex.cstr()) ;
				if ( !pAttr->IsColumnar() && !pAttr->IsColumnarExpr() )
				{
					sWarning.SetSprintf ( "hint error: attribute '%s' is not columnar", tHint.m_sIndex.cstr() );
					dWarnings.Add(sWarning);
				}
			}
		}
		break;

	case SecondaryIndexType_e::INDEX:
		if ( tHint.m_bForce )
		{
			if ( !IsSecondaryLibLoaded() )
				dWarnings.Add ( "hint error: secondary library not loaded" );
			else if ( GetSecondaryIndexDefault()==SIDefault_e::DISABLED )
				dWarnings.Add ( "hint error: secondary indexes are disabled" );
			else if ( tCtx.m_tSI.IsEmpty() )
				dWarnings.Add ( "hint error: table has no secondary indexes" );
			else if ( !tCtx.m_tSI.IsEnabled ( tHint.m_sIndex ) )
			{
				sWarning.SetSprintf ( "hint error: secondary index disabled for '%s' (attribute was updated?)", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
			else if ( pAttr->m_eAttrType==SPH_ATTR_STRING && tCtx.m_tQuery.m_eCollation!=SPH_COLLATION_DEFAULT )
			{
				sWarning.SetSprintf ( "hint error: unsupported collation; secondary index disabled for '%s'", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
			else if ( pAttr->m_pExpr.Ptr() && !pAttr->IsColumnarExpr() ) 
			{
				sWarning.SetSprintf ( "hint error: attribute is an expression; secondary index disabled for '%s'", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
			else if ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_STRING_LIST && tFilter.m_eType!=SPH_FILTER_RANGE && tFilter.m_eType!=SPH_FILTER_FLOATRANGE )
			{
				sWarning.SetSprintf ( "hint error: unsupported filter type; secondary index disabled for '%s'", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
			else if ( tFilter.m_eMvaFunc==SPH_MVAFUNC_ALL )
			{
				sWarning.SetSprintf ( "hint error: unsupported mva eval type; secondary index disabled for '%s'", tHint.m_sIndex.cstr() );
				dWarnings.Add(sWarning);
			}
		}
		break;

	default:
		break;
	}
}


static void CheckHints ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx, StrVec_t & dWarnings )
{
	const auto & dFilters = tCtx.m_dFilters;
	for ( auto & tHint : tCtx.m_tQuery.m_dIndexHints )
	{
		int iFilter = -1;
		ARRAY_FOREACH ( i, dFilters )
			if ( dFilters[i].m_sAttrName==tHint.m_sIndex )
			{
				iFilter = i;
				break;
			}

		if ( iFilter==-1 )
		{
			CSphString sWarning;
			sWarning.SetSprintf ( "hint error: filter not found for attribute '%s'", tHint.m_sIndex.cstr() );
			dWarnings.Add(sWarning);
		}
		else
			CheckHint ( tHint, dFilters[iFilter], dSIInfo[iFilter], tCtx, dWarnings );
	}

	ARRAY_FOREACH ( i, dFilters )
		for ( auto & tHint : tCtx.m_tQuery.m_dIndexHints )
			if ( tHint.m_sIndex==dFilters[i].m_sAttrName && tHint.m_bForce )
				if ( !dSIInfo[i].m_dCapabilities.any_of ( [&tHint]( auto eSupported ){ return tHint.m_eType==eSupported; } ) )
				{
					CSphString sWarning;
					sWarning.SetSprintf ( "hint error: requested hint type not supported for attribute '%s'", tHint.m_sIndex.cstr() );
					dWarnings.Add(sWarning);
				}
}

/////////////////////////////////////////////////////////////////////

CSphVector<SecondaryIndexInfo_t> SelectIterators ( const SelectIteratorCtx_t & tCtx, float & fBestCost, StrVec_t & dWarnings )
{
	fBestCost = FLT_MAX;

	CSphVector<SecondaryIndexInfo_t> dSIInfo ( tCtx.m_dFilters.GetLength() );
	ARRAY_FOREACH ( i, dSIInfo )
		dSIInfo[i].m_dCapabilities.Add ( SecondaryIndexType_e::FILTER );

	if ( !tCtx.m_pHistograms )
	{
		if ( tCtx.m_tQuery.m_dIndexHints.GetLength() )
			dWarnings.Add ( "index has no histograms; secondary indexes are unavailable" );

		return dSIInfo;
	}

	// no iterators with OR queries
	if ( !tCtx.m_tQuery.m_dFilterTree.IsEmpty() )
	{
		if ( tCtx.m_tQuery.m_dIndexHints.GetLength() )
			dWarnings.Add ( "secondary indexes are not available when using the OR operator between filters" );

		return dSIInfo;
	}

	FetchHistogramInfo ( dSIInfo, tCtx );
	MarkAvailableLookup ( dSIInfo, tCtx );
	MarkAvailableSI ( dSIInfo, tCtx );
	MarkAvailableAnalyzers ( dSIInfo, tCtx );
	MarkAvailableOptional ( dSIInfo, tCtx );
	RemoveOptionalColumnar ( dSIInfo, tCtx );
	ForceSI(dSIInfo);
	DisableRowidFilters ( dSIInfo, tCtx );
	FetchPartialColumnarMinMax ( dSIInfo, tCtx );
	FetchNumSIIterators ( dSIInfo, tCtx );
	CheckHints ( dSIInfo, tCtx, dWarnings );

	CSphVector<int> dCapabilities ( dSIInfo.GetLength() );
	CSphVector<int> dBest ( dSIInfo.GetLength() );
	dCapabilities.ZeroVec();
	dBest.ZeroVec();

	const int MAX_TRIES = 1024;
	for ( int iTry = 0; iTry < MAX_TRIES; iTry++ )
	{
		for ( int i = 0; i < dCapabilities.GetLength(); i++ )
			dSIInfo[i].m_eType = dSIInfo[i].m_dCapabilities.GetLength() ? dSIInfo[i].m_dCapabilities[dCapabilities[i]] : SecondaryIndexType_e::NONE;

		// don't use cutoff if we have more than one filter
		int iCutoff = dSIInfo.GetLength() > 1 ? -1 : tCtx.m_iCutoff;
		
		std::unique_ptr<CostEstimate_i> pCostEstimate ( CreateCostEstimate ( dSIInfo, tCtx, iCutoff ) );
		float fCost = pCostEstimate->CalcQueryCost();
		if ( fCost < fBestCost )
		{
			dBest = dCapabilities;
			fBestCost = fCost;
		}

		if ( !NextSet ( dCapabilities, dSIInfo ) )
			break;
	}

	for ( int i = 0; i < dBest.GetLength(); i++ )
		dSIInfo[i].m_eType = dSIInfo[i].m_dCapabilities.GetLength() ? dSIInfo[i].m_dCapabilities[dBest[i]] : SecondaryIndexType_e::NONE;

	return dSIInfo;
}


const CSphFilterSettings * GetRowIdFilter ( const CSphVector<CSphFilterSettings> & dFilters, RowID_t uTotalDocs, RowIdBoundaries_t & tRowidBounds )
{
	const CSphFilterSettings * pRowIdFilter = nullptr;
	for ( const auto & tFilter : dFilters )
		if ( tFilter.m_sAttrName=="@rowid" )
		{
			pRowIdFilter = &tFilter;
			break;
		}

	if ( pRowIdFilter )
		tRowidBounds = GetFilterRowIdBoundaries ( *pRowIdFilter, uTotalDocs );

	return pRowIdFilter;
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

//////////////////////////////////////////////////////////////////////////

class RowidEmptyIterator_c : public RowidIterator_i
{
public:
			RowidEmptyIterator_c ( const CSphString & sAttr ) : m_sAttr ( sAttr ){}

	bool	HintRowID ( RowID_t tRowID ) override { return false; }
	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override { return false; }
	int64_t	GetNumProcessed() const override { return 0; }
	void	SetCutoff ( int iCutoff ) override {}
	bool	WasCutoffHit() const override { return false; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { m_sAttr, "SecondaryIndex" } ); }

private:
	CSphString m_sAttr;
};

/////////////////////////////////////////////////////////////////////

class SIIteratorCreator_c
{
public:
						SIIteratorCreator_c ( const SIContainer_c & tSI, CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, int iCutoff );

	RowIteratorsWithEstimates_t Create();

private:
	const SIContainer_c &					m_tSI;
	CSphVector<SecondaryIndexInfo_t> &		m_dSIInfo;
	const CSphVector<CSphFilterSettings> &	m_dFilters;
	ESphCollation							m_eCollation;
	const ISphSchema &						m_tSchema;
	RowID_t									m_uRowsCount = 0;
	int										m_iCutoff = 0;

	RowIdBoundaries_t						m_tRowidBounds;
	const CSphFilterSettings *				m_pRowIdFilter = nullptr;

	bool				CreateSIIterators ( std::vector<common::BlockIterator_i *> & dFilterIt, const CSphFilterSettings & tFilter, int64_t iRsetSize );
	RowidIterator_i *	CreateRowIdIteratorFromSI ( std::vector<common::BlockIterator_i *> & dFilterIt, const CSphFilterSettings & tFilter );
};


SIIteratorCreator_c::SIIteratorCreator_c ( const SIContainer_c & tSI, CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, int iCutoff )
	: m_tSI ( tSI )
	, m_dSIInfo ( dSIInfo )
	, m_dFilters ( dFilters )
	, m_eCollation ( eCollation )
	, m_tSchema ( tSchema )
	, m_uRowsCount ( uRowsCount )
	, m_iCutoff ( iCutoff )
	, m_pRowIdFilter ( GetRowIdFilter ( dFilters, uRowsCount, m_tRowidBounds ) )
{}


bool SIIteratorCreator_c::CreateSIIterators ( std::vector<common::BlockIterator_i *> & dFilterIt, const CSphFilterSettings & tFilter, int64_t iRsetSize )
{
	common::RowidRange_t tRange { m_tRowidBounds.m_tMinRowID, m_tRowidBounds.m_tMaxRowID };

	if ( m_iCutoff>=0 )
		iRsetSize = Min ( iRsetSize, m_iCutoff );

	bool bCreated = false;
	common::Filter_t tColumnarFilter;
	CSphString sWarning;
	CSphString sError;
	if ( ToColumnarFilter ( tColumnarFilter, tFilter, m_eCollation, m_tSchema, sWarning ) )
		bCreated = m_tSI.CreateIterators ( dFilterIt, tColumnarFilter, m_pRowIdFilter ? &tRange : nullptr, m_uRowsCount, iRsetSize, m_iCutoff, sError );
	else
		sphWarning ( "secondary index %s: %s", tFilter.m_sAttrName.cstr(), sWarning.cstr() );

	if ( !bCreated )
	{
		// FIXME!!! return as query warning
		sphWarning ( "%s", sError.cstr() );
		for ( auto * pIt : dFilterIt ) { SafeDelete ( pIt ); }
		dFilterIt.resize ( 0 );
		return false;
	}

	if ( !sError.IsEmpty() )
	{
		// FIXME!!! return as query warning
		sphWarning ( "secondary index %s:%s", tFilter.m_sAttrName.cstr(), sError.cstr() );
		sError = "";
	}

	return true;
}


RowidIterator_i * SIIteratorCreator_c::CreateRowIdIteratorFromSI ( std::vector<common::BlockIterator_i *> & dFilterIt, const CSphFilterSettings & tFilter )
{
	RowidIterator_i * pIt = nullptr;
	if ( !dFilterIt.size() )
		pIt = new RowidEmptyIterator_c ( tFilter.m_sAttrName );
	else if ( dFilterIt.size()==1 )
	{
		if ( m_pRowIdFilter )
			pIt = new RowidIterator_Wrapper_T<true> ( dFilterIt[0], &m_tRowidBounds );
		else
			pIt = new RowidIterator_Wrapper_T<false> ( dFilterIt[0] );
	}
	else
	{
		if ( m_pRowIdFilter )
			pIt = new RowidIterator_Union_T<common::BlockIterator_i,true> ( &dFilterIt[0], (int)dFilterIt.size(), &m_tRowidBounds );
		else
			pIt = new RowidIterator_Union_T<common::BlockIterator_i,false> ( &dFilterIt[0], (int)dFilterIt.size() );
	}

	return pIt;
}


RowIteratorsWithEstimates_t SIIteratorCreator_c::Create()
{
	RowIteratorsWithEstimates_t dRes;

	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		auto & tSIInfo = m_dSIInfo[i];
		if ( tSIInfo.m_eType!=SecondaryIndexType_e::INDEX )
			continue;

		int64_t iRsetSize = tSIInfo.m_iRsetEstimate;
		const CSphFilterSettings & tFilter = m_dFilters[i];
		std::vector<common::BlockIterator_i *> dFilterIt;
		if ( !CreateSIIterators ( dFilterIt, tFilter, iRsetSize ) )
			continue;
		
		RowidIterator_i * pIt = CreateRowIdIteratorFromSI ( dFilterIt, tFilter );
		dRes.Add ( { pIt, iRsetSize } );
		tSIInfo.m_bCreated = true;
	}

	return dRes;
}

/////////////////////////////////////////////////////////////////////

bool SIContainer_c::Load ( const CSphString & sFile, CSphString & sError )
{
	SI::Index_i * pIndex = CreateSecondaryIndex ( sFile.cstr(), sError );
	if ( !pIndex )
		return false;

	m_dIndexes.Add ( { std::unique_ptr<SI::Index_i>(pIndex), sFile } );
	return true;
}


bool SIContainer_c::Drop ( const CSphString & sFile, CSphString & sError )
{
	ARRAY_FOREACH ( i, m_dIndexes )
		if ( m_dIndexes[i].m_sFile==sFile )
		{
			m_dIndexes.Remove(i);
			return true;
		}

	sError.SetSprintf ( "secondary index file '%s' not loaded", sFile.cstr() );
	return false;
}


void SIContainer_c::ColumnUpdated ( const CSphString & sAttr )
{
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled ( sAttr.cstr() ) )
			i.m_pIndex->ColumnUpdated ( sAttr.cstr() );
}


bool SIContainer_c::SaveMeta ( CSphString & sError ) const
{
	std::string sTmpError;
	for ( auto & i : m_dIndexes )
		if ( !i.m_pIndex->SaveMeta ( sTmpError ) )
		{
			sError = sTmpError.c_str();
			return false;
		}

	return true;
}


bool SIContainer_c::CreateIterators ( std::vector<common::BlockIterator_i *> & dIterators, const common::Filter_t & tFilter, const common::RowidRange_t * pBounds, uint32_t uMaxValues, int64_t iRsetSize, int iCutoff, CSphString & sError ) const
{
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled ( tFilter.m_sName ) )
		{
			std::string sErrorSTL;
			bool bOk = i.m_pIndex->CreateIterators ( dIterators, tFilter, pBounds, uMaxValues, iRsetSize, iCutoff, sErrorSTL );
			sError = sErrorSTL.c_str();
			return bOk;
		}

	return false;
}


int64_t	SIContainer_c::GetCountDistinct ( const CSphString & sAttr ) const
{
	std::string sAttrSTL = sAttr.cstr();
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled(sAttrSTL) )
			return i.m_pIndex->GetCountDistinct(sAttrSTL);

	return -1;
}


bool SIContainer_c::CalcCount ( uint32_t & uCount, const common::Filter_t & tFilter, uint32_t uMaxValues, CSphString & sError ) const
{
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled ( tFilter.m_sName ) )
		{
			std::string sErrorSTL;
			bool bOk = i.m_pIndex->CalcCount ( uCount, tFilter, uMaxValues, sErrorSTL );
			sError = sErrorSTL.c_str();
			return bOk;
		}

	return false;
}


uint32_t SIContainer_c::GetNumIterators ( const common::Filter_t & tFilter ) const
{
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled ( tFilter.m_sName ) )
			return i.m_pIndex->GetNumIterators(tFilter);

	return 0;
}


bool SIContainer_c::IsEnabled ( const CSphString & sAttr ) const
{
	for ( auto & i : m_dIndexes )
		if ( i.m_pIndex->IsEnabled ( sAttr.cstr() ) )
			return true;

	return false;
}


void SIContainer_c::GetIndexAttrInfo ( std::vector<SI::IndexAttrInfo_t> & dInfo ) const
{
	for ( auto & i : m_dIndexes )
		i.m_pIndex->GetAttrInfo(dInfo);
}


RowIteratorsWithEstimates_t SIContainer_c::CreateSecondaryIndexIterator ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, int iCutoff ) const
{
	// don't use cutoff if we have more than one instance of SecondaryIndex/ColumnarScan/Filter
	int iNumIterators = dSIInfo.count_of ( []( auto & tSI ){ return tSI.m_eType!=SecondaryIndexType_e::NONE; } );
	if ( iNumIterators > 1 )
		iCutoff = -1;

	SIIteratorCreator_c tCreator ( *this, dSIInfo, dFilters, eCollation, tSchema, uRowsCount, iCutoff );
	return tCreator.Create();
}

/////////////////////////////////////////////////////////////////////

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
		if ( tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR && tCol.IsIndexedKNN() )
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


static void GetAttrsProxy ( const ISphSchema & tSchema, common::Schema_t & tSISchema, CSphVector<PlainOrColumnar_t> & dDstAttrs )
{
	int iColumnar = 0;
	for ( const auto & i : tSISchema )
	{
		const CSphColumnInfo * pAttr = tSchema.GetAttr ( i.m_sName.c_str() );
		assert(pAttr);

		dDstAttrs.Add ( PlainOrColumnar_t ( *pAttr, iColumnar ) );
		if ( pAttr->IsColumnar() )
			iColumnar++;
	}
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int64_t iMemoryLimit, const CSphSchema & tSchema, CSphBitvec & tSIAttrs, const CSphString & sFile, int iBufferSize, CSphString & sError )
{
	common::Schema_t tSISchema;
	ConvertSchema ( tSchema, tSISchema, tSIAttrs );
	return CreateSecondaryIndexBuilder ( tSISchema, iMemoryLimit, sFile, iBufferSize, sError );
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int64_t iMemoryLimit, const CSphSchema & tSchema, const CSphString & sFile, CSphVector<PlainOrColumnar_t> & dAttrs, int iBufferSize, CSphString & sError )
{
	common::Schema_t tSISchema;
	CSphBitvec tSIAttrs ( tSchema.GetAttrsCount() );
	ConvertSchema ( tSchema, tSISchema, tSIAttrs );
	GetAttrsProxy ( tSchema, tSISchema, dAttrs );

	return CreateSecondaryIndexBuilder ( tSISchema, iMemoryLimit, sFile, iBufferSize, sError );
}


void BuildStoreSI ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, SI::Builder_i * pBuilder, CSphVector<int64_t> & dTmp )
{
	for ( int i = 0; i < dAttrs.GetLength(); i++ )
	{
		const PlainOrColumnar_t & tSrc = dAttrs[i];

		const BYTE * pSrc = nullptr;
		switch ( tSrc.m_eType )
		{
		case SPH_ATTR_UINT32SET:
			{
				int iValues = tSrc.Get ( tRowID, pRow, pPool, dIterators, pSrc ) / sizeof(DWORD);
				// need a 64-bit array as input. so we need to convert our 32-bit array to 64-bit entries
				dTmp.Resize ( iValues );
				ARRAY_FOREACH ( i, dTmp )
					dTmp[i] = ((DWORD*)pSrc)[i];

				pBuilder->SetAttr ( i, dTmp.Begin(), iValues );
			}
			break;

		case SPH_ATTR_INT64SET:
			{
				int iValues = tSrc.Get ( tRowID, pRow, pPool, dIterators, pSrc ) / sizeof(int64_t);
				pBuilder->SetAttr ( i, (const int64_t*)pSrc, iValues );
			}
			break;

		case SPH_ATTR_FLOAT_VECTOR:
		{
			int iValues = tSrc.Get ( tRowID, pRow, pPool, dIterators, pSrc ) / sizeof(float);
			// need a 64-bit array as input. so we need to convert our 32-bit array to 64-bit entries
			dTmp.Resize ( iValues );
			ARRAY_FOREACH ( i, dTmp )
				dTmp[i] = sphF2DW ( ((float*)pSrc)[i] );

			pBuilder->SetAttr ( i, dTmp.Begin(), iValues );
		}
		break;

		case SPH_ATTR_STRING:
		{
			const BYTE * pSrc = nullptr;
			int iBytes = tSrc.Get ( tRowID, pRow, pPool, dIterators, pSrc );
			pBuilder->SetAttr ( i, (const uint8_t*)pSrc, iBytes );
		}
		break;

		default:
			pBuilder->SetAttr ( i, tSrc.Get ( tRowID, pRow, dIterators ) );
			break;
		}
	}
}
