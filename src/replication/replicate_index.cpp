//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "replicate_index.h"

#include "std/string.h"
#include "std/vector.h"
#include "fileio.h"

#include "api_command_cluster.h"
#include "configuration.h"
#include "cluster_file_reserve.h"
#include "cluster_file_send.h"
#include "cluster_index_add_local.h"

#include <cmath>

// for send dist indexes
#include "nodes.h"

class IndexSaveGuard_c: public ISphNoncopyable
{
public:
	explicit IndexSaveGuard_c ( cServedIndexRefPtr_c pIndexDesc )
		: m_pServedIndex ( std::move ( pIndexDesc ) )
	{}

	void EnableSave()
	{
		if ( m_pServedIndex )
			RIdx_T<RtIndex_i*> ( m_pServedIndex )->EnableSave();

		m_pServedIndex = nullptr;
	}

	~IndexSaveGuard_c()
	{
		EnableSave();
	}

private:
	cServedIndexRefPtr_c m_pServedIndex;
};

static bool ActivateIndexOnRemotes ( const CSphString& sCluster, const CSphString& sIndex, IndexType_e eType, bool bSendOk, const VecTraits_T<const AgentDesc_t*>& dActivateIndexes, int64_t tmLongOpTimeout )
{
	// send a command to activate transferred index
	ClusterIndexAddLocalRequest_t tAddLocal;
	tAddLocal.m_sCluster = sCluster;
	tAddLocal.m_sIndex = sIndex;
	tAddLocal.m_eIndex = eType;
	tAddLocal.m_bSendFilesSuccess = bSendOk;

	VecRefPtrs_t<AgentConn_t*> dNodes;
	dNodes.Resize ( dActivateIndexes.GetLength() );
	ARRAY_FOREACH ( i, dActivateIndexes )
	{
		const AgentDesc_t& tDesc = *dActivateIndexes[i];
		dNodes[i] = ClusterIndexAddLocal_c::CreateAgent ( tDesc, ReplicationTimeoutQuery(), tAddLocal );
	}

	sphLogDebugRpl ( "sent table '%s' %s to %d nodes with timeout %d.%03d sec", sIndex.cstr(), ( bSendOk ? "loading" : "rollback" ), dNodes.GetLength(), (int)( tmLongOpTimeout / 1000 ), (int)( tmLongOpTimeout % 1000 ) );

	ClusterIndexAddLocal_c tReq;
	if ( !PerformRemoteTasksWrap ( dNodes, tReq, tReq, true ) )
		return false;

	sphLogDebugRpl ( "remote table '%s' %s", sIndex.cstr(), ( bSendOk ? "added" : "rolled-back" ) );
	return true;
}

bool SyncSrc_t::CalculateFilesSignatures()
{
	int64_t tmStart = sphMicroTimer();
	TLS_MSG_STRING ( sError );

	auto iMaxChunkBytes = InitSyncSrc();
	if ( !iMaxChunkBytes.has_value() )
		return false;

	SHA1_c tHashFile;
	SHA1_c tHashChunk;
	const int iFiles = m_dIndexFiles.GetLength();

	CSphFixedVector<BYTE> dReadBuf { iMaxChunkBytes.value() };

	for ( int iFile = 0; iFile < iFiles; ++iFile )
	{
		int64_t tmStartFile = sphMicroTimer();

		const CSphString& sFile = m_dIndexFiles[iFile];
		const FileChunks_t& tChunk = m_dChunks[iFile];

		CSphAutofile tIndexFile;
		if ( tIndexFile.Open ( sFile, SPH_O_READ, sError ) < 0 )
			return false;

		tHashFile.Init();

		int iChunk = 0;
		int64_t iReadTotal = 0;
		while ( iReadTotal < tChunk.m_iFileSize )
		{
			int64_t iLeftTotal = tChunk.m_iFileSize - iReadTotal;
			int64_t iLeft = Min ( iLeftTotal, tChunk.m_iChunkBytes );
			iReadTotal += iLeft;

			if ( !tIndexFile.Read ( dReadBuf.Begin(), iLeft, sError ) )
				return false;

			// update whole file hash
			tHashFile.Update ( dReadBuf.Begin(), iLeft );

			// update and flush chunk hash
			tHashChunk.Init();
			tHashChunk.Update ( dReadBuf.Begin(), iLeft );
			tHashChunk.Final ( GetChunkHash ( iFile, iChunk ) );
			++iChunk;
		}

		tIndexFile.Close();
		tHashFile.Final ( GetFileHash ( iFile ) );

		int64_t tmDeltaFile = ( sphMicroTimer() - tmStartFile ) / 1000;
		m_tmTimeoutFile = Max ( tmDeltaFile, m_tmTimeoutFile );
	}

	m_tmTimeout = Min ( ( sphMicroTimer() - tmStart ) / 1000, 300000 ); // long operation timeout but at least 5 minutes
	return true;
}

// send local index to remote nodes via API
bool ReplicateIndexToNodes ( const CSphString& sCluster, const CSphString& sIndex, const VecTraits_T<AgentDesc_t>& dDesc, const cServedIndexRefPtr_c& pServedIndex )
{
	assert ( !dDesc.IsEmpty ());

	StrVec_t dIndexFiles;
	IndexSaveGuard_c tIndexSaveGuard ( pServedIndex );
	RIdx_T<RtIndex_i*> pIndex { pServedIndex };
	pIndex->LockFileState ( dIndexFiles );

	CSphString sIndexPath = pServedIndex->m_sIndexPath;
	IndexType_e eType = pServedIndex->m_eType;

	assert ( !sIndexPath.IsEmpty ());
	assert ( !dIndexFiles.IsEmpty ());

	sphLogDebugRpl ( "calculate sha1 of table files chunks '%s'", sIndex.cstr() );

	SyncSrc_t tSigSrc { std::move ( dIndexFiles ) };
	if ( !tSigSrc.CalculateFilesSignatures () )
		return false;

	sphLogDebugRpl ( "calculated sha1 of table '%s', files %d, hashes %d", sIndex.cstr(), tSigSrc.m_dIndexFiles.GetLength(), tSigSrc.m_dHashes.GetLength() );

	int64_t tmLongOpTimeout = ReplicationTimeoutQuery ( tSigSrc.m_tmTimeout * 3 ); // timeout = sha verify (of all index files) + preload (of all index files) +1 (for slow io)

	FileReserveRequest_t tRequest;
	tRequest.m_sCluster = sCluster;
	tRequest.m_sIndex = sIndex;
	tRequest.m_pChunks = &tSigSrc;
	tRequest.m_sIndexFileName = GetBaseName ( sIndexPath );
	auto dNodes = ClusterFileReserve_c::MakeAgents ( dDesc, tmLongOpTimeout, tRequest );
	assert ( dDesc.GetLength() == dNodes.GetLength() );
	auto bOk = SendClusterFileReserve ( dNodes );

	sphLogDebugRpl ( "reserved table '%s' - %s", sIndex.cstr(), ( bOk ? "ok" : "failed" ) );
	if ( !bOk )
		return false;

	CSphVector<RemoteFileState_t> dSendStates;
	CSphVector<const AgentDesc_t*> dActivateIndexes;

	// collect remote file states and make list nodes and files to send
	auto & sErr = TlsMsg::Err ();
	sErr.StartBlock ( "; " );
	ARRAY_FOREACH ( iNode, dNodes )
	{
		FileReserveReply_t & tRes = ClusterFileReserve_c::GetRes ( *dNodes[iNode] );
		const CSphBitvec & tFilesDstMask = tRes.m_dNodeChunksMask;
		if ( tSigSrc.m_dBaseNames.GetLength() != tRes.m_dRemotePaths.GetLength() && tSigSrc.m_dHashes.GetLength() != tFilesDstMask.GetSize() )
		{
			sErr.Sprintf ( "'%s:%d' wrong stored files %d (expected %d), hashes %d (expected %d)",
					dNodes[iNode]->m_tDesc.m_sAddr.cstr (), dNodes[iNode]->m_tDesc.m_iPort,
					tRes.m_dRemotePaths.GetLength(), tSigSrc.m_dBaseNames.GetLength (),
					tRes.m_dNodeChunksMask.GetSize(), tSigSrc.m_dHashes.GetLength () );
			continue;
		}

		tSigSrc.m_tmTimeout = Max ( tSigSrc.m_tmTimeout, tRes.m_tmTimeout );
		tRes.m_tmTimeoutFile = ReplicationTimeoutQuery ( Max ( tSigSrc.m_tmTimeoutFile, tRes.m_tmTimeoutFile ) * 3 );

		bool bFilesMatched = true;
		for ( int iFile = 0; bFilesMatched && iFile < tSigSrc.m_dBaseNames.GetLength(); ++iFile )
			bFilesMatched &= tFilesDstMask.BitGet ( iFile );

		if ( bFilesMatched && tRes.m_bIndexActive )
			continue;

		RemoteFileState_t tRemoteState;
		tRemoteState.m_pAgentDesc = &dDesc[iNode];
		tRemoteState.m_pSyncSrc = &tSigSrc;
		tRemoteState.m_pSyncDst = &tRes;

		// no need to send index files to nodes there files matches exactly
		if ( !bFilesMatched )
			dSendStates.Add ( tRemoteState );

		// after file send need also to re-activate index with new files
		dActivateIndexes.Add ( &dDesc[iNode] );
	}
	sErr.FinishBlock ();

	// recalculate timeout after nodes reports
	tmLongOpTimeout = ReplicationTimeoutQuery ( tSigSrc.m_tmTimeout * 3 );

	if ( dSendStates.IsEmpty() && dActivateIndexes.IsEmpty() )
		return true;

	sphLogDebugRpl ( "sending table '%s'", sIndex.cstr() );

	bool bSendOk = true;
	if ( !dSendStates.IsEmpty() )
		bSendOk = RemoteClusterFileSend ( tSigSrc, dSendStates, sCluster, sIndex );

	// allow index local write operations passed without replicator
	tIndexSaveGuard.EnableSave ();

	return ActivateIndexOnRemotes ( sCluster, sIndex, eType, bSendOk, dActivateIndexes, tmLongOpTimeout ) && bSendOk;
}

struct DistIndexSendRequest_t : public ClusterRequest_t
{
	DistIndexSendRequest_t() = default;
	DistIndexSendRequest_t ( const DistributedIndex_t & tDistIndex, const CSphString & sCluster, const CSphString & sIndex )
	{
		m_sCluster = sCluster;
		m_sIndex = sIndex;

		JsonEscapedBuilder sDesc;
		IndexDescDistr_t tDesc = GetDistributedDesc ( tDistIndex );
		tDesc.Save ( sDesc );

		sDesc.MoveTo ( m_sDesc );
	}

	CSphString m_sIndex;
	CSphString m_sDesc;
};

void operator<< ( ISphOutputBuffer & tOut, const DistIndexSendRequest_t & tReq )
{
	tOut.SendString ( tReq.m_sCluster.cstr() );
	tOut.SendString ( tReq.m_sIndex.cstr() );
	tOut.SendString ( tReq.m_sDesc.cstr() );
}

void operator>> ( InputBuffer_c & tIn, DistIndexSendRequest_t & tReq )
{
	tReq.m_sCluster = tIn.GetString();
	tReq.m_sIndex = tIn.GetString();
	tReq.m_sDesc = tIn.GetString();
}

using ClusterSendDistIndex_c = ClusterCommand_T<E_CLUSTER::INDEX_ADD_DIST, DistIndexSendRequest_t>;

// send distributed index to remote nodes via API
bool ReplicateDistIndexToNodes ( const CSphString & sCluster, const CSphString & sIndex, const VecTraits_T<AgentDesc_t> & dDesc )
{
	cDistributedIndexRefPtr_t pDist ( GetDistr ( sIndex ) );
	if ( !pDist )
	{
		TlsMsg::Err() << "unknown or wrong type of table '" << sIndex << "'";
		return false;
	}

	ClusterSendDistIndex_c tReq;
	DistIndexSendRequest_t tSend ( *pDist, sCluster, sIndex );

	int64_t tmTimeout = ReplicationTimeoutQuery();
	auto dNodes = ClusterSendDistIndex_c::MakeAgents ( dDesc, tmTimeout, tSend );

	sphLogDebugRpl ( "sending table '%s' to %d nodes with timeout %d.%03d sec", sIndex.cstr(), dNodes.GetLength(), (int)( tmTimeout / 1000 ), (int)( tmTimeout % 1000 ) );

	return PerformRemoteTasksWrap ( dNodes, tReq, tReq, true );
}

static bool AddDistIndex ( const DistIndexSendRequest_t & tCmd )
{
	TLS_MSG_STRING ( sError );

	cDistributedIndexRefPtr_t pDist ( GetDistr ( tCmd.m_sIndex ) );
	if ( pDist && !pDist->m_sCluster.IsEmpty() )
		return TlsMsg::Err ( "distributed table '%s:%s' is already the part of the cluster %s, remove it first", tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), pDist->m_sCluster.cstr() );

	CSphVector<BYTE> dBsonParsed;
	if ( !sphJsonParse ( dBsonParsed, (char *)tCmd.m_sDesc.cstr(), false, false, false, sError ) )
		return false;

	using namespace bson;
	Bson_c tBson ( dBsonParsed );
	if ( tBson.IsEmpty() || !tBson.IsAssoc() )
		return TlsMsg::Err ( "bad json for distributed table '%s:%s': %s", tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), tCmd.m_sDesc.cstr() );

	CSphString sWarning;
	IndexDesc_t tIndexDesc;
	if ( !tIndexDesc.Parse ( tBson, tCmd.m_sIndex, sWarning ) )
		return false;

	if ( !sWarning.IsEmpty() )
		sphWarning ( "table '%s' create warning: %s", tCmd.m_sIndex.cstr(), sWarning.cstr() );

	CSphConfigSection hConf;
	tIndexDesc.Save ( hConf );

	StrVec_t dWarnings;
	DistributedIndexRefPtr_t pIdx ( new DistributedIndex_t );
	bool bOk = ConfigureDistributedIndex ( []( const auto & sIdx ){ return true; }, *pIdx, tCmd.m_sIndex.cstr(), hConf, sError, &dWarnings );
	for ( const CSphString & sMsg : dWarnings )
		sphWarning ( "distributed table '%s:%s': %s", tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), sMsg.cstr() );
	if ( !bOk || pIdx->IsEmpty() )
	{
		TlsMsg::Err ( "failed to create distributed table '%s:%s': %s", tCmd.m_sCluster.cstr(), tCmd.m_sIndex.cstr(), sError.cstr() );
		sError = "";
		return false;
	}

	// finally, check and add a new or replace an existed distributed index to global table
	g_pDistIndexes->AddOrReplace ( pIdx, tCmd.m_sIndex );

	return true;
}

void ReceiveDistIndex ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster )
{
	ClusterSendDistIndex_c tReq;
	DistIndexSendRequest_t tDistCmd;

	ClusterSendDistIndex_c::ParseRequest ( tBuf, tDistCmd );
	sCluster = tDistCmd.m_sCluster;
	if ( AddDistIndex ( tDistCmd ) )
		ClusterSendDistIndex_c::BuildReply ( tOut );
}