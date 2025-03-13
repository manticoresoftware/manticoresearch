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

#include "skip_cache.h"

#include "std/ints.h"
#include "std/crc32.h"
#include "std/lrucache.h"
#include "sphinxsearch.h"


bool operator== ( const SkipCacheKey_t& lhs, const SkipCacheKey_t& rhs ) noexcept
{
	return lhs.m_iIndexId == rhs.m_iIndexId && lhs.m_tWordId == rhs.m_tWordId;
}

struct SkipCacheUtil_t
{
	static DWORD GetHash ( SkipCacheKey_t tKey )
	{
		DWORD uCRC32 = sphCRC32 ( &tKey.m_iIndexId, sizeof ( tKey.m_iIndexId ) );
		return sphCRC32 ( &tKey.m_tWordId, sizeof ( tKey.m_tWordId ), uCRC32 );
	}

	static DWORD GetSize ( SkipData_t* pValue ) { return pValue ? pValue->m_dSkiplist.GetLengthBytes() : 0; }
	static void Reset ( SkipData_t*& pValue ) { SafeDelete ( pValue ); }
};


class SkipCache_c: public LRUCache_T<SkipCacheKey_t, SkipData_t*, SkipCacheUtil_t>
{
	using BASE = LRUCache_T<SkipCacheKey_t, SkipData_t*, SkipCacheUtil_t>;
	using BASE::BASE;

public:
	void DeleteAll ( int64_t iIndexId )
	{
		BASE::Delete ( [iIndexId] ( const SkipCacheKey_t& tKey ) { return tKey.m_iIndexId == iIndexId; } );
	}

	static void Init ( int64_t iCacheSize );
	static void Done() { SafeDelete ( m_pSkipCache ); }
	static SkipCache_c* Get() { return m_pSkipCache; }

private:
	static SkipCache_c* m_pSkipCache;
};

SkipCache_c* SkipCache_c::m_pSkipCache = nullptr;


void SkipCache_c::Init ( int64_t iCacheSize )
{
	assert ( !m_pSkipCache );
	if ( iCacheSize > 0 )
		m_pSkipCache = new SkipCache_c ( iCacheSize );
}


void InitSkipCache ( int64_t iCacheSize )
{
	SkipCache_c::Init ( iCacheSize );
}


void ShutdownSkipCache()
{
	SkipCache_c::Done();
}

void SkipCache::DeleteAll ( int64_t iIndexId )
{
	SkipCache_c* pSkipCache = SkipCache_c::Get();
	if ( pSkipCache )
		pSkipCache->DeleteAll ( iIndexId );
}

void SkipCache::Release ( SkipCacheKey_t tKey )
{
	SkipCache_c* pSkipCache = SkipCache_c::Get();
	if ( pSkipCache )
		pSkipCache->Release ( std::move ( tKey ) );
}

bool SkipCache::Find ( SkipCacheKey_t tKey, SkipData_t * & pData )
{
	SkipCache_c* pSkipCache = SkipCache_c::Get();
	if ( pSkipCache )
		return pSkipCache->Find ( std::move ( tKey ), pData );
	return false;
}

bool SkipCache::Add ( SkipCacheKey_t tKey, SkipData_t* pData )
{
	SkipCache_c* pSkipCache = SkipCache_c::Get();
	if ( pSkipCache )
		return pSkipCache->Add ( std::move ( tKey ), pData );
	return false;
}
