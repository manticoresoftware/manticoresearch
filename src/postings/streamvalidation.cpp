// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "streamvalidation.h"
#include "shortlist.h"
#include <algorithm>
#include <cassert>
#include <cstring>

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


bool Error ( std::string & sError, const char * sMessage )
{
	sError = sMessage;
	return false;
}


bool Frame ( StreamDesc_t tStream, Bytes_t & tData, uint32_t * pValues, size_t iCount, std::string & sError )
{
	if ( tData.m_iSize<4 )
		return Error ( sError, "truncated posting column" );

	assert ( tData.m_iSize>=4 );
	size_t iSize = size_t(Get(tData.m_pData,4));
	if ( iSize>tData.m_iSize-4 )
		return Error ( sError, "posting frame exceeds column" );

	assert ( iSize<=tData.m_iSize-4 );
	Bytes_t tFrame { tData.m_pData+4, iSize };
	IntegerDesc_t tDesc;
	if ( !InspectIntegers ( tStream, tFrame, tDesc, sError ) || tDesc.m_uCount!=iCount )
		return Error ( sError, "posting frame count/encoding mismatch" );

	assert ( tFrame.m_iSize>=INTEGER_HEADER_SIZE && Get(tFrame.m_pData+2,2)==iCount );
	DecodeIntegersInto ( tStream, tFrame, pValues, iCount, sError );
	tData.m_pData += 4+iSize;
	tData.m_iSize -= 4+iSize;
	return true;
}

}

#define CHECK_DATA(CONDITION,MESSAGE) \
	do { if ( !(CONDITION) ) return Fail ( sError, MESSAGE ); } while (false)

ReaderValidation_c::ReaderValidation_c ( Input_i & tInput ) : m_tInput(tInput) {}


bool ReaderValidation_c::Fail ( std::string & sError, const char * sMessage )
{
	m_bError = true;
	return Error ( sError, sMessage );
}


bool ReaderValidation_c::Open ( uint64_t uOffset, std::string & sError )
{
	m_bEOF = true;
	m_bError = false;
	m_bHaveLeaf = false;
	m_dLoaded.fill(false);
	m_dDecodeCounts.fill(0);
	m_tLeaf = {};
	std::array<uint8_t,TERM_HEADER_SIZE> d {};
	CHECK_DATA ( uOffset<=m_tInput.GetSize() && 8<=m_tInput.GetSize()-uOffset, "truncated posting term header" );
	if ( !m_tInput.Read ( uOffset, d.data(), 8, sError ) )
		return Fail ( sError, "truncated posting term header" );

	bool bShort = !std::memcmp ( d.data(), "PSS1", 4 );
	size_t iHeader = bShort ? SHORT_HEADER_SIZE : TERM_HEADER_SIZE;
	CHECK_DATA ( iHeader<=m_tInput.GetSize()-uOffset, "truncated posting term header" );
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

		if ( !ShortCheck<true> ( !(d[7]&~1U) && m_tDesc.m_uFields && m_tDesc.m_uFields<=MAX_FIELDS && m_tLeaf.m_uCount && m_tLeaf.m_uCount<=SHORT_LIST_MAX && m_tLeaf.m_uMaxTF && uBytes==m_tLeaf.m_uBytes && uBytes<=m_tInput.GetSize()-uOffset && m_tLeaf.m_dSizes[0] && (m_tLeaf.m_uSingleField==MAX_FIELDS ? m_tLeaf.m_dSizes[1]!=0 : m_tLeaf.m_uSingleField<m_tDesc.m_uFields && !m_tLeaf.m_dSizes[1]) && bool(m_tLeaf.m_dSizes[3])==m_tDesc.m_bPositions, sError ) )
			return Fail ( sError, "invalid short posting header" );

		m_uEnd = uOffset+uBytes;
		m_uNext = uOffset;
		m_bEOF = false;
		return true;
	}

	if ( std::memcmp ( d.data(), "PST2", 4 ) || Get(d.data()+4,2)!=COMPONENT_VERSION )
		return Fail ( sError, "unsupported posting component/version" );

	unsigned uFlags = unsigned(Get(d.data()+6,2));
	CHECK_DATA ( !((uFlags&~1U) || Get(d.data()+10,2) || Get(d.data()+20,4)), "unsupported posting capabilities" );

	m_tDesc.m_bPositions = (uFlags&1)!=0;
	m_tDesc.m_uFields = uint16_t(Get(d.data()+8,2));
	StreamDesc_t * dStreams[] = { &m_tDesc.m_tRows, &m_tDesc.m_tFields, &m_tDesc.m_tTF, &m_tDesc.m_tHitRefs };
	for ( unsigned i=0; i<4; ++i )
		if ( !ReadStreamDescriptor ( {d.data()+12+2*i,2}, *dStreams[i], sError ) )
		{
			m_bError = true;
			return false;
		}

	CHECK_DATA ( m_tDesc.m_tRows.m_eCodec!=Codec_e::BASE_DELTA && m_tDesc.m_tFields.m_eCodec!=Codec_e::BASE_DELTA && m_tDesc.m_tTF.m_eCodec!=Codec_e::BASE_DELTA, "invalid integer stream codec" );

	m_uEnd = Get(d.data()+24,8);
	m_uNext = uOffset+TERM_HEADER_SIZE;
	CHECK_DATA ( !(!m_tDesc.m_uFields || m_tDesc.m_uFields>MAX_FIELDS || m_uEnd<=m_uNext || m_uEnd>m_tInput.GetSize()), "invalid posting term extents" );

	m_bEOF = false;
	return true;
}


bool ReaderValidation_c::Next ( uint32_t uMinRow, std::string & sError )
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
			return true;
		}

		std::array<uint8_t,LEAF_HEADER_SIZE> d;
		CHECK_DATA ( m_uNext<=m_uEnd && d.size()<=m_uEnd-m_uNext, "truncated posting leaf header" );
		if ( !m_tInput.Read ( m_uNext, d.data(), d.size(), sError ) )
			return Fail ( sError, "truncated posting leaf header" );

		CHECK_DATA ( !std::memcmp ( d.data(), "BLK1", 4 ), "invalid posting leaf header" );

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

		CHECK_DATA ( !(uBytes!=t.m_uBytes || uBytes>MAX_LEAF_BYTES || uBytes>m_uEnd-m_uNext || !t.m_uCount || t.m_uCount>MAX_VALUES || t.m_uLast==UINT32_MAX || t.m_uFirst>t.m_uLast || (m_bHaveLeaf && t.m_uFirst<=m_uPrevious) || t.m_uFieldWords!=(m_tDesc.m_uFields+31)/32 || !t.m_uMaxTF || !t.m_dSizes[0] || (t.m_uSingleField==MAX_FIELDS ? !t.m_dSizes[1] : t.m_uSingleField>=m_tDesc.m_uFields || t.m_dSizes[1]) || bool(t.m_dSizes[3])!=m_tDesc.m_bPositions), "invalid posting leaf extents/order/capabilities" );

		m_uPrevious = t.m_uLast;
		m_bHaveLeaf = true;
		m_uNext += t.m_uBytes;
		m_tLeaf = t;
		m_dLoaded.fill(false);
		if ( t.m_uLast>=uMinRow )
			return true;
	}

	return true;
}


bool ReaderValidation_c::ReadColumn ( Column_e eColumn, Bytes_t & tData, std::vector<uint8_t> & dBuffer, std::string & sError )
{
	CHECK_DATA ( !(m_bError || m_bEOF || !m_bHaveLeaf), "no current posting leaf" );

	size_t iColumn = size_t(eColumn);
	uint64_t uOffset = m_tLeaf.m_uOffset+m_tLeaf.m_uHeaderSize;
	for ( size_t i=0; i<iColumn; ++i )
		uOffset += m_tLeaf.m_dSizes[i];

	size_t iSize = m_tLeaf.m_dSizes[iColumn];
	CHECK_DATA ( uOffset<=m_tInput.GetSize() && iSize<=m_tInput.GetSize()-uOffset, "posting column outside file" );

	if ( const uint8_t * pMap = m_tInput.GetMappedData() )
		tData = { pMap+uOffset, iSize };
	else
	{
		dBuffer.resize(iSize);
		if ( iSize && !m_tInput.Read ( uOffset, dBuffer.data(), iSize, sError ) )
			return Fail ( sError, "posting column read failed" );

		tData = { dBuffer.data(), dBuffer.size() };
	}

	return true;
}


bool ReaderValidation_c::ReadRows ( std::string & sError )
{
	if ( m_dLoaded[0] )
		return !m_bError;

	if ( m_tLeaf.m_bShort )
	{
		Bytes_t tData;
		if ( !ReadColumn ( Column_e::ROWS, tData, m_dReadBuffer, sError ) || !ShortRows<true> ( tData, m_tLeaf.m_uCount, m_dRows.data(), sError ) )
			return Fail ( sError, "invalid short posting rows" );

		m_tLeaf.m_uFirst = m_dRows[0];
		m_tLeaf.m_uLast = m_dRows[m_tLeaf.m_uCount-1];
		m_dLoaded[0] = true;
		++m_dDecodeCounts[0];
		return true;
	}

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::ROWS, tData, m_dReadBuffer, sError ) )
		return false;

	RowBlock_t t;
	if ( !InspectRows ( m_tDesc.m_tRows, tData, t, sError ) || t.m_uCount!=m_tLeaf.m_uCount || t.m_uFirst!=m_tLeaf.m_uFirst || t.m_uLast!=m_tLeaf.m_uLast )
		return Fail ( sError, "invalid posting row column" );

	DecodeRowsInto ( m_tDesc.m_tRows, tData, m_dRows.data(), MAX_VALUES, sError );
	assert ( Get(tData.m_pData+6,2)==m_tLeaf.m_uCount && m_dRows[0]==m_tLeaf.m_uFirst && m_dRows[m_tLeaf.m_uCount-1]==m_tLeaf.m_uLast );

	m_dLoaded[0] = true;
	++m_dDecodeCounts[0];
	return true;
}


bool ReaderValidation_c::ReadFields ( std::string & sError )
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
				if ( !ShortValue<true> ( t, uValue, sError ) || !ShortCheck<true> ( uValue<=UINT32_MAX, sError ) )
					return Fail ( sError, "invalid short posting fields" );

				dValues[i] = uint32_t(uValue);
			}
		} else if ( !Frame ( m_tDesc.m_tFields, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
			return Fail ( sError, "invalid posting field column" );

		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
			m_dFields[i][f] = dValues[i];
	}

	CHECK_DATA ( CheckFields(sError), "posting fields outside schema/empty" );

	CHECK_DATA ( !(t.m_iSize), "trailing posting field data" );

	m_dLoaded[1] = true;
	++m_dDecodeCounts[1];
	return true;
}


bool ReaderValidation_c::ReadTF ( std::string & sError )
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
		if ( !ShortIntegers<true> ( t, m_tLeaf.m_uCount, m_dTF.data(), sError ) )
			return Fail ( sError, "invalid short posting frequencies" );

		t.m_iSize = 0;
	} else if ( !Frame ( m_tDesc.m_tTF, t, m_dTF.data(), m_tLeaf.m_uCount, sError ) )
		return Fail ( sError, "invalid posting TF column" );

	CHECK_DATA ( !t.m_iSize, "trailing posting TF data" );

	CHECK_DATA ( CheckTF(sError), "invalid posting frequency/maximum" );

	m_dLoaded[2] = true;
	++m_dDecodeCounts[2];
	return true;
}


bool ReaderValidation_c::ReadHitRefs ( std::string & sError )
{
	if ( m_dLoaded[3] )
		return !m_bError;

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::HITREFS, tData, m_dReadBuffer, sError ) )
		return false;

	std::fill_n ( m_dHitRefs.begin(), m_tLeaf.m_uCount, 0 );
	if ( m_tLeaf.m_bShort )
	{
		if ( m_tDesc.m_bPositions && !ShortHitRefs<true> ( tData, m_tLeaf.m_uCount, m_dHitRefs.data(), sError ) )
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
		CHECK_DATA ( t.m_iSize>=9 && t.m_pData[8]<=1, "invalid base-relative hit reference header" );
		uBase = Get ( t.m_pData, 8 );
		uParts = 1+t.m_pData[8];
		CHECK_DATA ( uBase && uBase<=INT64_MAX, "invalid hit reference base" );
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
			CHECK_DATA ( m_dHitRefs[i]<=uint64_t(INT64_MAX)-uBase, "hit reference overflow" );
			m_dHitRefs[i] += uBase;
		}

	CHECK_DATA ( !(t.m_iSize), "trailing posting hit reference data" );

	CHECK_DATA ( CheckHitRefs(sError), "invalid external posting hit reference" );

	m_dLoaded[3] = true;
	++m_dDecodeCounts[3];
	return true;
}


bool ReaderValidation_c::CheckFields ( std::string & sError )
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


bool ReaderValidation_c::CheckTF ( std::string & sError )
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


bool ReaderValidation_c::CheckHitRefs ( std::string & sError )
{
	for ( unsigned i=0; m_tDesc.m_bPositions && i<m_tLeaf.m_uCount; ++i )
		if ( !m_dHitRefs[i] || m_dHitRefs[i]>INT64_MAX )
			return Fail ( sError, "invalid external posting hit reference" );

	return true;
}


bool ReaderValidation_c::Validate ( std::string & sError )
{
	if ( !ReadRows(sError) || !ReadFields(sError) || !ReadTF(sError) || !ReadHitRefs(sError) )
		return false;

	return true;
}

#undef CHECK_DATA

}
