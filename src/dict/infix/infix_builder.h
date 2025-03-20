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

#pragma once

#include "std/ints.h"
#include "fileio.h"

static constexpr Str_t g_sTagInfixBlocks = FROMS ( "infix-blocks" );
static constexpr Str_t g_sTagInfixEntries = FROMS ( "infix-entries" );
static constexpr Str_t g_sTagDictHeader = FROMS ( "dict-header" );

struct InfixBlock_t
{
	union {
		const char* m_sInfix;
		DWORD m_iInfixOffset;
	};
	DWORD m_iOffset;
};


/// infix hash builder
class ISphInfixBuilder
{
public:
	explicit ISphInfixBuilder() {}
	virtual ~ISphInfixBuilder() {}
	virtual void AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) = 0;
	virtual void SaveEntries ( CSphWriter& wrDict ) = 0;
	virtual int64_t SaveEntryBlocks ( CSphWriter& wrDict ) = 0;
	virtual int GetBlocksWordsSize() const = 0;
};

std::unique_ptr<ISphInfixBuilder> sphCreateInfixBuilder ( int iCodepointBytes, CSphString* pError );
bool sphLookupInfixCheckpoints ( const char* sInfix, int iBytes, const BYTE* pInfixes, const CSphVector<InfixBlock_t>& dInfixBlocks, int iInfixCodepointBytes, CSphVector<DWORD>& dCheckpoints );