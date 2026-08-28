//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxstd.h"
#include "sphinx.h"
#include "queryprofile.h"
#include "searchdaemon.h"
#include "searchdsql.h"
#include "searchd_shard.h"
#include "sphinxpq.h"

constexpr const char* szManticore = "Manticore";

struct BinaryPreparedStmt_t;
class PreparedStatements
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	PreparedStatements();
	~PreparedStatements();
	DWORD GetNextStmtID();
	void AddPreparedStatement (DWORD uStmtID, BinaryPreparedStmt_t&& tStmt);
	BinaryPreparedStmt_t* GetStmt(DWORD uStmtID);
	void RemoveStatement (DWORD uStmtID);
};

struct IndexerRtBulkState_t
{
	class Impl_c;

	int64_t m_iIndexId = -1;
	int m_iAlterGeneration = -1;
	CSphString m_sTable;
	CSphString m_sDir;
	ServedIndexWriteReservation_c m_tReservation;
	std::unique_ptr<Impl_c> m_pImpl;

	IndexerRtBulkState_t();
	~IndexerRtBulkState_t();
	bool IsEnabled() const { return m_tReservation.IsActive(); }
	bool HasPendingData() const { return !!m_pImpl; }
};

class ClientSession_c final
{
public:
	CSphString m_sError;
	CSphQueryResultMeta m_tLastMeta;
	CSphSessionAccum m_tAcc;
	ShardTxnState_t m_tShardTxn;
	CPqResult m_tPercolateMeta;
	SqlStmt_e m_eLastStmt { STMT_DUMMY };
	bool m_bFederatedUser = false;
	CSphString m_sFederatedQuery;
	CSphString m_sUser;
	CSphString m_sCurrentDbName { szManticore };

public:
	bool m_bAutoCommit = true;
	bool m_bInTransaction = false;
	IndexerRtBulkState_t m_tIndexerRtBulk;
	CSphVector<int64_t> m_dLastIds;
	CSphVector<CSphString> m_dLastIdStrings;
	QueryProfile_c m_tProfile;
	QueryProfile_c m_tLastProfile;
	bool m_bOptimizeById = true;
	bool m_bDeprecatedEOF = false;
	bool m_bShardPhysicalUpdate = false;
	StrVec_t m_dLockedTables;
	PreparedStatements m_dPreparedStatements;
	bool m_bAuthAllowBuddy = false;
	bool m_bAuthErrorSkipBuddy = false;

public:
	NONCOPYMOVABLE ( ClientSession_c );
	ClientSession_c() = default;
	bool Execute ( Str_t sQuery, RowBuffer_i& tOut );
	void FreezeLastMeta();
	~ClientSession_c();
};
