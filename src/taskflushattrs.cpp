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

#include "taskflushattrs.h"
#include "searchdtask.h"
#include "searchdaemon.h"
#include "coroutine.h"

//////////////////////////////////////////////////////////////////////////
struct FlushState_t
{
	CSphAutoEvent m_tFlushFinished;
	std::atomic<int> m_iDemandEvents {0}; // if worker need to set the event
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
	ServedSnap_t hLocals = g_pLocalIndexes->GetHash();
	for ( auto& tIt : *hLocals )
	{
		assert ( tIt.second );
		RIdx_c pIdx { tIt.second };
		if ( pIdx->GetAttributeStatus() )
		{
			bDirty = true;
			if ( !pIdx->SaveAttributes ( sError ) )
			{
				sphWarning ( "table %s: attrs save failed: %s", tIt.first.cstr(), sError.cstr() );
				eSaveState = Saved_e::NOT_ALL;
			}
		}

	}

	if ( !bDirty )
		return Saved_e::NOTHING;

	++g_tFlush.m_iFlushTag;
	return eSaveState;
}

/* About setting event g_tFlush.m_tFlushFinished
 * flushing attributes may be engaged in two ways: either by timer (scheduled), either by command 'flush attributes'.
 * First needs nothing - it just does the things and re-schedule itself.
 * Second needs event to trace end of operation.
 * So, there are 2 slightly different operations: 'just flush' and 'flush and signal'.
 * For this kind of task we may have at most 1 running job.
 * If you issued 'flush attributes' and it is already in progress, it will be dropped.
 * We increase the counter of awaited events. And when the task finishes, it check N of awaited and fire demanded N of events.
 */
namespace {
void SaveIndexesMT ()
{
	sphLogDebug ( "attrflush: doing the check" );

	auto pDesc = PublishSystemInfo ( "SAVE tables" );
	if ( CheckSaveIndexes ()==Saved_e::NOTHING )
		sphLogDebug ( "attrflush: no dirty tables found" );

	int iFireOnExit = g_tFlush.m_iDemandEvents.exchange ( 0 );
	for ( int i=0; i<iFireOnExit; ++i )
		g_tFlush.m_tFlushFinished.SetEvent ();
}
}


int CommandFlush () EXCLUDES ( MainThread )
{
	// force a check, and wait it until completes
	sphLogDebug ( "attrflush: forcing check, tag=%d", g_tFlush.m_iFlushTag );
	g_tFlush.m_iDemandEvents.fetch_add ( 1, std::memory_order_relaxed );
	Threads::StartJob ( SaveIndexesMT );
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

	static TaskID iScheduledSave = TaskManager::RegisterGlobal ( "Scheduled save tables", 1 );
	static auto iLastCheckFinishedTime = sphMicroTimer ();

	TaskManager::ScheduleJob ( iScheduledSave, iLastCheckFinishedTime + g_iAttrFlushPeriodUs, []
	{
		SaveIndexesMT();
		iLastCheckFinishedTime = sphMicroTimer();
		ScheduleFlushAttrs();
	});
}

// called from main shutdown and expects problem reporting
bool FinallySaveIndexes ()
{
	return Threads::CallCoroutineRes ( [] { return CheckSaveIndexes ()!=Saved_e::NOT_ALL; });
}
