//
// Copyright (c) 2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "networking_daemon.h"

bool IsCompressionAvailable();

// Replace pSource with it's compressed version.
// Mysql proto will be wrapped into compressed.
void MakeMysqlCompressedLayer ( AsyncNetBufferPtr_c & pSource );