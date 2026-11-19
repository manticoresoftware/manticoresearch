//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "../ints.h"
#include <cstddef>
#include <cstdint>

namespace bitmap
{

static_assert ( sizeof(DWORD)==4, "bitmap words must be 32 bits" );

enum class Backend_e { GENERIC, POPCNT, SSSE3, AVX2, AVX512 };

using Count_fn = uint64_t (*) ( const DWORD *, size_t );
using CountBinary_fn = uint64_t (*) ( const DWORD *, const DWORD *, size_t );
using Binary_fn = void (*) ( DWORD *, const DWORD *, const DWORD *, size_t );

// Lengths are in 32-bit words, NOT bits or bytes. All bits are significant;
// callers must mask unused bits in the last word. Buffers need only DWORD
// alignment, with no extra padding. Zero-length calls allow null pointers.
// Output may equal either input; other partial overlaps are not supported.
// AND-NOT always means left & ~right.
struct Operations_t
{
	Backend_e 		m_eBackend;
	const char *	m_szName;
	Count_fn 		m_fnCount;
	CountBinary_fn 	m_fnCountAnd;
	CountBinary_fn 	m_fnCountOr;
	CountBinary_fn 	m_fnCountXor;
	CountBinary_fn 	m_fnCountAndNot;
	Binary_fn 		m_fnAnd;
	Binary_fn 		m_fnOr;
	Binary_fn 		m_fnXor;
	Binary_fn 		m_fnAndNot;
};

// Thread-safe, immutable dispatch table, selected once per process.
// Preference: AVX512, AVX2, POPCNT, SSSE3, generic. Benchmark specific backends
// before tuning this policy for a workload. Non-x86-64 targets use generic.
const Operations_t & GetOperations();

// For benchmarks/tests: nullptr when not compiled or unsafe on this CPU/OS.
// Does not change the process-wide selection.
const Operations_t * GetOperations ( Backend_e eBackend );

inline uint64_t Count ( const DWORD * pWords, size_t nWords )
{
	return GetOperations().m_fnCount ( pWords, nWords );
}

inline uint64_t CountAnd ( const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	return GetOperations().m_fnCountAnd ( pLeft, pRight, nWords );
}

inline uint64_t CountOr ( const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	return GetOperations().m_fnCountOr ( pLeft, pRight, nWords );
}

inline uint64_t CountXor ( const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	return GetOperations().m_fnCountXor ( pLeft, pRight, nWords );
}

inline uint64_t CountAndNot ( const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	return GetOperations().m_fnCountAndNot ( pLeft, pRight, nWords );
}

inline void And ( DWORD * pDst, const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	GetOperations().m_fnAnd ( pDst, pLeft, pRight, nWords );
}

inline void Or ( DWORD * pDst, const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	GetOperations().m_fnOr ( pDst, pLeft, pRight, nWords );
}

inline void Xor ( DWORD * pDst, const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	GetOperations().m_fnXor ( pDst, pLeft, pRight, nWords );
}

inline void AndNot ( DWORD * pDst, const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	GetOperations().m_fnAndNot ( pDst, pLeft, pRight, nWords );
}

} // namespace bitmap
