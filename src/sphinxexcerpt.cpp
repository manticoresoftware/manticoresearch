//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
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
#include "fileutils.h"
#include "sphinxstem.h"

#include "snippetfunctor.h"
#include "snippetindex.h"
#include "snippetstream.h"

#include <math.h>


static bool SnippetTransformPassageMacros ( CSphString & sSrc, CSphString & sPost )
{
	const char sPassageMacro[] = "%PASSAGE_ID%";

	const char * sPass = NULL;
	if ( !sSrc.IsEmpty() )
		sPass = strstr ( sSrc.cstr(), sPassageMacro );

	if ( !sPass )
		return false;

	int iSrcLen = sSrc.Length();
	int iPassLen = sizeof ( sPassageMacro ) - 1;
	int iTailLen = iSrcLen - iPassLen - ( sPass - sSrc.cstr() );

	// copy tail
	if ( iTailLen )
		sPost.SetBinary ( sPass+iPassLen, iTailLen );

	CSphString sPre;
	sPre.SetBinary ( sSrc.cstr(), sPass - sSrc.cstr() );
	sSrc.Swap ( sPre );

	return true;
}


/////////////////////////////////////////////////////////////////////////////

void SnippetQuerySettings_t::Setup()
{
	m_bHasBeforePassageMacro = SnippetTransformPassageMacros ( m_sBeforeMatch, m_sBeforeMatchPassage );
	m_bHasAfterPassageMacro = SnippetTransformPassageMacros ( m_sAfterMatch, m_sAfterMatchPassage );
}


CSphString SnippetQuerySettings_t::AsString() const
{
	SnippetQuerySettings_t tDefault;
	StringBuilder_c tOut;
	tOut.StartBlock ( ",", "{", "}" );

	if ( m_sBeforeMatch!=tDefault.m_sBeforeMatch )			tOut.Appendf ( "before_match='%s'",		m_sBeforeMatch.cstr() );
	if ( m_sAfterMatch!=tDefault.m_sAfterMatch )			tOut.Appendf ( "after_match='%s'",		m_sAfterMatch.cstr() );
	if ( m_sChunkSeparator!=tDefault.m_sChunkSeparator )	tOut.Appendf ( "chunk_separator='%s'",	m_sChunkSeparator.cstr() );
	if ( m_sFieldSeparator!=tDefault.m_sFieldSeparator )	tOut.Appendf ( "field_separator='%s'",	m_sFieldSeparator.cstr() );
	if ( m_sStripMode!=tDefault.m_sStripMode )				tOut.Appendf ( "strip_mode='%s'",		m_sStripMode.cstr() );
	if ( m_iLimit!=tDefault.m_iLimit )						tOut.Appendf ( "limit=%d",				m_iLimit );
	if ( m_iLimitWords!=tDefault.m_iLimitWords )			tOut.Appendf ( "limit_words=%d",		m_iLimitWords );
	if ( m_iLimitPassages!=tDefault.m_iLimitPassages )		tOut.Appendf ( "limit_passages=%d",		m_iLimitPassages );
	if ( m_iAround!=tDefault.m_iAround )					tOut.Appendf ( "around=%d",				m_iAround );
	if ( m_iPassageId!=tDefault.m_iPassageId )				tOut.Appendf ( "start_passage_id=%d",	m_iPassageId );
	if ( m_bUseBoundaries!=tDefault.m_bUseBoundaries )		tOut.Appendf ( "use_boundaries=%d",		m_bUseBoundaries ? 1 : 0 );
	if ( m_bWeightOrder!=tDefault.m_bWeightOrder )			tOut.Appendf ( "weight_order=%d",		m_bWeightOrder ? 1 : 0 );
	if ( m_bForceAllWords!=tDefault.m_bForceAllWords )		tOut.Appendf ( "force_all_words=%d",	m_bForceAllWords ? 1 : 0 );
	if ( m_bAllowEmpty!=tDefault.m_bAllowEmpty )			tOut.Appendf ( "allow_empty=%d",		m_bAllowEmpty ? 1 : 0 );
	if ( m_bEmitZones!=tDefault.m_bEmitZones )				tOut.Appendf ( "emit_zones=%d",			m_bEmitZones ? 1 : 0 );
	if ( m_bForcePassages!=tDefault.m_bForcePassages )		tOut.Appendf ( "force_passages=%d",		m_bForcePassages ? 1 : 0 );
	if ( m_bJsonQuery!=tDefault.m_bJsonQuery )				tOut.Appendf ( "json_query=%d",			m_bJsonQuery ? 1 : 0 );
	if ( m_ePassageSPZ!=tDefault.m_ePassageSPZ )			tOut.Appendf ( "passage_boundary='%s'",	PassageBoundarySz(m_ePassageSPZ) );
	if ( m_bPackFields!=tDefault.m_bPackFields )			tOut.Appendf ( "pack_fields=%d",		m_bPackFields ? 1 : 0 );

	if ( m_uFilesMode!=tDefault.m_uFilesMode )
	{
		if ( m_uFilesMode & 1 ) tOut << "load_files=1";
		if ( m_uFilesMode & 2 ) tOut << "load_files_scattered=1";
	}

	tOut.FinishBlock(false);
	return tOut.cstr();
}

/////////////////////////////////////////////////////////////////////////////

#define UINT32_MASK 0xffffffffUL
#define UINT16_MASK 0xffff


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
public:
	SnippetZoneChecker_c ( const CSphVector<ZonePacked_t> & dDocZones, const SmallStringHash_T<int> & hDocNames, const StrVec_t & dQueryZones )
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
			tZone.m_dStarts.Add ( uPos );
			tZone.m_dEnds.Add ( uClosePos );
		}

#ifndef NDEBUG
		ARRAY_FOREACH ( i, m_dZones )
		{
			const ZoneHits_t & tZone = m_dZones[i];
			assert ( tZone.m_dStarts.GetLength()==tZone.m_dEnds.GetLength() );
			const Hitpos_t * pHit = tZone.m_dStarts.Begin()+1;
			const Hitpos_t * pMax = tZone.m_dStarts.Begin()+tZone.m_dStarts.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
			pHit = tZone.m_dEnds.Begin()+1;
			pMax = tZone.m_dEnds.Begin()+tZone.m_dEnds.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
		}
#endif
	}

	SphZoneHit_e IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan ) final
	{
		DWORD uPosWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
		int iOpen = FindSpan ( m_dZones[iZone].m_dStarts, uPosWithField );
		if ( pLastSpan )
			* pLastSpan = iOpen;
		return ( iOpen>=0 && uPosWithField<=m_dZones[iZone].m_dEnds[iOpen] ) ? SPH_ZONE_FOUND : SPH_ZONE_NO_SPAN;
	}

private:
	CSphVector<ZoneHits_t> m_dZones;
};

//////////////////////////////////////////////////////////////////////////

/// snippets query words for different cases
class SnippetsFastQword_c : public ISphQword
{
public:
	explicit SnippetsFastQword_c ( const CSphVector<DWORD> * pHits )
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

	const CSphMatch & GetNextDoc() override
	{
		m_dQwordFields.SetAll();
		m_tMatch.m_tRowID = m_tMatch.m_tRowID==INVALID_ROWID && HasHits() ? 0 : INVALID_ROWID;
		return m_tMatch;
	}

	Hitpos_t GetNextHit () override
	{
		if ( !HasHits() )
			return EMPTY_HIT;

		DWORD uPosition = *( m_pHits->Begin() + m_uMatchHits++ );
		return HITMAN::Create ( HITMAN::GetField(uPosition), HITMAN::GetPos(uPosition), (m_uLastPos==uPosition) );
	}

	void SeekHitlist ( SphOffset_t ) override {}

private:
	const CSphVector<DWORD> *	m_pHits;
	CSphMatch					m_tMatch;
	DWORD						m_uLastPos;
};


/// snippets query word setup
class SnippetsFastQwordSetup_c : public ISphQwordSetup
{
public:
	explicit SnippetsFastQwordSetup_c ( const SnippetsDocIndex_c & tIndex )
		: m_tIndex ( tIndex )
	{}

	ISphQword * QwordSpawn ( const XQKeyword_t & tWord ) const final
	{
		return new SnippetsFastQword_c ( m_tIndex.GetHitlist ( tWord, m_pDict ) );
	}

	bool QwordSetup ( ISphQword * pQword ) const final
	{
		SnippetsFastQword_c * pWord = (SnippetsFastQword_c *)pQword;
		pWord->Setup ( m_tIndex.GetLastPos() );
		return true;
	}

private:
	const SnippetsDocIndex_c & m_tIndex;
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


struct ScopedStreamers_t
{
public:
	CSphVector<CacheStreamer_i*> m_dStreamers;

	~ScopedStreamers_t()
	{
		for ( auto & i : m_dStreamers )
			SafeDelete(i);
	}
};


//////////////////////////////////////////////////////////////////////////
class SnippetBuilder_c : public SnippetBuilder_i
{
public:
	bool				Setup ( const CSphIndex * pIndex, const SnippetQuerySettings_t & tQuery, CSphString & sError ) final;
	bool				SetQuery ( const CSphString & sQuery, bool bIgnoreFields, CSphString & sError ) final;
	bool				Build ( TextSource_i * pSource, SnippetResult_t & tRes ) const final;
	CSphVector<BYTE>	PackResult ( SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const final;

private:
	struct ZoneData_t
	{
		CSphVector<ZonePacked_t>	m_dZones;
		FunctorZoneInfo_t			m_tInfo;
	};

	struct WeightedPassage_t
	{
		int	m_iId = 0;
		int	m_iWeight = 0;
	};

	struct WeightedPassageSort_fn
	{
		bool IsLess ( const WeightedPassage_t & a, const WeightedPassage_t  & b ) const;
	};

	const CSphIndex *				m_pIndex = nullptr;
	const SnippetQuerySettings_t *	m_pQuerySettings = nullptr;
	CSphScopedPtr<CSphHTMLStripper> m_pStripper { nullptr };
	CSphScopedPtr<QueryParser_i>	m_pQueryParser { nullptr };
	TokenizerRefPtr_c				m_pTokenizer;
	TokenizerRefPtr_c				m_pTokenizerJson;
	TokenizerRefPtr_c				m_pQueryTokenizer;
	DictRefPtr_c					m_pDict;
	CSphScopedPtr<XQQuery_t>		m_pExtQuery { nullptr };
	DWORD							m_eExtQuerySPZ = SPH_SPZ_NONE;
	FieldFilterRefPtr_c				m_pFieldFilter;

	bool							CheckSettings ( TextSource_i * pSource, CSphString & sError ) const;
	const CSphHTMLStripper *		GetStripperForText() const;
	const CSphHTMLStripper *		GetStripperForTokenization() const;

	bool							DoHighlighting ( TextSource_i & tSource, SnippetResult_t & tRes ) const;

	void							ExtractPassages ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, SnippetQuerySettings_t & tFixedSettings, const SnippetsDocIndex_c & tContainer,
										const CSphVector<SphHitMark_t> & dMarked, PassageContext_t & tPassageContext, DWORD uFoundWords, SnippetResult_t & tRes ) const;

	void							HighlightPassages ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, CSphVector<Passage_t> & dPassages, const SnippetQuerySettings_t & tFixedSettings,
										const CSphVector<SphHitMark_t> & dMarked, const FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes ) const;

	void							HighlightAll ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, const SnippetQuerySettings_t & tQuery, const CSphVector<SphHitMark_t> & dMarked, SnippetResult_t & tRes ) const;
	void							HighlightDocStart ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, const SnippetQuerySettings_t & tQuery, SnippetResult_t & tRes ) const;

	bool							MarkHits ( const SnippetsDocIndex_c & tContainer, CSphVector<SphHitMark_t> & dMarked, const ZoneData_t & tZoneData, SnippetResult_t & tRes ) const;
	void							SplitSpans ( const SnippetsDocIndex_c & tContainer, CSphVector<SphHitMark_t> & dMarked ) const;
	void							FoldHitsIntoSpans ( CSphVector<SphHitMark_t> & dMarked ) const;
	void							FixupQueryLimits ( const SnippetsDocIndex_c & tContainer, SnippetQuerySettings_t & tFixedSettings, CSphString & sWarning, DWORD uFoundTerms ) const;
	bool							CanHighlightAll ( const SnippetQuerySettings_t & tFixedSettings, int iDocLen ) const;
	bool							SetupStripperSPZ ( bool bSetupSPZ, CSphString & sError );

	void							GetPassageOrder ( const FieldResult_t & tField, CSphVector<WeightedPassage_t> & dPassageOrder ) const;
	void							PackAsData ( MemoryWriter_c & tWriter, SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const;
	void							PackAsString ( MemoryWriter_c & tWriter, CSphVector<BYTE> & dRes, SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const;
};


void SnippetBuilder_c::ExtractPassages ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, SnippetQuerySettings_t & tFixedSettings, const SnippetsDocIndex_c & tContainer, const CSphVector<SphHitMark_t> & dMarked,
	PassageContext_t & tPassageContext, DWORD uFoundWords, SnippetResult_t & tRes ) const
{
	assert ( m_pIndex && m_pQuerySettings );
	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();

	ARRAY_FOREACH ( i, tStreamers.m_dStreamers )
	{
		const char * szDoc = (const char*)tSource.GetText(i).Begin();
		int iDocLen = tSource.GetText(i).GetLength();

		CSphScopedPtr<PassageExtractor_i> pExtractor ( CreatePassageExtractor ( tContainer, tPassageContext, m_pTokenizer, tFixedSettings, tIndexSettings, szDoc, iDocLen, dMarked, uFoundWords, i, tRes ) );
		tStreamers.m_dStreamers[i]->Tokenize ( *pExtractor.Ptr() );

		if ( i==tStreamers.m_dStreamers.GetLength()-1 )
		{
			pExtractor->Finalize();
			tFixedSettings.m_bWeightOrder = pExtractor->GetWeightOrder();		// might have changed in Finalize()
		}
	}
}


void SnippetBuilder_c::HighlightPassages ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, CSphVector<Passage_t> & dPassages, const SnippetQuerySettings_t & tFixedSettings,
	const CSphVector<SphHitMark_t> & dMarked, const FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes ) const
{
	if ( !dPassages.GetLength() )
	{
		HighlightDocStart ( tStreamers, tSource, tFixedSettings, tRes );
		return;
	}

	ARRAY_FOREACH ( iField, tStreamers.m_dStreamers )
	{
		const char * szDoc = (const char*)tSource.GetText(iField).Begin();
		int iDocLen = tSource.GetText(iField).GetLength();

		CSphVector<Passage_t*> dFilteredPassages;
		ARRAY_FOREACH ( iPassage, dPassages )
			if ( dPassages[iPassage].m_iField==iField )
				dFilteredPassages.Add ( &dPassages[iPassage] );

		if ( !dFilteredPassages.GetLength() )
			continue;

		CSphScopedPtr<TokenFunctor_i> pHighlighter ( CreatePassageHighlighter ( dFilteredPassages, m_pTokenizer, tFixedSettings, m_pIndex->GetSettings(), szDoc, iDocLen, dMarked, tZoneInfo, iField, tRes ) );
		tStreamers.m_dStreamers[iField]->Tokenize ( *pHighlighter.Ptr() );
	}	
}


bool SnippetBuilder_c::MarkHits ( const SnippetsDocIndex_c & tContainer, CSphVector<SphHitMark_t> & dMarked, const ZoneData_t & tZoneData, SnippetResult_t & tRes ) const
{
	const XQQuery_t & tXQQuery = tContainer.GetQuery();

	SnippetZoneChecker_c tZoneChecker ( tZoneData.m_dZones, tZoneData.m_tInfo.m_hZones, tXQQuery.m_dZones );

	SnippetsFastQwordSetup_c tQwordSetup ( tContainer );
	tQwordSetup.SetDict ( m_pDict );
	tQwordSetup.m_pWarning = &tRes.m_sError;
	tQwordSetup.m_pZoneChecker = &tZoneChecker;

	// got a lot of stack allocated variables (up to 30K)
	// check that query not overflow stack here
	if ( !sphCheckQueryHeight ( tXQQuery.m_pRoot, tRes.m_sError ) )
		return false;

	CSphScopedPtr<CSphHitMarker> pMarker ( CSphHitMarker::Create ( tXQQuery.m_pRoot, tQwordSetup ) );
	if ( !pMarker.Ptr() )
		return false;

	pMarker->Mark(dMarked);

	return true;
}


void SnippetBuilder_c::SplitSpans ( const SnippetsDocIndex_c & tContainer, CSphVector<SphHitMark_t> & dMarked ) const
{
	const CSphVector<CSphVector<DWORD>> & dDocHits = tContainer.GetDocHits();

	// we just collected matching spans into dMarked, but!
	// certain spans might not match all words within the span
	// for instance, (one NEAR/3 two) could return a 5-word span
	// but we do have full matching keywords list in tContainer
	// so let's post-process and break down such spans
	// FIXME! what about phrase spans vs stopwords? they will be split now
	ARRAY_FOREACH ( i, dMarked )
	{
		if ( dMarked[i].m_uSpan==1 )
			continue;

		CSphVector<int> dMatched;
		for ( int j=0; j<(int)dMarked[i].m_uSpan; j++ )
		{
			// OPTIMZE? we can premerge all dochits vectors once
			const int iPos = dMarked[i].m_uPosition + j;
			for ( const auto & dDocHit : dDocHits )
				if ( dDocHit.BinarySearch(iPos) )
				{
					dMatched.Add(iPos);
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
}


void SnippetBuilder_c::FoldHitsIntoSpans ( CSphVector<SphHitMark_t> & dMarked ) const
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
			*++pOut = *pIn++;
	}

	if ( dMarked.GetLength()>1 )
		dMarked.Resize ( pOut - dMarked.Begin() + 1 );
}


void SnippetBuilder_c::HighlightAll ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, const SnippetQuerySettings_t & tQuery, const CSphVector<SphHitMark_t> & dMarked, SnippetResult_t & tRes ) const
{
	assert ( m_pIndex && m_pQuerySettings );
	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();

	ARRAY_FOREACH ( i, tStreamers.m_dStreamers )
	{
		const char * szDoc = (const char*)tSource.GetText(i).Begin();
		int iDocLen = tSource.GetText(i).GetLength();

		CSphScopedPtr<TokenFunctor_i> pHighlighter ( CreateQueryHighlighter ( m_pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, dMarked, i, tRes ) );
		tStreamers.m_dStreamers[i]->Tokenize ( *pHighlighter.Ptr() );
	}
}


void SnippetBuilder_c::HighlightDocStart ( ScopedStreamers_t & tStreamers, TextSource_i & tSource, const SnippetQuerySettings_t & tQuery, SnippetResult_t & tRes ) const
{
	if ( tQuery.m_bAllowEmpty )
		return;

	int iResultLenCP = 0;
	ARRAY_FOREACH ( i, tStreamers.m_dStreamers )
	{
		const char * szDoc = (const char*)tSource.GetText(i).Begin();
		int iDocLen = tSource.GetText(i).GetLength();

		CSphScopedPtr<DocStartHighlighter_i> pHighlighter ( CreateDocStartHighlighter ( m_pTokenizer, tQuery, m_pIndex->GetSettings(), szDoc, iDocLen, iResultLenCP, i, tRes ) );
		tStreamers.m_dStreamers[i]->Tokenize ( *pHighlighter.Ptr() );

		// did we collect enough tokens?
		if ( pHighlighter->CollectionStopped() )
			break;
	}
}


bool SnippetBuilder_c::CanHighlightAll ( const SnippetQuerySettings_t & tFixedSettings, int iDocLen ) const
{
	bool bHighlightAll = ( tFixedSettings.m_iLimit==0 || tFixedSettings.m_iLimit>=iDocLen ) && ( tFixedSettings.m_iLimitWords==0 || tFixedSettings.m_iLimitWords>iDocLen/2 )
		&& tFixedSettings.m_ePassageSPZ==SPH_SPZ_NONE;

	// might need separate passages
	if ( bHighlightAll && tFixedSettings.m_bForcePassages && ( tFixedSettings.m_iLimit!=0 || tFixedSettings.m_iLimitWords!=0 || tFixedSettings.m_iLimitPassages!=0 ) )
		bHighlightAll = false;

	return bHighlightAll;
}


bool SnippetBuilder_c::DoHighlighting ( TextSource_i & tSource, SnippetResult_t & tRes ) const
{
	assert ( m_pIndex && m_pQuerySettings );

	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();
	const SnippetQuerySettings_t & tQuerySettings = *m_pQuerySettings;

	SnippetQuerySettings_t tFixedSettings ( tQuerySettings );

	bool bRetainHtml = ( tFixedSettings.m_sStripMode=="retain" );

	// adjust tokenizer for markup-retaining mode
	if ( bRetainHtml )
		m_pTokenizer->AddSpecials ( "<" );

	// create query and hit lists container, parse query
	SnippetsDocIndex_c tContainer ( *m_pExtQuery.Ptr() );
	tContainer.ParseQuery ( m_pQueryTokenizer, m_pDict, m_eExtQuerySPZ );

	ScopedStreamers_t tStreamers;

	int iTotalDocLen = 0;
	for ( int iField = 0; iField < tSource.GetNumFields(); iField++ )
		iTotalDocLen += tSource.GetText(iField).GetLength();

	bool bHighlightAll = CanHighlightAll ( tFixedSettings, iTotalDocLen );
	int iSPZ = ConvertSPZ ( m_eExtQuerySPZ | ( bHighlightAll ? 0 : tFixedSettings.m_ePassageSPZ ) );

	ZoneData_t tZodeData;

	tRes.m_dFields.Resize ( tSource.GetNumFields() );

	DWORD uFoundWords = 0;
	for ( int iField = 0; iField < tSource.GetNumFields(); iField++ )
	{
		tRes.m_dFields[iField].m_sName = tSource.GetFieldName(iField);
		tRes.m_dFields[iField].m_dPassages.Resize(1);	// so that non-passage funcs will have something to work on

		const char * szDoc = (const char*)tSource.GetText(iField).Begin();
		int iDocLen = tSource.GetText(iField).GetLength();

		const CSphHTMLStripper * pStripper = GetStripperForTokenization();

		// do two passes over document
		// 1st pass will tokenize document, match keywords, and store positions into docindex
		// 2nd pass will highlight matching positions only (with some matching engine aid)

		CacheStreamer_i * pStreamer = CreateCacheStreamer(iDocLen);
		tStreamers.m_dStreamers.Add(pStreamer);

		CSphScopedPtr<HitCollector_i> pHitCollector ( CreateHitCollector ( tContainer, m_pTokenizer, m_pDict, tFixedSettings, tIndexSettings, szDoc, iDocLen, iField, *pStreamer, tZodeData.m_dZones, tZodeData.m_tInfo, tRes ) );
		TokenizeDocument ( *pHitCollector.Ptr(), pStripper, iSPZ );

		uFoundWords |= pHitCollector->GetFoundWords();
	}

	for ( auto & i : tStreamers.m_dStreamers )
		i->SetZoneInfo ( tZodeData.m_tInfo );

	FixupQueryLimits ( tContainer, tFixedSettings, tRes.m_sWarning, uFoundWords );

	CSphVector<SphHitMark_t> dMarked;
	if ( !MarkHits ( tContainer, dMarked, tZodeData, tRes ) )
	{
		HighlightDocStart ( tStreamers, tSource, tFixedSettings, tRes );
		return true;
	}

	SplitSpans ( tContainer, dMarked );
	FoldHitsIntoSpans(dMarked);

	if ( bHighlightAll )
		HighlightAll ( tStreamers, tSource, tFixedSettings, dMarked, tRes );
	else if ( !dMarked.GetLength() )
		HighlightDocStart ( tStreamers, tSource, tFixedSettings, tRes );
	else
	{
		PassageContext_t tPassageContext;
		ExtractPassages ( tStreamers, tSource, tFixedSettings, tContainer, dMarked, tPassageContext, uFoundWords, tRes );
		HighlightPassages ( tStreamers, tSource, tPassageContext.m_dPassages, tFixedSettings, dMarked, tZodeData.m_tInfo, tRes );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

class StringSourceTraits_c
{
public:
	void PrepareText ( const VecTraits_T<BYTE> & dSourceText, CSphVector<BYTE> & dDestText,  ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, bool & bUseOriginal ) const;
};


void StringSourceTraits_c::PrepareText ( const VecTraits_T<BYTE> & dSourceText, CSphVector<BYTE> & dDestText,  ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, bool & bUseOriginal ) const
{
	if ( !pFilter && !pStripper )
		return;

	if ( pFilter )
	{
		BYTE * pSource = dSourceText.Begin();
		int iSourceLen = dSourceText.GetLength();
		CSphVector<BYTE> dTmp;
		if ( !bUseOriginal )
		{
			dTmp.Resize ( dDestText.GetLength() );
			memcpy ( dTmp.Begin(), dDestText.Begin(), dDestText.GetLength() );
			pSource = dTmp.Begin();
			iSourceLen = dTmp.GetLength();
		}

		int iGot = pFilter->Apply ( pSource, iSourceLen, dDestText, false );
		if ( iGot )
		{
			dDestText.Resize(iGot);
			bUseOriginal = false;
		}
	}

	if ( pStripper )
	{
		if ( bUseOriginal )
		{
			dDestText.Resize ( dSourceText.GetLength() );
			memcpy ( dDestText.Begin(), dSourceText.Begin(), dSourceText.GetLength() );
			bUseOriginal = false;
		}

		if ( dDestText.GetLength() && dDestText.Last()!='\0' )
			dDestText.Add('\0');

		if ( dDestText.GetLength() )
		{
			pStripper->Strip ( dDestText.Begin() );
			dDestText.Resize ( (int) strlen ( (const char*)dDestText.Begin() ) );
		}
	}
}


//////////////////////////////////////////////////////////////////////////

class TextSourceString_c : public TextSource_i, public StringSourceTraits_c
{
public:
								TextSourceString_c() {}
								TextSourceString_c ( const VecTraits_T<const BYTE> & dString );

	bool						PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError ) override;
	VecTraits_T<BYTE>			GetText ( int iField ) const final;
	int							GetNumFields() const final { return 1; }
	const char *				GetFieldName ( int iField ) const final { return ""; }
	bool						TextFromIndex() const final;

protected:
	VecTraits_T<BYTE> 			m_dSourceText;			// this holds pointer to original text
	CSphVector<BYTE>			m_dBuffer;				// this holds text modified by filters/stripper (if any)
	bool						m_bUseOriginal = true;	// whether to use original or modified text
};


TextSourceString_c::TextSourceString_c ( const VecTraits_T<const BYTE> & dString )
	: m_dSourceText ( dString )
{}


bool TextSourceString_c::PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError )
{
	StringSourceTraits_c::PrepareText ( m_dSourceText, m_dBuffer, pFilter, pStripper, m_bUseOriginal );
	return true;
}


VecTraits_T<BYTE> TextSourceString_c::GetText ( int iField ) const
{
	assert ( !iField );

	if ( m_bUseOriginal )
		return m_dSourceText;

	return m_dBuffer;
}


bool TextSourceString_c::TextFromIndex() const
{
	return false;
}


//////////////////////////////////////////////////////////////////////////

class TextSourceFile_c : public TextSourceString_c
{
public:
					TextSourceFile_c ( const VecTraits_T<const BYTE> & dFilename );

	bool			PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError ) final;

private:
	CSphString		m_sFile;

	bool			LoadFile ( CSphString & sError );
};


TextSourceFile_c::TextSourceFile_c ( const VecTraits_T<const BYTE> & dFilename )
{
	m_sFile.SetBinary ( (const char*)dFilename.Begin(), dFilename.GetLength() );
}


bool TextSourceFile_c::LoadFile ( CSphString & sError )
{
	CSphAutofile tFile;
	CSphString sFilename;
	sFilename.SetSprintf ( "%s%s", g_sSnippetsFilePrefix.cstr(), m_sFile.scstr() );

	if ( !TestEscaping ( g_sSnippetsFilePrefix, sFilename ) )
	{
		sError.SetSprintf( "File '%s' escapes '%s' scope", sFilename.scstr(), g_sSnippetsFilePrefix.scstr());
		return false;
	}

	if ( m_sFile.IsEmpty() )
	{
		sError.SetSprintf ( "snippet file name is empty" );
		return false;
	}

	if ( !sFilename.IsEmpty () && tFile.Open ( sFilename.cstr(), SPH_O_READ, sError )<0 )
		return false;

	// will this ever trigger? time will tell; email me if it does!
	if ( tFile.GetSize()+1>=(SphOffset_t)INT_MAX )
	{
		sError.SetSprintf ( "%s too big for snippet (over 2 GB)", sFilename.cstr() );
		return false;
	}

	auto iFileSize = (int)tFile.GetSize();
	if ( iFileSize<0 )
		return false;

	m_dBuffer.Resize(iFileSize);
	if ( !tFile.Read ( m_dBuffer.Begin(), iFileSize, sError ) )
		return false;

	m_bUseOriginal = false;

	return true;
}


bool TextSourceFile_c::PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError )
{
	if ( !LoadFile(sError) )
		return false;

	return TextSourceString_c::PrepareText ( pFilter, pStripper, sError );
}


//////////////////////////////////////////////////////////////////////////
class TextSourceFields_c : public TextSource_i, public StringSourceTraits_c
{
public:
						TextSourceFields_c ( const CSphVector<FieldSource_t> & dAllFields );

	bool				PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError ) final;
	VecTraits_T<BYTE>	GetText ( int iField ) const final;
	int					GetNumFields() const final { return m_dFields.GetLength(); }
	const char *		GetFieldName ( int iField ) const final { return m_dFields[iField].m_sName.cstr(); }
	bool				TextFromIndex() const final { return true; }

private:
	const CSphVector<FieldSource_t> &	m_dFields;
	CSphVector<CSphVector<BYTE>>		m_dModifiedFields;
	CSphBitvec							m_tUseOriginal;
};


TextSourceFields_c::TextSourceFields_c ( const CSphVector<FieldSource_t> & dAllFields )
	: m_dFields ( dAllFields )
	, m_dModifiedFields ( dAllFields.GetLength() )
	, m_tUseOriginal ( dAllFields.GetLength() )
{
	m_tUseOriginal.Set();
}


bool TextSourceFields_c::PrepareText ( ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, CSphString & sError )
{
	ARRAY_FOREACH ( i, m_dFields )
	{
		bool bUseOriginal = true;
		StringSourceTraits_c::PrepareText ( m_dFields[i].m_dData, m_dModifiedFields[i], pFilter, pStripper, bUseOriginal );
		if ( !bUseOriginal )
			m_tUseOriginal.BitClear(i);
	}

	return true;
}


VecTraits_T<BYTE> TextSourceFields_c::GetText ( int iField ) const
{
	if ( m_tUseOriginal.BitGet(iField) )
		return m_dFields[iField].m_dData;

	return m_dModifiedFields[iField];
}


//////////////////////////////////////////////////////////////////////////

static TextSource_i * CreateTextSourceFile ( const VecTraits_T<const BYTE> & dFilename )
{
	return new TextSourceFile_c(dFilename);
}


static TextSource_i * CreateTextSourceString ( const VecTraits_T<const BYTE> & dString )
{
	return new TextSourceString_c(dString);
}


TextSource_i * CreateSnippetSource ( DWORD uFilesMode, const BYTE * pSource, int iLen )
{
	if ( uFilesMode )
		return CreateTextSourceFile ( VecTraits_T<const BYTE>(pSource, iLen) );

	return CreateTextSourceString ( VecTraits_T<const BYTE>(pSource, iLen) );
}


TextSource_i * CreateHighlightSource ( const CSphVector<FieldSource_t> & dAllFields )
{
	return new TextSourceFields_c ( dAllFields );
}


SnippetBuilder_i * CreateSnippetBuilder()
{
	return new SnippetBuilder_c;
}

/////////////////////////////////////////////////////////////////////////////

bool SnippetBuilder_c::CheckSettings ( TextSource_i * pSource, CSphString & sError ) const
{
	assert ( pSource && m_pQuerySettings );

	const SnippetQuerySettings_t & tOpt = *m_pQuerySettings;

	if ( tOpt.m_sStripMode=="retain" && !( tOpt.m_iLimit==0 && tOpt.m_iLimitPassages==0 && tOpt.m_iLimitWords==0 ) )
	{
		sError = "html_strip_mode=retain requires that all limits are zero";
		return false;
	}

	return true;
}


const CSphHTMLStripper * SnippetBuilder_c::GetStripperForText() const
{
	assert(m_pQuerySettings);

	if ( m_pQuerySettings->m_sStripMode=="strip" || m_pQuerySettings->m_sStripMode=="index" )
		return m_pStripper.Ptr();

	return nullptr;
}


const CSphHTMLStripper * SnippetBuilder_c::GetStripperForTokenization() const
{
	assert(m_pQuerySettings);

	if ( m_pQuerySettings->m_sStripMode!="retain" )
		return nullptr;

	return m_pStripper.Ptr();
}


void SnippetBuilder_c::FixupQueryLimits ( const SnippetsDocIndex_c & tContainer, SnippetQuerySettings_t & tFixedSettings, CSphString & sWarning, DWORD uFoundTerms ) const
{
	assert(m_pQuerySettings);
	const SnippetQuerySettings_t & tQuerySettings = *m_pQuerySettings;

	if ( tQuerySettings.m_iLimitWords && tQuerySettings.m_bForceAllWords )
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
		if ( tQuerySettings.m_bForceAllWords )
			iNewLimit = iTotalLen+nTermsUsed-1;

		if ( iNewLimit > tQuerySettings.m_iLimit )
		{
			sWarning.SetSprintf ( "query length (%d) is greater than the limit setting (%d): limit increased", iNewLimit, tQuerySettings.m_iLimit );
			tFixedSettings.m_iLimit = iNewLimit;
		}
	}
}



CSphString g_sSnippetsFilePrefix { "" };
bool SnippetBuilder_c::Build ( TextSource_i * pSource, SnippetResult_t & tRes ) const
{
	assert ( m_pIndex && m_pQuerySettings );

	if ( !CheckSettings ( pSource, tRes.m_sError ) )
		return false;

	assert ( pSource );
	if ( !pSource->PrepareText ( m_pFieldFilter, GetStripperForText(), tRes.m_sError ) )
		return false;
 
	assert ( pSource );
	DoHighlighting ( *pSource, tRes );

	return true;
}


void SnippetBuilder_c::PackAsData ( MemoryWriter_c & tWriter, SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const
{
	CSphVector<WeightedPassage_t> dPassageOrder;

	tWriter.ZipInt ( dRequestedFields.GetLength() );
	for ( auto i : dRequestedFields )
	{
		const FieldResult_t & tField = tRes.m_dFields[i];
		tWriter.PutString ( tField.m_sName );

		GetPassageOrder ( tField, dPassageOrder );
		tWriter.ZipInt ( tField.m_dPassages.GetLength() );
		for ( const auto & tWeighted : dPassageOrder )
		{
			PassageResult_t & tPassage = tField.m_dPassages[tWeighted.m_iId];

			tWriter.PutByte ( tPassage.m_bStartSeparator ? 1 : 0 );
			tWriter.PutByte ( tPassage.m_bEndSeparator ? 1 : 0 );
			tWriter.ZipInt ( tPassage.m_dText.GetLength() );
			tWriter.PutBytes ( tPassage.m_dText.Begin(), tPassage.m_dText.GetLength() );
		}
	}
}


bool SnippetBuilder_c::WeightedPassageSort_fn::IsLess ( const WeightedPassage_t & a, const WeightedPassage_t & b ) const
{
	if ( a.m_iWeight==b.m_iWeight )
		return a.m_iId<b.m_iId;

	return a.m_iWeight>b.m_iWeight;
}


void SnippetBuilder_c::GetPassageOrder ( const FieldResult_t & tField, CSphVector<WeightedPassage_t> & dPassageOrder ) const
{
	dPassageOrder.Resize ( tField.m_dPassages.GetLength() );

	ARRAY_FOREACH ( i, tField.m_dPassages )
	{
		dPassageOrder[i].m_iId = i;
		dPassageOrder[i].m_iWeight = tField.m_dPassages[i].m_iWeight;
	}

	assert(m_pQuerySettings);
	if ( m_pQuerySettings->m_bWeightOrder )
		dPassageOrder.Sort ( WeightedPassageSort_fn() );
}


void SnippetBuilder_c::PackAsString ( MemoryWriter_c & tWriter, CSphVector<BYTE> & dRes, SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const
{
	if ( tRes.m_dFields.GetLength()==1 && tRes.m_dFields[0].m_dPassages.GetLength()==1 && !tRes.m_dFields[0].m_dPassages[0].m_bStartSeparator && !tRes.m_dFields[0].m_dPassages[0].m_bEndSeparator )
	{
		dRes = std::move ( tRes.m_dFields[0].m_dPassages[0].m_dText );
		return;
	}

	assert(m_pQuerySettings);
	const SnippetQuerySettings_t & tOpts = *m_pQuerySettings;

	CSphVector<WeightedPassage_t> dPassageOrder;

	int iChunkSeparatorLen = tOpts.m_sChunkSeparator.Length();
	int iFieldSeparatorLen = tOpts.m_sFieldSeparator.Length();

	ARRAY_FOREACH ( i, dRequestedFields )
	{
		if ( dRes.GetLength() )
		{
			bool bHasMoreData = false;
			for ( int j = i; j < dRequestedFields.GetLength(); j++ )
			{
				const FieldResult_t & tNextField = tRes.m_dFields[dRequestedFields[j]];
				if ( tNextField.m_dPassages.GetLength() )
				{
					bHasMoreData = true;
					break;
				}
			}

			if ( bHasMoreData )
				tWriter.PutBytes ( m_pQuerySettings->m_sFieldSeparator.cstr(), iFieldSeparatorLen );
		}

		const FieldResult_t & tField = tRes.m_dFields[dRequestedFields[i]];
		GetPassageOrder ( tField, dPassageOrder );
		ARRAY_FOREACH ( iWeighted, dPassageOrder )
		{
			PassageResult_t & tPassage = tField.m_dPassages[dPassageOrder[iWeighted].m_iId];

			if ( !iWeighted && tPassage.m_bStartSeparator )
				tWriter.PutBytes ( tOpts.m_sChunkSeparator.cstr(), iChunkSeparatorLen );

			tWriter.PutBytes ( tPassage.m_dText.Begin(), tPassage.m_dText.GetLength() );

			if ( iWeighted<dPassageOrder.GetLength()-1 || tPassage.m_bEndSeparator )
				tWriter.PutBytes ( tOpts.m_sChunkSeparator.cstr(), iChunkSeparatorLen );
		}
	}
}


CSphVector<BYTE> SnippetBuilder_c::PackResult ( SnippetResult_t & tRes, const CSphVector<int> & dRequestedFields ) const
{
	assert(m_pQuerySettings);

	// 1st pass: remove redundant empty passages (if any)
	for ( auto & tField : tRes.m_dFields )
		if ( tField.m_dPassages.GetLength()==1 && !tField.m_dPassages[0].m_dText.GetLength() )
			tField.m_dPassages.Resize(0);

	CSphVector<BYTE> dRes;
	MemoryWriter_c tWriter(dRes);

	if ( m_pQuerySettings->m_bPackFields )
		PackAsData ( tWriter, tRes, dRequestedFields );
	else
		PackAsString ( tWriter, dRes, tRes, dRequestedFields );

	return dRes;
}


static DWORD CollectQuerySPZ ( const XQNode_t * pNode )
{
	if ( !pNode )
		return SPH_SPZ_NONE;

	DWORD eSPZ = SPH_SPZ_NONE;
	if ( pNode->GetOp ()==SPH_QUERY_SENTENCE )
		eSPZ |= SPH_SPZ_SENTENCE;
	else if ( pNode->GetOp ()==SPH_QUERY_PARAGRAPH )
		eSPZ |= SPH_SPZ_PARAGRAPH;

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		eSPZ |= CollectQuerySPZ ( pNode->m_dChildren[i] );

	return eSPZ;
}


bool SnippetBuilder_c::SetupStripperSPZ ( bool bSetupSPZ, CSphString & sError )
{
	m_pStripper.Reset();

	if ( bSetupSPZ && ( !m_pTokenizer->EnableSentenceIndexing(sError) || !m_pTokenizer->EnableZoneIndexing(sError) ) )
		return false;

	assert ( m_pIndex && m_pQuerySettings );
	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();
	const SnippetQuerySettings_t & q = *m_pQuerySettings;

	if ( q.m_sStripMode=="strip" || q.m_sStripMode=="retain" || ( q.m_sStripMode=="index" && tIndexSettings.m_bHtmlStrip ) )
	{
		// don't strip HTML markup in 'retain' mode - proceed zones only
		m_pStripper = new CSphHTMLStripper ( q.m_sStripMode!="retain" );

		if ( q.m_sStripMode=="index" )
		{
			if ( !m_pStripper->SetIndexedAttrs ( tIndexSettings.m_sHtmlIndexAttrs.cstr (), sError ) ||
				!m_pStripper->SetRemovedElements ( tIndexSettings.m_sHtmlRemoveElements.cstr (), sError ) )
			{
				sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr () );
				return false;
			}
		}

		if ( bSetupSPZ )
			m_pStripper->EnableParagraphs();

		// handle zone(s) in special mode only when passage_boundary enabled
		if ( bSetupSPZ && !m_pStripper->SetZones ( tIndexSettings.m_sZones.cstr (), sError ) )
		{
			sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr () );
			return false;
		}
	}

	return true;
}


bool SnippetBuilder_c::Setup ( const CSphIndex * pIndex, const SnippetQuerySettings_t & tSettings, CSphString & sError )
{
	assert(pIndex);
	assert(!m_pQuerySettings);

	m_pIndex = pIndex;
	m_pQuerySettings = &tSettings;
	m_pDict = GetStatelessDict ( pIndex->GetDictionary () );

	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();

	// OPTIMIZE! do a lightweight indexing clone here
	if ( tIndexSettings.m_uAotFilterMask )
		m_pTokenizer = sphAotCreateFilter ( TokenizerRefPtr_c ( pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX ) ), m_pDict, tIndexSettings.m_bIndexExactWords, tIndexSettings.m_uAotFilterMask );
	else
		m_pTokenizer = pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX );

	m_pQueryTokenizer = pIndex->GetQueryTokenizer()->Clone ( SPH_CLONE_QUERY_LIGHTWEIGHT );

	// setup exact dictionary if needed
	if ( tIndexSettings.m_bIndexExactWords )
		m_pDict = new CSphDictExact(m_pDict);

	if ( tSettings.m_bJsonQuery )
	{
		bool bWordDict = m_pDict->GetSettings().m_bWordDict;
		m_pTokenizerJson = pIndex->GetQueryTokenizer()->Clone ( SPH_CLONE_QUERY );
		sphSetupQueryTokenizer ( m_pTokenizerJson, pIndex->IsStarDict ( bWordDict ), tIndexSettings.m_bIndexExactWords, true );
		m_pQueryParser = sphCreateJsonQueryParser();
	}
	else
		m_pQueryParser = sphCreatePlainQueryParser();

	if ( pIndex->GetFieldFilter() )
		m_pFieldFilter = pIndex->GetFieldFilter()->Clone();

	return true;
}


bool SnippetBuilder_c::SetQuery ( const CSphString & sQuery, bool bIgnoreFields, CSphString & sError )
{
	assert(m_pIndex);
	assert(m_pQuerySettings);

	CSphVector<BYTE> dFiltered;
	const BYTE * szModifiedQuery = (BYTE *)sQuery.cstr();
	if ( m_pFieldFilter && szModifiedQuery )
	{
		if ( m_pFieldFilter->Apply ( szModifiedQuery, (int) strlen ( (char*)szModifiedQuery ), dFiltered, true ) )
			szModifiedQuery = dFiltered.Begin();
	}

	m_pExtQuery.Reset();
	m_pExtQuery = new XQQuery_t;

	const CSphIndexSettings & tIndexSettings = m_pIndex->GetSettings();

	// OPTIMIZE? double lightweight clone here? but then again it's lightweight
	if ( !m_pQueryParser->ParseQuery ( *m_pExtQuery.Ptr(), (const char*)szModifiedQuery, nullptr, m_pQueryTokenizer, m_pTokenizerJson, &m_pIndex->GetMatchSchema(), m_pDict, tIndexSettings ) )
	{
		sError = m_pExtQuery->m_sParseError;
		return false;
	}

	if ( bIgnoreFields && m_pExtQuery->m_pRoot )
		m_pExtQuery->m_pRoot->ClearFieldMask();

	m_eExtQuerySPZ = SPH_SPZ_NONE;
	m_eExtQuerySPZ |= CollectQuerySPZ ( m_pExtQuery->m_pRoot );
	if ( m_pExtQuery->m_dZones.GetLength () )
		m_eExtQuerySPZ |= SPH_SPZ_ZONE;

	if ( tIndexSettings.m_uAotFilterMask )
		TransformAotFilter ( m_pExtQuery->m_pRoot, m_pDict->GetWordforms(), tIndexSettings );

	bool bSetupSPZ = m_pQuerySettings->m_ePassageSPZ!=SPH_SPZ_NONE || m_eExtQuerySPZ!=SPH_SPZ_NONE || m_pQuerySettings->m_sStripMode=="retain";

	return SetupStripperSPZ ( bSetupSPZ, sError );
}


// check whether filepath from sPath does not escape area of sPrefix
bool TestEscaping( const CSphString& sPrefix, const CSphString& sPath )
{
	if ( sPrefix.IsEmpty() || sPrefix==sPath )
		return true;
	auto sNormalized = sphNormalizePath( sPath );
	return sPrefix==sNormalized.SubString( 0, sPrefix.Length());
}


ESphSpz GetPassageBoundary ( const CSphString & sPassageBoundaryMode )
{
	if ( sPassageBoundaryMode.IsEmpty() )
		return SPH_SPZ_NONE;

	ESphSpz eSPZ = SPH_SPZ_NONE;
	if ( sPassageBoundaryMode=="sentence" )
		eSPZ = SPH_SPZ_SENTENCE;
	else if ( sPassageBoundaryMode=="paragraph" )
		eSPZ = SPH_SPZ_PARAGRAPH;
	else if ( sPassageBoundaryMode=="zone" )
		eSPZ = SPH_SPZ_ZONE;

	return eSPZ;
}


const char * PassageBoundarySz ( ESphSpz eBoundary )
{
	switch ( eBoundary )
	{
	case SPH_SPZ_SENTENCE: return "sentence";
	case SPH_SPZ_PARAGRAPH: return "paragraph";
	case SPH_SPZ_ZONE: return "zone";
	default: return "";
	}
}


bool sphCheckOptionsSPZ ( const SnippetQuerySettings_t & q, ESphSpz eMode, CSphString & sError )
{
	if ( q.m_ePassageSPZ )
	{
		if ( q.m_iAround==0 )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and around=%d", PassageBoundarySz(eMode), q.m_iAround );
			return false;
		} else if ( q.m_bUseBoundaries )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and use_boundaries", PassageBoundarySz(eMode) );
			return false;
		}
	}

	if ( q.m_bEmitZones )
	{
		if ( q.m_ePassageSPZ!=SPH_SPZ_ZONE )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and emit_zones", PassageBoundarySz(eMode) );
			return false;
		}
		if ( !( q.m_sStripMode=="strip" || q.m_sStripMode=="index" ) )
		{
			sError.SetSprintf ( "invalid combination of strip=%s and emit_zones", q.m_sStripMode.cstr() );
			return false;
		}
	}

	return true;
}


void UnpackSnippetData ( const BYTE * pData, int iLength, SnippetResult_t & tRes )
{
	if ( !iLength )
		return;

	MemoryReader_c tReader ( pData, iLength );

	tRes.m_dFields.Resize ( tReader.UnzipInt() );
	for ( auto & tField : tRes.m_dFields )
	{
		tField.m_sName = tReader.GetString();
		tField.m_dPassages.Resize ( tReader.UnzipInt() );
		for ( auto & tPassage : tField.m_dPassages )
		{
			tPassage.m_bStartSeparator = !!tReader.GetByte();
			tPassage.m_bEndSeparator = !!tReader.GetByte();

			tPassage.m_dText.Resize ( tReader.UnzipInt() );
			tReader.GetBytes ( tPassage.m_dText.Begin(), tPassage.m_dText.GetLength() );
		}
	}
}
