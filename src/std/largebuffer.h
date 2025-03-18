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

#include "buffer.h"

//////////////////////////////////////////////////////////////////////////

/// in-memory buffer shared between processes
template<typename T, bool SHARED = false>
class CSphLargeBuffer: public CSphBufferTrait<T>
{
public:

	CSphLargeBuffer() {}
	~CSphLargeBuffer() override;

	/// allocate storage
	bool Alloc ( int64_t iEntries, CSphString& sError );

	/// deallocate storage
	void Reset() override;
};

#include "largebuffer_impl.h"
