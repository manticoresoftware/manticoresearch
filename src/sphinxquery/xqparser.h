//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxquery.h"

// a wrapper for sphParseExtendedQuery
std::unique_ptr<QueryParser_i> sphCreatePlainQueryParser();

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
/// lots of arguments here instead of simply the index pointer, because
/// a) we do not always have an actual real index class, and
/// b) might need to tweak stuff even we do
/// FIXME! remove either pQuery or sQuery
bool	sphParseExtendedQuery ( XQQuery_t & tQuery, const char * sQuery, const CSphQuery * pQuery, const TokenizerRefPtr_c& pTokenizer, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields );