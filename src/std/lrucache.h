//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

template <typename KEY, typename VALUE, typename HELPER>
class LRUCache_T
{
public:
			LRUCache_T ( int64_t iCacheSize );
			~LRUCache_T();

	bool	Find ( KEY tKey, VALUE & tData );
	bool	Add ( KEY tKey, const VALUE & tData );
	void	Release ( KEY tKey );

	template <typename COND>
	void	Delete ( COND && fnCond );

protected:
	struct LinkedEntry_t
	{
		VALUE			m_tValue;
		DWORD			m_uSize = 0;
		int				m_iRefcount = 0;
		LinkedEntry_t *	m_pPrev = nullptr;
		LinkedEntry_t *	m_pNext = nullptr;
		KEY				m_tKey;
	};

	LinkedEntry_t *		m_pHead = nullptr;
	LinkedEntry_t *		m_pTail = nullptr;
	int64_t				m_iCacheSize = 0;
	int64_t				m_iMemUsed = 0;
	CSphMutex			m_tLock;
	OpenHashTable_T<KEY, LinkedEntry_t *, HELPER> m_tHash;

	void	Delete ( LinkedEntry_t * pEntry );

private:
	void	MoveToHead ( LinkedEntry_t * pEntry );
	void	Add ( LinkedEntry_t * pEntry );
	void	SweepUnused ( DWORD uSpaceNeeded );
	bool	HaveSpaceFor ( DWORD uSpaceNeeded ) const;
};

#include "lrucache_impl.h"

#endif // _lrucache_
