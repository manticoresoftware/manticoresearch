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

#include "sorterprecalc.h"

#include "sortertraits.h"

static const char * GetPrecalcSorterName() { return "Precalc"; }


class FastBaseSorter_c : public MatchSorter_c, ISphNoncopyable, protected BaseGroupSorter_c
{
public:
			FastBaseSorter_c ( const CSphGroupSorterSettings & tSettings ) : BaseGroupSorter_c ( tSettings ) {}

	bool	IsGroupby () const final										{ return true; }
	bool	CanBeCloned() const final										{ return false; }
	void	SetMerge ( bool bMerge ) final									{}
	void	Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) final	{ if ( GetLength() ) tProcessor.Process ( &m_tData ); }
	int		GetLength() final												{ return m_bDataInitialized ? 1 : 0; }
	ISphMatchSorter * Clone() const final									{ return nullptr; }
	void	MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final			{ assert ( 0 && "Not supported"); }
	bool	IsPrecalc() const final											{ return true; }
	int		Flatten ( CSphMatch * pTo ) final;

protected:
	CSphMatch	m_tData;
	bool		m_bDataInitialized = false;
};


int FastBaseSorter_c::Flatten ( CSphMatch * pTo )
{
	assert ( m_bDataInitialized );
	Swap ( *pTo, m_tData );
	m_iTotal = 0;
	m_bDataInitialized = false;
	return 1;
}

// fast count distinct sorter
// works by using precalculated count distinct taken from secondary indexes
class FastCountDistinctSorter_c final : public FastBaseSorter_c
{
public:
			FastCountDistinctSorter_c ( int iCountDistinct, const CSphString & sAttr, const CSphGroupSorterSettings & tSettings );

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushEx(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final			{ return PushEx(tEntry); }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const final		{ dDesc.Add ( { m_sAttr, GetPrecalcSorterName() } ); }

private:
	int			m_iCountDistinct = 0;
	CSphString	m_sAttr;

	bool	PushEx ( const CSphMatch & tEntry );
};


FastCountDistinctSorter_c::FastCountDistinctSorter_c ( int iCountDistinct, const CSphString & sAttr, const CSphGroupSorterSettings & tSettings )
	: FastBaseSorter_c ( tSettings )
	, m_iCountDistinct ( iCountDistinct )
	, m_sAttr ( sAttr )
{}


FORCE_INLINE bool FastCountDistinctSorter_c::PushEx ( const CSphMatch & tEntry )
{
	if ( m_bDataInitialized )
		return true; // always return true, otherwise in RT indexes we won't be able to hit cutoff in disk chunks after the first one

	m_pSchema->CloneMatch ( m_tData, tEntry );
	m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
	m_tData.SetAttr ( m_tLocCount, 1 );
	m_tData.SetAttr ( m_tLocDistinct, m_iCountDistinct );

	m_bDataInitialized = true;
	m_iTotal++;
	return true;
}

// fast count sorter
// works by using precalculated count taken from secondary indexes
class FastCountFilterSorter_c final : public FastBaseSorter_c
{
public:
			FastCountFilterSorter_c ( int iCount, const CSphString & sAttr, const CSphGroupSorterSettings & tSettings );

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushEx(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final			{ return PushEx(tEntry); }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const final		{ dDesc.Add ( { m_sAttr, GetPrecalcSorterName() } ); }

private:
	int			m_iCount = 0;
	CSphString	m_sAttr;

	bool	PushEx ( const CSphMatch & tEntry );
};


FastCountFilterSorter_c::FastCountFilterSorter_c ( int iCount, const CSphString & sAttr, const CSphGroupSorterSettings & tSettings )
	: FastBaseSorter_c ( tSettings )
	, m_iCount ( iCount )
	, m_sAttr ( sAttr )
{}


FORCE_INLINE bool FastCountFilterSorter_c::PushEx ( const CSphMatch & tEntry )
{
	if ( m_bDataInitialized )
		return true; // always return true, otherwise in RT indexes we won't be able to hit cutoff in disk chunks after the first one

	m_pSchema->CloneMatch ( m_tData, tEntry );
	m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
	m_tData.SetAttr ( m_tLocCount, 1 );
	m_tData.SetAttr ( m_tLocCount, m_iCount );

	m_bDataInitialized = true;
	m_iTotal++;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

ISphMatchSorter * CreatePrecalcSorter ( const PrecalculatedSorterResults_t & tPrecalc, const CSphGroupSorterSettings & tSettings )
{
	if ( tPrecalc.m_iCountDistinct!=-1 )
		return new FastCountDistinctSorter_c ( tPrecalc.m_iCountDistinct, tPrecalc.m_sAttr, tSettings );

	if ( tPrecalc.m_iCountFilter!=-1 )
		return new FastCountFilterSorter_c ( tPrecalc.m_iCountFilter, tPrecalc.m_sAttr, tSettings );

	if ( tPrecalc.m_iCount!=-1 )
		return new FastCountFilterSorter_c ( tPrecalc.m_iCount, "count(*)", tSettings );

	return nullptr;
}
