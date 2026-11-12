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

// A chunk that bulk_import built for a cluster table goes to every node of the cluster before the cluster attaches it.
// A node receives the files into repl-<id>.recv/ in the staging root of the table, checks the chunk and renames the
// directory to repl-<id>/: a chunk is prepared when repl-<id>/chunk.sph exists. Every chunk has its own id, the
// receive session is keyed by (cluster, table, id), so it never meets the SST session of the table.

struct BulkChunkRequest_t : ClusterRequest_t
{
	CSphString	m_sTable;
	uint64_t	m_uId = 0;
};

uint64_t NewBulkChunkId(); // unique across the nodes and the restarts of this one, never 0
uint64_t BulkChunkSessionKey ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId );
CSphString BulkChunkDir ( const CSphString & sRoot, uint64_t uId, bool bReceiving );

// whether this node receives, keeps prepared or attaches a chunk of the table
bool HasBulkChunkSessions ( const CSphString & sTable );

// PURGE with clear_sessions=1 forgets the chunks of the table; a receive still running fails on its next packet and its
// state goes with CHUNK_ABORT, a state left by a gone loading node stays in memory until restart
void ForgetBulkChunkSessions ( const CSphString & sTable );

// the loading node keeps its own copy of the chunk in repl-<id>/ until the command
void RegisterBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId );

// The attach command BULK_IMPORT_ATTACH. AttachBulkChunk() moves the prepared chunk into the table, DropBulkChunk()
// removes it when the command skips it. Both remove repl-<id>/ and the session in any case.
bool AttachBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId, CSphString & sError );
void DropBulkChunk ( const CSphString & sCluster, const CSphString & sTable, uint64_t uId );

// what the attach command did on the loading node
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

bool ReceiveClusterChunkReserve ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs );
bool ReceiveClusterChunkPrepare ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs );
bool ReceiveClusterChunkAbort ( GenericOutputBuffer_c & tOut, InputBuffer_c & tBuf, CSphString & sCluster, WORD uReplyVersion, DWORD uHeartbeatIntervalMs );

// Send the chunk files dFiles (full paths) to the nodes dDesc: hashes, CHUNK_RESERVE, FILE_SEND and CHUNK_PREPARE, with a
// check of a shutdown or KILL between the steps. On a failure every node gets CHUNK_ABORT and TlsMsg has the reason.
bool ReplicateBulkChunkToNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, StrVec_t dFiles, const VecTraits_T<AgentDesc_t> & dDesc );
void AbortBulkChunkOnNodes ( const CSphString & sCluster, const CSphString & sUser, const CSphString & sTable, uint64_t uId, const VecTraits_T<AgentDesc_t> & dDesc );
