//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/string.h"
#include "searchdconfig.h"
#include "searchdha.h"
#include "api_command_cluster.h"

struct ClusterIndexAddLocalRequest_t : ClusterRequest_t
{
	CSphString m_sIndex; // local name of index
	IndexType_e m_eIndex = IndexType_e::ERROR_;
	bool m_bSendFilesSuccess = false;
};

void operator<< ( ISphOutputBuffer& tOut, const ClusterIndexAddLocalRequest_t& tReq );
void operator>> ( MemInputBuffer_c& tOut, ClusterIndexAddLocalRequest_t& tReq );

using ClusterIndexAddLocal_c = ClusterCommand_T<E_CLUSTER::INDEX_ADD_LOCAL, ClusterIndexAddLocalRequest_t>;

bool AddLoadedIndexIntoCluster ( const CSphString & sCluster, const CSphString & sIndex );