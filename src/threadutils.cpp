//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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

#if !USE_WINDOWS
// UNIX-specific headers and calls
#include <sys/syscall.h>

// for thr_self()
#ifdef __FreeBSD__
#include <sys/thr.h>
#endif

#endif


static RwLock_t g_tThdLock;
static List_t g_dThd GUARDED_BY ( g_tThdLock );   ///< existing threads table
static const char* g_sSystemName = "SYSTEM";

using namespace Threads;

const char* ThdStateName ( ThdState_e eState )
{
	switch (eState)
	{
		case ThdState_e::HANDSHAKE: return "handshake";
		case ThdState_e::NET_READ: return "net_read";
		case ThdState_e::NET_WRITE: return "net_write";
		case ThdState_e::QUERY: return "query";
		case ThdState_e::NET_IDLE: return "net_idle";
	}
	return "unknown";
}

const char* ProtoName ( Proto_e eProto )
{
	switch ( eProto )
	{
		case Proto_e::SPHINX: return "sphinxapi";
		case Proto_e::MYSQL41: return "sphinxql";
		case Proto_e::HTTP: return "http";
		case Proto_e::HTTPS: return "https";
		case Proto_e::REPLICATION: return "replication";
	}
	return "unknown";
}

ServiceThread_t::~ServiceThread_t ()
{
	Join ();
}

bool ServiceThread_t::Create ( void (* fnThread) ( void* ), void* pArg, const char* sName )
{
	m_bCreated = sphThreadCreate ( &m_tThread, fnThread, pArg, false, sName );
	return m_bCreated;
}

void ServiceThread_t::Join ()
{
	if ( m_bCreated && sphGetShutdown ())
		sphThreadJoin ( &m_tThread );
	m_bCreated = false;
}

void ThdDesc_t::SetThreadInfo ( const char* sTemplate, ... )
{
	ScopedMutex_t dLock ( m_tQueryLock );
	m_sBuf.Clear();
	va_list ap;
	va_start ( ap, sTemplate );
	m_sBuf.vSprintf ( sTemplate, ap );
	va_end ( ap );
}

void ThdDesc_t::SetSearchQuery ( const CSphQuery* pQuery )
{
	ScopedMutex_t dLock (m_tQueryLock);
	m_pQuery = pQuery;
}

ThreadSystem_t::ThreadSystem_t ( const char* sName )
{
	m_tDesc.m_bSystem = true;
	m_tDesc.m_tmStart = sphMicroTimer ();
	m_tDesc.m_iTid = GetOsThreadId ();
	m_tDesc.SetThreadInfo ( "SYSTEM %s", sName );
	m_tDesc.m_sCommand = g_sSystemName;

	m_tDesc.AddToGlobal();
}

ThreadSystem_t::~ThreadSystem_t ()
{
	m_tDesc.RemoveFromGlobal ();
}

ThreadLocal_t::ThreadLocal_t ( const ThdDesc_t& tDesc )
{
	m_tDesc.m_iTid = GetOsThreadId ();
	m_tDesc.m_eProto = tDesc.m_eProto;
	m_tDesc.m_iClientSock = tDesc.m_iClientSock;
	m_tDesc.m_sClientName = tDesc.m_sClientName;
	m_tDesc.m_eThdState = tDesc.m_eThdState;
	m_tDesc.m_sCommand = tDesc.m_sCommand;
	m_tDesc.m_iConnID = tDesc.m_iConnID;
	m_tDesc.m_iCookie = tDesc.m_iCookie;

	m_tDesc.m_tmConnect = tDesc.m_tmConnect;
	m_tDesc.m_tmStart = tDesc.m_tmStart;

	m_tDesc.AddToGlobal ();
}

ThreadLocal_t::~ThreadLocal_t ()
{
	m_tDesc.RemoveFromGlobal ();
}

void Threads::ThdDesc_t::AddToGlobal () EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Add ( this );
}

void Threads::ThdDesc_t::RemoveFromGlobal () EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Remove ( this );
}

Threads::ThdPublicInfo_t Threads::ThdDesc_t::GetPublicInfo ()
{
	ThdPublicInfo_t dResult;
	ThdInfo_t& dSemiRes = dResult;
	dSemiRes = *( ThdInfo_t* ) this;

	dResult.m_sThName = GetThreadName ( &m_tThd );

	ScopedMutex_t dLock ( m_tQueryLock );
	if ( m_pQuery )
		dResult.m_pQuery = new CSphQuery ( *m_pQuery );
	dResult.m_sRequestDescription = m_sBuf.cstr();
	return dResult;
}

Threads::ThdPublicInfo_t::~ThdPublicInfo_t ()
{
	SafeDelete ( m_pQuery );
}

void Threads::ThdPublicInfo_t::Swap( ThdPublicInfo_t& rhs )
{
	::Swap ( (*(ThdInfo_t*)(this)), *(ThdInfo_t*)(&rhs));
	::Swap ( m_sThName, rhs.m_sThName );
	::Swap( m_pQuery, rhs.m_pQuery );
	::Swap( m_sRequestDescription, rhs.m_sRequestDescription );
}

Threads::ThdPublicInfo_t::ThdPublicInfo_t( ThdPublicInfo_t && rhs) noexcept
{
	Swap(rhs);
}

Threads::ThdPublicInfo_t& Threads::ThdPublicInfo_t::operator=( ThdPublicInfo_t && rhs ) noexcept
{
	Swap(rhs);
	return *this;
}

int Threads::ThreadsNum () EXCLUDES ( g_tThdLock )
{
	ScRL_t dThdLock ( g_tThdLock );
	return g_dThd.GetLength ();
}

void Threads::ThdState ( ThdState_e eState, ThdDesc_t& tThd )
{
	tThd.m_eThdState = eState;
	tThd.m_tmStart = sphMicroTimer ();
}


List_t& Threads::GetUnsafeUnlockedUnprotectedGlobalThreadList () NO_THREAD_SAFETY_ANALYSIS
{
	return g_dThd;
}

CSphSwapVector<ThdPublicInfo_t> Threads::GetGlobalThreadInfos ()
{
	ScRL_t dThdLock ( g_tThdLock );
	CSphSwapVector<ThdPublicInfo_t> dResult;
	dResult.Reserve ( g_dThd.GetLength ());
	for ( const ListNode_t* pIt = g_dThd.Begin (); pIt!=g_dThd.End (); pIt = pIt->m_pNext )
		dResult.Add ((( ThdDesc_t* ) pIt )->GetPublicInfo ());
	return dResult;
}


int GetOsThreadId ()
{
#if USE_WINDOWS
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

namespace {
	thread_local int iNumber = 0;
	thread_local const char* szName = "main";
}

namespace logdetail {
	const char* name() { return szName; }
	int number() { return iNumber; }
}

#define LOG_COMPONENT_MT logdetail::name() << "#" << logdetail::number() << ": "

// start joinable thread running functional object (m.b. lambda)
SphThread_t makeTinyThread ( Handler tHandler, int iNum=0, const char * sName="main" )
{
	// let's place it inside function to avoid any outside usage
	struct TinyThread_t
	{
		Handler m_tHandler;
		const char * m_szName;
		int m_iNumber;

		explicit TinyThread_t ( Handler tHandler, const char* sName, int iNumber )
			: m_tHandler { std::move ( tHandler ) }
			, m_szName (sName)
			, m_iNumber (iNumber)
		{ }

		static void Do ( void * pTr )
		{
			auto * pThis = (TinyThread_t *) pTr;
			szName = pThis->m_szName;
			iNumber = pThis->m_iNumber;
			LOG( DEBUG, MT ) << "thread created";

			if ( pThis->m_tHandler )
				pThis->m_tHandler ();
			LOG( DEBUG, MT ) << "thread ended";
			delete pThis;
		}
	};

	StringBuilder_c sThdName;
	sThdName.Sprintf ( "%s_%d", sName, iNum );
	SphThread_t tThd;
	CSphScopedPtr<TinyThread_t> pHandler { new TinyThread_t ( std::move ( tHandler ), sName, iNum) };
	if ( sphThreadCreate ( &tThd, TinyThread_t::Do, pHandler.Ptr (), false, sThdName.cstr ()) )
		pHandler.LeakPtr ();
	return tThd;
}

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
			m_pFront = m_pFront->m_pNext;
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
	~SchedulerOperation_t () {}
};

// actual operation which completes given Handler
template <typename Handler>
class CompletionHandler_c : public SchedulerOperation_t
{
	Handler m_Handler;

public:
	CompletionHandler_c ( const Handler& h )
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
			dLocalHandler();

			// barrier ensures that no operations till here would be reordered below.
			std::atomic_thread_fence ( std::memory_order_release );
		}
	}
};


struct TaskServiceThreadInfo_t
{
	OpQueue_T <SchedulerOperation_t> m_dPrivateQueue;
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

/// performs tasks pushed with post() in one or many threads until they done.
/// Naming convention of members is inherited from boost::asio as drop-in replacement.
struct Service_t : public TaskService_t, public Service_i
{
	std::atomic<long> m_iOutstandingWork {0};	/// count of unfinished work
	mutable CSphMutex m_dMutex;					/// protect access to internal data
	bool m_bStopped = false;                	/// dispatcher has been stopped.
	bool m_bOneThread;                			/// optimize for singhe-threaded use case
	sph::Event_c m_tWakeupEvent;				/// event to wake up blocked threads
	OpQueue_T<SchedulerOperation_t> m_OpQueue;	/// The queue of handlers that are ready to be delivered

	// Per-thread call stack to track the state of each thread in the service.
	using ThreadCallStack_c = CallStack_c<TaskService_t, TaskServiceThreadInfo_t>;

	class Work_c;			/// Scoped RAII work to keep service running. calls work_started/work_finished
	friend class Work_c;

public:

	Service_t (bool bOneThread)
	: m_bOneThread ( bOneThread ) {}

	template<typename Handler>
	void post ( const Handler& handler )
	{
		// Allocate and construct an operation to wrap the handler.
		post_immediate_completion ( new CompletionHandler_c<Handler> ( handler ) );
	}

	void post_immediate_completion ( Service_t::operation * pOp )
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
		m_OpQueue.Push ( pOp );
		wake_one_thread_and_unlock ( dLock );
	}

	void run () override
	{
		LOG ( DETAIL, MT ) << "run " << m_iOutstandingWork << " st:" << !!m_bStopped;
		if ( m_iOutstandingWork==0 )
		{
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

	bool do_run_one ( ScopedMutex_t& dLock, TaskServiceThreadInfo_t& this_thread )
		RELEASE ( dLock ) TRY_ACQUIRE ( false, dLock )
	{
		while ( !m_bStopped )
		{
			LOG ( DETAIL, MT ) << "locked " << dLock.Locked();
			assert ( dLock.Locked ());
			if ( m_OpQueue.Empty () )
			{
				m_tWakeupEvent.Clear ( dLock );
				m_tWakeupEvent.Wait ( dLock );
				continue;
			}

			auto * pOp = m_OpQueue.Front ();
			m_OpQueue.Pop ();
			bool bMoreHandlers = !m_OpQueue.Empty ();

			if ( bMoreHandlers && !m_bOneThread )
				wake_one_thread_and_unlock ( dLock );
			else
				dLock.Unlock ();

			pOp->Complete (this);

			LOG ( DETAIL, MT ) << "completed & unlocked";
			if ( this_thread.m_iPrivateOutstandingWork>1 )
				m_iOutstandingWork += this_thread.m_iPrivateOutstandingWork-1;
			else if ( this_thread.m_iPrivateOutstandingWork<1 )
				work_finished ();

			this_thread.m_iPrivateOutstandingWork = 0;
			if ( !this_thread.m_dPrivateQueue.Empty ())
			{
				dLock.Lock ();
				m_OpQueue.Push ( this_thread.m_dPrivateQueue );
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
	}

	// Notify that some work has finished.
	void work_finished ()
	{
		LOG ( DETAIL, MT ) << "work_finished " << m_iOutstandingWork;
		if ( --m_iOutstandingWork==0 )
			stop ();
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

	Service_t& GetService() const
	{
		return m_tServiceRef;
	}
};

#define LOG_COMPONENT_TP LOG_COMPONENT_MT << "@" << this->szName() << ": "

struct ThreadPool_c::Impl_t
{
	Impl_t ( size_t iThreadCount, const char * szName )
		: m_szName {szName}
		, m_tService ( iThreadCount==1 )
	{
		createWork ();
		m_dThreads.Resize ( iThreadCount );
		for ( size_t i = 0; i<iThreadCount; ++i )
			m_dThreads[i] = makeTinyThread ( [this] { loop (); }, i, szName );
		LOG ( DEBUG, TP ) << "thread pool created with threads: " << iThreadCount;
	}

	~Impl_t ()
	{
		ScopedMutex_t dLock {m_dMutex};
		m_bStop = true;
		m_dWork.reset ();
		dLock.Unlock ();
		LOG ( DEBUG, TP ) << "stopping thread pool";
		for ( auto& dThread : m_dThreads )
			sphThreadJoin ( &dThread );
		LOG ( DEBUG, TP ) << "thread pool stopped";
	}

	void Wait ()
	{
		LOG ( DETAIL, TP ) << "Wait";
		ScopedMutex_t dLock {m_dMutex};
		m_dWork.reset ();
		while (true)
		{
			m_dCond.Clear ( dLock );
			m_dCond.Wait ( dLock );
			LOG ( DEBUG, TP ) << "WAIT: waitCompleted: " << !!m_dWork;
			if ( m_dWork )
				break;
		}
		LOG ( DETAIL, TP ) << "Wait finished";
	}

	template<typename F>
	void Post ( F && f )
	{
		LOG ( DETAIL, TP ) << "Post";
		m_tService.post ( std::forward<F> ( f ));
		LOG ( DETAIL, TP ) << "Post finished";
	}

	const char * szName () const
	{
		return m_szName;
	}

	Service_i & Service ()
	{
		return m_tService;
	}

private:
	void createWork ()
	{
		m_dWork.emplace ( m_tService );
	}

	void loop ()
	{
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
	}

	using Work = Service_t::Work_c;

	const char *	m_szName = nullptr;
	Service_t		m_tService;
	Optional_T<Work> m_dWork;
	CSphVector<SphThread_t> m_dThreads;
	CSphMutex 		m_dMutex;
	sph::Event_c	m_dCond;
	std::atomic<bool> m_bStop { false };
};

ThreadPool_c::ThreadPool_c ( size_t iThreadCount, const char * szName )
{
	m_pImpl = new Impl_t ( iThreadCount, szName );
}


ThreadPool_c::~ThreadPool_c ()
{
	SafeDelete ( m_pImpl );
}

void ThreadPool_c::Schedule ( Handler handler )
{
	m_pImpl->Post ( std::move ( handler ));
}

void ThreadPool_c::Wait ()
{
	m_pImpl->Wait ();
}

const char * ThreadPool_c::szName () const
{
	return m_pImpl->szName ();
}

} // namespace Threads



struct Handler_t : public ListNode_t
{
	Handler_fn m_fnCb;

	Handler_t ( Handler_fn && fnCb ) : m_fnCb ( std::move ( fnCb )) {}
};

static RwLock_t dShutdownGuard;
static List_t dShutdownList GUARDED_BY ( dShutdownGuard );

void * searchd::AddShutdownCb ( std::function<void ()> fnCb )
{
	auto * pCb = new Handler_t ( std::move ( fnCb ));
	ScWL_t tGuard ( dShutdownGuard );
	dShutdownList.Add ( pCb );
	return pCb;
}

// remove previously set shutdown cb by cookie
void searchd::DeleteShutdownCb ( void * pCookie )
{
	if ( !pCookie )
		return;

	auto * pCb = (Handler_t *) pCookie;

	ScWL_t tGuard ( dShutdownGuard );
	if ( !dShutdownList.GetLength ())
		return;

	dShutdownList.Remove ( pCb );
	SafeDelete ( pCb );
}

// invoke shutdown handlers
void searchd::FireShutdownCbs ()
{
	ScRL_t tGuard ( dShutdownGuard );
	while (dShutdownList.GetLength ()) {
		auto * pCb = (Handler_t *) dShutdownList.Begin ();
		dShutdownList.Remove ( pCb );
		pCb->m_fnCb ();
		SafeDelete( pCb );
	}
}


void * pGlobalThreadPoolDeleteCookie = nullptr;
Optional_T<Threads::ThreadPool_c> dPool;

void InitGlobalThreadPool ()
{
	int iThreads = sphGetNonZeroDistThreads ();
	//	iThreads = 1; // for debug purposes
	dPool.emplace ( iThreads, "rtsearch" );
	pGlobalThreadPoolDeleteCookie = searchd::AddShutdownCb ( [] { dPool.reset (); } );
}

void GlobalSchedule ( Threads::Handler dHandler )
{
	if ( !dPool )
		InitGlobalThreadPool ();
	dPool->Schedule ( std::move ( dHandler ));
}