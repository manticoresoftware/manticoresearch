//
// Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/blobs.h"
#include "std/stringbuilder.h"
#include <memory>

namespace Dispatcher {


class TaskSource_i
{
public:
	virtual ~TaskSource_i() = default;

	// if iTask<0 - consume new task,
	// if iTask>=0 - does nothing, returns true.
	virtual bool FetchTask ( int& iTask ) = 0;
};

class TaskDispatcher_i
{
public:
	virtual ~TaskDispatcher_i() = default;
	virtual int GetConcurrency() const = 0;
	virtual std::unique_ptr<TaskSource_i> MakeSource() = 0;
};

// trivial dispatcher just iterates from 0 to iJobs with single monotonic atomic shared across all workers
std::unique_ptr<TaskDispatcher_i> MakeTrivial ( int iJobs, int iConcurrency );

// RoundRobin dispatcher provides sequence for iFibers sources, each takes serie of iBatch jobs, as:
// let iFibers = 3. For given tasks they will come to follow fiber (1 to 3) depending on the batch:
// for tasks 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
// batch=1:  1 2 3 1 2 3 1 2 3  1  2  3  1  2  3  1  2  3  1  2  3  1  2  3  1  2  3  1  2  3
// batch=2:	 1 1 2 2 3 3 1 1 2  2  3  3  1  1  2  2  3  3  1  1  2  2  3  3  1  1  2  2  3  3
// batch=3:  1 1 1 2 2 2 3 3 3  1  1  1  2  2  2  3  3  3  1  1  1  2  2  2  3  3  3  1  1  1 ...
// with batch=1 it will give 1,4,7,10.. to 1-st fiber, 2,5,8,11... to 2-nd, 3,6,9,12... to 3-rd.
// with batch=3 it will give 1-3,10-12,19-21... to 1-st, 4-6,13-15,22-24... to 2-nd, 7-9,16-18,25-27... to 3-rd.
// iFibers should be <= iConcurrency (will be adjusted otherwise)
// if iConcurrency > iFibers, extra contexts will be idle
// default iFibers=0 makes iConcurrency fibers.
std::unique_ptr<TaskDispatcher_i> MakeRoundRobin ( int iJobs, int iConcurrency, int iBatch=1 );


} // namespace Dispatcher