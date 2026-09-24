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

#include "sphinx.h"

class CSphIndex;
class ISphMatchSorter;

const char * GetHybridScoreAttrName();
bool IsHybridScoreAttr ( const CSphString & sAttr );
bool IsKnnDistAttr ( const CSphString & sAttr );
bool IsHybridPostFusionAttr ( const CSphString & sAttr );
// facet query of a hybrid head as it runs over the fused candidates (the facet sorter must be created from it)
CSphQuery MakeHybridFacetScanQuery ( const CSphQuery & tHead, const CSphQuery & tFacet );
// dQueries is the hybrid query optionally followed by its facet queries (one sorter and result per query)
bool ExecuteHybridSearch ( const CSphIndex * pIndex, const VecTraits_T<CSphQuery> & dQueries, const SphQueueSettings_t & tQueueSettings, const VecTraits_T<CSphQueryResult> & dResults, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs );
