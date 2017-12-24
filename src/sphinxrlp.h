//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxrlp_
#define _sphinxrlp_

#include "sphinxstd.h"

//////////////////////////////////////////////////////////////////////////

#if USE_RLP

extern CSphString g_sRLPRoot;
extern CSphString g_sRLPEnv;
extern int g_iRLPMaxBatchSize;
extern int g_iRLPMaxBatchDocs;

struct StoredDoc_t
{
	CSphMatch							m_tDocInfo;
	CSphVector<CSphString>				m_dStrAttrs;
	CSphVector<DWORD>					m_dMva;
	CSphTightVector<BYTE*>				m_dFields;
	CSphVector<int>						m_dFieldLengths;
	CSphTightVector< CSphVector<BYTE> >	m_dFieldStorage;
};

// these are used to separate text before passing it to RLP
const int PROXY_DOCUMENT_START = 0xFFFA;
const int PROXY_FIELD_START = 0xFFFB;
const int PROXY_CHUNK_SEPARATOR = 0xFFFC;

const int PROXY_MARKER_LEN = 3;

inline static void COPY_MARKER (BYTE * _ptr, const BYTE * _marker)
{
	(_ptr)[0] = ' ';
    (_ptr)[1] = (_marker)[0];
    (_ptr)[2] = (_marker)[1];
    (_ptr)[3] = (_marker)[2];
    (_ptr)[4] = ' ';
}

inline static void APPEND_MARKER( BYTE * &_ptr, const BYTE * _marker )
{
    *(_ptr)++ = ' ';
    *(_ptr)++ = (_marker)[0];
    *(_ptr)++ = (_marker)[1];
    *(_ptr)++ = (_marker)[2];
    *(_ptr)++ = ' ';
}

inline static bool CMP_MARKER( const BYTE * _ptr, const BYTE * _marker)
{
	return (_ptr)[0]==(_marker)[0]
		&& (_ptr)[1]==(_marker)[1]
		&& (_ptr)[2]==(_marker)[2];
}

const int INITIAL_BUFFER_SIZE = 1024 * 1024; // typical default top is 50K, but we reserve 1M.

// proxy source
template <class T>
class CSphSource_Proxy : public T
{
	const int MAX_INDEX_LEN = 8;
public:
	explicit CSphSource_Proxy ( const char * sSourceName )
		: T ( sSourceName )
		, m_dBatchedDocs ( g_iRLPMaxBatchDocs )
	{
		Verify ( sphUTF8Encode ( m_pMarkerDocStart, PROXY_DOCUMENT_START )==PROXY_MARKER_LEN );
		Verify ( sphUTF8Encode ( m_pMarkerFieldStart, PROXY_FIELD_START )==PROXY_MARKER_LEN );
		m_dDocBuffer.Reserve ( INITIAL_BUFFER_SIZE );
	}

	// we run field filters on batches of documents, therefore we don't want
	// CSphSource_Document to run them again in IterateDocument 
	void SetFieldFilter ( ISphFieldFilter * pFilter ) final
	{
		m_pBatchFieldFilter = pFilter;
	}


	BYTE ** NextDocument ( CSphString & sError ) final
	{
		if ( !IsDocCacheEmpty() )
			return CopyDoc();

		if ( m_dFieldLengths.GetLength()!=T::m_tSchema.GetFieldsCount() )
		{
			int nFields = T::m_tSchema.GetFieldsCount();
			m_dFieldLengths.Resize ( nFields );
			m_dFieldHasChinese.Resize ( nFields );
		}

		m_iDocStart = 0;
		m_dDocBuffer.Resize(0);

		while ( !IsDocCacheFull() && m_dDocBuffer.GetLength()<g_iRLPMaxBatchSize )
		{
			BYTE ** pFields = T::NextDocument ( sError );
			if ( !pFields )
				break;

			const int * pFieldLengths = T::GetFieldLengths();

			int iTotalToDecode = 0;
			bool bDocHasChinese = false;
			for ( int i = 0; i < T::m_tSchema.GetFieldsCount(); ++i )
			{
				m_dFieldLengths[i] = pFieldLengths[i];
				m_dFieldHasChinese[i] = sphDetectChinese ( pFields[i], m_dFieldLengths[i] );
				if ( m_dFieldHasChinese[i] )
				{
					bDocHasChinese = true;
					iTotalToDecode += PROXY_MARKER_LEN+MAX_INDEX_LEN+m_dFieldLengths[i]+3;	// _[marker]_[field_id]_[field text]
				}
			}

			// no point in caching the document if its the first document and there's nothing CJK in it
			if ( IsDocCacheEmpty() && !bDocHasChinese )
			{
				T::m_tState.m_dFieldLengths.Resize ( m_dFieldLengths.GetLength() );
				ARRAY_FOREACH ( i, m_dFieldLengths )
					T::m_tState.m_dFieldLengths[i] = m_dFieldLengths[i];

				return pFields;
			}

			int iCurDoc;
			StoredDoc_t * pDoc = Push_back ( &iCurDoc );
			int nFields = T::m_tSchema.GetFieldsCount();
			CopyDocInfo ( pDoc->m_tDocInfo, T::m_tDocInfo );
			pDoc->m_dMva = T::m_dMva;
			pDoc->m_dStrAttrs = T::m_dStrAttrs;
			pDoc->m_dFields.Resize ( nFields );
			pDoc->m_dFieldLengths.Resize ( nFields );
			pDoc->m_dFieldStorage.Resize ( nFields );

			// fields that don't have chinese text will be stored in those docs
			// fields that have chinese text will be added later, after a RLP field filter pass
			for ( int i = 0; i < T::m_tSchema.GetFieldsCount(); ++i )
				if ( !m_dFieldHasChinese[i] )
				{
					int iFieldLength = m_dFieldLengths[i];
					pDoc->m_dFieldStorage[i].Resize ( iFieldLength+1 );
					BYTE * pStart = pDoc->m_dFieldStorage[i].Begin();
					memcpy ( pStart, pFields[i], iFieldLength );
					pDoc->m_dFields[i] = pStart;
					pDoc->m_dFieldLengths[i] = iFieldLength;

					// stripper still doesn't know about field lengths, so we need to add a zero
					pDoc->m_dFields[i][iFieldLength] = '\0';
				}

			// document doesnt have any CJK, so no copying/segmenting/etc
			if ( !bDocHasChinese )
				continue;

			BYTE * pCurDocPtr = m_dDocBuffer.AddN ( PROXY_MARKER_LEN + MAX_INDEX_LEN + 2 + iTotalToDecode);

			// document start tag
			APPEND_MARKER ( pCurDocPtr, m_pMarkerDocStart );

			// document id
			AddNumber ( pCurDocPtr, iCurDoc );

			// flatten all fields(+markers) to one buffer
			for ( int i = 0; i < T::m_tSchema.GetFieldsCount(); i++ )
				if ( m_dFieldHasChinese[i] )
				{
					APPEND_MARKER ( pCurDocPtr, m_pMarkerFieldStart );

					// field id
					AddNumber ( pCurDocPtr, i );
					*pCurDocPtr++ = ' ';

					int iFieldLen = m_dFieldLengths[i];
					memcpy ( pCurDocPtr, pFields[i], iFieldLen );
					pCurDocPtr += iFieldLen;
				}

			m_dDocBuffer.Resize ( pCurDocPtr-m_dDocBuffer.Begin() );
		}

		if ( IsDocCacheEmpty() )
			return nullptr;

		assert ( m_pBatchFieldFilter && !T::m_pFieldFilter );
		int iResultLen = m_pBatchFieldFilter->Apply ( m_dDocBuffer.Begin(), m_dDocBuffer.GetLength(), m_dResult, false );
		assert ( iResultLen );

		BYTE * pSegmentedStart = m_dResult.Begin();
		BYTE * pSegmentedEnd = pSegmentedStart+iResultLen;
		BYTE * pFieldStart = nullptr;
		StoredDoc_t * pCurDoc = nullptr;
		int iFieldId = -1;

		while ( pSegmentedStart < pSegmentedEnd )
		{
			BYTE * pTmp = pSegmentedStart;
			int iCode = sphUTF8Decode ( (const BYTE * &)pSegmentedStart );

			if ( iCode!=PROXY_DOCUMENT_START && iCode!=PROXY_FIELD_START )
				continue;

			// we have a segmented field that we need to place into the appropriate cached document
			if ( pFieldStart && pCurDoc )
			{
				assert ( iFieldId!=-1 );

				pCurDoc->m_dFields[iFieldId] = pFieldStart;
				pCurDoc->m_dFieldLengths[iFieldId] = pTmp-pFieldStart;

				// stripper still doesn't know about field lengths, so we need to add a zero
				pTmp[-1] = '\0';
			}

			if ( iCode==PROXY_DOCUMENT_START )
			{
				// fetch document number
				int iDoc = ReadNumber ( pSegmentedStart, pSegmentedEnd );
				pCurDoc = &(m_dBatchedDocs[iDoc]);
				iFieldId = -1;
				pFieldStart = nullptr;

			} else //if ( iCode==PROXY_FIELD_START )
			{
				// fetch field id
				iFieldId = ReadNumber ( pSegmentedStart, pSegmentedEnd );
				pFieldStart = pSegmentedStart;
			}
		}

		// last field
		if ( pFieldStart )
		{
			assert ( pCurDoc && iFieldId!=-1 );

			pCurDoc->m_dFields[iFieldId] = pFieldStart;
			pCurDoc->m_dFieldLengths[iFieldId] = pSegmentedEnd-pFieldStart;
		}

		return CopyDoc ();
	}


	const int *	GetFieldLengths () const final
	{
		return T::m_tState.m_dFieldLengths.Begin();
	}

private:
	CSphFixedVector<StoredDoc_t> m_dBatchedDocs;
	CSphTightVector<BYTE>	m_dDocBuffer;
	CSphVector<BYTE>		m_dResult;
	CSphTightVector<int>	m_dFieldLengths;
	CSphVector<bool>		m_dFieldHasChinese;
	int						m_iDocStart = 0;
	int						m_iDocCount = 0;
	ISphFieldFilter *		m_pBatchFieldFilter = nullptr;

	BYTE					m_pMarkerDocStart[PROXY_MARKER_LEN];
	BYTE					m_pMarkerFieldStart[PROXY_MARKER_LEN];

	bool					IsDocCacheEmpty() const	{ return !m_iDocCount; }
	bool					IsDocCacheFull() const { return m_iDocCount==g_iRLPMaxBatchDocs; }

	StoredDoc_t * Push_back ( int * pId )
	{
		assert ( pId );
		assert ( !IsDocCacheFull() );

		*pId = (m_iDocStart+m_iDocCount) % g_iRLPMaxBatchDocs;
		++m_iDocCount;

		return &(m_dBatchedDocs[*pId]);
	}

	StoredDoc_t * Pop_front ()
	{
		if ( IsDocCacheEmpty() )
			return nullptr;

		auto * pDoc = &(m_dBatchedDocs[m_iDocStart]);
		m_iDocStart = (m_iDocStart+1) % g_iRLPMaxBatchDocs;
		--m_iDocCount;
		return pDoc;
	}

	BYTE ** CopyDoc ()
	{
		StoredDoc_t * pDoc = Pop_front ();
		if (!pDoc )
			return nullptr;

		CopyDocInfo ( T::m_tDocInfo, pDoc->m_tDocInfo );
		T::m_tState.m_dFields = pDoc->m_dFields.Begin();
		T::m_tState.m_dFieldLengths.SwapData ( pDoc->m_dFieldLengths );
		T::m_dMva.SwapData ( pDoc->m_dMva );
		T::m_dStrAttrs.SwapData ( pDoc->m_dStrAttrs );

		return T::m_tState.m_dFields;
	}

	void CopyDocInfo ( CSphMatch & tTo, const CSphMatch & tFrom )
	{
		if ( tFrom.m_pDynamic )
		{
			int iDynamic = T::m_tSchema.GetRowSize();

			if ( !tTo.m_pDynamic )
				tTo.Reset ( iDynamic );

			if ( tTo.m_pDynamic )
				memcpy ( tTo.m_pDynamic, tFrom.m_pDynamic, iDynamic * sizeof ( CSphRowitem ) );
		}

		tTo.m_pStatic = nullptr;
		tTo.m_uDocID = tFrom.m_uDocID;
		tTo.m_iWeight = tFrom.m_iWeight;
		tTo.m_iTag = tFrom.m_iTag;
	}

	void AddNumber ( BYTE * & pPtr, int iNumber )
	{
		sphUItoA ( ( char ** ) &pPtr, iNumber );
	}

	int ReadNumber ( BYTE * & pPtr, BYTE * const pEndPtr ) // expect to read ' ', uint num, ' ' from utf8 buffer
	{
		auto & pPtrc = (const BYTE *&) pPtr;
		int iCode = sphUTF8Decode ( pPtrc );
		assert ( iCode==' ' );
		int iDoc = 0;
		for ( iCode = sphUTF8Decode ( pPtrc ); pPtrc<pEndPtr && iCode!=' '; iCode = sphUTF8Decode ( pPtrc ) )
			iDoc = iDoc * 10 + ( iCode - '0' );
		assert ( iCode==' ' );
		return iDoc;
	}
};

template<typename T>
T * CreateSourceWithProxy ( const char * sSourceName, bool bProxy )
{
	if ( bProxy )
		return new CSphSource_Proxy<T> ( sSourceName );

	return new T ( sSourceName );
}

#else // USE_RLP

template<typename T>
T * CreateSourceWithProxy ( const char * sSourceName, bool )
{
	return new T ( sSourceName );
}


#endif // USE_RLP


bool sphRLPCheckConfig ( CSphIndexSettings & tSettings, CSphString & sError );
bool sphSpawnRLPFilter ( ISphFieldFilter * & pFieldFilter, const CSphIndexSettings & m_tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError );
void sphConfigureRLP ( CSphConfigSection & hCommon );
void sphRLPDone();


#endif // _sphinxrlp_

//
// $Id$
//
