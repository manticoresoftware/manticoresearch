// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "stream.h"
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
	tDesc.m_bPositions = tDesc.m_bFieldTF = bPositions;
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
		t.m_dFieldTF[f] = bPositions ? t.m_uTF : 0;
		dExpected.push_back(t);
		Check ( pWriter->Add ( t, sError ) );
	}

	Check ( pWriter->Finish(sError) );
	if ( eCodec==Codec_e::PFOR && bPositions && n==1 )
		Check ( pWriter->GetLeaves()[0].m_dSizes[3]==20 );

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
		std::vector<uint8_t> dPayload ( tBlock.m_tPayload.m_pData, tBlock.m_tPayload.m_pData+tBlock.m_tPayload.m_iSize );
		alignas(16) std::array<uint32_t,MAX_VALUES> dLowFields;
		Check ( tReader.ReadLowFields ( dLowFields.data(), sError ) );
		Check ( std::equal ( dPayload.begin(), dPayload.end(), tBlock.m_tPayload.m_pData ) );
		Check ( tReader.ReadRows(sError) && tReader.ReadFields(sError) );
		Check ( tReader.GetDecodeCounts()[2]==0 && tReader.GetDecodeCounts()[3]==0 && tReader.GetDecodeCounts()[4]==0 );
		for ( unsigned i=0; i<tReader.GetLeaf().m_uCount; ++i, ++iRow )
		{
			Check ( dLowFields[i]==dExpected[iRow].m_dFields[0] );
			Check ( tReader.GetRows()[i]==dExpected[iRow].m_uRow );
			Check ( tReader.GetFields()[i]==dExpected[iRow].m_dFields );
		}
	}

	Check ( iRow==n );
	uint64_t uHeaders = TERM_HEADER_SIZE+pWriter->GetLeaves().size()*LEAF_HEADER_SIZE;
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

		// Keep field-TF bytes alive while unrelated columns overwrite buffered input.
		if ( bPositions )
		{
			std::array<uint32_t,MAX_VALUES> dTF;
			Check ( tChecked.ReadFieldTF ( 0, dTF, sError ) );
		}

		Check ( tChecked.Validate(sError) );
		Check ( tReader.ReadRows(sError) && tReader.ReadFields(sError) && tReader.ReadTF(sError) && tReader.ReadHitRefs(sError) );
		if ( bPositions )
			for ( unsigned f=0; f<uFields; ++f )
			{
				std::array<uint32_t,MAX_VALUES> dTrusted, dChecked;
				Check ( tReader.ReadFieldTF ( f, dTrusted, sError ) && tChecked.ReadFieldTF ( f, dChecked, sError ) );
				Check ( dTrusted==dChecked );
			}

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

	std::array<FieldStats_t,MAX_FIELDS> dStats;
	Check ( tChecked.ReadStats ( dStats, sError ) );
	for ( unsigned f=0; f<uFields; ++f )
	{
		uint64_t uDocs=0, uTF=0;
		for ( unsigned i=f; i<n; i+=uFields )
		{
			++uDocs;
			uTF += bPositions ? dExpected[i].m_uTF : 0;
		}

		Check ( dStats[f].m_uDocs==uDocs && dStats[f].m_uTF==uTF );
	}

	Check ( tChecked.Open ( 1, sError ) && tChecked.Next ( dExpected.back().m_uRow, sError ) && tChecked.ReadRows(sError) );
	Check ( tChecked.GetDecodeCounts()[0]==1 && tChecked.GetDecodeCounts()[2]==0 );
	Check ( tChecked.GetRows()[tChecked.GetLeaf().m_uCount-1]==dExpected.back().m_uRow );
	// The checker rejects malformed section metadata before decoding its payload.
	uint64_t uOff = 1+TERM_HEADER_SIZE+LEAF_HEADER_SIZE;
	for ( unsigned c=0; c<COLUMNS; ++c )
	{
		auto nBytes = pWriter->GetLeaves()[0].m_dSizes[c];
		if ( nBytes )
		{
			// Corrupt a section's structural marker, then restore the complete original bytes.
			auto dSaved = tMemory.m_dData;
			if ( c==3 && eCodec==Codec_e::PFOR )
				tMemory.m_dData[size_t(uOff)+8] = 255;
			else
			{
				tMemory.m_dData[size_t(uOff)] = 255;
				if ( c>0 && c<4 )
					tMemory.m_dData[size_t(uOff)+3] |= 128;
				if ( c==4 )
					tMemory.m_dData[size_t(uOff)+1] = 255;
			}

			Check ( tChecked.Open ( 1, sError ) && tChecked.Next ( 0, sError ) );
			Check ( !tChecked.Validate(sError) );
			tMemory.m_dData = dSaved;
			if ( c==3 && eCodec==Codec_e::PFOR )
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


int main()
{
	for ( auto eCodec : {Codec_e::FOR,Codec_e::PFOR,Codec_e::VARINT} )
		for ( unsigned n : {1U,7U,127U,128U,129U,255U,256U,257U,1025U} )
			for ( unsigned f : {1U,33U,256U} )
				for ( bool bPositions : {false,true} )
					for ( bool bMapped : {false,true} )
						Test ( n, f, eCodec, bPositions, bMapped );

	std::cout << g_uChecks << " stream checks passed\n";
}
