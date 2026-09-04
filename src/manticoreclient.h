// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#pragma once

#include <string>

enum class ManticoreClientTarget_e
{
	AUTO,
	LOCAL,
	GLOBAL,
	REMOTE
};

/// Run the Manticore SQL client against an inferred or explicitly selected target.
/// A null query starts interactive/stdin mode.
int ExecuteManticoreSql ( const char * szQuery, ManticoreClientTarget_e eTarget=ManticoreClientTarget_e::AUTO, const char * szConfigFile=nullptr, const char * szHost=nullptr, int iPort=9306 );

/// Execute one silent readiness query against the selected instance.
bool ProbeManticoreReady ( ManticoreClientTarget_e eTarget, const char * szConfigFile, std::string & sError, int64_t iDeadlineUS=0 );
