//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sortcomp_
#define _sortcomp_

#include "sphinxstd.h"

class ISphMatchComparator : public ISphRefcountedMT
{
	using fnFuncLessType = bool ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t );

public:
	FORCE_INLINE bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState ) const
	{
		assert ( m_fnLessFunc );
		return m_fnLessFunc ( a, b, tState );
	}

protected:
	ISphMatchComparator ( fnFuncLessType * fnLessFunc ) : m_fnLessFunc { fnLessFunc } {}
	~ISphMatchComparator () override = default;

private:
	fnFuncLessType * m_fnLessFunc = nullptr;
};


class MatchRelevanceLt_fn : public ISphMatchComparator
{
public:
	static bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState )
	{
		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_tRowID > b.m_tRowID;
	}

	MatchRelevanceLt_fn() : ISphMatchComparator (IsLess) {}
};


class MatchTimeSegments_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
		SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
		int iA = GetSegment ( aa, t.m_iNow );
		int iB = GetSegment ( bb, t.m_iNow );
		if ( iA!=iB )
			return iA > iB;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		if ( aa!=bb )
			return aa<bb;

		return a.m_tRowID > b.m_tRowID;
	}

	MatchTimeSegments_fn() : ISphMatchComparator (IsLess) {}

protected:
	static FORCE_INLINE int GetSegment ( SphAttr_t iStamp, SphAttr_t iNow )
	{
		if ( iStamp>=iNow-3600 ) return 0; // last hour
		if ( iStamp>=iNow-24*3600 ) return 1; // last day
		if ( iStamp>=iNow-7*24*3600 ) return 2; // last week
		if ( iStamp>=iNow-30*24*3600 ) return 3; // last month
		if ( iStamp>=iNow-90*24*3600 ) return 4; // last 3 months
		return 5; // everything else
	}
};


class MatchExpr_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		float aa = a.GetAttrFloat ( t.m_tLocator[0] ); // FIXME! OPTIMIZE!!! simplified (dword-granular) getter could be used here
		float bb = b.GetAttrFloat ( t.m_tLocator[0] );
		if ( aa!=bb )
			return aa<bb;

		return a.m_tRowID > b.m_tRowID;
	}

	MatchExpr_fn() : ISphMatchComparator (IsLess) {}
};

#define SPH_TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (t.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ROWID:		SPH_TEST_PAIR ( a.m_tRowID, b.m_tRowID, _idx ); break; \
		case SPH_KEYPART_WEIGHT:	SPH_TEST_PAIR ( a.m_iWeight, b.m_iWeight, _idx ); break; \
		case SPH_KEYPART_INT: \
		{ \
			SphAttr_t aa = a.GetAttr ( t.m_tLocator[_idx] ); \
			SphAttr_t bb = b.GetAttr ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ); \
			break; \
		} \
		case SPH_KEYPART_FLOAT: \
		{ \
			float aa = a.GetAttrFloat ( t.m_tLocator[_idx] ); \
			float bb = b.GetAttrFloat ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		case SPH_KEYPART_DOUBLE: \
		{ \
			double aa = a.GetAttrDouble ( t.m_tLocator[_idx] ); \
			double bb = b.GetAttrDouble ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		case SPH_KEYPART_STRINGPTR: \
		case SPH_KEYPART_STRING: \
		{ \
			int iCmp = t.CmpStrings ( a, b, _idx ); \
			if ( iCmp!=0 ) \
				return ( ( t.m_uAttrDesc >> (_idx) ) & 1 ) ^ ( iCmp>0 ); \
			break; \
		} \
	}


class MatchGeneric1_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		return a.m_tRowID>b.m_tRowID;
	}

	MatchGeneric1_fn() : ISphMatchComparator (IsLess) {}
};


class MatchGeneric2_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		return a.m_tRowID>b.m_tRowID;
	}

	MatchGeneric2_fn() : ISphMatchComparator (IsLess) {}
};


class MatchGeneric3_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		return a.m_tRowID>b.m_tRowID;
	}

	MatchGeneric3_fn() : ISphMatchComparator (IsLess) {}
};


class MatchGeneric4_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		return a.m_tRowID>b.m_tRowID;
	}

	MatchGeneric4_fn() : ISphMatchComparator (IsLess) {}
};


class MatchGeneric5_fn : public ISphMatchComparator
{
public:
	static FORCE_INLINE bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		SPH_TEST_KEYPART(4);
		return a.m_tRowID>b.m_tRowID;
	}

	MatchGeneric5_fn() : ISphMatchComparator (IsLess) {}
};

#endif // _sortcomp_
