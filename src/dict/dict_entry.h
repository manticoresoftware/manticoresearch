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

#pragma once

#include "sphinxstd.h"

/// dictionary entry
/// some of the fields might be unused depending on specific dictionary type
struct DictEntry_t
{
	SphWordID_t m_uWordID = 0;		   ///< keyword id (for dict=crc)
	const BYTE* m_sKeyword = nullptr;  ///< keyword text (for dict=keywords)
	int m_iDocs = 0;				   ///< number of matching documents
	int m_iHits = 0;				   ///< number of occurrences
	SphOffset_t m_iDoclistOffset = 0;  ///< absolute document list offset (into .spd)
	SphOffset_t m_iDoclistLength = 0;  ///< document list length in bytes
	SphOffset_t m_iSkiplistOffset = 0; ///< absolute skiplist offset (into .spe)
	int m_iDoclistHint = 0;			   ///< raw document list length hint value (0..255 range, 1 byte)
};