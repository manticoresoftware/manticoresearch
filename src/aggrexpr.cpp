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

#include <cfloat>
#include <climits>
#include <math.h>

#include "datetime.h"
#include "exprtraits.h"
#include "sphinxjsonquery.h"
#include "sphinxint.h"
#include "aggrexpr.h"

// the aggr range implementation
template < bool FLOAT >
class AggrRangeExpr_T : public Expr_ArgVsSet_T<int>
{
	AggrRangeSetting_t m_tRanges;

public:
	AggrRangeExpr_T ( ISphExpr * pAttr, const AggrRangeSetting_t & tRanges )
		: Expr_ArgVsSet_T ( pAttr )
		, m_tRanges ( tRanges )
	{
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iBucket = GetBucket ( tMatch );
		return iBucket;
	}

protected:
	int GetBucket ( const CSphMatch & tMatch ) const
	{
		if_const ( FLOAT )
		{
			double fVal = m_pArg->Eval ( tMatch );
			if ( m_tRanges.m_bOpenLeft && fVal<m_tRanges.First().m_fTo )
				return m_tRanges.First().m_iIdx;
			if ( m_tRanges.m_bOpenRight && fVal>=m_tRanges.Last().m_fFrom )
				return m_tRanges.Last().m_iIdx;

			if ( !m_tRanges.m_bOpenLeft && fVal<m_tRanges.First().m_fFrom )
				return m_tRanges.GetLength();
			if ( !m_tRanges.m_bOpenRight && fVal>=m_tRanges.Last().m_fTo )
				return m_tRanges.GetLength();

			int iItem = m_tRanges.GetFirst ([&](const RangeSetting_t& tRange) { return (tRange.m_fFrom<=fVal && fVal<tRange.m_fTo); });
			if ( iItem==-1 )
				return	m_tRanges.GetLength();
			return m_tRanges[iItem].m_iIdx;
		} else
		{
			int64_t iVal = m_pArg->Int64Eval ( tMatch );

			if ( m_tRanges.m_bOpenLeft && iVal<m_tRanges.First().m_iTo )
				return m_tRanges.First().m_iIdx;
			if ( m_tRanges.m_bOpenRight && iVal>=m_tRanges.Last().m_iFrom )
				return m_tRanges.Last().m_iIdx;

			if ( !m_tRanges.m_bOpenLeft && iVal<m_tRanges.First().m_iFrom )
				return m_tRanges.GetLength();
			if ( !m_tRanges.m_bOpenRight && iVal>=m_tRanges.Last().m_iTo )
				return m_tRanges.GetLength();

			int iItem = m_tRanges.GetFirst ( [&] ( const RangeSetting_t & tRange ) { return ( tRange.m_iFrom<=iVal && iVal<tRange.m_iTo ); } );
			if ( iItem==-1 )
				return	m_tRanges.GetLength();
			return m_tRanges[iItem].m_iIdx;
		}
	}


	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("AggrRangeExpr_T");
		CALC_POD_HASH ( m_tRanges );
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone() const final
	{
		return new AggrRangeExpr_T ( *this );
	}

private:
	AggrRangeExpr_T ( const AggrRangeExpr_T & rhs )
		: Expr_ArgVsSet_T ( rhs )
		, m_tRanges ( rhs.m_tRanges )
	{
	}
};

ISphExpr * CreateExprRange ( ISphExpr * pAttr, const AggrRangeSetting_t & tRanges )
{
	if ( tRanges.m_bFloat )
		return new AggrRangeExpr_T<true> ( pAttr, tRanges );
	else
		return new AggrRangeExpr_T<false> ( pAttr, tRanges );
}

static void DumpRange ( const RangeSetting_t & tRange, bool bFloat, StringBuilder_c & sRes )
{
	if ( bFloat )
		sRes.Appendf ( ", {range_from=%f, range_to=%f}", tRange.m_fFrom, tRange.m_fTo );
	else
		sRes.Appendf ( ", {range_from=" INT64_FMT ", range_to=" INT64_FMT "}", tRange.m_iFrom, tRange.m_iTo );
}

static void DumpRangeOpenLeft ( const RangeSetting_t & tRange, bool bFloat, StringBuilder_c & sRes )
{
	if ( bFloat )
		sRes.Appendf ( ", {range_to=%f}", tRange.m_fTo );
	else
		sRes.Appendf ( ", {range_to=" INT64_FMT "}", tRange.m_iTo );
}

static void DumpRangeOpenRigth ( const RangeSetting_t & tRange, bool bFloat, StringBuilder_c & sRes )
{
	if ( bFloat )
		sRes.Appendf ( ", {range_from=%f}", tRange.m_fFrom );
	else
		sRes.Appendf ( ", {range_from=" INT64_FMT "}", tRange.m_iFrom );
}

static void DumpRange ( int iItem, const AggrRangeSetting_t & tRanges, StringBuilder_c & sRes )
{
	const bool bFloat = tRanges.m_bFloat;
	const RangeSetting_t & tRange = tRanges[iItem];

	if ( iItem==0 && tRanges.m_bOpenLeft )
		DumpRangeOpenLeft ( tRange, bFloat, sRes );
	else if ( iItem==tRanges.GetLength()-1 && tRanges.m_bOpenRight )
		DumpRangeOpenRigth ( tRange, bFloat, sRes );
	else
		DumpRange ( tRange, bFloat, sRes );
}

static CSphString DumpAggrRange ( const CSphString & sCol, const AggrRangeSetting_t & tRanges )
{
	assert ( tRanges.GetLength() );

	StringBuilder_c sRes;
	sRes.Appendf ( "range(%s", sCol.cstr() );

	ARRAY_FOREACH ( i, tRanges )
		DumpRange ( i, tRanges, sRes );

	sRes += ")";
	return CSphString ( sRes );
}

static CSphString DumpAggrRange ( const CSphString & sCol, const AggrDateRangeSetting_t & tRanges )
{
	assert ( tRanges.GetLength() );

	StringBuilder_c sRes;
	sRes.Appendf ( "date_range(%s", sCol.cstr() );

	for ( DateRangeSetting_t & tRange : tRanges )
	{
		if ( !tRange.m_sFrom.IsEmpty() && !tRange.m_sTo.IsEmpty() )
			sRes.Appendf ( ", {range_from='%s', range_to='%s'}", tRange.m_sFrom.cstr(), tRange.m_sTo.cstr() );
		else if ( tRange.m_sFrom.IsEmpty() )
			sRes.Appendf ( ", {range_to='%s'}", tRange.m_sTo.cstr() );
		else
			sRes.Appendf ( ", {range_from='%s'}", tRange.m_sFrom.cstr() );
	}

	sRes += ")";
	return CSphString ( sRes );
}


bool ParseAggrRange ( const VecTraits_T< VecTraits_T < CSphNamedVariant > > & dSrcRanges, bool bDate, int iNow, AggrRangeSetting_t & tRanges, CSphString & sError )
{
	if ( dSrcRanges.IsEmpty() )
	{
		sError = "at least 1 range expected";
		return false;
	}

	ARRAY_FOREACH ( iItem, dSrcRanges )
	{
		const auto & dItem = dSrcRanges[iItem];
		if ( dItem.IsEmpty() )
		{
			sError.SetSprintf ( "empty range %d", iItem );
			return false;
		}

		bool bHasFrom = false;
		bool bHasTo = false;
		bool bFloatFrom = false;
		bool bFloatTo = false;

		auto & tRange = tRanges.Add();
		tRange.m_iIdx = iItem;

		ARRAY_FOREACH ( iVal, dItem )
		{
			const auto & tVal = dItem[iVal];
			if ( tVal.m_sKey=="range_from" )
			{
				if ( tVal.m_eType==VariantType_e::BIGINT )
					tRange.m_iFrom = tVal.m_iValue;
				else if ( tVal.m_eType==VariantType_e::FLOAT )
				{
					tRange.m_fFrom = tVal.m_fValue;
					bFloatFrom = true;

				} else if ( tVal.m_eType==VariantType_e::STRING )
				{
					time_t tFrom = 0;
					if ( !ParseDateMath ( tVal.m_sValue, iNow, tFrom ) )
					{
						sError.SetSprintf ( "date_range invalid from value '%s'", tVal.m_sValue.cstr() );
						return false;
					}
					tRange.m_iFrom = tFrom;
				} else
				{
					sError.SetSprintf ( "%s %d invalid value type %d", ( bDate ? "date_range" : "range" ), iItem, (int)tVal.m_eType );
					return false;
				}
				bHasFrom = true;

			} else if ( tVal.m_sKey=="range_to" )
			{
				if ( tVal.m_eType==VariantType_e::BIGINT )
					tRange.m_iTo = tVal.m_iValue;
				else if ( tVal.m_eType==VariantType_e::FLOAT )
				{
					tRange.m_fTo = tVal.m_fValue;
					bFloatTo = true;

				} else if ( tVal.m_eType==VariantType_e::STRING )
				{
					time_t tTo = 0;
					if ( !ParseDateMath ( tVal.m_sValue, iNow, tTo ) )
					{
						sError.SetSprintf ( "date_range invalid to value '%s'", tVal.m_sValue.cstr() );
						return false;
					}
					tRange.m_iTo = tTo;
				} else
				{
					sError.SetSprintf ( "%s %d invalid value type %d", ( bDate ? "date_range" : "range" ), iItem, (int)tVal.m_eType );
					return false;
				}
				bHasTo = true;
			}
		}

		if ( !bHasFrom && !bHasTo )
		{
			sError.SetSprintf ( "empty %s %d", ( bDate ? "date_range" : "range" ), iItem );
			return false;
		}

		if ( !bHasFrom )
		{
			tRanges.m_bOpenLeft = true;
			if ( bFloatFrom || bFloatTo || tRanges.m_bFloat )
				tRange.m_fFrom = -FLT_MAX;
			else
				tRange.m_iFrom = INT64_MIN;
		}
		if ( !bHasTo )
		{
			tRanges.m_bOpenRight = true;
			if ( bFloatFrom || bFloatTo || tRanges.m_bFloat )
				tRange.m_fTo = FLT_MAX;
			else
				tRange.m_iTo = INT64_MAX;
		}

		// convert both values to float
		if ( bFloatFrom^bFloatTo )
		{
			if ( bFloatFrom )
				tRange.m_fTo = tRange.m_iTo;
			else 
				tRange.m_fFrom = tRange.m_iFrom;

		} else if ( tRanges.m_bFloat && !( bFloatFrom && bFloatTo ) )
		{
			tRange.m_fTo = tRange.m_iTo;
			tRange.m_fFrom = tRange.m_iFrom;
		}

		// convert all prevoiuse values into floats
		if ( ( bFloatFrom || bFloatTo ) && !tRanges.m_bFloat )
		{
			if ( tRanges.GetLength()>1 )
			{
				tRanges.Slice( 0, tRanges.GetLength()-1 ).for_each ( [] ( auto & tRange )
				{
					tRange.m_fFrom = tRange.m_iFrom;
					tRange.m_fTo = tRange.m_iTo;
				} );
				if ( tRanges.m_bOpenLeft )
					tRanges[0].m_fFrom = -FLT_MAX;
			}

			tRanges.m_bFloat = true;
		}

		if ( tRanges.m_bFloat )
			tRanges.Sort ( ::bind ( &RangeSetting_t::m_fFrom ) );
		else
			tRanges.Sort ( ::bind ( &RangeSetting_t::m_iFrom ) );
	}

	return true;


}

CSphString GetAggrName ( int iItem, const CSphString & sCol )
{
	CSphString sName;
	sName.SetSprintf ( "aggs_%d_%s", iItem, sCol.cstr() );
	return sName;
}

static void FormatKeyFloat ( const RangeSetting_t & tRange, bool bHasFrom, bool bHasTo, RangeKeyDesc_t & tDesc )
{
	assert ( bHasFrom || bHasTo );
	if ( bHasFrom )
		tDesc.m_sFrom.SetSprintf ( "%f", tRange.m_fFrom );
	if ( bHasTo )
		tDesc.m_sTo.SetSprintf ( "%f", tRange.m_fTo );

	if ( bHasFrom && bHasTo )
		tDesc.m_sKey.SetSprintf ( "%f-%f", tRange.m_fFrom, tRange.m_fTo );
	else if ( bHasTo )
		tDesc.m_sKey.SetSprintf ( "*-%f", tRange.m_fTo );
	else
		tDesc.m_sKey.SetSprintf ( "%f-*", tRange.m_fFrom );

}

static void FormatKeyInt ( const RangeSetting_t & tRange, bool bHasFrom, bool bHasTo, RangeKeyDesc_t & tDesc )
{
	assert ( bHasFrom || bHasTo );
	if ( bHasFrom )
		tDesc.m_sFrom.SetSprintf ( INT64_FMT, tRange.m_iFrom );
	if ( bHasTo )
		tDesc.m_sTo.SetSprintf ( INT64_FMT, tRange.m_iTo );

	if ( bHasFrom && bHasTo )
		tDesc.m_sKey.SetSprintf ( INT64_FMT "-" INT64_FMT, tRange.m_iFrom, tRange.m_iTo );
	else if ( bHasTo )
		tDesc.m_sKey.SetSprintf ( "*-" INT64_FMT, tRange.m_iTo );
	else
		tDesc.m_sKey.SetSprintf ( INT64_FMT "-*", tRange.m_iFrom );
}

static void FormatDate ( const CSphString & sVal, int iNow, CSphString & sRes )
{
	time_t tSrcDate;
	Verify ( ParseDateMath ( sVal, iNow, tSrcDate ) );

	FormatDate ( tSrcDate, sRes );
}

static const char * g_sCompatDateFormat = "%Y-%m-%dT%H:%M:%S"; // YYYY-mm-dd'T'HH:mm:ss.SSS'Z'

static void FormatDate ( time_t tDate, char * sBuf, int iSize )
{
	std::tm tDstDate;
	gmtime_r ( &tDate, &tDstDate );

	[[maybe_unused]] auto tResult = strftime ( sBuf, iSize, g_sCompatDateFormat, &tDstDate );
	assert ( tResult>0 );
}

void FormatDate ( time_t tDate, CSphString & sRes )
{
	char sBuf[128];
	FormatDate  ( tDate, sBuf, sizeof(sBuf)-1 );
	sRes = sBuf;
}


void FormatDate ( time_t tDate, StringBuilder_c & sRes )
{
	char sBuf[128];
	FormatDate  ( tDate, sBuf, sizeof(sBuf)-1 );
	sRes.Appendf ( "%s", sBuf );
}

static void FormatKeyDate ( const DateRangeSetting_t & tRange, int iNow, RangeKeyDesc_t & tDesc )
{
	CSphString sFrom;
	CSphString sTo;

	const bool bHasFrom = !tRange.m_sFrom.IsEmpty();
	const bool bHasTo = !tRange.m_sTo.IsEmpty();
	assert ( bHasFrom || bHasTo );
	assert ( iNow>0 );

	if ( bHasFrom )
	{
		FormatDate ( tRange.m_sFrom, iNow, sFrom );
		tDesc.m_sFrom.SetSprintf ( "%s", sFrom.cstr() );
	}
	if ( bHasTo )
	{
		FormatDate ( tRange.m_sTo, iNow, sTo );
		tDesc.m_sTo.SetSprintf ( "%s", sTo.cstr() );
	}

	if ( bHasFrom && bHasTo )
		tDesc.m_sKey.SetSprintf ( "%s-%s", sFrom.cstr(), sTo.cstr() );
	else if ( bHasTo )
		tDesc.m_sKey.SetSprintf ( "*-%s", sTo.cstr() );
	else
		tDesc.m_sKey.SetSprintf ( "%s-*", sFrom.cstr() );
}

void GetRangeKeyNames ( const AggrRangeSetting_t & tRanges, RangeNameHash_t & hRangeNames )
{
	if ( tRanges.GetLength()==1 && tRanges.m_bOpenLeft && tRanges.m_bOpenRight )
	{
		auto & tDesc = hRangeNames.AddUnique ( 0 );
		tDesc.m_sKey = "*-*";
		return;
	}

	ARRAY_FOREACH ( i, tRanges )
	{
		const auto & tSrc = tRanges[i];
		auto & tDesc = hRangeNames.AddUnique ( i );

		bool bHasFrom = true;
		bool bHasTo = true;
		if ( i==0 && tRanges.m_bOpenLeft )
			bHasFrom = false;
		else if ( i==tRanges.GetLength()-1 && tRanges.m_bOpenRight )
			bHasTo = false;

		if ( tRanges.m_bFloat )
			FormatKeyFloat ( tSrc, bHasFrom, bHasTo, tDesc );
		else
			FormatKeyInt ( tSrc, bHasFrom, bHasTo, tDesc );
	}
}

void GetRangeKeyNames ( const AggrDateRangeSetting_t & tRanges, int iNow, RangeNameHash_t & hRangeNames )
{
	ARRAY_FOREACH ( i, tRanges )
	{
		const auto & tSrc = tRanges[i];
		auto & tDesc = hRangeNames.AddUnique ( i );
		FormatKeyDate ( tSrc, iNow, tDesc );
	}
}

static CSphString DumpAggrHist ( const CSphString & sCol, const AggrHistSetting_t & tHist )
{
	StringBuilder_c sRes;
	sRes.Appendf ( "histogram(%s, {", sCol.cstr() );

	if ( tHist.m_bFloat )
		sRes.Appendf ( "hist_interval=%f, hist_offset=%f", std::get<float>( tHist.m_tInterval ), std::get<float> ( tHist.m_tOffset ) );
	else
		sRes.Appendf ( "hist_interval=" INT64_FMT ", hist_offset=" INT64_FMT, std::get<int64_t>( tHist.m_tInterval ), std::get<int64_t>( tHist.m_tOffset ) );

	sRes += "})";
	return CSphString ( sRes );
}

static CSphString DumpAggrHist ( const CSphString & sCol, const AggrDateHistSetting_t & tHist )
{
	StringBuilder_c sRes;
	sRes.Appendf ( "date_histogram(%s, {", sCol.cstr() );
	if ( tHist.m_bFixed )
		sRes.Appendf ( "fixed_interval='%s'", tHist.m_sInterval.cstr() );
	else
		sRes.Appendf ( "calendar_interval='%s'", tHist.m_sInterval.cstr() );
	sRes += "})";
	return CSphString ( sRes );
}

CSphString DumpAggr ( const CSphString & sCol, const AggrSettings_t & tAggr )
{
	switch ( tAggr.m_eAggrFunc )
	{
	case Aggr_e::RANGE: return DumpAggrRange ( sCol, tAggr.m_tRange );
	case Aggr_e::DATE_RANGE: return DumpAggrRange ( sCol, tAggr.m_tDateRange );
	case Aggr_e::HISTOGRAM: return DumpAggrHist ( sCol, tAggr.m_tHist );
	case Aggr_e::DATE_HISTOGRAM: return DumpAggrHist ( sCol, tAggr.m_tDateHist );

	default: return sCol;
	}
}

static void ConvertIntoFloat ( AggrBound_t & tVal )
{
	if ( std::holds_alternative<int64_t> ( tVal ) )
		tVal = (float)std::get<int64_t> ( tVal );
}

void FixFloat ( AggrHistSetting_t & tHist )
{
	if ( tHist.m_tInterval.index()!=tHist.m_tOffset.index() )
	{
		ConvertIntoFloat ( tHist.m_tInterval );
		ConvertIntoFloat ( tHist.m_tOffset );
		tHist.m_bFloat = true;
	} else
	{
		tHist.m_bFloat = std::holds_alternative<float> ( tHist.m_tInterval );
	}
}

static void SetValue ( const CSphNamedVariant & tPair, AggrBound_t & tVal )
{
	switch ( tPair.m_eType )
	{
	case VariantType_e::BIGINT: tVal = tPair.m_iValue; return;
	case VariantType_e::FLOAT: tVal = tPair.m_fValue; return;
	default: assert ( 0 && "internal error: unhandled aggregate value" ); return;
	}
}

bool ParseAggrHistogram ( const VecTraits_T < CSphNamedVariant > & dVariants, AggrHistSetting_t & tHist, CSphString & sError )
{
	for ( const auto & tPair : dVariants )
	{
		if ( tPair.m_eType!=VariantType_e::BIGINT && tPair.m_eType!=VariantType_e::FLOAT )
		{
			sError.SetSprintf ( "invalid value '%s' type %d", tPair.m_sKey.cstr(), (int)tPair.m_eType );
			return false;
		}

		if ( tPair.m_sKey=="hist_interval" )
			SetValue ( tPair, tHist.m_tInterval );
		else if ( tPair.m_sKey=="hist_offset" )
			SetValue ( tPair, tHist.m_tOffset );
		else
		{
			sError.SetSprintf ( "unknow value '%s'", tPair.m_sKey.cstr() );
			return false;
		}
	}
	FixFloat ( tHist );

	return true;
}

bool ParseAggrDateHistogram ( const VecTraits_T < CSphNamedVariant > & dVariants, AggrDateHistSetting_t & tHist, CSphString & sError )
{
	for ( const auto & tPair : dVariants )
	{
		if ( tPair.m_eType!=VariantType_e::STRING )
		{
			sError.SetSprintf ( "invalid value '%s' type %d", tPair.m_sKey.cstr(), (int)tPair.m_eType );
			return false;
		}

		if ( tPair.m_sKey=="calendar_interval" || tPair.m_sKey=="fixed_interval" )
		{
			tHist.m_sInterval = tPair.m_sValue;
			tHist.m_bFixed = ( tPair.m_sKey=="fixed_interval" );

		} else
		{
			sError.SetSprintf ( "unknow value '%s'", tPair.m_sKey.cstr() );
			return false;
		}
	}

	if ( tHist.m_sInterval.IsEmpty() )
	{
		sError.SetSprintf ( "calendar_interval missed" );
		return false;
	}

	DateUnit_e eUnit = ParseDateInterval ( tHist.m_sInterval, tHist.m_bFixed, sError ).first;
	if ( eUnit==DateUnit_e::total_units )
		return false;

	return true;
}

// the aggr histogram implementation
template < bool FLOAT >
class AggrHistExpr_T : public Expr_ArgVsSet_T<int>
{
	AggrHistSetting_t m_tHist;

public:
	AggrHistExpr_T ( ISphExpr * pAttr, const AggrHistSetting_t & tHist )
		: Expr_ArgVsSet_T ( pAttr )
		, m_tHist ( tHist )
	{
		assert ( ( m_tHist.m_bFloat && std::get<float> ( m_tHist.m_tInterval )>0.0f ) || ( !m_tHist.m_bFloat && std::get<int64_t> ( m_tHist.m_tInterval )>0 ) );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iBucket = GetBucket ( tMatch );
		return iBucket;
	}

protected:
	int GetBucket ( const CSphMatch & tMatch ) const
	{
		if_const ( FLOAT )
		{
			float fInterval = std::get<float> ( m_tHist.m_tInterval );
			float fOffset = std::get<float> ( m_tHist.m_tOffset );

			float fVal = m_pArg->Eval ( tMatch );
			float fBucketNrm = floor ( ( fVal - fOffset ) / fInterval );
			int iBucket = fBucketNrm * fInterval + fOffset;
			return iBucket;
		} else
		{
			int64_t iInterval = std::get<int64_t> ( m_tHist.m_tInterval );
			int64_t iOffset = std::get<int64_t> ( m_tHist.m_tOffset );

			int64_t iVal = m_pArg->Int64Eval ( tMatch );
			double fBucketNrm = floor ( double ( iVal - iOffset ) / iInterval );
			int iBucket = fBucketNrm * iInterval + iOffset;
			return iBucket;
		}
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("AggrHistExpr_T");
		CALC_POD_HASH ( m_tHist );
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone() const final
	{
		return new AggrHistExpr_T ( *this );
	}

private:
	AggrHistExpr_T ( const AggrHistExpr_T & rhs )
		: Expr_ArgVsSet_T ( rhs )
		, m_tHist ( rhs.m_tHist )
	{
	}
};

ISphExpr * CreateExprHistogram ( ISphExpr * pAttr, const AggrHistSetting_t & tHist )
{
	if ( tHist.m_bFloat )
		return new AggrHistExpr_T<true> ( pAttr, tHist );
	else
		return new AggrHistExpr_T<false> ( pAttr, tHist );
}

// the aggr date histogram implementation
template < bool FIXED_INTERVAL >
class AggrDateHistExpr_T : public Expr_ArgVsSet_T<int>
{
	AggrDateHistSetting_t m_tHist;
	DateUnit_e m_eUnit = DateUnit_e::total_units;
	int m_iMulti = 1;

public:
	AggrDateHistExpr_T ( ISphExpr * pAttr, const AggrDateHistSetting_t & tHist )
		: Expr_ArgVsSet_T ( pAttr )
		, m_tHist ( tHist )
	{
		CSphString sError;
		auto[eUnit, iMulti] = ParseDateInterval ( tHist.m_sInterval, tHist.m_bFixed, sError );
		
		assert ( eUnit!=DateUnit_e::total_units );
		m_eUnit = eUnit;
		m_iMulti = iMulti;
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iBucket = GetBucket ( tMatch );
		return iBucket;
	}

protected:
	int GetBucket ( const CSphMatch & tMatch ) const
	{
		time_t iVal = m_pArg->IntEval ( tMatch );

		if_const ( FIXED_INTERVAL ) 
			RoundDate ( m_eUnit, m_iMulti, iVal );
		else
			RoundDate ( m_eUnit, iVal );

		return iVal;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("AggrDateHistExpr_T");
		CALC_POD_HASH ( m_tHist );
		CALC_POD_HASH ( m_eUnit );
		CALC_POD_HASH ( m_iMulti );
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone() const final
	{
		return new AggrDateHistExpr_T ( *this );
	}

private:
	AggrDateHistExpr_T ( const AggrDateHistExpr_T & rhs )
		: Expr_ArgVsSet_T ( rhs )
		, m_tHist ( rhs.m_tHist )
		, m_eUnit ( rhs.m_eUnit )
		, m_iMulti ( rhs.m_iMulti )
	{
	}
};

ISphExpr * CreateExprDateHistogram ( ISphExpr * pAttr, const AggrDateHistSetting_t & tHist )
{
	if ( tHist.m_bFixed )
		return new AggrDateHistExpr_T<true> ( pAttr, tHist );
	else
		return new AggrDateHistExpr_T<false> ( pAttr, tHist );
}