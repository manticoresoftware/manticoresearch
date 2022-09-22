//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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
#include "sphinxpq.h"

class ClientSession_c
{
public:
	CSphString m_sError;
	CSphQueryResultMeta m_tLastMeta;
	CSphSessionAccum m_tAcc;
	CPqResult m_tPercolateMeta;
	SqlStmt_e m_eLastStmt { STMT_DUMMY };
	bool m_bFederatedUser = false;
	CSphString m_sFederatedQuery;

public:
	bool m_bAutoCommit = true;
	bool m_bInTransaction = false;
	CSphVector<int64_t> m_dLastIds;
	QueryProfile_c m_tProfile;
	QueryProfile_c m_tLastProfile;
	bool m_bOptimizeById = true;

public:
	bool Execute ( Str_t sQuery, RowBuffer_i& tOut );
	void FreezeLastMeta();
};
