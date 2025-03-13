//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarmisc_
#define _columnarmisc_

#include "columnarlib.h"
#include "schema/locator.h"
#include "sphinxexpr.h"

using ScopedTypedIterator_t = std::pair<std::unique_ptr<columnar::Iterator_i>,ESphAttr>;

template <typename PITERATOR>
bool AdvanceIterator ( PITERATOR& pIterator, RowID_t tRowID )
{
	assert(pIterator);
	return pIterator->AdvanceTo(tRowID)==tRowID;
}

CSphVector<ScopedTypedIterator_t> CreateAllColumnarIterators ( const columnar::Columnar_i * pColumnar, const ISphSchema & tSchema );

SphAttr_t	SetColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder, std::unique_ptr<columnar::Iterator_i> & pIterator, RowID_t tRowID, CSphVector<int64_t> & dTmp );
void		SetDefaultColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder );
CSphString	ColumnarAttrType2Str ( common::AttrType_e eType );

struct PlainOrColumnar_t
{
	CSphAttrLocator	m_tLocator;
	int				m_iColumnarId = -1;
	ESphAttr		m_eType = SPH_ATTR_NONE;

					PlainOrColumnar_t() = default;
					PlainOrColumnar_t ( const CSphColumnInfo & tAttr, int iColumnar );

	SphAttr_t		Get ( RowID_t tRowID, const CSphRowitem * pRow, CSphVector<ScopedTypedIterator_t> & dIterators ) const;
	int				Get ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const uint8_t * & pData ) const;
};

namespace SI
{
	class Builder_i;
}

#endif // _columnarmisc_
