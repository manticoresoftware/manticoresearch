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

#pragma once

// HAVE_DLOPEN
#include "config.h"
#if HAVE_DLOPEN
#if _WIN32
	#define RTLD_LAZY		0
	#define RTLD_NOW		0
	#define RTLD_LOCAL		0
	#define RTLD_GLOBAL		0

	void *			dlsym ( void * lib, const char * name );
	void *			dlopen ( const char * libname, int );
	int				dlclose ( void * lib );
	const char *	dlerror();
#else
#	include <dlfcn.h>
#endif
#include "std/string.h"
#include <utility>
#endif // _WIN32


class ScopedHandle_c final
{
public:
	explicit ScopedHandle_c ( void * pHandle )
		: m_pHandle ( pHandle )
	{}

	~ScopedHandle_c();

	void * Leak() noexcept
	{
		return std::exchange ( m_pHandle, nullptr );
	}

	void * Get() const noexcept { return m_pHandle; }

private:
	void * m_pHandle = nullptr;
};

void * DlSym ( void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError );

template<typename T>
bool LoadFunc ( T & pFunc, void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError )
{
	pFunc = (T) DlSym ( pHandle, szFunc, sLib, sError  );
	return !!pFunc;
}

CSphString TryDifferentPaths ( const CSphString & sLibfile, const CSphString & sFullpath, int iVersion, const char * szPostfix = nullptr );
