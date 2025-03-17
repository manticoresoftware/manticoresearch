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


#include "cluster_file_reserve.h"
#include "cluster_commands.h"

#include "api_command_cluster.h"
#include "searchdreplication.h"
#include "digest_sha1.h"
#include "send_files.h"
#include "recv_state.h"

#include <cmath>

void GetArray ( StrVec_t& dBuf, InputBuffer_c& tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	for ( CSphString& sVal : dBuf )
		sVal = tIn.GetString();
}

void GetArray ( FixedStrVec_t& dBuf, InputBuffer_c& tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Reset ( iCount );
	for ( CSphString& sVal : dBuf )
		sVal = tIn.GetString();
}

template<typename T>
void GetArray ( CSphFixedVector<T>& dBuf, InputBuffer_c& tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Reset ( iCount );
	tIn.GetBytes ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
}

template<typename T>
void GetArray ( CSphVector<T>& dBuf, InputBuffer_c& tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	tIn.GetBytes ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
}


template<typename T>
void SendArray ( const VecTraits_T<T>& dBuf, ISphOutputBuffer& tOut )
{
	tOut.SendInt ( dBuf.GetLength() );
	if ( dBuf.GetLength() )
		tOut.SendBytes ( dBuf.Begin(), dBuf.GetLengthBytes() );
}

void SendArray ( const VecTraits_T<CSphString>& dBuf, ISphOutputBuffer& tOut )
{
	tOut.SendInt ( dBuf.GetLength() );
	for ( const CSphString& sVal : dBuf )
		tOut.SendString ( sVal.cstr() );
}

void operator<< ( ISphOutputBuffer& tOut, const FileReserveRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&) tReq;
	tOut.SendString ( tReq.m_sIndex.cstr() );
	tOut.SendString ( tReq.m_sIndexFileName.cstr() );

	assert ( tReq.m_pChunks );
	const SyncSrc_t* pSrc = tReq.m_pChunks;
	SendArray ( pSrc->m_dBaseNames, tOut );
	SendArray ( pSrc->m_dChunks, tOut );
	SendArray ( pSrc->m_dHashes, tOut );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const FileReserveRequest_t& tReq )
{
	const SyncSrc_t* pSrc = tReq.m_pChunks;
	tOut << (const ClusterRequest_t&)tReq
		<< "index" << tReq.m_sIndex << "indexfilename" << tReq.m_sIndexFileName
		<< "basenames:" << pSrc->m_dBaseNames.GetLength()
		<< "chunks:" << pSrc->m_dChunks.GetLength()
		<< "hashes:" << pSrc->m_dHashes.GetLength();
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, FileReserveRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tReq.m_sIndex = tIn.GetString();
	tReq.m_sIndexFileName = tIn.GetString();

	assert ( tReq.m_pChunks );
	SyncSrc_t* pSrc = tReq.m_pChunks;
	GetArray ( pSrc->m_dBaseNames, tIn );
	GetArray ( pSrc->m_dChunks, tIn );
	GetArray ( pSrc->m_dHashes, tIn );
}


void operator<< ( ISphOutputBuffer& tOut, const FileReserveReply_t& tReq )
{
	tOut.SendByte ( tReq.m_bIndexActive );
	SendArray ( tReq.m_dRemotePaths, tOut );
	tOut.SendInt ( tReq.m_dNodeChunksMask.GetSize() );
	tOut.SendBytes ( tReq.m_dNodeChunksMask.Begin(), tReq.m_dNodeChunksMask.GetSizeBytes() );
	tOut.SendUint64 ( tReq.m_tmTimeout );
	tOut.SendUint64 ( tReq.m_tmTimeoutFile );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const FileReserveReply_t& tReq )
{
	tOut << "index active:" << (tReq.m_bIndexActive?"yes":"no")
		<< "remote paths:" << tReq.m_dRemotePaths
		<< "mask size:" << tReq.m_dNodeChunksMask.GetSize()
		<< "timeout:"<< tReq.m_tmTimeout
		<< "timeout file:" << tReq.m_tmTimeoutFile;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, FileReserveReply_t& tReq )
{
	tReq.m_bIndexActive = !!tIn.GetByte();
	GetArray ( tReq.m_dRemotePaths, tIn );
	int iBits = tIn.GetInt();
	tReq.m_dNodeChunksMask.Init ( iBits );
	tIn.GetBytes ( tReq.m_dNodeChunksMask.Begin(), tReq.m_dNodeChunksMask.GetSizeBytes() );
	tReq.m_tmTimeout = (int64_t)tIn.GetUint64();
	tReq.m_tmTimeoutFile = (int64_t)tIn.GetUint64();
}


struct ScopedFilesRemoval_t: public ISphNoncopyable
{
	explicit ScopedFilesRemoval_t ( VecTraits_T<CSphString>* pFiles )
		: m_pFiles ( pFiles )
	{}
	~ScopedFilesRemoval_t()
	{
		if ( m_pFiles )
		{
			for ( const CSphString& sFile : *m_pFiles )
			{
				if ( !sFile.IsEmpty() && sphFileExists ( sFile.cstr() ) )
					::unlink ( sFile.cstr() );
			}
		}
	}
	VecTraits_T<CSphString>* m_pFiles { nullptr };
};

// command at remote node for CLUSTER_FILE_RESERVE to check
// - file could be allocated on disk at cluster path and reserve disk space for a file
// - or make sure that index has exact same index file, ie sha1 matched
bool ClusterFileReserve ( const FileReserveRequest_t & tCmd, FileReserveReply_t & tRes )
{
	sphLogDebugRpl ( "reserve table '%s'", tCmd.m_sIndex.cstr() );

	int64_t tmStartReserve = sphMicroTimer();
	CSphString sLocalIndexPath;

	assert ( tCmd.m_pChunks );
	// use index path first
	{
		cServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
		if ( ServedDesc_t::IsMutable ( pServed ) )
		{
			tRes.m_bIndexActive = true;
			sLocalIndexPath = pServed->m_sIndexPath;
			RIdx_T<RtIndex_i*> ( pServed )->ProhibitSave();
		}
	}

	tRes.m_dRemotePaths.Resize ( tCmd.m_pChunks->m_dBaseNames.GetLength ());

	// use cluster path as head of index path or existed index path
	if ( tRes.m_bIndexActive )
	{
		CSphString sPathOnly = GetPathOnly ( sLocalIndexPath );
		// set index files names into existing index files
		ARRAY_FOREACH ( iFile, tCmd.m_pChunks->m_dBaseNames )
		{
			const CSphString & sFile = tCmd.m_pChunks->m_dBaseNames[iFile];
			tRes.m_dRemotePaths[iFile].SetSprintf ( "%s%s", sPathOnly.cstr(), sFile.cstr() );
		}
	} else
	{
		auto tIndexPath = GetClusterPath ( tCmd.m_sCluster );
		if ( !tIndexPath )
			return false;
		// index in its own directory
		sLocalIndexPath.SetSprintf ( "%s/%s", tIndexPath->cstr(), tCmd.m_sIndexFileName.cstr() );
		MkDir ( sLocalIndexPath.cstr() );

		// set index files names into cluster folder
		ARRAY_FOREACH ( iFile, tCmd.m_pChunks->m_dBaseNames )
		{
			const CSphString & sFile = tCmd.m_pChunks->m_dBaseNames[iFile];
			tRes.m_dRemotePaths[iFile].SetSprintf ( "%s/%s", sLocalIndexPath.cstr(), sFile.cstr() );
		}
		sLocalIndexPath.SetSprintf ( "%s/%s/%s", tIndexPath->cstr(), tCmd.m_sIndexFileName.cstr(), tCmd.m_sIndexFileName.cstr() );
	}

	int iBits = tCmd.m_pChunks->m_dChunks.Last().m_iHashStartItem + tCmd.m_pChunks->m_dChunks.Last().GetChunksCount();
	tRes.m_dNodeChunksMask.Init ( iBits );

	CSphVector<BYTE> dReadBuf;
	int64_t tmTimeoutFile = 0;

	// check file exists, same size and same hash
	ARRAY_FOREACH ( iFile, tRes.m_dRemotePaths )
	{
		TlsMsg::KeepError_c sError;
		const CSphString & sFile = tRes.m_dRemotePaths[iFile];
		const FileChunks_t & tFile = tCmd.m_pChunks->m_dChunks[iFile];

		if ( sphIsReadable ( sFile ) )
		{
			int64_t iLen = 0;
			{
				CSphAutofile tOut ( sFile, SPH_O_READ, sError, false );
				if ( tOut.GetFD()<0 )
					return false;

				iLen = tOut.GetSize();
			}

			// check only in case size matched
			if ( iLen==tFile.m_iFileSize )
			{
				int64_t tmReadStart = sphMicroTimer();

				if ( !VerifyFileHash ( iFile, sFile, *tCmd.m_pChunks, tRes.m_dNodeChunksMask, dReadBuf, sError ) )
					return false;

				int64_t tmReadDelta = sphMicroTimer() - tmReadStart;
				tmTimeoutFile = Max ( tmReadDelta, tmTimeoutFile );
			}
		}
	}

	StrVec_t dLocalPaths ( tRes.m_dRemotePaths.GetLength() );
	ScopedFilesRemoval_t tFilesCleanup ( &dLocalPaths );

	// create files these will be transferred from donor
	ARRAY_FOREACH ( iFile, tRes.m_dRemotePaths )
	{
		// no need to create file for fully matched file
		if ( tRes.m_dNodeChunksMask.BitGet ( iFile ) )
			continue;

		// file content from donor will be saved into a file with a '.new' extension
		dLocalPaths[iFile].SetSprintf ( "%s.new", tRes.m_dRemotePaths[iFile].cstr() );
		const CSphString& sFile = dLocalPaths[iFile];
		const FileChunks_t& tFile = tCmd.m_pChunks->m_dChunks[iFile];

		// need to create file with specific size
		CSphString sError;
		CSphAutofile tOut ( sFile, SPH_O_NEW, sError, false );
		if ( tOut.GetFD()<0 )
			return TlsMsg::Err ( sError );

		int64_t iLen = sphSeek ( tOut.GetFD(), tFile.m_iFileSize, SEEK_SET );
		if ( iLen!=tFile.m_iFileSize )
		{
			if ( iLen<0 )
				return TlsMsg::Err ( "error: %d '%s'", errno, strerrorm ( errno ) );
			return TlsMsg::Err ( "error, expected: " INT64_FMT ", got " INT64_FMT, tFile.m_iFileSize, iLen );
		}
	}

	tFilesCleanup.m_pFiles = nullptr;
	tRes.m_dRemotePaths.SwapData ( dLocalPaths );
	tRes.m_tmTimeoutFile = tmTimeoutFile / 1000;
	tRes.m_tmTimeout = ( sphMicroTimer() - tmStartReserve ) / 1000;

	assert ( !RecvState::HasState ( DoubleStringKey ( tCmd.m_sCluster, tCmd.m_sIndex ) ) );
	RecvState::GetState ( DoubleStringKey ( tCmd.m_sCluster, tCmd.m_sIndex ) ).SetMerge ( *tCmd.m_pChunks, tRes, sLocalIndexPath, dLocalPaths );
	return true;
}

void ReceiveClusterFileReserve ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString& sCluster )
{
	SyncSrc_t tSrc;
	FileReserveRequest_t tCmd;
	tCmd.m_pChunks = &tSrc;
	ClusterFileReserve_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;

	FileReserveReply_t tRes;
	if ( ClusterFileReserve ( tCmd, tRes ) )
		ClusterFileReserve_c::BuildReply ( tOut, tRes );
}


bool SendClusterFileReserve ( VecRefPtrs_t<AgentConn_t*>& dAgents )
{
	if ( dAgents.IsEmpty() )
		return false;

	ClusterFileReserve_c tReq;
	return PerformRemoteTasksWrap ( dAgents, tReq, tReq, true );
}
