//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
struct OptimizeJob_t
{
	CSphString m_sIndex;
	OptimizeTask_t m_tTask;

	OptimizeJob_t ( CSphString sIndex, OptimizeTask_t tTask )
		: m_sIndex { std::move ( sIndex ) }
		, m_tTask { std::move ( tTask ) }
	{}
};

void EnqueueForOptimize ( CSphString sIndex, OptimizeTask_t tTask )
{
	static int iOptimizeTask = -1;
	if ( iOptimizeTask<0 )
		iOptimizeTask = TaskManager::RegisterGlobal ( "optimize",
			[] ( void* pPayload ) // worker
			{
				CSphScopedPtr<OptimizeJob_t> pJob { (OptimizeJob_t *) pPayload };
				auto pServed = GetServed ( pJob->m_sIndex );

				if ( !pServed )
					return;

				ServedDescRPtr_c dReadLocked ( pServed );
				if ( !dReadLocked->m_pIndex )
					return;

				// want to track optimize only at work
				auto pDesc = PublishSystemInfo ( "OPTIMIZE" );

				// FIXME: MVA update would wait w-lock here for a very long time
				assert ( dReadLocked->m_eType==IndexType_e::RT );
				static_cast<RtIndex_i*> ( dReadLocked->m_pIndex )->Optimize ( std::move ( pJob->m_tTask ) );
			},
			[] ( void* pPayload ) // releaser
			{
				CSphScopedPtr<OptimizeJob_t> pTask { (OptimizeJob_t *) pPayload };
			}, 1 );
	TaskManager::StartJob ( iOptimizeTask, new OptimizeJob_t ( std::move ( sIndex ), std::move ( tTask ) ) );
}

void ServeAutoOptimize()
{
	EnqueueForOptimizeExecutor() = &EnqueueForOptimize;
}