//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "netreceive_http.h"
#include "searchdssl.h"
#include "searchdhttp.h"
#include "tracer.h"

extern int g_iClientTimeoutS; // from searchd.cpp
extern volatile bool g_bMaintenance;



void HttpServe ( std::unique_ptr<AsyncNetBuffer_c> pBuf )
{
	auto& tSess = session::Info();

	// non-vip connections in maintainance should be already rejected on accept
	assert  ( !g_bMaintenance || tSess.GetVip() );

	bool bINeedSSL = tSess.GetProto ()==Proto_e::HTTPS;
	bool bHeNeedSSL = tSess.GetSsl();
	bool bICanSSL = bHeNeedSSL ? CheckWeCanUseSSL () : false;

	tSess.SetProto ( Proto_e::HTTP );

	// he needs, but I can't
	if ( bHeNeedSSL && !bICanSSL )
	{
		if ( bINeedSSL )
			LogNetError ( "Client tries to connect with https to secure port, but we can't serve" );

		// that will drop the connection (we can't say anything as can't encrypt our message)
		return;
	}

	// he connects to secured port with plain http
	if ( bINeedSSL && !bHeNeedSSL )
	{
		CSphVector<BYTE> dResult;
		sphHttpErrorReply ( dResult, EHTTP_STATUS::_400, "The plain HTTP request was sent to HTTPS port" );
		auto & tOut = *(GenericOutputBuffer_c *) pBuf.get();
		tOut.SwapData ( dResult );
		tOut.Flush (); // no need to check return code since we break anyway
		return;
	}


	// set off query guard
	auto & tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_eType = QUERY_JSON;

	// needed to check permission to turn maintenance mode on/off

	if ( bHeNeedSSL )
		tSess.SetSsl ( MakeSecureLayer ( pBuf ) );

	auto& tOut = *(GenericOutputBuffer_c *) pBuf.get();
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf.get();

	CSphString sError;
	bool bOk = true;
	HttpRequestParser_c tParser;
	CSphVector<BYTE> dResult;
	TRACE_CONN ( "conn", "HttpServe" );

	auto HttpReply = [&dResult, &tOut] ( EHTTP_STATUS eCode, Str_t sMsg )
	{
		if ( IsEmpty ( sMsg ) )
		{
			HttpBuildReply ( dResult, eCode, sMsg, false );
		} else
		{
			LogNetError ( sMsg.first );
			sphHttpErrorReply ( dResult, eCode, sMsg.first );
		}
		tOut.SwapData ( dResult );
		return tOut.Flush();
	};

	do
	{
		tIn.DiscardProcessed ( -1 ); // -1 means 'force flush'
		tParser.Reinit();

		tSess.SetKilled ( false );

		// read HTTP header
		while ( !tParser.ParseHeader ( tIn.Tail() ) )
		{
			tIn.PopTail ();
			auto iChunk = tIn.ReadAny();
			if ( iChunk > 0 )
				continue;

			if ( !iChunk || tIn.GetError() )
			{
				sError.SetSprintf ( "failed to receive HTTP request, %s", ( tIn.GetError() ? tIn.GetErrorMessage().cstr() : sphSockError() ) );
				HttpReply ( EHTTP_STATUS::_400, FromStr ( sError ) );
			}

			return;
		}

		// malformed header
		if ( tParser.Error() )
		{
			if ( tIn.GetError() )
				sError.SetSprintf ( "%s, %s", tIn.GetErrorMessage().cstr(), tParser.Error() );
			else
				sError = tParser.Error();
			HttpReply ( EHTTP_STATUS::_400, FromStr ( sError ) );
			break;
		}

		// check if we should interrupt because of maxed-out
		if ( IsMaxedOut() )
		{
			HttpReply ( EHTTP_STATUS::_503, g_sMaxedOutMessage );
			gStats().m_iMaxedOut.fetch_add ( 1, std::memory_order_relaxed );
			break;
		}

		// process keep-alive conditions
		if ( tParser.KeepAlive() )
		{
			if ( !tSess.GetPersistent() )
				tIn.SetTimeoutUS ( S2US * g_iClientTimeoutS );
			tSess.SetPersistent ( true );
		} else {
			if ( tSess.GetPersistent() )
				tIn.SetTimeoutUS ( S2US * g_iReadTimeoutS );
			tSess.SetPersistent ( false );
		}

		// if first chunk is (most probably) pure header, we can proceed special headers here
		if ( tParser.Expect100() && !tParser.ParsedBodyLength() )
		{
			if ( !HttpReply ( EHTTP_STATUS::_100, dEmptyStr ) )
				break;
			LogReplyStatus100();
		}

//		tracer.Instant ( [&tIn](StringBuilder_c& sOut) {sOut<< ",\"args\":{\"step\":"<<tIn.HasBytes()<<"}";} );
		bOk = tParser.ProcessClientHttp ( tIn, dResult );

		tOut.SwapData (dResult);
		if ( !tOut.Flush () )
			break;

		pBuf->SyncErrorState();
		if ( tIn.GetError() )
			LogNetError ( tIn.GetErrorMessage().cstr() );
		pBuf->ResetError();

	} while ( tSess.GetPersistent() && bOk );
}
