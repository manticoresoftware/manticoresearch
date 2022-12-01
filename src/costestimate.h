//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _costestimate_
#define _costestimate_

#include "sphinx.h"

class CostEstimate_i
{
public:
	virtual			~CostEstimate_i() = default;
	virtual float	CalcQueryCost() = 0;
};

struct SecondaryIndexInfo_t
{
	CSphVector<SecondaryIndexType_e> m_dCapabilities;
	SecondaryIndexType_e	m_eType = SecondaryIndexType_e::FILTER;
	SecondaryIndexType_e	m_eForce = SecondaryIndexType_e::NONE;
	int64_t					m_iRsetEstimate = 0;
	int64_t					m_iTotalValues = 0;	// total values (used mainly for MVAs; different from total docs)
	bool					m_bCreated = false;
};

namespace SI
{
	class Index_i;
}

struct SelectIteratorCtx_t
{
	const CSphVector<CSphFilterSettings> &	m_dFilters;
	const CSphVector<FilterTreeItem_t> &	m_dFilterTree;
	const CSphVector<IndexHint_t> &			m_dHints;
	const ISphSchema &						m_tSchema;
	const HistogramContainer_c *			m_pHistograms = nullptr;
	columnar::Columnar_i *					m_pColumnar = nullptr;
	SI::Index_i *							m_pSI = nullptr;
	ESphCollation							m_eCollation = SPH_COLLATION_DEFAULT;
	int										m_iCutoff = -1;
	int64_t									m_iTotalDocs = 0;
	int										m_iThreads = 1;

			SelectIteratorCtx_t ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> &	dFilterTree, const CSphVector<IndexHint_t> & dHints, const ISphSchema &	tSchema, const HistogramContainer_c * pHistograms, columnar::Columnar_i * pColumnar, SI::Index_i * pSI, ESphCollation eCollation, int iCutoff, int64_t iTotalDocs, int iThreads );

	bool	IsEnabled_SI ( const CSphFilterSettings & tFilter ) const;
	bool	IsEnabled_Analyzer ( const CSphFilterSettings & tFilter ) const;
};


CostEstimate_i * CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx );
#endif // _costestimate_
