//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "hazard_pointer.h"
#include "threadutils.h"

static const DWORD MIN_POINTERS = 100;
static const int MULTIPLIER = 2;

// if one hazard object owns another, etc. - how deep they can be nested.
// on finish we will stop unwinding under this N of steps and report error.
static const int NESTED_LEVELS = 16;

using namespace hazard;

namespace {
inline int GetCleanupSize ()
{
	DWORD uSize = Threads::GetNumOfRunning () * POINTERS_PER_THREAD * MULTIPLIER;
	return (int) Max ( uSize, MIN_POINTERS );
}
}

struct ListedPointer_t : public AtomicPointer_t
{
	ListedPointer_t* m_pNext;
};

using VecListedPointers_t = CSphFixedVector<ListedPointer_t>;

// raw pointer and specific deleter which knows how to deal with the pointer
using RetiredPointer_t = std::pair <void*,Deleter_fn>;

// for tracking that alloc/dealloc of hazard is in one thread
static ThreadRole thHazardThread;

using fnHazardProcessor=std::function<void ( Pointer_t )>;

// to be write-used in single thread, so no need to sync anything
// that is lowest level which actually stores current pointers.
struct Storage_t : public VecListedPointers_t
{
	ListedPointer_t* m_pHead GUARDED_BY ( thHazardThread );
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

	bool IsFull() const REQUIRES ( thHazardThread )
	{
		return !m_pHead;
	}

	ListedPointer_t* Alloc () ACQUIRE ( thHazardThread ) NO_THREAD_SAFETY_ANALYSIS
	{
		AcquireRole ( thHazardThread );
		assert (!IsFull());

		auto pElem = m_pHead;
		m_pHead = pElem->m_pNext;
		return pElem;
	}

	void Dealloc ( ListedPointer_t* pElem ) noexcept RELEASE ( thHazardThread ) NO_THREAD_SAFETY_ANALYSIS
	{
		if (!pElem)
			return;

		pElem->m_pData.store ( nullptr, std::memory_order_release );
		pElem->m_pNext = m_pHead;
		m_pHead = pElem;
		ReleaseRole ( thHazardThread );
	}
};

// to ensure whole retiring operation done in one thread
static ThreadRole thRetiringThread;

using VecRetiredPointers_t = CSphVector<RetiredPointer_t>;

// that is single-threaded in use, so no lock-free loops necessary.
struct VecOfRetired_t : public VecRetiredPointers_t, public ISphNoncopyable
{
	int m_iCurrent GUARDED_BY ( thRetiringThread ) = 0;

	inline bool IsFull () const noexcept REQUIRES ( thRetiringThread )
	{
		return m_iCurrent>=GetLength ();
	}

	inline bool NotEmpty () const noexcept REQUIRES ( thRetiringThread )
	{
		return m_iCurrent>0;
	}

	// retire element, and return false, if storage is full
	bool Retire ( RetiredPointer_t && tData ) noexcept REQUIRES ( thRetiringThread )
	{
		At ( m_iCurrent++ ) = tData;
		return m_iCurrent<GetLength ();
	}

	void Swap ( VecOfRetired_t& rhs) REQUIRES ( thRetiringThread )
	{
		// this long line is for atomic 'swap ( m_iCurrent, rhs.m_iCurrent )'
		::Swap ( m_iCurrent, rhs.m_iCurrent );
		SwapData ( rhs );
	}
};

// when thread hazard destructes, it may still have some alive retired
// we will leak them into this sink, and sometimes refine with another threads.
class RetiredSink_c
{
	CSphMutex m_dGuard;
	VecRetiredPointers_t m_dSink GUARDED_BY (m_dGuard);

	void PruneSink () REQUIRES ( m_dGuard );
	void MaybePruneSink () REQUIRES ( m_dGuard );

public:
	void AdoptRetired ( VecOfRetired_t& dRetired ) EXCLUDES ( m_dGuard );
	void FinallyPruneSink () EXCLUDES ( m_dGuard );
};

// main hazard pointer storage class (per-thread)
class ThreadState_c : public ISphNoncopyable
{
	Storage_t	m_tHazards;
	VecOfRetired_t m_tRetired  GUARDED_BY ( thRetiringThread );
	static RetiredSink_c m_dGlobalSink;

	// generic main GC procedure
	void Scan();

	// the part where only trimming (no resize adopt, no resize at the end).
	void PruneRetired ();

public:
	ThreadState_c ();
	~ThreadState_c ();

	// Main point to retire (delete) pointer. bNow forces to try actual removing right now
	void Retire ( RetiredPointer_t tPtr, bool bNow );

	// called once globally when daemon finishes - to finally delete everything
	void Shutdown();

	AtomicPointer_t* HazardAlloc();
	void HazardDealloc ( AtomicPointer_t * pPointer );
	void IterateHazards ( fnHazardProcessor&& fnProcessor ) const;

	static void FinallyPruneSink();
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
 	Gc performed in four steps:
 		1. We walk over all currently running threads and collect their hazard pointers
 		2. Sort and uniq the collection
 		3. Walk over current list of retired: if pointer is not reffered by any hazard - it is finaly pruned
 		4. Keep list of alive (reffered) pointers back to list of retired.

 		Since size of the list is by design greater then N of threads * N of hazards per thread, it WILL prune
 		some pointers definitely.
 */
CSphVector<Pointer_t> CollectActiveHazardPointers()
{
	// prepare vec for collect active hazard pointers
	CSphVector <Pointer_t> dActive;
	dActive.Reserve ( Threads::GetNumOfRunning () * POINTERS_PER_THREAD );

	// stage 1. Walk over ALL currently running threads and collect their hazard pointers
	Threads::IterateActive ([&dActive] ( Threads::LowThreadDesc_t * pDesc )
	{
		if (!pDesc)
			return;
		auto pOtherThreadState = (ThreadState_c *) pDesc->m_pHazards.load ( std::memory_order_relaxed );
		if ( !pOtherThreadState )
			return;

		pOtherThreadState->IterateHazards ( [&dActive] ( Pointer_t pPtr ) { dActive.Add ( pPtr ); } );
	});

	// stage 2. sort and uniq; we will use binsearch then
	dActive.Uniq();
	return dActive;
}

// Perform real work. If pruning object, in turn, includes hazard-guarded data inside,
// it will be retired into retiring list of current thread.
// Alive will be moved to the beginning of provided array, and function returns their quantity.
int PruneRetiredImpl ( RetiredPointer_t * pData, size_t iSize )
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
		else { // stage 4 - copy alive, back in the list
			if (pDst!=pSrc)
				*pDst=*pSrc;
			++pDst;
		}
	}

	return (int) (pDst - pData);
}

// that is shortcut for the special cases, like retiring huge blob of mem.
// We try to prune it immediately and return whether it was success or not
bool TryPruneOnePointer ( RetiredPointer_t* pSrc )
{
	auto dActive = CollectActiveHazardPointers ();
	if ( dActive.BinarySearch ( pSrc->first ) )
		return false;

	PrunePointer ( pSrc );
	return true;
}
} // unnamed namespace

// when thread hazard destructes, it may still have some alive retired
// we will leak them into this sink, and sometimes refine with another threads.
void RetiredSink_c::PruneSink ()
{
	auto iCompressed = PruneRetiredImpl ( m_dSink.begin (), m_dSink.GetLength () );
	m_dSink.Resize ( iCompressed );
}

void RetiredSink_c::MaybePruneSink ()
{
	if ( m_dSink.GetLength()>=GetCleanupSize () )
		PruneSink();
}

// adopt retired chunk from finishing thread
// called from thread d-tr, to sink pointers finaly alived.
// called from Shutdown, to sink last pointers in the process.
void RetiredSink_c::AdoptRetired ( VecOfRetired_t& dRetired )
{
	ScopedMutex_t _ ( m_dGuard );
	ScopedRole_c r ( thRetiringThread );
	m_dSink.Append ( dRetired.Slice ( 0, dRetired.m_iCurrent ) );
	dRetired.m_iCurrent = 0;
	MaybePruneSink ();
}

RetiredSink_c ThreadState_c::m_dGlobalSink;


static ThreadState_c & ThreadState ()
{
	static thread_local ThreadState_c tState;
	return tState;
}

ThreadState_c::ThreadState_c ()
	: m_tHazards { POINTERS_PER_THREAD }
{
	m_tRetired.Resize ( GetCleanupSize () );
	Threads::MyThd ().m_pHazards.store ( this, std::memory_order_relaxed );
}


ThreadState_c::~ThreadState_c ()
{
	while ( m_tRetired.NotEmpty() )
		m_dGlobalSink.AdoptRetired ( m_tRetired );
}

// GC enterpoint - called when no more place for retired
void ThreadState_c::Scan () REQUIRES ( thRetiringThread )
{
	// Before start, check if num of threads grown and so, we just need more space for retired
	if ( m_tRetired.GetLength() >= GetCleanupSize() )
		PruneRetired ();
	else
		m_tRetired.Resize ( GetCleanupSize () );
}

// main GC worker
void ThreadState_c::PruneRetired () REQUIRES ( thRetiringThread )
{
	// actually task is quite simple: we call ::PruneRetiredImpl, it keeps all alive pointers, prune all deads and return
	// the new size of our vec, containing only alive.
	// The problem is that deleting non-alive may include more to retire (if deleting object also has something to retire).
	// Since we're in call PruneRetired because we're full, we can't right now retire any more pointers.
	// So, we use this ping-pong with tmp vector to give room for nested retiring.
	VecOfRetired_t tCurrentRetired;
	tCurrentRetired.Resize ( GetCleanupSize () );

	m_tRetired.Swap ( tCurrentRetired );
	auto iRest = ::PruneRetiredImpl ( tCurrentRetired.begin (), tCurrentRetired.m_iCurrent );
	m_tRetired.Swap ( tCurrentRetired );
	m_tRetired.m_iCurrent = iRest;

	tCurrentRetired.Resize( tCurrentRetired.m_iCurrent );
	for ( auto dTailed : tCurrentRetired ) // some nested prune happened
		Retire ( dTailed, false );
}

void ThreadState_c::Retire ( RetiredPointer_t tPtr, bool bNow )
{
	if ( bNow && TryPruneOnePointer ( &tPtr ) )
		return;

	ScopedRole_c _ ( thRetiringThread );

	if ( m_tRetired.Retire( std::move (tPtr) ) )
		return;

	Scan();
	if ( m_tRetired.IsFull() )
		Scan();

	assert (!m_tRetired.IsFull());
}

AtomicPointer_t * ThreadState_c::HazardAlloc () ACQUIRE ( thHazardThread )
{
	return m_tHazards.Alloc();
}

void ThreadState_c::HazardDealloc ( AtomicPointer_t * pPointer ) RELEASE ( thHazardThread )
{
	m_tHazards.Dealloc ( (ListedPointer_t *) pPointer );
}

void ThreadState_c::IterateHazards ( fnHazardProcessor&& fnProcessor ) const
{
	for ( const auto & tHazard : m_tHazards )
	{
		// here 'acquire' semantic is complement to 'release' semantic on thread saving hazard ptr.
		auto pData = tHazard.m_pData.load ( std::memory_order_acquire );
		if ( pData )
			fnProcessor ( pData );
	}
}

void RetiredSink_c::FinallyPruneSink ()
{
	ScopedMutex_t _ ( m_dGuard );
	PruneSink ();

	// retire all non-sinked active
	for ( auto& dTailed : m_dSink )
		ThreadState ().Retire ( dTailed, false );
	m_dSink.Reset();
}

void ThreadState_c::FinallyPruneSink()
{
	m_dGlobalSink.FinallyPruneSink();
}

void ThreadState_c::Shutdown ()
{
	ScopedRole_c _ ( thRetiringThread );
	ThreadState_c::FinallyPruneSink ();

	int iIteration=1;
	while ( m_tRetired.NotEmpty () )
	{
		PruneRetired ();
		sphLogDebug ( "ThreadState_c::Shutdown() iteration %d, has %d", iIteration, m_tRetired.m_iCurrent );
		++iIteration;

		if ( iIteration>NESTED_LEVELS ) // assume there are max 10 nested levels, change
		{
			auto iActive = CollectActiveHazardPointers ().GetLength();
			auto iRunning = Threads::GetNumOfRunning ();
			auto iPointers = m_tRetired.m_iCurrent;
			sphWarning ( "Still %d threads, %d pointers, %d active pointers; assume deadlock, abort",
				iRunning, iPointers, iActive );
			return;
		}
	};
}

CSphVector<int> hazard::GetListOfPointed ( Accessor_fn&& fnAccess, int iCount )
{
	CSphVector<int> dResult;
	auto dActive = CollectActiveHazardPointers ();

	for (int i=0; i<iCount;++i )
		if ( dActive.BinarySearch ( fnAccess(i) ) )
			dResult.Add(i);

	return dResult;
}


hazard::Guard_c::Guard_c() ACQUIRE ( thHazardThread ) NO_THREAD_SAFETY_ANALYSIS
{
	m_pGuard = ThreadState ().HazardAlloc ();
	assert ( m_pGuard && "couldn't alloc hazard pointer. Not enough slots?" );
}

hazard::Guard_c::~Guard_c() RELEASE ( thHazardThread ) NO_THREAD_SAFETY_ANALYSIS
{
	if ( m_pGuard )
		ThreadState ().HazardDealloc ( (ListedPointer_t *) m_pGuard );
}

// main entry point to delete. On-the-fly filter out nullptr values.
void hazard::Retire ( Pointer_t pData, Deleter_fn fnDelete, bool bNow )
{
	if ( pData )
		ThreadState ().Retire ( { pData, fnDelete }, bNow );
}

void hazard::Shutdown ()
{
	ThreadState ().Shutdown();
	sphLogDebug ( "hazard::Shutdown() done" );
}
