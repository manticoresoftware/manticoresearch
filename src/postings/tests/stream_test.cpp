// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "stream.h"
#include "shortlist.h"
#include "hitrefs.h"
#include "streamvalidation.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

using namespace postings;
static unsigned g_uChecks = 0;

static void Check ( bool bCondition )
{
	++g_uChecks;
	if ( !bCondition )
	{
		std::cerr << "failed check " << g_uChecks << '\n';
		std::exit(1);
	}
}


class Memory_c : public Input_i, public Output_i
{
public:
	std::vector<uint8_t>	m_dData { 1 };

	bool		m_bMapped = false;
	uint64_t	m_uReadBytes = 0;

	const uint8_t *	GetMappedData() const final	{ return m_bMapped ? m_dData.data() : nullptr; }
	uint64_t		GetSize() const final		{ return m_dData.size(); }
	uint64_t		GetPos() const final		{ return m_dData.size(); }

	bool			Write ( Bytes_t t, std::string & ) final
	{
		if ( t.m_iSize )
			m_dData.insert ( m_dData.end(), t.m_pData, t.m_pData+t.m_iSize );

		return true;
	}


	bool			Patch ( uint64_t uOffset, Bytes_t t, std::string & ) final
	{
		Check ( uOffset<=m_dData.size() && t.m_iSize<=m_dData.size()-uOffset );
		std::memcpy ( m_dData.data()+uOffset, t.m_pData, t.m_iSize );
		return true;
	}


	bool			Read ( uint64_t uOffset, uint8_t * p, size_t n, std::string & ) final
	{
		Check ( uOffset<=m_dData.size() && n<=m_dData.size()-uOffset );
		m_uReadBytes += n;
		std::memcpy ( p, m_dData.data()+uOffset, n );
		return true;
	}
};


static void Test ( unsigned n, unsigned uFields, Codec_e eCodec, bool bPositions, bool bMapped )
{
	Memory_c tMemory;
	tMemory.m_bMapped = bMapped;
	auto pWriter = std::make_unique<Writer_c>(tMemory);
	TermDesc_t tDesc;
	tDesc.m_uFields = uint16_t(uFields);
	tDesc.m_tRows = {eCodec};
	tDesc.m_tTF = {eCodec};
	tDesc.m_tFields = {eCodec};
	tDesc.m_tHitRefs = { eCodec==Codec_e::PFOR ? Codec_e::BASE_DELTA : eCodec };
	tDesc.m_bPositions = bPositions;
	std::string sError;
	Check ( pWriter->Begin ( tDesc, sError ) );

	std::vector<Posting_t> dExpected;
	for ( unsigned i=0; i<n; ++i )
	{
		Posting_t t;
		t.m_uRow = i*3;
		t.m_uTF = i%29+1;
		t.m_uHitRef = bPositions ? (uint64_t(1)<<34)+i*333 : 0;
		if ( bPositions && uFields>1 && i%17==1 )
			t.m_uHitRef = uint64_t(INT64_MAX)-i;
		unsigned f = i%uFields;
		t.m_dFields[f/32] = 1U<<(f%32);
		dExpected.push_back(t);
		Check ( pWriter->Add ( t, sError ) );
	}

	Check ( pWriter->Finish(sError) );
	if ( eCodec==Codec_e::PFOR && bPositions && n==1 )
		Check ( pWriter->GetLeaves()[0].m_dSizes[3]==5 );

	Reader_c tReader(tMemory);
	Check ( tReader.Open ( 1, sError ) );

	unsigned iRow = 0;
	while ( true )
	{
		Check ( tReader.Next ( 0, sError ) );
		if ( tReader.IsEOF() )
			break;

		RowBlock_t tBlock;
		Check ( tReader.ReadRowBlock ( tBlock, sError ) );

		std::vector<uint8_t> dPayload;
		if ( tBlock.m_tPayload.m_iSize )
			dPayload.assign ( tBlock.m_tPayload.m_pData, tBlock.m_tPayload.m_pData+tBlock.m_tPayload.m_iSize );

		alignas(16) std::array<uint32_t,MAX_VALUES> dLowFields;
		Check ( tReader.ReadLowFields ( dLowFields.data(), sError ) );
		Check ( std::equal ( dPayload.begin(), dPayload.end(), tBlock.m_tPayload.m_pData ) );
		Check ( tReader.ReadRows(sError) && tReader.ReadFields(sError) );
		Check ( tReader.GetDecodeCounts()[2]==0 && tReader.GetDecodeCounts()[3]==0 );
		for ( unsigned i=0; i<tReader.GetLeaf().m_uCount; ++i, ++iRow )
		{
			Check ( dLowFields[i]==dExpected[iRow].m_dFields[0] );
			Check ( tReader.GetRows()[i]==dExpected[iRow].m_uRow );
			Check ( tReader.GetFields()[i]==dExpected[iRow].m_dFields );
		}
	}

	Check ( iRow==n );

	uint64_t uHeaders = n<=SHORT_LIST_MAX ? SHORT_HEADER_SIZE : TERM_HEADER_SIZE+pWriter->GetLeaves().size()*LEAF_HEADER_SIZE;
	Check ( bMapped ? tMemory.m_uReadBytes==uHeaders : tMemory.m_uReadBytes>uHeaders );

	ReaderValidation_c tChecked(tMemory);
	Check ( tReader.Open ( 1, sError ) );
	Check ( tChecked.Open ( 1, sError ) );
	iRow = 0;
	while ( true )
	{
		Check ( tChecked.Next ( 0, sError ) && tReader.Next ( 0, sError ) );
		Check ( tChecked.IsEOF()==tReader.IsEOF() );
		if ( tChecked.IsEOF() )
			break;

		Check ( tChecked.Validate(sError) );
		Check ( tReader.ReadRows(sError) && tReader.ReadFields(sError) && tReader.ReadTF(sError) && tReader.ReadHitRefs(sError) );

		auto dCounts = tChecked.GetDecodeCounts();
		Check ( tChecked.Validate(sError) );
		Check ( dCounts==tChecked.GetDecodeCounts() );

		for ( unsigned i=0; i<tChecked.GetLeaf().m_uCount; ++i, ++iRow )
		{
			Check ( tChecked.GetRows()[i]==tReader.GetRows()[i] );
			Check ( tChecked.GetFields()[i]==tReader.GetFields()[i] );
			Check ( tChecked.GetTF()[i]==tReader.GetTF()[i] );
			Check ( tChecked.GetHitRefs()[i]==tReader.GetHitRefs()[i] );
			Check ( tChecked.GetTF()[i]==dExpected[iRow].m_uTF );
			Check ( tChecked.GetHitRefs()[i]==dExpected[iRow].m_uHitRef );
		}
	}

	Check ( tChecked.Open ( 1, sError ) && tChecked.Next ( dExpected.back().m_uRow, sError ) && tChecked.ReadRows(sError) );
	Check ( tChecked.GetDecodeCounts()[0]==1 && tChecked.GetDecodeCounts()[2]==0 );
	Check ( tChecked.GetRows()[tChecked.GetLeaf().m_uCount-1]==dExpected.back().m_uRow );
	Check ( tReader.Open ( 1, sError ) && tReader.Next ( dExpected.back().m_uRow+1, sError ) && tReader.IsEOF() );
	if ( n<=SHORT_LIST_MAX )
	{
		// Impossible count, flags, field count, field ID, max TF and section extent.
		for ( auto tBad : { std::pair<unsigned,uint8_t>{6,0}, {6,17}, {7,128}, {9,2}, {11,2}, {12,0}, {17,255} } )
		{
			auto dSaved = tMemory.m_dData;
			tMemory.m_dData[1+tBad.first] = tBad.second;
			Check ( !tChecked.Open ( 1, sError ) );
			tMemory.m_dData = dSaved;
		}
	}

	// The checker rejects malformed section metadata before decoding its payload.
	uint64_t uOff = pWriter->GetLeaves()[0].m_uOffset+pWriter->GetLeaves()[0].m_uHeaderSize;
	for ( unsigned c=0; c<COLUMNS; ++c )
	{
		auto nBytes = pWriter->GetLeaves()[0].m_dSizes[c];
		if ( nBytes )
		{
			// Corrupt a section's structural marker, then restore the complete original bytes.
			auto dSaved = tMemory.m_dData;
			if ( n<=SHORT_LIST_MAX )
				std::fill_n ( tMemory.m_dData.data()+uOff, nBytes, 128 );
			else if ( c==3 && eCodec==Codec_e::PFOR )
				tMemory.m_dData[size_t(uOff)+8] = 255;
			else
			{
				tMemory.m_dData[size_t(uOff)] = 255;
				if ( c>0 && c<4 )
					tMemory.m_dData[size_t(uOff)+3] |= 128;
			}

			Check ( !(tChecked.Open ( 1, sError ) && tChecked.Next ( 0, sError ) && tChecked.Validate(sError)) );
			tMemory.m_dData = dSaved;
			if ( n>SHORT_LIST_MAX && c==3 && eCodec==Codec_e::PFOR )
				for ( unsigned uCase=0; uCase<(n>1 ? 3U : 2U); ++uCase )
				{
					// Zero/oversized base, then valid base with overflowing relative offsets.
					std::fill_n ( tMemory.m_dData.data()+uOff, 8, uCase ? 255 : 0 );
					if ( uCase==2 )
						tMemory.m_dData[size_t(uOff)+7] = 127;

					Check ( tChecked.Open ( 1, sError ) && tChecked.Next ( 0, sError ) );
					Check ( !tChecked.ReadHitRefs(sError) );
					tMemory.m_dData = dSaved;
				}
		}

		uOff += nBytes;
	}

	// The enclosing extent must be validated before any payload read.
	tMemory.m_dData.pop_back();
	Check ( !tChecked.Open ( 1, sError ) );
}


static void TestAdjacentTerms()
{
	Memory_c tMemory;
	auto pWriter = std::make_unique<Writer_c>(tMemory);
	TermDesc_t tDesc;
	tDesc.m_uFields = 256;
	std::string sError;
	std::vector<uint64_t> dOffsets;
	for ( unsigned n : { 1U, 2U, 16U, 17U, 257U, 1U } )
	{
		dOffsets.push_back(tMemory.GetPos());
		Check ( pWriter->Begin ( tDesc, sError ) );
		for ( unsigned i=0; i<n; ++i )
		{
			Posting_t t;
			t.m_uRow = UINT32_MAX-n+i-1;
			t.m_uTF = UINT32_MAX;
			t.m_uHitRef = uint64_t(INT64_MAX)-i;
			t.m_dFields[7] = 1U<<31;
			Check ( pWriter->Add ( t, sError ) );
		}

		Check ( pWriter->Finish(sError) );
	}

	dOffsets.push_back(tMemory.GetPos());

	Reader_c tReader(tMemory);
	ReaderValidation_c tChecked(tMemory);
	for ( size_t iTerm=dOffsets.size()-1; iTerm>0; --iTerm )
	{
		Check ( tReader.Open ( dOffsets[iTerm-1], sError ) && tChecked.Open ( dOffsets[iTerm-1], sError ) );
		Check ( tReader.GetEnd()==dOffsets[iTerm] && tChecked.GetEnd()==dOffsets[iTerm] );
		while ( true )
		{
			Check ( tReader.Next ( 0, sError ) && tChecked.Next ( 0, sError ) );
			Check ( tReader.IsEOF()==tChecked.IsEOF() );
			if ( tReader.IsEOF() )
				break;

			Check ( tChecked.Validate(sError) );
			Check ( tReader.ReadRows(sError) && tReader.ReadTF(sError) && tReader.ReadHitRefs(sError) );

			uint32_t dFields[MAX_VALUES];
			Check ( tReader.ReadLowFields ( dFields, sError ) );
			for ( unsigned i=0; i<tReader.GetLeaf().m_uCount; ++i )
			{
				Check ( !dFields[i] && tChecked.GetFields()[i][7]==(1U<<31) );
				Check ( tReader.GetRows()[i]==tChecked.GetRows()[i] );
				Check ( tReader.GetTF()[i]==UINT32_MAX );
				Check ( tReader.GetHitRefs()[i]==tChecked.GetHitRefs()[i] );
			}
		}
	}
}


static void CheckInlineCorruption ( Bytes_t tData, unsigned uCount, bool bShort )
{
	auto fnFrame = [] ( StreamDesc_t tStream, Bytes_t & tFrame, uint32_t * pValues, size_t iCount, std::string & sError )
	{
		if ( tFrame.m_iSize<4 )
			return false;

		uint32_t uBytes = 0;
		for ( unsigned i=0; i<4; ++i )
			uBytes |= uint32_t(tFrame.m_pData[i])<<(i*8);

		if ( uBytes>tFrame.m_iSize-4 )
			return false;

		IntegerDesc_t tDesc;
		Bytes_t tInts { tFrame.m_pData+4, uBytes };
		if ( !InspectIntegers ( tStream, tInts, tDesc, sError ) || tDesc.m_uCount!=iCount || !DecodeIntegersInto ( tStream, tInts, pValues, iCount, sError ) )
			return false;

		tFrame.m_pData += 4+uBytes;
		tFrame.m_iSize -= 4+uBytes;
		return true;
	};

	uint64_t dRefs[MAX_VALUES];
	std::string sError;
	for ( size_t i=0; i<tData.m_iSize; ++i )
		Check ( !ReadInlineHitRefs<true> ( {tData.m_pData,i}, uCount, bShort, dRefs, fnFrame, sError ) );

	std::vector<uint8_t> dExtra ( tData.m_pData, tData.m_pData+tData.m_iSize );
	dExtra.push_back(0);
	Check ( !ReadInlineHitRefs<true> ( {dExtra.data(),dExtra.size()}, uCount, bShort, dRefs, fnFrame, sError ) );
}


static void TestInlineBitmap()
{
	std::array<uint8_t,40> dBits;
	for ( unsigned uOffset=0; uOffset<8; ++uOffset )
		for ( unsigned uBytes=0; uBytes<=32; ++uBytes )
		{
			unsigned uExpected = 0;
			for ( unsigned i=0; i<uBytes; ++i )
			{
				dBits[uOffset+i] = uint8_t(i*73+uOffset*31);
				for ( unsigned j=0; j<8; ++j )
					uExpected += (dBits[uOffset+i]>>j)&1;
			}

			Check ( bitmap::CountBytes ( dBits.data()+uOffset, uBytes )==uExpected );
		}
}


static void TestInlineValues()
{
	auto fnNoFrame = [] ( StreamDesc_t, Bytes_t &, uint32_t *, size_t, std::string & ) { return false; };
	std::string sError;
	for ( uint32_t uHit : {1u,127u,128u,16383u,16384u,2097151u,2097152u,268435455u,268435456u,UINT32_MAX} )
	{
		std::vector<uint8_t> dData {uint8_t(HitRefMode_e::ALL_INLINE)};
		uint32_t uValue = uHit;
		do
		{
			uint8_t uByte = uint8_t(uValue&127);
			uValue >>= 7;
			dData.push_back ( uByte | (uValue ? 128 : 0) );
		} while ( uValue );

		uint64_t uChecked, uTrusted;
		Check ( ReadInlineHitRefs<true> ( {dData.data(),dData.size()}, 1, true, &uChecked, fnNoFrame, sError ) );
		Check ( ReadInlineHitRefs<false> ( {dData.data(),dData.size()}, 1, true, &uTrusted, fnNoFrame, sError ) );
		Check ( uChecked==(INLINE_HIT | uHit) && uTrusted==uChecked );
		CheckInlineCorruption ( {dData.data(),dData.size()}, 1, true );
	}

	const auto uInlineMode = uint8_t(HitRefMode_e::ALL_INLINE);
	for ( const std::vector<uint8_t> & dData : std::vector<std::vector<uint8_t>> { {uInlineMode,0}, {uInlineMode,128,0}, {uInlineMode,129,0}, {uInlineMode,128,128,128,128,16}, {uInlineMode,128,128,128,128,128,1}, {uInlineMode,255,255,255,255,255} } )
	{
		uint64_t uHit;
		Check ( !ReadInlineHitRefs<true> ( {dData.data(),dData.size()}, 1, true, &uHit, fnNoFrame, sError ) );
	}
}


static void TestInline ( unsigned int n, unsigned int k, bool bMapped, bool bWide, bool bHigh, bool bScatter=false )
{
	Memory_c tMemory, tHits;
	tMemory.m_bMapped = bMapped;
	Writer_c tWriter ( tMemory, &tHits );
	TermDesc_t tDesc;
	tDesc.m_uFields = bWide ? 256 : 1;
	tDesc.m_tHitRefs = {Codec_e::INLINE_HITS};
	std::string sError;
	Check ( tWriter.Begin ( tDesc, sError ) );

	std::vector<Posting_t> dPostings;
	for ( unsigned i=0; i<n; ++i )
	{
		Posting_t t;
		t.m_uRow = i*3;
		t.m_uTF = (bScatter ? i*97%256 : i%256)<k ? 1 : 2;
		unsigned f = bWide ? i%256 : 0;
		t.m_dFields[f/32] = 1U<<(f%32);
		t.m_uHitRef = t.m_uTF==1 ? INLINE_HIT | (uint64_t(f)<<24) | ((i&1) ? (1U<<23) : 0) | (i%449+1) : (bHigh ? uint64_t(1)<<35 : 4096)+i*20;
		if ( bScatter && !bWide && t.m_uTF==1 )
			t.m_uHitRef = INLINE_HIT | (1+i%127);

		dPostings.push_back(t);
		Check ( tWriter.Add ( t, sError ) );
	}

	Check ( tWriter.Finish(sError) );

	Reader_c tReader(tMemory);
	ReaderValidation_c tChecked(tMemory);
	Check ( tReader.Open ( 1, sError ) && tChecked.Open ( 1, sError ) );
	HitRefView_c tView;
	unsigned uRead = 0;
	while ( true )
	{
		Check ( tReader.Next ( 0, sError ) && tChecked.Next ( 0, sError ) );
		if ( tReader.IsEOF() )
			break;

		Check ( tReader.ReadHitRefView ( tView, sError ) );
		Check ( tReader.ReadFields(sError) );
		Check ( tReader.ReadHitRefs(sError) && !tReader.GetDecodeCounts()[2] );
		Check ( tChecked.Validate(sError) );
		const auto & tLeaf = tReader.GetLeaf();
		for ( unsigned i=0; i<tLeaf.m_uCount; ++i, ++uRead )
		{
			uint64_t uRef = tReader.GetHitRefs()[i];
			Check ( tView.Get(i)==uRef );
			Check ( uRef==tChecked.GetHitRefs()[i] );
			const auto & t = dPostings[uRead];
			if ( t.m_uTF!=1 || (uRef & INLINE_HIT) )
				Check ( uRef==t.m_uHitRef );
			else
			{
				Check ( uRef<tHits.m_dData.size() );
				uint32_t uHit = 0;
				uint8_t uByte;
				do
				{
					uByte = tHits.m_dData[uRef++];
					uHit = (uHit<<7) | (uByte&127);
				} while ( uByte&128 );

				Check ( uHit==uint32_t(t.m_uHitRef) && tHits.m_dData[uRef]==0 );
			}
		}

		if ( uRead==n )
		{
			Check ( tReader.ReadHitRefView ( tView, sError ) );
			for ( unsigned int i=0; i<tLeaf.m_uCount; ++i )
				Check ( tView.Get(i)==tReader.GetHitRefs()[i] );
		}

		// Verify the selected modes and reject an unknown mode.
		unsigned uOffset = unsigned(tLeaf.m_uOffset+tLeaf.m_uHeaderSize);
		for ( unsigned c=0; c<3; ++c )
			uOffset += tLeaf.m_dSizes[c];

		if ( !bWide && !bHigh && !bMapped )
			CheckInlineCorruption ( {tMemory.m_dData.data()+uOffset,tLeaf.m_dSizes[3]}, tLeaf.m_uCount, tLeaf.m_bShort );

		if ( n==256 && !bWide && !bHigh && !bMapped )
		{
			auto eMode = HitRefMode_e(tMemory.m_dData[uOffset]);
			if ( !k || k==1 || k==2 )
				Check ( eMode==HitRefMode_e::ALL_EXTERNAL );

			if ( k==256 )
				Check ( eMode==HitRefMode_e::ALL_INLINE );

			if ( k==192 )
				Check ( eMode==HitRefMode_e::MIXED );

			tMemory.m_dData[uOffset] = 3;
			ReaderValidation_c tBad(tMemory);
			Check ( tBad.Open ( 1, sError ) && tBad.Next ( 0, sError ) && !tBad.ReadHitRefs(sError) );
			tMemory.m_dData[uOffset] = uint8_t(eMode);
		}
	}

	Check ( uRead==n );
}


static void TestDictionarySingleton()
{
	Memory_c tDocs, tHits;
	Writer_c tWriter ( tDocs, &tHits );
	TermDesc_t tDesc;
	tDesc.m_uFields = MAX_FIELDS;
	tDesc.m_tHitRefs = { Codec_e::INLINE_HITS };
	std::string sError;
	Check ( tWriter.Begin ( tDesc, sError ) );

	Posting_t t;
	t.m_uRow = INT32_MAX-1;
	t.m_uTF = 1;
	t.m_uHitRef = INLINE_HIT | 0xff800023U;
	t.m_dFields.back() = 1U<<31;
	Check ( tWriter.Add ( t, sError ) );

	int64_t iLocator = 0;
	const uint8_t * pRecord = nullptr;
	Check ( tWriter.FinishDictionarySingleton ( iLocator, pRecord ) && !pRecord && iLocator<0 );
	Check ( tDocs.GetSize()==1 && tHits.GetSize()==1 && tWriter.GetLeaves().empty() );

	Reader_c tReader(tDocs);
	Check ( tReader.Open ( iLocator, sError ) && tReader.Next ( t.m_uRow, sError ) && !tReader.IsEOF() );
	RowBlock_t tBlock;
	Check ( tReader.ReadRowBlock ( tBlock, sError ) && tBlock.m_uFirst==t.m_uRow && tBlock.m_uLast==t.m_uRow );
	Check ( tReader.ReadTF(sError) && tReader.GetTF()[0]==1 );
	Check ( tReader.ReadFields(sError) && tReader.GetFields()[0]==t.m_dFields );
	Check ( tReader.ReadHitRefs(sError) && tReader.GetHitRefs()[0]==t.m_uHitRef );
	Check ( tReader.Next ( 0, sError ) && tReader.IsEOF() );
	Check ( tReader.Open ( iLocator, sError ) && tReader.Next ( t.m_uRow+1, sError ) && tReader.IsEOF() );
	for ( bool bWide : { false, true } )
	{
		t.m_uTF = 500;
		t.m_uHitRef = (uint64_t(1)<<40)+123;
		if ( bWide )
			t.m_dFields[0] = 3;

		Check ( tWriter.Begin ( tDesc, sError ) && tWriter.Add ( t, sError ) );
		Check ( tWriter.FinishDictionarySingleton ( iLocator, pRecord ) && pRecord && iLocator==MULTI_HIT_MARKER );
		Check ( tDocs.GetSize()==1 && tWriter.GetLeaves().empty() );
		Check ( tReader.Open ( iLocator, sError, pRecord ) && tReader.Next ( t.m_uRow, sError ) && !tReader.IsEOF() );
		Check ( tReader.GetTF()[0]==t.m_uTF && tReader.GetFields()[0]==t.m_dFields && tReader.GetHitRefs()[0]==t.m_uHitRef );
		Check ( tReader.Next ( 0, sError ) && tReader.IsEOF() );

		Posting_t tChecked;
		Check ( CheckDictionarySingleton ( pRecord, tChecked ) && tChecked.m_uRow==t.m_uRow && tChecked.m_dFields==t.m_dFields );
		SingletonBytes_t dTruncated {};
		std::copy_n ( pRecord, 1+*pRecord, dTruncated.data() );
		--dTruncated[0];
		Check ( !CheckDictionarySingleton ( dTruncated.data(), tChecked ) );
	}

	Posting_t tChecked;
	SingletonReadBuffer_t dInvalid {};
	Check ( !CheckDictionarySingleton ( dInvalid.data(), tChecked ) );
	dInvalid[0] = UINT8_MAX;
	Check ( !CheckDictionarySingleton ( dInvalid.data(), tChecked ) );

	// TF, row and field followed by an invalid .spp offset.
	SingletonBytes_t dRecord { 5, 2, 0, 0, 0x80, 1 };
	Check ( !CheckDictionarySingleton ( dRecord.data(), tChecked ) );
	dRecord[0] = uint8_t ( 3+ZipToPtrBE ( dRecord.data()+4, uint64_t(INT64_MAX) ) );
	Check ( CheckDictionarySingleton ( dRecord.data(), tChecked ) && tChecked.m_uHitRef==INT64_MAX );
	Check ( !CheckDictionarySingleton ( dRecord.data(), tChecked, false ) );
	dRecord[0] = uint8_t ( 3+ZipToPtrBE ( dRecord.data()+4, uint64_t(INT64_MAX)+1 ) );
	Check ( !CheckDictionarySingleton ( dRecord.data(), tChecked ) );

	Check ( !tDocs.m_uReadBytes );
}


int main()
{
	TestDictionarySingleton();
	TestInlineBitmap();
	TestInlineValues();
	TestAdjacentTerms();
	for ( unsigned n : {1u,2u,16u,17u,127u,128u,129u,255u,256u,257u,513u} )
		for ( unsigned k : {0u,1u,2u,8u,16u,32u,64u,127u,128u,129u,192u,255u,256u} )
			for ( bool bMapped : {false,true} )
				for ( bool bWide : {false,true} )
					for ( bool bHigh : {false,true} )
						TestInline ( n, k, bMapped, bWide, bHigh );

	for ( bool bMapped : {false,true} )
		for ( bool bWide : {false,true} )
			TestInline ( 513, 164, bMapped, bWide, true, true );

	for ( auto eCodec : {Codec_e::FOR,Codec_e::PFOR,Codec_e::VARINT} )
		for ( unsigned n : {1U,2U,3U,4U,7U,8U,15U,16U,17U,127U,128U,129U,255U,256U,257U,1025U} )
			for ( unsigned f : {1U,33U,256U} )
				for ( bool bPositions : {false,true} )
					for ( bool bMapped : {false,true} )
						Test ( n, f, eCodec, bPositions, bMapped );

	std::cout << g_uChecks << " stream checks passed\n";
}
