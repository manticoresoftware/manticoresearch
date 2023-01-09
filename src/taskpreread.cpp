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
#include "taskpreread.h"
#include "searchdtask.h"
#include "searchdaemon.h"

namespace {
OneshotEvent_c g_tPrereadFinished; // invoked from main thread, so use raw (not coro) event.

void DoPreread ()
{
	auto pDesc = PublishSystemInfo ( "PREREAD" );

	int64_t tmStart = sphMicroTimer ();

	StrVec_t dIndexes;
	ServedSnap_t hLocals = g_pLocalIndexes->GetHash();
	for ( auto& tIt : *hLocals )
	{
		if ( tIt.second )
			dIndexes.Add ( tIt.first );
	}

	sphInfo ( "prereading %d tables", dIndexes.GetLength ());
	int iRead = 0;

	for ( const CSphString& sName : dIndexes )
	{
		if ( sphInterrupted() )
			break;

		auto pServed = GetServed ( sName );
		if ( !pServed || pServed->m_eType==IndexType_e::TEMPLATE )
			continue;

		int64_t tmReading = sphMicroTimer ();

		sphLogDebug ( "prereading table '%s'", sName.cstr ());

		RWIdx_c pIdx {pServed};
		pIdx->Preread ();
		pServed->UpdateMass();
		if ( !pIdx->GetLastWarning ().IsEmpty ())
			sphWarning ( "'%s' preread: %s", sName.cstr (), pIdx->GetLastWarning ().cstr ());

		int64_t tmRead = sphMicroTimer () - tmReading;
		sphLogDebug ( "preread table '%s' in %0.3f sec", sName.cstr (), float ( tmRead ) / 1000000.0f );

		++iRead;
	}

	int64_t tmFinished = sphMicroTimer () - tmStart;
	sphInfo ( "preread %d tables in %0.3f sec", iRead, float ( tmFinished ) / 1000000.0f );
	g_tPrereadFinished.SetEvent ();
}
} // namespace

bool WaitPrereadFinished ( int64_t uSec )
{
	return g_tPrereadFinished.WaitEvent ( int ( uSec / 1000 ));
}

void PrereadIndexes ( bool bForce )
{
	if ( bForce )
		return Threads::CallCoroutine ( DoPreread );

	Threads::StartJob ( DoPreread );
}