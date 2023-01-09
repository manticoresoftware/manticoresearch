//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include <functional>

// up to 12: PQ_ADD_DELETE added
// 13 : changed txn format; now stores total documents also
constexpr unsigned int BINLOG_VERSION = 13;

namespace Binlog {

	/// Bin Log Operation
	enum Blop_e
	{
		COMMIT			= 1,
		UPDATE_ATTRS	= 2,
		ADD_INDEX		= 3,
		ADD_CACHE		= 4,
		RECONFIGURE		= 5,
		PQ_ADD			= 6,
		PQ_DELETE		= 7,
		PQ_ADD_DELETE	= 8,

		TOTAL
	};

	struct CheckTnxResult_t
	{
		bool m_bValid = false;
		bool m_bApply = false;
	};
	using CheckTxn_fn = std::function <CheckTnxResult_t()>;
}
