//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxstem_
#define _sphinxstem_

#include "sphinx.h"

/// initialize English stemmar
void	stem_en_init ();

/// initialize Russian stemmar
void	stem_ru_init ();

/// stem English word
void	stem_en ( BYTE * pWord );

/// stem Russian word in Windows-1251 encoding
void	stem_ru_cp1251 ( BYTE * pWord );

/// stem Russian word in UTF-8 encoding
void	stem_ru_utf8 ( WORD * pWord );

#endif // _sphinxstem_

//
// $Id$
//
