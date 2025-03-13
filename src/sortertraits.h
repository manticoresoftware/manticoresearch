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

#pragma once

#include "sphinxsort.h"
#include "aggregate.h"
#include "sphinxfilter.h"

class MatchSorter_c : public ISphMatchSorter
{
public:
	void				SetState ( const CSphMatchComparatorState & tState ) override;
	const CSphMatchComparatorState & GetState() const override { return m_tState; }

	void				SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override;
	const ISphSchema *	GetSchema() const override { return ( ISphSchema *) m_pSchema; }

	void				SetColumnar ( columnar::Columnar_i * pColumnar ) override { m_pColumnar = pColumnar; }
	int64_t				GetTotalCount() const override { return m_iTotal; }
	void				CloneTo ( ISphMatchSorter * pTrg ) const override;
	bool				CanBeCloned() const override;
	void				SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override;
	void				TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override;

	void				SetRandom ( bool bRandom ) override { m_bRandomize = bRandom; }
	bool				IsRandom() const override { return m_bRandomize; }

	int					GetMatchCapacity() const override { return m_iMatchCapacity; }

	RowTagged_t					GetJustPushed() const override { return m_tJustPushed; }
	VecTraits_T<RowTagged_t>	GetJustPopped() const override { return m_dJustPopped; }

protected:
	SharedPtr_t<ISphSchema>			m_pSchema;		///< sorter schema (adds dynamic attributes on top of index schema)
	CSphMatchComparatorState		m_tState;		///< protected to set m_iNow automatically on SetState() calls
	StrVec_t						m_dTransformed;

	columnar::Columnar_i *			m_pColumnar = nullptr;

	bool							m_bRandomize = false;
	int64_t							m_iTotal = 0;

	int								m_iMatchCapacity = 0;
	RowTagged_t						m_tJustPushed;
	CSphTightVector<RowTagged_t>	m_dJustPopped;
};

/// match-sorting priority queue traits
class CSphMatchQueueTraits : public MatchSorter_c, ISphNoncopyable
{
public:
	explicit	CSphMatchQueueTraits ( int iSize );
				~CSphMatchQueueTraits () override;

	int			GetLength() override					{ return Used(); }
	void		SwapMatchQueueTraits ( CSphMatchQueueTraits& rhs );
	const VecTraits_T<CSphMatch> & GetMatches() const	{ return m_dData; }

protected:
	int							m_iSize;	// size of internal struct we can operate
	CSphFixedVector<CSphMatch>	m_dData;
	CSphTightVector<int>		m_dIData;	// indexes into m_pData, to avoid extra moving of matches themselves

	CSphMatch * Last() const			{ return &m_dData[m_dIData.Last ()]; }
	CSphMatch & Get ( int iElem ) const	{ return m_dData[m_dIData[iElem]]; }
	CSphMatch & Add();
	int			Used() const			{ return m_dIData.GetLength(); }
	bool		IsEmpty() const			{ return m_dIData.IsEmpty(); }
	void		ResetAfterFlatten()		{ m_dIData.Resize(0); }
	int			ResetDynamic ( int iMaxUsed );
	int			ResetDynamicFreeData ( int iMaxUsed );
};


class MatchCloner_c
{
public:
	void		SetSchema ( const ISphSchema * pSchema );

	// clone plain part (incl. pointers) from src to dst
	// keep group part (aggregates, group_concat) of dst intact
	// it assumes that tDst m_pDynamic contains correct data, or wiped away.
	void		CloneKeepingAggrs ( CSphMatch & tDst, const CSphMatch & tSrc );

	// clone plain part (incl. pointers) from src to dst
	// group part (aggregates, group_concat) is not copied
	void		CloneWithoutAggrs ( CSphMatch & tDst, const CSphMatch & tSrc );
	
	// just write group part (aggregates, group_concat) without cloning
	// assumes tDst has allocated m_pDynamic. Fixme! look to #881 again...
	void		CopyAggrs ( CSphMatch & tDst, const CSphMatch & tSrc );

	// copy group part (aggregates)
	// move group_concat part without reallocating
	void		MoveAggrs ( CSphMatch & tDst, CSphMatch & tSrc );

	void		AddRaw ( const CSphAttrLocator& tLoc )	{ m_dAttrsGrp.Add(tLoc); }
	void		AddPtr ( const CSphAttrLocator &tLoc )	{ m_dAttrsPtr.Add(tLoc); }
	void		ResetAttrs();

	// (re)fill m_dMyPtrRows and m_dOtherPtrRows from m_dAttrsPtr
	void		CommitPtrs();

private:
	CSphFixedVector<CSphRowitem>	m_dRowBuf { 0 };
	CSphVector<CSphAttrLocator>		m_dAttrsGrp; // locators for grouping attrs (@groupby, @count, @distinct, etc.)
	CSphVector<CSphAttrLocator>		m_dAttrsPtr; // locators for group_concat attrs
	CSphVector<int>					m_dMyPtrRows; // rowids matching m_dAttrsPtr. i.e. grpconcat ptr result I own
	CSphVector<int>					m_dOtherPtrRows; // rest rowids NOT matching m_dAttrsPtr. i.e. other ptr results
	const CSphSchemaHelper *		m_pSchema = nullptr;
	bool							m_bPtrRowsCommited = false; // readiness of m_dMyPtrRows and m_dOtherPtrRows
};


class BaseGroupSorter_c : public BlobPool_c, protected CSphGroupSorterSettings
{
public:
			BaseGroupSorter_c ( const CSphGroupSorterSettings & tSettings ) { (CSphGroupSorterSettings&)(*this) = tSettings; }
			~BaseGroupSorter_c() override { ResetAggregates(); }

protected:
	MatchCloner_c				m_tPregroup;
	CSphVector<AggrFunc_i *>	m_dAggregates;

	void	SetColumnar ( columnar::Columnar_i * pColumnar );
	void	SetupBaseGrouper ( ISphSchema * pSchema, int iDistinct, CSphVector<AggrFunc_i *> * pAvgs = nullptr );

	// HAVING filtering
	bool	EvalHAVING ( const CSphMatch& tMatch );
	void	AggrUpdate ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge = false );
	void	AggrSetup ( CSphMatch & tDst, const CSphMatch & tSrc, bool bMerge = false );
	void	AggrUngroup ( CSphMatch & tMatch );

private:
	void	ResetAggregates();
};
