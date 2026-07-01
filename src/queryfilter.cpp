//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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

static BYTE * CopyKeywordString ( KeywordBuf_t & dDst, const char * sSrc )
{
	int iLen = (int)strlen ( sSrc );
	if ( iLen>=dDst.GetLength() )
		return nullptr;

	memcpy ( dDst.Begin(), sSrc, iLen );
	dDst.Begin()[iLen] = '\0';
	return dDst.Begin();
}


static BYTE * CopyExactKeywordString ( KeywordBuf_t & dDst, const BYTE * sSrc )
{
	int iLen = (int)strlen ( (const char*)sSrc );
	if ( iLen+1>=dDst.GetLength() )
		return nullptr;

	dDst.Begin()[0] = '=';
	memcpy ( dDst.Begin()+1, sSrc, iLen+1 );
	return dDst.Begin();
}


static void ReportSkippedOverLimitTokens ( ISphTokenizer * pTokenizer, CSphString * pWarning, CSphQueryResultMeta * pResult )
{
	if ( !pWarning && pResult )
		pWarning = &pResult->m_sWarning;

	if ( pWarning )
		WarnAppendSkipped ( *pWarning, pTokenizer->ResetOversizedTokenCount() );
	else
		pTokenizer->ResetOversizedTokenCount();
}


void ISphQueryFilter::GetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, ExpansionContext_t & tCtx )
{
	assert ( m_pTokenizer && m_pDict && m_pSettings );

	DictFormat_e eDictFormat = m_pDict->GetSettings().GetDictFormat();
	KeywordBuf_t dDstWord ( eDictFormat );
	KeywordBuf_t dTokenized ( eDictFormat );
	KeywordBuf_t dTmp ( eDictFormat );
	KeywordBuf_t dTmp2 ( eDictFormat );
	BYTE* sWord;
	int iQpos = 1;
	int iExactQpos = -1;
	CSphVector<int> dQposWildcards;
	CSphVector<BYTE> dSkipAotTransform;

	// FIXME!!! got rid of duplicated term stat and qword setup
	while ( ( sWord = m_pTokenizer->GetToken() ) )
	{
		if ( tCtx.m_iCutoff!=-1 && dKeywords.GetLength()>=tCtx.m_iCutoff )
		{
			dKeywords.Add(); // fake keyword to let code know we're past cutoff
			ReportSkippedOverLimitTokens ( m_pTokenizer.Ptr(), m_tFoldSettings.m_pWarning, tCtx.m_pResult );
			return;
		}

		if ( m_pTokenizer->WasTokenSpecial() )
		{
			if ( sWord[0]=='=' )
				iExactQpos = iQpos;
			continue;
		}

		if ( iExactQpos==iQpos )
		{
			sWord = CopyExactKeywordString ( dDstWord, sWord );
			if ( !sWord )
				continue;
		}

		const BYTE* sMultiform = m_pTokenizer->GetTokenizedMultiform();
		BYTE * sTokenized = CopyKeywordString ( dTokenized, sMultiform ? (const char*)sMultiform : (const char*)sWord );
		if ( !sTokenized )
			continue;

		int iKeywordStart = dKeywords.GetLength();
		if ( tCtx.m_bAllowExpansion && ( !m_tFoldSettings.m_bFoldWildcards || m_tFoldSettings.m_bStats ) && sphHasExpandableWildcards ( (const char*)sWord ) )
		{
			dQposWildcards.Add ( iQpos );

			ISphWordlist::Args_t tWordlist ( false, tCtx );
			bool bExpanded = sphExpandGetWords ( (const char*)sWord, tCtx, tWordlist );
			tCtx.m_bHasWildcards |= bExpanded;

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
		for ( int iKeyword = iKeywordStart; iKeyword < dKeywords.GetLength(); ++iKeyword )
			dSkipAotTransform.Add ( bBlended ? 1 : 0 );

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
	{
		ReportSkippedOverLimitTokens ( m_pTokenizer.Ptr(), m_tFoldSettings.m_pWarning, tCtx.m_pResult );
		return;
	}

	XQLimitSpec_t tSpec;
	CSphVector<const XQNode_t*> dChildren ( 64 );

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
		if ( tSkipTransform.BitGetOr ( iKeywordQpos ) )
			continue;
		if ( ShouldBypassMorphology ( m_pDict->GetSettings().GetDictFormat(), dKeywords[iTokenized].m_sNormalized.Length() ) )
			continue;

		if ( dSkipAotTransform[iTokenized] )
		{
			if ( m_pSettings->m_bIndexExactWords && dKeywords[iTokenized].m_sNormalized.cstr()[0]!='=' )
			{
				snprintf ( (char *)sTmp, sizeof(sTmp)-1, "=%s", dKeywords[iTokenized].m_sNormalized.cstr() );
				sTmp[sizeof(sTmp)-1] = '\0';

				strncpy ( (char*)sTmp2, dKeywords[iTokenized].m_sTokenized.scstr(), sizeof ( sTmp2 ) - 1 );
				sTmp2[sizeof(sTmp2)-1] = '\0';

				AddKeywordStats ( sTmp, sTmp2, iKeywordQpos, dKeywords );
			}
			continue;
		}

		// MUST copy as Dict::GetWordID changes word and might add symbols
		BYTE * sTokenized = CopyKeywordString ( dTokenized, dKeywords[iTokenized].m_sNormalized.scstr() );
		if ( !sTokenized )
			continue;
		int iPreAotCount = dKeywords.GetLength();

		XQNode_t tAotNode ( tSpec );
		XQKeyword_t dWord;
		dWord.m_sWord = (char *) sTokenized;
		tAotNode.AddDirtyWord ( std::move(dWord) );
		TransformAotFilter ( &tAotNode, m_pDict->GetWordforms(), *m_pSettings );

		dChildren.Resize ( 0 );
		dChildren.Add ( &tAotNode );

		// recursion unfolded
		ARRAY_FOREACH ( iChild, dChildren ) // notice dChildren.Add() below; ranged-for loop is not suitable
		{
			const XQNode_t * pChild = dChildren[iChild];

			// process all words at node
			for ( const auto& dAotKeyword: pChild->dWords() )
			{
				// MUST copy as Dict::GetWordID changes word and might add symbols
				BYTE * sTmp = CopyKeywordString ( dTmp, dAotKeyword.m_sWord.scstr() );
				// prevent use-after-free-bug due to vector grow: AddKeywordsStats() calls dKeywords.Add()
				BYTE * sTmp2 = CopyKeywordString ( dTmp2, dKeywords[iTokenized].m_sTokenized.scstr() );
				if ( !sTmp || !sTmp2 )
					continue;
				AddKeywordStats ( sTmp, sTmp2, iKeywordQpos, dKeywords );
			}

			// push all child nodes at node to process list
			for ( const XQNode_t* pGrand : pChild->dChildren() )
				dChildren.Add ( pGrand );
		}

		bool bGotLemmas = iPreAotCount != dKeywords.GetLength();
		if ( !bGotLemmas )
			continue;

		// remove (replace) original word in case of AOT taken place
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

	// sort by qpos
	if ( dKeywords.GetLength() != iTokenizedTotal )
		sphSort ( dKeywords.Begin(), dKeywords.GetLength(), bind ( &CSphKeywordInfo::m_iQpos ) );

	ReportSkippedOverLimitTokens ( m_pTokenizer.Ptr(), m_tFoldSettings.m_pWarning, tCtx.m_pResult );
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

void UniqKeywords ( CSphVector<CSphKeywordInfo> & dSrc )
{
	CSphOrderedHash < CSphKeywordInfo, uint64_t, IdentityHash_fn, 256 > hWords;
	ARRAY_FOREACH ( i, dSrc )
	{
		const CSphKeywordInfo & tInfo = dSrc[i];
		uint64_t uKey = sphFNV64 ( tInfo.m_iQpos );
		uKey = sphFNV64 ( tInfo.m_sNormalized.cstr(), tInfo.m_sNormalized.Length(), uKey );

		CSphKeywordInfo & tVal = hWords.AddUnique ( uKey );
		if ( !tVal.m_iQpos )
		{
			tVal = tInfo;
		} else
		{
			tVal.m_iDocs += tInfo.m_iDocs;
			tVal.m_iHits += tInfo.m_iHits;
		}
	}

	dSrc.Resize ( 0 );
	for ( const auto& tWord : hWords )
		dSrc.Add ( tWord.second );

	sphSort ( dSrc.Begin(), dSrc.GetLength(), KeywordSorter_fn() );
}
