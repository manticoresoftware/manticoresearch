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

#include <benchmark/benchmark.h>

#include "sphinxstd.h"

inline int BitCountDW1 ( DWORD n )
{
	// MIT HACKMEM count
	// works for 32-bit numbers only
	// fix last line for 64-bit numbers
	DWORD tmp;
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
		dValues.Resize ( 128+8 );
		for ( auto & c: dValues )
			c = ( (uint64_t)sphRand() << 32 ) | sphRand();
		i = 0;
		NRUNS = state.range ( 0 );
	}

	CSphVector<uint64_t> dValues;
	int	i;
	int64_t NRUNS = 0;
};

BENCHMARK_DEFINE_F( bench_popcount, artificial_no_division ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( BitCountDW2 ( dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS * sizeof ( DWORD ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ("DWORD BitCountDW2");
}

BENCHMARK_DEFINE_F ( bench_popcount, artificial_with_division ) ( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( BitCountDW1 ( dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS * sizeof ( DWORD ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ( "DWORD BitCountDW1" );
}

BENCHMARK_DEFINE_F ( bench_popcount, artificial_long_no_division ) ( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( BitCountU641 ( dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS * sizeof ( uint64_t ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ( "uint64_t BitCountU641" );
}

BENCHMARK_DEFINE_F ( bench_popcount, intrinsic_dword ) ( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( sphBitCount ( (DWORD)dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS * sizeof ( DWORD ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ( "DWORD sphBitCount" );
}

BENCHMARK_DEFINE_F ( bench_popcount, intrinsic_uint64 ) ( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( sphBitCount ( dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS * sizeof ( uint64_t ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ( "uint64_t sphBitCount" );
}

BENCHMARK_DEFINE_F ( bench_popcount, artificial_byte ) ( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( auto j = 0; j < NRUNS; ++j )
		{
			benchmark::DoNotOptimize ( sphBitCount ( (BYTE)dValues[++i] ) );
			if ( i > 127 )
				i = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetLabel ( "BYTE sphBitCount" );
}

BENCHMARK_REGISTER_F ( bench_popcount, artificial_no_division )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( bench_popcount, artificial_with_division )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( bench_popcount, artificial_long_no_division )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( bench_popcount, intrinsic_dword )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( bench_popcount, intrinsic_uint64 )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( bench_popcount, artificial_byte )->Range ( 1, 4096 );

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

static void BM_zippedlen64 ( benchmark::State& st )
{
	uint64_t uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( sphCalcZippedLen ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_zippedlen32 ( benchmark::State& st )
{
	DWORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( sphCalcZippedLen ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_zippedlen16 ( benchmark::State& st )
{
	WORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( sphCalcZippedLen ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK ( BM_zippedlen64 )->DenseRange ( 0, 64, 4 );
BENCHMARK ( BM_zippedlen32 )->DenseRange ( 0, 32, 4 );
BENCHMARK ( BM_zippedlen16 )->DenseRange ( 0, 16, 4 );


static void BM_zippedlenlog2_64 ( benchmark::State& st )
{
	uint64_t uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( CalcZippedLenViaLog2 ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_zippedlenlog2_32 ( benchmark::State& st )
{
	DWORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( CalcZippedLenViaLog2 ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_zippedlenlog2_16 ( benchmark::State& st )
{
	WORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( CalcZippedLenViaLog2 ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK ( BM_zippedlenlog2_64 )->DenseRange ( 0, 64, 4 );
BENCHMARK ( BM_zippedlenlog2_32 )->DenseRange ( 0, 32, 4 );
BENCHMARK ( BM_zippedlenlog2_16 )->DenseRange ( 0, 16, 4 );


static void BM_log2seq64 ( benchmark::State& st )
{
	uint64_t uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( Log2constUINT ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_log2seq32 ( benchmark::State& st )
{
	DWORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( Log2constUINT ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK ( BM_log2seq64 )->DenseRange ( 0, 64, 4 );
BENCHMARK ( BM_log2seq32 )->DenseRange ( 0, 32, 4 );

static void BM_log2int64 ( benchmark::State& st )
{
	uint64_t uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( sphLog2 ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_log2int32 ( benchmark::State& st )
{
	DWORD uValue = 1 << st.range ( 0 );
	for ( auto _ : st )
		benchmark::DoNotOptimize ( sphLog2 ( uValue ) );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK ( BM_log2int64 )->DenseRange ( 0, 64, 4 );
BENCHMARK ( BM_log2int32 )->DenseRange ( 0, 32, 4 );

#define SPH_VARINT_DECODE( _type, _getexpr ) \
	DWORD b = _getexpr; \
	_type res = 0; \
	while ( b & 0x80 ) \
	{ \
		res = ( res << 7 ) + ( b & 0x7f ); \
		b = _getexpr; \
	} \
	res = ( res << 7 ) + b; \
	return res;

class heavy
{
public:

	virtual int GetByte();

	BYTE* m_pBuff; ///< the buffer
	int m_iBuffPos = 0;
	int m_iBuffUsed = 0;
	bool m_bError;
	CSphString m_sError;

	virtual void UpdateCache() = 0;

	inline DWORD UnzipValueBEDWORD_reference()
	{
		return UnzipValueBE_reference<DWORD> ( [this]() mutable { return GetByte(); } );
	};

	inline DWORD UnzipValueBEDWORD()
	{
		return UnzipValueBE<DWORD> ( [this]() mutable { return GetByte(); } );
	};

	inline DWORD UnzipValueBEDWORD_macro()
	{
		SPH_VARINT_DECODE ( DWORD, GetByte() );
	};
};

class zipunzip: public benchmark::Fixture, public heavy
{
public:
	void SetUp ( const ::benchmark::State& state ) override
	{
		auto NSEPTETS = state.range ( 0 );
		uint64_t uMask = NSEPTETS > 9 ? 0xFFFFFFFFFFFFFFFFULL : ( ( 1ULL << NSEPTETS * 7 ) - 1ULL );

		auto NRUNS = state.range ( 1 );
		dValues.Resize ( NRUNS );
		dBufBE.Reserve ( NRUNS * 10 );
		dBufBE.Resize ( 0 );
		dBufLE.Reserve ( NRUNS * 10 );
		dBufLE.Resize ( 0 );
		dBufBE64.Reserve ( NRUNS * 10 );
		dBufBE64.Resize ( 0 );
		dBufLE64.Reserve ( NRUNS * 10 );
		dBufLE64.Resize ( 0 );

		sphSrand ( 0 );
		for ( auto& c : dValues )
		{
			uint64_t uVal = sphRand();
			uVal = ( uVal << 32 ) | sphRand();
			uVal &= uMask;
			c = uVal;
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, uVal & 0xFFFFFFFF );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE.Add ( b ); }, uVal & 0xFFFFFFFF );
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE64.Add ( b ); }, uVal );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE64.Add ( b ); }, uVal );
		}
		iRes = 0;
//		printf ( "SetUp with mask: %d, %p. %d %d %d %d\n", (int)NSEPTETS, (void*)uMask,
//			dBufBE.GetLength(), dBufLE.GetLength(), dBufBE64.GetLength(), dBufLE64.GetLength());
	}

	CSphVector<uint64_t> dValues;
	CSphVector<BYTE> dBufBE;
	CSphVector<BYTE> dBufLE;
	CSphVector<BYTE> dBufBE64;
	CSphVector<BYTE> dBufLE64;
	volatile int iRes = 0;

	void UpdateCache() override;
};

// metrics from real run searchd with index - mostly 1 byte, sometimes 2 (about 12%) and few cases when 3. Never 4 and 5
// for offsets - same way, mostly 1 byte, and about 0.1% when 4. Never 2, 3, 5+
// | unzip32_hist | 1132420000, 136150000, 50000, 0, 0 |
// | unzip64_hist | 7140000, 0, 0, 10000, 0, 0, 0, 0, 0, 0 |
class realunzip: public benchmark::Fixture, public heavy
{
public:
	static constexpr int iProfile = 113242 + 13615 + 5;
	static constexpr int iSize = 113242 * 1 + 13615 * 2 + 5 * 3;
	void SetUp ( const ::benchmark::State& state ) override
	{
		auto NRUNS = state.range ( 0 );
		dValues.Resize ( NRUNS*iProfile );
		dBufBE.Reserve ( NRUNS * iSize );
		dBufBE.Resize ( 0 );

		sphSrand ( 0 );
		uint64_t uMask = ( 1ULL << 1 * 7 ) - 1ULL;
		for ( int i=0; i<113242*NRUNS; ++i )
		{
			uint64_t uVal = sphRand();
			uVal = ( uVal << 32 ) | sphRand();
			uVal &= uMask;
			dValues[i] = uVal;
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, uVal & 0xFFFFFFFF );
		}

		uMask = ( 1ULL << 2 * 7 ) - 1ULL;
		for ( int i = 0; i < 13615 * NRUNS; ++i )
		{
			uint64_t uVal = sphRand();
			uVal = ( uVal << 32 ) | sphRand();
			uVal &= uMask;
			dValues[i] = uVal;
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, uVal & 0xFFFFFFFF );
		}

		uMask = ( 1ULL << 3 * 7 ) - 1ULL;
		for ( int i = 0; i < 5 * NRUNS; ++i )
		{
			uint64_t uVal = sphRand();
			uVal = ( uVal << 32 ) | sphRand();
			uVal &= uMask;
			dValues[i] = uVal;
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, uVal & 0xFFFFFFFF );
		}
		iRes = 0;
		//		printf ( "SetUp with mask: %d, %p. %d %d %d %d\n", (int)NSEPTETS, (void*)uMask,
		//			dBufBE.GetLength(), dBufLE.GetLength(), dBufBE64.GetLength(), dBufLE64.GetLength());
	}

	CSphVector<uint64_t> dValues;
	CSphVector<BYTE> dBufBE;
	volatile int iRes = 0;

	void UpdateCache() override;
};

int heavy::GetByte()
{
	if ( m_iBuffPos >= m_iBuffUsed )
	{
		UpdateCache();
		if ( m_iBuffPos >= m_iBuffUsed )
		{
			m_bError = true;
			m_sError.SetSprintf ( "pread error in: pos=" INT64_FMT ", len=%d", (int64_t)m_iBuffPos, 1 );
			return 0; // unexpected io failure
		}
	}

	assert ( m_iBuffPos < m_iBuffUsed );
	return m_pBuff[m_iBuffPos++];
}

void zipunzip::UpdateCache()
{
	m_iBuffUsed = dBufBE.GetLengthBytes();
}

void realunzip::UpdateCache()
{
	m_iBuffUsed = dBufBE.GetLengthBytes();
}

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

// 32 bits integer packing

BENCHMARK_DEFINE_F ( zipunzip, zipbe32 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrBE ( pBuf, (DWORD)dValues[i] );
		iBytes += pBuf-dBufBE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, zipbe32log2 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrBElog2 ( pBuf, (DWORD)dValues[i] );
		iBytes += pBuf - dBufBE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, ziple32 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrLE ( pBuf, (DWORD)dValues[i] );
		iBytes += pBuf - dBufLE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

static constexpr int MINRANGE = 2;
static constexpr int MAXRANGE=512;
static constexpr int STEPRANGE = 8;

BENCHMARK_REGISTER_F ( zipunzip, zipbe32 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, zipbe32log2 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, ziple32 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );


// 64 bits integers

BENCHMARK_DEFINE_F ( zipunzip, zipbe64 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrBE ( pBuf, dValues[i] );
		iBytes += pBuf - dBufBE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, zipbe64log2 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrBElog2 ( pBuf, dValues[i] );
		iBytes += pBuf - dBufBE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, ziple64 )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			pBuf += ZipToPtrLE ( pBuf, dValues[i] );
		iBytes += pBuf - dBufLE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_REGISTER_F ( zipunzip, zipbe64 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, zipbe64log2 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, ziple64 )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );

template<typename T, typename READER>
T UnzipValueLE2 ( READER fnGet )
{
	BYTE b = fnGet();
	BYTE iOff = 0;
	T res = 0;
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

BENCHMARK_DEFINE_F ( zipunzip, unzipbe32_fastest )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipIntBE ( pBuf ) );
		iBytes += pBuf - dBufBE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}


BENCHMARK_DEFINE_F ( zipunzip, unzipbe32ref )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipIntBEref ( pBuf ) );
		iBytes += pBuf - dBufBE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unzipbe32refGetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD_reference() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unzipbe32GetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unzipbe32macroGetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD_macro() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}


BENCHMARK_DEFINE_F ( zipunzip, unziple32_fastest )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipIntLE ( pBuf ) );
		iBytes += pBuf - dBufLE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}


BENCHMARK_DEFINE_F ( zipunzip, unziple32ref )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipIntLEref ( pBuf ) );
		iBytes += pBuf - dBufLE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unziple32opt )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipIntLE2 ( pBuf ) );
		iBytes += pBuf - dBufLE.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_REGISTER_F ( zipunzip, unzipbe32_fastest )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unzipbe32ref )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple32_fastest )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple32ref )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple32opt )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unzipbe32refGetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unzipbe32macroGetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unzipbe32GetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 5, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );

BENCHMARK_DEFINE_F ( realunzip, unzipbe32refGetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS* iProfile; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD_reference() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( realunzip, unzipbe32GetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS*iProfile; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( realunzip, unzipbe32macroGetByte )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
	{
		m_pBuff = dBufBE.begin();
		m_iBuffPos = m_iBuffUsed = 0;
		for ( auto i = 0; i < NRUNS*iProfile; ++i )
			benchmark::DoNotOptimize ( UnzipValueBEDWORD_macro() );
		iBytes += m_iBuffPos;
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_REGISTER_F ( realunzip, unzipbe32GetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 4, 1 ) } );
BENCHMARK_REGISTER_F ( realunzip, unzipbe32refGetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 4, 1 ) } );
BENCHMARK_REGISTER_F ( realunzip, unzipbe32macroGetByte )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 4, 1 ) } );

BENCHMARK_DEFINE_F ( zipunzip, unzipbe64_fastest )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipOffsetBE ( pBuf ) );
		iBytes += pBuf - dBufBE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unzipbe64ref )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufBE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipOffsetBEref ( pBuf ) );
		iBytes += pBuf - dBufBE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unziple64_fastest )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipOffsetLE ( pBuf ) );
		iBytes += pBuf - dBufLE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unziple64ref )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipOffsetLEref ( pBuf ) );
		iBytes += pBuf - dBufLE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_DEFINE_F ( zipunzip, unziple64opt )
( benchmark::State& st )
{
	int64_t iBytes = 0;
	auto NRUNS = st.range ( 1 );
	for ( auto _ : st )
	{
		const BYTE* pBuf = dBufLE64.begin();
		for ( auto i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( UnzipOffsetLE2 ( pBuf ) );
		iBytes += pBuf - dBufLE64.begin();
	}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_REGISTER_F ( zipunzip, unzipbe64_fastest )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unzipbe64ref )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple64_fastest )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple64ref )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );
BENCHMARK_REGISTER_F ( zipunzip, unziple64opt )->ArgsProduct ( { benchmark::CreateDenseRange ( 1, 10, 1 ), benchmark::CreateRange ( MINRANGE, MAXRANGE, STEPRANGE ) } );


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
