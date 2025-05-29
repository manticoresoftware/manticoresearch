//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

class Writer_i;
class JsonEscapedBuilder;

class ExceptionsTrie_c
{
	int m_dFirst[256];		  ///< table to speedup 1st byte lookup
	CSphVector<BYTE> m_dData; ///< data blob
	int m_iCount;			  ///< number of exceptions
	int m_iMappings;		  ///< offset where the nodes end, and output mappings start

	friend class ExceptionsTrieGen_c;

public:
	const BYTE* GetMapping ( int i ) const
	{
		assert ( i >= 0 && i < m_iMappings );
		int p = *(int*)&m_dData[i];
		if ( !p )
			return nullptr;
		assert ( p >= m_iMappings && p < m_dData.GetLength() );
		return &m_dData[p];
	}
	int GetFirst ( BYTE v ) const
	{
		return m_dFirst[v];
	}

	int GetNext ( int i, BYTE v ) const
	{
		assert ( i >= 0 && i < m_iMappings );
		if ( i == 0 )
			return m_dFirst[v];
		BYTE* p = &m_dData[i];
		int n = p[4];
		p += 5;
		for ( i = 0; i < n; i++ )
			if ( p[i] == v )
				return *(int*)&p[n + 4 * i]; // FIXME? unaligned
		return -1;
	}

	void Export ( Writer_i & w ) const;
	void Export ( JsonEscapedBuilder& tOut ) const;
};

/// exceptions trie builder
/// plain old text mappings in, nice useful trie out
class ExceptionsTrieGen_c
{
	class Impl_c;
	Impl_c* m_pImpl = nullptr;

	friend class ExceptionsTrie_c;

public:
	ExceptionsTrieGen_c();
	~ExceptionsTrieGen_c();
	void FoldSpace ( char* s ) const;
	bool ParseLine ( char* sBuffer, CSphString& sError );
	ExceptionsTrie_c* Build();
};
