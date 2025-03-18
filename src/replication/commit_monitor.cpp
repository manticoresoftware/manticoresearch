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

#include "commit_monitor.h"

#include "tracer.h"
#include "searchdreplication.h"

// commit for common commands
bool CommitMonitor_c::Commit ()
{
	TRACE_CONN ( "conn", "CommitMonitor_c::Commit" );
	using namespace TlsMsg;
	RtIndex_i * pIndex = m_tAcc.GetIndex ();

	// short path for usual accum without commands
	if ( m_tAcc.m_dCmd.IsEmpty ())
		return pIndex && pIndex->Commit ( m_pDeletedCount, &m_tAcc );

	ReplicationCommand_t& tCmd = *m_tAcc.m_dCmd[0];
	bool bTruncate = tCmd.m_eCommand == ReplCmd_e::TRUNCATE;
	bool bOnlyTruncate = bTruncate && ( m_tAcc.m_dCmd.GetLength() == 1 );

	// process with index from accum (no need to lock/unlock it)
	if ( pIndex )
		return CommitNonEmptyCmds ( pIndex, tCmd, bOnlyTruncate );

	auto pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
		return Err ( "requires an existing table" );

	// truncate needs wlocked index
	if ( ServedDesc_t::IsMutable ( pServed ) )
	{
		if ( bTruncate )
		{
			WIdx_T<RtIndex_i*> tLocked ( pServed );
			return CommitNonEmptyCmds ( tLocked, tCmd, bOnlyTruncate );
		} else
		{
			RIdx_T<RtIndex_i*> tLocked ( pServed );
			return CommitNonEmptyCmds ( tLocked, tCmd, bOnlyTruncate );
		}
	}

	return Err ( "requires an existing RT or percolate table" );
}

bool CommitMonitor_c::CommitNonEmptyCmds ( RtIndex_i * pIndex, const ReplicationCommand_t & tCmd, bool bOnlyTruncate ) const
{
	TLS_MSG_STRING ( sError );

	assert ( pIndex );
	if ( !bOnlyTruncate )
		return pIndex->Commit ( m_pDeletedCount, &m_tAcc, &sError );

	if ( !pIndex->Truncate ( sError, RtIndex_i::TRUNCATE ))
		return false;

	if ( !tCmd.m_tReconfigure )
		return true;

	CSphReconfigureSetup tSetup;
	StrVec_t dWarnings;
	bool bSame = pIndex->IsSameSettings ( *tCmd.m_tReconfigure, tSetup, dWarnings, sError );
	return ( bSame || !sError.IsEmpty() || pIndex->Reconfigure ( tSetup ) ) && sError.IsEmpty();
}

// commit for Total Order Isolation commands
bool CommitMonitor_c::CommitTOI()
{
	const ReplicationCommand_t & tCmd = *m_tAcc.m_dCmd[0];
	switch (tCmd.m_eCommand)
	{
	case ReplCmd_e::CLUSTER_ALTER_ADD:
	case ReplCmd_e::CLUSTER_ALTER_DROP:
	{
		bool bOk = SetIndexesClusterTOI ( &tCmd );
		sphLogDebugRpl ( "CommitTOI %s for '%s'; %s", ( bOk ? "finished" : "failed" ), tCmd.m_sCluster.cstr(), ( bOk ? "" : TlsMsg::szError() ) );
		return bOk;
	}
	default:
		return TlsMsg::Err ( "unknown command '%d'", (int) tCmd.m_eCommand );
	}
}


class EnabledSaveGuard_c
{
	RtIndex_i * m_pRt;

public:
	NONCOPYMOVABLE( EnabledSaveGuard_c );

	explicit EnabledSaveGuard_c ( RtIndex_i * pRt ) noexcept
			: m_pRt { pRt }
	{
		if ( m_pRt )
			m_pRt->WaitLockEnabledState ();
	}

	~EnabledSaveGuard_c () noexcept
	{
		if ( m_pRt )
			m_pRt->UnlockEnabledState ();
	}
};

static bool DoUpdate ( AttrUpdateArgs& tUpd, const cServedIndexRefPtr_c& pDesc, int& iUpdated, bool bUpdateAPI, bool bNeedWlock )
{
	TRACE_CORO ( "rt", "commit_monitor::DoUpdate" );

	RtIndex_i * pRt = ( pDesc->m_eType==IndexType_e::RT ) ? static_cast<RtIndex_i *> ( UnlockedHazardIdxFromServed ( *pDesc ) ) : nullptr;
	EnabledSaveGuard_c tSaveEnabled { pRt };

	if ( bUpdateAPI )
	{
		Debug ( bool bOk = ) [&]() {
			if ( bNeedWlock )
			{
				WIdx_c tLocked ( pDesc );
				return HandleUpdateAPI ( tUpd, tLocked, iUpdated );
			} else
			{
				RWIdx_c tLocked ( pDesc );
				return HandleUpdateAPI ( tUpd, tLocked, iUpdated );
			}
		}();
		assert ( bOk ); // fixme! handle this
		return ( iUpdated >= 0 );
	}

	HandleMySqlExtendedUpdate ( tUpd, pDesc, iUpdated, bNeedWlock );

	if ( tUpd.m_pError->IsEmpty() )
		iUpdated += tUpd.m_iAffected;

	return ( tUpd.m_pError->IsEmpty() );
}

bool CommitMonitor_c::UpdateTOI ()
{
	TRACE_CORO ( "rt", "commit_monitor::UpdateTOI" );
	using namespace TlsMsg;
	if ( m_tAcc.m_dCmd.IsEmpty ())
		return TlsMsg::Err ( "empty accumulator" );

	const ReplicationCommand_t & tCmd = *m_tAcc.m_dCmd[0];
	cServedIndexRefPtr_c pServed { GetServed ( tCmd.m_sIndex ) };
	if ( !pServed )
		return TlsMsg::Err ( "requires an existing table" );

	assert ( m_pUpdated );
	assert ( m_pWarning );
	assert ( tCmd.m_pUpdateAPI );

	TLS_MSG_STRING ( sError );

	AttrUpdateArgs tUpd;
	tUpd.m_pUpdate = tCmd.m_pUpdateAPI;
	tUpd.m_pError = &sError;
	tUpd.m_pWarning = m_pWarning;
	tUpd.m_pQuery = tCmd.m_pUpdateCond;
	tUpd.m_pIndexName = &tCmd.m_sIndex;
	tUpd.m_bJson = ( tCmd.m_eCommand == ReplCmd_e::UPDATE_JSON );

	bool bUpdateAPI = ( tCmd.m_eCommand == ReplCmd_e::UPDATE_API );
	assert ( bUpdateAPI || tCmd.m_pUpdateCond );

	return DoUpdate ( tUpd, pServed, *m_pUpdated, bUpdateAPI, tCmd.m_bBlobUpdate );
}

CommitMonitor_c::~CommitMonitor_c ()
{
	m_tAcc.Cleanup ();
}