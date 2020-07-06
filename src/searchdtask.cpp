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

#include "searchdtask.h"
#include "searchdaemon.h"

#ifndef VERBOSE_TASKMANAGER
#define VERBOSE_TASKMANAGER 0
#endif

#if VERBOSE_TASKMANAGER
#define WarnL( LETTER, ... ) TimePrefixed::Warning ( #LETTER " ", __VA_ARGS__)
#define InfoL( LETTER, ... ) TimePrefixed::Info ( #LETTER " ", __VA_ARGS__)
#define FatalL( LETTER, ... ) TimePrefixed::LogFatal ( #LETTER " ", __VA_ARGS__)
#define DebugL( LETTER, ... ) TimePrefixed::LogDebug ( #LETTER " ", __VA_ARGS__)
#define DebugvL( LETTER, ... ) TimePrefixed::LogDebugv ( #LETTER " ", __VA_ARGS__)
#define DebugvvL( LETTER, ... ) TimePrefixed::LogDebugvv ( #LETTER " ", __VA_ARGS__)
#else
#if USE_WINDOWS
#pragma warning(disable:4390)
#endif
#define WarnL( ... )
#define InfoL( ... )
#define FatalL( ... )
#define DebugL( ... )
#define DebugvL( ... )
#define DebugvvL( ... )
#endif

#define InfoX( ... ) InfoL(X, __VA_ARGS__)

#define DebugT( ... ) DebugL(T, __VA_ARGS__)
#define DebugM( ... ) DebugL(M, __VA_ARGS__)
#define DebugX( ... ) DebugL(X, __VA_ARGS__)

// period of idle, after which workers will finish.
static int64_t IDLE_TIME_TO_FINISH = 600 * 1000000LL; // 10m

// Max num of task flavours (we allocate fixed vec of this size)
// since we have only 7 different tasks for now, pool of 32 slots seems to be enough
static const int NUM_TASKS = 32;

inline static bool operator< ( const EnqueuedTimeout_t& dLeft, const EnqueuedTimeout_t& dRight )
{
	return dLeft.m_iTimeoutTimeUS<dRight.m_iTimeoutTimeUS;
}

void TimeoutQueue_c::ShiftUp ( int iHole )
{
	if ( m_dQueue.IsEmpty ())
		return;
	int iParent = ( iHole - 1 ) / 2;
	// shift up if needed, so that worst (lesser) ones float to the top
	while ( iHole && *m_dQueue[iHole]<*m_dQueue[iParent] )
	{
		Swap ( m_dQueue[iHole], m_dQueue[iParent] );
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
		iHole = iParent;
		iParent = ( iHole - 1 ) / 2;
	}
	m_dQueue[iHole]->m_iTimeoutIdx = iHole;
};

void TimeoutQueue_c::ShiftDown ( int iHole )
{
	if ( m_dQueue.IsEmpty () || iHole==m_dQueue.GetLength ())
		return;
	auto iMinChild = iHole * 2 + 1;
	auto iUsed = m_dQueue.GetLength ();
	while ( iMinChild<iUsed )
	{
		// select smallest child
		if ( iMinChild + 1<iUsed && *m_dQueue[iMinChild + 1]<*m_dQueue[iMinChild] )
			++iMinChild;

		// if smallest child is less than entry, do float it to the top
		if ( *m_dQueue[iHole]<*m_dQueue[iMinChild] )
			break;

		Swap ( m_dQueue[iHole], m_dQueue[iMinChild] );
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
		iHole = iMinChild;
		iMinChild = iHole * 2 + 1;
	}
	m_dQueue[iHole]->m_iTimeoutIdx = iHole;
}

void TimeoutQueue_c::Push ( EnqueuedTimeout_t* pTask )
{
	if ( !pTask )
		return;

	m_dQueue.Add ( pTask );
	ShiftUp ( m_dQueue.GetLength () - 1 );
	m_dCloud.Add (( uintptr_t ) pTask );
	m_dCloud.Uniq ();
}

void TimeoutQueue_c::Pop ()
{
	if ( m_dQueue.IsEmpty ())
		return;

	m_dQueue[0]->m_iTimeoutIdx = -1;
	Verify ( m_dCloud.RemoveValueFromSorted (( uintptr_t ) m_dQueue[0] ));
	m_dQueue.RemoveFast ( 0 );
	ShiftDown ( 0 );
}

void TimeoutQueue_c::Change ( EnqueuedTimeout_t* pTask )
{
	if ( !pTask )
		return;

	auto iHole = pTask->m_iTimeoutIdx;
	if ( iHole<0 )
	{
		Push ( pTask );
		return;
	}

	if ( iHole && *m_dQueue[iHole]<*m_dQueue[( iHole - 1 ) / 2] )
		ShiftUp ( iHole );
	else
		ShiftDown ( iHole );
}

void TimeoutQueue_c::Remove ( EnqueuedTimeout_t* pTask )
{
	if ( !pTask )
		return;

	auto iHole = pTask->m_iTimeoutIdx;
	if ( iHole<0 || iHole>=m_dQueue.GetLength ())
		return;

	Verify ( m_dCloud.RemoveValueFromSorted (( uintptr_t ) pTask ));
	m_dQueue.RemoveFast ( iHole );
	if ( iHole<m_dQueue.GetLength ())
	{
		if ( iHole && *m_dQueue[iHole]<*m_dQueue[( iHole - 1 ) / 2] )
			ShiftUp ( iHole );
		else
			ShiftDown ( iHole );
	}
	pTask->m_iTimeoutIdx = -1;
}

EnqueuedTimeout_t* TimeoutQueue_c::Root () const
{
	if ( m_dQueue.IsEmpty ())
		return nullptr;
	return m_dQueue[0];
}

CSphString TimeoutQueue_c::DebugDump ( const char* sPrefix ) const
{
	StringBuilder_c tBuild;
	for ( auto* cTask : m_dQueue )
		tBuild.Appendf ( tBuild.IsEmpty () ? "%p(" INT64_FMT ")":", %p("	INT64_FMT ")", cTask, cTask->m_iTimeoutTimeUS);
	CSphString sRes;
	if ( !m_dQueue.IsEmpty ())
		sRes.SetSprintf ( "%s%d:%s", sPrefix, m_dQueue.GetLength (), tBuild.cstr ());
	else
		sRes.SetSprintf ( "%sHeap empty.", sPrefix );
	return sRes;
}

void TimeoutQueue_c::DebugDump ( const std::function<void ( EnqueuedTimeout_t* )>& fcb ) const
{
	for ( auto* cTask : m_dQueue )
		fcb ( cTask );
}

//////////////////////////////////////////////////////////////////////////
// Tasks (job classes)
//////////////////////////////////////////////////////////////////////////
struct TaskFlavour_t
{
	fnThread_t m_fnWorker = nullptr; // main worker function
	fnThread_t m_fnReleasePayload = nullptr; // called to release
	CSphString m_sName; // informational name (for logs, etc.)
	int m_iMaxRunners = 0; // max num of threads running jobs from the same task (0 mean only main thread)
	int m_iMaxQueueSize = 0; // max possible num of enqueued tasks of this type. (-1 means unlimited)
};

struct TaskProperties_t
{
	CSphAtomic m_iCurrentRunners; // current number of jobs for this task.
	CSphAtomicL m_iTotalSpent;
	CSphAtomicL m_iTotalRun;
	CSphAtomicL m_iTotalDropped;
	CSphAtomicL m_iLastFinished;
	CSphMutex m_dQueueLock;
	List_t m_dQueue GUARDED_BY ( m_dQueueLock ); // queue of jobs of my type
};

// Task class descriptor just stores pointers to function-checker and function-worker

#if (USE_WINDOWS && _MSC_VER<1910)
// visual studio 2015 causes C2719 error when tries compile 'contains' for fixed fector of TaskProperties_t
TaskFlavour_t g_Tasks [ NUM_TASKS ];
TaskProperties_t g_TaskProps [ NUM_TASKS ];
#else
CSphFixedVector<TaskFlavour_t> g_Tasks { NUM_TASKS };
CSphFixedVector<TaskProperties_t> g_TaskProps { NUM_TASKS };
#endif

static CSphAtomic g_iTasks;

// task of determined flavour with stored payload
class Task_t: public ISphRefcountedMT
{
	TaskID m_ID;
	void* m_pPayload = nullptr;

public:
	explicit Task_t ( TaskID iTask, void* pPayload )
		: m_ID ( iTask )
		, m_pPayload ( pPayload )
	{
		assert ( iTask<=g_iTasks && iTask>=0 );
		DebugT ( "Task_t(id=%d - %s) ctr", iTask, g_Tasks[iTask].m_sName.cstr ());
	}

	const TaskFlavour_t& Descr () const { return g_Tasks[m_ID]; }
	TaskProperties_t& Prop () const	{ return g_TaskProps[m_ID]; }
	bool IsMT() const { return g_Tasks[m_ID].m_iMaxRunners>0; }

	void Action ()
	{
		Threads::JobTimer_t dTrack;
		DebugT ( "Task_t %s Action (%p)", GetName (), m_pPayload );
		++Prop ().m_iCurrentRunners;
		auto itmStart = sphMicroTimer ();
		Descr ().m_fnWorker ( m_pPayload );
		m_pPayload = nullptr;
		auto itmEnd = sphMicroTimer ();
		Prop ().m_iLastFinished = itmEnd;
		Prop ().m_iTotalSpent += ( itmEnd - itmStart );
		++Prop ().m_iTotalRun;
		--Prop ().m_iCurrentRunners;
	}

	const char* GetName () const
	{
		return Descr().m_sName.cstr ();
	}

	void Abort ( bool bCount = true )
	{
		if ( bCount )
			++Prop ().m_iTotalDropped;
		if ( m_pPayload && Descr ().m_fnReleasePayload )
			Descr ().m_fnReleasePayload ( m_pPayload );
		m_pPayload = nullptr;
	}

protected:
	~Task_t () override
	{
		Abort(false);
	}
};

using TaskRefP_c = CSphRefcountedPtr<Task_t>;
using VectorTask_c = CSphVector<Task_t*>;

// task pool thread context
ThreadRole MtJobThread;
ThreadRole TaskThread;
class LazyJobs_c;
struct TaskWorker_t: public ListNode_t
{
	Threads::LowThreadDesc_t& m_tDesc = Threads::MyThd (); // not owned, not need to me, need for outside iterations.
	int64_t m_iTotalTicked = 0;			// total num of times I've waked up
	int m_iMyTaskFlavour = 0;			// round-robin of tasks' flavours
	int m_iMyThreadID = 0;				// my id in workers queue

	// dig all the queues and extract the task we can work with
	Task_t* ExtractJobToWork () REQUIRES ( MtJobThread )
	{
		Task_t* pTask = nullptr;
		auto iFlavour = m_iMyTaskFlavour - 1;
		ListedData_t* pLeaf = nullptr;
		for ( int i = 0; i<g_iTasks; ++i )
		{
			++iFlavour;
			iFlavour %= g_iTasks;

			// skip queues full of runners (and also non-mt, since they have 0 runners)
			if ( g_TaskProps[iFlavour].m_iCurrentRunners>=g_Tasks[iFlavour].m_iMaxRunners )
				continue;

			// look into the queue
			auto& dProp = g_TaskProps[iFlavour];

			// scope for QueueLock
			ScopedMutex_t tTaskLock ( dProp.m_dQueueLock );

			// this queue is ok, extract the task and return it.
			if ( dProp.m_dQueue.GetLength ())
			{
				pLeaf = ( ListedData_t* ) dProp.m_dQueue.Begin ();
				dProp.m_dQueue.Remove ( pLeaf );
				break;
			}
		}

		if ( pLeaf )
		{
			// round-robin flavours: next time we'll round from last success job kind.
			m_iMyTaskFlavour = iFlavour;
			pTask = ( Task_t* ) pLeaf->m_pData;
			SafeDelete ( pLeaf );
		}
		return pTask;
	}

	// do task's job
	void PerformTask ( Task_t* pTask ) REQUIRES (MtJobThread)
	{
		assert ( pTask );
		// we've extracted the task and going to work on it.
		pTask->Action ();
		DebugL (M, "%d Done %d jobs, spend " INT64_FMT "us",
				   m_iMyThreadID, ( int ) m_tDesc.m_iTotalJobsDone, m_tDesc.m_tmTotalWorkedTimeUS; );
	}

	// returns timeout after which we'll regarded idle too much
	int TimeToDeadMS ( int64_t iIdlePeriod )
	{
		auto iLastJob = (m_tDesc.m_tmLastJobDoneTimeUS>0) ? m_tDesc.m_tmLastJobDoneTimeUS : sphMicroTimer();
		auto mSecs = (( iLastJob + iIdlePeriod ) - sphMicroTimer ()) / 1000;
		return Max ( mSecs, 0 );
	}
};

// scheduled job - will live in timeout heap and then fire when time is out.
struct ScheduledJob_t: public EnqueuedTimeout_t
{
	TaskRefP_c m_pTask;
	ScheduledJob_t ( Task_t* pTask, int64_t iTimestampUS )
		: m_pTask { pTask }
	{
		SafeAddRef ( m_pTask );
		m_iTimeoutTimeUS = iTimestampUS;
	}
};

void IterateLazyThreads ( Threads::ThreadFN & fnHandler );

LazyJobs_c& LazyTasker ();
class LazyJobs_c: ISphNoncopyable
{
	// stuff to transfer (enqueue) tasks
	VectorTask_c  m_dInternalTasks; // internal queue where we add our tasks without mutex
	VectorTask_c* m_pEnqueuedTasks GUARDED_BY ( m_dActiveLock ) = nullptr; // ext. mt queue where we add tasks
	RwLock_t m_dActiveLock;
	TimeoutQueue_c m_dTimeouts;
	int64_t m_iNextTimeoutUS = 0;
	OneshotEvent_c m_tSignal;
	TaskID m_iScheduler = -1;
	Threads::LowThreadDesc_t* m_pSchedulerThread = nullptr;

	// thread pool
	List_t m_dWorkers GUARDED_BY ( m_dWorkersLock );
	int m_iNextThreadId = 0;
	RwLock_t m_dWorkersLock;
	std::atomic<long> m_iIdleWorkers;
	int64_t m_tmIdlePeriodUS = IDLE_TIME_TO_FINISH; // workers will be finished after idle of this time, in uS
	CSphAutoEvent m_tJobSignal;
	volatile bool m_bShutdown = false;
	int m_iMaxWorkers = 32;

private:
	/// Atomically pick m-t queue to single-thread internal queue.
	VectorTask_c* PopQueue () REQUIRES ( TaskThread ) EXCLUDES ( m_dActiveLock )
	{
		// atomically get current vec; put zero instead.
		VectorTask_c* pReadyQueue = nullptr;
		{
			ScWL_t tLock ( m_dActiveLock );
			pReadyQueue = m_pEnqueuedTasks;
			m_pEnqueuedTasks = nullptr;
		}
		return pReadyQueue;
	}

	static void ProcessOneTask ( Task_t* pTask ) REQUIRES ( TaskThread )
	{
		DebugT ( "ProcessOneTask(%p) -> %s", pTask, pTask->GetName () );
		pTask->Action();
		SafeRelease ( pTask );
	}

	/// take current internal and external queues, parse it and process changes.
	void ProcessEnqueuedTasks () REQUIRES ( TaskThread )
	{
		DebugT ( "ProcessEnqueuedTasks" );

		auto VARIABLE_IS_NOT_USED uStartLen = m_dInternalTasks.GetLength ();

		auto pExternalQueue = PopQueue ();
		if ( pExternalQueue )
			m_dInternalTasks.Append ( *pExternalQueue );
		SafeDelete ( pExternalQueue );

		auto VARIABLE_IS_NOT_USED uLastLen = m_dInternalTasks.GetLength ();

		if ( m_dInternalTasks.IsEmpty ())
		{
			DebugT ( "No tasks in queue" );
			return;
		}
		DebugT ( "starting processing %d internal events (had %d, with new external %d)",
					   m_dInternalTasks.GetLength (), uStartLen, uLastLen );

		for ( auto* pTask : m_dInternalTasks )
		{
			DebugT ( "Start processing task %p", pTask );
			ProcessOneTask ( pTask );
			DebugT ( "Finish processing task %p", pTask );
		}
		DebugT ( "All events processed" );
		m_dInternalTasks.Reset ();
	}

	/// main event loop run in separate thread.
	void EventLoop () REQUIRES ( TaskThread )
	{
		while ( true )
			if ( !EventTick ())
				break;
	}

	/// abandon and release all timeouted events.
	/// \return next active timeout (in uS), or -1 for infinite.
	bool HasTimeoutActions () REQUIRES ( TaskThread )
	{
		bool bHasTimeout = false;
		while ( !m_dTimeouts.IsEmpty ())
		{
			auto* pScheduledTask = ( ScheduledJob_t* ) m_dTimeouts.Root ();
			assert ( pScheduledTask->m_iTimeoutTimeUS>0 );

			m_iNextTimeoutUS = pScheduledTask->m_iTimeoutTimeUS - sphMicroTimer ();
			if ( m_iNextTimeoutUS>999 )
				return bHasTimeout;

			// timeout reached; have to do an action
			bHasTimeout = true;

			DebugT ( "timeout happens for %p task (%s)", pScheduledTask, pScheduledTask->m_pTask->GetName ());
			DebugT ( "%s", m_dTimeouts.DebugDump ( "heap:" ).cstr ());
			AddToLocalQueue ( pScheduledTask->m_pTask );

			DebugT ( "Oneshot task removed" );
			m_dTimeouts.Pop ();
			SafeDelete ( pScheduledTask );
		}
		m_iNextTimeoutUS = -1;  /// means 'infinite'
		return bHasTimeout;
	}

	/// abandon and release all events (on shutdown)
	void AbortScheduled () REQUIRES ( TaskThread )
	{
		DebugT ( "AbortScheduled()" );
		while ( !m_dTimeouts.IsEmpty ())
		{
			auto pScheduled = ( ScheduledJob_t* ) m_dTimeouts.Root ();
			m_dTimeouts.Pop ();
			pScheduled->m_pTask->Abort ();
			DebugT ( "Aborted task (%p -> %p)", pScheduled, pScheduled->m_pTask.Ptr() );
			SafeDelete ( pScheduled );
		}
		ProcessEnqueuedTasks();
	}

	/// one event cycle.
	/// \return false to stop event loop and exit.
	bool EventTick () REQUIRES ( TaskThread )
	{
		DebugT ( "---------------------------- EventTick()" );
		do
			ProcessEnqueuedTasks ();
		while ( HasTimeoutActions ());

		int iMsec = ( m_iNextTimeoutUS>999 ) ? m_iNextTimeoutUS / 1000 : -1;
		DebugT ( "calculated timeout is %d ms", iMsec );
		auto iStarted = sphMicroTimer ();
		bool VARIABLE_IS_NOT_USED bWasKicked = m_tSignal.WaitEvent ( iMsec );
		auto VARIABLE_IS_NOT_USED iWaited = sphMicroTimer () - iStarted;
		DebugT ( "waited %t, reason=%s", iWaited, bWasKicked ? "kicked": "timeout or error" );

		if ( sphInterrupted () || m_bShutdown )
		{
			AbortScheduled ();
			DebugT ( "EventTick() exit because of shutdown=%d", sphInterrupted () );
			return false;
		}
		return true;
	}

	bool RouteJob ( Task_t* pTask )
	{
		if ( pTask->IsMT ())
		{
			AddNewMTJob ( pTask );
			return true;
		}
		if ( !pTask->Descr ().m_iMaxQueueSize )
		{
			DebugT ( "AddToLocalQueue: dropped task because queue is disabled" );
			pTask->Abort ();
			return true;
		}
		return false;
	}

	void AddToQueue ( Task_t* pTask )
	{
		if ( RouteJob ( pTask ))
			return;

		pTask->AddRef();
		ScWL_t tLock ( m_dActiveLock );
		if ( !m_pEnqueuedTasks )
			m_pEnqueuedTasks = new VectorTask_c;
		m_pEnqueuedTasks->Add ( pTask );
		DebugL (X, "AddToQueue (external=%d)", m_pEnqueuedTasks ? m_pEnqueuedTasks->GetLength () + 1 : 1 );
	}

	void AddToLocalQueue ( Task_t* pTask ) REQUIRES ( TaskThread )
	{
		if ( RouteJob ( pTask ))
			return;

		pTask->AddRef ();
		DebugT ( "AddToLocalQueue, %d", m_dInternalTasks.GetLength () + 1 );
		m_dInternalTasks.Add ( pTask );
	}

	void ProcessSchedulingEnqueue ( ScheduledJob_t* pScheduled ) REQUIRES ( TaskThread )
	{
		DebugT ( "ProcessSchedulingEnqueue (%p)", pScheduled );

		if ( sphInterrupted () )
			SafeDelete ( pScheduled );

		if ( !pScheduled )
			return;
		m_dTimeouts.Change ( pScheduled );
	}

	void WorkerFunc () REQUIRES ( !TaskThread )
	{
		m_pSchedulerThread = &Threads::MyThd ();
		ScopedRole_c thLazy ( TaskThread );
		DebugT ( "LazyJobs_c::WorkerFunc started" );
		EventLoop ();
		m_pSchedulerThread = nullptr;
	}

private:
	// thread pool
	/// one job cycle. Returns false to stop cycling.
	bool JobTick ( TaskWorker_t& tWorker ) REQUIRES ( MtJobThread )
	{
		DebugM ( "------------------ JobTick() %d", tWorker.m_iMyThreadID );
		if ( m_bShutdown || sphInterrupted () )
		{
			KickJobPool (); // kick next one before dead
			return false;
		}
		++tWorker.m_iTotalTicked;

		bool bSignaled = m_tJobSignal.WaitEvent ( tWorker.TimeToDeadMS ( m_tmIdlePeriodUS ));
		if ( !bSignaled ) // idle timeout happened. Fixme! m.b. better way to determine idles need.
		{
			DebugM ( "%d finishes because idle period exceeded", tWorker.m_iMyThreadID );
			return false;
		}

		m_iIdleWorkers.fetch_sub ( 1, std::memory_order_relaxed );
		auto dFreeIdle = AtScopeExit ( [this] { m_iIdleWorkers.fetch_add ( 1, std::memory_order_relaxed ); } );

		while (true)
		{
			TaskRefP_c pTask { tWorker.ExtractJobToWork () };
			if ( !pTask )
				return true;
			DebugM ( "%d starting job", tWorker.m_iMyThreadID );
			tWorker.PerformTask ( pTask );
		}
	}

	/// main event loop run in separate thread.
	void JobLoop () REQUIRES ( MtJobThread )
	{
		auto pWorker = new TaskWorker_t;
		CSphScopedPtr<TaskWorker_t> pThreadWorkerContext { pWorker };

		pWorker->m_iMyThreadID = ++m_iNextThreadId;
		Threads::MyThd ().m_sThreadName.SetSprintf( "TaskW_%d", pWorker->m_iMyThreadID );
		Threads::SetSysThreadName();

		{
			ScWL_t _ { m_dWorkersLock };
			m_dWorkers.Add ( pWorker );
		}


		DebugM ( "JobLoop started for %d", pWorker->m_iMyThreadID );
		while ( true )
			if ( !JobTick ( *pWorker ) )
				break;

		m_iIdleWorkers.fetch_sub ( 1, std::memory_order_relaxed );
		ScWL_t _ { m_dWorkersLock };
		m_dWorkers.Remove ( pWorker );
	}

	// adds a worker, until limit exceeded.
	void AddWorker ()
	{
		{
			ScRL_t _ ( m_dWorkersLock );
			if ( m_dWorkers.GetLength ()>=m_iMaxWorkers )
				return;
		}

		SphThread_t tThd;
		if ( Threads::Create ( &tThd, TheadPoolWorker, true ))
			m_iIdleWorkers.fetch_add ( 1, std::memory_order_relaxed );
	}

	void FinishAllWorkers () NO_THREAD_SAFETY_ANALYSIS
	{
		while ( m_dWorkers.GetLength ())
		{
			KickJobPool ();
			sphSleepMsec ( 50 );
		}
	}

	static void TheadPoolWorker () REQUIRES (!MtJobThread)
	{
		DebugM ( "LazyJobs_c::TheadPoolWorker started" );
		ScopedRole_c thMtThread ( MtJobThread );
		LazyTasker ().JobLoop ( );
	}

	static void RemoveAllJobs ()
	{
		for ( int i = 0; i<g_iTasks; ++i )
		{
			// skip single-threaded tasks
			if ( !g_Tasks[i].m_iMaxRunners )
				continue;

			// look into the queue
			auto& dProp = g_TaskProps[i];
			{ // scope for QueueLock
				ScopedMutex_t tTaskLock ( dProp.m_dQueueLock );

				// queue is empty;
				while ( dProp.m_dQueue.GetLength ())
				{
					auto* pLeaf = ( ListedData_t* ) dProp.m_dQueue.Begin ();
					dProp.m_dQueue.Remove ( pLeaf );
					auto* pTask = ( Task_t* ) pLeaf->m_pData;
					SafeDelete ( pLeaf );
					if ( pTask )
						pTask->Abort ();
					SafeRelease ( pTask );
				}
			}
		}
	}

	/// Try add (enqueue) immediate task.
	/// If queue length is limited and job not fit, it will be aborted and dropped immediately.
	void AddNewMTJob ( Task_t* pJob )
	{
		assert ( pJob );
		assert ( pJob->IsMT());

		auto iMaxJobs = pJob->Descr ().m_iMaxQueueSize;
		auto& dProp = pJob->Prop ();
		{
			ScopedMutex_t tTaskLock ( dProp.m_dQueueLock );
			auto iQueueLen = dProp.m_dQueue.GetLength ();
			if ( iMaxJobs>-1 && iQueueLen>=iMaxJobs )
			{
				DebugX ( "AddNewMTJob %s(%p) dropped exceeded job.", pJob->GetName (), pJob );
				pJob->Abort();
				return;
			}

			SafeAddRef ( pJob );
			dProp.m_dQueue.Add ( new ListedData_t ( pJob ));
			DebugX ( "AddNewMTJob %s(%p) success (%d in queue)", pJob->GetName (), pJob, iQueueLen );
		}

		if ( !m_iIdleWorkers.load(std::memory_order_relaxed) && dProp.m_iCurrentRunners<pJob->Descr ().m_iMaxRunners )
			AddWorker ();
		KickJobPool ();
	}

	/// Kick the tasker
	void KickJobPool ()
	{
		DebugX ( "Tasker kicked" );
		m_tJobSignal.SetEvent ();
	}

public:
	LazyJobs_c ()
	{
		Threads::RegisterIterator ( IterateLazyThreads );

		SphThread_t tThd;
		Threads::Create ( &tThd, [this] { WorkerFunc(); }, true, "TaskSched" );
		m_iScheduler = TaskManager::RegisterGlobal ( "Scheduler",
			[] ( void* pScheduledJob ) REQUIRES ( TaskThread )
			{
				auto& tThis = LazyTasker ();
				DebugX ( "LazyJobs_c::SchedulerFunc" );
				tThis.ProcessSchedulingEnqueue (( ScheduledJob_t* ) pScheduledJob );
			},
			[] ( void* pScheduledJob ) {
				auto pJ = ( ScheduledJob_t* ) pScheduledJob;
				SafeDelete ( pJ );
			});
	}

	~LazyJobs_c ()
	{
		DebugX ( "~LazyJobs_c. Shutdown=%d", sphInterrupted () );
		Shutdown();
	}

	void Shutdown()
	{
		InfoX( "Shutdown" );
		m_bShutdown = true;
		Kick ();
		FinishAllWorkers ();
		RemoveAllJobs ();
	}

	/// Add (enqueue) immediate task.
	void EnqueueNewTask ( Task_t* pTask )
	{
		assert ( pTask );
		AddToQueue ( pTask );
		Kick ();
	}

	/// Add (enqueue) deffered task.
	void EngageTask ( Task_t* pTask, int64_t iTimestampUS )
	{
		assert ( pTask );
		assert ( iTimestampUS>0 );
		assert ( m_iScheduler>=0 );

		/// To avoid concurrency on accessing queue of timeouts, it is done in 2 stages:
		///  1. We create internal immediate scheduler task (from any thread)
		///  2. When fired, sheduler actually enqueues timeout, but already from TaskThread
		// SchedulerFunc is worker of m_iScheduler
		EnqueueNewTask ( TaskRefP_c { new Task_t ( m_iScheduler, new ScheduledJob_t ( pTask, iTimestampUS )) } );
	}

	/// Kick the tasker
	void Kick ()
	{
		DebugX ( "Tasker kicked" );
		m_tSignal.SetEvent ();
	}

public:
	//statictics

	CSphVector<TaskManager::TaskInfo_t> GetTaskInfo () const NO_THREAD_SAFETY_ANALYSIS
	{
		CSphVector<TaskManager::TaskInfo_t> dRes;
		for ( auto i = 0; i<g_iTasks; ++i )
		{
			auto& dInfo = dRes.Add();
			dInfo.m_sName = g_Tasks[i].m_sName;
			dInfo.m_iMaxRunners = g_Tasks[i].m_iMaxRunners;
			dInfo.m_iMaxQueueSize = g_Tasks[i].m_iMaxQueueSize;
			dInfo.m_iCurrentRunners = g_TaskProps[i].m_iCurrentRunners;
			dInfo.m_iTotalSpent = g_TaskProps[i].m_iTotalSpent;
			dInfo.m_iLastFinished = g_TaskProps[i].m_iLastFinished;
			dInfo.m_iTotalRun = g_TaskProps[i].m_iTotalRun;
			dInfo.m_iTotalDropped = g_TaskProps[i].m_iTotalDropped;
			dInfo.m_inQueue = g_TaskProps[i].m_dQueue.GetLength (); // non th-safe, ok.
		}
		return dRes;
	}

	CSphVector<TaskManager::ThreadInfo_t> GetThreadsInfo ()
	{
		CSphVector<TaskManager::ThreadInfo_t> dRes;
		ScRL_t _ ( m_dWorkersLock );
		for ( const auto& tNode : m_dWorkers )
		{
			auto* pThd = (TaskWorker_t *) &tNode;
			auto& dThdDesc = pThd->m_tDesc;
			auto& dInfo = dRes.Add ();
			dInfo.m_iMyStartTimestamp = dThdDesc.m_tmStart;
			dInfo.m_iLastJobStartTime = dThdDesc.m_tmLastJobStartTimeUS;
			dInfo.m_iLastJobDoneTime = dThdDesc.m_tmLastJobDoneTimeUS;
			dInfo.m_iTotalWorkedTime = dThdDesc.m_tmTotalWorkedTimeUS;
			dInfo.m_iTotalJobsDone = dThdDesc.m_iTotalJobsDone;
			dInfo.m_iTotalTicked = pThd->m_iTotalTicked;
			dInfo.m_iMyThreadID = pThd->m_iMyThreadID;
			dInfo.m_iMyOSThreadID = dThdDesc.m_iThreadID;

		}
		return dRes;
	}

	CSphVector<TaskManager::ScheduleInfo_t> GetSchedInfo () const
	{
		CSphVector<TaskManager::ScheduleInfo_t> dRes;
		m_dTimeouts.DebugDump ( [&dRes] ( EnqueuedTimeout_t* pMember )
			{
				auto& dInfo = dRes.Add ();
				auto* pSheduled = ( ScheduledJob_t* ) pMember;
				dInfo.m_iTimeoutStamp = pSheduled->m_iTimeoutTimeUS;
				dInfo.m_sTask = pSheduled->m_pTask->GetName ();
			});
		return dRes;
	}

	void IterateThreads ( Threads::ThreadFN& fnHandler )
	{
		{
			ScRL_t _ ( m_dWorkersLock );
			for ( const auto & tNode : m_dWorkers )
				fnHandler ( &( (TaskWorker_t *) &tNode )->m_tDesc );
		}

		ScRL_t _ { m_dActiveLock };
		fnHandler ( m_pSchedulerThread );
	}
};

//! Get static (singletone) instance of lazy poller
//! C++11 guarantees it to be mt-safe (magic-static).
LazyJobs_c& LazyTasker ()
{
	static LazyJobs_c dEvents;
	return dEvents;
}

void IterateLazyThreads ( Threads::ThreadFN & fnHandler )
{
	LazyTasker ().IterateThreads ( fnHandler );
}

TaskID TaskManager::RegisterGlobal( CSphString sName, fnThread_t fnThread, fnThread_t fnFree, int iThreads, int iJobs )
{
	auto iTaskID = TaskID( g_iTasks++ );
	if ( !iTaskID ) // this is first class; start log timering
		TimePrefixed::TimeStart();

	InfoX( "Task class for %s registered with id=%d, max %d parallel jobs", sName.cstr(), iTaskID, iThreads );
	auto& dTask = g_Tasks[iTaskID];
	dTask.m_iMaxRunners = iThreads;
	dTask.m_iMaxQueueSize = iJobs;
	dTask.m_fnWorker = std::move( fnThread );
	dTask.m_fnReleasePayload = std::move( fnFree );
	dTask.m_sName = std::move( sName );
	return iTaskID;
}

void TaskManager::ScheduleJob ( TaskID iTask, int64_t iTimestamp, void* pPayload )
{
	InfoX( "ScheduleJob (id=%d(%s), period=%t, cookie = %p)",
		iTask, g_Tasks[iTask].m_sName.cstr(), iTimestamp-sphMicroTimer (), pPayload );
	LazyTasker ().EngageTask ( TaskRefP_c { new Task_t ( iTask, pPayload ) }, iTimestamp );
}

void TaskManager::StartJob ( TaskID iTask,  void* pPayload )
{
	InfoX( "StartJob (id=%d(%s), cookie = %p)", iTask, g_Tasks[iTask].m_sName.cstr (), pPayload );
	LazyTasker ().EnqueueNewTask ( TaskRefP_c { new Task_t ( iTask, pPayload )} );
}

CSphVector<TaskManager::TaskInfo_t> TaskManager::GetTaskInfo ()
{
	return LazyTasker ().GetTaskInfo();
}

CSphVector<TaskManager::ThreadInfo_t> TaskManager::GetThreadsInfo ()
{
	return LazyTasker ().GetThreadsInfo ();
}

CSphVector<TaskManager::ScheduleInfo_t> TaskManager::GetSchedInfo ()
{
	return LazyTasker ().GetSchedInfo ();
}

void TaskManager::ShutDown ()
{
	LazyTasker ().Shutdown();
}