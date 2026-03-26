//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "match_iterator.h"

// That is to sort tags in matches without moving rest of them.
class MatchTagSortAccessor_c
{
	const VecTraits_T<CSphMatch> & m_dTagOrder;
public:
	explicit MatchTagSortAccessor_c ( const VecTraits_T<CSphMatch> & dTagOrder) : m_dTagOrder ( dTagOrder ) {}
	using T = CSphMatch;
	using MEDIAN_TYPE = int;
	static MEDIAN_TYPE Key ( T * a ) { return a->m_iTag; }
	static void Swap ( T * a, T * b ) noexcept { ::Swap ( a->m_iTag, b->m_iTag ); }
	static T * Add ( T * p, int i ) noexcept { return p+i; }
	static int Sub ( T * b, T * a ) noexcept { return (int)(b-a); }
	static void CopyKey ( MEDIAN_TYPE * pMed, CSphMatch * pVal ) noexcept { *pMed = Key ( pVal ); }

	bool IsLess ( int a, int b ) const noexcept
	{
		return sphGetDocID ( m_dTagOrder[a].m_pDynamic )<sphGetDocID ( m_dTagOrder[b].m_pDynamic );
	}
};

// use space after end of matches to store indexes, WORD per match
bool MatchIterator_c::MaybeUseWordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const noexcept
{
	if ( dMatches.GetLength()>0x10000 )
		return false;

	int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
	if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( WORD ) ) )
		return false;

	// will use tail of the vec as blob of WORDs
	VecTraits_T dOrder = { (WORD *) dMatches.end (), m_iLimit };
	ARRAY_CONSTFOREACH( i, dOrder )
		dOrder[i] = i;
	dOrder.Sort ( Lesser ( [&dMatches] ( WORD a, WORD b ) {
		return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
	} ) );
	return true;
}

// use space after end of matches to store indexes, DWORD per match
bool MatchIterator_c::MaybeUseDwordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const noexcept
{
	if ( dMatches.GetLength64()>0x100000000 )
		return false;

	int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
	if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( DWORD ) ) )
		return false;

	// will use tail of the vec as blob of WORDs
	VecTraits_T dOrder = { (DWORD *) dMatches.end (), m_iLimit };
	for( DWORD i=0, uLen=dOrder.GetLength(); i<uLen; ++i )
		dOrder[i] = i;
	dOrder.Sort ( Lesser ( [&dMatches] ( DWORD a, DWORD b ) {
		return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
	} ) );
	return true;
}

// use tags to store indexes. No extra space, but random access order, many cash misses expected
void MatchIterator_c::UseTags ( VecTraits_T<CSphMatch> & dOrder ) noexcept
{
	ARRAY_CONSTFOREACH( i, dOrder )
		dOrder[i].m_iTag = i;

	MatchTagSortAccessor_c tOrder ( dOrder );
	sphSort ( dOrder.Begin (), dOrder.GetLength (), tOrder, tOrder );
	m_bTailClean = true;
}

MatchIterator_c::MatchIterator_c ( OneResultset_t & tResult )
	: m_tResult ( tResult )
{
	auto& dMatches = tResult.m_dMatches;
	m_iLimit = dMatches.GetLength();

	if ( MaybeUseWordOrder ( dMatches ) )
		m_fnOrder = [pData = (WORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
	else if ( MaybeUseDwordOrder ( dMatches ) )
		m_fnOrder = [pData = (DWORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
	else
	{
		UseTags ( dMatches );
		m_fnOrder = [this] ( int i ) { return m_tResult.m_dMatches[m_iRawIdx].m_iTag; };
	}

	m_iRawIdx = 0;
	m_iIdx = m_fnOrder(0);

	assert ( m_tResult.m_tSchema.GetAttr ( sphGetDocidName() ) );
	m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
}

MatchIterator_c::~MatchIterator_c ()
{
	if ( m_bTailClean )
		return;

	// need to reset state of some tail matches in order to avoid issues when deleting the vec of them
	// (since we used that memory region for own purposes)
	int iDirtyMatches = m_iLimit>0x10000 ? m_iLimit * sizeof ( DWORD ) : m_iLimit * sizeof ( WORD );
	iDirtyMatches = ( iDirtyMatches+sizeof ( CSphMatch )-1 ) / sizeof ( CSphMatch );
	for ( int i = 0; i<iDirtyMatches; ++i )
		( m_tResult.m_dMatches.end ()+i )->CleanGarbage();
}

bool MatchIterator_c::Step () noexcept
{
	++m_iRawIdx;
	if ( m_iRawIdx>=m_iLimit )
		return false;
	m_iIdx = m_fnOrder ( m_iRawIdx );
	m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
	return true;
}
