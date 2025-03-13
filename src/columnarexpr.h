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

#ifndef _columnarexpr_
#define _columnarexpr_

#include "sphinxdefs.h"
#include "sphinxexpr.h"

class ConstList_c;
ISphExpr * CreateExpr_ColumnarMva32In ( const CSphString & sName, ConstList_c * pConsts );
ISphExpr * CreateExpr_ColumnarMva64In ( const CSphString & sName, ConstList_c * pConsts );
ISphExpr * CreateExpr_ColumnarStringIn ( const CSphString & sName, ConstList_c * pConsts, ESphCollation eCollation );

ISphExpr * CreateExpr_ColumnarStringLength ( const CSphString & sName );
ISphExpr * CreateExpr_ColumnarMva32Length ( const CSphString & sName );
ISphExpr * CreateExpr_ColumnarMva64Length ( const CSphString & sName );

ISphExpr * CreateExpr_ColumnarMva32Aggr ( ISphExpr * pExpr, ESphAggrFunc eFunc );
ISphExpr * CreateExpr_ColumnarMva64Aggr ( ISphExpr * pExpr, ESphAggrFunc eFunc );

ISphExpr * CreateExpr_GetColumnarInt ( const CSphString & sName, bool bStored );
ISphExpr * CreateExpr_GetColumnarFloat ( const CSphString & sName, bool bStored );
ISphExpr * CreateExpr_GetColumnarString ( const CSphString & sName, bool bStored );
ISphExpr * CreateExpr_GetColumnarMva ( const CSphString & sName, bool bStored );

#endif // _columnarexpr_
