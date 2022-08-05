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

#include <cassert>

inline List_t::List_t()
{
	m_tStub.m_pPrev = &m_tStub;
	m_tStub.m_pNext = &m_tStub;
	m_iCount = 0;
}


/// Append the node to the tail
inline void List_t::Add ( ListNode_t* pNode )
{
	if ( !pNode )
		return;
	assert ( !pNode->m_pNext && !pNode->m_pPrev );
	pNode->m_pNext = m_tStub.m_pNext;
	pNode->m_pPrev = &m_tStub;
	m_tStub.m_pNext->m_pPrev = pNode;
	m_tStub.m_pNext = pNode;

	++m_iCount;
}


inline void List_t::HardReset()
{
	m_tStub.m_pPrev = &m_tStub;
	m_tStub.m_pNext = &m_tStub;
	m_iCount = 0;
}


inline void List_t::Remove ( ListNode_t* pNode )
{
	if ( !pNode )
		return;
	assert ( pNode->m_pNext && pNode->m_pPrev );
	pNode->m_pNext->m_pPrev = pNode->m_pPrev;
	pNode->m_pPrev->m_pNext = pNode->m_pNext;
	pNode->m_pNext = nullptr;
	pNode->m_pPrev = nullptr;

	--m_iCount;
}


inline int List_t::GetLength() const
{
	return m_iCount;
}


inline const ListNode_t* List_t::Begin() const
{
	return m_tStub.m_pNext;
}


inline const ListNode_t* List_t::End() const
{
	return &m_tStub;
}

inline List_t::Iterator_c::Iterator_c ( ListNode_t* pIterator  )
	: m_pIterator ( pIterator )
{
	if ( m_pIterator )
		m_pNext = m_pIterator->m_pNext;
}


inline ListNode_t& List_t::Iterator_c::operator*()
{
	return *m_pIterator;
}


inline List_t::Iterator_c& List_t::Iterator_c::operator++()
{
	assert ( m_pNext );
	m_pIterator = m_pNext;
	m_pNext = m_pIterator->m_pNext;
	return *this;
}


inline bool List_t::Iterator_c::operator!= ( const List_t::Iterator_c& rhs ) const
{
	return m_pIterator != rhs.m_pIterator;
}


// c++11 style iteration
inline List_t::Iterator_c List_t::begin() const
{
	return Iterator_c ( m_tStub.m_pNext );
}


inline List_t::Iterator_c List_t::end() const
{
	return Iterator_c ( const_cast<ListNode_t*> ( &m_tStub ) );
}

