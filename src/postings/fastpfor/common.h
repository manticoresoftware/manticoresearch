/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#ifndef COMMON_H_
#define COMMON_H_

// C headers (sorted)
#include <errno.h>
#include <fcntl.h>
#if defined(USE_SIMDE)
#define SIMDE_ENABLE_NATIVE_ALIASES 1
#include <simde/x86/sse4.1.h>
#else
#include <immintrin.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/mman.h>
#include <sys/resource.h>
#endif
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <time.h>

// C++ headers (sorted)
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

#ifdef _MSC_VER
#include <iso646.h>
#include <stdint.h>
#include <intrin.h>

#define __attribute__(n)
#define __restrict__ __restrict

#endif

#endif /* COMMON_H_ */
