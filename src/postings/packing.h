// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "codec.h"

namespace postings
{

// Internal kernels operate on aligned native words, exactly 128 values per call.
struct Packing_t
{
	void ( *m_fnPack ) ( const uint32_t *, uint32_t *, unsigned );
	void ( *m_fnUnpack ) ( const uint32_t *, uint32_t *, unsigned );
	const char * m_sName;
};

const Packing_t * GetPacking ( Backend_e eBackend );
#ifdef POSTINGS_HAVE_SSE2
extern const Packing_t g_tSse2;
#endif

}
