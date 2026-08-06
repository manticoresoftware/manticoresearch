// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Private interfaces for configless local searchd mode.

#pragma once

#include "sphinxutils.h"

#include <memory>
#include <string>
#include <functional>
#include <vector>

namespace localmode
{
std::string TrimInput ( const std::string & sValue );
bool PrintSqlResponse ( const std::string & sBody, CSphString & sError, bool bInteractive, int64_t iElapsedUS=-1, bool bAffectedRows=false, bool bVertical=false );
using CompletionProvider_fn = std::function<std::vector<std::string> ( const std::string & )>;

enum class QueryResult_e { OK, SQL_ERROR, CONNECTION_ERROR };

#if !_WIN32
int ConnectSocket ( CSphString & sError );
QueryResult_e ExecuteSqlBatch ( int & iSocket, const char * szQuery, bool bPrintStatements, bool bAligned );
#endif

class LineEditor_i
{
public:
	virtual ~LineEditor_i() = default;
	virtual bool Read ( std::string & sLine ) = 0;
	virtual bool AddHistory ( const std::string & sLine, CSphString & sError ) = 0;
	virtual void RefreshCompletions ( const std::string & sLine, bool bSuccessful ) = 0;
};

std::unique_ptr<LineEditor_i> CreateLineEditor ( const CSphString & sDataDir, CSphString & sWarning, CompletionProvider_fn fnCompletionProvider={} );
CompletionProvider_fn CreateCompletionProvider();
}
