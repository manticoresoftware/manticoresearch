#include "searchd_shard.h"

#include "client_session.h"
#include "docs_collector.h"
#include "searchdreplication.h"

#include <limits>
#include <utility>

void ShardDocStorage_t::Cleanup()
{
	m_dAttrRows.Resize ( 0 );
	m_dColumnar.Resize ( 0 );
	m_dMvaPool.Resize ( 0 );
	m_dFieldRefs.Resize ( 0 );
	m_dStringRefs.Resize ( 0 );
	m_dBlobPool.Resize ( 0 );
	m_dRows.Resize ( 0 );
}

bool ShardTxnState_t::HasPendingData() const
{
	return !m_sShard.IsEmpty();
}

void ShardTxnState_t::Cleanup()
{
	m_tStorage.Cleanup();
	m_dDeletePredicates.Resize ( 0 );
	m_dTargets.Resize ( 0 );
	m_dOrder.Resize ( 0 );
	m_sShard = "";
	m_pShardInstance = nullptr;
	m_eWriteStmt = STMT_DUMMY;
}

void StringPtrTraits_t::SavePointersTo ( VecTraits_T<const char *> & dStrings, bool bSkipInvalid ) const
{
	if ( bSkipInvalid )
	{
		ARRAY_FOREACH ( i, m_dOff )
		{
			int iOff = m_dOff[i];
			if ( iOff<0 )
				continue;
			dStrings[i] = (const char *)m_dPackedData.Begin() + iOff;
		}

	} else
	{
		ARRAY_FOREACH ( i, m_dOff )
		{
			int iOff = m_dOff[i];
			dStrings[i] = ( iOff>=0 ? (const char *)m_dPackedData.Begin() + iOff : nullptr );
		}
	}
}

void StringPtrTraits_t::Reset ()
{
	m_dPackedData.Resize ( 0 );
	m_dParserBuf.Resize ( 0 );
	m_dOff.Fill ( -1 );
}

BYTE * StringPtrTraits_t::ReserveBlob ( int iBlobSize, int iOffset )
{
	if ( !iBlobSize )
		return nullptr;

	m_dOff[iOffset] = m_dPackedData.GetLength();

	BYTE * pPacked = m_dPackedData.AddN ( sphCalcPackedLength ( iBlobSize ) );
	pPacked += ZipToPtrBE ( pPacked, iBlobSize );
	return pPacked;
}

DistributedIndex_t * ShardIndex_c::Clone() const
{
	ShardIndex_c * pShard ( new ShardIndex_c );
	pShard->m_dAgents = m_dAgents;
	pShard->m_dLocal = m_dLocal;
	pShard->m_iAgentRetryCount = m_iAgentRetryCount;
	pShard->m_bDivideRemoteRanges = m_bDivideRemoteRanges;
	pShard->m_eHaStrategy = m_eHaStrategy;
	pShard->m_sCluster = m_sCluster;
	pShard->SetAgentConnectTimeoutMs ( GetAgentConnectTimeoutMs ( true ) );
	pShard->SetAgentQueryTimeoutMs ( GetAgentQueryTimeoutMs ( true ) );
	pShard->m_sIndexPath = m_sIndexPath;
	pShard->m_tSchema = m_tSchema;
	pShard->m_tSettings = m_tSettings;
	pShard->m_tTokenizerSettings = m_tTokenizerSettings;
	pShard->m_tDictSettings = m_tDictSettings;
	pShard->m_tFieldFilterSettings = m_tFieldFilterSettings;
	pShard->m_dRouteTargets = m_dRouteTargets;
	return pShard;
}

bool EnsureNoPendingShardTxn ( const ClientSession_c & tSession, CSphString & sError )
{
	if ( !tSession.m_tShardTxn.HasPendingData() )
		return true;

	sError.SetSprintf ( "current txn is working with another table ('%s')", tSession.m_tShardTxn.m_sShard.cstr() );
	return false;
}

static bool InvalidateChangedShardTxn ( ShardTxnState_t & tShardTxn, CSphString & sError, bool bOnCommit )
{
	CSphString sShard = tShardTxn.m_sShard;
	tShardTxn.Cleanup();
	if ( bOnCommit )
		sError.SetSprintf ( "can not finish transaction, table changed '%s'", sShard.cstr() );
	else
		sError.SetSprintf ( "current txn is working with changed table '%s', restart session", sShard.cstr() );
	return false;
}

static bool IsShardWriteStmt ( SqlStmt_e eStmt )
{
	return eStmt==STMT_INSERT || eStmt==STMT_REPLACE;
}

static bool EnsureShardTxnScope ( ClientSession_c & tSession, const CSphString & sShard, const ShardIndex_c * pShard, SqlStmt_e eStmt, CSphString & sError )
{
	auto * pRtIndex = tSession.m_tAcc.GetIndex();
	if ( pRtIndex )
	{
		sError.SetSprintf ( "current txn is working with another table ('%s')", pRtIndex->GetName() );
		return false;
	}

	if ( !tSession.m_tShardTxn.HasPendingData() )
		return true;

	if ( tSession.m_tShardTxn.m_sShard!=sShard )
	{
		sError.SetSprintf ( "current txn is working with another table ('%s')", tSession.m_tShardTxn.m_sShard.cstr() );
		return false;
	}

	if ( !tSession.m_tShardTxn.m_pShardInstance )
		tSession.m_tShardTxn.m_pShardInstance = pShard;
	else if ( tSession.m_tShardTxn.m_pShardInstance!=pShard )
		return InvalidateChangedShardTxn ( tSession.m_tShardTxn, sError, false );

	if ( IsShardWriteStmt ( eStmt ) && tSession.m_tShardTxn.m_eWriteStmt!=STMT_DUMMY && tSession.m_tShardTxn.m_eWriteStmt!=eStmt )
	{
		sError.SetSprintf ( "current txn can not mix INSERT and REPLACE for table '%s'", sShard.cstr() );
		return false;
	}

	return true;
}

void RollbackShardTxn ( ShardTxnState_t & tShardTxn )
{
	tShardTxn.Cleanup();
}

static bool IsShardStringAttr ( ESphAttr eAttrType )
{
	return eAttrType==SPH_ATTR_STRING || eAttrType==SPH_ATTR_STRINGPTR || eAttrType==SPH_ATTR_JSON;
}

static int GetShardStringAttrCount ( const CSphSchema & tSchema )
{
	int iStringAttrs = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
		if ( IsShardStringAttr ( tSchema.GetAttr(i).m_eAttrType ) )
			++iStringAttrs;

	return iStringAttrs;
}

static ShardSpanRef_t StoreShardBlob ( CSphVector<BYTE> & dBlobPool, const BYTE * pData, int iLen )
{
	ShardSpanRef_t tRef;
	if ( !pData || iLen <= 0 )
		return tRef;

	tRef.m_iOff = dBlobPool.GetLength();
	tRef.m_iLen = iLen;
	dBlobPool.Append ( pData, iLen );
	return tRef;
}

static ShardSpanRef_t StoreShardString ( CSphVector<BYTE> & dBlobPool, const char * szData )
{
	ShardSpanRef_t tRef;
	if ( !szData )
		return tRef;

	int iLen = (int)strlen ( szData );
	tRef.m_iOff = dBlobPool.GetLength();
	tRef.m_iLen = iLen;
	dBlobPool.Append ( (const BYTE *)szData, iLen );
	dBlobPool.Add ( (BYTE)0 );
	return tRef;
}

static int StoreRow ( const AttributeConverter_c & tConverter, const CSphSchema & tSchema, ShardDocStorage_t & tStorage, bool bReplace, int iTxnOrdinal )
{
	ShardBufferedRow_t tRow;
	tRow.m_bReplace = bReplace;
	tRow.m_iTxnOrdinal = iTxnOrdinal;

	const int iAttrRowBytes = tSchema.GetRowSize() * sizeof(CSphRowitem);
	if ( iAttrRowBytes && tConverter.m_tDoc.m_pDynamic )
	{
		tRow.m_iAttrRowOff = tStorage.m_dAttrRows.GetLength();
		tStorage.m_dAttrRows.Append ( (const BYTE *)tConverter.m_tDoc.m_pDynamic, iAttrRowBytes );
	}

	const int iColumnarAttrs = tConverter.m_dColumnarAttrs.GetLength();
	if ( iColumnarAttrs )
	{
		tRow.m_iColumnarOff = tStorage.m_dColumnar.GetLength();
		tStorage.m_dColumnar.Append ( tConverter.m_dColumnarAttrs );
	}

	const auto & dMva = tConverter.GetMVAs();
	if ( dMva.GetLength() )
	{
		tRow.m_tMva.m_iOff = tStorage.m_dMvaPool.GetLength();
		tRow.m_tMva.m_iCount = dMva.GetLength();
		tStorage.m_dMvaPool.Append ( dMva );
	}

	const int iFields = tSchema.GetFieldsCount();
	if ( iFields )
	{
		tRow.m_iFieldRefOff = tStorage.m_dFieldRefs.GetLength();
		auto * pFieldRefs = tStorage.m_dFieldRefs.AddN ( iFields );
		for ( int iField = 0; iField < iFields; ++iField )
		{
			const auto & tField = tConverter.m_dFields[iField];
			pFieldRefs[iField] = StoreShardBlob ( tStorage.m_dBlobPool, (const BYTE *)tField.Begin(), (int)tField.GetLength() );
		}
	}

	const int iStringAttrs = GetShardStringAttrCount ( tSchema );
	if ( iStringAttrs )
	{
		tRow.m_iStringRefOff = tStorage.m_dStringRefs.GetLength();
		auto * pStringRefs = tStorage.m_dStringRefs.AddN ( iStringAttrs );
		int iStringAttr = 0;
		for ( int iAttr = 0; iAttr < tSchema.GetAttrsCount(); ++iAttr )
		{
			const auto & tAttr = tSchema.GetAttr(iAttr);
			if ( !IsShardStringAttr ( tAttr.m_eAttrType ) )
				continue;

			const char * pStr = iStringAttr < tConverter.m_dStrings.GetLength() ? tConverter.m_dStrings[iStringAttr] : nullptr;
			if ( tAttr.m_eAttrType == SPH_ATTR_JSON && pStr )
			{
				auto dPacked = sphUnpackPtrAttr ( (const BYTE *)pStr );
				pStringRefs[iStringAttr] = StoreShardBlob ( tStorage.m_dBlobPool, dPacked.first, sphCalcPackedLength ( dPacked.second ) );
			} else
				pStringRefs[iStringAttr] = StoreShardString ( tStorage.m_dBlobPool, pStr );

			++iStringAttr;
		}
	}

	int iRowId = tStorage.m_dRows.GetLength();
	tStorage.m_dRows.Add ( tRow );
	return iRowId;
}

static void RowToInsert ( const CSphSchema & tSchema, const ShardDocStorage_t & tStorage, int iRowId, InsertDocData_c & tDoc, bool & bReplace )
{
	assert ( iRowId >= 0 && iRowId < tStorage.m_dRows.GetLength() );
	const auto & tRow = tStorage.m_dRows[iRowId];
	bReplace = tRow.m_bReplace;
	const BYTE * pBlobBase = tStorage.m_dBlobPool.IsEmpty() ? nullptr : tStorage.m_dBlobPool.Begin();

	const int iAttrRowBytes = tSchema.GetRowSize() * sizeof(CSphRowitem);
	if ( iAttrRowBytes )
	{
		assert ( tRow.m_iAttrRowOff >= 0 );
		assert ( tRow.m_iAttrRowOff + iAttrRowBytes <= tStorage.m_dAttrRows.GetLength() );
		memcpy ( tDoc.m_tDoc.m_pDynamic, tStorage.m_dAttrRows.Begin() + tRow.m_iAttrRowOff, iAttrRowBytes );
	}

	const int iColumnarAttrs = tSchema.GetColumnarAttrsCount();
	tDoc.m_dColumnarAttrs.Resize ( iColumnarAttrs );
	if ( iColumnarAttrs )
	{
		assert ( tRow.m_iColumnarOff >= 0 );
		assert ( tRow.m_iColumnarOff + iColumnarAttrs <= tStorage.m_dColumnar.GetLength() );
		memcpy ( tDoc.m_dColumnarAttrs.Begin(), tStorage.m_dColumnar.Begin() + tRow.m_iColumnarOff, iColumnarAttrs * sizeof(SphAttr_t) );
	}

	tDoc.ResetMVAs();
	if ( tRow.m_tMva.m_iCount )
	{
		assert ( tRow.m_tMva.m_iOff >= 0 );
		assert ( tRow.m_tMva.m_iOff + tRow.m_tMva.m_iCount <= tStorage.m_dMvaPool.GetLength() );
		const int64_t * pMva = tStorage.m_dMvaPool.Begin() + tRow.m_tMva.m_iOff;
		for ( int i = 0; i < tRow.m_tMva.m_iCount; ++i )
			tDoc.AddMVAValue ( pMva[i] );
	}

	const int iFields = tSchema.GetFieldsCount();
	if ( iFields )
	{
		assert ( tRow.m_iFieldRefOff >= 0 );
		assert ( tRow.m_iFieldRefOff + iFields <= tStorage.m_dFieldRefs.GetLength() );
	}

	for ( int iField = 0; iField < iFields; ++iField )
	{
		const auto & tRef = tStorage.m_dFieldRefs[tRow.m_iFieldRefOff + iField];
		tDoc.m_dFields[iField] = ( tRef.m_iOff >= 0 && pBlobBase )
			? VecTraits_T<const char> ( (const char *)pBlobBase + tRef.m_iOff, tRef.m_iLen )
			: VecTraits_T<const char> ( nullptr, 0 );
	}

	const int iStringAttrs = GetShardStringAttrCount ( tSchema );
	tDoc.m_dStrings.Resize ( iStringAttrs );
	if ( iStringAttrs )
	{
		assert ( tRow.m_iStringRefOff >= 0 );
		assert ( tRow.m_iStringRefOff + iStringAttrs <= tStorage.m_dStringRefs.GetLength() );
	}

	for ( int iStringAttr = 0; iStringAttr < iStringAttrs; ++iStringAttr )
	{
		const auto & tRef = tStorage.m_dStringRefs[tRow.m_iStringRefOff + iStringAttr];
		tDoc.m_dStrings[iStringAttr] = ( tRef.m_iOff >= 0 && pBlobBase ) ? (const char *)pBlobBase + tRef.m_iOff : nullptr;
	}
}

static void BuildShardCompatSettings ( const ShardIndex_c & tShard, CSphReconfigureSettings & tSettings )
{
	tSettings = CSphReconfigureSettings();
	tSettings.m_tTokenizer = tShard.m_tTokenizerSettings;
	tSettings.m_tDict = tShard.m_tDictSettings;
	tSettings.m_tIndex = tShard.m_tSettings;
	tSettings.m_tFieldFilter = tShard.m_tFieldFilterSettings;
	tSettings.m_tSchema = tShard.m_tSchema;
	tSettings.m_tMutableSettings = MutableIndexSettings_c::GetDefaults();
	tSettings.m_bChangeSchema = true;
}

static bool ValidateShardTargetCompatibility ( const ShardIndex_c & tShard, const CSphString & sShardName, const CSphString & sTargetName, RtIndex_i * pIndex, CSphString & sError )
{
	CSphReconfigureSettings tSettings;
	BuildShardCompatSettings ( tShard, tSettings );

	CSphReconfigureSetup tSetup;
	StrVec_t dWarnings;
	if ( !pIndex->IsSameSettings ( tSettings, tSetup, dWarnings, sError ) )
	{
		if ( sError.IsEmpty() )
			sError.SetSprintf ( "local table '%s' is incompatible with shard table '%s'", sTargetName.cstr(), sShardName.cstr() );
		return false;
	}

	return sError.IsEmpty();
}

static bool CaptureLocalShardTargetGuard ( const ShardIndex_c & tShard, const CSphString & sShardName, ShardIndex_c::RouteTarget_t & tTarget, CSphString & sError )
{
	assert ( tTarget.IsLocal() );

	cServedIndexRefPtr_c pServed { GetServed ( tTarget.m_sName ) };
	if ( !pServed )
	{
		sError.SetSprintf ( "local table '%s' absent", tTarget.m_sName.cstr() );
		return false;
	}

	if ( pServed->m_eType != IndexType_e::RT )
	{
		sError.SetSprintf ( "local table '%s' must be real-time", tTarget.m_sName.cstr() );
		return false;
	}

	RIdx_T<RtIndex_i*> pIndex { pServed };
	if ( !ValidateShardTargetCompatibility ( tShard, sShardName, tTarget.m_sName, pIndex, sError ) )
		return false;

	tTarget.m_iLocalIndexId = pIndex->GetIndexId();
	tTarget.m_iLocalAlterGeneration = pIndex->GetAlterGeneration();
	return true;
}

static bool CaptureLocalShardTargetsGuards ( ShardIndex_c & tShard, const CSphString & sShardName, CSphString & sError )
{
	for ( auto & tTarget : tShard.m_dRouteTargets )
	{
		if ( !tTarget.IsLocal() )
			continue;

		if ( !CaptureLocalShardTargetGuard ( tShard, sShardName, tTarget, sError ) )
			return false;
	}

	return true;
}

static int JumpConsistentHash ( uint64_t uKey, int iBuckets )
{
	assert ( iBuckets > 0 );
	int64_t iBucket = -1;
	int64_t iJump = 0;

	while ( iJump < iBuckets )
	{
		iBucket = iJump;
		uKey = uKey * 2862933555777941757ULL + 1;
		iJump = static_cast<int64_t> ( ( iBucket + 1 ) * ( double ( 1ULL << 31 ) / double ( ( uKey >> 33 ) + 1 ) ) );
	}

	return static_cast<int> ( iBucket );
}

static int GetShardTargetOrdinal ( const ShardIndex_c & tShard, int64_t iDocID )
{
	int iTargets = tShard.m_dRouteTargets.GetLength();
	assert ( iTargets > 0 );
	if ( iTargets == 1 )
		return 0;

	CSphString sDocID;
	sDocID.SetSprintf ( INT64_FMT, iDocID );
	return JumpConsistentHash ( sphCRC32 ( sDocID.cstr(), sDocID.Length() ), iTargets );
}

static const ShardIndex_c::RouteTarget_t * GetShardRouteTarget ( const ShardIndex_c & tShard, int iTargetOrdinal )
{
	if ( iTargetOrdinal < 0 || iTargetOrdinal >= tShard.m_dRouteTargets.GetLength() )
		return nullptr;

	return &tShard.m_dRouteTargets[iTargetOrdinal];
}

static bool ValidateLocalShardTargetGuard ( const ShardIndex_c & tShard, const CSphString & sShardName, const ShardTargetBatch_t & tTarget, RtIndex_i * pIndex, CSphString & sError )
{
	const auto * pRouteTarget = GetShardRouteTarget ( tShard, tTarget.m_iRouteId );
	if ( !pRouteTarget || !pRouteTarget->IsLocal() || pRouteTarget->m_sName != tTarget.m_sIndex )
	{
		sError = "internal error: invalid local shard routing target";
		return false;
	}

	if ( pIndex->GetIndexId() == pRouteTarget->m_iLocalIndexId && pIndex->GetAlterGeneration() == pRouteTarget->m_iLocalAlterGeneration )
		return true;

	sError.SetSprintf ( "local table '%s' for shard table '%s' changed, reload shard table", tTarget.m_sIndex.cstr(), sShardName.cstr() );
	return false;
}

static int FindShardTxnTarget ( const ShardTxnState_t & tShardTxn, int iRouteId )
{
	for ( int i = 0; i < tShardTxn.m_dTargets.GetLength(); ++i )
		if ( tShardTxn.m_dTargets[i].m_iRouteId == iRouteId )
			return i;

	return -1;
}

static ShardTargetBatch_t & AddShardTxnTarget ( ShardTxnState_t & tShardTxn )
{
	auto & tTarget = tShardTxn.m_dTargets.Add();
	tTarget = ShardTargetBatch_t();
	return tTarget;
}

static int FindOrAddShardTxnTarget ( ShardTxnState_t & tShardTxn, const CSphString & sShardName, const ShardIndex_c & tShard, const ShardIndex_c::RouteTarget_t & tRouteTarget )
{
	int iTarget = FindShardTxnTarget ( tShardTxn, tRouteTarget.m_iRouteId );
	if ( iTarget >= 0 )
		return iTarget;

	if ( !tShardTxn.HasPendingData() )
	{
		tShardTxn.m_sShard = sShardName;
		tShardTxn.m_pShardInstance = &tShard;
	}

	auto & tNewTarget = AddShardTxnTarget ( tShardTxn );
	tNewTarget.m_sIndex = tRouteTarget.m_sName;
	tNewTarget.m_iRouteId = tRouteTarget.m_iRouteId;
	tNewTarget.m_bLocal = tRouteTarget.IsLocal();
	if ( !tRouteTarget.IsLocal() )
		tNewTarget.m_pAgent = tRouteTarget.m_pAgent;

	return tShardTxn.m_dTargets.GetLength() - 1;
}

static int64_t GetBufferedShardDocID ( const CSphSchema & tSchema, const ShardDocStorage_t & tStorage, int iRowId )
{
	assert ( iRowId >= 0 && iRowId < tStorage.m_dRows.GetLength() );
	const auto & tRow = tStorage.m_dRows[iRowId];
	const auto * pDocID = tSchema.GetAttr ( sphGetDocidName() );
	assert ( pDocID );

	if ( pDocID->IsColumnar() )
	{
		assert ( tRow.m_iColumnarOff >= 0 && tRow.m_iColumnarOff < tStorage.m_dColumnar.GetLength() );
		return tStorage.m_dColumnar[tRow.m_iColumnarOff];
	}

	const auto * pRow = (const CSphRowitem *)( tStorage.m_dAttrRows.Begin() + tRow.m_iAttrRowOff );
	auto tDocIDLoc = pDocID->m_tLocator;
	tDocIDLoc.m_bDynamic = true;
	return sphGetRowAttr ( pRow, tDocIDLoc );
}

template<typename ACTION>
static bool CollectShardTxnDocIds ( const CSphSchema & tSchema, const ShardTxnState_t & tShardTxn, bool bCommittedOnly, bool bIncludeDeletes, ACTION && tAction )
{
	bool bHaveProcessed = false;
	for ( const auto & tRef : tShardTxn.m_dOrder )
	{
		assert ( tRef.m_iTarget >= 0 && tRef.m_iTarget < tShardTxn.m_dTargets.GetLength() );
		const auto & tTarget = tShardTxn.m_dTargets[tRef.m_iTarget];
		if ( bCommittedOnly && !tTarget.m_bCommitted )
			continue;

		if ( tRef.m_eOp==ShardTxnOp_e::ROW_WRITE )
		{
			tAction ( GetBufferedShardDocID ( tSchema, tShardTxn.m_tStorage, tRef.m_iData ) );
			bHaveProcessed = true;
		}
		else if ( bIncludeDeletes && tRef.m_eOp==ShardTxnOp_e::DOC_DELETE )
		{
			assert ( tRef.m_iData >= 0 );
			assert ( tRef.m_iCount > 0 );
			assert ( tRef.m_iData + tRef.m_iCount <= tTarget.m_dDeleteDocids.GetLength() );
			for ( int i = 0; i < tRef.m_iCount; ++i )
				tAction ( tTarget.m_dDeleteDocids[tRef.m_iData + i] );
			bHaveProcessed = true;
		}
	}

	return bHaveProcessed;
}

static void AppendShardWarning ( CSphString & sWarning, const CSphString & sIncoming )
{
	if ( sIncoming.IsEmpty() )
		return;

	if ( sWarning.IsEmpty() )
		sWarning = sIncoming;
	else
		sWarning.SetSprintf ( "%s; %s", sWarning.cstr(), sIncoming.cstr() );
}

static void AppendShardWarning ( CSphString & sWarning, const char * szIncoming )
{
	if ( !szIncoming || !*szIncoming )
		return;

	AppendShardWarning ( sWarning, CSphString ( szIncoming ) );
}

static void CopyQueryToShardDeletePredicate ( const CSphQuery & tQuery, ShardDeletePredicate_t & tPredicate )
{
	tPredicate.m_sQuery = tQuery.m_sQuery;
	tPredicate.m_dFilters = tQuery.m_dFilters;
	tPredicate.m_dFilterTree = tQuery.m_dFilterTree;
	tPredicate.m_eQueryType = tQuery.m_eQueryType;
}

static void CopyShardDeletePredicateToQuery ( const ShardDeletePredicate_t & tPredicate, CSphQuery & tQuery )
{
	SetQueryDefaultsExt2 ( tQuery );
	tQuery.m_eQueryType = tPredicate.m_eQueryType;
	tQuery.m_sQuery = tPredicate.m_sQuery;
	tQuery.m_dFilters = tPredicate.m_dFilters;
	tQuery.m_dFilterTree = tPredicate.m_dFilterTree;
}

static void SaveFilter ( const CSphFilterSettings & tFilter, ISphOutputBuffer & tOut )
{
	tOut.SendString ( tFilter.m_sAttrName.cstr() );
	tOut.SendDword ( (DWORD)tFilter.m_eType );
	tOut.SendDword ( (DWORD)tFilter.m_eMvaFunc );
	tOut.SendUint64 ( tFilter.m_iMinValue );
	tOut.SendUint64 ( tFilter.m_iMaxValue );

	tOut.SendInt ( tFilter.GetNumValues() );
	for ( auto uValue : tFilter.GetValues() )
		tOut.SendUint64 ( uValue );

	tOut.SendInt ( tFilter.m_dStrings.GetLength() );
	for ( const auto & sValue : tFilter.m_dStrings )
		tOut.SendString ( sValue.cstr() );

	tOut.SendByte ( tFilter.m_bExclude ? 1 : 0 );
	tOut.SendByte ( tFilter.m_bHasEqualMin ? 1 : 0 );
	tOut.SendByte ( tFilter.m_bHasEqualMax ? 1 : 0 );
	tOut.SendByte ( tFilter.m_bOpenLeft ? 1 : 0 );
	tOut.SendByte ( tFilter.m_bOpenRight ? 1 : 0 );
	tOut.SendByte ( tFilter.m_bIsNull ? 1 : 0 );
}

static bool LoadFilter ( InputBuffer_c & tReq, CSphFilterSettings & tFilter, CSphString & sError )
{
	tFilter.m_sAttrName = tReq.GetString();
	tFilter.m_eType = (ESphFilter)tReq.GetDword();
	tFilter.m_eMvaFunc = (ESphMvaFunc)tReq.GetDword();
	tFilter.m_iMinValue = tReq.GetUint64();
	tFilter.m_iMaxValue = tReq.GetUint64();

	int iGot = 0;
	if ( !tReq.GetQwords ( tFilter.m_dValues, iGot, std::numeric_limits<int>::max() ) )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	int iStrings = tReq.GetInt();
	if ( iStrings < 0 )
	{
		sError = "invalid shard delete predicate strings count";
		return false;
	}

	tFilter.m_dStrings.Resize ( iStrings );
	for ( auto & sValue : tFilter.m_dStrings )
		sValue = tReq.GetString();

	tFilter.m_bExclude = tReq.GetByte()!=0;
	tFilter.m_bHasEqualMin = tReq.GetByte()!=0;
	tFilter.m_bHasEqualMax = tReq.GetByte()!=0;
	tFilter.m_bOpenLeft = tReq.GetByte()!=0;
	tFilter.m_bOpenRight = tReq.GetByte()!=0;
	tFilter.m_bIsNull = tReq.GetByte()!=0;

	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	return true;
}

static void SaveUpdate ( const CSphQuery & tQuery, ISphOutputBuffer & tOut )
{
	tOut.SendString ( tQuery.m_sQuery.cstr() );
	tOut.SendInt ( tQuery.m_dFilters.GetLength() );
	for ( const auto & tFilter : tQuery.m_dFilters )
		SaveFilter ( tFilter, tOut );

	tOut.SendInt ( tQuery.m_dFilterTree.GetLength() );
	if ( !tQuery.m_dFilterTree.IsEmpty() )
		tOut.SendBytes ( tQuery.m_dFilterTree.Begin(), tQuery.m_dFilterTree.GetLengthBytes() );

	tOut.SendDword ( (DWORD)tQuery.m_eQueryType );
}

static void SaveShardDeletePredicate ( const ShardDeletePredicate_t & tPredicate, ISphOutputBuffer & tOut )
{
	CSphQuery tQuery;
	CopyShardDeletePredicateToQuery ( tPredicate, tQuery );
	SaveUpdate ( tQuery, tOut );
}

static bool LoadUpdate ( InputBuffer_c & tReq, CSphQuery & tQuery, CSphString & sError )
{
	tQuery.m_sQuery = tReq.GetString();

	int iFilters = tReq.GetInt();
	if ( iFilters < 0 )
	{
		sError = "invalid shard delete predicate filters count";
		return false;
	}

	tQuery.m_dFilters.Resize ( iFilters );
	for ( auto & tFilter : tQuery.m_dFilters )
	{
		if ( !LoadFilter ( tReq, tFilter, sError ) )
			return false;
	}

	int iFilterTree = tReq.GetInt();
	if ( iFilterTree < 0 )
	{
		sError = "invalid shard delete predicate filter tree count";
		return false;
	}

	tQuery.m_dFilterTree.Resize ( iFilterTree );
	if ( iFilterTree && !tReq.GetBytes ( tQuery.m_dFilterTree.Begin(), (int)tQuery.m_dFilterTree.GetLengthBytes() ) )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	tQuery.m_eQueryType = (QueryType_e)tReq.GetDword();
	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	return true;
}

static bool LoadShardDeletePredicate ( InputBuffer_c & tReq, ShardDeletePredicate_t & tPredicate, CSphString & sError )
{
	CSphQuery tQuery;
	if ( !LoadUpdate ( tReq, tQuery, sError ) )
		return false;

	CopyQueryToShardDeletePredicate ( tQuery, tPredicate );
	return true;
}

template<typename T>
static void SendShardArray ( const VecTraits_T<T> & dBuf, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dBuf.GetLength() );
	if ( dBuf.GetLength() )
		tOut.SendBytes ( dBuf.Begin(), dBuf.GetLengthBytes() );
}

template<typename T>
static void ReceiveShardArray ( InputBuffer_c & tReq, CSphVector<T> & dBuf )
{
	dBuf.Resize ( 0 );
	int iCount = tReq.GetInt();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	if ( iCount )
		tReq.GetBytes ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
}

static void SendShardSpanRefs ( const CSphVector<ShardSpanRef_t> & dRefs, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dRefs.GetLength() );
	for ( const auto & tRef : dRefs )
	{
		tOut.SendInt ( tRef.m_iOff );
		tOut.SendInt ( tRef.m_iLen );
	}
}

static void ReceiveShardSpanRefs ( InputBuffer_c & tReq, CSphVector<ShardSpanRef_t> & dRefs )
{
	dRefs.Resize ( 0 );
	int iCount = tReq.GetInt();
	if ( !iCount )
		return;

	dRefs.Resize ( iCount );
	for ( auto & tRef : dRefs )
	{
		tRef.m_iOff = tReq.GetInt();
		tRef.m_iLen = tReq.GetInt();
	}
}

static ShardSpanRef_t StoreShardRawSpan ( CSphVector<BYTE> & dBlobPool, const BYTE * pData, int iLen, bool bAllowEmpty )
{
	ShardSpanRef_t tRef;
	if ( iLen < 0 || ( !pData && iLen > 0 ) )
		return tRef;

	if ( !bAllowEmpty && iLen <= 0 )
		return tRef;

	tRef.m_iOff = dBlobPool.GetLength();
	tRef.m_iLen = iLen;
	if ( iLen > 0 )
		dBlobPool.Append ( pData, iLen );
	return tRef;
}

static void GetShardRawSpan ( const ShardDocStorage_t & tStorage, const ShardSpanRef_t & tRef, const BYTE * & pData, int & iLen )
{
	pData = nullptr;
	iLen = 0;
	if ( tRef.m_iOff < 0 )
		return;

	iLen = tRef.m_iLen;
	pData = tStorage.m_dBlobPool.IsEmpty() ? nullptr : tStorage.m_dBlobPool.Begin() + tRef.m_iOff;
}

static void GetShardStringSpan ( const ShardDocStorage_t & tStorage, const ShardSpanRef_t & tRef, const char * & szData )
{
	szData = nullptr;
	const BYTE * pData = nullptr;
	int iLen = 0;
	GetShardRawSpan ( tStorage, tRef, pData, iLen );

	if ( tRef.m_iOff < 0 )
		return;

	szData = (const char *)pData;
}

static void AppendShardRowCopy ( const CSphSchema & tSchema, const ShardDocStorage_t & tSrc, int iRowId, ShardDocStorage_t & tDst, int iTxnOrdinal )
{
	assert ( iRowId >= 0 && iRowId < tSrc.m_dRows.GetLength() );
	const auto & tSrcRow = tSrc.m_dRows[iRowId];
	ShardBufferedRow_t tDstRow;
	tDstRow.m_bReplace = tSrcRow.m_bReplace;
	tDstRow.m_iTxnOrdinal = iTxnOrdinal;

	const int iAttrRowBytes = tSchema.GetRowSize() * sizeof(CSphRowitem);
	if ( iAttrRowBytes )
	{
		assert ( tSrcRow.m_iAttrRowOff >= 0 );
		assert ( tSrcRow.m_iAttrRowOff + iAttrRowBytes <= tSrc.m_dAttrRows.GetLength() );
		tDstRow.m_iAttrRowOff = tDst.m_dAttrRows.GetLength();
		tDst.m_dAttrRows.Append ( tSrc.m_dAttrRows.Begin() + tSrcRow.m_iAttrRowOff, iAttrRowBytes );
	}

	const int iColumnarAttrs = tSchema.GetColumnarAttrsCount();
	if ( iColumnarAttrs )
	{
		assert ( tSrcRow.m_iColumnarOff >= 0 );
		assert ( tSrcRow.m_iColumnarOff + iColumnarAttrs <= tSrc.m_dColumnar.GetLength() );
		tDstRow.m_iColumnarOff = tDst.m_dColumnar.GetLength();
		tDst.m_dColumnar.Append ( tSrc.m_dColumnar.Begin() + tSrcRow.m_iColumnarOff, iColumnarAttrs );
	}

	if ( tSrcRow.m_tMva.m_iCount )
	{
		assert ( tSrcRow.m_tMva.m_iOff >= 0 );
		assert ( tSrcRow.m_tMva.m_iOff + tSrcRow.m_tMva.m_iCount <= tSrc.m_dMvaPool.GetLength() );
		tDstRow.m_tMva.m_iOff = tDst.m_dMvaPool.GetLength();
		tDstRow.m_tMva.m_iCount = tSrcRow.m_tMva.m_iCount;
		tDst.m_dMvaPool.Append ( tSrc.m_dMvaPool.Begin() + tSrcRow.m_tMva.m_iOff, tSrcRow.m_tMva.m_iCount );
	}

	const int iFields = tSchema.GetFieldsCount();
	if ( iFields )
	{
		assert ( tSrcRow.m_iFieldRefOff >= 0 );
		assert ( tSrcRow.m_iFieldRefOff + iFields <= tSrc.m_dFieldRefs.GetLength() );
		tDstRow.m_iFieldRefOff = tDst.m_dFieldRefs.GetLength();
		auto * pDstFieldRefs = tDst.m_dFieldRefs.AddN ( iFields );
		for ( int iField = 0; iField < iFields; ++iField )
		{
			const BYTE * pData = nullptr;
			int iLen = 0;
			GetShardRawSpan ( tSrc, tSrc.m_dFieldRefs[tSrcRow.m_iFieldRefOff + iField], pData, iLen );
			pDstFieldRefs[iField] = StoreShardRawSpan ( tDst.m_dBlobPool, pData, iLen, false );
		}
	}

	const int iStringAttrs = GetShardStringAttrCount ( tSchema );
	if ( iStringAttrs )
	{
		assert ( tSrcRow.m_iStringRefOff >= 0 );
		assert ( tSrcRow.m_iStringRefOff + iStringAttrs <= tSrc.m_dStringRefs.GetLength() );
		tDstRow.m_iStringRefOff = tDst.m_dStringRefs.GetLength();
		auto * pDstStringRefs = tDst.m_dStringRefs.AddN ( iStringAttrs );
		int iStringAttr = 0;
		for ( int iAttr = 0; iAttr < tSchema.GetAttrsCount(); ++iAttr )
		{
			const auto & tAttr = tSchema.GetAttr(iAttr);
			if ( !IsShardStringAttr ( tAttr.m_eAttrType ) )
				continue;

			const auto & tSrcRef = tSrc.m_dStringRefs[tSrcRow.m_iStringRefOff + iStringAttr];
			if ( tAttr.m_eAttrType == SPH_ATTR_JSON )
			{
				const BYTE * pData = nullptr;
				int iLen = 0;
				GetShardRawSpan ( tSrc, tSrcRef, pData, iLen );
				pDstStringRefs[iStringAttr] = StoreShardRawSpan ( tDst.m_dBlobPool, pData, iLen, true );
			} else
			{
				const char * szData = nullptr;
				GetShardStringSpan ( tSrc, tSrcRef, szData );
				pDstStringRefs[iStringAttr] = StoreShardString ( tDst.m_dBlobPool, szData );
			}

			++iStringAttr;
		}
	}

	tDst.m_dRows.Add ( tDstRow );
}

static void BuildShardRemoteBatch ( const CSphSchema & tSchema, const ShardTargetBatch_t & tTarget, const ShardDocStorage_t & tSrcStorage, const CSphVector<ShardDeletePredicate_t> & dSrcPredicates, ShardDocStorage_t & tDstStorage, CSphVector<int64_t> & dDeleteDocids, CSphVector<ShardDeletePredicate_t> & dDeletePredicates, CSphVector<ShardBatchOp_t> & dOps )
{
	tDstStorage.Cleanup();
	dDeleteDocids.Resize ( 0 );
	dDeletePredicates.Resize ( 0 );
	dOps.Resize ( 0 );

	for ( const auto & tOp : tTarget.m_dOps )
	{
		auto & tDstOp = dOps.Add();
		tDstOp.m_eOp = tOp.m_eOp;
		tDstOp.m_iCount = tOp.m_iCount;
		if ( tOp.m_eOp==ShardTxnOp_e::ROW_WRITE )
		{
			AppendShardRowCopy ( tSchema, tSrcStorage, tOp.m_iData, tDstStorage, tDstStorage.m_dRows.GetLength() );
			tDstOp.m_iData = tDstStorage.m_dRows.GetLength() - 1;
		} else if ( tOp.m_eOp==ShardTxnOp_e::DOC_DELETE )
		{
			assert ( tOp.m_iData >= 0 );
			assert ( tOp.m_iCount > 0 );
			assert ( tOp.m_iData + tOp.m_iCount <= tTarget.m_dDeleteDocids.GetLength() );
			tDstOp.m_iData = dDeleteDocids.GetLength();
			dDeleteDocids.Append ( tTarget.m_dDeleteDocids.Begin() + tOp.m_iData, tOp.m_iCount );
		} else
		{
			assert ( tOp.m_iData >= 0 && tOp.m_iData < dSrcPredicates.GetLength() );
			tDstOp.m_iData = dDeletePredicates.GetLength();
			dDeletePredicates.Add ( dSrcPredicates[tOp.m_iData] );
		}
	}
}

static void SendShardWriteStorage ( const ShardDocStorage_t & tStorage, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( tStorage.m_dRows.GetLength() );
	SendShardArray ( tStorage.m_dAttrRows, tOut );
	SendShardArray ( tStorage.m_dColumnar, tOut );
	SendShardArray ( tStorage.m_dMvaPool, tOut );
	SendShardSpanRefs ( tStorage.m_dFieldRefs, tOut );
	SendShardSpanRefs ( tStorage.m_dStringRefs, tOut );
	SendShardArray ( tStorage.m_dBlobPool, tOut );
	for ( const auto & tRow : tStorage.m_dRows )
	{
		tOut.SendInt ( tRow.m_tMva.m_iOff );
		tOut.SendInt ( tRow.m_tMva.m_iCount );
		tOut.SendByte ( tRow.m_bReplace ? 1 : 0 );
	}
}

static bool ReceiveShardWriteStorage ( InputBuffer_c & tReq, const CSphSchema & tSchema, ShardDocStorage_t & tStorage, CSphString & sError )
{
	tStorage.Cleanup();
	int iRows = tReq.GetInt();
	if ( iRows < 0 )
	{
		sError = "invalid shard write rows length";
		return false;
	}

	ReceiveShardArray ( tReq, tStorage.m_dAttrRows );
	ReceiveShardArray ( tReq, tStorage.m_dColumnar );
	ReceiveShardArray ( tReq, tStorage.m_dMvaPool );
	ReceiveShardSpanRefs ( tReq, tStorage.m_dFieldRefs );
	ReceiveShardSpanRefs ( tReq, tStorage.m_dStringRefs );
	ReceiveShardArray ( tReq, tStorage.m_dBlobPool );
	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	const int iAttrRowBytes = tSchema.GetRowSize() * sizeof(CSphRowitem);
	const int iColumnarAttrs = tSchema.GetColumnarAttrsCount();
	const int iFields = tSchema.GetFieldsCount();
	const int iStringAttrs = GetShardStringAttrCount ( tSchema );

	tStorage.m_dRows.Resize ( iRows );
	for ( int iRow = 0; iRow < iRows; ++iRow )
	{
		auto & tRow = tStorage.m_dRows[iRow];
		tRow.m_iAttrRowOff = iAttrRowBytes ? iRow * iAttrRowBytes : -1;
		tRow.m_iColumnarOff = iColumnarAttrs ? iRow * iColumnarAttrs : -1;
		tRow.m_iFieldRefOff = iFields ? iRow * iFields : -1;
		tRow.m_iStringRefOff = iStringAttrs ? iRow * iStringAttrs : -1;
		tRow.m_tMva.m_iOff = tReq.GetInt();
		tRow.m_tMva.m_iCount = tReq.GetInt();
		tRow.m_bReplace = tReq.GetByte()!=0;
		tRow.m_iTxnOrdinal = iRow;
	}

	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	return true;
}

static bool ShardBatchHasRowWrites ( const CSphVector<ShardBatchOp_t> & dOps )
{
	for ( const auto & tOp : dOps )
		if ( tOp.m_eOp==ShardTxnOp_e::ROW_WRITE )
			return true;

	return false;
}

static void SendShardBatchOps ( const CSphVector<ShardBatchOp_t> & dOps, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dOps.GetLength() );
	for ( const auto & tOp : dOps )
	{
		tOut.SendByte ( (BYTE)tOp.m_eOp );
		tOut.SendInt ( tOp.m_iData );
		tOut.SendInt ( tOp.m_iCount );
	}
}

static void SendShardDeletePredicates ( const CSphVector<ShardDeletePredicate_t> & dPredicates, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dPredicates.GetLength() );
	for ( const auto & tPredicate : dPredicates )
		SaveShardDeletePredicate ( tPredicate, tOut );
}

static bool ReceiveShardDeletePredicates ( InputBuffer_c & tReq, CSphVector<ShardDeletePredicate_t> & dPredicates, CSphString & sError )
{
	dPredicates.Resize ( 0 );
	int iCount = tReq.GetInt();
	if ( iCount < 0 )
	{
		sError = "invalid shard write delete predicate count";
		return false;
	}

	for ( int i = 0; i < iCount; ++i )
	{
		auto & tPredicate = dPredicates.Add();
		if ( !LoadShardDeletePredicate ( tReq, tPredicate, sError ) )
			return false;
	}

	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	return true;
}

static bool ReceiveShardBatchOps ( InputBuffer_c & tReq, CSphVector<ShardBatchOp_t> & dOps, CSphString & sError )
{
	dOps.Resize ( 0 );
	int iCount = tReq.GetInt();
	if ( iCount < 0 )
	{
		sError = "invalid shard write ops length";
		return false;
	}

	dOps.Resize ( iCount );
	for ( auto & tOp : dOps )
	{
		tOp.m_eOp = (ShardTxnOp_e)tReq.GetByte();
		tOp.m_iData = tReq.GetInt();
		tOp.m_iCount = tReq.GetInt();
	}

	if ( tReq.GetError() )
	{
		sError = tReq.GetErrorMessage();
		return false;
	}

	return true;
}

static bool CollectDeleteDocidsFromTargetTable ( const CSphString & sTargetName, const cServedIndexRefPtr_c & pServed, const ShardDeletePredicate_t & tPredicate, CSphVector<DocID_t> & dDocids, CSphString & sError )
{
	dDocids.Resize ( 0 );

	CSphQuery tQuery;
	CopyShardDeletePredicateToQuery ( tPredicate, tQuery );
	const bool bJson = ( tPredicate.m_eQueryType==QUERY_JSON );
	DocsCollector_c tCollector { tQuery, bJson, sTargetName, pServed, &sError };
	if ( !sError.IsEmpty() )
		return false;

	dDocids.Append ( tCollector.GetValuesSlice() );
	return true;
}

static bool CommitBufferedShardBatch ( const CSphSchema & tSchema, const cServedIndexRefPtr_c & pServed, RtIndex_i * pIndex, const CSphString & sTargetName, const CSphString & sCluster, const CSphVector<ShardBatchOp_t> & dOps, const CSphVector<int64_t> & dDeleteDocids, const CSphVector<ShardDeletePredicate_t> & dDeletePredicates, const ShardDocStorage_t & tStorage, int & iDeleted, CSphString & sWarning, CSphString & sError )
{
	iDeleted = 0;
	if ( dOps.IsEmpty() )
		return true;

	RtAccum_t tAccum;
	CSphVector<DocID_t> dPredicateDeleteDocs;
	for ( const auto & tOp : dOps )
	{
		if ( tOp.m_eOp==ShardTxnOp_e::ROW_WRITE )
		{
			InsertDocData_c tDoc ( tSchema );
			bool bReplace = false;
			RowToInsert ( tSchema, tStorage, tOp.m_iData, tDoc, bReplace );
			if ( !pIndex->AddDocument ( tDoc, bReplace, CSphString(), sError, sWarning, &tAccum ) || !sError.IsEmpty() )
			{
				pIndex->RollBack ( &tAccum );
				return false;
			}
		} else if ( tOp.m_eOp==ShardTxnOp_e::DOC_DELETE )
		{
			assert ( tOp.m_iData >= 0 );
			assert ( tOp.m_iCount > 0 );
			assert ( tOp.m_iData + tOp.m_iCount <= dDeleteDocids.GetLength() );
			VecTraits_T<const DocID_t> dDeleteDocs ( dDeleteDocids.Begin() + tOp.m_iData, tOp.m_iCount );
			if ( !pIndex->DeleteDocument ( dDeleteDocs, sError, &tAccum ) )
			{
				pIndex->RollBack ( &tAccum );
				return false;
			}
		} else
		{
			assert ( tOp.m_iData >= 0 && tOp.m_iData < dDeletePredicates.GetLength() );
			if ( !CollectDeleteDocidsFromTargetTable ( sTargetName, pServed, dDeletePredicates[tOp.m_iData], dPredicateDeleteDocs, sError ) )
			{
				if ( tAccum.GetIndex() )
					pIndex->RollBack ( &tAccum );
				return false;
			}

			if ( dPredicateDeleteDocs.IsEmpty() )
				continue;

			if ( !pIndex->DeleteDocument ( dPredicateDeleteDocs, sError, &tAccum ) )
			{
				pIndex->RollBack ( &tAccum );
				return false;
			}
		}
	}

	if ( !tAccum.GetIndex() )
		return true;

	tAccum.AddCommand ( ReplCmd_e::RT_TRX, sTargetName, sCluster );
	if ( !HandleCmdReplicateDelete ( tAccum, iDeleted ) )
	{
		TlsMsg::MoveError ( sError );
		if ( tAccum.GetIndex() )
			pIndex->RollBack ( &tAccum );
		return false;
	}

	return true;
}

class ShardWriteRequestBuilder_c : public RequestBuilder_i
{
public:
	ShardWriteRequestBuilder_c ( CSphString sTargetName, ShardDocStorage_t tStorage, CSphVector<int64_t> dDeleteDocids, CSphVector<ShardDeletePredicate_t> dDeletePredicates, CSphVector<ShardBatchOp_t> dOps )
		: m_sTargetName ( std::move ( sTargetName ) )
		, m_tStorage ( std::move ( tStorage ) )
		, m_dDeleteDocids ( std::move ( dDeleteDocids ) )
		, m_dDeletePredicates ( std::move ( dDeletePredicates ) )
		, m_dOps ( std::move ( dOps ) )
	{
		assert ( m_tStorage.m_dRows.GetLength() || m_dDeleteDocids.GetLength() || m_dDeletePredicates.GetLength() || m_dOps.IsEmpty() );
	}

	void BuildRequest ( const AgentConn_t &, ISphOutputBuffer & tOut ) const final
	{
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SHARD_WRITE, VER_COMMAND_SHARD_WRITE );
		tOut.SendString ( m_sTargetName.cstr() );
		SendShardWriteStorage ( m_tStorage, tOut );
		SendShardArray ( m_dDeleteDocids, tOut );
		SendShardDeletePredicates ( m_dDeletePredicates, tOut );
		SendShardBatchOps ( m_dOps, tOut );
	}

private:
	CSphString							m_sTargetName;
	ShardDocStorage_t	m_tStorage;
	CSphVector<int64_t>					m_dDeleteDocids;
	CSphVector<ShardDeletePredicate_t>	m_dDeletePredicates;
	CSphVector<ShardBatchOp_t>			m_dOps;
};

class ShardWriteReplyParser_c : public ReplyParser_i
{
public:
	ShardWriteReplyParser_c ( int * pAffected, CSphString * pWarning )
		: m_pAffected ( pAffected )
		, m_pWarning ( pWarning )
	{}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		*m_pAffected += tReq.GetInt();
		if ( m_pWarning && tAgent.m_sFailure.Begins ( "remote warning: " ) )
			AppendShardWarning ( *m_pWarning, tAgent.m_sFailure.cstr() + strlen ( "remote warning: " ) );
		return true;
	}

private:
	int *			m_pAffected = nullptr;
	CSphString *	m_pWarning = nullptr;
};

static bool ApplyLocalShardBatch ( const ShardIndex_c & tShard, const CSphString & sShardName, const CSphSchema & tSchema, ShardTargetBatch_t & tTarget, const ShardDocStorage_t & tStorage, const CSphVector<ShardDeletePredicate_t> & dDeletePredicates, int & iDeleted, CSphString & sWarning, CSphString & sError )
{
	iDeleted = 0;
	cServedIndexRefPtr_c pServed { GetServed ( tTarget.m_sIndex ) };
	if ( !pServed )
	{
		sError.SetSprintf ( "table '%s' absent", tTarget.m_sIndex.cstr() );
		return false;
	}

	if ( pServed->m_eType != IndexType_e::RT )
	{
		sError.SetSprintf ( "local table '%s' must be real-time", tTarget.m_sIndex.cstr() );
		return false;
	}

	Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
	if ( !tWriting.CanWrite() )
	{
		sError.SetSprintf ( "table '%s' is locked", tTarget.m_sIndex.cstr() );
		return false;
	}

	RIdx_T<RtIndex_i*> pIndex { pServed };
	if ( ShardBatchHasRowWrites ( tTarget.m_dOps ) && !ValidateLocalShardTargetGuard ( tShard, sShardName, tTarget, pIndex, sError ) )
		return false;

	if ( tTarget.m_dOps.IsEmpty() )
	{
		tTarget.m_sCluster = pServed->m_sCluster;
		tTarget.m_bCommitted = true;
		return true;
	}

	if ( !CommitBufferedShardBatch ( tSchema, pServed, pIndex, tTarget.m_sIndex, pServed->m_sCluster, tTarget.m_dOps, tTarget.m_dDeleteDocids, dDeletePredicates, tStorage, iDeleted, sWarning, sError ) )
		return false;

	tTarget.m_sCluster = pServed->m_sCluster;
	tTarget.m_bCommitted = true;
	return true;
}

static bool ApplyRemoteShardBatch ( const ShardIndex_c & tShard, const CSphString & sShardName, const CSphSchema & tSchema, ShardTargetBatch_t & tTarget, const ShardDocStorage_t & tStorage, const CSphVector<ShardDeletePredicate_t> & dDeletePredicates, int & iDeleted, CSphString & sWarning, CSphString & sError )
{
	iDeleted = 0;
	if ( tTarget.m_dOps.IsEmpty() )
	{
		tTarget.m_bCommitted = true;
		return true;
	}

	if ( !tTarget.m_pAgent || !tTarget.m_pAgent->GetLength() )
	{
		sError.SetSprintf ( "table '%s': invalid remote shard target '%s'", sShardName.cstr(), tTarget.m_sIndex.cstr() );
		return false;
	}

	if ( !tTarget.m_bHasPinnedAgent )
	{
		tTarget.m_tPinnedAgent.CloneFrom ( tTarget.m_pAgent->ChooseAgent() );
		tTarget.m_bHasPinnedAgent = true;
	}

	ShardDocStorage_t tBatchStorage;
	CSphVector<int64_t> dDeleteDocids;
	CSphVector<ShardDeletePredicate_t> dBatchDeletePredicates;
	CSphVector<ShardBatchOp_t> dOps;
	BuildShardRemoteBatch ( tSchema, tTarget, tStorage, dDeletePredicates, tBatchStorage, dDeleteDocids, dBatchDeletePredicates, dOps );

	VecRefPtrsAgentConn_t dAgents;
	auto * pConn = new AgentConn_t;
	pConn->m_tDesc.CloneFrom ( tTarget.m_tPinnedAgent );
	pConn->m_iMyConnectTimeoutMs = tShard.GetAgentConnectTimeoutMs();
	pConn->m_iMyQueryTimeoutMs = tShard.GetAgentQueryTimeoutMs();
	dAgents.Add ( pConn );

	ShardWriteRequestBuilder_c tReqBuilder ( tTarget.m_sIndex, std::move ( tBatchStorage ), std::move ( dDeleteDocids ), std::move ( dBatchDeletePredicates ), std::move ( dOps ) );
	ShardWriteReplyParser_c tReplyParser ( &iDeleted, &sWarning );
	int iSuccesses = PerformRemoteTasks ( dAgents, &tReqBuilder, &tReplyParser, 0 );
	if ( iSuccesses!=1 || !dAgents[0]->m_bSuccess )
	{
		const AgentConn_t * pAgent = dAgents[0];
		if ( pAgent && !pAgent->m_sFailure.IsEmpty() )
			sError.SetSprintf ( "remote table '%s' at %s failed: %s", tTarget.m_sIndex.cstr(), pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
		else
			sError.SetSprintf ( "remote table '%s' failed", tTarget.m_sIndex.cstr() );
		return false;
	}

	tTarget.m_bCommitted = true;
	return true;
}

bool CommitShardTxn ( ClientSession_c & tSession, CSphString & sError, CSphVector<int64_t> * pCommittedDocIDs, int * pDeletedCount )
{
	auto & tShardTxn = tSession.m_tShardTxn;
	if ( !tShardTxn.HasPendingData() )
	{
		if ( pCommittedDocIDs )
			pCommittedDocIDs->Resize ( 0 );
		if ( pDeletedCount )
			*pDeletedCount = 0;
		return true;
	}

	auto pDist = GetDistr ( tShardTxn.m_sShard );
	const ShardIndex_c * pShard = AsShard ( pDist.Ptr() );
	if ( !pShard )
	{
		sError.SetSprintf ( "table '%s' absent", tShardTxn.m_sShard.cstr() );
		tShardTxn.Cleanup();
		if ( pCommittedDocIDs )
			pCommittedDocIDs->Resize ( 0 );
		if ( pDeletedCount )
			*pDeletedCount = 0;
		return false;
	}

	if ( tShardTxn.m_pShardInstance && tShardTxn.m_pShardInstance!=pShard )
	{
		if ( pCommittedDocIDs )
			pCommittedDocIDs->Resize ( 0 );
		if ( pDeletedCount )
			*pDeletedCount = 0;
		return InvalidateChangedShardTxn ( tShardTxn, sError, true );
	}

	for ( auto & tTarget : tShardTxn.m_dTargets )
		tTarget.m_bCommitted = false;

	bool bOk = true;
	int iDeleted = 0;
	auto & sWarning = tSession.m_tLastMeta.m_sWarning;
	for ( auto & tTarget : tShardTxn.m_dTargets )
	{
		int iTargetDeleted = 0;
		bool bTargetOk = tTarget.m_bLocal
			? ApplyLocalShardBatch ( *pShard, tShardTxn.m_sShard, pShard->m_tSchema, tTarget, tShardTxn.m_tStorage, tShardTxn.m_dDeletePredicates, iTargetDeleted, sWarning, sError )
			: ApplyRemoteShardBatch ( *pShard, tShardTxn.m_sShard, pShard->m_tSchema, tTarget, tShardTxn.m_tStorage, tShardTxn.m_dDeletePredicates, iTargetDeleted, sWarning, sError );
		if ( !bTargetOk )
		{
			bOk = false;
			break;
		}

		iDeleted += iTargetDeleted;
	}

	if ( !bOk )
	{
		StringBuilder_c sProcessedDocIDs ( "," );
		bool bGotValues = CollectShardTxnDocIds ( pShard->m_tSchema, tShardTxn, !bOk, true, [&] ( int64_t iDocID )
		{
			sProcessedDocIDs << iDocID;
		});
		if ( bGotValues )
			sError.SetSprintf ( "%s; processed_ids=%s", sError.cstr(), sProcessedDocIDs.cstr() );
	}

	// Shard commits can partially succeed across targets; on failure expose only IDs that were actually committed.
	if ( pCommittedDocIDs )
	{	
		pCommittedDocIDs->Resize ( 0 );
		CollectShardTxnDocIds ( pShard->m_tSchema, tShardTxn, !bOk, false, [&] ( int64_t iDocID )
		{
			pCommittedDocIDs->Add ( iDocID );
		});
	}

	tShardTxn.Cleanup();
	if ( pDeletedCount )
		*pDeletedCount = iDeleted;

	return bOk;
}

void HandleCommandShardWrite ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SHARD_WRITE, tOut ) )
		return;

	CSphString sTargetName = tReq.GetString();

	cServedIndexRefPtr_c pServed { GetServed ( sTargetName ) };
	if ( !pServed )
	{
		auto pLogical = GetDistr ( sTargetName );
		if ( pLogical )
			SendErrorReply ( tOut, "remote table '%s' must be a concrete real-time table", sTargetName.cstr() );
		else
			SendErrorReply ( tOut, "table '%s' absent", sTargetName.cstr() );
		return;
	}

	if ( pServed->m_eType != IndexType_e::RT )
	{
		SendErrorReply ( tOut, "remote table '%s' must be real-time", sTargetName.cstr() );
		return;
	}

	Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
	if ( !tWriting.CanWrite() )
	{
		SendErrorReply ( tOut, "table '%s' is locked", sTargetName.cstr() );
		return;
	}

	CSphString sError;
	RIdx_T<RtIndex_i*> pIndex { pServed };
	ShardDocStorage_t tStorage;
	if ( !ReceiveShardWriteStorage ( tReq, pIndex->GetInternalSchema(), tStorage, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	CSphVector<int64_t> dDeleteDocids;
	ReceiveShardArray ( tReq, dDeleteDocids );

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "%s", tReq.GetErrorMessage().cstr() );
		return;
	}

	CSphVector<ShardDeletePredicate_t> dDeletePredicates;
	if ( !ReceiveShardDeletePredicates ( tReq, dDeletePredicates, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	CSphVector<ShardBatchOp_t> dOps;
	if ( !ReceiveShardBatchOps ( tReq, dOps, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	CSphString sWarning;
	int iDeleted = 0;
	if ( !CommitBufferedShardBatch ( pIndex->GetInternalSchema(), pServed, pIndex, sTargetName, pServed->m_sCluster, dOps, dDeleteDocids, dDeletePredicates, tStorage, iDeleted, sWarning, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	auto tReply = APIAnswer ( tOut, uVer, sWarning.IsEmpty() ? SEARCHD_OK : SEARCHD_WARNING );
	if ( !sWarning.IsEmpty() )
		tOut.SendString ( sWarning.cstr() );
	tOut.SendInt ( iDeleted );
}

static bool ValidateShardDeleteStatement ( const SqlStmt_t & tStmt, CSphString & sError )
{
	const auto & tQuery = tStmt.m_tQuery;
	if ( !tQuery.m_sStore.IsEmpty() )
	{
		sError = "shard DELETE does not support STORE";
		return false;
	}

	return true;
}

static bool TryExtractShardDeleteDocids ( const SqlStmt_t & tStmt, CSphVector<int64_t> & dDocids )
{
	dDocids.Resize ( 0 );

	const auto & tQuery = tStmt.m_tQuery;
	if ( !tQuery.m_sQuery.IsEmpty() || !tQuery.m_dFilterTree.IsEmpty() || tQuery.m_dFilters.GetLength()!=1 )
		return false;

	const auto * pFilter = tQuery.m_dFilters.Begin();
	if ( !( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax )
		|| pFilter->m_bExclude
		|| pFilter->m_eType!=SPH_FILTER_VALUES
		|| ( pFilter->m_sAttrName!="@id" && pFilter->m_sAttrName!="id" ) )
		return false;

	dDocids.Append ( pFilter->GetValues() );
	return true;
}

static void ExtractShardDeletePredicate ( const SqlStmt_t & tStmt, ShardDeletePredicate_t & tPredicate )
{
	CopyQueryToShardDeletePredicate ( tStmt.m_tQuery, tPredicate );
}

bool AddDocumentShard ( const SqlStmt_t & tStmt, const ShardIndex_c & tShard, StmtErrorReporter_i & tOut, int & iAffectedRows )
{
	auto * pSession = session::GetClientSession();
	auto & sWarning = pSession->m_tLastMeta.m_sWarning;
	auto & dLastIds = pSession->m_dLastIds;
	const bool bReplaceStmt = ( tStmt.m_eStmt == STMT_REPLACE );
	iAffectedRows = 0;

	CSphString sError;
	if ( !ValidateClusterStatement ( tStmt.m_sIndex, tShard.m_sCluster, tStmt.m_sCluster, IsHttpStmt ( tStmt ) ) )
		TlsMsg::MoveError ( sError );

	if ( !sError.IsEmpty() )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	if ( !tStmt.m_sStringParam.IsEmpty() )
	{
		tOut.Error ( "table '%s': shard writes do not support token_filter_options", tStmt.m_sIndex.cstr() );
		return false;
	}

	const CSphSchema & tSchema = tShard.m_tSchema;
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.GetFieldsCount();
	if ( tShard.m_tSettings.m_bIndexFieldLens )
		iSchemaSz -= tSchema.GetFieldsCount();

	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
	{
		auto & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType == SPH_ATTR_STRING && tSchema.GetField ( tAttr.m_sName.cstr() ) )
			iSchemaSz--;
	}

	if ( tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		iSchemaSz--;

	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength() && iSchemaSz != tStmt.m_iSchemaSz )
	{
		tOut.Error ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		return false;
	}

	if ( ( iGot % iExp ) != 0 )
	{
		tOut.Error ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		return false;
	}

	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.GetFieldsCount() );
	CSphVector<bool> dFieldAttrs ( tSchema.GetFieldsCount() );
	if ( !CreateAttrMaps ( dAttrSchema, dFieldSchema, dFieldAttrs, tSchema, tStmt.m_dInsertSchema, tOut ) )
		return false;

	if ( !EnsureShardTxnScope ( *pSession, tStmt.m_sIndex, &tShard, tStmt.m_eStmt, sError ) )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	auto & tShardTxn = pSession->m_tShardTxn;
	if ( tShardTxn.m_eWriteStmt==STMT_DUMMY )
		tShardTxn.m_eWriteStmt = tStmt.m_eStmt;

	CSphVector<int64_t> dStmtDocIDs;
	dStmtDocIDs.Reserve ( tStmt.m_iRowsAffected );

	AttributeConverter_c tConverter ( tSchema, dFieldAttrs, sError, sWarning );

	for ( int iRow = 0; iRow < tStmt.m_iRowsAffected; ++iRow )
	{
		assert ( sError.IsEmpty() );
		tConverter.NewRow();

		int iSchemaAttrCount = tSchema.GetAttrsCount();
		int iFieldLenAttr = tSchema.GetAttrId_FirstFieldLen();
		if ( iFieldLenAttr >= 0 )
			iSchemaAttrCount = iFieldLenAttr;

		bool bOk = true;
		for ( int i = 0; i < iSchemaAttrCount && bOk; ++i )
		{
			int iQuerySchemaIdx = dAttrSchema[i];
			if ( iQuerySchemaIdx < 0 )
				tConverter.SetDefaultAttrValue ( i );
			else
				bOk = tConverter.SetAttrValue ( i, tStmt.m_dInsertValues[iQuerySchemaIdx + iRow * iExp], iRow, iQuerySchemaIdx, sError );
		}

		if ( !bOk )
			break;

		for ( int i = 0; i < tSchema.GetFieldsCount() && bOk; ++i )
		{
			int iQuerySchemaIdx = dFieldSchema[i];
			if ( iQuerySchemaIdx < 0 )
				tConverter.SetDefaultFieldValue ( i );
			else
				bOk = tConverter.SetFieldValue ( i, tStmt.m_dInsertValues[iQuerySchemaIdx + iRow * iExp], iRow, iQuerySchemaIdx );
		}

		if ( !bOk )
			break;

		tConverter.Finalize();

		bool bReplace = ( bReplaceStmt && tConverter.GetID() != 0 );
		if ( !tConverter.GetID() )
			tConverter.SetID ( UidShort() );

		int iTargetOrdinal = GetShardTargetOrdinal ( tShard, tConverter.GetID() );
		const auto * pRouteTarget = GetShardRouteTarget ( tShard, iTargetOrdinal );
		if ( !pRouteTarget )
		{
			sError.SetSprintf ( "table '%s': invalid shard routing target", tStmt.m_sIndex.cstr() );
			break;
		}

		int iTarget = FindShardTxnTarget ( tShardTxn, pRouteTarget->m_iRouteId );
		if ( iTarget < 0 )
		{
			if ( !tShardTxn.HasPendingData() )
			{
				tShardTxn.m_sShard = tStmt.m_sIndex;
				tShardTxn.m_pShardInstance = &tShard;
			}

			auto & tNewTarget = AddShardTxnTarget ( tShardTxn );
			tNewTarget.m_sIndex = pRouteTarget->m_sName;
			tNewTarget.m_iRouteId = pRouteTarget->m_iRouteId;
			tNewTarget.m_bLocal = pRouteTarget->IsLocal();
			if ( !pRouteTarget->IsLocal() )
				tNewTarget.m_pAgent = pRouteTarget->m_pAgent;
			iTarget = tShardTxn.m_dTargets.GetLength() - 1;
		}

		auto & tTarget = tShardTxn.m_dTargets[iTarget];
		int iStoredRow = StoreRow ( tConverter, tSchema, tShardTxn.m_tStorage, bReplace, tShardTxn.m_dOrder.GetLength() );
		tTarget.m_dRowIds.Add ( iStoredRow );
		auto & tOp = tTarget.m_dOps.Add();
		tOp.m_eOp = ShardTxnOp_e::ROW_WRITE;
		tOp.m_iData = iStoredRow;
		tOp.m_iCount = 1;

		auto & tRowRef = tShardTxn.m_dOrder.Add();
		tRowRef.m_iTarget = iTarget;
		tRowRef.m_eOp = ShardTxnOp_e::ROW_WRITE;
		tRowRef.m_iData = iStoredRow;
		tRowRef.m_iCount = 1;

		dStmtDocIDs.Add ( tConverter.GetID() );
		++iAffectedRows;
	}

	if ( !sError.IsEmpty() )
	{
		RollbackShardTxn ( tShardTxn );
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	if ( pSession->m_bAutoCommit && !pSession->m_bInTransaction )
	{
		CSphVector<int64_t> dCommittedDocIDs;
		if ( !CommitShardTxn ( *pSession, sError, &dCommittedDocIDs, nullptr ) )
		{
			dLastIds.SwapData ( dCommittedDocIDs );
			tOut.Error ( "%s", sError.cstr() );
			return false;
		}

		dLastIds.SwapData ( dCommittedDocIDs );
	} else
		dLastIds.SwapData ( dStmtDocIDs );

	int64_t iLastInsertID = dLastIds.GetLength() ? dLastIds.Last() : 0;
	tOut.Ok ( iAffectedRows, sWarning, iLastInsertID );
	return true;
}

bool DeleteDocumentShard ( const SqlStmt_t & tStmt, const ShardIndex_c & tShard, StmtErrorReporter_i & tOut, int & iAffectedRows )
{
	auto * pSession = session::GetClientSession();
	iAffectedRows = 0;

	CSphString sError;
	if ( !ValidateClusterStatement ( tStmt.m_sIndex, tShard.m_sCluster, tStmt.m_sCluster, IsHttpStmt ( tStmt ) ) )
		TlsMsg::MoveError ( sError );

	if ( !sError.IsEmpty() )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	CSphVector<int64_t> dDocids;
	if ( !ValidateShardDeleteStatement ( tStmt, sError ) )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	const bool bExactDocids = TryExtractShardDeleteDocids ( tStmt, dDocids );
	ShardDeletePredicate_t tPredicate;
	if ( !bExactDocids )
		ExtractShardDeletePredicate ( tStmt, tPredicate );

	if ( !EnsureShardTxnScope ( *pSession, tStmt.m_sIndex, &tShard, STMT_DELETE, sError ) )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	auto & tShardTxn = pSession->m_tShardTxn;

	if ( bExactDocids )
	{
		struct ShardStmtDeleteDoc_t
		{
			int		m_iTarget = -1;
			int		m_iStmtOrder = -1;
			int64_t	m_iDocid = 0;
		};

		struct ShardStmtDeleteDocSorter_t
		{
			static bool IsLess ( const ShardStmtDeleteDoc_t & a, const ShardStmtDeleteDoc_t & b )
			{
				return a.m_iTarget==b.m_iTarget ? a.m_iStmtOrder < b.m_iStmtOrder : a.m_iTarget < b.m_iTarget;
			}
		};

		CSphVector<ShardStmtDeleteDoc_t> dStmtDeleteDocids;
		for ( int64_t iDocid : dDocids )
		{
			int iTargetOrdinal = GetShardTargetOrdinal ( tShard, iDocid );
			const auto * pRouteTarget = GetShardRouteTarget ( tShard, iTargetOrdinal );
			if ( !pRouteTarget )
			{
				sError.SetSprintf ( "table '%s': invalid shard routing target", tStmt.m_sIndex.cstr() );
				break;
			}

			int iTarget = FindOrAddShardTxnTarget ( tShardTxn, tStmt.m_sIndex, tShard, *pRouteTarget );
			auto & tStmtDelete = dStmtDeleteDocids.Add();
			tStmtDelete.m_iTarget = iTarget;
			tStmtDelete.m_iStmtOrder = dStmtDeleteDocids.GetLength() - 1;
			tStmtDelete.m_iDocid = iDocid;
			++iAffectedRows;
		}

		if ( sError.IsEmpty() && !dStmtDeleteDocids.IsEmpty() )
		{
			dStmtDeleteDocids.Sort ( ShardStmtDeleteDocSorter_t() );

			for ( int i = 0; i < dStmtDeleteDocids.GetLength(); )
			{
				int iTarget = dStmtDeleteDocids[i].m_iTarget;
				auto & tTarget = tShardTxn.m_dTargets[iTarget];
				int iDeleteOff = tTarget.m_dDeleteDocids.GetLength();
				int iBegin = i;
				while ( i < dStmtDeleteDocids.GetLength() && dStmtDeleteDocids[i].m_iTarget == iTarget )
				{
					tTarget.m_dDeleteDocids.Add ( dStmtDeleteDocids[i].m_iDocid );
					++i;
				}

				int iDeleteCount = i - iBegin;

				auto & tOp = tTarget.m_dOps.Add();
				tOp.m_eOp = ShardTxnOp_e::DOC_DELETE;
				tOp.m_iData = iDeleteOff;
				tOp.m_iCount = iDeleteCount;

				auto & tOrder = tShardTxn.m_dOrder.Add();
				tOrder.m_iTarget = iTarget;
				tOrder.m_eOp = ShardTxnOp_e::DOC_DELETE;
				tOrder.m_iData = iDeleteOff;
				tOrder.m_iCount = iDeleteCount;
			}
		}
	} else
	{
		if ( tShard.m_dRouteTargets.IsEmpty() )
			sError.SetSprintf ( "table '%s': invalid shard routing target", tStmt.m_sIndex.cstr() );
		else
		{
			int iPredicateIdx = tShardTxn.m_dDeletePredicates.GetLength();
			tShardTxn.m_dDeletePredicates.Add ( tPredicate );
			for ( const auto & tRouteTarget : tShard.m_dRouteTargets )
			{
				int iTarget = FindOrAddShardTxnTarget ( tShardTxn, tStmt.m_sIndex, tShard, tRouteTarget );

				auto & tTarget = tShardTxn.m_dTargets[iTarget];
				auto & tOp = tTarget.m_dOps.Add();
				tOp.m_eOp = ShardTxnOp_e::QUERY_DELETE;
				tOp.m_iData = iPredicateIdx;
				tOp.m_iCount = 1;

				auto & tOrder = tShardTxn.m_dOrder.Add();
				tOrder.m_iTarget = iTarget;
				tOrder.m_eOp = ShardTxnOp_e::QUERY_DELETE;
				tOrder.m_iData = iPredicateIdx;
				tOrder.m_iCount = 1;
			}
		}
	}

	if ( !sError.IsEmpty() )
	{
		RollbackShardTxn ( tShardTxn );
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	if ( pSession->m_bAutoCommit && !pSession->m_bInTransaction )
	{
		int iDeleted = 0;
		if ( !CommitShardTxn ( *pSession, sError, nullptr, &iDeleted ) )
		{
			tOut.Error ( "%s", sError.cstr() );
			return false;
		}

		tOut.Ok ( iDeleted );
	} else
		tOut.Ok ( 0 );

	return true;
}

static void AddWarning ( StrVec_t * pWarnings, const char * szIndexName, CSphString & sWarning )
{
	if ( sWarning.IsEmpty() )
		return;

	if ( pWarnings )
		pWarnings->Add ( sWarning );
	else
		sphWarning ( "table '%s': %s", szIndexName, sWarning.cstr() );

	sWarning = "";
}

static bool ConfigureShardMetadataFromConfig ( ShardIndex_c & tShard, const char * szIndexName, const CSphConfigSection & hIndex, StrVec_t * pWarnings, CSphString & sError )
{
	bool bWordDict = true;
	auto sDictType = hIndex.GetStr ( "dict", "keywords" );
	if ( sDictType=="crc" )
		bWordDict = false;
	else if ( sDictType!="keywords" )
	{
		sError.SetSprintf ( "table '%s': unknown dict=%s; only 'keywords' or 'crc' values allowed", szIndexName, sDictType.cstr() );
		return false;
	}

	if ( !ConfigureRTPercolate ( tShard.m_tSchema, tShard.m_tSettings, szIndexName, hIndex, bWordDict, false, pWarnings, sError ) )
		return false;

	tShard.m_sIndexPath = hIndex["path"].strval();

	CSphString sWarning;
	tShard.m_tTokenizerSettings.Setup ( hIndex, sWarning );
	AddWarning ( pWarnings, szIndexName, sWarning );

	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( szIndexName );
	tShard.m_tDictSettings.Setup ( hIndex, pFilenameBuilder.get(), sWarning );
	AddWarning ( pWarnings, szIndexName, sWarning );

	tShard.m_tFieldFilterSettings.Setup ( hIndex, sWarning );
	AddWarning ( pWarnings, szIndexName, sWarning );
	return true;
}

ResultAndIndex_t AddShardIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bLoadMeta, CSphString & sError, StrVec_t * pWarnings )
{
	if ( !IsConfigless() )
	{
		sError.SetSprintf ( "table '%s': type='shard' requires data_dir to be set in the config file", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	DistributedIndexRefPtr_t pIdx ( new ShardIndex_c );
	auto * pShard = AsShard ( pIdx.Ptr() );
	assert ( pShard );
	pShard->m_sIndexPath = hIndex.GetStr ( "path" );

	if ( bLoadMeta )
	{
		StrVec_t dWarnings;
		if ( !LoadShardMeta ( szIndexName, pShard->m_sIndexPath, *pShard, dWarnings, sError ) )
			return { ADD_ERROR, nullptr };

		for ( auto & sWarning : dWarnings )
			AddWarning ( pWarnings, szIndexName, sWarning );
	} else
	{
		if ( !ConfigureShardMetadataFromConfig ( *pShard, szIndexName, hIndex, pWarnings, sError ) )
			return { ADD_ERROR, nullptr };
	}

	bool bOk = ConfigureDistributedIndex ( [&sError] ( const auto& sIdx ) {
		auto pLocal = GetServed ( sIdx );
		if ( !pLocal )
		{
			sError.SetSprintf ( "no such local table '%s'", sIdx.cstr() );
			return false;
		}

		if ( pLocal->m_eType!=IndexType_e::RT )
		{
			sError.SetSprintf ( "local table '%s' must be real-time", sIdx.cstr() );
			return false;
		}

		return true;
	}, *pShard, szIndexName, hIndex, sError, pWarnings );

	if ( !bOk || pIdx->IsEmpty() )
	{
		if ( !bOk )
			sError.SetSprintf ( "table '%s': %s", szIndexName, sError.cstr() );
		else
			sError.SetSprintf ( "table '%s': no valid local/remote tables in shard table", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	if ( !BuildShardRouteTargets ( szIndexName, *pShard, sError ) )
		return { ADD_ERROR, nullptr };

	if ( !CaptureLocalShardTargetsGuards ( *pShard, szIndexName, sError ) )
		return { ADD_ERROR, nullptr };

	if ( !g_pDistIndexes->Add ( pIdx, szIndexName ) )
	{
		sError.SetSprintf ( "table '%s': unable to add name (duplicate?)", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	return ResultAndIndex_t { ADD_DISTR, nullptr };
}
