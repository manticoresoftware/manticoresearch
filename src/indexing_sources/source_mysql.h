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

#pragma once

#include "source_sql.h"

/// MySQL source params
struct CSphSourceParams_MySQL : CSphSourceParams_SQL
{
	CSphString	m_sUsock;					///< UNIX socket
	int			m_iFlags;					///< connection flags
	CSphString	m_sSslKey;
	CSphString	m_sSslCert;
	CSphString	m_sSslCA;

	CSphSourceParams_MySQL ();	///< ctor. sets defaults
};

/// MysqlSQL source implementation
CSphSource * CreateSourceMysql ( const CSphSourceParams_MySQL & tParams, const char * sSourceName );
