// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "codec.h"
#include "packing.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>

namespace postings
{
namespace
{

bool Fail ( std::string & sError, const char * sMessage )
{
	sError = sMessage;
	return false;
}

uint32_t ReadLE ( const uint8_t * pData, unsigned uBytes )
{
	uint32_t uValue = 0;
	for ( unsigned i=0; i<uBytes; ++i )
		uValue |= uint32_t(pData[i])<<(i*8);

	return uValue;
}

void WriteLE ( uint8_t * pData, uint32_t uValue, unsigned uBytes )
{
	for ( unsigned i=0; i<uBytes; ++i )
		pData[i] = uint8_t(uValue>>(i*8));
}

bool IsKnownCodec ( Codec_e eCodec )
{
	return eCodec==Codec_e::FOR || eCodec==Codec_e::PFOR || eCodec==Codec_e::VARINT || eCodec==Codec_e::CONSTANT;
}

unsigned Width ( uint32_t uValue )
{
	unsigned uWidth = 0;
	while ( uValue )
	{
		++uWidth;
		uValue >>= 1;
	}

	return uWidth;
}

size_t PackedBytes ( size_t iCount, unsigned uWidth )
{
	return ((iCount+127)/128)*16*uWidth;
}

void Pack ( const uint32_t * pValues, size_t iCount, unsigned uWidth, uint8_t * pOutput, const Packing_t & tPacking )
{
	alignas(16) uint32_t dValues[128];
	alignas(16) uint32_t dPacked[128];
	for ( size_t i=0; i<iCount; i+=128 )
	{
		std::fill_n ( dValues, 128, 0 );
		std::copy_n ( pValues+i, std::min ( size_t(128), iCount-i ), dValues );
		tPacking.m_fnPack ( dValues, dPacked, uWidth );
		for ( unsigned j=0; j<4*uWidth; ++j )
			WriteLE ( pOutput+j*4, dPacked[j], 4 );

		pOutput += 16*uWidth;
	}
}

void Unpack ( const uint8_t * pInput, size_t iCount, unsigned uWidth, uint32_t * pValues, const Packing_t & tPacking )
{
	alignas(16) uint32_t dPacked[128];
	alignas(16) uint32_t dValues[128];
	for ( size_t i=0; i<iCount; i+=128 )
	{
		for ( unsigned j=0; j<4*uWidth; ++j )
			dPacked[j] = ReadLE ( pInput+j*4, 4 );

		tPacking.m_fnUnpack ( dPacked, dValues, uWidth );
		std::copy_n ( dValues, std::min ( size_t(128), iCount-i ), pValues+i );
		pInput += 16*uWidth;
	}
}

bool ReadVarints ( Bytes_t tInput, size_t iCount, uint32_t * pOutput )
{
	size_t iPos = 0;
	for ( size_t i=0; i<iCount; ++i )
	{
		uint32_t uValue = 0;
		unsigned j = 0;
		for ( ; j<5; ++j )
		{
			if ( iPos==tInput.m_iSize )
				return false;

			const uint8_t uByte = tInput.m_pData[iPos++];
			if ( j==4 && uByte>15 )
				return false;

			uValue |= uint32_t(uByte & 127)<<(7*j);
			if ( !(uByte & 128) )
			{
				if ( j && !uByte )
					return false; // reject noncanonical overlong encodings

				break;
			}
		}

		if ( j==5 )
			return false;

		if ( pOutput )
			pOutput[i] = uValue;
	}

	return iPos==tInput.m_iSize;
}

struct Choice_t
{
	unsigned m_uWidth;
	unsigned m_uExceptions;
	size_t m_iBytes;
};

Choice_t ChoosePacked ( const uint32_t * pValues, size_t iCount, bool bExceptions )
{
	uint32_t uUnion = 0;
	for ( size_t i=0; i<iCount; ++i )
		uUnion |= pValues[i];

	const unsigned uMaxWidth = Width(uUnion);
	Choice_t tBest { uMaxWidth, 0, PackedBytes ( iCount, uMaxWidth ) };
	if ( !bExceptions )
		return tBest;

	for ( unsigned b=0; b<uMaxWidth; ++b )
	{
		unsigned uExceptions = 0;
		bool bFits = true;
		for ( size_t i=0; i<iCount; ++i )
		{
			const uint32_t uHigh = pValues[i]>>b;
			if ( uHigh && (++uExceptions>7 || uHigh>255) )
			{
				bFits = false;
				break;
			}
		}

		const size_t iBytes = PackedBytes ( iCount, b )+uExceptions*2;
		if ( bFits && iBytes<tBest.m_iBytes )
			tBest = { b, uExceptions, iBytes };
	}

	return tBest;
}

void DecodePayload ( StreamDesc_t tStream, const IntegerDesc_t & tDesc, uint32_t * pOutput, const Packing_t & tPacking )
{
	const auto & tPayload = tDesc.m_tPayload;
	if ( tStream.m_eCodec==Codec_e::CONSTANT )
		std::fill_n ( pOutput, tDesc.m_uCount, ReadLE ( tPayload.m_pData, 4 ) );
	else if ( tStream.m_eCodec==Codec_e::VARINT )
		ReadVarints ( tPayload, tDesc.m_uCount, pOutput );
	else
	{
		Unpack ( tPayload.m_pData, tDesc.m_uCount, tDesc.m_uWidth, pOutput, tPacking );
		const uint8_t * pExceptions = tPayload.m_pData+PackedBytes ( tDesc.m_uCount, tDesc.m_uWidth );
		for ( unsigned i=0; i<tDesc.m_uExceptions; ++i )
			pOutput[pExceptions[2*i]] |= uint32_t(pExceptions[2*i+1])<<tDesc.m_uWidth;
	}
}

}

bool WriteStreamDescriptor ( StreamDesc_t tStream, std::vector<uint8_t> & dOutput, std::string & sError )
{
	sError.clear();
	if ( !IsKnownCodec(tStream.m_eCodec) )
		return Fail ( sError, "Unknown stream codec" );

	std::vector<uint8_t> dEncoded(STREAM_DESCRIPTOR_SIZE);
	WriteLE ( dEncoded.data(), uint16_t(tStream.m_eCodec), STREAM_DESCRIPTOR_SIZE );
	dOutput.swap(dEncoded);
	return true;
}

bool ReadStreamDescriptor ( Bytes_t tInput, StreamDesc_t & tStream, std::string & sError )
{
	sError.clear();
	if ( !tInput.m_pData || tInput.m_iSize!=STREAM_DESCRIPTOR_SIZE )
		return Fail ( sError, "Invalid stream descriptor length" );

	const Codec_e eCodec = Codec_e(ReadLE ( tInput.m_pData, STREAM_DESCRIPTOR_SIZE ));
	if ( !IsKnownCodec(eCodec) )
		return Fail ( sError, "Unknown stream codec" );

	tStream.m_eCodec = eCodec;
	return true;
}

bool EncodeIntegers ( StreamDesc_t tStream, const uint32_t * pValues, size_t iCount, std::vector<uint8_t> & dOutput, std::string & sError, Backend_e eBackend )
{
	sError.clear();
	const auto * pPacking = GetPacking(eBackend);
	if ( !pValues || !iCount || iCount>MAX_VALUES || !pPacking )
		return Fail ( sError, "Invalid integer count, input or backend" );

	const Codec_e eCodec = tStream.m_eCodec;
	if ( !IsKnownCodec(eCodec) )
		return Fail ( sError, "Unknown stream codec" );

	Choice_t tChoice { 0, 0, 0 };
	if ( eCodec==Codec_e::FOR || eCodec==Codec_e::PFOR )
		tChoice = ChoosePacked ( pValues, iCount, eCodec==Codec_e::PFOR );
	else if ( eCodec==Codec_e::VARINT )
	{
		for ( size_t i=0; i<iCount; ++i )
			tChoice.m_iBytes += std::max ( 1u, (Width(pValues[i])+6)/7 );
	} else
	{
		for ( size_t i=1; i<iCount; ++i )
			if ( pValues[i]!=pValues[0] )
				return Fail ( sError, "Nonconstant block in constant stream" );

		tChoice.m_iBytes = 4;
	}

	std::vector<uint8_t> dEncoded ( INTEGER_HEADER_SIZE+tChoice.m_iBytes, 0 );
	dEncoded[0] = uint8_t(tChoice.m_uWidth);
	dEncoded[1] = uint8_t(tChoice.m_uExceptions);
	WriteLE ( dEncoded.data()+2, uint32_t(iCount), 2 );
	WriteLE ( dEncoded.data()+4, uint32_t(tChoice.m_iBytes), 2 );
	uint8_t * pOut = dEncoded.data()+INTEGER_HEADER_SIZE;
	if ( eCodec==Codec_e::CONSTANT )
		WriteLE ( pOut, pValues[0], 4 );
	else if ( eCodec==Codec_e::VARINT )
	{
		for ( size_t i=0; i<iCount; ++i )
		{
			uint32_t uValue = pValues[i];
			while ( uValue>=128 )
			{
				*pOut++ = uint8_t(uValue & 127) | 128;
				uValue >>= 7;
			}

			*pOut++ = uint8_t(uValue);
		}
	} else
	{
		Pack ( pValues, iCount, tChoice.m_uWidth, pOut, *pPacking );
		pOut += PackedBytes ( iCount, tChoice.m_uWidth );
		if ( tChoice.m_uExceptions )
			for ( size_t i=0; i<iCount; ++i )
			{
				const uint32_t uHigh = pValues[i]>>tChoice.m_uWidth;
				if ( uHigh )
				{
					*pOut++ = uint8_t(i);
					*pOut++ = uint8_t(uHigh);
				}
			}
	}

	dOutput.swap(dEncoded);
	return true;
}

bool InspectIntegers ( StreamDesc_t tStream, Bytes_t tInput, IntegerDesc_t & tDesc, std::string & sError )
{
	sError.clear();
	if ( !tInput.m_pData || tInput.m_iSize<INTEGER_HEADER_SIZE )
		return Fail ( sError, "Truncated integer header" );

	const auto * p = tInput.m_pData;
	IntegerDesc_t tParsed { p[0], p[1], uint16_t(ReadLE ( p+2, 2 )), { p+6, ReadLE ( p+4, 2 ) } };
	if ( !IsKnownCodec(tStream.m_eCodec) )
		return Fail ( sError, "Unknown stream codec" );

	if ( !tParsed.m_uCount || tParsed.m_uCount>MAX_VALUES || tParsed.m_tPayload.m_iSize!=tInput.m_iSize-INTEGER_HEADER_SIZE )
		return Fail ( sError, "Invalid integer count or payload length" );

	if ( tStream.m_eCodec==Codec_e::FOR || tStream.m_eCodec==Codec_e::PFOR )
	{
		if ( tParsed.m_uWidth>32 || tParsed.m_uExceptions>7 ||
			(tStream.m_eCodec==Codec_e::FOR && tParsed.m_uExceptions) || (tParsed.m_uWidth==32 && tParsed.m_uExceptions) )
			return Fail ( sError, "Invalid packing width or exception count" );

		const size_t iPacked = PackedBytes ( tParsed.m_uCount, tParsed.m_uWidth );
		if ( iPacked+tParsed.m_uExceptions*2!=tParsed.m_tPayload.m_iSize )
			return Fail ( sError, "Invalid packed payload length" );

		const uint8_t * pExceptions = tParsed.m_tPayload.m_pData+iPacked;
		int iPrevious = -1;
		for ( unsigned i=0; i<tParsed.m_uExceptions; ++i )
		{
			const unsigned uOrdinal = pExceptions[2*i], uHigh = pExceptions[2*i+1];
			if ( int(uOrdinal)<=iPrevious || uOrdinal>=tParsed.m_uCount || !uHigh || uHigh>(UINT32_MAX>>tParsed.m_uWidth) )
				return Fail ( sError, "Invalid exception ordinal or high bits" );

			iPrevious = int(uOrdinal);
		}

		// Unused slots in a short final frame must be zero, including crossing words.
		for ( unsigned i=tParsed.m_uCount%128; i && i<128; ++i )
		{
			const unsigned uWidth = tParsed.m_uWidth;
			const auto * pFrame = tParsed.m_tPayload.m_pData+(tParsed.m_uCount/128)*16*uWidth;
			for ( unsigned b=0; b<uWidth; ++b )
			{
				const unsigned uBit = (i/4)*uWidth+b;
				if ( ReadLE ( pFrame+((uBit/32)*4+i%4)*4, 4 ) & (uint32_t(1)<<(uBit%32)) )
					return Fail ( sError, "Nonzero packed padding" );
			}
		}
	} else
	{
		if ( tParsed.m_uWidth || tParsed.m_uExceptions )
			return Fail ( sError, "Invalid unpacked metadata" );

		if ( tStream.m_eCodec==Codec_e::CONSTANT )
		{
			if ( tParsed.m_tPayload.m_iSize!=4 )
				return Fail ( sError, "Invalid constant length" );
		} else if ( tStream.m_eCodec==Codec_e::VARINT )
		{
			if ( !ReadVarints ( tParsed.m_tPayload, tParsed.m_uCount, nullptr ) )
				return Fail ( sError, "Invalid varint stream" );
		} else
			return Fail ( sError, "Unknown integer codec" );
	}

	tDesc = tParsed;
	return true;
}

bool DecodeIntegers ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend )
{
	IntegerDesc_t tDesc;
	if ( !InspectIntegers ( tStream, tInput, tDesc, sError ) )
		return false;

	const auto * pPacking = GetPacking(eBackend);
	if ( !pOutput || iCapacity<tDesc.m_uCount || !pPacking )
		return Fail ( sError, "Insufficient output capacity or unavailable backend" );

	uint32_t dValues[MAX_VALUES];
	DecodePayload ( tStream, tDesc, dValues, *pPacking );
	std::copy_n ( dValues, tDesc.m_uCount, pOutput );
	return true;
}

bool EncodeRows ( StreamDesc_t tStream, const uint32_t * pRows, size_t iCount, std::vector<uint8_t> & dOutput, std::string & sError, Rows_e eKind, Backend_e eBackend )
{
	sError.clear();
	if ( !pRows || !iCount || iCount>MAX_VALUES || !GetPacking(eBackend) )
		return Fail ( sError, "Invalid row count, input or backend" );

	uint32_t dDeltas[MAX_VALUES] {};
	for ( size_t i=0; i<iCount; ++i )
	{
		if ( pRows[i]==UINT32_MAX || (i && pRows[i]<=pRows[i-1]) )
			return Fail ( sError, "Rows must increase and exclude INVALID_ROWID" );

		if ( i )
			dDeltas[i] = pRows[i]-pRows[i-1];
	}

	const uint64_t uSpan = uint64_t(pRows[iCount-1])-pRows[0]+1;
	std::vector<uint8_t> dPayload;
	if ( !EncodeIntegers ( tStream, dDeltas, iCount, dPayload, sError, eBackend ) )
		return false;

	if ( eKind==Rows_e::AUTO )
	{
		eKind = Rows_e::DELTA;
		if ( uSpan==iCount )
			eKind = Rows_e::RANGE;
		else if ( uSpan<=iCount*32 && (uSpan+7)/8<dPayload.size() )
			eKind = Rows_e::BITMAP;
	}

	if ( eKind==Rows_e::RANGE )
	{
		if ( uSpan!=iCount )
			return Fail ( sError, "Nonconsecutive range" );

		dPayload.clear();
	} else if ( eKind==Rows_e::BITMAP )
	{
		if ( uSpan>MAX_BITMAP_BITS || uSpan>iCount*32 )
			return Fail ( sError, "Bitmap span exceeds block bound" );

		dPayload.assign ( size_t((uSpan+7)/8), 0 );
		for ( size_t i=0; i<iCount; ++i )
		{
			const uint32_t uBit = pRows[i]-pRows[0];
			dPayload[uBit/8] |= uint8_t(1u<<(uBit%8));
		}
	} else if ( eKind!=Rows_e::DELTA )
		return Fail ( sError, "Unknown row representation" );

	std::vector<uint8_t> dEncoded ( ROW_HEADER_SIZE+dPayload.size(), 0 );
	std::memcpy ( dEncoded.data(), "PST1", 4 );
	dEncoded[4] = 1;
	dEncoded[5] = uint8_t(eKind);
	WriteLE ( dEncoded.data()+6, uint32_t(iCount), 2 );
	WriteLE ( dEncoded.data()+8, pRows[0], 4 );
	WriteLE ( dEncoded.data()+12, pRows[iCount-1], 4 );
	WriteLE ( dEncoded.data()+16, uint32_t(dPayload.size()), 4 );
	std::copy ( dPayload.begin(), dPayload.end(), dEncoded.begin()+ROW_HEADER_SIZE );
	dOutput.swap(dEncoded);
	return true;
}

static bool ParseRows ( StreamDesc_t tStream, Bytes_t tInput, RowBlock_t & tBlock, uint32_t * pRows, std::string & sError, const Packing_t & tPacking )
{
	sError.clear();
	if ( !tInput.m_pData || tInput.m_iSize<ROW_HEADER_SIZE || std::memcmp ( tInput.m_pData, "PST1", 4 ) || tInput.m_pData[4]!=1 )
		return Fail ( sError, "Invalid row block magic/version or truncated header" );

	if ( !IsKnownCodec(tStream.m_eCodec) )
		return Fail ( sError, "Unknown stream codec" );

	const auto * p = tInput.m_pData;
	RowBlock_t tParsed { Rows_e(p[5]), uint16_t(ReadLE ( p+6, 2 )), ReadLE ( p+8, 4 ), ReadLE ( p+12, 4 ), { p+20, ReadLE ( p+16, 4 ) } };
	if ( !tParsed.m_uCount || tParsed.m_uCount>MAX_VALUES || tParsed.m_uFirst>tParsed.m_uLast ||
		tParsed.m_uLast==UINT32_MAX || tParsed.m_tPayload.m_iSize!=tInput.m_iSize-20 )
		return Fail ( sError, "Invalid row bounds, count or length" );

	const uint64_t uSpan = uint64_t(tParsed.m_uLast)-tParsed.m_uFirst+1;
	if ( tParsed.m_eKind==Rows_e::DELTA )
	{
		IntegerDesc_t tDesc;
		if ( !InspectIntegers ( tStream, tParsed.m_tPayload, tDesc, sError ) )
			return false;

		if ( tDesc.m_uCount!=tParsed.m_uCount )
			return Fail ( sError, "Row/delta count mismatch" );

		uint32_t dDeltas[MAX_VALUES];
		DecodePayload ( tStream, tDesc, dDeltas, tPacking );
		if ( dDeltas[0] )
			return Fail ( sError, "First delta must be zero" );

		uint64_t uRow = tParsed.m_uFirst;
		for ( unsigned i=0; i<tParsed.m_uCount; ++i )
		{
			if ( i && !dDeltas[i] )
				return Fail ( sError, "Nonincreasing row delta" );

			uRow += dDeltas[i];
			if ( uRow>tParsed.m_uLast )
				return Fail ( sError, "Row delta overflows endpoint" );

			if ( pRows )
				pRows[i] = uint32_t(uRow);
		}

		if ( uRow!=tParsed.m_uLast )
			return Fail ( sError, "Row endpoint mismatch" );
	} else if ( tParsed.m_eKind==Rows_e::RANGE )
	{
		if ( uSpan!=tParsed.m_uCount || tParsed.m_tPayload.m_iSize )
			return Fail ( sError, "Invalid range" );

		if ( pRows )
			for ( unsigned i=0; i<tParsed.m_uCount; ++i )
				pRows[i] = tParsed.m_uFirst+i;
	} else if ( tParsed.m_eKind==Rows_e::BITMAP )
	{
		const auto & tPayload = tParsed.m_tPayload;
		if ( uSpan>MAX_BITMAP_BITS || uSpan>tParsed.m_uCount*32u || tPayload.m_iSize!=(uSpan+7)/8 ||
			!(tPayload.m_pData[0] & 1) || !(tPayload.m_pData[(uSpan-1)/8] & (1u<<((uSpan-1)%8))) )
			return Fail ( sError, "Invalid bitmap span, size or endpoints" );

		unsigned uCount = 0;
		for ( size_t i=0; i<tPayload.m_iSize*8; ++i )
			if ( tPayload.m_pData[i/8] & (1u<<(i%8)) )
			{
				if ( i>=uSpan || uCount==tParsed.m_uCount )
					return Fail ( sError, "Invalid bitmap padding or cardinality" );

				if ( pRows )
					pRows[uCount] = tParsed.m_uFirst+uint32_t(i);
				++uCount;
			}

		if ( uCount!=tParsed.m_uCount )
			return Fail ( sError, "Bitmap cardinality mismatch" );
	} else
		return Fail ( sError, "Unknown row representation" );

	tBlock = tParsed;
	return true;
}

bool InspectRows ( StreamDesc_t tStream, Bytes_t tInput, RowBlock_t & tBlock, std::string & sError )
{
	return ParseRows ( tStream, tInput, tBlock, nullptr, sError, *GetPacking(Backend_e::AUTO) );
}

bool DecodeRows ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend )
{
	const auto * pPacking = GetPacking(eBackend);
	if ( !pOutput || !pPacking )
		return Fail ( sError, "Invalid output or unavailable backend" );

	uint32_t dRows[MAX_VALUES];
	RowBlock_t tBlock;
	if ( !ParseRows ( tStream, tInput, tBlock, dRows, sError, *pPacking ) )
		return false;

	if ( iCapacity<tBlock.m_uCount )
		return Fail ( sError, "Insufficient row output capacity" );

	std::copy_n ( dRows, tBlock.m_uCount, pOutput );
	return true;
}

}
