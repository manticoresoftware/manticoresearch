//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <benchmark/benchmark.h>

#include "sphinxstd.h"

inline int BitCountDW1 ( DWORD n )
{
	// MIT HACKMEM count
	// works for 32-bit numbers only
	// fix last line for 64-bit numbers
	register DWORD tmp;
	tmp = n - ( ( n >> 1 ) & 033333333333 ) - ( ( n >> 2 ) & 011111111111 );
	return ( ( tmp + ( tmp >> 3 ) ) & 030707070707 ) % 63;
}

inline int BitCountDW2 ( DWORD n )
{
	// MIT HACKMEM count without division
	unsigned long tmp = ( n >> 1 ) & 0x77777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x77777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x77777777UL;
	n -= tmp;
	n = ( n + ( n >> 4 ) ) & 0x0F0F0F0FUL;
	n = n * 0x01010101UL;
	return n >> 24;
}

inline int BitCountU641 ( uint64_t n )
{
	// MIT HACKMEM count without division
	unsigned long tmp = ( n >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	n = ( n + ( n >> 4 ) ) & 0x0F0F0F0F0F0F0F0FUL;
	n = n * 0x0101010101010101UL;
	return n >> 56;
}

class bench_popcount : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state ) override
	{
		sphSrand(0);
		dBytes.Resize ( 128 );
		for ( auto & c: dBytes )
			c = sphRand () & 0xFF;
		iRes = 0;
		i = 0;
	}

	CSphVector<BYTE> dBytes;
	int iRes;
	int	i;
};

BENCHMARK_F( bench_popcount, artificial_no_division ) ( benchmark::State & st )
{
	for ( auto _ : st )
		iRes += BitCountDW2 ( dBytes[++i & 0x7F] );
}

BENCHMARK_F ( bench_popcount, artificial_with_division ) ( benchmark::State& st )
{
	for ( auto _ : st )
		iRes += BitCountDW1 ( dBytes[++i & 0x7F] );
}

BENCHMARK_F ( bench_popcount, artificial_long_no_division ) ( benchmark::State& st )
{
	for ( auto _ : st )
		iRes += BitCountU641 ( dBytes[++i & 0x7F] );
}

BENCHMARK_F ( bench_popcount, intrinsic_dword ) ( benchmark::State& st )
{
	for ( auto _ : st )
		iRes += sphBitCount ( (DWORD)dBytes[++i & 0x7F] );
}

BENCHMARK_F ( bench_popcount, intrinsic_uint64 ) ( benchmark::State& st )
{
	for ( auto _ : st )
		iRes += sphBitCount ( (uint64_t)dBytes[++i & 0x7F] );
}

BENCHMARK_F ( bench_popcount, artificial_byte ) ( benchmark::State& st )
{
	for ( auto _ : st )
		iRes += sphBitCount ( dBytes[++i & 0x7F] );
}

class zippedlength: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state ) override
	{
		sphSrand ( 0 );
		dValues.Resize ( 1024 );
		for ( auto& c : dValues )
		{
			c = sphRand();
			c = ( c << 32 ) | sphRand();
			//c &= 0xFFFFFFFF;
		}
		iRes = 0;
	}

	CSphVector<uint64_t> dValues;
	volatile int iRes = 0;
};


namespace
{
template<typename UINT>
constexpr inline int Log2constUINT ( UINT uValue )
{
	int iBits = 0;
	do
	{
		uValue >>= 1;
		++iBits;
	} while ( uValue );
	return iBits;
}

template<typename T>
inline int CalcZippedLenViaLog2 ( T tValue )
{
	return (sphLog2(tValue)+6)/7; //  gives no branching, but stalled-cycles-frontend because of 'bsr' instruction. It works faster standalone, but slower in ZipValue
}

}


BENCHMARK_F ( zippedlength, zippedlen64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += sphCalcZippedLen ( dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, zippedlen32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += sphCalcZippedLen ( (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, zippedlen16 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += sphCalcZippedLen ( (WORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, zippedlenlog2_64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += CalcZippedLenViaLog2 ( dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, zippedlenlog2_32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += CalcZippedLenViaLog2 ( (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, zippedlenlog2_16 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += CalcZippedLenViaLog2 ( (WORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, log2seq32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += Log2constUINT ( (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, log2seq64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += Log2constUINT ( dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, log2int32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += sphLog2 ( (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zippedlength, log2int64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < 1024; ++i )
			iRes += sphLog2 ( dValues[i] );
	}
}


class zipunzip: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state ) override
	{
		sphSrand ( 0 );
		dValues.Resize ( 1024 );
		dBufBE.Reserve ( 1024 * 10 );
		dBufBE.Resize ( 0 );
		dBufLE.Reserve ( 1024 * 10 );
		dBufLE.Resize ( 0 );
		for ( auto& c : dValues )
		{
			c = sphRand();
			c = ( c << 32 ) | sphRand();
			// c &= 0xFFFFFFFF;
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, c & 0xFFFFFFFF );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE.Add ( b ); }, c & 0xFFFFFFFF );
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE64.Add ( b ); }, c );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE64.Add ( b ); }, c );
		}
		iRes = 0;
	}

	CSphVector<uint64_t> dValues;
	CSphVector<BYTE> dBufBE;
	CSphVector<BYTE> dBufLE;
	CSphVector<BYTE> dBufBE64;
	CSphVector<BYTE> dBufLE64;
	volatile int iRes = 0;
};

template<typename T, typename WRITER>
inline int ZipValueBElog2 ( WRITER fnPut, T tValue )
{
	int nBytes = ( sphLog2 ( tValue ) + 6 ) / 7;
	for ( int i = nBytes - 1; i >= 0; --i )
		fnPut ( ( 0x7f & ( tValue >> ( 7 * i ) ) ) | ( i ? 0x80 : 0 ) );

	return nBytes;
}

// big-endian (most significant septets first)
template<typename T>
inline int ZipToPtrBElog2 ( BYTE* pData, T tValue )
{
	return ZipValueBElog2 ( [pData] ( BYTE b ) mutable { *pData++ = b; }, tValue );
}

BENCHMARK_F ( zipunzip, zipbe32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrBE ( pBuf, (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zipunzip, zipbe32log2 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrBElog2 ( pBuf, (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zipunzip, ziple32 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrLE ( pBuf, (DWORD)dValues[i] );
	}
}

BENCHMARK_F ( zipunzip, zipbe64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrBE ( pBuf, dValues[i] );
	}
}

BENCHMARK_F ( zipunzip, zipbe64log2 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrBElog2 ( pBuf, dValues[i] );
	}
}

BENCHMARK_F ( zipunzip, ziple64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			pBuf += ZipToPtrLE ( pBuf, dValues[i] );
	}
}

template<typename T, typename READER>
T UnzipValueLE2 ( READER fnGet )
{
	register BYTE b = fnGet();
	register BYTE iOff = 0;
	register T res = 0;
#pragma clang loop unroll_count( sizeof( T ) * CHAR_BIT / 7 )
	for (; iOff < sizeof(T)*CHAR_BIT && ( b & 0x80 ); iOff+=7, b = fnGet() )
		res |= ( T ( b & 0x7FU ) ) << iOff;

	return res | ( T ( b ) ) << iOff;
}

inline DWORD UnzipIntBEref ( const BYTE*& pBuf )
{
	return UnzipValueBE_reference<DWORD> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline DWORD UnzipIntLEref ( const BYTE*& pBuf )
{
	return UnzipValueLE_reference<DWORD> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline DWORD UnzipIntLE2 ( const BYTE*& pBuf )
{
	return UnzipValueLE2<DWORD> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline SphOffset_t UnzipOffsetBEref ( const BYTE*& pBuf )
{
	return UnzipValueBE_reference<SphOffset_t> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline SphOffset_t UnzipOffsetLEref ( const BYTE*& pBuf )
{
	return UnzipValueLE_reference<SphOffset_t> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline SphOffset_t UnzipOffsetLE2 ( const BYTE*& pBuf )
{
	return UnzipValueLE2<SphOffset_t> ( [&pBuf]() mutable { return *pBuf++; } );
}

BENCHMARK_F ( zipunzip, unzipbe32_fastest )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipIntBE ( pBuf );
	}
}


BENCHMARK_F ( zipunzip, unzipbe32ref )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipIntBEref ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unziple32_fastest )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipIntLE ( pBuf );
	}
}


BENCHMARK_F ( zipunzip, unziple32ref )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipIntLEref ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unziple32opt )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipIntLE2 ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unzipbe64_fastest )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipOffsetBE ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unzipbe64ref )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipOffsetBEref ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unziple64_fastest )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipOffsetLE ( pBuf );
	}
}


BENCHMARK_F ( zipunzip, unziple64ref )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipOffsetLEref ( pBuf );
	}
}

BENCHMARK_F ( zipunzip, unziple64opt )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < 1024; ++i )
			iRes += UnzipOffsetLE2 ( pBuf );
	}
}

/*

Run on (8 X 3900 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
Load Average: 0.41, 0.13, 0.45
---------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations
---------------------------------------------------------------------------------------
bench_popcount/artificial_no_division              2.05 ns         2.05 ns    340980780
bench_popcount/artificial_with_division            2.75 ns         2.75 ns    254919225
bench_popcount/artificial_long_no_division         2.05 ns         2.05 ns    340869726
bench_popcount/intrinsic_dword                     1.88 ns         1.88 ns    372735795
bench_popcount/intrinsic_uint64                    1.88 ns         1.88 ns    372419634
bench_popcount/artificial_byte                     1.09 ns         1.09 ns    640643179

 Broadcom BCM2711, Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz

Run on (4 X 1500 MHz CPU s)
Load Average: 1.75, 2.63, 1.61
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
---------------------------------------------------------------------------------------
Benchmark                                             Time             CPU   Iterations
---------------------------------------------------------------------------------------
bench_popcount/artificial_no_division              5.57 ns         5.57 ns    125755867
bench_popcount/artificial_with_division            6.16 ns         6.15 ns    113984067
bench_popcount/artificial_long_no_division         6.02 ns         6.01 ns    116447607
bench_popcount/intrinsic_dword                     3.89 ns         3.88 ns    180829207
bench_popcount/intrinsic_uint64                    3.95 ns         3.94 ns    177579307
bench_popcount/artificial_byte                     3.56 ns         3.56 ns    196519955

*/
