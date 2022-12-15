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

int64_t& FlushPeriodUs()
{
	/// config searchd.rt_flush_period
	static int64_t iRtFlushPeriodUs = DEFAULT_FLUSH_PERIOD; // default period is 10 hours
	return iRtFlushPeriodUs;
}

void SetRtFlushPeriod ( int64_t iPeriod )
{
	FlushPeriodUs() = iPeriod;
}

// thread-safe stringset, internally guarded by rwlock
class StringSetMT_c
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

namespace
{
StringSetMT_c& FlushSet()
{
	static StringSetMT_c hFlushSet;
	return hFlushSet;
}

void ScheduleFlushTask ( CSphString sName )
{
	static int iRtFlushTask = TaskManager::RegisterGlobal ( "Flush mutable table" );
	static auto iLastFlushFinishedTime = sphMicroTimer();

	TaskManager::ScheduleJob ( iRtFlushTask, iLastFlushFinishedTime + FlushPeriodUs(), [sName = std::move ( sName )]() mutable
	{
		if ( FlushSet().IsDisabled() || !FlushSet().Contains ( sName ) )
			return;

		auto pServed = GetServed ( sName );
		if ( !pServed || !ServedDesc_t::IsMutable ( pServed ) ) // index went out or not suitable
		{
			FlushSet().Delete ( sName );
			return;
		}

		RIdx_T<RtIndex_i*> pRT { pServed };
		assert ( pRT );

		// do the flush
		pRT->ForceRamFlush ( "periodic" );

		// once more check for disabled - since ForceRamFlush may be long
		if ( FlushSet().IsDisabled() )
			return;

		iLastFlushFinishedTime = sphMicroTimer();
		ScheduleFlushTask ( std::move (sName) );
	} );
}

void SubscribeFlushIndex ( CSphString sName )
{
	if ( FlushSet().IsDisabled ())
		return;

	if ( FlushSet().AddUniq ( sName ))
		ScheduleFlushTask ( std::move ( sName ) );
};
} // namespace

void HookSubscribeMutableFlush ( const CSphString& sName )
{
	SubscribeFlushIndex ( sName );
}

void ShutdownFlushingMutable ()
{
	FlushSet().Disable();
}
