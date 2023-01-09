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

#include "sys.h"

#if _WIN32

#include "ints.h"

int sphCpuThreadsCount()
{
	SYSTEM_INFO tInfo;
	GetSystemInfo ( &tInfo );
	return tInfo.dwNumberOfProcessors;
}

static int GetMemPageSize()
{
	SYSTEM_INFO tInfo;
	GetSystemInfo ( &tInfo );
	return tInfo.dwPageSize;
}

#else

#include <unistd.h>

int sphCpuThreadsCount()
{
	return sysconf ( _SC_NPROCESSORS_ONLN );
}


static int GetMemPageSize()
{
	return getpagesize();
}

#endif

int sphGetMemPageSize()
{
	static int iMemPageSize = GetMemPageSize();
	return iMemPageSize;
}