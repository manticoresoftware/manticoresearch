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

template<typename CONTAINER, typename FILTER>
inline bool all_of ( const CONTAINER& dData, FILTER&& cond );

template<typename CONTAINER, typename FILTER>
inline bool any_of ( const CONTAINER& dData, FILTER&& cond );

template<typename CONTAINER, typename FILTER>
inline bool none_of ( const CONTAINER& dData, FILTER&& cond );

template<typename CONTAINER, typename FILTER>
inline int64_t count_of ( const CONTAINER& dData, FILTER&& cond );

template<typename CONTAINER, typename ACTION>
void for_each ( CONTAINER& dData, ACTION&& fnAction );

#define ARRAY_FOREACH( _index, _array ) \
	for ( int _index = 0; _index < _array.GetLength(); ++_index )

#define ARRAY_FOREACH_COND( _index, _array, _cond ) \
	for ( int _index = 0; _index < _array.GetLength() && ( _cond ); ++_index )

#define ARRAY_CONSTFOREACH( _index, _array ) \
	for ( int _index = 0, _bound = _array.GetLength(); _index < _bound; ++_index )

#define ARRAY_CONSTFOREACH_COND( _index, _array, _cond ) \
	for ( int _index = 0, _bound = _array.GetLength(); _index < _bound && ( _cond ); ++_index )

#include "iterations_impl.h"
