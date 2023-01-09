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

#include "thread_annotations.h"

template<typename CONTAINER, typename FILTER>
inline bool all_of ( const CONTAINER& dData, FILTER&& cond ) NO_THREAD_SAFETY_ANALYSIS
{
	for ( const auto& dItem : dData )
		if ( !cond ( dItem ) )
			return false;
	return true;
}

template<typename CONTAINER, typename FILTER>
inline bool any_of ( const CONTAINER& dData, FILTER&& cond ) NO_THREAD_SAFETY_ANALYSIS
{
	for ( const auto& dItem : dData )
		if ( cond ( dItem ) )
			return true;

	return false;
}

template<typename CONTAINER, typename FILTER>
inline bool none_of ( const CONTAINER& dData, FILTER&& cond ) NO_THREAD_SAFETY_ANALYSIS
{
	return !any_of ( dData, std::forward<FILTER> ( cond ) );
}

template<typename CONTAINER, typename FILTER>
inline int64_t count_of ( const CONTAINER& dData, FILTER&& cond ) NO_THREAD_SAFETY_ANALYSIS
{
	int64_t iRes = 0;
	for ( const auto& dItem : dData )
		if ( cond ( dItem ) )
			++iRes;

	return iRes;
}

template<typename CONTAINER, typename ACTION>
void for_each ( CONTAINER& dData, ACTION&& fnAction ) NO_THREAD_SAFETY_ANALYSIS
{
	for ( auto& dItem : dData )
		fnAction ( dItem );
}