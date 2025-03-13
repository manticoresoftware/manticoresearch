//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "receiver_ctx.h"

#include "tracer.h"
#include "sphinxpq.h"
#include "accumulator.h"
#include "memio.h"
#include "serialize.h"
#include "sphinxrt.h"
#include "searchdaemon.h"
#include "searchdreplication.h"


// verbose logging of replcating transactions, ruled by this env variable
static bool LOG_LEVEL_RPL_TNX = val_from_env ( "MANTICORE_LOG_RPL_TNX", false );
#define LOG_COMPONENT_RPL_TNX ""
#define RPL_TNX LOGMSG ( RPL_DEBUG, RPL_TNX, RPL_TNX )

// data passed to Galera and used at callbacks
class ReceiverCtx_c final: public Wsrep::Receiver_i
{
	// share of remote commands received between apply and commit callbacks
	RtAccum_t m_tAcc; // apply fn, commit fn
	CSphQuery m_tQuery;
	CSphString m_sName; // name of serving cluster
	Wsrep::Provider_i* m_pProvider = nullptr;
	std::function<void()> m_fnOnClean;

private:
	void Cleanup();
	~ReceiverCtx_c() final;

public:
	ReceiverCtx_c ( CSphString sName, Wsrep::Provider_i* pProvider, std::function<void()> fnOnClean );
	ReceiverCtx_c ( const ReceiverCtx_c& ) = delete;
	ReceiverCtx_c ( ReceiverCtx_c&& ) = delete;
	ReceiverCtx_c& operator= ( const ReceiverCtx_c& ) = delete;
	ReceiverCtx_c& operator= ( ReceiverCtx_c&& ) = delete;

	// implementation of Wsrep::Receiver_i
	bool ApplyWriteset ( ByteBlob_t tData, bool bIsolated ) final;
	void ApplyUnordered ( ByteBlob_t tData ) final;
	bool Commit ( const void* pHndTrx, uint32_t uFlags, const Wsrep::TrxMeta_t* pMeta, bool bCommit ) final;

private:
	static bool PQAdd ( ReplicationCommand_t* pCmd, ByteBlob_t tReq );
};

Wsrep::Receiver_i* MakeReceiverCtx ( CSphString sName, Wsrep::Provider_i* pProvider, std::function<void()> fnOnClean )
{
	return new ReceiverCtx_c ( std::move ( sName ), pProvider, std::move(fnOnClean) );
}

ReceiverCtx_c::ReceiverCtx_c ( CSphString sName, Wsrep::Provider_i * pProvider, std::function<void()> fnOnClean )
	: m_sName {std::move ( sName )}
	, m_pProvider ( pProvider )
	, m_fnOnClean { std::move ( fnOnClean ) }
{}

ReceiverCtx_c::~ReceiverCtx_c ()
{
	Cleanup ();
}

void ReceiverCtx_c::Cleanup ()
{
	m_tAcc.Cleanup();
	m_tQuery.m_dFilters.Reset();
	m_tQuery.m_dFilterTree.Reset();
	if ( m_fnOnClean )
		m_fnOnClean();
}

bool ReceiverCtx_c::PQAdd ( ReplicationCommand_t* pCmd, ByteBlob_t tReq )
{
	assert ( pCmd && pCmd->m_eCommand == ReplCmd_e::PQUERY_ADD );

	cServedIndexRefPtr_c pServed = GetServed ( pCmd->m_sIndex );
	if ( !pServed )
	{
		sphWarning ( "unknown table '%s' for replication, command %d", pCmd->m_sIndex.cstr(), (int)pCmd->m_eCommand );
		return false;
	}

	if ( pServed->m_eType != IndexType_e::PERCOLATE )
	{
		sphWarning ( "wrong type of table '%s' for replication, command %d", pCmd->m_sIndex.cstr(), (int)pCmd->m_eCommand );
		return false;
	}

	StoredQueryDesc_t tPQ;
	LoadStoredQuery ( tReq, tPQ );
	RPL_TNX << "pq-add, table '" << pCmd->m_sIndex.cstr() << "', uid " << tPQ.m_iQUID << " query " << tPQ.m_sQuery.cstr();

	CSphString sError;
	PercolateQueryArgs_t tArgs ( tPQ );
	tArgs.m_bReplace = true;
	pCmd->m_pStored = RIdx_T<PercolateIndex_i*> ( pServed )->CreateQuery ( tArgs, sError );

	if ( !pCmd->m_pStored )
	{
		sphWarning ( "pq-add replication error '%s', table '%s'", sError.cstr(), pCmd->m_sIndex.cstr() );
		return false;
	}
	return true;
}

// callback for Galera to parse replicated commands
bool ReceiverCtx_c::ApplyWriteset ( ByteBlob_t tData, bool bIsolated )
{
	MemoryReader_c tReader ( tData );
	while ( tReader.GetPos() < tData.second )
	{
		auto pCmd = std::make_unique<ReplicationCommand_t>();
		if ( !LoadCmdHeader ( tReader, pCmd.get() ))
		{
			sphWarning ( "%s", TlsMsg::szError() );
			return false;
		}

		auto iRequestLen = (int)tReader.GetDword();
		if ( iRequestLen + tReader.GetPos() > tData.second )
		{
			sphWarning ( "replication parse apply - out of buffer read %d+%d of %d", tReader.GetPos(), iRequestLen, tData.second );
			return false;
		}

		const BYTE * pRequest = tData.first + tReader.GetPos();
		tReader.SetPos ( tReader.GetPos() + iRequestLen );

		pCmd->m_sCluster = m_sName;
		pCmd->m_bIsolated = bIsolated;

		ByteBlob_t tReq { pRequest, iRequestLen };

		switch ( pCmd->m_eCommand )
		{
		case ReplCmd_e::PQUERY_ADD:
			if ( !PQAdd ( pCmd.get(), tReq ) )
				return false;
			break;

		case ReplCmd_e::PQUERY_DELETE:
			LoadDeleteQuery ( tReq, pCmd->m_dDeleteQueries, pCmd->m_sDeleteTags );
			RPL_TNX << "pq-delete, table '" << pCmd->m_sIndex.cstr() << "', queries " << pCmd->m_dDeleteQueries.GetLength() << ", tags " << pCmd->m_sDeleteTags.scstr();
			break;

		case ReplCmd_e::TRUNCATE:
			RPL_TNX << "pq-truncate, table '" << pCmd->m_sIndex.cstr() << "'";
			break;

		case ReplCmd_e::CLUSTER_ALTER_ADD:
			pCmd->m_bCheckIndex = false;
			RPL_TNX << "pq-cluster-alter-add, table '" << pCmd->m_sIndex.cstr() << "'";
			break;

		case ReplCmd_e::CLUSTER_ALTER_DROP:
			RPL_TNX << "pq-cluster-alter-drop, table '" << pCmd->m_sIndex.cstr() << "'";
			break;

		case ReplCmd_e::RT_TRX:
			m_tAcc.LoadRtTrx ( tReq, pCmd->m_uVersion );
			RPL_TNX << "rt trx, table '" << pCmd->m_sIndex.cstr() << "'";
			break;

		case ReplCmd_e::UPDATE_API:
			pCmd->m_pUpdateAPI = new CSphAttrUpdate;
			LoadAttrUpdate ( pRequest, iRequestLen, *pCmd->m_pUpdateAPI, pCmd->m_bBlobUpdate );
			RPL_TNX << "update, table '" << pCmd->m_sIndex.cstr() << "'";
			break;

		case ReplCmd_e::UPDATE_QL:
		case ReplCmd_e::UPDATE_JSON:
		{
			// can not handle multiple updates - only one update at time
			assert ( !m_tQuery.m_dFilters.GetLength() );

			pCmd->m_pUpdateAPI = new CSphAttrUpdate;
			int iGot = LoadAttrUpdate ( pRequest, iRequestLen, *pCmd->m_pUpdateAPI, pCmd->m_bBlobUpdate );
			assert ( iGot<iRequestLen );
			LoadUpdate ( pRequest + iGot, iRequestLen - iGot, m_tQuery );
			pCmd->m_pUpdateCond = &m_tQuery;
			RPL_TNX << "update " << ( pCmd->m_eCommand == ReplCmd_e::UPDATE_QL ? "ql" : "json" ) << ", table '" << pCmd->m_sIndex.cstr() << "'";
			break;
		}

		default:
			sphWarning ( "unsupported replication command %d", (int) pCmd->m_eCommand );
			return false;
		}

		m_tAcc.m_dCmd.Add ( std::move ( pCmd ) );
	}

	if ( tReader.GetPos() == tData.second )
		return true;

	sphWarning ( "replication parse apply - out of buffer read %d of %d", tReader.GetPos(), tData.second );
	return false;
}

void ReceiverCtx_c::ApplyUnordered ( ByteBlob_t foo )
{
	sphLogDebugRpl ( "unordered byteblob size %d", foo.second );
}

bool ReceiverCtx_c::Commit ( const void* pHndTrx, uint32_t uFlags, const Wsrep::TrxMeta_t* pMeta, bool bCommit )
{
	AT_SCOPE_EXIT ( [this] { Cleanup(); } );
	if ( !bCommit || m_tAcc.m_dCmd.IsEmpty() )
		return true;

	bool bOk = true;
	bool bIsolated = ( m_tAcc.m_dCmd[0]->m_bIsolated );

	if ( bIsolated || !m_pProvider->GetApplier() ) {
		bOk = HandleCmdReplicated ( m_tAcc );
	} else
	{
		m_pProvider->GetApplier()->ApplierPreCommit ( pHndTrx );
		bOk = HandleCmdReplicated ( m_tAcc );
		m_pProvider->GetApplier()->ApplierInterimPostCommit ( pHndTrx );
	}
	if ( TlsMsg::HasErr ())
		sphWarning ( "%s", TlsMsg::szError ());

	sphLogDebugRpl ( "seq " INT64_FMT ", committed %d, isolated %d", (int64_t) pMeta->m_tGtid.m_iSeqNo, (int) bOk, (int) bIsolated );
	return bOk;
}
