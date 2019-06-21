//
// Copyright (c) 2019, Manticore Software LTD (http://manticoresearch.com)
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
void EnqueueForOptimize ( CSphString sIndex )
{
	static int iOptimizeTask = -1;
	if ( iOptimizeTask<0 )
		iOptimizeTask = TaskManager::RegisterGlobal ( "optimize",
			[] ( void* pName ) // worker
			{
				CSphString sIndex;
				sIndex.Adopt (( char* ) pName );
				auto pServed = GetServed ( sIndex );

				if ( !pServed )
					return;

				ServedDescRPtr_c dReadLocked ( pServed );
				if ( !dReadLocked->m_pIndex )
					return;

				// want to track optimize only at work
				ThreadSystem_t tThdSystemDesc ( "OPTIMIZE" );

				// FIXME: MVA update would wait w-lock here for a very long time
				assert ( dReadLocked->m_eType==IndexType_e::RT );
				static_cast<RtIndex_i*>( dReadLocked->m_pIndex )->Optimize ();
			},
			[] ( void* pName ) // releaser
			{
				CSphString sName;
				sName.Adopt (( char* ) pName );
			}, 1 );
	TaskManager::StartJob ( iOptimizeTask, sIndex.Leak () );
}