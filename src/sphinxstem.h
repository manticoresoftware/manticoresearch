//
// $Id$
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

/// stem Russian word
void	stem_ru ( BYTE * pWord );

#endif // _sphinxstem_

//
// $Id$
//
