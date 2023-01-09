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

#pragma once

//////////////////////////////////////////////////////////////////////////


template<CRCALGO ALGO>
SphWordID_t CSphDictCRC<ALGO>::GetWordID ( BYTE* pWord )
{
	// apply stopword filter before stemmers
	if ( GetSettings().m_bStopwordsUnstemmed && !FilterStopword ( tHASH::DoCrc ( pWord ) ) )
		return 0;

	// skip stemmers for magic words
	if ( pWord[0] >= 0x20 )
		ApplyStemmers ( pWord );

	// stemmer might squeeze out the word
	if ( !pWord[0] )
		return 0;

	return GetSettings().m_bStopwordsUnstemmed
			 ? tHASH::DoCrc ( pWord )
			 : FilterStopword ( tHASH::DoCrc ( pWord ) );
}


template<CRCALGO ALGO>
SphWordID_t CSphDictCRC<ALGO>::GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = tHASH::DoCrc ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


template<CRCALGO ALGO>
SphWordID_t CSphDictCRC<ALGO>::GetWordIDWithMarkers ( BYTE* pWord )
{
	ApplyStemmers ( pWord + 1 );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord + 1 );
	auto iLength = strlen ( (const char*)( pWord + 1 ) );
	pWord[iLength + 1] = MAGIC_WORD_TAIL;
	pWord[iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? tHASH::DoCrc ( pWord ) : 0;
}


template<CRCALGO ALGO>
SphWordID_t CSphDictCRC<ALGO>::GetWordIDNonStemmed ( BYTE* pWord )
{
	// this method can generally receive both '=stopword' with a marker and 'stopword' without it
	// so for filtering stopwords, let's handle both
	int iOff = ( pWord[0] < ' ' );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord + iOff );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return tHASH::DoCrc ( pWord );
}


template<CRCALGO ALGO>
bool CSphDictCRC<ALGO>::IsStopWord ( const BYTE* pWord ) const
{
	return FilterStopword ( tHASH::DoCrc ( pWord ) ) == 0;
}
