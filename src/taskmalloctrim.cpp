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

#include "taskmalloctrim.h"

#if HAVE_MALLOC_TRIM

#include <malloc.h>
#include "searchdtask.h"

static volatile int64_t iLastMallocTrimTimestamp = -1;
volatile int64_t iMallocTrimPeriod; // 30m

int PerformMallocTrim ( size_t iPad )
{
	int iRes = malloc_trim ( iPad );
	iLastMallocTrimTimestamp = sphMicroTimer ();
	return iRes;
}

static void MallocTrim ( void* )
{
	if (( iLastMallocTrimTimestamp + iMallocTrimPeriod )<=sphMicroTimer ())
		PerformMallocTrim (0);

	ScheduleMallocTrim ( iMallocTrimPeriod );
}

void ScheduleMallocTrim ( int64_t iPeriod )
{
	static int iMallocTrimTask = -1;
	if ( iMallocTrimTask<0 )
	{
		iLastMallocTrimTimestamp = sphMicroTimer ();
		iMallocTrimTask = TaskManager::RegisterGlobal ( "malloc_trim(0) periodically", MallocTrim, nullptr, 1, 1 );
	}
	iMallocTrimPeriod = iPeriod;
	TaskManager::ScheduleJob ( iMallocTrimTask, iLastMallocTrimTimestamp + iMallocTrimPeriod );
}

#else
void ScheduleMallocTrim ( int64_t iPeriod ) {};
int PerformMallocTrim ( size_t ) {return 0;};
#endif

