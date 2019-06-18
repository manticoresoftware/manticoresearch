//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "threadutils.h"
#include "sphinx.h"

#if !USE_WINDOWS
// UNIX-specific headers and calls
#include <sys/syscall.h>

// for thr_self()
#ifdef __FreeBSD__
#include <sys/thr.h>
#endif

#endif


static RwLock_t g_tThdLock;
static List_t g_dThd GUARDED_BY ( g_tThdLock );   ///< existing threads table
static const char* g_sSystemName = "SYSTEM";

using namespace Threads;

const char* ThdStateName ( ThdState_e eState )
{
	switch (eState)
	{
		case ThdState_e::HANDSHAKE: return "handshake";
		case ThdState_e::NET_READ: return "net_read";
		case ThdState_e::NET_WRITE: return "net_write";
		case ThdState_e::QUERY: return "query";
		case ThdState_e::NET_IDLE: return "net_idle";
	}
	return "unknown";
}

const char* ProtoName ( Proto_e eProto )
{
	switch ( eProto )
	{
		case Proto_e::SPHINX: return "sphinxapi";
		case Proto_e::MYSQL41: return "sphinxql";
		case Proto_e::HTTP: return "http";
		case Proto_e::REPLICATION: return "replication";
	}
	return "unknown";
}

ServiceThread_t::~ServiceThread_t ()
{
	Join ();
}

bool ServiceThread_t::Create ( void (* fnThread) ( void* ), void* pArg, const char* sName )
{
	m_bCreated = sphThreadCreate ( &m_tThread, fnThread, pArg, false, sName );
	return m_bCreated;
}

void ServiceThread_t::Join ()
{
	if ( m_bCreated && sphGetShutdown ())
		sphThreadJoin ( &m_tThread );
	m_bCreated = false;
}

ThdDesc_t::ThdDesc_t ()
{
	m_dBuf[0] = '\0';
	m_dBuf.Last () = '\0';
}

void ThdDesc_t::SetThreadInfo ( const char* sTemplate, ... )
{
	// thread safe modification of string at m_dBuf
	m_dBuf[0] = '\0';
	m_dBuf.Last () = '\0';

	va_list ap;

	va_start ( ap, sTemplate );
	int iPrinted = vsnprintf ( m_dBuf.Begin (), m_dBuf.GetLength () - 1, sTemplate, ap );
	va_end ( ap );

	if ( iPrinted>0 )
		m_dBuf[Min ( iPrinted, m_dBuf.GetLength () - 1 )] = '\0';
}

void ThdDesc_t::SetSearchQuery ( const CSphQuery* pQuery )
{
	m_tQueryLock.Lock ();
	m_pQuery = pQuery;
	m_tQueryLock.Unlock ();
}

ThreadSystem_t::ThreadSystem_t ( const char* sName )
{
	m_tDesc.m_bSystem = true;
	m_tDesc.m_tmStart = sphMicroTimer ();
	m_tDesc.m_iTid = GetOsThreadId ();
	m_tDesc.SetThreadInfo ( "SYSTEM %s", sName );
	m_tDesc.m_sCommand = g_sSystemName;

	ThreadAdd ( &m_tDesc );
}

ThreadSystem_t::~ThreadSystem_t ()
{
	ThreadRemove ( &m_tDesc );
}

ThreadLocal_t::ThreadLocal_t ( const ThdDesc_t& tDesc )
{
	m_tDesc.m_iTid = GetOsThreadId ();
	m_tDesc.m_eProto = tDesc.m_eProto;
	m_tDesc.m_iClientSock = tDesc.m_iClientSock;
	m_tDesc.m_sClientName = tDesc.m_sClientName;
	m_tDesc.m_eThdState = tDesc.m_eThdState;
	m_tDesc.m_sCommand = tDesc.m_sCommand;
	m_tDesc.m_iConnID = tDesc.m_iConnID;
	m_tDesc.m_iCookie = tDesc.m_iCookie;

	m_tDesc.m_tmConnect = tDesc.m_tmConnect;
	m_tDesc.m_tmStart = tDesc.m_tmStart;

	ThreadAdd ( &m_tDesc );
}

ThreadLocal_t::~ThreadLocal_t ()
{
	ThreadRemove ( &m_tDesc );
}


void Threads::ThreadSetSnippetInfo ( const char* sQuery, int64_t iSizeKB, bool bApi, ThdDesc_t& tThd )
{
	if ( bApi )
		tThd.SetThreadInfo ( "api-snippet datasize=%d.%d""k query=\"%s\"", int ( iSizeKB / 10 ), int ( iSizeKB % 10 ),
							 sQuery );
	else
		tThd.SetThreadInfo ( "sphinxql-snippet datasize=%d.%d""k query=\"%s\"", int ( iSizeKB / 10 ),
							 int ( iSizeKB % 10 ), sQuery );
}

void Threads::ThreadSetSnippetInfo ( const char* sQuery, int64_t iSizeKB, ThdDesc_t& tThd )
{
	tThd.SetThreadInfo ( "snippet datasize=%d.%d""k query=\"%s\"", int ( iSizeKB / 10 ), int ( iSizeKB % 10 ), sQuery );
}

void Threads::ThreadAdd ( ThdDesc_t* pThd ) EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Add ( pThd );
}

void Threads::ThreadRemove ( ThdDesc_t* pThd ) EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Remove ( pThd );
}

int Threads::ThreadsNum () EXCLUDES ( g_tThdLock )
{
	ScRL_t dThdLock ( g_tThdLock );
	return g_dThd.GetLength ();
}

void Threads::ThdState ( ThdState_e eState, ThdDesc_t& tThd )
{
	tThd.m_eThdState = eState;
	tThd.m_tmStart = sphMicroTimer ();
	if ( eState==ThdState_e::NET_IDLE )
		tThd.m_dBuf[0] = '\0';
}

RlockedList_t RlockedList_t::GetGlobalThreads ()
{
	RlockedList_t tRes;
	tRes.m_pCore = &g_dThd;
	tRes.m_pLock = &g_tThdLock;
	g_tThdLock.ReadLock ();
	return tRes;
}

RlockedList_t::~RlockedList_t()
{
	if ( m_pLock )
		m_pLock->Unlock ();
}

void RlockedList_t::Swap ( RlockedList_t& rhs) noexcept
{
	::Swap ( m_pLock, rhs.m_pLock );
	::Swap ( m_pCore, rhs.m_pCore );
}

RlockedList_t& RlockedList_t::operator= ( RlockedList_t&& rhs ) noexcept
{
	Swap(rhs);
	return *this;
}

RlockedList_t::RlockedList_t ( RlockedList_t && rhs ) noexcept
{
	Swap (rhs);
}


int GetOsThreadId ()
{
#if USE_WINDOWS
	return GetCurrentThreadId();
#elif defined ( __APPLE__ )
	uint64_t tid;
	 pthread_threadid_np(NULL, &tid);
	return tid;
#elif defined(SYS_gettid)
	return syscall ( SYS_gettid );
#elif defined(__FreeBSD__)
	long tid;
	thr_self(&tid);
	return (int)tid;
#else
	return 0;
#endif
}