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

#include "fatal.h"

#include <cstdlib>
#include <cstdio>

#include "sphinxutils.h"

namespace
{
volatile bool& IsDied()
{
	static bool bDied = false;
	return bDied;
}

SphDieCallback_t& pfDieCallback()
{
	static SphDieCallback_t g_pfDieCallback = nullptr;
	return g_pfDieCallback;
}
} // namespace

bool sphIsDied()
{
	return IsDied();
}

void sphSetDied()
{
	IsDied() = true;
}

void sphSetDieCallback ( SphDieCallback_t pDieCallback )
{
	pfDieCallback() = pDieCallback;
}

void sphDieVa ( const char* sFmt, va_list ap )
{
	// if there's no callback,
	// or if callback returns true,
	// log to stdout
	sphSetDied();
	if ( !pfDieCallback() || pfDieCallback() ( true, sFmt, ap ) )
	{
		char sBuf[1024];
		vsnprintf ( sBuf, sizeof ( sBuf ), sFmt, ap );
		fprintf ( stdout, "FATAL: %s\n", sBuf );
	}
}

void sphDie ( const char* sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphDieVa ( sFmt, ap );
	va_end ( ap );
	exit ( 1 );
}

void sphDieRestart ( const char* sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphDieVa ( sFmt, ap );
	va_end ( ap );
	sphQuickExit ( 2 ); // almost CRASH_EXIT
}

void sphFatalVa ( const char* sFmt, va_list ap )
{
	g_pLogger() ( SPH_LOG_FATAL, sFmt, ap );
	if ( pfDieCallback() )
		pfDieCallback() ( false, sFmt, ap );
	exit ( 1 );
}

void sphFatal ( const char* sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	g_pLogger() ( SPH_LOG_FATAL, sFmt, ap );
	if ( pfDieCallback() )
		pfDieCallback() ( false, sFmt, ap );
	va_end ( ap );
	exit ( 1 );
}

void sphFatalLog ( const char* sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	g_pLogger() ( SPH_LOG_FATAL, sFmt, ap );
	va_end ( ap );
}