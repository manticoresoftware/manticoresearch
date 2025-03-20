//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "cluster_index_add_local.h"
#include "api_command_cluster.h"
#include "cluster_commands.h"

#include "digest_sha1.h"
#include "recv_state.h"

#include <cmath>


void operator<< ( ISphOutputBuffer& tOut, const ClusterIndexAddLocalRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&) tReq;
	tOut.SendString ( tReq.m_sIndex.cstr() );
	tOut.SendByte ( (BYTE)tReq.m_eIndex );
	tOut.SendByte ( tReq.m_bSendFilesSuccess );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterIndexAddLocalRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut << "index:" << tReq.m_sIndex;
	tOut << "type:" << (BYTE)tReq.m_eIndex;
	tOut << "SendFilesSuccess:" << tReq.m_bSendFilesSuccess;
	return tOut;
}

void operator>> ( MemInputBuffer_c& tIn, ClusterIndexAddLocalRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tReq.m_sIndex = tIn.GetString();
	tReq.m_eIndex = (IndexType_e)tIn.GetByte();
	tReq.m_bSendFilesSuccess = !!tIn.GetByte();
}


struct FilesTrait_t
{
	StrVec_t m_dOld;
	StrVec_t m_dRef;

	void Reset()
	{
		m_dOld.Reset();
		m_dRef.Reset();
	}
};

struct ScopedState_t
{
	explicit ScopedState_t ( uint64_t uState )
		: m_uState ( uState )
	{}
	~ScopedState_t() { RecvState::Free ( m_uState ); }
	uint64_t m_uState = 0;
};

struct RollbackFilesGuard_t
{
	RollbackFilesGuard_t() = default;
	~RollbackFilesGuard_t()
	{
		assert ( m_tFiles.m_dOld.GetLength() == m_tFiles.m_dRef.GetLength() );
		ARRAY_FOREACH ( i, m_tFiles.m_dOld )
		{
			if ( sph::rename ( m_tFiles.m_dOld[i].cstr(), m_tFiles.m_dRef[i].cstr() ) != 0 )
				sphWarning ( "rollback rename %s to %s failed, error %s (%d)", m_tFiles.m_dOld[i].cstr(), m_tFiles.m_dRef[i].cstr(), strerrorm ( errno ), errno );
		}
	}

	FilesTrait_t m_tFiles;
};

static void RemoveFiles ( const sph::StringSet& hActiveFiles, const StrVec_t& dFiles )
{
	for ( const auto& sFile : dFiles )
	{
		if ( hActiveFiles[sFile] )
			continue;

		if ( !sphFileExists ( sFile.cstr() ) )
			continue;

		if ( ::unlink ( sFile.cstr() ) )
			sphWarning ( "failed to unlink file %s, error %s (%d)", sFile.cstr(), strerrorm ( errno ), errno );
	}
}

static void RemoveFiles ( const FilesTrait_t& tIndexFiles, const StrVec_t& dRenamedOld )
{
	sph::StringSet hActiveFiles;
	for ( const auto& sFile : tIndexFiles.m_dRef )
		hActiveFiles.Add ( sFile );

	RemoveFiles ( hActiveFiles, tIndexFiles.m_dOld );
	RemoveFiles ( hActiveFiles, dRenamedOld );
}

static bool RotateFiles ( const std::unique_ptr<MergeState_t>& pState, FilesTrait_t& tFilesGuard )
{
	assert ( pState );
	int iMatched = 0;
	for ( int i = 0; i < pState->m_dFilesNew.GetLength(); i++ )
		iMatched += ( pState->m_dMergeMask.BitGet ( i ) ? 1 : 0 );

	// all matched files - no .new files these should be rotated at joiner
	// using joiner files as is
	if ( iMatched == pState->m_dFilesNew.GetLength() )
		return true;

	// rename index files to old
	StrVec_t dFilesRef2Old;
	StrVec_t dFilesOld;

	// rename new to index files
	StrVec_t dFilesNew;
	StrVec_t dFilesNew2Ref;

	for ( int iFile = 0; iFile < pState->m_dFilesNew.GetLength(); iFile++ )
	{
		// keep joiner file as is if whole file matched
		if ( pState->m_dMergeMask.BitGet ( iFile ) )
			continue;

		const CSphString& sNameNew = pState->m_dFilesNew[iFile];
		const CSphString& sNameRef = pState->m_dFilesRef[iFile];

		assert ( sphFileExists ( sNameNew.cstr() ) );
		dFilesNew.Add ( sNameNew );
		dFilesNew2Ref.Add ( sNameRef );

		if ( sphFileExists ( sNameRef.cstr(), nullptr ) )
		{
			dFilesRef2Old.Add ( sNameRef );
			dFilesOld.Add().SetSprintf ( "%s.old", sNameRef.cstr() );
		}
	}

	TLS_MSG_STRING ( sError );
	if ( !RenameWithRollback ( dFilesRef2Old, dFilesOld, sError ) )
		return false;

	if ( !RenameWithRollback ( dFilesNew, dFilesNew2Ref, sError ) )
	{
		RenameFiles ( dFilesOld, dFilesRef2Old, sError );
		return false;
	}

	tFilesGuard.m_dOld.SwapData ( dFilesOld );
	tFilesGuard.m_dRef.SwapData ( dFilesRef2Old );

	return true;
}

// load index from disk files for cluster use
static ServedIndexRefPtr_c LoadNewIndex ( const CSphString & sIndexPath, IndexType_e eIndexType, const char * szIndexName, FilesTrait_t & tIndexFiles )
{
	CSphConfigSection hIndex;
	hIndex.Add ( CSphVariant ( sIndexPath.cstr() ), "path" );
	hIndex.Add ( CSphVariant ( GetIndexTypeName ( eIndexType ) ), "type" );
	// dummy
	hIndex.Add ( CSphVariant ( "text" ), "rt_field" );
	hIndex.Add ( CSphVariant ( "gid" ), "rt_attr_uint" );

	TLS_MSG_STRING ( sError );
	ServedIndexRefPtr_c pResult;
	auto [eAdd, pNewServed] = AddIndex ( szIndexName, hIndex, false, true, nullptr, sError );

	assert ( eAdd == ADD_NEEDLOAD || eAdd == ADD_ERROR );
	if ( eAdd != ADD_NEEDLOAD )
		return pResult;

	assert ( pNewServed );

	StrVec_t dWarnings;
	bool bPrealloc = PreallocNewIndex ( *pNewServed, &hIndex, szIndexName, dWarnings, sError );
	if ( !bPrealloc )
		return pResult;

	UnlockedHazardIdxFromServed ( *pNewServed )->GetIndexFiles ( tIndexFiles.m_dRef, tIndexFiles.m_dRef );
	for ( const auto& i : dWarnings )
		sphWarning ( "table '%s': %s", szIndexName, i.cstr() );

	pResult = std::move ( pNewServed );
	return pResult;
}

// load index into daemon
static bool LoadAndReplaceIndex ( const CSphString& sIndexPath, IndexType_e eIndexType, const CSphString & sIndexName, FilesTrait_t & tIndexFiles )
{
	auto pNewIndex = LoadNewIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );
	if ( !pNewIndex )
		return false;

	g_pLocalIndexes->AddOrReplace ( pNewIndex, sIndexName );
	return true;
}

// load index into daemon
// in case index already exists prohibit it to save on index delete as disk files has fresh data received from remote node
static bool LoadIndex ( const CSphString & sIndexPath, IndexType_e eIndexType, const CSphString & sIndexName, FilesTrait_t & tIndexFiles )
{
	cServedIndexRefPtr_c pOldIndex = GetServed ( sIndexName );
	if ( !ServedDesc_t::IsMutable ( pOldIndex ) )
		return LoadAndReplaceIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );

	WIdx_T<RtIndex_i*> pIndex { pOldIndex };
	pIndex->ProhibitSave();
	pIndex->GetIndexFiles ( tIndexFiles.m_dOld, tIndexFiles.m_dOld );
	return LoadAndReplaceIndex ( sIndexPath, eIndexType, sIndexName.cstr(), tIndexFiles );
}


// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
static bool AddReceivedIndex ( const ClusterIndexAddLocalRequest_t& tAddCmd )
{
	uint64_t uKey = DoubleStringKey ( tAddCmd.m_sCluster, tAddCmd.m_sIndex );
	if ( !RecvState::HasState ( uKey ) )
		return TlsMsg::Err ( "missed writer state at joiner node for cluster '%s' table '%s'", tAddCmd.m_sCluster.cstr(), tAddCmd.m_sIndex.cstr() );

	ScopedState_t tStateGuard ( uKey );
	std::unique_ptr<MergeState_t> pMerge { RecvState::GetState ( uKey ).Flush () };

	if ( tAddCmd.m_eIndex!=IndexType_e::PERCOLATE && tAddCmd.m_eIndex!=IndexType_e::RT )
		return TlsMsg::Err ( "unsupported type '%s' in index '%s'", GetIndexTypeName ( tAddCmd.m_eIndex ), tAddCmd.m_sIndex.cstr() );

	sphLogDebugRpl ( "rotating table '%s' content from %s", tAddCmd.m_sIndex.cstr(), pMerge->m_sIndexPath.cstr() );
	RollbackFilesGuard_t tFilesGuard;
	if ( !RotateFiles ( pMerge, tFilesGuard.m_tFiles ) )
		return false;

	// verify whole index only in case debug replication
#ifndef NDEBUG
	// if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG )
	{
		sphLogDebugRpl ( "verify table '%s' from %s", tAddCmd.m_sIndex.cstr(), pMerge->m_sIndexPath.cstr() );

		// check that size matched and sha1 matched prior to loading index
		if ( !SyncSigVerify ( pMerge->m_dFilesRef, pMerge->m_dHashes ) )
			return false;
	}
#endif

	sphLogDebugRpl ( "%s table '%s' into cluster '%s' from %s", ( tAddCmd.m_bSendFilesSuccess ? "loading" : "rolling-back" ), tAddCmd.m_sIndex.cstr(), tAddCmd.m_sCluster.cstr(), pMerge->m_sIndexPath.cstr() );

	// rollback to old index files via RollbackFilesGuard_t
	if ( !tAddCmd.m_bSendFilesSuccess )
		return true;

	FilesTrait_t tIndexFiles;

	if ( !LoadIndex ( pMerge->m_sIndexPath, tAddCmd.m_eIndex, tAddCmd.m_sIndex, tIndexFiles ) )
		return TlsMsg::Err ( "failed to load index '%s'", tAddCmd.m_sIndex.cstr() );

	if ( !AddLoadedIndexIntoCluster ( tAddCmd.m_sCluster, tAddCmd.m_sIndex ) )
		return false;

	// keep: only files that donor has
	// remove:
	// - index files renamed to .old
	// - the old index files as the old index could have more files when the new index
	RemoveFiles ( tIndexFiles, tFilesGuard.m_tFiles.m_dOld );

	// clean rollback files lists
	tFilesGuard.m_tFiles.Reset();
	return true;
}

// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
void ReceiveClusterIndexAddLocal ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster )
{
	ClusterIndexAddLocalRequest_t tAddCmd;
	ClusterIndexAddLocal_c::ParseRequest ( tBuf, tAddCmd );
	sCluster = tAddCmd.m_sCluster;
	if ( AddReceivedIndex ( tAddCmd ) )
		ClusterIndexAddLocal_c::BuildReply ( tOut );
}