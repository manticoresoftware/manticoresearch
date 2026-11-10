//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchd_shard_bulk.h"

#include "api_reply_stream.h"
#include "auth/auth.h"
#include "daemon/failures_log.h"
#include "searchd_shard.h"
#include "searchdreplication.h"

static const bool LOG_LEVEL_BULK_IMPORT = env_exists ( "MANTICORE_LOG_BULK_IMPORT" ); // also rules the BULK_IMPORT_SHARD lines
#define ShardBulkInfo( ... ) do if ( LOG_LEVEL_BULK_IMPORT ) sphInfo_impl (__VA_ARGS__); while(0)

constexpr int SHARD_BULK_REMOTE_BATCH_BYTES = 256*1024;	// a remote target does not pay a request for every 32 KiB
constexpr int SHARD_BULK_PUBLISHED_NAMES = 20;			// published targets named by the error of a partial commit

static int64_t ShardBulkTimer()
{
	return LOG_LEVEL_BULK_IMPORT ? sphMicroTimer() : 0;
}


// The error of a target can carry the log of its indexer, longer than SphSprintf keeps.
static CSphString ShardBulkError ( const CSphString & sTable, const char * szWhat, const CSphString & sTarget, const CSphString & sError )
{
	StringBuilder_c sOut;
	sOut << "shared bulk_import into shard table '" << sTable << "' " << szWhat << " " << sTarget << ": " << sError;
	CSphString sResult;
	sOut.MoveTo ( sResult );
	return sResult;
}


// The operations of a remote target under one command, as E_CLUSTER under SEARCHD_COMMAND_CLUSTER; a new operation
// raises VER_COMMAND_SHARD_BULK, so an older node refuses it by the version.
enum class ShardBulkOp_e : WORD { START, DATA, COMMIT, CANCEL };


// The data and the commit send heartbeats while they work, as SHARD_WRITE does; the start and the cancel do not wait
// for a child or a chunk.
static bool HasHeartbeat ( ShardBulkOp_e eOp )
{
	return eOp==ShardBulkOp_e::DATA || eOp==ShardBulkOp_e::COMMIT;
}


// The operation, the preamble of SHARD_WRITE (heartbeat interval and the physical table, which the connection names),
// the load id, then what the operation takes.
class ShardBulkRequestBuilder_c final : public RequestBuilder_i
{
public:
	ShardBulkOp_e m_eOp = ShardBulkOp_e::CANCEL;
	int64_t m_iLoadId = 0;
	DWORD m_uHeartbeatIntervalMs = 0;
	Str_t m_tShard;			// start: the shard table, for messages
	Str_t m_tCsv;			// data
	int64_t m_iRows = 0;	// data

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SHARD_BULK, VER_COMMAND_SHARD_BULK );
		tOut.SendWord ( (WORD)m_eOp );
		SendShardWriteRequestPreamble ( tOut, m_uHeartbeatIntervalMs, tAgent.m_tDesc.m_sIndexes );
		tOut.SendUint64 ( m_iLoadId );
		if ( m_eOp==ShardBulkOp_e::START )
			tOut.SendString ( m_tShard );
		else if ( m_eOp==ShardBulkOp_e::DATA )
		{
			tOut.SendUint64 ( m_iRows );
			tOut.SendString ( m_tCsv );
		}
	}
};


// A remote target refuses with an OK reply that carries the reason, as replication commands answer: the agent retry
// then repeats only failures of the transport, and the reason is not replaced by "retries limit exceeded".
struct ShardBulkReply_t final : public iQueryResult
{
	CSphString m_sError;

	void Reset() final { m_sError = ""; }
	[[nodiscard]] bool HasWarnings() const final { return false; }
};


class ShardBulkReplyParser_c final : public ReplyParser_i
{
public:
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		if ( tReq.HasBytes() )
			static_cast<ShardBulkReply_t &> ( *tAgent.m_pResult ).m_sError.SetSprintf ( "remote error: %s", tReq.GetString().cstr() );
		return true;
	}
};


// agent_retry_count of the shard table for the agent retry. START, COMMIT and CANCEL are safe to repeat after any
// failure of the transport, the load id guards them. DATA is not: after a lost reply its rows would go to the pool
// twice, so CallTarget() repeats it only when it never reached the target. The load is pinned to its mirrors, so the
// count is not multiplied by mirrors as a search does.
static int ShardBulkRetries ( const ShardIndex_c & tShard, ShardBulkOp_e eOp )
{
	return eOp==ShardBulkOp_e::DATA ? 0 : tShard.m_iAgentRetryCount;
}


// The connection timed out before the request left: the agent reports this text only in the CONNECTING state, which
// ends with the first byte sent, so the target got nothing.
static bool IsNotConnected ( const AgentConn_t & tAgent )
{
	return !tAgent.m_bSuccess && tAgent.m_sFailure=="connect and query timed out";
}


// Success only with a confirmed reply without a refusal; otherwise the refusal or the failure of the transport.
static bool GetTargetResult ( const AgentConn_t & tAgent, CSphString & sError )
{
	if ( !tAgent.m_bSuccess )
	{
		sError = tAgent.m_sFailure.IsEmpty() ? CSphString ( "no reply" ) : tAgent.m_sFailure;
		return false;
	}

	sError = static_cast<const ShardBulkReply_t &> ( *tAgent.m_pResult ).m_sError;
	return sError.IsEmpty();
}


// One load into a shard table on the node that takes its requests; the CSV goes to the pools of the physical targets.
class ShardBulkLoad_c : public ISphRefcountedMT
{
public:
	const CSphString m_sTable;
	const int64_t m_iLoadId;
	const cDistributedIndexRefPtr_t m_pWrapper; // the shard table object of the first request; the commit fails the load if it was replaced
	const ShardIndex_c & m_tShard;
	CSphVector<int> m_dCsvAttrOrder; // CSV of the shard table schema, which every target has, as SHARD_WRITE assumes

	ShardBulkLoad_c ( const CSphString & sTable, cDistributedIndexRefPtr_t pWrapper );

	SharedBulkResult_e Enter ( CSphString & sError );
	void Leave();
	bool IsCurrentWrapper() const;
	SharedBulkResult_e Start ( CSphString & sError );
	SharedBulkResult_e WaitStarted ( CSphString & sError );
	SharedBulkResult_e Write ( int iRoute, Str_t tCsv, int64_t iRows, CSphString & sError );
	CSphString Fail ( const CSphString & sError, bool bCancelTargets=true );
	bool FailOpen ( const CSphString & sError );
	SharedBulkResult_e Publish ( bool bAdmitted, CSphString & sError );
	int GetBatchBytes ( int iRoute ) const;

private:
	class Call_c;

	Threads::Coro::Mutex_c m_tLock;
	Threads::Coro::ConditionVariable_c m_tChanged;
	CSphFixedVector<AgentDesc_t> m_dAgents; // by route: the mirror of a remote target, pinned for the whole load
	int m_iActive = 1;			// admitted requests; the creating request enters before the load is visible
	bool m_bStarted = false;	// every target runs its pool and takes data
	bool m_bClosing = false;	// a commit owns the targets
	CSphString m_sError;		// first error; a failed load stays until commit or PURGE
	std::atomic<bool> m_bFailed { false }; // m_sError is set and never changes again, so Write() reads it without the lock

	const ShardIndex_c::RouteTarget_t & Route ( int iRoute ) const { return m_tShard.m_dRouteTargets[iRoute]; }
	void FailLocked ( const CSphString & sError, CSphVector<int> & dCancel );
	CSphString DescribeTarget ( int iRoute ) const;
	AgentConn_t * MakeTargetConn ( int iRoute, ShardBulkRequestBuilder_c & tBuilder ) const;
	bool CallTarget ( ShardBulkRequestBuilder_c & tBuilder, int iRoute, CSphString & sError ) const;
	bool WriteTarget ( int iRoute, Str_t tCsv, int64_t iRows, bool & bRequestOnly, CSphString & sError ) const;
	void CancelTarget ( int iRoute ) const;
};

using ShardBulkLoadRefPtr_c = RefCountedRefPtr_T<ShardBulkLoad_c>;
static CSphMutex g_tShardBulkLock; // short sections without yields; DROP TABLE may come outside a coroutine
static SmallStringHash_T<ShardBulkLoadRefPtr_c> g_hShardBulk GUARDED_BY ( g_tShardBulkLock );


static ShardBulkLoadRefPtr_c GetShardBulkLoad ( const CSphString & sTable ) EXCLUDES ( g_tShardBulkLock )
{
	ScopedMutex_t tLock ( g_tShardBulkLock );
	auto * pLoad = g_hShardBulk ( sTable );
	return pLoad ? *pLoad : ShardBulkLoadRefPtr_c {};
}


// Add pNew as the load of sTable where there is none (pOld is nullptr), or remove the load pOld (pNew is nullptr);
// false when the current load of the table is not pOld.
static bool ReplaceShardBulkLoad ( const CSphString & sTable, const ShardBulkLoad_c * pOld, const ShardBulkLoadRefPtr_c & pNew ) EXCLUDES ( g_tShardBulkLock )
{
	ScopedMutex_t tLock ( g_tShardBulkLock );
	auto * pLoad = g_hShardBulk ( sTable );
	if ( ( pLoad ? pLoad->Ptr() : nullptr )!=pOld )
		return false;
	if ( pNew )
		g_hShardBulk.Add ( pNew, sTable );
	else
		g_hShardBulk.Delete ( sTable );
	return true;
}


ShardBulkLoad_c::ShardBulkLoad_c ( const CSphString & sTable, cDistributedIndexRefPtr_t pWrapper )
	: m_sTable ( sTable )
	, m_iLoadId ( UidShort() )
	, m_pWrapper ( std::move ( pWrapper ) )
	, m_tShard ( *AsShard ( m_pWrapper.Ptr() ) )
	, m_dAgents ( m_tShard.m_dRouteTargets.GetLength() )
{
	GetIndexerRtBulkCsvAttrOrder ( m_tShard.m_tSchema, m_dCsvAttrOrder );
	ARRAY_FOREACH ( iRoute, m_dAgents )
		if ( !Route ( iRoute ).IsLocal() ) // the config gives every remote route a non-empty agent
			m_dAgents[iRoute].CloneFrom ( Route ( iRoute ).m_pAgent->ChooseAgent() );
}


SharedBulkResult_e ShardBulkLoad_c::Enter ( CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}
	if ( m_bClosing )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is being published", m_sTable.cstr() );
		return SharedBulkResult_e::CLOSING;
	}
	++m_iActive;
	return SharedBulkResult_e::STAGED;
}


void ShardBulkLoad_c::Leave()
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	--m_iActive;
}


bool ShardBulkLoad_c::IsCurrentWrapper() const
{
	return GetDistr ( m_sTable ).Ptr()==m_pWrapper.Ptr();
}


int ShardBulkLoad_c::GetBatchBytes ( int iRoute ) const
{
	return m_tShard.m_dRouteTargets[iRoute].IsLocal() ? GetSharedBulkBatchBytes() : SHARD_BULK_REMOTE_BATCH_BYTES; // a local target takes batches as a Shared pool does
}


CSphString ShardBulkLoad_c::DescribeTarget ( int iRoute ) const
{
	if ( Route ( iRoute ).IsLocal() )
		return SphSprintf ( "table '%s'", Route ( iRoute ).m_sName.cstr() );
	return SphSprintf ( "remote table '%s' at %s", Route ( iRoute ).m_sName.cstr(), m_dAgents[iRoute].GetMyUrl().cstr() );
}


// A connection to the pinned mirror of a remote target, as ApplyRemoteShardBatch makes it: the timeouts of the shard
// table; it names the physical table of the request. The builder gets the load and the heartbeat.
AgentConn_t * ShardBulkLoad_c::MakeTargetConn ( int iRoute, ShardBulkRequestBuilder_c & tBuilder ) const
{
	const bool bHeartbeat = HasHeartbeat ( tBuilder.m_eOp );
	const int iQueryTimeoutMs = m_tShard.GetAgentQueryTimeoutMs();
	tBuilder.m_iLoadId = m_iLoadId;
	tBuilder.m_uHeartbeatIntervalMs = bHeartbeat ? CalcRemoteHeartbeatIntervalMs ( iQueryTimeoutMs ) : 0;

	auto * pConn = new AgentConn_t;
	pConn->m_tDesc.CloneFrom ( m_dAgents[iRoute] );
	pConn->m_tDesc.m_sIndexes = Route ( iRoute ).m_sName;
	pConn->m_iMyConnectTimeoutMs = m_tShard.GetAgentConnectTimeoutMs();
	pConn->m_iMyQueryTimeoutMs = iQueryTimeoutMs;
	pConn->m_pResult = std::make_unique<ShardBulkReply_t>();
	if ( bHeartbeat )
		pConn->EnableRemoteReplyHeartbeats();
	return pConn;
}


// One call to a remote target: success only with a confirmed reply without a refusal; any other outcome is a failure
// with its original text. A DATA call that never reached the target is repeated up to agent_retry_count times, as the
// agent repeats the other commands.
bool ShardBulkLoad_c::CallTarget ( ShardBulkRequestBuilder_c & tBuilder, int iRoute, CSphString & sError ) const
{
	for ( int iTry = 0; ; ++iTry )
	{
		VecRefPtrsAgentConn_t dAgents;
		dAgents.Add ( MakeTargetConn ( iRoute, tBuilder ) );
		ShardBulkReplyParser_c tParser;
		PerformRemoteTasks ( dAgents, &tBuilder, &tParser, ShardBulkRetries ( m_tShard, tBuilder.m_eOp ) );
		if ( GetTargetResult ( *dAgents[0], sError ) )
			return true;

		if ( tBuilder.m_eOp!=ShardBulkOp_e::DATA || iTry==m_tShard.m_iAgentRetryCount || !IsNotConnected ( *dAgents[0] ) )
			return false;
	}
}


// One operation on every target at once, in the shape of SearchHandler_c::RunSubset: Send() sends it to the remote
// targets, the caller runs the local ones meanwhile, Wait() collects the remote replies. Results are by route.
class ShardBulkLoad_c::Call_c
{
public:
	CSphFixedVector<bool> m_dOk;
	CSphFixedVector<CSphString> m_dErrors;
	CSphVector<int> m_dLocal; // after Send(): the routes the caller runs itself

	Call_c ( const ShardBulkLoad_c & tLoad, ShardBulkOp_e eOp );
	~Call_c() { Wait(); } // the scheduled jobs use m_tBuilder and m_tParser
	void Send();
	void Wait();

private:
	const ShardBulkLoad_c & m_tLoad;
	ShardBulkRequestBuilder_c m_tBuilder;
	ShardBulkReplyParser_c m_tParser;
	VecRefPtrsAgentConn_t m_dAgents;
	CSphVector<int> m_dRemote;
	CSphRefcountedPtr<RemoteAgentsObserver_i> m_pReporter;
};


ShardBulkLoad_c::Call_c::Call_c ( const ShardBulkLoad_c & tLoad, ShardBulkOp_e eOp )
	: m_dOk ( tLoad.m_dAgents.GetLength() )
	, m_dErrors ( tLoad.m_dAgents.GetLength() )
	, m_tLoad ( tLoad )
{
	m_dOk.Fill ( false );
	m_tBuilder.m_eOp = eOp;
	if ( eOp==ShardBulkOp_e::START )
		m_tBuilder.m_tShard = FromStr ( tLoad.m_sTable );
}


void ShardBulkLoad_c::Call_c::Send()
{
	ARRAY_FOREACH ( iRoute, m_tLoad.m_dAgents )
		if ( m_tLoad.Route ( iRoute ).IsLocal() )
			m_dLocal.Add ( iRoute );
		else
		{
			m_dRemote.Add ( iRoute );
			m_dAgents.Add ( m_tLoad.MakeTargetConn ( iRoute, m_tBuilder ) );
		}

	if ( m_dAgents.IsEmpty() )
		return;
	m_pReporter = GetObserver();
	ScheduleDistrJobs ( m_dAgents, &m_tBuilder, &m_tParser, m_pReporter, ShardBulkRetries ( m_tLoad.m_tShard, m_tBuilder.m_eOp ) ); // shard targets are never blackholes, none is dropped
}


void ShardBulkLoad_c::Call_c::Wait()
{
	if ( !m_pReporter )
		return;
	m_pReporter->Finish();
	m_pReporter = nullptr;
	ARRAY_FOREACH ( i, m_dRemote )
		m_dOk[m_dRemote[i]] = GetTargetResult ( *m_dAgents[i], m_dErrors[m_dRemote[i]] );
}


// Start the pools of every target, as a Shared pool starts all its indexers: the creating request runs it once,
// outside the lock, and the requests admitted meanwhile wait in WaitStarted() before their first document. A failed
// start fails the load, and this request cancels what it may have started, since nobody else knows these pools.
SharedBulkResult_e ShardBulkLoad_c::Start ( CSphString & sError )
{
	const int64_t tmStart = ShardBulkTimer();
	Call_c tCall ( *this, ShardBulkOp_e::START );
	tCall.Send(); // START is on its way to every remote target
	for ( int iRoute : tCall.m_dLocal )
	{
		const auto & tRoute = Route ( iRoute );
		tCall.m_dOk[iRoute] = StartShardBulkTarget ( tRoute.m_sName, m_iLoadId, m_sTable, tRoute.m_iLocalIndexId, tRoute.m_iLocalAlterGeneration, tCall.m_dErrors[iRoute] )==SharedBulkResult_e::STAGED;
	}
	tCall.Wait();
	const auto & dOk = tCall.m_dOk;
	const auto & dErrors = tCall.m_dErrors;

	CSphString sFailed;
	ARRAY_FOREACH ( iRoute, dOk )
		if ( !dOk[iRoute] && sFailed.IsEmpty() )
			sFailed = ShardBulkError ( m_sTable, "failed to start", DescribeTarget ( iRoute ), dErrors[iRoute] );
	ShardBulkInfo ( "BULK_IMPORT_SHARD started table=%s load=" INT64_FMT " targets=%d start_ms=%.3f ok=%d", m_sTable.cstr(), m_iLoadId, dOk.GetLength(), ( sphMicroTimer()-tmStart )/1000.0, (int)sFailed.IsEmpty() );

	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		if ( sFailed.IsEmpty() && m_sError.IsEmpty() )
		{
			m_bStarted = true;
			m_tChanged.NotifyAll();
			return SharedBulkResult_e::STAGED;
		}
		CSphVector<int> dNone; // the load is not started, FailLocked() cancels nothing
		if ( !sFailed.IsEmpty() )
			FailLocked ( sFailed, dNone );
		sError = m_sError;
	}

	// the started targets, and the remote ones without a confirmed start
	ARRAY_FOREACH ( iRoute, dOk )
		if ( dOk[iRoute] || !Route ( iRoute ).IsLocal() )
			CancelTarget ( iRoute );
	return sError==sFailed ? SharedBulkResult_e::FAILED : SharedBulkResult_e::FAILED_ALREADY;
}


SharedBulkResult_e ShardBulkLoad_c::WaitStarted ( CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	while ( m_sError.IsEmpty() && !m_bStarted )
	{
		if ( m_tChanged.WaitForMs ( tLock, GetSharedBulkWaitMs() ) )
			continue; // notified

		if ( CheckIndexerRtBulkCancelled ( sError ) )
			return SharedBulkResult_e::FAILED;
	}

	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}
	return SharedBulkResult_e::STAGED;
}


SharedBulkResult_e ShardBulkLoad_c::Write ( int iRoute, Str_t tCsv, int64_t iRows, CSphString & sError )
{
	if ( m_bFailed.load ( std::memory_order_acquire ) )
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}

	// a KILL or a shutdown fails only this request, as in Shared; a remote call would not notice it
	if ( CheckIndexerRtBulkCancelled ( sError ) )
		return SharedBulkResult_e::FAILED;

	CSphString sWriteError;
	bool bRequestOnly = false;
	if ( WriteTarget ( iRoute, tCsv, iRows, bRequestOnly, sWriteError ) )
		return SharedBulkResult_e::STAGED;

	if ( bRequestOnly ) // KILL or shutdown before the write; the pool and the load stay open, as in Shared
	{
		sError = sWriteError;
		return SharedBulkResult_e::FAILED;
	}

	const CSphString sOwnError = ShardBulkError ( m_sTable, "failed to write to", DescribeTarget ( iRoute ), sWriteError );
	sError = Fail ( sOwnError );
	return sError==sOwnError ? SharedBulkResult_e::FAILED : SharedBulkResult_e::FAILED_ALREADY;
}


// Keep the first error and wake everybody. The targets of a started load go to dCancel, for the caller to cancel
// outside the lock (the shutdown does not); a load that is starting is cleaned up by Start(), and a commit that runs
// finishes its targets.
void ShardBulkLoad_c::FailLocked ( const CSphString & sError, CSphVector<int> & dCancel )
{
	if ( m_sError.IsEmpty() )
	{
		m_sError = sError.IsEmpty() ? CSphString ( "shared bulk_import failed" ) : sError;
		m_bFailed.store ( true, std::memory_order_release );
		if ( LOG_LEVEL_BULK_IMPORT )
			sphWarning ( "shared bulk_import into shard table '%s' failed: %s", m_sTable.cstr(), m_sError.cstr() );
		if ( m_bStarted && !m_bClosing )
			ARRAY_FOREACH ( iRoute, m_dAgents )
				dCancel.Add ( iRoute );
	}
	m_tChanged.NotifyAll();
}


CSphString ShardBulkLoad_c::Fail ( const CSphString & sError, bool bCancelTargets )
{
	CSphVector<int> dCancel;
	CSphString sFirst;
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		FailLocked ( sError, dCancel );
		sFirst = m_sError;
	}
	if ( bCancelTargets )
		for ( int iRoute : dCancel )
			CancelTarget ( iRoute );
	return sFirst;
}


// Fail the load unless a commit is publishing it; for PURGE and for requests outside the load.
bool ShardBulkLoad_c::FailOpen ( const CSphString & sError )
{
	CSphVector<int> dCancel;
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		if ( m_bClosing )
			return false;
		FailLocked ( sError, dCancel );
	}
	for ( int iRoute : dCancel )
		CancelTarget ( iRoute );
	return true;
}


// Every target commits at once (Call_c); a failed one does not stop the others. The reply names the first
// failed target in route order and the published ones.
SharedBulkResult_e ShardBulkLoad_c::Publish ( bool bAdmitted, CSphString & sError )
{
	bool bPublish;
	const bool bChanged = !IsCurrentWrapper();
	CSphVector<int> dCancel;
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		if ( m_bClosing )
		{
			sError.SetSprintf ( "shared bulk_import into table '%s' is being published", m_sTable.cstr() );
			return SharedBulkResult_e::CLOSING;
		}
		bPublish = m_sError.IsEmpty();
		if ( bPublish && m_iActive>( bAdmitted ? 1 : 0 ) )
		{
			sError.SetSprintf ( "shared bulk_import into table '%s' has other active requests", m_sTable.cstr() );
			return SharedBulkResult_e::BUSY;
		}
		if ( bPublish && bChanged )
		{
			FailLocked ( SphSprintf ( "shard table '%s' changed while shared bulk_import into it was active", m_sTable.cstr() ), dCancel );
			bPublish = false;
		}
		if ( bPublish )
			m_bClosing = true; // from now on the targets belong to this commit: DROP TABLE or shutdown do not cancel them
		else
			sError = m_sError;
	}

	// a failed load only leaves the hash; its targets were cancelled when it failed
	if ( !bPublish )
	{
		for ( int iRoute : dCancel )
			CancelTarget ( iRoute );
		ReplaceShardBulkLoad ( m_sTable, this, nullptr );
		return SharedBulkResult_e::FAILED;
	}

	const int64_t tmStart = ShardBulkTimer();
	Call_c tCall ( *this, ShardBulkOp_e::COMMIT );
	tCall.Send(); // COMMIT is on its way to every remote target
	StrVec_t dNames;
	for ( int iRoute : tCall.m_dLocal )
		dNames.Add ( Route ( iRoute ).m_sName );
	CSphFixedVector<bool> dNameOk ( dNames.GetLength() );
	CSphFixedVector<CSphString> dNameErrors ( dNames.GetLength() );
	CommitShardBulkTargets ( dNames, m_iLoadId, dNameOk, dNameErrors ); // EOF to every local pool first, so they build together
	ARRAY_FOREACH ( i, tCall.m_dLocal )
	{
		tCall.m_dOk[tCall.m_dLocal[i]] = dNameOk[i];
		tCall.m_dErrors[tCall.m_dLocal[i]] = std::move ( dNameErrors[i] );
	}
	tCall.Wait();
	const auto & dOk = tCall.m_dOk;
	const auto & dErrors = tCall.m_dErrors;

	const int iTargets = dOk.GetLength();
	StrVec_t dPublished;
	CSphString sStop;
	ARRAY_FOREACH ( iRoute, dOk )
	{
		const auto & tRoute = Route ( iRoute );
		ShardBulkInfo ( "BULK_IMPORT_SHARD target_published table=%s load=" INT64_FMT " target=%s ok=%d", m_sTable.cstr(), m_iLoadId, tRoute.m_sName.cstr(), (int)dOk[iRoute] );
		if ( dOk[iRoute] )
		{
			dPublished.Add ( tRoute.m_sName );
			continue;
		}

		// a remote target without a confirmed commit is not counted as published and gets the cancel; it may have
		// published nevertheless, when only the reply was lost
		if ( !tRoute.IsLocal() )
			CancelTarget ( iRoute );
		const CSphString sFailed = ShardBulkError ( m_sTable, tRoute.IsLocal() ? "failed to commit" : "has no confirmed commit of", DescribeTarget ( iRoute ), dErrors[iRoute] );
		if ( sStop.IsEmpty() )
			sStop = sFailed;
		else
			sphWarning ( "%s", sFailed.cstr() ); // the reply names only the first failed target
	}

	ShardBulkInfo ( "BULK_IMPORT_SHARD finished table=%s load=" INT64_FMT " targets=%d published=%d finalize_ms=%.3f", m_sTable.cstr(), m_iLoadId, iTargets, dPublished.GetLength(), ( sphMicroTimer()-tmStart )/1000.0 );
	ReplaceShardBulkLoad ( m_sTable, this, nullptr );
	if ( sStop.IsEmpty() )
		return SharedBulkResult_e::PUBLISHED;

	// the first error, then the confirmed publications; the other targets failed or were cancelled
	StringBuilder_c sOut;
	sOut << sStop << "; published " << dPublished.GetLength() << " of " << iTargets << " tables";
	for ( int i=0; i<dPublished.GetLength() && i<SHARD_BULK_PUBLISHED_NAMES; ++i )
		sOut << ( i ? ", " : ": " ) << dPublished[i];
	if ( dPublished.GetLength()>SHARD_BULK_PUBLISHED_NAMES )
		sOut << " and " << dPublished.GetLength()-SHARD_BULK_PUBLISHED_NAMES << " more";
	sOut << "; not published: " << iTargets-dPublished.GetLength();
	sOut.MoveTo ( sError );
	return SharedBulkResult_e::FAILED;
}


bool ShardBulkLoad_c::WriteTarget ( int iRoute, Str_t tCsv, int64_t iRows, bool & bRequestOnly, CSphString & sError ) const
{
	if ( Route ( iRoute ).IsLocal() )
		return WriteShardBulkTarget ( Route ( iRoute ).m_sName, m_iLoadId, tCsv, iRows, bRequestOnly, sError )==SharedBulkResult_e::STAGED;

	ShardBulkRequestBuilder_c tBuilder;
	tBuilder.m_eOp = ShardBulkOp_e::DATA;
	tBuilder.m_tCsv = tCsv;
	tBuilder.m_iRows = iRows;
	return CallTarget ( tBuilder, iRoute, sError );
}


// Best effort: a target that is gone or busy publishing keeps what it has; PURGE BULK_IMPORT cleans up the rest.
void ShardBulkLoad_c::CancelTarget ( int iRoute ) const
{
	CSphString sError;
	bool bCancelled;
	if ( Route ( iRoute ).IsLocal() )
		bCancelled = CancelShardBulkTarget ( Route ( iRoute ).m_sName, m_iLoadId, sError );
	else
	{
		ShardBulkRequestBuilder_c tBuilder;
		tBuilder.m_eOp = ShardBulkOp_e::CANCEL;
		bCancelled = CallTarget ( tBuilder, iRoute, sError );
	}

	if ( !bCancelled )
		sphWarning ( "shared bulk_import into shard table '%s': cancel of %s failed: %s; if a pool is left there, PURGE BULK_IMPORT FROM TABLE %s OPTION clear_sessions=1 removes it", m_sTable.cstr(), DescribeTarget ( iRoute ).cstr(), sError.cstr(), m_sTable.cstr() );
}


//////////////////////////////////////////////////////////////////////////
// one HTTP request

struct ShardBulkRequest_c::Impl_t
{
	struct Batch_t
	{
		StringBuilder_c m_sCsv; // unwritten tail of the target, less than a batch; its buffer is reused by every batch
		int64_t m_iRows = 0;
	};

	CSphString m_sTable;
	CSphString m_sShared;			// N of shared=N: ignored, every target has one child
	bool m_bJoin = false;			// shared_join: never create a load
	ShardBulkLoadRefPtr_c m_pLoad;	// the load this request is admitted to
	bool m_bStarted = false;		// the first document waited for the start of the targets
	CSphFixedVector<Batch_t> m_dBatches { 0 };	// by route

	IndexerRtBulkLayout_t m_tLayout; // insert layout of the shard table schema

	void Admit ( ShardBulkLoadRefPtr_c pLoad )
	{
		m_pLoad = std::move ( pLoad );
		m_dBatches.Reset ( m_pLoad->m_tShard.m_dRouteTargets.GetLength() );
	}

	// Join the load of the shard table or, unless shared_join, create it with the first data line.
	SharedBulkResult_e Join ( CSphString & sError )
	{
		while ( true )
		{
			if ( CheckIndexerRtBulkCancelled ( sError ) )
				return SharedBulkResult_e::FAILED;

			auto pLoad = GetShardBulkLoad ( m_sTable );
			if ( pLoad )
			{
				auto eResult = pLoad->Enter ( sError );
				if ( eResult!=SharedBulkResult_e::STAGED )
					return eResult;
				Admit ( std::move ( pLoad ) );
				return SharedBulkResult_e::STAGED;
			}

			if ( m_bJoin )
			{
				sError.SetSprintf ( "no shared bulk_import into table '%s' to join", m_sTable.cstr() );
				return SharedBulkResult_e::NO_POOL;
			}

			auto pWrapper = GetDistr ( m_sTable );
			if ( !AsShard ( pWrapper.Ptr() ) )
			{
				sError.SetSprintf ( "bulk_import requires an existing local RT table; table '%s' is absent or distributed", m_sTable.cstr() );
				return SharedBulkResult_e::PRECONDITION;
			}

			pLoad = new ShardBulkLoad_c ( m_sTable, std::move ( pWrapper ) ); // already counts this request as admitted
			if ( !ReplaceShardBulkLoad ( m_sTable, nullptr, pLoad ) )
				continue;

			// a DROP TABLE between the lookup and the registration found no load to cancel, and nobody could remove this
			// one later; look again
			if ( !pLoad->IsCurrentWrapper() )
			{
				ReplaceShardBulkLoad ( m_sTable, pLoad, nullptr );
				continue;
			}

			ShardBulkInfo ( "BULK_IMPORT_SHARD created table=%s load=" INT64_FMT " routes=%d", m_sTable.cstr(), pLoad->m_iLoadId, pLoad->m_tShard.m_dRouteTargets.GetLength() );
			if ( !m_sShared.IsEmpty() || env_exists ( "INDEXER_RT_BULK_SHARED_POOL_SIZE" ) )
				ShardBulkInfo ( "BULK_IMPORT_SHARD pool_size_ignored table=%s requested=%s pool_size=1", m_sTable.cstr(), m_sShared.IsEmpty() ? "env" : m_sShared.cstr() );
			Admit ( std::move ( pLoad ) );
			return m_pLoad->Start ( sError ); // a failed start fails the load; it stays until commit or PURGE
		}
	}

	SharedBulkResult_e Flush ( int iRoute, CSphString & sError )
	{
		auto & tBatch = m_dBatches[iRoute];
		if ( !tBatch.m_sCsv.GetLength() )
			return SharedBulkResult_e::STAGED;
		auto eResult = m_pLoad->Write ( iRoute, (Str_t)tBatch.m_sCsv, std::exchange ( tBatch.m_iRows, 0 ), sError );
		tBatch.m_sCsv.Rewind(); // also after a failure: the request stops, its tail is dropped
		return eResult;
	}

	// An invalid document fails the open load, also when this request has not joined it yet; a load that is being
	// published is left alone by requests outside it.
	void Fail ( const CSphString & sError )
	{
		if ( m_pLoad )
			m_pLoad->Fail ( sError );
		else if ( auto pLoad = GetShardBulkLoad ( m_sTable ) )
			pLoad->FailOpen ( sError );
	}
};


ShardBulkRequest_c::ShardBulkRequest_c ( const CSphString & sTable, const CSphString & sShared, bool bJoin )
	: m_pImpl { std::make_unique<Impl_t>() }
{
	m_pImpl->m_sTable = sTable;
	m_pImpl->m_sShared = sShared;
	m_pImpl->m_bJoin = bJoin;
}


ShardBulkRequest_c::~ShardBulkRequest_c()
{
	if ( m_pImpl->m_pLoad )
		m_pImpl->m_pLoad->Leave(); // a broken body drops only its unwritten tails; the load stays open
}


// Admit before reading the body if the table has an open load, so that a commit elsewhere sees this request.
void ShardBulkRequest_c::Begin()
{
	auto pLoad = GetShardBulkLoad ( m_pImpl->m_sTable );
	CSphString sEnter;
	if ( pLoad && pLoad->Enter ( sEnter )==SharedBulkResult_e::STAGED )
		m_pImpl->Admit ( std::move ( pLoad ) );
}


SharedBulkResult_e ShardBulkRequest_c::StageLine ( Str_t tLine, CSphString & sError )
{
	GlobalCrashQueryGetRef().m_dQuery = { (const BYTE*) tLine.first, tLine.second };
	SqlStmt_t tStmt;
	tStmt.m_bJson = true;
	tStmt.m_tQuery.m_eQueryType = QUERY_JSON;
	CSphString sStmt, sQuery;
	DocID_t tDocId = 0;
	if ( sphParseJsonStatement ( tLine.first, tStmt, sStmt, sQuery, tDocId, sError, &m_pImpl->m_sTable ) )
		return Stage ( tStmt, sError );

	m_pImpl->Fail ( sError );
	return SharedBulkResult_e::FAILED_INPUT;
}


SharedBulkResult_e ShardBulkRequest_c::Stage ( const SqlStmt_t & tStmt, CSphString & sError )
{
	auto & tImpl = *m_pImpl;
	if ( tStmt.m_eStmt!=STMT_INSERT || tStmt.m_sIndex!=tImpl.m_sTable )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' accepts only INSERT into that table", tImpl.m_sTable.cstr() );
		tImpl.Fail ( sError );
		return SharedBulkResult_e::FAILED_INPUT;
	}

	if ( !tImpl.m_pLoad )
	{
		auto eResult = tImpl.Join ( sError );
		if ( eResult!=SharedBulkResult_e::STAGED )
			return eResult;
	}

	ShardBulkLoad_c & tLoad = *tImpl.m_pLoad;
	if ( !tImpl.m_bStarted )
	{
		// also a request that Begin() admitted while the targets were starting, as a Shared request waits for its pool
		auto eStarted = tLoad.WaitStarted ( sError );
		if ( eStarted!=SharedBulkResult_e::STAGED )
			return eStarted;
		tImpl.m_bStarted = true;
	}

	// the cluster prefix of the row as an ordinary write into the shard table checks it
	if ( !ValidateClusterStatement ( tStmt.m_sIndex, tLoad.m_tShard.m_sCluster, tStmt.m_sCluster, true ) )
	{
		TlsMsg::MoveError ( sError );
		tLoad.Fail ( sError );
		return SharedBulkResult_e::FAILED_INPUT;
	}

	if ( !tImpl.m_tLayout.Prepare ( tLoad.m_tShard.m_tSchema, tLoad.m_dCsvAttrOrder, tStmt, sError ) )
	{
		tLoad.Fail ( sError );
		return SharedBulkResult_e::FAILED_INPUT;
	}

	// the id as an ordinary insert takes it, routed as an ordinary write into the shard table routes it
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		const uint64_t uId = tImpl.m_tLayout.DocId ( tStmt, iRow );
		const int iRoute = GetShardTargetOrdinal ( tLoad.m_tShard, (int64_t)uId );
		auto & tBatch = tImpl.m_dBatches[iRoute];
		tImpl.m_tLayout.AppendRow ( tBatch.m_sCsv, tStmt, iRow, uId );
		++tBatch.m_iRows;
		if ( tBatch.m_sCsv.GetLength()<tLoad.GetBatchBytes ( iRoute ) )
			continue;

		auto eResult = tImpl.Flush ( iRoute, sError );
		if ( eResult!=SharedBulkResult_e::STAGED )
			return eResult;
	}
	return SharedBulkResult_e::STAGED;
}


// A commit request carries no documents; Begin() may have admitted it to the open load already.
SharedBulkResult_e ShardBulkRequest_c::Commit ( CSphString & sError )
{
	auto & tImpl = *m_pImpl;
	ShardBulkLoadRefPtr_c pLoad = tImpl.m_pLoad ? tImpl.m_pLoad : GetShardBulkLoad ( tImpl.m_sTable );
	if ( !pLoad )
	{
		sError.SetSprintf ( "no shared bulk_import into table '%s' to commit", tImpl.m_sTable.cstr() );
		return SharedBulkResult_e::NO_POOL;
	}
	return pLoad->Publish ( !!tImpl.m_pLoad, sError );
}


// The end of a data request writes the tails of every target, one by one.
SharedBulkResult_e ShardBulkRequest_c::End ( CSphString & sError )
{
	auto & tImpl = *m_pImpl;
	if ( !tImpl.m_pLoad )
		return SharedBulkResult_e::STAGED;

	ARRAY_FOREACH ( iRoute, tImpl.m_dBatches )
	{
		auto eResult = tImpl.Flush ( iRoute, sError );
		if ( eResult!=SharedBulkResult_e::STAGED )
			return eResult;
	}
	return SharedBulkResult_e::STAGED;
}


//////////////////////////////////////////////////////////////////////////
// control

bool PurgeShardBulkLoad ( const CSphString & sTable, bool bClearSessions, CSphString & sError )
{
	auto pLoad = GetShardBulkLoad ( sTable );
	if ( !pLoad )
		return true;
	if ( !bClearSessions )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is open or failed; commit it or use OPTION clear_sessions=1 to cancel it", sTable.cstr() );
		return false;
	}
	if ( !pLoad->FailOpen ( "shared bulk_import was cancelled by PURGE BULK_IMPORT" ) )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is being published; repeat PURGE after it finishes", sTable.cstr() );
		return false;
	}
	ReplaceShardBulkLoad ( sTable, pLoad, nullptr );
	return true;
}


// The same PURGE BULK_IMPORT as SQL to an agent table, as distributed DELETE forwards SQL.
class ShardBulkPurgeBuilder_c final : public RequestBuilder_i
{
public:
	explicit ShardBulkPurgeBuilder_c ( bool bClearSessions )
		: m_bClearSessions ( bClearSessions )
	{}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		CSphString sQuery;
		sQuery.SetSprintf ( "PURGE BULK_IMPORT FROM TABLE %s%s", tAgent.m_tDesc.m_sIndexes.cstr(), m_bClearSessions ? " OPTION clear_sessions=1" : "" );
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SPHINXQL, VER_COMMAND_SPHINXQL );
		{
			APIBlob_c dWrapper ( tOut ); // sphinxql is wrapped twice
			tOut.SendBytes ( sQuery );
		}
		tOut.SendDword ( 0 ); // API flags
	}

private:
	bool m_bClearSessions;
};


// Every mirror gets it, since the mirror a lost coordinator pinned is unknown; an agent line that names several
// tables gets one command per table. A failed call is reported, never hidden by an overall OK.
void PurgeRemoteBulkImport ( const DistributedIndex_t & tDistr, const CSphString & sTable, bool bClearSessions, SearchFailuresLog_c & dFailures )
{
	VecRefPtrsAgentConn_t dAgents;
	tDistr.ForEveryHost ( [&] ( const AgentDesc_t & tHost ) {
		if ( tHost.m_bBlackhole )
			return;
		StrVec_t dTables;
		ParseIndexList ( tHost.m_sIndexes, dTables );
		for ( const CSphString & sRemote : dTables )
		{
			auto * pConn = new AgentConn_t;
			pConn->m_tDesc.CloneFrom ( tHost );
			pConn->m_tDesc.m_sIndexes = sRemote;
			pConn->m_iMyConnectTimeoutMs = tDistr.GetAgentConnectTimeoutMs();
			pConn->m_iMyQueryTimeoutMs = tDistr.GetAgentQueryTimeoutMs();
			dAgents.Add ( pConn );
		}
	} );
	if ( dAgents.IsEmpty() )
		return;

	SetSessionAuth ( dAgents );
	ShardBulkPurgeBuilder_c tBuilder ( bClearSessions );
	int iUpdated = 0, iWarnings = 0;
	SearchFailuresLog_c dParserFailures;
	auto pParser = CreateReplyParser ( false, iUpdated, iWarnings, dParserFailures );
	PerformRemoteTasks ( dAgents, &tBuilder, pParser.get(), 0 );
	for ( const AgentConn_t * pAgent : dAgents )
		if ( !pAgent->m_bSuccess )
			dFailures.SubmitEx ( pAgent->m_tDesc.m_sIndexes, sTable.cstr(), "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.IsEmpty() ? "remote PURGE BULK_IMPORT failed" : pAgent->m_sFailure.cstr() );
}


void DropShardBulkImport ( const CSphString & sTable )
{
	auto pLoad = GetShardBulkLoad ( sTable );
	if ( !pLoad )
		return;
	pLoad->Fail ( "shared bulk_import was cancelled by DROP TABLE" );
	ReplaceShardBulkLoad ( sTable, pLoad, nullptr );
}


void ShutdownShardBulkImport()
{
	// shutdown callbacks run on the main thread; the load mutex needs a coroutine. Local pools are stopped by
	// ShutdownSharedBulkImport; remote ones stay until PURGE BULK_IMPORT, as after a crash.
	Threads::CallCoroutine ( [] {
		CSphVector<ShardBulkLoadRefPtr_c> dLoads;
		{
			ScopedMutex_t tLock ( g_tShardBulkLock );
			for ( const auto & tLoad : g_hShardBulk )
				dLoads.Add ( tLoad.second );
			g_hShardBulk.Reset();
		}
		for ( const auto & pLoad : dLoads )
			pLoad->Fail ( "Server shutdown in progress", false );
	} );
}


//////////////////////////////////////////////////////////////////////////
// a remote target

void HandleCommandShardBulk ( GenericOutputBuffer_c & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SHARD_BULK, tOut ) )
		return;

	const auto eOp = (ShardBulkOp_e)tReq.GetWord();
	const DWORD uHeartbeatIntervalMs = tReq.GetDword();
	const CSphString sTable = tReq.GetString();
	const auto iLoadId = (int64_t)tReq.GetUint64();
	CSphString sShard;
	Str_t tCsv { nullptr, 0 }; // stays in the request buffer, which lives through the handler
	int64_t iRows = 0;
	if ( eOp==ShardBulkOp_e::START )
		sShard = tReq.GetString();
	else if ( eOp==ShardBulkOp_e::DATA )
	{
		iRows = (int64_t)tReq.GetUint64();
		tCsv.second = tReq.GetInt();
		tReq.GetBytesZerocopy ( (const BYTE **)&tCsv.first, tCsv.second ); // a bad length sets the error checked below
	}

	// load id 0 marks an ordinary Shared pool, which these operations must never touch
	const bool bHeartbeat = HasHeartbeat ( eOp );
	if ( !iLoadId || tReq.GetError() || ( bHeartbeat && !uHeartbeatIntervalMs ) )
	{
		auto tReply = APIAnswer ( tOut, uVer );
		tOut.SendString ( "invalid or truncated shard bulk_import request" ); // a refusal (ShardBulkReply_t)
		return;
	}

	std::optional<ApiReplyStream_c> tStream;
	if ( bHeartbeat )
	{
		tStream.emplace ( uHeartbeatIntervalMs, uVer, tOut );
		tStream->Start();
	}

	CSphString sError;
	bool bOk;
	switch ( eOp )
	{
	case ShardBulkOp_e::START:	bOk = StartShardBulkTarget ( sTable, iLoadId, sShard, -1, 0, sError )==SharedBulkResult_e::STAGED; break;
	case ShardBulkOp_e::DATA:	{ bool bRequestOnly; bOk = WriteShardBulkTarget ( sTable, iLoadId, tCsv, iRows, bRequestOnly, sError )==SharedBulkResult_e::STAGED; break; }
	case ShardBulkOp_e::COMMIT:	bOk = CommitShardBulkTarget ( sTable, iLoadId, sError )==SharedBulkResult_e::PUBLISHED; break;
	default:					bOk = CancelShardBulkTarget ( sTable, iLoadId, sError ); break;
	}

	if ( tStream && !tStream->StopAndHandoff() )
		return;

	auto tReply = APIAnswer ( tOut, uVer );
	if ( !bOk )
		tOut.SendString ( sError.cstr() ); // a refusal (ShardBulkReply_t)
}
