//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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


//////////////////////////////////////////////////////////////////////////
/// simple linked list
//////////////////////////////////////////////////////////////////////////
struct ListNode_t
{
	ListNode_t* m_pPrev = nullptr;
	ListNode_t* m_pNext = nullptr;
};

/// wrap raw void* into ListNode_t to store it in List_t
struct ListedData_t: public ListNode_t
{
	const void* m_pData = nullptr;

	ListedData_t() = default;
	explicit ListedData_t ( const void* pData )
		: m_pData ( pData )
	{}
};


/// Simple linked list.
class List_t
{
public:
	List_t();

	/// Append the node to the tail
	void Add ( ListNode_t* pNode );
	void HardReset();
	void Remove ( ListNode_t* pNode );
	inline int GetLength() const;
	inline const ListNode_t* Begin() const;
	inline const ListNode_t* End() const;

	class Iterator_c
	{
		ListNode_t* m_pIterator = nullptr;
		ListNode_t* m_pNext = nullptr; // backup since original m.b. corrupted by dtr/free
	public:
		explicit Iterator_c ( ListNode_t* pIterator = nullptr );
		ListNode_t& operator*();
		Iterator_c& operator++();
		bool operator!= ( const Iterator_c& rhs ) const;
	};

	// c++11 style iteration
	Iterator_c begin() const;
	Iterator_c end() const;

private:
	ListNode_t m_tStub; ///< stub node
	volatile int m_iCount;
};

#include "slist_impl.h"
