// Copyright (c) 2017-2026, Manticore Software LTD
// Prototype implementation for dev#2761.
#pragma once

class ClientSession_c;
struct SqlStmt_t;
struct CSphString;

bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, CSphString & sError );
bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError );
void CleanupIndexerRtBulk ( ClientSession_c & tSession );
