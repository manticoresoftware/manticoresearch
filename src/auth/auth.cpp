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

#include "dynamic_idx.h"
#include "docs_collector.h"
#include "searchdreplication.h"
#include "searchdssl.h"

#include "auth_common.h"
#include "auth_log.h"
#include "daemon/logger.h"
#include "auth.h"

static CSphMutex g_tAuthLock;
static AuthUsersPtr_t g_tAuth;

static CSphString g_sAuthFile;

/////////////////////////////////////////////////////////////////////////////
/// auth load

static void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tDst );
static bool MakeBuddyToken ( AuthUsers_t & tAuth, CSphString & sError );

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	g_sAuthFile = AuthGetPath ( hSearchd );
	if ( g_sAuthFile.IsEmpty() )
		return;

#if !WITH_SSL
	sphFatal ( "requiring authentication but the daemon was built without SSL support. Please either disable authentication in the configuration or recompile with WITH_SSL." );
#endif

	CSphString sError;
	// need to create empty file for bootstrap to work
	if ( !sphIsReadable ( g_sAuthFile, nullptr ) )
	{
		if ( !CreateAuthFile ( g_sAuthFile, sError ) )
			sphFatal ( "Failed to create authentication file '%s': %s", g_sAuthFile.cstr(), sError.cstr() );
	}

	AuthUsersMutablePtr_t tAuth = ReadAuthFile ( g_sAuthFile, sError );
	if ( !tAuth )
		sphFatal ( "%s", sError.cstr() );

	if ( !MakeBuddyToken ( *tAuth.get(), sError ) )
		sphFatal ( "%s", sError.cstr() );

	tAuth->m_bEnabled = true;
	
	{
		ScopedMutex_t tLock ( g_tAuthLock );
		g_tAuth = std::move ( tAuth );
	}
	GetBearerCache().Invalidate();
}

void AuthLogInit ( const CSphConfigSection & hSearchd )
{
	if ( !IsAuthEnabled() )
		return;

	auto sDaemonLog = sphGetLogFile();
	bool bLogStdout = LogIsStdout();
	bool bLogSyslog = LogIsSyslog();
	AuthLog().Init ( sDaemonLog, hSearchd, bLogStdout, bLogSyslog );
}

void AuthDone()
{
	AuthLog().Close();
}

bool IsAuthEnabled()
{
	if ( !g_tAuth )
		return false;

	return g_tAuth->m_bEnabled;
}

const AuthUsersPtr_t GetAuth()
{
	ScopedMutex_t tLock ( g_tAuthLock );
	return g_tAuth;
}

AuthUsersMutablePtr_t CopyAuth()
{
	AuthUsersPtr_t tAuth = GetAuth();
	if ( tAuth )
		return std::make_unique<AuthUsers_t>( *tAuth );
	else
		return std::make_unique<AuthUsers_t>();
}

bool AuthReload ( CSphString & sError )
{
	if ( !IsAuthEnabled() )
	{
		sError = "authorization disabled, can not reload";
		return false;
	}

	AuthUsersMutablePtr_t tAuth = ReadAuthFile ( g_sAuthFile, sError );
	if ( !tAuth )
	{
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), ( "authentication data reload" ), sError );
		return false;
	}

	{
		ScopedMutex_t tLock ( g_tAuthLock );
		const AuthUserCred_t * pSrcBuddy = g_tAuth->m_hUserToken ( GetAuthBuddyName() );
		AddBuddyToken ( pSrcBuddy, *tAuth );
		tAuth->m_bEnabled = true;
		g_tAuth = std::move ( tAuth );
	}
	GetBearerCache().Invalidate();

	AuthLog().Reload ( AuthLogGetSessionCtx(), CSphString() );

	return true;
}

bool MakeBuddyToken ( AuthUsers_t & tAuth, CSphString & sError )
{
	AuthUserCred_t & tEntry = tAuth.m_hUserToken.AddUnique ( GetAuthBuddyName() );

	// username
	tEntry.m_sUser = GetAuthBuddyName();

	// salt
	tEntry.m_dSalt.Reset ( HASH20_SIZE );
	if ( !MakeRandBuf ( tEntry.m_dSalt, sError ) )
		return false;

	// mysql sha1
	VecTraits_T<BYTE> dSha1 ( tEntry.m_tPwdSha1.data(), tEntry.m_tPwdSha1.size() );
	if ( !MakeRandBuf ( dSha1, sError ) )
		return false;

	// sha256
	tEntry.m_dPwdSha256.Reset ( HASH256_SIZE );
	if ( !MakeRandBuf ( tEntry.m_dPwdSha256, sError ) )
		return false;

	// add admin perms
	UserPerms_t & dPerms = tAuth.m_hUserPerms.AddUnique ( GetAuthBuddyName() );
	for ( int i=0; i<(int)AuthAction_e::UNKNOWN; i++ )
	{
		UserPerm_t tPerm;
		tPerm.m_eAction = (AuthAction_e)i;
		tPerm.m_bAllow = true;
		tPerm.SetTarget ( "*" );
		dPerms.Add ( tPerm );
	}

	return true;
}

void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tAuth )
{
	if ( !pSrcBuddy )
		return;

	tAuth.m_hUserToken.Add ( *pSrcBuddy, pSrcBuddy->m_sUser );
	if ( !tAuth.m_hUserPerms.Exists ( pSrcBuddy->m_sUser ) )
	{
		UserPerms_t & tPerms = tAuth.m_hUserPerms.AddUnique ( pSrcBuddy->m_sUser );
		for ( int iAction=0; iAction<(int)AuthAction_e::UNKNOWN; iAction++ )
		{
			UserPerm_t & tPerm = tPerms.Add();
			tPerm.m_eAction = (AuthAction_e)iAction;
			tPerm.m_bAllow = true;
			tPerm.SetTarget ( "*" );
		};
	}
}

class DynamicAuthIndex_c : public GenericTableIndex_c
{
public:
	DynamicAuthIndex_c ( bool bForceFinalize )
	{
		 m_bForceFinalize = bForceFinalize;
	}
	virtual ~DynamicAuthIndex_c() = default;
	const CSphSchema & GetMatchSchema () const final { return m_tSchema; }

private:
	void SetSorterStuff ( CSphMatch * pMatch ) const final
	{
		m_pMatch = pMatch;
		m_pUsers = GetAuth();
		Start();
	}
	Str_t GetErrors () const final { return FromStr ( m_sError ); }

protected:
	CSphString m_sError;
	mutable CSphMatch * m_pMatch = nullptr;

	virtual void Start() const = 0;

	mutable AuthUsersPtr_t m_pUsers;
};

class AuthUsersIndex_c final : public DynamicAuthIndex_c
{
public:
	AuthUsersIndex_c ( bool bForceFinalize );

private:
	bool FillNextMatch () const final;
	void Start() const override;

	mutable SmallStringHash_T<AuthUserCred_t>::Iterator_c m_tItUser;

public:
	enum class SchemaColumn_e
	{
		Id = 0,
		Username = 1,
		Salt = 2,
		Hashes = 3,
		Hash_SHA1 = 4,
		Hash_SHA256 = 5,
		Hash_Bearer = 6,
		
		Total 
	};

	static const char * GetColumnName ( SchemaColumn_e eName );
};

class AuthPermsIndex_c final : public DynamicAuthIndex_c
{
public:
	AuthPermsIndex_c ( bool bForceFinalize );

private:
	bool FillNextMatch () const final;
	void Start() const override;

	mutable SmallStringHash_T<UserPerms_t>::Iterator_c m_tItPerm;
	mutable int m_iItemPerm = 0;

public:
	enum class SchemaColumn_e
	{
		Id = 0,
		Username = 1,
		Action = 2,
		Target = 3,
		Allow = 4,
		Budget = 5,
	};

	static const char * GetColumnName ( SchemaColumn_e eName );
};

AuthUsersIndex_c::AuthUsersIndex_c ( bool bForceFinalize )
	: DynamicAuthIndex_c ( bForceFinalize )
{
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Id ), SPH_ATTR_BIGINT ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Username ), SPH_ATTR_STRINGPTR ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Salt ), SPH_ATTR_STRINGPTR ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Hashes ), SPH_ATTR_JSON_PTR ), true );
}

const char * AuthUsersIndex_c::GetColumnName ( SchemaColumn_e eName )
{
	switch ( eName )
	{
	case SchemaColumn_e::Id: return sphGetDocidName();
	case SchemaColumn_e::Username: return "username";
	case SchemaColumn_e::Salt: return "salt";
	case SchemaColumn_e::Hashes: return "hashes";
	case SchemaColumn_e::Hash_SHA1: return "password_sha1_no_salt";
	case SchemaColumn_e::Hash_SHA256: return "password_sha256";
	case SchemaColumn_e::Hash_Bearer: return "bearer_sha256";

	default:
		break;
	}

	return nullptr;
}

void AuthUsersIndex_c::Start() const
{
	m_tItUser = m_pUsers->m_hUserToken.begin();
}

static void PutString ( const CSphString & sVal, const CSphAttrLocator & tLoc, CSphMatch * m_pMatch )
{
	int iLen = sVal.Length();

	BYTE * pData = nullptr;
	m_pMatch->SetAttr ( tLoc, (SphAttr_t)sphPackPtrAttr ( iLen, &pData ) );
	if ( iLen )
		memcpy ( pData, sVal.cstr(), iLen );
}

static void PutString ( const char * sVal, const CSphAttrLocator & tLoc, CSphMatch * m_pMatch )
{
	int iLen = ( sVal ? strlen ( sVal ) : 0 );

	BYTE * pData = nullptr;
	m_pMatch->SetAttr ( tLoc, (SphAttr_t)sphPackPtrAttr ( iLen, &pData ) );
	if ( iLen )
		memcpy ( pData, sVal, iLen );
}

static void PutBjson ( const VecTraits_T<BYTE> & dBson, const CSphAttrLocator & tLoc, CSphMatch * m_pMatch )
{
	BYTE * pData = nullptr;
	SphAttr_t uValue = (SphAttr_t)sphPackPtrAttr ( dBson.GetLength(), &pData );
	if ( dBson.GetLength() )
		memcpy ( pData, dBson.Begin(), dBson.GetLength() );
	m_pMatch->SetAttr ( tLoc, uValue );
}

static SphAttr_t Hash ( const AuthUserCred_t & tUser )
{
	SphAttr_t tID = sphFNV64 ( tUser.m_sUser.cstr(), tUser.m_sUser.Length(), SPH_FNV64_SEED );
	tID = abs ( tID );

	return tID;
}

static SphAttr_t Hash ( const CSphString & sUser, const UserPerm_t & tPerm )
{
	SphAttr_t tID = SPH_FNV64_SEED;
	tID = sphFNV64 ( sUser.cstr(), sUser.Length(), tID );
	tID = sphFNV64 ( &tPerm.m_eAction, 1, tID );
	tID = sphFNV64 ( tPerm.m_sTarget.cstr(), tPerm.m_sTarget.Length(), tID );
	tID = sphFNV64 ( &tPerm.m_bAllow, 1, tID );
	tID = sphFNV64 ( tPerm.m_sBudget.cstr(), tPerm.m_sBudget.Length(), tID );
	tID = abs ( tID );

	return tID;
}

bool AuthUsersIndex_c::FillNextMatch () const
{
	if ( m_tItUser!=m_pUsers->m_hUserToken.end() )
	{
		const AuthUserCred_t & tUser = m_tItUser->second;

		// docid
		m_pMatch->SetAttr ( m_tSchema.GetAttr ( (int)SchemaColumn_e::Id ).m_tLocator, Hash ( tUser ) );
		// username
		PutString ( tUser.m_sUser, m_tSchema.GetAttr ( (int)SchemaColumn_e::Username ).m_tLocator, m_pMatch );
		// salt
		PutString ( BinToHex ( tUser.m_dSalt ), m_tSchema.GetAttr ( (int)SchemaColumn_e::Salt ).m_tLocator, m_pMatch );
		// hashes
		CSphVector<BYTE> dHashes;
		{
			bson::Root_c tHashes ( dHashes );

			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_SHA1 ), BinToHex ( tUser.m_tPwdSha1 ).cstr() );
			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_SHA256 ), BinToHex ( tUser.m_dPwdSha256 ).cstr() );
			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_Bearer ), BinToHex ( tUser.m_dBearerSha256 ).cstr() );
		}
		PutBjson ( dHashes, m_tSchema.GetAttr ( (int)SchemaColumn_e::Hashes ).m_tLocator, m_pMatch );

		++m_tItUser;
		return true;
	}
	return false;
}

const char * AuthPermsIndex_c::GetColumnName ( SchemaColumn_e eName )
{
	switch ( eName )
	{
	case SchemaColumn_e::Id: return sphGetDocidName();
	case SchemaColumn_e::Username: return "username";
	case SchemaColumn_e::Action: return "action";
	case SchemaColumn_e::Target: return "target";
	case SchemaColumn_e::Allow: return "allow";
	case SchemaColumn_e::Budget: return "budget";

	default:
		break;
	}

	return nullptr;
}

AuthPermsIndex_c::AuthPermsIndex_c ( bool bForceFinalize )
	: DynamicAuthIndex_c ( bForceFinalize )
{
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Id ), SPH_ATTR_BIGINT ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Username ), SPH_ATTR_STRINGPTR ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Action ), SPH_ATTR_STRINGPTR ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Target ), SPH_ATTR_STRINGPTR ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Allow ), SPH_ATTR_BOOL ), true );
	m_tSchema.AddAttr ( CSphColumnInfo ( GetColumnName ( SchemaColumn_e::Budget ), SPH_ATTR_JSON_PTR ), true );
}

void AuthPermsIndex_c::Start() const
{
	m_tItPerm = m_pUsers->m_hUserPerms.begin();
	m_iItemPerm = 0;
}

bool AuthPermsIndex_c::FillNextMatch () const
{
	while ( m_tItPerm!=m_pUsers->m_hUserPerms.end() )
	{
		// skip empty perms vector or switch to next perms vector
		if ( m_iItemPerm>=m_tItPerm->second.GetLength() )
		{
			m_iItemPerm = 0;
			++m_tItPerm;
			continue;
		}

		const CSphString & sUser = m_tItPerm->first;
		const UserPerm_t & tPerm = m_tItPerm->second[m_iItemPerm];

		// docid
		m_pMatch->SetAttr ( m_tSchema.GetAttr ( (int)SchemaColumn_e::Id ).m_tLocator, Hash ( sUser, tPerm ) );
		// username
		PutString ( sUser, m_tSchema.GetAttr ( (int)SchemaColumn_e::Username ).m_tLocator, m_pMatch );
		// action
		PutString ( GetActionName ( tPerm.m_eAction ), m_tSchema.GetAttr ( (int)SchemaColumn_e::Action).m_tLocator, m_pMatch );
		// target
		PutString ( tPerm.m_sTarget, m_tSchema.GetAttr ( (int)SchemaColumn_e::Target ).m_tLocator, m_pMatch );
		// allow
		m_pMatch->SetAttr ( m_tSchema.GetAttr ( (int)SchemaColumn_e::Allow ).m_tLocator, (SphAttr_t)tPerm.m_bAllow );
		// budget
		CSphVector<BYTE> dBudget;
		if ( !tPerm.m_sBudget.IsEmpty() )
		{
			bson::BsonContainer_c tBudget ( tPerm.m_sBudget.cstr() );
			tBudget.BsonToBson ( dBudget );
		}
		PutBjson ( dBudget, m_tSchema.GetAttr ( (int)SchemaColumn_e::Budget ).m_tLocator, m_pMatch );

		m_iItemPerm++;

		return true;
	}

	return false;
}

static ServedIndexRefPtr_c MakeDynamicAuthIndex ( const CSphString & sName, bool bForceFinalize )
{
	if ( sName==GetIndexNameAuthUsers() )
		return MakeServed ( new AuthUsersIndex_c ( bForceFinalize ) );
	else if ( sName==GetIndexNameAuthPerms() )
		return MakeServed ( new AuthPermsIndex_c ( bForceFinalize ) );
	else
		return nullptr;
}

ServedIndexRefPtr_c MakeDynamicAuthIndex ( const CSphString & sName, StringBuilder_c & sError )
{
	if ( !IsAuthEnabled() )
	{
		sError << "authorization is disabled, cannot access table " << sName;
		return {};
	}

	ServedIndexRefPtr_c pIndex = MakeDynamicAuthIndex ( sName, false );
	if ( !pIndex )
		sError << "no such table " << sName;
	return pIndex;
}

static bool SaveAuth ( AuthUsersMutablePtr_t && tAuth, CSphString & sError )
{
	{
		ScopedMutex_t tLock ( g_tAuthLock );
		bool bSaved = SaveAuthFile ( *tAuth.get(), g_sAuthFile, sError );
		sphLogDebug ( "save auth '%s' %s, error: %s", g_sAuthFile.cstr(), ( bSaved ? "succeed" : "failed" ), sError.scstr() );
	 
		if ( !bSaved )
			return false;

		tAuth->m_bEnabled = true;
		g_tAuth = std::move ( tAuth );
	}
	GetBearerCache().Invalidate();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DELETE

static void DeleteUser ( AuthUsersMutablePtr_t & tAuth, const CSphString & sUserName, bool bWithPerms )
{
	if ( bWithPerms )
		tAuth->m_hUserPerms.Delete ( sUserName );

	tAuth->m_hUserToken.Delete ( sUserName );
}

static int DeleteUsers ( const VecTraits_T<DocID_t> & dSrcDocs, CSphString & sError )
{
	CSphFixedVector < std::pair <DocID_t, CSphString> > dDocsDelete ( dSrcDocs.GetLength() );
	ARRAY_FOREACH ( i, dSrcDocs )
		dDocsDelete[i].first = dSrcDocs[i];
	dDocsDelete.Sort();
	int iDocsCount = sphUniq ( dDocsDelete.Begin(), dDocsDelete.GetLength() );

	AuthUsersMutablePtr_t tAuth = CopyAuth();
	for ( const auto & tItem : tAuth->m_hUserToken )
	{
		SphAttr_t tUserHash = Hash ( tItem.second );
		auto * pDoc = dDocsDelete.BinarySearch ( ([](const auto & tItem ) { return tItem.first; }), tUserHash );
		if ( pDoc )
			pDoc->second = tItem.first;
	}

	int iDeleted = 0;
	for ( int i=0; i<iDocsCount; i++  )
	{
		const CSphString & sUserName = dDocsDelete[i].second;
		if ( sUserName.IsEmpty() )
			continue;

		DeleteUser ( tAuth, sUserName, true );
		iDeleted++;
	}

	if ( iDeleted )
	{
		if ( !SaveAuth ( std::move ( tAuth ), sError ) )
			return 0;
	}

	return iDeleted;
}

static int DeletePerms ( const VecTraits_T<DocID_t> & dDocs, CSphString & sError )
{
	int iDeleted = 0;

	AuthUsersMutablePtr_t tAuth = CopyAuth();
	for ( auto & tPerms : tAuth->m_hUserPerms )
	{
		ARRAY_FOREACH ( iPerm, tPerms.second )
		{
			SphAttr_t tPermHash = Hash ( tPerms.first, tPerms.second[iPerm] );
			if ( !dDocs.Contains ( tPermHash ) )
				continue;

			tPerms.second.Remove ( iPerm-- );
			iDeleted++;
		}
	}

	if ( iDeleted )
	{
		if ( !SaveAuth ( std::move ( tAuth ), sError ) )
			return 0;
	}

	return iDeleted;
}

bool DeleteAuthDocuments ( const RtAccum_t & tAccum, int * pDeletedCount, CSphString & sError )
{
	assert ( tAccum.m_dCmd.GetLength() );

	int iDeletedCount = 0;
	if ( tAccum.m_dCmd[0]->m_sIndex==GetIndexNameAuthUsers() )
		iDeletedCount = DeleteUsers ( tAccum.m_dAccumKlist, sError );
	else
		iDeletedCount = DeletePerms ( tAccum.m_dAccumKlist, sError );

	if ( pDeletedCount )
		*pDeletedCount = iDeletedCount;

	return ( sError.IsEmpty() );
}

static CSphString GetFilterString ( const SqlStmt_t & tStmt, const char * sName )
{
	assert ( sName );

	int iVal = tStmt.m_tQuery.m_dFilters.GetFirst ( [&sName] ( const auto & tFilter ) { return ( tFilter.m_sAttrName==sName && tFilter.m_eType==SPH_FILTER_STRING && tFilter.m_dStrings.GetLength()==1 ); } );
	if ( iVal!=-1 )
		return tStmt.m_tQuery.m_dFilters[iVal].m_dStrings[0];
	else
		return {};
}

int DeleteAuthDocuments ( const CSphString & sName, const SqlStmt_t & tStmt, CSphString & sError )
{
	bool bUser = ( sName==GetIndexNameAuthUsers() );
	const char * sLogAction = ( bUser ? "delete user" : "delete permission" );

	cServedIndexRefPtr_c pIndex { MakeDynamicAuthIndex ( sName, true ) };
	if ( !pIndex )
	{
		sError.SetSprintf ( "no such table %s", sName.cstr() );
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), sLogAction, sError );
		return 0;
	}

	CSphString sUser = GetFilterString ( tStmt, ( bUser ? AuthUsersIndex_c::GetColumnName ( AuthUsersIndex_c::SchemaColumn_e::Username ) : AuthPermsIndex_c::GetColumnName ( AuthPermsIndex_c::SchemaColumn_e::Username ) ) );
	if ( sUser.IsEmpty() )
	{
		sError.SetSprintf ( "no user set" );
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), sLogAction, sError );
		return 0;
	}

	DocsCollector_c tDocs { tStmt.m_tQuery, tStmt.m_bJson, sName, pIndex, &sError };
	auto dDocs = tDocs.GetValuesSlice();
	if ( !sError.IsEmpty() || dDocs.IsEmpty() )
	{
		if ( !sError.IsEmpty() )
			AuthLog().ActionFailed ( AuthLogGetSessionCtx(), sLogAction, sError );
		return 0;
	}

	RtAccum_t tAccum;
	tAccum.m_dAccumKlist.Append ( dDocs );
	auto pCmd = MakeReplicationCommand ( ReplCmd_e::AUTH_DELETE, sName, GetFirstClusterName() );
	pCmd->m_bCheckIndex = false;
	tAccum.m_dCmd.Add ( std::move ( pCmd ) );

	int iDeletedCount = 0;
	if ( !HandleCmdReplicateDelete ( tAccum, iDeletedCount ) )
	{
		sError = TlsMsg::MoveToString();
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), sLogAction, sError );
	} else
	{
		if ( bUser )
		{
			AuthLog().UserDeleted ( AuthLogGetSessionCtx(), sUser );
		} else
		{
			CSphString sAction = GetFilterString ( tStmt, AuthPermsIndex_c::GetColumnName ( AuthPermsIndex_c::SchemaColumn_e::Action ) );
			CSphString sActionTarget = GetFilterString ( tStmt, AuthPermsIndex_c::GetColumnName ( AuthPermsIndex_c::SchemaColumn_e::Target ) );
			AuthLog().PermissionRevoked ( AuthLogGetSessionCtx(), sUser, sAction, sActionTarget );
		}
	}
	return iDeletedCount;
}

/////////////////////////////////////////////////////////////////////////////
// INSERT

static int GetSchemaColumn ( const char * sDstColname, const StrVec_t & dSchema, CSphString & sError )
{
	int iSrcCol = dSchema.GetFirst ( [&] ( const auto & sSrcColName ) { return sSrcColName==sDstColname; }  );
	if ( iSrcCol==-1 )
		sError.SetSprintf ( "missed column %s", sDstColname );

	return iSrcCol;
}


static int GetSchemaColumn ( const AuthUsersIndex_c::SchemaColumn_e eCol, const StrVec_t & dSchema, CSphString & sError )
{
	return GetSchemaColumn ( AuthUsersIndex_c::GetColumnName ( eCol ), dSchema, sError );
}

static int GetSchemaColumn ( const AuthPermsIndex_c::SchemaColumn_e eCol, const StrVec_t & dSchema, CSphString & sError )
{
	return GetSchemaColumn ( AuthPermsIndex_c::GetColumnName ( eCol ), dSchema, sError );
}

static bool SetUserMember ( const AuthUsersIndex_c::SchemaColumn_e eCol, const SqlInsert_t & tVal, AuthUserCred_t & tUser, CSphString & sError )
{
	switch ( eCol )
	{
	case AuthUsersIndex_c::SchemaColumn_e::Username:
		tUser.m_sUser = tVal.m_sVal;
		if ( tUser.m_sUser.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'username'" );
			return false;
		}
	return true;

	case AuthUsersIndex_c::SchemaColumn_e::Salt:
		tUser.m_dSalt = ReadHexVec ( "salt", FromStr ( tVal.m_sVal ), HASH20_SIZE, false, sError );
	return ( sError.IsEmpty() );

	case AuthUsersIndex_c::SchemaColumn_e::Hashes:
	{
		if ( tVal.m_sVal.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'hashes'" );
			return false;
		}
		CSphVector<BYTE> tRawBson;
		if ( !sphJsonParse ( tRawBson, const_cast<char *>( tVal.m_sVal.cstr() ), false, false, false, sError ) )
		{
			sError.SetSprintf ( "can not read user 'hashes', error: %s", sError.cstr() );
			return false;
		}

		bson::Bson_c tBsonSrc ( tRawBson );
		if ( tBsonSrc.IsEmpty() || !tBsonSrc.IsAssoc() )
		{
			sError.SetSprintf ( "can not read user 'hashes', error: wrong json" );
			return false;
		}

		auto dSha1 = ReadHexVec ( "password_sha1_no_salt", tBsonSrc, HASH20_SIZE, false, sError );
		if ( !sError.IsEmpty() )
			return false;
		memcpy ( tUser.m_tPwdSha1.data(), dSha1.Begin(), HASH20_SIZE );

		tUser.m_dPwdSha256 = ReadHexVec ( "password_sha256", tBsonSrc, HASH256_SIZE, false, sError );
		tUser.m_dBearerSha256 = ReadHexVec ( "bearer_sha256", tBsonSrc, HASH256_SIZE, true, sError );

		return ( sError.IsEmpty() );
	}

	default:
		break;
	}

	sError.SetSprintf ( "unknown column %s", AuthUsersIndex_c::GetColumnName ( eCol ) );
	return false;
}

static CSphFixedVector< int > CheckInsertUser ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const int iExp = tStmt.m_iSchemaSz;
	const int iAttrsCount = (int)AuthUsersIndex_c::SchemaColumn_e::Hashes;
	CSphFixedVector< int > dMapping ( iAttrsCount );

	if ( tStmt.m_iRowsAffected>1 )
	{
		sError.SetSprintf ( "only single user document supported" );
		return dMapping;
	}

	if ( tStmt.m_dInsertSchema.GetLength() )
	{
		if ( tStmt.m_dInsertSchema.GetLength()!=iAttrsCount )
		{
			sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", tStmt.m_dInsertSchema.GetLength(), iAttrsCount );
			return dMapping;
		}
		// id calculated from the content
		if ( tStmt.m_dInsertSchema.Contains ( AuthUsersIndex_c::GetColumnName ( AuthUsersIndex_c::SchemaColumn_e::Id ) ) )
		{
			sError.SetSprintf ( "can not set id column" );
			return dMapping;
		}
		dMapping[0] = GetSchemaColumn ( AuthUsersIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertSchema, sError );
		dMapping[1] = GetSchemaColumn ( AuthUsersIndex_c::SchemaColumn_e::Salt, tStmt.m_dInsertSchema, sError );
		dMapping[2] = GetSchemaColumn ( AuthUsersIndex_c::SchemaColumn_e::Hashes, tStmt.m_dInsertSchema, sError );
		if ( !sError.IsEmpty() )
			return dMapping;

	} else
	{
		int iGot = tStmt.m_dInsertValues.GetLength();
		if ( iAttrsCount!=tStmt.m_iSchemaSz )
		{
			sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", iAttrsCount, iGot );
			return dMapping;
		}
		if ( ( iGot % iExp )!=0 )
		{
			sError.SetSprintf ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
			return dMapping;
		}

		dMapping[0] = 0;
		dMapping[1] = 1;
		dMapping[2] = 2;
	}

	return dMapping;
}

static void UserWrite ( MemoryWriter_c & tWriter, const AuthUserCred_t & tUser )
{
	tWriter.PutString ( tUser.m_sUser );
	SaveArray ( tUser.m_dSalt, tWriter );
	tWriter.PutBytes ( tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() );
	SaveArray ( tUser.m_dPwdSha256, tWriter );
	SaveArray ( tUser.m_dBearerSha256, tWriter );
	SaveArray ( tUser.m_dApiKey, tWriter );
}

static void UserRead ( MemoryReader_c & tReader, AuthUserCred_t & tUser )
{
	tUser.m_sUser = tReader.GetString();
	GetArrayFixed ( tUser.m_dSalt, tReader );
	tReader.GetBytes ( tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() );
	GetArrayFixed ( tUser.m_dPwdSha256, tReader );
	GetArrayFixed ( tUser.m_dBearerSha256, tReader );
	GetArrayFixed ( tUser.m_dApiKey, tReader );
}

static bool ApplyUser ( const SqlStmt_t & tStmt, bool bReplace, MemoryWriter_c & tWriter, CSphString & sError, CSphString & sUser )
{
	CSphFixedVector< int > dMapping = CheckInsertUser ( tStmt, sError );
	if ( !sError.IsEmpty() )
		return false;

	AuthUsersMutablePtr_t tAuthValidate = CopyAuth();

	AuthUserCred_t tUser;

	if ( !SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertValues[dMapping[0]], tUser, sError ) )
	{
		sError.SetSprintf ( "user '%s' error: %s", tUser.m_sUser.cstr(), sError.cstr() );
		return false;
	}
	if ( !SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Salt, tStmt.m_dInsertValues[dMapping[1]], tUser, sError ) )
	{
		sError.SetSprintf ( "user '%s' error: %s", tUser.m_sUser.cstr(), sError.cstr() );
		return false;
	}
	if ( !SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Hashes, tStmt.m_dInsertValues[dMapping[2]], tUser, sError ) )
	{
		sError.SetSprintf ( "user '%s' error: %s", tUser.m_sUser.cstr(), sError.cstr() );
		return false;
	}

	auto * pCurUser = tAuthValidate->m_hUserToken ( tUser.m_sUser );
	if ( pCurUser && !bReplace )
	{
		sError.SetSprintf ( "duplicate user '%s', use REPLACE", tUser.m_sUser.cstr() );
		return false;
	}

	if ( !tUser.MakeApiKey ( sError ) )
		return false;

	UserWrite ( tWriter, tUser );
	sUser = tUser.m_sUser;

	if ( pCurUser )
		*pCurUser = tUser;
	else
		AddUser ( tUser, tAuthValidate );

	return Validate ( tAuthValidate, sError );
}

static void CommitUser ( const VecTraits_T<BYTE> & dData, AuthUsersMutablePtr_t & tAuth )
{
	MemoryReader_c tReader ( dData );
	AuthUserCred_t tUser;
	UserRead ( tReader, tUser );

	// delete all hashes but keep the existed perms
	if ( tAuth->m_hUserToken.Exists ( tUser.m_sUser ) )
		DeleteUser ( tAuth, tUser.m_sUser, false );

	AddUser ( tUser, tAuth );
}

static bool SetPermMember ( const AuthPermsIndex_c::SchemaColumn_e eCol, const SqlInsert_t & tVal, UserPerm_t & tPerm, CSphString & sUser, CSphString & sError )
{
	switch ( eCol )
	{
	case AuthPermsIndex_c::SchemaColumn_e::Username:
		sUser = tVal.m_sVal;
		if ( sUser.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'username'" );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Action:
		tPerm.m_eAction = ReadAction ( FromStr ( tVal.m_sVal ) );
		if ( tPerm.m_eAction==AuthAction_e::UNKNOWN )
		{
			sError.SetSprintf ( "unknown action '%s'", tVal.m_sVal.cstr() );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Target:
		tPerm.SetTarget ( tVal.m_sVal );
		if ( tPerm.m_sTarget.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'target'" );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Allow:
	{
		int64_t iVal = tVal.GetValueInt();
		if ( iVal!=0 && iVal!=1 )
		{
			sError.SetSprintf ( "wrong allowed '" INT64_FMT "'", iVal );
			return false;
		}
		tPerm.m_bAllow = ( iVal==1 );
		return true;
	}

	case AuthPermsIndex_c::SchemaColumn_e::Budget:
	{
		if ( !tVal.m_sVal.IsEmpty() )
		{
			CSphVector<BYTE> tRawBson;
			if ( !sphJsonParse ( tRawBson, const_cast<char *>( tVal.m_sVal.cstr() ), false, false, false, sError ) )
			{
				sError.SetSprintf ( "can not read user 'budget', error: %s", sError.cstr() );
				return false;
			}
		}
		tPerm.m_sBudget = tVal.m_sVal;

		return true;
	}

	default:
		break;
	}

	sError.SetSprintf ( "unknown column %s", AuthPermsIndex_c::GetColumnName ( eCol ) );
	return false;
}

static CSphFixedVector< int > CheckInsertPerm ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const int iExp = tStmt.m_iSchemaSz;
	// attributes without ID
	const int iAttrsCount = (int)AuthPermsIndex_c::SchemaColumn_e::Budget;
	CSphFixedVector< int > dMapping ( iAttrsCount );

	if ( tStmt.m_iRowsAffected>1 )
	{
		sError.SetSprintf ( "only single permission document supported" );
		return dMapping;
	}

	if ( tStmt.m_dInsertSchema.GetLength() )
	{
		if ( tStmt.m_dInsertSchema.GetLength()!=iAttrsCount )
		{
			sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", tStmt.m_dInsertSchema.GetLength(), iAttrsCount );
			return dMapping;
		}
		// id calculated from the content
		if ( tStmt.m_dInsertSchema.Contains ( AuthPermsIndex_c::GetColumnName ( AuthPermsIndex_c::SchemaColumn_e::Id ) ) )
		{
			sError.SetSprintf ( "can not set id column" );
			return dMapping;
		}
		dMapping[0] = GetSchemaColumn ( AuthPermsIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertSchema, sError );
		dMapping[1] = GetSchemaColumn ( AuthPermsIndex_c::SchemaColumn_e::Action, tStmt.m_dInsertSchema, sError );
		dMapping[2] = GetSchemaColumn ( AuthPermsIndex_c::SchemaColumn_e::Target, tStmt.m_dInsertSchema, sError );
		dMapping[3] = GetSchemaColumn ( AuthPermsIndex_c::SchemaColumn_e::Allow, tStmt.m_dInsertSchema, sError );
		dMapping[4] = GetSchemaColumn ( AuthPermsIndex_c::SchemaColumn_e::Budget, tStmt.m_dInsertSchema, sError );
		if ( !sError.IsEmpty() )
			return dMapping;
	} else
	{
		int iGot = tStmt.m_dInsertValues.GetLength();
		if ( iAttrsCount!=tStmt.m_iSchemaSz )
		{
			sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", iAttrsCount, iGot );
			return dMapping;
		}
		if ( ( iGot % iExp )!=0 )
		{
			sError.SetSprintf ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
			return dMapping;
		}

		dMapping[0] = 0;
		dMapping[1] = 1;
		dMapping[2] = 2;
		dMapping[3] = 3;
		dMapping[4] = 4;
	}

	return dMapping;
}


static void PermWrite ( MemoryWriter_c & tWriter, const CSphString & sUser, const UserPerm_t & tPerm )
{
	tWriter.PutString ( sUser );
	tWriter.PutByte ( tPerm.m_bAllow ? 1 : 0 );
	tWriter.PutString ( GetActionName ( tPerm.m_eAction ) );
	tWriter.PutString ( tPerm.m_sTarget );
	tWriter.PutString ( tPerm.m_sBudget );
}

static void PermRead ( MemoryReader_c & tReader, CSphString & sUser, UserPerm_t & tPerm )
{
	sUser = tReader.GetString();
	tPerm.m_bAllow = !!tReader.GetVal<BYTE>();
	tPerm.m_eAction = ReadAction ( FromStr ( tReader.GetString() ) );
	tPerm.m_sTarget = tReader.GetString();
	tPerm.m_sBudget = tReader.GetString();

}

static bool ApplyPerm ( const SqlStmt_t & tStmt, bool bReplace, MemoryWriter_c & tWriter, CSphString & sError, CSphString & sUser, CSphString & sAction, CSphString & sTarget )
{
	CSphFixedVector< int > dMapping = CheckInsertPerm ( tStmt, sError );
	if ( !sError.IsEmpty() )
		return false;

	const AuthUsersPtr_t tAuth = GetAuth();

	UserPerm_t tPerm;

	bool bOk = true;
	bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertValues[dMapping[0]], tPerm, sUser, sError );
	bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Action, tStmt.m_dInsertValues[dMapping[1]], tPerm, sUser, sError );
	bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Target, tStmt.m_dInsertValues[dMapping[2]], tPerm, sUser, sError );
	bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Allow, tStmt.m_dInsertValues[dMapping[3]], tPerm, sUser, sError );
	bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Budget, tStmt.m_dInsertValues[dMapping[4]], tPerm, sUser, sError );
	if ( !bOk )
		return false;

	SphAttr_t tPermHash = Hash ( sUser, tPerm );
	const UserPerms_t * pPerms = tAuth->m_hUserPerms ( sUser );
	if ( pPerms )
	{
		int iReplaced = pPerms->GetFirst ( [&] ( const UserPerm_t & tItem ) { return ( Hash ( sUser, tItem )==tPermHash ); });
		if ( iReplaced!=-1 && !bReplace )
		{
			sError.SetSprintf ( "duplicate perms for user '%s', use REPLACE", sUser.cstr() );
			return false;
		}
	}

	PermWrite ( tWriter, sUser, tPerm );
	sAction = GetActionName ( tPerm.m_eAction );
	sTarget = tPerm.m_sTarget;

	return true;
}

static void CommitPerm ( const VecTraits_T<BYTE> & dData, AuthUsersMutablePtr_t & tAuth )
{
	MemoryReader_c tReader ( dData );
	
	AuthUserCred_t tUser;

	CSphString sUser;
	UserPerm_t tPerm;
	PermRead ( tReader, sUser, tPerm );

	UserPerms_t & dPerms = tAuth->m_hUserPerms.AddUnique ( sUser );
		
	SphAttr_t tPermHash = Hash ( sUser, tPerm );
	int iReplaced = dPerms.GetFirst ( [&] ( const UserPerm_t & tItem ) { return ( Hash ( sUser, tItem )==tPermHash ); });
	if ( iReplaced!=-1 )
		dPerms[iReplaced] =  tPerm;
	else
		dPerms.Add ( tPerm );

	SortUserPerms ( tAuth->m_hUserPerms[ sUser ] );
}

bool InsertAuthDocuments ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const bool bReplace = ( tStmt.m_eStmt==STMT_REPLACE );

	CSphVector<BYTE> dBuf;
	dBuf.Reserve ( 512 );
	MemoryWriter_c tWriter ( dBuf );

	CSphString sUser, sPermAction, sPermTarget;

	bool bOk = false;
	const bool bUser = ( tStmt.m_sIndex==GetIndexNameAuthUsers() );
	if ( bUser )
		bOk = ApplyUser ( tStmt, bReplace, tWriter, sError, sUser );
	else
		bOk = ApplyPerm ( tStmt, bReplace, tWriter, sError, sUser, sPermAction, sPermTarget );

	if ( bOk )
	{
		RtAccum_t tAccum;
		tAccum.m_dBlobs.Append ( dBuf );

		auto pCmd = MakeReplicationCommand ( ReplCmd_e::AUTH_ADD, tStmt.m_sIndex, GetFirstClusterName() );
		pCmd->m_bCheckIndex = false;
		tAccum.m_dCmd.Add ( std::move ( pCmd ) );

		bOk = HandleCmdReplicate ( tAccum );
	}

	if ( bOk )
	{
		if ( bUser )
		{
			AuthLog().UserAdded ( AuthLogGetSessionCtx(), sUser );
		} else
		{
			AuthLog().PermissionGranted ( AuthLogGetSessionCtx(), sUser, sPermAction, sPermTarget );
		}
	} else
	{
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), ( bUser ? "create user" : "grant permission" ), sUser, sError );
	}

	return bOk;
}

bool InsertAuthDocuments ( const RtAccum_t & tAccum, CSphString & sError )
{
	AuthUsersMutablePtr_t tAuth = CopyAuth();

	if ( tAccum.m_dCmd[0]->m_sIndex==GetIndexNameAuthUsers() )
		CommitUser ( tAccum.m_dBlobs, tAuth );
	else
		CommitPerm ( tAccum.m_dBlobs, tAuth );

	return SaveAuth ( std::move( tAuth ), sError );
}

static void ReportClusterFailure ( const CSphString & sClusterName, const CSphString & sNodeName, const CSphString & sError )
{
	CSphString sMsg;
	sMsg.SetSprintf ( "join cluster '%s' via node '%s'", sClusterName.cstr(), sNodeName.cstr() );
	AuthLog().ActionFailed ( AuthLogGetSessionCtx(), sMsg, sError );
}

bool ChangeAuth ( char * sSrc, const CSphString & sClusterName, const CSphString & sNodeName, CSphString & sError )
{
	AuthLog().ClusterJoinOverride ( sClusterName );

	AuthUsersMutablePtr_t tAuth = ReadAuth ( sSrc, sClusterName, sError );
	if ( !tAuth )
	{
		ReportClusterFailure ( sClusterName, sNodeName, sError );
		return false;
	}

	AuthUsersPtr_t pCurAuth = GetAuth();
	const AuthUserCred_t * pSrcBuddy = pCurAuth->m_hUserToken ( GetAuthBuddyName() );
	AddBuddyToken ( pSrcBuddy, *tAuth );

	bool bOk = SaveAuth ( std::move( tAuth ), sError );
	if ( bOk )
	{
		CSphString sCurAuth = WriteJson ( *pCurAuth.get() );
		AuthLog().ClusterBackupData ( sCurAuth, sClusterName, sNodeName );
	} else
	{
		ReportClusterFailure ( sClusterName, sNodeName, sError );
	}

	return bOk;
}

static CSphFixedVector<BYTE> CreateBearerToken ( const VecTraits_T<BYTE> & dSalt, const VecTraits_T<BYTE> & dRawToken )
{
	std::unique_ptr<SHA256_i> pHashToken { CreateSHA256() };
	pHashToken->Init();
	pHashToken->Update ( (const BYTE *)dRawToken.Begin(), dRawToken.GetLength() );
	auto tHashTokenData = pHashToken->FinalHash();

	std::unique_ptr<SHA256_i> pHashBearer256 { CreateSHA256() };
	pHashBearer256->Init();
	pHashBearer256->Update ( dSalt.Begin(), dSalt.GetLength() );
	pHashBearer256->Update ( tHashTokenData.data(), tHashTokenData.size() );
	auto tBearer256 = pHashBearer256->FinalHash();

	CSphFixedVector<BYTE> dBearer ( 0 );
	dBearer.CopyFrom ( VecTraits_T<BYTE> ( tBearer256.data(), tBearer256.size() ) );

	return dBearer;
}

static CSphString CreateBearerToken ( const CSphString & sUser, CSphString & sError )
{
	if ( !IsAuthEnabled() )
	{
		sError = "authorization disabled, can not create token";
		return {};
	}

	AuthUsersPtr_t pUsers = GetAuth();
	AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
	{
		sError.SetSprintf ( "user '%s' not found", sUser.cstr() );
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), ( "bearer token" ), sUser, sError );
		return {};
	}

	CSphFixedVector<BYTE> dRawToken ( AuthUserCred_t::m_iSourceTokenLen );
	if ( !MakeRandBuf ( dRawToken, sError ) )
	{
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), ( "bearer token" ), sUser, sError );
		return {};
	}

	AuthUserCred_t tNewUser = *pUser;
	tNewUser.m_dBearerSha256 = CreateBearerToken ( pUser->m_dSalt, dRawToken );

	AuthUsersMutablePtr_t tNewUsers = CopyAuth();
	DeleteUser ( tNewUsers, sUser, false );
	AddUser ( tNewUser, tNewUsers );

	// FIXMEE!!! save only via replicate, bot not for the buddy user
	if ( !SaveAuth ( std::move ( tNewUsers ), sError ) )
	{
		AuthLog().ActionFailed ( AuthLogGetSessionCtx(), ( "bearer token" ), sUser, sError );
		return {};
	}

	auto tHashTokenData = CalcBinarySHA2 ( dRawToken.Begin(), dRawToken.GetLength() );
	GetBearerCache().AddUser ( tHashTokenData, sUser );

	AuthLog().UserTokenChanged ( AuthLogGetSessionCtx(),  sUser );

	return BinToHex ( dRawToken );
}

CSphString CreateSessionToken ( CSphString & sError )
{
	return CreateBearerToken ( session::GetUser(), sError );
}

CSphString CreateBuddyToken ( CSphString & sError )
{
	return CreateBearerToken ( GetAuthBuddyName(), sError );
}
