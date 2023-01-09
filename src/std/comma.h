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

/// text delimiter
/// returns "" first time, then defined delimiter starting from 2-nd call
/// NOTE that using >1 call in one chain like out << comma << "foo" << comma << "bar" is NOT defined,
/// since order of calling 2 commas here is undefined (so, you may take "foo, bar", but may ", foobar" also).
/// Use out << comma << "foo"; out << comma << "bar"; in the case

class Comma_c
{
protected:
	Str_t m_sComma = dEmptyStr;
	bool m_bStarted = false;

public:
	// standalone - cast to 'Str_t' when necessary
	explicit Comma_c ( const char* sDelim = nullptr );
	explicit Comma_c ( Str_t sDelim );

	Comma_c ( const Comma_c& rhs ) = default;
	Comma_c ( Comma_c&& rhs ) noexcept = default;
	Comma_c& operator= ( Comma_c rhs );

	void Swap ( Comma_c& rhs ) noexcept;

	inline bool Started() const { return m_bStarted; }

	operator Str_t();
};

#include "comma_impl.h"