//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once
#include "wsrep_cxx.h"
#include "task_info.h"
#include "coroutine.h"

bool CheckNoWarning ( const char* szMsg );

enum class LogLevel_e {
	FATAL,
	ERROR_, // named this way because of macro conflict with winnt.h
	WARN,
	INFO,
	DEBUG
};

void WsrepLog ( LogLevel_e eLevel, const char* szMsg );

// log debug info about nodes as current nodes views that
void LogGroupView ( const Wsrep::ViewInfo_t* pView );

// display incoming string as client name in show threads
struct GaleraInfo_t: public TaskInfo_t
{
	DECLARE_RENDER ( GaleraInfo_t );
	CSphString m_sName; // set once on create, so is th-safe
	explicit GaleraInfo_t ( const CSphString& sName )
		: m_sName { sName }
	{}
};

enum class CbStatus_e {
	SUCCESS = 0,
	FAILURE,
};

inline constexpr CbStatus_e Bool2Cb ( bool b ) noexcept
{
	return b ? CbStatus_e::SUCCESS : CbStatus_e::FAILURE;
}

// common structs and flags
using InitArgs_t = void; // actually it is version-dependent struct
using PoHandle_t = void;

struct TxHandle_t
{
	uint64_t m_uTrx = UINT64_MAX;
	void* m_pCookie = nullptr;
};

struct Buf_t
{
	const void* m_pData;
	uint64_t m_uLen;
};

struct Key_t
{
	const Buf_t* m_pParts;
	uint64_t m_uCount;
};

enum class KeyType_e {
	SHARED = 0,
	SEMI,
	EXCLUSIVE
};

enum class DataType_e {
	ORDERED = 0,
	UNORDERED,
	ANNOTATION
};

struct DwFlags_t
{
	static constexpr DWORD COMMIT = 1ULL;
	static constexpr DWORD ROLLBACK = 2ULL;
	static constexpr DWORD ISOLATION = 4ULL;
	static constexpr DWORD PA_UNSAFE = 8ULL;
	static constexpr DWORD COMMUTATIVE = 16ULL;
	static constexpr DWORD NATIVE = 32ULL;
};

extern std::atomic<uint64_t> uWritesetConnIds;

// wrap replication writeset to RAII
template<typename WSREPWRAP>
class Writeset_T final: public Wsrep::Writeset_i
{
	using Status_e = typename WSREPWRAP::Status_e_;
	CSphRefcountedPtr<WSREPWRAP> m_pWsrep;
	uint64_t m_uConnId;
	Wsrep::TrxMeta_t m_tMeta;
	TxHandle_t m_tHnd;
	Status_e m_eLastRes = Status_e::OK;
	bool m_bNeedPostRollBack = false; // if we need rollback on exit, not just FreeConnection

	// check return code from Galera calls
	// FIXME!!! handle more status codes properly
	bool CheckResult ( const char* sAt, bool bCritical = false )
	{
		if ( LastOk() )
			return true;

		TlsMsg::Err ( "error at %s, code %d (%s), seqno %l", sAt, m_eLastRes, WSREPWRAP::szGetStatus ( m_eLastRes ), m_tMeta.m_tGtid.m_iSeqNo );
		sphWarning ( "%s", TlsMsg::szError() );
		if ( bCritical )
			m_bNeedPostRollBack = true;
		return false;
	}

public:
	explicit Writeset_T ( WSREPWRAP* pWsrep )
		: m_pWsrep { pWsrep }
		, m_uConnId { uWritesetConnIds.fetch_add ( 1, std::memory_order_relaxed ) }
	{
		pWsrep->AddRef();
		m_tHnd.m_uTrx = m_uConnId;
	}

	[[nodiscard]] int64_t LastSeqno() const noexcept final
	{
		return m_tMeta.m_tGtid.m_iSeqNo;
	}

	[[nodiscard]] bool LastOk() const noexcept final
	{
		return m_eLastRes == Status_e::OK;
	}

	[[nodiscard]] bool AppendKeys ( const VecTraits_T<uint64_t>& dBufKeys, bool bSharedKeys ) final
	{
		auto iKeysCount = dBufKeys.GetLength();

		// set keys wsrep_buf_t ptr and len
		CSphFixedVector<Buf_t> dBufProxy { iKeysCount };
		CSphFixedVector<Key_t> dKeys { iKeysCount };
		ARRAY_CONSTFOREACH ( i, dBufKeys )
		{
			dBufProxy[i].m_pData = &dBufKeys[i];
			dBufProxy[i].m_uLen = sizeof ( uint64_t );
			dKeys[i].m_pParts = &dBufProxy[i];
			dKeys[i].m_uCount = 1;
		}

		m_eLastRes = m_pWsrep->AppendKey ( &m_tHnd, dKeys.begin(), iKeysCount, ( bSharedKeys ? KeyType_e::SHARED : KeyType_e::EXCLUSIVE ), false );
		return CheckResult ( "AppendKeys" );
	}

	[[nodiscard]] bool AppendData ( const VecTraits_T<BYTE>& dData ) final
	{
		Buf_t tQueries { dData.Begin(), (uint64_t)dData.GetLength64() };
		m_eLastRes = m_pWsrep->AppendData ( &m_tHnd, &tQueries, 1, DataType_e::ORDERED, false );
		return CheckResult ( "AppendData" );
	}

	[[nodiscard]] bool PreCommit() final
	{
		m_eLastRes = m_pWsrep->PreCommit ( m_uConnId, &m_tHnd, DwFlags_t::COMMIT, &m_tMeta );
		return CheckResult ( "PreCommit", true );
	}

	void AbortPreCommit() final
	{
		m_eLastRes = m_pWsrep->AbortPreCommit ( Wsrep::WRONG_SEQNO, m_tHnd.m_uTrx );
		CheckResult ( "AbortPreCommit" );
		m_bNeedPostRollBack = true;
	}

	void InterimCommit() final;
	[[nodiscard]] bool Replicate() final
	{
		m_eLastRes = Status_e::OK; // no 'replicate' function in v25
		return true;
	}

	void PostCommit() final
	{
		m_eLastRes = m_pWsrep->PostCommit ( &m_tHnd );
		CheckResult ( "PostCommit" );
		m_bNeedPostRollBack = false;
	}

	// TOI stuff
	[[nodiscard]] bool ToExecuteStart ( const VecTraits_T<uint64_t>& dBufKeys, const VecTraits_T<BYTE>& dData ) final
	{
		auto iKeysCount = dBufKeys.GetLength();

		// set keys ptr and len
		CSphFixedVector<Buf_t> dBufProxy ( iKeysCount );
		CSphFixedVector<Key_t> dKeys ( iKeysCount );
		for ( int i = 0; i < iKeysCount; ++i )
		{
			dBufProxy[i].m_pData = &dBufKeys[i];
			dBufProxy[i].m_uLen = sizeof ( uint64_t );
			dKeys[i].m_pParts = &dBufProxy[i];
			dKeys[i].m_uCount = 1;
		}

		Buf_t tQueries { dData.Begin(), (uint64_t)dData.GetLength() };
		m_eLastRes = m_pWsrep->ToExecuteStart ( m_uConnId, dKeys.begin(), iKeysCount, &tQueries, 1, &m_tMeta );
		return CheckResult ( "ToExecuteStart" );
	}

	void ToExecuteEnd() final
	{
		m_eLastRes = m_pWsrep->ToExecuteEnd ( m_uConnId );
		CheckResult ( "ToExecuteEnd" );
	}

	~Writeset_T() final
	{
		if ( m_bNeedPostRollBack )
		{
			m_pWsrep->PostRollback ( &m_tHnd );
			CheckResult ( "PostRollback" );
		}
		m_pWsrep->FreeConnection ( m_uConnId );
	}
};

template<typename WSREPWRAP>
class Provider_T: public Wsrep::Provider_i
{
protected:
	using Status_e = typename WSREPWRAP::Status_e_;
	CSphRefcountedPtr<WSREPWRAP> m_pWsrep;
	Wsrep::Cluster_i* m_pCluster;
	Wsrep::ReceiverRefPtr_c m_pReceiver;
	Wsrep::GlobalTid_t m_tStateID;
	CSphString m_sName;

protected:
	static CbStatus_e ViewChanged_fn ( void* pAppCtx, void* pRecvCtx, const Wsrep::ViewInfo_t* pView, const char* pState, uint64_t iStateLen, void** ppSstReq, uint64_t* pSstReqLen )
	{
		assert ( pAppCtx );
		Wsrep::ClusterRefPtr_c pCluster { (Wsrep::Cluster_i*)pAppCtx };
		pCluster->AddRef();

		LogGroupView ( pView );
		pCluster->ChangeView ( pView, pState, iStateLen, ppSstReq, pSstReqLen );
		return Bool2Cb ( true );
	}

	// callback for Galera synced_cb that cluster fully synced and could accept transactions
	static void Synced_fn ( void* pAppCtx )
	{
		assert ( pAppCtx );
		Wsrep::ClusterRefPtr_c pCluster { (Wsrep::Cluster_i*)pAppCtx };
		pCluster->AddRef();
		pCluster->SetSynced();
	}

	static CbStatus_e Apply_fn ( void* pRecvCtx, const void* pData, uint64_t uSize, DWORD uFlags, const Wsrep::TrxMeta_t* pMeta )
	{
		assert ( pRecvCtx );
		Wsrep::ReceiverRefPtr_c pReceiver { (Wsrep::Receiver_i*)pRecvCtx };
		pReceiver->AddRef();

		bool bCommit = ( ( uFlags & DwFlags_t::COMMIT ) != 0 );
		bool bIsolated = ( ( uFlags & DwFlags_t::ISOLATION ) != 0 );
		sphLogDebugRpl ( "writeset at apply, seq " INT64_FMT ", size %d, flags %u, on %s", pMeta->m_tGtid.m_iSeqNo, (int)uSize, uFlags, ( bCommit ? "commit" : "rollback" ) );

		return Bool2Cb ( pReceiver->ApplyWriteset ( { (const BYTE*)pData, uSize }, bIsolated ) );
	}

	// callback for Galera to become of donor and start sending SST (all cluster indexes) to joiner
	static CbStatus_e SstDonate_fn ( void* pAppCtx, void* pRecvCtx, const void* sMsg, uint64_t uMsgLen, const Wsrep::GlobalTid_t* pStateID, const char* pState, uint64_t uStateLen, bool bBypass )
	{
		assert ( pAppCtx );
		Wsrep::ClusterRefPtr_c pCluster { (Wsrep::Cluster_i*)pAppCtx };
		pCluster->AddRef();

		CSphString sNode;
		sNode.SetBinary ( (const char*)sMsg, uMsgLen );

		return Bool2Cb ( pCluster->DonateSST ( std::move ( sNode ), pStateID, bBypass ) );
	}

	// template callback for Galera commit that transaction received and parsed before should be committed or rolled back
	static CbStatus_e Commit_fn ( void* pRecvCtx, const void* hndTrx, DWORD uFlags, const Wsrep::TrxMeta_t* pMeta, bool* pExit, bool bCommit )
	{
		assert ( pRecvCtx );
		Wsrep::ReceiverRefPtr_c pReceiver { (Wsrep::Receiver_i*)pRecvCtx };
		pReceiver->AddRef();

		sphLogDebugRpl ( "writeset at %s, seq " INT64_FMT ", flags %u", ( bCommit ? "commit" : "rollback" ), pMeta->m_tGtid.m_iSeqNo, uFlags );

		return Bool2Cb ( pReceiver->Commit ( hndTrx, uFlags, pMeta, bCommit ) );
	}

	// callback for Galera that unordered transaction received
	static CbStatus_e Unordered_fn ( void* pRecvCtx, const void* pData, uint64_t uSize )
	{
		assert ( pRecvCtx );
		Wsrep::ReceiverRefPtr_c pReceiver { (Wsrep::Receiver_i*)pRecvCtx };
		pReceiver->AddRef();
		pReceiver->ApplyUnordered ( { (const BYTE*)pData, uSize } );
		return Bool2Cb ( true );
	}

public:
	Provider_T ( Wsrep::Cluster_i* pCluster, WSREPWRAP* pWsrep )
		: m_pWsrep { pWsrep }
		, m_pCluster ( pCluster )
	{
		assert ( pCluster );
		pWsrep->AddRef();
	}

	void SstReceived ( const Wsrep::GlobalTid_t& tGtid, int iRes ) final
	{
		m_pWsrep->SstReceived ( &tGtid, nullptr, 0, iRes );
	}

	void SstSent ( const Wsrep::GlobalTid_t& tGtid, bool bOk ) final
	{
		m_pWsrep->SstSent ( &tGtid, ( bOk ? 0 : -ECANCELED ) );
	}

	void Disconnect() final
	{
		m_pWsrep->Disconnect();
	}

	bool OptionsSet ( const CSphString& sName, const CSphString& sValue ) final
	{
		StringBuilder_c sOpt;
		sOpt << sName << "=" << sValue;

		auto tOk = m_pWsrep->OptionsSet ( sOpt.cstr() );
		return tOk == Status_e::OK || TlsMsg::Err ( GetStatus ( tOk ) );
	}

	bool EnumFindStatsVar ( Wsrep::StatsVarsIteratorFN&& fnIterator ) final
	{
		auto* pAllVars = m_pWsrep->StatsGet();
		AT_SCOPE_EXIT ( [pAllVars, this]() { m_pWsrep->StatsFree ( pAllVars ); } );
		for ( auto* pVars = pAllVars; pVars->m_szName; ++pVars )
			if ( fnIterator ( *pVars ) )
				return true;
		return false;
	}

	void EnumStatsVars ( Wsrep::StatsVarsVisitorFN&& fnVisitor ) final
	{
		auto* pAllVars = m_pWsrep->StatsGet();
		AT_SCOPE_EXIT ( [pAllVars, this]() { m_pWsrep->StatsFree ( pAllVars ); } );
		for ( auto* pVars = pAllVars; pVars->m_szName; ++pVars )
			fnVisitor ( *pVars );
	}

	std::unique_ptr<Wsrep::Writeset_i> MakeWriteSet() final
	{
		return std::make_unique<Writeset_T<WSREPWRAP>> ( m_pWsrep );
	}


	bool Connect ( const char* szName, const char* szNodes, bool bBootStrap )
	{
		auto eRes = m_pWsrep->Connect ( szName, szNodes, "", bBootStrap );
		// might try to join existed cluster however that took too long to timeout in case no cluster started
		return eRes == Status_e::OK || TlsMsg::Err ( "replication connection failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
	}

	void StartListen ( Wsrep::Receiver_i* pReceiver )
	{
		assert ( pReceiver );
		assert ( !m_pReceiver );
		pReceiver->AddRef();
		m_pReceiver = pReceiver;

		auto sThName = SphSprintf ( "%s_repl", m_sName.cstr() );

		// let's start listening thread with proper provider set
		auto pScheduler = MakeSingleThreadExecutor ( -1, sThName.cstr() );
		Threads::Coro::Go ( [this]() {
			// publish stuff in 'show threads'
			ScopedInfo_T<GaleraInfo_t> _ { new GaleraInfo_t ( m_sName ) };
			ReplicationRecv();
		},
			pScheduler );
	}

	// main recv thread of Galera that handles cluster
	// This is the listening thread. It blocks in recv() call until
	// disconnect from cluster. It will apply and commit writesets through the
	// callbacks defined above.
	void ReplicationRecv()
	{
		assert ( m_pCluster );
		m_pCluster->AddRef();
		m_pCluster->OnRecvStarted();
		sphLogDebugRpl ( "receiver thread started" );
		Status_e eState = m_pWsrep->Recv ( m_pReceiver.Ptr() );
		sphLogDebugRpl ( "receiver done, code %d, %s", (int)eState, GetStatus ( eState ) );
		m_pCluster->OnRecvFinished ( eState != Status_e::CONNECTION_FAIL && eState != Status_e::NODE_FAIL && eState != Status_e::FATAL );
		m_pCluster->Release();
	}
};

using WsrepLoaderFN = int ( * ) ( void* );

struct WsrepLoader_t
{
	std::unique_ptr<CSphDynamicLibrary> m_pLibrary;
	int m_iIfaceVer = -1;
	WsrepLoaderFN m_WsrepLoaderFn = nullptr;
};

WsrepLoader_t TryWsrep();

inline bool LoadWsrep ( void* pWsrep, const WsrepLoader_t& tLoader )
{
	assert ( pWsrep );
	auto iRes = tLoader.m_WsrepLoaderFn ( pWsrep );
	return !!iRes || TlsMsg::Err ( "loading galera failed. Error %d - %s", iRes, strerrorm ( iRes ) );
}

template<typename PROVIDER>
Wsrep::Provider_i* MakeProvider ( WsrepLoader_t tLoader, Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions )
{
	using WSREPWRAP = typename PROVIDER::WSREPWRAP;
	CSphRefcountedPtr<WSREPWRAP> pWsrepWrap { new WSREPWRAP };
	auto iRes = tLoader.m_WsrepLoaderFn ( &pWsrepWrap->m_tWsrep );
	if ( iRes )
	{
		TlsMsg::Err ( "loading galera failed. Error %d - %s", iRes, strerrorm ( iRes ) );
		return nullptr;
	}
	pWsrepWrap->m_pWsrepLib = std::move ( tLoader.m_pLibrary );
	if ( !pWsrepWrap->CheckLoadWsrep() )
		return nullptr;

	auto pProvider = std::make_unique<PROVIDER> ( pCluster, pWsrepWrap );
	if ( !pProvider->Init ( std::move ( sName ), szListenAddr, szIncoming, szPath, szOptions ) )
		return nullptr;
	return pProvider.release();
}

Wsrep::Provider_i* MakeProviderV25 ( WsrepLoader_t tLoader, Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions );

Wsrep::Provider_i* MakeProviderV31 ( WsrepLoader_t tLoader, Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions );