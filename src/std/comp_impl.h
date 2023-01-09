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


template<typename T, typename C>
SphMemberLess_T<T, C>::SphMemberLess_T ( T C::*pMember )
	: m_pMember ( pMember )
{}

template<typename T, typename C>
inline bool SphMemberLess_T<T, C>::IsLess ( const C& a, const C& b ) const
{
	return ( ( &a )->*m_pMember ) < ( ( &b )->*m_pMember );
}

template<typename T, typename C>
inline SphMemberLess_T<T, C> sphMemberLess ( T C::*pMember )
{
	return SphMemberLess_T<T, C> ( pMember );
}

template<typename COMP>
SphLesser<COMP>::SphLesser ( COMP&& fnComp )
	: m_fnComp ( std::forward<COMP> ( fnComp ) )
{}

template<typename COMP>
template<typename T>
bool SphLesser<COMP>::IsLess ( T&& a, T&& b ) const
{
	return m_fnComp ( std::forward<T> ( a ), std::forward<T> ( b ) );
}


template<typename FNCOMP>
SphLesser<FNCOMP> Lesser ( FNCOMP&& fnComp )
{
	return SphLesser<FNCOMP> ( std::forward<FNCOMP> ( fnComp ) );
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename CLASS>
SphMemberFunctor_T<T,CLASS>::SphMemberFunctor_T ( T CLASS::*pMember )
	: m_pMember ( pMember )
{}

template<typename T, typename CLASS>
const T& SphMemberFunctor_T<T, CLASS>::operator() ( const CLASS& arg ) const { return ( &arg )->*m_pMember; }

template<typename T, typename CLASS>
inline bool SphMemberFunctor_T<T, CLASS>::IsLess ( const CLASS& a, const CLASS& b ) const
{
	return ( &a )->*m_pMember < ( &b )->*m_pMember;
}

template<typename T, typename CLASS>
inline bool SphMemberFunctor_T<T, CLASS>::IsEq ( const CLASS& a, T b )
{
	return ( ( &a )->*m_pMember ) == b;
}

