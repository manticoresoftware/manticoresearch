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
#include "searchdssl.h"
#include "auth/auth.h"

#define LOG_LEVEL_MULTIINFO false
#define LOG_LEVEL_TIMERCB ( LOG_LEVEL_MULTIINFO && false )
#define LOG_LEVEL_CULRSOCKET false
#define LOG_LEVEL_CURLEASY true
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

#if !WITH_CURL // !WITH_CURL

static const char* szNoCurlMsg = "No CURL support compiled in";

CSphString FetchUrl ( const CSphString& sUrl )
{
	return szNoCurlMsg;
}


UrlResult_t PostToHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<CSphString>& dHeaders )
{
	return UrlResult_t ( false, szNoCurlMsg );
}


void ShutdownCurl()
{}


bool IsCurlAvailable()
{
	return false;
}

CurlOpt_i * CreatePostBinaryOpt ( const ByteBlob_t & tData )
{
	return nullptr;
}

UrlResult_t PostBinaryUrl ( CSphString sUrl, const CurlOpt_i * pOpts )
{
	return ( false, szNoCurlMsg );
}


#else

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

static decltype ( &curl_multi_perform ) sph_curl_multi_perform = nullptr;
static decltype ( &curl_multi_wait ) sph_curl_multi_wait = nullptr;
static decltype ( &curl_easy_strerror ) sph_curl_easy_strerror = nullptr;
static decltype ( &curl_share_init ) sph_curl_share_init = nullptr;
static decltype ( &curl_share_cleanup ) sph_curl_share_cleanup = nullptr;
static decltype ( &curl_share_setopt ) sph_curl_share_setopt = nullptr;

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
		"curl_multi_perform",
		"curl_multi_wait",
		"curl_easy_strerror",
		"curl_share_init",
		"curl_share_cleanup",
		"curl_share_setopt"
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
		(void**)&sph_curl_multi_perform,
		(void**)&sph_curl_multi_wait,
		(void**)&sph_curl_easy_strerror,
		(void**)&sph_curl_share_init,
		(void**)&sph_curl_share_cleanup,
		(void**)&sph_curl_share_setopt
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
	return iWhat<(int)sizeof(WhatStrs) ? WhatStrs[iWhat] : "UNKNOWN";
}

// forward def
class CurlSocket_c;

// Common curl for all connections
class CurlMulti_c
{
private:
	CURLM * m_pCurlMulti;
	CURLSH * m_pCurlShare;
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

		m_pCurlShare = sph_curl_share_init();
		sph_curl_share_setopt ( m_pCurlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION );

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

	CURLM * GetMultiPtr() const
	{
		return m_pCurlMulti;
	}

	CURLSH * GetSharePtr() const
	{
		return m_pCurlShare;
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
		sph_curl_share_cleanup ( m_pCurlShare );
		sph_curl_multi_cleanup ( m_pCurlMulti );
		sph_curl_global_cleanup();
		m_bInitialized = false;
		MULTI_INFO;
	}
};

bool CurlMulti_c::m_bInitialized = false;

static CurlMulti_c & CurlMulti()
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

using CurlConnListHook_t = boost::intrusive::slist_member_hook<>;
using CurlOpt_t = std::pair<CURLoption, intptr_t>;

class CurlHeadersData_c
{
	struct curl_slist* m_pHttpHeadersList = nullptr;
	StrVec_t  m_dHeaders;

public:
	CurlHeadersData_c() = default;
	~CurlHeadersData_c()
	{
		Clear();
	}

	void Append ( const CSphString & sHeader )
	{
		assert ( !m_pHttpHeadersList );
		m_dHeaders.Add ( sHeader );
	}

	curl_slist * CurlSlist()
	{
		if ( !m_pHttpHeadersList && m_dHeaders.GetLength() )
		{
			for ( const CSphString & sHeader : m_dHeaders )
				m_pHttpHeadersList = sph_curl_slist_append ( m_pHttpHeadersList, sHeader.cstr() );
		}
		return m_pHttpHeadersList;
	}

	void Clear()
	{
		if ( !m_pHttpHeadersList )
			return;

		sph_curl_slist_free_all ( m_pHttpHeadersList );
		m_pHttpHeadersList = nullptr;
	}
};

class CurlConn_t
{
public:
	CurlConnListHook_t	m_tHook;
	int64_t           m_tmNextTry = 0;

private:
	CurlMulti_c *	m_pCurlMulti = nullptr;
	CURL*			m_pCurlEasy = nullptr;
	CURLcode		m_uReturnCode = CURLE_OK;
	char			m_sError[CURL_ERROR_SIZE] = { 0 };
	CSphString		m_sUrl;
	Threads::Coro::Waker_c m_tWaker;
	CSphVector<BYTE> m_dData;

	CSphVector<BYTE>  m_dReq;
	int               m_iRetriesLeft = 0;
	int               m_iDelay = 0;
	AgentConn_t * m_pRemote = nullptr;
	const RequestBuilder_i * m_pQuery = nullptr;
	ReplyParser_i * m_pParser = nullptr;
	CSphRefcountedPtr<Reporter_i> m_pReporter { nullptr };
	CurlHeadersData_c m_tCurlHeaders;

public:
	CurlConn_t ( CSphString sUrl, Threads::Coro::Worker_c * pWorker )
		: m_sUrl { std::move(sUrl) }
		, m_tWaker { Threads::CreateWaker ( pWorker ) }
	{
		InitEasyCommon();
		SetCurlOpt ( CURLOPT_URL, m_sUrl.cstr() );
	}

	CurlConn_t ( AgentConn_t * pRemote, const RequestBuilder_i * pQuery, ReplyParser_i * pParser, Reporter_i * pReporter, int iRetries, int iDelay, const CSphString & sBearer )
		: m_iRetriesLeft ( iRetries )
		, m_iDelay ( iDelay * 1000 )
		, m_pRemote ( pRemote )
		, m_pQuery ( pQuery )
		, m_pParser ( pParser )
	{
		SafeAddRef ( pReporter );
		m_pReporter = pReporter;

		m_pRemote->SetDescMultiAgent();
		if ( m_iRetriesLeft==-1 )
			m_iRetriesLeft = m_pRemote->GetRetries();
		if ( m_iDelay==-1 )
			m_iDelay = m_pRemote->GetDelay() * 1000;

		InitEasyCommon();

		// setup SSL
		if ( !GetSslCert().IsEmpty() )
		{
			SetCurlOpt ( CURLOPT_CAINFO, GetSslCert().cstr() );
			SetCurlOpt ( CURLOPT_SSL_VERIFYPEER, 1L );
			SetCurlOpt ( CURLOPT_SSL_VERIFYHOST, 2L );
		}

		CSphString sHeaderBearer;
		sHeaderBearer.SetSprintf ( "Authorization: Bearer %s", sBearer.cstr() );
		m_tCurlHeaders.Append ( sHeaderBearer );
		SetCurlOpt ( CURLOPT_HTTPHEADER, (intptr_t)m_tCurlHeaders.CurlSlist() );

		SetMultiUrl();
		SetMultiReqData();
		SetTimeouts();
	}

	~CurlConn_t()
	{
		CONN_INFO;
		Clean();
	}

	void AddToMulti() REQUIRES ( CurlStrand() );
	void OnDone ( CURLcode uResult ) REQUIRES ( CurlStrand() );

	void SetCurlOpt ( const VecTraits_T<CurlOpt_t> & dParams )
	{
		for ( const CurlOpt_t & tParam : dParams )
			SetCurlOpt ( tParam.first, tParam.second );
	}

	bool operator== ( const CurlConn_t & tOther ) const noexcept
	{
		return ( m_pCurlEasy==tOther.m_pCurlEasy );
	}

	UrlResult_t GetResult ( bool bBinary );
	void SetError ( const char * sError ) { strcpy ( m_sError, sError ); }
	void Retry();

private:

	template<typename TPARAM>
	void SetCurlOpt ( CURLoption eOpt, TPARAM tParam )
	{
		sph_curl_easy_setopt ( m_pCurlEasy, eOpt, tParam );
	}

	void InitEasyCommon()
	{
		m_pCurlEasy = sph_curl_easy_init();
		assert ( m_pCurlEasy );

		SetCurlOpt ( CURLOPT_SHARE, CurlMulti().GetSharePtr() );
		SetCurlOpt ( CURLOPT_TLS13_CIPHERS, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256" );
		SetCurlOpt ( CURLOPT_SSL_CIPHER_LIST, "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305" );
		SetCurlOpt ( CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2 );

		SetCurlOpt ( CURLOPT_FOLLOWLOCATION, 1 );
		SetCurlOpt ( CURLOPT_NOSIGNAL, 1 );
		SetCurlOpt ( CURLOPT_WRITEFUNCTION, WriteCbJump );
		SetCurlOpt ( CURLOPT_WRITEDATA, this );
		SetCurlOpt ( CURLOPT_ERRORBUFFER, m_sError );
		SetCurlOpt ( CURLOPT_PRIVATE, this );

		SetCurlOpt ( CURLOPT_FORBID_REUSE, 0L );
		SetCurlOpt ( CURLOPT_SSL_SESSIONID_CACHE, 1L );
		SetCurlOpt ( CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS );
		SetCurlOpt ( CURLOPT_TCP_KEEPALIVE, 1L );
		SetCurlOpt ( CURLOPT_TCP_KEEPIDLE, 30L );
		SetCurlOpt ( CURLOPT_TCP_KEEPINTVL, 15L );
	}

	void RemoveFromMulti() REQUIRES(CurlStrand())
	{
		assert ( m_pCurlMulti && m_pCurlEasy );
		sph_curl_multi_remove_handle ( m_pCurlMulti->GetMultiPtr(), m_pCurlEasy );
	}

	void Clean()
	{
		if ( m_pCurlEasy )
		{
			sph_curl_easy_cleanup ( m_pCurlEasy );
			m_pCurlEasy = nullptr;
		}
	}

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

	void OnDoneSingle ( CURLcode uResult );
	void OnDoneMulti ( CURLcode uResult );
	void SetMultiUrl();
	void SetMultiReqData();
	void SetTimeouts();
};

using CurlConnList_t = boost::intrusive::slist<CurlConn_t,
	boost::intrusive::member_hook<CurlConn_t, CurlConnListHook_t, &CurlConn_t::m_tHook>,
	boost::intrusive::constant_time_size<false>,
	boost::intrusive::cache_last<false>>;

CurlConnList_t g_tCurlConnections GUARDED_BY ( CurlStrand() );

void CurlConn_t::AddToMulti() REQUIRES ( CurlStrand() )
{
	m_pCurlMulti = &CurlMulti();
	CONN_INFO << "Add to multi " << m_pCurlMulti << " for " << m_sUrl;
		
	// FIXME!!! add tracker similar to simple requests for shutdown purposes
	if ( !m_pReporter )
		g_tCurlConnections.push_front ( *this );
	else
		m_pRemote->m_iStartQuery = sphMicroTimer();
			
	auto iRes = sph_curl_multi_add_handle ( m_pCurlMulti->GetMultiPtr(), m_pCurlEasy );
	CONN_INFO << "Add complete -> " << iRes;
}


void CurlConn_t::OnDone ( CURLcode uResult ) REQUIRES(CurlStrand())
{
	if ( m_pReporter )
		OnDoneMulti ( uResult );
	else
		OnDoneSingle ( uResult );
}

static void SetReplyError ( const VecTraits_T<BYTE> & dData, AgentConn_t * pRemote )
{
	if ( !dData.GetLength() )
		return;

	Str_t dReplyData ( dData );
	JsonObj_c tReplyParsed ( dReplyData );
	if ( !tReplyParsed )
		return;

	JsonObj_c tError = tReplyParsed.GetItem ( "error" ) ;
	if ( !tError )
		return;

	if ( tError.IsStr() )
		pRemote->m_sFailure = tError.StrVal();
}

void CurlConn_t::OnDoneMulti ( CURLcode uResult )
{
	bool bSuccess = false;
	if ( uResult==CURLE_OK )
	{
		long iHttpCode = 0;
		sph_curl_easy_getinfo ( m_pCurlEasy, CURLINFO_RESPONSE_CODE, &iHttpCode );
		if ( iHttpCode>=200 && iHttpCode<300 )
		{
			bSuccess = true;
			agent_stats_inc ( *m_pRemote, eNetworkNonCritical );
		} else
		{
			// HTTP error
			m_pRemote->m_sFailure.SetSprintf ( "HTTP request failed with code %d", (int)iHttpCode );
			agent_stats_inc ( *m_pRemote, eNetworkErrors );
			// could be remote error object stored in the reply data
			SetReplyError ( m_dData, m_pRemote );
		}

		// !COMMIT
		long iVer = 0;
		sph_curl_easy_getinfo ( m_pCurlEasy, CURLINFO_HTTP_VERSION, &iVer );
		sphInfo ( "curl ver %d: 1.1(%d), 2.0(%d), 2TLS(%d) ", (int)iVer, (int)CURL_HTTP_VERSION_1_1, (int)CURL_HTTP_VERSION_2_0, (int)CURL_HTTP_VERSION_2TLS );

	} else
	{
		agent_stats_inc ( *m_pRemote, eNetworkErrors );
		// curl error
		m_pRemote->m_sFailure = m_sError;
		if ( m_pRemote->m_sFailure.IsEmpty() )
			m_pRemote->m_sFailure = sph_curl_easy_strerror ( uResult );
	}

	RemoveFromMulti();

	if ( bSuccess )
	{
		if ( !m_pReporter->IsDone() && m_dData.GetLength() ) // should be non orphaned to work with parser
		{
			assert ( m_pParser );
			MemInputBuffer_c tIn ( m_dData );
			[[maybe_unused]] SearchdCommand_e eCmd = (SearchdCommand_e)tIn.GetWord ();
			[[maybe_unused]] DWORD uVer = tIn.GetWord();
			[[maybe_unused]] int iLen = tIn.GetInt();
			m_pParser->ParseReply ( tIn, *m_pRemote );
			// need move m_dData into m_pRemote.m_dReplyBuf as m_pParser->ParseReply keep pointers to data in this buffer
			// but conn could be destroyed 
			ByteBlob_t tBuf;
			tBuf.second = m_dData.GetLength();
			tBuf.first = m_dData.LeakData();
			m_pRemote->SetRecvBuf ( tBuf );
		}

		m_pRemote->m_bSuccess = true;
		m_pReporter->Report ( true );
		// cleanup finished handle
		Clean();
	} else
	{
		if ( m_iRetriesLeft>0 )
		{
			m_iRetriesLeft--;
			m_tmNextTry = sphMicroTimer() + m_iDelay;
			// will readd monitor coroutine loop do not neet to clean it yet
		} else
		{
			m_pRemote->m_bSuccess = false;
			m_pReporter->Report ( false );
			// cleanup failed handle without retry
			Clean();
		}
	}
	CONN_INFO << "DONE multi: (" << uResult << ") " << m_sError;
}

void CurlConn_t::OnDoneSingle ( CURLcode uResult )
{
	g_tCurlConnections.remove ( *this );
	m_uReturnCode = uResult;
	CONN_INFO << "DONE single: (" << uResult << ") " << m_sError;
	RemoveFromMulti();
	m_pCurlMulti = nullptr;
	m_tWaker.Wake ( true );
	// InvokeCurl deletes this \ unique_ptr after wake
}

UrlResult_t CurlConn_t::GetResult ( bool bBinary )
{
	if ( m_uReturnCode!=CURLE_OK )
		return { false, m_sError };

	if ( bBinary )
		return { true, std::move ( m_dData ) };
	else
		return { true, CSphString ( m_dData ) };
}

void CurlConn_t::Retry()
{
	m_dData.Resize ( 0 );
	m_tmNextTry = 0;

	uint64_t uPrevUrlHash = sphFNV64cont ( m_pRemote->m_tDesc.m_sAddr.cstr(), m_pRemote->m_tDesc.m_iPort );
	uint64_t uPrevIndexHash = sphFNV64 ( m_pRemote->m_tDesc.m_sIndexes.cstr() );
	m_pRemote->SetDescMultiAgent();
	uint64_t uUrlHash = sphFNV64cont ( m_pRemote->m_tDesc.m_sAddr.cstr(), m_pRemote->m_tDesc.m_iPort );
	uint64_t uIndexHash = sphFNV64 ( m_pRemote->m_tDesc.m_sIndexes.cstr() );

	if ( uPrevUrlHash!=uUrlHash )
		SetMultiUrl();
	if ( uPrevIndexHash!=uIndexHash )
		SetMultiReqData();

	// re-set timeouts counters
	SetTimeouts();

	AddToMulti();
}

void CurlConn_t::SetMultiUrl()
{
	if ( !GetSslCert().IsEmpty() )
		m_sUrl.SetSprintf ( "https://%s:%d/api", m_pRemote->m_tDesc.m_sAddr.cstr(), m_pRemote->m_tDesc.m_iPort );
	else
		m_sUrl.SetSprintf ( "http://%s:%d/api", m_pRemote->m_tDesc.m_sAddr.cstr(), m_pRemote->m_tDesc.m_iPort );

	SetCurlOpt ( CURLOPT_URL, m_sUrl.cstr() );
}

void CurlConn_t::SetMultiReqData()
{
	// build request body into curl buffer
	ISphOutputBuffer tOut;
	m_pQuery->BuildRequest ( *m_pRemote, tOut );
	m_dReq.SwapData ( tOut.m_dBuf );

	SetCurlOpt ( CURLOPT_POSTFIELDS, m_dReq.Begin() );
	SetCurlOpt ( CURLOPT_POSTFIELDSIZE, (long)m_dReq.GetLength() );
}

void CurlConn_t::SetTimeouts()
{
	// timeouts
	SetCurlOpt ( CURLOPT_CONNECTTIMEOUT_MS, (long)m_pRemote->m_iMyConnectTimeoutMs );
	SetCurlOpt ( CURLOPT_TIMEOUT_MS, (long)m_pRemote->m_iMyQueryTimeoutMs );
}

void CurlMulti_c::SocketAction ( curl_socket_t tCurlSocket, int iWhat ) const REQUIRES ( CurlStrand() )
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

		if ( pMsg->msg==CURLMSG_DONE )
		{
			CurlConn_t * pConn = nullptr;
			sph_curl_easy_getinfo ( pMsg->easy_handle, CURLINFO_PRIVATE, &pConn );
			assert ( pConn );
			pConn->OnDone ( pMsg->data.result );
		}
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

UrlResult_t InvokeCurl ( CSphString sUrl, const VecTraits_T<CurlOpt_t>& dParams, bool bBinary )
{
	if ( !IsCurlAvailable() )
		return { false, szNoCurlMsg };

	using namespace Threads::Coro;
	auto pWorker = CurrentWorker();

	std::unique_ptr<CurlConn_t> pRequest;

	YieldWith ( [&]() mutable {
		Go ( [&]() REQUIRES ( CurlStrand() )  mutable {
			pRequest = std::make_unique<CurlConn_t> ( std::move ( sUrl ), pWorker );
			pRequest->SetCurlOpt ( dParams );
			pRequest->AddToMulti();
		},
			CurlStrand() );
	} );

	return pRequest->GetResult ( bBinary );
}


CSphString FetchUrl ( const CSphString& sUrl )
{
	CSphVector<CurlOpt_t> dCurlOpts;
	dCurlOpts.Add ( { (CURLoption)237, 0L } ); // CURLOPT_PIPEWAIT. If not exist, will be just ignored
	auto tRes = InvokeCurl ( sUrl, dCurlOpts, false );
	return tRes.m_sRes;
}

UrlResult_t PostToHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<CSphString>& dHeaders )
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

	return InvokeCurl ( std::move ( sUrl ), dOptions, false );
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
			tConn.SetError ( "Interrupted due to shutdown" );
			tConn.OnDone( CURLE_ABORTED_BY_CALLBACK );
		}
		CurlMulti().Deinit();
	},
		CurlStrand() );
}

using SharedVecConn_t = std::shared_ptr<CSphVector<std::unique_ptr<CurlConn_t>>>;

static void InvokeMultiCurl ( SharedVecConn_t pConns, Reporter_i * pReporter ) REQUIRES ( CurlStrand() )
{
	using namespace Threads::Coro;

	CSphVector<CurlConn_t *> dRetries;
	while ( !pReporter->IsDone() )
	{
		CurlMulti().SocketAction ( CURL_SOCKET_TIMEOUT, 0 );

		int64_t iNow = sphMicroTimer();
		for ( int i=0; i<dRetries.GetLength(); ++i )
		{
			CurlConn_t * pConn = dRetries[i];
			if ( pConn->m_tmNextTry>0 && iNow>=pConn->m_tmNextTry )
			{
				pConn->Retry();
				dRetries.RemoveFast(i);
				i--;
			}
		}

		// collect retries
		for ( const auto & pConn : *pConns )
		{
			if ( pConn->m_tmNextTry>0 && !dRetries.any_of ( [&pConn] ( const CurlConn_t * pRetry ) { return pRetry==pConn.get(); } ) )
				dRetries.Add ( pConn.get() );
		}

		SleepMsec ( 1 ); // FIXME!!! add back Yield_() and m_tWaker.Wake(true);
	}
}

static void ScheduleHtppJobs ( VectorAgentConn_t & dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser, Reporter_i * pReporter, int iQueryRetry, int iQueryDelay )
{
	if ( dRemotes.IsEmpty() )
		return;

	using namespace Threads::Coro;

	SharedVecConn_t pConns = std::make_shared<CSphVector<std::unique_ptr<CurlConn_t>>>();
	pConns->Reserve ( dRemotes.GetLength() );

	CSphString sBearer = GetBearer ( session::GetUser() );

	for ( auto & pRemote : dRemotes )
	{
		auto pCon = std::make_unique<CurlConn_t> ( pRemote, pQuery, pParser, pReporter, iQueryRetry, iQueryDelay, sBearer );
		pReporter->FeedTask ( true );

		Go ( [ pRawCon = pCon.get() ]() { pRawCon->AddToMulti(); }
			, CurlStrand() );

		pConns->Add ( std::move ( pCon ) );
	}

	Go ( [ pConns, pReporter, pParser ]() mutable { InvokeMultiCurl ( pConns, pReporter ); }
		, CurlStrand() );
}


int PerformRemoteHttp ( VectorAgentConn_t & dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser, int iQueryRetry, int iQueryDelay )
{
	assert ( IsCurlAvailable() );

	RemoteAgentsObserver_i * pReporter = GetObserver();
	ScheduleHtppJobs ( dRemotes, pQuery, pParser, pReporter, iQueryRetry, iQueryDelay );

	if ( pReporter )
	{
		pReporter->Finish(); // Block until all tasks are done
		return pReporter->GetSucceeded();
	} else
	{
		return 0;
	}
}


#endif // WITH_CURL

// common code

UrlResult_t::UrlResult_t ( bool bOk, CSphString && sRes )
	: m_bOk ( bOk )
	, m_sRes ( std::move ( sRes ) )
{}

UrlResult_t::UrlResult_t ( bool bOk, CSphVector<BYTE> && dRes )
	: m_bOk ( bOk )
	, m_dRes ( std::move ( dRes ) )
{}

void ScheduleDistrJobs ( VectorAgentConn_t &dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser, Reporter_i * pReporter, int iQueryRetry, int iQueryDelay )
{
	if ( IsAuthEnabled() && IsCurlAvailable() )
	{
		ScheduleHtppJobs ( dRemotes, pQuery, pParser, pReporter, iQueryRetry, iQueryDelay );
	} else
	{
		ScheduleDistrJobsApi ( dRemotes, pQuery, pParser, pReporter, iQueryRetry, iQueryDelay );
	}
}

int PerformRemoteTasks ( VectorAgentConn_t &dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser, int iQueryRetry , int iQueryDelay )
{
	if ( IsAuthEnabled() && IsCurlAvailable() )
	{
		return PerformRemoteHttp ( dRemotes, pQuery, pParser, iQueryRetry, iQueryDelay );
	} else
	{
		return PerformRemoteTasksApi ( dRemotes, pQuery, pParser, iQueryRetry, iQueryDelay );
	}
}