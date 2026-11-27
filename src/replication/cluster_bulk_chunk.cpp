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

#include "cluster_file_reserve.h"
#include "cluster_file_send.h"
#include "cluster_index_add_local.h"
#include "cluster_sst_progress.h"
#include "recv_state.h"
#include "send_files.h"
#include "configuration.h"

#include <atomic>

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


CSphString BulkChunkFilebase ( const CSphString & sRoot, uint64_t uId )
{
	return SphSprintf ( "%s/repl-" UINT64_FMT, sRoot.cstr(), uId );
}

BulkChunkSession_t::BulkChunkSession_t ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, const CSphString & sFilebase )
	: m_sCluster ( sCluster )
	, m_sTable ( sTable )
	, m_sFilebase ( sFilebase )
	, m_uKey ( BulkChunkSessionKey ( sCluster, sTable, uId ) )
{}

BulkChunkSession_t::~BulkChunkSession_t() = default;

bool BulkChunkSession_t::BeginPrepare()
{
	auto eExpected = BulkChunkState_e::RECEIVING;
	return m_eState.compare_exchange_strong ( eExpected, BulkChunkState_e::PREPARING );
}

bool BulkChunkSession_t::Publish ( std::unique_ptr<CSphIndex> & pChunk )
{
	assert ( pChunk );
	auto eExpected = BulkChunkState_e::PREPARING;
	if ( !m_eState.compare_exchange_strong ( eExpected, BulkChunkState_e::READY ) )
		return false;
	m_pChunk = std::move ( pChunk );
	return true;
}

std::unique_ptr<CSphIndex> BulkChunkSession_t::Take()
{
	auto eExpected = BulkChunkState_e::READY;
	if ( !m_eState.compare_exchange_strong ( eExpected, BulkChunkState_e::APPLYING ) )
		return nullptr;
	return std::move ( m_pChunk );
}

void BulkChunkSession_t::Cancel()
{
	auto eState = m_eState.load();
	while ( eState!=BulkChunkState_e::FINISHED && eState!=BulkChunkState_e::CANCELLED
		&& !m_eState.compare_exchange_weak ( eState, BulkChunkState_e::CANCELLED ) )
	{}
}

void BulkChunkSession_t::Finish()
{
	m_pChunk.reset();
	m_eState.store ( BulkChunkState_e::FINISHED );
}

//////////////////////////////////////////////////////////////////////////
// The registry owns stable sessions, never an address into its movable vector.

static Threads::Coro::Mutex_c g_tBulkChunkSessionsLock;
static CSphVector<BulkChunkSessionRefPtr_t> g_dBulkChunkSessions GUARDED_BY ( g_tBulkChunkSessionsLock );
static bool g_bBulkChunkShutdown GUARDED_BY ( g_tBulkChunkSessionsLock ) = false;

static BulkChunkSessionRefPtr_t FindBulkChunkSession ( uint64_t uKey )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	for ( const auto & pSession : g_dBulkChunkSessions )
		if ( pSession->m_uKey==uKey )
			return pSession;
	return nullptr;
}

static bool AddBulkChunkSession ( const BulkChunkSessionRefPtr_t & pSession )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	if ( g_bBulkChunkShutdown || sphInterrupted() )
		return TlsMsg::Err ( "Server shutdown in progress" );
	for ( const auto & pOld : g_dBulkChunkSessions )
		if ( pOld->m_uKey==pSession->m_uKey )
			return TlsMsg::Err ( "bulk_import chunk of table '%s' already has a session", pSession->m_sTable.cstr() );
	g_dBulkChunkSessions.Add ( pSession );
	return true;
}

static void RemoveBulkChunkSession ( const BulkChunkSessionRefPtr_t & pSession )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	ARRAY_FOREACH ( i, g_dBulkChunkSessions )
		if ( g_dBulkChunkSessions[i].Ptr()==pSession.Ptr() )
		{
			g_dBulkChunkSessions.RemoveFast ( i );
			return;
		}
}

// Caller holds the operation lock. Close the object before unlinking its captured staging paths.
static void CleanupBulkChunkSession ( BulkChunkSession_t & tSession )
{
	if ( tSession.m_bReceiving && RecvState::HasState ( tSession.m_uKey ) )
		RecvState::Free ( tSession.m_uKey );
	tSession.m_bReceiving = false;
	tSession.Finish();
	RemoveFiles ( tSession.m_dFilesNew );
	RemoveFiles ( tSession.m_dFilesRef );
	tSession.m_dFilesNew.Reset();
	tSession.m_dFilesRef.Reset();
}

static void CancelBulkChunkSession ( const BulkChunkSessionRefPtr_t & pSession )
{
	pSession->Cancel(); // wins against a late Publish even while Load holds the operation lock
	{
		Threads::ScopedCoroMutex_t tLock ( pSession->m_tLock );
		CleanupBulkChunkSession ( *pSession );
	}
	RemoveBulkChunkSession ( pSession );
}

bool RegisterBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, std::unique_ptr<CSphIndex> pChunk, const VecTraits_T<CSphString> & dFiles, CSphString & sError )
{
	BulkChunkSessionRefPtr_t pSession { new BulkChunkSession_t ( sCluster, sTable, uId, pChunk->GetFilebase() ) };
	pSession->m_dFilesRef.Append ( dFiles );
	pSession->BeginPrepare();
	pSession->Publish ( pChunk );
	if ( AddBulkChunkSession ( pSession ) )
		return true;
	sError = TlsMsg::szError();
	CancelBulkChunkSession ( pSession );
	return false;
}

static void ForgetBulkChunks ( const CSphString * pTable, const CSphString * pCluster, bool bShutdown=false )
{
	CSphVector<BulkChunkSessionRefPtr_t> dSessions;
	{
		Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
		if ( bShutdown )
			g_bBulkChunkShutdown = true;
		for ( const auto & pSession : g_dBulkChunkSessions )
			if ( ( !pTable || pSession->m_sTable==*pTable ) && ( !pCluster || pSession->m_sCluster==*pCluster ) )
			{
				pSession->Cancel();
				dSessions.Add ( pSession );
			}
	}
	for ( const auto & pSession : dSessions )
		CancelBulkChunkSession ( pSession );
}

void ForgetBulkChunkSessions ( const CSphString & sTable )
{
	ForgetBulkChunks ( &sTable, nullptr );
}

void ForgetClusterBulkChunkSessions ( const CSphString & sCluster )
{
	ForgetBulkChunks ( nullptr, &sCluster );
}

void ShutdownBulkChunkSessions()
{
	Threads::CallCoroutine ( [] { ForgetBulkChunks ( nullptr, nullptr, true ); } );
}

bool HasBulkChunkSessions ( const CSphString & sTable )
{
	Threads::ScopedCoroMutex_t tLock ( g_tBulkChunkSessionsLock );
	return g_dBulkChunkSessions.any_of ( [&sTable] ( const auto & pSession ) { return pSession->m_sTable==sTable; } );
}

static bool GetBulkChunkRoot ( const CSphString & sTable, const CSphString * pCluster, CSphString & sRoot )
{
	cServedIndexRefPtr_c pServed = GetServed ( sTable );
	if ( !pServed || pServed->m_eType!=IndexType_e::RT )
		return TlsMsg::Err ( "bulk_import chunk: table '%s' is absent or not an RT table", sTable.cstr() );
	if ( pCluster && pServed->m_sCluster!=*pCluster )
		return TlsMsg::Err ( "bulk_import chunk: table '%s' is not in cluster '%s'", sTable.cstr(), pCluster->cstr() );
	RIdx_T<RtIndex_i *> pRt { pServed };
	sRoot = GetIndexerRtBulkRoot ( *pRt.Ptr() );
	return true;
}

bool ReserveBulkChunk ( const FileReserveRequest_t & tCmd, FileReserveReply_t & tRes )
{
	assert ( tCmd.m_pChunks );
	const SyncSrc_t & tFiles = *tCmd.m_pChunks;
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': reserve %d files", tCmd.m_uBulkChunkId, tCmd.m_sIndex.cstr(), tFiles.m_dBaseNames.GetLength() );
	if ( tFiles.m_dBaseNames.IsEmpty() || tFiles.m_dBaseNames.GetLength()!=tFiles.m_dChunks.GetLength() )
		return TlsMsg::Err ( "bulk_import chunk: invalid file list" );

	sph::StringSet hNames;
	for ( const auto & sFile : tFiles.m_dBaseNames )
	{
		if ( !IsReplicationFileName ( sFile ) || sFile.Length()<=6 || strncmp ( sFile.cstr(), "chunk.", 6 ) )
			return TlsMsg::Err ( "invalid replication chunk file name" );
		if ( hNames[sFile] )
			return TlsMsg::Err ( "duplicate replication chunk file name" );
		hNames.Add ( sFile );
	}

	int64_t iHashes = tFiles.m_dBaseNames.GetLength();
	for ( const auto & tFile : tFiles.m_dChunks )
	{
		if ( tFile.m_iFileSize<0 || tFile.m_iChunkBytes<=0 || tFile.m_iHashStartItem!=iHashes )
			return TlsMsg::Err ( "bulk_import chunk: invalid file layout" );
		iHashes += tFile.m_iFileSize ? 1+( tFile.m_iFileSize-1 )/tFile.m_iChunkBytes : 0;
		if ( iHashes>INT_MAX )
			return TlsMsg::Err ( "bulk_import chunk: too many hashes" );
	}
	if ( iHashes!=tFiles.m_dHashes.GetLength() )
		return TlsMsg::Err ( "bulk_import chunk: wrong hash count" );

	CSphString sRoot;
	if ( !GetBulkChunkRoot ( tCmd.m_sIndex, &tCmd.m_sCluster, sRoot ) )
		return false;
	BulkChunkSessionRefPtr_t pSession { new BulkChunkSession_t ( tCmd.m_sCluster, tCmd.m_sIndex, tCmd.m_uBulkChunkId, BulkChunkFilebase ( sRoot, tCmd.m_uBulkChunkId ) ) };
	if ( !AddBulkChunkSession ( pSession ) )
		return false;
	bool bReserved = false;
	AT_SCOPE_EXIT ( [&] { if ( !bReserved ) CancelBulkChunkSession ( pSession ); } );
	Threads::ScopedCoroMutex_t tLock ( pSession->m_tLock );
	if ( RecvState::HasState ( pSession->m_uKey ) )
		return TlsMsg::Err ( "bulk_import chunk already has a receive state" );
	TLS_MSG_STRING ( sError );
	if ( !PrepareStagingRoot ( sRoot, sError ) )
		return false;
	if ( sphFileExists ( pSession->m_sFilebase.cstr() ) || !FindFiles ( SphSprintf ( "%s.*", pSession->m_sFilebase.cstr() ).cstr() ).IsEmpty() )
		return TlsMsg::Err ( "bulk_import chunk " UINT64_FMT " of table '%s' already exists", tCmd.m_uBulkChunkId, tCmd.m_sIndex.cstr() );

	tRes.m_dNodeChunksMask.Init ( (int)iHashes );
	StrVec_t dFilesRef ( tFiles.m_dBaseNames.GetLength() );
	ARRAY_FOREACH ( i, dFilesRef )
		dFilesRef[i].SetSprintf ( "%s%s", pSession->m_sFilebase.cstr(), tFiles.m_dBaseNames[i].cstr()+5 );
	if ( !ReserveFiles ( dFilesRef, tFiles.m_dChunks, tRes.m_dNodeChunksMask, tRes.m_dRemotePaths, sError ) )
		return false;
	pSession->m_dFilesNew.Append ( tRes.m_dRemotePaths );
	pSession->m_dFilesRef = std::move ( dFilesRef );
	if ( pSession->m_eState.load()!=BulkChunkState_e::RECEIVING )
		return TlsMsg::Err ( "bulk_import chunk was cancelled" );
	RecvState::GetState ( pSession->m_uKey )->SetMerge ( tFiles, tRes, pSession->m_sFilebase, pSession->m_dFilesRef );
	pSession->m_bReceiving = true;
	bReserved = true;
	return true;
}

bool PrepareBulkChunk ( const ClusterIndexAddLocalRequest_t & tCmd )
{
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': prepare", tCmd.m_uBulkChunkId, tCmd.m_sIndex.cstr() );
	auto pSession = FindBulkChunkSession ( BulkChunkSessionKey ( tCmd.m_sCluster, tCmd.m_sIndex, tCmd.m_uBulkChunkId ) );
	if ( !pSession )
		return TlsMsg::Err ( "bulk_import chunk " UINT64_FMT " of table '%s' has no receive session", tCmd.m_uBulkChunkId, tCmd.m_sIndex.cstr() );
	Threads::ScopedCoroMutex_t tLock ( pSession->m_tLock );
	if ( !pSession->BeginPrepare() || !pSession->m_bReceiving )
		return TlsMsg::Err ( "bulk_import chunk has no active receive session" );
	std::unique_ptr<MergeState_t> pMerge { RecvState::GetState ( pSession->m_uKey )->Flush() };
	RecvState::Free ( pSession->m_uKey );
	pSession->m_bReceiving = false;
	ARRAY_FOREACH ( i, pMerge->m_dFilesNew )
	{
		int64_t iSize = sphGetFileSize ( pMerge->m_dFilesNew[i], nullptr );
		if ( iSize!=pMerge->m_dChunks[i].m_iFileSize )
			return TlsMsg::Err ( "bulk_import chunk file '%s' has " INT64_FMT " bytes, expected " INT64_FMT, pMerge->m_dFilesNew[i].cstr(), iSize, pMerge->m_dChunks[i].m_iFileSize );
	}
	TLS_MSG_STRING ( sError );
	FilesTrait_t tRotated;
	if ( !RotateFiles ( *pMerge, tRotated, sError ) )
		return false;
	for ( const auto & sOld : tRotated.m_dOld )
		pSession->m_dFilesRef.Add ( sOld );
	auto pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed || pServed->m_eType!=IndexType_e::RT || pServed->m_sCluster!=tCmd.m_sCluster )
		return TlsMsg::Err ( "bulk_import chunk: table '%s' is not an RT table of cluster '%s'", tCmd.m_sIndex.cstr(), tCmd.m_sCluster.cstr() );
	auto pChunk = LoadIndexerRtBulkChunk ( pServed, pSession->m_sFilebase, sError );
	if ( !pChunk )
		return false;
	{
		RIdx_T<RtIndex_i *> pRt { pServed };
		if ( !pRt->CanAttachDiskIndex ( pChunk.get(), sError ) )
			return false;
		// as on the loading node: the attach command has only the rows written since
		if ( pRt->ForceDiskChunkResult()!=RtActionResult_e::OK )
			return TlsMsg::Err ( "failed to flush RAM chunk of table '%s' before bulk_import attach: %s", tCmd.m_sIndex.cstr(), pRt->GetLastError().cstr() );
	}
	if ( !pSession->Publish ( pChunk ) )
		return TlsMsg::Err ( "bulk_import chunk was cancelled during preparation" );
	return true;
}

void DropBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId )
{
	const uint64_t uKey = BulkChunkSessionKey ( sCluster, sTable, uId );
	auto pSession = FindBulkChunkSession ( uKey );
	if ( !pSession )
	{
		// A restart loses the session, but its staging files still need an abort.
		CSphString sRoot;
		if ( GetBulkChunkRoot ( sTable, nullptr, sRoot ) )
		{
			BulkChunkSessionRefPtr_t pNew { new BulkChunkSession_t ( sCluster, sTable, uId, BulkChunkFilebase ( sRoot, uId ) ) };
			pNew->m_dFilesRef = FindFiles ( SphSprintf ( "%s.*", pNew->m_sFilebase.cstr() ).cstr() );
			pNew->Cancel(); // exclude a concurrent fallback attach while cleanup owns the files
			if ( AddBulkChunkSession ( pNew ) )
				pSession = std::move ( pNew );
			else
				pSession = FindBulkChunkSession ( uKey );
		}
		TlsMsg::ResetErr();
	}
	if ( pSession )
		CancelBulkChunkSession ( pSession );
}

bool AbortBulkChunk ( const ClusterIndexAddLocalRequest_t & tCmd )
{
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': abort", tCmd.m_uBulkChunkId, tCmd.m_sIndex.cstr() );
	DropBulkChunk ( tCmd.m_sCluster, tCmd.m_sIndex, tCmd.m_uBulkChunkId );
	return true;
}

bool AttachBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, CSphString & sError )
{
	const uint64_t uKey = BulkChunkSessionKey ( sCluster, sTable, uId );
	auto pSession = FindBulkChunkSession ( uKey );
	bool bFallback = false;
	if ( !pSession )
	{
		CSphString sRoot;
		if ( !GetBulkChunkRoot ( sTable, &sCluster, sRoot ) )
		{
			sError = TlsMsg::szError();
			return false;
		}
		BulkChunkSessionRefPtr_t pNew { new BulkChunkSession_t ( sCluster, sTable, uId, BulkChunkFilebase ( sRoot, uId ) ) };
		pNew->m_dFilesRef = FindFiles ( SphSprintf ( "%s.*", pNew->m_sFilebase.cstr() ).cstr() );
		if ( AddBulkChunkSession ( pNew ) )
		{
			pSession = std::move ( pNew );
			bFallback = true;
		} else
			pSession = FindBulkChunkSession ( uKey );
		if ( !pSession )
		{
			sError = TlsMsg::szError();
			return false;
		}
	}

	AT_SCOPE_EXIT ( [&] { RemoveBulkChunkSession ( pSession ); } );
	Threads::ScopedCoroMutex_t tLock ( pSession->m_tLock );
	AT_SCOPE_EXIT ( [&] { CleanupBulkChunkSession ( *pSession ); } );
	auto pServed = GetServed ( sTable );
	if ( !pServed || pServed->m_eType!=IndexType_e::RT || pServed->m_sCluster!=sCluster )
	{
		sError.SetSprintf ( "table '%s' is absent or not an RT table of cluster '%s'", sTable.cstr(), sCluster.cstr() );
		return false;
	}
	if ( !sphFileExists ( SphSprintf ( "%s.sph", pSession->m_sFilebase.cstr() ).cstr() ) )
	{
		sError.SetSprintf ( "bulk_import chunk " UINT64_FMT " of table '%s' is not prepared on this node", uId, sTable.cstr() );
		return false;
	}
	if ( bFallback && pSession->BeginPrepare() )
	{
		auto pLoaded = LoadIndexerRtBulkChunk ( pServed, pSession->m_sFilebase, sError );
		if ( !pLoaded )
			return false;
		{
			RIdx_T<RtIndex_i *> pRt { pServed };
			if ( !pRt->CanAttachDiskIndex ( pLoaded.get(), sError ) )
				return false;
		}
		pSession->Publish ( pLoaded );
	}
	auto pChunk = pSession->Take();
	if ( !pChunk )
	{
		sError.SetSprintf ( "bulk_import chunk " UINT64_FMT " of table '%s' has no prepared object", uId, sTable.cstr() );
		return false;
	}
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': take prepared object %p", uId, sTable.cstr(), pChunk.get() );
	WIdx_T<RtIndex_i *> pRt { pServed };
	bool bFatal = false;
	auto eResult = pRt->AttachDiskIndexWithMeta ( pChunk.get(), false, bFatal, sError );
	if ( eResult!=DiskAttachRes_e::NOT_ATTACHED )
		pChunk.release(); // the table owns the chunk now
	return eResult==DiskAttachRes_e::OK;
}


// drop the chunk at the nodes, including staging files left after a restart
static void AbortBulkChunkOnNodes ( ClusterIndexAddLocalRequest_t tReq, const CSphString & sUser, const VecTraits_T<AgentDesc_t> & dDesc )
{
	CSphString sReason = TlsMsg::MoveToString(); // the error that made the abort, the abort keeps it
	tReq.m_bSendFilesSuccess = false;
	auto dNodes = ClusterIndexAddLocal_c::MakeAgents ( dDesc, sUser, ReplicationSstQueryTimeout(), tReq );
	ClusterIndexAddLocal_c tAbort;
	if ( !PerformRemoteTasksWrap ( dNodes, tAbort, tAbort, true ) )
		sphWarning ( "bulk_import chunk " UINT64_FMT " of table '%s': abort failed: %s", tReq.m_uBulkChunkId, tReq.m_sIndex.cstr(), TlsMsg::szError() );
	TlsMsg::ResetErr();
	if ( !sReason.IsEmpty() )
		TlsMsg::Err ( sReason );
}


void AbortBulkChunkOnNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, const VecTraits_T<AgentDesc_t> & dDesc )
{
	if ( dDesc.IsEmpty() )
		return;

	ClusterIndexAddLocalRequest_t tReq;
	tReq.m_sCluster = sCluster;
	tReq.m_sIndex = sTable;
	tReq.m_eIndex = IndexType_e::RT;
	tReq.m_uBulkChunkId = uId;
	AbortBulkChunkOnNodes ( tReq, sUser, dDesc );
}


static bool BulkChunkCancelled()
{
	CSphString sError;
	if ( !CheckIndexerRtBulkCancelled ( sError ) )
		return false;

	TlsMsg::Err ( sError );
	return true;
}


bool ReplicateBulkChunkToNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, StrVec_t dFiles, const VecTraits_T<AgentDesc_t> & dDesc )
{
	assert ( !dDesc.IsEmpty() );
	const int64_t tmStart = sphMicroTimer();

	ClusterIndexAddLocalRequest_t tReq;
	tReq.m_sCluster = sCluster;
	tReq.m_sIndex = sTable;
	tReq.m_eIndex = IndexType_e::RT;
	tReq.m_uBulkChunkId = uId;
	tReq.m_bSendFilesSuccess = true; // prepare; the abort sends false

	// a progress of its own: the progress object of the cluster belongs to SST and ALTER CLUSTER ADD
	auto pProgress = CreateProgress();
	pProgress->Init ( SstProgress_i::Role_e::DONOR );

	SyncSrc_t tSrc { std::move ( dFiles ) };
	if ( !tSrc.CalculateFilesSignatures ( *pProgress ) )
		return false;

	// Physical paths and logical wire names have the same order and hash indices.
	CSphString sBase = SphSprintf ( "repl-" UINT64_FMT, uId );
	for ( auto & sName : tSrc.m_dBaseNames )
	{
		assert ( !strncmp ( sName.cstr(), sBase.cstr(), sBase.Length() ) );
		sName = SphSprintf ( "chunk%s", sName.cstr()+sBase.Length() );
	}

	const int64_t tmHashed = sphMicroTimer();
	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': sha1 of %d files in %.3f sec, sending to %d nodes", uId, sTable.cstr(), tSrc.m_dBaseNames.GetLength(), ( tmHashed-tmStart ) / 1000000.0f, dDesc.GetLength() );

	const int64_t iTimeoutMs = ReplicationSstQueryTimeout();
	bool bOk = [&]
	{
		if ( BulkChunkCancelled() )
			return false;

		// a repeated reserve or prepare finds the session of the first one and fails, so neither is retried
		FileReserveRequest_t tReserve;
		tReserve.m_sCluster = sCluster;
		tReserve.m_sIndex = sTable;
		tReserve.m_pChunks = &tSrc;
		tReserve.m_uBulkChunkId = uId;
		auto dReserve = ClusterFileReserve_c::MakeAgents ( dDesc, sUser, iTimeoutMs, tReserve );
		ClusterFileReserve_c tReserveCmd;
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

		auto dPrepare = ClusterIndexAddLocal_c::MakeAgents ( dDesc, sUser, iTimeoutMs, tReq );
		ClusterIndexAddLocal_c tPrepareCmd;
		return PerformRemoteTasksWrap ( dPrepare, tPrepareCmd, tPrepareCmd, false );
	}();

	if ( !bOk )
		AbortBulkChunkOnNodes ( tReq, sUser, dDesc );

	sphLogDebugRpl ( "bulk_import chunk " UINT64_FMT " of table '%s': %s on %d nodes in %.3f sec%s%s", uId, sTable.cstr(), bOk ? "prepared" : "failed", dDesc.GetLength(), ( sphMicroTimer()-tmStart ) / 1000000.0f, bOk ? "" : ": ", bOk ? "" : TlsMsg::szError() );
	return bOk;
}
