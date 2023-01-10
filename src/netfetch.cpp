//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
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
#define LOG_LEVEL_TIMERCB ( LOG_LEVEL_MULTIINFO & false )
#define LOG_LEVEL_CULRSOCKET false
#define LOG_LEVEL_CURLEASY false
#define LOG_LEVEL_CB false

#define CURL_VERBOSE false
#define CURL_PROGRESS false

/// logging helpers
#define LOG_COMPONENT_FUNCLINE __func__ << " @ " << __LINE__ << " "
#define LOGWARN( Level, Component ) LOGMSG ( WARNING, Level, Component )
#define STATIC_INFO LONGINFO ( CB, FUNCLINE )

// used in CurlMulti functions
#define LOG_COMPONENT_CURLMULTI "CurlMulti_c::" << LOG_COMPONENT_FUNCLINE
#define MULTI_LOG( LEVEL ) LOGMSG ( LEVEL, MULTIINFO, CURLMULTI )
#define MULTI_INFO MULTI_LOG ( INFO )
#define MULTI_WARN MULTI_LOG ( WARNING )
#define MULTI_DEBUG MULTI_LOG ( DEBUG )

// used in CurlEasy functions
#define LOG_COMPONENT_CONN "CurlConn_t::" << LOG_COMPONENT_FUNCLINE << this << "(" << m_pCurlEasy << ") " << "(" << m_sUrl << ") "
#define CONN_LOG( LEVEL ) LOGMSG ( LEVEL, CURLEASY, CONN )
#define CONN_INFO CONN_LOG ( INFO )
#define CONN_WARN CONN_LOG ( WARNING )
#define CONN_DEBUG CONN_LOG ( DEBUG )

// used in socket functions
#define LOG_COMPONENT_SOCKET "CurlSocket_t::" << LOG_COMPONENT_FUNCLINE << this << " (refs " << GetRefcount() << ", sock " << m_iSock << ") "
#define SOCKET_LOG( LEVEL ) LOGMSG ( LEVEL, CULRSOCKET, SOCKET )
#define SOCKET_INFO SOCKET_LOG ( INFO )
#define SOCKET_WARN SOCKET_LOG ( WARNING )
#define SOCKET_DEBUG SOCKET_LOG ( DEBUG )

// used in timer cb
#define LOG_COMPONENT_TIMER LOG_COMPONENT_CURLMULTI << STime() << " "
#define TIMER_LOG( LEVEL ) LOGMSG ( LEVEL, TIMERCB, TIMER )
#define TIMER_INFO TIMER_LOG ( INFO )
#define TIMER_WARNING TIMER_LOG ( WARNING )
#define TIMER_DEBUG TIMER_LOG ( DEBUG )

// used in static cbacks, one param must be called 'handle'
#define LOG_COMPONENT_CURLCB LOG_COMPONENT_FUNCLINE << handle << " "
#define CURLCB_LOG( LEVEL ) LOGMSG ( LEVEL, CB, CURLCB )
#define CB_INFO CURLCB_LOG ( INFO )
#define CB_WARN CURLCB_LOG ( WARN )
#define CB_DEBUG CURLCB_LOG ( DEBUG )

#if WITH_CURL

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

#if !_WIN32
	struct tm tmp;
	localtime_r ( &ts, &tmp );
#else
	struct tm tmp;
	tmp = *localtime ( &ts );
#endif
	StringBuilder_c tOut;
	tOut << Digits<2> ( tmp.tm_hour ) << ':' << Digits<2> ( tmp.tm_min ) << ':' << Digits<2> ( tmp.tm_sec )	 << '.' << FixedNum<10, 3, 0, '0'> ( ( iNow % 1000000 ) / 1000 );
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

inline static const char* CurlInfoType ( int iType )
{
	const char* InfotypeStrs[] = { "CURLINFO_TEXT", "CURLINFO_HEADER_IN", "CURLINFO_HEADER_OUT", "CURLINFO_DATA_IN", "CURLINFO_DATA_OUT", "CURLINFO_SSL_DATA_IN", "CURLINFO_SSL_DATA_OUT", "CURLINFO_END" };
	return iType < sizeof ( InfotypeStrs ) ? InfotypeStrs[iType] : "UNKNOWN";
}

Threads::SchedRole CurlStrand();

struct CurlSocket_t final : public ISphNetAction
{
	CSphRefcountedPtr<CSphNetLoop> m_pNetLoop;
	int m_iNotifiedCurlEvents = CURL_POLL_NONE;
	int m_iLastEngaged = CURL_POLL_NONE;

protected:
	~CurlSocket_t() final
	{
		SOCKET_INFO;
	}

public:
	explicit CurlSocket_t ( int iSock )
		: ISphNetAction( iSock )
	{
		auto pNetLoop = GetAvailableNetLoop();
		SafeAddRef ( pNetLoop );
		m_pNetLoop = pNetLoop;
		SOCKET_INFO;
	}

	void Process()
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

	void NotifyCurl ( int iCurlEvents );
	void WipeFromMulti() REQUIRES ( CurlStrand() );

	void CurlNotified() REQUIRES ( CurlStrand() )
	{
		SOCKET_DEBUG;
		m_iNotifiedCurlEvents = CURL_POLL_NONE;
		if ( m_iLastEngaged != CURL_POLL_REMOVE )
			Engage ( m_iLastEngaged, false );
	}

	void NetLoopDestroying() final
	{
		Release();
	}

	void Remove() REQUIRES ( CurlStrand() )
	{
		SOCKET_INFO;
		m_iLastEngaged = CURL_POLL_REMOVE;
		m_uIOChange = NetPollEvent_t::SET_NONE;
		m_pNetLoop->AddAction ( this );
		WipeFromMulti();
		Release();
	}

	void Engage (int iCurlWhat, bool bExternal=true ) REQUIRES ( CurlStrand() )
	{
		SOCKET_DEBUG << ( bExternal ? "external" : "internal" ) << " -> " << CurlPollName ( iCurlWhat );
		m_iLastEngaged = iCurlWhat;
		m_uIOChange = NetPollEvent_t::SET_ONESHOT;
		switch ( iCurlWhat )
		{
		case CURL_POLL_IN : m_uIOChange |= NetPollEvent_t::SET_READ; break;
		case CURL_POLL_INOUT: m_uIOChange |= NetPollEvent_t::SET_RW; break;
		case CURL_POLL_OUT: m_uIOChange |= NetPollEvent_t::SET_WRITE; break;
		case CURL_POLL_REMOVE: return Remove();
		default: break;
		}
		m_pNetLoop->AddAction ( this );
	}
};


// Common curl for all connections
class CurlMulti_c
{
public:
	CURLM* m_pCurlMulti;
	Threads::RoledSchedulerSharedPtr_t m_tStrand;
	MiniTimer_c m_tTimer;

	void WriteSocketCookie ( curl_socket_t tCurlSocket, void* pCookie=nullptr ) const REQUIRES ( CurlStrand() )
	{
		auto x = sph_curl_multi_assign ( m_pCurlMulti, tCurlSocket, pCookie );
		MULTI_INFO << "curl_multi_assign returned " << x;
	}

	CurlSocket_t* MakeAsyncSocket ( curl_socket_t tCurlSocket ) const
	{
		MULTI_INFO << "called with sock " << tCurlSocket;
		auto pSocket = new CurlSocket_t ( tCurlSocket );

		Threads::ScopedScheduler_c tSerialFiber { CurlStrand() };
		WriteSocketCookie ( tCurlSocket, reinterpret_cast<void*> ( pSocket ) );
		return pSocket;
	}

	int CmSocketCb ( curl_socket_t tCurlSocket, int iWhat, void* pSocketData ) const REQUIRES ( CurlStrand() ) NO_THREAD_SAFETY_ANALYSIS
	{
		MULTI_INFO << "called with sock " << tCurlSocket << " for " << CurlPollName(iWhat) << "(" << iWhat << "), data " << pSocketData;

		auto* pAsyncSocket = pSocketData ? ( CurlSocket_t* ) pSocketData : MakeAsyncSocket ( tCurlSocket );
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

	void CheckCompleted ( curl_socket_t tCurlSocket, int iWhat) REQUIRES ( CurlStrand() );

	void StrandCheckCompleted ( curl_socket_t tCurlSocket, int iWhat )
	{
		Threads::Coro::Go ( [this, tCurlSocket, iWhat]() REQUIRES ( CurlStrand() ) {
			CheckCompleted ( tCurlSocket, iWhat );
		}, CurlStrand() );
	}

	void CheckTimeouts(bool bPrint = true)
	{
		if ( bPrint )
			TIMER_DEBUG;
		StrandCheckCompleted ( CURL_SOCKET_TIMEOUT, 0 );
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
			StrandCheckCompleted( CURL_SOCKET_TIMEOUT, 0 );
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
		: m_tTimer { "SleepMsec", [this]() { CheckTimeouts(); } }
	{
		sph_curl_global_init ( CURL_GLOBAL_DEFAULT );
		m_pCurlMulti = sph_curl_multi_init();

		// socket cb
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_SOCKETFUNCTION, &CmSocketCbJump );
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_SOCKETDATA, this );

		// timer cb
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_TIMERFUNCTION, &CmTimerCbJump );
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_TIMERDATA, this );

#if LIBCURL_VERSION_NUM>=0x072B00 // CURLPIPE_MULTIPLEX
		// for CURLOPT_PIPEWAIT to work (m.b. no more need as it is default behaviour)
		sph_curl_multi_setopt ( m_pCurlMulti, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX );
#endif

		m_tStrand = MakeAloneScheduler ( GlobalWorkPool(), "serial" );
		MULTI_INFO;
	}

	~CurlMulti_c()
	{
		sph_curl_multi_cleanup ( m_pCurlMulti );
		sph_curl_global_cleanup();
		MULTI_INFO;
	}

	Threads::SchedRole CurlStrand() const RETURN_CAPABILITY ( m_tStrand )
	{
		return m_tStrand;
	}
};

CurlMulti_c& CurlMulti()
{
	static CurlMulti_c tCurlMulti;
	return tCurlMulti;
}

Threads::SchedRole CurlStrand() RETURN_CAPABILITY ( CurlMulti().CurlStrand() )
{
	return CurlMulti().CurlStrand();
}

struct CurlConn_t
{
	CURL*			m_pCurlEasy;
	CSphString		m_sUrl;
	char			m_sError[CURL_ERROR_SIZE];
	CURLcode		m_uReturnCode = CURLE_OK;
	CSphVector<BYTE> m_dData;
	Threads::Coro::Waker_c m_tWaker;

	// fixme! That is ad-hoc and should not work this way
	// we never wait something from curl, because it has no execution thread itself

	static bool CheckShutdown ( void * handle )
	{
		if ( sphInterrupted() )
		{
			const char * sUrl = "";
			auto * pConn = (CurlConn_t *)handle;
			if ( pConn )
				sUrl = pConn->m_sUrl.cstr();
			CB_DEBUG << " exit on shutdown: " << pConn->m_sUrl;

			return true;
		} else
		{
			return false;
		}
	}

	static void LogProgress( void * handle, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
	{
#if CURL_PROGRESS
		auto * pConn = (CurlConn_t *)handle;
		if ( pConn )
		{
			CB_DEBUG << ": " << pConn->m_sUrl
					<< " d (" << dlnow << "/" << dltotal << ")"
					<< " u (" << ulnow << "/" << ultotal << ")";
		}
#endif
	}

	static int ProgressCbv29 ( void * handle, double dltotal, double dlnow, double ultotal, double ulnow )
	{
		if ( CheckShutdown ( handle ) )
			return -1;

		LogProgress ( handle, dltotal, dlnow, ultotal, ulnow );
		return 0;
	}

	static int ProgressCb ( void * handle, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
	{
		if ( CheckShutdown ( handle ) )
			return -1;

		LogProgress ( handle, dltotal, dlnow, ultotal, ulnow );
		return 0;
	}

	int WriteCb ( ByteBlob_t dData )
	{
		CONN_DEBUG << "received " << dData.second << " bytes";
		m_dData.Append(dData);
		return dData.second;
	}

	int DebugCb ( CURL* handle, curl_infotype type, char* data, size_t size )
	{
		CB_INFO << "chunk " << CurlInfoType(type) << " with " << size << " bytes.";
		return 0;
	}

	static size_t WriteCbJump ( void* pData, size_t tSize, size_t nItems, void* pUserData )
	{
		assert ( tSize == 1 );
		auto pConn = (CurlConn_t*)pUserData;
		return pConn->WriteCb ( { (const BYTE*)pData, tSize * nItems } );
	}

	static int DebugCbJump ( CURL* handle, curl_infotype type, char* data, size_t size, void* pUserData )
	{
		auto pConn = (CurlConn_t*)pUserData;
		return pConn->DebugCb ( handle, type, data, size );
	}

	static void DoneCb ( CURLMsg* pMsg )
	{
		assert ( pMsg->msg == CURLMSG_DONE );
		auto pEasy = pMsg->easy_handle;
		CurlConn_t* pConn;
		sph_curl_easy_getinfo ( pEasy, CURLINFO_PRIVATE, &pConn );
		pConn->Done ( pMsg->data.result );
	}

	explicit CurlConn_t ( CSphString sUrl, Threads::Coro::Worker_c* pWorker ) REQUIRES ( CurlStrand() )
		: m_sUrl { std::move(sUrl) }
		, m_tWaker { Threads::CreateWaker ( pWorker ) }
	{
		m_pCurlEasy = sph_curl_easy_init();

		// basic options from customer
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_URL, m_sUrl.cstr() );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_FOLLOWLOCATION, 1 );

		// extra service options
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_NOSIGNAL, 1 );
#if LIBCURL_VERSION_NUM>=0x072B00 //CURLOPT_PIPEWAIT
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_PIPEWAIT, 1 );
#endif

		// options we absolutely NEED
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_WRITEFUNCTION, WriteCbJump );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_WRITEDATA, this );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_ERRORBUFFER, m_sError );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_PRIVATE, this ); // to use via CURLINFO_PRIVATE from curl_easy_getinfo

#if CURL_VERBOSE
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_DEBUGFUNCTION, DebugCbJump );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_DEBUGDATA, this );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_VERBOSE, 1 );
#endif

#if LIBCURL_VERSION_NUM>=0x072000 //CURLOPT_XFERINFOFUNCTION vs CURLOPT_PROGRESSFUNCTION
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_NOPROGRESS, 0 );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_XFERINFOFUNCTION, ProgressCb );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_XFERINFODATA, this );
#else
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_NOPROGRESS, 0 );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_PROGRESSFUNCTION, ProgressCbv29 );
		sph_curl_easy_setopt ( m_pCurlEasy, CURLOPT_PROGRESSDATA, this );
#endif
	}

	inline void SetCurlOpt ( CURLoption eOpt, intptr_t tParam )
	{
		sph_curl_easy_setopt ( m_pCurlEasy, eOpt, tParam );
	}

	void RunQuery()
	{
		CONN_INFO << "Add to multi " << &CurlMulti() << " for " << m_sUrl;
		auto iRes = sph_curl_multi_add_handle ( CurlMulti().m_pCurlMulti, m_pCurlEasy );
		CONN_INFO << "Add complete -> " << iRes;
	}

	void Done ( CURLcode uResult )
	{
		m_uReturnCode = uResult;
		CONN_INFO << "DONE: (" << uResult << ") " << m_sError;
		sph_curl_multi_remove_handle ( CurlMulti().m_pCurlMulti, m_pCurlEasy );
		m_tWaker.Wake ( true );
	}

	~CurlConn_t()
	{
		CONN_INFO;
		sph_curl_easy_cleanup ( m_pCurlEasy );
	}
};

void CurlMulti_c::CheckCompleted ( curl_socket_t tCurlSocket, int iWhat ) REQUIRES ( CurlStrand() )
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
			CurlConn_t::DoneCb ( pMsg );
	}
}

void CurlSocket_t::NotifyCurl ( int iCurlEvents )
{
	if ( m_iNotifiedCurlEvents == iCurlEvents )
		return;

	SOCKET_DEBUG << "got events " << m_uGotEvents << ", " << CurlPollName(iCurlEvents);
	m_iNotifiedCurlEvents = iCurlEvents;

	Threads::Coro::Go ( [this, iCurlEvents]() REQUIRES ( CurlStrand() ) {
		CSphRefcountedPtr<ISphNetAction> pWorkKeeper { this };
		AddRef();

		CurlMulti().CheckCompleted ( m_iSock, iCurlEvents );
		CurlNotified();

	}, CurlStrand() );
}

void CurlSocket_t::WipeFromMulti() REQUIRES ( CurlStrand() )
{
	CurlMulti().WriteSocketCookie ( m_iSock );
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

	inline operator intptr_t() const { return (intptr_t)m_pHttpHeadersList; }
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

std::pair<bool, CSphString> InvokeCurl ( CSphString sUrl, const VecTraits_T<CurlOpt_t>& dParams, const VecTraits_T<const char *> & dHeaders )
{
	if ( !IsCurlAvailable() )
		return { false, szNoCurlMsg };

	using namespace Threads::Coro;
	std::unique_ptr<CurlConn_t> pRequest;

	CurlHttpHeaders_c tHeaders;
	auto pWorker = CurrentWorker();

	YieldWith ( [&]() mutable {
		Go ( [&]() mutable {
			pRequest = std::make_unique<CurlConn_t> ( std::move ( sUrl ), pWorker );
			for ( const auto& tParam : dParams )
				pRequest->SetCurlOpt ( tParam.first, tParam.second );
			for ( const char* szHeader : dHeaders )
				tHeaders.Append ( szHeader );
			if ( tHeaders )
				pRequest->SetCurlOpt ( CURLOPT_HTTPHEADER, tHeaders );
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
	auto tResult = InvokeCurl ( sUrl, {}, {} );
	return tResult.second;
}

std::pair<bool, CSphString> FetchHelperUrl ( CSphString sUrl, Str_t sQuery )
{
	return FetchHelperUrl ( sUrl, sQuery, {} );
}

std::pair<bool, CSphString> FetchHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<const char *> & dHeaders )
{
	CSphVector<CurlOpt_t> dOptions;
	dOptions.Add ( { CURLOPT_POST, 1 } );
	dOptions.Add ( { CURLOPT_POSTFIELDSIZE, sQuery.second } );
	dOptions.Add ( { CURLOPT_POSTFIELDS, (intptr_t)sQuery.first } );
	
	CSphFixedVector<const char*> dHeadersPost ( dHeaders.GetLength() + 1 );
	dHeadersPost[dHeaders.GetLength()] = "Content-Type: application/json; charset=UTF-8";
	ARRAY_FOREACH ( i, dHeaders )
		dHeadersPost[i] = dHeaders[i];

	return InvokeCurl ( std::move ( sUrl ), dOptions, dHeadersPost );
}

#else // WITH_CURL

static const char* szNoCurlMsg = "No CURL support compiled in";

CSphString FetchUrl ( const CSphString& sUrl )
{
	return szNoCurlMsg;
}

std::pair<bool, CSphString> FetchHelperUrl ( CSphString sUrl, Str_t sQuery )
{
	return { false, szNoCurlMsg };
}

std::pair<bool, CSphString> FetchHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<const char *> & dHeaders )
{
	return { false, szNoCurlMsg };
}

bool IsCurlAvailable()
{
	return false;
}

#endif