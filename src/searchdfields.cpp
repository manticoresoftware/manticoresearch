//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxstd.h"
#include "searchdha.h"

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

struct DocHash_t : private OpenHash_T<int, DocID_t>
{
	explicit DocHash_t ( int iElems ) : OpenHash_T<int, DocID_t> { iElems } {}
	int Count () const { return (int)GetLength(); }
	bool Exists ( DocID_t tId ) const { return ( Find ( tId )!=nullptr ); }
	void Set ( DocID_t tId, int iOff ) { Acquire ( tId ) = iOff;}
	using OpenHash_T<int, DocID_t>::Acquire;
	using OpenHash_T<int, DocID_t>::Find;
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
};

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
		pReply->m_dDocs.Resize ( tReq.GetDword() );
		for ( auto& tDoc : pReply->m_dDocs )
			tDoc = tReq.GetUint64();
		pReply->m_dLocs.Resize ( tReq.GetDword() );
		for ( FieldLoc_t & tField : pReply->m_dLocs )
		{
			tField.m_iOff = tReq.GetDword();
			tField.m_iSize = tReq.GetDword();
		}
		int iFieldsLen = tReq.GetDword();
		if ( iFieldsLen )
			tReq.GetBytesZerocopy( &pReply->m_pFieldsRaw, iFieldsLen );

		return !tReq.GetError();
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
			dLocal.Add ( sIndex );
		} else
		{
			for ( const auto& pAgent : pDist->m_dAgents )
			{
				auto * pConn = new AgentConn_t;
				pConn->SetMultiAgent ( pAgent );
				pConn->m_iMyConnectTimeoutMs = pDist->m_iAgentConnectTimeoutMs;
				pConn->m_iMyQueryTimeoutMs = pDist->m_iAgentQueryTimeoutMs;
				pConn->m_pResult = std::make_unique<RemoteFieldsAnswer_t>();
				dRemotes.Add ( pConn );
			}
			dLocal.Append ( pDist->m_dLocal );
		}
	}

	dLocal.Uniq();
	return true;
}

bool GetFieldFromLocal ( const CSphString & sIndexName, const FieldRequest_t & tArgs, int64_t iSessionID,
		DocHash_t & hFetchedDocs, FieldBlob_t & tRes )
{
	auto pServed = GetServed ( sIndexName );
	if ( !pServed )
	{
		tRes.m_sError.SetSprintf ( "no such table %s", sIndexName.cstr() );
		return false;
	}

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
	for ( AgentConn_t * pAgent : dRemotes )
	{
		auto * pReply = (RemoteFieldsAnswer_t *) pAgent->m_pResult.get ();
		if ( !pAgent->m_bSuccess )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
				tRes.m_sError.SetSprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
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
				tRes.m_dBlob.Append ( pReply->m_pFieldsRaw+tSrcField.m_iOff, tSrcField.m_iSize );
			}
		}
	}

	return true;
}

bool GetFields ( const FieldRequest_t & tReq, FieldBlob_t & tRes, DocHash_t & hFetchedDocs )
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
		pDistReq = std::make_unique<ProxyFieldRequestBuilder_t> ( tReq );
		pDistReply = std::make_unique<GetFieldReplyParser_t>();

		pDistReporter = GetObserver();
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
		const FieldBlob_t& tRes, const DocHash_t& _tFetched )
{
	auto tReply = APIAnswer ( tOut, VER_COMMAND_GETFIELD );
	auto & tFetched = const_cast<DocHash_t &>(_tFetched); // non-const need for Acquire()
	auto iDocsCount = tFetched.Count();
	if ( !iDocsCount )
	{
		tOut.SendDword ( 0 ); // docs array
		tOut.SendDword ( 0 ); // locators array
		tOut.SendDword ( 0 ); // fields blob array
		return;
	}

	auto& dDocs = tRequest.m_dDocs;

	// send doclist and simultaneously wipe out absend docs
	// note that because of wiping doc's order will be broken!
	tOut.SendDword ( iDocsCount );
	ARRAY_FOREACH ( i, dDocs )
	{
		if ( tFetched.Exists ( dDocs[i] ) )
			tOut.SendUint64 ( dDocs[i] );
		else
			dDocs.RemoveFast(i--);
	}

	assert ( iDocsCount==dDocs.GetLength () );

	auto iFields = tRequest.m_dFieldNames.GetLength ();
	tOut.SendDword ( iDocsCount * iFields );
	for ( DocID_t tDoc : dDocs )
	{
		int iOff = tFetched.Acquire ( tDoc );
		for ( int i=0; i<iFields; ++i )
		{
			tOut.SendDword ( tRes.m_dLocs[iOff+i].m_iOff );
			tOut.SendDword ( tRes.m_dLocs[iOff+i].m_iSize );
		}
	}

	tOut.SendArray ( tRes.m_dBlob );
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
	ARRAY_CONSTFOREACH( i, dMatches )
	{
		const CSphMatch & tMatch = dMatches[i];
		ResLoc_t & tDoc = dIds.Add ();
		tDoc.m_iDocid = sphGetDocID ( tMatch.m_pDynamic );
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

// fill vec of ResLoc_t with remote matches from tRes, sorted by tags, inside groups sorted by DocID
void CollectTaggedDocs ( CSphVector<ResLoc_t> & dIds, const AggrResult_t & tRes, const VecTraits_T<CSphMatch> & dMatches )
{
	assert ( tRes.m_bTagsAssigned );
	assert ( tRes.m_bSingle );
	ARRAY_CONSTFOREACH( i, dMatches )
	{
		const CSphMatch & tMatch = dMatches[i];
		if ( !tRes.m_dResults[tMatch.m_iTag].m_bTag ) // process only matches came from remotes
			continue;
		ResLoc_t & tDoc = dIds.Add ();
		tDoc.m_iDocid = sphGetDocID ( tMatch.m_pDynamic );
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

void FillDocs ( VecTraits_T<CSphMatch> & dMatches, RemoteFieldsAnswer_t& dReply,
		const VecTraits_T<const CSphColumnInfo *>& dFieldCols )
{
	auto & dResDocs = dReply.m_dResDocs; // directly passed from source, sorted by DocID
	auto & dDocs = dReply.m_dDocs; // received, m.b. different (shrinked and unordered)
	int iStride = dFieldCols.GetLength ();

	if ( dDocs.IsEmpty () )
		return;

	// docs and locators placed in unknown order, but for merging we need them ordered.
	CSphFixedVector<int> dOrd { dDocs.GetLength () };
	ARRAY_CONSTFOREACH( i, dOrd )
		dOrd[i] = i;
	dOrd.Sort ( Lesser ( [&dDocs] ( int l, int r ) { return dDocs[l]<dDocs[r]; } ) );

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
			BYTE * pPacked = sphPackPtrAttr ( { dReply.m_pFieldsRaw+dLocators[i].m_iOff, dLocators[i].m_iSize } );
			pPacked = (BYTE*)ExchangeAttr ( tMatch, dFieldCols[i]->m_tLocator, (SphAttr_t)pPacked );
			sphDeallocatePacked ( pPacked );
		}

		++iDoc;
		if ( iDoc>=iDocs )
			break;

		iRecvDoc = dOrd[iDoc];
	}
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

	// fetch stored fields
	DocHash_t tFetched ( tRequest.m_dDocs.GetLength() );
	FieldBlob_t tRes;
	if ( !GetFields ( tRequest, tRes, tFetched ) )
	{
		SendErrorReply ( tOut, "%s", tRes.m_sError.cstr() );
		return;
	}

	SendAPICommandGetfieldAnswer ( tOut, tRequest, tRes, tFetched );
}

void RemotesGetField ( AggrResult_t & tRes, const CSphQuery & tQuery )
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
		return;

	int iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	int iCount = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOffset, iCount );

	// extract ranges and early return in case no remote matches
	CSphVector<ResLoc_t> dIds;
	auto dRangesIds = ExtractRanges ( dIds, tRes, dMatches );
	if ( dRangesIds.IsEmpty() )
		return;

	auto dAgents = GetAgents ( dRangesIds, tRes );
	assert ( dAgents.GetLength ()==dRangesIds.GetLength () );

	// connect to remote agents and query them
	GetFieldRequestBuilder_t tBuilder ( dFieldCols );
	GetFieldReplyParser_t tParser;
	PerformRemoteTasks ( dAgents, &tBuilder, &tParser );

	StringBuilder_c sError { "," };
	if ( !tRes.m_sWarning.IsEmpty () )
		sError << tRes.m_sWarning;
	for ( auto* pAgent : dAgents )
	{
		auto & dReply = *(RemoteFieldsAnswer_t *) pAgent->m_pResult.release ();
		if ( pAgent->m_bSuccess )
			FillDocs ( dMatches, dReply, dFieldCols );
		else if ( !pAgent->m_sFailure.IsEmpty() )
			sError.Sprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
	}
	sError.MoveTo ( tRes.m_sWarning );
}
