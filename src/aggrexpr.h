//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _aggsexpr_
#define _aggsexpr_

#include "sphinxdefs.h"
#include "sphinxexpr.h"
#include <variant>

using AggrBound_t = std::variant<int64_t, float>;

struct RangeSetting_t
{
	int m_iIdx = 0;
	union
	{
		int64_t			m_iFrom = LLONG_MIN;	///< range from
		float			m_fFrom;
	};
	union
	{
		int64_t			m_iTo = LLONG_MAX;	///< range to
		float			m_fTo;
	};
};

struct AggrRangeSetting_t : public CSphVector<RangeSetting_t>
{
	bool m_bFloat = false;
	bool m_bOpenLeft = false;
	bool m_bOpenRight = false;
	bool m_bKeyed = false;
};

struct AggrHistSetting_t
{
	AggrBound_t m_tInterval;
	AggrBound_t m_tOffset;
	bool m_bFloat = false;
	bool m_bKeyed = false;
};

enum class Aggr_e
{
	NONE,
	SIGNIFICANT,
	HISTOGRAM,
	DATE_HISTOGRAM,
	RANGE,
	DATE_RANGE,
	COMPOSITE,
	COUNT,
	MIN,
	MAX,
	SUM,
	AVG
};

struct DateRangeSetting_t
{
	int m_iIdx = 0;
	CSphString m_sFrom;
	CSphString m_sTo;
};

struct AggrDateRangeSetting_t : public CSphVector<DateRangeSetting_t>
{
	bool m_bKeyed = false;
};

struct AggrDateHistSetting_t
{
	CSphString m_sInterval;
	bool m_bKeyed = false;
	bool m_bFixed = false;
};

struct AggrSettings_t
{
	Aggr_e		m_eAggrFunc { Aggr_e::NONE };
	AggrRangeSetting_t m_tRange;
	AggrHistSetting_t m_tHist;
	AggrDateRangeSetting_t m_tDateRange;
	AggrDateHistSetting_t m_tDateHist;
};

ISphExpr * CreateExprRange ( ISphExpr * pAttr, const AggrRangeSetting_t & tRanges );

bool ParseAggrRange ( const VecTraits_T< VecTraits_T < CSphNamedVariant > > & dRanges, bool bDate, int iNow, AggrRangeSetting_t & tRanges, CSphString & sError );

CSphString GetAggrName ( int iItem, const CSphString & sCol );

struct RangeKeyDesc_t
{
	CSphString m_sKey;
	CSphString m_sFrom;
	CSphString m_sTo;
};
using RangeNameHash_t = CSphOrderedHash<RangeKeyDesc_t, int, IdentityHash_fn, 256>;

void GetRangeKeyNames ( const AggrRangeSetting_t & tRanges, RangeNameHash_t & hRangeNames );
void GetRangeKeyNames ( const AggrDateRangeSetting_t & tRanges, int iNow, RangeNameHash_t & hRangeNames );
void FormatDate ( time_t tDate, CSphString & sRes );
void FormatDate ( time_t tDate, StringBuilder_c & sRes );

ISphExpr * CreateExprHistogram ( ISphExpr * pAttr, const AggrHistSetting_t & tHist );

bool ParseAggrHistogram ( const VecTraits_T < CSphNamedVariant > & dVariants, AggrHistSetting_t & tHist, CSphString & sError );

ISphExpr * CreateExprDateHistogram ( ISphExpr * pAttr, const AggrDateHistSetting_t & tHist );

bool ParseAggrDateHistogram ( const VecTraits_T < CSphNamedVariant > & dVariants, AggrDateHistSetting_t & tHist, CSphString & sError );


CSphString DumpAggr ( const CSphString & sCol, const AggrSettings_t & tAggr );

void FixFloat ( AggrHistSetting_t & tHist );

#endif // _aggsexpr_
