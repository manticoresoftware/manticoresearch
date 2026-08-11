// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#pragma once

#include <string>

namespace manticorecli
{
enum class Command_e
{
	CLIENT,
	START,
	STOP,
	STATUS,
	HELP,
	VERSION
};

enum class Target_e
{
	AUTO,
	LOCAL,
	GLOBAL
};

enum class Marker_e
{
	ABSENT,
	DIRECTORY,
	INVALID,
	ERROR_
};

struct Options_t
{
	Command_e m_eCommand = Command_e::CLIENT;
	Target_e m_eTarget = Target_e::AUTO;
	std::string m_sConfig;
	std::string m_sQuery;
	bool m_bExecute = false;
};

struct ParseResult_t
{
	bool m_bOk = false;
	Options_t m_tOptions;
	std::string m_sError;
};

ParseResult_t ParseArgs ( int iArgc, const char * const * dArgv );
Target_e ResolveTarget ( Target_e eRequested, Marker_e eMarker );
Marker_e InspectLocalMarker ( const char * szDirectory, std::string & sError );
int Run ( const Options_t & tOptions, const char * szArgv0 );
void ShowHelp();
}
