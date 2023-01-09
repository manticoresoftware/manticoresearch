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

#include "blobs.h"
#include "ints.h"
#include "generics.h"
#include "string.h"
#include "variant.h"
#include "comma.h"

#include <type_traits>
#include <functional>

using StrBlock_t = std::tuple<Str_t, Str_t, Str_t>;

// common patterns
const StrBlock_t dEmptyBl { dEmptyStr, dEmptyStr, dEmptyStr }; // empty
const StrBlock_t dBracketsComma { FROMS(","), FROMS("("), FROMS(")") }; // collection in brackets, comma separated

/// string builder
/// somewhat quicker than a series of SetSprintf()s
/// lets you build strings bigger than 1024 bytes, too
class StringBuilder_c : public ISphNoncopyable
{
	class LazyComma_c;

public:
		// creates and m.b. start block
						explicit StringBuilder_c ( const char * sDel = nullptr, const char * sPref = nullptr, const char * sTerm = nullptr );
						StringBuilder_c ( StringBuilder_c&& rhs ) noexcept;
						~StringBuilder_c ();

	void				Swap ( StringBuilder_c& rhs ) noexcept;

	// reset to initial state
	void				Clear();

	// rewind to initial state, but don't clear the commas
	void 				Rewind();

	// get current build value
	const char *		cstr() const { return m_szBuffer ? m_szBuffer : ""; }
	explicit operator	CSphString() const { return { cstr() }; }
	explicit operator	Str_t() const { return m_szBuffer ? Str_t { m_szBuffer, m_iUsed } : dEmptyStr; }
	explicit operator	ByteBlob_t() const { return m_szBuffer ? ByteBlob_t { (const BYTE*) m_szBuffer, m_iUsed } : S2B(dEmptyStr); }

	// move out (de-own) value
	BYTE *				Leak();
	void				MoveTo ( CSphString &sTarget ); // leak to string

	// get state
	bool				IsEmpty () const { return !m_szBuffer || m_szBuffer[0]=='\0'; }
	inline int			GetLength () const { return m_iUsed; }

	// different kind of fullfillments
	StringBuilder_c &	AppendChunk ( const Str_t& sChunk, char cQuote = '\0' );
	StringBuilder_c &	AppendString ( const CSphString & sText, char cQuote = '\0' );
	StringBuilder_c &	AppendString ( const char* szText, char cQuote = '\0' );
	StringBuilder_c &	AppendString ( Str_t sText, char cQuote = '\0' );

	StringBuilder_c &	operator = ( StringBuilder_c rhs ) noexcept;
	template<typename T>
	StringBuilder_c &	operator += ( const T * pVal );
	StringBuilder_c &	operator += ( const char* sText );
	StringBuilder_c &	operator << ( const Str_t& sChunk );
	StringBuilder_c &	operator << ( const VecTraits_T<char> &sText );

	/*
	 * Two guys below distinguishes `const char*` param from `const char[]`.
	 * First one implies strlen() and uses it
	 * Second implies length to be known at compile time, and avoids strlen().
	 * So, << "bar" - will be faster, as we know size of that literal in compile time.
	 * Look at TEST ( functions, stringbuilder_templated ) for experiments.
	 */
	template<size_t N>
	StringBuilder_c &	operator << ( char const(& sLiteral)[N] ) { return *this << Str_t { sLiteral, N-1 }; }
	template<typename CHAR>
	StringBuilder_c &	operator << ( const CHAR * const& sText ) { return *this += sText; }

	StringBuilder_c &	operator << ( char cChar ) { return *this << Str_t {&cChar,1}; }
	StringBuilder_c &	operator << ( const CSphString &sText ) { return *this += sText.cstr (); }
	StringBuilder_c &	operator << ( const CSphVariant &sText )	{ return *this += sText.cstr (); }
	StringBuilder_c &	operator << ( const StringBuilder_c &sText )	{ return *this << (Str_t) sText; }
	StringBuilder_c &	operator << ( Comma_c& dComma ) { return *this << (Str_t)dComma; }

	StringBuilder_c &	operator << ( int iVal );
	StringBuilder_c &	operator << ( long iVal );
	StringBuilder_c &	operator << ( long long iVal );

	StringBuilder_c &	operator << ( unsigned int uVal );
	StringBuilder_c &	operator << ( unsigned long uVal );
	StringBuilder_c &	operator << ( unsigned long long uVal );

	StringBuilder_c &	operator << ( float fVal );
	StringBuilder_c &	operator << ( double fVal );
	StringBuilder_c &	operator << ( bool bVal );

	// support for sph::Sprintf - emulate POD 'char*'
	inline StringBuilder_c &	operator ++() { GrowEnough ( 1 ); ++m_iUsed; return *this; }
	inline void					operator += (int i) { GrowEnough ( i ); m_iUsed += i; }

	// append 1 char despite any blocks.
	inline void			RawC ( char cChar ) { GrowEnough ( 1 ); *end () = cChar; ++m_iUsed; }
	void				AppendRawChunk ( Str_t sText ); // append without any commas
	StringBuilder_c &	SkipNextComma();
	StringBuilder_c &	AppendName ( const char * szName, bool bQuoted=true ); // append "szName":
	StringBuilder_c &	AppendName ( const Str_t& sName, bool bQuoted = true );   // append "sName":

	// these use standard sprintf() inside
	StringBuilder_c &	vAppendf ( const char * sTemplate, va_list ap );
	StringBuilder_c &	Appendf ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );

	// these use or own implementation sph::Sprintf which provides also some sugar
	StringBuilder_c &	vSprintf ( const char * sTemplate, va_list ap );
	StringBuilder_c &	Sprintf ( const char * sTemplate, ... );

	// arbitrary output all params according to their << implementations (inlined in compile time).
	template<typename... Params>
	StringBuilder_c &	Sprint ( const Params&... tParams );

	// comma manipulations
	// start new comma block; return index of it (for future possible reference in FinishBlocks())
	int					StartBlock ( const char * sDel = ", ", const char * sPref = nullptr, const char * sTerm = nullptr );
	int 				StartBlock( const StrBlock_t& dBlock );
	int					MuteBlock ();

	// finish and close last opened comma block.
	// bAllowEmpty - close empty block output nothing(default), or prefix/suffix pair (if any).
	void				FinishBlock ( bool bAllowEmpty = true );

	// finish and close all blocks including pLevels (by default - all blocks)
	void				FinishBlocks ( int iLevels = 0, bool bAllowEmpty = true );

	inline char *		begin() const { return m_szBuffer; }
	inline char *		end () const { return m_szBuffer + m_iUsed; }

	// shrink, if necessary, to be able to fit at least iLen more chars
	void GrowEnough ( int iLen );

	template<typename INT, int iBase=10, int iWidth=0, int iPrec=0, char cFill=' '>
	void NtoA ( INT uVal );
//	template<int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' '>
//	void NtoA ( int64_t tVal );
	void FtoA ( float fVal );
	void DtoA ( double fVal );

	template<typename INT, int iPrec>
	void IFtoA ( FixedFrac_T<INT, iPrec> tVal );

protected:
	static constexpr BYTE GROW_STEP = 64; // how much to grow if no space left

	char *			m_szBuffer = nullptr;
	int				m_iSize = 0;
	int				m_iUsed = 0;
	CSphVector<LazyComma_c> m_dDelimiters;

	void			Grow ( int iLen ); // unconditionally shrink enough to place at least iLen more bytes

	void InitAddPrefix();
	const Str_t & Delim ();

private:
	void			NewBuffer ();
	void			InitBuffer ();

	// RAII comma for frequently used pattern of pushing into StringBuilder many values separated by ',', ';', etc.
	// When in scope, inject prefix before very first item, or delimiter before each next.
	class LazyComma_c : public Comma_c
	{
		bool m_bSkipNext = false;

	public:
		Str_t m_sPrefix = dEmptyStr;
		Str_t m_sSuffix = dEmptyStr;

		// c-tr for managed - linked StringBuilder will inject RawComma() on each call, terminator at end
		LazyComma_c ( const char * sDelim, const char * sPrefix, const char * sTerm );
		explicit LazyComma_c( const StrBlock_t& dBlock );
		LazyComma_c () = default;
		LazyComma_c ( const LazyComma_c & ) = default;
		LazyComma_c ( LazyComma_c && ) noexcept = default;
		LazyComma_c& operator= (LazyComma_c rhs)
		{
			Swap(rhs);
			return *this;
		}

		void Swap ( LazyComma_c & rhs ) noexcept;
		const Str_t & RawComma ( const std::function<void ()> & fnAddNext );

		void SkipNext ()
		{
			m_bSkipNext = true;
		}
	};
};

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const CSphNamedInt& tValue );
StringBuilder_c& operator<< ( StringBuilder_c& tOut, timespan_t tVal );
StringBuilder_c& operator<< ( StringBuilder_c& tOut, timestamp_t tVal );

template<typename INT, int iPrec>
StringBuilder_c& operator<< ( StringBuilder_c& tOut, FixedFrac_T<INT, iPrec>&& tVal );

template<typename INT, int iBase, int iWidth, int iPrec, char cFill>
StringBuilder_c& operator<< ( StringBuilder_c& tOut, FixedNum_T<INT, iBase, iWidth, iPrec, cFill>&& tVal );

// helpers
inline void Grow ( StringBuilder_c& tBuilder, int iInc )
{
	tBuilder.GrowEnough ( iInc );
}

inline char* Tail ( StringBuilder_c& tBuilder )
{
	return tBuilder.end();
}


#include "stringbuilder_impl.h"
