//
//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
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

	void		Add ( RowID_t tRowID );
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
bool CachedIterator_T<true>::HintRowID  ( RowID_t tRowID )
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
bool CachedIterator_T<false>::HintRowID  ( RowID_t tRowID )
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
						RowidIterator_LookupValues_T ( const SphAttr_t * pValues, int nValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "lookup" } ); }

private:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocidListReader_c	m_tFilterReader;
	bool				m_bFirstTime = true;

	bool				Fill();
};

template <bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupValues_T<ROWID_LIMITS, BITMAP>::RowidIterator_LookupValues_T ( const SphAttr_t * pValues, int nValues, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_tLookupReader ( pDocidLookup )
	, m_tFilterReader ( pValues, nValues )
{
	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupValues_T<ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bFirstTime )
	{
		m_bFirstTime = false;
		if ( !Fill() )
			return false;
	}

	return BASE::ReturnRowIdChunk(dRowIdBlock);
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

//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRange_T : public CachedIterator_T<BITMAP>
{
	using BASE = CachedIterator_T<BITMAP>;

public:
						RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries = nullptr );

	bool				GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;
	int64_t				GetNumProcessed() const override { return m_iProcessed; }
	void				AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override { dDesc.Add ( { "id", "lookup" } ); }

protected:
	RowIdBoundaries_t	m_tBoundaries;
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocID_t				m_tMinValue {0};
	DocID_t				m_tMaxValue {0};
	bool				m_bFirstTime = true;

private:
	bool				Fill();
};

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: BASE ( iRsetEstimate, uTotalDocs )
	, m_tLookupReader ( pDocidLookup )
	, m_tMinValue ( tMinValue )
	, m_tMaxValue ( tMaxValue )
{
	if_const ( !OPEN_LEFT )
		m_tLookupReader.HintDocID ( tMinValue );

	if ( pBoundaries )
		m_tBoundaries = *pBoundaries;
}

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if ( m_bFirstTime )
	{
		m_bFirstTime = false;
		if ( !Fill() )
			return false;
	}

	return BASE::ReturnRowIdChunk(dRowIdBlock);
}

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::Fill()
{
	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		m_iProcessed++;

		if ( !OPEN_LEFT && ( tLookupDocID < m_tMinValue || ( !HAS_EQUAL_MIN && tLookupDocID==m_tMinValue ) ) )
			continue;

		if ( !OPEN_RIGHT && ( tLookupDocID > m_tMaxValue || ( !HAS_EQUAL_MAX && tLookupDocID==m_tMaxValue ) ) )
			break;

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

//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
class RowidIterator_LookupRangeExclude_T : public RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>
{
	using BASE = RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>;

public:
			RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries );

	bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override;

private:
	bool	m_bLeft {true};
};


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, int64_t iRsetEstimate, DWORD uTotalDocs, const BYTE * pDocidLookup, const RowIdBoundaries_t * pBoundaries )
	: RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP> ( tMinValue, tMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries )
{
	if ( OPEN_LEFT && !OPEN_RIGHT )
		this->m_tLookupReader.HintDocID ( this->m_tMaxValue );

	m_bLeft = OPEN_RIGHT || ( !OPEN_LEFT && !OPEN_RIGHT ); 
}


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT, bool ROWID_LIMITS, bool BITMAP>
bool RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT,ROWID_LIMITS,BITMAP>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	if_const ( OPEN_LEFT && OPEN_RIGHT )
		return false;

	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	RowID_t * pRowIdStart = BASE::m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + BASE::m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && BASE::m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		BASE::m_iProcessed++;

		if ( m_bLeft )
		{
			// use everything ending with m_tMinValue
			if ( tLookupDocID > BASE::m_tMinValue || ( HAS_EQUAL_MIN && tLookupDocID==BASE::m_tMinValue ) )
			{
				// switch to right interval
				if_const ( !OPEN_LEFT && !OPEN_RIGHT )
				{
					m_bLeft = false;
					this->m_tLookupReader.HintDocID ( this->m_tMaxValue );
					continue;
				}

				return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
			}
		}
		else
		{
			// use everything starting from m_tMaxValue
			if ( tLookupDocID < BASE::m_tMaxValue || ( HAS_EQUAL_MAX && tLookupDocID==BASE::m_tMaxValue ) )
				continue;
		}

		if ( ROWID_LIMITS )
		{
			if ( tLookupRowID>=BASE::m_tBoundaries.m_tMinRowID && tLookupRowID<=BASE::m_tBoundaries.m_tMaxRowID )
				*pRowID++ = tLookupRowID;
		}
		else
			*pRowID++ = tLookupRowID;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

//////////////////////////////////////////////////////////////////////////

static bool NeedBitmapStorage ( int64_t iRsetSize, DWORD uTotalDocs )
{
	return float(iRsetSize)/uTotalDocs > 0.05f;
}

#define DECL_CREATEVALUES( _, n, params ) case n: return new RowidIterator_LookupValues_T<!!( n & 2 ), !!( n & 1 )> params;
#define DECL_CREATERANGEEX( _, n, params ) case n: return new RowidIterator_LookupRangeExclude_T<!!( n & 32 ), !!( n & 16 ), !!( n & 8 ), !!( n & 4 ), !!( n & 2 ), !!( n & 1 )> params;
#define DECL_CREATERANGE( _, n, params ) case n: return new RowidIterator_LookupRange_T<!!( n & 32 ), !!( n & 16 ), !!( n & 8 ), !!( n & 4 ), !!( n & 2 ), !!( n & 1 )> params;

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
				BOOST_PP_REPEAT ( 4, DECL_CREATEVALUES, ( tFilter.GetValueArray(), tFilter.GetNumValues(), iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
				default: assert ( 0 && "Internal error" ); return nullptr;
			}
		}
		break;

	case SPH_FILTER_RANGE:
		{
			int iIndex = tFilter.m_bHasEqualMin * 32 + tFilter.m_bHasEqualMax * 16 + tFilter.m_bOpenLeft * 8 + tFilter.m_bOpenRight * 4 + !!pBoundaries * 2 + bBitmap;
			if ( tFilter.m_bExclude )
				switch ( iIndex )
				{
					BOOST_PP_REPEAT ( 64, DECL_CREATERANGEEX, ( tFilter.m_iMinValue, tFilter.m_iMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
					default: assert ( 0 && "Internal error" ); return nullptr;
				}
			else
				switch ( iIndex )
				{
					BOOST_PP_REPEAT ( 64, DECL_CREATERANGE, ( tFilter.m_iMinValue, tFilter.m_iMaxValue, iRsetEstimate, uTotalDocs, pDocidLookup, pBoundaries ) )
					default: assert ( 0 && "Internal error" ); return nullptr;
				}
		}
		break;

	default:
		break;
	}

	return nullptr;
}

#undef DECL_CREATEVALUES
#undef DECL_CREATERANGEEX
#undef DECL_CREATERANGE


CSphVector<RowidIterator_i *> CreateLookupIterator ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, const BYTE * pDocidLookup, uint32_t uTotalDocs )
{
	RowIdBoundaries_t tBoundaries;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, uTotalDocs, tBoundaries );

	CSphVector<RowidIterator_i *> dIterators;

	ARRAY_FOREACH ( i, dSIInfo )
	{
		auto & tSIInfo = dSIInfo[i];
		if ( tSIInfo.m_eType!=SecondaryIndexType_e::LOOKUP )
			continue;

		RowidIterator_i * pIterator = CreateLookupIterator ( dFilters[i], tSIInfo.m_iRsetEstimate, uTotalDocs, pDocidLookup, pRowIdFilter ? &tBoundaries : nullptr );
		if ( pIterator )
		{
			dIterators.Add ( pIterator );
			tSIInfo.m_bCreated = true;
		}
	}

	return dIterators;
}

//////////////////////////////////////////////////////////////////////////

DocidLookupWriter_c::DocidLookupWriter_c ( DWORD nDocs )
	: m_nDocs ( nDocs )
{}


bool DocidLookupWriter_c::Open ( const CSphString & sFilename, CSphString & sError )
{
	assert ( !m_pWriter );
	m_pWriter = std::make_unique<CSphWriter>();

	if ( !m_pWriter->OpenFile ( sFilename, sError ) )
		return false;

	m_pWriter->PutDword ( m_nDocs );
	m_pWriter->PutDword ( DOCS_PER_LOOKUP_CHECKPOINT );

	m_tCheckpointStart = m_pWriter->GetPos();
	m_pWriter->PutOffset ( 0 );	// reserve space for max docid

	int nCheckpoints = (m_nDocs+DOCS_PER_LOOKUP_CHECKPOINT-1)/DOCS_PER_LOOKUP_CHECKPOINT;
	m_dCheckpoints.Reset ( nCheckpoints );
	for ( int i = 0; i < nCheckpoints; i++ )
	{
		// reserve space for checkpoints
		m_pWriter->PutOffset(0);
		m_pWriter->PutOffset(0);
	}

	return true;
}


void DocidLookupWriter_c::AddPair ( const DocidRowidPair_t & tPair )
{
	assert ( (uint64_t)tPair.m_tDocID>=(uint64_t)m_tLastDocID );

	if ( !(m_iProcessed % DOCS_PER_LOOKUP_CHECKPOINT) )
	{
		m_dCheckpoints[m_iCheckpoint].m_tBaseDocID = tPair.m_tDocID;
		m_dCheckpoints[m_iCheckpoint].m_tOffset = m_pWriter->GetPos();
		m_iCheckpoint++;

		// no need to store docid for 1st entry
		m_pWriter->PutDword ( tPair.m_tRowID );
	}
	else
	{
		m_pWriter->ZipOffset ( (uint64_t)tPair.m_tDocID-(uint64_t)m_tLastDocID );
		m_pWriter->PutDword ( tPair.m_tRowID );
	}

	m_tLastDocID = tPair.m_tDocID;
	m_iProcessed++;
}


bool DocidLookupWriter_c::Finalize ( CSphString & sError )
{
	m_pWriter->Flush();
	m_pWriter->SeekTo ( m_tCheckpointStart );
	m_pWriter->PutOffset ( m_tLastDocID );
	for ( const auto & i : m_dCheckpoints )
	{
		m_pWriter->PutOffset ( i.m_tBaseDocID );
		m_pWriter->PutOffset ( i.m_tOffset );
	}

	m_pWriter->CloseFile();
	if ( m_pWriter->IsError() )
	{
		sError = "error writing .SPT";
		return false;
	}

	return true;
}


CSphWriter & DocidLookupWriter_c::GetWriter()
{
	assert ( m_pWriter );
	return *m_pWriter;
}


bool WriteDocidLookup ( const CSphString & sFilename, const VecTraits_T<DocidRowidPair_t> & dLookup, CSphString & sError )
{
	DocidLookupWriter_c tWriter ( dLookup.GetLength() );
	if ( !tWriter.Open ( sFilename, sError ) )
		return false;

	for ( const auto & i : dLookup )
		tWriter.AddPair(i);

	if ( !tWriter.Finalize ( sError ) )
		return false;

	return true;
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
