//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#pragma once
#include "sphinxstd.h"
#include <atomic>


// Hazard pointers implementation
/*
 * Main idea: as long as somebody uses an object, we should not delete it.
 * To achieve it, two things in use:
 * 1. When we have to delete an object, we call hazard::Retire, providing pointer to the obj,
 *    or raw (void*) ptr and deleter function which has to be called with that ptr for cleaning.
 * 2. When access obj from the other thread by pointer, we access it via Guard_c::Protect.
 *    It ensures, that pointer is saved in thread-local set of hazard pointers.
 *
 * At the moment when thread-local list of retired object full, we collect all hazard pointers from all threads
 * and then filter our list of retired: if an object is not pointed by anyone, it is finally removed.
 *
 * Since hazard pointer protection intended for local variables only, just few hazard slots provided for each object
 * (usually 1 or 2 is enough).
 *
 * Since hazard pointer is thread-local, avoid switch context having some variable protected this way,
 * since yielding in one thread and awaking in another will lead to race of hazard pointer in original thread,
 * and since it is hot place, we want to keep it simpler (avoid any mt-related problem).
 *
 * RAII wrapper hazard::Ptr_T
 *
 * Comparing to ref-counting, hazard pointers are faster, since uses just light atomic store for saving pointers,
 * and avoids heavy compare-and-swap as used in atomic counters.
 *
 */

namespace hazard {

// 2 pointers per thread should be enough for the beginning
static const int POINTERS_PER_THREAD = 2;

// backend pointer type
using Pointer_t = void*;

// ref to fn which actualy deletes the object of given type
using Deleter_fn = void ( * ) ( void * );

// Free the object. It will be deleted now or later by gc
void Retire ( Pointer_t pData, Deleter_fn fnDelete, bool bNow=false );

template<typename PTR>
void Retire ( PTR pData, bool bNow=false )
{
	Retire ( (Pointer_t) pData, Deleter_T<PTR, ETYPE::SINGLE>::Delete, bNow );
}

template<typename PTR>
void RetireArray ( PTR pData, bool bNow=false )
{
	Retire ( (Pointer_t) pData, Deleter_T<PTR, ETYPE::ARRAY>::Delete, bNow );
}

// collect indexes of collection which is still used
using Accessor_fn = std::function<void*(int)>;
CSphVector<int> GetListOfPointed ( Accessor_fn&& fnAccess, int iCount );

template<typename T>
CSphVector<int> GetListOfPointed ( const VecTraits_T<T>& dCollection )
{
	return GetListOfPointed ( [&] ( int iIdx ) { return &dCollection[iIdx]; }, dCollection.GetLength () );
}

// RAII hazard pointer.
// on d-tr pointee (if any) will be postponed for deletion
template<typename PTR, typename DELETER>
class ScopedPtr_T : ISphNoncopyable
{
	using ATOMIC_PTR = std::atomic<PTR>;
	ATOMIC_PTR m_pPtr { nullptr };

public:
	///< default ctr (for vectors)
	explicit ScopedPtr_T () = default;

	/// construction from raw pointer
	explicit ScopedPtr_T ( PTR pPtr )
	{
		m_pPtr.store ( pPtr, std::memory_order_release );
	}

	ScopedPtr_T ( ScopedPtr_T && rhs ) noexcept
	{
		Swap ( rhs );
	}

	~ScopedPtr_T ()
	{
		Retire ( (void*)m_pPtr.load ( std::memory_order_relaxed ), DELETER::Delete );
	}

	void Swap ( ScopedPtr_T & rhs ) noexcept
	{
		using namespace std; // just to make following line shorter
		m_pPtr.store ( rhs.m_pPtr.exchange ( m_pPtr.load(memory_order_acquire), memory_order_acq_rel ), memory_order_release );
	}

	PTR operator-> () const noexcept
	{
		return m_pPtr.load ( std::memory_order_relaxed );
	}

	explicit operator bool () const noexcept
	{
		return m_pPtr.load ( std::memory_order_relaxed )!=nullptr;
	}

	explicit operator PTR () const noexcept
	{
		return m_pPtr.load ( std::memory_order_relaxed );
	}

	explicit operator ATOMIC_PTR& () noexcept
	{
		return m_pPtr;
	}

	// special case: assign new value and try to prune previous immediately
	void RetireNow ( PTR pPtr=nullptr )
	{
		Retire ( (Pointer_t) m_pPtr.exchange ( pPtr, std::memory_order_acq_rel ), DELETER::Delete, true );
	}

	/// assignment of a raw pointer, retires previous value
	ScopedPtr_T & operator= ( PTR pPtr )
	{
		Retire ( (Pointer_t) m_pPtr.exchange ( pPtr, std::memory_order_acq_rel ), DELETER::Delete );
		return *this;
	}
};

template<typename T>
using ScopedPtr_t = ScopedPtr_T<T, Deleter_T<T, ETYPE::SINGLE>>;

template<typename T>
using ScopedArr_t = ScopedPtr_T<T, Deleter_T<T, ETYPE::ARRAY>>;

template<typename PTR>
hazard::ScopedPtr_t <PTR> MakeScopedPtr ( PTR pPtr )
{
	return hazard::ScopedPtr_t<PTR> ( pPtr );
}

// hazard pointer store
struct AtomicPointer_t
{
	std::atomic<Pointer_t> m_pData;
	void Set ( const void * pData )
	{
		m_pData.store ( (Pointer_t)const_cast<void*>(pData), std::memory_order_release );
	}
};

// allocates hazard slot and then may protect any variable.
// while it is protected, it can't be deleted in any other thread.
class Guard_c : ISphNoncopyable
{
	AtomicPointer_t* m_pGuard;
public:
	Guard_c();
	~Guard_c();

	template <typename PTR>
	PTR Protect ( const std::atomic<PTR>& pMtVal )
	{
		assert ( m_pGuard );

		// we need to be sure that assigned value is exactly the one existing in pMtVal
		// so, if after assign we found that it is changed - repeat assigning again.
		PTR pAssignedPtr;
		PTR pCurrentPtr = pMtVal.load ( std::memory_order_relaxed );
		do {
			pAssignedPtr = pCurrentPtr;
			m_pGuard->Set ( pCurrentPtr );
			pCurrentPtr = pMtVal.load ( std::memory_order_acquire );
		} while ( pAssignedPtr!=pCurrentPtr );
		return pCurrentPtr;
	}

	template<typename PTR, typename DELETER>
	PTR Protect ( const ScopedPtr_T<PTR,DELETER> & pMtVal )
	{
		return Protect ( (const std::atomic<PTR>&) pMtVal );
	}

	// simple unset current guard - to reuse the slot without reallocating.
	void Release ()
	{
		assert ( m_pGuard );
		m_pGuard->Set ( nullptr );
	}
};

// helper, to be called from shutdown. Try to finally delete all weak objects.
// returns num of still alive. If it is non-zero - it means, some threads still alive,
// and points to the objects in their hazard pointers.
void Shutdown ();

} // namespace hazard
