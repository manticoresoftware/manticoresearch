//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#if defined(__x86_64__) || defined(__i386__)
	#include <cpuid.h>
#endif

inline bool IsSSE42Supported()
{
#if defined(__x86_64__) || defined(__i386__)
 	uint32_t dInfo[4];
	__cpuid ( 1, dInfo[0], dInfo[1], dInfo[2], dInfo[3] );
	return (dInfo[2] & (1 << 20)) != 0;
#else
	return true;	// assumes that it's ARM and simde is used
#endif
}
