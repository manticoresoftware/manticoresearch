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

enum class SstStage_e
{
	WAIT_NODES = 0,
	CALC_SHA1,
	RESERVE_FILES,
	SEND_FILES,
	ACTIVATE_TABLE,

	TOTAL
};

struct SstProgressStatus_t
{
	float      m_fTotal = 0.0f;     // progress (0.0 to 1.0)
	SstStage_e m_eCurrentStage = SstStage_e::TOTAL;
	float      m_fCurrentStageTotal = 0.0f; // progress of the current stage (0.0 to 1.0)

	int         m_iTablesCount = 0;	// total number of tables in the SST
	int         m_iTable = 0;		// current table
	CSphString  m_sTable;			// name of the current table
};

struct SstProgressContext_t
{
	float    m_fTotalBefore = 0.0f;
	float    m_fStageWeight = 0.0f;
	uint64_t m_uStageTotal = 0;
};

void operator<< ( ISphOutputBuffer & tOut, const SstProgressContext_t & tCtx );
void operator>> ( InputBuffer_c & tIn, SstProgressContext_t & tCtx );

class SstProgress_i : public ISphRefcountedMT
{
public:
	enum class Role_e { DONOR, JOINER, NONE };

	SstProgress_i() = default;
	virtual ~SstProgress_i() = default;

	virtual void Init ( Role_e eRole ) = 0;

	virtual void StageBegin ( SstStage_e eStage ) = 0;
	virtual void AddComplete ( uint64_t uBytes ) = 0;
	virtual void UpdateTotal ( uint64_t uBytes ) = 0;

	virtual void SetTable ( const CSphString & sName ) = 0;
	virtual void Finish() = 0;

	virtual void CollectFilesStats ( const VecTraits_T<CSphString> & dIndexes, int iAttempt, int iAgents ) = 0;

	virtual void GetStatus( SstProgressStatus_t & tStatus ) const = 0;

	// joiner
	virtual void UpdateFromRemote ( const SstProgressStatus_t& tStatus ) = 0;
	virtual void StartLocalStage ( const SstProgressContext_t & tCtx, SstStage_e eStage ) = 0;
	virtual void FinishLocalStage() = 0;

	virtual void SetDonor4Joiner ( int iFilesCount, SstProgressContext_t & tCtx ) = 0;
	static void StartPushUpdates ( const CSphString & sCluster, const VecTraits_T<AgentDesc_t> & dDesc, CSphRefcountedPtr<SstProgress_i> pProgress );
	virtual bool IsPushUpdateDone() = 0;
};

const char * SstGetStageName ( SstStage_e eStage );

SstProgress_i * CreateProgress();

struct SyncSrc_t;
void PrepareSendStage ( const SyncSrc_t & tSigSrc, const VecTraits_T<AgentConn_t*> & dNodes, SstProgress_i & tProgress );

void ReceiveSstProgress ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster );
