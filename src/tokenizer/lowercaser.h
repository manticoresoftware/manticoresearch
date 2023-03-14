//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "remap_range.h"
#include "sphinxstd.h"

class CSphLowercaser;
using LowercaserRefcountedPtr = CSphRefcountedPtr<CSphLowercaser>;
using LowercaserRefcountedConstPtr = CSphRefcountedPtr<const CSphLowercaser>;

/// lowercaser
class CSphLowercaser final: public ISphRefcountedMT
{
	static constexpr int CHARS_COUNT = 0x30000;
	static constexpr int CHUNK_BITS = 8;
	static constexpr int CHUNK_SIZE = 1 << CHUNK_BITS; // 0x100 for 8 bits, 0x200 for 9 bits
	static constexpr int CHUNK_COUNT = CHARS_COUNT >> CHUNK_BITS; // 0x300 for 8 bits, 0x180 for 9 bits
	static constexpr int CHUNK_BYTES = CHUNK_SIZE * sizeof ( DWORD );
	static constexpr int CHUNK_MASK = CHUNK_SIZE - 1;

	int m_iChunks = 0;							///< how much chunks are actually allocated
	CSphFixedVector<DWORD> m_dData {0};			///< chunks themselves. 1Kb per chunk (256 DWORDs)
	DWORD* m_pChunk[CHUNK_COUNT] { nullptr };	///< pointers to non-empty chunks. That is 6kB per table
	volatile int m_iGeneration = 0;				///< my generation. Each change increases generation
	mutable CSphMutex	m_tLock;				///< protects moment of cache creation from concurrency

	// with 32-bits pointers:
	// 8 bits per chunk: 3Kb chunks, 1Kb page.   1 page = 4Kb,   2 pages = 5Kb,   3 pages = 6Kb,   4 pages = 7Kb,   5 pages = 8Kb
	// 9 bits per chunk: 1.5Kb chunks, 2Kb page. 1 page = 3.5Kb, 2 pages = 5.5Kb, 3 pages = 7.5Kb, 4 pages = 9.5Kb, 5 pages = 11.5Kb
	// 10 bits per chunk: 0.75Kb chunks, 4Kb page. 1 page = 4.75Kb, 2 pages = 8.75Kb, 3 pages = 12.75Kb, 4 pages = 16.75Kb, 5 pages = 20.75Kb
	// with 64-bits pointers:
	// 8 bits per chunk: 6Kb chunks, 1Kb page. 1 page = 7Kb, 2 pages = 8Kb, 3 pages = 9Kb, 4 pages = 10Kb, 5 pages = 11Kb
	// 9 bits per chunk: 3Kb chunks, 2Kb page. 1 page = 5Kb, 2 pages = 7Kb, 3 pages = 9Kb, 4 pages = 11Kb, 5 pages = 13Kb
	// 10 bits per chunk: 1.5Kb chunks, 4Kb page. 1 page = 5.5Kb, 2 pages = 9.5Kb, 3 pages = 13.5Kb, 4 pages = 17.5Kb, 5 pages = 21.5Kb
	// seems that for 64-bits using 9 bits per chunk is better with typical configurations; need to test!

	void InvalidateStoredClones() noexcept;

protected:
	~CSphLowercaser() final = default;

public:
	static constexpr int MAX_CODE = CHARS_COUNT;

	CSphLowercaser() = default;

	// modifiers
	void Reset();
	void SetRemap ( const CSphLowercaser* pLC );
	void AddRemaps ( const VecTraits_T<CSphRemapRange>& dRemaps, DWORD uAddFlags=0, DWORD uResetFlags=0 );
	void AddChars ( const char* szChars, DWORD uAddFlags=0, DWORD uResetFlags=0 );

	// check that nothing from dRemaps is referenced somewhere
	bool CheckRemap ( CSphString& sError, const VecTraits_T<CSphRemapRange>& dRemaps, const char* sSource, bool bCanRemap ) const noexcept;

	// runtime use (const, noexcept, thread-safe)
	int ToLower ( int iCode ) const noexcept;
	int GetMaxCodepointLength() const noexcept;
	uint64_t GetFNV() const noexcept;

	// buffering stuff. Returns such cached clone instead of full cloning
#define LOWERCASE_CLONE( name ) \
private: \
	mutable volatile int m_i##name##Gen = -1;	\
	mutable LowercaserRefcountedConstPtr m_p##name##LC; \
	void Up##name##Clone() const noexcept;  \
                                \
public: \
	inline LowercaserRefcountedConstPtr Get##name##LC() const noexcept \
	{ \
		Up##name##Clone(); \
		assert ( m_p##name##LC ); \
		return m_p##name##LC; \
	}

	LOWERCASE_CLONE ( Query )				// GetQueryLC 				- special "\\" (all the rest also imply that special)
	LOWERCASE_CLONE ( QueryWildExactJson )	// GetQueryWildExactJsonLC	- "*?%="
	LOWERCASE_CLONE ( QueryWildExact )		// GetQueryWildExactLC		- "*?%=" and specials "\\=()|-!@~"/^$<"
	LOWERCASE_CLONE ( QueryWildJson )		// GetQueryWildJsonLC		- "*?%"
	LOWERCASE_CLONE ( QueryWild )			// GetQueryWildLC			- "*?%" and specials "()|-!@~"/^$<"
	LOWERCASE_CLONE ( QueryExactJson )		// GetQueryExactJsonLC		- "="
	LOWERCASE_CLONE ( QueryExact )			// GetQueryExactLC			- "=" and specials "=()|-!@~"/^$<"
//	LOWERCASE_CLONE ( QueryJson )			// the same as just 'Query'	- special "\\" - that is usual old cloned query
	LOWERCASE_CLONE ( Query_ )				// GetQuery_LC				- specials "()|-!@~"/^$<"

#undef LOWERCASE_CLONE
};


inline int CSphLowercaser::ToLower ( int iCode ) const noexcept
{
	if ( iCode < 0 || iCode >= MAX_CODE )
		return iCode;
	DWORD* pChunk = m_pChunk[iCode >> CHUNK_BITS];
	if ( pChunk )
		return (int)pChunk[iCode & CHUNK_MASK];
	return 0;
}
