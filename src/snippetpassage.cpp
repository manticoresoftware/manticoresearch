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

#include "snippetpassage.h"

#include "snippetindex.h"
#include "sphinxexcerpt.h"


static bool operator < ( const Passage_t & a, const Passage_t & b )
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

//////////////////////////////////////////////////////////////////////////

void Passage_t::Reset()
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
	m_iField = 0;
	m_dBeforeTokens.Resize(0);
	m_dAfterTokens.Resize(0);
}


void Passage_t::CopyData ( Passage_t & tPassage )
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
	m_iField = tPassage.m_iField;
	m_dBeforeTokens.SwapData ( tPassage.m_dBeforeTokens );
	m_dAfterTokens.SwapData ( tPassage.m_dAfterTokens );
}


int Passage_t::GetWeight() const
{
	return m_iQwordCount + m_iQwordsWeight*m_iMaxLCS + m_iMinGap;
}

//////////////////////////////////////////////////////////////////////////

static CSphVector<Passage_t> SelectBestPassages ( const CSphVector<Passage_t> & dPassages, const SnippetLimits_t & tLimits, const SnippetQuerySettings_t & tSettings, const SnippetsDocIndex_c & tContainer, DWORD uFoundWords )
{
	CSphVector<Passage_t> dShow;

	if ( !dPassages.GetLength() )
		return dShow;

	// our limits
	int iMaxPassages = tLimits.m_iLimitPassages
		? Min ( dPassages.GetLength(), tLimits.m_iLimitPassages )
		: dPassages.GetLength();
	int iMaxWords = tLimits.m_iLimitWords ? tLimits.m_iLimitWords : INT_MAX;
	int iMaxCp = tLimits.m_iLimit ? tLimits.m_iLimit : INT_MAX;

	DWORD uWords = 0; // mask of words in dShow so far
	int iTotalCodes = 0;
	int iTotalWords = 0;
	int iTotalKeywordCodes = 0;
	int iTotalKeywordWords = 0;

	CSphVector<int> dWeights ( dPassages.GetLength() );
	ARRAY_FOREACH ( i, dPassages )
		dWeights[i] = dPassages[i].m_iQwordsWeight;

	// collect enough best passages to show all keywords and max out the limits
	// don't care much if we're going over limits in this loop, it will be tightened below
	bool bAll = false;
	while ( dShow.GetLength() < iMaxPassages )
	{
		// get next best passage
		int iBest = -1;
		ARRAY_FOREACH ( i, dPassages )
		{
			Passage_t & tPass = dPassages[i];
			if ( tPass.m_iCodes && ( iBest==-1 || dPassages[iBest] < tPass ) )
				iBest = i;
		}
		if ( iBest<0 )
			break;

		Passage_t & tBest = dPassages[iBest];

		// force_all_keywords passage may be very big, so let's allow to show one of them
		if ( !tSettings.m_bForceAllWords || dShow.GetLength() )
			if ( iTotalKeywordCodes+tBest.m_iCodesBetweenKeywords>iMaxCp || iTotalKeywordWords+tBest.m_iWordsBetweenKeywords>iMaxWords )
				break;

		bool bFits = iTotalCodes+tBest.m_iCodes<=iMaxCp && iTotalWords+tBest.m_iWords<=iMaxWords;
		if ( uWords==uFoundWords && !bFits )
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
		if ( !bAll && uWords==uFoundWords )
		{
			bAll = true;
			ARRAY_FOREACH ( i, dPassages )
				dPassages[i].m_iQwordsWeight = dWeights[i];
		}

		// if we're already showing all words, re-weighting is not needed any more
		if ( bAll )
			continue;

		// re-weight passages, adjust for new mask of shown words
		// FIXME! re-weighting doesn't change m_iQwordCount (and qwords could possibly be duplicated) and LCS
		ARRAY_FOREACH ( i, dPassages )
		{
			Passage_t & tPass = dPassages[i];
			if ( !tPass.m_iCodes )
				continue;

			DWORD uMask = tBest.m_uQwords;
			for ( int iWord=0; uMask; iWord++, uMask >>= 1 )
				if ( ( uMask & 1 ) && ( tPass.m_uQwords & ( 1UL<<iWord ) ) )
				{
					tPass.m_iQwordsWeight -= tContainer.GetTermWeight(iWord);
					tPass.m_iQwordCount--; // doesn't account for dupes
					tPass.m_iUniqQwords--;
				}

			tPass.m_uQwords &= ~uWords;
		}
	}

	// if all passages won't fit into the limit, try to trim them a bit
	//
	// this step is skipped when use_boundaries is enabled, because
	// each passage must be a separate sentence (delimited by
	// boundaries) and we don't want to split them
	if ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !tSettings.m_bUseBoundaries )
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
	while ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !tSettings.m_bForceAllWords )
	{
		iTotalCodes -= dShow.Last().m_iCodes;
		iTotalWords -= dShow.Last().m_iWords;
		dShow.RemoveFast ( dShow.GetLength()-1 );
	}

	// sort passages in the document order
	dShow.Sort ( PassagePositionOrder_fn() );

	return dShow;
}

//////////////////////////////////////////////////////////////////////////

PassageContext_t::PassageContext_t()
{
	m_dPassages.Reserve(1024);
	memset ( m_dQwordWeights, 0, sizeof(m_dQwordWeights) );
}


CSphVector<Passage_t> PassageContext_t::SelectBest ( const SnippetLimits_t & tLimits, const SnippetQuerySettings_t & tSettings, const SnippetsDocIndex_c & tContainer, DWORD uFoundWords ) const
{
	CSphVector<Passage_t> dPassagesToShow;
	if ( !m_dPassages.GetLength() )
		return dPassagesToShow;

	dPassagesToShow = SelectBestPassages ( m_dPassages, tLimits, tSettings, tContainer, uFoundWords );
	for ( auto & i : dPassagesToShow )
	{
		i.m_dBeforeTokens.Reset();
		i.m_dAfterTokens.Reset();
	}

	return dPassagesToShow;
}
