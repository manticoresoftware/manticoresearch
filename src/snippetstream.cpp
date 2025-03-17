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

#include "snippetstream.h"

#include "sphinxint.h"
#include "stripper/html_stripper.h"
#include "tokenizer/tokenizer.h"

#define UINT32_MASK 0xffffffffUL
#define UINT16_MASK 0xffff


static void CopyString ( BYTE * sDst, const BYTE * sSrc, int iLen )
{
	const int MAX_WORD_BYTES = 3*SPH_MAX_WORD_LEN;
	int iBackup = ( iLen > MAX_WORD_BYTES ) ? MAX_WORD_BYTES : iLen;
	int iBackup2 = ( iBackup+3 )>>2;
	DWORD * d = (DWORD*)sDst;
	auto * s = (const DWORD*)sSrc;
	while ( iBackup2-->0 )
		*d++ = *s++;
	sDst[iBackup] = '\0';
}

// make zone name lowercase
static void CopyZoneName ( CSphVector<char> & dName, const char * sZone, int iLen )
{
	dName.Resize ( iLen+1 );
	char * pDst = dName.Begin();
	const char * pEnd = sZone + iLen;
	while ( sZone<pEnd )
		*pDst++ = (char)tolower ( *sZone++ );

	dName[iLen] = '\0';
}


static uint64_t PackZone ( DWORD uPosition, int iSiblingIndex, int iZoneType )
{
	assert ( iSiblingIndex>=0 && iSiblingIndex<UINT16_MASK );
	assert ( iZoneType>=0 && iZoneType<UINT16_MASK );

	return ( ( (uint64_t)uPosition<<32 )
		| ( ( iSiblingIndex & UINT16_MASK )<<16 )
		| ( iZoneType & UINT16_MASK ) );
}


static int FindAddZone ( const char * sZoneName, int iZoneNameLen, SmallStringHash_T<int> & hZones )
{
	CSphString sZone;
	sZone.SetBinary ( sZoneName, iZoneNameLen );

	int * pZoneIndex = hZones ( sZone );
	if ( pZoneIndex )
		return *pZoneIndex;

	int iZone = hZones.GetLength();
	hZones.Add ( iZone, sZone );
	return iZone;
}


static int AddZone ( const char * pStart, const char * pEnd, int uPosition, HitCollector_i & tFunctor, CSphVector<int> & dZoneStack, CSphVector<char> & dZoneName, const char * pBuf )
{
	CSphVector<ZonePacked_t> & dZones = tFunctor.GetZones();
	SmallStringHash_T<int> & hZones = tFunctor.GetZoneInfo().m_hZones;
	CSphVector<int>	& dZonePos = tFunctor.GetZoneInfo().m_dZonePos;
	CSphVector<int>	& dZoneParent = tFunctor.GetZoneInfo().m_dZoneParent;
	bool bNeedExtraZoneInfo = tFunctor.NeedExtraZoneInfo();

	int iZone;

	// span's management
	if ( *pStart!='/' )	// open zone
	{
		// zone stack management
		int iSelf = dZones.GetLength();
		dZoneStack.Add ( iSelf );

		// add zone itself
		int iZoneNameLen = int ( pEnd-pStart ) - 1;
		CopyZoneName ( dZoneName, pStart, iZoneNameLen );

		iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		dZones.Add ( PackZone ( uPosition, iSelf, iZone ) );

		if ( bNeedExtraZoneInfo )
		{
			// the parent for the open zone is the zone itself
			dZoneParent.Add ( iZone );

			// zone position in characters
			dZonePos.Add ( int ( pStart-pBuf ) );
		}

#ifndef NDEBUG
		if ( !bNeedExtraZoneInfo )
			dZonePos.Add ( int ( pStart-pBuf ) );
#endif

	} else					// close zone
	{
#ifndef NDEBUG
		// lets check open - close tags match
		assert ( dZoneStack.GetLength() && dZoneStack.Last()<dZones.GetLength() );
		int iOpening = dZonePos [ dZoneStack.Last() ];
		assert ( iOpening<pEnd-pBuf && strncmp ( pBuf+iOpening, pStart+1, pEnd-pStart-2 )==0 );
#endif

		int iZoneNameLen = int ( pEnd-pStart ) - 2;
		CopyZoneName ( dZoneName, pStart+1, iZoneNameLen );

		iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		int iOpen = dZoneStack.Last();
		int iClose = dZones.GetLength();
		uint64_t uOpenPacked = dZones[ iOpen ];
		DWORD uOpenPos = (DWORD)( ( uOpenPacked>>32 ) & UINT32_MASK );
		assert ( iZone==(int)( uOpenPacked & UINT16_MASK ) ); // check for zone's types match;

		dZones[iOpen] = PackZone ( uOpenPos, iClose, iZone );
		dZones.Add ( PackZone ( uPosition, iOpen, iZone ) );

		if ( bNeedExtraZoneInfo )
		{
			// zone position in characters
			dZonePos.Add ( int ( pStart-pBuf ) );

			// the parent for the closing zone is the previous zone on stack
			int iParentZone = dZoneStack.GetLength()>2 ? dZoneStack[dZoneStack.GetLength()-2] : 0;
			uint64_t uParentPacked = dZones.GetLength() && iParentZone<dZones.GetLength() ? dZones[iParentZone] : 0;
			dZoneParent.Add ( (int)( uParentPacked & UINT16_MASK ) );
		}

#ifndef NDEBUG
		if ( !bNeedExtraZoneInfo )
			dZonePos.Add ( int ( pStart-pBuf ) );
#endif


		// pop up current zone from zone's stack
		dZoneStack.Resize ( dZoneStack.GetLength()-1 );
	}

	return iZone;
}


static int FindTagEnd ( const char * sData )
{
	assert ( *sData=='<' );
	const char * s = sData+1;

	// we just scan until EOLN or tag end
	while ( *s && *s!='>' )
	{
		// exit on duplicate
		if ( *s=='<' )
			return -1;

		if ( *s=='\'' || *s=='"' )
			s = (const char *)SkipQuoted ( (const BYTE *)s );
		else
			s++;
	}

	if ( !*s )
		return -1;

	return int ( s-sData );
}

//////////////////////////////////////////////////////////////////////////
/// functor that maps collected tokens into a stream
class CacheStreamer_c : public CacheStreamer_i
{
public:
	explicit	CacheStreamer_c ( int iDocLen );

	void		StoreToken ( const TokenInfo_t & tTok, int iTermIndex ) final;
	void		StoreOverlap ( int iStart, int iLen, int iBoundary ) final;
	void		StoreSkipHtml ( int iStart, int iLen ) final;
	void		StoreSPZ ( BYTE iSPZ, DWORD uPosition, const char *, int iZone ) final;
	void		StoreTail ( int iStart, int iLen, int iBoundary ) final;

	void		SetZoneInfo ( const FunctorZoneInfo_t & tZoneInfo ) final { m_pZoneInfo = &tZoneInfo; }
	void		Tokenize ( TokenFunctor_i & tFunctor ) final;
	bool		IsEmpty() const final;

private:
	enum
	{
		TYPE_TOKEN1 = 0,	///< 1-byte token (4-bit code, and 4-bit len payload)
		TYPE_TOKEN2,		///< 2-byte token (4-bit code, and 8-bit len payload)
		TYPE_OVERLAP1,		///< 1-byte overlap (4-bit code, and 4-bit len payload)
		TYPE_TOKOVER1,		///< 1-byte token/overlap combo (4-bit code, 4-bit token len (overlap len is always 1))
		TYPE_TOKOVER2,		///< 1-byte token/overlap combo (4-bit code, 4-bit token len (overlap len is always 2))
		TYPE_TOKOVER3,		///< 1-byte token/overlap combo (4-bit code, 4-bit token len (overlap len is always 3))
		TYPE_TOKOVER4,		///< 1-byte token/overlap combo (4-bit code, 4-bit token len (overlap len is always 4))
		TYPE_TOKOVER5,		///< 1-byte token/overlap combo (4-bit code, 3-bit token len, 1-bit overlap len)
		TYPE_TOKEN,			///< generic fat token
		TYPE_OVERLAP,		///< generic fat overlap
		TYPE_SKIPHTML,
		TYPE_SPZ,
		TYPE_TAIL,
		TYPE_MULTIFORM,

		TYPE_TOTAL
	};
	STATIC_ASSERT ( TYPE_TOTAL<=15, OUT_OF_TYPECODES );

	CSphTightVector<BYTE>		m_dTokenStream;
	int							m_iReadPtr = 0;
	int							m_iLastStart = 0;	///< last delta coded token offset, in bytes
	int							m_iLastPos = 0;		///< last delta coded token number, in tokens
	int							m_eLastStored = TYPE_TOTAL;
	const FunctorZoneInfo_t *	m_pZoneInfo = nullptr;

	inline BYTE *	StoreEntry ( int iBytes );
	inline void		ZipInt ( DWORD uValue );
	inline DWORD	UnzipInt();
};


CacheStreamer_c::CacheStreamer_c ( int iDocLen )
{
	m_dTokenStream.Reserve ( (iDocLen*2)/5 );
	m_dTokenStream.Add ( 0 );
}


inline BYTE * CacheStreamer_c::StoreEntry ( int iBytes )
{
	return m_dTokenStream.AddN ( iBytes );
}


void CacheStreamer_c::StoreOverlap ( int iStart, int iLen, int iBoundary )
{
	assert ( iLen>0 && iLen<=USHRT_MAX );

	int iDstart = iStart - m_iLastStart;
	m_iLastStart = iStart + iLen;

	if ( iDstart==0 && iLen<16 && iBoundary<0 )
	{
		// try to store a token+overlap combo
		if ( m_eLastStored==TYPE_TOKEN1 )
		{
			int iTokLen = m_dTokenStream.Last() & 15;
			assert ( iTokLen > 0 );

			if ( iLen<=4 && iTokLen<=16 )
			{
				BYTE uType = (BYTE)(TYPE_TOKOVER1+iLen-1);
				m_dTokenStream.Last() = BYTE ( ( uType<<4 )+ (BYTE)iTokLen-1 );
				m_eLastStored = uType;
				return;
			} else if ( iLen>=5 && iLen<=6 && iTokLen<=8 )
			{
				m_dTokenStream.Last() = (BYTE)( ( TYPE_TOKOVER5<<4 ) + ( ( iTokLen-1 ) << 1 ) + iLen-5 );
				m_eLastStored = TYPE_TOKOVER5;
				return;
			}
		}

		// OVERLAP1, most frequent path
		// delta_start is 0, boundary is -1, length fits in 4 bits, so just 1 byte
		m_dTokenStream.Add ( (BYTE)( ( TYPE_OVERLAP1<<4 ) + iLen ) );
		m_eLastStored = TYPE_OVERLAP1;
		return;
	}

	// OVERLAP, stupid generic uncompressed path (can optimize with deltas, if needed)
	BYTE * p = StoreEntry ( 11 );
	p[0] = ( TYPE_OVERLAP<<4 );
	sphUnalignedWrite ( p+1, iStart );
	sphUnalignedWrite ( p+5, WORD(iLen) );
	sphUnalignedWrite ( p+7, iBoundary );
	m_eLastStored = TYPE_OVERLAP;
}


void CacheStreamer_c::StoreSkipHtml ( int iStart, int iLen )
{
	m_dTokenStream.Add ( TYPE_SKIPHTML<<4 );
	ZipInt ( iStart );
	ZipInt ( iLen );
	m_eLastStored = TYPE_SKIPHTML;
}


void CacheStreamer_c::StoreToken ( const TokenInfo_t & tTok, int iTermIndex )
{
	assert ( iTermIndex<USHRT_MAX );

	int iDstart = tTok.m_iStart - m_iLastStart;
	int iDpos = tTok.m_uPosition - m_iLastPos;

	m_iLastStart = tTok.m_iStart + tTok.m_iLen;
	m_iLastPos = tTok.m_uPosition;

	if ( iDstart==0 && iDpos==1 && tTok.m_bWord && !tTok.m_bStopWord && iTermIndex==-1 && !tTok.m_iMultiPosLen && tTok.m_iLen<=4095 )
	{
		if ( tTok.m_iLen<16 )
		{
			// TOKEN1, most frequent path
			m_dTokenStream.Add ( (BYTE)( ( TYPE_TOKEN1<<4 ) + tTok.m_iLen ) );
			m_eLastStored = TYPE_TOKEN1;
			return;
		} else
		{
			// TOKEN2, 2nd most frequent path
			m_dTokenStream.Add ( (BYTE)( TYPE_TOKEN2<<4 ) + ( tTok.m_iLen >> 8 ));
			m_dTokenStream.Add ( (BYTE)( 0xFF & tTok.m_iLen ) );
			m_eLastStored = TYPE_TOKEN2;
			return;
		}
	}

	// TOKEN, stupid generic uncompressed path (can optimize with deltas, if needed)
	bool bMultiform = ( tTok.m_iMultiPosLen>0 );

	BYTE* p = StoreEntry ( 5 );
	BYTE eTok = (BYTE)( bMultiform ? TYPE_MULTIFORM : TYPE_TOKEN );
	p[0] = BYTE ( eTok<<4 );
	sphUnalignedWrite ( p+1, tTok.m_iStart );
	ZipInt ( tTok.m_iLen );
	p = StoreEntry ( bMultiform ? 8 : 7 );
	sphUnalignedWrite ( p, tTok.m_uPosition );
	p[4] = BYTE ( ( tTok.m_bWord<<1 ) + tTok.m_bStopWord );
	sphUnalignedWrite ( p+5, (WORD)(iTermIndex+1) );
	if ( bMultiform )
		p[7] = (BYTE)tTok.m_iMultiPosLen;
	m_eLastStored = eTok;
}


void CacheStreamer_c::StoreSPZ ( BYTE iSPZ, DWORD uPosition, const char *, int iZone )
{
	m_dTokenStream.Add ( TYPE_SPZ<<4 );
	ZipInt ( iSPZ );
	ZipInt ( uPosition );
	ZipInt ( iZone==-1 ? 0 : 1 );
	if ( iZone!=-1 )
		ZipInt ( iZone );

	m_eLastStored = TYPE_SPZ;
}


void CacheStreamer_c::StoreTail ( int iStart, int iLen, int iBoundary )
{
	m_dTokenStream.Add ( TYPE_TAIL<<4 );
	ZipInt ( iStart );
	ZipInt ( iLen );
	ZipInt ( iBoundary==-1 ? 0 : 1 );
	if ( iBoundary!=-1 )
		ZipInt ( iBoundary );

	m_eLastStored = TYPE_TAIL;
}


void CacheStreamer_c::Tokenize ( TokenFunctor_i & tFunctor )
{
	m_iLastStart = 0;
	m_iLastPos = 0;

	m_iReadPtr = 1;
	TokenInfo_t tTok;
	bool bStop = false;
	CSphVector<SphWordID_t> dTmp;

	while ( m_iReadPtr < m_dTokenStream.GetLength() )
	{
		BYTE eTok = m_dTokenStream [ m_iReadPtr ]>>4;
		switch ( eTok )
		{
		case TYPE_OVERLAP1:
		{
			int iLen = m_dTokenStream [ m_iReadPtr++ ] & 15;
			bStop = !tFunctor.OnOverlap ( m_iLastStart, iLen, -1 );
			m_iLastStart += iLen;
		}
		break;

		case TYPE_OVERLAP:
		{
			BYTE * p = &m_dTokenStream [ m_iReadPtr ];
			int iStart = sphUnalignedRead ( *(DWORD*)(p+1) );
			int iLen = sphUnalignedRead ( *(WORD*)(p+5) );
			int iBoundary = sphUnalignedRead ( *(int*)(p+7) );
			m_iReadPtr += 11;
			m_iLastStart = iStart + iLen;
			bStop = !tFunctor.OnOverlap ( iStart, iLen, iBoundary );
		}
		break;

		case TYPE_SKIPHTML:
		{
			m_iReadPtr++;
			DWORD uStart = UnzipInt ();
			DWORD uLen = UnzipInt ();
			tFunctor.OnSkipHtml ( uStart, uLen );
		}
		break;

		case TYPE_TOKEN:
		case TYPE_MULTIFORM:
		{
			BYTE * p = &m_dTokenStream [ m_iReadPtr ];
			tTok.m_iStart = sphUnalignedRead ( *(DWORD*)(p+1) );
			m_iReadPtr += 5;
			tTok.m_iLen = UnzipInt(); // p[5];
			p = &m_dTokenStream[m_iReadPtr];
			tTok.m_uPosition = sphUnalignedRead ( *(DWORD*)(p) );
			tTok.m_bWord = ( p[4] & 2 )!=0;
			tTok.m_bStopWord = ( p[4] & 1 )!=0;
			tTok.m_iTermIndex = (int)sphUnalignedRead ( *(WORD*)(p+5) ) - 1;
			if ( eTok==TYPE_TOKEN )
			{
				tTok.m_iMultiPosLen = 0;
				m_iReadPtr += 7;
			} else
			{
				tTok.m_iMultiPosLen = (int)( p[7] );
				m_iReadPtr += 8;
			}

			m_iLastStart = tTok.m_iStart + tTok.m_iLen;
			m_iLastPos = tTok.m_uPosition;

			tTok.m_sWord = NULL;

			bStop = !tFunctor.OnToken ( tTok, dTmp, NULL );
		}
		break;

		case TYPE_TOKEN1:
		{
			tTok.m_iStart = m_iLastStart;
			tTok.m_iLen = m_dTokenStream [ m_iReadPtr++ ] & 15;
			m_iLastStart += tTok.m_iLen;
			tTok.m_uPosition = ++m_iLastPos;
			tTok.m_bWord = true;
			tTok.m_bStopWord = false;
			tTok.m_iTermIndex = -1;
			tTok.m_iMultiPosLen = 0;

			tTok.m_sWord = NULL;

			bStop = !tFunctor.OnToken ( tTok, dTmp, NULL );
		}
		break;

		case TYPE_TOKEN2:
		{
			tTok.m_iStart = m_iLastStart;
			tTok.m_iLen = ( ( m_dTokenStream[m_iReadPtr] & 15 ) << 8 ) + m_dTokenStream[m_iReadPtr + 1];
			m_iReadPtr += 2;
			m_iLastStart += tTok.m_iLen;
			tTok.m_uPosition = ++m_iLastPos;
			tTok.m_bWord = true;
			tTok.m_bStopWord = false;
			tTok.m_iTermIndex = -1;
			tTok.m_iMultiPosLen = 0;

			tTok.m_sWord = NULL;

			bStop = !tFunctor.OnToken ( tTok, dTmp, NULL );
		}
		break;

		case TYPE_TOKOVER1:
		case TYPE_TOKOVER2:
		case TYPE_TOKOVER3:
		case TYPE_TOKOVER4:
		{
			BYTE iStored = m_dTokenStream [ m_iReadPtr++ ];
			int iLen = ( iStored>>4 ) - TYPE_TOKOVER1 + 1;

			tTok.m_iStart = m_iLastStart;
			tTok.m_iLen = ( iStored & 15 ) + 1;
			m_iLastStart += tTok.m_iLen;
			tTok.m_uPosition = ++m_iLastPos;
			tTok.m_bWord = true;
			tTok.m_bStopWord = false;
			tTok.m_iTermIndex = -1;
			tTok.m_iMultiPosLen = 0;

			tTok.m_sWord = NULL;

			bStop = !tFunctor.OnToken ( tTok, dTmp, NULL );

			if ( bStop )
				break;

			bStop = !tFunctor.OnOverlap ( m_iLastStart, iLen, -1 );
			m_iLastStart += iLen;
		}
		break;

		case TYPE_TOKOVER5:
		{
			BYTE iStored = m_dTokenStream [ m_iReadPtr++ ];

			tTok.m_iStart = m_iLastStart;
			tTok.m_iLen = ( ( iStored >> 1 ) & 7 ) + 1;
			m_iLastStart += tTok.m_iLen;
			tTok.m_uPosition = ++m_iLastPos;
			tTok.m_bWord = true;
			tTok.m_bStopWord = false;
			tTok.m_iTermIndex = -1;
			tTok.m_iMultiPosLen = 0;

			tTok.m_sWord = NULL;

			bStop = !tFunctor.OnToken ( tTok, dTmp, NULL );

			if ( bStop )
				break;

			int iLen = ( iStored & 1 ) + 5;
			bStop = !tFunctor.OnOverlap ( m_iLastStart, iLen, -1 );
			m_iLastStart += iLen;
		}
		break;

		case TYPE_SPZ:
		{
			m_iReadPtr++;
			assert ( m_pZoneInfo );

			BYTE uSPZ = (BYTE)UnzipInt ();
			DWORD uPosition = UnzipInt ();
			DWORD uFlag = UnzipInt ();
			int iZone = -1;
			if ( uFlag==1 )
				iZone = UnzipInt ();

			const char * szZoneName = NULL;
			if ( iZone!=-1 )
			{
				// fixme: it can be a lot faster
				for ( const auto& tZone : m_pZoneInfo->m_hZones )
					if ( tZone.second==iZone )
					{
						szZoneName = tZone.first.cstr();
						break;
					}
			}

			tFunctor.OnSPZ ( uSPZ, uPosition, szZoneName, iZone );
		}
		break;

		case TYPE_TAIL:
		{
			m_iReadPtr++;
			DWORD uStart = UnzipInt ();
			DWORD uLen = UnzipInt ();
			DWORD uFlag = UnzipInt ();
			int iBoundary = -1;
			if ( uFlag==1 )
				iBoundary = UnzipInt ();

			tFunctor.OnTail ( uStart, uLen, iBoundary );
		}
		break;

		default:
			assert ( 0 && "INTERNAL ERROR: unhandled type in token cache" );
			bStop = true;
			break;
		}

		if ( bStop )
			break;
	}

	tFunctor.OnFinish();
}


bool CacheStreamer_c::IsEmpty() const
{
	return m_dTokenStream.IsEmpty() || ( m_dTokenStream.GetLength()==1 && !m_dTokenStream[0] );
}


inline void CacheStreamer_c::ZipInt ( DWORD uValue )
{
	ZipValueBE ( [this] ( BYTE b ) { m_dTokenStream.Add ( b ); }, uValue );
}


inline DWORD CacheStreamer_c::UnzipInt()
{
	return UnzipValueBE<DWORD> ( [this]() mutable { return m_dTokenStream[m_iReadPtr++]; } );
}


//////////////////////////////////////////////////////////////////////////

CacheStreamer_i * CreateCacheStreamer ( int iDocLen )
{
	return new CacheStreamer_c(iDocLen);
}


void TokenizeDocument ( HitCollector_i & tFunctor, const CSphHTMLStripper * pStripper, DWORD iSPZ )
{
	TokenizerRefPtr_c pTokenizer = tFunctor.GetTokenizer();
	DictRefPtr_c & pDict = tFunctor.GetDict();

	const char * pStartPtr = pTokenizer->GetBufferPtr ();
	const char * pLastTokenEnd = pStartPtr;
	const char * pBufferEnd = pTokenizer->GetBufferEnd();

	BYTE sNonStemmed [ 3*SPH_MAX_WORD_LEN+4];

	TokenInfo_t tTok;
	tTok.m_iStart = 0;
	tTok.m_uPosition = 0;
	tTok.m_sWord = sNonStemmed;
	tTok.m_bStopWord = false;
	tTok.m_iTermIndex = -1;
	tTok.m_iMultiPosLen = 0;

	const CSphIndexSettings & tIndexSettings = tFunctor.GetIndexSettings();
	const SnippetQuerySettings_t & tSnippetQuery = tFunctor.GetSnippetQuery();

	bool bRetainHtml = tSnippetQuery.m_sStripMode=="retain";
	int iBoundaryStep = tIndexSettings.m_iBoundaryStep;

	BYTE * sWord = NULL;
	DWORD uPosition = 0;
	DWORD uStep = 1;
	const char * pBlendedStart = NULL;
	const char * pBlendedEnd = NULL;
	bool bBlendedHead = false;
	bool bBlendedPart = false;
	CSphVector<SphWordID_t> dMultiToken;
	CSphVector<int>	dMultiPosDelta;

	CSphVector<int> dZoneStack;
	CSphVector<char> dZoneName ( 16+3*SPH_MAX_WORD_LEN );

	// FIXME!!! replace by query SPZ extraction pass
	if ( !iSPZ && bRetainHtml )
		iSPZ = MAGIC_CODE_ZONE;

	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		const char * pTokenStart = pTokenizer->GetTokenStart ();

		tTok.m_iMultiPosLen = 0;
		dMultiPosDelta.Resize ( 0 );

		if ( pBlendedEnd<pTokenStart )
		{
			// FIXME!!! implement proper handling of blend-chars
			if ( pLastTokenEnd<pBlendedEnd && bBlendedPart )
			{
				tTok.m_uWordId = 0;
				tTok.m_bStopWord = false;
				tTok.m_uPosition = uPosition; // let's stick to last blended part
				tTok.m_iStart = int ( pLastTokenEnd - pStartPtr );
				tTok.m_iLen = int ( pBlendedEnd - pLastTokenEnd );
				tTok.m_bWord = false;
				if ( !tFunctor.OnToken ( tTok, dMultiToken, NULL ) )
				{
					tFunctor.OnFinish();
					return;
				}
				pLastTokenEnd = pBlendedEnd;
			}

			dMultiToken.Resize ( 0 );
		}

		uPosition += uStep + pTokenizer->GetOvershortCount();
		if ( pTokenizer->GetBoundary() )
			uPosition += iBoundaryStep;
		if ( pTokenizer->TokenIsBlended() )
			uStep = 0;

		// collect all tokens from multi destination of multi word-form
		bool bMultiDestHead = false;
		int iDestCount = 0;
		pTokenizer->WasTokenMultiformDestination ( bMultiDestHead, iDestCount );
		if ( bMultiDestHead )
		{
			assert ( iDestCount>1 );
			tTok.m_iMultiPosLen = iDestCount;
			int iLastToken = iDestCount-1;

			// blended from destination wordform means multiple lemma from appropriate destination token
			bool bWasBlended = pTokenizer->TokenIsBlended ();
			dMultiToken.Add ( pDict->GetWordID ( sWord ) );
			dMultiPosDelta.Add ( 0 );

			int iToken = ( bWasBlended ? 0 : 1 );
			while (true)
			{
				sWord = pTokenizer->GetToken ();
				assert ( sWord );
				bool bBlended = pTokenizer->TokenIsBlended();
				if ( iToken==iLastToken && !bBlended )
					break;

				dMultiToken.Add ( pDict->GetWordID ( sWord ) );
				dMultiPosDelta.Add ( bWasBlended ? 0 : 1 );

				bWasBlended = bBlended;
				if ( !bBlended )
					iToken++;
			}
			dMultiPosDelta.Add ( bWasBlended ? 0 : 1 );
#ifndef NDEBUG
			int iDeltaPos = 0;
			ARRAY_FOREACH ( i, dMultiPosDelta )
				iDeltaPos += dMultiPosDelta[i];
			assert ( iDeltaPos==iDestCount-1 );
#endif
			uStep = iDestCount;
		}


		// handle only blended parts
		if ( pTokenizer->TokenIsBlended() && !bMultiDestHead )
		{
			if ( tIndexSettings.m_bIndexExactWords && pTokenizer->GetTokenMorph()!=SPH_TOKEN_MORPH_GUESS )
			{
				BYTE sTmpBuf [ 3*SPH_MAX_WORD_LEN+4];
				sTmpBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
				CopyString ( sTmpBuf+1, sWord, int ( pTokenizer->GetTokenEnd() - pTokenStart ) );
				dMultiToken.Add ( pDict->GetWordIDNonStemmed ( sTmpBuf ) );
			}

			// must be last because it can change (stem) sWord
			dMultiToken.Add ( pDict->GetWordID ( sWord ) );
			pBlendedStart = pTokenizer->GetTokenStart();
			pBlendedEnd = Max ( pBlendedEnd, pTokenizer->GetTokenEnd() );
			bBlendedHead = true;
			continue;
		}

		if ( pTokenStart>pLastTokenEnd )
		{
			bool bDone = false;
			if ( pBlendedStart<pTokenStart && bBlendedHead )
			{
				// FIXME!!! implement proper handling of blend-chars
				if ( ( pBlendedStart - pLastTokenEnd )>0 )
					bDone = !tFunctor.OnOverlap ( int ( pLastTokenEnd-pStartPtr ),  int ( pBlendedStart-pLastTokenEnd ), pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

				tTok.m_uWordId = 0;
				tTok.m_bStopWord = false;
				tTok.m_uPosition = uPosition; // let's stick to 1st blended part
				tTok.m_iStart = int ( pBlendedStart - pStartPtr );
				tTok.m_iLen = int ( pTokenStart - pBlendedStart );
				tTok.m_bWord = false;
				if ( !bDone )
					bDone = !tFunctor.OnToken ( tTok, dMultiToken, &dMultiPosDelta );
			} else
				bDone = !tFunctor.OnOverlap ( int ( pLastTokenEnd-pStartPtr ), int ( pTokenStart - pLastTokenEnd ), pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

			if ( bDone	)
			{
				tFunctor.OnFinish();
				return;
			}

			pLastTokenEnd = pTokenStart;
		}
		bBlendedHead = false;
		bBlendedPart = pTokenizer->TokenIsBlendedPart();

		if ( bRetainHtml && *pTokenStart=='<' )
		{
			const html_stripper::StripperTag_t * pTag = NULL;
			const BYTE * sZoneName = NULL;
			const char * pEndSPZ = NULL;
			int iZoneNameLen = 0;
			if ( iSPZ && pStripper && pTokenStart+2<pBufferEnd && ( pStripper->IsValidTagStart ( *(pTokenStart+1) ) || pTokenStart[1]=='/') )
			{
				pEndSPZ = (const char *)pStripper->FindTag ( (const BYTE *)pTokenStart+1, &pTag, &sZoneName, &iZoneNameLen );
			}

			// regular HTML markup - keep it
			int iTagEnd = FindTagEnd ( pTokenStart );
			if ( iTagEnd!=-1 )
			{
				assert ( pTokenStart+iTagEnd<pTokenizer->GetBufferEnd() );
				tFunctor.OnSkipHtml ( int ( pTokenStart-pStartPtr ), iTagEnd+1 );
				pTokenizer->SetBufferPtr ( pTokenStart+iTagEnd+1 );
				pLastTokenEnd = pTokenStart+iTagEnd+1; // fix it up to prevent adding last chunk on exit
			}

			if ( pTag ) // (!S)PZ fix-up
			{
				pEndSPZ += ( pEndSPZ+1<=pBufferEnd && ( *pEndSPZ )!='\0' ); // skip closing angle bracket, if any

				assert ( pTag->m_bPara || pTag->m_bZone );
				assert ( pTag->m_bPara || ( pEndSPZ && ( pEndSPZ[0]=='\0' || pEndSPZ[-1]=='>' ) ) ); // should be at tag's end
				assert ( pEndSPZ && pEndSPZ<=pBufferEnd );

				// handle paragraph boundaries
				if ( pTag->m_bPara )
				{
					tFunctor.OnSPZ ( MAGIC_CODE_PARAGRAPH, uPosition, NULL, -1 );
				} else if ( pTag->m_bZone ) // handle zones
				{
					int iZone = AddZone ( pTokenStart+1, pTokenStart+2+iZoneNameLen, uPosition, tFunctor, dZoneStack, dZoneName, pStartPtr );
					tFunctor.OnSPZ ( MAGIC_CODE_ZONE, uPosition, dZoneName.Begin(), iZone );
				}
			}

			if ( iTagEnd )
				continue;
		}

		// handle SPZ tokens GE then needed
		// add SENTENCE, PARAGRAPH, ZONE token, do junks and tokenizer and pLastTokenEnd fix up
		// FIXME!!! it heavily depends on such attitude MAGIC_CODE_SENTENCE < MAGIC_CODE_PARAGRAPH < MAGIC_CODE_ZONE
		if ( *sWord==MAGIC_CODE_SENTENCE || *sWord==MAGIC_CODE_PARAGRAPH || *sWord==MAGIC_CODE_ZONE )
		{
			int iZone = -1;

			if ( *sWord==MAGIC_CODE_ZONE )
			{
				const char * pZoneEnd = pTokenizer->GetBufferPtr();
				const char * pZoneStart = pZoneEnd;
				while ( *pZoneEnd && *pZoneEnd!=MAGIC_CODE_ZONE )
					pZoneEnd++;
				pZoneEnd++; // skip zone token too
				pTokenizer->SetBufferPtr ( pZoneEnd );
				pLastTokenEnd = pZoneEnd; // fix it up to prevent adding last chunk on exit

				iZone = AddZone ( pZoneStart, pZoneEnd, uPosition, tFunctor, dZoneStack, dZoneName, pStartPtr );
			}

			// SPZ token has position and could be last token too
			if ( iSPZ && *sWord>=iSPZ )
			{
				tFunctor.OnSPZ ( *sWord, uPosition, dZoneName.Begin(), iZone );
			} else
				uStep = 0;

			if ( *sWord==MAGIC_CODE_PARAGRAPH )
				pLastTokenEnd = pTokenStart+1;

			continue;
		}

		pLastTokenEnd = pTokenizer->GetTokenEnd ();

		// might differ when sbsc got replaced by utf codepoint
		int iTokenLen = int ( pLastTokenEnd - pTokenStart );
		auto iWordLen = (int) strlen ( ( const char *)sWord );

		bool bPopExactMulti = false;
		if ( tIndexSettings.m_bIndexExactWords )
		{
			BYTE sTmpBuf [ 3*SPH_MAX_WORD_LEN+4];
			sTmpBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			CopyString ( sTmpBuf+1, sWord, iWordLen );
			dMultiToken.Add ( pDict->GetWordIDNonStemmed ( sTmpBuf ) );
			bPopExactMulti = true;
		}

		// must be last because it can change (stem) sWord
		CopyString ( sNonStemmed, sWord, iWordLen );
		SphWordID_t iWord = pDict->GetWordID ( sWord );
		tTok.m_uWordId = iWord;

		tTok.m_bStopWord = false;
		if ( !iWord )
			tTok.m_bStopWord = pDict->IsStopWord ( sWord );

		// compute position
		if ( !iWord || tTok.m_bStopWord )
			uStep = tIndexSettings.m_iStopwordStep;
		else if ( !tTok.m_iMultiPosLen ) // keep position step from multi word-forms
			uStep = 1;

		tTok.m_uPosition = ( iWord || tTok.m_bStopWord ) ? uPosition : 0;
		tTok.m_iStart = int ( pTokenStart - pStartPtr );
		tTok.m_iLen = iTokenLen;
		tTok.m_bWord = !!iWord;

		// match & emit
		// star match needs non-stemmed word
		if ( !tFunctor.OnToken ( tTok, dMultiToken, &dMultiPosDelta ) )
		{
			tFunctor.OnFinish();
			return;
		}

		if ( bPopExactMulti )
			dMultiToken.Pop();
	}

	// last space if any
	if ( pLastTokenEnd<pBlendedEnd && bBlendedPart )
	{
		// FIXME!!! implement proper handling of blend-chars
		tTok.m_uWordId = 0;
		tTok.m_bStopWord = false;
		tTok.m_uPosition = uPosition; // let's stick to last blended part, uPosition and not uPosition-1 as no iteration happened at exit
		tTok.m_iStart = int ( pLastTokenEnd - pStartPtr );
		tTok.m_iLen = int ( pBlendedEnd - pLastTokenEnd );
		tTok.m_bWord = false;
		tTok.m_iMultiPosLen = 0;
		tFunctor.OnToken ( tTok, dMultiToken, &dMultiPosDelta );
		pLastTokenEnd = pBlendedEnd;
	}

	if ( pLastTokenEnd!=pTokenizer->GetBufferEnd() )
		tFunctor.OnTail ( int ( pLastTokenEnd-pStartPtr ), int ( pTokenizer->GetBufferEnd() - pLastTokenEnd ), pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

	tFunctor.OnFinish();
}
