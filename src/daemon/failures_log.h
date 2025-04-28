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

#pragma once

#include "std/string.h"
#include "std/stringbuilder.h"

struct SearchFailure_t
{
	CSphString m_sParentIndex;
	CSphString m_sIndex; ///< searched index name
	CSphString m_sError; ///< search error message
};

class SearchFailuresLog_c
{
	CSphVector<SearchFailure_t> m_dLog;

public:
	void Submit ( const CSphString & sIndex, const char * sParentIndex, const char * sError );

	void SubmitVa ( const char * sIndex, const char * sParentIndex, const char * sTemplate, va_list ap );


	void Append ( const SearchFailuresLog_c & rhs )
	{
		m_dLog.Append ( rhs.m_dLog );
	}


	void SubmitEx ( const char * sIndex, const char * sParentIndex, const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 4, 5 ) ) );

	void SubmitEx ( const CSphString & sIndex, const char * sParentIndex, const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 4, 5 ) ) );


	bool IsEmpty () const noexcept
	{
		return m_dLog.GetLength() == 0;
	}


	int GetReportsCount () const noexcept
	{
		return m_dLog.GetLength();
	}


	void BuildReport ( StringBuilder_c & sReport );
};