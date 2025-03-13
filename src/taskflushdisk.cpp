//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "taskflushdisk.h"

using namespace Threads;

static int g_iFlushWrite = 1LL;
static int g_iFlushSearch = 30LL;

int GetRtFlushDiskWrite ( bool bTestMode )
{
	return bTestMode ? -1 : g_iFlushWrite;
}

int GetRtFlushDiskSearch ()
{
	return g_iFlushSearch;
}

void SetRtFlushDiskPeriod ( int iFlushWrite, int iFlushSearch )
{
	g_iFlushWrite = iFlushWrite;
	g_iFlushSearch = iFlushSearch;
}

static void RtFlushDisk();

void ScheduleRtFlushDisk()
{
	static TaskID iScheduled = TaskManager::RegisterGlobal ( "Scheduled RT flush disk", 1 );
	static auto iLastCheckFinishedTime = sphMicroTimer();

	// 1 second is the minumum schedule interval
	// even global interval disable there could be RT indexes with local setting set
	TaskManager::ScheduleJob ( iScheduled, iLastCheckFinishedTime + 1LL * 1000000, []
	{
		RtFlushDisk();
		iLastCheckFinishedTime = sphMicroTimer();
		ScheduleRtFlushDisk();
	});
}

void RtFlushDisk()
{
	auto pDesc = PublishSystemInfo ( "RT flush disk" );

	ServedSnap_t hLocals = g_pLocalIndexes->GetHash();

	for ( auto & tIt : *hLocals )
	{
		if ( sphInterrupted() )
			break;

		assert ( tIt.second );
		if ( tIt.second->m_eType!=IndexType_e::RT )
			continue;

		RIdx_T<RtIndex_i *> pIdx { tIt.second };
		pIdx->ForceDiskChunk ( g_iFlushWrite, g_iFlushSearch );
	}
}
