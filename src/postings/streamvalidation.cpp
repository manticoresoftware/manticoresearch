// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "streamvalidation.h"
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
	m_pFieldTF.reset();
	m_dDecodeCounts.fill(0);
	std::array<uint8_t,TERM_HEADER_SIZE> d;
	CHECK_DATA ( uOffset<=m_tInput.GetSize() && d.size()<=m_tInput.GetSize()-uOffset, "truncated posting term header" );
	if ( !m_tInput.Read ( uOffset, d.data(), d.size(), sError ) )
		return Fail ( sError, "truncated posting term header" );

	CHECK_DATA ( !std::memcmp ( d.data(), "PST2", 4 ) && Get(d.data()+4,2)==COMPONENT_VERSION, "invalid posting component/version" );

	unsigned uFlags = unsigned(Get(d.data()+6,2));
	CHECK_DATA ( !((uFlags&~3U) || Get(d.data()+10,2) || Get(d.data()+20,4) || ((uFlags&2) && !(uFlags&1))), "unsupported posting capabilities" );

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

	m_uEnd = Get(d.data()+24,8);
	m_uStats = Get(d.data()+32,8);
	m_uNext = uOffset+TERM_HEADER_SIZE;
	CHECK_DATA ( !(!m_tDesc.m_uFields || m_tDesc.m_uFields>MAX_FIELDS || m_uStats<=m_uNext || m_uStats>m_uEnd || m_uEnd>m_tInput.GetSize() || m_uEnd-m_uStats!=16*uint64_t(m_tDesc.m_uFields)), "invalid posting term extents" );

	m_bEOF = false;
	return true;
}


bool ReaderValidation_c::Next ( uint32_t uMinRow, std::string & sError )
{
	if ( m_bError )
		return false;

	while ( !m_bEOF )
	{
		if ( m_uNext==m_uStats )
		{
			m_bEOF = true;
			m_dLoaded.fill(false);
			m_pFieldTF.reset();
			return true;
		}

		std::array<uint8_t,LEAF_HEADER_SIZE> d;
		CHECK_DATA ( m_uNext<=m_uStats && d.size()<=m_uStats-m_uNext, "truncated posting leaf header" );
		if ( !m_tInput.Read ( m_uNext, d.data(), d.size(), sError ) )
			return Fail ( sError, "truncated posting leaf header" );

		CHECK_DATA ( !std::memcmp ( d.data(), "BLK1", 4 ), "invalid posting leaf header" );

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

		CHECK_DATA ( !(uBytes!=t.m_uBytes || uBytes>MAX_LEAF_BYTES || uBytes>m_uStats-m_uNext || !t.m_uCount || t.m_uCount>MAX_VALUES || t.m_uLast==UINT32_MAX || t.m_uFirst>t.m_uLast || (m_bHaveLeaf && t.m_uFirst<=m_uPrevious) || t.m_uFieldWords!=(m_tDesc.m_uFields+31)/32 || !t.m_uMaxTF || !t.m_dSizes[0] || !t.m_dSizes[1] || !t.m_dSizes[2] || bool(t.m_dSizes[3])!=m_tDesc.m_bPositions || bool(t.m_dSizes[4])!=m_tDesc.m_bFieldTF), "invalid posting leaf extents/order/capabilities" );

		m_uPrevious = t.m_uLast;
		m_bHaveLeaf = true;
		m_uNext += t.m_uBytes;
		m_tLeaf = t;
		m_dLoaded.fill(false);
		m_pFieldTF.reset();
		if ( t.m_uLast>=uMinRow )
			return true;
	}

	return true;
}


bool ReaderValidation_c::ReadColumn ( Column_e eColumn, Bytes_t & tData, std::vector<uint8_t> & dBuffer, std::string & sError )
{
	CHECK_DATA ( !(m_bError || m_bEOF || !m_bHaveLeaf), "no current posting leaf" );

	size_t iColumn = size_t(eColumn);
	uint64_t uOffset = m_tLeaf.m_uOffset+LEAF_HEADER_SIZE;
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

	Bytes_t tData;
	if ( !ReadColumn ( Column_e::TF, tData, m_dReadBuffer, sError ) )
		return false;

	Bytes_t t = tData;
	if ( !Frame ( m_tDesc.m_tTF, t, m_dTF.data(), m_tLeaf.m_uCount, sError ) )
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

	m_dHitRefs.fill(0);
	Bytes_t t = tData;
	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	if ( m_tDesc.m_bPositions )
		for ( unsigned part=0; part<2; ++part )
		{
			if ( !Frame ( m_tDesc.m_tHitRefs, t, dValues.data(), m_tLeaf.m_uCount, sError ) )
				return Fail ( sError, "invalid posting hit reference column" );

			for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
				m_dHitRefs[i] |= uint64_t(dValues[i])<<(32*part);
		}

	CHECK_DATA ( !(t.m_iSize), "trailing posting hit reference data" );

	CHECK_DATA ( CheckHitRefs(sError), "invalid external posting hit reference" );

	m_dLoaded[3] = true;
	++m_dDecodeCounts[3];
	return true;
}


bool ReaderValidation_c::ReadFieldTF ( unsigned uField, std::array<uint32_t,MAX_VALUES> & dTF, std::string & sError )
{
	CHECK_DATA ( !(m_bError || m_bEOF || !m_bHaveLeaf || uField>=m_tDesc.m_uFields || !m_tDesc.m_bFieldTF), "per-field posting frequencies unavailable" );

	if ( !m_pFieldTF )
	{
		auto pCache = std::make_unique<FieldTFCache_t>();
		if ( !ReadColumn ( Column_e::FIELD_TF, pCache->m_tData, pCache->m_dBuffer, sError ) )
			return false;

		Bytes_t t = pCache->m_tData;
		int iPrevious = -1;
		while ( t.m_iSize )
		{
			CHECK_DATA ( !(t.m_iSize<6), "truncated per-field frequency entry" );

			unsigned f = unsigned(Get(t.m_pData,2));
			size_t n = size_t(Get(t.m_pData+2,4));
			CHECK_DATA ( !(f>=m_tDesc.m_uFields || int(f)<=iPrevious || n>t.m_iSize-6), "invalid per-field frequency directory" );

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


bool ReaderValidation_c::ReadStats ( std::array<FieldStats_t,MAX_FIELDS> & dStats, std::string & sError )
{
	if ( m_bError )
		return false;

	CHECK_DATA ( !(m_uEnd<=m_uStats), "posting term is not open" );

	std::vector<uint8_t> d ( size_t(m_uEnd-m_uStats) );
	if ( !m_tInput.Read ( m_uStats, d.data(), d.size(), sError ) )
		return Fail ( sError, "posting statistics read failed" );

	dStats.fill({});
	for ( unsigned f=0; f<m_tDesc.m_uFields; ++f )
	{
		dStats[f].m_uDocs = Get(d.data()+16*f,8);
		dStats[f].m_uTF = Get(d.data()+16*f+8,8);
		CHECK_DATA ( !(m_tDesc.m_bFieldTF ? dStats[f].m_uTF<dStats[f].m_uDocs : dStats[f].m_uTF!=0), "invalid posting term field statistics" );
	}

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

	if ( !m_tDesc.m_bFieldTF )
		return true;

	std::array<uint64_t,MAX_VALUES> dTotals {};
	std::array<uint32_t,MAX_VALUES> dTF;
	for ( unsigned f=0; f<m_tDesc.m_uFields; ++f )
	{
		if ( !ReadFieldTF ( f, dTF, sError ) )
			return false;

		for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
		{
			if ( bool(dTF[i])!=bool(m_dFields[i][f/32] & (1U<<(f%32))) )
				return Fail ( sError, "per-field frequency/membership mismatch" );

			dTotals[i] += dTF[i];
		}
	}

	for ( unsigned i=0; i<m_tLeaf.m_uCount; ++i )
		if ( dTotals[i]!=m_dTF[i] )
			return Fail ( sError, "per-field/total frequency mismatch" );

	return true;
}

#undef CHECK_DATA

}
