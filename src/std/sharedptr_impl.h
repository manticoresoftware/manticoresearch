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


#include "sharedptr.h"

template<typename T, typename DELETER, typename REFCOUNTED>
template<typename DELL, bool STATEFUL_DELETER>
template<typename DEL>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedState_T<DELL, STATEFUL_DELETER>::SharedState_T ( DEL&& fnDelete )
	: m_fnDelete ( std::forward<DEL> ( fnDelete ) )
{}


template<typename T, typename DELETER, typename REFCOUNTED>
template<typename DELL, bool STATEFUL_DELETER>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedState_T<DELL, STATEFUL_DELETER>::~SharedState_T ()
{
	m_fnDelete.Delete ( (void*)m_pPtr );
}


template<typename T, typename DELETER, typename REFCOUNTED>
template<typename DELL>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedState_T<DELL, false>::~SharedState_T()
{
	DELL::Delete ( (void*)m_pPtr );
	m_pPtr = nullptr;
}


template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedPtr_T ( SharedPtr_T::PTR pPtr )
	: m_tState ( new SharedState_t() )
{
	m_tState->m_pPtr = pPtr;
}

template<typename T, typename DELETER, typename REFCOUNTED>
template<typename DEL>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedPtr_T ( SharedPtr_T::PTR pPtr, DEL&& fn )
	: m_tState ( new SharedState_t ( std::forward<DEL> ( fn ) ) )
{
	m_tState->m_pPtr = pPtr;
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedPtr_T ( const SharedPtr_T& rhs )
	: m_tState ( rhs.m_tState )
{
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>::SharedPtr_T ( SharedPtr_T&& rhs ) noexcept
{
	Swap ( rhs );
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>& SharedPtr_T<T, DELETER, REFCOUNTED>::operator= ( SharedPtr_T rhs )
{
	Swap ( rhs );
	return *this;
}

template<typename T, typename DELETER, typename REFCOUNTED>
void SharedPtr_T<T, DELETER, REFCOUNTED>::Swap ( SharedPtr_T& rhs ) noexcept
{
	::Swap ( m_tState, rhs.m_tState );
}

template<typename T, typename DELETER, typename REFCOUNTED>
typename SharedPtr_T<T, DELETER, REFCOUNTED>::PTR SharedPtr_T<T, DELETER, REFCOUNTED>::operator->() const
{
	return m_tState->m_pPtr;
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>::operator bool() const
{
	return m_tState && m_tState->m_pPtr != nullptr;
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>::operator PTR() const
{
	return m_tState ? m_tState->m_pPtr : nullptr;
}

template<typename T, typename DELETER, typename REFCOUNTED>
SharedPtr_T<T, DELETER, REFCOUNTED>& SharedPtr_T<T, DELETER, REFCOUNTED>::operator= ( SharedPtr_T::PTR pPtr )
{
	m_tState = new SharedState_t;
	m_tState->m_pPtr = pPtr;
	return *this;
}
