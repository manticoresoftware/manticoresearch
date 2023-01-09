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
/// @file taskping.h
/// Periodic ping of HA hosts

/*
 * How async smart pinger works:
 * 1. We charge oneshot task to be run after host timestamp + g_iPingIntervalUs.
 * 2. On timeout engaged, the task is REMOVED from scheduler and performed.
 * 3. We check, whether the last host access timestamp is old enough to run the ping action.
 *  (if nothing, but ping in game, it seems that first check will success. If other queries invokes the host
 *  also, it very often means that timestamp is already refreshed, so, we not need to ping it).
 * 4a. If host is fresh enough (no need to ping) - go to pp 1.
 * 4b. If it is time to ping, we push the task to our async netloop with callback. Since networking is async,
 *   we send the task from the same worker (no wait, no need to switch into another thread), and set custom
 *   reporter with callback.
 * 5. On network callback - go to pp 1.
 * Removing the host is performed by set it's 'need_ping' flag to 0. The checker, noticing it, will not schedule
 *  next check or perform network task, but just release the host and that's all.
 * */


#ifndef MANTICORE_TASKPING_H
#define MANTICORE_TASKPING_H

namespace Ping {

	void Start();
};

#endif //MANTICORE_TASKPING_H
