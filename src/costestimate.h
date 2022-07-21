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
	bool					m_bCreated = false;
};

CostEstimate_i * CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, int64_t iTotalDocs, int iCutoff = -1 );

#endif // _costestimate_
