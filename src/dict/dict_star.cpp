//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

/// dict wrapper for star-syntax support in prefix-indexes
class DictStar_c: public DictProxy_c
{
	using DictProxy_c::GetWordID;

public:
	explicit DictStar_c ( DictRefPtr_c pDict )
		: DictProxy_c ( std::move (pDict) )
	{}

	SphWordID_t GetWordID ( BYTE* pWord ) final;
};


SphWordID_t DictStar_c::GetWordID ( BYTE* pWord )
{
	char sBuf[16 + 3 * SPH_MAX_WORD_LEN];
	assert ( strlen ( (const char*)pWord ) < 16 + 3 * SPH_MAX_WORD_LEN );

	if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && m_pDict->IsStopWord ( pWord ) )
		return 0;

	m_pDict->ApplyStemmers ( pWord );

	auto iLen = (int)strlen ( (const char*)pWord );
	assert ( iLen < 16 + 3 * SPH_MAX_WORD_LEN - 1 );
	// stemmer might squeeze out the word
	if ( iLen && !pWord[0] )
		return 0;

	memcpy ( sBuf, pWord, iLen + 1 );

	if ( iLen )
	{
		if ( sphIsWild ( sBuf[iLen - 1] ) )
		{
			iLen--;
			sBuf[iLen] = '\0';
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL;
			iLen++;
			sBuf[iLen] = '\0';
		}
	}

	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !m_pDict->GetSettings().m_bStopwordsUnstemmed );
}

void SetupStarDictOld ( DictRefPtr_c& pDict )
{
	pDict = new DictStar_c ( pDict );
}