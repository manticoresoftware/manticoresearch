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

#pragma once

#include <cstdint>
#include "sphinxdefs.h"


struct SmallAttrLocator_t
{
	int m_iBitOffset = -1;
	int m_iBitCount = -1;

	SmallAttrLocator_t () = default;
	explicit SmallAttrLocator_t ( int iBitOffset, int iBitCount = ROWITEM_BITS );

	inline bool IsBitfield () const noexcept;

	int CalcRowitem () const noexcept;

#ifndef NDEBUG

	/// get last item touched by this attr (for debugging checks only)
	int GetMaxRowitem () const noexcept;

#endif
};

/// attribute locator within the row
struct CSphAttrLocator : public SmallAttrLocator_t
{
	int				m_iBlobAttrId	= -1;
	int				m_iBlobRowOffset = 1;
	int				m_nBlobAttrs	= 0;
	bool			m_bDynamic		= true;

	// todo: don't forget to update sphCalcLocatorHash when adding fields!

	CSphAttrLocator () = default;

	bool IsBlobAttr() const noexcept;

	void Reset();

	// just sphFNV64 (&loc,sizeof(loc)) isn't correct as members are not aligned
	uint64_t FNV ( uint64_t uHash ) const;

	bool operator == ( const CSphAttrLocator & rhs ) const;
};

SphAttr_t sphGetRowAttr ( const CSphRowitem* pRow, SmallAttrLocator_t tLoc );

void sphSetRowAttr ( CSphRowitem* pRow, SmallAttrLocator_t tLoc, SphAttr_t uValue );

/// add numeric value of another attribute
void sphAddCounterAttr ( CSphRowitem* pRow, const CSphRowitem* pVal, SmallAttrLocator_t tLoc );

/// add scalar value to aligned numeric attribute
void sphAddCounterScalar ( CSphRowitem* pRow, SmallAttrLocator_t tLoc, SphAttr_t uValue );

#include "locator_impl.h"
