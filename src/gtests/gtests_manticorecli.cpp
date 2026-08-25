// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#include <gtest/gtest.h>

#include "manticorecli.h"
#include "daemon/daemon_ipc.h"
#include "fileutils.h"
#include "searchdaemon.h"
#include "searchdlocal.h"
#include "searchdlocalinternal.h"

#include <string>
#include <vector>
#include <cerrno>

#if !_WIN32
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace
{
manticorecli::ParseResult_t Parse ( std::initializer_list<const char *> dArgs )
{
	std::vector<const char *> dArgv { "manticore" };
	dArgv.insert ( dArgv.end(), dArgs.begin(), dArgs.end() );
	return manticorecli::ParseArgs ( (int)dArgv.size(), dArgv.data() );
}
}

#if defined(__linux__)
TEST ( manticore_cli, pidfd_fallback_is_only_for_unsupported_kernels )
{
	EXPECT_TRUE ( IsPidfdUnsupportedError(ENOSYS) );
	EXPECT_TRUE ( IsPidfdUnsupportedError(EINVAL) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(EMFILE) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(ENFILE) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(ENOMEM) );
}
#endif

#if !_WIN32
TEST ( manticore_cli, pid_path_replacement_is_not_unlinked_by_old_owner )
{
	char szDirectory[] = "/tmp/manticore-pid-path-XXXXXX";
	ASSERT_NE ( nullptr, mkdtemp(szDirectory) );
	std::string sPath = std::string(szDirectory) + "/searchd.pid";
	std::string sOwnedPath = sPath + ".owned";
	int iOwnedFD = open ( sPath.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0600 );
	ASSERT_GE ( iOwnedFD, 0 );
	ASSERT_EQ ( 0, rename(sPath.c_str(),sOwnedPath.c_str()) );
	int iReplacementFD = open ( sPath.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0600 );
	ASSERT_GE ( iReplacementFD, 0 );
	ASSERT_EQ ( 6, write(iReplacementFD,"424242",6) );
	close ( iReplacementFD );

	CSphString sError;
	EXPECT_FALSE ( UnlinkFileIfSameDescriptor(iOwnedFD,sPath.c_str(),sError) );
	EXPECT_TRUE ( sphFileExists(sPath.c_str()) );

	close ( iOwnedFD );
	unlink ( sPath.c_str() );
	unlink ( sOwnedPath.c_str() );
	EXPECT_EQ ( 0, rmdir(szDirectory) );
}

TEST ( manticore_cli, owned_pid_path_is_removed_through_quarantine )
{
	char szDirectory[] = "/tmp/manticore-pid-unlink-XXXXXX";
	ASSERT_NE ( nullptr, mkdtemp(szDirectory) );
	std::string sPath = std::string(szDirectory) + "/searchd.pid";
	int iOwnedFD = open ( sPath.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0600 );
	ASSERT_GE ( iOwnedFD, 0 );
	CSphString sError;
	EXPECT_TRUE ( UnlinkFileIfSameDescriptor(iOwnedFD,sPath.c_str(),sError) ) << sError.cstr();
	EXPECT_FALSE ( sphFileExists(sPath.c_str()) );
	close ( iOwnedFD );
	EXPECT_EQ ( 0, rmdir(szDirectory) );
}
#endif

TEST ( manticore_cli, bare_command_selects_automatic_client )
{
	auto tResult = Parse ( {} );
	ASSERT_TRUE ( tResult.m_bOk ) << tResult.m_sError;
	EXPECT_EQ ( tResult.m_tOptions.m_eCommand, manticorecli::Command_e::CLIENT );
	EXPECT_EQ ( tResult.m_tOptions.m_eTarget, manticorecli::Target_e::AUTO );
	EXPECT_FALSE ( tResult.m_tOptions.m_bExecute );
}

TEST ( manticore_cli, recognizes_complete_multiline_sql_input )
{
	using localmode::SqlInputState_e;
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT\n  1"), SqlInputState_e::PENDING );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT\n  1;"), SqlInputState_e::COMPLETE );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 'unterminated\n"), SqlInputState_e::UNTERMINATED_QUOTE );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT ';' AS value; -- trailing comment\n"), SqlInputState_e::COMPLETE );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 'first\nsecond';"), SqlInputState_e::COMPLETE );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT '\\G' AS value"), SqlInputState_e::PENDING );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 1 /* ;\ncomment */;"), SqlInputState_e::COMPLETE );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 1;\nSELECT 2"), SqlInputState_e::PENDING );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 1 /* open"), SqlInputState_e::UNTERMINATED_BLOCK_COMMENT );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT 1; /* open"), SqlInputState_e::UNTERMINATED_BLOCK_COMMENT );
	EXPECT_EQ ( localmode::InspectSqlInput("SELECT\n  1\\G"), SqlInputState_e::COMPLETE );
	EXPECT_EQ ( localmode::InspectSqlInput(nullptr), SqlInputState_e::EMPTY );
}

TEST ( manticore_cli, keeps_vertical_marker_inside_quotes )
{
	auto dStatements = SplitLocalSqlStatements ( "SELECT '\\G' AS value; SELECT 2" );
	ASSERT_EQ ( dStatements.size(), 2 );
	EXPECT_EQ ( dStatements[0], "SELECT '\\G' AS value" );
	EXPECT_EQ ( dStatements[1], "SELECT 2" );
}

TEST ( manticore_cli, extracts_completed_prefix_before_pending_suffix )
{
	std::string sInput = "SELECT 1;\nSELECT\n  2";
	std::string sComplete;
	ASSERT_TRUE ( localmode::ExtractCompleteSqlPrefix(sInput,sComplete) );
	EXPECT_EQ ( sComplete, "SELECT 1;" );
	EXPECT_EQ ( sInput, "\nSELECT\n  2" );
	EXPECT_FALSE ( localmode::ExtractCompleteSqlPrefix(sInput,sComplete) );
}

TEST ( manticore_cli, normalizes_multiline_history_without_extending_line_comments )
{
	EXPECT_EQ ( localmode::NormalizeSqlForHistory("SELECT 1 -- keep semantics\n+ 2 AS value;"), "SELECT 1 + 2 AS value;" );
	EXPECT_EQ ( localmode::NormalizeSqlForHistory("SELECT ';' AS value\\G"), "SELECT ';' AS value \\G" );
}

TEST ( manticore_cli, parses_client_target_overrides )
{
	auto tLocal = Parse ( { "--local" } );
	ASSERT_TRUE ( tLocal.m_bOk ) << tLocal.m_sError;
	EXPECT_EQ ( tLocal.m_tOptions.m_eTarget, manticorecli::Target_e::LOCAL );

	auto tGlobal = Parse ( { "--global", "-e", "SELECT 1" } );
	ASSERT_TRUE ( tGlobal.m_bOk ) << tGlobal.m_sError;
	EXPECT_EQ ( tGlobal.m_tOptions.m_eTarget, manticorecli::Target_e::GLOBAL );
	EXPECT_TRUE ( tGlobal.m_tOptions.m_bExecute );
	EXPECT_EQ ( tGlobal.m_tOptions.m_sQuery, "SELECT 1" );

	auto tConfig = Parse ( { "--config", "/tmp/manticore.conf" } );
	ASSERT_TRUE ( tConfig.m_bOk ) << tConfig.m_sError;
	EXPECT_EQ ( tConfig.m_tOptions.m_eTarget, manticorecli::Target_e::GLOBAL );
	EXPECT_EQ ( tConfig.m_tOptions.m_sConfig, "/tmp/manticore.conf" );

	auto tRemote = Parse ( { "-h", "search.example.com", "-P", "9306", "-e", "SELECT 1" } );
	ASSERT_TRUE ( tRemote.m_bOk ) << tRemote.m_sError;
	EXPECT_EQ ( tRemote.m_tOptions.m_eTarget, manticorecli::Target_e::REMOTE );
	EXPECT_EQ ( tRemote.m_tOptions.m_sHost, "search.example.com" );
	EXPECT_EQ ( tRemote.m_tOptions.m_iPort, 9306 );

	auto tHostOnly = Parse ( { "--host", "db.internal" } );
	ASSERT_TRUE ( tHostOnly.m_bOk ) << tHostOnly.m_sError;
	EXPECT_EQ ( tHostOnly.m_tOptions.m_eTarget, manticorecli::Target_e::REMOTE );
	EXPECT_EQ ( tHostOnly.m_tOptions.m_sHost, "db.internal" );
	EXPECT_EQ ( tHostOnly.m_tOptions.m_iPort, 9306 );

	auto tPortOnly = Parse ( { "--port", "19306" } );
	ASSERT_TRUE ( tPortOnly.m_bOk ) << tPortOnly.m_sError;
	EXPECT_EQ ( tPortOnly.m_tOptions.m_eTarget, manticorecli::Target_e::REMOTE );
	EXPECT_EQ ( tPortOnly.m_tOptions.m_sHost, "127.0.0.1" );
	EXPECT_EQ ( tPortOnly.m_tOptions.m_iPort, 19306 );
}

TEST ( manticore_cli, rejects_conflicting_client_targets_and_missing_values )
{
	EXPECT_FALSE ( Parse ( { "--local", "--global" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--local", "--config", "/tmp/manticore.conf" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--local", "--host", "127.0.0.1" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--config", "/tmp/manticore.conf", "--port", "9306" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--host" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--host", "" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--port" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--port", "0" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--port", "65536" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--port", "9306x" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "-e" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--config" } ).m_bOk );
}

TEST ( manticore_cli, help_uses_question_mark_after_h_becomes_host )
{
	auto tHelp = Parse ( { "-?" } );
	ASSERT_TRUE ( tHelp.m_bOk ) << tHelp.m_sError;
	EXPECT_EQ ( tHelp.m_tOptions.m_eCommand, manticorecli::Command_e::HELP );
	EXPECT_FALSE ( Parse ( { "-h" } ).m_bOk );
}

TEST ( manticore_cli, parses_lifecycle_targets )
{
	for ( auto eCommand : { manticorecli::Command_e::START, manticorecli::Command_e::STOP, manticorecli::Command_e::STATUS } )
	{
		const char * szCommand = eCommand==manticorecli::Command_e::START ? "start" : eCommand==manticorecli::Command_e::STOP ? "stop" : "status";
		auto tAuto = Parse ( { szCommand } );
		ASSERT_TRUE ( tAuto.m_bOk ) << tAuto.m_sError;
		EXPECT_EQ ( tAuto.m_tOptions.m_eCommand, eCommand );
		EXPECT_EQ ( tAuto.m_tOptions.m_eTarget, manticorecli::Target_e::AUTO );

		auto tLocal = Parse ( { szCommand, "local" } );
		ASSERT_TRUE ( tLocal.m_bOk ) << tLocal.m_sError;
		EXPECT_EQ ( tLocal.m_tOptions.m_eTarget, manticorecli::Target_e::LOCAL );

		auto tGlobal = Parse ( { szCommand, "global" } );
		ASSERT_TRUE ( tGlobal.m_bOk ) << tGlobal.m_sError;
		EXPECT_EQ ( tGlobal.m_tOptions.m_eTarget, manticorecli::Target_e::GLOBAL );
	}
}

TEST ( manticore_cli, rejects_removed_or_ambiguous_commands )
{
	EXPECT_FALSE ( Parse ( { "stopwait" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "start", "somewhere" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "status", "local", "extra" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "start", "--local" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "SELECT 1" } ).m_bOk );
}

TEST ( manticore_cli, resolves_automatic_target_without_falling_through_invalid_marker )
{
	EXPECT_EQ ( manticorecli::ResolveTarget ( manticorecli::Target_e::AUTO, manticorecli::Marker_e::ABSENT ), manticorecli::Target_e::GLOBAL );
	EXPECT_EQ ( manticorecli::ResolveTarget ( manticorecli::Target_e::AUTO, manticorecli::Marker_e::DIRECTORY ), manticorecli::Target_e::LOCAL );
	EXPECT_EQ ( manticorecli::ResolveTarget ( manticorecli::Target_e::AUTO, manticorecli::Marker_e::INVALID ), manticorecli::Target_e::LOCAL );
	EXPECT_EQ ( manticorecli::ResolveTarget ( manticorecli::Target_e::GLOBAL, manticorecli::Marker_e::DIRECTORY ), manticorecli::Target_e::GLOBAL );
	EXPECT_EQ ( manticorecli::ResolveTarget ( manticorecli::Target_e::LOCAL, manticorecli::Marker_e::ABSENT ), manticorecli::Target_e::LOCAL );
}

#if !_WIN32
TEST ( manticore_cli, listener_resolution_prefers_non_loopback_when_first_address_is_loopback )
{
	DWORD dAddresses[] = { htonl(0x7f000001), htonl(0x0a000007), htonl(0xc0000201) };
	EXPECT_EQ ( SelectListenerAddress ( dAddresses, 3 ), dAddresses[1] );
	EXPECT_EQ ( SelectListenerAddress ( dAddresses+1, 2 ), dAddresses[1] );
}
#endif
