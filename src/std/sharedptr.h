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

#include "refcounted_mt.h"
#include "deleter.h"
#include <type_traits>
#include <functional>
#include <utility>

/// shared pointer for any object, managed by refcount
template < typename T, typename DELETER, typename REFCOUNTED = ISphRefcountedMT >
class SharedPtr_T
{
	using PTR = T*;
	template <typename DELL, bool STATEFUL_DELETER = std::is_member_function_pointer<decltype ( &DELL::Delete )>::value>
	struct SharedState_T : public REFCOUNTED
	{
		PTR m_pPtr { nullptr };
		DELL m_fnDelete;

		SharedState_T() noexcept = default;
		template<typename DEL>	explicit SharedState_T ( DEL&& fnDelete );
		~SharedState_T() override;
	};

	template <typename DELL>
	struct SharedState_T<DELL, false> : public REFCOUNTED
	{
		PTR m_pPtr { nullptr };
		~SharedState_T() override;
	};

	using SharedState_t = SharedState_T<DELETER>;
	using StatePtr = CSphRefcountedPtr<SharedState_t>;

	StatePtr m_tState;

public:
	///< default ctr (for vectors)
	explicit SharedPtr_T () = default;

	/// construction from raw pointer, creates new shared state!
	explicit SharedPtr_T ( PTR pPtr );

	template <typename DEL>
	SharedPtr_T ( PTR pPtr, DEL&& fn );
	SharedPtr_T ( const SharedPtr_T& rhs );
	SharedPtr_T ( SharedPtr_T&& rhs ) noexcept;

	SharedPtr_T& operator= ( SharedPtr_T rhs );
	void Swap ( SharedPtr_T& rhs ) noexcept;

	PTR	operator -> () const;
		explicit operator bool() const;
		operator PTR () const;

public:

	/// assignment of a raw pointer
	SharedPtr_T & operator = ( PTR pPtr );
};

template <typename T, typename REFCOUNTED = ISphRefcountedMT>
using SharedPtr_t = SharedPtr_T<T, Deleter_T<T*, ETYPE::SINGLE>, REFCOUNTED>;

template<typename T, typename REFCOUNTED = ISphRefcountedMT>
using SharedPtrArr_t = SharedPtr_T<T, Deleter_T<T*, ETYPE::ARRAY>, REFCOUNTED>;

template<typename T, typename DELETER=std::function<void(T*)>, typename REFCOUNTED = ISphRefcountedMT>
using SharedPtrCustom_t = SharedPtr_T<T, CustomDeleter_T<T*, DELETER>, REFCOUNTED>;


#include "sharedptr_impl.h"