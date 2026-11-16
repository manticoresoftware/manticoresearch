//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

// Shared bulk_import into a shard table: one load on the node that takes the requests (the coordinator) routes rows
// as an ordinary write into the shard table does and feeds a shared pool of one indexer child on every physical
// target, which the first request starts; the commit publishes all the targets at once.

#pragma once

#include "indexer_rt_bulk.h"
#include "searchdha.h"

class SearchFailuresLog_c;

class ShardBulkRequest_c : public ISphNoncopyable
{
public:
	ShardBulkRequest_c ( const CSphString & sTable, const CSphString & sShared, bool bJoin );
	~ShardBulkRequest_c();

	void Begin();
	SharedBulkResult_e StageLine ( Str_t tLine, CSphString & sError );
	SharedBulkResult_e Stage ( const SqlStmt_t & tStmt, CSphString & sError );
	SharedBulkResult_e Commit ( CSphString & sError );
	SharedBulkResult_e End ( CSphString & sError );

private:
	struct Impl_t;
	std::unique_ptr<Impl_t> m_pImpl;
};

// PURGE BULK_IMPORT by the name of a distributed or shard table: the load of a shard table that this node runs, and
// the same command on every mirror of every agent table. The local tables are purged by the caller.
bool PurgeShardBulkLoad ( const CSphString & sTable, bool bClearSessions, CSphString & sError );
void PurgeRemoteBulkImport ( const DistributedIndex_t & tDistr, const CSphString & sTable, bool bClearSessions, SearchFailuresLog_c & dFailures );

void DropShardBulkImport ( const CSphString & sTable );

// The load into the shard table that this node runs, from its first document until its commit ends; a failed load,
// whose targets are cancelled, is not active.
bool HasActiveShardBulkLoad ( const CSphString & sTable );
void ShutdownShardBulkImport(); // fails the loads; remote targets get no cancel, PURGE cleans them up

void HandleCommandShardBulk ( GenericOutputBuffer_c & tOut, WORD uVer, InputBuffer_c & tReq );
