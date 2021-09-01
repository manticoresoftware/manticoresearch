//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "threadutils.h"
#include "sphinx.h"

#if !_WIN32
// UNIX-specific headers and calls
#include <sys/syscall.h>
#include <signal.h>

// for thr_self()
#ifdef __FreeBSD__
#include <sys/thr.h>
#endif

#endif

using namespace Threads;

const char* TaskStateName ( TaskState_e eState )
{
	switch (eState)
	{
		case TaskState_e::UNKNOWN: return "-";
		case TaskState_e::HANDSHAKE: return "handshake";
		case TaskState_e::NET_READ: return "net_read";
		case TaskState_e::NET_WRITE: return "net_write";
		case TaskState_e::QUERY: return "query";
		case TaskState_e::NET_IDLE: return "net_idle";
	}
	return "unknown";
}

const char* ProtoName ( Proto_e eProto )
{
	switch ( eProto )
	{
		case Proto_e::UNKNOWN: return "-";
		case Proto_e::SPHINX:
		case Proto_e::SPHINXSE: return "sphinx";
		case Proto_e::MYSQL41: return "mysql";
		case Proto_e::HTTP: return "http";
		case Proto_e::HTTPS: return "https";
		case Proto_e::REPLICATION: return "replication";
		default: break;
	}
	return "unknown";
}

const char * RelaxedProtoName ( Proto_e eProto )
{
	switch ( eProto )
	{
		case Proto_e::UNKNOWN: return "-";
		case Proto_e::MYSQL41: return "mysql";
		case Proto_e::REPLICATION: return "replication";
		case Proto_e::SPHINX:
		case Proto_e::HTTP: return "sphinx and http(s)";
		case Proto_e::HTTPS: return "https";
		case Proto_e::SPHINXSE: return "sphinx (to connect from SphinxSE)";
		default: break;
	}
	return "unknown";
}


int GetOsThreadId ()
{
#if _WIN32
	return GetCurrentThreadId();
#elif defined ( __APPLE__ )
	uint64_t tid;
	 pthread_threadid_np(NULL, &tid);
	return tid;
#elif defined(SYS_gettid)
	return syscall ( SYS_gettid );
#elif defined(__FreeBSD__)
	long tid;
	thr_self(&tid);
	return (int)tid;
#else
	return 0;
#endif
}

#include <atomic>
#include "event.h"
#include "optional.h"

//////////////////////////////////////////////////////////////////////////
/// functional threadpool with minimum footprint

#define LOG_LEVEL_DEBUG false
#define LOG_LEVEL_DETAIL false

namespace Threads {

namespace logdetail {
	const char* name() { return MyThd ().m_sThreadName.cstr(); }
}


#define LOG_COMPONENT_MT "(" << GetOsThreadId() << ") " << logdetail::name() << ": "

// list (FIFO queue) of operations to perform. Used in service queue.
template<typename Operation>
class OpQueue_T : public ISphNoncopyable
{
	Operation * m_pFront = nullptr; // The front of the queue.
	Operation * m_pBack = nullptr; // The back of the queue.

public:
	// destroys all operations.
	~OpQueue_T ()
	{
		while (Operation * pOp = m_pFront)
		{
			Pop ();
			pOp->Destroy();
		}
	}

	// Get the operation at the front of the queue.
	Operation * Front ()
	{
		return m_pFront;
	}

	// Pop an operation from the front of the queue.
	void Pop ()
	{
		if ( m_pFront )
		{
			auto * tmp = m_pFront;
			m_pFront = tmp->m_pNext;
			if ( !m_pFront )
				m_pBack = nullptr;
			tmp->m_pNext = nullptr;
		}
	}

	// Push an operation on to the back of the queue.
	void Push ( Operation * pOp )
	{
		pOp->m_pNext = nullptr;
		if ( m_pBack )
		{
			m_pBack->m_pNext = pOp;
			m_pBack = pOp;
		} else
			m_pFront = m_pBack = pOp;
	}

	// Push an operation on to the front of the queue.
	void Push_front ( Operation * pOp )
	{
		if ( m_pFront )
		{
			pOp->m_pNext = m_pFront;
			m_pFront = pOp;
		} else
		{
			pOp->m_pNext = nullptr;
			m_pFront = m_pBack = pOp;
		}
	}

	// Push all operations from another queue on to the back of the queue. The
	// source queue may contain operations of a derived type.
	template<typename OtherOperation>
	void Push ( OpQueue_T<OtherOperation> & rhs )
	{
		auto pRhsFront = rhs.m_pFront;
		if ( pRhsFront )
		{
			if ( m_pBack )
				m_pBack->m_pNext = pRhsFront;
			else
				m_pFront = pRhsFront;
			m_pBack = rhs.m_pBack;
			rhs.m_pFront = nullptr;
			rhs.m_pBack = nullptr;
		}
	}

	// Whether the queue is empty.
	bool Empty () const
	{
		return m_pFront==nullptr;
	}

	// Test whether an operation is already enqueued.
	bool IsEnqueued ( Operation * pOp ) const
	{
		return pOp->m_pNext || m_pBack==pOp;
	}

	// find elem (linear search), remove from list and destroy
	void RemoveAndDestroy ( Operation * pOp )
	{
		if ( !pOp )
			return;

		if ( m_pFront==pOp )
		{
			m_pFront = pOp->m_pNext;
		} else
		{
			for ( auto pCurOp = m_pFront; pCurOp!=nullptr; pCurOp = pCurOp->m_pNext )
			{
				if ( pCurOp->m_pNext==pOp )
				{
					pCurOp->m_pNext = pOp->m_pNext;
					if ( !pCurOp->m_pNext )
						m_pBack = pCurOp;
					break;
				}
			}
		}
		if ( !m_pFront )
			m_pBack = nullptr;
		pOp->Destroy ();
	}

	class Iterator_c
	{
		Operation * m_pIterator = nullptr;
	public:
		explicit Iterator_c ( Operation * pIterator = nullptr ) : m_pIterator ( pIterator )
		{}

		Operation & operator* ()
		{
			return *m_pIterator;
		}

		Iterator_c & operator++ ()
		{
			m_pIterator = m_pIterator->m_pNext;
			return *this;
		}

		bool operator!= ( const Iterator_c & rhs ) const
		{
			return m_pIterator!=rhs.m_pIterator;
		}
	};

	// c++11 style iteration
	Iterator_c begin () const
	{
		return Iterator_c ( m_pFront );
	}

	Iterator_c end () const
	{
		return Iterator_c();
	}
};

// Base class for all operations. A function pointer is used instead of virtual
// functions to avoid the associated overhead.
class SchedulerOperation_t
{
protected:
	using fnFuncType = void ( void*, SchedulerOperation_t* );
	friend class OpQueue_T<SchedulerOperation_t>;

private:
	SchedulerOperation_t * m_pNext = nullptr;
	fnFuncType * m_fnFunc;

public:
  void Complete( void* pOwner )
  {
	  m_fnFunc ( pOwner, this );
  }

  void Destroy()
  {
	  m_fnFunc ( nullptr, this );
  }

protected:
	// protect ctr and dtr of this type
	SchedulerOperation_t ( fnFuncType* fnFunc ) : m_fnFunc ( fnFunc ) {}
	~SchedulerOperation_t () = default;
};

using OpSchedule_t = OpQueue_T<SchedulerOperation_t>;

// actual operation which completes given Handler
template <typename Handler>
class CompletionHandler_c : public SchedulerOperation_t
{
	Handler m_Handler;

public:
	explicit CompletionHandler_c ( Handler h )
		: SchedulerOperation_t ( &CompletionHandler_c::DoComplete )
		, m_Handler ( static_cast<const Handler&> ( h )) // force copying
	{}

	static void DoComplete ( void * pOwner, SchedulerOperation_t * pBase )
	{
		// Take ownership of the handler object.
		auto * pHandler = static_cast<CompletionHandler_c *>(pBase);

		// make a copy of handler before upcall.
		Handler dLocalHandler ( static_cast<const Handler &> ( pHandler->m_Handler ));

		// deallocate before the upcall
		SafeDelete ( pHandler );

		// Make the upcall if required.
		if ( pOwner )
		{
			Threads::JobTimer_t dTrack;
			dLocalHandler();

			// barrier ensures that no operations till here would be reordered below.
			std::atomic_thread_fence ( std::memory_order_release );
		}
	}
};


struct TaskServiceThreadInfo_t
{
	OpSchedule_t m_dPrivateQueue;
	long m_iPrivateOutstandingWork = 0;
};

class TaskService_t
{
public:
	using operation = SchedulerOperation_t;
};

// Helper class to determine whether or not the current thread is inside an
// invocation of Service_t::run() for a specified service object.
// That may be used to optimize codeflow (like place continuation without locks)
template<typename Key, typename Value = BYTE>
class CallStack_c
{
public:
	// Context class automatically pushes the key/value pair on to the stack.
	class Context_c : public ISphNoncopyable
	{
		Key * m_pService; // The key associated with the context.
		Value * m_pThisContext; // The value associated with the context.
		Context_c * m_pNext; // The next element in the stack.
		friend class CallStack_c<Key, Value>;

	public:
		// Push the key on to the stack.
		explicit Context_c ( Key * pService )
			: m_pService ( pService ),
			m_pNext ( CallStack_c<Key, Value>::m_pTop )
		{
			m_pThisContext = (Value *)this;
			CallStack_c<Key, Value>::m_pTop = this;
		}

		// Push the key/value pair on to the stack.
		Context_c ( Key * pKey, Value & v )
			: m_pService ( pKey ),
			m_pThisContext ( &v ),
			m_pNext ( CallStack_c<Key, Value>::m_pTop )
		{
			CallStack_c<Key, Value>::m_pTop = this;
		}

		// Pop the key/value pair from the stack.
		~Context_c ()
		{
			CallStack_c<Key, Value>::m_pTop = m_pNext;
		}

		// Find the next context with the same key.
		Value * NextByKey () const
		{
			for ( auto* pElem = m_pNext; pElem!=nullptr; pElem = pElem->m_pNext )
				if ( pElem->m_pService==m_pService )
					return pElem->m_pThisContext;
			return nullptr;
		}
	};

	friend class Context_c;

	// Determine whether the specified owner is on the stack.
	// Returns address of key, if present, nullptr otherwise.
	static Value * Contains ( Key * pKey )
	{
		for ( auto* pElem = m_pTop; pElem!=nullptr; pElem = pElem->m_pNext )
			if ( pElem->m_pService==pKey )
				return pElem->m_pThisContext;
		return nullptr;
	}

	// Obtain the value at the top of the stack.
	static Value * Top ()
	{
		Context_c * pElem = m_pTop;
		return pElem ? pElem->m_pThisContext : nullptr;
	}

private:
	// The top of the stack of calls for the current thread.
	static thread_local Context_c* m_pTop;
};

template<typename Key, typename Value>
thread_local typename CallStack_c<Key, Value>::Context_c* CallStack_c<Key,Value>::m_pTop = nullptr;

struct Service_i
{
	virtual ~Service_i() {}
	virtual void run() = 0;
	virtual void reset() = 0;
};

#define LOG_LEVEL_WORKS false
#define LOG_LEVEL_ST false

/// performs tasks pushed with post() in one or many threads until they done.
/// Naming convention of members is inherited from boost::asio as drop-in replacement.
struct Service_t : public TaskService_t, public Service_i
{
	std::atomic<long> m_iOutstandingWork {0};	/// count of unfinished works
	mutable CSphMutex m_dMutex;					/// protect access to internal data
	bool m_bStopped = false;                	/// dispatcher has been stopped.
	bool m_bOneThread;                			/// optimize for single-threaded use case
	sph::Event_c m_tWakeupEvent;				/// event to wake up blocked threads
	OpSchedule_t m_OpQueue;						/// The queue of handlers that are ready to be delivered
	OpSchedule_t m_OpVipQueue;					/// The queue of handlers that have to be delivered BEFORE OpQueue

	// Per-thread call stack to track the state of each thread in the service.
	using ThreadCallStack_c = CallStack_c<Service_t, TaskServiceThreadInfo_t>;

	class Work_c;			/// Scoped RAII work to keep service running. calls work_started/work_finished
	friend class Work_c;

public:

	explicit Service_t (bool bOneThread)
	: m_bOneThread ( bOneThread ) {}

	template<typename Handler>
	void post ( Handler handler ) // post into secondary queue
	{
		// Allocate and construct an operation to wrap the handler.
		post_immediate_completion ( new CompletionHandler_c<Handler> ( std::move ( handler ) ), false );
	}

	template<typename Handler>
	void defer ( Handler handler ) // post into primary queue
	{
		// Allocate and construct an operation to wrap the handler.
		post_immediate_completion ( new CompletionHandler_c<Handler> ( std::move ( handler ) ), true );
	}

	template<typename Handler>
	void post_continuationHandler ( Handler handler ) // try to execute immediately, or then post to primary queue
	{
		// Allocate and construct an operation to wrap the handler.
		post_continuation ( new CompletionHandler_c<Handler> ( std::move ( handler ) ) );
	}

	void post_continuation ( Service_t::operation * pOp )
	{
		auto * pThisThread = ThreadCallStack_c::Contains ( this );
		if ( pThisThread )
		{
			++pThisThread->m_iPrivateOutstandingWork;
			pThisThread->m_dPrivateQueue.Push ( pOp );
			LOG ( DETAIL, MT ) << "post this";
			return;
		}

		work_started ();
		ScopedMutex_t dLock ( m_dMutex );
		LOG ( DETAIL, MT ) << "post";
		m_OpVipQueue.Push ( pOp );
		wake_one_thread_and_unlock ( dLock );
	}

	void post_immediate_completion ( Service_t::operation * pOp, bool bVip )
	{
		if ( m_bOneThread )
		{
			auto * pThisThread = ThreadCallStack_c::Contains ( this );
			if ( pThisThread )
			{
				++pThisThread->m_iPrivateOutstandingWork;
				pThisThread->m_dPrivateQueue.Push ( pOp );
				LOG ( DETAIL, MT ) << "post this";
				return;
			}
		}
		work_started ();
		ScopedMutex_t dLock ( m_dMutex );
		LOG ( DETAIL, MT ) << "post";
		if ( bVip )
			m_OpVipQueue.Push ( pOp );
		else
			m_OpQueue.Push ( pOp );
		wake_one_thread_and_unlock ( dLock );
	}

	void run () NO_THREAD_SAFETY_ANALYSIS override
	{
		LOG ( DETAIL, MT ) << "run " << m_iOutstandingWork << " st:" << !!m_bStopped;
		if ( m_iOutstandingWork==0 )
		{
			LOG ( WORKS, MT ) << "run m_iOutstandingWork " << m_iOutstandingWork << " " << &m_iOutstandingWork<< " stop!";
			stop();
			return;
		}

		TaskServiceThreadInfo_t dThisThread;
		dThisThread.m_iPrivateOutstandingWork = 0;
		ThreadCallStack_c::Context_c dCtx ( this, dThisThread );

		ScopedMutex_t dLock ( m_dMutex );
		while ( do_run_one ( dLock, dThisThread ) )
			dLock.Lock ();
	}

	bool queue_empty() const REQUIRES ( m_dMutex )
	{
		return m_OpQueue.Empty () && m_OpVipQueue.Empty ();
	}

	bool do_run_one ( ScopedMutex_t& dLock, TaskServiceThreadInfo_t& this_thread )
		REQUIRES ( dLock ) RELEASE ( dLock ) TRY_ACQUIRE ( false, dLock )
	{
		while ( !m_bStopped )
		{
			LOG ( DETAIL, MT ) << "locked " << dLock.Locked();
			assert ( dLock.Locked ());
			if ( queue_empty() )
			{
				m_tWakeupEvent.Clear ( dLock );
				m_tWakeupEvent.Wait ( dLock );
				continue;
			}

			auto & dOpQueue = m_OpVipQueue.Empty () ? m_OpQueue : m_OpVipQueue;
			auto * pOp = dOpQueue.Front ();
			dOpQueue.Pop ();

			if ( !queue_empty () && !m_bOneThread )
				wake_one_thread_and_unlock ( dLock );
			else
				dLock.Unlock ();

			pOp->Complete (this);

			LOG ( DETAIL, MT ) << "completed & unlocked";
			if ( this_thread.m_iPrivateOutstandingWork>1 )
			{
				m_iOutstandingWork += this_thread.m_iPrivateOutstandingWork-1;
				LOG ( WORKS, MT ) << "do_run_one m_iOutstandingWork " << m_iOutstandingWork << " " << &m_iOutstandingWork;
			}
			else if ( this_thread.m_iPrivateOutstandingWork<1 )
				work_finished ();

			this_thread.m_iPrivateOutstandingWork = 0;
			if ( !this_thread.m_dPrivateQueue.Empty ())
			{
				dLock.Lock ();
				m_OpVipQueue.Push ( this_thread.m_dPrivateQueue );
			}
			return true;
		}
		return false;
	}

	void stop()
	{
		LOG ( DETAIL, MT ) << "stop";
		ScopedMutex_t dLock ( m_dMutex );
		stop_all_threads ( dLock );
	}

	bool stopped () const
	{
		ScopedMutex_t dLock ( m_dMutex );
		return m_bStopped;
	}

	void reset () override
	{
		LOG ( DETAIL, MT ) << "reset stopped ";
		ScopedMutex_t dLock ( m_dMutex );
		m_bStopped = false;
	}

	// Notify that some work has started.
	void work_started ()
	{
		LOG ( DETAIL, MT ) << "work_started from " << m_iOutstandingWork;
		++m_iOutstandingWork;
		LOG ( WORKS, MT ) << "work_started m_iOutstandingWork " << m_iOutstandingWork << " " << &m_iOutstandingWork;
	}

	// Notify that some work has finished.
	void work_finished ()
	{
		LOG ( DETAIL, MT ) << "work_finished " << m_iOutstandingWork;
		if ( --m_iOutstandingWork==0 )
			stop ();

		LOG ( WORKS, MT ) << "work_finished m_iOutstandingWork " << m_iOutstandingWork << " " << &m_iOutstandingWork;
	}

	void stop_all_threads ( ScopedMutex_t & dLock ) REQUIRES ( dLock )
	{
		m_bStopped = true;
		m_tWakeupEvent.SignalAll ( dLock );
	}

	void wake_one_thread_and_unlock ( ScopedMutex_t & dLock ) REQUIRES ( dLock ) RELEASE ( dLock )
	{
		if ( !m_tWakeupEvent.MaybeUnlockAndSignalOne ( dLock ))
			dLock.Unlock ();
	}

	long works() const
	{
		return m_iOutstandingWork;
	}
};

/// helper to hold the service running
class Service_t::Work_c
{
	Service_t& m_tServiceRef;

public:
	explicit Work_c( Service_t& tService )
		: m_tServiceRef (tService)
	{
		m_tServiceRef.work_started ();
	}

	Work_c ( const Work_c& tOther)
		: m_tServiceRef ( tOther.m_tServiceRef )
	{
		m_tServiceRef.work_started ();
	}

	Work_c & operator= ( const Work_c & ) = delete;

	~Work_c()
	{
		m_tServiceRef.work_finished();
	}
};

#define LOG_COMPONENT_TP LOG_COMPONENT_MT << ": "
#define LOG_COMPONENT_ST LOG_COMPONENT_MT << " strand: "

// strand - sequental scheduler. Operations executed strictly sequentally and in FIFO order.
// It looks like 'single thread', but actual thread is provided from backend and may change.
class Strand_c final : public SchedulerWithBackend_i
{
	const char* m_szName = nullptr;
	// strand has no backend thread/threadpool and works over another scheduler.
	Scheduler_i * m_pBackend;

	CSphMutex m_dMutex;
	bool m_bLocked GUARDED_BY ( m_dMutex ) = false;
	bool m_bShutdown = false;
	OpSchedule_t m_OpReadyQueue;	/// The queue for current run
	OpSchedule_t m_OpWaitQueue GUARDED_BY ( m_dMutex );	/// The queue for the next run

	// Per-thread call stack to track the state of each thread in the service.
	using StrandCallStack_c = CallStack_c<Strand_c>;

	class Invoker_c
	{
		Keeper_t m_tParentKeeper;
		Strand_c * m_pOwner;

	public:
		explicit Invoker_c( Strand_c* pRand );
		Invoker_c ( const Invoker_c& rhs ) = default;
		Invoker_c ( Invoker_c && rhs ) noexcept;
		Invoker_c & operator= ( Invoker_c && rhs ) noexcept;
		void run ();
	};

	friend class Invoker_c;

	bool Enqueue ( Handler handler )
	{
		ScopedMutex_t tLock ( m_dMutex );
		if ( m_bShutdown )
			return false;

		auto * pOp = new CompletionHandler_c<Handler> ( std::move ( handler ) );
		if ( m_bLocked )
		{
			m_OpWaitQueue.Push (pOp);
			LOG ( ST, ST ) << "		enqueued to wait queue, was locked " << pOp;
			return false;
		}

		m_bLocked = true;
		tLock.Unlock();
		m_OpReadyQueue.Push(pOp);
		LOG ( ST, ST ) << "		enqueued to ready queue, locked " << pOp;
		return true;
	}

	// try to execute immediately, or then post to primary queue
	void PostContinuationToBackend ( Handler handler )
	{
		// Add the function to the strand and schedule the strand if required.
		if ( m_pBackend )
			m_pBackend->ScheduleContinuation ( std::move(handler) );
	}

	void PostContinuation ( Handler handler ) // try to execute immediately, or then post to primary queue
	{
		auto bThisThread = !!StrandCallStack_c::Contains ( this );
		if ( bThisThread )
		{
			LOG ( ST, ST ) << "PostContinuation fast in this thread";
			Threads::JobTimer_t dTrack;

			// barrier ensures that no operations till here would be reordered below.
			std::atomic_thread_fence ( std::memory_order_acquire );
			handler ();
			std::atomic_thread_fence ( std::memory_order_release );

			LOG ( ST, ST ) << "strand PostContinuation performed without queuing";
			return;
		}

		bool bFirst = Enqueue ( std::move ( handler ) );

		// Add the function to the strand and schedule the strand if required.
		if ( bFirst )
		{
			Invoker_c tInvoker { this };
			PostContinuationToBackend ( [t=std::move(tInvoker)] () mutable { t.run(); } );
		}
	}

	void Kick()
	{
		Invoker_c tInvoker { this };
		PostContinuationToBackend( [t = std::move ( tInvoker )] () mutable { t.run (); } );
	}

public:
	explicit Strand_c ( Scheduler_i* pBackend, const char* szName=nullptr )
		: m_szName { szName }
		, m_pBackend { pBackend }
	{}

	void Schedule ( Handler handler, bool bVip ) final
	{
		LOG ( ST, ST ) << "Post";
		bool bFirst = Enqueue ( std::move ( handler ) );
		if ( bFirst && m_pBackend )
		{
			LOG ( ST, ST ) << "Post scheduled invoker to backend";
			Invoker_c tInvoker { this };
			m_pBackend->Schedule ( [t=std::move(tInvoker)] () mutable { t.run (); }, bVip );
		}
		LOG ( ST, ST ) << "Post finished";
	}

	void ScheduleContinuation ( Handler handler ) final
	{
		LOG ( ST, ST ) << "ScheduleContinuation";
		PostContinuation ( std::move ( handler ) );
		LOG ( ST, ST ) << "Post finished";
	}

	Keeper_t KeepWorking () final
	{
		assert ( m_pBackend );
		return m_pBackend->KeepWorking();
	}

	bool SetBackend ( Scheduler_i* pBackend ) final
	{
		ScopedMutex_t tLock ( m_dMutex );
		if ( m_bLocked )
		{
			if ( m_pBackend ) // everything healthy and work, can't change right now
				return false;

			assert ( !m_pBackend );
			m_pBackend = pBackend;
			Kick();
		}

		m_pBackend = pBackend;
		return true;
	}

	const char * Name () const final { return m_szName; }
};

Strand_c::Invoker_c::Invoker_c ( Strand_c * pRand )
	: m_tParentKeeper { pRand->KeepWorking () }
	, m_pOwner ( pRand )
{}

Strand_c::Invoker_c::Invoker_c ( Strand_c::Invoker_c && rhs ) noexcept
	: m_pOwner ( rhs.m_pOwner )
{
	m_tParentKeeper.Swap ( rhs.m_tParentKeeper );
}

Strand_c::Invoker_c & Strand_c::Invoker_c::operator= ( Strand_c::Invoker_c && rhs ) noexcept
{
	m_tParentKeeper.Swap ( rhs.m_tParentKeeper );
	m_pOwner = rhs.m_pOwner;
	return *this;
}

void Strand_c::Invoker_c::run ()
{
	struct OnInvokerFinished_t
	{
		Strand_c::Invoker_c* m_pThis;

		~OnInvokerFinished_t()
		{
			bool bMoreHandlers;
			auto* pOwner = m_pThis->m_pOwner;
			{
				ScopedMutex_t tLock ( pOwner->m_dMutex );
				pOwner->m_OpReadyQueue.Push ( pOwner->m_OpWaitQueue );
				bMoreHandlers = pOwner->m_bLocked = !pOwner->m_OpReadyQueue.Empty ();
			}

			LOG ( ST, ST ) << "OnInvokerFinished_t: " << bMoreHandlers;

			if ( !bMoreHandlers )
			{
				LOG ( ST, ST ) << "OnInvokerFinished_t, abandoned, unlocked";
				return;
			}
			LOG ( ST, ST ) << "OnInvokerFinished_t, have more, locked";

			Strand_c::Invoker_c tInvoker { *m_pThis };
//			pOwner->Schedule ( [t=std::move(tInvoker)] () mutable { t.run (); }, true );
			pOwner->PostContinuationToBackend ( [t = std::move ( tInvoker )] () mutable { t.run (); } );
		}
	};

	StrandCallStack_c::Context_c dCtx ( m_pOwner );

	// that will ensure the next handler, if any, will be scheduled on block exit
	OnInvokerFinished_t VARIABLE_IS_NOT_USED dOnFinished = { this };

	// Run all ready handlers. No lock is required since the ready queue is
	// accessed only within the strand.
	while ( !m_pOwner->m_OpReadyQueue.Empty () )
	{
		auto * pOp = m_pOwner->m_OpReadyQueue.Front ();
		m_pOwner->m_OpReadyQueue.Pop ();
		LOG ( ST, ST ) << "run op: " << pOp;
		pOp->Complete ( pOp );
	}
}

class ThreadPool_c final : public Worker_i
{
	using Work = Service_t::Work_c;

	const char * m_szName = nullptr;
	Service_t m_tService;
	Optional_T<Work> m_dWork;
	CSphVector<SphThread_t> m_dThreads;
	CSphMutex m_dMutex;
	sph::Event_c m_dCond;
	std::atomic<bool> m_bStop {false};

	// support iteration over children for show threads and hazards
	RwLock_t m_dChildGuard;
	CSphVector<LowThreadDesc_t *> m_dChildren GUARDED_BY ( m_dChildGuard);

	template<typename F>
	void Post ( F && f, bool bVip = false ) // post to primary (vip) or secondary queue
	{
		LOG ( DETAIL, TP ) << "Post " << bVip;
		if ( bVip )
			m_tService.defer ( std::forward<F> ( f ));
		else
			m_tService.post ( std::forward<F> ( f ));
		LOG ( DETAIL, TP ) << "Post finished";
	}

	template<typename F>
	void PostContinuation ( F && f ) // 'very vip' - try to execute immediately, or post to the primary queue
	{
		LOG ( DETAIL, TP ) << "PostContinuation";
		m_tService.post_continuationHandler( std::forward<F> ( f ));
		LOG ( DETAIL, TP ) << "Post finished";
	}

	Service_i & Service ()
	{
		return m_tService;
	}

	void createWork ()
	{
		m_dWork.emplace ( m_tService );
	}

	void loop (int iChild)
	{
		{
			ScWL_t _ ( m_dChildGuard );
			m_dChildren[iChild] = &MyThd ();
		}
		while (true)
		{
			m_tService.run ();
			ScopedMutex_t dLock {m_dMutex};
			if ( m_bStop )
				break;
			if ( !m_dWork )
			{
				createWork ();
				m_tService.reset ();
				m_dCond.SignalAll (dLock);
			}
		}
		ScWL_t _ ( m_dChildGuard );
		m_dChildren[iChild] = nullptr;
	}

public:
	ThreadPool_c ( size_t iThreadCount, const char * szName )
		: m_szName {szName}
		, m_tService ( iThreadCount==1 )
	{
		createWork ();
		m_dThreads.Resize ( (int) iThreadCount );
		m_dChildren.Resize ( (int) iThreadCount );
		m_dChildren.ZeroVec(); // avoid iterations over not-yet-started threads with garbage here
		ARRAY_FOREACH ( i, m_dThreads )
			Threads::CreateQ ( &m_dThreads[i], [this,i] { loop (i); }, false, m_szName, i );
		LOG ( DEBUG, TP ) << "thread pool created with threads: " << iThreadCount;
	}

	~ThreadPool_c () final
	{
		StopAll();
		ScWL_t _ ( m_dChildGuard ); // that will keep children list if smbody still iterates over it
	}

	void DiscardOnFork () final
	{
		m_dThreads.Reset();
	}

	void Schedule ( Handler handler, bool bVip ) final
	{
		Post ( std::move ( handler ), bVip );
	}

	void ScheduleContinuation ( Handler handler ) final
	{
		PostContinuation ( std::move ( handler ) );
	}

	Keeper_t KeepWorking () final
	{
		m_tService.work_started ();
		return Keeper_t ( nullptr, [this] ( void * ) { m_tService.work_finished (); } );
	}

	int WorkingThreads () const final
	{
		return m_dThreads.GetLength ();
	}

	int Works () const final
	{
		return (int)m_tService.works ();
	}

	void IterateChildren ( ThreadFN& fnHandler ) final
	{
		ScRL_t _ ( m_dChildGuard );
		for ( auto* pThread : m_dChildren )
			fnHandler ( pThread );
	}

	void StopAll () final
	{
		ScopedMutex_t dLock { m_dMutex };
		m_bStop = true;
		m_dWork.reset ();
		dLock.Unlock ();
		LOG ( DEBUG, TP ) << "stopping thread pool";
		for ( auto & dThread : m_dThreads )
			Threads::Join ( &dThread );
		LOG ( DEBUG, TP ) << "thread pool stopped";
		m_dThreads.Resize ( 0 );
	}
};

class AloneThread_c final : public Worker_i
{
	CSphString m_sName;
	int m_iThreadNum;
	Service_t m_tService;
	std::atomic<bool> m_bStarted {false};
	static int m_iRunningAlones;

	template<typename F>
	void Post ( F && f, bool bVip=false ) // post to primary (vip) or secondary queue
	{
		LOG ( DETAIL, TP ) << "Post " << bVip;
		if ( bVip )
			m_tService.defer ( std::forward<F> ( f ) );
		else
			m_tService.post ( std::forward<F> ( f ));
		LOG ( DETAIL, TP ) << "Post finished";
		if ( !m_bStarted )
		{
			m_bStarted = true;
			SphThread_t tThd; // dummy, since we're starting detached
			Threads::CreateQ ( &tThd, [this] { loop (); }, true, m_sName.cstr (), m_iThreadNum );

			LOG ( DEBUG, TP ) << "alone thread created";
		}
	}

	void loop ()
	{
		Detached::AddThread ( &MyThd () );
		m_tService.run ();
		Detached::RemoveThread ( &MyThd () );
		delete this;
	}

public:
	explicit AloneThread_c ( int iNum, const char * szName )
		: m_sName {szName}
		, m_iThreadNum ( iNum )
		, m_tService ( true ) // true means 'single-thread'
	{
		++m_iRunningAlones;
		LOG ( DEBUG, TP ) << "alone worker created " << szName;
	}

	~AloneThread_c () final
	{
		LOG ( DEBUG, TP ) << "stopping thread";
		--m_iRunningAlones;
		LOG ( DEBUG, TP ) << "thread stopped";
	}

	void Schedule ( Handler handler, bool bVip ) final
	{
		Post ( std::move ( handler ), bVip );
	}

	Keeper_t KeepWorking () final
	{
		m_tService.work_started ();
		return Keeper_t ( nullptr, [this] ( void * ) { m_tService.work_finished (); } );
	}

	void StopAll () final {}

	static int GetRunners () { return m_iRunningAlones; }

	int Works () const final
	{
		return GetRunners ();
	}
};

int AloneThread_c::m_iRunningAlones = 0;


WorkerSharedPtr_t MakeThreadPool ( size_t iThreadCount, const char* szName )
{
	return WorkerSharedPtr_t { new ThreadPool_c ( iThreadCount, szName ) };
}

WorkerSharedPtr_t MakeAloneThread ( size_t iOrderNum, const char* szName )
{
	return WorkerSharedPtr_t { new AloneThread_c ( (int)iOrderNum, szName ) };
}

// Alone scheduler works on top of another scheduler and provides sequental execution of the tasks (each time only one
// task may be performed, no concurrent execution). It also gives FIFO ordering of the tasks.
SchedulerSharedPtr_t MakeAloneScheduler ( Scheduler_i* pBase, const char* szName )
{
	return SchedulerSharedPtr_t { new Strand_c ( pBase, szName ) };
}

} // namespace Threads


namespace {
	RwLock_t & g_lShutdownGuard ()
	{
		static RwLock_t lShutdownGuard;
		return lShutdownGuard;
	}

	OpSchedule_t & g_dShutdownList ()
	{
		static OpSchedule_t dShutdownList GUARDED_BY ( g_lShutdownGuard () );
		return dShutdownList;
	}

	OpSchedule_t & g_dOnForkList ()
	{
		static OpSchedule_t dOnForkList GUARDED_BY ( g_lShutdownGuard () );
		return dOnForkList;
	}
}

void searchd::AddShutdownCb ( Handler fnCb )
{
	auto pCb = ( new CompletionHandler_c<Handler> ( std::move ( fnCb ) ) );
	ScWL_t tGuard ( g_lShutdownGuard() );
	g_dShutdownList().Push_front( pCb );
}

void searchd::AddOnForkCleanupCb ( Threads::Handler fnCb )
{
	auto pCb = ( new CompletionHandler_c<Handler> ( std::move ( fnCb ) ) );
	ScWL_t tGuard ( g_lShutdownGuard () );
	g_dOnForkList ().Push_front ( pCb );
}

// invoke shutdown handlers
void searchd::FireShutdownCbs ()
{
	ScRL_t tGuard ( g_lShutdownGuard() );
	while ( !g_dShutdownList().Empty () )
	{
		auto * pOp = g_dShutdownList().Front ();
		g_dShutdownList().Pop ();
		pOp->Complete ( pOp );
	}
}

void searchd::CleanAfterFork () NO_THREAD_SAFETY_ANALYSIS
{
	while ( !g_dOnForkList ().Empty () )
	{
		auto * pOp = g_dOnForkList ().Front ();
		g_dOnForkList ().Pop ();
		pOp->Complete ( pOp );
	}

	while ( !g_dShutdownList().Empty () )
	{
		auto * pOp = g_dShutdownList().Front ();
		g_dShutdownList().Pop ();
		pOp->Destroy();
	}
}

static int g_iMaxChildrenThreads = 1;


namespace {
WorkerSharedPtr_t& GlobalPoolSingletone ()
{
	static WorkerSharedPtr_t pPool;
	return pPool;
}
}

void StartGlobalWorkPool ()
{
	sphLogDebug ( "StartGlobalWorkpool" );
	WorkerSharedPtr_t& pPool = GlobalPoolSingletone ();
	pPool = new ThreadPool_c ( g_iMaxChildrenThreads, "work" );
}

void SetMaxChildrenThreads ( int iThreads )
{
	sphLogDebug ( "SetMaxChildrenThreads to %d", iThreads );
	g_iMaxChildrenThreads = Max ( 1, iThreads );
}

Threads::Worker_i * GlobalWorkPool ()
{
	WorkerSharedPtr_t& pPool = GlobalPoolSingletone ();
	assert ( pPool && "invoke StartGlobalWorkPool first");
	return pPool;
}

void WipeGlobalSchedulerOnShutdownAndFork ()
{
#ifndef NDEBUG
	static bool bAlreadyInvoked = false;
	assert (!bAlreadyInvoked);
	bAlreadyInvoked = true;
#endif

	Threads::RegisterIterator ( [] ( ThreadFN & fnHandler ) {
		WorkerSharedPtr_t& pPool = GlobalPoolSingletone ();
		if ( pPool )
			pPool->IterateChildren ( fnHandler );
	} );

	searchd::AddOnForkCleanupCb ( [] {
		WorkerSharedPtr_t& pPool = GlobalPoolSingletone ();
		if ( pPool )
			pPool->DiscardOnFork ();
	} );

	searchd::AddShutdownCb ( [] {
		WorkerSharedPtr_t& pPool = GlobalPoolSingletone ();
		if ( pPool )
			pPool->StopAll ();
	} );
}

void WipeSchedulerOnFork ( Threads::Worker_i* pWorker )
{
	Threads::RegisterIterator ( [pWorker] ( ThreadFN& fnHandler ) {
		if ( pWorker )
			pWorker->IterateChildren ( fnHandler );
	} );

	searchd::AddOnForkCleanupCb ( [pWorker] {
		if ( pWorker )
			pWorker->DiscardOnFork();
	} );
}


namespace {
	static std::atomic<int> g_iRunningThreads {0};
}

int Threads::GetNumOfRunning()
{
	return g_iRunningThreads.load ( std::memory_order_relaxed );
}

//////////////////////////////////////////////////////////////////////////
/// helpers to iterate over all registered threads

class OperationsQueue_c::Impl_c
{
	CSphMutex m_tQueueGuard;
	OpSchedule_t m_tQueue GUARDED_BY ( m_tQueueGuard );

public:
	void AddOp ( Handler fnCb )
	{
		auto pCb = ( new CompletionHandler_c<Handler> ( std::move ( fnCb ) ) );
		ScopedMutex_t tGuard ( m_tQueueGuard );
		m_tQueue.Push_front ( pCb );
	}

	void RunAll ()
	{
		OpSchedule_t tQueue;
		{
			ScopedMutex_t tGuard ( m_tQueueGuard );
			if ( m_tQueue.Empty() )
				return;
			tQueue.Push ( m_tQueue );
		}
		while ( !tQueue.Empty() )
		{
			auto* pOp = tQueue.Front();
			tQueue.Pop();
			pOp->Complete ( pOp );
		}
	}

	bool IsEmpty() const NO_THREAD_SAFETY_ANALYSIS
	{
		return m_tQueue.Empty();
	}

	~Impl_c()
	{
		while ( !m_tQueue.Empty () )
		{
			auto * pOp = m_tQueue.Front ();
			m_tQueue.Pop ();
			pOp->Complete ( nullptr );
		}
	}
};

OperationsQueue_c::OperationsQueue_c()
	: m_pImpl ( new Impl_c )
{}

OperationsQueue_c::~OperationsQueue_c()
{
	SafeDelete ( m_pImpl );
}

void OperationsQueue_c::AddOp (Handler fnOp)
{
	assert ( m_pImpl );
	m_pImpl->AddOp(std::move(fnOp));
}

void OperationsQueue_c::RunAll()
{
	assert ( m_pImpl );
	m_pImpl->RunAll();
}

bool OperationsQueue_c::IsEmpty() const
{
	assert ( m_pImpl );
	return m_pImpl->IsEmpty();
}


namespace { // static

	class IterationHandler_c : public SchedulerOperation_t
	{
		ThreadIteratorFN m_Handler;

	public:
		explicit IterationHandler_c ( ThreadIteratorFN h )
				: SchedulerOperation_t ( &IterationHandler_c::DoComplete )
				  , m_Handler ( std::move ( h ) )
		{}

		static void DoComplete ( void * pOwner, SchedulerOperation_t * pBase )
		{
			auto * pHandler = (IterationHandler_c *) pBase;
			if ( pOwner )
				pHandler->m_Handler ( *(ThreadFN *) pOwner );
			else
				delete pHandler;
		}
	};

	struct IteratorsQueue_t
	{
		RwLock_t m_tQueueGuard;
		OpSchedule_t m_tQueue GUARDED_BY ( m_tQueueGuard );

		void RegisterIterator ( ThreadIteratorFN fnIterator )
		{
			auto pCb = ( new IterationHandler_c ( std::move ( fnIterator ) ) );
			ScWL_t tGuard ( m_tQueueGuard );
			m_tQueue.Push_front ( pCb );
		}

		// iterate over all (pooled and alone) threads.
		// over pooled we're not using locks, since pool is living 'as whole', so no lock accessing individual elem need.
		// iteration func, however, must check if param is nullptr.
		// note, non-iteratable threads can't use hazard pointers (just nobody knows they're hold something).
		void IterateActive ( ThreadFN fnHandler )
		{
			ScRL_t tGuard ( m_tQueueGuard );
			for ( auto & dOp : m_tQueue )
				dOp.Complete ( &fnHandler );
		}

		~IteratorsQueue_t()
		{
			ScWL_t tGuard ( m_tQueueGuard );
			while ( !m_tQueue.Empty () )
			{
				auto * pOp = m_tQueue.Front ();
				m_tQueue.Pop ();
				pOp->Complete ( nullptr );
			}
		}
	};

	IteratorsQueue_t & g_dIteratorsList ()
	{
		static IteratorsQueue_t dIteratorsList;
		return dIteratorsList;
	}
}

void Threads::RegisterIterator ( ThreadIteratorFN fnIterator )
{
	g_dIteratorsList ().RegisterIterator ( std::move ( fnIterator ) );
}

void Threads::IterateActive ( ThreadFN fnHandler )
{
	g_dIteratorsList ().IterateActive ( std::move ( fnHandler ) );
}

Threads::Scheduler_i * MakeSingleThreadExecutor ( int iMaxThreads, const char * szName )
{
	if ( iMaxThreads>0 && Threads::AloneThread_c::GetRunners ()>=iMaxThreads )
		return nullptr;

	static int iOrder = 0;
	return new Threads::AloneThread_c ( iOrder++, szName? szName: "alone" );
}

#if !_WIN32
void * Threads::Init ( bool bDetached )
#else
void * Threads::Init ( bool )
#endif
{
	static bool bInit = false;
#if !_WIN32
	static pthread_attr_t tJoinableAttr;
	static pthread_attr_t tDetachedAttr;
#endif

	if ( !bInit )
	{
#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
		sphMemStatInit();
#endif

#if !_WIN32
		if ( pthread_attr_init ( &tJoinableAttr ) )
			sphDie ( "FATAL: pthread_attr_init( joinable ) failed" );

		if ( pthread_attr_init ( &tDetachedAttr ) )
			sphDie ( "FATAL: pthread_attr_init( detached ) failed" );

		if ( pthread_attr_setdetachstate ( &tDetachedAttr, PTHREAD_CREATE_DETACHED ) )
			sphDie ( "FATAL: pthread_attr_setdetachstate( detached ) failed" );
#endif
		bInit = true;
	}
#if !_WIN32
	if ( pthread_attr_setstacksize ( &tJoinableAttr, STACK_SIZE ) )
		sphDie ( "FATAL: pthread_attr_setstacksize( joinable ) failed" );

	if ( pthread_attr_setstacksize ( &tDetachedAttr, STACK_SIZE ) )
		sphDie ( "FATAL: pthread_attr_setstacksize( detached ) failed" );

	return bDetached ? &tDetachedAttr : &tJoinableAttr;
#else
	return NULL;
#endif
}


#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
void Threads::Done ( int iFD )
{
	sphMemStatDump ( iFD );
	sphMemStatDone();
}
#else
void Threads::Done ( int )
{
}
#endif


/// get name of a thread
CSphString Threads::GetName ( const SphThread_t * pThread )
{
	if ( !pThread || !*pThread )
		return "";

#if HAVE_PTHREAD_GETNAME_NP
	char sClippedName[16];
	pthread_getname_np ( *pThread, sClippedName, 16 );
	return sClippedName;
#else
	return "";
#endif
}

/// my join thread wrapper
bool Threads::Join ( SphThread_t * pThread )
{
#if _WIN32
	DWORD uWait = WaitForSingleObject ( *pThread, INFINITE );
	CloseHandle ( *pThread );
	*pThread = NULL;
	return ( uWait==WAIT_OBJECT_0 || uWait==WAIT_ABANDONED );
#else
	return pthread_join ( *pThread, nullptr )==0;
#endif
}

/// my own thread
SphThread_t Threads::Self ()
{
#if _WIN32
	return GetCurrentThread();
#else
	return pthread_self ();
#endif
}

/// compares two thread ids
bool Threads::Same ( const LowThreadDesc_t * pFirst, const LowThreadDesc_t * pSecond )
{
	if ( !pFirst && !pSecond )
		return true;
	if ( !pFirst || !pSecond )
		return false;

#if _WIN32
	// can not use m_tThread on Windows as GetCurrentThread returns -2 and that handle valid only inside thread itself
	return ( pFirst->m_iThreadID==pSecond->m_iThreadID );
#else
	return pthread_equal ( pFirst->m_tThread, pSecond->m_tThread )!=0;
#endif
}

struct RuntimeThreadContext_t : ISphNoncopyable
{
	LowThreadDesc_t	m_tDesc;
	const void *	m_pMyThreadStack = nullptr;
	OpSchedule_t	m_pThreadCleanup;
	Handler			m_fnRun;

#if USE_GPROF
	pthread_mutex_t	m_dlock;
	pthread_cond_t	m_dwait;
	itimerval		m_ditimer;
#endif

#if SPH_ALLOCS_PROFILER
	void * m_pTLS = nullptr;
#endif

	// main thread execution func
	void Run ( const void * pStack );

	// prepare everything to make *this most robust
	void Prepare ( const void * pStack );

	// save name stored in desc as OS thread name
	void PropagateName ();
};

namespace { // static func
RuntimeThreadContext_t*& RuntimeThreadContext ()
{
	static RuntimeThreadContext_t tStubForMain;
	static thread_local RuntimeThreadContext_t* pLocalThread = &tStubForMain;
	return pLocalThread;
}
}

// to be used globally from thread env
RuntimeThreadContext_t& MyThreadContext()
{
	return *RuntimeThreadContext();
}

LowThreadDesc_t& Threads::MyThd ()
{
	return RuntimeThreadContext ()->m_tDesc;
}

void Threads::SetSysThreadName ()
{
	RuntimeThreadContext ()->PropagateName ();
}

void Threads::JobStarted ()
{
	auto& tDesc = Threads::MyThd ();
	tDesc.m_tmLastJobDoneTimeUS = -1;
	tDesc.m_tmLastJobStartTimeUS = sphMicroTimer ();
	tDesc.m_tmLastJobStartCPUTimeUS = sphCpuTimer ();
}

void Threads::JobFinished ( bool bIsDone )
{
	auto & tDesc = Threads::MyThd ();
	tDesc.m_tmLastJobDoneTimeUS = sphMicroTimer ();
	if ( bIsDone )
		++tDesc.m_iTotalJobsDone;
	tDesc.m_tmTotalWorkedTimeUS += tDesc.m_tmLastJobDoneTimeUS-tDesc.m_tmLastJobStartTimeUS;
	tDesc.m_tmTotalWorkedCPUTimeUS += sphCpuTimer()-tDesc.m_tmLastJobStartCPUTimeUS;
}

// Adds a function call (a new task for a wrapper) to a linked list
// of thread contexts. They will be executed one by one right after
// the main thread ends its execution. This is a way for a wrapper
// to free local resources allocated by its main thread.
void Threads::OnExitThread ( Threads::Handler fnHandle )
{
	auto pCb = ( new CompletionHandler_c<Handler> ( std::move(fnHandle) ) );
	MyThreadContext().m_pThreadCleanup.Push_front ( pCb );
}

const void * Threads::TopOfStack ()
{
	return MyThreadContext().m_pMyThreadStack;
}

void Threads::SetTopStack ( const void * pNewStack )
{
	MyThreadContext ().m_pMyThreadStack = pNewStack;
}

void Threads::SetMaxCoroStackSize ( int iStackSize )
{
	g_iMaxCoroStackSize = iStackSize;
}

void Threads::PrepareMainThread ( const void * PStack )
{
	MyThreadContext ().Prepare ( PStack );
}

void RuntimeThreadContext_t::PropagateName ()
{
	// set name of self
#if HAVE_PTHREAD_SETNAME_NP
	if ( !m_tDesc.m_sThreadName.IsEmpty() )
	{
		auto sSafeName = m_tDesc.m_sThreadName.SubString ( 0, 15 );
		assert ( sSafeName.cstr ()!=nullptr );
#if HAVE_PTHREAD_SETNAME_NP_1ARG
		pthread_setname_np ( sSafeName.cstr() );
#else
		pthread_setname_np ( m_tDesc.m_tThread, sSafeName.cstr() );
#endif
	}
#endif
}

void RuntimeThreadContext_t::Prepare ( const void * pStack )
{
	m_pMyThreadStack = pStack;
	m_tDesc.m_iThreadID = GetOsThreadId ();
	m_tDesc.m_tmStart = sphMicroTimer();
	m_tDesc.m_pTaskInfo.store ( nullptr, std::memory_order_release );
	m_tDesc.m_pHazards.store ( nullptr, std::memory_order_release );
	m_tDesc.m_tThread = Threads::Self ();

#if USE_GPROF
	// Set the profile timer value
	setitimer ( ITIMER_PROF, &m_ditimer, NULL );

	// Tell the calling thread that we don't need its data anymore
	pthread_mutex_lock ( &m_dlock );
	pthread_cond_signal ( &m_dwait );
	pthread_mutex_unlock ( &m_dlock );
#endif

	PropagateName ();
}

void RuntimeThreadContext_t::Run ( const void * pStack )
{
	RuntimeThreadContext () = this;
	Prepare ( pStack );

#if SPH_ALLOCS_PROFILER
	m_pTLS = sphMemStatThdInit();
#endif

	g_iRunningThreads.fetch_add ( 1, std::memory_order_acq_rel );
	LOG( DEBUG, MT ) << "thread created";
	m_fnRun();
	LOG( DEBUG, MT ) << "thread ended";
	g_iRunningThreads.fetch_sub ( 1, std::memory_order_acq_rel );

	while ( !m_pThreadCleanup.Empty () )
	{
		auto * pOp = m_pThreadCleanup.Front ();
		m_pThreadCleanup.Pop ();
		pOp->Complete ( pOp );
	}

#if SPH_ALLOCS_PROFILER
	sphMemStatThdCleanup ( m_pTLS );
#endif
}


#if _WIN32
DWORD __stdcall ThreadProcWrapper_fn ( void * pArg )
#else
void * ThreadProcWrapper_fn ( void * pArg )
#endif
{
	// This is the first local variable in the new thread. So, its address is the top of the stack.
	// We need to know thread stack size for both expression and query evaluating engines.
	// We store expressions as a linked tree of structs and execution is a calls of mutually
	// recursive methods. Before executing we compute tree height and multiply it by a constant
	// with experimentally measured value to check whether we have enough stack to execute current query.
	// The check is not ideal and do not work for all compilers and compiler settings.
	char	cTopOfMyStack;

	CSphScopedPtr<RuntimeThreadContext_t> pCtx { (RuntimeThreadContext_t *) pArg };
	pCtx->Run ( &cTopOfMyStack );

	return 0;
}

bool Threads::Create ( SphThread_t * pThread, Handler fnRun, bool bDetached, const char * sName, int iNum )
{
	// we can not put this on current stack because wrapper need to see
	// it all the time and it will destroy this data from heap by itself
	CSphScopedPtr<RuntimeThreadContext_t> pCtx { new RuntimeThreadContext_t };
	pCtx->m_fnRun = std::move ( fnRun );

	if ( sName )
	{
		if ( iNum<0 )
			pCtx->m_tDesc.m_sThreadName = sName;
		else
			pCtx->m_tDesc.m_sThreadName.SetSprintf ( "%s_%d", sName, iNum );
	}

	// create thread
#if _WIN32
	Threads::Init ( bDetached );
	*pThread = CreateThread ( NULL, STACK_SIZE, ThreadProcWrapper_fn, pCtx.Ptr(), 0, NULL );
	if ( *pThread )
	{
		pCtx.LeakPtr();
		return true;
	}
#else

#if USE_GPROF
	getitimer ( ITIMER_PROF, &pCtx->m_ditimer );
	pthread_cond_init ( &pCtx->m_dwait, NULL );
	pthread_mutex_init ( &pCtx->m_dlock, NULL );
	pthread_mutex_lock ( &pCtx->m_dlock );
#endif

	void * pAttr = Threads::Init ( bDetached );
	errno = pthread_create ( pThread, (pthread_attr_t*) pAttr, ThreadProcWrapper_fn, pCtx.Ptr() );

#if USE_GPROF
	if ( !errno )
		pthread_cond_wait ( &pCtx->m_dwait, &pCtx->m_dlock );

	pthread_mutex_unlock ( &pCtx->m_dlock );
	pthread_mutex_destroy ( &pCtx->m_dlock );
	pthread_cond_destroy ( &pCtx->m_dwait );
#endif

	if ( !errno )
	{
		pCtx.LeakPtr();
		return true;
	}

#endif // _WIN32

	// thread creation failed so we need to cleanup ourselves
	return false;
}

// Thread with crash query


namespace { // static func
CrashQuery_t** g_ppTlsCrashQuery ()
{
	static thread_local CrashQuery_t* pTlsCrashQuery = nullptr;
	return &pTlsCrashQuery;
}

void GlobalSetTopQueryTLS ( CrashQuery_t * pQuery )
{
	*g_ppTlsCrashQuery() = pQuery;
}

void GlobalCrashQuerySet ( const CrashQuery_t & tQuery )
{
	CrashQuery_t * pQuery = *g_ppTlsCrashQuery();
	assert ( pQuery );
	*pQuery = tQuery;
}
}

static CrashQuery_t g_tUnhandled;

CrashQuery_t & GlobalCrashQueryGetRef ()
{
	CrashQuery_t * pQuery = *g_ppTlsCrashQuery ();

	// in case TLS not set \ found handler still should process crash
	if ( pQuery )
		return *pQuery;

	sphWarning ("GlobalCrashQueryGetRef: thread-local info is not set! Use ad-hoc");
	return g_tUnhandled;
}

CrashQueryKeeper_c::CrashQueryKeeper_c ()
	: m_tReference ( GlobalCrashQueryGetRef() )
{}

CrashQueryKeeper_c::~CrashQueryKeeper_c ()
{
	RestoreCrashQuery();
}

void CrashQueryKeeper_c::RestoreCrashQuery () const
{
	GlobalCrashQuerySet ( m_tReference );
}


// create thread for query - it will have set CrashQuery to valid obj inside, alive during whole thread's live time.
bool Threads::CreateQ ( SphThread_t * pThread, Handler fnRun, bool bDetached, const char * sName, int iNum )
{
	return Create ( pThread, [fnCrashRun = std::move ( fnRun )]
	{
		CrashQuery_t tQueryTLS;
		GlobalSetTopQueryTLS ( &tQueryTLS );
		LOG( DEBUG, MT ) << "thread created";
		fnCrashRun();
		LOG( DEBUG, MT ) << "thread ended";
	}, bDetached, sName, iNum );
}

// capture crash query and set it before running fnHandler.
Threads::Handler Threads::WithCopiedCrashQuery ( Threads::Handler fnHandler )
{
	CrashQuery_t tParentCrashQuery = GlobalCrashQueryGetRef ();
	return [tCrashQuery = tParentCrashQuery, fnHandler = std::move ( fnHandler )] {
		// CrashQueryKeeper_c _; // restore previous crash query on exit. Seems, that is not necessary
		GlobalCrashQuerySet ( tCrashQuery );
		fnHandler ();
	};
}