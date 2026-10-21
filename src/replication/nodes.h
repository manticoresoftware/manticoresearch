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

#include "searchdha.h"

using VecAgentDesc_t = CSphVector<AgentDesc_t>;

// string to list
StrVec_t ParseNodesFromString ( CSphString sNodes );

// collect listener API with external address
enum class Resolve_e { QUICK, SLOW };
// bSkipOwn drops this node's own API address from the result
VecAgentDesc_t GetDescAPINodes ( const VecTraits_T<CSphString> & dNodes, Resolve_e eSpeed, bool bSkipOwn = true );

// the same, one node at a time (QUICK): a node that does not resolve is skipped with a warning instead of emptying the whole list;
// when no node resolved and some failed, TLS holds the error of the first failed node
VecAgentDesc_t GetDescAPINodesEach ( const VecTraits_T<CSphString> & dNodes, const CSphString & sCluster, bool bSkipOwn = true );

// pause in ms after the failed attempt iAttempt (0-based) of the SLOW schedule, -1 when no attempts are left
int SlowRetryDelayMs ( int iAttempt );

StrVec_t FilterNodesByProto ( const VecTraits_T<CSphString> & dSrcNodes, Proto_e eProto, bool bResolve = true );
