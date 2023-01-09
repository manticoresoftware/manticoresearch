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

/// lowercaser remap range
struct CSphRemapRange
{
	int m_iStart = -1;
	int m_iEnd = -1;
	int m_iRemapStart = -1;

	CSphRemapRange() = default;
	CSphRemapRange ( int iStart, int iEnd, int iRemapStart )
		: m_iStart ( iStart )
		, m_iEnd ( iEnd )
		, m_iRemapStart ( iRemapStart )
	{}
	explicit CSphRemapRange ( int iSingle )
		: m_iStart ( iSingle )
		, m_iEnd ( iSingle )
		, m_iRemapStart ( iSingle )
	{}
};


inline bool operator< ( const CSphRemapRange& a, const CSphRemapRange& b )
{
	return a.m_iStart < b.m_iStart;
}
