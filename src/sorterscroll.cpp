//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sorterscroll.h"

#include "std/base64.h"
#include "sortcomp.h"
#include "sphinxjson.h"

template <typename COMP>
class ScrollSorter_T : public ISphMatchSorter 
{
	using MYTYPE = ScrollSorter_T<COMP>;

public:
			ScrollSorter_T ( ISphMatchSorter * pSorter, const ScrollSettings_t & tScroll ) : m_pSorter ( pSorter ), m_tScroll ( tScroll ) {}
			~ScrollSorter_T() override { FreeDataPtrAttrs(); }

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushMatch(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) override;

	bool	IsGroupby() const override										{ return m_pSorter->IsGroupby(); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override	{ return m_pSorter->PushGrouped ( tEntry, bNewSet ); }
	int		GetLength () override											{ return m_pSorter->GetLength(); }
	void	Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) override { m_pSorter->Finalize ( tProcessor, bCallProcessInResultSetOrder, bFinalizeMatches ); }
	int		Flatten ( CSphMatch * pTo ) override							{ return m_pSorter->Flatten(pTo); }
	void	MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) override		{ m_pSorter->MoveTo ( ((MYTYPE*)pRhs)->m_pSorter.get(), bCopyMeta ); }

	ISphMatchSorter * Clone() const override;
	void	CloneTo ( ISphMatchSorter * pTrg ) const override;

	void	SetState ( const CSphMatchComparatorState & tState ) override;
	const CSphMatchComparatorState & GetState() const override				{ return m_pSorter->GetState(); }

	void	SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override;
	const ISphSchema * GetSchema() const override							{ return m_pSorter->GetSchema(); }

	void	SetColumnar ( columnar::Columnar_i * pColumnar ) override		{ m_pSorter->SetColumnar(pColumnar); }
	int64_t	GetTotalCount() const override									{ return m_pSorter->GetTotalCount(); }

	void	SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override { m_pSorter->SetFilteredAttrs ( hAttrs, bAddDocid ); }
	void	TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override { m_pSorter->TransformPooled2StandalonePtrs ( fnBlobPoolFromMatch, fnGetColumnarFromMatch, bFinalizeSorters ); }

	bool	IsRandom() const override 										{ return m_pSorter->IsRandom(); }
	void	SetRandom ( bool bRandom ) override								{ m_pSorter->SetRandom(bRandom); }

	int		GetMatchCapacity() const override								{ return m_pSorter->GetMatchCapacity(); }

	RowTagged_t					GetJustPushed() const override				{ return m_pSorter->GetJustPushed(); }
	VecTraits_T<RowTagged_t>	GetJustPopped() const override				{ return m_pSorter->GetJustPopped(); }

	void	SetMerge ( bool bMerge ) override								{ m_pSorter->SetMerge(bMerge); }

private:
	std::unique_ptr<ISphMatchSorter> m_pSorter;
	CSphMatch						m_tRefMatch;
	CSphVector<CSphRowitem>			m_dStatic;
	CSphMatchComparatorState		m_tState;
	ScrollSettings_t				m_tScroll;

	FORCE_INLINE bool PushMatch ( const CSphMatch & tEntry );
	void	SetupRefMatch();
	void	FreeDataPtrAttrs();
};

template <typename COMP>
void ScrollSorter_T<COMP>::Push ( const VecTraits_T<const CSphMatch> & dMatches )
{
	for ( auto & i : dMatches )
		PushMatch(i);
}

template <typename COMP>
ISphMatchSorter * ScrollSorter_T<COMP>::Clone() const
{
	auto pClone = new ScrollSorter_T<COMP> ( m_pSorter->Clone(), m_tScroll );
	CloneTo(pClone);
	return pClone;
}

template <typename COMP>
void ScrollSorter_T<COMP>::SetState ( const CSphMatchComparatorState & tState )
{
	m_pSorter->SetState(tState);
	m_tState = tState;
}

template <typename COMP>
void ScrollSorter_T<COMP>::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	m_pSorter->SetSchema ( pSchema, bRemapCmp );
	SetupRefMatch();
}

template <typename COMP>
void ScrollSorter_T<COMP>::CloneTo ( ISphMatchSorter * pTrg ) const
{
	pTrg->SetRandom ( IsRandom() );
	pTrg->SetState  ( GetState() );
	pTrg->SetSchema ( GetSchema()->CloneMe(), false );
}

template <typename COMP>
bool ScrollSorter_T<COMP>::PushMatch ( const CSphMatch & tEntry )
{
	if ( !COMP::IsLess ( tEntry, m_tRefMatch, m_tState ) )
		return false;

	return m_pSorter->Push(tEntry);
}

template <typename COMP>
void ScrollSorter_T<COMP>::SetupRefMatch()
{
	const ISphSchema * pSchema = m_pSorter->GetSchema();
	assert(pSchema);

	FreeDataPtrAttrs();
	m_tRefMatch.Reset ( pSchema->GetRowSize() );
	m_dStatic.Resize ( pSchema->GetStaticSize() );
	m_tRefMatch.m_pStatic = m_dStatic.Begin();

	for ( const auto & i : m_tScroll.m_dAttrs )
	{
		bool bWeight = i.m_sSortAttr=="weight()";
		if ( bWeight )
		{
			m_tRefMatch.m_iWeight = i.m_tValue;
			continue;
		}

		auto pAttr = pSchema->GetAttr ( i.m_sSortAttr.cstr() );
		auto pRowData = pAttr->m_tLocator.m_bDynamic ? m_tRefMatch.m_pDynamic : m_dStatic.Begin();
		switch ( i.m_eType )
		{
		case SPH_ATTR_STRINGPTR:
			sphSetRowAttr ( pRowData, pAttr->m_tLocator, (SphAttr_t)sphPackPtrAttr ( { (const BYTE*)i.m_sValue.cstr(), i.m_sValue.Length() } ) );
			break;

		case SPH_ATTR_FLOAT:
			sphSetRowAttr ( pRowData, pAttr->m_tLocator, sphF2DW(i.m_fValue) );
			break;

		default:
			sphSetRowAttr ( pRowData, pAttr->m_tLocator, i.m_tValue );
			break;
		}
	}
}

template <typename COMP>
void ScrollSorter_T<COMP>::FreeDataPtrAttrs()
{
	if ( !m_tRefMatch.m_pDynamic )
		return;

	const ISphSchema * pSchema = m_pSorter->GetSchema();
	assert(pSchema);

	for ( auto & i : m_tScroll.m_dAttrs )
	{
		if ( i.m_sSortAttr=="weight()" )
			continue;

		const CSphColumnInfo * pAttr = pSchema->GetAttr ( i.m_sSortAttr.cstr() );
		assert(pAttr);
		if ( sphIsDataPtrAttr ( pAttr->m_eAttrType ) )
		{
			auto pData = (BYTE *)m_tRefMatch.GetAttr ( pAttr->m_tLocator );
			sphDeallocatePacked(pData);
		}
	}
}

/////////////////////////////////////////////////////////////////////

static bool CanCreateScrollSorter ( bool bMulti, const ISphSchema & tSchema, const ScrollSettings_t & tScroll )
{
	if ( !tScroll.m_dAttrs.GetLength() )
		return false;

	// if we only have one sort attribute (id), we don't need scroll at all (a filter would be enough)
	if ( tScroll.m_dAttrs.GetLength()==1 )
	{
		assert ( tScroll.m_dAttrs[0].m_sSortAttr=="id" );
		return false;
	}

	if ( bMulti )
		return false;

	ESphAttr dSupportedTypes[]=
	{
		SPH_ATTR_INTEGER,
		SPH_ATTR_TIMESTAMP,
		SPH_ATTR_BOOL,
		SPH_ATTR_FLOAT,
		SPH_ATTR_BIGINT,
		SPH_ATTR_TOKENCOUNT,
		SPH_ATTR_DOUBLE,
		SPH_ATTR_UINT64,
		SPH_ATTR_STRINGPTR
	};

	for ( const auto & i : tScroll.m_dAttrs )
	{
		if ( i.m_sSortAttr=="weight()" )
			continue;

		auto pAttr = tSchema.GetAttr ( i.m_sSortAttr.cstr() );
		if ( !pAttr )
			return false;

		bool bSupported = false;
		for ( auto eType : dSupportedTypes )
			if ( pAttr->m_eAttrType==eType )
			{
				bSupported = true;
				break;
			}

		if ( !bSupported )
			return false;
	}

	return true;
}


ISphMatchSorter * CreateScrollSorter ( ISphMatchSorter * pSorter, const ISphSchema & tSchema, ESphSortFunc eSortFunc, const ScrollSettings_t & tScroll, bool bMulti )
{
	if ( !CanCreateScrollSorter ( bMulti, tSchema, tScroll ) )
		return pSorter;

	switch ( eSortFunc )
	{
	case FUNC_REL_DESC:		return new ScrollSorter_T<MatchRelevanceLt_fn>	( pSorter, tScroll );
	case FUNC_TIMESEGS:		return new ScrollSorter_T<MatchTimeSegments_fn>	( pSorter, tScroll );
	case FUNC_GENERIC1:		return new ScrollSorter_T<MatchGeneric1_fn>		( pSorter, tScroll );
	case FUNC_GENERIC2:		return new ScrollSorter_T<MatchGeneric2_fn>		( pSorter, tScroll );
	case FUNC_GENERIC3:		return new ScrollSorter_T<MatchGeneric3_fn>		( pSorter, tScroll );
	case FUNC_GENERIC4:		return new ScrollSorter_T<MatchGeneric4_fn>		( pSorter, tScroll );
	case FUNC_GENERIC5:		return new ScrollSorter_T<MatchGeneric5_fn>		( pSorter, tScroll );
	case FUNC_EXPR:			return new ScrollSorter_T<MatchExpr_fn>			( pSorter, tScroll );
	default:				return pSorter;
	}

	return pSorter;
}


bool ParseScroll ( CSphQuery & tQuery, const CSphString & sVal, CSphString & sError )
{
	CSphString sDecoded;
	if ( !DecodeBase64 ( sVal, sDecoded ) )
	{
		sError.SetSprintf ( "unable to decode BASE64: %s", sVal.cstr() );
		return false;
	}

	JsonObj_c tRoot ( sDecoded.cstr() );
	if ( !tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tRoot.GetErrorPtr() );
		return false;
	}

	ScrollSettings_t tScrollSettings;
	if ( !tRoot.FetchStrItem ( tScrollSettings.m_sSortBy, "order_by_str", sError ) )	return false;

	JsonObj_c tOrderByVec = tRoot.GetArrayItem ( "order_by", sError );
	if ( !tOrderByVec )
		return false;

	for ( const auto & tItem : tOrderByVec )
	{
		ScrollAttr_t & tNew = tScrollSettings.m_dAttrs.Add();
		if ( !tItem.FetchStrItem ( tNew.m_sSortAttr, "attr", sError ) )	return false;
		if ( !tItem.FetchBoolItem ( tNew.m_bDesc, "desc", sError ) )	return false;

		CSphString sType;
		if ( !tItem.FetchStrItem ( sType, "type", sError ) )			return false;
		if ( sType=="string" )
		{
			if ( !tItem.FetchStrItem ( tNew.m_sValue, "value", sError ) )	return false;
			tNew.m_eType = SPH_ATTR_STRINGPTR;
		}
		else if ( sType=="float" )
		{
			if ( !tItem.FetchFltItem ( tNew.m_fValue, "value", sError ) )	return false;
			tNew.m_eType = SPH_ATTR_FLOAT;
		}
		else
		{
			if ( !tItem.FetchInt64Item ( tNew.m_tValue, "value", sError ) )	return false;
			tNew.m_eType = SPH_ATTR_BIGINT;
		}
	}

	tQuery.m_tScrollSettings = std::move(tScrollSettings);
	return true;
}


static void AddScrollFilter ( CSphQuery & tQuery )
{
	const ScrollAttr_t & tFirst = tQuery.m_tScrollSettings.m_dAttrs[0];
	int iFilterId = tQuery.m_dFilters.GetLength();

	// we don't have string range filters
	if ( tFirst.m_eType==SPH_ATTR_STRINGPTR )
		return;

	bool bOnlyId = tQuery.m_tScrollSettings.m_dAttrs.GetLength()==1;

	CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
	tFilter.m_eType = tFirst.m_eType==SPH_ATTR_FLOAT ? SPH_FILTER_FLOATRANGE : SPH_FILTER_RANGE;
	tFilter.m_sAttrName = tFirst.m_sSortAttr=="weight()" ? "@weight" : tFirst.m_sSortAttr;
	tFilter.m_bHasEqualMin = tFilter.m_bHasEqualMax = !bOnlyId;

	if ( tFirst.m_eType==SPH_ATTR_FLOAT )
	{
		if ( tFirst.m_bDesc )
			tFilter.m_fMaxValue = tFirst.m_fValue;
		else
			tFilter.m_fMinValue = tFirst.m_fValue;
	}
	else
	{
		if ( tFirst.m_bDesc )
			tFilter.m_iMaxValue = tFirst.m_tValue;
		else
			tFilter.m_iMinValue = tFirst.m_tValue;
	}

	if ( tQuery.m_dFilterTree.GetLength() )
	{
		int iRootNodeId = tQuery.m_dFilterTree.GetLength()-1;
		FilterTreeItem_t & tFilter = tQuery.m_dFilterTree.Add();
		tFilter.m_iFilterItem = iFilterId;

		int iFilterNodeId = tQuery.m_dFilterTree.GetLength()-1;
		FilterTreeItem_t & tAnd = tQuery.m_dFilterTree.Add();
		tAnd.m_iLeft = iRootNodeId;
		tAnd.m_iRight = iFilterNodeId;
	}
}


bool SetupScroll ( CSphQuery & tQuery, CSphString & sError )
{
	if ( !tQuery.m_tScrollSettings.m_dAttrs.GetLength() )
		return true;

	// replace order by with order_by_str here (if order by is default)
	const char * szDefaultSortBy = "@weight desc";
	if ( tQuery.m_sSortBy==szDefaultSortBy )
		tQuery.m_sOrderBy = tQuery.m_sSortBy = tQuery.m_tScrollSettings.m_sSortBy;
	else if ( tQuery.m_sSortBy!=tQuery.m_tScrollSettings.m_sSortBy )
	{
		sError.SetSprintf ( "order by '%s' different from scroll order by '%s'", tQuery.m_sSortBy.cstr(), tQuery.m_tScrollSettings.m_sSortBy.cstr() );
		return false;
	}

	if ( !tQuery.m_tScrollSettings.m_dAttrs.any_of ( []( auto & tAttr ){ return tAttr.m_sSortAttr=="id"; } ) )
	{
		sError = "document id not present in scroll settings";
		return false;
	}

	// add filter on 1st scroll attribute
	AddScrollFilter(tQuery);
	return true;
}
