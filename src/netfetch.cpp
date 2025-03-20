//
// Copyright (c) 2022-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "netfetch.h"
#include "sphinxutils.h"
#include "coroutine.h"
#include "networking_daemon.h"

#define LOG_LEVEL_MULTIINFO false
#define LOG_LEVEL_TIMERCB ( LOG_LEVEL_MULTIINFO && false )
#define LOG_LEVEL_CULRSOCKET false
#define LOG_LEVEL_CURLEASY false
#define LOG_LEVEL_CB false

#define CURL_VERBOSE false
#define CURL_VERBOSE_CB ( CURL_VERBOSE && false )
#define CURL_PROGRESS false

/// usually DEBUG, but can be WARNING for easier bug investigations
#define LDEBUG DEBUG

/// logging helpers
#define LOG_COMPONENT_FUNCLINE __func__ << " @ " << __LINE__ << " "
#define LOGWARN( Level, Component ) LOGMSG ( WARNING, Level, Component )
#define STATIC_INFO LONGINFO ( CB, FUNCLINE )

// used in CurlMulti functions
#define LOG_COMPONENT_CURLMULTI "CurlMulti_c::" << LOG_COMPONENT_FUNCLINE
#define MULTI_LOG( LEVEL ) LOGMSG ( LEVEL, MULTIINFO, CURLMULTI )
#define MULTI_INFO MULTI_LOG ( INFO )
#define MULTI_WARN MULTI_LOG ( WARNING )
#define MULTI_DEBUG MULTI_LOG ( LDEBUG )

// used in CurlEasy functions
#define LOG_COMPONENT_CONN "CurlConn_t::" << LOG_COMPONENT_FUNCLINE << this << "(" << m_pCurlEasy << ") " << "(" << m_sUrl << ") "
#define CONN_LOG( LEVEL ) LOGMSG ( LEVEL, CURLEASY, CONN )
#define CONN_INFO CONN_LOG ( INFO )
#define CONN_WARN CONN_LOG ( WARNING )
#define CONN_DEBUG CONN_LOG ( LDEBUG )

// used in socket functions
#define LOG_COMPONENT_SOCKET "CurlSocket_t::" << LOG_COMPONENT_FUNCLINE << this << " (refs " << GetRefcount() << ", sock " << m_iSock << ") "
#define SOCKET_LOG( LEVEL ) LOGMSG ( LEVEL, CULRSOCKET, SOCKET )
#define SOCKET_INFO SOCKET_LOG ( INFO )
#define SOCKET_WARN SOCKET_LOG ( WARNING )
#define SOCKET_DEBUG SOCKET_LOG ( LDEBUG )

// used in timer cb
#define LOG_COMPONENT_TIMER LOG_COMPONENT_CURLMULTI << STime() << " "
#define TIMER_LOG( LEVEL ) LOGMSG ( LEVEL, TIMERCB, TIMER )
#define TIMER_INFO TIMER_LOG ( INFO )
#define TIMER_WARNING TIMER_LOG ( WARNING )
#define TIMER_DEBUG TIMER_LOG ( LDEBUG )

// used in static cbacks, one param must be called 'handle'
#define LOG_COMPONENT_CURLCB LOG_COMPONENT_FUNCLINE << handle << " "
#define CURLCB_LOG( LEVEL ) LOGMSG ( LEVEL, CB, CURLCB )
#define CB_INFO CURLCB_LOG ( INFO )
#define CB_WARN CURLCB_LOG ( WARN )
#define CB_DEBUG CURLCB_LOG ( LDEBUG )

#if WITH_CURL

#include "datetime.h"
#include <curl/curl.h>

#if DL_CURL

static decltype ( &curl_global_init ) sph_curl_global_init = nullptr;
static decltype ( &curl_global_cleanup ) sph_curl_global_cleanup = nullptr;
static decltype ( &curl_multi_init ) sph_curl_multi_init = nullptr;
static decltype ( &curl_multi_cleanup ) sph_curl_multi_cleanup = nullptr;
static decltype ( &curl_multi_add_handle ) sph_curl_multi_add_handle = nullptr;
static decltype ( &curl_multi_remove_handle ) sph_curl_multi_remove_handle = nullptr;
static decltype ( &curl_multi_setopt ) sph_curl_multi_setopt = nullptr;
static decltype ( &curl_multi_socket_action ) sph_curl_multi_socket_action = nullptr;
static decltype ( &curl_multi_info_read ) sph_curl_multi_info_read = nullptr;
static decltype ( &curl_multi_assign ) sph_curl_multi_assign = nullptr;
static decltype ( &curl_easy_init ) sph_curl_easy_init = nullptr;
static decltype ( &curl_easy_cleanup ) sph_curl_easy_cleanup = nullptr;
static decltype ( &curl_easy_setopt ) sph_curl_easy_setopt = nullptr;
static decltype ( &curl_easy_getinfo ) sph_curl_easy_getinfo = nullptr;
static decltype ( &curl_slist_append ) sph_curl_slist_append = nullptr;
static decltype ( &curl_slist_free_all ) sph_curl_slist_free_all = nullptr;

static bool InitDynamicCurl()
{
	const char* sFuncs[] = {
		"curl_global_init",
		"curl_global_cleanup",
		"curl_multi_init",
		"curl_multi_cleanup",
		"curl_multi_add_handle",
		"curl_multi_remove_handle",
		"curl_multi_setopt",
		"curl_multi_socket_action",
		"curl_multi_info_read",
		"curl_multi_assign",
		"curl_easy_init",
		"curl_easy_cleanup",
		"curl_easy_setopt",
		"curl_easy_getinfo",
		"curl_slist_append",
		"curl_slist_free_all",
	};
	void** pFuncs[] = {
		(void**)&sph_curl_global_init,
		(void**)&sph_curl_global_cleanup,
		(void**)&sph_curl_multi_init,
		(void**)&sph_curl_multi_cleanup,
		(void**)&sph_curl_multi_add_handle,
		(void**)&sph_curl_multi_remove_handle,
		(void**)&sph_curl_multi_setopt,
		(void**)&sph_curl_multi_socket_action,
		(void**)&sph_curl_multi_info_read,
		(void**)&sph_curl_multi_assign,
		(void**)&sph_curl_easy_init,
		(void**)&sph_curl_easy_cleanup,
		(void**)&sph_curl_easy_setopt,
		(void**)&sph_curl_easy_getinfo,
		(void**)&sph_curl_slist_append,
		(void**)&sph_curl_slist_free_all,
	};

	static CSphDynamicLibrary dLib ( CURL_LIB );
	return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void** ) );
}

#else

#define sph_curl_global_init curl_global_init
#define sph_curl_global_cleanup curl_global_cleanup
#define sph_curl_multi_init curl_multi_init
#define sph_curl_multi_cleanup curl_multi_cleanup
#define sph_curl_multi_add_handle curl_multi_add_handle
#define sph_curl_multi_remove_handle curl_multi_remove_handle
#define sph_curl_multi_setopt curl_multi_setopt
#define sph_curl_multi_socket_action curl_multi_socket_action
#define sph_curl_multi_info_read curl_multi_info_read
#define sph_curl_multi_assign curl_multi_assign
#define sph_curl_easy_init curl_easy_init
#define sph_curl_easy_cleanup curl_easy_cleanup
#define sph_curl_easy_setopt curl_easy_setopt
#define sph_curl_easy_getinfo curl_easy_getinfo
#define sph_curl_slist_append curl_slist_append
#define sph_curl_slist_free_all curl_slist_free_all
#define InitDynamicCurl() ( true )
#endif


static CSphString STime (int64_t iNow=-1)
{
	if (iNow<0)
		iNow = sphMicroTimer();
	time_t ts = (time_t)( iNow / 1000000 ); // on some systems (eg. FreeBSD 6.2), tv.tv_sec has another type, and we can't just pass it

	cctz::civil_second tCS = ConvertTimeUTC(ts);

	StringBuilder_c tOut;
	tOut << Digits<2> ( tCS.hour() ) << ':' << Digits<2> ( tCS.minute() ) << ':' << Digits<2> ( tCS.second() ) << '.' << FixedNum<10, 3, 0, '0'> ( ( iNow % 1000000 ) / 1000 );
	CSphString sRes;
	tOut.MoveTo ( sRes );
	return sRes;
}

// helper used for logging
inline static const char* CurlPollName ( int iWhat )
{
	static const char* WhatStrs[] = { "NONE", "IN", "OUT", "INOUT", "REMOVE" };
	return iWhat<sizeof(WhatStrs) ? WhatStrs[iWhat] : "UNKNOWN";
}

// forward def
class CurlSocket_c;

// Common curl for all connections
class CurlMulti_c
{
private:
	CURLM* m_pCurlMulti;
	Threads::RoledSchedulerSharedPtr_t m_tStrand;
	MiniTimer_c m_tTimer;
	static bool m_bInitialized;

	CurlSocket_c* MakeAsyncSocket ( curl_socket_t tCurlSocket ) const;

	int CmSocketCb ( curl_socket_t tCurlSocket, int iWhat, void* pSocketData ) const REQUIRES ( CurlStrand() );

	void StrandSocketAction ( curl_socket_t tCurlSocket, int iWhat ) const
	{
		Threads::Coro::Go ( [this, tCurlSocket, iWhat]() REQUIRES ( CurlStrand() ) {
			SocketAction ( tCurlSocket, iWhat );
		}, CurlStrand() );
	}

	void CheckTimeouts() const
	{
		TIMER_DEBUG;
		StrandSocketAction ( CURL_SOCKET_TIMEOUT, 0 );
	}

	int CmTimerCb ( long iTimeoutMS ) REQUIRES ( CurlStrand() )
	{
		if ( iTimeoutMS < 0 )
		{
			m_tTimer.UnEngage();
			TIMER_INFO << "unengage";
		}
		else if ( iTimeoutMS == 0 )
		{
			TIMER_DEBUG << "immediate check";
			StrandSocketAction ( CURL_SOCKET_TIMEOUT, 0 );
		} else
		{
			auto iEngaged = m_tTimer.Engage ( iTimeoutMS == 1 ? 2 : iTimeoutMS ); // add 1MS jitter
			TIMER_DEBUG << "Engage for " << iTimeoutMS << " (" << STime(iEngaged) << ")";
		}

		return 0;
	}

	static int CmSocketCbJump ( CURL*, curl_socket_t tCurlSocket, int iWhat, void* pUserData, void* pSocketData ) NO_THREAD_SAFETY_ANALYSIS
	{
		auto pThis = (CurlMulti_c*)pUserData;
		return pThis->CmSocketCb ( tCurlSocket, iWhat, pSocketData );
	}

	static int CmTimerCbJump ( CURLM* pcurl, long iTimeoutMS, void* pUserData ) NO_THREAD_SAFETY_ANALYSIS
	{
		auto pThis = (CurlMulti_c*)pUserData;
		assert ( pThis->m_pCurlMulti == pcurl );
		return pThis->CmTimerCb ( iTimeoutMS );
	}

public:
	CurlMulti_c()
		: m_tTimer { "CurlMulti", [this] { CheckTimeouts(); } }
	{
		sph_curl_global_init ( CURL_GLOBAL_DEFAULT );
		m_pCurlMulti = sph_curl_multi_init();

		// socket cb
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_SOCKETFUNCTION, &CmSocketCbJump );
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_SOCKETDATA, this );

		// timer cb
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_TIMERFUNCTION, &CmTimerCbJump );
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_TIMERDATA, this );

		m_tStrand = MakeAloneScheduler ( GlobalWorkPool(), "curl_serial" );
		m_bInitialized = true;
		MULTI_INFO;
	}

	~CurlMulti_c()
	{
		Deinit();
	}

	void WriteSocketCookie ( curl_socket_t tCurlSocket, void* pCookie ) const REQUIRES ( CurlStrand() )
	{
		auto x = sph_curl_multi_assign ( m_pCurlMulti, tCurlSocket, pCookie );
		MULTI_INFO << "curl_multi_assign for socket " << tCurlSocket << ", cookie " << pCookie << " returned " << x;
	}

	void SocketAction ( curl_socket_t tCurlSocket, int iWhat ) const REQUIRES ( CurlStrand() );

	Threads::SchedRole CurlStrand() const RETURN_CAPABILITY ( m_tStrand )
	{
		return m_tStrand;
	}

	CURLM* GetMultiPtr() const
	{
		return m_pCurlMulti;
	}

	inline static bool IsInitialized()
	{
		return m_bInitialized;
	}

	void Deinit()
	{
		MULTI_INFO;
		if ( !m_bInitialized )
			return;
		sph_curl_multi_cleanup ( m_pCurlMulti );
		sph_curl_global_cleanup();
		m_bInitialized = false;
		MULTI_INFO;
	}
};

bool CurlMulti_c::m_bInitialized = false;

CurlMulti_c& CurlMulti()
{
	static CurlMulti_c tCurlMulti;
	return tCurlMulti;
}

Threads::SchedRole CurlStrand() RETURN_CAPABILITY ( CurlMulti().CurlStrand() )
{
	return CurlMulti().CurlStrand();
}

// wrapper over curl socket.
// Notice, that curl manages connections itself, we don't open/accept anything here, but just provide async io.
class CurlSocket_c final: public ISphNetAction
{
	const CurlMulti_c* m_pCurlMulti;
	CSphRefcountedPtr<CSphNetLoop> m_pNetLoop;
	int m_iNotifiedCurlEvents = CURL_POLL_NONE;
	int m_iLastEngaged = CURL_POLL_NONE;

protected:
	~CurlSocket_c() final
	{
		SOCKET_INFO;
	}

public:
	CurlSocket_c ( int iSock, const CurlMulti_c* pOwner )
		: ISphNetAction ( iSock )
		, m_pCurlMulti ( pOwner )
	{
		assert ( m_pCurlMulti );
		auto pNetLoop = GetAvailableNetLoop();
		SafeAddRef ( pNetLoop );
		m_pNetLoop = pNetLoop;
		SOCKET_INFO;
	}

	void Process() final
	{
		int iCurlEvents = CURL_POLL_NONE;
		if ( CheckSocketError() || m_uGotEvents == IS_TIMEOUT ) // real socket error
		{
			iCurlEvents = CURL_POLL_REMOVE;
		} else
		{
			bool bRead = m_uGotEvents & NetPollEvent_t::IS_READ;
			bool bWrite = m_uGotEvents & NetPollEvent_t::IS_WRITE;
			if ( bRead && bWrite )
				iCurlEvents = CURL_POLL_INOUT;
			else if ( bRead )
				iCurlEvents = CURL_POLL_IN;
			else if ( bWrite )
				iCurlEvents = CURL_POLL_OUT;
			else
				iCurlEvents = CURL_POLL_NONE;
		}

		NotifyCurl ( iCurlEvents );
	}

	void NetLoopDestroying() final
	{
		Release();
	}

public:
	void Engage ( int iCurlWhat, bool bExternal = true ) REQUIRES ( m_pCurlMulti->CurlStrand() )
	{
		SOCKET_DEBUG << ( bExternal ? "external" : "internal" ) << " -> " << CurlPollName ( iCurlWhat );
		m_iLastEngaged = iCurlWhat;
		m_uIOChange = NetPollEvent_t::SET_ONESHOT;
		switch ( iCurlWhat )
		{
		case CURL_POLL_IN: m_uIOChange |= NetPollEvent_t::SET_READ; break;
		case CURL_POLL_INOUT: m_uIOChange |= NetPollEvent_t::SET_RW; break;
		case CURL_POLL_OUT: m_uIOChange |= NetPollEvent_t::SET_WRITE; break;
		case CURL_POLL_REMOVE: return Remove();
		default: break;
		}
		m_pNetLoop->AddAction ( this );
	}

private:
	void NotifyCurl ( int iCurlEvents )
	{
		if ( m_iNotifiedCurlEvents == iCurlEvents )
			return;

		SOCKET_DEBUG << "got events " << m_uGotEvents << ", " << CurlPollName ( iCurlEvents );
		m_iNotifiedCurlEvents = iCurlEvents;
		AddRef();
		Threads::Coro::Go ( [this, iCurlEvents]() REQUIRES ( m_pCurlMulti->CurlStrand() ) {
			CSphRefcountedPtr<ISphNetAction> pWorkKeeper { this };
			m_pCurlMulti->SocketAction ( m_iSock, iCurlEvents );
			CurlNotified();
		},
			m_pCurlMulti->CurlStrand() );
	}

	void CurlNotified() REQUIRES ( m_pCurlMulti->CurlStrand() )
	{
		SOCKET_DEBUG;
		m_iNotifiedCurlEvents = CURL_POLL_NONE;
		if ( m_iLastEngaged != CURL_POLL_REMOVE )
			Engage ( m_iLastEngaged, false );
	}

	void Remove() REQUIRES ( m_pCurlMulti->CurlStrand() )
	{
		SOCKET_INFO;
		m_iLastEngaged = CURL_POLL_REMOVE;
		m_uIOChange = NetPollEvent_t::SET_NONE;
		m_pNetLoop->AddAction ( this );
		m_pCurlMulti->WriteSocketCookie ( m_iSock, nullptr );
		Release();
	}
};

#ifdef CURLOPT_XFERINFOFUNCTION
static int ProgressCb ( void* handle, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
{
	auto* pConn = (CurlConn_t*)handle;
	CB_DEBUG << ": " << pConn->m_sUrl
			 << " d (" << dlnow << "/" << dltotal << ")"
			 << " u (" << ulnow << "/" << ultotal << ")";
	return 0;
}
#endif

#if CURL_VERBOSE_CB
inline static const char* CurlInfoType ( int iType )
{
	const char* InfotypeStrs[] = { "CURLINFO_TEXT", "CURLINFO_HEADER_IN", "CURLINFO_HEADER_OUT", "CURLINFO_DATA_IN", "CURLINFO_DATA_OUT", "CURLINFO_SSL_DATA_IN", "CURLINFO_SSL_DATA_OUT", "CURLINFO_END" };
	return iType < sizeof ( InfotypeStrs ) ? InfotypeStrs[iType] : "UNKNOWN";
}

static int DebugCbJump ( CURL* handle, curl_infotype type, char* data, size_t size, void* pUserData )
{
	CB_INFO << "chunk " << CurlInfoType ( type ) << " with " << size << " bytes.";
	return 0;
}
#endif

using CurlConnListHook_t = boost::intrusive::slist_member_hook<>;

struct CurlConn_t
{
	CURL*			m_pCurlEasy;
	CurlMulti_c*	m_pCurlMulti = nullptr;
	CSphString		m_sUrl;
	char			m_sError[CURL_ERROR_SIZE];
	CURLcode		m_uReturnCode = CURLE_OK;
	CSphVector<BYTE> m_dData;
	Threads::Coro::Waker_c m_tWaker;
	CurlConnListHook_t	m_tHook;

	int WriteCb ( ByteBlob_t dData )
	{
		CONN_DEBUG << "received " << dData.second << " bytes";
		m_dData.Append(dData);
		return dData.second;
	}

	static size_t WriteCbJump ( void* pData, size_t tSize, size_t nItems, void* pUserData )
	{
		assert ( tSize == 1 );
		auto pConn = (CurlConn_t*)pUserData;
		return pConn->WriteCb ( { (const BYTE*)pData, tSize * nItems } );
	}

	template<typename TPARAM>
	inline void SetCurlOpt ( CURLoption eOpt, TPARAM tParam ) const
	{
		sph_curl_easy_setopt ( m_pCurlEasy, eOpt, tParam );
	}

	CurlConn_t ( CSphString sUrl, Threads::Coro::Worker_c* pWorker ) REQUIRES ( CurlStrand() )
		: m_sUrl { std::move(sUrl) }
		, m_tWaker { Threads::CreateWaker ( pWorker ) }
	{
		m_pCurlEasy = sph_curl_easy_init();

		// basic options from customer
		SetCurlOpt ( CURLOPT_URL, m_sUrl.cstr() );
		SetCurlOpt ( CURLOPT_FOLLOWLOCATION, 1 );

		// extra service options
		SetCurlOpt ( CURLOPT_NOSIGNAL, 1 );

		// options we absolutely NEED
		SetCurlOpt ( CURLOPT_WRITEFUNCTION, WriteCbJump );
		SetCurlOpt ( CURLOPT_WRITEDATA, this );
		SetCurlOpt ( CURLOPT_ERRORBUFFER, m_sError );
		SetCurlOpt ( CURLOPT_PRIVATE, this ); // to use via CURLINFO_PRIVATE from curl_easy_getinfo

#if CURL_VERBOSE
		SetCurlOpt ( CURLOPT_VERBOSE, 1 );
#if CURL_VERBOSE_CB
		SetCurlOpt ( CURLOPT_DEBUGFUNCTION, DebugCbJump );
		SetCurlOpt ( CURLOPT_DEBUGDATA, this );
#endif
#endif

#if CURL_PROGRESS
		SetCurlOpt ( CURLOPT_NOPROGRESS, 0 );
#ifdef CURLOPT_XFERINFOFUNCTION
		SetCurlOpt ( CURLOPT_XFERINFOFUNCTION, ProgressCb );
#endif
#ifdef CURLOPT_XFERINFODATA
		SetCurlOpt ( CURLOPT_XFERINFODATA, this );
#endif
#endif
	}

	void RunQuery() REQUIRES ( CurlStrand() );
	void Done ( CURLcode uResult ) REQUIRES ( CurlStrand() );

	~CurlConn_t()
	{
		CONN_INFO;
		sph_curl_easy_cleanup ( m_pCurlEasy );
	}
};

static bool operator== ( const CurlConn_t& tA, const CurlConn_t& tB ) noexcept
{
	return tA.m_pCurlEasy == tB.m_pCurlEasy;
}

using CurlConnList_t = boost::intrusive::slist<CurlConn_t,
	boost::intrusive::member_hook<CurlConn_t, CurlConnListHook_t, &CurlConn_t::m_tHook>,
	boost::intrusive::constant_time_size<false>,
	boost::intrusive::cache_last<false>>;

CurlConnList_t g_tCurlConnections GUARDED_BY ( CurlStrand() );

void CurlConn_t::RunQuery() REQUIRES ( CurlStrand() )
{
	m_pCurlMulti = &CurlMulti();
	CONN_INFO << "Add to multi " << m_pCurlMulti << " for " << m_sUrl;
	g_tCurlConnections.push_front ( *this );
	auto iRes = sph_curl_multi_add_handle ( m_pCurlMulti->GetMultiPtr(), m_pCurlEasy );
	CONN_INFO << "Add complete -> " << iRes;
}

void CurlConn_t::Done ( CURLcode uResult ) REQUIRES ( CurlStrand() )
{
	g_tCurlConnections.remove ( *this ); // this op requires operator==
	assert ( m_pCurlMulti );
	m_uReturnCode = uResult;
	CONN_INFO << "DONE: (" << uResult << ") " << m_sError;
	sph_curl_multi_remove_handle ( m_pCurlMulti->GetMultiPtr(), m_pCurlEasy );
	m_pCurlMulti = nullptr;
	m_tWaker.Wake ( true );
}

static void CurlConnectionIsDone ( CURLMsg* pMsg ) REQUIRES ( CurlStrand() )
{
	assert ( pMsg->msg == CURLMSG_DONE );
	auto pEasy = pMsg->easy_handle;
	CurlConn_t* pConn;
	sph_curl_easy_getinfo ( pEasy, CURLINFO_PRIVATE, &pConn );
	pConn->Done ( pMsg->data.result );
}

void CurlMulti_c::SocketAction ( curl_socket_t tCurlSocket, int iWhat ) const REQUIRES ( CurlStrand() ) NO_THREAD_SAFETY_ANALYSIS
{
	MULTI_DEBUG << "invoke curl_multi_socket_action with socket " << tCurlSocket << ", " << CurlPollName(iWhat);
	int iLeft = 0;
	auto x = sph_curl_multi_socket_action ( m_pCurlMulti, tCurlSocket, iWhat, &iLeft );
	MULTI_DEBUG << "curl_multi_socket_action returned " << x << ", " << iLeft << " tasks running.";

	while ( true )
	{
		int iLeftInQueue;
		auto pMsg = sph_curl_multi_info_read ( m_pCurlMulti, &iLeftInQueue );
		if ( !pMsg )
			break;

		if ( pMsg->msg == CURLMSG_DONE )
			CurlConnectionIsDone ( pMsg );
	}
}

CurlSocket_c* CurlMulti_c::MakeAsyncSocket ( curl_socket_t tCurlSocket ) const
{
	MULTI_INFO << "called with sock " << tCurlSocket;
	auto pSocket = new CurlSocket_c ( tCurlSocket, this );

	Threads::ScopedScheduler_c tSerialFiber { CurlStrand() };
	WriteSocketCookie ( tCurlSocket, reinterpret_cast<void*> ( pSocket ) );
	return pSocket;
}

int CurlMulti_c::CmSocketCb ( curl_socket_t tCurlSocket, int iWhat, void* pSocketData ) const NO_THREAD_SAFETY_ANALYSIS
{
	MULTI_INFO << "called with sock " << tCurlSocket << " for " << CurlPollName ( iWhat ) << "(" << iWhat << "), data " << pSocketData;

	auto* pAsyncSocket = pSocketData ? (CurlSocket_c*)pSocketData : MakeAsyncSocket ( tCurlSocket );
	switch ( iWhat )
	{
	case CURL_POLL_REMOVE:
	case CURL_POLL_IN:
	case CURL_POLL_OUT:
	case CURL_POLL_INOUT:
		pAsyncSocket->Engage ( iWhat );

	default: break;
	}

	return 0;
}

class CurlHttpHeaders_c
{
	struct curl_slist* m_pHttpHeadersList = nullptr;
public:
	CurlHttpHeaders_c() = default;
	~CurlHttpHeaders_c()
	{
		if ( m_pHttpHeadersList )
			sph_curl_slist_free_all ( m_pHttpHeadersList );
	}

	void Append ( const char* szHeader )
	{
		m_pHttpHeadersList = sph_curl_slist_append ( m_pHttpHeadersList, szHeader );
	}

	inline curl_slist* CurlSlist() const { return m_pHttpHeadersList; }
};

bool IsCurlAvailable()
{
	static bool bCurlLoaded = false;
	if ( !bCurlLoaded )
		bCurlLoaded = InitDynamicCurl();

	return bCurlLoaded;
}

#if !defined (CURL_LIB)
#define CURL_LIB "internal"
#endif
static const char * szNoCurlMsg = CURL_LIB " not found";

using CurlOpt_t = std::pair<CURLoption, intptr_t>;

std::pair<bool, CSphString> InvokeCurl ( CSphString sUrl, const VecTraits_T<CurlOpt_t>& dParams )
{
	if ( !IsCurlAvailable() )
		return { false, szNoCurlMsg };

	using namespace Threads::Coro;
	auto pWorker = CurrentWorker();

	std::unique_ptr<CurlConn_t> pRequest;

	YieldWith ( [&]() mutable {
		Go ( [&]() REQUIRES ( CurlStrand() )  mutable {
			pRequest = std::make_unique<CurlConn_t> ( std::move ( sUrl ), pWorker );
			dParams.for_each ( [&pRequest] ( const auto tParam ) { pRequest->SetCurlOpt ( tParam.first, tParam.second ); } );
			pRequest->RunQuery();
		},
			CurlStrand() );
	} );

	if ( pRequest->m_uReturnCode != CURLE_OK )
		return { false, pRequest->m_sError };

	return { true, CSphString ( pRequest->m_dData ) };
}


CSphString FetchUrl ( const CSphString& sUrl )
{
	CSphVector<CurlOpt_t> dCurlOpts;
	dCurlOpts.Add ( { (CURLoption)237, 0L } ); // CURLOPT_PIPEWAIT. If not exist, will be just ignored
	auto [bSuccess, sResult] = InvokeCurl ( sUrl, dCurlOpts );
	return sResult;
}

std::pair<bool, CSphString> PostToHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<CSphString>& dHeaders )
{
	CSphVector<CurlOpt_t> dOptions;
	dOptions.Add ( { CURLOPT_POST, 1 } );
	dOptions.Add ( { CURLOPT_POSTFIELDSIZE, sQuery.second } );
	dOptions.Add ( { CURLOPT_POSTFIELDS, (intptr_t)sQuery.first } );
	dOptions.Add ( { (CURLoption)237, 0L } ); // CURLOPT_PIPEWAIT. If not exist, will be just ignored

	CurlHttpHeaders_c tHeaders;
	dHeaders.for_each ( [&tHeaders] ( const auto& sHeader ) { tHeaders.Append ( sHeader.cstr() ); } );
	tHeaders.Append ( "Content-Type: application/json; charset=UTF-8" );
	dOptions.Add ( { CURLOPT_HTTPHEADER, (intptr_t)tHeaders.CurlSlist() } );

	return InvokeCurl ( std::move ( sUrl ), dOptions );
}

void ShutdownCurl()
{
	if ( !CurlMulti_c::IsInitialized() )
		return;

	Threads::CallPlainCoroutine ( []() REQUIRES ( CurlStrand() ) {
		if ( !CurlMulti_c::IsInitialized() )
			return;
		while ( !g_tCurlConnections.empty() )
		{
			auto& tConn = g_tCurlConnections.front();
			strcpy ( tConn.m_sError, "Interrupted due to shutdown" );
			tConn.Done ( CURLE_ABORTED_BY_CALLBACK );
		}
		CurlMulti().Deinit();
	},
		CurlStrand() );
}

#else // WITH_CURL

static const char* szNoCurlMsg = "No CURL support compiled in";

CSphString FetchUrl ( const CSphString& sUrl )
{
	return szNoCurlMsg;
}


std::pair<bool, CSphString> PostToHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<CSphString>& dHeaders )
{
	return { false, szNoCurlMsg };
}


void ShutdownCurl()
{}


bool IsCurlAvailable()
{
	return false;
}

#endif