//
// Copyright (c) 2021-2024, Manticore Software LTD (https://manticoresearch.com)
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

#include "std/ints.h"
#include <functional>

// up to 12: PQ_ADD_DELETE added
// 13 : changed txn format; now stores total documents also
// 14 : ??
// 15 : remove PQ_ADD, PQ_DELETE, timestamps, RECONFIGURE; ops is 1 byte (unzipped); reorder ops

constexpr unsigned int BINLOG_VERSION = 15;

namespace Binlog {

	/// Bin Log Operation
	enum Blop_e : BYTE
	{
		ADD_INDEX		= 1,
		ADD_CACHE		= 2,
		COMMIT			= 3,
		UPDATE_ATTRS	= 4,
		PQ_ADD_DELETE	= 5,

		TOTAL
	};

	struct CheckTnxResult_t
	{
		bool m_bValid = false;
		bool m_bApply = false;
	};
	using CheckTxn_fn = std::function <CheckTnxResult_t()>;
}
