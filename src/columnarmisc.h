//
// Copyright (c) 2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarmisc_
#define _columnarmisc_

#include "sphinx.h"

#if USE_COLUMNAR

using ScopedTypedIterator_t = std::pair<std::unique_ptr<columnar::Iterator_i>,ESphAttr>;

template <typename ITERATOR>
bool AdvanceIterator ( ITERATOR * pIterator, RowID_t tRowID )
{
	assert(pIterator);
	return pIterator->AdvanceTo(tRowID)==tRowID;
}

CSphVector<ScopedTypedIterator_t> CreateAllColumnarIterators ( const columnar::Columnar_i * pColumnar, const ISphSchema & tSchema );

void SetColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder, columnar::Iterator_i * pIterator, CSphVector<int64_t> & dTmp );
void SetDefaultColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder );

#endif // USE_COLUMNAR

struct PlainOrColumnar_t
{
	CSphAttrLocator	m_tLocator;
	int				m_iColumnarId = -1;

#if USE_COLUMNAR
	SphAttr_t		Get ( const CSphRowitem * pRow, CSphVector<ScopedTypedIterator_t> & dIterators ) const;
#else
	SphAttr_t		Get ( const CSphRowitem * pRow ) const;
#endif
};

#endif // _columnarmisc_