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

#include "auth_perms.h"
#include "auth.h"
#include "daemon/logger.h"

#if !_WIN32
#include <sys/stat.h>
#endif

#if USE_SYSLOG
#include <syslog.h>
#endif

#include "auth_log.h"

constexpr int LOG_BUF_SIZE = 1024;

static const char * GetMethodName ( AccessMethod_e eMethod )
{
	switch ( eMethod )
	{
		case AccessMethod_e::API: return "API";
		case AccessMethod_e::SQL: return "SphixnQL";
		case AccessMethod_e::HTTP_BASIC: return "HTTP Basic";
		case AccessMethod_e::HTTP_BEARER: return "HTTP Bearer";
		default: return "unknown";
	}
}

class AuthLogger_c : public AuthLogger_i
{
public:
	~AuthLogger_c() override = default;

	void Init ( const CSphString & sDaemonLogPath, const CSphConfigSection & hSearchd, bool bLogStdout, bool bLogSyslog ) override;
	void Close() override;

	void Log ( AuthLogLevel_e eLevel, const char * sMessage ) override;

	void UserAdded ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) override;
	void UserDeleted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) override;
	void UserPasswordChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) override;
	void UserTokenChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) override;
	void PermissionGranted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget ) override;
	void PermissionRevoked ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget ) override;

	void AuthSuccess ( const CSphString & sUser, AccessMethod_e eMethod, const char * sRemoteAddr ) override;
	void AuthFailure ( const CSphString & sAttemptedUser, AccessMethod_e eMethod, const char * sRemoteAddr, const char * sReason ) override;
	void AuthDenied ( const CSphString & sUser, const CSphString & sRemoteAddr, AuthAction_e eAction, const CSphString & sTarget ) override;

	void ClusterJoinOverride ( const CSphString & sClusterName ) override;
	void ClusterBackupData ( const CSphString & sJsonData, const CSphString & sClusterName, const CSphString & sNodeName ) override;

	void Reload ( const AuthActionContext_t & tCtx, const CSphString & sTrigger ) override;

	void ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sTarget, const CSphString & sReason ) override;
	void ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sReason ) override;

private:
	AuthLogLevel_e m_eMinLevel = AuthLogLevel_e::DISABLED;
	bool m_bLogStdout = false;
	bool m_bLogSyslog = false;
	int m_iLogFile = -1;

	void WriteEntry ( AuthLogLevel_e eLevel, char * sBuf, int iLen );
	void LogVA ( AuthLogLevel_e eLevel, const char * sFmt, ... );
};

static AuthLogger_c g_tAuthLog;

AuthLogger_i & AuthLog()
{
	return g_tAuthLog;
}


void AuthLogger_c::Init ( const CSphString & sDaemonLogPath, const CSphConfigSection & hSearchd, bool bLogStdout, bool bLogSyslog )
{
	auto sLogLevel = hSearchd.GetStr ( "auth_log_level", "info" );

	m_bLogStdout = bLogStdout;
	m_bLogSyslog = bLogSyslog;
	if ( sLogLevel=="disabled" )
	{
		m_eMinLevel = AuthLogLevel_e::DISABLED;
		return;
	} else if ( sLogLevel=="info" )
	{
		m_eMinLevel = AuthLogLevel_e::INFO;
	} else if ( sLogLevel=="warning" )
	{
		m_eMinLevel = AuthLogLevel_e::WARNING;
	} else if ( sLogLevel=="error" )
	{
		m_eMinLevel = AuthLogLevel_e::ERR;
	} else if ( sLogLevel=="all" )
	{
		m_eMinLevel = AuthLogLevel_e::ALL;
	} else
	{
		sphFatal ( "unknown auth_log_level '%s'", sLogLevel.cstr() );
	}

	if ( m_bLogSyslog )
		return;

	CSphString sAuthLogPath;
	sAuthLogPath.SetSprintf ( "%s.auth", sDaemonLogPath.cstr() );
	
	int iFD = ::open ( sAuthLogPath.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
	if ( iFD < 0 )
		sphFatal ( "failed to open log file '%s': %s", sAuthLogPath.cstr(), strerrorm(errno) );

	m_iLogFile = iFD;
	ChangeLogFileMode ( m_iLogFile );
}

void AuthLogger_c::Close()
{
	if ( m_iLogFile>=0 )
		::close ( m_iLogFile );
	m_iLogFile = -1;
}

void AuthLogger_c::Log ( AuthLogLevel_e eLevel, const char * sMsg )
{
	LogVA ( eLevel, "%s", sMsg );
}

void AuthLogger_c::UserAdded ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser )
{
	LogVA ( AuthLogLevel_e::INFO, "user '%s' created by '%s' via %s", sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::UserDeleted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser )
{
	LogVA ( AuthLogLevel_e::INFO, "user '%s' deleted by '%s' via %s", sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::UserPasswordChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser )
{
	LogVA ( AuthLogLevel_e::INFO, "password for user '%s' changed by '%s' via %s", sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::UserTokenChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser )
{
	LogVA ( AuthLogLevel_e::INFO, "bearer token for user '%s' regenerated by '%s' via %s", sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::PermissionGranted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget )
{
	LogVA ( AuthLogLevel_e::INFO, "granted %s on '%s' to user '%s' by '%s' via %s", sAction.cstr(), sActionTarget.cstr(), sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::PermissionRevoked ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget )
{
	if ( !sAction.IsEmpty() && !sActionTarget.IsEmpty() )
		LogVA ( AuthLogLevel_e::INFO, "revoked %s on '%s' to user '%s' by '%s' via %s", sAction.cstr(), sActionTarget.cstr(), sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
	else if ( !sAction.IsEmpty() )
		LogVA ( AuthLogLevel_e::INFO, "revoked %s to user '%s' by '%s' via %s", sAction.cstr(), sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
	else if ( !sActionTarget.IsEmpty() )
		LogVA ( AuthLogLevel_e::INFO, "revoked on '%s' to user '%s' by '%s' via %s", sActionTarget.cstr(), sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
	else
		LogVA ( AuthLogLevel_e::INFO, "revoked a permission to user '%s' by '%s' via %s", sTargetUser.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
}

void AuthLogger_c::AuthSuccess ( const CSphString & sUser, AccessMethod_e eMethod, const char * sRemoteAddr )
{
	LogVA ( AuthLogLevel_e::ALL, "user '%s' successfully authenticated via %s from %s", sUser.cstr(), GetMethodName ( eMethod ), sRemoteAddr );
}

void AuthLogger_c::AuthFailure ( const CSphString & sAttemptedUser, AccessMethod_e eMethod, const char * sRemoteAddr, const char * sReason )
{
	if ( eMethod!=AccessMethod_e::HTTP_BEARER )
		LogVA ( AuthLogLevel_e::WARNING, "failed authentication attempt for user '%s' via %s from %s: %s", sAttemptedUser.cstr(), GetMethodName ( eMethod ), sRemoteAddr, sReason );
	else
		LogVA ( AuthLogLevel_e::WARNING, "failed authentication attempt for bearer '%s' via %s from %s: %s", sAttemptedUser.cstr(), GetMethodName ( eMethod ), sRemoteAddr, sReason );
}

void AuthLogger_c::AuthDenied ( const CSphString & sAttemptedUser, const CSphString & sRemoteAddr, AuthAction_e eAction, const CSphString & sTarget )
{
	LogVA ( AuthLogLevel_e::ERR, "user '%s' from %s denied %s on '%s'", sAttemptedUser.cstr(), sRemoteAddr.cstr(), GetActionName ( eAction ), sTarget.scstr() );
}

void AuthLogger_c::ClusterJoinOverride ( const CSphString & sClusterName )
{
	LogVA ( AuthLogLevel_e::CRITICAL, "node joining cluster '%s', local auth data is being overridden", sClusterName.cstr() );
}

void AuthLogger_c::ClusterBackupData ( const CSphString & sJsonData, const CSphString & sClusterName, const CSphString & sNodeName )
{
	LogVA ( AuthLogLevel_e::INFO, "joined cluster '%s' via node '%s' local auth data backup: %s", sClusterName.cstr(), sNodeName.cstr(), sJsonData.cstr() );
}

void AuthLogger_c::Reload ( const AuthActionContext_t & tCtx, const CSphString & sTrigger )
{
	if ( sTrigger.IsEmpty() )
		LogVA ( AuthLogLevel_e::INFO, "authentication data reloaded successfully by '%s' via %s", tCtx.m_sActorUser.cstr(), tCtx.m_sSource );
	else
		LogVA ( AuthLogLevel_e::INFO, "authentication data reloaded successfully by '%s' via %s, trigger: %s", tCtx.m_sActorUser.cstr(), tCtx.m_sSource, sTrigger.cstr() );
}

void AuthLogger_c::ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sTarget, const CSphString & sReason )
{
	if ( sTarget.IsEmpty() )
		LogVA ( AuthLogLevel_e::WARNING, "failed to %s by '%s' via %s. Error: %s", sAction.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource, sReason.cstr() );
	else
		LogVA ( AuthLogLevel_e::WARNING, "failed to %s on %s by '%s' via %s. Error: %s", sAction.cstr(), sTarget.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource, sReason.cstr() );
}

void AuthLogger_c::ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sReason )
{
	LogVA ( AuthLogLevel_e::WARNING, "failed to %s by '%s' via %s. Error: %s", sAction.cstr(), tCtx.m_sActorUser.cstr(), tCtx.m_sSource, sReason.cstr() );
}

void AuthLogger_c::WriteEntry ( AuthLogLevel_e eLevel, char * sBuf, int iLen )
{
#if USE_SYSLOG
	// syslog if enabled
	if ( m_bLogSyslog )
	{
		const int levels[] = { LOG_INFO, LOG_WARNING, LOG_ERR, LOG_CRIT };
		syslog ( levels[(int)eLevel], "%s", sBuf );
		return;
	}
#endif

	(void) eLevel;
	sBuf[iLen] = '\n';
	iLen++;

	// write to the file
	if ( m_iLogFile >= 0 )
	{
		sphSeek ( m_iLogFile, 0, SEEK_END );
		sphWrite ( m_iLogFile, sBuf, iLen );
	}

	// console if enabled
	if ( m_bLogStdout )
		sphWrite ( STDOUT_FILENO, sBuf, iLen );
}

void  AuthLogger_c::LogVA ( AuthLogLevel_e eLevel, const char * sFmt, ... )
{
	if ( !sFmt || eLevel<m_eMinLevel )
		return;

	char sTimeBuf[128];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );
	
	const char * sLevelBanner = "";
	switch ( eLevel )
	{
		case AuthLogLevel_e::ALL:		sLevelBanner = "ALL"; break;
		case AuthLogLevel_e::INFO:		sLevelBanner = "INFO"; break;
		case AuthLogLevel_e::WARNING:	sLevelBanner = "WARN"; break;
		case AuthLogLevel_e::ERR:	sLevelBanner = "ERROR"; break;
		case AuthLogLevel_e::CRITICAL:	sLevelBanner = "CRITICAL"; break;
		case AuthLogLevel_e::DISABLED: return;
	}

	char sBuf[LOG_BUF_SIZE];
	int iHeaderLen = snprintf ( sBuf, sizeof(sBuf)-1, "[%s][%d][%s] ", sTimeBuf, GetOsThreadId(), sLevelBanner );

	// need more space for tail zero and "\n" that added at sphLogEntry
	constexpr int iSafeGap = 4;
	int iGotMsgLen = sizeof(sBuf) - iHeaderLen - iSafeGap;
	
	va_list ap;
	va_start ( ap, sFmt );
	int iMsgLen = vsnprintf ( sBuf + iHeaderLen, iGotMsgLen, sFmt, ap );
	va_end ( ap );
	
	int iSafeLen = 0;
	if ( iMsgLen>=iGotMsgLen )
		iSafeLen = iGotMsgLen - 1;
	else
		iSafeLen = iMsgLen;
	
	iSafeLen += iHeaderLen;

	WriteEntry ( eLevel, sBuf, iSafeLen );
}

static const CSphString g_sNoSessionUser = "no session user";
static const char * g_sNoSessionClient = "no session client";

AuthActionContext_t AuthLogGetSessionCtx()
{
	if ( session::GetClientSession() )
	{
		const char * sClient = session::szClientName();
		if ( !sClient )
			sClient = g_sNoSessionClient;

		return AuthActionContext_t { session::GetUser(), sClient };
	} else
	{
		return AuthActionContext_t { g_sNoSessionUser, g_sNoSessionClient };
	}
}
