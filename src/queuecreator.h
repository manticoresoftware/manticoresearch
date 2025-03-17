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
#include "grouper.h"

/// additional group-by sorter settings
struct CSphGroupSorterSettings
{
	CSphAttrLocator		m_tLocGroupby;		///< locator for @groupby
	CSphAttrLocator		m_tLocCount;		///< locator for @count
	CSphAttrLocator		m_tLocDistinct;		///< locator for @distinct
	CSphAttrLocator		m_tLocGroupbyStr;	///< locator for @groupbystr

	bool				m_bDistinct = false;///< whether we need distinct
	CSphRefcountedPtr<CSphGrouper>		m_pGrouper;///< group key calculator
	CSphRefcountedPtr<DistinctFetcher_i> m_pDistinctFetcher;
	bool				m_bImplicit = false;///< for queries with aggregate functions but without group by clause
	SharedPtr_t<ISphFilter>	m_pAggrFilterTrait; ///< aggregate filter that got owned by grouper
	bool				m_bJson = false;	///< whether we're grouping by Json attribute
	int					m_iMaxMatches = 0;
	bool				m_bGrouped = false;	///< are we going to push already grouped matches to it?
	int					m_iDistinctAccuracy = 16;	///< HyperLogLog accuracy. 0 means "don't use HLL"

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema );
	void SetupDistinctAccuracy ( int iThresh );
};

struct PrecalculatedSorterResults_t
{
	int64_t		m_iCountDistinct = -1;
	int64_t		m_iCountFilter = -1;
	int64_t		m_iCount = -1;
	CSphString	m_sAttr;
};

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
/// if the pUpdate is given, creates the updater's queue and perform the index update
/// instead of searching
ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra = nullptr, QueryProfile_c * pProfile = nullptr );
void			sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile );

bool			HasImplicitGrouping ( const CSphQuery & tQuery );
bool			sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema ); // check query for expressions
int				GetAliasedAttrIndex ( const CSphString & sAttr, const CSphQuery & tQuery, const ISphSchema & tSchema );
bool			IsGroupbyMagic ( const CSphString & s );
ESphAttr		DetermineNullMaskType ( int iNumAttrs );
const char *	GetInternalAttrPrefix();
const char *	GetInternalJsonPrefix();
bool			IsSortStringInternal ( const CSphString & sColumnName );
bool			IsSortJsonInternal ( const CSphString & sColumnName );
CSphString		SortJsonInternalSet ( const CSphString & sColumnName );

