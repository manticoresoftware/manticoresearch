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

#pragma once

#if _WIN32
// workaround Windows quirks
	#define popen		_popen
	#define pclose		_pclose
#endif

DWORD sphToDword ( const char * s );
int64_t sphToInt64 ( const char * szNumber, CSphString * pError=nullptr );
float sphToFloat ( const char * s );