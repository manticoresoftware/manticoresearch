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
#include "taskglobalidf.h"
#include "searchdtask.h"
#include "searchdaemon.h"
#include "global_idf.h"

// logf() is not there sometimes (eg. Solaris 9)
#if !_WIN32 && !HAVE_LOGF
static inline float logf ( float v )
{
	return (float) log ( v );
}
#endif

void RotateGlobalIdf ()
{
	Threads::StartJob ( []
	{
		static Threads::Coro::Mutex_c tSerializer;
		Threads::Coro::ScopedMutex_t tLock { tSerializer };

		CSphVector<CSphString> dFiles;
		ServedSnap_t hLocals = g_pLocalIndexes->GetHash();
		for ( auto& tIt : *hLocals )
		{
			auto pIndex = tIt.second;
			if ( pIndex && !pIndex->m_sGlobalIDFPath.IsEmpty() )
				dFiles.Add ( pIndex->m_sGlobalIDFPath );
		}

		auto pDesc = PublishSystemInfo ( "ROTATE global IDF" );
		sph::UpdateGlobalIDFs ( dFiles );
	});
}
