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

#include <config_indexer.h>

#include "source_xmlpipe2.h"
#include "indexcheck.h"
#include "schema_configurator.h"
#include "attribute.h"
#include "conversion.h"
#include "dict/dict_base.h"

#define XMLIMPORT
#include "expat.h"

// workaround for expat versions prior to 1.95.7
#ifndef XMLCALL
#define XMLCALL
#endif

#if WITH_ICONV
#include "iconv.h"
#endif


/////////////////////////////////////////////////////////////////////////////
// XMLPIPE (v2)
/////////////////////////////////////////////////////////////////////////////

#ifndef EXPAT_LIB
#define EXPAT_LIB nullptr
#endif
static const char * GET_EXPAT_LIB ()
{
	const char * szEnv = getenv ( "EXPAT_LIB" );
	if ( szEnv )
		return szEnv;
	return EXPAT_LIB;
}

#ifndef ICONV_LIB
#define ICONV_LIB nullptr
#endif
static const char * GET_ICONV_LIB ()
{
	const char * szEnv = getenv ( "ICONV_LIB" );
	if ( szEnv )
		return szEnv;
	return ICONV_LIB;
}

#if DL_EXPAT

	static decltype (&XML_ParserFree) sph_XML_ParserFree = nullptr;
	static decltype (&XML_Parse) sph_XML_Parse = nullptr;
	static decltype (&XML_GetCurrentColumnNumber) sph_XML_GetCurrentColumnNumber = nullptr;
	static decltype (&XML_GetCurrentLineNumber) sph_XML_GetCurrentLineNumber = nullptr;
	static decltype (&XML_GetErrorCode) sph_XML_GetErrorCode = nullptr;
	static decltype (&XML_ErrorString) sph_XML_ErrorString = nullptr;
	static decltype (&XML_ParserCreate) sph_XML_ParserCreate = nullptr;
	static decltype (&XML_SetUserData) sph_XML_SetUserData = nullptr;
	static decltype (&XML_SetElementHandler) sph_XML_SetElementHandler = nullptr;
	static decltype (&XML_SetCharacterDataHandler) sph_XML_SetCharacterDataHandler = nullptr;
	static decltype (&XML_SetUnknownEncodingHandler) sph_XML_SetUnknownEncodingHandler = nullptr;
	static bool InitDynamicExpat ()
	{
		const char * sFuncs[] = { "XML_ParserFree", "XML_Parse",
				"XML_GetCurrentColumnNumber", "XML_GetCurrentLineNumber", "XML_GetErrorCode", "XML_ErrorString",
				"XML_ParserCreate", "XML_SetUserData", "XML_SetElementHandler", "XML_SetCharacterDataHandler",
				"XML_SetUnknownEncodingHandler" };

		void ** pFuncs[] = { (void **) & sph_XML_ParserFree, (void **) & sph_XML_Parse,
				(void **) & sph_XML_GetCurrentColumnNumber, (void **) & sph_XML_GetCurrentLineNumber,
				(void **) & sph_XML_GetErrorCode, (void **) & sph_XML_ErrorString,
				(void **) & sph_XML_ParserCreate, (void **) & sph_XML_SetUserData,
				(void **) & sph_XML_SetElementHandler, (void **) & sph_XML_SetCharacterDataHandler,
				(void **) & sph_XML_SetUnknownEncodingHandler };

		static CSphDynamicLibrary dLib ( GET_EXPAT_LIB() );
		return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) );
	}

#else
	#define sph_XML_ParserFree XML_ParserFree
	#define sph_XML_Parse XML_Parse
	#define sph_XML_GetCurrentColumnNumber XML_GetCurrentColumnNumber
	#define sph_XML_GetCurrentLineNumber XML_GetCurrentLineNumber
	#define sph_XML_GetErrorCode XML_GetErrorCode
	#define sph_XML_ErrorString XML_ErrorString
	#define sph_XML_ParserCreate XML_ParserCreate
	#define sph_XML_SetUserData XML_SetUserData
	#define sph_XML_SetElementHandler XML_SetElementHandler
	#define sph_XML_SetCharacterDataHandler XML_SetCharacterDataHandler
	#define sph_XML_SetUnknownEncodingHandler XML_SetUnknownEncodingHandler
	#define InitDynamicExpat() (true)
#endif

#if WITH_ICONV
	#if DL_ICONV
		static decltype (&iconv) sph_iconv = nullptr;
		static decltype (&iconv_close) sph_iconv_close = nullptr;
		static decltype (&iconv_open) sph_iconv_open = nullptr;

		static bool InitDynamicIconv ()
		{
			const char * sFuncs[] = { "iconv", "iconv_close", "iconv_open" };
			void ** pFuncs[] = { (void **) & sph_iconv, (void **) & sph_iconv_close, (void **) & sph_iconv_open, };

			static CSphDynamicLibrary dLib ( GET_ICONV_LIB() );
			return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void ** ) );
		}
	#else
		#define sph_iconv iconv
		#define sph_iconv_close iconv_close
		#define sph_iconv_open iconv_open
		#define InitDynamicIconv() (true)
	#endif
#else
	#define InitDynamicIconv() (true)
#endif

/// XML pipe source implementation (v2)
class CSphSource_XMLPipe2 final : public CSphSource, public CSphSchemaConfigurator<CSphSource_XMLPipe2>
{
public:
	explicit		CSphSource_XMLPipe2 ( const char * sName );
					~CSphSource_XMLPipe2 () final;

	bool			SetupXML ( int iFieldBufferMax, bool bFixupUTF8, FILE * pPipe, const CSphConfigSection & hSource, CSphString & sError );			///< memorize the command
	bool			Connect ( CSphString & sError ) final;			///< run the command and open the pipe
	void			Disconnect () final;							///< close the pipe

	bool			IterateStart ( CSphString & ) final { m_iPlainFieldsLength = m_tSchema.GetFieldsCount(); return true; }	///< Connect() starts getting documents automatically, so this one is empty
	BYTE **			NextDocument ( bool & bEOF, CSphString & sError ) final;			///< parse incoming chunk and emit some hits
	const int *		GetFieldLengths () const final { return m_dFieldLengths.Begin(); }

	bool			IterateMultivaluedStart ( int, CSphString & ) final{ return false; }
	bool			IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) final { return false; }
	bool			IterateKillListStart ( CSphString & ) final;
	bool			IterateKillListNext ( DocID_t & tDocId ) final;

	void			StartElement ( const char * szName, const char ** pAttrs );
	void			EndElement ( const char * szName );
	void			Characters ( const char * pCharacters, int iLen );

	void			Error ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	const char *	DecorateMessage ( const char * sTemplate, ... ) const __attribute__ ( ( format ( printf, 2, 3 ) ) );
	const char *	DecorateMessageVA ( const char * sTemplate, va_list ap ) const;

private:
	struct Document_t
	{
		DocID_t							m_tDocID;
		CSphVector < CSphVector<BYTE> >	m_dFields;
		StrVec_t						m_dAttrs;
	};

	Document_t *		m_pCurDocument = nullptr;
	CSphVector<Document_t *>	m_dParsedDocuments;

	FILE *				m_pPipe = nullptr;			///< incoming stream
	CSphString			m_sError;
	CSphString			m_sDocIDError;
	StrVec_t			m_dDefaultAttrs;
	StrVec_t			m_dInvalid;
	StrVec_t			m_dWarned;
	int					m_iElementDepth = 0;

	BYTE *				m_pBuffer = nullptr;
	int					m_iBufferSize = 1048576;

	CSphVector<BYTE*>	m_dFieldPtrs;
	CSphVector<int>		m_dFieldLengths;
	bool				m_bRemoveParsed = false;

	bool				m_bInDocset = false;
	bool				m_bInSchema = false;
	bool				m_bInDocument = false;
	bool				m_bInKillList = false;
	bool				m_bInId = false;
	bool				m_bInIgnoredTag = false;
	bool				m_bFirstTagAfterDocset = false;

	int					m_iKillListIterator = 0;
	CSphVector<DocID_t> m_dKillList;

	int					m_iCurField = -1;
	int					m_iCurAttr = -1;

	XML_Parser			m_pParser {nullptr};

	int					m_iFieldBufferMax = 65536;
	BYTE * 				m_pFieldBuffer = nullptr;
	int					m_iFieldBufferLen = 0;

	bool				m_bFixupUTF8 = false;	///< whether to replace invalid utf-8 codepoints with spaces
	int					m_iReparseStart = 0;	///< utf-8 fixerupper might need to postpone a few bytes, starting at this offset
	int					m_iReparseLen = 0;		///< and this much bytes (under 4)

	void				UnexpectedCharaters ( const char * pCharacters, int iLen, const char * szComment );

	bool				ParseNextChunk ( int iBufferLen, CSphString & sError );

	void DocumentError ( const char * sWhere )
	{
		Error ( "malformed source, <sphinx:document> found inside %s", sWhere );

		// Ideally I'd like to display a notice on the next line that
		// would say where exactly it's allowed. E.g.:
		//
		// <sphinx:document> must be contained in <sphinx:docset>
	}
};


// callbacks
static void XMLCALL xmlStartElement ( void * user_data, const XML_Char * name, const XML_Char ** attrs )
{
	auto * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->StartElement ( name, attrs );
}


static void XMLCALL xmlEndElement ( void * user_data, const XML_Char * name )
{
	auto * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->EndElement ( name );
}


static void XMLCALL xmlCharacters ( void * user_data, const XML_Char * ch, int len )
{
	auto * pSource = (CSphSource_XMLPipe2 *) user_data;
	pSource->Characters ( ch, len );
}

#if WITH_ICONV
static int XMLCALL xmlUnknownEncoding ( void *, const XML_Char * name, XML_Encoding * info )
{
	iconv_t pDesc = sph_iconv_open ( "UTF-16", name );
	if ( !pDesc )
		return XML_STATUS_ERROR;

	for ( size_t i = 0; i < 256; i++ )
	{
		char cIn = (char) i;
		char dOut[4];
		memset ( dOut, 0, sizeof ( dOut ) );
#if ICONV_INBUF_CONST
		const
#endif
				char * pInbuf = &cIn;
		char * pOutbuf = dOut;
		size_t iInBytesLeft = 1;
		size_t iOutBytesLeft = 4;

		if ( sph_iconv ( pDesc, &pInbuf, &iInBytesLeft, &pOutbuf, &iOutBytesLeft )!=size_t(-1) )
			info->map[i] = int ( BYTE ( dOut[0] ) ) << 8 | int ( BYTE ( dOut[1] ) );
		else
			info->map[i] = 0;
	}

	sph_iconv_close ( pDesc );

	return XML_STATUS_OK;
}
#endif

CSphSource_XMLPipe2::CSphSource_XMLPipe2 ( const char * sName )
	: CSphSource ( sName )
{}


CSphSource_XMLPipe2::~CSphSource_XMLPipe2 ()
{
	Disconnect ();
	SafeDeleteArray ( m_pBuffer );
	SafeDeleteArray ( m_pFieldBuffer );
	for ( auto& i: m_dParsedDocuments )
		SafeDelete ( i );
}


void CSphSource_XMLPipe2::Disconnect ()
{
	if ( m_pPipe )
	{
		pclose ( m_pPipe );
		m_pPipe = NULL;
	}

	if ( m_pParser )
	{
		sph_XML_ParserFree ( m_pParser );
		m_pParser = NULL;
	}

	m_tHits.Reset();
}


void CSphSource_XMLPipe2::Error ( const char * sTemplate, ... )
{
	if ( !m_sError.IsEmpty() )
		return;

	va_list ap;
	va_start ( ap, sTemplate );
	m_sError = DecorateMessageVA ( sTemplate, ap );
	va_end ( ap );
}


const char * CSphSource_XMLPipe2::DecorateMessage ( const char * sTemplate, ... ) const
{
	va_list ap;
	va_start ( ap, sTemplate );
	const char * sRes = DecorateMessageVA ( sTemplate, ap );
	va_end ( ap );
	return sRes;
}


const char * CSphSource_XMLPipe2::DecorateMessageVA ( const char * sTemplate, va_list ap ) const
{
	static char sBuf[1024];

	snprintf ( sBuf, sizeof(sBuf), "source '%s': ", m_tSchema.GetName() );
	auto iBufLen = strlen ( sBuf );
	auto iLeft = sizeof(sBuf) - iBufLen;
	char * szBufStart = sBuf + iBufLen;

	vsnprintf ( szBufStart, iLeft, sTemplate, ap );
	iBufLen = strlen ( sBuf );
	iLeft = sizeof(sBuf) - iBufLen;
	szBufStart = sBuf + iBufLen;

	if ( m_pParser )
	{
		DocID_t tFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			tFailedID = m_dParsedDocuments.Last()->m_tDocID;

		snprintf ( szBufStart, iLeft, " (line=%d, pos=%d, docid=" INT64_FMT ")",
			(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
			tFailedID );
	}

	return sBuf;
}


bool CSphSource_XMLPipe2::SetupXML ( int iFieldBufferMax, bool bFixupUTF8, FILE * pPipe, const CSphConfigSection & hSource, CSphString & sError )
{
	assert ( !m_pBuffer && !m_pFieldBuffer && !m_pPipe );

	m_pBuffer = new BYTE [m_iBufferSize];
	m_iFieldBufferMax = Max ( iFieldBufferMax, 65536 );
	m_pFieldBuffer = new BYTE [ m_iFieldBufferMax+1 ]; // safe gap for tail zero
	m_bFixupUTF8 = bFixupUTF8;
	m_pPipe = pPipe;
	m_tSchema.Reset ();
	bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );
	bool bOk = true;

	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_uint"),		SPH_ATTR_INTEGER,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_timestamp"),	SPH_ATTR_TIMESTAMP,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_bool"),		SPH_ATTR_BOOL,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_float"),		SPH_ATTR_FLOAT,		m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_bigint"),		SPH_ATTR_BIGINT,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_multi"),		SPH_ATTR_UINT32SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_multi_64"),	SPH_ATTR_INT64SET,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_string"),		SPH_ATTR_STRING,	m_tSchema, sError );
	bOk &= ConfigureAttrs ( hSource("xmlpipe_attr_json"),		SPH_ATTR_JSON,		m_tSchema, sError );

	bOk &= ConfigureAttrs ( hSource("xmlpipe_field_string"),	SPH_ATTR_STRING,	m_tSchema, sError );

	if ( !bOk )
		return false;

	if ( !DebugCheckSchema ( m_tSchema, sError ) )
		return false;

	ConfigureFields ( hSource("xmlpipe_field"), bWordDict, m_tSchema );
	ConfigureFields ( hSource("xmlpipe_field_string"), bWordDict, m_tSchema );

	AllocDocinfo();
	return true;
}


bool CSphSource_XMLPipe2::Connect ( CSphString & sError )
{
	assert ( m_pBuffer && m_pFieldBuffer );

	// source settings have been updated after ::Setup
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); ++i )
	{
		ESphWordpart eWordpart = GetWordpart ( m_tSchema.GetFieldName(i), m_pDict && m_pDict->GetSettings().m_bWordDict );
		m_tSchema.SetFieldWordpart ( i, eWordpart );
	}

	if_const ( !InitDynamicExpat() )
	{
		sError.SetSprintf ( "xmlpipe: failed to load libexpat library (tried %s)\n", GET_EXPAT_LIB() );
		return false;
	}

	if_const ( !InitDynamicIconv() )
	{
		sError.SetSprintf ( "xmlpipe: failed to load iconv library (tried %s)\n", GET_ICONV_LIB() );
		return false;
	}

	if ( !AddAutoAttrs ( sError ) )
		return false;

	AllocDocinfo();

	m_pParser = sph_XML_ParserCreate(nullptr);
	if ( !m_pParser )
	{
		sError.SetSprintf ( "xmlpipe: failed to create XML parser" );
		return false;
	}

	sph_XML_SetUserData ( m_pParser, this );
	sph_XML_SetElementHandler ( m_pParser, xmlStartElement, xmlEndElement );
	sph_XML_SetCharacterDataHandler ( m_pParser, xmlCharacters );

#if WITH_ICONV
	sph_XML_SetUnknownEncodingHandler ( m_pParser, xmlUnknownEncoding, nullptr );
#endif

	m_dKillList.Reserve ( 1024 );
	m_dKillList.Resize ( 0 );

	m_bRemoveParsed = false;
	m_bInDocset = false;
	m_bInSchema = false;
	m_bInDocument = false;
	m_bInKillList = false;
	m_bInId = false;
	m_bFirstTagAfterDocset = false;
	m_iCurField = -1;
	m_iCurAttr = -1;
	m_iElementDepth = 0;

	m_dParsedDocuments.Reset ();
	m_dDefaultAttrs.Reset ();
	m_dInvalid.Reset ();
	m_dWarned.Reset ();

	m_dParsedDocuments.Reserve ( 1024 );
	m_dParsedDocuments.Resize ( 0 );

	m_iKillListIterator = 0;

	m_sError = "";

	auto iBytesRead = (int) fread ( m_pBuffer, 1, m_iBufferSize, m_pPipe );

	if ( !ParseNextChunk ( iBytesRead, sError ) )
		return false;

	m_tHits.Reserve ( m_iMaxHits );

	return true;
}


bool CSphSource_XMLPipe2::ParseNextChunk ( int iBufferLen, CSphString & sError )
{
	if ( !iBufferLen )
		return true;

	bool bLast = ( iBufferLen!=m_iBufferSize );

	m_iReparseLen = 0;
	if ( m_bFixupUTF8 )
	{
		BYTE * p = m_pBuffer;
		BYTE * pMax = m_pBuffer + iBufferLen;

		while ( p<pMax )
		{
			BYTE v = *p;

			// fix control codes
			if ( v<0x20 && v!=0x0D && v!=0x0A )
			{
				*p++ = ' ';
				continue;
			}

			// accept ascii7 codes
			if ( v<128 )
			{
				p++;
				continue;
			}

			// remove invalid start bytes
			if ( v<0xC2 )
			{
				*p++ = ' ';
				continue;
			}

			// get and check byte count
			int iBytes = 0;
			while ( v & 0x80 )
			{
				iBytes++;
				v <<= 1;
			}
			if ( iBytes<2 || iBytes>3 )
			{
				*p++ = ' ';
				continue;
			}

			// if we're on a boundary, save these few bytes for the future
			if ( p+iBytes>pMax )
			{
				m_iReparseStart = (int)(p-m_pBuffer);
				m_iReparseLen = (int)(pMax-p);
				iBufferLen -= m_iReparseLen;
				break;
			}

			// otherwise (not a boundary), check them all
			int i = 1;
			int iVal = ( v >> iBytes );
			for ( ; i<iBytes; i++ )
			{
				if ( ( p[i] & 0xC0 )!=0x80 )
					break;
				iVal = ( iVal<<6 ) + ( p[i] & 0x3f );
			}

			if ( i!=iBytes // remove invalid sequences
				|| ( iVal>=0xd800 && iVal<=0xdfff ) // and utf-16 surrogate pairs
				|| ( iBytes==3 && iVal<0x800 ) // and overlong 3-byte codes
				|| ( iVal>=0xfff0 && iVal<=0xffff ) ) // and kinda-valid specials expat chokes on anyway
			{
				iBytes = i;
				for ( i=0; i<iBytes; i++ )
					p[i] = ' ';
			}

			// only move forward by the amount of succesfully processed bytes!
			p += i;
		}
	}

	if ( sph_XML_Parse ( m_pParser, (const char*) m_pBuffer, iBufferLen, bLast )!=XML_STATUS_OK )
	{
		DocID_t tFailedID = 0;
		if ( m_dParsedDocuments.GetLength() )
			tFailedID = m_dParsedDocuments.Last()->m_tDocID;

		if ( !m_sError.IsEmpty () )
			sError = m_sError;
		else
		{
			sError.SetSprintf ( "source '%s': XML parse error: %s (line=%d, pos=%d, docid=" INT64_FMT ")",
				m_tSchema.GetName(), sph_XML_ErrorString ( sph_XML_GetErrorCode ( m_pParser ) ),
				(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
				tFailedID );
		}

		m_tDocInfo.m_tRowID = 0;
		return false;
	}

	if ( !m_sError.IsEmpty () )
	{
		sError = m_sError;
		m_tDocInfo.m_tRowID = 0;
		return false;
	}

	return true;
}


BYTE **	CSphSource_XMLPipe2::NextDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_pBuffer && m_pFieldBuffer );

	bEOF = true;

	if ( m_bRemoveParsed )
	{
		SafeDelete ( m_dParsedDocuments[0] );
		m_dParsedDocuments.RemoveFast ( 0 );
		m_bRemoveParsed = false;
	}

	// ok by default
	m_tDocInfo.m_tRowID = INVALID_ROWID;

	int iReadResult = 0;

	while ( m_dParsedDocuments.GetLength()==0 )
	{
		// saved bytes to the front!
		if ( m_iReparseLen )
			memmove ( m_pBuffer, m_pBuffer+m_iReparseStart, m_iReparseLen );

		// read more data
		iReadResult = (int) fread ( m_pBuffer+m_iReparseLen, 1, m_iBufferSize-m_iReparseLen, m_pPipe );
		if ( iReadResult==0 )
			break;

		// and parse it
		if ( !ParseNextChunk ( iReadResult+m_iReparseLen, sError ) )
			return nullptr;
	}

	CSphString sWarn;

	while ( m_dParsedDocuments.GetLength()!=0 )
	{
		Document_t * pDocument = m_dParsedDocuments[0];
		int nAttrs = m_tSchema.GetAttrsCount ();

		int iFirstFieldLenAttr = m_tSchema.GetAttrId_FirstFieldLen();
		int iLastFieldLenAttr = m_tSchema.GetAttrId_LastFieldLen();

		m_dMvas.Resize ( m_tSchema.GetAttrsCount() );
		for ( auto & i : m_dMvas )
			i.Resize(0);

		// attributes
		for ( int i = 0; i < nAttrs; i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			if ( sphIsInternalAttr(tAttr) )
				continue;

			// reset, and the value will be filled by IterateHits()
			if ( i>=iFirstFieldLenAttr && i<=iLastFieldLenAttr )
			{
				assert ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT );
				m_tDocInfo.SetAttr ( tAttr.m_tLocator, 0 );
				continue;
			}

			const CSphString & sAttrValue = pDocument->m_dAttrs[i].IsEmpty () && m_dDefaultAttrs.GetLength()
				? m_dDefaultAttrs[i]
				: pDocument->m_dAttrs[i];

			switch ( tAttr.m_eAttrType )
			{
				case SPH_ATTR_STRING:
				case SPH_ATTR_JSON:
					m_dStrAttrs[i] = sAttrValue.cstr();
					if ( !m_dStrAttrs[i].cstr() )
						m_dStrAttrs[i] = "";

					break;

				case SPH_ATTR_FLOAT:
					{
						float fValue = sphToFloat ( sAttrValue.cstr() );
						m_dAttrs[i] = sphF2DW(fValue);

						if ( !tAttr.IsColumnar() )
							m_tDocInfo.SetAttrFloat ( tAttr.m_tLocator, fValue );
					}
					break;

				case SPH_ATTR_BIGINT:
					if ( i )
						m_dAttrs[i] = sphToInt64 ( sAttrValue.cstr(), &sWarn );
					else // negative number checks not necessary here as they were done before
						m_dAttrs[i] = (int64_t)StrToDocID ( sAttrValue.cstr(), sWarn );

					if ( !sWarn.IsEmpty() )
						sphWarn ( "%s", sWarn.cstr() );

					if ( !tAttr.IsColumnar() )
						m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[i] );
					break;

				case SPH_ATTR_UINT32SET:
				case SPH_ATTR_INT64SET:
					if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
						ParseFieldMVA ( i, sAttrValue.cstr() );
					break;

				case SPH_ATTR_BOOL:
					m_dAttrs[i] = sphToDword ( sAttrValue.cstr() ) ? 1 : 0;
					if ( !tAttr.IsColumnar() )
						m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[i] );
					break;

				default:
					m_dAttrs[i] = sphToDword ( sAttrValue.cstr() );
					if ( !tAttr.IsColumnar() )
						m_tDocInfo.SetAttr ( tAttr.m_tLocator, m_dAttrs[i] );
					break;
			}
		}

		m_bRemoveParsed = true;

		int nFields = m_tSchema.GetFieldsCount();
//		if ( !nFields )
//			return nullptr;

		m_dFieldPtrs.Resize ( nFields );
		m_dFieldLengths.Resize ( nFields );
		for ( int i = 0; i < nFields; ++i )
		{
			m_dFieldPtrs[i] = pDocument->m_dFields[i].Begin();
			m_dFieldLengths[i] = pDocument->m_dFields[i].GetLength();

			// skip trailing zero
			if ( m_dFieldLengths[i] && !m_dFieldPtrs[i][m_dFieldLengths[i]-1] )
				m_dFieldLengths[i]--;
		}

		bEOF = false;
		return m_dFieldPtrs.Begin();
	}

	return nullptr;
}


bool CSphSource_XMLPipe2::IterateKillListStart ( CSphString & )
{
	m_iKillListIterator = 0;
	return true;
}


bool CSphSource_XMLPipe2::IterateKillListNext ( DocID_t & tDocID )
{
	if ( m_iKillListIterator>=m_dKillList.GetLength () )
		return false;

	tDocID = m_dKillList [ m_iKillListIterator++ ];
	return true;
}

enum EXMLElem
{
	ELEM_DOCSET,
	ELEM_SCHEMA,
	ELEM_FIELD,
	ELEM_ATTR,
	ELEM_DOCUMENT,
	ELEM_KLIST,
	ELEM_NONE
};

static EXMLElem LookupElement ( const char * szName )
{
	if ( szName[0]!='s' )
		return ELEM_NONE;

	auto iLen = strlen(szName);
	if ( iLen>=11 && iLen<=15 )
	{
		char iHash = (char)( ( iLen + szName[7] ) & 15 );
		switch ( iHash )
		{
		case 1:		if ( !strcmp ( szName, "sphinx:docset" ) )		return ELEM_DOCSET; break;
		case 0:		if ( !strcmp ( szName, "sphinx:schema" ) )		return ELEM_SCHEMA;	break;
		case 2:		if ( !strcmp ( szName, "sphinx:field" ) )		return ELEM_FIELD;	break;
		case 12:	if ( !strcmp ( szName, "sphinx:attr" ) )		return ELEM_ATTR;	break;
		case 3:		if ( !strcmp ( szName, "sphinx:document" ) )	return ELEM_DOCUMENT;break;
		case 10:	if ( !strcmp ( szName, "sphinx:killlist" ) )	return ELEM_KLIST;	break;
		default: break;
		}
	}

	return ELEM_NONE;
}

void CSphSource_XMLPipe2::StartElement ( const char * szName, const char ** pAttrs )
{
	EXMLElem ePos = LookupElement ( szName );

	switch ( ePos )
	{
	case ELEM_DOCSET:
		m_bInDocset = true;
		m_bFirstTagAfterDocset = true;
		return;

	case ELEM_SCHEMA:
	{
		if ( !m_bInDocset || !m_bFirstTagAfterDocset )
		{
			Error ( "<sphinx:schema> is allowed immediately after <sphinx:docset> only" );
			return;
		}

		if ( m_tSchema.GetFieldsCount() > 0 || m_tSchema.GetAttrsCount () > 1 )
			sphWarn ( "%s", DecorateMessage ( "both embedded and configured schemas found; using embedded" ) );

		m_tSchema.Reset();
		CSphMatch tDocInfo;
		Swap ( m_tDocInfo, tDocInfo );
		m_dDefaultAttrs.Reset();

		m_bFirstTagAfterDocset = false;
		m_bInSchema = true;
	}
	return;

	case ELEM_FIELD:
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:field> is allowed inside <sphinx:schema> only" );
			return;
		}

		const char ** dAttrs = pAttrs;
		CSphColumnInfo Info;
		CSphString sDefault;
		bool bIsAttr = false;
		bool bWordDict = ( m_pDict && m_pDict->GetSettings().m_bWordDict );

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
			{
				AddFieldToSchema ( dAttrs[1], bWordDict, m_tSchema );
				Info.m_sName = dAttrs[1];
			} else if ( !strcmp ( *dAttrs, "attr" ) )
			{
				bIsAttr = true;
				if ( !strcmp ( dAttrs[1], "string" ) )
					Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( dAttrs[1], "json" ) )
					Info.m_eAttrType = SPH_ATTR_JSON;

			} else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];

			dAttrs += 2;
		}

		if ( bIsAttr )
		{
			if ( Info.m_sName.IsEmpty() || CSphSchema::IsReserved ( Info.m_sName.cstr() ) )
			{
				Error ( "%s is not a valid attribute name", Info.m_sName.cstr() );
				return;
			}

			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
		}
	}
	return;

	case ELEM_ATTR:
	{
		if ( !m_bInDocset || !m_bInSchema )
		{
			Error ( "<sphinx:attr> is allowed inside <sphinx:schema> only" );
			return;
		}

		bool bError = false;
		CSphString sDefault;

		CSphColumnInfo Info;
		Info.m_eAttrType = SPH_ATTR_INTEGER;

		const char ** dAttrs = pAttrs;

		while ( dAttrs[0] && dAttrs[1] && dAttrs[0][0] && dAttrs[1][0] && !bError )
		{
			if ( !strcmp ( *dAttrs, "name" ) )
				Info.m_sName = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "bits" ) )
				Info.m_tLocator.m_iBitCount = strtol ( dAttrs[1], NULL, 10 );
			else if ( !strcmp ( *dAttrs, "default" ) )
				sDefault = dAttrs[1];
			else if ( !strcmp ( *dAttrs, "type" ) )
			{
				const char * szType = dAttrs[1];
				if ( !strcmp ( szType, "int" ) )				Info.m_eAttrType = SPH_ATTR_INTEGER;
				else if ( !strcmp ( szType, "timestamp" ) )		Info.m_eAttrType = SPH_ATTR_TIMESTAMP;
				else if ( !strcmp ( szType, "bool" ) )			Info.m_eAttrType = SPH_ATTR_BOOL;
				else if ( !strcmp ( szType, "float" ) )			Info.m_eAttrType = SPH_ATTR_FLOAT;
				else if ( !strcmp ( szType, "bigint" ) )		Info.m_eAttrType = SPH_ATTR_BIGINT;
				else if ( !strcmp ( szType, "string" ) )		Info.m_eAttrType = SPH_ATTR_STRING;
				else if ( !strcmp ( szType, "json" ) )			Info.m_eAttrType = SPH_ATTR_JSON;
				else if ( !strcmp ( szType, "multi" ) )
				{
					Info.m_eAttrType = SPH_ATTR_UINT32SET;
					Info.m_eSrc = SPH_ATTRSRC_FIELD;
				} else if ( !strcmp ( szType, "multi_64" ) )
				{
					Info.m_eAttrType = SPH_ATTR_INT64SET;
					Info.m_eSrc = SPH_ATTRSRC_FIELD;
				} else
				{
					Error ( "unknown column type '%s'", szType );
					bError = true;
				}
			}

			dAttrs += 2;
		}

		if ( !bError )
		{
			if ( Info.m_sName.IsEmpty() || CSphSchema::IsReserved ( Info.m_sName.cstr() ) )
			{
				Error ( "%s is not a valid attribute name", Info.m_sName.cstr() );
				return;
			}

			Info.m_iIndex = m_tSchema.GetAttrsCount ();
			m_tSchema.AddAttr ( Info, true ); // all attributes are dynamic at indexing time
			m_dDefaultAttrs.Add ( sDefault );
		}
	}
	return;

	case ELEM_DOCUMENT:
	{
		if ( !m_bInDocset || m_bInSchema )
			return DocumentError ( "<sphinx:schema>" );

		if ( m_bInKillList )
			return DocumentError ( "<sphinx:killlist>" );

		if ( m_bInDocument )
			return DocumentError ( "<sphinx:document>" );

		if ( m_tSchema.GetFieldsCount()==0 && m_tSchema.GetAttrsCount()==0 )
		{
			Error ( "no schema configured, and no embedded schema found" );
			return;
		}

		m_bInDocument = true;

		assert ( !m_pCurDocument );
		m_pCurDocument = new Document_t;
		m_pCurDocument->m_tDocID = INT64_MAX;
		m_pCurDocument->m_dFields.Resize ( m_tSchema.GetFieldsCount() );
		// for safety
		ARRAY_FOREACH ( i, m_pCurDocument->m_dFields )
			m_pCurDocument->m_dFields[i].Add ( '\0' );
		m_pCurDocument->m_dAttrs.Resize ( m_tSchema.GetAttrsCount () );

		if ( pAttrs[0] && pAttrs[1] && pAttrs[0][0] && pAttrs[1][0] )
			if ( !strcmp ( pAttrs[0], "id" ) )
			{
				uint64_t uDocID = StrToDocID ( pAttrs[1], m_sDocIDError );
				m_pCurDocument->m_tDocID = (DocID_t)uDocID;
				m_pCurDocument->m_dAttrs[0] = pAttrs[1];
			}
	}
	return;

	case ELEM_KLIST:
	{
		if ( !m_bInDocset || m_bInDocument || m_bInSchema )
		{
			Error ( "<sphinx:killlist> is not allowed inside <sphinx:schema> or <sphinx:document>" );
			return;
		}

		m_bInKillList = true;
	}
	return;

	case ELEM_NONE: break; // avoid warning
	}

	if ( m_bInKillList )
	{
		if ( m_bInId )
		{
			m_iElementDepth++;
			return;
		}

		if ( !!strcmp ( szName, "id" ) )
		{
			Error ( "only 'id' is allowed inside <sphinx:killlist>" );
			return;
		}

		m_bInId = true;

	} else if ( m_bInDocument )
	{
		if ( m_iCurField!=-1 || m_iCurAttr!=-1 )
		{
			m_iElementDepth++;
			return;
		}

		m_iCurField = m_tSchema.GetFieldIndex ( szName );
		m_iCurAttr = m_tSchema.GetAttrIndex ( szName );

		if ( m_iCurAttr!=-1 || m_iCurField!=-1 )
			return;

		m_bInIgnoredTag = true;

		bool bInvalidFound = false;
		for ( int i = 0; i < m_dInvalid.GetLength () && !bInvalidFound; i++ )
			bInvalidFound = m_dInvalid[i]==szName;

		if ( !bInvalidFound )
		{
			sphWarn ( "%s", DecorateMessage ( "unknown field/attribute '%s'; ignored", szName ) );
			m_dInvalid.Add ( szName );
		}
	}
}


void CSphSource_XMLPipe2::EndElement ( const char * szName )
{
	m_bInIgnoredTag = false;

	EXMLElem ePos = LookupElement ( szName );

	switch ( ePos )
	{
	case ELEM_DOCSET:
		m_bInDocset = false;
		return;

	case ELEM_SCHEMA:
		m_bInSchema = false;
		m_tSchema.SetupFlags ( *this, false, nullptr );
		AddAutoAttrs ( m_sError, &m_dDefaultAttrs );
		AllocDocinfo();
		return;

	case ELEM_DOCUMENT:
		m_bInDocument = false;
		if ( !m_sDocIDError.IsEmpty() )
		{
			sphWarn ( "%s", DecorateMessage ( m_sDocIDError.cstr() ) );
			m_sDocIDError = "";
			delete m_pCurDocument;
		}
		else
		{
			if ( m_pCurDocument )
				m_dParsedDocuments.Add ( m_pCurDocument );
		}

		m_pCurDocument = nullptr;
		return;

	case ELEM_KLIST:
		m_bInKillList = false;
		return;

	case ELEM_FIELD: // avoid warnings
	case ELEM_ATTR:
	case ELEM_NONE: break;
	}

	if ( m_bInKillList )
	{
		if ( m_iElementDepth!=0 )
		{
			m_iElementDepth--;
			return;
		}

		if ( m_bInId )
		{
			m_pFieldBuffer [ Min ( m_iFieldBufferLen, m_iFieldBufferMax ) ] = '\0';
			m_dKillList.Add ( sphToInt64 ( (const char *)m_pFieldBuffer ) );
			m_iFieldBufferLen = 0;
			m_bInId = false;
		}

	} else if ( m_bInDocument && ( m_iCurAttr!=-1 || m_iCurField!=-1 ) )
	{
		if ( m_iElementDepth!=0 )
		{
			--m_iElementDepth;
			return;
		}

		if ( m_iCurField!=-1 )
		{
			assert ( m_pCurDocument );
			CSphVector<BYTE> & dBuf = m_pCurDocument->m_dFields [ m_iCurField ];

			dBuf.Last() = ' ';
			dBuf.Reserve ( dBuf.GetLength() + m_iFieldBufferLen + 6 ); // 6 is a safety gap
			dBuf.Append( m_pFieldBuffer, m_iFieldBufferLen );
			dBuf.Add ( '\0' );
		}
		if ( m_iCurAttr!=-1 )
		{
			assert ( m_pCurDocument );
			if ( !m_pCurDocument->m_dAttrs [ m_iCurAttr ].IsEmpty () )
				sphWarn ( "duplicate attribute node <%s> - using first value", m_tSchema.GetAttr ( m_iCurAttr ).m_sName.cstr() );
			else
				m_pCurDocument->m_dAttrs [ m_iCurAttr ].SetBinary ( (char*)m_pFieldBuffer, m_iFieldBufferLen );
		}

		m_iFieldBufferLen = 0;

		m_iCurAttr = -1;
		m_iCurField = -1;
	}
}


void CSphSource_XMLPipe2::UnexpectedCharaters ( const char * pCharacters, int iLen, const char * szComment )
{
	const int MAX_WARNING_LENGTH = 64;

	bool bSpaces = true;
	for ( int i = 0; i < iLen && bSpaces; i++ )
		if ( !sphIsSpace ( pCharacters[i] ) )
			bSpaces = false;

	if ( !bSpaces )
	{
		CSphString sWarning;
		sWarning.SetBinary ( pCharacters, Min ( iLen, MAX_WARNING_LENGTH ) );
		sphWarn ( "source '%s': unexpected string '%s' (line=%d, pos=%d) %s",
			m_tSchema.GetName(), sWarning.cstr (),
			(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ), szComment );
	}
}


void CSphSource_XMLPipe2::Characters ( const char * pCharacters, int iLen )
{
	if ( m_bInIgnoredTag )
		return;

	if ( !m_bInDocset )
	{
		UnexpectedCharaters ( pCharacters, iLen, "outside of <sphinx:docset>" );
		return;
	}

	if ( !m_bInSchema && !m_bInDocument && !m_bInKillList )
	{
		UnexpectedCharaters ( pCharacters, iLen, "outside of <sphinx:schema> and <sphinx:document>" );
		return;
	}

	if ( m_iCurAttr==-1 && m_iCurField==-1 && !m_bInKillList )
	{
		UnexpectedCharaters ( pCharacters, iLen, m_bInDocument ? "inside <sphinx:document>" : ( m_bInSchema ? "inside <sphinx:schema>" : "" ) );
		return;
	}

	if ( iLen + m_iFieldBufferLen <= m_iFieldBufferMax )
	{
		memcpy ( m_pFieldBuffer + m_iFieldBufferLen, pCharacters, iLen );
		m_iFieldBufferLen += iLen;

	} else
	{
		const char * szName = nullptr;
		if ( m_iCurField!=-1 )
			szName = m_tSchema.GetFieldName ( m_iCurField );
		else if ( m_iCurAttr!=-1 )
			szName = m_tSchema.GetAttr(m_iCurAttr).m_sName.cstr();

		assert ( szName );

		bool bWarned = false;
		for ( int i = 0; i < m_dWarned.GetLength () && !bWarned; i++ )
			bWarned = m_dWarned[i]==szName;

		if ( !bWarned )
		{
			sphWarn ( "source '%s': field/attribute '%s' length exceeds max length (line=%d, pos=%d, docid=" INT64_FMT ")",
				m_tSchema.GetName(), szName,
				(int)sph_XML_GetCurrentLineNumber ( m_pParser ), (int)sph_XML_GetCurrentColumnNumber ( m_pParser ),
				m_pCurDocument->m_tDocID );

			m_dWarned.Add ( szName );
		}
	}
}

CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, const char * szSourceName, int iMaxFieldLen, CSphString & sError )
{
	bool bUTF8 = pSource->GetInt ( "xmlpipe_fixup_utf8", 0 )!=0;

	auto * pXMLPipe = new CSphSource_XMLPipe2(szSourceName);
	if ( !pXMLPipe->SetupXML ( iMaxFieldLen, bUTF8, pPipe, *pSource, sError ) )
		SafeDelete ( pXMLPipe );

	return pXMLPipe;
}
