//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "networking_daemon.h"

#if WITH_ZSTD

bool IsZstdCompressionAvailable();

// Replace pSource with it's compressed version.
// Mysql proto will be wrapped into compressed.
void MakeZstdMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c>& pSource, int iLevel );

#else
inline bool IsZstdCompressionAvailable() { return false; }
inline void MakeZstdMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c>& pSource, int iLevel ) { };
#endif
