//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "match.h"
#include "std/string.h"
#include "std/vectraits.h"

struct CSphQuery;
class ISphSchema;
class ISphMatchSorter;
class ISphRanker;
class CSphQueryResultMeta;
class SortBoundFilter_c;
struct CSphMultiQueryArgs;

class SortBound_c : ISphNoncopyable
{
public:
	// Resolve a supported sort key and clear the previous threshold
	bool		Setup ( const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphMatchSorter & tSorter );

	// Copy a newly available or stronger threshold; return false when unavailable or unchanged.
	bool		Update();

	const CSphString & GetAttr() const	{ return m_sAttr; }
	bool		IsDescending() const	{ return m_bDescending; }
	bool		IsSet() const			{ return m_bSet; }
	SphAttr_t	GetValue() const		{ assert ( m_bSet ); return m_tValue; }

private:
	const ISphMatchSorter * m_pSorter = nullptr;
	CSphString		m_sAttr;
	CSphAttrLocator	m_tLocator;
	SphAttr_t		m_tValue = 0;
	bool			m_bDescending = false;
	bool			m_bSet = false;
};

bool CanUseSortBound ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, bool bRanklessCount );
void FinalizeSortBound ( const SortBoundFilter_c & tFilter, const SortBound_c & tBound, ISphRanker & tRanker, CSphQueryResultMeta & tMeta );
