// Compact exact per-field document lengths.
#pragma once

#include "postings_container_codecs.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace e1::norms
{

constexpr uint32_t VERSION = 1;
constexpr uint32_t HEADER_SIZE = 64;
constexpr uint32_t FIELD_ENTRY_SIZE = 16;
constexpr uint32_t GROUP_ENTRY_SIZE = 32;
constexpr uint32_t DEFAULT_GROUP_ROWS = 4096;

inline void Put32 ( std::vector<uint8_t> & dData, size_t uOffset, uint32_t uValue )
{
	for ( unsigned i=0; i<4; ++i )
		dData[uOffset+i] = uint8_t ( uValue >> ( 8*i ) );
}

inline void Put64 ( std::vector<uint8_t> & dData, size_t uOffset, uint64_t uValue )
{
	for ( unsigned i=0; i<8; ++i )
		dData[uOffset+i] = uint8_t ( uValue >> ( 8*i ) );
}

inline unsigned WidthFor ( uint32_t uMax )
{
	return uMax<=std::numeric_limits<uint8_t>::max() ? 1 : uMax<=std::numeric_limits<uint16_t>::max() ? 2 : 4;
}

class Builder
{
public:
	explicit Builder ( uint32_t uFields, uint32_t uGroupRows=DEFAULT_GROUP_ROWS )
		: m_uFields ( uFields )
		, m_uGroupRows ( uGroupRows )
		, m_dFields ( uFields )
	{}

	bool AddRow ( const uint32_t * pValues, uint32_t uFields, std::string & sError )
	{
		if ( !pValues || uFields!=m_uFields || !m_uFields || !m_uGroupRows )
		{
			sError = "norms: invalid row";
			return false;
		}
		for ( uint32_t i=0; i<m_uFields; ++i )
			m_dFields[i].push_back ( pValues[i] );
		++m_uRows;
		return true;
	}

	bool Build ( std::vector<uint8_t> & dOut, std::string & sError ) const
	{
		if ( !m_uFields || !m_uGroupRows )
		{
			sError = "norms: invalid dimensions";
			return false;
		}
		const uint32_t uGroups = m_uRows ? ( m_uRows+m_uGroupRows-1 )/m_uGroupRows : 0;
		const uint64_t uDirectory = HEADER_SIZE+uint64_t(m_uFields)*FIELD_ENTRY_SIZE;
		const uint64_t uPayload = uDirectory+uint64_t(m_uFields)*uGroups*GROUP_ENTRY_SIZE;
		if ( uPayload>std::numeric_limits<size_t>::max() )
		{
			sError = "norms: directory too large";
			return false;
		}
		dOut.assign ( size_t(uPayload), 0 );
		memcpy ( dOut.data(), "E1NORM01", 8 );
		Put32 ( dOut, 8, VERSION );
		Put32 ( dOut, 12, HEADER_SIZE );
		Put32 ( dOut, 16, m_uRows );
		Put32 ( dOut, 20, m_uFields );
		Put32 ( dOut, 24, m_uGroupRows );
		Put32 ( dOut, 28, uGroups );
		Put64 ( dOut, 32, uDirectory );
		Put64 ( dOut, 40, uPayload );

		for ( uint32_t iField=0; iField<m_uFields; ++iField )
		{
			uint64_t uFieldSum = 0;
			uint32_t uFieldNonzero = 0;
			for ( uint32_t uValue : m_dFields[iField] )
			{
				uFieldSum += uValue;
				uFieldNonzero += uValue!=0;
			}
			const size_t uFieldEntry = HEADER_SIZE+size_t(iField)*FIELD_ENTRY_SIZE;
			Put64 ( dOut, uFieldEntry, uFieldSum );
			Put32 ( dOut, uFieldEntry+8, uFieldNonzero );

			for ( uint32_t iGroup=0; iGroup<uGroups; ++iGroup )
			{
				const uint32_t uFirst = iGroup*m_uGroupRows;
				const uint32_t uCount = std::min ( m_uGroupRows, m_uRows-uFirst );
				uint64_t uSum = 0;
				uint32_t uMax = 0;
				uint32_t uNonzero = 0;
				for ( uint32_t i=0; i<uCount; ++i )
				{
					const uint32_t uValue = m_dFields[iField][uFirst+i];
					uSum += uValue;
					uMax = std::max ( uMax, uValue );
					uNonzero += uValue!=0;
				}
				const unsigned uWidth = WidthFor(uMax);
				const uint64_t uOffset = dOut.size();
				const size_t uEntry = size_t(uDirectory)+( size_t(iField)*uGroups+iGroup )*GROUP_ENTRY_SIZE;
				Put64 ( dOut, uEntry, uOffset );
				Put64 ( dOut, uEntry+8, uSum );
				Put32 ( dOut, uEntry+16, uCount );
				Put32 ( dOut, uEntry+20, uMax );
				Put32 ( dOut, uEntry+24, uNonzero );
				dOut[uEntry+28] = uint8_t(uWidth);
				dOut.resize ( dOut.size()+size_t(uCount)*uWidth );
				for ( uint32_t i=0; i<uCount; ++i )
				{
					const uint32_t uValue = m_dFields[iField][uFirst+i];
					for ( unsigned j=0; j<uWidth; ++j )
						dOut[size_t(uOffset)+size_t(i)*uWidth+j] = uint8_t ( uValue >> ( 8*j ) );
				}
			}
		}
		Put64 ( dOut, 48, dOut.size() );
		Put32 ( dOut, 56, e1::CRC(dOut.data()+HEADER_SIZE,dOut.size()-HEADER_SIZE) );
		return true;
	}

private:
	uint32_t m_uFields = 0;
	uint32_t m_uGroupRows = 0;
	uint32_t m_uRows = 0;
	std::vector<std::vector<uint32_t>> m_dFields;
};

class Store
{
public:
	bool Open ( const uint8_t * pData, uint64_t uSize, std::string & sError )
	{
		Reset();
		auto fnFail = [&] ( const char * szError ) { sError = std::string("norms: ")+szError; return false; };
		if ( !pData || uSize<HEADER_SIZE || memcmp(pData,"E1NORM01",8) || e1::U32(pData+8)!=VERSION || e1::U32(pData+12)!=HEADER_SIZE )
			return fnFail ( "invalid header" );
		const uint32_t uRows = e1::U32(pData+16);
		const uint32_t uFields = e1::U32(pData+20);
		const uint32_t uGroupRows = e1::U32(pData+24);
		const uint32_t uGroups = e1::U32(pData+28);
		const uint64_t uDirectory = e1::U64(pData+32);
		const uint64_t uPayload = e1::U64(pData+40);
		if ( !uFields || !uGroupRows || e1::U64(pData+48)!=uSize || uGroups!=( uRows ? (uRows+uGroupRows-1)/uGroupRows : 0 ) )
			return fnFail ( "invalid dimensions" );
		if ( uDirectory!=HEADER_SIZE+uint64_t(uFields)*FIELD_ENTRY_SIZE || uPayload!=uDirectory+uint64_t(uFields)*uGroups*GROUP_ENTRY_SIZE || uPayload>uSize )
			return fnFail ( "invalid directory" );
		if ( e1::CRC(pData+HEADER_SIZE,uSize-HEADER_SIZE)!=e1::U32(pData+56) )
			return fnFail ( "checksum mismatch" );
		uint64_t uExpectedOffset = uPayload;
		for ( uint32_t iField=0; iField<uFields; ++iField )
		{
			uint64_t uFieldSum = 0;
			uint32_t uFieldNonzero = 0;
			for ( uint32_t iGroup=0; iGroup<uGroups; ++iGroup )
			{
				const uint8_t * pEntry = pData+uDirectory+( uint64_t(iField)*uGroups+iGroup )*GROUP_ENTRY_SIZE;
				const uint64_t uOffset = e1::U64(pEntry);
				const uint64_t uSum = e1::U64(pEntry+8);
				const uint32_t uCount = e1::U32(pEntry+16);
				const uint32_t uMax = e1::U32(pEntry+20);
				const uint32_t uNonzero = e1::U32(pEntry+24);
				const unsigned uWidth = pEntry[28];
				const uint32_t uFirst = iGroup*uGroupRows;
				const uint32_t uExpectedCount = std::min ( uGroupRows, uRows-uFirst );
				if ( uOffset!=uExpectedOffset || uCount!=uExpectedCount || (uWidth!=1 && uWidth!=2 && uWidth!=4) || uWidth!=WidthFor(uMax) || uNonzero>uCount )
					return fnFail ( "invalid group" );
				if ( uint64_t(uCount)*uWidth>uSize-uExpectedOffset )
					return fnFail ( "truncated payload" );
				uExpectedOffset += uint64_t(uCount)*uWidth;
				uFieldSum += uSum;
				uFieldNonzero += uNonzero;
			}
			const uint8_t * pField = pData+HEADER_SIZE+uint64_t(iField)*FIELD_ENTRY_SIZE;
			if ( e1::U64(pField)!=uFieldSum || e1::U32(pField+8)!=uFieldNonzero )
				return fnFail ( "invalid field totals" );
		}
		if ( uExpectedOffset!=uSize )
			return fnFail ( "trailing bytes" );
		m_pData = pData;
		m_uRows = uRows;
		m_uFields = uFields;
		m_uGroupRows = uGroupRows;
		m_uGroups = uGroups;
		m_uDirectory = uDirectory;
		return true;
	}

	uint32_t Rows() const { return m_uRows; }
	uint32_t Fields() const { return m_uFields; }
	uint64_t Sum ( uint32_t uField ) const { return uField<m_uFields ? e1::U64(m_pData+HEADER_SIZE+uint64_t(uField)*FIELD_ENTRY_SIZE) : 0; }
	uint32_t Nonzero ( uint32_t uField ) const { return uField<m_uFields ? e1::U32(m_pData+HEADER_SIZE+uint64_t(uField)*FIELD_ENTRY_SIZE+8) : 0; }

	bool Get ( uint32_t uField, uint32_t uRow, uint32_t & uValue ) const
	{
		if ( !m_pData || uField>=m_uFields || uRow>=m_uRows )
			return false;
		const uint32_t uGroup = uRow/m_uGroupRows;
		const uint8_t * pEntry = Entry ( uField, uGroup );
		const unsigned uWidth = pEntry[28];
		const uint8_t * pValue = m_pData+e1::U64(pEntry)+uint64_t(uRow%m_uGroupRows)*uWidth;
		uValue = uWidth==1 ? pValue[0] : uWidth==2 ? uint32_t(pValue[0])|(uint32_t(pValue[1])<<8) : e1::U32(pValue);
		return true;
	}

	bool ReadRange ( uint32_t uField, uint32_t uFirst, uint32_t uCount, uint32_t * pOut ) const
	{
		if ( !pOut || uFirst>m_uRows || uCount>m_uRows-uFirst )
			return false;
		for ( uint32_t i=0; i<uCount; ++i )
			if ( !Get(uField,uFirst+i,pOut[i]) )
				return false;
		return true;
	}

	bool Gather128 ( uint32_t uField, const uint32_t * pRows, uint32_t * pOut ) const
	{
		if ( !pRows || !pOut )
			return false;
		if ( pRows[127]-pRows[0]==127 )
			return ReadRange ( uField, pRows[0], 128, pOut );
		for ( unsigned i=0; i<128; ++i )
			if ( !Get(uField,pRows[i],pOut[i]) )
				return false;
		return true;
	}

private:
	const uint8_t * Entry ( uint32_t uField, uint32_t uGroup ) const { return m_pData+m_uDirectory+( uint64_t(uField)*m_uGroups+uGroup )*GROUP_ENTRY_SIZE; }
	void Reset() { m_pData=nullptr; m_uRows=m_uFields=m_uGroupRows=m_uGroups=0; m_uDirectory=0; }

	const uint8_t * m_pData = nullptr;
	uint32_t m_uRows = 0;
	uint32_t m_uFields = 0;
	uint32_t m_uGroupRows = 0;
	uint32_t m_uGroups = 0;
	uint64_t m_uDirectory = 0;
};

} // namespace e1::norms
