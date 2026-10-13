// Copyright (c) 2026, Manticore Software LTD
// Indexer-assisted RT bulk loader.
#pragma once

#include "std/generics.h"
#include "std/string.h"

#include <memory>

enum class EMYSQL_ERR : WORD;

class ClientSession_c;
class RtIndex_i;
struct SqlStmt_t;

struct IndexerRtBulkFile_t
{
	CSphString	m_sPath;
	int64_t		m_iSize = 0;
};

CSphString GetIndexerRtBulkRoot ( const RtIndex_i & tRt );
bool ListIndexerRtBulkFiles ( const CSphString & sRoot, CSphVector<IndexerRtBulkFile_t> & dFiles, CSphString & sError );
bool RemoveIndexerRtBulkRoot ( const CSphString & sTable, const CSphString & sRoot, CSphString & sError ); // cancels the shared load into sTable first
bool ActivateIndexerRtBulk ( ClientSession_c & tSession, const CSphString & sTable, CSphString & sError );
bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, EMYSQL_ERR & eError, CSphString & sError );
bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError );
void AbortIndexerRtBulkBatch ( ClientSession_c & tSession );
void CleanupIndexerRtBulk ( ClientSession_c & tSession );

// Shared bulk_import (HTTP shared or shared=N): a table has at most one pool of indexer children that takes
// data from any number of concurrent requests and connections; a request without documents publishes it with an
// empty NDJSON line.
enum class SharedBulkResult_e
{
	STAGED,			// data is in the pool
	PUBLISHED,		// the pool was committed
	BUSY,			// commit while other data requests are active; the pool stays open
	CLOSING,		// the pool is being published
	NO_POOL,		// commit, or a shared_join request, without a pool
	PRECONDITION,	// request rejected before touching a pool
	FAILED_INPUT,	// invalid document; it fails the pool
	FAILED_ALREADY,	// the pool failed earlier
	FAILED
};

// One shared bulk_import HTTP request; lives on the handler stack and leaves the pool in its destructor,
// so a broken connection drops only its own unwritten tail.
class SharedBulkRequest_c : public ISphNoncopyable
{
public:
	SharedBulkRequest_c ( const CSphString & sTable, const CSphString & sShared, bool bJoin ); // sShared is N of shared=N, or empty; bJoin is shared_join
	~SharedBulkRequest_c();

	void Begin();
	SharedBulkResult_e StageLine ( Str_t tLine, CSphString & sError ); // one zero-terminated NDJSON document
	SharedBulkResult_e Commit ( CSphString & sError );
	SharedBulkResult_e End ( CSphString & sError );

private:
	struct Impl_t;
	std::unique_ptr<Impl_t> m_pImpl;

	SharedBulkResult_e Stage ( const SqlStmt_t & tStmt, CSphString & sError );
	void Fail ( const CSphString & sError );
};

bool PurgeSharedBulkImport ( const CSphString & sTable, bool bClearSessions, CSphString & sError );
void ShutdownSharedBulkImport();
