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

#include "networking_daemon.h"
#include "loop_profiler.h"
#include "net_action_accept.h"

#if USE_WINDOWS
// Win-specific headers and calls
#include <io.h>
#else
// UNIX-specific headers and calls
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#endif

int g_tmWait = -1;
int	g_iThrottleAction = 0;
static auto& g_bShutdown = sphGetShutdown ();

// event that wakes-up poll net loop from finished thread pool job
CSphWakeupEvent::CSphWakeupEvent ()
	: PollableEvent_t()
	, ISphNetAction ( m_iPollablefd )
{}

CSphWakeupEvent::~CSphWakeupEvent ()
{
	CloseSocket();
}

void CSphWakeupEvent::Wakeup ()
{
	if ( FireEvent () )
		return;
	int iErrno = PollableErrno ();
	sphLogDebugv ( "failed to wakeup net thread ( error %d,'%s')", iErrno, strerrorm ( iErrno ) );
}

NetEvent_e CSphWakeupEvent::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> &, CSphNetLoop * )
{
	if ( uGotEvents & NE_IN )
		DisposeEvent();
	return NE_KEEP;
}

NetEvent_e CSphWakeupEvent::Setup ( int64_t )
{
	sphLogDebugv ( "%p wakeup setup, read=%d, write=%d", this, m_iPollablefd, m_iSignalEvent );
	return NE_IN;
}

void CSphWakeupEvent::CloseSocket ()
{
	Close();
	*const_cast<int *>( &m_iSock ) = -1;
}



struct ThdJobCleanup_t final : public ISphJob
{
	CSphVector<ISphNetAction *> m_dCleanup;

	explicit ThdJobCleanup_t ( CSphVector<ISphNetAction *>&& dCleanup )
		: m_dCleanup ( std::move ( dCleanup ))
	{}
	~ThdJobCleanup_t () final { Clear(); }
	void				Call () final { Clear(); };
	void				Clear()
	{
		if ( g_eLogLevel>=SPH_LOG_VERBOSE_DEBUG && m_dCleanup.GetLength() )
		{
			StringBuilder_c sTmp;
			ARRAY_FOREACH ( i, m_dCleanup )
				sTmp.Sprintf ( "%p(%d), ", m_dCleanup[i], m_dCleanup[i]->m_iSock );

			sphLogDebugv ( "cleaned jobs(sock)=%d, %s", m_dCleanup.GetLength (), sTmp.cstr ());
		}

		ARRAY_FOREACH ( i, m_dCleanup )
			SafeDelete ( m_dCleanup[i] );

		m_dCleanup.Reset();
	}
};


class CSphNetLoop::Impl_c
{
	// since it is impl, everything is private and accessible by friendship
	friend class CSphNetLoop;

	DWORD &							m_uTick;
	CSphNetLoop *					m_pParent;

	CSphVector<ISphNetAction *>		m_dWorkExternal GUARDED_BY ( m_tExtLock );
	volatile bool					m_bGotExternal = false;
	CSphWakeupEvent *				m_pWakeupExternal = nullptr; // FIXME!!! owned\deleted by event loop
	CSphMutex						m_tExtLock;
	LoopProfiler_t					m_tPrf;
	ISphNetPoller *					m_pPoll = nullptr;
	void *							m_pShutdownCookie = nullptr;

	explicit Impl_c ( CSphVector<Listener_t> & dListeners, DWORD& uTick, CSphNetLoop* pParent )
		: m_uTick ( uTick )
		, m_pParent ( pParent )
	{
		m_pPoll = sphCreatePoll ( 1000 );
		int64_t tmNow = sphMicroTimer();
		CSphScopedPtr<CSphWakeupEvent> pWakeup ( new CSphWakeupEvent );
		if ( pWakeup->IsPollable() )
		{
			m_pWakeupExternal = pWakeup.LeakPtr ();
			SetupEvent ( m_pWakeupExternal, tmNow );
			m_pShutdownCookie = searchd::AddShutdownCb ( [this] { Kick (); } );
		} else
			sphWarning ( "net-loop use timeout due to %s", pWakeup->m_sError.cstr () );

		for ( const auto & dListener : dListeners )
		{
			auto * pCur = new NetActionAccept_c ( dListener );
			SetupEvent ( pCur, tmNow );
		}

		m_dWorkExternal.Reserve ( 1000 );
	}

	~Impl_c()
	{
		searchd::DeleteShutdownCb ( m_pShutdownCookie );
		ScopedMutex_t tExtLock ( m_tExtLock );
		for ( auto* pWork : m_dWorkExternal )
			SafeDelete ( pWork );
		m_pPoll->ForAll ( [] ( NetPollEvent_t * pWork )
		{
			SafeDelete ( pWork );
		} );
		SafeDelete ( m_pPoll );
	}

	void SetupEvent ( ISphNetAction * pWork, int64_t tmNow )
	{
		assert ( m_pPoll );
		assert ( pWork && pWork->m_iSock>=0 );

		NetEvent_e eSetup = pWork->Setup ( tmNow );
		assert ( eSetup==NE_IN || eSetup==NE_OUT );
		pWork->m_uNetEvents = ( eSetup==NE_IN ? NetPollEvent_t::READ : NetPollEvent_t::WRITE );
		m_pPoll->SetupEvent ( pWork );
	}

	void LoopNetPoll ()
	{
		CSphVector<ISphNetAction *> dWorkNext;
		dWorkNext.Reserve ( 1000 );
		int64_t tmLastWait = sphMicroTimer();

		while ( !g_bShutdown )
		{
			m_tPrf.Start();

			// lets spin net-loop thread without syscall\sleep\wait up to net_wait period
			// in case we got events recently or call job that might finish early
			// otherwise poll ( 1 ) \ epoll_wait ( 1 ) put off this thread and introduce some latency, ie
			// sysbench test with 1 thd and 3 empty indexes reports:
			// 3k qps for net-loop without spin-wait
			// 5k qps for net-loop with spin-wait
			int iSpinWait = 0;
			if ( g_tmWait==-1 || ( g_tmWait>0 && sphMicroTimer()-tmLastWait>I64C(10000)*g_tmWait ) )
				iSpinWait = m_pWakeupExternal ? WAIT_UNTIL_TIMEOUT : 1;

			m_tPrf.StartPoll();
			// need positive timeout for communicate threads back and shutdown
			bool bGot = m_pPoll->Wait ( iSpinWait );
			m_tPrf.EndTask();

			++m_uTick;

			// try to remove outdated items on no signals
			if ( !bGot && !m_bGotExternal )
			{
				AddCleanupAction ( RemoveOutdated () );
				m_tPrf.End();
				continue;
			}

			// add actions planned by jobs
			if ( m_bGotExternal )
			{
				m_tPrf.StartExt();
				ScopedMutex_t tExtLock ( m_tExtLock );
				m_tPrf.m_iPerfExt = m_dWorkExternal.GetLength();
				assert ( dWorkNext.IsEmpty() );
				dWorkNext.SwapData ( m_dWorkExternal );
				m_bGotExternal = false;
				m_tPrf.EndTask();
			}

			// handle events and collect stats
			m_tPrf.StartTick();

			int iGotEvents = m_pPoll->GetNumOfReady ();
			sphLogDebugv ( "got events=%d, tick=%u", iGotEvents, m_uTick );
			int iConnections = 0;
			int iMaxIters = 0;

			CSphVector<ISphNetAction *> dCleanup;
			dCleanup.Reserve ( 1000 );
			for ( NetPollEvent_t & dReady : *m_pPoll )
			{
				if ( g_iThrottleAction && iMaxIters>=g_iThrottleAction )
					break;
				m_tPrf.StartAt();
				dReady.m_uNetEvents &= NE_MASK; // filter out possible non-desirable bits
				assert ( dReady.m_uNetEvents );
				auto pWork = (ISphNetAction *) &dReady;

				NetEvent_e eEv = pWork->Loop ( dReady.m_uNetEvents, dWorkNext, m_pParent );
				pWork->GetStats ( iConnections );
				++m_tPrf.m_iPerfEv;
				++iMaxIters;

				if ( eEv==NE_KEEP )
				{
					m_tPrf.EndTask();
					continue;
				}

				m_tPrf.StartAr();
				if ( eEv==NE_REMOVE || eEv==NE_REMOVED )
				{
					if ( eEv==NE_REMOVE )
						m_pPoll->RemoveEvent ( &dReady );
					dCleanup.Add ( pWork );
				} else
				{
					m_pPoll->ChangeEvent ( &dReady,
							eEv==NE_IN ? NetPollEvent_t::READ : NetPollEvent_t::WRITE);
				}
				m_tPrf.EndTask();
				m_tPrf.EndTask();
			}
			m_tPrf.EndTask();

			dCleanup.Append ( RemoveOutdated () );
			AddCleanupAction ( std::move ( dCleanup ));

			m_tPrf.StartNext();
			int64_t	tmNow = sphMicroTimer();
			// setup new handlers
			dWorkNext.Apply ( [&] ( ISphNetAction * pWork ) { SetupEvent ( pWork, tmNow ); } );
			m_tPrf.m_iPerfNext = dWorkNext.GetLength();
			dWorkNext.Resize ( 0 );
			m_tPrf.EndTask();

			// update stats
			if ( iConnections )
			{
				m_tPrf.StartStat();
				g_tStats.m_iConnections += iConnections;
				m_tPrf.EndTask();
			}

			tmLastWait = sphMicroTimer();
			m_tPrf.End();
		}
	}

	CSphVector<ISphNetAction *> RemoveOutdated ()
	{
		CSphVector<ISphNetAction *> dCleanup;
		int64_t tmNow = sphMicroTimer();
		m_tPrf.StartRemove();

		// remove outdated items on no signals
		m_pPoll->ForAll ([&] ( NetPollEvent_t * pEvent )
		{
			auto * pWork = (ISphNetAction *) pEvent;
			if ( pWork->m_iTimeoutTime<=0 || tmNow<pWork->m_iTimeoutTime )
				return;

			sphLogDebugv ( "%p bailing on timeout no signal, sock=%d", pWork, pWork->m_iSock );
			m_pPoll->RemoveEvent ( pWork );

			// close socket immediately to prevent write by client into persist connection that just timed out
			// that does not need in case Work got removed at IterateRemove + SafeDelete ( pWork );
			// but deferred clean up by ThdJobCleanup_t needs to close socket here right after
			// it was removed from e(poll) set
			pWork->CloseSocket ();
			dCleanup.Add ( pWork );
		 });
		m_tPrf.EndTask();
		return dCleanup;
	}

	void AddCleanupAction ( CSphVector<ISphNetAction *>&& dCleanup )
	{
		if ( dCleanup.IsEmpty() )
			return;

		m_tPrf.m_iPerfClean = dCleanup.GetLength();
		m_tPrf.StartClean();
		g_pThdPool->AddJob ( new ThdJobCleanup_t ( std::move ( dCleanup )));
		m_tPrf.EndTask();
	}

	void Kick ()
	{
		if ( m_pWakeupExternal )
			m_pWakeupExternal->Wakeup ();
	}

	void AddAction ( ISphNetAction * pElem )
	{
		ScopedMutex_t tExtLock ( m_tExtLock );
		m_bGotExternal = true;
		m_dWorkExternal.Add ( pElem );
		Kick();
	}

	void RemoveIterEvent ( NetPollEvent_t * pEvent )
	{
		m_pPoll->RemoveEvent ( pEvent );
	}
};

CSphNetLoop::CSphNetLoop ( CSphVector<Listener_t> & dListeners )
{
	m_pImpl = new Impl_c ( dListeners, m_uTick, this );
}

CSphNetLoop::~CSphNetLoop ()
{
	SafeDelete ( m_pImpl );
}

void CSphNetLoop::LoopNetPoll ()
{
	assert ( m_pImpl );
	m_pImpl->LoopNetPoll();
}

void CSphNetLoop::Kick ()
{
	assert ( m_pImpl );
	m_pImpl->Kick ();
}

void CSphNetLoop::AddAction ( ISphNetAction * pElem )
{
	assert ( m_pImpl );
	m_pImpl->AddAction ( pElem );
}

void CSphNetLoop::RemoveIterEvent ( NetPollEvent_t * pEvent )
{
	assert ( m_pImpl );
	m_pImpl->RemoveIterEvent ( pEvent );
}

void ServeNetLoop ( CSphVector<Listener_t> & dListeners )
{
	CrashQuery_t tQueryTLS;
	SphCrashLogger_c::SetTopQueryTLS ( &tQueryTLS );
	CSphNetLoop ( dListeners ).LoopNetPoll ();
}