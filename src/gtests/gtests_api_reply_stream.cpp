//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <thread>
#include <vector>

#include "searchdaemon.h"
#include "searchdha.h"
#include "api_reply_stream.h"
#include "networking_daemon.h"
#include "searchd_shard.h"


void BuildRemoteOptimizeRequest ( ISphOutputBuffer & tOut, DWORD uHeartbeatIntervalMs, const CSphString & sTargets, int iCutoff );
void SendShardWriteRequestPreamble ( ISphOutputBuffer & tOut, DWORD uHeartbeatIntervalMs, const CSphString & sTargetName );
void FirePoller ();


class AgentConnTestPeer_c
{
public:
	static void PrepareReceive ( AgentConn_t & tAgent, int iSock, ReplyParser_i & tParser, bool bFresh )
	{
		tAgent.m_iSock = iSock;
		if ( !tAgent.m_tDesc.m_pDash )
			tAgent.m_tDesc.m_pDash = new HostDashboard_t;
		tAgent.m_pParser = &tParser;
		tAgent.InitReplyBuf ();
		tAgent.m_bConnectHandshake = bFresh;
		if ( !bFresh )
			tAgent.m_pReplyCur += sizeof ( int );
	}

	static bool Receive ( AgentConn_t & tAgent, int iSock, ReplyParser_i & tParser, bool bFresh )
	{
		PrepareReceive ( tAgent, iSock, tParser, bFresh );
		return tAgent.ReceiveAnswer ();
	}

	static bool ReceiveWithPrefix ( AgentConn_t & tAgent, int iSock, ReplyParser_i & tParser, bool bFresh, const BYTE * pPrefix, DWORD uBytes )
	{
		PrepareReceive ( tAgent, iSock, tParser, bFresh );
		if ( uBytes>tAgent.ReplyBufPlace () )
			return false;
		memcpy ( tAgent.m_pReplyCur, pPrefix, uBytes );
		return tAgent.ReceiveAnswer ( uBytes );
	}

	static void SetReporter ( AgentConn_t & tAgent, const CSphRefcountedPtr<Reporter_i> & pReporter )
	{
		tAgent.m_pReporter = pReporter;
	}

	static void SetLeaseProbe ( AgentConn_t & tAgent, int64_t iQueryTimeoutMs, int64_t iDeadlineUS, int64_t iPeriodUS )
	{
		tAgent.m_iMyQueryTimeoutMs = iQueryTimeoutMs;
		tAgent.m_iPoolerTimeoutUS = iDeadlineUS;
		tAgent.m_iPoolerTimeoutPeriodUS = iPeriodUS;
	}

	static int64_t GetLeaseDeadlineUS ( const AgentConn_t & tAgent )
	{
		return tAgent.m_iPoolerTimeoutUS;
	}

	static int64_t GetLeasePeriodUS ( const AgentConn_t & tAgent )
	{
		return tAgent.m_iPoolerTimeoutPeriodUS;
	}

	static void AcceptHeartbeat ( AgentConn_t & tAgent )
	{
		tAgent.AcceptRemoteReplyHeartbeat ();
	}

	static void ResetForNextRequest ( AgentConn_t & tAgent )
	{
		tAgent.GenericInit ( nullptr, nullptr, nullptr, -1, -1 );
	}

	static void SetRetryBudget ( AgentConn_t & tAgent, int iRetries )
	{
		tAgent.m_iRetries = iRetries;
	}

	static int GetRetryBudget ( const AgentConn_t & tAgent )
	{
		return tAgent.m_iRetries;
	}

	static void ArmHardQueryTimeoutWithoutRetry ( AgentConn_t & tAgent )
	{
		if ( !tAgent.m_tDesc.m_pDash )
			tAgent.m_tDesc.m_pDash = new HostDashboard_t;
		tAgent.m_iRetries = -1;
		tAgent.m_bManyTries = false;
		tAgent.m_eTimeoutKind = AgentConn_t::TIMEOUT_HARD;
	}
};


class IntReplyParser_c : public ReplyParser_i
{
public:
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		++m_iCalls;
		m_iValue = tReq.GetInt ();
		return true;
	}

	mutable int m_iCalls = 0;
	mutable int m_iValue = 0;
};


class EmptyReplyParser_c : public ReplyParser_i
{
public:
	bool ParseReply ( MemInputBuffer_c &, AgentConn_t & ) const final
	{
		++m_iCalls;
		return true;
	}

	mutable int m_iCalls = 0;
};


class TestPingRequestBuilder_c final : public RequestBuilder_i
{
public:
	void BuildRequest ( const AgentConn_t &, ISphOutputBuffer & tOut ) const final
	{
		auto tHeader = APIHeader ( tOut, SEARCHD_COMMAND_PING, VER_COMMAND_PING );
		tOut.SendInt ( 0 );
	}
};


struct ScheduledReplyResult_t
{
	int m_iSucceeded = 0;
	bool m_bAgentSuccess = false;
	CSphString m_sFailure;
	int m_iServerErrorStage = 0;
	int m_iServerSocketError = 0;
};


static std::atomic<bool> g_bScheduledReplyPollerUsed { false };


class ScheduledReplyPollerEnvironment_c final : public ::testing::Environment
{
public:
	void TearDown () final
	{
		if ( !g_bScheduledReplyPollerUsed.load ( std::memory_order_acquire ) )
			return;

		// Production daemon shutdown signals interruption before waking and
		// joining the lazy agent poller. The scheduler harness creates that
		// singleton too, so reproduce the same final process-lifetime contract.
		sphInterruptNow ();
		FirePoller ();
	}
};


::testing::Environment VARIABLE_IS_NOT_USED * const g_pScheduledReplyPollerEnvironment =
	::testing::AddGlobalTestEnvironment ( new ScheduledReplyPollerEnvironment_c );


class CountingReporter_c : public Reporter_i
{
public:
	void FeedTask ( bool ) final {}
	void Report ( bool bSuccess ) final
	{
		++m_iCalls;
		m_bSuccess = bSuccess;
	}
	bool IsDone () const final { return false; }

	int m_iCalls = 0;
	bool m_bSuccess = false;
};


class ReplyStreamOutput_c final : public GenericOutputBuffer_c
{
public:
	using SendObserver_f = std::function<void ( const VecTraits_T<BYTE> & )>;

	explicit ReplyStreamOutput_c ( int64_t iWriteTimeoutUS = 5 * S2US )
		: m_iWriteTimeoutUS ( iWriteTimeoutUS )
	{}

	bool SendBuffer ( const VecTraits_T<BYTE> & dData ) final
	{
		if ( m_bSessionReused.load ( std::memory_order_acquire ) )
			m_bWriteAfterReuse.store ( true, std::memory_order_release );
		if ( m_fnSendObserver )
			m_fnSendObserver ( dData );

		m_iSendCalls.fetch_add ( 1, std::memory_order_release );
		const int iFailurePrefix = m_iFailurePrefixBytes.load ( std::memory_order_acquire );
		if ( iFailurePrefix<0 )
		{
			m_iTotalSent.fetch_add ( dData.GetLength64 (), std::memory_order_relaxed );
			return true;
		}

		m_iTotalSent.fetch_add ( std::min<int64_t> ( iFailurePrefix, dData.GetLength64 () ), std::memory_order_relaxed );
		m_bError = true;
		m_sError = "forced reply-stream send failure";
		return false;
	}

	void SetWTimeoutUS ( int64_t iTimeoutUS ) final { m_iWriteTimeoutUS.store ( iTimeoutUS, std::memory_order_release ); }
	int64_t GetWTimeoutUS () const final { return m_iWriteTimeoutUS.load ( std::memory_order_acquire ); }
	int64_t GetTotalSent () const final { return m_iTotalSent.load ( std::memory_order_relaxed ); }
	int GetSendCalls () const { return m_iSendCalls.load ( std::memory_order_acquire ); }
	bool WroteAfterReuse () const { return m_bWriteAfterReuse.load ( std::memory_order_acquire ); }

	void SetSendObserver ( SendObserver_f fnObserver ) { m_fnSendObserver = std::move ( fnObserver ); }
	void FailSendsAfterPrefix ( int iPrefixBytes ) { m_iFailurePrefixBytes.store ( iPrefixBytes, std::memory_order_release ); }
	void MarkSessionReused () { m_bSessionReused.store ( true, std::memory_order_release ); }

private:
	std::atomic<int64_t> m_iWriteTimeoutUS;
	std::atomic<int64_t> m_iTotalSent { 0 };
	std::atomic<int> m_iSendCalls { 0 };
	std::atomic<int> m_iFailurePrefixBytes { -1 };
	std::atomic<bool> m_bSessionReused { false };
	std::atomic<bool> m_bWriteAfterReuse { false };
	SendObserver_f m_fnSendObserver;
};


static bool WaitForAttemptCount ( const std::atomic<int> & iAttempts, int iExpected, int iTimeoutMs = 500 )
{
	const int64_t iDeadlineUS = MonoMicroTimer () + int64_t ( iTimeoutMs ) * 1000;
	while ( iAttempts.load ( std::memory_order_acquire )<iExpected && MonoMicroTimer ()<iDeadlineUS )
		Threads::Coro::SleepMsec ( 1 );
	return iAttempts.load ( std::memory_order_acquire )>=iExpected;
}


static bool WaitForSendCount ( const ReplyStreamOutput_c & tOut, int iExpected, int iTimeoutMs = 500 )
{
	const int64_t iDeadlineUS = MonoMicroTimer () + int64_t ( iTimeoutMs ) * 1000;
	while ( tOut.GetSendCalls ()<iExpected && MonoMicroTimer ()<iDeadlineUS )
		Threads::Coro::SleepMsec ( 1 );
	return tOut.GetSendCalls ()>=iExpected;
}


static bool CreateTestSocketPair ( int & iRead, int & iWrite )
{
#if _WIN32
	sockaddr_in tAddr {};
	tAddr.sin_family = AF_INET;
	tAddr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
	tAddr.sin_port = 0;

	int iListen = (int)socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( iListen<0 || bind ( iListen, (sockaddr *)&tAddr, sizeof ( tAddr ) )<0 || listen ( iListen, 1 )<0 )
	{
		SafeCloseSocket ( iListen );
		return false;
	}

	int iAddrLen = sizeof ( tAddr );
	if ( getsockname ( iListen, (sockaddr *)&tAddr, &iAddrLen )<0 )
	{
		SafeCloseSocket ( iListen );
		return false;
	}

	iWrite = (int)socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( iWrite<0 || connect ( iWrite, (sockaddr *)&tAddr, sizeof ( tAddr ) )<0 )
	{
		SafeCloseSocket ( iListen );
		SafeCloseSocket ( iWrite );
		return false;
	}

	iRead = (int)accept ( iListen, nullptr, nullptr );
	SafeCloseSocket ( iListen );
	if ( iRead<0 )
	{
		SafeCloseSocket ( iWrite );
		return false;
	}
	return true;
#else
	int dSockets[2] = { -1, -1 };
	if ( socketpair ( AF_LOCAL, SOCK_STREAM, 0, dSockets )!=0 )
		return false;
	iRead = dSockets[0];
	iWrite = dSockets[1];
	return true;
#endif
}


static bool CreateTestListener ( int & iListen, int & iPort )
{
	sockaddr_in tAddr {};
	tAddr.sin_family = AF_INET;
	tAddr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
	tAddr.sin_port = 0;

	iListen = (int)socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( iListen<0 || bind ( iListen, (sockaddr *)&tAddr, sizeof ( tAddr ) )<0 || listen ( iListen, 1 )<0 )
	{
		SafeCloseSocket ( iListen );
		return false;
	}

#if _WIN32
	int iAddrLen = sizeof ( tAddr );
#else
	socklen_t iAddrLen = sizeof ( tAddr );
#endif
	if ( getsockname ( iListen, (sockaddr *)&tAddr, &iAddrLen )<0 )
	{
		SafeCloseSocket ( iListen );
		return false;
	}

	iPort = ntohs ( tAddr.sin_port );
	return true;
}


static bool ReceiveExact ( int iSock, BYTE * pData, int iBytes )
{
	int iReceived = 0;
	while ( iReceived<iBytes )
	{
		auto iRes = sphSockRecv ( iSock, (char *)pData + iReceived, iBytes - iReceived );
		if ( iRes<=0 )
			return false;
		iReceived += (int)iRes;
	}
	return true;
}


static bool RunScheduledReply ( const ISphOutputBuffer & tReply, ReplyParser_i & tParser, bool bExpectProgress,
	ScheduledReplyResult_t & tResult, bool bByteSplit = false )
{
	tResult = {};
	int iListen = -1;
	int iPort = 0;
	if ( !CreateTestListener ( iListen, iPort ) )
		return false;

#if _WIN32
	// Production searchd loads ConnectEx and enables client TFO during startup.
	// The gtest environment initializes Winsock only, so reproduce that transport
	// initialization for the real scheduler path and restore the process setting.
	const int iSavedTFO = sphGetTFO ();
	if ( !LoadExFunctions () )
	{
		tResult.m_iServerErrorStage = 5;
		tResult.m_iServerSocketError = sphSockGetErrno ();
		SafeCloseSocket ( iListen );
		return false;
	}
	sphGetTFO () = iSavedTFO | TFO_CONNECT;
#endif

	const auto * pReply = static_cast<const BYTE *> ( tReply.GetBufPtr () );
	std::vector<BYTE> dReply ( pReply, pReply + tReply.GetSentCount () );
	std::atomic<bool> bServerOk { true };
	std::atomic<int> iServerErrorStage { 0 };
	std::atomic<int> iServerSocketError { 0 };
	std::thread tServer ( [iListen, dReply = std::move ( dReply ), bByteSplit, &bServerOk, &iServerErrorStage, &iServerSocketError] () mutable
	{
		int iClient = (int)accept ( iListen, nullptr, nullptr );
		SafeCloseSocket ( iListen );
		if ( iClient<0 )
		{
			bServerOk.store ( false, std::memory_order_release );
			iServerErrorStage.store ( 1, std::memory_order_release );
			iServerSocketError.store ( sphSockGetErrno (), std::memory_order_release );
			return;
		}

		// A fresh API request is the 4-byte client handshake followed by the
		// 8-byte command header and 4-byte ping body. Consume it before close;
		// otherwise Windows can turn the close into a reset due to unread input.
		BYTE dRequest[16];
		if ( !ReceiveExact ( iClient, dRequest, sizeof ( dRequest ) ) )
		{
			bServerOk.store ( false, std::memory_order_release );
			iServerErrorStage.store ( 2, std::memory_order_release );
			iServerSocketError.store ( sphSockGetErrno (), std::memory_order_release );
			SafeCloseSocket ( iClient );
			return;
		}

		MemInputBuffer_c tRequest ( dRequest, sizeof ( dRequest ) );
		if ( tRequest.GetDword ()!=SPHINX_CLIENT_VERSION
			|| tRequest.GetWord ()!=SEARCHD_COMMAND_PING
			|| tRequest.GetWord ()!=VER_COMMAND_PING
			|| tRequest.GetInt ()!=4
			|| tRequest.GetInt ()!=0 )
		{
			bServerOk.store ( false, std::memory_order_release );
			iServerErrorStage.store ( 3, std::memory_order_release );
			SafeCloseSocket ( iClient );
			return;
		}

		int iSent = 0;
		while ( iSent<(int)dReply.size () )
		{
			const int iChunk = bByteSplit ? 1 : (int)dReply.size () - iSent;
			auto iRes = sphSockSend ( iClient, (const char *)dReply.data () + iSent, iChunk );
			if ( iRes<=0 )
			{
				bServerOk.store ( false, std::memory_order_release );
				iServerErrorStage.store ( 4, std::memory_order_release );
				iServerSocketError.store ( sphSockGetErrno (), std::memory_order_release );
				break;
			}
			iSent += (int)iRes;
			if ( bByteSplit )
				std::this_thread::sleep_for ( std::chrono::milliseconds ( 1 ) );
		}

		std::this_thread::sleep_for ( std::chrono::milliseconds ( 20 ) );
		SafeCloseSocket ( iClient );
	} );

	auto * pAgent = new AgentConn_t;
	pAgent->m_tDesc.m_iFamily = AF_INET;
	pAgent->m_tDesc.m_sAddr = "127.0.0.1";
	pAgent->m_tDesc.m_iPort = iPort;
	pAgent->m_tDesc.m_uAddr = htonl ( INADDR_LOOPBACK );
	pAgent->m_tDesc.m_bNeedResolve = false;
	pAgent->m_tDesc.m_pDash = new HostDashboard_t ( pAgent->m_tDesc );
	pAgent->m_iMyConnectTimeoutMs = 1000;
	pAgent->m_iMyQueryTimeoutMs = 1000;
	if ( bExpectProgress )
		pAgent->EnableRemoteReplyHeartbeats ();

	VecRefPtrsAgentConn_t dAgents;
	dAgents.Add ( pAgent );
	TestPingRequestBuilder_c tRequest;
	Threads::CallCoroutine ( [&]
	{
		tResult.m_iSucceeded = PerformRemoteTasks ( dAgents, &tRequest, &tParser, 0, 0 );
	} );
	g_bScheduledReplyPollerUsed.store ( true, std::memory_order_release );

	tServer.join ();
	tResult.m_bAgentSuccess = pAgent->m_bSuccess;
	tResult.m_sFailure = pAgent->m_sFailure;
	tResult.m_iServerErrorStage = iServerErrorStage.load ( std::memory_order_acquire );
	tResult.m_iServerSocketError = iServerSocketError.load ( std::memory_order_acquire );
#if _WIN32
	sphGetTFO () = iSavedTFO;
#endif
	return bServerOk.load ( std::memory_order_acquire );
}


static bool SendAll ( int iSock, const ISphOutputBuffer & tOut )
{
	int iSent = 0;
	while ( iSent<tOut.GetSentCount () )
	{
		auto iRes = sphSockSend ( iSock, (const char *)tOut.GetBufPtr () + iSent, tOut.GetSentCount () - iSent );
		if ( iRes<=0 )
			return false;
		iSent += (int)iRes;
	}
	return true;
}


static bool SendPrefix ( int iSock, const ISphOutputBuffer & tOut, int iBytes )
{
	int iSent = 0;
	while ( iSent<iBytes )
	{
		auto iRes = sphSockSend ( iSock, (const char *)tOut.GetBufPtr () + iSent, iBytes - iSent );
		if ( iRes<=0 )
			return false;
		iSent += (int)iRes;
	}
	return true;
}


static bool SendRange ( int iSock, const ISphOutputBuffer & tOut, int iOffset, int iBytes )
{
	int iSent = 0;
	while ( iSent<iBytes )
	{
		auto iRes = sphSockSend ( iSock, (const char *)tOut.GetBufPtr () + iOffset + iSent, iBytes - iSent );
		if ( iRes<=0 )
			return false;
		iSent += (int)iRes;
	}
	return true;
}


static void AddInProgressReply ( ISphOutputBuffer & tOut, WORD uVersion, bool bBody = false )
{
	auto tReply = APIAnswer ( tOut, uVersion, SEARCHD_IN_PROGRESS );
	if ( bBody )
		tOut.SendInt ( 1 );
}


static void AddTerminalReply ( ISphOutputBuffer & tOut, WORD uVersion, int iValue )
{
	auto tReply = APIAnswer ( tOut, uVersion, SEARCHD_OK );
	tOut.SendInt ( iValue );
}


static void AddEmptyTerminalReply ( ISphOutputBuffer & tOut, WORD uVersion )
{
	auto tReply = APIAnswer ( tOut, uVersion, SEARCHD_OK );
}


static void AddRawReplyHeader ( ISphOutputBuffer & tOut, WORD uVersion, int iBodyLength )
{
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( uVersion );
	tOut.SendInt ( iBodyLength );
}


TEST ( api_reply_stream, wire_constants )
{
	EXPECT_EQ ( WORD ( 4 ), WORD ( SEARCHD_IN_PROGRESS ) );
	EXPECT_EQ ( DWORD ( 1 ), DWORD ( API_FLAG_SHARD_PHYSICAL_UPDATE ) );
	EXPECT_EQ ( WORD ( 21 ), WORD ( SEARCHD_COMMAND_OPTIMIZE ) );
	EXPECT_EQ ( WORD ( 0x101 ), WORD ( VER_COMMAND_OPTIMIZE ) );
	EXPECT_EQ ( WORD ( 20 ), WORD ( SEARCHD_COMMAND_SHARD_WRITE ) );
	EXPECT_EQ ( WORD ( 0x101 ), WORD ( VER_COMMAND_SHARD_WRITE ) );
}


TEST ( api_reply_stream, shard_write_request_version_and_stream_settings )
{
	ISphOutputBuffer tWire;
	{
		auto tHdr = APIHeader ( tWire, SEARCHD_COMMAND_SHARD_WRITE, VER_COMMAND_SHARD_WRITE );
		SendShardWriteRequestPreamble ( tWire, 50, CSphString ( "rt" ) );
		tWire.SendDword ( 0x11223344 );
	}

	constexpr BYTE dExpected[] =
	{
		0x00, 0x14, 0x01, 0x01, 0x00, 0x00, 0x00, 0x0e,
		0x00, 0x00, 0x00, 0x32,
		0x00, 0x00, 0x00, 0x02, 'r', 't',
		0x11, 0x22, 0x33, 0x44
	};
	ASSERT_EQ ( int ( sizeof ( dExpected ) ), tWire.GetSentCount () );
	const auto * pWire = static_cast<const BYTE *> ( tWire.GetBufPtr () );
	for ( int i = 0; i < tWire.GetSentCount (); ++i )
		EXPECT_EQ ( dExpected[i], pWire[i] ) << "byte " << i;

	const auto fnRun = [] ( WORD uVersion, bool bSendInterval, DWORD uIntervalMs, bool bSendTarget = true )
	{
		ISphOutputBuffer tRequest;
		if ( bSendInterval )
			tRequest.SendDword ( uIntervalMs );
		if ( bSendTarget )
			tRequest.SendString ( "c10_07_missing" );

		InputBuffer_c tRequestIn ( tRequest.m_dBuf );
		ReplyStreamOutput_c tReply;
		HandleCommandShardWrite ( tReply, uVersion, tRequestIn );

		InputBuffer_c tReplyIn ( tReply.m_dBuf );
		EXPECT_EQ ( SEARCHD_ERROR, tReplyIn.GetWord () );
		EXPECT_EQ ( 0, tReplyIn.GetWord () );
		const int iBodyLength = tReplyIn.GetInt ();
		const CSphString sError = tReplyIn.GetString ();
		EXPECT_EQ ( 4 + sError.Length (), iBodyLength );
		EXPECT_FALSE ( tReplyIn.GetError () );
		EXPECT_EQ ( 0, tReplyIn.HasBytes () );
		return sError;
	};

	EXPECT_TRUE ( fnRun ( 0x100, false, 0 ).Begins ( "table 'c10_07_missing' absent" ) );
	EXPECT_TRUE ( fnRun ( VER_COMMAND_SHARD_WRITE, true, 50 ).Begins ( "table 'c10_07_missing' absent" ) );
	EXPECT_STREQ ( "invalid or missing shard write heartbeat interval", fnRun ( VER_COMMAND_SHARD_WRITE, false, 0, false ).cstr() );
	EXPECT_STREQ ( "invalid or missing shard write heartbeat interval", fnRun ( VER_COMMAND_SHARD_WRITE, true, 0 ).cstr() );
	EXPECT_TRUE ( fnRun ( 0x102, false, 0 ).Begins ( "client version is higher" ) );

	ReplyStreamOutput_c tVersionReply;
	EXPECT_FALSE ( CheckCommandVersion ( 0x101, 0x100, tVersionReply ) );
}


TEST ( api_reply_stream, heartbeat_interval_and_parser_opt_in )
{
	struct TimeoutCase_t
	{
		int64_t m_iTimeoutMs;
		DWORD m_uExpectedIntervalMs;
	};
	const TimeoutCase_t dCases[] =
	{
		{ 1, 1 },
		{ 3, 1 },
		{ 4, 1 },
		{ 200, 50 },
		{ INT_MAX, DWORD ( INT_MAX/4 ) }
	};
	for ( const auto & tCase : dCases )
	{
		SCOPED_TRACE ( tCase.m_iTimeoutMs );
		EXPECT_EQ ( tCase.m_uExpectedIntervalMs, CalcRemoteHeartbeatIntervalMs ( tCase.m_iTimeoutMs ) );
	}

	CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
	pAgent->m_iMyQueryTimeoutMs = 200;
	EXPECT_FALSE ( pAgent->ExpectsRemoteReplyHeartbeat () );
	pAgent->EnableRemoteReplyHeartbeats ();
	EXPECT_TRUE ( pAgent->ExpectsRemoteReplyHeartbeat () );
}


TEST ( api_reply_stream, optimize_request_wire_layout_and_validation )
{
	ISphOutputBuffer tWire;
	BuildRemoteOptimizeRequest ( tWire, 50, CSphString ( "rt_a,rt_b" ), 1 );

	constexpr BYTE dExpected[] =
	{
		0x00, 0x15, 0x01, 0x01, 0x00, 0x00, 0x00, 0x15,
		0x00, 0x00, 0x00, 0x32,
		0x00, 0x00, 0x00, 0x09, 'r', 't', '_', 'a', ',', 'r', 't', '_', 'b',
		0x00, 0x00, 0x00, 0x01
	};
	ASSERT_EQ ( int ( sizeof ( dExpected ) ), tWire.GetSentCount () );
	const auto * pWire = static_cast<const BYTE *> ( tWire.GetBufPtr () );
	for ( int i = 0; i < tWire.GetSentCount (); ++i )
		EXPECT_EQ ( dExpected[i], pWire[i] ) << "byte " << i;

	const auto fnRunInvalid = [] ( WORD uVersion, DWORD uIntervalMs, const char * szTarget, int iCutoff, bool bSendCutoff = true, bool bTrailing = false )
	{
		ISphOutputBuffer tRequest;
		tRequest.SendDword ( uIntervalMs );
		tRequest.SendString ( szTarget );
		if ( bSendCutoff )
			tRequest.SendInt ( iCutoff );
		if ( bTrailing )
			tRequest.SendInt ( 123 );

		InputBuffer_c tRequestIn ( tRequest.m_dBuf );
		ReplyStreamOutput_c tReply;
		HandleCommandOptimize ( tReply, uVersion, tRequestIn );

		InputBuffer_c tReplyIn ( tReply.m_dBuf );
		EXPECT_EQ ( SEARCHD_ERROR, tReplyIn.GetWord () );
		EXPECT_EQ ( 0, tReplyIn.GetWord () );
		const int iBodyLength = tReplyIn.GetInt ();
		const CSphString sError = tReplyIn.GetString ();
		EXPECT_EQ ( 4 + sError.Length (), iBodyLength );
		EXPECT_FALSE ( tReplyIn.GetError () );
		EXPECT_EQ ( 0, tReplyIn.HasBytes () );
		return sError;
	};

	EXPECT_TRUE ( fnRunInvalid ( 0x102, 50, "rt", 0 ).Begins ( "client version is higher" ) );
	EXPECT_STREQ ( "optimize request requires a heartbeat-capable command version", fnRunInvalid ( 0x100, 50, "rt", 0 ).cstr() );
	EXPECT_STREQ ( "invalid or truncated optimize request", fnRunInvalid ( VER_COMMAND_OPTIMIZE, 50, "rt", 0, false ).cstr() );
	EXPECT_STREQ ( "invalid or truncated optimize request", fnRunInvalid ( VER_COMMAND_OPTIMIZE, 50, "rt", 0, true, true ).cstr() );
	EXPECT_STREQ ( "optimize request requires a positive heartbeat interval", fnRunInvalid ( VER_COMMAND_OPTIMIZE, 0, "rt", 0 ).cstr() );
	EXPECT_STREQ ( "optimize target table list is empty", fnRunInvalid ( VER_COMMAND_OPTIMIZE, 50, "", 0 ).cstr() );
	EXPECT_STREQ ( "optimize cutoff must be nonnegative", fnRunInvalid ( VER_COMMAND_OPTIMIZE, 50, "rt", -1 ).cstr() );
}


TEST ( api_reply_stream, writer_lifecycle )
{
	Threads::CallCoroutine ( []
	{
		ReplyStreamOutput_c tOut;
		ApiReplyStream_c tNeverStarted ( 10, 0x0123, tOut );
		EXPECT_TRUE ( tNeverStarted.StopAndHandoff () );
		EXPECT_TRUE ( tNeverStarted.StopAndHandoff () );

		ApiReplyStream_c tStarted ( 10, 0x0123, tOut );
		tStarted.Start ();
		EXPECT_TRUE ( tStarted.StopAndHandoff () );
		EXPECT_TRUE ( tStarted.StopAndHandoff () );
	} );
}


TEST ( api_reply_stream, writer_structured_handoff_lifetime )
{
	std::atomic<bool> bScopeExited { false };
	std::atomic<int> iSessionWrites { 0 };
	std::atomic<bool> bSendExited { false };
	Threads::Coro::Event_c tSendEntered;
	Threads::Coro::Event_c tReleaseSend;

	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		{
			ApiReplyStream_c tNeverStarted ( 10, 0x0123, tOut );
			EXPECT_TRUE ( tNeverStarted.StopAndHandoff () );
		}
		Threads::Coro::SleepMsec ( 20 );
		EXPECT_EQ ( 0, tOut.GetSendCalls () );

		tOut.SetSendObserver ( [&] ( const VecTraits_T<BYTE> & )
		{
			iSessionWrites.fetch_add ( 1, std::memory_order_release );
			tSendEntered.SetEvent ();
			tReleaseSend.WaitEvent ();
			bSendExited.store ( true, std::memory_order_release );
		} );
		{
			ApiReplyStream_c tStarted ( 100, 0x0123, tOut );
			tStarted.Start ();
			tSendEntered.WaitEvent ();
			Threads::StartJob ( [&tReleaseSend]
			{
				Threads::Coro::SleepMsec ( 20 );
				tReleaseSend.SetEvent ();
			}, GlobalWorkPool () );
		}
		EXPECT_TRUE ( bSendExited.load ( std::memory_order_acquire ) );
		tOut.MarkSessionReused ();
		tOut.Rewind ( 0 );
		Threads::Coro::SleepMsec ( 120 );
		EXPECT_EQ ( 1, iSessionWrites.load ( std::memory_order_acquire ) );
		EXPECT_EQ ( 1, tOut.GetSendCalls () );
		EXPECT_FALSE ( tOut.WroteAfterReuse () );
		bScopeExited.store ( true, std::memory_order_release );
	} );
	EXPECT_TRUE ( bScopeExited.load ( std::memory_order_acquire ) );
}


TEST ( api_reply_stream, writer_delayed_periodic_queue_free )
{
	constexpr uint32_t uIntervalMs = 20;
	std::atomic<int> iAttempts { 0 };
	std::atomic<bool> bRanOnGlobalPool { false };
	std::atomic<int64_t> iFirstStartedUS { 0 };
	std::atomic<int64_t> iFirstFinishedUS { 0 };
	std::atomic<int64_t> iSecondStartedUS { 0 };
	Threads::Coro::Event_c tFirstEntered;
	Threads::Coro::Event_c tReleaseFirst;

	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		tOut.SetSendObserver ( [&] ( const VecTraits_T<BYTE> & )
		{
			bRanOnGlobalPool.store ( Threads::Coro::CurrentScheduler ()==GlobalWorkPool (), std::memory_order_release );
			const int iAttempt = iAttempts.fetch_add ( 1, std::memory_order_acq_rel ) + 1;
			if ( iAttempt==1 )
			{
				iFirstStartedUS.store ( sphMicroTimer (), std::memory_order_release );
				tFirstEntered.SetEvent ();
				tReleaseFirst.WaitEvent ();
				iFirstFinishedUS.store ( sphMicroTimer (), std::memory_order_release );
			} else if ( iAttempt==2 )
				iSecondStartedUS.store ( sphMicroTimer (), std::memory_order_release );
		} );
		ApiReplyStream_c tStream ( uIntervalMs, 0x0123, tOut );

		const int64_t iStartedUS = sphMicroTimer ();
		tStream.Start ();
		Threads::Coro::SleepMsec ( uIntervalMs/2 );
		EXPECT_EQ ( 0, iAttempts.load ( std::memory_order_acquire ) );
		tFirstEntered.WaitEvent ();
		EXPECT_EQ ( 1, iAttempts.load ( std::memory_order_acquire ) );
		EXPECT_GE ( iFirstStartedUS.load ( std::memory_order_acquire ) - iStartedUS, int64_t ( uIntervalMs ) * 1000 - sph::TICKS_GRANULARITY );

		Threads::Coro::SleepMsec ( uIntervalMs * 3 );
		EXPECT_EQ ( 1, iAttempts.load ( std::memory_order_acquire ) );
		tReleaseFirst.SetEvent ();
		ASSERT_TRUE ( WaitForAttemptCount ( iAttempts, 2 ) );
		EXPECT_TRUE ( bRanOnGlobalPool.load ( std::memory_order_acquire ) );
		const int64_t iObservedWaitUS = iSecondStartedUS.load ( std::memory_order_acquire ) - iFirstFinishedUS.load ( std::memory_order_acquire );
		const int64_t iIntervalFloorUS = int64_t ( uIntervalMs ) * 1000 - sph::TICKS_GRANULARITY;
		EXPECT_GE ( iObservedWaitUS, iIntervalFloorUS );

		EXPECT_TRUE ( tStream.StopAndHandoff () );
		const int iStoppedAt = iAttempts.load ( std::memory_order_acquire );
		Threads::Coro::SleepMsec ( uIntervalMs * 2 );
		EXPECT_EQ ( iStoppedAt, iAttempts.load ( std::memory_order_acquire ) );
	} );
}


TEST ( api_reply_stream, writer_progresses_while_command_suspended )
{
	constexpr uint32_t uIntervalMs = 20;
	std::atomic<int> iAttempts { 0 };
	std::atomic<bool> bCommandWorkStarted { false };

	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		tOut.SetSendObserver ( [&iAttempts] ( const VecTraits_T<BYTE> & ) { iAttempts.fetch_add ( 1, std::memory_order_release ); } );
		ApiReplyStream_c tStream ( uIntervalMs, 0x0123, tOut );
		tStream.Start ();

		bCommandWorkStarted.store ( true, std::memory_order_release );
		Threads::Coro::SleepMsec ( uIntervalMs * 4 );

		EXPECT_TRUE ( bCommandWorkStarted.load ( std::memory_order_acquire ) );
		EXPECT_GE ( iAttempts.load ( std::memory_order_acquire ), 2 );
		EXPECT_TRUE ( tStream.StopAndHandoff () );
	} );
}


TEST ( api_reply_stream, writer_fast_handoff_before_activation )
{
	std::atomic<int> iAttempts { 0 };
	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		tOut.SetSendObserver ( [&iAttempts] ( const VecTraits_T<BYTE> & ) { iAttempts.fetch_add ( 1, std::memory_order_release ); } );
		ApiReplyStream_c tStream ( 100, 0x0123, tOut );
		tStream.Start ();
		Threads::Coro::SleepMsec ( 20 );
		EXPECT_EQ ( 0, iAttempts.load ( std::memory_order_acquire ) );
		EXPECT_TRUE ( tStream.StopAndHandoff () );
		tOut.MarkSessionReused ();
		Threads::Coro::SleepMsec ( 120 );
		EXPECT_EQ ( 0, iAttempts.load ( std::memory_order_acquire ) );
		EXPECT_FALSE ( tOut.WroteAfterReuse () );
	} );
}


TEST ( api_reply_stream, writer_activation_handoff_race )
{
	Threads::CallCoroutine ( []
	{
		for ( int i = 0; i<50; ++i )
		{
			ReplyStreamOutput_c tOut;
			ApiReplyStream_c tStream ( 2, 0x0123, tOut );
			tStream.Start ();
			Threads::Coro::SleepMsec ( 1 + i%2 );
			EXPECT_TRUE ( tStream.StopAndHandoff () );
			tOut.MarkSessionReused ();
			Threads::Coro::SleepMsec ( 3 );
			EXPECT_LE ( tOut.GetSendCalls (), 1 );
			EXPECT_FALSE ( tOut.WroteAfterReuse () );
		}
	} );
}


TEST ( api_reply_stream, writer_fast_scope_outlives_activation_timer )
{
	Threads::CallCoroutine ( []
	{
		for ( int i = 0; i<200; ++i )
		{
			ReplyStreamOutput_c tOut;
			ApiReplyStream_c tStream ( 20, 0x0123, tOut );
			tStream.Start ();
			EXPECT_TRUE ( tStream.StopAndHandoff () );
		}

		Threads::Coro::SleepMsec ( 40 );
	} );
}


TEST ( api_reply_stream, writer_sleeping_handoff_does_not_wait_for_deadline )
{
	constexpr uint32_t uIntervalMs = 100;
	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		ApiReplyStream_c tStream ( uIntervalMs, 0x0123, tOut );
		tStream.Start ();
		ASSERT_TRUE ( WaitForSendCount ( tOut, 1 ) );
		Threads::Coro::SleepMsec ( 5 );

		const int64_t iHandoffStartedUS = sphMicroTimer ();
		EXPECT_TRUE ( tStream.StopAndHandoff () );
		const int64_t iHandoffElapsedUS = sphMicroTimer () - iHandoffStartedUS;
		EXPECT_LT ( iHandoffElapsedUS, int64_t ( uIntervalMs ) * 500 );

		tOut.MarkSessionReused ();
		Threads::Coro::SleepMsec ( uIntervalMs + 20 );
		EXPECT_EQ ( 1, tOut.GetSendCalls () );
		EXPECT_FALSE ( tOut.WroteAfterReuse () );
	} );
}


TEST ( api_reply_stream, writer_exclusive_output_handoff )
{
	std::atomic<int> iActiveWriters { 0 };
	std::atomic<int> iMaxActiveWriters { 0 };
	std::atomic<int> iWriterMutations { 0 };
	Threads::Coro::Event_c tWriterEntered;
	Threads::Coro::Event_c tReleaseWriter;

	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut;
		tOut.SetSendObserver ( [&] ( const VecTraits_T<BYTE> & )
		{
			const int iActive = iActiveWriters.fetch_add ( 1, std::memory_order_acq_rel ) + 1;
			int iObservedMax = iMaxActiveWriters.load ( std::memory_order_acquire );
			while ( iObservedMax<iActive && !iMaxActiveWriters.compare_exchange_weak ( iObservedMax, iActive, std::memory_order_acq_rel ) )
			{}
			iWriterMutations.fetch_add ( 1, std::memory_order_release );
			tWriterEntered.SetEvent ();
			tReleaseWriter.WaitEvent ();
			iActiveWriters.fetch_sub ( 1, std::memory_order_release );
		} );
		ApiReplyStream_c tStream ( 100, 0x0123, tOut );

		tStream.Start ();
		tWriterEntered.WaitEvent ();
		EXPECT_EQ ( 1, iActiveWriters.load ( std::memory_order_acquire ) );
		EXPECT_EQ ( 5 * S2US, tOut.GetWTimeoutUS () );

		const int iComputedCommandResult = 42;
		Threads::StartJob ( [&tReleaseWriter]
		{
			Threads::Coro::SleepMsec ( 20 );
			tReleaseWriter.SetEvent ();
		}, GlobalWorkPool () );
		const int64_t iHandoffStartedUS = sphMicroTimer ();
		const bool bCanSerializeTerminal = tStream.StopAndHandoff ();
		const int64_t iHandoffElapsedUS = sphMicroTimer () - iHandoffStartedUS;

		EXPECT_EQ ( 0, iActiveWriters.load ( std::memory_order_acquire ) );
		EXPECT_EQ ( 1, iMaxActiveWriters.load ( std::memory_order_acquire ) );
		EXPECT_EQ ( 1, iWriterMutations.load ( std::memory_order_acquire ) );
		EXPECT_GE ( iHandoffElapsedUS, 20000 - sph::TICKS_GRANULARITY );
		EXPECT_EQ ( 42, iComputedCommandResult );
		EXPECT_TRUE ( bCanSerializeTerminal );
		EXPECT_EQ ( 5 * S2US, tOut.GetWTimeoutUS () );

		int iTerminalMutations = 0;
		if ( bCanSerializeTerminal )
			++iTerminalMutations;
		EXPECT_EQ ( 1, iTerminalMutations );
		EXPECT_TRUE ( tStream.StopAndHandoff () );
	} );
}


TEST ( api_reply_stream, writer_slow_sink_handoff )
{
	constexpr uint32_t uIntervalMs = 30;
	constexpr int64_t iSendDelayUS = int64_t ( uIntervalMs ) * 1000;
	std::atomic<int64_t> iObservedTimeoutUS { 0 };
	Threads::Coro::Event_c tSendEntered;

	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut ( 5 * S2US );
		tOut.FailSendsAfterPrefix ( 0 );
		tOut.SetSendObserver ( [&] ( const VecTraits_T<BYTE> & )
		{
			iObservedTimeoutUS.store ( tOut.GetWTimeoutUS (), std::memory_order_release );
			tSendEntered.SetEvent ();
			Threads::Coro::SleepMsec ( uIntervalMs );
		} );
		ApiReplyStream_c tStream ( uIntervalMs, 0x0123, tOut );

		tStream.Start ();
		tSendEntered.WaitEvent ();
		const int iComputedCommandResult = 42;
		const int64_t iHandoffStartedUS = sphMicroTimer ();
		const bool bCanSerializeTerminal = tStream.StopAndHandoff ();
		const int64_t iHandoffElapsedUS = sphMicroTimer () - iHandoffStartedUS;

		EXPECT_EQ ( 5 * S2US, iObservedTimeoutUS.load ( std::memory_order_acquire ) );
		EXPECT_GE ( iHandoffElapsedUS, iSendDelayUS - sph::TICKS_GRANULARITY );
		EXPECT_LE ( iHandoffElapsedUS, iSendDelayUS + 70000 );
		EXPECT_EQ ( 42, iComputedCommandResult );
		EXPECT_EQ ( 1, tOut.GetSendCalls () );
		EXPECT_EQ ( 5 * S2US, tOut.GetWTimeoutUS () );
		EXPECT_TRUE ( tOut.GetError () );
		EXPECT_FALSE ( bCanSerializeTerminal );
	} );
}


TEST ( api_reply_stream, writer_real_socket_master_close )
{
	int iRead = -1;
	int iWrite = -1;
	ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );
	ASSERT_EQ ( 0, sphSetSockNB ( iWrite ) );
	SafeCloseSocket ( iRead );

	Threads::CallCoroutine ( [&]
	{
		const int iOwnedSocket = iWrite;
		iWrite = -1;
		auto pBuffer = MakeAsyncNetBuffer ( std::make_unique<SockWrapper_c> ( iOwnedSocket, nullptr ) );
		pBuffer->SetWTimeoutUS ( 20000 );
		ApiReplyStream_c tStream ( 20, 0x0123, *pBuffer );

		tStream.Start ();
		const int iComputedCommandResult = 42;
		const int64_t iDeadlineUS = MonoMicroTimer () + 500000;
		auto & tOut = static_cast<GenericOutputBuffer_c &> ( *pBuffer );
		while ( !tOut.GetError () && MonoMicroTimer ()<iDeadlineUS )
			Threads::Coro::SleepMsec ( 1 );
		const bool bCanSerializeTerminal = tStream.StopAndHandoff ();

		EXPECT_EQ ( 42, iComputedCommandResult );
		EXPECT_TRUE ( tOut.GetError () );
		EXPECT_FALSE ( bCanSerializeTerminal );
		EXPECT_EQ ( 20000, pBuffer->GetWTimeoutUS () );
	} );

	SafeCloseSocket ( iRead );
	SafeCloseSocket ( iWrite );
}


TEST ( api_reply_stream, writer_successful_flush )
{
	Threads::CallCoroutine ( [&]
	{
		ReplyStreamOutput_c tOut ( 77777 );
		ApiReplyStream_c tStream ( 100, 0x0123, tOut );
		tStream.Start ();
		ASSERT_TRUE ( WaitForSendCount ( tOut, 1 ) );
		EXPECT_TRUE ( tStream.StopAndHandoff () );

		EXPECT_EQ ( 1, tOut.GetSendCalls () );
		EXPECT_EQ ( 8, tOut.GetTotalSent () );
		EXPECT_EQ ( 0, tOut.GetSentCount () );
		EXPECT_EQ ( 77777, tOut.GetWTimeoutUS () );
	} );
}


TEST ( api_reply_stream, writer_failed_flush_poison )
{
	constexpr BYTE dExpected[] = { 0x00, 0x04, 0x01, 0x23, 0x00, 0x00, 0x00, 0x00 };
	for ( int iFailurePrefix : { 0, 3 } )
	{
		int iCommandResult = 0;

		Threads::CallCoroutine ( [&]
		{
			ReplyStreamOutput_c tOut ( 77777 );
			tOut.FailSendsAfterPrefix ( iFailurePrefix );
			ApiReplyStream_c tStream ( 20, 0x0123, tOut );
			tStream.Start ();
			ASSERT_TRUE ( WaitForSendCount ( tOut, 1 ) );

			iCommandResult = 42;
			Threads::Coro::SleepMsec ( 60 );
			const bool bCanSerializeTerminal = tStream.StopAndHandoff ();

			EXPECT_EQ ( 42, iCommandResult );
			EXPECT_FALSE ( bCanSerializeTerminal );
			EXPECT_EQ ( 1, tOut.GetSendCalls () );
			EXPECT_EQ ( iFailurePrefix, tOut.GetTotalSent () );
			EXPECT_TRUE ( tOut.GetError () );
			EXPECT_EQ ( 77777, tOut.GetWTimeoutUS () );
			ASSERT_EQ ( int ( sizeof ( dExpected ) ), tOut.GetSentCount () );
			const auto * pReply = static_cast<const BYTE *> ( tOut.GetBufPtr () );
			for ( int i = 0; i < tOut.GetSentCount (); ++i )
				EXPECT_EQ ( dExpected[i], pReply[i] ) << "byte " << i;

			EXPECT_FALSE ( tStream.StopAndHandoff () );
			EXPECT_EQ ( 1, tOut.GetSendCalls () );
		} );
	}
}


TEST ( api_reply_stream, empty_in_progress_frame )
{
	constexpr WORD uReplyVersion = 0x0123;
	constexpr BYTE dExpected[] = { 0x00, 0x04, 0x01, 0x23, 0x00, 0x00, 0x00, 0x00 };

	ISphOutputBuffer tOut;
	{
		auto tReply = APIAnswer ( tOut, uReplyVersion, SEARCHD_IN_PROGRESS );
	}

	ASSERT_EQ ( int ( sizeof ( dExpected ) ), tOut.GetSentCount() );
	const auto * pReply = static_cast<const BYTE *> ( tOut.GetBufPtr() );
	for ( int i = 0; i < tOut.GetSentCount(); ++i )
		EXPECT_EQ ( dExpected[i], pReply[i] ) << "byte " << i;
}


TEST ( api_reply_stream, remote_heartbeat_state_configuration )
{
	CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };

	EXPECT_FALSE ( pAgent->ExpectsRemoteReplyHeartbeat () );
	pAgent->EnableRemoteReplyHeartbeats ();
	EXPECT_TRUE ( pAgent->ExpectsRemoteReplyHeartbeat () );

	AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
	AgentConnTestPeer_c::SetRetryBudget ( *pAgent, 3 );
	const int64_t iBeforeUS = MonoMicroTimer ();
	AgentConnTestPeer_c::AcceptHeartbeat ( *pAgent );
	const int64_t iAfterUS = MonoMicroTimer ();
	EXPECT_EQ ( 3, AgentConnTestPeer_c::GetRetryBudget ( *pAgent ) );
	EXPECT_EQ ( 250 * 1000, AgentConnTestPeer_c::GetLeasePeriodUS ( *pAgent ) );
	EXPECT_GE ( AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ), iBeforeUS + 250 * 1000 );
	EXPECT_LE ( AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ), iAfterUS + 250 * 1000 );
	AgentConnTestPeer_c::ResetForNextRequest ( *pAgent );
	EXPECT_TRUE ( pAgent->ExpectsRemoteReplyHeartbeat () );
}


TEST ( api_reply_stream, repeated_progress_replies )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	constexpr int64_t iQueryTimeoutMs = 250;
	for ( bool bFresh : { false, true } )
		for ( int iHeartbeats : { 0, 1, 3 } )
		{
			int iRead = -1;
			int iWrite = -1;
			ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

			ISphOutputBuffer tOut;
			if ( bFresh )
				tOut.SendDword ( SPHINX_SEARCHD_PROTO );
			for ( int i = 0; i < iHeartbeats; ++i )
				AddInProgressReply ( tOut, uReplyVersion );
			AddTerminalReply ( tOut, uReplyVersion, 42 );
			ASSERT_TRUE ( SendAll ( iWrite, tOut ) );

			CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
			pAgent->EnableRemoteReplyHeartbeats ();
			AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, iQueryTimeoutMs, 17, 19 );
			IntReplyParser_c tParser;
			const int64_t iBeforeUS = MonoMicroTimer ();
			EXPECT_TRUE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, bFresh ) )
				<< "fresh=" << bFresh << ", heartbeats=" << iHeartbeats;
			const int64_t iAfterUS = MonoMicroTimer ();
			EXPECT_EQ ( 1, tParser.m_iCalls );
			EXPECT_EQ ( 42, tParser.m_iValue );

			if ( iHeartbeats )
			{
				EXPECT_EQ ( iQueryTimeoutMs * 1000, AgentConnTestPeer_c::GetLeasePeriodUS ( *pAgent ) );
				EXPECT_GE ( AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ), iBeforeUS + iQueryTimeoutMs * 1000 );
				EXPECT_LE ( AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ), iAfterUS + iQueryTimeoutMs * 1000 );
			} else
			{
				EXPECT_EQ ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
				EXPECT_EQ ( 19, AgentConnTestPeer_c::GetLeasePeriodUS ( *pAgent ) );
			}
			SafeCloseSocket ( iWrite );
		}
}


TEST ( api_reply_stream, rejects_invalid_progress_reply )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	for ( bool bBody : { false, true } )
	{
		int iRead = -1;
		int iWrite = -1;
		ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

		ISphOutputBuffer tOut;
		AddInProgressReply ( tOut, uReplyVersion, bBody );
		ASSERT_TRUE ( SendAll ( iWrite, tOut ) );

		CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
		if ( bBody )
			pAgent->EnableRemoteReplyHeartbeats ();
		AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
		IntReplyParser_c tParser;
		EXPECT_FALSE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, false ) );
		EXPECT_EQ ( 0, tParser.m_iCalls );
		EXPECT_EQ ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
		EXPECT_EQ ( 19, AgentConnTestPeer_c::GetLeasePeriodUS ( *pAgent ) );
		SafeCloseSocket ( iWrite );
	}
}


TEST ( api_reply_stream, partial_progress_does_not_renew )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	int iRead = -1;
	int iWrite = -1;
	ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

	ISphOutputBuffer tOut;
	AddInProgressReply ( tOut, uReplyVersion );
	ASSERT_TRUE ( SendPrefix ( iWrite, tOut, tOut.GetSentCount () - 1 ) );
	SafeCloseSocket ( iWrite );

	CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
	pAgent->EnableRemoteReplyHeartbeats ();
	AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
	IntReplyParser_c tParser;
	EXPECT_FALSE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, false ) );
	EXPECT_EQ ( 0, tParser.m_iCalls );
	EXPECT_EQ ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
	EXPECT_EQ ( 19, AgentConnTestPeer_c::GetLeasePeriodUS ( *pAgent ) );
}


TEST ( api_reply_stream, every_header_split_and_adjacent_frame )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	for ( bool bFresh : { false, true } )
		for ( bool bProgressFirst : { false, true } )
		{
			const int iInitialHeaderBytes = bFresh ? 12 : 8;
			for ( int iSplit = 1; iSplit<iInitialHeaderBytes; ++iSplit )
			{
				int iRead = -1;
				int iWrite = -1;
				ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

				ISphOutputBuffer tOut;
				if ( bFresh )
					tOut.SendDword ( SPHINX_SEARCHD_PROTO );
				if ( bProgressFirst )
					AddInProgressReply ( tOut, uReplyVersion );
				AddTerminalReply ( tOut, uReplyVersion, 42 );
				ASSERT_TRUE ( SendRange ( iWrite, tOut, iSplit, tOut.GetSentCount () - iSplit ) );

				CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
				if ( bProgressFirst )
					pAgent->EnableRemoteReplyHeartbeats ();
				AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
				IntReplyParser_c tParser;
				auto * pCountingReporter = new CountingReporter_c;
				CSphRefcountedPtr<Reporter_i> pReporter { pCountingReporter };
				AgentConnTestPeer_c::SetReporter ( *pAgent, pReporter );
				const auto * pWire = static_cast<const BYTE *> ( tOut.GetBufPtr () );
				EXPECT_TRUE ( AgentConnTestPeer_c::ReceiveWithPrefix ( *pAgent, iRead, tParser, bFresh, pWire, iSplit ) )
					<< "fresh=" << bFresh << ", progress=" << bProgressFirst << ", split=" << iSplit;
				EXPECT_EQ ( 1, tParser.m_iCalls );
				EXPECT_EQ ( 42, tParser.m_iValue );
				EXPECT_EQ ( 1, pCountingReporter->m_iCalls );
				EXPECT_TRUE ( pCountingReporter->m_bSuccess );
				EXPECT_EQ ( bProgressFirst, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent )!=17 );
				SafeCloseSocket ( iWrite );
			}
		}
}


TEST ( api_reply_stream, rejects_invalid_terminal_lengths )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	for ( int iBodyLength : { -1, g_iMaxPacketSize + 1 } )
	{
		int iRead = -1;
		int iWrite = -1;
		ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

		ISphOutputBuffer tOut;
		AddRawReplyHeader ( tOut, uReplyVersion, iBodyLength );
		ASSERT_TRUE ( SendAll ( iWrite, tOut ) );

		CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
		IntReplyParser_c tParser;
		EXPECT_FALSE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, false ) );
		EXPECT_EQ ( 0, tParser.m_iCalls );
		SafeCloseSocket ( iWrite );
	}
}


TEST ( api_reply_stream, eof_before_and_after_progress )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	for ( int iHeartbeats : { 0, 1 } )
	{
		int iRead = -1;
		int iWrite = -1;
		ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

		ISphOutputBuffer tOut;
		for ( int i = 0; i<iHeartbeats; ++i )
			AddInProgressReply ( tOut, uReplyVersion );
		ASSERT_TRUE ( SendAll ( iWrite, tOut ) );
		SafeCloseSocket ( iWrite );

		CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
		pAgent->EnableRemoteReplyHeartbeats ();
		AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
		IntReplyParser_c tParser;
		EXPECT_FALSE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, false ) );
		EXPECT_EQ ( 0, tParser.m_iCalls );
		if ( iHeartbeats )
			EXPECT_NE ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
		else
			EXPECT_EQ ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
	}
}


TEST ( api_reply_stream, query_timeout_before_and_after_progress )
{
	constexpr WORD uReplyVersion = 0x0123;
	for ( int iHeartbeats : { 0, 2 } )
	{
		CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
		pAgent->EnableRemoteReplyHeartbeats ();
		AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
		AgentConnTestPeer_c::SetRetryBudget ( *pAgent, 3 );
		for ( int i = 0; i<iHeartbeats; ++i )
			AgentConnTestPeer_c::AcceptHeartbeat ( *pAgent );

		EXPECT_EQ ( 3, AgentConnTestPeer_c::GetRetryBudget ( *pAgent ) );
		if ( iHeartbeats )
			EXPECT_NE ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );
		else
			EXPECT_EQ ( 17, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent ) );

		AgentConnTestPeer_c::ArmHardQueryTimeoutWithoutRetry ( *pAgent );
		CSphRefcountedPtr<AgentConn_t> pKeepAlive = pAgent;
		pAgent->TimeoutCallback ();
		EXPECT_STREQ ( "query timed out", pAgent->m_sFailure.cstr () );
	}
}


TEST ( api_reply_stream, empty_terminal_reply )
{
#if _WIN32
	GTEST_SKIP () << "direct ReceiveAnswer() harness bypasses the Windows IOCP poller; scheduler-path coverage follows";
#endif
	constexpr WORD uReplyVersion = 0x0123;
	for ( bool bFresh : { false, true } )
		for ( int iHeartbeats : { 0, 3 } )
		{
			int iRead = -1;
			int iWrite = -1;
			ASSERT_TRUE ( CreateTestSocketPair ( iRead, iWrite ) );

			ISphOutputBuffer tOut;
			if ( bFresh )
				tOut.SendDword ( SPHINX_SEARCHD_PROTO );
			for ( int i = 0; i < iHeartbeats; ++i )
				AddInProgressReply ( tOut, uReplyVersion );
			AddEmptyTerminalReply ( tOut, uReplyVersion );
			ASSERT_TRUE ( SendAll ( iWrite, tOut ) );

			CSphRefcountedPtr<AgentConn_t> pAgent { new AgentConn_t };
			pAgent->EnableRemoteReplyHeartbeats ();
			AgentConnTestPeer_c::SetLeaseProbe ( *pAgent, 250, 17, 19 );
			EmptyReplyParser_c tParser;
			EXPECT_TRUE ( AgentConnTestPeer_c::Receive ( *pAgent, iRead, tParser, bFresh ) )
				<< "fresh=" << bFresh << ", heartbeats=" << iHeartbeats;
			EXPECT_EQ ( 1, tParser.m_iCalls );
			EXPECT_EQ ( iHeartbeats!=0, AgentConnTestPeer_c::GetLeaseDeadlineUS ( *pAgent )!=17 );
			SafeCloseSocket ( iWrite );
		}
}


TEST ( api_reply_stream, scheduler_path_byte_split_progress_replies )
{
	constexpr WORD uReplyVersion = 0x0123;
	ISphOutputBuffer tReply;
	tReply.SendDword ( SPHINX_SEARCHD_PROTO );
	for ( int i = 0; i<3; ++i )
		AddInProgressReply ( tReply, uReplyVersion );
	AddTerminalReply ( tReply, uReplyVersion, 42 );

	IntReplyParser_c tParser;
	ScheduledReplyResult_t tResult;
	ASSERT_TRUE ( RunScheduledReply ( tReply, tParser, true, tResult, true ) )
		<< "server stage=" << tResult.m_iServerErrorStage << ", socket error=" << tResult.m_iServerSocketError
		<< ", agent failure=" << tResult.m_sFailure.cstr ();
	EXPECT_EQ ( 1, tResult.m_iSucceeded );
	EXPECT_TRUE ( tResult.m_bAgentSuccess );
	EXPECT_TRUE ( tResult.m_sFailure.IsEmpty () );
	EXPECT_EQ ( 1, tParser.m_iCalls );
	EXPECT_EQ ( 42, tParser.m_iValue );
}


TEST ( api_reply_stream, scheduler_path_rejects_invalid_progress_reply )
{
	constexpr WORD uReplyVersion = 0x0123;
	ISphOutputBuffer tReply;
	tReply.SendDword ( SPHINX_SEARCHD_PROTO );
	AddInProgressReply ( tReply, uReplyVersion, true );

	IntReplyParser_c tParser;
	ScheduledReplyResult_t tResult;
	ASSERT_TRUE ( RunScheduledReply ( tReply, tParser, true, tResult ) )
		<< "server stage=" << tResult.m_iServerErrorStage << ", socket error=" << tResult.m_iServerSocketError
		<< ", agent failure=" << tResult.m_sFailure.cstr ();
	EXPECT_EQ ( 0, tResult.m_iSucceeded );
	EXPECT_FALSE ( tResult.m_bAgentSuccess );
	EXPECT_EQ ( 0, tParser.m_iCalls );
	EXPECT_TRUE ( tResult.m_sFailure.Begins ( "invalid in-progress reply size" ) )
		<< "agent failure=" << tResult.m_sFailure.cstr ();
}


TEST ( api_reply_stream, scheduler_path_empty_terminal_reply )
{
	constexpr WORD uReplyVersion = 0x0123;
	ISphOutputBuffer tReply;
	tReply.SendDword ( SPHINX_SEARCHD_PROTO );
	AddInProgressReply ( tReply, uReplyVersion );
	AddEmptyTerminalReply ( tReply, uReplyVersion );

	EmptyReplyParser_c tParser;
	ScheduledReplyResult_t tResult;
	ASSERT_TRUE ( RunScheduledReply ( tReply, tParser, true, tResult, true ) )
		<< "server stage=" << tResult.m_iServerErrorStage << ", socket error=" << tResult.m_iServerSocketError
		<< ", agent failure=" << tResult.m_sFailure.cstr ();
	EXPECT_EQ ( 1, tResult.m_iSucceeded );
	EXPECT_TRUE ( tResult.m_bAgentSuccess );
	EXPECT_TRUE ( tResult.m_sFailure.IsEmpty () );
	EXPECT_EQ ( 1, tParser.m_iCalls );
}


TEST ( api_reply_stream, scheduler_path_rejects_empty_terminal_with_required_body )
{
	constexpr WORD uReplyVersion = 0x0123;
	ISphOutputBuffer tReply;
	tReply.SendDword ( SPHINX_SEARCHD_PROTO );
	AddInProgressReply ( tReply, uReplyVersion );
	AddEmptyTerminalReply ( tReply, uReplyVersion );

	IntReplyParser_c tParser;
	ScheduledReplyResult_t tResult;
	ASSERT_TRUE ( RunScheduledReply ( tReply, tParser, true, tResult, true ) )
		<< "server stage=" << tResult.m_iServerErrorStage << ", socket error=" << tResult.m_iServerSocketError
		<< ", agent failure=" << tResult.m_sFailure.cstr ();
	EXPECT_EQ ( 0, tResult.m_iSucceeded );
	EXPECT_FALSE ( tResult.m_bAgentSuccess );
	EXPECT_EQ ( 1, tParser.m_iCalls );
	EXPECT_TRUE ( tResult.m_sFailure.Begins ( "failed to parse remote reply: read overflows buffer" ) )
		<< "agent failure=" << tResult.m_sFailure.cstr ();
}
