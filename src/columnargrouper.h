//
// Copyright (c) 2020-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnargrouper_
#define _columnargrouper_

#include "collation.h"
#include "schema/columninfo.h"

class CSphGrouper;
CSphGrouper * CreateGrouperColumnarInt ( const CSphColumnInfo & tAttr );
CSphGrouper * CreateGrouperColumnarString ( const CSphColumnInfo & tAttr, ESphCollation eCollation );
CSphGrouper * CreateGrouperColumnarMVA ( const CSphColumnInfo & tAttr );
CSphGrouper * CreateGrouperColumnarMulti ( const CSphVector<CSphColumnInfo> & dAttrs, ESphCollation eCollation );

class DistinctFetcher_i;
DistinctFetcher_i * CreateColumnarDistinctFetcher ( const CSphString & sName, ESphAttr eType, ESphCollation eCollation );

bool NextSet ( CSphFixedVector<int> & dSet, const CSphFixedVector<CSphVector<int64_t>> & dAllKeys );

#endif // _columnargrouper_
