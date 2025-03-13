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

#include "std/ints.h"
#include "sphinxdefs.h"

struct SkipCacheKey_t
{
	int64_t m_iIndexId;
	SphWordID_t m_tWordId;
};

struct SkipData_t;

void InitSkipCache ( int64_t iCacheSize );
void ShutdownSkipCache();

namespace SkipCache
{
	void DeleteAll ( int64_t iIndexId );
	void Release ( SkipCacheKey_t tKey );
	bool Find ( SkipCacheKey_t tKey, SkipData_t * & pData );
	bool Add ( SkipCacheKey_t tKey, SkipData_t* pData );
}