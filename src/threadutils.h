//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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
};

enum class Proto_e
{
	UNKNOWN,
	SPHINX,
	MYSQL41,
	HTTP,
	REPLICATION,
	HTTPS
};

const char* TaskStateName ( TaskState_e eState );
const char* ProtoName ( Proto_e eProto );

int GetOsThreadId ();

namespace Threads {

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
struct JobTimer_t
{
	JobTimer_t () { JobStarted (); }
	~JobTimer_t () { JobFinished (); }
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
using Keeper_t = SharedPtrCustom_t<void*>;

struct Scheduler_i
{
	virtual ~Scheduler_i() {};
	virtual void Schedule ( Handler handler, bool bVip ) = 0;
	virtual void ScheduleContinuation ( Handler handler ) // if task already started
	{
		Schedule ( std::move ( handler ), false );
	}
	virtual Keeper_t KeepWorking() = 0;
	virtual int WorkingThreads() const = 0;
	virtual int Works () const = 0;
	virtual void StopAll () = 0;
	virtual void DiscardOnFork() {};
	virtual void IterateChildren ( ThreadFN & fnHandler ) {};
};

using SchedulerSharedPtr_t = SharedPtr_t<Scheduler_i *>;

// none of the functions below used in the code. Both maybe only in tests.
SchedulerSharedPtr_t MakeThreadPool ( size_t iThreadCount, const char * szName="" );
SchedulerSharedPtr_t MakeAloneThread ( size_t iOrderNum, const char * szName = "" );

/// stack of a thread (that is NOT stack of the coroutine!)
static const DWORD STACK_SIZE = 128 * 1024;

/// add (cleanup) callback to run on thread exit
void OnExitThread ( Handler fnHandle );

/// get the pointer to my thread's stack
const void * TopOfStack ();

/// limit size for my coro stacks
void SetMaxCoroStackSize ( int iStackSize );

/// store the address in the TLS
void MemorizeStack ( const void * PStack );

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
bool Same ( SphThread_t first, SphThread_t second );

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
#if !USE_WINDOWS
	void HandleCrash( int );
#else
	LONG WINAPI HandleCrash ( EXCEPTION_POINTERS * pExc );
#endif
	void SetupTimePID();
};

// Scheduler to global thread pool
Threads::Scheduler_i* GlobalWorkPool ();
void SetMaxChildrenThreads ( int iThreads );
void StartGlobalWorkPool ();

/// schedule stop of the global thread pool
void WipeGlobalSchedulerOnShutdownAndFork ();

void WipeSchedulerOnFork ( Threads::Scheduler_i * );

// Scheduler to dedicated thread (or nullptr, if current N of such threads >= iMaxThreads)
// you MUST schedule at least one job, or explicitly delete non-engaged scheduler (it will leak otherwise).
Threads::Scheduler_i* GetAloneScheduler ( int iMaxThreads, const char* szName=nullptr );

using SchedulerFabric_fn = std::function<Threads::Scheduler_i * ( void )>;

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
	void AloneShutdowncatch ();
}


#endif //MANTICORE_THREADUTILS_H
