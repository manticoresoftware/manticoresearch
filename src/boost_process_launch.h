//
// Copyright (c) 2026, Manticore Software LTD
//
// Shared synchronization for Boost.Process v1 launches.
//

#ifndef MANTICORE_BOOST_PROCESS_LAUNCH_H
#define MANTICORE_BOOST_PROCESS_LAUNCH_H

#if _WIN32

#include <mutex>

inline std::mutex & GetBoostProcessLaunchMutex()
{
	static std::mutex tMutex;
	return tMutex;
}

#endif

#endif // MANTICORE_BOOST_PROCESS_LAUNCH_H
