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
/// @file taskflushattrs.h
/// Task to check and flush modified attributes by demand and by timeout (if set)

#pragma once

#include "sphinxstd.h"

// set from param `attr_flush_period`, see conf_options_reference/searchd_program_configuration_options.html
void SetAttrFlushPeriod ( int64_t iPeriod );

// start periodical check and flush, if necessary
void ScheduleFlushAttrs();

//! \brief Pefrorm bulk save attributes for all local indexes, one-by-one
//! \returns true, if all indexes was saved. Called on shutdown (scheduler is already stopped there)
bool FinallySaveIndexes ();

/// Engages SaveIndexes task, and wait until it finished.
int CommandFlush ();
