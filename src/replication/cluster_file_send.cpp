//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "cluster_file_send.h"

#include "nodes.h"
#include "recv_state.h"

class SendBuf_c
{
	BYTE* m_pData = nullptr;
	int m_iSize = 0;
	int m_iLeft = 0;

public:
	SendBuf_c() = default;
	SendBuf_c ( BYTE* pData, int iSize )
		: m_pData ( pData )
		, m_iSize ( iSize )
	{}

	[[nodiscard]] inline BYTE* Begin() const noexcept { return m_pData; }
	[[nodiscard]] inline int GetLength() const noexcept { return m_iSize; }
	[[nodiscard]] inline BYTE* Data() const noexcept
	{
		int iOff = m_iSize - m_iLeft;
		return m_pData + iOff;
	}
	[[nodiscard]] inline int Consumed() const noexcept { return m_iSize - m_iLeft; }
	[[nodiscard]] inline int Left() const noexcept { return m_iLeft; }
	inline void Consume ( int iLen ) noexcept
	{
		assert ( iLen <= m_iLeft );
		m_iLeft -= iLen;
	}

	inline void Rewind() { m_iLeft = m_iSize; }
};

struct ClusterFileSendRequest_t
{
	uint64_t m_tWriterKey = 0;
	SendBuf_c m_tSendBuf;
	int m_iFile = 0;
	CSphVector<FileOp_t> m_dOps;
};

void operator<< ( ISphOutputBuffer& tOut, const ClusterFileSendRequest_t& tReq )
{
	tOut.SendUint64 ( tReq.m_tWriterKey );
	tOut.SendInt ( tReq.m_tSendBuf.Consumed() );
	tOut.SendInt ( tReq.m_iFile );
	tOut.SendBytes ( tReq.m_tSendBuf.Begin(), tReq.m_tSendBuf.Consumed() );

	tOut.SendInt ( tReq.m_dOps.GetLength() );
	for ( const FileOp_t& tItem : tReq.m_dOps )
	{
		tOut.SendByte ( (BYTE)tItem.m_eOp );
		tOut.SendUint64 ( tItem.m_iSize );
		tOut.SendUint64 ( tItem.m_iOffFile );
		tOut.SendDword ( tItem.m_iOffBuf );
	}
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterFileSendRequest_t& tReq )
{
	tOut << "writerkey:" << tReq.m_tWriterKey;
	tOut << "consumed:" << tReq.m_tSendBuf.Consumed();
	tOut << "file:" << tReq.m_iFile;
	tOut << "ops:" << tReq.m_dOps.GetLength();
	return tOut;
}

void operator>> ( InputBuffer_c& tBuf, ClusterFileSendRequest_t& tReq )
{
	tReq.m_tWriterKey = tBuf.GetUint64();
	int iSize = tBuf.GetInt();
	tReq.m_iFile = tBuf.GetInt();
	if ( iSize )
	{
		const BYTE* pData = nullptr;
		tBuf.GetBytesZerocopy ( &pData, iSize );

		tReq.m_tSendBuf = SendBuf_c ( (BYTE*)pData, iSize );
	}

	int iCount = tBuf.GetDword();
	tReq.m_dOps.Resize ( iCount );
	for ( FileOp_t& tItem : tReq.m_dOps )
	{
		tItem.m_eOp = (FileOp_e)tBuf.GetByte();
		tItem.m_iSize = tBuf.GetUint64();
		tItem.m_iOffFile = tBuf.GetUint64();
		tItem.m_iOffBuf = tBuf.GetDword();
	}
}

struct ClusterFileSendReply_t
{
	int m_iFile = -1;
	WriteResult_e m_eRes { WriteResult_e::WRITE_FAILED };
	CSphString m_sWarning;
};

void operator<< ( ISphOutputBuffer& tOut, const ClusterFileSendReply_t& tReq )
{
	tOut.SendDword ( tReq.m_iFile );
	tOut.SendByte ( (BYTE)tReq.m_eRes );
	tOut.SendString ( tReq.m_sWarning.cstr() );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterFileSendReply_t& tReq )
{
	tOut << "ifile:" << tReq.m_iFile
		<< "eres" << (BYTE)tReq.m_eRes
		<< "warning:" << tReq.m_sWarning;
	return tOut;
}

void operator>> ( InputBuffer_c& tBuf, ClusterFileSendReply_t& tReq )
{
	tReq.m_iFile = tBuf.GetDword();
	tReq.m_eRes = (WriteResult_e)tBuf.GetByte();
	tReq.m_sWarning = tBuf.GetString();
}

// API command to remote node of file send
using ClusterFileSend_c = ClusterCommand_T<E_CLUSTER::FILE_SEND, ClusterFileSendRequest_t, ClusterFileSendReply_t>;

struct FileReader_t
{
	CSphAutofile m_tFile;
	ClusterFileSendRequest_t m_tFileSendRequest;
	const AgentDesc_t * m_pAgentDesc = nullptr;

	const SyncSrc_t * m_pSyncSrc = nullptr;
	const SyncDst_t * m_pSyncDst = nullptr;

	bool m_bDone = false;
	bool m_bSuccess = false;
	int m_iPackets = 1;

	int m_iChunk { 0 };
	int m_iFileRetries { 0 };

public:
	bool Next ( StringBuilder_c & sErrors );
	bool StartFile ( int iFile, StringBuilder_c& sErrors );
	void RetryFile ( int iRemoteFile, bool bNetError, WriteResult_e eRes, StringBuilder_c& tErrors );

private:
	bool ReadChunk ( int64_t iFileOff, int iSize, StringBuilder_c& sErrors );
	void LogFileSend() const;
	bool AddChunks ( StringBuilder_c& sErrors );
	[[nodiscard]] int NextFile ( int iCurFile ) const noexcept;
	[[nodiscard]] int NextChunk ( int iCurChunk, const FileChunks_t& tChunk ) const noexcept;

};

int FileReader_t::NextFile ( int iCurFile ) const noexcept
{
	assert ( m_pSyncSrc );
	assert ( m_pSyncDst );

	const int iFiles = m_pSyncSrc->m_dBaseNames.GetLength();
	for ( ; iCurFile < iFiles; ++iCurFile )
	{
		if ( !m_pSyncDst->m_dNodeChunksMask.BitGet ( iCurFile ) )
			break;
	}

	return iCurFile;
}

int FileReader_t::NextChunk ( int iCurChunk, const FileChunks_t& tChunk ) const noexcept
{
	assert ( m_pSyncDst );

	const int iChunks = tChunk.GetChunksCount();
	for ( ; iCurChunk < iChunks; ++iCurChunk )
	{
		if ( !m_pSyncDst->m_dNodeChunksMask.BitGet ( tChunk.m_iHashStartItem + iCurChunk ) )
			break;
	}

	return iCurChunk;
}

bool FileReader_t::ReadChunk ( int64_t iFileOff, int iSize, StringBuilder_c& sErrors )
{
	if ( !iSize )
		return true;

	assert ( m_pSyncSrc );
	assert ( m_tFileSendRequest.m_iFile >= 0 && m_tFileSendRequest.m_iFile < m_pSyncSrc->m_dBaseNames.GetLength() );

	if ( m_tFile.GetFD() == -1 )
	{
		CSphString sReaderError;
		if ( m_tFile.Open ( m_pSyncSrc->m_dIndexFiles[m_tFileSendRequest.m_iFile], SPH_O_READ, sReaderError, false ) < 0 )
		{
			sErrors += sReaderError.cstr();
			return false;
		}
	}

	SendBuf_c& tBuf = m_tFileSendRequest.m_tSendBuf;
	assert ( iSize <= tBuf.Left() );

	// seek and read new chunk
	while ( iSize>0 )
	{
		auto iRes = sphPread ( m_tFile.GetFD(), tBuf.Data(), iSize, iFileOff );
		if ( iRes < 0 )
		{
			sErrors.Appendf ( "pread error %d '%s'", errno, strerrorm ( errno ) );
			return false;
		};
		tBuf.Consume ( iRes );
		iSize -= iRes; // according to man, pread may return not whole requested chunk, and that is not error
	}
	return true;
}

void FileReader_t::LogFileSend () const
{
	sphLogDebugRpl ( "sending file %s (%d) to %s:%d, packets %d, timeout %d.%03d sec", m_pSyncSrc->m_dBaseNames[m_tFileSendRequest.m_iFile].cstr(), m_tFileSendRequest.m_iFile, m_pAgentDesc->m_sAddr.cstr(), m_pAgentDesc->m_iPort, m_iPackets, (int)( m_pSyncDst->m_tmTimeoutFile / 1000 ), (int)( m_pSyncDst->m_tmTimeoutFile % 1000 ) );
}

// add chunks copied from at joiner node along with data send from donor node
bool FileReader_t::AddChunks ( StringBuilder_c& sErrors )
{
	assert ( m_pSyncSrc );
	assert ( m_pSyncDst );
	assert ( m_tFileSendRequest.m_iFile < m_pSyncSrc->m_dBaseNames.GetLength() );

	const FileChunks_t& tChunks = m_pSyncSrc->m_dChunks[m_tFileSendRequest.m_iFile];
	const int iChunks = tChunks.GetChunksCount();

	auto& dOps = m_tFileSendRequest.m_dOps;
	int iChunk = m_iChunk;
	while ( iChunk < iChunks )
	{
		int iCopyChunk = NextChunk ( iChunk, tChunks );
		if ( iCopyChunk != iChunk ) // copy data from joiner local file
		{
			FileOp_t& tOp = dOps.Add();
			tOp.m_eOp = FileOp_e::COPY_FILE;
			tOp.m_iOffFile = tChunks.GetChunkFileOffset ( iChunk );
			if ( iCopyChunk < iChunks )
				tOp.m_iSize = tChunks.GetChunkFileOffset ( iCopyChunk ) - tOp.m_iOffFile; // (int64_t)m_iChunkBytes * iCopyChunk - m_iChunkBytes * iChunk;
			else
				tOp.m_iSize = tChunks.m_iFileSize - tOp.m_iOffFile;
		}

		if ( iCopyChunk < iChunks )
		{
			FileOp_t tOp;
			tOp.m_iOffFile = tChunks.GetChunkFileOffset ( iCopyChunk );
			tOp.m_iSize = tChunks.GetChunkFileLength ( iCopyChunk );
			tOp.m_iOffBuf = m_tFileSendRequest.m_tSendBuf.Consumed();
			if ( tOp.m_iSize > m_tFileSendRequest.m_tSendBuf.Left() )
			{
				iChunk = iCopyChunk; // last chunk that was already processed
				break;
			}

			if ( !ReadChunk ( tOp.m_iOffFile, tOp.m_iSize, sErrors ) )
				return false;

			tOp.m_eOp = FileOp_e::COPY_BUFFER;
			dOps.Add ( tOp );
		}

		iChunk = iCopyChunk + 1;
	}

	m_iChunk = iChunk;
	if ( iChunk >= iChunks )
		dOps.Add().m_eOp = FileOp_e::VERIFY_FILE;

	return true;
}

bool FileReader_t::StartFile ( int iFile, StringBuilder_c& sErrors )
{
	auto& dOps = m_tFileSendRequest.m_dOps;
	dOps.Resize ( 0 );

	assert ( m_pSyncSrc );
	assert ( m_pSyncDst );
	m_tFileSendRequest.m_iFile = NextFile ( iFile );
	m_iChunk = 0;
	m_tFileSendRequest.m_tSendBuf.Rewind();

	LogFileSend ( );

	return AddChunks ( sErrors );
}


bool FileReader_t::Next ( StringBuilder_c & sErrors )
{
	if ( m_bDone )
		return true;

	assert ( m_pSyncSrc );
	assert ( m_pSyncDst );
	const SyncSrc_t* pSrc = m_pSyncSrc;

	const int iFiles = pSrc->m_dBaseNames.GetLength();
	assert ( m_tFileSendRequest.m_iFile < iFiles );

	auto& dOps = m_tFileSendRequest.m_dOps;
	dOps.Resize ( 0 );
	m_tFileSendRequest.m_tSendBuf.Rewind();

	// check for chunks left in last file that was already sent
	if ( m_iChunk >= pSrc->m_dChunks[m_tFileSendRequest.m_iFile].GetChunksCount() )
	{
		m_tFile.Close();
		m_iChunk = 0;
		m_tFileSendRequest.m_iFile = NextFile ( m_tFileSendRequest.m_iFile + 1 );
		if ( m_tFileSendRequest.m_iFile == iFiles )
		{
			m_bDone = true;
			m_bSuccess = true;
			return true;
		}

		m_iFileRetries = ReplicationFileRetryCount();
		LogFileSend ( );
	}

	return AddChunks ( sErrors );
}

void ReportSendStat ( const VecTraits_T<AgentConn_t *> & dNodes, const VecTraits_T<FileReader_t> & dReaders, const CSphString & sCluster, const CSphString & sIndex )
{
	if ( g_eLogLevel<SPH_LOG_RPL_DEBUG )
		return;

	int iTotal = 0;
	StringBuilder_c tLog;
	tLog.Sprintf ( "file sync packets sent '%s:%s' to ", sCluster.cstr(), sIndex.cstr() );
	ARRAY_FOREACH ( iAgent, dNodes )
	{
		const AgentConn_t * pAgent = dNodes[iAgent];
		const FileReader_t & tReader = dReaders[iAgent];

		tLog.Sprintf ( "'%s:%d':%d,", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, tReader.m_iPackets );
		iTotal += tReader.m_iPackets;
	}
	tLog.Sprintf ( " total:%d", iTotal );
	sphLogDebugRpl ( "%s", tLog.cstr() );
}

static void ReportErrorSendFile ( StringBuilder_c& tErrors, const char* sFmt, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
void ReportErrorSendFile ( StringBuilder_c& tErrors, const char* sFmt, ... )
{
	CSphString sError;
	va_list ap;
	va_start ( ap, sFmt );
	sError.SetSprintfVa ( sFmt, ap );
	va_end ( ap );

	tErrors += sError.cstr();
	sphLogDebugRpl ( "%s", sError.cstr() );
}

void FileReader_t::RetryFile ( int iRemoteFile, bool bNetError, WriteResult_e eRes, StringBuilder_c& tErrors )
{
	assert ( m_tFileSendRequest.m_iFile >= 0 && m_tFileSendRequest.m_iFile < m_pSyncSrc->m_dBaseNames.GetLength() );

	// validate and report joiner errors
	if ( !bNetError )
	{
		if ( eRes != WriteResult_e::VERIFY_FAILED )
		{
			ReportErrorSendFile ( tErrors, "file %s (%d) write error at remote node to %s:%d, retry %d", m_pSyncSrc->m_dBaseNames[m_tFileSendRequest.m_iFile].cstr(), m_tFileSendRequest.m_iFile, m_pAgentDesc->m_sAddr.cstr(), m_pAgentDesc->m_iPort, m_iFileRetries );
			m_bDone = true;
			return;
		}

		if ( m_tFileSendRequest.m_iFile != iRemoteFile )
			ReportErrorSendFile ( tErrors, "retry file %s to %s:%d, file mismatch: reported %d, current %d", m_pSyncSrc->m_dBaseNames[m_tFileSendRequest.m_iFile].cstr(), m_pAgentDesc->m_sAddr.cstr(), m_pAgentDesc->m_iPort, iRemoteFile, m_tFileSendRequest.m_iFile );
	}

	--m_iFileRetries;
	sphLogDebugRpl ( "sending file %s to %s:%d, retry %d", m_pSyncSrc->m_dBaseNames[m_tFileSendRequest.m_iFile].cstr(), m_pAgentDesc->m_sAddr.cstr(), m_pAgentDesc->m_iPort, m_iFileRetries );
	if ( m_iFileRetries <= 0 )
	{
		ReportErrorSendFile ( tErrors, "retry file %s to %s:%d, limit exceeded", m_pSyncSrc->m_dBaseNames[m_tFileSendRequest.m_iFile].cstr(), m_pAgentDesc->m_sAddr.cstr(), m_pAgentDesc->m_iPort );
		m_bDone = true;
		return;
	}

	m_bDone = !StartFile ( m_tFileSendRequest.m_iFile, tErrors );
}

// send file to multiple nodes by chunks as API command CLUSTER_FILE_SEND
bool RemoteClusterFileSend ( const SyncSrc_t & tSigSrc, const CSphVector<RemoteFileState_t> & dDesc, const CSphString & sCluster, const CSphString & sIndex )
{
	StringBuilder_c tErrors ( ";" );

	// setup buffers
	CSphFixedVector<BYTE> dReadBuf ( tSigSrc.m_iBufferSize * dDesc.GetLength() );
	CSphFixedVector<FileReader_t> dReaders ( dDesc.GetLength() );
	uint64_t tKey = DoubleStringKey ( sCluster, sIndex );

	ARRAY_FOREACH ( iNode, dReaders )
	{
		// setup file readers
		FileReader_t& tReader = dReaders[iNode];
		tReader.m_pAgentDesc = dDesc[iNode].m_pAgentDesc;
		tReader.m_pSyncSrc = dDesc[iNode].m_pSyncSrc;
		tReader.m_pSyncDst = dDesc[iNode].m_pSyncDst;

		tReader.m_tFileSendRequest.m_tSendBuf = SendBuf_c ( dReadBuf.Begin() + tSigSrc.m_iBufferSize * iNode, tSigSrc.m_iBufferSize );
		tReader.m_tFileSendRequest.m_tWriterKey = tKey;
		tReader.m_iFileRetries = ReplicationFileRetryCount();

		if ( !tReader.StartFile ( 0, tErrors ) )
			return TlsMsg::Err ( "%s", tErrors.cstr() );

		assert ( !tReader.m_bDone );
	}

	// create agents
	VecRefPtrs_t<AgentConn_t*> dNodes;
	dNodes.Resize ( dReaders.GetLength() );
	ARRAY_FOREACH ( i, dReaders )
		dNodes[i] = ClusterFileSend_c::CreateAgent ( *dReaders[i].m_pAgentDesc, dReaders[i].m_pSyncDst->m_tmTimeoutFile, dReaders[i].m_tFileSendRequest );

	// submit initial jobs
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver() );
	ClusterFileSend_c tReq;
	ScheduleDistrJobs ( dNodes, &tReq, &tReq, tReporter, ReplicationFileRetryCount(), ReplicationFileRetryDelay() );

	bool bDone = false;
	while ( !bDone )
	{
		// don't forget to check incoming replies after send was over
		bDone = tReporter->IsDone();
		// wait one or more remote queries to complete
		if ( !bDone )
			tReporter->WaitChanges();

		ARRAY_FOREACH ( iAgent, dNodes )
		{
			AgentConn_t* pAgent = dNodes[iAgent];
			if ( !pAgent->m_bSuccess )
				continue;

			FileReader_t& tReader = dReaders[iAgent];
			if ( tReader.m_bDone )
				continue;

			const ClusterFileSendReply_t& tReply = ClusterFileSend_c::GetRes ( *pAgent );
			bool bFileWritten = ( tReply.m_eRes == WriteResult_e::OK );
			bool bNetError = ( !pAgent->m_sFailure.IsEmpty() );

			// report errors first
			if ( bNetError )
				ReportErrorSendFile ( tErrors, "'%s:%d' %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
			pAgent->m_sFailure = "";
			if ( !tReply.m_sWarning.IsEmpty() )
				ReportErrorSendFile ( tErrors, "'%s:%d' %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, tReply.m_sWarning.cstr() );

			if ( !bFileWritten )
			{
				tReader.RetryFile ( tReply.m_iFile, bNetError, tReply.m_eRes, tErrors );

			} else if ( !tReader.Next ( tErrors ) )
			{
				pAgent->m_bSuccess = false;
				tReader.m_bDone = true;
			}

			if ( tReader.m_bDone )
				continue;

			// remove agent from main vector
			pAgent->Release();

			AgentConn_t* pNextJob = ClusterFileSend_c::CreateAgent ( *tReader.m_pAgentDesc, tReader.m_pSyncDst->m_tmTimeoutFile, tReader.m_tFileSendRequest );
			dNodes[iAgent] = pNextJob;

			VectorAgentConn_t dNewNode;
			dNewNode.Add ( pNextJob );
			ScheduleDistrJobs ( dNewNode, &tReq, &tReq, tReporter, ReplicationFileRetryCount(), ReplicationFileRetryDelay() );

			// reset done flag to process new item
			bDone = false;
			++tReader.m_iPackets;
		}
	}

	if ( !tErrors.IsEmpty() )
		TlsMsg::Err ( "%s", tErrors.cstr() );

	ReportSendStat ( dNodes, dReaders, sCluster, sIndex );

	return dReaders.all_of ( [] ( const auto& tReader ) { return tReader.m_bSuccess; } );
}

// command at remote node for CLUSTER_FILE_SEND to store data into file, data size and file offset defined by sender
void ReceiveClusterFileSend ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf )
{
	ClusterFileSendRequest_t tCmd;
	ClusterFileSend_c::ParseRequest ( tBuf, tCmd );
	ClusterFileSendReply_t tRes;

	assert ( RecvState::HasState ( tCmd.m_tWriterKey ) );
	auto& tState = RecvState::GetState ( tCmd.m_tWriterKey );

	VecTraits_T<BYTE> dBuf ( tCmd.m_tSendBuf.Begin(), tCmd.m_tSendBuf.GetLength() );
	tRes.m_eRes = tState.Write ( tCmd.m_iFile, tCmd.m_dOps, dBuf );
	tRes.m_iFile = tCmd.m_iFile;

	if ( tRes.m_eRes != WriteResult_e::OK )
	{
		tRes.m_sWarning.SetSprintf ( "write finished %s (%d), file %d, operations %d", TlsMsg::szError(), (int)tRes.m_eRes, tCmd.m_iFile, tCmd.m_dOps.GetLength() );
		sphWarning ( "%s", tRes.m_sWarning.cstr() );
	}

	ClusterFileSend_c::BuildReply ( tOut, tRes );
	TlsMsg::ResetErr();
}

