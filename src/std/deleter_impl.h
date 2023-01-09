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

#include <utility>

template<typename PTR, ETYPE TP>
inline void Deleter_T<PTR, TP>::Delete ( void* pArg )
{
	if ( pArg )
		delete (PTR)pArg;
}

template<typename PTR>
inline void Deleter_T<PTR, ETYPE::ARRAY>::Delete ( void* pArg )
{
	if ( pArg )
		delete[] (PTR)pArg;
}



template<typename PTR, typename DELETER>
inline void StaticDeleter_t<PTR, DELETER>::Delete ( void* pArg )
{
	if ( pArg )
		DELETER() ( PTR ( pArg ) );
}


template<typename PTR, typename DELETER>
CustomDeleter_T<PTR, DELETER>::CustomDeleter_T ( DELETER&& dDeleter ) noexcept
	: m_dDeleter { std::forward<DELETER> ( dDeleter ) }
{}


template<typename PTR, typename DELETER>
inline void CustomDeleter_T<PTR, DELETER>::Delete ( void* pArg )
{
	if ( m_dDeleter )
		m_dDeleter ( (PTR)pArg );
}
