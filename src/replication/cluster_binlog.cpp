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

#include "fileutils.h"
#include "wsrep_cxx.h"
#include "searchdaemon.h"
#include "searchdha.h"

#include "cluster_binlog.h"

static bool g_bRplBinlogEnabled = ( val_from_env ( "MANTICORE_REPLICATION_BINLOG", 1 )!=0 );
static bool LOG_LEVEL_RPLOG = val_from_env ( "MANTICORE_RPL_BINLOG", false ); // verbose logging for cluster binlog, ruled by this env variable

struct ClusterTid_t
{
	Wsrep::GlobalTid_t m_tGtid;
	int m_iOffSeqno = 0;
};

using Clusters_h = SmallStringHash_T<ClusterTid_t>;

class ClusterBinlog_c final : public ClusterBinlog_i
{
public:
	ClusterBinlog_c() = default;
	~ClusterBinlog_c() final = default;

	void Init ( const CSphString & sConfigFile, bool bDisabled ) final;
	void Close() final;

	void OnClusterDelete ( const CSphString & sCluster, const StrVec_t & dIndexes ) final;
	void OnClusterLoad ( ClusterBinlogData_c & tCluster ) final;

	void ClusterTnx ( const ClusterBinlogData_c & tCluster ) final;
	void OnClusterSynced ( const ClusterBinlogData_c & tCluster ) final;

private:
	SharedMemory_c * m_pBinlog = nullptr;
	CSphMutex m_tLock;
	CSphString m_sFile;
	CSphString m_sError;
	bool m_bValid = true;
	Clusters_h m_hClusters;

	bool IsValid() const { return m_bValid; }
	BYTE * GetWritePtr ( int64_t iSize );

	void LogError() const;

	static constexpr DWORD BINLOG_HEADER_MAGIC_SPBL = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
	static constexpr DWORD BINLOG_VERSION = 1;
	
	static constexpr int MAGIC_INITIAL_LIMIT = 1024;
	int64_t Relimit ( int64_t iLimit, int64_t iNewLimit )
	{
		if ( !iLimit )
			iLimit = MAGIC_INITIAL_LIMIT;
		while ( iLimit < iNewLimit )
		{
			iLimit = (int)( (float)iLimit * 1.2f );
			assert ( iLimit > 0 );
		}
		return iLimit;
	}

	bool Read ();
	bool Write ();
	void Invalidate();

	void WriteSeqno ( const ClusterTid_t & tCluster );
};

static std::unique_ptr<ClusterBinlog_i> g_pBinlog { new ClusterBinlog_c() };

ClusterBinlog_i * RplBinlog()
{
	return g_pBinlog.get();
}

void ReplicationBinlogStart ( const CSphString & sDataDir, bool bDisabled )
{
	g_pBinlog.reset ( new ClusterBinlog_c() );
	g_pBinlog->Init ( sDataDir, bDisabled );
}

static CSphString GetFileName ( const CSphString & sDataDir )
{
	CSphString sFile;
	if ( !sDataDir.Begins ( "/" ) )
		sFile.SetSprintf ( "/%s", sDataDir.cstr() );
	else
		sFile = sDataDir;

	char * sCur = const_cast<char *> ( sFile.cstr() );
	const char * sEnd = sCur + sFile.Length();
	if ( sCur<sEnd && *sCur=='/' )
		sCur++;

	for ( ; sCur<sEnd; sCur++ )
	{
		if ( *sCur=='/' )
			*sCur = '_';
	}

	return sFile;
}

void ClusterBinlog_c::Init ( const CSphString & sDataDir, bool bDisabled )
{
	ScopedMutex_t tLock ( m_tLock );

	m_bValid = false;
	m_sFile = GetFileName ( sDataDir );

	if ( !g_bRplBinlogEnabled )
		return;
	if ( bDisabled )
		return;

	std::unique_ptr<SharedMemory_c> pBinlog { new SharedMemory_c ( m_sFile ) };
	if ( !pBinlog->IsSupported() )
		return;

	if ( !m_sFile.IsEmpty() && LOG_LEVEL_RPLOG )
		sphLogDebugRpl ( "replication binlog file '%s'", m_sFile.cstr() );

	SharedMemory_c::OpenResult_e eRes = pBinlog->Open();

	// no binlog file is not error as daemon could shutdown clean and closes the binlog
	// or no replication and no need to keep the cluster binlog
	if ( eRes==SharedMemory_c::OpenResult_e::NO_FILE )
	{
		sphLogDebugRpl ( "replication binlog ok, but can not open file: %s", pBinlog->GetError().cstr() );
		m_bValid = true;
		return;
	}

	if ( eRes!=SharedMemory_c::OpenResult_e::OK )
	{
		m_sError = pBinlog->GetError();
		LogError();
		return;
	}

	sphLogDebugRpl ( "replication binlog opened '%s', size %d", m_sFile.cstr(), (int)pBinlog->GetLength64() );

	m_bValid = true;
	m_pBinlog = pBinlog.release();

	// close existed binlog in case of any error and allow to issue SST
	Read();
}

void ReplicationBinlogStop()
{
	g_pBinlog->Close();
}

void ClusterBinlog_c::Close()
{
	ScopedMutex_t tLock ( m_tLock );

	m_hClusters.Reset();
	m_bValid = false;

	if ( !m_pBinlog )
		return;

	sphLogDebugRpl ( "replication binlog closed, size %d", (int)m_pBinlog->GetLength64() );

	if ( !m_pBinlog->Close() )
	{
		m_sError = m_pBinlog->GetError();
		LogError();
	}
	SafeDelete ( m_pBinlog );
}

BYTE * ClusterBinlog_c::GetWritePtr ( int64_t iSize )
{
	if ( !IsValid() )
		return nullptr;

	// if no binlog at this point need to create new binlog file but not try to open existed
	if ( !m_pBinlog )
	{
		std::unique_ptr<SharedMemory_c> pBinlog { new SharedMemory_c ( m_sFile ) };
		if ( !pBinlog->Create ( Relimit ( 0, iSize ) ) )
		{
			m_bValid = false;
			m_sError = m_pBinlog->GetError();
			LogError();
			return nullptr;
		}
		m_pBinlog = pBinlog.release();
	}

	assert ( IsValid() && m_pBinlog );
	if ( m_pBinlog->GetLength64()<iSize && !m_pBinlog->Reset ( Relimit ( m_pBinlog->GetLength64(), iSize ) ) )
	{
		m_bValid = false;
		m_sError = m_pBinlog->GetError();
		LogError();
		return nullptr;
	}

	return m_pBinlog->GetWritePtr();
}

void ClusterBinlog_c::LogError() const
{
	sphFatalLog ( "replication binlog error: %s", m_sError.cstr() );
}

bool ClusterBinlog_c::Read()
{
	if ( !m_pBinlog || !m_pBinlog->GetLength64() )
		return true;

	MemoryReader_c tRd ( m_pBinlog->GetReadPtr(), m_pBinlog->GetLength64() );

	// emit header
	DWORD uHeader = tRd.GetDword();
	if ( uHeader!=BINLOG_HEADER_MAGIC_SPBL )
	{
		m_sError = "missing magic header (corrupted?)";
		Invalidate();
		return false;
	}

	DWORD uVersion = tRd.GetDword();
	if ( uVersion!=BINLOG_VERSION )
	{
		m_sError.SetSprintf ( "is v.%d, binary is v.%d; recovery requires previous binary version", uVersion, BINLOG_VERSION );
		Invalidate();
		return false;
	}

	int iClusters = tRd.UnzipInt();

	Clusters_h hClusters;
	for ( int i=0; i<iClusters; i++ )
	{
		if ( !tRd.HasData() )
		{
			m_sError.SetSprintf ( "invalid data at position %d", tRd.GetPos() );
			Invalidate();
			return false;
		}

		CSphString sName = tRd.GetString();
		auto & tCluster = hClusters.AddUnique ( sName );

		tRd.GetBytes ( tCluster.m_tGtid.m_tUuid.data(), sizeof ( tCluster.m_tGtid.m_tUuid ) );

		tCluster.m_iOffSeqno = tRd.GetPos();
		tCluster.m_tGtid.m_iSeqNo = tRd.GetOffset();
	}

	m_hClusters = std::move ( hClusters );

	return true;
}

bool ClusterBinlog_c::Write()
{
	if ( !IsValid() )
		return false;

	CSphVector<BYTE> dBuf;
	MemoryWriter_c tWr ( dBuf );

	int iClusters = m_hClusters.GetLength();

	// emit header
	tWr.PutDword ( BINLOG_HEADER_MAGIC_SPBL );
	tWr.PutDword ( BINLOG_VERSION );

	tWr.ZipInt ( iClusters );
	
	// clusters seqno and indexes tnx first
	// also update tnx write offset at the cluster data
	for ( auto & tCluster : m_hClusters )
	{
		tWr.PutString ( tCluster.first );

		tWr.PutBytes ( tCluster.second.m_tGtid.m_tUuid.data(), sizeof( tCluster.second.m_tGtid.m_tUuid ) );

		tCluster.second.m_iOffSeqno = tWr.GetPos();
		tWr.PutOffset ( tCluster.second.m_tGtid.m_iSeqNo );
	}

	BYTE * pDst = GetWritePtr ( dBuf.GetLength() );
	if ( !IsValid() )
	{
		Invalidate();
		return false;
	}

	assert ( IsValid() && pDst && m_pBinlog->GetLength64()>=dBuf.GetLength() );
	memcpy ( pDst, dBuf.Begin(), dBuf.GetLength() );

	sphLogDebugRpl ( "replication binlog flushed, size %d(%d)", (int)dBuf.GetLength(), (int)m_pBinlog->GetLength64() );
	return true;
}

void ClusterBinlog_c::Invalidate()
{
	LogError();
	Close();
	m_bValid = false;
}

/////////////////////////////////////////////////////////////////////////////
/// cluster binlog helpers

void ClusterBinlog_c::OnClusterDelete ( const CSphString & sCluster, const StrVec_t & dIndexes )
{
	if ( !IsValid() )
		return;

	ScopedMutex_t tLock ( m_tLock );

	m_hClusters.Delete ( sCluster );
	Write();
}

void ClusterBinlog_c::WriteSeqno ( const ClusterTid_t & tCluster )
{
	assert ( IsValid() && m_pBinlog->GetWritePtr() );
	assert ( tCluster.m_iOffSeqno>0 && tCluster.m_iOffSeqno<m_pBinlog->GetLength64() );

	BYTE * pBlog = m_pBinlog->GetWritePtr();
	memcpy ( pBlog + tCluster.m_iOffSeqno, &tCluster.m_tGtid.m_iSeqNo, sizeof ( tCluster.m_tGtid.m_iSeqNo ) );
}

void ClusterBinlog_c::ClusterTnx ( const ClusterBinlogData_c & tCluster )
{
	if ( !IsValid() )
		return;

	ScopedMutex_t tLock ( m_tLock );

	auto * pCluster = m_hClusters ( tCluster.m_sName );
	if ( pCluster )
		WriteSeqno ( *pCluster );

	if ( LOG_LEVEL_RPLOG )
		sphLogDebugRpl ( "replication binlog tnx '%s', seqno " INT64_FMT ", at %d", tCluster.m_sName.cstr(), tCluster.m_tGtid.m_iSeqNo, ( pCluster ? pCluster->m_iOffSeqno : -1 ) );
}

void ClusterBinlog_c::OnClusterLoad ( ClusterBinlogData_c & tCluster )
{
	if ( !IsValid() )
		return;

	ScopedMutex_t tLock ( m_tLock );

	auto * pCluster = m_hClusters ( tCluster.m_sName );
	if ( pCluster )
		tCluster.m_tGtid = pCluster->m_tGtid;

	sphLogDebugRpl ( "replication binlog %s cluster '%s', gtid %s", ( pCluster ? "loaded" : "missed" ), tCluster.m_sName.cstr(), Wsrep::Gtid2Str ( tCluster.m_tGtid ).cstr() );
}

void ClusterBinlog_c::OnClusterSynced ( const ClusterBinlogData_c & tCluster )
{
	if ( !IsValid() )
		return;

	ScopedMutex_t tLock ( m_tLock );

	auto * pItem = m_hClusters ( tCluster.m_sName );
	if ( pItem )
	{
		bool bUuidMatched = ( pItem->m_tGtid.m_tUuid==tCluster.m_tGtid.m_tUuid );
		bool bSeqnoMatched = ( pItem->m_tGtid.m_iSeqNo==tCluster.m_tGtid.m_iSeqNo );

		// do nothing if the gtid matched
		if ( bUuidMatched && bSeqnoMatched )
		{
			if ( LOG_LEVEL_RPLOG )
				sphLogDebugRpl ( "replication binlog synced '%s', seqno " INT64_FMT ", at %d, gtid matched", tCluster.m_sName.cstr(), pItem->m_tGtid.m_iSeqNo, pItem->m_iOffSeqno );
			return;
		} else if ( bUuidMatched )
		{
			pItem->m_tGtid.m_iSeqNo = tCluster.m_tGtid.m_iSeqNo;
			WriteSeqno ( *pItem );
			if ( LOG_LEVEL_RPLOG )
				sphLogDebugRpl ( "replication binlog synced '%s', seqno " INT64_FMT ", at %d, uuid matched", tCluster.m_sName.cstr(), pItem->m_tGtid.m_iSeqNo );
			return;
		}
	}

	m_hClusters.AddUnique ( tCluster.m_sName ).m_tGtid = tCluster.m_tGtid;
	Write();
	if ( LOG_LEVEL_RPLOG )
		sphLogDebugRpl ( "replication binlog synced '%s', seqno " INT64_FMT ", at %d, entry added", tCluster.m_sName.cstr(), pItem->m_tGtid.m_iSeqNo );
}
