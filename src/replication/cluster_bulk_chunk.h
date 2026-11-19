//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "api_command_cluster.h"
#include "coroutine.h"
#include "std/refcounted_mt.h"
#include <atomic>
#include <memory>

// Each node receives repl-<id>.<ext>.new in the table's staging root by the SST commands with the id of the chunk, then
// keeps the loaded chunk in its session until the ordered attach command. The wire uses logical chunk.<ext> names.
class CSphIndex;
struct FileReserveRequest_t;
struct FileReserveReply_t;
struct ClusterIndexAddLocalRequest_t;

uint64_t NewBulkChunkId(); // unique across the nodes and the restarts of this one, never 0
uint64_t BulkChunkSessionKey ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId );
CSphString BulkChunkFilebase ( const CSphString & sRoot, uint64_t uId );

enum class BulkChunkState_e { RECEIVING, PREPARING, READY, APPLYING, CANCELLED, FINISHED };

// Stable operation state. The operation lock protects paths and the unique owner; cancellation can prevent a
// late READY without waiting for Load. No table lock is held when waiting for this lock.
struct BulkChunkSession_t : ISphRefcountedMT
{
	BulkChunkSession_t ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, const CSphString & sFilebase );
	~BulkChunkSession_t() override;

	bool BeginPrepare();
	bool Publish ( std::unique_ptr<CSphIndex> & pChunk );
	std::unique_ptr<CSphIndex> Take();
	void Cancel();
	void Finish();

	CSphString m_sCluster;
	CSphString m_sTable;
	CSphString m_sFilebase;
	uint64_t m_uKey;
	Threads::Coro::Mutex_c m_tLock;
	std::atomic<BulkChunkState_e> m_eState { BulkChunkState_e::RECEIVING };
	std::unique_ptr<CSphIndex> m_pChunk;
	StrVec_t m_dFilesNew;
	StrVec_t m_dFilesRef;
	bool m_bReceiving = false;
};

using BulkChunkSessionRefPtr_t = RefCountedRefPtr_T<BulkChunkSession_t>;

bool HasBulkChunkSessions ( const CSphString & sTable );

// Close objects before removing their captured staging files, outside the registry lock.
void ForgetBulkChunkSessions ( const CSphString & sTable );
void ForgetClusterBulkChunkSessions ( const CSphString & sCluster );
void ShutdownBulkChunkSessions();

// The sender transfers its already loaded (and KNN-built) object to the same session used by receivers.
bool RegisterBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, std::unique_ptr<CSphIndex> pChunk, const VecTraits_T<CSphString> & dFiles, CSphString & sError );

// The attach command BULK_IMPORT_ATTACH. AttachBulkChunk() moves the prepared chunk into the table, DropBulkChunk()
// removes it when the command skips it. Cleanup uses captured staging paths even after the table is replaced.
bool AttachBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, CSphString & sError );
void DropBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId );

enum class BulkChunkAttach_e
{
	NOT_APPLIED,	// the command was not ordered: the nodes still keep the chunk
	ATTACHED,
	SKIPPED,		// the table left the cluster before the command
	FAILED			// a local failure on this node, the other nodes attached the chunk
};

struct BulkChunkAttach_t
{
	BulkChunkAttach_e	m_eResult = BulkChunkAttach_e::NOT_APPLIED;
	CSphString			m_sError;
};

// The chunk side of CLUSTER_FILE_RESERVE and CLUSTER_INDEX_ADD_LOCAL with a chunk id: reserve the files, then prepare
// (load and check) or abort the chunk.
bool ReserveBulkChunk ( const FileReserveRequest_t & tCmd, FileReserveReply_t & tRes );
bool PrepareBulkChunk ( const ClusterIndexAddLocalRequest_t & tCmd );
bool AbortBulkChunk ( const ClusterIndexAddLocalRequest_t & tCmd );

// Send the chunk files dFiles (full paths) to the nodes dDesc: hashes, FILE_RESERVE, FILE_SEND and INDEX_ADD_LOCAL, all
// with the chunk id, with a check of a shutdown or KILL between the steps. On a failure every node gets INDEX_ADD_LOCAL
// that aborts the chunk, and TlsMsg has the reason.
bool ReplicateBulkChunkToNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, StrVec_t dFiles, const VecTraits_T<AgentDesc_t> & dDesc );
void AbortBulkChunkOnNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, const VecTraits_T<AgentDesc_t> & dDesc );
