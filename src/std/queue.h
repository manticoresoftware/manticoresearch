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

/////////////////////////////////////////////////////////////////////////////

/// generic stateless priority queue
template<typename T, typename COMP>
class CSphQueue
{
public:
	explicit CSphQueue ( int iSize );
	~CSphQueue();

	void Reset ( int iSize );
	void Clear();

	/// add entry to the queue
	bool Push ( const T& tEntry );

	/// remove root (ie. top priority) entry
	void Pop();

	/// get entries count
	inline int GetLength() const { return m_iUsed; }

	/// get current root
	const T& Root() const;
	const T& Last() const;

private:
	T* m_pData = nullptr;
	int m_iUsed = 0;
	int m_iSize = 0;
};


#include "queue_impl.h"