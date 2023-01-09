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

#ifndef _snippetfunctor_
#define _snippetfunctor_

#include "sphinxstd.h"
#include "sphinxquery.h"
#include "sphinxsearch.h"
#include "sphinxdefs.h"

struct TokenInfo_t
{
	int				m_iStart;
	int				m_iLen;
	const BYTE *	m_sWord;
	DWORD			m_uPosition;
	bool			m_bWord;
	bool			m_bStopWord;
	int				m_iTermIndex;
	int				m_iMultiPosLen;

	SphWordID_t		m_uWordId;
};


struct FunctorZoneInfo_t
{
	SmallStringHash_T<int>	m_hZones;
	CSphVector<int>			m_dZonePos;
	CSphVector<int>			m_dZoneParent;
};


typedef uint64_t ZonePacked_t;
struct SnippetQuerySettings_t;
struct SnippetResult_t;
struct Passage_t;
struct PassageContext_t;
class CacheStreamer_i;

class TokenFunctor_i
{
public:
	virtual			~TokenFunctor_i() {}

	virtual bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * pMultiPosDelta ) = 0;
	virtual bool	OnOverlap ( int iStart, int iLen, int iBoundary ) = 0;
	virtual void	OnSkipHtml ( int iStart, int iLen ) = 0;
	virtual void	OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * szZone, int iZone ) = 0;
	virtual void	OnTail ( int iStart, int iLen, int iBoundary ) = 0;
	virtual void	OnFinish() = 0;
};


class SnippetsDocIndex_c;

class HitCollector_i : public TokenFunctor_i
{
public:
	virtual DictRefPtr_c &				GetDict() = 0;
	virtual TokenizerRefPtr_c &			GetTokenizer() = 0;
	virtual const CSphIndexSettings &	GetIndexSettings() = 0;
	virtual const SnippetQuerySettings_t & GetSnippetQuery() = 0;

	virtual CSphVector<ZonePacked_t> & 	GetZones() = 0;
	virtual FunctorZoneInfo_t &			GetZoneInfo() = 0;
	virtual bool						NeedExtraZoneInfo() const = 0;

	virtual DWORD						GetFoundWords() const = 0;
};


std::unique_ptr<TokenFunctor_i> CreateDocStartHighlighter ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, int & iResultCP, SnippetResult_t & tRes );

std::unique_ptr<TokenFunctor_i> CreateQueryHighlighter ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes );

std::unique_ptr<TokenFunctor_i> CreatePassageExtractor ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tContext, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits,
	const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes );

std::unique_ptr<TokenFunctor_i> CreatePassageHighlighter ( CSphVector<Passage_t*> & dPassages, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes );

std::unique_ptr<HitCollector_i> CreateHitCollector ( SnippetsDocIndex_c & tContainer, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes );

#endif // _snippetfunctor_
