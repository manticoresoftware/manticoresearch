//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "attrindex_merge.h"

class AttrMerger_c::Impl_c
{
	AttrIndexBuilder_c						m_tMinMax;
	HistogramContainer_c					m_tHistograms;
	CSphVector<PlainOrColumnar_t>			m_dAttrsForHistogram;
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

	CSphVector<PlainOrColumnar_t>	m_dSiAttrs;
	std::unique_ptr<SI::Builder_i>	m_pSIdxBuilder;

	CSphVector<ESphExt> 					m_dCreatedFiles;

private:
	bool CopyPureColumnarAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap );
	bool CopyMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap );
	CSphString GetTmpFilename ( const CSphIndex* pIdx, ESphExt eExt )
	{
		m_dCreatedFiles.Add ( eExt );
		assert ( pIdx );
		return  pIdx->GetTmpFilename ( eExt );
	}

public:
	Impl_c ( MergeCb_c & tMonitor, CSphString & sError, int64_t iTotalDocs )
		: m_tMonitor ( tMonitor )
		, m_sError ( sError )
		, m_iTotalDocs ( iTotalDocs )
	{}

	bool Prepare ( const CSphIndex * pSrcIndex, const CSphIndex * pDstIndex );
	bool CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive );
	bool FinishMergeAttributes ( const CSphIndex * pDstIndex, BuildHeader_t& tBuildHeader, StrVec_t* pCreatedFiles );
};

bool AttrMerger_c::Impl_c::Prepare ( const CSphIndex * pSrcIndex, const CSphIndex * pDstIndex )
{
	auto sSPA = GetTmpFilename ( pDstIndex, SPH_EXT_SPA );
	if ( pDstIndex->GetMatchSchema().HasNonColumnarAttrs() && !m_tWriterSPA.OpenFile ( sSPA, m_sError ) )
		return false;

	if ( pDstIndex->GetMatchSchema().HasBlobAttrs() )
	{
		m_pBlobRowBuilder = sphCreateBlobRowBuilder ( pSrcIndex->GetMatchSchema(), GetTmpFilename ( pDstIndex, SPH_EXT_SPB ), pSrcIndex->GetSettings().m_tBlobUpdateSpace, m_sError );
		if ( !m_pBlobRowBuilder )
			return false;
	}

	if ( pDstIndex->GetDocstore() )
	{
		m_pDocstoreBuilder = CreateDocstoreBuilder ( GetTmpFilename ( pDstIndex, SPH_EXT_SPDS ), pDstIndex->GetDocstore()->GetDocstoreSettings(), m_sError );
		if ( !m_pDocstoreBuilder )
			return false;

		for ( int i = 0; i < pDstIndex->GetMatchSchema().GetFieldsCount(); ++i )
			if ( pDstIndex->GetMatchSchema().IsFieldStored(i) )
				m_pDocstoreBuilder->AddField ( pDstIndex->GetMatchSchema().GetFieldName(i), DOCSTORE_TEXT );

		for ( int i = 0; i < pDstIndex->GetMatchSchema().GetAttrsCount(); ++i )
			if ( pDstIndex->GetMatchSchema().IsAttrStored(i) )
				m_pDocstoreBuilder->AddField ( pDstIndex->GetMatchSchema().GetAttr(i).m_sName, DOCSTORE_ATTR );
	}

	if ( pDstIndex->GetMatchSchema().HasColumnarAttrs() )
	{
		m_pColumnarBuilder = CreateColumnarBuilder ( pDstIndex->GetMatchSchema(), pDstIndex->GetSettings(), GetTmpFilename ( pDstIndex, SPH_EXT_SPC ), m_sError );
		if ( !m_pColumnarBuilder )
			return false;
	}

	if ( IsSecondaryLibLoaded() )
	{
		m_pSIdxBuilder = CreateIndexBuilder ( 64 * 1024 * 1024, pDstIndex->GetMatchSchema(), GetTmpFilename ( pDstIndex, SPH_EXT_SPIDX ), m_dSiAttrs, m_sError );
		if ( !m_pSIdxBuilder )
			return false;
	}

	m_tMinMax.Init ( pDstIndex->GetMatchSchema() );

	m_dDocidLookup.Reset ( m_iTotalDocs );
	CreateHistograms ( m_tHistograms, m_dAttrsForHistogram, pDstIndex->GetMatchSchema() );

	m_tResultRowID = 0;
	return true;
}


bool AttrMerger_c::Impl_c::CopyPureColumnarAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap )
{
	assert ( !tIndex.GetRawAttrs() );
	assert ( tIndex.GetMatchSchema().GetAttr ( 0 ).IsColumnar() );

	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		// limit granted by caller code
		assert ( m_tResultRowID != INVALID_ROWID );

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first, tRowID ) );
			SetColumnarAttr ( i, tIt.second, m_pColumnarBuilder.get(), tIt.first, dTmp );
		}

		BuildStoreHistograms ( nullptr, nullptr, dColumnarIterators, m_dAttrsForHistogram, m_tHistograms );

		if ( m_pDocstoreBuilder )
			m_pDocstoreBuilder->AddDoc ( m_tResultRowID, tIndex.GetDocstore()->GetDoc ( tRowID, nullptr, -1, false ) );

		if ( m_pSIdxBuilder )
		{
			m_pSIdxBuilder->SetRowID ( m_tResultRowID );
			BuilderStoreAttrs ( nullptr, nullptr, dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
		}

		m_dDocidLookup[m_tResultRowID] = { dColumnarIterators[0].first->Get(), m_tResultRowID };
		++m_tResultRowID;
	}
	return true;
}

bool AttrMerger_c::Impl_c::CopyMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap )
{
	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	int iColumnarIdLoc = tIndex.GetMatchSchema().GetAttr ( 0 ).IsColumnar() ? 0 : - 1;
	const CSphRowitem* pRow = tIndex.GetRawAttrs();
	assert ( pRow );
	int iStride = tIndex.GetMatchSchema().GetRowSize();
	CSphFixedVector<CSphRowitem> dTmpRow ( iStride );
	auto iStrideBytes = dTmpRow.GetLengthBytes();
	const CSphColumnInfo* pBlobLocator = tIndex.GetMatchSchema().GetAttr ( sphGetBlobLocatorName() );

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID, pRow += iStride )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		// limit granted by caller code
		assert ( m_tResultRowID != INVALID_ROWID );

		m_tMinMax.Collect ( pRow );

		if ( m_pBlobRowBuilder )
		{
			const BYTE* pOldBlobRow = tIndex.GetRawBlobAttrs() + sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );
			uint64_t	uNewOffset	= m_pBlobRowBuilder->Flush ( pOldBlobRow );

			memcpy ( dTmpRow.Begin(), pRow, iStrideBytes );
			sphSetRowAttr ( dTmpRow.Begin(), pBlobLocator->m_tLocator, uNewOffset );

			m_tWriterSPA.PutBytes ( dTmpRow.Begin(), iStrideBytes );
		} else if ( iStrideBytes )
			m_tWriterSPA.PutBytes ( pRow, iStrideBytes );

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first, tRowID ) );
			SetColumnarAttr ( i, tIt.second, m_pColumnarBuilder.get(), tIt.first, dTmp );
		}

		DocID_t tDocID = iColumnarIdLoc >= 0 ? dColumnarIterators[iColumnarIdLoc].first->Get() : sphGetDocID ( pRow );

		BuildStoreHistograms ( pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dAttrsForHistogram, m_tHistograms );

		if ( m_pDocstoreBuilder )
			m_pDocstoreBuilder->AddDoc ( m_tResultRowID, tIndex.GetDocstore()->GetDoc ( tRowID, nullptr, -1, false ) );

		if ( m_pSIdxBuilder )
		{
			m_pSIdxBuilder->SetRowID ( m_tResultRowID );
			BuilderStoreAttrs ( pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
		}

		m_dDocidLookup[m_tResultRowID] = { tDocID, m_tResultRowID };
		++m_tResultRowID;
	}
	return true;
}


bool AttrMerger_c::Impl_c::CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	if ( !uAlive )
		return true;

	// that is very empyric, however is better than nothing.
	m_iTotalBytes += tIndex.GetStats().m_iTotalBytes * ( (float)uAlive / (float)dRowMap.GetLength64() );

	if ( !tIndex.GetRawAttrs() )
		return CopyPureColumnarAttributes( tIndex, dRowMap );
	return CopyMixedAttributes ( tIndex, dRowMap );
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

	if ( !WriteDeadRowMap ( GetTmpFilename ( pDstIndex, SPH_EXT_SPM ), m_tResultRowID, m_sError ) )
		return false;

	if ( pCreatedFiles )
		m_dCreatedFiles.for_each ( [pCreatedFiles, pDstIndex] ( auto eExt ) { pCreatedFiles->Add ( pDstIndex->GetTmpFilename ( eExt ) ); } );

	return true;
}


AttrMerger_c::AttrMerger_c ( MergeCb_c& tMonitor, CSphString& sError, int64_t iTotalDocs )
	: m_pImpl { std::make_unique<Impl_c> ( tMonitor, sError, iTotalDocs ) }
{}

AttrMerger_c::~AttrMerger_c() = default;

bool AttrMerger_c::Prepare ( const CSphIndex* pSrcIndex, const CSphIndex* pDstIndex )
{
	return m_pImpl->Prepare ( pSrcIndex, pDstIndex );
}

bool AttrMerger_c::CopyAttributes ( const CSphIndex& tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	return m_pImpl->CopyAttributes ( tIndex, dRowMap, uAlive );
}

bool AttrMerger_c::FinishMergeAttributes ( const CSphIndex* pDstIndex, BuildHeader_t& tBuildHeader, StrVec_t* pCreatedFiles )
{
	return m_pImpl->FinishMergeAttributes ( pDstIndex, tBuildHeader, pCreatedFiles );
}


/////////////////////////////////////////////////////////////////////////////


class SiBuilder_c
{
public:
	SiBuilder_c (  MergeCb_c & tMonitor, CSphString & sError )
		: m_tMonitor ( tMonitor )
		, m_sError ( sError )
	{}

	bool CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap );

	CSphString m_sFilename;

private:
	MergeCb_c & m_tMonitor;
	CSphString & m_sError;

	CSphVector<PlainOrColumnar_t>	m_dSiAttrs;
	std::unique_ptr<SI::Builder_i>	m_pSIdxBuilder;

	bool CopyPureColumnarAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap );
	bool CopyMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap );
};

bool SiBuilder_c::CopyPureColumnarAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap )
{
	assert ( !tIndex.GetRawAttrs() );
	assert ( tIndex.GetMatchSchema().GetAttr ( 0 ).IsColumnar() );

	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );

	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID<tRows; ++tRowID )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first, tRowID ) );
		}

		m_pSIdxBuilder->SetRowID ( tRowID );
		BuilderStoreAttrs ( nullptr, nullptr, dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
	}
	return true;
}

bool SiBuilder_c::CopyMixedAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap )
{
	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.GetColumnar(), tIndex.GetMatchSchema() );
	CSphVector<int64_t> dTmp;

	const CSphRowitem * pRow = tIndex.GetRawAttrs();
	assert ( pRow );
	int iStride = tIndex.GetMatchSchema().GetRowSize();

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	AT_SCOPE_EXIT ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );

	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID<tRows; ++tRowID, pRow += iStride )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first, tRowID ) );
		}

		m_pSIdxBuilder->SetRowID ( tRowID );
		BuilderStoreAttrs ( pRow, tIndex.GetRawBlobAttrs(), dColumnarIterators, m_dSiAttrs, m_pSIdxBuilder.get(), dTmp );
	}
	return true;
}


bool SiBuilder_c::CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap )
{
	if ( IsSecondaryLibLoaded() )
	{
		m_sFilename = tIndex.GetTmpFilename ( SPH_EXT_SPIDX );
		m_pSIdxBuilder = CreateIndexBuilder ( 64*1024*1024, tIndex.GetMatchSchema(), m_sFilename, m_dSiAttrs, m_sError );
	} else
	{
		m_sError = "secondary index library not loaded";
	}

	if ( !m_pSIdxBuilder )
		return false;

	bool bOk = false;
	if ( !tIndex.GetRawAttrs() )
		bOk = CopyPureColumnarAttributes( tIndex, dRowMap );
	else
		bOk = CopyMixedAttributes ( tIndex, dRowMap );

	if ( !bOk )
		return false;

	std::string sError;
	if ( m_pSIdxBuilder.get() && !m_pSIdxBuilder->Done ( sError ) )
	{
		m_sError = sError.c_str();
		return false;
	}

	return true;
}

bool SiRecreate ( MergeCb_c & tMonitor, const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap, CSphString & sFile, CSphString & sError )
{
	SiBuilder_c tBuilder ( tMonitor, sError );
	if ( !tBuilder.CopyAttributes ( tIndex, dRowMap ) )
		return false;

	sFile = tBuilder.m_sFilename;

	return true;
}
