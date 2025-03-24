//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "sphinx.h"
#include "sphinxstd.h"
#include "searchdaemon.h"
#include "accumulator.h"

#include "serialize.h"

void SaveAttrUpdate ( const CSphAttrUpdate & tUpd, MemoryWriter_c & tWriter )
{
	tWriter.PutByte ( tUpd.m_bIgnoreNonexistent );
	tWriter.PutByte ( tUpd.m_bStrict );

	tWriter.PutDword ( tUpd.m_dAttributes.GetLength() );
	for ( const TypedAttribute_t& tElem : tUpd.m_dAttributes )
	{
		tWriter.PutString ( tElem.m_sName );
		tWriter.PutDword ( tElem.m_eType );
	}

	SaveArray ( tUpd.m_dDocids, tWriter );
	SaveArray ( tUpd.m_dRowOffset, tWriter );
	SaveArray ( tUpd.m_dPool, tWriter );
	SaveArray ( tUpd.m_dBlobs, tWriter );
}

int LoadAttrUpdate ( const BYTE * pBuf, int iLen, CSphAttrUpdate & tUpd, bool & bBlob )
{
	MemoryReader_c tIn ( pBuf, iLen );

	tUpd.m_bIgnoreNonexistent = !!tIn.GetVal<BYTE>();
	tUpd.m_bStrict = !!tIn.GetVal<BYTE>();

	bBlob = false;
	tUpd.m_dAttributes.Resize ( tIn.GetDword() );
	for ( TypedAttribute_t& tElem : tUpd.m_dAttributes )
	{
		tElem.m_sName = tIn.GetString();
		tElem.m_eType = (ESphAttr)tIn.GetDword();
		bBlob |= ( tElem.m_eType == SPH_ATTR_UINT32SET || tElem.m_eType == SPH_ATTR_INT64SET || tElem.m_eType == SPH_ATTR_JSON || tElem.m_eType == SPH_ATTR_STRING );
	}

	GetArray ( tUpd.m_dDocids, tIn );
	GetArray ( tUpd.m_dRowOffset, tIn );
	GetArray ( tUpd.m_dPool, tIn );
	GetArray ( tUpd.m_dBlobs, tIn );
	return tIn.GetPos();
}

enum
{
	FILTER_FLAG_EXCLUDE = 1UL << 0,
	FILTER_FLAG_HAS_EQUAL_MIN = 1UL << 1,
	FILTER_FLAG_HAS_EQUAL_MAX = 1UL << 2,
	FILTER_FLAG_OPEN_LEFT = 1UL << 3,
	FILTER_FLAG_OPEN_RIGHT = 1UL << 4,
	FILTER_FLAG_IS_NULL = 1UL << 5
};


namespace {

void SaveFilter ( const CSphFilterSettings & tItem, MemoryWriter_c & tWriter )
{
	tWriter.PutString ( tItem.m_sAttrName );

	DWORD uFlags = 0;
	uFlags |= FILTER_FLAG_EXCLUDE * tItem.m_bExclude;
	uFlags |= FILTER_FLAG_HAS_EQUAL_MIN * tItem.m_bHasEqualMin;
	uFlags |= FILTER_FLAG_HAS_EQUAL_MAX * tItem.m_bHasEqualMax;
	uFlags |= FILTER_FLAG_OPEN_LEFT * tItem.m_bOpenLeft;
	uFlags |= FILTER_FLAG_OPEN_RIGHT * tItem.m_bOpenRight;
	uFlags |= FILTER_FLAG_IS_NULL * tItem.m_bIsNull;
	tWriter.PutDword ( uFlags );

	tWriter.PutByte ( tItem.m_eType );
	tWriter.PutByte ( tItem.m_eMvaFunc );
	tWriter.PutUint64 ( tItem.m_iMinValue );
	tWriter.PutUint64 ( tItem.m_iMaxValue );
	SaveArray ( tItem.GetValues(), tWriter );
	SaveArray ( tItem.m_dStrings, tWriter );
	tWriter.PutDword ( 0 ); // legacy N of external values, now always 0
}

void LoadFilter ( CSphFilterSettings & tItem, MemoryReader_c & tReader )
{
	tItem.m_sAttrName = tReader.GetString();

	DWORD uFlags = tReader.GetDword();
	tItem.m_bExclude = !!( uFlags & FILTER_FLAG_EXCLUDE );
	tItem.m_bHasEqualMin = !!( uFlags & FILTER_FLAG_HAS_EQUAL_MIN );
	tItem.m_bHasEqualMax = !!( uFlags & FILTER_FLAG_HAS_EQUAL_MAX );
	tItem.m_bOpenLeft = !!( uFlags & FILTER_FLAG_OPEN_LEFT );
	tItem.m_bOpenRight = !!( uFlags & FILTER_FLAG_OPEN_RIGHT );
	tItem.m_bIsNull = !!( uFlags & FILTER_FLAG_IS_NULL );

	tItem.m_eType = (ESphFilter)tReader.GetVal<BYTE>();
	tItem.m_eMvaFunc = (ESphMvaFunc)tReader.GetVal<BYTE>();
	tItem.m_iMinValue = tReader.GetVal<uint64_t>();
	tItem.m_iMaxValue = tReader.GetVal<uint64_t>();
	GetArray ( tItem.m_dValues, tReader );
	GetArray ( tItem.m_dStrings, tReader );

	CSphVector<SphAttr_t> dOtherValues;
	GetArray ( dOtherValues, tReader ); // expected to be empty

	// legacy pass - extra values, just push them as plain values here.
	if ( !dOtherValues.IsEmpty() )
		std::swap ( tItem.m_dValues, dOtherValues );
}
}

void SaveUpdate ( const CSphQuery & tQuery, MemoryWriter_c & tWriter )
{
	tWriter.PutString ( tQuery.m_sQuery );
	tWriter.PutDword ( tQuery.m_dFilters.GetLength ());
	for ( const CSphFilterSettings & tItem : tQuery.m_dFilters )
		::SaveFilter ( tItem, tWriter );

	SaveArray ( tQuery.m_dFilterTree, tWriter );
}

int LoadUpdate ( const BYTE * pBuf, int iLen, CSphQuery & tQuery )
{
	MemoryReader_c tReader ( pBuf, iLen );

	tQuery.m_sQuery = tReader.GetString ();
	tQuery.m_dFilters.Resize ( tReader.GetDword ());
	for ( CSphFilterSettings & tItem : tQuery.m_dFilters )
		::LoadFilter ( tItem, tReader );

	GetArray ( tQuery.m_dFilterTree, tReader );

	return tReader.GetPos ();
}

// commands version (commands these got replicated via Galera)
// ver 0x104 added docstore from RT index
// ver 0x105 fixed CSphWordHit serialization - instead of direct raw blob copy only fields sent (16 bytes vs 24)
// ver 0x106 add total indexed bytes to accum
// ver 0x107 add blobs vector to replicate update statement
// ver 0x108 gtid is sent and parsed as blob (was string)
// ver 0x109 indexes support for ALTER ADD \ DROP table
static constexpr WORD VER_COMMAND_REPLICATE = 0x109;
bool LoadCmdHeader( MemoryReader_c& tReader, ReplicationCommand_t* pCmd )
{
	TlsMsg::ResetErr();
	auto eCommand = (ReplCmd_e) tReader.GetVal<WORD> ();
	if ( eCommand<ReplCmd_e::PQUERY_ADD || eCommand>ReplCmd_e::TOTAL )
		return TlsMsg::Err ( "bad replication command %d", (int) eCommand );

	pCmd->m_uVersion = tReader.GetVal<WORD> ();
	if ( pCmd->m_uVersion>VER_COMMAND_REPLICATE )
		return TlsMsg::Err ( "replication command %d, version mismatch %d, got %d", (int) eCommand, VER_COMMAND_REPLICATE, (int)pCmd->m_uVersion );

	pCmd->m_eCommand = eCommand;
	pCmd->m_sIndex = tReader.GetString ();
	return true;
}

void SaveCmdHeader ( const ReplicationCommand_t & tCmd, MemoryWriter_c & tWriter )
{
	tWriter.PutWord ((WORD) tCmd.m_eCommand );
	tWriter.PutWord ( VER_COMMAND_REPLICATE );
	tWriter.PutString ( tCmd.m_sIndex );
}

WORD GetVerCommandReplicate()
{
	return VER_COMMAND_REPLICATE;
}