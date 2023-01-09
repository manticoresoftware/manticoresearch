//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "source_pgsql.h"
#include <libpq-fe.h>
#include <config_indexer.h>

/////////////////////////////////////////////////////////////////////////////
// PGSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#ifndef POSTGRESQL_LIB
#define POSTGRESQL_LIB nullptr
#endif
static const char * GET_POSTGRESQL_LIB ()
{
	const char * szEnv = getenv ( "POSTGRESQL_LIB" );
	if ( szEnv )
		return szEnv;
	return POSTGRESQL_LIB;
}

#if DL_POSTGRESQL

	static decltype (&PQgetvalue) sph_PQgetvalue = nullptr;
	static decltype (&PQgetlength) sph_PQgetlength = nullptr;
	static decltype (&PQclear) sph_PQclear = nullptr;
	static decltype (&PQsetdbLogin) sph_PQsetdbLogin = nullptr;
	static decltype (&PQstatus) sph_PQstatus = nullptr;
	static decltype (&PQsetClientEncoding) sph_PQsetClientEncoding = nullptr;
	static decltype (&PQexec) sph_PQexec = nullptr;
	static decltype (&PQresultStatus) sph_PQresultStatus = nullptr;
	static decltype (&PQntuples) sph_PQntuples = nullptr;
	static decltype (&PQfname) sph_PQfname = nullptr;
	static decltype (&PQnfields) sph_PQnfields = nullptr;
	static decltype (&PQfinish) sph_PQfinish = nullptr;
	static decltype (&PQerrorMessage) sph_PQerrorMessage = nullptr;
	static decltype (&PQunescapeBytea) sph_PQunescapeBytea = nullptr;
	static decltype (&PQfreemem) sph_PQfreemem = nullptr;

	static bool InitDynamicPosgresql ()
	{
		const char * sFuncs[] = {"PQgetvalue", "PQgetlength", "PQclear",
				"PQsetdbLogin", "PQstatus", "PQsetClientEncoding", "PQexec",
				"PQresultStatus", "PQntuples", "PQfname", "PQnfields",
				"PQfinish", "PQerrorMessage", "PQunescapeBytea", "PQfreemem" };

		void ** pFuncs[] = {(void**)&sph_PQgetvalue, (void**)&sph_PQgetlength, (void**)&sph_PQclear,
				(void**)&sph_PQsetdbLogin, (void**)&sph_PQstatus, (void**)&sph_PQsetClientEncoding,
				(void**)&sph_PQexec, (void**)&sph_PQresultStatus, (void**)&sph_PQntuples,
				(void**)&sph_PQfname, (void**)&sph_PQnfields, (void**)&sph_PQfinish,
				(void**)&sph_PQerrorMessage, (void**)&sph_PQunescapeBytea, (void**)&sph_PQfreemem};

		static CSphDynamicLibrary dLib ( GET_POSTGRESQL_LIB() );
		return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) );
	}

#else

	#define sph_PQgetvalue  PQgetvalue
	#define sph_PQgetlength  PQgetlength
	#define sph_PQclear  PQclear
	#define sph_PQsetdbLogin  PQsetdbLogin
	#define sph_PQstatus  PQstatus
	#define sph_PQsetClientEncoding  PQsetClientEncoding
	#define sph_PQexec  PQexec
	#define sph_PQresultStatus  PQresultStatus
	#define sph_PQntuples  PQntuples
	#define sph_PQfname  PQfname
	#define sph_PQnfields  PQnfields
	#define sph_PQfinish  PQfinish
	#define sph_PQerrorMessage  PQerrorMessage
	#define sph_PQunescapeBytea  PQunescapeBytea;
	#define sph_PQfreemem  PQfreemem;
	#define InitDynamicPosgresql() (true)

#endif

/// PgSQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_PgSQL : CSphSource_SQL
{
	explicit				CSphSource_PgSQL ( const char * sName );
	bool					SetupPgSQL ( const CSphSourceParams_PgSQL & tParams );
	bool					IterateStart ( CSphString & sError ) final;

protected:
	PGresult * 				m_pPgResult = nullptr;	///< postgresql execution restult context
	PGconn *				m_tPgDriver = nullptr;	///< postgresql connection context

	int						m_iPgRows = 0;		///< how much rows last step returned
	int						m_iPgRow = 0;		///< current row (0 based, as in PQgetvalue)

	CSphString				m_sPgClientEncoding;
	CSphVector<bool>		m_dIsColumnBool;

protected:
	void			SqlDismissResult () final;
	bool			SqlQuery ( const char * sQuery ) final;
	bool			SqlIsError () final;
	const char *	SqlError () final;
	bool			SqlConnect () final;
	void			SqlDisconnect () final;
	int				SqlNumFields() final;
	bool			SqlFetchRow() final;
	DWORD			SqlColumnLength ( int iIndex ) final;
	const char *	SqlColumn ( int iIndex ) final;
	const char *	SqlFieldName ( int iIndex ) final;
	Str_t			SqlCompressedColumnStream ( int iFieldIndex ) final;
	void			SqlCompressedColumnReleaseStream ( Str_t tStream ) final;
};

CSphSourceParams_PgSQL::CSphSourceParams_PgSQL ()
{
	m_iRangeStep = 1024;
	m_uPort = 5432;
}


CSphSource_PgSQL::CSphSource_PgSQL ( const char * sName )
	: CSphSource_SQL	( sName )
{
	m_bCanUnpack = true;
}


bool CSphSource_PgSQL::SqlIsError ()
{
	return ( m_iPgRow<m_iPgRows ); // if we're over, it's just last row
}


const char * CSphSource_PgSQL::SqlError ()
{
	if ( sph_PQerrorMessage!=nullptr )
		return sph_PQerrorMessage ( m_tPgDriver );
	return "PgSQL source wasn't initialized. Wrong name in dlopen?";
}


bool CSphSource_PgSQL::SetupPgSQL ( const CSphSourceParams_PgSQL & tParams )
{
	// checks
	CSphSource_SQL::SetupSQL ( tParams );

	m_sPgClientEncoding = tParams.m_sClientEncoding;
	if ( !m_sPgClientEncoding.cstr() )
		m_sPgClientEncoding = "";

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "pgsql%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}


bool CSphSource_PgSQL::IterateStart ( CSphString & sError )
{
	bool bResult = CSphSource_SQL::IterateStart ( sError );
	if ( !bResult )
		return false;

	int iMaxIndex = 0;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.GetAttr(i).m_iIndex );

	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
		iMaxIndex = Max ( iMaxIndex, m_tSchema.GetField(i).m_iIndex );

	m_dIsColumnBool.Resize ( iMaxIndex + 1 );
	ARRAY_FOREACH ( i, m_dIsColumnBool )
		m_dIsColumnBool[i] = false;

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		if ( m_tSchema.GetAttr(i).m_iIndex >=0 )
			m_dIsColumnBool [ m_tSchema.GetAttr(i).m_iIndex ] = ( m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_BOOL );
	}

	return true;
}


bool CSphSource_PgSQL::SqlConnect ()
{
	if ( !InitDynamicPosgresql() )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL (NO POSGRES CLIENT LIB, tried %s)\n", GET_POSTGRESQL_LIB() );
		return false;
	}

	char sPort[64];
	snprintf ( sPort, sizeof(sPort), "%d", m_tParams.m_uPort );
	m_tPgDriver = sph_PQsetdbLogin ( m_tParams.m_sHost.cstr(), sPort, NULL, NULL,
		m_tParams.m_sDB.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr() );

	if ( sph_PQstatus ( m_tPgDriver )==CONNECTION_BAD )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	// set client encoding
	if ( !m_sPgClientEncoding.IsEmpty() )
		if ( -1==sph_PQsetClientEncoding ( m_tPgDriver, m_sPgClientEncoding.cstr() ) )
	{
		SqlDisconnect ();
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL\n" );
		return false;
	}

	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-CONNECT: ok\n" );
	return true;
}


void CSphSource_PgSQL::SqlDisconnect ()
{
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-DISCONNECT\n" );

	sph_PQfinish ( m_tPgDriver );
}


bool CSphSource_PgSQL::SqlQuery ( const char * sQuery )
{
	m_iPgRow = -1;
	m_iPgRows = 0;

	m_pPgResult = sph_PQexec ( m_tPgDriver, sQuery );

	ExecStatusType eRes = sph_PQresultStatus ( m_pPgResult );
	if ( ( eRes!=PGRES_COMMAND_OK ) && ( eRes!=PGRES_TUPLES_OK ) )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL\n", sQuery );
		return false;
	}
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-QUERY: %s: ok\n", sQuery );

	m_iPgRows = sph_PQntuples ( m_pPgResult );
	return true;
}


void CSphSource_PgSQL::SqlDismissResult ()
{
	if ( !m_pPgResult )
		return;

	sph_PQclear ( m_pPgResult );
	m_pPgResult = NULL;
}


int CSphSource_PgSQL::SqlNumFields ()
{
	if ( !m_pPgResult )
		return -1;

	return sph_PQnfields ( m_pPgResult );
}


const char * CSphSource_PgSQL::SqlColumn ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	const char * szValue = sph_PQgetvalue ( m_pPgResult, m_iPgRow, iIndex );
	if ( m_dIsColumnBool.GetLength() && m_dIsColumnBool[iIndex] && szValue[0]=='t' && !szValue[1] )
		return "1";

	return szValue;
}


const char * CSphSource_PgSQL::SqlFieldName ( int iIndex )
{
	if ( !m_pPgResult )
		return NULL;

	return sph_PQfname ( m_pPgResult, iIndex );
}


bool CSphSource_PgSQL::SqlFetchRow ()
{
	if ( !m_pPgResult )
		return false;
	return ( ++m_iPgRow<m_iPgRows );
}


DWORD CSphSource_PgSQL::SqlColumnLength ( int iIndex )
{
	return sph_PQgetlength ( m_pPgResult, m_iPgRow, iIndex );
}


Str_t CSphSource_PgSQL::SqlCompressedColumnStream ( int iFieldIndex )
{
	auto tRes = SqlColumnStream ( iFieldIndex );

	if ( tRes.first )
	{
		size_t uSize;
		tRes.first = (const char*)sph_PQunescapeBytea ( (const unsigned char*)tRes.first, &uSize );
		assert ( uSize < INT_MAX );
		tRes.second = int(uSize);
	}
	return tRes;
}


void CSphSource_PgSQL::SqlCompressedColumnReleaseStream ( Str_t tStream )
{
	if ( tStream.first )
		sph_PQfreemem( (void*)tStream.first );
}

// the fabrics
CSphSource * CreateSourcePGSQL ( const CSphSourceParams_PgSQL & tParams, const char * sSourceName )
{
	auto * pSrc = new CSphSource_PgSQL ( sSourceName );
	if ( !pSrc->SetupPgSQL ( tParams ) )
		SafeDelete ( pSrc );

	return pSrc;
}
