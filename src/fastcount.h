// Copyright (c) 2026, Manticore Software LTD. GPLv2 or later.
#pragma once

#include "sphinxsearch.h"
#include "sphinxsort.h"
#include "postings_container_reader.h"

// All state belongs to one physical segment/rowid partition of the normal
// query snapshot. No match-level cache, ranker or result stream is involved.
struct FullTextCountContext_t
{
	RowID_t m_tMinRowID = 0;
	RowID_t m_tMaxRowID = INVALID_ROWID-1;
	bool m_bStatsExact = false;
	std::function<bool(RowID_t)> m_fnAlive;
	e1::TermView (*m_fnPrimaryView)(ISphQword&)=nullptr;
	bool *m_pPrimaryUsed=nullptr;
	// Disk E1-only experiment, supplied only for a pinned delete-free full range.
	// No qword virtual/layout change; the executor additionally proves all fields.
	bool (*m_fnE1Window) ( ISphQword &, RowID_t, uint64_t *, RowID_t & ) = nullptr;
};

bool CanUseFastFullTextCount ( const CSphQuery &, const XQNode_t *, const VecTraits_T<ISphMatchSorter*> & );
// Conservative pre-split proof: a single unexpanded term over every indexed field.
bool IsFullTextCountMetadataTerm ( const XQNode_t *, const ISphSchema & );
bool CountFullTextDocs ( const XQNode_t *, const ISphQwordSetup &, const FullTextCountContext_t &, uint64_t & );
void PushFullTextCount ( uint64_t, const VecTraits_T<ISphMatchSorter*> &, const CSphRowitem * pStatic, int iTag=0 );

inline void RecordFullTextCount ( CSphQueryResultMeta & tMeta )
{
	IteratorStats_t tStats;
	tStats.m_dIterators.Add ( { "fulltext", "FastCount" } );
	tStats.m_iTotal = 1;
	tMeta.m_tIteratorStats.Merge ( tStats );
}
