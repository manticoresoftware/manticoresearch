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

#include "net_action_accept.h"
#include "netstate_api.h"
#include "netreceive_api.h"
#include "netreceive_ql.h"
#include "netreceive_http.h"
#include "coroutine.h"

#if _WIN32
// Win-specific headers and calls
#include <io.h>
using sph_sa_family_t=ADDRESS_FAMILY;
#else
// UNIX-specific headers and calls
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

using sph_sa_family_t = sa_family_t;
#endif

int g_iThrottleAccept = 0;
extern volatile bool g_bMaintenance;

void FormatClientAddress ( char szClientName[SPH_ADDRPORT_SIZE], const sockaddr_storage & saStorage )
{
	// format client address
	szClientName[0] = '\0';
	if ( saStorage.ss_family==AF_INET )
	{
		struct sockaddr_in * pSa = ( (struct sockaddr_in *) &saStorage );
		sphFormatIP ( szClientName, SPH_ADDRESS_SIZE, pSa->sin_addr.s_addr );

		char * d = szClientName;
		while ( *d )
			d++;
		snprintf ( d, 7, ":%d", (int) ntohs ( pSa->sin_port ) ); //NOLINT
	} else if ( saStorage.ss_family==AF_UNIX )
	{
		strncpy ( szClientName, "(local)", 8 );
	}
}

using NetConnection_t = std::pair<int, sph_sa_family_t>;

void SetTcpNodelay ( NetConnection_t tConn )
{
	if ( tConn.second==AF_INET )
		sphSetSockNodelay ( tConn.first );
}

void MultiServe ( std::unique_ptr<AsyncNetBuffer_c> pBuf, NetConnection_t tConn, Proto_e eProto )
{
	auto& tSess = session::Info();
	tSess.SetProto ( eProto ); // set initially provided proto, then m.b. switch to another by multi, if possible

	Proto_e eMultiProto;
	switch ( eProto )
	{
		case Proto_e::SPHINXSE: eMultiProto = Proto_e::SPHINXSE; break; // force sphinx SE
		default:
			eMultiProto = pBuf->Probe ( g_iMaxPacketSize, false );
	}

	switch ( eMultiProto )
	{
	case Proto_e::SPHINXSE:
	// case of legacy 'crasy squirell' client, which talks using short packages.
		if ( pBuf->HasBytes ()==4 )
			SetTcpNodelay ( tConn );
		// no break;
		// [[clang::fallthrough]];
	case Proto_e::SPHINX:
		ApiServe ( std::move ( pBuf ));
		break;
	case Proto_e::HTTPS:
		tSess.SetSsl ( true );
		// [[clang::fallthrough]];
	case Proto_e::HTTP:
		SetTcpNodelay ( tConn );
		HttpServe ( std::move ( pBuf ) );
		break;
	default:
		sphLogDebugv ( "Unkown proto" );
		break;
	}
}

class NetActionAccept_c::Impl_c
{
	Listener_t		m_tListener;

public:
	explicit Impl_c ( const Listener_t & tListener ) : m_tListener ( tListener ) {}
	void ProcessAccept ( DWORD uGotEvents, CSphNetLoop * pLoop );
};

static DWORD NextConnectionID()
{
	static std::atomic<DWORD> g_iConnectionID { 1 };        ///< global conn-id
	return g_iConnectionID.fetch_add ( 1, std::memory_order_relaxed );
}

void NetActionAccept_c::Impl_c::ProcessAccept ( DWORD uGotEvents, CSphNetLoop * _pLoop )
{
	if ( CheckSocketError ( uGotEvents ) || sphInterrupted () )
		return;

	// handle all incoming requests at once but not too much
	int iAccepted = 0;
	auto _ = AtScopeExit([&iAccepted] {	gStats().m_iConnections.fetch_add ( iAccepted, std::memory_order_relaxed ); });
	sockaddr_storage saStorage = {0};
	socklen_t uLength = sizeof(saStorage);

	CSphRefcountedPtr<CSphNetLoop> pLoop { _pLoop };
	SafeAddRef (_pLoop);
	while (true)
	{
		if ( g_iThrottleAccept && g_iThrottleAccept<iAccepted )
		{
			sphLogDebugv ( "%p accepted %d connections throttled", this, iAccepted );
			return;
		}

		// accept
		int iClientSock = accept ( m_tListener.m_iSock, (struct sockaddr *)&saStorage, &uLength );

		// handle failures and no more incoming clients
		if ( iClientSock<0 )
		{
			const int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR || iErrno==ECONNABORTED || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			{
				if ( iAccepted )
					sphLogDebugv ( "%p accepted %d connections all, tick=%u", this, iAccepted, myinfo::ref<ListenTaskInfo_t> ()->m_uTick );
				return;
			}

			if ( iErrno==EMFILE || iErrno==ENFILE )
			{
				sphWarning ( "accept() failed, raise ulimit -n and restart searchd: %s", sphSockError(iErrno) );
				return;
			}

			sphFatal ( "accept() failed: %s", sphSockError(iErrno) );
		}

		if ( sphSetSockNB ( iClientSock )<0 )
		{
			sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
			sphSockClose ( iClientSock );
			return;
		}

		if ( g_bMaintenance && !m_tListener.m_bVIP )
		{
			sphWarning ( "server is in maintenance mode: refusing connection" );
			sphSockClose ( iClientSock );
			return;
		}

		++iAccepted;
		int iConnID = NextConnectionID();

		/*
 * Modes of execution:
 * - usual: default scheduler + non-zero netloop. Polling performed by netloop; working by thread pool.
 * - vip: alone scheduler and zero netloop. All work (polling and calculations) performed by dedicated alone thread.
 */
		auto pClientNetLoop = pLoop;
		using SchedulerFabric_fn = std::function<Threads::Scheduler_i*( void )>;
		SchedulerFabric_fn fnMakeScheduler = nullptr;
		if ( m_tListener.m_bVIP )
		{
			pClientNetLoop = nullptr;
			// fixme! for now pass -1, which means 'no limit N of workers'. M.b. need to obey max_children here.
			fnMakeScheduler = [] { sphLogDebugv ( "-~-~-~-~-~-~-~-~ Alone sched created -~-~-~-~-~-~-~-~" ); return MakeSingleThreadExecutor ( -1 ); };
		} else
		{
			fnMakeScheduler = [] { sphLogDebugv ( "-~-~-~-~-~-~-~-~ MT sched created -~-~-~-~-~-~-~-~" ); return GlobalWorkPool (); };
		}

		char szClientName[SPH_ADDRPORT_SIZE];
		FormatClientAddress ( szClientName, saStorage );

		auto pClientInfo = std::make_unique<ClientTaskInfo_t>();
		pClientInfo->SetClientName ( szClientName );
		pClientInfo->SetConnID ( iConnID );
		pClientInfo->SetVip ( m_tListener.m_bVIP );
		pClientInfo->SetReadOnly( m_tListener.m_bReadOnly );

		NetConnection_t tConn = { iClientSock, saStorage.ss_family };
		auto pBuf = MakeAsyncNetBuffer ( std::make_unique<SockWrapper_c> ( iClientSock, pClientNetLoop ) );

		auto eProto = m_tListener.m_eProto;
		switch ( eProto )
		{
			case Proto_e::SPHINX:
			case Proto_e::SPHINXSE:
			case Proto_e::HTTPS:
			case Proto_e::HTTP :
			{
				Threads::Coro::Go ( [pRawBuf = pBuf.release(), tConn, _pInfo = pClientInfo.release(), eProto ] () mutable
					{
						ScopedClientInfo_t pInfo { _pInfo }; // make visible task info
						MultiServe ( std::unique_ptr<AsyncNetBuffer_c> ( pRawBuf ), tConn, eProto );
					}, fnMakeScheduler () );
				break;
			}
			case Proto_e::MYSQL41:
			{
				Threads::Coro::Go ( [pRawBuf = pBuf.release(), _pInfo = pClientInfo.release() ] () mutable
					{
						ScopedClientInfo_t pInfo { _pInfo }; // make visible task info
						SqlServe ( std::unique_ptr<AsyncNetBuffer_c> ( pRawBuf ) );
					}, fnMakeScheduler () );
				break;
			}
			case Proto_e::REPLICATION:
				assert (false && "replication must be processed on another level");
				break;

			default:
				break;
		}
		sphLogDebugv ( "%p accepted %s, sock=%d, tick=%u", this,
				RelaxedProtoName(m_tListener.m_eProto), iClientSock, myinfo::ref<ListenTaskInfo_t> ()->m_uTick );
	}
}

NetActionAccept_c::NetActionAccept_c ( const Listener_t & tListener )
	: ISphNetAction ( tListener.m_iSock )
	, m_pImpl ( std::make_unique<Impl_c> ( tListener ) )
{
	m_uNetEvents = NetPollEvent_t::READ;
}

NetActionAccept_c::~NetActionAccept_c () = default;

void NetActionAccept_c::Process ( DWORD uGotEvents, CSphNetLoop * pLoop )
{
	m_pImpl->ProcessAccept ( uGotEvents, pLoop );
}

