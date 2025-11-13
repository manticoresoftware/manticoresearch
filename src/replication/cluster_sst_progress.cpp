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

#include "sphinxstd.h"

#include "replication/common.h"
#include "replication/nodes.h"
#include "replication/cluster_file_reserve.h"
#include "searchdreplication.h"

#include "replication/cluster_sst_progress.h"

static const int g_iSstPushIntervalMs = val_from_env ( "MANTICORE_SST_PUSH_INTERVAL", 1 ) * 1000;

static const float ESTIMATED_DIFF_RATIO = 0.20f;
static const float g_dStageWeights[(int)SstStage_e::TOTAL] = { 0.01f, 1.0f, 1.0f, 2.5f, 0.5f };

static const bool LOG_LEVEL_SST_DBG = val_from_env ( "MANTICORE_LOG_SST_PROGRESS", false );
static const int LOG_SST_PROGRESS_INTERVAL_MS = val_from_env ( "MANTICORE_LOG_SST_PROGRESS_INTEVAL", 1000 );
#define LOG_COMPONENT_SST_DBG ""
#define SST_DBG LOGMSG ( RPL_DEBUG, SST_DBG, SST_DBG )

struct SstStageProgress_t
{
	SstStage_e m_eName;
	uint64_t m_uTotalUnits = 0;
	uint64_t m_uCompletedUnits = 0;

	float m_fWeight = 0.0;

	int64_t m_tmStarted = 0;
	int64_t m_tDelta = 0;

	float GetRaw() const;
	float GetTotal() const;
};

struct TableStat_t
{
	CSphString m_sName;
	uint64_t m_uTotalSize = 0;
	uint64_t m_uFiles = 0;

	float m_fWeight = 0.0;
};

class SstProgress_c : public SstProgress_i
{
public:
	SstProgress_c();
	~SstProgress_c();

	void Init ( Role_e eRole ) override;

	void StageBegin ( SstStage_e eStage ) override;
	void AddComplete ( uint64_t uBytes ) override;
	void UpdateTotal ( uint64_t uBytes ) override;

	void SetTable ( const CSphString & sName ) override;
	void Finish() override;

	void CollectFilesStats ( const VecTraits_T<CSphString> & dIndexes, int iAttempt, int iAgents ) override;

	void GetStatus ( SstProgressStatus_t & tStatus ) const override;

	// FIXME!!! implement these
	void UpdateFromRemote ( const SstProgressStatus_t& tStatus ) override;
	void StartLocalStage ( const SstProgressContext_t & tCtx, SstStage_e eStage ) override;
	void FinishLocalStage() override;

	void SetDonor4Joiner ( int iFilesCount, SstProgressContext_t & tCtx ) override;
	void StopPushUpdates ();
	bool IsPushUpdateDone() override { return m_bDonePushUpdates.load(); }

private:
	CSphString m_sTable;
	int m_iTable = -1;
	SstStage_e m_eStage = SstStage_e::TOTAL;
	int m_iAgents = 0;
	int m_iAttempt = 0;
	
	Role_e m_eRole = Role_e::NONE;
	SstProgressStatus_t m_tRemoteStatus;
	SstProgressContext_t m_tLocalCtx;
	uint64_t m_uLocalCompleted = 0;

	std::atomic<bool> m_bDonePushUpdates = true;

	CSphFixedVector<SstStageProgress_t> m_dProgress { 0 };
	CSphFixedVector<TableStat_t> m_dTables { 0 };

	void FinishPhase();
	SstStageProgress_t & GetStage() const;
	SstStageProgress_t * GetTableStage ( int iTable ) const;
	void RecalculateWeights ( int iTable );

	void SetTables ( const VecTraits_T<TableStat_t> & dTables, int iAttempt, int iAgents );

	float GetTotal() const;
	SstStage_e GetCurrentStage() const { return m_eStage; }
	float GetCurrentStageTotal() const;

	// debug log state
	int64_t m_tmLastLog = 0;
	void LogProgress ( bool bForce );
	void GetCurrentStageStats ( SstStageProgress_t & tStats ) const;

	friend class SstProgress_i;
};

SstProgress_i * CreateProgress ()
{
	return new SstProgress_c();
}

void SstProgress_c::CollectFilesStats ( const VecTraits_T<CSphString> & dIndexes, int iAttempt, int iAgents )
{
	CSphVector<TableStat_t> dStats;
	dStats.Reserve ( dIndexes.GetLength() );

	CSphString sError;
	for ( const CSphString & sIndex : dIndexes )
	{
		auto & tStat = dStats.Add();
		tStat.m_sName = sIndex;

		cServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !ServedDesc_t::IsMutable ( pServed ) )
			continue;

		RIdx_T<RtIndex_i*> pIndex { pServed };
		StrVec_t dFiles;
		pIndex->LockFileState ( dFiles );
		pIndex->EnableSave();

		tStat.m_uFiles = dFiles.GetLength();

		for ( const auto & sFile : dFiles )
		{
			int64_t iSize = sphGetFileSize ( sFile, nullptr );
			if ( iSize!=-1 )
				tStat.m_uTotalSize += iSize;
		}
	}

	SetTables ( dStats, iAttempt, iAgents );
}

SstProgress_c::SstProgress_c()
{
}

SstProgress_c::~SstProgress_c()
{
}

void SstProgress_c::AddComplete ( uint64_t uBytes )
{
	if ( m_eStage==SstStage_e::TOTAL || m_iTable==-1 )
		return;

	if ( m_eRole==Role_e::DONOR )
		GetStage().m_uCompletedUnits += uBytes;
	else
		m_uLocalCompleted += uBytes;

	LogProgress ( false );
}

void SstProgress_c::UpdateTotal ( uint64_t uBytes )
{
	assert( m_eRole==Role_e::DONOR );

	if ( m_eStage==SstStage_e::TOTAL || m_iTable==-1 )
		return;

	GetStage().m_uTotalUnits = uBytes;
	RecalculateWeights ( m_iTable );
}


void SstProgress_c::StageBegin ( SstStage_e eStage )
{
	assert( m_eRole==Role_e::DONOR );
	FinishPhase();
	m_eStage = eStage;

	if ( LOG_LEVEL_SST_DBG && m_iTable!=-1 )
	{
		SST_DBG << "[SST] start stage: " << SstGetStageName ( m_eStage ) << " for table '" << m_sTable << "'";
		LogProgress ( true );
	}
}

void SstProgress_c::Finish()
{
	FinishPhase();
	m_eStage = SstStage_e::TOTAL;
	StopPushUpdates();

	// log the final 100% progress
	if ( LOG_LEVEL_SST_DBG )
	{
		SST_DBG << "[SST] operation finished";
		LogProgress ( true );
	}
}

void SstProgress_c::SetTable ( const CSphString & sName )
{
	FinishPhase();
	
	m_sTable = sName;
	m_iTable = m_dTables.GetFirst( [&] ( const auto & tItem ) { return tItem.m_sName==m_sTable; } );

	if ( LOG_LEVEL_SST_DBG && m_iTable!=-1 )
		SST_DBG << "[SST] processing table '" << m_sTable << "'";
}

void SstProgress_c::FinishPhase()
{
	if ( m_eStage==SstStage_e::TOTAL || m_iTable==-1 )
		return;

	SstStageProgress_t & tCurStage = GetStage();
	tCurStage.m_uCompletedUnits = tCurStage.m_uTotalUnits;

	if ( !tCurStage.m_tmStarted )
		return;

	if ( !tCurStage.m_tDelta )
		tCurStage.m_tDelta = sphMicroTimer() - tCurStage.m_tmStarted;
}

SstStageProgress_t & SstProgress_c::GetStage() const
{
	assert ( m_eStage!=SstStage_e::TOTAL && m_iTable!=-1 );

	SstStageProgress_t & tItem = m_dProgress[m_iTable * (int)SstStage_e::TOTAL + (int)m_eStage];
	return tItem;
}

SstStageProgress_t * SstProgress_c::GetTableStage ( int iTable ) const
{
	assert ( iTable>=0 && iTable<m_dTables.GetLength() );
	return m_dProgress.Begin() + ( iTable * (int)SstStage_e::TOTAL );
}

void SstProgress_c::SetTables ( const VecTraits_T<TableStat_t> & dTables, int iAttempt, int iAgents )
{
	m_iAgents = iAgents;
	m_iAttempt = iAttempt;

	m_dTables.CopyFrom ( dTables );
	m_dProgress.Reset ( m_dTables.GetLength() * (int)SstStage_e::TOTAL );

	uint64_t uTotalBytes = 0;
	for ( const auto & tStat : m_dTables )
		uTotalBytes += tStat.m_uTotalSize;

	for ( int iTable=0; iTable<m_dTables.GetLength(); iTable++ )
	{
		SstStageProgress_t * pTableStats = GetTableStage ( iTable );
		TableStat_t & tTableStat = m_dTables[iTable];

		tTableStat.m_fWeight = float ( tTableStat.m_uTotalSize ) / uTotalBytes;

		pTableStats[(int)SstStage_e::WAIT_NODES].m_eName = SstStage_e::WAIT_NODES;
		pTableStats[(int)SstStage_e::WAIT_NODES].m_uTotalUnits = m_iAgents;
		pTableStats[(int)SstStage_e::CALC_SHA1].m_eName = SstStage_e::CALC_SHA1;
		pTableStats[(int)SstStage_e::CALC_SHA1].m_uTotalUnits = tTableStat.m_uTotalSize;
		
		// reserve files iterate files twice
		pTableStats[(int)SstStage_e::RESERVE_FILES].m_eName = SstStage_e::RESERVE_FILES;
		pTableStats[(int)SstStage_e::RESERVE_FILES].m_uTotalUnits = tTableStat.m_uTotalSize * m_iAgents;
		
		auto & tSendStage = pTableStats[(int)SstStage_e::SEND_FILES];
		tSendStage.m_eName = SstStage_e::SEND_FILES;
		tSendStage.m_uTotalUnits = (uint64_t)( tTableStat.m_uTotalSize * ESTIMATED_DIFF_RATIO ) * m_iAgents;
		tSendStage.m_uTotalUnits = Max ( tSendStage.m_uTotalUnits, 1 );

		pTableStats[(int)SstStage_e::ACTIVATE_TABLE].m_eName = SstStage_e::ACTIVATE_TABLE;
		pTableStats[(int)SstStage_e::ACTIVATE_TABLE].m_uTotalUnits = m_iAgents;
		
		RecalculateWeights ( iTable );
	}
}

void SstProgress_c::RecalculateWeights ( int iTable )
{
	if ( iTable==-1 )
		return;

	float fTotal = 0.0f;
	float dWeights[(int)SstStage_e::TOTAL];
	SstStageProgress_t * pTable = GetTableStage ( iTable );

	dWeights[(int)SstStage_e::WAIT_NODES]     = pTable[(int)SstStage_e::WAIT_NODES].m_uTotalUnits > 0 ? g_dStageWeights[(int)SstStage_e::WAIT_NODES] : 0;
	dWeights[(int)SstStage_e::CALC_SHA1]      = pTable[(int)SstStage_e::CALC_SHA1].m_uTotalUnits * g_dStageWeights[(int)SstStage_e::CALC_SHA1];
	dWeights[(int)SstStage_e::RESERVE_FILES]  = pTable[(int)SstStage_e::RESERVE_FILES].m_uTotalUnits * g_dStageWeights[(int)SstStage_e::RESERVE_FILES];
	dWeights[(int)SstStage_e::SEND_FILES]     = pTable[(int)SstStage_e::SEND_FILES].m_uTotalUnits * g_dStageWeights[(int)SstStage_e::SEND_FILES];
	dWeights[(int)SstStage_e::ACTIVATE_TABLE] = pTable[(int)SstStage_e::ACTIVATE_TABLE].m_uTotalUnits * g_dStageWeights[(int)SstStage_e::ACTIVATE_TABLE];

	for ( float fWeight : dWeights )
		fTotal += fWeight;

	if ( fTotal>0.0f )
	{
		for ( int i=0; i<(int)SstStage_e::TOTAL; ++i )
			pTable[i].m_fWeight = dWeights[i] / fTotal;
	}
}

float SstStageProgress_t::GetRaw() const
{
	if ( m_uTotalUnits==0 )
		return ( m_uCompletedUnits > 0 ? 1.0f : 0.0f );
	
	return Min ( 1.0f, (float)m_uCompletedUnits / m_uTotalUnits );
}

float SstStageProgress_t::GetTotal() const
{
	return GetRaw() * m_fWeight;
}

float SstProgress_c::GetTotal() const
{
	if ( m_dTables.IsEmpty() )
		return 0.0f;

	float fAll = 0.0f;
	for ( int iTable=0; iTable<m_dTables.GetLength(); iTable++ )
	{
		SstStageProgress_t * pTable = GetTableStage ( iTable );
		float fTable = 0.0f;
		for ( int iStage=0; iStage<(int)SstStage_e::TOTAL; iStage++ )
			fTable += pTable[iStage].GetTotal();

		fAll += fTable * m_dTables[iTable].m_fWeight;
	}
	
	return fAll;
}

float SstProgress_c::GetCurrentStageTotal() const
{
	if ( m_eStage==SstStage_e::TOTAL || m_dTables.IsEmpty() )
		return 0.0f;

	uint64_t uCompletedUnits = 0;
	uint64_t uTotalUnits = 0;

	for ( int iTable=0; iTable<m_dTables.GetLength(); iTable++ )
	{
		const SstStageProgress_t & tStageProgress = m_dProgress[iTable * (int)SstStage_e::TOTAL + (int)m_eStage];
		uCompletedUnits += tStageProgress.m_uCompletedUnits;
		uTotalUnits += tStageProgress.m_uTotalUnits;
	}

	if ( uTotalUnits==0 )
		return ( uCompletedUnits>0 ? 1.0f : 0.0f );

	return Min ( 1.0f, (float)uCompletedUnits/uTotalUnits );
}

void SstProgress_c::GetStatus( SstProgressStatus_t & tStatus ) const
{
	tStatus = SstProgressStatus_t();
	if ( m_eRole==Role_e::DONOR )
	{
		tStatus.m_fTotal = GetTotal();
		tStatus.m_eCurrentStage = GetCurrentStage();
		tStatus.m_fCurrentStageTotal = GetCurrentStageTotal();

		tStatus.m_iTablesCount = m_dTables.GetLength();
		if ( m_iTable!=-1 )
		{
			tStatus.m_iTable = m_iTable;
			tStatus.m_sTable = m_sTable;
		}

	} else if ( m_eRole==Role_e::JOINER )
	{
		// joiner with active local stage
		// or joinr with remote polled stage
		if ( m_eStage!=SstStage_e::TOTAL )
		{
			float fLocal = 0.0f;
			if ( m_tLocalCtx.m_uStageTotal>0 )
				fLocal = (float)m_uLocalCompleted / m_tLocalCtx.m_uStageTotal;
			
			tStatus.m_fTotal = m_tLocalCtx.m_fTotalBefore + ( Min ( 1.0f, fLocal) * m_tLocalCtx.m_fStageWeight );
			tStatus.m_eCurrentStage = m_eStage;
			tStatus.m_fCurrentStageTotal = fLocal;

			tStatus.m_iTablesCount = m_tRemoteStatus.m_iTablesCount;
			tStatus.m_iTable = m_tRemoteStatus.m_iTable;
			tStatus.m_sTable = m_tRemoteStatus.m_sTable;
		} else
		{
			tStatus = m_tRemoteStatus;
		}
	}
}

const char * SstGetStageName ( SstStage_e eStage )
{
	switch ( eStage )
	{
		case SstStage_e::WAIT_NODES:		return "await nodes sync"; break;
		case SstStage_e::CALC_SHA1:			return "block checksum calculate"; break;
		case SstStage_e::RESERVE_FILES:		return "analyze remote"; break;
		case SstStage_e::SEND_FILES:		return "send files"; break;
		case SstStage_e::ACTIVATE_TABLE:	return "activate tables"; break;

		default: break;
	}

	return "unknown";
}

void PrepareSendStage ( const SyncSrc_t & tSigSrc, const VecTraits_T<AgentConn_t*> & dNodes, SstProgress_i & tProgress )
{
	uint64_t uTotal = 0;

	for ( const auto & pNode : dNodes )
	{
		const FileReserveReply_t & tRes = ClusterFileReserve_c::GetRes ( *pNode );
		uTotal += tSigSrc.CalculateNeededBytes( tRes.m_dNodeChunksMask );
	}

	tProgress.StageBegin ( SstStage_e::SEND_FILES );
	tProgress.UpdateTotal ( uTotal );
}

// joiner

void SstProgress_c::UpdateFromRemote ( const SstProgressStatus_t & tStatus )
{
	assert ( m_eRole==Role_e::JOINER );

   if ( LOG_LEVEL_SST_DBG && m_tRemoteStatus.m_eCurrentStage!=tStatus.m_eCurrentStage )
	{
		SST_DBG << "[SST] received new stage from donor: " << SstGetStageName ( tStatus.m_eCurrentStage );
		LogProgress ( true );
	}

	m_tRemoteStatus = tStatus;
}

void SstProgress_c::StartLocalStage ( const SstProgressContext_t & tCtx, SstStage_e eStage )
{
	assert ( m_eRole==Role_e::JOINER );
	m_tLocalCtx = tCtx;
	m_eStage = eStage;
	// reset the counter for the new stage
	m_uLocalCompleted = 0;

	if ( LOG_LEVEL_SST_DBG )
	{
		SST_DBG << "[SST] start local stage: " << SstGetStageName ( eStage );
		LogProgress(true);
	}
}

void SstProgress_c::FinishLocalStage()
{
	assert ( m_eRole==Role_e::JOINER );

	if ( LOG_LEVEL_SST_DBG )
	{
		LogProgress ( true ); 
		SST_DBG << "[SST] finished local stage: " << SstGetStageName ( m_eStage );
	}

	m_eStage = SstStage_e::TOTAL;
	m_uLocalCompleted = 0;
}

void SstProgress_c::Init ( Role_e eRole )
{
	m_eRole = eRole;

	m_sTable = "";
	m_iTable = -1;
	m_eStage = SstStage_e::TOTAL;
	m_iAgents = 0;
	m_iAttempt = 0;
	
	m_tRemoteStatus = SstProgressStatus_t();
	m_tLocalCtx = SstProgressContext_t();
	m_uLocalCompleted = 0;

	m_bDonePushUpdates = true;

	m_dProgress.Reset ( 0 );
	m_dTables.Reset ( 0 );
}

void SstProgress_c::SetDonor4Joiner ( int iFilesCount, SstProgressContext_t & tCtx )
{
	assert ( m_eRole==Role_e::DONOR );
	
	// donor state
	SstProgressStatus_t tStatus;
	GetStatus ( tStatus );

	tCtx.m_fTotalBefore = tStatus.m_fTotal;

	assert ( m_iTable!=-1 );
	const SstStageProgress_t * pTableStages = GetTableStage ( m_iTable );
	// FIXME!!! make it work for all other stages
	const SstStageProgress_t & tReserve = pTableStages[(int)SstStage_e::RESERVE_FILES];
	// normalized weight for the stage
	tCtx.m_fStageWeight = tReserve.m_fWeight;

	tCtx.m_uStageTotal = iFilesCount;
}

void operator<< ( ISphOutputBuffer & tOut, const SstProgressContext_t & tCtx )
{
	tOut.SendFloat ( tCtx.m_fTotalBefore );
	tOut.SendFloat ( tCtx.m_fStageWeight );
	tOut.SendUint64 ( tCtx.m_uStageTotal );
}

void operator>> ( InputBuffer_c & tIn, SstProgressContext_t & tCtx )
{
	tCtx.m_fTotalBefore = tIn.GetFloat();
	tCtx.m_fStageWeight = tIn.GetFloat();
	tCtx.m_uStageTotal = tIn.GetUint64();
}

/////////////////////////////////////////////////////////////////////////////
// donor to node sst progress

struct UpdateSstProgress_t : public ClusterRequest_t
{
	SstProgressStatus_t m_tStatus;
};

using CmdUpdateSstProgress_c = ClusterCommand_T<E_CLUSTER::UPDATE_SST_PROGRESS, UpdateSstProgress_t>;

void operator<< ( ISphOutputBuffer & tOut, const SstProgressStatus_t & tStatus )
{
	tOut.SendFloat ( tStatus.m_fTotal );
	tOut.SendByte ( (BYTE)tStatus.m_eCurrentStage );
	tOut.SendFloat ( tStatus.m_fCurrentStageTotal );
	tOut.SendInt ( tStatus.m_iTablesCount );
	tOut.SendInt ( tStatus.m_iTable );
	tOut.SendString ( tStatus.m_sTable.cstr() );
}

void operator>> ( InputBuffer_c & tIn, SstProgressStatus_t & tStatus )
{
	tStatus.m_fTotal = tIn.GetFloat();
	tStatus.m_eCurrentStage = (SstStage_e)tIn.GetByte();
	tStatus.m_fCurrentStageTotal = tIn.GetFloat();
	tStatus.m_iTablesCount = tIn.GetInt();
	tStatus.m_iTable = tIn.GetInt();
	tStatus.m_sTable = tIn.GetString();
}

void operator<< ( ISphOutputBuffer & tOut, const UpdateSstProgress_t & tReq )
{
	tOut << (const ClusterRequest_t &)tReq;
	tOut << tReq.m_tStatus;
}

void operator>> ( InputBuffer_c & tIn, UpdateSstProgress_t & tReq )
{
	tIn >> (ClusterRequest_t &)tReq;
	tIn >> tReq.m_tStatus;
}

static bool SendSstProgress ( const CSphString & sCluster, const VecTraits_T<AgentDesc_t> & dDesc, const SstProgressStatus_t & tStatus )
{
	UpdateSstProgress_t tReqData;
	tReqData.m_tStatus = tStatus;
	tReqData.m_sCluster = sCluster;

	CmdUpdateSstProgress_c tReq;
	auto dNodes = tReq.MakeAgents ( dDesc, ReplicationTimeoutQuery(), tReqData );
	return PerformRemoteTasksWrap ( dNodes, tReq, tReq, true );
}

void ReceiveSstProgress ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster )
{
	UpdateSstProgress_t tCmd;
	CmdUpdateSstProgress_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;

	auto pProgress = GetClusterProgress ( sCluster );
	if ( pProgress )
	{
		pProgress->UpdateFromRemote ( tCmd.m_tStatus );
		CmdUpdateSstProgress_c::BuildReply ( tOut );
	} else
	{
		TlsMsg::Err ( "%s missed cluster", sCluster.cstr() );
		return;

	}
}

static void CoPushSstProgress(  const CSphString & sCluster, const CSphVector<AgentDesc_t> & dDesc, CSphRefcountedPtr<SstProgress_i> pProgress )
{
	while ( !pProgress->IsPushUpdateDone() )
	{
		SstProgressStatus_t tStatus;
		pProgress->GetStatus ( tStatus );

		SendSstProgress ( sCluster, dDesc, tStatus );
		if ( pProgress->IsPushUpdateDone() )
			break;

		Threads::Coro::SleepMsec ( g_iSstPushIntervalMs );
	}
}

void SstProgress_i::StartPushUpdates ( const CSphString & sCluster, const VecTraits_T<AgentDesc_t> & dDesc, CSphRefcountedPtr<SstProgress_i> pProgress )
{
	SstProgress_c * pProgressImpl = static_cast<SstProgress_c *> ( pProgress.Ptr() );
	assert( pProgressImpl->m_eRole==Role_e::DONOR );
	pProgressImpl->m_bDonePushUpdates = false;

	CSphVector<AgentDesc_t> dDescCopy;
	dDescCopy.Append ( dDesc );

	Threads::Coro::Go ( [sCluster, dDesc = std::move ( dDescCopy ), pProgress] { CoPushSstProgress( sCluster, dDesc, pProgress ); }, Threads::Coro::CurrentScheduler() );
}

void SstProgress_c::StopPushUpdates ()
{
	m_bDonePushUpdates = true;
}

/////////////////////////////////////////////////////////////////////////////
// debug log 

void SstProgress_c::GetCurrentStageStats ( SstStageProgress_t & tStats ) const
{
	tStats = SstStageProgress_t();
	tStats.m_eName = m_eStage;

	if ( m_eStage==SstStage_e::TOTAL || m_dTables.IsEmpty() )
		return;

	for ( int iTable=0; iTable<m_dTables.GetLength(); iTable++ )
	{
		const SstStageProgress_t & tStageProgress = m_dProgress[iTable * (int)SstStage_e::TOTAL + (int)m_eStage];
		tStats.m_uCompletedUnits += tStageProgress.m_uCompletedUnits;
		tStats.m_uTotalUnits += tStageProgress.m_uTotalUnits;
	}
}

void SstProgress_c::LogProgress ( bool bForce )
{
	if ( !LOG_LEVEL_SST_DBG )
		return;

	if ( !bForce && !sph::TimeExceeded ( m_tmLastLog + LOG_SST_PROGRESS_INTERVAL_MS * 1000 ) )
		return;

	m_tmLastLog = sph::LastTimestamp();

	SstProgressStatus_t tStatus;
	GetStatus ( tStatus );
	if ( tStatus.m_eCurrentStage==SstStage_e::TOTAL )
		return;

	SstStageProgress_t tStageStatus;
	GetCurrentStageStats ( tStageStatus );

	SST_DBG << "[SST] overall: " << (int)( tStatus.m_fTotal * 100.0f ) << ", stage (" << SstGetStageName ( tStatus.m_eCurrentStage ) << "): " << (int)( tStatus.m_fCurrentStageTotal * 100.0f ) << " (" << tStageStatus.m_uCompletedUnits << " / " << tStageStatus.m_uTotalUnits << ")";
}

