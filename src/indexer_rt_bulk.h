// Copyright (c) 2026, Manticore Software LTD
// Indexer-assisted RT bulk loader.
#pragma once

#include "std/string.h"

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
bool RemoveIndexerRtBulkRoot ( const CSphString & sRoot, CSphString & sError );
bool ActivateIndexerRtBulk ( ClientSession_c & tSession, const CSphString & sTable, CSphString & sError, bool * pRetryable=nullptr );
bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, EMYSQL_ERR & eError, CSphString & sError );
bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError );
void AbortIndexerRtBulkBatch ( ClientSession_c & tSession );
void CleanupIndexerRtBulk ( ClientSession_c & tSession );
