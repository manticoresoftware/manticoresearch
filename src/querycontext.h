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

#pragma once

#include "sphinx.h"
#include "sphinxfilter.h"

struct ContextCalcItem_t
{
	CSphAttrLocator			m_tLoc;						// result locator
	ESphAttr				m_eType = SPH_ATTR_NONE;	// result type
	ISphExprRefPtr_c		m_pExpr;					// evaluator (non-owned)
};


FORCE_INLINE void CalcContextItem ( CSphMatch & tMatch, const ContextCalcItem_t & tCalc )
{
	switch ( tCalc.m_eType )
	{
	case SPH_ATTR_BOOL:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
		tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->IntEval(tMatch) );
		break;

	case SPH_ATTR_BIGINT:
	case SPH_ATTR_UINT64:
	case SPH_ATTR_JSON_FIELD:
		tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->Int64Eval(tMatch) );
		break;

	case SPH_ATTR_STRINGPTR:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->StringEvalPacked ( tMatch ) );
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->FactorEvalPacked ( tMatch ) ); // FIXME! a potential leak of *previous* value?
		break;

	case SPH_ATTR_INT64SET_PTR:
	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_FLOAT_VECTOR_PTR:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->Int64Eval ( tMatch ) );
		break;

	case SPH_ATTR_DOUBLE:
		tMatch.SetAttrDouble ( tCalc.m_tLoc, tCalc.m_pExpr->Eval(tMatch) );
		break;

	default:
		tMatch.SetAttrFloat ( tCalc.m_tLoc, tCalc.m_pExpr->Eval(tMatch) );
		break;
	}
}


FORCE_INLINE void CalcContextItems ( CSphMatch & tMatch, const VecTraits_T<ContextCalcItem_t> & dItems )
{
	for ( auto & i : dItems )
		CalcContextItem ( tMatch, i );
}


FORCE_INLINE void FreeDataPtrAttrs ( CSphMatch & tMatch, const CSphVector<ContextCalcItem_t> & dItems, const IntVec_t & dItemIndexes )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( auto i : dItemIndexes )
	{
		const auto & tItem = dItems[i];

		auto * pData = (BYTE *)tMatch.GetAttr ( tItem.m_tLoc );
		// delete[] pData;
		if ( pData )
		{
			sphDeallocatePacked ( pData );
			tMatch.SetAttr ( tItem.m_tLoc, 0 );
		}
	}
}

class ISphRanker;
class Docstore_i;
struct CreateFilterContext_t;
using UservarIntSetValues_c = CSphVector<SphAttr_t>;
using UservarIntSet_c = SharedPtr_t<UservarIntSetValues_c>;

/// per-query search context
/// everything that index needs to compute/create to process the query
class CSphQueryContext : public ISphNoncopyable
{
public:
	// searching-only, per-query
	const CSphQuery &				m_tQuery;

	int								m_iWeights = 0;					///< search query field weights count
	int								m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights

	std::unique_ptr<ISphFilter>		m_pFilter;
	std::unique_ptr<ISphFilter>		m_pWeightFilter;

	bool							m_bSkipQCache = false;			///< whether do not cache this query

	CSphVector<ContextCalcItem_t>	m_dCalcFilter;			///< items to compute for filtering
	CSphVector<ContextCalcItem_t>	m_dCalcSort;			///< items to compute for sorting/grouping
	CSphVector<ContextCalcItem_t>	m_dCalcFinal;			///< items to compute when finalizing result set

	IntVec_t						m_dCalcFilterPtrAttrs;	///< items to free after computing filter stage
	IntVec_t						m_dCalcSortPtrAttrs;	///< items to free after computing sort stage

	const void *					m_pIndexData = nullptr;	///< backend specific data
	QueryProfile_c *				m_pProfile = nullptr;
	const SmallStringHash_T<int64_t> * m_pLocalDocs = nullptr;
	int64_t							m_iTotalDocs = 0;
	int64_t							m_iIndexTotalDocs = 0;

	explicit CSphQueryContext ( const CSphQuery & tQuery );
			~CSphQueryContext () { 	ResetFilters(); }

	void	BindWeights ( const CSphQuery & tQuery, const CSphSchema & tSchema, CSphString & sWarning );
	bool	SetupCalc ( CSphQueryResultMeta & tMeta, const ISphSchema & tInSchema, const ISphSchema & tSchema, const BYTE * pBlobPool, const columnar::Columnar_i * pColumnar, const CSphVector<const ISphSchema *> & dInSchemas );
	bool	CreateFilters ( CreateFilterContext_t & tCtx, CSphString &sError, CSphString &sWarning );

	void	CalcFilter ( CSphMatch & tMatch ) const									{ CalcContextItems ( tMatch, m_dCalcFilter ); }
	void	CalcSort ( CSphMatch & tMatch )	const									{ CalcContextItems ( tMatch, m_dCalcSort ); }
	void	CalcFinal ( CSphMatch & tMatch ) const									{ CalcContextItems ( tMatch, m_dCalcFinal ); }
	void	CalcItem ( CSphMatch & tMatch, const ContextCalcItem_t & tCalc ) const	{ CalcContextItem ( tMatch, tCalc ); }

	void	FreeDataFilter ( CSphMatch & tMatch ) const;
	void	FreeDataSort ( CSphMatch & tMatch ) const;

	// note that RT index bind pools at segment searching, not at time it setups context
	void	ExprCommand ( ESphExprCommand eCmd, void * pArg );
	void	SetBlobPool ( const BYTE * pBlobPool );
	void	SetColumnar ( const columnar::Columnar_i * pColumnar );
	void	SetDocstore ( const Docstore_i * pDocstore, int64_t iDocstoreSessionId );

	void	SetupExtraData ( ISphRanker * pRanker, ISphMatchSorter * pSorter );
	void	ResetFilters();

	void	SetPackedFactor ( DWORD uFlags );
	DWORD	GetPackedFactor () const;

private:
	CSphVector<UservarIntSet_c>		m_dUserVals;
	DWORD							m_uPackedFactorFlags { SPH_FACTOR_DISABLE }; ///< whether we need to calculate packed factors (and some extra options)

	void	AddToFilterCalc ( const ContextCalcItem_t & tCalc );
	void	AddToSortCalc ( const ContextCalcItem_t & tCalc );
};
