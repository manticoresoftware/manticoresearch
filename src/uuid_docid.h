//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxdefs.h"
#include "std/fnv64.h"
#include "std/refcounted_mt.h"

struct UuidDocidKey_t
{
	uint64_t m_uHi = 0;
	uint64_t m_uLo = 0;

	UuidDocidKey_t() = default;
	explicit UuidDocidKey_t ( int ) {}
};


struct UuidDocidKeyHash_fn
{
	static FORCE_INLINE DWORD GetHash ( const UuidDocidKey_t & tKey )
	{
		return (DWORD)sphFNV64 ( &tKey, sizeof(tKey) );
	}

	static FORCE_INLINE bool Equal ( const UuidDocidKey_t & a, const UuidDocidKey_t & b )
	{
		return a.m_uHi==b.m_uHi && a.m_uLo==b.m_uLo;
	}
};

UuidDocidKey_t sphGetUuidDocidKey ( Str_t tUuid );


class UuidDocidRegistry_i : public ISphRefcountedMT
{
public:
	virtual void		Acquire ( const UuidDocidKey_t & tKey ) = 0;
	virtual bool		TryAcquire ( const UuidDocidKey_t & tKey ) = 0;
	virtual DocID_t	GetDocid ( const UuidDocidKey_t & tKey ) const = 0;
	virtual DocID_t	SetDocidIfEmpty ( const UuidDocidKey_t & tKey, DocID_t tDocID ) = 0;
	virtual void		ReleaseKey ( const UuidDocidKey_t & tKey ) = 0;
	virtual int64_t	AllocatedBytes() const = 0;

protected:
	~UuidDocidRegistry_i() override = default;
};

using UuidDocidRegistryPtr_t = CSphRefcountedPtr<UuidDocidRegistry_i>;

UuidDocidRegistryPtr_t CreateUuidDocidRegistry();

class RtAccum_t;


class UuidDocidLease_c
{
public:
	UuidDocidLease_c() = default;
	NONCOPYABLE ( UuidDocidLease_c );

	UuidDocidLease_c ( UuidDocidLease_c && rhs ) noexcept;
	UuidDocidLease_c & operator = ( UuidDocidLease_c && rhs ) noexcept;
	~UuidDocidLease_c();

	void Acquire ( UuidDocidRegistry_i * pRegistry, const UuidDocidKey_t & tKey );
	bool TryAcquire ( UuidDocidRegistry_i * pRegistry, const UuidDocidKey_t & tKey );
	void Adopt ( RtAccum_t & tAccum );
	bool IsEmpty() const;

private:
	void Reset();
	void Swap ( UuidDocidLease_c & rhs ) noexcept;

	// The bound accumulator owns the registry for the entire lease lifetime.
	UuidDocidRegistry_i *	m_pRegistry = nullptr;
	UuidDocidKey_t			m_tKey;
	bool					m_bOwned = false;
};
