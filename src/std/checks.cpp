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

#include "checks.h"
#include "ints.h"

#include <cstdlib>

#if _WIN32
#ifndef NDEBUG

void sphAssert ( const char* sExpr, const char* sFile, int iLine )
{
	char sBuffer[1024];
	_snprintf ( sBuffer, sizeof ( sBuffer ), "%s(%d): assertion %s failed\n", sFile, iLine, sExpr );

	if ( MessageBox ( NULL, sBuffer, "Assert failed! Cancel to debug.", MB_OKCANCEL | MB_TOPMOST | MB_SYSTEMMODAL | MB_ICONEXCLAMATION ) != IDOK )
	{
		__debugbreak();
	} else
	{
		fprintf ( stdout, "%s", sBuffer );
		exit ( 1 );
	}
}

#endif // !NDEBUG
#endif // _WIN32