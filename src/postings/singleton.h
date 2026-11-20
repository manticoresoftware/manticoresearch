// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include <array>
#include <cstdint>
#include "../std/thread_annotations.h"
#include "../std/zip.h"

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

// The length prefix lets dictionary scans skip the field mask and position offset.
constexpr int MULTI_HIT_FORMAT_VERSION = 77;
constexpr int MULTI_HIT_MARKER = 1;
constexpr int TEMP_MULTI_HIT_MARKER = -1;
constexpr int MAX_SINGLETON_PAYLOAD = 63;
using SingletonBytes_t = std::array<uint8_t,MAX_SINGLETON_PAYLOAD+1>; // length, TF, row, field code, up to eight masks, .spp offset

// A reader can copy any byte-sized length before the checker inspects the record.
using SingletonReadBuffer_t = std::array<uint8_t,UINT8_MAX+1>;

inline int SingletonTF ( const uint8_t * pRecord )
{
	++pRecord;
	return int(UnzipIntBE(pRecord));
}

template<typename WRITER>
void WriteKeywordPosting ( WRITER & tWriter, int64_t iOffset, int iDocs, int iHits, const uint8_t * pRecord = nullptr, int iMultiMarker = MULTI_HIT_MARKER )
{
	if ( pRecord )
	{
		tWriter.ZipOffset(iMultiMarker);
		tWriter.PutBytes ( pRecord, 1+*pRecord );
	} else if ( iOffset<0 )
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
