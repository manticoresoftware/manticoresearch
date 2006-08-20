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

#include "sphinx.h"
#include <string.h>

/////////////////////////////////////////////////////////////////////////////
// SBCS windows-1251 implementation
/////////////////////////////////////////////////////////////////////////////

struct RussianAlphabetWin1251_t
{
	enum
	{
		A	= 0xE0,
		B	= 0xE1,
		V	= 0xE2,
		G	= 0xE3,
		D	= 0xE4,
		E	= 0xE5,
		YO	= 0xB8,
		ZH	= 0xE6,
		Z	= 0xE7,
		I	= 0xE8,
		IY	= 0xE9,
		K	= 0xEA,
		L	= 0xEB,
		M	= 0xEC,
		N	= 0xED,
		O	= 0xEE,
		P	= 0xEF,
		R	= 0xF0,
		S	= 0xF1,
		T	= 0xF2,
		U	= 0xF3,
		F	= 0xF4,
		H	= 0xF5,
		TS	= 0xF6,
		CH	= 0xF7,
		SH	= 0xF8,
		SCH	= 0xF9,
		TVY	= 0xFA, // TVYordiy znak
		Y	= 0xFB,
		MYA	= 0xFC, // MYAgkiy znak
		EE	= 0xFD,
		YU	= 0xFE,
		YA	= 0xFF
	};
};
	
#define LOC_CHAR_TYPE		unsigned char
#define LOC_PREFIX(_a)		_a##_cp1251
#define RUS					RussianAlphabetWin1251_t

#include "sphinxstemru.inl"

/////////////////////////////////////////////////////////////////////////////
// UTF-8 implementation
/////////////////////////////////////////////////////////////////////////////

struct RussianAlphabetUTF8_t
{
	enum
	{
		A	= 0xB0D0U,
		B	= 0xB1D0U,
		V	= 0xB2D0U,
		G	= 0xB3D0U,
		D	= 0xB4D0U,
		E	= 0xB5D0U,
		YO	= 0x91D1U,
		ZH	= 0xB6D0U,
		Z	= 0xB7D0U,
		I	= 0xB8D0U,
		IY	= 0xB9D0U,
		K	= 0xBAD0U,
		L	= 0xBBD0U,
		M	= 0xBCD0U,
		N	= 0xBDD0U,
		O	= 0xBED0U,
		P	= 0xBFD0U,
		R	= 0x80D1U,
		S	= 0x81D1U,
		T	= 0x82D1U,
		U	= 0x83D1U,
		F	= 0x84D1U,
		H	= 0x85D1U,
		TS	= 0x86D1U,
		CH	= 0x87D1U,
		SH	= 0x88D1U,
		SCH	= 0x89D1U,
		TVY	= 0x8AD1U, // TVYordiy znak
		Y	= 0x8BD1U,
		MYA	= 0x8CD1U, // MYAgkiy znak
		EE	= 0x8DD1U,
		YU	= 0x8ED1U,
		YA	= 0x8FD1U
	};
};

#define LOC_CHAR_TYPE		unsigned short
#define LOC_PREFIX(_a)		_a##_utf8
#define RUS					RussianAlphabetUTF8_t

#include "sphinxstemru.inl"

/////////////////////////////////////////////////////////////////////////////

void stem_ru_init ()
{
	stem_ru_init_cp1251 ();
	stem_ru_init_utf8 ();
}

//
// $Id$
//
