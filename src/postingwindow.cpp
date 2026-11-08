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
class BufferedPostingWindow_c final : public PostingWindow_i
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

	PostingWindowResult_t FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits ) override
	{
		// Keep per-word intersection work out of the scalar posting loop.
		DWORD dScratch[PostingWindow_t::WORDS] = {};
		auto tResult = FillOr ( tWindow, dScratch );
		const int iWords = int ( (tWindow.m_uEnd-tWindow.m_tBase+31)/32 );
		bitmap::And ( pBits, pBits, dScratch, iWords );
		tResult.m_iRows = int ( bitmap::Count ( pBits, iWords ) );
		return tResult;
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
	QwordDoc_t				m_dBatch[128];
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
			m_iLength = m_tWord.GetNextDocs ( m_dBatch, 128 );
			m_bEOF = m_iLength<128;
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


PostingWindowResult_t PostingWindow_i::FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits )
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


PostingBlockAnd_c::PostingBlockAnd_c ( const PostingWindowContext_t & tContext ) : m_tContext(tContext) {}


void PostingBlockAnd_c::Add ( std::unique_ptr<PostingBlock_i> pCursor )
{
	m_dCursors.push_back(std::move(pCursor));
}


void PostingBlockAnd_c::AddExcluded ( std::unique_ptr<PostingWindow_i> pCursor )
{
	m_dExcluded.push_back(std::move(pCursor));
}


bool PostingBlockAnd_c::SubtractRows()
{
	for ( auto & pCursor : m_dExcluded )
	{
		int iOut = 0;
		for ( int i=0; i<m_iCount; ++i )
		{
			auto tResult = pCursor->Probe(m_dRows[i]);
			if ( tResult.m_tState.m_bStopped )
				return false;

			if ( !tResult.m_bMatch )
				m_dRows[iOut++] = m_dRows[i];
		}

		m_iCount = iOut;
	}

	return true;
}


void PostingBlockAnd_c::ExtractRows ( RowID_t tBase, const DWORD * pBits, int iWords )
{
	m_iCount = 0;
	for ( int i=0; i<iWords; ++i )
	{
		DWORD uBits = pBits[i];
		while ( uBits )
		{
			m_dRows[m_iCount++] = tBase+i*32+sphLog2(uBits & (DWORD(0)-uBits))-1;
			uBits &= uBits-1;
		}
	}
}


bool PostingBlockAnd_c::FillCandidates ( uint64_t uEnd )
{
	while ( m_tNext<uEnd )
	{
		if ( m_tContext.m_fnContinue && !m_tContext.m_fnContinue(m_tContext.m_pContext) )
			return false;

		RowID_t tFirst = m_tNext;
		uint64_t uLimit = uEnd;
		for ( auto & pCursor : m_dCursors )
		{
			PostingLeaf_t tLeaf;
			if ( !pCursor->Prepare ( tFirst, tLeaf ) )
				return false;

			tFirst = std::max ( tFirst, tLeaf.m_tFirst );
			uLimit = std::min ( uLimit, tLeaf.m_uEnd );
		}

		m_tNext = tFirst;
		if ( tFirst>=uLimit )
			continue;

		uLimit = std::min ( uLimit, uint64_t(tFirst)+PostingWindow_t::ROWS );

		PostingWindow_t tWindow { tFirst, tFirst, uLimit };
		int iWords = int((uLimit-tFirst+31)/32);
		DWORD dBits[PostingWindow_t::WORDS];
		std::fill_n ( dBits, iWords, UINT32_MAX );

		int iCount = int(uLimit-tFirst);
		int iThreshold = std::max ( 1, iCount/32 );
		size_t iTerm = 0;
		for ( ; iTerm<m_dCursors.size() && iCount>=iThreshold; ++iTerm )
			iCount = m_dCursors[iTerm]->Intersect ( tWindow, dBits );

		m_iCount = 0;
		if ( iCount>=iThreshold && iTerm==m_dCursors.size() )
		{
			for ( auto & pCursor : m_dExcluded )
			{
				auto tResult = pCursor->FillAndNot ( tWindow, dBits );
				if ( tResult.m_bStopped )
					return false;

				iCount = tResult.m_iRows;
				if ( !iCount )
					break;
			}

			if ( iCount )
				ExtractRows ( tFirst, dBits, iWords );
		} else if ( iCount )
		{
			ExtractRows ( tFirst, dBits, iWords );
			for ( ; iTerm<m_dCursors.size() && m_iCount; ++iTerm )
				m_iCount = m_dCursors[iTerm]->FilterRows ( m_dRows.data(), m_iCount );

			if ( !SubtractRows() )
				return false;
		}

		m_tNext = RowID_t(uLimit);
		m_iRead = 0;
		if ( m_iCount )
			return true;
	}

	return false;
}


PostingCandidateRows_t PostingBlockAnd_c::ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity )
{
	assert ( iCapacity>0 && !m_dCursors.empty() );
	uEnd = std::min ( uEnd, uint64_t(INVALID_ROWID) );
	m_tNext = std::max ( m_tNext, tMin );
	m_iRead = int(std::lower_bound ( m_dRows.begin()+m_iRead, m_dRows.begin()+m_iCount, tMin )-m_dRows.begin());
	if ( m_iRead==m_iCount && !m_bDone )
		m_bDone = !FillCandidates(uEnd);

	if ( m_bDone || m_iRead==m_iCount || m_dRows[m_iRead]>=uEnd )
		return {};

	int iCount = std::min ( iCapacity, m_iCount-m_iRead );
	while ( iCount && m_dRows[m_iRead+iCount-1]>=uEnd )
		--iCount;

	const RowID_t * pRows = m_dRows.data()+m_iRead;
	m_iRead += iCount;
	return { pRows, iCount };
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
