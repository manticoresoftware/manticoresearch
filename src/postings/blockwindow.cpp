// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "blockqword.h"
#include "std/bitmap/bitmap.h"
#include "std/bitcount.h"
#include <algorithm>

class BlockPostingWindow_c final : public PostingWindow_i
{
public:
	BlockPostingWindow_c ( BlockIndexQword_c & tWord, DWORD uFields, const PostingWindowContext_t & tContext );

	PostingWindowResult_t	AdvanceTo ( RowID_t tMin ) final;
	PostingWindowResult_t	FillOr ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) final;
	PostingProbeResult_t		Probe ( RowID_t tRow ) final;
	void					Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) final;

private:
	bool					CheckContinue();
	bool					LoadLeaf ( RowID_t tMin );
	bool					EnsureCandidate ( RowID_t tMin );
	bool					Matches() const;
	DWORD					BitmapWord ( unsigned uBit ) const;
	int					FillDense ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd );
	void					IntersectLeaf ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd );
	PostingWindowResult_t	Result ( int iRows=0 ) const;

	BlockIndexQword_c &		m_tWord;
	postings::Reader_c &		m_tReader;
	const DWORD				m_uFields;
	PostingWindowContext_t	m_tContext;
	postings::RowBlock_t		m_tBlock;
	alignas(16) std::array<uint32_t,postings::MAX_VALUES> m_dFields;

	uint64_t	m_uMin = 0;
	RowID_t		m_tNext = INVALID_ROWID;
	unsigned	m_iPos = 0;
	bool		m_bOpen = false;
	bool		m_bLeaf = false;
	bool		m_bEOF = false;
	bool		m_bStopped = false;
	bool		m_bAllFields = false;
	bool		m_bDirect = false;
};


BlockPostingWindow_c::BlockPostingWindow_c ( BlockIndexQword_c & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
	: m_tWord(tWord), m_tReader(tWord.m_tReader), m_uFields(uFields)
{
	Reset ( tWord.m_iDocs!=0, tContext );
}


void BlockPostingWindow_c::Reset ( bool bHasDocs, const PostingWindowContext_t & tContext )
{
	m_tContext = tContext;
	m_uMin = 0;
	m_tNext = INVALID_ROWID;
	m_iPos = 0;
	m_bOpen = m_bLeaf = m_bStopped = false;
	m_bEOF = !bHasDocs;
	m_tBlock = {};
}


bool BlockPostingWindow_c::CheckContinue()
{
	if ( !m_bStopped && m_tContext.m_fnContinue && !m_tContext.m_fnContinue(m_tContext.m_pContext) )
		m_bStopped = true;

	return !m_bStopped;
}


PostingWindowResult_t BlockPostingWindow_c::Result ( int iRows ) const
{
	return { m_tNext, iRows, m_bStopped };
}


bool BlockPostingWindow_c::LoadLeaf ( RowID_t tMin )
{
	if ( !CheckContinue() )
		return false;

	if ( !m_bOpen )
	{
		if ( !m_tReader.Open ( m_tWord.m_iStart, m_tWord.m_sError ) )
			return false;

		m_bOpen = true;
	}

	for ( ;; )
	{
		if ( !m_tReader.Next ( 0, m_tWord.m_sError ) )
			return false;

		if ( m_tReader.IsEOF() )
		{
			m_bEOF = true;
			m_tWord.m_iNext = m_tReader.GetEnd();
			return false;
		}

		if ( m_tReader.GetLeaf().m_uLast>=tMin )
			break;

		if ( !CheckContinue() )
			return false;
	}

	if ( !m_tReader.ReadRowBlock ( m_tBlock, m_tWord.m_sError ) )
		return false;

	unsigned uFields = m_tReader.GetDesc().m_uFields;
	m_bAllFields = uFields<=32 && (m_uFields & (UINT32_MAX>>(32-uFields)))==(UINT32_MAX>>(32-uFields));
	if ( !m_bAllFields )
	{
		if ( !m_tReader.ReadLowFields ( m_dFields.data(), m_tWord.m_sError ) )
			return false;

		m_bAllFields = std::all_of ( m_dFields.begin(), m_dFields.begin()+m_tBlock.m_uCount, [this] ( DWORD uMask ) { return (uMask & m_uFields)!=0; } );
	}

	m_bDirect = m_bAllFields && m_tBlock.m_eKind!=postings::Rows_e::DELTA;
	if ( !m_bDirect && !m_tReader.ReadRows(m_tWord.m_sError) )
		return false;

	m_iPos = 0;
	m_bLeaf = true;
	m_tWord.m_iNext = m_tReader.GetLeaf().m_uOffset+m_tReader.GetLeaf().m_uBytes;
	if ( m_tContext.m_pStats )
		m_tContext.m_pStats->m_iFetchedDocs += m_tBlock.m_uCount;

	return true;
}


// Load at most five bytes: bitmap payloads need neither alignment nor tail padding.
DWORD BlockPostingWindow_c::BitmapWord ( unsigned uBit ) const
{
	const auto & tData = m_tBlock.m_tPayload;
	unsigned iByte = uBit/8;
	const uint8_t * p = tData.m_pData+iByte;
	unsigned uShift = uBit%8;
	if ( tData.m_iSize-iByte>=4 )
	{
		DWORD uWord = DWORD(p[0]) | (DWORD(p[1])<<8) | (DWORD(p[2])<<16) | (DWORD(p[3])<<24);
		uWord >>= uShift;
		if ( uShift && tData.m_iSize-iByte>4 )
			uWord |= DWORD(p[4])<<(32-uShift);

		return uWord;
	}

	DWORD uWord = 0;
	for ( unsigned i=0; i<tData.m_iSize-iByte; ++i )
		uWord |= DWORD(p[i])<<(8*i);

	return uWord>>uShift;
}


bool BlockPostingWindow_c::EnsureCandidate ( RowID_t tMin )
{
	m_uMin = std::max ( m_uMin, uint64_t(tMin) );
	while ( !m_bEOF && !m_bStopped && m_uMin<INVALID_ROWID )
	{
		if ( !m_bLeaf || m_uMin>m_tBlock.m_uLast )
		{
			m_tNext = INVALID_ROWID;
			if ( m_tContext.m_pStats && m_bLeaf && m_uMin>uint64_t(m_tBlock.m_uLast)+1 )
				++m_tContext.m_pStats->m_iSkips;

			if ( !LoadLeaf(RowID_t(m_uMin)) )
			{
				if ( !m_tWord.m_sError.empty() )
				{
					m_tWord.Fail();
					m_bEOF = true;
				}

				return false;
			}
		}

		uint32_t uRow = uint32_t(std::max ( m_uMin, uint64_t(m_tBlock.m_uFirst) ));
		if ( !m_bDirect )
		{
			const auto & dRows = m_tReader.GetRows();
			m_iPos = unsigned(std::lower_bound ( dRows.begin()+m_iPos, dRows.begin()+m_tBlock.m_uCount, uRow )-dRows.begin());
			if ( m_iPos<m_tBlock.m_uCount )
			{
				m_tNext = dRows[m_iPos];
				return true;
			}
		} else if ( m_tBlock.m_eKind==postings::Rows_e::RANGE )
		{
			m_tNext = uRow;
			return true;
		} else
		{
			uint64_t uBit = uRow-m_tBlock.m_uFirst;
			while ( uBit<=m_tBlock.m_uLast-m_tBlock.m_uFirst )
			{
				DWORD uBits = BitmapWord(unsigned(uBit));
				if ( uBits )
				{
					m_tNext = m_tBlock.m_uFirst+uint32_t(uBit)+sphLog2(uBits & (DWORD(0)-uBits))-1;
					return true;
				}

				uBit += 32;
			}
		}

		m_uMin = uint64_t(m_tBlock.m_uLast)+1;
	}

	m_tNext = INVALID_ROWID;
	return false;
}


bool BlockPostingWindow_c::Matches() const
{
	return m_bAllFields || (m_dFields[m_iPos] & m_uFields)!=0;
}


PostingWindowResult_t BlockPostingWindow_c::AdvanceTo ( RowID_t tMin )
{
	if ( CheckContinue() )
		EnsureCandidate(tMin);

	return Result();
}


PostingProbeResult_t BlockPostingWindow_c::Probe ( RowID_t tRow )
{
	if ( CheckContinue() && EnsureCandidate(tRow) && m_tNext==tRow )
	{
		if ( Matches() )
			return { Result(), true };

		m_uMin = uint64_t(tRow)+1;
		EnsureCandidate(tRow);
	}

	return { Result(), false };
}


int BlockPostingWindow_c::FillDense ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd )
{
	unsigned uBegin = m_tNext-tWindow.m_tBase;
	unsigned uLimit = unsigned(uEnd-tWindow.m_tBase);
	m_uMin = uEnd;
	if ( m_tBlock.m_eKind==postings::Rows_e::RANGE )
	{
		unsigned iFirst = uBegin/32;
		unsigned iLast = (uLimit-1)/32;
		DWORD uFirst = UINT32_MAX<<(uBegin%32);
		DWORD uLast = UINT32_MAX>>(31-(uLimit-1)%32);
		if ( iFirst==iLast )
			pBits[iFirst] |= uFirst & uLast;
		else
		{
			pBits[iFirst] |= uFirst;
			std::fill ( pBits+iFirst+1, pBits+iLast, UINT32_MAX );
			pBits[iLast] |= uLast;
		}

		return int(uLimit-uBegin);
	}

	bool bWhole = m_tNext==m_tBlock.m_uFirst && uEnd==uint64_t(m_tBlock.m_uLast)+1;
	int iRows = bWhole ? m_tBlock.m_uCount : 0;
	uint64_t uRow = m_tNext;
	while ( uRow<uEnd )
	{
		unsigned uOffset = unsigned(uRow-tWindow.m_tBase);
		unsigned uShift = uOffset%32;
		unsigned uCount = unsigned(std::min ( uEnd-uRow, uint64_t(32-uShift) ));
		DWORD uBits = BitmapWord(unsigned(uRow-m_tBlock.m_uFirst)) & (UINT32_MAX>>(32-uCount));
		pBits[uOffset/32] |= uBits<<uShift;
		if ( !bWhole )
			iRows += sphBitCount(uBits);

		uRow += uCount;
	}

	return iRows;
}


PostingWindowResult_t BlockPostingWindow_c::FillOr ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	assert ( tWindow.m_tBase<=tWindow.m_tMin && tWindow.m_tMin<=tWindow.m_uEnd && tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
	if ( !CheckContinue() )
		return Result();

	int iRows = 0;
	uint64_t uEnd = std::min ( tWindow.m_uEnd, uint64_t(INVALID_ROWID) );
	while ( EnsureCandidate(tWindow.m_tMin) && m_tNext<uEnd )
	{
		if ( m_bDirect )
			iRows += FillDense ( tWindow, pBits, std::min ( uEnd, uint64_t(m_tBlock.m_uLast)+1 ) );
		else
		{
			const auto & dRows = m_tReader.GetRows();
			while ( m_iPos<m_tBlock.m_uCount && dRows[m_iPos]<uEnd )
			{
				RowID_t tRow = dRows[m_iPos];
				if ( Matches() )
				{
					unsigned uOffset = tRow-tWindow.m_tBase;
					pBits[uOffset/32] |= DWORD(1)<<(uOffset%32);
					++iRows;
				}

				m_uMin = uint64_t(tRow)+1;
				++m_iPos;
			}
		}
	}

	return Result(iRows);
}


static void ClearWindowBits ( DWORD * pBits, unsigned uBegin, unsigned uEnd )
{
	if ( uBegin>=uEnd )
		return;

	unsigned iFirst = uBegin/32;
	unsigned iLast = (uEnd-1)/32;
	DWORD uFirst = UINT32_MAX<<(uBegin%32);
	DWORD uLast = UINT32_MAX>>(31-(uEnd-1)%32);
	if ( iFirst==iLast )
		pBits[iFirst] &= ~(uFirst & uLast);
	else
	{
		pBits[iFirst] &= ~uFirst;
		std::fill ( pBits+iFirst+1, pBits+iLast, 0 );
		pBits[iLast] &= ~uLast;
	}
}


// Intersect only this leaf's covered interval; adjacent leaves can share a word.
void BlockPostingWindow_c::IntersectLeaf ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd )
{
	m_uMin = uEnd;
	if ( m_bDirect && m_tBlock.m_eKind==postings::Rows_e::RANGE )
		return;

	const auto & dRows = m_tReader.GetRows();
	uint64_t uRow = m_tNext;
	while ( uRow<uEnd )
	{
		unsigned uOffset = unsigned(uRow-tWindow.m_tBase);
		unsigned uShift = uOffset%32;
		unsigned uCount = unsigned(std::min ( uEnd-uRow, uint64_t(32-uShift) ));
		DWORD uMask = (UINT32_MAX>>(32-uCount))<<uShift;
		DWORD uBits = 0;
		if ( m_bDirect )
			uBits = (BitmapWord(unsigned(uRow-m_tBlock.m_uFirst))<<uShift) & uMask;
		else
		{
			while ( m_iPos<m_tBlock.m_uCount && dRows[m_iPos]<uRow+uCount )
			{
				if ( Matches() )
					uBits |= DWORD(1)<<((dRows[m_iPos]-tWindow.m_tBase)%32);

				++m_iPos;
			}
		}

		pBits[uOffset/32] &= ~uMask | uBits;
		uRow += uCount;
	}
}


PostingWindowResult_t BlockPostingWindow_c::FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	assert ( tWindow.m_tBase<=tWindow.m_tMin && tWindow.m_tMin<=tWindow.m_uEnd && tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
	unsigned uProcessed = 0;
	unsigned iWords = unsigned((tWindow.m_uEnd-tWindow.m_tBase+31)/32);
	uint64_t uEnd = std::min ( tWindow.m_uEnd, uint64_t(INVALID_ROWID) );
	if ( CheckContinue() )
		while ( EnsureCandidate(tWindow.m_tMin) && m_tNext<uEnd )
		{
			ClearWindowBits ( pBits, uProcessed, m_tNext-tWindow.m_tBase );
			uint64_t uLeafEnd = std::min ( uEnd, uint64_t(m_tBlock.m_uLast)+1 );
			IntersectLeaf ( tWindow, pBits, uLeafEnd );
			uProcessed = unsigned(uLeafEnd-tWindow.m_tBase);
		}

	// Unvisited rows cannot survive, including when a refill was interrupted.
	ClearWindowBits ( pBits, uProcessed, iWords*32 );
	return Result(int(bitmap::Count ( pBits, iWords )));
}


PostingWindowResult_t BlockPostingWindow_c::ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity )
{
	assert ( iCapacity>0 );
	if ( !CheckContinue() )
		return Result();

	int iRows = 0;
	while ( EnsureCandidate(tWindow.m_tMin) && m_tNext<tWindow.m_uEnd )
	{
		if ( Matches() )
		{
			if ( iRows==iCapacity )
				break;

			pRows[iRows++] = m_tNext;
		}

		m_uMin = uint64_t(m_tNext)+1;
	}

	return Result(iRows);
}


std::unique_ptr<PostingWindow_i> BlockIndexQword_c::CreatePostingWindow ( DWORD uFields, const PostingWindowContext_t & tContext )
{
	SetMembershipOnly();
	return std::make_unique<BlockPostingWindow_c> ( *this, uFields, tContext );
}
