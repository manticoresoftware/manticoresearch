//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "indexfilebase.h"
#include "indexfiles.h"


//////////////////////////////////////////////////////////////////////////

// index filebase + .szSuffix - like idx.meta, idx.foo, etc
CSphString IndexFileBase_c::GetFilename ( const char* szExt ) const
{
	return SphSprintf ( "%s.%s", GetFilebase(), szExt );
}

// index filebase + .num - like idx.1, idx.100, etc.
CSphString IndexFileBase_c::GetFilename ( int iSuffix ) const
{
	return SphSprintf ( "%s.%d", GetFilebase(), iSuffix );
}

// index filebase + .ext - like idx.sph, idx.tmp.spa
CSphString IndexFileBase_c::GetFilename ( ESphExt eExt ) const
{
	return SphSprintf ( "%s%s", GetFilebase(), sphGetExt ( eExt ) );
}

// index filebase + .tmp + .ext - like idx.sph, idx.tmp.spa
CSphString IndexFileBase_c::GetTmpFilename ( ESphExt eExt ) const
{
	return SphSprintf ( "%s.tmp%s", GetFilebase(), sphGetExt ( eExt ) );
}