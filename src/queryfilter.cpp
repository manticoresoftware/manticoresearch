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

#include "queryfilter.h"

#include "sphinxsearch.h"

void ISphQueryFilter::GetKeywords ( CSphVector<CSphKeywordInfo>& dKeywords, const ExpansionContext_t& tCtx )
{
	assert ( m_pTokenizer && m_pDict && m_pSettings );

	BYTE sTokenized[3 * SPH_MAX_WORD_LEN + 4];
	BYTE* sWord;
	int iQpos = 1;
	CSphVector<int> dQposWildcards;

	// FIXME!!! got rid of duplicated term stat and qword setup
	while ( ( sWord = m_pTokenizer->GetToken() ) != NULL )
	{
		if ( tCtx.m_iCutoff!=-1 && dKeywords.GetLength()>=tCtx.m_iCutoff )
		{
			dKeywords.Add(); // fake keyword to let code know we're past cutoff
			return;
		}

		const BYTE* sMultiform = m_pTokenizer->GetTokenizedMultiform();
		strncpy ( (char*)sTokenized, sMultiform ? (const char*)sMultiform : (const char*)sWord, sizeof ( sTokenized ) - 1 );

		if ( ( !m_tFoldSettings.m_bFoldWildcards || m_tFoldSettings.m_bStats ) && sphHasExpandableWildcards ( (const char*)sWord ) )
		{
			dQposWildcards.Add ( iQpos );

			ISphWordlist::Args_t tWordlist ( false, tCtx.m_iExpansionLimit, tCtx.m_bHasExactForms, tCtx.m_eHitless, tCtx.m_pIndexData );
			bool bExpanded = sphExpandGetWords ( (const char*)sWord, tCtx, tWordlist );

			int iDocs = 0;
			int iHits = 0;

			// might fold wildcards but still want to sum up stats
			if ( m_tFoldSettings.m_bFoldWildcards && m_tFoldSettings.m_bStats )
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					iDocs += tWordlist.m_dExpanded[i].m_iDocs;
					iHits += tWordlist.m_dExpanded[i].m_iHits;
				}
				bExpanded = false;
			} else
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					CSphKeywordInfo& tInfo = dKeywords.Add();
					tInfo.m_sTokenized = (const char*)sWord;
					tInfo.m_sNormalized = tWordlist.GetWordExpanded ( i );
					tInfo.m_iDocs = tWordlist.m_dExpanded[i].m_iDocs;
					tInfo.m_iHits = tWordlist.m_dExpanded[i].m_iHits;
					tInfo.m_iQpos = iQpos;

					RemoveDictSpecials ( tInfo.m_sNormalized, ( m_pSettings->m_eBigramIndex!=SPH_BIGRAM_NONE ) );
				}
			}

			if ( !bExpanded || !tWordlist.m_dExpanded.GetLength() )
			{
				CSphKeywordInfo& tInfo = dKeywords.Add();
				tInfo.m_sTokenized = (const char*)sWord;
				tInfo.m_sNormalized = (const char*)sWord;
				tInfo.m_iDocs = iDocs;
				tInfo.m_iHits = iHits;
				tInfo.m_iQpos = iQpos;
			}
		} else
		{
			AddKeywordStats ( sWord, sTokenized, iQpos, dKeywords );
		}

		// FIXME!!! handle consecutive blended wo blended parts
		bool bBlended = m_pTokenizer->TokenIsBlended();

		if ( bBlended )
		{
			if ( m_tFoldSettings.m_bFoldBlended )
				iQpos += m_pTokenizer->SkipBlended();
		} else
		{
			iQpos++;
		}
	}

	if ( !m_pSettings->m_uAotFilterMask )
		return;

	XQLimitSpec_t tSpec;
	BYTE sTmp[3 * SPH_MAX_WORD_LEN + 4];
	BYTE sTmp2[3 * SPH_MAX_WORD_LEN + 4];
	CSphVector<XQNode_t*> dChildren ( 64 );

	CSphBitvec tSkipTransform;
	if ( dQposWildcards.GetLength() )
	{
		tSkipTransform.Init ( iQpos + 1 );
		ARRAY_FOREACH ( i, dQposWildcards )
			tSkipTransform.BitSet ( dQposWildcards[i] );
	}

	int iTokenizedTotal = dKeywords.GetLength();
	for ( int iTokenized = 0; iTokenized < iTokenizedTotal; iTokenized++ )
	{
		int iKeywordQpos = dKeywords[iTokenized].m_iQpos;

		// do not transform expanded wild-cards
		if ( !tSkipTransform.IsEmpty() && tSkipTransform.BitGet ( iKeywordQpos ) )
			continue;

		// MUST copy as Dict::GetWordID changes word and might add symbols
		strncpy ( (char*)sTokenized, dKeywords[iTokenized].m_sNormalized.scstr(), sizeof ( sTokenized ) - 1 );
		int iPreAotCount = dKeywords.GetLength();

		XQNode_t tAotNode ( tSpec );
		tAotNode.m_dWords.Resize ( 1 );
		tAotNode.m_dWords.Begin()->m_sWord = (char*)sTokenized;
		TransformAotFilter ( &tAotNode, m_pDict->GetWordforms(), *m_pSettings );

		dChildren.Resize ( 0 );
		dChildren.Add ( &tAotNode );

		// recursion unfolded
		ARRAY_FOREACH ( iChild, dChildren )
		{
			// process all words at node
			ARRAY_FOREACH ( iAotKeyword, dChildren[iChild]->m_dWords )
			{
				// MUST copy as Dict::GetWordID changes word and might add symbols
				strncpy ( (char*)sTmp, dChildren[iChild]->m_dWords[iAotKeyword].m_sWord.scstr(), sizeof ( sTmp ) - 1 );
				// prevent use-after-free-bug due to vector grow: AddKeywordsStats() calls dKeywords.Add()
				strncpy ( (char*)sTmp2, dKeywords[iTokenized].m_sTokenized.scstr(), sizeof ( sTmp2 ) - 1 );
				AddKeywordStats ( sTmp, sTmp2, iKeywordQpos, dKeywords );
			}

			// push all child nodes at node to process list
			const XQNode_t* pChild = dChildren[iChild];
			ARRAY_FOREACH ( iRec, pChild->m_dChildren )
				dChildren.Add ( pChild->m_dChildren[iRec] );
		}

		bool bGotLemmas = ( iPreAotCount != dKeywords.GetLength() );

		// remove (replace) original word in case of AOT taken place
		if ( bGotLemmas )
		{
			if ( !m_tFoldSettings.m_bFoldLemmas )
			{
				::Swap ( dKeywords[iTokenized], dKeywords.Last() );
				dKeywords.Resize ( dKeywords.GetLength() - 1 );
			} else
			{
				int iKeywordWithMaxHits = iPreAotCount;
				for ( int i = iPreAotCount + 1; i < dKeywords.GetLength(); i++ )
					if ( dKeywords[i].m_iHits > dKeywords[iKeywordWithMaxHits].m_iHits )
						iKeywordWithMaxHits = i;

				CSphString sNormalizedWithMaxHits = dKeywords[iKeywordWithMaxHits].m_sNormalized;

				int iDocs = 0;
				int iHits = 0;
				if ( m_tFoldSettings.m_bStats )
				{
					for ( int i = iPreAotCount; i < dKeywords.GetLength(); i++ )
					{
						iDocs += dKeywords[i].m_iDocs;
						iHits += dKeywords[i].m_iHits;
					}
				}
				::Swap ( dKeywords[iTokenized], dKeywords[iPreAotCount] );
				dKeywords.Resize ( iPreAotCount );
				dKeywords[iTokenized].m_iDocs = iDocs;
				dKeywords[iTokenized].m_iHits = iHits;
				dKeywords[iTokenized].m_sNormalized = sNormalizedWithMaxHits;

				RemoveDictSpecials ( dKeywords[iTokenized].m_sNormalized, ( m_pSettings->m_eBigramIndex!=SPH_BIGRAM_NONE ) );
			}
		}
	}

	// sort by qpos
	if ( dKeywords.GetLength() != iTokenizedTotal )
		sphSort ( dKeywords.Begin(), dKeywords.GetLength(), bind ( &CSphKeywordInfo::m_iQpos ) );
}

void CSphTemplateQueryFilter::AddKeywordStats ( BYTE* sWord, const BYTE* sTokenized, int iQpos, CSphVector<CSphKeywordInfo>& dKeywords )
{
	SphWordID_t iWord = m_pDict->GetWordID ( sWord );
	if ( !iWord )
		return;

	CSphKeywordInfo& tInfo = dKeywords.Add();
	tInfo.m_sTokenized = (const char*)sTokenized;
	tInfo.m_sNormalized = (const char*)sWord;
	tInfo.m_iDocs = 0;
	tInfo.m_iHits = 0;
	tInfo.m_iQpos = iQpos;

	RemoveDictSpecials ( tInfo.m_sNormalized, ( m_pSettings->m_eBigramIndex!=SPH_BIGRAM_NONE ) );
}

void CSphPlainQueryFilter::AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords )
{
	assert ( !m_tFoldSettings.m_bStats || ( m_pTermSetup && m_pQueryWord ) );

	SphWordID_t iWord = m_pDict->GetWordID ( sWord );
	if ( !iWord )
		return;

	if ( m_tFoldSettings.m_bStats )
	{
		m_pQueryWord->Reset ();
		m_pQueryWord->m_sWord = (const char*)sWord;
		m_pQueryWord->m_sDictWord = (const char*)sWord;
		m_pQueryWord->m_uWordID = iWord;
		m_pTermSetup->QwordSetup ( m_pQueryWord );
	}

	CSphKeywordInfo & tInfo = dKeywords.Add();
	tInfo.m_sTokenized = (const char *)sTokenized;
	tInfo.m_sNormalized = (const char*)sWord;
	tInfo.m_iDocs = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iDocs : 0;
	tInfo.m_iHits = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iHits : 0;
	tInfo.m_iQpos = iQpos;

	RemoveDictSpecials ( tInfo.m_sNormalized, ( m_pSettings->m_eBigramIndex!=SPH_BIGRAM_NONE ) );
}
