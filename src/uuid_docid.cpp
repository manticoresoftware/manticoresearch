//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "uuid_docid.h"

#include "accumulator.h"
#include "coroutine.h"
#include "indexsettings.h"
#include "std/openhash.h"

static int GetUuidHexDigit ( char c )
{
	if ( c>='0' && c<='9' )
		return c-'0';

	assert ( c>='a' && c<='f' );
	return c-'a'+10;
}


UuidDocidKey_t sphGetUuidDocidKey ( Str_t tUuid )
{
	assert ( sphIsNormalizedUuidDocid ( tUuid ) );

	UuidDocidKey_t tKey;
	int iDigit = 0;
	for ( int i=0; i<tUuid.second; ++i )
	{
		if ( tUuid.first[i]=='-' )
			continue;

		uint64_t & uDst = iDigit<16 ? tKey.m_uHi : tKey.m_uLo;
		uDst = ( uDst << 4 ) | GetUuidHexDigit ( tUuid.first[i] );
		++iDigit;
	}

	assert ( iDigit==32 );
	return tKey;
}

struct UuidEntry_t
{
	DocID_t		m_tDocID = 0;
	uint32_t	m_uRefs = 0;
};


class UuidDocidRegistry_c final : public UuidDocidRegistry_i
{
public:
	~UuidDocidRegistry_c() override
	{
		assert ( !m_hUuidDocids.GetLength() );
	}

	void Acquire ( const UuidDocidKey_t & tKey ) final
	{
		assert ( Threads::IsInsideCoroutine() );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );

		UuidEntry_t * pEntry = m_hUuidDocids.Find ( tKey );
		if ( pEntry )
		{
			assert ( pEntry->m_uRefs && pEntry->m_uRefs<UINT32_MAX );
			++pEntry->m_uRefs;
			return;
		}

		Verify ( m_hUuidDocids.Add ( tKey, { 0, 1 } ) );
	}

	bool TryAcquire ( const UuidDocidKey_t & tKey ) final
	{
		assert ( Threads::IsInsideCoroutine() );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );
		if ( m_hUuidDocids.Find ( tKey ) )
			return false;

		Verify ( m_hUuidDocids.Add ( tKey, { 0, 1 } ) );
		return true;
	}

	DocID_t GetDocid ( const UuidDocidKey_t & tKey ) const final
	{
		assert ( Threads::IsInsideCoroutine() );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );
		const UuidEntry_t * pEntry = m_hUuidDocids.Find ( tKey );
		assert ( pEntry && pEntry->m_uRefs );
		return pEntry->m_tDocID;
	}

	DocID_t SetDocidIfEmpty ( const UuidDocidKey_t & tKey, DocID_t tDocID ) final
	{
		assert ( Threads::IsInsideCoroutine() );
		assert ( tDocID );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );
		UuidEntry_t * pEntry = m_hUuidDocids.Find ( tKey );
		assert ( pEntry && pEntry->m_uRefs );
		if ( !pEntry->m_tDocID )
			pEntry->m_tDocID = tDocID;

		return pEntry->m_tDocID;
	}

	void ReleaseKey ( const UuidDocidKey_t & tKey ) final
	{
		assert ( Threads::IsInsideCoroutine() );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );
		UuidEntry_t * pEntry = m_hUuidDocids.Find ( tKey );
		assert ( pEntry && pEntry->m_uRefs );
		if ( pEntry->m_uRefs==1 )
			m_hUuidDocids.FindAndDelete ( tKey );
		else
			--pEntry->m_uRefs;
	}

	int64_t AllocatedBytes() const final
	{
		assert ( Threads::IsInsideCoroutine() );
		Threads::Coro::ScopedMutex_t tLock ( m_tLock );
		return m_hUuidDocids.GetLengthBytes();
	}

private:
	mutable Threads::Coro::Mutex_c m_tLock;
	OpenHashTable_T<UuidDocidKey_t, UuidEntry_t, UuidDocidKeyHash_fn> m_hUuidDocids { 0 };
};

UuidDocidRegistryPtr_t CreateUuidDocidRegistry()
{
	return UuidDocidRegistryPtr_t { new UuidDocidRegistry_c };
}


UuidDocidLease_c::UuidDocidLease_c ( UuidDocidLease_c && rhs ) noexcept
{
	Swap ( rhs );
}


UuidDocidLease_c & UuidDocidLease_c::operator = ( UuidDocidLease_c && rhs ) noexcept
{
	if ( this!=&rhs )
	{
		Reset();
		Swap ( rhs );
	}
	return *this;
}


UuidDocidLease_c::~UuidDocidLease_c()
{
	Reset();
}


void UuidDocidLease_c::Acquire ( UuidDocidRegistry_i * pRegistry, const UuidDocidKey_t & tKey )
{
	assert ( IsEmpty() );
	assert ( pRegistry );
	m_pRegistry = pRegistry;
	m_tKey = tKey;
	m_pRegistry->Acquire ( tKey );
	m_bOwned = true;
}


bool UuidDocidLease_c::TryAcquire ( UuidDocidRegistry_i * pRegistry, const UuidDocidKey_t & tKey )
{
	assert ( IsEmpty() );
	assert ( pRegistry );
	m_pRegistry = pRegistry;
	m_tKey = tKey;
	if ( !m_pRegistry->TryAcquire ( tKey ) )
	{
		m_pRegistry = nullptr;
		return false;
	}

	m_bOwned = true;
	return true;
}


void UuidDocidLease_c::Adopt ( RtAccum_t & tAccum )
{
	if ( IsEmpty() )
		return;

	tAccum.AdoptUuidLease ( m_pRegistry, m_tKey );
	m_bOwned = false;
	m_pRegistry = nullptr;
}


bool UuidDocidLease_c::IsEmpty() const
{
	assert ( m_bOwned==(bool)m_pRegistry );
	return !m_bOwned;
}


void UuidDocidLease_c::Reset()
{
	if ( m_bOwned )
		m_pRegistry->ReleaseKey ( m_tKey );

	m_bOwned = false;
	m_pRegistry = nullptr;
}


void UuidDocidLease_c::Swap ( UuidDocidLease_c & rhs ) noexcept
{
	::Swap ( m_pRegistry, rhs.m_pRegistry );
	::Swap ( m_tKey, rhs.m_tKey );
	::Swap ( m_bOwned, rhs.m_bOwned );
}
