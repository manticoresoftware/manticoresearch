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
#include "taskpreread.h"
#include "searchdtask.h"
#include "searchdaemon.h"

OneshotEvent_c g_tPrereadFinished;

static void PrereadFunc ( void* )
{
	auto pDesc = PublishSystemInfo ( "PREREAD" );

	int64_t tmStart = sphMicroTimer ();

	StrVec_t dIndexes;
	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next (); )
	{
		if ( it.Get ())
			dIndexes.Add ( it.GetName ());
	}

	sphInfo ( "prereading %d indexes", dIndexes.GetLength ());
	int iReaded = 0;

	for ( int i = 0; i<dIndexes.GetLength () && !sphInterrupted (); ++i )
	{
		const CSphString& sName = dIndexes[i];
		auto pServed = GetServed ( sName );
		if ( !pServed )
			continue;

		ServedDescRPtr_c dReadLock ( pServed );
		if ( dReadLock->m_eType==IndexType_e::TEMPLATE )
			continue;

		int64_t tmReading = sphMicroTimer ();

		sphLogDebug ( "prereading index '%s'", sName.cstr ());

		dReadLock->m_pIndex->Preread ();
		ServedDesc_t::UpdateMass ( dReadLock );
		if ( !dReadLock->m_pIndex->GetLastWarning ().IsEmpty ())
			sphWarning ( "'%s' preread: %s", sName.cstr (), dReadLock->m_pIndex->GetLastWarning ().cstr ());

		int64_t tmReaded = sphMicroTimer () - tmReading;
		sphLogDebug ( "prereaded index '%s' in %0.3f sec", sName.cstr (), float ( tmReaded ) / 1000000.0f );

		++iReaded;
	}

	int64_t tmFinished = sphMicroTimer () - tmStart;
	sphInfo ( "prereaded %d indexes in %0.3f sec", iReaded, float ( tmFinished ) / 1000000.0f );
	g_tPrereadFinished.SetEvent ();
}

bool WaitFinishPreread ( int64_t uSec )
{
	return g_tPrereadFinished.WaitEvent ( int ( uSec / 1000 ));
}

void DoPreread ()
{
	PrereadFunc ( nullptr );
}

void StartPreread ()
{
	static int iPreread = -1;
	if ( iPreread<0 )
		iPreread = TaskManager::RegisterGlobal ("Preread task", PrereadFunc, nullptr, 1);
	TaskManager::StartJob ( iPreread );
}