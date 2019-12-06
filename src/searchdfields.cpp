//
// $Id$
//

//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxstd.h"
#include "searchdha.h"

struct GetFieldArgs_t
{
	CSphString m_sIndexes;
	CSphFixedVector<const char *> m_dFieldNames { 0 };
	CSphFixedVector<DocID_t> m_dDocs { 0 };
};

struct FieldTrait_t
{
	int	m_iOff;
	int	m_iSize;
};

struct GetFieldRes_t
{
	CSphString m_sError;

	CSphVector<DocID_t> m_dDocs;
	CSphVector<FieldTrait_t> m_dOff;
	CSphVector<BYTE> m_dFields;
};

struct GotDocs_t
{
	OpenHash_T<int, DocID_t> m_hDocs { 0 };
	GotDocs_t ( const VecTraits_T<DocID_t> & dIds ) { Init ( dIds ); }
	void Init ( const VecTraits_T<DocID_t> & dIds );
	int Count (  const VecTraits_T<DocID_t> & dIds ) const;
	bool Exists ( DocID_t tId ) const;
	void Set ( DocID_t tId, int iOff );
};

struct RemoteAgentDoc_t
{
	DocID_t m_iDocid;
	int m_iAgent;
	int m_iIndex;
};

struct AgentFieldPaiload_t : public iQueryResult, public GetFieldRes_t
{
	virtual void Reset() override {}
	virtual bool HasWarnings() const override { return false; }

	// reply data
	const BYTE * m_pFieldsRaw = nullptr;

	// request data
	int m_iDocOff = 0;
	int m_iDocCount = 0;
};

struct GetFieldRequestBuilder_t : public RequestBuilder_i
{
	VecTraits_T<const CSphColumnInfo *> m_dFieldCols;
	VecTraits_T<RemoteAgentDoc_t> m_dDocs;

	GetFieldRequestBuilder_t() {};
	void BuildRequest ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		AgentFieldPaiload_t * pRes = (AgentFieldPaiload_t *)tAgent.m_pResult.Ptr();
		assert ( pRes );

		APICommand_t tCmd ( tOut, SEARCHD_COMMAND_GETFIELD, VER_COMMAND_GETFIELD );

		tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr() );
		tOut.SendDword ( m_dFieldCols.GetLength() );
		ARRAY_FOREACH ( i, m_dFieldCols )
			tOut.SendString ( m_dFieldCols[i]->m_sName.cstr() );
		tOut.SendDword ( pRes->m_iDocCount );
		for ( int i=0; i<pRes->m_iDocCount; i++ )
			tOut.SendUint64 ( m_dDocs[pRes->m_iDocOff+i].m_iDocid );
	}
};

struct GetFieldReplyParser_t : public ReplyParser_i
{
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		AgentFieldPaiload_t * pReply = (AgentFieldPaiload_t *)tAgent.m_pResult.Ptr();
		assert ( pReply );
		pReply->m_dDocs.Resize ( tReq.GetDword() );
		ARRAY_FOREACH ( i, pReply->m_dDocs )
			pReply->m_dDocs[i] = tReq.GetUint64();
		pReply->m_dOff.Resize ( tReq.GetDword() );
		for ( FieldTrait_t & tField : pReply->m_dOff )
		{
			tField.m_iOff = tReq.GetDword();
			tField.m_iSize = tReq.GetDword();
		}
		int iFieldsLen = tReq.GetDword();
		if ( iFieldsLen )
			tReq.GetBytesZerocopy( &pReply->m_pFieldsRaw, iFieldsLen );

		return !tReq.GetError();
	};
};

struct ProxyFieldRequestBuilder_t : public RequestBuilder_i
{
	const GetFieldArgs_t & m_tArgs;

	ProxyFieldRequestBuilder_t ( const GetFieldArgs_t & tArgs )
		: m_tArgs ( tArgs )
	{};
	void BuildRequest ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		APICommand_t tCmd ( tOut, SEARCHD_COMMAND_GETFIELD, VER_COMMAND_GETFIELD );

		tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr() );
		tOut.SendDword ( m_tArgs.m_dFieldNames.GetLength() );
		ARRAY_FOREACH ( i, m_tArgs.m_dFieldNames )
			tOut.SendString ( m_tArgs.m_dFieldNames[i] );
		tOut.SendDword ( m_tArgs.m_dDocs.GetLength() );
		for ( int i=0; i<m_tArgs.m_dDocs.GetLength(); i++ )
			tOut.SendUint64 ( m_tArgs.m_dDocs[i] );
	}
};

void GotDocs_t::Init ( const VecTraits_T<DocID_t> & dIds )
{
	m_hDocs.Reset ( dIds.GetLength() );
	for ( DocID_t tDoc : dIds )
		m_hDocs.Acquire ( tDoc ) = -1;
}

int GotDocs_t::Count ( const VecTraits_T<DocID_t> & dIds ) const
{
	int iCount = 0;
	for ( DocID_t tDoc : dIds )
	{
		if ( *m_hDocs.Find ( tDoc )!=-1 )
			iCount++;
	}

	return iCount;
}

bool GotDocs_t::Exists ( DocID_t tId ) const
{
	return ( *m_hDocs.Find ( tId )!=-1 );
}

void GotDocs_t::Set ( DocID_t tId, int iOff )
{
	m_hDocs.Acquire ( tId ) = iOff;
}

static bool GetIndexes ( const CSphString & sIndexes, CSphString & sError, StrVec_t & dLocal, VecRefPtrsAgentConn_t & dRemotes )
{
	StrVec_t dNames;
	ParseIndexList ( sIndexes, dNames );

	for ( const CSphString & sIndex : dNames )
	{
		auto pLocal = GetServed ( sIndex );
		auto pDist = GetDistr ( sIndex );

		if ( !pLocal && !pDist )
		{
			sError.SetSprintf ( "no such index %s", sIndex.cstr() );
			return false;
		}

		if ( pLocal )
		{
			dLocal.Add ( sIndex );
		} else
		{
			for ( auto * pAgent : pDist->m_dAgents )
			{
				auto * pConn = new AgentConn_t;
				pConn->SetMultiAgent ( sIndex, pAgent );
				pConn->m_iMyConnectTimeout = pDist->m_iAgentConnectTimeout;
				pConn->m_iMyQueryTimeout = pDist->m_iAgentQueryTimeout;
				pConn->m_pResult = new AgentFieldPaiload_t();
				dRemotes.Add ( pConn );
			}

			dLocal.Append ( pDist->m_dLocal );
		}
	}

	// eliminate local dupes that come from distributed indexes
	if ( !dRemotes.IsEmpty () && !dLocal.IsEmpty() )
		dLocal.Uniq();

	return true;
}

static bool GetFieldFromLocal ( const CSphString & sIndexName, const GetFieldArgs_t & tArgs, GotDocs_t & tGotDocs, GetFieldRes_t & tRes )
{
	auto pServed = GetServed ( sIndexName );
	if ( !pServed )
	{
		tRes.m_sError.SetSprintf ( "no such index %s", sIndexName.cstr() );
		return false;
	}

	ServedDescRPtr_c pDesc ( pServed );
	if ( !pDesc->m_pIndex )
	{
		tRes.m_sError.SetSprintf ( "missed index '%s'", sIndexName.cstr() );
		return false;
	}

	const CSphIndex * pIndex = pDesc->m_pIndex;

	DocstoreSession_c tSession;
	pIndex->CreateReader ( tSession.GetUID() );
	
	CSphVector<int> dFieldIds;
	CSphFixedVector<int> dFieldRemap ( tArgs.m_dFieldNames.GetLength() );
	ARRAY_FOREACH ( i, tArgs.m_dFieldNames )
	{
		int iField = pIndex->GetFieldId ( tArgs.m_dFieldNames[i], DOCSTORE_TEXT );
		if ( iField==-1 )
		{
			dFieldRemap[i] = -1;
			continue;
		}

		dFieldRemap[i] = dFieldIds.GetLength();
		dFieldIds.Add ( iField );
	}
	dFieldIds.Sort();

	DocstoreDoc_t tDoc;
	ARRAY_FOREACH ( i, tArgs.m_dDocs )
	{
		DocID_t tDocid = tArgs.m_dDocs[i];
		if ( tGotDocs.Exists ( tDocid ) )
			continue;

		if ( !pIndex->GetDoc ( tDoc, tDocid, &dFieldIds, tSession.GetUID(), false ) )
			continue;

		assert ( tDoc.m_dFields.GetLength()==dFieldIds.GetLength() );
		tGotDocs.Set ( tDocid, tRes.m_dOff.GetLength() );
		for ( int iField : dFieldRemap )
		{
			if ( iField==-1 )
			{
				FieldTrait_t & tFieldDesc = tRes.m_dOff.Add();
				tFieldDesc.m_iOff = tRes.m_dFields.GetLength();
				tFieldDesc.m_iSize = 0;
				continue;
			}

			assert ( iField<tDoc.m_dFields.GetLength() );
			const CSphVector<BYTE> & tFieldData = tDoc.m_dFields[iField];
			
			FieldTrait_t & tFieldDesc = tRes.m_dOff.Add();
			tFieldDesc.m_iOff = tRes.m_dFields.GetLength();
			tFieldDesc.m_iSize = tFieldData.GetLengthBytes();

			BYTE * pField = tRes.m_dFields.AddN ( tFieldDesc.m_iSize );
			memcpy ( pField, tFieldData.Begin(), tFieldDesc.m_iSize );
		}
	}

	return true;
}

static bool GetFieldFromDist ( const VecRefPtrsAgentConn_t & dRemotes, const GetFieldArgs_t & tArgs, GotDocs_t & tGotDocs, GetFieldRes_t & tRes )
{
	const int iFieldsCount = tArgs.m_dFieldNames.GetLength();
	for ( const AgentConn_t * pAgent : dRemotes )
	{
		if ( !pAgent->m_bSuccess )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
				tRes.m_sError.SetSprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
			continue;
		}


		const AgentFieldPaiload_t * pReply = (AgentFieldPaiload_t *)pAgent->m_pResult.Ptr();

		ARRAY_FOREACH ( iDoc, pReply->m_dDocs )
		{
			DocID_t tDocid = pReply->m_dDocs[iDoc];
			int & iFieldOff = tGotDocs.m_hDocs.Acquire ( tDocid );
			// already got fields for the document
			if ( iFieldOff!=-1 )
				continue;

			iFieldOff = tRes.m_dOff.GetLength();
			for ( int iField=0; iField<iFieldsCount; iField++ )
			{
				const FieldTrait_t & tSrcField = pReply->m_dOff[iDoc * iFieldsCount + iField];
					
				FieldTrait_t & tDstField = tRes.m_dOff.Add();
				tDstField.m_iOff = tRes.m_dFields.GetLength();
				tDstField.m_iSize = tSrcField.m_iSize;

				BYTE * pDst = tRes.m_dFields.AddN ( tSrcField.m_iSize );
				memcpy ( pDst, pReply->m_pFieldsRaw + tSrcField.m_iOff, tSrcField.m_iSize );
			}

		}
	}

	return true;
}

static bool GetField ( const GetFieldArgs_t & tArgs, GetFieldRes_t & tRes, GotDocs_t & tGotDocs )
{
	if ( !tArgs.m_dDocs.GetLength() )
		return true;

	StrVec_t dLocals;
	VecRefPtrsAgentConn_t dRemotes;
	if ( !GetIndexes ( tArgs.m_sIndexes, tRes.m_sError, dLocals, dRemotes ) )
		return false;

	if ( !dLocals.GetLength() && !dRemotes.GetLength() )
		return true;

	bool bOkLocal = true;
	bool bOkRemote = true;
	CSphRefcountedPtr<RemoteAgentsObserver_i> pDistReporter { nullptr };
	CSphScopedPtr<RequestBuilder_i> pDistReq { nullptr };
	CSphScopedPtr<ReplyParser_i> pDistReply { nullptr };

	if ( dRemotes.GetLength() )
	{
		pDistReq = new ProxyFieldRequestBuilder_t ( tArgs );
		pDistReply = new GetFieldReplyParser_t();

		pDistReporter = GetObserver();
		ScheduleDistrJobs ( dRemotes, pDistReq.Ptr(), pDistReply.Ptr(), pDistReporter.Ptr() );
	}

	ARRAY_FOREACH ( i, dLocals )
	{
		const CSphString & sLocal = dLocals[i];
		if ( !GetFieldFromLocal ( sLocal, tArgs, tGotDocs, tRes ) )
		{
			bOkLocal = false;
			break;
		}

		// early out on fields fetched for all docs
		if ( i!=dLocals.GetLength() && tGotDocs.Count ( tArgs.m_dDocs )==tArgs.m_dDocs.GetLength() )
			break;
	}

	if ( dRemotes.GetLength() )
	{
		pDistReporter->Finish();
		bOkRemote = GetFieldFromDist ( dRemotes, tArgs, tGotDocs, tRes );
	}

	return ( bOkLocal && bOkRemote );
}

void HandleCommandGetField ( CachedOutputBuffer_c & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_GETFIELD, tOut ) )
		return;

	GetFieldArgs_t tArgs;
	// parse remote request
	tArgs.m_sIndexes = tReq.GetString();

	tArgs.m_dFieldNames.Reset ( tReq.GetDword() );
	ARRAY_FOREACH ( i, tArgs.m_dFieldNames )
	{
		int iLen = tReq.GetDword();
		const BYTE * pStr = nullptr;
		tReq.GetBytesZerocopy ( &pStr, iLen );
		tArgs.m_dFieldNames[i] = (const char *)pStr;
	}
	tArgs.m_dDocs.Reset ( tReq.GetDword() );
	ARRAY_FOREACH ( i, tArgs.m_dDocs )
		tArgs.m_dDocs[i] = tReq.GetUint64();

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}

	// fetch stored fields
	GotDocs_t tGotDocs ( tArgs.m_dDocs );
	GetFieldRes_t tRes;
	if ( !GetField ( tArgs, tRes, tGotDocs ) )
	{
		SendErrorReply ( tOut, "%s", tRes.m_sError.cstr() );
		return;
	}

	const int iDocsCount = tGotDocs.Count ( tArgs.m_dDocs );

	// send reply
	APICommand_t dOk ( tOut, SEARCHD_OK, VER_COMMAND_GETFIELD );
	tOut.SendDword ( iDocsCount );
	if ( iDocsCount )
	{
		for ( DocID_t tDoc : tArgs.m_dDocs )
		{
			if ( tGotDocs.Exists ( tDoc ) )
				tOut.SendUint64 ( tDoc );
		}
	}
	tOut.SendDword ( iDocsCount * tArgs.m_dFieldNames.GetLength() );
	if ( iDocsCount )
	{
		for ( DocID_t tDoc : tArgs.m_dDocs )
		{
			int iOff = tGotDocs.m_hDocs.Acquire ( tDoc );
			if ( iOff==-1 )
				continue;

			ARRAY_FOREACH ( iField, tArgs.m_dFieldNames )
			{
				tOut.SendDword ( tRes.m_dOff[iOff+iField].m_iOff );
				tOut.SendDword ( tRes.m_dOff[iOff+iField].m_iSize );
			}
		}
	}
	tOut.SendArray ( tRes.m_dFields );
}

struct DistFieldRes_t
{
	StringBuilder_c m_sError { "," };

	CSphVector<const CSphColumnInfo *> m_dFieldCols;
	CSphVector<RemoteAgentDoc_t> m_dIds;

	AggrResult_t * m_pRes = nullptr;
	const CSphQuery * m_pQuery = nullptr;
	const VecRefPtrsAgentConn_t * m_pDesc = nullptr;
	VecRefPtrsAgentConn_t m_dAgents;

	CSphRefcountedPtr<RemoteAgentsObserver_i> m_pReporter { nullptr };
	CSphScopedPtr<RequestBuilder_i> m_pReq { nullptr };
	CSphScopedPtr<ReplyParser_i> m_pReply { nullptr };
};

struct RemoteAgentDoc_tSort_fn
{
	bool IsLess ( const RemoteAgentDoc_t & a, const RemoteAgentDoc_t & b ) const
	{
		if ( a.m_iAgent==b.m_iAgent )
			return a.m_iDocid<b.m_iDocid;
		else
			return a.m_iAgent<b.m_iAgent;
	}
};

static void DistGetFieldStart ( DistFieldRes_t & tRes )
{
	assert ( tRes.m_pDesc && tRes.m_pRes && tRes.m_pQuery );
	// remote agents requests send off thread
	if ( tRes.m_pDesc->IsEmpty() )
		return;

	const AggrResult_t & tAggrRes = *tRes.m_pRes; 
	for ( int i=0; i<tAggrRes.m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tAggrRes.m_tSchema.GetAttr ( i );
		if ( tCol.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
			tRes.m_dFieldCols.Add ( &tCol );
	}
	// early reject in case no stored fields found
	if ( tRes.m_dFieldCols.IsEmpty() )
		return;

	int iTo = tAggrRes.m_dMatches.GetLength();
	int iOff = Max ( tRes.m_pQuery->m_iOffset, tRes.m_pQuery->m_iOuterOffset );
	int iCount = ( tRes.m_pQuery->m_iOuterLimit ? tRes.m_pQuery->m_iOuterLimit : tRes.m_pQuery->m_iLimit );
	iTo = Max ( Min ( iOff + iCount, iTo ), 0 );
	int iFrom = Min ( iOff, iTo );

	for ( int i=iFrom; i<iTo; i++ )
	{
		const CSphMatch & tMatch = tAggrRes.m_dMatches[i];
		// remote match (tag highest bit 1)
		if ( ( tMatch.m_iTag & 0x80000000 )==0 )
			continue;

		RemoteAgentDoc_t & tDoc = tRes.m_dIds.Add();
		tDoc.m_iDocid = sphGetDocID ( tMatch.m_pDynamic );
		tDoc.m_iAgent = tMatch.m_iTag & 0x7FFFFFFF;
		tDoc.m_iIndex = i;
	}
	// early reject in case no documents from agents found
	if ( tRes.m_dIds.IsEmpty() )
		return;

	tRes.m_dIds.Sort ( RemoteAgentDoc_tSort_fn() );

	// create agents
	tRes.m_dAgents.Reserve ( tRes.m_pDesc->GetLength() );
	int iAgentMatch = -1;
	AgentFieldPaiload_t * pAgentData = nullptr;
	ARRAY_FOREACH ( i, tRes.m_dIds )
	{
		if ( iAgentMatch==tRes.m_dIds[i].m_iAgent )
			continue;

		iAgentMatch = tRes.m_dIds[i].m_iAgent;
		const AgentConn_t * pDesc = (*tRes.m_pDesc)[iAgentMatch];
		assert ( pDesc );

		AgentConn_t * pAgent = new AgentConn_t;
		pAgent->m_tDesc.CloneFrom ( pDesc->m_tDesc );
		HostDesc_t tHost;
		pAgent->m_tDesc.m_pDash = new HostDashboard_t ( tHost );
		pAgent->m_iMyConnectTimeout = pDesc->m_iMyConnectTimeout;
		pAgent->m_iMyQueryTimeout = pDesc->m_iMyQueryTimeout;

		// update doc count for last agent
		if ( pAgentData )
			pAgentData->m_iDocCount = i - pAgentData->m_iDocOff;

		pAgentData = new AgentFieldPaiload_t();
		pAgentData->m_iDocOff = i;
		pAgent->m_pResult = pAgentData;

		tRes.m_dAgents.Add ( pAgent );
	}
	assert ( pAgentData );
	pAgentData->m_iDocCount = tRes.m_dIds.GetLength() - pAgentData->m_iDocOff;

	// connect to remote agents and query them
	GetFieldRequestBuilder_t * pReq = new GetFieldRequestBuilder_t();
	pReq->m_dFieldCols = tRes.m_dFieldCols;
	pReq->m_dDocs = tRes.m_dIds;
	tRes.m_pReq = pReq;
	tRes.m_pReply = new GetFieldReplyParser_t();

	tRes.m_pReporter = GetObserver();
	ScheduleDistrJobs ( tRes.m_dAgents, tRes.m_pReq.Ptr(), tRes.m_pReply.Ptr(), tRes.m_pReporter.Ptr() );
}

static void DistGetFieldFinish ( DistFieldRes_t & tRes )
{
	if ( !tRes.m_pReporter.Ptr() )
		return;

	tRes.m_pReporter->Finish ();

	for ( const AgentConn_t * pAgent : tRes.m_dAgents )
	{
		if ( !pAgent->m_bSuccess )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
				tRes.m_sError.Appendf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );

			continue;
		}

		const AgentFieldPaiload_t * pReply = (AgentFieldPaiload_t *)pAgent->m_pResult.Ptr();
		int iDocFieldsOff = 0;

		const RemoteAgentDoc_t * pSrcDocs = tRes.m_dIds.Begin() + pReply->m_iDocOff;
		const RemoteAgentDoc_t * pSrcDocsEnd = pSrcDocs + pReply->m_iDocCount;
		const DocID_t * pRecvDocs = pReply->m_dDocs.Begin();
		const DocID_t * pRecvDocsEnd = pRecvDocs + pReply->m_dDocs.GetLength();

		while ( pSrcDocs<pSrcDocsEnd && pRecvDocs<pRecvDocsEnd )
		{
			while ( pSrcDocs->m_iDocid<*pRecvDocs && pSrcDocs<pSrcDocsEnd )
				pSrcDocs++;

			if ( pSrcDocs==pSrcDocsEnd )
				break;

			while ( *pRecvDocs<pSrcDocs->m_iDocid && pRecvDocs<pRecvDocsEnd )
			{
				pRecvDocs++;
				iDocFieldsOff += tRes.m_dFieldCols.GetLength();
			}
			if ( pRecvDocs==pRecvDocsEnd )
				break;

			if ( *pRecvDocs!=pSrcDocs->m_iDocid )
				continue;

			CSphMatch & tMatch = tRes.m_pRes->m_dMatches[pSrcDocs->m_iIndex];

			// found matched docs
			assert ( iDocFieldsOff<pReply->m_dOff.GetLength() );
			FieldTrait_t * pFieldDesc = pReply->m_dOff.Begin() + iDocFieldsOff;
			for ( const auto& dCol : tRes.m_dFieldCols )
			{
				BYTE * pPacked = sphPackPtrAttr ( {pReply->m_pFieldsRaw+pFieldDesc->m_iOff, pFieldDesc->m_iSize} );
				tMatch.SetAttr ( dCol->m_tLocator, (SphAttr_t) pPacked );
				pFieldDesc++;
			}

			pSrcDocs++;
			pRecvDocs++;
			iDocFieldsOff += tRes.m_dFieldCols.GetLength();
		}
	}
}

void RemotesGetField ( const VecRefPtrsAgentConn_t & dRemotes,	const CSphQuery & tQuery, AggrResult_t & tRes )
{
	DistFieldRes_t tFieldRes;
	tFieldRes.m_pDesc = &dRemotes;
	tFieldRes.m_pRes = &tRes;
	tFieldRes.m_pQuery = &tQuery;

	// TODO!!! try to start fetch prior to sorting
	DistGetFieldStart ( tFieldRes );
	DistGetFieldFinish ( tFieldRes );
}