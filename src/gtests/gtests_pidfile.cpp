// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)

#include <gtest/gtest.h>

#include "daemon/daemon_ipc.h"
#include "fileutils.h"

#include <cerrno>
#include <string>

#if !_WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
class PidFileTest_c : public testing::Test
{
protected:
	void SetUp() override
	{
		char szTemplate[] = "/tmp/manticore-pid-test-XXXXXX";
		char * szDirectory = mkdtemp ( szTemplate );
		ASSERT_NE ( nullptr, szDirectory );
		m_sDirectory = szDirectory;
		m_sPath = m_sDirectory + "/searchd.pid";
	}

	void TearDown() override
	{
		unlink ( m_sPath.c_str() );
		unlink ( ( m_sPath+".target" ).c_str() );
		rmdir ( m_sDirectory.c_str() );
	}

	void Write ( const std::string & sValue )
	{
		int iFD = open ( m_sPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600 );
		ASSERT_GE ( iFD, 0 );
		ASSERT_EQ ( (ssize_t)sValue.size(), write ( iFD, sValue.data(), sValue.size() ) );
		ASSERT_EQ ( 0, close(iFD) );
	}

	bool Open ( int & iFD, int & iPid, CSphString & sError )
	{
		return OpenPidFile ( m_sPath.c_str(), iFD, iPid, sError );
	}

	std::string m_sDirectory;
	std::string m_sPath;
};
}

TEST_F ( PidFileTest_c, parses_only_a_positive_decimal_pid_with_optional_line_ending )
{
	for ( const char * szValid : { "1", "42\n", "314\r\n" } )
	{
		Write ( szValid );
		int iFD = -1;
		int iPid = 0;
		CSphString sError;
		ASSERT_TRUE ( Open(iFD,iPid,sError) ) << sError.cstr();
		EXPECT_EQ ( atoi(szValid), iPid );
		SafeClose ( iFD );
	}

	for ( const char * szInvalid : { "", "0", "-1", "+1", " 1", "1 ", "1x", "1\n\n", "2147483648", "99999999999999999999999999999999999999999999999999999999999999999" } )
	{
		Write ( szInvalid );
		int iFD = -1;
		int iPid = 0;
		CSphString sError;
		EXPECT_FALSE ( Open(iFD,iPid,sError) ) << szInvalid;
		EXPECT_EQ ( -1, iFD ) << szInvalid;
		EXPECT_EQ ( 0, iPid ) << szInvalid;
	}
}

TEST_F ( PidFileTest_c, rejects_symlinks_and_non_regular_files )
{
	std::string sTarget = m_sPath + ".target";
	int iTargetFD = open ( sTarget.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0600 );
	ASSERT_GE ( iTargetFD, 0 );
	ASSERT_EQ ( 2, write(iTargetFD,"42",2) );
	ASSERT_EQ ( 0, close(iTargetFD) );
	ASSERT_EQ ( 0, symlink(sTarget.c_str(),m_sPath.c_str()) );

	int iFD = -1;
	int iPid = 0;
	CSphString sError;
	EXPECT_FALSE ( Open(iFD,iPid,sError) );
	EXPECT_EQ ( -1, iFD );
	ASSERT_EQ ( 0, unlink(m_sPath.c_str()) );

	ASSERT_EQ ( 0, mkfifo(m_sPath.c_str(),0600) );
	sError = nullptr;
	EXPECT_FALSE ( Open(iFD,iPid,sError) );
	EXPECT_EQ ( -1, iFD );
}

TEST_F ( PidFileTest_c, validates_that_the_recorded_pid_owns_the_write_lock )
{
	Write ( "1" );
	int dReady[2];
	int dDone[2];
	ASSERT_EQ ( 0, pipe(dReady) );
	ASSERT_EQ ( 0, pipe(dDone) );
	pid_t iChild = fork();
	ASSERT_GE ( iChild, 0 );
	if ( iChild==0 )
	{
		close ( dReady[0] );
		close ( dDone[1] );
		int iFD = open ( m_sPath.c_str(), O_RDWR );
		struct flock tLock {};
		tLock.l_type = F_WRLCK;
		tLock.l_whence = SEEK_SET;
		char cReady = iFD>=0 && fcntl(iFD,F_SETLK,&tLock)==0 ? '1' : '0';
		write ( dReady[1], &cReady, 1 );
		char cDone;
		read ( dDone[0], &cDone, 1 );
		if ( iFD>=0 ) close ( iFD );
		_exit ( cReady=='1' ? 0 : 1 );
	}

	close ( dReady[1] );
	close ( dDone[0] );
	char cReady = '0';
	ASSERT_EQ ( 1, read(dReady[0],&cReady,1) );
	ASSERT_EQ ( '1', cReady );
	close ( dReady[0] );

	Write ( std::to_string(iChild) );
	int iFD = -1;
	int iPid = 0;
	CSphString sError;
	ASSERT_TRUE ( Open(iFD,iPid,sError) ) << sError.cstr();
	EXPECT_TRUE ( ValidatePidFileOwner(iFD,iPid,sError) ) << sError.cstr();
	sError = nullptr;
	EXPECT_FALSE ( ValidatePidFileOwner(iFD,iPid+1,sError) );
	SafeClose ( iFD );

	ASSERT_EQ ( 1, write(dDone[1],"x",1) );
	close ( dDone[1] );
	int iStatus = 0;
	ASSERT_EQ ( iChild, waitpid(iChild,&iStatus,0) );
	ASSERT_TRUE ( WIFEXITED(iStatus) );
	ASSERT_EQ ( 0, WEXITSTATUS(iStatus) );

	ASSERT_TRUE ( Open(iFD,iPid,sError) ) << sError.cstr();
	sError = nullptr;
	EXPECT_FALSE ( ValidatePidFileOwner(iFD,iPid,sError) );
	SafeClose ( iFD );
}

TEST_F ( PidFileTest_c, removes_the_owned_path_through_quarantine )
{
	Write ( "42" );
	int iOwnedFD = open ( m_sPath.c_str(), O_RDONLY );
	ASSERT_GE ( iOwnedFD, 0 );
	CSphString sError;
	EXPECT_TRUE ( UnlinkFileIfSameDescriptor(iOwnedFD,m_sPath.c_str(),sError) ) << sError.cstr();
	EXPECT_EQ ( -1, access(m_sPath.c_str(),F_OK) );
	EXPECT_EQ ( ENOENT, errno );
	SafeClose ( iOwnedFD );
}

TEST_F ( PidFileTest_c, preserves_a_replacement_path_not_owned_by_the_descriptor )
{
	Write ( "42" );
	int iOwnedFD = open ( m_sPath.c_str(), O_RDONLY );
	ASSERT_GE ( iOwnedFD, 0 );
	std::string sOwnedPath = m_sPath + ".target";
	ASSERT_EQ ( 0, rename(m_sPath.c_str(),sOwnedPath.c_str()) );
	Write ( "314" );

	CSphString sError;
	EXPECT_FALSE ( UnlinkFileIfSameDescriptor(iOwnedFD,m_sPath.c_str(),sError) );
	int iReplacementFD = open ( m_sPath.c_str(), O_RDONLY );
	ASSERT_GE ( iReplacementFD, 0 );
	char sValue[4] {};
	EXPECT_EQ ( 3, read(iReplacementFD,sValue,3) );
	EXPECT_STREQ ( "314", sValue );
	SafeClose ( iReplacementFD );
	SafeClose ( iOwnedFD );
}
#endif

#if defined(__linux__)
TEST ( PidFile, pidfd_fallback_is_only_for_unsupported_kernels )
{
	EXPECT_TRUE ( IsPidfdUnsupportedError(ENOSYS) );
	EXPECT_TRUE ( IsPidfdUnsupportedError(EINVAL) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(EMFILE) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(ENFILE) );
	EXPECT_FALSE ( IsPidfdUnsupportedError(ENOMEM) );
}
#endif
