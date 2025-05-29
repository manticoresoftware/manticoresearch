//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

/// generic sort
template<typename T, typename U, typename V, typename INT>
void sphSort ( T* pData, INT iCount, U&& COMP, V&& ACC ) noexcept;

template<typename T, typename U, typename INT>
void sphSort ( T* pData, INT iCount, U&& COMP ) noexcept;

template<typename T, typename INT>
void sphSort ( T* pData, INT iCount ) noexcept;

#include "sort_impl.h"
