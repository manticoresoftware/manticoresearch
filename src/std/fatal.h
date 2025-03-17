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

#pragma once

#include <stdarg.h>

#include "attrstub.h"

#define NO_RETURN __attribute__ ( ( __noreturn__ ) )

using SphDieCallback_t = bool ( * ) ( bool bDie, const char*, va_list );

/// crash with an error message, and do not have searchd watchdog attempt to resurrect
void sphDie ( const char* sFmt, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) ) NO_RETURN;
void sphDieVa ( const char* sFmt, va_list ap );

/// crash with an error message, but have searchd watchdog attempt to resurrect
void sphDieRestart ( const char* sMessage, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) ) NO_RETURN;

/// shutdown (not crash) on unrrecoverable error
void sphFatal ( const char* sFmt, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) ) NO_RETURN;
void sphFatalVa ( const char* sFmt, va_list ap );

/// log fatal error, not shutdown
void sphFatalLog ( const char* sFmt, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );

/// setup a callback function to call from sphDie() before exit
/// if callback returns false, sphDie() will not log to stdout
void sphSetDieCallback ( SphDieCallback_t pfDieCallback );

bool sphIsDied();

void sphSetDied();

/// similar as abort but closes FD
/// not available on MacOS
#if defined( __APPLE__ )
#define sphQuickExit exit
#else
#define sphQuickExit std::quick_exit
#endif