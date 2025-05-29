//
// Copyright (c) 2008-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxstd.h"
#include "searchdaemon.h"
#include "http/http_parser.h"

using OptionsHash_t = SmallStringHash_T<CSphString>;
class AsyncNetInputBuffer_c;

class HttpRequestParser_c : public ISphNoncopyable
{
public:
	HttpRequestParser_c();
	void Reinit();
	bool ParseHeader ( ByteBlob_t tData );
	bool ProcessClientHttp ( AsyncNetInputBuffer_c& tIn, CSphVector<BYTE>& dResult );

	int ParsedBodyLength() const;
	bool Expect100() const;
	bool KeepAlive() const;
	const char* Error() const;
	bool IsBuddyQuery () const;

	static void ParseList ( Str_t sData, OptionsHash_t & hOptions );

private:

	// callbacks
	int ParserUrl ( Str_t sData );
	int ParserHeaderField ( Str_t sData );
	int ParserHeaderValue ( Str_t sData );
	int ParserBody ( Str_t sData );
	int ParseHeaderCompleted ();
	int MessageComplete();

	void FinishParserUrl();
	void FinishParserKeyVal();

private:
	// callbacks
	static int cbParserUrl ( http_parser* pParser, const char* sAt, size_t iLen );
	static int cbParserHeaderField ( http_parser* pParser, const char* sAt, size_t iLen );
	static int cbParserHeaderValue ( http_parser* pParser, const char* sAt, size_t iLen );
	static int cbParseHeaderCompleted ( http_parser* pParser );
	static int cbParserBody ( http_parser* pParser, const char* sAt, size_t iLen );
	static int cbMessageComplete ( http_parser* pParser );

	static int cbMessageBegin ( http_parser* pParser );
	static int cbMessageStatus ( http_parser* pParser, const char* sAt, size_t iLen );

private:
	bool m_bKeepAlive = false;
	const char* m_szError = nullptr;
	CSphString m_sEndpoint;
	StringBuilder_c m_sUrl;
	StringBuilder_c m_sCurField;
	StringBuilder_c m_sCurValue;
	OptionsHash_t m_hOptions;

	http_method m_eType = HTTP_GET;
	bool m_bHeaderDone = false;
	bool m_bBodyDone = false;
	CSphVector<Str_t> m_dParsedBodies;
	int m_iParsedBodyLength;
	int m_iLastParsed = 0;

	http_parser_settings m_tParserSettings;
	http_parser m_tParser;
};

void HttpBuildReply ( CSphVector<BYTE>& dData, EHTTP_STATUS eCode, Str_t sReply, bool bHtml );

///////////////////////////////////////////////////////////////////////
/// Stream reader
class CharStream_c
{
protected:
	bool m_bDone = false;
	AsyncNetInputBuffer_c * m_pIn;
	CSphString m_sError;

public:
	CharStream_c ( AsyncNetInputBuffer_c * pIn )
		: m_pIn ( pIn )
	{}

	virtual ~CharStream_c() = default;

	// return next chunk of data
	virtual Str_t Read() = 0;

	// return all available data
	virtual Str_t ReadAll() = 0;

	inline bool Eof() const noexcept { return m_bDone; }

	bool GetError() const;
	const CSphString & GetErrorMessage() const;
};

struct HttpProcessResult_t
{
	EHTTP_ENDPOINT m_eEndpoint { EHTTP_ENDPOINT::TOTAL };
	EHTTP_STATUS m_eReplyHttpCode = EHTTP_STATUS::_200;
	bool m_bOk { false };
	CSphString m_sError;
};

void ReplyBuf ( Str_t sResult, EHTTP_STATUS eStatus, bool bNeedHttpResponse, CSphVector<BYTE> & dData );
HttpProcessResult_t ProcessHttpQuery ( CharStream_c & tSource, Str_t & sSrcQuery, OptionsHash_t & hOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType );

namespace bson {
class Bson_c;
}

void ConvertJsonDataset ( const JsonObj_c & tRoot, const char * sStmt, RowBuffer_i & tOut );

using SplitAction_fn = std::function<void(const char *, int)>;
void SplitNdJson ( Str_t sBody, SplitAction_fn && fnAction);
bool HttpSetLogVerbosity ( const CSphString & sVal );
void LogReplyStatus100();
bool Ends ( const Str_t tVal, const char * sSuffix );
enum class HttpErrorType_e;

class HttpHandler_c
{
public:
	HttpHandler_c() = default;
	virtual ~HttpHandler_c() = default;
	virtual bool Process () = 0;
	void SetErrorFormat ( bool bNeedHttpResponse );
	CSphVector<BYTE> & GetResult();
	const CSphString & GetError () const;
	EHTTP_STATUS GetStatusCode () const;

protected:
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;
	CSphString			m_sError;
	EHTTP_STATUS		m_eHttpCode = EHTTP_STATUS::_200;

	void ReportError ( const char * szError, EHTTP_STATUS eStatus );
	void ReportError ( EHTTP_STATUS eStatus );
	void FormatError ( EHTTP_STATUS eStatus, const char * sError, ... );
	void BuildReply ( const CSphString & sResult, EHTTP_STATUS eStatus );
	void BuildReply ( const char* szResult, EHTTP_STATUS eStatus );
	void BuildReply ( Str_t sResult, EHTTP_STATUS eStatus );
	void BuildReply ( const StringBuilder_c & sResult, EHTTP_STATUS eStatus );
	bool CheckValid ( const ServedIndex_c* pServed, const CSphString& sIndex, IndexType_e eType );
	void ReportError ( const char * sError, HttpErrorType_e eType, EHTTP_STATUS eStatus, const char * sIndex=nullptr );
};

class HttpCompatBaseHandler_c : public HttpHandler_c
{
public:
	HttpCompatBaseHandler_c ( Str_t sBody, int iReqType, const SmallStringHash_T<CSphString> & hOpts );

protected:
	Str_t					GetBody() const { return m_sBody; }
	int						GetRequestType() const { return m_iReqType; }
	const CSphString &		GetFullURL() const { return m_hOpts["full_url"]; }
	const SmallStringHash_T<CSphString> &	GetOptions() const { return m_hOpts; }
	const StrVec_t &		GetUrlParts() const { return m_dUrlParts; }

	bool IsHead() { return GetRequestType()==HTTP_HEAD; }
	void BuildReplyHead ( Str_t sRes, EHTTP_STATUS eStatus );

	Str_t m_sBody;
	int m_iReqType { 0 };
	const SmallStringHash_T<CSphString> & m_hOpts;
	StrVec_t m_dUrlParts;
};

std::unique_ptr<HttpHandler_c> CreateCompatHandler ( Str_t sBody, int iReqType, const SmallStringHash_T<CSphString> & hOpts );

enum class HttpErrorType_e
{
	Unknown,
	Parse,
	IllegalArgument,
	ActionRequestValidation,
	IndexNotFound,
	ContentParse,
	VersionConflictEngine,
	DocumentMissing,
	ResourceAlreadyExists,
	AliasesNotFound
};

const char * GetErrorTypeName ( HttpErrorType_e eType );
