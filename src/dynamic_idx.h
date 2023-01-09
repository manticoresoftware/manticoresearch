//
// Copyright (c) 2020, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#pragma once

#include "searchdaemon.h"

// generator, should feed given RowBuffer_i with table data (i.e., first header, then data rows)
using TableFeeder_fn = std::function<void ( RowBuffer_i* )>;

// serve any generic table of columns as index with possibility to filter (full-scan)
ServedIndexRefPtr_c MakeDynamicIndex ( TableFeeder_fn fnFeed );

// schema of any generic table of columns as index with possibility to filter (full-scan)
ServedIndexRefPtr_c MakeDynamicIndexSchema ( TableFeeder_fn fnFeed );
