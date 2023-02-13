//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

/// star dict for index v.8+
class DictStarV8_c: public DictProxy_c
{
	using DictProxy_c::GetWordID;

public:
	DictStarV8_c ( DictRefPtr_c pDict, bool bInfixes )
		: DictProxy_c ( std::move (pDict) )
		, m_bInfixes ( bInfixes )
	{}

	SphWordID_t GetWordID ( BYTE* pWord ) final;

private:
	bool m_bInfixes;
};


SphWordID_t DictStarV8_c::GetWordID ( BYTE* pWord )
{
	char sBuf[16 + 3 * SPH_MAX_WORD_LEN];

	auto iLen = (int)strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16 + 3 * SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	bool bHeadStar = sphIsWild ( pWord[0] );
	bool bTailStar = sphIsWild ( pWord[iLen - 1] ) && ( iLen > 1 );
	bool bMagic = ( pWord[0] < ' ' );

	if ( !bHeadStar && !bTailStar && !bMagic )
	{
		if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;

		m_pDict->ApplyStemmers ( pWord );

		// stemmer might squeeze out the word
		if ( !pWord[0] )
			return 0;

		if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;
	}

	iLen = (int)strlen ( (const char*)pWord );
	assert ( iLen < 16 + 3 * SPH_MAX_WORD_LEN - 2 );

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
	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !bHeadStar && !bTailStar );
}


void SetupStarDictV8( DictRefPtr_c& pDict, bool bInfixes )
{
	pDict = new DictStarV8_c ( pDict, bInfixes );
}