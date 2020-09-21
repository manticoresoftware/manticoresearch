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
#include "taskoptimize.h"
#include "searchdtask.h"
#include "searchdaemon.h"

/////////////////////////////////////////////////////////////////////////////
// index optimization
/////////////////////////////////////////////////////////////////////////////
struct OptimizeTask_t
{
	int m_iFrom;
	int m_iTo;
	int m_iCutoff;
	CSphString m_sIndex;

	OptimizeTask_t (CSphString sIndex, int iCutoff, int iFrom, int iTo)
	: m_iFrom (iFrom)
	, m_iTo (iTo)
	, m_iCutoff (iCutoff)
	, m_sIndex {std::move (sIndex)}
	{}
};

void EnqueueForOptimize ( CSphString sIndex, int iCutoff, int iFrom, int iTo )
{
	static int iOptimizeTask = -1;
	if ( iOptimizeTask<0 )
		iOptimizeTask = TaskManager::RegisterGlobal ( "optimize",
			[] ( void* pPayload ) // worker
			{
				CSphScopedPtr<OptimizeTask_t> pTask { (OptimizeTask_t *) pPayload };
				auto pServed = GetServed ( pTask->m_sIndex );

				if ( !pServed )
					return;

				ServedDescRPtr_c dReadLocked ( pServed );
				if ( !dReadLocked->m_pIndex )
					return;

				// want to track optimize only at work
				auto pDesc = PublishSystemInfo ( "OPTIMIZE" );

				// FIXME: MVA update would wait w-lock here for a very long time
				assert ( dReadLocked->m_eType==IndexType_e::RT );
				static_cast<RtIndex_i*>( dReadLocked->m_pIndex )->Optimize (pTask->m_iCutoff, pTask->m_iFrom, pTask->m_iTo);
			},
			[] ( void* pPayload ) // releaser
			{
				CSphScopedPtr<OptimizeTask_t> pTask { (OptimizeTask_t *) pPayload };
			}, 1 );
	TaskManager::StartJob ( iOptimizeTask, new OptimizeTask_t (std::move(sIndex), iCutoff, iFrom, iTo) );
}