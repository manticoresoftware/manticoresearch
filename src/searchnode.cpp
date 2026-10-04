//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchnode.h"
#include "fastcount.h"
#include "querycontext.h"
#include "sphinxquery/sphinxquery.h"
#include "sphinxint.h"
#include "sphinxqcache.h"
#include "mini_timer.h"
#include "coroutine.h"
#include "secondaryindex.h"
#include "client_task_info.h"
#include "exact_bm25a_utils.h"

//////////////////////////////////////////////////////////////////////////

#if QDEBUG
#define QDEBUGARG(_arg) _arg
#else
#define QDEBUGARG(_arg)
#endif

//////////////////////////////////////////////////////////////////////////

/// costs for max_predicted_time estimations, in nanoseconds
/// YMMV, defaults were estimated in a very specific environment, and then rounded off
int g_iPredictorCostDoc		= 64;
int g_iPredictorCostHit		= 48;
int g_iPredictorCostSkip	= 2048;
int g_iPredictorCostMatch	= 64;

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V2
//////////////////////////////////////////////////////////////////////////

#define SPH_BM25_K1				1.2f

static const float COST_SCALE = 1.0f/1000000.0f;

static int E1SchemaFieldCount ( const ISphQwordSetup & tSetup )
{
	if ( !tSetup.m_pIndex )
		return 0;
	const auto & tSchema = tSetup.m_pIndex->GetMatchSchema();
	int iIndexed = 0;
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		iIndexed += ( tSchema.GetField(i).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )!=0;
	return iIndexed;
}

static bool E1FieldScopeCoversSchema ( const XQLimitSpec_t & tSpec, const ISphQwordSetup & tSetup )
{
	const int iFieldCount = E1SchemaFieldCount ( tSetup );
	if ( iFieldCount<=0 )
		return false;
	if ( !tSpec.m_bFieldSpec )
		return true;
	FieldMask_t tExpected;
	tExpected.UnsetAll();
	const auto & tSchema = tSetup.m_pIndex->GetMatchSchema();
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		if ( tSchema.GetField(i).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
			tExpected.Set(i);
	return E1MaskMatchesExpected ( tSpec.m_dFieldMask, tExpected );
}

static volatile bool g_bInterruptNow = false;

#if defined(MANTICORE_TEST)
static bool g_bE1TestLastWindow = false;
static uint32_t g_uE1TestLastWindow = 0;
static uint64_t g_uE1TestScratchDeclines = 0;
static uint64_t g_uE1TestDirectExecutorCalls = 0;
static bool g_bE1TestForceGenericRanked = false;
static E1TestRankStats_t g_tE1TestRankStats;

void SetE1TestForceGenericRanked ( bool bForce )
{
	g_bE1TestForceGenericRanked = bForce;
}

void ResetE1TestRankStats ()
{
	g_tE1TestRankStats = E1TestRankStats_t{};
}

E1TestRankStats_t GetE1TestRankStats ()
{
	return g_tE1TestRankStats;
}

void SetE1TestLastWindow ( uint32_t uLastWindow )
{
	g_bE1TestLastWindow = true;
	g_uE1TestLastWindow = uLastWindow;
	g_uE1TestScratchDeclines = 0;
	g_uE1TestDirectExecutorCalls = 0;
}

void ResetE1TestLastWindow ()
{
	g_bE1TestLastWindow = false;
	g_uE1TestLastWindow = 0;
}

uint64_t GetE1TestDirectExecutorCalls ()
{
	return g_uE1TestDirectExecutorCalls;
}

uint64_t GetE1TestScratchDeclines ()
{
	return g_uE1TestScratchDeclines;
}
#endif


static void PrintDocsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtDoc_t * QDEBUGARG(pDocs), const char * QDEBUGARG(sNode), void * QDEBUGARG(pNode), const char * sTerm=nullptr )
{
#if QDEBUG
	StringBuilder_c tRes;
	tRes.Appendf ( "node %s 0x%x:%p getdocs (%d)(%s) = ", sNode ? sNode : "???", iAtomPos, pNode, iCount, ( sTerm ? sTerm : "" ) );
	tRes.StartBlock (", ","[","]\n");
	for ( int i=0; i<iCount; ++i )
		tRes.Appendf ( "0x%x", DWORD ( pDocs[i].m_tRowID ) );
	tRes.FinishBlock ( false );
	printf ( "%s", tRes.cstr() );
#endif

}

static void PrintHitsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtHit_t * QDEBUGARG(pHits), void * QDEBUGARG(pNode) )
{
#if QDEBUG
	StringBuilder_c tRes;
	tRes.Appendf ( "node 0x%x:%p gethits (%d) = ", iAtomPos, pNode, iCount );
	tRes.StartBlock ( ", ", "[", "]\n" );
	for ( int i=0; i<iCount; ++i )
		tRes.Appendf ( "0x%x:0x%x", DWORD ( pHits[i].m_tRowID ), DWORD ( pHits[i].m_uHitpos ) );
	tRes.FinishBlock ( false );
	printf ( "%s\n", tRes.cstr() );
#endif

}


static void DebugIndent ( int iLevel )
{
	while ( iLevel-- )
		printf ( "    " );
}


static FORCE_INLINE bool HasDocs ( const ExtDoc_t * pDoc )
{
	return pDoc && pDoc->m_tRowID!=INVALID_ROWID;
}


static FORCE_INLINE bool HasHits ( const ExtHit_t * pHit )
{
	assert ( pHit );
	return pHit->m_tRowID!=INVALID_ROWID;
}


static FORCE_INLINE bool WarmupDocs ( const ExtDoc_t * & pDoc, ExtNode_i * pNode )
{
	assert(pNode);

	if ( HasDocs(pDoc) )
		return true;

	pDoc = pNode->GetDocsChunk();
	return HasDocs(pDoc);
}


static FORCE_INLINE bool WarmupDocs ( const ExtDoc_t * & pDoc, const ExtHit_t * & pHit, ExtNode_i * pNode )
{
	assert(pNode);

	if ( HasDocs(pDoc) )
		return true;

	pDoc = pNode->GetDocsChunk();
	if ( !HasDocs(pDoc) )
		return false;

	pHit = pNode->GetHits(pDoc);
	return true;
}


static FORCE_INLINE bool WarmupDocs ( const ExtDoc_t * & pDocL, const ExtDoc_t * pDocR, ExtNode_i * pLeft )
{
	assert(pLeft);

	if ( HasDocs(pDocL) )
		return true;

	if ( HasDocs(pDocR)  )
		pLeft->HintRowID ( pDocR->m_tRowID );

	pDocL = pLeft->GetDocsChunk();
	return HasDocs(pDocL);
}


static FORCE_INLINE bool E1FilterValue ( const E1RankFilter_t & tFilter, SphAttr_t iValue )
{
	if ( tFilter.m_eType==SPH_FILTER_VALUES )
		return iValue==tFilter.m_iValue;
	if ( tFilter.m_bOpenLeft )
		return tFilter.m_bHasEqualMax ? iValue<=tFilter.m_iMaxValue : iValue<tFilter.m_iMaxValue;
	if ( tFilter.m_bOpenRight )
		return tFilter.m_bHasEqualMin ? iValue>=tFilter.m_iMinValue : iValue>tFilter.m_iMinValue;
	const bool bMin = tFilter.m_bHasEqualMin ? iValue>=tFilter.m_iMinValue : iValue>tFilter.m_iMinValue;
	const bool bMax = tFilter.m_bHasEqualMax ? iValue<=tFilter.m_iMaxValue : iValue<tFilter.m_iMaxValue;
	return bMin && bMax;
}


static uint64_t E1ApplyFilterMask ( const E1RankFilter_t & tFilter, uint32_t uWindow, uint64_t * pMask, uint64_t & uExamined )
{
	uint64_t uPassed = 0;
	for ( uint32_t uWord=0; uWord<64; ++uWord )
	{
		uint64_t uBits = pMask[uWord];
		uint64_t uKeep = 0;
		while ( uBits )
		{
			const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
			uBits &= uBits-1;
			const uint64_t uRow = uint64_t(uWindow)*4096 + uint64_t(uWord)*64 + uBit;
			++uExamined;
			if ( uRow<tFilter.m_uRows && E1FilterValue ( tFilter, sphGetRowAttr ( tFilter.m_pAttrs+uRow*tFilter.m_iStride, tFilter.m_tLocator ) ) )
			{
				uKeep |= uint64_t(1)<<uBit;
				++uPassed;
			}
		}
		pMask[uWord] = uKeep;
	}
	return uPassed;
}

//////////////////////////////////////////////////////////////////////////

class ExtNode_c : public ExtNode_i
{
public:
	static const int		MAX_HITS = 512;

							ExtNode_c ( int64_t tmTimeout=0 );

	const ExtHit_t *		GetHits ( const ExtDoc_t * pDocs ) override;
	void					DebugDump ( int iLevel ) override;
	void 					SetAtomPos ( int iPos ) override;
	int						GetAtomPos() const override;
	void					SetQPosReverse();
	void					SetMaxTimeout ( int64_t iTimer );
	bool					TimeExceeded() const override;
	int64_t 				GetMaxTimeout() const override;

protected:
	ExtDoc_t				m_dDocs[MAX_BLOCK_DOCS];
	CSphVector<ExtHit_t>	m_dHits;
	bool					m_bQPosReverse {false};
	int						m_iAtomPos {0};		///< we now need it on this level for tricks like expanded keywords within phrases
	const int64_t&			m_iCheckTimePoint { Threads::Coro::GetNextTimePointUS() };
	int64_t					m_iMaxTimer; ///< work until this timestamp

	virtual void			CollectHits ( const ExtDoc_t * pDocs ) = 0;

	inline const ExtDoc_t *	ReturnDocsChunk ( int iCount, const char * sNode, const char * sTerm=nullptr );
	inline const ExtHit_t *	ReturnHitsChunk ( int iCount, const char * sNode, bool bReverse );
	inline const ExtHit_t *	ReturnHits ( bool bReverse );
};

//////////////////////////////////////////////////////////////////////////

// outputs docids returned by rowid iterators
class ExtIterator_c : public ExtNode_c
{
public:
						ExtIterator_c ( RowidIterator_i * pIterator ) : m_pIterator ( pIterator ) { assert ( pIterator ); }

	int					GetQwords ( ExtQwordsHash_t & hQwords )	override { return -1; }
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override {}
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override {}
	uint64_t			GetWordID () const override { return 0; }
	const ExtDoc_t *	GetDocsChunk() override;
	const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) final { return nullptr; }
	void				Reset ( const ISphQwordSetup & tSetup ) override {}
	void				HintRowID ( RowID_t tRowID ) override { m_pIterator->HintRowID(tRowID); }
	bool				GotHitless() override { return false; }
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return { 0.0f, 0, 0 }; }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override {}	// no need for filtering as iterators should output already filtered rowids

protected:
	RowidIterator_i *	m_pIterator = nullptr;	// not owned by the node
	bool				m_bWarmup = true;
	RowIdBlock_t		m_dIteratorRowIDs;
	int					m_iDocOffset = 0;

	void				CollectHits ( const ExtDoc_t * pDocs ) final	{ assert ( 0 && "ExtRowIdRange_c doesn't collect hits" ); }
};


const ExtDoc_t * ExtIterator_c::GetDocsChunk()
{
	if ( m_bWarmup )
	{
		if ( !m_pIterator->GetNextRowIdBlock(m_dIteratorRowIDs) )
			return nullptr;

		m_iDocOffset = 0;
		m_bWarmup = false;
	}

	RowID_t * pStart = m_dIteratorRowIDs.Begin()+m_iDocOffset;
	RowID_t * pEnd = pStart + Min ( MAX_BLOCK_DOCS-1, m_dIteratorRowIDs.GetLength()-m_iDocOffset );
	ExtDoc_t * pDocStart = m_dDocs;
	while ( pStart < pEnd )
		*pDocStart++ = { *pStart++, 0, 0.0f };

	if ( pEnd==m_dIteratorRowIDs.End() )
		m_bWarmup = true;
	else
		m_iDocOffset = pEnd-m_dIteratorRowIDs.Begin();

	return ReturnDocsChunk ( pDocStart-m_dDocs, "filter" );
}

//////////////////////////////////////////////////////////////////////////

/// single keyword streamer
template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
class ExtTerm_T : public ExtNode_c, ISphNoncopyable
{
public:
						ExtTerm_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bFullSchemaScope=false ) { Init ( pQword, dFields, tSetup, bNotWeighted, bFullSchemaScope ); }
						ExtTerm_T ( ISphQword * pQword, const ISphQwordSetup & tSetup );
						ExtTerm_T() { m_dQueriedFields.UnsetAll(); }
						~ExtTerm_T () override
	{
		if ( m_bE1Ranked && getenv("MANTICORE_E1_RANK_TRACE") )
		{
			fprintf ( stderr, "E1_RANKED bound_kind=%s rowid_docid_order=%d tie_public_id_capability=%d bound_entries_read=%llu nonempty_buckets=%llu selected_blocks=%llu skipped_blocks=%llu equality_skipped_blocks=%llu skipped_docs=%llu scored_docs=%llu metadata_groups_decoded=%llu threshold_at_scored=%llu topk_updates=%llu heap_worst_weight=%d heap_worst_id=%llu\n",
				E1RankedBoundKindName(m_eE1BoundKind), int(m_bE1RowidDocidOrder),
				int(m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO),
				(unsigned long long)m_uRankBoundEntries, (unsigned long long)m_uRankBuckets,
				(unsigned long long)m_uRankSelectedBlocks, (unsigned long long)m_uRankSkippedBlocks, (unsigned long long)m_uRankEqualitySkippedBlocks,
				(unsigned long long)m_uRankSkippedDocsTotal, (unsigned long long)m_uRankScored,
				(unsigned long long)m_uRankDecodedGroups, (unsigned long long)m_uRankThresholdAtScored,
				(unsigned long long)m_uRankTopKUpdates, m_iRankThreshold, (unsigned long long)m_uRankWorstTieKey );
			if ( m_tE1Filter.m_bEnabled )
				fprintf ( stderr, "E1_FILTER_TERM masks_built=%llu rows_examined=%llu candidates_before=%llu candidates_after=%llu exact_total=%llu eligibility_bytes=%llu eligibility_build_us=%lld ineligible_before_tf=%llu eligible_rows_scored=%llu generic_filter_bypass=1 persisted_best_first=1\n",
					(unsigned long long)m_uE1FilterMasksBuilt, (unsigned long long)m_uE1FilterRowsExamined,
					(unsigned long long)m_uE1FilterCandidatesBefore, (unsigned long long)m_uE1FilterCandidatesAfter,
					(unsigned long long)m_uE1EligibleTotal, (unsigned long long)m_dE1Eligibility.GetLength()*sizeof(uint64_t),
					(long long)m_iE1EligibilityBuildUS, (unsigned long long)m_uE1IneligibleBeforeTF,
					(unsigned long long)m_uRankScored );
			if ( m_bE1ScopedTerm )
				fprintf ( stderr, "E1_SCOPED_TERM requested_field=%d exact_total=%llu bitmap_bytes=%llu tf_bytes=%llu membership_checks=%llu aggregate_tf_available=%llu field_tf_metadata_hits=%llu field_local_tf_decodes=%llu hitlist_seeks=%llu positions_decoded=%llu hit_bytes=0 hit_bytes_measurable=0 metadata_groups_membership=%llu metadata_groups_ranked=%llu selected_blocks=%llu skipped_blocks=%llu candidates_scored=%llu eligibility_build_us=%lld ineligible_before_tf=%llu aggregate_bound_safe=1 persisted_best_first=1\n",
					m_iE1ScopedField, (unsigned long long)m_uE1ScopedTotal,
					(unsigned long long)m_dE1ScopedEligibility.GetLength()*sizeof(uint64_t),
					(unsigned long long)m_dE1ScopedTF.GetLength()*sizeof(uint32_t),
					(unsigned long long)m_uE1ScopedMembershipChecks, (unsigned long long)m_uE1ScopedAggregateTF,
					(unsigned long long)m_uE1ScopedFieldTFMetadataHits, (unsigned long long)m_uE1ScopedFieldTFDecodes, (unsigned long long)m_uE1ScopedHitlistSeeks,
					(unsigned long long)m_uE1ScopedPositionsDecoded, (unsigned long long)m_uE1ScopedMetadataGroups,
					(unsigned long long)m_uRankDecodedGroups, (unsigned long long)m_uRankSelectedBlocks,
					(unsigned long long)m_uRankSkippedBlocks, (unsigned long long)m_uRankScored,
					(long long)m_iE1EligibilityBuildUS, (unsigned long long)m_uE1IneligibleBeforeTF );
		}
#if defined(MANTICORE_TEST)
		if ( m_bE1Ranked && m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO )
		{
			g_tE1TestRankStats.m_eBoundKind = m_eE1BoundKind;
			g_tE1TestRankStats.m_uSelectedBlocks += m_uRankSelectedBlocks;
			g_tE1TestRankStats.m_uSkippedBlocks += m_uRankSkippedBlocks;
			g_tE1TestRankStats.m_uSkippedDocs += m_uRankSkippedDocsTotal;
			g_tE1TestRankStats.m_uScoredDocs += m_uRankScored;
		}
#endif
		SafeDelete ( m_pQword );
	}

	void				Init ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bFullSchemaScope=false );
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *	GetDocsChunk() override;

	void				CollectHits ( const ExtDoc_t * pMatched ) override;

	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override { return false; }
	int					GetDocsCount() const override { return m_pQword->m_iDocs; }
	int					GetHitsCount() const override { return m_pQword->m_iHits; }
	uint64_t			GetWordID () const override;
	void				HintRowID ( RowID_t tRowID ) override;
	void				SetCollectHits() override { m_bCollectHits = true; }
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return { float(m_pQword->m_iDocs)*COST_SCALE*60.0f, m_pQword->m_iDocs, 1 }; }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;
	bool				EnableE1Ranked() override
	{
		m_eE1BoundKind = m_pQword->GetE1RankedBoundKind();
#if defined(MANTICORE_TEST)
		if ( g_bE1TestForceGenericRanked )
		{
			g_tE1TestRankStats.m_eBoundKind = m_eE1BoundKind;
			++g_tE1TestRankStats.m_uFallbacks;
			return false;
		}
#endif
		if ( getenv("MANTICORE_E1_RANK_TRACE") )
			fprintf ( stderr, "E1_FIELD_SCOPE path=term schema_fields=%d full_schema=%d requested_mask0=%u\n", m_iE1SchemaFields, int(m_bE1FullSchemaScope), unsigned(m_dQueriedFields.GetMask32()) );
		if ( m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO
			&& ( !m_bE1FullSchemaScope || m_fIDF<=0.0f ) )
		{
#if defined(MANTICORE_TEST)
			g_tE1TestRankStats.m_eBoundKind = m_eE1BoundKind;
			++g_tE1TestRankStats.m_uFallbacks;
#endif
			if ( getenv("MANTICORE_E1_RANK_TRACE") )
				fprintf ( stderr, "E1_V7_FALLBACK reason=%s bound_kind=bm25a_ratio\n",
					!m_bE1FullSchemaScope ? "field_scope" : "nonpositive_idf" );
			return false;
		}
		m_iE1ScopedField = E1ScopedSingleField ( m_iE1SchemaFields, m_bE1FullSchemaScope, m_dQueriedFields.GetMask32() );
		m_bE1ScopedTerm = !ROWID_LIMITS && !m_tE1Filter.m_bEnabled && m_iE1ScopedField>=0
			&& !getenv("MANTICORE_E1_FORCE_GENERIC_SCOPED") && BuildE1ScopedEligibility();
		m_bE1Ranked = !ROWID_LIMITS && ( ( m_bE1FullSchemaScope && ( m_tE1Filter.m_bEnabled ? ( m_pQword->E1DirectOrSupported() || m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO ) : m_eE1BoundKind!=E1RankedBoundKind_e::NONE ) ) || m_bE1ScopedTerm );
		if ( m_bE1Ranked && m_tE1Filter.m_bEnabled )
		{
			m_bE1Ranked = m_pQword->GetE1DirectLastWindow ( m_uE1FilterLastWindow );
			m_dE1FilterMask.Resize(64);
			m_dE1Eligibility.Resize ( (m_tE1Filter.m_uRows+63)/64 );
			m_dE1Eligibility.Fill(0);
			const int64_t iStarted = sphMicroTimer();
			uint64_t uIgnoredBoundReads = 0;
			for ( uint32_t uWindow=0; m_bE1Ranked && uWindow<=m_uE1FilterLastWindow; ++uWindow )
			{
				m_dE1FilterMask.Fill(0);
				uint32_t uCardinality = 0, uMaxTF = 0;
				if ( !m_pQword->GetE1DirectWindow ( uWindow, m_dE1FilterMask.Begin(), nullptr, uCardinality, uMaxTF, uIgnoredBoundReads ) )
					continue;
				++m_uE1FilterMasksBuilt;
				m_uE1FilterCandidatesBefore += uCardinality;
				const uint64_t uPassed = E1ApplyFilterMask ( m_tE1Filter, uWindow, m_dE1FilterMask.Begin(), m_uE1FilterRowsExamined );
				m_uE1FilterCandidatesAfter += uPassed;
				m_uE1EligibleTotal += uPassed;
				const uint64_t uWordBase = uint64_t(uWindow)*64;
				for ( uint32_t uWord=0; uWord<64 && uWordBase+uWord<uint64_t(m_dE1Eligibility.GetLength()); ++uWord )
					m_dE1Eligibility[int(uWordBase+uWord)] = m_dE1FilterMask[uWord];
			}
			m_iE1EligibilityBuildUS = sphMicroTimer()-iStarted;
		}
		if ( m_bE1Ranked ) m_uRankTotalDocs=m_pQword->m_iDocs;
#if defined(MANTICORE_TEST)
		if ( m_bE1Ranked && m_tE1Filter.m_bEnabled )
			++g_tE1TestRankStats.m_uDirectFilter;
#endif
		return m_bE1Ranked;
	}
	bool				EnableE1BestFirst() override { return m_bE1Ranked; }
	void				SetRankThreshold ( int iWeight, uint64_t uWorstTieKey ) override { if ( m_eE1BoundKind!=E1RankedBoundKind_e::BM25A_RATIO ) uWorstTieKey = m_bE1RowidDocidOrder ? uWorstTieKey : UINT64_MAX; if ( iWeight!=m_iRankThreshold || uWorstTieKey!=m_uRankWorstTieKey ) { if ( !m_iRankThreshold ) m_uRankThresholdAtScored=m_uRankScored; ++m_uRankTopKUpdates; m_iRankThreshold=iWeight; m_uRankWorstTieKey=uWorstTieKey; } }
	uint64_t			TakeRankSkippedDocs() override
	{
		if ( m_tE1Filter.m_bEnabled || m_bE1ScopedTerm )
		{
			if ( !m_bE1RankFinished || m_bE1EligibleTotalReported )
				return 0;
			m_bE1EligibleTotalReported = true;
			const uint64_t uExactTotal = m_bE1ScopedTerm ? m_uE1ScopedTotal : m_uE1EligibleTotal;
			const uint64_t uRes = uExactTotal>m_uRankScored ? uExactTotal-m_uRankScored : 0;
			m_uRankSkippedDocsTotal += uRes;
			m_uRankSkippedDocs = 0;
			return uRes;
		}
		uint64_t uRes=m_uRankSkippedDocs; m_uRankSkippedDocsTotal+=uRes; m_uRankSkippedDocs=0; return uRes;
	}

	void				DebugDump ( int iLevel ) override;

protected:
	bool				BuildE1ScopedEligibility();
	uint32_t			CountE1FieldTF ( uint64_t uRef );

	struct StoredHit_t
	{
		SphOffset_t	m_tHitlistOffset;
		RowID_t		m_tRowID;
	};

	ISphQword *			m_pQword = nullptr;
	FieldMask_t			m_dQueriedFields;	///< accepted fields mask
	bool				m_bHasWideFields = false;	///< whether fields mask for this term refer to fields 32+
	float				m_fIDF = 0.0f;		///< IDF for this term (might be 0.0f for non-1st occurences in query)
	CSphString *		m_pWarning = nullptr;
	bool				m_bNotWeighted = true;
	CSphQueryStats *	m_pStats = nullptr;
	bool					m_bCollectHits = false;
	bool					m_bE1Ranked = false;
	bool					m_bE1RowidDocidOrder = false;
	bool					m_bE1FullSchemaScope = false;
	E1RankedBoundKind_e	m_eE1BoundKind = E1RankedBoundKind_e::NONE;
	bool				m_bE1ScopedTerm = false;
	int					m_iE1ScopedField = -1;
	int					m_iE1SchemaFields = 0;
	int					m_iRankThreshold = 0;
	uint64_t			m_uRankWorstTieKey = UINT64_MAX;
	uint64_t			m_uRankBoundEntries = 0;
	uint64_t			m_uRankBuckets = 0;
	uint64_t			m_uRankSelectedBlocks = 0;
	uint64_t			m_uRankSkippedBlocks = 0;
	uint64_t			m_uRankEqualitySkippedBlocks = 0;
	uint64_t			m_uRankSkippedDocs = 0;
	uint64_t			m_uRankSkippedDocsTotal = 0;
	uint64_t			m_uRankTotalDocs = 0;
	uint64_t			m_uRankDecodedGroups = 0;
	uint64_t			m_uRankScored = 0;
	uint64_t			m_uRankThresholdAtScored = 0;
	uint64_t			m_uRankTopKUpdates = 0;
	E1RankFilter_t		m_tE1Filter;
	uint32_t			m_uE1FilterLastWindow = UINT32_MAX;
	CSphVector<uint64_t>	m_dE1FilterMask;
	CSphVector<uint64_t>	m_dE1Eligibility;
	CSphVector<uint64_t>	m_dE1ScopedEligibility;
	CSphVector<uint32_t>	m_dE1ScopedTF;
	CSphVector<uint64_t>	m_dE1ScopedWindowMask;
	uint64_t			m_uE1ScopedTotal = 0;
	uint64_t			m_uE1ScopedMembershipChecks = 0;
	uint64_t			m_uE1ScopedAggregateTF = 0;
	uint64_t			m_uE1ScopedFieldTFDecodes = 0;
	uint64_t			m_uE1ScopedFieldTFMetadataHits = 0;
	uint64_t			m_uE1ScopedHitlistSeeks = 0;
	uint64_t			m_uE1ScopedPositionsDecoded = 0;
	uint64_t			m_uE1ScopedMetadataGroups = 0;
	uint64_t			m_uE1FilterMasksBuilt = 0;
	uint64_t			m_uE1FilterRowsExamined = 0;
	uint64_t			m_uE1FilterCandidatesBefore = 0;
	uint64_t			m_uE1FilterCandidatesAfter = 0;
	uint64_t			m_uE1EligibleTotal = 0;
	uint64_t			m_uE1IneligibleBeforeTF = 0;
	int64_t			m_iE1EligibilityBuildUS = 0;
	bool				m_bE1RankFinished = false;
	bool				m_bE1EligibleTotalReported = false;
	RowIdBoundaries_t	m_tBoundaries;

	CSphVector<StoredHit_t> m_dStoredHits;
};


/// single keyword streamer with artificial hitlist
template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
class ExtTermHitless_T : public ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>
{
	using BASE = ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>;

public:
						ExtTermHitless_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted );

	void				CollectHits ( const ExtDoc_t * pMatched ) final;
	bool				GotHitless () final { return true; }
};

//////////////////////////////////////////////////////////////////////////

/// position filter policy
template < TermPosFilter_e T >
class TermAcceptor_T
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t *, const ISphQwordSetup & ) {}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const { return true; }
	inline void					Reset() {}
};

template<>
class TermAcceptor_T<TERM_POS_FIELD_LIMIT> : public ISphNoncopyable
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & );

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const;
	inline void					Reset() {}

private:
	const int					m_iMaxFieldPos;
};

template<>
class TermAcceptor_T<TERM_POS_ZONES> : public ISphNoncopyable
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & tSetup );

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * pHit ) const;
	inline void					Reset();

	ISphZoneCheck *				m_pZoneChecker;				///< zone-limited searches query ranker about zones
	mutable CSphVector<int>		m_dZones;					///< zone ids for this particular term
	mutable RowID_t				m_tLastZoneRowID {INVALID_ROWID};
	mutable int					m_iCheckFrom {0};
};

//////////////////////////////////////////////////////////////////////////

class BufferedNode_c
{
protected:
	const ExtDoc_t *	m_pRawDocs = nullptr;				///< chunk start as returned by raw GetDocsChunk() (need to store it for raw CollectHits() calls)
	const ExtDoc_t *	m_pRawDoc = nullptr;				///< current position in raw docs chunk
	const ExtHit_t *	m_pRawHit = nullptr;				///< current position in raw hits chunk
	ExtDoc_t			m_dMyDocs[MAX_BLOCK_DOCS];			///< all documents within the required pos range
	CSphVector<ExtHit_t> m_dMyHits;							///< all hits within the required pos range


						BufferedNode_c();

	void				Reset();
	void				CopyMatchingHits ( CSphVector<ExtHit_t> & dHits, const ExtDoc_t * pDocs );
};


/// single keyword streamer, with term position filtering
template < TermPosFilter_e T, class NODE >
class ExtConditional_T : public ExtNode_c, public BufferedNode_c, protected TermAcceptor_T<T>
{
	using Acceptor_c = TermAcceptor_T<T>;

public:
	void				Reset ( const ISphQwordSetup & tSetup ) final;
	const ExtDoc_t *	GetDocsChunk() final;
	void				CollectHits ( const ExtDoc_t * pDocs ) final;
	void				HintRowID ( RowID_t tRowID ) override;
	bool				GotHitless () final { return false; }
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID() const override;
	void 				SetAtomPos ( int iPos ) override;
	int					GetAtomPos() const override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return m_tNode.Estimate(iTotalDocs); }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override { m_tNode.SetRowidBoundaries(tBoundaries); }

protected:
	NODE				m_tNode;

						ExtConditional_T ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup );
};

/// single keyword streamer, with term position filtering
template <TermPosFilter_e T, bool USE_BM25, bool ROWID_LIMITS, bool STATS>
class ExtTermPos_T : public ExtConditional_T<T,ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>>
{
public:
	ExtTermPos_T( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
		: ExtConditional_T<T,ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>> ( pQword, pNode, tSetup )
	{
		this->m_tNode.Init ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
};


/// multi-node binary-operation streamer traits
class ExtTwofer_c : public ExtNode_c
{
public:
						ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond );
						ExtTwofer_c () {} ///< to be used in pair with Init();

	void				Init ( ExtNode_i * pLeft, ExtNode_i * pRight );
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override;
	void				HintRowID ( RowID_t tRowID ) override;
	uint64_t			GetWordID() const override;
	void				SetCollectHits() override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;

	void				SetNodePos ( WORD uPosLeft, WORD uPosRight );

protected:
	std::unique_ptr<ExtNode_i> m_pLeft;
	std::unique_ptr<ExtNode_i> m_pRight;
	const ExtDoc_t *	m_pDocL = nullptr;
	const ExtDoc_t *	m_pDocR = nullptr;
	WORD				m_uNodePosL = 0;
	WORD				m_uNodePosR = 0;

	void				DebugDumpT ( const char * sName, int iLevel );
};

/// A-and-B streamer
class ExtAnd_c : public ExtTwofer_c
{
public:
						ExtAnd_c ( ExtNode_i * pLeft, ExtNode_i * pRight );
						ExtAnd_c() {} ///< to be used with Init()

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	int					GetDocsCount() const override { return m_bEmpty ? 0 : ExtTwofer_c::GetDocsCount(); }
	void				DebugDump ( int iLevel ) override;

private:
	bool				m_bEmpty = false;
};

// AND that returns hits from right node only
class ExtAndRightHits_c : public ExtAnd_c
{
public:
						ExtAndRightHits_c ( ExtNode_i * pLeft, ExtNode_i * pRight ) : ExtAnd_c ( pLeft, pRight ) {}

	int					GetQwords ( ExtQwordsHash_t & hQwords ) override	{ assert(m_pRight); return m_pRight->GetQwords(hQwords); }
	const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) override			{ assert(m_pRight); return m_pRight->GetHits(pDocs); }
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				DebugDump ( int iLevel ) override					{ DebugDumpT ( "ExtAndRightHits", iLevel ); }
};


void ExtAndRightHits_c::Reset ( const ISphQwordSetup & tSetup )
{
	assert(m_pRight);
	m_pRight->Reset(tSetup);
	m_pRight.release();
}


template <bool USE_BM25, bool TEST_FIELDS, bool ROWID_LIMITS>
class ExtMultiAnd_T : public ExtNode_c
{
public:
						ExtMultiAnd_T ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup, bool bE1Or=false );
						~ExtMultiAnd_T() override;

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	int					GetDocsCount() const override;
	void				HintRowID ( RowID_t tRowID ) override;
	void				SetCollectHits() override { m_bCollectHits = true; }
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override { m_tBoundaries = tBoundaries; }
	bool				EnableE1Ranked() override;
	bool				EnableE1BestFirst() override { m_bE1BestFirst = m_bE1Or && ( m_dNodes.GetLength()==2 || m_dNodes.GetLength()==4 ); m_bE1BatchedOr2 = m_bE1Or && m_dNodes.GetLength()==2; return m_bE1BestFirst || m_bE1DirectAnd; }
	void				SetRankThreshold ( int iWeight, uint64_t uWorstTieKey ) override { m_iE1RankThreshold = iWeight; m_tE1WorstTiedRow = m_bE1RowidDocidOrder && uWorstTieKey<=UINT32_MAX ? RowID_t(uWorstTieKey) : INVALID_ROWID; m_iE1FinalThreshold = iWeight; }
	uint64_t			TakeRankSkippedDocs() override { auto u=m_uE1SkippedMatches; m_uE1SkippedMatches=0; return u; }

private:
	struct NodeInfo_t
	{
		ISphQword *		m_pQword {nullptr};
		RowID_t			m_tRowID {INVALID_ROWID};
		bool			m_bHitsOver {false};

		float			m_fIDF {0.0f};
		WORD			m_uNodepos {0};
		int				m_iAtomPos {0};
		FieldMask_t		m_dQueriedFields;
		bool			m_bE1FullSchemaScope {false};
		bool			m_bHasWideFields {false};
		bool			m_bNotWeighted {true};

		void			UpdateWideFieldFlag ( const ISphQwordSetup & tSetup );
		bool			FitsFields() const;
	};

	struct StoredMultiHit_t
	{
		CSphFixedVector<SphOffset_t>	m_dHitlistOffsets;
		RowID_t							m_tRowID;

		// don't change to default ctr, centos6 will fail to compile!
		StoredMultiHit_t () : m_dHitlistOffsets ( 0 ) {}
	};

	struct HitInfo_t
	{
		ISphQword * m_pQword;
		WORD		m_uNodePos;
		WORD		m_uQueryPos;
		Hitpos_t	m_uHitpos;
	};

	struct SelectivitySorter_t
	{
		bool IsLess ( const NodeInfo_t & tA, const NodeInfo_t & tB ) const { return tA.m_pQword->m_iDocs < tB.m_pQword->m_iDocs; }
	};

	struct HitWithQpos_t
	{
		int			m_iNode;
		Hitpos_t	m_uHit;
		WORD		m_uQueryPos;

					HitWithQpos_t() = default;
					HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos );

		static bool	IsLess ( const HitWithQpos_t & a, const HitWithQpos_t & b ) { return ( a.m_uHit<b.m_uHit ) || ( a.m_uHit==b.m_uHit && a.m_uQueryPos<=b.m_uQueryPos ); }
	};


	bool							m_bFirstChunk {true};
	bool							m_bCollectHits {false};
	CSphVector<NodeInfo_t>			m_dNodes;
	CSphVector<StoredMultiHit_t>	m_dStoredHits;
	int								m_iNodesSet {0};
	RowIdBoundaries_t				m_tBoundaries;

	CSphString *					m_pWarning {nullptr};
	CSphQueryStats *				m_pStats {nullptr};

	CSphFixedVector<uint64_t>		m_dWordIds;
	CSphQueue<HitWithQpos_t, HitWithQpos_t > m_tQueue;

	FORCE_INLINE bool	AdvanceQwords();
	FORCE_INLINE RowID_t Advance ( int iNode );
	FORCE_INLINE RowID_t Advance ( int iNode, RowID_t tRowID );
	FORCE_INLINE bool	FitsFields ( const NodeInfo_t & tNode ) const;
	FORCE_INLINE DWORD	GetDocFieldsMask() const;
	FORCE_INLINE float	GetTFIDF() const;
	bool				FillE1Window();
	bool				FillE1DirectAndWindow();
	bool				FillE1OrWindow();
	bool				PrepareE1OrWindows();
	bool				BuildE1ScopedAndTerm ( int iNode, uint32_t uLastWindow );
	uint32_t			CountE1ScopedAndFieldTF ( int iNode, uint64_t uRef );
	int					GetQword ( NodeInfo_t & tNode, ExtQwordsHash_t & hQwords );
	FORCE_INLINE void	PushNextHit ( int iNode );
	FORCE_INLINE void	MergeHitsN ( const StoredMultiHit_t & tStoredHit );
	FORCE_INLINE void	MergeHits2 ( const StoredMultiHit_t & tStoredHit );
	FORCE_INLINE void	MergeHits3 ( const StoredMultiHit_t & tStoredHit );

	void				InitHitMerge ( HitInfo_t & tHitInfo, int iNode, const StoredMultiHit_t & tStoredHit );
	void				DoHitMerge ( RowID_t tRowID, HitInfo_t & tLeft, HitInfo_t & tRight );
	void				DoHitMerge ( RowID_t tRowID, HitInfo_t & tHit1, HitInfo_t & tHit2, HitInfo_t & tHit3 );
	void				CopyHits ( RowID_t tRowID, HitInfo_t & tHitInfo, int iNode );
	FORCE_INLINE void	AddHit ( RowID_t tRowID, HitInfo_t & tHit, int iNode );

	static bool			IsHitLess ( const HitInfo_t & tLeft, const HitInfo_t & tRight ) { return tLeft.m_uHitpos<tRight.m_uHitpos || ( tLeft.m_uHitpos==tRight.m_uHitpos && tLeft.m_uQueryPos<=tRight.m_uQueryPos ); }

	static constexpr uint32_t E1_AND_WINDOW_ROWS = 4096;
	bool				m_bE1Ranked = false;
	bool				m_bE1RowidDocidOrder = false;
	bool				m_bE1Or = false;
	int					m_iE1SchemaFields = 0;
	bool				m_bE1DirectAnd = false;
	bool				m_bE1ScopedAnd2 = false;
	bool				m_bE1MixedFieldAnd2 = false;
	int					m_iE1ScopedAndField = -1;
	int					m_dE1ScopedAndFields[2] { -1, -1 };
	bool				m_bE1ScopedOr2 = false;
	bool				m_bE1MixedFieldOr2 = false;
	bool				m_bE1StagedAnd4 = false;
	bool				m_bE1FusedAnd4 = false;
	uint32_t			m_uE1Window = UINT32_MAX;
	uint32_t			m_uE1LastWindow = UINT32_MAX;
	CSphVector<int>		m_dE1Canonical;
	CSphVector<ExtDoc_t>	m_dE1Pending;
	int					m_iE1PendingPos = 0;
	int					m_iE1RankThreshold = 0;
	RowID_t			m_tE1WorstTiedRow = INVALID_ROWID;
	int					m_iE1FinalThreshold = 0;
	uint64_t			m_uE1WindowsTotal = 0;
	uint64_t			m_uE1WindowsPruned = 0;
	uint64_t			m_uE1WindowsScored = 0;
	uint64_t			m_uE1BooleanMatches = 0;
	uint64_t			m_uE1TFProbes = 0;
	uint64_t			m_uE1MetadataGroups = 0;
	uint64_t			m_uE1SkippedMatches = 0;
	uint64_t			m_uE1CandidatesGenerated = 0;
	uint64_t			m_uE1CandidatesScored = 0;
	uint64_t			m_uE1EssentialRepartitions = 0;
	uint64_t			m_uE1EssentialTerms = 0;
	uint64_t			m_uE1NonessentialProbes = 0;
	uint64_t			m_uE1NonessentialHits = 0;
	uint64_t			m_uE1BitmapOps = 0;
	uint64_t			m_uE1SparseSeeks = 0;
	uint64_t			m_uE1ContainerOps = 0;
	uint64_t			m_uE1BoundReads = 0;
	uint64_t			m_uE1RandomTFProbes = 0;
	uint64_t			m_uE1BatchTFRowsRequested = 0;
	uint64_t			m_uE1BatchTFRowsWritten = 0;
	uint64_t			m_uE1BatchMetadataGroups = 0;
	uint64_t			m_uE1CandidateMaskRows = 0;
	uint64_t			m_uE1CandidateBoundRejects = 0;
	uint64_t			m_uE1TieBoundRejects = 0;
	uint64_t			m_uE1CandidateWordsExamined = 0;
	uint64_t			m_uE1BoundClassesRejected = 0;
	uint64_t			m_uE1WholeWordsRejected = 0;
	uint64_t			m_uE1ScalarCandidateInspections = 0;
	uint64_t			m_uE1IntersectionWindows = 0;
	uint64_t			m_uE1IntersectionMaskOps = 0;
	struct E1OrWindowDesc_t { uint32_t m_uWindow; int m_iUpperWeight; uint32_t m_uUnion; };
	CSphVector<E1OrWindowDesc_t>	m_dE1OrWindows;
	int					m_iE1OrWindowPos = 0;
	bool				m_bE1BestFirst = false;
	bool				m_bE1BatchedOr2 = false;
	bool				m_bE1UnboundedCompat = false;
	uint64_t			m_uE1BestFirstVisited = 0;
	uint64_t			m_uE1BestFirstSkipped = 0;
	uint64_t			m_uE1ExactUnionTotal = 0;
	E1RankFilter_t		m_tE1Filter;
	uint64_t			m_uE1FilterMasksBuilt = 0;
	uint64_t			m_uE1FilterRowsExamined = 0;
	uint64_t			m_uE1FilterCandidatesBefore = 0;
	uint64_t			m_uE1FilterCandidatesAfter = 0;
	int					m_dE1TFProbeOrder[4] {};
	int					m_dE1DirectOrder[4] {};
	CSphVector<uint64_t>	m_dE1AndMasks;
	CSphVector<E1DirectBoundWord_t>	m_dE1AndBoundWords;
	CSphVector<uint32_t>	m_dE1StagedExactTF;
	CSphVector<uint64_t>	m_dE1ScopedAndEligibility[2];
	CSphVector<uint32_t>	m_dE1ScopedAndTF[2];
	uint64_t			m_uE1ScopedAndExactTotal = 0;
	uint64_t			m_uE1ScopedOrOverlap = 0;
	uint64_t			m_uE1ScopedAndMembershipChecks = 0;
	uint64_t			m_uE1ScopedAndAggregateTF = 0;
	uint64_t			m_uE1ScopedAndFieldTFDecodes = 0;
	uint64_t			m_uE1ScopedAndHitlistSeeks = 0;
	uint64_t			m_uE1ScopedAndPositionsDecoded = 0;
	uint64_t			m_uE1ScopedAndMembershipGroups = 0;
	uint64_t			m_uE1ScopedAndTFRows = 0;
	int64_t			m_iE1ScopedAndBuildUS = 0;
	uint64_t			m_uE1MissingWindowShortCircuits = 0;
	uint64_t			m_uE1MasksFetched = 0;
	uint64_t			m_uE1FusedWords = 0;
	uint64_t			m_uE1IntersectionNonemptyWords = 0;
	uint64_t			m_uE1ClassCombinations = 0;
	uint64_t			m_uE1CoarseRejects = 0;
	uint64_t			m_uE1CoarseSurvivors = 0;
	uint64_t			m_uE1Stage0Input = 0;
	uint64_t			m_dE1StageRejects[4] {};
	uint64_t			m_dE1StageSurvivors[4] {};
	uint64_t			m_dE1StageBatchRequested[4] {};
	uint64_t			m_dE1StageBatchWritten[4] {};
	uint64_t			m_dE1StageMetadataGroups[4] {};
};


class ExtAndZonespanned_c : public ExtAnd_c
{
	friend class ExtAndZonespan_c;

public:
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;

protected:
	bool				IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const;

	ISphZoneCheck *		m_pZoneChecker = nullptr;
	CSphVector<int>		m_dZones;
};


class ExtAndZonespan_c : public ExtConditional_T < TERM_POS_NONE, ExtAndZonespanned_c >
{
public:
	ExtAndZonespan_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, const XQNode_t * pNode )
		: ExtConditional_T<TERM_POS_NONE,ExtAndZonespanned_c> ( NULL, pNode, tSetup )
	{
		m_tNode.Init ( pFirst, pSecond );
		m_tNode.m_pZoneChecker = tSetup.m_pZoneChecker;
		m_tNode.m_dZones = pNode->m_dSpec.m_dZones;
	}
};


/// A-or-B streamer
class ExtOr_c : public ExtTwofer_c
{
public:
						ExtOr_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
};


/// A-maybe-B streamer
class ExtMaybe_c : public ExtOr_c
{
public:
						ExtMaybe_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				DebugDump ( int iLevel ) override;
};


/// A-and-not-B streamer
class ExtAndNot_c : public ExtTwofer_c
{
public:
						ExtAndNot_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				SetCollectHits() override;
	void				DebugDump ( int iLevel ) override;

protected:
	bool				m_bPassthrough {false};
};


/// generic operator over N nodes
class ExtNWay_c : public ExtNode_c, public BufferedNode_c
{
public:
						ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup );
						~ExtNWay_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	uint64_t			GetWordID() const override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { assert(m_pNode); return m_pNode->Estimate(iTotalDocs); }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override { assert(m_pNode); m_pNode->SetRowidBoundaries(tBoundaries); }

protected:
	ExtNode_i *			m_pNode	{nullptr};					///< my and-node for all the terms
	const ExtDoc_t *	m_pDocs	{nullptr};					///< current docs chunk from and-node
	const ExtHit_t *	m_pHits	{nullptr};					///< current hits chunk from and-node

	inline void			ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup );
};


struct ExtNodeTF_fn
{
	bool IsLess ( ExtNode_i * pA, ExtNode_i * pB ) const
	{
		return pA->GetDocsCount() < pB->GetDocsCount();
	}
};


struct ExtNodeTFExt_fn
{
	const CSphVector<ExtNode_i *> & m_dNodes;

	explicit ExtNodeTFExt_fn ( const CSphVector<ExtNode_i *> & dNodes )
		: m_dNodes ( dNodes )
	{}

	ExtNodeTFExt_fn ( const ExtNodeTFExt_fn & rhs )
		: m_dNodes ( rhs.m_dNodes )
	{}

	bool IsLess ( WORD uA, WORD uB ) const
	{
		return m_dNodes[uA]->GetDocsCount() < m_dNodes[uB]->GetDocsCount();
	}

private:
	const ExtNodeTFExt_fn & operator = ( const ExtNodeTFExt_fn & )
	{
		return *this;
	}
};

/// FSM is Finite State Machine
template < class FSM >
class ExtNWay_T : public ExtNWay_c, private FSM
{
public:
						ExtNWay_T ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;
};


class FSMphrase_c
{
protected:
	struct State_t
	{
		int m_iTagQword;
		DWORD m_uExpHitposWithField;
	};

	CSphVector<int>			m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
	CSphVector<int>			m_dAtomPos;				///< lets use it as finite automata states and keep references on it
	CSphVector<State_t>		m_dStates;				///< pointers to states of finite automata
	DWORD					m_uQposMask {0};


							FSMphrase_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t* pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtPhrase"; }
};

/// exact phrase streamer
using ExtPhrase_c = ExtNWay_T<FSMphrase_c>;

/// proximity streamer
class FSMproximity_c
{
protected:
	int						m_iMaxDistance;
	DWORD					m_uWordsExpected;
	DWORD					m_uMinQpos;
	DWORD					m_uQLen;
	DWORD					m_uExpPos = 0;
	CSphVector<DWORD>		m_dProx; // proximity hit position for i-th word
	CSphVector<int> 		m_dDeltas; // used for weight calculation
	DWORD					m_uWords = 0;
	int						m_iMinQindex = 65535;
	DWORD					m_uQposMask = 0;


							FSMproximity_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtProximity"; }
};

/// exact phrase streamer
using ExtProximity_c = ExtNWay_T<FSMproximity_c>;

/// proximity streamer
class FSMmultinear_c
{
protected:
	int						m_iNear;			///< the NEAR distance
	DWORD					m_uPrelastP = 0;
	DWORD					m_uPrelastML = 0;
	DWORD					m_uPrelastSL = 0;
	DWORD					m_uPrelastW = 0;
	DWORD					m_uLastP = 0;		///< position of the last hit
	DWORD					m_uLastML = 0;		///< the length of the previous hit
	DWORD					m_uLastSL = 0;		///< the length of the previous hit in Query
	DWORD					m_uLastW = 0;		///< last weight
	DWORD					m_uWordsExpected;	///< now many hits we're expect
	DWORD					m_uWeight = 0;		///< weight accum
	DWORD					m_uFirstHit = 0;	///< hitpos of the beginning of the match chain
	WORD					m_uFirstNpos = 0;	///< N-position of the head of the chain
	WORD					m_uFirstQpos = 65535;		///< Q-position of the head of the chain (for twofers)
	CSphVector<WORD>		m_dNpos;			///< query positions for multinear
	CSphVector<ExtHit_t>	m_dRing;			///< ring buffer for multihit data
	int						m_iRing = 0;		///< the head of the ring
	bool					m_bTwofer;			///< if we have 2- or N-way NEAR
	bool					m_bQposMask;


							FSMmultinear_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtMultinear"; }

private:
	inline int				RingTail() const;
	inline void				Add2Ring ( const ExtHit_t * pHit );
	inline void				ShiftRing();
};

/// exact phrase streamer
using ExtMultinear_c = ExtNWay_T<FSMmultinear_c>;

/// quorum streamer
class ExtQuorum_c : public ExtNode_c, public BufferedNode_c
{
	friend struct QuorumNodeAtomPos_fn;

public:
						ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
						~ExtQuorum_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;

	static int			GetThreshold ( const XQNode_t & tNode, int iQwords );

private:
	struct TermTuple_t
	{
		ExtNode_i *			m_pTerm;		///< my children nodes (simply ExtTerm_c for now, not true anymore)
		const ExtDoc_t *	m_pCurDoc;		///< current positions into children doclists
		const ExtHit_t *	m_pCurHit;		///< current positions into children hitlists
		int					m_iCount;		///< terms count in case of dupes
	};

	CSphVector<TermTuple_t>	m_dInitialChildren;			///< my children nodes (simply ExtTerm_c for now)
	CSphVector<TermTuple_t>	m_dChildren;
	int						m_iThresh;					///< keyword count threshold
	// FIXME!!! also skip hits processing for children w\o constrains ( zones or field limit )
	bool					m_bHasDupes;				///< should we analyze hits on docs collecting

	// check for hits that matches and return flag that docs might be advanced
	bool				CollectMatchingHits ( RowID_t tRowID, int iQuorum );
	int					CountQuorum ( bool bFixDupes );
};


/// A-B-C-in-this-order streamer
class ExtOrder_c : public ExtNode_c, public BufferedNode_c
{
public:
								ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup );
								~ExtOrder_c() override;

	void						Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *			GetDocsChunk() override;
	void						CollectHits ( const ExtDoc_t * pDocs ) override;
	int							GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void						SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void						GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool						GotHitless () override { return false; }
	uint64_t					GetWordID () const override;
	void						HintRowID ( RowID_t tRowID ) override;
	NodeEstimate_t				Estimate ( int64_t iTotalDocs ) const override;
	void						SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;

protected:
	CSphVector<ExtNode_i *>		m_dChildren;
	CSphVector<const ExtDoc_t*>	m_dChildDocsChunk;	///< last document chunk (for hit fetching)
	CSphVector<const ExtDoc_t*>	m_dChildDoc;		///< current position in document chunk
	CSphVector<const ExtHit_t*>	m_dChildHit;		///< current position in hits chunk
	bool						m_bDone;

	int							GetChildIdWithNextHit ( RowID_t tRowID );	///< get next hit within given document, and return its child-id
	bool						GetMatchingHits ( RowID_t tRowID );			///< process candidate hits and stores actual matches while we can
};


/// same-text-unit streamer
/// (aka, A and B within same sentence, or same paragraph)
template <bool ROWID_LIMITS>
class ExtUnit_T : public ExtNode_c, public BufferedNode_c
{
public:
						ExtUnit_T ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t& dFields, const ISphQwordSetup & tSetup, const char * sUnit );
						~ExtUnit_T() override;

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;

protected:
	void				FilterHits ( const ExtDoc_t * pDoc1, const ExtDoc_t * pDoc2, const ExtHit_t * & pHit1, const ExtHit_t * & pHit2, const ExtHit_t * & pDotHit, DWORD uSentenceEnd, RowID_t tRowID, int & iDoc );

private:
	ExtNode_i *			m_pArg1 {nullptr};		///< left arg
	ExtNode_i *			m_pArg2 {nullptr};		///< right arg
	ExtNode_i *			m_pDot {nullptr};		///< dot positions

	const ExtDoc_t *	m_pDocs1 {nullptr};		///< last chunk start
	const ExtDoc_t *	m_pDocs2 {nullptr};		///< last chunk start
	const ExtDoc_t *	m_pDotDocs {nullptr};	///< last chunk start
	const ExtDoc_t *	m_pDoc1 {nullptr};		///< current in-chunk ptr
	const ExtDoc_t *	m_pDoc2 {nullptr};		///< current in-chunk ptr
	const ExtDoc_t *	m_pDotDoc {nullptr};	///< current in-chunk ptr
	const ExtHit_t *	m_pHit1 {nullptr};		///< current in-chunk ptr
	const ExtHit_t *	m_pHit2 {nullptr};		///< current in-chunk ptr
	const ExtHit_t *	m_pDotHit {nullptr};	///< current in-chunk ptr

	// need to keep this between GetDocsChunk
	// as one call of GetDocsChunk might fetch m_pDotDocs
	// but fetch m_pDotHit many calls later
	bool m_bNeedDotHits = false;
};


class ExtNotNear_c : public ExtTwofer_c, public BufferedNode_c
{
public:
						ExtNotNear_c ( ExtNode_i * pMust, ExtNode_i * pNot, int iDist );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				DebugDump ( int iLevel ) override;

private:
	const int			m_iDist = 1;
	CSphString			m_sNodeName;
	const ExtHit_t *	m_pHitL = nullptr;
	const ExtHit_t *	m_pHitR = nullptr;

	bool				FilterHits ( RowID_t tRowID, const ExtHit_t * & pMust, const ExtHit_t * & pNot ); // returns true if doc has matched hits
};


//////////////////////////////////////////////////////////////////////////


static ISphQword * CreateQueryWord ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict = nullptr )
{
	if ( !pZonesDict )
		pZonesDict = tSetup.Dict();

	assert ( tSetup.m_tKeywordBuf.GetLength()>=GetKeywordBufSize ( pZonesDict->GetSettings().GetDictFormat() ) );
	BYTE * pWordBuf = tSetup.m_tKeywordBuf.Begin();
	const int iWordBufLen = tSetup.m_tKeywordBuf.GetLength();

	ISphQword * pWord = tSetup.QwordSpawn ( tWord );
	pWord->m_sWord = tWord.m_sWord;

	int iWordBytes = Min ( tWord.m_sWord.Length(), iWordBufLen-1 );
	memcpy ( pWordBuf, tWord.m_sWord.cstr(), iWordBytes );
	pWordBuf[iWordBytes] = '\0';

	pWord->m_uWordID = tWord.m_bMorphed
		? pZonesDict->GetWordIDNonStemmed ( pWordBuf )
		: pZonesDict->GetWordID ( pWordBuf );
	pWord->m_sDictWord.SetBinary ( (const char*)pWordBuf, (int)strlen ( (const char*)pWordBuf ) );

	pWord->m_bExpanded = tWord.m_bExpanded;
	tSetup.QwordSetup ( pWord );

	if ( tWord.m_bFieldStart && tWord.m_bFieldEnd )	pWord->m_iTermPos = TERM_POS_FIELD_STARTEND;
	else if ( tWord.m_bFieldStart )					pWord->m_iTermPos = TERM_POS_FIELD_START;
	else if ( tWord.m_bFieldEnd )					pWord->m_iTermPos = TERM_POS_FIELD_END;
	else											pWord->m_iTermPos = TERM_POS_NONE;

	pWord->m_fBoost = tWord.m_fBoost;
	pWord->m_iAtomPos = tWord.m_iAtomPos;
	return pWord;
}

namespace
{
struct CountTerm_t
{
	const XQKeyword_t * m_pWord;
	FieldMask_t m_tFields;
};

bool CollectCountTerms ( const XQNode_t * pNode, XQOperator_e eOp, CSphVector<CountTerm_t> & dTerms )
{
	if ( !pNode || pNode->m_iOpArg || pNode->m_bVirtuallyPlain || pNode->m_bNotWeighted
		|| pNode->m_dSpec.m_iFieldMaxPos || pNode->m_dSpec.m_bZoneSpan || !pNode->m_dSpec.m_dZones.IsEmpty() )
		return false;
	bool bLeaf = pNode->dChildren().IsEmpty() && pNode->dWords().GetLength()==1;
	if ( pNode->GetOp()!=SPH_QUERY_AND && pNode->GetOp()!=SPH_QUERY_OR )
		return false;
	if ( !bLeaf && pNode->GetOp()!=eOp )
		return false;
	for ( const auto & tWord : pNode->dWords() )
	{
		if ( tWord.m_sWord.IsEmpty() || tWord.m_bFieldStart || tWord.m_bFieldEnd || tWord.m_bExpanded
			|| tWord.m_bExcluded || tWord.m_pPayload || tWord.m_bRegex || tWord.m_iBlendedGroup>=0 )
			return false;
		dTerms.Add ( { &tWord, pNode->m_dSpec.m_dFieldMask } );
	}
	for ( auto pChild : pNode->dChildren() )
		if ( !CollectCountTerms ( pChild, eOp, dTerms ) )
			return false;
	return true;
}
}

bool CanUseFastFullTextCount ( const CSphQuery & tQuery, const XQNode_t * pRoot, const VecTraits_T<ISphMatchSorter*> & dSorters )
{
	if ( tQuery.m_dItems.GetLength()!=1 || tQuery.m_dItems[0].m_sExpr!="count(*)"
		|| !tQuery.m_sGroupBy.IsEmpty() || !tQuery.m_sGroupDistinct.IsEmpty() || !tQuery.m_tHaving.m_sAttrName.IsEmpty()
		|| tQuery.m_sQuery.IsEmpty() || tQuery.HasKnn() || tQuery.m_bHybridSearch || tQuery.m_eJoinType!=JoinType_e::NONE
		|| tQuery.m_bFacet || tQuery.IsInternalQuery() || tQuery.m_bHasOuter || tQuery.m_bExplicitOrderBy
		|| tQuery.m_bZSlist || tQuery.m_iCutoff>0 || !tQuery.m_dFilterTree.IsEmpty()
		|| !tQuery.m_sQueryTokenFilterLib.IsEmpty() || !tQuery.m_dIndexHints.IsEmpty() )
		return false;
	// A pseudo-shard's internal rowid range is not an attribute predicate.
	for ( const auto & tFilter : tQuery.m_dFilters )
		if ( tFilter.m_sAttrName!="@rowid" || tFilter.m_eType!=SPH_FILTER_RANGE || tFilter.m_bExclude )
			return false;
	// Keep syntactically unsupported constructs on the legacy path even if
	// query simplification happens to reduce them to one supported atom.
	if ( strpbrk ( tQuery.m_sQuery.cstr(), "\"~!/*?^$<>=-\\" ) )
		return false;
	if ( dSorters.GetLength()!=1 || dSorters.any_of ( []( auto p ) { return !p->IsGroupby() || p->IsPrecalc() || !p->GetSchema()->GetAttr("@count"); } ) )
		return false;
	CSphVector<CountTerm_t> dTerms;
	return pRoot && CollectCountTerms ( pRoot, pRoot->GetOp(), dTerms ) && !dTerms.IsEmpty();
}

bool IsFullTextCountMetadataTerm ( const XQNode_t * pRoot, const ISphSchema & tSchema )
{
	// Wide hitless terms can decline the count executor after qword setup.
	// Keep those on the existing scheduling path rather than guessing here.
	if ( !pRoot || !pRoot->dChildren().IsEmpty() || pRoot->dWords().GetLength()!=1 || tSchema.GetFieldsCount()>32 )
		return false;
	CSphVector<CountTerm_t> dTerms;
	if ( !CollectCountTerms ( pRoot, pRoot->GetOp(), dTerms ) || dTerms.GetLength()!=1 )
		return false;
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		if ( ( tSchema.GetField(i).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED ) && !dTerms[0].m_tFields.Test(i) )
			return false;
	return true;
}

// Separate instantiations keep OR scratch and codegen off scalar AND/TERM.
template<bool UNION, bool DIRECT=false>
#if _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static bool CountFullTextDocsImpl ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup, const FullTextCountContext_t & tCtx, uint64_t & uCount )
{
	CSphVector<CountTerm_t> dTerms;
	if ( !pRoot || !CollectCountTerms ( pRoot, pRoot->GetOp(), dTerms ) || dTerms.IsEmpty() )
		return false;
	CSphScopedProfile tProfile ( tSetup.m_pCtx ? tSetup.m_pCtx->m_pProfile : nullptr, SPH_QSTATE_FAST_COUNT );
	bool bInterrupted = false;
	auto Checkpoint = [&]
	{
		if ( Threads::Coro::RuntimeExceeded() ) Threads::Coro::RescheduleAndKeepCrashQuery();
		bInterrupted = g_bInterruptNow || session::GetKilled() || sph::TimeExceeded ( tSetup.m_iMaxTimer );
		if ( bInterrupted && tSetup.m_pWarning ) *tSetup.m_pWarning = "query timed out or was interrupted";
		return bInterrupted;
	};
	uCount = 0;
	if ( Checkpoint() ) return true;
	std::vector<std::unique_ptr<ISphQword>> dWords;
	for ( const auto & tTerm : dTerms )
		dWords.emplace_back ( CreateQueryWord ( *tTerm.m_pWord, tSetup ) );
	if ( tSetup.m_bHasWideFields )
		for ( const auto & pWord : dWords )
			if ( !pWord->m_bHasHitlist ) return false;
	uCount = 0;
	if ( dWords.size()==1 && tCtx.m_bStatsExact )
	{
		bool bAllFields = true;
		const auto & tSchema = tSetup.m_pIndex->GetMatchSchema();
		for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
			if ( tSchema.GetField(i).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
				bAllFields &= dTerms[0].m_tFields.Test(i);
		if ( bAllFields )
		{
			uCount = uint64_t(uint32_t(dWords[0]->m_iDocs));
			return true;
		}
	}
	std::vector<RowID_t> dRows ( dWords.size(), INVALID_ROWID );
	uint64_t uSteps = 0;
	auto Advance = [&] ( int i, RowID_t tTarget )
	{
		auto & tWord = *dWords[i];
		if ( bInterrupted || !tWord.m_iDocs ) return dRows[i] = INVALID_ROWID;
		if ( tTarget && (dRows[i]==INVALID_ROWID || tTarget>dRows[i]+1) ) tWord.HintRowID ( tTarget );
		while ( true )
		{
			if ( !(++uSteps & 4095) && Checkpoint() )
				return dRows[i] = INVALID_ROWID;
			RowID_t tRow = tWord.GetNextDoc().m_tRowID;
			if ( tRow==INVALID_ROWID || tRow>tCtx.m_tMaxRowID ) return dRows[i] = INVALID_ROWID;
			if ( tRow<tTarget || (tCtx.m_fnAlive && !tCtx.m_fnAlive(tRow)) ) continue;
			if ( tSetup.m_bHasWideFields ) tWord.CollectHitMask();
			for ( int j=0; j<FieldMask_t::SIZE; ++j )
				if ( tWord.m_dQwordFields[j] & dTerms[i].m_tFields[j] ) return dRows[i] = tRow;
		}
	};
	if ( UNION && pRoot->GetOp()==SPH_QUERY_OR )
	{
		struct CountBatch_t
		{
			RowID_t m_dRows[128];
			int m_iNext = 0;
			int m_iSize = 0;
			// AND's skip-driven alignment can make read-ahead decode unused rows.
			// Restrict batching to the sequential union traversal.
			bool m_bEnabled = true;
			bool m_bEOF = false;
		};
		std::vector<CountBatch_t> dBatches ( dWords.size() );
		const auto & tSchema = tSetup.m_pIndex->GetMatchSchema();
		for ( int i=0; i<(int)dWords.size(); ++i )
		{
			for ( int j=0; j<tSchema.GetFieldsCount(); ++j )
				if ( ( tSchema.GetField(j).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED ) && !dTerms[i].m_tFields.Test(j) )
					dBatches[i].m_bEnabled = false;
		}
		uint64_t uDecodeWork = 0;
		auto AdvanceUnion = [&] ( int i, RowID_t tTarget )
		{
			auto & tWord = *dWords[i];
			if ( bInterrupted || !tWord.m_iDocs ) return dRows[i] = INVALID_ROWID;
			auto & tBatch = dBatches[i];
			if ( tBatch.m_bEnabled )
			{
				// The qword is at the END of the buffered batch. Hint only after
				// its unread rows cannot satisfy the target; never rewind it.
				if ( tBatch.m_iSize && tTarget>tBatch.m_dRows[tBatch.m_iSize-1] )
					tBatch.m_iNext = tBatch.m_iSize;
				while ( true )
				{
					if ( tBatch.m_iNext==tBatch.m_iSize )
					{
						if ( tBatch.m_bEOF ) return dRows[i] = INVALID_ROWID;
						// Charge capacity BEFORE decoding, including short/empty
						// batches. Bound decode work independently of merge work.
						// Checking every 128 rows amplifies 1ms early-expiry yields;
						// use the scalar executor's 4096-posting budget instead.
						uDecodeWork += 128;
						if ( uDecodeWork>=4096 )
						{
							uDecodeWork = 0;
							if ( Checkpoint() ) return dRows[i] = INVALID_ROWID;
						}
						if ( tTarget ) tWord.HintRowID ( tTarget );
						tBatch.m_iSize = tWord.GetCountDocs ( tBatch.m_dRows, 128 );
						tBatch.m_iNext = 0;
						if ( tBatch.m_iSize<0 ) { tBatch.m_bEnabled = false; break; }
						tBatch.m_bEOF = tBatch.m_iSize<128;
						if ( !tBatch.m_iSize ) return dRows[i] = INVALID_ROWID;
					}
					RowID_t tRow = tBatch.m_dRows[tBatch.m_iNext++];
					if ( tRow>tCtx.m_tMaxRowID ) return dRows[i] = INVALID_ROWID;
					if ( tRow>=tTarget && ( !tCtx.m_fnAlive || tCtx.m_fnAlive(tRow) ) ) return dRows[i] = tRow;
				}
			}
			return Advance ( i, tTarget );
		};
		for ( int i=0; i<(int)dWords.size() && !bInterrupted; ++i )
		{
			if ( !(i & 4095) && Checkpoint() ) break;
			AdvanceUnion ( i, tCtx.m_tMinRowID );
		}
		if ( !bInterrupted ) Checkpoint(); // Includes all-empty/EOF initialization.
		// Bounded scratch, independent of segment size. Sparse windows collect
		// at most 32 rows before promotion: do not pay 64 word clears/counts
		// for a handful of widely separated postings. The gate uses actual
		// accepted rows, not global dictionary density (which ignores gaps,
		// field masks, deletions and the current pseudo-shard).
		uint64_t dMask[64] = {};
		RowID_t dSparse[32];
		size_t uMergeWork = 0;
		auto MergeCheckpoint = [&]
		{
			if ( ++uMergeWork<4096 ) return false;
			uMergeWork = 0;
			return Checkpoint();
		};
		RowID_t tNext = INVALID_ROWID;
		for ( auto tRow : dRows )
		{
			if ( MergeCheckpoint() ) break;
			tNext = Min ( tNext, tRow );
		}
		while ( !bInterrupted && tNext!=INVALID_ROWID )
		{
			const RowID_t tBase = tNext & ~RowID_t(4095);
			// Widen before adding: the last rowid window borders INVALID_ROWID.
			const uint64_t uEnd = uint64_t(tBase)+4096;
			tNext = INVALID_ROWID;
			int iSparse = 0;
			bool bDense = false;
			for ( int i=0; i<(int)dRows.size() && !bInterrupted; ++i )
			{
				if ( MergeCheckpoint() ) break;
				while ( dRows[i]!=INVALID_ROWID && uint64_t(dRows[i])<uEnd )
				{
					if ( MergeCheckpoint() ) break;
					const RowID_t tOffset = dRows[i]-tBase;
					if ( !bDense && iSparse==32 )
					{
						for ( auto tSparse : dSparse ) dMask[tSparse>>6] |= uint64_t(1)<<(tSparse&63);
						bDense = true;
					}
					if ( bDense ) dMask[tOffset>>6] |= uint64_t(1)<<(tOffset&63);
					else dSparse[iSparse++] = tOffset;
					if constexpr ( DIRECT )
					{
						RowID_t tLast = dRows[i];
						auto & tBatch = dBatches[i];
						// The last buffered/pending row was admitted above. Only now is
						// the cursor at a clean boundary with no lookahead to lose/replay.
						if ( tBatch.m_bEnabled && !tBatch.m_bEOF && tBatch.m_iNext==tBatch.m_iSize )
							while ( !bInterrupted )
							{
								// Independent precharges, including declined/EOF attempts.
								uDecodeWork += 128;
								if ( uDecodeWork>=4096 )
								{
									uDecodeWork = 0;
									if ( Checkpoint() ) break;
								}
								uMergeWork += 128+32;
								if ( uMergeWork>=4096 )
								{
									uMergeWork = 0;
									if ( Checkpoint() ) break;
								}
								if ( !tCtx.m_fnE1Window ( *dWords[i], tBase, dMask, tLast ) ) break;
								if ( !bDense )
								{
									for ( int j=0; j<iSparse; ++j ) dMask[dSparse[j]>>6] |= uint64_t(1)<<(dSparse[j]&63);
									bDense = true;
								}
							}
						if ( bInterrupted ) break;
						AdvanceUnion ( i, tLast+1 );
					} else
						AdvanceUnion ( i, dRows[i]+1 );
				}
				// One minimum reduction per WINDOW, never per union result.
				tNext = Min ( tNext, dRows[i] );
			}
			// Count even an interrupted partial window: only verified live,
			// in-range rows entered scratch. Fixed-size work remains bounded.
			if ( bDense )
			{
				for ( auto & uWord : dMask )
				{
					uCount += sphBitCount ( uWord );
					uWord = 0;
				}
			} else
			{
				std::sort ( dSparse, dSparse+iSparse );
				uCount += std::unique ( dSparse, dSparse+iSparse )-dSparse;
			}
			// Include sparse sorting/promotion/popcount and EOF in the bound.
			// This is at most 32 rows or 64 words, regardless of fanout.
			if ( !bInterrupted && Checkpoint() ) break;
		}
	} else
	{
		for ( int i=0; i<(int)dWords.size(); ++i ) Advance ( i, tCtx.m_tMinRowID );
		int iLead = 0;
		for ( int i=1; i<(int)dWords.size(); ++i ) if ( dWords[i]->m_iDocs<dWords[iLead]->m_iDocs ) iLead=i;
		while ( dRows[iLead]!=INVALID_ROWID )
		{
			RowID_t tRow = dRows[iLead];
			bool bMatch = true;
			for ( int i=0; i<(int)dWords.size(); ++i )
			{
				if ( dRows[i]<tRow ) Advance ( i, tRow );
				if ( dRows[i]>tRow )
				{
					if ( dRows[i]==INVALID_ROWID ) dRows[iLead]=INVALID_ROWID;
					else Advance ( iLead, dRows[i] );
					bMatch = false;
					break;
				}
			}
			if ( bMatch ) { ++uCount; Advance ( iLead, tRow+1 ); }
		}
	}
	if ( bInterrupted && tSetup.m_pWarning ) *tSetup.m_pWarning = "query timed out or was interrupted";
	return true;
}

static bool CountPrimaryContainers(const XQNode_t *pRoot,const ISphQwordSetup &tSetup,const FullTextCountContext_t &tCtx,uint64_t &uCount)
{
 CSphVector<CountTerm_t> terms;
 if(!pRoot || !tCtx.m_bStatsExact || tCtx.m_tMinRowID || !CollectCountTerms(pRoot,pRoot->GetOp(),terms) || terms.GetLength()<2 || terms.GetLength()>32)return false;
 bool isAnd=pRoot->GetOp()==SPH_QUERY_AND;
 const auto &schema=tSetup.m_pIndex->GetMatchSchema();
 for(const auto &t:terms)for(int i=0;i<schema.GetFieldsCount();++i)if((schema.GetField(i).m_uFieldFlags&CSphColumnInfo::FIELD_INDEXED)&&!t.m_tFields.Test(i))return false;
 std::vector<e1::TermView> frequent;
 std::vector<std::unique_ptr<ISphQword>> rare;
 bool present=false;
 for(const auto &t:terms){
  std::unique_ptr<ISphQword> word(CreateQueryWord(*t.m_pWord,tSetup));
  if(word->m_iDocs<0)return false;
  if(!word->m_iDocs){frequent.push_back({});continue;}
  auto v=tCtx.m_fnPrimaryView(*word);
  if(!v.term || v.DF()!=uint32_t(word->m_iDocs))return false;
  if(!v.Frequent()){if(isAnd)return false;rare.push_back(std::move(word));}
  else {frequent.push_back(v);present=true;}
 }
 if(!present)return false;
 CSphScopedProfile profile(tSetup.m_pCtx?tSetup.m_pCtx->m_pProfile:nullptr,SPH_QSTATE_FAST_COUNT);
 *tCtx.m_pPrimaryUsed=true;
 auto check=[&]{if(Threads::Coro::RuntimeExceeded())Threads::Coro::RescheduleAndKeepCrashQuery();bool stop=g_bInterruptNow||session::GetKilled()||sph::TimeExceeded(tSetup.m_iMaxTimer);if(stop&&tSetup.m_pWarning)*tSetup.m_pWarning="query timed out or was interrupted";return stop;};
 uCount=e1::Count(frequent,isAnd,rare.size(),[&](size_t i){return rare[i]->GetNextDoc().m_tRowID;},check);
 return true;
}

bool CountFullTextDocs ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup, const FullTextCountContext_t & tCtx, uint64_t & uCount )
{
	if(tCtx.m_fnPrimaryView && CountPrimaryContainers(pRoot,tSetup,tCtx,uCount))return true;
	if ( pRoot && pRoot->GetOp()==SPH_QUERY_OR )
	{
		if ( tCtx.m_fnE1Window )
			return CountFullTextDocsImpl<true, true> ( pRoot, tSetup, tCtx, uCount );
		return CountFullTextDocsImpl<true, false> ( pRoot, tSetup, tCtx, uCount );
	}
	return CountFullTextDocsImpl<false> ( pRoot, tSetup, tCtx, uCount );
}

void PushFullTextCount ( uint64_t uCount, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphRowitem * pStatic, int iTag )
{
	// One aggregate row per worker, not one match per posting. Implicit group
	// sorters already merge 64-bit @count through PushGrouped()/MoveTo().
	if ( !uCount ) return;
	for ( auto pSorter : dSorters )
	{
		const auto & tSchema = *pSorter->GetSchema();
		CSphMatch tMatch;
		tMatch.Reset ( tSchema.GetDynamicSize() );
		tMatch.m_tRowID = 0;
		tMatch.m_pStatic = pStatic;
		tMatch.m_iTag = iTag;
		tMatch.SetAttr ( tSchema.GetAttr("@count")->m_tLocator, uCount );
		tMatch.SetAttr ( tSchema.GetAttr("@groupby")->m_tLocator, 1 );
		pSorter->PushGrouped ( tMatch, true );
	}
}

struct AtomPosQWord_fn
{
	bool operator () ( ISphQword * ) const { return true; }
};

struct AtomPosExtNode_fn
{
	bool operator () ( ExtNode_i * pNode ) const { return !pNode->GotHitless(); }
};

template <typename T, typename NODE_CHECK>
int CountAtomPos ( const CSphVector<T *> & dNodes, const NODE_CHECK & fnCheck )
{
	if ( dNodes.GetLength()<2 )
		return dNodes.GetLength();

	int iMinPos = INT_MAX;
	int iMaxPos = 0;
	ARRAY_FOREACH ( i, dNodes )
	{
		T * pNode = dNodes[i];
		if ( fnCheck ( pNode ) )
		{
			iMinPos = Min ( pNode->GetAtomPos(), iMinPos );
			iMaxPos = Max ( pNode->GetAtomPos(), iMaxPos );
		}
	}
	if ( iMinPos==INT_MAX )
		return 0;

	CSphBitvec dAtomPos ( iMaxPos - iMinPos + 1 );
	ARRAY_FOREACH ( i, dNodes )
	{
		if ( fnCheck ( dNodes[i] ) )
			dAtomPos.BitSet ( dNodes[i]->GetAtomPos() - iMinPos );
	}

	return dAtomPos.BitCount();
}


template < typename T >
static ExtNode_i * CreateMultiNode ( const XQNode_t * pQueryNode, const ISphQwordSetup & tSetup, bool bNeedsHitlist, bool bUseBM25, const RowIdBoundaries_t * pBoundaries )
{
	///////////////////////////////////
	// virtually plain (expanded) node
	///////////////////////////////////
	assert ( pQueryNode );
	if ( pQueryNode->dChildren().GetLength() )
	{
		CSphVector<ExtNode_i *> dNodes;
		CSphVector<ExtNode_i *> dTerms;
		ARRAY_FOREACH ( i, pQueryNode->dChildren() )
		{
			ExtNode_i * pTerm = ExtNode_i::Create ( pQueryNode->dChildren()[i], tSetup, bUseBM25, pBoundaries );
			assert ( !pTerm || pTerm->GetAtomPos()>=0 );
			if ( pTerm )
			{
				if ( !pTerm->GotHitless() )
					dNodes.Add ( pTerm );
				else
					dTerms.Add ( pTerm );
			}
		}

		int iAtoms = CountAtomPos ( dNodes, AtomPosExtNode_fn() );
		if ( iAtoms<2 )
		{
			ARRAY_FOREACH ( i, dNodes )
				SafeDelete ( dNodes[i] );
			ARRAY_FOREACH ( i, dTerms )
				SafeDelete ( dTerms[i] );
			if ( tSetup.m_pWarning )
				tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)", iAtoms, pQueryNode->dChildren().GetLength() );
			return NULL;
		}

		// FIXME! tricky combo again
		// quorum+expand used KeywordsEqual() path to drill down until actual nodes
		ExtNode_i * pResult = new T ( dNodes, *pQueryNode, tSetup );

		// AND result with the words that had no hitlist
		if ( dTerms.GetLength () )
		{
			pResult = new ExtAnd_c ( pResult, dTerms[0] );
			for ( int i=1; i<dTerms.GetLength (); i++ )
				pResult = new ExtAnd_c ( pResult, dTerms[i] );
		}

		if ( pQueryNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );

		return pResult;
	}

	//////////////////////
	// regular plain node
	//////////////////////

	ExtNode_i * pResult = NULL;
	CSphVector<ISphQword *> dQwordsHit;	// have hits
	CSphVector<ISphQword *> dQwords;	// don't have hits

	// partition phrase words
	const CSphVector<XQKeyword_t> & dWords = pQueryNode->dWords();
	ARRAY_FOREACH ( i, dWords )
	{
		ISphQword * pWord = CreateQueryWord ( dWords[i], tSetup );
		if ( pWord->m_bHasHitlist || !bNeedsHitlist )
			dQwordsHit.Add ( pWord );
		else
			dQwords.Add ( pWord );
	}

	// see if we can create the node
	int iAtoms = CountAtomPos ( dQwordsHit, AtomPosQWord_fn() );
	if ( iAtoms<2 )
	{
		ARRAY_FOREACH ( i, dQwords )
			SafeDelete ( dQwords[i] );
		ARRAY_FOREACH ( i, dQwordsHit )
			SafeDelete ( dQwordsHit[i] );
		if ( tSetup.m_pWarning )
			tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)",
				iAtoms, dWords.GetLength() );
		return NULL;

	} else
	{
		// at least two words have hitlists, creating phrase node
		assert ( pQueryNode->dWords().GetLength() );
		assert ( pQueryNode->GetOp()==SPH_QUERY_PHRASE || pQueryNode->GetOp()==SPH_QUERY_PROXIMITY || pQueryNode->GetOp()==SPH_QUERY_QUORUM );

		// create nodes
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, dQwordsHit )
		{
			dNodes.Add ( ExtNode_i::Create ( dQwordsHit[i], pQueryNode, tSetup, bUseBM25, pBoundaries ) );
			dNodes.Last()->SetAtomPos ( dQwordsHit[i]->m_iAtomPos );
		}

		pResult = new T ( dNodes, *pQueryNode, tSetup );
	}

	// AND result with the words that had no hitlist
	if ( dQwords.GetLength() )
	{
		ExtNode_i * pNode = ExtNode_i::Create ( dQwords[0], pQueryNode, tSetup, bUseBM25, pBoundaries );
		for ( int i=1; i<dQwords.GetLength(); i++ )
			pNode = new ExtAnd_c ( pNode, ExtNode_i::Create ( dQwords[i], pQueryNode, tSetup, bUseBM25, pBoundaries ) );

		pResult = new ExtAnd_c ( pResult, pNode );
	}

	if ( pQueryNode->GetCount() )
		return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );

	return pResult;
}

// Dedicated E1/5 exact phrase executor. It intersects primary containers and
// verifies positional existence before exposing an exact-TF document to BM25A.
class ExtE1Phrase_c final : public ExtNode_c
{
public:
	struct Input_t { const XQKeyword_t * m_pWord; FieldMask_t m_dFields; bool m_bPhrase; };
	ExtE1Phrase_c ( const CSphVector<Input_t> & dInput, int iField, const ISphQwordSetup & tSetup )
		: ExtNode_c ( tSetup.m_iMaxTimer ), m_iField ( iField ), m_pStats ( tSetup.m_pStats )
	{
		m_dNodes.Resize ( dInput.GetLength() );
		for ( int i=0; i<dInput.GetLength(); ++i )
		{
			m_dNodes[i].m_pQword = CreateQueryWord ( *dInput[i].m_pWord, tSetup );
			m_dNodes[i].m_iAtomPos = m_dNodes[i].m_pQword->m_iAtomPos;
			m_dNodes[i].m_bPhrase = dInput[i].m_bPhrase;
		}
		m_dCurrentMeta.Resize ( dInput.GetLength() );
	}
	~ExtE1Phrase_c() override
	{
		if ( m_bEnabled && getenv("MANTICORE_E1_RANK_TRACE") )
			fprintf ( stderr, "E1_DIRECT_PHRASE terms=%d direct_candidates=%llu positional_docs_verified=%llu exact_phrase_matches=%llu phrase_occurrences_full=%llu hitlist_seeks=%llu decoded_positions_term0=%llu decoded_positions_term1=%llu existence_only_docs=%llu full_count_docs=%llu decoded_positions_existence_term0=%llu decoded_positions_existence_term1=%llu decoded_positions_full_term0=%llu decoded_positions_full_term1=%llu selected_meta_rows=%llu selected_meta_groups=%llu bound_lookup_hits=%llu bound_lookup_fallbacks=%llu dl_reads=%llu scored_rows=%llu pre_bound_rejects=%llu post_count_bound_rejects=%llu generic_hits_bypassed=%llu folded_hits_bypassed=%llu windows=%llu\n",
				m_dNodes.GetLength(), (unsigned long long)m_uDirectCandidates,
				(unsigned long long)m_uPositionalDocsVerified, (unsigned long long)m_uExactPhraseMatches,
				(unsigned long long)m_uPhraseOccurrences,
				(unsigned long long)m_uHitlistSeeks, (unsigned long long)m_dDecodedPositions[0],
				(unsigned long long)m_dDecodedPositions[1], (unsigned long long)m_uExistenceOnlyDocs,
				(unsigned long long)m_uFullCountDocs, (unsigned long long)m_dDecodedPositionsByMode[0][0],
				(unsigned long long)m_dDecodedPositionsByMode[0][1], (unsigned long long)m_dDecodedPositionsByMode[1][0],
				(unsigned long long)m_dDecodedPositionsByMode[1][1],
				(unsigned long long)m_uSelectedMetaRows, (unsigned long long)m_uSelectedMetaGroups,
				(unsigned long long)m_uBoundLookupHits, (unsigned long long)m_uBoundLookupFallbacks,
				(unsigned long long)(m_uExactPhraseMatches-m_uEarlyBoundRejects-m_uPostCountBoundRejects), (unsigned long long)(m_uExactPhraseMatches-m_uEarlyBoundRejects-m_uPostCountBoundRejects),
				(unsigned long long)m_uEarlyBoundRejects, (unsigned long long)m_uPostCountBoundRejects,
				(unsigned long long)m_uExactPhraseMatches, (unsigned long long)m_uExactPhraseMatches,
				(unsigned long long)m_uWindows );
		for ( auto & tNode : m_dNodes ) SafeDelete ( tNode.m_pQword );
	}
	bool EnableE1Ranked() override
	{
		if ( m_bEnabled ) return true;
#if defined(MANTICORE_TEST)
		if ( g_bE1TestForceGenericRanked ) { ++g_tE1TestRankStats.m_uFallbacks; return false; }
#endif
		if ( m_dNodes.GetLength()<2 || m_dNodes.GetLength()>3 ) return false;
		uint32_t uLast = UINT32_MAX;
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			Node_t & tNode=m_dNodes[i];
			if ( !tNode.m_pQword->E1DirectContainerSupported() ) return false;
			uint32_t uTermLast=0; if ( !tNode.m_pQword->GetE1DirectLastWindow(uTermLast) ) return false;
			uLast = uLast==UINT32_MAX ? uTermLast : Min(uLast,uTermLast);
			if ( tNode.m_bPhrase ) { if(m_iPhrase[0]<0)m_iPhrase[0]=i;else if(m_iPhrase[1]<0)m_iPhrase[1]=i;else return false; }
			for ( int j=0; j<i; ++j ) if ( tNode.m_pQword->m_sWord==m_dNodes[j].m_pQword->m_sWord ) return false;
		}
		if ( m_iPhrase[1]<0 ) return false;
		if ( m_dNodes[m_iPhrase[0]].m_iAtomPos>m_dNodes[m_iPhrase[1]].m_iAtomPos ) Swap(m_iPhrase[0],m_iPhrase[1]);
		m_iPhraseDelta=m_dNodes[m_iPhrase[1]].m_iAtomPos-m_dNodes[m_iPhrase[0]].m_iAtomPos; if(m_iPhraseDelta<=0)return false;
		m_dCanonical.Resize(m_dNodes.GetLength()); for(int i=0;i<m_dNodes.GetLength();++i)m_dCanonical[i]=i;
		std::sort(m_dCanonical.Begin(),m_dCanonical.End(),[this](int a,int b){return m_dNodes[a].m_iAtomPos<m_dNodes[b].m_iAtomPos;});
		for(int i=0;i<m_dCanonical.GetLength();++i)if(m_dNodes[m_dCanonical[i]].m_iAtomPos!=i+1)return false;
		m_uLastWindow=uLast; m_bEnabled=true;
#if defined(MANTICORE_TEST)
		++g_tE1TestRankStats.m_uDirectPhrase;
#endif
		return true;
	}
	bool EnableE1BestFirst() override { return m_bEnabled; }
	void SetRankThreshold ( int iWeight, uint64_t uWorstTieKey ) override { m_iRankThreshold=iWeight; m_tWorstTiedRow=uWorstTieKey<=UINT32_MAX?RowID_t(uWorstTieKey):INVALID_ROWID; }
	uint64_t TakeRankSkippedDocs() override { const uint64_t uSkipped=m_uSkippedMatches; m_uSkippedMatches=0; return uSkipped; }
	const ExtDoc_t * GetDocsChunk() override
	{
		while(m_iPending>=m_dPending.GetLength()){if(!FillWindow())return nullptr;if(m_dPending.IsEmpty())continue;}
		int n=0;while(n<MAX_BLOCK_DOCS-1&&m_iPending<m_dPending.GetLength())m_dDocs[n++]=m_dPending[m_iPending++];
		if(m_pStats)m_pStats->m_iFetchedDocs+=n;return ReturnDocsChunk(n,"e1-phrase");
	}
	void Reset ( const ISphQwordSetup & tSetup ) override
	{
		m_uWindow=0;m_iPending=0;m_dPending.Resize(0);SetMaxTimeout(tSetup.m_iMaxTimer);
		for(auto & n:m_dNodes){n.m_pQword->Reset();tSetup.QwordSetup(n.m_pQword);}
	}
	void HintRowID(RowID_t) override {}
	bool GotHitless() override { return false; }
	int GetDocsCount() const override { int n=INT_MAX;for(const auto & x:m_dNodes)n=Min(n,x.m_pQword->m_iDocs);return n; }
	uint64_t GetWordID() const override
	{
		CSphVector<uint64_t> d(m_dNodes.GetLength());for(int i=0;i<m_dNodes.GetLength();++i)d[i]=m_dNodes[i].m_pQword->m_uWordID?m_dNodes[i].m_pQword->m_uWordID:sphFNV64(m_dNodes[i].m_pQword->m_sDictWord.cstr());return sphFNV64(d.Begin(),d.GetLengthBytes());
	}
	NodeEstimate_t Estimate(int64_t) const override { return {float(GetDocsCount())*COST_SCALE*60.0f,GetDocsCount(),m_dNodes.GetLength()}; }
	void SetRowidBoundaries(const RowIdBoundaries_t &) override {}
	void SetCollectHits() override {}
	void DebugDump(int iLevel) override { DebugIndent(iLevel);printf("ExtE1Phrase\n"); }
	int GetQwords(ExtQwordsHash_t & h) override
	{
		int mx=-1;for(auto & n:m_dNodes){n.m_fIDF=0.0f;ExtQword_t * p=h(n.m_pQword->m_sWord);if(p&&!p->m_bExcluded)p->m_iQueryPos=Min(p->m_iQueryPos,n.m_iAtomPos);if(!p){n.m_fIDF=-1.0f;ExtQword_t q;q.m_sWord=n.m_pQword->m_sWord;q.m_sDictWord=n.m_pQword->m_sDictWord;q.m_iDocs=n.m_pQword->m_iDocs;q.m_iHits=n.m_pQword->m_iHits;q.m_iQueryPos=n.m_iAtomPos;q.m_fIDF=-1.0f;q.m_fBoost=n.m_pQword->m_fBoost;q.m_bExpanded=n.m_pQword->m_bExpanded;q.m_bExcluded=n.m_pQword->m_bExcluded;h.Add(q,n.m_pQword->m_sWord);}mx=Max(mx,n.m_iAtomPos);}return mx;
	}
	void SetQwordsIDF(const ExtQwordsHash_t & h) override
	{
		float dIDF[3] {}; int dCanonical[3] {}; bool dPhrase[3] {};
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			m_dNodes[i].m_fIDF=h[m_dNodes[i].m_pQword->m_sWord].m_fIDF;
			dIDF[i]=m_dNodes[i].m_fIDF; dPhrase[i]=m_dNodes[i].m_bPhrase; dCanonical[i]=m_dCanonical[i];
		}
		m_tBoundLookup.Build(dIDF,dCanonical,dPhrase,m_dNodes.GetLength());
	}
	void GetTerms(const ExtQwordsHash_t & h,CSphVector<TermPos_t> & d) const override
	{
		for(const auto & n:m_dNodes){const ExtQword_t & q=h[n.m_pQword->m_sWord];TermPos_t & t=d.Add();t.m_uAtomPos=(WORD)n.m_iAtomPos;t.m_uQueryPos=(WORD)q.m_iQueryPos;}
	}
private:
	void CollectHits ( const ExtDoc_t * ) override { m_dHits.Resize(0); }
	static constexpr uint32_t E1_ROWS=4096;
	struct Node_t { ISphQword * m_pQword=nullptr;int m_iAtomPos=0;float m_fIDF=0.0f;bool m_bPhrase=false; };
	enum class PhraseVerifyMode_e { EXISTENCE_ONLY, FULL_COUNT };
	uint32_t VerifyPhraseOccurrences ( PhraseVerifyMode_e eMode )
	{
		++m_uPositionalDocsVerified;
		const int iMode = eMode==PhraseVerifyMode_e::EXISTENCE_ONLY ? 0 : 1;
		if ( eMode==PhraseVerifyMode_e::EXISTENCE_ONLY ) ++m_uExistenceOnlyDocs; else ++m_uFullCountDocs;
		const int iFirst = m_iPhrase[0];
		const int iSecond = m_iPhrase[1];
		ISphQword * pFirst = m_dNodes[iFirst].m_pQword;
		ISphQword * pSecond = m_dNodes[iSecond].m_pQword;
		pFirst->SeekHitlist ( m_dCurrentMeta[iFirst].m_uRef );
		pSecond->SeekHitlist ( m_dCurrentMeta[iSecond].m_uRef );
		m_uHitlistSeeks += 2;

		// The generic phrase node emits one folded span hit for every non-overlapping
		// occurrence. The expression ranker expands each span to both query positions,
		// so BM25A sees phrase-occurrence frequency for both phrase qwords, not their
		// primary-posting TF. Reproduce that stream semantically without materializing
		// folded hits. Advancing both cursors after a match mirrors FSMphrase_c::ResetFSM.
		Hitpos_t uFirst = pFirst->GetNextHit();
		Hitpos_t uSecond = pSecond->GetNextHit();
		auto CountDecoded = [this,iMode] ( int iTerm ) { ++m_dDecodedPositions[iTerm]; ++m_dDecodedPositionsByMode[iMode][iTerm]; };
		if ( uFirst!=EMPTY_HIT ) CountDecoded(0);
		if ( uSecond!=EMPTY_HIT ) CountDecoded(1);
		uint32_t uOccurrences = 0;
		while ( uFirst!=EMPTY_HIT && uSecond!=EMPTY_HIT )
		{
			if ( HITMAN::GetField(uFirst)!=m_iField )
			{
				uFirst = pFirst->GetNextHit();
				if ( uFirst!=EMPTY_HIT ) CountDecoded(0);
				continue;
			}
			if ( HITMAN::GetField(uSecond)!=m_iField )
			{
				uSecond = pSecond->GetNextHit();
				if ( uSecond!=EMPTY_HIT ) CountDecoded(1);
				continue;
			}

			const DWORD uFirstPos = HITMAN::GetPosWithField ( uFirst );
			const DWORD uSecondPos = HITMAN::GetPosWithField ( uSecond );
			const DWORD uWanted = uFirstPos + DWORD(m_iPhraseDelta);
			if ( uSecondPos<uWanted )
			{
				uSecond = pSecond->GetNextHit();
				if ( uSecond!=EMPTY_HIT ) CountDecoded(1);
				continue;
			}
			if ( uSecondPos>uWanted )
			{
				uFirst = pFirst->GetNextHit();
				if ( uFirst!=EMPTY_HIT ) CountDecoded(0);
				continue;
			}

			++uOccurrences;
			if ( eMode==PhraseVerifyMode_e::EXISTENCE_ONLY )
				return uOccurrences;
			uFirst = pFirst->GetNextHit();
			uSecond = pSecond->GetNextHit();
			if ( uFirst!=EMPTY_HIT ) CountDecoded(0);
			if ( uSecond!=EMPTY_HIT ) CountDecoded(1);
		}
		m_uPhraseOccurrences += uOccurrences;
		return uOccurrences;
	}
	bool PreOccurrenceScoreCanImprove ( RowID_t tRowID )
	{
		if ( m_iRankThreshold<=0 )
			return true;
		const uint32_t uOccurrenceUpper = Min(m_dCurrentMeta[m_iPhrase[0]].m_uTF,m_dCurrentMeta[m_iPhrase[1]].m_uTF);
		return ScoreCanImprove ( uOccurrenceUpper, tRowID );
	}
	bool ScoreCanImprove ( uint32_t uOccurrences, RowID_t tRowID )
	{
		uint32_t uMandatory = 0;
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
			if ( !m_dNodes[i].m_bPhrase ) { uMandatory=m_dCurrentMeta[i].m_uTF; break; }
		bool bHit = false;
		const int iUpperWeight = m_tBoundLookup.Weight(uOccurrences,uMandatory,bHit);
		if ( bHit ) ++m_uBoundLookupHits; else ++m_uBoundLookupFallbacks;
		return !E1TieAwareBoundReject ( iUpperWeight, uint64_t(tRowID), m_iRankThreshold, uint64_t(m_tWorstTiedRow) );
	}
	bool FillWindow()
	{
		m_dPending.Resize(0);m_iPending=0;
		while(m_uWindow<=m_uLastWindow)
		{
			uint32_t win=m_uWindow++;++m_uWindows;std::array<uint64_t,64> both;both.fill(~uint64_t(0));bool all=true;
			for(auto & n:m_dNodes){std::array<uint64_t,64> bits{};uint32_t card=0,maxTF=0;uint64_t reads=0;if(!n.m_pQword->GetE1DirectWindow(win,bits.data(),nullptr,card,maxTF,reads)){all=false;break;}for(int w=0;w<64;++w)both[w]&=bits[w];}
			if(!all)continue;uint64_t candidates=0;for(uint64_t x:both)candidates+=__builtin_popcountll(x);if(!candidates)continue;m_uDirectCandidates+=candidates;
			for(int i=0;i<m_dNodes.GetLength();++i){uint64_t requested=0;if(!m_dNodes[i].m_pQword->BeginE1SelectedMeta(win,both.data(),requested)||requested!=candidates)return false;}
			for(uint64_t iCandidate=0;iCandidate<candidates;++iCandidate)
			{
				uint32_t local=UINT32_MAX;
				for(int i=0;i<m_dNodes.GetLength();++i){uint64_t decoded=0;if(!m_dNodes[i].m_pQword->NextE1SelectedMeta(m_dCurrentMeta[i],decoded))return false;m_uSelectedMetaGroups+=decoded;++m_uSelectedMetaRows;if(i==0)local=m_dCurrentMeta[i].m_uLocal;else if(local!=m_dCurrentMeta[i].m_uLocal)return false;}
				bool field=true;for(int i=0;i<m_dNodes.GetLength();++i)if(!(m_dCurrentMeta[i].m_uMask&(uint32_t(1)<<m_iField))){field=false;break;}if(!field)continue;const RowID_t row=RowID_t(win*E1_ROWS+local);const bool bEarlyCanImprove=PreOccurrenceScoreCanImprove(row);const uint32_t occurrences=VerifyPhraseOccurrences(bEarlyCanImprove ? PhraseVerifyMode_e::FULL_COUNT : PhraseVerifyMode_e::EXISTENCE_ONLY);if(!occurrences)continue;++m_uExactPhraseMatches;if(!bEarlyCanImprove){++m_uSkippedMatches;++m_uEarlyBoundRejects;continue;}if(!ScoreCanImprove(occurrences,row)){++m_uSkippedMatches;++m_uPostCountBoundRejects;continue;}
				ExtDoc_t & doc=m_dPending.Add();doc.m_tRowID=row;doc.m_uDocFields=uint32_t(1)<<m_iField;doc.m_fTFIDF=0.0f;doc.m_uExactTF=0;doc.m_uExactTerms=BYTE(m_dNodes.GetLength());doc.m_bExactOr=0;for(int c=0;c<m_dCanonical.GetLength();++c){const int node=m_dCanonical[c];doc.m_dExactTF[c]=m_dNodes[node].m_bPhrase?occurrences:m_dCurrentMeta[node].m_uTF;}
			}
			return true;
		}
		return false;
	}
	CSphVector<Node_t> m_dNodes;CSphVector<int> m_dCanonical;CSphVector<ExtDoc_t> m_dPending;CSphVector<E1SelectedMeta_t> m_dCurrentMeta;E1PhraseBoundLookup_c m_tBoundLookup;
	int m_iField=0,m_iPhrase[2]{-1,-1},m_iPhraseDelta=0,m_iPending=0,m_iRankThreshold=0;uint32_t m_uWindow=0,m_uLastWindow=UINT32_MAX;RowID_t m_tWorstTiedRow=INVALID_ROWID;bool m_bEnabled=false;CSphQueryStats * m_pStats=nullptr;
	uint64_t m_uDirectCandidates=0,m_uPositionalDocsVerified=0,m_uExactPhraseMatches=0,m_uPhraseOccurrences=0,m_uHitlistSeeks=0,m_dDecodedPositions[2]{},m_dDecodedPositionsByMode[2][2]{},m_uExistenceOnlyDocs=0,m_uFullCountDocs=0,m_uSelectedMetaRows=0,m_uSelectedMetaGroups=0,m_uBoundLookupHits=0,m_uBoundLookupFallbacks=0,m_uWindows=0,m_uSkippedMatches=0,m_uEarlyBoundRejects=0,m_uPostCountBoundRejects=0;
};

static bool E1OneExplicitField(const XQNode_t * pNode,int & field)
{
	if(!pNode->m_dSpec.m_bFieldSpec)return false;const FieldMask_t & f=pNode->m_dSpec.m_dFieldMask;for(int i=1;i<FieldMask_t::SIZE;++i)if(f[i])return false;uint32_t mask=f.GetMask32();if(!mask||(mask&(mask-1)))return false;field=__builtin_ctz(mask);return true;
}

static bool E1ExactTwoTermPhrase ( const XQNode_t * pNode )
{
	if ( pNode->GetOp()!=SPH_QUERY_PHRASE )
		return false;
	if ( pNode->dWords().GetLength()==2 && pNode->dChildren().IsEmpty() )
		return true;
	if ( !pNode->dWords().IsEmpty() || pNode->dChildren().GetLength()!=2 )
		return false;
	for ( const XQNode_t * pChild : pNode->dChildren() )
		if ( pChild->dWords().GetLength()!=1 || !pChild->dChildren().IsEmpty() ) return false;
	return true;
}

static ExtNode_i * TryCreateE1Phrase(const XQNode_t * pNode,const ISphQwordSetup & setup,const RowIdBoundaries_t * bounds)
{
	if(!setup.m_bE1RankedRequested||setup.m_tE1RankFilter.m_bEnabled||bounds)return nullptr;const XQNode_t * phrase=nullptr,*extra=nullptr;
	if(E1ExactTwoTermPhrase(pNode))phrase=pNode;
	else if(pNode->GetOp()==SPH_QUERY_AND&&pNode->dWords().IsEmpty()&&pNode->dChildren().GetLength()==2){for(const XQNode_t * c:pNode->dChildren())if(E1ExactTwoTermPhrase(c))phrase=c;else if(c->dWords().GetLength()==1&&c->dChildren().IsEmpty())extra=c;else return nullptr;if(!phrase||!extra)return nullptr;}else return nullptr;
	int field=-1,other=-1;if(!E1OneExplicitField(phrase,field)||!E1FieldScopeCoversSchema(phrase->m_dSpec,setup))return nullptr;CSphVector<ExtE1Phrase_c::Input_t> input;
	auto fnAddPhraseWord = [&] ( const XQKeyword_t & word, const FieldMask_t & dFields )
	{
		if(word.m_bExpanded||word.m_bFieldStart||word.m_bFieldEnd||word.m_pPayload)return false;
		for(const auto & prior:input)if(prior.m_pWord->m_sWord==word.m_sWord)return false;
		auto & x=input.Add();x={&word,dFields,true};return true;
	};
	if ( phrase->dWords().GetLength()==2 )
	{
		for ( const XQKeyword_t & word : phrase->dWords() )
			if ( !fnAddPhraseWord(word,phrase->m_dSpec.m_dFieldMask) ) return nullptr;
	}
	else
		for(const XQNode_t * child:phrase->dChildren()){if(!E1OneExplicitField(child,other)||other!=field||!E1FieldScopeCoversSchema(child->m_dSpec,setup))return nullptr;if(!fnAddPhraseWord(child->dWord(0),child->m_dSpec.m_dFieldMask))return nullptr;}
	if(extra){if(!E1OneExplicitField(extra,other)||other!=field||!E1FieldScopeCoversSchema(extra->m_dSpec,setup))return nullptr;const XQKeyword_t & word=extra->dWord(0);if(word.m_bExpanded||word.m_bFieldStart||word.m_bFieldEnd||word.m_pPayload)return nullptr;for(const auto & prior:input)if(prior.m_pWord->m_sWord==word.m_sWord)return nullptr;auto & x=input.Add();x={&word,extra->m_dSpec.m_dFieldMask,false};}
	auto pDirect = std::make_unique<ExtE1Phrase_c> ( input, field, setup );
	if ( !pDirect->EnableE1Ranked() )
		return nullptr;
	return pDirect.release();
}

static ExtNode_i * CreateOrderNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, const RowIdBoundaries_t * pBoundaries )
{
	if ( pNode->dChildren().GetLength()<2 )
	{
		if ( tSetup.m_pWarning )
			tSetup.m_pWarning->SetSprintf ( "order node requires at least two children" );
		return NULL;
	}

	CSphVector<ExtNode_i *> dChildren;
	ARRAY_FOREACH ( i, pNode->dChildren() )
	{
		ExtNode_i * pChild = ExtNode_i::Create ( pNode->dChildren()[i], tSetup, bUseBM25, pBoundaries );
		if ( !pChild || pChild->GotHitless() )
		{
			if ( tSetup.m_pWarning )
				tSetup.m_pWarning->SetSprintf ( "failed to create order node, hitlist unavailable" );
			ARRAY_FOREACH ( j, dChildren )
				SafeDelete ( dChildren[j] );
			return NULL;
		}
		dChildren.Add ( pChild );
	}
	ExtNode_i * pResult = new ExtOrder_c ( dChildren, tSetup );

	if ( pNode->GetCount() )
		return tSetup.m_pNodeCache->CreateProxy ( pResult, pNode, tSetup );

	return pResult;
}

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits )
{
	return Create ( CreateQueryWord ( tWord, tSetup ), pNode, tSetup, bUseBM25, bRowidLimits );
}


template <TermPosFilter_e TERMPOS>
static ExtNode_i * CreateTermposNode ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits )
{
	int iSwitch = 4*(bUseBM25?1:0) + 2*(bRowidLimits?1:0) + (tSetup.m_pStats?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtTermPos_T<TERMPOS, false, false, false>	( pQword, pNode, tSetup );
	case 1:	return new ExtTermPos_T<TERMPOS, false, false, true>	( pQword, pNode, tSetup );
	case 2:	return new ExtTermPos_T<TERMPOS, false, true,  false>	( pQword, pNode, tSetup );
	case 3:	return new ExtTermPos_T<TERMPOS, false, true,  true>	( pQword, pNode, tSetup );
	case 4:	return new ExtTermPos_T<TERMPOS, true,  false, false>	( pQword, pNode, tSetup );
	case 5:	return new ExtTermPos_T<TERMPOS, true,  false, true>	( pQword, pNode, tSetup );
	case 6:	return new ExtTermPos_T<TERMPOS, true,  true,  false>	( pQword, pNode, tSetup );
	case 7:	return new ExtTermPos_T<TERMPOS, true,  true,  true>	( pQword, pNode, tSetup );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static ExtNode_i * CreateHitlessNode ( ISphQword * pQword, const FieldMask_t & tFieldMask, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bUseBM25, bool bRowidLimits )
{
	int iSwitch = 4*(bUseBM25?1:0) + 2*(bRowidLimits?1:0) + (tSetup.m_pStats?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtTermHitless_T<false, false, false>( pQword, tFieldMask, tSetup, bNotWeighted );
	case 1:	return new ExtTermHitless_T<false, false, true>	( pQword, tFieldMask, tSetup, bNotWeighted );
	case 2:	return new ExtTermHitless_T<false, true,  false>( pQword, tFieldMask, tSetup, bNotWeighted );
	case 3:	return new ExtTermHitless_T<false, true,  true>	( pQword, tFieldMask, tSetup, bNotWeighted );
	case 4:	return new ExtTermHitless_T<true,  false, false>( pQword, tFieldMask, tSetup, bNotWeighted );
	case 5:	return new ExtTermHitless_T<true,  false, true>	( pQword, tFieldMask, tSetup, bNotWeighted );
	case 6:	return new ExtTermHitless_T<true,  true,  false>( pQword, tFieldMask, tSetup, bNotWeighted );
	case 7:	return new ExtTermHitless_T<true,  true,  true>	( pQword, tFieldMask, tSetup, bNotWeighted );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static ExtNode_i * CreateTermNode ( ISphQword * pQword, const FieldMask_t & tFieldMask, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bUseBM25, bool bRowidLimits, bool bFullSchemaScope )
{
	int iSwitch = 4*(bUseBM25?1:0) + 2*(bRowidLimits?1:0) + (tSetup.m_pStats?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtTerm_T<false, false, false>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 1:	return new ExtTerm_T<false, false, true>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 2:	return new ExtTerm_T<false, true,  false>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 3:	return new ExtTerm_T<false, true,  true>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 4:	return new ExtTerm_T<true,  false, false>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 5:	return new ExtTerm_T<true,  false, true>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 6:	return new ExtTerm_T<true,  true,  false>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	case 7:	return new ExtTerm_T<true,  true,  true>	( pQword, tFieldMask, tSetup, bNotWeighted, bFullSchemaScope );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static ExtNode_i * CreateTermNode ( ISphQword * pQword, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits )
{
	int iSwitch = 4*(bUseBM25?1:0) + 2*(bRowidLimits?1:0) + (tSetup.m_pStats?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtTerm_T<false, false, false>	( pQword, tSetup );
	case 1:	return new ExtTerm_T<false, false, true>	( pQword, tSetup );
	case 2:	return new ExtTerm_T<false, true,  false>	( pQword, tSetup );
	case 3:	return new ExtTerm_T<false, true,  true>	( pQword, tSetup );
	case 4:	return new ExtTerm_T<true,  false, false>	( pQword, tSetup );
	case 5:	return new ExtTerm_T<true,  false, true>	( pQword, tSetup );
	case 6:	return new ExtTerm_T<true,  true,  false>	( pQword, tSetup );
	case 7:	return new ExtTerm_T<true,  true,  true>	( pQword, tSetup );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static ExtNode_i * CreateMultiAndNode ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits )
{
	bool bNeedFieldSpec = false;
	for ( const auto & i : dXQNodes )
		bNeedFieldSpec |= !E1FieldScopeCoversSchema ( i->m_dSpec, tSetup );

	int iSwitch = 4*(bUseBM25?1:0) + 2*(bNeedFieldSpec?1:0) + (bRowidLimits?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtMultiAnd_T<false, false, false> ( dXQNodes, tSetup );
	case 1:	return new ExtMultiAnd_T<false, false, true>  ( dXQNodes, tSetup );
	case 2:	return new ExtMultiAnd_T<false, true,  false> ( dXQNodes, tSetup );
	case 3:	return new ExtMultiAnd_T<false, true,  true>  ( dXQNodes, tSetup );
	case 4:	return new ExtMultiAnd_T<true,  false, false> ( dXQNodes, tSetup );
	case 5:	return new ExtMultiAnd_T<true,  false, true>  ( dXQNodes, tSetup );
	case 6:	return new ExtMultiAnd_T<true,  true,  false> ( dXQNodes, tSetup );
	case 7:	return new ExtMultiAnd_T<true,  true,  true>  ( dXQNodes, tSetup );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static ExtNode_i * CreateMultiOrNode ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	// This node has AND behavior outside its direct E1 implementation. Do not
	// install it merely because the parsed shape is eligible: a sparse child has
	// no direct container representation and must keep the generic ExtOr tree.
	const bool bNeedFieldSpec = dXQNodes.any_of ( [&tSetup] ( const XQNode_t * pNode ) { return !E1FieldScopeCoversSchema ( pNode->m_dSpec, tSetup ); } );
	std::unique_ptr<ExtNode_i> pDirect;
	if ( bUseBM25 )
		pDirect.reset ( bNeedFieldSpec ? static_cast<ExtNode_i*>(new ExtMultiAnd_T<true, true, false> ( dXQNodes, tSetup, true )) : static_cast<ExtNode_i*>(new ExtMultiAnd_T<true, false, false> ( dXQNodes, tSetup, true )) );
	else
		pDirect.reset ( bNeedFieldSpec ? static_cast<ExtNode_i*>(new ExtMultiAnd_T<false, true, false> ( dXQNodes, tSetup, true )) : static_cast<ExtNode_i*>(new ExtMultiAnd_T<false, false, false> ( dXQNodes, tSetup, true )) );
	if ( !pDirect->EnableE1Ranked() )
		return nullptr;
	return pDirect.release();
}


ExtNode_i * ExtNode_i::Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits )
{
	assert ( pQword );

	if ( pNode->m_dSpec.m_iFieldMaxPos )
		pQword->m_iTermPos = TERM_POS_FIELD_LIMIT;

	if ( pNode->m_dSpec.m_dZones.GetLength() )
		pQword->m_iTermPos = TERM_POS_ZONES;

	if ( !pQword->m_bHasHitlist )
	{
		if ( tSetup.m_pWarning && pQword->m_iTermPos!=TERM_POS_NONE )
			tSetup.m_pWarning->SetSprintf ( "hitlist unavailable, position limit ignored" );

		return CreateHitlessNode ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted, bUseBM25, bRowidLimits );
	}

	switch ( pQword->m_iTermPos )
	{
		case TERM_POS_FIELD_STARTEND:	return CreateTermposNode<TERM_POS_FIELD_STARTEND> ( pQword, pNode, tSetup, bUseBM25, bRowidLimits );
		case TERM_POS_FIELD_START:		return CreateTermposNode<TERM_POS_FIELD_START> ( pQword, pNode, tSetup, bUseBM25, bRowidLimits );
		case TERM_POS_FIELD_END:		return CreateTermposNode<TERM_POS_FIELD_END> ( pQword, pNode, tSetup, bUseBM25, bRowidLimits );
		case TERM_POS_FIELD_LIMIT:		return CreateTermposNode<TERM_POS_FIELD_LIMIT> ( pQword, pNode, tSetup, bUseBM25, bRowidLimits );
		case TERM_POS_ZONES:			return CreateTermposNode<TERM_POS_ZONES> ( pQword, pNode, tSetup, bUseBM25, bRowidLimits );
		default:						return CreateTermNode ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted, bUseBM25, bRowidLimits, E1FieldScopeCoversSchema ( pNode->m_dSpec, tSetup ) );
	}
}

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict, bool bUseBM25, bool bRowidLimits )
{
	return CreateTermNode ( CreateQueryWord ( tWord, tSetup, std::move (pZonesDict) ), tSetup, bUseBM25, bRowidLimits );
}

//////////////////////////////////////////////////////////////////////////

ExtNode_c::ExtNode_c ( int64_t tmTimeout )
	: m_iMaxTimer { tmTimeout }
{
	m_dDocs[0].m_tRowID = INVALID_ROWID;
	m_dHits.Reserve ( MAX_BLOCK_DOCS );
}


void ExtNode_c::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtNode\n" );
}


void ExtNode_c::SetAtomPos ( int iPos )
{
	m_iAtomPos = iPos;
}


int ExtNode_c::GetAtomPos() const
{
	return m_iAtomPos;
}


void ExtNode_c::SetQPosReverse ()
{
	m_bQPosReverse = true;
}

void ExtNode_c::SetMaxTimeout ( int64_t iTimer )
{
	m_iMaxTimer = iTimer;
}

bool ExtNode_c::TimeExceeded() const
{
	return sph::TimeExceeded ( m_iMaxTimer );
}

int64_t ExtNode_c::GetMaxTimeout() const
{
	return m_iMaxTimer;
}


const ExtHit_t * ExtNode_c::GetHits ( const ExtDoc_t * pDocs )
{
	m_dHits.Resize(0);
	CollectHits ( pDocs );

	return ReturnHits ( m_bQPosReverse );
}


inline const ExtDoc_t * ExtNode_c::ReturnDocsChunk ( int iCount, const char * sNode, const char * sTerm )
{
	assert ( iCount>=0 && iCount<MAX_BLOCK_DOCS );
	m_dDocs[iCount].m_tRowID = INVALID_ROWID;

	PrintDocsChunk ( iCount, m_iAtomPos, m_dDocs, sNode, this, sTerm );
	return iCount ? m_dDocs : nullptr;
}


inline const ExtHit_t * ExtNode_c::ReturnHits ( bool bReverse )
{
	int iCount = m_dHits.GetLength();
	m_dHits.Add().m_tRowID = INVALID_ROWID;

#ifndef NDEBUG
	for ( int i=1; i<iCount; i++ )
	{
		bool bQPosPassed = ( ( bReverse && m_dHits[i-1].m_uQuerypos>=m_dHits[i].m_uQuerypos ) || ( !bReverse && m_dHits[i-1].m_uQuerypos<=m_dHits[i].m_uQuerypos ) );
		assert ( m_dHits[i-1].m_tRowID!=m_dHits[i].m_tRowID ||
			( m_dHits[i-1].m_uHitpos<m_dHits[i].m_uHitpos || ( m_dHits[i-1].m_uHitpos==m_dHits[i].m_uHitpos && bQPosPassed ) ) );

		assert ( m_dHits[i-1].m_tRowID <= m_dHits[i].m_tRowID );
	}
#endif

	PrintHitsChunk ( iCount, m_iAtomPos, iCount ? m_dHits.Begin() : nullptr, this );
	return m_dHits.Begin();
}


inline const ExtHit_t * ExtNode_c::ReturnHitsChunk ( int iCount, const char * sNode, bool bReverse )
{
	assert ( iCount>=0 && iCount<MAX_HITS );
	m_dHits[iCount].m_tRowID = INVALID_ROWID;

#ifndef NDEBUG
	for ( int i=1; i<iCount; i++ )
	{
		bool bQPosPassed = ( ( bReverse && m_dHits[i-1].m_uQuerypos>=m_dHits[i].m_uQuerypos ) || ( !bReverse && m_dHits[i-1].m_uQuerypos<=m_dHits[i].m_uQuerypos ) );
		assert ( m_dHits[i-1].m_tRowID!=m_dHits[i].m_tRowID ||
			( m_dHits[i-1].m_uHitpos<m_dHits[i].m_uHitpos || ( m_dHits[i-1].m_uHitpos==m_dHits[i].m_uHitpos && bQPosPassed ) ) );
	}
#endif

	const ExtHit_t * pHits = iCount ? m_dHits.Begin() : nullptr;
	PrintHitsChunk ( iCount, m_iAtomPos, pHits, this );
	return pHits;
}


//////////////////////////////////////////////////////////////////////////

struct ExtPayloadEntry_t
{
	RowID_t		m_tRowID;
	Hitpos_t	m_uHitpos;

	bool operator < ( const ExtPayloadEntry_t & rhs ) const
	{
		if ( m_tRowID!=rhs.m_tRowID )
			return ( m_tRowID<rhs.m_tRowID );
		return ( m_uHitpos<rhs.m_uHitpos );
	}
};

struct ExtPayloadKeyword_t : public XQKeyword_t
{
	CSphString	m_sDictWord;
	SphWordID_t	m_uWordID;
	float		m_fIDF;
	int			m_iDocs;
	int			m_iHits;
};

/// simple in-memory multi-term cache
template <bool ROWID_LIMITS>
class ExtPayloadBase_T : public ExtNode_c
{
public:
						ExtPayloadBase_T ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, const RowIdBoundaries_t * pBoundaries );

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				HintRowID ( RowID_t ) override {} // FIXME!!! implement with tree
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t &, CSphVector<TermPos_t> & ) const override;
	bool				GotHitless () override { return false; }
	int					GetDocsCount() const override { return m_tWord.m_iDocs; }
	uint64_t			GetWordID () const override { return m_tWord.m_uWordID; }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override {}

protected:
	CSphVector<ExtPayloadEntry_t> m_dCache;
	ExtPayloadKeyword_t	m_tWord;

	int					m_iCurDocsEnd;		///< end of the last docs chunk returned, exclusive, ie [begin,end)
	int					m_iCurHit;			///< end of the last hits chunk (within the last docs chunk) returned, exclusive

	CSphString *		m_pWarning;

private:
	FieldMask_t			m_dFieldMask;
	RowIdBoundaries_t	m_tBoundaries;

	void				PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat );
	void				FetchHits ( ISphQword * pQword, bool bFillStat );
	void				FilterHits();
};

template<bool USE_BM25, bool ROWID_LIMITS>
class ExtPayload_T : public ExtPayloadBase_T<ROWID_LIMITS>
{
	using BASE = ExtPayloadBase_T<ROWID_LIMITS>;

public:
						ExtPayload_T ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, const RowIdBoundaries_t * pBoundaries ) : BASE ( pNode, tSetup, pBoundaries ) {}

	const ExtDoc_t *	GetDocsChunk() final;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const final { return { 0.0f, 0, 0 }; }
};

template<bool ROWID_LIMITS>
ExtPayloadBase_T<ROWID_LIMITS>::ExtPayloadBase_T ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, const RowIdBoundaries_t * pBoundaries )
	: ExtNode_c { tSetup.m_iMaxTimer }
{
	// sanity checks
	// this node must be only created for a huge OR of tiny expansions
	assert ( pNode->dWords().GetLength()==1 );
	assert ( pNode->dWord(0).m_pPayload );
	assert ( pNode->m_dSpec.m_dZones.GetLength()==0 && !pNode->m_dSpec.m_bZoneSpan );

	(XQKeyword_t &)m_tWord = pNode->dWord(0);
	m_dFieldMask = pNode->m_dSpec.m_dFieldMask;
	m_iAtomPos = m_tWord.m_iAtomPos;

	BYTE sTmpWord [ 3*SPH_MAX_WORD_LEN + 4 ];
	// our little stemming buffer (morphology aware dictionary might need to change the keyword)
	strncpy ( (char*)sTmpWord, m_tWord.m_sWord.cstr(), sizeof(sTmpWord) );
	sTmpWord[sizeof(sTmpWord)-1] = '\0';

	// setup keyword disk reader
	m_tWord.m_uWordID = tSetup.m_pDict->GetWordID ( sTmpWord );
	m_tWord.m_sDictWord = (const char *)sTmpWord;
	m_tWord.m_fIDF = -1.0f;
	m_tWord.m_iDocs = 0;
	m_tWord.m_iHits = 0;

	m_pWarning = tSetup.m_pWarning;
	SetMaxTimeout ( tSetup.m_iMaxTimer );

	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;

	PopulateCache ( tSetup, true );
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::FetchHits ( ISphQword * pQword, bool bFillStat )
{
	m_dCache.Reserve ( Max ( pQword->m_iHits, pQword->m_iDocs ) );

	while (true)
	{
		const CSphMatch & tMatch = pQword->GetNextDoc();
		if ( tMatch.m_tRowID==INVALID_ROWID )
			break;

		pQword->SeekHitlist ( pQword->m_iHitlistPos );
		for ( Hitpos_t uHit = pQword->GetNextHit(); uHit!=EMPTY_HIT; uHit = pQword->GetNextHit() )
		{
			// apply field limits
			if ( !m_dFieldMask.Test ( HITMAN::GetField(uHit) ) )
				continue;

			if constexpr ( ROWID_LIMITS )
			{
				if ( !bFillStat && ( tMatch.m_tRowID < m_tBoundaries.m_tMinRowID || tMatch.m_tRowID > m_tBoundaries.m_tMaxRowID ) )
					continue;
			}

			// FIXME!!! apply zone limits too

			// apply field-start/field-end modifiers
			if ( m_tWord.m_bFieldStart && HITMAN::GetPos(uHit)!=1 )
				continue;
			if ( m_tWord.m_bFieldEnd && !HITMAN::IsEnd(uHit) )
				continue;

			// ok, this hit works, copy it
			ExtPayloadEntry_t & tEntry = m_dCache.Add();
			tEntry.m_tRowID = tMatch.m_tRowID;
			tEntry.m_uHitpos = uHit;
		}
	}

	m_dCache.Sort();
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::FilterHits()
{
	auto * pFront = m_dCache.Begin();
	auto * pBack = (m_dCache.End()-1);

	if ( pBack->m_tRowID < m_tBoundaries.m_tMinRowID || pFront->m_tRowID > m_tBoundaries.m_tMaxRowID )
		m_dCache.Resize(0);
	else
	{
		bool bCutFront = pFront->m_tRowID < m_tBoundaries.m_tMinRowID;
		bool bCutBack =  pBack->m_tRowID > m_tBoundaries.m_tMaxRowID;
		if ( bCutFront || bCutBack )
		{
			auto * pPtr = pFront;
			auto * pEnd = pBack+1;
			int iCutFront = 0;

			if ( bCutFront )
			{
				pPtr = std::lower_bound ( pPtr, pEnd, m_tBoundaries.m_tMinRowID, []( auto & tEntry, RowID_t tValue ){ return tEntry.m_tRowID < tValue; } );
				iCutFront = pPtr-pFront;
			}

			if ( bCutBack )
			{
				pEnd = std::upper_bound ( pPtr, pEnd, m_tBoundaries.m_tMaxRowID, []( RowID_t tValue, auto & tEntry ){ return tValue < tEntry.m_tRowID; } );
				m_dCache.Resize ( pEnd-pFront );
			}

			m_dCache.Remove ( 0, iCutFront );
		}
	}
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat )
{
	std::unique_ptr<ISphQword> pQword = std::unique_ptr<ISphQword>(tSetup.QwordSpawn(m_tWord));
	pQword->m_sWord = m_tWord.m_sWord;
	pQword->m_uWordID = m_tWord.m_uWordID;
	pQword->m_sDictWord = m_tWord.m_sDictWord;
	pQword->m_bExpanded = true;

	bool bOk = tSetup.QwordSetup ( pQword.get() );

	// setup keyword idf and stats
	if ( bFillStat )
	{
		m_tWord.m_iDocs = pQword->m_iDocs;
		m_tWord.m_iHits = pQword->m_iHits;
	}

	// read and cache all docs and hits
	if ( bOk )
		FetchHits ( pQword.get(), bFillStat );
	
	if ( bFillStat && m_dCache.GetLength() )
	{
		// there might be duplicate documents, but not hits, lets recalculate docs count
		// FIXME!!! that not work for RT index - get rid of ExtPayloadBase_T and move PopulateCache code to index specific QWord
		// FIXME! if we have pseudo_sharding=1, we read all hits just to calculate the correct number of docs (instead of early filtering by rowid)
		//		  because of this we also can't hint individual qwords to the start of rowid boundaries
		RowID_t tLastRowID = m_dCache.Begin()->m_tRowID;
		const ExtPayloadEntry_t * pCur = m_dCache.Begin() + 1;
		const ExtPayloadEntry_t * pEnd = m_dCache.Begin() + m_dCache.GetLength();
		int iDocsTotal = 1;
		while ( pCur!=pEnd )
		{
			iDocsTotal += ( tLastRowID!=pCur->m_tRowID );
			tLastRowID = pCur->m_tRowID;
			pCur++;
		}
		m_tWord.m_iDocs = iDocsTotal;

		// remove hits that don't belong to our pseudo shard
		// we could do this earlier, but we need to collect hits to calculate the correct number of docs (affects weight calc)
		if constexpr ( ROWID_LIMITS )
			FilterHits();
	}

	m_iCurDocsEnd = 0;
	m_iCurHit = 0;
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::Reset ( const ISphQwordSetup & tSetup )
{
	SetMaxTimeout ( tSetup.m_iMaxTimer );
	m_dCache.Resize ( 0 );
	PopulateCache ( tSetup, false );
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::CollectHits ( const ExtDoc_t * pDocs )
{
	int iCurHit = m_iCurHit;
	const int iCurDocsEnd = m_iCurDocsEnd;

	while ( HasDocs(pDocs) && iCurHit<iCurDocsEnd )
	{
		// skip rejected documents
		while ( iCurHit<iCurDocsEnd && m_dCache[iCurHit].m_tRowID<pDocs->m_tRowID )
			iCurHit++;
		if ( iCurHit>=iCurDocsEnd )
			break;

		// skip non-matching documents
		RowID_t tRowID = m_dCache[iCurHit].m_tRowID;
		if ( pDocs->m_tRowID<tRowID )
		{
			while ( pDocs->m_tRowID<tRowID )
				pDocs++;
			if ( pDocs->m_tRowID!=tRowID )
				continue;
		}

		// copy accepted documents
		while ( iCurHit<iCurDocsEnd && m_dCache[iCurHit].m_tRowID==pDocs->m_tRowID )
		{
			ExtHit_t & tHit = m_dHits.Add();
			tHit.m_tRowID = m_dCache[iCurHit].m_tRowID;
			tHit.m_uHitpos = m_dCache[iCurHit].m_uHitpos;
			tHit.m_uQuerypos = (WORD) m_tWord.m_iAtomPos;
			tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
			iCurHit++;
		}
	}

	m_iCurHit = iCurHit;
}

template<bool ROWID_LIMITS>
int ExtPayloadBase_T<ROWID_LIMITS>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ExtQword_t tQword;
	tQword.m_sWord = m_tWord.m_sWord;
	tQword.m_sDictWord = m_tWord.m_sDictWord;
	tQword.m_iDocs = m_tWord.m_iDocs;
	tQword.m_iHits = m_tWord.m_iHits;
	tQword.m_fIDF = -1.0f;
	tQword.m_fBoost = m_tWord.m_fBoost;
	tQword.m_iQueryPos = m_tWord.m_iAtomPos;
	tQword.m_bExpanded = true;
	tQword.m_bExcluded = m_tWord.m_bExcluded;

	hQwords.Add ( tQword, m_tWord.m_sWord );
	if ( !m_tWord.m_bExcluded )
		iMax = Max ( iMax, m_tWord.m_iAtomPos );

	return iMax;
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	// pull idfs
	if ( m_tWord.m_fIDF<0.0f )
	{
		assert ( hQwords ( m_tWord.m_sWord ) );
		m_tWord.m_fIDF = hQwords ( m_tWord.m_sWord )->m_fIDF;
	}
}

template<bool ROWID_LIMITS>
void ExtPayloadBase_T<ROWID_LIMITS>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_tWord.m_bExcluded )
		return;

	ExtQword_t & tQword = hQwords[ m_tWord.m_sWord ];

	TermPos_t & tPos = dTermDupes.Add();
	tPos.m_uAtomPos = (WORD)m_tWord.m_iAtomPos;
	tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
}

//////////////////////////////////////////////////////////////////////////

template <bool USE_BM25, bool ROWID_LIMITS>
const ExtDoc_t * ExtPayload_T<USE_BM25,ROWID_LIMITS>::GetDocsChunk()
{
	// max_query_time
	if ( BASE::TimeExceeded() )
	{
		if ( BASE::m_pWarning )
			*BASE::m_pWarning = "query time exceeded max_query_time";
		return nullptr;
	}

	// interrupt by sitgerm
	if ( sph::TimeExceeded ( BASE::m_iCheckTimePoint ) )
	{
		if ( g_bInterruptNow )
		{
			if ( BASE::m_pWarning )
				*BASE::m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( BASE::m_pWarning )
				*BASE::m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	int iDoc = 0;
	int iEnd = BASE::m_iCurDocsEnd;
	while ( iDoc<MAX_BLOCK_DOCS-1 && iEnd<BASE::m_dCache.GetLength() )
	{
		RowID_t tRowID = BASE::m_dCache[iEnd].m_tRowID;

		ExtDoc_t & tDoc = BASE::m_dDocs[iDoc++];
		tDoc.m_tRowID = tRowID;
		tDoc.m_uDocFields = 0;

		int iHitStart = iEnd;
		while ( iEnd<BASE::m_dCache.GetLength() && BASE::m_dCache[iEnd].m_tRowID==tRowID )
		{
			tDoc.m_uDocFields |= 1<< ( HITMAN::GetField ( BASE::m_dCache[iEnd].m_uHitpos ) );
			iEnd++;
		}

		if constexpr ( USE_BM25 )
		{
			int iHits = iEnd - iHitStart;
			tDoc.m_fTFIDF = float(iHits) / float(SPH_BM25_K1+iHits) * BASE::m_tWord.m_fIDF;
		}
	}

	BASE::m_iCurDocsEnd = iEnd;

	return BASE::ReturnDocsChunk ( iDoc, "payload", BASE::m_tWord.m_sDictWord.cstr() );
}

//////////////////////////////////////////////////////////////////////////

static ExtNode_i * CreatePayloadNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, const RowIdBoundaries_t * pBoundaries )
{
	int iSwitch = 2*(bUseBM25?1:0) + (pBoundaries?1:0);
	switch ( iSwitch )
	{
	case 0:	return new ExtPayload_T<false, false> ( pNode, tSetup, pBoundaries );
	case 1:	return new ExtPayload_T<false, true>  ( pNode, tSetup, pBoundaries );
	case 2: return new ExtPayload_T<true,  false> ( pNode, tSetup, pBoundaries );
	case 3: return new ExtPayload_T<true,  true>  ( pNode, tSetup, pBoundaries );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


ExtNode_i * ExtNode_i::Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, const RowIdBoundaries_t * pBoundaries )
{
	// empty node?
	if ( pNode->IsEmpty() && pNode->GetOp()!=SPH_QUERY_SCAN )
		return nullptr;

	// Install the dedicated positional physical plan before generic phrase/AND
	// construction. Unsupported shapes return nullptr and keep the exact fallback.
	if ( ExtNode_i * pE1Phrase = TryCreateE1Phrase ( pNode, tSetup, pBoundaries ) )
		return pE1Phrase;

	bool bRowidLimits = !!pBoundaries;

	if ( pNode->GetOp()==SPH_QUERY_SCAN )
		return CreateHitlessNode ( tSetup.ScanSpawn ( pNode->m_iAtomPos ), pNode->m_dSpec.m_dFieldMask, tSetup, true, bUseBM25, bRowidLimits );

	if ( pNode->dWords().GetLength() || pNode->m_bVirtuallyPlain )
	{
		const int iWords = pNode->m_bVirtuallyPlain
			? pNode->dChildren().GetLength()
			: pNode->dWords().GetLength();

		if ( iWords==1 )
		{
			if ( pNode->dWord(0).m_bExpanded && pNode->dWord(0).m_pPayload )
				return CreatePayloadNode ( pNode, tSetup, bUseBM25, pBoundaries );

			if ( pNode->m_bVirtuallyPlain )
				return Create ( pNode->dChildren()[0], tSetup, bUseBM25, pBoundaries );
			return Create ( pNode->dWord(0), pNode, tSetup, bUseBM25, bRowidLimits );
		}

		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_PHRASE:
				return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true, bUseBM25, pBoundaries );

			case SPH_QUERY_PROXIMITY:
				return CreateMultiNode<ExtProximity_c> ( pNode, tSetup, true, bUseBM25, pBoundaries );

			case SPH_QUERY_NEAR:
				return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true, bUseBM25, pBoundaries );

			case SPH_QUERY_QUORUM:
			{
				assert ( pNode->dWords().IsEmpty() || pNode->dChildren().IsEmpty() );
				int iQuorumCount = pNode->dWords().GetLength()+pNode->dChildren().GetLength();
				int iThr = ExtQuorum_c::GetThreshold ( *pNode, iQuorumCount );
				bool bOrOperator = false;
				if ( iThr>=iQuorumCount )
				{
					// threshold is too high
					if ( tSetup.m_pWarning && !pNode->m_bPercentOp )
						tSetup.m_pWarning->SetSprintf ( "quorum threshold too high (words=%d, thresh=%d); replacing quorum operator with AND operator",
							iQuorumCount, pNode->m_iOpArg );

				} else if ( iQuorumCount>256 )
				{
					// right now quorum can only handle 256 words
					if ( tSetup.m_pWarning )
						tSetup.m_pWarning->SetSprintf ( "too many words (%d) for quorum; replacing with an AND", iQuorumCount );
				} else if ( iThr==1 )
				{
					bOrOperator = true;
				} else // everything is ok; create quorum node
				{
					return CreateMultiNode<ExtQuorum_c> ( pNode, tSetup, false, bUseBM25, pBoundaries );
				}

				// couldn't create quorum, make an AND node instead
				CSphVector<ExtNode_i*> dTerms;
				dTerms.Reserve ( iQuorumCount );

				for ( const XQKeyword_t& tWord: pNode->dWords() )
					dTerms.Add ( Create ( tWord, pNode, tSetup, bUseBM25, bRowidLimits ) );

				for ( const XQNode_t* tNode: pNode->dChildren() )
					dTerms.Add ( Create ( tNode, tSetup, bUseBM25, pBoundaries ) );

				// make not simple, but optimized AND node.
				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
				{
					if ( !bOrOperator )
						pCur = new ExtAnd_c ( pCur, dTerms[i] );
					else
						pCur = new ExtOr_c ( pCur, dTerms[i] );
				}

				if ( pNode->GetCount() )
					return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
				return pCur;
			}
			default:
				assert ( 0 && "unexpected plain node type" );
				return NULL;
		}

	} else
	{
		int iChildren = pNode->dChildren().GetLength ();
		assert ( iChildren>0 );

		// Dedicated physical flat OR; never install it for a query that can need
		// generic hit/factor finalization.
		bool bE1OrTerms = tSetup.m_bE1RankedRequested && pNode->GetOp()==SPH_QUERY_OR && ( iChildren==2 || iChildren==4 );
		for ( int i=0; i<iChildren && bE1OrTerms; ++i )
		{
			const XQNode_t * pChild = pNode->dChildren()[i];
			bE1OrTerms = pChild->dWords().GetLength()==1 && pChild->dChildren().IsEmpty()
				&& !pChild->dWord(0).m_bFieldStart && !pChild->dWord(0).m_bFieldEnd && !pChild->dWord(0).m_pPayload
				&& !pChild->m_dSpec.m_iFieldMaxPos && pChild->m_dSpec.m_dZones.IsEmpty();
			if ( !bE1OrTerms || E1FieldScopeCoversSchema ( pChild->m_dSpec, tSetup ) )
				continue;
			const int iField = E1ScopedSingleField ( E1SchemaFieldCount(tSetup), false, pChild->m_dSpec.m_dFieldMask.GetMask32() );
			if ( iChildren!=2 || iField<0 )
				bE1OrTerms = false;
		}
		if ( bE1OrTerms )
		{
			if ( ExtNode_i * pDirectOr = CreateMultiOrNode ( pNode->dChildren(), tSetup, bUseBM25 ) )
				return pDirectOr;
		}

		// special case, operator BEFORE
		if ( pNode->GetOp ()==SPH_QUERY_BEFORE )
		{
			// before operator can not handle ZONESPAN
			if ( tSetup.m_pWarning
				&& pNode->dChildren().any_of ( [] ( XQNode_t * pChild ) { return pChild->m_dSpec.m_bZoneSpan; } ) )
				tSetup.m_pWarning->SetSprintf ( "BEFORE operator is incompatible with ZONESPAN, ZONESPAN ignored" );
			return CreateOrderNode ( pNode, tSetup, bUseBM25, pBoundaries );
		}

		// special case, AND over terms (internally reordered for speed)
		bool bAndTerms = ( pNode->GetOp()==SPH_QUERY_AND );
		for ( int i=0; i<iChildren && bAndTerms; ++i )
		{
			const XQNode_t * pChildren = pNode->dChildren()[i];
			bAndTerms = ( pChildren->dWords().GetLength()==1 );
		}

		bool bZonespan = bAndTerms;
		for ( int i=0; i<iChildren && bZonespan; i++ )
			bZonespan &= pNode->dChildren()[i]->m_dSpec.m_bZoneSpan;

		if ( bAndTerms )
		{
			// check if we can create multi-and node
			bool bMultiAnd = !bZonespan && pNode->dChildren().GetLength()>1;
			for ( int i=0; i<iChildren && bMultiAnd; i++ )
			{
				const XQNode_t * pChild = pNode->dChildren()[i];
				const XQKeyword_t & tWord = pChild->dWord(0);
				if ( tWord.m_bFieldStart || tWord.m_bFieldEnd || tWord.m_pPayload || pChild->m_dSpec.m_iFieldMaxPos || pChild->m_dSpec.m_dZones.GetLength() )
				{
					bMultiAnd = false;
					break;
				}
			}

			ESphHitless eMode = tSetup.m_pIndex ? tSetup.m_pIndex->GetSettings().m_eHitless : SPH_HITLESS_NONE;
			if ( eMode==SPH_HITLESS_SOME || eMode==SPH_HITLESS_ALL )
				bMultiAnd = false;

			if ( !bMultiAnd )
			{
				// create eval-tree terms from query-tree nodes
				CSphVector<ExtNode_i*> dTerms;
				for ( int i=0; i<iChildren; i++ )
				{
					const XQNode_t * pChild = pNode->dChildren()[i];
					ExtNode_i * pTerm = ExtNode_i::Create ( pChild, tSetup, bUseBM25, pBoundaries );
					if ( pTerm )
						dTerms.Add ( pTerm );
				}

				// sort them by frequency, to speed up AND matching
				dTerms.Sort ( ExtNodeTF_fn() );

				// create the right eval-tree node
				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
					if ( bZonespan )
						pCur = new ExtAndZonespan_c ( pCur, dTerms[i], tSetup, pNode->dChildren()[0] );
					else
						pCur = new ExtAnd_c ( pCur, dTerms[i] );

				// zonespan has Extra data which is not (yet?) covered by common-node optimizations,
				// so we need to avoid those for zonespan
				if ( !bZonespan && pNode->GetCount() )
					return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );

				return pCur;
			}

			return CreateMultiAndNode ( pNode->dChildren(), tSetup, bUseBM25, bRowidLimits );
		}

		// Multinear and phrase could be also non-plain, so here is the second entry for it.
		if ( pNode->GetOp()==SPH_QUERY_NEAR )
			return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true, bUseBM25, pBoundaries );
		if ( pNode->GetOp()==SPH_QUERY_PHRASE )
			return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true, bUseBM25, pBoundaries );

		// generic create
		ExtNode_i * pCur = NULL;
		for ( int i=0; i<iChildren; i++ )
		{
			ExtNode_i * pNext = Create ( pNode->dChildren()[i], tSetup, bUseBM25, pBoundaries );
			if ( !pNext ) continue;
			if ( !pCur )
			{
				pCur = pNext;
				continue;
			}
			switch ( pNode->GetOp() )
			{
				case SPH_QUERY_OR:			pCur = new ExtOr_c ( pCur, pNext ); break;
				case SPH_QUERY_MAYBE:		pCur = new ExtMaybe_c ( pCur, pNext ); break;
				case SPH_QUERY_AND:			pCur = new ExtAnd_c ( pCur, pNext ); break;
				case SPH_QUERY_ANDNOT:		pCur = new ExtAndNot_c ( pCur, pNext ); break;
				case SPH_QUERY_SENTENCE:
				case SPH_QUERY_PARAGRAPH:
					{
						const char * szUnit = pNode->GetOp()==SPH_QUERY_SENTENCE ? MAGIC_WORD_SENTENCE : MAGIC_WORD_PARAGRAPH;
						if ( bRowidLimits )
							pCur = new ExtUnit_T<true> ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, szUnit );
						else
							pCur = new ExtUnit_T<false> ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, szUnit );
					}
					break;
				case SPH_QUERY_NOTNEAR:		pCur = new ExtNotNear_c ( pCur, pNext, pNode->m_iOpArg ); break;
				default:					assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pCur && pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
inline void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::Init ( ISphQword * pQword, const FieldMask_t & tFields, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bFullSchemaScope )
{
	m_pQword = pQword;
	m_pWarning = tSetup.m_pWarning;
	m_bNotWeighted = bNotWeighted;
	m_tE1Filter = tSetup.m_tE1RankFilter;
	m_bE1RowidDocidOrder = tSetup.m_bE1RowidDocidOrder;
	m_bE1FullSchemaScope = bFullSchemaScope;
	m_iE1SchemaFields = E1SchemaFieldCount ( tSetup );
	m_iAtomPos = pQword->m_iAtomPos;
	m_dQueriedFields = tFields;
	m_bHasWideFields = false;
	if ( tSetup.m_bHasWideFields )
		for ( int i=1; i<FieldMask_t::SIZE && !m_bHasWideFields; i++ )
			if ( m_dQueriedFields[i] )
				m_bHasWideFields = true;
	SetMaxTimeout ( tSetup.m_iMaxTimer );

	if constexpr(STATS)
	{
		m_pStats = tSetup.m_pStats;
	}
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::ExtTerm_T ( ISphQword * pQword, const ISphQwordSetup & tSetup )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
	, m_tE1Filter ( tSetup.m_tE1RankFilter )
{
	m_bE1FullSchemaScope = E1SchemaFieldCount(tSetup)>0;
	m_iE1SchemaFields = E1SchemaFieldCount ( tSetup );
	m_iAtomPos = pQword->m_iAtomPos;
	m_dQueriedFields.SetAll();
	m_bHasWideFields = tSetup.m_bHasWideFields;
	SetMaxTimeout( tSetup.m_iMaxTimer );

	if constexpr ( STATS )
	{
		m_pStats = tSetup.m_pStats;
	}
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
uint32_t ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::CountE1FieldTF ( uint64_t uRef )
{
	m_pQword->SeekHitlist ( uRef );
	++m_uE1ScopedHitlistSeeks;
	uint32_t uTF = 0;
	for ( Hitpos_t uHit=m_pQword->GetNextHit(); uHit!=EMPTY_HIT; uHit=m_pQword->GetNextHit() )
	{
		++m_uE1ScopedPositionsDecoded;
		if ( int(HITMAN::GetField(uHit))==m_iE1ScopedField )
			++uTF;
	}
	return uTF;
}


template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
bool ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::BuildE1ScopedEligibility()
{
	uint32_t uLastWindow = 0;
	if ( !m_pQword->E1DirectOrSupported() || !m_pQword->GetE1DirectLastWindow(uLastWindow) )
		return false;
	const uint64_t uRows = (uint64_t(uLastWindow)+1)*4096;
	const uint64_t uBitmapWords = (uRows+63)/64;
	const uint64_t uScratchBytes = uRows*sizeof(uint32_t)+uBitmapWords*sizeof(uint64_t);
	if ( uRows>INT_MAX || uBitmapWords>INT_MAX || uScratchBytes>64*1024*1024 )
		return false;

	m_dE1ScopedTF.Resize ( int(uRows) );
	// Eligibility is the initialization bitmap: every set bit gets its exact TF
	// below, and ranked lookup reads TF only after testing that bit. Avoid
	// clearing the entire row-space array for sparse field scopes.
	m_dE1ScopedEligibility.Resize ( int(uBitmapWords) );
	m_dE1ScopedEligibility.Fill ( 0 );
	m_dE1ScopedWindowMask.Resize ( 64 );
	const uint32_t uRequestedMask = uint32_t(1)<<m_iE1ScopedField;
	const int64_t iStarted = sphMicroTimer();
	uint64_t uIgnoredBoundReads = 0;
	for ( uint32_t uWindow=0; uWindow<=uLastWindow; ++uWindow )
	{
		m_dE1ScopedWindowMask.Fill ( 0 );
		uint32_t uCardinality = 0, uMaxTF = 0;
		if ( !m_pQword->GetE1DirectWindow ( uWindow, m_dE1ScopedWindowMask.Begin(), nullptr, uCardinality, uMaxTF, uIgnoredBoundReads ) )
			continue;
		m_uE1ScopedMembershipChecks += uCardinality;
		uint64_t uRequested = 0;
		if ( !m_pQword->BeginE1SelectedMeta ( uWindow, m_dE1ScopedWindowMask.Begin(), uRequested ) )
			return false;
		E1SelectedMeta_t tMeta;
		while ( m_pQword->NextE1SelectedMeta ( tMeta, m_uE1ScopedMetadataGroups, m_iE1ScopedField ) )
		{
			m_uE1ScopedAggregateTF += tMeta.m_uTF;
			if ( !(tMeta.m_uMask&uRequestedMask) )
				continue;
			uint32_t uDecodedTF = tMeta.m_uScopedTF;
			if ( tMeta.m_uMask!=uRequestedMask && uDecodedTF )
				++m_uE1ScopedFieldTFMetadataHits;
			else if ( tMeta.m_uMask!=uRequestedMask )
			{
				++m_uE1ScopedFieldTFDecodes;
				uDecodedTF = CountE1FieldTF ( tMeta.m_uRef );
			}
			const uint32_t uTF = E1ScopedExactTF ( tMeta.m_uTF, tMeta.m_uMask, m_iE1ScopedField, uDecodedTF );
			if ( !uTF || !E1ScopedAggregateBoundSafe(uTF,tMeta.m_uTF) )
				continue;
			const uint32_t uRow = uWindow*4096+tMeta.m_uLocal;
			m_dE1ScopedTF[uRow] = uTF;
			m_dE1ScopedEligibility[uRow/64] |= uint64_t(1)<<(uRow%64);
			++m_uE1ScopedTotal;
		}
	}
	m_iE1EligibilityBuildUS = sphMicroTimer()-iStarted;
	return true;
}


template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::Reset ( const ISphQwordSetup & tSetup )
{
	SetMaxTimeout ( tSetup.m_iMaxTimer );
	m_pQword->Reset ();
	tSetup.QwordSetup ( m_pQword );
	m_dStoredHits.Resize(0);
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
const ExtDoc_t * ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::GetDocsChunk()
{
	if ( !m_pQword->m_iDocs )
		return NULL;

	// max_query_time
	if ( TimeExceeded () )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		// interrupt by sitgerm
		if ( g_bInterruptNow )
		{
			if ( m_pWarning )
				*m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	StoredHit_t * pStoredHit = nullptr;
	StoredHit_t * pFirstHit = nullptr;
	if ( m_bE1Ranked )
		m_dStoredHits.Resize(0); // each arbitrary bound-priority batch is normalized independently
	if ( m_bCollectHits )
	{
		int iLength = m_dStoredHits.GetLength();
		m_dStoredHits.Reserve ( iLength+MAX_BLOCK_DOCS );
		pStoredHit = m_dStoredHits.End();
		pFirstHit = pStoredHit-iLength;	// hack to get to m_pData
	}

	int iDoc = 0;
	uint32_t uMinTF = 0;
	uint32_t uEqualBoundTF = 0;
	if ( m_bE1Ranked && m_eE1BoundKind==E1RankedBoundKind_e::MAX_TF && m_iRankThreshold>0 && m_fIDF>0.0f )
	{
		// dl>=0, therefore k1*(1-b+b*dl/256)>=0.3 for the
		// supported k1=1.2,b=.75 expression.  This duplicates the ranker's
		// float operations at dl=0, a true upper bound; integer conversion is
		// the same truncation used by RankerState_Expr_fn::Finalize().
		while ( uMinTF<0x00ffffffU )
		{
			++uMinTF;
			float fUpper = 1000.0f * ( 0.5f + float(uMinTF)/(float(uMinTF)+0.3f)*m_fIDF );
			if ( (int)fUpper>=m_iRankThreshold )
			{
				if ( (int)fUpper==m_iRankThreshold )
					uEqualBoundTF = uMinTF;
				break;
			}
		}
		if ( uEqualBoundTF )
		{
			const int iSaturatedUpper = (int)(1000.0f*(0.5f+m_fIDF));
			if ( iSaturatedUpper==m_iRankThreshold )
				uEqualBoundTF = UINT32_MAX;
			else
				while ( uEqualBoundTF<254 )
				{
					const uint32_t uNext = uEqualBoundTF+1;
					const float fUpper = 1000.0f * ( 0.5f + float(uNext)/(float(uNext)+0.3f)*m_fIDF );
					if ( (int)fUpper!=m_iRankThreshold )
						break;
					uEqualBoundTF = uNext;
				}
		}
	}
	else if ( m_bE1Ranked && m_eE1BoundKind==E1RankedBoundKind_e::MAX_TF && m_iRankThreshold==500 && m_fIDF<=0.0f )
		uEqualBoundTF = UINT32_MAX;
	// The first native-sized ranker chunk is already drawn from highest-bound blocks;
	// later chunks continue by bound priority using the exact live top-K threshold.
	const int iRankBatchLimit = MAX_BLOCK_DOCS-1;
	while ( iDoc<iRankBatchLimit )
	{
		CSphMatch tRankedMatch;
		const CSphMatch * pMatch = nullptr;
		RowID_t tRankedRow = INVALID_ROWID;
		uint32_t uRankedTF = 0;
		if ( m_bE1Ranked )
		{
			const bool bScopedEligibility = m_bE1ScopedTerm;
			const bool bFilterEligibility = m_tE1Filter.m_bEnabled;
			const uint64_t * pEligibility = bScopedEligibility ? m_dE1ScopedEligibility.Begin() : ( bFilterEligibility ? m_dE1Eligibility.Begin() : nullptr );
			const uint32_t uEligibilityWords = bScopedEligibility ? uint32_t(m_dE1ScopedEligibility.GetLength()) : ( bFilterEligibility ? uint32_t(m_dE1Eligibility.GetLength()) : 0 );
			uint64_t * pIneligibleBeforeTF = (bScopedEligibility || bFilterEligibility) ? &m_uE1IneligibleBeforeTF : nullptr;
			const uint32_t uKnownMask = bScopedEligibility ? uint32_t(1)<<m_iE1ScopedField : 0;
			if ( !m_pQword->GetE1RankedDoc ( uMinTF, tRankedRow, uRankedTF, m_uRankBoundEntries, m_uRankBuckets, m_uRankSelectedBlocks, m_uRankSkippedBlocks, m_uRankSkippedDocs, m_uRankDecodedGroups, pEligibility, uEligibilityWords, pIneligibleBeforeTF, uEqualBoundTF, m_uRankWorstTieKey, uKnownMask,
				m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO ? m_fIDF : 0.0f,
				m_eE1BoundKind==E1RankedBoundKind_e::BM25A_RATIO ? m_iRankThreshold : 0, &m_uRankEqualitySkippedBlocks ) )
			{
				m_bE1RankFinished = true;
				m_pQword->m_iDocs = 0;
				break;
			}
			if ( bScopedEligibility )
				uRankedTF = m_dE1ScopedTF[tRankedRow];
			tRankedMatch.m_tRowID = tRankedRow;
			++m_uRankScored;
			pMatch = &tRankedMatch;
		} else
			pMatch = &m_pQword->GetNextDoc();
		const CSphMatch & tMatch = *pMatch;
		if constexpr ( ROWID_LIMITS )
		{
			if ( tMatch.m_tRowID<m_tBoundaries.m_tMinRowID )
				continue;

			if ( tMatch.m_tRowID>m_tBoundaries.m_tMaxRowID )
			{
				m_pQword->m_iDocs = 0;
				break;
			}
		}

		if ( tMatch.m_tRowID==INVALID_ROWID )
		{
			m_pQword->m_iDocs = 0;
			break;
		}

		if ( !m_bHasWideFields )
		{
			// fields 0-31 can be quickly checked right here, right now
			if (!( m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32() ))
				continue;
		} else
		{
			// fields 32+ need to be checked with CollectHitMask() and stuff
			m_pQword->CollectHitMask();
			bool bHasSameFields = false;
			for ( int i=0; i<FieldMask_t::SIZE && !bHasSameFields; i++ )
				bHasSameFields = ( m_pQword->m_dQwordFields[i] & m_dQueriedFields[i] )!=0;
			if ( !bHasSameFields )
				continue;
		}

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_tRowID = tMatch.m_tRowID;
		tDoc.m_uDocFields = m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32(); // OPTIMIZE: only needed for phrase node
		tDoc.m_uExactTF = m_bE1Ranked ? uRankedTF : 0;

		if_const ( USE_BM25 )
			tDoc.m_fTFIDF = float(m_pQword->m_uMatchHits) / float(m_pQword->m_uMatchHits+SPH_BM25_K1) * m_fIDF;

		// store some hit info here, we can't reuse m_dDocs in CollectHits
		// but only if the ranker uses hits
		if ( m_bCollectHits )
		{
			pStoredHit->m_tHitlistOffset = m_pQword->m_iHitlistPos;
			pStoredHit->m_tRowID = tDoc.m_tRowID;
			pStoredHit++;
		}
	}

	// SortRankedBatch: the cursor is bound-priority, while the existing hit/ranker
	// contract is row-monotone. Normalize each decoded batch before exposing it.
	if ( m_bE1Ranked && iDoc>1 )
		std::sort ( m_dDocs, m_dDocs+iDoc, [] ( const ExtDoc_t & a, const ExtDoc_t & b ) { return a.m_tRowID<b.m_tRowID; } );

	if ( m_bCollectHits )
	{
		m_dStoredHits.Resize ( pStoredHit-pFirstHit );
		if ( m_bE1Ranked && m_dStoredHits.GetLength()>1 )
			std::sort ( m_dStoredHits.Begin(), m_dStoredHits.End(), [] ( const StoredHit_t & a, const StoredHit_t & b ) { return a.m_tRowID<b.m_tRowID; } );
	}

	if constexpr ( STATS )
	{
		assert(m_pStats);
		m_pStats->m_iFetchedDocs += iDoc;
	}

	return ReturnDocsChunk ( iDoc, "term", m_pQword->m_sDictWord.cstr() );
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	StoredHit_t * pStoredHit = m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		// setup hitlist reader
		m_pQword->SeekHitlist ( pStoredHit->m_tHitlistOffset );

		while (true)
		{
			// get next hit
			Hitpos_t uHit = m_pQword->GetNextHit();
			if ( uHit==EMPTY_HIT )
			{
				// no more hits; get next acceptable document
				pStoredHit++;
				break;
			}

			if ( !( m_dQueriedFields.Test ( HITMAN::GetField(uHit) ) ) )
				continue;

			ExtHit_t & tHit = m_dHits.Add();
			tHit.m_tRowID = pStoredHit->m_tRowID;
			tHit.m_uHitpos = uHit;
			tHit.m_uQuerypos = (WORD) m_iAtomPos; // assume less that 64K words per query
			tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
		}
	}

	if constexpr ( STATS )
	{
		int nHits = m_dHits.GetLength();
		assert(m_pStats);
		m_pStats->m_iFetchedHits += nHits;
	}

	// we assume that GetHits doesn't get called multiple times for the same docids in pMatched
	// so let's drop the stored hits that we already used
	// so that we won't need to loop through them the next time GetHits gets called for the same docs chunk
	// however, remove only the hits that we've processed. others will be processed in the next GetDocsChunk() call
	int nProcessed = int ( pStoredHit-m_dStoredHits.Begin() );
	m_dStoredHits.Pop();	// end marker
	m_dStoredHits.Remove ( 0, nProcessed );
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
int ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_fIDF = 0.0f;

	ExtQword_t * pQword = hQwords ( m_pQword->m_sWord );
	if ( !m_bNotWeighted && pQword && !pQword->m_bExcluded )
		pQword->m_iQueryPos = Min ( pQword->m_iQueryPos, m_pQword->m_iAtomPos );

	if ( m_bNotWeighted || pQword )
		return m_pQword->m_bExcluded ? -1 : m_pQword->m_iAtomPos;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_pQword->m_sWord;
	tInfo.m_sDictWord = m_pQword->m_sDictWord;
	tInfo.m_iDocs = m_pQword->m_iDocs;
	tInfo.m_iHits = m_pQword->m_iHits;
	tInfo.m_iQueryPos = m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	tInfo.m_fBoost = m_pQword->m_fBoost;
	tInfo.m_bExpanded = m_pQword->m_bExpanded;
	tInfo.m_bExcluded = m_pQword->m_bExcluded;
	hQwords.Add ( tInfo, m_pQword->m_sWord );
	return m_pQword->m_bExcluded ? -1 : m_pQword->m_iAtomPos;
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	if ( m_fIDF<0.0f )
	{
		assert ( hQwords ( m_pQword->m_sWord ) );
		m_fIDF = hQwords ( m_pQword->m_sWord )->m_fIDF;
	}
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_bNotWeighted || m_pQword->m_bExcluded )
		return;

	ExtQword_t & tQword = hQwords[ m_pQword->m_sWord ];

	TermPos_t & tPos = dTermDupes.Add ();
	tPos.m_uAtomPos = (WORD)m_pQword->m_iAtomPos;
	tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
uint64_t ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::GetWordID () const
{
	if ( m_pQword->m_uWordID )
		return m_pQword->m_uWordID;

	return sphFNV64 ( m_pQword->m_sDictWord.cstr() );
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::HintRowID ( RowID_t tRowID )
{
	m_pQword->HintRowID ( tRowID );

	if constexpr ( STATS )
	{
		assert(m_pStats);
		m_pStats->m_iSkips++;
	}
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	m_tBoundaries = tBoundaries;
	HintRowID ( tBoundaries.m_tMinRowID );
}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTerm_T<USE_BM25,ROWID_LIMITS,STATS>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtTerm: %s at: %d ", m_pQword->m_sWord.cstr(), m_pQword->m_iAtomPos );

	if ( m_dQueriedFields.TestAll(true) )
		printf ( "(all)\n" );
	else
	{
		bool bFirst = true;
		printf ( "in: " );
		for ( int iField=0; iField<SPH_MAX_FIELDS; iField++ )
		{
			if ( m_dQueriedFields.Test ( iField ) )
			{
				if ( !bFirst )
					printf ( ", " );
				printf ( "%d", iField );
				bFirst = false;
			}
		}
		printf ( "\n" );
	}
}


//////////////////////////////////////////////////////////////////////////

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
ExtTermHitless_T<USE_BM25,ROWID_LIMITS,STATS>::ExtTermHitless_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
	: BASE ( pQword, dFields, tSetup, bNotWeighted )
{}

template<bool USE_BM25, bool ROWID_LIMITS, bool STATS>
void ExtTermHitless_T<USE_BM25,ROWID_LIMITS,STATS>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	this->m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	typename BASE::StoredHit_t * pStoredHit = this->m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		DWORD uMaxFields = SPH_MAX_FIELDS;
		if ( !this->m_bHasWideFields )
		{
			uMaxFields = 0;
			DWORD uFields = pMatched->m_uDocFields;
			while ( uFields ) // count up to highest bit, max value is 32
			{
				uFields >>= 1;
				uMaxFields++;
			}
		}

		for ( DWORD uFieldPos=0; uFieldPos<uMaxFields; uFieldPos++ )
		{
			if ( ( pMatched->m_uDocFields & ( 1 << uFieldPos ) ) && this->m_dQueriedFields.Test ( uFieldPos ) )
			{
				// emit hit
				ExtHit_t & tHit = this->m_dHits.Add();
				tHit.m_tRowID = pMatched->m_tRowID;
				tHit.m_uHitpos = HITMAN::Create ( uFieldPos, -1 );
				tHit.m_uQuerypos = (WORD) (this->m_iAtomPos);
				tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
			}
		}
	}

	if constexpr ( STATS )
	{
		int nHits = this->m_dHits.GetLength();
		assert ( this->m_pStats );
		this->m_pStats->m_iFetchedHits += nHits;
	}

	// same logic as in ExtTerm_T::CollectHits
	int nProcessed = int ( pStoredHit-this->m_dStoredHits.Begin() );
	this->m_dStoredHits.Pop();	// end marker
	this->m_dStoredHits.Remove ( 0, nProcessed );
}

//////////////////////////////////////////////////////////////////////////

BufferedNode_c::BufferedNode_c()
{
	Reset();
}


void BufferedNode_c::Reset()
{
	m_pRawDocs = nullptr;
	m_pRawDoc = nullptr;
	m_pRawHit = nullptr;
	m_dMyDocs[0].m_tRowID = INVALID_ROWID;
	m_dMyHits.Resize(0);
}


void BufferedNode_c::CopyMatchingHits ( CSphVector<ExtHit_t> & dHits, const ExtDoc_t * pDocs )
{
	m_dMyHits.Add().m_tRowID = INVALID_ROWID;

	dHits.Resize(0);
	const ExtHit_t * pMyHit = m_dMyHits.Begin();

	while ( HasDocs(pDocs) )
	{
		while ( pMyHit->m_tRowID < pDocs->m_tRowID )
			pMyHit++;

		while ( pMyHit->m_tRowID==pDocs->m_tRowID )
			dHits.Add ( *pMyHit++ );

		pDocs++;
	}

	// remove only the hits that we've processed. others will be processed in the next GetDocsChunk() call
	int nProcessed = int ( pMyHit-m_dMyHits.Begin() );
	m_dMyHits.Pop();	// end marker
	m_dMyHits.Remove ( 0, nProcessed );
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T, class NODE >
ExtConditional_T<T,NODE>::ExtConditional_T ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: ExtNode_c { tSetup.m_iMaxTimer }
	, BufferedNode_c ()
	, Acceptor_c ( pQword, pNode, tSetup )
{
	// we still need those hits even if the ranker hints us that we can ignore them
	m_tNode.SetCollectHits();
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T,NODE>::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();
	TermAcceptor_T<T>::Reset();
	m_tNode.Reset(tSetup);
}

//////////////////////////////////////////////////////////////////////////

TermAcceptor_T<TERM_POS_FIELD_LIMIT>::TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & )
	: m_iMaxFieldPos ( pNode->m_dSpec.m_iFieldMaxPos )
{}


inline bool TermAcceptor_T<TERM_POS_FIELD_LIMIT>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )<=m_iMaxFieldPos;
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_START>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1;
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_END>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::IsEnd ( pHit->m_uHitpos );
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_STARTEND>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1 && HITMAN::IsEnd ( pHit->m_uHitpos );
}


TermAcceptor_T<TERM_POS_ZONES>::TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: m_pZoneChecker ( tSetup.m_pZoneChecker )
	, m_dZones ( pNode->m_dSpec.m_dZones )
{}


inline bool TermAcceptor_T<TERM_POS_ZONES>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	assert ( m_pZoneChecker );

	if ( m_tLastZoneRowID!=pHit->m_tRowID )
		m_iCheckFrom = 0;

	m_tLastZoneRowID = pHit->m_tRowID;

	// only check zones that actually match this document
	for ( int i=m_iCheckFrom; i<m_dZones.GetLength(); i++ )
	{
		SphZoneHit_e eState = m_pZoneChecker->IsInZone ( m_dZones[i], pHit, NULL );
		switch ( eState )
		{
			case SPH_ZONE_FOUND:
				return true;
			case SPH_ZONE_NO_DOCUMENT:
				Swap ( m_dZones[i], m_dZones[m_iCheckFrom] );
				m_iCheckFrom++;
				break;
			default:
				break;
		}
	}
	return false;
}


inline void TermAcceptor_T<TERM_POS_ZONES>::Reset()
{
	m_tLastZoneRowID = INVALID_ROWID;
	m_iCheckFrom = 0;
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T, class NODE >
const ExtDoc_t * ExtConditional_T<T,NODE>::GetDocsChunk()
{
	// fetch more docs if needed
	if ( !HasDocs(m_pRawDocs) )
	{
		m_pRawDocs = m_tNode.GetDocsChunk();
		if ( !HasDocs(m_pRawDocs) )
			return nullptr;

		m_pRawDoc = m_pRawDocs;
		m_pRawHit = m_tNode.GetHits(m_pRawDoc);
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;

	const ExtDoc_t * pDoc = m_pRawDoc;
	const ExtHit_t * pHit = m_pRawHit;

	while (true)
	{
		if ( iMyDoc==MAX_BLOCK_DOCS-1 )
			break;

		// did we touch all the hits we had? if so, we're fully done with
		// current raw docs block, and should start a new one
		if ( !HasHits(pHit) )
		{
			m_pRawDocs = m_tNode.GetDocsChunk();
			if ( !HasDocs(m_pRawDocs) )
				break;

			pDoc = m_pRawDocs;
			pHit = m_tNode.GetHits(pDoc);
			continue;
		}

		// scan until next acceptable hit
		while ( pHit->m_tRowID < pDoc->m_tRowID ) // skip leftovers
			pHit++;

		while ( HasHits(pHit) && !Acceptor_c::IsAcceptableHit(pHit) ) // skip unneeded hits
			pHit++;

		if ( !HasHits(pHit) ) // check for eof
			continue;

		// find and emit new document
		while ( pDoc->m_tRowID<pHit->m_tRowID )
			pDoc++; // FIXME? unsafe in broken cases

		assert ( pDoc->m_tRowID==pHit->m_tRowID );
		assert ( iMyDoc<MAX_BLOCK_DOCS-1 );

		m_dMyDocs[iMyDoc++] = *pDoc;
		m_dMyHits.Add ( *(pHit++) );

		// copy acceptable hits for this document
		for ( ; pHit->m_tRowID==pDoc->m_tRowID; pHit++ )
		{
			if ( Acceptor_c::IsAcceptableHit ( pHit ) )
				m_dMyHits.Add ( *pHit );
		}
	}

	m_pRawDoc = pDoc;
	m_pRawHit = pHit;

	assert ( iMyDoc>=0 && iMyDoc<MAX_BLOCK_DOCS );
	m_dMyDocs[iMyDoc].m_tRowID = INVALID_ROWID;

	PrintDocsChunk ( iMyDoc, m_tNode.GetAtomPos(), m_dMyDocs, "cond", this );

	return iMyDoc ? m_dMyDocs : nullptr;
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_tNode.GetAtomPos(), m_dHits.Begin(), this );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::HintRowID ( RowID_t tRowID )
{
	m_tNode.HintRowID ( tRowID );
}


template < TermPosFilter_e T, class NODE >
int ExtConditional_T<T, NODE>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	return m_tNode.GetQwords ( hQwords );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	return m_tNode.SetQwordsIDF ( hQwords );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	return m_tNode.GetTerms ( hQwords, dTermDupes );
}


template < TermPosFilter_e T, class NODE >
uint64_t ExtConditional_T<T, NODE>::GetWordID() const
{
	return m_tNode.GetWordID();
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::SetAtomPos ( int iPos )
{
	m_tNode.SetAtomPos(iPos);
}


template < TermPosFilter_e T, class NODE >
int ExtConditional_T<T, NODE>::GetAtomPos() const
{
	return m_tNode.GetAtomPos();
}


//////////////////////////////////////////////////////////////////////////

ExtTwofer_c::ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
{
	Init ( pFirst, pSecond );
}

inline void	ExtTwofer_c::Init ( ExtNode_i * pLeft, ExtNode_i * pRight )
{
	m_pLeft = std::unique_ptr<ExtNode_i>(pLeft);
	m_pRight = std::unique_ptr<ExtNode_i>(pRight);
	m_pDocL = nullptr;
	m_pDocR = nullptr;
	m_uNodePosL = 0;
	m_uNodePosR = 0;
	m_iAtomPos = ( pLeft && pLeft->GetAtomPos() ) ? pLeft->GetAtomPos() : 0;
	if ( pRight && pRight->GetAtomPos() && pRight->GetAtomPos()<m_iAtomPos && m_iAtomPos!=0 )
		m_iAtomPos = pRight->GetAtomPos();
	int64_t tmTimeout = 0;
	if ( pLeft )
		tmTimeout = pLeft->GetMaxTimeout();
	if ( !tmTimeout && pRight )
		tmTimeout = pRight->GetMaxTimeout();
	SetMaxTimeout ( tmTimeout );
}

void ExtTwofer_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pLeft->Reset ( tSetup );
	m_pRight->Reset ( tSetup );
	m_pDocL = nullptr;
	m_pDocR = nullptr;
}

int ExtTwofer_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax1 = m_pLeft->GetQwords ( hQwords );
	int iMax2 = m_pRight->GetQwords ( hQwords );
	return Max ( iMax1, iMax2 );
}

void ExtTwofer_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pLeft->SetQwordsIDF ( hQwords );
	m_pRight->SetQwordsIDF ( hQwords );
}


void ExtTwofer_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	m_pLeft->GetTerms ( hQwords, dTermDupes );
	m_pRight->GetTerms ( hQwords, dTermDupes );
}


bool ExtTwofer_c::GotHitless ()
{
	return m_pLeft->GotHitless() || m_pRight->GotHitless();
}


void ExtTwofer_c::DebugDumpT ( const char * sName, int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "%s:\n", sName );
	m_pLeft->DebugDump ( iLevel+1 );
	m_pRight->DebugDump ( iLevel+1 );
}


void ExtTwofer_c::SetNodePos ( WORD uPosLeft, WORD uPosRight )
{
	m_uNodePosL = uPosLeft;
	m_uNodePosR = uPosRight;
}


void ExtTwofer_c::HintRowID ( RowID_t tRowID )
{
	m_pLeft->HintRowID ( tRowID );
	m_pRight->HintRowID ( tRowID );
}


uint64_t ExtTwofer_c::GetWordID () const
{
	uint64_t dHash[2];
	dHash[0] = m_pLeft->GetWordID();
	dHash[1] = m_pRight->GetWordID();
	return sphFNV64 ( dHash, sizeof(dHash) );
}


void ExtTwofer_c::SetCollectHits()
{
	if ( m_pLeft )
		m_pLeft->SetCollectHits();

	if ( m_pRight )
		m_pRight->SetCollectHits();
}


NodeEstimate_t ExtTwofer_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tLeft = { 0.0f, 0, 0 };
	if ( m_pLeft )
		tLeft = m_pLeft->Estimate(iTotalDocs);

	NodeEstimate_t tRight = { 0.0f, 0, 0 };
	if ( m_pRight )
		tRight = m_pRight->Estimate(iTotalDocs);

	tLeft += tRight;
	return tLeft;
}


void ExtTwofer_c::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	if ( m_pLeft ) m_pLeft->SetRowidBoundaries(tBoundaries);
	if ( m_pRight ) m_pRight->SetRowidBoundaries(tBoundaries);
}

//////////////////////////////////////////////////////////////////////////
ExtAnd_c::ExtAnd_c ( ExtNode_i * pLeft, ExtNode_i * pRight )
	: ExtTwofer_c ( pLeft, pRight )
{
	m_bEmpty = ( !m_pLeft || !m_pLeft->GetDocsCount() ) || ( !m_pRight || !m_pRight->GetDocsCount() );
	if ( m_pLeft && m_pLeft->GetDocsCount() && ( !m_pRight || !m_pRight->GetDocsCount() ) )
		std::swap ( m_pLeft, m_pRight );
}


const ExtDoc_t * ExtAnd_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, pDocR, m_pLeft.get() ) )
			break;

		if ( !WarmupDocs ( pDocR, pDocL, m_pRight.get() ) )
			break;

		assert ( pDocL && pDocR );
		
		if ( pDocL->m_tRowID==pDocR->m_tRowID )
		{
			// emit it
			ExtDoc_t & tDoc = m_dDocs[iDoc++];
			tDoc.m_tRowID = pDocL->m_tRowID;
			tDoc.m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields; // not necessary
			tDoc.m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;

			// skip it
			pDocL++;
			pDocR++;
		}
		else if ( pDocL->m_tRowID<pDocR->m_tRowID )
			pDocL++;
		else
			pDocR++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "and" );
}


static inline bool IsHitLess ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 )
{
	assert ( pHit1 && pHit2 );
	return ( pHit1->m_uHitpos<pHit2->m_uHitpos ) || ( pHit1->m_uHitpos==pHit2->m_uHitpos && pHit1->m_uQuerypos<=pHit2->m_uQuerypos );
}


struct CmpAndHitReverse_fn
{
	inline bool IsLess ( const ExtHit_t & a, const ExtHit_t & b ) const
	{
		return ( a.m_tRowID<b.m_tRowID || ( a.m_tRowID==b.m_tRowID && a.m_uHitpos<b.m_uHitpos ) || ( a.m_tRowID==b.m_tRowID && a.m_uHitpos==b.m_uHitpos && a.m_uQuerypos>b.m_uQuerypos ) );
	}
};


void ExtAnd_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);
	const WORD uNodePosL = m_uNodePosL;
	const WORD uNodePosR = m_uNodePosR;

	RowID_t tMatchedRowID = INVALID_ROWID;

	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		bool bLeft = false;

		if ( pCurL->m_tRowID < pCurR->m_tRowID )
		{
			if ( pCurL->m_tRowID==tMatchedRowID )
				m_dHits.Add ( *pCurL++ );
			else
			{
				pCurL++;
				continue;
			}

			bLeft = true;
		}
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
		{
			if ( pCurR->m_tRowID==tMatchedRowID )
				m_dHits.Add ( *pCurR++ );
			else
			{
				pCurR++;
				continue;
			}
		}
		else
		{
			tMatchedRowID = pCurL->m_tRowID;

			if ( IsHitLess ( pCurL, pCurR ) )
			{
				m_dHits.Add ( *pCurL++ );
				bLeft = true;
			}
			else
				m_dHits.Add ( *pCurR++ );
		}

		if ( bLeft )
		{
			if ( uNodePosL!=0 )
				m_dHits.Last().m_uNodepos = uNodePosL;
		}
		else
		{
			if ( uNodePosR!=0 )
				m_dHits.Last().m_uNodepos = uNodePosR;
		}
	}

	while ( HasHits(pCurL) && pCurL->m_tRowID==tMatchedRowID )
	{
		m_dHits.Add ( *pCurL++ );
		if ( uNodePosL!=0 )
			m_dHits.Last().m_uNodepos = uNodePosL;
	}

	while ( HasHits(pCurR) && pCurR->m_tRowID==tMatchedRowID )
	{
		m_dHits.Add ( *pCurR++ );
		if ( uNodePosR!=0 )
			m_dHits.Last().m_uNodepos = uNodePosR;
	}

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}


NodeEstimate_t ExtAnd_c::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pLeft && m_pRight );

	auto tLeftEstimate = m_pLeft->Estimate(iTotalDocs);
	auto tRightEstimate = m_pRight->Estimate(iTotalDocs);

	if ( !tLeftEstimate.m_iDocs || !tRightEstimate.m_iDocs || iTotalDocs<=0 )
		return { 0.0f, 0, tLeftEstimate.m_iTerms+tRightEstimate.m_iTerms };

	float fIntersection = float(tLeftEstimate.m_iDocs)/iTotalDocs*float(tRightEstimate.m_iDocs)/iTotalDocs;
	int64_t iDocs = int64_t(fIntersection*iTotalDocs);
	iDocs = Min ( iDocs, Min ( tLeftEstimate.m_iDocs, tRightEstimate.m_iDocs ) );

	float fCost = CalcFTIntersectCost ( tLeftEstimate, tRightEstimate, iTotalDocs, MAX_BLOCK_DOCS, MAX_BLOCK_DOCS );
	return { fCost, iDocs, tLeftEstimate.m_iTerms+tRightEstimate.m_iTerms };
}


void ExtAnd_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAnd", iLevel );
}

//////////////////////////////////////////////////////////////////////////

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::NodeInfo_t::UpdateWideFieldFlag ( const ISphQwordSetup & tSetup )
{
	m_bHasWideFields = false;
	if ( tSetup.m_bHasWideFields )
		for ( int i=1; i<FieldMask_t::SIZE && !m_bHasWideFields; i++ )
			if ( m_dQueriedFields[i] )
				m_bHasWideFields = true;
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::NodeInfo_t::FitsFields() const
{
	if ( !m_bHasWideFields )
	{
		// fields 0-31 can be quickly checked right here, right now
		if (!( m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32() ))
			return false;
	} else
	{
		// fields 32+ need to be checked with CollectHitMask() and stuff
		m_pQword->CollectHitMask();
		bool bHasSameFields = false;
		for ( int i=0; i<FieldMask_t::SIZE && !bHasSameFields; i++ )
			bHasSameFields = ( m_pQword->m_dQwordFields[i] & m_dQueriedFields[i] )!=0;

		if ( !bHasSameFields )
			return false;
	}

	return true;
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::HitWithQpos_t::HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos )
	: m_iNode ( iNode )
	, m_uHit ( uHit )
	, m_uQueryPos ( uQueryPos )
{}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::ExtMultiAnd_T ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup, bool bE1Or )
	: ExtNode_c { tSetup.m_iMaxTimer }
	, m_dWordIds ( dXQNodes.GetLength() )
	, m_tQueue ( dXQNodes.GetLength() )
	, m_bE1RowidDocidOrder ( tSetup.m_bE1RowidDocidOrder )
	, m_bE1Or ( bE1Or )
	, m_iE1SchemaFields ( E1SchemaFieldCount(tSetup) )
	, m_tE1Filter ( tSetup.m_tE1RankFilter )
{
	m_dNodes.Resize ( dXQNodes.GetLength() );
	ARRAY_FOREACH ( i, m_dNodes )
	{
		NodeInfo_t & tNode = m_dNodes[i];
		const XQNode_t & tXQNode = *dXQNodes[i];

		tNode.m_pQword = CreateQueryWord ( tXQNode.dWord(0), tSetup );
		assert ( tNode.m_pQword );
		tNode.m_iAtomPos = tNode.m_pQword->m_iAtomPos;
		tNode.m_uNodepos = (WORD)i;
		tNode.m_bNotWeighted = tXQNode.m_bNotWeighted;
		tNode.m_dQueriedFields = tXQNode.m_dSpec.m_dFieldMask;
		tNode.m_bE1FullSchemaScope = E1FieldScopeCoversSchema ( tXQNode.m_dSpec, tSetup );
		tNode.UpdateWideFieldFlag(tSetup);
	}

	m_dNodes.Sort ( SelectivitySorter_t() );
	m_iNodesSet = m_dNodes.GetLength();

	m_pWarning = tSetup.m_pWarning;
	m_pStats = tSetup.m_pStats;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::~ExtMultiAnd_T()
{
	if ( m_bE1Ranked && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "%s terms=%d windows_total=%llu windows_pruned=%llu windows_scored=%llu boolean_matches=%llu intersection_windows=%llu intersection_mask_ops=%llu descriptors=%d best_first_windows_visited=%llu best_first_windows_skipped=%llu exact_union_total=%llu candidates_generated=%llu candidate_mask_rows=%llu surviving_mask_rows=%llu candidate_bound_rejects=%llu tie_bound_rejects=%llu candidate_words_examined=%llu bound_classes_rejected=%llu whole_words_rejected=%llu scalar_candidate_inspections=%llu candidates_scored=%llu essential_repartitions=%llu average_essential_terms=%.3f nonessential_probes=%llu nonessential_hits=%llu bitmap_ops=%llu sparse_seeks=%llu container_mask_ops=%llu bound_reads=%llu random_tf_probes=%llu batch_tf_rows_requested=%llu batch_tf_rows_written=%llu batch_metadata_groups_decoded=%llu tf_probes=%llu metadata_groups_decoded=%llu final_threshold=%d hitlist_seeks=0 decoded_positions=0 general_factor_finalizations=0\n",
			m_bE1Or ? "E1_MULTI_OR" : "E1_MULTI_AND",
			m_dNodes.GetLength(),
			(unsigned long long)m_uE1WindowsTotal, (unsigned long long)m_uE1WindowsPruned,
			(unsigned long long)m_uE1WindowsScored, (unsigned long long)m_uE1BooleanMatches,
			(unsigned long long)m_uE1IntersectionWindows, (unsigned long long)m_uE1IntersectionMaskOps,
			m_dE1OrWindows.GetLength(), (unsigned long long)m_uE1BestFirstVisited,
			(unsigned long long)m_uE1BestFirstSkipped, (unsigned long long)m_uE1ExactUnionTotal,
			(unsigned long long)m_uE1CandidatesGenerated, (unsigned long long)m_uE1CandidateMaskRows,
			(unsigned long long)m_uE1CandidateMaskRows,
			(unsigned long long)m_uE1CandidateBoundRejects, (unsigned long long)m_uE1TieBoundRejects,
			(unsigned long long)m_uE1CandidateWordsExamined, (unsigned long long)m_uE1BoundClassesRejected,
			(unsigned long long)m_uE1WholeWordsRejected, (unsigned long long)m_uE1ScalarCandidateInspections,
			(unsigned long long)m_uE1CandidatesScored,
			(unsigned long long)m_uE1EssentialRepartitions,
			m_uE1EssentialRepartitions ? double(m_uE1EssentialTerms)/double(m_uE1EssentialRepartitions) : 0.0,
			(unsigned long long)m_uE1NonessentialProbes, (unsigned long long)m_uE1NonessentialHits,
			(unsigned long long)m_uE1BitmapOps, (unsigned long long)m_uE1SparseSeeks,
			(unsigned long long)m_uE1ContainerOps, (unsigned long long)m_uE1BoundReads,
			(unsigned long long)m_uE1RandomTFProbes,
			(unsigned long long)m_uE1BatchTFRowsRequested, (unsigned long long)m_uE1BatchTFRowsWritten,
			(unsigned long long)m_uE1BatchMetadataGroups,
			(unsigned long long)m_uE1TFProbes, (unsigned long long)m_uE1MetadataGroups,
			m_iE1FinalThreshold );
	if ( m_bE1Ranked && m_tE1Filter.m_bEnabled && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_FILTER_OR masks_built=%llu rows_examined=%llu candidates_before=%llu candidates_after=%llu tf_rows_requested=%llu dl_rows=%llu scored_rows=%llu generic_filter_bypass=1\n",
			(unsigned long long)m_uE1FilterMasksBuilt, (unsigned long long)m_uE1FilterRowsExamined,
			(unsigned long long)m_uE1FilterCandidatesBefore, (unsigned long long)m_uE1FilterCandidatesAfter,
			(unsigned long long)m_uE1BatchTFRowsRequested, (unsigned long long)m_uE1CandidatesScored,
			(unsigned long long)m_uE1CandidatesScored );
	if ( m_bE1Ranked && m_bE1FusedAnd4 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_AND4_FUSED missing_window_short_circuits=%llu masks_fetched=%llu fused_words=%llu intersection_nonempty_words=%llu class_combinations=%llu coarse_rejects=%llu coarse_survivors=%llu bound_reads=%llu intersection_mask_ops=%llu\n",
			(unsigned long long)m_uE1MissingWindowShortCircuits, (unsigned long long)m_uE1MasksFetched,
			(unsigned long long)m_uE1FusedWords, (unsigned long long)m_uE1IntersectionNonemptyWords,
			(unsigned long long)m_uE1ClassCombinations, (unsigned long long)m_uE1CoarseRejects,
			(unsigned long long)m_uE1CoarseSurvivors, (unsigned long long)m_uE1BoundReads,
			(unsigned long long)m_uE1IntersectionMaskOps );
	if ( m_bE1Ranked && m_bE1StagedAnd4 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_AND4_STAGED stage0_input=%llu tf_probe_order=%d,%d,%d,%d stage1_rejects=%llu stage1_survivors=%llu stage1_batch_requested=%llu stage1_batch_written=%llu stage1_metadata_groups=%llu stage2_rejects=%llu stage2_survivors=%llu stage2_batch_requested=%llu stage2_batch_written=%llu stage2_metadata_groups=%llu stage3_rejects=%llu stage3_survivors=%llu stage3_batch_requested=%llu stage3_batch_written=%llu stage3_metadata_groups=%llu stage4_rejects=%llu stage4_survivors=%llu stage4_batch_requested=%llu stage4_batch_written=%llu stage4_metadata_groups=%llu final_all_term_survivors=%llu random_tf_probes=%llu\n",
			(unsigned long long)m_uE1Stage0Input,
			m_dNodes[m_dE1TFProbeOrder[0]].m_iAtomPos, m_dNodes[m_dE1TFProbeOrder[1]].m_iAtomPos,
			m_dNodes[m_dE1TFProbeOrder[2]].m_iAtomPos, m_dNodes[m_dE1TFProbeOrder[3]].m_iAtomPos,
			(unsigned long long)m_dE1StageRejects[0], (unsigned long long)m_dE1StageSurvivors[0], (unsigned long long)m_dE1StageBatchRequested[0], (unsigned long long)m_dE1StageBatchWritten[0], (unsigned long long)m_dE1StageMetadataGroups[0],
			(unsigned long long)m_dE1StageRejects[1], (unsigned long long)m_dE1StageSurvivors[1], (unsigned long long)m_dE1StageBatchRequested[1], (unsigned long long)m_dE1StageBatchWritten[1], (unsigned long long)m_dE1StageMetadataGroups[1],
			(unsigned long long)m_dE1StageRejects[2], (unsigned long long)m_dE1StageSurvivors[2], (unsigned long long)m_dE1StageBatchRequested[2], (unsigned long long)m_dE1StageBatchWritten[2], (unsigned long long)m_dE1StageMetadataGroups[2],
			(unsigned long long)m_dE1StageRejects[3], (unsigned long long)m_dE1StageSurvivors[3], (unsigned long long)m_dE1StageBatchRequested[3], (unsigned long long)m_dE1StageBatchWritten[3], (unsigned long long)m_dE1StageMetadataGroups[3],
			(unsigned long long)m_dE1StageSurvivors[3], (unsigned long long)m_uE1RandomTFProbes );
	if ( m_bE1Ranked && m_bE1ScopedAnd2 && !m_bE1MixedFieldAnd2 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_SCOPED_AND2 requested_field=%d exact_total=%llu bitmap_bytes=%llu tf_bytes=%llu scratch_bytes=%llu membership_checks=%llu aggregate_tf_available=%llu field_local_tf_decodes=%llu hitlist_seeks=%llu positions_decoded=%llu metadata_groups_membership=%llu intersection_windows=%llu intersection_mask_ops=%llu bound_reads=%llu bound_survivors=%llu tf_rows=%llu scored_rows=%llu final_threshold=%d build_us=%lld aggregate_bound_safe=1\n",
			m_iE1ScopedAndField, (unsigned long long)m_uE1ScopedAndExactTotal,
			(unsigned long long)(m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t),
			(unsigned long long)(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD),
			(unsigned long long)((m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t)+(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD)),
			(unsigned long long)m_uE1ScopedAndMembershipChecks, (unsigned long long)m_uE1ScopedAndAggregateTF,
			(unsigned long long)m_uE1ScopedAndFieldTFDecodes, (unsigned long long)m_uE1ScopedAndHitlistSeeks,
			(unsigned long long)m_uE1ScopedAndPositionsDecoded, (unsigned long long)m_uE1ScopedAndMembershipGroups,
			(unsigned long long)m_uE1IntersectionWindows, (unsigned long long)m_uE1IntersectionMaskOps,
			(unsigned long long)m_uE1BoundReads, (unsigned long long)m_uE1CandidateMaskRows,
			(unsigned long long)m_uE1ScopedAndTFRows, (unsigned long long)m_uE1CandidatesScored,
			m_iE1FinalThreshold, (long long)m_iE1ScopedAndBuildUS );
	if ( m_bE1Ranked && m_bE1MixedFieldAnd2 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_MIXED_FIELD_AND2 field_masks=%u,%u requested_fields=%d,%d exact_intersection=%llu bitmap_bytes=%llu tf_bytes=%llu scratch_bytes=%llu membership_checks=%llu aggregate_tf_available=%llu field_local_tf_decodes=%llu hitlist_seeks=%llu positions_decoded=%llu metadata_groups_membership=%llu intersection_windows=%llu intersection_mask_ops=%llu bound_reads=%llu bound_survivors=%llu tf_rows=%llu scored_rows=%llu K_dynamic=1 final_threshold=%d build_us=%lld aggregate_bound_safe=1\n",
			unsigned(m_dNodes[0].m_dQueriedFields.GetMask32()), unsigned(m_dNodes[1].m_dQueriedFields.GetMask32()),
			m_dE1ScopedAndFields[0], m_dE1ScopedAndFields[1], (unsigned long long)m_uE1ScopedAndExactTotal,
			(unsigned long long)(m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t),
			(unsigned long long)(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD),
			(unsigned long long)((m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t)+(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD)),
			(unsigned long long)m_uE1ScopedAndMembershipChecks, (unsigned long long)m_uE1ScopedAndAggregateTF,
			(unsigned long long)m_uE1ScopedAndFieldTFDecodes, (unsigned long long)m_uE1ScopedAndHitlistSeeks,
			(unsigned long long)m_uE1ScopedAndPositionsDecoded, (unsigned long long)m_uE1ScopedAndMembershipGroups,
			(unsigned long long)m_uE1IntersectionWindows, (unsigned long long)m_uE1IntersectionMaskOps,
			(unsigned long long)m_uE1BoundReads, (unsigned long long)m_uE1CandidateMaskRows,
			(unsigned long long)m_uE1ScopedAndTFRows, (unsigned long long)m_uE1CandidatesScored,
			m_iE1FinalThreshold, (long long)m_iE1ScopedAndBuildUS );
	if ( m_bE1Ranked && m_bE1ScopedOr2 && !m_bE1MixedFieldOr2 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_SCOPED_OR2 requested_field=%d exact_union=%llu overlap=%llu bitmap_bytes=%llu tf_bytes=%llu scratch_bytes=%llu membership_checks=%llu aggregate_tf_available=%llu field_local_tf_decodes=%llu hitlist_seeks=%llu positions_decoded=%llu metadata_groups_membership=%llu mask_words=%llu bound_rejects=%llu tf_rows=%llu scored_rows=%llu final_threshold=%d build_us=%lld aggregate_bound_safe=1\n",
			m_iE1ScopedAndField, (unsigned long long)m_uE1ScopedAndExactTotal,
			(unsigned long long)m_uE1ScopedOrOverlap,
			(unsigned long long)(m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t),
			(unsigned long long)(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD),
			(unsigned long long)((m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t)+(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD)),
			(unsigned long long)m_uE1ScopedAndMembershipChecks, (unsigned long long)m_uE1ScopedAndAggregateTF,
			(unsigned long long)m_uE1ScopedAndFieldTFDecodes, (unsigned long long)m_uE1ScopedAndHitlistSeeks,
			(unsigned long long)m_uE1ScopedAndPositionsDecoded, (unsigned long long)m_uE1ScopedAndMembershipGroups,
			(unsigned long long)m_uE1CandidateWordsExamined, (unsigned long long)m_uE1CandidateBoundRejects,
			(unsigned long long)m_uE1ScopedAndTFRows, (unsigned long long)m_uE1CandidatesScored,
			m_iE1FinalThreshold, (long long)m_iE1ScopedAndBuildUS );
	if ( m_bE1Ranked && m_bE1MixedFieldOr2 && getenv("MANTICORE_E1_RANK_TRACE") )
		fprintf ( stderr, "E1_MIXED_FIELD_OR2 field_masks=%u,%u requested_fields=%d,%d exact_union=%llu overlap=%llu bitmap_bytes=%llu tf_bytes=%llu scratch_bytes=%llu membership_checks=%llu aggregate_tf_available=%llu field_local_tf_decodes=%llu hitlist_seeks=%llu positions_decoded=%llu metadata_groups_membership=%llu mask_words=%llu bound_rejects=%llu tf_rows=%llu scored_rows=%llu K_dynamic=1 final_threshold=%d build_us=%lld aggregate_bound_safe=1\n",
			unsigned(m_dNodes[0].m_dQueriedFields.GetMask32()), unsigned(m_dNodes[1].m_dQueriedFields.GetMask32()),
			m_dE1ScopedAndFields[0], m_dE1ScopedAndFields[1], (unsigned long long)m_uE1ScopedAndExactTotal,
			(unsigned long long)m_uE1ScopedOrOverlap,
			(unsigned long long)(m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t),
			(unsigned long long)(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD),
			(unsigned long long)((m_dE1ScopedAndEligibility[0].GetLength()+m_dE1ScopedAndEligibility[1].GetLength())*sizeof(uint64_t)+(m_dE1ScopedAndTF[0].GetLength()+m_dE1ScopedAndTF[1].GetLength())*sizeof(DWORD)),
			(unsigned long long)m_uE1ScopedAndMembershipChecks, (unsigned long long)m_uE1ScopedAndAggregateTF,
			(unsigned long long)m_uE1ScopedAndFieldTFDecodes, (unsigned long long)m_uE1ScopedAndHitlistSeeks,
			(unsigned long long)m_uE1ScopedAndPositionsDecoded, (unsigned long long)m_uE1ScopedAndMembershipGroups,
			(unsigned long long)m_uE1CandidateWordsExamined, (unsigned long long)m_uE1CandidateBoundRejects,
			(unsigned long long)m_uE1ScopedAndTFRows, (unsigned long long)m_uE1CandidatesScored,
			m_iE1FinalThreshold, (long long)m_iE1ScopedAndBuildUS );
	for ( auto & i : m_dNodes )
		SafeDelete ( i.m_pQword );
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::EnableE1Ranked()
{
	if constexpr ( ROWID_LIMITS )
		return false;
	// OR admission probes this method while constructing the physical node and
	// the ranker enables it again later. Scoped preparation is query-owned and
	// must run exactly once.
	if ( m_bE1Ranked )
		return true;
#if defined(MANTICORE_TEST)
	if ( g_bE1TestForceGenericRanked ) { ++g_tE1TestRankStats.m_uFallbacks; return false; }
#endif
	if ( getenv("MANTICORE_E1_RANK_TRACE") )
	{
		const bool bFullSchema = m_dNodes.all_of ( [] ( const NodeInfo_t & tNode ) { return tNode.m_bE1FullSchemaScope; } );
		fprintf ( stderr, "E1_FIELD_SCOPE path=%s schema_fields=%d full_schema=%d terms=%d\n", m_bE1Or ? "or" : "and", m_iE1SchemaFields, int(bFullSchema), m_dNodes.GetLength() );
	}
	if ( m_tE1Filter.m_bEnabled && ( !m_bE1Or || m_dNodes.GetLength()!=2 ) )
		return false;
	if ( !( m_dNodes.GetLength()==2 || m_dNodes.GetLength()==4 ) )
		return false;
	if ( m_bE1Or && !( m_bE1Or && ( m_dNodes.GetLength()==2 || m_dNodes.GetLength()==4 ) ) )
		return false;
	const int iScoped0 = m_dNodes.GetLength()==2 ? E1ScopedSingleField ( m_iE1SchemaFields, m_dNodes[0].m_bE1FullSchemaScope, m_dNodes[0].m_dQueriedFields.GetMask32() ) : -1;
	const int iScoped1 = m_dNodes.GetLength()==2 ? E1ScopedSingleField ( m_iE1SchemaFields, m_dNodes[1].m_bE1FullSchemaScope, m_dNodes[1].m_dQueriedFields.GetMask32() ) : -1;
	const int iScopedAndField = m_dNodes.GetLength()==2 ? ( m_bE1Or ? E1ScopedOr2Field : E1ScopedAnd2Field ) ( m_iE1SchemaFields, m_dNodes[0].m_bE1FullSchemaScope, m_dNodes[0].m_dQueriedFields.GetMask32(), m_dNodes[1].m_bE1FullSchemaScope, m_dNodes[1].m_dQueriedFields.GetMask32() ) : -1;
	int iMixed0 = -1, iMixed1 = -1;
	const bool bMixedAnd2 = !m_bE1Or && m_dNodes.GetLength()==2 && E1MixedScopedAnd2Fields ( m_iE1SchemaFields,
		m_dNodes[0].m_bE1FullSchemaScope, m_dNodes[0].m_dQueriedFields.GetMask32(),
		m_dNodes[1].m_bE1FullSchemaScope, m_dNodes[1].m_dQueriedFields.GetMask32(), iMixed0, iMixed1 );
	const bool bMixedOr2 = m_bE1Or && m_dNodes.GetLength()==2 && E1MixedScopedOr2Fields ( m_iE1SchemaFields,
		m_dNodes[0].m_bE1FullSchemaScope, m_dNodes[0].m_dQueriedFields.GetMask32(),
		m_dNodes[1].m_bE1FullSchemaScope, m_dNodes[1].m_dQueriedFields.GetMask32(), iMixed0, iMixed1 );
	const bool bForceAllScoped = getenv("MANTICORE_E1_FORCE_GENERIC_SCOPED");
	m_bE1MixedFieldAnd2 = bMixedAnd2 && !m_tE1Filter.m_bEnabled && !bForceAllScoped && !getenv("MANTICORE_E1_FORCE_GENERIC_MIXED_AND2");
	m_bE1MixedFieldOr2 = bMixedOr2 && !m_tE1Filter.m_bEnabled && !bForceAllScoped && !getenv("MANTICORE_E1_FORCE_GENERIC_MIXED_OR2");
	m_bE1ScopedAnd2 = !m_bE1Or && !m_tE1Filter.m_bEnabled && m_dNodes.GetLength()==2 && !bForceAllScoped
		&& ( iScopedAndField>=0 || m_bE1MixedFieldAnd2 );
	m_bE1ScopedOr2 = m_bE1Or && !m_tE1Filter.m_bEnabled && m_dNodes.GetLength()==2 && !bForceAllScoped
		&& ( iScopedAndField>=0 || m_bE1MixedFieldOr2 );
	m_iE1ScopedAndField = ( m_bE1ScopedAnd2 || m_bE1ScopedOr2 ) ? iScopedAndField : -1;
	m_dE1ScopedAndFields[0] = ( m_bE1MixedFieldAnd2 || m_bE1MixedFieldOr2 ) ? iMixed0 : m_iE1ScopedAndField;
	m_dE1ScopedAndFields[1] = ( m_bE1MixedFieldAnd2 || m_bE1MixedFieldOr2 ) ? iMixed1 : m_iE1ScopedAndField;
	if ( !m_bE1ScopedAnd2 && !m_bE1Or && m_dNodes.GetLength()==2 && getenv("MANTICORE_E1_RANK_TRACE")
		&& !m_dNodes.all_of ( [] ( const NodeInfo_t & tNode ) { return tNode.m_bE1FullSchemaScope; } ) )
		fprintf ( stderr, "%s reason=%s masks=%u,%u scoped_fields=%d,%d\n",
			bMixedAnd2 ? "E1_MIXED_FIELD_AND2_FALLBACK" : "E1_SCOPED_AND2_FALLBACK",
			bMixedAnd2 && getenv("MANTICORE_E1_FORCE_GENERIC_MIXED_AND2") ? "forced_generic" : "ineligible",
			unsigned(m_dNodes[0].m_dQueriedFields.GetMask32()), unsigned(m_dNodes[1].m_dQueriedFields.GetMask32()), iScoped0, iScoped1 );
	if ( !m_bE1ScopedOr2 && m_bE1Or && m_dNodes.GetLength()==2 && getenv("MANTICORE_E1_RANK_TRACE")
		&& !m_dNodes.all_of ( [] ( const NodeInfo_t & tNode ) { return tNode.m_bE1FullSchemaScope; } ) )
		fprintf ( stderr, "%s reason=%s masks=%u,%u scoped_fields=%d,%d\n",
			bMixedOr2 ? "E1_MIXED_FIELD_OR2_FALLBACK" : "E1_SCOPED_OR2_FALLBACK",
			bMixedOr2 && getenv("MANTICORE_E1_FORCE_GENERIC_MIXED_OR2") ? "forced_generic" : "ineligible",
			unsigned(m_dNodes[0].m_dQueriedFields.GetMask32()), unsigned(m_dNodes[1].m_dQueriedFields.GetMask32()), iScoped0, iScoped1 );
	uint32_t uDirectAndLast = UINT32_MAX;
	bool bHasRatioBounds = false;
	for ( const auto & tNode : m_dNodes )
	{
		if ( !tNode.m_bE1FullSchemaScope && !m_bE1ScopedAnd2 && !m_bE1ScopedOr2 )
			return false;
		if ( !tNode.m_pQword->E1DirectContainerSupported() )
		{
			if ( m_bE1Or && !tNode.m_pQword->m_iDocs )
				continue;
			return false;
		}
		bHasRatioBounds |= tNode.m_pQword->GetE1RankedBoundKind()==E1RankedBoundKind_e::BM25A_RATIO;
		uint32_t uLast = 0;
		if ( !tNode.m_pQword->GetE1DirectLastWindow(uLast) )
		{
			if ( m_bE1Or && !tNode.m_pQword->m_iDocs )
				continue;
			return false;
		}
#if defined(MANTICORE_TEST)
		if ( g_bE1TestLastWindow )
			uLast = g_uE1TestLastWindow;
#endif
		if ( m_bE1Or )
		{
			m_uE1LastWindow = m_uE1LastWindow==UINT32_MAX ? uLast : Max ( m_uE1LastWindow, uLast );
		}
		else
			uDirectAndLast = uDirectAndLast==UINT32_MAX ? uLast : Min ( uDirectAndLast, uLast );
	}
	if ( m_bE1Or && m_uE1LastWindow==UINT32_MAX )
		return false;
	m_dE1Canonical.Resize ( m_dNodes.GetLength() );
	for ( int i=0; i<m_dNodes.GetLength(); ++i )
		m_dE1Canonical[i] = i;
	std::sort ( m_dE1Canonical.Begin(), m_dE1Canonical.End(), [this] ( int a, int b ) { return m_dNodes[a].m_iAtomPos<m_dNodes[b].m_iAtomPos; } );
	for ( int i=0; i<m_dE1Canonical.GetLength(); ++i )
		if ( m_dNodes[m_dE1Canonical[i]].m_iAtomPos!=i+1 )
			return false;
	m_bE1DirectAnd = !m_bE1Or;
	m_bE1StagedAnd4 = m_bE1DirectAnd && m_dNodes.GetLength()==4;
	m_bE1FusedAnd4 = m_bE1StagedAnd4;
	if ( m_bE1DirectAnd )
		m_uE1LastWindow = uDirectAndLast;
	if ( m_bE1ScopedAnd2 || m_bE1ScopedOr2 )
	{
		if ( !E1ScopedScratchAllowed(m_uE1LastWindow) )
		{
#if defined(MANTICORE_TEST)
			if ( g_bE1TestLastWindow )
				++g_uE1TestScratchDeclines;
#endif
			if ( getenv("MANTICORE_E1_RANK_TRACE") )
				fprintf ( stderr, "%s reason=scratch_limit last_window=%u limit_bytes=%llu\n",
					m_bE1ScopedOr2 ? "E1_SCOPED_OR2_FALLBACK" : "E1_SCOPED_AND2_FALLBACK",
					m_uE1LastWindow, (unsigned long long)E1_SCOPED_SCRATCH_LIMIT );
			return false;
		}
		// V7 ratio bounds are admitted only by the full-schema single-term
		// executor. Preserve the scoped scratch-decline gate above, then fail
		// closed before allocating or building any scoped direct state.
		if ( bHasRatioBounds )
		{
			if ( getenv("MANTICORE_E1_RANK_TRACE") )
				fprintf ( stderr, "E1_V7_FALLBACK reason=field_scope bound_kind=bm25a_ratio\n" );
			return false;
		}
		const int64_t iStarted = sphMicroTimer();
		if ( !BuildE1ScopedAndTerm ( 0, m_uE1LastWindow ) || !BuildE1ScopedAndTerm ( 1, m_uE1LastWindow ) )
			return false;
		for ( int i=0; i<m_dE1ScopedAndEligibility[0].GetLength(); ++i )
		{
			const uint64_t uLeft = m_dE1ScopedAndEligibility[0][i];
			const uint64_t uRight = m_dE1ScopedAndEligibility[1][i];
			m_uE1ScopedOrOverlap += uint64_t(__builtin_popcountll(uLeft&uRight));
			m_uE1ScopedAndExactTotal += uint64_t(__builtin_popcountll(m_bE1ScopedOr2 ? (uLeft|uRight) : (uLeft&uRight)));
		}
		m_iE1ScopedAndBuildUS = sphMicroTimer()-iStarted;
	}
	// V7 stores BM25A ratio bounds, not max-TF. Keep the direct container and
	// exact-TF paths, but disable every max-TF pruning decision. Ratio bytes
	// must never be reinterpreted as term frequency.
	m_bE1UnboundedCompat = bHasRatioBounds;
	if ( m_bE1UnboundedCompat && m_dNodes.GetLength()!=2 )
		return false;
	if ( m_bE1StagedAnd4 )
	{
		for ( int i=0; i<4; ++i )
		{
			m_dE1TFProbeOrder[i] = i;
			m_dE1DirectOrder[i] = i;
		}
		// Positive IDF estimates useful tightening; document frequency estimates
		// metadata-decode cost. Probe the best tightening/cost ratio first.
		std::sort ( m_dE1TFProbeOrder, m_dE1TFProbeOrder+4, [this] ( int a, int b ) {
			const double fA = double(Max(0.0f,m_dNodes[a].m_fIDF))/double(m_dNodes[a].m_pQword->m_iDocs+1);
			const double fB = double(Max(0.0f,m_dNodes[b].m_fIDF))/double(m_dNodes[b].m_pQword->m_iDocs+1);
			return fA>fB || ( fA==fB && m_dNodes[a].m_pQword->m_iDocs<m_dNodes[b].m_pQword->m_iDocs );
		} );
		std::sort ( m_dE1DirectOrder, m_dE1DirectOrder+4, [this] ( int a, int b ) {
			return m_dNodes[a].m_pQword->m_iDocs<m_dNodes[b].m_pQword->m_iDocs;
		} );
		m_dE1AndMasks.Resize ( 4*64 );
		m_dE1AndBoundWords.Resize ( 4*64 );
		m_dE1StagedExactTF.Resize ( 4*E1_AND_WINDOW_ROWS );
	}
	// Commit direct execution only after every fallible admission/preparation step.
	// Otherwise an oversized scoped scratch request can return false while
	// GetDocsChunk() still observes a half-initialized direct executor.
	m_bE1Ranked = true;
	m_bCollectHits = false;
#if defined(MANTICORE_TEST)
	if ( m_bE1Or ) ++g_tE1TestRankStats.m_uDirectOr; else ++g_tE1TestRankStats.m_uDirectAnd;
#endif
	return true;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
uint32_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::CountE1ScopedAndFieldTF ( int iNode, uint64_t uRef )
{
	auto * pQword = m_dNodes[iNode].m_pQword;
	pQword->SeekHitlist ( uRef );
	++m_uE1ScopedAndHitlistSeeks;
	uint32_t uTF = 0;
	for ( Hitpos_t uHit=pQword->GetNextHit(); uHit!=EMPTY_HIT; uHit=pQword->GetNextHit() )
	{
		++m_uE1ScopedAndPositionsDecoded;
		if ( int(HITMAN::GetField(uHit))==m_dE1ScopedAndFields[iNode] )
			++uTF;
	}
	return uTF;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::BuildE1ScopedAndTerm ( int iNode, uint32_t uLastWindow )
{
	const uint64_t uRows = (uint64_t(uLastWindow)+1)*E1_AND_WINDOW_ROWS;
	const uint64_t uWords = (uRows+63)/64;
	auto & dEligibility = m_dE1ScopedAndEligibility[iNode];
	auto & dTF = m_dE1ScopedAndTF[iNode];
	dEligibility.Resize ( int(uWords) );
	dEligibility.Fill ( 0 );
	dTF.Resize ( int(uRows) );
	dTF.Fill ( 0 );
	CSphVector<uint64_t> dWindow;
	dWindow.Resize(64);
	const uint32_t uRequestedMask = uint32_t(1)<<m_dE1ScopedAndFields[iNode];
	uint64_t uIgnoredBoundReads = 0;
	auto * pQword = m_dNodes[iNode].m_pQword;
	for ( uint32_t uWindow=0; uWindow<=uLastWindow; ++uWindow )
	{
		dWindow.Fill(0);
		uint32_t uCardinality = 0, uMaxTF = 0;
		if ( !pQword->GetE1DirectWindow ( uWindow, dWindow.Begin(), nullptr, uCardinality, uMaxTF, uIgnoredBoundReads ) )
			continue;
		m_uE1ScopedAndMembershipChecks += uCardinality;
		uint64_t uRequested = 0;
		if ( !pQword->BeginE1SelectedMeta ( uWindow, dWindow.Begin(), uRequested ) )
			return false;
		E1SelectedMeta_t tMeta;
		while ( pQword->NextE1SelectedMeta ( tMeta, m_uE1ScopedAndMembershipGroups, m_dE1ScopedAndFields[iNode] ) )
		{
			m_uE1ScopedAndAggregateTF += tMeta.m_uTF;
			if ( !(tMeta.m_uMask&uRequestedMask) )
				continue;
			uint32_t uDecodedTF = tMeta.m_uScopedTF;
			if ( tMeta.m_uMask!=uRequestedMask && !uDecodedTF )
			{
				++m_uE1ScopedAndFieldTFDecodes;
				uDecodedTF = CountE1ScopedAndFieldTF ( iNode, tMeta.m_uRef );
			}
			const uint32_t uTF = E1ScopedExactTF ( tMeta.m_uTF, tMeta.m_uMask, m_dE1ScopedAndFields[iNode], uDecodedTF );
			if ( !uTF || !E1ScopedAggregateBoundSafe(uTF,tMeta.m_uTF) )
				continue;
			const uint32_t uRow = uWindow*E1_AND_WINDOW_ROWS+tMeta.m_uLocal;
			dTF[uRow] = uTF;
			dEligibility[uRow/64] |= uint64_t(1)<<(uRow%64);
		}
	}
	return true;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::PrepareE1OrWindows()
{
	if ( !m_bE1BestFirst )
		return true;
	std::array<std::array<uint64_t,64>,4> dTermBits {};
	for ( uint32_t uWindow=0; uWindow<=m_uE1LastWindow; ++uWindow )
	{
		std::array<uint64_t,64> dUnion {};
		float fUpperSum = 0.0f;
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			uint32_t uCardinality = 0, uMaxTF = 0;
			if ( !m_dNodes[i].m_pQword->GetE1DirectWindow ( uWindow, dTermBits[i].data(), nullptr, uCardinality, uMaxTF, m_uE1BoundReads ) )
				continue;
			++m_uE1ContainerOps;
			for ( int w=0; w<64; ++w ) dUnion[w] |= dTermBits[i][w];
			const float fIDF = Max ( 0.0f, m_dNodes[i].m_fIDF );
			if ( uMaxTF==UINT32_MAX ) fUpperSum += fIDF;
			else if ( uMaxTF ) { const float fTF=float(uMaxTF); fUpperSum += fTF/(fTF+0.3f)*fIDF; }
		}
		uint32_t uUnion = 0;
		for ( uint64_t uWord : dUnion ) uUnion += uint32_t(__builtin_popcountll(uWord));
		if ( !uUnion ) continue;
		E1OrWindowDesc_t & tDesc = m_dE1OrWindows.Add();
		tDesc = { uWindow, int(ceilf(1000.0f*(fUpperSum+0.5f)))+1, uUnion };
		++m_uE1WindowsTotal;
		m_uE1BooleanMatches += uUnion;
		m_uE1ExactUnionTotal += uUnion;
	}
	std::sort ( m_dE1OrWindows.Begin(), m_dE1OrWindows.End(), [] ( const E1OrWindowDesc_t & a, const E1OrWindowDesc_t & b ) {
		return a.m_iUpperWeight>b.m_iUpperWeight || ( a.m_iUpperWeight==b.m_iUpperWeight && a.m_uWindow<b.m_uWindow );
	} );
	return !m_dE1OrWindows.IsEmpty();
}


static uint64_t AdmitE1Or2BoundMasks ( uint64_t uCandidates, const uint64_t dTermBits[2], const uint64_t dClassMasks[2][2], const uint8_t dClassBounds[2][2], const uint32_t dClassCounts[2], const float dIDF[2], const int dCanonical[2], uint64_t uFirstRow, int iThreshold, uint64_t uWorstTiedRow, uint64_t & uRejectedRows, uint64_t & uTieRejectedRows, uint64_t & uRejectedClasses )
{
	uint64_t uAdmitted = 0;
	for ( uint32_t iClass0=0; iClass0<=dClassCounts[0]; ++iClass0 )
		for ( uint32_t iClass1=0; iClass1<=dClassCounts[1]; ++iClass1 )
		{
			const uint64_t uMask0 = iClass0<dClassCounts[0] ? dClassMasks[0][iClass0] : ~dTermBits[0];
			const uint64_t uMask1 = iClass1<dClassCounts[1] ? dClassMasks[1][iClass1] : ~dTermBits[1];
			const uint64_t uRows = uCandidates & uMask0 & uMask1;
			if ( !uRows )
				continue;

			const uint8_t dBounds[2] = { iClass0<dClassCounts[0] ? dClassBounds[0][iClass0] : uint8_t(0), iClass1<dClassCounts[1] ? dClassBounds[1][iClass1] : uint8_t(0) };
			float fUpper = 0.0f;
			for ( int iCanonical=0; iCanonical<2; ++iCanonical )
			{
				const int iNode = dCanonical[iCanonical];
				const float fTerm = dBounds[iNode]==255 ? E1SafeUpperSaturatedTerm(dIDF[iNode]) : E1SafeUpperTerm(dBounds[iNode],dIDF[iNode]);
				fUpper = E1SafeUpperAdd ( fUpper, fTerm );
			}
			const int iUpperWeight = E1SafeUpperWeight ( fUpper );
			if ( iThreshold<=0 || iUpperWeight>iThreshold )
			{
				uAdmitted |= uRows;
				continue;
			}
			if ( iUpperWeight<iThreshold || uWorstTiedRow<=uFirstRow )
			{
				const uint64_t uCount = uint64_t(__builtin_popcountll(uRows));
				uRejectedRows += uCount;
				uTieRejectedRows += iUpperWeight==iThreshold ? uCount : 0;
				++uRejectedClasses;
				continue;
			}
			const uint32_t uWinningRows = uint32_t ( Min ( uint64_t(64), uWorstTiedRow-uFirstRow ) );
			const uint64_t uWinningMask = uWinningRows==64 ? ~uint64_t(0) : ( uint64_t(1)<<uWinningRows )-1;
			const uint64_t uWinning = uRows & uWinningMask;
			const uint64_t uRejected = uRows & ~uWinningMask;
			uAdmitted |= uWinning;
			const uint64_t uRejectedCount = uint64_t(__builtin_popcountll(uRejected));
			uRejectedRows += uRejectedCount;
			uTieRejectedRows += uRejectedCount;
			if ( !uWinning )
				++uRejectedClasses;
		}
	return uAdmitted;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::FillE1OrWindow()
{
#if defined(MANTICORE_TEST)
	if ( g_bE1TestLastWindow )
		++g_uE1TestDirectExecutorCalls;
#endif
	m_dE1Pending.Resize(0);
	m_iE1PendingPos = 0;
	if ( m_bFirstChunk )
	{
		if ( !m_iNodesSet || m_uE1LastWindow==UINT32_MAX )
			return false;
		m_uE1Window = 0;
		m_bFirstChunk = false;
		if ( !PrepareE1OrWindows() )
			return false;
	}

	while ( m_bE1BestFirst ? m_iE1OrWindowPos<m_dE1OrWindows.GetLength() : m_uE1Window<=m_uE1LastWindow )
	{
		const E1OrWindowDesc_t * pDesc = m_bE1BestFirst ? &m_dE1OrWindows[m_iE1OrWindowPos++] : nullptr;
		const uint32_t uWindow = pDesc ? pDesc->m_uWindow : m_uE1Window++;
		if ( pDesc )
		{
			if ( m_iE1RankThreshold>0 && pDesc->m_iUpperWeight < m_iE1RankThreshold )
			{
				const int iSkipped = m_dE1OrWindows.GetLength()-m_iE1OrWindowPos+1;
				m_uE1BestFirstSkipped += iSkipped;
				m_uE1WindowsPruned += iSkipped;
				for ( int i=m_iE1OrWindowPos-1; i<m_dE1OrWindows.GetLength(); ++i )
					m_uE1SkippedMatches += m_dE1OrWindows[i].m_uUnion;
				m_iE1OrWindowPos = m_dE1OrWindows.GetLength();
				return true;
			}
			++m_uE1BestFirstVisited;
		}
		std::array<std::array<uint64_t,64>,4> dTermBits {};
		std::array<std::array<uint8_t,E1_AND_WINDOW_ROWS>,4> dOrdinalBounds;
		if ( !m_bE1BatchedOr2 )
			for ( auto & dBounds : dOrdinalBounds )
				dBounds.fill(0);
		std::array<uint64_t,64> dUnion {}, dCandidates {};
		std::array<uint32_t,4> dMaxTF {}, dCount {};
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			uint32_t uCardinality = 0, uMaxTF = 0;
			if ( !m_dNodes[i].m_pQword->GetE1DirectWindow ( uWindow, dTermBits[i].data(), m_bE1BatchedOr2 ? nullptr : dOrdinalBounds[i].data(), uCardinality, uMaxTF, m_uE1BoundReads ) )
				continue;
			if ( m_bE1ScopedOr2 )
			{
				const uint64_t uWordBase = uint64_t(uWindow)*64;
				uCardinality = 0;
				for ( int w=0; w<64; ++w )
				{
					dTermBits[i][w] = m_dE1ScopedAndEligibility[i][int(uWordBase+w)];
					uCardinality += uint32_t(__builtin_popcountll(dTermBits[i][w]));
				}
			}
			++m_uE1ContainerOps;
			dCount[i] = uCardinality;
			dMaxTF[i] = uMaxTF;
			for ( int w=0; w<64; ++w )
				dUnion[w] |= dTermBits[i][w];
		}

		uint64_t uUnion = 0;
		for ( uint64_t uWord : dUnion )
			uUnion += uint64_t(__builtin_popcountll(uWord));
		if ( !uUnion )
			continue;
		if ( m_tE1Filter.m_bEnabled )
		{
			m_uE1FilterCandidatesBefore += uUnion;
			++m_uE1FilterMasksBuilt;
			uUnion = E1ApplyFilterMask ( m_tE1Filter, uWindow, dUnion.data(), m_uE1FilterRowsExamined );
			m_uE1FilterCandidatesAfter += uUnion;
			if ( !uUnion )
				continue;
		}
		if ( !m_bE1BestFirst )
		{
			++m_uE1WindowsTotal;
			m_uE1BooleanMatches += uUnion;
			m_uE1ExactUnionTotal += uUnion;
		}

		std::array<float,4> dUpper {};
		float fUpperSum = 0.0f;
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			const float fPositiveIDF = Max ( 0.0f, m_dNodes[i].m_fIDF );
			if ( dMaxTF[i]==UINT32_MAX )
				dUpper[i] = E1SafeUpperSaturatedTerm ( fPositiveIDF );
			else
				dUpper[i] = E1SafeUpperTerm ( dMaxTF[i], fPositiveIDF );
			fUpperSum = E1SafeUpperAdd ( fUpperSum, dUpper[i] );
		}
		const int iUpperWeight = E1SafeUpperWeight ( fUpperSum );
		if ( m_iE1RankThreshold>0 && iUpperWeight < m_iE1RankThreshold )
		{
			++m_uE1WindowsPruned;
			m_uE1SkippedMatches += uUnion;
			return true;
		}
		++m_uE1WindowsScored;

		std::array<int,4> dOrder {};
		for ( int i=0; i<m_dNodes.GetLength(); ++i ) dOrder[i]=i;
		std::sort ( dOrder.begin(), dOrder.begin()+m_dNodes.GetLength(), [&] ( int a, int b ) {
			return dUpper[a]/float(dCount[a]+1) < dUpper[b]/float(dCount[b]+1);
		} );
		std::array<bool,4> dEssential {};
		for ( int i=0; i<m_dNodes.GetLength(); ++i ) dEssential[i]=true;
		float fNonessential = 0.0f;
		int iNonessential = 0;
		if ( m_iE1RankThreshold>0 )
			for ( int i=0; i<m_dNodes.GetLength()-1; ++i )
			{
				const int iNode = dOrder[i];
				const float fNext = E1SafeUpperAdd ( fNonessential, dUpper[iNode] );
				if ( E1SafeUpperWeight(fNext) >= m_iE1RankThreshold )
					break;
				dEssential[iNode]=false;
				fNonessential=fNext;
				++iNonessential;
			}
		++m_uE1EssentialRepartitions;
		m_uE1EssentialTerms += m_dNodes.GetLength()-iNonessential;
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
			if ( dEssential[i] )
				for ( int w=0; w<64; ++w ) { dCandidates[w] |= dTermBits[i][w]; ++m_uE1BitmapOps; }
		if ( m_tE1Filter.m_bEnabled )
			for ( int w=0; w<64; ++w ) dCandidates[w] &= dUnion[w];

		std::array<uint64_t,64> dAdmitted {};
		if ( m_bE1UnboundedCompat )
		{
			for ( int w=0; w<64; ++w )
			{
				dAdmitted[w] = dCandidates[w];
				const uint64_t uAdmitted = uint64_t(__builtin_popcountll(dAdmitted[w]));
				m_uE1CandidatesGenerated += uAdmitted;
				m_uE1CandidateMaskRows += uAdmitted;
			}
		}
		else if ( m_bE1BatchedOr2 )
		{
			const float dIDF[2] = { Max ( 0.0f, m_dNodes[0].m_fIDF ), Max ( 0.0f, m_dNodes[1].m_fIDF ) };
			const int dCanonical[2] = { m_dE1Canonical[0], m_dE1Canonical[1] };
			for ( int w=0; w<64; ++w )
			{
				const uint64_t uCandidates = dCandidates[w];
				if ( !uCandidates )
					continue;
				++m_uE1CandidateWordsExamined;
				const uint64_t uCandidateCount = uint64_t(__builtin_popcountll(uCandidates));
				m_uE1CandidatesGenerated += uCandidateCount;
				uint64_t dClassMasks[2][2] {};
				uint8_t dClassBounds[2][2] {};
				uint32_t dClassCounts[2] {};
				for ( int iNode=0; iNode<2; ++iNode )
				{
					if ( dCount[iNode] && !m_dNodes[iNode].m_pQword->GetE1DirectBoundWord ( uWindow, uint32_t(w), dClassMasks[iNode], dClassBounds[iNode], dClassCounts[iNode] ) )
						return false;
					if ( !dEssential[iNode] )
					{
						m_uE1NonessentialProbes += uCandidateCount;
						m_uE1NonessentialHits += uint64_t(__builtin_popcountll(uCandidates & dTermBits[iNode][w]));
					}
				}
				const uint64_t dWordTermBits[2] = { dTermBits[0][w], dTermBits[1][w] };
				dAdmitted[w] = AdmitE1Or2BoundMasks ( uCandidates, dWordTermBits, dClassMasks, dClassBounds, dClassCounts, dIDF, dCanonical, uint64_t(uWindow)*E1_AND_WINDOW_ROWS+uint64_t(w)*64, m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow), m_uE1CandidateBoundRejects, m_uE1TieBoundRejects, m_uE1BoundClassesRejected );
				const uint64_t uAdmitted = uint64_t(__builtin_popcountll(dAdmitted[w]));
				m_uE1CandidateMaskRows += uAdmitted;
				if ( !uAdmitted )
					++m_uE1WholeWordsRejected;
			}
		}
		else
			for ( int w=0; w<64; ++w )
			{
				uint64_t uBits = dCandidates[w];
				while ( uBits )
				{
					const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
					uBits &= uBits-1;
					const uint32_t uLocal = uint32_t(w)*64+uBit;
					const RowID_t tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
					++m_uE1CandidatesGenerated;

					float fCandidateBound = 0.0f;
					for ( int iCanonical=0; iCanonical<m_dE1Canonical.GetLength(); ++iCanonical )
					{
						const int iNode = m_dE1Canonical[iCanonical];
						if ( !dEssential[iNode] )
							++m_uE1NonessentialProbes;
						const uint8_t uBound = dOrdinalBounds[iNode][uLocal];
						if ( !uBound )
							continue;
						if ( !dEssential[iNode] )
							++m_uE1NonessentialHits;
						const float fIDF = Max ( 0.0f, m_dNodes[iNode].m_fIDF );
						const float fTerm = uBound==255 ? E1SafeUpperSaturatedTerm(fIDF) : E1SafeUpperTerm(uBound,fIDF);
						fCandidateBound = E1SafeUpperAdd ( fCandidateBound, fTerm );
					}
					const int iBoundWeight = E1SafeUpperWeight ( fCandidateBound );
					if ( E1TieAwareBoundReject ( iBoundWeight, uint64_t(tRowID), m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow) ) )
					{
						++m_uE1CandidateBoundRejects;
						if ( iBoundWeight==m_iE1RankThreshold )
							++m_uE1TieBoundRejects;
						continue;
					}
					dAdmitted[w] |= uint64_t(1)<<uBit;
					++m_uE1CandidateMaskRows;
				}
			}

		std::array<std::array<uint32_t,E1_AND_WINDOW_ROWS>,4> dExactTF {};
		if ( m_bE1BatchedOr2 )
			for ( int iNode=0; iNode<m_dNodes.GetLength(); ++iNode )
			{
				std::array<uint64_t,64> dSelected {};
				bool bAny = false;
				for ( int w=0; w<64; ++w )
				{
					dSelected[w] = dAdmitted[w] & dTermBits[iNode][w];
					bAny |= dSelected[w]!=0;
				}
				if ( !bAny )
					continue;
				if ( m_bE1ScopedOr2 )
				{
					for ( int w=0; w<64; ++w )
					{
						uint64_t uBits = dSelected[w];
						while ( uBits )
						{
							const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
							uBits &= uBits-1;
							const uint32_t uLocal = uint32_t(w)*64+uBit;
							dExactTF[iNode][uLocal] = m_dE1ScopedAndTF[iNode][uWindow*E1_AND_WINDOW_ROWS+uLocal];
							++m_uE1ScopedAndTFRows;
						}
					}
					continue;
				}
				uint64_t uRequested = 0, uWritten = 0, uDecoded = 0;
				if ( !m_dNodes[iNode].m_pQword->ExtractE1DirectTFBatch ( uWindow, dSelected.data(), dExactTF[iNode].data(), uRequested, uWritten, uDecoded ) || uRequested!=uWritten )
					return false;
				m_uE1BatchTFRowsRequested += uRequested;
				m_uE1BatchTFRowsWritten += uWritten;
				m_uE1BatchMetadataGroups += uDecoded;
				m_uE1MetadataGroups += uDecoded;
				m_uE1TFProbes += uWritten;
			}
		else
			for ( int w=0; w<64; ++w )
			{
				uint64_t uBits = dAdmitted[w];
				while ( uBits )
				{
					const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
					uBits &= uBits-1;
					const uint32_t uLocal = uint32_t(w)*64+uBit;
					const RowID_t tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
					for ( int iNode=0; iNode<m_dNodes.GetLength(); ++iNode )
					{
						if ( !dOrdinalBounds[iNode][uLocal] )
							continue;
						if ( dOrdinalBounds[iNode][uLocal]==1 )
							dExactTF[iNode][uLocal] = 1;
						else if ( !m_dNodes[iNode].m_pQword->ProbeE1DirectTF(tRowID, dExactTF[iNode][uLocal]) )
							return false;
						else
							++m_uE1RandomTFProbes;
						++m_uE1TFProbes;
					}
				}
			}

		for ( int w=0; w<64; ++w )
		{
			uint64_t uBits = dAdmitted[w];
			while ( uBits )
			{
				const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
				uBits &= uBits-1;
				const uint32_t uLocal = uint32_t(w)*64+uBit;
				const RowID_t tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
				m_uE1ScalarCandidateInspections += m_bE1BatchedOr2;
				uint32_t dCanonicalTF[4] = {};
				float fCandidateUpper = 0.0f;
				for ( int iCanonical=0; iCanonical<m_dE1Canonical.GetLength(); ++iCanonical )
				{
					const int iNode = m_dE1Canonical[iCanonical];
					dCanonicalTF[iCanonical] = dExactTF[iNode][uLocal];
					fCandidateUpper = E1SafeUpperAdd ( fCandidateUpper, E1SafeUpperTerm ( dCanonicalTF[iCanonical], Max ( 0.0f, m_dNodes[iNode].m_fIDF ) ) );
				}
				const int iCandidateUpper = E1SafeUpperWeight ( fCandidateUpper );
				if ( E1TieAwareBoundReject ( iCandidateUpper, uint64_t(tRowID), m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow) ) )
				{
					++m_uE1CandidateBoundRejects;
					if ( iCandidateUpper==m_iE1RankThreshold )
						++m_uE1TieBoundRejects;
					continue;
				}
				ExtDoc_t & tDoc = m_dE1Pending.Add();
				tDoc.m_tRowID = tRowID;
				tDoc.m_uDocFields = UINT32_MAX;
				tDoc.m_fTFIDF = 0.0f;
				tDoc.m_uExactTF = 0;
				tDoc.m_uExactTerms = BYTE(m_dNodes.GetLength());
				tDoc.m_bExactOr = 1;
				for ( int i=0; i<m_dNodes.GetLength(); ++i ) tDoc.m_dExactTF[i]=dCanonicalTF[i];
				++m_uE1CandidatesScored;
			}
		}
		for ( auto & tNode : m_dNodes )
			m_uE1MetadataGroups += tNode.m_pQword->TakeE1MetadataGroupsDecoded();
		m_uE1SkippedMatches += uUnion-m_dE1Pending.GetLength();
		return true;
	}
	return false;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::FillE1DirectAndWindow()
{
#if defined(MANTICORE_TEST)
	if ( g_bE1TestLastWindow )
		++g_uE1TestDirectExecutorCalls;
#endif
	m_dE1Pending.Resize(0);
	m_iE1PendingPos = 0;
	if ( m_bFirstChunk )
	{
		m_uE1Window = 0;
		m_bFirstChunk = false;
	}

	while ( m_uE1Window<=m_uE1LastWindow )
	{
		const uint32_t uWindow = m_uE1Window++;
		std::array<uint64_t,64> dIntersection {};
		std::array<uint64_t,64> dStageSurvivors {};
		float dIDF[4] {};
		int dCanonical[4] {};
		for ( int i=0; i<m_dNodes.GetLength(); ++i )
		{
			dIDF[i] = Max ( 0.0f, m_dNodes[i].m_fIDF );
			dCanonical[i] = m_dE1Canonical[i];
		}
		bool bAllTerms = true;
		uint64_t uIntersection = 0;
		if ( m_bE1FusedAnd4 )
		{
			uint32_t dCardinality[4] {};
			for ( int iFetch=0; iFetch<4; ++iFetch )
			{
				const int iNode = m_dE1DirectOrder[iFetch];
				if ( !m_dNodes[iNode].m_pQword->GetE1DirectWindowClasses ( uWindow, m_dE1AndMasks.Begin()+iNode*64, m_dE1AndBoundWords.Begin()+iNode*64, dCardinality[iNode], m_uE1BoundReads ) )
				{
					bAllTerms = false;
					++m_uE1MissingWindowShortCircuits;
					break;
				}
				++m_uE1MasksFetched;
				++m_uE1ContainerOps;
			}
			++m_uE1IntersectionWindows;
			if ( !bAllTerms )
				continue;
			int dOrder[4] { 0,1,2,3 };
			std::sort ( dOrder, dOrder+4, [&dCardinality] ( int a, int b ) { return dCardinality[a]<dCardinality[b]; } );
			for ( int w=0; w<64; ++w )
			{
				++m_uE1FusedWords;
				uint64_t uCandidates = m_dE1AndMasks[dOrder[0]*64+w];
				for ( int i=1; i<4 && uCandidates; ++i )
				{
					uCandidates &= m_dE1AndMasks[dOrder[i]*64+w];
					++m_uE1IntersectionMaskOps;
				}
				dIntersection[w] = uCandidates;
				if ( !uCandidates )
					continue;
				++m_uE1IntersectionNonemptyWords;
				++m_uE1CandidateWordsExamined;
				const uint64_t uCandidateCount = uint64_t(__builtin_popcountll(uCandidates));
				uIntersection += uCandidateCount;
				m_uE1CandidatesGenerated += uCandidateCount;
				if ( m_iE1RankThreshold<=0 )
					dStageSurvivors[w] = uCandidates;
				else
				{
					uint64_t dClassMasks[4][2] {};
					uint8_t dClassBounds[4][2] {};
					uint32_t dClassCounts[4] {};
					for ( int iNode=0; iNode<4; ++iNode )
					{
						const auto & tWord = m_dE1AndBoundWords[iNode*64+w];
						dClassMasks[iNode][0] = tWord.m_dMasks[0];
						dClassMasks[iNode][1] = tWord.m_dMasks[1];
						dClassBounds[iNode][0] = tWord.m_dBounds[0];
						dClassBounds[iNode][1] = tWord.m_dBounds[1];
						dClassCounts[iNode] = tWord.m_uClasses;
					}
					dStageSurvivors[w] = E1AdmitAndBoundMasks ( uCandidates, dClassMasks, dClassBounds, dClassCounts, dIDF, dCanonical, 4, uint64_t(uWindow)*E1_AND_WINDOW_ROWS+uint64_t(w)*64, m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow), m_uE1CandidateBoundRejects, m_uE1TieBoundRejects, m_uE1BoundClassesRejected, &m_uE1ClassCombinations );
				}
				const uint64_t uSurvivors = uint64_t(__builtin_popcountll(dStageSurvivors[w]));
				m_uE1CoarseRejects += uCandidateCount-uSurvivors;
				m_uE1CoarseSurvivors += uSurvivors;
				m_uE1CandidateMaskRows += uSurvivors;
				if ( !uSurvivors )
					++m_uE1WholeWordsRejected;
			}
		}
		else
		{
			std::array<std::array<uint64_t,64>,4> dTermBits {};
			dIntersection.fill ( ~uint64_t(0) );
			for ( int i=0; i<m_dNodes.GetLength(); ++i )
			{
				uint32_t uCardinality = 0, uMaxTF = 0;
				if ( !m_dNodes[i].m_pQword->GetE1DirectWindow ( uWindow, dTermBits[i].data(), nullptr, uCardinality, uMaxTF, m_uE1BoundReads ) )
				{
					bAllTerms = false;
					break;
				}
				if ( m_bE1ScopedAnd2 )
				{
					const uint64_t uWordBase = uint64_t(uWindow)*64;
					uCardinality = 0;
					for ( int w=0; w<64; ++w )
					{
						dTermBits[i][w] = m_dE1ScopedAndEligibility[i][int(uWordBase+w)];
						uCardinality += uint32_t(__builtin_popcountll(dTermBits[i][w]));
					}
				}
				++m_uE1ContainerOps;
				for ( int w=0; w<64; ++w )
				{
					dIntersection[w] &= dTermBits[i][w];
					++m_uE1IntersectionMaskOps;
				}
			}
			++m_uE1IntersectionWindows;
			if ( !bAllTerms )
				continue;
			for ( uint64_t uWord : dIntersection )
				uIntersection += uint64_t(__builtin_popcountll(uWord));
			for ( int w=0; w<64; ++w )
			{
				const uint64_t uCandidates = dIntersection[w];
				if ( !uCandidates )
					continue;
				++m_uE1CandidateWordsExamined;
				m_uE1CandidatesGenerated += uint64_t(__builtin_popcountll(uCandidates));
				uint64_t dClassMasks[4][2] {};
				uint8_t dClassBounds[4][2] {};
				uint32_t dClassCounts[4] {};
				if ( m_bE1UnboundedCompat )
					dStageSurvivors[w] = uCandidates;
				else
				{
					for ( int iNode=0; iNode<m_dNodes.GetLength(); ++iNode )
						if ( !m_dNodes[iNode].m_pQword->GetE1DirectBoundWord ( uWindow, uint32_t(w), dClassMasks[iNode], dClassBounds[iNode], dClassCounts[iNode] ) )
							return false;
					dStageSurvivors[w] = E1AdmitAndBoundMasks ( uCandidates, dClassMasks, dClassBounds, dClassCounts, dIDF, dCanonical, m_dNodes.GetLength(), uint64_t(uWindow)*E1_AND_WINDOW_ROWS+uint64_t(w)*64, m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow), m_uE1CandidateBoundRejects, m_uE1TieBoundRejects, m_uE1BoundClassesRejected );
				}
				const uint64_t uSurvivors = uint64_t(__builtin_popcountll(dStageSurvivors[w]));
				m_uE1CandidateMaskRows += uSurvivors;
				if ( !uSurvivors )
					++m_uE1WholeWordsRejected;
			}
		}
		if ( !uIntersection )
			continue;
		++m_uE1WindowsTotal;
		m_uE1BooleanMatches += uIntersection;

		if ( m_bE1StagedAnd4 )
		{
			uint64_t uStage0 = 0;
			for ( uint64_t uWord : dStageSurvivors ) uStage0 += uint64_t(__builtin_popcountll(uWord));
			m_uE1Stage0Input += uStage0;
			uint32_t uExactMask = 0;
			for ( uint32_t uStage=0; uStage<4; ++uStage )
			{
				const int iNode = m_dE1TFProbeOrder[uStage];
				uint64_t uRequested = 0, uWritten = 0, uDecoded = 0;
				uint32_t * pExact = m_dE1StagedExactTF.Begin()+iNode*E1_AND_WINDOW_ROWS;
				if ( !m_dNodes[iNode].m_pQword->ExtractE1DirectTFBatch ( uWindow, dStageSurvivors.data(), pExact, uRequested, uWritten, uDecoded, uStage ) || uRequested!=uWritten )
					return false;
				m_dE1StageBatchRequested[uStage] += uRequested;
				m_dE1StageBatchWritten[uStage] += uWritten;
				m_dE1StageMetadataGroups[uStage] += uDecoded;
				m_uE1BatchTFRowsRequested += uRequested;
				m_uE1BatchTFRowsWritten += uWritten;
				m_uE1BatchMetadataGroups += uDecoded;
				m_uE1MetadataGroups += uDecoded;
				m_uE1TFProbes += uWritten;
				uExactMask |= uint32_t(1)<<iNode;
				uint64_t uRejected = 0, uSurvivors = 0;
				for ( int w=0; w<64; ++w )
				{
					uint64_t uBits = dStageSurvivors[w];
					while ( uBits )
					{
						const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
						uBits &= uBits-1;
						const uint32_t uLocal = uint32_t(w)*64+uBit;
						uint32_t dTF[4];
						uint8_t dBounds[4];
						for ( int i=0; i<4; ++i )
						{
							dTF[i] = m_dE1StagedExactTF[i*E1_AND_WINDOW_ROWS+uLocal];
							dBounds[i] = E1DirectBoundForBit ( m_dE1AndBoundWords[i*64+w], uBit );
						}
						const RowID_t tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
						const int iUpper = E1PartialUpperWeight ( dTF, uExactMask, dBounds, dIDF, dCanonical, 4 );
						if ( E1TieAwareBoundReject ( iUpper, uint64_t(tRowID), m_iE1RankThreshold, uint64_t(m_tE1WorstTiedRow) ) )
						{
							dStageSurvivors[w] &= ~( uint64_t(1)<<uBit );
							++uRejected;
							++m_uE1CandidateBoundRejects;
							if ( iUpper==m_iE1RankThreshold ) ++m_uE1TieBoundRejects;
						}
						else
							++uSurvivors;
					}
				}
				m_dE1StageRejects[uStage] += uRejected;
				m_dE1StageSurvivors[uStage] += uSurvivors;
			}

			for ( int w=0; w<64; ++w )
			{
				uint64_t uBits = dStageSurvivors[w];
				while ( uBits )
				{
					const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
					uBits &= uBits-1;
					const uint32_t uLocal = uint32_t(w)*64+uBit;
					ExtDoc_t & tDoc = m_dE1Pending.Add();
					tDoc.m_tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
					tDoc.m_uDocFields = UINT32_MAX;
					tDoc.m_fTFIDF = 0.0f;
					tDoc.m_uExactTF = 0;
					tDoc.m_uExactTerms = 4;
					tDoc.m_bExactOr = 0;
					for ( int iCanonical=0; iCanonical<4; ++iCanonical )
						tDoc.m_dExactTF[iCanonical] = m_dE1StagedExactTF[m_dE1Canonical[iCanonical]*E1_AND_WINDOW_ROWS+uLocal];
					++m_uE1ScalarCandidateInspections;
					++m_uE1CandidatesScored;
				}
			}
		}
		else
		{
			std::array<std::array<uint32_t,E1_AND_WINDOW_ROWS>,4> dExactTF {};
			for ( int iNode=0; iNode<m_dNodes.GetLength(); ++iNode )
			{
				if ( m_bE1ScopedAnd2 )
				{
					for ( int w=0; w<64; ++w )
					{
						uint64_t uBits = dStageSurvivors[w];
						while ( uBits )
						{
							const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
							uBits &= uBits-1;
							const uint32_t uLocal = uint32_t(w)*64+uBit;
							dExactTF[iNode][uLocal] = m_dE1ScopedAndTF[iNode][uWindow*E1_AND_WINDOW_ROWS+uLocal];
							++m_uE1ScopedAndTFRows;
						}
					}
					continue;
				}
				uint64_t uRequested = 0, uWritten = 0, uDecoded = 0;
				if ( !m_dNodes[iNode].m_pQword->ExtractE1DirectTFBatch ( uWindow, dStageSurvivors.data(), dExactTF[iNode].data(), uRequested, uWritten, uDecoded ) || uRequested!=uWritten )
					return false;
				m_uE1BatchTFRowsRequested += uRequested;
				m_uE1BatchTFRowsWritten += uWritten;
				m_uE1BatchMetadataGroups += uDecoded;
				m_uE1MetadataGroups += uDecoded;
				m_uE1TFProbes += uWritten;
			}

			for ( int w=0; w<64; ++w )
			{
				uint64_t uBits = dStageSurvivors[w];
				while ( uBits )
				{
					const uint32_t uBit = uint32_t(__builtin_ctzll(uBits));
					uBits &= uBits-1;
					const uint32_t uLocal = uint32_t(w)*64+uBit;
					ExtDoc_t & tDoc = m_dE1Pending.Add();
					tDoc.m_tRowID = RowID_t(uWindow*E1_AND_WINDOW_ROWS+uLocal);
					tDoc.m_uDocFields = UINT32_MAX;
					tDoc.m_fTFIDF = 0.0f;
					tDoc.m_uExactTF = 0;
					tDoc.m_uExactTerms = BYTE(m_dNodes.GetLength());
					tDoc.m_bExactOr = 0;
					for ( int iCanonical=0; iCanonical<m_dE1Canonical.GetLength(); ++iCanonical )
						tDoc.m_dExactTF[iCanonical] = dExactTF[m_dE1Canonical[iCanonical]][uLocal];
					++m_uE1ScalarCandidateInspections;
					++m_uE1CandidatesScored;
				}
			}
		}
		m_uE1SkippedMatches += uIntersection-m_dE1Pending.GetLength();
		if ( m_dE1Pending.IsEmpty() )
			++m_uE1WindowsPruned;
		else
			++m_uE1WindowsScored;
		return true;
	}
	return false;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::FillE1Window()
{
#if defined(MANTICORE_TEST)
	if ( g_bE1TestLastWindow )
		++g_uE1TestDirectExecutorCalls;
#endif
	m_dE1Pending.Resize(0);
	m_iE1PendingPos = 0;
	if ( m_bFirstChunk )
	{
		if ( m_iNodesSet!=m_dNodes.GetLength() || !m_dNodes[0].m_pQword->m_iDocs )
			return false;
		Advance(0);
		m_bFirstChunk = false;
	}
	if ( m_dNodes[0].m_tRowID==INVALID_ROWID || !AdvanceQwords() )
	{
		m_dNodes[0].m_tRowID = INVALID_ROWID;
		return false;
	}

	m_uE1Window = uint32_t(m_dNodes[0].m_tRowID) / E1_AND_WINDOW_ROWS;
	DWORD dMaxTF[4] = {};
	for (;;)
	{
		if ( m_dNodes[0].m_tRowID==INVALID_ROWID || uint32_t(m_dNodes[0].m_tRowID)/E1_AND_WINDOW_ROWS!=m_uE1Window )
			break;
		ExtDoc_t & tDoc = m_dE1Pending.Add();
		tDoc.m_tRowID = m_dNodes[0].m_tRowID;
		tDoc.m_uDocFields = GetDocFieldsMask();
		tDoc.m_fTFIDF = 0.0f;
		tDoc.m_uExactTF = 0;
		tDoc.m_uExactTerms = BYTE(m_dNodes.GetLength());
		for ( int iCanonical=0; iCanonical<m_dE1Canonical.GetLength(); ++iCanonical )
		{
			const DWORD uTF = m_dNodes[m_dE1Canonical[iCanonical]].m_pQword->m_uMatchHits;
			tDoc.m_dExactTF[iCanonical] = uTF;
			dMaxTF[iCanonical] = Max ( dMaxTF[iCanonical], uTF );
			++m_uE1TFProbes;
		}
		++m_uE1BooleanMatches;
		Advance(0);
		if ( m_dNodes[0].m_tRowID==INVALID_ROWID || !AdvanceQwords() )
		{
			m_dNodes[0].m_tRowID = INVALID_ROWID;
			break;
		}
	}
	for ( auto & tNode : m_dNodes )
		m_uE1MetadataGroups += tNode.m_pQword->TakeE1MetadataGroupsDecoded();
	if ( m_dE1Pending.IsEmpty() )
		return false;

	++m_uE1WindowsTotal;
	float fUpperSum = 0.0f;
	for ( int iCanonical=0; iCanonical<m_dE1Canonical.GetLength(); ++iCanonical )
	{
		const auto & tNode = m_dNodes[m_dE1Canonical[iCanonical]];
		const float fPositiveIDF = Max ( 0.0f, tNode.m_fIDF );
		const float fTF = float(dMaxTF[iCanonical]);
		fUpperSum += fTF/(fTF+0.3f)*fPositiveIDF;
	}
	// Exact TF maxima are query-local to this 4096-row range.  ceil plus one
	// keeps the bound conservative across float evaluation and integer truncation;
	// equality is retained by the strict comparison.
	const int iUpperWeight = int(ceilf(1000.0f*(fUpperSum+0.5f)))+1;
	if ( m_iE1RankThreshold>0 && iUpperWeight < m_iE1RankThreshold )
	{
		++m_uE1WindowsPruned;
		m_uE1SkippedMatches += m_dE1Pending.GetLength();
		m_dE1Pending.Resize(0);
		return true;
	}
	++m_uE1WindowsScored;
	return true;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
DWORD ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetDocFieldsMask() const
{
	DWORD uMask = 0;
	for ( const auto & i : m_dNodes )
		uMask |= i.m_pQword->m_dQwordFields.GetMask32() & i.m_dQueriedFields.GetMask32();

	return uMask;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
float ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetTFIDF() const
{
	float fTFIDF = 0.0f;

	if constexpr ( USE_BM25 )
	{
		for ( const auto & i : m_dNodes )	
			fTFIDF += float(i.m_pQword->m_uMatchHits) / float(i.m_pQword->m_uMatchHits+SPH_BM25_K1) * i.m_fIDF;
	}

	return fTFIDF;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
RowID_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::Advance ( int iNode )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	do
	{
		tNode.m_tRowID = tNode.m_pQword->GetNextDoc().m_tRowID;
	}
	while ( tNode.m_tRowID!=INVALID_ROWID && !tNode.FitsFields() );

	return tNode.m_tRowID;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
RowID_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::Advance ( int iNode, RowID_t tRowID )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	if ( tRowID==tNode.m_tRowID )
		return tRowID;

	tNode.m_tRowID = tNode.m_pQword->AdvanceTo ( tRowID );
	while ( tNode.m_tRowID!=INVALID_ROWID )
	{
		if constexpr ( ROWID_LIMITS )
		{
			// don't check left boundary as we already enforced it when we advanced node #0
			if ( tNode.m_tRowID > m_tBoundaries.m_tMaxRowID )
			{
				tNode.m_tRowID = INVALID_ROWID;
				break;
			}
		}

		if ( tNode.FitsFields() )
			break;
		
		tNode.m_tRowID = tNode.m_pQword->GetNextDoc().m_tRowID;
	}

	return tNode.m_tRowID;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::AdvanceQwords()
{
	RowID_t tMaxRowID = m_dNodes[0].m_tRowID;
	for ( int i=1; i < m_dNodes.GetLength(); i++ )
	{
		NodeInfo_t & tCurNode = m_dNodes[i];
		if ( tCurNode.m_tRowID==tMaxRowID )
			continue;
		
		Advance ( i, tMaxRowID );
		
		if ( tCurNode.m_tRowID==INVALID_ROWID )
			return false;
		else if ( tCurNode.m_tRowID>tMaxRowID )
		{
			if ( Advance ( 0, tCurNode.m_tRowID )==INVALID_ROWID )
				return false;

			tMaxRowID = m_dNodes[0].m_tRowID;
			i = 0;
		}
	}

	return true;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
const ExtDoc_t * ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetDocsChunk()
{
	// since we're working directly with qwords, we need to check all those things here and not in ExtTerm
	// max_query_time
	if ( TimeExceeded () )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		// interrupt by sitgerm
		if ( g_bInterruptNow )
		{
			if ( m_pWarning )
				*m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	if ( m_bE1Ranked )
	{
		while ( m_iE1PendingPos>=m_dE1Pending.GetLength() )
		{
			if ( !( m_bE1Or ? FillE1OrWindow() : ( m_bE1DirectAnd ? FillE1DirectAndWindow() : FillE1Window() ) ) )
				return nullptr;
			// A safely pruned window has no rows to expose, but its Boolean
			// cardinality is returned through TakeRankSkippedDocs().
			if ( m_dE1Pending.IsEmpty() )
				continue;
		}
		int iDoc = 0;
		while ( iDoc<MAX_BLOCK_DOCS-1 && m_iE1PendingPos<m_dE1Pending.GetLength() )
			m_dDocs[iDoc++] = m_dE1Pending[m_iE1PendingPos++];
		if ( m_pStats )
			m_pStats->m_iFetchedDocs += iDoc;
		return ReturnDocsChunk ( iDoc, m_bE1Or ? "e1-multior" : "e1-multiand" );
	}

	if ( m_bFirstChunk )
	{
		if ( m_iNodesSet!=m_dNodes.GetLength() || !m_dNodes[0].m_pQword->m_iDocs )
			return nullptr;

		if constexpr ( ROWID_LIMITS )
			Advance ( 0, m_tBoundaries.m_tMinRowID );
		else
			Advance(0);

		m_bFirstChunk = false;
	}

	StoredMultiHit_t * pStoredHit = nullptr;
	StoredMultiHit_t * pFirstHit = nullptr;
	if ( m_bCollectHits )
	{
		int iLength = m_dStoredHits.GetLength();
		m_dStoredHits.Reserve ( iLength+MAX_BLOCK_DOCS );
		pStoredHit = m_dStoredHits.End();
		pFirstHit = pStoredHit-iLength;	// hack to get to m_pData
	}

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( m_dNodes[0].m_tRowID==INVALID_ROWID )
			break;

		if ( !AdvanceQwords() )
		{
			m_dNodes[0].m_tRowID=INVALID_ROWID;
			break;
		}

		RowID_t tMatchedRowID = m_dNodes[0].m_tRowID;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_tRowID = tMatchedRowID;
		tDoc.m_uDocFields = GetDocFieldsMask();
		tDoc.m_fTFIDF = GetTFIDF();

		if ( m_bCollectHits )
		{
			pStoredHit->m_tRowID = tMatchedRowID;
			pStoredHit->m_dHitlistOffsets.Reset(m_dNodes.GetLength());
			ARRAY_FOREACH ( i, m_dNodes )
				pStoredHit->m_dHitlistOffsets[i] = m_dNodes[i].m_pQword->m_iHitlistPos;

			pStoredHit++;
		}

		// we assume that the 1st node returns the least docs
		Advance(0);
	}

	if ( m_bCollectHits )
		m_dStoredHits.Resize ( pStoredHit-pFirstHit );

	if (m_pStats)
		m_pStats->m_iFetchedDocs += iDoc;

	return ReturnDocsChunk ( iDoc, "multiand" );
}


static inline bool IsHitLess ( Hitpos_t uHitposL, WORD uQueryPosL, Hitpos_t uHitposR, WORD uQueryPosR )
{
	return uHitposL<uHitposR || ( uHitposL==uHitposR && uQueryPosL<=uQueryPosR );
}


struct HitWithQpos_t
{
	int			m_iNode;
	Hitpos_t	m_uHit;
	WORD		m_uQueryPos;

	HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos )
		: m_iNode ( iNode )
		, m_uHit ( uHit )
		, m_uQueryPos ( uQueryPos )
	{}

	
	bool operator < ( const HitWithQpos_t & rhs ) const
	{
		return IsHitLess ( rhs.m_uHit, rhs.m_uQueryPos, m_uHit, m_uQueryPos );
	}
};

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::InitHitMerge ( HitInfo_t & tHitInfo, int iNode, const StoredMultiHit_t & tStoredHit )
{
	const NodeInfo_t & tNode = m_dNodes[iNode];

	tHitInfo.m_pQword = tNode.m_pQword;
	assert ( tHitInfo.m_pQword );
	tHitInfo.m_uNodePos = tNode.m_uNodepos;
	tHitInfo.m_uQueryPos = (WORD)tNode.m_iAtomPos;
	tHitInfo.m_pQword->SeekHitlist ( tStoredHit.m_dHitlistOffsets[iNode] );
	tHitInfo.m_uHitpos = tHitInfo.m_pQword->GetNextHit();
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::AddHit ( RowID_t tRowID, HitInfo_t & tHit, int iNode )
{
	if constexpr(TEST_FIELDS)
	{
		if ( m_dNodes[iNode].m_dQueriedFields.Test ( HITMAN::GetField ( tHit.m_uHitpos ) ) )
			m_dHits.Add ( ExtHit_t { tRowID, tHit.m_uHitpos, tHit.m_uQueryPos, tHit.m_uNodePos, 1, 1, 1, 0 } );
	}
	else
		m_dHits.Add ( ExtHit_t { tRowID, tHit.m_uHitpos, tHit.m_uQueryPos, tHit.m_uNodePos, 1, 1, 1, 0 } );

	tHit.m_uHitpos = tHit.m_pQword->GetNextHit();
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::DoHitMerge ( RowID_t tRowID, HitInfo_t & tLeft, HitInfo_t & tRight )
{
	while ( tLeft.m_uHitpos!=EMPTY_HIT && tRight.m_uHitpos!=EMPTY_HIT )
	{		
		if ( IsHitLess ( tLeft, tRight ) )
			AddHit ( tRowID, tLeft, 0 );
		else
			AddHit ( tRowID, tRight, 1 );
	}
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::DoHitMerge ( RowID_t tRowID, HitInfo_t & tHit1, HitInfo_t & tHit2, HitInfo_t & tHit3 )
{
	while ( tHit1.m_uHitpos!=EMPTY_HIT && tHit2.m_uHitpos!=EMPTY_HIT && tHit3.m_uHitpos!=EMPTY_HIT )
	{
		if ( IsHitLess ( tHit1, tHit2 ) && IsHitLess ( tHit1, tHit3 ) )
			AddHit ( tRowID, tHit1, 0 );
		else if ( IsHitLess ( tHit2, tHit1 ) && IsHitLess ( tHit2, tHit3 ) )
			AddHit ( tRowID, tHit2, 1 );
		else
			AddHit ( tRowID, tHit3, 2 );
	}

	if ( tHit1.m_uHitpos==EMPTY_HIT )
		DoHitMerge ( tRowID, tHit2, tHit3 );
	else if ( tHit2.m_uHitpos==EMPTY_HIT )
		DoHitMerge ( tRowID, tHit1, tHit3 );
	else
		DoHitMerge ( tRowID, tHit1, tHit2 );
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::CopyHits ( RowID_t tRowID, HitInfo_t & tHitInfo, int iNode )
{
	while ( tHitInfo.m_uHitpos!=EMPTY_HIT )
	{
		if constexpr (TEST_FIELDS)
		{
			if ( m_dNodes[iNode].m_dQueriedFields.Test ( HITMAN::GetField ( tHitInfo.m_uHitpos ) ) )
				m_dHits.Add ( ExtHit_t { tRowID, tHitInfo.m_uHitpos, tHitInfo.m_uQueryPos, tHitInfo.m_uNodePos, 1, 1, 1, 0 } );
		}
		else
			m_dHits.Add ( ExtHit_t { tRowID, tHitInfo.m_uHitpos, tHitInfo.m_uQueryPos, tHitInfo.m_uNodePos, 1, 1, 1, 0 } );

		tHitInfo.m_uHitpos = tHitInfo.m_pQword->GetNextHit();
	}
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::MergeHits2 ( const StoredMultiHit_t & tStoredHit )
{
	const int NUM_STREAMS = 2;
	HitInfo_t dHits[NUM_STREAMS];
	RowID_t tRowID = tStoredHit.m_tRowID;

	for ( int i = 0; i < NUM_STREAMS; i++ )
		InitHitMerge ( dHits[i], i, tStoredHit );

	DoHitMerge ( tRowID, dHits[0], dHits[1] );

	for ( int i = 0; i < NUM_STREAMS; i++ )
		CopyHits ( tRowID, dHits[i], i );
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::MergeHits3 ( const StoredMultiHit_t & tStoredHit )
{
	const int NUM_STREAMS = 3;
	HitInfo_t dHits[NUM_STREAMS];
	RowID_t tRowID = tStoredHit.m_tRowID;

	for ( int i = 0; i < NUM_STREAMS; i++ )
		InitHitMerge ( dHits[i], i, tStoredHit );

	DoHitMerge ( tRowID, dHits[0], dHits[1], dHits[2] );

	for ( int i = 0; i < NUM_STREAMS; i++ )
		CopyHits ( tRowID, dHits[i], i );
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::PushNextHit ( int iNode )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	while ( !tNode.m_bHitsOver )
	{
		Hitpos_t uHit = tNode.m_pQword->GetNextHit();
		if ( uHit==EMPTY_HIT )
			tNode.m_bHitsOver = true;
		else if ( tNode.m_dQueriedFields.Test ( HITMAN::GetField(uHit) ) )
		{
			m_tQueue.Push ( HitWithQpos_t ( iNode, uHit, (WORD)tNode.m_iAtomPos ) );
			break;
		}
	}
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::MergeHitsN ( const StoredMultiHit_t & tStoredHit )
{
	// setup hitlist reader
	ARRAY_FOREACH ( i, m_dNodes )
	{
		m_dNodes[i].m_pQword->SeekHitlist ( tStoredHit.m_dHitlistOffsets[i] );
		m_dNodes[i].m_bHitsOver = false;
	}

	// merge hitlists from all nodes for a given rowid
	assert ( !m_tQueue.GetLength() );
	ARRAY_FOREACH ( i, m_dNodes )
		PushNextHit(i);

	while ( m_tQueue.GetLength() )
	{
		const HitWithQpos_t & tHitWithQpos = m_tQueue.Root();
		int iNode = tHitWithQpos.m_iNode;
		NodeInfo_t & tNode = m_dNodes[iNode];
		ExtHit_t & tHit = m_dHits.Add();
		tHit.m_tRowID = tStoredHit.m_tRowID;
		tHit.m_uHitpos = tHitWithQpos.m_uHit;
		tHit.m_uQuerypos = tHitWithQpos.m_uQueryPos; // assume less that 64K words per query
		tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
		tHit.m_uNodepos = tNode.m_uNodepos;

		m_tQueue.Pop();

		PushNextHit(iNode);
	}
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	StoredMultiHit_t * pStoredHit = m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		switch ( m_dNodes.GetLength() )
		{
		case 2:		MergeHits2 ( *pStoredHit );	break;
		case 3:		MergeHits3 ( *pStoredHit );	break;
		default:	MergeHitsN ( *pStoredHit );	break;
		}
	}

	int nHits = m_dHits.GetLength();
	if ( m_pStats )
		m_pStats->m_iFetchedHits += nHits;

	// look at ExtTerm_T for more info on this code
	int nProcessed = int ( pStoredHit-m_dStoredHits.Begin() );
	m_dStoredHits.Pop();	// end marker
	m_dStoredHits.Remove ( 0, nProcessed );

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::Reset ( const ISphQwordSetup & tSetup )
{
	m_bFirstChunk = true;
	m_iNodesSet = 0;
	m_uE1Window = UINT32_MAX;
	m_dE1Pending.Resize(0);
	m_iE1PendingPos = 0;
	SetMaxTimeout ( tSetup.m_iMaxTimer );
	for ( auto & i : m_dNodes )
	{
		i.m_tRowID = INVALID_ROWID;
		i.m_bHitsOver = false;
		i.m_pQword->Reset ();
		// need to track active nodes for every segment
		// however AND requires all nodes that is why can use fast reject
		if ( tSetup.QwordSetup ( i.m_pQword ) )
			m_iNodesSet++;
	}
	
	m_dStoredHits.Resize(0);
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
int ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetQword ( NodeInfo_t & tNode, ExtQwordsHash_t & hQwords )
{
	tNode.m_fIDF = 0.0f;

	ExtQword_t * pQword = hQwords ( tNode.m_pQword->m_sWord );
	if ( !tNode.m_bNotWeighted && pQword && !pQword->m_bExcluded )
		pQword->m_iQueryPos = Min ( pQword->m_iQueryPos, tNode.m_pQword->m_iAtomPos );

	if ( tNode.m_bNotWeighted || pQword )
		return tNode.m_pQword->m_bExcluded ? -1 : tNode.m_pQword->m_iAtomPos;

	tNode.m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = tNode.m_pQword->m_sWord;
	tInfo.m_sDictWord = tNode.m_pQword->m_sDictWord;
	tInfo.m_iDocs = tNode.m_pQword->m_iDocs;
	tInfo.m_iHits = tNode.m_pQword->m_iHits;
	tInfo.m_iQueryPos = tNode.m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	tInfo.m_fBoost = tNode.m_pQword->m_fBoost;
	tInfo.m_bExpanded = tNode.m_pQword->m_bExpanded;
	tInfo.m_bExcluded = tNode.m_pQword->m_bExcluded;
	hQwords.Add ( tInfo, tNode.m_pQword->m_sWord );
	return tNode.m_pQword->m_bExcluded ? -1 : tNode.m_pQword->m_iAtomPos;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
int ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	for ( auto & i : m_dNodes )
	{
		int iRes = GetQword ( i, hQwords );
		iMax = Max ( iRes, iMax );
	}

	return iMax;
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	for ( auto & i : m_dNodes )
		if ( i.m_fIDF<0.0f )
		{
			assert ( hQwords ( i.m_pQword->m_sWord ) );
			i.m_fIDF = hQwords ( i.m_pQword->m_sWord )->m_fIDF;
		}
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	for ( const auto & i : m_dNodes )
		if ( i.m_bNotWeighted || !i.m_pQword->m_bExcluded )
		{
			ExtQword_t & tQword = hQwords[i.m_pQword->m_sWord];

			TermPos_t & tPos = dTermDupes.Add();
			tPos.m_uAtomPos = (WORD)i.m_pQword->m_iAtomPos;
			tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
		}
}


template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
uint64_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetWordID() const
{
	ARRAY_FOREACH ( i, m_dNodes )
	{
		NodeInfo_t & tNode = m_dNodes[i];
		if ( tNode.m_pQword->m_uWordID ) 
			m_dWordIds[i] = tNode.m_pQword->m_uWordID;
		else
			m_dWordIds[i] = sphFNV64 ( tNode.m_pQword->m_sDictWord.cstr() );
	}

	return sphFNV64 ( m_dWordIds.Begin(), (int) m_dWordIds.GetLengthBytes() );
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
int ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::GetDocsCount() const
{
	if ( !m_dNodes.GetLength() || !m_dNodes[0].m_pQword->m_iDocs )
		return 0;

	return ExtNode_c::GetDocsCount();
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::HintRowID ( RowID_t tRowID )
{
	if ( !m_dNodes[0].m_pQword->m_iDocs )
		return;

	if constexpr ( ROWID_LIMITS )
		tRowID = Max ( tRowID, m_tBoundaries.m_tMinRowID );

	if ( m_bFirstChunk || ( m_dNodes[0].m_tRowID!=INVALID_ROWID && tRowID>m_dNodes[0].m_tRowID ) )
	{
		if ( m_bFirstChunk && m_iNodesSet!=m_dNodes.GetLength() )
			return;

		Advance ( 0, tRowID );
		m_bFirstChunk = false;
	}
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtMultiAnd\n" );

	DebugIndent ( iLevel+1 );
	for ( const auto & i : m_dNodes )
	{
		printf ( "%s at: %d ", i.m_pQword->m_sWord.cstr(), i.m_pQword->m_iAtomPos );

		if ( i.m_dQueriedFields.TestAll(true) )
			printf ( "(all)\n" );
		else
		{
			bool bFirst = true;
			printf ( "in: " );
			for ( int iField=0; iField<SPH_MAX_FIELDS; iField++ )
			{
				if ( i.m_dQueriedFields.Test ( iField ) )
				{
					if ( !bFirst )
						printf ( ", " );
					printf ( "%d", iField );
					bFirst = false;
				}
			}
			printf ( "\n" );
		}
	}	
}


static float CalcQwordReadCost ( ISphQword * pQword )
{
	assert(pQword);
	return float(pQword->m_iDocs)*COST_SCALE*55.0f;
}

template <bool USE_BM25,bool TEST_FIELDS,bool ROWID_LIMITS>
NodeEstimate_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS,ROWID_LIMITS>::Estimate ( int64_t iTotalDocs ) const
{
	float fRatio = 1.0f;
	float fCostLeft = 0.0f;
	ARRAY_FOREACH ( i, m_dNodes )
	{
		const auto & tNode = m_dNodes[i];
		assert(tNode.m_pQword);

		if (!i)
			fCostLeft = CalcQwordReadCost ( tNode.m_pQword );
		else
		{
			float fCostRight = CalcQwordReadCost ( tNode.m_pQword );
			NodeEstimate_t tEst1 = { fCostLeft, int64_t(fRatio*iTotalDocs), i };
			NodeEstimate_t tEst2 = { fCostRight, tNode.m_pQword->m_iDocs, 1 };
			fCostLeft = CalcFTIntersectCost ( tEst1, tEst2, iTotalDocs, MAX_BLOCK_DOCS, MAX_BLOCK_DOCS );
		}

		fRatio *= float(tNode.m_pQword->m_iDocs) / iTotalDocs;
	}

	return { fCostLeft, int64_t(fRatio*iTotalDocs), m_dNodes.GetLength() };
}

//////////////////////////////////////////////////////////////////////////

bool ExtAndZonespanned_c::IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const
{
	for ( auto iZone : m_dZones )
	{
		int iSpan1, iSpan2;
		if ( m_pZoneChecker->IsInZone ( iZone, pHit1, &iSpan1 )==SPH_ZONE_FOUND && m_pZoneChecker->IsInZone ( iZone, pHit2, &iSpan2 )==SPH_ZONE_FOUND )
		{
			assert ( iSpan1>=0 && iSpan2>=0 );
			if ( iSpan1==iSpan2 )
				return true;
		}
	}
	return false;
}

void ExtAndZonespanned_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);
	const WORD uNodePosL = m_uNodePosL;
	const WORD uNodePosR = m_uNodePosR;

	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		if ( pCurL->m_tRowID < pCurR->m_tRowID )
			pCurL++;
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
			pCurR++;
		else
		{
			if ( IsHitLess ( pCurL, pCurR ) )
			{
				if ( IsSameZonespan ( pCurL, pCurR ) )
				{
					m_dHits.Add ( *pCurL );
					if ( uNodePosL!=0 )
						m_dHits.Last().m_uNodepos = uNodePosL;
				}

				pCurL++;
			}
			else
			{
				if ( IsSameZonespan ( pCurL, pCurR ) )
				{
					m_dHits.Add ( *pCurR );
					if ( uNodePosR!=0 )
						m_dHits.Last().m_uNodepos = uNodePosR;	
				}

				pCurR++;
			}
		}
	}

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}


void ExtAndZonespanned_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAndZonespan", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtOr_c::ExtOr_c ( ExtNode_i * pLeft, ExtNode_i * pRight )
	: ExtTwofer_c ( pLeft, pRight )
{}

const ExtDoc_t * ExtOr_c::GetDocsChunk()
{
	int iDoc = 0;

	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !HasDocs(pDocL) )
		{
			pDocL = m_pLeft->GetDocsChunk();
			if ( !pDocL && TimeExceeded() )
				break;
		}

		if ( !HasDocs(pDocR) )
		{
			pDocR = m_pRight->GetDocsChunk();
			if ( !pDocR && TimeExceeded() )
				break;
		}

		if ( !HasDocs(pDocL) && !HasDocs(pDocR) )
			break;

		ExtDoc_t & tNewDoc = m_dDocs[iDoc];

		// merge lists while we can, copy tail while if we can not
		if ( HasDocs(pDocL) && HasDocs(pDocR) )
		{
			if ( pDocL->m_tRowID==pDocR->m_tRowID )
			{
				tNewDoc = *pDocL;
				tNewDoc.m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields; // not necessary
				tNewDoc.m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;
				pDocL++;
				pDocR++;
			}
			else if ( pDocL->m_tRowID<pDocR->m_tRowID )
				tNewDoc = *pDocL++;
			else
				tNewDoc = *pDocR++;
		}
		else if ( HasDocs(pDocL) )
			tNewDoc = *pDocL++;
		else
			tNewDoc = *pDocR++;

		iDoc++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "or" );
}


void ExtOr_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);

	// merge, while possible
	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		if ( pCurL->m_tRowID < pCurR->m_tRowID )
			m_dHits.Add ( *pCurL++ );
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
			m_dHits.Add ( *pCurR++ );
		else
		{
			if ( IsHitLess ( pCurL, pCurR ) )
				m_dHits.Add ( *pCurL++ );
			else
				m_dHits.Add ( *pCurR++ );
		}
	}

	while ( HasHits(pCurL) )
		m_dHits.Add ( *pCurL++ );

	while ( HasHits(pCurR) )
		m_dHits.Add ( *pCurR++ );
}


void ExtOr_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtOr", iLevel );
}


NodeEstimate_t ExtOr_c::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pLeft && m_pRight );

	auto tLeftEstimate = m_pLeft->Estimate(iTotalDocs);
	auto tRightEstimate = m_pRight->Estimate(iTotalDocs);

	float fIntersection = float(tLeftEstimate.m_iDocs)/iTotalDocs*float(tRightEstimate.m_iDocs)/iTotalDocs;
	int64_t iIntersectionDocs = int64_t(fIntersection*iTotalDocs);

	int64_t iResDocs = tLeftEstimate.m_iDocs+tRightEstimate.m_iDocs>=iIntersectionDocs ? tLeftEstimate.m_iDocs+tRightEstimate.m_iDocs-iIntersectionDocs : iIntersectionDocs;

	float fMergeCost = float(tLeftEstimate.m_iDocs + tRightEstimate.m_iDocs)*COST_SCALE*10.0f;
	return { tLeftEstimate.m_fCost + tRightEstimate.m_fCost + fMergeCost, iResDocs, tLeftEstimate.m_iTerms + tRightEstimate.m_iTerms };
}

//////////////////////////////////////////////////////////////////////////

ExtMaybe_c::ExtMaybe_c ( ExtNode_i * pLeft, ExtNode_i * pRight )
	: ExtOr_c ( pLeft, pRight )
{}

// returns documents from left subtree only
//
// each call returns only one document and rewinds docs in rhs to look for the
// same docID as in lhs
//
// we do this to return hits from rhs too which we need to affect match rank
const ExtDoc_t * ExtMaybe_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	bool bRightEmpty = false;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, m_pLeft.get() ) )
			break;

		if ( !bRightEmpty )
		{
			if ( !WarmupDocs ( pDocR, m_pRight.get() ) )
			{
				if ( TimeExceeded() )
					break;

				bRightEmpty = true;
			}
		}

		if ( !bRightEmpty )
		{
			if ( pDocL->m_tRowID==pDocR->m_tRowID )
			{
				m_dDocs[iDoc] = *pDocL;
				m_dDocs[iDoc].m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields;
				m_dDocs[iDoc].m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;
				iDoc++;
				pDocL++;
				pDocR++;
			}
			else if ( pDocL->m_tRowID<pDocR->m_tRowID )
				m_dDocs[iDoc++] = *pDocL++;
			else
				pDocR++;
		}
		else
			m_dDocs[iDoc++] = *pDocL++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "maybe" );
}


void ExtMaybe_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtMaybe", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtAndNot_c::ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
	: ExtTwofer_c ( pFirst, pSecond )
{}

const ExtDoc_t * ExtAndNot_c::GetDocsChunk()
{
	// if reject-list is over, simply pass through to accept-list
	if ( m_bPassthrough )
		return m_pLeft->GetDocsChunk();

	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, m_pLeft.get() ) )
			break;

		if ( !WarmupDocs ( pDocR, m_pRight.get() ) && TimeExceeded() )
			break;

		// if there's nothing to filter against, simply copy leftovers
		if ( !HasDocs(pDocR) )
		{
			while ( HasDocs(pDocL) && iDoc<MAX_BLOCK_DOCS-1 )
				m_dDocs[iDoc++] = *pDocL++;

			m_bPassthrough = !HasDocs(pDocL);
			break;
		}

		// perform filtering
		assert ( pDocL );
		assert ( pDocR );
		while (true)
		{
			assert ( iDoc<MAX_BLOCK_DOCS-1 );
			assert ( HasDocs(pDocL) && HasDocs(pDocR) );

			// copy accepted until min rejected id
			while ( pDocL->m_tRowID < pDocR->m_tRowID && iDoc<MAX_BLOCK_DOCS-1 )
				m_dDocs[iDoc++] = *pDocL++;

			if ( !HasDocs(pDocL) || iDoc==MAX_BLOCK_DOCS-1 )
				break;

			// skip rejected until min accepted id
			while ( pDocR->m_tRowID < pDocL->m_tRowID )
				pDocR++;

			if ( !HasDocs(pDocR) )
				break;

			// skip both while ids match
			while ( pDocL->m_tRowID==pDocR->m_tRowID && HasDocs(pDocL) )
			{
				pDocL++;
				pDocR++;
			}

			if ( !HasDocs(pDocL) || !HasDocs(pDocR) )
				break;
		}
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "andnot" );
}


void ExtAndNot_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pHit = m_pLeft->GetHits(pDocs);
	while ( HasHits(pHit) )
		m_dHits.Add ( *pHit++ );
}

void ExtAndNot_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bPassthrough = false;
	ExtTwofer_c::Reset ( tSetup );
}

void ExtAndNot_c::SetCollectHits()
{
	m_pLeft->SetCollectHits();
	// m_pRight always ignores hits
}

void ExtAndNot_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAndNot", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtNWay_c::ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup )
	: ExtNode_c { tSetup.m_iMaxTimer }
{
	assert ( dNodes.GetLength()>1 );
	m_iAtomPos = dNodes[0]->GetAtomPos();
}

ExtNWay_c::~ExtNWay_c ()
{
	SafeDelete ( m_pNode );
}

void ExtNWay_c::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();

	m_pNode->Reset ( tSetup );
	m_pDocs = nullptr;
	m_pHits = nullptr;
}

int ExtNWay_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	return m_pNode->GetQwords ( hQwords );
}

void ExtNWay_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

void ExtNWay_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	assert ( m_pNode );
	m_pNode->GetTerms ( hQwords, dTermDupes );
}


void ExtNWay_c::HintRowID ( RowID_t tRowID )
{
	m_pNode->HintRowID ( tRowID );
}


uint64_t ExtNWay_c::GetWordID() const
{
	assert ( m_pNode );
	return m_pNode->GetWordID();
}


inline void ExtNWay_c::ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup )
{
	assert ( m_pNode==NULL );
	WORD uLPos = dPositions[0];
	ExtNode_i * pCur = dNodes[uLPos++]; // ++ for zero-based to 1-based
	ExtAnd_c * pCurEx = NULL;
	DWORD uLeaves = dNodes.GetLength();
	WORD uRPos;
	for ( DWORD i=1; i<uLeaves; i++ )
	{
		uRPos = dPositions[i];
		pCur = pCurEx = new ExtAnd_c ( pCur, dNodes[uRPos++] ); // ++ for zero-based to 1-based
		pCurEx->SetNodePos ( uLPos, uRPos );
		uLPos = 0;
	}
	if ( pCurEx )
		pCurEx->SetQPosReverse();

	pCur->SetCollectHits();
	m_pNode = pCur;
}


//////////////////////////////////////////////////////////////////////////

template < class FSM >
ExtNWay_T<FSM>::ExtNWay_T ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: ExtNWay_c ( dNodes, tSetup )
	, FSM ( dNodes, tNode, tSetup )
{
	CSphVector<WORD> dPositions ( dNodes.GetLength() );
	ARRAY_FOREACH ( i, dPositions )
		dPositions[i] = (WORD) i;
	dPositions.Sort ( ExtNodeTFExt_fn ( dNodes ) );
	ConstructNode ( dNodes, dPositions, tSetup );
}


template < class FSM >
const ExtDoc_t * ExtNWay_T<FSM>::GetDocsChunk()
{
	if ( !WarmupDocs ( m_pDocs, m_pHits, m_pNode ) )
		return nullptr;

	const ExtDoc_t * pDoc = m_pDocs;
	const ExtHit_t * pHit = m_pHits;

	FSM::ResetFSM();

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		assert ( pHit->m_tRowID==pDoc->m_tRowID );

		FSM::ResetFSM();

		// iterate all hits for this doc
		while ( pHit->m_tRowID==pDoc->m_tRowID )
		{
			// emit document, if its new and acceptable
			if ( FSM::HitFSM ( pHit, m_dMyHits ) && ( !iDoc || pHit->m_tRowID!=m_dDocs[iDoc-1].m_tRowID ) )
			{
				m_dDocs[iDoc].m_tRowID = pHit->m_tRowID;
				m_dDocs[iDoc].m_uDocFields = 1<< ( HITMAN::GetField ( pHit->m_uHitpos ) ); // not necessary
				m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
				iDoc++;
			}

			pHit++;
		}

		pDoc++;

		if ( !WarmupDocs ( pDoc, pHit, m_pNode ) )
			break;
	}

	m_pDocs = pDoc;
	m_pHits = pHit;

	return ReturnDocsChunk ( iDoc, "nway" );
}


template < class FSM >
void ExtNWay_T<FSM>::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
}


template < class FSM >
void ExtNWay_T<FSM>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "%s\n", FSM::GetName() );
	m_pNode->DebugDump ( iLevel+1 );
}


//////////////////////////////////////////////////////////////////////////

static DWORD GetQposMask ( const CSphVector<ExtNode_i *> & dQwords )
{
	DWORD uQposMask = 0;
	for ( const ExtNode_i * pNode : dQwords )
	{
		int iQpos = pNode->GetAtomPos();
		// no more than 32 query terms could be checked, all other skipped
		if ( iQpos<0x1f )
			uQposMask |= ( 1 << iQpos );
	}

	return uQposMask;
}


FSMphrase_c::FSMphrase_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & , const ISphQwordSetup & tSetup )
	: m_dAtomPos ( dQwords.GetLength() )
{
	ARRAY_FOREACH ( i, dQwords )
		m_dAtomPos[i] = dQwords[i]->GetAtomPos();

	assert ( ( m_dAtomPos.Last()-m_dAtomPos[0]+1 )>0 );
	m_dQposDelta.Resize ( m_dAtomPos.Last()-m_dAtomPos[0]+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;
	for ( int i=1; i<(int)m_dAtomPos.GetLength(); i++ )
		m_dQposDelta [ dQwords[i-1]->GetAtomPos() - dQwords[0]->GetAtomPos() ] = dQwords[i]->GetAtomPos() - dQwords[i-1]->GetAtomPos();

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}

inline bool FSMphrase_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );

	// adding start state for start hit
	if ( pHit->m_uQuerypos==m_dAtomPos[0] )
	{
		State_t & tState = m_dStates.Add();
		tState.m_iTagQword = 0;
		tState.m_uExpHitposWithField = uHitposWithField + m_dQposDelta[0];
	}

	// updating states
	for ( int i=m_dStates.GetLength()-1; i>=0; i-- )
	{
		if ( m_dStates[i].m_uExpHitposWithField<uHitposWithField )
		{
			m_dStates.RemoveFast(i); // failed to match
			continue;
		}

		// get next state
		if ( m_dStates[i].m_uExpHitposWithField==uHitposWithField && m_dAtomPos [ m_dStates[i].m_iTagQword+1 ]==pHit->m_uQuerypos )
		{
			m_dStates[i].m_iTagQword++; // check for next elm in query
			m_dStates[i].m_uExpHitposWithField = uHitposWithField + m_dQposDelta [ pHit->m_uQuerypos - m_dAtomPos[0] ];
		}

		// checking if state successfully matched
		if ( m_dStates[i].m_iTagQword==m_dAtomPos.GetLength()-1 )
		{
			DWORD uSpanlen = m_dAtomPos.Last() - m_dAtomPos[0];

			ExtHit_t & tTarget = dHits.Add();
			tTarget.m_tRowID = pHit->m_tRowID;
			tTarget.m_uHitpos = uHitposWithField - uSpanlen;
			tTarget.m_uQuerypos = (WORD) m_dAtomPos[0];
			tTarget.m_uMatchlen = tTarget.m_uSpanlen = (WORD)( uSpanlen + 1 );
			tTarget.m_uWeight = m_dAtomPos.GetLength();
			tTarget.m_uQposMask = m_uQposMask;
			ResetFSM ();
			return true;
		}
	}

	return false;
}


inline void FSMphrase_c::ResetFSM()
{
	m_dStates.Resize(0);
}


//////////////////////////////////////////////////////////////////////////

FSMproximity_c::FSMproximity_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iMaxDistance ( tNode.m_iOpArg )
	, m_uWordsExpected ( dQwords.GetLength() )
{
	assert ( m_iMaxDistance>0 );
	m_uMinQpos = dQwords[0]->GetAtomPos();
	m_uQLen = dQwords.Last()->GetAtomPos() - m_uMinQpos;
	m_dProx.Resize ( m_uQLen+1 );
	m_dDeltas.Resize ( m_uQLen+1 );

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}


inline bool FSMproximity_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	// walk through the hitlist and update context
	int iQindex = pHit->m_uQuerypos - m_uMinQpos;
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );

	// check if the word is new
	if ( m_dProx[iQindex]==UINT_MAX )
		m_uWords++;

	// update the context
	m_dProx[iQindex] = uHitposWithField;

	// check if the incoming hit is out of bounds, or affects min pos
	if ( uHitposWithField>=m_uExpPos // out of expected bounds
		|| iQindex==m_iMinQindex ) // or simply affects min pos
	{
		m_iMinQindex = iQindex;
		int iMinPos = uHitposWithField - m_uQLen - m_iMaxDistance;

		ARRAY_FOREACH ( i, m_dProx )
			if ( m_dProx[i]!=UINT_MAX )
			{
				if ( (int)m_dProx[i]<=iMinPos )
				{
					m_dProx[i] = UINT_MAX;
					m_uWords--;
					continue;
				}
				if ( m_dProx[i]<uHitposWithField )
				{
					m_iMinQindex = i;
					uHitposWithField = m_dProx[i];
				}
			}

		m_uExpPos = m_dProx[m_iMinQindex] + m_uQLen + m_iMaxDistance;
	}

	// all words were found within given distance?
	if ( m_uWords!=m_uWordsExpected )
		return false;

	// compute phrase weight
	//
	// FIXME! should also account for proximity factor, which is in 1 to maxdistance range:
	// m_iMaxDistance - ( pHit->m_uHitpos - m_dProx[m_iMinQindex] - m_uQLen )
	DWORD uMax = 0;
	ARRAY_FOREACH ( i, m_dProx )
		if ( m_dProx[i]!=UINT_MAX )
		{
			m_dDeltas[i] = m_dProx[i] - i;
			uMax = Max ( uMax, m_dProx[i] );
		} else
			m_dDeltas[i] = INT_MAX;

	m_dDeltas.Sort ();

	DWORD uCurWeight = 0;
	DWORD uWeight = 0;
	int iLast = -INT_MAX;
	ARRAY_FOREACH_COND ( i, m_dDeltas, m_dDeltas[i]!=INT_MAX )
	{
		if ( m_dDeltas[i]==iLast )
			uCurWeight++;
		else
		{
			uWeight += uCurWeight ? ( 1+uCurWeight ) : 0;
			uCurWeight = 0;
		}
		iLast = m_dDeltas[i];
	}

	uWeight += uCurWeight ? ( 1+uCurWeight ) : 0;
	if ( !uWeight )
		uWeight = 1;

	// emit hit
	ExtHit_t & tTarget = dHits.Add();
	tTarget.m_tRowID = pHit->m_tRowID;
	tTarget.m_uHitpos = Hitpos_t ( m_dProx[m_iMinQindex] ); // !COMMIT strictly speaking this is creation from LCS not value
	tTarget.m_uQuerypos = (WORD) m_uMinQpos;
	tTarget.m_uSpanlen = tTarget.m_uMatchlen = (WORD)( uMax-m_dProx[m_iMinQindex]+1 );
	tTarget.m_uWeight = uWeight;
	tTarget.m_uQposMask = m_uQposMask;

	// remove current min, and force recompue
	m_dProx[m_iMinQindex] = UINT_MAX;
	m_iMinQindex = -1;
	m_uWords--;
	m_uExpPos = 0;
	return true;
}


inline void FSMproximity_c::ResetFSM()
{
	m_uExpPos = 0;
	m_uWords = 0;
	m_iMinQindex = -1;
	ARRAY_FOREACH ( i, m_dProx )
		m_dProx[i] = UINT_MAX;
}


//////////////////////////////////////////////////////////////////////////

FSMmultinear_c::FSMmultinear_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iNear ( tNode.m_iOpArg )
	, m_uWordsExpected ( dNodes.GetLength() )
	, m_bQposMask ( tSetup.m_bSetQposMask )
{
	if ( m_uWordsExpected==2 )
		m_bTwofer = true;
	else
	{
		m_dNpos.Reserve ( m_uWordsExpected );
		m_dRing.Resize ( m_uWordsExpected );
		m_bTwofer = false;
	}
	assert ( m_iNear>0 );
}

inline bool FSMmultinear_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	// walk through the hitlist and update context
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
	WORD uNpos = pHit->m_uNodepos;
	WORD uQpos = pHit->m_uQuerypos;

	// skip dupe hit (may be emitted by OR node, for example)
	if ( m_uLastP==uHitposWithField )
	{
		// lets choose leftmost (in query) from all dupes. 'a NEAR/2 a' case
		if ( m_bTwofer && uNpos<m_uFirstNpos )
		{
			m_uFirstQpos = uQpos;
			m_uFirstNpos = uNpos;
			return false;
		} else if ( !m_bTwofer && uNpos<m_dRing [ RingTail() ].m_uNodepos ) // 'a NEAR/2 a NEAR/2 a' case
		{
			WORD * p = const_cast<WORD *>( m_dNpos.BinarySearch ( uNpos ) );
			if ( !p )
			{
				p = const_cast<WORD *>( m_dNpos.BinarySearch ( m_dRing [ RingTail() ].m_uNodepos ) );
				*p = uNpos;
				m_dNpos.Sort();
				m_dRing [ RingTail() ].m_uNodepos = uNpos;
				m_dRing [ RingTail() ].m_uQuerypos = uQpos;
			}
			return false;
		} else if ( m_uPrelastP && m_uLastML < pHit->m_uMatchlen ) // check if the hit is subset of another one
		{
			// roll back pre-last to check agains this new hit.
			m_uLastML = m_uPrelastML;
			m_uLastSL = m_uPrelastSL;
			m_uFirstHit = m_uLastP = m_uPrelastP;
			m_uWeight = m_uWeight - m_uLastW + m_uPrelastW;
		} else
			return false;
	}

	// probably new chain
	if ( m_uLastP==0 || ( m_uLastP + m_uLastML + m_iNear )<=uHitposWithField )
	{
		m_uFirstHit = m_uLastP = uHitposWithField;
		m_uLastML = pHit->m_uMatchlen;
		m_uLastSL = pHit->m_uSpanlen;
		m_uWeight = m_uLastW = pHit->m_uWeight;
		m_uFirstQpos = uQpos;
		if ( m_bTwofer )
		{
			m_uFirstNpos = uNpos;
		} else
		{
			m_dNpos.Resize(1);
			m_dNpos[0] = uNpos;
			Add2Ring ( pHit );
		}
		return false;
	}

	// this hit (with such querypos) already was there. Skip the hit.
	if ( m_bTwofer )
	{
		// special case for twofer: hold the overlapping
		if ( ( m_uFirstHit + m_uLastML )>uHitposWithField
			&& ( m_uFirstHit + m_uLastML )<( uHitposWithField + pHit->m_uMatchlen )
			&& m_uLastML!=pHit->m_uMatchlen )
		{
			m_uFirstHit = m_uLastP = uHitposWithField;
			m_uLastML = pHit->m_uMatchlen;
			m_uLastSL = pHit->m_uSpanlen;
			m_uWeight = m_uLastW = pHit->m_uWeight;
			m_uFirstQpos = uQpos;
			m_uFirstNpos = uNpos;
			return false;
		}
		if ( uNpos==m_uFirstNpos )
		{
			if ( m_uLastP < uHitposWithField )
			{
				m_uPrelastML = m_uLastML;
				m_uPrelastSL = m_uLastSL;
				m_uPrelastP = m_uLastP;
				m_uPrelastW = pHit->m_uWeight;

				m_uFirstHit = m_uLastP = uHitposWithField;
				m_uLastML = pHit->m_uMatchlen;
				m_uLastSL = pHit->m_uSpanlen;
				m_uWeight = m_uLastW = m_uPrelastW;
				m_uFirstQpos = uQpos;
				m_uFirstNpos = uNpos;
			}
			return false;
		}
	} else
	{
		if ( uNpos < m_dNpos[0] )
		{
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
			m_dNpos.Insert ( 0, uNpos );
		} else if ( uNpos > m_dNpos.Last() )
		{
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
			m_dNpos.Add ( uNpos );
		} else if ( uNpos!=m_dNpos[0] && uNpos!=m_dNpos.Last() )
		{
			int iEnd = m_dNpos.GetLength();
			int iStart = 0;
			int iMid = -1;
			while ( iEnd-iStart>1 )
			{
				iMid = ( iStart + iEnd ) / 2;
				if ( uNpos==m_dNpos[iMid] )
				{
					const ExtHit_t& dHit = m_dRing[m_iRing];
					// last addition same as the first. So, we can shift
					if ( uNpos==dHit.m_uNodepos )
					{
						m_uWeight -= dHit.m_uWeight;
						m_uFirstHit = HITMAN::GetPosWithField ( dHit.m_uHitpos );
						ShiftRing();
					// last addition same as the first. So, we can shift
					} else if ( uNpos==m_dRing [ RingTail() ].m_uNodepos )
						m_uWeight -= m_dRing [ RingTail() ].m_uWeight;
					else
						return false;
				}

				if ( uNpos<m_dNpos[iMid] )
					iEnd = iMid;
				else
					iStart = iMid;
			}
			m_dNpos.Insert ( iEnd, uNpos );
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
		// last addition same as the first. So, we can shift
		} else if ( uNpos==m_dRing[m_iRing].m_uNodepos )
		{
			m_uWeight -= m_dRing[m_iRing].m_uWeight;
			m_uFirstHit = HITMAN::GetPosWithField ( m_dRing[m_iRing].m_uHitpos );
			ShiftRing();
		// last addition same as the tail. So, we can move the tail onto it.
		} else if ( uNpos==m_dRing [ RingTail() ].m_uNodepos )
			m_uWeight -= m_dRing [ RingTail() ].m_uWeight;
		else
			return false;
	}

	m_uWeight += pHit->m_uWeight;
	m_uLastML = pHit->m_uMatchlen;
	m_uLastSL = pHit->m_uSpanlen;
	Add2Ring ( pHit );

	// finally got the whole chain - emit it!
	// warning: we don't support overlapping in generic chains.
	if ( m_bTwofer || (int)m_uWordsExpected==m_dNpos.GetLength() )
	{
		ExtHit_t & tTarget = dHits.Add();
		tTarget.m_tRowID = pHit->m_tRowID;
		tTarget.m_uHitpos = Hitpos_t ( m_uFirstHit ); // !COMMIT strictly speaking this is creation from LCS not value
		tTarget.m_uMatchlen = (WORD)( uHitposWithField - m_uFirstHit + m_uLastML );
		tTarget.m_uWeight = m_uWeight;
		m_uPrelastP = 0;

		if ( m_bTwofer ) // for exactly 2 words allow overlapping - so, just shift the chain, not reset it
		{
			tTarget.m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			tTarget.m_uSpanlen = 2;
			tTarget.m_uQposMask = ( 1 << ( Max ( m_uFirstQpos, pHit->m_uQuerypos ) - tTarget.m_uQuerypos ) );
			m_uFirstHit = m_uLastP = uHitposWithField;
			m_uWeight = pHit->m_uWeight;
			m_uFirstQpos = pHit->m_uQuerypos;
		} else
		{
			tTarget.m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			tTarget.m_uSpanlen = (WORD) m_dNpos.GetLength();
			tTarget.m_uQposMask = 0;
			m_uLastP = 0;
			if ( m_bQposMask && tTarget.m_uSpanlen>1 )
			{
				int iNpos0 = m_dNpos[0];
				ARRAY_FOREACH ( i, m_dNpos )
				{
					int iQposDelta = ( m_dNpos[i] - iNpos0 ) + tTarget.m_uQuerypos;
					if ( iQposDelta<(int)sizeof(tTarget.m_uQposMask)*8 )
						tTarget.m_uQposMask |= ( 1u << iQposDelta );
				}
			}
		}
		return true;
	}

	m_uLastP = uHitposWithField;
	return false;
}


inline void FSMmultinear_c::ResetFSM()
{
	m_iRing = m_uLastP = m_uPrelastP = 0;
}


inline int FSMmultinear_c::RingTail() const
{
	return ( m_iRing + m_dNpos.GetLength() - 1 ) % m_uWordsExpected;
}


inline void FSMmultinear_c::Add2Ring ( const ExtHit_t* pHit )
{
	if ( !m_bTwofer )
		m_dRing [ RingTail() ] = *pHit;
}


inline void FSMmultinear_c::ShiftRing()
{
	if ( ++m_iRing==(int)m_uWordsExpected )
		m_iRing=0;
}


//////////////////////////////////////////////////////////////////////////

struct QuorumDupeNodeHash_t
{
	uint64_t m_uWordID;
	int m_iIndex;

	bool operator < ( const QuorumDupeNodeHash_t & b ) const
	{
		if ( m_uWordID==b.m_uWordID )
			return m_iIndex<b.m_iIndex;
		else
			return m_uWordID<b.m_uWordID;
	}
};

struct QuorumNodeAtomPos_fn
{
	inline bool IsLess ( const ExtQuorum_c::TermTuple_t & a, const ExtQuorum_c::TermTuple_t & b ) const
	{
		return a.m_pTerm->GetAtomPos() < b.m_pTerm->GetAtomPos();
	}
};


ExtQuorum_c::ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: ExtNode_c ( tSetup.m_iMaxTimer)
{
	assert ( tNode.GetOp()==SPH_QUERY_QUORUM );
	assert ( dQwords.GetLength()<MAX_HITS );

	m_iThresh = GetThreshold ( tNode, dQwords.GetLength() );
	m_iThresh = Max ( m_iThresh, 1 );
	m_bHasDupes = false;

	assert ( dQwords.GetLength()>1 ); // use TERM instead
	assert ( dQwords.GetLength()<=256 ); // internal masks are upto 256 bits
	assert ( m_iThresh>=1 ); // 1 is also OK; it's a bit different from just OR
	assert ( m_iThresh<dQwords.GetLength() ); // use AND instead

	if ( dQwords.GetLength()>0 )
	{
		m_iAtomPos = dQwords[0]->GetAtomPos();

		// compute duplicate keywords mask (aka dupe mask)
		// FIXME! will fail with wordforms and stuff; sorry, no wordforms vs expand vs quorum support for now!
		CSphFixedVector<QuorumDupeNodeHash_t> dHashes ( dQwords.GetLength() );
		ARRAY_FOREACH ( i, dQwords )
		{
			dHashes[i].m_uWordID = dQwords[i]->GetWordID();
			dHashes[i].m_iIndex = i;
		}
		sphSort ( dHashes.Begin(), dHashes.GetLength() );

		QuorumDupeNodeHash_t tParent = *dHashes.Begin();
		m_dInitialChildren.Add().m_pTerm = dQwords[tParent.m_iIndex];
		m_dInitialChildren.Last().m_iCount = 1;
		tParent.m_iIndex = 0;

		for ( int i=1; i<dHashes.GetLength(); i++ )
		{
			QuorumDupeNodeHash_t & tElem = dHashes[i];
			if ( tParent.m_uWordID!=tElem.m_uWordID )
			{
				tParent = tElem;
				tParent.m_iIndex = m_dInitialChildren.GetLength();
				m_dInitialChildren.Add().m_pTerm = dQwords [ tElem.m_iIndex ];
				m_dInitialChildren.Last().m_iCount = 1;
			} else
			{
				m_dInitialChildren[tParent.m_iIndex].m_iCount++;
				SafeDelete ( dQwords[tElem.m_iIndex] );
				m_bHasDupes = true;
			}
		}

		// sort back to qpos order
		m_dInitialChildren.Sort ( QuorumNodeAtomPos_fn() );
	}

	ARRAY_FOREACH ( i, m_dInitialChildren )
	{
		m_dInitialChildren[i].m_pCurDoc = NULL;
		m_dInitialChildren[i].m_pCurHit = NULL;
		m_dInitialChildren[i].m_pTerm->SetCollectHits();
	}

	m_dChildren = m_dInitialChildren;
}

ExtQuorum_c::~ExtQuorum_c ()
{
	ARRAY_FOREACH ( i, m_dInitialChildren )
		SafeDelete ( m_dInitialChildren[i].m_pTerm );
}

void ExtQuorum_c::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();

	m_dChildren = m_dInitialChildren;

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->Reset ( tSetup );
}

int ExtQuorum_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		int iKidMax = m_dChildren[i].m_pTerm->GetQwords ( hQwords );
		iMax = Max ( iMax, iKidMax );
	}
	return iMax;
}

void ExtQuorum_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->SetQwordsIDF ( hQwords );
}

void ExtQuorum_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtQuorum_c::GetWordID() const
{
	uint64_t uHash = SPH_FNV64_SEED;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		uint64_t uCur = m_dChildren[i].m_pTerm->GetWordID();
		uHash = sphFNV64 ( uCur, uHash );
	}

	return uHash;
}


void ExtQuorum_c::HintRowID ( RowID_t tRowID )
{
	for ( auto & i : m_dChildren )
		if ( i.m_pTerm )
			i.m_pTerm->HintRowID ( tRowID );
}


NodeEstimate_t ExtQuorum_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tEst;
	for ( auto & i : m_dChildren )
		if ( i.m_pTerm )
			tEst += i.m_pTerm->Estimate(iTotalDocs);

	return tEst;
}


void ExtQuorum_c::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	for ( auto & i : m_dChildren )
		if ( i.m_pTerm )
			i.m_pTerm->SetRowidBoundaries(tBoundaries);
}


const ExtDoc_t * ExtQuorum_c::GetDocsChunk()
{
	// warmup
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];
		if ( HasDocs(tElem.m_pCurDoc) )
			continue;

		tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
		if ( tElem.m_pCurDoc )
			tElem.m_pCurHit = tElem.m_pTerm->GetHits ( tElem.m_pCurDoc );
		else
		{
			m_dChildren.RemoveFast(i);
			i--;
		}
	}

	// main loop
	int iDoc = 0;
	int iQuorumLeft = CountQuorum ( true );
	while ( iDoc<MAX_BLOCK_DOCS-1 && iQuorumLeft>=m_iThresh )
	{
		// find min document ID, count occurrences
		ExtDoc_t tCand;

		tCand.m_tRowID = INVALID_ROWID; // current candidate id
		tCand.m_uDocFields = 0; // non necessary
		tCand.m_fTFIDF = 0.0f;

		int iQuorum = 0;
		for ( auto & tChild : m_dChildren )
		{
			assert ( HasDocs ( tChild.m_pCurDoc ) );
			if ( tChild.m_pCurDoc->m_tRowID < tCand.m_tRowID )
			{
				tCand = *tChild.m_pCurDoc;
				iQuorum = tChild.m_iCount;
			}
			else if ( tChild.m_pCurDoc->m_tRowID==tCand.m_tRowID )
			{
				tCand.m_uDocFields |= tChild.m_pCurDoc->m_uDocFields; // FIXME!!! check hits in case of dupes or field constrain
				tCand.m_fTFIDF += tChild.m_pCurDoc->m_fTFIDF;
				iQuorum += tChild.m_iCount;
			}
		}

		if ( iQuorum>=m_iThresh && CollectMatchingHits ( tCand.m_tRowID, m_iThresh ) )
			m_dDocs[iDoc++] = tCand;

		// advance children
		int iNumChildren = m_dChildren.GetLength();
		ARRAY_FOREACH ( i, m_dChildren )
		{
			TermTuple_t & tElem = m_dChildren[i];
			if ( tElem.m_pCurDoc->m_tRowID!=tCand.m_tRowID )
				continue;

			tElem.m_pCurDoc++;
			if ( HasDocs(tElem.m_pCurDoc) )
				continue;

			tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
			if ( tElem.m_pCurDoc )
				tElem.m_pCurHit = tElem.m_pTerm->GetHits ( tElem.m_pCurDoc );
			else
			{
				m_dChildren.RemoveFast ( i );
				i--;
			}
		}

		if ( iNumChildren!=m_dChildren.GetLength() )
			iQuorumLeft = CountQuorum ( false );
	}

	return ReturnDocsChunk ( iDoc, "quorum" );
}


struct QuorumCmpHitPos_fn
{
	inline bool IsLess ( const ExtHit_t & a, const ExtHit_t & b ) const
	{
		if ( a.m_tRowID==b.m_tRowID )
		{
			DWORD uHitPosA = HITMAN::GetPosWithField(a.m_uHitpos);
			DWORD uHitPosB = HITMAN::GetPosWithField(b.m_uHitpos);

			if ( uHitPosA==uHitPosB )
			{
				if ( a.m_uQuerypos==b.m_uQuerypos )
					return HITMAN::IsEnd ( a.m_uHitpos ) < HITMAN::IsEnd ( b.m_uHitpos );
				else
					return ( a.m_uQuerypos<b.m_uQuerypos );
			}

			return uHitPosA<uHitPosB;
		}

		return a.m_tRowID<b.m_tRowID;
	}
};


void ExtQuorum_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	m_dHits.Sort ( QuorumCmpHitPos_fn() );
}


int ExtQuorum_c::CountQuorum ( bool bFixDupes )
{
	if ( !m_bHasDupes )
		return m_dChildren.GetLength();

	int iSum = 0;
	bool bHasDupes = false;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		iSum += m_dChildren[i].m_iCount;
		bHasDupes |= ( m_dChildren[i].m_iCount>1 );
	}

#if QDEBUG
	if ( bFixDupes && bHasDupes!=m_bHasDupes )
		printf ( "quorum dupes %d -> %d\n", m_bHasDupes, bHasDupes );
#endif

	m_bHasDupes = bFixDupes ? bHasDupes : m_bHasDupes;
	return iSum;
}


int ExtQuorum_c::GetThreshold ( const XQNode_t & tNode, int iQwords )
{
	return ( tNode.m_bPercentOp ? (int)floor ( 1.0f / 100.0f * tNode.m_iOpArg * iQwords + 0.5f ) : tNode.m_iOpArg );
}

bool ExtQuorum_c::CollectMatchingHits ( RowID_t tRowID, int iThreshold )
{
	if ( !m_bHasDupes )
	{
		for ( auto & tChild : m_dChildren )
		{
			while ( tChild.m_pCurHit->m_tRowID < tRowID )
				tChild.m_pCurHit++;

			while ( tChild.m_pCurHit->m_tRowID==tRowID )
				m_dMyHits.Add ( *tChild.m_pCurHit++ );
		}

		return true;
	}

	int iOldHitLen = m_dMyHits.GetLength();
	int iQuorum = 0;
	for ( auto & tChild : m_dChildren )
	{
		const ExtHit_t * & pHit = tChild.m_pCurHit;
		if ( !HasHits(pHit) )
			continue;

		while ( pHit->m_tRowID<tRowID )
			pHit++;

		// collect matched hits but only up to quorum.count per-term
		for ( int iTermHits = 0; iTermHits<tChild.m_iCount && pHit->m_tRowID==tRowID; iTermHits++, iQuorum++ )
			m_dMyHits.Add ( *pHit++ );

		// got quorum - no need to check further
		if ( iQuorum>=iThreshold )
			break;
	}

	// discard collected hits in case of no quorum matched
	if ( iQuorum<iThreshold )
	{
		m_dMyHits.Resize ( iOldHitLen );
		return false;
	}

	// collect all hits to move docs/hits further
	for ( auto & tChild : m_dChildren )
	{
		while ( tChild.m_pCurHit->m_tRowID==tRowID )
			m_dMyHits.Add ( *tChild.m_pCurHit++ );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

ExtOrder_c::ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup )
	: ExtNode_c { tSetup.m_iMaxTimer }
	, m_dChildren ( dChildren )
	, m_bDone ( false )
{
	int iChildren = dChildren.GetLength();
	assert ( iChildren>=2 );

	m_dChildDoc.Resize ( iChildren );
	m_dChildHit.Resize ( iChildren );
	m_dChildDocsChunk.Resize ( iChildren );

	if ( dChildren.GetLength()>0 )
		m_iAtomPos = dChildren[0]->GetAtomPos();

	ARRAY_FOREACH ( i, dChildren )
	{
		assert ( m_dChildren[i] );
		m_dChildDoc[i] = nullptr;
		m_dChildHit[i] = nullptr;
		m_dChildren[i]->SetCollectHits();
	}
}


void ExtOrder_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bDone = false;
	m_dMyHits.Resize(0);

	m_dChildDoc.Fill(nullptr);
	m_dChildHit.Fill(nullptr);

	for ( auto pChild : m_dChildren )
	{
		assert(pChild);
		pChild->Reset(tSetup);
	}
}


ExtOrder_c::~ExtOrder_c()
{
	for ( auto & pChild : m_dChildren )
		SafeDelete ( pChild );
}


// rewinds all children hitlists to given row id
// returns the one with min hitpos
int ExtOrder_c::GetChildIdWithNextHit ( RowID_t tRowID )
{
	// OPTIMIZE! implement PQ instead of full-scan
	DWORD uMinPosWithField = UINT_MAX;
	int iChild = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		const ExtHit_t * & pHit = m_dChildHit[i];

		// skip until proper hit
		while ( pHit->m_tRowID<tRowID )
			pHit++;

		// is this our man at all?
		if ( pHit->m_tRowID==tRowID )
		{
			// is he the best we can get?
			if ( HITMAN::GetPosWithField ( pHit->m_uHitpos ) < uMinPosWithField )
			{
				uMinPosWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
				iChild = i;
			}
		}
	}
	return iChild;
}


bool ExtOrder_c::GetMatchingHits ( RowID_t tRowID )
{
	// my trackers
	CSphVector<ExtHit_t> dAccLongest;
	CSphVector<ExtHit_t> dAccRecent;
	int iPosLongest = 0; // needed to handle cases such as "a b c" << a
	int iPosRecent = 0;
	int iField = -1;

	dAccLongest.Reserve ( m_dChildren.GetLength() );
	dAccRecent.Reserve ( m_dChildren.GetLength() );

	int nOldHits = m_dMyHits.GetLength();

	while ( true )
	{
		// get next hit (in hitpos ascending order)
		int iChild = GetChildIdWithNextHit ( tRowID );
		if ( iChild<0 )
			break; // OPTIMIZE? no trailing hits on this route

		const ExtHit_t * & pHit = m_dChildHit[iChild];
		assert ( pHit->m_tRowID==tRowID );

		// most recent subseq must never be longer
		assert ( dAccRecent.GetLength()<=dAccLongest.GetLength() );

		// handle that hit!
		int iHitField = HITMAN::GetField ( pHit->m_uHitpos );
		int iHitPos = HITMAN::GetPos ( pHit->m_uHitpos );

		if ( iHitField!=iField )
		{
			// new field; reset both trackers
			dAccLongest.Resize ( 0 );
			dAccRecent.Resize ( 0 );

			// initial seeding, if needed
			if ( iChild==0 )
			{
				dAccLongest.Add ( *pHit );
				iPosLongest = iHitPos + pHit->m_uSpanlen;
				iField = iHitField;
			}

		} else if ( iChild==dAccLongest.GetLength() && iHitPos>=iPosLongest )
		{
			// it fits longest tracker
			dAccLongest.Add ( *pHit );
			iPosLongest = iHitPos + pHit->m_uSpanlen;

			// fully matched subsequence
			if ( dAccLongest.GetLength()==m_dChildren.GetLength() )
			{
				// flush longest tracker into buffer, and keep it terminated
				ARRAY_FOREACH ( i, dAccLongest )
					m_dMyHits.Add ( dAccLongest[i] );

				// reset both trackers
				dAccLongest.Resize ( 0 );
				dAccRecent.Resize ( 0 );
				iPosRecent = iPosLongest;
			}

		} else if ( iChild==0 )
		{
			// it restarts most-recent tracker
			dAccRecent.Resize ( 0 );
			dAccRecent.Add ( *pHit );
			iPosRecent = iHitPos + pHit->m_uSpanlen;
			if ( !dAccLongest.GetLength() )
			{
				dAccLongest.Add	( *pHit );
				iPosLongest = iHitPos + pHit->m_uSpanlen;
			}
		} else if ( iChild==dAccRecent.GetLength() && iHitPos>=iPosRecent )
		{
			// it fits most-recent tracker
			dAccRecent.Add ( *pHit );
			iPosRecent = iHitPos + pHit->m_uSpanlen;

			// maybe most-recent just became longest too?
			if ( dAccRecent.GetLength()==dAccLongest.GetLength() )
			{
				dAccLongest.SwapData ( dAccRecent );
				dAccRecent.Resize ( 0 );
				iPosLongest = iPosRecent;
			}
		}

		// advance hit stream
		pHit++;
	}

	return nOldHits!=m_dMyHits.GetLength();
}


const ExtDoc_t * ExtOrder_c::GetDocsChunk()
{
	if ( m_bDone )
		return nullptr;

	// warm up
	ARRAY_FOREACH ( i, m_dChildren )
	{
		if ( !m_dChildDoc[i] )
		{
			m_dChildDoc[i] = m_dChildDocsChunk[i] = m_dChildren[i]->GetDocsChunk();
			m_dChildHit[i] = nullptr;
		}

		if ( !m_dChildDoc[i] )
		{
			m_bDone = true;
			return nullptr;
		}
	}

	// match while there's enough space in buffer
	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		// find next candidate document (that has all the words)
		RowID_t tRowID = m_dChildDoc[0]->m_tRowID;
		assert ( tRowID!=INVALID_ROWID );

		int iChild = 1;
		while ( iChild < m_dChildren.GetLength() )
		{
			// skip docs with too small ids
			assert ( m_dChildDoc[iChild] );
			while ( m_dChildDoc[iChild]->m_tRowID < tRowID )
				m_dChildDoc[iChild]++;

			// block end? pull next block and keep scanning
			if ( !HasDocs ( m_dChildDoc[iChild] ) )
			{
				m_dChildDoc[iChild] = m_dChildDocsChunk[iChild] = m_dChildren[iChild]->GetDocsChunk();
				m_dChildHit[iChild] = nullptr;
				if ( !m_dChildDoc[iChild] )
				{
					m_bDone = true;
					return ReturnDocsChunk ( iDoc, "order" );
				}
				continue;
			}

			// too big id? its out next candidate
			if ( m_dChildDoc[iChild]->m_tRowID > tRowID )
			{
				tRowID = m_dChildDoc[iChild]->m_tRowID;
				iChild = 0;
				continue;
			}

			assert ( m_dChildDoc[iChild]->m_tRowID==tRowID );
			iChild++;
		}

		#ifndef NDEBUG
		assert ( tRowID!=INVALID_ROWID );
		for ( auto pChildDoc : m_dChildDoc )
		{
			assert ( pChildDoc );
			assert ( pChildDoc->m_tRowID==tRowID );
		}
		#endif

		// fetch hits
		ARRAY_FOREACH ( i, m_dChildren )
		{
			if ( !m_dChildHit[i] )
				m_dChildHit[i] = m_dChildren[i]->GetHits ( m_dChildDoc[i] );
		}

		// match and save hits
		if ( GetMatchingHits ( tRowID ) )
			m_dDocs[iDoc++] = *m_dChildDoc[0];

		// advance doc stream
		m_dChildDoc[0]++;
		if ( !HasDocs ( m_dChildDoc[0] ) )
		{
			m_dChildDoc[0] = m_dChildDocsChunk[0] = m_dChildren[0]->GetDocsChunk();
			m_dChildHit[0] = nullptr;
			if ( !m_dChildDoc[0] )
			{
				m_bDone = true;
				break;
			}
		}
	}

	return ReturnDocsChunk ( iDoc, "order" );
}


void ExtOrder_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}


int ExtOrder_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		int iKidMax = m_dChildren[i]->GetQwords ( hQwords );
		iMax = Max ( iMax, iKidMax );
	}
	return iMax;
}

void ExtOrder_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetQwordsIDF ( hQwords );
}

void ExtOrder_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtOrder_c::GetWordID () const
{
	uint64_t uHash = SPH_FNV64_SEED;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		uint64_t uCur = m_dChildren[i]->GetWordID();
		uHash = sphFNV64 ( uCur, uHash );
	}

	return uHash;
}


void ExtOrder_c::HintRowID ( RowID_t tRowID )
{
	for ( auto i : m_dChildren )
		i->HintRowID ( tRowID );
}


NodeEstimate_t ExtOrder_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tEst;
	for ( const auto & i : m_dChildren )
		tEst += i->Estimate(iTotalDocs);

	return tEst;
}


void ExtOrder_c::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	for ( auto & i : m_dChildren )
		i->SetRowidBoundaries(tBoundaries);
}

//////////////////////////////////////////////////////////////////////////

template<bool ROWID_LIMITS>
ExtUnit_T<ROWID_LIMITS>::ExtUnit_T ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t & uFields, const ISphQwordSetup & tSetup, const char * szUnit )
	: ExtNode_c { tSetup.m_iMaxTimer }
	, m_pArg1 ( pFirst )
	, m_pArg2 ( pSecond )
{
	XQKeyword_t tDot;
	tDot.m_sWord = szUnit;

	if ( tSetup.m_pStats )
		m_pDot = new ExtTerm_T<false,ROWID_LIMITS,true> ( CreateQueryWord ( tDot, tSetup ), uFields, tSetup, true );
	else
		m_pDot = new ExtTerm_T<false,ROWID_LIMITS,false> ( CreateQueryWord ( tDot, tSetup ), uFields, tSetup, true );

	m_pArg1->SetCollectHits();
	m_pArg2->SetCollectHits();
	m_pDot->SetCollectHits();
}

template<bool ROWID_LIMITS>
ExtUnit_T<ROWID_LIMITS>::~ExtUnit_T ()
{
	SafeDelete ( m_pArg1 );
	SafeDelete ( m_pArg2 );
	SafeDelete ( m_pDot );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::Reset ( const ISphQwordSetup & tSetup )
{
	m_pArg1->Reset ( tSetup );
	m_pArg2->Reset ( tSetup );
	m_pDot->Reset ( tSetup );

	m_pDocs1 = m_pDocs2 = m_pDotDocs = nullptr;
	m_pDoc1 = m_pDoc2 = m_pDotDoc = nullptr;
	m_bNeedDotHits = false;

	BufferedNode_c::Reset();
}

template<bool ROWID_LIMITS>
int ExtUnit_T<ROWID_LIMITS>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax1 = m_pArg1->GetQwords ( hQwords );
	int iMax2 = m_pArg2->GetQwords ( hQwords );
	return Max ( iMax1, iMax2 );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pArg1->SetQwordsIDF ( hQwords );
	m_pArg2->SetQwordsIDF ( hQwords );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	m_pArg1->GetTerms ( hQwords, dTermDupes );
	m_pArg2->GetTerms ( hQwords, dTermDupes );
}

template<bool ROWID_LIMITS>
uint64_t ExtUnit_T<ROWID_LIMITS>::GetWordID() const
{
	uint64_t dHash[2];
	dHash[0] = m_pArg1->GetWordID();
	dHash[1] = m_pArg2->GetWordID();
	return sphFNV64 ( dHash, sizeof(dHash) );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::HintRowID ( RowID_t tRowID )
{
	m_pArg1->HintRowID ( tRowID );
	m_pArg2->HintRowID ( tRowID );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtSentence\n" );
	m_pArg1->DebugDump ( iLevel+1 );
	m_pArg2->DebugDump ( iLevel+1 );
}

template<bool ROWID_LIMITS>
NodeEstimate_t ExtUnit_T<ROWID_LIMITS>::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pArg1 && m_pArg2 && m_pDot );

	NodeEstimate_t tRes;
	tRes += m_pArg1->Estimate(iTotalDocs);
	tRes += m_pArg2->Estimate(iTotalDocs);
	tRes += m_pDot->Estimate(iTotalDocs);

	return tRes;
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	m_pArg1->SetRowidBoundaries(tBoundaries);
	m_pArg2->SetRowidBoundaries(tBoundaries);
	m_pDot->SetRowidBoundaries(tBoundaries);
}

/// skips hits within current document while their position is less or equal than the given limit
/// returns true if a matching hit (with big enough position, and in current document) was found
/// returns false otherwise
static inline bool SkipHitsLtePos ( const ExtHit_t * & pHits, Hitpos_t uPos )
{
	assert ( pHits );
	RowID_t tRowID = pHits->m_tRowID;
	if ( tRowID==INVALID_ROWID )
		return false;

	while ( pHits->m_tRowID==tRowID && pHits->m_uHitpos<=uPos )
		pHits++;

	return pHits->m_tRowID==tRowID;
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::FilterHits ( const ExtDoc_t * pDoc1, const ExtDoc_t * pDoc2, const ExtHit_t * & pHit1, const ExtHit_t * & pHit2, const ExtHit_t * & pDotHit, DWORD uSentenceEnd, RowID_t tRowID, int & iDoc )
{
	bool bRegistered = false;
	while ( true )
	{
		if ( uSentenceEnd )
		{
			// we're in a matched sentence state
			// copy hits until next dot
			bool bValid1 = pHit1->m_tRowID==tRowID && pHit1->m_uHitpos<uSentenceEnd;
			bool bValid2 = pHit2->m_tRowID==tRowID && pHit2->m_uHitpos<uSentenceEnd;

			if ( !bValid1 && !bValid2 )
			{
				// no more hits in this sentence
				uSentenceEnd = 0;
				if ( pHit1->m_tRowID==tRowID && pHit2->m_tRowID==tRowID )
					continue; // no more in-sentence hits, but perhaps more sentences in this document
				else
					break; // document is over
			}

			// register document as matching
			if ( !bRegistered )
			{
				ExtDoc_t & tDoc = m_dDocs[iDoc++];
				tDoc.m_tRowID = pDoc1->m_tRowID;
				tDoc.m_uDocFields = pDoc1->m_uDocFields | pDoc2->m_uDocFields; // non necessary
				tDoc.m_fTFIDF = pDoc1->m_fTFIDF + pDoc2->m_fTFIDF;
				bRegistered = true; // just once
			}

			if ( bValid1 && ( !bValid2 || IsHitLess ( pHit1, pHit2 ) ) )
				m_dMyHits.Add ( *pHit1++ );
			else
				m_dMyHits.Add ( *pHit2++ );
		}
		else
		{
			// no sentence matched yet
			// let's check the next hit pair
			assert ( pHit1->m_tRowID==tRowID );
			assert ( pHit2->m_tRowID==tRowID );
			assert ( pDotHit->m_tRowID==tRowID );

			// our current hit pair locations
			DWORD uMin = Min ( pHit1->m_uHitpos, pHit2->m_uHitpos );
			DWORD uMax = Max ( pHit1->m_uHitpos, pHit2->m_uHitpos );

			// skip all dots beyond the min location
			if ( !SkipHitsLtePos ( pDotHit, uMin ) )
			{
				// we have a match!
				// moreover, no more dots past min location in current document
				// copy hits until next document
				uSentenceEnd = UINT_MAX;
				continue;
			}

			// does the first post-pair-start dot separate our hit pair?
			if ( pDotHit->m_uHitpos < uMax )
			{
				// yes, got an "A dot B" case
				// rewind candidate hits past this dot, break if current document is over
				if ( !SkipHitsLtePos ( pHit1, pDotHit->m_uHitpos ) )
					break;
				if ( !SkipHitsLtePos ( pHit2, pDotHit->m_uHitpos ) )
					break;

				continue;
			}
			else
			{
				// we have a match!
				// copy hits until next dot
				if ( !SkipHitsLtePos ( pDotHit, uMax ) )
					uSentenceEnd = UINT_MAX; // correction, no next dot, so make it "next document"
				else
					uSentenceEnd = pDotHit->m_uHitpos;

				assert ( uSentenceEnd );
			}
		}
	}
}

template<bool ROWID_LIMITS>
const ExtDoc_t * ExtUnit_T<ROWID_LIMITS>::GetDocsChunk()
{
	// SENTENCE operator is essentially AND on steroids
	// that also takes relative dot positions into account
	//
	// when document matches both args but not the dot, it degenerates into AND
	// we immediately lookup and copy matching document hits anyway, though
	// this is suboptimal (because these hits might never be required at all)
	// but this is expected to be rare case, so let's keep code simple
	//
	// when document matches both args and the dot, we need to filter the hits
	// only those left/right pairs that are not (!) separated by a dot should match

	int iDoc = 0;
	const ExtHit_t * pHit1 = m_pHit1;
	const ExtHit_t * pHit2 = m_pHit2;
	const ExtHit_t * pDotHit = m_pDotHit;
	const ExtDoc_t * pDoc1 = m_pDoc1;
	const ExtDoc_t * pDoc2 = m_pDoc2;
	const ExtDoc_t * pDotDoc = m_pDotDoc;

	bool bNeedDoc1Hits = false;
	bool bNeedDoc2Hits = false;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		// fetch more candidate docs, if needed
		if ( !HasDocs(pDoc1) )
		{
			if ( HasDocs(pDoc2)  )
				m_pArg1->HintRowID ( pDoc2->m_tRowID );

			pDoc1 = m_pArg1->GetDocsChunk();
			if ( !HasDocs(pDoc1) )
				break;

			bNeedDoc1Hits = true;
		}

		if ( !HasDocs(pDoc2) )
		{
			if ( HasDocs(pDoc1)  )
				m_pArg2->HintRowID ( pDoc1->m_tRowID );

			pDoc2 = m_pArg2->GetDocsChunk();
			if ( !HasDocs(pDoc2) )
				break;

			bNeedDoc2Hits = true;
		}

		// find next candidate match
		while ( pDoc1->m_tRowID!=pDoc2->m_tRowID && HasDocs(pDoc1) && HasDocs(pDoc2) )
		{
			while ( pDoc1->m_tRowID < pDoc2->m_tRowID && HasDocs(pDoc2) )
				pDoc1++;
			while ( pDoc1->m_tRowID > pDoc2->m_tRowID && HasDocs(pDoc1) )
				pDoc2++;
		}

		// got our candidate that matches AND?
		RowID_t tRowID = pDoc1->m_tRowID;
		if ( !HasDocs(pDoc1) || !HasDocs(pDoc2) )
			continue;

		// yes, now fetch more dots docs, if needed
		// note how NULL is accepted here, "A and B but no dots" case is valid!
		if ( !HasDocs(pDotDoc) )
		{
			m_pDot->HintRowID(tRowID);
			pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();
			m_bNeedDotHits = true;
		}

		// skip preceding docs
		while ( pDotDoc && pDotDoc->m_tRowID < tRowID )
		{
			while ( pDotDoc->m_tRowID < tRowID )
				pDotDoc++;

			if ( !HasDocs(pDotDoc) )
			{
				pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();
				m_bNeedDotHits = true;
			}
		}

		// we will need document hits on both routes below
		if ( bNeedDoc1Hits )
		{
			pHit1 = m_pArg1->GetHits(pDoc1);
			bNeedDoc1Hits = false;
		}

		while ( pHit1->m_tRowID < tRowID )
			pHit1++;

		if ( bNeedDoc2Hits )
		{
			pHit2 = m_pArg2->GetHits(pDoc2);
			bNeedDoc2Hits = false;
		}

		while ( pHit2->m_tRowID < tRowID )
			pHit2++;

		assert ( pHit1->m_tRowID==tRowID );
		assert ( pHit2->m_tRowID==tRowID );

		DWORD uSentenceEnd = 0;
		if ( !pDotDoc || pDotDoc->m_tRowID!=tRowID )
		{
			// no dots in current document?
			// just copy all hits until next document
			uSentenceEnd = UINT_MAX;

		} else
		{
			// got both hits and dots
			// rewind to relevant dots hits, then do sentence boundary detection
			if ( m_bNeedDotHits )
			{
				pDotHit = m_pDot->GetHits ( pDotDoc );
				m_bNeedDotHits = false;
			}

			while ( pDotHit->m_tRowID < tRowID )
				pDotHit++;
		}

		// do those hits
		FilterHits ( pDoc1, pDoc2, pHit1, pHit2, pDotHit, uSentenceEnd, tRowID, iDoc );

		// all hits copied; do the next candidate
		pDoc1++;
		pDoc2++;
	}

	m_pDoc1 = pDoc1;
	m_pDoc2 = pDoc2;
	m_pDotDoc = pDotDoc;
	m_pHit1 = pHit1;
	m_pHit2 = pHit2;
	m_pDotHit = pDotHit;

	return ReturnDocsChunk ( iDoc, "unit" );
}

template<bool ROWID_LIMITS>
void ExtUnit_T<ROWID_LIMITS>::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}

//////////////////////////////////////////////////////////////////////////

ExtNotNear_c::ExtNotNear_c ( ExtNode_i * pMust, ExtNode_i * pNot, int iDist )
	: ExtTwofer_c ( pMust, pNot )
	, m_iDist ( iDist )
{
	m_sNodeName.SetSprintf ( "NOTNEAR/%d", m_iDist );

	// need hits from both nodes
	pMust->SetCollectHits();
	pNot->SetCollectHits();
}


void ExtNotNear_c::Reset ( const ISphQwordSetup & tSetup )
{
	ExtTwofer_c::Reset ( tSetup );
	BufferedNode_c::Reset();
	m_pHitL = nullptr;
	m_pHitR = nullptr;
}


void ExtNotNear_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtNotNear_c", iLevel );
}

bool ExtNotNear_c::FilterHits ( RowID_t tRowID, const ExtHit_t * & pMust, const ExtHit_t * & pNot )
{
	assert ( pMust && pNot && HasHits ( pMust ) && HasHits ( pNot ) );

	const int iWasHits = m_dMyHits.GetLength();
	
	while ( pMust->m_tRowID==tRowID )
	{
		DWORD uMustField = HITMAN::GetField ( pMust->m_uHitpos );
		DWORD uMustPos = HITMAN::GetPosWithField ( pMust->m_uHitpos ); 
		
		// pNot sliding window start pos
		//( notEnd + dist ) < mustStart means Not very far left and safe to move
		while ( pNot->m_tRowID==tRowID )
		{
			DWORD uNotField = HITMAN::GetField ( pNot->m_uHitpos );
			if ( uNotField<uMustField ) 
			{
				pNot++;
				continue;
			}
			
			if ( uNotField>uMustField )
				break;

			DWORD uNotPos = HITMAN::GetPosWithField ( pNot->m_uHitpos );
			
			// ( notEnd + dist ) < mustStart
			if ( ( uNotPos + pNot->m_uMatchlen - 1 + m_iDist )<uMustPos )
			{
				pNot++;
				continue;
			}

			// pNot is inside backward traking
			break;
		}

		bool bWindowHit = false;
		const ExtHit_t * pScan = pNot;
		while ( pScan->m_tRowID==tRowID )
		{
			DWORD uScanField = HITMAN::GetField ( pScan->m_uHitpos );
			
			if ( uScanField>uMustField )
				break;

			if ( uScanField<uMustField )
			{
				pScan++;
				continue;
			}

			DWORD uScanPos = HITMAN::GetPosWithField ( pScan->m_uHitpos );
			// ( mustEnd + dist ) < scanStart
			if ( ( uMustPos + pMust->m_uMatchlen - 1 + m_iDist )<uScanPos )
				break; 

			bWindowHit = true;
			break;
		}

		if ( !bWindowHit )
			m_dMyHits.Add ( *pMust );

		pMust++;
	}

	return ( iWasHits<m_dMyHits.GetLength() );
}

const ExtDoc_t * ExtNotNear_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;
	const ExtHit_t * pHitL = m_pHitL;
	const ExtHit_t * pHitR = m_pHitR;

	int iDoc = 0;
	bool bRightEmpty = false;

	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, pHitL, m_pLeft.get() ) )
			break;
		
		if ( !bRightEmpty )
		{
			if ( HasDocs(pDocL) )
				m_pRight->HintRowID ( pDocL->m_tRowID );

			if ( !WarmupDocs ( pDocR, pHitR, m_pRight.get() ) )
			{
				if ( TimeExceeded() )
					break;

				bRightEmpty = true;
			}
		}

		RowID_t tNotRowID = ( bRightEmpty ? INVALID_ROWID : pDocR->m_tRowID );

		// copy none matched from MUST
		while ( pDocL->m_tRowID < tNotRowID && iDoc<MAX_BLOCK_DOCS-1 )
		{
			m_dDocs[iDoc++] = *pDocL;

			while ( pHitL->m_tRowID<pDocL->m_tRowID )
				pHitL++;

			while ( pHitL->m_tRowID==pDocL->m_tRowID )
				m_dMyHits.Add ( *pHitL++ );

			pDocL++;
		}

		if ( !HasDocs(pDocL) || iDoc==MAX_BLOCK_DOCS-1 )
			continue;

		if ( bRightEmpty )
			continue;

		// skip NOT until min accepted id
		while ( pDocR->m_tRowID<pDocL->m_tRowID ) pDocR++;
		while ( pHitR->m_tRowID<pDocR->m_tRowID ) pHitR++;
		if ( !HasHits(pHitR) || pDocL->m_tRowID!=pDocR->m_tRowID )
			continue;

		// filter must with not
		assert ( HasDocs(pDocL) );
		assert ( pDocL->m_tRowID==pDocR->m_tRowID );
		assert ( pDocL->m_tRowID==pHitL->m_tRowID && pDocL->m_tRowID==pHitR->m_tRowID );

		bool bMatched = FilterHits ( pDocL->m_tRowID, pHitL, pHitR );
		bMatched |= pHitL->m_tRowID==pDocL->m_tRowID;

		// copy MUST tail hits
		while ( pHitL->m_tRowID==pDocL->m_tRowID )
			m_dMyHits.Add ( *pHitL++ );

		if ( bMatched )
			m_dDocs[iDoc++] = *pDocL;

		pDocL++;
		pDocR++;

		if ( HasDocs(pDocL) )
		{
			while ( pHitL->m_tRowID<pDocL->m_tRowID )
				pHitL++;
		}

		if ( HasDocs(pDocR) )
		{
			while ( pHitR->m_tRowID<pDocR->m_tRowID )
				pHitR++;
		}
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;
	m_pHitL = pHitL;
	m_pHitR = pHitR;

	return ReturnDocsChunk ( iDoc, "notnear" );
}


void ExtNotNear_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}


//////////////////////////////////////////////////////////////////////////
// INTRA-BATCH CACHING
//////////////////////////////////////////////////////////////////////////

/// container that does intra-batch query-sub-tree caching
/// actually carries the cached data, NOT to be recreated frequently (see thin wrapper below)
class NodeCacheContainer_c
{
	friend class ExtNodeCached_c;
	friend class CSphQueryNodeCache;

public:
	void						Release();
	ExtNode_i *					CreateCachedWrapper ( ExtNode_i* pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup );

private:
	int							m_iRefCount {1};
	bool						m_bStateOk {true};
	const ISphQwordSetup *		m_pSetup {nullptr};

	CSphVector<ExtDoc_t>		m_dDocs;
	CSphVector<ExtHit_t>		m_dHits;
	int							m_iAtomPos {0}; // minimal position from original donor, used for shifting

	CSphQueryNodeCache *		m_pNodeCache {nullptr};

	bool						WarmupCache ( ExtNode_i * pChild, int iQWords );
	void						Invalidate();
};


/// cached node wrapper to be injected into actual search trees
/// (special container actually carries all the data and does the work, see below)
class ExtNodeCached_c : public ExtNode_c
{
	friend class NodeCacheContainer_c;

public:
						~ExtNodeCached_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				HintRowID ( RowID_t tRowID ) override {}
	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pMatched ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override;
	uint64_t			GetWordID() const override;
	void				SetCollectHits() override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return { 0.0f, 0, 0 }; }
	void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) override;

private:
	NodeCacheContainer_c * m_pNode;
	const ExtDoc_t *	m_pHitDoc;			///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	CSphString *		m_pWarning;
	int					m_iHitIndex;		///< store the current position in m_Hits for GetHitsChunk()
	int					m_iDocIndex;		///< store the current position in m_Docs for GetDocsChunk()
	ExtNode_i *			m_pChild;			///< pointer to donor for the sake of AtomPos procession
	int					m_iQwords;			///< number of tokens in parent query


						// creation possible ONLY via NodeCacheContainer_c
						ExtNodeCached_c ( NodeCacheContainer_c * pNode, ExtNode_i * pChild );

	void				StepForwardToHitsFor ( RowID_t tRowID );
	bool				RewindDocs ( const ExtDoc_t * & pDoc, const ExtDoc_t * & pMatched );
};

//////////////////////////////////////////////////////////////////////////

void NodeCacheContainer_c::Release()
{
	if ( --m_iRefCount<=0 )
		Invalidate();
}


ExtNode_i * NodeCacheContainer_c::CreateCachedWrapper ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup )
{
	if ( !m_bStateOk )
		return pChild;

	// wow! we have a virgin!
	if ( !m_dDocs.GetLength() )
	{
		m_iRefCount = pRawChild->GetCount();
		m_pSetup = &tSetup;
	}
	return new ExtNodeCached_c ( this, pChild );
}


bool NodeCacheContainer_c::WarmupCache ( ExtNode_i * pChild, int iQwords )
{
	assert ( pChild );
	assert ( m_pSetup );

	m_iAtomPos = pChild->GetAtomPos();
	const ExtDoc_t * pChunk = pChild->GetDocsChunk();

	while ( pChunk )
	{
		const ExtDoc_t * pChunkHits = pChunk;
		bool iHasDocs = false;
		for ( ; HasDocs(pChunk); pChunk++ )
		{
			m_dDocs.Add ( *pChunk );
			// exclude number or Qwords from FIDF
			m_dDocs.Last().m_fTFIDF *= iQwords;
			m_pNodeCache->m_iMaxCachedDocs--;
			iHasDocs = true;
		}

		if ( iHasDocs )
		{
			const ExtHit_t * pHit = pChild->GetHits(pChunkHits);
			while (	HasHits(pHit) )
			{
				m_dHits.Add ( *pHit++ );
				m_pNodeCache->m_iMaxCachedHits--;
			}
		}

		// too many values, stop caching
		if ( m_pNodeCache->m_iMaxCachedDocs<0 || m_pNodeCache->m_iMaxCachedHits<0 )
		{
			Invalidate ();
			pChild->Reset ( *m_pSetup );
			m_pSetup = NULL;
			return false;
		}
		pChunk = pChild->GetDocsChunk();
	}

	m_dDocs.Add().m_tRowID = INVALID_ROWID;
	m_dHits.Add().m_tRowID = INVALID_ROWID;
	pChild->Reset ( *m_pSetup );
	m_pSetup = NULL;
	return true;
}


void NodeCacheContainer_c::Invalidate()
{
	m_pNodeCache->m_iMaxCachedDocs += m_dDocs.GetLength();
	m_pNodeCache->m_iMaxCachedHits += m_dDocs.GetLength();
	m_dDocs.Reset();
	m_dHits.Reset();
	m_bStateOk = false;
}

//////////////////////////////////////////////////////////////////////////

ExtNodeCached_c::~ExtNodeCached_c ()
{
	SafeDelete ( m_pChild );
	SafeRelease ( m_pNode );
}


void ExtNodeCached_c::Reset ( const ISphQwordSetup & tSetup )
{
	if ( m_pChild )
		m_pChild->Reset ( tSetup );

	m_iHitIndex = 0;
	m_iDocIndex = 0;
	m_pHitDoc = NULL;
	SetMaxTimeout ( tSetup.m_iMaxTimer );
	m_pWarning = tSetup.m_pWarning;
}


int ExtNodeCached_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	if ( !m_pChild )
		return -1;

	int iChildAtom = m_pChild->GetQwords ( hQwords );
	if ( iChildAtom<0 )
		return -1;

	return m_iAtomPos + iChildAtom;
}


void ExtNodeCached_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_iQwords = hQwords.GetLength();
	if ( m_pNode->m_pSetup && m_pChild )
	{
		m_pChild->SetQwordsIDF ( hQwords );
		m_pNode->WarmupCache ( m_pChild, m_iQwords );
	}
}


void ExtNodeCached_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_pChild )
		m_pChild->GetTerms ( hQwords, dTermDupes );
}


bool ExtNodeCached_c::GotHitless ()
{
	if ( m_pChild )
		return m_pChild->GotHitless();

	return false;
}


uint64_t ExtNodeCached_c::GetWordID() const
{
	if ( m_pChild )
		return m_pChild->GetWordID();

	return 0;
}


void ExtNodeCached_c::SetCollectHits()
{
	if ( m_pChild )
		m_pChild->SetCollectHits();
}


void ExtNodeCached_c::SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries )
{
	if ( m_pChild )
		m_pChild->SetRowidBoundaries(tBoundaries);
}


ExtNodeCached_c::ExtNodeCached_c ( NodeCacheContainer_c * pNode, ExtNode_i * pChild )
	: ExtNode_c (0)
	, m_pNode ( pNode )
	, m_pHitDoc ( NULL )
	, m_pWarning ( NULL )
	, m_iHitIndex ( 0 )
	, m_iDocIndex ( 0 )
	, m_pChild ( pChild )
	, m_iQwords ( 0 )
{
	m_iAtomPos = pChild->GetAtomPos();
}


void ExtNodeCached_c::StepForwardToHitsFor ( RowID_t tRowID )
{
	assert ( m_pNode );
	assert ( m_pNode->m_bStateOk );

	CSphVector<ExtHit_t> & dHits = m_pNode->m_dHits;

	int iEnd = dHits.GetLength()-1;
	if ( m_iHitIndex>=iEnd )
		return;

	if ( dHits[m_iHitIndex].m_tRowID==tRowID )
		return;

	m_iHitIndex = sphBinarySearchFirst ( dHits.Begin(), m_iHitIndex, iEnd, bind ( &ExtHit_t::m_tRowID ),tRowID );
}


const ExtDoc_t * ExtNodeCached_c::GetDocsChunk()
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_bStateOk )
		return m_pChild->GetDocsChunk();

	if ( TimeExceeded() )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return nullptr;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	int iDoc = Min ( m_iDocIndex+MAX_BLOCK_DOCS-1, m_pNode->m_dDocs.GetLength()-1 ) - m_iDocIndex;
	memcpy ( &m_dDocs[0], &m_pNode->m_dDocs[m_iDocIndex], sizeof(ExtDoc_t)*iDoc );
	m_iDocIndex += iDoc;

	// funny trick based on the formula of FIDF calculation.
	for ( int i=0; i<iDoc; i++ )
		m_dDocs[i].m_fTFIDF /= m_iQwords;

	return ReturnDocsChunk ( iDoc, "cached" );
}


bool ExtNodeCached_c::RewindDocs ( const ExtDoc_t * & pDoc, const ExtDoc_t * & pMatched )
{
	do
	{
		while ( pDoc->m_tRowID < pMatched->m_tRowID )
			pDoc++;

		if ( !HasDocs(pDoc) )
			return false; // matched docs block is over for me, gimme another one

		while ( pMatched->m_tRowID < pDoc->m_tRowID )
			pMatched++;

		if ( !HasDocs(pMatched) )
			return false; // matched doc block did not yet begin for me, gimme another one
	}
	while ( pDoc->m_tRowID!=pMatched->m_tRowID );

	// setup hitlist reader
	StepForwardToHitsFor ( pDoc->m_tRowID );

	return true;
}


void ExtNodeCached_c::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !m_pNode || !m_pChild )
		return;

	if ( !m_pNode->m_bStateOk )
	{
		const ExtHit_t * pHit = m_pChild->GetHits(pMatched);
		while ( HasHits(pHit) )
			m_dHits.Add ( *pHit++ );

		return;
	}

	if ( !pMatched )
		return;

	// aim to the right document
	const ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// find match
		pDoc = m_dDocs;
		RewindDocs ( pDoc, pMatched );
	}

	// hit emission
	while ( true )
	{
		// get next hit
		ExtHit_t & tCachedHit = m_pNode->m_dHits[m_iHitIndex];
		if ( !HasHits(&tCachedHit) )
			break;

		if ( tCachedHit.m_tRowID==pDoc->m_tRowID )
		{
			m_iHitIndex++;
			ExtHit_t & tHit = m_dHits.Add();
			tHit = tCachedHit;
			tHit.m_uQuerypos = (WORD)( tCachedHit.m_uQuerypos + m_iAtomPos - m_pNode->m_iAtomPos );
		}
		else
		{
			// no more hits; get next acceptable document
			pDoc++;
			if ( !RewindDocs ( pDoc, pMatched ) )
			{
				pDoc = nullptr;
				break;
			}

			assert ( pDoc->m_tRowID==pMatched->m_tRowID );

			// setup hitlist reader
			StepForwardToHitsFor ( pDoc->m_tRowID );
		}
	}

	m_pHitDoc = pDoc;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryNodeCache::CSphQueryNodeCache ( int iCells, int iMaxCachedDocs, int iMaxCachedHits )
{
	if ( iCells>0 && iMaxCachedHits>0 && iMaxCachedDocs>0 )
	{
		m_pPool = new NodeCacheContainer_c [ iCells ];
		for ( int i=0; i<iCells; i++ )
			m_pPool[i].m_pNodeCache = this;
	}
	m_iMaxCachedDocs = iMaxCachedDocs / sizeof(ExtDoc_t);
	m_iMaxCachedHits = iMaxCachedHits / sizeof(ExtHit_t);
}

CSphQueryNodeCache::~CSphQueryNodeCache ()
{
	SafeDeleteArray ( m_pPool );
}

ExtNode_i * CSphQueryNodeCache::CreateProxy ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup )
{
	// TEMPORARILY DISABLED
	return pChild;

/*	if ( m_iMaxCachedDocs<=0 || m_iMaxCachedHits<=0 )
		return pChild;

	assert ( pRawChild );
	return m_pPool [ pRawChild->GetOrder() ].CreateCachedWrapper ( pChild, pRawChild, tSetup );*/
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<ExtNode_i> CreatePseudoFTNode ( ExtNode_i * pNode, RowidIterator_i * pIterator )
{
	return std::make_unique<ExtAndRightHits_c> ( new ExtIterator_c(pIterator), pNode );
}

/// Immediately interrupt current operation
void sphInterruptNow()
{
	g_bInterruptNow = true;
}

bool sphInterrupted()
{
	return g_bInterruptNow;
}
