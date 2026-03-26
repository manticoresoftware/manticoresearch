//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//
// declared to be used in ParseSysVarsAndTables

#include "search_handler.h"
#include "dynamic_idx.h"
#include "searchdsql.h"
#include "debug_cmds.h"

void HandleShowThreads ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleShowTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleShowInformationTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleShowSessions ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleCmdDescribe ( RowBuffer_i & tOut, SqlStmt_t * pStmt );
void HandleSelectIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleSelectFiles ( RowBuffer_i & tOut, const CSphString & sIndex, const CSphString & sThreadFormat );

// process @@system.something
static bool ParseSystem ( TableFeeder_fn & fnFeed, const CSphString & sName, SqlStmt_t * pStmt )
{
	if ( StrEqN ( FROMS (".threads"), sName.cstr() ) ) // select .. from @@system.threads
	{
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleShowThreads ( *pBuf, pStmt ); };
		if ( pStmt->m_sThreadFormat.IsEmpty() ) // override format to show all columns by default
			pStmt->m_sThreadFormat = "all";
	} else if ( StrEqN ( FROMS (".tables"), sName.cstr() ) ) // select .. from @@system.tables
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleShowTables ( *pBuf, pStmt ); };
	else if ( StrEqN ( FROMS (".tasks"), sName.cstr() ) ) // select .. from @@system.tasks
		fnFeed = [] ( RowBuffer_i * pBuf ) { HandleTasks ( *pBuf ); };
	else if ( StrEqN ( FROMS (".sched"), sName.cstr() ) ) // select .. from @@system.sched
		fnFeed = [] ( RowBuffer_i * pBuf ) { HandleSched ( *pBuf ); };
	else if ( StrEqN ( FROMS (".sessions"), sName.cstr() ) ) // select .. from @@system.sched
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleShowSessions ( *pBuf, pStmt ); };
	else
		return false;
	return true;
}

static bool ParseInformationSchema ( TableFeeder_fn & fnFeed, const CSphString & sName, SqlStmt_t * pStmt )
{
	if ( StrEqN ( FROMS (".tables"), sName.cstr() ) ) // select .. from INFORMATION_SCHEMA.TABLES
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleShowInformationTables ( *pBuf, pStmt ); };
	else
		return false;
	return true;
}

// select .. idx.@something
static bool ParseSubkeys ( TableFeeder_fn & fnFeed, const CSphString & sName, SqlStmt_t * pStmt )
{
	if ( StrEqN ( FROMS (".@table"), sName.cstr() ) ) // select .. idx.table
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleCmdDescribe ( *pBuf, pStmt ); };
	else if ( StrEqN ( FROMS (".@status"), sName.cstr() ) ) // select .. idx.status
		fnFeed = [pStmt] ( RowBuffer_i * pBuf ) { HandleSelectIndexStatus ( *pBuf, pStmt ); };
	else if ( StrEqN ( FROMS (".@files"), sName.cstr() ) ) // select .. from idx.files
	{
		CSphString sIndex = pStmt->m_sIndex;
		CSphString sThreadFormat = pStmt->m_sThreadFormat;
		fnFeed = [sIndex = std::move ( sIndex ), sThreadFormat = std::move ( sThreadFormat )] ( RowBuffer_i * pBuf ) { HandleSelectFiles ( *pBuf, sIndex, sThreadFormat ); };
	}
	else
		return false;
	return true;
}


bool SearchHandler_c::ParseSysVarsAndTables ()
{
	const char* szVar = m_dLocal.First().m_sName.cstr();
	const auto & dSubkeys = m_dNQueries.First().m_dStringSubkeys;
	const char* szEssence = "variable";
	bool bValid = false;
	AT_SCOPE_EXIT ([&,this] {
		if ( bValid )
			return;
		m_sError << "No such " << szEssence << " " << szVar;
		dSubkeys.for_each ( [this] ( const auto & s ) { m_sError << s; } );
	});

	if ( dSubkeys.IsEmpty() )
		return false;

	const char * szFirst = dSubkeys.First().cstr();
	const char * szLast = dSubkeys.Last().cstr();
	bool bSchema = StrEqN ( FROMS ( ".@table" ), szLast );
	TableFeeder_fn fnFeed;
	if ( StrEqN ( FROMS ("@@system"), szVar ) )
		bValid = ParseSystem ( fnFeed, szFirst, m_pStmt );

	// fixme! Disabled because of conflict with Buddy.
	// bSysVar = bSysVar || StrEqN ( FROMS ("information_schema"), tQuery.m_sIndexes.cstr() );
	// ^ uncomment this line in searchd_handler.cpp to enable.
	if ( !bValid )
	{
		if ( !StrEqN ( FROMS ( "information_schema" ), szVar ) )
			return false;

		szEssence = "table";
		bValid = ParseInformationSchema ( fnFeed, dSubkeys.First(), m_pStmt );
	}

	if ( !bValid )
		return false;

	cServedIndexRefPtr_c pIndex;
	if ( bSchema )
	{
		m_dLocal.First().m_sName.SetSprintf ( "%s%s%s", szVar, szFirst, szLast );
		pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );
	} else
	{
		m_dLocal.First().m_sName.SetSprintf ( "%s%s", szVar, szFirst );
		pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
	}
	m_dAcquired.AddIndex ( m_dLocal.First().m_sName, std::move ( pIndex ) );
	return true;
}


bool SearchHandler_c::ParseIdxSubkeys ()
{
	const char* szVar = m_dLocal.First().m_sName.cstr();
	const auto & dSubkeys = m_dNQueries.First().m_dStringSubkeys;

	assert ( !dSubkeys.IsEmpty () );
	const char * szFirst = dSubkeys.First().cstr();
	const char * szLast = dSubkeys.Last().cstr();

	bool bSchema = dSubkeys.GetLength() > 1 && StrEqN (FROMS ( ".@table" ), szLast);
	TableFeeder_fn fnFeed;
	if ( !ParseSubkeys ( fnFeed, dSubkeys.First(), m_pStmt ) )
	{
		m_sError << "No such table " << szVar;
		dSubkeys.for_each ( [this] ( const auto & s ) { m_sError << s; } );
		return false;
	}

	cServedIndexRefPtr_c pIndex;
	if ( bSchema )
	{
		m_dLocal.First().m_sName.SetSprintf ( "%s%s%s", szVar, szFirst, szLast );
		pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );
	} else
	{
		m_dLocal.First().m_sName.SetSprintf ( "%s%s", szVar, szFirst );
		pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
	}

	m_dAcquired.AddIndex ( m_dLocal.First().m_sName, std::move ( pIndex ) );
	return true;
}
