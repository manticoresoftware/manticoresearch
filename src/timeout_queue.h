//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "std/vector.h"
#include "std/string.h"

#include <functional>

/// member type for priority queue used for timeout task managing
struct EnqueuedTimeout_t
{
	int64_t m_iTimeoutTimeUS = -1;	// active timeout (used for bin heap morph in comparing)
	mutable int m_iTimeoutIdx = -1; // idx inside timeouts bin heap (or -1 if not there), internal
};

/// priority queue for timeouts - as CSphQueue,
/// but specific (can resize, stores internal index in an object)
class TimeoutQueue_c final
{
	CSphTightVector<EnqueuedTimeout_t*> m_dQueue;

	void ShiftUp ( int iHole );
	void ShiftDown ( int iHole );
	void Push ( EnqueuedTimeout_t* pTask );

public:
	/// remove root (ie. top priority) entry
	void Pop();

	/// add new, or change already added entry
	void Change ( EnqueuedTimeout_t* pTask );

	/// erase elem (uses stored m_iTimeoutIdx)
	void Remove ( EnqueuedTimeout_t* pTask );

	inline bool IsEmpty() const
	{
		return m_dQueue.IsEmpty();
	}

	/// get minimal (root) elem
	EnqueuedTimeout_t* Root() const;

	CSphString DebugDump ( const char* sPrefix ) const;
	void DebugDump ( const std::function<void ( EnqueuedTimeout_t* )>& fcb ) const;
};
