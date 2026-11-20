//
// Copyright (c) 2019-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "cluster_index_add_local.h"
#include "api_command_cluster.h"
#include "cluster_commands.h"
#include "searchdreplication.h"

#include "api_reply_stream.h"
#include "digest_sha1.h"
#include "recv_state.h"
#include "cluster_bulk_chunk.h"


void operator<< ( ISphOutputBuffer& tOut, const ClusterIndexAddLocalRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&) tReq;
	tOut.SendString ( tReq.m_sIndex.cstr() );
	tOut.SendByte ( (BYTE)tReq.m_eIndex );
	tOut.SendByte ( tReq.m_bSendFilesSuccess );
	tOut.SendUint64 ( tReq.m_uBulkChunkId );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterIndexAddLocalRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut << "index:" << tReq.m_sIndex;
	tOut << "type:" << (BYTE)tReq.m_eIndex;
	tOut << "SendFilesSuccess:" << tReq.m_bSendFilesSuccess;
	tOut << "bulk_import chunk:" << tReq.m_uBulkChunkId;
	return tOut;
}

void operator>> ( MemInputBuffer_c& tIn, ClusterIndexAddLocalRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tReq.m_sIndex = tIn.GetString();
	tReq.m_eIndex = (IndexType_e)tIn.GetByte();
	tReq.m_bSendFilesSuccess = !!tIn.GetByte();
	tReq.m_uBulkChunkId = tIn.GetUint64();
}


struct ScopedState_t
{
	explicit ScopedState_t ( uint64_t uState )
		: m_uState ( uState )
	{}
	~ScopedState_t() { RecvState::Free ( m_uState ); }
	uint64_t m_uState = 0;
};

static void RemoveFiles ( const sph::StringSet& hActiveFiles, const VecTraits_T<CSphString>& dFiles )
{
	StrVec_t dRemove;
	for ( const auto& sFile : dFiles )
		if ( !hActiveFiles[sFile] )
			dRemove.Add ( sFile );
	RemoveFiles ( dRemove );
}

static void RemoveFiles ( const FilesTrait_t& tIndexFiles, const StrVec_t& dRenamedOld )
{
	sph::StringSet hActiveFiles;
	for ( const auto& sFile : tIndexFiles.m_dRef )
		hActiveFiles.Add ( sFile );

	RemoveFiles ( hActiveFiles, tIndexFiles.m_dOld );
	RemoveFiles ( hActiveFiles, dRenamedOld );
}

// load index from disk files for cluster use
static ServedIndexRefPtr_c LoadNewIndex ( const CSphString & sIndexPath, IndexType_e eIndexType, const char * szIndexName, FilesTrait_t & tIndexFiles )
{
	CSphConfigSection hIndex;
	hIndex.Add ( CSphVariant ( sIndexPath.cstr() ), "path" );
	hIndex.Add ( CSphVariant ( GetIndexTypeName ( eIndexType ) ), "type" );
	// dummy
	hIndex.Add ( CSphVariant ( "text" ), "rt_field" );
	hIndex.Add ( CSphVariant ( "gid" ), "rt_attr_uint" );

	TLS_MSG_STRING ( sError );
	ServedIndexRefPtr_c pResult;
	auto [eAdd, pNewServed] = AddIndex ( szIndexName, hIndex, false, true, true, nullptr, sError );

	assert ( eAdd == ADD_NEEDLOAD || eAdd == ADD_ERROR );
	if ( eAdd != ADD_NEEDLOAD )
		return pResult;

	assert ( pNewServed );

	StrVec_t dWarnings;
	bool bPrealloc = PreallocNewIndex ( *pNewServed, &hIndex, szIndexName, dWarnings, sError );
	if ( !bPrealloc )
		return pResult;

	UnlockedHazardIdxFromServed ( *pNewServed )->GetIndexFiles ( tIndexFiles.m_dRef, tIndexFiles.m_dRef );
	for ( const auto& i : dWarnings )
		sphWarning ( "table '%s': %s", szIndexName, i.cstr() );

	pResult = std::move ( pNewServed );
	return pResult;
}

// load index into daemon
static bool LoadAndReplaceIndex ( const CSphString& sIndexPath, IndexType_e eIndexType, const CSphString & sIndexName, FilesTrait_t & tIndexFiles )
{
	auto pNewIndex = LoadNewIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );
	if ( !pNewIndex )
		return false;

	g_pLocalIndexes->AddOrReplace ( pNewIndex, sIndexName );
	return true;
}

// load index into daemon
// in case index already exists prohibit it to save on index delete as disk files has fresh data received from remote node
static bool LoadIndex ( const CSphString & sIndexPath, IndexType_e eIndexType, const CSphString & sIndexName, FilesTrait_t & tIndexFiles )
{
	cServedIndexRefPtr_c pOldIndex = GetServed ( sIndexName );
	if ( !ServedDesc_t::IsMutable ( pOldIndex ) )
		return LoadAndReplaceIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );

	WIdx_T<RtIndex_i*> pIndex { pOldIndex };
	pIndex->ProhibitSave();
	pIndex->GetIndexFiles ( tIndexFiles.m_dOld, tIndexFiles.m_dOld );
	return LoadAndReplaceIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );
}


// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
static bool AddReceivedIndex ( const ClusterIndexAddLocalRequest_t& tAddCmd )
{
	if ( tAddCmd.m_uBulkChunkId )
		return tAddCmd.m_bSendFilesSuccess ? PrepareBulkChunk ( tAddCmd ) : AbortBulkChunk ( tAddCmd );

	uint64_t uKey = DoubleStringKey ( tAddCmd.m_sCluster, tAddCmd.m_sIndex );
	if ( !RecvState::HasState ( uKey ) )
		return TlsMsg::Err ( "missed writer state at joiner node for cluster '%s' table '%s'", tAddCmd.m_sCluster.cstr(), tAddCmd.m_sIndex.cstr() );

	ScopedState_t tStateGuard ( uKey );
	std::unique_ptr<MergeState_t> pMerge { RecvState::GetState ( uKey )->Flush () };

	if ( tAddCmd.m_eIndex!=IndexType_e::PERCOLATE && tAddCmd.m_eIndex!=IndexType_e::RT )
		return TlsMsg::Err ( "unsupported type '%s' in index '%s'", GetIndexTypeName ( tAddCmd.m_eIndex ), tAddCmd.m_sIndex.cstr() );

	if ( !CanReplaceIndex ( tAddCmd.m_sCluster, tAddCmd.m_sIndex ) )
	{
		RemoveStagedFiles ( *pMerge );
		return false;
	}

	sphLogDebugRpl ( "rotating table '%s' content from %s", tAddCmd.m_sIndex.cstr(), pMerge->m_sIndexPath.cstr() );
	RollbackFilesGuard_t tFilesGuard;
	TLS_MSG_STRING ( sError );
	if ( !RotateFiles ( *pMerge, tFilesGuard.m_tFiles, sError ) )
		return false;

	// verify whole index only in case debug replication
#ifndef NDEBUG
	// if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG )
	{
		sphLogDebugRpl ( "verify table '%s' from %s", tAddCmd.m_sIndex.cstr(), pMerge->m_sIndexPath.cstr() );

		// check that size matched and sha1 matched prior to loading index
		if ( !SyncSigVerify ( pMerge->m_dFilesRef, pMerge->m_dHashes ) )
			return false;
	}
#endif

	sphLogDebugRpl ( "%s table '%s' into cluster '%s' from %s", ( tAddCmd.m_bSendFilesSuccess ? "loading" : "rolling-back" ), tAddCmd.m_sIndex.cstr(), tAddCmd.m_sCluster.cstr(), pMerge->m_sIndexPath.cstr() );

	// rollback to old index files via RollbackFilesGuard_t
	if ( !tAddCmd.m_bSendFilesSuccess )
		return true;

	FilesTrait_t tIndexFiles;

	if ( !LoadIndex ( pMerge->m_sIndexPath, tAddCmd.m_eIndex, tAddCmd.m_sIndex, tIndexFiles ) )
		return TlsMsg::Err ( "failed to load index '%s'", tAddCmd.m_sIndex.cstr() );

	if ( !AddLoadedIndexIntoCluster ( tAddCmd.m_sCluster, tAddCmd.m_sIndex ) )
		return false;

	// keep: only files that donor has
	// remove:
	// - index files renamed to .old
	// - the old index files as the old index could have more files when the new index
	RemoveFiles ( tIndexFiles, tFilesGuard.m_tFiles.m_dOld );

	// clean rollback files lists
	tFilesGuard.m_tFiles.Reset();
	return true;
}

// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
bool ReceiveClusterIndexAddLocal ( GenericOutputBuffer_c& tOut, InputBuffer_c& tBuf, CSphString& sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs )
{
	ClusterIndexAddLocalRequest_t tAddCmd;
	ClusterIndexAddLocal_c::ParseRequest ( tBuf, tAddCmd );
	sCluster = tAddCmd.m_sCluster;

	ApiReplyStream_c tStream ( uHeartbeatIntervalMs, uReplyVersion, tOut );
	tStream.Start ();
	const bool bAdded = AddReceivedIndex ( tAddCmd );
	if ( !tStream.StopAndHandoff () )
		return false;

	if ( bAdded )
		ClusterIndexAddLocal_c::BuildReply ( tOut );
	return true;
}
