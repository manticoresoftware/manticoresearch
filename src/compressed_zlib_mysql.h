//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "networking_daemon.h"

#if WITH_ZLIB

bool IsZlibCompressionAvailable();

// Replace pSource with it's compressed version.
// Mysql proto will be wrapped into compressed.
void MakeZlibMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c> & pSource );

#else
inline bool IsZlibCompressionAvailable() { return false; }
inline void MakeZlibMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c> & pSource ) { };
#endif
