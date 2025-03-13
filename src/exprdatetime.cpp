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

#include "exprdatetime.h"
#include "exprtraits.h"

static bool g_bUseUTC = false;

static FORCE_INLINE int FormatTime ( time_t tTime, const BYTE ** ppStr )
{
	cctz::civil_second tLocTime = ConvertTime(tTime);

	CSphString sVal;
	sVal.SetSprintf ( "%02d:%02d:%02d", tLocTime.hour(), tLocTime.minute(), tLocTime.second() );

	int iLength = sVal.Length();
	*ppStr = (const BYTE*) sVal.Leak();
	return iLength;
}


static FORCE_INLINE int FormatDate ( time_t tTime, const BYTE ** ppStr )
{
	cctz::civil_second tLocTime = ConvertTime(tTime);

	CSphString sVal;
	sVal.SetSprintf ( "%04d-%02d-%02d", (int)tLocTime.year(), tLocTime.month(), tLocTime.day() );

	int iLength = sVal.Length();
	*ppStr = (const BYTE*) sVal.Leak();
	return iLength;
}


static FORCE_INLINE int FormatDayName ( time_t tTime, const BYTE ** ppStr )
{
	static const char * dWeekDays[] = 
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
	};

	CSphString sVal = dWeekDays[GetWeekDay ( ConvertTime(tTime), true ) - 1];
	int iLength = sVal.Length();
	*ppStr = (const BYTE*) sVal.Leak();
	return iLength;
}


static FORCE_INLINE int FormatMonthName ( time_t tTime, const BYTE ** ppStr )
{
	static const char * dMonths[] = 
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	CSphString sVal = dMonths[ConvertTime(tTime).month() - 1];
	int iLength = sVal.Length();
	*ppStr = (const BYTE*) sVal.Leak();
	return iLength;
}

/////////////////////////////////////////////////////////////////////

template<bool UTC>
class TimeTraits_T
{
protected:
	FORCE_INLINE cctz::civil_second GetConvertedTime ( ISphExpr * pExpr, const CSphMatch & tMatch ) const { return ConvertGroupbyTime<UTC> ( (time_t)pExpr->Int64Eval(tMatch) ); }
};

template<bool UTC>
class Expr_TimeTraits_T : public Expr_Unary_c, public TimeTraits_T<UTC>
{
	using Expr_Unary_c::Expr_Unary_c;

public:
	float	Eval ( const CSphMatch & tMatch ) const final		{ return (float)Int64Eval(tMatch); }
	int		IntEval ( const CSphMatch & tMatch ) const final	{ return (int)Int64Eval(tMatch); }

protected:
	FORCE_INLINE cctz::civil_second GetConvertedTime ( const CSphMatch & tMatch ) const { return TimeTraits_T<UTC>::GetConvertedTime ( m_pFirst, tMatch ); }
};

template<bool UTC>
class Expr_TimeTraitsBinary_T : public Expr_Binary_c, public TimeTraits_T<UTC>
{
	using Expr_Binary_c::Expr_Binary_c;

public:
	float	Eval ( const CSphMatch & tMatch ) const final		{ return (float)Int64Eval(tMatch); }
	int		IntEval ( const CSphMatch & tMatch ) const final	{ return (int)Int64Eval(tMatch); }

protected:
	FORCE_INLINE cctz::civil_second GetConvertedTime ( const CSphMatch & tMatch ) const { return TimeTraits_T<UTC>::GetConvertedTime ( m_pFirst, tMatch ); }
};


class Expr_Now_c : public Expr_NoLocator_c
{
public:
			Expr_Now_c ( int iNow ) : m_iNow ( iNow ) {}

	int		IntEval ( const CSphMatch & ) const final { return m_iNow; }
	float	Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)IntEval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Now_c");
		CALC_POD_HASH(m_iNow);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final { return new Expr_Now_c ( *this ); }

private:
	int		m_iNow {0};
			Expr_Now_c ( const Expr_Now_c & rhs) : m_iNow (rhs.m_iNow) {}
};


template<bool UTC, bool DATE>
class Expr_CurTime_T : public ISphExpr
{
public:
	int IntEval ( const CSphMatch & ) const final
	{
		time_t tTime = time(nullptr);
		if constexpr ( UTC )
			return tTime;

		return PackLocalTimeAsUTC(tTime);
	}

	int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const override
	{
		cctz::civil_second tLocTime;
		time_t tTime = time(nullptr);
		if constexpr ( UTC )
			tLocTime = ConvertTimeUTC(tTime);
		else
			tLocTime = ConvertTime(tTime);

		CSphString sVal;
		if constexpr ( DATE )
			sVal.SetSprintf ( "%04d-%02d-%02d %02d:%02d:%02d", (int)tLocTime.year(), tLocTime.month(), tLocTime.day(), tLocTime.hour(), tLocTime.minute(), tLocTime.second() );
		else
			sVal.SetSprintf ( "%02d:%02d:%02d", tLocTime.hour(), tLocTime.minute(), tLocTime.second() );

		int iLength = sVal.Length();
		*ppStr = (const BYTE*) sVal.Leak();
		return iLength;
	}

	float		Eval ( const CSphMatch & tMatch ) const final		{ return (float)IntEval ( tMatch ); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)IntEval ( tMatch ); }
	bool		IsDataPtrAttr () const final						{ return true; }
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final {}
	ISphExpr *	Clone() const override								{ return new Expr_CurTime_T; }

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true;
		return 0;
	}
};


class Expr_CurDate_c : public Expr_CurTime_T<false,false>
{
public:
	int			StringEval ( const CSphMatch &, const BYTE ** ppStr ) const final { return FormatDate ( time(nullptr), ppStr ); }
	ISphExpr *	Clone() const final { return new Expr_CurDate_c; }
};


class Expr_Time_c : public Expr_TimeTraits_T<false>
{
	using BASE = Expr_TimeTraits_T<false>;
	using BASE::BASE;

public:
				Expr_Time_c ( ISphExpr * pExpr ) : BASE ( "Expr_Time_c", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return m_pFirst->IntEval(tMatch); }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override	{ return FormatTime ( m_pFirst->IntEval(tMatch), ppStr ); }
	bool		IsDataPtrAttr () const final	{ return true; }
	ISphExpr *	Clone() const override			{ return new Expr_Time_c(*this); }
};


class Expr_Date_c : public Expr_TimeTraits_T<false>
{
	using BASE = Expr_TimeTraits_T<false>;
	using BASE::BASE;

public:
				Expr_Date_c ( ISphExpr * pExpr ) : BASE ( "Expr_Date_c", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final							{ return m_pFirst->IntEval(tMatch); }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override	{ return FormatDate ( m_pFirst->IntEval(tMatch), ppStr ); }
	bool		IsDataPtrAttr () const final												{ return true; }
	ISphExpr *	Clone() const override														{ return new Expr_Date_c(*this); }
};


class Expr_StringUnary_c : public Expr_Unary_c
{
	using Expr_Unary_c::Expr_Unary_c;

public:
	float		Eval ( const CSphMatch & ) const final { assert ( 0 && "one just does not simply evaluate a string as float" ); return 0; }
	int			IntEval ( const CSphMatch & ) const final { assert ( 0 && "one just does not simply evaluate a string as int" ); return 0; }
	int64_t		Int64Eval ( const CSphMatch & ) const final { assert ( 0 && "one just does not simply evaluate a string as bigint" ); return 0; }
	bool		IsDataPtrAttr () const final { return true; }
};


class Expr_DayName_c : public Expr_StringUnary_c
{
	using Expr_StringUnary_c::Expr_StringUnary_c;

public:
				Expr_DayName_c ( ISphExpr * pExpr ) : Expr_StringUnary_c ( "Expr_DayName_c", pExpr ) {}

	ISphExpr *	Clone() const override														{ return new Expr_DayName_c(*this); }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override	{ return FormatDayName ( m_pFirst->IntEval(tMatch), ppStr ); }
};


class Expr_MonthName_c : public Expr_StringUnary_c
{
	using Expr_StringUnary_c::Expr_StringUnary_c;

public:
				Expr_MonthName_c ( ISphExpr * pExpr ) : Expr_StringUnary_c ( "Expr_MonthName_c", pExpr ) {}

	ISphExpr *	Clone() const override														{ return new Expr_MonthName_c(*this); }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override	{ return FormatMonthName ( m_pFirst->IntEval(tMatch), ppStr ); }
};


class Expr_TimeDiff_c : public Expr_Binary_c
{
public:
	Expr_TimeDiff_c ( ISphExpr * pFirst, ISphExpr * pSecond )
		: Expr_Binary_c ( "Expr_TimeDiff_c", pFirst, pSecond )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		assert ( m_pFirst && m_pSecond );
		return m_pFirst->IntEval ( tMatch )-m_pSecond->IntEval ( tMatch );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		int iVal = IntEval ( tMatch );
		CSphString sVal;
		int t = iVal<0 ? -iVal : iVal;
		sVal.SetSprintf ( "%s%02d:%02d:%02d", iVal<0 ? "-" : "", t/60/60, (t/60)%60, t%60 );
		int iLength = sVal.Length();
		*ppStr = (const BYTE*) sVal.Leak();
		return iLength;
	}

	float		Eval ( const CSphMatch & tMatch ) const final		{ return (float)IntEval ( tMatch ); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)IntEval ( tMatch ); }
	bool		IsDataPtrAttr() const final							{ return true; }
	ISphExpr *	Clone() const final									{ return new Expr_TimeDiff_c ( *this );	}
};


class Expr_DateDiff_c : public Expr_Binary_c
{
public:
	Expr_DateDiff_c ( ISphExpr * pFirst, ISphExpr * pSecond )
		: Expr_Binary_c ( "Expr_DateDiff_c", pFirst, pSecond )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		assert ( m_pFirst && m_pSecond );

		cctz::civil_day tDay1 = cctz::civil_day ( ConvertTime ( m_pFirst->IntEval(tMatch) ) );
		cctz::civil_day tDay2 = cctz::civil_day ( ConvertTime ( m_pSecond->IntEval(tMatch) ) );
				
		return tDay1-tDay2;
	}

	float		Eval ( const CSphMatch & tMatch ) const final		{ return (float)IntEval ( tMatch ); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)IntEval ( tMatch ); }
	ISphExpr *	Clone() const final									{ return new Expr_DateDiff_c ( *this );	}
};


class ExprDateFormat_c : public ISphStringExpr
{
public:
	ExprDateFormat_c ( ISphExpr * pArg, ISphExpr * pFmt )
		: m_pArg ( pArg )
	{
		assert( pArg );
		SafeAddRef( pArg );

		CSphMatch tTmp;
		const BYTE * sVal = nullptr;
		int iLen = pFmt->StringEval ( tTmp, &sVal );
		m_sFmt = CSphString ( (const char*)sVal, iLen );
		FreeDataPtr ( pFmt, sVal );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		int64_t iTime = m_pArg->Int64Eval ( tMatch );

		// FIXME: modify this to use static buffer (performance issue)
		CSphString sRes = FormatTime ( time_t(iTime), m_sFmt.cstr() );
		int iLen = sRes.Length();
		*ppStr = (const BYTE *)sRes.Leak();

		return iLen;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		if ( m_pArg )
			m_pArg->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( m_pArg )
			m_pArg->Command ( eCmd, pArg );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	bool IsConst () const final { return false; }

	ISphExpr * Clone () const final
	{
		return new ExprDateFormat_c ( *this );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("ExprDateFormat_c");
		CALC_CHILD_HASH(m_pArg);
		return CALC_DEP_HASHES();
	}


private:
	ExprDateFormat_c ( const ExprDateFormat_c & rhs )
		: m_pArg ( SafeClone ( rhs.m_pArg ) )
		, m_sFmt ( rhs.m_sFmt )
	{}

	CSphRefcountedPtr<ISphExpr> m_pArg;
	CSphString m_sFmt;
};

template<bool UTC>
class Expr_Day_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Day_T ( ISphExpr * pExpr ) : BASE ( "Expr_Day_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).day(); }
	ISphExpr *	Clone() const final									{ return new Expr_Day_T(*this); }
};

template<bool UTC>
class Expr_Week_T : public Expr_TimeTraitsBinary_T<UTC>
{
	using BASE=Expr_TimeTraitsBinary_T<UTC>;

public:
	Expr_Week_T ( ISphExpr * pFirst, ISphExpr * pSecond ) : BASE ( "Expr_Week_T", pFirst, pSecond )
	{
		if ( !pSecond || pSecond->IsConst() )
		{
			m_bConst = true;
			if ( pSecond )
			{
				CSphMatch tMatch;
				m_uFlags = pSecond->IntEval(tMatch);
			}
		}
	}

	int64_t	Int64Eval ( const CSphMatch & tMatch ) const final
	{
		if ( m_bConst )
			return CalcWeekNumber ( BASE::GetConvertedTime(tMatch), m_uFlags );

		return CalcWeekNumber ( BASE::GetConvertedTime(tMatch), BASE::m_pSecond->IntEval(tMatch) );
	}

	ISphExpr *	Clone() const final									{ return new Expr_Week_T(*this); }

private:
	bool		m_bConst = false;
	uint32_t	m_uFlags = 3;
};

template<bool UTC>
class Expr_Month_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Month_T ( ISphExpr * pExpr ) : BASE ( "Expr_Month_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).month(); }
	ISphExpr *	Clone() const final									{ return new Expr_Month_T(*this); }
};

template<bool UTC>
class Expr_Year_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Year_T ( ISphExpr * pExpr ) : BASE ( "Expr_Year_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).year(); }
	ISphExpr *	Clone() const final									{ return new Expr_Year_T(*this); }
};

template<bool UTC>
class Expr_YearMonth_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_YearMonth_T ( ISphExpr * pExpr ) : BASE ( "Expr_YearMonth_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return CalcYearMonth ( BASE::GetConvertedTime(tMatch) ); }
	ISphExpr *	Clone() const final	{ return new Expr_YearMonth_T(*this); }
};

template<bool UTC>
class Expr_YearMonthDay_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_YearMonthDay_T ( ISphExpr * pExpr ) : BASE ( "Expr_YearMonthDay_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return CalcYearMonthDay ( BASE::GetConvertedTime(tMatch) ); }
	ISphExpr *	Clone() const final									{ return new Expr_YearMonthDay_T(*this); }
};

template<bool UTC>
class Expr_YearWeek_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_YearWeek_T ( ISphExpr * pExpr ) : BASE ( "Expr_YearWeek_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return CalcYearWeek ( BASE::GetConvertedTime(tMatch) ); }
	ISphExpr *	Clone() const final									{ return new Expr_YearWeek_T(*this); }
};


template<bool UTC>
class Expr_Hour_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Hour_T ( ISphExpr * pExpr ) : BASE ( "Expr_Hour_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).hour(); }
	ISphExpr *	Clone() const final									{ return new Expr_Hour_T(*this); }
};

template<bool UTC>
class Expr_Minute_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Minute_T ( ISphExpr * pExpr ) : BASE ( "Expr_Minute_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).minute(); }
	ISphExpr *	Clone() const final									{ return new Expr_Minute_T(*this); }
};

template<bool UTC>
class Expr_Second_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Second_T ( ISphExpr * pExpr ) : BASE ( "Expr_Second_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return BASE::GetConvertedTime(tMatch).second(); }
	ISphExpr *	Clone() const final									{ return new Expr_Second_T(*this); }
};

template<bool UTC>
class Expr_DayOfWeek_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_DayOfWeek_T ( ISphExpr * pExpr ) : BASE ( "Expr_DayOfWeek_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return GetWeekDay ( BASE::GetConvertedTime(tMatch), true ); }
	ISphExpr *	Clone() const final									{ return new Expr_DayOfWeek_T(*this); }
};

template<bool UTC>
class Expr_DayOfYear_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_DayOfYear_T ( ISphExpr * pExpr ) : BASE ( "Expr_DayOfYear_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return GetYearDay ( BASE::GetConvertedTime(tMatch) ); }
	ISphExpr *	Clone() const final									{ return new Expr_DayOfYear_T(*this); }
};

template<bool UTC>
class Expr_Quarter_T : public Expr_TimeTraits_T<UTC>
{
	using BASE=Expr_TimeTraits_T<UTC>;

public:
				Expr_Quarter_T ( ISphExpr * pExpr ) : BASE ( "Expr_Quarter_T", pExpr ) {}
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return GetQuarter ( BASE::GetConvertedTime(tMatch) ); }
	ISphExpr *	Clone() const final									{ return new Expr_Quarter_T(*this); }
};


class Expr_DateAdd_c : public Expr_Binary_c
{
public:
				Expr_DateAdd_c ( ISphExpr * pFirst, ISphExpr * pSecond, TimeUnit_e eUnit, bool bAdd );

	int			IntEval ( const CSphMatch & tMatch ) const final;
	float		Eval ( const CSphMatch & tMatch ) const final		{ return (float)IntEval ( tMatch ); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)IntEval ( tMatch ); }
	ISphExpr *	Clone() const final									{ return new Expr_DateAdd_c ( *this );	}

private:
	TimeUnit_e	m_eUnit = TimeUnit_e::SECOND;
	bool		m_bAdd = true;
	bool		m_bConst = true;
	int			m_iAdd = 0;
};


Expr_DateAdd_c::Expr_DateAdd_c ( ISphExpr * pFirst, ISphExpr * pSecond, TimeUnit_e eUnit, bool bAdd )
	: Expr_Binary_c ( "Expr_DateAdd_c", pFirst, pSecond )
	, m_eUnit ( eUnit )
	, m_bAdd ( bAdd )
{
	m_bConst = m_pSecond->IsConst();
	if ( m_bConst )
	{
		CSphMatch tMatch;
		m_iAdd = m_pSecond->IntEval(tMatch);
	}
}


int Expr_DateAdd_c::IntEval ( const CSphMatch & tMatch ) const
{
	cctz::civil_second tCS = ConvertTime ( m_pFirst->IntEval(tMatch) );
	int iAdd = m_bConst ? m_iAdd : m_pSecond->IntEval(tMatch);
	iAdd *= m_bAdd ? 1 : -1;

	switch ( m_eUnit )
	{
	case TimeUnit_e::SECOND:
		return ConvertTime ( cctz::civil_second ( tCS.year(), tCS.month(), tCS.day(), tCS.hour(), tCS.minute(), tCS.second() + iAdd ) );

	case TimeUnit_e::MINUTE:
		return ConvertTime ( cctz::civil_second ( tCS.year(), tCS.month(), tCS.day(), tCS.hour(), tCS.minute() + iAdd, tCS.second() ) );

	case TimeUnit_e::HOUR:
		return ConvertTime ( cctz::civil_second ( tCS.year(), tCS.month(), tCS.day(), tCS.hour() + iAdd, tCS.minute(), tCS.second() ) );

	case TimeUnit_e::DAY:
	case TimeUnit_e::WEEK:
	{
		iAdd *= m_eUnit==TimeUnit_e::WEEK ? 7 : 1;
		return ConvertTime ( cctz::civil_second ( tCS.year(), tCS.month(), tCS.day() + iAdd, tCS.hour(), tCS.minute(), tCS.second() ) );
	}

	case TimeUnit_e::MONTH:
	case TimeUnit_e::QUARTER:
	{
		iAdd *= m_eUnit==TimeUnit_e::QUARTER ? 3 : 1;

		const auto tNextMonth = cctz::civil_month(tCS) + iAdd;
		const auto tLastDayOfNextMonth = cctz::civil_second ( cctz::civil_day(tNextMonth + 1) - 1 );
		const auto tNormalized = cctz::civil_second ( tCS.year(), tCS.month() + iAdd, tCS.day(), tCS.hour(), tCS.minute(), tCS.second() );
		return ConvertTime ( std::min ( tNormalized, tLastDayOfNextMonth ) );
	}

	case TimeUnit_e::YEAR:
		return ConvertTime ( cctz::civil_second ( tCS.year() + iAdd, tCS.month(), tCS.day(), tCS.hour(), tCS.minute(), tCS.second() ) );

	default:
		assert ( 0 && "Unknown time unit" );
		return 0;
	}
}


void SetGroupingInUTC ( bool bGroupingInUtc )
{
	g_bUseUTC = bGroupingInUtc;
}


bool GetGroupingInUTC()
{
	return g_bUseUTC;
}


ISphExpr * CreateExprNow ( int iNow )
{
	return new Expr_Now_c(iNow);
}

ISphExpr * CreateExprDateFormat ( ISphExpr * pArg, ISphExpr * pFmt )
{
	return new ExprDateFormat_c ( pArg, pFmt );
}

ISphExpr * CreateExprCurTime ( bool bUTC, bool bDate )
{
	int iIndex = 2*(bUTC ? 1 : 0) + (bDate ? 1 : 0);
	switch ( iIndex )
	{
	case 0:	return new Expr_CurTime_T<false,false>;
	case 1:	return new Expr_CurTime_T<false,true>;
	case 2:	return new Expr_CurTime_T<true,false>;
	case 3:	return new Expr_CurTime_T<true,true>;
	default:
		assert ( 0 && "CreateExprTime error" );
		return nullptr;
	}
}

ISphExpr * CreateExprCurDate()
{
	return new Expr_CurDate_c;
}

ISphExpr * CreateExprTime ( ISphExpr * pArg )
{
	return new Expr_Time_c(pArg);
}

ISphExpr * CreateExprDate ( ISphExpr * pArg )
{
	return new Expr_Date_c(pArg);
}

ISphExpr * CreateExprDayName ( ISphExpr * pArg )
{
	return new Expr_DayName_c(pArg);
}

ISphExpr * CreateExprMonthName ( ISphExpr * pArg )
{
	return new Expr_MonthName_c(pArg);
}

ISphExpr * CreateExprTimeDiff ( ISphExpr * pFirst, ISphExpr * pSecond )
{
	return new Expr_TimeDiff_c ( pFirst, pSecond );
}

ISphExpr * CreateExprDateDiff ( ISphExpr * pFirst, ISphExpr * pSecond )
{
	return new Expr_DateDiff_c ( pFirst, pSecond );
}

ISphExpr * CreateExprDateAdd ( ISphExpr * pFirst, ISphExpr * pSecond, TimeUnit_e eUnit, bool bAdd )
{
	return new Expr_DateAdd_c ( pFirst, pSecond, eUnit, bAdd );
}

ISphExpr * CreateExprDay ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Day_T<true>(pExpr) : (ISphExpr *)new Expr_Day_T<false>(pExpr);
}

ISphExpr * CreateExprWeek ( ISphExpr * pFirst, ISphExpr * pSecond )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Week_T<true>( pFirst, pSecond ) : (ISphExpr *)new Expr_Week_T<false>( pFirst, pSecond );
}

ISphExpr * CreateExprMonth ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Month_T<true>(pExpr) : (ISphExpr *)new Expr_Month_T<false>(pExpr);
}

ISphExpr * CreateExprYear ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Year_T<true>(pExpr) : (ISphExpr *)new Expr_Year_T<false>(pExpr);
}

ISphExpr * CreateExprYearMonth ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_YearMonth_T<true>(pExpr) : (ISphExpr *)new Expr_YearMonth_T<false>(pExpr);
}

ISphExpr * CreateExprYearMonthDay ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_YearMonthDay_T<true>(pExpr) : (ISphExpr *)new Expr_YearMonthDay_T<false>(pExpr);
}

ISphExpr * CreateExprYearWeek ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_YearWeek_T<true>(pExpr) : (ISphExpr *)new Expr_YearWeek_T<false>(pExpr);
}

ISphExpr * CreateExprHour ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Hour_T<true>(pExpr) : (ISphExpr *)new Expr_Hour_T<false>(pExpr);
}

ISphExpr * CreateExprMinute ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Minute_T<true>(pExpr) : (ISphExpr *)new Expr_Minute_T<false>(pExpr);
}

ISphExpr * CreateExprSecond ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Second_T<true>(pExpr) : (ISphExpr *)new Expr_Second_T<false>(pExpr);
}

ISphExpr * CreateExprDayOfWeek ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_DayOfWeek_T<true>(pExpr) : (ISphExpr *)new Expr_DayOfWeek_T<false>(pExpr);
}

ISphExpr * CreateExprDayOfYear ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_DayOfYear_T<true>(pExpr) : (ISphExpr *)new Expr_DayOfYear_T<false>(pExpr);
}

ISphExpr * CreateExprQuarter ( ISphExpr * pExpr )
{
	return g_bUseUTC ? (ISphExpr *)new Expr_Quarter_T<true>(pExpr) : (ISphExpr *)new Expr_Quarter_T<false>(pExpr);
}
