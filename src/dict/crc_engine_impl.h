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

/////////////////////////////////////////////////////////////////////////////

#include "sphinx.h"

template<>
inline SphWordID_t CCRCEngine<CRCALGO::CRC32>::DoCrc ( const BYTE* pWord )
{
	return sphCRC32 ( pWord );
}


template<>
inline SphWordID_t CCRCEngine<CRCALGO::FNV64>::DoCrc ( const BYTE* pWord )
{
	return (SphWordID_t)sphFNV64 ( pWord );
}


template<>
inline SphWordID_t CCRCEngine<CRCALGO::CRC32>::DoCrc ( const BYTE* pWord, int iLen )
{
	return sphCRC32 ( pWord, iLen );
}


template<>
inline SphWordID_t CCRCEngine<CRCALGO::FNV64>::DoCrc ( const BYTE* pWord, int iLen )
{
	return (SphWordID_t)sphFNV64 ( pWord, iLen );
}