//
//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "docidlookup.h"

#include "secondaryindex.h"
#include "sphinxfilter.h"
#include "killlist.h"
#include "attribute.h"
#include "fileio.h"
#include "columnarfilter.h"

#include <boost/preprocessor/repetition/repeat.hpp>

template <bool BITMAP>
class CachedIterator_T : public RowidIterator_i
{
public:
				CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs );

	bool		HintRowID ( RowID_t tRowID ) override;
	void		SetCutoff ( int iCutoff ) override {}	// fixme! add real cutoff
	bool		WasCutoffHit() const override { return false; }

protected:
	static const int MAX_COLLECTED = 512;
	CSphFixedVector<RowID_t> m_dCollected {MAX_COLLECTED};

	CSphTightVector<RowID_t> m_dRowIDs;
	int			m_iId = 0;

	BitVec_T<uint64_t> m_tBitmap;
	RowID_t		m_tRowID = 0;
	RowID_t		m_tMinRowID = INVALID_ROWID;
	RowID_t		m_tMaxRowID = INVALID_ROWID;
	bool		m_bFirstTime = true;

	FORCE_INLINE void Add ( RowID_t tRowID );
	bool		Finalize();
	bool		ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock );
};

template <>
CachedIterator_T<true>::CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs )
{
	m_tBitmap.Init(uTotalDocs);
}

template <>
CachedIterator_T<false>::CachedIterator_T ( int64_t iRsetEstimate, DWORD uTotalDocs )
{
	m_dRowIDs.Reserve(iRsetEstimate);
}

template <>
bool CachedIterator_T<true>::HintRowID ( RowID_t tRowID )
{
	if ( tRowID<m_tRowID )
		return true;

	m_tRowID = tRowID;
	while ( m_tRowID<=m_tMaxRowID )
	{
		if ( m_tBitmap.BitGet(m_tRowID) )
			return true;

		m_tRowID++;
	}
	
	return false;
}

template <>
bool CachedIterator_T<false>::HintRowID ( RowID_t tRowID )
{
	RowID_t * pRowID = m_dRowIDs.Begin() + m_iId;
	RowID_t * pRowIdMax = m_dRowIDs.End();

	if ( m_dRowIDs.IsEmpty() || pRowID>=pRowIdMax )
		return false;

	const int64_t LINEAR_THRESH = 256;
	if ( tRowID - *pRowID < LINEAR_THRESH )
	{
		const RowID_t * pRowIdStart = m_dRowIDs.Begin();

		while ( pRowID<pRowIdMax && *pRowID<tRowID )
			pRowID++;

		m_iId = pRowID-pRowIdStart;
		return pRowID<pRowIdMax;
	}
	else
	{
		// we assume we are never rewinding backwards
		const RowID_t * pFound = sphBinarySearchFirst ( pRowID, pRowIdMax-1, SphIdentityFunctor_T<RowID_t>(), tRowID );
		assert(pFound);

		if ( *pFound < tRowID )
			return false;

		m_iId = pFound-m_dRowIDs.Begin();
		return true;
	}
}

template <>
void CachedIterator_T<true>::Add ( RowID_t tRowID )
{
	if ( m_tMinRowID==INVALID_ROWID )
	{
		m_tMinRowID = tRowID;
		m_tMaxRowID = tRowID;
	}
	else
	{
		m_tMinRowID = Min ( m_tMinRowID, tRowID );
		m_tMaxRowID = Max ( m_tMaxRowID, tRowID );
	}

	m_tBitmap.BitSet(tRowID);
}

template<>
void CachedIterator_T<false>::Add ( RowID_t tRowID )
{
	m_dRowIDs.Add(tRowID);
}

template<>
bool CachedIterator_T<true>::Finalize()
{
	m_tRowID = m_tMinRowID;
	return m_tMinRowID!=INVALID_ROWID;
}

template<>
bool CachedIterator_T<false>::Finalize()
{
	m_dRowIDs.Sort();
	return !m_dRowIDs.IsEmpty();
}

template <>
bool CachedIterator_T<true>::ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + m_dCollected.GetLength();
	RowID_t * pRowID = pRowIdStart;

	while ( m_tRowID<=m_tMaxRowID && pRowID<pRowIdMax )
	{
		if ( m_tBitmap.BitGet(m_tRowID) )
			*pRowID++ = m_tRowID;

		m_tRowID++;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <>
bool CachedIterator_T<false>::ReturnRowIdChunk ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dRowIDs.Begin() + m_iId;
	RowID_t * pRowID = Min ( pRowIdStart+m_dCollected.GetLength(), m_dRowIDs.End() );
	m_iId += pRowID-pRowIdStart;

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupValues_T : public CachedIterator_T<BITMAP>
{
	using BASE = CachedIterator_T<BITMAP>;

public:
						RowidIterator_LookupValues_T ( const VecTraits_T<DocID_t>& tValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	bool				HintRowID ( RowID_t tRowID ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "DocidIndex" } ); }

private:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocidListReader_c	m_tFilterReader;

	bool				Fill();
	FORCE_INLINE bool	FillIfFirstTime();
};

template <bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupValues_T<ROWID_LIMITS, BITMAP>::RowidIterator_LookupValues_T ( const VecTraits_T<DocID_t>& tValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_tLookupReader ( pDocidLookup )
	, m_tFilterReader ( tValues )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( !FillIfFirstTime() )
		return false;

	return BASE::ReturnRowIdChunk(dRowIdBlock);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::HintRowID ( RowID_t tRowID )
{
	if ( !FillIfFirstTime() )
		return false;

	return BASE::HintRowID(tRowID);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t	tLookupDocID = 0;
	DocID_t	tFilterDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;
	bool bHaveFilterDocs = m_tFilterReader.ReadDocID(tFilterDocID);
	bool bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
	m_iProcessed += bHaveFilterDocs ? 1 : 0;
	m_iProcessed += bHaveLookupDocs ? 1 : 0;

	while ( bHaveFilterDocs && bHaveLookupDocs )
	{
		if ( tFilterDocID < tLookupDocID )
		{
			m_tFilterReader.HintDocID(tLookupDocID);
			bHaveFilterDocs = m_tFilterReader.ReadDocID ( tFilterDocID );
		}
		else if ( tFilterDocID > tLookupDocID )
		{
			m_tLookupReader.HintDocID(tFilterDocID);
			bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
		}
		else
		{
			// lookup reader can have duplicates; filter reader can't have duplicates
			// advance only the lookup reader
			if ( ROWID_LIMITS )
			{
				if ( tLookupRowID>=m_tBoundaries.m_tMinRowID && tLookupRowID<=m_tBoundaries.m_tMaxRowID )
					BASE::Add(tLookupRowID);
			}
			else
				BASE::Add(tLookupRowID);

			bHaveLookupDocs = m_tLookupReader.Read ( tLookupDocID, tLookupRowID );
		}

		m_iProcessed++;
	}

	return BASE::Finalize();
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::FillIfFirstTime()
{
	if ( !BASE::m_bFirstTime )
		return true;

	BASE::m_bFirstTime = false;
	return Fill();
}

//////////////////////////////////////////////////////////////////////////
class DocIdCheck_i
{
public:
	virtual							~DocIdCheck_i() = default;

	virtual void					Init() {};
	virtual void					DisableRewinding() = 0;
	virtual std::pair<bool,bool>	Check ( uint64_t uValue ) = 0;
};


class DocIdCheck_c : public DocIdCheck_i
{
public:
	DocIdCheck_c ( uint64_t uValue, std::shared_ptr<LookupReaderIterator_c> & pReader )
		: m_uValue ( uValue )
		, m_pReader ( pReader )
	{}

	 void		DisableRewinding() override { m_bCanRewind = false; }

protected:
	static constexpr uint64_t MIN_NEG = uint64_t(INT64_MIN);

	uint64_t	m_uValue;
	bool		m_bCanRewind = true;
	bool		m_bRewound = false;
	std::shared_ptr<LookupReaderIterator_c> m_pReader;
};

template <bool EQ>
class GtPos_T : public DocIdCheck_c
{
	using DocIdCheck_c::DocIdCheck_c;

public:
	void Init() override
	{
		if ( m_bCanRewind )
			m_pReader->HintDocID(m_uValue);
	}

	std::pair<bool,bool> Check ( uint64_t uValue ) override
	{
		if ( uValue >= MIN_NEG )
			return { false, true };

		if_const ( EQ )
			return { uValue >= m_uValue, false };

		return {  uValue > m_uValue, false };
	}
};

template <bool EQ>
class LtPos_T : public DocIdCheck_c
{
	using DocIdCheck_c::DocIdCheck_c;

public:
	std::pair<bool,bool> Check ( uint64_t uValue ) override
	{
		if_const ( EQ )
		{
			if ( uValue <= m_uValue ) return { true, false };
		}
		else
			if ( uValue < m_uValue ) return { true, false };

		if ( uValue < MIN_NEG )
		{
			if ( m_bCanRewind && !m_bRewound )
			{
				m_bRewound = true;
				m_pReader->HintDocID(MIN_NEG);
			}

			return { false, false };
		}

		return { true, false };
	}
};


template <bool EQ>
class GtNeg_T : public DocIdCheck_c
{
	using DocIdCheck_c::DocIdCheck_c;

public:
	inline std::pair<bool,bool> Check ( uint64_t uValue ) override
	{
		if ( uValue < MIN_NEG )
			return { true, false };

		if ( m_bCanRewind && !m_bRewound && uValue>=MIN_NEG )
		{
			m_pReader->HintDocID(m_uValue);
			m_bRewound = true;
			return { false, false };
		}

		if_const ( EQ )
			return { uValue >= m_uValue, false };

		return { uValue > m_uValue, false };
	}
};

template <bool EQ>
class LtNeg_T : public DocIdCheck_c
{
	using DocIdCheck_c::DocIdCheck_c;

public:
	void Init() override
	{
		if ( m_bCanRewind )
			m_pReader->HintDocID(MIN_NEG);
	}

	inline std::pair<bool,bool> Check ( uint64_t uValue ) override
	{
		if ( uValue < MIN_NEG )
			return { false, false };

		if_const ( EQ )
		{
			if ( uValue <= m_uValue ) return { true, false };
		}
		else
			if ( uValue < m_uValue ) return { true, false };

		return { false, true };
	}
};

template <bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRange_T : public CachedIterator_T<BITMAP>
{
	using BASE = CachedIterator_T<BITMAP>;

public:
						RowidIterator_LookupRange_T ( std::shared_ptr<LookupReaderIterator_c> & pReader, DocIdCheck_i * pCheck1, DocIdCheck_i * pCheck2, int64_t iRsetEstimate, DWORD uTotalDocs, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	bool				HintRowID ( RowID_t tRowID ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "DocidIndex" } ); }

protected:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	std::shared_ptr<LookupReaderIterator_c> m_pReader;
	std::unique_ptr<DocIdCheck_i> m_pCheck1;
	std::unique_ptr<DocIdCheck_i> m_pCheck2;

	virtual bool		Fill();
	FORCE_INLINE bool	FillIfFirstTime();
};

template <bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupRange_T<ROWID_LIMITS,BITMAP>::RowidIterator_LookupRange_T ( std::shared_ptr<LookupReaderIterator_c> & pReader, DocIdCheck_i * pCheck1, DocIdCheck_i * pCheck2, int64_t iRsetEstimate, DWORD uTotalDocs, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_pReader ( pReader )
	, m_pCheck1 ( pCheck1 )
	, m_pCheck2 ( pCheck2 )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( !FillIfFirstTime() )
		return false;

	return BASE::ReturnRowIdChunk(dRowIdBlock);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<ROWID_LIMITS,BITMAP>::HintRowID ( RowID_t tRowID )
{
	if ( !FillIfFirstTime() )
		return false;

	return BASE::HintRowID(tRowID);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;
	while ( m_pReader->Read ( tLookupDocID, tLookupRowID ) )
	{
		m_iProcessed++;

		if ( m_pCheck1 )
		{
			auto [ bAccept, bBreak ] = m_pCheck1->Check(tLookupDocID);
			if ( bBreak )
				break;

			if ( !bAccept )
				continue;
		}

		if ( m_pCheck2 )
		{
			auto [ bAccept, bBreak ] = m_pCheck2->Check(tLookupDocID);
			if ( bBreak )
				break;

			if ( !bAccept )
				continue;
		}

		if ( ROWID_LIMITS )
		{
			if ( tLookupRowID >= m_tBoundaries.m_tMinRowID && tLookupRowID <= m_tBoundaries.m_tMaxRowID )
				BASE::Add(tLookupRowID);
		}
		else
			BASE::Add(tLookupRowID);
	}

	return BASE::Finalize();
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<ROWID_LIMITS,BITMAP>::FillIfFirstTime()
{
	if ( !BASE::m_bFirstTime )
		return true;

	if ( m_pCheck1 )
		m_pCheck1->Init();

	if ( m_pCheck2 )
		m_pCheck2->Init();

	BASE::m_bFirstTime = false;
	return Fill();
}

//////////////////////////////////////////////////////////////////////////

template <bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRangeExclude_T : public RowidIterator_LookupRange_T<ROWID_LIMITS, BITMAP>
{
	using BASE = RowidIterator_LookupRange_T<ROWID_LIMITS, BITMAP>;
	using BASE::BASE;

protected:
	bool	Fill() override;

private:
	void	Add ( RowID_t tLookupRowID );
};

template <bool ROWID_LIMITS, bool BITMAP>
void RowidIterator_LookupRangeExclude_T<ROWID_LIMITS,BITMAP>::Add ( RowID_t tLookupRowID )
{
	if ( ROWID_LIMITS )
	{
		if ( tLookupRowID>=BASE::m_tBoundaries.m_tMinRowID && tLookupRowID<=BASE::m_tBoundaries.m_tMaxRowID )
			BASE::Add(tLookupRowID);
	}
	else
		BASE::Add(tLookupRowID);
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRangeExclude_T<ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && ( BASE::m_pCheck1 || BASE::m_pCheck2 ) && BASE::m_pReader->Read ( tLookupDocID, tLookupRowID ) )
	{
		BASE::m_iProcessed++;

		if ( BASE::m_pCheck1 )
		{
			auto [ bAccept, bBreak ] = BASE::m_pCheck1->Check(tLookupDocID);
			if ( bBreak )
				BASE::m_pCheck1.reset();

			if ( bAccept )
			{
				Add ( tLookupRowID );
				continue;
			}
		}

		if ( BASE::m_pCheck2 )
		{
			auto [ bAccept, bBreak ] = BASE::m_pCheck2->Check(tLookupDocID);
			if ( bBreak )
				BASE::m_pCheck2.reset();

			if ( bAccept )
			{
				Add ( tLookupRowID );
				continue;
			}
		}
	}

	return BASE::Finalize();
}

//////////////////////////////////////////////////////////////////////////

static bool NeedBitmapStorage ( int64_t iRsetSize, DWORD uTotalDocs )
{
	return float(iRsetSize)/uTotalDocs > 0.001f;
}


static DocIdCheck_i * CreateCheckGt ( int64_t iMinValue, bool bHasEqualMin, std::shared_ptr<LookupReaderIterator_c> & pReader )
{
	int iIndex = ( iMinValue>=0 ? 1 : 0 )*2 + ( bHasEqualMin ? 1 : 0 );
	uint64_t uVal = uint64_t(iMinValue);
	switch ( iIndex )
	{
	case 0: return new GtNeg_T<false> ( uVal, pReader );
	case 1: return new GtNeg_T<true>  ( uVal, pReader );
	case 2: return new GtPos_T<false> ( uVal, pReader );
	case 3: return new GtPos_T<true>  ( uVal, pReader );
	default: return nullptr;
	}
}


static DocIdCheck_i * CreateCheckLt ( int64_t iMaxValue, bool bHasEqualMax, std::shared_ptr<LookupReaderIterator_c> & pReader )
{
	int iIndex = ( iMaxValue>=0 ? 1 : 0 )*2 + ( bHasEqualMax ? 1 : 0 );
	uint64_t uVal = uint64_t(iMaxValue);
	switch ( iIndex )
	{
	case 0: return new LtNeg_T<false> ( uVal, pReader );
	case 1: return new LtNeg_T<true>  ( uVal, pReader );
	case 2: return new LtPos_T<false> ( uVal, pReader );
	case 3:	return new LtPos_T<true>  ( uVal, pReader );
	default: return nullptr;
	}
}

#define DECL_CREATEVALUES( _, n, params ) case n: return new RowidIterator_LookupValues_T<!!( n & 2 ), !!( n & 1 )> params;

static RowidIterator_i * CreateRowidLookupRange ( std::shared_ptr<LookupReaderIterator_c> & pReader, DocIdCheck_i * pCheck1, DocIdCheck_i * pCheck2, int64_t iRsetEstimate, DWORD uTotalDocs, const RowIdBoundaries_t * pBoundaries, bool bBitmap )
{
	int iIndex = ( pBoundaries ? 1 : 0 )*2 + ( bBitmap ? 1 : 0 );
	switch ( iIndex )
	{
	case 0: return new RowidIterator_LookupRange_T<false, false> ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 1: return new RowidIterator_LookupRange_T<false, true>  ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 2: return new RowidIterator_LookupRange_T<true, false>  ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 3: return new RowidIterator_LookupRange_T<true, true>   ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static RowidIterator_i * CreateRowidLookupRangeExclude ( std::shared_ptr<LookupReaderIterator_c> & pReader, DocIdCheck_i * pCheck1, DocIdCheck_i * pCheck2, int64_t iRsetEstimate, DWORD uTotalDocs, const RowIdBoundaries_t * pBoundaries, bool bBitmap )
{
	int iIndex = ( pBoundaries ? 1 : 0 )*2 + ( bBitmap ? 1 : 0 );
	switch ( iIndex )
	{
	case 0: return new RowidIterator_LookupRangeExclude_T<false, false> ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 1: return new RowidIterator_LookupRangeExclude_T<false, true>  ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 2: return new RowidIterator_LookupRangeExclude_T<true, false>  ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	case 3: return new RowidIterator_LookupRangeExclude_T<true, true>   ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries );
	default:
		assert ( 0 && "Internal error" );
		return nullptr;
	}
}


static RowidIterator_i * CreateLookupIterator ( const CSphFilterSettings & tFilter, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
{
	if ( tFilter.m_sAttrName!=sphGetDocidName() )
		return nullptr;

	bool bBitmap = NeedBitmapStorage ( iRsetEstimate, uTotalDocs );

	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
		{
			int iIndex = !!pBoundaries * 2 + bBitmap;
			switch ( iIndex )
			{
				BOOST_PP_REPEAT ( 4, DECL_CREATEVALUES, ( tFilter.GetValues(), iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
				default: assert ( 0 && "Internal error" ); return nullptr;
			}
		}
		break;

	case SPH_FILTER_RANGE:
		{
			auto pReader = std::make_shared<LookupReaderIterator_c>(pDocidLookup);
			
			if ( tFilter.m_bExclude )
			{
				DocIdCheck_i * pCheck1 = tFilter.m_bOpenLeft ? nullptr : CreateCheckLt ( tFilter.m_iMinValue, !tFilter.m_bHasEqualMin, pReader );
				DocIdCheck_i * pCheck2 = tFilter.m_bOpenRight ? nullptr : CreateCheckGt ( tFilter.m_iMaxValue, !tFilter.m_bHasEqualMax, pReader );
				if ( pCheck1 && pCheck2 )
				{
					pCheck1->DisableRewinding();
					pCheck2->DisableRewinding();
				}

				return CreateRowidLookupRangeExclude ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries, bBitmap );
			}

			DocIdCheck_i * pCheck1 = tFilter.m_bOpenLeft ? nullptr : CreateCheckGt ( tFilter.m_iMinValue, tFilter.m_bHasEqualMin, pReader );
			DocIdCheck_i * pCheck2 = tFilter.m_bOpenRight ? nullptr : CreateCheckLt ( tFilter.m_iMaxValue, tFilter.m_bHasEqualMax, pReader );
			return CreateRowidLookupRange ( pReader, pCheck1, pCheck2, iRsetEstimate, uTotalDocs, pBoundaries, bBitmap ); 
		}

	default:
		break;
	}

	return nullptr;
}

#undef DECL_CREATEVALUES
#undef DECL_CREATERANGEEX
#undef DECL_CREATERANGE


RowIteratorsWithEstimates_t CreateLookupIterator ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, const BYTE * pDocidLookup, uint32_t uTotalDocs )
{
	RowIdBoundaries_t tBoundaries;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, uTotalDocs, tBoundaries );

	RowIteratorsWithEstimates_t dIterators;

	ARRAY_FOREACH ( i, dSIInfo )
	{
		auto & tSIInfo = dSIInfo[i];
		if ( tSIInfo.m_eType!=SecondaryIndexType_e::LOOKUP )
			continue;

		RowidIterator_i * pIterator = CreateLookupIterator ( dFilters[i], tSIInfo.m_iRsetEstimate, uTotalDocs, pDocidLookup, pRowIdFilter ? &tBoundaries : nullptr );
		if ( pIterator )
		{
			dIterators.Add ( { pIterator, tSIInfo.m_iRsetEstimate } );
			tSIInfo.m_bCreated = true;
		}
	}

	return dIterators;
}

//////////////////////////////////////////////////////////////////////////

DocidLookupWriter_c::DocidLookupWriter_c ( CSphWriter& tWriter, DWORD nDocs )
	: m_tWriter { tWriter }
	, m_nDocs ( nDocs )
{}


void DocidLookupWriter_c::Start()
{
	m_tWriter.PutDword ( m_nDocs );
	m_tWriter.PutDword ( DOCS_PER_LOOKUP_CHECKPOINT );

	m_tCheckpointStart = m_tWriter.GetPos();
	m_tWriter.PutOffset ( 0 );	// reserve space for max docid

	int nCheckpoints = (m_nDocs+DOCS_PER_LOOKUP_CHECKPOINT-1)/DOCS_PER_LOOKUP_CHECKPOINT;
	m_dCheckpoints.Reset ( nCheckpoints );
	for ( int i = 0; i < nCheckpoints; ++i )
	{
		// reserve space for checkpoints
		m_tWriter.PutOffset(0);
		m_tWriter.PutOffset(0);
	}
}


void DocidLookupWriter_c::AddPair ( const DocidRowidPair_t & tPair )
{
	assert ( (uint64_t)tPair.m_tDocID>=(uint64_t)m_tLastDocID );

	if ( !(m_iProcessed % DOCS_PER_LOOKUP_CHECKPOINT) )
	{
		m_dCheckpoints[m_iCheckpoint].m_tBaseDocID = tPair.m_tDocID;
		m_dCheckpoints[m_iCheckpoint].m_tOffset = m_tWriter.GetPos();
		++m_iCheckpoint;

		// no need to store docid for 1st entry
	}
	else
	{
		m_tWriter.ZipOffset ( (uint64_t)tPair.m_tDocID-(uint64_t)m_tLastDocID );
	}

	m_tWriter.PutDword ( tPair.m_tRowID );

	m_tLastDocID = tPair.m_tDocID;
	++m_iProcessed;
}


bool DocidLookupWriter_c::Finalize ( CSphString & sError )
{
	m_tWriter.Flush();
	m_tWriter.SeekTo ( m_tCheckpointStart );
	m_tWriter.PutOffset ( m_tLastDocID );
	for ( const auto & i : m_dCheckpoints )
	{
		m_tWriter.PutOffset ( i.m_tBaseDocID );
		m_tWriter.PutOffset ( i.m_tOffset );
	}

	m_tWriter.CloseFile();
	if ( m_tWriter.IsError() )
	{
		sError = "error writing .SPT";
		return false;
	}

	return true;
}

bool WriteDocidLookup ( const CSphString & sFilename, const VecTraits_T<DocidRowidPair_t> & dLookup, CSphString & sError )
{
	CSphWriter tfWriter;
	if ( !tfWriter.OpenFile ( sFilename, sError ) )
		return false;

	DocidLookupWriter_c tWriter ( tfWriter, dLookup.GetLength() );
	tWriter.Start();

	for ( const auto & i : dLookup )
		tWriter.AddPair(i);

	return tWriter.Finalize ( sError );
}

//////////////////////////////////////////////////////////////////////////

LookupReader_c::LookupReader_c ( const BYTE * pData )
{
	SetData ( pData );
}


void LookupReader_c::SetData ( const BYTE * pData )
{
	m_pData = pData;

	if ( !pData )
		return;

	const BYTE * p = pData;
	m_nDocs = *(const DWORD*)p;
	p += sizeof(DWORD);
	m_nDocsPerCheckpoint = *(const DWORD*)p;
	p += sizeof(DWORD);
	m_tMaxDocID = *(const DocID_t*)p;
	p += sizeof(DocID_t);

	m_nCheckpoints = (m_nDocs+m_nDocsPerCheckpoint-1)/m_nDocsPerCheckpoint;
	m_pCheckpoints = (const DocidLookupCheckpoint_t *)p;
}

//////////////////////////////////////////////////////////////////////////

LookupReaderIterator_c::LookupReaderIterator_c ( const BYTE * pData )
{
	SetData(pData);
}


void LookupReaderIterator_c::SetData ( const BYTE * pData )
{
	LookupReader_c::SetData(pData);
	SetCheckpoint ( m_pCheckpoints );
}
