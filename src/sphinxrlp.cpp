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

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxrlp.h"


#if USE_RLP

#include "bt_rlp_c.h"
#include <bt_xwchar.h>

#if ( USE_WINDOWS )
	#pragma comment(linker, "/defaultlib:btrlpc.lib")
	#pragma message("Automatically linking with btrlpc.lib")
	#pragma comment(linker, "/defaultlib:btutils.lib")
	#pragma message("Automatically linking with btutils.lib")
#endif


#ifndef SHAREDIR
#define SHAREDIR "."
#endif


CSphString				g_sRLPRoot				= SHAREDIR;
CSphString				g_sRLPEnv				= SHAREDIR"/rlp-environment.xml";
int						g_iRLPMaxBatchSize		= 51200;
int						g_iRLPMaxBatchDocs		= 50;

BT_RLP_EnvironmentC *	g_pRLPEnv = NULL;
int						g_iRLPEnvRefCount = 0;


static void RLPLog ( void *, int iChannel, const char * szMessage )
{
	switch ( iChannel )
	{
	case 0:
		sphWarning ( "%s", szMessage );
		break;

	case 1:
		sphLogFatal ( "%s", szMessage );
		break;

	default:
		sphInfo ( "%s", szMessage );
		break;
	}
}


static bool sphRLPInit ( const char * szRootPath, const char * szEnvPath, CSphString & sError )
{
	if ( !g_pRLPEnv )
	{
		if ( !BT_RLP_CLibrary_VersionIsCompatible ( BT_RLP_CLIBRARY_INTERFACE_VERSION ) )
		{
			sError.SetSprintf ( "RLP library mismatch: have %ld expect %d", BT_RLP_CLibrary_VersionNumber(), BT_RLP_CLIBRARY_INTERFACE_VERSION );
			return false;
		}

		BT_RLP_Environment_SetBTRootDirectory ( szRootPath );
		BT_RLP_Environment_SetLogCallbackFunction ( NULL, RLPLog );
		BT_RLP_Environment_SetLogLevel ( "error" );

		g_pRLPEnv = BT_RLP_Environment_Create();
		if ( !g_pRLPEnv )
		{
			sError = "Unable to initialize RLP environment";
			return false;
		}

		BT_Result iRes = BT_RLP_Environment_InitializeFromFile ( g_pRLPEnv, szEnvPath );
		if ( iRes!=BT_OK )
		{
			sError = "Unable to initialize the RLP environment";
			BT_RLP_Environment_Destroy ( g_pRLPEnv );
			g_pRLPEnv = NULL;
			return false;
		}
	}

	g_iRLPEnvRefCount++;
	return true;
}


static void sphRLPFree ()
{
	g_iRLPEnvRefCount--;
	if ( !g_iRLPEnvRefCount )
	{
		assert ( g_pRLPEnv );
		BT_RLP_Environment_Destroy ( g_pRLPEnv );
		g_pRLPEnv = NULL;
	}
}


class CSphRLPPreprocessor
{
public:
	CSphRLPPreprocessor ( const char * szRootPath, const char * szEnvPath, const char * szCtxPath )
		: m_pContext ( NULL )
		, m_pFactory ( NULL )
		, m_pTokenIterator ( NULL )
		, m_iNextCompoundComponent ( -1 )
		, m_sRootPath ( szRootPath )
		, m_sEnvPath ( szEnvPath )
		, m_sCtxPath ( szCtxPath )
		, m_bInitialized ( false )
	{
		sphUTF8Encode ( m_pMarkerChunkSeparator, PROXY_CHUNK_SEPARATOR );
	}

	virtual ~CSphRLPPreprocessor()
	{
		if ( m_pTokenIterator )
			BT_RLP_TokenIterator_Destroy ( m_pTokenIterator );

		if ( m_pFactory )
			BT_RLP_TokenIteratorFactory_Destroy ( m_pFactory );

		if ( m_pContext )
			BT_RLP_Environment_DestroyContext ( g_pRLPEnv, m_pContext );

		sphRLPFree();
	}

	bool Init ( CSphString & sError )
	{
		assert ( !m_bInitialized );

		if ( !sphRLPInit ( m_sRootPath.cstr(), m_sEnvPath.cstr(), sError ) )
			return false;

		assert ( g_pRLPEnv );

		BT_Result iRes = BT_RLP_Environment_GetContextFromFile ( g_pRLPEnv, m_sCtxPath.cstr(), &m_pContext );
		if ( iRes!=BT_OK )
		{
			sError = "Unable to create RLP context";
			return false;
		}

		m_pFactory = BT_RLP_TokenIteratorFactory_Create();
		if ( !m_pFactory )
		{
			sError = "Unable to create RLP token iterator factory";
			return false;
		}

		BT_RLP_TokenIteratorFactory_SetReturnCompoundComponents ( m_pFactory, true );

		m_bInitialized = true;

		return true;
	}

	bool Process ( const BYTE * pBuffer, int iLength, CSphVector<BYTE> & dOut, bool bQuery )
	{
		if ( !pBuffer || !iLength )
			return false;

		if ( !sphDetectChinese ( pBuffer, iLength ) )
			return false;

		dOut.Resize(0);

		const BYTE * pSegment = pBuffer;
		int iLengthLeft = iLength;
		while ( iLengthLeft )
		{
			int iSegmentLength = GetNextLengthToSegment ( pSegment, iLengthLeft );
			if ( iSegmentLength )
				ProcessSegment ( pSegment, iSegmentLength, dOut, bQuery );

			pSegment += iSegmentLength;
			iLengthLeft -= iSegmentLength;
		}

		return true;
	}

	bool SetBlendChars ( const char * szBlendChars, CSphString & sError )
	{
		m_sBlendChars = szBlendChars;
		CSphCharsetDefinitionParser tParser;
		if ( !tParser.Parse ( szBlendChars, m_dBlendChars ) )
		{
			sError = tParser.GetLastError();
			return false;
		}

		return true;
	}

protected:
	CSphString				m_sBlendChars;
	CSphString				m_sRootPath;
	CSphString				m_sEnvPath;
	CSphString				m_sCtxPath;

private:
	struct TextChunk_t
	{
		int m_iStart;
		int m_iLength;
	};

	static const int		MAX_TOKEN_LEN = 1024;
	static const int		MAX_CHUNK_SIZE = 10485760;

	BT_RLP_ContextC *		m_pContext;
	BT_RLP_TokenIteratorFactoryC * m_pFactory;
	BT_RLP_TokenIteratorC *	m_pTokenIterator;
	int						m_iNextCompoundComponent;
	bool					m_bInitialized;
	CSphTightVector<BYTE>	m_dCJKBuffer;
	CSphTightVector<BYTE>	m_dNonCJKBuffer;
	CSphTightVector<TextChunk_t> m_dNonCJKChunks;
	BYTE					m_dUTF8Buffer[MAX_TOKEN_LEN];
	BYTE					m_pMarkerChunkSeparator[PROXY_MARKER_LEN];
	CSphVector<CSphRemapRange> m_dBlendChars;


	void ProcessSegment ( const BYTE * pIn, int iLen, CSphVector<BYTE> & dOut, bool bQuery )
	{
		assert ( pIn && iLen );

		m_dCJKBuffer.Resize(0);
		m_dNonCJKBuffer.Resize(0);
		m_dNonCJKChunks.Resize(0);

		const BYTE * pBuffer = pIn;
		const BYTE * pBufferMax = pIn+iLen;

		bool bWasChineseCode = false;
		const BYTE * pChunkStart = pBuffer;
		bool bFirstCode = true;
		while ( pBuffer<pBufferMax )
		{
			const BYTE * pTmp = pBuffer;
			int iCode = sphUTF8Decode ( pBuffer );
			bool bIsChineseCode = sphIsChineseCode(iCode);
			if ( !bFirstCode && bWasChineseCode!=bIsChineseCode )
			{
				AddTextChunk ( pChunkStart, pTmp-pChunkStart, bWasChineseCode );
				pChunkStart = pTmp;
			}
			bWasChineseCode = bIsChineseCode;
			bFirstCode = false;
		}

		AddTextChunk ( pChunkStart, pBuffer-pChunkStart, bWasChineseCode );

		ProcessBufferRLP ( m_dCJKBuffer.Begin(), m_dCJKBuffer.GetLength() );

		dOut.Reserve ( m_dCJKBuffer.GetLength() + m_dNonCJKBuffer.GetLength() );

		// reconstruct the buffer
		BYTE * pToken;
		int iCurNonCJKToken = 0;
		while ( (pToken = GetNextTokenRLP())!=NULL )
		{
			int iResLen = dOut.GetLength();
			int iTokenLen = strlen ( (const char *)pToken );
			bool bAddSpace = iResLen && !sphIsSpace ( dOut[iResLen-1] );
			if ( iTokenLen==PROXY_MARKER_LEN && CMP_MARKER ( pToken, m_pMarkerChunkSeparator ) )
			{
				const TextChunk_t & tChunk = m_dNonCJKChunks[iCurNonCJKToken++];
				BYTE * pChunkStart = m_dNonCJKBuffer.Begin()+tChunk.m_iStart;
				bAddSpace &= !sphIsSpace( *pChunkStart ) && ( !bQuery || !IsSpecialQueryCode ( *pChunkStart ) ) && !IsBlendChar( *pChunkStart );
				dOut.Resize ( iResLen + tChunk.m_iLength + ( bAddSpace ? 1 : 0 ) );
				BYTE * pOut = dOut.Begin()+iResLen;
				if ( bAddSpace )
					*pOut++ = ' ';
				memcpy ( pOut, pChunkStart, tChunk.m_iLength );
			} else
			{
				dOut.Resize ( iResLen + iTokenLen + ( bAddSpace ? 1 : 0 ) );
				BYTE * pOut = dOut.Begin()+iResLen;
				if ( iResLen )
					bAddSpace &= ( !bQuery || !IsSpecialQueryCode ( pOut[-1] ) ) && !IsBlendChar( pOut[-1] );

				if ( bAddSpace )
					*pOut++ = ' ';

				memcpy ( pOut, pToken, iTokenLen );
			}
		}
	}

	void AddTextChunk ( const BYTE * pStart, int iLen, bool bChinese )
	{
		if ( bChinese )
		{
			// fixme! maybe surround these chinese text chunks by spaces?
			int iOldBufferLen = m_dCJKBuffer.GetLength();
			m_dCJKBuffer.Resize ( iOldBufferLen+iLen );
			BYTE * pCurDocPtr = &(m_dCJKBuffer[iOldBufferLen]);
			memcpy ( pCurDocPtr, pStart, iLen );
		} else
		{
			// store non-chinese content
			int iOldBufferLen = m_dNonCJKBuffer.GetLength();
			TextChunk_t & tChunk = m_dNonCJKChunks.Add();
			tChunk.m_iStart = iOldBufferLen;
			tChunk.m_iLength = iLen;
			m_dNonCJKBuffer.Resize ( iOldBufferLen+iLen );
			BYTE * pNonChinesePtr = &(m_dNonCJKBuffer[iOldBufferLen]);
			memcpy ( pNonChinesePtr, pStart, iLen );

			// copy marker to chinese buffer
			iOldBufferLen = m_dCJKBuffer.GetLength();
			m_dCJKBuffer.Resize ( iOldBufferLen+PROXY_MARKER_LEN+2 ); // marker+2 spaces around it
			BYTE * pCurDocPtr = &(m_dCJKBuffer[iOldBufferLen]);
			COPY_MARKER ( pCurDocPtr, m_pMarkerChunkSeparator );
		}
	}

	int GetNextLengthToSegment ( const BYTE * pBuffer, int iLength ) const
	{
		if ( iLength <= MAX_CHUNK_SIZE )
			return iLength;

		const BYTE * pCurBuf = pBuffer;
		const BYTE * pLastSeparator = NULL;
		int iLengthLeft = Min ( iLength, MAX_CHUNK_SIZE );
		while ( pCurBuf<pBuffer+iLengthLeft )
		{
			int iCode = sphUTF8Decode ( pCurBuf );
			if ( IsChineseSeparator ( iCode ) )
				pLastSeparator = pCurBuf;
		}

		return pLastSeparator ? pLastSeparator-pBuffer : iLengthLeft;
	}

	void ProcessBufferRLP ( const BYTE * pBuffer, int iLength )
	{
		assert ( !m_pTokenIterator );

		BT_Result iRes = BT_RLP_Context_ProcessBuffer ( m_pContext, pBuffer, iLength, BT_LANGUAGE_SIMPLIFIED_CHINESE, "UTF-8", NULL );
		// iteration should still work ok in this case
		if ( iRes!=BT_OK )
			sphWarning ( "BT_RLP_Context_ProcessBuffer error" );

		m_pTokenIterator = BT_RLP_TokenIteratorFactory_CreateIterator ( m_pFactory, m_pContext );
		if ( !m_pTokenIterator )
			sphWarning ( "BT_RLP_TokenIteratorFactory_CreateIterator error" );
	}


	BYTE * GetNextTokenRLP()
	{
		static const char * RPL_SPECIAL_STOPWORD = "rlpspecialstopword";

		if ( !m_pTokenIterator )
			return NULL;

		if ( m_iNextCompoundComponent!=-1 )
		{
			if ( m_iNextCompoundComponent>=(int)BT_RLP_TokenIterator_GetNumberOfCompoundComponents ( m_pTokenIterator ) )
				m_iNextCompoundComponent = -1;
			else
			{
				const BT_Char16 * pToken = BT_RLP_TokenIterator_GetCompoundComponent ( m_pTokenIterator, m_iNextCompoundComponent++ );
				if ( BT_RLP_TokenIterator_IsStopword ( m_pTokenIterator ) )
					strncpy ( (char*)m_dUTF8Buffer, RPL_SPECIAL_STOPWORD, MAX_TOKEN_LEN );
				else
				{
					assert ( pToken );
					bt_xutf16toutf8 ( (char*)m_dUTF8Buffer, pToken, sizeof(m_dUTF8Buffer) );
				}

				return &(m_dUTF8Buffer[0]);
			}
		}

		while ( BT_RLP_TokenIterator_Next ( m_pTokenIterator ) )
		{
			const char * szPartOfSpeech = BT_RLP_TokenIterator_GetPartOfSpeech ( m_pTokenIterator );

			if ( IsJunkPOS ( szPartOfSpeech ) )
				continue;

			const BT_Char16 * pToken;
			int nCC = BT_RLP_TokenIterator_GetNumberOfCompoundComponents ( m_pTokenIterator );
			if ( nCC>0 )
			{
				m_iNextCompoundComponent = 0;
				pToken = BT_RLP_TokenIterator_GetCompoundComponent ( m_pTokenIterator, m_iNextCompoundComponent++ );
			} else
				pToken = BT_RLP_TokenIterator_GetToken ( m_pTokenIterator );

			if ( BT_RLP_TokenIterator_IsStopword ( m_pTokenIterator ) )
				strncpy ( (char*)m_dUTF8Buffer, RPL_SPECIAL_STOPWORD, MAX_TOKEN_LEN );
			else
			{
				assert ( pToken );
				bt_xutf16toutf8 ( (char*)m_dUTF8Buffer, pToken, sizeof(m_dUTF8Buffer) );
			}

			return &(m_dUTF8Buffer[0]);
		}

		DestroyIteratorRLP();

		return NULL;
	}


	void DestroyIteratorRLP()
	{
		if ( m_pTokenIterator )
		{
			BT_RLP_TokenIterator_Destroy ( m_pTokenIterator );
			m_pTokenIterator = NULL;
		}

		if ( m_pContext )
			BT_RLP_Context_DestroyResultStorage ( m_pContext );
	}

	bool IsJunkPOS ( const char * szPOS )
	{
		// drop EOS and PUNCT
		return !szPOS || !*szPOS || ( *szPOS=='E' && *(szPOS+1)=='O' ) || ( *szPOS=='P' && *(szPOS+1)=='U' );
	}

	bool IsSpecialCode ( int iCode ) const
	{
		return iCode==PROXY_DOCUMENT_START || iCode==PROXY_FIELD_START;
	}

	bool IsChineseSeparator ( int iCode ) const
	{
		return ( iCode>=0x3000 && iCode<=0x303F ) || sphIsSpace(iCode) || IsSpecialCode(iCode);
	}

	bool IsSpecialQueryCode ( int iCode ) const
	{
		return iCode=='!' || iCode=='^' || iCode=='$' || iCode=='*' || iCode=='=';
	}

	bool IsBlendChar ( int iCode ) const
	{
		ARRAY_FOREACH ( i, m_dBlendChars )
			if ( iCode>=m_dBlendChars[i].m_iStart && iCode<=m_dBlendChars[i].m_iEnd )
				return true;

		return false;
	}
};


class CSphFieldFilterRLP : public ISphFieldFilter, public CSphRLPPreprocessor
{
public:
	CSphFieldFilterRLP ( const char * szRLPRoot, const char * szRLPEnv, const char * szRLPCtx );


	virtual	int				Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery );
	virtual	void			GetSettings ( CSphFieldFilterSettings & tSettings ) const;
	virtual ISphFieldFilter * Clone();
};


CSphFieldFilterRLP::CSphFieldFilterRLP ( const char * szRLPRoot, const char * szRLPEnv, const char * szRLPCtx )
	: CSphRLPPreprocessor ( szRLPRoot, szRLPEnv, szRLPCtx )
{
}


int CSphFieldFilterRLP::Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery )
{
	if ( m_pParent )
	{
		int iResultLength = m_pParent->Apply ( sField, iLength, dStorage, bQuery );
		if ( iResultLength ) // can't use dStorage.GetLength() because of the safety gap
		{
			CSphFixedVector<BYTE> dTmp ( iResultLength );
			memcpy ( dTmp.Begin(), dStorage.Begin(), dStorage.GetLength() );
			if ( !Process ( dTmp.Begin(), iLength, dStorage, bQuery ) )
				return iResultLength;

			// add safety gap
			int iStorageLength = dStorage.GetLength();
			if ( iStorageLength )
			{
				dStorage.Resize ( iStorageLength+4 );
				dStorage[iStorageLength]='\0';
			}

			return iStorageLength;
		}
	}

	if ( !Process ( sField, iLength, dStorage, bQuery ) )
		return 0;

	int iStorageLength = dStorage.GetLength();
	dStorage.Resize ( iStorageLength+4 );
	dStorage[iStorageLength]='\0';

	return iStorageLength;
}


void CSphFieldFilterRLP::GetSettings ( CSphFieldFilterSettings & tSettings ) const
{
	if ( m_pParent )
		m_pParent->GetSettings ( tSettings );
}


ISphFieldFilter * CSphFieldFilterRLP::Clone()
{
	ISphFieldFilter * pClonedParent = NULL;
	if ( m_pParent )
		pClonedParent = m_pParent->Clone();

	CSphString sError;
	return sphCreateRLPFilter ( pClonedParent, m_sRootPath.cstr(), m_sEnvPath.cstr(), m_sCtxPath.cstr(), m_sBlendChars.cstr(), sError );
}

ISphFieldFilter * sphCreateRLPFilter ( ISphFieldFilter * pParent, const char * szRLPRoot, const char * szRLPEnv, const char * szRLPCtx, const char * szBlendChars, CSphString & sError )
{
	CSphFieldFilterRLP * pFilter = new CSphFieldFilterRLP ( szRLPRoot, szRLPEnv, szRLPCtx );
	if ( !pFilter->Init ( sError ) )
	{
		SafeDelete ( pFilter );
		return pParent;
	}

	if ( szBlendChars && *szBlendChars && !pFilter->SetBlendChars ( szBlendChars, sError ) )
	{
		SafeDelete ( pFilter );
		return pParent;
	}

	pFilter->SetParent ( pParent );

	return pFilter;
}


bool sphRLPCheckConfig ( CSphIndexSettings &, CSphString & )
{
	return true;
}

bool sphSpawnRLPFilter ( ISphFieldFilter * & pFieldFilter, const CSphIndexSettings & m_tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError )
{
	if ( m_tSettings.m_eChineseRLP==SPH_RLP_NONE )
		return true;

	ISphFieldFilter * pRLPFilter = sphCreateRLPFilter ( pFieldFilter, g_sRLPRoot.cstr(), g_sRLPEnv.cstr(), m_tSettings.m_sRLPContext.cstr(), tTokSettings.m_sBlendChars.cstr(), sError );
	if ( pRLPFilter==pFieldFilter && m_tSettings.m_eChineseRLP==SPH_RLP_BATCHED )
	{
		sError.SetSprintf ( "index '%s': Error initializing RLP: %s", szIndex, sError.cstr() );
		return false;
	}

	pFieldFilter = pRLPFilter;
	return true;
}

void sphConfigureRLP ( CSphConfigSection & hCommon )
{
	g_sRLPRoot = hCommon.GetStr ( "rlp_root" );
	g_sRLPEnv = hCommon.GetStr ( "rlp_environment" );
	g_iRLPMaxBatchSize = hCommon.GetSize ( "rlp_max_batch_size", 51200 );
	g_iRLPMaxBatchDocs = hCommon.GetInt ( "rlp_max_batch_docs", 50 );
}

#else


ISphFieldFilter * sphCreateRLPFilter ( ISphFieldFilter * pParent, CSphString & )
{
	return pParent;
}


bool sphRLPCheckConfig ( CSphIndexSettings & tSettings, CSphString & sError )
{
	if ( tSettings.m_eChineseRLP!=SPH_RLP_NONE || !tSettings.m_sRLPContext.IsEmpty() )
	{
		tSettings.m_eChineseRLP = SPH_RLP_NONE;
		sError.SetSprintf ( "RLP options specified, but no RLP support compiled; ignoring\n" );
		return false;
	}

	return true;
}


bool sphSpawnRLPFilter ( ISphFieldFilter * &, const CSphIndexSettings &, const CSphTokenizerSettings &, const char *, CSphString & )
{
	return true;
}

void sphConfigureRLP ( CSphConfigSection & )
{
}

#endif


//
// $Id$
//
