//
// Copyright (c) 2019-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "wsrep_cxx.h"

#include "std/iterations.h"
#include "coroutine.h"
#include "task_info.h"

#include <cerrno>

static const char* INTERFACE_VERSION = "31";

static const char* g_dReplicatorPatterns[] = {
	"Could not open state file for reading:",
	"No persistent state found. Bootstrapping with default state",
	"Fail to access the file (" };

static bool CheckNoWarning ( const char* szMsg )
{
	if ( !szMsg || !szMsg[0] )
		return false;

	for ( const char* sPattern : g_dReplicatorPatterns )
	{
		if ( strncmp ( szMsg, sPattern, strlen ( sPattern ) ) == 0 )
			return true;
	}
	return false;
}

enum class LogLevel_e {
	FATAL,
	ERROR_, // named this way because of macro conflict with winnt.h
	WARN,
	INFO,
	DEBUG
};

// callback for Galera logger_cb to log messages and errors
static void WsrepLog ( LogLevel_e eLevel, const char* szMsg )
{
	using namespace Wsrep;

	// in normal flow need to skip certain messages from Galera but keep message in debug replication verbosity level
	// don't want to patch Galera source code
	if ( g_eLogLevel < SPH_LOG_RPL_DEBUG && eLevel == LogLevel_e::WARN && CheckNoWarning ( szMsg ) )
		return;

	ESphLogLevel eLevelDst = SPH_LOG_INFO;
	switch ( eLevel )
	{
	case LogLevel_e::FATAL:
	case LogLevel_e::ERROR_:
		eLevelDst = SPH_LOG_FATAL;
		break;
	case LogLevel_e::WARN:
		eLevelDst = SPH_LOG_WARNING;
		break;
	case LogLevel_e::INFO:
	case LogLevel_e::DEBUG:
	default:
		eLevelDst = SPH_LOG_RPL_DEBUG;
	}
	sphLogf ( eLevelDst, "%s", szMsg );
}

enum class Status_e {
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
	NOT_IMPL, // not implemented
};

inline const char* GetStatus ( Status_e eStatus ) noexcept
{
	using namespace Wsrep;
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

namespace Raw
{
struct InitArgs_t;
struct TxHandle_t;
struct Key_t;
struct Buf_t;
struct PoHandle_t;
struct NodeInfo_t;
enum class KeyType_e;
enum class DataType_e;

struct DwFlags_t
{
	static constexpr DWORD COMMIT = 1ULL;
	static constexpr DWORD ROLLBACK = 2ULL;
	static constexpr DWORD ISOLATION = 4ULL;
	static constexpr DWORD PA_UNSAFE = 8ULL;
	static constexpr DWORD COMMUTATIVE = 16ULL;
	static constexpr DWORD NATIVE = 32ULL;
};

// types from wsrep internals
struct Wsrep_t;

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
	const char * m_szInterfaceVersion = nullptr;

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

	const char* m_szName = nullptr;
	const char* m_szVersion = nullptr;
	const char* m_szVendor = nullptr;

	FnFree m_fnFree;

	void *m_pDlh, *m_pCtx;
};

// boring wrapper of everything in Raw::Wsrep_c, refcounted
class Wsrep_c final : public ISphRefcountedMT
{
	Raw::Wsrep_t m_tWsrep;
	using WsrepLoaderFN = int ( * ) ( Wsrep_t* );
	CSphDynamicLibrary m_tWsrepLib;
	bool m_bValid = false;

public:
	Wsrep_c ( const Wsrep_c& ) = delete;
	Wsrep_c ( Wsrep_c&& ) = delete;
	Wsrep_c& operator= ( const Wsrep_c& ) = delete;
	Wsrep_c& operator= ( Wsrep_c&& ) = delete;

	Status_e Init ( const Raw::InitArgs_t * pArgs )
	{
		if ( !m_bValid )
			return Status_e::NOT_IMPL;
		assert ( m_bValid );
		return m_tWsrep.m_fnInit ( &m_tWsrep, pArgs );
	}

	uint64_t Capabilities ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnCapabilities ( &m_tWsrep );
	}

	Status_e OptionsSet ( const char * szConf )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnOptionsSet ( &m_tWsrep, szConf );
	}

	char * OptionsGet ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnOptionsGet ( &m_tWsrep );
	}

	Status_e Connect ( const char * szCluster, const char * szClusterUrl, const char * szStateDonor, bool bBootstrap )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnConnect ( &m_tWsrep, szCluster, szClusterUrl, szStateDonor, bBootstrap );
	}

	Status_e Disconnect ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnDisconnect ( &m_tWsrep );
	}

	Status_e Recv ( void * pRecvCtx )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnRecv ( &m_tWsrep, pRecvCtx );
	}

	Status_e Replicate ( uint64_t uConnID, TxHandle_t * pHandle, DWORD uFlags, Wsrep::TrxMeta_t * pMeta )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnReplicate ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e PreCommit ( uint64_t uConnID, TxHandle_t * pHandle, DWORD uFlags, Wsrep::TrxMeta_t * pMeta )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPreCommit ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e ReplicatePreCommit ( uint64_t uConnID, TxHandle_t * pHandle, DWORD uFlags, Wsrep::TrxMeta_t * pMeta )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnReplicatePreCommit ( &m_tWsrep, uConnID, pHandle, uFlags, pMeta );
	}

	Status_e InterimCommit ( TxHandle_t * pHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnInterimCommit ( &m_tWsrep, pHandle );
	}

	Status_e PostCommit ( TxHandle_t * pHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPostCommit ( &m_tWsrep, pHandle );
	}

	Status_e ApplierPreCommit ( void * pTrxHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnApplierPreCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e ApplierInterimCommit ( void * pTrxHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnApplierInterimCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e ApplierPostCommit ( void * pTrxHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnApplierPostCommit ( &m_tWsrep, pTrxHandle );
	}

	Status_e PostRollback ( TxHandle_t * pHandle )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPostRollback ( &m_tWsrep, pHandle );
	}

	Status_e ReplayTrx ( TxHandle_t * pHandle, void * pTrxCtx )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnReplayTrx ( &m_tWsrep, pHandle, pTrxCtx );
	}

	Status_e AbortPreCommit ( int64_t iSeqNo, uint64_t tVictimTrx )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnAbortPreCommit ( &m_tWsrep, iSeqNo, tVictimTrx );
	}

	Status_e AppendKey ( TxHandle_t * pHandle, const Key_t * pKeys, uint64_t iCount, KeyType_e eKeyType, bool bCopy )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnAppendKey ( &m_tWsrep, pHandle, pKeys, iCount, eKeyType, bCopy );
	}

	Status_e AppendData ( TxHandle_t * pHandle, const Buf_t* pData, uint64_t iCount,
			DataType_e eDatatype, bool bCopy )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnAppendData ( &m_tWsrep, pHandle, pData, iCount, eDatatype, bCopy );
	}

	Status_e CausalRead ( Wsrep::GlobalTid_t * pGtid )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnCasualRead ( &m_tWsrep, pGtid );
	}

	Status_e FreeConnection ( uint64_t uConnID )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnFreeConnection ( &m_tWsrep, uConnID) ;
	}

	Status_e ToExecuteStart ( uint64_t uConnID, const Key_t * pKeys, uint64_t NKeys, const Buf_t * pAction, uint64_t uCount, Wsrep::TrxMeta_t * pMeta )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnToExecuteStart ( &m_tWsrep, uConnID, pKeys, NKeys, pAction, uCount, pMeta );
	}

	Status_e ToExecuteEnd ( uint64_t uConnID )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnToExecuteEnd ( &m_tWsrep, uConnID );
	}

	Status_e PreorderedCollect ( PoHandle_t* pHandle, const Buf_t * pData, uint64_t uCount, bool bCopy )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPreorderedCollect ( &m_tWsrep, pHandle, pData, uCount, bCopy );
	}

	Status_e PreorderedCommit ( PoHandle_t * pHandle, const Wsrep::UUID_t * pSourceId, DWORD uFlags, int iRange, bool bCommit )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPreorderedCommit ( &m_tWsrep, pHandle, pSourceId, uFlags, iRange, bCommit );
	}

	Status_e SstSent ( const Wsrep::GlobalTid_t * state_id, int iCode )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnSstSent ( &m_tWsrep, state_id, iCode );
	}

	Status_e SstReceived ( const Wsrep::GlobalTid_t * pStateID, const void * pState, uint64_t uLen, int iCode )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnSstReceived ( &m_tWsrep, pStateID, pState, uLen, iCode );
	}

	Status_e Snapshot ( const void * pMsg, uint64_t uLen, const char * szDonorSpec )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnSnapshot ( &m_tWsrep, pMsg, uLen, szDonorSpec );
	}

	Wsrep::StatsVars_t * StatsGet ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnStatsGet ( &m_tWsrep );
	}

	void StatsFree ( Wsrep::StatsVars_t* pVars )
	{
		assert ( m_bValid );
		m_tWsrep.m_fnStatsFree ( &m_tWsrep, pVars );
	}

	void StatsReset ()
	{
		assert ( m_bValid );
		m_tWsrep.m_fnStatsReset ( &m_tWsrep );
	}

	void FetchPfsInfo ( NodeInfo_t * pNodes, DWORD uSize )
	{
		assert ( m_bValid );
		m_tWsrep.m_fnFetchPfsInfo ( &m_tWsrep, pNodes, uSize );
	}

	int64_t Pause () // returns SeqNo
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnPause ( &m_tWsrep );
	}

	Status_e Resume ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnResume ( &m_tWsrep );
	}

	Status_e Desync ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnDesync ( &m_tWsrep );
	}

	Status_e Resync ()
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnResync ( &m_tWsrep );
	}

	Status_e Lock ( const char * szName, bool bShared, uint64_t uOwner, int64_t uOut )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnLock ( &m_tWsrep, szName, bShared, uOwner, uOut );
	}

	Status_e Unlock ( const char * szName, uint64_t uOwner )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnUnlock ( &m_tWsrep, szName, uOwner );
	}

	bool IsLocked ( const char * szName, uint64_t * pConn, Wsrep::UUID_t * pNode )
	{
		assert ( m_bValid );
		return m_tWsrep.m_fnIsLocked ( &m_tWsrep, szName, pConn, pNode );
	}

	void Free ()
	{
		assert ( m_bValid );
		m_tWsrep.m_fnFree ( &m_tWsrep );
		m_bValid = false;
	}

	Wsrep_c () : m_tWsrepLib( GET_GALERA_FULLPATH().cstr(), false )
	{
		TlsMsg::ResetErr();
		auto eLogLvl = LogLevel_e::ERROR_;
		AT_SCOPE_EXIT ( [&eLogLvl] { if ( TlsMsg::HasErr() ) WsrepLog ( eLogLvl, TlsMsg::szError()); TlsMsg::ResetErr(); } );

		std::array<const char*, 2> sSyms { "wsrep_loader", "wsrep_interface_version" };
		WsrepLoaderFN fnWsrepLoader = nullptr;
		const char** pszIfaceVer = nullptr;
		std::array<void**, 2> ppSyms { (void**)&fnWsrepLoader, (void**)&pszIfaceVer };
		if ( !m_tWsrepLib.LoadSymbols ( (const char**)sSyms.data(), (void***)ppSyms.data(), 2 ) )
			return;

		if ( !!strcmp ( INTERFACE_VERSION, *pszIfaceVer ) )
		{
			TlsMsg::Err ("wrong galera interface version. Need %s, got %s", INTERFACE_VERSION, *pszIfaceVer );
			return;
		}

		assert ( fnWsrepLoader );
		auto iRes = fnWsrepLoader ( &m_tWsrep );
		if ( iRes )
		{
			TlsMsg::Err ( "loading galera failed. Error %d - %s", iRes, strerrorm ( iRes ) );
			return;
		}

		m_tWsrep.m_pDlh = m_tWsrepLib.GetLib();

		if ( !!strcmp ( INTERFACE_VERSION, m_tWsrep.m_szInterfaceVersion ) )
		{
			TlsMsg::Err ( "wrong galera interface version. Need %s, got %s", INTERFACE_VERSION, m_tWsrep.m_szInterfaceVersion );
			return;
		}

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
			m_bValid = true;
		}
	}

protected:
	~Wsrep_c () final
	{
		if ( m_bValid )
			Free();
	}
};

} // namespace Raw

using WsrepRefPtr_c = CSphRefcountedPtr<Raw::Wsrep_c>;

const char * Wsrep::GetViewStatus ( Wsrep::ViewStatus_e eStatus ) noexcept
{
	using namespace Wsrep;
	switch ( eStatus )
	{
	case ViewStatus_e::PRIMARY:			return "primary";
	case ViewStatus_e::NON_PRIMARY:		return "non-primary";
	case ViewStatus_e::DISCONNECTED:	return "disconnected";
	default:	return "unknown (MAX)";
	}
}

struct Raw::TxHandle_t
{
	uint64_t		m_uTrx = UINT64_MAX;
	void*			m_pCookie = nullptr;
};

struct Raw::Buf_t
{
	const void * 	m_pData;
	uint64_t 		m_uLen;
};

struct Raw::Key_t
{
	const Raw::Buf_t*	m_pParts;
	uint64_t			m_uCount;
};

enum class Raw::KeyType_e
{
	SHARED = 0,
	SEMI,
	EXCLUSIVE
};

enum class Raw::DataType_e
{
	ORDERED = 0,
	UNORDERED,
	ANNOTATION
};

// wrap replication writeset to RAII
class Wsrep::Writeset_c::Impl_c
{
	WsrepRefPtr_c m_pWsrep;
	uint64_t m_uConnId;
	static std::atomic<uint64_t> m_uConnIds;
	Wsrep::TrxMeta_t m_tMeta;
	Raw::TxHandle_t m_tHnd;
	Status_e m_eLastRes = Status_e::OK;
	bool m_bNeedPostRollBack = false; // if we need rollback on exit, not just FreeConnection

	// check return code from Galera calls
	// FIXME!!! handle more status codes properly
	bool CheckResult ( const char* sAt, bool bCritical = false )
	{
		if ( LastOk () )
			return true;

		TlsMsg::Err ( "error at %s, code %d, seqno %l", sAt, m_eLastRes, m_tMeta.m_tGtid.m_iSeqNo );
		sphWarning ( "%s", TlsMsg::szError() );
		if ( bCritical )
			m_bNeedPostRollBack = true;
		return false;
	}

public:
	explicit Impl_c ( Raw::Wsrep_c* pWsrep )
		: m_pWsrep { pWsrep }
		, m_uConnId { m_uConnIds.fetch_add ( 1, std::memory_order_relaxed ) }
	{
		pWsrep->AddRef();
		m_tHnd.m_uTrx = m_uConnId;
	}

	int64_t LastSeqno() const noexcept
	{
		return m_tMeta.m_tGtid.m_iSeqNo;
	}

	bool LastOk() const noexcept
	{
		return m_eLastRes == Status_e::OK;
	}

	bool AppendKeys ( const VecTraits_T<uint64_t>& dBufKeys, bool bSharedKeys )
	{
		auto iKeysCount = dBufKeys.GetLength();

		// set keys wsrep_buf_t ptr and len
		CSphFixedVector<Raw::Buf_t> dBufProxy { iKeysCount };
		CSphFixedVector<Raw::Key_t> dKeys { iKeysCount };
		ARRAY_CONSTFOREACH ( i, dBufKeys )
		{
			dBufProxy[i].m_pData = &dBufKeys[i];
			dBufProxy[i].m_uLen = sizeof ( uint64_t );
			dKeys[i].m_pParts = &dBufProxy[i];
			dKeys[i].m_uCount = 1;
		}

		m_eLastRes = m_pWsrep->AppendKey ( &m_tHnd, dKeys.begin(), iKeysCount, ( bSharedKeys ? Raw::KeyType_e::SHARED : Raw::KeyType_e::EXCLUSIVE ), false );
		return CheckResult ( "AppendKeys" );
	}

	bool AppendData ( const VecTraits_T<BYTE>& dData )
	{
		Raw::Buf_t tQueries { dData.Begin(), (uint64_t)dData.GetLength64() };
		m_eLastRes = m_pWsrep->AppendData ( &m_tHnd, &tQueries, 1, Raw::DataType_e::ORDERED, false );
		return CheckResult ( "AppendData" );
	}

	bool Replicate()
	{
		m_eLastRes = m_pWsrep->Replicate ( m_uConnId, &m_tHnd, Raw::DwFlags_t::COMMIT, &m_tMeta );
		return CheckResult( "Replicate", true );
	}

	bool PreCommit()
	{
		m_eLastRes = m_pWsrep->PreCommit ( m_uConnId, &m_tHnd, Raw::DwFlags_t::COMMIT, &m_tMeta );
		return CheckResult( "PreCommit", true );
	}

	void AbortPreCommit()
	{
		m_eLastRes = m_pWsrep->AbortPreCommit ( WRONG_SEQNO, m_tHnd.m_uTrx );
		CheckResult ( "AbortPreCommit" );
		m_bNeedPostRollBack = true;
	}

	void InterimCommit()
	{
		m_eLastRes = m_pWsrep->InterimCommit ( &m_tHnd );
		CheckResult( "InterimCommit" );
		m_bNeedPostRollBack = false;
	}

	void PostCommit()
	{
		m_eLastRes = m_pWsrep->PostCommit ( &m_tHnd );
		CheckResult( "PostCommit" );
		m_bNeedPostRollBack = false;
	}

	// TOI stuff
	bool ToExecuteStart( const VecTraits_T<uint64_t>& dBufKeys, const VecTraits_T<BYTE>& dData )
	{
		auto iKeysCount = dBufKeys.GetLength();

		// set keys ptr and len
		CSphFixedVector<Raw::Buf_t> dBufProxy( iKeysCount );
		CSphFixedVector<Raw::Key_t> dKeys( iKeysCount );
		for ( int i = 0; i<iKeysCount; ++i )
		{
			dBufProxy[i].m_pData = &dBufKeys[i];
			dBufProxy[i].m_uLen = sizeof ( uint64_t );
			dKeys[i].m_pParts = &dBufProxy[i];
			dKeys[i].m_uCount = 1;
		}

		Raw::Buf_t tQueries { dData.Begin(), (uint64_t)dData.GetLength() };
		m_eLastRes = m_pWsrep->ToExecuteStart ( m_uConnId, dKeys.begin(), iKeysCount, &tQueries, 1, &m_tMeta );
		return CheckResult ( "ToExecuteStart" );
	}

	void ToExecuteEnd()
	{
		m_eLastRes = m_pWsrep->ToExecuteEnd ( m_uConnId );
		CheckResult ( "ToExecuteEnd" );
	}

	~Impl_c ()
	{
		if ( m_bNeedPostRollBack )
		{
			m_pWsrep->PostRollback ( &m_tHnd );
			CheckResult( "PostRollback" );
		}
		m_pWsrep->FreeConnection ( m_uConnId );
	}
};

std::atomic<uint64_t> Wsrep::Writeset_c::Impl_c::m_uConnIds { 1 };

Wsrep::Writeset_c::Writeset_c ( void* pProvider )
	: m_pImpl { new Wsrep::Writeset_c::Impl_c ( (Raw::Wsrep_c*)pProvider ) }
{}

Wsrep::Writeset_c::~Writeset_c() = default;
int64_t Wsrep::Writeset_c::LastSeqno() const noexcept { return m_pImpl->LastSeqno(); }
bool Wsrep::Writeset_c::LastOk() const noexcept { return m_pImpl->LastOk(); }
bool Wsrep::Writeset_c::AppendKeys ( const VecTraits_T<uint64_t>& dBufKeys, bool bSharedKeys ) { return m_pImpl->AppendKeys(dBufKeys, bSharedKeys); }
bool Wsrep::Writeset_c::AppendData ( const VecTraits_T<BYTE>& dData ) { return m_pImpl->AppendData(dData);}
bool Wsrep::Writeset_c::Replicate() { return m_pImpl->Replicate(); }
bool Wsrep::Writeset_c::PreCommit() { return m_pImpl->PreCommit(); }
void Wsrep::Writeset_c::AbortPreCommit() { m_pImpl->AbortPreCommit(); }
void Wsrep::Writeset_c::InterimCommit() { m_pImpl->InterimCommit(); }
void Wsrep::Writeset_c::PostCommit() { m_pImpl->PostCommit(); }
bool Wsrep::Writeset_c::ToExecuteStart ( const VecTraits_T<uint64_t>& dBufKeys, const VecTraits_T<BYTE>& dData ) { return m_pImpl->ToExecuteStart(dBufKeys,dData);}
void Wsrep::Writeset_c::ToExecuteEnd() { m_pImpl->ToExecuteEnd(); }


enum class PfsType_e { UNKNOWN, MUTEX, CONDVAR, THREAD, FILE };
enum class PfsOps_e { UNKNOWN, INIT, DESTROY, LOCK, UNLOCK, WAIT, TIMEDWAIT, SIGNAL, BROADCAST, CREATE, OPEN, CLOSE, DELETE_ }; // named this way because of macro conflict with winnt.h
enum class PfsTag_e : int;

// callback for Galera pfs_instr_cb there all mutex \ threads \ events should be implemented.
void Instr_fn( PfsType_e eType, PfsOps_e eOps, PfsTag_e eTag, void** ppValue, void** ppAliedValue, const void* pTs )
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
			assert( 0 );
			break;
		}
	}
#endif
	}
}


// log debug info about nodes as current nodes views that
static void LogGroupView ( const Wsrep::ViewInfo_t* pView )
{
	if ( g_eLogLevel < SPH_LOG_RPL_DEBUG )
		return;

	using namespace Wsrep;

	sphLogDebugRpl ( "new cluster membership: %d(%d), global seqno: " INT64_FMT ", status %s, gap: %s",
		pView->m_iIdx, pView->m_iNMembers, pView->m_tStateId.m_iSeqNo, GetViewStatus ( pView->m_eStatus ), pView->m_bGap ? "true" : "false" );

	StringBuilder_c sBuf;
	const MemberInfo_t* pBoxes = &pView->m_tMemInfo;
	for ( int i = 0; i < pView->m_iNMembers; ++i )
		sBuf.Appendf ( "\n'%s', '%s' %s", pBoxes[i].m_sName, pBoxes[i].m_sIncoming, ( i == pView->m_iIdx ? "*" : "" ) );

	sphLogDebugRpl ( "%s", sBuf.IsEmpty() ? "no nodes" : sBuf.cstr() );
}

enum class CbStatus_e {
	SUCCESS = 0,
	FAILURE,
};

inline constexpr CbStatus_e Bool2Cb ( bool b ) noexcept
{
	return b ? CbStatus_e::SUCCESS : CbStatus_e::FAILURE;
}

// display incoming string as client name in show threads
struct GaleraInfo_t: public TaskInfo_t
{
	DECLARE_RENDER ( GaleraInfo_t );
	CSphString m_sName; // set once on create, so is th-safe
	explicit GaleraInfo_t ( const CSphString& sName ) : m_sName { sName } {}
};

DEFINE_RENDER ( GaleraInfo_t )
{
	auto& tInfo = *(GaleraInfo_t*)const_cast<void*> ( pSrc );
	dDst.m_sChain << "Repl ";
	dDst.m_sClientName << "wsrep " << tInfo.m_sName.cstr();
}

class Wsrep::Provider_c::Impl_c
{
	WsrepRefPtr_c m_pWsrep;
	Wsrep::Cluster_i* m_pCluster;
	Wsrep::ReceiverRefPtr_c m_pReceiver;
	Wsrep::GlobalTid_t m_tStateID;
	CSphString	m_sName;

private:
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

		bool bCommit = ( ( uFlags & Raw::DwFlags_t::COMMIT ) != 0 );
		bool bIsolated = ( ( uFlags & Raw::DwFlags_t::ISOLATION ) != 0 );
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

	// callback for Galera commit that transaction received and parsed before should be committed or rolled back
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

	// abort callback. We have to do nothing here, just log record
	static void ReplicationAbort()
	{
		sphWarning( "abort from replication provider" );
	}

public:
	explicit Impl_c ( Wsrep::Cluster_i* pCluster )
		: m_pWsrep { new Raw::Wsrep_c }
		, m_pCluster ( pCluster )
	{
		assert ( pCluster );
	}

	void ApplierPreCommit ( const void* pTrx_handle )
	{
		m_pWsrep->ApplierPreCommit ( const_cast<void*> ( pTrx_handle ) );
	}

	void ApplierInterimPostCommit ( const void* pTrx_handle )
	{
		m_pWsrep->ApplierInterimCommit ( const_cast<void*> ( pTrx_handle ) );
		m_pWsrep->ApplierPostCommit ( const_cast<void*> ( pTrx_handle ) );
	}

	void SstReceived ( const Wsrep::GlobalTid_t& tGtid, int iRes )
	{
		m_pWsrep->SstReceived ( &tGtid, nullptr, 0, iRes );
	}

	void SstSent ( const Wsrep::GlobalTid_t& tGtid, bool bOk )
	{
		m_pWsrep->SstSent ( &tGtid, ( bOk ? 0 : -ECANCELED ) );
	}

	void Disconnect()
	{
		m_pWsrep->Disconnect();
	}

	bool OptionsSet ( const CSphString& sName, const CSphString& sValue )
	{
		StringBuilder_c sOpt;
		sOpt << sName << "=" << sValue;

		auto tOk = m_pWsrep->OptionsSet ( sOpt.cstr() );
		return tOk == Status_e::OK || TlsMsg::Err ( GetStatus ( tOk ) );
	}

	bool EnumFindStatsVar ( Wsrep::StatsVarsIteratorFN&& fnIterator )
	{
		auto* pAllVars = m_pWsrep->StatsGet();
		AT_SCOPE_EXIT ( [pAllVars, this]() { m_pWsrep->StatsFree ( pAllVars ); } );
		for ( auto* pVars = pAllVars; pVars->m_szName; ++pVars )
		if ( fnIterator ( *pVars ) )
			return true;
		return false;
	}

	void EnumStatsVars ( Wsrep::StatsVarsVisitorFN&& fnVisitor )
	{
		auto* pAllVars = m_pWsrep->StatsGet();
		AT_SCOPE_EXIT ( [pAllVars, this]() { m_pWsrep->StatsFree ( pAllVars ); } );
		for ( auto* pVars = pAllVars; pVars->m_szName; ++pVars )
		fnVisitor ( *pVars );
	}

	Wsrep::Writeset_c MakeWriteSet()
	{
		return Wsrep::Writeset_c { m_pWsrep };
	}

	bool Init ( CSphString sName, const char* szListenAddr, const char* szIncoming, const char * szPath, const char* szOptions )
	{
		if ( !m_pWsrep )
			return false;

		m_sName = std::move(sName);

		struct {
			void*		m_pCtx;
			const char* m_szName;
			const char* m_szAddress;
			const char* m_szIncoming;
			const char* m_szPath;
			const char* m_szOptions;
			int 		m_iProtoVer;
			const Wsrep::GlobalTid_t* m_pStateID;
			const char*	m_sState;
			uint64_t	m_uStateLen;

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
		} tArgs = { m_pCluster, m_sName.cstr(), szListenAddr, szIncoming, szPath, szOptions, 127, &m_tStateID, "", 0
			, WsrepLog
			, ViewChanged_fn // app + recv
			, Apply_fn // recv
			, Commit_fn // recv
			, Unordered_fn // recv
			, SstDonate_fn // app + recv
			, Synced_fn // app
			, ReplicationAbort
			, Instr_fn
		};

		auto eRes = m_pWsrep->Init ( (Raw::InitArgs_t*)&tArgs );
		return eRes == Status_e::OK || TlsMsg::Err ( "replication init failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
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
	void ReplicationRecv ()
	{
		assert (m_pCluster);
		m_pCluster->AddRef();
		m_pCluster->OnRecvStarted ();
		sphLogDebugRpl ( "receiver thread started" );
		Status_e eState = m_pWsrep->Recv ( m_pReceiver.Ptr ());
		sphLogDebugRpl ( "receiver done, code %d, %s", (int)eState, GetStatus ( eState ));
		m_pCluster->OnRecvFinished ( eState!= Status_e::CONNECTION_FAIL && eState!= Status_e::NODE_FAIL && eState!= Status_e::FATAL );
		m_pCluster->Release ();
	}
	friend class Wsrep::Provider_c;
};

Wsrep::Provider_c::~Provider_c() = default;
void Wsrep::Provider_c::ApplierPreCommit ( const void* pTrx_handle ) { return m_pImpl->ApplierPreCommit ( pTrx_handle ); }
void Wsrep::Provider_c::ApplierInterimPostCommit ( const void* pTrx_handle) { return m_pImpl->ApplierInterimPostCommit ( pTrx_handle ); }
void Wsrep::Provider_c::SstReceived ( const GlobalTid_t& tGtid, int iRes ) { return m_pImpl->SstReceived ( tGtid, iRes ); }
void Wsrep::Provider_c::SstSent ( const GlobalTid_t& tGtid, bool bOk ) { return m_pImpl->SstSent ( tGtid, bOk ); }
void Wsrep::Provider_c::Disconnect() { return m_pImpl->Disconnect(); }
bool Wsrep::Provider_c::OptionsSet ( const CSphString& sName, const CSphString& sValue ) { return m_pImpl->OptionsSet ( sName, sValue ); }
bool Wsrep::Provider_c::EnumFindStatsVar ( Wsrep::StatsVarsIteratorFN && fnIterator ) { return m_pImpl->EnumFindStatsVar ( std::move ( fnIterator) ); }
void Wsrep::Provider_c::EnumStatsVars ( Wsrep::StatsVarsVisitorFN && fnVisitor ) { return m_pImpl->EnumStatsVars ( std::move (fnVisitor) ); }
Wsrep::Writeset_c Wsrep::Provider_c::MakeWriteSet() { return m_pImpl->MakeWriteSet(); }
bool Wsrep::Provider_c::Connect ( const char* szName, const char* szNodes, bool bBootStrap ) { return m_pImpl->Connect ( szName, szNodes, bBootStrap ); }
void Wsrep::Provider_c::StartListen ( Wsrep::Receiver_i* pReceiver ) { m_pImpl->StartListen ( pReceiver ); }

Wsrep::Provider_c* Wsrep::MakeProvider ( Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions )
{
	auto pImpl = std::make_unique<Provider_c::Impl_c> ( pCluster );
	if ( !pImpl->Init ( std::move ( sName ), szListenAddr, szIncoming, szPath, szOptions ) )
		return nullptr;
	auto pRes = new Provider_c;
	pRes->m_pImpl = std::move ( pImpl );
	return pRes;
}

CSphString Wsrep::Uuid2Str ( const Wsrep::UUID_t& tUuid )
{
	auto h = [&tUuid] ( int idx ) { return FixedNum_T<BYTE, 16, 2, 0, '0'> ( tUuid[idx] ); };
	CSphString sResult;
	StringBuilder_c{}.Sprint (h(0),h(1),h(2),h(3),'-',h(4),h(5),'-',h(6),h(7),'-',h(8),h(9),'-',h(10),h(11),h(12),h(13),h(14),h(15)).MoveTo(sResult);
	return sResult;
}

CSphString Wsrep::Gtid2Str ( const Wsrep::GlobalTid_t& tGtid )
{
	CSphString sResult;
	StringBuilder_c {}.Sprint ( Wsrep::Uuid2Str ( tGtid.m_tUuid ), ':', tGtid.m_iSeqNo ).MoveTo ( sResult );
	return sResult;
}

#if __has_include( <charconv>)
template <typename INT>
inline static std::from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, INT& iValue, int iBase=10 )
{
	return std::from_chars ( pBegin, pEnd, iValue, iBase );
}
#else
struct from_chars_result
{
	const char* ptr;
	std::errc ec;
};
inline static from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, int64_t& iValue )
{
	from_chars_result tRes;
	iValue = strtoll ( pBegin, (char**)&tRes.ptr, 10 );
	if ( tRes.ptr < pEnd )
		tRes.ec = std::errc::invalid_argument;
	return tRes;
}

inline static from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, BYTE& uValue, int iBase )
{
	assert ( iBase == 16 );
	from_chars_result tRes { pBegin, std::errc::invalid_argument };

	auto Char2Hex = [] ( BYTE uChar ) -> int {
		switch ( uChar ) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a':
		case 'A': return 10;
		case 'b':
		case 'B': return 11;
		case 'c':
		case 'C': return 12;
		case 'd':
		case 'D': return 13;
		case 'e':
		case 'E': return 14;
		case 'f':
		case 'F': return 15;
		default: return -1;
		}
	};

	auto iVal = Char2Hex ( *tRes.ptr );
	if ( iVal < 0 )
		return tRes;
	uValue = iVal << 4;
	++tRes.ptr;
	iVal = Char2Hex ( *tRes.ptr );
	if ( iVal < 0 )
		return tRes;
	uValue += iVal;
	++tRes.ptr;
	tRes.ec = static_cast<std::errc> ( 0 );
	return tRes;
}
#endif


const char * Str2UuidImpl ( Wsrep::UUID_t& dRes, const CSphString & sUuid )
{
	int iUuidBytes = 0;
	const char* pSrc = sUuid.cstr();
	const char* pDigitEnd = pSrc + 2;
	const char* pEnd = pSrc + sUuid.Length();
	while ( pDigitEnd <= pEnd && iUuidBytes < 16 )
	{
		auto tChars = from_chars_wrap ( pSrc, pDigitEnd, dRes[iUuidBytes++], 16 );
		if ( tChars.ec == std::errc::invalid_argument )
			break;
		pSrc = tChars.ptr + ( iUuidBytes != 16 && tChars.ptr < pEnd && *tChars.ptr == '-' );
		pDigitEnd = pSrc + 2;
	}
	if ( iUuidBytes < 16 )
		dRes = {};
	return pSrc;
}

Wsrep::UUID_t Wsrep::Str2Uuid ( const CSphString & sUuid )
{
	Wsrep::UUID_t dRes;
	Str2UuidImpl ( dRes, sUuid );
	return dRes;
}

Wsrep::GlobalTid_t Wsrep::Str2Gtid ( const CSphString& sGtid )
{
	Wsrep::GlobalTid_t dRes;
	const char* pEnd = sGtid.cstr() + sGtid.Length();
	const char* pSrc = Str2UuidImpl ( dRes.m_tUuid, sGtid );
	if ( pSrc+1 < pEnd && *pSrc == ':' )
	{
		++pSrc;
		auto tChars = from_chars_wrap ( pSrc, pEnd, dRes.m_iSeqNo );
		if ( tChars.ec == std::errc::invalid_argument )
			dRes = {};
	} else dRes = {};
	return dRes;
}