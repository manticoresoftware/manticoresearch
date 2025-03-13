//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/ints.h"
#include "std/vectraits.h"
#include "std/refcounted_mt.h"
#include "std/string.h"
#include "std/blobs.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <array>

namespace Wsrep {

enum class ViewStatus_e
{
	PRIMARY,		// have quorum
	NON_PRIMARY,	// haven't quorum
	DISCONNECTED,
	MAX
};

using UUID_t = std::array<BYTE, 16>;

constexpr int64_t WRONG_SEQNO = -1;

// global transaction identifier
struct GlobalTid_t {
	UUID_t m_tUuid {}; // { 0 } by default
	int64_t m_iSeqNo { WRONG_SEQNO };
};

struct StatsVars_t
{
	const char* m_szName;
	enum {
		STRING, // point to asciiz
		INT64,
		DOUBLE
	} m_eType;
	union {
		const char* szString;
		int64_t		iVal64;
		double		fDouble;
	} m_tValue;
};

using StatsVarsIteratorFN = std::function<bool ( const StatsVars_t& )>;
using StatsVarsVisitorFN = std::function<void ( const StatsVars_t& )>;

struct MemberInfo_t
{
	UUID_t	m_tUuid {};
	char 	m_sName[32];
	char 	m_sIncoming[256];
};

struct ViewInfo_t
{
	GlobalTid_t		m_tStateId;
	int64_t 		m_ViewSeqNo;
	ViewStatus_e	m_eStatus;
	bool 			m_bGap;
	int 			m_iIdx;
	int 			m_iNMembers;
	int 			m_iProto;
	MemberInfo_t	m_tMemInfo;
};

class Writeset_i
{
public:
	[[nodiscard]] virtual int64_t LastSeqno () const noexcept = 0;
	[[nodiscard]] virtual bool LastOk () const noexcept = 0;
	[[nodiscard]] virtual bool AppendKeys ( const VecTraits_T<uint64_t> & dBufKeys, bool bSharedKeys ) = 0;
	[[nodiscard]] virtual bool AppendData ( const VecTraits_T<BYTE> & dData ) = 0;
	[[nodiscard]] virtual bool Replicate () = 0;
	[[nodiscard]] virtual bool PreCommit () = 0;
	virtual void AbortPreCommit () = 0;
	virtual void InterimCommit () = 0;
	virtual void PostCommit () = 0;
	[[nodiscard]] virtual bool ToExecuteStart ( const VecTraits_T<uint64_t> & dBufKeys, const VecTraits_T<BYTE> & dData ) = 0;
	virtual void ToExecuteEnd () = 0;
	virtual ~Writeset_i() = default;
};

class Cluster_i : public ISphRefcountedMT
{

public:
	// cb invoked when cluster's structure changed. No other callbacks are called concurrently with it.
	virtual void ChangeView ( const ViewInfo_t* pView, const char* pState, uint64_t uStateLen, void** ppSstReq, uint64_t* pSstReqLen ) = 0;

	// cluster synced and could accept trx
	virtual void SetSynced() = 0;

	// transfer full state to another node.
	virtual bool DonateSST ( CSphString sNode, const GlobalTid_t* pStateID, bool bBypass ) = 0;

	// cb invoked when started recv
	virtual void OnRecvStarted() = 0;

	// cb invoked when recv is to be finished
	virtual void OnRecvFinished ( bool bSuccess ) = 0;
};

using ClusterRefPtr_c = CSphRefcountedPtr<Cluster_i>;


// transaction meta data
struct TrxMeta_t
{
	GlobalTid_t	m_tGtid;
	int64_t		m_iDepSeqNo;
};

class Receiver_i : public ISphRefcountedMT
{
public:
	virtual bool ApplyWriteset ( ByteBlob_t tData, bool bIsolated ) = 0;
	virtual void ApplyUnordered ( ByteBlob_t tData ) = 0;
	virtual bool Commit ( const void * pHndTrx, uint32_t uFlags, const TrxMeta_t * pMeta, bool bCommit ) = 0;
};

using ReceiverRefPtr_c = CSphRefcountedPtr<Receiver_i>;

class Applier_i
{
public:
	virtual void ApplierPreCommit ( const void* ) = 0;
	virtual void ApplierInterimPostCommit ( const void* ) = 0;
};

class Provider_i : ISphNoncopyable
{
public:
	virtual ~Provider_i() = default;
	virtual void SstReceived ( const GlobalTid_t & tGtid, int iRes ) = 0;
	virtual void SstSent ( const GlobalTid_t & tGtid, bool bOk ) = 0;
	virtual void Disconnect () = 0;
	[[nodiscard]] virtual bool OptionsSet ( const CSphString & sName, const CSphString & sValue ) = 0;
	virtual bool EnumFindStatsVar ( StatsVarsIteratorFN&& fnCondition ) = 0;
	virtual void EnumStatsVars ( StatsVarsVisitorFN&& fnVisitor ) = 0;
	[[nodiscard]] virtual bool Connect ( const char * szName, const char * szNodes, bool bBootStrap ) = 0;
	virtual void StartListen ( Wsrep::Receiver_i * pReceiver ) = 0;
	virtual std::unique_ptr<Writeset_i> MakeWriteSet() = 0;
	virtual Applier_i* GetApplier() = 0;
};

Provider_i * MakeProvider ( Cluster_i * pCluster, CSphString sName, const char * szListenAddr, const char * szIncoming, const char * szPath, const char * szOptions );

// these guys tested in gtests_wsrep.cpp
CSphString Uuid2Str ( const UUID_t& tUuid );
UUID_t Str2Uuid ( const CSphString& sUuid );

CSphString Gtid2Str ( const GlobalTid_t& tGtid );
GlobalTid_t Str2Gtid ( const CSphString& sGtid );

const char * GetViewStatus ( ViewStatus_e tStatus ) noexcept;

} // namespace Wsrep

// comparing moved out from Wsrep namespace
inline bool operator== ( const Wsrep::GlobalTid_t & lhs, const Wsrep::GlobalTid_t & rhs ) noexcept
{
	return lhs.m_iSeqNo == rhs.m_iSeqNo && lhs.m_tUuid == rhs.m_tUuid;
}

inline bool operator!= ( const Wsrep::GlobalTid_t & lhs, const Wsrep::GlobalTid_t & rhs ) noexcept
{
	return !( lhs == rhs );
}
