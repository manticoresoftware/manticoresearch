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
#include <array>
#include <vector>

class ISphQword;
struct CSphQueryStats;
struct QwordDoc_t;

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
	int		m_iRows = 0;				// FillAnd/FillAndNot: surviving bits; FillOr/ReadRows: matching unique rows consumed
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
// their candidate; FillOr/FillAnd/FillAndNot/ReadRows consume rows. Methods may be mixed, but cannot
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

	// Remove exact matches in [min,end), clearing bits outside that interval.
	// EOF leaves remaining candidates intact; interruption clears the output.
	// Returns the surviving bit count. The default uses FillOr and a scratch bitmap.
	virtual PostingWindowResult_t FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits );

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

// The current leaf remains available until Prepare advances into another leaf.
struct PostingLeaf_t
{
	RowID_t		m_tFirst = INVALID_ROWID;
	uint64_t	m_uEnd = 0;
};

// Exclusive posting view for candidate intersection followed by lazy payload
// gathering. The qword must outlive it; only hit reads may use the qword directly.
class PostingBlock_i
{
public:
	virtual ~PostingBlock_i() = default;

	// Find a leaf containing a candidate at/after min. False means EOF, interruption
	// or I/O failure. Bounds may include rows rejected by the bound field mask.
	virtual bool	Prepare ( RowID_t tMin, PostingLeaf_t & tLeaf ) = 0;

	// Intersect within the prepared leaf without advancing to another leaf.
	// Its payloads remain available to ReadPayloads(). The window is at most
	// ROWS long; clear padding through the last accessed word.
	virtual int		Intersect ( const PostingWindow_t & tWindow, DWORD * pBits ) = 0;

	// Compact sorted candidates to exact field-aware matches in the prepared leaf.
	virtual int		FilterRows ( RowID_t * pRows, int iCount ) = 0;

	// Gather payloads for sorted surviving rows, including after Intersect/FilterRows.
	// Decode only requested columns; unrequested TF/hit references are zeroed.
	virtual bool	ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits ) = 0;
};

struct PostingCandidateRows_t
{
	const RowID_t *	m_pRows = nullptr;
	int				m_iCount = 0;
};

// Intersects leaf-bounded intervals, then holds every participating leaf until
// the returned rows' payloads have been gathered. Excluded terms only supply
// exact membership; sparse survivors use row probes.
class PostingBlockAnd_c
{
public:
	explicit PostingBlockAnd_c ( const PostingWindowContext_t & tContext );

	void					Add ( std::unique_ptr<PostingBlock_i> pCursor );
	void					AddExcluded ( std::unique_ptr<PostingWindow_i> pCursor );
	PostingBlock_i &		GetCursor ( int i )	{ return *m_dCursors[i]; }

	// Sorted rows in [min,end), valid (with their payloads) until the next call.
	PostingCandidateRows_t	ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity );

private:
	bool	FillCandidates ( uint64_t uEnd );
	void	ExtractRows ( RowID_t tBase, const DWORD * pBits, int iWords );
	bool	SubtractRows();

	std::vector<std::unique_ptr<PostingBlock_i>>	m_dCursors;
	std::vector<std::unique_ptr<PostingWindow_i>>	m_dExcluded;
	std::array<RowID_t,PostingWindow_t::ROWS>		m_dRows;

	PostingWindowContext_t	m_tContext;
	RowID_t					m_tNext = 0;
	int						m_iCount = 0;
	int						m_iRead = 0;
	bool					m_bDone = false;
};


// Counts distinct clause membership independently for every bit (up to 32 clauses).
class PostingQuorum_c
{
public:
	explicit PostingQuorum_c ( int iThreshold );

	void	Reset();
	void	Add ( const DWORD * pBits );
	bool	Finish ( DWORD * pBits ) const;

private:
	DWORD	m_dCounts[6][PostingWindow_t::WORDS];
	int		m_iThreshold;
	int		m_iTerms = 0;
};
