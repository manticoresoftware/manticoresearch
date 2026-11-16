//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "termproducer.h"
#include "../termproducerbase.h"

#include <algorithm>
#include <array>

namespace
{

class BlockTermProducer_c final : public TermProducerBase_c
{
public:
	BlockTermProducer_c ( ISphQword & tWord, const TermProducerSetup_t & tSetup, std::unique_ptr<PostingBlock_i> pCursor );

	void						HintRowID ( RowID_t tMin ) final;
	PostingCandidateBatch_t	NextCandidates ( int iCapacity ) final;
	const QwordDoc_t *		ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount ) final;

private:
	bool						FillRows ( RowID_t tMin, uint64_t uEnd );
	PostingCandidateBatch_t	ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity );

	std::unique_ptr<PostingBlock_i> m_pCursor;
	std::array<RowID_t,PostingWindow_t::ROWS> m_dRows;
	PostingCandidateBatch_t m_tRows;

	RowID_t				m_tNext = 0;
	int					m_iRead = 0;
	bool				m_bDone = false;
	RowID_t				m_tHint = 0;
};


BlockTermProducer_c::BlockTermProducer_c ( ISphQword & tWord, const TermProducerSetup_t & tSetup, std::unique_ptr<PostingBlock_i> pCursor )
	: TermProducerBase_c ( tWord, tSetup )
	, m_pCursor(std::move(pCursor))
{}


void BlockTermProducer_c::HintRowID ( RowID_t tMin )
{
	m_tHint = std::max ( m_tHint, tMin );
}


PostingCandidateBatch_t BlockTermProducer_c::NextCandidates ( int iCapacity )
{
	assert ( iCapacity>0 && iCapacity<=m_dDocs.GetLength() );
	if ( m_bDone )
		return {};

	m_tRows = ReadRows ( std::max ( m_tHint, m_tSetup.m_tMin ), m_tSetup.m_uEnd, iCapacity );
	if ( !m_tRows.m_iCount )
		m_bDone = true;

	return m_tRows;
}


const QwordDoc_t * BlockTermProducer_c::ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount )
{
	assert ( iTerm==0 );
	if ( !pRows )
	{
		pRows = m_tRows.m_pRows;
		iCount = m_tRows.m_iCount;
	}

	assert ( iCount>=0 && iCount<=m_tRows.m_iCount );

	const auto & tPayloads = m_tSetup.m_tPayloads;
	if ( !m_pCursor->ReadPayloads ( pRows, iCount, m_dDocs.Begin(), tPayloads.m_bTF || tPayloads.m_bHits, tPayloads.m_bHits ) )
	{
		m_bDone = true;
		return nullptr;
	}

	return m_dDocs.Begin();
}


bool BlockTermProducer_c::FillRows ( RowID_t tMin, uint64_t uEnd )
{
	PostingLeaf_t tLeaf;
	if ( !m_pCursor->Prepare ( tMin, tLeaf ) )
		return false;

	RowID_t tFirst = std::max ( tMin, tLeaf.m_tFirst );
	uint64_t uWindowEnd = (uint64_t(tFirst)/PostingWindow_t::ROWS+1)*PostingWindow_t::ROWS;
	uint64_t uLimit = std::min ( { uEnd, tLeaf.m_uEnd, uWindowEnd } );
	if ( tFirst>=uLimit )
		return false;

	PostingWindow_t tWindow { tFirst, tFirst, uLimit };
	m_iRead = 0;
	m_iCount = m_pCursor->ReadRows ( tWindow, m_dRows.data() );

	m_tNext = RowID_t(uLimit);
	return true;
}


PostingCandidateBatch_t BlockTermProducer_c::ReadRows ( RowID_t tMin, uint64_t uEnd, int iCapacity )
{
	assert ( iCapacity>0 );
	uEnd = std::min ( uEnd, uint64_t(INVALID_ROWID) );
	while ( true )
	{
		if ( m_tSetup.m_tContext.m_fnContinue && !m_tSetup.m_tContext.m_fnContinue(m_tSetup.m_tContext.m_pContext) )
			return {};

		RowID_t tFirst = m_iRead<m_iCount ? m_dRows[m_iRead] : m_tNext;
		tFirst = m_tSetup.m_pFilter->AdvanceTo ( std::max ( tFirst, tMin ) );
		if ( tFirst>=uEnd )
			return {};

		m_iRead = int(std::lower_bound ( m_dRows.begin()+m_iRead, m_dRows.begin()+m_iCount, tFirst )-m_dRows.begin());
		if ( m_iRead==m_iCount )
		{
			if ( !FillRows ( std::max ( tFirst, m_tNext ), uEnd ) )
				return {};

			if ( !m_iCount )
				continue;
		}

		int iCount = std::min ( iCapacity, m_iCount-m_iRead );
		while ( iCount && m_dRows[m_iRead+iCount-1]>=uEnd )
			--iCount;

		if ( !iCount )
			return {};

		RowID_t * pRows = m_dRows.data()+m_iRead;
		m_iRead += iCount;
		iCount = m_tSetup.m_pFilter->SelectRows ( pRows, iCount );
		if ( iCount )
			return { pRows, nullptr, iCount };
	}
}

}


std::unique_ptr<PostingProducer_i> CreateBlockTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
{
	auto pCursor = tWord.CreatePostingBlock ( tSetup.m_tFields.GetMask32(), tSetup.m_tContext );
	if ( !pCursor )
		return nullptr;

	return std::make_unique<BlockTermProducer_c> ( tWord, tSetup, std::move(pCursor) );
}
