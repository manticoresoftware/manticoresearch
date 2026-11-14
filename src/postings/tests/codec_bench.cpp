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
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

using namespace postings;
using Clock_t = std::chrono::steady_clock;
using Values_t = std::vector<uint32_t>;
static volatile uint64_t g_uChecksum = 0;

static void Check ( bool bOk, const std::string & sError )
{
	if ( !bOk )
		throw std::runtime_error(sError);
}

static const char * CodecName ( Codec_e eCodec )
{
	switch ( eCodec )
	{
	case Codec_e::FOR: return "for";
	case Codec_e::PFOR: return "pfor";
	case Codec_e::VARINT: return "varint";
	case Codec_e::CONSTANT: return "constant";
	default: return "invalid";
	}
}

// Benchmark-only policy: select once using the complete sampled stream.
static StreamDesc_t SelectStream ( const std::vector<Values_t> & dBlocks, bool bRows )
{
	StreamDesc_t tBest;
	size_t iBest = size_t(-1);
	std::string sError;
	std::vector<uint8_t> dEncoded;
	for ( auto eCodec : { Codec_e::FOR, Codec_e::PFOR, Codec_e::VARINT, Codec_e::CONSTANT } )
	{
		size_t iBytes = STREAM_DESCRIPTOR_SIZE;
		bool bValid = true;
		for ( const auto & d : dBlocks )
		{
			bValid = bRows ? EncodeRows ( { eCodec }, d.data(), d.size(), dEncoded, sError ) : EncodeIntegers ( { eCodec }, d.data(), d.size(), dEncoded, sError );
			if ( !bValid )
				break;

			iBytes += dEncoded.size();
		}

		if ( bValid && iBytes<iBest )
		{
			iBest = iBytes;
			tBest.m_eCodec = eCodec;
		}
	}

	Check ( tBest.m_eCodec!=Codec_e::INVALID, "No stream codec available" );
	return tBest;
}

static void Bench ( const std::string & sName, const std::vector<Values_t> & dBlocks, Codec_e eCodec, const char * sCodec, Backend_e eBackend, bool bRows, size_t iGroup )
{
	const StreamDesc_t tStream = eCodec==Codec_e::INVALID ? SelectStream ( dBlocks, bRows ) : StreamDesc_t { eCodec };
	std::string sError;
	std::vector<uint8_t> dDescriptor;
	Check ( WriteStreamDescriptor ( tStream, dDescriptor, sError ), sError );
	std::vector<std::vector<uint8_t>> dEncoded(dBlocks.size());
	size_t iValues = 0, iBytes = dDescriptor.size();
	uint32_t dDecoded[MAX_VALUES];
	for ( size_t i=0; i<dBlocks.size(); ++i )
	{
		const auto & d = dBlocks[i];
		Check ( bRows ? EncodeRows ( tStream, d.data(), d.size(), dEncoded[i], sError, Rows_e::AUTO, eBackend ) :
			EncodeIntegers ( tStream, d.data(), d.size(), dEncoded[i], sError, eBackend ), sError );
		Bytes_t tInput { dEncoded[i].data(), dEncoded[i].size() };
		Check ( bRows ? DecodeRows ( tStream, tInput, dDecoded, MAX_VALUES, sError, eBackend ) :
			DecodeIntegers ( tStream, tInput, dDecoded, MAX_VALUES, sError, eBackend ), sError );
		Check ( std::equal ( d.begin(), d.end(), dDecoded ), "round-trip mismatch" );
		iValues += d.size();
		iBytes += dEncoded[i].size();
	}

	const size_t iRepeats = std::max ( size_t(2), size_t(300000)/iValues );
	std::array<double,5> dEncodeTimes, dDecodeTimes;
	for ( unsigned s=0; s<5; ++s )
	{
		auto tStart = Clock_t::now();
		std::vector<uint8_t> dTemp;
		for ( size_t r=0; r<iRepeats; ++r )
			for ( const auto & d : dBlocks )
			{
				Check ( bRows ? EncodeRows ( tStream, d.data(), d.size(), dTemp, sError, Rows_e::AUTO, eBackend ) :
					EncodeIntegers ( tStream, d.data(), d.size(), dTemp, sError, eBackend ), sError );
				g_uChecksum += dTemp.size();
			}

		dEncodeTimes[s] = std::chrono::duration<double,std::nano>(Clock_t::now()-tStart).count()/(iValues*iRepeats);
		tStart = Clock_t::now();
		for ( size_t r=0; r<iRepeats; ++r )
			for ( const auto & d : dEncoded )
			{
				Bytes_t tInput { d.data(), d.size() };
				Check ( bRows ? DecodeRows ( tStream, tInput, dDecoded, MAX_VALUES, sError, eBackend ) :
					DecodeIntegers ( tStream, tInput, dDecoded, MAX_VALUES, sError, eBackend ), sError );
				g_uChecksum += dDecoded[0];
			}

		dDecodeTimes[s] = std::chrono::duration<double,std::nano>(Clock_t::now()-tStart).count()/(iValues*iRepeats);
	}

	std::sort ( dEncodeTimes.begin(), dEncodeTimes.end() );
	std::sort ( dDecodeTimes.begin(), dDecodeTimes.end() );
	std::cout << sName << ',' << iGroup << ',' << sCodec << ',' << CodecName(tStream.m_eCodec) << ',' << BackendName(eBackend) << ',' << dBlocks.size() << ','
		<< iValues << ',' << iBytes << ',' << double(iBytes)/iValues << ',' << dEncodeTimes[2] << ',' << dDecodeTimes[2] << '\n';
}

int main ( int argc, char ** argv )
{
	try
	{
		std::map<std::string,std::vector<Values_t>> dData;
		std::mt19937 tRandom(2777);
		for ( unsigned i=0; i<128; ++i )
		{
			Values_t dSmall(512), dPatched(512), dFull(512), dConstant ( 512, 1 ), dRows(512);
			for ( unsigned j=0; j<512; ++j )
			{
				dSmall[j] = tRandom()%8;
				dPatched[j] = j%127 ? dSmall[j] : 511;
				dFull[j] = tRandom();
				dRows[j] = j*2+i*4096;
			}

			dData["synthetic-small"].push_back(dSmall);
			dData["synthetic-patched"].push_back(dPatched);
			dData["synthetic-u32"].push_back(dFull);
			dData["synthetic-constant"].push_back(dConstant);
			dData["rows-synthetic"].push_back(dRows);
		}

		if ( argc>1 )
		{
			std::ifstream tFile(argv[1]);
			Check ( bool(tFile), "Cannot open corpus" );
			std::string sLine;
			while ( std::getline ( tFile, sLine ) )
			{
				std::istringstream tLine(sLine);
				std::string sName;
				size_t iCount;
				Check ( bool(tLine>>sName>>iCount) && iCount>0 && iCount<=512, "Invalid corpus record" );
				Values_t dValues(iCount);
				for ( auto & uValue : dValues )
					Check ( bool(tLine>>uValue), "Invalid corpus value" );

				dData[sName].push_back(std::move(dValues));
			}
		}

		std::cout << "dataset,group,codec,selected_codec,backend,frames,values,bytes,bytes_per_value,encode_ns_per_value,decode_ns_per_value\n";
		for ( const auto & tData : dData )
			for ( size_t iGroup : {128u,256u,512u} )
			{
				// A 512-value experiment is two independent <=256-value frames.
				std::vector<Values_t> dBlocks;
				for ( const auto & d : tData.second )
					for ( size_t i=0; i<d.size(); i+=std::min ( iGroup, MAX_VALUES ) )
						dBlocks.emplace_back ( d.begin()+i, d.begin()+std::min ( d.size(), i+std::min ( iGroup, MAX_VALUES ) ) );

				const bool bRows = tData.first.rfind("rows-",0)==0;
				for ( auto eBackend : { Backend_e::SCALAR, Backend_e::AUTO } )
					for ( const auto & tCodec : { std::pair<Codec_e,const char *>(Codec_e::INVALID,"stream-auto"), {Codec_e::FOR,"for"}, {Codec_e::PFOR,"pfor"}, {Codec_e::VARINT,"varint"} } )
					{
						if ( bRows && tCodec.first!=Codec_e::INVALID )
							continue;

						Bench ( tData.first, dBlocks, tCodec.first, tCodec.second, eBackend, bRows, iGroup );
					}
			}
	} catch ( const std::exception & tError )
	{
		std::cerr << tError.what() << '\n';
		return 1;
	}

	return 0;
}
