//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>
#include "std/bitmap/bitmap.h"
#include <algorithm>
#include <random>
#include <vector>
#if !defined(_WIN32)
#include <sys/mman.h>
#endif

namespace
{
class Bitmap : public ::testing::TestWithParam<bitmap::Backend_e> {};

uint64_t ReferenceCount ( DWORD u )
{
	uint64_t n = 0;
	for ( int i=0; i<32; ++i ) n += ( u>>i ) & 1;
	return n;
}

void CheckOperations ( const bitmap::Operations_t & t, const DWORD * a, const DWORD * b, size_t n )
{
	uint64_t uCount = 0;
	for ( size_t i=0; i<n; ++i ) uCount += ReferenceCount(a[i]);
	EXPECT_EQ ( t.m_fnCount ( a, n ), uCount );
	const bitmap::Binary_fn dBinary[] = { t.m_fnAnd, t.m_fnOr, t.m_fnXor, t.m_fnAndNot };
	const bitmap::CountBinary_fn dCount[] = { t.m_fnCountAnd, t.m_fnCountOr, t.m_fnCountXor, t.m_fnCountAndNot };
	for ( int op=0; op<4; ++op )
	{
		SCOPED_TRACE(op);
		std::vector<DWORD> dExpected(n), dOutput ( n+2, 0xdeadbeef );
		uCount = 0;
		for ( size_t i=0; i<n; ++i )
		{
			dExpected[i] = op==0 ? a[i]&b[i] : op==1 ? a[i]|b[i] : op==2 ? a[i]^b[i] : a[i]&~b[i];
			uCount += ReferenceCount(dExpected[i]);
		}
		EXPECT_EQ ( dCount[op] ( a, b, n ), uCount );
		dBinary[op] ( dOutput.data()+1, a, b, n );
		EXPECT_TRUE ( std::equal ( dExpected.begin(), dExpected.end(), dOutput.begin()+1 ) );
		EXPECT_EQ ( dOutput.front(), 0xdeadbeef );
		EXPECT_EQ ( dOutput.back(), 0xdeadbeef );
		// Both exact aliases are supported, including asymmetric AND-NOT.
		if ( n )
		{
			std::vector<DWORD> dLeft ( a, a+n ), dRight ( b, b+n );
			dBinary[op] ( dLeft.data(), dLeft.data(), b, n );
			EXPECT_EQ ( dLeft, dExpected );
			dBinary[op] ( dRight.data(), a, dRight.data(), n );
			EXPECT_EQ ( dRight, dExpected );
			dLeft.assign ( a, a+n );
			dBinary[op] ( dLeft.data(), dLeft.data(), dLeft.data(), n );
			for ( size_t i=0; i<n; ++i ) EXPECT_EQ ( dLeft[i], op<2 ? a[i] : 0u );
		}
	}
}

TEST_P ( Bitmap, TailsAndAlignment )
{
	const auto * p = bitmap::GetOperations(GetParam());
	if ( !p ) GTEST_SKIP() << "backend unavailable on this build/CPU";
	std::mt19937 tRandom(2777);
	std::vector<DWORD> a(320), b(320);
	for ( auto & u : a ) u = tRandom();
	for ( auto & u : b ) u = tRandom();
	for ( size_t n=0; n<=257; ++n )
		for ( size_t offset=0; offset<16; ++offset )
		{
			SCOPED_TRACE ( ::testing::Message() << "words=" << n << " offset=" << offset );
			CheckOperations ( *p, a.data()+offset, b.data()+15-offset, n );
		}
}

TEST_P ( Bitmap, PatternsAndLongInputs )
{
	const auto * p = bitmap::GetOperations(GetParam());
	if ( !p ) GTEST_SKIP();
	for ( DWORD u : { 0u, 0xffffffffu, 0x55555555u, 0x80000000u } )
	{
		std::vector<DWORD> a ( 65539, u ), b ( a.size(), 0xaaaaaaaa );
		CheckOperations ( *p, a.data(), b.data(), a.size() );
	}
}

TEST_P ( Bitmap, Empty )
{
	const auto * p = bitmap::GetOperations(GetParam());
	if ( !p ) GTEST_SKIP();
	CheckOperations ( *p, nullptr, nullptr, 0 );
	p->m_fnAnd ( nullptr, nullptr, nullptr, 0 );
	p->m_fnOr ( nullptr, nullptr, nullptr, 0 );
	p->m_fnXor ( nullptr, nullptr, nullptr, 0 );
	p->m_fnAndNot ( nullptr, nullptr, nullptr, 0 );
}

// The last input/output word touches a protected page: any SIMD overread or
// overwrite faults, even when ordinary vector allocations would hide it.
TEST_P ( Bitmap, GuardPage )
{
	const auto * p = bitmap::GetOperations(GetParam());
	if ( !p ) GTEST_SKIP();
#if defined(_WIN32)
	SYSTEM_INFO tInfo;
	GetSystemInfo(&tInfo);
	const size_t nPage = tInfo.dwPageSize;
	auto * pMemory = static_cast<unsigned char *>(VirtualAlloc ( nullptr, 2*nPage, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE ));
	ASSERT_NE ( pMemory, nullptr );
	DWORD uOld;
	const bool bProtected = VirtualProtect ( pMemory+nPage, nPage, PAGE_NOACCESS, &uOld )!=0;
#else
	const size_t nPage = size_t(sysconf(_SC_PAGESIZE));
	auto * pMemory = static_cast<unsigned char *>(mmap ( nullptr, 2*nPage, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 ));
	ASSERT_NE ( pMemory, MAP_FAILED );
	const bool bProtected = mprotect ( pMemory+nPage, nPage, PROT_NONE )==0;
#endif
	EXPECT_TRUE(bProtected);
	if ( bProtected )
		for ( size_t n=1; n<=65; ++n )
		{
			auto * a = reinterpret_cast<DWORD *>(pMemory+nPage)-n;
			std::fill ( a, a+n, 0xabcdef12 );
			CheckOperations ( *p, a, a, n );
			p->m_fnOr ( a, a, a, n );
			p->m_fnAnd ( a, a, a, n );
			p->m_fnXor ( a, a, a, n );
			p->m_fnAndNot ( a, a, a, n );
		}
#if defined(_WIN32)
	VirtualFree ( pMemory, 0, MEM_RELEASE );
#else
	munmap ( pMemory, 2*nPage );
#endif
}

INSTANTIATE_TEST_SUITE_P ( Backends, Bitmap, ::testing::Values (
	bitmap::Backend_e::GENERIC, bitmap::Backend_e::POPCNT, bitmap::Backend_e::SSSE3,
	bitmap::Backend_e::AVX2, bitmap::Backend_e::AVX512 ) );

TEST ( BitmapDispatch, SelectionAndWrappers )
{
	const auto & t = bitmap::GetOperations();
	EXPECT_EQ ( &t, &bitmap::GetOperations() );
	EXPECT_EQ ( &t, bitmap::GetOperations(t.m_eBackend) );
	EXPECT_NE ( bitmap::GetOperations(bitmap::Backend_e::GENERIC), nullptr );
	EXPECT_EQ ( bitmap::GetOperations(static_cast<bitmap::Backend_e>(999)), nullptr );
	const DWORD a[] = { 0x55555555, 0xffffffff }, b[] = { 0xaaaaaaaa, 0xffffffff };
	DWORD d[2];
	EXPECT_EQ ( bitmap::Count ( a, 2 ), 48u );
	EXPECT_EQ ( bitmap::CountAnd ( a, b, 2 ), 32u );
	EXPECT_EQ ( bitmap::CountOr ( a, b, 2 ), 64u );
	EXPECT_EQ ( bitmap::CountXor ( a, b, 2 ), 32u );
	EXPECT_EQ ( bitmap::CountAndNot ( a, b, 2 ), 16u );
	bitmap::And ( d, a, b, 2 ); EXPECT_EQ ( d[0], 0u ); EXPECT_EQ ( d[1], 0xffffffffu );
	bitmap::Or ( d, a, b, 2 ); EXPECT_EQ ( d[0], 0xffffffffu ); EXPECT_EQ ( d[1], 0xffffffffu );
	bitmap::Xor ( d, a, b, 2 ); EXPECT_EQ ( d[0], 0xffffffffu ); EXPECT_EQ ( d[1], 0u );
	bitmap::AndNot ( d, a, b, 2 ); EXPECT_EQ ( d[0], 0x55555555u ); EXPECT_EQ ( d[1], 0u );
}
} // namespace
