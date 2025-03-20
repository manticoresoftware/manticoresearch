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

#ifndef _sphinxqcache_
#define _sphinxqcache_

#include "sphinxsearch.h"
#include "sphinxstd.h"

/// cached match is a simple {docid,weight} pair
struct QcacheMatch_t
{
	RowID_t		m_tRowID;
	DWORD		m_uWeight;
};

/// query cache entry
/// a compressed list of {docid,weight} pairs
class QcacheEntry_c : public ISphRefcountedMT
{
	friend class QcacheRanker_c;
	~QcacheEntry_c() override {}

public:
	int64_t						m_iIndexId = -1;
	int64_t						m_tmStarted { sphMicroTimer() };
	int							m_iElapsedMsec = 0;
	CSphVector<uint64_t>		m_dFilters;			///< hashes of the filters that were applied to cached query
	uint64_t					m_Key = 0;
	int							m_iMruPrev = -1;
	int							m_iMruNext = -1;

private:
	static const int			MAX_FRAME_SIZE = 32;

	// commonly used members
	int							m_iTotalMatches = 0;///< total matches
	CSphTightVector<BYTE>		m_dData;			///< compressed frames
	CSphTightVector<int>		m_dWeights;			///< weights table

	// entry build-time only members
	CSphVector<QcacheMatch_t>	m_dFrame;			///< current compression frame
	OpenHashTable_T<int, int>	m_hWeights;			///< hashed weights
	RowID_t						m_tLastRowID = INVALID_ROWID;	///< last encoded rowid

public:

	void						Append ( RowID_t uRowid, DWORD uWeight );
	void						Finish();
	int64_t						GetSize() const { return sizeof(*this) + m_dFilters.AllocatedBytes() + m_dData.AllocatedBytes() + m_dWeights.AllocatedBytes(); }
	void						RankerReset();

private:
	void						FlushFrame();
};

using QcacheEntryRefPtr_t = CSphRefcountedPtr<QcacheEntry_c>;

/// query cache status
struct QcacheStatus_t
{
	// settings that can be changed
	int64_t		m_iMaxBytes;		///< max RAM bytes
	int			m_iThreshMs;		///< minimum wall time to cache, in msec
	int			m_iTtlS;			///< cached query TTL, in msec

	// report-only statistics
	int			m_iCachedQueries;	///< cached queries counts
	int64_t		m_iUsedBytes;		///< used RAM bytes
	int64_t		m_iHits;			///< cache hits
};


void					QcacheAdd ( const CSphQuery & q, QcacheEntry_c * pResult, const ISphSchema & tSorterSchema );
QcacheEntry_c *			QcacheFind ( int64_t iIndexId, const CSphQuery & q, const ISphSchema & tSorterSchema );
std::unique_ptr<ISphRanker>			QcacheRanker ( QcacheEntry_c * pEntry, const ISphQwordSetup & tSetup );
const QcacheStatus_t &	QcacheGetStatus();
void					QcacheSetup ( int64_t iMaxBytes, int iThreshMsec, int iTtlSec );
void					QcacheDeleteIndex ( int64_t iIndexId );

#endif // _sphinxqcache_
