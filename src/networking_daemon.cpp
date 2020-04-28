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
#include "coroutine.h"

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
static auto& g_bGotSigterm = sphGetGotSigterm ();

/////////////////////////////////////////////////////////////////////////////
/// CSphWakeupEvent - used to kick poller from outside
/////////////////////////////////////////////////////////////////////////////

// event that wakes-up poll net loop from finished thread pool job
CSphWakeupEvent::CSphWakeupEvent ()
	: PollableEvent_t()
	, ISphNetAction ( m_iPollablefd )
{
	m_uNetEvents = NetPollEvent_t::READ;
}

CSphWakeupEvent::~CSphWakeupEvent ()
{
	Close ();
	m_iSock = -1;
}

void CSphWakeupEvent::Wakeup ()
{
	if ( FireEvent () )
		return;
	int iErrno = PollableErrno ();
	sphLogDebugv ( "failed to wakeup net thread ( error %d,'%s')", iErrno, strerrorm ( iErrno ) );
}

NetEvent_e CSphWakeupEvent::Process ( DWORD uGotEvents, CSphNetLoop * )
{
	if ( uGotEvents & NetPollEvent_t::READ )
		DisposeEvent();
	return NE_KEEP;
}


#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////
/// CSphNetLoop - main poller. Used for serving accepts and all socket operations
/////////////////////////////////////////////////////////////////////////////

ThreadRole PollThread; // thread which serves network polling

class CSphNetLoop::Impl_c
{
	// since it is impl, everything is private and accessible by friendship
	friend class CSphNetLoop;

	DWORD &							m_uTick;
	CSphNetLoop *					m_pParent;

	CSphVector<ISphNetAction *> 	m_dWorkInternal;
	CSphVector<ISphNetAction *>		m_dWorkExternal GUARDED_BY ( m_tExtLock );
	volatile bool					m_bGotExternal = false;
	CSphWakeupEvent *				m_pWakeup = nullptr;
	CSphMutex						m_tExtLock;
	LoopProfiler_t					m_tPrf;
	CSphScopedPtr<NetPooller_c>		m_pPoll;
	void *							m_pShutdownCookie = nullptr;

	explicit Impl_c ( const VecTraits_T<Listener_t> & dListeners, DWORD& uTick, CSphNetLoop* pParent )
		: m_uTick ( uTick )
		, m_pParent ( pParent )
		, m_pPoll { new NetPooller_c ( 1000 )}
	{
		CSphScopedPtr<CSphWakeupEvent> pWakeup ( new CSphWakeupEvent );
		if ( pWakeup->IsPollable() )
		{
			m_pWakeup = pWakeup.LeakPtr ();
			sphLogDebugvv ( "Setup wakeup as %d, %d", m_pWakeup->m_iSock, (int) m_pWakeup->m_iTimeoutTimeUS );
			m_pPoll->SetupEvent ( m_pWakeup );
			m_pShutdownCookie = searchd::AddShutdownCb ( [this] { m_pShutdownCookie = nullptr; Kick (); } );
		} else
			sphWarning ( "net-loop use timeout due to %s", pWakeup->m_sError.cstr () );

		Threads::AloneShutdowncatch ();

		for ( const auto & dListener : dListeners )
		{
			auto * pCur = new NetActionAccept_c ( dListener );
			sphLogDebugvv ( "setup listener as %d, %d", pCur->m_iSock, (int) pCur->m_iTimeoutTimeUS );
			m_pPoll->SetupEvent ( pCur );
		}

		m_dWorkExternal.Reserve ( 1000 );
		m_dWorkInternal.Reserve ( 1000 );
	}

	~Impl_c()
	{
		sphLogDebugv ( "~CSphNetLoop::Impl_c (%p) invoked", this );
		searchd::DeleteShutdownCb ( m_pShutdownCookie );
		ScopedMutex_t tExtLock ( m_tExtLock );
		for ( auto* pWork : m_dWorkExternal )
			pWork->Destroy ();

		m_pPoll->ForAll ( [] ( NetPollEvent_t * pWork ) { ((ISphNetAction *) pWork)->Destroy (); } );
	}

	// add actions planned by jobs
	void PickNewActions () REQUIRES ( PollThread )
	{
		if ( !m_bGotExternal )
			return;

		m_bGotExternal = false;
		m_tPrf.StartExt ();
		ScopedMutex_t tExtLock ( m_tExtLock );
		m_tPrf.m_iPerfExt = m_dWorkExternal.GetLength ();
		assert ( m_dWorkInternal.IsEmpty ());
		m_dWorkInternal.SwapData ( m_dWorkExternal );
		m_tPrf.EndTask ();
	}

	int ProcessReady () REQUIRES ( PollThread )
	{
		int iConnections = 0;
		int iMaxIters = 0;

		for ( NetPollEvent_t & dReady : *m_pPoll.Ptr() )
		{
			if ( g_iThrottleAction && iMaxIters>=g_iThrottleAction )
				break;
			m_tPrf.StartAt ();
			assert ( dReady.m_uNetEvents );
			auto pWork = (ISphNetAction *) &dReady;
			m_pPoll->DeactivateTimer ( pWork ); // ensure that timer (if any) will no more fire

			NetEvent_e eEv = pWork->Process ( dReady.m_uNetEvents, m_pParent );
			++m_tPrf.m_iPerfEv;
			++iMaxIters;

			if ( eEv==NE_ACCEPT )
				iConnections += ( (NetActionAccept_c *) pWork )->GetStats ();

			m_tPrf.EndTask ();
		}
		// update stats
		if ( iConnections )
			g_tStats.m_iConnections += iConnections;
		return iMaxIters;
	}

	void Poll ( int64_t tmLastWait ) REQUIRES ( PollThread )
	{
		// lets spin net-loop thread without syscall\sleep\wait up to net_wait period
		// in case we got events recently or call job that might finish early
		// otherwise poll ( 1 ) \ epoll_wait ( 1 ) put off this thread and introduce some latency, ie
		// sysbench test with 1 thd and 3 empty indexes reports:
		// 3k qps for net-loop without spin-wait
		// 5k qps for net-loop with spin-wait
		int iWaitMs = 0;
		if ( g_tmWait==-1 || ( g_tmWait>0 && sphMicroTimer ()-tmLastWait>I64C( 10000 ) * g_tmWait ))
			iWaitMs = m_pWakeup ? WAIT_UNTIL_TIMEOUT : 1;

		m_tPrf.StartPoll ();
		// need positive timeout for communicate threads back and shutdown
		m_pPoll->Wait ( iWaitMs );
		m_tPrf.EndTask ();
	}

	void LoopNetPoll () REQUIRES ( PollThread )
	{
		int64_t tmLastWait = sphMicroTimer();
		while ( !g_bShutdown )
		{
			m_tPrf.Start();

			Poll ( tmLastWait );
			++m_uTick;

			// add actions planned by jobs
			PickNewActions ();

			// handle events and collect stats
			m_tPrf.StartTick();
			sphLogDebugv ( "got events=%d, tick=%u", m_pPoll->GetNumOfReady (), m_uTick );
			auto iProcessed = ProcessReady();
			m_tPrf.EndTask();

			// setup or refresh handlers
			m_tPrf.StartNext();
			m_dWorkInternal.Apply ( [&] ( ISphNetAction * pWork ) REQUIRES ( PollThread )
			{
				assert ( pWork && pWork->m_iSock>=0 );
				m_pPoll->SetupEvent ( pWork );
			});
			m_dWorkInternal.Resize ( 0 );
			m_tPrf.EndTask();

			// will remove outdated even if they're just added (to avoid polling them)
			iProcessed += RemoveOutdated ();

			if ( iProcessed )
				tmLastWait = sphMicroTimer();
			m_tPrf.End();
		}
	}

	int RemoveOutdated () REQUIRES ( PollThread )
	{
		int64_t tmNow = sphMicroTimer();
		m_tPrf.StartRemove();
		int iRemoved = 0;

		// remove outdated items on no signals
		m_pPoll->ForAll ([&] ( NetPollEvent_t * pEvent )
		{
			auto * pWork = (ISphNetAction *) pEvent;

			// skip eternal (non-timeouted)
			if ( pWork->m_iTimeoutIdx<0 || pWork->m_iTimeoutTimeUS<=0 || !TimeoutReached (pWork->m_iTimeoutTimeUS, tmNow))
				return;

			sphLogDebugv ( "%p bailing on timeout no signal, sock=%d", pWork, pWork->m_iSock );
			m_pPoll->RemoveEvent ( pWork );
			pWork->Process ( NetPollEvent_t::TIMEOUT, m_pParent );
			++iRemoved;
		 });
		m_tPrf.EndTask();
		return iRemoved;
	}

	void Kick ()
	{
		sphLogDebugvv ( "Kick" );
		if ( m_pWakeup )
			m_pWakeup->Wakeup ();
	}

	void AddAction ( ISphNetAction * pElem )
	{
		sphLogDebugvv ( "AddAction action as %d, events %d, timeout %d",
				pElem->m_iSock, pElem->m_uNetEvents, (int) pElem->m_iTimeoutTimeUS );
		ScopedMutex_t tExtLock ( m_tExtLock );
		m_bGotExternal = true;
		m_dWorkExternal.Add ( pElem );
		Kick();
	}

	// mark externally destroyed action as removed - to avoid race call
	void Unlink ( ISphNetAction * pElem, bool bWillClose )
	{
		m_pPoll->Unlink ( pElem, bWillClose );
	}

	void RemoveIterEvent ( NetPollEvent_t * pEvent ) REQUIRES ( PollThread )
	{
		m_pPoll->RemoveEvent ( pEvent );
	}
};

/////////////////////////////////////////////////////////////////////////////

CSphNetLoop::CSphNetLoop ( const VecTraits_T<Listener_t> & dListeners )
{
	m_pImpl = new Impl_c ( dListeners, m_uTick, this );
}

CSphNetLoop::~CSphNetLoop ()
{
	SafeDelete ( m_pImpl );
	sphLogDebugv ( "~CSphNetLoop() (%p) completed", this );
}

void CSphNetLoop::LoopNetPoll () REQUIRES ( PollThread )
{
	assert ( m_pImpl );
	m_pImpl->LoopNetPoll();
}

void CSphNetLoop::AddAction ( ISphNetAction * pElem )
{
	assert ( m_pImpl );
	m_pImpl->AddAction ( pElem );
}

void CSphNetLoop::Unlink ( ISphNetAction * pElem, bool bWillClose )
{
	assert ( m_pImpl );
	m_pImpl->Unlink ( pElem, bWillClose );
}

void CSphNetLoop::RemoveIterEvent ( NetPollEvent_t * pEvent ) REQUIRES ( PollThread )
{
	assert ( m_pImpl );
	m_pImpl->RemoveIterEvent ( pEvent );
}

void ServeNetLoop ( const VecTraits_T<Listener_t> & dListeners )
{
	ScopedRole_c thPoll ( PollThread );
	CSphNetLoop ( dListeners ).LoopNetPoll ();
}

/////////////////////////////////////////////////////////////////////////////
/// SockWrapper_c::Impl_c internal async socket implementation
/////////////////////////////////////////////////////////////////////////////
class SockWrapper_c::Impl_c final : public ISphNetAction
{
	friend class SockWrapper_c;
	CSphNetLoop * m_pNetLoop;
	Handler m_fnRestart = nullptr;
	bool m_bKeep;

	Impl_c ( int iSocket, bool bKeep, CSphNetLoop * pNetLoop );
	~Impl_c () final;
	int SockPoll ( int64_t tmTimeUntil, bool bWrite );
	int64_t SockRecv ( char * pBuf, int64_t iLeftBytes );
	int64_t SockSend ( const char * pBuf, int64_t iLeftBytes );

	void ResumeWaiterIfAny();
	void EngageWaiterAndYield( int64_t tmTimeUntil );

public:
	NetEvent_e Process ( DWORD uGotEvents, CSphNetLoop * ) REQUIRES ( PollThread ) final;
	void Destroy () final;
};

SockWrapper_c::Impl_c::Impl_c ( int iSocket, bool bKeep, CSphNetLoop * pNetLoop )
	: ISphNetAction ( iSocket )
	, m_pNetLoop ( pNetLoop )
	, m_bKeep ( bKeep )
{}

SockWrapper_c::Impl_c::~Impl_c ()
{
	if ( m_pNetLoop )
		m_pNetLoop->Unlink (this, !m_bKeep);
	if ( !m_bKeep && m_iSock>=0 )
	{
		sphLogDebugv ( "closing sock=%d", m_iSock );
		sphSockClose ( m_iSock );
	}
}

// this is non-blocking awaiter. Continuation is scheduled, but not run right in current thread.
void SockWrapper_c::Impl_c::ResumeWaiterIfAny ()
{
	assert ( m_fnRestart && "Socket which must be disabled, fired an event" );
	Handler fnRestart = std::move( m_fnRestart);
	sphLogDebugv ( "SockWrapper_c::Loop continuation" );
	fnRestart ();
}

// this is blocking function. Aware, that current thread may change when it finished.
void SockWrapper_c::Impl_c::EngageWaiterAndYield ( int64_t tmTimeUntilUs )
{
	assert ( m_pNetLoop );
	sphLogDebugv ( "CoYieldWith (m_iEvent=%d), timeout %d", m_uNetEvents, int(tmTimeUntilUs-sphMicroTimer ()) );
	m_iTimeoutTimeUS = tmTimeUntilUs;
	m_fnRestart = Threads::CurrentRestarter ();
	Threads::CoYieldWith ( [this] { m_pNetLoop->AddAction ( this ); } );
	sphLogDebugv ( "EngageWaiterAndYield awake (m_iSock=%d, events=%d)", m_iSock, m_uNetEvents );
}

//  called from netloop
NetEvent_e SockWrapper_c::Impl_c::Process ( DWORD uGotEvents, CSphNetLoop * ) REQUIRES ( PollThread )
{
	if ( CheckSocketError ( uGotEvents ) && ( m_iTimeoutIdx!=-1 ) ) // we were enqueued
		m_pNetLoop->RemoveIterEvent ( this );

	m_uNetEvents = uGotEvents;
	ResumeWaiterIfAny ();
	return NE_KEEP;
}

// special destroy rules for socket wrapper:
// 1. Unlink from the poller.
// 2. Resume continuation, if any.
// 3. Leave socket as is (continuation would manage it itself).
void SockWrapper_c::Impl_c::Destroy ()
{
	m_iTimeoutIdx = -1;
	if ( m_pNetLoop )
		m_pNetLoop->Unlink ( this, true );
	m_pNetLoop = nullptr;
	m_uNetEvents = TIMEOUT;
	sphSockSetErrno ( EINTR );
	if (m_fnRestart)
		ResumeWaiterIfAny();
}

// as usual sphPoll - returns 1 on success, 0 on timeout, -1 on error.
int SockWrapper_c::Impl_c::SockPoll ( int64_t tmTimeUntilUs, bool bWrite )
{
	if ( !m_pNetLoop ) // single thread
	{
		int64_t tmMicroLeft = ( tmTimeUntilUs-sphMicroTimer () );
		if ( tmMicroLeft<0 )
			tmMicroLeft = 0;
		auto iRes = sphPoll ( m_iSock, tmMicroLeft, bWrite );
		sphLogDebugv ( "sphPoll for alone returned %d", iRes );
		return iRes;
	}

	// thread pool
	m_uNetEvents = NetPollEvent_t::ONCE | ( bWrite ? NetPollEvent_t::WRITE : NetPollEvent_t::READ );
	EngageWaiterAndYield ( tmTimeUntilUs );
	if ( m_uNetEvents == NetPollEvent_t::TIMEOUT )
		return 0;
	if ( CheckSocketError ( m_uNetEvents ) )
		return -1;
	return 1;
}

int64_t SockWrapper_c::Impl_c::SockSend ( const char * pBuf, int64_t iLeftBytes )
{
	return sphSockSend ( m_iSock, pBuf, iLeftBytes );
}

int64_t SockWrapper_c::Impl_c::SockRecv ( char * pBuf, int64_t iLeftBytes )
{
	sphLogDebugvv ( "SockRecv %d, for " INT64_FMT " bytes", m_iSock, iLeftBytes );
	return sphSockRecv ( m_iSock, pBuf, iLeftBytes );
}

/////////////////////////////////////////////////////////////////////////////
/// SockWrapper_c frontend implementation
/////////////////////////////////////////////////////////////////////////////

SockWrapper_c::SockWrapper_c ( int iSocket, bool bKeep, CSphNetLoop * pNetLoop )
		: m_pImpl ( new Impl_c ( iSocket, bKeep, pNetLoop ) )
{}

SockWrapper_c::~SockWrapper_c ()
{
	SafeDelete ( m_pImpl );
}

int64_t SockWrapper_c::SockSend ( const char * pData, int64_t iLen )
{
	assert ( m_pImpl );
	return m_pImpl->SockSend ( pData, iLen );
}

int64_t SockWrapper_c::SockRecv ( char * pData, int64_t iLen )
{
	assert ( m_pImpl );
	return m_pImpl->SockRecv ( pData, iLen );
}

int SockWrapper_c::SockPoll ( int64_t tmTimeUntilUs, bool bWrite )
{
	assert ( m_pImpl );
	return m_pImpl->SockPoll ( tmTimeUntilUs, bWrite );
}

/////////////////////////////////////////////////////////////////////////////
/// Helpers
/////////////////////////////////////////////////////////////////////////////

// Send a blob into socket.
// Alone worker will use waiting in poll.
// Cooperative worker will yield and resume instead of waiting.
// timeout is ruled by g_iWriteTimeoutS.
static bool SyncSend ( SockWrapper_c* pSock, const char * pBuffer, int64_t iLen )
{
	if ( g_bShutdown )
		sphLogDebug ( "SIGTERM in SockWrapper_c::Send" );

	if ( iLen<=0 )
		return false;

	sphLogDebugv ( "AsyncSend " INT64_FMT " bytes", iLen );

	auto iTimeoutUntilUs = sphMicroTimer () + S2US * g_iWriteTimeoutS; // separate g_iClientQlTimeout?
	do
	{
		auto iRes = pSock->SockSend ( pBuffer, iLen );
		if ( iRes<0 )
		{
			int iErrno = sphSockGetErrno ();
			if ( iErrno==EINTR ) // interrupted before any data was sent; just loop
				continue;
			if ( iErrno!=EAGAIN && iErrno!=EWOULDBLOCK ) {
				sphWarning ( "send() failed: %d: %s", iErrno, sphSockError ( iErrno ));
				return false;
			}
		} else {
			if ( iLen==iRes )
				return true; // we're finished
			iLen -= iRes;
			pBuffer += iRes;
		}

		sphLogDebugv ("Still need to send " INT64_FMT " bytes...", iLen );
	} while (pSock->SockPoll ( iTimeoutUntilUs, true ) );
	sphWarning ( "timed out while performing SyncSend to flush network buffers" );
	return false;
}

// fetch a chunk of bytes from socket and adjust position/rest of bytes
static int AsyncRecvNBChunk ( SockWrapper_c * pSock, BYTE *& pBuf, int & iLeftBytes )
{
	// try to receive next chunk
	auto iRes = pSock->SockRecv ( (char*) pBuf, iLeftBytes );
	sphLogDebugv ( "=========== AsyncRecvNBChunk " INT64_FMT " when read %d bytes", iRes, iLeftBytes );

	if ( iRes>0 )
	{
		pBuf += iRes;
		iLeftBytes -= iRes;
	}
	return (int) iRes;
}

#if USE_WINDOWS
#define EMULATE_EINTR 1
#endif

//#define EMULATE_EINTR 1

// flexible receive data from socket. iLen indicates, how many bytes to read. iSpace - how many is _safe_ to read.
// (i.e. if you want 1 byte and space for 100 - you can read up to 100 bytes, but not 101).
static int SyncSockRead ( SockWrapper_c * pSock, BYTE* pBuf, int iLen, int iSpace, int iReadTimeoutS, bool bIntr )
{
	assert ( iSpace>=iLen );

	// try to receive available chunk
	int iReceived = AsyncRecvNBChunk ( pSock, pBuf, iSpace );
	sphLogDebugv ( "AsyncRecvNBChunk %d bytes (%d requested)", iReceived, iLen );
	if ( iReceived>=iLen ) // all, and m.b. more read in one-shot
		return iReceived;

	// immediate error (most probably it is E_AGAIN; check!)
	if ( iReceived<0 )
		iReceived = 0;

	iLen -= iReceived;

	if ( !iLen )
		return iReceived;

	int64_t tmMaxTimer = sphMicroTimer() + S2US * Max( 1, iReadTimeoutS ); // in microseconds

	int iErr = 0;
	int iRes = -1;

	while ( iLen>0 )
	{
		int64_t tmNextStopUs = tmMaxTimer;

#if EMULATE_EINTR
		// Windows EINTR emulation
		// Ctrl-C will not interrupt select on Windows, so let's handle that manually
		// forcibly limit select() to 100 ms, and check flag afterwards
		if ( bIntr )
			tmNextStopUs = Min ( tmMaxTimer, sphMicroTimer () + 100000 );
#endif

		if ( ( tmNextStopUs - sphMicroTimer() )<=0 )
			break; // timed out

		// wait until there is data
		sphLogDebugv ( "Still need to receive %d bytes...", iLen );
		iRes = pSock->SockPoll ( tmNextStopUs, false );

		// if there was EINTR, retry
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				if ( !( g_bGotSigterm && bIntr ))
					continue;
				sphLogDebug( "SyncSockRead: select got SIGTERM, exit -1" );
			}
			return -1;
		}

		// if there was a timeout, report it as an error
		if ( iRes==0 )
		{
#if EMULATE_EINTR
			// EINTR emulation
			if ( bIntr )
			{
				// got that SIGTERM
				if ( g_bGotSigterm )
				{
					sphLogDebug ( "SyncSockRead: got SIGTERM emulation on Windows, exit -1" );
					sphSockSetErrno ( EINTR );
					return -1;
				}

				// timeout might not be fully over just yet, so re-loop
				continue;
			}
#endif
			sphLogDebugv ( "return TIMEOUT..." );
			sphSockSetErrno( ETIMEDOUT );
			return -1;
		}

		// try to receive next chunk
		iRes = AsyncRecvNBChunk( pSock, pBuf, iSpace );
		sphLogDebugv ( "SyncSockRead: AsyncRecvNBChunk returned %d", iRes );

		// if there was eof, we're done
		if ( !iRes )
		{
			sphLogDebugv ( "SyncSockRead: connection reset" );
			sphSockSetErrno( ECONNRESET );
			return -1;
		}

		// if there was EINTR, retry
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				if ( !( g_bGotSigterm && bIntr ))
					continue;
				sphLogDebug( "SyncSockRead: select got SIGTERM, exit -1" );
			}
			return -1;
		}

		iReceived += iRes;
		iLen-= iRes;

		// avoid partial buffer loss in case of signal during the 2nd (!) read
		bIntr = false;
	}

	// if there was a timeout, report it as an error
	if ( iLen>0 )
	{
		sphSockSetErrno( ETIMEDOUT );
		return -1;
	}

	return iReceived;
}

/////////////////////////////////////////////////////////////////////////////
/// AsyncNetInputBuffer_c
/////////////////////////////////////////////////////////////////////////////

AsyncNetInputBuffer_c::AsyncNetInputBuffer_c ()
	: STORE ( NET_MINIBUFFER_SIZE )
	, InputBuffer_c ( m_pData, NET_MINIBUFFER_SIZE )
{
	Resize ( 0 );
	m_iLen = 0;
}

Proto_e AsyncNetInputBuffer_c::Probe ( int iHardLimit, bool bLight )
{
	Proto_e eResult = Proto_e::UNKNOWN;
	m_bIntr = false;
	int iRest;
	if ( !HasBytes() )
	{
		iRest = GetRoomForTail ( iHardLimit );
		if ( !iRest )
			return eResult; // hard limit reached
		AppendData ( 0, Min ( iRest, 4096 ), 0, true );
	}

	auto iHas = HasBytes();
	if (!iHas)
	{
		if ( bLight )
		{
			sphLogDebugv ( "+++++ Light probing revealed nothing, bail" );
			return eResult;
		}
		sphLogDebugv ( "+++++ Light probing revealed nothing, try blocking" );
		AppendData ( 1, Min ( iRest, 4096 ), g_iReadTimeoutS, true );
		iHas = HasBytes ();
	}

	StringBuilder_c sBytes;
	auto tBlob = Tail ();
	if ( tBlob.second >=4 )
	{
		if ( !memcmp (tBlob.first,"\0\0\0\1",4) )
		{
			sBytes << "SphinxAPI, usual byte order";
			eResult = Proto_e::SPHINX;
		}
		else if ( !memcmp ( tBlob.first, "GET", 3)
				|| !memcmp ( tBlob.first, "POST", 4 )
				|| !memcmp ( tBlob.first, "PUT", 3 )
				|| !memcmp ( tBlob.first, "DELE", 4 ) )
		{
			eResult = Proto_e::HTTP;
			sBytes << "HTTP";
		}
		else if ( !memcmp ( tBlob.first, "\1\0\0\0", 4 ) )
		{
			sBytes << "SphinxAPI, inversed byte order";
			eResult = Proto_e::SPHINX;
		}
		else
		{
			eResult = Proto_e::HTTPS; // m.b. more accurate probe on ssl header, but not important right now
			sBytes << "Unknown, assume HTTPS";
		}
	} else
	{
		sBytes.StartBlock ( " ", "Short [", "]" );
		for ( int i = 0; i<tBlob.second; ++i )
			sBytes << tBlob.first[i];
		sBytes.FinishBlocks();
	}

	sphLogDebugv ( "+++++ Probing revealed %d bytes: %s", iHas, sBytes.cstr() );
	return eResult;
}

bool AsyncNetInputBuffer_c::ReadFrom( int iLen, int iTimeoutS, bool bIntr )
{
	m_bIntr = false;
	if ( iLen<=0 || iLen>g_iMaxPacketSize )
		return false;

	auto iRest = iLen - HasBytes();
	if ( iRest<=0 ) // lazy case: prev ReadFrom already achieved requested bytes
		return true;

	m_bError = AppendData ( iRest, iRest, iTimeoutS, bIntr )<iRest;
	return !m_bError;
}

// ensure iSpace bytes in buffer, then read at least iNeed, up to vector's GetLimit().
// returns -1 on error, or N of appended bytes.
int AsyncNetInputBuffer_c::AppendData ( int iNeed, int iSpace, int iTimeoutS, bool bIntr )
{
	assert ( iNeed<=iSpace );
	auto iPos = DiscardAndReserve ( int ( m_pCur-m_pBuf ), GetLength()+iSpace );
	iSpace = GetLimit ()-GetLength ();
	m_pBuf = ByteBlob_t ( *this ).first; // realign after possible reserve, byteblob ensures it is not nullptr
	m_pCur = m_pBuf+iPos;

	int iGot = ReadFromBackend ( AddN(0), iNeed, iSpace, iTimeoutS, bIntr );
	if ( g_bGotSigterm && bIntr )
	{
		sphLogDebug ( "AsyncNetInputBuffer_c::AppendData: got SIGTERM, return -1" );
		m_bError = true;
		m_bIntr = true;
		return -1;
	}

	if ( iGot==-1 )
	{
		m_bError = true;
		auto iErr = sphSockPeekErrno ();
		m_bIntr = iErr==EINTR;
		sphLogDebug ( "AsyncNetInputBuffer_c::AppendData: error %d (%s) return -1", iErr, strerrorm ( iErr ) );
		return -1;
	}

	AddN ( iGot );
	m_iLen = GetLength();
	m_bIntr = false;
	return iGot;
}

int AsyncNetInputBuffer_c::ReadAny ( int iHardLimit, int iTimeoutS )
{
	m_bIntr = false;

	auto iRest = GetRoomForTail ( iHardLimit );
	if ( !iRest )
		return 0;

	return AppendData ( 1, Min ( iRest, 4096 ), iTimeoutS, true );
}


ByteBlob_t AsyncNetInputBuffer_c::Tail ()
{
	return { m_pCur, HasBytes ()};
}

ByteBlob_t AsyncNetInputBuffer_c::PopTail ( int iSize )
{
	const BYTE * pBuf = nullptr;
	auto iBufLen = HasBytes ();
	if ( iSize>=0 )
	{
		assert ( iSize <= iBufLen );
		iBufLen = iSize;
	}
	if ( iBufLen>0 && GetBytesZerocopy ( &pBuf, iBufLen ) )
		return { pBuf, iBufLen };
	else
		return { nullptr, 0 };
}

int AsyncNetInputBuffer_c::GetRoomForTail ( int iHardLimit )
{
	if ( iHardLimit-m_iLen<=0 )
		DiscardProcessed ( -1 );

	return iHardLimit-m_iLen;
}

void AsyncNetInputBuffer_c::DiscardProcessed ( int iHowMany )
{
	auto iPos = int ( m_pCur-m_pBuf );
	assert ( m_iLen == GetLength() );
	assert ( iHowMany >=-1 ); // we don't even expect values less then -1.
	assert ( iPos <= m_iLen );
	auto iOldLen = m_iLen;

	switch ( iHowMany ) {
		case 0:
			if ( iPos==m_iLen ) {
				Resize ( 0 );
				iHowMany = iPos;
			}
			break;
		case -1: iHowMany = iPos;
		default: Remove ( iPos-iHowMany, iHowMany );
	}

	m_pCur -= iHowMany;
	m_iLen = STORE::GetLength();
	sphLogDebugv ( "DiscardProcessed(%d) iPos=%d->0, iLen=%d->%d",
			iHowMany, (int)iPos, iOldLen, m_iLen );
}

BYTE AsyncNetInputBuffer_c::Terminate ( int iPos, BYTE uNewVal )
{
	auto pPos = m_pCur + iPos;
	auto pLimit = m_pData + GetLimit();
	if ( pPos >= pLimit ) // no place for terminator
	{
		auto iIdx = m_pCur-m_pBuf;
		ReserveGap(1);
		m_pBuf = m_pData;
		m_pCur = m_pBuf+iIdx;
		pPos = m_pCur+iPos;
	}

	auto uOld = *pPos;
	*const_cast<BYTE*>(pPos) = uNewVal;
	return uOld;
}

class AsyncSockInputBuffer_c : public AsyncNetInputBuffer_c
{
	SockWrapperPtr_c m_pSocket;

	int ReadFromBackend ( BYTE * pBuf, int iNeed, int iHaveSpace, int iReadTimeoutS, bool bIntr ) final
	{
		assert ( iNeed<=iHaveSpace );
		return SyncSockRead ( m_pSocket, pBuf, iNeed, iHaveSpace, iReadTimeoutS, bIntr );
	}

public:
	explicit AsyncSockInputBuffer_c ( SockWrapperPtr_c pSock )
		: m_pSocket ( std::move ( pSock ) )
	{}
};

/////////////////////////////////////////////////////////////////////////////
/// AsyncNetOutputBuffer_c
/////////////////////////////////////////////////////////////////////////////

/// Flush works via generic socket wrapper netloop
class AsyncNetOutputBuffer_c final : public NetGenericOutputBuffer_c
{
	SockWrapperPtr_c m_pSocket;

public:
	explicit AsyncNetOutputBuffer_c ( SockWrapperPtr_c pSock )
			: m_pSocket ( std::move ( pSock ) )
	{}

	/// Flush works via federeate netloop
	void Flush () final
	{
		assert ( m_pSocket );
		CommitAllMeasuredLengths ();
		if ( m_dBuf.IsEmpty() )
			return; // nothing to send
		CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_NET_WRITE );
		m_bError = !SyncSend ( m_pSocket, (const char *) m_dBuf.begin (), m_dBuf.GetLength64 () );
		m_dBuf.Resize ( 0 );
	}
};

/////////////////////////////////////////////////////////////////////////////
/// AsyncBufferedSocket_c - provides wrapper for sending and receiving
/////////////////////////////////////////////////////////////////////////////

class AsyncBufferedSocket_c : public AsyncNetBuffer_c
{
	AsyncSockInputBuffer_c m_tReceiver;
	AsyncNetOutputBuffer_c m_tSender;

public:
	explicit AsyncBufferedSocket_c ( SockWrapperPtr_c && pSock ) : m_tReceiver ( pSock ), m_tSender ( pSock ) {}
	AsyncNetInputBuffer_c & In () override { return m_tReceiver; }
	NetGenericOutputBuffer_c & Out () override { return m_tSender; }
};

// main fabric
AsyncNetBufferPtr_c MakeAsyncNetBuffer ( SockWrapperPtr_c pSock )
{
	return AsyncNetBufferPtr_c ( new AsyncBufferedSocket_c ( std::move ( pSock )));
}