//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

/// Deleter abstraction is used in smart pointer (shared ptr, hazard ptr, etc.)

// tags
enum class ETYPE { SINGLE, ARRAY };

template<typename PTR, ETYPE>
struct Deleter_T
{
	static void Delete ( void* pArg );
};

template<typename PTR>
struct Deleter_T<PTR, ETYPE::ARRAY>
{
	static void Delete ( void* pArg );
};

// stateless (i.e. may use pointer to fn)
template<typename PTR, typename DELETER>
struct StaticDeleter_t
{
	static void Delete ( void * pArg );
};

// statefull (i.e. contains state, implies using of lambda with captures)
template<typename PTR, typename DELETER>
class CustomDeleter_T
{
	DELETER m_dDeleter;
public:

	CustomDeleter_T () = default;
	CustomDeleter_T ( DELETER&& dDeleter ) noexcept;
	void Delete ( void * pArg );
};

#include "deleter_impl.h"