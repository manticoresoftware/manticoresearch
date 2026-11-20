// Copyright (c) 2026, Manticore Software LTD
// Indexer-assisted RT bulk loader.
#pragma once

#include "searchdsql.h"

#include <memory>

enum class EMYSQL_ERR : WORD;

class ClientSession_c;
class RtIndex_i;
class CSphIndex;

struct IndexerRtBulkFile_t
{
	CSphString	m_sPath;
	int64_t		m_iSize = 0;
};

CSphString GetIndexerRtBulkRoot ( const RtIndex_i & tRt );

// takes the common and indexer options of the main config for the indexer children; call after sphConfigureCommon
void ConfigureIndexerRtBulk ( const CSphConfig & hConf );

std::unique_ptr<CSphIndex> LoadIndexerRtBulkChunk ( const CSphString & sPath, const CSphString & sTable, CSphString & sError );

bool PrepareStagingRoot ( const CSphString & sRoot, CSphString & sError );

bool ListIndexerRtBulkFiles ( const CSphString & sRoot, CSphVector<IndexerRtBulkFile_t> & dFiles, CSphString & sError );
bool RemoveIndexerRtBulkRoot ( const CSphString & sTable, const CSphString & sRoot, CSphString & sError ); // cancels the shared load into sTable first
bool ActivateIndexerRtBulk ( ClientSession_c & tSession, const CSphString & sTable, CSphString & sError, bool bShard=false );
bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, EMYSQL_ERR & eError, CSphString & sError );
bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError );
void AbortIndexerRtBulkBatch ( ClientSession_c & tSession );
bool RollbackIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError );
void CleanupIndexerRtBulk ( ClientSession_c & tSession );

// CSV of INSERT statements for the generated indexer config; a bulk_import into a shard table builds it with the
// schema of the shard table and routes its rows.

// one CSV column: the INSERT column of its value (-1 for none) and the type that formats it
struct IndexerRtBulkColumn_t
{
	int m_iColumn;
	ESphAttr m_eType;
	bool m_bDocid;
};

// INSERT column of every schema attribute and field (-1 for none) and of the id
struct IndexerRtBulkInputMap_t
{
	CSphFixedVector<int> m_dAttrs { 0 };
	CSphFixedVector<int> m_dFields { 0 };
	int m_iId = -1;
};

void GetIndexerRtBulkCsvAttrOrder ( const CSphSchema & tSchema, CSphVector<int> & dCsvAttrOrder );
bool CheckIndexerRtBulkCancelled ( CSphString & sError );

// The insert layout of a request: where the columns of its statements go in the CSV of a table. Prepare() checks the
// values of a statement with the columns of the previous one, as an ordinary INSERT checks them; other columns
// rebuild the layout.
struct IndexerRtBulkLayout_t
{
	bool m_bPrepared = false;

	bool Prepare ( const CSphSchema & tSchema, const CSphVector<int> & dCsvAttrOrder, const SqlStmt_t & tStmt, CSphString & sError );
	uint64_t DocId ( const SqlStmt_t & tStmt, int iRow ) const;
	void AppendRow ( StringBuilder_c & sOut, const SqlStmt_t & tStmt, int iRow, uint64_t uId ) const;

private:
	StrVec_t m_dInsertSchema;
	int m_iSchemaSz = 0;
	IndexerRtBulkInputMap_t m_tMap;
	CSphVector<IndexerRtBulkColumn_t> m_dColumns;
};

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
int GetSharedBulkBatchBytes();
int GetSharedBulkWaitMs(); // a wait for the pool wakes up this often only to notice KILL

// A physical target of a Shared bulk_import into a shard table: a shared pool of the table with one child that
// belongs to the load iLoadId. Ordinary shared requests do not join, commit or fail it; PURGE BULK_IMPORT by the
// table name cancels it as any pool. For a local target (iIndexId>=0) the start checks the index id and alter
// generation that the shard table captured, as an ordinary write checks them; a remote target has the schema of the
// shard table, as SHARD_WRITE assumes.
SharedBulkResult_e StartShardBulkTarget ( const CSphString & sTable, int64_t iLoadId, const CSphString & sShard, int64_t iIndexId, int iAlterGeneration, CSphString & sError );
SharedBulkResult_e WriteShardBulkTarget ( const CSphString & sTable, int64_t iLoadId, Str_t tCsv, int64_t iRows, bool & bRequestOnly, CSphString & sError );
SharedBulkResult_e CommitShardBulkTarget ( const CSphString & sTable, int64_t iLoadId, CSphString & sError );
// Commit several local targets of one load at once: EOF to all of them first, so their children build together, then
// wait for and attach each. dOk[i] tells whether dTables[i] was published; dErrors[i] says why not.
void CommitShardBulkTargets ( const StrVec_t & dTables, int64_t iLoadId, CSphFixedVector<bool> & dOk, CSphFixedVector<CSphString> & dErrors );
bool CancelShardBulkTarget ( const CSphString & sTable, int64_t iLoadId, CSphString & sError ); // true also when there is nothing to cancel
