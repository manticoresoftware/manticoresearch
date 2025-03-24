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

#include "nodes.h"
#include "std/string.h"
#include "wsrep_cxx.h"
#include "api_command_cluster.h"

struct ClusterSyncedRequest_t : ClusterRequest_t
{
	Wsrep::GlobalTid_t m_tGtid; // GTID received
	StrVec_t m_dIndexes;		// index list received
	bool m_bSendFilesSuccess = true;
	CSphString m_sMsg;
};

bool SendClusterSynced ( const VecAgentDesc_t& dDesc, const ClusterSyncedRequest_t& tRequest );


