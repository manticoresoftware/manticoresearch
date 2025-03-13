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

#include "taskmalloctrim.h"

#if HAVE_MALLOC_TRIM

#include <malloc.h>
#include "searchdtask.h"

int PerformMallocTrim ( size_t iPad )
{
	return malloc_trim ( iPad );
}

void ScheduleMallocTrim ()
{
	static int iMallocTrimTask = TaskManager::RegisterGlobal ( "malloc_trim(0) periodically", 1 );
	static int64_t iLastMallocTrimTimestamp = sphMicroTimer();

	TaskManager::ScheduleJob ( iMallocTrimTask, iLastMallocTrimTimestamp + DEFAULT_MALLOC_TRIM_PERIOD, []
	{
		PerformMallocTrim ( 0 );
		iLastMallocTrimTimestamp = sphMicroTimer();
		ScheduleMallocTrim ();
	} );
}

#else
int PerformMallocTrim ( size_t ) {return 0;};
void ScheduleMallocTrim () {};
#endif

