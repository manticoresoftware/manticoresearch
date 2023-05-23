//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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


class BlobPool_c
{
public:
	virtual			~BlobPool_c() = default;
	virtual void	SetBlobPool ( const BYTE * pBlobPool ) { m_pBlobPool = pBlobPool; }
	const BYTE *	GetBlobPool () const { return m_pBlobPool; }

protected:
	const BYTE *	m_pBlobPool {nullptr};
};

/// groupby key type
typedef int64_t SphGroupKey_t;

/// base grouper (class that computes groupby key)
class CSphGrouper : public BlobPool_c, public ISphRefcountedMT
{
public:
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual ESphAttr		GetResultType () const = 0;
	virtual CSphGrouper *	Clone() const = 0;
	virtual bool			IsMultiValue() const { return false; }
	virtual void			SetColumnar ( const columnar::Columnar_i * ) {}

protected:
							~CSphGrouper () override {} // =default causes bunch of errors building on wheezy
};

class DistinctFetcher_i : public ISphRefcountedMT
{
public:
	virtual SphAttr_t		GetKey ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const = 0;
	virtual void			SetBlobPool ( const BYTE * pBlobPool ) = 0;
	virtual void			SetColumnar ( const columnar::Columnar_i * pColumnar ) = 0;
	virtual void			FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) = 0;
	virtual bool			IsMultiValue() const = 0;
	virtual DistinctFetcher_i *	Clone() const = 0;
};

const char *	GetInternalAttrPrefix();
int 			GetStringRemapCount ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema );
bool			IsSortStringInternal ( const CSphString & sColumnName );
bool			IsSortJsonInternal ( const CSphString & sColumnName );
CSphString		SortJsonInternalSet ( const CSphString & sColumnName );
void			SetGroupingInUtcSort ( bool bGroupingInUtc );
int				GetAliasedAttrIndex ( const CSphString & sAttr, const CSphQuery & tQuery, const ISphSchema & tSchema );

void			SetAccurateAggregationDefault ( bool bEnabled );
bool			GetAccurateAggregationDefault();

void			SetDistinctThreshDefault ( int iThresh );
int 			GetDistinctThreshDefault();

int				ApplyImplicitCutoff ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters );
bool			HasImplicitGrouping ( const CSphQuery & tQuery );

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
/// if the pUpdate is given, creates the updater's queue and perform the index update
/// instead of searching
ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra = nullptr, QueryProfile_c * pProfile = nullptr );

void sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile );

#endif // _sphinxsort_
