//
// Copyright (c) 2008-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

//#include "charset_definition_parser.h"
#include "tok_internals.h"

CSphVector<CharsetAlias_t> CSphCharsetDefinitionParser::m_dCharsetAliases;

const CSphVector<CharsetAlias_t>& CSphCharsetDefinitionParser::GetCharsetAliases()
{
	return m_dCharsetAliases;
}

CSphString CSphCharsetDefinitionParser::GetLastError()
{
	return (CSphString)m_sError;
}

bool CSphCharsetDefinitionParser::IsEof()
{
	return ( *m_pCurrent ) == 0;
}


bool CSphCharsetDefinitionParser::CheckEof()
{
	if ( !IsEof() )
		return false;

	Error ( "unexpected end of line" );
	return true;
}


bool CSphCharsetDefinitionParser::Error ( const char* szMessage )
{
	m_sError.Clear();
	m_sError << szMessage << " near '" << m_pCurrent << "'";
	return false;
}


inline static int HexDigit ( int c )
{
	if ( c >= '0' && c <= '9' )
		return c - '0';
	if ( c >= 'a' && c <= 'f' )
		return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' )
		return c - 'A' + 10;
	return 0;
}


void CSphCharsetDefinitionParser::SkipSpaces()
{
	while ( ( *m_pCurrent ) && isspace ( (BYTE)*m_pCurrent ) )
		++m_pCurrent;
}


int CSphCharsetDefinitionParser::ParseCharsetCode()
{
	const char* p = m_pCurrent;
	int iCode = 0;

	if ( p[0] == 'U' && p[1] == '+' )
	{
		p += 2;
		while ( isxdigit ( *p ) )
		{
			iCode = iCode * 16 + HexDigit ( *p++ );
		}
		while ( isspace ( *p ) )
			++p;

	} else
	{
		if ( ( *(const BYTE*)p ) < 32 || ( *(const BYTE*)p ) > 127 )
		{
			Error ( "non-ASCII characters not allowed, use 'U+00AB' syntax" );
			return -1;
		}

		iCode = *p++;
		while ( isspace ( *p ) )
			++p;
	}

	m_pCurrent = p;
	return iCode;
}

bool AddRange ( CSphRemapRange tRange, CSphVector<RemapRangeTagged_t>& dRanges, CSphString* pError )
{
	if ( tRange.m_iRemapStart < 0x20 )
	{
		if ( pError )
			pError->SetSprintf ( "dest range (U+%x) below U+20, not allowed", tRange.m_iRemapStart );
		return false;
	}

	dRanges.Add ( RemapRangeTagged_t { tRange } );
	dRanges.Last().m_iTag = dRanges.GetLength();
	return true;
}

bool CSphCharsetDefinitionParser::AddRange ( CSphRemapRange tRange, CSphVector<RemapRangeTagged_t>& dRanges )
{
	CSphString sError;
	if ( !::AddRange ( tRange, dRanges, &sError ) )
	{
		Error ( sError.cstr() );
		return false;
	}
	return true;
}

// Charsets relocated to folder 'charsets', each one in separate .txt file.
// When you change the content of the folder,
// reconfigure the project with cmake in order to pick the changes.
#include "globalaliases.h"

bool CSphCharsetDefinitionParser::InitCharsetAliasTable ( CSphString& sError )
{
	m_dCharsetAliases.Reset();
	CSphVector<CharsetAlias_t> dAliases;
	CSphVector<char> dConcat;

	const int iTotalChunks = sizeof ( globalaliases ) / sizeof ( globalaliases[0] );
	int iCurAliasChunk = 0;
	for ( const char* szAliasName : globalaliases_names )
	{
		CharsetAlias_t& tCur = dAliases.Add();
		tCur.m_sName = szAliasName;
		tCur.m_iNameLen = tCur.m_sName.Length();

		dConcat.Resize ( 0 );
		while ( iCurAliasChunk < iTotalChunks && globalaliases[iCurAliasChunk] )
		{
			auto iChunkLen = (int)strlen ( globalaliases[iCurAliasChunk] );
			char* szChunk = dConcat.AddN ( iChunkLen );
			memcpy ( szChunk, globalaliases[iCurAliasChunk], iChunkLen );
			++iCurAliasChunk;
		}

		dConcat.Add ( 0 );
		++iCurAliasChunk;

		if ( !sphParseCharset ( dConcat.Begin(), tCur.m_dRemaps, &sError ) )
			return false;
	}

	m_dCharsetAliases.SwapData ( dAliases );
	return true;
}

void RebaseRange ( CSphRemapRange& dRange, int iNewStart )
{
	dRange.m_iRemapStart = iNewStart + dRange.m_iRemapStart - std::exchange ( dRange.m_iStart, iNewStart );
}

void MergeIntersectedRanges ( CSphVector<RemapRangeTagged_t>& dRanges )
{
	// need a stable sort with the desc order of the mappings
	// to keep the last mapping definition and merge into it all next entries (entries defined prior to the last mapping)
	for ( bool bKeepGoing = true; bKeepGoing; )
	{
		bKeepGoing = false;
		// first stage - we flatten all the ranges
		dRanges.Sort();
		CSphVector<RemapRangeTagged_t> dExtraRanges;
		for ( int i = 0; i < dRanges.GetLength() - 1; ++i )
		{
			auto& dFirst = dRanges[i];
			auto& dSecond = dRanges[i+1];

			assert ( dFirst.m_iStart <= dSecond.m_iStart ); // because vec is sorted

			if ( dFirst.m_iEnd < dSecond.m_iStart ) // no intersection, bail
				continue;

			if ( dFirst.m_iStart == dSecond.m_iStart )
			{
				if ( dFirst.m_iEnd == dSecond.m_iEnd )
				{
					assert ( dSecond.m_iTag < dFirst.m_iTag ); // because of sorting order
					dRanges.Remove ( i + 1 ); // ranges are the same - keep one with bigger tag
					--i;
					bKeepGoing = true;
					continue;
				}

				if ( dFirst.m_iEnd > dSecond.m_iEnd )
					std::swap ( dFirst, dSecond );

				assert ( dFirst.m_iEnd < dSecond.m_iEnd );
				// 11
				// 222 => produce 11, 22 and extra tail 2
				dExtraRanges.Add ( dSecond );
				RebaseRange ( dExtraRanges.Last(), dFirst.m_iEnd + 1 );
				dSecond.m_iEnd = dFirst.m_iEnd;
				continue;
			}

			// 111...
			//  22...
			assert ( dFirst.m_iStart < dSecond.m_iStart );
			if ( dFirst.m_iEnd < dSecond.m_iEnd )
			{
				// 111
				//  222 => produce head 1, head 22 and extra middle 11, and tail 2
				dExtraRanges.Add ( dFirst );
				RebaseRange ( dExtraRanges.Last(), dSecond.m_iStart );
				dExtraRanges.Add ( dSecond );
				RebaseRange ( dExtraRanges.Last(), dFirst.m_iEnd + 1 );
				dSecond.m_iEnd = std::exchange ( dFirst.m_iEnd, dSecond.m_iStart - 1 );
				continue;
			}

			if ( dFirst.m_iEnd == dSecond.m_iEnd )
			{
				// 111
				//  22 => produce head 1, and extra middle 11
				dExtraRanges.Add ( dFirst );
				RebaseRange ( dExtraRanges.Last(), dSecond.m_iStart );
				dFirst.m_iEnd = dSecond.m_iStart - 1;
				continue;
			}

			assert ( dFirst.m_iEnd > dSecond.m_iEnd );

			// 1111
			//  22 => produce head 1, extra middle 11 and tail 1
			dExtraRanges.Add ( dFirst );
			dExtraRanges.Last().m_iEnd = dSecond.m_iEnd;
			RebaseRange ( dExtraRanges.Last(), dSecond.m_iStart );
			dExtraRanges.Add ( dFirst );
			RebaseRange ( dExtraRanges.Last(), dSecond.m_iEnd + 1 );
			dFirst.m_iEnd = dSecond.m_iStart - 1;
		}
		dRanges.Append ( dExtraRanges );
		bKeepGoing |= !dExtraRanges.IsEmpty();
	}
	dRanges.Sort();

#define PARANOID 0
#ifndef NDEBUG
#if PARANOID
	for ( int i = 0; i < dRanges.GetLength() - 1; ++i )
	{
		auto& dFirst = dRanges[i];
		auto& dSecond = dRanges[i + 1];
		assert ( dFirst.m_iStart <= dFirst.m_iEnd );
		assert ( dSecond.m_iStart <= dSecond.m_iEnd );
		assert ( dFirst.m_iEnd < dSecond.m_iStart );
	}
#endif
#endif
	// stage 2 - merge sibling ranges. Reuse tag as 'delta'
	for ( auto& dRange : dRanges ) dRange.m_iTag = dRange.m_iRemapStart - dRange.m_iStart;
	for ( int i = 0; i < dRanges.GetLength() - 1; ++i )
	{
		auto& dFirst = dRanges[i];
		auto& dSecond = dRanges[i + 1];
		if ( dFirst.m_iEnd + 1 == dSecond.m_iStart && dFirst.m_iTag == dSecond.m_iTag )
		{
			dFirst.m_iEnd = dSecond.m_iEnd;
			dRanges.Remove ( i + 1 );
			--i;
		}
	}
}

bool CSphCharsetDefinitionParser::Parse ( const char* sConfig, CSphVector<CSphRemapRange>& dRanges )
{
	m_pCurrent = sConfig;
	dRanges.Reset();
	CSphVector<RemapRangeTagged_t> dOrderedRanges;

	AT_SCOPE_EXIT([&dRanges,&dOrderedRanges]() {
		dRanges.Resize ( dOrderedRanges.GetLength() );
		ARRAY_CONSTFOREACH ( i, dOrderedRanges )
			dRanges[i] = (CSphRemapRange)dOrderedRanges[i]; // slice out m_iTag
	});

	// do parse
	while ( *m_pCurrent )
	{
		SkipSpaces();
		if ( IsEof() )
			break;

		// check for stray comma
		if ( *m_pCurrent == ',' )
			return Error ( "stray ',' not allowed, use 'U+002C' instead" );

		// alias
		bool bGotAlias = false;
		ARRAY_FOREACH_COND ( i, m_dCharsetAliases, !bGotAlias )
		{
			const CharsetAlias_t& tCur = m_dCharsetAliases[i];
			bGotAlias = ( strncmp ( tCur.m_sName.cstr(), m_pCurrent, tCur.m_iNameLen ) == 0 && ( !m_pCurrent[tCur.m_iNameLen] || m_pCurrent[tCur.m_iNameLen] == ',' ) );
			if ( !bGotAlias )
				continue;

			// skip to next definition
			m_pCurrent += tCur.m_iNameLen;
			if ( *m_pCurrent && *m_pCurrent == ',' )
				++m_pCurrent;

			for ( const auto& dRemap : tCur.m_dRemaps )
			{
				if ( !AddRange ( dRemap, dOrderedRanges ) )
					return false;
			}
		}
		if ( bGotAlias )
			continue;

		// parse char code
		const char* pStart = m_pCurrent;
		int iStart = ParseCharsetCode();
		if ( iStart < 0 )
			return false;

		// stray char?
		if ( !*m_pCurrent || *m_pCurrent == ',' )
		{
			// stray char
			if ( !AddRange ( { iStart, iStart, iStart }, dOrderedRanges ) )
				return false;

			if ( IsEof() )
				break;
			++m_pCurrent;
			continue;
		}

		// stray remap?
		if ( m_pCurrent[0] == '-' && m_pCurrent[1] == '>' )
		{
			// parse and add
			m_pCurrent += 2;
			int iDest = ParseCharsetCode();
			if ( iDest < 0 )
				return false;
			if ( !AddRange ( { iStart, iStart, iDest }, dOrderedRanges ) )
				return false;

			// it's either end of line now, or must be followed by comma
			if ( *m_pCurrent )
				if ( *m_pCurrent++ != ',' )
					return Error ( "syntax error" );
			continue;
		}

		// range start?
		if ( !( m_pCurrent[0] == '.' && m_pCurrent[1] == '.' ) )
			return Error ( "syntax error" );
		m_pCurrent += 2;

		SkipSpaces();
		if ( CheckEof() )
			return false;

		// parse range end char code
		int iEnd = ParseCharsetCode();
		if ( iEnd < 0 )
			return false;
		if ( iStart > iEnd )
		{
			m_pCurrent = pStart;
			return Error ( "range end less than range start" );
		}

		// stray range?
		if ( !*m_pCurrent || *m_pCurrent == ',' )
		{
			if ( !AddRange ( { iStart, iEnd, iStart }, dOrderedRanges ) )
				return false;

			if ( IsEof() )
				break;
			++m_pCurrent;
			continue;
		}

		// "checkerboard" range?
		if ( m_pCurrent[0] == '/' && m_pCurrent[1] == '2' )
		{
			for ( int i = iStart; i < iEnd; i += 2 )
			{
				if ( !AddRange ( { i, i, i + 1 }, dOrderedRanges ) )
					return false;
				if ( !AddRange ( { i + 1, i + 1, i + 1 }, dOrderedRanges ) )
					return false;
			}

			// skip "/2", expect ","
			m_pCurrent += 2;
			SkipSpaces();
			if ( *m_pCurrent )
				if ( *m_pCurrent++ != ',' )
					return Error ( "expected end of line or ','" );
			continue;
		}

		// remapped range?
		if ( !( m_pCurrent[0] == '-' && m_pCurrent[1] == '>' ) )
			return Error ( "expected end of line, ',' or '-><char>'" );
		m_pCurrent += 2;

		SkipSpaces();
		if ( CheckEof() )
			return false;

		// parse dest start
		const char* pRemapStart = m_pCurrent;
		int iRemapStart = ParseCharsetCode();
		if ( iRemapStart < 0 )
			return false;

		// expect '..'
		if ( CheckEof() )
			return false;
		if ( !( m_pCurrent[0] == '.' && m_pCurrent[1] == '.' ) )
			return Error ( "expected '..'" );
		m_pCurrent += 2;

		// parse dest end
		int iRemapEnd = ParseCharsetCode();
		if ( iRemapEnd < 0 )
			return false;

		// check dest range
		if ( iRemapStart > iRemapEnd )
		{
			m_pCurrent = pRemapStart;
			return Error ( "dest range end less than dest range start" );
		}

		// check for length mismatch
		if ( ( iRemapEnd - iRemapStart ) != ( iEnd - iStart ) )
		{
			m_pCurrent = pStart;
			return Error ( "dest range length must match src range length" );
		}

		// remapped ok
		if ( !AddRange ( { iStart, iEnd, iRemapStart }, dOrderedRanges ) )
			return false;

		if ( IsEof() )
			break;
		if ( *m_pCurrent != ',' )
			return Error ( "expected ','" );
		++m_pCurrent;
	}

	MergeIntersectedRanges ( dOrderedRanges );
	return true;
}

/// public exports
bool sphParseCharset ( const char* szCharset, CSphVector<CSphRemapRange>& dRemaps, CSphString *pError )
{
	CSphCharsetDefinitionParser tParser;

	if ( tParser.Parse ( szCharset, dRemaps ) )
		return true;

	if ( pError )
		*pError = tParser.GetLastError();

	return false;
}

const CSphVector<CharsetAlias_t>& GetCharsetAliases()
{
	return CSphCharsetDefinitionParser::GetCharsetAliases();
}

bool sphInitCharsetAliasTable ( CSphString& sError )
{
	return CSphCharsetDefinitionParser::InitCharsetAliasTable ( sError );
}
