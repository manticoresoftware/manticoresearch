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

#include "recv_state.h"
#include "cluster_recv_state_cleanup.h"

void operator<< ( ISphOutputBuffer & tOut, const RecvStateCleanupRequest_t & tReq )
{
	tOut << (const ClusterRequest_t &) tReq;
	tOut.SendString ( tReq.m_sIndex.cstr() );
}

void operator>> ( MemInputBuffer_c & tIn, RecvStateCleanupRequest_t & tReq )
{
	tIn >> (ClusterRequest_t &)tReq;
	tReq.m_sIndex = tIn.GetString();
}

static bool CleanupRecvState ( const RecvStateCleanupRequest_t & tCmd )
{
	uint64_t uKey = DoubleStringKey ( tCmd.m_sCluster, tCmd.m_sIndex );
	sphLogDebugRpl ( "CleanupRecvState: checking RecvState for cluster %s index %s, key " INT64_FMT, tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), uKey );

	if ( !RECVSTATE_HASSTATE ( uKey ) )
	{
		sphLogDebugRpl ( "CleanupRecvState: RecvState not found for cluster %s index %s, key " INT64_FMT " (may have been already cleaned up)", 
			tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), uKey );
		return true;
	}

	sphLogDebugRpl ( "CleanupRecvState: freeing RecvState for cluster %s index %s, key " INT64_FMT, tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), uKey );
	RECVSTATE_FREE ( uKey );
	sphLogDebugRpl ( "CleanupRecvState: RecvState freed for cluster %s index %s, key " INT64_FMT, tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), uKey );

	return true;
}

// command at remote to free RecvState
void ReceiveClusterRecvStateCleanup ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster )
{
	RecvStateCleanupRequest_t tCmd;
	ClusterRecvStateCleanup_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;
	if ( CleanupRecvState ( tCmd ) )
		ClusterRecvStateCleanup_c::BuildReply ( tOut );
}

