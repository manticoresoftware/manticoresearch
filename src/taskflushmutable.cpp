//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
#include "taskflushmutable.h"
#include "searchdtask.h"
#include "searchdaemon.h"

using namespace Threads;

/// flushing period, defined from `rt_flush_period` config param
static int64_t g_iRtFlushPeriodUs = 36000000000ll; // default period is 10 hours
void SetRtFlushPeriod ( int64_t iPeriod )
{
	g_iRtFlushPeriodUs = iPeriod;
}

// thread-safe stringset, internally guarded by rwlock
class StringSetMT
{
	Threads::Coro::RWLock_c m_dGuard;
	sph::StringSet m_dSet GUARDED_BY ( m_dGuard );
	volatile bool m_dDisabled = false;

public:
	bool AddUniq ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		SccWL_t wLock ( m_dGuard );
		return m_dSet.Add ( sName );
	}

	void Delete ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		SccWL_t wLock ( m_dGuard );
		m_dSet.Delete ( sName );
	}

	bool Contains ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		if ( m_dDisabled )
			return false;
		SccRL_t rLock ( m_dGuard );
		return m_dSet[sName];
	}

	void Disable()
	{
		m_dDisabled = true;
	}

	bool IsDisabled() const
	{
		return m_dDisabled;
	}

};

static StringSetMT g_Flushable;

static void ScheduleFlushTask ( void* pName, int64_t iNextTimestamp=-1 )
{
	static int iRtFlushTask = -1;
	if ( iRtFlushTask<0 )
		iRtFlushTask = TaskManager::RegisterGlobal ( "Flush mutable index",
			[] ( void* pName ) // worker
			{
				CSphString sName;
				sName.Adopt (( char* ) pName );

				if ( g_Flushable.IsDisabled ())
					return;

				if ( !g_Flushable.Contains ( sName ))
					return;

				auto pServed = GetServed ( sName );
				if ( !pServed ) // index went out.
				{
					g_Flushable.Delete ( sName );
					return;
				}

				if ( !ServedDesc_t::IsMutable ( pServed ))
				{
					g_Flushable.Delete ( sName );
					return;
				}

				RIdx_T<RtIndex_i*> pRT { pServed };
				assert ( pRT );

				// check timeout, schedule or run immediately.
				auto iLastTimestamp = pRT->GetLastFlushTimestamp ();
				auto iPlannedTimestamp = iLastTimestamp+g_iRtFlushPeriodUs;
				bool bNeedFlush = pRT->IsFlushNeed();
				if ( bNeedFlush && ( iPlannedTimestamp-1000 )<=sphMicroTimer() )
				{
					pRT->ForceRamFlush ( "periodic" );
					iPlannedTimestamp = pRT->GetLastFlushTimestamp()+g_iRtFlushPeriodUs;
				}

				if ( !bNeedFlush )
					iPlannedTimestamp = sphMicroTimer() + g_iRtFlushPeriodUs;

				// once more check for disabled - since ForceRamFlush may be long
				if ( g_Flushable.IsDisabled ())
					return;

				// reschedule or post-schedule
				ScheduleFlushTask ( sName.Leak(), iPlannedTimestamp );
			},
			[] ( void* pName ) // deleter
			{
				CSphString sFoo;
				sFoo.Adopt (( char* ) pName );
			},
		2 );

	if ( iNextTimestamp<0 )
		TaskManager::StartJob ( iRtFlushTask, pName );
	else
		TaskManager::ScheduleJob ( iRtFlushTask, iNextTimestamp, pName );
}

static void SubscribeFlushIndex ( CSphString sName )
{
	if ( g_Flushable.IsDisabled ())
		return;

	if ( g_Flushable.AddUniq ( sName ))
		ScheduleFlushTask ( sName.Leak ());
};

void HookSubscribeMutableFlush ( const CSphString& sName )
{
	SubscribeFlushIndex ( sName );
}

void ShutdownFlushingMutable ()
{
	g_Flushable.Disable();
}
