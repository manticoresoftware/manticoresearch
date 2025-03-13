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

#ifndef _aggregate_
#define _aggregate_

#include "match.h"
#include "columnarlib.h"

class AggrFunc_i
{
public:
	virtual			~AggrFunc_i() {}

	virtual void	Ungroup ( CSphMatch & tDst ) {}
	virtual void	Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) = 0;
	virtual void	Setup ( CSphMatch & tDst, const CSphMatch & tSrc, bool bMerge ) {}
	virtual void	Finalize ( CSphMatch & tDst ) {}
	virtual void	SetColumnar ( columnar::Columnar_i * pColumnar ) {}
};


AggrFunc_i * CreateAggrSum ( const CSphColumnInfo & tAttr );
AggrFunc_i * CreateAggrAvg ( const CSphColumnInfo & tAttr, const CSphAttrLocator & tCount );
AggrFunc_i * CreateAggrMin ( const CSphColumnInfo & tAttr );
AggrFunc_i * CreateAggrMax ( const CSphColumnInfo & tAttr );

AggrFunc_i * CreateAggrConcat ( const CSphColumnInfo & tAttr );

#endif // _aggregate_
