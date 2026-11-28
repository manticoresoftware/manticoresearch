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

#include "std/string.h"

enum class SecondaryIndexType_e
{
	NONE,
	FILTER,
	LOOKUP,
	INDEX,
	ANALYZER,
	SORT_BOUND,		///< fullscan ordered by block bounds

	TOTAL
};


struct IndexHint_t
{
	CSphString				m_sIndex;
	SecondaryIndexType_e	m_eType = SecondaryIndexType_e::NONE;
	bool					m_bForce = true;

	bool IsFilterIteratorHint() const { return m_eType!=SecondaryIndexType_e::SORT_BOUND; }
};


class IndexHints_c
{
public:
	bool				Add ( IndexHint_t tHint, CSphString & sError );
	bool				Normalize ( CSphString & sError );

	const IndexHint_t *	GetSortBoundHint() const;
	bool				HasFilterIteratorHints() const;
	bool				HasForcedSecondaryIndex() const;
	bool				AllowsIterator ( const CSphString & sAttr, SecondaryIndexType_e eType, bool & bForce ) const;

	VecTraits_T<const IndexHint_t> GetAll() const	{ return { m_dHints.Begin(), m_dHints.GetLength() }; }
	void				Reserve ( int iCount )			{ m_dHints.Reserve(iCount); }

private:
	CSphVector<IndexHint_t> m_dHints;
};
