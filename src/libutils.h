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

#include "sphinxstd.h"

#if _WIN32
	#define RTLD_LAZY		0
	#define RTLD_NOW		0
	#define RTLD_LOCAL		0
	#define RTLD_GLOBAL		0

	void *			dlsym ( void * lib, const char * name );
	void *			dlopen ( const char * libname, int );
	int				dlclose ( void * lib );
	const char *	dlerror();
#else // !_WIN32
#include "config.h"
#if HAVE_DLOPEN
#	include <dlfcn.h>
#endif // HAVE_DLOPEN
#endif // _WIN32

#if HAVE_DLOPEN

#include <utility>

class ScopedHandle_c final
{
public:
	explicit ScopedHandle_c ( void * pHandle )
		: m_pHandle ( pHandle )
	{}

	~ScopedHandle_c()
	{
		if ( m_pHandle )
			dlclose ( m_pHandle );
	}

	void * Leak()
	{
		return std::exchange ( m_pHandle, nullptr );
	}

	void * Get() { return m_pHandle; }

private:
	void * m_pHandle = nullptr;
};

template <typename T>
bool LoadFunc ( T & pFunc, void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError )
{
	pFunc = (T) dlsym ( pHandle, szFunc );
	if ( !pFunc )
	{
		sError.SetSprintf ( "symbol '%s' not found in '%s'", szFunc, sLib.cstr() );
		dlclose ( pHandle );
		return false;
	}

	return true;
}
#else
template <typename T>
bool LoadFunc ( T & pFunc, void * pHandle, const char * szFunc, const CSphString & sLib, CSphString & sError )
{
	return false;
}
#endif // HAVE_DLOPEN

CSphString TryDifferentPaths ( const CSphString & sLibfile, const CSphString & sFullpath, int iVersion );
