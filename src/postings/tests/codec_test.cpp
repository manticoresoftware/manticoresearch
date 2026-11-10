// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "codec.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif

using namespace postings;
static constexpr StreamDesc_t tStream { Codec_e::PFOR };
static size_t g_iChecks = 0;
static uint64_t g_uBytesHash = 14695981039346656037ull;
#define CHECK(x) do { ++g_iChecks; if (!(x)) { std::cerr << __LINE__ << ": " << #x << "\n"; std::exit(1); } } while (false)
static Bytes_t View ( const std::vector<uint8_t> & d ) { return { d.data(), d.size() }; }

static void RoundTrip ( const std::vector<uint32_t> & dValues, Codec_e eCodec )
{
	const StreamDesc_t tStream { eCodec };
	std::string sError;
	std::vector<uint8_t> dScalar, dAuto;
	CHECK ( EncodeIntegers ( tStream, dValues.data(), dValues.size(), dScalar, sError, Backend_e::SCALAR ) );
	CHECK ( EncodeIntegers ( tStream, dValues.data(), dValues.size(), dAuto, sError ) );
	CHECK ( dScalar==dAuto );
	for ( uint8_t uByte : dScalar )
		g_uBytesHash = (g_uBytesHash ^ uByte)*1099511628211ull;
	IntegerDesc_t tDesc;
	CHECK ( InspectIntegers ( tStream, View(dAuto), tDesc, sError ) );
	CHECK ( tDesc.m_uCount==dValues.size() );
	for ( auto eBackend : { Backend_e::SCALAR, Backend_e::AUTO } )
		for ( auto fnDecode : { DecodeIntegers, DecodeIntegersInto } )
			for ( unsigned uOffset=1; uOffset<=4; ++uOffset )
			{
				alignas(16) std::array<uint32_t,MAX_VALUES+8> dDecoded;
				dDecoded.fill(0xdeadbeef);
				CHECK ( fnDecode ( tStream, View(dAuto), dDecoded.data()+uOffset, dValues.size(), sError, eBackend ) );
				CHECK ( std::equal ( dValues.begin(), dValues.end(), dDecoded.begin()+uOffset ) );
				CHECK ( dDecoded[uOffset-1]==0xdeadbeef && dDecoded[dValues.size()+uOffset]==0xdeadbeef );
			}

	// Input alignment must have no effect on decoding or persistent bytes.
	for ( unsigned uOffset=1; uOffset<=16; ++uOffset )
	{
		std::vector<uint8_t> dUnaligned ( uOffset+dAuto.size() );
		std::copy ( dAuto.begin(), dAuto.end(), dUnaligned.begin()+uOffset );
		alignas(16) uint32_t dDecoded[MAX_VALUES];
		for ( auto fnDecode : { DecodeIntegers, DecodeIntegersInto } )
		{
			CHECK ( fnDecode ( tStream, { dUnaligned.data()+uOffset, dAuto.size() }, dDecoded, MAX_VALUES, sError, Backend_e::AUTO ) );
			CHECK ( std::equal ( dValues.begin(), dValues.end(), dDecoded ) );
		}
	}
}


static void Golden()
{
	std::string sError;
	std::vector<uint8_t> dEncoded;
	// Four independent lanes, 32 one-bit values in each lane.
	std::vector<uint32_t> dValues(128);
	for ( size_t i=0; i<dValues.size(); ++i )
		dValues[i] = (i%4==0 || (i%4==1 && i/4%2==0) || (i%4==2 && i/4==31));

	CHECK ( EncodeIntegers ( { Codec_e::FOR }, dValues.data(), dValues.size(), dEncoded, sError ) );
	const std::vector<uint8_t> dGolden { 1,0,128,0,16,0, 255,255,255,255, 85,85,85,85, 0,0,0,128, 0,0,0,0 };
	CHECK ( dEncoded==dGolden );

	dValues.assign ( 256, 0 );
	dValues[0] = 1;
	dValues[255] = 255;
	CHECK ( EncodeIntegers ( { Codec_e::PFOR }, dValues.data(), dValues.size(), dEncoded, sError ) );
	CHECK ( dEncoded==std::vector<uint8_t> ( { 0,2,0,1,4,0,0,1,255,255 } ) );

	const uint32_t uMax = UINT32_MAX;
	CHECK ( EncodeIntegers ( { Codec_e::VARINT }, &uMax, 1, dEncoded, sError ) );
	CHECK ( dEncoded==std::vector<uint8_t> ( { 0,0,1,0,5,0,255,255,255,255,15 } ) );
	CHECK ( EncodeIntegers ( { Codec_e::CONSTANT }, &uMax, 1, dEncoded, sError ) );
	CHECK ( dEncoded==std::vector<uint8_t> ( { 0,0,1,0,4,0,255,255,255,255 } ) );
}


static void Streams()
{
	std::string sError;
	std::vector<uint8_t> dHeader;
	for ( auto eCodec : { Codec_e::FOR, Codec_e::PFOR, Codec_e::VARINT, Codec_e::CONSTANT } )
	{
		CHECK ( WriteStreamDescriptor ( { eCodec }, dHeader, sError ) );
		CHECK ( dHeader==std::vector<uint8_t> ( { uint8_t(eCodec),0 } ) );
		StreamDesc_t tRead;
		CHECK ( ReadStreamDescriptor ( View(dHeader), tRead, sError ) );
		CHECK ( tRead.m_eCodec==eCodec );

		// One descriptor governs multiple frames, including the short final block.
		for ( size_t n : {256u,128u,3u} )
		{
			std::vector<uint32_t> dValues ( n, uint32_t(n) );
			std::vector<uint8_t> dBlock;
			CHECK ( EncodeIntegers ( tRead, dValues.data(), n, dBlock, sError ) );
			uint32_t dOutput[MAX_VALUES];
			CHECK ( DecodeIntegers ( tRead, View(dBlock), dOutput, MAX_VALUES, sError ) );
			CHECK ( std::equal ( dValues.begin(), dValues.end(), dOutput ) );
		}
	}

	for ( auto dBad : { std::vector<uint8_t>{0,0}, {255,255}, {1,1}, {5,0} } )
	{
		StreamDesc_t tUnchanged { Codec_e::PFOR };
		CHECK ( !ReadStreamDescriptor ( View(dBad), tUnchanged, sError ) );
		CHECK ( tUnchanged.m_eCodec==Codec_e::PFOR );
	}

	CHECK ( !WriteStreamDescriptor ( {}, dHeader, sError ) );
	const uint32_t dMixed[] = {1,2};
	CHECK ( !EncodeIntegers ( { Codec_e::CONSTANT }, dMixed, 2, dHeader, sError ) );
	CHECK ( EncodeIntegers ( tStream, dMixed, 2, dHeader, sError ) );
	IntegerDesc_t tDesc;
	CHECK ( !InspectIntegers ( {}, View(dHeader), tDesc, sError ) );
}


static void Integers()
{
	std::mt19937 tRandom(0x2777);
	for ( size_t iCount : { 1u,2u,31u,127u,128u,129u,255u,256u } )
		for ( unsigned uWidth=0; uWidth<=32; ++uWidth )
		{
			std::vector<uint32_t> dValues(iCount);
			const uint32_t uMask = uWidth ? UINT32_MAX>>(32-uWidth) : 0;
			for ( auto & uValue : dValues )
				uValue = tRandom() & uMask;

			for ( auto eCodec : { Codec_e::FOR, Codec_e::PFOR, Codec_e::VARINT } )
				RoundTrip ( dValues, eCodec );

			std::fill ( dValues.begin(), dValues.end(), uMask );
			RoundTrip ( dValues, Codec_e::CONSTANT );
		}

	for ( unsigned uCount=0; uCount<=8; ++uCount )
	{
		std::vector<uint32_t> dValues ( 256, 1 );
		for ( unsigned i=0; i<uCount; ++i )
			dValues[255-i] = 511;

		RoundTrip ( dValues, Codec_e::PFOR );
	}
}


static void Corruption()
{
	std::string sError;
	IntegerDesc_t tDesc;
	CHECK ( !InspectIntegers ( tStream, {}, tDesc, sError ) );
	uint32_t uValue = 0;
	std::vector<uint8_t> dUnchanged {42};
	CHECK ( !EncodeIntegers ( { Codec_e(255) }, &uValue, 1, dUnchanged, sError ) );
	CHECK ( dUnchanged==std::vector<uint8_t>{42} );
	if ( !HasSimd() )
		CHECK ( !EncodeIntegers ( { Codec_e::PFOR }, &uValue, 1, dUnchanged, sError, Backend_e::SIMD ) );

	std::vector<uint32_t> dValues ( 256, 1 );
	dValues[255] = 511;
	std::vector<uint8_t> dGood;
	CHECK ( EncodeIntegers ( { Codec_e::PFOR }, dValues.data(), dValues.size(), dGood, sError ) );
	CHECK ( dGood[1]==1 );
	for ( size_t i=0; i<dGood.size(); ++i )
		CHECK ( !InspectIntegers ( tStream, { dGood.data(), i }, tDesc, sError ) );

	for ( auto tEdit : { std::pair<size_t,uint8_t>(0,33), {1,8}, {2,1}, {3,2}, {4,0}, {dGood.size()-1,0} } )
	{
		auto dBad = dGood;
		dBad[tEdit.first] = tEdit.second;
		CHECK ( !InspectIntegers ( tStream, View(dBad), tDesc, sError ) );
	}

	for ( auto dBad : { std::vector<uint8_t>{0,0,1,0,2,0,128,0}, {0,0,1,0,5,0,255,255,255,255,31} } )
		CHECK ( !InspectIntegers ( { Codec_e::VARINT }, View(dBad), tDesc, sError ) );

	std::vector<uint8_t> dPadded ( INTEGER_HEADER_SIZE+16, 0 );
	dPadded[0] = 1;
	dPadded[2] = 1;
	dPadded[4] = 16;
	dPadded[6] = 1;
	for ( auto eCodec : { Codec_e::FOR, Codec_e::PFOR } )
		CHECK ( !InspectIntegers ( {eCodec}, View(dPadded), tDesc, sError ) );

	const std::vector<uint8_t> dBadWidth {32,1,1,0,0,0};
	CHECK ( !InspectIntegers ( tStream, View(dBadWidth), tDesc, sError ) );

	// Fuzz descriptor and payload parsing. Valid mutations must be backend-equivalent.
	std::mt19937 tRandom(7731);
	for ( unsigned i=0; i<20000; ++i )
	{
		auto dBad = dGood;
		dBad[tRandom()%dBad.size()] ^= uint8_t(1u<<(tRandom()%8));
		uint32_t dScalar[MAX_VALUES], dAuto[MAX_VALUES];
		if ( InspectIntegers ( tStream, View(dBad), tDesc, sError ) )
		{
			CHECK ( DecodeIntegers ( tStream, View(dBad), dScalar, MAX_VALUES, sError, Backend_e::SCALAR ) );
			CHECK ( DecodeIntegers ( tStream, View(dBad), dAuto, MAX_VALUES, sError ) );
			CHECK ( std::equal ( dScalar, dScalar+tDesc.m_uCount, dAuto ) );
		}
	}
}


static void Rows()
{
	std::string sError;
	std::mt19937 tRandom(17);
	for ( unsigned uCount : {1u,2u,127u,128u,129u,256u} )
		for ( unsigned uStep : {1u,2u,31u,1000000u} )
			for ( uint32_t uBase : {0u,UINT32_MAX-1-uCount*uStep} )
			{
				std::vector<uint32_t> dRows(uCount);
				for ( unsigned i=0; i<uCount; ++i )
					dRows[i] = uBase+i*uStep;

				for ( auto eKind : { Rows_e::AUTO, Rows_e::DELTA, Rows_e::BITMAP, Rows_e::RANGE } )
				{
					std::vector<uint8_t> dEncoded;
					if ( eKind==Rows_e::BITMAP && uint64_t(dRows.back())-dRows.front()+1>uCount*32 )
						continue;
					if ( eKind==Rows_e::RANGE && uStep!=1 && uCount!=1 )
						continue;

					CHECK ( EncodeRows ( tStream, dRows.data(), dRows.size(), dEncoded, sError, eKind ) );
					RowBlock_t tBlock;
					CHECK ( InspectRows ( tStream, View(dEncoded), tBlock, sError ) );
					CHECK ( tBlock.m_uCount==uCount && tBlock.m_uFirst==dRows.front() && tBlock.m_uLast==dRows.back() );
					for ( auto eBackend : { Backend_e::SCALAR, Backend_e::AUTO } )
						for ( auto fnDecode : { DecodeRows, DecodeRowsInto } )
							for ( unsigned uOffset=1; uOffset<=4; ++uOffset )
							{
								alignas(16) std::array<uint32_t,MAX_VALUES+8> dDecoded;
								dDecoded.fill(0xdeadbeef);
								CHECK ( fnDecode ( tStream, View(dEncoded), dDecoded.data()+uOffset, uCount, sError, eBackend ) );
								CHECK ( std::equal ( dRows.begin(), dRows.end(), dDecoded.begin()+uOffset ) );
								CHECK ( dDecoded[uOffset-1]==0xdeadbeef && dDecoded[uCount+uOffset]==0xdeadbeef );
							}

					for ( size_t i=0; i<dEncoded.size(); ++i )
						CHECK ( !InspectRows ( tStream, { dEncoded.data(), i }, tBlock, sError ) );

					for ( unsigned i=0; i<100; ++i )
					{
						auto dBad = dEncoded;
						dBad[tRandom()%dBad.size()] ^= uint8_t(1u<<(tRandom()%8));
						std::array<uint32_t,MAX_VALUES> dDecoded;
						if ( InspectRows ( tStream, View(dBad), tBlock, sError ) )
						{
							CHECK ( DecodeRows ( tStream, View(dBad), dDecoded.data(), MAX_VALUES, sError ) );
							CHECK ( std::adjacent_find ( dDecoded.begin(), dDecoded.begin()+tBlock.m_uCount, std::greater_equal<uint32_t>() )==dDecoded.begin()+tBlock.m_uCount );
						}
					}
				}
			}
}


static void GuardPages ( StreamDesc_t tStream )
{
#ifndef _WIN32
	const size_t iPage = size_t(sysconf(_SC_PAGESIZE));
	auto * pMap = static_cast<uint8_t *>( mmap ( nullptr, iPage*2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) );
	CHECK ( pMap!=MAP_FAILED );
	CHECK ( !mprotect ( pMap+iPage, iPage, PROT_NONE ) );
	auto * pOutMap = static_cast<uint8_t *>( mmap ( nullptr, iPage*2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) );
	CHECK ( pOutMap!=MAP_FAILED );
	CHECK ( !mprotect ( pOutMap+iPage, iPage, PROT_NONE ) );
	std::string sError;
	for ( size_t n : {1u,127u,128u,129u,256u} )
		for ( unsigned b=0; b<=32; ++b )
		{
			std::vector<uint32_t> dValues ( n, b ? UINT32_MAX>>(32-b) : 0 );
			if ( n>1 )
				dValues.back() /= 2;

			std::vector<uint8_t> dEncoded;
			CHECK ( EncodeIntegers ( tStream, dValues.data(), n, dEncoded, sError ) );
			auto * pInput = pMap+iPage-dEncoded.size();
			std::copy ( dEncoded.begin(), dEncoded.end(), pInput );
			uint32_t dDecoded[MAX_VALUES];
			CHECK ( DecodeIntegers ( tStream, { pInput, dEncoded.size() }, dDecoded, MAX_VALUES, sError ) );
			CHECK ( std::equal ( dValues.begin(), dValues.end(), dDecoded ) );
			auto * pDirect = reinterpret_cast<uint32_t *>(pOutMap+iPage-n*sizeof(uint32_t));
			CHECK ( DecodeIntegersInto ( tStream, { pInput, dEncoded.size() }, pDirect, n, sError ) );
			CHECK ( std::equal ( dValues.begin(), dValues.end(), pDirect ) );
			// Output ends exactly at the guard page and can overlap the encoded input.
			auto * pOutput = reinterpret_cast<uint32_t *>(pMap+iPage-n*sizeof(uint32_t));
			CHECK ( DecodeIntegers ( tStream, { pInput, dEncoded.size() }, pOutput, n, sError ) );
			CHECK ( std::equal ( dValues.begin(), dValues.end(), pOutput ) );
			std::vector<uint8_t> dCopy;
			CHECK ( EncodeIntegers ( tStream, pOutput, n, dCopy, sError ) );
			CHECK ( dEncoded==dCopy );
		}

	for ( size_t n : {1u,127u,128u,129u,256u} )
		for ( unsigned uStep : {1u,2u,1000u} )
		{
			std::vector<uint32_t> dRows(n);
			for ( size_t i=0; i<n; ++i )
				dRows[i] = uint32_t(i)*uStep;

			std::vector<uint8_t> dEncoded;
			CHECK ( EncodeRows ( tStream, dRows.data(), n, dEncoded, sError ) );
			auto * pInput = pMap+iPage-dEncoded.size();
			std::copy ( dEncoded.begin(), dEncoded.end(), pInput );
			auto * pOutput = reinterpret_cast<uint32_t *>(pOutMap+iPage-n*sizeof(uint32_t));
			CHECK ( DecodeRowsInto ( tStream, { pInput, dEncoded.size() }, pOutput, n, sError ) );
			CHECK ( std::equal ( dRows.begin(), dRows.end(), pOutput ) );
			pOutput = reinterpret_cast<uint32_t *>(pMap+iPage-n*sizeof(uint32_t));
			CHECK ( DecodeRows ( tStream, { pInput, dEncoded.size() }, pOutput, n, sError ) );
			CHECK ( std::equal ( dRows.begin(), dRows.end(), pOutput ) );
		}

	CHECK ( !munmap ( pOutMap, iPage*2 ) );
	CHECK ( !munmap ( pMap, iPage*2 ) );
#endif
}


static void ShortArraysAndTails ( Codec_e eCodec )
{
	std::string sError;
	std::vector<uint8_t> dEncoded;
	std::mt19937 tRandom(0x20261110);
	for ( unsigned n=1; n<=MAX_VALUES; ++n )
	{
		std::vector<uint32_t> dValues(n);
		for ( auto & uValue : dValues )
			uValue = tRandom();

		RoundTrip ( dValues, eCodec );
		for ( uint32_t uValue : {0U,1U,UINT32_MAX} )
		{
			std::fill ( dValues.begin(), dValues.end(), uValue );
			RoundTrip ( dValues, eCodec );
			CHECK ( EncodeIntegers ( {eCodec}, dValues.data(), n, dEncoded, sError ) );
			CHECK ( dEncoded.size()==INTEGER_HEADER_SIZE+(uValue==UINT32_MAX ? 5 : 1) );
		}
	}

	const uint32_t dShort[] = {1,128,UINT32_MAX};
	CHECK ( EncodeIntegers ( {eCodec}, dShort, 3, dEncoded, sError ) );
	CHECK ( dEncoded==std::vector<uint8_t> ( {34,0,3,0,8,0,1,128,1,255,255,255,255,15} ) );
	IntegerDesc_t tDesc;
	for ( unsigned i=0; i<dEncoded.size(); ++i )
		CHECK ( !InspectIntegers ( {eCodec}, {dEncoded.data(),i}, tDesc, sError ) );

	std::vector<uint32_t> dValues ( 129, 0 );
	dValues[127] = 255;
	dValues[128] = UINT32_MAX;
	CHECK ( EncodeIntegers ( {Codec_e::PFOR}, dValues.data(), dValues.size(), dEncoded, sError ) );
	CHECK ( dEncoded==std::vector<uint8_t> ( {0,1,129,0,7,0,127,255,255,255,255,255,15} ) );
	CHECK ( InspectIntegers ( {Codec_e::PFOR}, View(dEncoded), tDesc, sError ) );
	dEncoded[6] = 128;
	CHECK ( !InspectIntegers ( {Codec_e::PFOR}, View(dEncoded), tDesc, sError ) );
	dEncoded[6] = 127;
	dEncoded.back() = 128;
	CHECK ( !InspectIntegers ( {Codec_e::PFOR}, View(dEncoded), tDesc, sError ) );

	CHECK ( WriteStreamDescriptor ( {Codec_e::BASE_DELTA}, dEncoded, sError ) );
	StreamDesc_t tRead;
	CHECK ( ReadStreamDescriptor ( View(dEncoded), tRead, sError ) && tRead.m_eCodec==Codec_e::BASE_DELTA );
	CHECK ( !EncodeIntegers ( tRead, dShort, 3, dEncoded, sError ) );
}


int main()
{
	ShortArraysAndTails ( Codec_e::FOR );
	ShortArraysAndTails ( Codec_e::PFOR );
	Golden();
	Streams();
	Integers();
	Corruption();
	Rows();
	GuardPages ( {Codec_e::FOR} );
	GuardPages ( {Codec_e::PFOR} );
	std::cout << "PASS " << g_iChecks << " checks; backend=" << BackendName() << "; bytes_hash=" << g_uBytesHash << "\n";
}
