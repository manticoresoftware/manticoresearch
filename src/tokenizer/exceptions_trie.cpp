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

#include "exceptions_trie.h"

#include "sphinxstd.h"
#include "fileio.h"
#include "sphinxutils.h"
#include "sphinxjson.h"

/////////////////////////////////////////////////////////////////////////////
// TOKENIZING EXCEPTIONS
/////////////////////////////////////////////////////////////////////////////

/// exceptions trie, stored in a tidy simple blob
/// we serialize each trie node as follows:
///
/// int result_offset, 0 if no output mapping
/// BYTE num_bytes, 0 if no further valid bytes can be accepted
/// BYTE values[num_bytes], known accepted byte values
/// BYTE offsets[num_bytes], and the respective next node offsets
///
/// output mappings themselves are serialized just after the nodes,
/// as plain old ASCIIZ strings

template<typename WRITER>
void ExceptionsTrie_c::Export ( WRITER&& W, CSphVector<BYTE>& dPrefix, int iNode, int* pCount ) const
{
	assert ( iNode >= 0 && iNode < m_iMappings );
	BYTE* p = &m_dData[iNode];

	int iTo = *(int*)const_cast<BYTE*> ( p );
	if ( iTo > 0 )
	{
		CSphString s;
		const char* sTo = (char*)&m_dData[iTo];
		s.SetBinary ( (char*)dPrefix.Begin(), dPrefix.GetLength() );
		s.SetSprintf ( "%s => %s\n", s.cstr(), sTo );
		W ( s.cstr() );
		( *pCount )++;
	}

	int n = p[4];
	if ( n == 0 )
		return;

	p += 5;
	for ( int i = 0; i < n; i++ )
	{
		dPrefix.Add ( p[i] );
		Export ( W, dPrefix, *(int*)&p[n + 4 * i], pCount );
		dPrefix.Pop();
	}
}

void ExceptionsTrie_c::Export ( CSphWriter & w ) const
{
	CSphVector<BYTE> dPrefix;
	int iCount = 0;

	w.PutDword ( m_iCount );
	Export ( [&w] (const char* szLine) { w.PutString ( szLine ); }, dPrefix, 0, &iCount);
	assert ( iCount==m_iCount );
}

void ExceptionsTrie_c::Export ( JsonEscapedBuilder & tOut ) const
{
	CSphVector<BYTE> dPrefix;
	int iCount = 0;

	Export ( [&tOut] (const char* szLine) { tOut.FixupSpacedAndAppendEscaped ( szLine ); }, dPrefix, 0, &iCount);
	assert ( iCount==m_iCount );
}

/// intermediate exceptions trie node
/// only used by ExceptionsTrieGen_c, while building a blob
class ExceptionsTrieNode_c
{
	friend class ExceptionsTrieGen_c;

	struct Entry_t
	{
		BYTE m_uValue;
		ExceptionsTrieNode_c* m_pKid;
	};

	CSphString m_sTo;			 ///< output mapping for current prefix, if any
	CSphVector<Entry_t> m_dKids; ///< known and accepted incoming byte values

public:
	~ExceptionsTrieNode_c()
	{
		for ( auto& dKid: m_dKids )
			SafeDelete ( dKid.m_pKid );
	}

	/// returns false on a duplicate "from" part, or true on success
	bool AddMapping ( const BYTE* sFrom, const BYTE* sTo )
	{
		// no more bytes to consume? this is our output mapping then
		if ( !*sFrom )
		{
			if ( !m_sTo.IsEmpty() )
				return false;
			m_sTo = (const char*)sTo;
			return true;
		}

		int i;
		for ( i = 0; i < m_dKids.GetLength(); i++ )
			if ( m_dKids[i].m_uValue == *sFrom )
				break;
		if ( i == m_dKids.GetLength() )
		{
			Entry_t& t = m_dKids.Add();
			t.m_uValue = *sFrom;
			t.m_pKid = new ExceptionsTrieNode_c();
		}
		return m_dKids[i].m_pKid->AddMapping ( sFrom + 1, sTo );
	}
};


/// exceptions trie builder
/// plain old text mappings in, nice useful trie out
class ExceptionsTrieGen_c::Impl_c
{
	ExceptionsTrieNode_c* m_pRoot;
	int m_iCount;

public:
	Impl_c()
	{
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
	}

	~Impl_c()
	{
		SafeDelete ( m_pRoot );
	}

	/// trims left/right whitespace, folds inner whitespace
	void FoldSpace ( char* s ) const
	{
		// skip leading spaces
		char* d = s;
		while ( *s && sphIsSpace ( *s ) )
			s++;

		// handle degenerate (empty string) case
		if ( !*s )
		{
			*d = '\0';
			return;
		}

		while ( *s )
		{
			// copy another token, add exactly 1 space after it, and skip whitespace
			while ( *s && !sphIsSpace ( *s ) )
				*d++ = *s++;
			*d++ = ' ';
			while ( sphIsSpace ( *s ) )
				s++;
		}

		// replace that last space that we added
		d[-1] = '\0';
	}

	bool ParseLine ( char* sBuffer, CSphString& sError )
	{
#define LOC_ERR( _arg ) { sError = _arg; return false; }
		assert ( m_pRoot );

		// extract map-from and map-to parts
		char* sSplit = strstr ( sBuffer, "=>" );
		if ( !sSplit )
			LOC_ERR ( "mapping token (=>) not found" );

		char* sFrom = sBuffer;
		char* sTo = sSplit + 2; // skip "=>"
		*sSplit = '\0';

		// trim map-from, map-to
		FoldSpace ( sFrom );
		FoldSpace ( sTo );
		if ( !*sFrom )
			LOC_ERR ( "empty map-from part" );
		if ( !*sTo )
			LOC_ERR ( "empty map-to part" );
		if ( (int)strlen ( sFrom ) > MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );
		if ( (int)strlen ( sTo ) > MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );

		// all parsed ok; add it!
		if ( m_pRoot->AddMapping ( (BYTE*)sFrom, (BYTE*)sTo ) )
			m_iCount++;
		else
			LOC_ERR ( "duplicate map-from part" );

		return true;
#undef LOC_ERR
	}

	ExceptionsTrie_c* Build()
	{
		if ( !m_pRoot || !m_pRoot->m_sTo.IsEmpty() || m_pRoot->m_dKids.GetLength() == 0 )
			return nullptr;

		auto* pRes = new ExceptionsTrie_c();
		pRes->m_iCount = m_iCount;

		// save the nodes themselves
		CSphVector<BYTE> dMappings;
		SaveNode ( pRes, m_pRoot, dMappings );

		// append and fixup output mappings
		CSphVector<BYTE>& d = pRes->m_dData;
		pRes->m_iMappings = d.GetLength();
		d.Append ( dMappings );

		BYTE* p = d.Begin();
		BYTE* pMax = p + pRes->m_iMappings;
		while ( p < pMax )
		{
			// fixup offset in the current node, if needed
			int* pOff = (int*)p; // FIXME? unaligned
			if ( ( *pOff ) < 0 )
				*pOff = 0; // convert -1 to 0 for non-outputs
			else
				( *pOff ) += pRes->m_iMappings; // fixup offsets for outputs

			// proceed to the next node
			int n = p[4];
			p += 5 + 5 * n;
		}
		assert ( p == pMax );

		// build the speedup table for the very 1st byte
		for (int & i : pRes->m_dFirst)
			i = -1;
		int n = d[4];
		for ( int i = 0; i < n; i++ )
			pRes->m_dFirst[d[5 + i]] = *(int*)&pRes->m_dData[5 + n + 4 * i];

		SafeDelete ( m_pRoot );
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
		return pRes;
	}

private:
	void SaveInt ( CSphVector<BYTE>& v, int p, int x )
	{
#if USE_LITTLE_ENDIAN
		v[p] = x & 0xff;
		v[p + 1] = ( x >> 8 ) & 0xff;
		v[p + 2] = ( x >> 16 ) & 0xff;
		v[p + 3] = ( x >> 24 ) & 0xff;
#else
		v[p] = ( x >> 24 ) & 0xff;
		v[p + 1] = ( x >> 16 ) & 0xff;
		v[p + 2] = ( x >> 8 ) & 0xff;
		v[p + 3] = x & 0xff;
#endif
	}

	int SaveNode ( ExceptionsTrie_c* pRes, ExceptionsTrieNode_c* pNode, CSphVector<BYTE>& dMappings )
	{
		CSphVector<BYTE>& d = pRes->m_dData; // shortcut

		// remember the start node offset
		int iRes = d.GetLength();
		int n = pNode->m_dKids.GetLength();
		assert ( !( pNode->m_sTo.IsEmpty() && n == 0 ) );

		// save offset into dMappings, or temporary (!) save -1 if there is no output mapping
		// note that we will fixup those -1's to 0's afterwards
		int iOff = -1;
		if ( !pNode->m_sTo.IsEmpty() )
		{
			iOff = dMappings.GetLength();
			int iLen = pNode->m_sTo.Length();
			memcpy ( dMappings.AddN ( iLen + 1 ), pNode->m_sTo.cstr(), iLen + 1 );
		}
		d.AddN ( 4 );
		SaveInt ( d, d.GetLength() - 4, iOff );

		// sort children nodes by value
		pNode->m_dKids.Sort ( bind ( &ExceptionsTrieNode_c::Entry_t::m_uValue ) );

		// save num_values, and values[]
		d.Add ( (BYTE)n );
		ARRAY_FOREACH ( i, pNode->m_dKids )
			d.Add ( pNode->m_dKids[i].m_uValue );

		// save offsets[], and the respective child nodes
		int p = d.GetLength();
		d.AddN ( 4 * n );
		for ( int i = 0; i < n; i++, p += 4 )
			SaveInt ( d, p, SaveNode ( pRes, pNode->m_dKids[i].m_pKid, dMappings ) );
		assert ( p == iRes + 5 + 5 * n );

		// done!
		return iRes;
	}
};


ExceptionsTrieGen_c::ExceptionsTrieGen_c()
{
	m_pImpl = new Impl_c;
}

ExceptionsTrieGen_c::~ExceptionsTrieGen_c()
{
	delete m_pImpl;
}

void ExceptionsTrieGen_c::FoldSpace ( char* s ) const
{
	m_pImpl->FoldSpace ( s );
}

bool ExceptionsTrieGen_c::ParseLine ( char* sBuffer, CSphString& sError )
{
	return m_pImpl->ParseLine ( sBuffer, sError );
}

ExceptionsTrie_c* ExceptionsTrieGen_c::Build()
{
	return m_pImpl->Build();
}
