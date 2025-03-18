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


#include "string.h"

//////////////////////////////////////////////////////////////////////////

char CSphString::EMPTY[] = "";

void ToLower ( Str_t sVal )
{
	char * s = const_cast<char *> ( sVal.first );
	const char * sEnd = s + sVal.second;
	for ( ; s<sEnd; s++ )
		*s = (char)tolower ( *s );
}
