//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace bitmap
{

using CountBytes_fn = uint64_t (*) ( const uint8_t *, size_t );

// Count all set bits in an unaligned byte buffer, without reading past its end.
// No padding is required; a zero-length call allows a null pointer.
uint64_t CountBytes ( const uint8_t * pBytes, size_t nBytes );

} // namespace bitmap
