//
// Copyright (c) 2019, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef MANTICORE_TASKMALLOCTRIM_H
#define MANTICORE_TASKMALLOCTRIM_H

#include "sphinxutils.h"

#define DEFAULT_MALLOC_TRIM_PERIOD (30*60*1000000)

void ScheduleMallocTrim ( int64_t iPeriod = DEFAULT_MALLOC_TRIM_PERIOD);
int PerformMallocTrim ( size_t iPad );

#endif //MANTICORE_TASKMALLOCTRIM_H
