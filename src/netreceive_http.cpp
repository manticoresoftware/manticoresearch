//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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
			sphWarning ( "Client tries to connect with https to secure port, but we can't serve" );

		// that will drop the connection (we can't say anything as can't encrypt our message)
		return;
	}

	// he connects to secured port with plain http
	if ( bINeedSSL && !bHeNeedSSL )
	{
		CSphVector<BYTE> dResult;
		sphHttpErrorReply ( dResult, SPH_HTTP_STATUS_400, "The plain HTTP request was sent to HTTPS port" );
		auto & tOut = *(NetGenericOutputBuffer_c *) pBuf.get();
		tOut.SwapData ( dResult );
		tOut.Flush (); // no need to check return code since we break anyway
		return;
	}


	// set off query guard
	auto & tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_eType = QUERY_JSON;

	int iCID = tSess.GetConnID();
	const char * sClientIP = tSess.szClientName();

	// needed to check permission to turn maintenance mode on/off

	if ( bHeNeedSSL )
		tSess.SetSsl ( MakeSecureLayer ( pBuf ) );

	auto& tOut = *(NetGenericOutputBuffer_c *) pBuf.get();
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf.get();

	HttpRequestParser_c tParser;

	do
	{
		tIn.DiscardProcessed ( -1 ); // -1 means 'force flush'
		tParser.Reinit();

		while ( !tParser.ParseHeader ( tIn.Tail() ) )
		{
			tIn.PopTail ();
			auto iChunk = tIn.ReadAny();
			if ( iChunk > 0 )
				continue;

			if ( !iChunk && tIn.GetError() )
				sphWarning ( "failed to receive HTTP request (client=%s(%d)) max packet size(%d) exceeded)", sClientIP, iCID, g_iMaxPacketSize );

			return;
		}
		int iBody = tParser.ParsedBodyLength();
		tIn.PopTail ( tIn.Tail().second - iBody );

		// if first chunk is pure header, we have chance to proceed special headers here
		if ( !iBody )
		{
			if ( tParser.Expect100() )
			{
				CSphVector<BYTE> dResult;
				HttpBuildReply ( dResult, SPH_HTTP_STATUS_100, dEmptyStr, false );
				tOut.SwapData ( dResult );
				tOut.Flush(); // fixme! check return code
				sphLogDebug ("100 Continue sent");
			}
			if ( !tIn.HasBytes() )
				tIn.DiscardProcessed ( -1 );
		}

		if ( tParser.ContentLength()>0 )
			iBody += tParser.ContentLength();

		if ( iBody && !tIn.ReadFrom ( iBody )) {
			sphWarning ( "failed to receive HTTP request (client=%s(%d), exp=%d, error='%s')", sClientIP, iCID, iBody, sphSockError ());
			return;
		}

		// Temporary write \0 at the end, since parser wants z-terminated buf
		auto uOldByte = tIn.Terminate ( iBody, '\0' );
		auto tPacket = tIn.PopTail ( iBody );

		CSphVector<BYTE> dResult;
		if ( IsMaxedOut() )
		{
			sphHttpErrorReply ( dResult, SPH_HTTP_STATUS_503, g_sMaxedOutMessage );
			tOut.SwapData ( dResult );
			tOut.Flush (); // no need to check return code since we break anyway
			gStats().m_iMaxedOut.fetch_add ( 1, std::memory_order_relaxed );
			break;
		}

		tCrashQuery.m_dQuery = tPacket;
		tParser.ProcessClientHttp ( tPacket, dResult );

		if ( tParser.KeepAlive() )
		{
			if ( !tSess.GetPersistent() )
				tIn.SetTimeoutUS ( S2US * g_iClientTimeoutS );
			tSess.SetPersistent(true);
		} else {
			if ( tSess.GetPersistent() )
				tIn.SetTimeoutUS ( S2US * g_iReadTimeoutS );
			tSess.SetPersistent(false);
		}

		tIn.Terminate ( 0, uOldByte ); // return back prev byte

		tOut.SwapData (dResult);
		if ( !tOut.Flush () )
			break;
	} while ( tSess.GetPersistent() );
}
