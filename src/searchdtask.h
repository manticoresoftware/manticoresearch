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

/// @file searchdtask.h
/// Task manager

#pragma once
#include "sphinxutils.h"

/// member type for priority queue used for timeout task managing
struct EnqueuedTimeout_t
{
	int64_t m_iTimeoutTimeUS = -1;    // active timeout (used for bin heap morph in comparing)
	mutable int m_iTimeoutIdx = -1;    // idx inside timeouts bin heap (or -1 if not there), internal
};

/// priority queue for timeouts - as CSphQueue,
/// but specific (can resize, stores internal index in an object)
class TimeoutQueue_c
{
	CSphTightVector<EnqueuedTimeout_t*> m_dQueue;
	CSphTightVector <uintptr_t> m_dCloud;

	void ShiftUp ( int iHole );
	void ShiftDown ( int iHole );
	void Push ( EnqueuedTimeout_t* pTask );

public:
	/// remove root (ie. top priority) entry
	void Pop ();

	/// add new, or change already added entry
	void Change ( EnqueuedTimeout_t* pTask );

	/// erase elem (uses stored m_iTimeoutIdx)
	void Remove ( EnqueuedTimeout_t* pTask );

	inline bool IsEmpty () const
	{
		return m_dQueue.IsEmpty ();
	}

	inline bool IsNotHere ( const EnqueuedTimeout_t* pTask ) const
	{
		return !m_dCloud.BinarySearch (( uintptr_t ) pTask );
	}

	/// get minimal (root) elem
	EnqueuedTimeout_t* Root () const;

	CSphString DebugDump ( const char* sPrefix ) const;
	void DebugDump ( const std::function<void ( EnqueuedTimeout_t* )>& fcb ) const;
};

using fnThread_t = std::function<void ( void* )>;
using TaskID = int;

namespace TaskManager {

	struct TaskInfo_t
	{
		CSphString m_sName;
		int m_iMaxRunners; // max num of threads running jobs from the same task (0 mean only main thread)
		int m_iMaxQueueSize; // max possible num of enqueued tasks of this type. (0 means unlimited)
		int m_iCurrentRunners; // current number of jobs for this task.
		int64_t m_iTotalSpent;
		int64_t m_iLastFinished;
		int64_t m_iTotalRun;
		int64_t m_iTotalDropped;
		int m_inQueue;
	};

	struct ThreadInfo_t
	{
		int64_t m_iMyStartTimestamp;
		int64_t m_iLastJobStartTime;
		int64_t m_iLastJobDoneTime;
		int64_t m_iTotalWorkedTime;
		int64_t m_iTotalJobsDone;
		int64_t m_iTotalTicked;
		int m_iMyThreadID;
		int m_iMyOSThreadID;
	};

	struct ScheduleInfo_t
	{
		int64_t m_iTimeoutStamp;
		CSphString 	m_sTask;
	};

	/*!
	 * @brief Register global task flavour.
	 * @param sName - informational name of the task, will be used for logging, etc.
	 * @param fnThread - worker wich will perform the job
	 * @param fnFree - deleter for payload, will be called INSTEAD of worker if job has to be aborted
	 * @param iThreads - max num of parallel jobs of the class. 0 (default) is special for lightweight jobs
	 * running directly in scheduler's thread.
	 * @param iJobs - max num of deffered MT jobs of such class. Jobs are pushed into dedicated queue, and the
	 * param limits queue's size. Workers then pulls the jobs and abandon the queue. If for the moment queue size
	 * limit is reached, the job will be dropped. 0 disables the queue (so, no jobs of this class
	 * may be pushed in the case), it works both for single-threaded and MT jobs. -1 means 'infinite'. All
	 * other numbers determine concrete limit.
	 * @return integer ID which has to be used to schedule/start jobs of that kind.
	 */
	TaskID RegisterGlobal ( CSphString sName, fnThread_t fnThread, fnThread_t fnFree, int iThreads = 0,
		int iJobs = -1 );

	/*!
	 * @brief schedule job which will be engaged at given time (in microseconds). Scheduling will not run the
	 * job, but just schedule it, so it doesn't honour job's queue limit (however it will be honoured when
	 * the time to run the jub appears).
	 * @param iTask - kind of job (registered with RegisterGlobal)
	 * @param iTimestamp - time (absolute) when job need to be executed
	 * @param pPayload - optional param.
	 */
	void ScheduleJob ( TaskID iTask, int64_t iTimestamp, void* pPayload=nullptr );

	/*!
	 * @brief enqueue job which will be run ASAP. For MT jobs queue's limit will be honored, so exceeding job will
	 * be immediately dropped. For ST only if limit=0 job will be dropped, for other values it will be executed.
	 * @param iTask - kind of job (registered with RegisterGlobal)
	 * @param pPayload - optional param.
	 */
	void StartJob ( TaskID iTask, void* pPayload = nullptr );

	/// causes finishing the tasker
	void ShutDown();

	CSphVector<TaskInfo_t> GetTaskInfo ();
	CSphVector<ThreadInfo_t> GetThreadsInfo ();
	CSphVector<ScheduleInfo_t> GetSchedInfo ();
}
