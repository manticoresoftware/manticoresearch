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

#include "source_svpipe.h"

#include "indexcheck.h"
#include "schema_configurator.h"
#include "attribute.h"
#include "sphinxint.h"
#include "conversion.h"
#include "tokenizer/tokenizer.h"

class CSphSource_BaseSV : public CSphSource, public CSphSchemaConfigurator<CSphSource_BaseSV>
{
	using BASE = CSphSource;

public:
	explicit		CSphSource_BaseSV ( const char * sName );
					~CSphSource_BaseSV () override;

	bool	Connect ( CSphString & sError ) override;			///< run the command and open the pipe
	void	Disconnect () override;								///< close the pipe
	const char *	DecorateMessage ( const char * sTemplate, ... ) const __attribute__ ( ( format ( printf, 2, 3 ) ) );

	bool	IterateStart ( CSphString & ) override;				///< Connect() starts getting documents automatically, so this one is empty
	BYTE **	NextDocument ( bool & bEOF, CSphString & ) override;				///< parse incoming chunk and emit some hits
	const int *	GetFieldLengths () const override { return m_dFieldLengths.Begin(); }

	bool	IterateMultivaluedStart ( int, CSphString & ) override	{ return false; }
	bool	IterateMultivaluedNext ( int64_t &, int64_t & ) override{ return false; }
	bool	IterateKillListStart ( CSphString & ) override			{ return false; }
	bool	IterateKillListNext ( DocID_t & ) override				{ return false; }

	void	Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings ) override;
	bool	SetupPipe ( const CSphConfigSection & hSource, FILE * pPipe, CSphString & sError );

protected:
	enum ESphParseResult
	{
		PARSING_FAILED,
		GOT_DOCUMENT,
		DATA_OVER
	};

	CSphVector<BYTE>			m_dBuf;
	CSphFixedVector<char>		m_dError {1024};
	CSphFixedVector<int>		m_dColumnsLen {0};
	CSphVector<RemapXSV_t>		m_dRemap;

	// output
	CSphFixedVector<BYTE *>		m_dFields {0};
	CSphFixedVector<int>		m_dFieldLengths {0};

	FILE *						m_pFP = nullptr;
	int							m_iDataStart = 0;	///< where the next line to parse starts in m_dBuf
	int							m_iDocStart = 0;	///< where the last parsed document stats in m_dBuf
	int							m_iBufUsed = 0;		///< bytes [0,m_iBufUsed) are actually currently used; the rest of m_dBuf is free
	int							m_iLine = 0;

	BYTE **					ReportDocumentError();
	virtual bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) = 0;
	virtual ESphParseResult	SplitColumns ( CSphString & ) = 0;

private:
	bool	StoreAttribute ( int iAttr, int iOff );
};


class CSphSource_TSV : public CSphSource_BaseSV
{
	using CSphSource_BaseSV::CSphSource_BaseSV;

public:
	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
};


class CSphSource_CSV : public CSphSource_BaseSV
{
public:
	explicit		CSphSource_CSV ( const char * sName );

	ESphParseResult	SplitColumns ( CSphString & sError ) final;					///< parse incoming chunk and emit some hits
	bool			SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError ) final;
	void			SetDelimiter ( const char * sDelimiter );

private:
	BYTE			m_iDelimiter;
};


CSphSource * sphCreateSourceTSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName )
{
	CSphString sError;
	auto * pTSV = new CSphSource_TSV(sSourceName);
	if ( !pTSV->SetupPipe ( *pSource, pPipe, sError ) )
	{
		SafeDelete ( pTSV );
		fprintf ( stdout, "ERROR: tsvpipe: %s", sError.cstr() );
	}

	return pTSV;
}


CSphSource * sphCreateSourceCSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName )
{
	CSphString sError;
	auto sDelimiter = pSource->GetStr ( "csvpipe_delimiter" );
	auto * pCSV = new CSphSource_CSV(sSourceName);
	pCSV->SetDelimiter ( sDelimiter.cstr() );
	if ( !pCSV->SetupPipe ( *pSource, pPipe, sError ) )
	{
		SafeDelete ( pCSV );
		fprintf ( stdout, "ERROR: csvpipe: %s", sError.cstr() );
	}

	return pCSV;
}


CSphSource_BaseSV::CSphSource_BaseSV ( const char * sName )
	: CSphSource ( sName )
	{}



CSphSource_BaseSV::~CSphSource_BaseSV ()
{
	Disconnect();
}


bool CSphSource_BaseSV::SetupPipe ( const CSphConfigSection & hSource, FILE * pPipe, CSphString & sError )
{
	m_pFP = pPipe;
	m_tSchema.Reset ();
	bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );

	if ( !SetupSchema ( hSource, bWordDict, sError ) )
		return false;

	if ( !DebugCheckSchema ( m_tSchema, sError ) )
		return false;

	if ( !AddAutoAttrs ( sError ) )
		return false;

	int nFields = m_tSchema.GetFieldsCount();
	m_dFields.Reset ( nFields );
	m_dFieldLengths.Reset ( nFields );

	// build hash from schema names
	SmallStringHash_T<RemapXSV_t> hSchema;

	for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		RemapXSV_t tField;
		tField.m_iField = i;
		hSchema.Add ( tField, m_tSchema.GetFieldName(i) );
	}

	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphString & sAttrName = m_tSchema.GetAttr(i).m_sName;
		RemapXSV_t * pRemap = hSchema ( sAttrName );
		if ( pRemap )
			pRemap->m_iAttr = i;
		else
		{
			RemapXSV_t tAttr;
			tAttr.m_iAttr = i;
			hSchema.Add ( tAttr, sAttrName );
		}
	}

	// restore order for declared columns
	CSphString sColumn;
	for ( const auto& tVal : hSource )
	{
		const CSphVariant * pVal = &tVal.second;
		while ( pVal )
		{
			sColumn = pVal->strval();
			// uint attribute might have bit count that should by cut off from name
			const char * pColon = strchr ( sColumn.cstr(), ':' );
			if ( pColon )
			{
				int iColon = int ( pColon-sColumn.cstr() );
				CSphString sTmp;
				sTmp.SetBinary ( sColumn.cstr(), iColon );
				sColumn.Swap ( sTmp );
			}

			// let's handle different char cases
			sColumn.ToLower();

			RemapXSV_t * pColumn = hSchema ( sColumn );
			assert ( !pColumn || pColumn->m_iAttr>=0 || pColumn->m_iField>=0 );
			assert ( !pColumn || pColumn->m_iTag==-1 );
			if ( pColumn )
				pColumn->m_iTag = pVal->m_iTag;

			pVal = pVal->m_pNext;
		}
	}

	RemapXSV_t * pIdCol = hSchema ( sphGetDocidName() );
	assert ( pIdCol && pIdCol->m_iTag==-1 && pIdCol->m_iAttr==0 );
	pIdCol->m_iTag = 0;

	for ( const auto& tCol : hSchema )
		if ( tCol.second.m_iTag>=0 )
			m_dRemap.Add ( tCol.second );

	m_dColumnsLen.Reset ( m_dRemap.GetLength() );

	sphSort ( m_dRemap.Begin(), m_dRemap.GetLength(), bind ( &RemapXSV_t::m_iTag ) );

	return true;
}


void CSphSource_BaseSV::Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	// detect a case when blob row locator was removed (because all blobs became columnar)
	bool bHadLocator = !!m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	BASE::Setup ( tSettings, pWarnings );
	bool bHaveLocator = !!m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	if ( bHadLocator && !bHaveLocator )
	{
		const int iBlobLocatorId = 1;
		for ( auto & i : m_dRemap )
			if ( i.m_iAttr>iBlobLocatorId )
				i.m_iAttr--;
	}
}


bool CSphSource_BaseSV::Connect ( CSphString & sError )
{
	// source settings have been updated after ::Setup
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		ESphWordpart eWordpart = GetWordpart ( m_tSchema.GetFieldName(i), m_pDict && m_pDict->GetSettings().m_bWordDict );
		m_tSchema.SetFieldWordpart ( i, eWordpart );
	}

	if ( !AddAutoAttrs ( sError ) )
		return false;

	AllocDocinfo();

	m_tHits.Reserve ( m_iMaxHits );
	m_dBuf.Resize ( DEFAULT_READ_BUFFER );

	return true;
}


void CSphSource_BaseSV::Disconnect()
{
	if ( m_pFP )
	{
		pclose ( m_pFP );
		m_pFP = nullptr;
	}

	m_tHits.Reset();
}


const char * CSphSource_BaseSV::DecorateMessage ( const char * sTemplate, ... ) const
{
	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( m_dError.Begin (), m_dError.GetLength (), sTemplate, ap );
	va_end ( ap );
	return m_dError.Begin();
}

static const BYTE g_dBOM[] = { 0xEF, 0xBB, 0xBF };

bool CSphSource_BaseSV::IterateStart ( CSphString & sError )
{

	m_iLine = 0;
	m_iDataStart = 0;

	// initial buffer update
	m_iBufUsed = (int) fread ( m_dBuf.Begin(), 1, m_dBuf.GetLength(), m_pFP );
	if ( !m_iBufUsed )
	{
		sError.SetSprintf ( "source '%s': read error '%s'", m_tSchema.GetName(), strerrorm(errno) );
		return false;
	}
	m_iPlainFieldsLength = m_tSchema.GetFieldsCount();

	// space out BOM like xml-pipe does
	if ( m_iBufUsed>(int)sizeof(g_dBOM) && memcmp ( m_dBuf.Begin(), g_dBOM, sizeof ( g_dBOM ) )==0 )
		memset ( m_dBuf.Begin(), ' ', sizeof(g_dBOM) );
	return true;
}

BYTE ** CSphSource_BaseSV::ReportDocumentError ()
{
	m_tDocInfo.m_tRowID = 0; // INVALID_ROWID means legal eof
	m_iDataStart = 0;
	m_iBufUsed = 0;
	return nullptr;
}


bool CSphSource_BaseSV::StoreAttribute ( int iAttr, int iOff )
{
	// if+if for field-string attribute case
	const RemapXSV_t & tRemap = m_dRemap[iAttr];

	// field column
	if ( tRemap.m_iField!=-1 )
	{
		m_dFields[tRemap.m_iField] = m_dBuf.Begin() + iOff;
		m_dFieldLengths[tRemap.m_iField] = (int) strlen ( (char *)m_dFields[tRemap.m_iField] );
	}

	// attribute column
	if ( tRemap.m_iAttr==-1 )
		return true;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr ( tRemap.m_iAttr );
	const char * sVal = (const char *)m_dBuf.Begin() + iOff;

	CSphString & sCurStrAttr = m_dStrAttrs[tRemap.m_iAttr];
	SphAttr_t & tCurIntAttr = m_dAttrs[tRemap.m_iAttr];

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_JSON:
		sCurStrAttr = sVal;
		break;

	case SPH_ATTR_FLOAT:
		{
			float fValue = sphToFloat(sVal);
			tCurIntAttr = sphF2DW(fValue);

			if ( !tAttr.IsColumnar() )
				m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, fValue );
		}
		break;

	case SPH_ATTR_BIGINT:
		{
			CSphString sWarn;
			if ( tRemap.m_iAttr )
			{
				tCurIntAttr = sphToInt64 ( sVal, &sWarn );
				if ( !sWarn.IsEmpty() )
					sphWarn ( "%s", sWarn.cstr() );
			}
			else
			{
				tCurIntAttr = (int64_t)StrToDocID ( sVal, sWarn );
				if ( !sWarn.IsEmpty() )
				{
					sphWarn ( "%s", sWarn.cstr() );
					return false;
				}
			}

			if ( !tAttr.IsColumnar() )
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		}
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		ParseFieldMVA ( tRemap.m_iAttr, sVal );
		break;

	case SPH_ATTR_TOKENCOUNT:
		m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
		break;

	case SPH_ATTR_BOOL:
		tCurIntAttr = sphToDword(sVal) ? 1 : 0;
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		break;

	default:
		tCurIntAttr = sphToDword(sVal);
		if ( !tAttr.IsColumnar() )
			m_tDocInfo.SetAttr ( tAttr.m_tLocator, tCurIntAttr );
		break;
	}

	return true;
}


BYTE **	CSphSource_BaseSV::NextDocument ( bool & bEOF, CSphString & sError )
{
	bEOF = false;

	bool bSkipDoc = false;
	do
	{
		ESphParseResult eRes = SplitColumns ( sError );
		if ( eRes==PARSING_FAILED )
			return ReportDocumentError();
		else if ( eRes==DATA_OVER )
		{
			bEOF = true;
			return nullptr;
		}

		assert ( eRes==GOT_DOCUMENT );

		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & i : m_dMvas )
			i.Resize(0);

		int iOff = m_iDocStart;

		bSkipDoc = false;
		ARRAY_FOREACH ( i, m_dRemap )
		{
			if ( !StoreAttribute ( i, iOff ) )
			{
				bSkipDoc = true;
				break;
			}

			iOff += m_dColumnsLen[i] + 1; // length of value plus null-terminator
		}

		m_iLine++;
	}
	while ( bSkipDoc );

	return m_dFields.Begin();
}


CSphSource_BaseSV::ESphParseResult CSphSource_TSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	BYTE * pData = m_dBuf.Begin() + m_iDataStart;
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed;
	m_iDocStart = m_iDataStart;

	while (true)
	{
		if ( iCol>=iColumns )
		{
			sError.SetSprintf ( "source '%s': too many columns found (found=%d, declared=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}

		// move to next control symbol
		while ( pData<pEnd && *pData && *pData!='\t' && *pData!='\r' && *pData!='\n' )
			pData++;

		if ( pData<pEnd )
		{
			assert ( *pData=='\t' || !*pData || *pData=='\r' || *pData=='\n' );
			bool bNull = !*pData;
			bool bEOL = ( *pData=='\r' || *pData=='\n' );

			int iLen = int ( pData - m_dBuf.Begin() ) - iColumnStart;
			assert ( iLen>=0 );
			m_dColumnsLen[iCol] = iLen;
			*pData++ = '\0';
			iCol++;

			if ( bNull )
			{
				// null terminated string found
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL )
			{
				// end of document found
				// skip all EOL characters
				while ( pData<pEnd && *pData && ( *pData=='\r' || *pData=='\n' ) )
					pData++;
				break;
			}

			// column separator found
			iColumnStart = int ( pData - m_dBuf.Begin() );
			continue;
		}

		int iOff = int ( pData - m_dBuf.Begin() );

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		auto iGot = (int) fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			// error in case no data left in middle of data stream
			sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restored pointers after buffer resize
		pData = m_dBuf.Begin() + iOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( pData<=pEnd );
	m_iDataStart = int ( pData - m_dBuf.Begin() );
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_TSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("tsvpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	ConfigureFields ( hSource("tsvpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("tsvpipe_field_string"), bWordDict, m_tSchema );

	return true;
}


CSphSource_CSV::CSphSource_CSV ( const char * sName )
	: CSphSource_BaseSV ( sName )
{
	m_iDelimiter = BYTE ( ',' );
}


CSphSource_BaseSV::ESphParseResult CSphSource_CSV::SplitColumns ( CSphString & sError )
{
	int iColumns = m_dRemap.GetLength();
	int iCol = 0;
	int iColumnStart = m_iDataStart;
	int iQuotPrev = -1;
	int	iEscapeStart = -1;
	const BYTE * s = m_dBuf.Begin() + m_iDataStart; // parse this line
	BYTE * d = m_dBuf.Begin() + m_iDataStart; // do parsing in place
	const BYTE * pEnd = m_dBuf.Begin() + m_iBufUsed; // until we reach the end of current buffer
	m_iDocStart = m_iDataStart;
	bool bOnlySpace = true;
	bool bQuoted = false;
	bool bHasQuot = false;

	while (true)
	{
		assert ( d<=s );

		// move to next control symbol
		while ( s<pEnd && *s && *s!=m_iDelimiter && *s!='"' && *s!='\\' && *s!='\r' && *s!='\n' )
		{
			bOnlySpace &= sphIsSpace ( *s );
			*d++ = *s++;
		}

		if ( s<pEnd )
		{
			assert ( !*s || *s==m_iDelimiter || *s=='"' || *s=='\\' || *s=='\r' || *s=='\n' );
			bool bNull = !*s;
			bool bEOL = ( *s=='\r' || *s=='\n' );
			bool bDelimiter = ( *s==m_iDelimiter );
			bool bQuot = ( *s=='"' );
			bool bEscape = ( *s=='\\' );
			int iOff = int ( s - m_dBuf.Begin() );
			bool bEscaped = ( iEscapeStart>=0 && iEscapeStart+1==iOff );

			// escape symbol outside double quotation
			if ( !bQuoted && !bDelimiter && ( bEscape || bEscaped ) )
			{
				if ( bEscaped ) // next to escape symbol proceed as regular
				{
					*d++ = *s++;
				} else // escape just started
				{
					iEscapeStart = iOff;
					s++;
				}
				continue;
			}

			// double quote processing
			// [ " ... " ]
			// [ " ... "" ... " ]
			// [ " ... """ ]
			// [ " ... """" ... " ]
			// any symbol inside double quote proceed as regular
			// but quoted quote proceed as regular symbol
			if ( bQuot )
			{
				if ( bOnlySpace && iQuotPrev==-1 )
				{
					// enable double quote
					bQuoted = true;
					bHasQuot = true;
				} else if ( bQuoted )
				{
					// close double quote on 2st quote symbol
					bQuoted = false;
				} else if ( bHasQuot && iQuotPrev!=-1 && iQuotPrev+1==iOff )
				{
					// escaped quote found, re-enable double quote and copy symbol itself
					bQuoted = true;
					*d++ = '"';
				} else
				{
					*d++ = *s;
				}

				s++;
				iQuotPrev = iOff;
				continue;
			}

			if ( bQuoted )
			{
				*d++ = *s++;
				continue;
			}

			int iLen = int ( d - m_dBuf.Begin() - iColumnStart );
			assert ( iLen>=0 );
			if ( iCol<m_dColumnsLen.GetLength() )
				m_dColumnsLen[iCol] = iLen;
			*d++ = '\0';
			s++;
			iCol++;

			if ( bNull ) // null terminated string found
			{
				m_iDataStart = m_iBufUsed = 0;
				break;
			} else if ( bEOL ) // end of document found
			{
				// skip all EOL characters
				while ( s<pEnd && *s && ( *s=='\r' || *s=='\n' ) )
					s++;
				break;
			}

			assert ( bDelimiter );
			// column separator found
			iColumnStart = int ( d - m_dBuf.Begin() );
			bOnlySpace = true;
			bQuoted = false;
			bHasQuot = false;
			iQuotPrev = -1;
			continue;
		}

		/////////////////////
		// read in more data
		/////////////////////

		int iDstOff = int ( s - m_dBuf.Begin() );
		int iSrcOff = int ( d - m_dBuf.Begin() );

		// if there is space at the start, move data around
		// if not, resize the buffer
		if ( m_iDataStart>0 )
		{
			memmove ( m_dBuf.Begin(), m_dBuf.Begin() + m_iDataStart, m_iBufUsed - m_iDataStart );
			m_iBufUsed -= m_iDataStart;
			iDstOff -= m_iDataStart;
			iSrcOff -= m_iDataStart;
			iColumnStart -= m_iDataStart;
			if ( iQuotPrev!=-1 )
				iQuotPrev -= m_iDataStart;
			iEscapeStart -= m_iDataStart;
			m_iDataStart = 0;
			m_iDocStart = 0;
		} else if ( m_iBufUsed==m_dBuf.GetLength() )
		{
			m_dBuf.Resize ( m_dBuf.GetLength()*2 );
		}

		// do read
		auto iGot = (int) fread ( m_dBuf.Begin() + m_iBufUsed, 1, m_dBuf.GetLength() - m_iBufUsed, m_pFP );
		if ( !iGot )
		{
			if ( !iCol )
			{
				// normal file termination - no pending columns and documents
				m_iDataStart = m_iBufUsed = 0;
				m_tDocInfo.m_tRowID = INVALID_ROWID;
				return CSphSource_BaseSV::DATA_OVER;
			}

			if ( iCol!=iColumns )
				sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d, error='%s')",	m_tSchema.GetName(), iCol, iColumns, m_iLine, strerror(errno) );
			else
			{
				// error in case no data left in middle of data stream
				sError.SetSprintf ( "source '%s': read error '%s' (line=%d)", m_tSchema.GetName(), strerror(errno), m_iLine );
			}
			return CSphSource_BaseSV::PARSING_FAILED;
		}
		m_iBufUsed += iGot;

		// restore pointers because of the resize
		s = m_dBuf.Begin() + iDstOff;
		d = m_dBuf.Begin() + iSrcOff;
		pEnd = m_dBuf.Begin() + m_iBufUsed;

		// skip all EOL characters left from previous row
		if ( !iCol )
		{
			while ( s<pEnd && *s && ( *s=='\r' || *s=='\n' ) )
				s++;
		}
	}

	// all columns presence check
	if ( iCol!=iColumns )
	{
		sError.SetSprintf ( "source '%s': not all columns found (found=%d, total=%d, line=%d)", m_tSchema.GetName(), iCol, iColumns, m_iLine );
		return CSphSource_BaseSV::PARSING_FAILED;
	}

	// tail data
	assert ( s<=pEnd );
	m_iDataStart = int ( s - m_dBuf.Begin() );
	return CSphSource_BaseSV::GOT_DOCUMENT;
}


bool CSphSource_CSV::SetupSchema ( const CSphConfigSection & hSource, bool bWordDict, CSphString & sError )
{
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("csvpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	ConfigureFields ( hSource("csvpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("csvpipe_field_string"), bWordDict, m_tSchema );

	return true;
}


void CSphSource_CSV::SetDelimiter ( const char * sDelimiter )
{
	if ( sDelimiter && *sDelimiter )
		m_iDelimiter = *sDelimiter;
}

