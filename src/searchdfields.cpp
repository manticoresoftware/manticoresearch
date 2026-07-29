//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxstd.h"
#include "searchdha.h"
#include "auth/auth.h"

struct FieldRequest_t
{
	CSphString m_sIndexes;
	CSphFixedVector<CSphString> m_dFieldNames { 0 };
	CSphVector<DocID_t> m_dDocs { 0 };
};

struct FieldLoc_t
{
	int	m_iOff;
	int	m_iSize;
};

struct FieldBlob_t
{
	CSphString				m_sError;
	CSphVector<BYTE>		m_dBlob;
	CSphVector<FieldLoc_t>	m_dLocs;
};

struct DocHash_t : private OpenHashTable_T<DocID_t, int>
{
	explicit DocHash_t ( int iElems ) : OpenHashTable_T<DocID_t,int> { iElems } {}
	int Count () const { return (int)GetLength(); }
	bool Exists ( DocID_t tId ) const { return ( Find ( tId )!=nullptr ); }
	void Set ( DocID_t tId, int iOff ) { Acquire ( tId ) = iOff;}
	using OpenHashTable_T<DocID_t,int>::Acquire;
	using OpenHashTable_T<DocID_t,int>::Find;
};

struct ResLoc_t
{
	DocID_t m_iDocid;
	int m_iIndex;
};

struct RemoteFieldsAnswer_t : public iQueryResult, public FieldBlob_t
{
	void Reset() final {}
	bool HasWarnings() const final { return false; }

	// request data, not owned
	VecTraits_T<ResLoc_t> m_dResDocs;
	int					  m_iTag=0;

	// reply data
	CSphVector<DocID_t> m_dDocs;
	const BYTE * m_pFieldsRaw = nullptr; // not owned
	int m_iFieldsLen = 0;
	int m_iExpectedFields = -1;
	int m_iRequestedDocs = -1;
};

static bool FailReply ( AgentConn_t & tAgent, const char * sFmt, ... )
{
	CSphString sMessage;
	sMessage.SetSprintf ( "invalid GETFIELD reply: %s", sFmt );

	va_list ap;
	va_start ( ap, sFmt );
	tAgent.m_sFailure.SetSprintfVa ( sMessage.cstr(), ap );
	va_end ( ap );

	return false;
}


static bool ValidateFieldLocs ( const VecTraits_T<FieldLoc_t> & dLocs, int iFieldsLen, CSphString & sError )
{
	ARRAY_CONSTFOREACH ( i, dLocs )
	{
		const auto & tLoc = dLocs[i];
		if ( tLoc.m_iOff<0 || tLoc.m_iSize<0 )
		{
			sError.SetSprintf ( "locator %d has negative offset or size (off=%d, size=%d)",
					i, tLoc.m_iOff, tLoc.m_iSize );
			return false;
		}

		if ( int64_t ( tLoc.m_iOff ) + tLoc.m_iSize>iFieldsLen )
		{
			sError.SetSprintf ( "locator %d out of fields blob range (off=%d, size=%d, fields_len=%d)",
					i, tLoc.m_iOff, tLoc.m_iSize, iFieldsLen );
			return false;
		}
	}

	return true;
}


static bool ValidateFields ( const RemoteFieldsAnswer_t & tReply, CSphString & sError )
{
	if ( tReply.m_iExpectedFields<0 )
	{
		sError = "invalid expected field count";
		return false;
	}

	if ( tReply.m_iRequestedDocs<0 )
	{
		sError = "invalid requested doc count";
		return false;
	}

	if ( tReply.m_dDocs.GetLength()>tReply.m_iRequestedDocs )
	{
		sError.SetSprintf ( "doc count %d exceeds requested doc count %d",
				tReply.m_dDocs.GetLength(), tReply.m_iRequestedDocs );
		return false;
	}

	int64_t iExpectedLocs = int64_t ( tReply.m_dDocs.GetLength() ) * tReply.m_iExpectedFields;
	if ( iExpectedLocs!=tReply.m_dLocs.GetLength() )
	{
		sError.SetSprintf ( "invalid locator count (docs=%d, fields=%d, locators=%d)",
				tReply.m_dDocs.GetLength(), tReply.m_iExpectedFields, tReply.m_dLocs.GetLength() );
		return false;
	}

	if ( tReply.m_iFieldsLen<0 || ( tReply.m_iFieldsLen && !tReply.m_pFieldsRaw ) )
	{
		sError = "invalid fields blob";
		return false;
	}

	if ( !ValidateFieldLocs ( tReply.m_dLocs, tReply.m_iFieldsLen, sError ) )
		return false;

	return true;
}


static void SetFieldReplyContract ( RemoteFieldsAnswer_t & tReply, int iExpectedFields, int iRequestedDocs )
{
	tReply.m_iExpectedFields = iExpectedFields;
	tReply.m_iRequestedDocs = iRequestedDocs;
}


static void SetFieldReplyContract ( VecRefPtrsAgentConn_t & dRemotes, int iExpectedFields, int iRequestedDocs )
{
	for ( AgentConn_t * pAgent : dRemotes )
	{
		auto * pReply = (RemoteFieldsAnswer_t *) pAgent->m_pResult.get();
		assert ( pReply );
		SetFieldReplyContract ( *pReply, iExpectedFields, iRequestedDocs );
	}
}


static bool ValidateReturnedDocs ( const RemoteFieldsAnswer_t & tReply, const DocHash_t & hRequestedDocs, CSphString & sError )
{
	for ( DocID_t tDocid : tReply.m_dDocs )
		if ( !hRequestedDocs.Exists ( tDocid ) )
		{
			sError.SetSprintf ( "returned doc " UINT64_FMT " was not requested", uint64_t ( tDocid ) );
			return false;
		}

	return true;
}


struct GetFieldRequestBuilder_t : public RequestBuilder_i
{
	const VecTraits_T<const CSphColumnInfo *>& m_dFieldCols;

	explicit GetFieldRequestBuilder_t ( const VecTraits_T<const CSphColumnInfo *>& dFieldCols )
		: m_dFieldCols ( dFieldCols )
	{}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		auto * pRes = (RemoteFieldsAnswer_t *)tAgent.m_pResult.get();
		assert ( pRes );

		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_GETFIELD, VER_COMMAND_GETFIELD );

		tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr() );
		tOut.SendDword ( m_dFieldCols.GetLength() );
		for ( auto* pFieldCol : m_dFieldCols )
			tOut.SendString ( pFieldCol->m_sName.cstr() );
		tOut.SendDword ( pRes->m_dResDocs.GetLength() );
		for ( auto& iDoc : pRes->m_dResDocs )
			tOut.SendUint64 ( iDoc.m_iDocid );
	}
};

struct GetFieldReplyParser_t : public ReplyParser_i
{
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		auto * pReply = (RemoteFieldsAnswer_t *)tAgent.m_pResult.get();
		assert ( pReply );

		DWORD uDocs = tReq.GetDword();
		if ( uDocs>INT_MAX )
			return FailReply ( tAgent, "doc count %u exceeds INT_MAX", uDocs );

		if ( int64_t ( uDocs ) * sizeof ( DocID_t ) + 2*sizeof ( DWORD )>tReq.HasBytes() )
			return FailReply ( tAgent, "doc count %u exceeds remaining packet", uDocs );

		CSphVector<DocID_t> dDocs;
		dDocs.Resize ( (int)uDocs );
		for ( auto & tDoc : dDocs )
			tDoc = tReq.GetUint64();

		DWORD uLocs = tReq.GetDword();
		if ( uLocs>INT_MAX )
			return FailReply ( tAgent, "locator count %u exceeds INT_MAX", uLocs );

		if ( int64_t ( uLocs ) * 2*sizeof ( DWORD ) + sizeof ( DWORD )>tReq.HasBytes() )
			return FailReply ( tAgent, "locator count %u exceeds remaining packet", uLocs );

		CSphVector<FieldLoc_t> dLocs;
		dLocs.Resize ( (int)uLocs );
		ARRAY_FOREACH ( i, dLocs )
		{
			DWORD uOff = tReq.GetDword();
			DWORD uSize = tReq.GetDword();

			if ( uOff>INT_MAX )
				return FailReply ( tAgent, "locator %d offset %u exceeds INT_MAX", i, uOff );

			if ( uSize>INT_MAX )
				return FailReply ( tAgent, "locator %d size %u exceeds INT_MAX", i, uSize );

			dLocs[i] = { (int)uOff, (int)uSize };
		}

		DWORD uFieldsLen = tReq.GetDword();
		if ( uFieldsLen>INT_MAX )
			return FailReply ( tAgent, "fields blob length %u exceeds INT_MAX", uFieldsLen );

		int iFieldsLen = (int)uFieldsLen;
		if ( iFieldsLen>tReq.HasBytes() )
			return FailReply ( tAgent, "fields blob length %d exceeds remaining packet", iFieldsLen );

		const BYTE * pFieldsRaw = nullptr;
		if ( iFieldsLen )
			tReq.GetBytesZerocopy ( &pFieldsRaw, iFieldsLen );

		if ( tReq.GetError() )
			return FailReply ( tAgent, "failed to read reply: %s", tReq.GetErrorMessage().cstr() );

		pReply->m_dDocs.SwapData ( dDocs );
		pReply->m_dLocs.SwapData ( dLocs );
		pReply->m_iFieldsLen = iFieldsLen;
		pReply->m_pFieldsRaw = pFieldsRaw;
		return true;
	}
};

struct ProxyFieldRequestBuilder_t : public RequestBuilder_i
{
	const FieldRequest_t & m_tArgs;

	explicit ProxyFieldRequestBuilder_t ( const FieldRequest_t & tArgs )
		: m_tArgs ( tArgs )
	{}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_GETFIELD, VER_COMMAND_GETFIELD );
		tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr() );
		tOut.SendDword ( m_tArgs.m_dFieldNames.GetLength() );
		ARRAY_FOREACH ( i, m_tArgs.m_dFieldNames )
			tOut.SendString ( m_tArgs.m_dFieldNames[i].cstr() );
		tOut.SendDword ( m_tArgs.m_dDocs.GetLength() );
		for ( int i = 0, iDocs = m_tArgs.m_dDocs.GetLength (); i<iDocs; ++i )
			tOut.SendUint64 ( m_tArgs.m_dDocs[i] );
	}
};

namespace { // static

bool GetIndexes ( const CSphString & sIndexes, CSphString & sError, StrVec_t & dLocal, VecRefPtrsAgentConn_t & dRemotes )
{
	StrVec_t dNames;
	ParseIndexList ( sIndexes, dNames );

	sph::StringSet hDups;

	for ( const CSphString & sIndex : dNames )
	{
		auto pLocal = GetServed ( sIndex );
		auto pDist = GetDistr ( sIndex );

		if ( !pLocal && !pDist )
		{
			sError.SetSprintf ( "no such table %s", sIndex.cstr() );
			return false;
		}

		if ( pLocal )
		{
			if ( hDups.Add ( sIndex ) )
				dLocal.Add ( sIndex );

		} else
		{
			for ( const auto& pAgent : pDist->m_dAgents )
			{
				auto * pConn = new AgentConn_t;
				pConn->SetMultiAgent ( pAgent );
				pConn->m_iMyConnectTimeoutMs = pDist->GetAgentConnectTimeoutMs();
				pConn->m_iMyQueryTimeoutMs = pDist->GetAgentQueryTimeoutMs();
				pConn->m_pResult = std::make_unique<RemoteFieldsAnswer_t>();
				dRemotes.Add ( pConn );
			}
			for ( const auto & sDistLocal : pDist->m_dLocal )
			{
				if ( hDups.Add ( sDistLocal ) )
					dLocal.Add ( sDistLocal );
			}
		}
	}

	return true;
}

static bool GetFieldFromLocal ( const CSphString & sIndexName, const FieldRequest_t & tArgs, int64_t iSessionID, DocHash_t & hFetchedDocs, FieldBlob_t & tRes )
{
	auto pServed = GetServed ( sIndexName );
	if ( !pServed )
	{
		tRes.m_sError.SetSprintf ( "no such table %s", sIndexName.cstr() );
		return false;
	}

	pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_GETFIELD );

	auto& tRefCrashQuery = GlobalCrashQueryGetRef();
	tRefCrashQuery.m_dIndex = { sIndexName.cstr(), sIndexName.Length() };

	RIdx_c pIndex { pServed };
	pIndex->CreateReader ( iSessionID );

	// collect fieldids and remap as:
	// for fieldnames: [fake four third five]
	// fieldids is: [3,4,5]
	// remap is: [-1,1,0,2] -> as field[-1] absent, field[1] is four, field[0] is third, field[2] is five
	CSphFixedVector<int> dFieldRemap ( tArgs.m_dFieldNames.GetLength() );
	CSphVector<int> dFieldIds;
	{
		using fld_t = std::pair<int,int>;
		CSphVector<fld_t> dFields;
		ARRAY_CONSTFOREACH ( i, tArgs.m_dFieldNames )
		{
			int iField = pIndex->GetFieldId ( tArgs.m_dFieldNames[i], DOCSTORE_TEXT );
			if ( iField==-1 )
				dFieldRemap[i] = -1;
			else
				dFields.Add ( { iField, i } );
		}
		dFields.Sort ( Lesser ( [] ( fld_t & l, fld_t & r ) { return l.first<r.first; } ) );
		dFieldIds.Resize ( dFields.GetLength () );
		ARRAY_CONSTFOREACH ( i, dFields )
		{
			dFieldIds[i] = dFields[i].first;
			dFieldRemap[dFields[i].second] = i;
		}
	}

	DocstoreDoc_t tDoc;
	for ( DocID_t tDocid : tArgs.m_dDocs )
	{
		if ( hFetchedDocs.Exists ( tDocid ) )
			continue;

		if ( !pIndex->GetDoc ( tDoc, tDocid, &dFieldIds, iSessionID, false ) )
			continue;

		assert ( tDoc.m_dFields.GetLength()==dFieldIds.GetLength() );
		hFetchedDocs.Set ( tDocid, tRes.m_dLocs.GetLength() );
		for ( int iField : dFieldRemap )
		{
			if ( iField==-1 )
			{
				tRes.m_dLocs.Add ( { tRes.m_dBlob.GetLength (), 0 } );
				continue;
			}

			assert ( iField<tDoc.m_dFields.GetLength() );
			const CSphVector<BYTE> & tFieldData = tDoc.m_dFields[iField];

			tRes.m_dLocs.Add ( { tRes.m_dBlob.GetLength (), (int) tFieldData.GetLengthBytes () } );
			tRes.m_dBlob.Append ( tFieldData );
		}
	}

	return true;
}

bool GetFieldFromDist ( VecRefPtrsAgentConn_t & dRemotes, const FieldRequest_t & tArgs, DocHash_t & hFetchedDocs, FieldBlob_t & tRes )
{
	const int iFieldsCount = tArgs.m_dFieldNames.GetLength();
	DocHash_t hRequestedDocs ( tArgs.m_dDocs.GetLength() );
	for ( DocID_t tDocid : tArgs.m_dDocs )
		hRequestedDocs.Set ( tDocid, 0 );

	bool bOk = true;
	for ( AgentConn_t * pAgent : dRemotes )
	{
		auto * pReply = (RemoteFieldsAnswer_t *) pAgent->m_pResult.get ();
		if ( !pAgent->m_bSuccess )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
				tRes.m_sError.SetSprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
			else
				tRes.m_sError.SetSprintf ( "agent %s: failed to fetch stored fields", pAgent->m_tDesc.GetMyUrl().cstr() );
			bOk = false;
			continue;
		}

		if ( !ValidateFields ( *pReply, tRes.m_sError ) )
		{
			tRes.m_sError.SetSprintf ( "agent %s: invalid GETFIELD reply: %s", pAgent->m_tDesc.GetMyUrl().cstr(), tRes.m_sError.cstr() );
			bOk = false;
			continue;
		}

		if ( !ValidateReturnedDocs ( *pReply, hRequestedDocs, tRes.m_sError ) )
		{
			tRes.m_sError.SetSprintf ( "agent %s: invalid GETFIELD reply: %s", pAgent->m_tDesc.GetMyUrl().cstr(), tRes.m_sError.cstr() );
			bOk = false;
			continue;
		}

		ARRAY_FOREACH ( iDoc, pReply->m_dDocs )
		{
			DocID_t tDocid = pReply->m_dDocs[iDoc];
			// already got fields for the document
			if ( hFetchedDocs.Exists(tDocid) )
				continue;

			hFetchedDocs.Set ( tDocid, tRes.m_dLocs.GetLength () );
			auto dLocs = pReply->m_dLocs.Slice ( iDoc * iFieldsCount, iFieldsCount );
			for ( const auto& tSrcField : dLocs  )
			{
				tRes.m_dLocs.Add ( { tRes.m_dBlob.GetLength (), tSrcField.m_iSize } );
				const BYTE * pField = tSrcField.m_iSize ? pReply->m_pFieldsRaw+tSrcField.m_iOff : nullptr;
				tRes.m_dBlob.Append ( pField, tSrcField.m_iSize );
			}
		}
	}

	return bOk;
}

static bool GetFields ( const FieldRequest_t & tReq, FieldBlob_t & tRes, DocHash_t & hFetchedDocs )
{
	if ( tReq.m_dDocs.IsEmpty() )
		return true;

	StrVec_t dLocals;
	VecRefPtrsAgentConn_t dRemotes;
	if ( !GetIndexes ( tReq.m_sIndexes, tRes.m_sError, dLocals, dRemotes ) )
		return false;

	if ( dLocals.IsEmpty() && dRemotes.IsEmpty() )
		return true;

	bool bOkLocal = true;
	bool bOkRemote = true;
	CSphRefcountedPtr<RemoteAgentsObserver_i> pDistReporter { nullptr };
	std::unique_ptr<RequestBuilder_i> pDistReq;
	std::unique_ptr<ReplyParser_i> pDistReply;

	if ( !dRemotes.IsEmpty () )
	{
		SetFieldReplyContract ( dRemotes, tReq.m_dFieldNames.GetLength(), tReq.m_dDocs.GetLength() );

		pDistReq = std::make_unique<ProxyFieldRequestBuilder_t> ( tReq );
		pDistReply = std::make_unique<GetFieldReplyParser_t>();

		pDistReporter = GetObserver();
		SetSessionAuth ( dRemotes );
		ScheduleDistrJobs ( dRemotes, pDistReq.get(), pDistReply.get(), pDistReporter.Ptr() );
	}

	{
		DocstoreSession_c tSession;
		for ( const auto & sLocal : dLocals )
		{
			if ( !GetFieldFromLocal ( sLocal, tReq, tSession.GetUID(), hFetchedDocs, tRes ) )
			{
				bOkLocal = false;
				break;
			}

			// early out on fields fetched for all docs
			if ( hFetchedDocs.Count ()==tReq.m_dDocs.GetLength() )
				break;
		}
	}

	if ( !dRemotes.IsEmpty() )
	{
		pDistReporter->Finish();
		bOkRemote = GetFieldFromDist ( dRemotes, tReq, hFetchedDocs, tRes );
	}

	return ( bOkLocal && bOkRemote );
}


FieldRequest_t ParseAPICommandGetfield ( InputBuffer_c & tReq )
{
	FieldRequest_t tArgs;
	// parse remote request
	tArgs.m_sIndexes = tReq.GetString ();

	tArgs.m_dFieldNames.Reset ( tReq.GetDword () );
	for ( auto & sName : tArgs.m_dFieldNames )
		sName = tReq.GetString ();

	tArgs.m_dDocs.Resize ( tReq.GetDword () );
	for ( auto & tDocID : tArgs.m_dDocs )
		tDocID = tReq.GetUint64 ();
	return tArgs;
}

void SendAPICommandGetfieldAnswer ( ISphOutputBuffer & tOut, FieldRequest_t& tRequest,
		const FieldBlob_t& tRes, const DocHash_t& tFetched )
{
	auto tReply = APIAnswer ( tOut, VER_COMMAND_GETFIELD );
	auto iDocsCount = tFetched.Count();
	if ( !iDocsCount )
	{
		tOut.SendDword ( 0 ); // docs array
		tOut.SendDword ( 0 ); // locators array
		tOut.SendDword ( 0 ); // fields blob array
		return;
	}

	auto& dDocs = tRequest.m_dDocs;

	// wipe out absent docs
	ARRAY_FOREACH ( i, dDocs )
		if ( !tFetched.Exists ( dDocs[i] ) )
			dDocs.RemoveFast ( i-- );

	if ( iDocsCount != dDocs.GetLength() )
		dDocs.Uniq();

	assert ( iDocsCount==dDocs.GetLength () );
	tOut.SendDword ( iDocsCount );
	ARRAY_FOREACH ( i, dDocs )
	{
		assert ( tFetched.Exists ( dDocs[i] ) );
		tOut.SendUint64 ( dDocs[i] );
	}

	auto iFields = tRequest.m_dFieldNames.GetLength ();
	tOut.SendDword ( iDocsCount * iFields );
	for ( DocID_t tDoc : dDocs )
	{
		auto pDoc = tFetched.Find ( tDoc );
		if (!pDoc)
			continue;
		const int iOff = *pDoc;
		for ( int i=0; i<iFields; ++i )
		{
			tOut.SendDword ( tRes.m_dLocs[iOff+i].m_iOff );
			tOut.SendDword ( tRes.m_dLocs[iOff+i].m_iSize );
		}
	}

	tOut.SendArray ( tRes.m_dBlob );
}

int GetDocIDOffset ( const AggrResult_t& tRes )
{
	const CSphColumnInfo* pId = tRes.m_tSchema.GetAttr ( sphGetDocidName() );
	if ( pId )
		return pId->m_tLocator.m_iBitOffset >> ROWITEM_SHIFT;
	return 0;
}

// fill vec of ResLoc_t with remote matches from tRes, sorted by DocID
int CollectUntaggedDocs ( CSphVector<ResLoc_t>& dIds, const AggrResult_t & tRes, const VecTraits_T<CSphMatch> & dMatches )
{
	assert ( !tRes.m_bTagsAssigned );
	assert ( tRes.m_bSingle );
	if ( !tRes.m_dResults.First ().m_bTag ) // process only remote resultsets
		return -1;

	DocID_t iLastDocID = DOCID_MIN;
	bool bNeedSort = false;
	auto iIdOffset = GetDocIDOffset ( tRes );
	ARRAY_CONSTFOREACH( i, dMatches )
	{
		const CSphMatch & tMatch = dMatches[i];
		ResLoc_t & tDoc = dIds.Add ();
		tDoc.m_iDocid = sphGetDocID ( tMatch.m_pDynamic + iIdOffset );
		tDoc.m_iIndex = i;
		if ( bNeedSort )
			continue;
		bNeedSort = tDoc.m_iDocid<iLastDocID;
		iLastDocID = tDoc.m_iDocid;
	}
	if ( bNeedSort )
		dIds.Sort ( Lesser ( [] ( const ResLoc_t & a, const ResLoc_t & b ) { return a.m_iDocid<b.m_iDocid; } ) );
	return tRes.m_dResults.First ().m_iTag;
}

int TagOf ( const ResLoc_t& tLoc, const VecTraits_T<CSphMatch> & dMatches )
{
	assert ( tLoc.m_iIndex>=0 && tLoc.m_iIndex<dMatches.GetLength () );
	return dMatches[tLoc.m_iIndex].m_iTag;
}

static bool ValidateSortedReturnedDocs ( const VecTraits_T<ResLoc_t> & dResDocs, const VecTraits_T<DocID_t> & dDocs, const VecTraits_T<int> & dOrd, CSphString & sError )
{
	int iDocs = dDocs.GetLength();
	if ( !iDocs )
		return true;

	int iDoc = 0;
	int iRecvDoc = dOrd[iDoc];
	for ( const auto & dResDoc : dResDocs )
	{
		if ( dResDoc.m_iDocid<dDocs[iRecvDoc] )
			continue;

		if ( dResDoc.m_iDocid>dDocs[iRecvDoc] )
		{
			sError.SetSprintf ( "returned doc " UINT64_FMT " was not requested", uint64_t ( dDocs[iRecvDoc] ) );
			return false;
		}

		++iDoc;
		if ( iDoc>=iDocs )
			break;

		iRecvDoc = dOrd[iDoc];
	}

	if ( iDoc<iDocs )
	{
		sError.SetSprintf ( "returned doc " UINT64_FMT " was not requested", uint64_t ( dDocs[dOrd[iDoc]] ) );
		return false;
	}

	return true;
}

// fill vec of ResLoc_t with remote matches from tRes, sorted by tags, inside groups sorted by DocID
void CollectTaggedDocs ( CSphVector<ResLoc_t> & dIds, const AggrResult_t & tRes, const VecTraits_T<CSphMatch> & dMatches )
{
	assert ( tRes.m_bTagsAssigned );
	assert ( tRes.m_bSingle );
	auto iIdOffset = GetDocIDOffset ( tRes );
	ARRAY_CONSTFOREACH( i, dMatches )
	{
		const CSphMatch & tMatch = dMatches[i];
		if ( !tRes.m_dResults[tMatch.m_iTag].m_bTag ) // process only matches came from remotes
			continue;
		ResLoc_t & tDoc = dIds.Add ();
		tDoc.m_iDocid = sphGetDocID ( tMatch.m_pDynamic + iIdOffset );
		tDoc.m_iIndex = i;
	}
	dIds.Sort ( Lesser ( [&dMatches] ( const ResLoc_t & a, const ResLoc_t & b )
	{
		auto iTagA = TagOf ( a, dMatches );
		auto iTagB = TagOf ( b, dMatches );
		if ( iTagA==iTagB )
			return a.m_iDocid<b.m_iDocid;
		return iTagA<iTagB;
	}));
}

void AddRange ( CSphVector<RemoteFieldsAnswer_t> & dRanges, const VecTraits_T<ResLoc_t> & dIds, int iTag )
{
	if ( dIds.IsEmpty() )
		return;

	auto & dRange = dRanges.Add();
	dRange.m_dResDocs = dIds;
	dRange.m_iTag = iTag;
}

// fill dRanges with chunks of dIds, grouped by same tag
void CollectTaggedRanges ( CSphVector<RemoteFieldsAnswer_t> & dRanges, const VecTraits_T<ResLoc_t> & dIds, const VecTraits_T<CSphMatch> & dMatches )
{
	int iStart = 0;
	if ( dIds.IsEmpty () )
		return;

	auto iStartTag = TagOf ( dIds[iStart], dMatches );
	ARRAY_CONSTFOREACH ( i, dIds )
	{
		if ( TagOf ( dIds[i], dMatches )!=iStartTag )
		{
			AddRange ( dRanges, dIds.Slice ( iStart, i-iStart ), iStartTag );
			iStart = i;
			iStartTag = TagOf ( dIds[iStart], dMatches );
		}
	}
	AddRange ( dRanges, dIds.Slice ( iStart, dIds.GetLength()-iStart ), iStartTag );
}

// fill dIds with remote matches and return ranges of them, grouped by tag. First match of every chunk has valid tag
CSphVector<RemoteFieldsAnswer_t> ExtractRanges ( CSphVector<ResLoc_t>& dIds, const AggrResult_t & tRes, const VecTraits_T<CSphMatch>& dMatches )
{
	CSphVector<RemoteFieldsAnswer_t> dRanges;

	if ( tRes.m_bTagsAssigned )
	{
		CollectTaggedDocs ( dIds, tRes, dMatches );
		CollectTaggedRanges ( dRanges, dIds, dMatches );
	}
	else
	{
		int iOnlyTag = CollectUntaggedDocs( dIds, tRes, dMatches );
		AddRange ( dRanges, dIds, iOnlyTag ); // only one range
	}
	return dRanges;
}

// create agents
VecRefPtrsAgentConn_t GetAgents( const VecTraits_T<RemoteFieldsAnswer_t>& dRangesIds, AggrResult_t & tRes )
{
	assert ( tRes.m_bIdxByTag );
	VecRefPtrsAgentConn_t dAgents;
	dAgents.Reserve ( dRangesIds.GetLength () );
	for ( auto & dRange : dRangesIds )
	{
		const AgentConn_t * pDesc = tRes.m_dResults[dRange.m_iTag].Agent ();
		assert ( pDesc );
		auto * pAgent = new AgentConn_t;
		pAgent->m_tDesc.CloneFrom ( pDesc->m_tDesc );
		pAgent->m_iMyConnectTimeoutMs = pDesc->m_iMyConnectTimeoutMs;
		pAgent->m_iMyQueryTimeoutMs = pDesc->m_iMyQueryTimeoutMs;
		pAgent->m_pResult.reset ( &dRange ); // fixme! that is hack with reset/release, not very good fit to unique_ptr
		dAgents.Add ( pAgent );
	}
	return dAgents;
}

bool FillDocs ( VecTraits_T<CSphMatch> & dMatches, RemoteFieldsAnswer_t& dReply,
		const VecTraits_T<const CSphColumnInfo *>& dFieldCols, CSphString & sError )
{
	auto & dResDocs = dReply.m_dResDocs; // directly passed from source, sorted by DocID
	auto & dDocs = dReply.m_dDocs; // received, m.b. different (shrinked and unordered)
	int iStride = dFieldCols.GetLength ();

	if ( !ValidateFields ( dReply, sError ) )
		return false;

	if ( dDocs.IsEmpty () )
		return true;

	// docs and locators placed in unknown order, but for merging we need them ordered.
	CSphFixedVector<int> dOrd { dDocs.GetLength () };
	ARRAY_CONSTFOREACH( i, dOrd )
		dOrd[i] = i;
	dOrd.Sort ( Lesser ( [&dDocs] ( int l, int r ) { return dDocs[l]<dDocs[r]; } ) );

	if ( !ValidateSortedReturnedDocs ( dResDocs, dDocs, dOrd, sError ) )
		return false;

	int iDocs = dDocs.GetLength();
	int iDoc = 0;
	int iRecvDoc = dOrd[iDoc];
	for ( auto & dResDoc : dResDocs )
	{
		if ( dResDoc.m_iDocid<dDocs[iRecvDoc] )
			continue;

		assert ( dResDoc.m_iDocid==dDocs[iRecvDoc] );

		// found matched docs
		CSphMatch & tMatch = dMatches[dResDoc.m_iIndex];
		auto dLocators = dReply.m_dLocs.Slice ( iStride * iDoc, iStride );
		ARRAY_CONSTFOREACH( i, dFieldCols )
		{
			const BYTE * pField = dLocators[i].m_iSize ? dReply.m_pFieldsRaw+dLocators[i].m_iOff : nullptr;
			BYTE * pPacked = sphPackPtrAttr ( { pField, dLocators[i].m_iSize } );
			pPacked = (BYTE*)ExchangeAttr ( tMatch, dFieldCols[i]->m_tLocator, (SphAttr_t)pPacked );
			sphDeallocatePacked ( pPacked );
		}

		++iDoc;
		if ( iDoc>=iDocs )
			break;

		iRecvDoc = dOrd[iDoc];
	}

	return true;
}

static bool FillRemoteDocs ( AgentConn_t * pAgent, VecTraits_T<CSphMatch> & dMatches,
		const VecTraits_T<const CSphColumnInfo *>& dFieldCols, StringBuilder_c & sError )
{
	auto & dReply = *(RemoteFieldsAnswer_t *) pAgent->m_pResult.release ();
	if ( pAgent->m_bSuccess )
	{
		CSphString sFailure;
		if ( FillDocs ( dMatches, dReply, dFieldCols, sFailure ) )
			return true;

		sError.Sprintf ( "agent %s: invalid GETFIELD reply: %s", pAgent->m_tDesc.GetMyUrl().cstr(), sFailure.cstr() );
		return false;
	}

	if ( !pAgent->m_sFailure.IsEmpty() )
		sError.Sprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
	else
		sError.Sprintf ( "agent %s: failed to fetch stored fields", pAgent->m_tDesc.GetMyUrl().cstr() );

	return false;
}

CSphVector<const CSphColumnInfo *> GetStoredColumnList ( const CSphSchema & tSchema )
{
	CSphVector<const CSphColumnInfo *> dFieldCols;
	for ( int i = 0, iAttrsCount = tSchema.GetAttrsCount (); i<iAttrsCount; ++i )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
		if ( IsNotRealAttribute ( tCol ) )
			dFieldCols.Add ( &tCol );
	}
	return dFieldCols;
}

bool Env_ManticoreStoredTimeout()
{
	static auto iTimeout = env_long ("MANTICORE_STORED_TIMEOUT");
	return iTimeout? true : false;
}

bool Env_ManticoreStoredTimeoutAgent()
{
	static auto iTimeout = env_long ("MANTICORE_STORED_TIMEOUT_AGENT");
	return iTimeout? true : false;
}

} // static namespace

void HandleCommandGetField ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_GETFIELD, tOut ) )
		return;

	// parse request
	auto tRequest = ParseAPICommandGetfield ( tReq );

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}
	if ( Env_ManticoreStoredTimeoutAgent() )
		return;

	if ( !ApiCheckPerms ( session::GetUser(), AuthAction_e::READ, tRequest.m_sIndexes, tOut ) )
		return;

	// fetch stored fields
	DocHash_t tFetched ( tRequest.m_dDocs.GetLength() );
	FieldBlob_t tRes;
	if ( !GetFields ( tRequest, tRes, tFetched ) )
	{
		SendErrorReply ( tOut, "%s", tRes.m_sError.cstr() );
		return;
	}
	if ( Env_ManticoreStoredTimeout() )
		return;

	SendAPICommandGetfieldAnswer ( tOut, tRequest, tRes, tFetched );
}

bool RemotesGetField ( AggrResult_t & tRes, const CSphQuery & tQuery )
{
	assert ( tRes.m_bSingle );
	assert ( tRes.m_bOneSchema );

	// TODO!!! try to start fetch prior to sorting

	assert ( tRes.m_bOneSchema );
	assert ( tRes.m_bTagsCompacted );
	assert ( tRes.m_bIdxByTag );
	assert ( tRes.m_bSingle );

	auto dFieldCols = GetStoredColumnList ( tRes.m_tSchema );

	// early reject in case no stored fields found
	if ( dFieldCols.IsEmpty() )
		return true;

	int iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	int iCount = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOffset, iCount );

	// extract ranges and early return in case no remote matches
	CSphVector<ResLoc_t> dIds;
	auto dRangesIds = ExtractRanges ( dIds, tRes, dMatches );
	if ( dRangesIds.IsEmpty() )
		return true;

	auto dAgents = GetAgents ( dRangesIds, tRes );
	assert ( dAgents.GetLength ()==dRangesIds.GetLength () );
	for ( auto & dRange : dRangesIds )
		SetFieldReplyContract ( dRange, dFieldCols.GetLength(), dRange.m_dResDocs.GetLength() );

	// connect to remote agents and query them
	GetFieldRequestBuilder_t tBuilder ( dFieldCols );
	GetFieldReplyParser_t tParser;
	SetSessionAuth ( dAgents );
	PerformRemoteTasks ( dAgents, &tBuilder, &tParser );

	bool bOk = true;
	StringBuilder_c sError { "," };
	if ( !tRes.m_sWarning.IsEmpty () )
		sError << tRes.m_sWarning;
	for ( auto* pAgent : dAgents )
		if ( !FillRemoteDocs ( pAgent, dMatches, dFieldCols, sError ) )
			bOk = false;

	if ( bOk )
		sError.MoveTo ( tRes.m_sWarning );
	else
		sError.MoveTo ( tRes.m_sError );

	return bOk;
}
