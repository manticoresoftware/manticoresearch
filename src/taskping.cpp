//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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

	void BuildRequest ( const AgentConn_t&, CachedOutputBuffer_c& tOut ) const final
	{
		// API header
		APICommand_t tWr { tOut, SEARCHD_COMMAND_PING, VER_COMMAND_PING };
		tOut.SendInt ( m_iSendCookie );
	}

	bool ParseReply ( MemInputBuffer_c& tReq, AgentConn_t& ) const final
	{
		m_iReceivedCookie = tReq.GetInt ();
		return true;
	}

private:
	~PingBuilder_c () final
	{}

private:
	const int m_iSendCookie;
	mutable int m_iReceivedCookie = 0;
};

static auto& g_bShutdown = sphGetShutdown ();
void SchedulePing ( HostDashboard_t* pHost );

static void PingWorker ( void* pCookie )
{
	ThreadSystem_t tThdSystemDesc ( "PING" );
	CSphRefcountedPtr<HostDashboard_t> pHost (( HostDashboard_t* ) pCookie );
	if ( g_bShutdown || pHost->m_iNeedPing<1 )
		return;

	auto iEngage = pHost->EngageTime ();
	auto iNow = sphMicroTimer ();
	if (( iEngage - 1000 )>iNow ) // more time to wait (round to 1ms); reschedule and return
	{
		SchedulePing ( pHost );
		return;
	}

	// it is time to ping. Make the connection and schedule the command.
	// prepare the agent
	using AgentConnRefPtr_t = CSphRefcountedPtr<AgentConn_t>;
	AgentConnRefPtr_t pConn { new AgentConn_t };
	pConn->m_tDesc.CloneFromHost ( pHost->m_tHost );
	assert ( !pHost->m_tHost.m_pDash );

	// fixme! Review the timeouts (g_iPingInterval for both came from legacy)
	pConn->m_iMyConnectTimeout = g_iPingInterval/1000;
	pConn->m_iMyQueryTimeout = g_iPingInterval/1000;
	pConn->m_tDesc.m_pDash = pHost;

	// Run network task
//	sphWarning ( "Ping %s", pConn->m_tDesc.GetMyUrl ().cstr ());
	CSphRefcountedPtr<PingBuilder_c> pPinger ( new PingBuilder_c (( int ) iNow ));
	RunRemoteTask ( pConn, pPinger, pPinger, [ pPinger, pHost ] ( bool b ) { SchedulePing (pHost); } );
}

static void PingAborter ( void* pCookie )
{
	auto* pHost = ( HostDashboard_t* ) pCookie;
	SafeRelease ( pHost );
}

void SchedulePing ( HostDashboard_t* pHost )
{
	if ( !pHost )
		return;

	static int iPingTask = -1;
	if ( iPingTask<0 )
		iPingTask = TaskManager::RegisterGlobal ( "Ping service", PingWorker, PingAborter );
	assert ( iPingTask>=0 && "failed to create ping service task" );
	SafeAddRef ( pHost );
	TaskManager::ScheduleJob ( iPingTask, pHost->EngageTime (), pHost );
}

class Pinger_c: public IPinger
{
public:
	void Subscribe ( HostDashboard_t* pHost ) final
	{
		if ( !pHost )
			return;

		if ( pHost->m_iNeedPing<1 )
			return;

		SchedulePing ( pHost );
	};
};

void Ping::Start()
{
	static Pinger_c dPinger;
	SetGlobalPinger ( &dPinger );
}