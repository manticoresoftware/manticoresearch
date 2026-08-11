// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#include <gtest/gtest.h>

#include "manticorecli.h"
#include "daemon/daemon_ipc.h"
#include "fileutils.h"
#include "searchdaemon.h"

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
}

TEST ( manticore_cli, rejects_conflicting_client_targets_and_missing_values )
{
	EXPECT_FALSE ( Parse ( { "--local", "--global" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--local", "--config", "/tmp/manticore.conf" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "-e" } ).m_bOk );
	EXPECT_FALSE ( Parse ( { "--config" } ).m_bOk );
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
