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

// magic to determine widest from provided types and initialize whole unions
// for example,
/*
 *	union foo {
 *		BYTE	a;
 *		char	b;
 *		DWORD	c;
 *		WORDID	w;
 *		sphDocid_t d;
 *		void*	p;
 *		WIDEST<BYTE,char,DWORD,WORDID,sphDocid_t,void*>::T _init = 0;
 *	};
 */
template<typename T1, typename T2, bool = ( sizeof ( T1 ) < sizeof ( T2 ) )>
struct WIDER
{
	using T = T2;
};

template<typename T1, typename T2>
struct WIDER<T1, T2, false>
{
	using T = T1;
};

template<typename T1, typename... TYPES>
struct WIDEST
{
	using T = typename WIDER<T1, typename WIDEST<TYPES...>::T>::T;
};

template<typename T1, typename T2>
struct WIDEST<T1, T2>
{
	using T = typename WIDER<T1, T2>::T;
};