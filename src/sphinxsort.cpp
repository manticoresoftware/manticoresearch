//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxsort.h"
#include "sortcomp.h"
#include "aggregate.h"
#include "distinct.h"
#include "netreceive_ql.h"
#include "queuecreator.h"
#include "sortertraits.h"
#include "sortergroup.h"
#include "grouper.h"
#include "knnmisc.h"
#include "joinsorter.h"
#include "querycontext.h"

#include <ctime>

#if !_WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

static bool g_bAccurateAggregation = false;
static int g_iDistinctThresh = 3500;

void SetAccurateAggregationDefault ( bool bEnabled )
{
	g_bAccurateAggregation = bEnabled;
}


bool GetAccurateAggregationDefault()
{
	return g_bAccurateAggregation;
}


void SetDistinctThreshDefault ( int iThresh )
{
	g_iDistinctThresh = iThresh;
}


int GetDistinctThreshDefault()
{
	return g_iDistinctThresh;
}

//////////////////////////////////////////////////////////////////////////
// SORTING QUEUES
//////////////////////////////////////////////////////////////////////////

template < typename COMP >
struct InvCompareIndex_fn
{
	const VecTraits_T<CSphMatch>& m_dBase;
	const CSphMatchComparatorState & m_tState;

	explicit InvCompareIndex_fn ( const CSphMatchQueueTraits & tBase )
		: m_dBase ( tBase.GetMatches() )
		, m_tState ( tBase.GetState() )
	{}

	bool IsLess ( int a, int b ) const // inverts COMP::IsLess
	{
		return COMP::IsLess ( m_dBase[b], m_dBase[a], m_tState );
	}
};

#define LOG_COMPONENT_KMQ __LINE__ << " *(" << this << ") "
#define LOG_LEVEL_DIAG false

#define KMQ LOC(DIAG,KMQ)
/// heap sorter
/// plain binary heap based PQ
template < typename COMP, bool NOTIFICATIONS >
class CSphMatchQueue final : public CSphMatchQueueTraits
{
	using MYTYPE = CSphMatchQueue<COMP, NOTIFICATIONS>;
	LOC_ADD;

public:
	/// ctor
	explicit CSphMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize )
		, m_fnComp ( *this )
	{
		if constexpr ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);
	}

	bool	IsGroupby () const final										{ return false; }
	const CSphMatch * GetWorst() const final								{ return m_dIData.IsEmpty() ? nullptr : Root(); }
	bool	Push ( const CSphMatch & tEntry ) final							{ return PushT ( tEntry, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch ) { m_pSchema->CloneMatch ( tTrg, tMatch ); }); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final
	{
		for ( auto & i : dMatches )
			if ( i.m_tRowID!=INVALID_ROWID )
				PushT ( i, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch ) { m_pSchema->CloneMatch ( tTrg, tMatch ); } );
			else
				m_iTotal++;
	}

	bool	PushGrouped ( const CSphMatch &, bool ) final					{ assert(0); return false; }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) final
	{
		KMQ << "flatten";
		assert ( !IsEmpty() );
		int iReadyMatches = Used();
		pTo += iReadyMatches;
		while ( !IsEmpty() )
		{
			--pTo;
			m_pSchema->FreeDataPtrs(*pTo);
			pTo->ResetDynamic();
			PopAndProcess_T ( [pTo] ( CSphMatch & tRoot )
				{
					Swap ( *pTo, tRoot );
					return true;
				}
			);
		}
		m_iTotal = 0;
		return iReadyMatches;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) final
	{
		KMQ << "finalize";
		if ( !GetLength() )
			return;

		if ( bCallProcessInResultSetOrder )
			m_dIData.Sort ( m_fnComp );

		if ( tProcessor.ProcessInRowIdOrder() )
		{
			CSphFixedVector<int> dSorted ( m_dIData.GetLength() );
			memcpy ( dSorted.Begin(), m_dIData.Begin(), m_dIData.GetLength()*sizeof(m_dIData[0]) );

			// sort by tag, rowid. minimize columnar switches inside expressions and minimize seeks inside columnar iterators
			dSorted.Sort ( Lesser ( [this] ( int l, int r )
				{
					int iTagL = m_dData[l].m_iTag;
					int iTagR = m_dData[r].m_iTag;
					if ( iTagL!=iTagR )
						return iTagL < iTagR;

					return m_dData[l].m_tRowID < m_dData[r].m_tRowID;
				}
			) );

			CSphFixedVector<CSphMatch *> dMatchPtrs ( dSorted.GetLength() );
			ARRAY_FOREACH ( i, dSorted )
				dMatchPtrs[i] = &m_dData[dSorted[i]];

			tProcessor.Process(dMatchPtrs);
		}
		else
		{
			for ( auto iMatch : m_dIData )
				tProcessor.Process ( &m_dData[iMatch] );
		}
	}

	// fixme! test
	ISphMatchSorter * Clone () const final
	{
		auto pClone = new MYTYPE ( m_iSize );
		CloneTo ( pClone );
		return pClone;
	}

	// FIXME! test CSphMatchQueue
	void MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final
	{
		KMQ << "moveto";
//		m_dLogger.Print ();

		auto& dRhs = *(MYTYPE *) pRhs;
		if ( IsEmpty() )
			return; // no matches, nothing to do.

//		dRhs.m_dLogger.Print ();
		// install into virgin sorter - no need to do something; just swap
		if ( dRhs.IsEmpty() )
		{
			SwapMatchQueueTraits ( dRhs );
			return;
		}

		// work as in non-ordered finalize call, but we not need to
		// clone the matches, may just move them instead.

		// total need special care: just add two values and don't rely
		// on result of moving, since it will be wrong
		auto iTotal = dRhs.m_iTotal;
		for ( auto i : m_dIData )
			dRhs.PushT ( m_dData[i], [] ( CSphMatch & tTrg, CSphMatch & tMatch ) { Swap ( tTrg, tMatch ); } );

		dRhs.m_iTotal = m_iTotal + iTotal;
	}

	void SetMerge ( bool bMerge ) final {}

private:
	InvCompareIndex_fn<COMP> m_fnComp;

	CSphMatch * Root() const
	{
		return &m_dData [ m_dIData.First() ];
	}

	/// generic add entry to the queue
	template <typename MATCH, typename PUSHER>
	bool PushT ( MATCH && tEntry, PUSHER && PUSH )
	{
		++m_iTotal;

		if constexpr ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			m_dJustPopped.Resize(0);
		}

		if ( Used()==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, *Root(), m_tState ) )
				return true;
			else
				PopAndProcess_T ( [] ( const CSphMatch & ) { return false; } );
		}

		// do add
		PUSH ( Add(), std::forward<MATCH> ( tEntry ));

		if constexpr ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( *Last() );

		int iEntry = Used()-1;

		// shift up if needed, so that worst (lesser) ones float to the top
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) / 2;
			if ( !m_fnComp.IsLess ( m_dIData[iParent], m_dIData[iEntry] ) )
				break;

			// entry is less than parent, should float to the top
			Swap ( m_dIData[iEntry], m_dIData[iParent] );
			iEntry = iParent;
		}
		return true;
	}

	/// remove root (ie. top priority) entry
	template<typename POPPER>
	void PopAndProcess_T ( POPPER && fnProcess )
	{
		assert ( !IsEmpty() );

		auto& iJustRemoved = m_dIData.Pop();
		if ( !IsEmpty() ) // for empty just popped is the root
			Swap ( m_dIData.First (), iJustRemoved );

		if ( !fnProcess ( m_dData[iJustRemoved] ) )
		{
			// make the last entry my new root
			if constexpr ( NOTIFICATIONS )
			{
				if ( m_dJustPopped.IsEmpty () )
					m_dJustPopped.Add (  RowTagged_t ( m_dData[iJustRemoved] ) );
				else
					m_dJustPopped[0] =  RowTagged_t ( m_dData[iJustRemoved] );
			}
		}

		// sift down if needed
		int iEntry = 0;
		auto iUsed = Used();
		while (true)
		{
			// select child
			int iChild = (iEntry*2) + 1;
			if ( iChild>=iUsed )
				break;

			// select smallest child
			if ( iChild+1<iUsed )
				if ( m_fnComp.IsLess ( m_dIData[iChild], m_dIData[iChild+1] ) )
					++iChild;

			// if smallest child is less than entry, do float it to the top
			if ( m_fnComp.IsLess ( m_dIData[iEntry], m_dIData[iChild] ) )
			{
				Swap ( m_dIData[iChild], m_dIData[iEntry] );
				iEntry = iChild;
				continue;
			}
			break;
		}
	}
};

#define LOG_COMPONENT_KBF __LINE__ << " *(" << this << ") "

#define KBF LOC(DIAG,KBF)
//////////////////////////////////////////////////////////////////////////
/// K-buffer (generalized double buffer) sorter
/// faster worst-case but slower average-case than the heap sorter
/// invoked with select ... OPTION sort_method=kbuffer
template < typename COMP, bool NOTIFICATIONS >
class CSphKbufferMatchQueue : public CSphMatchQueueTraits
{
	using MYTYPE = CSphKbufferMatchQueue<COMP, NOTIFICATIONS>;
	InvCompareIndex_fn<COMP> m_dComp;

	LOC_ADD;

public:
	/// ctor
	explicit CSphKbufferMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize*COEFF )
		, m_dComp ( *this )
	{
		m_iSize /= COEFF;
		if constexpr ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize*(COEFF-1) );
	}

	bool	IsGroupby () const final	{ return false; }
	int		GetLength () final			{ return Min ( Used(), m_iSize ); }
	bool	Push ( const CSphMatch & tEntry ) override	{  return PushT ( tEntry, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch ) { m_pSchema->CloneMatch ( tTrg, tMatch ); }); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) override
	{
		for ( const auto & i : dMatches )
			if ( i.m_tRowID!=INVALID_ROWID )
				PushT ( i, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch ) { m_pSchema->CloneMatch ( tTrg, tMatch ); } );
			else
				m_iTotal++;
	}

	bool	PushGrouped ( const CSphMatch &, bool ) final	{ assert(0); return false; }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) final
	{
		KBF << "Flatten";
		FinalizeMatches ();
		auto iReadyMatches = Used();

		for ( auto iMatch : m_dIData )
		{
			KBF << "fltn " << m_dData[iMatch].m_iTag << ":" << m_dData[iMatch].m_tRowID;
			Swap ( *pTo, m_dData[iMatch] );
			++pTo;
		}

		m_iMaxUsed = ResetDynamic ( m_iMaxUsed );

		// clean up for the next work session
		m_pWorst = nullptr;
		m_iTotal = 0;
		m_bFinalized = false;
		m_dIData.Resize(0);

		return iReadyMatches;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) final
	{
		KBF << "Finalize";
		if ( IsEmpty() )
			return;

		if ( bFinalizeMatches )
			FinalizeMatches();

		for ( auto iMatch : m_dIData )
			tProcessor.Process ( &m_dData[iMatch] );
	}


	ISphMatchSorter* Clone() const final
	{
		auto pClone = new MYTYPE ( m_iSize );
		CloneTo ( pClone );
		return pClone;
	}

	// FIXME! test CSphKbufferMatchQueue
	// FIXME! need to deal with justpushed/justpopped any other way!
	void MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final
	{
		auto& dRhs = *(CSphKbufferMatchQueue<COMP, NOTIFICATIONS>*) pRhs;

		if ( IsEmpty () )
			return;

		if ( dRhs.IsEmpty () )
		{
			SwapMatchQueueTraits (dRhs);
			dRhs.m_pWorst = m_pWorst;
			dRhs.m_bFinalized = m_bFinalized;
			return;
		}

		FinalizeMatches();


		// both are non-empty - need to process.
		// work as finalize call, but don't clone the matches; move them instead.

		// total need special care!
		auto iTotal = dRhs.m_iTotal;
		for ( auto iMatch : m_dIData )
		{
			dRhs.PushT ( m_dData[iMatch],
			[] ( CSphMatch & tTrg, CSphMatch & tMatch ) {
				Swap ( tTrg, tMatch );
			});
		}
		dRhs.m_iTotal = m_iTotal + iTotal;
	}

	void				SetMerge ( bool bMerge ) final {}

protected:
	CSphMatch *			m_pWorst = nullptr;
	bool				m_bFinalized = false;
	int					m_iMaxUsed = -1;

	static const int COEFF = 4;

private:
	void SortMatches () // sort from best to worst
	{
		m_dIData.Sort ( m_dComp );
	}

	void FreeMatch ( int iMatch )
	{
		if constexpr ( NOTIFICATIONS )
			m_dJustPopped.Add ( RowTagged_t ( m_dData[iMatch] ) );
		m_pSchema->FreeDataPtrs ( m_dData[iMatch] );
	}

	void CutTail()
	{
		if ( Used()<=m_iSize)
			return;

		m_iMaxUsed = Max ( m_iMaxUsed, this->m_dIData.GetLength () ); // memorize it for free dynamics later.

		m_dIData.Slice ( m_iSize ).Apply ( [this] ( int iMatch ) { FreeMatch ( iMatch ); } );
		m_dIData.Resize ( m_iSize );
	}

	// conception: we have array of N*COEFF elems.
	// We need only N the best elements from it (rest have to be disposed).
	// direct way: rsort, then take first N elems.
	// this way: rearrange array by performing one pass of quick sort
	// if we have exactly N elems left hand from pivot - we're done.
	// otherwise repeat rearranging only to right or left part until the target achieved.
	void BinaryPartition ()
	{
		int iPivot = m_dIData[m_iSize / COEFF+1];
		int iMaxIndex = m_iSize-1;
		int a=0;
		int b=Used()-1;
		while (true)
		{
			int i=a;
			int j=b;
			while (i<=j)
			{
				while (m_dComp.IsLess (m_dIData[i],iPivot)) 	++i;
				while (m_dComp.IsLess (iPivot, m_dIData[j]))	--j;
				if ( i<=j ) ::Swap( m_dIData[i++], m_dIData[j--]);
			}
			if ( iMaxIndex == j )
				break;

			if ( iMaxIndex < j)
				b = j;  // too many elems acquired; continue with left part
			else
				a = i;  // too less elems acquired; continue with right part
			iPivot = m_dIData[( a * ( COEFF-1 )+b ) / COEFF];
		}
	}

	void RepartitionMatches ()
	{
		assert ( Used ()>m_iSize );
		BinaryPartition ();

		CutTail();
	}

	void FinalizeMatches ()
	{
		if ( m_bFinalized )
			return;

		m_bFinalized = true;

		if ( Used ()>m_iSize )
			RepartitionMatches();

		SortMatches();
	}

	// generic push entry (add it some way to the queue clone or swap PUSHER depends on)
	template<typename MATCH, typename PUSHER>
	FORCE_INLINE bool PushT ( MATCH && tEntry, PUSHER && PUSH )
	{
		if constexpr ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			m_dJustPopped.Resize(0);
		}

		// quick early rejection checks
		++m_iTotal;
		if ( m_pWorst && COMP::IsLess ( tEntry, *m_pWorst, m_tState ) )
			return true;

		// quick check passed
		// fill the data, back to front
		m_bFinalized = false;
		PUSH ( Add(), std::forward<MATCH> ( tEntry ));

		if constexpr ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( *Last() );

		// do the initial sort once
		if ( m_iTotal==m_iSize )
		{
			assert ( Used()==m_iSize && !m_pWorst );
			SortMatches();
			m_pWorst = Last();
			m_bFinalized = true;
			return true;
		}

		if ( Used ()<m_iSize*COEFF )
			return true;

		// do the sort/cut when the K-buffer is full
		assert ( Used ()==m_iSize*COEFF );

		RepartitionMatches();
		SortMatches ();
		m_pWorst = Last ();
		m_bFinalized = true;
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////

/// collect list of matched DOCIDs in aside compressed blob
/// (mainly used to collect docs in `DELETE... WHERE` statement)
class CollectQueue_c final : public MatchSorter_c, ISphNoncopyable
{
	using BASE = MatchSorter_c;

public:
						CollectQueue_c ( int iSize, CSphVector<BYTE>& dCollectedValues );

	bool				IsGroupby () const final { return false; }
	int					GetLength () final { return 0; } // that ensures, flatten() will never called;
	bool				Push ( const CSphMatch& tEntry ) final { return PushMatch(tEntry); }
	void				Push ( const VecTraits_T<const CSphMatch> & dMatches ) final
	{
		for ( const auto & i : dMatches )
			if ( i.m_tRowID!=INVALID_ROWID )
				PushMatch(i);
	}

	bool				PushGrouped ( const CSphMatch &, bool ) final { assert(0); return false; }
	int					Flatten ( CSphMatch * ) final { return 0; }
	void				Finalize ( MatchProcessor_i &, bool, bool ) final;
	bool				CanBeCloned() const final { return false; }
	ISphMatchSorter *	Clone () const final { return nullptr; }
	void				MoveTo ( ISphMatchSorter *, bool ) final {}
	void				SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final;
	bool				IsCutoffDisabled() const final { return true; }
	void				SetMerge ( bool bMerge ) final {}

private:
	DocID_t						m_iLastID;
	int							m_iMaxMatches;
	CSphVector<DocID_t>			m_dUnsortedDocs;
	MemoryWriter_c				m_tWriter;
	bool						m_bDocIdDynamic = false;

	inline bool			PushMatch ( const CSphMatch & tEntry );
	inline void			ProcessPushed();
};


CollectQueue_c::CollectQueue_c ( int iSize, CSphVector<BYTE>& dCollectedValues )
	: m_iLastID ( 0 )
	, m_iMaxMatches ( iSize  )
	, m_tWriter ( dCollectedValues )
{}


/// sort/uniq already collected and store them to writer
void CollectQueue_c::ProcessPushed()
{
	m_dUnsortedDocs.Uniq();
	for ( auto& iCurId : m_dUnsortedDocs )
		m_tWriter.ZipOffset ( iCurId - std::exchange ( m_iLastID, iCurId ) );
	m_dUnsortedDocs.Resize ( 0 );
}


bool CollectQueue_c::PushMatch ( const CSphMatch & tEntry )
{
	if ( m_dUnsortedDocs.GetLength() >= m_iMaxMatches && m_dUnsortedDocs.GetLength() == m_dUnsortedDocs.GetLimit() )
		ProcessPushed();

	m_dUnsortedDocs.Add ( sphGetDocID ( m_bDocIdDynamic ? tEntry.m_pDynamic : tEntry.m_pStatic ) );
	return true;
}

/// final update pass
void CollectQueue_c::Finalize ( MatchProcessor_i&, bool, bool )
{
	ProcessPushed();
	m_iLastID = 0;
}


void CollectQueue_c::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	BASE::SetSchema ( pSchema, bRemapCmp );

	const CSphColumnInfo * pDocId = pSchema->GetAttr ( sphGetDocidName() );
	assert(pDocId);
	m_bDocIdDynamic = pDocId->m_tLocator.m_bDynamic;
}


ISphMatchSorter * CreateCollectQueue ( int iMaxMatches, CSphVector<BYTE> & tCollection )
{
	return new CollectQueue_c ( iMaxMatches, tCollection );
}

//////////////////////////////////////////////////////////////////////////

void SendSqlSchema ( const ISphSchema& tSchema, RowBuffer_i* pRows, const VecTraits_T<int>& dOrder )
{
	pRows->HeadBegin ();
	ARRAY_CONSTFOREACH ( i, dOrder )
	{
		const CSphColumnInfo& tCol = tSchema.GetAttr ( dOrder[i] );
		if ( sphIsInternalAttr ( tCol ) )
			continue;
		if ( i == 0 )
		{
			assert (tCol.m_sName == "id");
			pRows->HeadColumn ( "id", ESphAttr2MysqlColumnStreamed ( SPH_ATTR_UINT64 ) );
			continue;
		}
		if ( tCol.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			continue;
		pRows->HeadColumn ( tCol.m_sName.cstr(), ESphAttr2MysqlColumnStreamed ( tCol.m_eAttrType ) );
	}

	pRows->HeadEnd ( false, 0 );
}

using SqlEscapedBuilder_c = EscapedStringBuilder_T<BaseQuotation_T<SqlQuotator_t>>;

void SendSqlMatch ( const ISphSchema& tSchema, RowBuffer_i* pRows, CSphMatch& tMatch, const BYTE* pBlobPool, const VecTraits_T<int>& dOrder, bool bDynamicDocid )
{
	auto& dRows = *pRows;
	ARRAY_CONSTFOREACH ( i, dOrder )
	{
		const CSphColumnInfo& dAttr = tSchema.GetAttr ( dOrder[i] );
		if ( sphIsInternalAttr ( dAttr ) )
			continue;
		if ( dAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			continue;

		CSphAttrLocator tLoc = dAttr.m_tLocator;
		ESphAttr eAttrType = dAttr.m_eAttrType;

		if ( i == 0 )
			eAttrType = SPH_ATTR_UINT64;

		switch ( eAttrType )
		{
		case SPH_ATTR_STRING:
			dRows.PutArray ( sphGetBlobAttr ( tMatch, tLoc, pBlobPool ) );
			break;
		case SPH_ATTR_STRINGPTR:
			{
				const BYTE* pStr = nullptr;
				if ( dAttr.m_eStage == SPH_EVAL_POSTLIMIT )
				{
					if ( bDynamicDocid )
					{
						dAttr.m_pExpr->StringEval ( tMatch, &pStr );
					} else
					{
						auto pDynamic = tMatch.m_pDynamic;
						if ( tMatch.m_pStatic )
							tMatch.m_pDynamic = nullptr;
						dAttr.m_pExpr->StringEval ( tMatch, &pStr );
						tMatch.m_pDynamic = pDynamic;
					}
					dRows.PutString ( (const char*)pStr );
					SafeDeleteArray ( pStr );
				} else {
					pStr = (const BYTE*)tMatch.GetAttr ( tLoc );
					auto dString = sphUnpackPtrAttr ( pStr );
					dRows.PutArray ( dString );
				}
			}
			break;

		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
			dRows.PutNumAsString ( (DWORD)tMatch.GetAttr ( tLoc ) );
			break;

		case SPH_ATTR_BIGINT:
			dRows.PutNumAsString ( tMatch.GetAttr ( tLoc ) );
			break;

		case SPH_ATTR_UINT64:
			dRows.PutNumAsString ( (uint64_t)tMatch.GetAttr ( tLoc ) );
			break;

		case SPH_ATTR_FLOAT:
			dRows.PutFloatAsString ( tMatch.GetAttrFloat ( tLoc ) );
			break;

		case SPH_ATTR_DOUBLE:
			dRows.PutDoubleAsString ( tMatch.GetAttrDouble ( tLoc ) );
			break;

		case SPH_ATTR_INT64SET:
		case SPH_ATTR_UINT32SET:
			{
				StringBuilder_c dStr;
				auto dMVA = sphGetBlobAttr ( tMatch, tLoc, pBlobPool );
				dStr << "(";
				sphMVA2Str ( dMVA, eAttrType == SPH_ATTR_INT64SET, dStr );
				dStr << ")";
				dRows.PutArray ( dStr, false );
				break;
			}

		case SPH_ATTR_INT64SET_PTR:
		case SPH_ATTR_UINT32SET_PTR:
			{
				StringBuilder_c dStr;
				dStr << "(";
				sphPackedMVA2Str ( (const BYTE*)tMatch.GetAttr ( tLoc ), eAttrType == SPH_ATTR_INT64SET_PTR, dStr );
				dStr << ")";
				dRows.PutArray ( dStr, false );
				break;
			}

		case SPH_ATTR_FLOAT_VECTOR:
			{
				StringBuilder_c dStr;
				auto dFloatVec = sphGetBlobAttr ( tMatch, tLoc, pBlobPool );
				dStr << "(";
				sphFloatVec2Str ( dFloatVec, dStr );
				dStr << ")";
				dRows.PutArray ( dStr, false );
			}
			break;

		case SPH_ATTR_FLOAT_VECTOR_PTR:
			{
				StringBuilder_c dStr;
				dStr << "(";
				sphPackedFloatVec2Str ( (const BYTE*)tMatch.GetAttr(tLoc), dStr );
				dStr << ")";
				dRows.PutArray ( dStr, false );
			}
			break;

		case SPH_ATTR_JSON:
			{
				auto pJson = sphGetBlobAttr ( tMatch, tLoc, pBlobPool );
				JsonEscapedBuilder sTmp;
				if ( pJson.second )
					sphJsonFormat ( sTmp, pJson.first );
				auto sJson = Str_t(sTmp);
				SqlEscapedBuilder_c dEscaped;
				dEscaped.FixupSpacedAndAppendEscapedNoQuotes ( sJson.first, sJson.second );
				dRows.PutArray ( dEscaped, false );
			}
			break;
		case SPH_ATTR_JSON_PTR:
			{
				auto* pString = (const BYTE*)tMatch.GetAttr ( tLoc );
				JsonEscapedBuilder sTmp;
				if ( pString )
				{
					auto dJson = sphUnpackPtrAttr ( pString );
					sphJsonFormat ( sTmp, dJson.first );
				}
				auto sJson = Str_t ( sTmp );
				SqlEscapedBuilder_c dEscaped;
				dEscaped.FixupSpacedAndAppendEscapedNoQuotes ( sJson.first, sJson.second );
				dRows.PutArray ( dEscaped, false );
			}
			break;

		case SPH_ATTR_FACTORS:
		case SPH_ATTR_FACTORS_JSON:
		case SPH_ATTR_JSON_FIELD:
		case SPH_ATTR_JSON_FIELD_PTR:
			assert ( false ); // index schema never contain such column
			break;

		default:
			dRows.Add ( 1 );
			dRows.Add ( '-' );
			break;
		}
	}
	if ( !dRows.Commit() )
		session::SetKilled ( true );
}

/// stream out matches
class DirectSqlQueue_c final : public MatchSorter_c, ISphNoncopyable
{
	using BASE = MatchSorter_c;

public:
						DirectSqlQueue_c ( RowBuffer_i * pOutput, void ** ppOpaque1, void ** ppOpaque2, StrVec_t dColumns );
						~DirectSqlQueue_c() override;

	bool				IsGroupby () const final { return false; }
	int					GetLength () final { return 0; } // that ensures, flatten() will never called;
	bool				Push ( const CSphMatch& tEntry ) final { return PushMatch(const_cast<CSphMatch&>(tEntry)); }
	void				Push ( const VecTraits_T<const CSphMatch> & dMatches ) final
	{
		for ( const auto & i : dMatches )
			if ( i.m_tRowID!=INVALID_ROWID )
				PushMatch(const_cast<CSphMatch&>(i));
	}

	bool				PushGrouped ( const CSphMatch &, bool ) final { assert(0); return false; }
	int					Flatten ( CSphMatch * ) final { return 0; }
	void				Finalize ( MatchProcessor_i &, bool, bool ) final;
	bool				CanBeCloned() const final { return false; }
	ISphMatchSorter *	Clone () const final { return nullptr; }
	void				MoveTo ( ISphMatchSorter *, bool ) final {}
	void				SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final;
	bool				IsCutoffDisabled() const final { return true; }
	void				SetMerge ( bool bMerge ) final {}

	void SetBlobPool ( const BYTE* pBlobPool ) final
	{
		m_pBlobPool = pBlobPool;
		MakeCtx();
	}

	void SetColumnar ( columnar::Columnar_i* pColumnar ) final
	{
		m_pColumnar = pColumnar;
		MakeCtx();
	}

private:
	bool 						m_bSchemaSent = false;
	int64_t						m_iDocs = 0;
	RowBuffer_i*				m_pOutput;
	const BYTE*					m_pBlobPool = nullptr;
	columnar::Columnar_i*		m_pColumnar = nullptr;
	CSphVector<ISphExpr*>		m_dDocstores;
	CSphVector<ISphExpr*>		m_dFinals;
	void ** 					m_ppOpaque1 = nullptr;
	void ** 					m_ppOpaque2 = nullptr;
	void *						m_pCurDocstore = nullptr;
	void *						m_pCurDocstoreReader = nullptr;
	CSphQuery					m_dFake;
	CSphQueryContext			m_dCtx;
	StrVec_t					m_dColumns;
	CSphVector<int>				m_dOrder;
	bool						m_bDynamicDocid;
	bool						m_bNotYetFinalized = true;

	inline bool			PushMatch ( CSphMatch & tEntry );
	void				SendSchemaOnce();
	void				FinalizeOnce();
	void				MakeCtx();
};


DirectSqlQueue_c::DirectSqlQueue_c ( RowBuffer_i * pOutput, void ** ppOpaque1, void ** ppOpaque2, StrVec_t dColumns )
	: m_pOutput ( pOutput )
	, m_ppOpaque1 ( ppOpaque1 )
	, m_ppOpaque2 ( ppOpaque2 )
	, m_dCtx (m_dFake)
	, m_dColumns ( std::move ( dColumns ) )
{}

DirectSqlQueue_c::~DirectSqlQueue_c()
{
	FinalizeOnce();
}

void DirectSqlQueue_c::SendSchemaOnce()
{
	if ( m_bSchemaSent )
		return;

	assert ( !m_iDocs );
	for ( const auto& sColumn : m_dColumns )
	{
		auto iIdx = m_pSchema->GetAttrIndex ( sColumn.cstr() );
		if ( iIdx >= 0 )
			m_dOrder.Add ( iIdx );
	}

	for ( int i = 0; i < m_pSchema->GetAttrsCount(); ++i )
	{
		auto& tCol = const_cast< CSphColumnInfo &>(m_pSchema->GetAttr ( i ));
		if ( tCol.m_sName == sphGetDocidName() )
			m_bDynamicDocid = tCol.m_tLocator.m_bDynamic;

		if ( !tCol.m_pExpr )
			continue;
		switch ( tCol.m_eStage )
		{
		case SPH_EVAL_FINAL : m_dFinals.Add ( tCol.m_pExpr ); break;
		case SPH_EVAL_POSTLIMIT: m_dDocstores.Add ( tCol.m_pExpr ); break;
		default:
			sphWarning ("Unknown stage in SendSchema(): %d", tCol.m_eStage);
		}
	}

	SendSqlSchema ( *m_pSchema, m_pOutput, m_dOrder );
	m_bSchemaSent = true;
}

void DirectSqlQueue_c::MakeCtx()
{
	CSphQueryResultMeta tFakeMeta;
	CSphVector<const ISphSchema*> tFakeSchemas;
	m_dCtx.SetupCalc ( tFakeMeta, *m_pSchema, *m_pSchema, m_pBlobPool, m_pColumnar, tFakeSchemas );
}


bool DirectSqlQueue_c::PushMatch ( CSphMatch & tEntry )
{
	SendSchemaOnce();
	++m_iDocs;

	if ( m_ppOpaque1 )
	{
		auto pDocstoreReader = *m_ppOpaque1;
		if ( pDocstoreReader!=std::exchange (m_pCurDocstore, pDocstoreReader) && pDocstoreReader )
		{
			DocstoreSession_c::InfoDocID_t tSessionInfo;
			tSessionInfo.m_pDocstore = (const DocstoreReader_i *)pDocstoreReader;
			tSessionInfo.m_iSessionId = -1;

			// value is copied; no leak of pointer to local here.
			m_dDocstores.for_each ( [&tSessionInfo] ( ISphExpr* pExpr ) { pExpr->Command ( SPH_EXPR_SET_DOCSTORE_DOCID, &tSessionInfo ); } );
		}
	}

	if ( m_ppOpaque2 )
	{
		auto pDocstore = *m_ppOpaque2;
		if ( pDocstore != std::exchange ( m_pCurDocstoreReader, pDocstore ) && pDocstore )
		{
			DocstoreSession_c::InfoRowID_t tSessionInfo;
			tSessionInfo.m_pDocstore = (Docstore_i*)pDocstore;
			tSessionInfo.m_iSessionId = -1;

			// value is copied; no leak of pointer to local here.
			m_dFinals.for_each ( [&tSessionInfo] ( ISphExpr* pExpr ) { pExpr->Command ( SPH_EXPR_SET_DOCSTORE_ROWID, &tSessionInfo ); } );
		}
	}

	m_dCtx.CalcFinal(tEntry);

	SendSqlMatch ( *m_pSchema, m_pOutput, tEntry, m_pBlobPool, m_dOrder, m_bDynamicDocid );
	return true;
}

/// final update pass
void DirectSqlQueue_c::Finalize ( MatchProcessor_i&, bool, bool bFinalizeMatches )
{
	if ( !bFinalizeMatches )
		return;

	FinalizeOnce();
}

void DirectSqlQueue_c::FinalizeOnce ()
{
	if ( !std::exchange ( m_bNotYetFinalized, false ) )
		return;

	SendSchemaOnce();
	m_pOutput->Eof();
}


void DirectSqlQueue_c::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	BASE::SetSchema ( pSchema, bRemapCmp );
}


ISphMatchSorter * CreateDirectSqlQueue ( RowBuffer_i * pOutput, void ** ppOpaque1, void ** ppOpaque2, const StrVec_t & dColumns )
{
	return new DirectSqlQueue_c ( pOutput, ppOpaque1, ppOpaque2, dColumns );
}

//////////////////////////////////////////////////////////////////////////
// SORT CLAUSE PARSER
//////////////////////////////////////////////////////////////////////////

class SortClauseTokenizer_t
{
protected:
	const char * m_pCur;
	const char * m_pMax;
	char * m_pBuf;

protected:
	char ToLower ( char c )
	{
		// 0..9, A..Z->a..z, _, a..z, @, .
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || c=='_' || c=='@' || c=='.' || c=='[' || c==']' || c=='\'' || c=='\"' || c=='(' || c==')' || c=='*' )
			return c;
		if ( c>='A' && c<='Z' )
			return c-'A'+'a';
		return 0;
	}

public:
	explicit SortClauseTokenizer_t ( const char * sBuffer )
	{
		auto iLen = (int) strlen(sBuffer);
		m_pBuf = new char [ iLen+1 ];
		m_pMax = m_pBuf+iLen;
		m_pCur = m_pBuf;

		// make string lowercase but keep case of JSON.field
		bool bJson = false;
		for ( int i=0; i<=iLen; i++ )
		{
			char cSrc = sBuffer[i];
			char cDst = ToLower ( cSrc );
			bJson = ( cSrc=='.' || cSrc=='[' || ( bJson && cDst>0 ) ); // keep case of valid char sequence after '.' and '[' symbols
			m_pBuf[i] = bJson ? cSrc : cDst;
		}
	}

	~SortClauseTokenizer_t ()
	{
		SafeDeleteArray ( m_pBuf );
	}

	const char * GetToken ()
	{
		// skip spaces
		while ( m_pCur<m_pMax && !*m_pCur )
			m_pCur++;
		if ( m_pCur>=m_pMax )
			return nullptr;

		// memorize token start, and move pointer forward
		const char * sRes = m_pCur;
		while ( *m_pCur )
			m_pCur++;
		return sRes;
	}

	bool IsSparseCount ( const char * sTok )
	{
		const char * sSeq = "(*)";
		for ( ; sTok<m_pMax && *sSeq; sTok++ )
		{
			bool bGotSeq = ( *sSeq==*sTok );
			if ( bGotSeq )
				sSeq++;

			// stop checking on any non-space char outside sequence or sequence end
			if ( ( !bGotSeq && !sphIsSpace ( *sTok ) && *sTok!='\0' ) || !*sSeq )
				break;
		}

		if ( !*sSeq && sTok+1<m_pMax && !sTok[1] )
		{
			// advance token iterator after composite count(*) token
			m_pCur = sTok+1;
			return true;
		} else
		{
			return false;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING INSTANTIATION
//////////////////////////////////////////////////////////////////////////

ISphMatchSorter * CreateSorter ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const PrecalculatedSorterResults_t & tPrecalc )
{
	CSphRefcountedPtr<ISphMatchComparator> pComp;
	if ( !tSettings.m_bImplicit )
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn(); break;
			case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn(); break;
			case FUNC_GENERIC1:		pComp = new MatchGeneric1_fn(); break;
			case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn(); break;
			case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn(); break;
			case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn(); break;
			case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn(); break;
			case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
		}

	return CreateGroupSorter ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
}

/////////////////////////
// SORTING QUEUE FACTORY
/////////////////////////

template < typename COMP >
static ISphMatchSorter * CreatePlainSorter ( bool bKbuffer, int iMaxMatches, bool bFactors )
{
	if ( bKbuffer )
	{
		if ( bFactors )
			return new CSphKbufferMatchQueue<COMP, true> ( iMaxMatches );
		return new CSphKbufferMatchQueue<COMP, false> ( iMaxMatches );
	}
	if ( bFactors )
		return new CSphMatchQueue<COMP, true> ( iMaxMatches );
	return new CSphMatchQueue<COMP, false> ( iMaxMatches );
}


ISphMatchSorter * CreatePlainSorter ( ESphSortFunc eMatchFunc, bool bKbuffer, int iMaxMatches, bool bFactors )
{
	switch ( eMatchFunc )
	{
		case FUNC_REL_DESC:		return CreatePlainSorter<MatchRelevanceLt_fn>	( bKbuffer, iMaxMatches, bFactors );
		case FUNC_TIMESEGS:		return CreatePlainSorter<MatchTimeSegments_fn>	( bKbuffer, iMaxMatches, bFactors );
		case FUNC_GENERIC1:		return CreatePlainSorter<MatchGeneric1_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_GENERIC2:		return CreatePlainSorter<MatchGeneric2_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_GENERIC3:		return CreatePlainSorter<MatchGeneric3_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_GENERIC4:		return CreatePlainSorter<MatchGeneric4_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_GENERIC5:		return CreatePlainSorter<MatchGeneric5_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_EXPR:			return CreatePlainSorter<MatchExpr_fn>			( bKbuffer, iMaxMatches, bFactors );
		default:				return nullptr;
	}
}


int ApplyImplicitCutoff ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters, bool bFT )
{
	bool bAllPrecalc = dSorters.GetLength() && dSorters.all_of ( []( auto pSorter ){ return pSorter->IsPrecalc(); } );
	if ( bAllPrecalc )
		return 1;	// only need one match for precalc sorters

	if ( tQuery.m_iCutoff>0 )
		return tQuery.m_iCutoff;

	if ( !tQuery.m_iCutoff )
		return -1;

	// this is the same as checking the sorters for disabled cutoff
	// but this works when sorters are not yet available (e.g. GetPseudoShardingMetric())
	if ( HasImplicitGrouping ( tQuery ) )
		return -1;

	if ( !tQuery.m_sKNNAttr.IsEmpty() )
		return -1;

	bool bDisableCutoff = dSorters.any_of ( []( auto * pSorter ){ return pSorter->IsCutoffDisabled(); } );
	if ( bDisableCutoff )
		return -1;

	// implicit cutoff when there's no sorting and no grouping
	if ( !bFT && ( tQuery.m_sSortBy=="@weight desc" || tQuery.m_sSortBy.IsEmpty() ) && tQuery.m_sGroupBy.IsEmpty() && !tQuery.m_bFacet && !tQuery.m_bFacetHead )
		return tQuery.m_iLimit+tQuery.m_iOffset;

	return -1;
}
