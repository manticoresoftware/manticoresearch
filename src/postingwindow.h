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

#include "sphinxdefs.h"
#include <memory>

class ISphQword;
struct CSphQueryStats;

struct PostingWindow_t
{
	static constexpr int ROWS = 4096;
	static constexpr int WORDS = ROWS/32;
	RowID_t		m_tBase;	// bit zero in the destination
	RowID_t		m_tMin;		// inclusive lower bound, at or above base
	uint64_t	m_uEnd;		// exclusive; widened for arithmetic near INVALID_ROWID
};

struct PostingWindowResult_t
{
	RowID_t m_tNext = INVALID_ROWID;	// first unread candidate; INVALID means EOF only when !m_bStopped
	int		m_iRows = 0;				// FillAnd: surviving bits; FillOr/ReadRows: matching unique rows consumed
	bool	m_bStopped = false;			// interrupted; output may be incomplete
};

struct PostingProbeResult_t
{
	PostingWindowResult_t	m_tState;
	bool					m_bMatch = false;
};

struct PostingWindowContext_t
{
	void * m_pContext = nullptr;
	bool (*m_fnContinue) ( void * ) = nullptr; // checked at entry and before refills
	CSphQueryStats * m_pStats = nullptr;
};

// Exclusive, count/membership-only view of an already set-up qword. The qword
// must outlive the cursor and must not be read separately while it is in use.
// All methods share retained lookahead and move forwards. AdvanceTo/Probe retain
// their candidate; FillOr/FillAnd/ReadRows consume rows. Methods may be mixed, but cannot
// revisit consumed rows. Candidates may fail the bound low-32-bit field mask.
class PostingWindow_i
{
public:
	virtual ~PostingWindow_i() = default;
	virtual PostingWindowResult_t AdvanceTo ( RowID_t tMin ) = 0;

	// OR into caller-owned storage without clearing it. On completion, next>=end
	// or EOF. The span end-base is at most ROWS; no padding is required.
	virtual PostingWindowResult_t FillOr ( const PostingWindow_t & tWindow, DWORD * pBits ) = 0;

	// Intersect with matching postings in [min,end), returning the surviving bit count.
	// Accesses ceil((end-base)/32) words; bits outside [min,end) in those words are cleared.
	// On completion, next>=end or EOF. Interrupted output/count cover only consumed postings.
	virtual PostingWindowResult_t FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits ) = 0;

	// Output is sorted and unique. May stop at capacity before reaching end;
	// next identifies where to resume. Capacity must be positive.
	virtual PostingWindowResult_t ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) = 0;

	// Exact field-aware membership, unlike the conservative AdvanceTo candidate.
	// A successful probe does not consume the matching posting.
	virtual PostingProbeResult_t Probe ( RowID_t tRow ) = 0;

	// Caller resets/re-runs QwordSetup first; then discard this cursor's state.
	virtual void Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) = 0;
};

std::unique_ptr<PostingWindow_i> CreateBufferedPostingWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext );
