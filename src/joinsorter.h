//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinx.h"

void				SetJoinCacheSize ( int64_t iSize );
int64_t				GetJoinCacheSize();
void				SetJoinBatchSize ( int iSize );
int					GetJoinBatchSize();

CSphVector<std::pair<int,bool>> FetchJoinRightTableFilters ( const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSchema, const char * szJoinedIndex );
bool				NeedToMoveMixedJoinFilters ( const CSphQuery & tQuery, const ISphSchema & tSchema );
bool				ExprHasLeftTableAttrs ( const CSphString & sAttr, const ISphSchema & tLeftSchema );
std::unique_ptr<ISphFilter> CreateJoinNullFilter ( const CSphFilterSettings & tSettings, const CSphAttrLocator & tNullMapLocator );

ISphMatchSorter *	CreateJoinSorter ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, ISphMatchSorter * pSorter, const CSphQuery & tJoinQueryOptions, bool bJoinedGroupSort, int iBatchSize, CSphString & sError );
bool				CreateJoinMultiSorter ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const SphQueueSettings_t & tSettings, const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<CSphQuery> & dJoinQueryOptions, VecTraits_T<ISphMatchSorter *> & dSorters, int iBatchSize, CSphString & sError );
