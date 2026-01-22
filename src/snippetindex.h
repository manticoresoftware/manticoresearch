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

#ifndef _snippetindex_
#define _snippetindex_

#include "sphinxquery/sphinxquery.h"

//////////////////////////////////////////////////////////////////////////
/// mini-index for a single document
/// keeps query words
/// keeps hit lists for every query keyword
class SnippetsDocIndex_i
{
public:
	virtual ~SnippetsDocIndex_i() = default;

	virtual void	SetupHits() = 0;
	virtual int		FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const = 0;
	virtual void	AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition ) = 0;
	virtual void	ParseQuery ( const DictRefPtr_c& pDict, DWORD eExtQuerySPZ ) = 0;
	virtual int		GetTermWeight ( int iQueryPos ) const = 0;
	virtual int		GetNumTerms () const = 0;
	virtual DWORD	GetLastPos() const = 0;
	virtual void	SetLastPos ( DWORD uLastPos ) = 0;

	virtual const XQQuery_t	& GetQuery() const = 0;
	virtual const CSphVector<CSphVector<DWORD>> & GetDocHits() const = 0;
	virtual const CSphVector<DWORD> * GetHitlist ( const XQKeyword_t & tWord, const DictRefPtr_c & pDict ) const = 0;
};

SnippetsDocIndex_i * CreateSnippetsDocIndex ( const XQQuery_t & tQuery );


#endif // _snippetindex_
