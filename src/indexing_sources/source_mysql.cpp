//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "source_mysql.h"

#if _WIN32
#include <winsock2.h>
#endif

#include <mysql.h>
#include <config_indexer.h>

/////////////////////////////////////////////////////////////////////////////
// MYSQL SOURCE
/////////////////////////////////////////////////////////////////////////////

#ifndef MYSQL_LIB
#define MYSQL_LIB nullptr
#endif
static const char * GET_MYSQL_LIB()
{
	const char * szEnv = getenv ( "MYSQL_LIB" );
	if ( szEnv )
		return szEnv;
	return MYSQL_LIB;
}

#define MYSQL57PLUS MYSQL_VERSION_ID >= 50700
#if MYSQL57PLUS
#define MYSQL_OPTIONS_OR_SSL_SET mysql_options
#else
#define MYSQL_OPTIONS_OR_SSL_SET mysql_ssl_set
#endif

#define XSTR( s ) STR ( s )
#define STR( s ) #s

#if DL_MYSQL

	static decltype (&mysql_free_result) sph_mysql_free_result = nullptr;
	static decltype (&mysql_next_result) sph_mysql_next_result = nullptr;
	static decltype (&mysql_use_result) sph_mysql_use_result = nullptr;
	static decltype (&mysql_num_rows) sph_mysql_num_rows = nullptr;
	static decltype (&mysql_query) sph_mysql_query = nullptr;
	static decltype (&mysql_errno) sph_mysql_errno = nullptr;
	static decltype (&mysql_error) sph_mysql_error = nullptr;
	static decltype (&mysql_init) sph_mysql_init = nullptr;
	static decltype ( &MYSQL_OPTIONS_OR_SSL_SET ) sph_mysql_options_or_ssl_set = nullptr;
	static decltype (&mysql_real_connect) sph_mysql_real_connect = nullptr;
	static decltype (&mysql_close) sph_mysql_close = nullptr;
	static decltype (&mysql_num_fields) sph_mysql_num_fields = nullptr;
	static decltype (&mysql_fetch_row) sph_mysql_fetch_row = nullptr;
	static decltype (&mysql_fetch_fields) sph_mysql_fetch_fields = nullptr;
	static decltype (&mysql_fetch_lengths) sph_mysql_fetch_lengths = nullptr;

	static bool InitDynamicMysql()
	{
		const char * sFuncs[] = { "mysql_free_result", "mysql_next_result", "mysql_use_result"
			, "mysql_num_rows", "mysql_query", "mysql_errno", "mysql_error"
			, "mysql_init", XSTR ( MYSQL_OPTIONS_OR_SSL_SET ), "mysql_real_connect", "mysql_close"
			, "mysql_num_fields", "mysql_fetch_row", "mysql_fetch_fields"
			, "mysql_fetch_lengths" };

		void ** pFuncs[] = { (void **) &sph_mysql_free_result, (void **) &sph_mysql_next_result
			, (void **) &sph_mysql_use_result, (void **) &sph_mysql_num_rows, (void **) &sph_mysql_query
			, (void **) &sph_mysql_errno, (void **) &sph_mysql_error, (void **) &sph_mysql_init
			, (void **) &sph_mysql_options_or_ssl_set, (void **) &sph_mysql_real_connect, (void **) &sph_mysql_close
			, (void **) &sph_mysql_num_fields, (void **) &sph_mysql_fetch_row
			, (void **) &sph_mysql_fetch_fields, (void **) &sph_mysql_fetch_lengths };

		static CSphDynamicLibrary dLib ( GET_MYSQL_LIB() );
		return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) );
	}

#else
	#define sph_mysql_free_result mysql_free_result
	#define sph_mysql_next_result mysql_next_result
	#define sph_mysql_use_result mysql_use_result
	#define sph_mysql_num_rows mysql_num_rows
	#define sph_mysql_query mysql_query
	#define sph_mysql_errno mysql_errno
	#define sph_mysql_error mysql_error
	#define sph_mysql_init mysql_init
	#define sph_mysql_options_or_ssl_set mysql_ssl_set
	#define sph_mysql_real_connect mysql_real_connect
	#define sph_mysql_close mysql_close
	#define sph_mysql_num_fields mysql_num_fields
	#define sph_mysql_fetch_row mysql_fetch_row
	#define sph_mysql_fetch_fields mysql_fetch_fields
	#define sph_mysql_fetch_lengths mysql_fetch_lengths
	#define InitDynamicMysql() (true)

#endif

/// MySQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_MySQL : CSphSource_SQL
{
	explicit				CSphSource_MySQL ( const char * sName );
	bool					SetupMySQL ( const CSphSourceParams_MySQL & tParams );

protected:
	MYSQL_RES *				m_pMysqlResult = nullptr;
	MYSQL_FIELD *			m_pMysqlFields = nullptr;
	MYSQL_ROW				m_tMysqlRow = nullptr;
	MYSQL					m_tMysqlDriver;
	unsigned long *			m_pMysqlLengths = nullptr;

	CSphString				m_sMysqlUsock;
	unsigned long			m_iMysqlConnectFlags = 0;
	CSphString				m_sSslKey;
	CSphString				m_sSslCert;
	CSphString				m_sSslCA;

protected:
	void			SqlDismissResult () override;
	bool			SqlQuery ( const char * sQuery ) override;
	bool			SqlIsError () override;
	const char *	SqlError () override;
	bool			SqlConnect () override;
	void			SqlDisconnect () override;
	int				SqlNumFields() override;
	bool			SqlFetchRow() override;
	DWORD			SqlColumnLength ( int iIndex ) override;
	const char *	SqlColumn ( int iIndex ) override;
	const char *	SqlFieldName ( int iIndex ) override;
};


CSphSourceParams_MySQL::CSphSourceParams_MySQL ()
	: m_iFlags ( 0 )
{
	m_uPort = 3306;
}


CSphSource_MySQL::CSphSource_MySQL ( const char * sName )
	: CSphSource_SQL	( sName )
{
	m_bCanUnpack = true;
	memset ( &m_tMysqlDriver, 0, sizeof ( m_tMysqlDriver ) );
}


void CSphSource_MySQL::SqlDismissResult ()
{
	if ( !m_pMysqlResult )
		return;

	while ( m_pMysqlResult )
	{
		sph_mysql_free_result ( m_pMysqlResult );
		m_pMysqlResult = NULL;

		// stored procedures might return multiple result sets
		// FIXME? we might want to index all of them
		// but for now, let's simply dismiss additional result sets
		if ( sph_mysql_next_result ( &m_tMysqlDriver )==0 )
		{
			m_pMysqlResult = sph_mysql_use_result ( &m_tMysqlDriver );

			static bool bOnce = false;
			if ( !bOnce && m_pMysqlResult && sph_mysql_num_rows ( m_pMysqlResult ) )
			{
				sphWarn ( "indexing of multiple result sets is not supported yet; some results sets were dismissed!" );
				bOnce = true;
			}
		}
	}

	m_pMysqlFields = nullptr;
	m_pMysqlLengths = nullptr;
}


bool CSphSource_MySQL::SqlQuery ( const char * sQuery )
{
	if ( sph_mysql_query ( &m_tMysqlDriver, sQuery ) )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-QUERY: %s: FAIL\n", sQuery );
		return false;
	}
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-QUERY: %s: ok\n", sQuery );

	m_pMysqlResult = sph_mysql_use_result ( &m_tMysqlDriver );
	m_pMysqlFields = nullptr;
	return true;
}


bool CSphSource_MySQL::SqlIsError ()
{
	return sph_mysql_errno ( &m_tMysqlDriver )!=0;
}


const char * CSphSource_MySQL::SqlError ()
{
	if ( sph_mysql_error!=nullptr )
		return sph_mysql_error ( &m_tMysqlDriver );
	return "MySQL source wasn't initialized. Wrong name in dlopen?";
}


bool CSphSource_MySQL::SqlConnect ()
{
	if_const ( !InitDynamicMysql() )
	{
		if ( m_tParams.m_bPrintQueries )
			fprintf ( stdout, "SQL-CONNECT: FAIL (NO MYSQL CLIENT LIB, tried %s)\n", GET_MYSQL_LIB() );
		return false;
	}

	sph_mysql_init ( &m_tMysqlDriver );
	if ( !m_sSslKey.IsEmpty() || !m_sSslCert.IsEmpty() || !m_sSslCA.IsEmpty() )
	{
#if MYSQL57PLUS
		sph_mysql_options_or_ssl_set ( &m_tMysqlDriver, MYSQL_OPT_SSL_KEY, m_sSslKey.cstr() );
		sph_mysql_options_or_ssl_set ( &m_tMysqlDriver, MYSQL_OPT_SSL_CERT, m_sSslCert.cstr() );
		sph_mysql_options_or_ssl_set ( &m_tMysqlDriver, MYSQL_OPT_SSL_CA, m_sSslCA.cstr() );
#else
		sph_mysql_options_or_ssl_set ( &m_tMysqlDriver, m_sSslKey.cstr(), m_sSslCert.cstr(), m_sSslCA.cstr(), NULL, NULL );
#endif
	}

	m_iMysqlConnectFlags |= CLIENT_MULTI_RESULTS; // we now know how to handle this
	bool bRes = ( nullptr!=sph_mysql_real_connect ( &m_tMysqlDriver,
		m_tParams.m_sHost.cstr(), m_tParams.m_sUser.cstr(), m_tParams.m_sPass.cstr(),
		m_tParams.m_sDB.cstr(), m_tParams.m_uPort, m_sMysqlUsock.cstr(), m_iMysqlConnectFlags ) );
	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, bRes ? "SQL-CONNECT: ok\n" : "SQL-CONNECT: FAIL\n" );
	return bRes;
}


void CSphSource_MySQL::SqlDisconnect ()
{
	m_pMysqlResult = nullptr;

	if ( m_tParams.m_bPrintQueries )
		fprintf ( stdout, "SQL-DISCONNECT\n" );

	sph_mysql_close ( &m_tMysqlDriver );

	m_sCollectDump.FinishBlocks ();
	auto fpDump = m_fpDumpRows;
	if ( !fpDump )
		fpDump = stdout;
	if ( m_sCollectDump.GetLength ()>0)
		fprintf ( fpDump, "%s", m_sCollectDump.cstr ());
	m_sCollectDump.Clear ();
}


int CSphSource_MySQL::SqlNumFields ()
{
	if ( !m_pMysqlResult )
		return -1;

	return sph_mysql_num_fields ( m_pMysqlResult );
}


bool CSphSource_MySQL::SqlFetchRow ()
{
	if ( !m_pMysqlResult )
		return false;

	m_tMysqlRow = sph_mysql_fetch_row ( m_pMysqlResult );
	return m_tMysqlRow!=NULL;
}


const char * CSphSource_MySQL::SqlColumn ( int iIndex )
{
	if ( !m_pMysqlResult )
		return NULL;

	return m_tMysqlRow[iIndex];
}


const char * CSphSource_MySQL::SqlFieldName ( int iIndex )
{
	if ( !m_pMysqlResult )
		return NULL;

	if ( !m_pMysqlFields )
		m_pMysqlFields = sph_mysql_fetch_fields ( m_pMysqlResult );

	return m_pMysqlFields[iIndex].name;
}


DWORD CSphSource_MySQL::SqlColumnLength ( int iIndex )
{
	if ( !m_pMysqlResult )
		return 0;

	if ( !m_pMysqlLengths )
		m_pMysqlLengths = sph_mysql_fetch_lengths ( m_pMysqlResult );

	return m_pMysqlLengths[iIndex];
}


bool CSphSource_MySQL::SetupMySQL ( const CSphSourceParams_MySQL & tParams )
{
	if ( !CSphSource_SQL::SetupSQL ( tParams ) )
		return false;

	m_sMysqlUsock = tParams.m_sUsock;
	m_iMysqlConnectFlags = tParams.m_iFlags;
	m_sSslKey = tParams.m_sSslKey;
	m_sSslCert = tParams.m_sSslCert;
	m_sSslCA = tParams.m_sSslCA;

	// build and store DSN for error reporting
	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "mysql%s", m_sSqlDSN.cstr()+3 );
	m_sSqlDSN = sBuf;

	return true;
}


// the fabrics
CSphSource * CreateSourceMysql ( const CSphSourceParams_MySQL & tParams, const char * sSourceName )
{
	auto * pSrc = new CSphSource_MySQL ( sSourceName );
	if ( !pSrc->SetupMySQL ( tParams ) )
		SafeDelete ( pSrc );

	return pSrc;
}
