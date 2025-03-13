//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
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
#include "secondary/secondary.h"

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

using RowIteratorsWithEstimates_t = CSphVector<std::pair<RowidIterator_i *,int64_t>>;

class SIContainer_c
{
	friend void operator << ( JsonEscapedBuilder & tOut, const SIContainer_c & tSI );

public:
	bool		Load ( const CSphString & sFile, CSphString & sError );
	bool		Drop ( const CSphString & sFile, CSphString & sError );
	bool		IsEmpty() const { return m_dIndexes.IsEmpty(); }
	void		Reset() { m_dIndexes.Reset(); }

	void		ColumnUpdated ( const CSphString & sAttr );
	bool		SaveMeta ( CSphString & sError ) const;
	bool		CreateIterators ( std::vector<common::BlockIterator_i *> & dIterators, const common::Filter_t & tFilter, const common::RowidRange_t * pBounds, uint32_t uMaxValues, int64_t iRsetSize, int iCutoff, CSphString & sError ) const;
	int64_t		GetCountDistinct ( const CSphString & sAttr ) const;
	bool		CalcCount ( uint32_t & uCount, const common::Filter_t & tFilter, uint32_t uMaxValues, CSphString & sError ) const;
	uint32_t	GetNumIterators ( const common::Filter_t & tFilter ) const;
	bool		IsEnabled ( const CSphString & sAttr ) const;
	void		GetIndexAttrInfo ( std::vector<SI::IndexAttrInfo_t> & dInfo ) const;

	RowIteratorsWithEstimates_t CreateSecondaryIndexIterator ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, int iCutoff ) const;

private:
	struct IndexInfo_t
	{
		std::unique_ptr<SI::Index_i>	m_pIndex;
		CSphString						m_sFile;
	};

	CSphVector<IndexInfo_t> m_dIndexes;
};

struct RowIdBoundaries_t;

RowidIterator_i *	CreateIteratorIntersect ( CSphVector<RowidIterator_i*> & dIterators, const RowIdBoundaries_t * pBoundaries );
RowidIterator_i *	CreateIteratorWrapper ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries );

const CSphFilterSettings * GetRowIdFilter ( const CSphVector<CSphFilterSettings> & dFilters, RowID_t uTotalDocs, RowIdBoundaries_t & tRowidBounds );
bool				ReturnIteratorResult ( RowID_t * pRowID, RowID_t * pRowIdStart, RowIdBlock_t & dRowIdBlock );

CSphVector<SecondaryIndexInfo_t> SelectIterators ( const SelectIteratorCtx_t & tCtx, float & fBestCost, StrVec_t & dWarnings );

namespace SI
{
	class Index_i;
	class Builder_i;
}


std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int64_t iMemoryLimit, const CSphSchema & tSchema, CSphBitvec & tSIAttrs, const CSphString & sFile, int iBufferSize, CSphString & sError );
std::unique_ptr<SI::Builder_i> CreateIndexBuilder ( int64_t iMemoryLimit, const CSphSchema & tSchema, const CSphString & sFile, CSphVector<PlainOrColumnar_t> & dAttrs, int iBufferSize, CSphString & sError );

void BuildStoreSI ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, SI::Builder_i * pBuilder, CSphVector<int64_t> & dTmp );

#endif // _secondaryindex_
