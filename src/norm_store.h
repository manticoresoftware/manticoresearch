// Compact exact per-field document lengths.
#pragma once

#include "postings_container_codecs.h"
#include "field_norms.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
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

	bool Set ( uint32_t uRow, uint32_t uField, uint32_t uValue, std::string & sError )
	{
		if ( uField>=m_uFields || uRow>=m_uRows )
		{
			sError = "norms: invalid row update";
			return false;
		}
		m_dFields[uField][uRow] = uValue;
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

// Bounded-memory writer used by production indexing. Values are staged in one
// anonymous file per field, then transcoded group-by-group into the final store.
class StagedBuilder
{
public:
	explicit StagedBuilder ( uint32_t uFields, uint32_t uGroupRows=DEFAULT_GROUP_ROWS, bool bMutable=false )
		: m_uFields ( uFields )
		, m_uGroupRows ( uGroupRows )
		, m_bMutable ( bMutable )
	{
		if ( !m_uGroupRows )
			return;
		m_dFields.resize ( m_uFields, nullptr );
		m_dStaging.resize ( m_uFields );
		m_dGroupMeta.resize ( m_uFields );
		for ( FILE * & pFile : m_dFields )
			if ( !( pFile=tmpfile() ) )
				return;
		for ( auto & dField : m_dStaging )
			dField.reserve ( m_uGroupRows );
		m_bValid = true;
	}

	~StagedBuilder()
	{
		for ( FILE * pFile : m_dFields )
			if ( pFile )
				fclose ( pFile );
	}

	StagedBuilder ( const StagedBuilder & ) = delete;
	StagedBuilder & operator = ( const StagedBuilder & ) = delete;

	template<typename VALUE>
	bool AddRow ( const VALUE * pValues, uint32_t uFields, std::string & sError )
	{
		static_assert ( sizeof(VALUE)==sizeof(uint32_t), "norm values must be 32-bit" );
		if ( !m_bValid || ( m_uFields && !pValues ) || uFields!=m_uFields || m_uRows==std::numeric_limits<uint32_t>::max() )
			return Fail ( sError, "invalid staged row" );
		for ( uint32_t i=0; i<m_uFields; ++i )
			m_dStaging[i].push_back ( uint32_t(pValues[i]) );
		++m_uRows;
		if ( m_uFields && m_dStaging[0].size()==m_uGroupRows && !FlushStaging(sError) )
			return false;
		return true;
	}

	bool Set ( uint32_t uRow, uint32_t uField, uint32_t uValue, std::string & sError )
	{
		if ( !m_bValid || uField>=m_uFields || uRow>=m_uRows )
			return Fail ( sError, "invalid staged row update" );
		const uint32_t uBuffered = m_uFields ? uint32_t(m_dStaging[0].size()) : 0;
		const uint32_t uBufferedFirst = m_uRows-uBuffered;
		if ( uRow>=uBufferedFirst )
		{
			m_dStaging[uField][uRow-uBufferedFirst] = uValue;
			return true;
		}
		if ( !m_bMutable )
			return Fail ( sError, "staged row is immutable" );
		FILE * pFile = m_dFields[uField];
		if ( !Seek(pFile,uint64_t(uRow)*sizeof(uint32_t)) || fwrite(&uValue,sizeof(uValue),1,pFile)!=1 || !Seek(pFile,uint64_t(uBufferedFirst)*sizeof(uint32_t)) )
			return Fail ( sError, "staging update failed" );
		return true;
	}

	bool Finish ( const char * szOutput, std::string & sError )
	{
		if ( !m_bValid || !szOutput || !*szOutput )
			return Fail ( sError, "invalid staged builder" );
		if ( !FlushStaging(sError) )
			return false;
		if ( !m_bMutable )
			return FinishPacked ( szOutput, sError );
		for ( FILE * pFile : m_dFields )
			if ( fflush(pFile) )
				return Fail ( sError, "staging flush failed" );

		FILE * pMetadata = tmpfile();
		if ( !pMetadata )
			return Fail ( sError, "metadata staging failed" );
		const uint32_t uGroups = m_uRows ? ( m_uRows+m_uGroupRows-1 )/m_uGroupRows : 0;
		const uint64_t uDirectory = HEADER_SIZE+uint64_t(m_uFields)*FIELD_ENTRY_SIZE;
		uint64_t uOffset = uDirectory+uint64_t(m_uFields)*uGroups*GROUP_ENTRY_SIZE;
		std::vector<uint8_t> dFieldEntries ( size_t(m_uFields)*FIELD_ENTRY_SIZE, 0 );
		std::vector<uint32_t> dValues ( m_uGroupRows );

		for ( uint32_t iField=0; iField<m_uFields; ++iField )
		{
			FILE * pField = m_dFields[iField];
			if ( !Seek(pField,0) )
				return CloseMetadata ( pMetadata, sError, "staging seek failed" );
			uint64_t uFieldSum = 0;
			uint32_t uFieldNonzero = 0;
			for ( uint32_t iGroup=0; iGroup<uGroups; ++iGroup )
			{
				const uint32_t uCount = std::min ( m_uGroupRows, m_uRows-iGroup*m_uGroupRows );
				if ( fread(dValues.data(),sizeof(uint32_t),uCount,pField)!=uCount )
					return CloseMetadata ( pMetadata, sError, "staging read failed" );
				uint64_t uSum = 0;
				uint32_t uMax = 0;
				uint32_t uNonzero = 0;
				for ( uint32_t i=0; i<uCount; ++i )
				{
					uSum += dValues[i];
					uMax = std::max ( uMax, dValues[i] );
					uNonzero += dValues[i]!=0;
				}
				const unsigned uWidth = WidthFor(uMax);
				std::vector<uint8_t> dEntry ( GROUP_ENTRY_SIZE, 0 );
				Put64 ( dEntry, 0, uOffset );
				Put64 ( dEntry, 8, uSum );
				Put32 ( dEntry, 16, uCount );
				Put32 ( dEntry, 20, uMax );
				Put32 ( dEntry, 24, uNonzero );
				dEntry[28] = uint8_t(uWidth);
				if ( fwrite(dEntry.data(),1,dEntry.size(),pMetadata)!=dEntry.size() )
					return CloseMetadata ( pMetadata, sError, "metadata write failed" );
				uOffset += uint64_t(uCount)*uWidth;
				uFieldSum += uSum;
				uFieldNonzero += uNonzero;
			}
			const size_t uFieldEntry = size_t(iField)*FIELD_ENTRY_SIZE;
			Put64 ( dFieldEntries, uFieldEntry, uFieldSum );
			Put32 ( dFieldEntries, uFieldEntry+8, uFieldNonzero );
		}

		FILE * pOutput = fopen ( szOutput, "wb" );
		if ( !pOutput )
		{
			const int iError = errno;
			fclose ( pMetadata );
			return FailSystem ( sError, "unable to open output", szOutput, iError );
		}
		std::vector<uint8_t> dHeader ( HEADER_SIZE, 0 );
		memcpy ( dHeader.data(), "E1NORM01", 8 );
		Put32 ( dHeader, 8, VERSION );
		Put32 ( dHeader, 12, HEADER_SIZE );
		Put32 ( dHeader, 16, m_uRows );
		Put32 ( dHeader, 20, m_uFields );
		Put32 ( dHeader, 24, m_uGroupRows );
		Put32 ( dHeader, 28, uGroups );
		Put64 ( dHeader, 32, uDirectory );
		Put64 ( dHeader, 40, uDirectory+uint64_t(m_uFields)*uGroups*GROUP_ENTRY_SIZE );
		Put64 ( dHeader, 48, uOffset );

		uint32_t uCRC = ~0u;
		bool bOK = Write ( pOutput, dHeader.data(), dHeader.size(), nullptr )
			&& Write ( pOutput, dFieldEntries.data(), dFieldEntries.size(), &uCRC )
			&& Seek ( pMetadata, 0 );
		std::vector<uint8_t> dCopy ( 64*1024 );
		while ( bOK )
		{
			const size_t uRead = fread ( dCopy.data(), 1, dCopy.size(), pMetadata );
			if ( uRead )
				bOK = Write ( pOutput, dCopy.data(), uRead, &uCRC );
			if ( uRead<dCopy.size() )
			{
				bOK = bOK && feof(pMetadata);
				break;
			}
		}

		std::vector<uint8_t> dPacked ( size_t(m_uGroupRows)*sizeof(uint32_t) );
		for ( uint32_t iField=0; bOK && iField<m_uFields; ++iField )
		{
			FILE * pField = m_dFields[iField];
			bOK = Seek ( pField, 0 );
			for ( uint32_t iGroup=0; bOK && iGroup<uGroups; ++iGroup )
			{
				const uint32_t uCount = std::min ( m_uGroupRows, m_uRows-iGroup*m_uGroupRows );
				bOK = fread(dValues.data(),sizeof(uint32_t),uCount,pField)==uCount;
				uint32_t uMax = 0;
				for ( uint32_t i=0; i<uCount; ++i )
					uMax = std::max ( uMax, dValues[i] );
				const unsigned uWidth = WidthFor(uMax);
				for ( uint32_t i=0; i<uCount; ++i )
					for ( unsigned j=0; j<uWidth; ++j )
						dPacked[size_t(i)*uWidth+j] = uint8_t ( dValues[i] >> ( 8*j ) );
				bOK = bOK && Write ( pOutput, dPacked.data(), size_t(uCount)*uWidth, &uCRC );
			}
		}

		Put32 ( dHeader, 56, ~uCRC );
		bOK = bOK && fflush(pOutput)==0 && Seek(pOutput,0) && Write(pOutput,dHeader.data(),dHeader.size(),nullptr) && fflush(pOutput)==0;
		bOK = fclose(pOutput)==0 && bOK;
		fclose ( pMetadata );
		if ( !bOK )
		{
			std::remove ( szOutput );
			return Fail ( sError, "output write failed" );
		}
		return true;
	}

private:
	struct GroupMeta_t
	{
		uint64_t m_uSum = 0;
		uint32_t m_uCount = 0;
		uint32_t m_uMax = 0;
		uint32_t m_uNonzero = 0;
		uint8_t m_uWidth = 0;
	};

	bool FinishPacked ( const char * szOutput, std::string & sError )
	{
		for ( FILE * pFile : m_dFields )
			if ( fflush(pFile) || !Seek(pFile,0) )
				return Fail ( sError, "packed staging flush failed" );
		const uint32_t uGroups = m_uRows ? ( m_uRows+m_uGroupRows-1 )/m_uGroupRows : 0;
		const uint64_t uDirectory = HEADER_SIZE+uint64_t(m_uFields)*FIELD_ENTRY_SIZE;
		const uint64_t uPayload = uDirectory+uint64_t(m_uFields)*uGroups*GROUP_ENTRY_SIZE;
		uint64_t uOffset = uPayload;
		std::vector<uint8_t> dFieldEntries ( size_t(m_uFields)*FIELD_ENTRY_SIZE, 0 );
		std::vector<uint8_t> dGroupEntries ( size_t(m_uFields)*uGroups*GROUP_ENTRY_SIZE, 0 );
		for ( uint32_t iField=0; iField<m_uFields; ++iField )
		{
			if ( m_dGroupMeta[iField].size()!=uGroups )
				return Fail ( sError, "packed staging group mismatch" );
			uint64_t uFieldSum = 0;
			uint32_t uFieldNonzero = 0;
			for ( uint32_t iGroup=0; iGroup<uGroups; ++iGroup )
			{
				const GroupMeta_t & tMeta = m_dGroupMeta[iField][iGroup];
				const size_t uEntry = ( size_t(iField)*uGroups+iGroup )*GROUP_ENTRY_SIZE;
				Put64 ( dGroupEntries, uEntry, uOffset );
				Put64 ( dGroupEntries, uEntry+8, tMeta.m_uSum );
				Put32 ( dGroupEntries, uEntry+16, tMeta.m_uCount );
				Put32 ( dGroupEntries, uEntry+20, tMeta.m_uMax );
				Put32 ( dGroupEntries, uEntry+24, tMeta.m_uNonzero );
				dGroupEntries[uEntry+28] = tMeta.m_uWidth;
				uOffset += uint64_t(tMeta.m_uCount)*tMeta.m_uWidth;
				uFieldSum += tMeta.m_uSum;
				uFieldNonzero += tMeta.m_uNonzero;
			}
			const size_t uFieldEntry = size_t(iField)*FIELD_ENTRY_SIZE;
			Put64 ( dFieldEntries, uFieldEntry, uFieldSum );
			Put32 ( dFieldEntries, uFieldEntry+8, uFieldNonzero );
		}

		FILE * pOutput = fopen ( szOutput, "wb" );
		if ( !pOutput )
			return FailSystem ( sError, "unable to open output", szOutput, errno );
		std::vector<uint8_t> dHeader ( HEADER_SIZE, 0 );
		memcpy ( dHeader.data(), "E1NORM01", 8 );
		Put32 ( dHeader, 8, VERSION ); Put32 ( dHeader, 12, HEADER_SIZE );
		Put32 ( dHeader, 16, m_uRows ); Put32 ( dHeader, 20, m_uFields );
		Put32 ( dHeader, 24, m_uGroupRows ); Put32 ( dHeader, 28, uGroups );
		Put64 ( dHeader, 32, uDirectory ); Put64 ( dHeader, 40, uPayload ); Put64 ( dHeader, 48, uOffset );
		uint32_t uCRC = ~0u;
		bool bOK = Write ( pOutput, dHeader.data(), dHeader.size(), nullptr )
			&& Write ( pOutput, dFieldEntries.data(), dFieldEntries.size(), &uCRC )
			&& Write ( pOutput, dGroupEntries.data(), dGroupEntries.size(), &uCRC );
		std::vector<uint8_t> dCopy ( 64*1024 );
		for ( uint32_t iField=0; bOK && iField<m_uFields; ++iField )
			while ( bOK )
			{
				const size_t uRead = fread ( dCopy.data(), 1, dCopy.size(), m_dFields[iField] );
				if ( uRead ) bOK = Write ( pOutput, dCopy.data(), uRead, &uCRC );
				if ( uRead<dCopy.size() ) { bOK = bOK && feof(m_dFields[iField]); break; }
			}
		Put32 ( dHeader, 56, ~uCRC );
		bOK = bOK && fflush(pOutput)==0 && Seek(pOutput,0) && Write(pOutput,dHeader.data(),dHeader.size(),nullptr) && fflush(pOutput)==0;
		bOK = fclose(pOutput)==0 && bOK;
		if ( !bOK ) { std::remove(szOutput); return Fail(sError,"output write failed"); }
		return true;
	}

	bool FlushStaging ( std::string & sError )
	{
		if ( !m_uFields || m_dStaging[0].empty() )
			return true;
		const size_t uRows = m_dStaging[0].size();
		for ( uint32_t i=0; i<m_uFields; ++i )
		{
			if ( m_dStaging[i].size()!=uRows )
				return Fail ( sError, "staging row mismatch" );
			if ( m_bMutable )
			{
				if ( fwrite(m_dStaging[i].data(),sizeof(uint32_t),uRows,m_dFields[i])!=uRows )
					return Fail ( sError, "staging write failed" );
			}
			else
			{
				GroupMeta_t tMeta;
				tMeta.m_uCount = uint32_t(uRows);
				for ( uint32_t uValue : m_dStaging[i] )
				{
					tMeta.m_uSum += uValue;
					tMeta.m_uMax = std::max ( tMeta.m_uMax, uValue );
					tMeta.m_uNonzero += uValue!=0;
				}
				tMeta.m_uWidth = uint8_t(WidthFor(tMeta.m_uMax));
				m_dPackedScratch.resize ( uRows*tMeta.m_uWidth );
				for ( size_t uRow=0; uRow<uRows; ++uRow )
					for ( unsigned uByte=0; uByte<tMeta.m_uWidth; ++uByte )
						m_dPackedScratch[uRow*tMeta.m_uWidth+uByte] = uint8_t(m_dStaging[i][uRow]>>(8*uByte));
				if ( fwrite(m_dPackedScratch.data(),1,m_dPackedScratch.size(),m_dFields[i])!=m_dPackedScratch.size() )
					return Fail ( sError, "packed staging write failed" );
				m_dGroupMeta[i].push_back ( tMeta );
			}
			m_dStaging[i].clear();
		}
		return true;
	}

	static bool Fail ( std::string & sError, const char * szError ) { sError = std::string("norms: ")+szError; return false; }
	static bool FailSystem ( std::string & sError, const char * szAction, const char * szPath, int iError )
	{
		sError = std::string("norms: ")+szAction+" '"+szPath+"': "+std::strerror(iError);
		return false;
	}
	static bool CloseMetadata ( FILE * pFile, std::string & sError, const char * szError ) { fclose(pFile); return Fail(sError,szError); }
	static bool Seek ( FILE * pFile, uint64_t uOffset )
	{
#if defined(_WIN32)
		return _fseeki64 ( pFile, static_cast<__int64>(uOffset), SEEK_SET )==0;
#else
		return fseeko ( pFile, static_cast<off_t>(uOffset), SEEK_SET )==0;
#endif
	}
	static bool Write ( FILE * pFile, const void * pData, size_t uSize, uint32_t * pCRC )
	{
		if ( uSize && fwrite(pData,1,uSize,pFile)!=uSize )
			return false;
		if ( pCRC )
			*pCRC = e1::CRCUpdate ( *pCRC, static_cast<const uint8_t *>(pData), uSize );
		return true;
	}

	uint32_t m_uFields = 0;
	uint32_t m_uGroupRows = 0;
	uint32_t m_uRows = 0;
	bool m_bValid = false;
	bool m_bMutable = false;
	std::vector<FILE *> m_dFields;
	std::vector<std::vector<uint32_t>> m_dStaging;
	std::vector<std::vector<GroupMeta_t>> m_dGroupMeta;
	std::vector<uint8_t> m_dPackedScratch;
};

class Store final : public FieldNormReader_i
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
		if ( !uGroupRows || e1::U64(pData+48)!=uSize || uGroups!=( uRows ? (uRows+uGroupRows-1)/uGroupRows : 0 ) )
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
				uint64_t uDecodedSum = 0;
				uint32_t uDecodedMax = 0;
				uint32_t uDecodedNonzero = 0;
				for ( uint32_t i=0; i<uCount; ++i )
				{
					const uint8_t * pValue = pData+uOffset+uint64_t(i)*uWidth;
					const uint32_t uValue = uWidth==1 ? pValue[0] : uWidth==2 ? uint32_t(pValue[0])|(uint32_t(pValue[1])<<8) : e1::U32(pValue);
					uDecodedSum += uValue;
					uDecodedMax = std::max ( uDecodedMax, uValue );
					uDecodedNonzero += uValue!=0;
				}
				if ( uDecodedSum!=uSum || uDecodedMax!=uMax || uDecodedNonzero!=uNonzero )
					return fnFail ( "group metadata mismatch" );
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
		BuildTotalCache();
		return true;
	}

	uint32_t Rows() const override { return m_uRows; }
	uint32_t Fields() const override { return m_uFields; }
	uint64_t Sum ( uint32_t uField ) const override { return uField<m_uFields ? e1::U64(m_pData+HEADER_SIZE+uint64_t(uField)*FIELD_ENTRY_SIZE) : 0; }
	uint32_t Nonzero ( uint32_t uField ) const { return uField<m_uFields ? e1::U32(m_pData+HEADER_SIZE+uint64_t(uField)*FIELD_ENTRY_SIZE+8) : 0; }

	bool Get ( uint32_t uField, uint32_t uRow, uint32_t & uValue ) const override
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

	bool ReadRange ( uint32_t uField, uint32_t uFirst, uint32_t uCount, uint32_t * pOut ) const override
	{
		if ( !pOut || uFirst>m_uRows || uCount>m_uRows-uFirst )
			return false;
		for ( uint32_t i=0; i<uCount; ++i )
			if ( !Get(uField,uFirst+i,pOut[i]) )
				return false;
		return true;
	}

	bool Gather128 ( uint32_t uField, const uint32_t * pRows, uint32_t * pOut ) const override
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

	bool GatherTotal ( const uint32_t * pRows, uint32_t uCount, uint32_t * pOut ) const override
	{
		if ( !pRows || !pOut || !m_uTotalWidth )
			return false;
		if ( m_uTotalWidth==1 )
		{
			for ( uint32_t i=0; i<uCount; ++i )
			{
				if ( pRows[i]>=m_uRows )
					return false;
				pOut[i] = m_dTotal8[pRows[i]];
			}
		} else if ( m_uTotalWidth==2 )
		{
			for ( uint32_t i=0; i<uCount; ++i )
			{
				if ( pRows[i]>=m_uRows )
					return false;
				pOut[i] = m_dTotal16[pRows[i]];
			}
		} else
		{
			for ( uint32_t i=0; i<uCount; ++i )
			{
				if ( pRows[i]>=m_uRows )
					return false;
				pOut[i] = m_dTotal32[pRows[i]];
			}
		}
		return true;
	}
	uint64_t TotalCacheBytes() const { return uint64_t(m_uRows)*m_uTotalWidth; }

private:
	void BuildTotalCache()
	{
		uint32_t uMax = 0;
		for ( uint32_t uRow=0; uRow<m_uRows; ++uRow )
		{
			uint64_t uTotal = 0;
			for ( uint32_t iField=0; iField<m_uFields; ++iField )
			{
				uint32_t uValue = 0;
				Get ( iField, uRow, uValue );
				uTotal += uValue;
			}
			if ( uTotal>UINT32_MAX )
				return;
			uMax = std::max ( uMax, uint32_t(uTotal) );
		}
		m_uTotalWidth = WidthFor ( uMax );
		if ( m_uTotalWidth==1 ) m_dTotal8.resize ( m_uRows );
		else if ( m_uTotalWidth==2 ) m_dTotal16.resize ( m_uRows );
		else m_dTotal32.resize ( m_uRows );
		for ( uint32_t uRow=0; uRow<m_uRows; ++uRow )
		{
			uint32_t uTotal = 0;
			for ( uint32_t iField=0; iField<m_uFields; ++iField )
			{
				uint32_t uValue = 0;
				Get ( iField, uRow, uValue );
				uTotal += uValue;
			}
			if ( m_uTotalWidth==1 ) m_dTotal8[uRow] = uint8_t(uTotal);
			else if ( m_uTotalWidth==2 ) m_dTotal16[uRow] = uint16_t(uTotal);
			else m_dTotal32[uRow] = uTotal;
		}
	}
	const uint8_t * Entry ( uint32_t uField, uint32_t uGroup ) const { return m_pData+m_uDirectory+( uint64_t(uField)*m_uGroups+uGroup )*GROUP_ENTRY_SIZE; }
	void Reset() { m_pData=nullptr; m_uRows=m_uFields=m_uGroupRows=m_uGroups=m_uTotalWidth=0; m_uDirectory=0; m_dTotal8.clear(); m_dTotal16.clear(); m_dTotal32.clear(); }

	const uint8_t * m_pData = nullptr;
	uint32_t m_uRows = 0;
	uint32_t m_uFields = 0;
	uint32_t m_uGroupRows = 0;
	uint32_t m_uGroups = 0;
	uint32_t m_uTotalWidth = 0;
	uint64_t m_uDirectory = 0;
	std::vector<uint8_t> m_dTotal8;
	std::vector<uint16_t> m_dTotal16;
	std::vector<uint32_t> m_dTotal32;
};

} // namespace e1::norms
