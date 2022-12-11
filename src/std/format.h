//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "config.h"

#if HAVE_INTTYPES_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

#ifndef PRIi64
#define PRIi64 "lld"
#endif

#define UINT64_FMT "%" PRIu64
#define INT64_FMT "%" PRIi64

#ifndef PRIx64
#define PRIx64 "llx"
#endif