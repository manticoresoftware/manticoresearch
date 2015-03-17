//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxqcache_
#define _sphinxqcache_

#include "sphinx.h"
#include "sphinxsearch.h"

/// cached match is a simple {docid,weight} pair
struct QcacheMatch_t
{
	SphDocID_t		m_uDocid;
	int				m_iWeight;
};

/// query cache entry
/// a compressed list of {docid,weight} pairs
class QcacheEntry_c : public ISphRefcountedMT
{
	friend class QcacheRanker_c;

public:
	int64_t						m_iIndexId;
	int64_t						m_tmStarted;
	int							m_iElapsedMsec;
	CSphVector<uint64_t>		m_dFilters;			///< hashes of the filters that were applied to cached query
	uint64_t					m_Key;
	int							m_iMruPrev;
	int							m_iMruNext;

private:
	static const int			MAX_FRAME_SIZE = 32;

	// commonly used members
	int							m_iTotalMatches;	///< total matches
	CSphTightVector<BYTE>		m_dData;			///< compressed frames
	CSphTightVector<int>		m_dWeights;			///< weights table

	// entry build-time only members
	CSphVector<QcacheMatch_t>	m_dFrame;			///< current compression frame
	CSphHash<int>				m_hWeights;			///< hashed weights
	SphDocID_t					m_uLastDocid;		///< last encoded docid

public:
	QcacheEntry_c();

	void						Append ( SphDocID_t uDocid, int iWeight );
	void						Finish();
	int							GetSize() const { return sizeof(*this) + m_dFilters.GetSizeBytes() + m_dData.GetSizeBytes() + m_dWeights.GetSizeBytes(); }
	void						RankerReset();

private:
	void						FlushFrame();
};

/// query cache status
struct QcacheStatus_t
{
	// settings that can be changed
	int64_t		m_iMaxBytes;		///< max RAM bytes
	int			m_iThreshMsec;		///< minimum wall time to cache, in msec
	int			m_iTtlSec;			///< cached query TTL, in msec

	// report-only statistics
	int			m_iCachedQueries;	///< cached queries counts
	int64_t		m_iUsedBytes;		///< used RAM bytes
	int64_t		m_iHits;			///< cache hits
};


void					QcacheAdd ( const CSphQuery & q, QcacheEntry_c * pResult );
QcacheEntry_c *			QcacheFind ( int64_t iIndexId, const CSphQuery & q );
ISphRanker *			QcacheRanker ( QcacheEntry_c * pEntry, const ISphQwordSetup & tSetup );
const QcacheStatus_t &	QcacheGetStatus();
void					QcacheSetup ( int64_t iMaxBytes, int iThreshMsec, int iTtlSec );
void					QcacheDeleteIndex ( int64_t iIndexId );

#endif // _sphinxqcache_

//
// $Id$
//
