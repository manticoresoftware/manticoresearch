//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <string.h> // For strerror
#endif

#include "searchdssl.h"
#include "auth_common.h"
#include "daemon/daemon_ipc.h"

constexpr int g_iSignalTimeoutUs = 3000000; // default timeout on daemon auth rotation is 3 seconds

static bool IsConsoleInput()
{
	return isatty ( STDIN_FILENO ) && isatty ( STDOUT_FILENO );
}

static bool ReadLine ( CSphString & sOut )
{
	CSphVector<char> dBuf;
	char iCh = 0;

	auto fnReadChar = [&iCh]() {
#ifdef _WIN32
		return _read ( STDIN_FILENO, &iCh, 1 );
#else
		return ::read ( STDIN_FILENO, &iCh, 1 );
#endif
	};

	while ( true )
	{
		int iRead = fnReadChar();
		if ( iRead<=0 )
		{
			sOut = CSphString { dBuf };
			return !dBuf.IsEmpty();
		}

		if ( iCh == '\n' || iCh == '\r' )
			break;

		dBuf.Add ( iCh );
	}

	sOut = CSphString { dBuf };
	return true;
}

static bool ReadPassword ( bool bInteractive, CSphString & sOut, CSphString & sError )
{
	if ( !bInteractive )
	{
		if ( !ReadLine ( sOut ) )
		{
			sError = "unexpected end of input while reading password";
			return false;
		}
		return true;
	}

	CSphVector<char> dBuf;

#ifndef _WIN32
	// set up the terminal to disable character echoing.
	termios tOldT;
	if ( tcgetattr ( STDIN_FILENO, &tOldT )!=0 )
	{
		sError.SetSprintf ( "failed to read terminal settings: %s", strerror(errno) );
		return false;
	}
	termios tNewT = tOldT;
	tNewT.c_lflag &= ~ECHO; // disable echoing
	if ( tcsetattr ( STDIN_FILENO, TCSANOW, &tNewT )!=0 )
	{
		sError.SetSprintf ( "failed to update terminal settings: %s", strerror(errno) );
		return false;
	}
#endif

	auto fnReadChar = []() {
#ifdef _WIN32
		return _getch();
#else
		char iCh;
		return ::read ( STDIN_FILENO, &iCh, 1 )>0 ? iCh : -1;
#endif
	};

	char iCh;
	while ( true )
	{
		int iRead = fnReadChar();
		if ( iRead<0 )
		{
#ifndef _WIN32
			tcsetattr ( STDIN_FILENO, TCSANOW, &tOldT );
#endif
			sError = "unexpected end of input while reading password";
			return false;
		}

		iCh = (char)iRead;
		if ( iCh=='\n' || iCh == '\r' )
			break;

		// Handle backspace (127 for Linux/macOS)
		if ( iCh == '\b' || iCh == 127 )
		{ 
			if ( !dBuf.IsEmpty() )
			{
				dBuf.Pop();
				fprintf ( stdout, "\b \b" ); // erase the character and the asterisk
			}
		} else
		{
			dBuf.Add ( iCh );
			fprintf ( stdout, "*" );
		}
	}

#ifndef _WIN32
	// restore original terminal
	if ( tcsetattr ( STDIN_FILENO, TCSANOW, &tOldT )!=0 )
	{
		sError.SetSprintf ( "failed to restore terminal settings: %s", strerror(errno) );
		return false;
	}
#endif

	fprintf ( stdout, "\n" ); // print newline after password entry
	sOut = CSphString { dBuf };
	return true;
}

static bool ReadUserCredNonInteractive ( PasswordPolicy_e ePolicy, int iMinLen, CSphString & sLogin, CSphString & sPwd, CSphString & sError )
{
	if ( !ReadLine ( sLogin ) )
	{
		sError = "unexpected end of input while reading administrator login";
		return false;
	}
	sLogin.Trim();
	if ( sLogin.IsEmpty() )
	{
		sError = "login is empty";
		return false;
	}

	if ( !ReadPassword ( false, sPwd, sError ) )
		return false;
	sPwd.Trim();

	CSphString sValidationError;
	if ( !ValidatePassword ( sPwd, ePolicy, iMinLen, sValidationError ) )
	{
		sError = sValidationError;
		return false;
	}

	CSphString sPwdRe;
	if ( !ReadPassword ( false, sPwdRe, sError ) )
		return false;
	sPwdRe.Trim();

	if ( sPwd!=sPwdRe )
	{
		sError = "passwords do not match";
		return false;
	}

	return true;
}

static bool ReadUserCred ( PasswordPolicy_e ePolicy, int iMinLen, CSphString & sLogin, CSphString & sPwd, CSphString & sError )
{
	while ( true )
	{
		fprintf ( stdout, "Enter a new administrator login:\n" );
		if ( !ReadLine ( sLogin ) )
		{
			sError = "unexpected end of input while reading administrator login";
			return false;
		}
		sLogin.Trim();
		if ( sLogin.IsEmpty() )
			fprintf ( stdout, "error: login is empty, please try again\n" );
		else
			break;
	}

	while ( true )
	{
		// first pwd entry
		fprintf ( stdout, "Enter password:\n" );
		if ( !ReadPassword ( true, sPwd, sError ) )
			return false;
		sPwd.Trim();
		CSphString sValidationError;
		if ( !ValidatePassword ( sPwd, ePolicy, iMinLen, sValidationError ) )
		{
			fprintf ( stdout, "error: %s, please try again\n", sValidationError.cstr() );
			continue;
		}

		// pwd confirmation
		fprintf ( stdout, "Re-enter password to confirm:\n" );
		CSphString sPwdRe;
		if ( !ReadPassword ( true, sPwdRe, sError ) )
			return false;
		sPwdRe.Trim();

		if ( sPwd==sPwdRe )
		{
			fprintf ( stdout, "passwords match, proceeding with authentication setup...\n" );
			return true;
		} else
		{
			fprintf ( stdout, "error: passwords do not match, please try again\n" );
		}
	}

	return false;
}

static bool CheckAuthFile ( const CSphString & sFile, CSphString & sError )
{
	// fails if the auth.json file is not empty

	// if file does not exist or is not readable - valid state for bootstrap
	if ( !sphIsReadable ( sFile, nullptr ) )
		return true;

	// file exists - check its size
	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
	{
		sError.SetSprintf ( "could not open existing auth file '%s': %s", sFile.cstr(), sError.cstr() );
		return false;
	}

	int64_t iSize = tReader.GetFilesize();

	// ff file is empty \ size 0 - valid state
	if ( iSize==0 )
		return true;

	// if file exists and not empty - read and parse it
	// to make sure it empty \ contains only {}
	CSphFixedVector<char> dRawJson ( iSize + 2 );
	int64_t iRead = sphReadThrottled ( tReader.GetFD(), dRawJson.Begin(), iSize );
	if ( iRead !=iSize )
	{
		sError.SetSprintf ( "failed to read content from '%s'", sFile.cstr() );
		return false;
	}
	dRawJson[iSize] = '\0';
	dRawJson[iSize+1] = '\0';

	// parse JSON
	CSphVector<BYTE> tRawBson;
	if ( !sphJsonParse ( tRawBson, dRawJson.Begin(), false, false, false, sError ) )
	{
		sError.SetSprintf ( "auth file '%s' contains invalid JSON: %s", sFile.cstr(), sError.cstr() );
		return false;
	}

	bson::Bson_c tBsonSrc ( tRawBson );

	// empty JSON - valid state
	if ( tBsonSrc.IsEmpty() )
		return true;

	if ( !tBsonSrc.IsAssoc() )
	{
		sError.SetSprintf ( "auth file '%s' has wrong JSON structure; expected an object", sFile.cstr() );
		return false;
	}

	// users and permissions empty arrays - valid state
	bson::Bson_c tUsers ( tBsonSrc.ChildByName ( "users" ) );
	bson::Bson_c tPerms ( tBsonSrc.ChildByName ( "permissions" ) );
	if ( ( !tUsers || tUsers.IsEmpty() ) && ( !tPerms || tPerms.IsEmpty() ) )
		return true;

	// here - file contains user or permission data, error for the bootstrap
	sError.SetSprintf ( "'%s' is not empty, can not create administrator login for a non-empty authentication file", sFile.cstr() );
	return false;
}

int AuthBootstrap ( const CSphConfigSection & hSearchd, const CSphString & sConfigFilePath, bool bForceNonInteractive )
{
	PasswordPolicy_e ePolicy = ParsePasswordPolicy ( hSearchd );
	int iMinLen = GetPasswordMinLength ( hSearchd );

	CSphString sPidFile = hSearchd.GetStr ( "pid_file" );
	if ( sPidFile.IsEmpty() )
		sphFatal ( "could not read 'pid_file' option from the '%s' section 'searchd'", sConfigFilePath.cstr () );

	FixPathAbsolute ( sPidFile );
	FILE * fp = fopen ( sPidFile.cstr(), "r" );
	if ( !fp )
		sphFatal ( "pid file '%s' does not exist or is not readable, run daemon first", sPidFile.cstr() );

	char sBuf[16];
	int iLen = (int) fread ( sBuf, 1, sizeof(sBuf)-1, fp );
	sBuf[iLen] = '\0';
	fclose ( fp );

	int iPid = atoi(sBuf);
	if ( iPid<=0 )
		sphFatal ( "failed to read valid pid from '%s'", sPidFile.cstr() );

	CSphString sAuthFile = AuthGetPath ( hSearchd );
	if ( sAuthFile.IsEmpty() )
		sphFatal ( "authentication disabled, can not proceed" );

	CSphString sError;
	if ( !CheckAuthFile ( sAuthFile, sError ) )
		sphFatal ( "%s", sError.cstr() );

	CSphString sLogin, sPwd;
	bool bNonInteractive = bForceNonInteractive || !IsConsoleInput();
	bool bOk = bNonInteractive
		? ReadUserCredNonInteractive ( ePolicy, iMinLen, sLogin, sPwd, sError )
		: ReadUserCred ( ePolicy, iMinLen, sLogin, sPwd, sError );
	if ( !bOk )
		sphFatal ( "failed to read bootstrap credentials: %s", sError.cstr() );

	AuthUserCred_t tEntry;
	// username
	tEntry.m_sUser = sLogin;
	// salt
	tEntry.m_dSalt.Reset ( HASH20_SIZE );
	if ( !MakeRandBuf ( tEntry.m_dSalt, sError ) )
		sphFatal ( "%s, can not create salt", sError.cstr() );
	// mysql sha1
	tEntry.m_tPwdSha1 = CalcBinarySHA1 ( sPwd.cstr(), sPwd.Length() );
	// sha256
	auto tPwdSha256 = GetPwdHash256 ( tEntry, sPwd );
	tEntry.m_dPwdSha256.CopyFrom ( VecTraits_T<BYTE> ( tPwdSha256.data(), tPwdSha256.size() ) );

	AuthUsersMutablePtr_t tAuth { new AuthUsers_t };
	AddUser ( tEntry, tAuth );

	// add admin perms
	UserPerms_t & dPerms = tAuth->m_hUserPerms.AddUnique ( sLogin );
	for ( int i=0; i<(int)AuthAction_e::UNKNOWN; i++ )
	{
		UserPerm_t tPerm;
		tPerm.m_eAction = (AuthAction_e)i;
		tPerm.m_bAllow = true;
		tPerm.SetTarget ( "*" );
		dPerms.Add ( tPerm );
	}

	if ( !SaveAuthFile ( *tAuth.get(), sAuthFile, sError ) )
		sphFatal ( "%s, can save authentication file", sError.cstr() );

	ReloadAuthResult_e eRes = CommandReloadAuthAndWaitReply ( iPid, g_iSignalTimeoutUs, sError );
	if ( !sError.IsEmpty() )
		fprintf ( stderr, "%s\n", sError.cstr() );

	switch ( eRes )
	{
		case ReloadAuthResult_e::SUCCESS:
			fprintf ( stdout, "authentication settings successfully created and reloaded\n" );
			break;
		case ReloadAuthResult_e::CLIENT_ERROR:
			fprintf ( stderr, "error: could not send reload command to the daemon or timed out waiting for a reply\n" );
			break;
		case ReloadAuthResult_e::DAEMON_ERROR:
			fprintf ( stderr, "error: the daemon reported a failure while reloading authentication settings, check the daemon log for details\n" );
			break;
		case ReloadAuthResult_e::IPC_ERROR:
			fprintf ( stderr, "error: a communication error occurred with the daemon process\n" );
			break;
	}

	return (int)eRes;
}
