//
// Copyright (c) 2008-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxstd.h"
#include "remap_range.h"

struct CharsetAlias_t
{
	CSphString m_sName;
	int m_iNameLen;
	CSphVector<CSphRemapRange> m_dRemaps;
};

// used in ICU
const CSphVector<CharsetAlias_t>& GetCharsetAliases();

bool sphInitCharsetAliasTable ( CSphString& sError );

/// parse charset table
bool sphParseCharset ( const char* szCharset, CSphVector<CSphRemapRange>& dRemaps, CSphString* pError = nullptr );
