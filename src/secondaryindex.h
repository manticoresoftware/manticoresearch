//
// Copyright (c) 2018-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _secondaryindex_
#define _secondaryindex_

#include "sphinxstd.h"
#include "sphinx.h"
#include "columnarmisc.h"
#include "costestimate.h"

#include <math.h>
#include <vector>


using RowIdBlock_t = VecTraits_T<RowID_t>;

class RowidIterator_i
{
public:
	virtual			~RowidIterator_i(){}

	virtual bool	HintRowID ( RowID_t tRowID ) = 0;
	virtual bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) = 0;
	virtual int64_t	GetNumProcessed() const = 0;
	virtual void	SetCutoff ( int iCutoff ) = 0;
	virtual bool	WasCutoffHit() const = 0;
	virtual void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const = 0;
};


struct RowIdBoundaries_t;

RowidIterator_i *	CreateIteratorIntersect ( CSphVector<RowidIterator_i*> & dIterators, const RowIdBoundaries_t * pBoundaries );
RowidIterator_i *	CreateIteratorWrapper ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries );
RowidIterator_i *	CreateIteratorIntersect ( std::vector<common::BlockIterator_i *> & dIterators, const RowIdBoundaries_t * pBoundaries );

const CSphFilterSettings * GetRowIdFilter ( const CSphVector<CSphFilterSettings> & dFilters, RowID_t uTotalDocs, RowIdBoundaries_t & tRowidBounds );
bool				ReturnIteratorResult ( RowID_t * pRowID, RowID_t * pRowIdStart, RowIdBlock_t & dRowIdBlock );

CSphVector<SecondaryIndexInfo_t> SelectIterators ( const SelectIteratorCtx_t & tCtx, float & fBestCost, StrVec_t & dWarnings );
bool				HaveAvailableSI ( const SelectIteratorCtx_t & tCtx );

namespace SI
{
	class Index_i;
	class Builder_i;
}

CSphVector<RowidIterator_i *> CreateSecondaryIndexIterator ( const SI::Index_i * pSIIndex, CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, int iCutoff );

std::unique_ptr<SI::Builder_i>	CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, CSphBitvec & tSIAttrs, const CSphString & sFile, CSphString & sError );
std::unique_ptr<SI::Builder_i>	CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, const CSphString & sFile, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError );

#endif // _secondaryindex_
