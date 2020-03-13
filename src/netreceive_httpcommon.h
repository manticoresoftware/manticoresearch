//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#define NET_STATE_HTTP_SIZE 4000

struct HttpHeaderStreamParser_t
{
	int m_iHeaderEnd;
	int m_iFieldContentLenStart;
	int m_iFieldContentLenVal;

	int m_iCur;
	int m_iCRLF;
	int m_iName;

	HttpHeaderStreamParser_t ();
	bool HeaderFound ( const BYTE * pBuf, int iLen );
};

class ThdJobHttp_c final : public ISphJob
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	ThdJobHttp_c ( CSphNetLoop * pLoop, NetStateAPI_t * pState, bool bSsl );
	~ThdJobHttp_c () final;
	void		Call () final;
};