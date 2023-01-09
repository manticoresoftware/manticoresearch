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
#include "taskoptimize.h"
#include "searchdtask.h"
#include "searchdaemon.h"

/////////////////////////////////////////////////////////////////////////////
// index optimization
/////////////////////////////////////////////////////////////////////////////
void RunOptimizeRtIndex ( OptimizeTask_t tTask )
{
	Threads::StartJob ( [tTask = std::move ( tTask )]() mutable
	{
		auto pServed = GetServed ( tTask.m_sIndex );
		if ( !pServed )
			return;

		// want to track optimize only at work
		auto pDesc = PublishSystemInfo ( "OPTIMIZE" );

		// FIXME: MVA update would wait w-lock here for a very long time
		assert ( pServed->m_eType == IndexType_e::RT );
		RIdx_T<RtIndex_i*> ( pServed )->Optimize ( std::move ( tTask ) );
	});
}

void ServeAutoOptimize()
{
	OptimizeExecutor() = &RunOptimizeRtIndex;
}
