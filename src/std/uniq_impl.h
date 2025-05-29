//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "comp.h"

namespace sphstd {

template<typename T, typename T_COUNTER, typename COMP>
T_COUNTER Uniq ( T* pData, T_COUNTER iCount, COMP&& tComp )
{
	if ( !iCount )
		return 0;

	T_COUNTER iSrc = 1, iDst = 1;
	while ( iSrc < iCount )
	{
		if ( tComp.IsEq ( pData[iDst - 1], pData[iSrc] ) )
			iSrc++;
		else
			pData[iDst++] = pData[iSrc++];
	}
	return iDst;
}

} // namespace sphstd

template<typename T, typename T_COUNTER, typename COMP>
T_COUNTER sphUniq ( T* pData, T_COUNTER iCount, COMP&& tComp )
{
	return sphstd::Uniq ( pData, iCount, std::forward<COMP> ( tComp ) );
}

template<typename T, typename T_COUNTER>
T_COUNTER sphUniq ( T* pData, T_COUNTER iCount )
{
	return sphstd::Uniq ( pData, iCount, SphEqualityFunctor_T<T>() );
}