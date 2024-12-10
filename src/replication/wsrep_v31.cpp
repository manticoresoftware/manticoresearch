//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "wsrep_cxx_int.h"

namespace RAW31
{

static const char* INTERFACE_VERSION = "31";

enum class Status_e : int {
	OK = 0,
	WARNING, // minor warning
	TRX_MISSED,
	TRX_FAIL,
	BRUTEFORCE_ABORT,
	SIZE_EXCEEDED,
	CONNECTION_FAIL, // must abort
	NODE_FAIL,		 // must reinit
	FATAL,			 // must abort
	PRECOMMIT_ABORT,
	NOT_IMPL,		 // not implemented
};

inline const char* GetStatus ( Status_e eStatus ) noexcept
{
	switch ( eStatus )
	{
	case Status_e::OK: return "success";
	case Status_e::WARNING: return "warning";
	case Status_e::TRX_MISSED: return "transaction is not known";
	case Status_e::TRX_FAIL: return "transaction aborted, server can continue";
	case Status_e::BRUTEFORCE_ABORT: return "transaction was victim of brute force abort";
	case Status_e::SIZE_EXCEEDED: return "data exceeded maximum supported size";
	case Status_e::CONNECTION_FAIL: return "error in client connection, must abort";
	case Status_e::NODE_FAIL: return "error in node state, must reinit";
	case Status_e::FATAL: return "fatal error, server must abort";
	case Status_e::PRECOMMIT_ABORT: return "transaction was aborted before commencing pre-commit";
	case Status_e::NOT_IMPL: return "feature not implemented";
	default: return strerror ( static_cast<int> ( eStatus ) );
	}
}

// types from wsrep internals
struct Wsrep_t;
struct NodeInfo_t;

// init/deinit
using FnInit = Status_e ( * ) ( Wsrep_t*, const InitArgs_t* );
using FnFree = void ( * ) ( Wsrep_t* );
using FnCapabilities = uint64_t ( * ) ( Wsrep_t* );

// options
using FnOptionsSet = Status_e ( * ) ( Wsrep_t*, const char* );
using FnOptionsGet = char* (*)( Wsrep_t* );

// connect
using FnConnect = Status_e ( * ) ( Wsrep_t*, const char*, const char*, const char*, bool );
using FnDisconnect = Status_e ( * ) ( Wsrep_t* );

// replication
using FnRecv = Status_e ( * ) ( Wsrep_t*, void* );
using FnReplicate = Status_e ( * ) ( Wsrep_t*, uint64_t, TxHandle_t*, DWORD, Wsrep::TrxMeta_t* );
using FnPreCommit = Status_e ( * ) ( Wsrep_t*, uint64_t, TxHandle_t*, DWORD, Wsrep::TrxMeta_t* );
using FnReplicatePreCommit = Status_e ( * ) ( Wsrep_t*, uint64_t, TxHandle_t*, DWORD, Wsrep::TrxMeta_t* );
using FnInterimCommit = Status_e ( * ) ( Wsrep_t*, TxHandle_t* );
using FnPostCommit = Status_e ( * ) ( Wsrep_t*, TxHandle_t* );
using FnApplierPreCommit = Status_e ( * ) ( Wsrep_t*, void* );
using FnApplierInterimCommit = Status_e ( * ) ( Wsrep_t*, void* );
using FnApplierPostCommit = Status_e ( * ) ( Wsrep_t*, void* );
using FnPostRollback = Status_e ( * ) ( Wsrep_t*, TxHandle_t* );
using FnReplayTrx = Status_e ( * ) ( Wsrep_t*, TxHandle_t*, void* );
using FnAbortPreCommit = Status_e ( * ) ( Wsrep_t*, int64_t, uint64_t );
using FnAppendKey = Status_e ( * ) ( Wsrep_t*, TxHandle_t*, const Key_t*, uint64_t, KeyType_e, bool );
using FnAppendData = Status_e ( * ) ( Wsrep_t*, TxHandle_t*, const Buf_t*, uint64_t, DataType_e, bool );
using FnCasualRead = Status_e ( * ) ( Wsrep_t*, Wsrep::GlobalTid_t* );
using FnFreeConnection = Status_e ( * ) ( Wsrep_t*, uint64_t );

// total order
using FnToExecuteStart = Status_e ( * ) ( Wsrep_t*, uint64_t, const Key_t*, uint64_t, const Buf_t*, uint64_t, Wsrep::TrxMeta_t* );
using FnToExecuteEnd = Status_e ( * ) ( Wsrep_t*, uint64_t );

// preordered
using FnPreorderedCollect = Status_e ( * ) ( Wsrep_t*, PoHandle_t*, const Buf_t*, uint64_t, bool );
using FnPreorderedCommit = Status_e ( * ) ( Wsrep_t*, PoHandle_t*, const Wsrep::UUID_t*, DWORD, int, bool );

// sst
using FnSstSent = Status_e ( * ) ( Wsrep_t*, const Wsrep::GlobalTid_t*, int );
using FnSstReceived = Status_e ( * ) ( Wsrep_t*, const Wsrep::GlobalTid_t*, const void*, uint64_t, int );
using FnSnapshot = Status_e ( * ) ( Wsrep_t*, const void*, uint64_t, const char* );

// stat var
using FnStatsGet = Wsrep::StatsVars_t* (*)( Wsrep_t* );
using FnStatsFree = void ( * ) ( Wsrep_t*, const Wsrep::StatsVars_t* );
using FnStatsReset = void ( * ) ( Wsrep_t* );

// misc
using FnFetchPfsInfo = void ( * ) ( Wsrep_t*, NodeInfo_t*, DWORD );
using FnPause = int64_t ( * ) ( Wsrep_t* ); // returns SeqNo
using FnResume = Status_e ( * ) ( Wsrep_t* );
using FnDesync = Status_e ( * ) ( Wsrep_t* );
using FnResync = Status_e ( * ) ( Wsrep_t* );

// lock/unlock
using FnLock = Status_e ( * ) ( Wsrep_t*, const char*, bool, uint64_t, int64_t );
using FnUnlock = Status_e ( * ) ( Wsrep_t*, const char*, uint64_t );
using FnIsLocked = bool ( * ) ( Wsrep_t*, const char*, uint64_t*, Wsrep::UUID_t* );

struct Wsrep_t
{
	const char* m_szInterfaceVersion;

	FnInit m_fnInit;
	FnCapabilities m_fnCapabilities;
	FnOptionsSet m_fnOptionsSet;
	FnOptionsGet m_fnOptionsGet;
	FnConnect m_fnConnect;
	FnDisconnect m_fnDisconnect;
	FnRecv m_fnRecv;
	FnReplicate m_fnReplicate;
	FnPreCommit m_fnPreCommit;
	FnReplicatePreCommit m_fnReplicatePreCommit;
	FnInterimCommit m_fnInterimCommit;
	FnPostCommit m_fnPostCommit;
	FnApplierPreCommit m_fnApplierPreCommit;
	FnApplierInterimCommit m_fnApplierInterimCommit;
	FnApplierPostCommit m_fnApplierPostCommit;
	FnPostRollback m_fnPostRollback;
	FnReplayTrx m_fnReplayTrx;
	FnAbortPreCommit m_fnAbortPreCommit;
	FnAppendKey m_fnAppendKey;
	FnAppendData m_fnAppendData;
	FnCasualRead m_fnCasualRead;
	FnFreeConnection m_fnFreeConnection;
	FnToExecuteStart m_fnToExecuteStart;
	FnToExecuteEnd m_fnToExecuteEnd;
	FnPreorderedCollect m_fnPreorderedCollect;
	FnPreorderedCommit m_fnPreorderedCommit;
	FnSstSent m_fnSstSent;
	FnSstReceived m_fnSstReceived;
	FnSnapshot m_fnSnapshot;
	FnStatsGet m_fnStatsGet;
	FnStatsFree m_fnStatsFree;
	FnStatsReset m_fnStatsReset;
	FnFetchPfsInfo m_fnFetchPfsInfo;
	FnPause m_fnPause;
	FnResume m_fnResume;
	FnDesync m_fnDesync;
	FnResync m_fnResync;
	FnLock m_fnLock;
	FnUnlock m_fnUnlock;
	FnIsLocked m_fnIsLocked;

	const char* m_szName;
	const char* m_szVersion;
	const char* m_szVendor;

	FnFree m_fnFree;

	void *m_pDlh, *m_pCtx;
};

// boring wrapper of everything in Raw::Wsrep_c, refcounted
struct WrappedWsrep_t final : public ISphRefcountedMT
{
	Wsrep_t m_tWsrep;
	std::unique_ptr<CSphDynamicLibrary> m_pWsrepLib;

	Status_e Init ( const InitArgs_t* pArgs )
	{
		if ( !m_tWsrep.m_fnInit )
			return Status_e::NOT_IMPL;
		return m_tWsrep.m_fnInit ( &m_tWsrep, pArgs );
	}

	uint64_t Capabilities()
	{
		assert ( m_tWsrep.m_fnCapabilities );
		return m_tWsrep.m_fnCapabilities ( &m_tWsrep );
	}

	Status_e OptionsSet ( const char* szConf )
	{
		assert ( m_tWsrep.m_fnOptionsSet );
		return m_tWsrep.m_fnOptionsSet ( &m_tWsrep, szConf );
	}

	char* OptionsGet()
	{
		assert ( m_tWsrep.m_fnOptionsGet );
		return m_tWsrep.m_fnOptionsGet ( &m_tWsrep );
	}

	Status_e Connect ( const char* szCluster, const char* szClusterUrl, const char* szStateDonor, bool bBootstrap )
	{
		assert ( m_tWsrep.m_fnConnect );
		return m_tWsrep.m_fnConnect ( &m_tWsrep, szCluster, szClusterUrl, szStateDonor, bBootstrap );
	}

	Status_e Disconnect()
	{
		assert ( m_tWsrep.m_fnDisconnect );
		return m_tWsrep.m_fnDisconnect ( &m_tWsrep );
	}

	Status_e Recv ( void* pRecvCtx )
	{
		assert ( m_tWsrep.m_fnRecv );
		return m_tWsrep.m_fnRecv ( &m_tWsrep, pRecvCtx );
	}

	Status_e Replicate ( uint64_t uConnID, TxHandle_t* pHandle, DWORD uFlags, Wsrep::TrxMeta_t* pMeta )
	{
		assert ( m_tWsrep.m_fnReplicate );
		return m_tWsrep.m_fnReplicate ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e PreCommit ( uint64_t uConnID, TxHandle_t* pHandle, DWORD uFlags, Wsrep::TrxMeta_t* pMeta )
	{
		assert ( m_tWsrep.m_fnPreCommit );
		return m_tWsrep.m_fnPreCommit ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e ReplicatePreCommit ( uint64_t uConnID, TxHandle_t* pHandle, DWORD uFlags, Wsrep::TrxMeta_t* pMeta )
	{
		assert ( m_tWsrep.m_fnReplicatePreCommit );
		return m_tWsrep.m_fnReplicatePreCommit ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e InterimCommit ( TxHandle_t* pHandle )
	{
		assert ( m_tWsrep.m_fnInterimCommit );
		return m_tWsrep.m_fnInterimCommit ( &m_tWsrep, pHandle );
	}

	Status_e PostCommit ( TxHandle_t* pHandle )
	{
		assert ( m_tWsrep.m_fnPostCommit );
		return m_tWsrep.m_fnPostCommit ( &m_tWsrep, pHandle );
	}

	Status_e ApplierPreCommit ( void* pTrxHandle )
	{
		assert ( m_tWsrep.m_fnApplierPreCommit );
		return m_tWsrep.m_fnApplierPreCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e ApplierInterimCommit ( void* pTrxHandle )
	{
		assert ( m_tWsrep.m_fnApplierInterimCommit );
		return m_tWsrep.m_fnApplierInterimCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e ApplierPostCommit ( void* pTrxHandle )
	{
		assert ( m_tWsrep.m_fnApplierPostCommit );
		return m_tWsrep.m_fnApplierPostCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e PostRollback ( TxHandle_t* pHandle )
	{
		assert ( m_tWsrep.m_fnPostRollback );
		return m_tWsrep.m_fnPostRollback ( &m_tWsrep, pHandle );
	}

	Status_e ReplayTrx ( TxHandle_t* pHandle, void* pTrxCtx )
	{
		assert ( m_tWsrep.m_fnReplayTrx );
		return m_tWsrep.m_fnReplayTrx ( &m_tWsrep, pHandle, pTrxCtx );
	}

	Status_e AbortPreCommit ( int64_t iSeqNo, uint64_t tVictimTrx )
	{
		assert ( m_tWsrep.m_fnAbortPreCommit );
		return m_tWsrep.m_fnAbortPreCommit ( &m_tWsrep, iSeqNo, tVictimTrx );
	}

	Status_e AppendKey ( TxHandle_t* pHandle, const Key_t* pKeys, uint64_t iCount, KeyType_e eKeyType, bool bCopy )
	{
		assert ( m_tWsrep.m_fnAppendKey );
		return m_tWsrep.m_fnAppendKey ( &m_tWsrep, pHandle, pKeys, iCount, eKeyType, bCopy );
	}

	Status_e AppendData ( TxHandle_t* pHandle, const Buf_t* pData, uint64_t iCount, DataType_e eDatatype, bool bCopy )
	{
		assert ( m_tWsrep.m_fnAppendData );
		return m_tWsrep.m_fnAppendData ( &m_tWsrep, pHandle, pData, iCount, eDatatype, bCopy );
	}

	Status_e CausalRead ( Wsrep::GlobalTid_t* pGtid )
	{
		assert ( m_tWsrep.m_fnCasualRead );
		return m_tWsrep.m_fnCasualRead ( &m_tWsrep, pGtid );
	}

	Status_e FreeConnection ( uint64_t uConnID )
	{
		assert ( m_tWsrep.m_fnFreeConnection );
		return m_tWsrep.m_fnFreeConnection ( &m_tWsrep, uConnID );
	}

	Status_e ToExecuteStart ( uint64_t uConnID, const Key_t* pKeys, uint64_t NKeys, const Buf_t* pAction, uint64_t uCount, Wsrep::TrxMeta_t* pMeta )
	{
		assert ( m_tWsrep.m_fnToExecuteStart );
		return m_tWsrep.m_fnToExecuteStart ( &m_tWsrep, uConnID, pKeys, NKeys, pAction, uCount, pMeta );
	}

	Status_e ToExecuteEnd ( uint64_t uConnID )
	{
		assert ( m_tWsrep.m_fnToExecuteEnd );
		return m_tWsrep.m_fnToExecuteEnd ( &m_tWsrep, uConnID );
	}

	Status_e PreorderedCollect ( PoHandle_t* pHandle, const Buf_t* pData, uint64_t uCount, bool bCopy )
	{
		assert ( m_tWsrep.m_fnPreorderedCollect );
		return m_tWsrep.m_fnPreorderedCollect ( &m_tWsrep, pHandle, pData, uCount, bCopy );
	}

	Status_e PreorderedCommit ( PoHandle_t* pHandle, const Wsrep::UUID_t* pSourceId, DWORD uFlags, int iRange, bool bCommit )
	{
		assert ( m_tWsrep.m_fnPreorderedCommit );
		return m_tWsrep.m_fnPreorderedCommit ( &m_tWsrep, pHandle, pSourceId, uFlags, iRange, bCommit );
	}

	Status_e SstSent ( const Wsrep::GlobalTid_t* state_id, int iCode )
	{
		assert ( m_tWsrep.m_fnSstSent );
		return m_tWsrep.m_fnSstSent ( &m_tWsrep, state_id, iCode );
	}

	Status_e SstReceived ( const Wsrep::GlobalTid_t* pStateID, const void* pState, uint64_t uLen, int iCode )
	{
		assert ( m_tWsrep.m_fnSstReceived );
		return m_tWsrep.m_fnSstReceived ( &m_tWsrep, pStateID, pState, uLen, iCode );
	}

	Status_e Snapshot ( const void* pMsg, uint64_t uLen, const char* szDonorSpec )
	{
		assert ( m_tWsrep.m_fnSnapshot );
		return m_tWsrep.m_fnSnapshot ( &m_tWsrep, pMsg, uLen, szDonorSpec );
	}

	Wsrep::StatsVars_t* StatsGet()
	{
		assert ( m_tWsrep.m_fnStatsGet );
		return m_tWsrep.m_fnStatsGet ( &m_tWsrep );
	}

	void StatsFree ( Wsrep::StatsVars_t* pVars )
	{
		assert ( m_tWsrep.m_fnStatsFree );
		m_tWsrep.m_fnStatsFree ( &m_tWsrep, pVars );
	}

	void StatsReset()
	{
		assert ( m_tWsrep.m_fnStatsReset );
		m_tWsrep.m_fnStatsReset ( &m_tWsrep );
	}

	void FetchPfsInfo ( NodeInfo_t* pNodes, DWORD uSize )
	{
		assert ( m_tWsrep.m_fnFetchPfsInfo );
		m_tWsrep.m_fnFetchPfsInfo ( &m_tWsrep, pNodes, uSize );
	}

	int64_t Pause()
	{
		assert ( m_tWsrep.m_fnPause );
		return m_tWsrep.m_fnPause ( &m_tWsrep );
	}

	Status_e Resume()
	{
		assert ( m_tWsrep.m_fnResume );
		return m_tWsrep.m_fnResume ( &m_tWsrep );
	}

	Status_e Desync()
	{
		assert ( m_tWsrep.m_fnDesync );
		return m_tWsrep.m_fnDesync ( &m_tWsrep );
	}

	Status_e Resync()
	{
		assert ( m_tWsrep.m_fnResync );
		return m_tWsrep.m_fnResync ( &m_tWsrep );
	}

	Status_e Lock ( const char* szName, bool bShared, uint64_t uOwner, int64_t uOut )
	{
		assert ( m_tWsrep.m_fnLock );
		return m_tWsrep.m_fnLock ( &m_tWsrep, szName, bShared, uOwner, uOut );
	}

	Status_e Unlock ( const char* szName, uint64_t uOwner )
	{
		assert ( m_tWsrep.m_fnUnlock );
		return m_tWsrep.m_fnUnlock ( &m_tWsrep, szName, uOwner );
	}

	bool IsLocked ( const char* szName, uint64_t* pConn, Wsrep::UUID_t* pNode )
	{
		assert ( m_tWsrep.m_fnIsLocked );
		return m_tWsrep.m_fnIsLocked ( &m_tWsrep, szName, pConn, pNode );
	}

public:
	using Status_e_ = Status_e;
	inline static const char* szGetStatus ( Status_e eStatus ) noexcept
	{
		return GetStatus ( eStatus );
	}

public:
	// nothing should be used if LoadWsrep() returned false.
	bool CheckLoadWsrep()
	{
		TlsMsg::ResetErr();
		auto eLogLvl = LogLevel_e::ERROR_;
		AT_SCOPE_EXIT ( [&eLogLvl] { if ( TlsMsg::HasErr() ) WsrepLog ( eLogLvl, TlsMsg::szError()); TlsMsg::ResetErr(); } );
		m_tWsrep.m_pDlh = m_pWsrepLib->GetLib();

		if ( !!strcmp ( INTERFACE_VERSION, m_tWsrep.m_szInterfaceVersion ) )
			return TlsMsg::Err ( "wrong galera interface version. Need %s, got %s", INTERFACE_VERSION, m_tWsrep.m_szInterfaceVersion );

		if ( ( m_tWsrep.m_fnInit || TlsMsg::Err ( "wrong Init" ) )
			 && ( m_tWsrep.m_fnCapabilities || TlsMsg::Err ( "wrong Capabilities" ) )
			 && ( m_tWsrep.m_fnOptionsSet || TlsMsg::Err ( "wrong OptionsSet" ) )
			 && ( m_tWsrep.m_fnOptionsGet || TlsMsg::Err ( "wrong OptionsGet" ) )
			 && ( m_tWsrep.m_fnConnect || TlsMsg::Err ( "wrong Connect" ) )
			 && ( m_tWsrep.m_fnDisconnect || TlsMsg::Err ( "wrong Disconnect" ) )
			 && ( m_tWsrep.m_fnRecv || TlsMsg::Err ( "wrong Recv" ) )
			 && ( m_tWsrep.m_fnReplicate || TlsMsg::Err ( "wrong Replicate" ) )
			 && ( m_tWsrep.m_fnPreCommit || TlsMsg::Err ( "wrong PreCommit" ) )
			 && ( m_tWsrep.m_fnReplicatePreCommit || TlsMsg::Err ( "wrong ReplicatePreCommit" ) )
			 && ( m_tWsrep.m_fnInterimCommit || TlsMsg::Err ( "wrong InterimCommit" ) )
			 && ( m_tWsrep.m_fnPostCommit || TlsMsg::Err ( "wrong PostCommit" ) )
			 && ( m_tWsrep.m_fnApplierPreCommit || TlsMsg::Err ( "wrong ApplierPreCommit" ) )
			 && ( m_tWsrep.m_fnApplierInterimCommit || TlsMsg::Err ( "wrong ApplierInterimCommit" ) )
			 && ( m_tWsrep.m_fnApplierPostCommit || TlsMsg::Err ( "wrong ApplierPostCommit" ) )
			 && ( m_tWsrep.m_fnPostRollback || TlsMsg::Err ( "wrong PostRollback" ) )
			 && ( m_tWsrep.m_fnReplayTrx || TlsMsg::Err ( "wrong ReplayTrx" ) )
			 && ( m_tWsrep.m_fnAbortPreCommit || TlsMsg::Err ( "wrong AbortPreCommit" ) )
			 && ( m_tWsrep.m_fnAppendKey || TlsMsg::Err ( "wrong AppendKey" ) )
			 && ( m_tWsrep.m_fnAppendData || TlsMsg::Err ( "wrong AppendData" ) )
			 && ( m_tWsrep.m_fnCasualRead || TlsMsg::Err ( "wrong CasualRead" ) )
			 && ( m_tWsrep.m_fnFreeConnection || TlsMsg::Err ( "wrong FreeConnection" ) )
			 && ( m_tWsrep.m_fnToExecuteStart || TlsMsg::Err ( "wrong ToExecuteStart" ) )
			 && ( m_tWsrep.m_fnToExecuteEnd || TlsMsg::Err ( "wrong ToExecuteEnd" ) )
			 && ( m_tWsrep.m_fnPreorderedCollect || TlsMsg::Err ( "wrong PreorderedCollect" ) )
			 && ( m_tWsrep.m_fnPreorderedCommit || TlsMsg::Err ( "wrong PreorderedCommit" ) )
			 && ( m_tWsrep.m_fnSstSent || TlsMsg::Err ( "wrong SstSent" ) )
			 && ( m_tWsrep.m_fnSstReceived || TlsMsg::Err ( "wrong SstReceived" ) )
			 && ( m_tWsrep.m_fnSnapshot || TlsMsg::Err ( "wrong Snapshot" ) )
			 && ( m_tWsrep.m_fnStatsGet || TlsMsg::Err ( "wrong StatsGet" ) )
			 && ( m_tWsrep.m_fnStatsFree || TlsMsg::Err ( "wrong StatsFree" ) )
			 && ( m_tWsrep.m_fnStatsReset || TlsMsg::Err ( "wrong StatsReset" ) )
			 && ( m_tWsrep.m_fnFetchPfsInfo || TlsMsg::Err ( "wrong FetchPfsInfo" ) )
			 && ( m_tWsrep.m_fnPause || TlsMsg::Err ( "wrong Pause" ) )
			 && ( m_tWsrep.m_fnResume || TlsMsg::Err ( "wrong Resume" ) )
			 && ( m_tWsrep.m_fnDesync || TlsMsg::Err ( "wrong Desync" ) )
			 && ( m_tWsrep.m_fnResync || TlsMsg::Err ( "wrong Resync" ) )
			 && ( m_tWsrep.m_fnLock || TlsMsg::Err ( "wrong Lock" ) )
			 && ( m_tWsrep.m_fnUnlock || TlsMsg::Err ( "wrong Unlock" ) )
			 && ( m_tWsrep.m_fnIsLocked || TlsMsg::Err ( "wrong IsLocked" ) ) )
		{
			eLogLvl = LogLevel_e::INFO;
			TlsMsg::Err() << m_tWsrep.m_szName << " " << m_tWsrep.m_szVersion << " by " << m_tWsrep.m_szVendor << " loaded ok.";
			return true;
		}
		return false;
	}

protected:
	~WrappedWsrep_t() final
	{
		if ( !m_tWsrep.m_fnFree )
			return;

		m_tWsrep.m_fnFree ( &m_tWsrep );
		m_tWsrep.m_fnFree = nullptr;
	}
};

enum class PfsType_e { UNKNOWN, MUTEX, CONDVAR, THREAD, FILE };
enum class PfsOps_e { UNKNOWN, INIT, DESTROY, LOCK, UNLOCK, WAIT, TIMEDWAIT, SIGNAL, BROADCAST, CREATE, OPEN, CLOSE, DELETE_ }; // named this way because of macro conflict with winnt.h
enum class PfsTag_e : int;

class Provider_c final : public Provider_T<WrappedWsrep_t>, public Wsrep::Applier_i
{
	using BASE = Provider_T<WrappedWsrep_t>;
	static void ReplicationAbort()
	{
		sphWarning ( "abort from replication provider" );
	}

	// callback for Galera pfs_instr_cb there all mutex \ threads \ events should be implemented.
	static void Instr_fn ( PfsType_e eType, PfsOps_e eOps, PfsTag_e eTag, void** ppValue, void** ppAliedValue, const void* pTs )
	{
		switch ( eType )
		{
		case PfsType_e::THREAD:
		case PfsType_e::FILE:
		default:
			return;

#if !_WIN32
		case PfsType_e::MUTEX:
			{
				switch ( eOps )
				{
				case PfsOps_e::INIT:
					{
						auto* pMutex = new pthread_mutex_t;
						pthread_mutex_init ( pMutex, nullptr );
						*ppValue = pMutex;
					}
					break;

				case PfsOps_e::DESTROY:
					{
						auto* pMutex = (pthread_mutex_t*)( *ppValue );
						assert ( pMutex );
						pthread_mutex_destroy ( pMutex );
						delete ( pMutex );
						*ppValue = nullptr;
					}
					break;

				case PfsOps_e::LOCK:
					{
						auto* pMutex = (pthread_mutex_t*)( *ppValue );
						assert ( pMutex );
						pthread_mutex_lock ( pMutex );
					}
					break;

				case PfsOps_e::UNLOCK:
					{
						auto* pMutex = (pthread_mutex_t*)( *ppValue );
						assert ( pMutex );
						pthread_mutex_unlock ( pMutex );
					}
					break;

				default:
					assert ( 0 );
					break;
				}
			}
		case PfsType_e::CONDVAR:
			{
				switch ( eOps )
				{
				case PfsOps_e::INIT:
					{
						auto* pCond = new pthread_cond_t;
						pthread_cond_init ( pCond, nullptr );
						*ppValue = pCond;
					}
					break;

				case PfsOps_e::DESTROY:
					{
						auto* pCond = (pthread_cond_t*)( *ppValue );
						assert ( pCond );
						pthread_cond_destroy ( pCond );
						delete ( pCond );
						*ppValue = nullptr;
					}
					break;

				case PfsOps_e::WAIT:
					{
						auto* pCond = (pthread_cond_t*)( *ppValue );
						auto* pMutex = (pthread_mutex_t*)( *ppAliedValue );
						assert ( pCond && pMutex );
						pthread_cond_wait ( pCond, pMutex );
					}
					break;

				case PfsOps_e::TIMEDWAIT:
					{
						auto* pCond = (pthread_cond_t*)( *ppValue );
						auto* pMutex = (pthread_mutex_t*)( *ppAliedValue );
						const auto* wtime = (const timespec*)pTs;
						assert ( pCond && pMutex );
						pthread_cond_timedwait ( pCond, pMutex, wtime );
					}
					break;

				case PfsOps_e::SIGNAL:
					{
						auto* pCond = (pthread_cond_t*)( *ppValue );
						assert ( pCond );
						pthread_cond_signal ( pCond );
					}
					break;

				case PfsOps_e::BROADCAST:
					{
						auto* pCond = (pthread_cond_t*)( *ppValue );
						assert ( pCond );
						pthread_cond_broadcast ( pCond );
					}
					break;

				default:
					assert ( 0 );
					break;
				}
			}
#endif
		}
	}

public:
	using WSREPWRAP = WrappedWsrep_t;
    Provider_c ( Wsrep::Cluster_i* pCluster, WSREPWRAP* pWsrep )
        : BASE { pCluster, pWsrep }
    { }

	bool Init ( CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions )
	{
		if ( !m_pWsrep )
			return false;

		m_sName = std::move ( sName );

		struct
		{
			void* m_pCtx;
			const char* m_szName;
			const char* m_szAddress;
			const char* m_szIncoming;
			const char* m_szPath;
			const char* m_szOptions;
			int m_iProtoVer;
			const Wsrep::GlobalTid_t* m_pStateID;
			const char* m_sState;
			uint64_t m_uStateLen;

			// callbacks
			void ( *m_fnLoger ) ( LogLevel_e, const char* );
			CbStatus_e ( *m_fnViewChanged ) ( void*, void*, const Wsrep::ViewInfo_t*, const char*, uint64_t, void**, uint64_t* );

			CbStatus_e ( *m_fnApply ) ( void*, const void*, uint64_t, DWORD, const Wsrep::TrxMeta_t* );
			CbStatus_e ( *m_fnCommit ) ( void*, const void*, DWORD, const Wsrep::TrxMeta_t*, bool*, bool );
			CbStatus_e ( *Unordered_fn ) ( void*, const void*, uint64_t );

			CbStatus_e ( *SstDonate_fn ) ( void*, void*, const void*, uint64_t, const Wsrep::GlobalTid_t*, const char*, uint64_t, bool );
			void ( *m_fnSynced ) ( void* );
			void ( *m_fnReplicationAbort )();
			void ( *m_fnInstr ) ( PfsType_e, PfsOps_e, PfsTag_e, void**, void**, const void* );

		} tArgs = {
			m_pCluster, m_sName.cstr(), szListenAddr, szIncoming, szPath, szOptions, 127, &m_tStateID, "", 0
			, WsrepLog
			, ViewChanged_fn // app + recv
			, Apply_fn // recv
			, &BASE::Commit_fn // recv
			, Unordered_fn // recv
			, SstDonate_fn // app + recv
			, Synced_fn // app
			, ReplicationAbort
			, Instr_fn
		};

		auto eRes = m_pWsrep->Init ( &tArgs );
		return eRes == Status_e::OK || TlsMsg::Err ( "replication init failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
	}

	void ApplierPreCommit ( const void* pTrx_handle ) final
	{
		m_pWsrep->ApplierPreCommit ( const_cast<void*> ( pTrx_handle ) );
	}

	void ApplierInterimPostCommit ( const void* pTrx_handle ) final
	{
		m_pWsrep->ApplierInterimCommit ( const_cast<void*> ( pTrx_handle ) );
		m_pWsrep->ApplierPostCommit ( const_cast<void*> ( pTrx_handle ) );
	}

	Wsrep::Applier_i* GetApplier() final
	{
		return this;
	}
};

} // namespace RAW31

template<>
void Writeset_T<RAW31::WrappedWsrep_t>::InterimCommit()
{
	m_eLastRes = m_pWsrep->InterimCommit ( &m_tHnd );
	CheckResult ( "InterimCommit" );
	m_bNeedPostRollBack = false;
}

template<>
[[nodiscard]] bool Writeset_T<RAW31::WrappedWsrep_t>::Replicate()
{
	m_eLastRes = m_pWsrep->Replicate ( m_uConnId, &m_tHnd, DwFlags_t::COMMIT, &m_tMeta );
	return CheckResult ( "Replicate", true );
}


Wsrep::Provider_i* MakeProviderV31 ( WsrepLoader_t tLoader, Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions )
{
	return MakeProvider<RAW31::Provider_c> ( std::move ( tLoader ), pCluster, std::move ( sName ), szListenAddr, szIncoming, szPath, szOptions );
}