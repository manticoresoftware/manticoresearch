// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Local zero-configuration searchd startup and query client.

#pragma once

#include "sphinxutils.h"

#include <string>
#include <vector>

/// Create/validate ./manticore_data and build the in-memory local searchd configuration.
bool BuildLocalSearchdConfig ( CSphConfig & hConf, CSphString & sDataDir, CSphString & sError );

/// Split a local -e SQL batch without treating delimiters in quoted values,
/// quoted identifiers, escapes, or comments as statement boundaries.
std::vector<std::string> SplitLocalSqlStatements ( const char * szSql );

/// Execute one or more SQL statements against ./manticore_data/searchd.sock.
/// A null or empty query starts the simple line-oriented interactive client.
int ExecuteLocalSql ( const char * sQuery );
