//
// Copyright (c) 2020, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "hazard_pointer.h"
#include "threadutils.h"

static const int MIN_POINTERS = 100;
static const int MULTIPLIER = 2;

using namespace hazard;

namespace {
inline int GetCleanupSize ()
{
	int uSize = Threads::GetNumOfRunning () * POINTERS_PER_THREAD * MULTIPLIER;
	return Max ( uSize, MIN_POINTERS );
}
}

struct ListedPointer_t : public AtomicPointer_t
{
	ListedPointer_t* m_pNext;
};

using VecListedPointers_t = CSphFixedVector<ListedPointer_t>;

// raw pointer and specific deleter which knows how to deal with the pointer
using RetiredPointer_t = std::pair <void*,Deleter_fn>;

// to be write-used in single thread, so no need to sync anything
// that is lowest level which actually stores current pointers.
struct Storage_t : public VecListedPointers_t
{
	ListedPointer_t* m_pHead;
	explicit Storage_t ( int iSize )
		: VecListedPointers_t ( iSize )
	{
		// link as list
		for (auto i=0; i<m_iCount; ++i )
		{
			m_pData[i].m_pNext = m_pData+i+1;
			m_pData[i].m_pData.store ( nullptr, std::memory_order_relaxed );
		}
		Last().m_pNext = nullptr;
		m_pHead = m_pData;
	}

	bool IsFull() const
	{
		return !m_pHead;
	}

	ListedPointer_t* Alloc ()
	{
		assert (!IsFull());

		auto pElem = m_pHead;
		m_pHead = pElem->m_pNext;
		return pElem;
	}

	void Dealloc ( ListedPointer_t* pElem ) noexcept
	{
		if (!pElem)
			return;

		pElem->m_pData.store ( nullptr, std::memory_order_release );
		pElem->m_pNext = m_pHead;
		m_pHead = pElem;
	}
};

using VecRetiredPointers_t = CSphVector<RetiredPointer_t>;

// that is single-threaded in use, so no lock-free loops necessary.
struct VecOfRetired_t : public VecRetiredPointers_t, public ISphNoncopyable
{
	std::atomic<int> m_iCurrent { 0 };

	inline bool IsFull() const noexcept
	{
		return m_iCurrent.load ( std::memory_order_relaxed )==GetLength ();
	}

	// retire element, and return false, if storage is full
	bool Retire (RetiredPointer_t&& tData) noexcept
	{
		auto iCurrent = m_iCurrent.load(std::memory_order_relaxed);
		At ( iCurrent ) = tData;
		m_iCurrent.store ( iCurrent+1, std::memory_order_relaxed );
		return ( iCurrent+1 )<GetLength ();
	}

	void Swap ( VecOfRetired_t& rhs)
	{
		// this long line is for atomic 'swap ( m_iCurrent, rhs.m_iCurrent )'
		m_iCurrent.store ( rhs.m_iCurrent.exchange ( m_iCurrent.load ( std::memory_order_acquire ), std::memory_order_acq_rel )
					   , std::memory_order_release );
		SwapData ( rhs );
	}
};

// main hazard pointer storage class (per-thread)
struct ThreadState_t : public ISphNoncopyable
{
	Storage_t	m_tHazards;
	VecOfRetired_t m_tRetired;

public:
	ThreadState_t ();
	~ThreadState_t ();

	// generic main GC procedure
	void Scan();

	// the part where only trimming (no resize adopt, no resize at the end).
	void PruneRetired ();

	void Retire ( RetiredPointer_t tPtr, bool bNow );
};

namespace { // unnamed (static)

// raw pointer and specific deleter which knows how to deal with the pointer
using RetiredPointer_t = std::pair <void*,Deleter_fn>;

// delete an object stored in RetiredPointer
void PrunePointer ( RetiredPointer_t* pPtr )
{
	assert (pPtr);
	assert (pPtr->second);
	pPtr->second ( pPtr->first );
}

// main GC procedure. Implements hazard pointers cleaning
/*
 * Sources:
	- [2002] Maged M.Michael "Safe memory reclamation for dynamic lock-freeobjects using atomic reads and writes"
	- [2003] Maged M.Michael "Hazard Pointers: Safe memory reclamation for lock-free objects"
	- [2004] Andrei Alexandrescy, Maged Michael "Lock-free Data Structures with Hazard Pointers"
	- Inspired with libcds by Maxim Khizhinsky (libcds.dev@gmail.com) - http://github.com/khizmax/libcds/
 */
CSphVector<Pointer_t> CollectActiveHazardPointers()
{
	// prepare vec for collect active hazard pointers
	CSphVector <Pointer_t> dActive;
	dActive.Reserve ( Threads::GetNumOfRunning () * POINTERS_PER_THREAD );

	// stage 1. Walk over ALL threads and collect their hazard pointers
	Threads::IterateActive ([&dActive] ( Threads::LowThreadDesc_t * pDesc )
	{
		if (!pDesc)
			return;
		auto pOtherThreadState = (ThreadState_t *) pDesc->m_pHazards.load ( std::memory_order_relaxed );
		if ( !pOtherThreadState )
			return;
		for ( const auto& tHazard : pOtherThreadState->m_tHazards )
		{
			// here 'acquire' semantic is complement to 'release' semantic on thread saving hazard ptr.
			auto pData = tHazard.m_pData.load ( std::memory_order_acquire );
			if ( pData )
				dActive.Add ( pData );
		}
	});

	// stage 2. sort and uniq; we will use binsearch then
	dActive.Uniq();
	return dActive;
}

// provided blob of retired pointers, returns size of compacted blob after gc.
int PruneRetired ( RetiredPointer_t * pData, size_t iSize )
{
	if ( !iSize )
		return 0;

	auto dActive = CollectActiveHazardPointers();

	// stage 3-4. Keep in m_tRetired only active elems.
	// Note that m_tRetired contains only unique elems, since it works as 'delete'
	// (having non-unique immediately means that kind of double-free bug is in application).
	auto * pDst = pData;
	auto * pEnd = pData + iSize;
	for ( auto * pSrc = pData; pSrc<pEnd; ++pSrc )
	{
		if ( !dActive.BinarySearch ( pSrc->first ) )
			PrunePointer (pSrc); // stage 3 - delete non-alive
		else { // stage 4 - copy alive
			if (pDst!=pSrc)
				*pDst=*pSrc;
			++pDst;
		}
	}

	return (int) (pDst - pData);
}

// that is shortcut for the special cases, like retiring huge blob of mem.
// We try to prune it immediately, and if not success, add it to usual list of retired
bool TryPruneOnePointer ( RetiredPointer_t* pSrc )
{
	auto dActive = CollectActiveHazardPointers ();
	if ( dActive.BinarySearch ( pSrc->first ) )
		return false;

	PrunePointer ( pSrc );
	return true;
}

// when thread hazard destructes, it may still have some alive retired
// we will leak them into this sink, and sometimes refine with another threads.
struct RetiredSink_t
{
	std::atomic<int> m_iSize {0};
	CSphMutex m_dGuard;
	VecRetiredPointers_t m_dSink GUARDED_BY (m_dGuard);

	bool IsEmpty()
	{
		return m_iSize.load ( std::memory_order_relaxed )==0;
	}

	// adopt retired chunk from finishing thread
	void AddSink ( const VecRetiredPointers_t& dRetired ) EXCLUDES ( m_dGuard )
	{
		RetireSink();

		if ( dRetired.IsEmpty() )
			return;

		ScopedMutex_t _ ( m_dGuard );
		m_dSink.Append ( dRetired );
		m_iSize.store ( m_dSink.GetLength (), std::memory_order_relaxed );
	}

	// prune sinked if necessary
	// returns num of still non-deleted elements
	int RetireSink ( bool bForce = false ) EXCLUDES ( m_dGuard )
	{
		if ( m_iSize.load ( std::memory_order_relaxed )<GetCleanupSize () )
			return 0;

		ScopedMutex_t _ ( m_dGuard );
		auto iCompressed = PruneRetired ( m_dSink.begin(), m_dSink.GetLength() );
		m_dSink.Resize ( iCompressed );
		m_iSize.store ( iCompressed, std::memory_order_relaxed );
		return iCompressed;
	}

	void RetireAll() NO_THREAD_SAFETY_ANALYSIS
	{
		VecRetiredPointers_t tOldRetired;
		{
			ScopedMutex_t _ ( m_dGuard );
			tOldRetired.SwapData ( m_dSink );
		}

		for ( auto & dPointer: tOldRetired )
			PrunePointer ( &dPointer );

		{
			ScopedMutex_t _ ( m_dGuard );
			tOldRetired.SwapData ( m_dSink );
		}
	}
};

RetiredSink_t& dGlobalSink()
{
	static RetiredSink_t g_dGlobalSink;
	return g_dGlobalSink;
}


} // unnamed namespace

ThreadState_t & ThreadState ()
{
	static thread_local ThreadState_t tState;
	return tState;
}

ThreadState_t::ThreadState_t ()
	: m_tHazards { POINTERS_PER_THREAD }
{
	m_tRetired.Resize ( GetCleanupSize () );
	Threads::MyThd ().m_pHazards.store ( this, std::memory_order_relaxed );
}


ThreadState_t::~ThreadState_t ()
{
	PruneRetired ();

	// here in m_tRetired m.b. left elements which are still in use by some other thread.
	m_tRetired.Resize ( m_tRetired.m_iCurrent );
	dGlobalSink ().AddSink ( m_tRetired );
}

// GC enterpoint - called when no more place for retired
void ThreadState_t::Scan ()
{
	// Before start, check if num of threads grown and so, we just need more space for retired
	if ( m_tRetired.GetLength() >= GetCleanupSize() )
		PruneRetired ();

	m_tRetired.Resize ( GetCleanupSize () );
}

// main GC worker
void ThreadState_t::PruneRetired ()
{
	// actually task is quite simple: we call ::PruneRetired, it keeps all alive pointers, prune all deads and return
	// the new size of our vec, containing only alive.
	// The problem is that deleting non-alive may include more to retire (if deleting object also has something to retire).
	// Since we're in call PruneRetired because we're full, we can't right now retire any more pointers.
	// So, we use this ping-pong with tmp vector to give room for nested retiring.
	VecOfRetired_t tOldRetired;
	tOldRetired.Resize ( GetCleanupSize () );

	m_tRetired.Swap ( tOldRetired );
	auto iRest = ::PruneRetired ( tOldRetired.begin (), tOldRetired.m_iCurrent.load ( std::memory_order_relaxed ) );
	m_tRetired.Swap ( tOldRetired );
	m_tRetired.m_iCurrent.store ( iRest, std::memory_order_relaxed );

	tOldRetired.Resize( tOldRetired.m_iCurrent.load ( std::memory_order_relaxed ) );
	for (auto dTailed : tOldRetired ) // some nested prune happened
		Retire ( dTailed, false );
}

void ThreadState_t::Retire ( RetiredPointer_t tPtr, bool bNow )
{
	if ( bNow && TryPruneOnePointer ( &tPtr ) )
		return;

	if ( m_tRetired.Retire( std::move (tPtr) ) )
		return;

	Scan();
	if ( m_tRetired.IsFull() )
		Scan();

	assert (!m_tRetired.IsFull());
}

CSphVector<int> hazard::GetListOfPointed ( Accessor_fn fnAccess, int iCount )
{
	CSphVector<int> dResult;
	auto dActive = CollectActiveHazardPointers ();

	for (int i=0; i<iCount;++i )
		if ( dActive.BinarySearch ( fnAccess(i) ) )
			dResult.Add(i);

	return dResult;
}

hazard::Guard_c::Guard_c()
{
	m_pGuard = ThreadState ().m_tHazards.Alloc ();
	assert ( m_pGuard && "couldn't alloc hazard pointer. Not enough slots?" );
}

hazard::Guard_c::~Guard_c()
{
	if ( m_pGuard )
		ThreadState ().m_tHazards.Dealloc ( (ListedPointer_t *) m_pGuard );
}

// main entry point to delete. On-the-fly filter out nullptr values.
void hazard::Retire ( Pointer_t pData, Deleter_fn fnDelete, bool bNow )
{
	if ( pData )
		ThreadState ().Retire ( { pData, fnDelete }, bNow );
}

void hazard::Shutdown ()
{
	auto & tState = ThreadState ();
	do {
		tState.m_tRetired.Resize ( tState.m_tRetired.m_iCurrent );
		dGlobalSink ().AddSink ( tState.m_tRetired );
		tState.m_tRetired.m_iCurrent = 0;
		dGlobalSink ().RetireAll ();
	} while ( tState.m_tRetired.m_iCurrent>0 );
	sphLogDebug ( "hazard::Shutdown() done");
}