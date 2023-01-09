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

#include <type_traits>

// workaround missing "is_trivially_copyable" in g++ < 5.0
#if defined( __GNUG__ ) && ( __GNUC__ < 5 ) && !defined( __clang__ )
#define IS_TRIVIALLY_COPYABLE( T ) __has_trivial_copy ( T )
#define IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE( T ) std::has_trivial_default_constructor<T>::value
#else
#define IS_TRIVIALLY_COPYABLE( T ) std::is_trivially_copyable<T>::value
#define IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE( T ) std::is_trivially_default_constructible<T>::value
#endif