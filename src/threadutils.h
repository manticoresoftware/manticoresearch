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
/// @file threadutils.h
/// Different thread-related things (moved here from searchd.cpp)

#ifndef MANTICORE_THREADUTILS_H
#define MANTICORE_THREADUTILS_H

#include "sphinxstd.h"
#include "sphinxint.h"

struct CSphQuery;

enum class ThdState_e
{
	HANDSHAKE,
	NET_READ,
	NET_WRITE,
	QUERY,
	NET_IDLE,
};

enum class Proto_e
{
	UNKNOWN,
	SPHINX,
	MYSQL41,
	HTTP,
	REPLICATION,
	HTTPS
};

const char* ThdStateName( ThdState_e eState);
const char* ProtoName ( Proto_e eProto );

int GetOsThreadId ();

namespace Threads {

struct ServiceThread_t
{
	SphThread_t m_tThread;
	bool m_bCreated = false;

	~ServiceThread_t ();
	bool Create ( void (* fnThread) ( void* ), void* pArg, const char* sName = nullptr );
	void Join ();
};

// common thread properties
struct ThdInfo_t
{
	Proto_e	   m_eProto		 = Proto_e::MYSQL41;
	int		   m_iClientSock = 0;
	CSphString m_sClientName;
	bool	   m_bVip = false;

	ThdState_e	m_eThdState = ThdState_e::HANDSHAKE;
	const char* m_sCommand	= nullptr;
	int			m_iConnID	= -1; ///< current conn-id for this thread
	bool 		m_bSsl = false;

	// stuff for SHOW THREADS
	int		m_iTid		= 0; ///< OS thread id, or 0 if unknown
	int64_t m_tmConnect = 0; ///< when did the client connect?
	int64_t m_tmStart	= 0; ///< when did the current request start?
	bool	m_bSystem	= false;
};

// trivial info for public use (no locks, everything owned)
struct ThdPublicInfo_t : ThdInfo_t, public ISphNoncopyable
{
	CSphString m_sThName;
	CSphQuery* m_pQuery = nullptr;
	CSphString m_sRequestDescription;

	ThdPublicInfo_t() = default;
	~ThdPublicInfo_t();

	void Swap( ThdPublicInfo_t& rhs );
	ThdPublicInfo_t( ThdPublicInfo_t&& rhs) noexcept;
	ThdPublicInfo_t& operator= ( ThdPublicInfo_t&& rhs) noexcept;
};



struct ThdDesc_t: public ThdInfo_t, private ListNode_t
{
	SphThread_t m_tThd { 0 };
	CSphMutex		 m_tQueryLock;
	const CSphQuery* m_pQuery GUARDED_BY ( m_tQueryLock ) = nullptr;
	StringBuilder_c m_sBuf GUARDED_BY ( m_tQueryLock ); ///< current request description

	void SetThreadInfo ( const char* sTemplate, ... ) REQUIRES ( !m_tQueryLock );
	void SetSearchQuery ( const CSphQuery* pQuery ) REQUIRES ( !m_tQueryLock );

	void AddToGlobal ();
	void RemoveFromGlobal ();
	ThdPublicInfo_t GetPublicInfo();
};

struct ThreadSystem_t
{
	ThdDesc_t m_tDesc;
	explicit ThreadSystem_t ( const char* sName );
	~ThreadSystem_t ();
};

struct ThreadLocal_t
{
	ThdDesc_t m_tDesc;
	bool m_bInitialized = false;
	explicit ThreadLocal_t ( const ThdDesc_t& tDesc );
	ThreadLocal_t () = default;
	~ThreadLocal_t ();

	void FinishInit();
	void ThdState ( ThdState_e eState );
};

int ThreadsNum ();
void ThdState ( ThdState_e eState, ThdDesc_t& tThd );

// WARNING! Should be used only in crash dumping, where any copying is UB!
List_t& GetUnsafeUnlockedUnprotectedGlobalThreadList();

// Trivial vec of global thread descriptors
CSphSwapVector<ThdPublicInfo_t> GetGlobalThreadInfos ();

//////////////////////////////////////////////////////////////////////////
/// functional threadpool with minimum footprint
using Handler = std::function<void ()>;
using Predicate = std::function<bool ()>;
using Handlers = std::initializer_list<Handler>;

// used to RAII keep Scheduler running (when work finished - it is usually destroyed)
using Keeper_t = SharedPtrCustom_t<void*>;

struct Scheduler_i
{
	virtual ~Scheduler_i() {};
	virtual void Schedule ( Handler handler, bool bVip ) = 0;
	virtual void ScheduleContinuation ( Handler handler )
	{
		Schedule ( std::move ( handler ), false );
	}
	virtual Keeper_t KeepWorking() = 0;
	virtual const char * szName () const { return "<unknown>"; }
	virtual int WorkingThreads() const = 0;
	virtual long Works () const = 0;
	virtual void Wait () = 0;
	virtual void DiscardOnFork() {};
};

using SchedulerSharedPtr_t = SharedPtr_t<Scheduler_i *>;

// none of the functions below used in the code. Both maybe only in tests.
SchedulerSharedPtr_t MakeThreadPool ( size_t iThreadCount, const char * szName="" );
SchedulerSharedPtr_t MakeAloneThread ( size_t iOrderNum, const char * szName = "" );

// schedule global watchdog to gracefully join alone threads on exit
void AloneShutdowncatch ();

} // namespace Threads



extern ThreadRole MainThread;


namespace CrashLogger
{
#if !USE_WINDOWS
	void HandleCrash( int );
#else
	LONG WINAPI HandleCrash ( EXCEPTION_POINTERS * pExc );
#endif
	void SetupTimePID();
};

// Scheduler to global thread pool
Threads::Scheduler_i* GetGlobalScheduler();
void WipeGlobalSchedulerAfterFork();
long GetGlobalQueueSize();
long GetGlobalThreads ();
void SetGlobalThreads ( int iThreads );

// Scheduler to dedicated thread (or nullptr, if current N of such threads >= iMaxThreads)
// you MUST schedule at least one job, or explicitly delete non-engaged scheduler (it will leak otherwise).
Threads::Scheduler_i* GetAloneScheduler ( int iMaxThreads, const char* szName=nullptr );

using SchedulerFabric_fn = std::function<Threads::Scheduler_i * ( void )>;

// add handler which will be called on daemon's shutdown right after
// sphInterrupted() is set to true. Returns cookie for refer the callback in future.
using Handler_fn = std::function<void ()>;

namespace searchd {

	void* AddShutdownCb ( Handler_fn fnCb );

	// remove previously set shutdown cb by cookie
	void DeleteShutdownCb ( void* pCookie );

	// invoke shutdown handlers
	void FireShutdownCbs ();

	void CleanAfterFork();
}


#endif //MANTICORE_THREADUTILS_H
