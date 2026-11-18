// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include <cstdint>

namespace postings
{

// Physical doclist offsets are nonnegative. A disk chunk holds at most INT_MAX
// documents, leaving the top bit available when carrying a row and hit together.
constexpr uint64_t DICT_SINGLETON = uint64_t(1)<<63;

inline int64_t PackSingleton ( uint32_t uRow, uint32_t uHit )
{
	return int64_t(DICT_SINGLETON | (uint64_t(uRow)<<32) | uHit);
}

inline uint32_t SingletonRow ( uint64_t uLocator )
{
	return uint32_t((uLocator & ~DICT_SINGLETON)>>32);
}

inline uint32_t SingletonHit ( uint64_t uLocator )
{
	return uint32_t(uLocator);
}

// Keyword entries use three big-endian varints. A zero offset introduces
// a row ID and packed hit, with both dictionary counts implicitly one.
template<typename WRITER>
void WriteKeywordPosting ( WRITER & tWriter, int64_t iOffset, int iDocs, int iHits )
{
	if ( iOffset<0 )
	{
		tWriter.ZipOffset(0);
		tWriter.ZipInt ( SingletonRow(iOffset) );
		tWriter.ZipInt ( SingletonHit(iOffset) );
	} else
	{
		tWriter.ZipOffset(iOffset);
		tWriter.ZipInt(iDocs);
		tWriter.ZipInt(iHits);
	}
}

inline void UnpackKeywordPosting ( int64_t & iOffset, int & iDocs, int & iHits )
{
	if ( !iOffset )
	{
		iOffset = PackSingleton ( uint32_t(iDocs), uint32_t(iHits) );
		iDocs = iHits = 1;
	}
}

}
