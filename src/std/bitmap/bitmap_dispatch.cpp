//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "bitmap_backend.h"
#include <initializer_list>

#if defined(__x86_64__) || defined(_M_X64)
#define BITMAP_X64 1
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#else
#define BITMAP_X64 0
#endif

namespace bitmap
{
namespace
{
struct Features_t
{
	bool m_bPopcnt = false;
	bool m_bSSSE3 = false;
	bool m_bAVX2 = false;
	bool m_bAVX512 = false;
};

#if BITMAP_X64
void CPUID ( uint32_t uLeaf, uint32_t dInfo[4] )
{
#if defined(_MSC_VER)
	int dRegs[4];
	__cpuidex ( dRegs, int(uLeaf), 0 );
	for ( int i=0; i<4; ++i ) dInfo[i] = uint32_t(dRegs[i]);
#else
	__cpuid_count ( uLeaf, 0, dInfo[0], dInfo[1], dInfo[2], dInfo[3] );
#endif
}
#endif

Features_t DetectFeatures()
{
	// Native kernels only: unlike the general sys.h helpers, do not treat ARM
	// as x86-compatible through SIMDe. Match each compiler's enabled ISA set.
	Features_t t;
#if BITMAP_X64
	uint32_t dInfo[4];
	CPUID ( 0, dInfo );
	const uint32_t uMaxLeaf = dInfo[0];
	if ( uMaxLeaf<1 ) return t;
	CPUID ( 1, dInfo );
	const uint32_t uECX = dInfo[2];
	t.m_bPopcnt = ( uECX & (1u<<23) )!=0;
	t.m_bSSSE3 = ( uECX & (1u<<9) )!=0;
	// AVX requires both CPU support and OS-managed XMM/YMM register state.
	constexpr uint32_t AVX_MASK = (1u<<26) | (1u<<27) | (1u<<28);
	if ( uMaxLeaf<7 || ( uECX & AVX_MASK )!=AVX_MASK ) return t;
	uint64_t uXCR0;
#if defined(_MSC_VER)
	uXCR0 = _xgetbv(0);
#else
	uint32_t uLow, uHigh;
	__asm__ volatile ( "xgetbv" : "=a"(uLow), "=d"(uHigh) : "c"(0) );
	uXCR0 = ( uint64_t(uHigh)<<32 ) | uLow;
#endif
	if ( ( uXCR0 & 0x6 )!=0x6 ) return t;
	CPUID ( 7, dInfo );
	// Include lower ISA levels and POPCNT that the compiler may use in tails.
	constexpr uint32_t SSE_MASK = (1u<<9) | (1u<<19) | (1u<<20) | (1u<<23);
	t.m_bAVX2 = ( dInfo[1] & (1u<<5) ) && ( uECX & SSE_MASK )==SSE_MASK;
#if defined(_MSC_VER) && !defined(__clang__)
	// MSVC /arch:AVX2 also permits BMI, BMI2, FMA and LZCNT instructions.
	const bool bBMI = ( dInfo[1] & ((1u<<3) | (1u<<8)) )==((1u<<3) | (1u<<8));
	uint32_t dExtended[4];
	CPUID ( 0x80000000, dExtended );
	bool bLZCNT = false;
	if ( dExtended[0]>=0x80000001 )
	{
		CPUID ( 0x80000001, dExtended );
		bLZCNT = ( dExtended[2] & (1u<<5) )!=0;
	}
	t.m_bAVX2 = t.m_bAVX2 && bBMI && bLZCNT && ( uECX & (1u<<12) );
#endif
	t.m_bAVX512 = t.m_bAVX2 && ( uXCR0 & 0xe6 )==0xe6
		&& ( dInfo[1] & (1u<<16) ) && ( dInfo[2] & (1u<<14) ); // F + VPOPCNTDQ
#if defined(_MSC_VER) && !defined(__clang__)
	// /arch:AVX512 enables DQ, CD, BW and VL as well as F.
	constexpr uint32_t AVX512_MASK = (1u<<17) | (1u<<28) | (1u<<30) | (1u<<31);
	t.m_bAVX512 = t.m_bAVX512 && ( dInfo[1] & AVX512_MASK )==AVX512_MASK;
#endif
#endif
	return t;
}
} // namespace

const Operations_t * GetOperations ( Backend_e eBackend )
{
	static const Features_t tFeatures = DetectFeatures();
	switch ( eBackend )
	{
	case Backend_e::GENERIC: return &detail::g_tGeneric;
#if defined(BITMAP_HAVE_POPCNT)
	case Backend_e::POPCNT: return tFeatures.m_bPopcnt ? &detail::g_tPopcnt : nullptr;
#endif
#if defined(BITMAP_HAVE_SSSE3)
	case Backend_e::SSSE3: return tFeatures.m_bSSSE3 ? &detail::g_tSSSE3 : nullptr;
#endif
#if defined(BITMAP_HAVE_AVX2)
	case Backend_e::AVX2: return tFeatures.m_bAVX2 ? &detail::g_tAVX2 : nullptr;
#endif
#if defined(BITMAP_HAVE_AVX512)
	case Backend_e::AVX512: return tFeatures.m_bAVX512 ? &detail::g_tAVX512 : nullptr;
#endif
	default: return nullptr;
	}
}

const Operations_t & GetOperations()
{
	static const Operations_t * pOps = [] {
		// Capability order, not a claim that the widest ISA wins every workload.
		for ( auto eBackend : { Backend_e::AVX512, Backend_e::AVX2, Backend_e::POPCNT, Backend_e::SSSE3 } )
			if ( const auto * p = GetOperations(eBackend) ) return p;
		return &detail::g_tGeneric;
	}();
	return *pOps;
}
} // namespace bitmap
