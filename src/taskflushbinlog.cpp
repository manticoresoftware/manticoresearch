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

#include "taskflushbinlog.h"
#include "searchdtask.h"
#include "searchdaemon.h"
#include "binlog.h"

static void ScheduleFlushBinlogNext ()
{
	static int iFlushBinlogTask = TaskManager::RegisterGlobal ( "Flush binlog", 1 );
	TaskManager::ScheduleJob ( iFlushBinlogTask, Binlog::NextFlushTimestamp (), []
	{
		auto pDesc = PublishSystemInfo ( "FLUSH RT BINLOG" );
		Binlog::Flush();
		ScheduleFlushBinlogNext();
	});
}

void StartRtBinlogFlushing ()
{
	if ( !Binlog::IsFlushEnabled() )
		return;

	ScheduleFlushBinlogNext ();
}
