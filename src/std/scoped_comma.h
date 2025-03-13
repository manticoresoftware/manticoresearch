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

#include "generics.h"
#include "stringbuilder.h"

class ScopedComma_c: public ISphNoncopyable
{
public:
	ScopedComma_c() = default;
	ScopedComma_c ( StringBuilder_c& tOwner, const char* sDel, const char* sPref = nullptr, const char* sTerm = nullptr, bool bAllowEmpty = true );
	ScopedComma_c ( StringBuilder_c& tOwner, const StrBlock_t& dBlock, bool bAllowEmpty = true );

	ScopedComma_c ( ScopedComma_c&& rhs ) noexcept
	{
		Swap ( rhs );
	}

	ScopedComma_c& operator= ( ScopedComma_c&& rhs ) noexcept
	{
		Swap ( rhs );
		return *this;
	}

	~ScopedComma_c();

	void Swap ( ScopedComma_c& rhs ) noexcept;
	void Init ( StringBuilder_c& tOwner, const char* sDel, const char* sPref = nullptr, const char* sTerm = nullptr );
	StringBuilder_c& Sink() const;

private:
	StringBuilder_c* m_pOwner = nullptr;
	int m_iLevel = 0;
	bool m_bAllowEmpty = true;
};

#include "scoped_comma_impl.h"
