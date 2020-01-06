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
		case Proto_e::HTTPS: return "https";
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

void ThdDesc_t::SetThreadInfo ( const char* sTemplate, ... )
{
	ScopedMutex_t dLock ( m_tQueryLock );
	m_sBuf.Clear();
	va_list ap;
	va_start ( ap, sTemplate );
	m_sBuf.vSprintf ( sTemplate, ap );
	va_end ( ap );
}

void ThdDesc_t::SetSearchQuery ( const CSphQuery* pQuery )
{
	ScopedMutex_t dLock (m_tQueryLock);
	m_pQuery = pQuery;
}

ThreadSystem_t::ThreadSystem_t ( const char* sName )
{
	m_tDesc.m_bSystem = true;
	m_tDesc.m_tmStart = sphMicroTimer ();
	m_tDesc.m_iTid = GetOsThreadId ();
	m_tDesc.SetThreadInfo ( "SYSTEM %s", sName );
	m_tDesc.m_sCommand = g_sSystemName;

	m_tDesc.AddToGlobal();
}

ThreadSystem_t::~ThreadSystem_t ()
{
	m_tDesc.RemoveFromGlobal ();
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

	m_tDesc.AddToGlobal ();
}

ThreadLocal_t::~ThreadLocal_t ()
{
	m_tDesc.RemoveFromGlobal ();
}

void Threads::ThdDesc_t::AddToGlobal () EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Add ( this );
}

void Threads::ThdDesc_t::RemoveFromGlobal () EXCLUDES ( g_tThdLock )
{
	ScWL_t dThdLock ( g_tThdLock );
	g_dThd.Remove ( this );
}

Threads::ThdPublicInfo_t Threads::ThdDesc_t::GetPublicInfo ()
{
	ThdPublicInfo_t dResult;
	ThdInfo_t& dSemiRes = dResult;
	dSemiRes = *( ThdInfo_t* ) this;

	dResult.m_sThName = GetThreadName ( &m_tThd );

	ScopedMutex_t dLock ( m_tQueryLock );
	if ( m_pQuery )
		dResult.m_pQuery = new CSphQuery ( *m_pQuery );
	dResult.m_sRequestDescription = m_sBuf.cstr();
	return dResult;
}

Threads::ThdPublicInfo_t::~ThdPublicInfo_t ()
{
	SafeDelete ( m_pQuery );
}

void Threads::ThdPublicInfo_t::Swap( ThdPublicInfo_t& rhs )
{
	::Swap ( (*(ThdInfo_t*)(this)), *(ThdInfo_t*)(&rhs));
	::Swap ( m_sThName, rhs.m_sThName );
	::Swap( m_pQuery, rhs.m_pQuery );
	::Swap( m_sRequestDescription, rhs.m_sRequestDescription );
}

Threads::ThdPublicInfo_t::ThdPublicInfo_t( ThdPublicInfo_t && rhs) noexcept
{
	Swap(rhs);
}

Threads::ThdPublicInfo_t& Threads::ThdPublicInfo_t::operator=( ThdPublicInfo_t && rhs ) noexcept
{
	Swap(rhs);
	return *this;
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
}


List_t& Threads::GetUnsafeUnlockedUnprotectedGlobalThreadList () NO_THREAD_SAFETY_ANALYSIS
{
	return g_dThd;
}

CSphSwapVector<ThdPublicInfo_t> Threads::GetGlobalThreadInfos ()
{
	ScRL_t dThdLock ( g_tThdLock );
	CSphSwapVector<ThdPublicInfo_t> dResult;
	dResult.Reserve ( g_dThd.GetLength ());
	for ( const ListNode_t* pIt = g_dThd.Begin (); pIt!=g_dThd.End (); pIt = pIt->m_pNext )
		dResult.Add ((( ThdDesc_t* ) pIt )->GetPublicInfo ());
	return dResult;
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