//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sortertraits.h"

#include "schematransform.h"


void MatchSorter_c::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	assert ( pSchema );
	m_tState.FixupLocators ( m_pSchema, pSchema, bRemapCmp );

	m_pSchema = pSchema;
}


void MatchSorter_c::SetState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	m_tState.m_iNow = (DWORD) time ( nullptr );
}


void MatchSorter_c::CloneTo ( ISphMatchSorter * pTrg ) const
{
	assert ( pTrg );
	pTrg->SetRandom(m_bRandomize);
	pTrg->SetState(m_tState);
	pTrg->SetSchema ( m_pSchema->CloneMe(), false );
}


bool MatchSorter_c::CanBeCloned() const
{
	if ( !m_pSchema )
		return true;

	bool bGotStatefulUDF = false;
	for ( int i = 0; i < m_pSchema->GetAttrsCount() && !bGotStatefulUDF; i++ )
	{
		auto & pExpr = m_pSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->Command ( SPH_EXPR_GET_STATEFUL_UDF, &bGotStatefulUDF );
	}

	return !bGotStatefulUDF;
}


void MatchSorter_c::SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid )
{
	assert ( m_pSchema );

	m_dTransformed.Reserve ( hAttrs.GetLength() );

	if ( bAddDocid && !hAttrs[sphGetDocidName()] )
		m_dTransformed.Add ( sphGetDocidName() );

	for ( auto & tName : hAttrs )
	{
		const CSphColumnInfo * pCol = m_pSchema->GetAttr ( tName.first.cstr() );
		if ( pCol )
			m_dTransformed.Add ( pCol->m_sName );
	}
}

void MatchSorter_c::TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters )
{
	auto * pOldSchema = GetSchema();
	assert ( pOldSchema );

	// create new standalone schema (from old, or from filtered)
	auto * pNewSchema = new CSphSchema ( "standalone" );
	for ( int i = 0; i<pOldSchema->GetFieldsCount (); ++i )
		pNewSchema->AddField ( pOldSchema->GetField(i) );

	TransformedSchemaBuilder_c tBuilder ( *pOldSchema, *pNewSchema );

	if ( m_dTransformed.IsEmpty() )
	{
		// keep id as the first attribute
		const CSphColumnInfo* pId = pOldSchema->GetAttr ( sphGetDocidName() );
		if ( pId )
			tBuilder.AddAttr ( sphGetDocidName() );

		// add the rest
		for ( int i = 0; i<pOldSchema->GetAttrsCount (); i++ )
		{
			const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
			if ( tAttr.m_sName!=sphGetDocidName() && tAttr.m_sName!=GetNullMaskAttrName() )
				tBuilder.AddAttr ( tAttr.m_sName );
		}
	}
	else
	{
		// keep id as the first attribute, then the rest.
		m_dTransformed.any_of ( [&tBuilder] ( const auto& sName ) { auto bID = ( sName==sphGetDocidName() ); if ( bID ) tBuilder.AddAttr(sName); return bID; } );
		m_dTransformed.for_each ( [&tBuilder] ( const auto& sName ) { if ( sName!=sphGetDocidName() && sName!=GetNullMaskAttrName() ) tBuilder.AddAttr(sName); } );
	}

	for ( int i = 0; i <pNewSchema->GetAttrsCount(); ++i )
	{
		auto & pExpr = pNewSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	tBuilder.Finalize();

	auto pMatchesToNewSchema = std::unique_ptr<MatchProcessor_i>( CreateMatchSchemaTransform ( pOldSchema, pNewSchema, std::move ( fnBlobPoolFromMatch ), std::move ( fnGetColumnarFromMatch ) ) );
	Finalize ( *pMatchesToNewSchema, false, bFinalizeSorters );
	SetSchema ( pNewSchema, true );
}

///////////////////////////////////////////////////////////////////////////////

CSphMatchQueueTraits::CSphMatchQueueTraits ( int iSize )
	: m_iSize ( iSize )
	, m_dData { iSize }
{
	assert ( iSize>0 );
	m_iMatchCapacity = iSize;
	m_dIData.Resize ( iSize );
	m_tState.m_iNow = (DWORD) time ( nullptr );

	ARRAY_FOREACH ( i, m_dIData )
		m_dIData[i] = i;
	m_dIData.Resize ( 0 );
}

/// dtor make FreeDataPtrs here, then ResetDynamic also get called on m_dData d-tr.
CSphMatchQueueTraits::~CSphMatchQueueTraits ()
{
	if ( m_pSchema )
		m_dData.Apply ( [this] ( CSphMatch& tMatch ) { m_pSchema->FreeDataPtrs ( tMatch ); } );
}


void CSphMatchQueueTraits::SwapMatchQueueTraits ( CSphMatchQueueTraits& rhs )
{
	// ISphMatchSorter
	::Swap ( m_iTotal, rhs.m_iTotal );

	// MatchSorter_c
	::Swap ( m_iMatchCapacity, rhs.m_iMatchCapacity );

	// CSphMatchQueueTraits
	m_dData.SwapData ( rhs.m_dData );
	m_dIData.SwapData ( rhs.m_dIData );
	::Swap ( m_iSize, rhs.m_iSize );
}


CSphMatch & CSphMatchQueueTraits::Add()
{
	// proper ids at m_dIData already set at constructor
	// they will be same during life-span - that is why Add used like anti-Pop
	int iLast = m_dIData.Add();
	return m_dData[iLast];
}


int CSphMatchQueueTraits::ResetDynamic ( int iMaxUsed )
{
	for ( int i=0; i<iMaxUsed; i++ )
		m_dData[i].ResetDynamic();

	return -1;
}


int CSphMatchQueueTraits::ResetDynamicFreeData ( int iMaxUsed )
{
	for ( int i=0; i<iMaxUsed; i++ )
	{
		m_pSchema->FreeDataPtrs ( m_dData[i] );
		m_dData[i].ResetDynamic();
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////

void MatchCloner_c::SetSchema ( const ISphSchema * pSchema )
{
	m_pSchema = (const CSphSchemaHelper *) pSchema; /// lazy hack
	m_dRowBuf.Reset ( m_pSchema->GetDynamicSize() );
}

// clone plain part (incl. pointers) from src to dst
// keep group part (aggregates, group_concat) of dst intact
// it assumes that tDst m_pDynamic contains correct data, or wiped away.
void MatchCloner_c::CloneKeepingAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
{
	assert ( m_pSchema );
	assert ( m_bPtrRowsCommited );

	// memorize old dynamic first
	memcpy ( m_dRowBuf.Begin(), tDst.m_pDynamic, m_dRowBuf.GetLengthBytes() );

	m_pSchema->CloneMatchSpecial ( tDst, tSrc, m_dOtherPtrRows );

	/*
	FreeDataSpecial ( tDst, m_dOtherPtrRows );
	pDst->Combine ( *pSrc, GetDynamicSize () );
	CopyPtrsSpecial ( tDst, tSrc, m_dOtherPtrRows );
	*/

	// restore back group-by attributes
	for ( auto & tAttrGrp : m_dAttrsGrp )
		tDst.SetAttr ( tAttrGrp, sphGetRowAttr ( m_dRowBuf.Begin(), tAttrGrp ) );

	// restore back group_concat attribute(s)
	for ( auto & tAttrPtr : m_dAttrsPtr )
		tDst.SetAttr ( tAttrPtr, sphGetRowAttr ( m_dRowBuf.Begin (), tAttrPtr ) );
}

// clone plain part (incl. pointers) from src to dst
// group part (aggregates, group_concat) is not copied
void MatchCloner_c::CloneWithoutAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
{
	assert ( m_pSchema );
	assert ( m_bPtrRowsCommited );

	m_pSchema->CloneMatchSpecial ( tDst, tSrc, m_dOtherPtrRows );
	/*
	FreeDataSpecial ( tDst, m_dOtherPtrRows );
	pDst->Combine ( *pSrc, GetDynamicSize () );
	CopyPtrsSpecial ( tDst, tSrc, m_dOtherPtrRows );
	*/
}

// just write group part (aggregates, group_concat) without cloning
// assumes tDst has allocated m_pDynamic. Fixme! look to #881 again...
void MatchCloner_c::CopyAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
{
	assert ( m_pSchema );
	assert ( m_bPtrRowsCommited );
	assert ( &tDst!=&tSrc );
	assert ( tDst.m_pDynamic );

	for ( auto & dAttrGrp : m_dAttrsGrp )
		tDst.SetAttr ( dAttrGrp, tSrc.GetAttr ( dAttrGrp ));

	CSphSchemaHelper::FreeDataSpecial ( tDst, m_dMyPtrRows );
	CSphSchemaHelper::CopyPtrsSpecial ( tDst, tSrc, m_dMyPtrRows );
}

// copy group part (aggregates)
// move group_concat part without reallocating
void MatchCloner_c::MoveAggrs ( CSphMatch & tDst, CSphMatch & tSrc )
{
	assert ( m_pSchema );
	assert ( m_bPtrRowsCommited );
	assert ( &tDst!=&tSrc );
	assert ( tDst.m_pDynamic );

	for ( auto & dAttrGrp : m_dAttrsGrp )
		tDst.SetAttr ( dAttrGrp, tSrc.GetAttr ( dAttrGrp ));

	CSphSchemaHelper::MovePtrsSpecial( tDst, tSrc, m_dMyPtrRows );
}


void MatchCloner_c::ResetAttrs()
{
	m_dAttrsGrp.Resize ( 0 );
	m_dAttrsPtr.Resize ( 0 );
}


void MatchCloner_c::CommitPtrs()
{
	assert ( m_pSchema );
	static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

	if ( m_bPtrRowsCommited )
		m_dMyPtrRows.Resize(0);

	for ( const CSphAttrLocator &tLoc : m_dAttrsPtr )
		m_dMyPtrRows.Add ( tLoc.m_iBitOffset / SIZE_OF_ROW );

	m_dOtherPtrRows = m_pSchema->SubsetPtrs ( m_dMyPtrRows );

#ifndef NDEBUG
	// sanitize check
	m_dMyPtrRows = m_pSchema->SubsetPtrs ( m_dOtherPtrRows );
	assert ( m_dMyPtrRows.GetLength ()==m_dAttrsPtr.GetLength () );
#endif
	m_bPtrRowsCommited = true;
}



///////////////////////////////////////////////////////////////////////////////

void BaseGroupSorter_c::SetColumnar ( columnar::Columnar_i * pColumnar )
{
	for ( auto i : m_dAggregates )
		i->SetColumnar(pColumnar);
}


void BaseGroupSorter_c::SetupBaseGrouper ( ISphSchema * pSchema, int iDistinct, CSphVector<AggrFunc_i *> * pAvgs )
{
	m_tPregroup.ResetAttrs();
	ResetAggregates();

	m_tPregroup.SetSchema ( pSchema );
	m_tPregroup.AddRaw ( m_tLocGroupby ); // @groupby
	m_tPregroup.AddRaw ( m_tLocCount ); // @count
	if ( iDistinct )
		m_tPregroup.AddRaw ( m_tLocDistinct ); // @distinct

	// extract aggregates
	for ( int i = 0; i<pSchema->GetAttrsCount (); ++i )
	{
		const CSphColumnInfo &tAttr = pSchema->GetAttr ( i );

		if ( tAttr.m_eAggrFunc==SPH_AGGR_NONE
			|| IsGroupbyMagic ( tAttr.m_sName ) // @count, @groupby, @groupbystr, @distinct, count(*), groupby()
			|| IsSortStringInternal ( tAttr.m_sName.cstr () ) )
			continue;

		switch ( tAttr.m_eAggrFunc )
		{
		case SPH_AGGR_SUM:	m_dAggregates.Add ( CreateAggrSum(tAttr) );	break;
		case SPH_AGGR_AVG:
			m_dAggregates.Add ( CreateAggrAvg ( tAttr, m_tLocCount ) );

			// store avg to calculate these attributes prior to groups sort
			if ( pAvgs )
				pAvgs->Add ( m_dAggregates.Last() );
			break;

		case SPH_AGGR_MIN:	m_dAggregates.Add ( CreateAggrMin(tAttr) );	break;
		case SPH_AGGR_MAX:	m_dAggregates.Add ( CreateAggrMax(tAttr) );	break;
		case SPH_AGGR_CAT:
			m_dAggregates.Add ( CreateAggrConcat(tAttr) );
			m_tPregroup.AddPtr ( tAttr.m_tLocator );
			break;

		default: assert ( 0 && "internal error: unhandled aggregate function" );
			break;
		}

		if ( tAttr.m_eAggrFunc!=SPH_AGGR_CAT )
			m_tPregroup.AddRaw ( tAttr.m_tLocator );
	}
	m_tPregroup.CommitPtrs();
}


bool BaseGroupSorter_c::EvalHAVING ( const CSphMatch& tMatch )
{
	return !m_pAggrFilterTrait || m_pAggrFilterTrait->Eval ( tMatch );
}


void BaseGroupSorter_c::AggrUpdate ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge )
{
	for ( auto * pAggregate : this->m_dAggregates )
		pAggregate->Update ( tDst, tSrc, bGrouped, bMerge );
}


void BaseGroupSorter_c::AggrSetup ( CSphMatch & tDst, const CSphMatch & tSrc, bool bMerge )
{
	for ( auto * pAggregate : this->m_dAggregates )
		pAggregate->Setup ( tDst, tSrc, bMerge );
}


void BaseGroupSorter_c::AggrUngroup ( CSphMatch & tMatch )
{
	for ( auto * pAggregate : this->m_dAggregates )
		pAggregate->Ungroup ( tMatch );
}


void BaseGroupSorter_c::ResetAggregates()
{
	for ( auto & pAggregate : m_dAggregates )
		SafeDelete ( pAggregate );

	m_dAggregates.Resize(0);
}
