//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "charset_definition_parser.h"

struct RemapRangeTagged_t: public CSphRemapRange
{
	int m_iTag = -1;

	RemapRangeTagged_t() = default;
	explicit RemapRangeTagged_t ( const CSphRemapRange& tRange )
		: CSphRemapRange { tRange }
	{}
};

// need a stable sort with the major tag first
inline bool operator< ( const RemapRangeTagged_t& a, const RemapRangeTagged_t& b )
{
	if ( a.m_iStart == b.m_iStart )
		return ( a.m_iTag > b.m_iTag );

	return ( a.m_iStart < b.m_iStart );
}


class CSphCharsetDefinitionParser
{
	StringBuilder_c m_sError;
	const char* m_pCurrent = nullptr;
	static CSphVector<CharsetAlias_t> m_dCharsetAliases;

	bool Error ( const char* szMessage );
	void SkipSpaces();
	bool IsEof();
	bool CheckEof();
	int ParseCharsetCode();
	bool AddRange ( CSphRemapRange tRange, CSphVector<RemapRangeTagged_t>& dRanges );

public:
	bool Parse ( const char* sConfig, CSphVector<CSphRemapRange>& dRanges );
	CSphString GetLastError();
	static const CSphVector<CharsetAlias_t>& GetCharsetAliases();
	static bool InitCharsetAliasTable ( CSphString& sError );
};

// used in tests
void MergeIntersectedRanges ( CSphVector<RemapRangeTagged_t>& dRanges );
bool AddRange ( CSphRemapRange tRange, CSphVector<RemapRangeTagged_t>& dRanges, CSphString* pError = nullptr );

struct GtQuotation_t
{
	static constexpr BYTE EscapingSpace ( BYTE c )
	{
		return ( c=='=' || c == '>' ) ? 1 : 0;
	}

	static void EscapeChar ( BYTE *& pOut, BYTE c )
	{
		if ( EscapingSpace ( c ) )
			*pOut++ = '\\';
		*pOut++ = c;
	}

	static void EscapeCharWithSpaces ( BYTE *& pOut, BYTE c )
	{
		if ( EscapingSpace ( c ) )
		{
			pOut[0] = '\\';
			pOut[1] = c;
			pOut += 2;
		} else
			*pOut++ = c;
	}
};

using GtEscapedBuilder = EscapedStringBuilder_T<GtQuotation_t>;
