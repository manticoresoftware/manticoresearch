//
// Copyright (c) 2021, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxstd.h"

#include <math.h>
#include <float.h>

DWORD sphToDword ( const char * s )
{
	if ( !s ) return 0;
	return strtoul ( s, NULL, 10 );
}


// move to source mysql
float sphToFloat ( const char * s )
{
	if ( !s ) return 0.0f;
	double fRes = strtod ( s, NULL );
	if ( fRes==-HUGE_VAL || fRes<=-FLT_MAX )
		return -FLT_MAX;

	if ( fRes==HUGE_VAL || fRes>=FLT_MAX )
		return FLT_MAX;

	return (float)fRes;
}