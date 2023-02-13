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

#include "attrstub.h"
#include "vector.h"
#include "blobs.h"

bool StrEq ( const char* l, const char* r );
bool StrEqN ( const char* l, const char* r );


/// immutable C string proxy
struct CSphString
{
protected:
	char* m_sValue = nullptr;
	// Empty ("") string optimization.
	static char EMPTY[];

private:
	/// safety gap after the string end; for instance, UTF-8 Russian stemmer
	/// which treats strings as 16-bit word sequences needs this in some cases.
	/// note that this zero-filled gap does NOT include trailing C-string zero,
	/// and does NOT affect strlen() as well.
	static constexpr int SAFETY_GAP = 4;

	void SafeFree();

public:
	CSphString() noexcept = default;

	// take a note this is not an explicit constructor
	// so a lot of silent constructing and deleting of strings is possible
	// Example:
	// SmallStringHash_T<int> hHash;
	// ...
	// hHash.Exists ( "asdf" ); // implicit CSphString construction and deletion here
	CSphString ( const CSphString& rhs );

	CSphString ( CSphString&& rhs ) noexcept
	{
		Swap ( rhs );
	}

	~CSphString()
	{
		SafeFree();
	}

	const char* cstr() const
	{
		return m_sValue;
	}

	const char* scstr() const
	{
		return m_sValue ? m_sValue : EMPTY;
	}

	inline bool operator== ( const char* t ) const
	{
		return StrEq ( t, m_sValue );
	}

	inline bool operator== ( const CSphString& t ) const
	{
		return operator== ( t.cstr() );
	}

	inline bool operator!= ( const CSphString& t ) const
	{
		return !operator== ( t );
	}

	bool operator!= ( const char* t ) const
	{
		return !operator== ( t );
	}

	// compare ignoring case
	inline bool EqN ( const char* t ) const
	{
		return StrEqN ( t, m_sValue );
	}

	inline bool EqN ( const CSphString& t ) const
	{
		return EqN ( t.cstr() );
	}

	CSphString ( const char* szString );

	// create even if source is null/empty (result will be valid string with valid tail of zeros - for parsers)
	enum guarded_e { always_create };
	CSphString ( const char* szString, guarded_e );

	CSphString ( const char* sValue, int iLen )
	{
		SetBinary ( sValue, iLen );
	}

	CSphString ( Str_t sValue )
	{
		SetBinary ( sValue );
	}

	// pass by value - replaces both copy and move assignments.
	CSphString& operator= ( CSphString rhs )
	{
		Swap ( rhs );
		return *this;
	}

	CSphString SubString ( int iStart, int iCount ) const;

	// tries to reuse memory buffer, but calls Length() every time
	// hope this won't kill performance on a huge strings
	void SetBinary ( const char* sValue, int iLen );
	void SetBinary ( Str_t sValue ) { SetBinary ( sValue.first, sValue.second ); }

	void Reserve ( int iLen );

	const CSphString& SetSprintf ( const char* sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );

	/// format value using provided va_list
	const CSphString& SetSprintfVa ( const char* sTemplate, va_list ap );

	/// \return true if internal char* ptr is null, of value is empty.
	bool IsEmpty() const;

	CSphString& ToLower();

	CSphString& ToUpper();

	void Swap ( CSphString& rhs );

	/// \return true if the string begins with sPrefix
	bool Begins ( const char* sPrefix ) const;

	/// \return true if the string ends with sSuffix
	bool Ends ( const char* sSuffix ) const;

	/// trim leading and trailing spaces
	CSphString& Trim();

	int Length() const;

	/// \return internal string and releases it from being destroyed in d-tr
	char* Leak();

	/// internal string and releases it from being destroyed in d-tr
	void LeakToVec ( CSphVector<BYTE>& dVec );

	/// take string from outside and 'adopt' it as own child.
	void Adopt ( char** sValue );
	void Adopt ( char*&& sValue );

	/// compares using strcmp
	bool operator<( const CSphString& b ) const;

	void Unquote();

	static int GetGap() { return SAFETY_GAP; }

	explicit operator ByteBlob_t() const;
};

/// string swapper
void Swap ( CSphString& v1, CSphString& v2 );

/// directly make formatted string
CSphString SphSprintfVa ( const char* sTemplate, va_list ap );
CSphString SphSprintf ( const char* sTemplate, ... );

/// commonly used
using StrVec_t = CSphVector<CSphString>;
using StrtVec_t = CSphVector<Str_t>;

void ToLower ( Str_t sVal );

#include "string_impl.h"
