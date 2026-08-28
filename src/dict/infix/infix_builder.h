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

#include "std/ints.h"
#include "std/blobs.h"
#include "std/vector.h"
#include "indexsettings.h"
#include <memory>

static constexpr Str_t g_sTagInfixBlocks = FROMS ( "infix-blocks" );
static constexpr Str_t g_sTagInfixEntries = FROMS ( "infix-entries" );
static constexpr Str_t g_sTagDictHeader = FROMS ( "dict-header" );
static constexpr Str_t g_sTagDictHeaderV2 = FROMS ( "dict-header-v2" );
static constexpr DWORD INFIX_CHECKPOINT_ID_MAX = 0x00ffffffUL;

struct InfixBlock_t
{
	union {
		const char* m_sInfix;
		int64_t m_iInfixOffset;
	};
	SphOffset_t m_iOffset;
};

class CSphWriter;

/// infix hash builder
class ISphInfixBuilder
{
public:
	explicit ISphInfixBuilder() {}
	virtual ~ISphInfixBuilder() {}
	virtual void AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) = 0;
	virtual void SaveEntries ( CSphWriter& wrDict ) = 0;
	virtual int64_t SaveEntryBlocks ( CSphWriter& wrDict ) = 0;
	virtual int64_t GetBlocksWordsSize() const = 0;
};

std::unique_ptr<ISphInfixBuilder> sphCreateInfixBuilder ( int iCodepointBytes, CSphString* pError, DictFormat_e eDictFormat = DictFormat_e::KEYWORDS );
bool sphLookupInfixCheckpoints ( const char* sInfix, int iBytes, const BYTE* pInfixes, const CSphVector<InfixBlock_t>& dInfixBlocks, int iInfixCodepointBytes, CSphVector<DWORD>& dCheckpoints, DictFormat_e eDictFormat = DictFormat_e::KEYWORDS );
