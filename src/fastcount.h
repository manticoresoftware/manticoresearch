//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxsearch.h"
#include "sphinxsort.h"

struct XQNode_t;
struct XQKeyword_t;
struct KeywordBuf_t;

// Fast path for `SELECT COUNT(*) FROM t WHERE MATCH('term')`: the count is taken from the per-term
// document counts stored in the dictionary instead of iterating the posting list.
// It is exact only when the chunk/segment has no killed rows and the term covers every indexed field.

/// query and sorter part of the eligibility check; does not depend on the parsed full-text tree
bool IsFastTermCountQuery ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs );

/// tree part: the single plain keyword whose field spec covers every indexed field, or nullptr
const XQKeyword_t * GetFastTermCountKeyword ( const XQNode_t * pRoot, const ISphSchema & tSchema );

/// normalize the keyword the way the ranker does and fill the qword ids (no readers, no setup)
void SetupFastCountQword ( ISphQword & tQword, const XQKeyword_t & tWord, const DictRefPtr_c & pDict, KeywordBuf_t & tBuf );

/// push one grouped match carrying a precomputed count into implicit-group sorters
void PushFastCountMatch ( const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphRowitem * pStatic, int iTag, int iIndexWeight, int64_t iCount );

/// record the fast path in the iterator stats (SHOW META 'index' row)
void RecordFastCount ( CSphQueryResultMeta & tMeta );
