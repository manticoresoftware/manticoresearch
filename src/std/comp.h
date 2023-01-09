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

/// generic comparator
template<typename T> struct SphLess_T
{
	inline static bool IsLess ( const T& a, const T& b ) { return a < b; }
};


/// generic comparator
template<typename T> struct SphGreater_T
{
	inline static bool IsLess ( const T& a, const T& b ) { return b < a; }
};


/// generic comparator
template<typename T, typename C>
struct SphMemberLess_T
{
	const T C::*m_pMember;

	explicit SphMemberLess_T ( T C::*pMember );
	inline bool IsLess ( const C& a, const C& b ) const;
};

template<typename T, typename C>
inline SphMemberLess_T<T, C> sphMemberLess ( T C::*pMember );

/// generic comparator initialized by functor
template<typename COMP>
struct SphLesser
{
	COMP m_fnComp;
	explicit SphLesser ( COMP&& fnComp );

	template<typename T>
	bool IsLess ( T&& a, T&& b ) const;
};

// make
template<typename FNCOMP>
SphLesser<FNCOMP> Lesser ( FNCOMP&& fnComp );

//////////////////////////////////////////////////////////////////////////

/// member functor, wraps object member access
template<typename T, typename CLASS>
struct SphMemberFunctor_T
{
	const T CLASS::*m_pMember;

	explicit SphMemberFunctor_T ( T CLASS::*pMember );
	const T& operator() ( const CLASS& arg ) const;
	inline bool IsLess ( const CLASS& a, const CLASS& b ) const;
	inline bool IsEq ( const CLASS& a, T b );
};


/// handy member functor generator
/// this sugar allows you to write like this
/// dArr.Sort ( bind ( &CSphType::m_iMember ) );
/// dArr.BinarySearch ( bind ( &CSphType::m_iMember ), iValue );
template<typename T, typename CLASS>
inline SphMemberFunctor_T<T, CLASS> bind ( T CLASS::*ptr )
{
	return SphMemberFunctor_T<T, CLASS> ( ptr );
}

/// identity functor
template<typename T>
struct SphIdentityFunctor_T
{
	inline const T& operator() ( const T& arg ) const { return arg; }
};

/// equality functor
template<typename T>
struct SphEqualityFunctor_T
{
	inline static bool IsEq ( const T& a, const T& b ) { return a == b; }
};

/// very popular and so, moved here
/// use integer values as hash values (like document IDs, for example)
struct IdentityHash_fn
{
	template<typename INT>
	static inline INT Hash ( INT iValue ) { return iValue; }
};

#include "comp_impl.h"
