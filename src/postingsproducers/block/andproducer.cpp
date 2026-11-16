//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "andproducer.h"
#include "../andproducerbase.h"

#include <algorithm>
#include <array>

namespace
{

class BlockAndProducer_c final : public AndProducerBase_c
{
public:
	BlockAndProducer_c ( const AndProducerSetup_t & tSetup, std::vector<std::unique_ptr<PostingBlock_i>> dCursors, std::vector<std::unique_ptr<PostingWindow_i>> dExcluded );

	PostingCandidateBatch_t	NextCandidates ( int iCapacity ) final;
	const QwordDoc_t *		ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount ) final;

private:
	PostingCandidateBatch_t	ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity );
	bool						FillCandidates ( uint64_t uEnd );
	bool						IntersectBitmap ( const PostingWindow_t & tWindow );
	void						ExtractRows ( RowID_t tBase, const DWORD * pBits, int iWords );
	bool						SubtractRows();

	std::vector<std::unique_ptr<PostingBlock_i>>	m_dCursors;
	std::vector<std::unique_ptr<PostingWindow_i>>	m_dExcluded;
	std::array<RowID_t,PostingWindow_t::ROWS>		m_dRows;
	std::vector<QwordDoc_t>						m_dPayloads;
	PostingCandidateBatch_t						m_tRows;

	RowID_t					m_tNext = 0;
	int						m_iCount = 0;
	int						m_iRead = 0;
	int						m_iEarlyFilterRetry = 0;
	bool					m_bDone = false;
};


BlockAndProducer_c::BlockAndProducer_c ( const AndProducerSetup_t & tSetup, std::vector<std::unique_ptr<PostingBlock_i>> dCursors, std::vector<std::unique_ptr<PostingWindow_i>> dExcluded )
	: AndProducerBase_c(tSetup)
	, m_dCursors(std::move(dCursors))
	, m_dExcluded(std::move(dExcluded))
	, m_dPayloads(tSetup.m_iCapacity)
{}


PostingCandidateBatch_t BlockAndProducer_c::NextCandidates ( int iCapacity )
{
	assert ( iCapacity>0 && iCapacity<=m_tSetup.m_iCapacity );
	m_tRows = ReadRows ( m_tHint, m_tSetup.m_uEnd, iCapacity );
	return m_tRows;
}


const QwordDoc_t * BlockAndProducer_c::ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount )
{
	assert ( iTerm>=0 && iTerm<int(m_tSetup.m_dTerms.size()) );
	if ( !pRows )
	{
		pRows = m_tRows.m_pRows;
		iCount = m_tRows.m_iCount;
	}

	assert ( iCount>=0 && iCount<=m_tRows.m_iCount );

	const auto & tRequirements = m_tSetup.m_tPayloads;
	if ( !m_dCursors[iTerm]->ReadPayloads ( pRows, iCount, m_dPayloads.data(), tRequirements.m_bTF, tRequirements.m_bHits ) )
		return nullptr;

	return m_dPayloads.data();
}


bool BlockAndProducer_c::SubtractRows()
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


void BlockAndProducer_c::ExtractRows ( RowID_t tBase, const DWORD * pBits, int iWords )
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


bool BlockAndProducer_c::FillCandidates ( uint64_t uEnd )
{
	while ( m_tNext<uEnd )
	{
		if ( m_tSetup.m_tContext.m_fnContinue && !m_tSetup.m_tContext.m_fnContinue(m_tSetup.m_tContext.m_pContext) )
			return false;

		RowID_t tFirst = m_tSetup.m_pFilter ? m_tSetup.m_pFilter->AdvanceTo(m_tNext) : m_tNext;
		if ( tFirst>=uEnd )
			return false;

		uint64_t uLimit = uEnd;
		bool bDecoded = false;
		for ( auto & pCursor : m_dCursors )
		{
			PostingLeaf_t tLeaf;
			if ( !pCursor->Prepare ( tFirst, tLeaf ) )
				return false;

			if ( pCursor==m_dCursors.front() )
				bDecoded = tLeaf.m_bDecoded;

			tFirst = std::max ( tFirst, tLeaf.m_tFirst );
			uLimit = std::min ( uLimit, tLeaf.m_uEnd );
		}

		m_tNext = tFirst;
		if ( tFirst>=uLimit )
			continue;

		uLimit = std::min ( uLimit, uint64_t(tFirst)+PostingWindow_t::ROWS );

		PostingWindow_t tWindow { tFirst, tFirst, uLimit };
		if ( (bDecoded || m_dCursors.size()==1) && m_dExcluded.empty() )
		{
			m_iCount = m_dCursors.front()->ReadRows ( tWindow, m_dRows.data() );
			if ( m_tSetup.m_pFilter && m_tSetup.m_pFilter->GetFilterComplexity()<=1.0f && m_iCount && m_dCursors.size()>1 )
			{
				if ( !m_iEarlyFilterRetry )
				{
					int iBefore = m_iCount;
					m_iCount = m_tSetup.m_pFilter->SelectRows ( m_dRows.data(), m_iCount );
					// Retry every 32 batches when the filter rejects less than half the candidates.
					m_iEarlyFilterRetry = m_iCount*2<=iBefore ? 0 : 31;
				} else
					--m_iEarlyFilterRetry;
			}

			for ( size_t i=1; i<m_dCursors.size() && m_iCount; ++i )
				m_iCount = m_dCursors[i]->FilterRows ( m_dRows.data(), m_iCount );
		} else if ( !IntersectBitmap(tWindow) )
			return false;

		m_tNext = RowID_t(uLimit);
		m_iRead = 0;
		if ( m_iCount )
			return true;
	}

	return false;
}


bool BlockAndProducer_c::IntersectBitmap ( const PostingWindow_t & tWindow )
{
	int iWords = int((tWindow.m_uEnd-tWindow.m_tBase+31)/32);
	DWORD dBits[PostingWindow_t::WORDS];
	std::fill_n ( dBits, iWords, UINT32_MAX );

	int iCount = int(tWindow.m_uEnd-tWindow.m_tBase);
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
			ExtractRows ( tWindow.m_tBase, dBits, iWords );
	} else if ( iCount )
	{
		ExtractRows ( tWindow.m_tBase, dBits, iWords );
		for ( ; iTerm<m_dCursors.size() && m_iCount; ++iTerm )
			m_iCount = m_dCursors[iTerm]->FilterRows ( m_dRows.data(), m_iCount );

		if ( !SubtractRows() )
			return false;
	}

	return true;
}


PostingCandidateBatch_t BlockAndProducer_c::ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity )
{
	assert ( iCapacity>0 && !m_dCursors.empty() );
	uEnd = std::min ( uEnd, uint64_t(INVALID_ROWID) );
	m_tNext = std::max ( m_tNext, tMin );
	m_iRead = int(std::lower_bound ( m_dRows.begin()+m_iRead, m_dRows.begin()+m_iCount, tMin )-m_dRows.begin());
	while ( !m_bDone )
	{
		if ( m_iRead==m_iCount )
			m_bDone = !FillCandidates(uEnd);

		if ( m_bDone || m_iRead==m_iCount || m_dRows[m_iRead]>=uEnd )
			return {};

		int iCount = std::min ( iCapacity, m_iCount-m_iRead );
		while ( iCount && m_dRows[m_iRead+iCount-1]>=uEnd )
			--iCount;

		RowID_t * pRows = m_dRows.data()+m_iRead;
		m_iRead += iCount;
		if ( m_tSetup.m_pFilter )
			iCount = m_tSetup.m_pFilter->SelectRows ( pRows, iCount );

		if ( iCount )
			return { pRows, nullptr, iCount };
	}

	return {};
}

}


std::unique_ptr<PostingProducer_i> CreateBlockAndProducer ( const AndProducerSetup_t & tSetup )
{
	std::vector<std::unique_ptr<PostingBlock_i>> dCursors;
	std::vector<std::unique_ptr<PostingWindow_i>> dExcluded;
	for ( const auto & tTerm : tSetup.m_dTerms )
	{
		auto pCursor = tTerm.m_pQword->CreatePostingBlock ( tTerm.m_tFields.GetMask32(), tSetup.m_tContext );
		if ( !pCursor )
			return nullptr;

		dCursors.push_back(std::move(pCursor));
	}

	for ( const auto & tTerm : tSetup.m_dExcluded )
	{
		if ( !tTerm.m_pQword->m_iDocs )
			continue;

		auto pCursor = tTerm.m_pQword->CreatePostingWindow ( tTerm.m_tFields.GetMask32(), tSetup.m_tContext );
		if ( !pCursor )
			return nullptr;

		dExcluded.push_back(std::move(pCursor));
	}

	return std::make_unique<BlockAndProducer_c> ( tSetup, std::move(dCursors), std::move(dExcluded) );
}
