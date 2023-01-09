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

#ifndef _snippetpassage_
#define _snippetpassage_

#include "sphinxstd.h"


struct StoredExcerptToken_t
{
	int				m_iLengthCP;
	BYTE			m_iWordFlag;
};


struct Passage_t
{
	int		m_iStart;			///< start token index
	int		m_iTokens;			///< token count
	int		m_iCodes;			///< codepoints count
	int		m_iWords;			///< words count
	DWORD	m_uQwords;			///< matching query words mask
	int		m_iQwordsWeight;	///< passage weight factor
	int		m_iQwordCount;		///< passage weight factor
	int		m_iUniqQwords;		///< passage weight factor
	int		m_iMaxLCS;			///< passage weight factor
	int		m_iMinGap;			///< passage weight factor
	int		m_iStartLimit;		///< start of match in passage
	int		m_iEndLimit;		///< end of match in passage
	int		m_iAroundBefore;	///< words before first matched words
	int		m_iAroundAfter;		///< words after last matched words
	int		m_iCodesBetweenKeywords;	///< codepoints between m_iStartLimit and m_iEndLimit
	int		m_iWordsBetweenKeywords;	///< words between m_iStartLimit and m_iEndLimit
	int		m_iField;			///< field id

	CSphVector<StoredExcerptToken_t> m_dBeforeTokens;	///< stored tokens before match
	CSphVector<StoredExcerptToken_t> m_dAfterTokens;	///< stored tokens after match

	void	Reset();
	void	CopyData ( Passage_t & tPassage );
	int		GetWeight () const;
};


struct SnippetQuerySettings_t;
struct SnippetLimits_t;
class SnippetsDocIndex_c;

struct PassageContext_t
{
	DWORD					m_uPassagesQwords = 0;
	int						m_dQwordWeights[32];
	CSphVector<int>			m_dTopPassageWeights;
	CSphVector<Passage_t>	m_dPassages;

							PassageContext_t();

	CSphVector<Passage_t>	SelectBest ( const SnippetLimits_t & tLimits, const SnippetQuerySettings_t & tSettings, const SnippetsDocIndex_c & tContainer, DWORD uFoundWords ) const;
};


#endif // _snippetpassage_
