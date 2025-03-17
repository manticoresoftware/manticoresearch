//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _datetime_
#define _datetime_

#include "sphinxstd.h"
#include "cctz/civil_time.h"
#include "cctz/time_zone.h"

enum class TimeUnit_e
{
	SECOND,
	MINUTE,
	HOUR,
	DAY,
	WEEK,
	MONTH,
	QUARTER,
	YEAR
};

void		InitTimeZones ( StrVec_t & dWarnings );
bool		SetTimeZone ( const char * szTZ, CSphString & sError );
bool		IsTimeZoneSet();
CSphString	GetTimeZoneName();
CSphString	GetLocalTimeZoneName();

cctz::civil_second ConvertTime ( time_t tTime );		// use local time zone by default; can be overridden by `timezone` setting
cctz::civil_second ConvertTimeLocal ( time_t tTime );	// always use local time zone
cctz::civil_second ConvertTimeUTC ( time_t tTime );		// always use UTC time zone

time_t		ConvertTime ( const cctz::civil_second & tCS );

time_t		PackLocalTimeAsUTC ( time_t tTime );
CSphString	FormatTime ( time_t tTime, const char * szFmt );
bool		ParseAsLocalTime ( const char * szFmt, const CSphString & sTime, time_t & tRes );
int			GetWeekDay ( const cctz::civil_second & tTime, bool bSundayFirst );
int			GetYearDay ( const cctz::civil_second & tTime );
int			GetQuarter ( const cctz::civil_second & tTime );
int			CalcNumYearDays ( const cctz::civil_second & tTime );
bool		IsLeapYear ( const cctz::civil_second & tTime );

// misc year+month+week combos. used in grouping
int			CalcYearMonth ( const cctz::civil_second & tTime );
int			CalcYearMonthDay ( const cctz::civil_second & tTime );
int			CalcYearWeek ( const cctz::civil_second & tTime );

int			CalcWeekNumber ( const cctz::civil_second & tTime, uint32_t uFlags );

template<bool UTC>
FORCE_INLINE cctz::civil_second ConvertGroupbyTime ( time_t tTime )
{
	if constexpr ( UTC )
		return ConvertTimeUTC(tTime);
	else
		return ConvertTime(tTime);
}

#endif // _datetime_
