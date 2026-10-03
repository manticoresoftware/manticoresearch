//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "attribute.h"
#include "sphinxrt.h"
#include "sphinxsort.h"
#include "fastcount.h"
#include "searchdaemon.h"
#include "binlog.h"
#include "accumulator.h"
#include "sphinxudf.h"
#include "sphinxquery/xqparser.h"
#include "indexfiles.h"
#include "memio.h"
#include "searchnode.h"

#include <gmock/gmock.h>

#include <limits>


//////////////////////////////////////////////////////////////////////////

namespace
{

struct RtTrxPayload_t
{
	CSphVector<BYTE> m_dData;
	int m_iNormCountOffset = -1;
};

RtTrxPayload_t MakeRtTrxPayload ( DWORD uDocs, DWORD uNorms, int iNormValues=0, DWORD uVersion=0x10C )
{
	RtTrxPayload_t tPayload;
	MemoryWriter_c tWriter ( tPayload.m_dData );
	tWriter.PutByte ( 0 );
	tWriter.PutDword ( uDocs );
	if ( uVersion>=0x106 )
		tWriter.PutVal<int64_t> ( 0 );
	tWriter.PutDword ( 0 ); // hits
	tWriter.PutDword ( 0 ); // rows
	if ( uVersion>=0x10C )
	{
		tPayload.m_iNormCountOffset = tWriter.GetPos();
		tWriter.PutDword ( uNorms );
		for ( int i=0; i<iNormValues; ++i )
			tWriter.PutDword ( i+1 );
	}
	tWriter.PutDword ( 0 ); // blobs
	tWriter.PutDword ( 0 ); // per-document hit counts
	tWriter.PutDword ( 0 ); // packed keywords
	tWriter.PutByte ( 0 ); // docstore
	tWriter.PutByte ( 0 ); // columnar storage
	tWriter.PutDword ( 0 ); // kill list
	return tPayload;
}

ByteBlob_t AsBlob ( const CSphVector<BYTE> & dData )
{
	return { dData.Begin(), dData.GetLength() };
}

CSphSchema MakeRtTrxSchema ( int iFields )
{
	CSphSchema tSchema;
	for ( int i=0; i<iFields; ++i )
	{
		CSphColumnInfo tField;
		tField.m_sName.SetSprintf ( "field%d", i );
		tField.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
		tSchema.AddField ( tField );
	}
	return tSchema;
}

struct CompleteRtTrxPayload_t
{
	CSphVector<BYTE> m_dData;
	CSphSchema m_tSchema;
	int m_iHitsCount = 0;
	int m_iHitsData = 0;
	int m_iRowsCount = 0;
	int m_iRowsData = 0;
	int m_iNormsCount = 0;
	int m_iBlobsCount = 0;
	int m_iBlobsData = 0;
	int m_iPerDocCount = 0;
	int m_iPerDocData = 0;
	int m_iPackedCount = 0;
	int m_iPackedData = 0;
	int m_iDocstoreFlag = 0;
	int m_iDocstoreFields = 0;
	int m_iDocstoreDocs = 0;
	int m_iDocstoreDocLength = 0;
	int m_iDocstoreFieldLength = 0;
	int m_iDocstoreData = 0;
	int m_iColumnarFlag = 0;
	int m_iColumnarCount = 0;
	CSphVector<int> m_dColumnarTypes;
	CSphVector<int> m_dColumnarValueCounts;
	CSphVector<int> m_dColumnarValueData;
	int m_iStringOffsetsData = -1;
	int m_iMvaOffsetsData = -1;
	int m_iKillCount = 0;
	int m_iKillData = 0;
};

CompleteRtTrxPayload_t MakeCompleteRtTrxPayload()
{
	CompleteRtTrxPayload_t tPayload;
	CSphSchema tSchema;
	CSphColumnInfo tField ( "body" );
	tField.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED;
	tSchema.AddField ( tField );
	CSphColumnInfo tStoredOnly ( "stored_only" );
	tStoredOnly.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
	tSchema.AddField ( tStoredOnly );
	auto fnAddAttr = [&] ( const char * szName, ESphAttr eType, bool bColumnar )
	{
		CSphColumnInfo tAttr ( szName, eType );
		if ( bColumnar )
			tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR;
		tSchema.AddAttr ( tAttr, true );
	};
	fnAddAttr ( sphGetDocidName(), SPH_ATTR_BIGINT, true );
	fnAddAttr ( sphGetBlobLocatorName(), SPH_ATTR_BIGINT, false );
	fnAddAttr ( "row_value", SPH_ATTR_INTEGER, false );
	fnAddAttr ( "payload", SPH_ATTR_STRING, false );
	fnAddAttr ( "flag", SPH_ATTR_BOOL, true );
	fnAddAttr ( "tag", SPH_ATTR_STRING, true );
	fnAddAttr ( "numbers", SPH_ATTR_UINT32SET, true );

	auto pIndex = sphCreateIndexRT ( "rt_trx_fixture", "rt_trx_fixture", tSchema, 1024*1024 );
	EXPECT_TRUE ( pIndex );
	tPayload.m_tSchema = pIndex->GetMatchSchema();

	RtAccum_t tSaved;
	tSaved.SetIndex ( pIndex.get() );
	InsertDocData_c tDoc ( tPayload.m_tSchema );
	tDoc.SetID ( 47 );
	const CSphColumnInfo * pRowAttr = tPayload.m_tSchema.GetAttr ( "row_value" );
	EXPECT_NE ( pRowAttr, nullptr );
	tDoc.m_tDoc.SetAttr ( pRowAttr->m_tLocator, 11 );
	EXPECT_GE ( tDoc.m_dColumnarAttrs.GetLength(), 2 );
	tDoc.m_dColumnarAttrs[1] = 1;
	tDoc.m_dStrings.Add ( "row-blob" );
	tDoc.m_dStrings.Add ( "abc" );
	tDoc.AddMVALength ( 2 );
	tDoc.AddMVAValue ( 7 );
	tDoc.AddMVAValue ( 9 );
	BYTE dStored[] = { 'd', 'o', 'c' };
	DocstoreBuilder_i::Doc_t tStored;
	tStored.m_dFields.Add ( { dStored, 3 } );
	tStored.m_dFields.Add ( { nullptr, 0 } );
	const DWORD dFieldLengths[] = { 13, 0 };
	tSaved.AddDocument ( nullptr, tDoc, true, tPayload.m_tSchema.GetRowSize(), &tStored, dFieldLengths );
	tSaved.m_iAccumBytes = 17;
	auto & tHit = tSaved.m_dAccum.Add();
	tHit.m_tRowID = 0;
	tHit.m_uWordID = 5;
	tHit.m_uWordPos = 7;
	tSaved.m_dPerDocHitsCount[0] = 1;

	MemoryWriter_c tWriter ( tPayload.m_dData );
	EXPECT_TRUE ( tSaved.SaveRtTrx ( tWriter ) ) << TlsMsg::szError();

	MemoryReader_c tReader ( AsBlob(tPayload.m_dData) );
	(void)tReader.GetVal<BYTE>();
	(void)tReader.GetDword();
	(void)tReader.GetVal<int64_t>();
	tPayload.m_iHitsCount = tReader.GetPos();
	DWORD uCount = tReader.GetDword();
	tPayload.m_iHitsData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount*( sizeof(RowID_t)+sizeof(SphWordID_t)+sizeof(DWORD) ) );
	tPayload.m_iRowsCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tPayload.m_iRowsData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount*sizeof(CSphRowitem) );
	tPayload.m_iNormsCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tReader.SetPos ( tReader.GetPos()+uCount*sizeof(DWORD) );
	tPayload.m_iBlobsCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tPayload.m_iBlobsData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount );
	tPayload.m_iPerDocCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tPayload.m_iPerDocData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount*sizeof(DWORD) );
	tPayload.m_iPackedCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tPayload.m_iPackedData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount );
	tPayload.m_iDocstoreFlag = tReader.GetPos();
	EXPECT_EQ ( tReader.GetVal<BYTE>(), 1 );
	tPayload.m_iDocstoreFields = tReader.GetPos();
	DWORD uFields = tReader.GetDword();
	tPayload.m_iDocstoreDocs = tReader.GetPos();
	EXPECT_EQ ( tReader.UnzipInt(), 1u );
	tPayload.m_iDocstoreDocLength = tReader.GetPos();
	DWORD uDocLength = tReader.UnzipInt();
	tPayload.m_iDocstoreFieldLength = tReader.GetPos();
	const BYTE * pPacked = tPayload.m_dData.Begin()+tReader.GetPos();
	for ( DWORD i=0; i<uFields; ++i )
	{
		DWORD uLength = UnzipIntBE ( pPacked );
		pPacked += uLength;
	}
	tPayload.m_iDocstoreData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uDocLength );
	tPayload.m_iColumnarFlag = tReader.GetPos();
	EXPECT_EQ ( tReader.GetVal<BYTE>(), 1 );
	tPayload.m_iColumnarCount = tReader.GetPos();
	uCount = tReader.GetDword();
	for ( DWORD i=0; i<uCount; ++i )
	{
		tPayload.m_dColumnarTypes.Add ( tReader.GetPos() );
		ESphAttr eType = ESphAttr(tReader.GetDword());
		if ( eType==SPH_ATTR_INTEGER || eType==SPH_ATTR_TIMESTAMP || eType==SPH_ATTR_FLOAT || eType==SPH_ATTR_TOKENCOUNT || eType==SPH_ATTR_BOOL || eType==SPH_ATTR_BIGINT )
			tReader.SetPos ( tReader.GetPos()+sizeof(SphOffset_t) );
		tPayload.m_dColumnarValueCounts.Add ( tReader.GetPos() );
		DWORD uValues = tReader.GetDword();
		tPayload.m_dColumnarValueData.Add ( tReader.GetPos() );
		switch ( eType )
		{
		case SPH_ATTR_BOOL: tReader.SetPos ( tReader.GetPos()+uValues ); break;
		case SPH_ATTR_BIGINT: tReader.SetPos ( tReader.GetPos()+uValues*sizeof(int64_t) ); break;
		case SPH_ATTR_STRING:
			tPayload.m_iStringOffsetsData = tReader.GetPos();
			tReader.SetPos ( tReader.GetPos()+uValues*sizeof(int64_t) );
			tPayload.m_dColumnarValueCounts.Last() = tReader.GetPos();
			uValues = tReader.GetDword();
			tReader.SetPos ( tReader.GetPos()+uValues );
			break;
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_FLOAT_VECTOR:
		case SPH_ATTR_FLOAT_VECTOR_ARRAY:
		case SPH_ATTR_INT64SET:
			tPayload.m_iMvaOffsetsData = tReader.GetPos();
			tReader.SetPos ( tReader.GetPos()+uValues*sizeof(int) );
			tPayload.m_dColumnarValueCounts.Last() = tReader.GetPos();
			uValues = tReader.GetDword();
			tReader.SetPos ( tReader.GetPos()+uValues*( eType==SPH_ATTR_INT64SET ? sizeof(int64_t) : sizeof(DWORD) ) );
			break;
		default: tReader.SetPos ( tReader.GetPos()+uValues*sizeof(DWORD) ); break;
		}
	}
	tPayload.m_iKillCount = tReader.GetPos();
	uCount = tReader.GetDword();
	tPayload.m_iKillData = tReader.GetPos();
	tReader.SetPos ( tReader.GetPos()+uCount*sizeof(DocID_t) );
	EXPECT_EQ ( tReader.GetPos(), tPayload.m_dData.GetLength() );
	return tPayload;
}

void SetPayloadDword ( CSphVector<BYTE> & dData, int iOffset, DWORD uValue )
{
	ASSERT_GE ( iOffset, 0 );
	ASSERT_LE ( iOffset+(int)sizeof(uValue), dData.GetLength() );
	memcpy ( dData.Begin()+iOffset, &uValue, sizeof(uValue) );
}

template<typename T>
void SetPayloadValue ( CSphVector<BYTE> & dData, int iOffset, T tValue )
{
	ASSERT_GE ( iOffset, 0 );
	ASSERT_LE ( iOffset+(int)sizeof(tValue), dData.GetLength() );
	memcpy ( dData.Begin()+iOffset, &tValue, sizeof(tValue) );
}

CSphVector<BYTE> WithPackedKeywords ( const CompleteRtTrxPayload_t & tPayload, const BYTE * pPacked, int iPacked, SphWordID_t uOffset )
{
	CSphVector<BYTE> dResult;
	dResult.Append ( tPayload.m_dData.Begin(), tPayload.m_iPackedData );
	dResult.Append ( pPacked, iPacked );
	dResult.Append ( tPayload.m_dData.Begin()+tPayload.m_iPackedData, tPayload.m_dData.GetLength()-tPayload.m_iPackedData );
	SetPayloadDword ( dResult, tPayload.m_iPackedCount, iPacked );
	SetPayloadValue ( dResult, tPayload.m_iHitsData+sizeof(RowID_t), uOffset );
	return dResult;
}

} // namespace

TEST ( RtTrx, SchemaLessSaveRoundTripsEmptyNorms )
{
	RtAccum_t tSaved;
	tSaved.m_dBlobs.Add ( 17 );
	tSaved.m_dAccumKlist.Add ( 29 );
	CSphVector<BYTE> dPayload;
	MemoryWriter_c tWriter ( dPayload );
	ASSERT_TRUE ( tSaved.SaveRtTrx ( tWriter ) ) << TlsMsg::szError();

	MemoryReader_c tReader ( AsBlob(dPayload) );
	(void)tReader.GetVal<BYTE>();
	(void)tReader.GetDword();
	(void)tReader.GetVal<int64_t>();
	EXPECT_EQ ( tReader.GetDword(), 0u ); // hits
	EXPECT_EQ ( tReader.GetDword(), 0u ); // rows
	EXPECT_EQ ( tReader.GetDword(), 0u ); // norms

	RtAccum_t tLoaded;
	ASSERT_TRUE ( tLoaded.LoadRtTrx ( AsBlob(dPayload), 0x10C ) ) << TlsMsg::szError();
	ASSERT_EQ ( tLoaded.m_dBlobs.GetLength(), 1 );
	EXPECT_EQ ( tLoaded.m_dBlobs[0], 17 );
	ASSERT_EQ ( tLoaded.m_dAccumKlist.GetLength(), 1 );
	EXPECT_EQ ( tLoaded.m_dAccumKlist[0], 29 );
	EXPECT_TRUE ( tLoaded.m_dNorms.IsEmpty() );
}

TEST ( RtAccum, ReusesFallbackFieldLengthsWithoutDocumentBleed )
{
	CSphSchema tSchema;
	CSphColumnInfo tTitle ( "title" );
	tTitle.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED;
	tSchema.AddField ( tTitle );
	CSphColumnInfo tStoredOnly ( "stored_only" );
	tStoredOnly.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
	tSchema.AddField ( tStoredOnly );
	CSphColumnInfo tBody ( "body" );
	tBody.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
	tSchema.AddField ( tBody );
	tSchema.AddAttr ( CSphColumnInfo ( sphGetDocidName(), SPH_ATTR_BIGINT ), true );
	auto pIndex = sphCreateIndexRT ( "rt_field_length_scratch", "rt_field_length_scratch", tSchema, 1024*1024 );
	ASSERT_TRUE ( pIndex );

	RtAccum_t tAccum;
	tAccum.SetIndex ( pIndex.get() );
	auto fnAdd = [&] ( DocID_t tID, std::initializer_list<std::tuple<int,int,SphWordID_t>> dPositions, const DWORD * pExactFieldLengths=nullptr )
	{
		InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
		tDoc.SetID ( tID );
		ISphHits dHits;
		RowID_t tRowID = tAccum.GenerateRowID();
		for ( const auto & [ iField, iPos, uWordID ] : dPositions )
		{
			CSphWordHit & tHit = dHits.Add();
			tHit.m_tRowID = tRowID;
			tHit.m_uWordID = uWordID;
			tHit.m_uWordPos = HITMAN::Create ( iField, iPos, false );
		}
		tAccum.AddDocument ( dHits.IsEmpty() ? nullptr : &dHits, tDoc, false, pIndex->GetMatchSchema().GetRowSize(), nullptr, pExactFieldLengths );
	};

	fnAdd ( 1, { {0,1,1}, {0,1,1}, {0,1,2}, {0,3,3}, {2,1,4} } );
	fnAdd ( 2, { {2,1,5}, {2,1,6}, {2,2,7} } );
	const DWORD dExact[] = { 7, 101, 11 };
	fnAdd ( 3, {}, dExact );

	ASSERT_EQ ( tAccum.m_dNorms.GetLength(), 6 );
	const DWORD dExpected[] = { 2, 1, 0, 2, 7, 11 };
	for ( int i = 0; i < tAccum.m_dNorms.GetLength(); ++i )
		EXPECT_EQ ( tAccum.m_dNorms[i], dExpected[i] ) << "norm " << i;
	ASSERT_EQ ( tAccum.m_dPerDocHitsCount.GetLength(), 3 );
	EXPECT_EQ ( tAccum.m_dPerDocHitsCount[0], 4u ); // one exact duplicate removed
	EXPECT_EQ ( tAccum.m_dPerDocHitsCount[1], 3u );
	EXPECT_EQ ( tAccum.m_dPerDocHitsCount[2], 0u );

	// A new transaction must reuse capacity but not the previous document values.
	tAccum.Cleanup();
	tAccum.SetIndex ( pIndex.get() );
	fnAdd ( 4, {} );
	ASSERT_EQ ( tAccum.m_dNorms.GetLength(), 2 );
	EXPECT_EQ ( tAccum.m_dNorms[0], 0u );
	EXPECT_EQ ( tAccum.m_dNorms[1], 0u );
}

TEST ( RtAccum, ResizesFallbackFieldLengthsAfterSchemaChangeAndHandlesNoFields )
{
	CSphSchema tWideSchema;
	for ( int i = 0; i < 4; ++i )
	{
		CSphColumnInfo tField;
		tField.m_sName.SetSprintf ( "field%d", i );
		tField.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
		tWideSchema.AddField ( tField );
	}
	tWideSchema.AddAttr ( CSphColumnInfo ( sphGetDocidName(), SPH_ATTR_BIGINT ), true );
	auto pWideIndex = sphCreateIndexRT ( "rt_field_length_wide", "rt_field_length_wide", tWideSchema, 1024*1024 );
	ASSERT_TRUE ( pWideIndex );

	RtAccum_t tAccum;
	tAccum.SetIndex ( pWideIndex.get() );
	InsertDocData_c tWideDoc ( pWideIndex->GetMatchSchema() );
	tWideDoc.SetID ( 1 );
	ISphHits dWideHits;
	dWideHits.Add() = { 1, 1, HITMAN::Create ( 3, 5, false ) };
	tAccum.AddDocument ( &dWideHits, tWideDoc, false, pWideIndex->GetMatchSchema().GetRowSize(), nullptr );

	tAccum.Cleanup();
	CSphSchema tNarrowSchema;
	CSphColumnInfo tField ( "only" );
	tField.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
	tNarrowSchema.AddField ( tField );
	tNarrowSchema.AddAttr ( CSphColumnInfo ( sphGetDocidName(), SPH_ATTR_BIGINT ), true );
	auto pNarrowIndex = sphCreateIndexRT ( "rt_field_length_narrow", "rt_field_length_narrow", tNarrowSchema, 1024*1024 );
	ASSERT_TRUE ( pNarrowIndex );
	tAccum.SetIndex ( pNarrowIndex.get() );
	InsertDocData_c tNarrowDoc ( pNarrowIndex->GetMatchSchema() );
	tNarrowDoc.SetID ( 2 );
	ISphHits dNarrowHits;
	dNarrowHits.Add() = { 1, 1, HITMAN::Create ( 0, 1, false ) };
	tAccum.AddDocument ( &dNarrowHits, tNarrowDoc, false, pNarrowIndex->GetMatchSchema().GetRowSize(), nullptr );
	ASSERT_EQ ( tAccum.m_dNorms.GetLength(), 1 );
	EXPECT_EQ ( tAccum.m_dNorms[0], 1u );

	tAccum.Cleanup();
	CSphSchema tNoFieldsSchema;
	tNoFieldsSchema.AddAttr ( CSphColumnInfo ( sphGetDocidName(), SPH_ATTR_BIGINT ), true );
	auto pNoFieldsIndex = sphCreateIndexRT ( "rt_field_length_none", "rt_field_length_none", tNoFieldsSchema, 1024*1024 );
	ASSERT_TRUE ( pNoFieldsIndex );
	tAccum.SetIndex ( pNoFieldsIndex.get() );
	InsertDocData_c tNoFieldsDoc ( pNoFieldsIndex->GetMatchSchema() );
	tNoFieldsDoc.SetID ( 3 );
	tAccum.AddDocument ( nullptr, tNoFieldsDoc, false, pNoFieldsIndex->GetMatchSchema().GetRowSize(), nullptr );
	EXPECT_TRUE ( tAccum.m_dNorms.IsEmpty() );
}

TEST ( RtTrx, RejectsUnexpectedStoragesWithoutSchema )
{
	RtTrxPayload_t tPayload = MakeRtTrxPayload ( 0, 0 );
	MemoryReader_c tReader ( AsBlob(tPayload.m_dData) );
	(void)tReader.GetVal<BYTE>();
	(void)tReader.GetDword();
	(void)tReader.GetVal<int64_t>();
	const int dElemSizes[] = {
		int(sizeof(RowID_t)+sizeof(SphWordID_t)+sizeof(DWORD)), int(sizeof(CSphRowitem)),
		int(sizeof(DWORD)), 1, int(sizeof(DWORD)), 1 };
	for ( int i=0; i<6; ++i )
	{
		DWORD uCount = tReader.GetDword();
		tReader.SetPos ( tReader.GetPos()+uCount*dElemSizes[i] );
	}
	const int iDocstoreFlag = tReader.GetPos();
	const int iColumnarFlag = iDocstoreFlag+1;

	for ( int iFlag : { iDocstoreFlag, iColumnarFlag } )
	{
		CSphVector<BYTE> dForged = tPayload.m_dData;
		dForged[iFlag] = 1;
		RtAccum_t tLoaded;
		TlsMsg::ResetErr();
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(dForged), 0x10C ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "unexpected" ), nullptr ) << TlsMsg::szError();
	}
}

TEST ( RtTrx, RejectsSchemaLessSaveWithDocumentsOrNorms )
{
	RtAccum_t tSaved;
	tSaved.m_uAccumDocs = 1;
	tSaved.m_dNorms.Add ( 3 );
	CSphVector<BYTE> dPayload;
	MemoryWriter_c tWriter ( dPayload );
	TlsMsg::ResetErr();
	EXPECT_FALSE ( tSaved.SaveRtTrx ( tWriter ) );
	EXPECT_TRUE ( dPayload.IsEmpty() );
	EXPECT_NE ( strstr ( TlsMsg::szError(), "schema-less" ), nullptr ) << TlsMsg::szError();
}

TEST ( RtTrx, RejectsMalformedSchemaNormCounts )
{
	const CSphSchema tSchema = MakeRtTrxSchema ( 2 );
	auto fnFails = [&] ( DWORD uCount, const char * szError )
	{
		TlsMsg::ResetErr();
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( 1, uCount );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10C, &tSchema ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), szError ), nullptr ) << TlsMsg::szError();
	};
	fnFails ( 0, "count mismatch" );
	fnFails ( 1, "count mismatch" );
	fnFails ( 3, "count mismatch" );

	TlsMsg::ResetErr();
	RtTrxPayload_t tTruncated = MakeRtTrxPayload ( 1, 2, 1 );
	tTruncated.m_dData.Resize ( tTruncated.m_iNormCountOffset+2*sizeof(DWORD) );
	RtAccum_t tLoaded;
	EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tTruncated.m_dData), 0x10C, &tSchema ) );
	EXPECT_NE ( strstr ( TlsMsg::szError(), "truncated" ), nullptr ) << TlsMsg::szError();
}

TEST ( RtTrx, RejectsSchemaLessRowsAndNormCountLimits )
{
	{
		TlsMsg::ResetErr();
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( 1, 0 );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10C ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "schema-less" ), nullptr ) << TlsMsg::szError();
	}
	{
		TlsMsg::ResetErr();
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( 1, 0, 0, 0x10B );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10B ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "schema-less" ), nullptr ) << TlsMsg::szError();
	}
	{
		TlsMsg::ResetErr();
		const CSphSchema tSchema = MakeRtTrxSchema ( 2 );
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( std::numeric_limits<DWORD>::max(), 0 );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10C, &tSchema ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "count overflow" ), nullptr ) << TlsMsg::szError();
	}
	{
		TlsMsg::ResetErr();
		const CSphSchema tSchema = MakeRtTrxSchema ( 1 );
		constexpr DWORD LARGE_COUNT = DWORD(1)<<30;
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( LARGE_COUNT, LARGE_COUNT );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10C, &tSchema ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "reader limit" ), nullptr ) << TlsMsg::szError();
	}
	{
		TlsMsg::ResetErr();
		const CSphSchema tSchema = MakeRtTrxSchema ( 1 );
		RtTrxPayload_t tPayload = MakeRtTrxPayload ( std::numeric_limits<DWORD>::max(), 0, 0, 0x10B );
		RtAccum_t tLoaded;
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10B, &tSchema ) );
		EXPECT_NE ( strstr ( TlsMsg::szError(), "reader limit" ), nullptr ) << TlsMsg::szError();
	}
}

TEST ( RtTrx, SchemaBackedSaveRejectsShortLongAndOverflowNormsBeforeWriting )
{
	CSphSchema tSchema = MakeRtTrxSchema ( 2 );
	auto pIndex = sphCreateIndexRT ( "rt_trx_save_validation", "rt_trx_save_validation", tSchema, 1024*1024 );
	ASSERT_TRUE ( pIndex );

	RtAccum_t tSaved;
	tSaved.SetIndex ( pIndex.get() );
	auto fnFailsWithoutWriting = [&] ( DWORD uRows, int iNorms, const char * szError )
	{
		tSaved.m_uAccumDocs = uRows;
		tSaved.m_dNorms.Resize ( iNorms );
		CSphVector<BYTE> dPayload;
		dPayload.Add ( 0xa5 );
		MemoryWriter_c tWriter ( dPayload );
		TlsMsg::ResetErr();
		EXPECT_FALSE ( tSaved.SaveRtTrx ( tWriter ) );
		ASSERT_EQ ( dPayload.GetLength(), 1 );
		EXPECT_EQ ( dPayload[0], 0xa5 );
		EXPECT_NE ( strstr ( TlsMsg::szError(), szError ), nullptr ) << TlsMsg::szError();
	};

	fnFailsWithoutWriting ( 1, 1, "count mismatch" );
	fnFailsWithoutWriting ( 1, 3, "count mismatch" );
	fnFailsWithoutWriting ( std::numeric_limits<DWORD>::max(), 0, "count overflow" );
}

TEST ( RtTrx, ValidatesKeywordDictionaryRecordsByCapturedFormat )
{
	const CompleteRtTrxPayload_t tPayload = MakeCompleteRtTrxPayload();
	auto fnLoad = [&] ( const BYTE * pPacked, int iPacked, SphWordID_t uOffset, DictFormat_e eFormat )
	{
		RtAccum_t tLoaded;
		CSphVector<BYTE> dData = WithPackedKeywords ( tPayload, pPacked, iPacked, uOffset );
		TlsMsg::ResetErr();
		return tLoaded.LoadRtTrx ( AsBlob(dData), 0x10C, &tPayload.m_tSchema, eFormat );
	};

	const BYTE dLegacy[] = { 0, 3, 'o', 'n', 'e' };
	EXPECT_TRUE ( fnLoad ( dLegacy, sizeof(dLegacy), 1, DictFormat_e::KEYWORDS ) ) << TlsMsg::szError();
	EXPECT_FALSE ( fnLoad ( dLegacy, sizeof(dLegacy), sizeof(dLegacy), DictFormat_e::KEYWORDS ) );
	const BYTE dLegacyBadPrefix[] = { 0, 0 };
	EXPECT_FALSE ( fnLoad ( dLegacyBadPrefix, sizeof(dLegacyBadPrefix), 1, DictFormat_e::KEYWORDS ) );
	const BYTE dLegacyTruncated[] = { 0, 4, 'x' };
	EXPECT_FALSE ( fnLoad ( dLegacyTruncated, sizeof(dLegacyTruncated), 1, DictFormat_e::KEYWORDS ) );

	CSphVector<BYTE> dV2;
	dV2.Add ( 0 );
	dV2.Add ( 0x82 ); // 130, encoded by ZipToPtrLE as 0x82 0x01
	dV2.Add ( 0x01 );
	for ( int i=0; i<130; ++i )
		dV2.Add ( BYTE('a'+i%26) );
	EXPECT_TRUE ( fnLoad ( dV2.Begin(), dV2.GetLength(), 1, DictFormat_e::KEYWORDS_V2 ) ) << TlsMsg::szError();
	const BYTE dV2BadPrefix[] = { 0, 0x80 };
	EXPECT_FALSE ( fnLoad ( dV2BadPrefix, sizeof(dV2BadPrefix), 1, DictFormat_e::KEYWORDS_V2 ) );
	const BYTE dV2Truncated[] = { 0, 5, 'x' };
	EXPECT_FALSE ( fnLoad ( dV2Truncated, sizeof(dV2Truncated), 1, DictFormat_e::KEYWORDS_V2 ) );

	// CRC word IDs are hashes, not offsets, and never index the packed payload.
	EXPECT_TRUE ( fnLoad ( nullptr, 0, SphWordID_t(-1), DictFormat_e::CRC ) ) << TlsMsg::szError();
}

TEST ( RtTrx, RejectsBlobLocatorsNestedEndsAndInvalidHitFields )
{
	const CompleteRtTrxPayload_t tPayload = MakeCompleteRtTrxPayload();
	auto fnRejects = [&] ( CSphVector<BYTE> dData )
	{
		RtAccum_t tLoaded;
		TlsMsg::ResetErr();
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(dData), 0x10C, &tPayload.m_tSchema ) );
		EXPECT_NE ( TlsMsg::szError()[0], '\0' );
	};

	CSphVector<BYTE> dBadLocator = tPayload.m_dData;
	CSphFixedVector<CSphRowitem> dRow ( tPayload.m_tSchema.GetRowSize() );
	memcpy ( dRow.Begin(), dBadLocator.Begin()+tPayload.m_iRowsData, dRow.GetLengthBytes() );
	const CSphColumnInfo * pBlobLocator = tPayload.m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	ASSERT_NE ( pBlobLocator, nullptr );
	sphSetRowAttr ( dRow.Begin(), pBlobLocator->m_tLocator, 1 );
	memcpy ( dBadLocator.Begin()+tPayload.m_iRowsData, dRow.Begin(), dRow.GetLengthBytes() );
	fnRejects ( std::move(dBadLocator) );

	CSphVector<BYTE> dBadBlobEnd = tPayload.m_dData;
	dBadBlobEnd[tPayload.m_iBlobsData+1] = 0xff;
	fnRejects ( std::move(dBadBlobEnd) );

	CSphVector<BYTE> dMissingField = tPayload.m_dData;
	SetPayloadValue<Hitpos_t> ( dMissingField, tPayload.m_iHitsData+sizeof(RowID_t)+sizeof(SphWordID_t), HITMAN::Create(tPayload.m_tSchema.GetFieldsCount(),1,true) );
	fnRejects ( std::move(dMissingField) );

	CSphVector<BYTE> dStoredOnlyField = tPayload.m_dData;
	SetPayloadValue<Hitpos_t> ( dStoredOnlyField, tPayload.m_iHitsData+sizeof(RowID_t)+sizeof(SphWordID_t), HITMAN::Create(1,1,true) );
	fnRejects ( std::move(dStoredOnlyField) );
}

TEST ( RtTrx, ReplicationValidationIdentityRejectsReplacementSchemaAndAlterChanges )
{
	RtAccum_t tAccum;
	tAccum.CaptureReplicationValidation ( CSphString("rt_binding"), 101, 202, 3, DictFormat_e::KEYWORDS_V2 );
	CSphString sError;
	EXPECT_TRUE ( tAccum.CheckReplicationValidation ( 101, 202, 3, DictFormat_e::KEYWORDS_V2, sError ) );
	EXPECT_FALSE ( tAccum.CheckReplicationValidation ( 102, 202, 3, DictFormat_e::KEYWORDS_V2, sError ) );
	EXPECT_FALSE ( tAccum.CheckReplicationValidation ( 101, 203, 3, DictFormat_e::KEYWORDS_V2, sError ) );
	EXPECT_FALSE ( tAccum.CheckReplicationValidation ( 101, 202, 4, DictFormat_e::KEYWORDS_V2, sError ) );
	EXPECT_FALSE ( tAccum.CheckReplicationValidation ( 101, 202, 3, DictFormat_e::KEYWORDS, sError ) );
}

TEST ( RtTrx, RejectsMalformedCompletePayloadWithoutPublishingPartialState )
{
	const CompleteRtTrxPayload_t tGood = MakeCompleteRtTrxPayload();
	const CSphSchema & tSchema = tGood.m_tSchema;
	int iCase = 0;
	auto fnRejects = [&] ( CSphVector<BYTE> dPayload )
	{
		++iCase;
		RtAccum_t tLoaded;
		tLoaded.m_uAccumDocs = 77;
		tLoaded.m_iAccumBytes = 79;
		tLoaded.m_dAccumRows.Add ( 83 );
		tLoaded.m_dNorms.Add ( 89 );
		tLoaded.m_dBlobs.Add ( 97 );
		tLoaded.m_dPerDocHitsCount.Add ( 101 );
		tLoaded.m_dAccumKlist.Add ( 103 );
		tLoaded.MarkPreparedForCommit();
		TlsMsg::ResetErr();
		EXPECT_FALSE ( tLoaded.LoadRtTrx ( AsBlob(dPayload), 0x10C, &tSchema ) ) << "case=" << iCase << ", payload bytes=" << dPayload.GetLength();
		EXPECT_NE ( TlsMsg::szError()[0], '\0' );
		EXPECT_EQ ( tLoaded.m_uAccumDocs, 77u );
		EXPECT_EQ ( tLoaded.m_iAccumBytes, 79 );
		ASSERT_EQ ( tLoaded.m_dAccumRows.GetLength(), 1 );
		EXPECT_EQ ( tLoaded.m_dAccumRows[0], 83u );
		ASSERT_EQ ( tLoaded.m_dNorms.GetLength(), 1 );
		EXPECT_EQ ( tLoaded.m_dNorms[0], 89u );
		ASSERT_EQ ( tLoaded.m_dBlobs.GetLength(), 1 );
		EXPECT_EQ ( tLoaded.m_dBlobs[0], 97u );
		ASSERT_EQ ( tLoaded.m_dPerDocHitsCount.GetLength(), 1 );
		EXPECT_EQ ( tLoaded.m_dPerDocHitsCount[0], 101u );
		ASSERT_EQ ( tLoaded.m_dAccumKlist.GetLength(), 1 );
		EXPECT_EQ ( tLoaded.m_dAccumKlist[0], 103u );
		EXPECT_TRUE ( tLoaded.IsPreparedForCommit() );
	};

	for ( int iLength : { 0, 1+(int)sizeof(DWORD), tGood.m_iHitsData+1, tGood.m_iRowsData+1,
		tGood.m_iBlobsData+1, tGood.m_iPerDocData+1, tGood.m_iPackedData+1,
		tGood.m_iDocstoreData+1, tGood.m_dColumnarValueData[0]+1, tGood.m_iKillData+1 } )
	{
		SCOPED_TRACE ( iLength );
		CSphVector<BYTE> dTruncated = tGood.m_dData;
		dTruncated.Resize ( iLength );
		fnRejects ( std::move(dTruncated) );
	}

	for ( int iCountOffset : { tGood.m_iHitsCount, tGood.m_iRowsCount, tGood.m_iBlobsCount,
		tGood.m_iPerDocCount, tGood.m_iPackedCount, tGood.m_iColumnarCount, tGood.m_iKillCount } )
	{
		CSphVector<BYTE> dForged = tGood.m_dData;
		SetPayloadDword ( dForged, iCountOffset, std::numeric_limits<DWORD>::max() );
		fnRejects ( std::move(dForged) );
	}

	CSphVector<BYTE> dForgedDocstore = tGood.m_dData;
	dForgedDocstore[tGood.m_iDocstoreDocs] = 0x7f;
	fnRejects ( std::move(dForgedDocstore) );

	auto fnRejectDword = [&] ( int iOffset, DWORD uValue )
	{
		CSphVector<BYTE> dForged = tGood.m_dData;
		SetPayloadDword ( dForged, iOffset, uValue );
		fnRejects ( std::move(dForged) );
	};
	fnRejectDword ( tGood.m_iRowsCount, tSchema.GetRowSize()+1 );
	fnRejectDword ( tGood.m_iPerDocCount, 0 );
	fnRejectDword ( tGood.m_iPerDocData, 0 );
	fnRejectDword ( tGood.m_iDocstoreFields, 3 );
	fnRejectDword ( tGood.m_iColumnarCount, tSchema.GetColumnarAttrsCount()-1 );
	fnRejectDword ( tGood.m_dColumnarTypes[1], SPH_ATTR_INTEGER );
	fnRejectDword ( tGood.m_dColumnarValueCounts[1], 0 );
	fnRejectDword ( tGood.m_dColumnarValueCounts[2], 0 );

	CSphVector<BYTE> dBadHit = tGood.m_dData;
	SetPayloadValue<RowID_t> ( dBadHit, tGood.m_iHitsData, 1 );
	fnRejects ( std::move(dBadHit) );
	CSphVector<BYTE> dBadDocLength = tGood.m_dData;
	dBadDocLength[tGood.m_iDocstoreFieldLength] = 0x7f;
	fnRejects ( std::move(dBadDocLength) );
	CSphVector<BYTE> dUnterminatedDocFieldLength = tGood.m_dData;
	dUnterminatedDocFieldLength.Insert ( tGood.m_iDocstoreFieldLength, 4 );
	for ( int i=0; i<5; ++i )
		dUnterminatedDocFieldLength[tGood.m_iDocstoreFieldLength+i] = 0x80;
	dUnterminatedDocFieldLength[tGood.m_iDocstoreDocLength] = 6;
	fnRejects ( std::move(dUnterminatedDocFieldLength) );
	CSphVector<BYTE> dBadStringOffset = tGood.m_dData;
	SetPayloadValue<int64_t> ( dBadStringOffset, tGood.m_iStringOffsetsData, 100 );
	fnRejects ( std::move(dBadStringOffset) );
	CSphVector<BYTE> dBadMvaOffset = tGood.m_dData;
	SetPayloadValue<int> ( dBadMvaOffset, tGood.m_iMvaOffsetsData, -1 );
	fnRejects ( std::move(dBadMvaOffset) );
	CSphVector<BYTE> dBadBool = tGood.m_dData;
	dBadBool[tGood.m_dColumnarValueData[1]] = 2;
	fnRejects ( std::move(dBadBool) );
	CSphVector<BYTE> dMissingDocstore = tGood.m_dData;
	dMissingDocstore[tGood.m_iDocstoreFlag] = 0;
	fnRejects ( std::move(dMissingDocstore) );
	CSphVector<BYTE> dMissingColumnar = tGood.m_dData;
	dMissingColumnar[tGood.m_iColumnarFlag] = 0;
	fnRejects ( std::move(dMissingColumnar) );

	CSphVector<BYTE> dTrailing = tGood.m_dData;
	dTrailing.Add ( 0xff );
	fnRejects ( std::move(dTrailing) );
}

TEST ( RtTrx, LoadsFullyValidatedCompletePayload )
{
	CompleteRtTrxPayload_t tPayload = MakeCompleteRtTrxPayload();
	RtAccum_t tLoaded;
	ASSERT_TRUE ( tLoaded.LoadRtTrx ( AsBlob(tPayload.m_dData), 0x10C, &tPayload.m_tSchema ) ) << TlsMsg::szError();
	EXPECT_EQ ( tLoaded.m_uAccumDocs, 1u );
	EXPECT_EQ ( tLoaded.m_iAccumBytes, 17 );
	ASSERT_EQ ( tLoaded.m_dAccum.GetLength(), 1 );
	ASSERT_EQ ( tLoaded.m_dAccumRows.GetLength(), tPayload.m_tSchema.GetRowSize() );
	const CSphColumnInfo * pRowValue = tPayload.m_tSchema.GetAttr ( "row_value" );
	ASSERT_NE ( pRowValue, nullptr );
	EXPECT_EQ ( sphGetRowAttr ( tLoaded.m_dAccumRows.Begin(), pRowValue->m_tLocator ), 11u );
	ASSERT_EQ ( tLoaded.m_dNorms.GetLength(), 1 );
	EXPECT_EQ ( tLoaded.m_dNorms[0], 13u );
	EXPECT_FALSE ( tLoaded.m_dBlobs.IsEmpty() );
	ASSERT_EQ ( tLoaded.m_dPerDocHitsCount.GetLength(), 1 );
	EXPECT_EQ ( tLoaded.m_dPerDocHitsCount[0], 1u );
	ASSERT_EQ ( tLoaded.m_dAccumKlist.GetLength(), 1 );
	EXPECT_EQ ( tLoaded.m_dAccumKlist[0], 47u );
}

TEST ( FastCount, Aggregate64 )
{
	CSphSchema tSchema;
	tSchema.AddAttr ( CSphColumnInfo ( "id", SPH_ATTR_BIGINT ), false );
	CSphQuery tQuery;
	tQuery.m_sQuery = "foo";
	auto & tItem = tQuery.m_dItems.Add();
	tItem.m_sExpr = "count(*)";
	tItem.m_sAlias = "n";
	SphQueueSettings_t tSettings ( tSchema );
	SphQueueRes_t tRes;
	CSphString sError;
	std::unique_ptr<ISphMatchSorter> pSorter ( sphCreateQueue ( tSettings, tQuery, sError, tRes ) );
	ASSERT_TRUE ( pSorter ) << sError.cstr();
	ASSERT_TRUE ( pSorter->IsGroupby() );
	auto p = pSorter.get();
	DWORD dStatic[2] = {};
	const uint64_t uLarge = (uint64_t(1)<<32)+17;
	PushFullTextCount ( uLarge, { &p, 1 }, dStatic );
	PushFullTextCount ( uLarge, { &p, 1 }, dStatic );
	CSphMatch tMatch;
	ASSERT_EQ ( pSorter->Flatten ( &tMatch ), 1 );
	EXPECT_EQ ( tMatch.GetAttr ( pSorter->GetSchema()->GetAttr("@count")->m_tLocator ), 2*uLarge );
}

static void DeleteIndexFiles ( const char * sIndex )
{
	if ( !sIndex )
		return;

	const char * sExts[] = {
		"kill", "lock", "meta", "ram", "0.spa", "0.spd", "0.spe", "0.sph", "0.spi", "0.spk", "0.spm", "0.spp" };

	CSphString sName;
	for (auto & sExt : sExts)
	{
		sName.SetSprintf ( "%s.%s", sIndex, sExt );
		unlink ( sName.cstr () );
	}
}

static void DeleteAttachTestFiles ( const char * szBase )
{
	CSphString sName;
	for ( const auto & tExt : sphGetExts() )
	{
		sName.SetSprintf ( "%s%s", szBase, tExt.m_szExt );
		unlink ( sName.cstr() );
		sName.SetSprintf ( "%s.0%s", szBase, tExt.m_szExt );
		unlink ( sName.cstr() );
	}

	for ( const char * szExt : { "lock", "meta", "meta.new", "ram", "kill" } )
	{
		sName.SetSprintf ( "%s.%s", szBase, szExt );
		unlink ( sName.cstr() );
	}
}

void TestRTInit ()
{
	CSphConfigSection tRTConfig;

	sphRTInit ( "" );
	Binlog::Configure ( tRTConfig, 0 );

	SmallStringHash_T<CSphIndex *> hIndexes;
	Binlog::Replay ( hIndexes );
}

#define RT_INDEX_FILE_NAME "test_temp"

class MockTestDoc_c : public CSphSource
{
public:
	explicit MockTestDoc_c ( const CSphSchema &tSchema, BYTE ** ppDocs, int iDocs, int iFields, const std::vector<DocID_t> * pIDs=nullptr )
		: CSphSource ( "test_doc" )
	{
		m_tSchema = tSchema;
		m_ppDocs = ppDocs;
		m_iDocCount = iDocs;
		m_iFields = iFields;
		m_pIDs = pIDs;
		m_dFieldLengths.Resize ( m_iFields );
		m_dFields.Reserve ( iFields );
	}

	BYTE ** NextDocument ( bool & bEOF, CSphString & ) override
	{
		bEOF = false;
		int iDoc = (int)++m_iDocsCounter;
		if ( iDoc>=m_iDocCount )
		{
			bEOF = true;
			return nullptr;
		}

		for ( int i = 0; i<m_iFields; i++ )
		{
			char * szField = ( char * ) ( m_ppDocs + iDoc * m_iFields )[i];
			m_dFieldLengths[i] = (int) strlen ( szField );
		}

		const CSphColumnInfo * pId = m_tSchema.GetAttr ( sphGetDocidName() );
		assert ( pId );
		m_tDocInfo.SetAttr( pId->m_tLocator, m_pIDs ? (*m_pIDs)[iDoc] : DocID_t(iDoc+1) );

		return m_ppDocs + iDoc * m_iFields;
	}

	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void() );

	bool IterateStart ( CSphString & ) final
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		m_iDocsCounter = -1;
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString& )); // return false;
	MOCK_METHOD2 ( IterateMultivaluedNext, bool(int64_t &, int64_t &)); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (DocID_t & ) ) ; // return false
	int GetFieldCount () const { return m_iFields; }

	CSphVector<VecTraits_T<const char>> GetFields ()
	{
		m_dFields.Resize(0);
		for ( int i=0; i<m_iFields; ++i)
		{
			auto pStr = (const char*) m_ppDocs[m_iDocsCounter*m_iFields + i];
			m_dFields.Add ( VecTraits_T<const char> (pStr,strlen(pStr)));
		}
		return m_dFields;
	}

	int m_iDocsCounter;
	int m_iDocCount;
	int m_iFields;
	BYTE ** m_ppDocs;
	const std::vector<DocID_t> * m_pIDs = nullptr;
	CSphVector<VecTraits_T<const char> > m_dFields;
	CSphVector<int> m_dFieldLengths;
};


class MockDocRandomizer_c : public CSphSource
{
public:
	static const int m_iMaxFields = 2;
	static const int m_iMaxFieldLen = 512;
	char m_dFields[m_iMaxFields][m_iMaxFieldLen];
	char * m_ppFields[m_iMaxFields];
	CSphVector<VecTraits_T<const char> > m_dMeasuredFields;
	int m_dFieldLengths[m_iMaxFields];
	int	m_iDocsCounter;

	explicit MockDocRandomizer_c ( const CSphSchema & tSchema ) : CSphSource ( "test_doc" )
	{
		m_tSchema = tSchema;
		m_dMeasuredFields.Reserve(m_iMaxFields);
		for ( int i=0; i<m_iMaxFields; ++i )
			m_ppFields[i] = (char *)&m_dFields[i];
	}

	BYTE ** NextDocument ( bool & bEOF, CSphString & ) override
	{
		bEOF = false;
		if ( m_iDocsCounter>800 )
		{
			bEOF = true;
			return nullptr;
		}

		++m_tDocInfo.m_tRowID;
		++m_iDocsCounter;

		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(0).m_tLocator, m_tDocInfo.m_tRowID+1000 );
		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(1).m_tLocator, 1313 );

		snprintf ( m_dFields[0], m_iMaxFieldLen, "cat title%d title%d title%d title%d title%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		snprintf ( m_dFields[1], m_iMaxFieldLen, "dog contentwashere%d contentwashere%d contentwashere%d contentwashere%d contentwashere%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		for ( int i=0; i < m_iMaxFields; ++i )
			m_dFieldLengths[i] = (int) strlen ( m_ppFields[i] );

		return (BYTE**) &m_ppFields[0];
	}


	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void () );

	bool IterateStart ( CSphString & ) final
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iDocsCounter = 0;
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString & ) ); // return false;
	MOCK_METHOD2 ( IterateMultivaluedNext, bool (int64_t &, int64_t &) ); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (DocID_t & ) ); // return false
	int  GetFieldCount () const { return m_iMaxFields; }

	CSphVector<VecTraits_T<const char>> GetFields ()
	{
		m_dMeasuredFields.Resize ( 0 );
		for ( const char * pStr : m_ppFields )
			m_dMeasuredFields.Add ( VecTraits_T<const char> ( pStr, strlen ( pStr ) ) );
		return m_dMeasuredFields;
	}
};


//////////////////////////////////////////////////////////////////////////



class RT : public ::testing::Test
{

protected:
	void SetUp() override
	{
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
		TestRTInit();
		tDictSettings.SetDictFormat ( DictFormat_e::CRC );

		pTok = Tokenizer::Detail::CreateUTF8Tokenizer ();

		tSrcSchema.Reset ();
		tSrcSchema.AddField ( "title" );
		tSrcSchema.AddField ( "content" );
	}

	void TearDown() override
	{
		Binlog::Deinit ();
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
	}

	CSphColumnInfo tCol;
	CSphSchema tSrcSchema;
	CSphString sError, sWarning;

	TokenizerRefPtr_c pTok;

	CSphDictSettings tDictSettings;
};

TEST_F ( RT, ScopedAnd2ScratchDeclineFallsBackToCanonicalBM25A )
{
	using Result_t = std::pair<int64_t,int>;
	struct QueryResult_t
	{
		std::vector<Result_t> m_dRows;
		int64_t m_iTotal = 0;
	};
	struct TestSeamGuard_t
	{
		~TestSeamGuard_t() { ResetE1TestLastWindow(); }
	} tGuard;

	Threads::CallCoroutine ( [&] {
		tCol.m_sName = "id";
		tCol.m_eAttrType = SPH_ATTR_BIGINT;
		tSrcSchema.AddAttr ( tCol, true );
		CSphSchema tSchema;
		for ( int i=0; i<tSrcSchema.GetFieldsCount(); ++i )
			tSchema.AddField ( tSrcSchema.GetField(i) );
		for ( int i=0; i<tSrcSchema.GetAttrsCount(); ++i )
			tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "scoped-and2", false, 32, nullptr, sError );
		ASSERT_TRUE ( pDict );
		auto pIndex = sphCreateIndexRT ( "scoped_and2_scratch", RT_INDEX_FILE_NAME, tSchema, 128*1024 );
		pIndex->SetTokenizer ( pTok );
		pIndex->SetDictionary ( pDict );
		pIndex->PostSetup();
		StrVec_t dWarnings;
		ASSERT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

		constexpr int DOCS = 4096;
		std::vector<std::string> dText;
		dText.reserve ( DOCS*2 );
		for ( int i=0; i<DOCS; ++i )
		{
			dText.emplace_back ( "title" );
			switch ( i%4 )
			{
			case 0: dText.emplace_back ( "error failed" ); break;
			case 1: dText.emplace_back ( "error error failed" ); break;
			case 2: dText.emplace_back ( "error failed failed" ); break;
			default: dText.emplace_back ( "error error failed failed" ); break;
			}
		}
		std::vector<BYTE*> dFields;
		dFields.reserve ( dText.size() );
		for ( auto & sText : dText )
			dFields.push_back ( (BYTE*)sText.data() );
		auto pSrc = new MockTestDoc_c ( tSrcSchema, dFields.data(), DOCS, 2 );
		EXPECT_CALL ( *pSrc, Connect ( testing::_ ) ).WillOnce ( testing::Return(true) );
		EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( DOCS ).WillRepeatedly ( testing::Return(pSrc->m_dFieldLengths.Begin()) );
		EXPECT_CALL ( *pSrc, Disconnect () );
		pSrc->SetTokenizer ( pIndex->GetTokenizer()->Clone(SPH_CLONE_INDEX) );
		pSrc->SetDict ( pIndex->GetDictionary()->Clone() );
		pSrc->Setup ( CSphSourceSettings(), nullptr );
		ASSERT_TRUE ( pSrc->Connect(sError) );
		ASSERT_TRUE ( pSrc->IterateStart(sError) );
		ASSERT_TRUE ( pSrc->UpdateSchema(&tSrcSchema,sError) );

		InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
		RtAccum_t tAcc;
		CSphString sFilter;
		const int iDynamic = pIndex->GetMatchSchema().GetRowSize();
		bool bEOF = false;
		while ( true )
		{
			ASSERT_TRUE ( pSrc->IterateDocument(bEOF,sError) );
			if ( bEOF )
				break;
			tDoc.m_dFields = pSrc->GetFields();
			tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
			ASSERT_TRUE ( pIndex->AddDocument(tDoc,false,sFilter,sError,sWarning,&tAcc) );
		}
		ASSERT_TRUE ( pIndex->Commit(nullptr,&tAcc,&sError) );
		pSrc->Disconnect();
		SafeDelete ( pSrc );
		ASSERT_TRUE ( pIndex->ForceDiskChunk() );

		auto fnQuery = [&] ( const char * szQuery, bool bGeneric, int iOffset, int iLimit )
		{
			QueryResult_t tOut;
			CSphQuery tQuery;
			SetQueryDefaultsExt2 ( tQuery );
			tQuery.m_sQuery = szQuery;
			tQuery.m_bExplicitRanker = true;
			tQuery.m_eRanker = bGeneric ? SPH_RANK_EXPR : SPH_RANK_BM25A;
			if ( bGeneric )
				tQuery.m_sRankerExpr = "1000*bm25a(1.2,0.75,256)";
			tQuery.m_iOffset = iOffset;
			tQuery.m_iLimit = iLimit;
			auto pParser = sphCreatePlainQueryParser();
			tQuery.m_pQueryParser = pParser.get();
			AggrResult_t tResult;
			CSphQueryResult tQueryResult;
			tQueryResult.m_pMeta = &tResult;
			CSphMultiQueryArgs tArgs ( 1 );
			SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema() );
			tQueueSettings.m_iMaxMatches = iOffset+iLimit;
			SphQueueRes_t tRes;
			ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
			EXPECT_TRUE ( pSorter );
			if ( !pSorter )
				return tOut;
			EXPECT_TRUE ( pIndex->MultiQuery(tQueryResult,tQuery,{&pSorter,1},tArgs) ) << tResult.m_sError.cstr();
			tOut.m_iTotal = pSorter->GetTotalCount();
			auto & tOne = tResult.m_dResults.Add();
			tOne.FillFromSorter ( pSorter );
			const CSphColumnInfo * pID = tOne.m_tSchema.GetAttr ( sphGetDocidName() );
			EXPECT_TRUE ( pID );
			if ( pID )
				for ( const auto & tMatch : tOne.m_dMatches )
					tOut.m_dRows.emplace_back ( tMatch.GetAttr(pID->m_tLocator), tMatch.m_iWeight );
			SafeDelete ( pSorter );
			if ( iOffset>0 && iOffset<=int(tOut.m_dRows.size()) )
				tOut.m_dRows.erase ( tOut.m_dRows.begin(), tOut.m_dRows.begin()+iOffset );
			return tOut;
		};

		constexpr uint32_t OVERSIZED_LAST_WINDOW = 1985;
		ASSERT_FALSE ( E1ScopedScratchAllowed(OVERSIZED_LAST_WINDOW) );
		SetE1TestLastWindow ( OVERSIZED_LAST_WINDOW );
		const auto tNamed = fnQuery ( "@content error failed", false, 0, 20 );
		const auto tGeneric = fnQuery ( "@content error failed", true, 0, 20 );
		const auto tParenNamed = fnQuery ( "@content (error failed)", false, 0, 20 );
		const auto tParenGeneric = fnQuery ( "@content (error failed)", true, 0, 20 );
		const auto tPage = fnQuery ( "@content error failed", false, 1, 2 );
		const auto tGenericPage = fnQuery ( "@content error failed", true, 1, 2 );

		EXPECT_EQ ( GetE1TestScratchDeclines(), 6u );
		EXPECT_EQ ( GetE1TestDirectExecutorCalls(), 0u );
		EXPECT_EQ ( tNamed.m_dRows, tGeneric.m_dRows );
		EXPECT_EQ ( tNamed.m_iTotal, tGeneric.m_iTotal );
		EXPECT_EQ ( tParenNamed.m_dRows, tParenGeneric.m_dRows );
		EXPECT_EQ ( tParenNamed.m_iTotal, tParenGeneric.m_iTotal );
		EXPECT_EQ ( tNamed.m_dRows, tParenNamed.m_dRows );
		EXPECT_EQ ( tNamed.m_iTotal, tParenNamed.m_iTotal );
		EXPECT_EQ ( tPage.m_dRows, tGenericPage.m_dRows );
		EXPECT_EQ ( tPage.m_iTotal, tGenericPage.m_iTotal );
		EXPECT_EQ ( tNamed.m_iTotal, DOCS );
		ASSERT_EQ ( tNamed.m_dRows.size(), 20u );
		std::vector<Result_t> dExpected;
		for ( int64_t iID=1; iID<=77; iID+=4 )
			dExpected.emplace_back ( iID, 118 );
		EXPECT_EQ ( tNamed.m_dRows, dExpected );
		ASSERT_EQ ( tPage.m_dRows.size(), 2u );
		EXPECT_EQ ( tPage.m_dRows[0], tNamed.m_dRows[1] );
		EXPECT_EQ ( tPage.m_dRows[1], tNamed.m_dRows[2] );
	} );
}

TEST_F ( RT, E1Post07RatioBoundsDriveExactCanonicalBM25A )
{
	using Result_t = std::pair<int64_t,int>;
	struct QueryResult_t
	{
		std::vector<Result_t> m_dRows;
		int64_t m_iTotal = 0;
	};
	struct TestSeamGuard_t
	{
		~TestSeamGuard_t() { SetE1TestForceGenericRanked(false); ResetE1TestRankStats(); }
	} tGuard;

	Threads::CallCoroutine ( [&] {
		tCol.m_sName = "id";
		tCol.m_eAttrType = SPH_ATTR_BIGINT;
		tSrcSchema.AddAttr ( tCol, true );
		CSphSchema tSchema;
		for ( int i=0; i<tSrcSchema.GetFieldsCount(); ++i )
			tSchema.AddField ( tSrcSchema.GetField(i) );
		for ( int i=0; i<tSrcSchema.GetAttrsCount(); ++i )
			tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "e1post07-ratio", false, 32, nullptr, sError );
		ASSERT_TRUE ( pDict );
		auto pIndex = sphCreateIndexRT ( "e1post07_ratio", RT_INDEX_FILE_NAME, tSchema, 128*1024 );
		pIndex->SetTokenizer ( pTok );
		pIndex->SetDictionary ( pDict );
		pIndex->PostSetup();
		StrVec_t dWarnings;
		ASSERT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

		std::string sHigh, sLow = "hot", sAbsent = "cold";
		for ( int i=0; i<20; ++i )
			sHigh += i ? " hot" : "hot";
		for ( int i=0; i<100; ++i )
			sLow += " filler";
		constexpr int DOCS = 10000;
		constexpr int MATCHES = 4101; // 64 full posting blocks plus a five-row tail
		std::vector<std::string> dText;
		std::vector<DocID_t> dIDs;
		dText.reserve ( DOCS*2 );
		dIDs.reserve ( DOCS );
		for ( int i=0; i<DOCS; ++i )
		{
			dText.emplace_back ( "title" );
			dText.emplace_back ( i<128 ? sHigh : i<MATCHES ? sLow : sAbsent );
			dIDs.push_back ( DocID_t(DOCS-i) );
		}
		std::vector<BYTE*> dFields;
		dFields.reserve ( dText.size() );
		for ( auto & sText : dText )
			dFields.push_back ( (BYTE*)sText.data() );
		auto pSrc = new MockTestDoc_c ( tSrcSchema, dFields.data(), DOCS, 2, &dIDs );
		EXPECT_CALL ( *pSrc, Connect ( testing::_ ) ).WillOnce ( testing::Return(true) );
		EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( DOCS ).WillRepeatedly ( testing::Return(pSrc->m_dFieldLengths.Begin()) );
		EXPECT_CALL ( *pSrc, Disconnect () );
		pSrc->SetTokenizer ( pIndex->GetTokenizer()->Clone(SPH_CLONE_INDEX) );
		pSrc->SetDict ( pIndex->GetDictionary()->Clone() );
		pSrc->Setup ( CSphSourceSettings(), nullptr );
		ASSERT_TRUE ( pSrc->Connect(sError) );
		ASSERT_TRUE ( pSrc->IterateStart(sError) );
		ASSERT_TRUE ( pSrc->UpdateSchema(&tSrcSchema,sError) );
		InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
		RtAccum_t tAcc;
		CSphString sFilter;
		const int iDynamic = pIndex->GetMatchSchema().GetRowSize();
		bool bEOF = false;
		while ( true )
		{
			ASSERT_TRUE ( pSrc->IterateDocument(bEOF,sError) );
			if ( bEOF )
				break;
			tDoc.m_dFields = pSrc->GetFields();
			tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
			ASSERT_TRUE ( pIndex->AddDocument(tDoc,false,sFilter,sError,sWarning,&tAcc) );
		}
		ASSERT_TRUE ( pIndex->Commit(nullptr,&tAcc,&sError) );
		pSrc->Disconnect();
		SafeDelete ( pSrc );
		ASSERT_TRUE ( pIndex->ForceDiskChunk() );

		auto fnQuery = [&] ( const char * szQuery, bool bForceGeneric, int iOffset, int iLimit, const char * szExpr=nullptr, const char * szSort=nullptr, int iNamedFieldWeight=0, int iPositionalFieldWeight=0, bool bFilter=false )
		{
			SetE1TestForceGenericRanked ( bForceGeneric );
			ResetE1TestRankStats();
			QueryResult_t tOut;
			CSphQuery tQuery;
			SetQueryDefaultsExt2 ( tQuery );
			tQuery.m_sQuery = szQuery;
			tQuery.m_bExplicitRanker = true;
			tQuery.m_eRanker = szExpr ? SPH_RANK_EXPR : SPH_RANK_BM25A;
			if ( szExpr )
				tQuery.m_sRankerExpr = szExpr;
			if ( szSort )
			{
				tQuery.m_bExplicitOrderBy = true;
				tQuery.m_sSortBy = szSort;
				tQuery.m_sOrderBy = szSort;
			}
			if ( iNamedFieldWeight )
				tQuery.m_dFieldWeights.Add ( { CSphString("content"), iNamedFieldWeight } );
			if ( iPositionalFieldWeight )
				tQuery.m_dWeights.Add ( DWORD(iPositionalFieldWeight) );
			if ( bFilter )
			{
				auto & tFilter = tQuery.m_dFilters.Add();
				tFilter.m_eType = SPH_FILTER_RANGE;
				tFilter.m_sAttrName = sphGetDocidName();
				tFilter.m_iMinValue = DOCS/2+1;
				tFilter.m_iMaxValue = DOCS;
			}
			tQuery.m_iOffset = iOffset;
			tQuery.m_iLimit = iLimit;
			auto pParser = sphCreatePlainQueryParser();
			tQuery.m_pQueryParser = pParser.get();
			AggrResult_t tResult;
			CSphQueryResult tQueryResult;
			tQueryResult.m_pMeta = &tResult;
			CSphMultiQueryArgs tArgs ( 1 );
			SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema() );
			tQueueSettings.m_iMaxMatches = iOffset+iLimit;
			SphQueueRes_t tRes;
			ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
			EXPECT_TRUE ( pSorter );
			if ( !pSorter )
				return tOut;
			EXPECT_TRUE ( pIndex->MultiQuery(tQueryResult,tQuery,{&pSorter,1},tArgs) ) << tResult.m_sError.cstr();
			tOut.m_iTotal = pSorter->GetTotalCount();
			auto & tOne = tResult.m_dResults.Add();
			tOne.FillFromSorter ( pSorter );
			const CSphColumnInfo * pID = tOne.m_tSchema.GetAttr ( sphGetDocidName() );
			EXPECT_TRUE ( pID );
			if ( pID )
				for ( const auto & tMatch : tOne.m_dMatches )
					tOut.m_dRows.emplace_back ( tMatch.GetAttr(pID->m_tLocator), tMatch.m_iWeight );
			SafeDelete ( pSorter );
			if ( iOffset>0 && iOffset<=int(tOut.m_dRows.size()) )
				tOut.m_dRows.erase ( tOut.m_dRows.begin(), tOut.m_dRows.begin()+iOffset );
			return tOut;
		};

		const auto tDirect = fnQuery ( "hot", false, 0, 10 );
		const E1TestRankStats_t tDirectStats = GetE1TestRankStats();
		const auto tGeneric = fnQuery ( "hot", true, 0, 10 );
		const E1TestRankStats_t tGenericStats = GetE1TestRankStats();
		EXPECT_EQ ( tDirect.m_dRows, tGeneric.m_dRows );
		EXPECT_EQ ( tDirect.m_iTotal, tGeneric.m_iTotal );
		EXPECT_EQ ( tDirect.m_iTotal, MATCHES );
		EXPECT_EQ ( tDirectStats.m_eBoundKind, E1RankedBoundKind_e::BM25A_RATIO );
		EXPECT_GT ( tDirectStats.m_uSelectedBlocks, 0u );
		EXPECT_GT ( tDirectStats.m_uSkippedBlocks, 0u );
		EXPECT_GT ( tDirectStats.m_uSkippedDocs, 0u );
		EXPECT_LT ( tDirectStats.m_uScoredDocs, uint64_t(MATCHES) );
		EXPECT_GT ( tGenericStats.m_uFallbacks, 0u );
		EXPECT_EQ ( tGenericStats.m_uSelectedBlocks, 0u );

		const auto tPage = fnQuery ( "hot", false, 3, 7 );
		const auto tGenericPage = fnQuery ( "hot", true, 3, 7 );
		EXPECT_EQ ( tPage.m_dRows, tGenericPage.m_dRows );
		EXPECT_EQ ( tPage.m_iTotal, tGenericPage.m_iTotal );

		const auto tPublicIDTie = fnQuery ( "hot", false, 0, 10, nullptr, "@weight desc, id asc" );
		const auto tPublicIDTieGeneric = fnQuery ( "hot", true, 0, 10, nullptr, "@weight desc, id asc" );
		EXPECT_EQ ( tPublicIDTie.m_dRows, tPublicIDTieGeneric.m_dRows );
		EXPECT_EQ ( tPublicIDTie.m_iTotal, tPublicIDTieGeneric.m_iTotal );

		// E1POST07 ratio bytes are not max-TF. Compound compatibility keeps the
		// v5/v6 direct container and exact-TF lanes while disabling bound pruning.
		const auto tAnd = fnQuery ( "hot title", false, 0, 10 );
		const E1TestRankStats_t tAndStats = GetE1TestRankStats();
		const auto tAndGeneric = fnQuery ( "hot title", true, 0, 10 );
		EXPECT_EQ ( tAnd.m_dRows, tAndGeneric.m_dRows );
		EXPECT_EQ ( tAnd.m_iTotal, tAndGeneric.m_iTotal );
		EXPECT_EQ ( tAnd.m_iTotal, MATCHES );
		EXPECT_GT ( tAndStats.m_uDirectAnd, 0u );

		const auto tOr = fnQuery ( "hot | title", false, 0, 10 );
		const E1TestRankStats_t tOrStats = GetE1TestRankStats();
		const auto tOrGeneric = fnQuery ( "hot | title", true, 0, 10 );
		EXPECT_EQ ( tOr.m_dRows, tOrGeneric.m_dRows );
		EXPECT_EQ ( tOr.m_iTotal, tOrGeneric.m_iTotal );
		EXPECT_EQ ( tOr.m_iTotal, DOCS );
		EXPECT_GT ( tOrStats.m_uDirectOr, 0u );

		const auto tFilter = fnQuery ( "hot", false, 0, 10, nullptr, nullptr, 0, 0, true );
		const E1TestRankStats_t tFilterStats = GetE1TestRankStats();
		const auto tFilterGeneric = fnQuery ( "hot", true, 0, 10, nullptr, nullptr, 0, 0, true );
		EXPECT_EQ ( tFilter.m_dRows, tFilterGeneric.m_dRows );
		EXPECT_EQ ( tFilter.m_iTotal, tFilterGeneric.m_iTotal );
		EXPECT_GT ( tFilterStats.m_uDirectFilter, 0u );

		// A true field subset on the two-field schema must fail closed.
		const auto tScoped = fnQuery ( "@content hot", false, 0, 10 );
		const E1TestRankStats_t tScopedStats = GetE1TestRankStats();
		const auto tScopedGeneric = fnQuery ( "@content hot", true, 0, 10 );
		EXPECT_EQ ( tScoped.m_dRows, tScopedGeneric.m_dRows );
		EXPECT_EQ ( tScoped.m_iTotal, tScopedGeneric.m_iTotal );
		EXPECT_EQ ( tScopedStats.m_uSelectedBlocks, 0u );
		EXPECT_GT ( tScopedStats.m_uFallbacks, 0u );

		// Canonical-expression spelling is supported; coefficient and sort near
		// misses remain exact generic execution.
		const auto tExpr = fnQuery ( "hot", false, 0, 10, "1000*bm25a(1.2,0.75,256)" );
		EXPECT_EQ ( tExpr.m_dRows, tGeneric.m_dRows );
		EXPECT_EQ ( tExpr.m_iTotal, tGeneric.m_iTotal );
		const auto tAlt = fnQuery ( "hot", false, 0, 10, "1000*bm25a(1.3,0.75,256)" );
		const E1TestRankStats_t tAltStats = GetE1TestRankStats();
		EXPECT_EQ ( tAltStats.m_uSelectedBlocks, 0u );
		EXPECT_EQ ( tAlt.m_iTotal, MATCHES );
		const auto tAltSort = fnQuery ( "hot", false, 0, 10, nullptr, "id asc" );
		const E1TestRankStats_t tSortStats = GetE1TestRankStats();
		EXPECT_EQ ( tSortStats.m_uSelectedBlocks, 0u );
		EXPECT_EQ ( tAltSort.m_iTotal, MATCHES );

		const auto tNamedWeight = fnQuery ( "hot", false, 0, 10, nullptr, nullptr, 2 );
		const E1TestRankStats_t tNamedWeightStats = GetE1TestRankStats();
		const auto tNamedWeightGeneric = fnQuery ( "hot", true, 0, 10, nullptr, nullptr, 2 );
		EXPECT_EQ ( tNamedWeight.m_dRows, tNamedWeightGeneric.m_dRows );
		EXPECT_EQ ( tNamedWeight.m_iTotal, tNamedWeightGeneric.m_iTotal );
		EXPECT_EQ ( tNamedWeightStats.m_uSelectedBlocks, 0u );

		const auto tPositionalWeight = fnQuery ( "hot", false, 0, 10, nullptr, nullptr, 0, 2 );
		const E1TestRankStats_t tPositionalWeightStats = GetE1TestRankStats();
		const auto tPositionalWeightGeneric = fnQuery ( "hot", true, 0, 10, nullptr, nullptr, 0, 2 );
		EXPECT_EQ ( tPositionalWeight.m_dRows, tPositionalWeightGeneric.m_dRows );
		EXPECT_EQ ( tPositionalWeight.m_iTotal, tPositionalWeightGeneric.m_iTotal );
		EXPECT_EQ ( tPositionalWeightStats.m_uSelectedBlocks, 0u );

		CSphVector<DocID_t> dDelete;
		dDelete.Add ( dIDs[100] );
		RtAccum_t tDeleteAcc;
		ASSERT_TRUE ( pIndex->DeleteDocument(dDelete,sError,&tDeleteAcc) );
		ASSERT_TRUE ( pIndex->Commit(nullptr,&tDeleteAcc,&sError) );
		const auto tDeleted = fnQuery ( "hot", false, 0, 10 );
		const E1TestRankStats_t tDeletedStats = GetE1TestRankStats();
		const auto tDeletedGeneric = fnQuery ( "hot", true, 0, 10 );
		EXPECT_EQ ( tDeleted.m_dRows, tDeletedGeneric.m_dRows );
		EXPECT_EQ ( tDeleted.m_iTotal, MATCHES-1 );
		EXPECT_EQ ( tDeletedStats.m_uSelectedBlocks, 0u );
	} );
}

TEST_F ( RT, AttachReportsMetadataFailure )
{
	static constexpr const char * SOURCE_PATH = "test_attach_source";
	static constexpr const char * TARGET_PATH = "test_attach_target";
	DeleteAttachTestFiles ( SOURCE_PATH );
	DeleteAttachTestFiles ( TARGET_PATH );

	Threads::CallCoroutine ( [&] {
		tCol.m_sName = "id";
		tCol.m_eAttrType = SPH_ATTR_BIGINT;
		tSrcSchema.AddAttr ( tCol, true );

		CSphSchema tSchema;
		for ( int i=0; i<tSrcSchema.GetFieldsCount(); ++i )
			tSchema.AddField ( tSrcSchema.GetField(i) );
		for ( int i=0; i<tSrcSchema.GetAttrsCount(); ++i )
			tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "attach", false, 32, nullptr, sError );
		ASSERT_TRUE ( pDict );

		auto fnCreateRt = [&] ( const char * szName, const char * szPath ) {
			auto pIndex = sphCreateIndexRT ( szName, szPath, tSchema, 128 * 1024 );
			pIndex->SetTokenizer ( pTok->Clone ( SPH_CLONE_INDEX ) );
			pIndex->SetDictionary ( pDict->Clone() );
			pIndex->PostSetup();
			return pIndex;
		};

		auto pSource = fnCreateRt ( "attach_source", SOURCE_PATH );
		StrVec_t dWarnings;
		ASSERT_TRUE ( pSource->Prealloc ( false, nullptr, dWarnings ) );

		InsertDocData_c tDoc ( pSource->GetMatchSchema() );
		tDoc.SetID ( 101 );
		const char * szTitle = "attached row";
		const char * szContent = "metadata failure";
		tDoc.m_dFields.Add ( { szTitle, (int64_t)strlen ( szTitle ) } );
		tDoc.m_dFields.Add ( { szContent, (int64_t)strlen ( szContent ) } );

		RtAccum_t tAcc;
		CSphString sFilter;
		ASSERT_TRUE ( pSource->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc ) );
		ASSERT_TRUE ( pSource->Commit ( nullptr, &tAcc, &sError ) );
		ASSERT_TRUE ( pSource->ForceDiskChunk() );
		pSource.reset();

		auto pPlain = sphCreateIndexPhrase ( "attach_plain", SphSprintf ( "%s.0", SOURCE_PATH ) );
		dWarnings.Reset();
		ASSERT_TRUE ( pPlain->Prealloc ( false, nullptr, dWarnings ) );

		auto pTarget = fnCreateRt ( "attach_target", TARGET_PATH );
		dWarnings.Reset();
		ASSERT_TRUE ( pTarget->Prealloc ( false, nullptr, dWarnings ) );
		pTarget->ProhibitSave();

		bool bFatal = false;
		sError = "";
		auto eResult = pTarget->AttachDiskIndexWithMeta ( pPlain.get(), false, bFatal, sError );
		if ( eResult!=DiskAttachRes_e::NOT_ATTACHED )
			pPlain.release();

		EXPECT_EQ ( eResult, DiskAttachRes_e::META_FAILED );
		EXPECT_FALSE ( bFatal );
		EXPECT_THAT ( sError.cstr(), testing::HasSubstr ( "failed to save metadata" ) );
		EXPECT_EQ ( pTarget->GetStats().m_iTotalDocuments, 1 );
		pTarget->EnableSave();
	} );

	DeleteAttachTestFiles ( SOURCE_PATH );
	DeleteAttachTestFiles ( TARGET_PATH );
}

/*
 * It was instantiated several times, but that wasn't work, since on every instantiation couple of attributes was inserted into schema, having idex's schema the same.
 */

TEST_F ( RT, WeightBoundary )
{
	DWORD uParam = 1500;
	using namespace testing;
	Threads::CallCoroutine ( [&] {
	DictRefPtr_c pDict { sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "weight", false, 32, nullptr, sError ) };

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "channel_id";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	const char * dFields[] = { "If I were a cat...", "We are the greatest cat" };
	auto * pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, 1, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).WillOnce ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );
	pSrc->Setup ( CSphSourceSettings(), nullptr );

	EXPECT_TRUE ( pSrc->Connect ( sError ) );
	EXPECT_TRUE ( pSrc->IterateStart ( sError ) );
	EXPECT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( "testrt", RT_INDEX_FILE_NAME, tSchema, 32 * 1024 * 1024 );

	// tricky bit
	// index owns its tokenizer/dict pair, and MAY do whatever it wants
	// and starting with meta v4, it WILL deallocate tokenizer/dict in Prealloc()
	// in favor of tokenizer/dict loaded from the saved settings in meta
	// however, source still needs those guys!
	// so for simplicity i just clone them
	pIndex->SetTokenizer ( pTok->Clone ( SPH_CLONE_INDEX ) );
	pIndex->SetDictionary ( pDict->Clone () );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	EXPECT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;

	CSphString sFilter;
	bool bEOF = false;
	while (true)
	{
		EXPECT_TRUE ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
		pIndex->Commit ( nullptr, &tAcc );
	}

	pSrc->Disconnect ();

	ASSERT_EQ ( pSrc->GetStats ().m_iTotalDocuments, 1) << "docs committed";

	CSphQuery tQuery;
	tQuery.m_eRanker = SPH_RANK_PROXIMITY_BM25;
	tQuery.m_bExplicitRanker = true;
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tQuery.m_sQuery = "@title cat";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	SphQueueRes_t tRes;
	ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
	ASSERT_TRUE ( pSorter );
	ASSERT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
	auto & tOneRes = tResult.m_dResults.Add ();
	tOneRes.FillFromSorter ( pSorter );
	ASSERT_EQ ( tResult.GetLength (), 1 ) << "results found";
	ASSERT_EQ ( tOneRes.m_dMatches[0].m_tRowID, 0 ) << "rowID" ;
	ASSERT_EQ ( tOneRes.m_dMatches[0].m_iWeight, uParam) << "weight" ;

	SafeDelete ( pSorter );
	SafeDelete ( pSrc );
	});
}


TEST_F ( RT, RankerFactors )
{
	using namespace testing;
	Threads::CallCoroutine ( [&] {

	const char * dFields[] = {
		"Seven lies multiplied by seven", "", "Multiplied by seven again", "", "Seven lies multiplied by seven"
		, "Multiplied by seven again", "Mary vs Lamb", "Mary had a little lamb little lamb little lamb"
		, "Mary vs Lamb 2: Return of The Lamb", "...whose fleece was white as snow", "Mary vs Lamb 3: The Resurrection"
		, "Snow! Bloody snow!", "the who", "what the foo"
	};
	const char * dQueries[] = {
		"seven !(angels !by)", // matched by 0-2
		"Mary lamb", // matched by 3-5
		"(the who) | (the foo)", // matched by 6
	};

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "idd";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", false, 32, nullptr, sError );

	auto pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, sizeof ( dFields ) / sizeof ( dFields[0] ) / 2
									, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( 7 ).WillRepeatedly ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( CSphSourceSettings(), nullptr );
	ASSERT_TRUE ( pSrc->Connect ( sError ) );
	ASSERT_TRUE ( pSrc->IterateStart ( sError ) );

	ASSERT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( "testrt", RT_INDEX_FILE_NAME, tSchema, 128 * 1024 );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "rt", false, 32, nullptr, sError ) );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	Verify ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	CSphString sFilter;
	InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;
	bool bEOF = false;
	while (true)
	{
		Verify ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
	}
	pIndex->Commit ( nullptr, &tAcc );
	pSrc->Disconnect ();

	CSphQuery tQuery;
	CSphQueryItem &tFactor = tQuery.m_dItems.Add ();
	tFactor.m_sExpr = "packedfactors()";
	tFactor.m_sAlias = "pf";
	tQuery.m_sRankerExpr = "1";
	tQuery.m_eRanker = SPH_RANK_EXPR;
	tQuery.m_bExplicitRanker = true;
	tQuery.m_eMode = SPH_MATCH_EXTENDED2;
	tQuery.m_eSort = SPH_SORT_EXTENDED;
	tQuery.m_sSortBy = "@weight desc";
	tQuery.m_sOrderBy = "@weight desc";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tArgs.m_uPackedFactorFlags = SPH_FACTOR_ENABLE | SPH_FACTOR_CALC_ATC;

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	tQueueSettings.m_bComputeItems = true;
	SphQueueRes_t tRes;

	for ( auto szQuery : dQueries )
	{
		tQuery.m_sQuery = szQuery;

		auto pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
		ASSERT_TRUE ( pSorter );
		ASSERT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
		auto & tOneRes = tResult.m_dResults.Add ();
		tOneRes.FillFromSorter ( pSorter );

		tResult.m_tSchema = *pSorter->GetSchema ();
		const CSphAttrLocator &tLoc = tResult.m_tSchema.GetAttr ( "pf" )->m_tLocator;

		for ( int iMatch = 0; iMatch<tOneRes.m_dMatches.GetLength (); ++iMatch )
		{
			const BYTE * pAttr = (const BYTE *) tOneRes.m_dMatches[iMatch].GetAttr ( tLoc );
			ASSERT_TRUE ( pAttr );

			auto * pFactors = (const unsigned int *) sphUnpackPtrAttr ( pAttr ).first;

			SPH_UDF_FACTORS tUnpacked;
			sphinx_factors_init ( &tUnpacked );
			sphinx_factors_unpack ( pFactors, &tUnpacked );

			// doc level factors
			ASSERT_EQ ( tUnpacked.doc_bm25, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_BM25 ) );
			ASSERT_EQ ( tUnpacked.doc_bm25a, sphinx_get_doc_factor_float ( pFactors, SPH_DOCF_BM25A ) );
			ASSERT_EQ ( tUnpacked.matched_fields, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MATCHED_FIELDS ) );
			ASSERT_EQ ( tUnpacked.doc_word_count, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_DOC_WORD_COUNT ) );
			ASSERT_EQ ( tUnpacked.num_fields, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_NUM_FIELDS ) );
			ASSERT_EQ ( tUnpacked.max_uniq_qpos, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MAX_UNIQ_QPOS ) );

			// field level factors
			for ( int iField = 0; iField<tUnpacked.num_fields; ++iField )
			{
				if ( !tUnpacked.field[iField].hit_count )
					continue;

				const unsigned int * pField = sphinx_get_field_factors ( pFactors, iField );
				ASSERT_TRUE ( pField );
				ASSERT_EQ ( tUnpacked.field[iField].hit_count, sphinx_get_field_factor_int ( pField
																							 , SPH_FIELDF_HIT_COUNT ) );
				ASSERT_EQ ( tUnpacked.field[iField].lcs, sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCS ) );
				ASSERT_EQ ( tUnpacked.field[iField].word_count, sphinx_get_field_factor_int ( pField
																							  , SPH_FIELDF_WORD_COUNT ) );
				ASSERT_EQ ( tUnpacked.field[iField].tf_idf, sphinx_get_field_factor_float ( pField
																							, SPH_FIELDF_TF_IDF ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_idf, sphinx_get_field_factor_float ( pField
																							 , SPH_FIELDF_MIN_IDF ) );
				ASSERT_EQ (
					tUnpacked.field[iField].max_idf, sphinx_get_field_factor_float ( pField, SPH_FIELDF_MAX_IDF ) );
				ASSERT_EQ (
					tUnpacked.field[iField].sum_idf, sphinx_get_field_factor_float ( pField, SPH_FIELDF_SUM_IDF ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_hit_pos, sphinx_get_field_factor_int ( pField
																							   , SPH_FIELDF_MIN_HIT_POS ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_best_span_pos, sphinx_get_field_factor_int ( pField
																									 , SPH_FIELDF_MIN_BEST_SPAN_POS ) );
				ASSERT_EQ ( tUnpacked.field[iField].max_window_hits, sphinx_get_field_factor_int ( pField
																								   , SPH_FIELDF_MAX_WINDOW_HITS ) );
				ASSERT_EQ (
					tUnpacked.field[iField].min_gaps, sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_GAPS ) );
				ASSERT_EQ ( tUnpacked.field[iField].atc, sphinx_get_field_factor_float ( pField, SPH_FIELDF_ATC ) );
				ASSERT_EQ ( tUnpacked.field[iField].lccs, sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCCS ) );
				ASSERT_EQ ( tUnpacked.field[iField].wlccs, sphinx_get_field_factor_float ( pField, SPH_FIELDF_WLCCS ) );
				bool bExactHitSame = ( ( ( tUnpacked.field[iField].exact_hit << iField )
					& sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_EXACT_HIT_MASK ) )!=0 );
				ASSERT_TRUE ( tUnpacked.field[iField].exact_hit==0 || bExactHitSame );
				bool bExactOrderSame = ( ( ( tUnpacked.field[iField].exact_order << iField )
					& sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_EXACT_ORDER_MASK ) )!=0 );
				ASSERT_TRUE ( tUnpacked.field[iField].exact_order==0 || bExactOrderSame );
			}

			// term level factors
			for ( int iWord = 0; iWord<tUnpacked.max_uniq_qpos; iWord++ )
			{
				if ( !tUnpacked.term[iWord].keyword_mask )
					continue;

				const unsigned int * pTerm = sphinx_get_term_factors ( pFactors, iWord + 1 );
				ASSERT_TRUE ( pTerm );
				ASSERT_EQ ( tUnpacked.term[iWord].tf, sphinx_get_term_factor_int ( pTerm, SPH_TERMF_TF ) );
				ASSERT_EQ ( tUnpacked.term[iWord].idf, sphinx_get_term_factor_float ( pTerm, SPH_TERMF_IDF ) );
			}

			sphinx_factors_deinit ( &tUnpacked );

			SafeDelete ( pSorter );
		}
	}

	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
	});
}


TEST_F ( RT, SendVsMerge )
{
	using namespace testing;
	Threads::CallCoroutine ( [&] {

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", false, 32, nullptr, sError );

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "tag1";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "tag2";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	auto pSrc = new MockDocRandomizer_c ( tSrcSchema );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( 801 ).WillRepeatedly ( Return ( pSrc->m_dFieldLengths ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( CSphSourceSettings(), nullptr );
	ASSERT_TRUE ( pSrc->Connect ( sError ) );
	ASSERT_TRUE ( pSrc->IterateStart ( sError ) );

	ASSERT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( "testrt", RT_INDEX_FILE_NAME, tSchema, 128 * 1024 );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	ASSERT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	CSphQuery tQuery;
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tQuery.m_sQuery = "@title cat";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	CSphQueryItem & tItem = tQuery.m_dItems.Add ();
	tItem.m_sExpr = "*";
	tItem.m_sAlias = "*";
	tQuery.m_sSelect = "*";

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	tQueueSettings.m_bComputeItems = true;
	SphQueueRes_t tRes;
	auto pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
	ASSERT_TRUE ( pSorter );

	CSphString sFilter;
	InsertDocData_c tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;

	bool bEOF = false;
	while (true)
	{
		ASSERT_TRUE ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
		sError = ""; // need to reset error message
		if ( pSrc->m_iDocsCounter==350 )
		{
			pIndex->Commit ( NULL, &tAcc );
			EXPECT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
			auto & tOneRes = tResult.m_dResults.Add ();
			tOneRes.FillFromSorter ( pSorter );
		}
	}
	pIndex->Commit ( NULL, &tAcc );

	pSrc->Disconnect ();

	tResult.m_tSchema = *pSorter->GetSchema ();

	auto & tOneRes = tResult.m_dResults.First ();
	ASSERT_EQ ( tResult.GetLength (), 20 );
	for ( int i = 0; i<tResult.GetLength (); ++i )
	{
		const RowID_t uID = tOneRes.m_dMatches[i].m_tRowID;
		const SphAttr_t tTag1 = tOneRes.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 0 ).m_tLocator );
		const SphAttr_t tTag2 = tOneRes.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 1 ).m_tLocator );
		ASSERT_TRUE ( ( RowID_t ) tTag1==uID + 1000 );
		ASSERT_TRUE ( tTag2==1313 );
	}

	SafeDelete ( pSorter );
	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
	});
}
