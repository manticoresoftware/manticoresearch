//
// Copyright (c) 2019-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "std/vectraits.h"
#include "searchdha.h"

class SstProgress_i;
// send local index to remote nodes via API
bool ReplicateIndexToNodes ( const CSphString& sCluster, const CSphString& sIndex, const CSphString & sUser, const VecTraits_T<AgentDesc_t>& dDesc, const cServedIndexRefPtr_c& pServedIndex, SstProgress_i & tProgress );

// send distributed index to remote nodes via API
bool ReplicateDistIndexToNodes ( const CSphString & sCluster, const CSphString & sIndex, const CSphString & sUser, const VecTraits_T<AgentDesc_t> & dDesc );
