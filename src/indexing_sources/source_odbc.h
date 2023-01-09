//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
// source_odbc supports indexing both MSSQL (on windows), and ODBC (on others) via UnixODBC/iODBC library

#pragma once

#include "source_sql.h"

struct CSphSourceParams_ODBC: CSphSourceParams_SQL
{
	CSphString	m_sOdbcDSN;			///< ODBC DSN
	CSphString	m_sColBuffers;		///< column buffer sizes (eg "col1=2M, col2=4M")
	bool		m_bWinAuth = false;	///< auth type (MS SQL only)
};

/// ODBC source implementation
CSphSource * CreateSourceODBC ( const CSphSourceParams_ODBC & tParams, const char * sSourceName );
CSphSource * CreateSourceMSSQL ( const CSphSourceParams_ODBC & tParams, const char * sSourceName );
