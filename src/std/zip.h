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

// big-endian (most significant septets first)
template<typename T>
int sphCalcZippedLen ( T tValue );

// big-endian (most significant septets first)
template<typename T, typename WRITER>
int ZipValueBE ( WRITER fnPut, T tValue );

// big-endian (most significant septets first)
template<typename T>
int ZipToPtrBE ( BYTE* pData, T tValue );

// big-endian (most significant septets first)
template<typename T, typename READER>
std::enable_if_t<sizeof ( T ) == 4, T> UnzipValueBE ( READER fnGet );

template<typename T, typename READER>
std::enable_if_t<sizeof ( T ) == 8, T> UnzipValueBE ( READER fnGet );

// big-endian (most significant septets first)
DWORD UnzipIntBE ( const BYTE*& pBuf );

// big-endian (most significant septets first)
SphOffset_t UnzipOffsetBE ( const BYTE*& pBuf );

// little-endian (least significant septets first)
template<typename T, typename WRITER>
int ZipValueLE ( WRITER fnPut, T tValue );

// little-endian (least significant septets first)
template<typename T>
int ZipToPtrLE ( BYTE* pData, T tValue );

// little-endian (least significant septets first)
template<typename T, typename READER>
std::enable_if_t<sizeof ( T ) == 4, T> UnzipValueLE ( READER fnGet );

template<typename T, typename READER>
std::enable_if_t<sizeof ( T ) == 8, T> UnzipValueLE ( READER fnGet );

// little-endian (least significant septets first)
DWORD UnzipIntLE ( const BYTE*& pBuf );

// little-endian (least significant septets first)
SphOffset_t UnzipOffsetLE ( const BYTE*& pBuf );


#include "zip_impl.h"