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

/// @file searchdtask.h
/// Task manager

#pragma once

#include "sphinxutils.h"
#include "threadutils.h"

using TaskID = int;

namespace TaskManager {

	struct TaskInfo_t
	{
		CSphString m_sName;							// informational name (for logs, etc.)
		int m_iMaxRunners = 0;						// max num of threads running jobs from the same task, 0=unlimited
		std::atomic<int> m_iCurrentRunners { 0 };	// current number of running jobs
		std::atomic<int> m_iAllRunners { 0 };		// current number of enqueued+running jobs
		std::atomic<int> m_iTotalDropped { 0 };		// current number of jobs for this task.
		std::atomic<int64_t> m_iTotalSpent { 0 };	// total time spend to this job
		std::atomic<int64_t> m_iTotalRun { 0 };		// total N of times job invoked
		std::atomic<int64_t> m_iLastFinished { 0 }; // timestamp when last job finished
	};

	/*!
	 * @brief Register global task flavour.
	 * @param sName - informational name of the task, will be used for logging, etc.
	 * @param iThreads - max num of parallel jobs of the class (0=infinite)
	 * @return integer ID which has to be used to schedule/start jobs of that kind.
	 */
	TaskID RegisterGlobal ( CSphString sName, int iThreads = 0 );

	/*!
	 * @brief schedule job which will be engaged at given time (in microseconds). Scheduling will not run the
	 * job, but just schedule it, so it doesn't honour job's queue limit (however it will be honoured when
	 * the time to run the jub appears).
	 * @param iTask - kind of job (registered with RegisterGlobal)
	 * @param iTimestamp - time (absolute) when job need to be executed
	 * @param fnJob - task route
	 */
	void ScheduleJob ( TaskID iTask, int64_t iTimeStampUS, Threads::Handler fnJob );

	/*!
	 * @brief enqueue job which will be run ASAP. For MT jobs queue's limit will be honored, so exceeding job will
	 * be immediately dropped. For ST only if limit=0 job will be dropped, for other values it will be executed.
	 * @param iTask - kind of job (registered with RegisterGlobal)
	 * @param fnJob - task route
	 */
	void StartJob ( TaskID iTask, Threads::Handler fnJob );

	VecTraits_T<TaskInfo_t> GetTaskInfo ();
}
