//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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
extern int g_iHttpLogFile; // from searchd.cpp

int GetGlobalReqID () noexcept
{
	static std::atomic<int> iReqID { 1 };
	return iReqID.fetch_add ( 1, std::memory_order_relaxed );
}

static void logRec ( const StringBuilder_c& sData )
{
	sphSeek ( g_iHttpLogFile, 0, SEEK_END );
	sphWrite ( g_iHttpLogFile, sData.cstr (), sData.GetLength () );
}

static void logOutput ( VecTraits_T<BYTE> dData, int iReqID )
{
	StringBuilder_c tBuf;
	tBuf << "[";
	sphFormatCurrentTime ( tBuf );
	tBuf << "] [Request-ID: " << iReqID << "] - Outgoing HTTP Response:\n";
	auto iLine = tBuf.GetLength();
	for ( auto i = 1; i<iLine; ++i ) tBuf << ">";
	tBuf << "\n";
	if ( dData.IsEmpty () )
		tBuf << "<empty response>";
	else
		tBuf << Str_t { dData };
	tBuf << "\n\n";
	logRec ( tBuf );
}


static void logInput ( VecTraits_T<BYTE> sData, int iReqID, int64_t iTimestamp )
{
	StringBuilder_c tBuf;
	tBuf << "[";
	sphFormatTime ( iTimestamp, tBuf );
	tBuf << "] [Request-ID: " << iReqID << "] - Incoming HTTP Request:\n";
	auto iLine = tBuf.GetLength ();
	for ( auto i = 1; i<iLine; ++i )
		tBuf << "<";
	tBuf << "\n";
	if ( sData.IsEmpty () )
		tBuf << "<empty request>";
	else
		tBuf.AppendRawChunk ( sData );
	tBuf << "\n\n";
	logRec ( tBuf );
}

class LoggingAsyncNetBuffer_c final : public AsyncNetBuffer_c
{
	std::unique_ptr<AsyncNetBuffer_c> m_pFrontend;
	AsyncNetInputBuffer_c & m_tIn;
	GenericOutputBuffer_c & m_tOut;
	int m_iReqID;
	int64_t m_iIncomingTimestamp;
	CSphTightVector<BYTE> m_dInput;

protected:
	int ReadFromBackend ( int iNeed, int iSpace, bool bIntr ) final
	{
		int iRead = 0;
		while ( iNeed>iRead )
		{
			// read raw packet
			if ( !m_tIn.ReadFrom ( iNeed, bIntr ) )
				return -1;

			iNeed =  Clamp ( iNeed, iSpace, m_tIn.HasBytes () );

			auto dChunk = AllocateBuffer ( iNeed );
			m_tIn.GetBytes ( dChunk.begin (), iNeed );
			iRead += iNeed;
			m_dInput.Append ( dChunk.begin(), iNeed );
			if ( !m_iIncomingTimestamp )
				m_iIncomingTimestamp = sphMicroTimer ();
		}
		return iRead;
	}

public:
	explicit LoggingAsyncNetBuffer_c ( std::unique_ptr<AsyncNetBuffer_c>&& pFrontend )
			: m_pFrontend ( std::move ( pFrontend ) )
			, m_tIn ( *m_pFrontend )
			, m_tOut ( *m_pFrontend )
			, m_iReqID ( 0 )
			, m_iIncomingTimestamp ( 0 )
	{}

	void SetWTimeoutUS ( int64_t iTimeoutUS ) final
	{
		m_tOut.SetWTimeoutUS ( iTimeoutUS );
	};

	int64_t GetWTimeoutUS () const final
	{
		return m_tOut.GetWTimeoutUS ();
	}

	void SetTimeoutUS ( int64_t iTimeoutUS ) final
	{
		m_tIn.SetTimeoutUS ( iTimeoutUS );
	};

	int64_t GetTimeoutUS () const final
	{
		return m_tIn.GetTimeoutUS ();
	}

	int64_t GetTotalReceived () const final
	{
		return m_tIn.GetTotalReceived ();
	}

	int64_t GetTotalSent () const final
	{
		return m_tOut.GetTotalSent ();
	}

	void SetReqID ( int iID ) noexcept
	{
		m_iReqID = iID;
	}

	bool SendBuffer ( const VecTraits_T<BYTE> & dData ) final
	{
		logInput ( m_dInput, m_iReqID, std::exchange ( m_iIncomingTimestamp, 0 ) );
		m_dInput.Reset();
		logOutput ( dData, m_iReqID );
		m_tOut.SendBytes ( dData );
		return m_tOut.Flush();
	}
};

bool MakeLoggingLayer ( std::unique_ptr<AsyncNetBuffer_c> & pSource )
{
	pSource = std::make_unique<LoggingAsyncNetBuffer_c> ( std::move ( pSource ) );
	return true;
}

bool SendReply ( GenericOutputBuffer_c & tOut, CSphVector<BYTE>& dData ) noexcept
{
	tOut.SwapData ( dData );
	return tOut.Flush ();
};

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

	bool bHttpLogging = g_iHttpLogFile>=0;
	if ( bHttpLogging )
		MakeLoggingLayer ( pBuf );

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
		return SendReply ( tOut, dResult );
	};

	do
	{
		tIn.DiscardProcessed ( -1 ); // -1 means 'force flush'
		tParser.Reinit();

		tSess.SetKilled ( false );

		if ( bHttpLogging )
		{
			auto pLoggingBuf = static_cast<LoggingAsyncNetBuffer_c *> (pBuf.get ());
			pLoggingBuf->SetReqID ( GetGlobalReqID() );
		}

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

		session::Info().SetBuddy ( tParser.IsBuddyQuery() );

		// check if we should interrupt because of maxed-out
		// but not for buddy queries
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

		if ( !SendReply ( tOut, dResult ) )
			break;

		pBuf->SyncErrorState();
		if ( tIn.GetError() )
			LogNetError ( tIn.GetErrorMessage().cstr() );
		pBuf->ResetError();

	} while ( tSess.GetPersistent() && bOk );
}
