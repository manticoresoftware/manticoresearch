//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _libutils_
#define _libutils_

#include "sphinxstd.h"

#if !USE_WINDOWS
	#include <unistd.h>
	#include <sys/time.h>
	#if HAVE_DLOPEN
		#include <dlfcn.h>
	#endif // HAVE_DLOPEN
#endif // !USE_WINDOWS

#if !USE_WINDOWS
	#ifndef HAVE_DLERROR
		#define dlerror() ""
	#endif // HAVE_DLERROR
#endif // !USE_WINDOWS

#if USE_WINDOWS
#undef HAVE_DLOPEN
#define HAVE_DLOPEN		1
#define RTLD_LAZY		0
#define RTLD_LOCAL		0
#define RTLD_GLOBAL		0

void *			dlsym ( void * lib, const char * name );
void *			dlopen ( const char * libname, int );
int				dlclose ( void * lib );
const char *	dlerror();

#endif // USE_WINDOWS

#endif // _libutils_
