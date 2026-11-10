// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "stream.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>

namespace postings
{
namespace
{

uint64_t Get ( const uint8_t * p, unsigned n )
{
	uint64_t u = 0;
	for ( unsigned i=0; i<n; ++i )
		u |= uint64_t(p[i])<<(8*i);

	return u;
}


void Put ( uint8_t * p, uint64_t u, unsigned n )
{
	for ( unsigned i=0; i<n; ++i )
		p[i] = uint8_t(u>>(8*i));
}


void Append ( std::vector<uint8_t> & d, uint64_t u, unsigned n )
{
	size_t i = d.size();
	d.resize ( i+n );
	Put ( d.data()+i, u, n );
}


Bytes_t Bytes ( const std::vector<uint8_t> & d ) { return { d.data(), d.size() }; }


bool Error ( std::string & sError, const char * sMessage )
{
	sError = sMessage;
	return false;
}


bool Header ( const TermDesc_t & tDesc, uint64_t uEnd, uint64_t uStats, std::array<uint8_t,TERM_HEADER_SIZE> & dHeader, std::string & sError )
{
	assert ( tDesc.m_uFields && tDesc.m_uFields<=MAX_FIELDS && (!tDesc.m_bFieldTF || tDesc.m_bPositions) );

	dHeader.fill(0);
	std::memcpy ( dHeader.data(), "PST2", 4 );
	Put ( dHeader.data()+4, COMPONENT_VERSION, 2 );
	Put ( dHeader.data()+6, unsigned(tDesc.m_bPositions) | (unsigned(tDesc.m_bFieldTF)<<1), 2 );
	Put ( dHeader.data()+8, tDesc.m_uFields, 2 );
	StreamDesc_t dStreams[] = { tDesc.m_tRows, tDesc.m_tFields, tDesc.m_tTF, tDesc.m_tHitRefs };
	for ( unsigned i=0; i<4; ++i )
	{
		std::vector<uint8_t> d;
		if ( !WriteStreamDescriptor ( dStreams[i], d, sError ) )
			return false;

		std::copy ( d.begin(), d.end(), dHeader.begin()+12+i*2 );
	}

	Put ( dHeader.data()+24, uEnd, 8 );
	Put ( dHeader.data()+32, uStats, 8 );
	return true;
}


bool AddFrame ( StreamDesc_t tStream, const uint32_t * pValues, size_t iCount, std::vector<uint8_t> & dOutput, std::string & sError )
{
	std::vector<uint8_t> dFrame;
	if ( !EncodeIntegers ( tStream, pValues, iCount, dFrame, sError ) )
		return false;

	Append ( dOutput, dFrame.size(), 4 );
	dOutput.insert ( dOutput.end(), dFrame.begin(), dFrame.end() );
	return true;
}


bool Frame ( StreamDesc_t tStream, Bytes_t & tData, uint32_t * pValues, size_t iCount, std::string & sError )
{
	assert ( tData.m_iSize>=4 );
	size_t iSize = size_t(Get(tData.m_pData,4));
	assert ( iSize<=tData.m_iSize-4 );
	Bytes_t tFrame { tData.m_pData+4, iSize };
	assert ( tFrame.m_iSize>=INTEGER_HEADER_SIZE && Get(tFrame.m_pData+2,2)==iCount );
	DecodeIntegersInto ( tStream, tFrame, pValues, iCount, sError );
	tData.m_pData += 4+iSize;
	tData.m_iSize -= 4+iSize;
	return true;
}

}


Writer_c::Writer_c ( Output_i & tOutput ) : m_tOutput(tOutput) {}


bool Writer_c::Begin ( const TermDesc_t & tDesc, std::string & sError )
{
	assert ( !m_bOpen );

	std::array<uint8_t,TERM_HEADER_SIZE> dHeader;
	if ( !Header ( tDesc, 0, 0, dHeader, sError ) )
		return false;

	m_tDesc = tDesc;
	m_uStart = m_tOutput.GetPos();
	m_iCount = 0;
	m_bHaveRows = false;
	m_dStats.fill({});
	m_dLeaves.clear();
	m_bOpen = m_tOutput.Write ( {dHeader.data(),dHeader.size()}, sError );
	return m_bOpen;
}


bool Writer_c::Add ( const Posting_t & tPosting, std::string & sError )
{
	assert ( m_bOpen && tPosting.m_uRow!=UINT32_MAX && (!m_bHaveRows || tPosting.m_uRow>m_uLast) && tPosting.m_uTF );
	assert ( !m_tDesc.m_bPositions || (tPosting.m_uHitRef && tPosting.m_uHitRef<=INT64_MAX) );
#ifndef NDEBUG
	uint64_t uTF = 0;
	bool bFields = false;
	for ( unsigned i=0; i<MAX_FIELDS; ++i )
	{
		bool bSet = (tPosting.m_dFields[i/32] & (1U<<(i%32)))!=0;
		assert ( i<m_tDesc.m_uFields || (!bSet && !tPosting.m_dFieldTF[i]) );
		assert ( !m_tDesc.m_bFieldTF || bSet==(tPosting.m_dFieldTF[i]!=0) );
		bFields |= bSet;
		uTF += tPosting.m_dFieldTF[i];
	}
	assert ( bFields && (!m_tDesc.m_bFieldTF || uTF==tPosting.m_uTF) );
#endif

	for ( unsigned i=0; i<m_tDesc.m_uFields; ++i )
	{
		m_dStats[i].m_uDocs += (tPosting.m_dFields[i/32]>>(i%32))&1;
		m_dStats[i].m_uTF += m_tDesc.m_bFieldTF ? tPosting.m_dFieldTF[i] : 0;
	}

	m_dPostings[m_iCount++] = tPosting;
	m_uLast = tPosting.m_uRow;
	m_bHaveRows = true;
	if ( m_iCount<MAX_VALUES || Flush(sError) )
		return true;

	m_bOpen = false;
	return false;
}


bool Writer_c::EncodeHitRefs ( std::vector<uint8_t> & dOutput, std::string & sError ) const
{
	if ( !m_tDesc.m_bPositions )
		return true;

	StreamDesc_t tStream = m_tDesc.m_tHitRefs;
	uint64_t uBase = 0;
	unsigned uParts = 2;
	if ( tStream.m_eCodec==Codec_e::BASE_DELTA )
	{
		uBase = m_dPostings[0].m_uHitRef;
		uint64_t uMax = uBase;
		for ( size_t i=1; i<m_iCount; ++i )
		{
			uBase = std::min ( uBase, m_dPostings[i].m_uHitRef );
			uMax = std::max ( uMax, m_dPostings[i].m_uHitRef );
		}

		uParts = uMax-uBase>UINT32_MAX ? 2 : 1;
		Append ( dOutput, uBase, 8 );
		Append ( dOutput, uParts-1, 1 );
		tStream = {Codec_e::PFOR};
	}

	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( unsigned part=0; part<uParts; ++part )
	{
		for ( size_t i=0; i<m_iCount; ++i )
			dValues[i] = uint32_t((m_dPostings[i].m_uHitRef-uBase)>>(32*part));

		if ( !AddFrame ( tStream, dValues.data(), m_iCount, dOutput, sError ) )
			return false;
	}

	return true;
}


bool Writer_c::Flush ( std::string & sError )
{
	if ( !m_iCount )
		return true;

	std::array<std::vector<uint8_t>,COLUMNS> dColumns;
	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( size_t i=0; i<m_iCount; ++i )
		dValues[i] = m_dPostings[i].m_uRow;

	if ( !EncodeRows ( m_tDesc.m_tRows, dValues.data(), m_iCount, dColumns[0], sError ) )
		return false;

	unsigned uWords = (m_tDesc.m_uFields+31)/32;
	for ( unsigned f=0; f<uWords; ++f )
	{
		for ( size_t i=0; i<m_iCount; ++i )
			dValues[i] = m_dPostings[i].m_dFields[f];

		if ( !AddFrame ( m_tDesc.m_tFields, dValues.data(), m_iCount, dColumns[1], sError ) )
			return false;
	}

	uint32_t uMaxTF = 0;
	for ( size_t i=0; i<m_iCount; ++i )
	{
		dValues[i] = m_dPostings[i].m_uTF;
		uMaxTF = std::max ( uMaxTF, dValues[i] );
	}

	if ( !AddFrame ( m_tDesc.m_tTF, dValues.data(), m_iCount, dColumns[2], sError ) )
		return false;

	if ( !EncodeHitRefs ( dColumns[3], sError ) )
		return false;

	if ( m_tDesc.m_bFieldTF )
		for ( unsigned f=0; f<m_tDesc.m_uFields; ++f )
		{
			bool bPresent = false;
			for ( size_t i=0; i<m_iCount; ++i )
			{
				dValues[i] = m_dPostings[i].m_dFieldTF[f];
				bPresent |= dValues[i]!=0;
			}

			if ( !bPresent )
				continue;

			Append ( dColumns[4], f, 2 );
			if ( !AddFrame ( m_tDesc.m_tTF, dValues.data(), m_iCount, dColumns[4], sError ) )
				return false;
		}

	Leaf_t tLeaf;
	tLeaf.m_uOffset = m_tOutput.GetPos();
	tLeaf.m_uCount = uint16_t(m_iCount);
	tLeaf.m_uFirst = m_dPostings[0].m_uRow;
	tLeaf.m_uLast = m_dPostings[m_iCount-1].m_uRow;
	tLeaf.m_uMaxTF = uMaxTF;
	tLeaf.m_uFieldWords = uint16_t(uWords);
	tLeaf.m_uBytes = LEAF_HEADER_SIZE;
	std::array<uint8_t,LEAF_HEADER_SIZE> dHeader {};
	std::memcpy ( dHeader.data(), "BLK1", 4 );
	Put ( dHeader.data()+8, tLeaf.m_uCount, 2 );
	Put ( dHeader.data()+10, uWords, 2 );
	Put ( dHeader.data()+12, tLeaf.m_uFirst, 4 );
	Put ( dHeader.data()+16, tLeaf.m_uLast, 4 );
	Put ( dHeader.data()+20, uMaxTF, 4 );
	for ( size_t i=0; i<COLUMNS; ++i )
	{
		tLeaf.m_dSizes[i] = uint32_t(dColumns[i].size());
		tLeaf.m_uBytes += tLeaf.m_dSizes[i];
		Put ( dHeader.data()+24+4*i, tLeaf.m_dSizes[i], 4 );
	}

	Put ( dHeader.data()+4, tLeaf.m_uBytes, 4 );
	if ( !m_tOutput.Write ( {dHeader.data(),dHeader.size()}, sError ) )
		return false;

	for ( const auto & dColumn : dColumns )
		if ( !m_tOutput.Write ( Bytes(dColumn), sError ) )
			return false;

	m_dLeaves.push_back(tLeaf);
	m_iCount = 0;
	return true;
}


bool Writer_c::Finish ( std::string & sError )
{
	assert ( m_bOpen && m_bHaveRows );

	if ( !Flush(sError) )
		return false;

	uint64_t uStats = m_tOutput.GetPos();
	std::vector<uint8_t> dStats;
	for ( unsigned i=0; i<m_tDesc.m_uFields; ++i )
	{
		Append ( dStats, m_dStats[i].m_uDocs, 8 );
		Append ( dStats, m_dStats[i].m_uTF, 8 );
	}

	if ( !m_tOutput.Write ( Bytes(dStats), sError ) )
		return false;

	std::array<uint8_t,TERM_HEADER_SIZE> dHeader;
	if ( !Header ( m_tDesc, m_tOutput.GetPos(), uStats, dHeader, sError ) || !m_tOutput.Patch ( m_uStart, {dHeader.data(),dHeader.size()}, sError ) )
		return false;

	m_bOpen = false;
	return true;
}


Reader_c::Reader_c ( Input_i & tInput ) : m_tInput(tInput) {}


bool Reader_c::Fail ( std::string & sError, const char * sMessage )
{
	m_bError = true;
	return Error ( sError, sMessage );
}


bool Reader_c::Open ( uint64_t uOffset, std::string & sError )
{
	m_bEOF = true;
	m_bError = false;
	m_bHaveLeaf = false;
	m_dLoaded.fill(false);
	m_tRowData = {};
	m_pFieldTF.reset();
	m_dDecodeCounts.fill(0);
	std::array<uint8_t,TERM_HEADER_SIZE> d;
	assert ( uOffset<=m_tInput.GetSize() && d.size()<=m_tInput.GetSize()-uOffset );
	if ( !m_tInput.Read ( uOffset, d.data(), d.size(), sError ) )
		return Fail ( sError, "truncated posting term header" );

	if ( std::memcmp ( d.data(), "PST2", 4 ) || Get(d.data()+4,2)!=COMPONENT_VERSION )
		return Fail ( sError, "unsupported posting component/version" );

	unsigned uFlags = unsigned(Get(d.data()+6,2));
	if ( (uFlags&~3U) || Get(d.data()+10,2) || Get(d.data()+20,4) || ((uFlags&2) && !(uFlags&1)) )
		return Fail ( sError, "unsupported posting capabilities" );

	m_tDesc.m_bPositions = (uFlags&1)!=0;
	m_tDesc.m_bFieldTF = (uFlags&2)!=0;
	m_tDesc.m_uFields = uint16_t(Get(d.data()+8,2));
	StreamDesc_t * dStreams[] = { &m_tDesc.m_tRows, &m_tDesc.m_tFields, &m_tDesc.m_tTF, &m_tDesc.m_tHitRefs };
	for ( unsigned i=0; i<4; ++i )
		if ( !ReadStreamDescriptor ( {d.data()+12+2*i,2}, *dStreams[i], sError ) )
		{
			m_bError = true;
			return false;
		}

	assert ( m_tDesc.m_tRows.m_eCodec!=Codec_e::BASE_DELTA && m_tDesc.m_tFields.m_eCodec!=Codec_e::BASE_DELTA && m_tDesc.m_tTF.m_eCodec!=Codec_e::BASE_DELTA );

	m_uEnd = Get(d.data()+24,8);
	m_uStats = Get(d.data()+32,8);
	m_uNext = uOffset+TERM_HEADER_SIZE;
	assert ( !(!m_tDesc.m_uFields || m_tDesc.m_uFields>MAX_FIELDS || m_uStats<=m_uNext || m_uStats>m_uEnd || m_uEnd>m_tInput.GetSize() || m_uEnd-m_uStats!=16*uint64_t(m_tDesc.m_uFields)) );

	m_bEOF = false;
	return true;
}


bool Reader_c::Next ( uint32_t uMinRow, std::string & sError )
{
	if ( m_bError )
		return false;

	while ( !m_bEOF )
	{
		if ( m_uNext==m_uStats )
		{
			m_bEOF = true;
			m_dLoaded.fill(false);
			m_tRowData = {};
			m_pFieldTF.reset();
			return true;
		}

		std::array<uint8_t,LEAF_HEADER_SIZE> d;
		assert ( m_uNext<=m_uStats && d.size()<=m_uStats-m_uNext );
		if ( !m_tInput.Read ( m_uNext, d.data(), d.size(), sError ) )
			return Fail ( sError, "truncated posting leaf header" );

		assert ( !std::memcmp ( d.data(), "BLK1", 4 ) );

		Leaf_t t;
		t.m_uOffset = m_uNext;
		t.m_uBytes = uint32_t(Get(d.data()+4,4));
		t.m_uCount = uint16_t(Get(d.data()+8,2));
		t.m_uFieldWords = uint16_t(Get(d.data()+10,2));
		t.m_uFirst = uint32_t(Get(d.data()+12,4));
		t.m_uLast = uint32_t(Get(d.data()+16,4));
		t.m_uMaxTF = uint32_t(Get(d.data()+20,4));
		uint64_t uBytes = LEAF_HEADER_SIZE;
		for ( size_t i=0; i<COLUMNS; ++i )
		{
			t.m_dSizes[i] = uint32_t(Get(d.data()+24+4*i,4));
			uBytes += t.m_dSizes[i];
		}

		assert ( !(uBytes!=t.m_uBytes || uBytes>MAX_LEAF_BYTES || uBytes>m_uStats-m_uNext || !t.m_uCount || t.m_uCount>MAX_VALUES || t.m_uLast==UINT32_MAX || t.m_uFirst>t.m_uLast || (m_bHaveLeaf && t.m_uFirst<=m_uPrevious) || t.m_uFieldWords!=(m_tDesc.m_uFields+31)/32 || !t.m_uMaxTF || !t.m_dSizes[0] || !t.m_dSizes[1] || !t.m_dSizes[2] || bool(t.m_dSizes[3])!=m_tDesc.m_bPositions || bool(t.m_dSizes[4])!=m_tDesc.m_bFieldTF) );

		m_uPrevious = t.m_uLast;
		m_bHaveLeaf = true;
		m_uNext += t.m_uBytes;
		m_tLeaf = t;
		m_dLoaded.fill(false);
		m_tRowData = {};
		m_pFieldTF.reset();
		if ( t.m_uLast>=uMinRow )
			return true;
	}

	return true;
}


bool Reader_c::ReadColumn ( Column_e eColumn, Bytes_t & tData, std::vector<uint8_t> & dBuffer, std::string & sError )
{
	assert ( !(m_bError || m_bEOF || !m_bHaveLeaf) );

	size_t iColumn = size_t(eColumn);
	uint64_t uOffset = m_tLeaf.m_uOffset+LEAF_HEADER_SIZE;
	for ( size_t i=0; i<iColumn; ++i )
		uOffset += m_tLeaf.m_dSizes[i];

	size_t iSize = m_tLeaf.m_dSizes[iColumn];
	assert ( uOffset<=m_tInput.GetSize() && iSize<=m_tInput.GetSize()-uOffset );

	if ( const uint8_t * pMap = m_tInput.GetMappedData() )
		tData = { pMap+uOffset, iSize };
	else
	{
		dBuffer.resize(iSize);
		if ( iSize && !m_tInput.Read ( uOffset, dBuffer.data(), iSize, sError ) )
			return Fail ( sError, "posting column read failed" );

		tData = Bytes(dBuffer);
	}

	return true;
}


bool Reader_c::ReadRowBlock ( RowBlock_t & tBlock, std::string & sError )
{
	if ( !m_tRowData.m_pData && !ReadColumn ( Column_e::ROWS, m_tRowData, m_dRowBuffer, sError ) )
		return false;

	const uint8_t * p = m_tRowData.m_pData;
	assert ( m_tRowData.m_iSize>=ROW_HEADER_SIZE );
	assert ( Get(p+6,2)==m_tLeaf.m_uCount && Get(p+8,4)==m_tLeaf.m_uFirst && Get(p+12,4)==m_tLeaf.m_uLast );
	tBlock = { Rows_e(p[5]), m_tLeaf.m_uCount, m_tLeaf.m_uFirst, m_tLeaf.m_uLast, { p+ROW_HEADER_SIZE, size_t(Get(p+16,4)) } };
	assert ( tBlock.m_tPayload.m_iSize==m_tRowData.m_iSize-ROW_HEADER_SIZE );
	return true;
}


bool Reader_c::ReadLowFields ( uint32_t * pFields, std::string & sError )
{
	Bytes_t tData;
	if ( !ReadColumn ( Column_e::FIELDS, tData, m_dReadBuffer, sError ) )
		return false;

	++m_dDecodeCounts[size_t(Column_e::FIELDS)];
	return Frame ( m_tDesc.m_tFields, tData, pFields, m_tLeaf.m_uCount, sError );
}


bool Reader_c::ReadRows ( std::string & sError )
{
	if ( m_dLoaded[0] )
		return !m_bError;

	RowBlock_t tBlock;
	if ( !ReadRowBlock ( tBlock, sError ) )
		return false;

	DecodeRowsInto ( m_tDesc.m_tRows, m_tRowData, m_dRows.data(), MAX_VALUES, sError );
	assert ( tBlock.m_uCount==m_tLeaf.m_uCount && m_dRows[0]==m_tLeaf.m_uFirst && m_dRows[m_tLeaf.m_uCount-1]==m_tLeaf.m_uLast );

	m_dLoaded[0] = true;
	++m_dDecodeCounts[0];
	return true;
}


bool Reader_c::ReadFields ( std::string & sError )
{
	if ( m_dLoaded[1] )
		return !m_bError;

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::FIELDS, tData, m_dReadBuffer, sError ) )
		return false;

	Bytes_t t = tData;
	m_dFields.fill({});
	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( unsigned f=0; f<m_tLeaf.m_uFieldWords; ++f )
	{
		if ( !Frame ( m_tDesc.m_tFields, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
			return Fail ( sError, "invalid posting field column" );

		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
			m_dFields[i][f] = dValues[i];
	}

	assert ( CheckFields(sError) );

	assert ( !(t.m_iSize) );

	m_dLoaded[1] = true;
	++m_dDecodeCounts[1];
	return true;
}


bool Reader_c::ReadTF ( std::string & sError )
{
	if ( m_dLoaded[2] )
		return !m_bError;

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::TF, tData, m_dReadBuffer, sError ) )
		return false;

	Bytes_t t = tData;
	if ( !Frame ( m_tDesc.m_tTF, t, m_dTF.data(), m_tLeaf.m_uCount, sError ) )
		return Fail ( sError, "invalid posting TF column" );

	assert ( !t.m_iSize );

	assert ( CheckTF(sError) );

	m_dLoaded[2] = true;
	++m_dDecodeCounts[2];
	return true;
}


bool Reader_c::ReadHitRefs ( std::string & sError )
{
	if ( m_dLoaded[3] )
		return !m_bError;

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::HITREFS, tData, m_dReadBuffer, sError ) )
		return false;

	m_dHitRefs.fill(0);
	Bytes_t t = tData;
	StreamDesc_t tStream = m_tDesc.m_tHitRefs;
	uint64_t uBase = 0;
	unsigned uParts = 2;
	if ( m_tDesc.m_bPositions && tStream.m_eCodec==Codec_e::BASE_DELTA )
	{
		assert ( t.m_iSize>=9 && t.m_pData[8]<=1 );
		uBase = Get ( t.m_pData, 8 );
		uParts = 1+t.m_pData[8];
		assert ( uBase && uBase<=INT64_MAX );
		t.m_pData += 9;
		t.m_iSize -= 9;
		tStream = {Codec_e::PFOR};
	}

	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	if ( m_tDesc.m_bPositions )
		for ( unsigned part=0; part<uParts; ++part )
		{
			if ( !Frame ( tStream, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
				return Fail ( sError, "invalid posting hit reference column" );

			for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
				m_dHitRefs[i] |= uint64_t(dValues[i])<<(32*part);
		}

	if ( uBase )
		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
		{
			assert ( m_dHitRefs[i]<=uint64_t(INT64_MAX)-uBase );
			m_dHitRefs[i] += uBase;
		}

	assert ( !(t.m_iSize) );

	assert ( CheckHitRefs(sError) );

	m_dLoaded[3] = true;
	++m_dDecodeCounts[3];
	return true;
}


bool Reader_c::ReadFieldTF ( unsigned uField, std::array<uint32_t,MAX_VALUES> & dTF, std::string & sError )
{
	assert ( !(m_bError || m_bEOF || !m_bHaveLeaf || uField>=m_tDesc.m_uFields || !m_tDesc.m_bFieldTF) );

	if ( !m_pFieldTF )
	{
		auto pCache = std::make_unique<FieldTFCache_t>();
		if ( !ReadColumn ( Column_e::FIELD_TF, pCache->m_tData, pCache->m_dBuffer, sError ) )
			return false;

		Bytes_t t = pCache->m_tData;
		int iPrevious = -1;
		while ( t.m_iSize )
		{
			assert ( !(t.m_iSize<6) );

			unsigned f = unsigned(Get(t.m_pData,2));
			size_t n = size_t(Get(t.m_pData+2,4));
			assert ( !(f>=m_tDesc.m_uFields || int(f)<=iPrevious || n>t.m_iSize-6) );

			iPrevious = int(f);
			pCache->m_dOffsets[f] = size_t(t.m_pData-pCache->m_tData.m_pData)+2;
			t.m_pData += 6+n;
			t.m_iSize -= 6+n;
		}

		m_pFieldTF = std::move(pCache);
	}

	auto & dValues = m_pFieldTF->m_dValues[uField];
	if ( dValues.empty() )
	{
		dValues.resize(m_tLeaf.m_uCount);
		size_t iOffset = m_pFieldTF->m_dOffsets[uField];
		if ( iOffset )
		{
			const uint8_t * p = m_pFieldTF->m_tData.m_pData+iOffset;
			Bytes_t t { p, 4+size_t(Get(p,4)) };
			if ( !Frame ( m_tDesc.m_tTF, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
				return Fail ( sError, "invalid per-field frequency frame" );
		}

		++m_dDecodeCounts[4];
	}

	dTF.fill(0);
	std::copy ( dValues.begin(), dValues.end(), dTF.begin() );
	return true;
}


bool Reader_c::ReadStats ( std::array<FieldStats_t,MAX_FIELDS> & dStats, std::string & sError )
{
	if ( m_bError )
		return false;

	assert ( !(m_uEnd<=m_uStats) );

	std::vector<uint8_t> d ( size_t(m_uEnd-m_uStats) );
	if ( !m_tInput.Read ( m_uStats, d.data(), d.size(), sError ) )
		return Fail ( sError, "posting statistics read failed" );

	dStats.fill({});
	for ( unsigned f=0; f<m_tDesc.m_uFields; ++f )
	{
		dStats[f].m_uDocs = Get(d.data()+16*f,8);
		dStats[f].m_uTF = Get(d.data()+16*f+8,8);
		assert ( !(m_tDesc.m_bFieldTF ? dStats[f].m_uTF<dStats[f].m_uDocs : dStats[f].m_uTF!=0) );
	}

	return true;
}


bool Reader_c::CheckFields ( std::string & sError )
{

	for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
	{
		uint32_t uAny = 0;
		for ( uint32_t u : m_dFields[i] )
			uAny |= u;

		unsigned uTail = m_tDesc.m_uFields%32;
		if ( !uAny || (uTail && (m_dFields[i][m_tLeaf.m_uFieldWords-1]>>uTail)) )
			return Fail ( sError, "posting fields outside schema/empty" );
	}

	return true;
}


bool Reader_c::CheckTF ( std::string & sError )
{
	uint32_t uMax = 0;
	for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
	{
		if ( !m_dTF[i] )
			return Fail ( sError, "zero posting frequency" );

		uMax = std::max ( uMax, m_dTF[i] );
	}

	if ( uMax!=m_tLeaf.m_uMaxTF )
		return Fail ( sError, "posting maximum frequency mismatch" );

	return true;
}


bool Reader_c::CheckHitRefs ( std::string & sError )
{
	for ( unsigned i=0; m_tDesc.m_bPositions && i<m_tLeaf.m_uCount; ++i )
		if ( !m_dHitRefs[i] || m_dHitRefs[i]>INT64_MAX )
			return Fail ( sError, "invalid external posting hit reference" );

	return true;
}

}
