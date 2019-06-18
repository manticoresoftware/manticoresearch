//
// Copyright (c) 2019, Manticore Software LTD (http://manticoresearch.com)
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
	SPHINX,
	MYSQL41,
	HTTP,
	REPLICATION,
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

struct ThdDesc_t: public ListNode_t
{
	SphThread_t m_tThd { 0 };
	Proto_e m_eProto = Proto_e::MYSQL41;
	int m_iClientSock = 0;
	CSphString m_sClientName;
	bool m_bVip = false;

	ThdState_e m_eThdState = ThdState_e::HANDSHAKE;
	const char* m_sCommand = nullptr;
	int m_iConnID = -1;    ///< current conn-id for this thread

	// stuff for SHOW THREADS
	int m_iTid = 0;        ///< OS thread id, or 0 if unknown
	int64_t m_tmConnect = 0;    ///< when did the client connect?
	int64_t m_tmStart = 0;    ///< when did the current request start?
	bool m_bSystem = false;
	CSphFixedVector<char> m_dBuf { 512 };    ///< current request description
	int m_iCookie = 0;    ///< may be used in case of pool to distinguish threads

	CSphMutex m_tQueryLock;
	const CSphQuery* m_pQuery GUARDED_BY (m_tQueryLock) = nullptr;

	ThdDesc_t ();
	void SetThreadInfo ( const char* sTemplate, ... );
	void SetSearchQuery ( const CSphQuery* pQuery );
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
	explicit ThreadLocal_t ( const ThdDesc_t& tDesc );
	~ThreadLocal_t ();
};

void ThreadSetSnippetInfo ( const char* sQuery, int64_t iSizeKB, bool bApi, ThdDesc_t& tThd );
void ThreadSetSnippetInfo ( const char* sQuery, int64_t iSizeKB, ThdDesc_t& tThd );
void ThreadAdd ( ThdDesc_t* pThd );
void ThreadRemove ( ThdDesc_t* pThd );
int ThreadsNum ();
void ThdState ( ThdState_e eState, ThdDesc_t& tThd );

// used to hide global list of threads and give only rlocked read-only ref on demand.
class RlockedList_t: ISphNoncopyable
{
	RlockedList_t() = default;
public:
	static RlockedList_t GetGlobalThreads ();
	/// unlock on going out of scope
	~RlockedList_t ();

	const List_t* operator-> () const 	{ return m_pCore; }
	explicit operator bool () const	{ return m_pCore!=nullptr; }
	explicit operator const List_t* () const { return m_pCore; }

	void Swap ( RlockedList_t& rhs ) noexcept;
	RlockedList_t& operator= ( RlockedList_t&& rhs) noexcept;
	RlockedList_t ( RlockedList_t&& rhs ) noexcept;

private:
	const List_t* m_pCore = nullptr;
	RwLock_t* m_pLock = nullptr;
};

} // namespace Threads



#endif //MANTICORE_THREADUTILS_H
