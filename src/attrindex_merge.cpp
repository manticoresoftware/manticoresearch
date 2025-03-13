//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "histogram.h"
#include "docidlookup.h"
#include "docstore.h"
#include "indexfiles.h"
#include "killlist.h"
#include "attrindex_builder.h"
#include "secondarylib.h"
#include "knnmisc.h"
#include "jsonsi.h"
#include "attrindex_merge.h"
#include "tracer.h"

#include <boost/preprocessor/repetition/repeat.hpp>

class AttrMerger_c::Impl_c
{
	AttrIndexBuilder_c						m_tMinMax;
	HistogramContainer_c					m_tHistograms;
	CSphVector<PlainOrColumnar_t>			m_dAttrsForHistogram;
	std::unique_ptr<knn::Builder_i>			m_pKNNBuilder;
	std::unique_ptr<JsonSIBuilder_i>		m_pJsonSIBuilder;
	CSphVector<PlainOrColumnar_t>			m_dAttrsForKNN;
	CSphFixedVector<DocidRowidPair_t> 		m_dDocidLookup {0};
	CSphWriter								m_tWriterSPA;
	std::unique_ptr<BlobRowBuilder_i>		m_pBlobRowBuilder;
	std::unique_ptr<DocstoreBuilder_i>		m_pDocstoreBuilder;
	std::unique_ptr<columnar::Builder_i>	m_pColumnarBuilder;
	RowID_t									m_tResultRowID = 0;
	int64_t									m_iTotalBytes = 0;

	MergeCb_c & 							m_tMonitor;
	CSphString &							m_sError;
	int64_t									m_iTotalDocs;
	BuildBufferSettings_t					m_tBufferSettings;

	CSphVector<PlainOrColumnar_t>	m_dSiAttrs;
	std::unique_ptr<SI::Builder_i>	m_pSIdxBuilder;

	CSphVector<ESphExt> 					m_dCreatedFiles;

private:
	template <bool WITH_BLOB, bool WITH_STRIDE, bool WITH_DOCSTORE, bool WITH_SI, bool WITH_KNN, bool PURE_COLUMNAR>
	bool CopyMixedAttributes_T ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap );

	bool AnalyzeMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap );

	CSphString GetTmpFilename ( const CSphIndex* pIdx, ESphExt eExt )
	{
		m_dCreatedFiles.Add ( eExt );
		assert ( pIdx );
		return  pIdx->GetTmpFilename ( eExt );
	}

public:
	Impl_c ( MergeCb_c & tMonitor, CSphString & sError, int64_t iTotalDocs, const BuildBufferSettings_t & tSettings )
		: m_tMonitor ( tMonitor )
		, m_sError ( sError )
		, m_iTotalDocs ( iTotalDocs )
		, m_tBufferSettings ( tSettings )
	{}

	bool Prepare ( const CSphIndex * pSrcIndex, const CSphIndex * pDstIndex );
	bool AnalyzeAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive );
	bool CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive );
	bool FinishMergeAttributes ( const CSphIndex * pDstIndex, BuildHeader_t& tBuildHeader, StrVec_t* pCreatedFiles );

	void AddCreatedFiles ( const CSphIndex * pDstIndex, StrVec_t * pCreatedFiles )
	{
		if ( pCreatedFiles )
			m_dCreatedFiles.for_each ( [pCreatedFiles, pDstIndex] ( auto eExt ) { pCreatedFiles->Add ( pDstIndex->GetTmpFilename ( eExt ) ); } );
	}
};

bool AttrMerger_c::Impl_c::Prepare ( const CSphIndex * pSrcIndex, const CSphIndex * pDstIndex )
{
	auto sSPA = GetTmpFilename ( pDstIndex, SPH_EXT_SPA );
	const CSphSchema & tDstSchema = pDstIndex->GetMatchSchema();
	if ( tDstSchema.HasNonColumnarAttrs() && !m_tWriterSPA.OpenFile ( sSPA, m_sError ) )
		return false;

	if ( tDstSchema.HasBlobAttrs() )
	{
		m_pBlobRowBuilder = sphCreateBlobRowBuilder ( pSrcIndex->GetMatchSchema(), GetTmpFilename ( pDstIndex, SPH_EXT_SPB ), pSrcIndex->GetSettings().m_tBlobUpdateSpace, m_tBufferSettings.m_iBufferAttributes, m_sError );
		if ( !m_pBlobRowBuilder )
			return false;
	}

	if ( pDstIndex->GetDocstore() )
	{
		m_pDocstoreBuilder = CreateDocstoreBuilder ( GetTmpFilename ( pDstIndex, SPH_EXT_SPDS ), pDstIndex->GetDocstore()->GetDocstoreSettings(), m_tBufferSettings.m_iBufferStorage, m_sError );
		if ( !m_pDocstoreBuilder )
			return false;

		for ( int i = 0; i < tDstSchema.GetFieldsCount(); ++i )
			if ( tDstSchema.IsFieldStored(i) )
				m_pDocstoreBuilder->AddField ( tDstSchema.GetFieldName(i), DOCSTORE_TEXT );

		for ( int i = 0; i < tDstSchema.GetAttrsCount(); ++i )
			if ( tDstSchema.IsAttrStored(i) )
				m_pDocstoreBuilder->AddField ( tDstSchema.GetAttr(i).m_sName, DOCSTORE_ATTR );
	}

	if ( tDstSchema.HasColumnarAttrs() )
	{
		m_pColumnarBuilder = CreateColumnarBuilder ( tDstSchema, GetTmpFilename ( pDstIndex, SPH_EXT_SPC ), m_tBufferSettings.m_iBufferColumnar, m_sError );
		if ( !m_pColumnarBuilder )
			return false;
	}

	if ( tDstSchema.HasKNNAttrs() )
	{
		m_pKNNBuilder = BuildCreateKNN ( tDstSchema, m_iTotalDocs, m_dAttrsForKNN, m_sError );
		if ( !m_pKNNBuilder )
			return false;
	}

	if ( tDstSchema.HasJsonSIAttrs() )
	{
		m_pJsonSIBuilder = CreateJsonSIBuilder ( tDstSchema, pDstIndex->GetTmpFilename(SPH_EXT_SPB), GetTmpFilename ( pDstIndex, SPH_EXT_SPJIDX ), m_sError );
		if ( !m_pJsonSIBuilder )
			return false;
	}

	if ( IsSecondaryLibLoaded() )
	{
		m_pSIdxBuilder = CreateIndexBuilder ( m_tBufferSettings.m_iSIMemLimit, tDstSchema, GetTmpFilename ( pDstIndex, SPH_EXT_SPIDX ), m_dSiAttrs, m_tBufferSettings.m_iBufferStorage, m_sError );
		if ( !m_pSIdxBuilder )
			return false;
	}

	m_tMinMax.Init ( tDstSchema );

	m_dDocidLookup.Reset ( m_iTotalDocs );
	BuildCreateHistograms ( m_tHistograms, m_dAttrsForHistogram, tDstSchema );

	m_tResultRowID = 0;
	return true;
}

template <bool WITH_BLOB, bool WITH_STRIDE, bool WITH_DOCSTORE, bool WITH_SI, bool WITH_KNN, bool PURE_COLUMNAR>
bool AttrMerger_c::Impl_c::CopyMixedAttributes_T ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap )
{
	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	int iColumnarIdLoc = PURE_COLUMNAR ? 0 : ( tIndex.GetMatchSchema ().GetAttr ( 0 ).IsColumnar () ? 0 : -1 );
	const CSphRowitem * pRow = tIndex.GetRawAttrs ();
	const BYTE * pRawBlobAttrs = PURE_COLUMNAR ? nullptr : tIndex.GetRawBlobAttrs ();
	int iStride = tIndex.GetMatchSchema().GetRowSize();
	CSphFixedVector<CSphRowitem> dTmpRow ( iStride );
	auto iStrideBytes = dTmpRow.GetLengthBytes();
	const CSphColumnInfo* pBlobLocator = WITH_BLOB ? tIndex.GetMatchSchema().GetAttr ( sphGetBlobLocatorName() ) : nullptr;

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID, pRow += PURE_COLUMNAR ? 0 : iStride )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_PULSE, iChunk );

		if ( m_tMonitor.NeedStop() )
			return false;

		// limit granted by caller code
		assert ( m_tResultRowID != INVALID_ROWID );

		if constexpr ( !PURE_COLUMNAR )
			m_tMinMax.Collect ( pRow );

		if constexpr ( WITH_BLOB )
		{
			const BYTE * pOldBlobRow = pRawBlobAttrs + sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );
			std::pair<SphOffset_t, SphOffset_t> tOffsetSize = m_pBlobRowBuilder->Flush ( pOldBlobRow );

			if ( m_pJsonSIBuilder )
				m_pJsonSIBuilder->AddRowOffsetSize ( tOffsetSize );

			memcpy ( dTmpRow.Begin(), pRow, iStrideBytes );
			sphSetRowAttr ( dTmpRow.Begin(), pBlobLocator->m_tLocator, tOffsetSize.first );

			m_tWriterSPA.PutBytes ( dTmpRow.Begin(), iStrideBytes );
		} else if constexpr ( WITH_STRIDE )
			m_tWriterSPA.PutBytes ( pRow, iStrideBytes );

		DocID_t tDocID = 0;
		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto & tIt = dColumnarIterators[i];
			SphAttr_t tAttr = SetColumnarAttr ( i, tIt.second, m_pColumnarBuilder.get(), tIt.first, tRowID, dTmp );
			if ( i==iColumnarIdLoc )
				tDocID = tAttr;
		}

		if constexpr ( !PURE_COLUMNAR )
		{
			if ( iColumnarIdLoc < 0 )
				tDocID = sphGetDocID(pRow);
		}

		if constexpr ( PURE_COLUMNAR )
		{
			assert ( !pRow );
			assert ( !pRawBlobAttrs );
		}

		BuildStoreHistograms ( tRowID, pRow, pRawBlobAttrs, dColumnarIterators, m_dAttrsForHistogram, m_tHistograms );

		if constexpr ( WITH_DOCSTORE )
			m_pDocstoreBuilder->AddDoc ( m_tResultRowID, tIndex.GetDocstore()->GetDoc ( tRowID, nullptr, -1, false ) );

		if constexpr ( WITH_SI )
		{
			m_pSIdxBuilder->SetRowID ( m_tResultRowID );
			BuildStoreSI ( tRowID, pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
		}

		if constexpr ( WITH_KNN )
			if ( !BuildStoreKNN ( tRowID, pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dAttrsForKNN, *m_pKNNBuilder ) )
			{
				m_sError = m_pKNNBuilder->GetError().c_str();
				return false;
			}

		m_dDocidLookup[m_tResultRowID] = { tDocID, m_tResultRowID };
		++m_tResultRowID;
	}

	return true;
}


bool AttrMerger_c::Impl_c::AnalyzeMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap )
{
	if ( !m_pKNNBuilder )
		return true;

	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );

	const CSphRowitem * pRow = tIndex.GetRawAttrs ();
	int iStride = tIndex.GetMatchSchema().GetRowSize();
	int iChunk = tIndex.m_iChunk;
	if ( !tIndex.GetRawAttrs() )
		iStride = 0;

	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID, pRow += iStride )
	{
		if ( dRowMap[tRowID]==INVALID_ROWID )
			continue;

		m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_PULSE, iChunk );
		if ( m_tMonitor.NeedStop() )
			return false;

		BuildTrainKNN ( tRowID, pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dAttrsForKNN, *m_pKNNBuilder );
	}

	return true;
}


bool AttrMerger_c::Impl_c::AnalyzeAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap, DWORD uAlive )
{
	if ( !uAlive )
		return true;

	return AnalyzeMixedAttributes ( tIndex, dRowMap );
}


bool AttrMerger_c::Impl_c::CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	if ( !uAlive )
		return true;

	// that is very empyric, however is better than nothing.
	m_iTotalBytes += tIndex.GetStats().m_iTotalBytes * ( (float)uAlive / (float)dRowMap.GetLength64() );

	const bool bPureColumnar = !tIndex.GetRawAttrs ();
	const bool bBlob = !bPureColumnar && !!m_pBlobRowBuilder;
	const bool bStride = !bPureColumnar && tIndex.GetMatchSchema ().GetRowSize ()>0;
	const bool bDocstore = !!m_pDocstoreBuilder;
	const bool bSI = !!m_pSIdxBuilder;
	const bool bKNN = !!m_pKNNBuilder;

	int iIndex = bPureColumnar*32+bKNN*16+bSI*8+bDocstore*4+bStride*2+bBlob;

	switch ( iIndex )
	{
#define DECL_COPYMIX( _, n, params ) case n: return CopyMixedAttributes_T<!!(n&1), !!(n&2), !!(n&4), !!(n&8), !!(n&16), !!(n&32)> params;
		BOOST_PP_REPEAT ( 64, DECL_COPYMIX, ( tIndex, dRowMap ) )
#undef DECL_COPYMIX
	default:
		assert ( 0 && "Internal error" );
		break;
	}
	return false;
}


bool AttrMerger_c::Impl_c::FinishMergeAttributes ( const CSphIndex * pDstIndex, BuildHeader_t& tBuildHeader, StrVec_t* pCreatedFiles )
{
	m_tMinMax.FinishCollect();
	assert ( m_tResultRowID==m_iTotalDocs );
	tBuildHeader.m_iDocinfo = m_iTotalDocs;
	tBuildHeader.m_iTotalDocuments = m_iTotalDocs;
	tBuildHeader.m_iTotalBytes = m_iTotalBytes;

	m_dDocidLookup.Sort ( CmpDocidLookup_fn() );
	if ( !WriteDocidLookup ( GetTmpFilename ( pDstIndex, SPH_EXT_SPT ), m_dDocidLookup, m_sError ) )
		return false;

	if ( pDstIndex->GetMatchSchema().HasNonColumnarAttrs() )
	{
		if ( m_iTotalDocs )
		{
			tBuildHeader.m_iMinMaxIndex = m_tWriterSPA.GetPos() / sizeof(CSphRowitem);
			const auto& dMinMaxRows		 = m_tMinMax.GetCollected();
			m_tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLengthBytes64() );
			tBuildHeader.m_iDocinfoIndex = ( dMinMaxRows.GetLength() / pDstIndex->GetMatchSchema().GetRowSize() / 2 ) - 1;
		}

		m_tWriterSPA.CloseFile();
		if ( m_tWriterSPA.IsError() )
			return false;
	}

	if ( m_pBlobRowBuilder && !m_pBlobRowBuilder->Done(m_sError) )
		return false;

	std::string sErrorSTL;
	if ( m_pColumnarBuilder && !m_pColumnarBuilder->Done(sErrorSTL) )
	{
		m_sError = sErrorSTL.c_str();
		return false;
	}

	if ( !m_tHistograms.Save ( GetTmpFilename ( pDstIndex, SPH_EXT_SPHI ), m_sError ) )
		return false;

	if ( !CheckDocsCount ( m_tResultRowID, m_sError ) )
		return false;

	if ( m_pDocstoreBuilder )
		m_pDocstoreBuilder->Finalize();

	std::string sError;
	if ( m_pSIdxBuilder && !m_pSIdxBuilder->Done ( sError ) )
	{
		m_sError = sError.c_str();
		return false;
	}

	if ( m_pJsonSIBuilder && !m_pJsonSIBuilder->Done(m_sError) )
		return false;

	if ( m_pKNNBuilder && !m_pKNNBuilder->Save ( GetTmpFilename ( pDstIndex, SPH_EXT_SPKNN ).cstr(), m_tBufferSettings.m_iBufferStorage, sError ) )
	{
		m_sError = sError.c_str();
		return false;
	}

	if ( !WriteDeadRowMap ( GetTmpFilename ( pDstIndex, SPH_EXT_SPM ), m_tResultRowID, m_sError ) )
		return false;

	return true;
}


AttrMerger_c::AttrMerger_c ( MergeCb_c& tMonitor, CSphString& sError, int64_t iTotalDocs, const BuildBufferSettings_t & tSettings )
	: m_pImpl { std::make_unique<Impl_c> ( tMonitor, sError, iTotalDocs, tSettings ) }
{}

AttrMerger_c::~AttrMerger_c() = default;

bool AttrMerger_c::Prepare ( const CSphIndex* pSrcIndex, const CSphIndex* pDstIndex )
{
	TRACE_CORO ( "sph", "AttrMerger_c::Prepare" );
	return m_pImpl->Prepare ( pSrcIndex, pDstIndex );
}

bool AttrMerger_c::AnalyzeAttributes ( const CSphIndex& tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	TRACE_CORO ( "sph", "AttrMerger_c::AnalyzeAttributes" );
	return m_pImpl->AnalyzeAttributes ( tIndex, dRowMap, uAlive );
}

bool AttrMerger_c::CopyAttributes ( const CSphIndex& tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	TRACE_CORO ( "sph", "AttrMerger_c::CopyAttributes" );
	return m_pImpl->CopyAttributes ( tIndex, dRowMap, uAlive );
}

bool AttrMerger_c::FinishMergeAttributes ( const CSphIndex* pDstIndex, BuildHeader_t& tBuildHeader, StrVec_t* pCreatedFiles )
{
	TRACE_CORO ( "sph", "AttrMerger_c::FinishMergeAttributes" );
	bool bOk = m_pImpl->FinishMergeAttributes ( pDstIndex, tBuildHeader, pCreatedFiles );
	m_pImpl->AddCreatedFiles ( pDstIndex, pCreatedFiles );
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////

class SiBuilder_c
{
public:
				SiBuilder_c ( const CSphIndex & tIndex, MergeCb_c & tMonitor, int64_t iNumDocs, CSphString & sError );

	bool		Build();
	StrVec_t	GetOldFiles() const { return m_dOldFiles; }
	StrVec_t	GetNewFiles() const { return m_dNewFiles; }

private:
	const CSphIndex &	m_tIndex;
	MergeCb_c &			m_tMonitor;
	int64_t				m_iNumDocs;
	CSphString &		m_sError;
	StrVec_t			m_dOldFiles;
	StrVec_t			m_dNewFiles;

	CSphVector<PlainOrColumnar_t>	m_dSiAttrs;
	std::unique_ptr<SI::Builder_i>	m_pSIdxBuilder;
	std::unique_ptr<JsonSIBuilder_i> m_pJsonSIBuilder;

	bool		ProcessPureColumnarAttributes();
	bool		ProcessMixedAttributes();
};


SiBuilder_c::SiBuilder_c ( const CSphIndex & tIndex, MergeCb_c & tMonitor, int64_t iNumDocs, CSphString & sError )
	: m_tIndex ( tIndex )
	, m_tMonitor ( tMonitor )
	, m_iNumDocs ( iNumDocs )
	, m_sError ( sError )
{}


bool SiBuilder_c::ProcessPureColumnarAttributes()
{
	assert ( !m_tIndex.GetRawAttrs() );
	assert ( m_tIndex.GetMatchSchema().GetAttr ( 0 ).IsColumnar() );

	auto dColumnarIterators = CreateAllColumnarIterators ( m_tIndex.GetColumnar(), m_tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	int iChunk = m_tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );

	for ( RowID_t tRowID = 0; tRowID<(RowID_t)m_iNumDocs; ++tRowID )
	{
		if ( m_tMonitor.NeedStop() )
			return false;

		m_pSIdxBuilder->SetRowID ( tRowID );
		BuildStoreSI ( tRowID, nullptr, nullptr, dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
	}
	return true;
}


bool SiBuilder_c::ProcessMixedAttributes()
{
	auto dColumnarIterators = CreateAllColumnarIterators ( m_tIndex.GetColumnar(), m_tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	const CSphRowitem * pRow = m_tIndex.GetRawAttrs();
	assert(pRow);
	const BYTE * pBlobRow = m_tIndex.GetRawBlobAttrs();
	const CSphSchema & tSchema = m_tIndex.GetMatchSchema();
	const CSphColumnInfo * pBlobLoc = tSchema.GetAttr ( sphGetBlobLocatorName() );
	int iStride = tSchema.GetRowSize();
	int iNumBlobAttrs = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
		if ( tSchema.GetAttr(i).m_tLocator.IsBlobAttr() )
			iNumBlobAttrs++;

	int iChunk = m_tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );

	for ( RowID_t tRowID = 0; tRowID<(RowID_t)m_iNumDocs; ++tRowID, pRow += iStride )
	{
		if ( m_tMonitor.NeedStop() )
			return false;

		m_pSIdxBuilder->SetRowID(tRowID);
		BuildStoreSI ( tRowID, pRow, m_tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );

		if ( m_pJsonSIBuilder )
		{
			assert ( pBlobRow && pBlobLoc );
			SphAttr_t tBlobRowOffset = sphGetRowAttr ( pRow, pBlobLoc->m_tLocator );
			m_pJsonSIBuilder->AddRowOffsetSize ( { tBlobRowOffset, sphGetBlobTotalLen ( pBlobRow+tBlobRowOffset, iNumBlobAttrs ) } );
		}
	}
	return true;
}


bool SiBuilder_c::Build()
{
	if ( !IsSecondaryLibLoaded() )
	{
		m_sError = "secondary index library not loaded";
		return false;
	}

	CSphString sSPIDX = m_tIndex.GetTmpFilename ( SPH_EXT_SPIDX );
	CSphString sSPJIDX = m_tIndex.GetTmpFilename ( SPH_EXT_SPJIDX );
	m_dNewFiles.Add(sSPIDX);
	m_dOldFiles.Add ( m_tIndex.GetFilename ( SPH_EXT_SPIDX ) );

	BuildBufferSettings_t tSettings; // use default buffer settings
	m_pSIdxBuilder = CreateIndexBuilder ( tSettings.m_iSIMemLimit, m_tIndex.GetMatchSchema(), sSPIDX, m_dSiAttrs, tSettings.m_iBufferStorage, m_sError );
	if ( !m_pSIdxBuilder )
		return false;

	if ( m_tIndex.GetMatchSchema().HasJsonSIAttrs() )
	{
		m_dNewFiles.Add(sSPJIDX);
		m_dOldFiles.Add ( m_tIndex.GetFilename ( SPH_EXT_SPJIDX ) );
		m_pJsonSIBuilder = CreateJsonSIBuilder ( m_tIndex.GetMatchSchema(), m_tIndex.GetFilename(SPH_EXT_SPB), sSPJIDX, m_sError );
		if ( !m_pJsonSIBuilder )
			return false;
	}

	bool bOk = false;
	if ( !m_tIndex.GetRawAttrs() )
		bOk = ProcessPureColumnarAttributes();
	else
		bOk = ProcessMixedAttributes();

	if ( !bOk )
		return false;

	std::string sError;
	if ( !m_pSIdxBuilder->Done(sError) )
	{
		m_sError = sError.c_str();
		return false;
	}

	if ( m_pJsonSIBuilder && !m_pJsonSIBuilder->Done(m_sError) )
		return false;

	return true;
}


bool SiRecreate ( MergeCb_c & tMonitor, const CSphIndex & tIndex, int64_t iNumDocs, StrVec_t & dOldFiles, StrVec_t & dNewFiles, CSphString & sError )
{
	SiBuilder_c tBuilder ( tIndex, tMonitor, iNumDocs, sError );
	if ( !tBuilder.Build() )
		return false;

	dOldFiles = tBuilder.GetOldFiles();
	dNewFiles = tBuilder.GetNewFiles();

	return true;
}
