//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "attribute.h"

#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// TRAITS
//////////////////////////////////////////////////////////////////////////

void sphFixupLocator ( CSphAttrLocator & tLocator, const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	// first time schema setup?
	if ( !pOldSchema )
		return;

	if ( tLocator.m_iBlobAttrId==-1 && tLocator.m_iBitCount==-1 )
		return;

	assert ( pNewSchema );
	for ( int i = 0; i < pOldSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
		if ( tLocator==tAttr.m_tLocator )
		{
			const CSphColumnInfo * pAttrInNewSchema = pNewSchema->GetAttr ( tAttr.m_sName.cstr() );
			if ( pAttrInNewSchema )
			{
				tLocator = pAttrInNewSchema->m_tLocator;
				return;
			}
		}
	}
}

namespace {

const char g_sIntAttrPrefix[] = "@int_str2ptr_";
const char g_sIntJsonPrefix[] = "@groupbystr";

template <typename FN>
void FnSortGetStringRemap ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema, FN fnProcess )
{
	for ( int i = 0; i<tDstSchema.GetAttrsCount (); ++i )
	{
		const CSphColumnInfo & tDst = tDstSchema.GetAttr ( i );
		// remap only static strings
		if ( tDst.m_eAttrType==SPH_ATTR_STRINGPTR || !IsSortStringInternal ( tDst.m_sName ) )
			continue;

		auto iSrcCol = tSrcSchema.GetAttrIndex ( tDst.m_sName.cstr ()+sizeof ( g_sIntAttrPrefix )-1 );
		if ( iSrcCol!=-1 ) // skip internal attributes received from agents
			fnProcess ( iSrcCol, i );
	}
}
} // unnamed (static) namespace

int GetStringRemapCount ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema )
{
	int iMaps = 0;
	FnSortGetStringRemap ( tDstSchema, tSrcSchema, [&iMaps] ( int, int ) { ++iMaps; } );
	return iMaps;
}

//////////////////////////////////////////////////////////////////////////
void ISphMatchSorter::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	assert ( pSchema );
	m_tState.FixupLocators ( m_pSchema, pSchema, bRemapCmp );

	m_pSchema = pSchema;
}


void ISphMatchSorter::SetState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	for ( ISphExpr *&pExpr :  m_tState.m_tSubExpr ) SafeAddRef ( pExpr );
	m_tState.m_iNow = (DWORD) time ( nullptr );
}

void ISphMatchSorter::CopyState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	for ( ISphExpr *& pExpr :  m_tState.m_tSubExpr )
		pExpr = SafeClone (pExpr);
	m_tState.m_iNow = (DWORD) time ( nullptr );
}

void ISphMatchSorter::CloneTo ( ISphMatchSorter * pTrg ) const
{
	assert ( pTrg );
	pTrg->m_bRandomize = m_bRandomize;
	pTrg->m_dJustPopped.Reserve ( m_dJustPopped.GetLimit () );
	pTrg->m_pSchema = m_pSchema->CloneMe();
	pTrg->CopyState ( m_tState );
}

void ISphMatchSorter::SetFilteredAttrs ( const sph::StringSet & hAttrs )
{
	assert ( m_pSchema );

	m_dTransormed.Reserve ( hAttrs.GetLength() );

	// DocID attribute always MUST to be the first
	// FIXME!!! DocID used at sorter to merge multiple result sets at KillDupes, for cases: for non group, for group but with ranker=none
	// this auto add DocID should be removed when implicit order by DocID is removed too (WAND ranking maybe?)
	m_dTransormed.Add ( sphGetDocidName() );

	for ( auto& tName : hAttrs )
	{
		const CSphColumnInfo * pCol = m_pSchema->GetAttr ( tName.first.cstr() );
		if ( pCol && pCol->m_sName!=sphGetDocidName() )
			m_dTransormed.Add ( pCol->m_sName );
	}
}

// helper
class MatchesToNewSchema_c : public ISphMatchProcessor
{
	struct MapAction_t {

		// what is to do with current position
		enum Action_e {
			SETZERO,		// set default (0)
			COPY,			// copy as is (plain attribute)
			COPYBLOB,		// deep copy (unpack/pack) the blob
			COPYJSONFIELD,	// json field (packed blob with type)
		};

		const CSphAttrLocator*	m_pFrom;
		const CSphAttrLocator*	m_pTo;
		Action_e				m_eAction;
	};

	int	m_iDynamicSize;		// target dynamic size, from schema
	CSphVector<MapAction_t> m_dActions;		// the recipe
	CSphVector<std::pair<CSphAttrLocator, CSphAttrLocator>> m_dRemapCmp;	// remap @int_str2ptr_ATTR -> ATTR
	CSphVector<int> m_dDataPtrAttrs;		// orphaned attrs we have to free before swap to new attr
	fnGetBlobPoolFromMatch m_fnGetBlobPool;	// provides base for pool copying

public:
	MatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, fnGetBlobPoolFromMatch fnGetBlobPool )
		: m_iDynamicSize ( pNewSchema->GetDynamicSize () )
		, m_fnGetBlobPool ( std::move ( fnGetBlobPool ) )
	{
		assert ( pOldSchema && pNewSchema );

		// prepare transforming recipe

		// initial state: set all new columns to be reset by default
		for ( int i = 0; i<pNewSchema->GetAttrsCount(); ++i )
			m_dActions.Add ( { nullptr, &pNewSchema->GetAttr(i).m_tLocator, MapAction_t::SETZERO } );

		// add mapping from old to new according to column type
		for ( int i = 0; i<pOldSchema->GetAttrsCount(); ++i )
		{
			const CSphColumnInfo & dOld = pOldSchema->GetAttr(i);
			auto iNewIdx = pNewSchema->GetAttrIndex ( dOld.m_sName.cstr () );
			if ( iNewIdx == -1 )
			{
				// dataptr present in old, but not in the new - mark it for releasing
				if ( sphIsDataPtrAttr ( dOld.m_eAttrType ) && dOld.m_tLocator.m_bDynamic )
					m_dDataPtrAttrs.Add( dOld.m_tLocator.m_iBitOffset >> ROWITEM_SHIFT );
				continue;
			}

			const CSphColumnInfo & dNew = pNewSchema->GetAttr(iNewIdx);

			auto & tAction = m_dActions[iNewIdx];
			tAction.m_pFrom = &dOld.m_tLocator;

			// same type - just copy attr as is
			if ( dOld.m_eAttrType==dNew.m_eAttrType )
			{
				tAction.m_eAction = MapAction_t::COPY;
				continue;
			}

			assert ( !sphIsDataPtrAttr ( dOld.m_eAttrType ) && sphIsDataPtrAttr ( dNew.m_eAttrType ) );

			switch ( dOld.m_eAttrType )
			{
				case SPH_ATTR_UINT32SET:
				case SPH_ATTR_INT64SET:
				case SPH_ATTR_STRING:
				case SPH_ATTR_JSON:
					tAction.m_eAction = MapAction_t::COPYBLOB;
					break;

				case SPH_ATTR_JSON_FIELD:
					tAction.m_eAction = MapAction_t::COPYJSONFIELD;
					break;

				default:
					assert ( 0 && "Unexpected attribute type" );
					break;
			}
		}


		// need to update @int_str2ptr_ locator to use new schema
		// no need to pass pOldSchema as we remapping only new schema pointers
		// also need to update group sorter keypart to be str_ptr in caller code SetSchema
		FnSortGetStringRemap ( *pNewSchema, *pNewSchema, [this, pNewSchema] ( int iSrc, int iDst ) {
			m_dRemapCmp.Add ( { pNewSchema->GetAttr(iSrc).m_tLocator, pNewSchema->GetAttr(iDst).m_tLocator } );
		});
	}

	// performs actual processing acording created plan
	void Process ( CSphMatch * pMatch ) final
	{
		CSphMatch tResult;
		tResult.Reset ( m_iDynamicSize );

		const BYTE * pBlobPool = m_fnGetBlobPool ( pMatch );

		for ( const auto & tAction : m_dActions )
		{
			SphAttr_t uValue = 0;
			switch ( tAction.m_eAction )
			{
			case MapAction_t::SETZERO:
				break;

			case MapAction_t::COPY:
				uValue = pMatch->GetAttr ( *tAction.m_pFrom );
				break;

			case MapAction_t::COPYBLOB:
				{
					auto dBlob = sphGetBlobAttr ( *pMatch, *tAction.m_pFrom, pBlobPool );
					uValue = (SphAttr_t) sphPackPtrAttr ( dBlob );
				}
				break;

			case MapAction_t::COPYJSONFIELD:
				{
					SphAttr_t uPacked = pMatch->GetAttr ( *tAction.m_pFrom );
					const BYTE * pStr = uPacked ? pBlobPool+sphJsonUnpackOffset ( uPacked ) : nullptr;
					ESphJsonType eJson = sphJsonUnpackType ( uPacked );

					if ( pStr && eJson!=JSON_NULL )
					{
						int iLengthBytes = sphJsonNodeSize ( eJson, pStr );
						BYTE * pData = nullptr;
						uValue = (SphAttr_t) sphPackPtrAttr ( iLengthBytes+1, &pData );

						// store field type before the field
						*pData = (BYTE) eJson;
						memcpy ( pData+1, pStr, iLengthBytes );
					}
				}
				break;

			default:
				assert(false && "Unknown state");
			}

			tResult.SetAttr ( *tAction.m_pTo, uValue );
		}

		// remap comparator attributes
		for ( const auto & tRemap : m_dRemapCmp )
			tResult.SetAttr ( tRemap.second, tResult.GetAttr ( tRemap.first ) );

		// free original orphaned pointers
		CSphSchemaHelper::FreeDataSpecial ( *pMatch, m_dDataPtrAttrs );

		Swap ( pMatch->m_pDynamic, tResult.m_pDynamic );
		pMatch->m_pStatic = nullptr;
	}
};


void ISphMatchSorter::TransformPooled2StandalonePtrs ( fnGetBlobPoolFromMatch fnBlobPoolFromMatch )
{
	auto * pOldSchema = GetSchema ();
	assert ( pOldSchema );

	// create new standalone schema (from old, or from filtered)
	auto * pNewSchema = new CSphSchema ( "standalone" );
	for ( int i = 0; i<pOldSchema->GetFieldsCount (); ++i )
		pNewSchema->AddField ( pOldSchema->GetField(i) );

	if ( m_dTransormed.IsEmpty() )
	{
		// no need to filter schema attributes for query with star \ select all items
		for ( int i = 0; i<pOldSchema->GetAttrsCount (); ++i )
		{
			CSphColumnInfo tAttr = pOldSchema->GetAttr ( i );
			tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );
			tAttr.m_tLocator.m_iBitOffset = -1;
			tAttr.m_tLocator.m_iBitCount = -1;
			tAttr.m_tLocator.m_iBlobAttrId = -1;
			tAttr.m_tLocator.m_nBlobAttrs = 0;
			pNewSchema->AddAttr ( tAttr, true );
		}
	} else
	{
		for ( const CSphString & sName : m_dTransormed )
		{
			const CSphColumnInfo * pAttr = pOldSchema->GetAttr ( sName.cstr() );
			if ( !pAttr )
				continue;

			CSphColumnInfo tAttr = *pAttr;
			tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );
			tAttr.m_tLocator.m_iBitOffset = -1;
			tAttr.m_tLocator.m_iBitCount = -1;
			tAttr.m_tLocator.m_iBlobAttrId = -1;
			tAttr.m_tLocator.m_nBlobAttrs = 0;
			pNewSchema->AddAttr ( tAttr, true );
		}
	}

	for ( int i = 0; i <pNewSchema->GetAttrsCount(); ++i )
	{
		auto & pExpr = pNewSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	MatchesToNewSchema_c fnFinal ( pOldSchema, pNewSchema, std::move ( fnBlobPoolFromMatch ) );
	Finalize ( fnFinal, false );
	SetSchema ( pNewSchema, true );
}


//////////////////////////////////////////////////////////////////////////

/// groupby key type
typedef int64_t				SphGroupKey_t;

class BlobPool_c
{
	const BYTE * m_pBlobPool {nullptr};

public:
	virtual void SetBlobPool ( const BYTE * pBlobPool ) { m_pBlobPool = pBlobPool; }
	const BYTE * GetBlobPool () const { return m_pBlobPool; }
};

/// base grouper (class that computes groupby key)
class CSphGrouper : public BlobPool_c, public ISphRefcountedMT
{
protected:
	virtual					~CSphGrouper () {}; // =default causes bunch of errors building on wheezy
public:
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual ESphAttr		GetResultType () const = 0;
};


/// match-sorting priority queue traits
class CSphMatchQueueTraits : public ISphMatchSorter, ISphNoncopyable
{
protected:
	int							m_iSize;	// size of internal struct we can operate
	CSphFixedVector<CSphMatch>	m_dData;
	CSphTightVector<int>		m_dIData;	// indexes into m_pData, to avoid extra moving of matches themselves
	bool						m_bStorageFilled = false; // false on initial run; true once free chain is not solid

public:
	/// ctor
	explicit CSphMatchQueueTraits ( int iSize )
		: m_iSize ( iSize )
		, m_dData { iSize }
	{
		assert ( iSize>0 );
		m_iMatchCapacity = iSize;
		m_dIData.Reserve ( iSize );
		m_tState.m_iNow = (DWORD) time ( nullptr );
	}

	/// dtor make FreeDataPtrs here, then ResetDynamic also get called on m_dData d-tr.
	~CSphMatchQueueTraits () override
	{
		if ( m_pSchema )
			m_dData.Apply ( [this] ( CSphMatch& tMatch ) { m_pSchema->FreeDataPtrs ( tMatch ); } );
	}

public:
	int			GetLength () const override								{ return Used(); }
//	int			GetDataLength () const override							{ return m_iMatchCapacity; }

	// helper
	void SwapMatchQueueTraits ( CSphMatchQueueTraits& rhs )
	{
		// ISphMatchSorter
		::Swap ( m_iTotal, rhs.m_iTotal );

		// CSphMatchQueueTraits
		m_dData.SwapData ( rhs.m_dData );
		m_dIData.SwapData ( rhs.m_dIData );
		::Swap ( m_bStorageFilled, rhs.m_bStorageFilled );
		assert ( m_iSize==rhs.m_iSize );
	}

	const VecTraits_T<CSphMatch>& GetMatches() const { return m_dData; }

protected:
	CSphMatch * Last () const
	{
		return &m_dData[m_dIData.Last ()];
	}

	CSphMatch & Get ( int iElem ) const
	{
		return m_dData[m_dIData[iElem]];
	}

	CSphMatch & Add ()
	{
		auto iLast = m_dIData.GetLength ();
		m_dIData.Add ();
		if ( !m_bStorageFilled )
			m_dIData[iLast] = iLast;
		return m_dData[m_dIData.Last ()];
	}

	int Used() const
	{
		return m_dIData.GetLength();
	}

	bool IsEmpty() const
	{
		return m_dIData.IsEmpty();
	}

	void ResetAfterFlatten()
	{
		m_dIData.Resize(0);
		m_bStorageFilled = false;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORTING QUEUES
//////////////////////////////////////////////////////////////////////////

template < typename COMP >
struct InvCompareIndex_fn
{
	const VecTraits_T<CSphMatch>& m_dBase;
	const CSphMatchComparatorState & m_tState;

	explicit InvCompareIndex_fn ( const CSphMatchQueueTraits & dBase )
		: m_dBase ( dBase.GetMatches () )
		, m_tState ( dBase.GetComparatorState() )
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
	InvCompareIndex_fn<COMP> m_dComp;
	LOC_ADD;
public:
	/// ctor
	explicit CSphMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize )
		, m_dComp ( *this )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);
	}

	/// check if this sorter does groupby
	bool IsGroupby () const final
	{
		return false;
	}

	const CSphMatch * GetWorst() const final
	{
		return m_dIData.IsEmpty () ? nullptr : Root ();
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		KMQ << "push " << tEntry.m_iTag << ":" << tEntry.m_tRowID;
		return PushT ( tEntry, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch )
		{
			m_pSchema->CloneMatch ( tTrg, tMatch );
		});
	}

	/// add grouped entry (must not happen)
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( false );
		return false;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) final
	{
		KMQ << "flatten";
		assert ( !IsEmpty() );
		int iReadyMatches = Used();
		pTo += iReadyMatches;
		while ( !IsEmpty() )
		{
			--pTo;
//			m_pSchema->FreeDataPtrs ( *pTo );
			PopAndProcess_T ( [pTo] ( CSphMatch & tRoot ) { Swap ( *pTo, tRoot ); return true; } );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
		}
		m_iTotal = 0;
		return iReadyMatches;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( ISphMatchProcessor & tProcessor, bool bCallProcessInResultSetOrder ) final
	{
		KMQ << "finalize";
		if ( !GetLength() )
			return;

		if ( bCallProcessInResultSetOrder )
			m_dIData.Sort ( m_dComp );

		for ( auto iMatch : m_dIData )
			tProcessor.Process ( &m_dData[iMatch] );
	}

	// fixme! test
	ISphMatchSorter * Clone () const final
	{
		auto pClone = new MYTYPE ( m_iSize );
		CloneTo ( pClone );
		return pClone;
	}

	// FIXME! test CSphMatchQueue
	void MoveTo ( ISphMatchSorter * pRhs ) final
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
		for ( auto iMatch : m_dIData )
			dRhs.PushT ( m_dData[iMatch],
				[] ( CSphMatch & tTrg, CSphMatch & tMatch ) {
					Swap ( tTrg, tMatch );
				});
		dRhs.m_iTotal = m_iTotal + iTotal;
	}

private:
	CSphMatch* Root() const
	{
		return &m_dData [ m_dIData.First() ];
	}

	/// generic add entry to the queue
	template <typename MATCH, typename PUSHER>
	bool PushT ( MATCH && tEntry, PUSHER && PUSH )
	{
		++m_iTotal;

		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
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

		if_const ( NOTIFICATIONS )
			m_iJustPushed = Last()->m_tRowID;

		int iEntry = Used()-1;

		// shift up if needed, so that worst (lesser) ones float to the top
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) / 2;
			if ( !m_dComp.IsLess ( m_dIData[iParent], m_dIData[iEntry] ) )
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
		m_bStorageFilled = true;
		assert ( !IsEmpty() );

		auto& iJustRemoved = m_dIData.Pop();
		if ( !IsEmpty() ) // for empty just popped is the root
			Swap ( m_dIData.First (), iJustRemoved );

		if ( !fnProcess ( m_dData[iJustRemoved] ) )
		{
			// make the last entry my new root
			if_const ( NOTIFICATIONS )
			{
				if ( m_dJustPopped.IsEmpty () )
					m_dJustPopped.Add (  m_dData[iJustRemoved] .m_tRowID );
				else
					m_dJustPopped[0] =  m_dData[iJustRemoved] .m_tRowID;
			}

			m_pSchema->FreeDataPtrs ( m_dData[iJustRemoved] );
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
				if ( m_dComp.IsLess ( m_dIData[iChild], m_dIData[iChild+1] ) )
					++iChild;

			// if smallest child is less than entry, do float it to the top
			if ( m_dComp.IsLess ( m_dIData[iEntry], m_dIData[iChild] ) )
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
class CSphKbufferMatchQueue final : public CSphMatchQueueTraits
{
	using MYTYPE = CSphKbufferMatchQueue<COMP, NOTIFICATIONS>;
	InvCompareIndex_fn<COMP> m_dComp;

	CSphMatch *			m_pWorst = nullptr;
	bool				m_bFinalized = false;
	int					m_iMaxUsed = -1;

	static const int COEFF = 4;
	LOC_ADD;
public:
	/// ctor
	explicit CSphKbufferMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize*COEFF )
		, m_dComp ( *this )
	{
		m_iSize /= COEFF;
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize*(COEFF-1) );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const final
	{
		return false;
	}

	/// current result set length
	int GetLength () const final
	{
		return Min ( Used(), m_iSize );
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		return PushT ( tEntry, [this] ( CSphMatch & tTrg, const CSphMatch & tMatch )
		{
			m_pSchema->CloneMatch ( tTrg, tMatch );
		});
	}

	/// add grouped entry (must not happen)
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( false );
		return false;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) final
	{
		KBF << "Flatten";
		FinalizeMatches ();
		auto iReadyMatches = Used();

		for ( auto iMatch : m_dIData )
		{
			KBF << "fltn " << m_dData[iMatch].m_iTag << ":" << m_dData[iMatch].m_tRowID;
			Swap ( *pTo, m_dData[iMatch] );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			++pTo;
		}

		for ( int i = 0; i<m_iMaxUsed; ++i )
			m_dData[i].ResetDynamic ();
		m_iMaxUsed = -1;

		// clean up for the next work session
		m_pWorst = nullptr;
		m_iTotal = 0;
		m_bFinalized = false;
		m_dIData.Resize(0);
		m_bStorageFilled = false;

		return iReadyMatches;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( ISphMatchProcessor & tProcessor, bool ) final
	{
		KBF << "Finalize";
		if ( IsEmpty() )
			return;

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
	void MoveTo ( ISphMatchSorter * pRhs ) final
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

private:
	void SortMatches () // sort from best to worst
	{
		m_dIData.Sort ( m_dComp );
	}

	void FreeMatch ( int iMatch )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Add ( m_dData[iMatch].m_tRowID );
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
	// We need only N best elements from it (rest have to be disposed).
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
				b = j;  // too many elems aquired; continue with left part
			else
				a = i;  // too less elems aquired; continue with right part
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

	// generic push entry (add it some way to the queue clone or swap, PUSHER depends)
	template<typename MATCH, typename PUSHER>
	bool PushT ( MATCH && tEntry, PUSHER && PUSH )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
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

		if_const ( NOTIFICATIONS )
			m_iJustPushed = Last()->m_tRowID;

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

		// that will switch Add() to reuse the indexes
		m_bStorageFilled = true;

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

/// collector for UPDATE statement
class CSphUpdateQueue final : public CSphMatchQueueTraits
{
	CSphAttrUpdate		m_tWorkSet;
	CSphIndex *			m_pIndex;
	CSphString *		m_pError;
	CSphString *		m_pWarning;
	int *				m_pAffected;

public:
	/// ctor
	CSphUpdateQueue ( int iSize, CSphAttrUpdateEx * pUpdate, bool bIgnoreNonexistent, bool bStrict )
		: CSphMatchQueueTraits ( iSize )
	{
		m_tWorkSet.m_dRowOffset.Reserve ( m_iSize );
		m_tWorkSet.m_dDocids.Reserve ( m_iSize );

		m_tWorkSet.m_bIgnoreNonexistent = bIgnoreNonexistent;
		m_tWorkSet.m_bStrict = bStrict;
		m_tWorkSet.m_dAttributes = pUpdate->m_pUpdate->m_dAttributes;
		m_tWorkSet.m_dPool = pUpdate->m_pUpdate->m_dPool;
		m_tWorkSet.m_dBlobs = pUpdate->m_pUpdate->m_dBlobs;
		m_pIndex = pUpdate->m_pIndex;
		m_pError = pUpdate->m_pError;
		m_pWarning = pUpdate->m_pWarning;
		m_pAffected = &pUpdate->m_iAffected;
	}

	/// stub
	bool IsGroupby () const final
	{
		return false;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		++m_iTotal;

		if ( Used()==m_iSize )
			DoUpdate();

		// do add
		m_pSchema->CloneMatch ( Add(), tEntry );
		return true;
	}

	/// stub
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( 0 );
		return false;
	}

	/// final update pass
	int Flatten ( CSphMatch *, int ) final
	{
		DoUpdate();
		m_iTotal = 0;
		return 0;
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) final
	{
		if ( IsEmpty() )
			return;

		// just evaluate in heap order
		for ( auto iMatch : m_dIData )
			tProcessor.Process ( &m_dData[iMatch] );
	}

	bool CanBeCloned () const final { return false; }

	// stub
	ISphMatchSorter * Clone () const final
	{
		return nullptr;
	}

	// stub
	void MoveTo ( ISphMatchSorter* ) final {}

private:
	void DoUpdate()
	{
		if ( IsEmpty() )
			return;

		m_tWorkSet.m_dRowOffset.Resize ( Used() );
		m_tWorkSet.m_dDocids.Resize ( Used() );

		ARRAY_FOREACH ( i, m_tWorkSet.m_dDocids )
		{
			auto& tMatch = Get(i);
			m_tWorkSet.m_dRowOffset[i] = 0;
			m_tWorkSet.m_dDocids[i] = sphGetDocID ( tMatch.m_pStatic ? tMatch.m_pStatic : tMatch.m_pDynamic );
		}

		bool bCritical = false;
		*m_pAffected += m_pIndex->UpdateAttributes ( m_tWorkSet, -1, bCritical, *m_pError, *m_pWarning );
		assert ( !bCritical ); // fixme! handle this

		// fixme! Do we need release/delete cloned matches anyway here? Check with valgrind!
		m_dIData.Resize(0);
	}
};

//////////////////////////////////////////////////////////////////////////

/// collect list of matched DOCIDs
/// (mainly used to collect docs in `DELETE... WHERE` statement)
class CSphCollectQueue final : public CSphMatchQueueTraits
{
public:
	/// ctor
	CSphCollectQueue ( int iSize, CSphVector<DocID_t> * pValues )
		: CSphMatchQueueTraits ( 1 )
		, m_pValues ( pValues )
	{
		m_pValues->Reserve ( iSize );
	}

	/// stub
	bool IsGroupby () const final
	{
		return false;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		++m_iTotal;
		m_pValues->Add ( sphGetDocID( tEntry.m_pStatic ) );
		return true;
	}

	/// stub
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( 0 );
		return false;
	}

	/// stub
	int Flatten ( CSphMatch *, int ) final
	{
		m_iTotal = 0;
		return 0;
	}

	// stub
	void Finalize ( ISphMatchProcessor &, bool ) final
	{}

	bool CanBeCloned () const final { return false; }

	// stub
	ISphMatchSorter * Clone () const final
	{
		return nullptr;
	}

	// stub
	void MoveTo ( ISphMatchSorter* ) final {}

private:
	CSphVector<DocID_t>* m_pValues;
};

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING QUEUE
//////////////////////////////////////////////////////////////////////////

static bool IsCount ( const CSphString & s )
{
	return s=="@count" || s=="count(*)";
}

static bool IsGroupby ( const CSphString & s )
{
	return s=="@groupby"
		|| s=="@distinct"
		|| s=="groupby()"
		|| IsSortJsonInternal(s);
}

static bool IsGroupbyMagic ( const CSphString & s )
{
	return IsGroupby ( s ) || IsCount ( s );
}

/// groupers
#define GROUPER_BEGIN(_name) \
	class _name : public CSphGrouper \
	{ \
	protected: \
		CSphAttrLocator m_tLocator; \
	public: \
		explicit _name ( const CSphAttrLocator & tLoc ) : m_tLocator ( tLoc ) {} \
		virtual void GetLocator ( CSphAttrLocator & tOut ) const { tOut = m_tLocator; } \
		virtual ESphAttr GetResultType () const { return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; } \
		virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const { return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); } \
		virtual SphGroupKey_t KeyFromValue ( SphAttr_t uValue ) const \
		{
// NOLINT

#define GROUPER_END \
		} \
	};

#define GROUPER_BEGIN_SPLIT(_name) \
	GROUPER_BEGIN(_name) \
	time_t tStamp = (time_t)uValue; \
	struct tm tSplit; \
	localtime_r ( &tStamp, &tSplit );

GROUPER_BEGIN ( CSphGrouperAttr )
	return uValue;
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperDay )
	return (tSplit.tm_year+1900)*10000 + (1+tSplit.tm_mon)*100 + tSplit.tm_mday;
GROUPER_END

GROUPER_BEGIN_SPLIT ( CSphGrouperWeek )
	int iPrevSunday = (1+tSplit.tm_yday) - tSplit.tm_wday; // prev Sunday day of year, base 1
	int iYear = tSplit.tm_year+1900;
	if ( iPrevSunday<=0 ) // check if we crossed year boundary
	{
		// adjust day and year
		iPrevSunday += 365;
		iYear--;

		// adjust for leap years
		if ( iYear%4==0 && ( iYear%100!=0 || iYear%400==0 ) )
			iPrevSunday++;
	}
	return iYear*1000 + iPrevSunday;
GROUPER_END

GROUPER_BEGIN_SPLIT ( CSphGrouperMonth )
	return (tSplit.tm_year+1900)*100 + (1+tSplit.tm_mon);
GROUPER_END

GROUPER_BEGIN_SPLIT ( CSphGrouperYear )
	return (tSplit.tm_year+1900);
GROUPER_END

#define GROUPER_BEGIN_SPLIT_UTC( _name ) \
    GROUPER_BEGIN(_name) \
    time_t tStamp = (time_t)uValue; \
    struct tm tSplit; \
    gmtime_r ( &tStamp, &tSplit );

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperDayUtc )
		return (tSplit.tm_year + 1900) * 10000 + (1 + tSplit.tm_mon) * 100 + tSplit.tm_mday;
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperWeekUtc )
		int iPrevSunday = (1 + tSplit.tm_yday) - tSplit.tm_wday; // prev Sunday day of year, base 1
		int iYear = tSplit.tm_year + 1900;
		if ( iPrevSunday<=0 ) // check if we crossed year boundary
		{
			// adjust day and year
			iPrevSunday += 365;
			iYear--;

			// adjust for leap years
			if ( iYear % 4==0 && (iYear % 100!=0 || iYear % 400==0) )
				iPrevSunday++;
		}
		return iYear * 1000 + iPrevSunday;
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperMonthUtc )
		return (tSplit.tm_year + 1900) * 100 + (1 + tSplit.tm_mon);
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperYearUtc )
		return (tSplit.tm_year + 1900);
GROUPER_END

bool bGroupingInUtc = false;

CSphGrouper * getDayGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
			? (CSphGrouper *) new CSphGrouperDayUtc ( tLoc )
			: (CSphGrouper *) new CSphGrouperDay ( tLoc );
}

CSphGrouper * getWeekGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperWeekUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperWeek ( tLoc );
}

CSphGrouper * getMonthGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperMonthUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperMonth ( tLoc );
}

CSphGrouper * getYearGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperYearUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperYear ( tLoc );
}

template <class PRED>
class CSphGrouperString final : public CSphGrouperAttr, public PRED
{
public:
	explicit CSphGrouperString ( const CSphAttrLocator & tLoc )
		: CSphGrouperAttr ( tLoc )
	{}

	ESphAttr GetResultType () const override
	{
		return SPH_ATTR_BIGINT;
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const override
	{
		auto dBlobAttr = tMatch.FetchAttrData ( m_tLocator, GetBlobPool() );
		if ( IsNull ( dBlobAttr ) )
			return 0;

		return PRED::Hash ( dBlobAttr.first,dBlobAttr.second );
	}
};


class BinaryHash_fn
{
public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );
		return sphFNV64 ( pStr, iLen, uPrev );
	}
};


template < typename T >
inline static char * FormatInt ( char sBuf[32], T v )
{
	if_const ( sizeof(T)==4 && v==INT_MIN )
		return strncpy ( sBuf, "-2147483648", 32 );
	if_const ( sizeof(T)==8 && v==LLONG_MIN )
		return strncpy ( sBuf, "-9223372036854775808", 32 );

	bool s = ( v<0 );
	if ( s )
		v = -v;

	char * p = sBuf+31;
	*p = 0;
	do
	{
		*--p = '0' + char ( v % 10 );
		v /= 10;
	} while ( v );
	if ( s )
		*--p = '-';
	return p;
}


/// lookup JSON key, group by looked up value (used in CSphKBufferJsonGroupSorter)
class CSphGrouperJsonField final : public CSphGrouper
{
public:
	CSphGrouperJsonField ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
		: m_tLocator ( tLoc )
		, m_pExpr ( pExpr )
	{
		SafeAddRef ( pExpr );
	}

	void SetBlobPool ( const BYTE * pBlobPool ) final
	{
		CSphGrouper::SetBlobPool ( pBlobPool );
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	}

	void GetLocator ( CSphAttrLocator & tOut ) const final
	{
		tOut = m_tLocator;
	}

	ESphAttr GetResultType () const final
	{
		return SPH_ATTR_BIGINT;
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const final
	{
		if ( !m_pExpr )
			return SphGroupKey_t();
		return m_pExpr->Int64Eval ( tMatch );
	}

	SphGroupKey_t KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }

protected:
	CSphAttrLocator				m_tLocator;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
};


template <class PRED>
class CSphGrouperMulti final: public CSphGrouper, public PRED
{
public:
	CSphGrouperMulti ( CSphVector<CSphAttrLocator> dLocators, CSphVector<ESphAttr> dAttrTypes, VecRefPtrs_t<ISphExpr *> dJsonKeys)
		: m_dLocators ( std::move(dLocators) )
		, m_dAttrTypes ( std::move(dAttrTypes) )
		, m_dJsonKeys ( std::move(dJsonKeys) )
	{
		assert ( m_dLocators.GetLength()>1 );
		assert ( m_dLocators.GetLength()==m_dAttrTypes.GetLength() && m_dLocators.GetLength()==m_dJsonKeys.GetLength() );
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const final
	{
		auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;

		for ( int i=0; i<m_dLocators.GetLength(); i++ )
		{
			if ( m_dAttrTypes[i]==SPH_ATTR_STRING )
			{
				int iLen = 0;
				const BYTE * pStr = sphGetBlobAttr( tMatch, m_dLocators[i], GetBlobPool(), iLen );
				if ( !pStr || !iLen )
					continue;

				tKey = PRED::Hash ( pStr, iLen, tKey );

			} else if ( m_dAttrTypes[i]==SPH_ATTR_JSON )
			{
				int iLen = 0;
				const BYTE * pStr = sphGetBlobAttr( tMatch, m_dLocators[i], GetBlobPool(), iLen );
				if ( !pStr || !iLen )
					continue;

				uint64_t uPacked = m_dJsonKeys[i]->Int64Eval ( tMatch );

				ESphJsonType eType = sphJsonUnpackType ( uPacked );
				const BYTE * pValue = GetBlobPool () + sphJsonUnpackOffset ( uPacked );

				int i32Val;
				int64_t i64Val;
				double fVal;
				switch ( eType )
				{
				case JSON_STRING:
					iLen = sphJsonUnpackInt ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( pValue, iLen, tKey );
					break;
				case JSON_INT32:
					i32Val = sphJsonLoadInt ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &i32Val, sizeof(i32Val), tKey );
					break;
				case JSON_INT64:
					i64Val = sphJsonLoadBigint ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &i64Val, sizeof(i64Val), tKey );
					break;
				case JSON_DOUBLE:
					fVal = sphQW2D ( sphJsonLoadBigint ( &pValue ) );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &fVal, sizeof(fVal), tKey );
					break;
				default:
					break;
				}

			} else
			{
				SphAttr_t tAttr = tMatch.GetAttr ( m_dLocators[i] );
				tKey = ( SphGroupKey_t ) sphFNV64 ( &tAttr, sizeof(SphAttr_t), tKey );
			}
		}

		return tKey;
	}

	void SetBlobPool ( const BYTE * pBlobPool ) final
	{
		CSphGrouper::SetBlobPool ( pBlobPool );
		ARRAY_FOREACH ( i, m_dJsonKeys )
		{
			if ( m_dJsonKeys[i] )
				m_dJsonKeys[i]->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
		}
	}

	SphGroupKey_t KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void GetLocator ( CSphAttrLocator & ) const final { assert(0); }
	ESphAttr GetResultType() const final { return SPH_ATTR_BIGINT; }

private:
	CSphVector<CSphAttrLocator>	m_dLocators;
	CSphVector<ESphAttr>		m_dAttrTypes;
	VecRefPtrs_t<ISphExpr *>	m_dJsonKeys;
};

/////////////////////////////////////////////////////////////////////////////
/// (attrvalue,count) pair
struct SphUngroupedValue_t : public std::pair<SphAttr_t,int>
{
	using BASE = std::pair<SphAttr_t,int>;

	FWD_BASECTOR (SphUngroupedValue_t)

	inline bool operator < ( const SphUngroupedValue_t & rhs ) const
	{
		if ( first!=rhs.first ) return first<rhs.first;
		return second>rhs.second;
	}

	inline bool operator == ( const SphUngroupedValue_t & rhs ) const
	{
		return first==rhs.first;
	}
};

/// (group,attrvalue,count) tuplet
struct SphGroupedValue_t
{
public:
	SphGroupKey_t	m_uGroup;
	SphAttr_t		m_uValue;
	int				m_iCount;

public:
	SphGroupedValue_t () = default;

	SphGroupedValue_t ( SphGroupKey_t uGroup, SphAttr_t uValue, int iCount )
		: m_uGroup ( uGroup )
		, m_uValue ( uValue )
		, m_iCount ( iCount )
	{}

	inline bool operator < ( const SphGroupedValue_t & rhs ) const
	{
		if ( m_uGroup!=rhs.m_uGroup ) return m_uGroup<rhs.m_uGroup;
		if ( m_uValue!=rhs.m_uValue ) return m_uValue<rhs.m_uValue;
		return m_iCount>rhs.m_iCount;
	}

	inline bool operator == ( const SphGroupedValue_t & rhs ) const
	{
		return m_uGroup==rhs.m_uGroup && m_uValue==rhs.m_uValue;
	}
};


/// unique values counter
/// used for COUNT(DISTINCT xxx) GROUP BY yyy queries
class CSphUniqounter : public CSphVector<SphGroupedValue_t>
{
public:
#ifndef NDEBUG
					CSphUniqounter () { Reserve ( 16384 ); }
	void			Add ( const SphGroupedValue_t & tValue )	{ CSphVector<SphGroupedValue_t>::Add ( tValue ); m_bSorted = false; }
	void			Sort ()										{ CSphVector<SphGroupedValue_t>::Sort (); m_bSorted = true; }
#endif

public:
	int				CountStart ( SphGroupKey_t * pOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t * pOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( VecTraits_T<SphGroupKey_t> & dRemoveGroups );

protected:
	int				m_iCountPos = 0;

#ifndef NDEBUG
	bool			m_bSorted = true;
#endif
};


int CSphUniqounter::CountStart ( SphGroupKey_t * pOutGroup )
{
	m_iCountPos = 0;
	return CountNext ( pOutGroup );
}


int CSphUniqounter::CountNext ( SphGroupKey_t * pOutGroup )
{
	assert ( m_bSorted );
	if ( m_iCountPos>=m_iCount )
		return 0;

	auto uGroup = m_pData[m_iCountPos].m_uGroup;
	auto uValue = m_pData[m_iCountPos].m_uValue;
	auto iCount = m_pData[m_iCountPos].m_iCount;
	*pOutGroup = uGroup;

	while ( m_iCountPos<m_iCount && m_pData[m_iCountPos].m_uGroup==uGroup )
	{
		if ( m_pData[m_iCountPos].m_uValue!=uValue )
			iCount += m_pData[m_iCountPos].m_iCount;
		uValue = m_pData[m_iCountPos].m_uValue;
		++m_iCountPos;
	}
	return iCount;
}

// works like 'uniq', and also throw away provided values to remove.
void CSphUniqounter::Compact ( VecTraits_T<SphGroupKey_t>& dRemoveGroups )
{
	assert ( m_bSorted );
	if ( !m_iCount )
		return;

	dRemoveGroups.Sort();
	auto * pRemoveGroups = dRemoveGroups.begin ();
	auto iRemoveGroups = dRemoveGroups.GetLength ();

	SphGroupedValue_t * pSrc = m_pData;
	SphGroupedValue_t * pDst = m_pData;
	SphGroupedValue_t * pEnd = m_pData + m_iCount;

	// skip remove-groups which are not in my list
	while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
	{
		++pRemoveGroups;
		--iRemoveGroups;
	}

	for ( ; pSrc<pEnd; ++pSrc )
	{
		// check if this entry needs to be removed
		while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
		{
			++pRemoveGroups;
			--iRemoveGroups;
		}
		if ( iRemoveGroups && pSrc->m_uGroup==*pRemoveGroups )
			continue;

		// check if it's a dupe
		if ( pDst>m_pData && pDst[-1]==pSrc[0] )
			continue;

		*pDst++ = *pSrc;
	}

	assert ( pDst-m_pData<=m_iCount );
	m_iCount = pDst-m_pData;
}

/////////////////////////////////////////////////////////////////////////////
/// group sorting functor
template < typename COMPGROUP >
struct GroupSorter_fn : public CSphMatchComparatorState, public MatchSortAccessor_t
{
	const VecTraits_T<CSphMatch> & m_dBase;

	explicit GroupSorter_fn ( const CSphMatchQueueTraits& dBase )
		: m_dBase ( dBase.GetMatches() )
	{}

	bool IsLess ( int a, int b ) const
	{
		return COMPGROUP::IsLess ( m_dBase[b], m_dBase[a], *this );
	}
};

/// match comparator interface from group-by sorter point of view
struct ISphMatchComparator : public ISphRefcountedMT
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState ) const = 0;
protected:
	            ~ISphMatchComparator () override = default;
};

/// additional group-by sorter settings
struct CSphGroupSorterSettings
{
	CSphAttrLocator		m_tLocGroupby;		///< locator for @groupby
	CSphAttrLocator		m_tLocCount;		///< locator for @count
	CSphAttrLocator		m_tLocDistinct;		///< locator for @distinct
	CSphAttrLocator		m_tDistinctAttr;	///< locator for attribute to compute count(distinct) for
	CSphAttrLocator		m_tLocGroupbyStr;	///< locator for @groupbystr

	ESphAttr			m_eDistinctAttr = SPH_ATTR_NONE;	///< type of attribute to compute count(distinct) for
	bool				m_bDistinct = false;///< whether we need distinct
	bool				m_bMVA = false;		///< whether we're grouping by MVA attribute
	bool				m_bMva64 = false;
	CSphRefcountedPtr<CSphGrouper>		m_pGrouper;///< group key calculator
	bool				m_bImplicit = false;///< for queries with aggregate functions but without group by clause
	SharedPtr_t<ISphFilter *>	m_pAggrFilterTrait; ///< aggregate filter that got owned by grouper
	bool				m_bJson = false;	///< whether we're grouping by Json attribute
	int					m_iMaxMatches = 0;

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
	{
		sphFixupLocator ( m_tLocGroupby, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocCount, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocDistinct, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tDistinctAttr, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocGroupbyStr, pOldSchema, pNewSchema );
	}
};


/// aggregate function interface
class IAggrFunc
{
public:
	virtual			~IAggrFunc() {};
	virtual void	Ungroup ( CSphMatch & ) {}
	virtual void	Update ( CSphMatch &, const CSphMatch &, bool bGrouped ) = 0;
	virtual void	Finalize ( CSphMatch & ) {}
};


/// aggregate traits for different attribute types
template < typename T >
class IAggrFuncTraits : public IAggrFunc
{
public:
	explicit		IAggrFuncTraits ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}
	T				GetValue ( const CSphMatch & tRow );
	void			SetValue ( CSphMatch & tRow, T val );

protected:
	CSphAttrLocator	m_tLocator;
};

template<>
inline DWORD IAggrFuncTraits<DWORD>::GetValue ( const CSphMatch & tRow )
{
	return (DWORD) tRow.GetAttr ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<DWORD>::SetValue ( CSphMatch & tRow, DWORD val )
{
	tRow.SetAttr ( m_tLocator, val );
}

template<>
inline int64_t IAggrFuncTraits<int64_t>::GetValue ( const CSphMatch & tRow )
{
	return tRow.GetAttr ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<int64_t>::SetValue ( CSphMatch & tRow, int64_t val )
{
	tRow.SetAttr ( m_tLocator, val );
}

template<>
inline float IAggrFuncTraits<float>::GetValue ( const CSphMatch & tRow )
{
	return tRow.GetAttrFloat ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<float>::SetValue ( CSphMatch & tRow, float val )
{
	tRow.SetAttrFloat ( m_tLocator, val );
}



/// SUM() implementation
template < typename T >
class AggrSum_t final : public IAggrFuncTraits<T>
{
public:
	explicit AggrSum_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool ) final
	{
		this->SetValue ( tDst, this->GetValue(tDst)+this->GetValue(tSrc) );
	}
};


/// AVG() implementation
template < typename T >
class AggrAvg_t final : public IAggrFuncTraits<T>
{
	CSphAttrLocator m_tCountLoc;
	using IAggrFuncTraits<T>::GetValue;
	using IAggrFuncTraits<T>::SetValue;

public:
	AggrAvg_t ( const CSphAttrLocator & tLoc, const CSphAttrLocator & tCountLoc )
		: IAggrFuncTraits<T> ( tLoc ), m_tCountLoc ( tCountLoc )
	{}

	void Ungroup ( CSphMatch & tDst ) final
	{
		SetValue ( tDst, T ( GetValue ( tDst ) * tDst.GetAttr ( m_tCountLoc ) ) );
	}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped ) final
	{
		if ( bGrouped )
			SetValue ( tDst, T ( GetValue ( tDst ) + GetValue ( tSrc ) * tSrc.GetAttr ( m_tCountLoc ) ) );
		else
			SetValue ( tDst, GetValue ( tDst ) + GetValue ( tSrc ) );
	}

	void Finalize ( CSphMatch & tDst ) final
	{
		auto uAttr = tDst.GetAttr ( m_tCountLoc );
		if ( uAttr )
			SetValue ( tDst, T ( GetValue ( tDst ) / uAttr ) );
	}
};


/// MAX() implementation
template < typename T >
class AggrMax_t final : public IAggrFuncTraits<T>
{
public:
	explicit AggrMax_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool ) final
	{
		this->SetValue ( tDst, Max ( this->GetValue(tDst), this->GetValue(tSrc) ) );
	}
};


/// MIN() implementation
template < typename T >
class AggrMin_t final : public IAggrFuncTraits<T>
{
public:
	explicit AggrMin_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool ) final
	{
		this->SetValue ( tDst, Min ( this->GetValue(tDst), this->GetValue(tSrc) ) );
	}
};

/// GROUP_CONCAT

/* What is that magic about?
 *
 * In simplest usecase - you have in group matches with 'foo', 'bar' -> group_concat produces 'foo,bar' - no magic.
 *
 * Make a bit complex: one and same sorter processes sequentaly several indexes (chunks), and collects group result.
 * In this case if 1-st chunk gives 'foo', 2-nd gives 'bar', you still can achieve 'foo,bar' naturally, no magic.
 *
 * A bit more complex: several sorters process cloud of chunks in parallel, then merge results.
 * Say, you have 3 chunks, giving 'foo', 'bar' and 'bazz'. Result you've expect is 'foo,bar,bazz'.
 * In parallel with, say, 2 sorters, one processed 1-st and 3-rd chunk, second - middle.
 * One gives you 'foo,bazz', second - 'bar'.
 *
 * What is to do on merge then?
 *
 * Simplest: just merge 'as is'. I.e., return 'foo,bazz,bar' despite the broken order.
 * However that is appropriate only in the narrow case when ordering is not requested. That is *NOT* our way.
 *
 * Each match came from a chunk is tagged with the order num of that chunk.
 * When we have matches from the same chunk, we just group them usual way with no magic, naturally concatenating strings.
 * If all the matches tagged same - we just achieve usual string out of the box, with no magic at all.
 * If into non-empty group came match with another tag, we use this model of blob:
 *
 * '\0', <N> <TAG1> <STRLEN1> chars1 <TAG2> <STRLEN2> chars2 ... <TAGN> <STRLEN> bytesN
 *
 * First \0 marks that the whole blob is special.
 * Each tagged string inside includes concatenated values of the matches from that tag.
 * For described foo-bar-baz in two sorters it will look like:
 *
 * '\0' 2 1 3 foo 3 4 bazz // in the 1-st sorter. 2 chunks, from tag 1 with len 3 'foo', from tag 3 with len 4 'bazz'
 * bar // in the 2-nd sorter. Simple plain 'bar' (tag is not saved here, it is still an attribute of the match itself).
 *
 * Then we can merge results, taking tag for value came from 2-nd sorter from that match itself.
 *
 * '\0' 3 1 3 foo 2 3 bar 3 4 bazz
 *
 * That finally deserializes into expected user string 'foo,bar,bazz'. So, target achieved.
 *
 * One optimization here is that we expect matches with monotonically changing tags. I.e., if we have processed chunk 1,
 * and stay on chunk 3 - then next match will never came with tag 1 or 2, as these numbers already passed.
 * So, no need to 'insert into middle', we always pushes new data to tail of the blob. That makes everything simpler.
 *
 */

// helpers to blob serialization
using BStream_c = TightPackedVec_T<BYTE>;

BStream_c & operator<< ( BStream_c & dOut, const ByteBlob_t & tData )
{
	dOut.Append ( tData.first, tData.second );
	return dOut;
}

template <typename NUM>
BStream_c & operator<< ( BStream_c & dOut, NUM iNum )
{
	sphUnalignedWrite ( dOut.AddN ( sizeof ( NUM ) ), iNum );
	return dOut;
}

template<typename T>
BStream_c & operator<< ( BStream_c & dOut, const VecTraits_T<T> & tData )
{
	dOut << tData.GetLength ();
	tData.Apply ( [&dOut] ( const T & tChunk ) { dOut << tChunk; } );
	return dOut;
}

BStream_c & operator<< ( BStream_c & dOut, const VecTraits_T<BYTE> & tData )
{
	return dOut << tData.GetLength () << ByteBlob_t { tData.begin(), tData.GetLength () };
}

BStream_c & operator<< ( BStream_c & dOut, const CSphString& sData )
{
	return dOut << VecTraits_T<BYTE> ( (BYTE*) sData.cstr(), sData.Length() );
}

BStream_c & operator<< ( BStream_c & dOut, const StringBuilder_c & sData )
{
	return dOut << VecTraits_T<BYTE> ( (BYTE*) sData.cstr (), sData.GetLength () );
}

// helpers to de-serialize
template<typename NUM>
ByteBlob_t & operator>> ( ByteBlob_t & dIn, NUM & iNum )
{
	assert ( dIn.first );
	assert ( dIn.second>=(int)sizeof (NUM) );
	iNum = sphUnalignedRead ( *(NUM *) dIn.first );
	dIn.first += sizeof ( NUM );
	dIn.second -= sizeof ( NUM );
	return dIn;
}

ByteBlob_t & operator>> ( ByteBlob_t & dIn, ByteBlob_t & tData )
{
	assert ( dIn.first );
	assert ( dIn.second>=tData.second );
	tData.first = dIn.first;
	dIn.first += tData.second;
	dIn.second -= tData.second;
	return dIn;
}

template<typename T>
ByteBlob_t & operator>> ( ByteBlob_t & dIn, CSphVector<T>& tData )
{
	int iLen;
	dIn >> iLen;
	tData.Resize ( iLen );
	tData.Apply ( [&dIn] ( T & tChunk ) { dIn >> tChunk; } );
	return dIn;
}

ByteBlob_t & operator>> ( ByteBlob_t & dIn, VecTraits_T<BYTE> & tData )
{
	int iLen;
	dIn >> iLen;
	ByteBlob_t tChunk { nullptr, iLen };
	dIn >> tChunk;
	tData = tChunk;
	return dIn;
}

// The GROUP_CONCAT() implementation
class AggrConcat_t final : public IAggrFunc
{
	CSphAttrLocator	m_tLoc;
	using FixedVectorByte = CSphFixedVector<BYTE, sph::DefaultCopy_T<BYTE>, sph::CustomStorage_T<BYTE>>;

public:
	explicit AggrConcat_t ( const CSphColumnInfo & tCol )
		: m_tLoc ( tCol.m_tLocator )
	{
		assert ( tCol.m_eAttrType==SPH_ATTR_STRINGPTR );
		assert ( !m_tLoc.IsBlobAttr ()); // otherwise we will fail on fetching data!
	}

	// here we convert back to plain string
	void Finalize ( CSphMatch & tMatch ) final
	{
		auto dSrc = tMatch.FetchAttrData ( m_tLoc, nullptr ); // expect serialized tagged strings

		// empty match
		if ( !dSrc.first )
			return;

		// already grouped match
		if ( *dSrc.first )
			return;

		auto dBlob = dSrc;
		int iSize, iTag, iFinalSize;
		BStream_c dOut;
		VecTraits_T<BYTE> dString;

		BYTE uZero; dBlob >> uZero;
		dBlob >> iSize;
		iFinalSize = dBlob.second - ( iSize * 2 * sizeof ( int ) ) + iSize - 1 + 20; // -tag, -len, +commas-1, +packlen
		dOut.Reserve ( iFinalSize );

		for ( int i=0; i<iSize; ++i )
		{
			if ( i>0 ) dOut << ',';
			dBlob >> iTag >> dString;
			dOut << ByteBlob_t { dString.begin (), dString.GetLength () }; // write raw blob, without length
		}

		// release previous, write converted
		sphDeallocatePacked ( sphPackedBlob ( dSrc ) );
		sphPackPtrAttrInPlace ( dOut );
		tMatch.SetAttr ( m_tLoc, (SphAttr_t) dOut.LeakData () );
	}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool ) final
	{
		ByteBlob_t dSrc = tSrc.FetchAttrData ( m_tLoc, nullptr ); // ok since it is NOT a blob attr
		ByteBlob_t dDst = tDst.FetchAttrData ( m_tLoc, nullptr );

		// empty source? kinda weird, but done!
		if ( !dSrc.first || !dSrc.second )
			return;

		BStream_c dOut;
		if ( !dDst.first )
			dOut << dSrc;

		// first byte is a mark: 0 means data packed, another is part of real string.
		else if ( *dSrc.first && *dDst.first )
			AppendStringToString ( dOut, dDst, tDst.m_iTag, dSrc, tSrc.m_iTag );
		else if ( *dSrc.first && !*dDst.first )
			AppendBlobToString ( dOut, dSrc, tSrc.m_iTag, dDst, false );
		else if ( !*dSrc.first && *dDst.first )
			AppendBlobToString ( dOut, dDst, tDst.m_iTag, dSrc, true );
		else // if ( !*dSrc.first && !*dDst.first )
			AppendBlobToBlob ( dOut, dDst, dSrc );

		// Dispose previous packet
		sphDeallocatePacked ( sphPackedBlob ( dDst ) );

		// update saved data
		sphPackPtrAttrInPlace (dOut);
		tDst.SetAttr ( m_tLoc, (SphAttr_t) dOut.LeakData () );
	}

private:

	// merge two simple matches
	static void AppendStringToString ( BStream_c& dOut, const ByteBlob_t& dInDst, int iTagDst,
			const ByteBlob_t& dInSrc, int iTagSrc )
	{
		if ( iTagDst==iTagSrc ) // plain concat of 2 strings
			dOut << dInDst << ',' << dInSrc;
		else // produce complex match
			dOut << '\0' << int(2)
			<< iTagDst << dInDst.second << dInDst
			<< iTagSrc << dInSrc.second << dInSrc;
	}

	static void WriteCount ( BStream_c& dOut, int iCount )
	{
		// update total num of elements
		int iCurrentLen = dOut.GetLength ();
		dOut.Resize ( 1 ); // since 1-st came '\0' mark
		dOut << iCount;
		dOut.Resize ( iCurrentLen );
	}

	// merge two complex matches
	static void AppendBlobToBlob ( BStream_c& dOut, ByteBlob_t dInDst, ByteBlob_t dInSrc )
	{
		int iOut = 0;
		dOut << '\0' << iOut; // mark of complex and placeholder to num of elems.

		int iSizeSrc = 0, iSizeDst = 0, iTagSrc, iTagDst;
		VecTraits_T<BYTE> dBlobSrc, dBlobDst;

		// read num of elements in both matches
		char cZero;
		dInSrc >> cZero >> iSizeSrc;
		assert ( cZero=='\0' );
		dInDst >> cZero >> iSizeDst;
		assert ( cZero=='\0' );

		auto fnNextSrc = [&] { if (iSizeSrc<=0) iTagSrc = INT_MIN; else {dInSrc >> iTagSrc >> dBlobSrc; --iSizeSrc;} };
		auto fnNextDst = [&] { if (iSizeDst<=0) iTagDst = INT_MIN; else {dInDst >> iTagDst >> dBlobDst; --iSizeDst;} };

		// merge two matches
		fnNextSrc ();
		fnNextDst ();
		while ( iTagSrc!=INT_MIN || iTagDst!=INT_MIN )
		{
			if ( iTagSrc < iTagDst ) {
				dOut << iTagDst << dBlobDst;
				fnNextDst();
			} else if ( iTagDst < iTagSrc ) {
				dOut << iTagSrc << dBlobSrc;
				fnNextSrc();
			} else {
				assert ( iTagSrc!=INT_MAX || iTagDst!=INT_MAX );
				dOut << iTagSrc;
				if ( dBlobDst.IsEmpty() )
					dOut << dBlobSrc;
				else
					dOut << dBlobDst.GetLength() + dBlobSrc.GetLength() + 1
					<< ByteBlob_t ( dBlobDst.begin(), dBlobDst.GetLength() )
					<< ',' << ByteBlob_t ( dBlobSrc.begin(), dBlobSrc.GetLength() );
				fnNextSrc();
				fnNextDst();
			}
			++iOut;
		}

		WriteCount ( dOut, iOut );
	}

	// merge string and blob. Last bool determines what will came first
	static void AppendBlobToString ( BStream_c& dOut, const ByteBlob_t& dString, int iTagString,
			ByteBlob_t dBlob, bool bStringFirst=true )
	{
		int iOut;
		char cZero;
		dBlob >> cZero >> iOut;
		assert ( cZero=='\0' );
		dOut << cZero << iOut; // mark of complex and placeholder to num of elems.

		int iTagSrc;
		VecTraits_T<BYTE> dBlobSrc;
		bool bCopied = false;

		// copy elems looking for the place of new one
		for ( int i=0, iOldLen=iOut; i<iOldLen; ++i)
		{
			dBlob >> iTagSrc >> dBlobSrc;
			if ( bCopied )
				dOut << iTagSrc << dBlobSrc;
			else {
				if ( !bCopied && iTagString > iTagSrc ) {
					dOut << iTagString << dString.second << dString << iTagSrc << dBlobSrc;
					++iOut;
					bCopied = true;
				} else if ( !bCopied && iTagString==iTagSrc ) {
					dOut << iTagString << dString.second + dBlobSrc.GetLength() + 1;
					if ( bStringFirst )
						dOut << dString << ',' << ByteBlob_t ( dBlobSrc.begin (), dBlobSrc.GetLength () );
					else
						dOut << ByteBlob_t ( dBlobSrc.begin (), dBlobSrc.GetLength () ) << ',' << dString;
					bCopied = true;
				} else
					dOut << iTagSrc << dBlobSrc;
			}
		}
		if ( !bCopied )
		{
			dOut << iTagString << dString.second << dString;
			++iOut;
		}

		WriteCount ( dOut, iOut );
	}
};


struct MatchCloner_t
{
private:
	CSphFixedVector<CSphRowitem>	m_dRowBuf { 0 };
	CSphVector<CSphAttrLocator>		m_dAttrsGrp; // locators for grouping attrs (@groupby, @count, @distinct, etc.)
	CSphVector<CSphAttrLocator>		m_dAttrsPtr; // locators for group_concat attrs
	CSphVector<int>					m_dMyPtrRows; // rowids matching m_dAttrsPtr. i.e. grpconcat ptr result I own
	CSphVector<int>					m_dOtherPtrRows; // rest rowids NOT matching m_dAttrsPtr. i.e. other ptr results
	const CSphSchemaHelper *		m_pSchema = nullptr;
	bool							m_bPtrRowsCommited = false; // readiness of m_dMyPtrRows and m_dOtherPtrRows

public:
	void SetSchema ( const ISphSchema * pSchema )
	{
		m_pSchema = ( CSphSchemaHelper * ) pSchema; /// lazy hack
		m_dRowBuf.Reset ( m_pSchema->GetDynamicSize() );
	}

	// clone plain part (incl. pointers) from src to dst
	// keep group part (aggregates, group_concat) of dst intact
	// it assumes that tDst m_pDynamic contains correct data, or wiped away.
	void CloneKeepingAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
	{
		assert ( m_pSchema );
		assert ( m_bPtrRowsCommited );

		// memorize old dynamic first
		memcpy ( m_dRowBuf.Begin(), tDst.m_pDynamic, m_dRowBuf.GetLengthBytes() );

		m_pSchema->FreeDataSpecial ( tDst, m_dOtherPtrRows );
		m_pSchema->CloneMatchSpecial ( tDst, tSrc, m_dOtherPtrRows );

		/*
			FreeDataSpecial ( tDst, m_dOtherPtrRows );
			pDst->Combine ( *pSrc, GetDynamicSize () );
			CopyPtrsSpecial ( tDst, tSrc, m_dOtherPtrRows );
		*/

		// restore back group-by attributes
		for ( auto& dAttrGrp : m_dAttrsGrp )
			tDst.SetAttr ( dAttrGrp, sphGetRowAttr ( m_dRowBuf.Begin(), dAttrGrp ) );

		// restore back group_concat attribute(s)
		for ( auto& dAttrPtr : m_dAttrsPtr )
			tDst.SetAttr ( dAttrPtr, sphGetRowAttr ( m_dRowBuf.Begin (), dAttrPtr ) );
	}

	// clone plain part (incl. pointers) from src to dst
	// group part (aggregates, group_concat) is not copied
	void CloneWithoutAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
	{
		assert ( m_pSchema );
		assert ( m_bPtrRowsCommited );

		m_pSchema->CloneMatchSpecial ( tDst, tSrc, m_dOtherPtrRows );
		/*
			FreeDataSpecial ( tDst, m_dOtherPtrRows );
			pDst->Combine ( *pSrc, GetDynamicSize () );
			CopyPtrsSpecial ( tDst, tSrc, m_dOtherPtrRows );
		*/
	}

	// just write group part (aggregates, group_concat) without cloning
	// assumes tDst has allocated m_pDynamic. Fixme! look to #881 again...
	void CopyAggrs ( CSphMatch & tDst, const CSphMatch & tSrc )
	{
		assert ( m_pSchema );
		assert ( m_bPtrRowsCommited );
		assert ( &tDst!=&tSrc );
		assert ( tDst.m_pDynamic );

		for ( auto & dAttrGrp : m_dAttrsGrp )
			tDst.SetAttr ( dAttrGrp, tSrc.GetAttr ( dAttrGrp ));

		CSphSchemaHelper::FreeDataSpecial ( tDst, m_dMyPtrRows );
		CSphSchemaHelper::CopyPtrsSpecial ( tDst, tSrc, m_dMyPtrRows );
	}

	// copy group part (aggregates)
	// move group_concat part without reallocating
	void MoveAggrs ( CSphMatch & tDst, CSphMatch & tSrc )
	{
		assert ( m_pSchema );
		assert ( m_bPtrRowsCommited );
		assert ( &tDst!=&tSrc );
		assert ( tDst.m_pDynamic );

		for ( auto & dAttrGrp : m_dAttrsGrp )
			tDst.SetAttr ( dAttrGrp, tSrc.GetAttr ( dAttrGrp ));

		CSphSchemaHelper::MovePtrsSpecial( tDst, tSrc, m_dMyPtrRows );
	}

	inline void AddRaw ( const CSphAttrLocator& tLoc )
	{
		m_dAttrsGrp.Add ( tLoc );
	}

	inline void AddPtr ( const CSphAttrLocator &tLoc )
	{
		m_dAttrsPtr.Add ( tLoc );
	}
	inline void ResetAttrs()
	{
		m_dAttrsGrp.Resize ( 0 );
		m_dAttrsPtr.Resize ( 0 );
	}

	// (re)fill m_dMyPtrRows and m_dOtherPtrRows from m_dAttrsPtr
	inline void CommitPtrs ()
	{
		assert ( m_pSchema );
		static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

		if ( m_bPtrRowsCommited )
			m_dMyPtrRows.Resize(0);

		for ( const CSphAttrLocator &tLoc : m_dAttrsPtr )
			m_dMyPtrRows.Add ( tLoc.m_iBitOffset / SIZE_OF_ROW );

		m_dOtherPtrRows = m_pSchema->SubsetPtrs ( m_dMyPtrRows );

#ifndef NDEBUG
		// sanitize check
		m_dMyPtrRows = m_pSchema->SubsetPtrs ( m_dOtherPtrRows );
		assert ( m_dMyPtrRows.GetLength ()==m_dAttrsPtr.GetLength () );
#endif
		m_bPtrRowsCommited = true;
	}
};

template <typename MVA, typename ADDER>
static void AddGroupedMVA ( ADDER && fnAdd, const ByteBlob_t& dRawMVA )
{
	VecTraits_T<MVA> dMvas {dRawMVA};
	for ( auto& dValue : dMvas )
		fnAdd ( sphUnalignedRead ( dValue ) );
}

template <typename PUSH>
bool PushJsonField ( int64_t iValue, const BYTE * pBlobPool, PUSH && fnPush )
{
	int iLen;
	char szBuf[32];
	SphGroupKey_t uGroupKey;

	ESphJsonType eJson = sphJsonUnpackType ( iValue );
	const BYTE * pValue = pBlobPool + sphJsonUnpackOffset ( iValue );

	switch ( eJson )
	{
		case JSON_ROOT:
		{
			iLen = sphJsonNodeSize ( JSON_ROOT, pValue );
			bool bEmpty = iLen==5; // mask and JSON_EOF
			uGroupKey = bEmpty ? 0 : sphFNV64 ( pValue, iLen );
			return fnPush ( bEmpty ? nullptr : &iValue, uGroupKey );
		}

		case JSON_STRING:
		case JSON_OBJECT:
		case JSON_MIXED_VECTOR:
			iLen = sphJsonUnpackInt ( &pValue );
			uGroupKey = ( iLen==1 && eJson!=JSON_STRING ) ? 0 : sphFNV64 ( pValue, iLen );
			return fnPush ( ( iLen==1 && eJson!=JSON_STRING ) ? nullptr : &iValue, uGroupKey );

		case JSON_STRING_VECTOR:
		{
			bool bRes = false;
			sphJsonUnpackInt ( &pValue );
			iLen = sphJsonUnpackInt ( &pValue );
			for ( int i=0;i<iLen;i++ )
			{
				int64_t iNewValue = sphJsonPackTypeOffset ( JSON_STRING, pValue-pBlobPool );

				int iStrLen = sphJsonUnpackInt ( &pValue );
				uGroupKey = sphFNV64 ( pValue, iStrLen );
				bRes |= fnPush ( &iNewValue, uGroupKey );
				pValue += iStrLen;
			}
			return bRes;
		}

		case JSON_INT32:
			return fnPush ( &iValue, sphFNV64 ( (BYTE*)FormatInt ( szBuf, (int)sphGetDword(pValue) ) ) );

		case JSON_INT64:
			return fnPush ( &iValue, sphFNV64 ( (BYTE*)FormatInt ( szBuf, (int)sphJsonLoadBigint ( &pValue ) ) ) );

		case JSON_DOUBLE:
			snprintf ( szBuf, sizeof(szBuf), "%f", sphQW2D ( sphJsonLoadBigint ( &pValue ) ) );
			return fnPush ( &iValue, sphFNV64 ( (const BYTE*)szBuf ) );

		case JSON_INT32_VECTOR:
		{
			bool bRes = false;
			iLen = sphJsonUnpackInt ( &pValue );
			auto p = (int*)pValue;
			for ( int i=0;i<iLen;i++ )
			{
				int64_t iPacked = sphJsonPackTypeOffset ( JSON_INT32, (BYTE*)p-pBlobPool );
				uGroupKey = *p++;
				bRes |= fnPush ( &iPacked, uGroupKey );
			}
			return bRes;
		}

		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
		{
			bool bRes = false;
			iLen = sphJsonUnpackInt ( &pValue );
			auto p = (int64_t*)pValue;
			ESphJsonType eType = eJson==JSON_INT64_VECTOR ? JSON_INT64 : JSON_DOUBLE;
			for ( int i=0;i<iLen;i++ )
			{
				int64_t iPacked = sphJsonPackTypeOffset ( eType, (BYTE*)p-pBlobPool );
				uGroupKey = *p++;
				bRes |= fnPush ( &iPacked, uGroupKey );
			}
			return bRes;
		}

		default:
			uGroupKey = 0;
			iValue = 0;
			return fnPush ( &iValue, uGroupKey );
	}
}

template <typename ADDER>
static void AddDistinctKeys ( const CSphMatch & tEntry, CSphAttrLocator & tDistinctLoc, ESphAttr eDistinctAttr, const BYTE * pBlobPool, ADDER && fnAdd )
{
	switch ( eDistinctAttr )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR: // fixme! also json?
		{
			auto dBlob = tEntry.FetchAttrData ( tDistinctLoc, pBlobPool );
			auto tAttr = (SphAttr_t) sphFNV64 ( dBlob );
			fnAdd ( tAttr );
		}
		break;

	case SPH_ATTR_JSON_FIELD:
		PushJsonField ( tEntry.GetAttr(tDistinctLoc), pBlobPool, [fnAdd]( SphAttr_t * pAttr, SphGroupKey_t uGroupKey )
			{
				if ( uGroupKey )
					fnAdd(uGroupKey);

				return true;
			}
		);
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_UINT32SET_PTR:
		AddGroupedMVA<DWORD> ( fnAdd, tEntry.FetchAttrData ( tDistinctLoc, pBlobPool ) );
		break;

	case SPH_ATTR_INT64SET:
	case SPH_ATTR_INT64SET_PTR:
		AddGroupedMVA<int64_t> ( fnAdd, tEntry.FetchAttrData ( tDistinctLoc, pBlobPool ) );
		break;

	// fixme! what about json?
	default:
		fnAdd ( tEntry.GetAttr ( tDistinctLoc ) );
		break;
	}
}

class BaseGroupSorter_c : public BlobPool_c, protected CSphGroupSorterSettings
{
	using BASE = CSphGroupSorterSettings;

protected:
	MatchCloner_t			m_tPregroup;
	CSphVector<IAggrFunc *>	m_dAggregates;

public:

	FWD_BASECTOR( BaseGroupSorter_c )

	virtual ~BaseGroupSorter_c()
	{
		for ( auto &pAggregate : m_dAggregates )
			SafeDelete ( pAggregate );
	}

protected:
	/// schema, aggregates setup
	template <bool DISTINCT>
	inline void SetupBaseGrouper ( ISphSchema * pSchema, const ESphSortKeyPart * pSortKeyPart = nullptr,
		const CSphAttrLocator * pAttrLocator=nullptr, CSphVector<IAggrFunc *>* pAvgs = nullptr )
	{
		m_tPregroup.SetSchema ( pSchema );
		m_tPregroup.AddRaw ( m_tLocGroupby ); // @groupby
		m_tPregroup.AddRaw ( m_tLocCount ); // @count
		if_const ( DISTINCT )
			m_tPregroup.AddRaw ( m_tLocDistinct ); // @distinct

		// extract aggregates
		for ( int i = 0; i<pSchema->GetAttrsCount (); ++i )
		{
			const CSphColumnInfo &tAttr = pSchema->GetAttr ( i );

			if ( tAttr.m_eAggrFunc==SPH_AGGR_NONE
				|| IsGroupbyMagic ( tAttr.m_sName ) // @count, @groupby, @groupbystr, @distinct, count(*), groupby()
				|| IsSortStringInternal ( tAttr.m_sName.cstr () ) )
				continue;

			switch ( tAttr.m_eAggrFunc )
			{
			case SPH_AGGR_SUM:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrSum_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrSum_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrSum_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_AVG:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER:
					m_dAggregates.Add ( new AggrAvg_t<DWORD> ( tAttr.m_tLocator, m_tLocCount ) ); break;
				case SPH_ATTR_BIGINT:
					m_dAggregates.Add ( new AggrAvg_t<int64_t> ( tAttr.m_tLocator, m_tLocCount ) );break;
				case SPH_ATTR_FLOAT:
					m_dAggregates.Add ( new AggrAvg_t<float> ( tAttr.m_tLocator, m_tLocCount ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				// store avg to calculate these attributes prior to groups sort
				if ( pAvgs && pSortKeyPart && pAttrLocator )
					for ( int iState = 0; iState<CSphMatchComparatorState::MAX_ATTRS; ++iState )
					{
						auto eKeypart = pSortKeyPart[iState];
						const auto & tLoc = pAttrLocator[iState];
						if ( ( eKeypart==SPH_KEYPART_INT || eKeypart==SPH_KEYPART_FLOAT )
							&& tLoc.m_bDynamic==tAttr.m_tLocator.m_bDynamic
							&& tLoc.m_iBitOffset==tAttr.m_tLocator.m_iBitOffset
							&& tLoc.m_iBitCount==tAttr.m_tLocator.m_iBitCount )
						{
							pAvgs->Add ( m_dAggregates.Last () );
							break;
						}
					}
				break;

			case SPH_AGGR_MIN:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrMin_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrMin_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrMin_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_MAX:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrMax_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrMax_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrMax_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_CAT:
				m_dAggregates.Add ( new AggrConcat_t ( tAttr ) );
				m_tPregroup.AddPtr ( tAttr.m_tLocator );
				break;

			default: assert ( 0 && "internal error: unhandled aggregate function" );
				break;
			}

			if ( tAttr.m_eAggrFunc!=SPH_AGGR_CAT )
				m_tPregroup.AddRaw ( tAttr.m_tLocator );
		}
		m_tPregroup.CommitPtrs();
	}

	// HAVING filtering
	bool EvalHAVING ( const CSphMatch& tMatch )
	{
		return !m_pAggrFilterTrait || m_pAggrFilterTrait->Eval ( tMatch );
	}

	void AggrUpdate ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped )
	{
		for ( auto * pAggregate : this->m_dAggregates )
			pAggregate->Update ( tDst, tSrc, bGrouped );
	};

	void AggrUngroup ( CSphMatch & tMatch )
	{
		for ( auto * pAggregate : this->m_dAggregates )
			pAggregate->Ungroup ( tMatch );
	}
};

class SubGroupSorter_fn : public ISphNoncopyable
{
	const VecTraits_T<CSphMatch> & m_dBase;
	const CSphMatchComparatorState& m_tState;
	const ISphMatchComparator * m_pComp;

public:
	SubGroupSorter_fn ( const CSphMatchQueueTraits & dBase, const ISphMatchComparator * pC )
		: m_dBase ( dBase.GetMatches () )
		, m_tState ( dBase.GetComparatorState() )
		, m_pComp ( pC )
	{
		assert ( m_pComp );
		m_pComp->AddRef();
	}

	~SubGroupSorter_fn()
	{
		m_pComp->Release();
	}

	const ISphMatchComparator * GetComparator() const
	{
		return m_pComp;
	}

	bool MatchIsGreater ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return m_pComp->VirtualIsLess ( b, a, m_tState );
	}

	// inverse order, i.e. work as IsGreater
	bool IsLess ( int a, int b ) const
	{
		return m_pComp->VirtualIsLess ( m_dBase[b], m_dBase[a], m_tState );
	}
};

/// match sorter with k-buffering and group-by - common part
template<typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS>
struct KBufferGroupSorter_T : public CSphMatchQueueTraits, protected BaseGroupSorter_c
{
	using MYTYPE = KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>;
	ESphGroupBy 				m_eGroupBy;     ///< group-by function
	CSphRefcountedPtr<CSphGrouper>	m_pGrouper;
	int							m_iLimit;		///< max matches to be retrieved
	CSphUniqounter				m_tUniq;
	bool						m_bSortByDistinct = false;
	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	SubGroupSorter_fn			m_tSubSorter;
	CSphVector<IAggrFunc *>		m_dAvgs;
	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	KBufferGroupSorter_T ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
			const CSphGroupSorterSettings & tSettings )
			: CSphMatchQueueTraits ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
			, BaseGroupSorter_c ( tSettings )
			, m_eGroupBy ( pQuery->m_eGroupFunc )
			, m_pGrouper ( tSettings.m_pGrouper )
			, m_iLimit ( tSettings.m_iMaxMatches )
			, m_tGroupSorter (*this)
			, m_tSubSorter ( *this, pComp )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( !DISTINCT || tSettings.m_tDistinctAttr.m_iBitOffset>=0 );
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final
	{
		if ( m_pSchema )
		{
			FixupLocators ( m_pSchema, pSchema );
			m_tGroupSorter.FixupLocators ( m_pSchema, pSchema, bRemapCmp );
			m_tPregroup.ResetAttrs ();
			m_dAggregates.Apply ( [] ( IAggrFunc * pAggr ) { SafeDelete ( pAggr ); } );
			m_dAggregates.Resize ( 0 );
			m_dAvgs.Resize ( 0 );
		}

		ISphMatchSorter::SetSchema ( pSchema, bRemapCmp );
		SetupBaseGrouper<DISTINCT> ( pSchema, m_tGroupSorter.m_eKeypart, m_tGroupSorter.m_tLocator, &m_dAvgs );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const final
	{
		return true;
	}

	/// set blob pool pointer (for string+groupby sorters)
	void SetBlobPool ( const BYTE * pBlobPool ) final
	{
		BlobPool_c::SetBlobPool ( pBlobPool );
		m_pGrouper->SetBlobPool ( pBlobPool );
	}

	/// get entries count
	int GetLength () const override
	{
		return Min ( Used(), m_iLimit );
	}

	/// set group comparator state
	void SetGroupState ( const CSphMatchComparatorState & tState ) final
	{
		m_tGroupSorter.m_fnStrCmp = tState.m_fnStrCmp;

		// FIXME! manual bitwise copying.. yuck
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
		{
			m_tGroupSorter.m_eKeypart[i] = tState.m_eKeypart[i];
			m_tGroupSorter.m_tLocator[i] = tState.m_tLocator[i];
		}
		m_tGroupSorter.m_uAttrDesc = tState.m_uAttrDesc;
		m_tGroupSorter.m_iNow = tState.m_iNow;

		// check whether we sort by distinct
		if_const ( DISTINCT && m_tDistinctAttr.m_iBitOffset>=0 )
			for ( auto& tLocator : m_tGroupSorter.m_tLocator )
				if ( tLocator.m_iBitOffset==m_tDistinctAttr.m_iBitOffset )
				{
					m_bSortByDistinct = true;
					break;
				}
	}

	bool CanBeCloned () const final
	{
		return !DISTINCT;
	}

protected:
	/// finalize distinct counters
	template <typename FIND>
	void Distinct ( FIND&& fnFind )
	{
		m_tUniq.Sort ();
		SphGroupKey_t uGroup;
		for ( int iCount = m_tUniq.CountStart ( &uGroup ); iCount; iCount = m_tUniq.CountNext ( &uGroup ) )
		{
			CSphMatch * pMatch = fnFind ( uGroup );
			if ( pMatch )
				pMatch->SetAttr ( m_tLocDistinct, iCount );
		}
	}

	inline void SetupBaseGrouperWrp ( ISphSchema * pSchema, const ESphSortKeyPart * pSortKeyPart = nullptr,
			const CSphAttrLocator * pAttrLocator = nullptr, CSphVector<IAggrFunc *> * pAvgs = nullptr )
	{
		SetupBaseGrouper<DISTINCT> ( pSchema, pSortKeyPart, pAttrLocator, pAvgs );
	}

	void CloneKBufferGroupSorter ( MYTYPE* pClone ) const
	{
		// basic clone
		ISphMatchSorter::CloneTo ( pClone );

		// actions from SetGroupState
		pClone->m_bSortByDistinct = m_bSortByDistinct;
		pClone->m_tGroupSorter.m_fnStrCmp = m_tGroupSorter.m_fnStrCmp;
		for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		{
			pClone->m_tGroupSorter.m_eKeypart[i] = m_tGroupSorter.m_eKeypart[i];
			pClone->m_tGroupSorter.m_tLocator[i] = m_tGroupSorter.m_tLocator[i];
		}
		pClone->m_tGroupSorter.m_uAttrDesc = m_tGroupSorter.m_uAttrDesc;
		pClone->m_tGroupSorter.m_iNow = m_tGroupSorter.m_iNow;

		// complete SetSchema
		pClone->m_dAvgs.Resize ( 0 );
		pClone->SetupBaseGrouperWrp ( m_pSchema, pClone->m_tGroupSorter.m_eKeypart,
				pClone->m_tGroupSorter.m_tLocator, &pClone->m_dAvgs );
	}

	CSphVector<IAggrFunc *> GetAggregatesWithoutAvgs() const
	{
		CSphVector<IAggrFunc *> dAggrs;
		if ( m_dAggregates.GetLength ()!=m_dAvgs.GetLength ())
		{
			dAggrs = m_dAggregates;
			for ( auto * pAvg : this->m_dAvgs )
				dAggrs.RemoveValue ( pAvg );
		}
		return dAggrs;
	}

	void FreeMatchPtrs ( int iMatch, bool bNotify=true )
	{
		if_const ( NOTIFICATIONS && bNotify )
			m_dJustPopped.Add ( m_dData[iMatch].m_tRowID );
		m_pSchema->FreeDataPtrs ( m_dData[iMatch] );

		// on final pass we totally wipe match.
		// That is need, since otherwise such 'garbage' matches with non-null m_pDynamic
		// will be targeted in d-tr with FreeDataPtrs with possible another(!) schema
		if ( !bNotify )
			m_dData[iMatch].ResetDynamic ();
	}

	inline void UpdateDistinct ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped )
	{
		int iCount = bGrouped ? (int) tEntry.GetAttr ( m_tLocDistinct ) : 1;
		AddDistinctKeys ( tEntry, m_tDistinctAttr, m_eDistinctAttr, GetBlobPool(),
				[this, uGroupKey, iCount] ( SphAttr_t b ) { m_tUniq.Add ( {uGroupKey, b, iCount} ); });
	}

	void RemoveDistinct ( VecTraits_T<SphGroupKey_t>& dRemove )
	{
		// sort and compact
		if ( !m_bSortByDistinct )
			m_tUniq.Sort ();
		m_tUniq.Compact ( dRemove );
	}
};

/// match sorter with k-buffering and group-by
/// invoking by select ... group by ... where only plain attributes (i.e. NO mva, NO jsons)
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferGroupSorter : public KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	bool m_bMatchesFinalized = false;
	int m_iMaxUsed = -1;

protected:
	OpenHash_T < CSphMatch *, SphGroupKey_t >	m_hGroup2Match;

	// since we inherit from template, we need to write boring 'using' block
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	using KBufferGroupSorter::m_eGroupBy;
	using KBufferGroupSorter::m_pGrouper;
	using KBufferGroupSorter::m_iLimit;
	using KBufferGroupSorter::m_tUniq;
	using KBufferGroupSorter::m_bSortByDistinct;
	using KBufferGroupSorter::m_tGroupSorter;
	using KBufferGroupSorter::m_tSubSorter;
	using KBufferGroupSorter::m_dAvgs;
	using KBufferGroupSorter::GROUPBY_FACTOR;
	using KBufferGroupSorter::CloneKBufferGroupSorter;
	using KBufferGroupSorter::GetAggregatesWithoutAvgs;
	using KBufferGroupSorter::Distinct;
	using KBufferGroupSorter::UpdateDistinct;
	using KBufferGroupSorter::RemoveDistinct;
	using KBufferGroupSorter::FreeMatchPtrs;

	using CSphGroupSorterSettings::m_tLocGroupby;
	using CSphGroupSorterSettings::m_tLocCount;
	using CSphGroupSorterSettings::m_tLocDistinct;

	using ISphMatchSorter::m_iTotal;
	using ISphMatchSorter::m_iJustPushed;
	using ISphMatchSorter::m_pSchema;

	using BaseGroupSorter_c::EvalHAVING;
	using BaseGroupSorter_c::AggrUpdate;
	using BaseGroupSorter_c::AggrUngroup;

	using CSphMatchQueueTraits::m_iSize;
	using CSphMatchQueueTraits::m_dData;
	using CSphMatchQueueTraits::m_bStorageFilled;
	using CSphMatchQueueTraits::Get;
	using CSphMatchQueueTraits::Add;
	using CSphMatchQueueTraits::Used;
	using CSphMatchQueueTraits::ResetAfterFlatten;

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
			const CSphGroupSorterSettings & tSettings )
		: KBufferGroupSorter ( pComp, pQuery, tSettings )
		, m_hGroup2Match ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
	{}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		SphGroupKey_t uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool ) override
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, false );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		FinalizeMatches();

		auto dAggrs = GetAggregatesWithoutAvgs();
		const CSphMatch * pBegin = pTo;

		for ( auto iMatch : this->m_dIData )
		{
			CSphMatch & tMatch = m_dData[iMatch];
			dAggrs.Apply ( [&tMatch] ( IAggrFunc * pAggr ) { pAggr->Finalize ( tMatch ); } );
			if ( !EvalHAVING ( tMatch ))
			{
				FreeMatchPtrs ( iMatch, false );
				continue;
			}

			Swap ( *pTo, tMatch );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			++pTo;
		}

		m_iTotal = 0;
		m_bMatchesFinalized = false;

		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		ResetAfterFlatten ();

		for ( int i = 0; i<m_iMaxUsed; ++i )
			m_dData[i].ResetDynamic ();
		m_iMaxUsed = -1;

		return int ( pTo-pBegin );
	}

	// TODO! TEST!
	ISphMatchSorter * Clone () const override
	{
		CSphQuery dFoo;
		dFoo.m_iMaxMatches = m_iLimit;
		dFoo.m_eGroupFunc = m_eGroupBy;
		auto pClone = new MYTYPE ( m_tSubSorter.GetComparator (), &dFoo, *this );
		CloneKBufferGroupSorter ( pClone );
		return pClone;
	}

	// FIXME! test CSphKBufferGroupSorter
	void MoveTo ( ISphMatchSorter * pRhs) final
	{
		if ( !Used () )
			return;

		auto& dRhs = *(MYTYPE *) pRhs;
		if ( !dRhs.Used () )
		{
			CSphMatchQueueTraits::SwapMatchQueueTraits ( dRhs );
			m_hGroup2Match.Swap ( dRhs.m_hGroup2Match );
			dRhs.m_bMatchesFinalized = m_bMatchesFinalized;
			dRhs.m_iMaxUsed = m_iMaxUsed;
			m_iMaxUsed = -1;
			return;
		}

		FinalizeMatches ();

		// just push in heap order
		// since we have grouped matches, it is not always possible to move them,
		// so use plain push instead
		for ( auto iMatch : this->m_dIData )
			dRhs.PushGrouped ( m_dData[iMatch], false );
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !Used() )
			return;

		FinalizeMatches ();

		// just evaluate in heap order
		for ( auto iMatch : this->m_dIData )
			tProcessor.Process ( &m_dData[iMatch] );
	}

protected:
	bool PushIntoExistingGroup( CSphMatch & tGroup, const CSphMatch & tEntry, SphGroupKey_t uGroupKey,
			bool bGrouped, SphAttr_t * pAttr )
	{
		assert ( tGroup.GetAttr ( m_tLocGroupby )==uGroupKey );
		assert ( tGroup.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

		auto & tLocCount = m_tLocCount;
		if ( bGrouped )
			tGroup.AddCounterAttr ( tLocCount, tEntry );
		else
			tGroup.AddCounterScalar ( tLocCount, 1 );

		// update aggregates
		AggrUpdate ( tGroup, tEntry, bGrouped );

		// if new entry is more relevant, update from it
		if ( m_tSubSorter.MatchIsGreater ( tEntry, tGroup ) )
		{
			if_const ( NOTIFICATIONS )
			{
				m_iJustPushed = tEntry.m_tRowID;
				this->m_dJustPopped.Add ( tGroup.m_tRowID );
			}

			// clone the low part of the match
			this->m_tPregroup.CloneKeepingAggrs ( tGroup, tEntry );
			if ( pAttr )
				UpdateGroupbyStr ( tGroup, pAttr );
		}

		// submit actual distinct value
		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, uGroupKey, bGrouped );
		return false; // since it is dupe
	}

	/// add entry to the queue
	bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey,
			bool bGrouped, bool, SphAttr_t * pAttr=nullptr )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			this->m_dJustPopped.Resize ( 0 );
		}
		auto & tLocCount = m_tLocCount;

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match.Find ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tLocGroupby )==uGroupKey );
			return PushIntoExistingGroup ( *pMatch, tEntry, uGroupKey, bGrouped, pAttr );
		}

		// submit actual distinct value
		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, uGroupKey, bGrouped );

		// if we're full, let's cut off some worst groups
		if ( Used()==m_iSize )
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );

		// do add
		assert ( Used()<m_iSize );
		CSphMatch & tNew = Add();
		m_pSchema->CloneMatch ( tNew, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_tRowID;

		if ( bGrouped )
			AggrUngroup ( tNew );
		else {
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );

			if ( pAttr )
				UpdateGroupbyStr ( tNew, pAttr );
		}

		m_hGroup2Match.Add ( uGroupKey, &tNew );
		++m_iTotal;
		return true;
	}

private:
	enum class Avg_e { FINALIZE, UNGROUP };
	void CalcAvg ( Avg_e eGroup )
	{
		if ( m_dAvgs.IsEmpty() )
			return;

		if ( eGroup==Avg_e::FINALIZE )
			for ( auto i : this->m_dIData )
				m_dAvgs.Apply( [this,i] ( IAggrFunc * pAvg ) { pAvg->Finalize ( m_dData[i] ); } );
		else
			for ( auto i : this->m_dIData )
				m_dAvgs.Apply ( [this,i] ( IAggrFunc * pAvg ) { pAvg->Ungroup ( m_dData[i] ); } );
	}

	/// finalize counted distinct values
	void CountDistinct ()
	{
		Distinct ( [this] ( SphGroupKey_t uGroup )->CSphMatch *
		{
			auto ppMatch = m_hGroup2Match.Find ( uGroup );
			return ppMatch ? *ppMatch : nullptr;
		});
	}

	// make final order before finalize/flatten call
	void FinalizeMatches ()
	{
		if ( m_bMatchesFinalized )
			return;
		m_bMatchesFinalized = true;

		if ( Used ()>m_iLimit )
			CutWorst ( m_iLimit, true );
		else {
			if_const ( DISTINCT )
				CountDistinct ();
			CalcAvg ( Avg_e::FINALIZE );
			SortGroups ();
		}
	}

	void RebuildHash ()
	{
		for ( auto iMatch : this->m_dIData ) {
			auto & tMatch = m_dData[iMatch];
			m_hGroup2Match.Add ( tMatch.GetAttr ( m_tLocGroupby ), &tMatch );
		}
	}

	/// cut worst N groups off the buffer tail, and maybe sort the best part
	void CutWorst ( int iBound, bool bFinalize=false )
	{
		// prepare to partition - finalize distinct, avgs to provide smooth sorting
		if_const ( DISTINCT && m_bSortByDistinct )
			CountDistinct ();

		CalcAvg ( Avg_e::FINALIZE );

		// relocate best matches to the low part (up to the iBound)
		BinaryPartition (iBound);

		// take worst matches and free them (distinct stuff, data ptrs)
		auto dWorst = this->m_dIData.Slice ( iBound );

		if_const ( DISTINCT )
		{
			CSphVector<SphGroupKey_t> dRemove;
			for ( auto iMatch : dWorst )
				dRemove.Add ( m_dData[iMatch].GetAttr ( m_tLocGroupby ));
			RemoveDistinct ( dRemove );
		}

		dWorst.Apply ( [this,bFinalize] ( int iMatch ) { FreeMatchPtrs ( iMatch, !bFinalize ); } );

		m_iMaxUsed = Max ( m_iMaxUsed, this->m_dIData.GetLength() ); // memorize it for free dynamics later.
		this->m_dIData.Resize ( iBound );
		m_hGroup2Match.Clear();

		if ( bFinalize ) {
			SortGroups ();
			if_const ( DISTINCT && !m_bSortByDistinct ) // since they wasn't counted at the top
				CountDistinct ();
		} else {
			// we've called CalcAvg ( Avg_e::FINALIZE ) before partitioning groups
			// now we can undo this calculation for the rest apart of thrown away
			// on finalize (sorting) cut we don't need to ungroup here
			CalcAvg ( Avg_e::UNGROUP );
			RebuildHash();
		}
		m_bStorageFilled = true;
	}

	/// sort groups buffer
	void SortGroups ()
	{
		this->m_dIData.Sort ( m_tGroupSorter );
	}

	// update @groupbystr value, if available
	void UpdateGroupbyStr ( CSphMatch& tMatch, const SphAttr_t * pAttr )
	{
		if ( this->m_tLocGroupbyStr.m_bDynamic )
			tMatch.SetAttr ( this->m_tLocGroupbyStr, *pAttr );
	}

	// lazy resort matches so that best are located up to iBound
	void BinaryPartition ( int iBound )
	{
		float COEFF = Max ( 1.0, float(Used ()) / iBound );
		int iPivot = this->m_dIData[iBound/COEFF];

		--iBound;
		int a=0;
		int b=Used()-1;
		while (true)
		{
			int i=a;
			int j=b;
			while (i<=j)
			{
				while (m_tGroupSorter.IsLess (this->m_dIData[i],iPivot)) 	++i;
				while (m_tGroupSorter.IsLess (iPivot, this->m_dIData[j]))	--j;
				if ( i<=j ) ::Swap( this->m_dIData[i++], this->m_dIData[j--]);
			}
			if ( iBound == j )
				break;

			if ( iBound < j)
				b = j;  // too many elems aquired; continue with left part
			else
				a = i;  // too less elems aquired; continue with right part
			iPivot = this->m_dIData[( a * ( COEFF-1 )+b ) / COEFF];
		}
	}
};


#define LOG_COMPONENT_NG __FILE__ << ":" << __LINE__ << " -"
#define LOG_LEVEL_DIAG false

#define DBG LOC(DIAG,NG)

/// match sorter with k-buffering and N-best group-by

/* Trick explanation
 *
 * Here we keep several grouped matches, but each one is not a single match, but a group.
 * On the backend we have solid vector of real matches. They are allocated once and freed, and never moved around.
 * To work with them we have vector of indexes, so that each index points to corresponding match in the backend.
 * So when performing moving operations (sort, etc.) we actually change indexes and never move matches themselves.
 *
 * Say, when user pushes matches with weights of 5,2,3,1,4,6 and we then sort them, we will have following relation:
 *
 * m5 m2 m3 m1 m4 m6 // backend, placed in natural order as they come here
 *  1  2  3  4  5  6 // original indexes, just points directly to backend matches.
 *
 * After, say, sort by asc matches weights, only index vector modified and becames this:
 *
 *  4  2  3  5  1  6 // reading match[i[k]] for k in 0..5 will return matches in weight ascending order.
 *
 * When grouping we collect several matches together and sort them.
 * Say, if one group contains matches m1, m2, m5, m6 and second - m4, m3, we have to keep 2 sets of matches in hash:
 *
 * h1: m1 m2 m5 m6
 * h2: m4 m3
 *
 * How to store that sequences?
 *
 * Well, we can do it directly, set by set, keeping heads in hash:
 * m1 m2 m5 m6 m4 m3, heads 1, 5
 *
 * going to indirection indexes we have sequence
 *  4  2  1  6  5  3, hash 1, 4
 *
 * That looks ok, but since sets can dynamically change, it is hard to insert more into existing group.
 * That is like insertion into the middle of vector.
 *
 * Let's try to make a list (chain). Don't care about in-group ordering, just keep things chained.
 * To make things easier, ring the list (connect tail back to head), and store pos of one of the elems in the hash
 * (since it is ring - that is not important which exactly, just to have something to glue).
 *
 * m5 -> 1 heads 1
 * m2 -> 2, 1 heads 2
 * m3 -> 2, 1, 3, heads 2, 3
 * m1 -> 2, 4, 3, 1, heads 4, 3
 * m4 -> 2, 4, 5, 1, 3, heads 4, 5
 * m6 -> 2, 4, 5, 6, 3, 1  heads 6, 5
 *
 * On insert we store old head into new elem, and new elem into the place of old head.
 * One thing rest here is indirect ref by position. I.e. we assume that index at position 6 points to match at position 6.
 * However we can notice, that since it is ring, left elem of 6-th points to it directly by number 6.
 * So we can just shift heads back by one position - and that's all, indirect assumption no more necessary.
 * Final sequence will be this one:
 * m5 m2 m3 m1 m4 m6 - matches in their natural order
 * 2, 4, 5, 6, 3, 1 - indirection vec. 4, 3. - heads of groups.
 *
 * Iteration: take 1-st group with head 4:
 * 6->1->2->4*. Each num is both index of the link, and index of backend match. So, matches here are:
 * m6 m5 m2 m1, and we can resort them as necessary (indirectly). Viola!
 *
 * On deletion item goes to freelist.
 * Allocation of an elem is separate task, it is achieved by linear allocation (first), and by freelist (when filled).
 *
 */

template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferNGroupSorter : public KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferNGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS>;

protected:
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	using KBufferGroupSorter::m_eGroupBy;
	using KBufferGroupSorter::m_pGrouper;
	using KBufferGroupSorter::m_iLimit;
	using KBufferGroupSorter::m_tUniq;
	using KBufferGroupSorter::m_bSortByDistinct;
	using KBufferGroupSorter::m_tGroupSorter;
	using KBufferGroupSorter::m_tSubSorter;
	using KBufferGroupSorter::m_dAvgs;
	using KBufferGroupSorter::GROUPBY_FACTOR;
	using KBufferGroupSorter::CloneKBufferGroupSorter;
	using KBufferGroupSorter::GetAggregatesWithoutAvgs;
	using KBufferGroupSorter::Distinct;
	using KBufferGroupSorter::FreeMatchPtrs;
	using KBufferGroupSorter::UpdateDistinct;
	using KBufferGroupSorter::RemoveDistinct;

	using CSphGroupSorterSettings::m_tLocGroupby;
	using CSphGroupSorterSettings::m_tLocCount;
	using CSphGroupSorterSettings::m_tLocDistinct;
//	using CSphGroupSorterSettings::m_tLocGroupbyStr; // check! unimplemented?

	using ISphMatchSorter::m_iTotal;
	using ISphMatchSorter::m_iJustPushed;
	using ISphMatchSorter::m_pSchema;

	using BaseGroupSorter_c::EvalHAVING;
	using BaseGroupSorter_c::AggrUpdate;
	using BaseGroupSorter_c::AggrUngroup;

	using CSphMatchQueueTraits::m_iSize;
	using CSphMatchQueueTraits::m_dData;

	int m_iStorageSolidFrom = 0; // edge from witch storage is not yet touched and need no chaining freelist
	OpenHash_T<int, SphGroupKey_t> m_hGroup2Index; // used to quickly locate group for incoming match

protected:
	const int		m_iGLimit;		///< limit per one group
	SphGroupKey_t	m_uLastGroupKey = -1;	///< helps to determine in pushEx whether the new subgroup started
	int				m_iFree = 0;			///< current insertion point
	int				m_iUsed = 0;

	// final cached data valid when everything is finalized
	bool			m_bFinalized = false;	// helper to avoid double work
	CSphVector<int> m_dFinalizedHeads;	/// < sorted finalized heads
	int				m_iLastGroupCutoff;	/// < cutof edge of last group to fit limit

#ifndef NDEBUG
	int				m_iruns = 0;		///< helpers for conditional breakpoints on debug
	int				m_ipushed = 0;
#endif
	LOC_ADD;

public:
	/// ctor
	CSphKBufferNGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
			const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: KBufferGroupSorter ( pComp, pQuery, tSettings )
		, m_hGroup2Index ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
		, m_iGLimit ( Min ( pQuery->m_iGroupbyLimit, m_iLimit ) )
	{
		assert ( m_iGLimit > 1 );
#ifndef NDEBUG
		DBG << "Created iruns = " << m_iruns << " ipushed = " << m_ipushed;
#endif
		this->m_dIData.Resize ( m_iSize ); // m_iLimit * GROUPBY_FACTOR
	}

	int GetLength () const override
	{
		return Min ( m_iUsed, m_iLimit );
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		auto uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, bNewSet );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		if ( !GetLength() )
			return 0;

		if ( !m_bFinalized )
		{
			FinalizeChains ();
			PrepareForExport ();
			CountDistinct ();
		}

		auto fnSwap = [&pTo,iTag] ( CSphMatch & tSrc ) 	{ // the writer
			Swap ( *pTo, tSrc );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			++pTo;
		};

		const CSphMatch * pBegin = pTo;
		for ( auto iHead : m_dFinalizedHeads )
		{
			CSphMatch & tGroupHead = m_dData[iHead];

			if ( !EvalHAVING ( tGroupHead ))
			{
				DeleteChain ( iHead, false );
				continue;
			}

			fnSwap ( tGroupHead ); // move top group match
			for ( int i=this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				fnSwap ( m_dData[i] ); // move tail matches
		}

		// final clean up before possible next pass
		m_uLastGroupKey = -1;
		m_iFree = 0;
		m_iUsed = 0;
		m_bFinalized = false;
		m_iStorageSolidFrom = 0;
		m_iTotal = 0;
		m_dFinalizedHeads.Reset ();
		m_hGroup2Index.Clear();
		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		return int ( pTo-pBegin );
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !GetLength() )
			return;

		if ( !m_bFinalized ) {
			FinalizeChains ();
			PrepareForExport ();
			CountDistinct ();
		}

		for ( auto iHead : m_dFinalizedHeads )
		{
			tProcessor.Process ( &m_dData[iHead] ); // process top group match
			for ( int i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				tProcessor.Process ( &m_dData[i] ); // process tail matches
		}
	}

	// TODO! TEST!
	ISphMatchSorter * Clone () const override
	{
		CSphQuery dFoo;
		dFoo.m_iGroupbyLimit = m_iGLimit;
		dFoo.m_iMaxMatches = m_iLimit;
		dFoo.m_eGroupFunc = m_eGroupBy;
		auto pClone = new MYTYPE ( this->m_tSubSorter.GetComparator (), &dFoo, *this );
		CloneKBufferGroupSorter ( pClone );
		return pClone;
	}

	// FIXME! todo CSphKBufferNGroupSorter
	void MoveTo ( ISphMatchSorter * pRhs) final
	{
#ifndef NDEBUG
		DBG << " MoveTo " << pRhs << " iRuns:iPushed - " << m_iruns << " " << m_ipushed;
#endif
		auto& dRhs = *(MYTYPE *) pRhs;
		if ( !dRhs.m_iTotal )
		{
			DBG << " Rhs is empty, adopt! ";
			CSphMatchQueueTraits::SwapMatchQueueTraits ( dRhs );
			m_hGroup2Index.Swap ( dRhs.m_hGroup2Index );
			::Swap ( m_uLastGroupKey, dRhs.m_uLastGroupKey );
			::Swap ( m_iFree, dRhs.m_iFree );
			::Swap ( m_iUsed, dRhs.m_iUsed );
			::Swap ( m_bFinalized, dRhs.m_bFinalized );
			m_dFinalizedHeads.SwapData ( dRhs.m_dFinalizedHeads );
			::Swap ( m_iStorageSolidFrom, dRhs.m_iStorageSolidFrom );
#ifndef NDEBUG
			::Swap ( m_iruns, dRhs.m_iruns );
			::Swap ( m_ipushed, dRhs.m_ipushed );
			LOC_SWAP(dRhs);
#endif
			return;
		}

		if ( !m_bFinalized ) {
			FinalizeChains ();
//			PrepareForExport (); // for moving we not need fine-finaled matches; just cleaned is enough
			CountDistinct ();
		}

		for ( auto iHead : m_dFinalizedHeads )
		{
			auto uGroupKey = m_dData[iHead].GetAttr ( m_tLocGroupby );
			dRhs.PushEx ( m_dData[iHead], uGroupKey, true, false, true );
			for ( int i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				dRhs.PushEx ( m_dData[i], uGroupKey, false, false, true );
			DeleteChain ( iHead, false );
		}
	}


protected:
	/// add entry to the queue
	/*
	 * Every match according to uGroupKey came to own subset.
	 * Head match of each group stored in the hash to quickly locate on next pushes
	 * It hold all calculated stuff from agregates/group_concat until finalization.
	 */
	bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey,
			bool bGrouped, bool bNewSet, bool bTailFinalized=false )
	{

#ifndef NDEBUG
		++m_ipushed;
		DBG << "PushEx: tag" << tEntry.m_iTag << ",g" << uGroupKey << ": pushed" << m_ipushed
			<< " g" << bGrouped << " n" << bNewSet;
#endif

		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			this->m_dJustPopped.Resize ( 0 );
		}

		// place elem into the set
		auto iNew = AllocateMatch ();
		CSphMatch & tNew = m_dData[iNew];

		// if such group already hashed
		int * pGroupIdx = m_hGroup2Index.Find ( uGroupKey );
		if ( pGroupIdx )
			return PushAlreadyHashed ( pGroupIdx, iNew, tEntry, uGroupKey, bGrouped, bNewSet, bTailFinalized);

		// match came from MoveTo of another sorter, it is tail and it has no group here (m.b. it is already
		// deleted during finalization as one of worst). Just discard the whole group in the case.
		if ( bTailFinalized && !bGrouped )
		{
			DeallocateMatch ( iNew );
			return false;
		}

		m_pSchema->CloneMatch ( tNew, tEntry ); // fixme! check if essential data cloned
		//	else
		//			this->m_tPregroup.CloneWithoutAggrs ( tNew, tEntry );
		//			this->m_tPregroup.CopyAggrs ( tNew, tEntry );


		// submit actual distinct value in all cases
		if_const ( DISTINCT )
			UpdateDistinct ( tNew, uGroupKey, bGrouped );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_tRowID;

		this->m_dIData[iNew] = iNew; // new head - points to self (0-ring)
		Verify ( m_hGroup2Index.Add ( uGroupKey, iNew ));
		++m_iTotal;

		if ( bGrouped )
		{
			m_uLastGroupKey = uGroupKey;
			AggrUngroup ( m_dData[iNew] );
		} else
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );
		}
		return true;
	}

private:

	// surely give place for a match (do vacuum-cleaning, if there is no place)
	inline int AllocateMatch ()
	{
		auto iPlace = TryAllocateMatch ();
		if ( iPlace<0 )
		{
			VacuumClean ();
			iPlace = TryAllocateMatch ();
		}
		assert ( iPlace>=0 && iPlace<m_iSize );
		DBG << "allocated: " << iPlace;
		return iPlace;
	}

	// return match and free it's dataptrs
	inline void FreeMatch ( int iElem, bool bNotify ) // fixme! intersects with parent by name
	{
		FreeMatchPtrs ( iElem, bNotify );
		DeallocateMatch ( iElem );
	}

	inline int TryAllocateMatch ()
	{
		if ( m_iUsed==m_iSize )
			return -1; // no more place..

		++m_iUsed;
		auto iElem = m_iFree;
		if ( iElem<m_iStorageSolidFrom )
			m_iFree = this->m_dIData[iElem];
		else {
			++m_iFree;
			m_iStorageSolidFrom = m_iFree;
		}
		return iElem;
	}

	inline void DeallocateMatch (int iElem)
	{
		--m_iUsed;
		this->m_dIData[iElem] = m_iFree; // put to chain
		m_iFree = iElem;
		assert ( m_iFree >=0 );
	}

	// return length of the matches chain (-1 terminated)
	int ChainLen(int iPos)
	{
		int iChainLen = 1;
		for ( int i = this->m_dIData[iPos]; i!=iPos; i = this->m_dIData[i] )
			++iChainLen;
		return iChainLen;
	}

	// add new match into the chain. Aggregates are relaxed and not managed till finalize
	/*
	 * chain of the matches is actually ring of integers. Each one points to the coherent
	 * match in the storage, and simultaneously next member of the ring.
	 * We can iterate over the chain starting from the head and looking until same index met again.
	 */
	void AddToChain ( int iNew, const CSphMatch & tEntry, int iHead )
	{
		CSphMatch & tNew = m_dData[iNew];
		this->m_tPregroup.CloneWithoutAggrs ( tNew, tEntry );
		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_tRowID;

		// put after the head
		auto iPrevChain = this->m_dIData[iHead];
		this->m_dIData[iNew] = iPrevChain;
		this->m_dIData[iHead] = iNew;
	}

	// add entry to existing group
	/*
	 * If group is not full, and new match is less then head, it will replace the head.
	 * calculated stuff will be moved and adopted by this new replacement.
	 * If group is full, and new match is less then head, it will be early rejected.
	 * In all other cases new match will be inserted into the group right after head
	 */
	bool PushAlreadyHashed ( int * pHead, int iNew, const CSphMatch & tEntry,
			const SphGroupKey_t uGroupKey, bool bGrouped, bool bNewSet, bool bTailFinalized )
	{
		int & iHead = *pHead;

		assert ( m_dData[iHead].GetAttr ( m_tLocGroupby )==uGroupKey );
		assert ( m_dData[iHead].m_pDynamic[-1]==tEntry.m_pDynamic[-1] );
		DBG << "existing " << m_dData[iHead].m_iTag << "," << uGroupKey
			<< " m_pDynamic: " << m_dData[iHead].m_pDynamic;

		// check if we need to push the match at all
		if ( m_tSubSorter.MatchIsGreater ( tEntry, m_dData[iHead] ) )
			AddToChain ( iNew, tEntry, iHead ); // always add; bad will be filtered later in gc
		else if ( ChainLen ( iHead )>=m_iGLimit ) // less then worst, drop it
			DeallocateMatch ( iNew );
		else {
			AddToChain ( iNew, tEntry, iHead );
			this->m_tPregroup.MoveAggrs ( m_dData[iNew], m_dData[iHead] );
			*pHead = iNew;
		}

		auto & tHeadMatch = m_dData[iHead];
		// submit actual distinct value in all cases
		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, uGroupKey, bGrouped );

		// update group-wide counters
		auto & tLocCount = m_tLocCount;
		if ( bGrouped ) {
			// it's already grouped match
			// sum grouped matches count
			if ( bNewSet || uGroupKey!=m_uLastGroupKey ) {
				tHeadMatch.AddCounterAttr ( tLocCount, tEntry );
				m_uLastGroupKey = uGroupKey;
				bNewSet = true;
			}
		} else if ( !bTailFinalized ) {
			// it's a simple match
			// increase grouped matches count
			tHeadMatch.AddCounterScalar ( tLocCount, 1 );
			bNewSet = true;
		}

		// update aggregates
		if ( bNewSet )
			AggrUpdate ( tHeadMatch, tEntry, bGrouped );

		// since it is dupe (i.e. such group is already pushed) - return false;
		return false;
	}

	enum class Avg_e { FINALIZE, UNGROUP };
	void CalcAvg ( Avg_e eGroup )
	{
		if ( this->m_dAvgs.IsEmpty() )
			return;

		int64_t i = 0;
		if ( eGroup==Avg_e::FINALIZE )
			for ( auto tData = m_hGroup2Index.Iterate ( &i ); tData.second; tData = m_hGroup2Index.Iterate ( &i ))
				m_dAvgs.Apply ( [this, &tData] ( IAggrFunc * pAvg ) {
					pAvg->Finalize ( m_dData[*tData.second] );
				});
		else
			for ( auto tData = m_hGroup2Index.Iterate ( &i ); tData.second; tData = m_hGroup2Index.Iterate ( &i ))
				m_dAvgs.Apply ( [this, &tData] ( IAggrFunc * pAvg ) {
					pAvg->Ungroup ( m_dData[*tData.second] );
				});
	}

	void BinaryPartitionTail ( VecTraits_T<int>& dData, int iBound )
	{
		--iBound;
		int iPivot = dData[iBound];
		int a = 0;
		int b = dData.GetLength ()-1;
		while (true) {
			int i = a;
			int j = b;
			while (i<=j) {
				while ( m_tSubSorter.IsLess ( dData[i], iPivot )) ++i;
				while ( m_tSubSorter.IsLess ( iPivot, dData[j] )) --j;
				if ( i<=j ) ::Swap ( dData[i++], dData[j--] );
			}
			if ( iBound==j )
				break;

			if ( iBound<j )
				b = j;  // too many elems aquired; continue with left part
			else
				a = i;  // too less elems aquired; continue with right part
			iPivot = dData[( a+b ) / 2];
		}
	}

	CSphVector<int> GetAllHeads()
	{
		CSphVector<int> dAllHeads;
		dAllHeads.Reserve ( m_hGroup2Index.GetLength ());
		int64_t i = 0;
		for ( auto tData = m_hGroup2Index.Iterate ( &i ); tData.second; tData = m_hGroup2Index.Iterate ( &i ))
			dAllHeads.Add ( *tData.second );
		return dAllHeads;
	}

	// free place for new matches
	void VacuumClean()
	{
		auto iLimit = m_iLimit * GROUPBY_FACTOR / 2;

		// first try to cut out too long tails
		int iSize = 0;
		int64_t i = 0;
		for ( auto tData = m_hGroup2Index.Iterate(&i); tData.second; tData = m_hGroup2Index.Iterate(&i) )
			iSize += VacuumTail ( tData.second, m_iGLimit );

		// if we reached the limit now - bail, no need to free more.
		if ( iSize<=iLimit )
			return;

		// if we're here, just vacuuming tails wasn't effective enough and some deeper cleaning necessary
		SortThenVacuumWorstHeads ( iLimit );
	}

	// final pass before iface finalize/flatten - cut worst, sort everything
	void FinalizeChains()
	{
		if ( m_bFinalized )
			return;
		m_bFinalized = true;

		int64_t i = 0;
		for ( auto tData = m_hGroup2Index.Iterate(&i); tData.second; tData = m_hGroup2Index.Iterate(&i) )
			VacuumTail ( tData.second, m_iGLimit, Stage_e::FINAL );

		// Continue by cut out whole groups
		SortThenVacuumWorstHeads ( m_iLimit, Stage_e::FINAL ); // false since it is already sorted

		// also free matches in the chain were cleared with FreeDataPtrs, but *now* we also need to free their dynamics
		// otherwize in d-tr FreDataPtr on non-zero dynamics will be called again with propably another schema and crash
		auto iFree = m_iFree;
		for ( auto iElem = TryAllocateMatch (); iElem>=0; iElem = TryAllocateMatch () )
			m_dData[iElem].ResetDynamic ();
		m_iFree = iFree;
	}

	/*
	 * Here we
	 * 1) Cut off very last head if it would exceed the limit.
	 * 1) Copy all calculated stuff (aggr attributes) from head match to every other match of a group
	 * 2) Sort group in decreasing order, and then shift the ring ahead to 1 match.
	 * That is necessary since head is worst match, and next after it is the best one (since just sorted)
	 * Since it is ring, by moving ahead we will have 1-st match the best, last - the worst.
	 */
	void PrepareForExport()
	{
		VacuumTail ( &m_dFinalizedHeads.Last(), m_iLastGroupCutoff, Stage_e::FINAL );
		auto dAggrs = GetAggregatesWithoutAvgs ();
		for ( auto& iHead : m_dFinalizedHeads ) {
			for ( auto * pAggr : dAggrs )
				pAggr->Finalize ( m_dData[iHead] );
			PropagateAggregates ( iHead );
			iHead = this->m_dIData[iHead]; // shift
		}
	}

	void PropagateAggregates ( int iHead )
	{
		for ( auto i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
			this->m_tPregroup.CopyAggrs ( m_dData[i], m_dData[iHead] );
	}

	// at collect stage we don't need to strictly sort matches inside groups,
	// but we need to track pushed/deleted matches.
	// at finalize stage, in opposite, no tracking need, but matches must be sorted.
	enum class Stage_e { COLLECT, FINAL };

	// sorts by next-to-worst element in the chain
	struct FinalGroupSorter_t
	{
		const GroupSorter_fn<COMPGROUP> &	m_tGroupSorter;
		const CSphTightVector<int> &		m_dIData;

		FinalGroupSorter_t ( const GroupSorter_fn<COMPGROUP> & tSorter, const CSphTightVector<int> & dIData )
			: m_tGroupSorter ( tSorter )
			, m_dIData ( dIData )
		{}

		bool IsLess ( int a, int b ) const
		{
			return m_tGroupSorter.IsLess ( m_dIData[a], m_dIData[b] );
		}
	};

	// full clean - sort the groups, then iterate on them until iLimit elems counted. Cut out the rest.
	// if last group is not fit into rest of iLimit, it still keeped whole, no fraction performed over it.
	// returns desired length of the last chain to make the limit hard ( 1..m_iGLimit )
	void SortThenVacuumWorstHeads ( int iSoftLimit, Stage_e eStage = Stage_e::COLLECT )
	{
		m_dFinalizedHeads = GetAllHeads();
		CalcAvg ( Avg_e::FINALIZE );

		// in this final sort we need to keep the heads but to sort by next-to-head element (which is the best in group)
		FinalGroupSorter_t tFinalSorter ( m_tGroupSorter, this->m_dIData );
		m_dFinalizedHeads.Sort ( tFinalSorter );

		int iRetainMatches = 0;
		CSphVector<SphGroupKey_t> dRemovedHeads; // to remove distinct

		// delete worst heads
		ARRAY_FOREACH ( i, m_dFinalizedHeads )
			if ( iSoftLimit > iRetainMatches )
				iRetainMatches += ChainLen ( m_dFinalizedHeads[i] );
			else {
				 // all quota exceeded, the rest just to be cut totally
				auto iRemoved = DeleteChain ( m_dFinalizedHeads[i], eStage==Stage_e::COLLECT );
				if_const ( DISTINCT )
					dRemovedHeads.Add( iRemoved );
				m_dFinalizedHeads.RemoveFast ( i-- );
			}

		// discard removed distincts
		if_const ( DISTINCT )
			RemoveDistinct ( dRemovedHeads );

		CalcAvg ( Avg_e::UNGROUP );
		m_iLastGroupCutoff = m_iGLimit+iSoftLimit-iRetainMatches;
	}

	// for given chain throw out worst elems to fit in iLimit quota.
	// Returns length of the chain
	int VacuumTail ( int* pHead, int iLimit, Stage_e eStage = Stage_e::COLLECT )
	{
		assert ( iLimit>0 );
		CSphVector<int> dChain;
		dChain.Add ( *pHead );
		for ( auto i = this->m_dIData[*pHead]; i!=*pHead; i = this->m_dIData[i] )
			dChain.Add ( i );

		if ( dChain.GetLength()==1 )
			return 1; // fast over

		auto dWorstTail = dChain.Slice ( iLimit );

		// if no sort necessary and limit not exceeded - nothing to do
		if ( eStage==Stage_e::COLLECT && dWorstTail.IsEmpty() )
			return dChain.GetLength();

		// chain need to be shortened
		if ( !dWorstTail.IsEmpty () )
		{
			BinaryPartitionTail ( dChain, iLimit );
			dChain.Resize ( iLimit );
		}

		// sort if necessary and ensure last elem of chain is the worst one
		if ( eStage==Stage_e::FINAL ) {
			dChain.Sort( m_tSubSorter ); // sorted in reverse order, so the worst match here is the last one.
			iLimit = dChain.GetLength();
		} else {
			assert ( dChain.GetLength ()==iLimit );
			// not sorted, need to find worst match for new head
			int iWorst = 0;
			for (int i=1; i<iLimit; ++i) {
				if ( m_tSubSorter.IsLess ( dChain[iWorst], dChain[i] ))
					iWorst = i;
			}
			::Swap ( dChain[iWorst], dChain[iLimit-1] );
		}

		auto iNewHead = dChain.Last ();

		// move calculated aggregates to the new head
		if ( iNewHead!=*pHead ) {
			this->m_tPregroup.MoveAggrs ( m_dData[iNewHead], m_dData[*pHead] );
			*pHead = iNewHead;
		}

		// now we can safely free worst matches
		for ( auto iWorst : dWorstTail )
			FreeMatch ( iWorst, eStage==Stage_e::COLLECT );

		// recreate the chain. It is actually ring, and external hash points to the minimal elem
		this->m_dIData[iNewHead] = dChain[0]; // head points to begin of chain
		for ( int i = 0; i<iLimit-1; ++i ) // each elem points to the next, last again to head
			this->m_dIData[dChain[i]] = dChain[i+1];
		return iLimit;
	}

	// delete whole chain (and remove from hash also).
	SphGroupKey_t DeleteChain ( int iPos, bool bNotify )
	{
		SphGroupKey_t uGroupKey = m_dData[iPos].GetAttr ( m_tLocGroupby );
		m_hGroup2Index.Delete ( uGroupKey );
		int iNext = this->m_dIData[iPos];
		FreeMatch ( iPos, bNotify );
		for ( auto i = iNext; i!=iPos; i = iNext ) {
			iNext = this->m_dIData[i];
			FreeMatch ( i, bNotify );
		}

		return uGroupKey;
	}

	/// count distinct values if necessary
	void CountDistinct ()
	{
		if_const ( DISTINCT )
			Distinct ( [this] ( SphGroupKey_t uGroup )->CSphMatch *
			{
				auto pIdx = m_hGroup2Index.Find ( uGroup );
				return pIdx? &m_dData[*pIdx] : nullptr;
			});
	}
};


/// match sorter with k-buffering and group-by for MVAs
template < typename T, typename MVA >
class MVAGroupSorter_T : public T
{
public:
	/// ctor
	MVAGroupSorter_T ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: T ( pComp, pQuery, tSettings )
	{
		this->m_pGrouper->GetLocator ( m_tMvaLocator );
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		if ( !T::GetBlobPool() )
			return false;

		int iLengthBytes = 0;
		const BYTE * pMva = sphGetBlobAttr ( tEntry, m_tMvaLocator, T::GetBlobPool(), iLengthBytes );
		int nValues = iLengthBytes / sizeof(MVA);
		const MVA * pValues = (const MVA*)pMva;

		bool bRes = false;
		for ( int i = 0; i<nValues; i++ )
		{
			SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( pValues[i] );
			bRes |= this->PushEx ( tEntry, uGroupkey, false, false );
		}

		return bRes;
	}

	/// add pre-grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tLocGroupby ), true, bNewSet );
	}

protected:
	CSphAttrLocator		m_tMvaLocator;
};

template < typename COMPGROUP, typename MVA, bool DISTINCT, bool NOTIFICATIONS >
struct MvaGroupSorter_c : public MVAGroupSorter_T < CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >
{
	using BASE=MVAGroupSorter_T < CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >;
	using MYTYPE = MvaGroupSorter_c < COMPGROUP, MVA, DISTINCT, NOTIFICATIONS >;

	/// ctor
	FWD_BASECTOR( MvaGroupSorter_c );

	ISphMatchSorter * Clone () const final
	{
		CSphQuery dFoo;
		dFoo.m_iMaxMatches = this->m_iLimit;
		dFoo.m_eGroupFunc = this->m_eGroupBy;
		auto pClone = new MYTYPE ( this->m_tSubSorter.GetComparator (), &dFoo, *this );
		this->CloneKBufferGroupSorter ( pClone );
		pClone->m_tMvaLocator = this->m_tMvaLocator;
		return pClone;
	}
};


template < typename COMPGROUP, typename MVA, bool DISTINCT, bool NOTIFICATIONS >
struct MvaNGroupSorter_c : public MVAGroupSorter_T < CSphKBufferNGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >
{
	using BASE = MVAGroupSorter_T < CSphKBufferNGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >;
	using MYTYPE = MvaNGroupSorter_c < COMPGROUP, MVA, DISTINCT, NOTIFICATIONS >;

	/// ctor
	FWD_BASECTOR( MvaNGroupSorter_c );

	ISphMatchSorter * Clone () const final
	{
		CSphQuery dFoo;
		dFoo.m_iGroupbyLimit = this->m_iGLimit;
		dFoo.m_iMaxMatches = this->m_iLimit;
		dFoo.m_eGroupFunc = this->m_eGroupBy;
		auto pClone = new MYTYPE ( this->m_tSubSorter.GetComparator (), &dFoo, *this );
		this->CloneKBufferGroupSorter ( pClone );
		pClone->m_tMvaLocator = this->m_tMvaLocator;
		return pClone;
	}
};


/// match sorter with k-buffering and group-by for JSON arrays
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferJsonGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >
{
public:
	using BASE = CSphKBufferGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	using MYTYPE = CSphKBufferJsonGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS>;

	// since we inherit from template, we need to write boring 'using' block
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	using KBufferGroupSorter::m_eGroupBy;
	using KBufferGroupSorter::m_iLimit;
	using KBufferGroupSorter::m_tSubSorter;
	using KBufferGroupSorter::CloneKBufferGroupSorter;

	/// ctor
	FWD_BASECTOR( CSphKBufferJsonGroupSorter );

	/// add entry to the queue
	bool Push ( const CSphMatch & tMatch ) override
	{
		SphGroupKey_t uGroupKey = this->m_pGrouper->KeyFromMatch ( tMatch );

		auto iValue = (int64_t)uGroupKey;
		CSphGrouper * pGrouper = this->m_pGrouper;
		const BYTE * pBlobPool = ((CSphGrouperJsonField*)pGrouper)->GetBlobPool();

		return PushJsonField ( iValue, pBlobPool, [this, &tMatch]( SphAttr_t * pAttr, SphGroupKey_t uGroupKey )
			{
				return this->PushEx ( tMatch, uGroupKey, false, false, pAttr );
			}
		);
	}

	/// add pre-grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tLocGroupby ), true, bNewSet );
	}

	ISphMatchSorter * Clone () const final
	{
		CSphQuery dFoo;
		dFoo.m_iMaxMatches = m_iLimit;
		dFoo.m_eGroupFunc = m_eGroupBy;
		auto pClone = new MYTYPE ( m_tSubSorter.GetComparator (), &dFoo, *this );
		CloneKBufferGroupSorter ( pClone );
		return pClone;
	}
};


/// implicit group-by sorter
/// invoked when no 'group-by', but count(*) or count(distinct attr) are in game
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphImplicitGroupSorter final : public ISphMatchSorter, ISphNoncopyable, protected BaseGroupSorter_c
{
	using MYTYPE = CSphImplicitGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS>;
protected:
	CSphMatch		m_tData;
	bool			m_bDataInitialized = false;

	CSphVector<SphUngroupedValue_t>	m_dUniq;

public:
	/// ctor
	CSphImplicitGroupSorter ( const ISphMatchComparator * DEBUGARG(pComp), const CSphQuery *,
			const CSphGroupSorterSettings & tSettings )
		: BaseGroupSorter_c ( tSettings )
	{
		assert ( !DISTINCT || tSettings.m_tDistinctAttr.m_iBitOffset>=0 );
		assert ( !pComp );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);

		if_const ( DISTINCT )
			m_dUniq.Reserve ( 16384 );
		m_iMatchCapacity = 1;
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final
	{
		if ( m_pSchema )
		{
			FixupLocators ( m_pSchema, pSchema );
			m_tPregroup.ResetAttrs ();
			m_dAggregates.Apply ( [] ( IAggrFunc * pAggr ) {SafeDelete ( pAggr ); } );
			m_dAggregates.Resize ( 0 );
		}
		ISphMatchSorter::SetSchema ( pSchema, bRemapCmp );
		SetupBaseGrouper<DISTINCT> ( pSchema );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const final
	{
		return true;
	}

	/// set blob pool pointer (for string+groupby sorters)
	void SetBlobPool ( const BYTE * pBlobPool ) final
	{
		BlobPool_c::SetBlobPool ( pBlobPool );
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		return PushEx ( tEntry, false );
	}

	/// add grouped entry to the queue. bNewSet indicates the beginning of resultset returned by an agent.
	bool PushGrouped ( const CSphMatch & tEntry, bool ) final
	{
		return PushEx ( tEntry, true );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) final
	{
		assert ( m_bDataInitialized );

		CountDistinct ();

		for ( auto * pAggregate : m_dAggregates )
			pAggregate->Finalize ( m_tData );

		int iCopied = 0;
		if ( EvalHAVING ( m_tData ) )
		{
			iCopied = 1;
			Swap ( *pTo, m_tData );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
		} else
		{
			m_pSchema->FreeDataPtrs ( m_tData );
			m_tData.ResetDynamic ();
		}

		m_iTotal = 0;
		m_bDataInitialized = false;

		if_const ( DISTINCT )
			m_dUniq.Resize(0);

		return iCopied;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( ISphMatchProcessor & tProcessor, bool ) final
	{
		if ( !GetLength() )
			return;

		tProcessor.Process ( &m_tData );
	}

	/// get entries count
	int GetLength () const final
	{
		return m_bDataInitialized ? 1 : 0;
	}

	bool CanBeCloned () const final
	{
		return !DISTINCT;
	}

	// TODO! test.
	ISphMatchSorter * Clone () const final
	{
		auto pClone = new MYTYPE ( nullptr, nullptr, *this );
		CloneTo ( pClone );
		pClone->SetupBaseGrouperWrp (m_pSchema);
		return pClone;
	}

	// FIXME! test CSphImplicitGroupSorter
	void MoveTo ( ISphMatchSorter * pRhs ) final
	{
		if (!m_bDataInitialized)
			return;

		auto& dRhs = *(MYTYPE *) pRhs;
		if ( !dRhs.m_bDataInitialized )
		{
			// ISphMatchSorter
			::Swap ( m_iTotal, dRhs.m_iTotal );
			::Swap ( m_tData, dRhs.m_tData );
			::Swap ( m_bDataInitialized, dRhs.m_bDataInitialized );
			m_dUniq.SwapData ( dRhs.m_dUniq );
			return;
		}

		// other step is a bit tricky:
		// we just can't add current count uniq to final; need to append m_dUniq instead,
		// so that final flatten will calculate real uniq count.
		dRhs.AddCount ( m_tData );
		dRhs.UpdateAggregates ( m_tData );
		dRhs.CheckReplaceEntry ( m_tData );
		if_const ( DISTINCT )
			dRhs.UpdateDistinct ( m_tData );
	}

private:
	inline void SetupBaseGrouperWrp ( ISphSchema * pSchema )
	{
		SetupBaseGrouper<DISTINCT> ( pSchema );
	}

	void AddCount ( const CSphMatch & tEntry )
	{
		m_tData.AddCounterAttr ( m_tLocCount, tEntry );
	}

	void UpdateAggregates ( const CSphMatch & tEntry, bool bGrouped = true )
	{
		AggrUpdate ( m_tData, tEntry, bGrouped );
	}

	// if new entry is more relevant, update from it
	void CheckReplaceEntry ( const CSphMatch & tEntry )
	{
		if ( tEntry.m_tRowID<m_tData.m_tRowID )
		{
			if_const ( NOTIFICATIONS )
			{
				m_iJustPushed = tEntry.m_tRowID;
				m_dJustPopped.Add ( m_tData.m_tRowID );
			}
			m_tPregroup.CloneKeepingAggrs ( m_tData, tEntry );
		}
	}

	// submit actual distinct value in all cases
	void UpdateDistinct ( const CSphMatch & tEntry, bool bGrouped = true )
	{
		int iCount = 1;
		if ( bGrouped )
			iCount = (int) tEntry.GetAttr ( m_tLocDistinct );

		AddDistinctKeys ( tEntry, m_tDistinctAttr, m_eDistinctAttr, GetBlobPool (), [this, iCount] ( SphAttr_t b )
			{ this->m_dUniq.Add ( std::make_pair ( b, iCount ) ); } );
	}

	/// add entry to the queue
	bool PushEx ( const CSphMatch & tEntry, bool bGrouped )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize(0);
		}

		if ( m_bDataInitialized )
		{
			assert ( m_tData.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				AddCount ( tEntry );
			} else
			{
				// it's a simple match
				// increase grouped matches count
				m_tData.AddCounterScalar ( m_tLocCount, 1 );
			}

			// update aggregates
			UpdateAggregates ( tEntry, bGrouped );
			CheckReplaceEntry ( tEntry );
		}

		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, bGrouped );

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( m_bDataInitialized )
			return false;

		// add first
		m_pSchema->CloneMatch ( m_tData, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = m_tData.m_tRowID;

		if ( !bGrouped )
		{
			m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
			m_tData.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				m_tData.SetAttr ( m_tLocDistinct, 0 );
		} else
			AggrUngroup ( m_tData );

		m_bDataInitialized = true;
		++m_iTotal;
		return true;
	}

	/// count distinct values if necessary
	void CountDistinct ()
	{
		if_const ( DISTINCT )
		{
			assert ( m_bDataInitialized );

			m_dUniq.Sort ();
			int iCount = m_dUniq[0].second;

			for ( int i = 1, iLen = m_dUniq.GetLength (); i<iLen; ++i )
			{
				if ( m_dUniq[i-1]==m_dUniq[i] )
					continue;
				iCount += m_dUniq[i].second;
			}

			m_tData.SetAttr ( m_tLocDistinct, iCount );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// PLAIN SORTING FUNCTORS
//////////////////////////////////////////////////////////////////////////

static bool IsDocidLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState )
{
	DocID_t tDocidA, tDocidB;
	
	switch ( tState.m_eKeypart[1] )
	{
		case SPH_KEYPART_DOCID_S:
			tDocidA = sphGetDocID ( a.m_pStatic );
			tDocidB = sphGetDocID ( b.m_pStatic );
			break;

		case SPH_KEYPART_DOCID_D:
			tDocidA = sphGetDocID ( a.m_pDynamic );
			tDocidB = sphGetDocID ( b.m_pDynamic );
			break;

		default:
			return a.m_tRowID > b.m_tRowID;
	}

	if ( tDocidA!=tDocidB )
		return tDocidA > tDocidB;

	return a.m_tRowID > b.m_tRowID;
}

/// match sorter
struct MatchRelevanceLt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState )
	{
		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return IsDocidLess ( a, b, tState );
	}
};


/// match sorter
struct MatchAttrLt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( t.m_eKeypart[0]!=SPH_KEYPART_STRING )
		{
			SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
			SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
			if ( aa!=bb )
				return aa<bb;
		} else
		{
			int iCmp = t.CmpStrings ( a, b, 0 );
			if ( iCmp!=0 )
				return iCmp<0;
		}

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return IsDocidLess ( a, b, t );
	}
};


/// match sorter
struct MatchAttrGt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( t.m_eKeypart[0]!=SPH_KEYPART_STRING )
		{
			SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
			SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
			if ( aa!=bb )
				return aa>bb;
		} else
		{
			int iCmp = t.CmpStrings ( a, b, 0 );
			if ( iCmp!=0 )
				return iCmp>0;
		}

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return IsDocidLess ( a, b, t );
	}
};


/// match sorter
struct MatchTimeSegments_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
		SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
		int iA = GetSegment ( aa, t.m_iNow );
		int iB = GetSegment ( bb, t.m_iNow );
		if ( iA!=iB )
			return iA > iB;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		if ( aa!=bb )
			return aa<bb;

		return IsDocidLess ( a, b, t );
	}

protected:
	static inline int GetSegment ( SphAttr_t iStamp, SphAttr_t iNow )
	{
		if ( iStamp>=iNow-3600 ) return 0; // last hour
		if ( iStamp>=iNow-24*3600 ) return 1; // last day
		if ( iStamp>=iNow-7*24*3600 ) return 2; // last week
		if ( iStamp>=iNow-30*24*3600 ) return 3; // last month
		if ( iStamp>=iNow-90*24*3600 ) return 4; // last 3 months
		return 5; // everything else
	}
};


/// match sorter
struct MatchExpr_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		float aa = a.GetAttrFloat ( t.m_tLocator[0] ); // FIXME! OPTIMIZE!!! simplified (dword-granular) getter could be used here
		float bb = b.GetAttrFloat ( t.m_tLocator[0] );
		if ( aa!=bb )
			return aa<bb;

		return IsDocidLess ( a, b, t );
	}
};

/////////////////////////////////////////////////////////////////////////////

#define SPH_TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (t.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ROWID:		SPH_TEST_PAIR ( a.m_tRowID, b.m_tRowID, _idx ); break; \
		case SPH_KEYPART_DOCID_S:	return sphGetDocID(a.m_pStatic) > sphGetDocID(b.m_pStatic); break; \
		case SPH_KEYPART_DOCID_D:	return sphGetDocID(a.m_pDynamic) > sphGetDocID(b.m_pDynamic); break; \
		case SPH_KEYPART_WEIGHT:	SPH_TEST_PAIR ( a.m_iWeight, b.m_iWeight, _idx ); break; \
		case SPH_KEYPART_INT: \
		{ \
			register SphAttr_t aa = a.GetAttr ( t.m_tLocator[_idx] ); \
			register SphAttr_t bb = b.GetAttr ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ); \
			break; \
		} \
		case SPH_KEYPART_FLOAT: \
		{ \
			register float aa = a.GetAttrFloat ( t.m_tLocator[_idx] ); \
			register float bb = b.GetAttrFloat ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		case SPH_KEYPART_STRINGPTR: \
		case SPH_KEYPART_STRING: \
		{ \
			int iCmp = t.CmpStrings ( a, b, _idx ); \
			if ( iCmp!=0 ) \
				return ( ( t.m_uAttrDesc >> (_idx) ) & 1 ) ^ ( iCmp>0 ); \
			break; \
		} \
	}


struct MatchGeneric1_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric2_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric3_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric4_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric5_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		SPH_TEST_KEYPART(4);
		return a.m_tRowID>b.m_tRowID;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORT CLAUSE PARSER
//////////////////////////////////////////////////////////////////////////

static const int MAX_SORT_FIELDS = 5; // MUST be in sync with CSphMatchComparatorState::m_iAttr


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

			// stop checking on any non space char outside sequence or sequence end
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


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_FLOAT:
			return SPH_KEYPART_FLOAT;

		case SPH_ATTR_STRING:
			return SPH_KEYPART_STRING;

		case SPH_ATTR_JSON:
		case SPH_ATTR_JSON_PTR:
		case SPH_ATTR_JSON_FIELD:
		case SPH_ATTR_JSON_FIELD_PTR:
		case SPH_ATTR_STRINGPTR:
			return SPH_KEYPART_STRINGPTR;

		default:
			return SPH_KEYPART_INT;
	}
}


static bool SetupSortByDocID ( const ISphSchema & tSchema, CSphMatchComparatorState & tState, const CSphQuery * pQuery, bool bInitialSearch )
{
	const CSphColumnInfo * pDocID = tSchema.GetAttr ( sphGetDocidName() );
	assert ( pDocID && pDocID->m_eAttrType==SPH_ATTR_BIGINT );

	if ( pQuery && pQuery->m_eRanker==SPH_RANK_NONE && tState.m_eKeypart[0]==SPH_KEYPART_WEIGHT && bInitialSearch )
	{
		// if it's the first local search and there's no ranker, use sort by rowid
		// if we're merging results, sort by weight/docid
		tState.m_eKeypart[0] = SPH_KEYPART_ROWID;
		tState.m_uAttrDesc = 0;
		return false;
	}
	else
	{
		if ( pDocID->m_tLocator.m_iBitOffset==0 )
			tState.m_eKeypart[1] = pDocID->m_tLocator.m_bDynamic ? SPH_KEYPART_DOCID_D : SPH_KEYPART_DOCID_S;	// fastpath, static or dynamic docid at offset 0
		else
			tState.m_eKeypart[1] = SPH_KEYPART_INT;	// generic path, fetch docid as a plain attribute
		
		tState.m_tLocator[1] = pDocID->m_tLocator;
		tState.m_dAttrs[1] = tSchema.GetAttrIndex ( sphGetDocidName() );
		return true;
	}
}


ESortClauseParseResult sphParseSortClause ( const CSphQuery * pQuery, const char * sClause,
		const ISphSchema & tSchema, ESphSortFunc & eFunc, CSphMatchComparatorState & tState,
		bool bComputeItems, CSphString & sError )
{
	for ( auto &dAttr : tState.m_dAttrs )
		dAttr = -1;

	// mini parser
	SortClauseTokenizer_t tTok ( sClause );

	bool bField = false; // whether i'm expecting field name or sort order
	int iField = 0;

	for ( const char * pTok=tTok.GetToken(); pTok; pTok=tTok.GetToken() )
	{
		bField = !bField;

		// special case, sort by random
		if ( iField==0 && bField && strcmp ( pTok, "@random" )==0 )
			return SORT_CLAUSE_RANDOM;

		// handle sort order
		if ( !bField )
		{
			// check
			if ( strcmp ( pTok, "desc" ) && strcmp ( pTok, "asc" ) )
			{
				sError.SetSprintf ( "invalid sorting order '%s'", pTok );
				return SORT_CLAUSE_ERROR;
			}

			// set
			if ( !strcmp ( pTok, "desc" ) )
				tState.m_uAttrDesc |= ( 1<<iField );

			iField++;
			continue;
		}

		// handle attribute name
		if ( iField==MAX_SORT_FIELDS )
		{
			sError.SetSprintf ( "too many sort-by attributes; maximum count is %d", MAX_SORT_FIELDS );
			return SORT_CLAUSE_ERROR;
		}

		if ( !strcasecmp ( pTok, "@relevance" )
			|| !strcasecmp ( pTok, "@rank" )
			|| !strcasecmp ( pTok, "@weight" )
			|| !strcasecmp ( pTok, "weight()" ) )
		{
			tState.m_eKeypart[iField] = SPH_KEYPART_WEIGHT;

		} else
		{
			if ( !strcasecmp ( pTok, "@group" ) )
				pTok = "@groupby";
			else if ( !strcasecmp ( pTok, "count(*)" ) )
				pTok = "@count";
			else if ( !strcasecmp ( pTok, "facet()" ) )
				pTok = "@groupby"; // facet() is essentially a @groupby alias
			else if ( strcasecmp ( pTok, "count" )>=0 && tTok.IsSparseCount ( pTok + sizeof ( "count" ) - 1 ) ) // epression count(*) with various spaces
				pTok = "@count";


			// try to lookup plain attr in sorter schema
			int iAttr = tSchema.GetAttrIndex ( pTok );

			// do not order by mva (the result is undefined)			
			if ( iAttr>=0 )
			{
				ESphAttr eAttrType = tSchema.GetAttr(iAttr).m_eAttrType;
				if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET || eAttrType==SPH_ATTR_UINT32SET_PTR || eAttrType==SPH_ATTR_INT64SET_PTR )
				{
					sError.SetSprintf ( "order by MVA is undefined" );
					return SORT_CLAUSE_ERROR;
				}
			}

			// try to lookup aliased count(*) and aliased groupby() in select items
			if ( iAttr<0 )
			{
				ARRAY_FOREACH ( i, pQuery->m_dItems )
				{
					const CSphQueryItem & tItem = pQuery->m_dItems[i];
					if ( !tItem.m_sAlias.cstr() || strcasecmp ( tItem.m_sAlias.cstr(), pTok ) )
						continue;
					if ( tItem.m_sExpr.Begins("@") )
						iAttr = tSchema.GetAttrIndex ( tItem.m_sExpr.cstr() );
					else if ( tItem.m_sExpr=="count(*)" )
						iAttr = tSchema.GetAttrIndex ( "@count" );
					else if ( tItem.m_sExpr=="groupby()" )
					{
						CSphString sGroupJson = SortJsonInternalSet ( pQuery->m_sGroupBy );
						iAttr = tSchema.GetAttrIndex ( sGroupJson.cstr() );
						// try numeric group by
						if ( iAttr<0 )
							iAttr = tSchema.GetAttrIndex ( "@groupby" );
					}
					break; // break in any case; because we did match the alias
				}
			}

			// try JSON attribute and use JSON attribute instead of JSON field
			bool bJsonAttr = false;

			if ( iAttr>=0 )
			{
				ESphAttr eAttrType = tSchema.GetAttr(iAttr).m_eAttrType;
				if ( eAttrType==SPH_ATTR_JSON_FIELD || eAttrType==SPH_ATTR_JSON_FIELD_PTR || eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_PTR )
					bJsonAttr = true;
			}

			if ( iAttr<0 || bJsonAttr )
			{
				if ( iAttr>=0 )
				{
					// aliased SPH_ATTR_JSON_FIELD, reuse existing expression
					const CSphColumnInfo * pAttr = &tSchema.GetAttr(iAttr);
					if ( pAttr->m_pExpr )
						pAttr->m_pExpr->AddRef(); // SetupSortRemap uses refcounted pointer, but does not AddRef() itself, so help it
					tState.m_tSubExpr[iField] = pAttr->m_pExpr;
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, (int) strlen ( pTok ) );

				} else
				{
					CSphString sJsonCol, sJsonKey;
					if ( sphJsonNameSplit ( pTok, &sJsonCol, &sJsonKey ) )
					{
						iAttr = tSchema.GetAttrIndex ( sJsonCol.cstr() );
						if ( iAttr>=0 )
						{
							ExprParseArgs_t tExprArgs;
							tState.m_tSubExpr[iField] = sphExprParse ( pTok, tSchema, sError, tExprArgs );
							tState.m_tSubKeys[iField] = JsonKey_t ( pTok, (int) strlen ( pTok ) );
						}
					}
				}
			}

			// try json conversion functions (integer()/double()/bigint() in the order by clause)
			ESphAttr eAttrType = SPH_ATTR_NONE;
			if ( iAttr<0 )
			{
				ExprParseArgs_t tExprArgs;
				tExprArgs.m_pAttrType = &eAttrType;
				ISphExpr * pExpr = sphExprParse ( pTok, tSchema, sError, tExprArgs );
				if ( pExpr )
				{
					tState.m_tSubExpr[iField] = pExpr;
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, (int) strlen(pTok) );
					tState.m_tSubKeys[iField].m_uMask = 0;
					tState.m_tSubType[iField] = eAttrType;
					iAttr = 0; // will be remapped in SetupSortRemap
				}
			}

			// try precalculated json fields received from agents (prefixed with @int_*)
			if ( iAttr<0 )
			{
				CSphString sName;
				sName.SetSprintf ( "%s%s", g_sIntAttrPrefix, pTok );
				iAttr = tSchema.GetAttrIndex ( sName.cstr() );
			}

			// epic fail
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}

			const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
			tState.m_eKeypart[iField] = Attr2Keypart ( eAttrType!=SPH_ATTR_NONE ? eAttrType : tCol.m_eAttrType );
			tState.m_tLocator[iField] = tCol.m_tLocator;
			tState.m_dAttrs[iField] = iAttr;
		}
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	if ( iField==1 )
	{
		if ( SetupSortByDocID ( tSchema, tState, pQuery, bComputeItems ) )
			iField++;
	}

	switch ( iField )
	{
		case 1:		eFunc = FUNC_GENERIC1; break;
		case 2:		eFunc = FUNC_GENERIC2; break;
		case 3:		eFunc = FUNC_GENERIC3; break;
		case 4:		eFunc = FUNC_GENERIC4; break;
		case 5:		eFunc = FUNC_GENERIC5; break;
		default:	sError.SetSprintf ( "INTERNAL ERROR: %d fields in sphParseSortClause()", iField ); return SORT_CLAUSE_ERROR;
	}
	return SORT_CLAUSE_OK;
}

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING INSTANTIATION
//////////////////////////////////////////////////////////////////////////

#define CREATE_SORTER_4TH(SORTER,COMPGROUP,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS) \
{ \
	if ( SETTINGS.m_bDistinct ) \
	{ \
		if ( HAS_PACKEDFACTORS ) \
			return new SORTER<COMPGROUP,true,true>(COMP,QUERY,SETTINGS); \
		else \
			return new SORTER<COMPGROUP,true,false>(COMP,QUERY,SETTINGS); \
	} else \
	{ \
		if ( HAS_PACKEDFACTORS ) \
			return new SORTER<COMPGROUP,false,true>(COMP,QUERY,SETTINGS); \
		else \
			return new SORTER<COMPGROUP,false,false>(COMP,QUERY,SETTINGS); \
	} \
}

#define CREATE_SORTER_4TH_MVA(SORTER,COMPGROUP,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS) \
{ \
	BYTE uSelector = (tSettings.m_bDistinct?1:0) + (bHasPackedFactors?2:0) + (tSettings.m_bMva64?4:0); \
	switch ( uSelector ) \
	{ \
	case 0:	return new SORTER<COMPGROUP,DWORD,false,false> ( pComp, pQuery, tSettings ); \
	case 1:	return new SORTER<COMPGROUP,DWORD,true,false> ( pComp, pQuery, tSettings ); \
	case 2:	return new SORTER<COMPGROUP,DWORD,false,true> ( pComp, pQuery, tSettings ); \
	case 3:	return new SORTER<COMPGROUP,DWORD,true,true> ( pComp, pQuery, tSettings ); \
	case 4:	return new SORTER<COMPGROUP,int64_t,false,false> ( pComp, pQuery, tSettings ); \
	case 5:	return new SORTER<COMPGROUP,int64_t,true,false> ( pComp, pQuery, tSettings ); \
	case 6:	return new SORTER<COMPGROUP,int64_t,false,true> ( pComp, pQuery, tSettings ); \
	case 7:	return new SORTER<COMPGROUP,int64_t,true,true> ( pComp, pQuery, tSettings ); \
	default: assert(0); return nullptr; \
	} \
}


template < typename COMPGROUP >
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	BYTE uSelector = (tSettings.m_bMVA?1:0)
		+(tSettings.m_bImplicit?2:0)
		+((pQuery->m_iGroupbyLimit>1)?4:0)
		+(tSettings.m_bJson?8:0);

	switch ( uSelector )
	{
	case 0:	CREATE_SORTER_4TH		( CSphKBufferGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 1: CREATE_SORTER_4TH_MVA	( MvaGroupSorter_c,			COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 2: CREATE_SORTER_4TH		( CSphImplicitGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 4:	CREATE_SORTER_4TH		( CSphKBufferNGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 5: CREATE_SORTER_4TH_MVA	( MvaNGroupSorter_c,		COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 8: CREATE_SORTER_4TH		( CSphKBufferJsonGroupSorter, COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	default: assert(0); return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	switch ( eGroupFunc )
	{
		case FUNC_GENERIC1:		return sphCreateSorter3rd<MatchGeneric1_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>		( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		default:				return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	CSphRefcountedPtr<ISphMatchComparator> pComp;
	if ( !tSettings.m_bImplicit )
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn(); break;
			case FUNC_ATTR_DESC:	pComp = new MatchAttrLt_fn(); break;
			case FUNC_ATTR_ASC:		pComp = new MatchAttrGt_fn(); break;
			case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn(); break;
			case FUNC_GENERIC1:		pComp = new MatchGeneric1_fn(); break;
			case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn(); break;
			case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn(); break;
			case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn(); break;
			case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn(); break;
			case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
		}

	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors );
}

//////////////////////////////////////////////////////////////////////////
// GEODIST
//////////////////////////////////////////////////////////////////////////

struct ExprGeodist_t : public ISphExpr
{
public:
						ExprGeodist_t () = default;

	bool				Setup ( const CSphQuery * pQuery, const ISphSchema & tSchema, CSphString & sError );
	float				Eval ( const CSphMatch & tMatch ) const final;
	void				FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final;
	void				Command ( ESphExprCommand eCmd, void * pArg ) final;
	uint64_t			GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *			Clone() const final { return new ExprGeodist_t; }

protected:
	CSphAttrLocator		m_tGeoLatLoc;
	CSphAttrLocator		m_tGeoLongLoc;
	float				m_fGeoAnchorLat;
	float				m_fGeoAnchorLong;
	int					m_iLat;
	int					m_iLon;
};


bool ExprGeodist_t::Setup ( const CSphQuery * pQuery, const ISphSchema & tSchema, CSphString & sError )
{
	if ( !pQuery->m_bGeoAnchor )
	{
		sError.SetSprintf ( "INTERNAL ERROR: no geoanchor, can not create geodist evaluator" );
		return false;
	}

	int iLat = tSchema.GetAttrIndex ( pQuery->m_sGeoLatAttr.cstr() );
	if ( iLat<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLatAttr.cstr() );
		return false;
	}

	int iLong = tSchema.GetAttrIndex ( pQuery->m_sGeoLongAttr.cstr() );
	if ( iLong<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLongAttr.cstr() );
		return false;
	}

	m_tGeoLatLoc = tSchema.GetAttr(iLat).m_tLocator;
	m_tGeoLongLoc = tSchema.GetAttr(iLong).m_tLocator;
	m_fGeoAnchorLat = pQuery->m_fGeoLatitude;
	m_fGeoAnchorLong = pQuery->m_fGeoLongitude;
	m_iLat = iLat;
	m_iLon = iLong;
	return true;
}


static inline double sphSqr ( double v )
{
	return v*v;
}


float ExprGeodist_t::Eval ( const CSphMatch & tMatch ) const
{
	const double R = 6384000;
	float plat = tMatch.GetAttrFloat ( m_tGeoLatLoc );
	float plon = tMatch.GetAttrFloat ( m_tGeoLongLoc );
	double dlat = plat - m_fGeoAnchorLat;
	double dlon = plon - m_fGeoAnchorLong;
	double a = sphSqr ( sin ( dlat/2 ) ) + cos(plat)*cos(m_fGeoAnchorLat)*sphSqr(sin(dlon/2));
	double c = 2*asin ( Min ( 1.0, sqrt(a) ) );
	return (float)(R*c);
}


void ExprGeodist_t::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	sphFixupLocator ( m_tGeoLatLoc, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tGeoLongLoc, pOldSchema, pNewSchema );
}


void ExprGeodist_t::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
	{
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLat );
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLon );
	}
}

uint64_t ExprGeodist_t::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	uint64_t uHash = sphCalcExprDepHash ( this, tSorterSchema, uPrevHash, bDisable );

	static const char * EXPR_TAG = "ExprGeodist_t";
	uHash = sphFNV64 ( EXPR_TAG, (int) strlen(EXPR_TAG), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLat, sizeof(m_fGeoAnchorLat), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLong, sizeof(m_fGeoAnchorLong), uHash );

	return uHash;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );
static CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators,
		const CSphVector<ESphAttr> & dAttrTypes, VecRefPtrs_t<ISphExpr *> dJsonKeys,
		ESphCollation eCollation );

class QueueCreator_c
{
	const SphQueueSettings_t& m_tSettings;
	const CSphQuery & m_tQuery;
	CSphString & m_sError;
	StrVec_t * m_pExtra = nullptr;

	bool m_bHasCount = false;
	bool m_bHasGroupByExpr = false;
	sph::StringSet m_hQueryAttrs;
	CSphScopedPtr<CSphRsetSchema> m_pSorterSchema;

	bool m_bGotGroupby;
	bool m_bRandomize;
	ESphSortFunc m_eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc m_eGroupFunc = FUNC_REL_DESC;
	CSphMatchComparatorState m_tStateMatch;
	CSphMatchComparatorState m_tStateGroup;
	CSphGroupSorterSettings m_tGroupSorterSettings;
	CSphVector<int> m_dGroupColumns;
	bool m_bHeadWOGroup;
	bool m_bGotDistinct;

	// for sorter to create pooled attributes
	bool m_bHaveStar = false;

	// fixme! transform to StringSet on end of merge!
	sph::StringSet m_hQueryColumns; // FIXME!!! unify with Extra schema after merge master into branch
	sph::StringSet m_hQueryDups;
	sph::StringSet m_hExtra;

private:
	// helpers
	bool ParseQueryItem ( const CSphQueryItem & tItem );
	bool MaybeAddGeodistColumn ();
	bool MaybeAddExprColumn ();
	bool MaybeAddExpressionsFromSelectList ();
	bool MaybeAddGroupbyMagic ( bool bGotDistinct );
	bool CheckHavingConstraints () const;
	bool SetupGroupbySettings ( bool bHasImplicitGrouping );
	void AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount );
	void RemapStaticStringsAndJsons ( CSphMatchComparatorState & tState );
	bool SetupMatchesSortingFunc ();
	bool SetupGroupSortingFunc ( bool bGotDistinct );
	bool AddGroupbyStuff();
	bool SetGroupSorting ();
	void ExtraAddSortkeys ( const int * dAttrs );

	ISphMatchSorter * SpawnQueue ();
	ISphFilter * CreateAggrFilter () const;
	void SetupCollation ();
	bool Err ( const char * sFmt, ... ) const;

public:
	QueueCreator_c ( const SphQueueSettings_t& tSettings, const CSphQuery & tQuery,
			CSphString& sError, StrVec_t * pExtra )
		: m_tSettings ( tSettings )
		, m_tQuery ( tQuery )
		, m_sError ( sError )
		, m_pExtra ( pExtra )
		, m_pSorterSchema { new CSphRsetSchema }
	{
		// short-cuts
		m_sError = "";
		*m_pSorterSchema.Ptr () = m_tSettings.m_tSchema;
	}

	bool SetupComputeQueue ();
	bool SetupGroupQueue ();
	bool SetupQueue ();

	CSphRsetSchema& SorterSchema() const { return *m_pSorterSchema.Ptr(); }
	bool HasJson () const { return m_tGroupSorterSettings.m_bJson; }
	/// creates proper queue for given query
	/// may return NULL on error; in this case, error message is placed in sError
	/// if the pUpdate is given, creates the updater's queue and perform the index update
	/// instead of searching
	ISphMatchSorter * CreateQueue ();

	bool m_bMulti = false;
	bool m_bCreate = true;
	bool m_bZonespanlist = false;
	DWORD m_uPackedFactorFlags = SPH_FACTOR_DISABLE;

};

bool QueueCreator_c::SetupGroupbySettings ( bool bHasImplicitGrouping )
{
	if ( m_tQuery.m_sGroupBy.IsEmpty() && !bHasImplicitGrouping )
		return true;

	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
		return Err ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema.Ptr();

	m_tGroupSorterSettings.m_iMaxMatches = m_tSettings.m_iMaxMatches;

	// setup distinct attr
	m_tGroupSorterSettings.m_tDistinctAttr.m_iBitOffset = -1;
	if ( !m_tQuery.m_sGroupDistinct.IsEmpty () )
	{
		int iDistinct = tSchema.GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr () );
		if ( iDistinct<0 )
			return Err ( "group-count-distinct attribute '%s' not found", m_tQuery.m_sGroupDistinct.cstr () );

		const auto & tDistinctAttr = tSchema.GetAttr ( iDistinct );
		m_tGroupSorterSettings.m_tDistinctAttr = tDistinctAttr.m_tLocator;
		m_tGroupSorterSettings.m_eDistinctAttr = tDistinctAttr.m_eAttrType;
	}


	CSphString sJsonColumn;
	CSphString sJsonKey;
	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_MULTIPLE )
	{
		CSphVector<CSphAttrLocator> dLocators;
		CSphVector<ESphAttr> dAttrTypes;
		VecRefPtrs_t<ISphExpr *> dJsonKeys;

		StrVec_t dGroupBy;
		sph::Split ( m_tQuery.m_sGroupBy.cstr (), -1, ",", [&] ( const char * sToken, int iLen )
		{
			CSphString sGroupBy ( sToken, iLen );
			sGroupBy.Trim ();
			dGroupBy.Add ( std::move ( sGroupBy ));
		} );
		dGroupBy.Uniq();

		for ( auto& sGroupBy : dGroupBy )
		{
			CSphString sJsonExpr;
			if ( sphJsonNameSplit ( sGroupBy.cstr(), &sJsonColumn, &sJsonKey ) )
			{
				sJsonExpr = sGroupBy;
				sGroupBy = sJsonColumn;
			}

			const int iAttr = tSchema.GetAttrIndex ( sGroupBy.cstr() );
			if ( iAttr<0 )
				return Err( "group-by attribute '%s' not found", sGroupBy.cstr() );

			auto tAttr = tSchema.GetAttr ( iAttr );
			ESphAttr eType = tAttr.m_eAttrType;
			if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
				return Err ( "MVA values can't be used in multiple group-by" );

			if ( eType==SPH_ATTR_JSON && sJsonExpr.IsEmpty() )
				return Err ( "JSON blob can't be used in multiple group-by" );

			dLocators.Add ( tAttr.m_tLocator );
			dAttrTypes.Add ( eType );
			m_dGroupColumns.Add ( iAttr );

			if ( !sJsonExpr.IsEmpty() )
			{
				ExprParseArgs_t tExprArgs;
				dJsonKeys.Add ( sphExprParse ( sJsonExpr.cstr(), tSchema, m_sError, tExprArgs ) );
			}
			else
				dJsonKeys.Add ( nullptr );
		}

		m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperMulti ( dLocators, dAttrTypes,
				std::move(dJsonKeys), m_tQuery.m_eCollation );
		return true;
	}

	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr(), &sJsonColumn, &sJsonKey ) )
	{
		const int iAttr = tSchema.GetAttrIndex ( sJsonColumn.cstr() );
		if ( iAttr<0 )
			return Err ( "groupby: no such attribute '%s'", sJsonColumn.cstr ());

		if ( tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON
			&& tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON_PTR )
			return Err ( "groupby: attribute '%s' does not have subfields (must be sql_attr_json)",
					sJsonColumn.cstr() );

		if ( m_tQuery.m_eGroupFunc!=SPH_GROUPBY_ATTR )
			return Err ( "groupby: legacy groupby modes are not supported on JSON attributes" );

		m_dGroupColumns.Add ( iAttr );

		ExprParseArgs_t tExprArgs;
		tExprArgs.m_eCollation = m_tQuery.m_eCollation;

		CSphRefcountedPtr<ISphExpr> pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(),
				tSchema, m_sError, tExprArgs ) };
		m_tGroupSorterSettings.m_pGrouper = new CSphGrouperJsonField ( tSchema.GetAttr(iAttr).m_tLocator, pExpr );
		m_tGroupSorterSettings.m_bJson = true;
		return true;

	}

	if ( bHasImplicitGrouping )
	{
		m_tGroupSorterSettings.m_bImplicit = true;
		return true;
	}

	// setup groupby attr
	int iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_sGroupBy.cstr() );

	if ( iGroupBy<0 )
	{
		// try aliased groupby attr (facets)
		ARRAY_FOREACH ( i, m_tQuery.m_dItems )
			if ( m_tQuery.m_sGroupBy==m_tQuery.m_dItems[i].m_sExpr )
			{
				iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_dItems[i].m_sAlias.cstr() );
				break;

			} else if ( m_tQuery.m_sGroupBy==m_tQuery.m_dItems[i].m_sAlias )
			{
				iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_dItems[i].m_sExpr.cstr() );
				break;
			}
	}

	if ( iGroupBy<0 )
		return Err ( "group-by attribute '%s' not found", m_tQuery.m_sGroupBy.cstr() );

	ESphAttr eType = tSchema.GetAttr ( iGroupBy ).m_eAttrType;
	CSphAttrLocator tLoc = tSchema.GetAttr ( iGroupBy ).m_tLocator;
	switch (m_tQuery.m_eGroupFunc )
	{
		case SPH_GROUPBY_DAY:
			m_tGroupSorterSettings.m_pGrouper = getDayGrouper ( tLoc ); break;
		case SPH_GROUPBY_WEEK:
			m_tGroupSorterSettings.m_pGrouper = getWeekGrouper ( tLoc ); break;
		case SPH_GROUPBY_MONTH:
			m_tGroupSorterSettings.m_pGrouper = getMonthGrouper ( tLoc ); break;
		case SPH_GROUPBY_YEAR:
			m_tGroupSorterSettings.m_pGrouper = getYearGrouper ( tLoc ); break;
		case SPH_GROUPBY_ATTR:
			if ( eType==SPH_ATTR_JSON || eType==SPH_ATTR_JSON_FIELD )
			{
				ExprParseArgs_t tExprArgs;
				tExprArgs.m_eCollation = m_tQuery.m_eCollation;

				CSphRefcountedPtr<ISphExpr> pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(),
						tSchema, m_sError, tExprArgs ) };
				m_tGroupSorterSettings.m_pGrouper = new CSphGrouperJsonField ( tLoc, pExpr );
				m_tGroupSorterSettings.m_bJson = true;

			} else if ( eType==SPH_ATTR_STRING || eType==SPH_ATTR_STRINGPTR ) // percolate select list push matches with string_ptr
				m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperString ( tLoc, m_tQuery.m_eCollation );
			else
				m_tGroupSorterSettings.m_pGrouper = new CSphGrouperAttr ( tLoc );
			break;
		default:
			return Err ( "invalid group-by mode (mode=%d)", m_tQuery.m_eGroupFunc );
	}

	m_tGroupSorterSettings.m_bMVA = ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET );
	m_tGroupSorterSettings.m_bMva64 = ( eType==SPH_ATTR_INT64SET );
	m_dGroupColumns.Add ( iGroupBy );
	return true;
}

// move expressions used in ORDER BY or WITHIN GROUP ORDER BY to presort phase
void QueueCreator_c::AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount )
{
	assert ( pAttrs );
	assert ( m_pSorterSchema );

	CSphVector<int> dCur;

	// add valid attributes to processing list
	for ( int i=0; i<iAttrCount; ++i )
		if ( pAttrs[i]>=0 )
			dCur.Add ( pAttrs[i] );

	// collect columns which affect current expressions
	for ( int i=0; i<dCur.GetLength(); ++i )
	{
		const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[i] );
		if ( tCol.m_eStage>SPH_EVAL_PRESORT && tCol.m_pExpr )
			tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
	}

	// get rid of dupes
	dCur.Uniq();

	// fix up of attributes stages
	for ( int iAttr : dCur )
	{
		if ( iAttr<0 )
			continue;

		auto & tCol = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( iAttr ) );
		if ( tCol.m_eStage==SPH_EVAL_FINAL )
			tCol.m_eStage = SPH_EVAL_PRESORT;
	}
}


// expression that transform string pool base + offset -> ptr
class ExprSortStringAttrFixup_c : public BlobPool_c, public ISphExpr
{
public:
	explicit ExprSortStringAttrFixup_c ( const CSphAttrLocator & tLocator )
		: m_tLocator ( tLocator )
	{}

	float Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }

	const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const override
	{
		// our blob strings are not null-terminated!
		// we can either store nulls in .SPB or add them here
		return sphPackPtrAttr ( sphGetBlobAttr ( tMatch, m_tLocator, GetBlobPool() ) );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			SetBlobPool( (const BYTE*)pArg);
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) override
	{
		assert ( 0 && "remap expressions in filters" );
		return 0;
	}

	ISphExpr * Clone() const final
	{
		return new ExprSortStringAttrFixup_c ( *this );
	}

public:
	CSphAttrLocator		m_tLocator;				///< string attribute to fix

private:
	ExprSortStringAttrFixup_c ( const ExprSortStringAttrFixup_c& rhs ) : m_tLocator ( rhs.m_tLocator ) {}
};


// expression that transform string pool base + offset -> ptr
class ExprSortJson2StringPtr_c : public BlobPool_c, public ISphExpr
{
public:
	ExprSortJson2StringPtr_c ( const CSphAttrLocator & tLocator, ISphExpr * pExpr )
		: m_tJsonCol ( tLocator )
		, m_pExpr ( pExpr )
	{
		if ( pExpr ) // adopt the expression
			pExpr->AddRef();
	}

	bool IsDataPtrAttr () const final { return true; }

	float Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override
	{
		if ( !GetBlobPool() || !m_pExpr )
		{
			*ppStr = nullptr;
			return 0;
		}

		uint64_t uPacked = m_pExpr->Int64Eval ( tMatch );

		const BYTE * pVal = GetBlobPool() + sphJsonUnpackOffset ( uPacked );
		ESphJsonType eJson = sphJsonUnpackType ( uPacked );

		CSphString sVal;

		// FIXME!!! make string length configurable for STRING and STRING_VECTOR to compare and allocate only Min(String.Length, CMP_LENGTH)
		switch ( eJson )
		{
		case JSON_INT32:
			sVal.SetSprintf ( "%d", sphJsonLoadInt ( &pVal ) );
			break;
		case JSON_INT64:
			sVal.SetSprintf ( INT64_FMT, sphJsonLoadBigint ( &pVal ) );
			break;
		case JSON_DOUBLE:
			sVal.SetSprintf ( "%f", sphQW2D ( sphJsonLoadBigint ( &pVal ) ) );
			break;
		case JSON_STRING:
		{
			int iLen = sphJsonUnpackInt ( &pVal );
			sVal.SetBinary ( (const char *)pVal, iLen );
			break;
		}
		case JSON_STRING_VECTOR:
		{
			int iTotalLen = sphJsonUnpackInt ( &pVal );
			int iCount = sphJsonUnpackInt ( &pVal );

			CSphFixedVector<BYTE> dBuf ( iTotalLen + 4 ); // data and tail GAP
			BYTE * pDst = dBuf.Begin();

			// head element
			if ( iCount )
			{
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
			}

			// tail elements separated by space
			for ( int i=1; i<iCount; i++ )
			{
				*pDst++ = ' ';
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
			}

			int iStrLen = pDst-dBuf.Begin();
			// filling junk space
			while ( pDst<dBuf.Begin()+dBuf.GetLength() )
				*pDst++ = '\0';

			*ppStr = dBuf.LeakData();
			return iStrLen;
		}
		default:
			break;
		}

		int iStriLen = sVal.Length();
		*ppStr = (const BYTE *)sVal.Leak();
		return iStriLen;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		sphFixupLocator ( m_tJsonCol, pOldSchema, pNewSchema );
		if ( m_pExpr )
			m_pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
		{
			SetBlobPool((const BYTE*)pArg);
			if ( m_pExpr )
				m_pExpr->Command ( eCmd, pArg );
		}
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) override
	{
		assert ( 0 && "remap expression in filters" );
		return 0;
	}

	ISphExpr * Clone() const final
	{
		return new ExprSortJson2StringPtr_c ( *this );
	}

private:
	CSphAttrLocator		m_tJsonCol;				///< JSON attribute to fix
	CSphRefcountedPtr<ISphExpr>	m_pExpr;

private:
	ExprSortJson2StringPtr_c ( const ExprSortJson2StringPtr_c & rhs )
		: m_tJsonCol ( rhs.m_tJsonCol )
		, m_pExpr ( SafeClone (rhs.m_pExpr) )
	{}
};


bool IsSortStringInternal ( const CSphString & sColumnName )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntAttrPrefix, sizeof ( g_sIntAttrPrefix )-1 )==0 );
}

bool IsSortJsonInternal ( const CSphString& sColumnName  )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntJsonPrefix, sizeof ( g_sIntJsonPrefix )-1 )==0 );
}

CSphString SortJsonInternalSet ( const CSphString& sColumnName )
{
	CSphString sName;
	if ( !sColumnName.IsEmpty() )
		( StringBuilder_c () << g_sIntJsonPrefix << "_" << sColumnName ).MoveTo ( sName );
	return sName;
}

////////////////////
// BINARY COLLATION
////////////////////

static const char * EMPTY_STR = "";

inline static void UnpackStrings ( const BYTE * & pStr1, const BYTE * & pStr2, bool bDataPtr, int & iLen1, int & iLen2 )
{
	// strings that are stored in index don't need to be unpacked
	if ( bDataPtr )
	{
		iLen1 = sphUnpackPtrAttr ( pStr1, &pStr1 );
		iLen2 = sphUnpackPtrAttr ( pStr2, &pStr2 );
	}

	if ( !pStr1 ) pStr1 = (const BYTE*)EMPTY_STR;
	if ( !pStr2 ) pStr2 = (const BYTE*)EMPTY_STR;
}


int sphCollateBinary ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

	int iRes = memcmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}

///////////////////////////////
// LIBC_CI, LIBC_CS COLLATIONS
///////////////////////////////

/// libc_ci, wrapper for strcasecmp
int sphCollateLibcCI ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

	int iRes = strncasecmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}


/// libc_cs, wrapper for strcoll
int sphCollateLibcCS ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	#define COLLATE_STACK_BUFFER 1024

	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );
	
	// strcoll wants asciiz strings, so we would have to copy them over
	// lets use stack buffer for smaller ones, and allocate from heap for bigger ones
	int iRes = 0;
	int iLen = Min ( iLen1, iLen2 );
	if ( iLen<COLLATE_STACK_BUFFER )
	{
		// small strings on stack
		BYTE sBuf1[COLLATE_STACK_BUFFER];
		BYTE sBuf2[COLLATE_STACK_BUFFER];

		memcpy ( sBuf1, pStr1, iLen );
		memcpy ( sBuf2, pStr2, iLen );
		sBuf1[iLen] = sBuf2[iLen] = '\0';
		iRes = strcoll ( (const char*)sBuf1, (const char*)sBuf2 );
	} else
	{
		// big strings on heap
		char * pBuf1 = new char[iLen + 1];
		char * pBuf2 = new char[iLen + 1];

		memcpy ( pBuf1, pStr1, iLen );
		memcpy ( pBuf2, pStr2, iLen );
		pBuf1[iLen] = pBuf2[iLen] = '\0';
		iRes = strcoll ( (const char*)pBuf1, (const char*)pBuf2 );

		SafeDeleteArray ( pBuf2 );
		SafeDeleteArray ( pBuf1 );
	}

	return iRes ? iRes : ( iLen1-iLen2 );
}

/////////////////////////////
// UTF8_GENERAL_CI COLLATION
/////////////////////////////

/// 1st level LUT
static unsigned short * g_dCollPlanes_UTF8CI[0x100];

/// 2nd level LUT, non-trivial collation data
static unsigned short g_dCollWeights_UTF8CI[0xb00] =
{
	// weights for 0x0 to 0x5ff
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 924, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	65, 65, 65, 65, 65, 65, 198, 67, 69, 69, 69, 69, 73, 73, 73, 73,
	208, 78, 79, 79, 79, 79, 79, 215, 216, 85, 85, 85, 85, 89, 222, 83,
	65, 65, 65, 65, 65, 65, 198, 67, 69, 69, 69, 69, 73, 73, 73, 73,
	208, 78, 79, 79, 79, 79, 79, 247, 216, 85, 85, 85, 85, 89, 222, 89,
	65, 65, 65, 65, 65, 65, 67, 67, 67, 67, 67, 67, 67, 67, 68, 68,
	272, 272, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 71, 71, 71, 71,
	71, 71, 71, 71, 72, 72, 294, 294, 73, 73, 73, 73, 73, 73, 73, 73,
	73, 73, 306, 306, 74, 74, 75, 75, 312, 76, 76, 76, 76, 76, 76, 319,
	319, 321, 321, 78, 78, 78, 78, 78, 78, 329, 330, 330, 79, 79, 79, 79,
	79, 79, 338, 338, 82, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 83,
	83, 83, 84, 84, 84, 84, 358, 358, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 85, 85, 87, 87, 89, 89, 89, 90, 90, 90, 90, 90, 90, 83,
	384, 385, 386, 386, 388, 388, 390, 391, 391, 393, 394, 395, 395, 397, 398, 399,
	400, 401, 401, 403, 404, 502, 406, 407, 408, 408, 410, 411, 412, 413, 414, 415,
	79, 79, 418, 418, 420, 420, 422, 423, 423, 425, 426, 427, 428, 428, 430, 85,
	85, 433, 434, 435, 435, 437, 437, 439, 440, 440, 442, 443, 444, 444, 446, 503,
	448, 449, 450, 451, 452, 452, 452, 455, 455, 455, 458, 458, 458, 65, 65, 73,
	73, 79, 79, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 398, 65, 65,
	65, 65, 198, 198, 484, 484, 71, 71, 75, 75, 79, 79, 79, 79, 439, 439,
	74, 497, 497, 497, 71, 71, 502, 503, 78, 78, 65, 65, 198, 198, 216, 216,
	65, 65, 65, 65, 69, 69, 69, 69, 73, 73, 73, 73, 79, 79, 79, 79,
	82, 82, 82, 82, 85, 85, 85, 85, 83, 83, 84, 84, 540, 540, 72, 72,
	544, 545, 546, 546, 548, 548, 65, 65, 69, 69, 79, 79, 79, 79, 79, 79,
	79, 79, 89, 89, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575,
	576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591,
	592, 593, 594, 385, 390, 597, 393, 394, 600, 399, 602, 400, 604, 605, 606, 607,
	403, 609, 610, 404, 612, 613, 614, 615, 407, 406, 618, 619, 620, 621, 622, 412,
	624, 625, 413, 627, 628, 415, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639,
	422, 641, 642, 425, 644, 645, 646, 647, 430, 649, 433, 434, 652, 653, 654, 655,
	656, 657, 439, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671,
	672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687,
	688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703,
	704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719,
	720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735,
	736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751,
	752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767,
	768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783,
	784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799,
	800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815,
	816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831,
	832, 833, 834, 835, 836, 921, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847,
	848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863,
	864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879,
	880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895,
	896, 897, 898, 899, 900, 901, 913, 903, 917, 919, 921, 907, 927, 909, 933, 937,
	921, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927,
	928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 921, 933, 913, 917, 919, 921,
	933, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927,
	928, 929, 931, 931, 932, 933, 934, 935, 936, 937, 921, 933, 927, 933, 937, 975,
	914, 920, 978, 978, 978, 934, 928, 983, 984, 985, 986, 986, 988, 988, 990, 990,
	992, 992, 994, 994, 996, 996, 998, 998, 1000, 1000, 1002, 1002, 1004, 1004, 1006, 1006,
	922, 929, 931, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023,
	1045, 1045, 1026, 1043, 1028, 1029, 1030, 1030, 1032, 1033, 1034, 1035, 1050, 1048, 1059, 1039,
	1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055,
	1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071,
	1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055,
	1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071,
	1045, 1045, 1026, 1043, 1028, 1029, 1030, 1030, 1032, 1033, 1034, 1035, 1050, 1048, 1059, 1039,
	1120, 1120, 1122, 1122, 1124, 1124, 1126, 1126, 1128, 1128, 1130, 1130, 1132, 1132, 1134, 1134,
	1136, 1136, 1138, 1138, 1140, 1140, 1140, 1140, 1144, 1144, 1146, 1146, 1148, 1148, 1150, 1150,
	1152, 1152, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1164, 1166, 1166,
	1168, 1168, 1170, 1170, 1172, 1172, 1174, 1174, 1176, 1176, 1178, 1178, 1180, 1180, 1182, 1182,
	1184, 1184, 1186, 1186, 1188, 1188, 1190, 1190, 1192, 1192, 1194, 1194, 1196, 1196, 1198, 1198,
	1200, 1200, 1202, 1202, 1204, 1204, 1206, 1206, 1208, 1208, 1210, 1210, 1212, 1212, 1214, 1214,
	1216, 1046, 1046, 1219, 1219, 1221, 1222, 1223, 1223, 1225, 1226, 1227, 1227, 1229, 1230, 1231,
	1040, 1040, 1040, 1040, 1236, 1236, 1045, 1045, 1240, 1240, 1240, 1240, 1046, 1046, 1047, 1047,
	1248, 1248, 1048, 1048, 1048, 1048, 1054, 1054, 1256, 1256, 1256, 1256, 1069, 1069, 1059, 1059,
	1059, 1059, 1059, 1059, 1063, 1063, 1270, 1271, 1067, 1067, 1274, 1275, 1276, 1277, 1278, 1279,
	1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1293, 1294, 1295,
	1296, 1297, 1298, 1299, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311,
	1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327,
	1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343,
	1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359,
	1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1374, 1375,
	1376, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343,
	1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359,
	1360, 1361, 1362, 1363, 1364, 1365, 1366, 1415, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423,
	1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439,
	1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455,
	1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469, 1470, 1471,
	1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487,
	1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503,
	1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519,
	1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535,

	// weights for codepoints 0x1e00 to 0x1fff
	65, 65, 66, 66, 66, 66, 66, 66, 67, 67, 68, 68, 68, 68, 68, 68,
	68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 70, 70,
	71, 71, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 73, 73, 73, 73,
	75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77,
	77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79,
	79, 79, 79, 79, 80, 80, 80, 80, 82, 82, 82, 82, 82, 82, 82, 82,
	83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 84, 84, 84, 84, 84, 84,
	84, 84, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 86, 86, 86, 86,
	87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 88, 88, 88, 88, 89, 89,
	90, 90, 90, 90, 90, 90, 72, 84, 87, 89, 7834, 83, 7836, 7837, 7838, 7839,
	65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
	65, 65, 65, 65, 65, 65, 65, 65, 69, 69, 69, 69, 69, 69, 69, 69,
	69, 69, 69, 69, 69, 69, 69, 69, 73, 73, 73, 73, 79, 79, 79, 79,
	79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
	79, 79, 79, 79, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 89, 89, 89, 89, 89, 89, 89, 89, 7930, 7931, 7932, 7933, 7934, 7935,
	913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913,
	917, 917, 917, 917, 917, 917, 7958, 7959, 917, 917, 917, 917, 917, 917, 7966, 7967,
	919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919,
	921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921,
	927, 927, 927, 927, 927, 927, 8006, 8007, 927, 927, 927, 927, 927, 927, 8014, 8015,
	933, 933, 933, 933, 933, 933, 933, 933, 8024, 933, 8026, 933, 8028, 933, 8030, 933,
	937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937,
	913, 8123, 917, 8137, 919, 8139, 921, 8155, 927, 8185, 933, 8171, 937, 8187, 8062, 8063,
	913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913,
	919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919,
	937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937,
	913, 913, 913, 913, 913, 8117, 913, 913, 913, 913, 913, 8123, 913, 8125, 921, 8127,
	8128, 8129, 919, 919, 919, 8133, 919, 919, 917, 8137, 919, 8139, 919, 8141, 8142, 8143,
	921, 921, 921, 8147, 8148, 8149, 921, 921, 921, 921, 921, 8155, 8156, 8157, 8158, 8159,
	933, 933, 933, 8163, 929, 929, 933, 933, 933, 933, 933, 8171, 929, 8173, 8174, 8175,
	8176, 8177, 937, 937, 937, 8181, 937, 937, 927, 8185, 937, 8187, 937, 8189, 8190, 8191

	// space for codepoints 0x21xx, 0x24xx, 0xffxx (generated)
};


/// initialize collation LUTs
void sphCollationInit()
{
	const int dWeightPlane[0x0b] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x1e, 0x1f, 0x21, 0x24, 0xff };

	// generate missing weights
	for ( int i=0; i<0x100; i++ )
	{
		g_dCollWeights_UTF8CI[i+0x800] = (unsigned short)( 0x2100 + i - ( i>=0x70 && i<=0x7f )*16 ); // 2170..217f, -16
		g_dCollWeights_UTF8CI[i+0x900] = (unsigned short)( 0x2400 + i - ( i>=0xd0 && i<=0xe9 )*26 ); // 24d0..24e9, -26
		g_dCollWeights_UTF8CI[i+0xa00] = (unsigned short)( 0xff00 + i - ( i>=0x41 && i<=0x5a )*32 ); // ff41..ff5a, -32
	}

	// generate planes table
	for ( auto& dCollPlanes : g_dCollPlanes_UTF8CI )
		dCollPlanes = nullptr;

	for ( int i=0; i<0x0b; i++ )
		g_dCollPlanes_UTF8CI [ dWeightPlane[i] ] = g_dCollWeights_UTF8CI + 0x100*i;
}


/// collate a single codepoint
static inline int CollateUTF8CI ( int iCode )
{
	return ( ( iCode>>16 ) || !g_dCollPlanes_UTF8CI [ iCode>>8 ] )
		? iCode
		: g_dCollPlanes_UTF8CI [ iCode>>8 ][ iCode&0xff ];
}


/// utf8_general_ci
int sphCollateUtf8GeneralCI ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

	const BYTE * pMax1 = pStr1 + iLen1;
	const BYTE * pMax2 = pStr2 + iLen2;

	while ( pStr1<pMax1 && pStr2<pMax2 )
	{
		// FIXME! on broken data, decode might go beyond buffer bounds
		int iCode1 = sphUTF8Decode ( pStr1 );
		int iCode2 = sphUTF8Decode ( pStr2 );
		if ( !iCode1 && !iCode2 )
			return 0;
		if ( !iCode1 || !iCode2 )
			return !iCode1 ? -1 : 1;

		if ( iCode1==iCode2 )
			continue;
		iCode1 = CollateUTF8CI ( iCode1 );
		iCode2 = CollateUTF8CI ( iCode2 );
		if ( iCode1!=iCode2 )
			return iCode1-iCode2;
	}

	if ( pStr1>=pMax1 && pStr2>=pMax2 )
		return 0;

	return ( pStr1<pMax1 ) ? 1 : -1;
}


/////////////////////////////
// hashing functions
/////////////////////////////


class LibcCSHash_fn
{
	static const int LOCALE_SAFE_GAP = 16;

public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		int iCompositeLen = iLen + 1 + (int)( 3.0f * iLen ) + LOCALE_SAFE_GAP;
		CSphFixedVector<BYTE> dBuf { iCompositeLen };

		memcpy ( dBuf.Begin(), pStr, iLen );
		dBuf[iLen] = '\0';

		BYTE * pDst = dBuf.Begin()+iLen+1;
		int iDstAvailable = dBuf.GetLength() - iLen - LOCALE_SAFE_GAP;

		auto iDstLen = (int) strxfrm ( (char *)pDst, (const char *) dBuf.Begin(), iDstAvailable );
		assert ( iDstLen<iDstAvailable+LOCALE_SAFE_GAP );

		uint64_t uAcc = sphFNV64 ( pDst, iDstLen, uPrev );

		return uAcc;
	}
};


class LibcCIHash_fn
{
public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		uint64_t uAcc = uPrev;
		while ( iLen-- )
		{
			int iChar = tolower ( *pStr++ );
			uAcc = sphFNV64 ( &iChar, 4, uAcc );
		}

		return uAcc;
	}
};


class Utf8CIHash_fn
{
public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		uint64_t uAcc = uPrev;
		while ( iLen-- )
		{
			const BYTE * pCur = pStr++;
			int iCode = sphUTF8Decode ( pCur );
			iCode = CollateUTF8CI ( iCode );
			uAcc = sphFNV64 ( &iCode, 4, uAcc );
		}

		return uAcc;
	}
};


CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI : return new CSphGrouperString<Utf8CIHash_fn> ( tLoc );
	case SPH_COLLATION_LIBC_CI : return new CSphGrouperString<LibcCIHash_fn> ( tLoc );
	case SPH_COLLATION_LIBC_CS : return new CSphGrouperString<LibcCSHash_fn> ( tLoc );
	default: return new CSphGrouperString<BinaryHash_fn> ( tLoc );
	}
}

CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators,
		const CSphVector<ESphAttr> & dAttrTypes, VecRefPtrs_t<ISphExpr *> dJsonKeys,
		ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI :
		return new CSphGrouperMulti<Utf8CIHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	case SPH_COLLATION_LIBC_CI :
		return new CSphGrouperMulti<LibcCIHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	case SPH_COLLATION_LIBC_CS :
		return new CSphGrouperMulti<LibcCSHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	default:
		return new CSphGrouperMulti<BinaryHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	}
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


static ISphMatchSorter * CreatePlainSorter ( ESphSortFunc eMatchFunc, bool bKbuffer, int iMaxMatches, bool bFactors )
{
	switch ( eMatchFunc )
	{
		case FUNC_REL_DESC:		return CreatePlainSorter<MatchRelevanceLt_fn>	( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_ATTR_DESC:	return CreatePlainSorter<MatchAttrLt_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_ATTR_ASC:		return CreatePlainSorter<MatchAttrGt_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_TIMESEGS:		return CreatePlainSorter<MatchTimeSegments_fn>	( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC1:		return CreatePlainSorter<MatchGeneric1_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC2:		return CreatePlainSorter<MatchGeneric2_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC3:		return CreatePlainSorter<MatchGeneric3_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC4:		return CreatePlainSorter<MatchGeneric4_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC5:		return CreatePlainSorter<MatchGeneric5_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_EXPR:			return CreatePlainSorter<MatchExpr_fn>			( bKbuffer, iMaxMatches, bFactors ); break;
		default:				return nullptr;
	}
}


void QueueCreator_c::ExtraAddSortkeys ( const int * dAttrs )
{
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
		if ( dAttrs[i]>=0 )
			m_hExtra.Add ( m_pSorterSchema->GetAttr ( dAttrs[i] ).m_sName );
}

bool QueueCreator_c::Err ( const char * sFmt, ... ) const
{
	va_list ap;
	va_start ( ap, sFmt );
	m_sError.SetSprintfVa ( sFmt, ap );
	va_end ( ap );
	return false;
}

bool QueueCreator_c::ParseQueryItem ( const CSphQueryItem & tItem )
{
	assert ( m_pSorterSchema );
	const CSphString & sExpr = tItem.m_sExpr;
	bool bIsCount = IsCount(sExpr);
	m_bHasCount |= bIsCount;

	if ( sExpr=="*" )
	{
		m_bHaveStar = true;
		for ( int i=0; i<m_tSettings.m_tSchema.GetAttrsCount(); ++i )
		{
			m_hQueryDups.Add ( m_tSettings.m_tSchema.GetAttr(i).m_sName );
			m_hQueryColumns.Add ( m_tSettings.m_tSchema.GetAttr(i).m_sName );
		}
	}

	// for now, just always pass "plain" attrs from index to sorter; they will be filtered on searchd level
	int iAttrIdx = m_tSettings.m_tSchema.GetAttrIndex ( sExpr.cstr() );
	bool bPlainAttr = ( ( sExpr=="*" || ( iAttrIdx>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE ) ) &&
		( tItem.m_sAlias.IsEmpty() || tItem.m_sAlias==tItem.m_sExpr ) );
	if ( iAttrIdx>=0 )
	{
		ESphAttr eAttr = m_tSettings.m_tSchema.GetAttr ( iAttrIdx ).m_eAttrType;
		if ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR
			|| eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET )
		{
			if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE )
				return Err ( "can not aggregate non-scalar attribute '%s'",
						tItem.m_sExpr.cstr() );

			if ( !bPlainAttr && ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR ) )
			{
				bPlainAttr = true;
				for ( const auto & i : m_tQuery.m_dItems )
					if ( sExpr==i.m_sAlias )
						bPlainAttr = false;
			}
		}
	}

	if ( bPlainAttr || IsGroupby ( sExpr ) || bIsCount )
	{
		if ( sExpr!="*" && !tItem.m_sAlias.IsEmpty() )
		{
			m_hQueryDups.Add ( tItem.m_sAlias );
			if ( bPlainAttr )
				m_hQueryColumns.Add ( tItem.m_sExpr );
		}
		m_bHasGroupByExpr = IsGroupby ( sExpr );
		return true;
	}

	// not an attribute? must be an expression, and must be aliased by query parser
	assert ( !tItem.m_sAlias.IsEmpty() );

	// tricky part
	// we might be fed with precomputed matches, but it's all or nothing
	// the incoming match either does not have anything computed, or it has everything
	int iSorterAttr = m_pSorterSchema->GetAttrIndex ( tItem.m_sAlias.cstr() );
	if ( iSorterAttr>=0 )
	{
		if ( m_hQueryDups[tItem.m_sAlias] )
			return Err ( "alias '%s' must be unique (conflicts with another alias)",
					tItem.m_sAlias.cstr() );
		m_pSorterSchema->RemoveStaticAttr ( iSorterAttr );
	}

	// a new and shiny expression, lets parse
	CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
	DWORD uQueryPackedFactorFlags = SPH_FACTOR_DISABLE;
	bool bHasZonespanlist = false;

	ExprParseArgs_t tExprParseArgs;
	tExprParseArgs.m_pAttrType = &tExprCol.m_eAttrType;
	tExprParseArgs.m_pUsesWeight = &tExprCol.m_bWeight;
	tExprParseArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprParseArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprParseArgs.m_pHook = m_tSettings.m_pHook;
	tExprParseArgs.m_pZonespanlist = &bHasZonespanlist;
	tExprParseArgs.m_pPackedFactorsFlags = &uQueryPackedFactorFlags;
	tExprParseArgs.m_pEvalStage = &tExprCol.m_eStage;
	tExprParseArgs.m_pStoredField = &tExprCol.m_uFieldFlags;

	// tricky bit
	// GROUP_CONCAT() adds an implicit TO_STRING() conversion on top of its argument
	// and then the aggregate operation simply concatenates strings as matches arrive
	// ideally, we would instead pass ownership of the expression to G_C() implementation
	// and also the original expression type, and let the string conversion happen in G_C() itself
	// but that ideal route seems somewhat more complicated in the current architecture
	if ( tItem.m_eAggrFunc==SPH_AGGR_CAT )
	{
		CSphString sExpr2;
		sExpr2.SetSprintf ( "TO_STRING(%s)", sExpr.cstr() );
		tExprCol.m_pExpr = sphExprParse ( sExpr2.cstr(), *m_pSorterSchema.Ptr (), m_sError, tExprParseArgs );
	} else
	{
		tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), *m_pSorterSchema.Ptr (), m_sError, tExprParseArgs );
	}

	m_uPackedFactorFlags |= uQueryPackedFactorFlags;
	m_bZonespanlist |= bHasZonespanlist;
	tExprCol.m_eAggrFunc = tItem.m_eAggrFunc;
	if ( !tExprCol.m_pExpr )
		return Err ( "parse error: %s", m_sError.cstr() );

	// force AVG() to be computed in floats
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_AVG )
	{
		tExprCol.m_eAttrType = SPH_ATTR_FLOAT;
		tExprCol.m_tLocator.m_iBitCount = 32;
	}

	// force explicit type conversion for JSON attributes
	if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE && tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		return Err ( "ambiguous attribute type '%s', use INTEGER(), BIGINT() or "
			   "DOUBLE() conversion functions", tItem.m_sExpr.cstr() );

	if ( uQueryPackedFactorFlags & SPH_FACTOR_JSON_OUT )
		tExprCol.m_eAttrType = SPH_ATTR_FACTORS_JSON;

	// force GROUP_CONCAT() to be computed as strings
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_CAT )
	{
		tExprCol.m_eAttrType = SPH_ATTR_STRINGPTR;
		tExprCol.m_tLocator.m_iBitCount = ROWITEMPTR_BITS;
	}

	// postpone aggregates, add non-aggregates
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_NONE )
	{
		// is this expression used in filter?
		// OPTIMIZE? hash filters and do hash lookups?
		if ( tExprCol.m_eAttrType!=SPH_ATTR_JSON_FIELD )
			ARRAY_FOREACH ( i, m_tQuery.m_dFilters )
			if ( m_tQuery.m_dFilters[i].m_sAttrName==tExprCol.m_sName )
			{
				// is this a hack?
				// m_bWeight is computed after EarlyReject() get called
				// that means we can't evaluate expressions with WEIGHT() in prefilter phase
				if ( tExprCol.m_bWeight )
				{
					tExprCol.m_eStage = SPH_EVAL_PRESORT; // special, weight filter ( short cut )
					break;
				}

				// so we are about to add a filter condition
				// but it might depend on some preceding columns (e.g. SELECT 1+attr f1 ... WHERE f1>5)
				// lets detect those and move them to prefilter \ presort phase too
				CSphVector<int> dCur;
				tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

				// usual filter
				tExprCol.m_eStage = SPH_EVAL_PREFILTER;
				ARRAY_FOREACH ( j, dCur )
				{
					const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[j] );
					if ( tCol.m_bWeight )
					{
						tExprCol.m_eStage = SPH_EVAL_PRESORT;
						tExprCol.m_bWeight = true;
					}
					// handle chains of dependencies (e.g. SELECT 1+attr f1, f1-1 f2 ... WHERE f2>5)
					if ( tCol.m_pExpr )
						tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
				}
				dCur.Uniq();

				ARRAY_FOREACH ( j, dCur )
				{
					auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( dCur[j] ) );
					if ( tDep.m_eStage>tExprCol.m_eStage )
						tDep.m_eStage = tExprCol.m_eStage;
				}
				break;
			}

		// add it!
		// NOTE, "final" stage might need to be fixed up later
		// we'll do that when parsing sorting clause
		m_pSorterSchema->AddAttr ( tExprCol, true );
	} else // some aggregate
	{
		tExprCol.m_eStage = SPH_EVAL_PRESORT; // sorter expects computed expression
		m_pSorterSchema->AddAttr ( tExprCol, true );
		m_hExtra.Add ( tExprCol.m_sName );

		/// update aggregate dependencies (e.g. SELECT 1+attr f1, min(f1), ...)
		CSphVector<int> dCur;
		tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

		ARRAY_FOREACH ( j, dCur )
		{
			const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[j] );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
		}
		dCur.Uniq();

		ARRAY_FOREACH ( j, dCur )
		{
			auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( dCur[j] ) );
			if ( tDep.m_eStage>tExprCol.m_eStage )
				tDep.m_eStage = tExprCol.m_eStage;
		}
	}

	m_hQueryDups.Add ( tExprCol.m_sName );
	m_hQueryColumns.Add ( tExprCol.m_sName );

	// need to add all dependent columns for post limit expressions
	if ( tExprCol.m_eStage==SPH_EVAL_POSTLIMIT && tExprCol.m_pExpr )
	{
		CSphVector<int> dCur;
		tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

		ARRAY_FOREACH ( j, dCur )
		{
			const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[j] );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
		}
		dCur.Uniq ();

		ARRAY_FOREACH ( j, dCur )
		{
			const CSphColumnInfo & tDep = m_pSorterSchema->GetAttr ( dCur[j] );
			m_hQueryColumns.Add ( tDep.m_sName );
		}
	}

	return true;
}

// Test for @geodist and setup, if any
bool QueueCreator_c::MaybeAddGeodistColumn ()
{

	if ( m_tQuery.m_bGeoAnchor && m_pSorterSchema->GetAttrIndex ( "@geodist" )<0 )
	{
		auto pExpr = new ExprGeodist_t ();
		if ( !pExpr->Setup ( &m_tQuery, *m_pSorterSchema.Ptr(), m_sError ))
		{
			pExpr->Release ();
			return false;
		}
		CSphColumnInfo tCol ( "@geodist", SPH_ATTR_FLOAT );
		tCol.m_pExpr = pExpr; // takes ownership, no need to for explicit pExpr release
		tCol.m_eStage = SPH_EVAL_PREFILTER; // OPTIMIZE? actual stage depends on usage
		m_pSorterSchema->AddAttr ( tCol, true );
		m_hExtra.Add ( tCol.m_sName );
		m_hQueryAttrs.Add ( tCol.m_sName );
	}
	return true;
}

// Test for @expr and setup, if any
bool QueueCreator_c::MaybeAddExprColumn ()
{

	if ( m_tQuery.m_eSort==SPH_SORT_EXPR && m_pSorterSchema->GetAttrIndex ( "@expr" )<0 )
	{
		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility
		// (ie. too lazy to fix those tests right now)
		bool bHasZonespanlist;
		ExprParseArgs_t tExprArgs;
		tExprArgs.m_pProfiler = m_tSettings.m_pProfiler;
		tExprArgs.m_eCollation = m_tQuery.m_eCollation;
		tExprArgs.m_pZonespanlist = &bHasZonespanlist;

		tCol.m_pExpr = sphExprParse ( m_tQuery.m_sSortBy.cstr (), *m_pSorterSchema.Ptr(), m_sError, tExprArgs );
		m_bZonespanlist |= bHasZonespanlist;
		if ( !tCol.m_pExpr )
			return false;
		tCol.m_eStage = SPH_EVAL_PRESORT;
		m_pSorterSchema->AddAttr ( tCol, true );
		m_hQueryAttrs.Add ( tCol.m_sName );
	}
	return true;
}

// Add computed items
bool QueueCreator_c::MaybeAddExpressionsFromSelectList ()
{
	// expressions from select items
	if ( m_tSettings.m_bComputeItems )
	{
		if ( !m_tQuery.m_dItems.TestAll (
				[&] ( const CSphQueryItem & v ) { return ParseQueryItem ( v ); } ))
			return false;

		// add expressions for stored fields
		if ( m_bHaveStar )
			for ( int i = 0; i<m_tSettings.m_tSchema.GetFieldsCount (); ++i )
			{
				const CSphColumnInfo & tField = m_tSettings.m_tSchema.GetField ( i );
				if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
				{
					CSphQueryItem tItem;
					tItem.m_sExpr = tItem.m_sAlias = tField.m_sName;
					if ( !ParseQueryItem ( tItem ))
						return false;
				}
			}
	}
	return true;
}

bool QueueCreator_c::MaybeAddGroupbyMagic ( bool bGotDistinct )
{
	CSphString sJsonGroupBy;
	// now lets add @groupby etc if needed
	if ( m_bGotGroupby && m_pSorterSchema->GetAttrIndex ( "@groupby" )<0 )
	{
		ESphAttr eGroupByResult = ( !m_tGroupSorterSettings.m_bImplicit )
				? m_tGroupSorterSettings.m_pGrouper->GetResultType ()
				: SPH_ATTR_INTEGER; // implicit do not have grouper
		// all FACET group by should be widest possible type
		if ( m_tGroupSorterSettings.m_bMva64
			|| m_tQuery.m_bFacet
			|| m_tQuery.m_bFacetHead
			|| m_bMulti )
			eGroupByResult = SPH_ATTR_BIGINT;

		CSphColumnInfo tGroupby ( "@groupby", eGroupByResult );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_BIGINT );

		tGroupby.m_eStage = SPH_EVAL_SORTER;
		tCount.m_eStage = SPH_EVAL_SORTER;

		auto AddColumn = [this] ( const CSphColumnInfo & tCol )
		{
			m_pSorterSchema->AddAttr ( tCol, true );
			m_hQueryColumns.Add ( tCol.m_sName );
		};

		AddColumn ( tGroupby );
		AddColumn ( tCount );

		if ( bGotDistinct )
		{
			CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );
			tDistinct.m_eStage = SPH_EVAL_SORTER;
			AddColumn ( tDistinct );
		}

		// add @groupbystr last in case we need to skip it on sending (like @int_str2ptr_*)
		if ( m_tGroupSorterSettings.m_bJson )
		{
			sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
			if ( !m_pSorterSchema->GetAttr ( sJsonGroupBy.cstr() ) )
			{
				CSphColumnInfo tGroupbyStr ( sJsonGroupBy.cstr(), SPH_ATTR_JSON_FIELD );
				tGroupbyStr.m_eStage = SPH_EVAL_SORTER;
				AddColumn ( tGroupbyStr );
			}
		}
	}

	#define LOC_CHECK( _cond, _msg ) if (!(_cond)) { m_sError = "invalid schema: " _msg; return false; }

	int iGroupby = m_pSorterSchema->GetAttrIndex ( "@groupby" );
	if ( iGroupby>=0 ) {
		m_tGroupSorterSettings.m_bDistinct = bGotDistinct;
		m_tGroupSorterSettings.m_tLocGroupby = m_pSorterSchema->GetAttr ( iGroupby ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocGroupby.m_bDynamic, "@groupby must be dynamic" );

		int iCount = m_pSorterSchema->GetAttrIndex ( "@count" );
		LOC_CHECK ( iCount>=0, "missing @count" );

		m_tGroupSorterSettings.m_tLocCount = m_pSorterSchema->GetAttr ( iCount ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocCount.m_bDynamic, "@count must be dynamic" );

		int iDistinct = m_pSorterSchema->GetAttrIndex ( "@distinct" );
		if ( bGotDistinct ) {
			LOC_CHECK ( iDistinct>=0, "missing @distinct" );
			m_tGroupSorterSettings.m_tLocDistinct = m_pSorterSchema->GetAttr ( iDistinct ).m_tLocator;
			LOC_CHECK ( m_tGroupSorterSettings.m_tLocDistinct.m_bDynamic, "@distinct must be dynamic" );
		} else LOC_CHECK ( iDistinct<=0, "unexpected @distinct" );

		int iGroupbyStr = m_pSorterSchema->GetAttrIndex ( sJsonGroupBy.cstr() );
		if ( iGroupbyStr>=0 )
			m_tGroupSorterSettings.m_tLocGroupbyStr = m_pSorterSchema->GetAttr ( iGroupbyStr ).m_tLocator;
	}

	if ( m_bHasCount )
		LOC_CHECK ( m_pSorterSchema->GetAttrIndex ( "@count" )>=0
					, "Count(*) or @count is queried, but not available in the schema" );

	#undef LOC_CHECK
	return true;
}

bool QueueCreator_c::CheckHavingConstraints () const
{
	if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty () )
	{
		if ( !m_bGotGroupby )
			return Err ( "can not use HAVING without GROUP BY" );

		// should be column named at group by or it's alias or aggregate
		const CSphString & sHaving = m_tSettings.m_pAggrFilter->m_sAttrName;
		if ( !IsGroupbyMagic ( sHaving ) )
		{
			bool bValidHaving = false;
			for ( const CSphQueryItem & tItem : m_tQuery.m_dItems )
			{
				if ( tItem.m_sAlias!=sHaving )
					continue;

				bValidHaving = ( IsGroupbyMagic ( tItem.m_sExpr ) || tItem.m_eAggrFunc!=SPH_AGGR_NONE );
				break;
			}

			if ( !bValidHaving )
				return Err ( "can not use HAVING with attribute not related to GROUP BY" );
		}
	}
	return true;
}


// only STRING ( static packed ) and JSON fields must be remapped
void QueueCreator_c::RemapStaticStringsAndJsons ( CSphMatchComparatorState & tState )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema.Ptr();

	int iColWasCount = tSorterSchema.GetAttrsCount();
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
	{
		bool bIsStr = ( tState.m_eKeypart[i]==SPH_KEYPART_STRING );
		bool bIsJson = ( !tState.m_tSubKeys[i].m_sKey.IsEmpty() );
		const CSphColumnInfo * pGroupStrBase = nullptr;

		if ( !bIsStr && !bIsJson )
		{
			int iAttrSorted = tState.m_dAttrs[i];
			if ( m_tSettings.m_bComputeItems && iAttrSorted>=0 && iAttrSorted<iColWasCount && tSorterSchema.GetAttr ( iAttrSorted ).m_sName=="@groupby" && m_dGroupColumns.GetLength() )
			{
				// FIXME!!! add support of multi group by
				const CSphColumnInfo & tGroupCol = tSorterSchema.GetAttr ( m_dGroupColumns[0] );
				if ( tGroupCol.m_eAttrType==SPH_ATTR_STRING || tGroupCol.m_eAttrType==SPH_ATTR_STRINGPTR )
					pGroupStrBase = &tGroupCol;
			}

			if ( !pGroupStrBase )
				continue;
		}

		assert ( tState.m_dAttrs[i]>=0 && tState.m_dAttrs[i]<iColWasCount );

		bool bIsFunc = ( bIsJson && tState.m_tSubKeys[i].m_uMask==0 );

		const char * sRemapBaseName = nullptr;
		if ( pGroupStrBase )
		{
			sRemapBaseName = pGroupStrBase->m_sName.cstr();
		} else
		{
			sRemapBaseName = ( bIsJson ? tState.m_tSubKeys[i].m_sKey.cstr() : tSorterSchema.GetAttr ( tState.m_dAttrs[i] ).m_sName.cstr() );
		}

		CSphString sRemapCol;
		sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, sRemapBaseName );

		int iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
		if ( iRemap==-1 && bIsJson )
		{
			CSphString sRemapLowercase = sRemapCol;
			sRemapLowercase.ToLower();
			iRemap = tSorterSchema.GetAttrIndex ( sRemapLowercase.cstr() );
		}

		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;
			if ( bIsJson )
			{
				if ( bIsFunc )
				{
					tRemapCol.m_pExpr = tState.m_tSubExpr[i];
					tRemapCol.m_pExpr->AddRef();
				} else
				{
					tRemapCol.m_pExpr = new ExprSortJson2StringPtr_c ( tState.m_tLocator[i], tState.m_tSubExpr[i] );
				}
			} else
			{
				if ( pGroupStrBase )
					tState.m_tLocator[i] = pGroupStrBase->m_tLocator;
				tRemapCol.m_pExpr = new ExprSortStringAttrFixup_c ( tState.m_tLocator[i] );
				tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
			}

			if ( bIsFunc )
			{
				tRemapCol.m_eAttrType = tState.m_tSubType[i];
				tState.m_eKeypart[i] = Attr2Keypart ( tState.m_tSubType[i] );
			}

			iRemap = tSorterSchema.GetAttrsCount();
			tSorterSchema.AddAttr ( tRemapCol, true );
		} else if ( pGroupStrBase )
		{
			tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
		}
		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
	}

	// need another sort keys add after setup remap
	if ( iColWasCount!=tSorterSchema.GetAttrsCount ())
		ExtraAddSortkeys ( tState.m_dAttrs );
}

// matches sorting function
bool QueueCreator_c::SetupMatchesSortingFunc ()
{
	m_bRandomize = false;
	if ( m_tQuery.m_eSort==SPH_SORT_EXTENDED )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( &m_tQuery, m_tQuery.m_sSortBy.cstr(),
				*m_pSorterSchema.Ptr(), m_eMatchFunc, m_tStateMatch, m_tSettings.m_bComputeItems,
				m_sError );

		if ( eRes==SORT_CLAUSE_ERROR )
			return false;

		if ( eRes==SORT_CLAUSE_RANDOM )
			m_bRandomize = true;

		ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

		AssignOrderByToPresortStage ( m_tStateMatch.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );
		RemapStaticStringsAndJsons ( m_tStateMatch );
		return true;
	}

	if ( m_tQuery.m_eSort==SPH_SORT_EXPR )
	{
		m_tStateMatch.m_eKeypart[0] = SPH_KEYPART_INT;
		m_tStateMatch.m_tLocator[0] = m_pSorterSchema->GetAttr ( m_pSorterSchema->GetAttrIndex ( "@expr" ) ).m_tLocator;
		m_tStateMatch.m_eKeypart[1] = SPH_KEYPART_ROWID;
		m_tStateMatch.m_uAttrDesc = 1;
		m_eMatchFunc = FUNC_EXPR;
		return true;
	}

	// check sort-by attribute
	if ( m_tQuery.m_eSort!=SPH_SORT_RELEVANCE )
	{
		int iSortAttr = m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sSortBy.cstr() );
		if ( iSortAttr<0 )
		{
			Err ( "sort-by attribute '%s' not found", m_tQuery.m_sSortBy.cstr() );
			return false;
		}

		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr ( iSortAttr );
		m_tStateMatch.m_eKeypart[0] = Attr2Keypart ( tAttr.m_eAttrType );
		m_tStateMatch.m_tLocator[0] = tAttr.m_tLocator;
		m_tStateMatch.m_dAttrs[0] = iSortAttr;
		RemapStaticStringsAndJsons ( m_tStateMatch );
	}

	SetupSortByDocID ( *m_pSorterSchema.Ptr(), m_tStateMatch, nullptr, m_tSettings.m_bComputeItems );
	ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

	// find out what function to use and whether it needs attributes
	switch (m_tQuery.m_eSort )
	{
		case SPH_SORT_ATTR_DESC:	m_eMatchFunc = FUNC_ATTR_DESC; break;
		case SPH_SORT_ATTR_ASC:		m_eMatchFunc = FUNC_ATTR_ASC; break;
		case SPH_SORT_TIME_SEGMENTS:	m_eMatchFunc = FUNC_TIMESEGS; break;
		case SPH_SORT_RELEVANCE:	m_eMatchFunc = FUNC_REL_DESC; break;
		default:
			Err ( "unknown sorting mode %d", m_tQuery.m_eSort );
			return false;
	}
	return true;
}

bool QueueCreator_c::SetupGroupSortingFunc ( bool bGotDistinct )
{
	assert ( m_bGotGroupby );
	ESortClauseParseResult eRes = sphParseSortClause ( &m_tQuery,
			m_tQuery.m_sGroupSortBy.cstr(), *m_pSorterSchema.Ptr(), m_eGroupFunc,
			m_tStateGroup, m_tSettings.m_bComputeItems, m_sError );

	if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
	{
		if ( eRes==SORT_CLAUSE_RANDOM )
			m_sError = "groups can not be sorted by @random";
		return false;
	}

	ExtraAddSortkeys ( m_tStateGroup.m_dAttrs );

	assert ( m_dGroupColumns.GetLength() || m_tGroupSorterSettings.m_bImplicit );
	if ( !m_tGroupSorterSettings.m_bImplicit )
	{
		for ( const auto& dGroupColumn : m_dGroupColumns )
			m_hExtra.Add ( m_pSorterSchema->GetAttr ( dGroupColumn ).m_sName );
	}

	if ( bGotDistinct )
	{
		m_dGroupColumns.Add ( m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr() ) );
		assert ( m_dGroupColumns.Last()>=0 );
		m_hExtra.Add ( m_pSorterSchema->GetAttr ( m_dGroupColumns.Last() ).m_sName );
	}

	if ( !m_dGroupColumns.IsEmpty() ) // implicit case
		AssignOrderByToPresortStage ( m_dGroupColumns.Begin (), m_dGroupColumns.GetLength ());

	AssignOrderByToPresortStage ( m_tStateGroup.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );

	// GroupSortBy str attributes setup
	RemapStaticStringsAndJsons ( m_tStateGroup );
	return true;
}

// set up aggregate filter for grouper
ISphFilter * QueueCreator_c::CreateAggrFilter () const
{
	assert ( m_bGotGroupby );
	if ( m_pSorterSchema->GetAttr ( m_tSettings.m_pAggrFilter->m_sAttrName.cstr() ) )
		return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, m_tSettings.m_pAggrFilter->m_sAttrName,
				*m_pSorterSchema.Ptr(), m_sError );

	// having might reference aliased attributes but @* attributes got stored without alias in sorter schema
	CSphString sHaving;
	for ( const auto& tItem : m_tQuery.m_dItems )
		if ( tItem.m_sAlias==m_tSettings.m_pAggrFilter->m_sAttrName )
		{
			sHaving = tItem.m_sExpr;
			break;
		}

	if ( sHaving=="groupby()" )
		sHaving = "@groupby";
	else if ( sHaving=="count(*)" )
		sHaving = "@count";

	return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, sHaving, *m_pSorterSchema.Ptr(), m_sError );
}

void QueueCreator_c::SetupCollation ()
{
	switch (m_tQuery.m_eCollation)
	{
	case SPH_COLLATION_LIBC_CI:
		m_tStateMatch.m_fnStrCmp = sphCollateLibcCI;
		m_tStateGroup.m_fnStrCmp = sphCollateLibcCI;
		break;
	case SPH_COLLATION_LIBC_CS:
		m_tStateMatch.m_fnStrCmp = sphCollateLibcCS;
		m_tStateGroup.m_fnStrCmp = sphCollateLibcCS;
		break;
	case SPH_COLLATION_UTF8_GENERAL_CI:
		m_tStateMatch.m_fnStrCmp = sphCollateUtf8GeneralCI;
		m_tStateGroup.m_fnStrCmp = sphCollateUtf8GeneralCI;
		break;
	case SPH_COLLATION_BINARY:
		m_tStateMatch.m_fnStrCmp = sphCollateBinary;
		m_tStateGroup.m_fnStrCmp = sphCollateBinary;
		break;
	}
}

bool QueueCreator_c::AddGroupbyStuff ()
{
	////////////////////////////////////////////
	// setup groupby settings, create shortcuts
	////////////////////////////////////////////

	// need schema with group related columns however not need grouper
	m_bHeadWOGroup = ( m_tQuery.m_sGroupBy.IsEmpty () && m_tQuery.m_bFacetHead );
	auto fnIsImplicit = [] ( const CSphQueryItem & t ) {
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	bool bHasImplicitGrouping = m_tQuery.m_sGroupBy.IsEmpty ()
			? m_tQuery.m_dItems.FindFirst ( fnIsImplicit ) : false;

	// count(*) and distinct wo group by at main query should keep implicit flag
	if ( bHasImplicitGrouping && m_bHeadWOGroup )
		m_bHeadWOGroup = !m_tQuery.m_dRefItems.FindFirst ( fnIsImplicit );

	if ( !SetupGroupbySettings ( bHasImplicitGrouping ))
		return false;

	// or else, check in SetupGroupbySettings() would already fail
	m_bGotGroupby = !m_tQuery.m_sGroupBy.IsEmpty () || m_tGroupSorterSettings.m_bImplicit;
	m_bGotDistinct = ( m_tGroupSorterSettings.m_tDistinctAttr.m_iBitOffset>=0 );

	if ( m_bHasGroupByExpr && !m_bGotGroupby )
		return Err ( "GROUPBY() is allowed only in GROUP BY queries" );

	// check for HAVING constrains
	if ( !CheckHavingConstraints ())
		return false;

	// now lets add @groupby stuff, if necessary
	return MaybeAddGroupbyMagic ( m_bGotDistinct );
}

bool QueueCreator_c::SetGroupSorting ()
{
	if ( m_bGotGroupby ) {
		if ( !SetupGroupSortingFunc ( m_bGotDistinct ))
			return false;

		if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty ()) {
			auto pFilter = CreateAggrFilter ();
			if ( !pFilter )
				return false;
			m_tGroupSorterSettings.m_pAggrFilterTrait = pFilter;
		}
	}

	for ( auto& tIdx: m_hExtra )
	{
		m_hQueryColumns.Add ( tIdx.first );
		if ( m_pExtra )
			m_pExtra->Add ( tIdx.first );
	}

	return true;
}

ISphMatchSorter * QueueCreator_c::SpawnQueue()
{
	if ( !m_bGotGroupby )
	{
		if ( m_tSettings.m_pUpdate )
			return new CSphUpdateQueue ( m_tSettings.m_iMaxMatches, m_tSettings.m_pUpdate,
					m_tQuery.m_bIgnoreNonexistent, m_tQuery.m_bStrict );

		if ( m_tSettings.m_pCollection )
			return new CSphCollectQueue ( m_tSettings.m_iMaxMatches, m_tSettings.m_pCollection );

		return CreatePlainSorter ( m_eMatchFunc, m_tQuery.m_bSortKbuffer, m_tSettings.m_iMaxMatches,
			m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
	}
	return sphCreateSorter1st ( m_eMatchFunc, m_eGroupFunc, &m_tQuery, m_tGroupSorterSettings,
		m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
}

bool QueueCreator_c::SetupComputeQueue ()
{
	return MaybeAddGeodistColumn ()
		&& MaybeAddExprColumn ()
		&& MaybeAddExpressionsFromSelectList ();
}

bool QueueCreator_c::SetupGroupQueue ()
{
	return AddGroupbyStuff ()
		&& SetupMatchesSortingFunc ()
		&& SetGroupSorting ();
}

bool QueueCreator_c::SetupQueue ()
{
	return SetupComputeQueue () && SetupGroupQueue ();
}

ISphMatchSorter * QueueCreator_c::CreateQueue ()
{
	SetupCollation();

	if ( m_bHeadWOGroup && m_tGroupSorterSettings.m_bImplicit )
	{
		m_tGroupSorterSettings.m_bImplicit = false;
		m_bGotGroupby = false;
	}

	///////////////////
	// spawn the queue
	///////////////////

	ISphMatchSorter * pTop = SpawnQueue();
	if ( !pTop )
	{
		Err ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)", m_eMatchFunc, m_bGotGroupby, m_eGroupFunc );
		return nullptr;
	}

	assert ( pTop );
	pTop->SetState ( m_tStateMatch );
	pTop->SetGroupState ( m_tStateGroup );
	pTop->SetSchema ( m_pSorterSchema.LeakPtr(), false );
	pTop->m_bRandomize = m_bRandomize;
	if ( !m_bHaveStar && m_hQueryColumns.GetLength() )
		pTop->SetFilteredAttrs ( m_hQueryColumns );

	if ( m_bRandomize )
	{
		if ( m_tQuery.m_iRandSeed>=0 )
			sphSrand ( (DWORD)m_tQuery.m_iRandSeed );
		else
			sphAutoSrand();
	}

	return pTop;
}

static ISphMatchSorter * CreateQueue ( QueueCreator_c & tCreator, SphQueueRes_t & tRes )
{
	ISphMatchSorter * pSorter = tCreator.CreateQueue ();
	tRes.m_bZonespanlist = tCreator.m_bZonespanlist;
	tRes.m_uPackedFactorFlags = tCreator.m_uPackedFactorFlags;
	return pSorter;
}

bool sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema )
{
	for ( const CSphQueryItem &tItem : tQuery.m_dItems )
	{
		const CSphString & sExpr = tItem.m_sExpr;

		// all expressions that come from parser are automatically aliased
		assert ( !tItem.m_sAlias.IsEmpty() );

		if ( !( sExpr=="*"
			|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && tItem.m_sAlias==sExpr )
			|| IsGroupbyMagic ( sExpr ) ) )
			return true;
	}
	return false;
}

static void CreateSorters ( const VecTraits_T<CSphQuery> & dQueries,
	const VecTraits_T<ISphMatchSorter*> & dSorters, const VecTraits_T<QueueCreator_c> & dCreators,
	const VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes )
{
	ARRAY_FOREACH ( i, dCreators )
	{
		if ( !dCreators[i].m_bCreate )
			continue;

		dSorters[i] = CreateQueue ( dCreators[i], tRes );
		assert ( dSorters[i]!=nullptr );
	}

	if ( tRes.m_bAlowMulti )
	{
		ISphMatchSorter * pSorter0 = nullptr;
		for ( int iCheck=0; iCheck<dSorters.GetLength(); ++iCheck )
		{
			if ( !dCreators[iCheck].m_bCreate )
				continue;

			assert ( dSorters[iCheck] );
			if ( !pSorter0 )
			{
				pSorter0 = dSorters[iCheck];
				continue;
			}

			assert ( dSorters[iCheck]->GetSchema()->GetAttrsCount()==pSorter0->GetSchema()->GetAttrsCount() );
		}
	}
}

ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra )
{
	QueueCreator_c tCreator ( tQueue, tQuery, sError, pExtra );
	if ( !tCreator.SetupQueue () )
		return nullptr;

	return CreateQueue ( tCreator, tRes );
}

template<typename T>
using RawVector_T = sph::Vector_T<T, sph::SwapCopy_T<T>, sph::DefaultRelimit, sph::RawStorage_T<T>>;

void CreateMultiQueue ( RawVector_T<QueueCreator_c>& dCreators, const SphQueueSettings_t & tQueue,
		const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter*> & dSorters,
		VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, VecTraits_T<StrVec_t> & dExtras )
{
	assert ( dSorters.GetLength()>1 );
	assert ( dSorters.GetLength()==dQueries.GetLength() );
	assert ( dExtras.IsEmpty() || dSorters.GetLength()==dExtras.GetLength() );
	assert ( dSorters.GetLength()==dErrors.GetLength() );

	dCreators.Reserve_static ( dSorters.GetLength () );
	dCreators.Emplace_back( tQueue, dQueries[0], dErrors[0], dExtras.IsEmpty () ? nullptr : dExtras.begin () );
	dCreators[0].m_bMulti = true;

	// same as SetupQueue
	dCreators[0].SetupComputeQueue ();
	// copy schema WO group by and internals
	CSphRsetSchema tRefSchema = dCreators[0].SorterSchema();
	bool bHasJson = dCreators[0].HasJson();
	bool bJsonMixed = false;

	dCreators[0].SetupGroupQueue ();

	// create rest of schemas
	for ( int i=1; i<dSorters.GetLength(); ++i )
	{
		// fill extra only for initial pass
		dCreators.Emplace_back ( tQueue, dQueries[i], dErrors[i], dExtras.IsEmpty () ? nullptr : &dExtras[i] );
		dCreators[i].m_bMulti = true;
		if ( !dCreators[i].SetupQueue () )
		{
			dCreators[i].m_bCreate = false;
			continue;
		}

		bJsonMixed |= ( bHasJson!=dCreators[i].HasJson () );
		bHasJson |= dCreators[i].HasJson();
	}

	// FIXME!!! check attributes and expressions matches
	bool bSame = !bJsonMixed;
	const auto& tSchema0 = dCreators[0].SorterSchema();
	for ( int i=1; i<dCreators.GetLength() && bSame; ++i )
	{
		const auto & tCur = dCreators[i].SorterSchema();
		bSame &= ( tSchema0.GetDynamicSize()==tCur.GetDynamicSize() && tSchema0.GetAttrsCount()==tCur.GetAttrsCount() );
	}

	// same schemes
	if ( bSame )
		return;

	CSphRsetSchema tMultiSchema = tRefSchema;

	int iMinGroups = INT_MAX;
	int iMaxGroups = 0;
	bool bHasMulti = false;
	ARRAY_FOREACH ( iSchema, dCreators )
	{
		if ( !dCreators[iSchema].m_bCreate )
			continue;

		int iGroups = 0;
		const CSphRsetSchema & tSchema = dCreators[iSchema].SorterSchema();
		for ( int iCol=0; iCol<tSchema.GetAttrsCount(); ++iCol )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr ( iCol );
			if ( !tCol.m_tLocator.m_bDynamic )
				continue;

			if ( IsGroupbyMagic ( tCol.m_sName ) )
			{
				++iGroups;
				if ( !IsSortJsonInternal ( tCol.m_sName ))
					continue;
			}

			const CSphColumnInfo * pMultiCol = tMultiSchema.GetAttr ( tCol.m_sName.cstr() );
			if ( pMultiCol )
			{
				bool bDisable1 = false;
				bool bDisable2 = false;
				// no need to add attributes that already exists
				if ( pMultiCol->m_eAttrType==tCol.m_eAttrType &&
					pMultiCol->m_pExpr->GetHash ( tMultiSchema, SPH_FNV64_SEED, bDisable1 )==tCol.m_pExpr->GetHash ( tSchema, SPH_FNV64_SEED, bDisable2 ) )
					continue;

				// if attr or expr differs need to create regular sorters and issue search WO multi-query
				tRes.m_bAlowMulti = false;
				return;
			}

			bHasMulti = true;
			tMultiSchema.AddAttr ( tCol, true );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->FixupLocator ( &tSchema, &tMultiSchema );

		}

		iMinGroups = Min ( iMinGroups, iGroups );
		iMaxGroups = Max ( iMaxGroups, iGroups );
	}

	// usual multi query should all have similar group by
	if ( iMinGroups!=iMaxGroups && !dQueries[0].m_bFacetHead && !dQueries[0].m_bFacet )
	{
		tRes.m_bAlowMulti = false;
		return;
	}

	// only group attributes differs - create regular sorters
	if ( !bHasMulti && !bJsonMixed )
		return;

	// setup common schemas
	for ( auto& dCreator : dCreators )
	{
		if ( !dCreator.m_bCreate )
			continue;

		dCreator.SorterSchema() = tMultiSchema;
		if ( !dCreator.SetupGroupQueue () )
			dCreator.m_bCreate = false;
	}
}

void sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries
		, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes
		, VecTraits_T<StrVec_t> & dExtras )
{
	RawVector_T<QueueCreator_c> dCreators;
	CreateMultiQueue ( dCreators, tQueue, dQueries, dSorters, dErrors, tRes, dExtras );
	CreateSorters ( dQueries, dSorters, dCreators, dErrors, tRes );
}