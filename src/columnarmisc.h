//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
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

void SetColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder, std::unique_ptr<columnar::Iterator_i>& pIterator, CSphVector<int64_t> & dTmp );
void SetDefaultColumnarAttr ( int iAttr, ESphAttr eType, columnar::Builder_i * pBuilder );

struct PlainOrColumnar_t
{
	CSphAttrLocator	m_tLocator;
	int				m_iColumnarId = -1;
	ESphAttr		m_eType = SPH_ATTR_NONE;

	SphAttr_t		Get ( const CSphRowitem * pRow, CSphVector<ScopedTypedIterator_t> & dIterators ) const;
	int				Get ( const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const uint8_t * & pData ) const;
};

class HistogramContainer_c;
void BuildStoreHistograms ( const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, HistogramContainer_c & tHistograms );

namespace SI
{
	class Builder_i;
}

void BuilderStoreAttrs ( const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, SI::Builder_i * pBuilder, CSphVector<int64_t> & dTmp );

#endif // _columnarmisc_
