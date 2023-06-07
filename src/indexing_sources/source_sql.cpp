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

#include "source_sql.h"
#include "attribute.h"
#include "sphinxint.h"
#include "conversion.h"

#if WITH_ZLIB
#include <zlib.h>
#endif

#define SPH_UNPACK_BUFFER_SIZE	4096
#include <ctime>

static char * sphStrMacro ( const char * sTemplate, const char * sMacro, uint64_t uValue )
{
	// expand macro
	char sExp[32];
	snprintf ( sExp, sizeof(sExp), UINT64_FMT, uValue );

	// calc lengths
	auto iExp = (int) strlen ( sExp );
	auto iMacro = (int) strlen ( sMacro );
	int iDelta = iExp-iMacro;

	// calc result length
	auto iRes = (int) strlen ( sTemplate );
	const char * sCur = sTemplate;
	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
	{
		iRes += iDelta;
		sCur++;
	}

	// build result
	char * sRes = new char [ iRes+1 ];
	char * sOut = sRes;
	const char * sLast = sTemplate;
	sCur = sTemplate;

	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
	{
		strncpy ( sOut, sLast, sCur-sLast ); sOut += sCur-sLast;
		strcpy ( sOut, sExp ); sOut += iExp; // NOLINT
		sCur += iMacro;
		sLast = sCur;
	}

	if ( *sLast )
		strcpy ( sOut, sLast ); // NOLINT

	assert ( (int)strlen(sRes)==iRes );
	return sRes;
}

/////////////////////////////////////////////////////////////////////////////
// GENERIC SQL SOURCE
/////////////////////////////////////////////////////////////////////////////
const char * const CSphSource_SQL::MACRO_VALUES [ CSphSource_SQL::MACRO_COUNT ] =
{
	"$start",
	"$end"
};


CSphSource_SQL::CSphSource_SQL ( const char * sName )
	: CSphSource	( sName )
{
}


bool CSphSource_SQL::SetupSQL ( const CSphSourceParams_SQL & tParams )
{
	// checks
	assert ( !tParams.m_sQuery.IsEmpty() );

	m_tParams = tParams;

	// defaults
	#define LOC_FIX_NULL(_arg) if ( !m_tParams._arg.cstr() ) m_tParams._arg = "";
	LOC_FIX_NULL ( m_sHost );
	LOC_FIX_NULL ( m_sUser );
	LOC_FIX_NULL ( m_sPass );
	LOC_FIX_NULL ( m_sDB );
	#undef LOC_FIX_NULL

	#define LOC_FIX_QARRAY(_arg) \
		ARRAY_FOREACH ( i, m_tParams._arg ) \
			if ( m_tParams._arg[i].IsEmpty() ) \
				m_tParams._arg.Remove ( i-- );
	LOC_FIX_QARRAY ( m_dQueryPre );
	LOC_FIX_QARRAY ( m_dQueryPost );
	LOC_FIX_QARRAY ( m_dQueryPostIndex );
	#undef LOC_FIX_QARRAY

	// build and store default DSN for error reporting
	m_sSqlDSN.SetSprintf ( "sql://%s:***@%s:%d/%s",
		m_tParams.m_sUser.cstr(), m_tParams.m_sHost.cstr(),
		m_tParams.m_uPort, m_tParams.m_sDB.cstr() );

	if ( m_tParams.m_iMaxFileBufferSize > 0 )
		m_iMaxFileBufferSize = m_tParams.m_iMaxFileBufferSize;
	m_eOnFileFieldError = m_tParams.m_eOnFileFieldError;

	return true;
}


static const char * SubstituteParams ( const char * sQuery, const char * const * dMacroses, const char ** dValues, int iMcount )
{
	// OPTIMIZE? things can be precalculated
	const char * sCur = sQuery;
	size_t iLen = 0;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<iMcount; i++ )
				if ( strncmp ( dMacroses[i], sCur, strlen ( dMacroses[i] ) )==0 )
				{
					sCur += strlen ( dMacroses[i] );
					iLen += strlen ( dValues[i] );
					break;
				}
			if ( i<iMcount )
				continue;
		}

		sCur++;
		iLen++;
	}
	iLen++; // trailing zero

	// do interpolation
	auto * sRes = new char [ iLen ];
	sCur = sQuery;

	char * sDst = sRes;
	while ( *sCur )
	{
		if ( *sCur=='$' )
		{
			int i;
			for ( i=0; i<iMcount; i++ )
				if ( strncmp ( dMacroses[i], sCur, strlen ( dMacroses[i] ) )==0 )
				{
					strcpy ( sDst, dValues[i] ); // NOLINT
					sCur += strlen ( dMacroses[i] );
					sDst += strlen ( dValues[i] );
					break;
				}
			if ( i<iMcount )
				continue;
		}
		*sDst++ = *sCur++;
	}
	*sDst++ = '\0';
	assert ( sDst-sRes==(int) iLen );
	return sRes;
}


bool CSphSource_SQL::RunQueryStep ( const char * sQuery, CSphString & sError )
{
	sError = "";

	if ( m_tParams.m_iRangeStep<=0 )
		return false;
	if ( m_tCurrentID>m_tMaxID )
		return false;

	static const int iBufSize = 32;
	const char * sRes = nullptr;

	sphSleepMsec ( m_tParams.m_iRangedThrottleMs );

	//////////////////////////////////////////////
	// range query with $start/$end interpolation
	//////////////////////////////////////////////

	assert ( m_tMinID>0 );
	assert ( m_tMaxID>0 );
	assert ( m_tMinID<=m_tMaxID );
	assert ( sQuery );

	char sValues [ MACRO_COUNT ] [ iBufSize ];
	const char * pValues [ MACRO_COUNT ];
	DocID_t tNextID = Min ( m_tCurrentID + (DocID_t)m_tParams.m_iRangeStep - 1, m_tMaxID );
	snprintf ( sValues[0], iBufSize, INT64_FMT, m_tCurrentID );
	snprintf ( sValues[1], iBufSize, INT64_FMT, tNextID );
	pValues[0] = sValues[0];
	pValues[1] = sValues[1];
	g_iIndexerCurrentRangeMin = m_tCurrentID;
	g_iIndexerCurrentRangeMax = tNextID;
	m_tCurrentID = 1 + tNextID;

	sRes = SubstituteParams ( sQuery, MACRO_VALUES, pValues, MACRO_COUNT );

	// run query
	SqlDismissResult ();
	bool bRes = SqlQuery ( sRes );

	if ( !bRes )
		sError.SetSprintf ( "sql_range_query: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );

	SafeDeleteArray ( sRes );
	return bRes;
}

static bool HookConnect ( const char* szCommand )
{
	FILE * pPipe = popen ( szCommand, "r" );
	if ( !pPipe )
		return false;
	pclose ( pPipe );
	return true;
}

inline static const char* skipspace ( const char* pBuf, const char* pBufEnd )
{
	assert ( pBuf );
	assert ( pBufEnd );

	while ( (pBuf<pBufEnd) && isspace ( *pBuf ) )
		++pBuf;
	return pBuf;
}

inline static const char* scannumber ( const char* pBuf, const char* pBufEnd, DocID_t * pRes )
{
	assert ( pBuf );
	assert ( pBufEnd );
	assert ( pRes );

	if ( pBuf<pBufEnd )
	{
		*pRes = 0;
		// FIXME! could check for overflow
		while ( isdigit ( *pBuf ) && pBuf<pBufEnd )
			(*pRes) = 10*(*pRes) + (int)( (*pBuf++)-'0' );
	}
	return pBuf;
}

static bool HookQueryRange ( const char* szCommand, DocID_t* pMin, DocID_t* pMax )
{
	FILE * pPipe = popen ( szCommand, "r" );
	if ( !pPipe )
		return false;

	const int MAX_BUF_SIZE = 1024;
	char dBuf [MAX_BUF_SIZE];
	auto iRead = (int)fread ( dBuf, 1, MAX_BUF_SIZE, pPipe );
	pclose ( pPipe );
	const char* pStart = dBuf;
	const char* pEnd = pStart + iRead;
	// leading whitespace and 1-st number
	pStart = skipspace ( pStart, pEnd );
	pStart = scannumber ( pStart, pEnd, pMin );
	// whitespace and 2-nd number
	pStart = skipspace ( pStart, pEnd );
	scannumber ( pStart, pEnd, pMax );
	return true;
}

static bool HookPostIndex ( const char* szCommand, DocID_t tLastIndexed )
{
	const char * sMacro = "$maxid";
	char sValue[32];
	const char* pValue = sValue;
	snprintf ( sValue, sizeof(sValue), UINT64_FMT, tLastIndexed );

	const char * pCmd = SubstituteParams ( szCommand, &sMacro, &pValue, 1 );

	FILE * pPipe = popen ( pCmd, "r" );
	SafeDeleteArray ( pCmd );
	if ( !pPipe )
		return false;
	pclose ( pPipe );
	return true;
}

/// connect to SQL server
bool CSphSource_SQL::Connect ( CSphString & sError )
{
	// do not connect twice
	if ( m_bSqlConnected )
		return true;

	// try to connect
	if ( !SqlConnect() )
	{
		sError.SetSprintf ( "sql_connect: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	m_tHits.Reserve ( m_iMaxHits );

	// all good
	m_bSqlConnected = true;
	if ( !m_tParams.m_sHookConnect.IsEmpty() && !HookConnect ( m_tParams.m_sHookConnect.cstr() ) )
	{
		sError.SetSprintf ( "hook_connect: runtime error %s when running external hook", strerrorm(errno) );
		return false;
	}
	return true;
}

static void FormatEscaped ( FILE * fp, const char * sLine )
{
	// handle empty lines
	if ( !sLine || !*sLine )
	{
		fprintf ( fp, "''" );
		return;
	}

	// pass one, count the needed buffer size
	auto iLen = (int) strlen(sLine);
	int iOut = 0;
	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\t':
		case '\'':
		case '\\':
			iOut += 2;
			break;

		default:
			iOut++;
			break;
	}
	iOut += 2; // quotes

	// allocate the buffer
	char sMinibuffer[8192];
	char * sMaxibuffer = NULL;
	char * sBuffer = sMinibuffer;

	if ( iOut>(int)sizeof(sMinibuffer) )
	{
		sMaxibuffer = new char [ iOut+4 ]; // 4 is just my safety gap
		sBuffer = sMaxibuffer;
	}

	// pass two, escape it
	char * sOut = sBuffer;
	*sOut++ = '\'';

	for ( int i=0; i<iLen; i++ )
		switch ( sLine[i] )
	{
		case '\t':
		case '\'':
		case '\\':	*sOut++ = '\\'; // no break intended
		// [[clang::fallthrough]];
		default:	*sOut++ = sLine[i];
	}
	*sOut++ = '\'';

	// print!
	assert ( sOut==sBuffer+iOut );
	fwrite ( sBuffer, 1, iOut, fp );

	// cleanup
	SafeDeleteArray ( sMaxibuffer );
}


#define LOC_ERROR(_msg,_arg)			{ sError.SetSprintf ( _msg, _arg ); return false; }
#define LOC_ERROR2(_msg,_arg,_arg2)		{ sError.SetSprintf ( _msg, _arg, _arg2 ); return false; }

/// setup them ranges (called both for document range-queries and MVA range-queries)
bool CSphSource_SQL::SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError, ERangesReason iReason )
{
	// check step
	if ( m_tParams.m_iRangeStep<=0 )
		LOC_ERROR ( "sql_range_step=" INT64_FMT ": must be non-zero positive", m_tParams.m_iRangeStep );

	if ( m_tParams.m_iRangeStep<128 )
		sphWarn ( "sql_range_step=" INT64_FMT ": too small; might hurt indexing performance!", m_tParams.m_iRangeStep );

	// check query for macros
	for ( const char* sMacro : MACRO_VALUES )
		if ( !strstr ( sQuery, sMacro ) )
			LOC_ERROR2 ( "%s: macro '%s' not found in match fetch query", sPrefix, sMacro );

	// run query
	if ( !SqlQuery ( sRangeQuery ) )
	{
		sError.SetSprintf ( "%s: range-query failed: %s (DSN=%s)", sPrefix, SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	// fetch min/max
	int iCols = SqlNumFields ();
	if ( iCols!=2 )
		LOC_ERROR2 ( "%s: expected 2 columns (min_id/max_id), got %d", sPrefix, iCols );

	if ( !SqlFetchRow() )
	{
		sError.SetSprintf ( "%s: range-query fetch failed: %s (DSN=%s)", sPrefix, SqlError(), m_sSqlDSN.cstr() );
		return false;
	}

	if ( ( SqlColumn(0)==NULL || !SqlColumn(0)[0] ) && ( SqlColumn(1)==NULL || !SqlColumn(1)[0] ) )
	{
		// the source seems to be empty; workaround
		m_tMinID = 1;
		m_tMaxID = 1;

	} else
	{
		// get and check min/max id
		const char * sCol0 = SqlColumn(0);
		const char * sCol1 = SqlColumn(1);
		m_tMinID = sphToInt64 ( sCol0 );
		m_tMaxID = sphToInt64 ( sCol1 );
		if ( !sCol0 ) sCol0 = "(null)";
		if ( !sCol1 ) sCol1 = "(null)";

		if ( m_tMinID>m_tMaxID )
			LOC_ERROR2 ( "sql_query_range: min_id='%s', max_id='%s': min_id must be less than max_id", sCol0, sCol1 );
	}

	SqlDismissResult ();

	if ( iReason==SRE_DOCS && ( !m_tParams.m_sHookQueryRange.IsEmpty() ) )
	{
		if ( !HookQueryRange ( m_tParams.m_sHookQueryRange.cstr(), &m_tMinID, &m_tMaxID ) )
			LOC_ERROR ( "hook_query_range: runtime error %s when running external hook", strerror(errno) );
		if ( m_tMinID>m_tMaxID )
			LOC_ERROR2 ( "hook_query_range: min_id=" INT64_FMT ", max_id=" INT64_FMT ": min_id must be less than max_id", m_tMinID, m_tMaxID );
	}

	return true;
}


/// issue main rows fetch query
bool CSphSource_SQL::IterateStart ( CSphString & sError )
{
	assert ( m_bSqlConnected );

	if ( !QueryPreAll ( sError ) )
		return false;

	// run pre-queries
	ARRAY_FOREACH ( i, m_tParams.m_dQueryPre )
	{
		if ( !SqlQuery ( m_tParams.m_dQueryPre[i].cstr() ) )
		{
			sError.SetSprintf ( "sql_query_pre[%d]: %s (DSN=%s)", i, SqlError(), m_sSqlDSN.cstr() );
			SqlDisconnect ();
			return false;
		}

		SqlDismissResult ();
	}

	while (true)
	{
		m_tParams.m_iRangeStep = 0;

		// issue first fetch query
		if ( !m_tParams.m_sQueryRange.IsEmpty() )
		{
			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;
			// run range-query; setup ranges
			if ( !SetupRanges ( m_tParams.m_sQueryRange.cstr(), m_tParams.m_sQuery.cstr(), "sql_query_range: ", sError, SRE_DOCS ) )
				return false;

			// issue query
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( m_tParams.m_sQuery.cstr(), sError ) )
				return false;
		} else
		{
			// normal query; just issue
			if ( !SqlQuery ( m_tParams.m_sQuery.cstr() ) )
			{
				sError.SetSprintf ( "sql_query: %s (DSN=%s)", SqlError(), m_sSqlDSN.cstr() );
				return false;
			}
		}
		break;
	}

	// some post-query setup
	m_tSchema.Reset();

	for (auto & i : m_dUnpack)
		i = SPH_UNPACK_NONE;

	m_iSqlFields = SqlNumFields(); // for rowdump

	CSphVector<bool> dFound;
	dFound.Resize ( m_tParams.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, dFound )
		dFound[i] = false;

	const bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// map plain attrs from SQL
	for ( int i=0; i<m_iSqlFields; i++ )
	{
		const char * sName = SqlFieldName(i);
		if ( !sName )
			LOC_ERROR ( "column number %d has no name", i+1 );

		CSphColumnInfo tCol ( sName );
		ARRAY_FOREACH ( j, m_tParams.m_dAttrs )
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dAttrs[j].m_sName.cstr() ) )
			{
				const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[j];

				tCol.m_eAttrType = tAttr.m_eAttrType;
				assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );

				if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
					LOC_ERROR ( "multi-valued attribute '%s' of wrong source-type found in query; must be 'field'", tAttr.m_sName.cstr() );

				tCol = tAttr;
				dFound[j] = true;
				break;
			}

		for ( auto & tJoined : m_tParams.m_dJoinedFields )
			if ( tJoined.m_sName==sName )
				LOC_ERROR ( "joined field '%s' has the same name as a fulltext field", sName );
		
		if ( !i )
		{
			// id column coming from sql may have another name
			tCol.m_sName = sphGetDocidName();
			tCol.m_eAttrType = SPH_ATTR_BIGINT;
		}

		ARRAY_FOREACH ( j, m_tParams.m_dFileFields )
		{
			if ( !strcasecmp ( tCol.m_sName.cstr(), m_tParams.m_dFileFields[j].cstr() ) )
				tCol.m_bFilename = true;
		}

		tCol.m_iIndex = i;
		tCol.m_eWordpart = GetWordpart ( tCol.m_sName.cstr(), bWordDict );

		if ( tCol.m_eAttrType==SPH_ATTR_NONE || tCol.m_bIndexed )
		{
			m_tSchema.AddField ( tCol );
			ARRAY_FOREACH ( k, m_tParams.m_dUnpack )
			{
				CSphUnpackInfo & tUnpack = m_tParams.m_dUnpack[k];
				if ( tUnpack.m_sName==tCol.m_sName )
				{
					if ( !m_bCanUnpack )
					{
						sError.SetSprintf ( "this source does not support column unpacking" );
						return false;
					}
					int iIndex = m_tSchema.GetFieldsCount() - 1;
					if ( iIndex < SPH_MAX_FIELDS )
					{
						m_dUnpack[iIndex] = tUnpack.m_eFormat;
						m_dUnpackBuffers[iIndex].Resize ( SPH_UNPACK_BUFFER_SIZE );
					}
					break;
				}
			}
		}

		if ( tCol.m_eAttrType!=SPH_ATTR_NONE )
		{
			if ( CSphSchema::IsReserved ( tCol.m_sName.cstr() ) )
				LOC_ERROR ( "%s is not a valid attribute name", tCol.m_sName.cstr() );

			m_tSchema.AddAttr ( tCol, true ); // all attributes are dynamic at indexing time
		}
	}

	// map multi-valued attrs
	ARRAY_FOREACH ( i, m_tParams.m_dAttrs )
	{
		const CSphColumnInfo & tAttr = m_tParams.m_dAttrs[i];
		if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			CSphColumnInfo tMva;
			tMva = tAttr;
			tMva.m_iIndex = m_tSchema.GetAttrsCount();

			if ( CSphSchema::IsReserved ( tMva.m_sName.cstr() ) )
				LOC_ERROR ( "%s is not a valid attribute name", tMva.m_sName.cstr() );

			m_tSchema.AddAttr ( tMva, true ); // all attributes are dynamic at indexing time
			dFound[i] = true;
		}
	}

	// warn if some attrs went unmapped
	ARRAY_FOREACH ( i, dFound )
		if ( !dFound[i] )
			sphWarn ( "attribute '%s' not found - IGNORING", m_tParams.m_dAttrs[i].m_sName.cstr() );

	// joined fields
	m_iPlainFieldsLength = m_tSchema.GetFieldsCount();

	for ( const auto & tJoinedField: m_tParams.m_dJoinedFields )
	{
		CSphColumnInfo tCol;
		tCol.m_iIndex = -1;
		tCol.m_sName = tJoinedField.m_sName;
		tCol.m_sQuery = tJoinedField.m_sQuery;
		tCol.m_bPayload = tJoinedField.m_bPayload;
		tCol.m_sQueryRange = tJoinedField.m_sRanged;
		tCol.m_eWordpart = GetWordpart ( tCol.m_sName.cstr(), bWordDict );

		tCol.m_eSrc = ( !tJoinedField.m_bRangedMain ? SPH_ATTRSRC_QUERY : SPH_ATTRSRC_RANGEDMAINQUERY );
		if ( !tJoinedField.m_sRanged.IsEmpty() )
			tCol.m_eSrc = SPH_ATTRSRC_RANGEDQUERY;

		m_tSchema.AddField ( tCol );
	}

	// auto-computed length attributes
	if ( !AddAutoAttrs ( sError ) )
		return false;

	StrVec_t dWarnings;
	m_tSchema.SetupFlags ( *this, false, &dWarnings );
	for ( const auto & i : dWarnings )
		sphWarn ( "%s", i.cstr() );

	// check it
	if ( m_tSchema.GetFieldsCount()>SPH_MAX_FIELDS )
		LOC_ERROR2 ( "too many fields (fields=%d, max=%d)", m_tSchema.GetFieldsCount(), SPH_MAX_FIELDS );

	// alloc storage
	AllocDocinfo();

	// log it
	DumpRowsHeader();
	return true;
}

bool CSphSource_SQL::QueryPreAll ( CSphString& sError )
{
	// run pre-queries
	ARRAY_FOREACH ( i, m_tParams.m_dQueryPreAll )
	{
		if ( !SqlQuery ( m_tParams.m_dQueryPreAll[i].cstr() ) )
		{
			sError.SetSprintf ( "sql_query_pre_all[%d]: %s (DSN=%s)", i, SqlError(), m_sSqlDSN.cstr() );
			SqlDisconnect();
			return false;
		}
//		sphWarn ( "query_pre_app %d: %s", i, m_tParams.m_dQueryPreAll[i].cstr() );
		SqlDismissResult();
	}
	return true;
}

// dump schema to given file or stdout, if rt inserts expected
void CSphSource_SQL::DumpRowsHeader ()
{
	if ( m_tParams.m_bPrintRTQueries )
	{
		DumpRowsHeaderSphinxql();
		return;
	}

	if ( !m_fpDumpRows )
		return;

	const char * sTable = m_tSchema.GetName ();

	time_t iNow = time ( nullptr );
	fprintf ( m_fpDumpRows, "#\n# === source %s ts %d\n# %s#\n", sTable, (int) iNow, ctime ( &iNow ));
	for ( int i = 0; i<m_tSchema.GetFieldsCount (); ++i )
		fprintf ( m_fpDumpRows, "# field %d: %s\n", i, m_tSchema.GetFieldName ( i ));

	for ( int i = 0; i<m_tSchema.GetAttrsCount (); i++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr ( i );
		fprintf ( m_fpDumpRows, "# %s = %s # attr %d\n", sphTypeDirective ( tCol.m_eAttrType ), tCol.m_sName.cstr (),
				  i );
	}

	fprintf ( m_fpDumpRows, "#\n\nDROP TABLE IF EXISTS rows_%s;\nCREATE TABLE rows_%s (\n  id VARCHAR(32) NOT NULL,\n",
			  sTable, sTable );
	for ( int i = 1; i<m_iSqlFields; ++i )
		fprintf ( m_fpDumpRows, "  %s VARCHAR(4096) NOT NULL,\n", SqlFieldName ( i ));
	fprintf ( m_fpDumpRows, "  KEY(id) );\n\n" );
}

// dump schema to given file or stdout, if rt inserts expected
void CSphSource_SQL::DumpRowsHeaderSphinxql ()
{
	const char * sTable = m_tSchema.GetName ();

	time_t iNow = time ( nullptr );
	m_sCollectDump.Clear();
	m_sCollectDump.Sprintf ( "#\n# === source %s ts %d\n# %s#\n", sTable, (int) iNow, ctime ( &iNow ));

	SmallStringHash_T<int> hSqlSchema;
	for ( int i = 0; i<m_iSqlFields; ++i )
		hSqlSchema.Add ( i, SqlFieldName ( i ));

	m_dDumpMap.Reset();
	m_dDumpMap.Add ( {hSqlSchema.Exists("id") ? hSqlSchema["id"] : 0, false} );

	for ( int i = 0; i<m_tSchema.GetFieldsCount (); ++i )
	{
		if ( hSqlSchema.Exists ( m_tSchema.GetFieldName ( i ) ))
			m_dDumpMap.Add ( {hSqlSchema[m_tSchema.GetFieldName ( i )], true} );
		m_sCollectDump.Sprintf ( "#\trt_field = %s # field %d\n", m_tSchema.GetFieldName ( i ), i );
	}

	auto * sBlobLocator = sphGetBlobLocatorName ();
	auto * sIdLocator = sphGetDocidName ();
	for ( int i = 0; i<m_tSchema.GetAttrsCount (); ++i )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr ( i );
		if ( tCol.m_sName!=sBlobLocator && tCol.m_sName!=sIdLocator )
		{
			if ( hSqlSchema.Exists ( tCol.m_sName ))
				m_dDumpMap.Add ( {hSqlSchema[tCol.m_sName],
						tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_STRINGPTR} );
			auto sTypeName = sphRtTypeDirective ( tCol.m_eAttrType );
			if ( sTypeName )
				m_sCollectDump.Sprintf ( "#\t%s = %s # attr %d\n", sTypeName, tCol.m_sName.cstr (), i );
		}
	}

	m_sCollectDump << "#\n\n";

	auto fpDump = m_fpDumpRows;
	if ( !fpDump )
		fpDump = stdout;

	fprintf ( fpDump, "%s", m_sCollectDump.cstr() );
	m_sCollectDump.Clear();
}

#undef LOC_ERROR
#undef LOC_ERROR2
#undef LOC_SQL_ERROR


void CSphSource_SQL::Disconnect ()
{
	SafeDeleteArray ( m_pReadFileBuffer );
	m_tHits.Reset();

	if ( m_bSqlConnected )
		SqlDisconnect ();

	m_bSqlConnected = false;
}


bool CSphSource_SQL::StoreAttribute ( int iAttr )
{
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_JSON:
		// memorize string, fixup NULLs
		m_dStrAttrs[iAttr] = SqlColumn ( tAttr.m_iIndex );
		if ( !m_dStrAttrs[iAttr].cstr() )
			m_dStrAttrs[iAttr] = "";

		break;

	case SPH_ATTR_FLOAT:
	{
		float fValue = sphToFloat ( SqlColumn ( tAttr.m_iIndex ) ); // FIXME? report conversion errors maybe?
		m_dAttrs[iAttr] = sphF2DW(fValue);
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, fValue );
	}
	break;

	case SPH_ATTR_BIGINT:
		if ( tAttr.m_iIndex<0 )
		{
			assert ( tAttr.m_sName==sphGetBlobLocatorName() );
		} else
		{
			bool bDocId = !iAttr;
			const char * szNumber = SqlColumn ( tAttr.m_iIndex );

			CSphString sWarn;
			if ( bDocId )
			{
				uint64_t uDocID = StrToDocID ( szNumber, sWarn );
				if ( !sWarn.IsEmpty() )
				{
					sphWarn ( "%s", sWarn.cstr() );
					return false;
				}

				m_dAttrs[iAttr] = (int64_t)uDocID;
				m_tMaxFetchedID = (int64_t)Max ( (uint64_t)m_tMaxFetchedID, uDocID );
			}
			else
				m_dAttrs[iAttr] = sphToInt64 ( szNumber, &sWarn );

			if ( !sWarn.IsEmpty() )
				sphWarn ( "%s", sWarn.cstr() );

			if ( !tAttr.IsColumnar() )
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[iAttr] );
		}
		break;

	case SPH_ATTR_TOKENCOUNT:
		// reset, and the value will be filled by IterateHits()
		m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
			ParseFieldMVA ( iAttr, SqlColumn ( tAttr.m_iIndex ) );
		break;

	case SPH_ATTR_BOOL:
		m_dAttrs[iAttr] = sphToDword ( SqlColumn ( tAttr.m_iIndex ) ) ? 1 : 0;
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[iAttr] ); // FIXME? report conversion errors maybe?
		break;

	default:
		// just store as uint by default
		m_dAttrs[iAttr] = sphToDword ( SqlColumn ( tAttr.m_iIndex ) ); // FIXME? report conversion errors maybe?
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[iAttr] ); // FIXME? report conversion errors maybe?
		break;
	}

	return true;
}


BYTE ** CSphSource_SQL::NextDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_bSqlConnected );

	bool bSkipDoc = false;

	do
	{
		// try to get next row
		bool bGotRow = SqlFetchRow ();

		bEOF = false;

		// when the party's over...
		while ( !bGotRow )
		{
			// is that an error?
			if ( SqlIsError() )
			{
				sError.SetSprintf ( "sql_fetch_row: %s", SqlError() );
				return nullptr;
			}

			// maybe we can do next step yet?
			if ( !RunQueryStep ( m_tParams.m_sQuery.cstr(), sError ) )
			{
				// if there's a message, there's an error
				// otherwise, we're just over
				if ( !sError.IsEmpty() )
					return nullptr;

			} else
			{
				// step went fine; try to fetch
				bGotRow = SqlFetchRow ();
				continue;
			}

			SqlDismissResult();

			// ok, we're over
			ARRAY_FOREACH ( i, m_tParams.m_dQueryPost )
			{
				if ( !SqlQuery ( m_tParams.m_dQueryPost[i].cstr() ) )
				{
					sphWarn ( "sql_query_post[%d]: error=%s, query=%s",	i, SqlError(), m_tParams.m_dQueryPost[i].cstr() );
					break;
				}

				SqlDismissResult();
			}

			bEOF = true;
			return nullptr;
		}

		// cleanup attrs
		for ( int i=0; i<m_tSchema.GetRowSize(); i++ )
			m_tDocInfo.m_pDynamic[i] = 0;

		// split columns into fields and attrs
		for ( int i=0; i<m_iPlainFieldsLength; i++ )
		{
			// get that field
			auto tCol = SqlUnpackColumn ( i, m_dUnpack[i] );
			m_dFields[i] = (BYTE*)const_cast<char*> ( tCol.first );
			m_dFieldLengths[i] = tCol.second;
		}

		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & i : m_dMvas )
			i.Resize(0);

		bSkipDoc = false;
		for ( int i=0; i<m_tSchema.GetAttrsCount() && !bSkipDoc; i++ )
			if ( !StoreAttribute(i) )
				bSkipDoc = true;
	}
	while ( bSkipDoc );

	// log it
	DumpDocument();
	return m_dFields;
}

void CSphSource_SQL::DumpDocument ()
{
	if ( m_tParams.m_bPrintRTQueries ) {
		DumpDocumentSphinxql ();
		return;
	}

	if ( !m_fpDumpRows )
		return;

	fprintf ( m_fpDumpRows, "INSERT INTO rows_%s VALUES (", m_tSchema.GetName ());
	for ( int i = 0; i<m_iSqlFields; ++i )
	{
		if ( i )
			fprintf ( m_fpDumpRows, ", " );
		FormatEscaped ( m_fpDumpRows, SqlColumn ( i ));
	}
	fprintf ( m_fpDumpRows, ");\n" );
}

void CSphSource_SQL::DumpDocumentSphinxql ()
{
	if (m_sCollectDump.IsEmpty ())
	{
		m_sCollectDump.Sprintf ( "INSERT INTO %s VALUES ", m_tParams.m_sDumpRTIndex.cstr() );
		m_sCollectDump.StartBlock (",",nullptr,";\n");
	}
	m_sCollectDump.StartBlock( dBracketsComma );
	ARRAY_FOREACH ( i, m_dDumpMap )
	{
		if ( m_dDumpMap[i].second )
			m_sCollectDump.FixupSpacedAndAppendEscaped ( SqlColumn ( m_dDumpMap[i].first ) );
		else
			m_sCollectDump << SqlColumn ( m_dDumpMap[i].first );
	}
	m_sCollectDump.FinishBlock();

	if ( m_sCollectDump.GetLength ()>m_iCutoutDumpSize )
	{
		m_sCollectDump.FinishBlocks ();
		auto fpDump = m_fpDumpRows;
		if ( !fpDump )
			fpDump = stdout;
		fprintf ( fpDump, "%s", m_sCollectDump.cstr ());
		m_sCollectDump.Clear ();
	}
}


const int * CSphSource_SQL::GetFieldLengths() const
{
	return m_dFieldLengths;
}


void CSphSource_SQL::PostIndex ()
{
	if ( ( !m_tParams.m_dQueryPostIndex.GetLength() ) && m_tParams.m_sHookPostIndex.IsEmpty() )
		return;

	assert ( !m_bSqlConnected );

	const char * sSqlError = NULL;
	if ( m_tParams.m_dQueryPostIndex.GetLength() )
	{
#define LOC_SQL_ERROR(_msg) { sSqlError = _msg; break; }

		while (true)
		{
			if ( !SqlConnect () )
				LOC_SQL_ERROR ( "mysql_real_connect" );

			ARRAY_FOREACH ( i, m_tParams.m_dQueryPostIndex )
			{
				char * sQuery = sphStrMacro ( m_tParams.m_dQueryPostIndex[i].cstr(), "$maxid", m_tMaxFetchedID );
				bool bRes = SqlQuery ( sQuery );
				delete [] sQuery;

				if ( !bRes )
					LOC_SQL_ERROR ( "sql_query_post_index" );

				SqlDismissResult ();
			}

			break;
		}

		if ( sSqlError )
			sphWarn ( "%s: %s (DSN=%s)", sSqlError, SqlError(), m_sSqlDSN.cstr() );

#undef LOC_SQL_ERROR

		SqlDisconnect ();
	}
	if ( !m_tParams.m_sHookPostIndex.IsEmpty() && !HookPostIndex ( m_tParams.m_sHookPostIndex.cstr(), m_tMaxFetchedID ) )
	{
		sphWarn ( "hook_post_index: runtime error %s when running external hook", strerrorm(errno) );
	}
}


bool CSphSource_SQL::IterateMultivaluedStart ( int iAttr, CSphString & sError )
{
	if ( iAttr<0 || iAttr>=m_tSchema.GetAttrsCount() )
		return false;

	m_iMultiAttr = iAttr;
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(iAttr);

	if ( !(tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) )
		return false;

	if ( !QueryPreAll ( sError ) )
		return false;

	CSphString sPrefix;
	switch ( tAttr.m_eSrc )
	{
	case SPH_ATTRSRC_FIELD:
		return false;

	case SPH_ATTRSRC_QUERY:
		// run simple query
		if ( !SqlQuery ( tAttr.m_sQuery.cstr() ) )
		{
			sError.SetSprintf ( "multi-valued attr '%s' query failed: %s", tAttr.m_sName.cstr(), SqlError() );
			return false;
		}
		break;

	case SPH_ATTRSRC_RANGEDQUERY:
			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

			// setup ranges
			sPrefix.SetSprintf ( "multi-valued attr '%s' ranged query: ", tAttr.m_sName.cstr() );
			if ( !SetupRanges ( tAttr.m_sQueryRange.cstr(), tAttr.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_MVA ) )
				return false;

			// run first step (in order to report errors)
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sError ) )
				return false;

			break;

	case SPH_ATTRSRC_RANGEDMAINQUERY:
			if ( m_tParams.m_sQueryRange.IsEmpty() )
			{
				sError.SetSprintf ( "multi-valued attr '%s': empty main range query", tAttr.m_sName.cstr() );
				return false;
			}

			m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

			// setup ranges
			sPrefix.SetSprintf ( "multi-valued attr '%s' ranged query: ", tAttr.m_sName.cstr() );
			if ( !SetupRanges ( m_tParams.m_sQueryRange.cstr(), tAttr.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_MVA ) )
				return false;

			// run first step (in order to report errors)
			m_tCurrentID = m_tMinID;
			if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sError ) )
				return false;

			break;

	default:
		sError.SetSprintf ( "INTERNAL ERROR: unknown multi-valued attr source type %d", tAttr.m_eSrc );
		return false;
	}

	// check fields count
	if ( SqlNumFields()!=2 )
	{
		sError.SetSprintf ( "multi-valued attr '%s' query returned %d fields (expected 2)", tAttr.m_sName.cstr(), SqlNumFields() );
		SqlDismissResult ();
		return false;
	}
	return true;
}


bool CSphSource_SQL::IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue )
{
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( m_iMultiAttr );

	assert ( m_bSqlConnected );
	assert ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET );

	// fetch next row
	bool bGotRow = SqlFetchRow ();
	while ( !bGotRow )
	{
		if ( SqlIsError() )
			sphDie ( "sql_fetch_row: %s", SqlError() ); // FIXME! this should be reported

		if ( tAttr.m_eSrc!=SPH_ATTRSRC_RANGEDQUERY &&  tAttr.m_eSrc!=SPH_ATTRSRC_RANGEDMAINQUERY )
		{
			SqlDismissResult();
			return false;
		}

		CSphString sTmp;
		if ( !RunQueryStep ( tAttr.m_sQuery.cstr(), sTmp ) ) // FIXME! this should be reported
			return false;

		bGotRow = SqlFetchRow ();
		continue;
	}

	// return that tuple or offset to storage for MVA64 value
	iDocID = sphToInt64 ( SqlColumn(0) );
	iMvaValue = sphToInt64 ( SqlColumn(1) );

	return true;
}


bool CSphSource_SQL::IterateKillListStart ( CSphString & sError )
{
	if ( m_tParams.m_sQueryKilllist.IsEmpty () )
		return false;

	if ( !SqlQuery ( m_tParams.m_sQueryKilllist.cstr () ) )
	{
		sError.SetSprintf ( "killlist query failed: %s", SqlError() );
		return false;
	}

	return true;
}


bool CSphSource_SQL::IterateKillListNext ( DocID_t & tDocID )
{
	if ( SqlFetchRow () )
		tDocID = sphToInt64 ( SqlColumn(0) );
	else
	{
		if ( SqlIsError() )
			sphDie ( "sql_query_killlist: %s", SqlError() ); // FIXME! this should be reported
		else
		{
			SqlDismissResult ();
			return false;
		}
	}

	return true;
}


void CSphSource_SQL::ReportUnpackError ( int iIndex, int iError )
{
	if ( !m_bUnpackFailed )
	{
		m_bUnpackFailed = true;
		sphWarn ( "failed to unpack column '%s', error=%d, rowid=%u", SqlFieldName(iIndex), iError, m_tDocInfo.m_tRowID );
	}
}


Str_t CSphSource_SQL::SqlColumnStream ( int iFieldIndex )
{
	int iIndex = m_tSchema.GetField ( iFieldIndex ).m_iIndex;
	Str_t tResult { SqlColumn ( iIndex ), SqlColumnLength ( iIndex ) };
	if ( IsEmpty ( tResult ) )
		tResult.first = nullptr;
	return tResult;
}

Str_t CSphSource_SQL::SqlCompressedColumnStream ( int iFieldIndex )
{
	return SqlColumnStream ( iFieldIndex );
}

void CSphSource_SQL::SqlCompressedColumnReleaseStream ( Str_t /*tStream*/ )
{
}

#if WITH_ZLIB
namespace {
Str_t UnpackZlib ( CSphVector<char>& tBuffer, Str_t tInputStream )
{
	Str_t tResult { nullptr, 0 };
	uLong uBufferOffset = 0;

	z_stream tStream;
	tStream.zalloc = Z_NULL;
	tStream.zfree = Z_NULL;
	tStream.opaque = Z_NULL;
	tStream.avail_in = tInputStream.second;
	tStream.next_in = (Bytef*)tInputStream.first;

	tResult.second = inflateInit ( &tStream );
	if ( tResult.second != Z_OK )
		return tResult;

	while ( true )
	{
		tStream.next_out = (Bytef*)&tBuffer[static_cast<int64_t> ( uBufferOffset )];
		tStream.avail_out = tBuffer.GetLength() - uBufferOffset - 1;
		tResult.second = inflate ( &tStream, Z_NO_FLUSH );

		if ( tResult.second == Z_OK )
		{
			assert ( tStream.avail_out == 0 );

			tBuffer.Resize ( tBuffer.GetLength() * 2 );
			uBufferOffset = tStream.total_out;
			continue;
		}

		if ( tResult.second == Z_STREAM_END )
		{
			tBuffer[static_cast<int64_t> ( tStream.total_out )] = '\0';
			tResult.first = &tBuffer[0];
			tResult.second = static_cast<int>(tStream.total_out);
		}
		break;
	}

	inflateEnd ( &tStream );
	return tResult;
}
}
#endif

Str_t CSphSource_SQL::SqlUnpackColumn ( int iFieldIndex, ESphUnpackFormat eFormat )
{
	int iIndex = m_tSchema.GetField ( iFieldIndex ).m_iIndex;
	Str_t tResult { nullptr, 0 };

	switch ( eFormat )
	{
#if WITH_ZLIB
	case SPH_UNPACK_ZLIB:
		{
			auto tSqlCompressedStream = SqlCompressedColumnStream ( iFieldIndex );
			auto _ = AtScopeExit ( [tSqlCompressedStream, this] { SqlCompressedColumnReleaseStream ( tSqlCompressedStream ); } );
			tResult = UnpackZlib ( 	m_dUnpackBuffers[iFieldIndex], tSqlCompressedStream );
		}
		break;
	case SPH_UNPACK_MYSQL_COMPRESS:
		{
			auto tSqlStream = SqlColumnStream ( iFieldIndex );
			if ( tSqlStream.second <= 4 )
			{
				if ( !m_bUnpackFailed )
				{
					m_bUnpackFailed = true;
					sphWarn ( "failed to unpack '%s', invalid column size (size=%d), rowid=%u", SqlFieldName ( iIndex ), tSqlStream.second, m_tDocInfo.m_tRowID );
				}
				break;
			}

			uLong uSize = 0;
			for ( int i = 0; i < 4; ++i )
				uSize += ( static_cast<uLong> ( (BYTE)tSqlStream.first[i] ) ) << ( 8 * i );
			uSize &= 0x3FFFFFFF;

			if ( uSize > m_tParams.m_uUnpackMemoryLimit )
			{
				if ( !m_bUnpackOverflow )
				{
					m_bUnpackOverflow = true;
					sphWarn ( "failed to unpack '%s', column size limit exceeded (size=%d), rowid=%u", SqlFieldName ( iIndex ), (int)uSize, m_tDocInfo.m_tRowID );
				}
				break;
			}

			CSphVector<char> & tBuffer = m_dUnpackBuffers[iFieldIndex];
			tBuffer.Resize ( static_cast<int64_t> ( uSize ) + 1 );
			unsigned long uLen = tSqlStream.second - 4;
			int iResult = uncompress ( (Bytef*)tBuffer.Begin(), &uSize, (Bytef*)tSqlStream.first + 4, uLen );
			if ( iResult == Z_OK )
			{
				tBuffer[static_cast<int64_t> ( uSize )] = 0;
				tResult.first = &tBuffer[0];
				tResult.second = static_cast<int> ( uSize );
			} else
				tResult.second = iResult;
		}
		break;
#endif
	case SPH_UNPACK_NONE:
	default:
		tResult = SqlColumnStream ( iFieldIndex );
		return tResult;
	}

	if ( !tResult.first )
	{
		ReportUnpackError ( iIndex, tResult.second );
		tResult.second = 0;
	}
	return tResult;
}

struct CmpPairs_fn
{
	bool IsLess ( const IDPair_t & tA, const IDPair_t & tB ) const
	{
		if ( tA.m_tDocID < tB.m_tDocID )
			return true;
		else if ( tA.m_tDocID > tB.m_tDocID )
			return false;

		return tA.m_tRowID < tB.m_tRowID;
	}

	bool IsEq ( const IDPair_t & tA, const IDPair_t & tB ) const
	{
		return tA.m_tDocID==tB.m_tDocID;
	}
};


static uint64_t CreateKey ( DocID_t tDocID, int iEntry )
{
	uint64_t uRes = sphFNV64 ( &tDocID, sizeof(tDocID) );
	return sphFNV64 ( &iEntry, sizeof(iEntry), uRes );
}


bool CSphSource_SQL::FetchJoinedFields ( CSphAutofile & tFile, CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets, CSphString & sError )
{
	if ( m_iJoinedHitField>=m_tSchema.GetFieldsCount() )
	{
		m_iJoinedHitField = -1;
		return true;
	}

	if ( !QueryPreAll ( sError ) )
		return false;

	dJoinedOffsets.Resize(m_tSchema.GetFieldsCount());
	CSphWriter tWriter;
	tWriter.SetFile ( tFile, nullptr, sError );

	bool bProcessingRanged = true;

	while ( m_iJoinedHitField<m_tSchema.GetFieldsCount() )
	{
		if ( SqlFetchRow() )
		{
			if ( !dJoinedOffsets[m_iJoinedHitField] )
				dJoinedOffsets[m_iJoinedHitField] = std::make_unique<OpenHashTable_T<uint64_t, uint64_t>>();

			auto & hOffsets = *dJoinedOffsets[m_iJoinedHitField];
			DocID_t tDocId = sphToInt64 ( SqlColumn(0) ); // FIXME! handle conversion errors and zero/max values?

			int iEntry=0;
			while ( hOffsets.Find ( CreateKey ( tDocId, iEntry ) ) )
				iEntry++;

			// add only if there's no existing entry
			hOffsets.Add ( CreateKey ( tDocId, iEntry ), tWriter.GetPos() );

			tWriter.ZipOffset(tDocId);
			tWriter.ZipInt(m_iJoinedHitField);
			if ( m_tSchema.GetField(m_iJoinedHitField).m_bPayload )
				tWriter.ZipInt ( sphToDword ( SqlColumn(2) ) );

			BYTE * pText = (BYTE *)const_cast<char*>( SqlColumn(1) );
			DWORD uLength = SqlColumnLength(1);
			tWriter.ZipInt(uLength);
			tWriter.PutBytes ( pText, uLength );
		}
		else if ( SqlIsError() )
		{
			// error while fetching row
			sError = SqlError();
			return false;

		} else
		{
			int iLastField = m_iJoinedHitField;
			bool bRanged = ( m_iJoinedHitField>=m_iPlainFieldsLength && m_iJoinedHitField<m_tSchema.GetFieldsCount()
				&& ( m_tSchema.GetField(m_iJoinedHitField).m_eSrc==SPH_ATTRSRC_RANGEDQUERY || m_tSchema.GetField(m_iJoinedHitField).m_eSrc==SPH_ATTRSRC_RANGEDMAINQUERY ) );

			// current field is over, continue to next field
			if ( m_iJoinedHitField<0 )
				m_iJoinedHitField = m_iPlainFieldsLength;
			else if ( !bRanged || !bProcessingRanged )
				m_iJoinedHitField++;

			SqlDismissResult();

			// eof check
			if ( m_iJoinedHitField>=m_tSchema.GetFieldsCount() )
			{
				m_iJoinedHitField = -1;
				return true;
			}

			bProcessingRanged = false;
			bool bCheckNumFields = true;
			const CSphColumnInfo & tJoined = m_tSchema.GetField(m_iJoinedHitField);

			bool bJoinedRanged = ( tJoined.m_eSrc==SPH_ATTRSRC_RANGEDQUERY || tJoined.m_eSrc==SPH_ATTRSRC_RANGEDMAINQUERY );
			// start fetching next field
			if ( !bJoinedRanged )
			{
				if ( !SqlQuery ( tJoined.m_sQuery.cstr() ) )
				{
					sError = SqlError();
					return false;
				}
			}
			else
			{
				m_tParams.m_iRangeStep = m_tParams.m_iRefRangeStep;

				// setup ranges for next field
				if ( iLastField!=m_iJoinedHitField )
				{
					const CSphString & sRange = ( tJoined.m_eSrc==SPH_ATTRSRC_RANGEDQUERY ? tJoined.m_sQueryRange : m_tParams.m_sQueryRange );

					CSphString sPrefix;
					sPrefix.SetSprintf ( "joined field '%s' ranged query: ", tJoined.m_sName.cstr() );
					if ( !SetupRanges ( sRange.cstr(), tJoined.m_sQuery.cstr(), sPrefix.cstr(), sError, SRE_JOINEDHITS ) )
						return false;

					m_tCurrentID = m_tMinID;
				}

				// run first step (in order to report errors)
				bool bRes = RunQueryStep ( tJoined.m_sQuery.cstr(), sError );
				bProcessingRanged = bRes; // select next documents in range or loop once to process next field
				bCheckNumFields = bRes;

				if ( !sError.IsEmpty() )
					return false;
			}

			const int iExpected = m_tSchema.GetField(m_iJoinedHitField).m_bPayload ? 3 : 2;
			if ( bCheckNumFields && SqlNumFields()!=iExpected )
			{
				const char * szName = m_tSchema.GetField(m_iJoinedHitField).m_sName.cstr();
				sError.SetSprintf ( "joined field '%s': query MUST return exactly %d columns, got %d", szName, iExpected, SqlNumFields() );
				return false;
			}
		}
	}

	m_iJoinedHitField = -1;
	return true;
}


ISphHits * CSphSource_SQL::IterateJoinedHits ( CSphReader & tReader, CSphString & sError )
{
	// iterating of joined hits happens after iterating hits from main query
	// so we may be sure at this moment no new IDs will be put in m_dAllIds
	if ( !m_bIdsSorted )
	{
		// sorted by docids, but we may have duplicates
		m_dAllIds.Sort ( CmpPairs_fn() );
		IDPair_t * pStart = m_dAllIds.Begin();
		int iLeft = sphUniq ( pStart, m_dAllIds.GetLength(), CmpPairs_fn() );
		m_dAllIds.Resize(iLeft);
		m_bIdsSorted = true;

		m_iJoinedFileSize = tReader.GetFilesize();
	}

	m_tHits.Resize(0);
	while ( m_tState.m_bProcessingHits || tReader.GetPos()<m_iJoinedFileSize )
	{
		if ( !m_tState.m_bProcessingHits )
		{
			DocID_t tDocId = tReader.UnzipOffset();
			int iField = tReader.UnzipInt();
			int iStartPos = 0;
			if ( m_tSchema.GetField(iField).m_bPayload )
				iStartPos = tReader.UnzipInt();

			DWORD uLength = tReader.UnzipInt();
			m_dJoinedField.Resize(uLength);
			tReader.GetBytes ( m_dJoinedField.Begin(), uLength );

			// lets skip joined document totally if there was no such document ID returned by main query
			const IDPair_t * pIdPair = m_dAllIds.BinarySearch ( bind ( &IDPair_t::m_tDocID ), tDocId );
			if ( !pIdPair )
				continue;

			// next field/document? reset position
			if ( tDocId!=m_iJoinedHitID || iField!=m_iJoinedHitField )
			{
				m_iJoinedHitField = iField;
				m_iJoinedHitID = tDocId;
				m_iJoinedHitPos = 0;
			}

			m_tState = CSphBuildHitsState_t();
			m_tState.m_iField = m_iJoinedHitField;
			m_tState.m_iStartField = m_iJoinedHitField;
			m_tState.m_iEndField = m_iJoinedHitField+1;

			if ( m_tSchema.GetField(m_iJoinedHitField).m_bPayload )
				m_tState.m_iStartPos = iStartPos;
			else
				m_tState.m_iStartPos = m_iJoinedHitPos;

			// build those hits
			m_pJoinedFields = m_dJoinedField.Begin();
			m_tState.m_dFields = &m_pJoinedFields;
			m_tState.m_dFieldLengths.Resize(1);
			m_tState.m_dFieldLengths[0] = uLength;

			m_tDocInfo.m_tRowID = pIdPair->m_tRowID;
		}
				
		BuildHits ( sError, true );

		// update current position
		if ( !m_tSchema.GetField(m_iJoinedHitField).m_bPayload && !m_tState.m_bProcessingHits && m_tHits.GetLength() )
			m_iJoinedHitPos = HITMAN::GetPos ( m_tHits.Last().m_uWordPos );

		if ( m_tState.m_bProcessingHits )
			break;
	}

	// eof check
	if ( !m_tState.m_bProcessingHits && tReader.GetPos()>=m_iJoinedFileSize )
		m_tDocInfo.m_tRowID = ( m_tHits.GetLength() ? 0 : INVALID_ROWID );

	return &m_tHits;
}
