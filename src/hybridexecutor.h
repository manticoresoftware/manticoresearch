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
bool ExecuteHybridSearch ( const CSphIndex * pIndex, const CSphQuery & tQuery, const SphQueueSettings_t & tQueueSettings, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs );
