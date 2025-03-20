//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "searchdha.h"
#include "api_command_cluster.h"

struct SyncSrc_t;
struct FileReserveRequest_t: ClusterRequest_t
{
	CSphString m_sIndex;
	CSphString m_sIndexFileName;
	SyncSrc_t* m_pChunks = nullptr;
};

void operator<< ( ISphOutputBuffer& tOut, const FileReserveRequest_t& tReq );
void operator>> ( InputBuffer_c& tIn, FileReserveRequest_t& tReq );
StringBuilder_c& operator<< ( StringBuilder_c& tOut, const FileReserveRequest_t& tReq );

struct SyncDst_t
{
	CSphBitvec m_dNodeChunksMask;
	StrVec_t m_dRemotePaths;
	int64_t m_tmTimeout = 0;
	int64_t m_tmTimeoutFile = 0;
};

struct FileReserveReply_t: SyncDst_t
{
	bool m_bIndexActive = false;
};

void operator<< ( ISphOutputBuffer& tOut, const FileReserveReply_t& tReq );
void operator>> ( InputBuffer_c& tIn, FileReserveReply_t& tReq );
StringBuilder_c& operator<< ( StringBuilder_c& tOut, const FileReserveReply_t& tReq );

// API command to remote node prior to file send
using ClusterFileReserve_c = ClusterCommand_T<E_CLUSTER::FILE_RESERVE, FileReserveRequest_t, FileReserveReply_t>;

bool SendClusterFileReserve ( VecRefPtrs_t<AgentConn_t*>& dAgents );



