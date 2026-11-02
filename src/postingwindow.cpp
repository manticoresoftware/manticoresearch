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
		return Consume ( tWindow, PostingWindow_t::ROWS, [=] ( RowID_t tRow, int ) {
			const DWORD uOffset = tRow-tWindow.m_tBase;
			pBits[uOffset>>5] |= DWORD(1)<<(uOffset&31);
		} );
	}

	PostingWindowResult_t ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) override
	{
		assert ( iCapacity>0 );
		return Consume ( tWindow, iCapacity, [=] ( RowID_t tRow, int i ) { pRows[i] = tRow; } );
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

	template<typename WRITE>
	PostingWindowResult_t Consume ( const PostingWindow_t & tWindow, int iCapacity, WRITE fnWrite )
	{
		assert ( tWindow.m_tBase<=tWindow.m_tMin && tWindow.m_tMin<=tWindow.m_uEnd );
		assert ( tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
		assert ( tWindow.m_uEnd<=uint64_t(INVALID_ROWID)+1 );

		if ( !CheckContinue() )
			return Result();

		int iRows = 0;
		while ( EnsureCandidate(tWindow.m_tMin) )
		{
			// Sorted rows in this batch already meet the lower bound.
			while ( m_iPos<m_iLength )
			{
				const auto & tDoc = m_dBatch[m_iPos];
				const RowID_t tRow = tDoc.m_tRowID;
				if ( tRow==INVALID_ROWID || uint64_t(tRow)>=tWindow.m_uEnd )
					return Result(iRows);

				if ( !(tDoc.m_uFields & m_uFields) || tRow==m_tLastEmitted )
				{
					++m_iPos;
					continue;
				}

				if ( iRows==iCapacity )
					return Result(iRows);

				fnWrite ( tRow, iRows++ );
				m_tLastEmitted = tRow;
				++m_iPos;
			}

			// Refill even at capacity: an empty batch alone does not mean EOF.
		}

		return Result(iRows);
	}
};
}

std::unique_ptr<PostingWindow_i> CreateBufferedPostingWindow ( ISphQword & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
{
	return std::make_unique<BufferedPostingWindow_c> ( tWord, uFields, tContext );
}
