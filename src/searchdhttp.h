//
// Copyright (c) 2008-2023, Manticore Software LTD (https://manticoresearch.com)
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

void HttpBuildReply ( CSphVector<BYTE>& dData, ESphHttpStatus eCode, Str_t sReply, bool bHtml );

///////////////////////////////////////////////////////////////////////
/// Stream reader
class CharStream_c
{
protected:
	bool m_bDone = false;

public:
	virtual ~CharStream_c() = default;

	// return next chunk of data
	virtual Str_t Read() = 0;

	// return all available data
	virtual Str_t ReadAll() = 0;

	inline bool Eof() const noexcept { return m_bDone; }
};

struct HttpProcessResult_t
{
	ESphHttpEndpoint m_eEndpoint { SPH_HTTP_ENDPOINT_TOTAL };
	bool m_bOk { false };
	CSphString m_sError;
};

void ReplyBuf ( Str_t sResult, ESphHttpStatus eStatus, bool bNeedHttpResponse, CSphVector<BYTE> & dData );
HttpProcessResult_t ProcessHttpQuery ( CharStream_c & tSource, Str_t & sQuery, OptionsHash_t & hOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType );

namespace bson {
class Bson_c;
}

void ConvertJsonDataset ( const bson::Bson_c & tBson, const char * sStmt, RowBuffer_i & tOut );

using SplitAction_fn = std::function<void(const char *, int)>;
void SplitNdJson ( Str_t sBody, SplitAction_fn && fnAction);
bool HttpSetLogVerbosity ( const CSphString & sVal );
void LogReplyStatus100();
bool Ends ( const Str_t tVal, const char * sSuffix );

class HttpHandler_c
{
public:

	virtual ~HttpHandler_c() = default;
	virtual bool Process () = 0;
	void SetErrorFormat ( bool bNeedHttpResponse );
	CSphVector<BYTE> & GetResult();
	const CSphString & GetError () const;

protected:
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;
	CSphString			m_sError;

	void ReportError ( const char * szError, ESphHttpStatus eStatus );
	void ReportError ( ESphHttpStatus eStatus );
	void FormatError ( ESphHttpStatus eStatus, const char * sError, ... );
	void BuildReply ( const CSphString & sResult, ESphHttpStatus eStatus );
	void BuildReply ( const char* szResult, ESphHttpStatus eStatus );
	void BuildReply ( Str_t sResult, ESphHttpStatus eStatus );
	void BuildReply ( const StringBuilder_c & sResult, ESphHttpStatus eStatus );
	bool CheckValid ( const ServedIndex_c* pServed, const CSphString& sIndex, IndexType_e eType );
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
	void BuildReplyHead ( Str_t sRes, ESphHttpStatus eStatus );
	void ReportError ( const char * sError, const char * sErrorType, ESphHttpStatus eStatus, const char * sIndex=nullptr );

	Str_t m_sBody;
	int m_iReqType { 0 };
	const SmallStringHash_T<CSphString> & m_hOpts;
	StrVec_t m_dUrlParts;
};

std::unique_ptr<HttpHandler_c> CreateCompatHandler ( Str_t sBody, int iReqType, const SmallStringHash_T<CSphString> & hOpts );
