//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "dict_proxy.h"
#include "sphinxint.h"

class DictStar_c: public DictProxy_c
{
	using DictProxy_c::GetWordID;

public:
	DictStar_c ( DictRefPtr_c pDict, bool bInfixes )
		: DictProxy_c ( std::move (pDict) )
		, m_bInfixes ( bInfixes )
		, m_eDictFormat ( m_pDict->GetSettings().GetDictFormat() )
		, m_dBuf ( m_eDictFormat==DictFormat_e::KEYWORDS_V2 ? GetKeywordBufSize ( m_eDictFormat ) : 0 )
	{}

	SphWordID_t GetWordID ( BYTE* pWord ) final;

private:
	bool m_bInfixes;
	DictFormat_e m_eDictFormat;
	CSphFixedVector<BYTE> m_dBuf { 0 };

	void ValidateClone() const final { assert ( false && "DictStar_c must be operation-local" ); }
};

SphWordID_t DictStar_c::GetWordID ( BYTE* pWord )
{
	BYTE dBuf[GetKeywordBufSize ( DictFormat_e::KEYWORDS )];
	const bool bKeywordsV2 = ( m_eDictFormat==DictFormat_e::KEYWORDS_V2 );
	const int iBufSize = GetKeywordBufSize ( m_eDictFormat );
	BYTE * sBuf = bKeywordsV2 ? m_dBuf.Begin() : dBuf;

	assert ( bKeywordsV2 || m_dBuf.GetLength()==0 );
	assert ( !bKeywordsV2 || m_dBuf.GetLength()==GetKeywordBufSize ( DictFormat_e::KEYWORDS_V2 ) );
	assert ( bKeywordsV2 || iBufSize==(int)sizeof ( dBuf ) );

	auto iRawLen = (int)strlen ( (const char*)pWord );
	int iLen = Min ( iRawLen, iBufSize - 1 );

	if ( !iLen )
		return 0;

	bool bHeadStar = sphIsWild ( pWord[0] );
	bool bTailStar = sphIsWild ( pWord[iLen - 1] ) && ( iLen > 1 );
	bool bMagic = ( pWord[0] < ' ' );

	if ( !bHeadStar && !bTailStar && !bMagic )
	{
		if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;

		if ( iRawLen < iBufSize )
			m_pDict->ApplyStemmers ( pWord );

		// stemmer might squeeze out the word
		if ( !pWord[0] )
			return 0;

		if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;
	}

	if ( iRawLen < iBufSize )
		iRawLen = (int) strlen ( (const char *) pWord );
	iLen = Min ( iRawLen, iBufSize - 1 );
	assert ( iLen < iBufSize );

	if ( !iLen || ( bHeadStar && iLen == 1 ) )
		return 0;

	if ( bMagic ) // pass throu MAGIC_* words
	{
		memcpy ( sBuf, pWord, iLen );
		sBuf[iLen] = '\0';

	} else if ( m_bInfixes )
	{
		////////////////////////////////////
		// infix or mixed infix+prefix mode
		////////////////////////////////////

		// handle head star
		if ( bHeadStar )
		{
			memcpy ( sBuf, pWord + 1, iLen-- ); // chops star, copies trailing zero, updates iLen
		} else
		{
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf + 1, pWord, ++iLen ); // copies everything incl trailing zero, updates iLen
		}

		// handle tail star
		if ( bTailStar )
		{
			sBuf[--iLen] = '\0'; // got star, just chop it away
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL; // no star, add tail marker
			sBuf[++iLen] = '\0';
		}

	} else
	{
		////////////////////
		// prefix-only mode
		////////////////////

		// always ignore head star in prefix mode
		if ( bHeadStar )
		{
			pWord++;
			iLen--;
		}

		// handle tail star
		if ( !bTailStar )
		{
			// exact word search request, always (ie. both in infix/prefix mode) mangles to "\1word\1" in v.8+
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf + 1, pWord, iLen );
			sBuf[iLen + 1] = MAGIC_WORD_TAIL;
			sBuf[iLen + 2] = '\0';
			iLen += 2;

		} else
		{
			// prefix search request, mangles to word itself (just chop away the star)
			memcpy ( sBuf, pWord, iLen );
			sBuf[--iLen] = '\0';
		}
	}

	// calc id for mangled word
	return m_pDict->GetWordID ( sBuf, iLen, !bHeadStar && !bTailStar );
}


void SetupStarDict( DictRefPtr_c& pDict, bool bInfixes )
{
	pDict = new DictStar_c ( pDict, bInfixes );
}
