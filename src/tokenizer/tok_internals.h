//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
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
	bool AddRange ( CSphRemapRange tRange, CSphVector<CSphRemapRange>& dRanges );

public:
	bool Parse ( const char* sConfig, CSphVector<CSphRemapRange>& dRanges );
	CSphString GetLastError();
	static const CSphVector<CharsetAlias_t>& GetCharsetAliases();
	static bool InitCharsetAliasTable ( CSphString& sError );
};

void MergeIntersectedRanges ( CSphVector<CSphRemapRange>& dRanges );
bool AddRange ( CSphRemapRange tRange, CSphVector<CSphRemapRange>& dRanges, CSphString* pError = nullptr );