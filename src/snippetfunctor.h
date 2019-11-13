//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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

#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxquery.h"
#include "sphinxsearch.h"

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
	int					m_iField;			///< field id

	CSphVector<StoredExcerptToken_t> m_dBeforeTokens;	///< stored tokens before match
	CSphVector<StoredExcerptToken_t> m_dAfterTokens;	///< stored tokens after match

	void		Reset();
	void		CopyData ( Passage_t & tPassage );
	inline int	GetWeight () const;
};


struct PassageContext_t
{
	DWORD					m_uPassagesQwords = 0;
	int						m_dQwordWeights[32];
	CSphVector<int>			m_dTopPassageWeights;
	CSphVector<Passage_t>	m_dPassages;

	PassageContext_t();
};


typedef uint64_t ZonePacked_t;
struct SnippetQuerySettings_t;
struct SnippetResult_t;
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


class DocStartHighlighter_i : public TokenFunctor_i
{
public:
	virtual bool	CollectionStopped() const = 0;
};


class PassageExtractor_i : public TokenFunctor_i
{
public:
	virtual bool	GetWeightOrder() const = 0;
	virtual void	Finalize() = 0;
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


DocStartHighlighter_i *	CreateDocStartHighlighter ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, 
	int & iResultLenCP, int iField, SnippetResult_t & tRes );

TokenFunctor_i *		CreateQueryHighlighter ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes );

PassageExtractor_i *	CreatePassageExtractor ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tContext, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, DWORD uFoundWords, int iField, SnippetResult_t & tRes );

TokenFunctor_i *		CreatePassageHighlighter ( CSphVector<Passage_t*> & dPassages, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes );

HitCollector_i *		CreateHitCollector ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes );

#endif // _snippetfunctor_
