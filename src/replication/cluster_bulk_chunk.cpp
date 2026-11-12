//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "cluster_bulk_chunk.h"

#include "std/fnv64.h"
#include "std/at_scope_exit.h"
#include "coroutine.h"
#include "fileutils.h"
#include "searchdaemon.h"
#include "sphinxrt.h"
#include "indexer_rt_bulk.h"
#include "api_reply_stream.h"

#include "cluster_file_reserve.h"
#include "cluster_file_send.h"
#include "cluster_sst_progress.h"
#include "recv_state.h"
#include "send_files.h"
#include "configuration.h"

#include <atomic>

void operator<< ( ISphOutputBuffer & tOut, const BulkChunkRequest_t & tReq )
{
	tOut << (const ClusterRequest_t &)tReq;
	tOut.SendString ( tReq.m_sTable.cstr() );
	tOut.SendUint64 ( tReq.m_uId );
}

void operator>> ( InputBuffer_c & tIn, BulkChunkRequest_t & tReq )
{
	tIn >> (ClusterRequest_t &)tReq;
	tReq.m_sTable = tIn.GetString();
	tReq.m_uId = tIn.GetUint64();
}

StringBuilder_c & operator<< ( StringBuilder_c & tOut, const BulkChunkRequest_t & tReq )
{
	tOut << (const ClusterRequest_t &)tReq << "table:" << tReq.m_sTable << "id:" << tReq.m_uId;
	return tOut;
}

// the files of the chunk: names, hash layout and hashes; there is nothing at the node to compare them with
struct BulkChunkReserveRequest_t : BulkChunkRequest_t
{
	SyncSrc_t * m_pFiles = nullptr;
};

void operator<< ( ISphOutputBuffer & tOut, const BulkChunkReserveRequest_t & tReq )
{
	tOut << (const BulkChunkRequest_t &)tReq;
	assert ( tReq.m_pFiles );
	SendSyncSrcLayout ( tOut, *tReq.m_pFiles );
}

void operator>> ( InputBuffer_c & tIn, BulkChunkReserveRequest_t & tReq )
{
	tIn >> (BulkChunkRequest_t &)tReq;
	assert ( tReq.m_pFiles );
	GetSyncSrcLayout ( tIn, *tReq.m_pFiles );
}

StringBuilder_c & operator<< ( StringBuilder_c & tOut, const BulkChunkReserveRequest_t & tReq )
{
	tOut << (const BulkChunkRequest_t &)tReq << "files:" << ( tReq.m_pFiles ? tReq.m_pFiles->m_dBaseNames.GetLength() : 0 );
	return tOut;
}

using ClusterChunkReserve_c = ClusterCommand_T<E_CLUSTER::CHUNK_RESERVE, BulkChunkReserveRequest_t>;
using ClusterChunkPrepare_c = ClusterCommand_T<E_CLUSTER::CHUNK_PREPARE, BulkChunkRequest_t>;
using ClusterChunkAbort_c = ClusterCommand_T<E_CLUSTER::CHUNK_ABORT, BulkChunkRequest_t>;


// the API address of this node with its port, the pid, the time and a process counter: nodes that start in one second
// on one host still differ by the port
uint64_t NewBulkChunkId()
{
	static std::atomic<uint64_t> uSeq { 0 };
	uint64_t uId = sphFNV64 ( szIncomingProto() );
	uId = sphFNV64 ( (int64_t)getpid(), uId );
	uId = sphFNV64 ( sphMicroTimer(), uId );
	uId = sphFNV64 ( uSeq.fetch_add ( 1, std::memory_order_relaxed ), uId );
	return uId ? uId : 1;
}


uint64_t BulkChunkSessionKey ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId )
{
	return sphFNV64 ( uId, DoubleStringKey ( sCluster, sTable ) );
}


CSphString BulkChunkDir ( const CSphString & sRoot, uint64_t uId, bool bReceiving )
{
	return SphSprintf ( "%s/repl-" UINT64_FMT "%s", sRoot.cstr(), uId, bReceiving ? ".recv" : "" );
}

//////////////////////////////////////////////////////////////////////////
// chunks of tables that this node receives, keeps prepared or attaches

struct BulkChunkSession_t
{
	CSphString	m_sTable;
	uint64_t	m_uKey;
};

static Threads::Coro::Mutex_c g_tBulkChunkSessionsLock;
static CSphVector<BulkChunkSession_t> g_dBulkChunkSessions GUARDED_BY ( g_tBulkChunkSessionsLock );

static void AddBulkChunkSession ( const CSphString & sTable, uint64_t uKey )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	g_dBulkChunkSessions.Add ( { sTable, uKey } );
}

static void RemoveBulkChunkSession ( uint64_t uKey )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	ARRAY_FOREACH ( i, g_dBulkChunkSessions )
		if ( g_dBulkChunkSessions[i].m_uKey==uKey )
		{
			g_dBulkChunkSessions.RemoveFast ( i );
			return;
		}
}

void RegisterBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId )
{
	AddBulkChunkSession ( sTable, BulkChunkSessionKey ( sCluster, sTable, uId ) );
}

void ForgetBulkChunkSessions ( const CSphString & sTable )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	for ( int i = g_dBulkChunkSessions.GetLength()-1; i>=0; --i )
		if ( g_dBulkChunkSessions[i].m_sTable==sTable )
			g_dBulkChunkSessions.RemoveFast ( i );
}

bool HasBulkChunkSessions ( const CSphString & sTable )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	return g_dBulkChunkSessions.any_of ( [&sTable] ( const BulkChunkSession_t & tSession ) { return tSession.m_sTable==sTable; } );
}

//////////////////////////////////////////////////////////////////////////
// receive side

// the staging root of the table of the chunk; bInCluster: the table must be an RT table of the cluster of the request
static bool GetBulkChunkRoot ( const BulkChunkRequest_t & tCmd, bool bInCluster, CSphString & sRoot )
{
	cServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sTable );
	if ( !pServed || pServed->m_eType!=IndexType_e::RT )
		return TlsMsg::Err ( "bulk_import chunk: table '%s' is absent or not an RT table", tCmd.m_sTable.cstr() );

	if ( bInCluster && pServed->m_sCluster!=tCmd.m_sCluster )
		return TlsMsg::Err ( "bulk_import chunk: table '%s' is not in cluster '%s'", tCmd.m_sTable.cstr(), tCmd.m_sCluster.cstr() );

	RIdx_T<RtIndex_i *> pRt { pServed };
	sRoot = GetIndexerRtBulkRoot ( *pRt.Ptr() );
	return true;
}


// a new receive session of the chunk files into repl-<id>.recv/
static bool ReserveBulkChunk ( const BulkChunkReserveRequest_t & tCmd )
{
	assert ( tCmd.m_pFiles );
	const SyncSrc_t & tFiles = *tCmd.m_pFiles;
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': reserve %d files", tCmd.m_uId, tCmd.m_sTable.cstr(), tFiles.m_dBaseNames.GetLength() );

	if ( tFiles.m_dBaseNames.IsEmpty() || tFiles.m_dBaseNames.GetLength()!=tFiles.m_dChunks.GetLength() )
		return TlsMsg::Err ( "bulk_import chunk: wrong file list, %d names, %d layouts", tFiles.m_dBaseNames.GetLength(), tFiles.m_dChunks.GetLength() );

	for ( const CSphString & sFile : tFiles.m_dBaseNames )
		if ( !IsReplicationFileName ( sFile ) )
			return TlsMsg::Err ( "invalid replication file name" );

	const FileChunks_t & tLast = tFiles.m_dChunks.Last();
	const int iHashes = tLast.m_iHashStartItem + tLast.GetChunksCount();
	if ( iHashes!=tFiles.m_dHashes.GetLength() )
		return TlsMsg::Err ( "bulk_import chunk: %d hashes, expected %d", tFiles.m_dHashes.GetLength(), iHashes );

	CSphString sRoot;
	if ( !GetBulkChunkRoot ( tCmd, true, sRoot ) )
		return false;

	const uint64_t uKey = BulkChunkSessionKey ( tCmd.m_sCluster, tCmd.m_sTable, tCmd.m_uId );
	if ( RecvState::HasState ( uKey ) )
		return TlsMsg::Err ( "bulk_import chunk " UINT64_FMT " of table '%s' is already being received", tCmd.m_uId, tCmd.m_sTable.cstr() );

	if ( sphDirExists ( BulkChunkDir ( sRoot, tCmd.m_uId, false ) ) )
		return TlsMsg::Err ( "bulk_import chunk " UINT64_FMT " of table '%s' already exists", tCmd.m_uId, tCmd.m_sTable.cstr() );

	CSphString sDir = BulkChunkDir ( sRoot, tCmd.m_uId, true );
	TLS_MSG_STRING ( sError );
	if ( !CreateIndexerRtBulkDir ( sRoot, sDir, sError ) )
		return false;

	// the directory is new: every block comes from the sender, there are no local files to copy blocks from
	SyncDst_t tDst;
	tDst.m_dNodeChunksMask.Init ( iHashes );
	tDst.m_dRemotePaths.Resize ( tFiles.m_dBaseNames.GetLength() );
	ARRAY_FOREACH ( i, tDst.m_dRemotePaths )
		tDst.m_dRemotePaths[i].SetSprintf ( "%s/%s", sDir.cstr(), tFiles.m_dBaseNames[i].cstr() );
	StrVec_t dNoRefs ( tDst.m_dRemotePaths.GetLength() );

	RecvState::GetState ( uKey ).SetMerge ( tFiles, tDst, sDir, dNoRefs );
	AddBulkChunkSession ( tCmd.m_sTable, uKey );
	return true;
}


// all the files came: load and check the chunk, then rename repl-<id>.recv/ to repl-<id>/
static bool PrepareBulkChunk ( const BulkChunkRequest_t & tCmd )
{
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': prepare", tCmd.m_uId, tCmd.m_sTable.cstr() );

	const uint64_t uKey = BulkChunkSessionKey ( tCmd.m_sCluster, tCmd.m_sTable, tCmd.m_uId );
	if ( !RecvState::HasState ( uKey ) )
		return TlsMsg::Err ( "bulk_import chunk " UINT64_FMT " of table '%s' has no receive session", tCmd.m_uId, tCmd.m_sTable.cstr() );

	std::unique_ptr<MergeState_t> pMerge { RecvState::GetState ( uKey ).Flush() };
	RecvState::Free ( uKey );

	// every file was verified by its hash as it was written; a missing or short one was not sent to the end
	ARRAY_FOREACH ( i, pMerge->m_dFilesNew )
	{
		int64_t iSize = sphGetFileSize ( pMerge->m_dFilesNew[i], nullptr );
		if ( iSize!=pMerge->m_dChunks[i].m_iFileSize )
			return TlsMsg::Err ( "bulk_import chunk file '%s' has " INT64_FMT " bytes, expected " INT64_FMT, pMerge->m_dFilesNew[i].cstr(), iSize, pMerge->m_dChunks[i].m_iFileSize );
	}

	CSphString sRoot;
	if ( !GetBulkChunkRoot ( tCmd, true, sRoot ) )
		return false;

	CSphString sDir = BulkChunkDir ( sRoot, tCmd.m_uId, true );
	TLS_MSG_STRING ( sError );
	{
		auto pChunk = LoadIndexerRtBulkChunk ( SphSprintf ( "%s/chunk", sDir.cstr() ), sError );
		if ( !pChunk )
			return false;

		cServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sTable );
		if ( !pServed )
			return TlsMsg::Err ( "bulk_import chunk: table '%s' is absent", tCmd.m_sTable.cstr() );

		RIdx_T<RtIndex_i *> pRt { pServed };
		if ( !CanAttachIndexerRtBulkChunk ( *pRt.Ptr(), *pChunk, sError ) )
			return false;
	}

	return RenameIndexerRtBulkDir ( sDir, BulkChunkDir ( sRoot, tCmd.m_uId, false ), sError );
}


void DropBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId )
{
	const uint64_t uKey = BulkChunkSessionKey ( sCluster, sTable, uId );
	if ( RecvState::HasState ( uKey ) )
		RecvState::Free ( uKey );
	RemoveBulkChunkSession ( uKey );

	// the table may be gone or out of the cluster already; its staging root is still the place of the files
	BulkChunkRequest_t tCmd;
	tCmd.m_sCluster = sCluster;
	tCmd.m_sTable = sTable;
	CSphString sRoot;
	if ( GetBulkChunkRoot ( tCmd, false, sRoot ) )
	{
		RemoveIndexerRtBulkDir ( BulkChunkDir ( sRoot, uId, true ) );
		RemoveIndexerRtBulkDir ( BulkChunkDir ( sRoot, uId, false ) );
	}
	TlsMsg::ResetErr();
}


// the chunk is not going to be attached: drop its session and its files
static bool AbortBulkChunk ( const BulkChunkRequest_t & tCmd )
{
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': abort", tCmd.m_uId, tCmd.m_sTable.cstr() );
	DropBulkChunk ( tCmd.m_sCluster, tCmd.m_sTable, tCmd.m_uId );
	return true;
}


bool AttachBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, CSphString & sError )
{
	cServedIndexRefPtr_c pServed = GetServed ( sTable );
	if ( !pServed || pServed->m_eType!=IndexType_e::RT )
	{
		DropBulkChunk ( sCluster, sTable, uId );
		sError.SetSprintf ( "table '%s' is absent or not an RT table", sTable.cstr() );
		return false;
	}

	CSphString sDir;
	{
		RIdx_T<RtIndex_i *> pRt { pServed };
		sDir = BulkChunkDir ( GetIndexerRtBulkRoot ( *pRt.Ptr() ), uId, false );
	}
	AT_SCOPE_EXIT ( [&sCluster, &sTable, uId] { DropBulkChunk ( sCluster, sTable, uId ); } );

	// a node that joined after the delivery, or one that lost the directory
	CSphString sChunk = SphSprintf ( "%s/chunk", sDir.cstr() );
	if ( !sphFileExists ( SphSprintf ( "%s.sph", sChunk.cstr() ).cstr() ) )
	{
		sError.SetSprintf ( "bulk_import chunk " UINT64_FMT " of table '%s' is not prepared on this node", uId, sTable.cstr() );
		return false;
	}

	auto pChunk = LoadIndexerRtBulkChunk ( sChunk, sError );
	if ( !pChunk )
		return false;

	WIdx_T<RtIndex_i *> pRt { pServed };
	bool bCantAttach = false;
	return AttachLoadedIndexerRtBulkChunk ( *pRt.Ptr(), pChunk, bCantAttach, sError )==DiskAttachRes_e::OK;
}


template<typename COMMAND, typename REQUEST, typename ACTION>
static bool ReceiveBulkChunkCommand ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs, REQUEST & tCmd, ACTION && fnAction )
{
	COMMAND::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;

	ApiReplyStream_c tStream ( uHeartbeatIntervalMs, uReplyVersion, tOut );
	tStream.Start ();
	const bool bOk = fnAction ( tCmd );
	if ( !tStream.StopAndHandoff () )
		return false;

	if ( bOk )
		COMMAND::BuildReply ( tOut );
	return true;
}


bool ReceiveClusterChunkReserve ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs )
{
	SyncSrc_t tFiles;
	BulkChunkReserveRequest_t tCmd;
	tCmd.m_pFiles = &tFiles;
	return ReceiveBulkChunkCommand<ClusterChunkReserve_c> ( tOut, tBuf, sCluster, uReplyVersion, uHeartbeatIntervalMs, tCmd, ReserveBulkChunk );
}


bool ReceiveClusterChunkPrepare ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs )
{
	BulkChunkRequest_t tCmd;
	return ReceiveBulkChunkCommand<ClusterChunkPrepare_c> ( tOut, tBuf, sCluster, uReplyVersion, uHeartbeatIntervalMs, tCmd, PrepareBulkChunk );
}


bool ReceiveClusterChunkAbort ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs )
{
	BulkChunkRequest_t tCmd;
	return ReceiveBulkChunkCommand<ClusterChunkAbort_c> ( tOut, tBuf, sCluster, uReplyVersion, uHeartbeatIntervalMs, tCmd, AbortBulkChunk );
}

//////////////////////////////////////////////////////////////////////////
// send side

// drop the chunk at the nodes; a node without the session just has nothing to drop
static void AbortBulkChunkOnNodes ( const BulkChunkRequest_t & tReq, const CSphString & sUser, const VecTraits_T<AgentDesc_t> & dDesc )
{
	CSphString sReason = TlsMsg::MoveToString(); // the error that made the abort, the abort keeps it
	auto dNodes = ClusterChunkAbort_c::MakeAgents ( dDesc, sUser, ReplicationSstQueryTimeout(), tReq );
	ClusterChunkAbort_c tAbort;
	if ( !PerformRemoteTasksWrap ( dNodes, tAbort, tAbort, true ) )
		sphWarning ( "bulk_import chunk " UINT64_FMT " of table '%s': abort failed: %s", tReq.m_uId, tReq.m_sTable.cstr(), TlsMsg::szError() );
	TlsMsg::ResetErr();
	if ( !sReason.IsEmpty() )
		TlsMsg::Err ( sReason );
}


void AbortBulkChunkOnNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, const VecTraits_T<AgentDesc_t> & dDesc )
{
	if ( dDesc.IsEmpty() )
		return;

	BulkChunkRequest_t tReq;
	tReq.m_sCluster = sCluster;
	tReq.m_sTable = sTable;
	tReq.m_uId = uId;
	AbortBulkChunkOnNodes ( tReq, sUser, dDesc );
}


static bool BulkChunkCancelled()
{
	CSphString sError;
	if ( !CheckIndexerRtBulkCancelled ( sError ) )
		return false;

	TlsMsg::Err ( sError ); // returns false
	return true;
}


bool ReplicateBulkChunkToNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, StrVec_t dFiles, const VecTraits_T<AgentDesc_t> & dDesc )
{
	assert ( !dDesc.IsEmpty() );
	const int64_t tmStart = sphMicroTimer();

	BulkChunkRequest_t tReq;
	tReq.m_sCluster = sCluster;
	tReq.m_sTable = sTable;
	tReq.m_uId = uId;

	// a progress of its own: the progress object of the cluster belongs to SST and ALTER CLUSTER ADD
	auto pProgress = CreateProgress();
	pProgress->Init ( SstProgress_i::Role_e::DONOR );

	SyncSrc_t tSrc { std::move ( dFiles ) };
	if ( !tSrc.CalculateFilesSignatures ( *pProgress ) )
		return false;

	const int64_t tmHashed = sphMicroTimer();
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': sha1 of %d files in %.3f sec, sending to %d nodes", uId, sTable.cstr(), tSrc.m_dBaseNames.GetLength(), ( tmHashed-tmStart ) / 1000000.0f, dDesc.GetLength() );

	const int64_t iTimeoutMs = ReplicationSstQueryTimeout();
	bool bOk = [&]
	{
		if ( BulkChunkCancelled() )
			return false;

		// a repeated reserve or prepare finds the session of the first one and fails, so neither is retried
		BulkChunkReserveRequest_t tReserve;
		(BulkChunkRequest_t &)tReserve = tReq;
		tReserve.m_pFiles = &tSrc;
		auto dReserve = ClusterChunkReserve_c::MakeAgents ( dDesc, sUser, iTimeoutMs, tReserve );
		ClusterChunkReserve_c tReserveCmd;
		if ( !PerformRemoteTasksWrap ( dReserve, tReserveCmd, tReserveCmd, false ) )
			return false;

		if ( BulkChunkCancelled() )
			return false;

		// the nodes have nothing to copy blocks from: every block goes
		SyncDst_t tDst;
		tDst.m_dNodeChunksMask.Init ( tSrc.m_dHashes.GetLength() );
		CSphVector<RemoteFileState_t> dStates;
		for ( const AgentDesc_t & tDesc : dDesc )
		{
			auto & tState = dStates.Add();
			tState.m_pAgentDesc = &tDesc;
			tState.m_pSyncSrc = &tSrc;
			tState.m_pSyncDst = &tDst;
		}
		if ( !RemoteClusterFileSend ( tSrc, dStates, sCluster, sTable, BulkChunkSessionKey ( sCluster, sTable, uId ), sUser, *pProgress ) )
			return false;

		if ( BulkChunkCancelled() )
			return false;

		auto dPrepare = ClusterChunkPrepare_c::MakeAgents ( dDesc, sUser, iTimeoutMs, tReq );
		ClusterChunkPrepare_c tPrepareCmd;
		return PerformRemoteTasksWrap ( dPrepare, tPrepareCmd, tPrepareCmd, false );
	}();

	if ( !bOk )
		AbortBulkChunkOnNodes ( tReq, sUser, dDesc );

	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': %s on %d nodes in %.3f sec%s%s", uId, sTable.cstr(), bOk ? "prepared" : "failed", dDesc.GetLength(), ( sphMicroTimer()-tmStart ) / 1000000.0f, bOk ? "" : ": ", bOk ? "" : TlsMsg::szError() );
	return bOk;
}
