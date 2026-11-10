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

template<bool MAPPED>
class BlockPostingWindow_T final : public PostingWindow_i, public PostingBlock_i
{
public:
	BlockPostingWindow_T ( BlockIndexQword_T<MAPPED> & tWord, DWORD uFields, const PostingWindowContext_t & tContext );

	PostingWindowResult_t	AdvanceTo ( RowID_t tMin ) final;
	PostingWindowResult_t	FillOr ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	PostingWindowResult_t	ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity ) final;
	PostingProbeResult_t		Probe ( RowID_t tRow ) final;
	void					Reset ( bool bHasDocs, const PostingWindowContext_t & tContext ) final;

	bool					Prepare ( RowID_t tMin, PostingLeaf_t & tLeaf ) final;
	int						Intersect ( const PostingWindow_t & tWindow, DWORD * pBits ) final;
	int						ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows ) final;
	int						FilterRows ( RowID_t * pRows, int iCount ) final;
	bool					ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits ) final;

private:
	int						DrainDecodedRows ( uint64_t uEnd, RowID_t * pRows, int iCapacity );
	const RowID_t *			FindDecodedRow ( RowID_t tRow, unsigned iHint ) const;
	void					BuildBitmapOrdinals();
	bool					CheckContinue();
	bool					LoadLeaf ( RowID_t tMin );
	bool					EnsureCandidate ( RowID_t tMin );
	bool					PassesFieldCheck() const;
	DWORD					BitmapWord ( unsigned uBit ) const;
	int					FillDense ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd );
	template<bool SUBTRACT>
	void					ApplyLeaf ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd );
	PostingWindowResult_t	Result ( int iRows=0 ) const;

	BlockIndexQword_T<MAPPED> &		m_tWord;
	postings::Reader_c &		m_tReader;
	const DWORD				m_uFields;
	PostingWindowContext_t	m_tContext;
	postings::RowBlock_t		m_tBlock;
	alignas(16) std::array<uint32_t,postings::MAX_VALUES> m_dFields;

	uint64_t	m_uMin = 0;
	RowID_t		m_tNext = INVALID_ROWID;
	unsigned	m_iPos = m_iPayloadPos = 0;
	unsigned	m_iPayloadPos = 0;
	bool		m_bOpen = false;
	bool		m_bLeaf = false;
	bool		m_bEOF = false;
	bool		m_bStopped = false;
	bool		m_bAllFields = false;
	bool		m_bDirect = false;
	bool		m_bLowFields = false;
	bool		m_bOrdinals = false;

	std::array<uint16_t,postings::MAX_BITMAP_BITS/32+1> m_dOrdinals;
};


template<bool MAPPED>
BlockPostingWindow_T<MAPPED>::BlockPostingWindow_T ( BlockIndexQword_T<MAPPED> & tWord, DWORD uFields, const PostingWindowContext_t & tContext )
	: m_tWord(tWord), m_tReader(tWord.m_tReader), m_uFields(uFields)
{
	Reset ( tWord.m_iDocs!=0, tContext );
}


template<bool MAPPED>
void BlockPostingWindow_T<MAPPED>::Reset ( bool bHasDocs, const PostingWindowContext_t & tContext )
{
	m_tContext = tContext;
	m_uMin = 0;
	m_tNext = INVALID_ROWID;
	m_iPos = m_iPayloadPos = 0;
	m_bOpen = m_bLeaf = m_bStopped = false;
	m_bEOF = !bHasDocs;
	m_tBlock = {};
}


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::CheckContinue()
{
	if ( !m_bStopped && m_tContext.m_fnContinue && !m_tContext.m_fnContinue(m_tContext.m_pContext) )
		m_bStopped = true;

	return !m_bStopped;
}


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::Result ( int iRows ) const
{
	return { m_tNext, iRows, m_bStopped };
}


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::LoadLeaf ( RowID_t tMin )
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

	m_bLowFields = m_bOrdinals = false;

	unsigned uFields = m_tReader.GetDesc().m_uFields;
	m_bAllFields = uFields<=32 && (m_uFields & (UINT32_MAX>>(32-uFields)))==(UINT32_MAX>>(32-uFields));
	if ( !m_bAllFields )
	{
		if ( !m_tReader.ReadLowFields ( m_dFields.data(), m_tWord.m_sError ) )
			return false;

		m_bLowFields = true;
		m_bAllFields = std::all_of ( m_dFields.begin(), m_dFields.begin()+m_tBlock.m_uCount, [this] ( DWORD uMask ) { return (uMask & m_uFields)!=0; } );
	}

	m_bDirect = m_bAllFields && m_tBlock.m_eKind!=postings::Rows_e::DELTA;
	if ( !m_bDirect && !m_tReader.ReadRows(m_tWord.m_sError) )
		return false;

	m_iPos = m_iPayloadPos = 0;
	m_bLeaf = true;
	m_tWord.m_iNext = m_tReader.GetLeaf().m_uOffset+m_tReader.GetLeaf().m_uBytes;
	if ( m_tContext.m_pStats )
		m_tContext.m_pStats->m_iFetchedDocs += m_tBlock.m_uCount;

	return true;
}


// Load at most five bytes: bitmap payloads need neither alignment nor tail padding.
template<bool MAPPED>
DWORD BlockPostingWindow_T<MAPPED>::BitmapWord ( unsigned uBit ) const
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


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::EnsureCandidate ( RowID_t tMin )
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
			if ( m_iPos<m_tBlock.m_uCount && dRows[m_iPos]<uRow )
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


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::PassesFieldCheck() const
{
	return m_bAllFields || (m_dFields[m_iPos] & m_uFields)!=0;
}


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::AdvanceTo ( RowID_t tMin )
{
	if ( CheckContinue() )
		EnsureCandidate(tMin);

	return Result();
}


template<bool MAPPED>
PostingProbeResult_t BlockPostingWindow_T<MAPPED>::Probe ( RowID_t tRow )
{
	if ( CheckContinue() && EnsureCandidate(tRow) && m_tNext==tRow )
	{
		if ( PassesFieldCheck() )
			return { Result(), true };

		m_uMin = uint64_t(tRow)+1;
		EnsureCandidate(tRow);
	}

	return { Result(), false };
}


template<bool MAPPED>
int BlockPostingWindow_T<MAPPED>::FillDense ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd )
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


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::FillOr ( const PostingWindow_t & tWindow, DWORD * pBits )
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
				if ( PassesFieldCheck() )
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


// Modify only this leaf's covered interval; adjacent leaves can share a word.
template<bool MAPPED>
template<bool SUBTRACT>
void BlockPostingWindow_T<MAPPED>::ApplyLeaf ( const PostingWindow_t & tWindow, DWORD * pBits, uint64_t uEnd )
{
	m_uMin = uEnd;
	if ( m_bDirect && m_tBlock.m_eKind==postings::Rows_e::RANGE )
	{
		if constexpr ( SUBTRACT )
			ClearWindowBits ( pBits, m_tNext-tWindow.m_tBase, unsigned(uEnd-tWindow.m_tBase) );

		return;
	}

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
				if ( PassesFieldCheck() )
					uBits |= DWORD(1)<<((dRows[m_iPos]-tWindow.m_tBase)%32);

				++m_iPos;
			}
		}

		if constexpr ( SUBTRACT )
			pBits[uOffset/32] &= ~uBits;
		else
			pBits[uOffset/32] &= ~uMask | uBits;

		uRow += uCount;
	}
}


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::FillAnd ( const PostingWindow_t & tWindow, DWORD * pBits )
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
			ApplyLeaf<false> ( tWindow, pBits, uLeafEnd );
			uProcessed = unsigned(uLeafEnd-tWindow.m_tBase);
		}

	// Unvisited rows cannot survive, including when a refill was interrupted.
	ClearWindowBits ( pBits, uProcessed, iWords*32 );
	return Result(int(bitmap::Count ( pBits, iWords )));
}


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::FillAndNot ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	assert ( tWindow.m_tBase<=tWindow.m_tMin && tWindow.m_tMin<=tWindow.m_uEnd && tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
	unsigned iWords = unsigned((tWindow.m_uEnd-tWindow.m_tBase+31)/32);
	uint64_t uEnd = std::min ( tWindow.m_uEnd, uint64_t(INVALID_ROWID) );
	if ( CheckContinue() )
		while ( EnsureCandidate(tWindow.m_tMin) && m_tNext<uEnd )
			ApplyLeaf<true> ( tWindow, pBits, std::min ( uEnd, uint64_t(m_tBlock.m_uLast)+1 ) );

	if ( m_bStopped )
		std::fill_n ( pBits, iWords, 0 );
	else
	{
		ClearWindowBits ( pBits, 0, tWindow.m_tMin-tWindow.m_tBase );
		ClearWindowBits ( pBits, unsigned(uEnd-tWindow.m_tBase), iWords*32 );
	}

	return Result(int(bitmap::Count ( pBits, iWords )));
}


// Consume decoded rows sequentially, retaining the first row outside the output or window limit.
template<bool MAPPED>
int BlockPostingWindow_T<MAPPED>::DrainDecodedRows ( uint64_t uEnd, RowID_t * pRows, int iCapacity )
{
	const auto & dRows = m_tReader.GetRows();
	int iRows = 0;
	while ( m_iPos<m_tBlock.m_uCount && dRows[m_iPos]<uEnd )
	{
		if ( PassesFieldCheck() )
		{
			if ( iRows==iCapacity )
				break;

			pRows[iRows++] = dRows[m_iPos];
		}

		m_uMin = uint64_t(dRows[m_iPos++])+1;
	}

	return iRows;
}


template<bool MAPPED>
PostingWindowResult_t BlockPostingWindow_T<MAPPED>::ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows, int iCapacity )
{
	assert ( iCapacity>0 );
	if ( !CheckContinue() )
		return Result();

	int iRows = 0;
	while ( EnsureCandidate(tWindow.m_tMin) && m_tNext<tWindow.m_uEnd )
	{
		if ( !m_bDirect )
		{
			iRows += DrainDecodedRows ( tWindow.m_uEnd, pRows+iRows, iCapacity-iRows );
			if ( m_iPos<m_tBlock.m_uCount )
			{
				m_tNext = m_tReader.GetRows()[m_iPos];
				return Result(iRows);
			}

			continue;
		}

		if ( PassesFieldCheck() )
		{
			if ( iRows==iCapacity )
				break;

			pRows[iRows++] = m_tNext;
		}

		m_uMin = uint64_t(m_tNext)+1;
	}

	return Result(iRows);
}


template<bool MAPPED>
std::unique_ptr<PostingWindow_i> BlockIndexQword_T<MAPPED>::CreatePostingWindow ( DWORD uFields, const PostingWindowContext_t & tContext )
{
	SetPayloadRequirements ( false, false );
	return std::make_unique<BlockPostingWindow_T<MAPPED>> ( *this, uFields, tContext );
}


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::Prepare ( RowID_t tMin, PostingLeaf_t & tLeaf )
{
	if ( !CheckContinue() || !EnsureCandidate(tMin) )
		return false;

	tLeaf = { m_tNext, uint64_t(m_tBlock.m_uLast)+1, !m_bDirect };
	return true;
}


template<bool MAPPED>
int BlockPostingWindow_T<MAPPED>::Intersect ( const PostingWindow_t & tWindow, DWORD * pBits )
{
	assert ( m_bLeaf && tWindow.m_tBase==tWindow.m_tMin && tWindow.m_tMin>=m_tNext && tWindow.m_tMin<tWindow.m_uEnd );
	assert ( tWindow.m_uEnd<=uint64_t(m_tBlock.m_uLast)+1 && tWindow.m_uEnd<=uint64_t(tWindow.m_tBase)+PostingWindow_t::ROWS );
	// The caller aligned all leaves before choosing the common interval. Seeking
	// inside this leaf cannot invalidate any payload retained for the interval.
	if ( !EnsureCandidate(tWindow.m_tMin) || m_tNext>=tWindow.m_uEnd )
	{
		std::fill_n ( pBits, (tWindow.m_uEnd-tWindow.m_tBase+31)/32, 0 );
		return 0;
	}

	ClearWindowBits ( pBits, 0, m_tNext-tWindow.m_tBase );
	ApplyLeaf<false> ( tWindow, pBits, tWindow.m_uEnd );

	unsigned iWords = unsigned((tWindow.m_uEnd-tWindow.m_tBase+31)/32);
	ClearWindowBits ( pBits, unsigned(tWindow.m_uEnd-tWindow.m_tBase), iWords*32 );
	return int(bitmap::Count ( pBits, iWords ));
}


template<bool MAPPED>
int BlockPostingWindow_T<MAPPED>::ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows )
{
	assert ( m_bLeaf && !m_bDirect && tWindow.m_tMin>=m_tNext && tWindow.m_tMin<tWindow.m_uEnd );
	assert ( tWindow.m_uEnd<=uint64_t(m_tBlock.m_uLast)+1 && tWindow.m_uEnd-tWindow.m_tMin<=PostingWindow_t::ROWS );
	if ( !EnsureCandidate(tWindow.m_tMin) || m_tNext>=tWindow.m_uEnd )
		return 0;

	m_iPayloadPos = m_iPos;
	return DrainDecodedRows ( tWindow.m_uEnd, pRows, PostingWindow_t::ROWS );
}


// Reuse a known ordinal when it still identifies the requested row.
template<bool MAPPED>
const RowID_t * BlockPostingWindow_T<MAPPED>::FindDecodedRow ( RowID_t tRow, unsigned iHint ) const
{
	const auto & dRows = m_tReader.GetRows();
	if ( iHint<m_tBlock.m_uCount && dRows[iHint]==tRow )
		return dRows.data()+iHint;

	return std::lower_bound ( dRows.data(), dRows.data()+m_tBlock.m_uCount, tRow );
}


template<bool MAPPED>
void BlockPostingWindow_T<MAPPED>::BuildBitmapOrdinals()
{
	if ( m_bOrdinals )
		return;

	unsigned iWords = (m_tBlock.m_uLast-m_tBlock.m_uFirst+32)/32;
	m_dOrdinals[0] = 0;
	for ( unsigned i=0; i<iWords; ++i )
		m_dOrdinals[i+1] = uint16_t(m_dOrdinals[i]+sphBitCount(BitmapWord(i*32)));

	m_bOrdinals = true;
}


template<bool MAPPED>
int BlockPostingWindow_T<MAPPED>::FilterRows ( RowID_t * pRows, int iCount )
{
	if ( !iCount || (m_bDirect && m_tBlock.m_eKind==postings::Rows_e::RANGE) )
		return iCount;

	int iOut = 0;
	if ( m_bDirect )
	{
		for ( int i=0; i<iCount; ++i )
			if ( BitmapWord(pRows[i]-m_tBlock.m_uFirst) & 1 )
				pRows[iOut++] = pRows[i];
	} else
	{
		const auto & dRows = m_tReader.GetRows();
		const auto pEnd = dRows.begin()+m_tBlock.m_uCount;
		auto pRow = FindDecodedRow ( pRows[0], m_iPos );
		for ( int i=0; i<iCount; ++i )
		{
			while ( pRow!=pEnd && *pRow<pRows[i] )
				++pRow;

			if ( pRow!=pEnd && *pRow==pRows[i] && (m_bAllFields || (m_dFields[pRow-dRows.begin()] & m_uFields)) )
			{
				if ( !iOut )
					m_iPayloadPos = unsigned(pRow-dRows.data());

				pRows[iOut++] = pRows[i];
			}
		}
	}

	return iOut;
}


template<bool MAPPED>
bool BlockPostingWindow_T<MAPPED>::ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits )
{
	if ( !iCount )
		return true;

	if ( (!m_bLowFields && !m_tReader.ReadLowFields ( m_dFields.data(), m_tWord.m_sError ))
		|| (bTF && !m_tReader.ReadTF(m_tWord.m_sError)) || (bHits && !m_tReader.ReadHitRefs(m_tWord.m_sError)) )
	{
		m_tWord.Fail();
		return false;
	}

	m_bLowFields = true;

	auto Store = [&] ( int i, unsigned iOrdinal )
	{
		assert ( iOrdinal<m_tBlock.m_uCount );
		pDocs[i] = { pRows[i], m_dFields[iOrdinal], bTF ? m_tReader.GetTF()[iOrdinal] : 0, bHits ? SphOffset_t(m_tReader.GetHitRefs()[iOrdinal]) : 0 };
	};

	if ( m_bDirect && m_tBlock.m_eKind==postings::Rows_e::RANGE )
	{
		for ( int i=0; i<iCount; ++i )
			Store ( i, pRows[i]-m_tBlock.m_uFirst );
	} else if ( m_bDirect )
	{
		BuildBitmapOrdinals();
		for ( int i=0; i<iCount; ++i )
		{
			unsigned uBit = pRows[i]-m_tBlock.m_uFirst;
			DWORD uMask = (DWORD(1)<<(uBit%32))-1;
			Store ( i, m_dOrdinals[uBit/32]+sphBitCount(BitmapWord(uBit & ~31U) & uMask) );
		}
	} else
	{
		const auto & dRows = m_tReader.GetRows();
		auto pRow = FindDecodedRow ( pRows[0], m_iPayloadPos );
		for ( int i=0; i<iCount; ++i )
		{
			while ( pRow!=dRows.begin()+m_tBlock.m_uCount && *pRow<pRows[i] )
				++pRow;

			assert ( pRow!=dRows.begin()+m_tBlock.m_uCount && *pRow==pRows[i] );
			Store ( i, unsigned(pRow-dRows.begin()) );
		}

		m_iPayloadPos = unsigned(pRow-dRows.data())+1;
	}

	return true;
}


template<bool MAPPED>
std::unique_ptr<PostingBlock_i> BlockIndexQword_T<MAPPED>::CreatePostingBlock ( DWORD uFields, const PostingWindowContext_t & tContext )
{
	return std::make_unique<BlockPostingWindow_T<MAPPED>> ( *this, uFields, tContext );
}


template std::unique_ptr<PostingWindow_i> BlockIndexQword_T<true>::CreatePostingWindow ( DWORD, const PostingWindowContext_t & );


template std::unique_ptr<PostingBlock_i> BlockIndexQword_T<true>::CreatePostingBlock ( DWORD, const PostingWindowContext_t & );


template std::unique_ptr<PostingWindow_i> BlockIndexQword_T<false>::CreatePostingWindow ( DWORD, const PostingWindowContext_t & );


template std::unique_ptr<PostingBlock_i> BlockIndexQword_T<false>::CreatePostingBlock ( DWORD, const PostingWindowContext_t & );
