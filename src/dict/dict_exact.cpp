//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "dict_proxy.h"

#include "sphinxint.h"

//////////////////////////////////////////////////////////////////////////

/// dict wrapper for exact-word syntax
class DictExact_c: public DictProxy_c
{
	using DictProxy_c::GetWordID;

public:
	explicit DictExact_c ( DictRefPtr_c pDict )
		: DictProxy_c ( std::move (pDict) )
	{}

	SphWordID_t GetWordID ( BYTE* pWord ) final;
};

SphWordID_t DictExact_c::GetWordID ( BYTE* pWord )
{
	auto iLen = (int)strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16 + 3 * SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	if ( pWord[0] == '=' )
		pWord[0] = MAGIC_WORD_HEAD_NONSTEMMED;

	if ( pWord[0] < ' ' )
		return m_pDict->GetWordIDNonStemmed ( pWord );

	return m_pDict->GetWordID ( pWord );
}


void SetupExactDict ( DictRefPtr_c& pDict )
{
	pDict = new DictExact_c ( pDict );
}