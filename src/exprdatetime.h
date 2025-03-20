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

#ifndef _exprdatetime_
#define _exprdatetime_

#include "datetime.h"

class ISphExpr;

void		SetGroupingInUTC ( bool bGroupingInUtc );
bool		GetGroupingInUTC();

ISphExpr *	CreateExprNow ( int iNow );
ISphExpr *	CreateExprDateFormat ( ISphExpr * pArg, ISphExpr * pFmt );
ISphExpr *	CreateExprCurTime ( bool bUTC, bool bDate );
ISphExpr *	CreateExprCurDate();
ISphExpr *	CreateExprTime ( ISphExpr * pArg );
ISphExpr *	CreateExprDate ( ISphExpr * pArg );
ISphExpr *	CreateExprDayName ( ISphExpr * pArg );
ISphExpr *	CreateExprMonthName ( ISphExpr * pArg );
ISphExpr *	CreateExprTimeDiff ( ISphExpr * pFirst, ISphExpr * pSecond );
ISphExpr *	CreateExprDateDiff ( ISphExpr * pFirst, ISphExpr * pSecond );
ISphExpr *	CreateExprDateAdd ( ISphExpr * pFirst, ISphExpr * pSecond, TimeUnit_e eUnit, bool bAdd );
ISphExpr *	CreateExprDay ( ISphExpr * pExpr );
ISphExpr *	CreateExprWeek ( ISphExpr * pFirst, ISphExpr * pSecond );
ISphExpr *	CreateExprMonth ( ISphExpr * pExpr );
ISphExpr *	CreateExprYear ( ISphExpr * pExpr );
ISphExpr *	CreateExprYearMonth ( ISphExpr * pExpr );
ISphExpr *	CreateExprYearMonthDay ( ISphExpr * pExpr );
ISphExpr *	CreateExprYearWeek ( ISphExpr * pExpr );
ISphExpr *	CreateExprHour ( ISphExpr * pExpr );
ISphExpr *	CreateExprMinute ( ISphExpr * pExpr );
ISphExpr *	CreateExprSecond ( ISphExpr * pExpr );
ISphExpr *	CreateExprDayOfWeek ( ISphExpr * pExpr );
ISphExpr *	CreateExprDayOfYear ( ISphExpr * pExpr );
ISphExpr *	CreateExprQuarter ( ISphExpr * pExpr );

#endif // _exprdatetime_

