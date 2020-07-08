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

#include "taskflushattrs.h"
#include "searchdtask.h"
#include "searchdaemon.h"
#include "coroutine.h"

//////////////////////////////////////////////////////////////////////////
struct FlushState_t
{
	CSphAutoEvent m_tFlushFinished;
	CSphAtomic m_iDemandEvents; // if worker need to set the event
	int64_t m_iLastCheckFinishedTime = 0;
	int m_iFlushTag = 0;        ///< last flushed tag
};

static FlushState_t g_tFlush;


enum class Saved_e {
	NOTHING,	// no dirty indexes found, nothing saved
	NOT_ALL,	// not all indexes saved (some failed)
	ALL,		// whole save successfully completed
};

static Saved_e CheckSaveIndexes ()
{
	CSphString sError;
	auto eSaveState = Saved_e::ALL;
	bool bDirty = false;
	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next (); )
	{
		ServedDescRPtr_c pServed ( it.Get ());
		if ( pServed && pServed->m_pIndex->GetAttributeStatus ())
		{
			bDirty = true;
			if ( !pServed->m_pIndex->SaveAttributes ( sError ))
			{
				sphWarning ( "index %s: attrs save failed: %s", it.GetName ().cstr (), sError.cstr ());
				eSaveState = Saved_e::NOT_ALL;
			}
		}
	}
	g_tFlush.m_iLastCheckFinishedTime = sphMicroTimer ();

	if ( !bDirty )
		return Saved_e::NOTHING;

	++g_tFlush.m_iFlushTag;
	return eSaveState;
}

/* About setting event g_tFlush.m_tFlushFinished
 * flushing attributes may be engaged in two ways: either by timer (scheduled), either by command 'flush attributes'.
 * First needs nothing - it just do the things and re-schedule itself.
 * Second needs event to trace end of operation.
 * So, there are 2 slightly different operations: 'just flush' and 'flush and signal'.
 * For this kind of task we may have at most 1 job in queue, (1 one currently running - already out of queue).
 * If saving job is in progress, you may schedule at most 1 more; others will be dropped.
 * If you issued 'flush attributes' and it is in progress, and then usual timeouted task happened and enqueued
 * the next saving right after one in work, any other try to call 'flush attributes' would be just dropped and then
 * deadlocked. But instead we increase the counter of awaited events. And when the task in queue begin to work
 * (despite the source - issued both by demand or by timeout), it memorize the number of awaiters and on finish fires
 * demanded num of events. So, if one flush is in work another is scheduled - you may run any num of extra 'flush
 * attributes' and this scheduled task will resolve all of them a time.
 */
static void SaveIndexesMT ( void* = nullptr )
{
	int iFireOnExit = g_tFlush.m_iDemandEvents;
	g_tFlush.m_iDemandEvents -= iFireOnExit;

	if ( iFireOnExit )
		sphLogDebug ( "attrflush: doing forced check for %d waiters", iFireOnExit );
	else
		sphLogDebug ( "attrflush: doing periodic check" );

	auto pDesc = PublishSystemInfo ( "SAVE indexes" );
	if ( CheckSaveIndexes ()==Saved_e::NOTHING )
		sphLogDebug ( "attrflush: no dirty indexes found" );

	for ( int i=0; i<iFireOnExit; ++i )
		g_tFlush.m_tFlushFinished.SetEvent ();

	ScheduleFlushAttrs ();
}

void EngageSaveIndexes ()
{
	static TaskID iSaveTask = -1;
	if ( iSaveTask<0 )
		iSaveTask = TaskManager::RegisterGlobal ( "Save indexes", SaveIndexesMT,nullptr,1,1);
	TaskManager::StartJob ( iSaveTask );
}

int CommandFlush () EXCLUDES ( MainThread )
{
	// force a check, and wait it until completes
	sphLogDebug ( "attrflush: forcing check, tag=%d", g_tFlush.m_iFlushTag );
	++g_tFlush.m_iDemandEvents;
	EngageSaveIndexes ();

	g_tFlush.m_tFlushFinished.WaitEvent ();
	sphLogDebug ( "attrflush: check finished, tag=%d", g_tFlush.m_iFlushTag );
	return g_tFlush.m_iFlushTag;
}


static int64_t g_iAttrFlushPeriodUs = 0;            // in useconds; 0 means "do not flush"
void SetAttrFlushPeriod ( int64_t iPeriod )
{
	g_iAttrFlushPeriodUs = iPeriod;
}

void ScheduleFlushAttrs ()
{
	if ( !g_iAttrFlushPeriodUs )
		return;

	static TaskID iScheduledSave = -1;
	if ( iScheduledSave<0 )
	{
		iScheduledSave = TaskManager::RegisterGlobal ( "Sheduled save indexes",
		[] (void*) // save task lambda
		{
			if ( (g_tFlush.m_iLastCheckFinishedTime + g_iAttrFlushPeriodUs - 1000 )>sphMicroTimer() )
				ScheduleFlushAttrs ();
			else
				SaveIndexesMT();
		}, nullptr);
	}

	if ( !g_tFlush.m_iLastCheckFinishedTime )
		g_tFlush.m_iLastCheckFinishedTime = sphMicroTimer ();

	TaskManager::ScheduleJob ( iScheduledSave, g_tFlush.m_iLastCheckFinishedTime + g_iAttrFlushPeriodUs);
}

// called from main shutdown and expects problem reporting
bool FinallySaveIndexes ()
{
	bool bRes;
	Threads::CallCoroutine ( [&bRes] { bRes = CheckSaveIndexes ()!=Saved_e::NOT_ALL; });
	return bRes;
}