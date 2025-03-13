//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "ints.h"

/// microsecond precision timestamp
/// current UNIX timestamp in seconds multiplied by 1000000, plus microseconds since the beginning of current second
int64_t sphMicroTimer();

/// monotonic microsecond precision timestamp
/// value of CLOCK_MONOTONIC source
int64_t MonoMicroTimer();

/// return cpu time, in microseconds. CLOCK_THREAD_CPUTIME_ID, or CLOCK_PROCESS_CPUTIME_ID or fall to sphMicroTimer().
/// defined in searchd.cpp since depends from g_bCpuStats
int64_t sphProcessCpuTimer();
int64_t sphThreadCpuTimer();


/// returns sphThreadCpuTimer() adjusted to current coro task (coro may jump from thread to thread, so sphThreadCpuTimer() is irrelevant)
int64_t sphTaskCpuTimer();