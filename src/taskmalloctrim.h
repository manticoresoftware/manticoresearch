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

#ifndef MANTICORE_TASKMALLOCTRIM_H
#define MANTICORE_TASKMALLOCTRIM_H

#include "sphinxutils.h"

#define DEFAULT_MALLOC_TRIM_PERIOD (30*60*1000000)

void ScheduleMallocTrim ( int64_t iPeriod = DEFAULT_MALLOC_TRIM_PERIOD);
int PerformMallocTrim ( size_t iPad );

#endif //MANTICORE_TASKMALLOCTRIM_H
