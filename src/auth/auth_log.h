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

#pragma once

#include "sphinxstd.h"

struct AuthActionContext_t
{
	const CSphString & m_sActorUser;
	const char * m_sSource; // client details
};

class CSphConfigSection;

enum class AccessMethod_e
{
	API,
	SQL,
	HTTP_BASIC,
	HTTP_BEARER,
};

enum class AuthLogLevel_e
{
	ALL,
	INFO,
	WARNING,
	ERR,
	CRITICAL,

	DISABLED
};

class AuthLogger_i
{
public:
	virtual ~AuthLogger_i() = default;

	virtual void Init ( const CSphString & sDaemonLogPath, const CSphConfigSection & hSearchd, bool bLogStdout, bool bLogSyslog ) = 0;
	virtual void Close() = 0;

	virtual void Log ( AuthLogLevel_e eLevel, const char * sMessage ) = 0;

	/// user and permission

	/// creation of a new user
	virtual void UserAdded ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) = 0;

	/// deletion of a user
	virtual void UserDeleted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) = 0;

	/// password change for a user
	// TODO: add after functionality implemented
	virtual void UserPasswordChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) = 0;
	
	/// new bearer token
	virtual void UserTokenChanged ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser ) = 0;

	/// permission being granted
	virtual void PermissionGranted ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget ) = 0;

	/// permission being revoked
	virtual void PermissionRevoked ( const AuthActionContext_t & tCtx, const CSphString & sTargetUser, const CSphString & sAction, const CSphString & sActionTarget ) = 0;

	/// runtime events
	
	/// successful authentication attempt
	virtual void AuthSuccess ( const CSphString & sUser, AccessMethod_e eMethod, const char * sRemoteAddr ) = 0;

	/// failed authentication attempt
	virtual void AuthFailure ( const CSphString & sAttemptedUser, AccessMethod_e eMethod, const char * sRemoteAddr, const char * sReason ) = 0;

	/// authenticated user is denied an action
	virtual void AuthDenied ( const CSphString & sUser, const CSphString & sRemoteAddr, AuthAction_e eAction, const CSphString & sTarget ) = 0;

	/// cluster events

	/// local auth data is overridden by the cluster
	virtual void ClusterJoinOverride ( const CSphString & sClusterName ) = 0;

	/// backup of the old local data before it got overridden
	virtual void ClusterBackupData ( const CSphString & sJsonData, const CSphString & sClusterName, const CSphString & sNodeName ) = 0;
	
	/// successful reload of the auth configuration
	virtual void Reload ( const AuthActionContext_t & tCtx, const CSphString & sTrigger ) = 0;
	
	/// FIXME!!! add failure to load or validate auth data

	virtual void ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sTarget, const CSphString & sReason ) = 0;
	virtual void ActionFailed ( const AuthActionContext_t & tCtx, const CSphString & sAction, const CSphString & sReason ) = 0;
};

AuthLogger_i & AuthLog();

AuthActionContext_t AuthLogGetSessionCtx();

