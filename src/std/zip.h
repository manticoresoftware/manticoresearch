//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

// big-endian (most significant septets first)
template<typename T>
int sphCalcZippedLen ( T tValue );

// big-endian (most significant septets first)
template<typename T, typename WRITER>
int sphZipValue ( WRITER fnPut, T tValue );

// big-endian (most significant septets first)
template<typename T>
int sphZipToPtr ( BYTE* pData, T tValue );

// big-endian (most significant septets first)
DWORD sphUnzipInt ( const BYTE*& pBuf );

// big-endian (most significant septets first)
SphOffset_t sphUnzipOffset ( const BYTE*& pBuf );

#include "zip_impl.h"