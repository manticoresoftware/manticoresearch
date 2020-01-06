//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
#ifndef MANTICORE_TASKPREREAD_H
#define MANTICORE_TASKPREREAD_H

#include "sphinxstd.h"

void DoPreread ();
void StartPreread ();
bool WaitFinishPreread ( int64_t uSec );

#endif //MANTICORE_TASKPREREAD_H
