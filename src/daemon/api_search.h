//
// Copyright (c) 2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "searchdha.h"

class SearchRequestBuilder_c final : public RequestBuilder_i
{
	const VecTraits_T<CSphQuery> & m_dQueries;
	const int m_iDivideLimits;

public:
	NONCOPYMOVABLE ( SearchRequestBuilder_c );


	SearchRequestBuilder_c ( const VecTraits_T<CSphQuery> & dQueries, int iDivideLimits )
		: m_dQueries ( dQueries ), m_iDivideLimits ( iDivideLimits )
	{
	}


	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const;

private:
	void SendQuery ( const char * sIndexes, ISphOutputBuffer & tOut, const CSphQuery & q, int iWeight ) const;
};


class SearchReplyParser_c final : public ReplyParser_i
{
	int m_iResults;

public:
	NONCOPYMOVABLE ( SearchReplyParser_c );


	explicit SearchReplyParser_c ( int iResults )
		: m_iResults ( iResults )
	{
	}


	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const;
};

struct cSearchResult final : iQueryResult
{
	CSphVector<AggrResult_t> m_dResults;

	void Reset ();

	bool HasWarnings () const;
};