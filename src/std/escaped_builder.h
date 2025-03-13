//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "ints.h"
#include "stringbuilder.h"

struct EqualQuotator_t
{
	// returns N of additional chars need to escape
	static constexpr BYTE EscapingSpace ( BYTE c ) { return 0; }
};

struct EscapeQuotator_t
{
	static constexpr BYTE EscapingSpace ( BYTE c )
	{
		return ( c == '\\' || c == '\'' ) ? 1 : 0;
	}
};

struct SqlQuotator_t
{
	static constexpr BYTE EscapingSpace ( BYTE c )
	{
		return ( c == '\\' || c == '\'' || c == '\t' ) ? 1 : 0;
	}
};

struct FixupSpace_t
{
	// replaces \t, \n, \r into spaces
	static constexpr void FixupSpace ( BYTE*& pOut, BYTE c )
	{
		alignas ( 16 ) constexpr BYTE dSpacesLookupTable[] = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, ' ', ' ', 0x0b, 0x0c, ' ', 0x0e, 0x0f };
		*pOut++ = ( c & 0xF0 ) ? c : dSpacesLookupTable[c];
	}
};

template<typename QUOTATOR = EqualQuotator_t, typename FIXUP = FixupSpace_t>
struct BaseQuotation_T : public QUOTATOR, public FIXUP
{
	// represents char for quote
	static const char cQuote = '\'';

	static void EscapeChar ( BYTE*& pOut, BYTE c )
	{
		if ( QUOTATOR::EscapingSpace ( c ) )
			*pOut++ = '\\';
		*pOut++ = c;
	}

	static void EscapeCharWithSpaces ( BYTE*& pOut, BYTE c )
	{
		if ( QUOTATOR::EscapingSpace ( c ) )
		{
			pOut[0] = '\\';
			pOut[1] = c;
			pOut += 2;
		} else
			FIXUP::FixupSpace ( pOut, c );
	}
};


namespace EscBld
{ // what kind of changes will do AppendEscaped of escaped string builder:
enum eAct : BYTE {
	eNone = 0,		 // [comma,] append raw text without changes
	eFixupSpace = 1, // [comma,] change \t, \n, \r into spaces
	eEscape = 2,	 // [comma,] all escaping according to provided interface
	eAll = 3,		 // [comma,] escape and change spaces
	eSkipComma = 4,	 // force to NOT prefix comma (if any active)
	eSkipQuotes = 8, // don't enclose text into quotes
	eNoLimit = 16,	 // internal - set if iLen is not set (i.e. -1). To convert conditions into switch cases
};
}

template<typename Q = BaseQuotation_T<>>
class EscapedStringBuilder_T: public StringBuilder_c
{
	bool AppendEmpty ( const char* sText );
	void AppendEmptyQuotes();
	bool AppendEmptyEscaped ( const char* sText );

public:
	// dedicated EscBld::eEscape | EscBld::eSkipComma
	void AppendEscapedSkippingComma ( const char* sText );
	void AppendEscapedSkippingCommaNoQuotes ( const char* sText );

	// dedicated EscBld::eEscape with comma
	void AppendEscapedWithComma ( const char* sText );
	void AppendEscapedWithCommaNoQuotes ( const char* sText );

	// dedicated EscBld::eEscape with comma with external len
	void AppendEscapedWithComma ( const char* sText, int iLen );
	void AppendEscapedWithCommaNoQuotes ( const char* sText, int iLen );

	// dedicated EscBld::eFixupSpace
	void FixupSpacesAndAppend ( const char* sText );

	// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape )
	void FixupSpacedAndAppendEscaped ( const char* sText );
	void FixupSpacedAndAppendEscapedNoQuotes ( const char* sText );

	// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape ) with external len
	void FixupSpacedAndAppendEscaped ( const char* sText, int iLen );
	void FixupSpacedAndAppendEscapedNoQuotes ( const char* sText, int iLen );

	// generic implementation. Used this way in tests. For best performance consider to use specialized versions
	// (see selector switch inside) directly.
	void AppendEscaped ( const char* sText, BYTE eWhat = EscBld::eAll, int iLen = -1 );

	EscapedStringBuilder_T& SkipNextComma();
	EscapedStringBuilder_T& AppendName ( const char* sName, bool bQuoted = true );

private:
	template<bool BQUOTE = true>
	void AppendEscapedSkippingCommaT ( const char* sText );
	template<bool BQUOTE = true>
	void AppendEscapedWithCommaT ( const char* sText );
	template <bool BQUOTE = true>
	void AppendEscapedWithCommaT ( const char* sText, int iLen );
	template<bool BQUOTE = true>
	void FixupSpacedAndAppendEscapedT ( const char* sText );
	template<bool BQUOTE = true>
	void FixupSpacedAndAppendEscapedT ( const char* sText, int iLen );
};

#include "escaped_builder_impl.h"
