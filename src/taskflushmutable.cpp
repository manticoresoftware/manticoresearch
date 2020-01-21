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
#include "taskflushmutable.h"
#include "searchdtask.h"
#include "searchdaemon.h"

/// flushing period, defined from `rt_flush_period` config param
static int64_t g_iRtFlushPeriod = 36000000000ll; // default period is 10 hours
void SetRtFlushPeriod ( int64_t iPeriod )
{
	g_iRtFlushPeriod = iPeriod;
}

// thread-safe stringset, internally guarded by rwlock
class StringSetMT
{
	RwLock_t m_dGuard;
	sph::StringSet m_dSet GUARDED_BY ( m_dGuard );
	bool m_dDisabled = false;

public:
	bool AddUniq ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		ScWL_t wLock ( m_dGuard );
		if ( m_dSet[sName] )
			return false;

		m_dSet.Add ( sName );
		return true;
	}

	void Delete ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		ScWL_t wLock ( m_dGuard );
		m_dSet.Delete ( sName );
	}

	bool Contains ( const CSphString& sName ) EXCLUDES ( m_dGuard )
	{
		if ( m_dDisabled )
			return false;
		ScRL_t rLock ( m_dGuard );
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

				ServedDescRPtr_c dRlocked ( pServed );
				if ( !ServedDesc_t::IsMutable ( dRlocked ))
				{
					g_Flushable.Delete ( sName );
					return;
				}

				auto* pRT = ( RtIndex_i* ) dRlocked->m_pIndex;
				if ( !pRT )
				{
					g_Flushable.Delete ( sName );
					return;
				}

				// check timeout, schedule or run immediately.
				auto iLastTimestamp = pRT->GetLastFlushTimestamp ();
				auto iPlannedTimestamp = iLastTimestamp+g_iRtFlushPeriod;
				bool bNeedFlush = pRT->IsFlushNeed();
				if ( bNeedFlush && ( iPlannedTimestamp-1000 )<=sphMicroTimer() )
				{
					pRT->ForceRamFlush ( true );
					iPlannedTimestamp = pRT->GetLastFlushTimestamp()+g_iRtFlushPeriod;
				}

				if ( !bNeedFlush )
					iPlannedTimestamp = sphMicroTimer() + g_iRtFlushPeriod;

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

void HookSubscribeMutableFlush ( ISphRefcountedMT* pCounter, const CSphString& sName )
{
	if ( !pCounter ) // skip added null (disabled) indexes
		return;

	SubscribeFlushIndex ( sName );
}

void ShutdownFlushingMutable ()
{
	g_Flushable.Disable();
}