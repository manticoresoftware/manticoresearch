//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
/// @file threadutils.h
/// Different thread-related things (moved here from searchd.cpp)

#ifndef MANTICORE_THREADUTILS_H
#define MANTICORE_THREADUTILS_H

#include "sphinxstd.h"
#include "sphinxint.h"
#include "hazard_pointer.h"

struct CSphQuery;

enum class TaskState_e
{
	UNKNOWN,
	HANDSHAKE,
	NET_READ,
	NET_WRITE,
	QUERY,
	NET_IDLE,
	RETIRED,
};

enum class Proto_e
{
	UNKNOWN,
	SPHINX,
	MYSQL41,
	HTTP,
	REPLICATION,
	HTTPS,
	SPHINXSE,
};

const char* TaskStateName ( TaskState_e eState );
const char* ProtoName ( Proto_e eProto );
const char* RelaxedProtoName ( Proto_e eProto );

int GetOsThreadId ();
int GetOsProcessId();

/// my thread handle and thread func magic
#if _WIN32
using SphThread_t = HANDLE;
#else
using SphThread_t = pthread_t;
#endif

namespace Threads {
namespace details { class SchedulerOperation_t; }

// most basic struct with details interested not only to thread itself, but also observable from outside
// Describes just 'execution'. Used as is from main to handle detached threads on shutdown
// each thread own such structure by design, ref from inside m.b. accessed by Threads::MyThd()
struct LowThreadDesc_t
{
	SphThread_t			m_tThread;		///< m.b. used to send signals to the thread
	int					m_iThreadID;	///< OS thread id
	int64_t				m_tmStart;		///< when did the current thread started? Initialized from thread itself
	int64_t 			m_tmLastJobStartTimeUS = -1; ///< last time where I've started something useful
	int64_t 			m_tmLastJobStartCPUTimeUS = -1; ///< last time where I've started something useful
	int64_t				m_tmLastJobDoneTimeUS = -1;	///< last time where I've done something useful
	int64_t				m_tmTotalWorkedTimeUS = 0;	///< total time I've worked on useful tasks
	int64_t				m_tmTotalWorkedCPUTimeUS = 0;    ///< total time I've worked on useful tasks
	int64_t				m_iTotalJobsDone = 0;		///< total jobs I've completed
	CSphString			m_sThreadName;
	std::atomic<void *>	m_pTaskInfo;	///< what kind of task I'm doing now (nullptr - idle, i.e. nothing)
	std::atomic<void *> m_pHazards;		///< my hazard pointers
};

// thread-local description available globaly from any thread
LowThreadDesc_t& MyThd ();

// save name from my local LowThreadDesc into OS thread name
void SetSysThreadName();

using ThreadFN = std::function<void ( Threads::LowThreadDesc_t * )>;
using ThreadIteratorFN = std::function<void ( ThreadFN& )>;

// apply handler to all running threads
void IterateActive ( ThreadFN fnHandler );

// add function which will be invoked from IterateActive
void RegisterIterator ( ThreadIteratorFN fnIterator );

int GetNumOfRunning ();

// track the time
void JobStarted();
void JobFinished( bool bIsDone=true );

// RAII time tracker
struct JobTracker_t
{
	JobTracker_t () { JobStarted (); }
	~JobTracker_t () { JobFinished (); }
};

struct IdleTimer_t
{
	IdleTimer_t () { JobFinished (false); }
	~IdleTimer_t () { JobStarted (); }
};

//////////////////////////////////////////////////////////////////////////
/// functional threadpool with minimum footprint
using Handler = std::function<void ()>;
using Predicate = std::function<bool ()>;
using Handlers = std::initializer_list<Handler>;

// used to RAII keep Scheduler running (when work finished - it is usually destroyed)
using Keeper_t = SharedPtrCustom_t<void>;

struct Scheduler_i
{
	virtual ~Scheduler_i() = default;

	virtual void ScheduleOp ( Threads::details::SchedulerOperation_t* pOp, bool bVip ) = 0;
	virtual void ScheduleContinuationOp ( Threads::details::SchedulerOperation_t* pOp ) // if task already started
	{
		ScheduleOp ( pOp, false );
	}
	// RAII keeper of scheduler (when it exists, scheduler will not finish). That is necessary, say, if the only work is
	// paused and moved somewhere (for example, as cb in epoll polling). Without keeper scheduler then finish and it will
	// be impossible to resume it later.
	virtual Keeper_t KeepWorking() = 0;

	// number of parallel contextes might be necessary when working via this scheduler
	// alone and strand obviously has 1 worker; threadpool has many.
	virtual int WorkingThreads() const
	{
		return 1;
	};
	virtual const char* Name() const
	{
		return "unnamed_sched";
	}

	template<typename HANDLER>
	void Schedule ( HANDLER handler, bool bVip );

	template<typename HANDLER>
	void ScheduleContinuation ( HANDLER handler );
};

struct SchedulerWithBackend_i: public Scheduler_i
{
	virtual bool SetBackend ( Scheduler_i* pBackend ) = 0;
};

struct Worker_i: public Scheduler_i
{
	virtual int Works() const = 0;
	virtual void StopAll () = 0;
	virtual void DiscardOnFork() {}
	virtual void IterateChildren ( ThreadFN & fnHandler ) {}
};

using SchedulerSharedPtr_t = SharedPtr_t<Scheduler_i>;
using WorkerSharedPtr_t = SharedPtr_t<Worker_i>;

// none of the functions below used in the code. Both maybe only in tests.
WorkerSharedPtr_t MakeThreadPool ( size_t iThreadCount, const char* szName = "" );
WorkerSharedPtr_t MakeAloneThread ( size_t iOrderNum, const char* szName = "" );

// Alone scheduler works on top of another scheduler and provides sequental execution of the tasks (each time only one
// task may be performed, no concurrent execution). It also gives FIFO ordering of the tasks.
SchedulerSharedPtr_t MakeAloneScheduler ( Scheduler_i* pBase, const char* szName = nullptr );

SchedulerSharedPtr_t WrapRawScheduler ( Scheduler_i* pBase, const char* szName = nullptr );

class OperationsQueue_c
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	OperationsQueue_c();
	~OperationsQueue_c();

	void AddOp (Handler fnOp);
	void RunAll();
	bool IsEmpty() const;
};

/// stack of a thread (that is NOT stack of the coroutine!)
static const DWORD STACK_SIZE = 128 * 1024;

/// get the pointer to my thread's stack
const void * TopOfStack ();

/// limit size for my coro stacks
void SetMaxCoroStackSize ( int iStackSize );
int GetMaxCoroStackSize ();

/// store the address in the TLS and prepare whole thread (name, hazards, etc.)
void PrepareMainThread ( const void * PStack );

/// owerwrite top ot stack pointer (m.b. need for mocking)
void SetTopStack ( const void * pNewStack );

/// my threading initialize routine
void * Init ( bool bDetached = false );

/// my threading deinitialize routine
void Done ( int iFD );

/// get name of a thread
CSphString GetName ( const SphThread_t * pThread );

/// my join thread wrapper
bool Join ( SphThread_t * pThread );

/// my own thread
SphThread_t Self ();

/// compares two thread ids
bool Same ( const LowThreadDesc_t * pFirst, const LowThreadDesc_t * pSecond );

/// my create thread wrapper
/// for threads serving clients use CreateQ instead
bool Create ( SphThread_t * pThread, Handler fnRun, bool bDetached = false, const char * sName = nullptr, int iNum=-1 );

/// create thread and run thread func having global CrashQuery_t initialized
bool CreateQ ( SphThread_t * pThread, Handler fnRun, bool bDetached = false, const char * sName = nullptr, int iNum=-1 );

/// place copy of current crash query into fnHandler context
Handler WithCopiedCrashQuery ( Handler fnHandler );

} // namespace Threads

extern ThreadRole MainThread;


namespace CrashLogger
{
#if !_WIN32
	void HandleCrash( int );
#else
	LONG WINAPI HandleCrash ( EXCEPTION_POINTERS * pExc );
#endif
	void SetupTimePID();
};

// Scheduler to global thread pool
Threads::Worker_i* GlobalWorkPool ();
void SetMaxChildrenThreads ( int iThreads );
void StartGlobalWorkPool ();

/// schedule stop of the global thread pool
void WipeGlobalSchedulerOnShutdownAndFork ();

void WipeSchedulerOnFork ( Threads::Worker_i * );

// Scheduler to dedicated thread (or nullptr, if current N of such threads >= iMaxThreads)
// you MUST schedule at least one job, or explicitly delete non-engaged scheduler (it will leak otherwise).
Threads::Scheduler_i* MakeSingleThreadExecutor ( int iMaxThreads, const char* szName = nullptr );

// add handler which will be called on daemon's shutdown right after
// sphInterrupted() is set to true. Returns cookie for refer the callback in future.
namespace searchd
{
	// add global (process-wide) callback to be called on shutdown.
	// That is stack, i.e. LIFO sequence.
	void AddShutdownCb ( Threads::Handler fnCb );

	// add global cb to be called on fork. LIFO
	void AddOnForkCleanupCb ( Threads::Handler fnCb );

	// execute shutdown handlers
	void FireShutdownCbs ();

	// Shutdown cb chain also forked, but don't need to be executed on fork
	void CleanAfterFork();
}

// add and remove unjoined threads to global tracer which will
// send them SIGTERM on shutdown
namespace Detached
{
	void AddThread ( Threads::LowThreadDesc_t* pThread );

	void RemoveThread ( Threads::LowThreadDesc_t* pThread );

	// schedule global watchdog to gracefully join alone threads on exit
	void MakeAloneIteratorAvailable ();

	void ShutdownAllAlones();
}

#include "threadutils_impl.h"

#endif //MANTICORE_THREADUTILS_H
