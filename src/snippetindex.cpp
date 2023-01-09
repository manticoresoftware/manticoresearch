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

#include "snippetindex.h"
#include "sphinxint.h"


struct KeywordCmp_t
{
	const char * m_pBase;
	explicit KeywordCmp_t ( const char * pBase )
		: m_pBase ( pBase )
	{}

	inline int Cmp ( const SnippetsDocIndex_c::Keyword_t & a, const SnippetsDocIndex_c::Keyword_t & b ) const
	{
		assert ( m_pBase );
		return strncmp ( m_pBase + a.m_iWord, m_pBase + b.m_iWord, 3*SPH_MAX_WORD_LEN+4 );
	}

	inline bool IsLess ( const SnippetsDocIndex_c::Keyword_t & a, const SnippetsDocIndex_c::Keyword_t & b ) const
	{
		int iCmp = Cmp ( a, b );
		if ( iCmp==0 )
			return a.m_iQueryPos<b.m_iQueryPos;
		else
			return iCmp<0;
	}

	inline bool IsSameTerm ( const SnippetsDocIndex_c::Keyword_t & a, const SnippetsDocIndex_c::Keyword_t & b ) const
	{
		return Cmp ( a, b )==0;
	}
};


struct TermCmp_t
{
	inline bool IsLess ( const SnippetsDocIndex_c::Term_t & a, const SnippetsDocIndex_c::Term_t & b ) const
	{
		if ( a.m_iWordId==b.m_iWordId )
			return a.m_iQueryPos<b.m_iQueryPos;
		else
			return a.m_iWordId<b.m_iWordId;
	}

	bool IsSameTerm ( const SnippetsDocIndex_c::Term_t & a, const SnippetsDocIndex_c::Term_t & b ) const
	{
		return ( a.m_iWordId==b.m_iWordId );
	}
};


//////////////////////////////////////////////////////////////////////////
SnippetsDocIndex_c::SnippetsDocIndex_c ( const XQQuery_t & tQuery )
	: m_uLastPos ( 0 )
	, m_tQuery ( tQuery )
{}


void SnippetsDocIndex_c::SetupHits ()
{
	m_dDocHits.Resize ( m_dTerms.GetLength() + m_dStars.GetLength() );
	m_uLastPos = 0;
}


bool SnippetsDocIndex_c::MatchStar ( const Keyword_t & tTok, const BYTE * sWord ) const
{
	assert ( tTok.m_bStar );
	const BYTE * sKeyword = m_dStarBuffer.Begin() + tTok.m_iWord;
	const char * sWildcard = (const char*) sKeyword;
	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;
	return sphWildcardMatch ( (const char*)sWord, (const char*)sKeyword, pWildcard );
}


int SnippetsDocIndex_c::FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const
{
	const Term_t * pQueryTerm = iWordID ? m_dTerms.BinarySearch ( bind ( &Term_t::m_iWordId ), iWordID ) : NULL;
	if ( pQueryTerm )
		return pQueryTerm->m_iQueryPos;

	if ( sWord && iWordLen )
		ARRAY_FOREACH ( i, m_dStars )
		if ( MatchStar ( m_dStars[i], sWord ) )
			return m_dStars[i].m_iQueryPos;

	return -1;
}

int SnippetsDocIndex_c::FindStarred ( const char * sWord ) const
{
	if ( !sWord )
		return -1;

	const BYTE * pBuf = m_dStarBuffer.Begin();
	auto iLen = (int) strlen ( sWord );
	ARRAY_FOREACH ( i, m_dStars )
	{
		const Keyword_t & tTok = m_dStars[i];
		if ( tTok.m_iLength==iLen && tTok.m_bStar && memcmp ( pBuf+tTok.m_iWord, sWord, iLen )==0 )
			return m_dStars[i].m_iQueryPos;
	}

	return -1;
}


void SnippetsDocIndex_c::AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition )
{
	assert ( m_dDocHits.GetLength()==m_dTerms.GetLength()+m_dStars.GetLength() );

	// FIXME!!! replace to 6well formed full-blown infix keyword dict
	const Term_t * pQueryTerm = iWordID ? m_dTerms.BinarySearch ( bind ( &Term_t::m_iWordId ), iWordID ) : NULL;
	if ( pQueryTerm )
	{
		int iQPos = pQueryTerm->m_iQueryPos;
		if ( !m_dDocHits[iQPos].GetLength() || DWORD( m_dDocHits[iQPos].Last() )!=uPosition )
			m_dDocHits [iQPos].Add ( uPosition );

		// might add hit to star hit-list too
		if ( !m_dStarred.BinarySearch ( iWordID ) )
			return;
	}

	if ( sWord && iWordLen )
	{
		ARRAY_FOREACH ( i, m_dStars )
		{
			if ( MatchStar ( m_dStars[i], sWord ) )
			{
				int iQPos = m_dStars[i].m_iQueryPos;
				if ( !m_dDocHits[iQPos].GetLength() || DWORD ( m_dDocHits[iQPos].Last() )!=uPosition )
					m_dDocHits [iQPos].Add ( uPosition );
			}
		}
	}
}


bool HasWildcards ( const char * sWord )
{
	if ( !sWord )
		return false;

	for ( ; *sWord; sWord++ )
	{
		if ( sphIsWild ( *sWord ) )
			return true;
	}

	return false;
}


template<typename T>
void TermShiftDownQpos ( CSphVector<T> & dTerms, const CSphVector<int> & dRemovedQPos )
{
	if ( !dRemovedQPos.GetLength() )
		return;

	ARRAY_FOREACH ( i, dTerms )
	{
		int iCurQPos = dTerms[i].m_iQueryPos;
		int iRemoved = 0;
		while ( iRemoved<dRemovedQPos.GetLength() && iCurQPos>dRemovedQPos[iRemoved] )
			iRemoved++;

		dTerms[i].m_iQueryPos = iCurQPos - iRemoved;
	}
}

template<typename T, typename CMP>
void TermRemoveDup ( CSphVector<T> & dTerms, CSphVector<int> & dRemovedQPos, const CMP & tCmp )
{
	assert ( dTerms.GetLength()>1 );

	int iSrc = 1, iDst = 1;
	while ( iSrc<dTerms.GetLength() )
	{
		if ( tCmp.IsSameTerm ( dTerms[iDst-1], dTerms[iSrc] ) )
		{
			dRemovedQPos.Add ( dTerms[iSrc].m_iQueryPos );
			iSrc++;
		} else
		{
			dTerms[iDst++] = dTerms[iSrc++];
		}
	}
	dTerms.Resize ( iDst );
}


void SnippetsDocIndex_c::ParseQuery ( const DictRefPtr_c& pDict, DWORD eExtQuerySPZ )
{
	int iQPos = 0;

	iQPos = ExtractWords ( m_tQuery.m_pRoot, pDict, iQPos );

	if ( eExtQuerySPZ & SPH_SPZ_SENTENCE )
	{
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord)-1 );
		AddWord ( pDict->GetWordID ( m_sTmpWord ), (int) strlen ( (char*)m_sTmpWord ), iQPos );
		iQPos++;
	}
	if ( eExtQuerySPZ & SPH_SPZ_PARAGRAPH )
	{
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord)-1 );
		AddWord ( pDict->GetWordID ( m_sTmpWord ), (int) strlen ( (char*)m_sTmpWord ), iQPos );
		iQPos++;
	}

	// should be in sync with ExtRanker_c constructor
	ARRAY_FOREACH ( i, m_tQuery.m_dZones )
	{
		snprintf ( (char *)m_sTmpWord, sizeof(m_sTmpWord)-1, "%c%s", MAGIC_CODE_ZONE, m_tQuery.m_dZones[i].cstr() );
		AddWord ( pDict->GetWordID ( m_sTmpWord ), (int) strlen ( (char*)m_sTmpWord ), iQPos );
		iQPos++;
	}

	assert ( !m_dStars.GetLength() || m_dStarBuffer.GetLength() );

	// all ok, remove dupes but keep their positions (needed to calculate LCS)
	CSphVector<int> dRemovedQPos;
	if ( m_dTerms.GetLength()>1 )
	{
		TermCmp_t tCmp;
		m_dTerms.Sort ( tCmp );
		TermRemoveDup ( m_dTerms, dRemovedQPos, tCmp );
	}
	if ( m_dStars.GetLength()>1 )
	{
		KeywordCmp_t tCmp ( (const char *)m_dStarBuffer.Begin() );
		m_dStars.Sort ( tCmp );
		TermRemoveDup ( m_dStars, dRemovedQPos, tCmp );
	}
	if ( dRemovedQPos.GetLength() )
	{
		dRemovedQPos.Sort();
		TermShiftDownQpos ( m_dTerms, dRemovedQPos );
		TermShiftDownQpos ( m_dStars, dRemovedQPos );
	}

	// per qpos weights
	m_dQposToWeight.Resize ( m_dTerms.GetLength() + m_dStars.GetLength() );
#ifndef NDEBUG
	m_dQposToWeight.Fill ( -1 );
#endif

	ARRAY_FOREACH ( i, m_dTerms )
		m_dQposToWeight[m_dTerms[i].m_iQueryPos] = m_dTerms[i].m_iWeight;
	ARRAY_FOREACH ( i, m_dStars )
		m_dQposToWeight[m_dStars[i].m_iQueryPos] = m_dStars[i].m_iWeight;

#ifndef NDEBUG
	bool bFilled = m_dQposToWeight.any_of ( [] ( int iWeight ) { return -1==iWeight; } );
	assert ( !bFilled );
#endif

	// plain terms could also match as starred terms
	if ( m_dStars.GetLength() && m_dTerms.GetLength() )
	{
		CSphVector<const XQNode_t *> dChildren;
		dChildren.Add ( m_tQuery.m_pRoot );
		ARRAY_FOREACH ( i, dChildren )
		{
			const XQNode_t * pChild = dChildren[i];
			if ( !pChild )
				continue;

			for ( const auto & dChild : pChild->m_dChildren )
				dChildren.Add ( dChild );

			for ( const auto& dWord : pChild->m_dWords )
			{
				if ( HasWildcards ( dWord.m_sWord.cstr() ) )
					continue;

				const auto * sWord = (const BYTE *) dWord.m_sWord.cstr();
				int iLen = dWord.m_sWord.Length();
				for ( const auto& dStar : m_dStars )
				{
					if ( MatchStar ( dStar, sWord ) )
					{
						memcpy ( m_sTmpWord, sWord, iLen );
						m_dStarred.Add ( pDict->GetWordID ( m_sTmpWord ) );
						break;
					}
				}
			}
		}

		m_dStarred.Uniq();
	}
}


int SnippetsDocIndex_c::GetTermWeight ( int iQueryPos ) const
{
	return m_dQposToWeight[iQueryPos];
}


int	SnippetsDocIndex_c::GetNumTerms () const
{
	return m_dQposToWeight.GetLength();
}


void SnippetsDocIndex_c::AddWord ( SphWordID_t iWordID, int iLengthCP, int iQpos )
{
	assert ( iWordID );
	Term_t & tTerm = m_dTerms.Add();
	tTerm.m_iWordId = iWordID;
	tTerm.m_iWeight = iLengthCP;
	tTerm.m_iQueryPos = iQpos;
}


void SnippetsDocIndex_c::AddWordStar ( const char * sWord, int iLengthCP, int iQpos )
{
	auto iLen = (int) strlen ( sWord );
	int iOff = m_dStarBuffer.GetLength();

	m_dStarBuffer.Append ( sWord, iLen+1);
	assert (m_dStarBuffer[iOff+iLen] == 0);

	Keyword_t & tTok = m_dStars.Add();
	tTok.m_iWord = iOff;
	tTok.m_iLength = iLen;
	tTok.m_bStar = true;
	tTok.m_iWeight = iLengthCP;
	tTok.m_iQueryPos = iQpos;
}


int SnippetsDocIndex_c::ExtractWords ( XQNode_t * pNode, const DictRefPtr_c& pDict, int iQpos )
{
	if ( !pNode )
		return iQpos;

	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		const XQKeyword_t & tWord = pNode->m_dWords[i];

		int iLenCP = sphUTF8Len ( tWord.m_sWord.cstr() );
		if ( HasWildcards ( tWord.m_sWord.cstr() ) )
		{
			AddWordStar ( tWord.m_sWord.cstr(), iLenCP, iQpos );
			iQpos++;
		} else
		{
			strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord)-1 );
			SphWordID_t iWordID = pDict->GetWordID ( m_sTmpWord );
			if ( iWordID )
			{
				AddWord ( iWordID, iLenCP, iQpos );
				iQpos++;
			}
		}
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		iQpos = ExtractWords ( pNode->m_dChildren[i], pDict, iQpos );

	return iQpos;
}


const CSphVector<DWORD> * SnippetsDocIndex_c::GetHitlist ( const XQKeyword_t & tWord, const DictRefPtr_c & pDict ) const
{
	int iWord = -1;
	if ( HasWildcards ( tWord.m_sWord.cstr() ) )
		iWord = FindStarred ( tWord.m_sWord.cstr() );
	else
	{
		strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord)-1 );
		SphWordID_t iWordID = pDict->GetWordID(m_sTmpWord);
		if ( iWordID )
			iWord = FindWord ( iWordID, NULL, 0 );
	}

	if ( iWord!=-1 )
	{
		auto * pFirstHit = m_dDocHits.Begin();
		assert ( pFirstHit );
		return pFirstHit+iWord;
	}

	return nullptr;
}
