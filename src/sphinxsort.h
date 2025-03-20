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

#ifndef _sphinxsort_
#define _sphinxsort_

#include "sortsetup.h"
#include "queuecreator.h"

namespace columnar
{
	class Columnar_i;
}


class MatchProcessor_i
{
public:
	virtual			~MatchProcessor_i () {}
	virtual void	Process ( CSphMatch * pMatch ) = 0;
	virtual void	Process ( VecTraits_T<CSphMatch *> & dMatches ) = 0;
	virtual bool	ProcessInRowIdOrder() const = 0;
};

using GetBlobPoolFromMatch_fn = std::function< const BYTE* ( const CSphMatch * )>;
using GetColumnarFromMatch_fn = std::function< columnar::Columnar_i * ( const CSphMatch * )>;

/// generic match sorter interface
class ISphMatchSorter
{
public:
	virtual				~ISphMatchSorter() {}

	/// check if this sorter does groupby
	virtual bool		IsGroupby() const = 0;

	/// set match comparator state
	virtual void		SetState ( const CSphMatchComparatorState & tState ) = 0;

	/// get match comparator stat
	virtual const CSphMatchComparatorState & GetState() const = 0;

	/// set group comparator state
	virtual void		SetGroupState ( const CSphMatchComparatorState & ) {}

	/// set blob pool pointer (for string+groupby sorters)
	virtual void		SetBlobPool ( const BYTE * ) {}

	/// set columnar (to work with columnar attributes)
	virtual void		SetColumnar ( columnar::Columnar_i * pColumnar ) = 0;

	/// set sorter schema
	virtual void		SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) = 0;

	/// get incoming schema
	virtual const ISphSchema *	GetSchema() const = 0;

	/// base push
	/// returns false if the entry was rejected as duplicate
	/// returns true otherwise (even if it was not actually inserted)
	virtual bool		Push ( const CSphMatch & tEntry ) = 0;

	/// push multiple matches at once
	virtual void		Push ( const VecTraits_T<const CSphMatch> & dMatches ) = 0;

	/// submit pre-grouped match. bNewSet indicates that the match begins the bunch of matches got from one source
	virtual bool		PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) = 0;

	/// get	rough entries count, due of aggregate filtering phase
	virtual int			GetLength() = 0;

	/// get total count of non-duplicates Push()ed through this queue
	virtual int64_t		GetTotalCount() const = 0;

	/// process collected entries up to length count
	virtual void		Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) = 0;

	/// store all entries into specified location and remove them from the queue
	/// entries are stored in properly sorted order
	/// return sorted entries count, might be less than length due of aggregate filtering phase
	virtual int			Flatten ( CSphMatch * pTo ) = 0;

	/// get a pointer to the worst element, NULL if there is no fixed location
	virtual const CSphMatch * GetWorst() const { return nullptr; }

	/// returns whether the sorter can be cloned to distribute processing over multi threads
	/// (delete and update sorters are too complex by side effects and can't be cloned)
	virtual bool		CanBeCloned() const { return true; }

	/// make same sorter (for MT processing)
	virtual ISphMatchSorter * Clone() const = 0;

	/// move resultset into target
	virtual void		MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) = 0;

	/// makes the same sorter
	virtual void		CloneTo ( ISphMatchSorter * pTrg ) const = 0;

	/// set attributes list these should copied into result set \ final matches
	virtual void		SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) = 0;

	/// transform collected matches into standalone (copy all pooled attrs to ptrs, drop unused)
	/// param fnBlobPoolFromMatch provides pool pointer from currently processed match pointer.
	virtual void		TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) = 0;

	virtual void		SetRandom ( bool bRandom ) = 0;
	virtual bool		IsRandom() const = 0;

	virtual int			GetMatchCapacity() const = 0;

	virtual RowTagged_t					GetJustPushed() const = 0;
	virtual VecTraits_T<RowTagged_t>	GetJustPopped() const = 0;

	/// whether we can do implicit cutoff or not (update/delete sorters should disable cutoff)
	virtual bool		IsCutoffDisabled() const { return false; }

	// tells the sorter whether we are working on a raw index and it can use columnar storage
	// or it should use the values stored in the matches
	// used by columnar aggregate functions
	virtual void		SetMerge ( bool bMerge ) = 0;

	/// is it a sorter that uses precalculated data and does not require real matches?
	virtual bool		IsPrecalc() const { return false; }

	virtual bool		IsJoin() const { return false; }
	virtual bool		FinalizeJoin ( CSphString & sError, CSphString & sWarning ) { return true; }

	/// add optional description to display in meta
	virtual void		AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const {}
};


struct CmpPSortersByRandom_fn
{
	inline static bool IsLess ( const ISphMatchSorter * a, const ISphMatchSorter * b )
	{
		assert ( a );
		assert ( b );
		return a->IsRandom()<b->IsRandom();
	}
};


void	SetAccurateAggregationDefault ( bool bEnabled );
bool	GetAccurateAggregationDefault();

void	SetDistinctThreshDefault ( int iThresh );
int 	GetDistinctThreshDefault();

int		ApplyImplicitCutoff ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters, bool bFT );

ISphMatchSorter *	CreateCollectQueue ( int iMaxMatches, CSphVector<BYTE> & tCollection );
ISphMatchSorter *	CreateDirectSqlQueue ( RowBuffer_i * pOutput, void ** ppOpaque1, void ** ppOpaque2, const StrVec_t & dColumns );
ISphMatchSorter *	CreatePlainSorter ( ESphSortFunc eMatchFunc, bool bKbuffer, int iMaxMatches, bool bFactors );

struct CSphGroupSorterSettings;
struct PrecalculatedSorterResults_t;
ISphMatchSorter *	CreateSorter ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const PrecalculatedSorterResults_t & tPrecalc );

#endif // _sphinxsort_
