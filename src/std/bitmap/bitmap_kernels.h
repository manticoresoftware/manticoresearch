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

#include "bitmap.h"
#include <cstring>

// Internal linkage is essential: no template instantiation or inline helper
// from one ISA translation unit may be substituted for another at link time.
namespace
{
enum class Op_e { COPY, AND, OR, XOR, AND_NOT };

template<Op_e OP, typename T>
T Combine ( T a, T b )
{
	if constexpr ( OP==Op_e::AND ) return a & b;
	if constexpr ( OP==Op_e::OR ) return a | b;
	if constexpr ( OP==Op_e::XOR ) return a ^ b;
	if constexpr ( OP==Op_e::AND_NOT ) return a & ~b;
	return a;
}

inline uint64_t ScalarCount ( uint64_t uWord )
{
	uWord -= ( uWord >> 1 ) & UINT64_C(0x5555555555555555);
	uWord = ( uWord & UINT64_C(0x3333333333333333) ) + ( ( uWord >> 2 ) & UINT64_C(0x3333333333333333) );
	uWord = ( uWord + ( uWord >> 4 ) ) & UINT64_C(0x0f0f0f0f0f0f0f0f);
	return ( uWord * UINT64_C(0x0101010101010101) ) >> 56;
}

struct ScalarTraits_t
{
	using Vec_t = uint64_t;
	static constexpr size_t WORDS = 2;
	static Vec_t Zero() { return 0; }
	static Vec_t Load ( const DWORD * p ) { Vec_t u; std::memcpy ( &u, p, sizeof(u) ); return u; }
	static void Store ( DWORD * p, Vec_t u ) { std::memcpy ( p, &u, sizeof(u) ); }
	template<Op_e OP> static Vec_t Apply ( Vec_t a, Vec_t b ) { return Combine<OP> ( a, b ); }
	static Vec_t Count ( Vec_t u ) { return ScalarCount(u); }
	static Vec_t Add ( Vec_t a, Vec_t b ) { return a+b; }
	static uint64_t Sum ( Vec_t u ) { return u; }
};

template<typename T, Op_e OP>
uint64_t CountWords ( const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	auto tSum = T::Zero();
	size_t i = 0;
	for ( ; nWords-i>=T::WORDS; i+=T::WORDS )
	{
		auto tWord = T::Load ( pLeft+i );
		if constexpr ( OP!=Op_e::COPY )
			tWord = T::template Apply<OP> ( tWord, T::Load ( pRight+i ) );
		tSum = T::Add ( tSum, T::Count(tWord) );
	}
	uint64_t uCount = T::Sum(tSum);
	for ( ; i<nWords; ++i )
	{
		DWORD uWord = pLeft[i];
		if constexpr ( OP!=Op_e::COPY )
			uWord = Combine<OP> ( uWord, pRight[i] );
		uCount += ScalarCount(uWord);
	}
	return uCount;
}

template<typename T>
uint64_t CountWords ( const DWORD * pWords, size_t nWords )
{
	return CountWords<T, Op_e::COPY> ( pWords, nullptr, nWords );
}

template<typename T, Op_e OP>
void BinaryWords ( DWORD * pDst, const DWORD * pLeft, const DWORD * pRight, size_t nWords )
{
	size_t i = 0;
	for ( ; nWords-i>=T::WORDS; i+=T::WORDS )
		T::Store ( pDst+i, T::template Apply<OP> ( T::Load ( pLeft+i ), T::Load ( pRight+i ) ) );
	for ( ; i<nWords; ++i )
		pDst[i] = Combine<OP> ( pLeft[i], pRight[i] );
}

template<typename T>
constexpr bitmap::Operations_t MakeOperations ( bitmap::Backend_e eBackend, const char * sName )
{
	return { eBackend, sName, &CountWords<T>,
		&CountWords<T, Op_e::AND>, &CountWords<T, Op_e::OR>, &CountWords<T, Op_e::XOR>, &CountWords<T, Op_e::AND_NOT>,
		&BinaryWords<T, Op_e::AND>, &BinaryWords<T, Op_e::OR>, &BinaryWords<T, Op_e::XOR>, &BinaryWords<T, Op_e::AND_NOT> };
}
} // namespace
