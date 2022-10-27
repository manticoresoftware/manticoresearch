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

#include "taskping.h"
#include "searchdha.h"
#include "searchdtask.h"

// Ping API proto
class PingBuilder_c final : public RequestBuilder_i, public ReplyParser_i, public ISphRefcountedMT
{
public:
	explicit PingBuilder_c ( int iCookie )
		: m_iSendCookie ( iCookie )
	{}

	void BuildRequest ( const AgentConn_t&, ISphOutputBuffer& tOut ) const final
	{
		// API header
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_PING, VER_COMMAND_PING );
		tOut.SendInt ( m_iSendCookie );
	}

	bool ParseReply ( MemInputBuffer_c& tReq, AgentConn_t& tConn ) const final
	{
		m_iReceivedCookie = tReq.GetInt ();
		tConn.m_tDesc.m_pDash->m_uPingTripUS = sphMicroTimer()-m_iReceivedCookie;
		return true;
	}

private:
	~PingBuilder_c () final = default;

private:
	const int m_iSendCookie;
	mutable int m_iReceivedCookie = 0;
};

void SchedulePing ( HostDashboardRefPtr_t pHost )
{
	static int iPingTask = TaskManager::RegisterGlobal ( "Ping service" );
	assert ( iPingTask>=0 && "failed to create ping service task" );
	TaskManager::ScheduleJob ( iPingTask, pHost->EngageTime (), [pHost]
	{
		auto pDesc = PublishSystemInfo ( "PING" );
		if ( sphInterrupted() || pHost->m_iNeedPing < 1 )
			return;

		auto iEngage = pHost->EngageTime();
		auto iNow = sphMicroTimer();

		// check host engage time (which is linked to last answer time) and don't ping
		// if time is not exceeded (that is, if host is under usual load, we don't waste network for pings)
		if ( !sph::TimeExceeded ( iEngage, iNow ) )
			return SchedulePing ( pHost );

		// it is time to ping. Make the connection and schedule the command.
		// prepare the agent
		using AgentConnRefPtr_t = CSphRefcountedPtr<AgentConn_t>;
		AgentConnRefPtr_t pConn { new AgentConn_t };
		pConn->m_tDesc.CloneFromHost ( pHost->m_tHost );
		assert ( !pHost->m_tHost.m_pDash );

		// fixme! Review the timeouts (g_iPingIntervalUs for both came from legacy)
		pConn->m_iMyConnectTimeoutMs = int ( g_iPingIntervalUs / 1000 );
		pConn->m_iMyQueryTimeoutMs = g_iPingIntervalUs / 1000;
		pConn->m_tDesc.m_pDash = pHost;

		// Run network task
		//	sphWarning ( "Ping %s", pConn->m_tDesc.GetMyUrl ().cstr ());
		// todo! we send current time and receive it back, but don't use it. We can compare upon receiving with current time and calculate round-trip time, that looks like quite useful metric!
		CSphRefcountedPtr<PingBuilder_c> pPinger { new PingBuilder_c ( (int)iNow ) };
		RunRemoteTask ( pConn, pPinger, pPinger, [pPinger, pHost] ( bool ) { SchedulePing ( pHost ); } );
	});
}

class Pinger_c: public IPinger
{
public:
	void Subscribe ( HostDashboardRefPtr_t pHost ) final
	{
		if ( pHost && pHost->m_iNeedPing >= 1 )
			SchedulePing ( pHost );
	}
};

void Ping::Start()
{
	static Pinger_c dPinger;
	SetGlobalPinger ( &dPinger );
}
