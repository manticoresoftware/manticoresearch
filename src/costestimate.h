//
// Copyright (c) 2018-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _costestimate_
#define _costestimate_

#include "sphinxfilter.h"

class CostEstimate_i
{
public:
	virtual			~CostEstimate_i() = default;
	virtual float	CalcQueryCost() = 0;
};

float EstimateMTCost ( float fCost, int iThreads );
float EstimateMTCostCS ( float fCost, int iThreads );
float EstimateMTCostSI ( float fCost, int iThreads );
float EstimateMTCostSIFT ( float fCost, int iThreads );

struct SecondaryIndexInfo_t
{
	CSphVector<SecondaryIndexType_e> m_dCapabilities;
	SecondaryIndexType_e	m_eType = SecondaryIndexType_e::FILTER;
	SecondaryIndexType_e	m_eForce = SecondaryIndexType_e::NONE;
	int64_t					m_iRsetEstimate = 0;
	int64_t					m_iTotalValues = 0;	// total values (used mainly for MVAs; different from total docs)
	int64_t					m_iPartialColumnarMinMax = -1;
	uint32_t				m_uNumSIIterators = 0;
	bool					m_bHasHistograms = false;
	bool					m_bUsable = false;
	bool					m_bCreated = false;
};

class SIContainer_c;

struct SelectIteratorCtx_t
{
	const CSphQuery &						m_tQuery;
	const CSphVector<CSphFilterSettings> &	m_dFilters;
	const ISphSchema &						m_tIndexSchema;
	const ISphSchema &						m_tSorterSchema;
	const HistogramContainer_c *			m_pHistograms = nullptr;
	columnar::Columnar_i *					m_pColumnar = nullptr;
	const SIContainer_c &					m_tSI;
	int										m_iCutoff = -1;
	int64_t									m_iTotalDocs = 0;
	int										m_iThreads = 1;
	bool									m_bCalcPushCost = true;
	bool									m_bFromIterator = false;
	float									m_fDocsLeft = 1.0f;

			SelectIteratorCtx_t ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, const HistogramContainer_c * pHistograms, columnar::Columnar_i * pColumnar, const SIContainer_c & tSI, int iCutoff, int64_t iTotalDocs, int iThreads );

	bool	IsEnabled_SI ( const CSphFilterSettings & tFilter ) const;
	bool	IsEnabled_Analyzer ( const CSphFilterSettings & tFilter ) const;
	void	IgnorePushCost() { m_bCalcPushCost = false; }
};

// Relative filter evaluation complexity; a simple scalar filter has complexity 1.
float CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx );

class SortBound_c;

struct SortBoundScanPlan_t
{
	RowIdBoundaries_t	m_tRange;
	int					m_iBlockRows = 0;

	CSphAttrLocator		m_tLocator;
	int					m_iStride = 0;
	const DWORD *		m_pMinMax = nullptr;
	const columnar::BlockBounds_i * m_pColumnarBounds = nullptr;
	int					m_iLeafLevel = 0;

	bool				IsColumnar() const { return !!m_pColumnarBounds; }
	SphAttr_t			GetBlockBound ( RowID_t tBlock, bool bMax ) const;
};

// Select a sort-bound fullscan and retain the block bounds needed to spawn it.
std::optional<SortBoundScanPlan_t> PlanSortBoundScan ( const SelectIteratorCtx_t & tSelect, const CreateFilterContext_t & tFlx, const CSphQueryContext & tCtx, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, const CSphIndex & tIndex, const DWORD * pMinMax, bool bSelectIterators, SortBound_c & tBound, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning, const char * & szReason );

struct NodeEstimate_t;

CostEstimate_i *	CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx, int iCutoff );
float				CalcFTIntersectCost ( const NodeEstimate_t & tEst1, const NodeEstimate_t & tEst2, int64_t iTotalDocs, int iDocsPerBlock1, int iDocsPerBlock2 );
int64_t				EstimateFilterSelectivity ( const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx );
int64_t				EstimateFilterSelectivity ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> * pFilterTree, const CreateFilterContext_t & tCtx );

#endif // _costestimate_
