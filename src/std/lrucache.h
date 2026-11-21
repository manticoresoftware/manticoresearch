//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#ifndef _lrucache_
#define _lrucache_

#include "ints.h"
#include "mutex.h"
#include "openhash.h"
#include <atomic>

template <typename KEY, typename VALUE, typename HELPER>
class LRUCache_T
{
public:
	using Ref_t = std::atomic<int> *;

			LRUCache_T ( int64_t iCacheSize, int64_t iMaxBlockSize = -1, int64_t iAdaptiveFloor = 0 );
			~LRUCache_T();

	bool	Find ( KEY tKey, VALUE & tData, Ref_t * ppRef = nullptr );
	bool	Add ( KEY tKey, const VALUE & tData, Ref_t * ppRef = nullptr );
	void	Release ( KEY tKey );
	// A pin returned by Find/Add must be released exactly once after using the value.
	static void ReleaseRef ( Ref_t pRef )
	{
		[[maybe_unused]] const int iPrevious = pRef->fetch_sub ( 1, std::memory_order_release );
		assert ( iPrevious>0 );
	}
	int64_t GetUsedBytes();

	template <typename COND>
	void	Delete ( COND && fnCond );

protected:
	struct LinkedEntry_t
	{
		VALUE			m_tValue;
		DWORD			m_uSize = 0;
		std::atomic<int> m_iRefcount { 0 };
		uint32_t		m_uAccessEpoch = 0;
		LinkedEntry_t *	m_pPrev = nullptr;
		LinkedEntry_t *	m_pNext = nullptr;
		KEY				m_tKey;
	};

	// Packed generic entries can return misaligned key/value references.
	struct HashEntry_t
	{
		KEY m_Key;
		LinkedEntry_t * m_Value = nullptr;

		template <typename STATE> FORCE_INLINE bool IsUsed ( const STATE & ) const { return m_bUsed; }
		template <typename STATE> FORCE_INLINE void SetUsed ( const STATE &, bool bUsed ) { m_bUsed = bUsed; }

	private:
		bool m_bUsed = false;
	};

	LinkedEntry_t *		m_pHead = nullptr;
	LinkedEntry_t *		m_pTail = nullptr;
	int64_t				m_iCacheSize = 0;
	int64_t				m_iMemUsed = 0;
	DWORD			m_uMaxBlockSize = 0;
	CSphMutex			m_tLock;
	OpenHashTable_T<KEY, LinkedEntry_t *, HELPER, HashEntry_t> m_tHash;

	void	Delete ( LinkedEntry_t * pEntry );

private:
	// Access epochs advance under the existing shard lock, without a global counter.
	int64_t m_iAdaptiveFloor = 0;
	int64_t m_iAdaptiveTarget = 0;
	int64_t m_iActiveBytes = 0;
	uint32_t m_uAccessEpoch = 1;
	int m_iWindowAccesses = 0;
	int m_iWindowHits = 0;
	int m_iStableWindows = 0;

	void	Observe ( LinkedEntry_t * pEntry );
	void	Touch ( LinkedEntry_t * pEntry );
	void	TrimCold();
	void	MoveToHead ( LinkedEntry_t * pEntry );
	void	Add ( LinkedEntry_t * pEntry );
	void	SweepUnused ( DWORD uSpaceNeeded );
	bool	HaveSpaceFor ( DWORD uSpaceNeeded ) const;
};

#include "lrucache_impl.h"

#endif // _lrucache_
