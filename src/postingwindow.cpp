//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "postingwindow.h"
#include "sphinxsearch.h"
#include "std/bitmap/bitmap.h"
#include <algorithm>

namespace
{
constexpr int POSTING_BATCH_SIZE = 128;


class BufferedPostingWindow_c final : public PostingWindowBase_c
{
public:
	BufferedPostingWindow_c ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
		: m_tWord(tWord)
		, m_uFields(uFields)
	{
		Reset ( tWord.m_iDocs!=0, tContext );
	}

	PostingWindowResult_t AdvanceTo ( RowID_t tMin ) override
	{
		if ( CheckContinue() )
			EnsureCandidate(tMin);

		return Result();
	}

	PostingWindowResult_t FillOr ( const PostingWindow_t & tWindow, DWORD * pBits ) override
	{
		// Window bounds already limit bitmap output to ROWS unique rows.
		return Consume<false> ( tWindow, PostingWindow_t::ROWS, [=] ( RowID_t tRow, int ) {
			const DWORD uOffset = tRow-tWindow.m_tBase;
			pBits[uOffset>>5] |= DWORD(1)<<(uOffset&31);
		} );
	}

	PostingWindowResult_t ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) override
	{
		assert ( iCapacity>0 );
		return Consume<true> ( tWindow, iCapacity, [=] ( RowID_t tRow, int i ) { pRows[i] = tRow; } );
	}

	PostingProbeResult_t Probe ( RowID_t tRow ) override
	{
		if ( CheckContinue() )
			while ( EnsureCandidate(tRow) && m_dBatch[m_iPos].m_tRowID==tRow )
			{
				if ( m_dBatch[m_iPos].m_uFields & m_uFields )
					return { Result(), true };

				++m_iPos; // field-rejected postings are not exact matches
			}
		return { Result(), false };
	}

	void Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) override
	{
		m_tContext = tContext;
		m_iPos = m_iLength = 0;
		m_bEOF = !bHasDocs;
		m_bStopped = false;
		m_tLastEmitted = INVALID_ROWID;
	}

private:
	ISphQword &				m_tWord;
	const DWORD				m_uFields;
	PostingWindowContext_t	m_tContext;
	QwordDoc_t				m_dBatch[POSTING_BATCH_SIZE];
	int						m_iPos = 0;
	int						m_iLength = 0;
	bool					m_bEOF = false;
	bool					m_bStopped = false;
	RowID_t					m_tLastEmitted = INVALID_ROWID;

	bool CheckContinue()
	{
		if ( !m_bStopped && m_tContext.m_fnContinue && !m_tContext.m_fnContinue(m_tContext.m_pContext) )
			m_bStopped = true;

		return !m_bStopped;
	}

	PostingWindowResult_t Result ( int iRows=0 ) const
	{
		return { m_iPos<m_iLength ? m_dBatch[m_iPos].m_tRowID : INVALID_ROWID, iRows, m_bStopped };
	}

	bool EnsureCandidate ( RowID_t tMin )
	{
		while ( !m_bStopped )
		{
			while ( m_iPos<m_iLength && m_dBatch[m_iPos].m_tRowID<tMin )
				++m_iPos;

			if ( m_iPos<m_iLength )
				return m_dBatch[m_iPos].m_tRowID!=INVALID_ROWID;

			if ( m_bEOF || !CheckContinue() )
				return false;

			// GetNextDocs positions the underlying qword at the LAST decoded row.
			// Never hint it until every retained row before the target is drained.
			if ( tMin && (!m_iLength || m_dBatch[m_iLength-1].m_tRowID<tMin) )
			{
				m_tWord.HintRowID(tMin);
				if ( m_tContext.m_pStats )
					++m_tContext.m_pStats->m_iSkips;
			}

			m_iPos = 0;
			m_iLength = m_tWord.GetNextDocs ( m_dBatch, POSTING_BATCH_SIZE );
			m_bEOF = m_iLength<POSTING_BATCH_SIZE;
			if ( m_tContext.m_pStats )
				m_tContext.m_pStats->m_iFetchedDocs += m_iLength;
		}

		return false;
	}

	template<bool LIMIT_ROWS, typename WRITE>
	PostingWindowResult_t Consume ( const PostingWindow_t & tWindow, int iCapacity, WRITE fnWrite )
	{
		assert ( tWindow.m_tBase<=tWindow.m_tMin && tWindow.m_tMin<=tWindow.m_uEnd );
		assert ( tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
		assert ( tWindow.m_uEnd<=uint64_t(INVALID_ROWID)+1 );

		if ( !CheckContinue() )
			return Result();

		int iRows = 0;

		// Exclude the EOF sentinel even when the widened window end is INVALID_ROWID+1.
		const RowID_t tEnd = RowID_t ( Min ( tWindow.m_uEnd, uint64_t(INVALID_ROWID) ) );
		const DWORD uFields = m_uFields;
		while ( EnsureCandidate(tWindow.m_tMin) )
		{
			// Sorted rows in this batch already meet the lower bound.
			// Publish cursor state only after draining the batch or retaining lookahead.
			const QwordDoc_t * pDoc = m_dBatch+m_iPos;
			const QwordDoc_t * pEnd = m_dBatch+m_iLength;
			RowID_t tLastEmitted = m_tLastEmitted;
			while ( pDoc<pEnd )
			{
				const auto & tDoc = *pDoc;
				const RowID_t tRow = tDoc.m_tRowID;
				if ( tRow>=tEnd )
					break;

				if ( !(tDoc.m_uFields & uFields) || tRow==tLastEmitted )
				{
					++pDoc;
					continue;
				}

				if constexpr ( LIMIT_ROWS )
					if ( iRows==iCapacity )
						break;

				fnWrite ( tRow, iRows++ );
				tLastEmitted = tRow;
				++pDoc;
			}

			m_iPos = int(pDoc-m_dBatch);
			m_tLastEmitted = tLastEmitted;
			if ( pDoc<pEnd )
				return Result(iRows);

			// Refill even at capacity: an empty batch alone does not mean EOF.
		}

		return Result(iRows);
	}
};


// Presents the matching rows from a packed expansion as one sorted posting stream.
class PackedExpansionWindow_c final : public PostingWindowBase_c
{
public:
							PackedExpansionWindow_c ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext );

	PostingWindowResult_t	AdvanceTo ( RowID_t tMin ) final;
	PostingWindowResult_t	FillOr ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) final;
	PostingProbeResult_t	Probe ( RowID_t tRow ) final;
	void					Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) final;

private:
	ISphQword &			m_tWord;			// Reads the constituent terms' doclists one after another.
	const DWORD			m_uFields;			// Query field mask. A posting qualifies if any selected field matches.
	std::vector<RowID_t> m_dRows;			// Sorted, unique row IDs matching the field mask after Reset completes.
	size_t				m_iPos = 0;			// Index of the first unread row in m_dRows.
	bool				m_bStopped = false;	// Reset was interrupted while reading the packed doclists.

	PostingWindowResult_t	Result ( int iRows=0 ) const;
};


PackedExpansionWindow_c::PackedExpansionWindow_c ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
	: m_tWord(tWord)
	, m_uFields(uFields)
{
	Reset ( tWord.m_iDocs!=0, tContext );
}


void PackedExpansionWindow_c::Reset ( bool bHasDocs, const PostingWindowContext_t & tContext )
{
	m_iPos = 0;
	m_bStopped = false;
	m_dRows.clear();
	if ( !bHasDocs )
		return;

	m_tWord.SetPayloadRequirements ( false, false );

	QwordDoc_t dBatch[POSTING_BATCH_SIZE];
	int iDocs;
	do
	{
		if ( tContext.m_fnContinue && !tContext.m_fnContinue(tContext.m_pContext) )
		{
			m_bStopped = true;
			return;
		}

		iDocs = m_tWord.GetNextDocs ( dBatch, POSTING_BATCH_SIZE );
		if ( tContext.m_pStats )
			tContext.m_pStats->m_iFetchedDocs += iDocs;

		for ( int i=0; i<iDocs; ++i )
			if ( dBatch[i].m_uFields & m_uFields )
				m_dRows.push_back ( dBatch[i].m_tRowID );
	}
	while ( iDocs==POSTING_BATCH_SIZE );

	// Each doclist is sorted, but the next one can start at a lower row ID. Sort for forward traversal.
	// Several expanded terms can match the same row. Keep one entry so ReadRows emits it once.
	std::sort ( m_dRows.begin(), m_dRows.end() );
	m_dRows.erase ( std::unique ( m_dRows.begin(), m_dRows.end() ), m_dRows.end() );
	if ( !m_dRows.empty() )
		m_tWord.m_iDocs = int(m_dRows.size()); // same field-filtered DF as the expanded payload node
}


PostingWindowResult_t PackedExpansionWindow_c::Result ( int iRows ) const
{
	return { m_iPos<m_dRows.size() ? m_dRows[m_iPos] : INVALID_ROWID, iRows, m_bStopped };
}


PostingWindowResult_t PackedExpansionWindow_c::AdvanceTo ( RowID_t tMin )
{
	if ( m_iPos<m_dRows.size() && m_dRows[m_iPos]<tMin )
		m_iPos = std::lower_bound ( m_dRows.begin()+m_iPos, m_dRows.end(), tMin )-m_dRows.begin();

	return Result();
}


PostingWindowResult_t PackedExpansionWindow_c::FillOr ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	AdvanceTo ( tWindow.m_tMin );

	const size_t iStart = m_iPos;
	while ( m_iPos<m_dRows.size() && m_dRows[m_iPos]<tWindow.m_uEnd )
	{
		const DWORD uBit = m_dRows[m_iPos++]-tWindow.m_tBase;
		pBits[uBit>>5] |= DWORD(1) << ( uBit & 31 );
	}

	return Result ( int(m_iPos-iStart) );
}


PostingWindowResult_t PackedExpansionWindow_c::ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity )
{
	AdvanceTo(tWindow.m_tMin);

	int iRows = 0;
	while ( iRows<iCapacity && m_iPos<m_dRows.size() && m_dRows[m_iPos]<tWindow.m_uEnd )
		pRows[iRows++] = m_dRows[m_iPos++];

	return Result(iRows);
}


PostingProbeResult_t PackedExpansionWindow_c::Probe ( RowID_t tRow )
{
	auto tResult = AdvanceTo(tRow);
	return { tResult, tRow!=INVALID_ROWID && tResult.m_tNext==tRow };
}
}


std::unique_ptr<PostingWindow_i> ISphQword::CreatePostingWindow ( DWORD uFields, const PostingWindowContext_t & tContext )
{
	return CreateBufferedPostingWindow ( *this, uFields, tContext );
}


std::unique_ptr<PostingWindow_i> CreateBufferedPostingWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
{
	tWord.SetPayloadRequirements ( false, false );
	return std::make_unique<BufferedPostingWindow_c> ( tWord, uFields, tContext );
}


std::unique_ptr<PostingWindow_i> CreatePackedExpansionWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
{
	return std::make_unique<PackedExpansionWindow_c> ( tWord, uFields, tContext );
}


PostingWindowResult_t PostingWindowBase_c::FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	// Keep per-word intersection work out of the scalar posting loop.
	DWORD dScratch[PostingWindow_t::WORDS] = {};
	auto tResult = FillOr ( tWindow, dScratch );
	const int iWords = int ( (tWindow.m_uEnd-tWindow.m_tBase+31)/32 );
	bitmap::And ( pBits, pBits, dScratch, iWords );
	tResult.m_iRows = int ( bitmap::Count ( pBits, iWords ) );
	return tResult;
}


PostingWindowResult_t PostingWindowBase_c::FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	DWORD dExcluded[PostingWindow_t::WORDS] {};
	auto tResult = FillOr ( tWindow, dExcluded );

	unsigned iWords = unsigned((tWindow.m_uEnd-tWindow.m_tBase+31)/32);
	bitmap::AndNot ( pBits, pBits, dExcluded, iWords );
	for ( unsigned i=0; i<iWords; ++i )
	{
		uint64_t uFirst = uint64_t(tWindow.m_tBase)+i*32;
		uint64_t uBegin = std::max ( uint64_t(tWindow.m_tMin), uFirst );
		uint64_t uEnd = std::min ( std::min ( tWindow.m_uEnd, uint64_t(INVALID_ROWID) ), uFirst+32 );
		if ( tResult.m_bStopped || uBegin>=uEnd )
			pBits[i] = 0;
		else
			pBits[i] &= (UINT32_MAX>>(32-unsigned(uEnd-uFirst))) & (UINT32_MAX<<unsigned(uBegin-uFirst));
	}

	tResult.m_iRows = int(bitmap::Count ( pBits, iWords ));
	return tResult;
}

PostingQuorum_c::PostingQuorum_c ( int iThreshold ) : m_iThreshold(iThreshold)
{
	assert ( iThreshold>=1 && iThreshold<=32 );
	Reset();
}


void PostingQuorum_c::Reset()
{
	m_iTerms = 0;
	for ( int i=0; i<(m_iThreshold==2 ? 2 : 6); ++i )
		std::fill_n ( m_dCounts[i], PostingWindow_t::WORDS, 0 );
}


void PostingQuorum_c::Add ( const DWORD * pBits )
{
	assert ( m_iTerms<32 );
	++m_iTerms;
	for ( int i=0; i<PostingWindow_t::WORDS; ++i )
	{
		DWORD uCarry = pBits[i];
		if ( m_iThreshold==2 )
		{
			// Saturating two-clause count: seen once, and seen at least twice.
			m_dCounts[1][i] |= m_dCounts[0][i] & uCarry;
			m_dCounts[0][i] |= uCarry;
		} else
			for ( int j=0; uCarry; ++j )
			{
				DWORD uNext = m_dCounts[j][i] & uCarry;
				m_dCounts[j][i] ^= uCarry;
				uCarry = uNext;
			}
	}
}


bool PostingQuorum_c::Finish ( DWORD * pBits ) const
{
	DWORD uAny = 0;
	for ( int i=0; i<PostingWindow_t::WORDS; ++i )
	{
		DWORD uMatch = m_dCounts[1][i];
		if ( m_iThreshold!=2 )
		{
			DWORD uEqual = UINT32_MAX;
			uMatch = 0;
			for ( int j=5; j>=0; --j )
				if ( m_iThreshold & (1<<j) )
					uEqual &= m_dCounts[j][i];
				else
				{
					uMatch |= uEqual & m_dCounts[j][i];
					uEqual &= ~m_dCounts[j][i];
				}

			uMatch |= uEqual;
		}

		pBits[i] = uMatch;
		uAny |= uMatch;
	}

	return uAny!=0;
}
