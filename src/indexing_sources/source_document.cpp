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

#include "source_document.h"
#include "stripper/html_stripper.h"
#include "tokenizer/tokenizer.h"
#include "sphinxint.h"

void CSphSource::SetDict ( const DictRefPtr_c& pDict )
{
	assert ( pDict );
	m_pDict = pDict;
}


const CSphSourceStats & CSphSource::GetStats ()
{
	return m_tStats;
}


bool CSphSource::SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements, bool bDetectParagraphs, const char * sZones, CSphString & sError )
{
	if ( !m_pStripper )
		m_pStripper = new CSphHTMLStripper ( true );

	if ( !m_pStripper->SetIndexedAttrs ( sExtractAttrs, sError ) )
		return false;

	if ( !m_pStripper->SetRemovedElements ( sRemoveElements, sError ) )
		return false;

	if ( bDetectParagraphs )
		m_pStripper->EnableParagraphs ();

	if ( !m_pStripper->SetZones ( sZones, sError ) )
		return false;

	return true;
}


void CSphSource::SetFieldFilter ( std::unique_ptr<ISphFieldFilter> pFilter )
{
	m_pFieldFilter = std::move ( pFilter );
}

void CSphSource::SetTokenizer ( TokenizerRefPtr_c pTokenizer )
{
	assert ( pTokenizer );
	m_pTokenizer = std::move ( pTokenizer );
}


bool CSphSource::UpdateSchema ( CSphSchema * pInfo, CSphString & sError )
{
	assert ( pInfo );

	// fill it
	if ( pInfo->GetFieldsCount()==0 && pInfo->GetAttrsCount()==0 )
	{
		*pInfo = m_tSchema;
		return true;
	}

	// check it
	return m_tSchema.CompareTo ( *pInfo, sError );
}


void CSphSource::Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	SetMinPrefixLen ( Max ( tSettings.RawMinPrefixLen(), 0 ) );
	m_iMinInfixLen = Max ( tSettings.m_iMinInfixLen, 0 );
	m_iMaxSubstringLen = Max ( tSettings.m_iMaxSubstringLen, 0 );
	m_iBoundaryStep = Max ( tSettings.m_iBoundaryStep, -1 );
	m_bIndexExactWords = tSettings.m_bIndexExactWords;
	m_iOvershortStep = Min ( Max ( tSettings.m_iOvershortStep, 0 ), 1 );
	m_iStopwordStep = Min ( Max ( tSettings.m_iStopwordStep, 0 ), 1 );
	m_bIndexSP = tSettings.m_bIndexSP;
	m_dPrefixFields = tSettings.m_dPrefixFields;
	m_dInfixFields = tSettings.m_dInfixFields;
	m_dStoredFields = tSettings.m_dStoredFields;
	m_dStoredOnlyFields = tSettings.m_dStoredOnlyFields;
	m_dColumnarAttrs = tSettings.m_dColumnarAttrs;
	m_dColumnarNonStoredAttrs = tSettings.m_dColumnarNonStoredAttrs;
	m_dRowwiseAttrs = tSettings.m_dRowwiseAttrs;
	m_dColumnarStringsNoHash = tSettings.m_dColumnarStringsNoHash;
	m_bIndexFieldLens = tSettings.m_bIndexFieldLens;
	m_eEngine = tSettings.m_eEngine;

	m_tSchema.SetupFlags ( *this, false, pWarnings );
}


bool CSphSource::SetupMorphFields ( CSphString & sError )
{
	return ParseMorphFields ( m_pDict->GetSettings().m_sMorphology, m_pDict->GetSettings().m_sMorphFields, m_tSchema.GetFields(), m_tMorphFields, sError );
}


ISphHits * CSphSource::IterateJoinedHits ( CSphReader & tReader, CSphString & )
{
	static ISphHits dDummy;
	m_tDocInfo.m_tRowID = INVALID_ROWID; // pretend that's an eof
	return &dDummy;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////
CSphSource::CSphBuildHitsState_t::CSphBuildHitsState_t ()
{
	Reset();
}

CSphSource::CSphBuildHitsState_t::~CSphBuildHitsState_t ()
{
	Reset();
}

void CSphSource::CSphBuildHitsState_t::Reset ()
{
	m_bProcessingHits = false;
	m_bDocumentDone = false;
	m_dFields = nullptr;
	m_dFieldLengths.Resize(0);
	m_iStartPos = 0;
	m_iHitPos = 0;
	m_iField = 0;
	m_iStartField = 0;
	m_iEndField = 0;
	m_iBuildLastStep = 1;

	ARRAY_FOREACH ( i, m_dTmpFieldStorage )
		SafeDeleteArray ( m_dTmpFieldStorage[i] );

	m_dTmpFieldStorage.Resize ( 0 );
	m_dTmpFieldPtrs.Resize ( 0 );
	m_dFiltered.Resize ( 0 );
}

CSphSource::CSphSource ( const char * sName )
	: m_iMaxHits ( MAX_SOURCE_HITS )
	, m_tSchema ( sName )
{
}

CSphSource::~CSphSource()
{
	SafeDeleteArray ( m_pReadFileBuffer );
	SafeDelete ( m_pStripper );
}


bool CSphSource::IterateDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_pTokenizer );
	assert ( !m_tState.m_bProcessingHits );

	m_tHits.Resize ( 0 );

	m_tState.Reset();
	m_tState.m_iEndField = m_iPlainFieldsLength;
	m_tState.m_dFieldLengths.Resize ( m_tState.m_iEndField );

	if ( m_pFieldFilter )
	{
		m_tState.m_dTmpFieldPtrs.Resize ( m_tState.m_iEndField );
		m_tState.m_dTmpFieldStorage.Resize ( m_tState.m_iEndField );

		ARRAY_FOREACH ( i, m_tState.m_dTmpFieldPtrs )
		{
			m_tState.m_dTmpFieldPtrs[i] = NULL;
			m_tState.m_dTmpFieldStorage[i] = NULL;
		}
	}

	m_dDocFields.Resize ( m_tSchema.GetFieldsCount() );
	for ( auto & i : m_dDocFields )
		i.Resize(0);

	// clean up field length counters
	if ( m_pFieldLengthAttrs )
		memset ( m_pFieldLengthAttrs, 0, sizeof ( DWORD ) * m_tSchema.GetFieldsCount() );

	// fetch next document
	while (true)
	{
		m_tState.m_dFields = NextDocument ( bEOF, sError );
		if ( bEOF )
			return ( sError.IsEmpty() );

		if ( !m_tState.m_dFields && !sError.IsEmpty() )
			return false;

		const int * pFieldLengths = GetFieldLengths ();
		for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
		{
			m_tState.m_dFieldLengths[iField] = pFieldLengths[iField];

			if ( m_tSchema.GetField(iField).m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
			{
				int iFieldLen = m_tState.m_dFieldLengths[iField];
				m_dDocFields[iField].Resize(iFieldLen);
				memcpy ( m_dDocFields[iField].Begin(), m_tState.m_dFields[iField], iFieldLen );
			}
		}

		// tricky bit
		// we can only skip document indexing from here, IterateHits() is too late
		// so in case the user chose to skip documents with file field problems
		// we need to check for those here
		if ( m_eOnFileFieldError==FFE_SKIP_DOCUMENT || m_eOnFileFieldError==FFE_FAIL_INDEX )
		{
			bool bOk = true;
			for ( int iField=0; iField<m_tState.m_iEndField && bOk; iField++ )
			{
				const BYTE * sFilename = m_tState.m_dFields[iField];
				if ( m_tSchema.GetField(iField).m_bFilename )
					bOk &= CheckFileField ( sFilename );

				if ( !bOk && m_eOnFileFieldError==FFE_FAIL_INDEX )
				{
					sError.SetSprintf ( "error reading file field data (docid=" INT64_FMT ", filename=%s)",	m_dAttrs[0], sFilename );
					return false;
				}
			}
			if ( !bOk && m_eOnFileFieldError==FFE_SKIP_DOCUMENT )
				continue;
		}

		if ( m_pFieldFilter )
		{
			bool bHaveModifiedFields = false;
			for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
			{
				if ( m_tSchema.GetField(iField).m_bFilename )
				{
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
					continue;
				}

				CSphVector<BYTE> dFiltered;
				int iFilteredLen = m_pFieldFilter->Apply ( m_tState.m_dFields[iField], m_tState.m_dFieldLengths[iField], dFiltered, false );
				if ( iFilteredLen )
				{
					m_tState.m_dTmpFieldStorage[iField] = dFiltered.LeakData();
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dTmpFieldStorage[iField];
					m_tState.m_dFieldLengths[iField] = iFilteredLen;
					bHaveModifiedFields = true;
				} else
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
			}

			if ( bHaveModifiedFields )
				m_tState.m_dFields = (BYTE **)&( m_tState.m_dTmpFieldPtrs[0] );
		}

		// we're good
		break;
	}

	++m_tStats.m_iTotalDocuments;
	return true;
}


// hack notification for joined fields
void CSphSource::RowIDAssigned ( DocID_t tDocID, RowID_t tRowID )
{
	if ( HasJoinedFields() )
	{
		IDPair_t & tPair = m_dAllIds.Add();
		tPair.m_tDocID = tDocID;
		tPair.m_tRowID = tRowID;
	}
}


ISphHits * CSphSource::IterateHits ( CSphString & sError )
{
	if ( m_tState.m_bDocumentDone )
		return NULL;

	m_tHits.Resize ( 0 );

	BuildHits ( sError, false );

	return &m_tHits;
}


bool CSphSource::CheckFileField ( const BYTE * sField )
{
	CSphAutofile tFileSource;
	CSphString sError;

	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return false;
	}

	return true;
}


/// returns file size on success, and replaces *ppField with a pointer to data
/// returns -1 on failure (and emits a warning)
int CSphSource::LoadFileField ( BYTE ** ppField, CSphString & sError )
{
	CSphAutofile tFileSource;

	BYTE * sField = *ppField;
	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return -1;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return -1;
	}

	int iFieldBytes = (int)iFileSize;
	if ( !iFieldBytes )
		return 0;

	int iBufSize = Max ( m_iReadFileBufferSize, 1 << sphLog2 ( iFieldBytes+15 ) );
	if ( m_iReadFileBufferSize < iBufSize )
		SafeDeleteArray ( m_pReadFileBuffer );

	if ( !m_pReadFileBuffer )
	{
		m_pReadFileBuffer = new char [ iBufSize ];
		m_iReadFileBufferSize = iBufSize;
	}

	if ( !tFileSource.Read ( m_pReadFileBuffer, iFieldBytes, sError ) )
	{
		sphWarning ( "read failed: %s", sError.cstr() );
		return -1;
	}

	m_pReadFileBuffer[iFieldBytes] = '\0';

	*ppField = (BYTE*)m_pReadFileBuffer;
	return iFieldBytes;
}


bool AddFieldLens ( CSphSchema & tSchema, bool bDynamic, CSphString & sError )
{
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		CSphColumnInfo tCol;
		tCol.m_sName.SetSprintf ( "%s_len", tSchema.GetFieldName(i) );

		int iGot = tSchema.GetAttrIndex ( tCol.m_sName.cstr() );
		if ( iGot>=0 )
		{
			if ( tSchema.GetAttr(iGot).m_eAttrType==SPH_ATTR_TOKENCOUNT )
			{
				// looks like we already added these
				assert ( tSchema.GetAttr(iGot).m_sName==tCol.m_sName );
				return true;
			}

			sError.SetSprintf ( "attribute %s conflicts with index_field_lengths=1; remove it", tCol.m_sName.cstr() );
			return false;
		}

		tCol.m_eAttrType = SPH_ATTR_TOKENCOUNT;
		tSchema.AddAttr ( tCol, bDynamic ); // everything's dynamic at indexing time
	}
	return true;
}


bool CSphSource::AddAutoAttrs ( CSphString & sError, StrVec_t * pDefaults )
{
	// id is the first attr
	if ( m_tSchema.GetAttr ( sphGetDocidName() ) )
	{
		assert ( m_tSchema.GetAttrIndex ( sphGetDocidName() )==0 );
		assert ( m_tSchema.GetAttr ( sphGetDocidName() )->m_eAttrType==SPH_ATTR_BIGINT );
	} else
	{
		CSphColumnInfo tCol ( sphGetDocidName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		for ( const auto & i : m_dColumnarAttrs )
			if ( i==tCol.m_sName )
				tCol.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR;

		m_tSchema.InsertAttr ( 0, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 0, "" );
	}

	if ( m_tSchema.HasBlobAttrs() && !m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		CSphColumnInfo tCol ( sphGetBlobLocatorName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		m_tSchema.InsertAttr ( 1, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 1, "" );
	}

	// rebuild locators in the schema
	const char * szTmpColName = "$_tmp";
	CSphColumnInfo tCol ( szTmpColName, SPH_ATTR_BIGINT );
	m_tSchema.AddAttr ( tCol, true );
	m_tSchema.RemoveAttr ( szTmpColName, true );

	// auto-computed length attributes
	if ( m_bIndexFieldLens && !AddFieldLens ( m_tSchema, true, sError ) )
		return false;

	return true;
}


void CSphSource::AllocDocinfo()
{
	// tricky bit
	// with in-config schema, attr storage gets allocated in Setup() when source is initially created
	// so when this AddAutoAttrs() additionally changes the count, we have to change the number of attributes
	// but Reset() prohibits that, because that is usually a programming mistake, hence the Swap() dance
	CSphMatch tNew;
	tNew.Reset ( m_tSchema.GetRowSize() );
	Swap ( m_tDocInfo, tNew );

	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );
	m_dAttrs.Resize ( m_tSchema.GetAttrsCount() );

	if ( m_bIndexFieldLens && m_tSchema.GetAttrsCount() && m_tSchema.GetFieldsCount() )
	{
		int iFirst = m_tSchema.GetAttrId_FirstFieldLen();
		assert ( m_tSchema.GetAttr ( iFirst ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		assert ( m_tSchema.GetAttr ( iFirst+m_tSchema.GetFieldsCount()-1 ).m_eAttrType==SPH_ATTR_TOKENCOUNT );

		m_pFieldLengthAttrs = m_tDocInfo.m_pDynamic + ( m_tSchema.GetAttr ( iFirst ).m_tLocator.m_iBitOffset / 32 );
	}
}

//////////////////////////////////////////////////////////////////////////
// HIT GENERATORS
//////////////////////////////////////////////////////////////////////////

bool CSphSource::BuildZoneHits ( RowID_t tRowID, BYTE uCode )
{
	switch (uCode)
	{
	case MAGIC_CODE_SENTENCE:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = 1;
		return true;
	case MAGIC_CODE_PARAGRAPH:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_PARAGRAPH) ), m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = 1;
		return true;
	case MAGIC_CODE_ZONE:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_PARAGRAPH) ), m_tState.m_iHitPos } );
		{
			BYTE * pZone = (BYTE*)const_cast<char *> ( m_pTokenizer->GetBufferPtr() );
			BYTE * pEnd = pZone;
			while ( *pEnd && *pEnd!=MAGIC_CODE_ZONE )
				++pEnd;

			if ( *pEnd==MAGIC_CODE_ZONE )
			{
				*pEnd = '\0';
				m_tHits.Add ( { tRowID, m_pDict->GetWordID ( pZone-1 ), m_tState.m_iHitPos } );
				m_pTokenizer->SetBufferPtr ( (const char *) pEnd+1 );
			}
		}
		m_tState.m_iBuildLastStep = 1;
		return true;
	default:
		return false;
	}
}

// track blended start and reset on not blended token
static int TrackBlendedStart ( const TokenizerRefPtr_c& pTokenizer, int iBlendedHitsStart, int iHitsCount )
{
	if ( pTokenizer->TokenIsBlended() )
		return iHitsCount;

	return pTokenizer->TokenIsBlendedPart () ? iBlendedHitsStart : -1;
}

#define BUILD_SUBSTRING_HITS_COUNT 4

void CSphSource::BuildSubstringHits ( RowID_t tRowID, bool bPayload, ESphWordpart eWordpart, int & iBlendedHitsStart )
{
	bool bPrefixField = ( eWordpart==SPH_WORDPART_PREFIX );
	bool bInfixMode = m_iMinInfixLen > 0;

	int iMinInfixLen = bPrefixField ? GetMinPrefixLen ( false ) : m_iMinInfixLen;
	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iIterHitCount = BUILD_SUBSTRING_HITS_COUNT;
	if ( bPrefixField )
		iIterHitCount += SPH_MAX_WORD_LEN - GetMinPrefixLen ( false );
	else
		iIterHitCount += ( ( m_iMinInfixLen+SPH_MAX_WORD_LEN ) * ( SPH_MAX_WORD_LEN-m_iMinInfixLen ) / 2 );

	// FIELDEND_MASK at blended token stream should be set for HEAD token too
	iBlendedHitsStart = -1;

	// index all infixes
	while ( ( m_iMaxHits==0 || m_tHits.GetLength()+iIterHitCount<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.GetLength() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, Max ( m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep, 0 ) );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
			m_tState.m_iBuildLastStep = 1;
		}

		if ( BuildZoneHits ( tRowID, *sWord ) )
			continue;

		int iLen = m_pTokenizer->GetLastTokenLen ();

		// always index full word (with magic head/tail marker(s))
		auto iBytes = (int) strlen ( (const char*)sWord );
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = '\0';

		SphWordID_t uExactWordid = 0;
		if ( m_bIndexExactWords )
		{
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			uExactWordid = m_pDict->GetWordIDNonStemmed ( sBuf );
		}

		sBuf[0] = MAGIC_WORD_HEAD;

		// stemmed word w/markers
		SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
		if ( !iWord )
		{
			m_tState.m_iBuildLastStep = m_iStopwordStep;
			continue;
		}

		if ( m_bIndexExactWords )
			m_tHits.Add ( { tRowID, uExactWordid, m_tState.m_iHitPos } );
		iBlendedHitsStart = iLastBlendedStart;
		m_tHits.Add ( { tRowID, iWord, m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;

		// restore stemmed word
		auto iStemmedLen = (int) strlen ( ( const char *)sBuf );
		sBuf [iStemmedLen - 1] = '\0';

		// stemmed word w/o markers
		if ( strcmp ( (const char *)sBuf + 1, (const char *)sWord ) )
			m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sBuf + 1, iStemmedLen - 2, true ), m_tState.m_iHitPos } );

		// restore word
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = MAGIC_WORD_TAIL;
		sBuf[iBytes+2] = '\0';

		// if there are no infixes, that's it
		if ( iMinInfixLen > iLen )
		{
			// index full word
			m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sWord ), m_tState.m_iHitPos } );
			continue;
		}

		// process all infixes
		int iMaxStart = bPrefixField ? 0 : ( iLen - iMinInfixLen );

		BYTE * sInfix = sBuf + 1;

		for ( int iStart=0; iStart<=iMaxStart; iStart++ )
		{
			BYTE * sInfixEnd = sInfix;
			for ( int i = 0; i < iMinInfixLen; i++ )
				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );

			int iMaxSubLen = ( iLen-iStart );
			if ( m_iMaxSubstringLen )
				iMaxSubLen = Min ( m_iMaxSubstringLen, iMaxSubLen );

			for ( int i=iMinInfixLen; i<=iMaxSubLen; i++ )
			{
				m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix, int ( sInfixEnd-sInfix ), false ), m_tState.m_iHitPos } );

				// word start: add magic head
				if ( bInfixMode && iStart==0 )
					m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix - 1, int ( sInfixEnd-sInfix ) + 1, false ), m_tState.m_iHitPos } );

				// word end: add magic tail
				if ( bInfixMode && i==iLen-iStart )
					m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix, int ( sInfixEnd-sInfix ) + 1, false ), m_tState.m_iHitPos } );

				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );
			}

			sInfix += m_pTokenizer->GetCodepointLength ( *sInfix );
		}
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );
}


#define BUILD_REGULAR_HITS_COUNT 6

void CSphSource::BuildRegularHits ( RowID_t tRowID, bool bPayload, int & iBlendedHitsStart )
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;
	bool bGlobalPartialMatch = !bWordDict && ( GetMinPrefixLen ( bWordDict ) > 0 || m_iMinInfixLen > 0 );

	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	// FIELDEND_MASK at last token stream should be set for HEAD token too
	iBlendedHitsStart = -1;

	// bMorphDisabled introduced in e0f8754e
	bool bMorphDisabled = !m_tMorphFields.BitGetOr ( m_tState.m_iField, true );

	// index words only
	while ( ( m_iMaxHits==0 || m_tHits.GetLength()+BUILD_REGULAR_HITS_COUNT<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.GetLength() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, Max ( m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep, 0 ) );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
		}

		if ( BuildZoneHits ( tRowID, *sWord ) )
			continue;

		if ( bGlobalPartialMatch )
		{
			auto iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD;
			sBuf[iBytes+1] = '\0';
			m_tHits.Add ( { tRowID, m_pDict->GetWordIDWithMarkers ( sBuf ), m_tState.m_iHitPos } );
		}

		ESphTokenMorph eMorph = m_pTokenizer->GetTokenMorph();
		if ( m_bIndexExactWords && eMorph != SPH_TOKEN_MORPH_GUESS )
		{
			auto iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			sBuf[iBytes + 1] = '\0';

			if ( eMorph == SPH_TOKEN_MORPH_ORIGINAL || bMorphDisabled )
			{
				// can not use GetWordID here due to exception vs missed hit, ie
				// stemmed sWord hasn't got added to hit stream but might be added as exception to dictionary
				// that causes error at hit sorting phase \ dictionary HitblockPatch
				if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed )
					m_pDict->ApplyStemmers ( sWord );

				if ( !m_pDict->IsStopWord ( sWord ) )
					m_tHits.Add ( { tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos } );

				m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
				continue;
			}
		}

		SphWordID_t iWord = ( eMorph==SPH_TOKEN_MORPH_GUESS )
			? m_pDict->GetWordIDNonStemmed ( sWord ) // tokenizer did morphology => dict must not stem
			: m_pDict->GetWordID ( sWord ); // tokenizer did not => stemmers can be applied
		if ( iWord )
		{
#if 0
			if ( HITMAN::GetPos ( m_tState.m_iHitPos )==1 )
				printf ( "\n" );
			printf ( "doc %d. pos %d. %s\n", uDocid, HITMAN::GetPos ( m_tState.m_iHitPos ), sWord );
#endif
			iBlendedHitsStart = iLastBlendedStart;
			m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
			m_tHits.Add ( { tRowID, iWord, m_tState.m_iHitPos } );
			if ( m_bIndexExactWords && eMorph!=SPH_TOKEN_MORPH_GUESS )
				m_tHits.Add ( { tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos } );
		} else
		{
			// need to count all blended part tokens to match query
			m_tState.m_iBuildLastStep = ( m_pTokenizer->TokenIsBlendedPart() ? 1 : m_iStopwordStep );
		}
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );
}

static void CountFieldLengths ( const VecTraits_T<CSphWordHit> & dHits, DWORD * pFieldLengthAttrs )
{
	const CSphWordHit * pHit = dHits.Begin();
	if ( !pHit )
		return;

	const CSphWordHit * pEnd = dHits.End();
	assert ( pEnd );
	Hitpos_t uLastHit = pHit->m_uWordPos;
	DWORD uLastCount = 1;

	for ( ; pHit!=pEnd; pHit++ )
	{
		if ( HITMAN::GetField ( uLastHit )!=HITMAN::GetField ( pHit->m_uWordPos ) )
		{
			pFieldLengthAttrs [ HITMAN::GetField ( uLastHit ) ] += uLastCount;
			uLastCount = 1;
			uLastHit = pHit->m_uWordPos;
		}

		// skip blended part, lemmas and duplicates
		if ( HITMAN::GetPos ( pHit->m_uWordPos )>HITMAN::GetPos ( uLastHit ) )
		{
			uLastHit = pHit->m_uWordPos;
			uLastCount++;
		}
	}

	if ( uLastCount )
	{
		pFieldLengthAttrs [ HITMAN::GetField ( uLastHit ) ] += uLastCount;
	}
}

static void ProcessCollectedHits ( VecTraits_T<CSphWordHit> & dHits, int iHitsBegin, bool bMarkTail, int iBlendedHitsStart, bool bHasStopwords, DWORD * pFieldLengthAttrs )
{
	// mark trailing hit
	// and compute field lengths
	if ( bMarkTail )
	{
		auto * pTail = const_cast < CSphWordHit * > ( &dHits.Last() );

		if ( pFieldLengthAttrs && !bHasStopwords )
			pFieldLengthAttrs [ HITMAN::GetField ( pTail->m_uWordPos ) ] = HITMAN::GetPos ( pTail->m_uWordPos );

		Hitpos_t uEndPos = pTail->m_uWordPos;
		if ( iBlendedHitsStart>=0 )
		{
			assert ( iBlendedHitsStart>=0 && iBlendedHitsStart<dHits.GetLength() );
			Hitpos_t uBlendedPos = dHits[iBlendedHitsStart].m_uWordPos;
			uEndPos = Min ( uEndPos, uBlendedPos );
		}

		// set end marker for all tail hits
		const CSphWordHit * pStart = dHits.Begin();
		while ( pStart<=pTail && uEndPos<=pTail->m_uWordPos )
		{
			HITMAN::SetEndMarker ( &pTail->m_uWordPos );
			--pTail;
		}
	}

	// for stopwords need to process whole stream of collected tokens
	if ( pFieldLengthAttrs && bHasStopwords )
		CountFieldLengths ( VecTraits_T<CSphWordHit> ( dHits.Begin()+iHitsBegin, dHits.GetLength()-iHitsBegin ), pFieldLengthAttrs );

}

void CSphSource::BuildHits ( CSphString & sError, bool bSkipEndMarker )
{
	RowID_t tRowID = m_tDocInfo.m_tRowID;

	for ( ; m_tState.m_iField<m_tState.m_iEndField; m_tState.m_iField++ )
	{
		if ( !m_tState.m_bProcessingHits )
		{
			// get that field
			BYTE * sField = m_tState.m_dFields[m_tState.m_iField-m_tState.m_iStartField];
			int iFieldBytes = m_tState.m_dFieldLengths[m_tState.m_iField-m_tState.m_iStartField];
			if ( !sField || !(*sField) || !iFieldBytes )
				continue;

			// load files
			const BYTE * sTextToIndex;
			const CSphColumnInfo & tField = m_tSchema.GetField(m_tState.m_iField);
			if ( tField.m_bFilename )
			{
				LoadFileField ( &sField, sError );
				sTextToIndex = sField;
				iFieldBytes = (int) strlen ( (char*)sField );

				if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
				{
					m_dDocFields[m_tState.m_iField].Resize(iFieldBytes);
					memcpy ( m_dDocFields[m_tState.m_iField].Begin(), sField, iFieldBytes );
				}

				if ( m_pFieldFilter && iFieldBytes )
				{
					m_tState.m_dFiltered.Resize ( 0 );
					int iFiltered = m_pFieldFilter->Apply ( sTextToIndex, iFieldBytes, m_tState.m_dFiltered, false );
					if ( iFiltered )
					{
						sTextToIndex = m_tState.m_dFiltered.Begin();
						iFieldBytes = iFiltered;
					}
				}
			} else
				sTextToIndex = sField;

			if ( iFieldBytes<=0 )
				continue;

			// strip html
			if ( m_pStripper )
			{
				m_pStripper->Strip ( const_cast<BYTE*>(sTextToIndex) );
				iFieldBytes = (int) strlen ( (char*)const_cast<BYTE*>(sTextToIndex) );
			}

			// tokenize and build hits
			m_tStats.m_iTotalBytes += iFieldBytes;

			m_pTokenizer->BeginField ( m_tState.m_iField );
			m_pTokenizer->SetBuffer ( const_cast<BYTE*> ( sTextToIndex ), iFieldBytes );

			m_tState.m_iHitPos = HITMAN::Create ( m_tState.m_iField, m_tState.m_iStartPos );
		}

		const CSphColumnInfo & tField = m_tSchema.GetField ( m_tState.m_iField );

		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
		{
			int iBlendedHitsStart = -1;
			int iHitsBegin = m_tHits.GetLength();

			if ( tField.m_eWordpart!=SPH_WORDPART_WHOLE )
				BuildSubstringHits ( tRowID, tField.m_bPayload, tField.m_eWordpart, iBlendedHitsStart );
			else
				BuildRegularHits ( tRowID, tField.m_bPayload, iBlendedHitsStart );

			ProcessCollectedHits ( m_tHits, iHitsBegin, ( !bSkipEndMarker && !m_tState.m_bProcessingHits && m_tHits.GetLength() ), iBlendedHitsStart, !m_pDict->GetSettings().m_sStopwords.IsEmpty(), m_pFieldLengthAttrs );
		}

		if ( m_tState.m_bProcessingHits )
			break;
	}

	m_tState.m_bDocumentDone = !m_tState.m_bProcessingHits;
}

//////////////////////////////////////////////////////////////////////////

CSphVector<int64_t> * CSphSource::GetFieldMVA ( int iAttr )
{
	return &m_dMvas[iAttr];
}


const CSphString & CSphSource::GetStrAttr ( int iAttr )
{
	return m_dStrAttrs[iAttr];
}


SphAttr_t CSphSource::GetAttr ( int iAttr )
{
	return m_dAttrs[iAttr];
}


void CSphSource::GetDocFields ( CSphVector<VecTraits_T<BYTE>> & dFields )
{
	dFields.Resize ( m_dDocFields.GetLength() );
	ARRAY_FOREACH ( i, m_dDocFields )
		dFields[i] = VecTraits_T<BYTE>( m_dDocFields[i].Begin(), m_dDocFields[i].GetLength() );
}


void CSphSource::ParseFieldMVA ( int iAttr, const char * szValue )
{
	if ( !szValue )
		return;

	const char * pPtr = szValue;
	const char * pDigit = NULL;
	const int MAX_NUMBER_LEN = 64;
	char szBuf [MAX_NUMBER_LEN];

	while ( *pPtr )
	{
		if ( ( *pPtr>='0' && *pPtr<='9' ) || *pPtr=='-' )
		{
			if ( !pDigit )
				pDigit = pPtr;
		} else
		{
			if ( pDigit )
			{
				if ( pPtr - pDigit < MAX_NUMBER_LEN )
				{
					strncpy ( szBuf, pDigit, pPtr - pDigit );
					szBuf [pPtr - pDigit] = '\0';
					m_dMvas[iAttr].Add ( sphToInt64 ( szBuf ) );
				}

				pDigit = NULL;
			}
		}

		pPtr++;
	}

	if ( pDigit )
		m_dMvas[iAttr].Add ( sphToInt64 ( pDigit ) );
}
