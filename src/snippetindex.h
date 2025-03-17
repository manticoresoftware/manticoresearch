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

#ifndef _snippetindex_
#define _snippetindex_

#include "sphinxquery.h"

//////////////////////////////////////////////////////////////////////////
/// mini-index for a single document
/// keeps query words
/// keeps hit lists for every query keyword
class SnippetsDocIndex_c
{
	friend struct KeywordCmp_t;
	friend struct TermCmp_t;

public:
				SnippetsDocIndex_c ( const XQQuery_t & tQuery );

	void		SetupHits();
	int			FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const;
	int			FindStarred ( const char * sWord ) const;
	void		AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition );
	void		ParseQuery ( const DictRefPtr_c& pDict, DWORD eExtQuerySPZ );
	int			GetTermWeight ( int iQueryPos ) const;
	int			GetNumTerms () const;
	DWORD		GetLastPos() const { return m_uLastPos; }
	void		SetLastPos ( DWORD uLastPos ) { m_uLastPos=uLastPos; }

	const XQQuery_t	& GetQuery() const { return m_tQuery; }
	const CSphVector<CSphVector<DWORD>> & GetDocHits() const { return m_dDocHits; }
	const CSphVector<DWORD> * GetHitlist ( const XQKeyword_t & tWord, const DictRefPtr_c & pDict ) const;

private:
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

	DWORD							m_uLastPos;
	CSphVector<CSphVector<DWORD>>	m_dDocHits;
	const XQQuery_t	&				m_tQuery;

	CSphVector<Term_t>		m_dTerms;
	CSphVector<SphWordID_t>	m_dStarred;
	CSphVector<Keyword_t>	m_dStars;
	CSphVector<BYTE>		m_dStarBuffer;
	CSphVector<int>			m_dQposToWeight;

	mutable BYTE			m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];

	bool		MatchStar ( const Keyword_t & tTok, const BYTE * sWord ) const;
	void		AddWord ( SphWordID_t iWordID, int iLengthCP, int iQpos );
	void		AddWordStar ( const char * sWord, int iLengthCP, int iQpos );
	int			ExtractWords ( XQNode_t * pNode, const DictRefPtr_c& pDict, int iQpos );
};

#endif // _snippetindex_
