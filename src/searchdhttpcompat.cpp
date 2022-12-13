//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "sphinxjsonquery.h"

#include "http/http_parser.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdsql.h"
#include "searchdhttp.h"
#include "client_session.h"

//#undef snprintf
//#undef strtoull
//#undef strtoll
#include "nlohmann/json.hpp"

#include <iostream>

using nljson = nlohmann::json;
#include "http/log_management.h"

static SmallStringHash_T<CSphString> g_hAlias;
static RwLock_t g_tLockAlias;

static RwLock_t g_tLockKbnTable;
static nljson g_tKbnTable = "{}"_json;

static RwLock_t g_tLockSettings;
static nljson g_tSettings = "{}"_json;

static bool g_bEnabled = true; // shortcut to /log_management/settings/enabled

static CSphString g_sLogHttpFilter;
static CSphString g_sKbnTableName = ".kibana";
static CSphString g_sKbnTableAlias = ".kibana_1";
static std::vector<CSphString> g_dKbnTablesNames { ".kibana_task_manager", ".apm-agent-configuration", g_sKbnTableName };

static nljson::json_pointer g_tConfigTables ( "/log_management/tables" );
static nljson::json_pointer g_tConfigSettings ( "/log_management/settings" );

//////////////////////////////////////////////////////////////////////////
// compatibility mode

static void ReplaceSubstring ( std::string & s, const std::string & f, const std::string & t )
{
    assert ( !f.empty() );
    for (auto pos = s.find(f);                // find first occurrence of f
            pos != std::string::npos;         // make sure f was found
            s.replace(pos, f.size(), t),      // replace with t, and
            pos = s.find(f, pos + t.size()))  // find next occurrence of f
    {}
}

static void CreateKbnTable ( const CSphString & sParent, bool bRoot, const nljson & tProps, CreateTableSettings_t & tOpts )
{
	if ( tProps.contains( "properties" ) )
	{
		CreateKbnTable ( sParent, false, tProps["properties"], tOpts );
		return;
	}

	for ( auto & tVal : tProps.items() )
	{
		if ( !tVal.value().is_object() )
			continue;

		CSphString sName;
		if ( bRoot )
			sName = tVal.key().c_str();
		else
			sName.SetSprintf ( "%s.%s", sParent.cstr(), tVal.key().c_str() );

		if ( tVal.value().contains( "properties" ) )
		{
			// create all fields from children objects
			CreateKbnTable ( sName, false, tVal.value()["properties"], tOpts );

			// complex types is simple JSON attribute
			if ( bRoot )
			{
				CreateTableAttr_t & tAttr = tOpts.m_dAttrs.Add();
				tAttr.m_tAttr.m_sName = tVal.key().c_str();
				tAttr.m_tAttr.m_eAttrType = SPH_ATTR_JSON;
			}
			continue;
		}

		if ( !tVal.value().contains( "type" ) )
			continue;

		std::string sType = tVal.value()["type"].get<std::string>();

		// fields at all levels
		if ( sType=="text" )
		{
			CSphColumnInfo & tRaw = tOpts.m_dFields.Add();
			tRaw.m_sName = sName;
			tRaw.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
		}

		if ( !bRoot )
			continue;
		
		// simple types at top level are attributes
		if ( sType=="keyword" || strncmp ( sType.c_str(), "date", sType.size() )==0 )
		{
			CreateTableAttr_t & tAttr = tOpts.m_dAttrs.Add();
			tAttr.m_tAttr.m_sName = sName;
			tAttr.m_tAttr.m_eAttrType = SPH_ATTR_STRING;

			// keywords also got indexed to allows filter by columns
			if ( sType=="keyword" )
			{
				CSphColumnInfo & tRaw = tOpts.m_dFields.Add();
				tRaw.m_sName = sName;
				tRaw.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;
			}
		} else if ( sType=="integer" || sType=="long" || sType=="boolean" )
		{
			CreateTableAttr_t & tAttr = tOpts.m_dAttrs.Add();
			tAttr.m_tAttr.m_sName = sName;
			tAttr.m_tAttr.m_eAttrType = SPH_ATTR_INTEGER;
		} else if ( sType=="object" )
		{
			CreateTableAttr_t & tAttr = tOpts.m_dAttrs.Add();
			tAttr.m_tAttr.m_sName = sName;
			tAttr.m_tAttr.m_eAttrType = SPH_ATTR_JSON;
		} else
			sphWarning ( "skipped column '%s' %s", sName.cstr(), tVal.value().dump().c_str() );
	}
}

typedef CSphVector< std::pair < nljson::json_pointer, CSphString > > ComplexFields_t;

static void AddComplexField ( const char * sFieldName, ComplexFields_t & dFields )
{
	assert ( sFieldName );
	const char * sDot = strchr ( sFieldName, '.' );
	if ( !sDot )
		return;

	std::string sName = sFieldName;
	ReplaceSubstring ( sName, ".", "/" );
	sName = "/" + sName;

	auto & tFieldTrait = dFields.Add ();
	tFieldTrait.first = nljson::json_pointer ( sName );
	tFieldTrait.second = sFieldName;
}

static void CreateKbnTable ( CreateTableSettings_t & tOpts, const nljson & tTbl, ComplexFields_t & dFields )
{
	NameValueStr_t & tIdxType = tOpts.m_dOpts.Add();
	tIdxType.m_sName = "type";
	tIdxType.m_sValue = "rt";

	// need for exists bool query
	NameValueStr_t & tIdxFL = tOpts.m_dOpts.Add();
	tIdxFL.m_sName = "index_field_lengths";
	tIdxFL.m_sValue = "1";

	CSphColumnInfo & tRaw = tOpts.m_dFields.Add();
	tRaw.m_sName = "_raw";
	tRaw.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;

	CSphColumnInfo & tMissedExists = tOpts.m_dFields.Add();
	tMissedExists.m_sName = "missed_exists";
	tMissedExists.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED;

	CreateTableAttr_t & tId = tOpts.m_dAttrs.Add();
	tId.m_tAttr.m_sName = 	"_id";
	tId.m_tAttr.m_eAttrType = SPH_ATTR_STRING;
	CreateTableAttr_t & tVersion = tOpts.m_dAttrs.Add();
	tVersion.m_tAttr.m_sName = 	"_version";
	tVersion.m_tAttr.m_eAttrType = SPH_ATTR_INTEGER;

	const nljson & tProps = tTbl["mappings"]["properties"];
	if ( tProps.is_object() )
	{
		CSphString sName;
		CreateKbnTable ( sName, true, tProps, tOpts );
	}

	for ( const CSphColumnInfo & tField : tOpts.m_dFields )
		AddComplexField ( tField.m_sName.cstr(), dFields );
}

int64_t GetDocID ( const char * sID )
{
	int64_t iID = (int64_t)sphFNV64 ( sID );
	if ( iID<0 )
		iID = -iID;

	return iID;
}

int GetVersion ( const nljson & tDoc )
{
	int iVer = 1;
	if ( tDoc.contains ( "_version" ) )
		iVer = tDoc["_version"].get<int>();

	return iVer;
}

static bool InsertDoc ( const CSphString & sSrc, bool bReplace, CSphString & sError )
{
	DocID_t tTmpID;
	SqlStmt_t tStmt;
	if ( !sphParseJsonInsert ( sSrc.cstr(), tStmt, tTmpID, false, true, sError ) )
		return false;

	assert ( tTmpID>=0 );

	JsonObj_c tResult = JsonNull;
	tStmt.m_eStmt = ( bReplace ? STMT_REPLACE : STMT_INSERT );

	std::unique_ptr<StmtErrorReporter_i> pReporter ( CreateHttpErrorReporter() );
	sphHandleMysqlInsert ( *pReporter.get(), tStmt );

	if ( pReporter->IsError() )
	{
		sError = pReporter->GetError();
		return false;
	}

	return true;
}

static bool InsertDoc ( const CSphString & sIndex, const ComplexFields_t & dFields, const nljson & tSrc, bool bReplace, const char * sId, int iVersion, CSphString & sError )
{
	nljson tVal;
	tVal["index"] = sIndex.cstr();
	tVal["id"] = GetDocID ( sId );
	tVal["doc"] = tSrc;
	tVal["doc"]["_id"] = sId;
	tVal["doc"]["_version"] = iVersion;
	tVal["doc"]["_raw"] = tSrc;
	// add fields from objects
	for ( const auto & tField : dFields )
	{
		if ( tSrc.contains ( tField.first ) )
			tVal["doc"][tField.second.cstr()] = tSrc[tField.first].dump().c_str();
	}

	CSphString sSrc = tVal.dump().c_str();
	return InsertDoc ( sSrc, bReplace, sError );
}

static void InsertIntoKbnTable ( const CSphString & sIndex, const nljson & tTbl, const ComplexFields_t & dFields )
{
	CSphString sError;
	int iDocs = 0;
	int iFailed = 0;
	const nljson & tHits = tTbl["hits"];
	for ( auto & tDoc : tHits.items() )
	{
		if ( !tDoc.value().is_object() )
			continue;

		iDocs++;
		const CSphString sRefDocID ( tDoc.value()["_id"].get<std::string>().c_str() );
		const nljson & tSrc = tDoc.value()["_source"];

		if ( !InsertDoc ( sIndex, dFields, tSrc, false, sRefDocID.cstr(), GetVersion ( tDoc.value() ), sError ) )
		{
			iFailed++;
			sphWarning ( "doc '%s', error: %s", sRefDocID.cstr(), sError.cstr() );
			continue;
		}
	}

	//sphInfo ( "kibana table '%s' docs: inserted %d, failed %d", sIndex.cstr(), (int)iDocs-iFailed, iFailed ); // !COMMIT
}

static void CreateAliases()
{
	ScRL_t tLockTbl ( g_tLockKbnTable );
	for ( auto & tTbl : g_tKbnTable.items() )
	{
		if ( !tTbl.value().is_object() || !tTbl.value().contains( "aliases" ) )
			continue;

		CSphString sName = tTbl.key().c_str();

		ScWL_t tLock ( g_tLockAlias );
		for ( auto & tAlias : tTbl.value()["aliases"].items() )
		{
			CSphString sAlias = tAlias.key().c_str();
			g_hAlias.Add ( sName, sAlias );
		}
	}

	sphInfo ( "created %d aliases, tables %d", g_hAlias.GetLength(), (int)g_tKbnTable.size() );
}

static void CreateKbnIndexes()
{
	ScRL_t tLockTbl ( g_tLockKbnTable );
	for ( auto & tTbl : g_tKbnTable.items() )
	{
		if ( !tTbl.value().is_object() )
			continue;

		CSphString sName = tTbl.key().c_str();
		ComplexFields_t dFields;

		if ( tTbl.value().contains( "mappings" ) )
		{
			StrVec_t dWarnings;
			CSphString sError;

			if ( !DropIndexInt ( sName, true, sError ) )
				sphWarning ( "%s", sError.cstr() );

			CreateTableSettings_t tOpts;
			CreateKbnTable ( tOpts, tTbl.value(), dFields );

			if ( !CreateNewIndexConfigless ( sName, tOpts, dWarnings, sError ) )
				sphWarning ( "%s", sError.cstr() );

			for ( const CSphString & sWarn : dWarnings )
				sphWarning ( "%s", sWarn.cstr() );

			sphInfo ( "created kibana table '%s'", sName.cstr() );
		}

		if ( tTbl.value().contains( "hits" ) )
		{
			InsertIntoKbnTable ( sName, tTbl.value(), dFields );
			tTbl.value().erase ( "hits" );
		}
	}
}

static void CreateScripts();
static void CatColumnsSetup();

void SetupCompatHttp()
{
	if ( g_tSettings.empty() )
		return;

	g_bEnabled = g_tSettings["enabled"];
	if ( !g_bEnabled )
		return;

	Threads::CallCoroutine ( [] { 
	CreateAliases();

	CreateScripts();
	CatColumnsSetup();
	} );
}

void LoadCompatHttp ( const char * sData )
{
	nljson tRaw = nljson::parse ( sData );
	int iLoadedItems = 0;

	if ( tRaw.contains ( g_tConfigTables ) )
	{
		ScWL_t tLock ( g_tLockKbnTable );
		g_tKbnTable = tRaw[g_tConfigTables];
		iLoadedItems = (int)g_tKbnTable.size();
	}

	{
		ScWL_t tLock ( g_tLockSettings );
		if ( tRaw.contains ( g_tConfigSettings ) )
			g_tSettings = tRaw[g_tConfigSettings];

		if ( !g_tSettings.contains ( "enabled" ) )
			g_tSettings["enabled"] = false;

		if ( !g_tSettings.contains ( "version_string" ) )
			g_tSettings["version_string"] = "ElasticSearch 7.4.1";
	}

	sphInfo ( "load compat http complete, loaded %d items, enabled %d", iLoadedItems, (int)g_tSettings["enabled"] );
}

void SaveCompatHttp ( JsonObj_c & tRoot )
{
	JsonObj_c tRaw ( false );
	{
		ScRL_t tLockTbl ( g_tLockKbnTable );
		ScRL_t tLockSettings ( g_tLockSettings );
		JsonObj_c tTable ( g_tKbnTable.dump().c_str() );
		JsonObj_c tSettings ( g_tSettings.dump().c_str() );
		tRaw.AddItem ( g_tConfigTables.back().c_str(), tTable );
		tRaw.AddItem ( g_tConfigSettings.back().c_str(), tSettings );
	}
	tRoot.AddItem ( g_tConfigTables.parent_pointer().back().c_str(), tRaw );
}

void DumpHttp ( int iReqType, const CSphString & sURL, const CSphString & sBody, const VecTraits_T<BYTE> & dResult )
{
	JsonEscapedBuilder sReq;
	sReq += R"({"request": {)";
	sReq.Appendf ( R"( "method": "%s")", http_method_str ( (http_method)iReqType ) );
	sReq.Appendf ( R"(, "url": "%s")", sURL.cstr() );
	if ( !sBody.IsEmpty() )
	{
		sReq.StartBlock ( nullptr, R"(, "postData": { "text": )", " }" );
		sReq.AppendEscaped ( sBody.cstr(), EscBld::eEscape );
		sReq.FinishBlock ( false );
	}
	sReq += "}";
	if ( !dResult.IsEmpty() )
	{
		const BYTE * pReply = (const BYTE *)memchr  ( dResult.Begin(), '{', dResult.GetLength() );
		if ( pReply )
		{
			int iLen = dResult.GetLength() - ( pReply - dResult.Begin() );
			sReq.StartBlock ( nullptr, R"( , "reply": )", nullptr );
			sReq.AppendEscaped ( (const char *)pReply, EscBld::eNone, iLen );
			sReq.FinishBlock ( false );
		}
	}
	sReq += "}";

	sphWarning ( "--->\n%s\n<---", sReq.cstr() );
}

void DumpHttp ( int iReqType, const CSphString & sURL, const CSphString & sBody )
{
	DumpHttp ( iReqType, sURL, sBody, VecTraits_T<BYTE>() );
}

static StrVec_t SplitURL ( const CSphString & sURL )
{
	StrVec_t dParts;
	sph::Split ( sURL.cstr(), sURL.Length(), "/", [&dParts, &sURL] ( const char * sPart, int iLen )
		{
			if ( !iLen )
				return;

			CSphString sBuf;
			sBuf.SetBinary ( sPart, iLen );
			Str_t tItem = FromStr ( sBuf );
			UriPercentReplace ( tItem );
			dParts.Add ( tItem );
		}
	);

	return dParts;
}

static bool IsEq ( const HttpOptionsHash_t & hOpts, const CSphString & sName, const CSphString & sVal )
{
	const CSphString * pVal = hOpts ( sName );
	return ( pVal && *pVal==sVal );
}

void DumpNLJson ( const nljson & tVal, int iTabs )
{
	const bool bIsObject = tVal.is_object();
	for ( auto it=tVal.cbegin(); it!=tVal.cend(); it++ )
	{
		if ( bIsObject )
		{
			for ( int iT=0; iT<iTabs; iT++ )
				std::cout << "\t";
			std::cout << "key = " << it.key() << " : " << "\n";
		}
		for ( int iT=0; iT<iTabs; iT++ )
			std::cout << "\t";
		std::cout << it.value() << "\n";

		if ( it->is_object() || it->is_array() )
		{
			DumpNLJson ( it.value(), iTabs+1 );
		}
	}
}

class ColumnFuxupTrait_t
{
public:
	ColumnFuxupTrait_t ( const CSphSchema & tSchema )
	{
		for ( int i=0; i<tSchema.GetFieldsCount(); i++ )
		{
			const CSphString & sName = tSchema.GetField ( i ).m_sName;
			const char * sDot = strchr ( sName.cstr(), '.' );
			if ( !sDot )
				continue;

			CSphString sParentName;
			sParentName.SetBinary ( sName.cstr(), sDot-sName.cstr() );
			// might be already has field with such name
			if ( tSchema.GetFieldIndex ( sParentName.cstr() )!=-1 )
				continue;

			StrVec_t & tFields = m_hParentFields.AddUnique ( sParentName );
			tFields.Add ( sName );
		}
	}

	SmallStringHash_T < StrVec_t > m_hParentFields;
	CSphVector<nljson::json_pointer> m_dAttr;
	CSphVector<nljson::json_pointer> m_dFieldsExists;
	CSphVector<nljson::json_pointer> m_dMissedExists;

	void AddColumn ( const CSphSchema & tSchema, const char * sName, const nljson & tItem, const nljson::json_pointer & tPath )
	{
		AddReplaceColumn ( tSchema, sName, tItem, tPath );
		AddExistsParentField ( tSchema, sName, tItem, tPath );
	}

	void ReplaceColumn ( nljson & tFullQuery )
	{
		ReplaceAttrs ( tFullQuery );
		ReplaceFields ( tFullQuery );
		ReplaceMissedExists ( tFullQuery );
	}

private:
	void AddReplaceColumn ( const CSphSchema & tSchema, const char * sName, const nljson & tItem, const nljson::json_pointer & tPath )
	{
		bool bMatch = strcmp ( sName, "match" )==0 || strcmp ( sName, "term" )==0;
		bool bTerms = strcmp ( sName, "terms" )==0;
		bool bPhrase = strcmp ( sName, "match_phrase" )==0;

		bool bAdd = false;

		if ( bMatch || bPhrase )
		{
			assert ( tItem.is_object() && tItem.size()==1 );
			bAdd = ( tSchema.GetFieldIndex ( tItem.cbegin().key().c_str() )==-1 );
		} else if ( bTerms )
		{
			assert ( tItem.is_object() );
			bAdd = ( tSchema.GetFieldIndex ( tItem.cbegin().key().c_str() )==-1 );
		}

		if ( bAdd )
			m_dAttr.Add ( tPath );
	}

	void AddExistsParentField ( const CSphSchema & tSchema, const char * sName, const nljson & tItem, const nljson::json_pointer & tPath )
	{
		bool bExists = strcmp ( sName, "exists" )==0;
		if ( bExists )
		{
			assert ( tItem.is_object() && tItem.size()==1 && tItem.cbegin().key()=="field" );
			CSphString sFieldName = tItem.cbegin().value().get<std::string>().c_str();
			if ( tSchema.GetFieldIndex ( sFieldName.cstr() )!=-1 )
				return;

			if ( m_hParentFields.Exists ( sFieldName ) )
				m_dFieldsExists.Add ( tPath );
			else
				m_dMissedExists.Add ( tPath );
		}
	}

	void ReplaceAttrs ( nljson & tFullQuery )
	{
		if ( !m_dAttr.GetLength() )
			return;

		//std::cout << "attrs to replace: " << dAttr.GetLength() << "\n";
		for ( const auto & tIt : m_dAttr )
		{
			//std::cout << tIt << " : " << tFullQuery[tIt] << "\n";

			nljson::json_pointer tParent = tIt.parent_pointer();
			nljson::json_pointer tNew = tParent;
			tNew /= "equals";
			tFullQuery[tNew] = tFullQuery[tIt];
			tFullQuery[tParent].erase ( tIt.back() );

			if ( tFullQuery[tNew].cbegin().value().count ( "query" )==1 )
			{
				nljson tEq = R"({})"_json;
				tEq[tFullQuery[tNew].cbegin().key()] = tFullQuery[tNew].cbegin().value()["query"];
				tFullQuery[tNew] = tEq;
			}

			//std::cout << tParent << " : " << tFullQuery[tParent] << "\n";
		}

		//std::cout << tFullQuery << "\n";
	}
	
	void ReplaceFields ( nljson & tFullQuery )
	{
		if ( !m_dFieldsExists.GetLength() )
			return;

		//std::cout << "fields to replace: " << m_dFieldsExists.GetLength() << "\n";
		for ( const auto & tIt : m_dFieldsExists )
		{
			//std::cout << tIt << " : " << tFullQuery[tIt] << "\n";
			const nljson & tRefExists = tFullQuery[tIt];

			assert ( tRefExists.is_object() && tRefExists.size()==1 && tRefExists.cbegin().key()=="field" );
			CSphString sFieldName = tRefExists.cbegin().value().get<std::string>().c_str();

			const StrVec_t & dFields = m_hParentFields[sFieldName];

			nljson tExistVec = R"([])"_json;
			for ( const auto & tFieldIt : dFields )
			{
				nljson tNewField;
				tNewField["exists"] = { { "field", tFieldIt.cstr() } };
				tExistVec.push_back ( tNewField );
			}

			nljson tShouldObj;
			tShouldObj["should"] = tExistVec;
			nljson::json_pointer tParent = tIt.parent_pointer();
			tFullQuery[tParent] = tShouldObj;

			//std::cout << tParent << " : " << tFullQuery[tParent] << "\n";
		}

		//std::cout << tFullQuery << "\n";
	}

	void ReplaceMissedExists ( nljson & tFullQuery )
	{
		if ( !m_dMissedExists.GetLength() )
			return;

		// std::cout << "missed exists: " << m_dMissedExists.GetLength() << "\n";
		for ( const auto & tIt : m_dMissedExists )
		{
			//std::cout << tIt << " : " << tFullQuery[tIt] << "\n";

			nljson::json_pointer tParent = tIt.parent_pointer();
			tFullQuery[tParent] = R"({ "term": { "missed_exists": "none" } })"_json;

			//std::cout << tFullQuery[tParent] << "\n";
		}

		//std::cout << tFullQuery << "\n";
	}
};

static void FixupFilterFeilds ( const CSphSchema & tSchema, const nljson::json_pointer & tParent, const nljson & tVal, ColumnFuxupTrait_t & tArg )
{
	const bool bIsObject = tVal.is_object();
	const bool bIsArray = tVal.is_array();
	int iItem = 0;
	for ( auto tIt=tVal.cbegin(); tIt!=tVal.cend(); tIt++ )
	{
		//std::cout << "(" << tParent << ")\t";
		//if ( bIsObject )
		//	std::cout << tIt.key() << " : \t";

		//if ( bIsObject )
		//	std::cout << "obj \n";
		//if ( bIsArray )
		//	std::cout << "arr[" << iItem << "]\n";

		if ( tIt->is_object() || tIt->is_array() )
		{
			nljson::json_pointer tPath = tParent;
			if ( bIsObject )
				tPath /= tIt.key();
			if ( bIsArray )
				tPath /= iItem;

			if ( bIsObject ) 
				tArg.AddColumn ( tSchema, tIt.key().c_str(), tIt.value(), tPath );

			FixupFilterFeilds ( tSchema, tPath, tIt.value(), tArg );
		}

		iItem++;
	}

}

static void FixupFilterMatchPhrase ( const CSphSchema & tSchema, nljson & tVal )
{
	const auto & tAttrObj = tVal.begin();
	if ( !tAttrObj.value().is_object() )
		return;

	const auto & tAttrType = tAttrObj.value().begin();
	if ( !tAttrType.value().is_object() || !tAttrType.value().contains( "query" ) )
		return;

	if ( tSchema.GetField ( tAttrType.key().c_str() ) )
		return;

	nljson tNew;
	tNew["equals"][tAttrType.key()] = tAttrType.value()["query"];
	tVal = tNew;
}

static void FixupFilterType ( const CSphSchema & tSchema, nljson & tVal )
{
	auto tAttrObj = tVal.begin();
	const CSphColumnInfo * pCol = tSchema.GetAttr ( tAttrObj.key().c_str() );
	if ( !pCol )
		return;

	// FIXME!!! implement full N to M mappings
	if ( tAttrObj.value().is_string() && pCol->m_eAttrType!=SPH_ATTR_STRING )
	{
		CSphVariant tVal ( tAttrObj.value().get<std::string>().c_str() );
		switch ( pCol->m_eAttrType )
		{
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_BOOL:
			tAttrObj.value() = tVal.intval();
			break;
		case SPH_ATTR_FLOAT:
			tAttrObj.value() = tVal.floatval();
			break;
		case SPH_ATTR_BIGINT:
			tAttrObj.value() = tVal.int64val();
			break;

		default: break; // implement full N to M mappings
		}
	}
}

static void FixupFilter ( const StrVec_t & dIndexes, nljson & tFullQuery )
{
	CSphSchema tSchema;
	for ( const CSphString & sName : dIndexes )
	{
		auto tIndex ( GetServed ( sName ) );
		if ( tIndex )
		{
			tSchema = RIdx_c( tIndex )->GetMatchSchema();
			break;
		}
	}

	if ( !tSchema.GetAttrsCount() )
		return;
	
	if ( !tFullQuery.contains ( "query" ) )
		return;

	nljson & tQuery = tFullQuery["query"];

	ColumnFuxupTrait_t tArg ( tSchema );
	nljson::json_pointer tParent;
	FixupFilterFeilds ( tSchema, tParent, tQuery, tArg );
	tArg.ReplaceColumn ( tQuery );

	// FIXME!!! move into FixupFilterFeilds
	nljson::json_pointer tQueryFilter ( "/query/bool/filter" );
	if ( !tFullQuery.contains ( tQueryFilter ) )
		return;

	for ( auto & tVal : tFullQuery[tQueryFilter].items() )
	{
		if ( !tVal.value().is_object() )
			continue;

		if ( tVal.value().cbegin().key()=="match_phrase" )
		{
			FixupFilterMatchPhrase ( tSchema, tVal.value() );
			continue;
		}

		if ( tVal.value().cbegin().key()=="equals" )
		{
			FixupFilterType ( tSchema, tVal.value().begin().value() );
			continue;
		}
	}
}

template<class T_FN>
static void RecursiveIterate ( const nljson::json_pointer & tParent, const nljson & tVal, T_FN fn )
{
	const bool bIsObject = tVal.is_object();
	const bool bIsArray = tVal.is_array();
	int iItem = 0;
	for ( auto tIt=tVal.cbegin(); tIt!=tVal.cend(); tIt++ )
	{
		if ( tIt->is_structured() )
		{
			nljson::json_pointer tPath = tParent;
			if ( bIsObject )
				tPath /= tIt.key();
			if ( bIsArray )
				tPath /= iItem;
			RecursiveIterate ( tPath, tIt.value(), fn );
		} else
		{
			fn ( tIt, bIsObject, bIsArray, tParent );
		}
		iItem++;
	}

}

static bool IsKibanTable ( const VecTraits_T<CSphString> & dIndexes )
{
	if ( dIndexes.GetLength()!=1 )
		return false;

	bool bKbnTable = false;
	{
		ScRL_t tLockTbl ( g_tLockKbnTable );
		bKbnTable = ( dIndexes.any_of ( [] ( const CSphString & tVal ) { return g_tKbnTable.contains ( tVal.cstr() ); } ) );
	}
	if ( !bKbnTable )
	{
		ScRL_t tLock ( g_tLockAlias );
		bKbnTable = ( g_hAlias.Exists ( dIndexes[0] ) );
	}

	return bKbnTable;
}

static void EscapeKibanaColumnNames ( const StrVec_t & dIndexes, nljson & tFullQuery )
{
	if ( !IsKibanTable ( dIndexes ) )
		return;

	CSphVector<nljson::json_pointer> dEscNames;
	nljson::json_pointer tRootPath;
	RecursiveIterate ( tRootPath, tFullQuery, [&] ( nljson::const_iterator tIt, bool bIsObject, bool bIsArray, const nljson::json_pointer & tParent )
		{
			if ( !bIsObject )
				return;

			const std::string & sKey = tIt.key();
			if ( strchr ( sKey.c_str(), '-' )!= nullptr && strchr ( sKey.c_str(), '.' )!=nullptr )
			{
				nljson::json_pointer tPath = tParent;
				tPath /= tIt.key();
				dEscNames.Add ( tPath );
			}
		}
	);

	StringBuilder_c sNewKey;
	for ( const auto & tPath : dEscNames )
	{
		const char * sKey = tPath.back().c_str();
		sNewKey.Clear();
		sNewKey << '`';
		while ( *sKey )
		{
			if ( *sKey=='.' )
				sNewKey << "`.`";
			else
				sNewKey << *sKey;

			sKey++;
		}
		sNewKey << '`';

		nljson::json_pointer tParent = tPath.parent_pointer();
		nljson::json_pointer tNewPath = tParent;
		tNewPath /= sNewKey.cstr();

		nljson tVal = tFullQuery[tPath];
		tFullQuery[tNewPath] = tVal;
		tFullQuery[tParent].erase ( tPath.back() );
	}
}

static void ReplaceAtColumnNames ( nljson & tFullQuery )
{
	CSphVector<nljson::json_pointer> dAtNames;
	nljson::json_pointer tRootPath;
	RecursiveIterate ( tRootPath, tFullQuery, [&] ( nljson::const_iterator tIt, bool bIsObject, bool bIsArray, const nljson::json_pointer & tParent )
		{
			if ( !bIsObject )
				return;

			const std::string & sKey = tIt.key();
			if ( sKey.c_str()[0]=='@' )
			{
				nljson::json_pointer tPath = tParent;
				tPath /= tIt.key();
				dAtNames.Add ( tPath );
			}
		}
	);

	StringBuilder_c sNewKey;
	for ( const auto & tPath : dAtNames )
	{
		const char * sKey = tPath.back().c_str();
		sNewKey.Clear();
		sNewKey << '_';
		sNewKey << ( sKey + 1 );

		nljson::json_pointer tParent = tPath.parent_pointer();
		nljson::json_pointer tNewPath = tParent;
		tNewPath /= sNewKey.cstr();

		nljson tVal = tFullQuery[tPath];
		tFullQuery[tNewPath] = tVal;
		tFullQuery[tParent].erase ( tPath.back() );
	}
}

static void FixupKibana ( const StrVec_t & dIndexes, nljson & tFullQuery )
{
	// kibana tables query fixup
	nljson::json_pointer tSortScript ( "/sort/_script" );
	if ( tFullQuery.contains ( tSortScript ) )
	{
		tFullQuery.erase ( "sort" );

		if ( dIndexes.GetLength() )
		{
			bool bKbnTable = false;
			{
				ScRL_t tLockTbl ( g_tLockKbnTable );
				bKbnTable = ( dIndexes.any_of ( [] ( const CSphString & tVal ) { return g_tKbnTable.contains ( tVal.cstr() ); } ) );
			}
			if ( !bKbnTable )
			{
				ScRL_t tLock ( g_tLockAlias );
				bKbnTable = ( g_hAlias.Exists ( dIndexes[0] ) );
			}

			if ( !bKbnTable )
				sphWarning ( "removed sort[_script] property at query to not kibana index '%s'", dIndexes[0].cstr() );
		}
	}
}

static void ReportError ( const CSphString & sError, const char * sErrorType , ESphHttpStatus eStatus, CSphVector<BYTE> & dResult, const char * sIndex, bool bHeadReply )
{
	sphWarning ( "%s", sError.cstr() );

	int iStatus = HttpGetStatusCodes ( eStatus );
	CSphString sReply = JsonEncodeResultError ( sError, sErrorType, iStatus, sIndex );
	HttpBuildReplyHead ( dResult, eStatus, sReply.cstr(), sReply.Length(), bHeadReply );
}

static void ReportError ( const CSphString & sError, const char * sErrorType , ESphHttpStatus eStatus, CSphVector<BYTE> & dResult, const char * sIndex )
{
	ReportError ( sError, sErrorType, eStatus, dResult, sIndex, false );
}


void ReportError ( const CSphString & sError, const char * sErrorType , ESphHttpStatus eStatus, CSphVector<BYTE> & dResult )
{
	ReportError ( sError, sErrorType, eStatus, dResult, nullptr, false );
}

static void ReportMissedIndex ( const CSphString & sIndex, const StrVec_t & dUrlParts, bool bHeadReply, CSphVector<BYTE> & dResult )
{
	StringBuilder_c sLocation ( "/", "/", "/" );
	dUrlParts.Apply ( [&sLocation] ( const CSphString & sItem ) { sLocation << sItem; } );
	sphWarning ( "missed index '%s' at '%s'", sIndex.cstr(), sLocation.cstr() );

	CSphString sMsg;
	sMsg.SetSprintf ( "no such index [%s]", sIndex.cstr() );
	ReportError ( sMsg.cstr(), "index_not_found_exception", SPH_HTTP_STATUS_404, dResult, sIndex.cstr(), bHeadReply );
}

static void ReportMissedIndex ( const CSphString & sIndex, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	ReportMissedIndex ( sIndex, dUrlParts, false, dResult );
}
 

static StrVec_t ExpandIndexes ( const CSphString & sSrcIndexes, CSphString & sResIndex )
{
	StrVec_t dLocalIndexes;

	StrVec_t dNames = sphSplit ( sSrcIndexes.cstr(), "," );

	for ( const CSphString & sName : dNames )
	{
		if ( !HasWildcard ( sName.cstr() ) )
		{
			// look for alias
			const CSphString * pAliasIndex = g_hAlias ( sName );
			const CSphString * pIndexName = ( pAliasIndex ? pAliasIndex : &sName );
			
			// then look for local index
			auto pServed ( GetServed ( *pIndexName ) );
			if ( pServed )
				dLocalIndexes.Add ( *pIndexName );
		} else
		{
			StrVec_t dIndexName;

			// look for alias
			// scope for alias hash lock
			{
				ScRL_t tLock ( g_tLockAlias );
				for  ( const auto & tAliasIt : g_hAlias )
				{
					const CSphString & sAliasIndex = tAliasIt.second;
					if ( !sphWildcardMatch ( sAliasIndex.cstr(), sName.cstr() ) )
						continue;

					dIndexName.Add ( sAliasIndex );
				}
			}

			// look for local indexes from alias
			if ( dIndexName.GetLength() )
			{
				for ( const CSphString & sIndexName : dIndexName )
				{
					auto pServed ( GetServed ( sIndexName ) );
					if ( pServed )
						dLocalIndexes.Add ( sIndexName );
				}
			}

			// look for local indexes from wildcards
			ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
			for ( const auto & tIt : *hLocal )
			{
				if ( !tIt.second )
					continue;

				if ( !sphWildcardMatch ( tIt.first.cstr(), sName.cstr() ) )
					continue;

				dLocalIndexes.Add ( tIt.first );
			}
		}
	}

	// remove duplicates
	dLocalIndexes.Uniq();

	StringBuilder_c sIndexes ( "," );
	dLocalIndexes.Apply ( [&sIndexes] ( const CSphString & sName ) { sIndexes += sName.cstr(); }  );

	sResIndex = sIndexes.cstr();
	return dLocalIndexes;
}

static CSphString g_sEmptySearch = R"(
{
  "took": 0,
  "timed_out": false,
  "_shards": {
    "total": 0,
    "successful": 0,
    "skipped": 0,
    "failed": 0
  },
  "hits": {
    "total": {
      "value": 0,
      "relation": "eq"
    },
    "max_score": 0,
    "hits": []
  }
}
)";

static bool DoSearch ( const CSphString & sDefaultIndex, nljson & tReq, const CSphString & sURL, CSphString & sRes )
{
	CSphString sError, sWarning;

	// expand index(es) to index list
	CSphString sIndex = sDefaultIndex;
	if ( tReq.contains ( "index" ) )
		sIndex = tReq["index"].get<std::string>().c_str();

	CSphString sExpandedIndex;
	StrVec_t dIndexes = ExpandIndexes ( sIndex, sExpandedIndex );

	if ( !dIndexes.GetLength() )
	{
		sRes = R"(
{
  "took": 0,
  "timed_out": false,
  "_shards": {
    "total": 0,
    "successful": 0,
    "skipped": 0,
    "failed": 0
  },
  "hits": {
    "total": {
      "value": 0,
      "relation": "eq"
    },
    "max_score": 0,
    "hits": []
  }
}
)";
		return true;
	}

	tReq["index"] = sExpandedIndex.cstr();

	EscapeKibanaColumnNames ( dIndexes, tReq );
	FixupKibana ( dIndexes, tReq );
	FixupFilter ( dIndexes, tReq );
	ReplaceAtColumnNames ( tReq );
		
	CSphString sReqFixed = tReq.dump().c_str();
	JsonObj_c tMntReq = JsonObj_c ( sReqFixed.cstr() );

	JsonQuery_c tQuery;
	tQuery.m_eQueryType = QUERY_JSON;
	tQuery.m_sRawQuery = sReqFixed;

	bool bProfile = false;
	if ( !sphParseJsonQuery ( tMntReq, tQuery, bProfile, sError, sWarning ) )
	{
		sphWarning ( "%s at '%s' body '%s'", sError.cstr(), sURL.cstr(), sReqFixed.cstr() );
		sRes = JsonEncodeResultError ( sError, "parse_exception", 400 );
		return false;
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "%s", sWarning.cstr() );

	std::unique_ptr<PubSearchHandler_c> tHandler ( CreateMsearchHandler ( sphCreateJsonQueryParser(), QUERY_JSON, tQuery ) );
	tHandler->RunQueries();

	CSphFixedVector<AggrResult_t *> dAggsRes ( 1 + tQuery.m_dAggs.GetLength() );
	dAggsRes[0] = tHandler->GetResult ( 0 );
	ARRAY_FOREACH ( i, tQuery.m_dAggs )
		dAggsRes[i+1] = tHandler->GetResult ( i+1 );
	sRes = sphEncodeResultJson ( dAggsRes, tQuery, nullptr, true );

	// want to see at log url and query for search error
	for ( const AggrResult_t * pAggr : dAggsRes )
	{
		if ( !pAggr->m_iSuccesses )
		{
			sphWarning ( "'%s' at '%s' body '%s'", pAggr->m_sError.cstr(), sURL.cstr(), sReqFixed.cstr() );
			break;
		}
	}

	return true;
}

static bool ProcessMSearch ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	if ( tParser.GetBody().IsEmpty() )
	{
		ReportError ( "request body or source parameter is required", "parse_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}
	if ( !tParser.GetBody().Ends( "\n" ) )
	{
		ReportError ( "The msearch request must be terminated by a newline [\n]", "illegal_argument_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	int64_t tmStarted = sphMicroTimer();
	CSphString sError, sWarning;
	//const HttpOptionsHash_t & hOpts = tParser.GetOptions();

	CSphString sDefaultIndex;
	if ( dUrlParts.GetLength()>1 )
		sDefaultIndex = dUrlParts[0];

	//bool bRestTotalHitsInt = IsTrue ( hOpts, "rest_total_hits_as_int" );

	CSphVector<nljson> tSourceReq;
	int iSourceLine = 0;
	SplitNdJson ( tParser.GetBody().cstr(), tParser.GetBody().Length(),
		[&] ( const char * sLine, int iLen )
		{
			nljson tItem = nljson::parse ( sLine );
			if ( ( iSourceLine%2 )==0 )
			{
				tSourceReq.Add ( tItem );
			} else
			{
				tSourceReq.Last().update ( tItem );
			}
			iSourceLine++;
		}
	);

	if ( iSourceLine<2 )
	{
		ReportError ( "Validation Failed: 1: no requests added;", "action_request_validation_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	CSphFixedVector<CSphString> dRes ( tSourceReq.GetLength() );
	for ( int i=0; i<dRes.GetLength(); i++ )
	{
		nljson & tReq = tSourceReq[i];
		DoSearch ( sDefaultIndex, tReq, tParser.GetFullURL(), dRes[i] );
	}

	int64_t tmTook = sphMicroTimer() - tmStarted;

	JsonEscapedBuilder tReply;

	tReply.Appendf ( R"({"took":%d,"responses": [)", (int)( tmTook/1000 ));

	ARRAY_FOREACH ( i, dRes )
	{
		if ( i )
			tReply += ",\n";
		else
			tReply += "\n";

		tReply += dRes[i].cstr();
	}

	tReply += "\n";
	tReply += "]\n";
	tReply += "}";

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), false );

	return true;
}

typedef CSphVector< std::pair < CSphString, int > > DocIdVer_t;

static bool GetDocIds ( const char * sIndexName, const char * sFilterID, DocIdVer_t & dIds, CSphString & sError )
{
	assert ( sIndexName );

	const char * sIdName = "_id";
	const char * sVerName = "_version";

	JsonQuery_c tQuery;
	CSphQueryItem & tItem = tQuery.m_dItems.Add();
	tItem.m_sAlias = sIdName;
	tItem.m_sExpr = sIdName;
	CSphQueryItem & tVer = tQuery.m_dItems.Add();
	tVer.m_sAlias = sVerName;
	tVer.m_sExpr = sVerName;
	tQuery.m_sIndexes = sIndexName;
	// need full index
	tQuery.m_iLimit = 1000;
	tQuery.m_iMaxMatches = 1000;

	if ( sFilterID )
	{
		CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = "_id";
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_dStrings.Add ( sFilterID );
		tFilter.m_bExclude = false;
	}

	std::unique_ptr<PubSearchHandler_c> tHandler ( CreateMsearchHandler ( sphCreateJsonQueryParser(), QUERY_JSON, tQuery ) );
	tHandler->RunQueries();
	const AggrResult_t * pRes = tHandler->GetResult ( 0 );

	if ( !pRes )
	{
		sError.SetSprintf ( "invalid search for index '%s'", sIndexName );
		return false;
	}

	if ( !pRes->m_iSuccesses )
	{
		sError.SetSprintf ( "%s", pRes->m_sError.cstr() );
		return false;
	}

	if ( !pRes->m_sWarning.IsEmpty() )
		sphWarning ( "%s", pRes->m_sWarning.cstr() );

	const ISphSchema & tSchema = pRes->m_tSchema;
	const CSphColumnInfo * pColId = tSchema.GetAttr ( sIdName );
	const CSphColumnInfo * pColVer = tSchema.GetAttr ( sVerName );

	if ( !pColId || !pColVer)
	{
		sError.SetSprintf ( "invalid attrs count %d, id=%d, version=%d, index '%s'", tSchema.GetAttrsCount(), ( pColId ? 1 : 0 ), ( pColVer ? 1 : 0 ), sIndexName );
		return false;
	}
	if ( pColId->m_eAttrType!=SPH_ATTR_STRINGPTR )
	{
		sError.SetSprintf ( "invalid attr type '%s', index '%s'", AttrType2Str ( pColVer->m_eAttrType ), sIndexName );
		return false;
	}
	if ( pColVer->m_eAttrType!=SPH_ATTR_INTEGER )
	{
		sError.SetSprintf ( "invalid attr type '%s', index '%s'", AttrType2Str ( pColId->m_eAttrType ), sIndexName );
		return false;
	}

	const CSphAttrLocator & tLocId = pColId->m_tLocator;
	const CSphAttrLocator & tLocVer = pColVer->m_tLocator;

	auto dMatches = pRes->m_dResults.First ().m_dMatches.Slice ( pRes->m_iOffset, pRes->m_iCount );
	for ( const CSphMatch & tMatch : dMatches )
	{
		const BYTE * pData = ( const BYTE * ) tMatch.GetAttr ( tLocId );
		ByteBlob_t tStr = sphUnpackPtrAttr ( pData );

		auto & tId = dIds.Add();
		tId.first.SetBinary ( (const char *)tStr.first, tStr.second );
		tId.second = tMatch.GetAttr ( tLocVer );
	}

	return true;
}

static bool GetIndexDoc ( const CSphIndex * pIndex, const char * sID, int64_t iSessionUID, CSphVector<BYTE> & dField, CSphString & sError )
{
	assert ( pIndex );

	CSphFixedVector<int> dFieldIds ( 1 );
	dFieldIds[0] = pIndex->GetFieldId ( "_raw", DOCSTORE_TEXT );
	if ( dFieldIds[0]==-1 )
	{
		sError.SetSprintf ( "unknown '_raw' stored field" );
		return false;
	}

	DocID_t tDocid = GetDocID ( sID );

	DocstoreDoc_t tDoc;
	pIndex->GetDoc ( tDoc, tDocid, &dFieldIds, iSessionUID, false );
	if ( !tDoc.m_dFields.GetLength() )
		return true;

	assert ( tDoc.m_dFields.GetLength()==1 );
	tDoc.m_dFields[0].SwapData ( dField );
	return true;
}

static void TableGetDoc ( const CSphString & sId, const CSphIndex * pIndex, const CSphString & sIndex, CSphVector<BYTE> & dRawDoc, int & iVersion )
{
	CSphString sError;
	DocIdVer_t dDocVer;
	if ( !GetDocIds ( sIndex.cstr(), sId.cstr(), dDocVer, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	// doc not found
	if ( !dDocVer.GetLength() )
		return;

	if ( dDocVer.GetLength()>2 )
	{
		sphWarning ( "%s multiple documents", sId.cstr() );
		return;
	}
	iVersion = dDocVer[0].second;

	DocstoreSession_c tSession;
	pIndex->CreateReader ( tSession.GetUID() );

	if ( !GetIndexDoc ( pIndex, sId.cstr(), tSession.GetUID(), dRawDoc, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}
}

static bool ProcessKbnTableDoc ( const StrVec_t & dUrlParts, bool bHeadReply, CSphVector<BYTE> & dResult )
{
	const CSphString & sId = dUrlParts.Last();

	CSphString sIndex = dUrlParts[0];
	{
		ScRL_t tLock ( g_tLockAlias );
		const CSphString * pAliasIndex = g_hAlias ( sIndex );
		if ( pAliasIndex )
			sIndex = *pAliasIndex;
	}

	auto tServed ( GetServed ( sIndex ) );
	if ( !tServed )
	{
		sphWarning ( "unknown kibana table %s", sIndex.cstr() );
		return false;
	}

	RIdx_c pIndex ( tServed );

	DocstoreSession_c tSession;
	pIndex->CreateReader ( tSession.GetUID() );

	CSphString sError;
	CSphVector<BYTE> dField;
	if ( !GetIndexDoc ( pIndex, sId.cstr(), tSession.GetUID(), dField, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	DocIdVer_t dDocVer;
	if ( !GetDocIds ( sIndex.cstr(), sId.cstr(), dDocVer, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}
	if ( dField.GetLength() && !dDocVer.GetLength() )
	{
		sphWarning ( "%s mismatch document", sId.cstr() );
		return false;
	}

	nljson tDoc;
	tDoc["_index"] = sIndex.cstr();
	tDoc["_type"] = "_doc";
	tDoc["_id"] = sId.cstr();

	if ( !dField.GetLength() )
	{
		tDoc["found"] = false;
	} else
	{
		nljson tSrc = nljson::parse ( dField.Begin() );
		tDoc["_source"] = tSrc;
		tDoc["_version"] = dDocVer[0].second;
		tDoc["_seq_no"] = 0;
		tDoc["_primary_term"] = 1;
		tDoc["found"] = true;
	}

	JsonEscapedBuilder tReply;
	tReply += tDoc.dump().c_str();

	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), bHeadReply );

	return true;
}

static nljson ReportGetDocError ( const CSphString & sError, const char * sErrorType, const CSphString & sId, const CSphString & sIndex )
{
	nljson tRes = R"({})"_json;
	tRes["_index"] = sIndex.cstr();
	tRes["_id"] = sId.cstr();
	nljson tResError = R"({})"_json;
	tResError["type"] = sErrorType;
	tResError["reason"] = sError.cstr();
	tResError["reason"] = sIndex.cstr();
	tRes["error"] = tResError;

	return tRes;
}

static bool ProcessKbnTableMGet ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	nljson tReq = nljson::parse ( tParser.GetBody().cstr() );
	
	nljson::json_pointer tDocs ( "/docs" );
	nljson::json_pointer tIds ( "/ids" );

	bool bCaseDocs = ( tReq.contains ( tDocs ) );
	bool bCaseIds = ( !bCaseDocs && tReq.contains ( tIds ) );

	if ( !bCaseDocs && !bCaseIds )
	{
		ReportError ( "unknown key for a START_ARRAY, expected [docs] or [ids]", "parsing_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	CSphString sError;
	CSphString sIndex;
	if ( bCaseIds )
	{
		if ( dUrlParts.GetLength()<2 )
		{
			ReportError ( "Validation Failed: 1: index is missing for doc 0;", "action_request_validation_exception", SPH_HTTP_STATUS_400, dResult );
			return true;
		}

		sIndex = dUrlParts[2];
		ScRL_t tLock ( g_tLockAlias );
		const CSphString * pAliasIndex = g_hAlias ( sIndex );
		if ( pAliasIndex )
			sIndex = *pAliasIndex;

		if ( sIndex.IsEmpty() )
		{
			sError.SetSprintf ( "no such index [%s]", dUrlParts[2].cstr() );
			ReportError ( sError.cstr(), "index_not_found_exception", SPH_HTTP_STATUS_400, dResult );
			return true;
		}

		auto tIndex ( GetServed ( sIndex ) );
		if ( !tIndex )
		{
			sError.SetSprintf ( "no such index [%s]", dUrlParts[2].cstr() );
			ReportError ( sError.cstr(), "index_not_found_exception", SPH_HTTP_STATUS_400, dResult  );
			return true;
		}
	}

	nljson tRes = R"({"docs":[]})"_json;
	nljson & tResDocs = tRes[tDocs];

	CSphVector<BYTE> dRawDoc;
	CSphString sId;
	int iDoc = 0;
	nljson::json_pointer tDocId ( "/_id" );
	nljson::json_pointer tDocIdx ( "/_index" );

	for ( const nljson & tDoc : tReq[tDocs] )
	{
		if ( bCaseDocs )
		{
			if ( !tDoc.contains ( tDocId ) )
			{
				sError.SetSprintf ( "Validation Failed: 1: id is missing for doc %d;", iDoc );
				ReportError ( sError.cstr(), "action_request_validation_exception", SPH_HTTP_STATUS_400, dResult  );
				return true;
			}
			if ( !tDoc.contains ( tDocIdx ) )
			{
				sError.SetSprintf ( "Validation Failed: 1: index is missing for doc %d;", iDoc );
				ReportError ( sError.cstr(), "action_request_validation_exception", SPH_HTTP_STATUS_400, dResult  );
				return true;
			}

			sId = tDoc[tDocId].get<std::string>().c_str();
			sIndex = tDoc[tDocIdx].get<std::string>().c_str();

			{
				ScRL_t tLock ( g_tLockAlias );
				const CSphString * pAliasIndex = g_hAlias ( sIndex );
				if ( pAliasIndex )
					sIndex = *pAliasIndex;
			}

			if ( sIndex.IsEmpty() )
			{
				sError.SetSprintf ( "no such index [%s]", sIndex.cstr() );
				tResDocs.push_back ( ReportGetDocError ( sError, "index_not_found_exception", sId, sIndex ) );
				continue;
			}
		} else
		{
			sId = tDoc.get<std::string>().c_str();
		}

		auto tIndex ( GetServed ( sIndex ) );
		if ( !tIndex )
		{
			sError.SetSprintf ( "no such index [%s]", sIndex.cstr() );
			tResDocs.push_back ( ReportGetDocError ( sError, "index_not_found_exception", sId, sIndex ) );
			continue;
		}

		int iVersion = 0;
		dRawDoc.Resize ( 0 );
		TableGetDoc ( sId, RIdx_c ( tIndex ), sIndex, dRawDoc, iVersion );

		nljson tResDoc = R"({"_type": "_doc"})"_json;
		tResDoc["_index"] = sIndex.cstr();
		tResDoc["_id"] = sId.cstr();

		if ( dRawDoc.GetLength() )
		{
			tResDoc["found"] = true;
			tResDoc["_version"] = iVersion;
			tResDoc["_seq_no"] = 1; 
			tResDoc["_primary_term"] = 1;
			tResDoc["_source"] = nljson::parse ( dRawDoc.Begin() );
		} else
		{
			tResDoc["found"] = false;
		}
		tResDocs.push_back ( tResDoc );

		iDoc++;
	}

	JsonEscapedBuilder tReply;
	tReply += tRes.dump().c_str();

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), false );

	return true;
}

static void ProcessPutTemplate ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	assert ( dUrlParts.GetLength() );

	const CSphString & sTblName = dUrlParts[1];

	nljson tTbl = nljson::parse ( tParser.GetBody().cstr() );
	if ( !tTbl.contains ( "order" ) )
		tTbl["order"] = 0;
	if ( !tTbl.contains ( "version" ) )
		tTbl["version"] = 1;

	{
		ScWL_t tLockTbl ( g_tLockKbnTable ); 
		g_tKbnTable["templates"][sTblName.cstr()] = tTbl;
	}

	JsonEscapedBuilder tReply;
	tReply += R"({"acknowledged":true})";

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), false );
}

static void ProcessIgnored ( CSphVector<BYTE> & dResult )
{
	nljson tReplyJs = R"({"took":0, "ignored":true, "errors":false})"_json;

	JsonEscapedBuilder tReply;
	tReply += tReplyJs.dump().c_str();

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), false );
}

static bool ProcessFilterPath ( const nljson & tNode, const VecTraits_T<CSphString> & dParts, int iPart, nljson & tPart )
{
	if ( iPart>=dParts.GetLength() )
		return false;

	const CSphString & sPart = dParts[iPart];
	if ( sPart!="*" )
	{
		nljson::const_iterator tIt = tNode.find ( sPart.cstr() );
		if ( tIt==tNode.end() )
			return false;

		if ( iPart+1<dParts.GetLength() && ( tIt->is_object() || tIt->is_array() ) )
		{
			nljson tVal ( tIt->is_array() ? nljson::array() : nljson::object() );
			tPart[sPart.cstr()] = tVal;
			return ProcessFilterPath ( *tIt, dParts, iPart+1, tPart[sPart.cstr()] );
		} else
		{
			tPart[sPart.cstr()] = *tIt;
			return true;
		}

	} else
	{
		bool bGotMatch = false;
		for ( const auto & tItem : tNode.items() )
		{
			if ( iPart+1<dParts.GetLength() && ( tItem.value().is_object() || tItem.value().is_array() ) )
			{
				nljson tVal ( tItem.value().is_array() ? nljson::array() : nljson::object() );
				if ( tPart.is_array() )
				{
					tPart.push_back ( tVal );
					bGotMatch |= ProcessFilterPath ( tItem.value(), dParts, iPart+1, tPart.back() );
				} else
				{
					tPart[tItem.key()] = ( tVal );
					bGotMatch |= ProcessFilterPath ( tItem.value(), dParts, iPart+1, tPart[tItem.key()] );
				}
			} else if ( iPart+1==dParts.GetLength() ) // add only matched leaf items
			{
				if ( tPart.is_array() )
				{
					tPart.push_back ( tItem.value() );
				} else
				{
					tPart[tItem.key()] = tItem.value();
				}
				bGotMatch = true;
			}
		}

		return bGotMatch;
	}
}

static int ProcessFilter ( const CSphString * sFilters, nljson & tRes )
{
	if ( !sFilters )
		return 0;

	nljson tResFiltered = R"({})"_json;

	StrVec_t dFilters = sphSplit ( sFilters->cstr(), "," );
	for ( const CSphString & sFilter : dFilters )
	{
		nljson tPartJs = R"({})"_json;
		StrVec_t dParts = sphSplit ( sFilter.cstr(), "." );
		if ( ProcessFilterPath ( tRes, dParts, 0, tPartJs ) )
			tResFiltered.merge_patch ( tPartJs );
	}

	tRes = tResFiltered;
	return 1;
}

static bool ProcessKbnTableGet ( const StrVec_t & dUrlParts, const CSphString * sFilters, bool bHeadReply, CSphVector<BYTE> & dResult )
{
	const CSphString & sSrcIndexName = dUrlParts[0];

	nljson::json_pointer tTblName;
	{
		ScRL_t tLockTbl ( g_tLockKbnTable );
		if ( g_tKbnTable.contains ( sSrcIndexName.cstr() ) )
		{
			tTblName /= sSrcIndexName.cstr();
		} else
		{
			ScRL_t tLock ( g_tLockAlias );
			const CSphString * pAliasIndex = g_hAlias ( sSrcIndexName );
			if ( pAliasIndex )
				tTblName /= pAliasIndex->cstr();
		}

		if ( tTblName.empty() || !g_tKbnTable.contains ( tTblName ) )
		{
			ReportMissedIndex ( sSrcIndexName, dUrlParts, bHeadReply, dResult );
			return true;
		}
	}

	nljson tRes;
	if ( tTblName.to_string().rfind ( "/_" )==0 )
	{
		ScRL_t tLockTbl ( g_tLockKbnTable );
		tRes = g_tKbnTable[tTblName];
	} else
	{
		{
			ScRL_t tLockTbl ( g_tLockKbnTable );
			tRes[tTblName] = g_tKbnTable[tTblName];
		}

		// get inner object
		if ( dUrlParts.GetLength()>1 )
		{
			nljson::json_pointer tInner = tTblName;
			for ( int i=1; i<dUrlParts.GetLength(); i++ )
				tInner /= dUrlParts[i].cstr();

			if ( tRes.contains ( tInner ) )
			{
				nljson tTbl = tRes[tInner];
				tRes = tTbl;
			}
		}
	}

	ProcessFilter ( sFilters, tRes );

	JsonEscapedBuilder tReply;
	tReply += tRes.dump().c_str();

	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, tReply.cstr(), tReply.GetLength(), bHeadReply );
	return true;
}

static int ProcessFilterSource ( const CSphString * sSourceFilter, nljson & tRes )
{
	if ( !sSourceFilter )
		return 0;

	// create list of json_pointer to columns at _source field
	StrVec_t dFilters = sphSplit ( sSourceFilter->cstr(), "," );
	CSphFixedVector<nljson::json_pointer> dColumns ( dFilters.GetLength() );
	ARRAY_FOREACH ( i, dColumns )
	{
		std::string sVal = dFilters[i].cstr();
		ReplaceSubstring ( sVal, ".", "/" );
		sVal = "/" + sVal;
		dColumns[i] = nljson::json_pointer ( sVal );
	}

	// filter _source field
	nljson::json_pointer tHits ( "/hits/hits" );
	nljson::json_pointer tSrcCol ( "/_source" );
	for ( auto & tItem : tRes[tHits].items() )
	{
		nljson & tHit = tItem.value();
		if ( !tHit.is_object() || !tHit.contains ( tSrcCol ) )
			continue;
		
		const nljson & tSrc = tHit[tSrcCol];

		nljson tSrcFiltered = R"({})"_json;
		for ( const auto & tCol : dColumns )
		{
			if ( tSrc.contains ( tCol ) )
				tSrcFiltered[tCol] = tSrc[tCol];
		}
		tHit[tSrcCol] = tSrcFiltered;
	}

	return dColumns.GetLength();
}

static void ProcessKbnResult ( const CSphString * sSourceFilter, const CSphString * sFilterPath, CSphString & sRes )
{
	nljson tRes = nljson::parse ( sRes.cstr() );

	nljson::json_pointer tHits ( "/hits/hits" );
	if ( !tRes.contains ( tHits ) )
		return;

	int iFixed = 0;
	nljson::json_pointer tRawCol ( "/_source/_raw" );

	for ( auto & tItem : tRes[tHits].items() )
	{
		nljson & tHit = tItem.value();
		if ( !tHit.is_object() || !tHit.contains ( tRawCol ) )
			continue;

		const auto & tRaw = tHit[tRawCol];
		const auto tRawStr = tRaw.get<std::string>();
		if ( tRawStr.empty() )
			continue;

		nljson tRawObj = nljson::parse ( tRawStr );
		
		nljson & tSrc = tHit["_source"];
		tSrc.merge_patch ( tRawObj );
		tSrc.erase ( "_raw" );
		iFixed++;
	}

	iFixed += ProcessFilter ( sFilterPath, tRes );
	iFixed += ProcessFilterSource ( sSourceFilter, tRes );

	if ( iFixed )
		sRes = tRes.dump().c_str();
}

// FIXME!!! replace with requests to every index with count(*) \ implicit grouper instead of this hack
// or implement group by IndexTag
static bool EmulateIndexCount ( const CSphString & sIndex, const nljson & tReq, CSphVector<BYTE> & dResult )
{
	bool bCountFromIdx = ( tReq.size()>=1 && ( tReq.contains ( "aggs" ) || ( tReq.size()==2 && tReq.contains ( "size" ) ) ) );
	if ( !bCountFromIdx )
		return false;

	nljson::json_pointer tField ( "/aggs/indices/terms/field" );
	bool bFieldIndex = ( tReq.contains ( tField ) && tReq[tField]=="_index" );
	if ( !bFieldIndex )
		return false;

	CSphString sFilter = sIndex;
	if ( sFilter=="_all" )
		sFilter = "*";

	nljson tRes = R"({
    "took": 10, 
    "timed_out": false, 
    "_shards": {
        "total": 0, 
        "successful": 0, 
        "skipped": 0, 
        "failed": 0
    }, 
    "hits": {
        "total": {
            "value": 10, 
            "relation": "eq"
        }, 
        "max_score": null, 
        "hits": []
    }, 
    "aggregations": {
        "indices": {
            "doc_count_error_upper_bound": 0, 
            "sum_other_doc_count": 0, 
            "buckets": []
        }
    }
})"_json;

	nljson::json_pointer tBuckets ( "/aggregations/indices/buckets" );

	int iIndexes = 0;
	int iDocsTotal = 0;
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		if ( !sFilter.IsEmpty() && !sphWildcardMatch ( tIt.first.cstr(), sFilter.cstr() ) )
			continue;

		RIdx_c tIdx ( tIt.second );
		int iDocsCount = tIdx->GetStats().m_iTotalDocuments;
		if ( !iDocsCount )
			continue;

		iIndexes++;
		iDocsTotal += iDocsCount;
		nljson tItem = R"({})"_json;
		tItem["key"] = tIt.first.cstr();
		tItem["doc_count"] = iDocsCount;

		tRes[tBuckets].push_back ( tItem );
	}

	tRes["_shards"]["total"] = iIndexes;
	tRes["_shards"]["successful"] = iIndexes;
	tRes["hits"]["total"]["value"] = iDocsTotal;

	StringBuilder_c sRes;
	sRes += tRes.dump().c_str();

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.GetLength(), false );

	return true;
}

static bool ProcessSearch ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	if ( tParser.GetBody().IsEmpty() )
	{
		ReportError ( "request body or source parameter is required", "parse_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	const CSphString & sIndex = dUrlParts[0];
	nljson tReq = nljson::parse ( tParser.GetBody().cstr() );

	if ( EmulateIndexCount ( sIndex, tReq, dResult ) )
		return true;

	CSphString sRes;
	if ( !DoSearch ( sIndex, tReq, tParser.GetFullURL(), sRes ) )
	{
		HttpBuildReply ( dResult, SPH_HTTP_STATUS_400, sRes.cstr(), sRes.Length(), false );
		return true;
	}

	// filter_path and _source uri params
	ProcessKbnResult ( tParser.GetOptions()( "_source" ), tParser.GetOptions()( "filter_path" ), sRes );

	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessCount ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	if ( tParser.GetBody().IsEmpty() )
	{
		ReportError ( "request body or source parameter is required", "parse_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	const CSphString & sIndex = dUrlParts[0];
	nljson tReq = nljson::parse ( tParser.GetBody().cstr() );

	CSphString sRes;
	if ( !DoSearch ( sIndex, tReq, tParser.GetFullURL(), sRes ) )
	{
		HttpBuildReply ( dResult, SPH_HTTP_STATUS_400, sRes.cstr(), sRes.Length(), false );
		return true;
	}

	nljson tRef = nljson::parse ( sRes.cstr() );
	if ( !tRef.contains ( "error" ) )
	{
		// _count transform
		// FIXME !!! work only up to limit now
		nljson tRes;
		tRes["count"] = tRef["hits"]["total"];
		tRes["_shards"] = tRef["_shards"];
		sRes = tRes.dump().c_str();
	}

	// filter_path and _source uri params
	ProcessKbnResult ( tParser.GetOptions()( "_source" ), tParser.GetOptions()( "filter_path" ), sRes );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static void CatAliases ( bool bJson, StringBuilder_c & sRes )
{
	nljson tJsonRes = R"([])"_json;

	{
		ScRL_t tLock ( g_tLockAlias );
		for ( const auto & tAliasIt : g_hAlias )
		{
			const CSphString & sIndex = tAliasIt.second;
			const CSphString & sAlias = tAliasIt.first;

			if ( bJson )
			{
				nljson tItem = 
R"({
		"filter": "-", 
		"routing.index": "-", 
		"routing.search": "-", 
		"is_write_index": "-"
})"_json;

				tItem["alias"] = sAlias.cstr();
				tItem["index"] = sIndex.cstr();

				tJsonRes.push_back ( tItem );
			} else
			{
				sRes.Appendf ( "%s %s  - - - -\n", sAlias.cstr(), sIndex.cstr() );
			}
		}
	}
	if ( bJson )
		sRes += tJsonRes.dump().c_str();
}

static void CatMaster ( bool bJson, StringBuilder_c & sRes )
{
	ScRL_t tLockTbl ( g_tLockKbnTable );
	const nljson & tMasterTbl = g_tKbnTable["master"];
	if ( bJson )
	{
		nljson tJsonRes = R"([])"_json;
		tJsonRes.push_back ( tMasterTbl );
		sRes += tJsonRes.dump().c_str();
	} else
	{
		sRes.Appendf ( "%s %s %s %s", tMasterTbl["id"].get<std::string>().c_str(), tMasterTbl["host"].get<std::string>().c_str(), tMasterTbl["ip"].get<std::string>().c_str(), tMasterTbl["node"].get<std::string>().c_str() );
	}
}

static void CatTemplates ( bool bJson, const char * sFilter, StringBuilder_c & sRes )
{
	nljson tJsonRes = R"([])"_json;
	StringBuilder_c sTmp;

	ScRL_t tLockTbl ( g_tLockKbnTable );
	const nljson & tTemplates = g_tKbnTable["templates"];
	for ( const auto & tIt : tTemplates.items() )
	{
		const std::string & sName = tIt.key();

		if ( sFilter && !sphWildcardMatch ( sName.c_str(), sFilter ) )
			continue;

		const nljson & tPattern = tIt.value()["index_patterns"];
		const nljson & tOrder = tIt.value()["order"];
		const nljson & tVersion = tIt.value()["version"];

		sTmp.Clear();
		sTmp.StartBlock ( ", ", "[", "]" );
		for ( const auto & tItPattern : tPattern.items() )
			sTmp += tItPattern.value().get<std::string>().c_str();
		sTmp.FinishBlock ( false );

		if ( bJson )
		{
			nljson tItem;

			tItem["name"] = sName;
			tItem["index_patterns"] = sTmp.cstr();
			tItem["order"] = tOrder;
			tItem["version"] = tVersion;

			tJsonRes.push_back ( tItem );
		} else
		{
			sRes.Appendf ( "%s %s %s %s\n", sName.c_str(), sTmp.cstr(), tOrder.get<std::string>().c_str(), tVersion.get<std::string>().c_str() );
		}
	}

	if ( bJson )
		sRes += tJsonRes.dump().c_str();
}

struct CatIndexDesc_t
{
	CSphString m_sName;
	int64_t m_iDocs;
	int64_t m_iSize;
};

// FIXME!!! add support of another 120 column names and shortcuts
enum class CatColumns_e
{
	HEALTH = 0, 
	STATUS, 
	INDEX, 
	UUID, 
	PRI, 
	REP, 
	DOCS_COUNT, 
	DOCS_DEL, 
	STORE_SIZE, 
	PRI_SIZE,

	TOTAL
};

static const char * g_sCatColumnNames[(int)CatColumns_e::TOTAL] = {
	"health,h",
	"status,s", 
	"index,i,idx", 
	"id,uuid", 
	"pri,p,shards.primary,shardsPrimary", 
	"rep,r,shards.replica,shardsReplica", 
	"docs.count,dc,docsCount", 
	"docs.deleted,dd,docsDeleted", 
	"store.size,ss,storeSize", 
	"pri.store.size"
};

static SmallStringHash_T<CatColumns_e> g_hCatColumnNames;

static void CatColumnsSetup()
{
	StrVec_t dNames;
	for ( int i=0; i<(int)CatColumns_e::TOTAL; i++ )
	{
		dNames = sphSplit ( g_sCatColumnNames[i], "," );
		for ( const CSphString & sName : dNames )
			g_hCatColumnNames.Add ( (CatColumns_e)i, sName );
	}
}

static CSphVector<CSphNamedInt> GetCatIndexesColumns ( const CSphString * pColumns )
{
	CSphVector<CSphNamedInt> dCol;
	if ( !pColumns )
	{
		dCol.Resize ( (int)CatColumns_e::TOTAL );
		for ( int i=0; i<(int)CatColumns_e::TOTAL; i++ )
		{
			dCol[i].second = i;
			const char * sName = g_sCatColumnNames[i];
			const char * sDel = strchr ( sName, ',' );
			if ( sDel )
				dCol[i].first.SetBinary ( sName, sDel-sName );
			else
				dCol[i].first = sName;
		}
	} else
	{
		StrVec_t dParsed = sphSplit ( pColumns->cstr(), "," );
		ARRAY_FOREACH ( i, dParsed )
		{
			const CatColumns_e * pCol = g_hCatColumnNames ( dParsed[i] );
			if ( pCol )
				dCol.Add ( std::make_pair ( dParsed[i], (int)( *pCol ) ) );
		}
	}

	return dCol;
}

static void CatPrintColumn ( bool bJson, const CSphNamedInt & tCol, const CatIndexDesc_t & tDesc, nljson & tJsonItem, StringBuilder_c & sRes )
{
	switch ( (CatColumns_e)tCol.second )
	{
	case CatColumns_e::HEALTH: 
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = "green";
		else
			sRes << "green";
	break;

	case CatColumns_e::STATUS:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = "open";
		else
			sRes << "open";
	break;

	case CatColumns_e::INDEX:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = tDesc.m_sName.cstr();
		else
			sRes << tDesc.m_sName;
	break;

	case CatColumns_e::UUID:
	{
		CSphString sBuf;
		sBuf.SetSprintf ( "%016" PRIx64, sphFNV64 ( tDesc.m_sName.cstr() ) );
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = sBuf.cstr();
		else
			sRes << sBuf;
	}
	break;

	case CatColumns_e::PRI:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = 1;
		else
			sRes << 1;
	break;

	case CatColumns_e::REP:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = 0;
		else
			sRes << 0;
	break;

	case CatColumns_e::DOCS_COUNT:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = tDesc.m_iDocs;
		else
			sRes << tDesc.m_iDocs;
	break;

	case CatColumns_e::DOCS_DEL:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = 0;
		else
			sRes << 0;
	break;

	case CatColumns_e::STORE_SIZE:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = tDesc.m_iSize;
		else
			sRes << tDesc.m_iSize;
	break;

	case CatColumns_e::PRI_SIZE:
		if ( bJson )
			tJsonItem[tCol.first.cstr()] = tDesc.m_iSize;
		else
			sRes << tDesc.m_iSize;
	break;

	default: break;
	}
}

// FXIME!!! add support of 'v' option - column names
static void CatIndexes ( bool bJson, const char * sFilter, const CSphString * pColumns, StringBuilder_c & sRes )
{
	nljson tJsonRes = R"([])"_json;

	CSphVector<CatIndexDesc_t> dDesc;
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		if ( sFilter && !sphWildcardMatch ( tIt.first.cstr(), sFilter ) )
			continue;

		RIdx_c tIdx ( tIt.second );

		CatIndexDesc_t & tDesc = dDesc.Add();
		tDesc.m_sName = tIt.first;
		const CSphSourceStats & tStat = tIdx->GetStats();
		tDesc.m_iDocs = tStat.m_iTotalDocuments;
		CSphIndexStatus tStatus;
		tIdx->GetStatus ( &tStatus );
		tDesc.m_iSize = tStatus.m_iDiskUse;
	}

	auto dCol = GetCatIndexesColumns ( pColumns );

	nljson tJsonItem = R"({})"_json;
	for ( const CatIndexDesc_t & tDesc : dDesc )
	{
		if ( bJson )
			tJsonItem = R"({})"_json;
		else
			sRes.StartBlock ( " " );

		for ( const auto & tCol : dCol )
			CatPrintColumn ( bJson, tCol, tDesc, tJsonItem, sRes );

		if ( bJson )
			tJsonRes.push_back ( tJsonItem );
		else
		{
			sRes.FinishBlock ( false );
			sRes += "\n";
		}
	}

	if ( bJson )
		sRes += tJsonRes.dump().c_str();
}

static bool ProcessCat ( const StrVec_t & dUrlParts, const HttpOptionsHash_t & hOpts, const CSphString & sFullURL, bool bHeadReply, CSphVector<BYTE> & dResult )
{
	assert ( dUrlParts.GetLength()>=2 );
	bool bJson = IsEq ( hOpts, "format", "json" );

	StringBuilder_c sRes;

	if ( dUrlParts[1]=="aliases" )
	{
		CatAliases ( bJson, sRes );
	} else if ( dUrlParts[1]=="master" )
	{
		CatMaster ( bJson, sRes );
	} else if ( dUrlParts[1]=="templates" )
	{
		CatTemplates ( bJson, ( dUrlParts.GetLength()>=3 ? dUrlParts[2].cstr() : nullptr ), sRes );
	} else if ( dUrlParts[1]=="indices" )
	{
		CatIndexes ( bJson, ( dUrlParts.GetLength()>=3 ? dUrlParts[2].cstr() : nullptr ), hOpts ( "h" ), sRes );
	} else
	{
		sRes.Sprintf ( "Incorrect HTTP method for uri [%s] and method [GET], allowed: [POST]", sFullURL.cstr() );
		HttpErrorReply ( dResult, SPH_HTTP_STATUS_405, sRes.cstr() );
		return true;
	}

	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.GetLength(), bHeadReply );
	return true;
}

static void ProcessEmptyHead ( bool bHeadReply, CSphVector<BYTE> & dResult )
{
	nljson tJsonRes = R"(
{
  "name" : "4e9d933ebde2",
  "cluster_name" : "docker-cluster",
  "cluster_uuid" : "Z7igA6xDRDKCVwnMuyXCOQ",
  "version" : {
    "number" : "7.4.1",
    "build_flavor" : "default",
    "build_type" : "docker",
    "build_hash" : "fc0eeb6e2c25915d63d871d344e3d0b45ea0ea1e",
    "build_date" : "2019-10-22T17:16:35.176724Z",
    "build_snapshot" : false,
    "lucene_version" : "8.2.0",
    "minimum_wire_compatibility_version" : "6.8.0",
    "minimum_index_compatibility_version" : "6.0.0-beta1"
  },
  "tagline" : "You Know, for Search"
}
)"_json;

	StringBuilder_c sRes;
	sRes += tJsonRes.dump().c_str();

	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.GetLength(), bHeadReply );
}

static bool GetIndexComplexFields ( const CSphString & sIndex, ComplexFields_t & dFields )
{
	auto tIndex ( GetServed ( sIndex ) );
	if ( !tIndex )
	{
		sphWarning ( "unknown kibana table %s", sIndex.cstr() );
		return false;
	}

	const CSphSchema & tSchema = RIdx_c( tIndex )->GetMatchSchema();
	for ( const CSphColumnInfo & tField : tSchema.GetFields() )
		AddComplexField ( tField.m_sName.cstr(), dFields );

	return true;
}

struct CompatInsert_t
{
	const CSphString & m_sBody;
	const CSphString & m_sIndex;
	const bool m_bReplace;
	const char * m_sId { nullptr };

	CompatInsert_t ( const CSphString & sBody, const CSphString & sIndex, bool bReplace )
		: m_sBody ( sBody ), m_sIndex ( sIndex ), m_bReplace ( bReplace )
	{}
};

static void ProcessInsertIntoIdx ( const CompatInsert_t & tIns, CSphVector<BYTE> & dResult )
{
	StringBuilder_c tSrc;
		tSrc.Appendf ( R"({"index" : "%s", )", tIns.m_sIndex.cstr() );

	if ( tIns.m_sId )
		tSrc.Appendf ( R"( "id" : %s, )" , tIns.m_sId );

	tSrc += R"( "doc" : )";
	tSrc += tIns.m_sBody.cstr();
	tSrc += "}";

	CSphString sError;
	bool bInserted = InsertDoc ( tSrc.cstr(), tIns.m_bReplace, sError );

	if ( !bInserted )
		sphWarning ( "doc '%s', error: %s", tIns.m_sId, sError.cstr() );

	if ( !bInserted )
	{
		ReportError ( sError.cstr(), "x_content_parse_exception", SPH_HTTP_STATUS_400, dResult, tIns.m_sIndex.cstr() );
	} else
	{
		DocID_t tLastDoc = 0;
		if ( session::LastIds().GetLength() )
			tLastDoc = session::LastIds().Last();

		nljson tRes;
		tRes["_index"] = tIns.m_sIndex.cstr();
		tRes["_type"] = "_doc";
		tRes["_id"] = tLastDoc;
		tRes["_version"] = 1;
		tRes["_seq_no"] = 0;
		tRes["_primary_term"] = 1;
		tRes["result"] = ( ( tIns.m_bReplace ) ? "created" : "updated" );
		tRes["_shards"] = R"( { "total": 1, "successful": 1, "failed": 0 } )"_json;

		CSphString sRes ( tRes.dump().c_str() );
		HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	}
}

static bool ProcessInsert ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	if ( tParser.GetBody().IsEmpty() )
	{
		ReportError ( "request body or source parameter is required", "parse_exception", SPH_HTTP_STATUS_400, dResult );
		return true;
	}

	CSphString sIndex;
	StrVec_t dIndexes = ExpandIndexes ( dUrlParts[0], sIndex );

	if ( sIndex.IsEmpty() )
	{
		ReportMissedIndex ( dUrlParts[0], dUrlParts, dResult );
		return true;
	}

	CompatInsert_t tIns ( tParser.GetBody(), sIndex, ( dUrlParts[1]=="_doc" ) );
	if ( dUrlParts.GetLength()>2 )
		tIns.m_sId = dUrlParts[2].cstr();

	if ( !IsKibanTable ( dIndexes ) )
	{
		 ProcessInsertIntoIdx ( tIns, dResult );
		 return true;
	}

	nljson tSrc = nljson::parse ( tParser.GetBody().cstr() );
	int iVersion = 1;

	CSphString sError;

	// check \ get document version vs _create \ _doc
	DocIdVer_t dIds;
	if ( !GetDocIds ( sIndex.cstr(), tIns.m_sId, dIds, sError ) )
	{
		sphWarning ( "doc '%s', error: %s", tIns.m_sId, sError.cstr() );
		return false;
	}
	if ( dIds.GetLength() )
	{
		if ( dIds.GetLength()!=1 )
		{
			sphWarning ( "multiple (%d) docs '%s' found", dIds.GetLength(), tIns.m_sId );
			return false;
		}

		iVersion = dIds[0].second + 1;
		if ( !tIns.m_bReplace )
		{
			CSphString sMsg;
			sMsg.SetSprintf ( "[%s]: version conflict, document already exists (current version [%d])", tIns.m_sId, iVersion );
			ReportError ( sMsg.cstr(), "version_conflict_engine_exception", SPH_HTTP_STATUS_409, dResult, sIndex.cstr() );
			return true;
		}
	}

	ComplexFields_t dFields;
	if ( !GetIndexComplexFields ( sIndex, dFields ) )
		return false;

	bool bInserted = InsertDoc ( sIndex, dFields, tSrc, tIns.m_bReplace, tIns.m_sId, iVersion, sError );
	
	if ( !bInserted )
		sphWarning ( "doc '%s', error: %s", tIns.m_sId, sError.cstr() );

	if ( !bInserted )
	{
		CSphString sMsg;
		sMsg.SetSprintf ( "[%s]: version conflict, document already exists (current version [%d])", tIns.m_sId, iVersion );
		ReportError ( sMsg.cstr(), "version_conflict_engine_exception", SPH_HTTP_STATUS_409, dResult, sIndex.cstr() );
	} else
	{
		nljson tRes;
		tRes["_index"] = sIndex.cstr();
		tRes["_type"] = "_doc";
		tRes["_id"] = tIns.m_sId;
		tRes["_version"] = iVersion;
		tRes["_seq_no"] = 0;
		tRes["_primary_term"] = 1;
		tRes["result"] = ( ( dUrlParts[1]=="_create" || iVersion==1 ) ? "created" : "updated" );
		tRes["_shards"] = R"( { "total": 1, "successful": 1, "failed": 0 } )"_json;

		CSphString sRes ( tRes.dump().c_str() );
		HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	}

	return true;
}

static bool ProcessDeleteDoc ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	CSphString sIndex;
	ExpandIndexes ( dUrlParts[0], sIndex );
	const CSphString & sId = dUrlParts[2];

	if ( sIndex.IsEmpty() )
	{
		ReportMissedIndex ( dUrlParts[0], dUrlParts, dResult );
		return true;
	}

	// get document version vs _create
	CSphString sError;
	int iVersion = 1;
	DocIdVer_t dIds;
	if ( !GetDocIds ( sIndex.cstr(), sId.cstr(), dIds, sError ) )
	{
		sphWarning ( "doc '%s', error: %s", sId.cstr(), sError.cstr() );
		return false;
	}

	if ( dIds.GetLength() )
	{
		if ( dIds.GetLength()!=1 )
		{
			sphWarning ( "multiple (%d) docs '%s' found", dIds.GetLength(), sId.cstr() );
			return false;
		}

		iVersion = dIds[0].second;

		SqlStmt_t tStmt;
		tStmt.m_sIndex = sIndex;
		tStmt.m_tQuery.m_sIndexes = sIndex;
		CSphFilterSettings & tFilter = tStmt.m_tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = "id";
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( GetDocID ( sId.cstr() ) );
		tFilter.m_bExclude = false;

		std::unique_ptr<StmtErrorReporter_i> pReporter ( CreateHttpErrorReporter() );
		sphHandleMysqlDelete ( *pReporter.get(), tStmt, Str_t() );

		if ( pReporter->IsError() )
		{
			sphWarning ( "doc '%s', error: %s", sId.cstr(), pReporter->GetError() );
			ReportError ( "request body or source parameter is required", "parse_exception", SPH_HTTP_STATUS_400, dResult );
			return true;
		}
	}

	nljson tRes;
	tRes["_index"] = sIndex.cstr();
	tRes["_type"] = "_doc";
	tRes["_id"] = sId.cstr();
	tRes["_version"] = iVersion;
	tRes["_seq_no"] = 0;
	tRes["_primary_term"] = 1;
	tRes["result"] = ( dIds.GetLength() ? "deleted" : "not_found" );
	tRes["_shards"] = R"( { "total": 1, "successful": 1, "failed": 0 } )"_json;

	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );

	return true;
}

typedef bool ( *fnScript ) ( const nljson & tParams, int & iVersion, nljson & tDoc, CSphString & sError );

static bool fnScriptUpdate1 ( const nljson & tParams, int & iVersion, nljson & tDoc, CSphString & sError )
{
	nljson::json_pointer tParamType ( "/type" );
	nljson::json_pointer tParamCntName ( "/counterFieldName" );
	nljson::json_pointer tParamCnt ( "/count" );
	nljson::json_pointer tParamTime ( "/time" );
	if ( !tParams.contains ( tParamType ) || !tParams.contains ( tParamCntName ) || !tParams.contains ( tParamCnt ) || !tParams.contains ( tParamTime ) )
	{
		sError.SetSprintf ( "missed parameter: %s=%d,  %s=%d,  %s=%d,  %s=%d",
			tParamType.to_string().c_str(), (int)tParams.contains ( tParamType ),
			tParamCntName.to_string().c_str(), (int)tParams.contains ( tParamCntName ), 
			tParamCnt.to_string().c_str(), (int)tParams.contains ( tParamCnt ), 
			tParamTime.to_string().c_str(), (int)tParams.contains ( tParamTime ) );
		return false;
	}

	std::string sValType = tParams[tParamType];
	std::string sValCntName = tParams[tParamCntName];
	int iValCnt = tParams[tParamCnt];

	if ( !tDoc.contains ( sValType ) ||! tDoc[sValType].contains ( sValCntName ) )
	{
		tDoc[sValType][sValCntName] = iValCnt;
	} else
	{
		int iPrevCnt = tDoc[sValType][sValCntName];
		tDoc[sValType][sValCntName] = iPrevCnt + iValCnt;
	}

	tDoc["updated_at"] = tParams[tParamTime];

	return true;
}

static std::pair < const char *, fnScript >  g_sScripts[] = {
	{ R"(
if (ctx._source[params.type][params.counterFieldName] == null)
{
	ctx._source[params.type][params.counterFieldName] = params.count;
} else {
	ctx._source[params.type][params.counterFieldName] += params.count;
}
ctx._source.updated_at = params.time;
		)", fnScriptUpdate1 },
	{ nullptr, nullptr }
};

static SmallStringHash_T< fnScript > g_hScripts;

static void StripSpaces ( char * sBuf )
{
	char * sDst = sBuf;
	char * sSrc = sBuf;
	while ( *sSrc )
	{
		if ( sphIsSpace ( *sSrc ) )
			sSrc++;
		else
			*sDst++ = *sSrc++;
	}

	*sDst = '\0';
}

void CreateScripts()
{
	int iScript = 0;
	for ( const auto & tItem : g_sScripts )
	{
		if ( !tItem.first )
			break;

		CSphString sScript = tItem.first;
		StripSpaces ( const_cast<char *>( sScript.cstr() ) );
		
		if ( !g_hScripts.Add ( tItem.second, sScript ) )
			sphWarning ( "duplicate script %d found %s", iScript, tItem.first );

		iScript++;
	}
}

static void ReportUpated ( const char * sId, int iVersion, const char * sIndex, const char * sOperation, const nljson & tDoc, CSphVector<BYTE> & dResult )
{
	nljson tRes;
	tRes["_index"] = sIndex;
	tRes["_type"] = "_doc";
	tRes["_id"] = sId;
	tRes["_version"] = iVersion;
	tRes["result"] = sOperation;
	tRes["_shards"] = R"( { "total": 1, "successful": 1, "failed": 0 } )"_json;
	tRes["_seq_no"] = 0;
	tRes["_primary_term"] = 1;

	nljson tGet;
	tGet["_seq_no"] = 0;
	tGet["_primary_term"] = 1;
	tGet["found"] = true;
	tGet["_source"] = tDoc;

	tRes["get"] = tGet;

	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
}

static void ReportMissedScript ( const CSphString & sIndex,  const CSphString & sBody, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	StringBuilder_c sLocation ( "/", "/", "/" );
	dUrlParts.Apply ( [&sLocation] ( const CSphString & sItem ) { sLocation << sItem; } );
	nljson tReq = nljson::parse ( sBody.cstr() );
	sphWarning ( "missed script '%s' at '%s' body '%s'", sIndex.cstr(), sLocation.cstr(), tReq.dump().c_str() );

	ReportError ( "failed to execute script", "illegal_argument_exception", SPH_HTTP_STATUS_400, dResult );
}


static bool ProcessUpdateDoc ( const HttpRequestParsed_t & tParser, const StrVec_t & dUrlParts, CSphVector<BYTE> & dResult )
{
	CSphString sIndex;
	ExpandIndexes ( dUrlParts[0], sIndex );
	const CSphString & sId = dUrlParts[2];

	if ( sIndex.IsEmpty() )
	{
		ReportMissedIndex ( dUrlParts[0], dUrlParts, dResult );
		return true;
	}

	nljson tUpd = nljson::parse ( tParser.GetBody().cstr() );

	bool bHasScript = tUpd.contains ( "script" );

	nljson::json_pointer tScriptName ( "/script/source" );
	nljson::json_pointer tScriptParamsName ( "/script/params" );
	if ( bHasScript &&( !tUpd.contains ( tScriptName ) || !tUpd.contains ( tScriptParamsName )  ) )
	{
		ReportMissedScript ( sIndex, tParser.GetBody(), dUrlParts, dResult );
		return true;
	}

	fnScript * pUpdateScript = nullptr;
	if ( bHasScript )
	{
		// compact script by removing space characters and get script function
		CSphString sScript = tUpd[tScriptName].get<std::string>().c_str();
		StripSpaces ( const_cast<char *>( sScript.cstr() ) );
		pUpdateScript = g_hScripts ( sScript );
		if ( !pUpdateScript )
		{
			ReportMissedScript ( sIndex, tParser.GetBody(), dUrlParts, dResult );
			return true;
		}
	}

	CSphString sError;
	DocIdVer_t dIds;
	if ( !GetDocIds ( sIndex.cstr(), sId.cstr(), dIds, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	// validate document source for insert new document
	nljson::json_pointer tSrcName ( "/upsert" );
	if ( !dIds.GetLength() && !tUpd.contains ( tSrcName ) )
	{
		sphWarning ( "doc '%s' source '%s' missed", sId.cstr(), tSrcName.to_string().c_str() );
		CSphString sMsg;
		sMsg.SetSprintf ( "[_doc][%s]: document missing", sId.cstr() );
		ReportError ( sMsg.cstr(), "document_missing_exception", SPH_HTTP_STATUS_404, dResult );
		return true;
	}

	int iVersion = 1;

	ComplexFields_t dComplexFields;
	if ( !GetIndexComplexFields ( sIndex, dComplexFields ) )
		return false;

	// create new document
	if ( !dIds.GetLength() )
	{
		const nljson & tSrc = tUpd[tSrcName];
		if ( !InsertDoc ( sIndex, dComplexFields, tSrc, false, sId.cstr(), iVersion, sError ) )
		{
			sphWarning ( "doc '%s', error: %s", sId.cstr(), sError.cstr() );

			CSphString sMsg;
			sMsg.SetSprintf ( "[%s]: version conflict, document already exists (current version [%d])", sId.cstr(), iVersion );
			ReportError ( sMsg.cstr(), "version_conflict_engine_exception", SPH_HTTP_STATUS_409, dResult, sIndex.cstr() );
			return true;
		} else
		{
			ReportUpated ( sId.cstr(), iVersion, sIndex.cstr(), "created", tSrc, dResult );
			return true;
		}
	}

	if ( dIds.GetLength()!=1 )
	{
		sphWarning ( "multiple %d documents found for '%s'", dIds.GetLength(), sId.cstr() );
		ReportError ( "failed to execute script", "illegal_argument_exception", SPH_HTTP_STATUS_400, dResult );
		return false;
	}
	if ( dIds[0].first!=sId )
	{
		sphWarning ( "wrong document found '%s' for '%s'", dIds[0].first.cstr(), sId.cstr() );
		ReportError ( "failed to execute script", "illegal_argument_exception", SPH_HTTP_STATUS_400, dResult );
		return false;
	}

	auto tServed ( GetServed ( sIndex ) );
	if ( !tServed )
	{
		sphWarning ( "unknown kibana table %s", sIndex.cstr() );
		return false;
	}

	RIdx_c pIndex ( tServed );

	DocstoreSession_c tSession;
	pIndex->CreateReader ( tSession.GetUID() );
	CSphVector<BYTE> dRawDoc;

	//const auto & tDocId = dIds[0];
	iVersion = dIds[0].second + 1;
	if ( !GetIndexDoc ( pIndex, sId.cstr(), tSession.GetUID(), dRawDoc, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	nljson tSrc = nljson::parse ( dRawDoc.Begin() );

	// update raw source document
	if ( bHasScript )
	{
		assert ( pUpdateScript );
		if ( !((*pUpdateScript)( tUpd[tScriptParamsName], iVersion, tSrc, sError ) ) )
		{
			sphWarning ( "%s", sError.cstr() );
			ReportError ( "failed to execute script", "illegal_argument_exception", SPH_HTTP_STATUS_400, dResult );
			return true;
		}
	} else if ( tUpd.contains ( "doc" ) )
	{
		const nljson & tDocUpd = tUpd["doc"];
		tSrc.update ( tDocUpd );
	} else
	{
		sphWarning ( "doc '%s' source 'doc' missed", sId.cstr() );
		CSphString sMsg;
		sMsg.SetSprintf ( "[_doc][%s]: document missing", sId.cstr() );
		ReportError ( sMsg.cstr(), "document_missing_exception", SPH_HTTP_STATUS_404, dResult );
		return true;
	}

	// reinsert updated document
	if ( !InsertDoc ( sIndex, dComplexFields, tSrc, true, sId.cstr(), iVersion, sError ) )
	{
			sphWarning ( "doc '%s', error: %s", sId.cstr(), sError.cstr() );

			CSphString sMsg;
			sMsg.SetSprintf ( "[%s]: version conflict, document already exists (current version [%d])", sId.cstr(), iVersion );
			ReportError ( sMsg.cstr(), "version_conflict_engine_exception", SPH_HTTP_STATUS_409, dResult, sIndex.cstr() );
			return true;
	}


	ReportUpated ( sId.cstr(), iVersion, sIndex.cstr(), "updated", tSrc, dResult );
	return true;
}

static void DropTable ( const CSphString & sName )
{
	CSphString sError;
	if ( !DropIndexInt ( sName, true, sError ) )
		sphWarning ( "%s", sError.cstr() );

	{
		CSphVector<CSphString> dIndexes;
		ScWL_t tLock ( g_tLockAlias );
		for ( auto tIt : g_hAlias )
		{
			const CSphString & sAliasIndex = tIt.second;
			if ( sName==sAliasIndex )
				dIndexes.Add ( tIt.first );
		}

		for ( const auto & sIndex : dIndexes )
			g_hAlias.Delete ( sIndex );
	}

	{
		ScWL_t tLockTbl ( g_tLockKbnTable );
		g_tKbnTable.erase ( sName.cstr() );
	}
}

static bool ProcessCreateTable ( const HttpRequestParsed_t & tParser, const CSphString & sName, CSphVector<BYTE> & dResult )
{
	bool bDropExistTable = false;
	CSphString sError;
	{
		auto tIndex ( GetServed ( sName ) );
		if ( tIndex )
		{
			ScRL_t tLockTbl ( g_tLockKbnTable );
			bDropExistTable = !g_tKbnTable.contains ( sName.cstr() );
			if ( !bDropExistTable )
			{
				sError.SetSprintf ( "index [%s] already exists", sName.cstr() );
				ReportError ( sError.cstr(), "resource_already_exists_exception", SPH_HTTP_STATUS_400, dResult, sName.cstr() );
				return true;
			}
		}
	}

	nljson tTbl = nljson::parse ( tParser.GetBody().cstr() );

	// direct create index path (wo template)
	if ( !tTbl.contains( "mappings" ) )
		return false;

	// need to delete table loaded by manticore but without meta at json file
	DropTable ( sName );

	ComplexFields_t dFields;
	CreateTableSettings_t tOpts;
	CreateKbnTable ( tOpts, tTbl, dFields );

	StrVec_t dWarnings;
	if ( !CreateNewIndexConfigless ( sName, tOpts, dWarnings, sError ) )
		sphWarning ( "%s", sError.cstr() );

	for ( const CSphString & sWarn : dWarnings )
		sphWarning ( "%s", sWarn.cstr() );

	{
		ScWL_t tLockTbl ( g_tLockKbnTable );
		g_tKbnTable[sName.cstr()] = tTbl;
	}

	sphInfo ( "created table '%s'", sName.cstr() );
	CreateAliases(); // FIXME!!! create only this table alias

	nljson tRes = R"(
{
    "acknowledged": true, 
    "shards_acknowledged": true
})"_json;
	tRes["index"] = sName.cstr();

	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessDeleteTable ( const CSphString & sName, CSphVector<BYTE> & dResult )
{
	CSphString sError;
	{
		auto tIndex ( GetServed ( sName ) );
		if ( !tIndex )
		{
			sError.SetSprintf ( "no such index [%s]", sName.cstr() );
			ReportError ( sError.cstr(), "index_not_found_exception", SPH_HTTP_STATUS_404, dResult, sName.cstr() );
			return true;
		}
	}

	DropTable ( sName );

	nljson tRes = R"({ "acknowledged": true })"_json;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessAliasGet ( const StrVec_t & dUrlParts, bool bHeadReply, CSphVector<BYTE> & dResult )
{
	CSphString sIndex;
	StrVec_t dFilters;
	if ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_alias" )
	{
		sIndex = dUrlParts[0];
		if ( dUrlParts.GetLength()>=3 )
			sphSplit ( dFilters, dUrlParts[2].cstr(), "," );
	} else if ( dUrlParts.GetLength()>=2 && dUrlParts[0]=="_alias" )
	{
		sphSplit ( dFilters, dUrlParts[1].cstr(), "," );
	}
	dFilters.Apply ( [] ( CSphString & sItem ) {
		if ( sItem=="_all" )
			sItem = "*";
		});

	nljson tRes = R"({})"_json;
	{
		ScRL_t tLock ( g_tLockAlias );
		for ( const auto & tIt : g_hAlias )
		{
			const CSphString & sAliasName = tIt.first;
			const CSphString & sAliasIndex = tIt.second;

			if ( !sIndex.IsEmpty() && !sphWildcardMatch ( sAliasIndex.cstr(), sIndex.cstr() ) )
				continue;

			if ( dFilters.GetLength() && !dFilters.any_of ( [&] ( const CSphString & sFilter ) { return sphWildcardMatch ( sAliasName.cstr(), sFilter.cstr() ); } ) )
				continue;

			if ( !tRes.contains ( sAliasIndex.cstr() ) )
				tRes[sAliasIndex.cstr()] = R"({ "aliases": {} })"_json;

			tRes[sAliasIndex.cstr()]["aliases"][sAliasName.cstr()] = R"({})"_json;
		}
	}

	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), bHeadReply );
	return true;
}

// FIXME!!! add support of these forms too
// PUT /<index>/_alias/<alias>
// POST /<index>/_alias/<alias>
// PUT /<index>/_aliases/<alias>
// POST /<index>/_aliases/<alias>

static bool ProcessAliasSet ( const HttpRequestParsed_t & tParser, CSphVector<BYTE> & dResult )
{
	nljson tAliases = nljson::parse ( tParser.GetBody().cstr() );
	if ( tAliases.contains( "actions" ) )
	{
		// FIXME!!! add support of aliases and indices options
		nljson::json_pointer tIndexName ( "/index" );
		nljson::json_pointer tAliasName ( "/alias" );
		for ( const auto & tIt : tAliases["actions"].items() )
		{
			const auto & tItem = tIt.value().cbegin();
			if ( !tItem.value().contains ( tIndexName ) || !tItem.value().contains ( tAliasName )  )
			{
				ReportError ( "[aliases] failed to parse field [actions]", "x_content_parse_exception", SPH_HTTP_STATUS_400, dResult );
				return true;
			}

			CSphString sIndex = tItem.value()[tIndexName].get<std::string>().c_str();
			CSphString sAlias = tItem.value()[tAliasName].get<std::string>().c_str();

			{
				auto tIndex ( GetServed ( sIndex ) );
				if ( !tIndex )
				{
					ReportMissedIndex ( sIndex, StrVec_t(), dResult );
					return true;
				}
			}


			if ( tItem.key()=="add" )
			{
				{
					ScWL_t tLock ( g_tLockAlias );
					g_hAlias.Add ( sIndex, sAlias );
				}
				{
					ScWL_t tLock ( g_tLockKbnTable );
					if ( g_tKbnTable.contains ( sIndex.cstr() ) )
					{
						nljson & tIdx = g_tKbnTable[sIndex.cstr()];
						if ( !tIdx.contains ( "aliases" ) )
							tIdx["aliases"] = R"({})"_json;

						tIdx["aliases"][sAlias.cstr()] = R"({})"_json;
					}
				}
			} else if ( tItem.key()=="remove" )
			{
				{
					ScWL_t tLock ( g_tLockAlias );
					if ( !g_hAlias.Delete ( sAlias ) )
					{
						CSphString sError;
						sError.SetSprintf ( "aliases [%s] missing", sAlias.cstr() );
						ReportError ( sError.cstr(), "aliases_not_found_exception", SPH_HTTP_STATUS_404, dResult );
						return true;
					}
				}

				{
					ScWL_t tLock ( g_tLockKbnTable );
					if ( g_tKbnTable.contains ( sIndex.cstr() ) )
					{
						nljson & tIdx = g_tKbnTable[sIndex.cstr()];
						if ( tIdx.contains ( "aliases" ) )
							tIdx["aliases"].erase ( sAlias.cstr() );
					}
				}
			} else if ( tItem.key()=="remove_index" )
			{
				DropTable ( sIndex );
			} else
			{
				ReportError ( "[aliases] failed to parse field [actions]", "x_content_parse_exception", SPH_HTTP_STATUS_400, dResult );
				return true;
			}
		}
	}

	nljson tRes = R"({ "acknowledged": true })"_json;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessRefresh ( const CSphString * pName, CSphVector<BYTE> & dResult )
{
	CSphString sError;
	if ( pName )
	{
		auto tIndex ( GetServed ( *pName ) );
		if ( !tIndex )
		{
			ReportMissedIndex ( *pName, StrVec_t(), dResult );
			return true;
		}
	}

	nljson tRes = R"({ "_shards": { "total": 1, "successful": 1, "failed": 0 } })"_json;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessCCR ( bool bHeadReply, CSphVector<BYTE> & dResult )
{
	nljson tRes = R"({ "follower_indices": [] })"_json;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), bHeadReply );
	return true;
}

static bool ProcessILM ( bool bHeadReply, CSphVector<BYTE> & dResult )
{
	nljson tItems = R"({})"_json;

	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		nljson tItem = R"({"managed": false})"_json;
		tItem["index"] = tIt.first.cstr();

		tItems[tIt.first.cstr()] = tItem;
	}

	nljson tRes = R"({})"_json;
	tRes["indices"] = tItems;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), bHeadReply );
	return true;
}

static void EmptyReply ( bool bHeadReply, CSphVector<BYTE> & dResult )
{
	nljson tRes = R"({})"_json;
	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReplyHead ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), bHeadReply );
}

static nljson GetFieldDesc ( const CSphColumnInfo & tCol, bool bField )
{
	nljson tField = R"({"searchable":false, "aggregatable": false})"_json;
	if ( bField )
	{
		tField["type"] = "text";
		tField["searchable"] = true;
	} else if ( tCol.m_eAttrType!=SPH_ATTR_NONE )
	{
		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_TIMESTAMP:
			tField["type"] = "date";
			tField["aggregatable"] = true;
			tField["searchable"] = true;
		break;

		case SPH_ATTR_INTEGER:
		case SPH_ATTR_BOOL:
			tField["type"] = "integer";
			tField["aggregatable"] = true;
			tField["searchable"] = true;
		break;

		case SPH_ATTR_FLOAT:
			tField["type"] = "float";
			tField["searchable"] = true;
		break;

		case SPH_ATTR_BIGINT:
			tField["type"] = "long";
			tField["aggregatable"] = true;
			tField["searchable"] = true;
		break;

		case SPH_ATTR_STRING:
			tField["type"] = "keyword";
			tField["aggregatable"] = true;
			tField["searchable"] = true;
		break;

		case SPH_ATTR_JSON:
			tField["type"] = "object";
			tField["searchable"] = true;
		break;

		default:
			break;
		}
	}

	return tField;
}

static bool CheckFieldDesc ( const nljson & tFields, const char * sColName, const nljson & tDesc, const CSphString & sIndex, StringBuilder_c & sError )
{
	if ( !tDesc.contains ( "type" ) )
	{
		sError.Appendf ( "index '%s' has unmapped column '%s'", sIndex.cstr(), sColName );
		return false;
	}

	if ( !tFields.contains ( sColName ) )
		return true;

	assert ( tDesc.contains ( "type" ) );
	if ( tFields[sColName].contains ( tDesc["type"] ) )
		return true;

	const nljson & tField = tFields[sColName];
	sError.Appendf ( "'%s' already has type '%s' but index '%s' type is '%s'", sColName, tField.cbegin().key().c_str(), sIndex.cstr(), tDesc["type"].get<std::string>().c_str() );

	return false;
}

static void AddSpecialColumns ( nljson & tFields )
{
	tFields["_index"] = R"( { "_index": { "type": "_index", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_feature"] = R"( { "_feature": { "type": "_feature", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_ignored"] = R"( { "_ignored": { "type": "_ignored", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_version"] = R"( { "_version": { "type": "_version", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_type"] = R"( { "_type": { "type": "_type", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_seq_no"] = R"( { "_seq_no": { "type": "_seq_no", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_field_names"] = R"( { "_field_names": { "type": "_field_names", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_source"] = R"( { "_source": { "type": "_source", "searchable": false, "aggregatable": false } } )"_json;
	tFields["_id"] = R"( { "_id": { "type": "_id", "searchable": false, "aggregatable": false } } )"_json;
}

static const char * FixupAtName ( const char * sName, StringBuilder_c & tTmpName )
{
	if ( sName[0]!='_' )
		return sName;

	tTmpName.Clear();
	tTmpName.Sprintf ( "@%s", sName+1 );
	return tTmpName.cstr();
}

// FIXME!!! add support of Elastic \ Kibana tables
static bool ProcessFields ( const CSphString & sIndex, CSphVector<BYTE> & dResult )
{
	nljson tRes = R"({"indices":[], "fields":{}})"_json;

	StringBuilder_c tTmpName;
	StringBuilder_c sError ( "," );
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		if ( !sIndex.IsEmpty() && !sphWildcardMatch ( tIt.first.cstr(), sIndex.cstr() ) )
			continue;

		RIdx_c tIdx ( tIt.second );

		tRes["indices"].push_back ( tIt.first.cstr() );

		const CSphSchema & tSchema = tIdx->GetMatchSchema();
		for ( const CSphColumnInfo & tCol : tSchema.GetFields() )
		{
			// field-string types will be processed at attributes
			if ( tSchema.GetAttr ( tCol.m_sName.cstr() ) )
				continue;

			const char * sColName = tCol.m_sName.cstr();

			nljson tField = GetFieldDesc ( tCol, true );
			if ( !CheckFieldDesc ( tRes["fields"], sColName, tField, sIndex, sError ) )
				continue;

			tRes["fields"][sColName][tField["type"].get<std::string>()] = tField;
		}

		for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr ( i );

			// skip special only manticore related attributes and tokecounts types
			if ( tCol.m_sName=="@version" || tCol.m_sName==sphGetBlobLocatorName() || tCol.m_eAttrType==SPH_ATTR_TOKENCOUNT )
				continue;

			bool bField = ( tSchema.GetField ( tCol.m_sName.cstr() ) );

			const char * sColName = FixupAtName ( tCol.m_sName.cstr(), tTmpName );

			nljson tField = GetFieldDesc ( tCol, bField );
			if ( !CheckFieldDesc ( tRes["fields"], sColName, tField, sIndex, sError ) )
				continue;

			tRes["fields"][sColName][tField["type"].get<std::string>()] = tField;
		}
	}

	// add special only elastic related attributes
	AddSpecialColumns ( tRes["fields"] );

	if ( !sError.IsEmpty() )
		sphWarning ( "%s", sError.cstr() );

	CSphString sRes ( tRes.dump().c_str() );
	HttpBuildReply ( dResult, SPH_HTTP_STATUS_200, sRes.cstr(), sRes.Length(), false );
	return true;
}

static bool ProcessHttp ( const HttpRequestParsed_t & tParser, CSphVector<BYTE> & dResult )
{
	const HttpOptionsHash_t & hOpts = tParser.GetOptions();
	const CSphString & sEndpoint = hOpts["endpoint"];
	StrVec_t dUrlParts = SplitURL ( sEndpoint );

	if ( dUrlParts.GetLength() && dUrlParts.Last().Ends ( "_msearch" ) )
		return ProcessMSearch ( tParser, dUrlParts, dResult );

	if ( tParser.GetRequestType()==HTTP_GET || tParser.GetRequestType()==HTTP_HEAD )
	{
		const bool bHeadReply = ( tParser.GetRequestType()==HTTP_HEAD );
		if ( !dUrlParts.GetLength() )
		{
			ProcessEmptyHead ( bHeadReply, dResult );
			return true;
		}

		if ( dUrlParts.GetLength()>1 && dUrlParts[1].Begins( "_" ) &&
			dUrlParts[1]=="_doc" &&
			( dUrlParts[0]==g_sKbnTableName || dUrlParts[0]==g_sKbnTableAlias ) &&
			ProcessKbnTableDoc ( dUrlParts, bHeadReply, dResult ) )
				return true;

		if ( dUrlParts.GetLength()>=2 && dUrlParts[0]=="_cat" && ProcessCat ( dUrlParts, hOpts, tParser.GetFullURL(), bHeadReply, dResult ) )
			return true;

		if ( ( dUrlParts.GetLength() && dUrlParts[0]=="_alias" ) || ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_alias" ) )
		{
			ProcessAliasGet ( dUrlParts, bHeadReply, dResult );
			return true;
		}

		if ( ( dUrlParts.GetLength() && dUrlParts[0]=="_rollup" ) || ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_rollup" )
			|| ( dUrlParts.GetLength() && dUrlParts[0]=="_ingest" ) )
		{
			EmptyReply ( bHeadReply, dResult );
			return true;
		}

		if ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_ilm" )
		{
			ProcessILM ( bHeadReply, dResult );
			return true;
		}

		if ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_ccr" )
		{
			ProcessCCR ( bHeadReply, dResult );
			return true;
		}

		if ( dUrlParts.GetLength() && ProcessKbnTableGet ( dUrlParts, hOpts ( "filter_path" ), bHeadReply, dResult ) )
			return true;
	}

	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>1 && dUrlParts[1]=="_search" && ProcessSearch ( tParser, dUrlParts, dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>1 && dUrlParts[1]=="_count" && ProcessCount ( tParser, dUrlParts, dResult ) )
		return true;

	if ( ( tParser.GetRequestType()==HTTP_POST || tParser.GetRequestType()==HTTP_PUT )
		&& dUrlParts.GetLength()>1 && ( dUrlParts[1]=="_doc" || dUrlParts[1]=="_create" )
		&& ProcessInsert ( tParser, dUrlParts, dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>0 && dUrlParts[0]=="_mget" && ProcessKbnTableMGet ( tParser, dUrlParts, dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_PUT )
	{
		if ( dUrlParts.GetLength()>1 && ( dUrlParts[0]=="_template" || dUrlParts[0]=="_monitoring" ) )
		{
			ProcessPutTemplate ( tParser, dUrlParts, dResult );
			return true;
		}

		if ( dUrlParts.GetLength() && dUrlParts[0]=="_monitoring" )
		{
			ProcessIgnored ( dResult );
			return true;
		}

		if ( dUrlParts.GetLength() && ProcessCreateTable ( tParser, dUrlParts[0], dResult ) )
			return true;
	}

	if ( tParser.GetRequestType()==HTTP_DELETE && dUrlParts.GetLength()>2 && dUrlParts[1]=="_doc" 
		&& ProcessDeleteDoc ( tParser, dUrlParts, dResult ) )
		return true;


	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>2 && dUrlParts[1]=="_update" 
		&& ProcessUpdateDoc ( tParser, dUrlParts, dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_DELETE && dUrlParts.GetLength()==1 && ProcessDeleteTable ( dUrlParts[0], dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>0 && dUrlParts[0]=="_aliases" && ProcessAliasSet ( tParser, dResult ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_POST &&
		( ( dUrlParts.GetLength()>=2 && dUrlParts[1]=="_refresh" && ProcessRefresh ( dUrlParts.Begin(), dResult ) ) ||
		( dUrlParts.GetLength()>=1 && dUrlParts[0]=="_refresh" && ProcessRefresh ( nullptr, dResult ) ) ) )
		return true;

	if ( tParser.GetRequestType()==HTTP_POST && dUrlParts.GetLength()>=2 && dUrlParts[1]=="_field_caps" && ProcessFields ( dUrlParts[0], dResult ) )
		return true;

	if ( ( tParser.GetRequestType()==HTTP_POST || tParser.GetRequestType()==HTTP_PUT ) && dUrlParts.GetLength() && dUrlParts[0]=="_monitoring" )
	{
		ProcessIgnored ( dResult );
		return true;
	}

	return false;
}

CSphMutex g_tReqStatLock;
SmallStringHash_T<int> g_tReqStat;

void ProcessCompatHttp ( const HttpRequestParsed_t & tParser, CSphVector<BYTE> & dResult )
{
	CSphString sRefBody;
	bool bDumpHttp = false;
	if ( !g_sLogHttpFilter.IsEmpty() && sphWildcardMatch ( tParser.GetOptions()["endpoint"].cstr(), g_sLogHttpFilter.cstr() ) )
	{
		bDumpHttp = true;
		sRefBody = tParser.GetBody();
	}

	if ( !ProcessHttp ( tParser, dResult ) )
	{
		CSphString sError;
		DumpHttp ( tParser.GetRequestType(), tParser.GetFullURL(), tParser.GetBody() );
		sError.SetSprintf ( "%s - unsupported endpoint", tParser.GetFullURL().cstr() );
		sphFatalLog ( "%s", sError.cstr() );
		HttpErrorReply ( dResult, SPH_HTTP_STATUS_501, sError.cstr() );
	} else if ( bDumpHttp )
	{
		DumpHttp ( tParser.GetRequestType(), tParser.GetFullURL(), sRefBody, dResult );
	}

	ScopedMutex_t tLock ( g_tReqStatLock );
	int * pCounter = g_tReqStat ( tParser.GetFullURL() );
	if ( pCounter )
	{
		*pCounter = (*pCounter) + 1;
	} else
	{
		g_tReqStat.Add ( 1, tParser.GetFullURL() );
	}
}

void SetLogHttpFilter ( const CSphString & sVal )
{
	if ( sVal=="dumpq" )
	{
		StringBuilder_c tOut ( "\n" );

		ScopedMutex_t tLock ( g_tReqStatLock );
		for ( const auto & tIt : g_tReqStat )
			tOut.Appendf ( "%s, total = %d", tIt.first.cstr(), tIt.second );
		tOut.Appendf ( "total %d\n", g_tReqStat.GetLength() );

		sphSeek ( GetLogFD(), 0, SEEK_END );
		sphWrite ( GetLogFD(), tOut.cstr(), tOut.GetLength() );

		return;
	}
	g_sLogHttpFilter = sVal;
}

static void DropKbnTables()
{
	CSphVector<CSphString> dIdx;
	{
		ScRL_t tLock ( g_tLockKbnTable );
		for ( auto & tTbl : g_tKbnTable.items() )
			dIdx.Add ( tTbl.key().c_str() );
	}
	{
		ScWL_t tLock ( g_tLockKbnTable );
		g_tKbnTable = "{}"_json;
	}
	{
		ScWL_t tLock ( g_tLockAlias );
		g_hAlias.Reset();
	}

	// look for local indexes with kibana names
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		for ( const CSphString & sName : g_dKbnTablesNames )
		{
			if ( tIt.first.Begins ( sName.cstr() ) )
			{
				dIdx.Add ( tIt.first );
				break;
			}
		}
	}

	dIdx.Uniq();

	CSphString sError;
	for ( const CSphString & sName : dIdx )
	{
		if ( !DropIndexInt ( sName, true, sError ) )
			sphWarning ( "%s", sError.cstr() );
	}

	if ( dIdx.GetLength() )
		sphWarning ( "dropped %d system tables", dIdx.GetLength() );
}

void SetLogManagement ( bool bEnable )
{
	ScRL_t tLock ( g_tLockSettings );
	g_tSettings["enabled"] = bEnable;

	DropKbnTables();

	g_bEnabled = bEnable;
	if ( g_bEnabled )
	{
		nljson tSys = GetSystemTable();
		g_tKbnTable.update ( tSys );
		CreateKbnIndexes();
		CreateAliases();
	}
}

bool IsLogManagementEnabled ()
{
	return g_bEnabled;
}