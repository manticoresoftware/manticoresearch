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

#include "sphinxstd.h"

namespace html_stripper {

struct StripperTag_t
{
	CSphString m_sTag;			///< tag name
	int m_iTagLen = 0;			///< tag name length
	bool m_bInline = false;		///< whether this tag is inline
	bool m_bIndexAttrs = false; ///< whether to index attrs
	bool m_bRemove = false;		///< whether to remove contents
	bool m_bPara = false;		///< whether to mark a paragraph boundary
	bool m_bZone = false;		///< whether to mark a zone boundary
	bool m_bZonePrefix = false; ///< whether the zone name is a full name or a prefix
	StrVec_t m_dAttrs;			///< attr names to index
};

inline bool operator< ( const StripperTag_t& lhs, const StripperTag_t& rhs )
{
	return strcmp ( lhs.m_sTag.cstr(), rhs.m_sTag.cstr() ) < 0;
}

}
