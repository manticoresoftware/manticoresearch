//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxqcache.h"
#include "exprtraits.h"
#include "mini_timer.h"

//////////////////////////////////////////////////////////////////////////
// QUERY CACHE
//////////////////////////////////////////////////////////////////////////

// TODO: maybe optmized storage for const weight frames?
// TODO: stop accumulating once entry is bigger than max total cache size
// TODO: maybe estimate and report peak temporary RAM usage
// TODO: maybe account and report locking time

#define QCACHE_NO_ENTRY			(NULL)
#define QCACHE_DEAD_ENTRY		((QcacheEntry_c*)-1)

/// query cache
class Qcache_c : public QcacheStatus_t
{
private:
	CSphMutex					m_tLock;			///< hash lock
	CSphVector<QcacheEntry_c*>	m_hData;			///< our little queries hash
	int							m_iMaxQueries;		///< max load
	int							m_iMruHead;			///< most recently used entry

public:
								Qcache_c();
								~Qcache_c();

	void						Setup ( int64_t iMaxBytes, int iThreshMsec, int iTtlSec );
	void						Add ( const CSphQuery & q, QcacheEntry_c * pResult, const ISphSchema & tSorterSchema );
	QcacheEntry_c *				Find ( int64_t iIndexId, const CSphQuery & q, const ISphSchema & tSorterSchema );
	void						DeleteIndex ( int64_t iIndexId ) EXCLUDES ( m_tLock );

private:
	static uint64_t				GetKey ( int64_t iIndexId, const CSphQuery & q );
	bool						IsValidEntry ( int i ) { return m_hData[i]!=QCACHE_NO_ENTRY && m_hData[i]!=QCACHE_DEAD_ENTRY; }
	void						EnforceLimits ( bool bSizeOnly ) EXCLUDES ( m_tLock );
	void						MruToHead ( int iRes );
	void						DeleteEntry ( int iEntry );
	bool						CanCacheQuery ( const CSphQuery & q ) const;
};

/// ranker that servers cached results
class QcacheRanker_c final : public ISphRanker
{
protected:
	QcacheEntryRefPtr_t			m_pEntry;										///< cache entry we are decoding
	CSphMatch					m_dMatches [ QcacheEntry_c::MAX_FRAME_SIZE ];	///< matches buffer
	BYTE *						m_pCur;											///< current position in compressed data
	BYTE *						m_pMax;											///< max position in compressed data
	RowID_t						m_uLastId = INVALID_ROWID;						///< docid delta decoder state
	const CSphIndex *			m_pIndex;
	CSphQueryContext *			m_pCtx;

	void ResetImpl ( const ISphQwordSetup & tSetup ); // to avoid call real virtual Reset() from c-tr.

public:
	explicit					QcacheRanker_c ( QcacheEntry_c * pEntry, const ISphQwordSetup & tSetup );

	CSphMatch *					GetMatchesBuffer() final { return m_dMatches; }
	int							GetMatches() final;
	void						Reset ( const ISphQwordSetup & tSetup ) final { ResetImpl ( tSetup ); }
	bool						IsCache() const final { return true; }
	NodeEstimate_t				Estimate ( int64_t iTotalDocs ) const final { return { 0.0f, 0, 0 }; }
};

/// query cache instance
static Qcache_c						g_Qcache;

//////////////////////////////////////////////////////////////////////////
void QcacheEntry_c::Append ( RowID_t uRowid, DWORD uWeight )
{
	m_iTotalMatches++;

	QcacheMatch_t & m = m_dFrame.Add();
	m.m_tRowID = uRowid;
	m.m_uWeight = uWeight;

	if ( m_dFrame.GetLength()==MAX_FRAME_SIZE )
		FlushFrame();
}


static inline int NumBytes ( RowID_t tValue )
{
	int iRes = 0;
	while ( tValue!=0 )
	{
		tValue >>= 8;
		iRes++;
	}
	return iRes;
}


void QcacheEntry_c::RankerReset()
{
	FlushFrame();

	// 01000000 is a delta restart marker
	m_dData.Add(0x40);
	m_tLastRowID = INVALID_ROWID;
}


void QcacheEntry_c::FlushFrame()
{
	/////////////////////////////
	// store an incomplete frame
	/////////////////////////////

	if ( !m_dFrame.GetLength() )
		return;

	if ( m_dFrame.GetLength()<MAX_FRAME_SIZE )
	{
		// begin with two marker bytes
		// 100wwddd 00lllll
		// ww = 1..4 bytes per weight
		// ddd = 1..8 bytes per delta
		// lllll = 1..31 {delta,weight} pairs
		int iDeltaBytes = 1;
		int iWeightBytes = 1;
		RowID_t tLastId = m_tLastRowID;
		ARRAY_FOREACH ( i, m_dFrame )
		{
			RowID_t tDelta = m_dFrame[i].m_tRowID - tLastId - 1;
			tLastId = m_dFrame[i].m_tRowID;
			iDeltaBytes = Max ( iDeltaBytes, NumBytes ( tDelta ) );
			iWeightBytes = Max ( iWeightBytes, NumBytes ( m_dFrame[i].m_uWeight ) );
		}

		assert ( iDeltaBytes>=1 && iDeltaBytes<=8 );
		assert ( iWeightBytes>=1 && iWeightBytes<=4 );
		m_dData.Add ( (BYTE)( 0x80 + ( iDeltaBytes-1 ) + ( ( iWeightBytes-1 )<<3 ) ) );
		m_dData.Add ( (BYTE)m_dFrame.GetLength() );

		BYTE * p = m_dData.AddN ( m_dFrame.GetLength()*( iDeltaBytes + iWeightBytes ) );
		tLastId = m_tLastRowID;
		ARRAY_FOREACH ( i, m_dFrame )
		{
			RowID_t uDelta = m_dFrame[i].m_tRowID - tLastId - 1;
			tLastId = m_dFrame[i].m_tRowID;
			memcpy ( p, &uDelta, iDeltaBytes );
			p += iDeltaBytes;
			memcpy ( p, &m_dFrame[i].m_uWeight, iWeightBytes );
			p += iWeightBytes;
		}

		m_dFrame.Resize(0);
		return;
	}

	///////////////////////////
	// store a complete frame
	///////////////////////////

	assert ( m_dFrame.GetLength()==MAX_FRAME_SIZE );

	// frame begins with a marker byte
	// marker format is 00iwwddd, with bit meanings as follows
	// i = weights can be either indexed or stored directly
	// ww = weight indexes (when i==1) can use 1..2 bytes per index
	// ww = weight values (when i==0) can use 1..4 bytes per weight
	// ddd = docid deltas can use 0..7 bytes per delta

	RowID_t uLastId = m_tLastRowID;
	int iDeltaBytes = 1;

	bool bIndexWeights = ( m_hWeights.GetLength() + MAX_FRAME_SIZE )<=0xffff;
	int iWeightBytes = 1;

	ARRAY_FOREACH ( i, m_dFrame )
	{
		RowID_t tDelta = m_dFrame[i].m_tRowID - uLastId - 1;
		iDeltaBytes = Max ( iDeltaBytes, NumBytes ( tDelta ) );
		uLastId = m_dFrame[i].m_tRowID;

		if ( bIndexWeights )
			m_dFrame[i].m_uWeight = m_hWeights.FindOrAdd ( m_dFrame[i].m_uWeight, (int)m_hWeights.GetLength() );
		iWeightBytes = Max ( iWeightBytes, NumBytes ( m_dFrame[i].m_uWeight ) );
	}

	// add marker byte
	assert ( iDeltaBytes>=1 && iDeltaBytes<=8 );
	assert ( iWeightBytes>=1 && iWeightBytes<=4 );
	m_dData.Add ( (BYTE)( ( bIndexWeights<<5 ) + ( ( iWeightBytes-1 )<<3 ) + ( iDeltaBytes-1 ) ) );

	// encode data
	BYTE * p = m_dData.AddN ( MAX_FRAME_SIZE*( iDeltaBytes + iWeightBytes ) );
	uLastId = m_tLastRowID;
	ARRAY_FOREACH ( i, m_dFrame )
	{
		RowID_t tDelta = m_dFrame[i].m_tRowID - uLastId - 1;
		memcpy ( p, &tDelta, iDeltaBytes );
		p += iDeltaBytes;
		uLastId = m_dFrame[i].m_tRowID;

		memcpy ( p, &m_dFrame[i].m_uWeight, iWeightBytes );
		p += iWeightBytes;
	}
	assert ( p==( m_dData.Begin() + m_dData.GetLength() ) );
	m_tLastRowID = m_dFrame.Last().m_tRowID;

	m_dFrame.Resize(0);
}


void QcacheEntry_c::Finish()
{
	FlushFrame();
	m_dFrame.Reset();

	// convert indexed weights from hash to array
	m_dWeights.Resize ( m_hWeights.GetLength() );
	m_dWeights.Fill ( -1 );

	int64_t i = 0;
	int iWeight;
	int * pIndex;
	while ( ( pIndex = m_hWeights.Iterate ( &i, &iWeight ) )!=NULL )
	{
		assert ( *pIndex>=0 && *pIndex<m_dWeights.GetLength() );
		m_dWeights [ *pIndex ] = iWeight;
	}

#ifndef NDEBUG
	ARRAY_FOREACH ( iCheck, m_dWeights )
		assert ( m_dWeights[iCheck]>=0 );
#endif

	m_hWeights.Reset(0);
	m_iElapsedMsec = (int)( ( sphMicroTimer() - m_tmStarted + 500 )/1000 );
}

//////////////////////////////////////////////////////////////////////////

Qcache_c::Qcache_c()
{
	// defaults are here
	m_iMaxBytes = 16777216;
#ifndef NDEBUG
	m_iMaxBytes = 0; // disable qcache in debug builds
#endif

	m_iThreshMs = 3000;
	m_iTtlS = 60;

	m_iCachedQueries = 0;
	m_iUsedBytes = 0;
	m_iHits = 0;
	m_iMruHead = -1;

	m_hData.Resize ( 256 );
	m_hData.Fill ( QCACHE_NO_ENTRY );
	m_iMaxQueries = (int)( m_hData.GetLength()*0.7f );
}

Qcache_c::~Qcache_c()
{
	ScopedMutex_t dLock ( m_tLock );
	ARRAY_FOREACH ( i, m_hData )
		if ( IsValidEntry(i) )
			SafeRelease ( m_hData[i] );
}

void Qcache_c::Setup ( int64_t iMaxBytes, int iThreshMsec, int iTtlSec )
{
	m_iMaxBytes = Max ( iMaxBytes, 0 );
	m_iThreshMs = Max ( iThreshMsec, 0 );
	m_iTtlS = Max ( iTtlSec, 1 );
	EnforceLimits ( false );
}

void Qcache_c::MruToHead ( int iRes )
{
	// already the head? nothing to do
	if ( iRes==m_iMruHead )
		return;

	// detach from previous node, prev.next = my.next
	QcacheEntry_c * p = m_hData[iRes];
	if ( p->m_iMruPrev>=0 )
		m_hData [ p->m_iMruPrev ]->m_iMruNext = p->m_iMruNext;

	// detach from next node, next.prev = my.prev
	if ( p->m_iMruNext>=0 )
		m_hData [ p->m_iMruNext ]->m_iMruPrev = p->m_iMruPrev;

	// become the new head
	p->m_iMruPrev = -1;
	p->m_iMruNext = m_iMruHead;
	if ( p->m_iMruNext>=0 )
	{
		assert ( m_hData [ p->m_iMruNext ]->m_iMruPrev<0 );
		m_hData [ p->m_iMruNext ]->m_iMruPrev = iRes;
	}
	m_iMruHead = iRes;
}


static bool CalcFilterHashes ( CSphVector<uint64_t> & dFilters, const CSphQuery & q, const ISphSchema & tSorterSchema )
{
	dFilters.Resize(0);

	ARRAY_FOREACH ( i, q.m_dFilters )
	{
		const CSphFilterSettings & tFS = q.m_dFilters[i];
		uint64_t uFilterHash = q.m_dFilters[i].GetHash();

		// need this cast because ISphExpr::Command is not const
		CSphColumnInfo * pAttr = const_cast<CSphColumnInfo *>(tSorterSchema.GetAttr ( tFS.m_sAttrName.cstr() ));
		if ( pAttr )
		{
			if ( pAttr->m_pExpr )
			{
				bool bDisableCaching = false;
				uFilterHash = pAttr->m_pExpr->GetHash ( tSorterSchema, uFilterHash, bDisableCaching );
				if ( bDisableCaching )
					return false;
			} else
				uFilterHash = sphCalcLocatorHash ( pAttr->m_tLocator, uFilterHash );
		}

		dFilters.Add ( uFilterHash );
	}
	ARRAY_FOREACH ( i, q.m_dFilterTree )
	{
		dFilters.Add ( q.m_dFilterTree[i].GetHash() );
	}

	dFilters.Sort();
	return true;
}


void Qcache_c::Add ( const CSphQuery & q, QcacheEntry_c * pResult, const ISphSchema & tSorterSchema )
{
	pResult->Finish();

	// do not cache too fast queries or too big rsets, for obvious reasons
	// do not cache full scans, because we'll get an incorrect empty result set here
	if ( pResult->m_iElapsedMsec < m_iThreshMs || pResult->GetSize() > m_iMaxBytes )
		return;

	if ( !CanCacheQuery(q) )
		return;

	if ( !CalcFilterHashes ( pResult->m_dFilters, q, tSorterSchema ) )
		return;	// this query can't be cached because of the nature of expressions in filters

	pResult->AddRef();
	pResult->m_Key = GetKey ( pResult->m_iIndexId, q );

	ScopedMutex_t dLock (m_tLock);

	// rehash if needed
	if ( m_iCachedQueries>=m_iMaxQueries )
	{
		CSphVector<QcacheEntry_c*> hNew ( 2*m_hData.GetLength() );
		hNew.Fill ( QCACHE_NO_ENTRY );

		CSphVector<int> dRemap ( m_hData.GetLength() );
		dRemap.Fill ( -1 );

		int iLenMask = hNew.GetLength() - 1;
		ARRAY_FOREACH ( i, m_hData )
			if ( IsValidEntry(i) )
		{
			int j = m_hData[i]->m_Key & iLenMask;
			while ( hNew[j]!=NULL )
				j = ( j+1 ) & iLenMask;
			hNew[j] = m_hData[i];
			dRemap[i] = j;
		}

		ARRAY_FOREACH ( i, m_hData )
			if ( IsValidEntry(i) )
		{
			QcacheEntry_c * p = hNew [ dRemap[i] ];
			if ( p->m_iMruNext>=0 )
				p->m_iMruNext = dRemap [ p->m_iMruNext ];
			if ( p->m_iMruPrev>=0 )
				p->m_iMruPrev = dRemap [ p->m_iMruPrev ];
		}

		m_hData.SwapData ( hNew );
		m_iMruHead = dRemap [ m_iMruHead ];

		m_iMaxQueries *= 2;
	}

	// add entry
	int iLenMask = m_hData.GetLength() - 1;
	int j = pResult->m_Key & iLenMask;
	while ( IsValidEntry(j) )
		j = ( j+1 ) & iLenMask;
	m_hData[j] = pResult;

	m_iCachedQueries++;
	m_iUsedBytes += pResult->GetSize();
	MruToHead(j);

	dLock.Unlock();
	EnforceLimits ( true );
}

QcacheEntry_c * Qcache_c::Find ( int64_t iIndexId, const CSphQuery & q, const ISphSchema & tSorterSchema )
{
	if ( m_iMaxBytes<=0 )
		return nullptr;

	if ( !CanCacheQuery(q) )
		return nullptr;

	uint64_t k = GetKey ( iIndexId, q );

	bool bFilterHashesCalculated = false;
	CSphVector<uint64_t> dFilters;
	
	ScopedMutex_t dLock (m_tLock);

	int64_t tmMin = sphMicroTimer() - int64_t( m_iTtlS)*1000000;
	int iLenMask = m_hData.GetLength() - 1;
	int iLoop = m_hData.GetLength();
	int iRes = -1;
	for ( int i = k & iLenMask; m_hData[i]!=QCACHE_NO_ENTRY && iLoop--!=0; i = ( i+1 ) & iLenMask )
	{
		// check that entry is alive
		QcacheEntry_c * e = m_hData[i]; // shortcut
		if ( e==QCACHE_DEAD_ENTRY )
			continue;

		// check if we need to evict this one based on ttl
		if ( e->m_tmStarted < tmMin )
		{
			DeleteEntry(i);
			continue;
		}

		// check that key matches
		if ( e->m_Key!=k )
			continue;

		// check that filters are compatible (ie. that entry filters are a subset of query filters)
		if ( !bFilterHashesCalculated )
		{
			bFilterHashesCalculated = true;

			if ( !CalcFilterHashes ( dFilters, q, tSorterSchema ) )
				return nullptr;	// this query can't be cached because of the nature of expressions in filters
		}

		int j = 0;
		for ( ; j < e->m_dFilters.GetLength(); j++ )
			if ( !dFilters.BinarySearch ( e->m_dFilters[j] ) )
				break;

		// filters are good, return it
		if ( j==e->m_dFilters.GetLength() )
		{
			iRes = i;
			m_iHits++;
			break;
		}
	}

	QcacheEntry_c * p = nullptr;
	if ( iRes>=0 )
	{
		p = m_hData[iRes];
		p->AddRef();
		MruToHead(iRes);
	}

	return p;
}

uint64_t Qcache_c::GetKey ( int64_t iIndexId, const CSphQuery & q )
{
	// query cache key combines a bunch of data affecting things:
	// - index id
	// - MATCH() part
	// - ranker
	uint64_t k = sphFNV64 ( &iIndexId, sizeof(iIndexId) );
	k = sphFNV64cont ( q.m_sQuery.cstr(), k );
	k = sphFNV64 ( &q.m_eRanker, 1, k );
	if ( q.m_eRanker==SPH_RANK_EXPR )
		k = sphFNV64cont ( q.m_sRankerExpr.cstr(), k );
	if ( q.m_eRanker==SPH_RANK_PLUGIN )
	{
		k = sphFNV64cont ( q.m_sUDRanker.cstr(), k );
		k = sphFNV64cont ( q.m_sUDRankerOpts.cstr(), k );
	}
	return k;
}

void Qcache_c::DeleteEntry ( int i )
{
	assert ( IsValidEntry(i) );
	QcacheEntry_c * p = m_hData[i];

	// adjust MRU list
	if ( p->m_iMruNext>=0 )
		m_hData[p->m_iMruNext]->m_iMruPrev = p->m_iMruPrev;

	assert ( p->m_iMruPrev>=0 || m_iMruHead==i );
	if ( p->m_iMruPrev>=0 )
		m_hData[p->m_iMruPrev]->m_iMruNext = p->m_iMruNext;
	else
		m_iMruHead = p->m_iMruNext;

	// adjust stats
	m_iCachedQueries--;
	m_iUsedBytes -= p->GetSize();

	// release entry
	p->Release();
	m_hData[i] = QCACHE_DEAD_ENTRY;
}


bool Qcache_c::CanCacheQuery ( const CSphQuery & q ) const
{
	return q.m_eMode!=SPH_MATCH_FULLSCAN && !q.m_sQuery.IsEmpty();
}

void Qcache_c::EnforceLimits ( bool bSizeOnly )
{
	if ( bSizeOnly && m_iUsedBytes<=m_iMaxBytes )
		return;

	ScopedMutex_t dLock ( m_tLock );

	// first, enforce size limits
	int iCur = m_iMruHead;
	int64_t iBytes = 0;
	while ( iCur>=0 && m_iUsedBytes>m_iMaxBytes )
	{
		assert ( IsValidEntry(iCur) );
		int iNext = m_hData[iCur]->m_iMruNext;
		if ( iBytes + m_hData[iCur]->GetSize() > m_iMaxBytes )
			DeleteEntry ( iCur );
		else
			iBytes += m_hData[iCur]->GetSize();
		iCur = iNext;
	}

	if ( bSizeOnly )
		return;

	// if requested, do a full sweep, and recheck ttl and thresh limits
	int64_t tmMin = sphMicroTimer() - int64_t( m_iTtlS)*1000000;
	ARRAY_FOREACH ( i, m_hData )
		if ( IsValidEntry(i) && ( m_hData[i]->m_tmStarted < tmMin || m_hData[i]->m_iElapsedMsec < m_iThreshMs ) )
			DeleteEntry(i);

}

void Qcache_c::DeleteIndex ( int64_t iIndexId )
{
	ScopedMutex_t dLock ( m_tLock );
	ARRAY_FOREACH ( i, m_hData )
		if ( IsValidEntry(i) && m_hData[i]->m_iIndexId==iIndexId )
			DeleteEntry(i);
}

//////////////////////////////////////////////////////////////////////////

QcacheRanker_c::QcacheRanker_c ( QcacheEntry_c * pEntry, const ISphQwordSetup & tSetup )
	: m_pEntry ( pEntry )
{
	SafeAddRef ( pEntry );
	ResetImpl ( tSetup );
}

void QcacheRanker_c::ResetImpl ( const ISphQwordSetup & tSetup )
{
	m_pCur = m_pEntry->m_dData.Begin();
	m_pMax = m_pCur + m_pEntry->m_dData.GetLength();
	m_uLastId = INVALID_ROWID;
	m_pIndex = tSetup.m_pIndex;
	m_pCtx = tSetup.m_pCtx;

	for ( auto & m_dMatche : m_dMatches )
		m_dMatche.Reset ( tSetup.m_iDynamicRowitems );
}

int QcacheRanker_c::GetMatches()
{
	int iRes = 0;
	while ( !iRes )
	{
		// end of buffer? bail
		BYTE * p = m_pCur;
		if ( p>=m_pMax )
			return 0;

		// handle delta restart
		if ( *p==0x40 )
		{
			m_uLastId = INVALID_ROWID;
			m_pCur++;
			continue;
		}

		// decode next frame header
		bool bIndexedWeights = ( *p & 32 )!=0;
		int iWeightBytes = 1 + ( ( *p>>3 ) & 3 );
		int iDeltaBytes = 1 + ( *p & 7 );
		int iMatches = QcacheEntry_c::MAX_FRAME_SIZE;
		if ( *p & 128 )
		{
			iMatches = p[1];
			bIndexedWeights = false;
			p++;
		}
		p++;

		int iFrameBytes = iMatches * ( iDeltaBytes + iWeightBytes );
		if ( p+iFrameBytes > m_pMax )
		{
			// unexpected frame end? bail
			// FIXME? return an error somehow?
			m_pCur = m_pMax;
			return 0;
		}

		// decode frame data
		for ( int i=0; i<iMatches; i++ )
		{
			RowID_t uDelta = 0;
			memcpy ( &uDelta, p, iDeltaBytes );
			p += iDeltaBytes;
			m_uLastId += uDelta + 1;

			int iWeight = 0;
			memcpy ( &iWeight, p, iWeightBytes );
			p += iWeightBytes;
			if ( bIndexedWeights )
				iWeight = m_pEntry->m_dWeights [ iWeight ];

			CSphMatch & m = m_dMatches[iRes];
			m.m_tRowID = m_uLastId;
			m.m_iWeight = iWeight;

			// re-filter the cached match with new filters
			if ( !m_pIndex->EarlyReject ( m_pCtx, m ) )
				iRes++;
		}

		assert ( p==m_pCur + iFrameBytes + 1 + ( iMatches!=QcacheEntry_c::MAX_FRAME_SIZE ) );
		m_pCur = p;
	}
	return iRes;
}

//////////////////////////////////////////////////////////////////////////

void QcacheAdd ( const CSphQuery & q, QcacheEntry_c * pResult, const ISphSchema & tSorterSchema )
{
	return g_Qcache.Add ( q, pResult, tSorterSchema );
}

QcacheEntry_c * QcacheFind ( int64_t iIndexId, const CSphQuery & q, const ISphSchema & tSorterSchema )
{
	return g_Qcache.Find ( iIndexId, q, tSorterSchema );
}

std::unique_ptr<ISphRanker> QcacheRanker ( QcacheEntry_c * pEntry, const ISphQwordSetup & tSetup )
{
	return std::make_unique<QcacheRanker_c> ( pEntry, tSetup );
}

const QcacheStatus_t & QcacheGetStatus()
{
	return g_Qcache;
}

void QcacheSetup ( int64_t iMaxBytes, int iThreshMsec, int iTtlSec )
{
	g_Qcache.Setup ( iMaxBytes, iThreshMsec, iTtlSec );
}

void QcacheDeleteIndex ( int64_t iIndexId )
{
	g_Qcache.DeleteIndex ( iIndexId );
}
