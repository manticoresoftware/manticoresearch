// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "stream.h"
#include "shortlist.h"
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


void AppendShortValue ( std::vector<uint8_t> & dOutput, uint64_t uValue )
{
	while ( uValue>=128 )
	{
		dOutput.push_back ( uint8_t(uValue&127) | 128 );
		uValue >>= 7;
	}

	dOutput.push_back ( uint8_t(uValue) );
}


unsigned SingleField ( const Posting_t * pPostings, unsigned uCount, unsigned uFields )
{
	unsigned uField = MAX_FIELDS;
	for ( unsigned f=0; f<uFields; ++f )
		if ( pPostings[0].m_dFields[f/32] & (1U<<(f%32)) )
		{
			if ( uField!=MAX_FIELDS )
				return MAX_FIELDS;

			uField = f;
		}

	for ( unsigned i=1; i<uCount; ++i )
		if ( pPostings[i].m_dFields!=pPostings[0].m_dFields )
			return MAX_FIELDS;

	return uField;
}


bool Header ( const TermDesc_t & tDesc, uint64_t uEnd, std::array<uint8_t,TERM_HEADER_SIZE> & dHeader, std::string & sError )
{
	assert ( tDesc.m_uFields && tDesc.m_uFields<=MAX_FIELDS );

	dHeader.fill(0);
	std::memcpy ( dHeader.data(), "PST2", 4 );
	Put ( dHeader.data()+4, COMPONENT_VERSION, 2 );
	Put ( dHeader.data()+6, unsigned(tDesc.m_bPositions), 2 );
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
	if ( !Header ( tDesc, 0, dHeader, sError ) )
		return false;

	m_tDesc = tDesc;
	m_uStart = m_tOutput.GetPos();
	m_iCount = 0;
	m_bHaveRows = false;
	m_dLeaves.clear();
	m_bOpen = true;
	return m_bOpen;
}


bool Writer_c::Add ( const Posting_t & tPosting, std::string & sError )
{
	assert ( m_bOpen && tPosting.m_uRow!=UINT32_MAX && (!m_bHaveRows || tPosting.m_uRow>m_uLast) && tPosting.m_uTF );
	assert ( !m_tDesc.m_bPositions || (tPosting.m_uHitRef && tPosting.m_uHitRef<=INT64_MAX) );
#ifndef NDEBUG
	bool bFields = false;
	for ( unsigned i=0; i<MAX_FIELDS; ++i )
	{
		bool bSet = (tPosting.m_dFields[i/32] & (1U<<(i%32)))!=0;
		assert ( i<m_tDesc.m_uFields || !bSet );
		bFields |= bSet;
	}
	assert ( bFields );
#endif

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


bool Writer_c::WriteShort ( std::string & sError )
{
	std::array<std::vector<uint8_t>,COLUMNS> dColumns;
	unsigned uSingleField = SingleField ( m_dPostings.data(), unsigned(m_iCount), m_tDesc.m_uFields );
	uint32_t uMaxTF = 0, uPrevious = 0;
	uint64_t uBase = m_dPostings[0].m_uHitRef;
	bool bConstantTF = true;
	for ( size_t i=0; i<m_iCount; ++i )
	{
		const auto & t = m_dPostings[i];
		AppendShortValue ( dColumns[0], t.m_uRow-uPrevious );
		uPrevious = t.m_uRow;
		uMaxTF = std::max ( uMaxTF, t.m_uTF );
		bConstantTF &= t.m_uTF==m_dPostings[0].m_uTF;
		uBase = std::min ( uBase, t.m_uHitRef );
	}

	if ( uSingleField==MAX_FIELDS )
		for ( unsigned f=0; f<(m_tDesc.m_uFields+31)/32; ++f )
			for ( size_t i=0; i<m_iCount; ++i )
				AppendShortValue ( dColumns[1], m_dPostings[i].m_dFields[f] );

	if ( !bConstantTF )
		for ( size_t i=0; i<m_iCount; ++i )
			AppendShortValue ( dColumns[2], m_dPostings[i].m_uTF );

	if ( m_tDesc.m_bPositions )
	{
		AppendShortValue ( dColumns[3], uBase );
		if ( m_iCount>1 )
			for ( size_t i=0; i<m_iCount; ++i )
				AppendShortValue ( dColumns[3], m_dPostings[i].m_uHitRef-uBase );
	}

	Leaf_t tLeaf;
	tLeaf.m_uOffset = m_uStart;
	tLeaf.m_uCount = uint16_t(m_iCount);
	tLeaf.m_uFirst = m_dPostings[0].m_uRow;
	tLeaf.m_uLast = m_dPostings[m_iCount-1].m_uRow;
	tLeaf.m_uMaxTF = uMaxTF;
	tLeaf.m_uFieldWords = (m_tDesc.m_uFields+31)/32;
	tLeaf.m_uSingleField = uint16_t(uSingleField);
	tLeaf.m_uHeaderSize = SHORT_HEADER_SIZE;
	tLeaf.m_bShort = true;
	tLeaf.m_uBytes = SHORT_HEADER_SIZE;
	std::array<uint8_t,SHORT_HEADER_SIZE> dHeader {};
	std::memcpy ( dHeader.data(), "PSS1", 4 );
	dHeader[6] = uint8_t(m_iCount);
	dHeader[7] = m_tDesc.m_bPositions;
	Put ( dHeader.data()+8, m_tDesc.m_uFields, 2 );
	Put ( dHeader.data()+10, uSingleField, 2 );
	Put ( dHeader.data()+12, uMaxTF, 4 );
	for ( size_t i=0; i<COLUMNS; ++i )
	{
		tLeaf.m_dSizes[i] = uint32_t(dColumns[i].size());
		tLeaf.m_uBytes += tLeaf.m_dSizes[i];
		Put ( dHeader.data()+16+2*i, tLeaf.m_dSizes[i], 2 );
	}

	assert ( tLeaf.m_uBytes<=UINT16_MAX );
	Put ( dHeader.data()+4, tLeaf.m_uBytes, 2 );
	if ( !m_tOutput.Write ( {dHeader.data(),dHeader.size()}, sError ) )
		return false;

	for ( const auto & dColumn : dColumns )
		if ( !m_tOutput.Write ( Bytes(dColumn), sError ) )
			return false;

	m_dLeaves.push_back(tLeaf);
	m_iCount = 0;
	m_bOpen = false;
	return true;
}


bool Writer_c::Flush ( std::string & sError )
{
	if ( !m_iCount )
		return true;

	if ( m_dLeaves.empty() )
	{
		std::array<uint8_t,TERM_HEADER_SIZE> dHeader;
		if ( !Header ( m_tDesc, 0, dHeader, sError ) || !m_tOutput.Write ( {dHeader.data(),dHeader.size()}, sError ) )
			return false;
	}

	std::array<std::vector<uint8_t>,COLUMNS> dColumns;
	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( size_t i=0; i<m_iCount; ++i )
		dValues[i] = m_dPostings[i].m_uRow;

	if ( !EncodeRows ( m_tDesc.m_tRows, dValues.data(), m_iCount, dColumns[0], sError ) )
		return false;

	unsigned uWords = (m_tDesc.m_uFields+31)/32;
	unsigned uSingleField = SingleField ( m_dPostings.data(), unsigned(m_iCount), m_tDesc.m_uFields );
	for ( unsigned f=0; uSingleField==MAX_FIELDS && f<uWords; ++f )
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

	if ( !std::all_of ( dValues.begin(), dValues.begin()+m_iCount, [uMaxTF] ( uint32_t uTF ) { return uTF==uMaxTF; } ) && !AddFrame ( m_tDesc.m_tTF, dValues.data(), m_iCount, dColumns[2], sError ) )
		return false;

	if ( !EncodeHitRefs ( dColumns[3], sError ) )
		return false;

	Leaf_t tLeaf;
	tLeaf.m_uOffset = m_tOutput.GetPos();
	tLeaf.m_uCount = uint16_t(m_iCount);
	tLeaf.m_uFirst = m_dPostings[0].m_uRow;
	tLeaf.m_uLast = m_dPostings[m_iCount-1].m_uRow;
	tLeaf.m_uMaxTF = uMaxTF;
	tLeaf.m_uFieldWords = uint16_t(uWords);
	tLeaf.m_uSingleField = uint16_t(uSingleField);
	tLeaf.m_uBytes = LEAF_HEADER_SIZE;
	std::array<uint8_t,LEAF_HEADER_SIZE> dHeader {};
	std::memcpy ( dHeader.data(), "BLK1", 4 );
	Put ( dHeader.data()+8, tLeaf.m_uCount, 2 );
	Put ( dHeader.data()+10, uSingleField==MAX_FIELDS ? uWords : (0x8000|uSingleField), 2 );
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

	if ( m_dLeaves.empty() && m_iCount<=SHORT_LIST_MAX )
		return WriteShort(sError);

	if ( !Flush(sError) )
		return false;

	std::array<uint8_t,TERM_HEADER_SIZE> dHeader;
	if ( !Header ( m_tDesc, m_tOutput.GetPos(), dHeader, sError ) || !m_tOutput.Patch ( m_uStart, {dHeader.data(),dHeader.size()}, sError ) )
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
	m_dDecodeCounts.fill(0);
	m_tLeaf = {};
	std::array<uint8_t,TERM_HEADER_SIZE> d {};
	assert ( uOffset<=m_tInput.GetSize() && 8<=m_tInput.GetSize()-uOffset );
	if ( !m_tInput.Read ( uOffset, d.data(), 8, sError ) )
		return Fail ( sError, "truncated posting term header" );

	bool bShort = !std::memcmp ( d.data(), "PSS1", 4 );
	size_t iHeader = bShort ? SHORT_HEADER_SIZE : TERM_HEADER_SIZE;
	assert ( iHeader<=m_tInput.GetSize()-uOffset );
	if ( !m_tInput.Read ( uOffset+8, d.data()+8, iHeader-8, sError ) )
		return Fail ( sError, "truncated posting term header" );

	if ( bShort )
	{
		m_tDesc = {};
		m_tDesc.m_bPositions = d[7]&1;
		m_tDesc.m_uFields = uint16_t(Get(d.data()+8,2));
		m_tLeaf.m_bShort = true;
		m_tLeaf.m_uOffset = uOffset;
		m_tLeaf.m_uHeaderSize = SHORT_HEADER_SIZE;
		m_tLeaf.m_uBytes = uint32_t(Get(d.data()+4,2));
		m_tLeaf.m_uCount = d[6];
		m_tLeaf.m_uFieldWords = (m_tDesc.m_uFields+31)/32;
		m_tLeaf.m_uSingleField = uint16_t(Get(d.data()+10,2));
		m_tLeaf.m_uMaxTF = uint32_t(Get(d.data()+12,4));
		uint64_t uBytes = SHORT_HEADER_SIZE;
		for ( size_t i=0; i<COLUMNS; ++i )
		{
			m_tLeaf.m_dSizes[i] = uint32_t(Get(d.data()+16+2*i,2));
			uBytes += m_tLeaf.m_dSizes[i];
		}

		if ( !ShortCheck<false> ( !(d[7]&~1U) && m_tDesc.m_uFields && m_tDesc.m_uFields<=MAX_FIELDS && m_tLeaf.m_uCount && m_tLeaf.m_uCount<=SHORT_LIST_MAX && m_tLeaf.m_uMaxTF && uBytes==m_tLeaf.m_uBytes && uBytes<=m_tInput.GetSize()-uOffset && m_tLeaf.m_dSizes[0] && (m_tLeaf.m_uSingleField==MAX_FIELDS ? m_tLeaf.m_dSizes[1]!=0 : m_tLeaf.m_uSingleField<m_tDesc.m_uFields && !m_tLeaf.m_dSizes[1]) && bool(m_tLeaf.m_dSizes[3])==m_tDesc.m_bPositions, sError ) )
			return Fail ( sError, "invalid short posting header" );

		m_uEnd = uOffset+uBytes;
		m_uNext = uOffset;
		m_bEOF = false;
		return true;
	}

	if ( std::memcmp ( d.data(), "PST2", 4 ) || Get(d.data()+4,2)!=COMPONENT_VERSION )
		return Fail ( sError, "unsupported posting component/version" );

	unsigned uFlags = unsigned(Get(d.data()+6,2));
	if ( (uFlags&~1U) || Get(d.data()+10,2) || Get(d.data()+20,4) )
		return Fail ( sError, "unsupported posting capabilities" );

	m_tDesc.m_bPositions = (uFlags&1)!=0;
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
	m_uNext = uOffset+TERM_HEADER_SIZE;
	assert ( !(!m_tDesc.m_uFields || m_tDesc.m_uFields>MAX_FIELDS || m_uEnd<=m_uNext || m_uEnd>m_tInput.GetSize()) );

	m_bEOF = false;
	return true;
}


bool Reader_c::Next ( uint32_t uMinRow, std::string & sError )
{
	if ( m_bError )
		return false;

	if ( m_tLeaf.m_bShort && !m_bEOF && m_uNext!=m_uEnd )
	{
		m_bHaveLeaf = true;
		if ( !ReadRows(sError) )
			return false;

		m_uNext = m_uEnd;
		if ( m_tLeaf.m_uLast>=uMinRow )
			return true;
	}

	while ( !m_bEOF )
	{
		if ( m_uNext==m_uEnd )
		{
			m_bEOF = true;
			m_dLoaded.fill(false);
			m_tRowData = {};
			return true;
		}

		std::array<uint8_t,LEAF_HEADER_SIZE> d;
		assert ( m_uNext<=m_uEnd && d.size()<=m_uEnd-m_uNext );
		if ( !m_tInput.Read ( m_uNext, d.data(), d.size(), sError ) )
			return Fail ( sError, "truncated posting leaf header" );

		assert ( !std::memcmp ( d.data(), "BLK1", 4 ) );

		Leaf_t t;
		t.m_uOffset = m_uNext;
		t.m_uBytes = uint32_t(Get(d.data()+4,4));
		t.m_uCount = uint16_t(Get(d.data()+8,2));
		unsigned uFieldCode = unsigned(Get(d.data()+10,2));
		t.m_uSingleField = (uFieldCode&0x8000) ? uint16_t(uFieldCode&0x7fff) : MAX_FIELDS;
		t.m_uFieldWords = (uFieldCode&0x8000) ? (m_tDesc.m_uFields+31)/32 : uint16_t(uFieldCode);
		t.m_uFirst = uint32_t(Get(d.data()+12,4));
		t.m_uLast = uint32_t(Get(d.data()+16,4));
		t.m_uMaxTF = uint32_t(Get(d.data()+20,4));
		uint64_t uBytes = LEAF_HEADER_SIZE;
		for ( size_t i=0; i<COLUMNS; ++i )
		{
			t.m_dSizes[i] = uint32_t(Get(d.data()+24+4*i,4));
			uBytes += t.m_dSizes[i];
		}

		assert ( !(uBytes!=t.m_uBytes || uBytes>MAX_LEAF_BYTES || uBytes>m_uEnd-m_uNext || !t.m_uCount || t.m_uCount>MAX_VALUES || t.m_uLast==UINT32_MAX || t.m_uFirst>t.m_uLast || (m_bHaveLeaf && t.m_uFirst<=m_uPrevious) || t.m_uFieldWords!=(m_tDesc.m_uFields+31)/32 || !t.m_uMaxTF || !t.m_dSizes[0] || (t.m_uSingleField==MAX_FIELDS ? !t.m_dSizes[1] : t.m_uSingleField>=m_tDesc.m_uFields || t.m_dSizes[1]) || bool(t.m_dSizes[3])!=m_tDesc.m_bPositions) );

		m_uPrevious = t.m_uLast;
		m_bHaveLeaf = true;
		m_uNext += t.m_uBytes;
		m_tLeaf = t;
		m_dLoaded.fill(false);
		m_tRowData = {};
		if ( t.m_uLast>=uMinRow )
			return true;
	}

	return true;
}


bool Reader_c::ReadColumn ( Column_e eColumn, Bytes_t & tData, std::vector<uint8_t> & dBuffer, std::string & sError )
{
	assert ( !(m_bError || m_bEOF || !m_bHaveLeaf) );

	size_t iColumn = size_t(eColumn);
	uint64_t uOffset = m_tLeaf.m_uOffset+m_tLeaf.m_uHeaderSize;
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
	if ( m_tLeaf.m_bShort )
	{
		if ( !ReadRows(sError) )
			return false;

		tBlock = { m_tLeaf.m_uCount==1 ? Rows_e::RANGE : Rows_e::DELTA, m_tLeaf.m_uCount, m_tLeaf.m_uFirst, m_tLeaf.m_uLast, {} };
		return true;
	}

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
	if ( m_tLeaf.m_uSingleField!=MAX_FIELDS )
	{
		uint32_t uMask = m_tLeaf.m_uSingleField<32 ? 1U<<m_tLeaf.m_uSingleField : 0;
		std::fill_n ( pFields, m_tLeaf.m_uCount, uMask );
		++m_dDecodeCounts[size_t(Column_e::FIELDS)];
		return true;
	}

	if ( m_tLeaf.m_bShort )
	{
		if ( !ReadFields(sError) )
			return false;

		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
			pFields[i] = m_dFields[i][0];

		return true;
	}

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

	if ( m_tLeaf.m_bShort )
	{
		Bytes_t tData;
		if ( !ReadColumn ( Column_e::ROWS, tData, m_dReadBuffer, sError ) || !ShortRows<false> ( tData, m_tLeaf.m_uCount, m_dRows.data(), sError ) )
			return Fail ( sError, "invalid short posting rows" );

		m_tLeaf.m_uFirst = m_dRows[0];
		m_tLeaf.m_uLast = m_dRows[m_tLeaf.m_uCount-1];
		m_dLoaded[0] = true;
		++m_dDecodeCounts[0];
		return true;
	}

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

	if ( m_tLeaf.m_uSingleField!=MAX_FIELDS )
	{
		std::fill_n ( m_dFields.begin(), m_tLeaf.m_uCount, std::array<uint32_t,FIELD_WORDS>{} );
		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
			m_dFields[i][m_tLeaf.m_uSingleField/32] = 1U<<(m_tLeaf.m_uSingleField%32);

		m_dLoaded[1] = true;
		++m_dDecodeCounts[1];
		return true;
	}

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::FIELDS, tData, m_dReadBuffer, sError ) )
		return false;

	Bytes_t t = tData;
	std::fill_n ( m_dFields.begin(), m_tLeaf.m_uCount, std::array<uint32_t,FIELD_WORDS>{} );

	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( unsigned f=0; f<m_tLeaf.m_uFieldWords; ++f )
	{
		if ( m_tLeaf.m_bShort )
		{
			for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
			{
				uint64_t uValue;
				if ( !ShortValue<false> ( t, uValue, sError ) || !ShortCheck<false> ( uValue<=UINT32_MAX, sError ) )
					return Fail ( sError, "invalid short posting fields" );

				dValues[i] = uint32_t(uValue);
			}
		} else if ( !Frame ( m_tDesc.m_tFields, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
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

	if ( !m_tLeaf.m_dSizes[2] )
	{
		std::fill_n ( m_dTF.data(), m_tLeaf.m_uCount, m_tLeaf.m_uMaxTF );
		m_dLoaded[2] = true;
		++m_dDecodeCounts[2];
		return true;
	}

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::TF, tData, m_dReadBuffer, sError ) )
		return false;

	Bytes_t t = tData;
	if ( m_tLeaf.m_bShort )
	{
		if ( !ShortIntegers<false> ( t, m_tLeaf.m_uCount, m_dTF.data(), sError ) )
			return Fail ( sError, "invalid short posting frequencies" );

		t.m_iSize = 0;
	} else if ( !Frame ( m_tDesc.m_tTF, t, m_dTF.data(), m_tLeaf.m_uCount, sError ) )
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

	std::fill_n ( m_dHitRefs.begin(), m_tLeaf.m_uCount, 0 );
	if ( m_tLeaf.m_bShort )
	{
		if ( m_tDesc.m_bPositions && !ShortHitRefs<false> ( tData, m_tLeaf.m_uCount, m_dHitRefs.data(), sError ) )
			return Fail ( sError, "invalid short posting hit references" );

		m_dLoaded[3] = true;
		++m_dDecodeCounts[3];
		return true;
	}

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
