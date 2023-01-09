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

#include "ints.h"
#include "string.h"

/////////////////////////////////////////////////////////////////////////////

/// immutable string/int/float variant list proxy
/// used in config parsing
struct CSphVariant
{
protected:
	CSphString m_sValue;
	int m_iValue = 0;
	int64_t m_i64Value = 0;
	float m_fValue = 0.0f;

public:
	CSphVariant* m_pNext = nullptr;
	// tags are used for handling multiple same keys
	bool m_bTag = false; // 'true' means override - no multi-valued; 'false' means multi-valued - chain them
	int m_iTag = 0;		 // stores order like in config file

public:
	/// default ctor
	CSphVariant() = default;

	/// ctor from C string
	explicit CSphVariant ( const char* sString, int iTag = 0 );

	/// copy ctor
	CSphVariant ( const CSphVariant& rhs );

	/// move ctor
	CSphVariant ( CSphVariant&& rhs ) noexcept;

	/// default dtor
	/// WARNING: automatically frees linked items!
	~CSphVariant();

	const char* cstr() const { return m_sValue.cstr(); }

	const CSphString& strval() const { return m_sValue; }
	int intval() const { return m_iValue; }
	int64_t int64val() const { return m_i64Value; }
	float floatval() const { return m_fValue; }

	/// default copy operator
	CSphVariant& operator= ( CSphVariant rhs );

	void Swap ( CSphVariant& rhs ) noexcept;

	bool operator== ( const char* s ) const { return m_sValue == s; }
	bool operator!= ( const char* s ) const { return m_sValue != s; }
};

#include "variant_impl.h"
