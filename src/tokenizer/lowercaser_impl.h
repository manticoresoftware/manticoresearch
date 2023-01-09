//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "lowercaser.h"

#include "sphinxstd.h"

enum : DWORD {
	MASK_CODEPOINT = 0x00ffffffUL,			// mask off codepoint flags
	MASK_FLAGS = 0xff000000UL,				// mask off codepoint value
	FLAG_CODEPOINT_SPECIAL = 0x01000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL = 0x02000000UL,		// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM = 0x04000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_BOUNDARY = 0x10000000UL, // this codepoint is phrase boundary
	FLAG_CODEPOINT_IGNORE = 0x20000000UL,	// this codepoint is ignored
	FLAG_CODEPOINT_BLEND = 0x40000000UL		// this codepoint is "blended" (indexed both as a character, and as a separator)
};
