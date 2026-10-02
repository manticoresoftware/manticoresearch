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

#include "accumulator.h"
#include "sphinxrt.h"
#include "columnarmisc.h"
#include "coroutine.h"
#include "memio.h"
#include "tracer.h"
#include "indexsettings.h"
#include "rt_field_norms.h"

#include <limits>
#include <memory>

RtAccum_t::~RtAccum_t()
{
	ResetUuidLeases();
}

std::unique_ptr<ReplicationCommand_t> MakeReplicationCommand ( ReplCmd_e eCommand, CSphString sIndex, CSphString sCluster )
{
	auto pCmd = std::make_unique<ReplicationCommand_t>();
	pCmd->m_eCommand = eCommand;
	pCmd->m_sCluster = std::move ( sCluster );
	pCmd->m_sIndex = std::move ( sIndex );
	return pCmd;
}

ReplicationCommand_t* RtAccum_t::AddCommand ( ReplCmd_e eCmd, CSphString sIndex, CSphString sCluster )
{
	// all writes to RT index go as single command to serialize accumulator
	if ( eCmd == ReplCmd_e::RT_TRX && !m_dCmd.IsEmpty() && m_dCmd.Last()->m_eCommand == ReplCmd_e::RT_TRX )
		return m_dCmd.Last().get();

	m_dCmd.Add ( MakeReplicationCommand ( eCmd, std::move ( sIndex ), std::move ( sCluster ) ) );
	return m_dCmd.Last().get();
}

void RtAccum_t::SetupDict ( const RtIndex_i* pIndex, const DictRefPtr_c& pDict, DictFormat_e eDictFormat )
{
	if ( pIndex == m_pIndex && pDict.Ptr() == m_pRefDict && eDictFormat == m_eDictFormat )
		return;

	m_eDictFormat = eDictFormat;
	m_pRefDict = pDict.Ptr();
	m_pDict = GetStatelessDict ( pDict );
	if ( IsKeywordDict() )
	{
		m_pDict = m_pDictRt = sphCreateRtKeywordsDictionaryWrapper ( m_pDict, pIndex->NeedStoreWordID() );
	}
}

void RtAccum_t::ResetDict()
{
	assert ( !IsKeywordDict() || m_pDictRt );
	if ( m_pDictRt )
		m_pDictRt->ResetKeywords();

	m_dPackedKeywords.Reset ( 0 );
}

const BYTE* RtAccum_t::GetPackedKeywords() const
{
	return m_dPackedKeywords.IsEmpty() ? m_pDictRt->GetPackedKeywords() : m_dPackedKeywords.begin();
}

int RtAccum_t::GetPackedLen() const
{
	return m_dPackedKeywords.IsEmpty() ? m_pDictRt->GetPackedLen() : m_dPackedKeywords.GetLength();
}

static inline bool IsEarlierHit ( const CSphWordHit & a, const CSphWordHit & b, int iWordCmp )
{
	return ( iWordCmp < 0 )
		|| ( iWordCmp == 0 && a.m_tRowID < b.m_tRowID )
		|| ( iWordCmp == 0 && a.m_tRowID == b.m_tRowID && HITMAN::GetPosWithField ( a.m_uWordPos ) < HITMAN::GetPosWithField ( b.m_uWordPos ) );
}


void RtAccum_t::Sort()
{
	TRACE_CONN ( "conn", "RtAccum_t::Sort" );
	switch ( m_eDictFormat )
	{
	case DictFormat_e::CRC:
		m_dAccum.Sort ( Lesser ( [] ( const CSphWordHit& a, const CSphWordHit& b )
		{
			return 	( a.m_uWordID<b.m_uWordID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_uWordPos )<HITMAN::GetPosWithField ( b.m_uWordPos ) );
		}));
		return;

	case DictFormat_e::KEYWORDS_V2:
		assert ( m_pDictRt );
		{
			const BYTE * pPackedKeywords = GetPackedKeywords();
			m_dAccum.Sort ( Lesser ( [pPackedKeywords] ( const CSphWordHit & a, const CSphWordHit & b )
			{
				ByteBlob_t tA = GetPackedKeywordV2 ( pPackedKeywords + a.m_uWordID );
				ByteBlob_t tB = GetPackedKeywordV2 ( pPackedKeywords + b.m_uWordID );
				int iCmp = sphDictCmpStrictly ( (const char *)tA.first, tA.second, (const char *)tB.first, tB.second );
				return IsEarlierHit ( a, b, iCmp );
			}));
		}
		return;

	case DictFormat_e::KEYWORDS:
		assert ( m_pDictRt );
		{
			const BYTE * pPackedKeywords = GetPackedKeywords();
			m_dAccum.Sort ( Lesser ( [pPackedKeywords] ( const CSphWordHit & a, const CSphWordHit & b )
			{
				ByteBlob_t tA = GetPackedKeywordLegacy ( pPackedKeywords + a.m_uWordID );
				ByteBlob_t tB = GetPackedKeywordLegacy ( pPackedKeywords + b.m_uWordID );
				int iCmp = sphDictCmpStrictly ( (const char *)tA.first, tA.second, (const char *)tB.first, tB.second );
				return IsEarlierHit ( a, b, iCmp );
			}));
		}
		return;
	}

	assert ( 0 && "unknown dict format" );
}


static bool RepackBlob ( const CSphColumnInfo & tAttr, const CSphColumnInfo & tBlobLoc, int iBlobAttr, const CSphRowitem * pRow, const BYTE * pBlobPool, std::unique_ptr<BlobRowBuilder_i> & pBlobWriter, CSphString & sError )
{
	SphAttr_t tBlobRowOffset = sphGetRowAttr ( pRow, tBlobLoc.m_tLocator );
	const BYTE * pBlobRow = pBlobPool + tBlobRowOffset;
	ByteBlob_t tBlob = sphGetBlobAttr ( pBlobRow, tAttr.m_tLocator );

	BlobAttrInput_e eInput = BlobAttrInput_e::RAW_BYTES;
	if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR || tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR_ARRAY )
		eInput = BlobAttrInput_e::MVA_DWORD;
	else if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
		eInput = BlobAttrInput_e::MVA_INT64;

	return pBlobWriter->SetAttr ( iBlobAttr, tBlob.first, tBlob.second, eInput, sError );
}


static bool StoreEmbeddings ( const CSphSchema & tSchema, int iAttr, int iBlobAttr, int iColumnarAttr, DocstoreDoc_t & tDoc, std::unique_ptr<BlobRowBuilder_i> & pNewBlobBuilder, std::unique_ptr<ColumnarBuilderRT_i> & pNewColumnarBuilder, const IntVec_t & dDocstoreRemap, const std::vector<std::vector<float>> & dEmbeddings, size_t iFrom, size_t iTo, CSphVector<int64_t> & dTmp, CSphString & sError )
{
	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);

	dTmp.Resize(0);

	const int iExpectedDims = tAttr.m_tKNN.m_iDims;
	if ( iExpectedDims>0 )
		for ( size_t i = iFrom; i < iTo; i++ )
			if ( (int)dEmbeddings[i].size()!=iExpectedDims )
			{
				sError.SetSprintf ( "attribute '%s': model returned a %d-value vector, index needs %d values", tAttr.m_sName.cstr(), (int)dEmbeddings[i].size(), iExpectedDims );
				return false;
			}

	if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR_ARRAY )
	{
		// [dims][N*dims floats]. An empty range is a legal "no vectors" value
		if ( iTo>iFrom )
		{
			const int iDims = (int)dEmbeddings[iFrom].size();
			if ( !iDims )
			{
				sError.SetSprintf ( "attribute '%s': model returned an empty vector", tAttr.m_sName.cstr() );
				return false;
			}

			dTmp.Add(iDims);
			for ( size_t i = iFrom; i < iTo; i++ )
			{
				const std::vector<float> & dVec = dEmbeddings[i];
				if ( (int)dVec.size()!=iDims )
				{
					sError.SetSprintf ( "attribute '%s': model returned vectors of different widths (%d and %d) for one document", tAttr.m_sName.cstr(), iDims, (int)dVec.size() );
					return false;
				}

				for ( float fVal : dVec )
					dTmp.Add ( sphF2DW(fVal) );
			}
		}
	}
	else
	{
		assert ( iTo-iFrom<=1 );
		if ( iTo>iFrom )
			for ( float fVal : dEmbeddings[iFrom] )
				dTmp.Add ( sphF2DW(fVal) );
	}

	if ( tAttr.IsColumnar() )
		pNewColumnarBuilder->SetAttr ( iColumnarAttr, dTmp.Begin(), dTmp.GetLength() );
	else
	{
		if ( !pNewBlobBuilder->SetAttr ( iBlobAttr, (const BYTE*)dTmp.Begin(), dTmp.GetLengthBytes(), BlobAttrInput_e::MVA_INT64, sError ) )
			return false;
	}

	if ( tAttr.IsStored() )
	{
		int iId = dDocstoreRemap[iAttr];
		tDoc.m_dFields[iId].Resize ( dTmp.GetLength()*sizeof(DWORD) );
		BYTE * pStart = tDoc.m_dFields[iId].Begin();
		for ( auto i : dTmp )
		{
			*(DWORD*)pStart = (DWORD)i;
			pStart += sizeof(DWORD);
		}
	}

	return true;
}


bool RtAccum_t::RebuildStoragesForEmbeddings ( RowID_t tRowID, CSphRowitem * pRow, const CSphVector<AttrWithModel_t> & dAttrsWithModels, std::unique_ptr<BlobRowBuilder_i> & pNewBlobBuilder, std::unique_ptr<ColumnarBuilderRT_i> & pNewColumnarBuilder, std::unique_ptr<DocstoreRT_i> & pNewDocstoreBuilder, CSphVector<ScopedTypedIterator_t> & dAllIterators, const IntVec_t & dDocstoreRemap, const CSphColumnInfo * pBlobLoc, std::vector<std::vector<std::vector<float>>> & dAllEmbeddings, std::vector<std::vector<size_t>> & dAllOffsets, CSphVector<int64_t> & dTmp, CSphString & sError )
{
	int iBlobAttr = 0;
	int iColumnarAttr = 0;
	int iAttrWithModel = 0;
	DocstoreDoc_t tDoc;

	// fetch all fields and attributes without model from docstore (they will not be modified)
	if ( pNewDocstoreBuilder )
		tDoc = m_pDocstore->GetDoc ( tRowID, nullptr, -1, false );

	assert(m_pIndex);
	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();

	ARRAY_FOREACH ( i, dAttrsWithModels )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		const AttrWithModel_t & tAttrWithModel = dAttrsWithModels[i];
		bool bStoreGenerated = false;

		if ( tAttrWithModel.m_pModel )
		{
			assert ( m_pEmbeddingsSrc );
			bStoreGenerated = m_pEmbeddingsSrc->IsDefault ( tRowID, iAttrWithModel );
		}

		if ( bStoreGenerated )
		{
			const std::vector<size_t> & dOffsets = dAllOffsets[i];
			assert ( (size_t)tRowID+1 < dOffsets.size() );
			if ( !StoreEmbeddings ( tSchema, i, iBlobAttr, iColumnarAttr, tDoc, pNewBlobBuilder, pNewColumnarBuilder, dDocstoreRemap, dAllEmbeddings[i], dOffsets[tRowID], dOffsets[tRowID+1], dTmp, sError ) )
				return false;
		}
		else
		{
			if ( pNewBlobBuilder && !tAttr.IsColumnar() && sphIsBlobAttr(tAttr) )
			{
				assert(pBlobLoc);
				if ( !RepackBlob ( tAttr, *pBlobLoc, iBlobAttr, pRow, m_dBlobs.Begin(), pNewBlobBuilder, sError ) )
					return false;
			}

			if ( pNewColumnarBuilder && tAttr.IsColumnar() )
			{
				auto & tIt = dAllIterators[iColumnarAttr];
				SetColumnarAttr ( iColumnarAttr, tIt.second, pNewColumnarBuilder.get(), tIt.first, tRowID, dTmp );
			}
		}

		if ( sphIsBlobAttr(tAttr) )
			iBlobAttr++;

		if ( tAttr.IsColumnar() )
			iColumnarAttr++;

		if ( tAttrWithModel.m_pModel )
			iAttrWithModel++;
	}

	if ( pNewBlobBuilder )
	{
		assert(pBlobLoc);
		sphSetRowAttr ( pRow, pBlobLoc->m_tLocator, pNewBlobBuilder->Flush().first );
	}

	if ( pNewDocstoreBuilder )
		pNewDocstoreBuilder->AddDoc ( tRowID, tDoc );

	return true;
}


bool RtAccum_t::GenerateEmbeddings ( int iAttr, int iAttrWithModel, const CSphVector<AttrWithModel_t> & dAttrsWithModels, std::vector<std::vector<std::vector<float>>> & dAllEmbeddings, std::vector<std::vector<size_t>> & dAllOffsets, CSphString & sError )
{
	const AttrWithModel_t & tAttrWithModel = dAttrsWithModels[iAttr];
	if ( !tAttrWithModel.m_pModel )
		return true;

	assert(m_pIndex);
	const auto & tAttr = m_pIndex->GetInternalSchema().GetAttr(iAttr);

	std::vector<std::vector<float>> & dEmbeddingsForAttr = dAllEmbeddings[iAttr];
	std::vector<size_t> & dOffsetsForAttr = dAllOffsets[iAttr];
	std::vector<std::string_view> dTexts;
	DWORD uNumSkipped = 0;
	IntVec_t dResultIds(m_uAccumDocs);
	for ( RowID_t tRowID = 0; tRowID < m_uAccumDocs; ++tRowID )
	{
		if ( !m_pEmbeddingsSrc || !m_pEmbeddingsSrc->Has ( tRowID, iAttrWithModel ) )
		{
			sError.SetSprintf ( "Error generating embeddings for attribute '%s': missing source text in transaction", tAttr.m_sName.cstr() );
			return false;
		}

		bool bDefault = m_pEmbeddingsSrc->IsDefault ( tRowID, iAttrWithModel );

		if ( bDefault )
		{
			dResultIds[tRowID] = dTexts.size();
			const auto & dConcat = m_pEmbeddingsSrc->Get ( tRowID, iAttrWithModel );
			dTexts.push_back ( { dConcat.Begin(), (size_t)dConcat.GetLength() } );
		}
		else
		{
			dResultIds[tRowID] = -1;
			uNumSkipped++;
		}
	}

	std::string sErrorSTL;
	std::vector<std::vector<float>> dTmpEmbeddings;
	std::vector<size_t> dTmpOffsets;
	bool bConverted = true;
	if ( uNumSkipped!=m_uAccumDocs )
	{
		// a multi-vector strategy returns several vectors per text; the single-vector ones still report
		// offsets, they are just the identity. Asking for them always keeps one code path here
		auto fnConvert = [&]
		{
			return tAttrWithModel.m_pModel->Convert ( dTexts, dTmpEmbeddings, sErrorSTL, GetEmbeddingsThreadsToUse(), &tAttr.m_tKNNChunk, &dTmpOffsets );
		};

		if ( Threads::IsInsideCoroutine() )
			Threads::Coro::Continue ( Threads::GetMaxCoroStackSize(), [&] { bConverted = fnConvert(); } );
		else
			bConverted = fnConvert();
	}

	if ( !bConverted )
	{
		sError = sErrorSTL.c_str();
		return false;
	}

	if ( !dTexts.empty() )
	{
		bool bOk = dTmpOffsets.size()==dTexts.size()+1 && dTmpOffsets.front()==0 && dTmpOffsets.back()==dTmpEmbeddings.size();
		for ( size_t i = 1; bOk && i < dTmpOffsets.size(); i++ )
			bOk = dTmpOffsets[i]>=dTmpOffsets[i-1];

		if ( !bOk )
		{
			sError.SetSprintf ( "Error generating embeddings for attribute '%s': model returned %d vectors for %d input texts, grouped inconsistently", tAttr.m_sName.cstr(), (int)dTmpEmbeddings.size(), (int)dTexts.size() );
			return false;
		}
	}

	// flatten into row order: row r owns dEmbeddingsForAttr[ dOffsetsForAttr[r] .. dOffsetsForAttr[r+1] ).
	// a row that supplied its own vector keeps an empty range
	dEmbeddingsForAttr.clear();
	dOffsetsForAttr.resize ( (size_t)m_uAccumDocs+1 );
	dOffsetsForAttr[0] = 0;
	ARRAY_FOREACH ( i, dResultIds )
	{
		int iResultId = dResultIds[i];
		if ( iResultId!=-1 )
			for ( size_t v = dTmpOffsets[iResultId]; v < dTmpOffsets[iResultId+1]; v++ )
				dEmbeddingsForAttr.push_back ( std::move ( dTmpEmbeddings[v] ) );

		dOffsetsForAttr[i+1] = dEmbeddingsForAttr.size();
	}

	return true;
}


bool RtAccum_t::FetchEmbeddings ( TableEmbeddings_c * pEmbeddings, const CSphVector<AttrWithModel_t> & dAttrsWithModels, CSphString & sError )
{
	if ( !pEmbeddings )
		return true;

	assert(m_pIndex);
	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();
	assert ( dAttrsWithModels.GetLength()==tSchema.GetAttrsCount() ); // fix #4872

	// we need to rebuild our blobs/columnar storage (to replace empty data with fetched floatvectors), but they are immutable by design
	// so we'll have to rebuild the blobs/columnar storage fully
	const CSphColumnInfo * pBlobLoc = tSchema.GetAttr ( sphGetBlobLocatorName() );
	bool bRebuildColumnar = false;
	bool bRebuildBlobs = false;
	bool bRebuildDocstore = false;
	IntVec_t dDocstoreRemap;
	dDocstoreRemap.Resize ( dAttrsWithModels.GetLength() );
	dDocstoreRemap.Fill(-1);
	int iNumColumnarAttrs = 0;
	ARRAY_FOREACH ( i, dAttrsWithModels )
	{
		auto & tAttr = tSchema.GetAttr(i);
		bool bColumnar = tAttr.IsColumnar();
		iNumColumnarAttrs += bColumnar;

		if ( !dAttrsWithModels[i].m_pModel )
			continue;

		assert ( tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR || tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR_ARRAY );
		bRebuildColumnar |= bColumnar;
		bRebuildBlobs |= !bColumnar;
		bRebuildDocstore |= tAttr.IsStored();

		dDocstoreRemap[i] = m_pDocstore ? ((DocstoreBuilder_i*)m_pDocstore.get())->GetFieldId ( tAttr.m_sName, DOCSTORE_ATTR ) : -1;
	}

	CSphTightVector<BYTE> dNewBlobPool;
	std::unique_ptr<BlobRowBuilder_i> pNewBlobBuilder = bRebuildBlobs ? sphCreateBlobRowBuilder ( tSchema, dNewBlobPool ) : nullptr;
	std::unique_ptr<ColumnarBuilderRT_i> pNewColumnarBuilder = bRebuildColumnar ? CreateColumnarBuilderRT(tSchema) : nullptr;
	std::unique_ptr<ColumnarRT_i> pColumnar = bRebuildColumnar ? CreateLightColumnarRT ( tSchema, m_pColumnarBuilder.get() ) : nullptr;
	CSphVector<ScopedTypedIterator_t> dAllIterators;
	if ( bRebuildColumnar )
		dAllIterators = CreateAllColumnarIterators ( pColumnar.get(), tSchema );
	else
		dAllIterators.Resize(iNumColumnarAttrs);

	std::unique_ptr<DocstoreRT_i> pNewDocstoreBuilder;
	if ( bRebuildDocstore )
	{
		pNewDocstoreBuilder = CreateDocstoreRT();
		SetupDocstoreFields ( *pNewDocstoreBuilder, tSchema );
	}

	// 1st pass - generate all embeddings for each attribute
	int iRowSize = tSchema.GetRowSize();
	int iAttrWithModel = 0;
	std::vector<std::vector<std::vector<float>>> dAllEmbeddings;
	std::vector<std::vector<size_t>> dAllOffsets;
	dAllEmbeddings.resize ( dAttrsWithModels.GetLength() );
	dAllOffsets.resize ( dAttrsWithModels.GetLength() );
	ARRAY_FOREACH ( i, dAttrsWithModels )
	{
		if ( !GenerateEmbeddings( i, iAttrWithModel, dAttrsWithModels, dAllEmbeddings, dAllOffsets, sError ) )
			return false;

		if ( dAttrsWithModels[i].m_pModel )
			iAttrWithModel++;
	}

	// 2nd pass - rebuild attribute storages
	std::vector<float> dEmbedding;
	CSphVector<int64_t> dTmp;
	CSphRowitem * pRow = m_dAccumRows.Begin();
	for ( RowID_t tRowID = 0; tRowID < m_uAccumDocs; ++tRowID, pRow += iRowSize )
		if ( !RebuildStoragesForEmbeddings ( tRowID, pRow, dAttrsWithModels, pNewBlobBuilder, pNewColumnarBuilder, pNewDocstoreBuilder, dAllIterators, dDocstoreRemap, pBlobLoc, dAllEmbeddings, dAllOffsets, dTmp, sError ) )
			return false;

	if ( bRebuildBlobs )
	{
		m_pBlobWriter = std::move(pNewBlobBuilder);
		m_dBlobs.SwapData(dNewBlobPool);
	}

	if ( bRebuildColumnar )
		m_pColumnarBuilder = std::move(pNewColumnarBuilder);

	if ( bRebuildDocstore )
		m_pDocstore = std::move(pNewDocstoreBuilder);

	return true;
}


void RtAccum_t::CleanupPart()
{
	ResetPreparedForCommit();
	m_dAccumRows.Resize ( 0 );
	m_dNorms.Resize ( 0 );
	m_dBlobs.Resize ( 0 );
	m_pColumnarBuilder.reset();
	m_dPerDocHitsCount.Resize ( 0 );
	m_dAccum.Resize ( 0 );
	m_pDocstore.reset();
	m_pEmbeddingsSrc.reset();

	ResetDict();
	ResetRowID();
}

void RtAccum_t::Cleanup()
{
	CleanupPart();

	m_pIndex = nullptr;
	m_pBlobWriter.reset();
	m_uAccumDocs = 0;
	m_iAccumBytes = 0;
	m_dAccumKlist.Reset();
	m_sIndexName = CSphString();
	m_iIndexId = 0;
	m_uSchemaHash = 0;
	m_iIndexGeneration = 0;
	m_bReplicationValidation = false;
	m_eValidationDictFormat = DictFormat_e::CRC;

	m_dCmd.Reset();
	ResetUuidLeases();
}

void RtAccum_t::CleanReplicated()
{
	m_tCmdReplicated = ReplicatedCommand_t();
}

void RtAccum_t::SetupDocstore()
{
	if ( m_pDocstore )
		return;

	m_pDocstore = CreateDocstoreRT();
	assert ( m_pDocstore );
	SetupDocstoreFields ( *m_pDocstore, m_pIndex->GetInternalSchema() );
}

bool RtAccum_t::SetupDocstore ( const RtIndex_i& tIndex, CSphString& sError )
{
	const CSphSchema& tSchema = tIndex.GetInternalSchema();
	if ( !m_pDocstore && !tSchema.HasStoredFields() && !tSchema.HasStoredAttrs() )
		return true;

	// might be a case when replicated trx was wo docstore but index has docstore
	if ( !m_pDocstore )
		m_pDocstore = CreateDocstoreRT();

	assert ( m_pDocstore );
	SetupDocstoreFields ( *m_pDocstore, tSchema );
	return m_pDocstore->CheckFieldsLoaded ( sError );
}


[[nodiscard]] bool RtAccum_t::IsClusterCommand() const noexcept
{
	return ( m_dCmd.GetLength () && !m_dCmd[0]->m_sCluster.IsEmpty () );
}


[[nodiscard]] bool RtAccum_t::IsRtTrxCommand() const noexcept
{
	return ( m_dCmd.GetLength () && m_dCmd[0]->m_eCommand==ReplCmd_e::RT_TRX );
}


[[nodiscard]] bool RtAccum_t::IsUpdateCommand() const noexcept
{
	return ( m_dCmd.GetLength () &&
			( m_dCmd[0]->m_eCommand==ReplCmd_e::UPDATE_API
					|| m_dCmd[0]->m_eCommand==ReplCmd_e::UPDATE_QL
					|| m_dCmd[0]->m_eCommand==ReplCmd_e::UPDATE_JSON ) );
}

static void ResetTailHit ( CSphWordHit * pHit )
{
	if ( pHit->m_tRowID!=pHit[1].m_tRowID || pHit->m_uWordID!=pHit[1].m_uWordID )
		return;

	if ( HITMAN::GetField ( pHit->m_uWordPos )==HITMAN::GetField ( pHit[1].m_uWordPos ) && HITMAN::IsEnd ( pHit[1].m_uWordPos ) )
		pHit->m_uWordPos = HITMAN::GetPosWithField ( pHit->m_uWordPos );
}


static const char * FetchStringFromDoc ( int iAttr, const InsertDocData_c & tDoc, const ISphSchema & tSchema )
{
	const char ** ppStr = tDoc.m_dStrings.Begin();
	int iStrAttr = 0;

	for ( int i=0; i < tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		if ( tAttr.m_eAttrType!=SPH_ATTR_STRING )
			continue;

		if ( iAttr==i )
			return ppStr[iStrAttr];

		iStrAttr++;
	}

	return nullptr;
}


static CSphVector<char> ConcatFromFields ( const InsertDocData_c & tDoc, const AttrWithModel_t & tAttr, const ISphSchema & tSchema )
{
	CSphVector<char> dTmp;
	ARRAY_FOREACH ( i, tAttr.m_dFrom )
	{
		auto tFrom = tAttr.m_dFrom[i];
		int iAttrFieldId = tFrom.first;
		VecTraits_T<const char> dSrc;
		if ( tFrom.second )
			dSrc = { tDoc.m_dFields[iAttrFieldId].Begin(), tDoc.m_dFields[iAttrFieldId].GetLength() };
		else
		{
			const char * szString = FetchStringFromDoc ( iAttrFieldId, tDoc, tSchema );
			dSrc = { szString, (int64_t)( szString ? strlen(szString) : 0 ) };
		}

		int iOldSize = dTmp.GetLength();
		int iOffset = iOldSize + ( iOldSize ? 1 : 0 );
		dTmp.Resize ( dSrc.GetLength() + iOffset );
		if ( iOldSize )
			dTmp[iOldSize] = ' ';

		memcpy ( dTmp.Begin() + iOffset, dSrc.Begin(), dSrc.GetLength() );
	}

	return dTmp;
}


static bool IsDefaultEmbedding ( const InsertDocData_c & tDoc, const CSphColumnInfo & tAttr, int & iMva )
{
	// float_vector shares the same compact storage stream as MVAs in InsertDocData_c,
	// so we must advance the cursor (iMva) for every MVA-backed schema attr to keep it aligned.
	if ( !IsMvaAttr ( tAttr.m_eAttrType ) )
		return false;

	int iNumValues = 0;
	bool bDefault = false;
	const int64_t * pMva = tDoc.GetMVA ( iMva );
	std::tie ( iNumValues, bDefault ) = tDoc.ReadMVALength ( pMva );
	iMva += iNumValues + 1;
	return bDefault;
}


void RtAccum_t::FetchEmbeddingsSrc ( InsertDocData_c & tDoc, const CSphVector<AttrWithModel_t> & dAttrsWithModels )
{
	if ( !m_pEmbeddingsSrc )
	{
		int iAttrsWithModels = dAttrsWithModels.count_of ( []( auto & tData ){ return !!tData.m_pModel; } );
		m_pEmbeddingsSrc = std::make_unique<EmbeddingsSrc_c>(iAttrsWithModels);
	}

	assert(m_pIndex);
	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();

	int iAttrWithModel = 0;
	int iMva = 0;
	ARRAY_FOREACH ( i, dAttrsWithModels )
	{
		bool bDefault = IsDefaultEmbedding ( tDoc, tSchema.GetAttr(i), iMva );

		const AttrWithModel_t & tAttrWithModel = dAttrsWithModels[i];
		if ( !tAttrWithModel.m_pModel )
			continue;

		auto dConcat = ConcatFromFields ( tDoc, tAttrWithModel, tSchema );
		m_pEmbeddingsSrc->Add ( iAttrWithModel, dConcat, bDefault );
		iAttrWithModel++;
	}
}


static DWORD * SetupFieldLengths ( const CSphSchema & tSchema, CSphVector<DWORD> & dFieldLengths )
{
	dFieldLengths.Resize ( tSchema.GetFieldsCount() );
	dFieldLengths.ZeroVec();
	return dFieldLengths.Begin();
}


static const DocstoreBuilder_i::Doc_t * StoreFieldLengths ( CSphRowitem * pRow, std::unique_ptr<ColumnarBuilderRT_i> & pColumnarBuilder, const CSphVector<DWORD> & dFieldLengths, const CSphSchema & tSchema, DocstoreBuilder_i::Doc_t & dUpdatedStoredDoc, const DocstoreBuilder_i::Doc_t * pStoredDoc )
{
	const DocstoreBuilder_i::Doc_t * pUpdatedStoredDoc = pStoredDoc;

	int iNumStoredAttrs = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
		if ( tSchema.GetAttr(i).IsStored() )
			iNumStoredAttrs++;

	bool bInitialized = false;
	int iColumnar = 0;
	int iStored = 0;
	int iFirstFieldLen = tSchema.GetAttrId_FirstFieldLen();	
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		if ( i>=iFirstFieldLen && i<=tSchema.GetAttrId_LastFieldLen() )
		{
			assert ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT );
			DWORD & uData = dFieldLengths[i-iFirstFieldLen];
			if ( tAttr.IsColumnar() )
				pColumnarBuilder->SetAttr ( iColumnar, uData );
			else
				sphSetRowAttr ( pRow, tAttr.m_tLocator, uData );

			if ( tAttr.IsStored() )
			{
				if ( !bInitialized )
				{
					dUpdatedStoredDoc = *pStoredDoc;
					pUpdatedStoredDoc = &dUpdatedStoredDoc;
					bInitialized = true;
				}

				// we assume that stored attrs come last in the schema
				int iTotalStored = dUpdatedStoredDoc.m_dFields.GetLength();
				dUpdatedStoredDoc.m_dFields[iTotalStored - iNumStoredAttrs + iStored] = { (BYTE*)&uData, sizeof(DWORD) };
			}
		}

		if ( tAttr.IsColumnar() )
			iColumnar++;

		if ( tAttr.IsStored() )
			iStored++;
	}

	return pUpdatedStoredDoc;
}


void RtAccum_t::AddDocument ( ISphHits * pHits, const InsertDocData_c & tDoc, bool bReplace, int iRowSize, const DocstoreBuilder_i::Doc_t * pStoredDoc, const DWORD * pExactFieldLengths )
{
	MEMORY ( MEM_RT_ACCUM );
	ResetPreparedForCommit();

	// FIXME? what happens on mixed insert/replace?
	m_bReplace = bReplace;

	DocID_t tDocID = tDoc.GetID();

	// schedule existing copies for deletion
	m_dAccumKlist.Add ( tDocID );

	// reserve some hit space on first use
	if ( pHits && pHits->GetLength() && !m_dAccum.GetLength() )
		m_dAccum.Reserve ( 128 * 1024 );

	// accumulate row data; expect fully dynamic rows
	assert ( !tDoc.m_tDoc.m_pStatic );
	assert ( !( !tDoc.m_tDoc.m_pDynamic && iRowSize != 0 ) );
	assert ( !( tDoc.m_tDoc.m_pDynamic && (int)tDoc.m_tDoc.m_pDynamic[-1] != iRowSize ) );

	CSphRowitem* pRow = nullptr;
	if ( iRowSize )
	{
		m_dAccumRows.Append ( tDoc.m_tDoc.m_pDynamic, iRowSize );
		pRow = &m_dAccumRows[m_dAccumRows.GetLength() - iRowSize];
	}

	CSphString sError;

	int iStrAttr = 0;
	int iBlobAttr = 0;
	int iColumnarAttr = 0;
	int iMva = 0;

	const char** ppStr = tDoc.m_dStrings.Begin();
	const CSphSchema& tSchema = m_pIndex->GetInternalSchema();
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tColumn = tSchema.GetAttr(i);

		switch ( tColumn.m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:
			{
				const BYTE* pStr = ppStr ? (const BYTE*)ppStr[iStrAttr++] : nullptr;
				ByteBlob_t dStr;
				if ( tColumn.m_eAttrType == SPH_ATTR_STRING )
					dStr = { pStr, pStr ? (int)strlen ( (const char*)pStr ) : 0 };
				else // SPH_ATTR_JSON - packed len + data
					dStr = sphUnpackPtrAttr ( pStr );

				if ( tColumn.IsColumnar() )
					m_pColumnarBuilder->SetAttr ( iColumnarAttr, dStr.first, dStr.second );
				else
					m_pBlobWriter->SetAttr ( iBlobAttr, dStr.first, dStr.second, BlobAttrInput_e::RAW_BYTES, sError );
			}
			break;

		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_FLOAT_VECTOR:
		case SPH_ATTR_FLOAT_VECTOR_ARRAY:
			{
				int iNumValues = 0;
				bool bDefault = false;
				const int64_t * pMva = tDoc.GetMVA(iMva);
				std::tie ( iNumValues, bDefault ) = tDoc.ReadMVALength(pMva);
				iMva += iNumValues + 1;

				if ( tColumn.IsColumnar() )
					m_pColumnarBuilder->SetAttr ( iColumnarAttr, pMva, iNumValues );
				else
					m_pBlobWriter->SetAttr ( iBlobAttr, (const BYTE*)pMva, iNumValues * sizeof ( int64_t ), BlobAttrInput_e::MVA_INT64, sError );
			}
			break;

		case SPH_ATTR_TOKENCOUNT:
			break; // skip for now, will be set later

		default:
			if ( tColumn.IsColumnar() )
				m_pColumnarBuilder->SetAttr ( iColumnarAttr, tDoc.m_dColumnarAttrs[iColumnarAttr] );

			break;
		}

		if ( tColumn.IsColumnar() )
			++iColumnarAttr;
		else if ( sphIsBlobAttr ( tColumn ) )
			++iBlobAttr;
	}

	if ( m_pBlobWriter )
	{
		const CSphColumnInfo* pBlobLoc = tSchema.GetAttr ( sphGetBlobLocatorName() );
		assert ( pBlobLoc );

		sphSetRowAttr ( pRow, pBlobLoc->m_tLocator, m_pBlobWriter->Flush().first );
	}

	CSphVector<DWORD> dFieldLengths;
	DWORD * pFieldLengths = pExactFieldLengths ? nullptr : SetupFieldLengths ( tSchema, dFieldLengths );

	// accumulate hits
	int iHits = 0;
	if ( pHits && !pHits->IsEmpty() )
	{
		CSphWordHit tLastHit;
		tLastHit.m_tRowID = INVALID_ROWID;
		tLastHit.m_uWordID = 0;
		tLastHit.m_uWordPos = 0;

		Hitpos_t uFieldLastHit = pHits->Begin()->m_uWordPos;
		DWORD uFieldLastCount = 1;

		m_dAccum.ReserveGap ( pHits->GetLength() );
		iHits = 0;
		for ( CSphWordHit* pHit = pHits->Begin(); pHit < pHits->End(); ++pHit )
		{
			// ignore duplicate hits
			if ( *pHit == tLastHit )
				continue;

			// update field lengths
			if ( pFieldLengths )
			{
				if ( HITMAN::GetField ( uFieldLastHit ) != HITMAN::GetField ( pHit->m_uWordPos ) )
				{
					pFieldLengths[HITMAN::GetField ( uFieldLastHit )] += uFieldLastCount;
					uFieldLastCount = 1;
					uFieldLastHit = pHit->m_uWordPos;
				}

				// skip blended part, lemmas and duplicates
				if ( HITMAN::GetPos ( pHit->m_uWordPos ) > HITMAN::GetPos ( uFieldLastHit ) )
				{
					uFieldLastHit = pHit->m_uWordPos;
					uFieldLastCount++;
				}
			}

			// need original hit for duplicate removal
			tLastHit = *pHit;
			// reset field end for not very last position in this field
			if ( HITMAN::IsEnd ( pHit->m_uWordPos ) && pHit!=&pHits->Last() )
				ResetTailHit ( pHit );

			// accumulate
			m_dAccum.Add ( *pHit );
			++iHits;
		}

		if ( pFieldLengths && uFieldLastCount )
			pFieldLengths [ HITMAN::GetField(uFieldLastHit) ] += uFieldLastCount;
	}
	DocstoreBuilder_i::Doc_t dUpdatedStoredDoc;
	if ( pExactFieldLengths )
	{
		const int iFields = tSchema.GetFieldsCount();
		if ( m_pIndex->GetSettings().m_bIndexFieldLens )
		{
			dFieldLengths.Append ( pExactFieldLengths, iFields );
			pStoredDoc = StoreFieldLengths ( pRow, m_pColumnarBuilder, dFieldLengths, tSchema, dUpdatedStoredDoc, pStoredDoc );
		}
	}
	else
	{
		if ( m_pIndex->GetSettings().m_bIndexFieldLens )
			pStoredDoc = StoreFieldLengths ( pRow, m_pColumnarBuilder, dFieldLengths, tSchema, dUpdatedStoredDoc, pStoredDoc );
	}
	const DWORD * pSchemaNorms = pExactFieldLengths ? pExactFieldLengths : dFieldLengths.Begin();
	for ( int iField=0; iField<tSchema.GetFieldsCount(); ++iField )
		if ( tSchema.GetField(iField).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
			m_dNorms.Add ( pSchemaNorms[iField] );

	// make sure to get real count without duplicated hits
	m_dPerDocHitsCount.Add ( iHits );

	if ( pStoredDoc )
	{
		SetupDocstore();
		m_pDocstore->AddDoc ( m_uAccumDocs, *pStoredDoc );
	}

	++m_uAccumDocs;
	m_iAccumBytes += tDoc.m_iTotalBytes;
}

struct AccumDocHits_t
{
	DocID_t m_tDocID;
	int m_iDocIndex;
//	int m_iHitIndex;
//	int m_iHitCount;
};


void RtAccum_t::CleanupDuplicates ( int iRowSize )
{
	TRACE_CONN ( "conn", "RtAccum_t::CleanupDuplicates" );

	if ( m_uAccumDocs <= 1 )
		return;

	assert ( m_uAccumDocs == (DWORD)m_dPerDocHitsCount.GetLength() );
	CSphVector<AccumDocHits_t> dDocHits ( m_dPerDocHitsCount.GetLength() );

	assert ( m_pIndex );
	const CSphSchema& tSchema = m_pIndex->GetInternalSchema();
	bool bColumnarId = tSchema.GetAttr ( 0 ).IsColumnar();

	{
		// create temporary columnar accessor; don't take ownership of built attributes
		auto pColumnar = CreateLightColumnarRT ( m_pIndex->GetInternalSchema(), m_pColumnarBuilder.get() );

		std::string sError;
		std::unique_ptr<columnar::Iterator_i> pColumnarIdIterator;
		if ( bColumnarId )
		{
			pColumnarIdIterator = CreateColumnarIterator ( pColumnar.get(), sphGetDocidName(), sError );
			assert ( pColumnarIdIterator );
		}

	//	int iHitIndex = 0;
		CSphRowitem* pRow = m_dAccumRows.Begin();
		for ( DWORD i = 0; i < m_uAccumDocs; ++i, pRow += iRowSize )
		{
			AccumDocHits_t& tElem = dDocHits[i];
			if ( !bColumnarId )
				tElem.m_tDocID = sphGetDocID ( pRow );
			else
				tElem.m_tDocID = pColumnarIdIterator->Get(i);

			tElem.m_iDocIndex = i;
	//		tElem.m_iHitIndex = iHitIndex;
	//		tElem.m_iHitCount = m_dPerDocHitsCount[i];
	//		iHitIndex += m_dPerDocHitsCount[i];
		}
	}

	dDocHits.Sort ( Lesser ( [] ( const AccumDocHits_t& a, const AccumDocHits_t& b )
		{
			return ( a.m_tDocID < b.m_tDocID || ( a.m_tDocID == b.m_tDocID && a.m_iDocIndex < b.m_iDocIndex ) );
		}));

	DocID_t uPrev = 0;
	if ( !dDocHits.any_of ( [&] ( const AccumDocHits_t& dDoc ) {
			 bool bRes = dDoc.m_tDocID == uPrev;
			 uPrev = dDoc.m_tDocID;
			 return bRes;
		 } ) )
		return;

	CSphFixedVector<RowID_t> dRowMap ( m_uAccumDocs );
	for ( auto& i : dRowMap )
		i = 0;

	// identify duplicates to kill
	if ( m_bReplace )
	{
		// replace mode, last value wins, precending values are duplicate
		for ( DWORD i = 0; i < m_uAccumDocs - 1; ++i )
			if ( dDocHits[i].m_tDocID == dDocHits[i + 1].m_tDocID )
				dRowMap[dDocHits[i].m_iDocIndex] = INVALID_ROWID;
	} else
	{
		// insert mode, first value wins, subsequent values are duplicates
		for ( DWORD i = 1; i < m_uAccumDocs; ++i )
			if ( dDocHits[i].m_tDocID == dDocHits[i - 1].m_tDocID )
				dRowMap[dDocHits[i].m_iDocIndex] = INVALID_ROWID;
	}

	RowID_t tNextRowID = 0;
	for ( auto& i : dRowMap )
		if ( i != INVALID_ROWID )
			i = tNextRowID++;

	// remove duplicate hits and compact hit.rowid
	// might be document without hits
	// but hits after that document should be still remapped \ compacted
	// that is why can not use short-cut of
	// if ( tSrcRowID!=INVALID_ROWID ) -> if ( i!=iDstRow )
	int iDstRow = 0;
	for ( int i = 0, iLen = m_dAccum.GetLength(); i < iLen; ++i )
	{
		const auto& dSrcHit = m_dAccum[i];
		RowID_t tSrcRowID = dRowMap[dSrcHit.m_tRowID];
		if ( tSrcRowID != INVALID_ROWID )
		{
			CSphWordHit& tDstHit = m_dAccum[iDstRow];
			tDstHit = dSrcHit;
			tDstHit.m_tRowID = tSrcRowID;
			++iDstRow;
		}
	}

	m_dAccum.Resize ( iDstRow );

	RemoveColumnarDuplicates ( m_pColumnarBuilder, dRowMap, tSchema );

	iDstRow = 0;
	ARRAY_FOREACH ( i, dRowMap )
	{
		if ( dRowMap[i] != INVALID_ROWID )
		{
			if ( i != iDstRow )
			{
				// remove duplicate docinfo
				// but all attributes could be columnar
				if ( iRowSize )
					memcpy ( &m_dAccumRows[iDstRow * iRowSize], &m_dAccumRows[i * iRowSize], iRowSize * sizeof ( CSphRowitem ) );

				m_dPerDocHitsCount[iDstRow] = m_dPerDocHitsCount[i];
				const int iNormFields = RtFieldNorms_c(tSchema).DenseFields();
				if ( iNormFields )
					memmove ( &m_dNorms[iDstRow*iNormFields], &m_dNorms[i*iNormFields], iNormFields*sizeof(DWORD) );

				// remove duplicate docstore
				if ( m_pDocstore )
					m_pDocstore->SwapRows ( iDstRow, i );

				if ( m_pEmbeddingsSrc )
					m_pEmbeddingsSrc->SwapRows ( iDstRow, i );
			}
			++iDstRow;
		}
	}

	m_dAccumRows.Resize ( iDstRow * iRowSize );
	m_dPerDocHitsCount.Resize ( iDstRow );
	m_dNorms.Resize ( int64_t(iDstRow)*RtFieldNorms_c(tSchema).DenseFields() );
	m_uAccumDocs = iDstRow;
	if ( m_pDocstore )
		m_pDocstore->DropTail ( iDstRow );
	if ( m_pEmbeddingsSrc )
		m_pEmbeddingsSrc->DropTail ( iDstRow );
}

void RtAccum_t::ForEachUuidDocid ( const std::function<void ( ByteBlob_t )> & fnVisitor ) const
{
	assert ( m_uAccumDocs );
	assert ( m_pIndex );

	const CSphSchema & tSchema = m_pIndex->GetInternalSchema();
	assert ( sphHasUuidDocid ( tSchema ) );

	const CSphColumnInfo * pUuidAttr = tSchema.GetAttr ( sphGetUuidDocidName() );
	assert ( pUuidAttr );
	assert ( pUuidAttr->m_eAttrType==SPH_ATTR_STRING );

	if ( pUuidAttr->IsColumnar() )
	{
		assert ( m_pColumnarBuilder );
		std::unique_ptr<ColumnarRT_i> pColumnar = CreateLightColumnarRT ( tSchema, m_pColumnarBuilder.get() );
		std::string sError;
		std::unique_ptr<columnar::Iterator_i> pUuidIt = CreateColumnarIterator ( pColumnar.get(), pUuidAttr->m_sName.cstr(), sError );
		assert ( pUuidIt );

		for ( DWORD uRow = 0; uRow < m_uAccumDocs; ++uRow )
		{
			const BYTE * pUuid = nullptr;
			int iUuidLen = pUuidIt->Get ( uRow, pUuid );
			fnVisitor ( { pUuid, iUuidLen } );
		}

		return;
	}

	assert ( tSchema.GetAttr ( sphGetBlobLocatorName() ) );
	assert ( !m_dBlobs.IsEmpty() );
	const int iRowSize = tSchema.GetRowSize();
	assert ( iRowSize>0 );
	assert ( m_dAccumRows.GetLength()==int64_t ( m_uAccumDocs ) * iRowSize );

	const BYTE * pBlobPool = m_dBlobs.Begin();
	const CSphRowitem * pRow = m_dAccumRows.Begin();
	for ( DWORD uRow = 0; uRow < m_uAccumDocs; ++uRow, pRow += iRowSize )
		fnVisitor ( sphGetBlobAttr ( pRow, pUuidAttr->m_tLocator, pBlobPool ) );
}


void RtAccum_t::GrabLastWarning ( CSphString& sWarning )
{
	if ( m_pDictRt && m_pDictRt->GetLastWarning() )
	{
		sWarning = m_pDictRt->GetLastWarning();
		m_pDictRt->ResetWarning();
	}
}


void RtAccum_t::SetIndex ( RtIndex_i * pIndex )
{
	assert ( pIndex );
	ResetPreparedForCommit();
	m_iIndexGeneration = pIndex->GetAlterGeneration();
	m_pIndex = pIndex;
	m_pBlobWriter.reset();
	m_sIndexName = pIndex->GetName();
	m_iIndexId = pIndex->GetIndexId();

	const CSphSchema& tSchema = pIndex->GetInternalSchema();
	if ( tSchema.HasBlobAttrs() )
		m_pBlobWriter = sphCreateBlobRowBuilder ( tSchema, m_dBlobs );

	if ( !m_pColumnarBuilder )
		m_pColumnarBuilder = CreateColumnarBuilderRT ( tSchema );

	m_uSchemaHash = pIndex->GetSchemaHash();
}


void RtAccum_t::CaptureReplicationValidation ( const RtIndex_i & tIndex )
{
	CaptureReplicationValidation ( tIndex.GetName(), tIndex.GetIndexId(), tIndex.GetSchemaHash(), tIndex.GetAlterGeneration(), tIndex.GetDictFormat() );
}


void RtAccum_t::CaptureReplicationValidation ( const CSphString & sName, int64_t iIndexId, uint64_t uSchemaHash, int iGeneration, DictFormat_e eDictFormat )
{
	m_iIndexGeneration = iGeneration;
	m_sIndexName = sName;
	m_iIndexId = iIndexId;
	m_uSchemaHash = uSchemaHash;
	m_eValidationDictFormat = eDictFormat;
	m_bReplicationValidation = true;
}


bool RtAccum_t::CheckReplicationValidation ( int64_t iIndexId, uint64_t uSchemaHash, int iGeneration, DictFormat_e eDictFormat, CSphString & sError ) const
{
	if ( !m_bReplicationValidation )
		return true;
	if ( m_iIndexId==iIndexId && m_uSchemaHash==uSchemaHash && m_iIndexGeneration==iGeneration && m_eValidationDictFormat==eDictFormat )
		return true;
	sError.SetSprintf ( "replication target table '%s' changed after RT transaction validation (id " INT64_FMT "->" INT64_FMT ", schema " UINT64_FMT "->" UINT64_FMT ", generation %d->%d, dictionary %d->%d)",
		m_sIndexName.cstr(), m_iIndexId, iIndexId, m_uSchemaHash, uSchemaHash, m_iIndexGeneration, iGeneration, int(m_eValidationDictFormat), int(eDictFormat) );
	return false;
}


bool RtAccum_t::CheckReplicationValidation ( const RtIndex_i & tIndex, CSphString & sError ) const
{
	return CheckReplicationValidation ( tIndex.GetIndexId(), tIndex.GetSchemaHash(), tIndex.GetAlterGeneration(), tIndex.GetDictFormat(), sError );
}


RowID_t RtAccum_t::GenerateRowID()
{
	return m_tNextRowID++;
}


void RtAccum_t::ResetRowID()
{
	m_tNextRowID = 0;
}

void RtAccum_t::BindUuidRegistry ( const UuidDocidRegistryPtr_t & pRegistry )
{
	assert ( pRegistry );
	assert ( !m_pUuidRegistry );
	assert ( m_dUuidLeases.IsEmpty() );
	m_pUuidRegistry = pRegistry;
}


bool RtAccum_t::IsUuidRegistry ( const UuidDocidRegistry_i * pRegistry ) const
{
	return m_pUuidRegistry.Ptr()==pRegistry;
}


void RtAccum_t::AdoptUuidLease ( const UuidDocidRegistry_i * pRegistry, const UuidDocidKey_t & tKey )
{
	assert ( pRegistry );
	assert ( m_pUuidRegistry.Ptr()==pRegistry );
	assert ( m_pUuidRegistry->GetDocid ( tKey ) );
	m_dUuidLeases.Add ( tKey );
}


void RtAccum_t::ResetUuidLeases()
{
	if ( m_dUuidLeases.IsEmpty() )
	{
		m_pUuidRegistry = nullptr;
		return;
	}

	assert ( m_pUuidRegistry );
	for ( const UuidDocidKey_t & tKey : m_dUuidLeases )
		m_pUuidRegistry->ReleaseKey ( tKey );

	m_dUuidLeases.Reset();
	m_pUuidRegistry = nullptr;
}


class RtTrxPreflight_c
{
public:
	explicit RtTrxPreflight_c ( ByteBlob_t tData )
		: m_pData ( tData.first )
		, m_iLength ( tData.second )
	{}

	template<typename T>
	bool Read ( T & tValue, const char * szWhat )
	{
		if ( !Skip ( sizeof(T), szWhat, &tValue ) )
			return false;
		return true;
	}

	bool ReadArray ( size_t iElemSize, const char * szWhat, DWORD * pCount=nullptr, const BYTE ** ppData=nullptr )
	{
		DWORD uCount = 0;
		if ( !Read ( uCount, szWhat ) )
			return false;
		if ( pCount )
			*pCount = uCount;
		if ( uCount>DWORD(std::numeric_limits<int>::max()) || ( iElemSize && uCount>size_t(std::numeric_limits<int>::max())/iElemSize ) )
			return TlsMsg::Err ( "replication RT transaction %s count is too large: %u", szWhat, uCount );
		if ( ppData )
			*ppData = m_pData+m_iPos;
		return Skip ( size_t(uCount)*iElemSize, szWhat );
	}

	bool ReadZipDword ( DWORD & uValue, const char * szWhat )
	{
		uValue = 0;
		for ( int i=0; i<5; ++i )
		{
			BYTE uByte = 0;
			if ( !Read ( uByte, szWhat ) )
				return false;
			uValue |= DWORD(uByte & 0x7f) << ( 7*i );
			if ( !( uByte & 0x80 ) )
				return true;
		}
		return true; // matches the five-byte DWORD decoder used by MemoryReader_c
	}

	bool ReadZipDwordBE ( DWORD & uValue, const char * szWhat )
	{
		uValue = 0;
		for ( int i=0; i<5; ++i )
		{
			BYTE uByte = 0;
			if ( !Read ( uByte, szWhat ) )
				return false;
			uValue = ( uValue << 7 ) | ( uByte & 0x7f );
			if ( !( uByte & 0x80 ) )
				return true;
		}
		return TlsMsg::Err ( "replication RT transaction has unterminated %s", szWhat );
	}

	bool Skip ( size_t iBytes, const char * szWhat, void * pValue=nullptr )
	{
		if ( iBytes>size_t(std::numeric_limits<int>::max()) || iBytes>size_t(m_iLength-m_iPos) )
			return TlsMsg::Err ( "replication RT transaction is truncated in %s", szWhat );
		if ( pValue && iBytes )
			memcpy ( pValue, m_pData+m_iPos, iBytes );
		m_iPos += (int)iBytes;
		return true;
	}

	int Remaining() const { return m_iLength-m_iPos; }
	const BYTE * Current() const { return m_pData+m_iPos; }

private:
	const BYTE * m_pData = nullptr;
	int m_iLength = 0;
	int m_iPos = 0;
};


static int GetDocstoreFields ( const CSphSchema & tSchema )
{
	int iFields = 0;
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		iFields += tSchema.IsFieldStored(i);
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		iFields += tSchema.IsAttrStored(i);
	return iFields;
}


static bool PreflightDocstore ( RtTrxPreflight_c & tReader, DWORD uRows, const CSphSchema & tSchema )
{
	DWORD uFields = 0;
	DWORD uDocs = 0;
	if ( !tReader.Read ( uFields, "docstore field count" ) || !tReader.ReadZipDword ( uDocs, "docstore document count" ) )
		return false;
	const DWORD uExpectedFields = GetDocstoreFields ( tSchema );
	if ( uFields!=uExpectedFields )
		return TlsMsg::Err ( "replication RT transaction docstore field count mismatch: got %u, expected %u", uFields, uExpectedFields );
	if ( uDocs!=uRows )
		return TlsMsg::Err ( "replication RT transaction docstore document count mismatch: got %u, expected %u", uDocs, uRows );
	if ( uDocs>DWORD(std::numeric_limits<int>::max()) || uDocs>DWORD(tReader.Remaining()) )
		return TlsMsg::Err ( "replication RT transaction docstore document count is too large: %u", uDocs );

	for ( DWORD uDoc=0; uDoc<uDocs; ++uDoc )
	{
		DWORD uLength = 0;
		if ( !tReader.ReadZipDword ( uLength, "docstore document length" ) )
			return false;
		if ( uLength>DWORD(tReader.Remaining()) )
			return TlsMsg::Err ( "replication RT transaction is truncated in docstore document" );
		const int iAfterDoc = tReader.Remaining()-uLength;
		for ( DWORD uField=0; uField<uFields; ++uField )
		{
			DWORD uFieldLength = 0;
			if ( !tReader.ReadZipDwordBE ( uFieldLength, "docstore field length" ) || uFieldLength>DWORD(tReader.Remaining()-iAfterDoc) )
				return TlsMsg::Err ( "replication RT transaction docstore field overruns document %u", uDoc );
			if ( !tReader.Skip ( uFieldLength, "docstore field" ) )
				return false;
		}
		if ( tReader.Remaining()!=iAfterDoc )
			return TlsMsg::Err ( "replication RT transaction docstore document %u has trailing bytes", uDoc );
	}
	return true;
}


static bool PreflightColumnarArray ( RtTrxPreflight_c & tReader, size_t iElemSize, const char * szWhat, DWORD & uCount, const BYTE ** ppData=nullptr )
{
	return tReader.ReadArray ( iElemSize, szWhat, &uCount, ppData );
}


template<typename T>
static T ReadUnaligned ( const BYTE * pData, DWORD uIndex )
{
	T tValue;
	memcpy ( &tValue, pData+size_t(uIndex)*sizeof(T), sizeof(T) );
	return tValue;
}


template<typename T>
static bool CheckColumnarOffsets ( const BYTE * pOffsets, DWORD uOffsets, DWORD uRows, DWORD uValues, const char * szWhat )
{
	if ( uOffsets!=uRows )
		return TlsMsg::Err ( "replication RT transaction %s count mismatch: got %u, expected %u", szWhat, uOffsets, uRows );
	T iPrevious = 0;
	for ( DWORD i=0; i<uOffsets; ++i )
	{
		const T iOffset = ReadUnaligned<T> ( pOffsets, i );
		if ( iOffset<iPrevious || iOffset<0 || uint64_t(iOffset)>uValues )
			return TlsMsg::Err ( "replication RT transaction %s has invalid offset at row %u", szWhat, i );
		iPrevious = iOffset;
	}
	if ( uint64_t(iPrevious)!=uValues )
		return TlsMsg::Err ( "replication RT transaction %s terminal offset mismatch: got " INT64_FMT ", expected %u", szWhat, int64_t(iPrevious), uValues );
	return true;
}


static bool PreflightColumnar ( RtTrxPreflight_c & tReader, DWORD uRows, const CSphSchema & tSchema )
{
	DWORD uAttrs = 0;
	if ( !tReader.Read ( uAttrs, "columnar attribute count" ) )
		return false;
	if ( uAttrs!=DWORD(tSchema.GetColumnarAttrsCount()) )
		return TlsMsg::Err ( "replication RT transaction columnar attribute count mismatch: got %u, expected %d", uAttrs, tSchema.GetColumnarAttrsCount() );
	if ( uAttrs>DWORD(std::numeric_limits<int>::max()) || uAttrs>DWORD(tReader.Remaining()/int(sizeof(DWORD))) )
		return TlsMsg::Err ( "replication RT transaction columnar attribute count is too large: %u", uAttrs );

	int iSchemaAttr = 0;
	for ( DWORD uAttr=0; uAttr<uAttrs; ++uAttr )
	{
		while ( !tSchema.GetAttr(iSchemaAttr).IsColumnar() )
			++iSchemaAttr;
		const CSphColumnInfo & tSchemaAttr = tSchema.GetAttr(iSchemaAttr++);
		DWORD uType = 0;
		if ( !tReader.Read ( uType, "columnar attribute type" ) )
			return false;
		if ( ESphAttr(uType)!=tSchemaAttr.m_eAttrType )
			return TlsMsg::Err ( "replication RT transaction columnar attribute %u type mismatch: got %u, expected %u", uAttr, uType, DWORD(tSchemaAttr.m_eAttrType) );
		DWORD uValues = 0;
		const BYTE * pValues = nullptr;
		switch ( (ESphAttr)uType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_FLOAT:
		case SPH_ATTR_TOKENCOUNT:
		{
			SphOffset_t uMask = 0;
			if ( !tReader.Read ( uMask, "columnar attribute mask" ) || !PreflightColumnarArray ( tReader, sizeof(DWORD), "columnar values", uValues ) )
				return false;
			const int iBits = tSchemaAttr.m_tLocator.m_iBitCount;
			const SphOffset_t uExpectedMask = iBits==64 ? SphOffset_t(-1) : ( SphOffset_t(1)<<iBits )-1;
			if ( uMask!=uExpectedMask )
				return TlsMsg::Err ( "replication RT transaction columnar attribute %u mask mismatch", uAttr );
			if ( uValues!=uRows )
				return TlsMsg::Err ( "replication RT transaction columnar values count mismatch: got %u, expected %u", uValues, uRows );
			break;
		}
		case SPH_ATTR_BOOL:
		{
			SphOffset_t uMask = 0;
			if ( !tReader.Read ( uMask, "columnar attribute mask" ) || !PreflightColumnarArray ( tReader, sizeof(BYTE), "columnar bool values", uValues, &pValues ) )
				return false;
			if ( uMask!=1 )
				return TlsMsg::Err ( "replication RT transaction columnar bool mask mismatch" );
			if ( uValues!=uRows )
				return TlsMsg::Err ( "replication RT transaction columnar bool values count mismatch: got %u, expected %u", uValues, uRows );
			for ( DWORD i=0; i<uValues; ++i )
				if ( pValues[i]>1 )
					return TlsMsg::Err ( "replication RT transaction columnar bool value is invalid at row %u", i );
			break;
		}
		case SPH_ATTR_BIGINT:
		{
			SphOffset_t uMask = 0;
			if ( !tReader.Read ( uMask, "columnar attribute mask" ) || !PreflightColumnarArray ( tReader, sizeof(int64_t), "columnar bigint values", uValues ) )
				return false;
			const int iBits = tSchemaAttr.m_tLocator.m_iBitCount;
			const SphOffset_t uExpectedMask = iBits==64 ? SphOffset_t(-1) : ( SphOffset_t(1)<<iBits )-1;
			if ( uMask!=uExpectedMask )
				return TlsMsg::Err ( "replication RT transaction columnar bigint mask mismatch" );
			if ( uValues!=uRows )
				return TlsMsg::Err ( "replication RT transaction columnar bigint values count mismatch: got %u, expected %u", uValues, uRows );
			break;
		}
		case SPH_ATTR_STRING:
		{
			DWORD uOffsets = 0;
			const BYTE * pOffsets = nullptr;
			if ( !PreflightColumnarArray ( tReader, sizeof(int64_t), "columnar string offsets", uOffsets, &pOffsets ) || !PreflightColumnarArray ( tReader, sizeof(BYTE), "columnar string data", uValues ) )
				return false;
			if ( !CheckColumnarOffsets<int64_t> ( pOffsets, uOffsets, uRows, uValues, "columnar string offsets" ) )
				return false;
			break;
		}
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_FLOAT_VECTOR:
		case SPH_ATTR_FLOAT_VECTOR_ARRAY:
		{
			DWORD uOffsets = 0;
			const BYTE * pOffsets = nullptr;
			if ( !PreflightColumnarArray ( tReader, sizeof(int), "columnar MVA offsets", uOffsets, &pOffsets ) || !PreflightColumnarArray ( tReader, sizeof(DWORD), "columnar MVA values", uValues ) )
				return false;
			if ( !CheckColumnarOffsets<int> ( pOffsets, uOffsets, uRows, uValues, "columnar MVA offsets" ) )
				return false;
			break;
		}
		case SPH_ATTR_INT64SET:
		{
			DWORD uOffsets = 0;
			const BYTE * pOffsets = nullptr;
			if ( !PreflightColumnarArray ( tReader, sizeof(int), "columnar MVA offsets", uOffsets, &pOffsets ) || !PreflightColumnarArray ( tReader, sizeof(int64_t), "columnar MVA values", uValues ) )
				return false;
			if ( !CheckColumnarOffsets<int> ( pOffsets, uOffsets, uRows, uValues, "columnar MVA offsets" ) )
				return false;
			break;
		}
		default:
			return TlsMsg::Err ( "replication RT transaction has invalid columnar attribute type %u", uType );
		}
	}
	return true;
}


static bool CheckNormCounts ( DWORD uRows, const CSphSchema * pSchema, DWORD uCount, bool bWireCount )
{
	if ( !pSchema && uRows )
		return TlsMsg::Err ( "schema-less replication RT transaction has %u documents", uRows );

	const uint64_t uFields = pSchema ? pSchema->GetFieldsCount() : 0;
	if ( uFields && uint64_t(uRows)>std::numeric_limits<DWORD>::max()/uFields )
		return TlsMsg::Err ( "replication RT transaction norms count overflow: rows=%u, fields=" UINT64_FMT, uRows, uFields );

	const uint64_t uExpected = uint64_t(uRows)*uFields;
	if ( bWireCount && uExpected!=uCount )
		return TlsMsg::Err ( "replication RT transaction norms count mismatch: got %u, expected " UINT64_FMT " (rows=%u, fields=" UINT64_FMT ")", uCount, uExpected, uRows, uFields );
	if ( uExpected>uint64_t(std::numeric_limits<int>::max())/sizeof(DWORD) )
		return TlsMsg::Err ( "replication RT transaction norms size exceeds reader limit: count=" UINT64_FMT, uExpected );

	RtFieldNorms_c tNorms;
	if ( pSchema )
		tNorms.Reset ( *pSchema );
	const uint64_t uDenseCount = uint64_t(uRows)*tNorms.DenseFields();
	if ( uDenseCount>uint64_t(std::numeric_limits<int>::max())/sizeof(DWORD) )
		return TlsMsg::Err ( "replication RT transaction dense norms size exceeds reader limit: rows=%u, fields=%d", uRows, tNorms.DenseFields() );
	return true;
}


static bool ValidatePackedKeyword ( const BYTE * pPacked, DWORD uPacked, SphWordID_t uWordID, DictFormat_e eDictFormat, DWORD uHit )
{
	if ( eDictFormat==DictFormat_e::CRC )
		return true;
	if ( uWordID>=uPacked )
		return TlsMsg::Err ( "replication RT transaction hit %u has packed-keyword offset " UINT64_FMT " outside %u-byte payload", uHit, uint64_t(uWordID), uPacked );

	const BYTE * pCur = pPacked+uWordID;
	const BYTE * pEnd = pPacked+uPacked;
	DWORD uLength = 0;
	if ( eDictFormat==DictFormat_e::KEYWORDS )
	{
		uLength = *pCur++;
		if ( !uLength || uLength>=SPH_MAX_KEYWORD_LEN )
			return TlsMsg::Err ( "replication RT transaction hit %u has invalid legacy packed-keyword length %u", uHit, uLength );
	}
	else if ( eDictFormat==DictFormat_e::KEYWORDS_V2 )
	{
		int iShift = 0;
		bool bDone = false;
		for ( int i=0; i<5 && pCur<pEnd; ++i, iShift+=7 )
		{
			const BYTE uByte = *pCur++;
			if ( i==4 && ( uByte & 0xf0 ) )
				return TlsMsg::Err ( "replication RT transaction hit %u has overflowing packed-keyword length prefix", uHit );
			uLength |= DWORD(uByte & 0x7f) << iShift;
			if ( !( uByte & 0x80 ) )
			{
				bDone = true;
				break;
			}
		}
		if ( !bDone )
			return TlsMsg::Err ( "replication RT transaction hit %u has truncated packed-keyword length prefix", uHit );
		if ( !uLength || uLength>DWORD(GetKeywordMaxStoredBytes(DictFormat_e::KEYWORDS_V2)) )
			return TlsMsg::Err ( "replication RT transaction hit %u has invalid keywords_32k packed-keyword length %u", uHit, uLength );
	}
	else
		return TlsMsg::Err ( "replication RT transaction has unsupported dictionary format %d", int(eDictFormat) );

	if ( size_t(pEnd-pCur)<uLength )
		return TlsMsg::Err ( "replication RT transaction hit %u packed-keyword record overruns payload", uHit );
	return true;
}


template<typename T>
static bool ReadBlobLength ( const BYTE * pRow, size_t iRemaining, int iAttr, T & uValue )
{
	const size_t iOffset = 1+size_t(iAttr)*sizeof(T);
	if ( iOffset+sizeof(T)>iRemaining )
		return false;
	memcpy ( &uValue, pRow+iOffset, sizeof(T) );
	return true;
}


static bool ValidateBlobRows ( const BYTE * pRows, DWORD uRows, const BYTE * pBlobs, DWORD uBlobs, const CSphSchema & tSchema )
{
	int iBlobAttrs = 0;
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		iBlobAttrs += sphIsBlobAttr ( tSchema.GetAttr(i) );
	if ( !iBlobAttrs )
		return !uBlobs || TlsMsg::Err ( "replication RT transaction has %u unexpected blob bytes for a schema without rowwise blob attributes", uBlobs );

	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );
	if ( !pBlobLocator )
		return TlsMsg::Err ( "replication RT target schema has rowwise blob attributes but no blob locator" );
	if ( !uRows )
		return !uBlobs || TlsMsg::Err ( "replication RT transaction has blob bytes without documents" );

	CSphFixedVector<CSphRowitem> dRow ( tSchema.GetRowSize() );
	uint64_t uExpectedOffset = 0;
	for ( DWORD uRow=0; uRow<uRows; ++uRow )
	{
		memcpy ( dRow.Begin(), pRows+uint64_t(uRow)*tSchema.GetRowSize()*sizeof(CSphRowitem), dRow.GetLengthBytes() );
		const uint64_t uOffset = sphGetRowAttr ( dRow.Begin(), pBlobLocator->m_tLocator );
		if ( uOffset!=uExpectedOffset || uOffset>=uBlobs )
			return TlsMsg::Err ( "replication RT transaction row %u has invalid blob locator " UINT64_FMT " (expected " UINT64_FMT ", pool %u)", uRow, uOffset, uExpectedOffset, uBlobs );
		const BYTE * pBlobRow = pBlobs+uOffset;
		const size_t iRemaining = uBlobs-size_t(uOffset);
		const BYTE uKind = pBlobRow[0];
		const size_t iWidth = uKind==0 ? 1 : ( uKind==1 ? 2 : ( uKind==2 ? 4 : 0 ) );
		if ( !iWidth )
			return TlsMsg::Err ( "replication RT transaction row %u has invalid blob-row length format %u", uRow, uKind );
		const size_t iHeader = 1+size_t(iBlobAttrs)*iWidth;
		if ( iHeader>iRemaining )
			return TlsMsg::Err ( "replication RT transaction row %u blob header overruns pool", uRow );

		uint64_t uPrevious = 0;
		for ( int iAttr=0; iAttr<iBlobAttrs; ++iAttr )
		{
			uint64_t uEnd = 0;
			if ( iWidth==1 ) { BYTE u=0; ReadBlobLength ( pBlobRow, iRemaining, iAttr, u ); uEnd=u; }
			else if ( iWidth==2 ) { WORD u=0; ReadBlobLength ( pBlobRow, iRemaining, iAttr, u ); uEnd=u; }
			else { DWORD u=0; ReadBlobLength ( pBlobRow, iRemaining, iAttr, u ); uEnd=u; }
			if ( uEnd<uPrevious || uEnd>iRemaining-iHeader )
				return TlsMsg::Err ( "replication RT transaction row %u blob attribute %d has invalid end offset " UINT64_FMT, uRow, iAttr, uEnd );
			uPrevious = uEnd;
		}
		uExpectedOffset += iHeader+uPrevious;
		if ( uExpectedOffset>uBlobs )
			return TlsMsg::Err ( "replication RT transaction row %u blob record overruns pool", uRow );
	}
	if ( uExpectedOffset!=uBlobs )
		return TlsMsg::Err ( "replication RT transaction blob pool has trailing bytes: validated " UINT64_FMT " of %u", uExpectedOffset, uBlobs );
	return true;
}


static bool PreflightRtTrx ( ByteBlob_t tTrx, DWORD uVer, const CSphSchema * pSchema, DictFormat_e eDictFormat )
{
	if ( tTrx.second<0 )
		return TlsMsg::Err ( "replication RT transaction has invalid length %d", tTrx.second );
	if ( tTrx.second && !tTrx.first )
		return TlsMsg::Err ( "replication RT transaction has no data" );

	RtTrxPreflight_c tReader ( tTrx );
	BYTE uReplace = 0;
	DWORD uRows = 0;
	int64_t iBytes = 0;
	if ( !tReader.Read ( uReplace, "header" ) || !tReader.Read ( uRows, "document count" ) || ( uVer>=0x106 && !tReader.Read ( iBytes, "accumulated byte count" ) ) )
		return false;
	if ( uReplace>1 )
		return TlsMsg::Err ( "replication RT transaction has invalid replace flag %u", uReplace );
	if ( uRows && !pSchema )
		return TlsMsg::Err ( "schema-less replication RT transaction has %u documents", uRows );

	DWORD uHits = 0;
	if ( !tReader.Read ( uHits, "hit count" ) )
		return false;
	constexpr size_t HIT_BYTES = sizeof(RowID_t)+sizeof(SphWordID_t)+sizeof(DWORD);
	if ( uHits>DWORD(std::numeric_limits<int>::max()) || uHits>size_t(std::numeric_limits<int>::max())/HIT_BYTES )
		return TlsMsg::Err ( "replication RT transaction hit count is too large or truncated: %u", uHits );
	const BYTE * pHits = tReader.Current();
	for ( DWORD i=0; i<uHits; ++i )
	{
		RowID_t tRowID = 0;
		SphWordID_t uWordID = 0;
		DWORD uWordPos = 0;
		if ( !tReader.Read ( tRowID, "hit row id" ) || !tReader.Read ( uWordID, "hit word id" ) || !tReader.Read ( uWordPos, "hit word position" ) )
			return false;
		if ( tRowID>=uRows )
			return TlsMsg::Err ( "replication RT transaction hit %u has invalid row id %u for %u documents", i, tRowID, uRows );
		if ( pSchema && uWordPos!=EMPTY_HIT )
		{
			const int iField = HITMAN::GetField ( uWordPos );
			if ( iField>=pSchema->GetFieldsCount() )
				return TlsMsg::Err ( "replication RT transaction hit %u references missing field %d", i, iField );
			if ( !( pSchema->GetField(iField).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED ) )
				return TlsMsg::Err ( "replication RT transaction hit %u references non-indexed field %d", i, iField );
		}
	}

	DWORD uRowItems = 0;
	const BYTE * pRows = nullptr;
	if ( !tReader.ReadArray ( sizeof(CSphRowitem), "accumulator rows", &uRowItems, &pRows ) )
		return false;
	const uint64_t uExpectedRows = uint64_t(uRows)*( pSchema ? pSchema->GetRowSize() : 0 );
	if ( uExpectedRows>std::numeric_limits<DWORD>::max() || uExpectedRows>uint64_t(std::numeric_limits<int>::max())/sizeof(CSphRowitem) )
		return TlsMsg::Err ( "replication RT transaction accumulator row count overflow: rows=%u, row_size=%d", uRows, pSchema ? pSchema->GetRowSize() : 0 );
	if ( uRowItems!=uExpectedRows )
		return TlsMsg::Err ( "replication RT transaction accumulator row item count mismatch: got %u, expected " UINT64_FMT, uRowItems, uExpectedRows );
	if ( uVer>=0x10C )
	{
		DWORD uNorms = 0;
		if ( !tReader.Read ( uNorms, "norm count" ) || !CheckNormCounts ( uRows, pSchema, uNorms, true ) || !tReader.Skip ( size_t(uNorms)*sizeof(DWORD), "norms" ) )
			return false;
	}
	else if ( !CheckNormCounts ( uRows, pSchema, 0, false ) )
		return false;

	DWORD uBlobs = 0;
	const BYTE * pBlobs = nullptr;
	DWORD uPerDoc = 0;
	const BYTE * pPerDoc = nullptr;
	DWORD uPacked = 0;
	const BYTE * pPacked = nullptr;
	if ( !tReader.ReadArray ( sizeof(BYTE), "blobs", &uBlobs, &pBlobs ) || !tReader.ReadArray ( sizeof(DWORD), "per-document hit counts", &uPerDoc, &pPerDoc ) || !tReader.ReadArray ( sizeof(BYTE), "packed keywords", &uPacked, &pPacked ) )
		return false;
	if ( pSchema && !ValidateBlobRows ( pRows, uRows, pBlobs, uBlobs, *pSchema ) )
		return false;
	for ( DWORD i=0; i<uHits; ++i )
	{
		SphWordID_t uWordID = 0;
		memcpy ( &uWordID, pHits+size_t(i)*HIT_BYTES+sizeof(RowID_t), sizeof(uWordID) );
		if ( !ValidatePackedKeyword ( pPacked, uPacked, uWordID, eDictFormat, i ) )
			return false;
	}
	if ( eDictFormat==DictFormat_e::CRC && uPacked )
		return TlsMsg::Err ( "replication RT transaction has %u packed-keyword bytes for CRC dictionary", uPacked );
	if ( eDictFormat!=DictFormat_e::CRC && uHits && !uPacked )
		return TlsMsg::Err ( "replication RT transaction is missing packed-keyword payload" );
	if ( uPerDoc!=uRows )
		return TlsMsg::Err ( "replication RT transaction per-document hit count length mismatch: got %u, expected %u", uPerDoc, uRows );
	uint64_t uPerDocHits = 0;
	for ( DWORD uRow=0; uRow<uRows; ++uRow )
	{
		const DWORD uCount = ReadUnaligned<DWORD> ( pPerDoc, uRow );
		if ( uCount>uint64_t(uHits)-uPerDocHits )
			return TlsMsg::Err ( "replication RT transaction per-document hit counts exceed total at row %u", uRow );
		uPerDocHits += uCount;
	}
	if ( uPerDocHits!=uHits )
		return TlsMsg::Err ( "replication RT transaction per-document hit counts total " UINT64_FMT " does not match hit count %u", uPerDocHits, uHits );

	BYTE uHaveDocstore = 0;
	if ( !tReader.Read ( uHaveDocstore, "docstore flag" ) )
		return false;
	if ( uHaveDocstore>1 )
		return TlsMsg::Err ( "replication RT transaction has invalid docstore flag %u", uHaveDocstore );
	const bool bNeedDocstore = uRows && pSchema && ( pSchema->HasStoredFields() || pSchema->HasStoredAttrs() );
	if ( bool(uHaveDocstore)!=bNeedDocstore )
		return TlsMsg::Err ( "replication RT transaction has %s docstore for target schema", uHaveDocstore ? "unexpected" : "missing" );
	if ( uHaveDocstore && !PreflightDocstore ( tReader, uRows, *pSchema ) )
		return false;
	BYTE uHaveColumnar = 0;
	if ( !tReader.Read ( uHaveColumnar, "columnar flag" ) )
		return false;
	if ( uHaveColumnar>1 )
		return TlsMsg::Err ( "replication RT transaction has invalid columnar flag %u", uHaveColumnar );
	const bool bNeedColumnar = pSchema && pSchema->HasColumnarAttrs();
	if ( bool(uHaveColumnar)!=bNeedColumnar )
		return TlsMsg::Err ( "replication RT transaction has %s columnar storage for target schema", uHaveColumnar ? "unexpected" : "missing" );
	if ( uHaveColumnar && !PreflightColumnar ( tReader, uRows, *pSchema ) )
		return false;
	if ( !tReader.ReadArray ( sizeof(DocID_t), "kill list" ) )
		return false;
	if ( tReader.Remaining() )
		return TlsMsg::Err ( "replication RT transaction has %d trailing bytes", tReader.Remaining() );
	return true;
}


static bool LoadSchemaNorms ( MemoryReader_c & tReader, CSphTightVector<DWORD> & dNorms, DWORD uRows, const CSphSchema * pSchema )
{
	const DWORD uCount = tReader.GetDword();
	if ( !CheckNormCounts ( uRows, pSchema, uCount, true ) )
		return false;

	CSphTightVector<DWORD> dSchemaNorms;
	dSchemaNorms.Resize ( uCount );
	if ( uCount )
		tReader.GetBytes ( dSchemaNorms.Begin(), int64_t(uCount)*sizeof(DWORD) );

	RtFieldNorms_c tNorms;
	if ( pSchema )
		tNorms.Reset ( *pSchema );
	CSphTightVector<DWORD> dDense;
	dDense.Resize ( int64_t(uRows)*tNorms.DenseFields() );
	if ( tNorms.DenseFields() )
		for ( DWORD uRow=0; uRow<uRows; ++uRow )
			tNorms.Compact ( dSchemaNorms.Begin()+int64_t(uRow)*tNorms.Fields(), dDense.Begin()+int64_t(uRow)*tNorms.DenseFields() );
	dNorms.SwapData ( dDense );
	return true;
}


static bool ValidateSchemaNorms ( const CSphTightVector<DWORD> & dNorms, DWORD uRows, const CSphSchema & tSchema )
{
	RtFieldNorms_c tNorms ( tSchema );
	const uint64_t uSchemaCount = uint64_t(uRows)*tNorms.Fields();
	if ( uSchemaCount>std::numeric_limits<DWORD>::max() )
		return TlsMsg::Err ( "replication RT transaction schema norms count overflow: rows=%u, fields=%d", uRows, tNorms.Fields() );
	if ( uSchemaCount>uint64_t(std::numeric_limits<int>::max())/sizeof(DWORD) )
		return TlsMsg::Err ( "replication RT transaction schema norms size exceeds project limit: count=" UINT64_FMT, uSchemaCount );

	const uint64_t uDenseCount = uint64_t(uRows)*tNorms.DenseFields();
	if ( uDenseCount!=uint64_t(dNorms.GetLength64()) )
		return TlsMsg::Err ( "replication RT transaction dense norms count mismatch: got " INT64_FMT ", expected " UINT64_FMT " (rows=%u, fields=%d)", dNorms.GetLength64(), uDenseCount, uRows, tNorms.DenseFields() );

	if ( uint64_t(tNorms.Fields())>uint64_t(std::numeric_limits<int>::max())/sizeof(DWORD) )
		return TlsMsg::Err ( "replication RT transaction schema norm row is too large: fields=%d", tNorms.Fields() );
	if ( uDenseCount>uint64_t(std::numeric_limits<int>::max())/sizeof(DWORD) )
		return TlsMsg::Err ( "replication RT transaction dense norms size exceeds project limit: count=" UINT64_FMT, uDenseCount );
	return true;
}


static bool SaveSchemaNorms ( const CSphTightVector<DWORD> & dNorms, DWORD uRows, const CSphSchema & tSchema, MemoryWriter_c & tWriter )
{
	if ( !ValidateSchemaNorms ( dNorms, uRows, tSchema ) )
		return false;

	RtFieldNorms_c tNorms ( tSchema );
	tWriter.PutDword ( DWORD(uint64_t(uRows)*tNorms.Fields()) );
	CSphFixedVector<DWORD> dSchemaRow ( tNorms.Fields() );
	for ( DWORD uRow=0; uRow<uRows && tNorms.Fields(); ++uRow )
	{
		const DWORD * pDenseRow = tNorms.DenseFields() ? dNorms.Begin()+int64_t(uRow)*tNorms.DenseFields() : nullptr;
		tNorms.Expand ( pDenseRow, dSchemaRow.Begin() );
		tWriter.PutBytes ( dSchemaRow.Begin(), dSchemaRow.GetLengthBytes() );
	}
	return true;
}


bool RtAccum_t::LoadRtTrx ( ByteBlob_t tTrx, DWORD uVer, const CSphSchema * pSchema, DictFormat_e eDictFormat )
{
	assert ( !m_pUuidRegistry );
	assert ( m_dUuidLeases.IsEmpty() );
	if ( !PreflightRtTrx ( tTrx, uVer, pSchema, eDictFormat ) )
		return false;

	bool bReplace = false;
	DWORD uAccumDocs = 0;
	int64_t iAccumBytes = 0;
	CSphTightVector<CSphWordHit> dAccum;
	CSphTightVector<CSphRowitem> dAccumRows;
	CSphTightVector<DWORD> dNorms;
	CSphTightVector<BYTE> dBlobs;
	CSphVector<DWORD> dPerDocHitsCount;
	CSphFixedVector<BYTE> dPackedKeywords ( 0 );
	std::unique_ptr<DocstoreRT_i> pDocstore;
	std::unique_ptr<ColumnarBuilderRT_i> pColumnarBuilder;
	CSphVector<DocID_t> dAccumKlist;

	MemoryReader_c tReader ( tTrx );
	bReplace = !!tReader.GetVal<BYTE>();
	tReader.GetVal ( uAccumDocs );
	if ( uVer>=0x106 )
		tReader.GetVal ( iAccumBytes );

	dAccum.Resize ( tReader.GetDword() );
	for ( CSphWordHit & tHit : dAccum )
	{
		tReader.GetVal ( tHit.m_tRowID );
		tReader.GetVal ( tHit.m_uWordID );
		tReader.GetVal ( tHit.m_uWordPos );
	}
	GetArray ( dAccumRows, tReader );
	if ( uVer>=0x10C )
	{
		if ( !LoadSchemaNorms ( tReader, dNorms, uAccumDocs, pSchema ) )
			return false;
	}
	GetArray ( dBlobs, tReader );
	GetArray ( dPerDocHitsCount, tReader );

	dPackedKeywords.Reset ( tReader.GetDword() );
	tReader.GetBytes ( dPackedKeywords.Begin(), (int)dPackedKeywords.GetLengthBytes() );

	if ( tReader.GetVal<BYTE>() )
	{
		pDocstore = CreateDocstoreRT();
		pDocstore->Load ( tReader );
	}
	if ( tReader.GetVal<BYTE>() )
		pColumnarBuilder = CreateColumnarBuilderRT ( tReader );
	GetArray ( dAccumKlist, tReader );

	if ( uVer<0x10C )
	{
		RtFieldNorms_c tNorms;
		if ( pSchema )
			tNorms.Reset ( *pSchema );
		dNorms.Resize ( int64_t(uAccumDocs)*tNorms.DenseFields() );
		dNorms.ZeroVec(); // legacy replication payloads did not carry exact lengths
	}

	ResetPreparedForCommit();
	m_bReplace = bReplace;
	m_uAccumDocs = uAccumDocs;
	m_iAccumBytes = iAccumBytes;
	m_dAccum.SwapData ( dAccum );
	m_dAccumRows.SwapData ( dAccumRows );
	m_dNorms.SwapData ( dNorms );
	m_dBlobs.SwapData ( dBlobs );
	m_dPerDocHitsCount.SwapData ( dPerDocHitsCount );
	m_dPackedKeywords.SwapData ( dPackedKeywords );
	m_pDocstore = std::move ( pDocstore );
	m_pColumnarBuilder = std::move ( pColumnarBuilder );
	m_dAccumKlist.SwapData ( dAccumKlist );
	return true;
}


bool RtAccum_t::SaveRtTrx ( MemoryWriter_c& tWriter ) const
{
	const CSphSchema * pSchema = m_pIndex ? &m_pIndex->GetMatchSchema() : nullptr;
	if ( !pSchema && ( m_uAccumDocs || !m_dNorms.IsEmpty() ) )
		return TlsMsg::Err ( "schema-less replication RT transaction has %u documents and %d norms", m_uAccumDocs, m_dNorms.GetLength() );
	if ( pSchema && !ValidateSchemaNorms ( m_dNorms, m_uAccumDocs, *pSchema ) )
		return false;

	tWriter.PutByte ( m_bReplace ); // this need only for data sort on commit
	tWriter.PutDword ( m_uAccumDocs );
	tWriter.PutVal ( m_iAccumBytes );

	// insert and replace
	tWriter.PutDword ( m_dAccum.GetLength() );
	for ( const CSphWordHit& tHit : m_dAccum )
	{
		tWriter.PutVal ( tHit.m_tRowID );
		tWriter.PutVal ( tHit.m_uWordID );
		tWriter.PutVal ( tHit.m_uWordPos );
	}
	SaveArray ( m_dAccumRows, tWriter );
	if ( pSchema )
	{
		if ( !SaveSchemaNorms ( m_dNorms, m_uAccumDocs, *pSchema, tWriter ) )
			return false;
	}
	else
		tWriter.PutDword ( 0 );
	SaveArray ( m_dBlobs, tWriter );
	SaveArray ( m_dPerDocHitsCount, tWriter );

	// packed keywords default length is 1 no need to pass that
	int iLen = ( IsKeywordDict() && m_pDictRt->GetPackedLen() > 1 ? (int)m_pDictRt->GetPackedLen() : 0 );
	tWriter.PutDword ( iLen );
	if ( iLen )
		tWriter.PutBytes ( m_pDictRt->GetPackedKeywords(), iLen );
	tWriter.PutByte ( m_pDocstore != nullptr );
	if ( m_pDocstore )
		m_pDocstore->Save ( tWriter );

	tWriter.PutByte ( m_pColumnarBuilder != nullptr );
	if ( m_pColumnarBuilder )
		m_pColumnarBuilder->Save ( tWriter );

	// delete
	SaveArray ( m_dAccumKlist, tWriter );
	return true;
}
