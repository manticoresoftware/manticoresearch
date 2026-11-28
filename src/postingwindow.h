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
#include <limits>

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
	// Returns the surviving bit count.
	virtual PostingWindowResult_t FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits ) = 0;

	// Output is sorted and unique. May stop at capacity before reaching end;
	// next identifies where to resume. Capacity must be positive.
	virtual PostingWindowResult_t ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) = 0;

	// Exact field-aware membership, unlike the conservative AdvanceTo candidate.
	// A successful probe does not consume the matching posting.
	virtual PostingProbeResult_t Probe ( RowID_t tRow ) = 0;

	// Caller resets/re-runs QwordSetup first; then discard this cursor's state.
	virtual void Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) = 0;
};

// Implements intersection and exclusion using a scratch bitmap populated by FillOr.
class PostingWindowBase_c : public PostingWindow_i
{
public:
	PostingWindowResult_t FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits ) override;
	PostingWindowResult_t FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits ) override;
};

std::unique_ptr<PostingWindow_i> CreateBufferedPostingWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext );
std::unique_ptr<PostingWindow_i> CreatePackedExpansionWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext );

// The current leaf remains available until Prepare advances into another leaf.
struct PostingLeaf_t
{
	RowID_t		m_tFirst = INVALID_ROWID;
	uint64_t	m_uEnd = 0;
	bool		m_bDecoded = false;
};

// Row and field columns for a decoded leaf. Array indices are posting ordinals.
struct DecodedPostingRows_t
{
	const uint32_t *	m_pRows = nullptr;
	const uint32_t *	m_pFields = nullptr;
	DWORD				m_uMask = 0;
	int					m_iBegin = 0;
	int					m_iEnd = 0;

	bool PassesFieldCheck ( int i ) const { return !m_pFields || (m_pFields[i] & m_uMask); }
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

	// Enumerate matching rows within the prepared leaf, retaining it for payload reads.
	// Handles decoded rows, bitmaps and ranges; output capacity must be at least ROWS.
	virtual int		ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows ) = 0;

	// Compact sorted candidates to exact field-aware matches in the prepared leaf.
	virtual int		FilterRows ( RowID_t * pRows, int iCount ) = 0;

	// Access the rows of a decoded leaf for intersection by posting ordinal.
	virtual DecodedPostingRows_t GetDecodedRows() const { return {}; }

	// Gather payloads for sorted surviving rows, including after Intersect/FilterRows.
	// Optional ordinals locate these rows in the prepared leaf.
	// Decode only requested columns; unrequested TF/hit references are zeroed.
	virtual bool	ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits, const uint16_t * pOrdinals=nullptr ) = 0;
};

// A conservative prefilter. May retain false positives
class PostingCandidateFilter_c
{
public:
	virtual ~PostingCandidateFilter_c() = default;

	virtual RowID_t	AdvanceTo ( RowID_t tMin ) = 0;
	virtual int		SelectRows ( RowID_t * pRows, int iCount ) = 0;

	void		SetFilterComplexity ( float fComplexity ) { m_fFilterComplexity = fComplexity; }
	float		GetFilterComplexity() const	{ return m_fFilterComplexity; }

	// Create a fresh cursor sharing the threshold and pruning state.
	virtual std::unique_ptr<PostingCandidateFilter_c> CloneCursor() { return nullptr; }

private:
	float		m_fFilterComplexity = std::numeric_limits<float>::infinity();
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
