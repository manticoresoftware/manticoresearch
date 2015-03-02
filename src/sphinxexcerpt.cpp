//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxexcerpt.h"
#include "sphinxutils.h"
#include "sphinxsearch.h"
#include "sphinxquery.h"
#include "sphinxint.h"

#include <math.h>

/////////////////////////////////////////////////////////////////////////////
// THE EXCERPTS GENERATOR
/////////////////////////////////////////////////////////////////////////////

static const int MAX_HIGHLIGHT_WORDS = 256;

#define UINT32_MASK 0xffffffffUL
#define UINT16_MASK 0xffff
typedef uint64_t ZonePacked_t;


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

	return s-sData;
}

uint64_t sphPackZone ( DWORD uPosition, int iSiblingIndex, int iZoneType )
{
	assert ( iSiblingIndex>=0 && iSiblingIndex<UINT16_MASK );
	assert ( iZoneType>=0 && iZoneType<UINT16_MASK );

	return ( ( (uint64_t)uPosition<<32 )
		| ( ( iSiblingIndex & UINT16_MASK )<<16 )
		| ( iZoneType & UINT16_MASK ) );
}

int FindAddZone ( const char * sZoneName, int iZoneNameLen, SmallStringHash_T<int> & hZones )
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


static inline int GetWordsLimit ( const ExcerptQuery_t & q, int iQwords )
{
	return q.m_iLimitWords ? q.m_iLimitWords : 2*q.m_iAround + iQwords;
}

//////////////////////////////////////////////////////////////////////////
struct DocQueryZonePair_t
{
	int m_iDoc;
	int m_iQuery;
	bool operator<( const DocQueryZonePair_t & b ) const { return m_iDoc<b.m_iDoc; }
	bool operator>( const DocQueryZonePair_t & b ) const { return m_iDoc>b.m_iDoc; }
	bool operator==( const DocQueryZonePair_t & b ) const { return m_iDoc==b.m_iDoc; }
};

/// hit-in-zone check implementation for the matching engine
class SnippetZoneChecker_c : public ISphZoneCheck
{
private:
	struct ZoneHits_t
	{
		CSphVector<Hitpos_t> m_dOpen;
		CSphVector<Hitpos_t> m_dClose;
	};

	CSphVector<ZoneHits_t> m_dZones;

public:
	SnippetZoneChecker_c ( const CSphVector<ZonePacked_t> & dDocZones,
		const SmallStringHash_T<int> & hDocNames, const CSphVector<CSphString> & dQueryZones )
	{
		if ( !dQueryZones.GetLength() )
			return;

		CSphVector<DocQueryZonePair_t> dCheckedZones;
		ARRAY_FOREACH ( i, dQueryZones )
		{
			int * pZone = hDocNames ( dQueryZones[i] );
			if ( pZone )
			{
				DocQueryZonePair_t & tPair = dCheckedZones.Add ();
				tPair.m_iDoc = *pZone;
				tPair.m_iQuery = i;
			}
		}

		dCheckedZones.Sort();
		m_dZones.Resize ( dQueryZones.GetLength() );

		ARRAY_FOREACH ( i, dDocZones )
		{
			uint64_t uZonePacked = dDocZones[i];
			DWORD uPos = (DWORD)( ( uZonePacked >>32 ) & UINT32_MASK );
			int iSibling = (int)( ( uZonePacked>>16 ) & UINT16_MASK );
			int iZone = (int)( uZonePacked & UINT16_MASK );
			assert ( iSibling>=0 && iSibling<dDocZones.GetLength() );
			assert ( iZone==(int)( dDocZones[iSibling] & UINT16_MASK ) );

			// skip cases:
			// + close zone (tSpan.m_iSibling<i) - skipped
			// + open without close zone (tSpan.m_iSibling==i) - skipped
			// + open zone position > close zone position
			// + zone type not in query zones
			if ( iSibling<=i || uPos>=( ( dDocZones[iSibling]>>32 ) & UINT32_MASK ) )
				continue;

			DocQueryZonePair_t tRefZone;
			tRefZone.m_iDoc = iZone;
			const DocQueryZonePair_t * pPair = dCheckedZones.BinarySearch ( tRefZone );
			if ( !pPair )
				continue;

			uint64_t uClosePacked = dDocZones[iSibling];
			DWORD uClosePos = ( (int)( uClosePacked>>32 ) & UINT32_MASK );

			ZoneHits_t & tZone = m_dZones[pPair->m_iQuery];
			tZone.m_dOpen.Add ( uPos );
			tZone.m_dClose.Add ( uClosePos );
		}

#ifndef NDEBUG
		ARRAY_FOREACH ( i, m_dZones )
		{
			const ZoneHits_t & tZone = m_dZones[i];
			assert ( tZone.m_dOpen.GetLength()==tZone.m_dClose.GetLength() );
			const Hitpos_t * pHit = tZone.m_dOpen.Begin()+1;
			const Hitpos_t * pMax = tZone.m_dOpen.Begin()+tZone.m_dOpen.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
			pHit = tZone.m_dClose.Begin()+1;
			pMax = tZone.m_dClose.Begin()+tZone.m_dClose.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
		}
#endif
	}

	virtual SphZoneHit_e IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan )
	{
		DWORD uPosWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
		int iOpen = FindSpan ( m_dZones[iZone].m_dOpen, uPosWithField );
		if ( pLastSpan )
			* pLastSpan = iOpen;
		return ( iOpen>=0 && uPosWithField<=m_dZones[iZone].m_dClose[iOpen] ) ? SPH_ZONE_FOUND : SPH_ZONE_NO_SPAN;
	}
};

//////////////////////////////////////////////////////////////////////////
/// mini-index for a single document
/// keeps query words
/// keeps hit lists for every query keyword
class SnippetsDocIndex_c : public ISphNoncopyable
{
public:
	// document related
	DWORD									m_uLastPos;
	CSphVector< CSphVector<int> >			m_dDocHits;

	// query parsing result
	const XQQuery_t	&						m_tQuery;

protected:
	// query keywords and parsing stuff
	struct Keyword_t
	{
		int			m_iWord;
		int			m_iLength;
		bool		m_bStar;
		int			m_iWeight;
		int			m_iQueryPos;
	};

	struct Term_t
	{
		SphWordID_t	m_iWordId;
		int			m_iWeight;
		int			m_iQueryPos;
	};

	CSphVector<Term_t>						m_dTerms;
	CSphVector<SphWordID_t>					m_dStarred;
	CSphVector<Keyword_t>					m_dStars;
	CSphVector<BYTE>						m_dStarBuffer;
	CSphVector<int>							m_dQposToWeight;

	bool									m_bQueryMode;
	BYTE									m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];

public:
	SnippetsDocIndex_c ( bool bQueryMode, const XQQuery_t & tQuery );
	void		SetupHits ();
	int			FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const;
	int			FindStarred ( const char * sWord ) const;
	void		AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition );
	void		ParseQuery ( const char * sQuery, ISphTokenizer * pTokenizer, CSphDict * pDict, DWORD eExtQuerySPZ );
	int			GetTermWeight ( int iQueryPos ) const;
	int			GetNumTerms () const;

protected:
	bool		MatchStar ( const Keyword_t & tTok, const BYTE * sWord ) const;
	void		AddWord ( SphWordID_t iWordID, int iLengthCP, int iQpos );
	void		AddWordStar ( const char * sWord, int iLengthCP, int iQpos );
	int			ExtractWords ( XQNode_t * pNode, ISphTokenizer * pTokenizer, CSphDict * pDict, int iQpos );

	struct KeywordCmp_t
	{
		const char * m_pBase;
		explicit KeywordCmp_t ( const char * pBase )
			: m_pBase ( pBase )
		{}

		inline int Cmp ( const Keyword_t & a, const Keyword_t & b ) const
		{
			assert ( m_pBase );
			return strncmp ( m_pBase + a.m_iWord, m_pBase + b.m_iWord, 3*SPH_MAX_WORD_LEN+4 );
		}

		inline bool IsLess ( const Keyword_t & a, const Keyword_t & b ) const
		{
			int iCmp = Cmp ( a, b );
			if ( iCmp==0 )
				return a.m_iQueryPos<b.m_iQueryPos;
			else
				return iCmp<0;
		}

		bool IsSameTerm ( const Keyword_t & a, const Keyword_t & b ) const
		{
			return Cmp ( a, b )==0;
		}
	};

	struct TermCmp_t
	{
		inline bool IsLess ( const Term_t & a, const Term_t & b ) const
		{
			if ( a.m_iWordId==b.m_iWordId )
				return a.m_iQueryPos<b.m_iQueryPos;
			else
				return a.m_iWordId<b.m_iWordId;
		}

		bool IsSameTerm ( const Term_t & a, const Term_t & b ) const
		{
			return ( a.m_iWordId==b.m_iWordId );
		}
	};
};


SnippetsDocIndex_c::SnippetsDocIndex_c ( bool bQueryMode, const XQQuery_t & tQuery )
	: m_uLastPos ( 0 )
	, m_tQuery ( tQuery )
	, m_bQueryMode ( bQueryMode )
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
	return sphWildcardMatch ( (const char*)sWord, (const char*)sKeyword );
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
	int iLen = strlen ( sWord );
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
		ARRAY_FOREACH ( i, m_dStars )
			if ( MatchStar ( m_dStars[i], sWord ) )
			{
				int iQPos = m_dStars[i].m_iQueryPos;
				if ( !m_dDocHits[iQPos].GetLength() || DWORD ( m_dDocHits[iQPos].Last() )!=uPosition )
					m_dDocHits [iQPos].Add ( uPosition );
			}
}

static bool HasStars ( const XQKeyword_t & w )
{
	return w.m_sWord.Begins("*") || w.m_sWord.Ends("*");
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


void SnippetsDocIndex_c::ParseQuery ( const char * sQuery, ISphTokenizer * pTokenizer, CSphDict * pDict, DWORD eExtQuerySPZ )
{
	int iQPos = 0;

	if ( !m_bQueryMode )
	{
		// parse bag-of-words query
		int iQueryLen = strlen ( sQuery ); // FIXME!!! get length as argument
		pTokenizer->SetBuffer ( (BYTE *)sQuery, iQueryLen );

		BYTE * sWord = NULL;
		// FIXME!!! add warning on query words overflow
		while ( ( sWord = pTokenizer->GetToken() )!=NULL && iQPos<MAX_HIGHLIGHT_WORDS )
		{
			SphWordID_t uWordID = pDict->GetWordID ( sWord );
			if ( uWordID )
			{
				if ( sWord[0]=='*' || sWord [ strlen ( (const char*)sWord )-1 ]=='*' )
					AddWordStar ( (const char *)sWord, pTokenizer->GetLastTokenLen(), iQPos );
				else
					AddWord ( uWordID, pTokenizer->GetLastTokenLen(), iQPos );

				iQPos++;
			}
			if ( pTokenizer->TokenIsBlended() )
				pTokenizer->SkipBlended();
		}
	} else
	{
		iQPos = ExtractWords ( m_tQuery.m_pRoot, pTokenizer, pDict, iQPos );

		if ( eExtQuerySPZ & SPH_SPZ_SENTENCE )
		{
			strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord) );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ), iQPos );
			iQPos++;
		}
		if ( eExtQuerySPZ & SPH_SPZ_PARAGRAPH )
		{
			strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord) );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ), iQPos );
			iQPos++;
		}

		// should be in sync with ExtRanker_c constructor
		ARRAY_FOREACH ( i, m_tQuery.m_dZones )
		{
			snprintf ( (char *)m_sTmpWord, sizeof(m_sTmpWord), "%c%s", MAGIC_CODE_ZONE, m_tQuery.m_dZones[i].cstr() );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ), iQPos );
			iQPos++;
		}
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
	bool bFilled = ARRAY_ANY ( bFilled, m_dQposToWeight, ( m_dQposToWeight[_any]==-1 ) );
	assert ( !bFilled );
#endif

	// plain terms could also match as starred terms
	if ( m_dStars.GetLength() && m_dTerms.GetLength() && m_bQueryMode )
	{
		CSphVector<const XQNode_t *> dChildren;
		dChildren.Add ( m_tQuery.m_pRoot );
		ARRAY_FOREACH ( i, dChildren )
		{
			const XQNode_t * pChild = dChildren[i];
			if ( !pChild )
				continue;

			ARRAY_FOREACH ( j, pChild->m_dChildren )
				dChildren.Add ( pChild->m_dChildren[j] );

			ARRAY_FOREACH ( j, pChild->m_dWords )
			{
				if ( HasStars ( pChild->m_dWords[j] ) )
					continue;

				const BYTE * sWord = (const BYTE *)pChild->m_dWords[j].m_sWord.cstr();
				int iLen = pChild->m_dWords[j].m_sWord.Length();
				ARRAY_FOREACH ( k, m_dStars )
				{
					if ( MatchStar ( m_dStars[k], sWord ) )
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
	int iLen = strlen ( sWord );
	int iOff = m_dStarBuffer.GetLength();

	m_dStarBuffer.Resize ( iOff+iLen+1 ); // reserve space for word + trailing zero
	memcpy ( &m_dStarBuffer[iOff], sWord, iLen );
	m_dStarBuffer[iOff+iLen] = 0;

	Keyword_t & tTok = m_dStars.Add();
	tTok.m_iWord = iOff;
	tTok.m_iLength = iLen;
	tTok.m_bStar = true;
	tTok.m_iWeight = iLengthCP;
	tTok.m_iQueryPos = iQpos;
}


int SnippetsDocIndex_c::ExtractWords ( XQNode_t * pNode, ISphTokenizer * pTokenizer, CSphDict * pDict, int iQpos )
{
	if ( !pNode )
		return iQpos;

	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		const XQKeyword_t & tWord = pNode->m_dWords[i];

		int iLenCP = sphUTF8Len ( tWord.m_sWord.cstr() );
		if ( HasStars ( tWord ) )
		{
			AddWordStar ( tWord.m_sWord.cstr(), iLenCP, iQpos );
			iQpos++;
		} else
		{
			strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord) );
			SphWordID_t iWordID = pDict->GetWordID ( m_sTmpWord );
			if ( iWordID )
			{
				AddWord ( iWordID, iLenCP, iQpos );
				iQpos++;
			}
		}
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		iQpos = ExtractWords ( pNode->m_dChildren[i], pTokenizer, pDict, iQpos );

	return iQpos;
}


//////////////////////////////////////////////////////////////////////////
struct FunctorZoneInfo_t
{
	SmallStringHash_T<int>	m_hZones;
	CSphVector<int>			m_dZonePos;
	CSphVector<int>			m_dZoneParent;
};


/// document token processor functor traits
class TokenFunctorTraits_c : public ISphNoncopyable, public ExcerptQuery_t
{
public:
	FunctorZoneInfo_t			m_tZoneInfo;
	CSphVector<ZonePacked_t>	m_dZones;
	mutable CSphVector<BYTE>	m_dResult;

	SnippetsDocIndex_c &	m_tContainer;
	ISphTokenizer *			m_pTokenizer;
	CSphDict *				m_pDict;
	const char *			m_szDocBuffer;
	const char *			m_pDoc;
	const char *			m_pDocMax;

	int		m_iBoundaryStep;
	int		m_iStopwordStep;
	bool	m_bIndexExactWords;
	int		m_iDocLen;
	int		m_iMatchesCount;
	bool	m_bCollectExtraZoneInfo;

	explicit TokenFunctorTraits_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen )
		: m_tContainer ( tContainer )
		, m_pTokenizer ( pTokenizer )
		, m_pDict ( pDict )
		, m_szDocBuffer ( sDoc )
		, m_pDoc ( NULL )
		, m_pDocMax ( NULL )
		, m_iBoundaryStep ( tSettingsIndex.m_iBoundaryStep )
		, m_iStopwordStep ( tSettingsIndex.m_iStopwordStep )
		, m_bIndexExactWords ( tSettingsIndex.m_bIndexExactWords )
		, m_iDocLen ( iDocLen )
		, m_iMatchesCount ( 0 )
		, m_bCollectExtraZoneInfo ( false )
	{
		assert ( m_pTokenizer && m_pDict );
		ExcerptQuery_t::operator = ( tQuery );
		m_pTokenizer->SetBuffer ( (BYTE*)sDoc, m_iDocLen );
		m_pDoc = m_pTokenizer->GetBufferPtr();
		m_pDocMax = m_pTokenizer->GetBufferEnd();
	}

	void ResultEmit ( const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0,
		const char * pPost=NULL, int iPostLen=0 ) const
	{
		ResultEmit ( m_dResult, pSrc, iLen, bHasPassageMacro, iPassageId, pPost, iPostLen );
	}

	void ResultEmit ( CSphVector<BYTE> & dResult, const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0,
		const char * pPost=NULL, int iPostLen=0 ) const
	{
		if ( iLen>0 )
		{
			int iOutLen = dResult.GetLength();
			dResult.Resize ( iOutLen+iLen );
			memcpy ( &dResult[iOutLen], pSrc, iLen );
		}

		if ( !bHasPassageMacro )
			return;

		char sBuf[16];
		int iPassLen = snprintf ( sBuf, sizeof(sBuf), "%d", iPassageId );
		int iOutLen = dResult.GetLength();
		dResult.Resize ( iOutLen + iPassLen + iPostLen );

		if ( iPassLen )
			memcpy ( dResult.Begin()+iOutLen, sBuf, iPassLen );
		if ( iPostLen )
			memcpy ( dResult.Begin()+iOutLen+iPassLen, pPost, iPostLen );
	}
};

//////////////////////////////////////////////////////////////////////////

struct TokenInfo_t
{
	int				m_iStart;
	int				m_iLen;
	const BYTE *	m_sWord;
	DWORD			m_uPosition;
	bool			m_bWord;
	bool			m_bStopWord;
	int				m_iTermIndex;

	SphWordID_t		m_uWordId;
};


/// functor that maps collected tokens into a stream
class CacheStreamer_c
{
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

		TYPE_TOTAL
	};
	STATIC_ASSERT ( TYPE_TOTAL<=15, OUT_OF_TYPECODES );

public:
	FunctorZoneInfo_t *		m_pZoneInfo;

private:
	CSphTightVector<BYTE>	m_dTokenStream;
	int						m_iReadPtr;
	int						m_iLastStart;	///< last delta coded token offset, in bytes
	int						m_iLastPos;		///< last delta coded token number, in tokens
	int						m_eLastStored;

public:
	explicit CacheStreamer_c ( int iDocLen )
		: m_pZoneInfo ( NULL )
		, m_iReadPtr ( 0 )
		, m_iLastStart ( 0 )
		, m_iLastPos ( 0 )
		, m_eLastStored ( TYPE_TOTAL )
	{
		m_dTokenStream.Reserve ( (iDocLen*2)/5 );
		m_dTokenStream.Add ( 0 );
	}

	inline BYTE * StoreEntry ( int iBytes )
	{
		m_dTokenStream.Resize ( m_dTokenStream.GetLength() + iBytes );
		return m_dTokenStream.Begin() + m_dTokenStream.GetLength() - iBytes;
	}

	void StoreOverlap ( int iStart, int iLen, int iBoundary )
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
					m_dTokenStream.Last() = ( uType<<4 )+ (BYTE)iTokLen-1;
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

	void StoreSkipHtml ( int iStart, int iLen )
	{
		m_dTokenStream.Add ( TYPE_SKIPHTML<<4 );
		ZipInt ( iStart );
		ZipInt ( iLen );
		m_eLastStored = TYPE_SKIPHTML;
	}

	void StoreToken ( const TokenInfo_t & tTok )
	{
		assert ( tTok.m_iTermIndex<USHRT_MAX );
		assert ( tTok.m_iLen<=255 );

		int iDstart = tTok.m_iStart - m_iLastStart;
		int iDpos = tTok.m_uPosition - m_iLastPos;

		m_iLastStart = tTok.m_iStart + tTok.m_iLen;
		m_iLastPos = tTok.m_uPosition;

		if ( iDstart==0 && iDpos==1 && tTok.m_bWord && !tTok.m_bStopWord && tTok.m_iTermIndex==-1 )
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
				m_dTokenStream.Add ( (BYTE)( TYPE_TOKEN2<<4 ) );
				m_dTokenStream.Add ( (BYTE) tTok.m_iLen );
				m_eLastStored = TYPE_TOKEN2;
				return;
			}
		}

		// TOKEN, stupid generic uncompressed path (can optimize with deltas, if needed)
		BYTE * p = StoreEntry ( 13 );

		p[0] = ( TYPE_TOKEN<<4 );
		sphUnalignedWrite ( p+1, tTok.m_iStart );
		p[5] = BYTE(tTok.m_iLen);
		sphUnalignedWrite ( p+6, tTok.m_uPosition );
		p[10] = ( tTok.m_bWord<<1 ) + tTok.m_bStopWord;
		sphUnalignedWrite ( p+11, (WORD)(tTok.m_iTermIndex+1) );
		m_eLastStored = TYPE_TOKEN;
	}

	void StoreSPZ ( BYTE iSPZ, DWORD uPosition, const char *, int iZone )
	{
		m_dTokenStream.Add ( TYPE_SPZ<<4 );
		ZipInt ( iSPZ );
		ZipInt ( uPosition );
		ZipInt ( iZone==-1 ? 0 : 1 );
		if ( iZone!=-1 )
			ZipInt ( iZone );

		m_eLastStored = TYPE_SPZ;
	}

	void StoreTail ( int iStart, int iLen, int iBoundary )
	{
		m_dTokenStream.Add ( TYPE_TAIL<<4 );
		ZipInt ( iStart );
		ZipInt ( iLen );
		ZipInt ( iBoundary==-1 ? 0 : 1 );
		if ( iBoundary!=-1 )
			ZipInt ( iBoundary );

		m_eLastStored = TYPE_TAIL;
	}

	template < typename T >
	void Tokenize ( T & tFunctor )
	{
		m_iLastStart = 0;
		m_iLastPos = 0;

		m_iReadPtr = 1;
		TokenInfo_t tTok;
		bool bStop = false;
		CSphVector<SphWordID_t> dTmp;

		while ( m_iReadPtr < m_dTokenStream.GetLength() )
		{
			switch ( m_dTokenStream [ m_iReadPtr ]>>4 )
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
				{
					BYTE * p = &m_dTokenStream [ m_iReadPtr ];
					tTok.m_iStart = sphUnalignedRead ( *(DWORD*)(p+1) );
					tTok.m_iLen = p[5];
					tTok.m_uPosition = sphUnalignedRead ( *(DWORD*)(p+6) );
					tTok.m_bWord = ( p[10] & 2 )!=0;
					tTok.m_bStopWord = ( p[10] & 1 )!=0;
					tTok.m_iTermIndex = (int)sphUnalignedRead ( *(WORD*)(p+11) ) - 1;
					m_iReadPtr += 13;

					m_iLastStart = tTok.m_iStart + tTok.m_iLen;
					m_iLastPos = tTok.m_uPosition;

					tTok.m_sWord = NULL;

					bStop = !tFunctor.OnToken ( tTok, dTmp );
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

					tTok.m_sWord = NULL;

					bStop = !tFunctor.OnToken ( tTok, dTmp );
				}
				break;

			case TYPE_TOKEN2:
				{
					tTok.m_iStart = m_iLastStart;
					tTok.m_iLen = m_dTokenStream [ ++m_iReadPtr ];
					m_iReadPtr++;
					m_iLastStart += tTok.m_iLen;
					tTok.m_uPosition = ++m_iLastPos;
					tTok.m_bWord = true;
					tTok.m_bStopWord = false;
					tTok.m_iTermIndex = -1;

					tTok.m_sWord = NULL;

					bStop = !tFunctor.OnToken ( tTok, dTmp );
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

					tTok.m_sWord = NULL;

					bStop = !tFunctor.OnToken ( tTok, dTmp );

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

					tTok.m_sWord = NULL;

					bStop = !tFunctor.OnToken ( tTok, dTmp );

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
						m_pZoneInfo->m_hZones.IterateStart();
						while ( m_pZoneInfo->m_hZones.IterateNext () )
							if ( m_pZoneInfo->m_hZones.IterateGet()==iZone )
							{
								szZoneName = m_pZoneInfo->m_hZones.IterateGetKey().cstr();
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

private:
	inline void ZipInt ( DWORD uValue )
	{
		int iBytes = 1;

		DWORD u = ( uValue>>7 );
		while ( u )
		{
			u >>= 7;
			iBytes++;
		}

		while ( iBytes-- )
		{
			int iData = ( 0x7f & ( uValue >> (7*iBytes) ) ) | ( iBytes ? 0x80 : 0 );
			m_dTokenStream.Add ( BYTE ( iData & 0xff ) );
		}
	}

	inline DWORD UnzipInt()
	{
		DWORD uByte = m_dTokenStream[m_iReadPtr++];
		DWORD uRes = 0;
		while ( uByte & 0x80 )
		{
			uRes = ( uRes<<7 ) + ( uByte & 0x7f );
			uByte = m_dTokenStream[m_iReadPtr++];
		}

		uRes = ( uRes<<7 ) + uByte;

		return uRes;
	}
};


/// functor that processes tokens and collects matching keyword hits into mini-index
class HitCollector_c : public TokenFunctorTraits_c
{
public:
	mutable BYTE		m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];
	SphWordID_t			m_uSentenceID;
	SphWordID_t			m_uParagraphID;
	DWORD				m_uFoundWords;
	CacheStreamer_c &	m_tTokenContainer;

public:
	explicit HitCollector_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen, CacheStreamer_c & tTokenContainer )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_uFoundWords ( 0 )
		, m_tTokenContainer ( tTokenContainer )
	{
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord) );
		m_uSentenceID = pDict->GetWordID ( m_sTmpWord );
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord) );
		m_uParagraphID = pDict->GetWordID ( m_sTmpWord );
		m_tContainer.SetupHits();

		m_bCollectExtraZoneInfo = true;
	}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens )
	{
		bool bReal = false;
		if ( tTok.m_uWordId )
		{
			m_tContainer.AddHits ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen, tTok.m_uPosition );
			bReal = true;
		}
		ARRAY_FOREACH ( i, dTokens )
		{
			if ( dTokens[i] )
			{
				m_tContainer.AddHits ( dTokens[i], tTok.m_sWord, tTok.m_iLen, tTok.m_uPosition );
				bReal = true;
			}
		}

		m_tContainer.m_uLastPos = bReal ? tTok.m_uPosition : m_tContainer.m_uLastPos;

		int iTermIndex = m_tContainer.FindWord ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen );
		ARRAY_FOREACH_COND ( i, dTokens, iTermIndex==-1 )
			iTermIndex = m_tContainer.FindWord ( dTokens[i], NULL, 0 );

		m_uFoundWords |= iTermIndex==-1 ? 0 : 1 << iTermIndex;
		tTok.m_iTermIndex = iTermIndex;

		m_tTokenContainer.StoreToken ( tTok );

		return true;
	}

	void OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * sZoneName, int iZone )
	{
		switch ( iSPZ )
		{
		case MAGIC_CODE_SENTENCE:
			m_tContainer.AddHits ( m_uSentenceID, NULL, 0, uPosition );
			break;
		case MAGIC_CODE_PARAGRAPH:
			m_tContainer.AddHits ( m_uParagraphID, NULL, 0, uPosition );
			break;
		case MAGIC_CODE_ZONE:
			assert ( m_dZones.GetLength() );
			assert ( ( ( m_dZones.Last()>>32 ) & UINT32_MASK )==uPosition );
			assert ( sZoneName );

			m_tContainer.AddHits ( m_pDict->GetWordID ( (BYTE *)sZoneName ), NULL, 0, uPosition );
			break;
		default: assert ( 0 && "impossible SPZ" );
		}

		m_tContainer.m_uLastPos = uPosition;

		m_tTokenContainer.StoreSPZ ( iSPZ, uPosition, sZoneName, iZone );
	}

	const CSphVector<int> * GetHitlist ( const XQKeyword_t & tWord ) const
	{
		int iWord = -1;
		if ( HasStars ( tWord ) )
			iWord = m_tContainer.FindStarred ( tWord.m_sWord.cstr() );
		else
		{
			strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord) );
			SphWordID_t iWordID = m_pDict->GetWordID ( m_sTmpWord );
			if ( iWordID )
				iWord = m_tContainer.FindWord ( iWordID, NULL, 0 );
		}

		if ( iWord!=-1 )
			return m_tContainer.m_dDocHits.Begin()+iWord;
		else
			return NULL;
	}

	bool OnOverlap ( int iStart, int iLen, int iBoundary )
	{
		m_tTokenContainer.StoreOverlap ( iStart, iLen, iBoundary );
		return true;
	}

	void OnSkipHtml ( int iStart, int iLen )
	{
		m_tTokenContainer.StoreSkipHtml ( iStart, iLen );
	}

	void OnTail ( int iStart, int iLen, int iBoundary )
	{
		m_tTokenContainer.StoreTail ( iStart, iLen, iBoundary );
	}

	void OnFinish () {}
};


/// functor that matches tokens against query words from mini-index and highlights them
class HighlightPlain_c : public TokenFunctorTraits_c
{
protected:
	int		m_iBeforeLen;
	int		m_iAfterLen;
	int		m_iBeforePostLen;
	int		m_iAfterPostLen;

public:
	HighlightPlain_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen, int iReserveResult )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_iBeforeLen ( tQuery.m_sBeforeMatch.Length() )
		, m_iAfterLen ( tQuery.m_sAfterMatch.Length() )
		, m_iBeforePostLen ( tQuery.m_sBeforeMatchPassage.Length() )
		, m_iAfterPostLen ( tQuery.m_sAfterMatchPassage.Length() )
	{
		m_dResult.Reserve ( iReserveResult );
	}

	bool OnOverlap ( int iStart, int iLen, int )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		ResultEmit ( m_pDoc+iStart, iLen );
		return true;
	}

	void OnSkipHtml ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens )
	{
		assert ( m_pDoc );
		assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

		bool bMatch = m_tContainer.FindWord ( tTok.m_uWordId, tTok.m_sWord , tTok.m_iLen )!=-1; // the primary one; need this for star matching
		ARRAY_FOREACH_COND ( i, dTokens, !bMatch )
			bMatch = ( m_tContainer.FindWord ( dTokens[i], NULL, 0 )!=-1 );

		if ( bMatch )
		{
			ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro,
				m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
			m_iMatchesCount++;
		}

		ResultEmit ( m_pDoc+tTok.m_iStart, tTok.m_iLen );

		if ( bMatch )
			ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro,
				m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );

		return true;
	}

	void OnSPZ ( BYTE , DWORD, const char *, int ) {}

	void OnTail ( int iStart, int iLen, int )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	void OnFinish () {}
	const CSphVector<int> * GetHitlist ( const XQKeyword_t & ) const { return NULL;	}
};


static int EstimateResult ( const ExcerptQuery_t & q, int iDocLen )
{
	if ( q.m_iLimit>0 )
		return 2*q.m_iLimit;
	if ( q.m_iLimitPassages )
		return 256*q.m_iLimitPassages;
	if ( q.m_iLimitWords )
		return 16*q.m_iLimitWords;
	return iDocLen;
}


/// functor that matches tokens against hit positions from mini-index and highlights them
class HighlightQuery_c : public HighlightPlain_c
{
public:
	const SphHitMark_t *	m_pHit;
	const SphHitMark_t *	m_pHitEnd;
	int						m_iOpenTill; // blend-chars has same positions as blend-part tokens
	int						m_iLastPos;

public:
	HighlightQuery_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict,
		const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits )
		: HighlightPlain_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen, EstimateResult ( tQuery, iDocLen ) )
		, m_pHit ( dHits.Begin() )
		, m_pHitEnd ( dHits.Begin()+dHits.GetLength() )
		, m_iOpenTill ( 0 )
		, m_iLastPos ( 0 )
	{}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & )
	{
		assert ( m_pDoc );
		assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

		// fast forward until next potentially matching hit (hits are sorted by position)
		while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=tTok.m_uPosition )
			m_pHit++;

		CheckClose ( tTok.m_uPosition );

		// marker folding, emit "before" marker at span start only
		if ( m_pHit<m_pHitEnd && tTok.m_uPosition==m_pHit->m_uPosition && !m_iOpenTill )
		{
			ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro,
				m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
			m_iMatchesCount++;
			m_iOpenTill = m_pHit->m_uPosition+m_pHit->m_uSpan;
		}

		// emit token itself
		ResultEmit ( m_pDoc+tTok.m_iStart, tTok.m_iLen );
		m_iLastPos = tTok.m_uPosition;
		return true;
	}

	void OnTail ( int iStart, int iLen, int )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		CheckClose ( m_iLastPos+1 );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	void OnFinish ()
	{
		if ( !m_iOpenTill )
			return;
		ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	}

	bool OnOverlap ( int iStart, int iLen, int )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		CheckClose ( m_iLastPos+1 );
		ResultEmit ( m_pDoc+iStart, iLen );
		return true;
	}

	void OnSkipHtml ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
		CheckClose ( m_iLastPos+1 );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

private:
	void CheckClose ( int iPos )
	{
		// marker folding, emit "after" marker at span end only
		if ( !m_iOpenTill || iPos<m_iOpenTill )
			return;

		ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
		m_iOpenTill = 0;
	}
};


struct StoredExcerptToken_t
{
	int				m_iLengthCP;
	BYTE			m_iWordFlag;
};

struct Passage_t
{
	int					m_iStart;			///< start token index
	int					m_iTokens;			///< token count
	int					m_iCodes;			///< codepoints count
	int					m_iWords;			///< words count
	DWORD				m_uQwords;			///< matching query words mask
	int					m_iQwordsWeight;	///< passage weight factor
	int					m_iQwordCount;		///< passage weight factor
	int					m_iUniqQwords;		///< passage weight factor
	int					m_iMaxLCS;			///< passage weight factor
	int					m_iMinGap;			///< passage weight factor
	int					m_iStartLimit;		///< start of match in passage
	int					m_iEndLimit;		///< end of match in passage
	int					m_iAroundBefore;	///< words before first matched words
	int					m_iAroundAfter;		///< words after last matched words
	int					m_iCodesBetweenKeywords;	///< codepoints between m_iStartLimit and m_iEndLimit
	int					m_iWordsBetweenKeywords;	///< words between m_iStartLimit and m_iEndLimit

	CSphVector<StoredExcerptToken_t> m_dBeforeTokens;	///< stored tokens before match
	CSphVector<StoredExcerptToken_t> m_dAfterTokens;	///< stored tokens after match

	void Reset ()
	{
		m_iStart = 0;
		m_iTokens = 0;
		m_iCodes = 0;
		m_uQwords = 0;
		m_iQwordsWeight = 0;
		m_iQwordCount = 0;
		m_iUniqQwords = 0;
		m_iMaxLCS = 0;
		m_iMinGap = 0;
		m_iAroundBefore = 0;
		m_iAroundAfter = 0;
		m_iCodesBetweenKeywords = 0;
		m_iWordsBetweenKeywords = 0;
		m_dBeforeTokens.Resize(0);
		m_dAfterTokens.Resize(0);
	}

	void CopyData ( Passage_t & tPassage )
	{
		m_iStart = tPassage.m_iStart;
		m_iTokens = tPassage.m_iTokens;
		m_iCodes = tPassage.m_iCodes;
		m_iWords = tPassage.m_iWords;
		m_uQwords = tPassage.m_uQwords;
		m_iQwordsWeight = tPassage.m_iQwordsWeight;
		m_iQwordCount = tPassage.m_iQwordCount;
		m_iUniqQwords = tPassage.m_iUniqQwords;
		m_iMaxLCS = tPassage.m_iMaxLCS;
		m_iMinGap = tPassage.m_iMinGap;
		m_iStartLimit = tPassage.m_iStartLimit;
		m_iEndLimit = tPassage.m_iEndLimit;
		m_iAroundBefore = tPassage.m_iAroundBefore;
		m_iAroundAfter = tPassage.m_iAroundAfter;
		m_iCodesBetweenKeywords = tPassage.m_iCodesBetweenKeywords;
		m_iWordsBetweenKeywords = tPassage.m_iWordsBetweenKeywords;
		m_dBeforeTokens.SwapData ( tPassage.m_dBeforeTokens );
		m_dAfterTokens.SwapData ( tPassage.m_dAfterTokens );
	}

	inline int GetWeight () const
	{
		return m_iQwordCount + m_iQwordsWeight*m_iMaxLCS + m_iMinGap;
	}
};


inline bool operator < ( const Passage_t & a, const Passage_t & b )
{
	if ( a.m_iUniqQwords==b.m_iUniqQwords )
	{
		int iWeightA = a.GetWeight();
		int iWeightB = b.GetWeight();

		return iWeightA==iWeightB ? a.m_iCodes < b.m_iCodes : iWeightA < iWeightB;
	} else
		return a.m_iUniqQwords < b.m_iUniqQwords;
}


struct PassagePositionOrder_fn
{
	inline bool IsLess ( const Passage_t & a, const Passage_t & b ) const
	{
		return a.m_iStart < b.m_iStart;
	}
};


struct SpanToken_t
{
	DWORD				m_uQwordMask;		///< query words mask
	int					m_iLengthCP;		///< token length in codepoints
	BYTE				m_iWordFlag;		///< token type
};


struct Space_t
{
	int					m_iStartBytes;		///< offset from doc start
	int					m_iLengthBytes;		///< length in bytes
	int					m_iLengthCP;		///< length in codepoints
};


struct TokenSpan_t
{
public:
	int		m_iStart;	///< starting token
	int		m_iWords;	///< number of TOK_WORDS tokens
	int		m_iQwords;	///< number of words matching query
	int		m_iCodes;	///< total length in codepoints (cached)

private:
	int		m_iStartIndex;
	int		m_iNumTokens;
	int		m_iBufferBits;	///< log2(m_dTokens.GetLength())
	DWORD	m_uBufferMask;

	CSphTightVector<SpanToken_t>	m_dTokens;

public:
	void Init ( int nStoredTokens )
	{
		m_iBufferBits = sphLog2 ( nStoredTokens-1 );
		m_uBufferMask = ( 1<<m_iBufferBits )-1;
		m_dTokens.Resize ( 1<<m_iBufferBits );
		Reset ();
	}

	void Reset ()
	{
		m_iStart = -1;
		m_iWords = 0;
		m_iQwords = 0;
		m_iCodes = 0;
		m_iStartIndex = 0;
		m_iNumTokens = 0;
	}

	void ScaleUp()
	{
		// not enough preallocated tokens, realloc
		CSphTightVector<SpanToken_t> dNewTokens;
		dNewTokens.Resize ( m_dTokens.GetLength()*2 );

		for ( int i=0; i<m_iNumTokens; i++ )
			dNewTokens[i] = GetToken(i);
		dNewTokens.SwapData ( m_dTokens );

		m_iBufferBits++;
		m_uBufferMask = ( 1<<m_iBufferBits )-1;

		m_iStartIndex = 0;
	}

	void Add ( int iToken, bool bWordFlag, bool bQWord, int iTermIndex, int iLengthCP )
	{
		bQWord &= bWordFlag;

		if ( m_iNumTokens==m_dTokens.GetLength() )
			ScaleUp();

		int iEndIndex = ( m_iStartIndex + m_iNumTokens ) & m_uBufferMask;
		m_iNumTokens++;

		SpanToken_t & tToken = m_dTokens [ iEndIndex ];
		tToken.m_uQwordMask = ( bQWord && ( iTermIndex>=0 ) ) ? ( 1<<iTermIndex ) : 0;
		tToken.m_iLengthCP = iLengthCP;
		tToken.m_iWordFlag = bWordFlag;

		m_iCodes += iLengthCP;
		m_iWords += bWordFlag;
		m_iQwords += bQWord;

		if ( m_iStart<0 )
			m_iStart = iToken;
	}

	int GetNumTokens () const
	{
		return m_iNumTokens;
	}

	const SpanToken_t & GetToken ( int iToken ) const
	{
		assert ( iToken<=m_iNumTokens );
		int iIndex = ( m_iStartIndex + iToken ) & m_uBufferMask;
		return m_dTokens[iIndex];
	}

	void RemoveStartingTokens ( int nTokens )
	{
		assert ( nTokens<m_iNumTokens );
		m_iStartIndex = ( m_iStartIndex + nTokens ) & m_uBufferMask;
		m_iNumTokens -= nTokens;
	}
};


static void SplitSpaceIntoTokens ( CSphVector<Space_t> & dSpaces, const char * pDoc, int iStart, int iLen, int iBoundary = -1 )
{
	// most frequent case
	if ( sphIsSpace ( pDoc[iStart] ) && iLen==1 )
	{
		dSpaces.Resize(1);
		dSpaces[0].m_iStartBytes = iStart;
		dSpaces[0].m_iLengthBytes = 1;
		dSpaces[0].m_iLengthCP = 1;
		return;
	}

	dSpaces.Resize(0);

	int iGapStart = iStart;
	bool bWasSpace = sphIsSpace ( *(pDoc+iStart) );
	bool bWasBoundary = iBoundary==iStart;

	for ( int i=iStart; i<iStart+iLen; i++ )
	{
		bool bSpace = sphIsSpace ( *(pDoc+i) );
		bool bBoundary = i==iBoundary;
		if ( bSpace!=bWasSpace || bBoundary!=bWasBoundary )
		{
			Space_t & tLastSpace = dSpaces.Add();
			tLastSpace.m_iStartBytes = iGapStart;
			tLastSpace.m_iLengthBytes = i-iGapStart;
			tLastSpace.m_iLengthCP = sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes );

			bWasSpace = bSpace;
			bWasBoundary = bBoundary;
			iGapStart = i;
		}
	}

	if ( iGapStart < iStart+iLen )
	{
		Space_t & tLastSpace = dSpaces.Add();
		tLastSpace.m_iStartBytes = iGapStart;
		tLastSpace.m_iLengthBytes = iStart+iLen-iGapStart;
		tLastSpace.m_iLengthCP = sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes );
	}
}


// passage collecting functor traits
class PassageCollectorTraits_c : public TokenFunctorTraits_c
{
public:
	bool					m_bFixedWeightOrder;
	CSphVector<Passage_t>	m_dPassages;
	CSphVector<BYTE>		m_dStartResult;
	CSphVector<SphHitMark_t>m_dCollectedHits;
	DWORD					m_uFoundWords;

	int						m_iThresh;
	DWORD					m_uPassagesQwords;
	int						m_dQwordWeights[32];
	CSphVector<int>			m_dTopPassageWeights;

protected:
	const SphHitMark_t *	m_pHit;
	const SphHitMark_t *	m_pHitEnd;
	TokenSpan_t				m_tSpan;
	Passage_t				m_tPass;
	int						m_iCurToken;
	bool					m_bCollectionStopped;
	CSphVector<Space_t>		m_dSpaces;
	int						m_iResultLenCP;


	PassageCollectorTraits_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> * dHits )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_bFixedWeightOrder ( m_bWeightOrder )
		, m_uFoundWords	( 0 )
		, m_iThresh ( 0 )
		, m_uPassagesQwords ( 0 )
		, m_pHit ( dHits ? dHits->Begin() : NULL )
		, m_pHitEnd ( dHits ? dHits->Begin()+dHits->GetLength() : NULL )
		, m_iCurToken ( 0 )
		, m_bCollectionStopped ( false )
		, m_iResultLenCP ( 0 )
	{
		const int AVG_WORD_LEN = 5;
		int iSpanSize = m_iLimit ? 8*m_iLimit/AVG_WORD_LEN : 2*m_iLimitWords;
		if ( !iSpanSize )
			iSpanSize = 128;

		m_tSpan.Init ( iSpanSize );
		m_tPass.Reset();
		m_dPassages.Reserve(1024);

		if ( !dHits )
			m_dCollectedHits.Reserve(4096);

		if ( tQuery.m_iLimitPassages>0 )
			m_iThresh = tQuery.m_iLimitPassages;
		else if ( tQuery.m_iLimitWords>0 )
			m_iThresh = tQuery.m_iLimitWords / 2;
		else if ( tQuery.m_iLimit>0 )
			m_iThresh = tQuery.m_iLimit / 4;

		m_iThresh = 1 << sphLog2 ( m_iThresh );

		memset ( m_dQwordWeights, 0, sizeof(m_dQwordWeights) );
	}

	void CollectStartTokens ( int iStart, int iLen, int iLenghCP = -1 )
	{
		if ( m_bAllowEmpty || m_bCollectionStopped )
			return;

		int iCalcLengthCP = iLenghCP;
		if ( iCalcLengthCP==-1 )
			iCalcLengthCP = sphUTF8Len ( m_pDoc+iStart, iLen );

		bool bLengthOk = m_iResultLenCP+iCalcLengthCP<=m_iLimit;
		if ( bLengthOk || !m_dStartResult.GetLength() )
		{
			ResultEmit ( m_dStartResult, m_pDoc+iStart, iLen );
			m_iResultLenCP += iCalcLengthCP;
		}

		if ( !bLengthOk )
		{
			ResultEmit ( m_dStartResult, m_sChunkSeparator.cstr(), m_sChunkSeparator.Length() );
			m_bCollectionStopped = true;
		}
	}

	void CollectStartSpaces ()
	{
		if ( m_bAllowEmpty || m_bCollectionStopped )
			return;

		ARRAY_FOREACH_COND ( i, m_dSpaces, !m_bCollectionStopped )
		{
			Space_t & tSpace = m_dSpaces[i];
			CollectStartTokens ( tSpace.m_iStartBytes, tSpace.m_iLengthBytes, tSpace.m_iLengthCP );
		}
	}

	void CalcPassageWeight ( int iMaxWords )
	{
		DWORD uLast = 0;
		int iLCS = 1;

		m_tPass.m_iMaxLCS = 1;
		m_tPass.m_uQwords = 0;
		m_tPass.m_iMinGap = iMaxWords-1;
		m_tPass.m_iStartLimit = INT_MAX;
		m_tPass.m_iEndLimit = INT_MIN;
		m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;
		m_tPass.m_iQwordCount = 0;
		m_tPass.m_iUniqQwords = 0;
		m_tPass.m_iQwordsWeight = 0;

		int iWord = -1;
		for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken(i);
			if ( !tTok.m_iWordFlag )
				continue;
			iWord++;

			// update mask
			m_tPass.m_uQwords |= tTok.m_uQwordMask;

			// update match boundary
			if ( tTok.m_uQwordMask )
			{
				int iTok = m_tSpan.m_iStart+i;
				m_tPass.m_iStartLimit = Min ( m_tPass.m_iStartLimit, iTok );
				m_tPass.m_iEndLimit = Max ( m_tPass.m_iEndLimit, iTok );
				m_tPass.m_iQwordCount++;
			}

			// update LCS
			uLast = tTok.m_uQwordMask & ( uLast<<1 );
			if ( uLast )
			{
				iLCS++;
				m_tPass.m_iMaxLCS = Max ( iLCS, m_tPass.m_iMaxLCS );
			} else
			{
				iLCS = 1;
				uLast = tTok.m_uQwordMask;
			}

			// update min gap
			if ( tTok.m_uQwordMask )
			{
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, iWord );
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, m_tSpan.m_iWords-1-iWord );
			}

			m_tPass.m_iAroundBefore += ( m_tPass.m_uQwords==0 );
			m_tPass.m_iAroundAfter = (tTok.m_uQwordMask ? 0 : m_tPass.m_iAroundAfter+1 );
		}
		assert ( m_tPass.m_iMinGap>=0 );
		assert ( m_tSpan.m_iWords==iWord+1 );

		// we do it only once because we don't need duplicate weights
		// but m_iQwordCount will still show the total amount of qwords w/dupes
		DWORD uWords = m_tPass.m_uQwords;
		for ( iWord=0; uWords; uWords >>= 1, iWord++ )
			if ( uWords & 1 )
			{
				m_tPass.m_iQwordsWeight += m_tContainer.GetTermWeight(iWord);
				m_tPass.m_iUniqQwords++;
			}

		// total number of words is important too, so lets boost it a bit
		m_tPass.m_iQwordCount *= 2;
	}

	void AppendBeforeAfterTokens ( Passage_t & tPassage, const TokenSpan_t & tSpan )
	{
		// maybe we don't need no extra token info
		if ( ( ( m_iLimit==0 || m_iLimit>=m_iDocLen ) && !m_iLimitWords && m_ePassageSPZ==SPH_SPZ_NONE ) || m_bUseBoundaries )
			return;

		tPassage.m_iCodesBetweenKeywords = tPassage.m_iCodes;
		tPassage.m_iWordsBetweenKeywords = tPassage.m_iWords;

		tPassage.m_dBeforeTokens.Resize(0);
		tPassage.m_dAfterTokens.Resize(0);

		tPassage.m_dBeforeTokens.Reserve ( (m_iAround+1)*2 );
		tPassage.m_dAfterTokens.Reserve ( (m_iAround+1)*2 );

		int iBefore = 0;
		for ( int i = tPassage.m_iStartLimit-tSpan.m_iStart-1; i>=tPassage.m_iStart-m_tSpan.m_iStart; i-- )
		{
			const SpanToken_t & tTok = tSpan.GetToken(i);

			tPassage.m_iCodesBetweenKeywords -= tTok.m_iLengthCP;
			tPassage.m_iWordsBetweenKeywords -= tTok.m_iWordFlag;

			if ( iBefore+tTok.m_iWordFlag<=tPassage.m_iAroundBefore )
			{
				StoredExcerptToken_t & tBeforeToken = tPassage.m_dBeforeTokens.Add();
				tBeforeToken.m_iWordFlag = tTok.m_iWordFlag;
				tBeforeToken.m_iLengthCP = tTok.m_iLengthCP;
				iBefore += tTok.m_iWordFlag;
			} else
				break;
		}

		int iAfter = 0;
		for ( int i = tPassage.m_iEndLimit-tSpan.m_iStart+1; i < tPassage.m_iTokens; i++ )
		{
			const SpanToken_t & tTok = tSpan.GetToken(i);
			BYTE iWordFlag = tSpan.GetToken(i).m_iWordFlag;

			tPassage.m_iCodesBetweenKeywords -= tTok.m_iLengthCP;
			tPassage.m_iWordsBetweenKeywords -= iWordFlag;

			if ( iAfter+iWordFlag<=tPassage.m_iAroundAfter )
			{
				StoredExcerptToken_t & tAfterToken = tPassage.m_dAfterTokens.Add();
				tAfterToken.m_iWordFlag = tTok.m_iWordFlag;
				tAfterToken.m_iLengthCP = tTok.m_iLengthCP;
				iAfter += iWordFlag;
			} else
				break;
		}

		assert ( tPassage.m_iWordsBetweenKeywords>0 && tPassage.m_iCodesBetweenKeywords>0 );
	}

	bool SelectBestPassages ( CSphVector<Passage_t> & dShow )
	{
		assert ( m_dPassages.GetLength() );

		// our limits
		int iMaxPassages = m_iLimitPassages
			? Min ( m_dPassages.GetLength(), m_iLimitPassages )
			: m_dPassages.GetLength();
		int iMaxWords = m_iLimitWords ? m_iLimitWords : INT_MAX;
		int iMaxCp = m_iLimit ? m_iLimit : INT_MAX;

		DWORD uWords = 0; // mask of words in dShow so far
		int iTotalCodes = 0;
		int iTotalWords = 0;
		int iTotalKeywordCodes = 0;
		int iTotalKeywordWords = 0;

		CSphVector<int> dWeights ( m_dPassages.GetLength() );
		ARRAY_FOREACH ( i, m_dPassages )
			dWeights[i] = m_dPassages[i].m_iQwordsWeight;

		// collect enough best passages to show all keywords and max out the limits
		// don't care much if we're going over limits in this loop, it will be tightened below
		bool bAll = false;
		while ( dShow.GetLength() < iMaxPassages )
		{
			// get next best passage
			int iBest = -1;
			ARRAY_FOREACH ( i, m_dPassages )
			{
				if ( m_dPassages[i].m_iCodes && ( iBest==-1 || m_dPassages[iBest] < m_dPassages[i] ) )
					iBest = i;
			}
			if ( iBest<0 )
				break;

			Passage_t & tBest = m_dPassages[iBest];

			// force_all_keywords passage may be very big, so let's allow to show one of them
			if ( !m_bForceAllWords || dShow.GetLength() )
				if ( iTotalKeywordCodes+tBest.m_iCodesBetweenKeywords>iMaxCp || iTotalKeywordWords+tBest.m_iWordsBetweenKeywords>iMaxWords )
					break;

			bool bFits = iTotalCodes+tBest.m_iCodes<=iMaxCp && iTotalWords+tBest.m_iWords<=iMaxWords;
			if ( uWords==m_uFoundWords && !bFits )
			{
				// there might be just enough space to partially display this passage
				if ( iTotalCodes+tBest.m_iCodesBetweenKeywords<=iMaxCp && iTotalWords+tBest.m_iWordsBetweenKeywords<=iMaxWords )
				{
					iTotalWords += tBest.m_iWords;
					iTotalCodes += tBest.m_iCodes;
					dShow.Add ( tBest );
				}
				break;
			}

			// save it, despite limits or whatever, we'll tighten everything in the loop below
			dShow.Add ( tBest );
			uWords |= tBest.m_uQwords;
			iTotalKeywordWords += tBest.m_iWordsBetweenKeywords;
			iTotalKeywordCodes += tBest.m_iCodesBetweenKeywords;
			iTotalWords += tBest.m_iWords;
			iTotalCodes += tBest.m_iCodes;
			tBest.m_iCodes = 0; // no longer needed here, abusing to mark displayed passages

			// we just managed to show all words? do one final re-weighting run
			if ( !bAll && uWords==m_uFoundWords )
			{
				bAll = true;
				ARRAY_FOREACH ( i, m_dPassages )
					m_dPassages[i].m_iQwordsWeight = dWeights[i];
			}

			// if we're already showing all words, re-weighting is not needed any more
			if ( bAll )
				continue;

			// re-weight passages, adjust for new mask of shown words
			// FIXME! re-weighting doesn't change m_iQwordCount (and qwords could possibly be duplicated) and LCS
			ARRAY_FOREACH ( i, m_dPassages )
			{
				if ( !m_dPassages[i].m_iCodes )
					continue;
				DWORD uMask = tBest.m_uQwords;
				for ( int iWord=0; uMask; iWord++, uMask >>= 1 )
					if ( ( uMask & 1 ) && ( m_dPassages[i].m_uQwords & ( 1UL<<iWord ) ) )
					{
						m_dPassages[i].m_iQwordsWeight -= m_tContainer.GetTermWeight(iWord);
						m_dPassages[i].m_iQwordCount--; // doesn't account for dupes
						m_dPassages[i].m_iUniqQwords--;
					}

				m_dPassages[i].m_uQwords &= ~uWords;
			}
		}

		// if all passages won't fit into the limit, try to trim them a bit
		//
		// this step is skipped when use_boundaries is enabled, because
		// each passage must be a separate sentence (delimited by
		// boundaries) and we don't want to split them
		if ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_bUseBoundaries )
		{
			// trim passages
			bool bFirst = true;
			bool bDone = false;
			int iCodes = iTotalCodes;
			while ( !bDone )
			{
				// drop one token from each passage starting from the least relevant
				for ( int i=dShow.GetLength(); i > 0; i-- )
				{
					Passage_t & tPassage = dShow[i-1];

					if ( !tPassage.m_dBeforeTokens.GetLength() && !tPassage.m_dAfterTokens.GetLength() )
						continue;

					bool bDropFirst;
					if ( tPassage.m_dBeforeTokens.GetLength() > tPassage.m_dAfterTokens.GetLength() )
						bDropFirst = true;
					else if ( tPassage.m_dBeforeTokens.GetLength() < tPassage.m_dAfterTokens.GetLength() )
						bDropFirst = false;
					else if ( !tPassage.m_dBeforeTokens.Last().m_iWordFlag && tPassage.m_dAfterTokens.Last().m_iWordFlag )
						bDropFirst = true;
					else if ( tPassage.m_dBeforeTokens.Last().m_iWordFlag && !tPassage.m_dAfterTokens.Last().m_iWordFlag )
						bDropFirst = false;
					else
						bDropFirst = bFirst;

					if ( bDropFirst )
					{
						// drop first
						const StoredExcerptToken_t & tTok = tPassage.m_dBeforeTokens.Pop();
						tPassage.m_iStart++;
						tPassage.m_iTokens--;
						tPassage.m_iCodes -= tTok.m_iLengthCP;
						iTotalCodes -= tTok.m_iLengthCP;
						iTotalWords -= tTok.m_iWordFlag;

					} else
					{
						// drop last
						const StoredExcerptToken_t & tTok = tPassage.m_dAfterTokens.Pop();
						tPassage.m_iTokens--;
						tPassage.m_iCodes -= tTok.m_iLengthCP;
						iTotalCodes -= tTok.m_iLengthCP;
						iTotalWords -= tTok.m_iWordFlag;
					}

					if ( iTotalCodes<=iMaxCp && iTotalWords<=iMaxWords )
					{
						bDone = true;
						break;
					}
				}
				if ( iTotalCodes==iCodes )
					break; // couldn't reduce anything
				iCodes = iTotalCodes;
				bFirst = !bFirst;
			}
		}

		// if passages still don't fit start dropping least significant ones, limit is sacred.
		while ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_bForceAllWords )
		{
			iTotalCodes -= dShow.Last().m_iCodes;
			iTotalWords -= dShow.Last().m_iWords;
			dShow.RemoveFast ( dShow.GetLength()-1 );
		}

		if ( !dShow.GetLength() )
			return false;

		bool bPreSorted = false;
		if ( m_bWeightOrder )
		{
			// maybe the weight order is the same as position order?
			bool bPositionOrder = true;
			for ( int i = 1; i<dShow.GetLength() && bPositionOrder; i++ )
				if ( dShow[i-1].m_iStart>dShow[i].m_iStart )
					bPositionOrder = false;

			bPreSorted = bPositionOrder;

			if ( bPositionOrder )
				m_bFixedWeightOrder = false;
		}

		// sort passages in the document order
		if ( !bPreSorted )
			dShow.Sort ( PassagePositionOrder_fn() );

		return true;
	}

	void SubmitPassages()
	{
		if ( !m_dPassages.GetLength() )
			return;

		CSphVector<Passage_t> dPassagesToShow;
		if ( SelectBestPassages ( dPassagesToShow ) )
		{
			ARRAY_FOREACH ( i, dPassagesToShow )
			{
				dPassagesToShow[i].m_dBeforeTokens.Reset();
				dPassagesToShow[i].m_dAfterTokens.Reset();
			}
			m_dPassages = dPassagesToShow;

		} else
			m_dPassages.Resize(0);
	}
};


/// functor that highlights the start of the document
class DocStartHighlighter_c : public PassageCollectorTraits_c
{
public:
	DocStartHighlighter_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen )
		: PassageCollectorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen, NULL )
	{
		assert ( !m_bAllowEmpty );
	}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & )
	{
		CollectStartTokens ( tTok.m_iStart, tTok.m_iLen );
		return !m_bCollectionStopped;
	}

	bool OnOverlap ( int iStart, int iLen, int )
	{
		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen );
		CollectStartSpaces ();
		return !m_bCollectionStopped;
	}

	void OnTail ( int iStart, int iLen, int )
	{
		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen );
		CollectStartSpaces ();
	}

	void OnSkipHtml ( int, int ) {}
	void OnSPZ ( BYTE, DWORD, const char *, int ) {}
	void OnFinish () {}
	const CSphVector<int> * GetHitlist ( const XQKeyword_t & ) const { return NULL; }
};


/// functor that extracts passages for further highlighting
class ExtractExcerpts_c : public PassageCollectorTraits_c
{
public:
	ExtractExcerpts_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> * dHits,
		CacheStreamer_c * pTokenContainer )
		: PassageCollectorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen, dHits )
		, m_eState		( STATE_WINDOW_SETUP )
		, m_bQwordsChanged ( true )
		, m_bAppendSentenceEnd ( false )
		, m_pTokenContainer ( pTokenContainer )
	{
	}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens )
	{
		assert ( m_pDoc );
		assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

		bool bQWord = false;
		int iTermIndex = -1;
		if ( m_pHit )
		{
			// this means we've collected hits beforehand
			while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=tTok.m_uPosition )
				m_pHit++;

			if ( m_pHit<m_pHitEnd && tTok.m_uPosition>=m_pHit->m_uPosition && tTok.m_uPosition<=m_pHit->m_uPosition+m_pHit->m_uSpan )
				bQWord = true;

			iTermIndex = tTok.m_iTermIndex;
		} else
		{
			// no collected hits beforehand
			iTermIndex = m_tContainer.FindWord ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen );
			ARRAY_FOREACH_COND ( i, dTokens, iTermIndex==-1 )
				iTermIndex = m_tContainer.FindWord ( dTokens[i], NULL, 0 );

			bQWord = iTermIndex!=-1;
			m_uFoundWords |= iTermIndex==-1 ? 0 : 1 << iTermIndex;

			// collect hits now
			if ( bQWord )
			{
				SphHitMark_t & tHit = m_dCollectedHits.Add();
				tHit.m_uPosition = tTok.m_uPosition;
				tHit.m_uSpan = 1;
			}
		}

		int iLengthCP = sphUTF8Len ( m_pDoc+tTok.m_iStart, tTok.m_iLen );

		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			{
				DWORD uAllQwords = ( 1 << m_tContainer.GetNumTerms() )-1;
				const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;
				bool bLimitsOk = m_tSpan.m_iCodes+iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetWordsLimit ( *this, m_tSpan.m_iQwords );

				if ( ( m_bForceAllWords && m_tPass.m_uQwords==uAllQwords && !bLimitsOk ) ||
					( !m_bForceAllWords && !bLimitsOk ) )
				{
					m_bQwordsChanged = true;
					WeightAndSubmit ();
					m_eState = STATE_ADD_WORD;
				}

				m_tSpan.Add ( m_iCurToken, tTok.m_bWord || tTok.m_bStopWord, bQWord, iTermIndex, iLengthCP );
				m_bQwordsChanged |= bQWord;

				if ( m_eState==STATE_ADD_WORD && (tTok.m_bWord || tTok.m_bStopWord) )
				{
					m_bQwordsChanged |= bQWord;
					ShrinkSpanHead();
					WeightAndSubmit();
				}
			}
			break;

		case STATE_ADD_WORD:
			m_tSpan.Add ( m_iCurToken, tTok.m_bWord || tTok.m_bStopWord, bQWord, iTermIndex, iLengthCP );

			if ( tTok.m_bWord || tTok.m_bStopWord )
			{
				m_bQwordsChanged |= bQWord;
				ShrinkSpanHead();
				WeightAndSubmit();
			}
			break;
		}

		m_iCurToken++;

		if ( m_pTokenContainer )
			m_pTokenContainer->StoreToken ( tTok );
		return true;
	}

	void OnFinish ()
	{
		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			WeightAndSubmit();
			break;
		case STATE_ADD_WORD:
			break;
		}
		SubmitPassages();
	}

	bool OnOverlap ( int iStart, int iLen, int iBoundary )
	{
		// most frequent case
		if ( sphIsSpace ( m_pDoc[iStart] ) && iLen==1 && iBoundary<0 && m_eState==STATE_ADD_WORD )
		{
			m_tSpan.Add ( m_iCurToken++, false, false, -1, 1 );
			if ( m_bAppendSentenceEnd )
				OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
		} else
		{
			SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
			AddSpaces ( iBoundary );
		}

		if ( m_pTokenContainer )
			m_pTokenContainer->StoreOverlap ( iStart, iLen, iBoundary );
		return true;
	}

	void OnSkipHtml ( int iStart, int iLen )
	{
		if ( m_pTokenContainer )
			m_pTokenContainer->StoreSkipHtml ( iStart, iLen );
	}

	void OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * szZone, int iZone )
	{
		if ( m_bAppendSentenceEnd )
			m_bAppendSentenceEnd = false;
		else if ( iSPZ==MAGIC_CODE_SENTENCE )
		{
			m_bAppendSentenceEnd = true;
			return;
		}

		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			WeightAndSubmit();
			break;

		case STATE_ADD_WORD:
			m_bQwordsChanged = true;
			WeightAndSubmit();
			m_eState = STATE_WINDOW_SETUP;
			break;
		}

		m_bQwordsChanged = true;
		m_tSpan.Reset();

		if ( m_pTokenContainer )
			m_pTokenContainer->StoreSPZ ( iSPZ, uPosition, szZone, iZone );
	}

	void OnTail ( int iStart, int iLen, int iBoundary )
	{
		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
		AddSpaces ( iBoundary );
		ShrinkSpanHead();
		WeightAndSubmit();

		if ( m_pTokenContainer )
			m_pTokenContainer->StoreTail ( iStart, iLen, iBoundary );
	}

	const CSphVector<int> * GetHitlist ( const XQKeyword_t & ) const
	{
		return NULL;
	}

private:
	enum State_e
	{
		STATE_WINDOW_SETUP,
		STATE_ADD_WORD
	};

	State_e				m_eState;
	bool				m_bQwordsChanged;
	bool				m_bAppendSentenceEnd;
	CacheStreamer_c *	m_pTokenContainer;


	void AddSpaces ( int iBoundary )
	{
		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			{
				DWORD uAllQwords = ( 1 << m_tContainer.GetNumTerms() )-1;
				const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;
				ARRAY_FOREACH ( i, m_dSpaces )
				{
					bool bBoundary = iBoundary==m_dSpaces[i].m_iStartBytes;
					bool bLimitsOk = m_tSpan.m_iCodes+m_dSpaces[i].m_iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetWordsLimit ( *this, m_tSpan.m_iQwords );

					if ( m_bForceAllWords && m_tPass.m_uQwords!=uAllQwords )
						bLimitsOk = true;

					if ( ( bBoundary || !bLimitsOk ) && m_eState!=STATE_ADD_WORD )
					{
						if ( bBoundary )
						{
							m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );
							WeightAndSubmit();
							m_tSpan.Reset();

						} else
						{
							WeightAndSubmit();
							m_eState = STATE_ADD_WORD;
						}
					}

					if ( !bBoundary )
						m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );

					if ( m_bAppendSentenceEnd )
						OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
				}
			}
			break;

		case STATE_ADD_WORD:
			ARRAY_FOREACH ( i, m_dSpaces )
			{
				m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );
				if ( iBoundary==m_dSpaces[i].m_iStartBytes )
				{
					WeightAndSubmit ();
					m_tSpan.Reset();
					m_eState = STATE_WINDOW_SETUP;
				}

				if ( !i && m_bAppendSentenceEnd )
					OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
			}
			break;
		}
	}

	void WeightAndSubmit ()
	{
		if ( m_tSpan.m_iQwords )
		{
			if ( m_bQwordsChanged )
			{
				CalcPassageWeight ( GetWordsLimit ( *this, m_tSpan.m_iQwords ) );
				m_bQwordsChanged = false;
			} else
				UpdateGaps ( GetWordsLimit ( *this, m_tSpan.m_iQwords ) );

			if ( m_tPass.m_uQwords )
				FlushPassage();
		}
	}

	void UpdateGaps ( int iMaxWords )
	{
		m_tPass.m_iMinGap = iMaxWords-1;
		m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;
		DWORD uQwords = 0;
		int iWord = -1;
		for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken(i);
			if ( !tTok.m_iWordFlag )
				continue;

			iWord++;
			if ( tTok.m_uQwordMask )
			{
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, iWord );
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, m_tSpan.m_iWords-1-iWord );
			}

			uQwords |= tTok.m_uQwordMask;
			m_tPass.m_iAroundBefore += ( uQwords==0 );
			m_tPass.m_iAroundAfter = ( tTok.m_uQwordMask ? 0 : m_tPass.m_iAroundAfter+1 );
		}
		assert ( m_tPass.m_iMinGap>=0 );
	}

	void FlushPassage ()
	{
		m_tPass.m_iStart = m_tSpan.m_iStart;
		m_tPass.m_iTokens = m_tSpan.GetNumTokens();
		m_tPass.m_iCodes = m_tSpan.m_iCodes;
		m_tPass.m_iWords = m_tSpan.m_iWords;

		int iBefore = 0;
		while ( m_tPass.m_iAroundBefore>m_iAround )
		{
			assert ( m_tPass.m_iStart<m_tPass.m_iStartLimit );
			const SpanToken_t & tTok = m_tSpan.GetToken ( iBefore );
			assert ( tTok.m_uQwordMask==0 );

			m_tPass.m_iCodes -= tTok.m_iLengthCP;
			m_tPass.m_iAroundBefore -= tTok.m_iWordFlag;
			m_tPass.m_iStart++;
			m_tPass.m_iTokens--;
			m_tPass.m_iWords -= tTok.m_iWordFlag;
			iBefore++;
		}

		int iAfter = m_tSpan.GetNumTokens()-1;
		while ( m_tPass.m_iAroundAfter>m_iAround )
		{
			assert ( m_tPass.m_iEndLimit<m_tPass.m_iStart+m_tPass.m_iTokens-1 );
			const SpanToken_t & tTok = m_tSpan.GetToken ( iAfter );
			assert ( tTok.m_uQwordMask==0 );

			m_tPass.m_iCodes -= tTok.m_iLengthCP;
			m_tPass.m_iAroundAfter -= tTok.m_iWordFlag;
			m_tPass.m_iTokens--;
			m_tPass.m_iWords -= tTok.m_iWordFlag;
			iAfter--;
		}

		// if it's the very first one, do add
		if ( !m_dPassages.GetLength() )
		{
			Passage_t & tPassage = m_dPassages.Add ();
			tPassage.CopyData ( m_tPass );
			AppendBeforeAfterTokens ( tPassage, m_tSpan );
			UpdateTopPassages ( m_tPass.m_iQwordCount==1 ? sphLog2 ( m_tPass.m_uQwords )-1 : -1, m_tPass.GetWeight() );
			return;
		}

		// check if it's new or better than the last one
		Passage_t & tLast = m_dPassages.Last();
		if ( ( m_tPass.m_iStartLimit<=tLast.m_iStartLimit && tLast.m_iEndLimit<=m_tPass.m_iEndLimit )
			|| ( tLast.m_iStartLimit<=m_tPass.m_iStartLimit && m_tPass.m_iEndLimit<=tLast.m_iEndLimit ) )
		{
			// overlapping passages, check which one is better centered
			int iPassPre = m_tPass.m_iStartLimit - m_tPass.m_iStart + 1;
			int iPassPost = m_tPass.m_iStart + m_tPass.m_iTokens - m_tPass.m_iEndLimit + 1;
			float fPassGap = (float)Max ( iPassPre, iPassPost ) / (float)Min ( iPassPre, iPassPost );

			int iLastPre = tLast.m_iStartLimit - tLast.m_iStart + 1;
			int iLastPost = tLast.m_iStart + tLast.m_iTokens - tLast.m_iEndLimit + 1;
			float fLastGap = (float)Max ( iLastPre, iLastPost ) / (float)Min ( iLastPre, iLastPost );

			int iWeightLast = tLast.GetWeight();
			int iWeightPass = m_tPass.GetWeight();
			// centered snippet wins last passage
			if ( tLast.m_iUniqQwords<=m_tPass.m_iUniqQwords &&
				( iWeightLast<iWeightPass || ( iWeightLast==iWeightPass && fPassGap<fLastGap ) ) )
			{
				tLast.CopyData ( m_tPass );
				AppendBeforeAfterTokens ( tLast, m_tSpan );
			}
			return;
		}

		// after a certain threshold, start being picky
		// only accept passages with new keywords, or big enough weight
		int iWeight = m_tPass.GetWeight();
		int iQword = -1;
		if ( m_tPass.m_iQwordCount==1 )
			iQword = sphLog2 ( m_tPass.m_uQwords )-1;

		while ( m_dPassages.GetLength()>m_iThresh )
		{
			// completely new keyword? accept
			if ( m_tPass.m_uQwords & ~m_uPassagesQwords )
				break;

			// single keyword passage? accept if better weight, otherwise reject
			if ( iQword>=0 )
			{
				if ( iWeight<=m_dQwordWeights[iQword] )
					return;
				break;
			}

			// multi-keyword passage? accept if weight within top-N
			assert ( iQword<0 );
			if ( iWeight<=m_dTopPassageWeights[m_iThresh] )
				return;
			break;
		}

		// kill them all, god will know his own
		Passage_t & tPassage = m_dPassages.Add();
		tPassage.CopyData ( m_tPass );
		AppendBeforeAfterTokens ( tPassage, m_tSpan );
		UpdateTopPassages ( iQword, iWeight );
	}

	void UpdateTopPassages ( int iQword, int iWeight )
	{
		m_uPassagesQwords |= m_tPass.m_uQwords;
		if ( iQword>=0 )
			m_dQwordWeights[iQword] = Max ( m_dQwordWeights[iQword], iWeight );
		m_dTopPassageWeights.Add ( iWeight );
		if ( ( m_dTopPassageWeights.GetLength() & ( m_iThresh-1 ) )==0 )
			m_dTopPassageWeights.RSort();
	}

	void ShrinkSpanHead ()
	{
		const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;

		int iTokenStart = 0;
		const int iMaxToken = m_tSpan.GetNumTokens() - 1;

		// drop front tokens until the window fits into both word and CP limits
		while ( iTokenStart < iMaxToken
			&& ( m_tSpan.m_iCodes > iCpLimit || m_tSpan.m_iWords > GetWordsLimit ( *this, m_tSpan.m_iQwords ) ) )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken ( iTokenStart );
			if ( tTok.m_uQwordMask )
			{
				m_tSpan.m_iQwords--; // FIXME? might not be true if we remove a duped keyword
				m_bQwordsChanged = true;
			}
			m_tSpan.m_iWords -= tTok.m_iWordFlag;
			m_tSpan.m_iCodes -= tTok.m_iLengthCP;
			iTokenStart++;
		}
		m_tSpan.m_iStart += iTokenStart;

		// remove extra tokens
		if ( iTokenStart>=m_tSpan.GetNumTokens() )
			m_tSpan.Reset();
		else if ( iTokenStart>0 )
			m_tSpan.RemoveStartingTokens ( iTokenStart );
	}
};


/// functor that highlights selected passages
class HighlightPassages_c : public HighlightPlain_c
{
public:
	CSphVector<int> m_dPassageHeads;

	HighlightPassages_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen,
		const CSphVector<SphHitMark_t> * dHits, const CSphVector<Passage_t> & dPassages, const FunctorZoneInfo_t * pZoneInfo )
		: HighlightPlain_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen, EstimateResult ( tQuery, iDocLen ) )
		, m_pHit ( dHits ? dHits->Begin() : NULL )
		, m_pHitEnd ( dHits ? dHits->Begin()+dHits->GetLength() : NULL )
		, m_dPassages ( dPassages )
		, m_iCurToken ( 0 )
		, m_iCurPassage	( -1 )
		, m_iSeparatorLen ( m_sChunkSeparator.Length() )
		, m_bLastWasSeparator ( false )
		, m_pZoneInfo ( pZoneInfo )
		, m_iOpenTill ( 0 )
		, m_iLastPos ( 0 )
	{
		if ( m_bWeightOrder )
			m_dPassageHeads.Reserve(1024);
	}

	bool OnToken ( TokenInfo_t & tTok, const CSphVector<SphWordID_t> & )
	{
		assert ( m_pDoc );
		assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

		CheckClose ( tTok.m_uPosition );
		UpdatePassage ( tTok.m_iStart );

		if ( m_iCurPassage!=-1 )
		{
			m_bLastWasSeparator = false;
			while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=tTok.m_uPosition )
				m_pHit++;

			if ( m_bExactPhrase )
			{
				// marker folding, emit "before" marker at span start only
				if ( m_pHit<m_pHitEnd && tTok.m_uPosition==m_pHit->m_uPosition )
					ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro, m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );

				// emit token itself
				ResultEmit ( m_pDoc+tTok.m_iStart, tTok.m_iLen );

				// marker folding, emit "after" marker at span end only
				if ( m_pHit<m_pHitEnd && tTok.m_uPosition==m_pHit->m_uPosition+m_pHit->m_uSpan-1 )
					ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );

			} else
			{
				bool bHit = m_pHit<m_pHitEnd && tTok.m_uPosition>=m_pHit->m_uPosition && tTok.m_uPosition<=m_pHit->m_uPosition+m_pHit->m_uSpan-1;
				if ( bHit && !m_iOpenTill )
				{
					ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro, m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
					m_iOpenTill = m_pHit->m_uPosition+m_pHit->m_uSpan;
				}

				// emit token itself
				ResultEmit ( m_pDoc+tTok.m_iStart, tTok.m_iLen );
			}
		}

		m_iLastPos = tTok.m_uPosition;
		m_iCurToken++;
		return true;
	}

	bool OnOverlap ( int iStart, int iLen, int iBoundary )
	{
		CheckClose ( m_iLastPos+1 );
		EmitSpaces ( iStart, iLen, iBoundary );
		return true;
	}

	void OnTail ( int iStart, int iLen, int iBoundary )
	{
		CheckClose ( m_iLastPos+1 );
		EmitSpaces ( iStart, iLen, iBoundary );
	}

	void OnFinish()
	{
		if ( !m_iOpenTill )
			return;
		ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	}

private:
	const SphHitMark_t * m_pHit;
	const SphHitMark_t * m_pHitEnd;
	const CSphVector<Passage_t> &	m_dPassages;
	int								m_iCurToken;
	int								m_iCurPassage;
	int								m_iSeparatorLen;
	bool							m_bLastWasSeparator;
	TokenSpan_t						m_tTmpSpan;
	CSphVector<Space_t>				m_dSpaces;
	const FunctorZoneInfo_t *		m_pZoneInfo;
	int								m_iOpenTill;
	int								m_iLastPos;

	void EmitZoneName ( int iStart ) const
	{
		if ( !m_bEmitZones || !m_pZoneInfo || !m_pZoneInfo->m_dZonePos.GetLength() )
			return;

		int iZone = FindSpan ( m_pZoneInfo->m_dZonePos, iStart );
		if ( iZone!=-1 )
		{
			int iParent = m_pZoneInfo->m_dZoneParent[iZone];
			m_pZoneInfo->m_hZones.IterateStart();
			while ( m_pZoneInfo->m_hZones.IterateNext() )
			{
				if ( m_pZoneInfo->m_hZones.IterateGet()!=iParent )
					continue;

				ResultEmit ( "<", 1 );
				ResultEmit ( m_pZoneInfo->m_hZones.IterateGetKey().cstr(), m_pZoneInfo->m_hZones.IterateGetKey().Length() );
				ResultEmit ( ">", 1 );
				break;
			}
		}
	}


	void EmitSpaces ( int iStart, int iLen, int iBoundary )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );

		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
		ARRAY_FOREACH ( i, m_dSpaces )
		{
			UpdatePassage ( iStart );

			if ( m_iCurPassage!=-1 )
				ResultEmit ( m_pDoc+m_dSpaces[i].m_iStartBytes, m_dSpaces[i].m_iLengthBytes );

			m_iCurToken++;
		}
	}

	void UpdatePassage ( int iStart )
	{
		const Passage_t * pPassage = m_dPassages.Begin() + m_iCurPassage;
		const Passage_t * pNext = m_dPassages.Begin() + m_iCurPassage + 1;
		if ( !m_bLastWasSeparator && m_iCurPassage!=-1 && !m_bWeightOrder
			&& m_iCurToken==( pPassage->m_iStart + pPassage->m_iTokens )
			&& !( ( m_iCurPassage+1 )<m_dPassages.GetLength() && m_iCurToken>=pNext->m_iStart
				&& m_iCurToken<( pNext->m_iStart + pNext->m_iTokens ) ) )
		{
			ResultEmit ( m_sChunkSeparator.cstr(), m_iSeparatorLen );
			m_bLastWasSeparator = true;
		}

		int iPassage = m_iCurPassage;
		if ( m_iCurPassage==-1 || m_iCurToken<pPassage->m_iStart || m_iCurToken>( pPassage->m_iStart + pPassage->m_iTokens - 1 ) )
		{
			int iNextPassage = 0;
			if ( m_iCurPassage!=-1 && m_iCurToken>( pPassage->m_iStart + pPassage->m_iTokens - 1 ) )
				iNextPassage = m_iCurPassage+1;

			m_iCurPassage = -1;
			for ( int i=iNextPassage; i<m_dPassages.GetLength(); i++ )
				if ( m_iCurToken>=m_dPassages[i].m_iStart && m_iCurToken<=( m_dPassages[i].m_iStart + m_dPassages[i].m_iTokens - 1 ) )
				{
					m_iCurPassage = i;
					break;
				}
		}

		if ( m_iCurPassage!=-1 && iPassage!=m_iCurPassage )
		{
			if ( !m_bLastWasSeparator && m_iCurToken && !m_bWeightOrder )
			{
				ResultEmit ( m_sChunkSeparator.cstr(), m_iSeparatorLen );
				m_bLastWasSeparator = true;
			}

			if ( m_bWeightOrder )
				m_dPassageHeads.Add ( m_dResult.GetLength() );

			EmitZoneName ( iStart );
		}
	}

	void CheckClose ( int iPos )
	{
		// marker folding, emit "after" marker at span end only
		if ( !m_iOpenTill || iPos<m_iOpenTill )
			return;

		ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
		m_iOpenTill = 0;
	}
};


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


static int AddZone ( const char * pStart, const char * pEnd, int uPosition, TokenFunctorTraits_c & tFunctor
						, CSphVector<int> & dZoneStack, CSphVector<char> & dZoneName, const char * pBuf )
{
	CSphVector<ZonePacked_t> & dZones = tFunctor.m_dZones;
	SmallStringHash_T<int> & hZones = tFunctor.m_tZoneInfo.m_hZones;
	CSphVector<int>	& dZonePos = tFunctor.m_tZoneInfo.m_dZonePos;
	CSphVector<int>	& dZoneParent = tFunctor.m_tZoneInfo.m_dZoneParent;
	bool bNeedExtraZoneInfo = tFunctor.m_bCollectExtraZoneInfo;

	int iZone;

	// span's management
	if ( *pStart!='/' )	// open zone
	{
		// zone stack management
		int iSelf = dZones.GetLength();
		dZoneStack.Add ( iSelf );

		// add zone itself
		int iZoneNameLen = pEnd-pStart-1;
		CopyZoneName ( dZoneName, pStart, iZoneNameLen );

		iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		dZones.Add ( sphPackZone ( uPosition, iSelf, iZone ) );

		if ( bNeedExtraZoneInfo )
		{
			// the parent for the open zone is the zone itself
			dZoneParent.Add ( iZone );

			// zone position in characters
			dZonePos.Add ( pStart-pBuf );
		}

#ifndef NDEBUG
		if ( !bNeedExtraZoneInfo )
			dZonePos.Add ( pStart-pBuf );
#endif

	} else					// close zone
	{
#ifndef NDEBUG
		// lets check open - close tags match
		assert ( dZoneStack.GetLength() && dZoneStack.Last()<dZones.GetLength() );
		int iOpening = dZonePos [ dZoneStack.Last() ];
		assert ( iOpening<pEnd-pBuf && strncmp ( pBuf+iOpening, pStart+1, pEnd-pStart-2 )==0 );
#endif

		int iZoneNameLen = pEnd-pStart-2;
		CopyZoneName ( dZoneName, pStart+1, iZoneNameLen );

		iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		int iOpen = dZoneStack.Last();
		int iClose = dZones.GetLength();
		uint64_t uOpenPacked = dZones[ iOpen ];
		DWORD uOpenPos = (DWORD)( ( uOpenPacked>>32 ) & UINT32_MASK );
		assert ( iZone==(int)( uOpenPacked & UINT16_MASK ) ); // check for zone's types match;

		dZones[iOpen] = sphPackZone ( uOpenPos, iClose, iZone );
		dZones.Add ( sphPackZone ( uPosition, iOpen, iZone ) );

		if ( bNeedExtraZoneInfo )
		{
			// zone position in characters
			dZonePos.Add ( pStart-pBuf );

			// the parent for the closing zone is the previous zone on stack
			int iParentZone = dZoneStack.GetLength()>2 ? dZoneStack[dZoneStack.GetLength()-2] : 0;
			uint64_t uParentPacked = dZones.GetLength() && iParentZone<dZones.GetLength() ? dZones[iParentZone] : 0;
			dZoneParent.Add ( (int)( uParentPacked & UINT16_MASK ) );
		}

#ifndef NDEBUG
		if ( !bNeedExtraZoneInfo )
			dZonePos.Add ( pStart-pBuf );
#endif


		// pop up current zone from zone's stack
		dZoneStack.Resize ( dZoneStack.GetLength()-1 );
	}

	return iZone;
}


static void CopyString ( BYTE * sDst, const BYTE * sSrc, int iLen )
{
	const int MAX_WORD_BYTES = 3*SPH_MAX_WORD_LEN;
	int iBackup = ( iLen > MAX_WORD_BYTES ) ? MAX_WORD_BYTES : iLen;
	int iBackup2 = ( iBackup+3 )>>2;
	DWORD * d = (DWORD*)sDst;
	DWORD * s = (DWORD*)sSrc;
	while ( iBackup2-->0 )
		*d++ = *s++;
	sDst[iBackup] = '\0';
}


/// tokenize document using a given functor
template < typename T >
static void TokenizeDocument ( T & tFunctor, const CSphHTMLStripper * pStripper, DWORD iSPZ )
{
	ISphTokenizer * pTokenizer = tFunctor.m_pTokenizer;
	CSphDict * pDict = tFunctor.m_pDict;

	const char * pStartPtr = pTokenizer->GetBufferPtr ();
	const char * pLastTokenEnd = pStartPtr;
	const char * pBufferEnd = pTokenizer->GetBufferEnd();
	assert ( pStartPtr && pLastTokenEnd );

	BYTE sNonStemmed [ 3*SPH_MAX_WORD_LEN+4];

	TokenInfo_t tTok;
	tTok.m_iStart = 0;
	tTok.m_uPosition = 0;
	tTok.m_sWord = sNonStemmed;
	tTok.m_bStopWord = false;
	tTok.m_iTermIndex = -1;

	bool bRetainHtml = tFunctor.m_sStripMode=="retain";
	BYTE * sWord = NULL;
	DWORD uPosition = 0;
	DWORD uStep = 1;
	const char * pBlendedStart = NULL;
	const char * pBlendedEnd = NULL;
	bool bBlendedHead = false;
	bool bBlendedPart = false;
	CSphVector<SphWordID_t> dMultiToken;

	CSphVector<int> dZoneStack;
	CSphVector<char> dZoneName ( 16+3*SPH_MAX_WORD_LEN );

	// FIXME!!! replace by query SPZ extraction pass
	if ( !iSPZ && ( bRetainHtml && tFunctor.m_bHighlightQuery ) )
		iSPZ = MAGIC_CODE_ZONE;

	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		const char * pTokenStart = pTokenizer->GetTokenStart ();

		if ( pBlendedEnd<pTokenStart )
		{
			// FIXME!!! implement proper handling of blend-chars
			if ( pLastTokenEnd<pBlendedEnd && bBlendedPart )
			{
				tTok.m_uWordId = 0;
				tTok.m_bStopWord = false;
				tTok.m_uPosition = uPosition; // let's stick to last blended part
				tTok.m_iStart = pLastTokenEnd - pStartPtr;
				tTok.m_iLen = pBlendedEnd - pLastTokenEnd;
				tTok.m_bWord = false;
				if ( !tFunctor.OnToken ( tTok, dMultiToken ) )
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
			uPosition += tFunctor.m_iBoundaryStep;
		if ( pTokenizer->TokenIsBlended() )
			uStep = 0;

		// handle only blended parts
		if ( pTokenizer->TokenIsBlended() )
		{
			if ( tFunctor.m_bIndexExactWords && pTokenizer->GetTokenMorph()!=SPH_TOKEN_MORPH_GUESS )
			{
				BYTE sTmpBuf [ 3*SPH_MAX_WORD_LEN+4];
				sTmpBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
				CopyString ( sTmpBuf+1, sWord, pTokenizer->GetTokenEnd() - pTokenStart );
				dMultiToken.Add ( tFunctor.m_pDict->GetWordIDNonStemmed ( sTmpBuf ) );
			}

			// must be last because it can change (stem) sWord
			dMultiToken.Add ( tFunctor.m_pDict->GetWordID ( sWord ) );
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
					bDone = !tFunctor.OnOverlap ( pLastTokenEnd-pStartPtr, pBlendedStart - pLastTokenEnd, pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

				tTok.m_uWordId = 0;
				tTok.m_bStopWord = false;
				tTok.m_uPosition = uPosition; // let's stick to 1st blended part
				tTok.m_iStart = pBlendedStart - pStartPtr;
				tTok.m_iLen = pTokenStart - pBlendedStart;
				tTok.m_bWord = false;
				if ( !bDone )
					bDone = !tFunctor.OnToken ( tTok, dMultiToken );
			} else
			{
				bDone = !tFunctor.OnOverlap ( pLastTokenEnd-pStartPtr, pTokenStart - pLastTokenEnd, pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );
			}

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
			const CSphHTMLStripper::StripperTag_t * pTag = NULL;
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
				tFunctor.OnSkipHtml ( pTokenStart-pStartPtr, iTagEnd+1 );
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
		int iWordLen = pLastTokenEnd - pTokenStart;

		bool bPopExactMulti = false;
		if ( tFunctor.m_bIndexExactWords )
		{
			BYTE sTmpBuf [ 3*SPH_MAX_WORD_LEN+4];
			sTmpBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			CopyString ( sTmpBuf+1, sWord, iWordLen );
			dMultiToken.Add ( tFunctor.m_pDict->GetWordIDNonStemmed ( sTmpBuf ) );
			bPopExactMulti = true;
		}

		// must be last because it can change (stem) sWord
		CopyString ( sNonStemmed, sWord, iWordLen );
		SphWordID_t iWord = tFunctor.m_pDict->GetWordID ( sWord );
		tTok.m_uWordId = iWord;

		tTok.m_bStopWord = false;
		if ( !iWord )
			tTok.m_bStopWord = pDict->IsStopWord ( sWord );

		// compute position
		if ( !iWord || tTok.m_bStopWord )
			uStep = tFunctor.m_iStopwordStep;
		else
			uStep = 1;

		tTok.m_uPosition = ( iWord || tTok.m_bStopWord ) ? uPosition : 0;
		tTok.m_iStart = pTokenStart - pStartPtr;
		tTok.m_iLen = iWordLen;
		tTok.m_bWord = !!iWord;

		// match & emit
		// star match needs non-stemmed word
		if ( !tFunctor.OnToken ( tTok, dMultiToken ) )
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
		tTok.m_iStart = pLastTokenEnd - pStartPtr;
		tTok.m_iLen = pBlendedEnd - pLastTokenEnd;
		tTok.m_bWord = false;
		tFunctor.OnToken ( tTok, dMultiToken );
		pLastTokenEnd = pBlendedEnd;
	}

	if ( pLastTokenEnd!=pTokenizer->GetBufferEnd() )
		tFunctor.OnTail ( pLastTokenEnd-pStartPtr, pTokenizer->GetBufferEnd() - pLastTokenEnd, pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

	tFunctor.OnFinish();
}


/// snippets query words for different cases
class SnippetsFastQword_c : public ISphQword
{
public:
	const CSphVector<int> *		m_pHits;
	CSphMatch					m_tMatch;
	DWORD						m_uLastPos;

public:
	explicit SnippetsFastQword_c ( const CSphVector<int> * pHits )
		: m_pHits ( pHits )
		, m_uLastPos ( 0 )
	{}

	virtual ~SnippetsFastQword_c () {}

	void Setup ( DWORD uLastPos )
	{
		m_iDocs = 0;
		m_iHits = 0;
		m_uLastPos = uLastPos;
		if ( m_pHits && m_pHits->GetLength() )
		{
			m_iDocs = 1;
			m_iHits = m_pHits->GetLength();
			m_uMatchHits = 0;
			m_bHasHitlist = true;
		}
	}

	bool HasHits () const
	{
		return m_pHits && m_uMatchHits<(DWORD)m_pHits->GetLength();
	}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		m_dQwordFields.SetAll();
		m_tMatch.m_uDocID = !m_tMatch.m_uDocID && HasHits() ? 1 : 0;
		return m_tMatch;
	}

	virtual Hitpos_t GetNextHit ()
	{
		if ( !HasHits() )
			return EMPTY_HIT;

		int iPosition = *( m_pHits->Begin() + m_uMatchHits++ );
		return HITMAN::Create ( 0, iPosition, (m_uLastPos==(DWORD)iPosition) );
	}

	virtual void SeekHitlist ( SphOffset_t ) {}
};


/// snippets query word setup
class SnippetsFastQwordSetup_c : public ISphQwordSetup
{
public:
	const HitCollector_c * m_pHiglighter;

public:
	explicit SnippetsFastQwordSetup_c ( const HitCollector_c * pHiglighter )
	{
		m_pHiglighter = pHiglighter;
	}

	virtual ~SnippetsFastQwordSetup_c () {}

	virtual ISphQword * QwordSpawn ( const XQKeyword_t & tWord ) const
	{
		return new SnippetsFastQword_c ( m_pHiglighter->GetHitlist ( tWord ) );
	}

	virtual bool QwordSetup ( ISphQword * pQword ) const
	{
		SnippetsFastQword_c * pWord = (SnippetsFastQword_c *)pQword;
		pWord->Setup ( m_pHiglighter->m_tContainer.m_uLastPos );
		return true;
	}
};


inline bool operator < ( const SphHitMark_t & a, const SphHitMark_t & b )
{
	return a.m_uPosition < b.m_uPosition;
}


// with sentence in query we should consider SENTECE, PARAGRAPH, ZONE
// with paragraph in query we should consider PARAGRAPH, ZONE
// with zone in query we should consider ZONE
int ConvertSPZ ( DWORD eSPZ )
{
	if ( eSPZ & SPH_SPZ_SENTENCE )
		return MAGIC_CODE_SENTENCE;
	else if ( eSPZ & SPH_SPZ_PARAGRAPH )
		return MAGIC_CODE_PARAGRAPH;
	else if ( eSPZ & SPH_SPZ_ZONE )
		return MAGIC_CODE_ZONE;
	else
		return 0;
}


static void EmitPassagesOrdered ( CSphVector<BYTE> & dResult, const CSphVector<BYTE> & dPassageText, const CSphVector<int> & dPassageHeads, CSphVector<Passage_t> & dPassages,
	const CSphString & sChunkSeparator )
{
	int iSeparatorLen = sChunkSeparator.Length();
	dResult.Reserve ( dPassageText.GetLength() + dPassageHeads.GetLength()*iSeparatorLen*2 );
	dResult.Resize(0);

	int iOutLen = dResult.GetLength();
	dResult.Resize ( iOutLen+iSeparatorLen );
	memcpy ( &dResult[iOutLen], sChunkSeparator.cstr(), iSeparatorLen );

	ARRAY_FOREACH ( iPassage, dPassages )
	{
		int iBest = -1;
		ARRAY_FOREACH ( i, dPassages )
			if ( iBest==-1 || ( i!=iBest && dPassages[i].GetWeight()>=dPassages[iBest].GetWeight() ) )
				iBest = i;

		assert ( iBest!=-1 );

		dPassages[iBest].m_iQwordCount = 0;
		dPassages[iBest].m_iUniqQwords = 0;
		dPassages[iBest].m_iQwordsWeight = 0;
		dPassages[iBest].m_iMinGap = -1;

		int iLen = ( iBest==dPassageHeads.GetLength()-1 ? dPassageText.GetLength() : dPassageHeads[iBest+1] ) - dPassageHeads[iBest];

		iOutLen = dResult.GetLength();
		dResult.Resize ( iOutLen+iLen );
		memcpy ( &dResult[iOutLen], &(dPassageText[dPassageHeads[iBest]]), iLen );

		iOutLen = dResult.GetLength();
		dResult.Resize ( iOutLen+iSeparatorLen );
		memcpy ( &dResult[iOutLen], sChunkSeparator.cstr(), iSeparatorLen );
	}
}


static void HighlightPassages ( CacheStreamer_c & tStreamer, ExtractExcerpts_c & tExtractor, ExcerptQuery_t & tFixedSettings, const CSphIndexSettings & tIndexSettings, SnippetsDocIndex_c & tContainer,
	const char * sDoc, int iDocLen, CSphDict * pDict, ISphTokenizer * pTokenizer, const CSphVector<SphHitMark_t> * dMarked, FunctorZoneInfo_t * pZoneInfo, CSphVector<BYTE> & dRes )
{
	tFixedSettings.m_bWeightOrder = tExtractor.m_bFixedWeightOrder;

	if ( tExtractor.m_dPassages.GetLength() )
	{
		// 3rd pass: highlight passages
		HighlightPassages_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, dMarked ? dMarked : &tExtractor.m_dCollectedHits,
			tExtractor.m_dPassages, pZoneInfo );
		tStreamer.Tokenize ( tHighlighter );

		if ( tFixedSettings.m_bWeightOrder )
		{
			CSphVector<BYTE> dResult;
			EmitPassagesOrdered ( dResult, tHighlighter.m_dResult, tHighlighter.m_dPassageHeads, tExtractor.m_dPassages, tFixedSettings.m_sChunkSeparator );
			dRes.SwapData ( dResult );

		} else
			dRes.SwapData ( tHighlighter.m_dResult );

	} else if ( !tFixedSettings.m_bAllowEmpty )
	{
		DocStartHighlighter_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen );
		tStreamer.Tokenize ( tHighlighter );

		dRes.SwapData ( tHighlighter.m_dStartResult );
	}
}


static void FixupQueryLimits ( const SnippetsDocIndex_c & tContainer, const ExcerptQuery_t & tQuerySettings, ExcerptQuery_t & tFixedSettings, CSphString & sWarning, DWORD uFoundTerms = 0 )
{
	if ( tQuerySettings.m_iLimitWords && ( tQuerySettings.m_bExactPhrase || tQuerySettings.m_bForceAllWords ) )
	{
		int iNewWordLimit = 0;

		if ( !uFoundTerms )
			iNewWordLimit = tContainer.GetNumTerms();
		else
		{
			DWORD uFound = uFoundTerms;
			for ( ; uFound; iNewWordLimit++ )
				uFound &= uFound-1;
		}

		if ( iNewWordLimit > tQuerySettings.m_iLimitWords )
		{
			sWarning.SetSprintf ( "number of query terms (%d) is greater than the word limit setting (%d): limit increased", iNewWordLimit, tQuerySettings.m_iLimitWords );
			tFixedSettings.m_iLimitWords = iNewWordLimit;
		}
	}

	if ( tQuerySettings.m_iLimit )
	{
		int iTotalLen = 0;
		int iMaxLen = 0;
		int nTermsUsed = 0;

		for ( int i = 0; i < tContainer.GetNumTerms(); i++ )
		{
			int iLenCP = tContainer.GetTermWeight(i);
			if ( !uFoundTerms || ( uFoundTerms && ( uFoundTerms & ( 1UL << i ) ) ) )
			{
				iTotalLen += iLenCP;
				iMaxLen = Max ( iMaxLen, iLenCP );
				nTermsUsed++;
			}
		}

		int iNewLimit = iMaxLen;
		if ( tQuerySettings.m_bExactPhrase || tQuerySettings.m_bForceAllWords )
			iNewLimit = iTotalLen+nTermsUsed-1;

		if ( iNewLimit > tQuerySettings.m_iLimit )
		{
			sWarning.SetSprintf ( "query length (%d) is greater than the limit setting (%d): limit increased", iNewLimit, tQuerySettings.m_iLimit );
			tFixedSettings.m_iLimit = iNewLimit;
		}
	}
}


static void DoHighlighting ( const ExcerptQuery_t & tQuerySettings,
	const CSphIndexSettings & tIndexSettings, const XQQuery_t & tExtQuery, DWORD eExtQuerySPZ,
	const char * sDoc, int iDocLen,
	CSphDict * pDict, ISphTokenizer * pTokenizer, const CSphHTMLStripper * pStripper,
	CSphString & sWarning, CSphString & sError, ISphTokenizer * pQueryTokenizer, CSphVector<BYTE> & dRes )
{
	assert ( !tIndexSettings.m_uAotFilterMask || ( !tQuerySettings.m_bExactPhrase && tQuerySettings.m_bHighlightQuery ) );
	ExcerptQuery_t tFixedSettings ( tQuerySettings );

	// exact_phrase emulation
	// bug of words is replaced by query_mode=1 + "query words"
	XQQuery_t tExactPhraseQuery;
	bool bPhraseEmulation = tQuerySettings.m_bExactPhrase;
	if ( bPhraseEmulation )
	{
		if ( !tQuerySettings.m_bHighlightQuery && tQuerySettings.m_sWords.Length() && strchr ( tQuerySettings.m_sWords.cstr(), 0x22 )==NULL )
			tFixedSettings.m_sWords.SetSprintf ( "\"%s\"", tQuerySettings.m_sWords.cstr() );

		tFixedSettings.m_bHighlightQuery = true;

		CSphSchema tSchema;
		if ( !sphParseExtendedQuery ( tExactPhraseQuery, tFixedSettings.m_sWords.cstr(), NULL, pQueryTokenizer, &tSchema, pDict, tIndexSettings ) )
		{
			sError = tExactPhraseQuery.m_sParseError;
			return;
		}
		sWarning = tExactPhraseQuery.m_sParseWarning;
		if ( tExactPhraseQuery.m_pRoot )
			tExactPhraseQuery.m_pRoot->ClearFieldMask();
	}

	bool bRetainHtml = ( tFixedSettings.m_sStripMode=="retain" );

	// adjust tokenizer for markup-retaining mode
	if ( bRetainHtml )
		pTokenizer->AddSpecials ( "<" );

	// create query and hit lists container, parse query
	SnippetsDocIndex_c tContainer ( tFixedSettings.m_bHighlightQuery, ( bPhraseEmulation ? tExactPhraseQuery : tExtQuery ) );
	tContainer.ParseQuery ( tFixedSettings.m_sWords.cstr(), pQueryTokenizer, pDict, eExtQuerySPZ );

	bool bHighlightAll = ( tFixedSettings.m_iLimit==0 || tFixedSettings.m_iLimit>=iDocLen ) && ( tFixedSettings.m_iLimitWords==0 || tFixedSettings.m_iLimitWords>iDocLen/2 )
		&& tFixedSettings.m_ePassageSPZ==SPH_SPZ_NONE;

	int iSPZ = ConvertSPZ ( eExtQuerySPZ | ( bHighlightAll ? 0 : tFixedSettings.m_ePassageSPZ ) );

	// do highlighting
	if ( !tFixedSettings.m_bHighlightQuery )
	{
		if ( bHighlightAll )
		{
			// simple bag of words query
			// do just one tokenization pass over the document (not storing any tokens), matching and highlighting keywords
			HighlightPlain_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, iDocLen );
			TokenizeDocument ( tHighlighter, NULL, 0 );

			if ( !tHighlighter.m_iMatchesCount && tFixedSettings.m_bAllowEmpty )
				tHighlighter.m_dResult.Reset();

			dRes.SwapData ( tHighlighter.m_dResult );

		} else
		{
			FixupQueryLimits ( tContainer, tQuerySettings, tFixedSettings, sWarning );

			CacheStreamer_c tStreamer ( iDocLen );
			ExtractExcerpts_c tExtractor ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, NULL, &tStreamer );
			tExtractor.m_bCollectExtraZoneInfo = true;

			TokenizeDocument ( tExtractor, pStripper, iSPZ );

			tStreamer.m_pZoneInfo = &tExtractor.m_tZoneInfo;
			HighlightPassages ( tStreamer, tExtractor, tFixedSettings, tIndexSettings, tContainer, sDoc, iDocLen, pDict, pTokenizer, NULL, &tExtractor.m_tZoneInfo, dRes );
		}

		// add trailing zero, and return
		dRes.Add(0);

	} else
	{
		// query with syntax
		// do two passes over document
		// 1st pass will tokenize document, match keywords, and store positions into docindex
		// 2nd pass will highlight matching positions only (with some matching engine aid)

		CacheStreamer_c tStreamer ( iDocLen );

		// do the 1st pass
		HitCollector_c tHitCollector ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, tStreamer );
		TokenizeDocument ( tHitCollector, pStripper, iSPZ );

		FunctorZoneInfo_t * pZoneInfo = &tHitCollector.m_tZoneInfo;
		tStreamer.m_pZoneInfo = pZoneInfo;

		FixupQueryLimits ( tContainer, tQuerySettings, tFixedSettings, sWarning, tHitCollector.m_uFoundWords );

		// prepare for the 2nd pass (that is, extract matching hits)
		SnippetZoneChecker_c tZoneChecker ( tHitCollector.m_dZones, tHitCollector.m_tZoneInfo.m_hZones, tContainer.m_tQuery.m_dZones );

		SnippetsFastQwordSetup_c tQwordSetup ( &tHitCollector );
		tQwordSetup.m_pDict = pDict;
		tQwordSetup.m_eDocinfo = SPH_DOCINFO_EXTERN;
		tQwordSetup.m_pWarning = &sError;
		tQwordSetup.m_pZoneChecker = &tZoneChecker;

		// got a lot of stack allocated variables (up to 30K)
		// check that query not overflow stack here
		if ( !sphCheckQueryHeight ( tContainer.m_tQuery.m_pRoot, sError ) )
			return;

		CSphScopedPtr<CSphHitMarker> pMarker ( CSphHitMarker::Create ( tContainer.m_tQuery.m_pRoot, tQwordSetup ) );
		if ( !pMarker.Ptr() )
		{
			// no hits - just highlight document start
			if ( !tFixedSettings.m_bAllowEmpty )
			{
				DocStartHighlighter_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen );
				tStreamer.Tokenize ( tHighlighter );

				dRes.SwapData ( tHighlighter.m_dStartResult );
			}
			dRes.Add(0);
			return;
		}

		CSphVector<SphHitMark_t> dMarked;
		pMarker->Mark ( dMarked );

		// we just collected matching spans into dMarked, but!
		// certain spans might not match all words within the span
		// for instance, (one NEAR/3 two) could return a 5-word span
		// but we do have full matching keywords list in tContainer
		// so let's post-process and break down such spans
		// FIXME! what about phrase spans vs stopwords? they will be split now
		if ( !tQuerySettings.m_bExactPhrase )
			ARRAY_FOREACH ( i, dMarked )
		{
			if ( dMarked[i].m_uSpan==1 )
				continue;

			CSphVector<int> dMatched;
			for ( int j=0; j<(int)dMarked[i].m_uSpan; j++ )
			{
				// OPTIMZE? we can premerge all dochits vectors once
				const int iPos = dMarked[i].m_uPosition + j;
				ARRAY_FOREACH ( k, tContainer.m_dDocHits )
					if ( tContainer.m_dDocHits[k].BinarySearch ( iPos ) )
				{
					dMatched.Add ( iPos );
					break;
				}
			}

			// this is something that must never happen
			// we got a span out of the matching engine that does not match any keywords?!
			assert ( dMatched.GetLength() );
			if ( !dMatched.GetLength() )
			{
				dMarked.RemoveFast ( i-- ); // remove, rescan
				continue;
			}

			// append all matching keywords as 1-long spans
			ARRAY_FOREACH ( j, dMatched )
			{
				SphHitMark_t & tMarked = dMarked.Add();
				tMarked.m_uPosition = dMatched[j];
				tMarked.m_uSpan = 1;
			}

			// this swaps current span with the last 1-long span we added
			// which is by definition okay; so we need not rescan it
			dMarked.RemoveFast ( i );
		}
		dMarked.Uniq();

		if ( !tQuerySettings.m_bExactPhrase )
		{
			// we just exploded spans into actual matching hits
			// now lets fold marked and matched hits back into contiguous spans
			// so that we could highlight such spans instead of every individual word
			SphHitMark_t * pOut = dMarked.Begin(); // last emitted folded token
			SphHitMark_t * pIn = dMarked.Begin() + 1; // next token to process
			SphHitMark_t * pMax = dMarked.Begin() + dMarked.GetLength();
			while ( pIn<pMax )
			{
				if ( pIn->m_uPosition==( pOut->m_uPosition + pOut->m_uSpan ) )
				{
					pOut->m_uSpan += pIn->m_uSpan;
					pIn++;
				} else
				{
					*++pOut = *pIn++;
				}
			}
			if ( dMarked.GetLength()>1 )
				dMarked.Resize ( pOut - dMarked.Begin() + 1 );
		}

		if ( bHighlightAll )
		{
			// 2nd pass
			HighlightQuery_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, dMarked );
			tStreamer.Tokenize ( tHighlighter );

			if ( !tHighlighter.m_iMatchesCount && tFixedSettings.m_bAllowEmpty )
				tHighlighter.m_dResult.Reset();

			dRes.SwapData ( tHighlighter.m_dResult );

		} else if ( !dMarked.GetLength() )
		{
			// no hits - just highlight document start
			if ( !tFixedSettings.m_bAllowEmpty )
			{
				DocStartHighlighter_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen );
				tStreamer.Tokenize ( tHighlighter );

				dRes.SwapData ( tHighlighter.m_dStartResult );
			}

		} else
		{
			ExtractExcerpts_c tExtractor ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, &dMarked, NULL );
			tExtractor.m_bCollectExtraZoneInfo = false;
			tExtractor.m_uFoundWords = tHitCollector.m_uFoundWords;

			tStreamer.Tokenize ( tExtractor );

			HighlightPassages ( tStreamer, tExtractor, tFixedSettings, tIndexSettings, tContainer, sDoc, iDocLen, pDict, pTokenizer, &dMarked, pZoneInfo, dRes );
		}

		dRes.Add(0);
	}
}

/////////////////////////////////////////////////////////////////////////////

ExcerptQuery_t::ExcerptQuery_t ()
	: m_sBeforeMatch ( "<b>" )
	, m_sAfterMatch ( "</b>" )
	, m_sChunkSeparator ( " ... " )
	, m_sStripMode ( "index" )
	, m_iLimit ( 256 )
	, m_iLimitWords ( 0 )
	, m_iLimitPassages ( 0 )
	, m_iAround ( 5 )
	, m_iPassageId ( 1 )
	, m_bRemoveSpaces ( false )
	, m_bExactPhrase ( false )
	, m_bUseBoundaries ( false )
	, m_bWeightOrder ( false )
	, m_bHighlightQuery ( false )
	, m_bForceAllWords ( false )
	, m_iLoadFiles ( 0 )
	, m_bAllowEmpty ( false )
	, m_bEmitZones ( false )
	, m_iRawFlags ( -1 )
	, m_sFilePrefix ( "" )
	, m_iSize ( 0 )
	, m_iSeq ( 0 )
	, m_iNext ( -2 )
	, m_bHasBeforePassageMacro ( false )
	, m_bHasAfterPassageMacro ( false )
	, m_ePassageSPZ ( SPH_SPZ_NONE )
{
}

/////////////////////////////////////////////////////////////////////////////


void sphBuildExcerpt ( ExcerptQuery_t & tOptions, const CSphIndex * pIndex, const CSphHTMLStripper * pStripper, const XQQuery_t & tExtQuery,
						DWORD eExtQuerySPZ, CSphString & sWarning, CSphString & sError, CSphDict * pDict, ISphTokenizer * pDocTokenizer, ISphTokenizer * pQueryTokenizer )
{
	if ( tOptions.m_sStripMode=="retain"
		&& !( tOptions.m_iLimit==0 && tOptions.m_iLimitPassages==0 && tOptions.m_iLimitWords==0 ) )
	{
		sError = "html_strip_mode=retain requires that all limits are zero";
		return;
	}

	char * pData = const_cast<char*> ( tOptions.m_sSource.cstr() );
	CSphFixedVector<char> pBuffer ( 0 );

	if ( tOptions.m_iLoadFiles )
	{
		CSphAutofile tFile;
		if ( tOptions.m_sFilePrefix!="" )
		{
			CSphString sFilename;
			sFilename.SetSprintf ( "%s%s", tOptions.m_sFilePrefix.cstr(), tOptions.m_sSource.cstr() );
			if ( tFile.Open ( sFilename.cstr(), SPH_O_READ, sError )<0 )
				return;
		} else if ( tOptions.m_sSource.IsEmpty() )
		{
			sError.SetSprintf ( "snippet file name is empty" );
			return;
		} else if ( tFile.Open ( tOptions.m_sSource.cstr(), SPH_O_READ, sError )<0 )
			return;

		// will this ever trigger? time will tell; email me if it does!
		if ( tFile.GetSize()+1>=(SphOffset_t)INT_MAX )
		{
			sError.SetSprintf ( "%s too big for snippet (over 2 GB)", pData );
			return;
		}

		int iFileSize = (int)tFile.GetSize();
		if ( iFileSize<0 )
			return;

		pBuffer.Reset ( iFileSize+1 );
		if ( !tFile.Read ( pBuffer.Begin(), iFileSize, sError ) )
			return;

		pBuffer[iFileSize] = 0;
		pData = pBuffer.Begin();
	}

	// strip if we have to
	if ( pStripper && ( tOptions.m_sStripMode=="strip" || tOptions.m_sStripMode=="index" ) )
		pStripper->Strip ( (BYTE*)pData );

	if ( tOptions.m_sStripMode!="retain" )
		pStripper = NULL;

	// FIXME!!! check on real data (~100 Mb) as stripper changes len
	int iDataLen = strlen ( pData );

	DoHighlighting ( tOptions, pIndex->GetSettings(), tExtQuery, eExtQuerySPZ, pData, iDataLen, pDict, pDocTokenizer, pStripper,
		sWarning, sError, pQueryTokenizer, tOptions.m_dRes );
}

//
// $Id$
//
