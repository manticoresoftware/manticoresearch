//
// $Id$
//

//
// Copyright (c) 2001-2013, Andrew Aksyonoff
// Copyright (c) 2008-2013, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"

void stem_soundex ( BYTE * pWord )
{
	static BYTE dLetter2Code[27] = "01230120022455012623010202";

	// check if the word only contains lowercase English letters
	BYTE * p = pWord;
	while ( *p>='a' && *p<='z' )
		p++;
	if ( *p )
		return;

	// do soundex
	p = pWord+1;
	BYTE * pOut = pWord+1;
	while ( *p )
	{
		BYTE c = dLetter2Code [ (*p)-'a' ];
		if ( c!='0' && pOut[-1]!=c )
			*pOut++ = c;
		p++;
	}

	while ( pOut-pWord<4 && pOut<p )
		*pOut++ = '0';

	*pOut++ = '\0';
}

//
// $Id$
//
