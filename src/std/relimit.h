//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "ints.h"
#include <limits.h>

//////////////////////////////////////////////////////////////////////////
/// Resize backends for vector
/// Each backend provides Relimit

namespace sph
{

/// Default relimit: grow 2x
class DefaultRelimit
{
public:
	static constexpr int64_t SANE_SIZE = INT_MAX / 2;
	static constexpr int MAGIC_INITIAL_LIMIT = 8;
	static inline int64_t Relimit ( int64_t iLimit, int64_t iNewLimit );
};

/// tight-vector policy
/// grow only 1.2x on resize (not 2x) starting from a certain threshold
class TightRelimit: public DefaultRelimit
{
public:
	static constexpr float GROW = 1.2f;
	static constexpr int64_t SANE_SIZE = (double)INT_MAX / GROW; // double, since INT_MAX is not convertible to float precisely
	static constexpr int SLOW_GROW_TRESHOLD = 1024;
	static inline int64_t Relimit ( int64_t iLimit, int64_t iNewLimit );
};

} // namespace sph

#include "relimit_impl.h"
