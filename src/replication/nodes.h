//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "searchdha.h"

using VecAgentDesc_t = CSphVector<AgentDesc_t>;

// string to list
StrVec_t ParseNodesFromString ( CSphString sNodes );

// collect listener API with external address
enum class Resolve_e { QUICK, SLOW };
VecAgentDesc_t GetDescAPINodes ( const VecTraits_T<CSphString> & dNodes, Resolve_e eSpeed );

StrVec_t FilterNodesByProto ( const VecTraits_T<CSphString> & dSrcNodes, Proto_e eProto, bool bResolve = true );
