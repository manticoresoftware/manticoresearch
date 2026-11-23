//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "indexhints.h"

#include "std/sort.h"
#include "std/uniq.h"


bool IndexHints_c::Add ( IndexHint_t tHint, CSphString & sError )
{
	if ( tHint.m_eType==SecondaryIndexType_e::SORT_BOUND )
	{
		const auto * pHint = GetSortBoundHint();
		if ( pHint )
		{
			if ( pHint->m_sIndex==tHint.m_sIndex && pHint->m_bForce==tHint.m_bForce )
				return true;

			sError = "conflicting sort-bound hints";
			return false;
		}
	}

	m_dHints.Add ( std::move(tHint) );
	return true;
}


struct HintComp_fn
{
	bool IsLess ( const IndexHint_t & tA, const IndexHint_t & tB ) const
	{
		int iCmp = strcmp ( tA.m_sIndex.cstr(), tB.m_sIndex.cstr() );
		if ( iCmp )
			return iCmp<0;

		if ( tA.m_eType!=tB.m_eType )
			return tA.m_eType<tB.m_eType;

		return tA.m_bForce<tB.m_bForce;
	}

	bool IsEq ( const IndexHint_t & tA, const IndexHint_t & tB ) const
	{
		return tA.m_sIndex==tB.m_sIndex && tA.m_eType==tB.m_eType && tA.m_bForce==tB.m_bForce;
	}
};


bool IndexHints_c::Normalize ( CSphString & sError )
{
	sphSort ( m_dHints.Begin(), m_dHints.GetLength(), HintComp_fn() );
	m_dHints.Shrink ( sphUniq ( m_dHints.Begin(), m_dHints.GetLength(), HintComp_fn() ) );

	for ( int i = 1; i < m_dHints.GetLength(); i++ )
		if ( m_dHints[i-1].m_sIndex==m_dHints[i].m_sIndex && m_dHints[i-1].m_eType==m_dHints[i].m_eType )
		{
			sError.SetSprintf ( "conflicting hints specified for table '%s'", m_dHints[i-1].m_sIndex.cstr() );
			return false;
		}

	return true;
}


const IndexHint_t * IndexHints_c::GetSortBoundHint() const
{
	for ( const auto & tHint : m_dHints )
		if ( tHint.m_eType==SecondaryIndexType_e::SORT_BOUND )
			return &tHint;

	return nullptr;
}


bool IndexHints_c::HasFilterIteratorHints() const
{
	return m_dHints.any_of ( []( const IndexHint_t & tHint ) { return tHint.IsFilterIteratorHint(); } );
}


bool IndexHints_c::HasForcedSecondaryIndex() const
{
	return m_dHints.any_of ( []( const IndexHint_t & tHint ) { return tHint.m_bForce && tHint.m_eType==SecondaryIndexType_e::INDEX; } );
}


bool IndexHints_c::AllowsIterator ( const CSphString & sAttr, SecondaryIndexType_e eType, bool & bForce ) const
{
	bForce = false;

	for ( const auto & tHint : m_dHints )
		if ( tHint.m_sIndex==sAttr && tHint.m_eType==eType )
		{
			bForce = tHint.m_bForce;
			return bForce;
		}

	return true;
}
