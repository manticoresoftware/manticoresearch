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

#include "std/base64.h"
#include "searchdha.h"
#include "dynamic_idx.h"
#include "searchdsql.h"
#include "docs_collector.h"
#include "client_session.h"
#include "searchdreplication.h"
#include "netfetch.h"

#include "auth_common.h"
#include "auth_proto_mysql.h"
#include "auth_proto_http.h"
#include "auth_proto_api.h"
#include "auth.h"

static CSphMutex g_tAuthLock;
static AuthUsersPtr_t g_tAuth;

static CSphString g_sAuthFile;

/////////////////////////////////////////////////////////////////////////////
/// auth load

static void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tDst );

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	CSphString sFile = hSearchd.GetStr ( "auth" );
	if ( sFile.IsEmpty() || sFile=="0" )
		return;

	if ( !IsCurlAvailable() )
		sphFatal ( "no curl found, can not start auth" );

	CSphString sError;
	if ( IsConfigless() )
	{
		if ( sFile!="1" )
			sphFatal ( "cat not set file name in RT mode, enable auth with the '1'" );
		sFile = GetDatadirPath ( "auth.json" );

		// need to create ewmpty file in the RT mode
		if ( !sphIsReadable ( sFile, nullptr ) && !CreateAuthFile ( sFile, sError ) )
			sphFatal ( "%s", sError.cstr() );
	}

	g_sAuthFile = RealPath ( sFile );
	AuthUsersMutablePtr_t tAuth = ReadAuthFile ( g_sAuthFile, sError );
	if ( !tAuth )
		sphFatal ( "%s", sError.cstr() );

	AddBuddyToken ( nullptr, *tAuth );
	tAuth->m_bEnabled = true;
	
	ScopedMutex_t tLock ( g_tAuthLock );
	g_tAuth = std::move ( tAuth );
}

bool IsAuthEnabled()
{
	if ( !g_tAuth )
		return false;

	return g_tAuth->m_bEnabled;
}

CSphString GetBuddyToken()
{
	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pBuddy = pUsers->m_hUserToken ( GetAuthBuddyName() );
	if ( !pBuddy )
		return CSphString();
	return pBuddy->m_sRawBearerSha256;
}

const CSphString CreateSessionToken()
{
	CSphString sToken;

	if ( !IsAuthEnabled() )
		return sToken;

	AuthUsersPtr_t pUsers = GetAuth();
	AuthUserCred_t * pAuth = pUsers->m_hUserToken ( session::GetUser() );
	if ( !pAuth )
		return sToken;

	sToken = pAuth->m_sRawBearerSha256;
	return sToken;
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
		return false;

	ScopedMutex_t tLock ( g_tAuthLock );
	const AuthUserCred_t * pSrcBuddy = g_tAuth->m_hUserToken ( GetAuthBuddyName() );
	AddBuddyToken ( pSrcBuddy, *tAuth );
	tAuth->m_bEnabled = true;
	g_tAuth = std::move ( tAuth );

	return true;
}

void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tAuth )
{
	AuthUserCred_t tBuddy;
	if ( !pSrcBuddy )
	{
		tBuddy.m_sUser = GetAuthBuddyName();

		SHA1_c tSaltHash;
		tSaltHash.Init();
		DWORD uScramble[4] = { sphRand(), sphRand(), sphRand(), sphRand() };
		tSaltHash.Update ( (const BYTE *)&uScramble, sizeof ( uScramble ) );
		int64_t iDT = sphMicroTimer();
		tSaltHash.Update ( (const BYTE *)&iDT, sizeof ( iDT ) );
		tSaltHash.Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_sSalt = BinToHex ( tSaltHash.FinalHash() );

		SHA1_c tSaltPwd1;
		tSaltPwd1.Init();
		tSaltPwd1.Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		tSaltPwd1.Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_tPwdSha1 = tSaltPwd1.FinalHash();

		std::unique_ptr<SHA256_i> pPwd256 { CreateSHA256() };
		pPwd256->Init();
		pPwd256->Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		pPwd256->Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_sPwdSha256 = BinToHex ( pPwd256->FinalHash() );

		std::unique_ptr<SHA256_i> pToken { CreateSHA256() };
		pToken->Init();
		pToken->Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		pToken->Update ( (const BYTE *)tBuddy.m_sPwdSha256.cstr(), tBuddy.m_sPwdSha256.Length() );
		tBuddy.m_sRawBearerSha256 = tBuddy.m_sBearerSha256 = BinToHex ( pToken->FinalHash() );

		pSrcBuddy = &tBuddy;
	}

	tAuth.m_hUserToken.AddUnique ( pSrcBuddy->m_sUser ) = *pSrcBuddy;
	tAuth.m_hHttpToken2User.AddUnique ( pSrcBuddy->m_sRawBearerSha256 ) = pSrcBuddy->m_sUser;
	tAuth.m_hApiToken2User.AddUnique ( pSrcBuddy->m_sBearerSha256 ) = pSrcBuddy->m_sUser;

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
		PutString ( BinToHex ( (const BYTE *)tUser.m_sSalt.scstr(), tUser.m_sSalt.Length() ), m_tSchema.GetAttr ( (int)SchemaColumn_e::Salt ).m_tLocator, m_pMatch );
		// hashes
		CSphVector<BYTE> dHashes;
		{
			bson::Root_c tHashes ( dHashes );

			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_SHA1 ), BinToHex ( (const BYTE *)tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() ).cstr() );
			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_SHA256 ), BinToHex ( (const BYTE *)tUser.m_sPwdSha256.cstr(), tUser.m_sPwdSha256.Length() ).cstr() );
			tHashes.AddString ( GetColumnName ( SchemaColumn_e::Hash_Bearer ), BinToHex ( (const BYTE *)tUser.m_sBearerSha256.cstr(), tUser.m_sBearerSha256.Length() ).cstr() );
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
	ServedIndexRefPtr_c pIndex = MakeDynamicAuthIndex ( sName, false );
	if ( !pIndex )
		sError << "no such table " << sName;
	return pIndex;
}

static bool SaveAuth ( AuthUsersMutablePtr_t && tAuth, CSphString & sError )
{
	ScopedMutex_t tLock ( g_tAuthLock );
	bool bSaved = SaveAuthFile ( *tAuth.get(), g_sAuthFile, sError );
	sphLogDebug ( "save auth '%s' %s, error: %s", g_sAuthFile.cstr(), ( bSaved ? "succeed" : "failed" ), sError.scstr() );
	 
	if ( !bSaved )
		return false;

	tAuth->m_bEnabled = true;
	g_tAuth = std::move ( tAuth );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DELETE

static void DeleteUser ( AuthUsersMutablePtr_t & tAuth, const CSphString & sUserName )
{
	const auto & tUser = tAuth->m_hUserToken[sUserName];
	tAuth->m_hHttpToken2User.Delete ( tUser.m_sRawBearerSha256 );
	tAuth->m_hApiToken2User.Delete ( tUser.m_sBearerSha256 );
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

		DeleteUser ( tAuth, sUserName );
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

bool DeleteAuthDocuments (const RtAccum_t & tAccum, int * pDeletedCount, CSphString & sError)
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

int DeleteAuthDocuments ( const CSphString & sName, const SqlStmt_t & tStmt, CSphString & sError )
{
	cServedIndexRefPtr_c pIndex { MakeDynamicAuthIndex ( sName, true ) };
	if ( !pIndex )
	{
		sError.SetSprintf ( "no such table %s", sName.cstr() );
		return 0;
	}

	DocsCollector_c tDocs { tStmt.m_tQuery, tStmt.m_bJson, sName, pIndex, &sError };
	auto dDocs = tDocs.GetValuesSlice();
	if ( !sError.IsEmpty() || dDocs.IsEmpty() )
		return 0;

	RtAccum_t tAccum;
	tAccum.m_dAccumKlist.Append ( dDocs );
	auto pCmd = MakeReplicationCommand ( ReplCmd_e::AUTH_DELETE, sName, GetFirstClusterName() );
	pCmd->m_bCheckIndex = false;
	tAccum.m_dCmd.Add ( std::move ( pCmd ) );

	int iDeletedCount = 0;
	HandleCmdReplicateDelete ( tAccum, iDeletedCount );
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


static bool SetUserMember ( const AuthUsersIndex_c::SchemaColumn_e eCol, const SqlInsert_t & tVal, int iRow, AuthUserCred_t & tUser, CSphString & sError )
{
	switch ( eCol )
	{
	case AuthUsersIndex_c::SchemaColumn_e::Username:
		tUser.m_sUser = tVal.m_sVal;
		if ( tUser.m_sUser.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'username' at %d document", iRow );
			return false;
		}
	return true;

	case AuthUsersIndex_c::SchemaColumn_e::Salt:
		tUser.m_sSalt = ReadHex ( FromStr ( tVal.m_sVal ), HASH20_SIZE, sError );
	return true;

	case AuthUsersIndex_c::SchemaColumn_e::Hashes:
	{
		if ( tVal.m_sVal.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'hashes' at %d document", iRow );
			return false;
		}
		CSphVector<BYTE> tRawBson;
		if ( !sphJsonParse ( tRawBson, const_cast<char *>( tVal.m_sVal.cstr() ), false, false, false, sError ) )
		{
			sError.SetSprintf ( "can not read user 'hashes' at %d document, error: %s", iRow, sError.cstr() );
			return false;
		}

		bson::Bson_c tBsonSrc ( tRawBson );
		if ( tBsonSrc.IsEmpty() || !tBsonSrc.IsAssoc() )
		{
			sError.SetSprintf ( "can not read user 'hashes' at %d document, error: wrong json", iRow );
			return false;
		}

		CSphString sSha1 = ReadHex ( "password_sha1_no_salt", HASH20_SIZE, tBsonSrc, sError );
		memcpy ( tUser.m_tPwdSha1.data(), sSha1.cstr(), HASH20_SIZE );
		tUser.m_sPwdSha256 = ReadHex ( "password_sha256", HASH256_SIZE, tBsonSrc, sError );
		tUser.m_sBearerSha256 = ReadHex ( "bearer_sha256", HASH256_SIZE, tBsonSrc, sError );
		tUser.m_sRawBearerSha256 = bson::String ( tBsonSrc.ChildByName ( "bearer_sha256" ) );

		return sError.IsEmpty();
	}

	default:
		break;
	}

	sError.SetSprintf ( "unknown column %s", AuthUsersIndex_c::GetColumnName ( eCol ) );
	return false;
}

static CSphFixedVector< int > CheckInsertUsers ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const int iExp = tStmt.m_iSchemaSz;
	const int iAttrsCount = (int)AuthUsersIndex_c::SchemaColumn_e::Hashes;
	CSphFixedVector< int > dMapping ( iAttrsCount );

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
	tWriter.PutString ( tUser.m_sSalt );
	tWriter.PutBytes ( tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() );
	tWriter.PutString ( tUser.m_sPwdSha256 );
	tWriter.PutString ( tUser.m_sBearerSha256 );
}

static void UserRead ( MemoryReader_c & tReader, AuthUserCred_t & tUser )
{
	tUser.m_sUser = tReader.GetString();
	tUser.m_sSalt = tReader.GetString();
	tReader.GetBytes ( tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() );
	tUser.m_sPwdSha256 = tReader.GetString();
	tUser.m_sBearerSha256 = tReader.GetString();
}

static bool ApplyUsers ( const SqlStmt_t & tStmt, bool bReplace, MemoryWriter_c & tWriter, CSphString & sError )
{
	CSphFixedVector< int > dMapping = CheckInsertUsers ( tStmt, sError );
	if ( !sError.IsEmpty() )
		return false;

	const AuthUsersPtr_t tAuth = GetAuth();

	tWriter.ZipInt ( tStmt.m_iRowsAffected );

	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; iRow++ )
	{
		AuthUserCred_t tUser;

		bool bOk = true;
		bOk &= SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertValues[dMapping[0] + iRow * dMapping.GetLength()], iRow, tUser, sError );
		bOk &= SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Salt, tStmt.m_dInsertValues[dMapping[1] + iRow * dMapping.GetLength()], iRow, tUser, sError );
		bOk &= SetUserMember ( AuthUsersIndex_c::SchemaColumn_e::Hashes, tStmt.m_dInsertValues[dMapping[2] + iRow * dMapping.GetLength()], iRow, tUser, sError );
		if ( !bOk )
			return false;

		const auto * pCurUser = tAuth->m_hUserToken ( tUser.m_sUser );
		if ( pCurUser && !bReplace )
		{
			sError.SetSprintf ( "duplicate user '%s', use REPLACE", tUser.m_sUser.cstr() );
			return false;
		}

		UserWrite ( tWriter, tUser );
	}

	return true;
}

static void CommitUsers ( const VecTraits_T<BYTE> & dData, AuthUsersMutablePtr_t & tAuth )
{
	MemoryReader_c tReader ( dData );
	int iUsers = tReader.UnzipInt();
	
	AuthUserCred_t tUser;
	for ( int i=0; i<iUsers; i++ )
	{
		UserRead ( tReader, tUser );

		if ( tAuth->m_hUserToken.Exists ( tUser.m_sUser ) )
			DeleteUser ( tAuth, tUser.m_sUser );

		AddUser ( tUser, tAuth );
	}
}

static bool SetPermMember ( const AuthPermsIndex_c::SchemaColumn_e eCol, const SqlInsert_t & tVal, int iRow, UserPerm_t & tPerm, CSphString & sUser, CSphString & sError )
{
	switch ( eCol )
	{
	case AuthPermsIndex_c::SchemaColumn_e::Username:
		sUser = tVal.m_sVal;
		if ( sUser.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'username' at %d document", iRow );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Action:
		tPerm.m_eAction = ReadAction ( FromStr ( tVal.m_sVal ) );
		if ( tPerm.m_eAction==AuthAction_e::UNKNOWN )
		{
			sError.SetSprintf ( "unknown action '%s' at %d document", tVal.m_sVal.cstr(), iRow );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Target:
		tPerm.SetTarget ( tVal.m_sVal );
		if ( tPerm.m_sTarget.IsEmpty() )
		{
			sError.SetSprintf ( "empty 'target' at %d document", iRow );
			return false;
		}
	return true;

	case AuthPermsIndex_c::SchemaColumn_e::Allow:
	{
		int64_t iVal = tVal.GetValueInt();
		if ( iVal!=0 && iVal!=1 )
		{
			sError.SetSprintf ( "wrong allowed '" INT64_FMT "' at %d document", iVal, iRow );
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
				sError.SetSprintf ( "can not read user 'budget' at %d document, error: %s", iRow, sError.cstr() );
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

static CSphFixedVector< int > CheckInsertPerms ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const int iExp = tStmt.m_iSchemaSz;
	// attributes without ID
	const int iAttrsCount = (int)AuthPermsIndex_c::SchemaColumn_e::Budget;
	CSphFixedVector< int > dMapping ( iAttrsCount );

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

static bool ApplyPerms ( const SqlStmt_t & tStmt, bool bReplace, MemoryWriter_c & tWriter, CSphString & sError )
{
	CSphFixedVector< int > dMapping = CheckInsertPerms ( tStmt, sError );
	if ( !sError.IsEmpty() )
		return false;

	const AuthUsersPtr_t tAuth = GetAuth();

	tWriter.ZipInt ( tStmt.m_iRowsAffected );

	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; iRow++ )
	{
		CSphString sUser;
		UserPerm_t tPerm;

		bool bOk = true;
		bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Username, tStmt.m_dInsertValues[dMapping[0] + iRow * dMapping.GetLength()], iRow, tPerm, sUser, sError );
		bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Action, tStmt.m_dInsertValues[dMapping[1] + iRow * dMapping.GetLength()], iRow, tPerm, sUser, sError );
		bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Target, tStmt.m_dInsertValues[dMapping[2] + iRow * dMapping.GetLength()], iRow, tPerm, sUser, sError );
		bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Allow, tStmt.m_dInsertValues[dMapping[3] + iRow * dMapping.GetLength()], iRow, tPerm, sUser, sError );
		bOk &= SetPermMember ( AuthPermsIndex_c::SchemaColumn_e::Budget, tStmt.m_dInsertValues[dMapping[4] + iRow * dMapping.GetLength()], iRow, tPerm, sUser, sError );
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
	}

	return true;
}

static void CommitPerms ( const VecTraits_T<BYTE> & dData, AuthUsersMutablePtr_t & tAuth )
{
	sph::StringSet hUsers;

	MemoryReader_c tReader ( dData );
	int iUsers = tReader.UnzipInt();
	
	AuthUserCred_t tUser;
	for ( int i=0; i<iUsers; i++ )
	{
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

		hUsers.Add ( sUser );
	}

	for ( const auto & tUser : hUsers )
		SortUserPerms ( tAuth->m_hUserPerms[ tUser.first ] );
}

bool InsertAuthDocuments ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const bool bReplace = ( tStmt.m_eStmt==STMT_REPLACE );

	CSphVector<BYTE> dBuf;
	dBuf.Reserve ( 512 );
	MemoryWriter_c tWriter ( dBuf );

	bool bOk = false;
	if ( tStmt.m_sIndex==GetIndexNameAuthUsers() )
		bOk = ApplyUsers ( tStmt, bReplace, tWriter, sError );
	else
		bOk = ApplyPerms ( tStmt, bReplace, tWriter, sError );

	if ( !bOk )
		return false;

	RtAccum_t tAccum;
	tAccum.m_dBlobs.Append ( dBuf );

	auto pCmd = MakeReplicationCommand ( ReplCmd_e::AUTH_ADD, tStmt.m_sIndex, GetFirstClusterName() );
	pCmd->m_bCheckIndex = false;
	tAccum.m_dCmd.Add ( std::move ( pCmd ) );

	return HandleCmdReplicate ( tAccum );
}

bool InsertAuthDocuments ( const RtAccum_t & tAccum, CSphString & sError )
{
	AuthUsersMutablePtr_t tAuth = CopyAuth();

	if ( tAccum.m_dCmd[0]->m_sIndex==GetIndexNameAuthUsers() )
		CommitUsers ( tAccum.m_dBlobs, tAuth );
	else
		CommitPerms ( tAccum.m_dBlobs, tAuth );

	return SaveAuth ( std::move( tAuth ), sError );
}

bool ChangeAuth ( char * sSrc, const CSphString & sSrcName, CSphString & sError )
{
	AuthUsersMutablePtr_t tAuth = ReadAuth ( sSrc, sSrcName, sError );
	if ( !tAuth )
		return false;

	AuthUsersPtr_t pCurAuth = GetAuth();
	const AuthUserCred_t * pSrcBuddy = pCurAuth->m_hUserToken ( GetAuthBuddyName() );
	AddBuddyToken ( pSrcBuddy, *tAuth );

	return SaveAuth ( std::move( tAuth ), sError );
}