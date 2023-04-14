//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "sphinxint.h"
#include "sphinxjson.h"
#include "attribute.h"
#include "collation.h"
#include "memio.h"
#include "columnargrouper.h"
#include "columnarexpr.h"
#include "exprtraits.h"
#include "columnarsort.h"
#include "sortcomp.h"
#include "conversion.h"
#include "docstore.h"
#include "schema/rset.h"
#include "aggregate.h"

#include <time.h>
#include <math.h>

#if !_WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

static bool g_bAccurateAggregation = false;

void SetAccurateAggregationDefault ( bool bEnabled )
{
	g_bAccurateAggregation = bEnabled;
}


bool GetAccurateAggregationDefault()
{
	return g_bAccurateAggregation;
}


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

const char g_sIntAttrPrefix[] = "@int_attr_";
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

class TransformedSchemaBuilder_c
{
public:
	TransformedSchemaBuilder_c ( const ISphSchema & tOldSchema, CSphSchema & tNewSchema );

	void	AddAttr ( const CSphString & sName );

private:
	const ISphSchema &	m_tOldSchema;
	CSphSchema &		m_tNewSchema;

	void	ReplaceColumnarAttrWithExpression ( CSphColumnInfo & tAttr, int iLocator );
};


TransformedSchemaBuilder_c::TransformedSchemaBuilder_c ( const ISphSchema & tOldSchema, CSphSchema & tNewSchema )
	: m_tOldSchema ( tOldSchema )
	, m_tNewSchema ( tNewSchema )
{}


void TransformedSchemaBuilder_c::AddAttr ( const CSphString & sName )
{
	const CSphColumnInfo * pAttr = m_tOldSchema.GetAttr ( sName.cstr() );
	if ( !pAttr )
		return;

	CSphColumnInfo tAttr = *pAttr;
	tAttr.m_tLocator.Reset();

	// check if new columnar attributes were added (that were not in the select list originally)
	if ( tAttr.IsColumnar() )
		ReplaceColumnarAttrWithExpression ( tAttr, m_tNewSchema.GetAttrsCount() );

	tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );

	m_tNewSchema.AddAttr ( tAttr, true );
}


void TransformedSchemaBuilder_c::ReplaceColumnarAttrWithExpression ( CSphColumnInfo & tAttr, int iLocator )
{
	assert ( tAttr.IsColumnar() );
	assert ( !tAttr.m_pExpr );

	// temporarily add attr to new schema
	// when result set is finalized, corresponding columnar expression (will be spawned later)
	// will be evaluated and put into the match
	// and this expression will be used to fetch that value
	tAttr.m_uAttrFlags &= ~CSphColumnInfo::ATTR_COLUMNAR;
	tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );
	m_tNewSchema.AddAttr ( tAttr, true );

	// parse expression as if it is not columnar
	CSphString		 sError;
	ExprParseArgs_t	 tExprArgs;
	tAttr.m_pExpr = sphExprParse ( tAttr.m_sName.cstr(), m_tNewSchema, sError, tExprArgs );
	assert ( tAttr.m_pExpr );

	// now remove it from schema (it will be added later with the supplied expression)
	m_tNewSchema.RemoveAttr( tAttr.m_sName.cstr(), true );
}

//////////////////////////////////////////////////////////////////////////

class MatchesToNewSchema_c : public MatchProcessor_i
{
public:
		MatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, GetBlobPoolFromMatch_fn fnGetBlobPool, GetColumnarFromMatch_fn fnGetColumnar );

	// performs actual processing according created plan
	void Process ( CSphMatch * pMatch ) final			{ ProcessMatch(pMatch); }
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }
	bool ProcessInRowIdOrder() const final				{ return m_dActions.any_of ( []( const MapAction_t & i ){ return i.IsExprEval(); } ); }

private:
	struct MapAction_t
	{
		// what is to do with current position
		enum Action_e
		{
			SETZERO,		// set default (0)
			COPY,			// copy as is (plain attribute)
			COPYBLOB,		// deep copy (unpack/pack) the blob
			COPYJSONFIELD,	// json field (packed blob with type)
			EVALEXPR_INT,	// evaluate the expression for the recently added int attribute
			EVALEXPR_BIGINT,// evaluate the expression for the recently added bigint attribute
			EVALEXPR_STR,	// evaluate the expression for the recently added string attribute
			EVALEXPR_MVA	// evaluate the expression for the recently added mva attribute
		};

		const CSphAttrLocator *	m_pFrom;
		const CSphAttrLocator *	m_pTo;
		ISphExprRefPtr_c		m_pExpr;
		Action_e				m_eAction;

		mutable columnar::Columnar_i * m_pPrevColumnar = nullptr;

		bool IsExprEval() const
		{
			return m_eAction==EVALEXPR_INT || m_eAction==EVALEXPR_BIGINT || m_eAction==EVALEXPR_STR || m_eAction==EVALEXPR_MVA;
		}
	};

	int						m_iDynamicSize;		// target dynamic size, from schema
	CSphVector<MapAction_t>	m_dActions;			// the recipe
	CSphVector<std::pair<CSphAttrLocator, CSphAttrLocator>> m_dRemapCmp;	// remap @int_attr_ATTR -> ATTR
	CSphVector<int>			m_dDataPtrAttrs;	// orphaned attrs we have to free before swap to new attr
	GetBlobPoolFromMatch_fn	m_fnGetBlobPool;	// provides base for pool copying
	GetColumnarFromMatch_fn	m_fnGetColumnar;	// columnar storage getter

	static void SetupAction ( const CSphColumnInfo & tOld, const CSphColumnInfo & tNew, const ISphSchema * pOldSchema, MapAction_t & tAction );
	inline void	ProcessMatch ( CSphMatch * pMatch );

	inline static void PerformAction ( const MapAction_t & tAction, CSphMatch * pMatch, CSphMatch & tResult, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar );
};


MatchesToNewSchema_c::MatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, GetBlobPoolFromMatch_fn fnGetBlobPool, GetColumnarFromMatch_fn fnGetColumnar )
	: m_iDynamicSize ( pNewSchema->GetDynamicSize () )
	, m_fnGetBlobPool ( std::move ( fnGetBlobPool ) )
	, m_fnGetColumnar ( std::move ( fnGetColumnar ) )
{
	assert ( pOldSchema && pNewSchema );

	// prepare transforming recipe

	// initial state: set all new columns to be reset by default
	for ( int i = 0; i<pNewSchema->GetAttrsCount(); ++i )
		m_dActions.Add ( { nullptr, &pNewSchema->GetAttr(i).m_tLocator, nullptr, MapAction_t::SETZERO } );

	// add mapping from old to new according to column type
	for ( int i = 0; i<pOldSchema->GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tOld = pOldSchema->GetAttr(i);
		auto iNewIdx = pNewSchema->GetAttrIndex ( tOld.m_sName.cstr () );
		if ( iNewIdx == -1 )
		{
			// dataptr present in old, but not in the new - mark it for releasing
			if ( sphIsDataPtrAttr ( tOld.m_eAttrType ) && tOld.m_tLocator.m_bDynamic )
				m_dDataPtrAttrs.Add( tOld.m_tLocator.m_iBitOffset >> ROWITEM_SHIFT );
			continue;
		}

		const CSphColumnInfo & tNew = pNewSchema->GetAttr(iNewIdx);

		auto & tAction = m_dActions[iNewIdx];

		SetupAction ( tOld, tNew, pOldSchema, tAction );
	}

	// need to update @int_attr_ locator to use new schema
	// no need to pass pOldSchema as we remap only new schema pointers
	// also need to update group sorter keypart to be str_ptr in caller code SetSchema
	FnSortGetStringRemap ( *pNewSchema, *pNewSchema, [this, pNewSchema] ( int iSrc, int iDst )
		{
			m_dRemapCmp.Add ( { pNewSchema->GetAttr(iSrc).m_tLocator, pNewSchema->GetAttr(iDst).m_tLocator } );
		} );
}


void MatchesToNewSchema_c::SetupAction ( const CSphColumnInfo & tOld, const CSphColumnInfo & tNew, const ISphSchema * pOldSchema, MapAction_t & tAction )
{
	tAction.m_pFrom = &tOld.m_tLocator;

	// columnar attr replaced by an expression
	// we now need to create an expression that fetches data from columnar storage
	if ( tOld.IsColumnar() && tNew.m_pExpr )
	{
		CSphString		sError;
		ExprParseArgs_t tExprArgs;
		tAction.m_pExpr =  sphExprParse ( tOld.m_sName.cstr(), *pOldSchema, sError, tExprArgs );
		assert ( tAction.m_pExpr );

		switch ( tNew.m_eAttrType )
		{
		case SPH_ATTR_STRINGPTR:	tAction.m_eAction = MapAction_t::EVALEXPR_STR;		break;
		case SPH_ATTR_BIGINT:		tAction.m_eAction = MapAction_t::EVALEXPR_BIGINT;	break;
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET_PTR:	tAction.m_eAction = MapAction_t::EVALEXPR_MVA;		break;
		default:					tAction.m_eAction = MapAction_t::EVALEXPR_INT;		break;
		}

		return;
	}

	// same type - just copy attr as is
	if ( tOld.m_eAttrType==tNew.m_eAttrType )
	{
		tAction.m_eAction = MapAction_t::COPY;
		return;
	}

	assert ( !sphIsDataPtrAttr ( tOld.m_eAttrType ) && sphIsDataPtrAttr ( tNew.m_eAttrType ) );

	if ( tOld.m_eAttrType==SPH_ATTR_JSON_FIELD )
		tAction.m_eAction = MapAction_t::COPYJSONFIELD;
	else
		tAction.m_eAction = MapAction_t::COPYBLOB;
}


void MatchesToNewSchema_c::ProcessMatch ( CSphMatch * pMatch )
{
	CSphMatch tResult;
	tResult.Reset ( m_iDynamicSize );

	const BYTE * pBlobPool = m_fnGetBlobPool(pMatch);
	columnar::Columnar_i * pColumnar = m_fnGetColumnar(pMatch);
	for ( const auto & tAction : m_dActions )
		PerformAction ( tAction, pMatch, tResult, pBlobPool, pColumnar );

	// remap comparator attributes
	for ( const auto & tRemap : m_dRemapCmp )
		tResult.SetAttr ( tRemap.second, tResult.GetAttr ( tRemap.first ) );

	// free original orphaned pointers
	CSphSchemaHelper::FreeDataSpecial ( *pMatch, m_dDataPtrAttrs );

	Swap ( pMatch->m_pDynamic, tResult.m_pDynamic );
	pMatch->m_pStatic = nullptr;
}


inline void MatchesToNewSchema_c::PerformAction ( const MapAction_t & tAction, CSphMatch * pMatch, CSphMatch & tResult, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar )
{
	// try to minimize columnar switches inside the expression as this leads to recreating iterators
	if ( tAction.IsExprEval() && pColumnar!=tAction.m_pPrevColumnar )
	{
		tAction.m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );
		tAction.m_pPrevColumnar = pColumnar;
	}

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

	case MapAction_t::EVALEXPR_INT:
		uValue = (SphAttr_t)tAction.m_pExpr->IntEval(*pMatch);
		break;

	case MapAction_t::EVALEXPR_BIGINT:
		uValue = (SphAttr_t)tAction.m_pExpr->Int64Eval(*pMatch);
		break;

	case MapAction_t::EVALEXPR_STR:
		uValue = (SphAttr_t)tAction.m_pExpr->StringEvalPacked(*pMatch);
		break;

	case MapAction_t::EVALEXPR_MVA:
		uValue = (SphAttr_t)tAction.m_pExpr->Int64Eval(*pMatch);
		break;

	default:
		assert(false && "Unknown state");
	}

	tResult.SetAttr ( *tAction.m_pTo, uValue );
}

//////////////////////////////////////////////////////////////////////////

class MatchSorter_c : public ISphMatchSorter
{
public:
	void				SetState ( const CSphMatchComparatorState & tState ) override;
	const CSphMatchComparatorState & GetState() const override { return m_tState; }

	void				SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override;
	const ISphSchema *	GetSchema() const override { return ( ISphSchema *) m_pSchema; }

	void				SetColumnar ( columnar::Columnar_i * pColumnar ) override { m_pColumnar = pColumnar; }
	int64_t				GetTotalCount() const override { return m_iTotal; }
	void				CloneTo ( ISphMatchSorter * pTrg ) const override;
	bool				CanBeCloned() const override;
	void				SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override;
	void				TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override;

	void				SetRandom ( bool bRandom ) override { m_bRandomize = bRandom; }
	bool				IsRandom() const override { return m_bRandomize; }

	int					GetMatchCapacity() const override { return m_iMatchCapacity; }

	RowTagged_t					GetJustPushed() const override { return m_tJustPushed; }
	VecTraits_T<RowTagged_t>	GetJustPopped() const override { return m_dJustPopped; }

protected:
	SharedPtr_t<ISphSchema>		m_pSchema;		///< sorter schema (adds dynamic attributes on top of index schema)
	CSphMatchComparatorState	m_tState;		///< protected to set m_iNow automatically on SetState() calls
	StrVec_t					m_dTransformed;

	columnar::Columnar_i *		m_pColumnar = nullptr;

	bool						m_bRandomize = false;
	int64_t						m_iTotal = 0;

	int							m_iMatchCapacity = 0;
	RowTagged_t						m_tJustPushed;
	CSphTightVector<RowTagged_t>	m_dJustPopped;
};


void MatchSorter_c::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	assert ( pSchema );
	m_tState.FixupLocators ( m_pSchema, pSchema, bRemapCmp );

	m_pSchema = pSchema;
}


void MatchSorter_c::SetState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	m_tState.m_iNow = (DWORD) time ( nullptr );
}


void MatchSorter_c::CloneTo ( ISphMatchSorter * pTrg ) const
{
	assert ( pTrg );
	pTrg->SetRandom(m_bRandomize);
	pTrg->SetState(m_tState);
	pTrg->SetSchema ( m_pSchema->CloneMe(), false );
}


bool MatchSorter_c::CanBeCloned() const
{
	if ( !m_pSchema )
		return true;

	bool bGotStatefulUDF = false;
	for ( int i = 0; i < m_pSchema->GetAttrsCount() && !bGotStatefulUDF; i++ )
	{
		auto & pExpr = m_pSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->Command ( SPH_EXPR_GET_STATEFUL_UDF, &bGotStatefulUDF );
	}

	return !bGotStatefulUDF;
}


void MatchSorter_c::SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid )
{
	assert ( m_pSchema );

	m_dTransformed.Reserve ( hAttrs.GetLength() );

	if ( bAddDocid && !hAttrs[sphGetDocidName()] )
		m_dTransformed.Add ( sphGetDocidName() );

	for ( auto & tName : hAttrs )
	{
		const CSphColumnInfo * pCol = m_pSchema->GetAttr ( tName.first.cstr() );
		if ( pCol )
			m_dTransformed.Add ( pCol->m_sName );
	}
}

void MatchSorter_c::TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters )
{
	auto * pOldSchema = GetSchema();
	assert ( pOldSchema );

	// create new standalone schema (from old, or from filtered)
	auto * pNewSchema = new CSphSchema ( "standalone" );
	for ( int i = 0; i<pOldSchema->GetFieldsCount (); ++i )
		pNewSchema->AddField ( pOldSchema->GetField(i) );

	TransformedSchemaBuilder_c tBuilder ( *pOldSchema, *pNewSchema );

	if ( m_dTransformed.IsEmpty() )
	{
		// keep id as the first attribute
		for ( int i = 0; i<pOldSchema->GetAttrsCount (); i++ )
		{
			const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
			if ( tAttr.m_sName==sphGetDocidName() )
			{
				tBuilder.AddAttr ( tAttr.m_sName );
				break;
			}
		}

		// add the rest
		for ( int i = 0; i<pOldSchema->GetAttrsCount (); i++ )
		{
			const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
			if ( tAttr.m_sName!=sphGetDocidName() )
				tBuilder.AddAttr ( tAttr.m_sName );
		}
	}
	else
	{
		// keep id as the first attribute
		for ( const CSphString & sName : m_dTransformed )
			if ( sName==sphGetDocidName() )
			{
				tBuilder.AddAttr(sName);
				break;
			}

		// add the rest
		for ( const CSphString & sName : m_dTransformed )
			if ( sName!=sphGetDocidName() )
				tBuilder.AddAttr(sName);
	}

	for ( int i = 0; i <pNewSchema->GetAttrsCount(); ++i )
	{
		auto & pExpr = pNewSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	MatchesToNewSchema_c fnFinal ( pOldSchema, pNewSchema, std::move ( fnBlobPoolFromMatch ), std::move ( fnGetColumnarFromMatch ) );
	Finalize ( fnFinal, false, bFinalizeSorters );
	SetSchema ( pNewSchema, true );
}

//////////////////////////////////////////////////////////////////////////
/// match-sorting priority queue traits
class CSphMatchQueueTraits : public MatchSorter_c, ISphNoncopyable
{
protected:
	int							m_iSize;	// size of internal struct we can operate
	CSphFixedVector<CSphMatch>	m_dData;
	CSphTightVector<int>		m_dIData;	// indexes into m_pData, to avoid extra moving of matches themselves

public:
	/// ctor
	explicit CSphMatchQueueTraits ( int iSize )
		: m_iSize ( iSize )
		, m_dData { iSize }
	{
		assert ( iSize>0 );
		m_iMatchCapacity = iSize;
		m_dIData.Resize ( iSize );
		m_tState.m_iNow = (DWORD) time ( nullptr );

		ARRAY_FOREACH ( i, m_dIData )
			m_dIData[i] = i;
		m_dIData.Resize ( 0 );
	}

	/// dtor make FreeDataPtrs here, then ResetDynamic also get called on m_dData d-tr.
	~CSphMatchQueueTraits () override
	{
		if ( m_pSchema )
			m_dData.Apply ( [this] ( CSphMatch& tMatch ) { m_pSchema->FreeDataPtrs ( tMatch ); } );
	}

public:
	int			GetLength() override								{ return Used(); }

	// helper
	void SwapMatchQueueTraits ( CSphMatchQueueTraits& rhs )
	{
		// ISphMatchSorter
		::Swap ( m_iTotal, rhs.m_iTotal );

		// CSphMatchQueueTraits
		m_dData.SwapData ( rhs.m_dData );
		m_dIData.SwapData ( rhs.m_dIData );
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
		// proper ids at m_dIData already set at constructor
		// they will be same during life-span - that is why Add used like anti-Pop
		int iLast = m_dIData.Add();
		return m_dData[iLast];
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
		if_const ( NOTIFICATIONS )
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
//			m_pSchema->FreeDataPtrs ( *pTo );
			PopAndProcess_T ( [pTo] ( CSphMatch & tRoot ) { Swap ( *pTo, tRoot ); return true; } );
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

		if_const ( NOTIFICATIONS )
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

		if_const ( NOTIFICATIONS )
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
			if_const ( NOTIFICATIONS )
			{
				if ( m_dJustPopped.IsEmpty () )
					m_dJustPopped.Add (  RowTagged_t ( m_dData[iJustRemoved] ) );
				else
					m_dJustPopped[0] =  RowTagged_t ( m_dData[iJustRemoved] );
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
		if_const ( NOTIFICATIONS )
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

		for ( int i = 0; i<m_iMaxUsed; ++i )
			m_dData[i].ResetDynamic ();
		m_iMaxUsed = -1;

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
		if_const ( NOTIFICATIONS )
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
		if_const ( NOTIFICATIONS )
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

		if_const ( NOTIFICATIONS )
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

bool IsGroupbyMagic ( const CSphString & s )
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
		void			GetLocator ( CSphAttrLocator & tOut ) const override { tOut = m_tLocator; } \
		ESphAttr		GetResultType() const override  { return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; } \
		SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const override { return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); } \
		void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override { assert(0); } \
        CSphGrouper *	Clone() const override { return new _name (m_tLocator); } \
		SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override \
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

static bool g_bSortGroupingInUtc = false;

void SetGroupingInUtcSort ( bool bGroupingInUtc )
{
	g_bSortGroupingInUtc = bGroupingInUtc;
}


static CSphGrouper * getDayGrouper ( const CSphAttrLocator &tLoc )
{
	return g_bSortGroupingInUtc
			? (CSphGrouper *) new CSphGrouperDayUtc ( tLoc )
			: (CSphGrouper *) new CSphGrouperDay ( tLoc );
}

static CSphGrouper * getWeekGrouper ( const CSphAttrLocator &tLoc )
{
	return g_bSortGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperWeekUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperWeek ( tLoc );
}

static CSphGrouper * getMonthGrouper ( const CSphAttrLocator &tLoc )
{
	return g_bSortGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperMonthUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperMonth ( tLoc );
}

static CSphGrouper * getYearGrouper ( const CSphAttrLocator &tLoc )
{
	return g_bSortGroupingInUtc
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
		if ( IsEmpty ( dBlobAttr ) )
			return 0;

		return PRED::Hash ( dBlobAttr.first,dBlobAttr.second );
	}

	CSphGrouper * Clone () const final
	{
		return new CSphGrouperString ( m_tLocator );
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

	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { assert(0); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	CSphGrouper *	Clone() const final { return new CSphGrouperJsonField (*this); }

protected:
	CSphGrouperJsonField ( const CSphGrouperJsonField& rhs )
		: m_tLocator ( rhs.m_tLocator )
		, m_pExpr ( SafeClone ( rhs.m_pExpr ) )
	{}

	CSphAttrLocator		m_tLocator;
	ISphExprRefPtr_c	m_pExpr;
};


template <class PRED>
class GrouperStringExpr_T final : public CSphGrouper, public PRED
{
public:
	explicit GrouperStringExpr_T ( ISphExpr * pExpr )
		: m_pExpr ( pExpr )
	{
		assert(m_pExpr);
		SafeAddRef(pExpr);
	}

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return SPH_ATTR_BIGINT; }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { assert(0); }

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const final
	{
		assert ( !m_pExpr->IsDataPtrAttr() );
		const BYTE * pStr = nullptr;
		int iLen = m_pExpr->StringEval ( tMatch, &pStr );
		if ( !iLen )
			return 0;

		return PRED::Hash ( pStr, iLen );
	}

	CSphGrouper *	Clone() const final { return new GrouperStringExpr_T(*this); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final { m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar ); }

protected:
	GrouperStringExpr_T (const GrouperStringExpr_T& rhs)
		: m_pExpr { SafeClone ( rhs.m_pExpr ) }
	{}

	ISphExprRefPtr_c	m_pExpr;
};

template <typename MVA, typename ADDER>
static void AddGroupedMVA ( ADDER && fnAdd, const ByteBlob_t& dRawMVA )
{
	VecTraits_T<MVA> dMvas {dRawMVA};
	for ( auto & tValue : dMvas )
		fnAdd ( sphUnalignedRead(tValue) );
}


template <typename PUSH>
static bool PushJsonField ( int64_t iValue, const BYTE * pBlobPool, PUSH && fnPush )
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
		auto p = (const int*)pValue;
		for ( int i=0;i<iLen;i++ )
		{
			int64_t iPacked = sphJsonPackTypeOffset ( JSON_INT32, (const BYTE*)p-pBlobPool );
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
		auto p = (const int64_t*)pValue;
		ESphJsonType eType = eJson==JSON_INT64_VECTOR ? JSON_INT64 : JSON_DOUBLE;
		for ( int i=0;i<iLen;i++ )
		{
			int64_t iPacked = sphJsonPackTypeOffset ( eType, (const BYTE*)p-pBlobPool );
			uGroupKey = *p++;
			bRes |= fnPush ( &iPacked, uGroupKey );
		}
		return bRes;
	}

	case JSON_TRUE:
	case JSON_FALSE:
		uGroupKey = eJson;
		return fnPush ( &iValue, uGroupKey );

	default:
		uGroupKey = 0;
		iValue = 0;
		return fnPush ( &iValue, uGroupKey );
	}
}

template<typename T>
void FetchMVAKeys ( CSphVector<SphGroupKey_t> & dKeys, const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool )
{
	dKeys.Resize(0);

	if ( !pBlobPool )
		return;

	int iLengthBytes = 0;
	const BYTE * pMva = sphGetBlobAttr ( tMatch, tLocator, pBlobPool, iLengthBytes );
	int iNumValues = iLengthBytes / sizeof(T);
	const T * pValues = (const T*)pMva;

	dKeys.Resize(iNumValues);
	for ( int i = 0; i<iNumValues; i++ )
		dKeys[i] = (SphGroupKey_t)pValues[i];
}

template <class PRED, bool HAVE_COLUMNAR>
class CSphGrouperMulti final: public CSphGrouper, public PRED
{
	using MYTYPE = CSphGrouperMulti<PRED,HAVE_COLUMNAR>;

public:
					CSphGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation );

	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	void			SetBlobPool ( const BYTE * pBlobPool ) final;
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final;
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			GetLocator ( CSphAttrLocator & ) const final { assert(0); }
	ESphAttr		GetResultType() const final { return SPH_ATTR_BIGINT; }
	bool			IsMultiValue() const final;

private:
	CSphVector<CSphColumnInfo>	m_dAttrs;
	VecRefPtrs_t<ISphExpr *>	m_dJsonKeys;
	ESphCollation				m_eCollation = SPH_COLLATION_DEFAULT;
	CSphVector<CSphRefcountedPtr<CSphGrouper>> m_dSingleKeyGroupers;
	CSphVector<CSphRefcountedPtr<CSphGrouper>> m_dMultiKeyGroupers;

	SphGroupKey_t	FetchStringKey ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, SphGroupKey_t tPrevKey ) const;
	void			SpawnColumnarGroupers();
};

template <class PRED, bool HAVE_COLUMNAR>
CSphGrouperMulti<PRED,HAVE_COLUMNAR>::CSphGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation )
	: m_dAttrs ( dAttrs )
	, m_dJsonKeys ( std::move(dJsonKeys) )
	, m_eCollation ( eCollation )
{
	assert ( dAttrs.GetLength()>1 );
	assert ( dAttrs.GetLength()==m_dJsonKeys.GetLength() );
	
	if constexpr ( HAVE_COLUMNAR )
		SpawnColumnarGroupers();
}

template <class PRED, bool HAVE_COLUMNAR>
SphGroupKey_t CSphGrouperMulti<PRED,HAVE_COLUMNAR>::KeyFromMatch ( const CSphMatch & tMatch ) const
{
	auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;

	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		if constexpr ( HAVE_COLUMNAR )
		{
			if ( m_dSingleKeyGroupers[i] )
			{
				// use pre-spawned grouper
				SphGroupKey_t tColumnarKey = m_dSingleKeyGroupers[i]->KeyFromMatch(tMatch);
				tKey = ( SphGroupKey_t ) sphFNV64 ( tColumnarKey, tKey );
				continue;
			}
		}

		switch ( m_dAttrs[i].m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			tKey = FetchStringKey ( tMatch, m_dAttrs[i].m_tLocator, tKey );
			break;

		default:
			{
				SphAttr_t tAttr = tMatch.GetAttr ( m_dAttrs[i].m_tLocator );
				tKey = ( SphGroupKey_t ) sphFNV64 ( tAttr, tKey );
			}
			break;
		}
	}

	return tKey;
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED, HAVE_COLUMNAR>::SetBlobPool ( const BYTE * pBlobPool )
{
	CSphGrouper::SetBlobPool ( pBlobPool );
	for ( auto & i : m_dJsonKeys )
		if ( i )
			i->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	CSphGrouper::SetColumnar ( pColumnar );

	for ( auto & i : m_dSingleKeyGroupers )
		if ( i )
			i->SetColumnar ( pColumnar );

	for ( auto & i : m_dMultiKeyGroupers )
		if ( i )
			i->SetColumnar ( pColumnar );
}

template <class PRED, bool HAVE_COLUMNAR>
CSphGrouper * CSphGrouperMulti<PRED,HAVE_COLUMNAR>::Clone() const
{
	VecRefPtrs_t<ISphExpr *> dJsonKeys;
	m_dJsonKeys.for_each ( [&dJsonKeys] ( ISphExpr * p ) { dJsonKeys.Add ( SafeClone ( p ) ); } );
	return new MYTYPE ( m_dAttrs, std::move(dJsonKeys), m_eCollation );
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	dKeys.Resize(0);

	CSphFixedVector<CSphVector<SphGroupKey_t>> dAllKeys { m_dAttrs.GetLength() };
	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		auto & dCurKeys = dAllKeys[i];

		if constexpr ( HAVE_COLUMNAR )
		{
			if ( m_dMultiKeyGroupers[i] )
			{
				// use pre-spawned grouper
				m_dMultiKeyGroupers[i]->MultipleKeysFromMatch ( tMatch, dCurKeys );
				continue;
			}
		}

		switch ( m_dAttrs[i].m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
			FetchMVAKeys<DWORD> ( dCurKeys, tMatch, m_dAttrs[i].m_tLocator, GetBlobPool() );
			break;

		case SPH_ATTR_INT64SET:
			FetchMVAKeys<int64_t> ( dCurKeys, tMatch, m_dAttrs[i].m_tLocator, GetBlobPool() );
			break;

		case SPH_ATTR_JSON:
			PushJsonField ( m_dJsonKeys[i]->Int64Eval(tMatch), m_pBlobPool, [&dCurKeys]( SphAttr_t * pAttr, SphGroupKey_t uMatchGroupKey ){ dCurKeys.Add(uMatchGroupKey); return true; } );
			break;

		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			{
				SphGroupKey_t tStringKey = FetchStringKey ( tMatch, m_dAttrs[i].m_tLocator, SPH_FNV64_SEED );
				if ( tStringKey!=(SphGroupKey_t)SPH_FNV64_SEED )
					dAllKeys[i].Add ( tStringKey );
			}
			break;

		default:
			dAllKeys[i].Add ( tMatch.GetAttr ( m_dAttrs[i].m_tLocator ) );
			break;
		}
	}

	CSphFixedVector<int> dIndexes { m_dAttrs.GetLength() };
	dIndexes.ZeroVec();

	do
	{
		auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;
		ARRAY_FOREACH ( i, dAllKeys )
			if ( dAllKeys[i].GetLength() )
				tKey = (SphGroupKey_t)sphFNV64 ( dAllKeys[i][dIndexes[i]], tKey );

		dKeys.Add(tKey);
	}
	while ( NextSet ( dIndexes, dAllKeys ) );
}

template <class PRED, bool HAVE_COLUMNAR>
bool CSphGrouperMulti<PRED,HAVE_COLUMNAR>::IsMultiValue() const
{
	return m_dAttrs.any_of ( []( auto & tAttr ){ return tAttr.m_eAttrType==SPH_ATTR_JSON || tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET; } );
}

template <class PRED, bool HAVE_COLUMNAR>
SphGroupKey_t CSphGrouperMulti<PRED,HAVE_COLUMNAR>::FetchStringKey ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, SphGroupKey_t tPrevKey ) const
{
	ByteBlob_t tData = tMatch.FetchAttrData ( tLocator, GetBlobPool() );
	if ( !tData.first || !tData.second )
		return tPrevKey;

	return PRED::Hash ( tData.first, tData.second, tPrevKey );
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::SpawnColumnarGroupers()
{
	m_dSingleKeyGroupers.Resize ( m_dAttrs.GetLength() );
	m_dMultiKeyGroupers.Resize ( m_dAttrs.GetLength() );

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const auto & tAttr = m_dAttrs[i];
		if ( !tAttr.IsColumnar() && !tAttr.IsColumnarExpr() )
			continue;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			m_dSingleKeyGroupers[i] = CreateGrouperColumnarString ( tAttr, m_eCollation );
			break;

		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_INT64SET_PTR:
			m_dMultiKeyGroupers[i] = CreateGrouperColumnarMVA(tAttr);
			break;

		default:
			m_dSingleKeyGroupers[i] = CreateGrouperColumnarInt(tAttr);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

template<typename T>
class GrouperMVA_T : public CSphGrouper
{
public:
					explicit GrouperMVA_T ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}

	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const override					{ assert(0); return SphGroupKey_t(); }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const override	{ assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override;
	void			GetLocator ( CSphAttrLocator & tOut ) const override { tOut = m_tLocator; }
	ESphAttr		GetResultType () const override;
	CSphGrouper *	Clone() const override { return new GrouperMVA_T ( m_tLocator ); }
	bool			IsMultiValue() const override { return true; }

private:
	CSphAttrLocator	m_tLocator;
};

template<>
ESphAttr GrouperMVA_T<DWORD>::GetResultType() const
{
	return SPH_ATTR_INTEGER;
}

template<>
ESphAttr GrouperMVA_T<int64_t>::GetResultType() const
{
	return SPH_ATTR_BIGINT;
}

template<typename T>
void GrouperMVA_T<T>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	FetchMVAKeys<T> ( dKeys, tMatch, m_tLocator, GetBlobPool() );
}


class DistinctFetcher_c : public DistinctFetcher_i
{
public:
			explicit DistinctFetcher_c ( const CSphAttrLocator & tLocator ) : m_tLocator(tLocator) {}

	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override {}
	void	SetBlobPool ( const BYTE * pBlobPool ) override {}
	void	FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override { sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema ); }

protected:
	CSphAttrLocator m_tLocator;
};


class DistinctFetcherBlob_c : public DistinctFetcher_c
{
	using DistinctFetcher_c::DistinctFetcher_c;

public:
	void	SetBlobPool ( const BYTE * pBlobPool ) override { m_pBlobPool = pBlobPool; }

protected:
	const BYTE * m_pBlobPool = nullptr;
};


class DistinctFetcherInt_c : public DistinctFetcher_c
{
	using DistinctFetcher_c::DistinctFetcher_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherInt_c(m_tLocator); }
};


void DistinctFetcherInt_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(1);
	dKeys[0] = tMatch.GetAttr(m_tLocator);
}


class DistinctFetcherString_c : public DistinctFetcherBlob_c
{
	using DistinctFetcherBlob_c::DistinctFetcherBlob_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherString_c(m_tLocator); }
};


void DistinctFetcherString_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(1);
	auto dBlob = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
	dKeys[0] = (SphAttr_t) sphFNV64 ( dBlob );
}


class DistinctFetcherJsonField_c : public DistinctFetcherBlob_c
{
	using DistinctFetcherBlob_c::DistinctFetcherBlob_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherJsonField_c(m_tLocator); }
};


void DistinctFetcherJsonField_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);
	PushJsonField ( tMatch.GetAttr(m_tLocator), m_pBlobPool, [&dKeys]( SphAttr_t * pAttr, SphGroupKey_t uGroupKey )
		{
			if ( uGroupKey )
				dKeys.Add(uGroupKey);

			return true;
		} );
}


template<typename T>
class DistinctFetcherMva_T : public DistinctFetcherBlob_c
{
	using DistinctFetcherBlob_c::DistinctFetcherBlob_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherMva_T(m_tLocator); }
};

template<typename T>
void DistinctFetcherMva_T<T>::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);
	AddGroupedMVA<T> ( [&dKeys]( SphAttr_t tAttr ){ dKeys.Add(tAttr); }, tMatch.FetchAttrData ( m_tLocator, m_pBlobPool ) );
}


static DistinctFetcher_i * CreateDistinctFetcher ( const CSphString & sName, const CSphAttrLocator & tLocator, ESphAttr eType )
{
	// fixme! what about json?
	switch ( eType )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:		return new DistinctFetcherString_c(tLocator);
	case SPH_ATTR_JSON_FIELD:		return new DistinctFetcherJsonField_c(tLocator);
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_UINT32SET_PTR:	return new DistinctFetcherMva_T<DWORD>(tLocator);
	case SPH_ATTR_INT64SET:
	case SPH_ATTR_INT64SET_PTR:		return new DistinctFetcherMva_T<int64_t>(tLocator);
	default:						return new DistinctFetcherInt_c(tLocator);
	}
}

/////////////////////////////////////////////////////////////////////////////
/// (attrvalue,count) pair
struct SphUngroupedValue_t
{
	SphAttr_t attr;
	int count;
};

inline bool operator<( const SphUngroupedValue_t& lhs, const SphUngroupedValue_t& rhs )
{
	if ( lhs.attr != rhs.attr )
		return lhs.attr < rhs.attr;
	return lhs.count > rhs.count;
}

inline bool operator== ( const SphUngroupedValue_t& lhs, const SphUngroupedValue_t& rhs )
{
	return lhs.attr == rhs.attr;
}

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
					CSphUniqounter () { Reserve ( 16384 ); }
	void			Add ( const SphGroupedValue_t & tValue )	{ CSphVector<SphGroupedValue_t>::Add ( tValue ); m_bSorted = false; }
	void			Sort();

	int				CountStart ( SphGroupKey_t * pOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t * pOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( VecTraits_T<SphGroupKey_t> & dRemoveGroups );
	void			CopyTo ( CSphUniqounter & dRhs );

protected:
	int				m_iCountPos = 0;
	bool			m_bSorted = true;
};


void CSphUniqounter::Sort()
{
	if ( m_bSorted )
		return;

	CSphVector<SphGroupedValue_t>::Sort();
	m_bSorted = true;
}


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


void CSphUniqounter::CopyTo ( CSphUniqounter & dRhs )
{
	if ( m_bSorted && dRhs.m_bSorted )
	{
		CSphUniqounter dRes;
		dRes.MergeSorted ( dRhs, *this );
		dRes.m_bSorted = true;
		dRhs = std::move(dRes);
	}
	else
	{
		auto * pNewValues = dRhs.AddN ( GetLength() );
		memcpy ( pNewValues, Begin(), GetLengthBytes64() );
		dRhs.m_bSorted = false;
	}
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


/// additional group-by sorter settings
struct CSphGroupSorterSettings
{
	CSphAttrLocator		m_tLocGroupby;		///< locator for @groupby
	CSphAttrLocator		m_tLocCount;		///< locator for @count
	CSphAttrLocator		m_tLocDistinct;		///< locator for @distinct
	CSphAttrLocator		m_tLocGroupbyStr;	///< locator for @groupbystr

	bool				m_bDistinct = false;///< whether we need distinct
	CSphRefcountedPtr<CSphGrouper>		m_pGrouper;///< group key calculator
	CSphRefcountedPtr<DistinctFetcher_i> m_pDistinctFetcher;
	bool				m_bImplicit = false;///< for queries with aggregate functions but without group by clause
	SharedPtr_t<ISphFilter>	m_pAggrFilterTrait; ///< aggregate filter that got owned by grouper
	bool				m_bJson = false;	///< whether we're grouping by Json attribute
	int					m_iMaxMatches = 0;

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
	{
		sphFixupLocator ( m_tLocGroupby, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocCount, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocDistinct, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocGroupbyStr, pOldSchema, pNewSchema );

		if ( m_pDistinctFetcher )
			m_pDistinctFetcher->FixupLocators ( pOldSchema, pNewSchema );
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
		m_pSchema = (const CSphSchemaHelper *) pSchema; /// lazy hack
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
		for ( auto & tAttrGrp : m_dAttrsGrp )
			tDst.SetAttr ( tAttrGrp, sphGetRowAttr ( m_dRowBuf.Begin(), tAttrGrp ) );

		// restore back group_concat attribute(s)
		for ( auto & tAttrPtr : m_dAttrsPtr )
			tDst.SetAttr ( tAttrPtr, sphGetRowAttr ( m_dRowBuf.Begin (), tAttrPtr ) );
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


class BaseGroupSorter_c : public BlobPool_c, protected CSphGroupSorterSettings
{
	using BASE = CSphGroupSorterSettings;

public:
	FWD_BASECTOR( BaseGroupSorter_c )

	~BaseGroupSorter_c() override { ResetAggregates(); }

protected:
	MatchCloner_t			m_tPregroup;
	CSphVector<AggrFunc_i *>	m_dAggregates;

	void SetColumnar ( columnar::Columnar_i * pColumnar )
	{
		for ( auto i : m_dAggregates )
			i->SetColumnar(pColumnar);
	}

	/// schema, aggregates setup
	template <bool DISTINCT>
	inline void SetupBaseGrouper ( ISphSchema * pSchema, CSphVector<AggrFunc_i *> * pAvgs = nullptr )
	{
		m_tPregroup.ResetAttrs();
		ResetAggregates();

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
			case SPH_AGGR_SUM:	m_dAggregates.Add ( CreateAggrSum(tAttr) );	break;
			case SPH_AGGR_AVG:
				m_dAggregates.Add ( CreateAggrAvg ( tAttr, m_tLocCount ) );

				// store avg to calculate these attributes prior to groups sort
				if ( pAvgs )
					pAvgs->Add ( m_dAggregates.Last() );
				break;

			case SPH_AGGR_MIN:	m_dAggregates.Add ( CreateAggrMin(tAttr) );	break;
			case SPH_AGGR_MAX:	m_dAggregates.Add ( CreateAggrMax(tAttr) );	break;
			case SPH_AGGR_CAT:
				m_dAggregates.Add ( CreateAggrConcat(tAttr) );
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

	void AggrUpdate ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge = false )
	{
		for ( auto * pAggregate : this->m_dAggregates )
			pAggregate->Update ( tDst, tSrc, bGrouped, bMerge );
	}

	void AggrSetup ( CSphMatch & tDst, const CSphMatch & tSrc, bool bMerge = false )
	{
		for ( auto * pAggregate : this->m_dAggregates )
			pAggregate->Setup ( tDst, tSrc, bMerge );
	}

	void AggrUngroup ( CSphMatch & tMatch )
	{
		for ( auto * pAggregate : this->m_dAggregates )
			pAggregate->Ungroup ( tMatch );
	}

private:
	void ResetAggregates()
	{
		for ( auto & pAggregate : m_dAggregates )
			SafeDelete ( pAggregate );

		m_dAggregates.Resize(0);
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
		, m_tState ( dBase.GetState() )
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
class KBufferGroupSorter_T : public CSphMatchQueueTraits, protected BaseGroupSorter_c
{
	using MYTYPE = KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>;
	using BASE = CSphMatchQueueTraits;

public:
	KBufferGroupSorter_T ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: CSphMatchQueueTraits ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
		, BaseGroupSorter_c ( tSettings )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_iLimit ( tSettings.m_iMaxMatches )
		, m_tGroupSorter (*this)
		, m_tSubSorter ( *this, pComp )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( !DISTINCT || tSettings.m_pDistinctFetcher );
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

		m_pGrouper = tSettings.m_pGrouper;
		m_pDistinctFetcher = tSettings.m_pDistinctFetcher;
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final
	{
		if ( m_pSchema )
		{
			FixupLocators ( m_pSchema, pSchema );
			m_tGroupSorter.FixupLocators ( m_pSchema, pSchema, bRemapCmp );
			m_tPregroup.ResetAttrs ();
			m_dAggregates.Apply ( [] ( AggrFunc_i * pAggr ) { SafeDelete ( pAggr ); } );
			m_dAggregates.Resize ( 0 );
			m_dAvgs.Resize ( 0 );
		}

		BASE::SetSchema ( pSchema, bRemapCmp );
		SetupBaseGrouper<DISTINCT> ( pSchema, &m_dAvgs );
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
		if ( m_pDistinctFetcher )
			m_pDistinctFetcher->SetBlobPool(pBlobPool);
	}

	void SetColumnar ( columnar::Columnar_i * pColumnar ) final
	{
		CSphMatchQueueTraits::SetColumnar(pColumnar);
		BaseGroupSorter_c::SetColumnar(pColumnar);
		m_pGrouper->SetColumnar(pColumnar);
		if ( m_pDistinctFetcher )
			m_pDistinctFetcher->SetColumnar(pColumnar);
	}

	/// get entries count
	int GetLength () override
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
		if_const ( DISTINCT )
		{
			const CSphColumnInfo * pDistinct = m_pSchema->GetAttr("@distinct");
			assert(pDistinct);

			for ( const auto & tLocator : m_tGroupSorter.m_tLocator )
				if ( tLocator==pDistinct->m_tLocator )
				{
					m_bSortByDistinct = true;
					break;
				}
		}
	}

	bool CanBeCloned() const final { return !DISTINCT && BASE::CanBeCloned(); }

protected:
	ESphGroupBy 				m_eGroupBy;     ///< group-by function
	int							m_iLimit;		///< max matches to be retrieved
	CSphUniqounter				m_tUniq;
	bool						m_bSortByDistinct = false;
	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	SubGroupSorter_fn			m_tSubSorter;
	CSphVector<AggrFunc_i *>	m_dAvgs;
	bool						m_bAvgFinal = false;
	CSphVector<SphAttr_t>		m_dDistinctKeys;
	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

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

	inline void SetupBaseGrouperWrp ( ISphSchema * pSchema, CSphVector<AggrFunc_i *> * pAvgs )
	{
		SetupBaseGrouper<DISTINCT> ( pSchema, pAvgs );
	}

	void CloneKBufferGroupSorter ( MYTYPE* pClone ) const
	{
		// basic clone
		BASE::CloneTo ( pClone );

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
		pClone->SetupBaseGrouperWrp ( pClone->m_pSchema, &pClone->m_dAvgs );

		// m_pGrouper also need to be cloned (otherwise SetBlobPool will cause races)
		if ( m_pGrouper )
			pClone->m_pGrouper = m_pGrouper->Clone ();

		if ( m_pDistinctFetcher )
			pClone->m_pDistinctFetcher = m_pDistinctFetcher->Clone ();
	}

	template<typename SORTER> SORTER * CloneSorterT () const
	{
		CSphQuery dFoo;
		dFoo.m_iMaxMatches = m_iLimit;
		dFoo.m_eGroupFunc = m_eGroupBy;
		auto pClone = new SORTER ( m_tSubSorter.GetComparator (), &dFoo, *this );
		CloneKBufferGroupSorter ( pClone );
		return pClone;
	}

	CSphVector<AggrFunc_i *> GetAggregatesWithoutAvgs() const
	{
		CSphVector<AggrFunc_i *> dAggrs;
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
			m_dJustPopped.Add ( RowTagged_t ( m_dData[iMatch] ) );
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

		assert(m_pDistinctFetcher);
		m_pDistinctFetcher->GetKeys ( tEntry, this->m_dDistinctKeys );
		for ( auto i : this->m_dDistinctKeys )
			m_tUniq.Add ( {uGroupKey, i, iCount} );
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
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferGroupSorter : public KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
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
	using KBufferGroupSorter::GetAggregatesWithoutAvgs;
	using KBufferGroupSorter::Distinct;
	using KBufferGroupSorter::UpdateDistinct;
	using KBufferGroupSorter::RemoveDistinct;
	using KBufferGroupSorter::FreeMatchPtrs;
	using KBufferGroupSorter::m_bAvgFinal;

	using CSphGroupSorterSettings::m_tLocGroupby;
	using CSphGroupSorterSettings::m_tLocCount;
	using CSphGroupSorterSettings::m_tLocDistinct;

	using BaseGroupSorter_c::EvalHAVING;
	using BaseGroupSorter_c::AggrSetup;
	using BaseGroupSorter_c::AggrUpdate;
	using BaseGroupSorter_c::AggrUngroup;

	using CSphMatchQueueTraits::m_iSize;
	using CSphMatchQueueTraits::m_dData;
	using CSphMatchQueueTraits::Get;
	using CSphMatchQueueTraits::Add;
	using CSphMatchQueueTraits::Used;
	using CSphMatchQueueTraits::ResetAfterFlatten;

	using MatchSorter_c::m_iTotal;
	using MatchSorter_c::m_tJustPushed;
	using MatchSorter_c::m_dJustPopped;
	using MatchSorter_c::m_pSchema;

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: KBufferGroupSorter ( pComp, pQuery, tSettings )
		, m_hGroup2Match ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
	{}

	bool	Push ( const CSphMatch & tEntry ) override						{ return PushEx<false> ( tEntry, m_pGrouper->KeyFromMatch(tEntry), false ); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) override	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) override			{ return PushEx<true> ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), false ); }
	ISphMatchSorter * Clone() const override								{ return this->template CloneSorterT<MYTYPE>(); }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) override
	{
		FinalizeMatches();

		auto dAggrs = GetAggregatesWithoutAvgs();
		const CSphMatch * pBegin = pTo;

		for ( auto iMatch : this->m_dIData )
		{
			CSphMatch & tMatch = m_dData[iMatch];
			if_const ( HAS_AGGREGATES )
				dAggrs.Apply ( [&tMatch] ( AggrFunc_i * pAggr ) { pAggr->Finalize ( tMatch ); } );

			if ( !EvalHAVING ( tMatch ))
			{
				FreeMatchPtrs ( iMatch, false );
				continue;
			}

			Swap ( *pTo, tMatch );
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


	void MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final
	{
		if ( !Used () )
			return;

		auto& dRhs = *(MYTYPE *) pRhs;
		if ( dRhs.IsEmpty () )
		{
			CSphMatchQueueTraits::SwapMatchQueueTraits ( dRhs );
			m_hGroup2Match.Swap ( dRhs.m_hGroup2Match );
			dRhs.m_bMatchesFinalized = m_bMatchesFinalized;
			dRhs.m_iMaxUsed = m_iMaxUsed;
			if ( !m_bMatchesFinalized && bCopyMeta )
				dRhs.m_tUniq = m_tUniq;

			m_iMaxUsed = -1;
			return;
		}

		bool bUniqUpdated = false;
		if ( !m_bMatchesFinalized && bCopyMeta )
		{
			m_tUniq.CopyTo ( dRhs.m_tUniq );
			bUniqUpdated = true;
		}

		// if we're copying meta (uniq counters), we don't need distinct calculation right now
		// we can do it later after all sorters are merged
		FinalizeMatches ( !bCopyMeta );

		// matches in dRhs are using a new (standalone) schema
		// however, some supposedly unused matches still have old schema
		// they were not cleared immediately for performance reasons
		// we need to do that now
		for ( int i = dRhs.m_dIData.GetLength(); i < dRhs.m_dData.GetLength(); i++ )
		{
			int iId = *(dRhs.m_dIData.Begin()+i);
			dRhs.m_dData[iId].ResetDynamic();
		}

		dRhs.m_bUpdateDistinct = !bUniqUpdated;
		dRhs.SetMerge(true);

		// just push in heap order
		// since we have grouped matches, it is not always possible to move them,
		// so use plain push instead
		for ( auto iMatch : this->m_dIData )
			dRhs.PushGrouped ( m_dData[iMatch], false );

		dRhs.m_bUpdateDistinct = true;
		dRhs.SetMerge(false);

		// once we're done copying, cleanup
		for ( int i = 0; i<m_iMaxUsed; ++i )
			m_dData[i].ResetDynamic();

		m_iMaxUsed = -1;
	}

	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) override
	{
		if ( !Used() )
			return;

		if ( bFinalizeMatches )
			FinalizeMatches();
		else if_const ( DISTINCT )
		{
			// if we are not finalizing matches, we are using global sorters
			// let's try to remove dupes while we are processing data in separate threads
			// so that the main thread will have fewer data to work with
			m_tUniq.Sort();
			VecTraits_T<SphGroupKey_t> dStub;
			m_tUniq.Compact(dStub);
		}

		// just evaluate in heap order
		for ( auto iMatch : this->m_dIData )
			tProcessor.Process ( &m_dData[iMatch] );
	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	bool PushIntoExistingGroup( CSphMatch & tGroup, const CSphMatch & tEntry, SphGroupKey_t uGroupKey, bool bGrouped, SphAttr_t * pAttr )
	{
		assert ( tGroup.GetAttr ( m_tLocGroupby )==uGroupKey );
		assert ( tGroup.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

		auto & tLocCount = m_tLocCount;
		if ( bGrouped )
			tGroup.AddCounterAttr ( tLocCount, tEntry );
		else
			tGroup.AddCounterScalar ( tLocCount, 1 );

		if_const ( HAS_AGGREGATES )
			AggrUpdate ( tGroup, tEntry, bGrouped, m_bMerge );

		// if new entry is more relevant, update from it
		if ( m_tSubSorter.MatchIsGreater ( tEntry, tGroup ) )
		{
			if_const ( NOTIFICATIONS )
			{
				m_tJustPushed = RowTagged_t ( tEntry );
				this->m_dJustPopped.Add ( RowTagged_t ( tGroup ) );
			}

			// clone the low part of the match
			this->m_tPregroup.CloneKeepingAggrs ( tGroup, tEntry );
			if ( pAttr )
				UpdateGroupbyStr ( tGroup, pAttr );
		}

		// submit actual distinct value
		if ( DISTINCT && m_bUpdateDistinct )
			UpdateDistinct ( tEntry, uGroupKey, bGrouped );

		return false; // since it is a dupe
	}

	/// add entry to the queue
	template <bool GROUPED>
	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool, SphAttr_t * pAttr=nullptr )
	{
		if_const ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			this->m_dJustPopped.Resize ( 0 );
		}
		auto & tLocCount = m_tLocCount;

		m_bMatchesFinalized = false;
		if_const ( HAS_AGGREGATES && m_bAvgFinal )
			CalcAvg ( Avg_e::UNGROUP );

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match.Find ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tLocGroupby )==uGroupKey );
			return PushIntoExistingGroup ( *pMatch, tEntry, uGroupKey, GROUPED, pAttr );
		}

		// submit actual distinct value
		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, uGroupKey, GROUPED );

		// if we're full, let's cut off some worst groups
		if ( Used()==m_iSize )
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );

		// do add
		assert ( Used()<m_iSize );
		CSphMatch & tNew = Add();
		m_pSchema->CloneMatch ( tNew, tEntry );

		if_const ( HAS_AGGREGATES )
			AggrSetup ( tNew, tEntry, m_bMerge );

		if_const ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( tNew );

		if_const ( GROUPED )
		{
			if_const ( HAS_AGGREGATES )
				AggrUngroup(tNew);
		}
		else
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( tLocCount, 1 );
			if ( DISTINCT && m_bUpdateDistinct )
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
	bool	m_bUpdateDistinct = true;
	bool	m_bMerge = false;

	void CalcAvg ( Avg_e eGroup )
	{
		if ( m_dAvgs.IsEmpty() )
			return;

		m_bAvgFinal = ( eGroup==Avg_e::FINALIZE );

		if ( eGroup==Avg_e::FINALIZE )
			for ( auto i : this->m_dIData )
				m_dAvgs.Apply( [this,i] ( AggrFunc_i * pAvg ) { pAvg->Finalize ( m_dData[i] ); } );
		else
			for ( auto i : this->m_dIData )
				m_dAvgs.Apply ( [this,i] ( AggrFunc_i * pAvg ) { pAvg->Ungroup ( m_dData[i] ); } );
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
	void FinalizeMatches ( bool bCountDistinct=true )
	{
		if ( m_bMatchesFinalized )
			return;

		m_bMatchesFinalized = true;

		if ( Used() > m_iLimit )
			CutWorst ( m_iLimit, true );
		else
		{
			if ( DISTINCT && bCountDistinct )
				CountDistinct();

			CalcAvg ( Avg_e::FINALIZE );
			SortGroups();
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
			if_const ( DISTINCT && !m_bSortByDistinct ) // since they haven't counted at the top
			{
				RebuildHash(); // distinct uses m_hGroup2Match
				CountDistinct();
			}
		} else {
			// we've called CalcAvg ( Avg_e::FINALIZE ) before partitioning groups
			// now we can undo this calculation for the rest apart from thrown away
			// on finalize (sorting) cut we don't need to ungroup here
			CalcAvg ( Avg_e::UNGROUP );
			RebuildHash();
		}
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
		float COEFF = Max ( 1.0f, float(Used()) / iBound );
		int iPivot = this->m_dIData[ int(iBound/COEFF) ];

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
				b = j;  // too many elems acquired; continue with left part
			else
				a = i;  // too less elems acquired; continue with right part

			int iPivotIndex = int ( ( a * ( COEFF-1 )+b ) / COEFF );
			iPivot = this->m_dIData[iPivotIndex];
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
 * To work with them, we have vector of indexes, so that each index points to corresponding match in the backend.
 * So when performing moving operations (sort, etc.) we actually change indexes and never move matches themselves.
 *
 * Say, when user pushes matches with weights of 5,2,3,1,4,6, and we then sort them, we will have the following relations:
 *
 * m5 m2 m3 m1 m4 m6 // backend, placed in natural order as they come here
 *  1  2  3  4  5  6 // original indexes, just points directly to backend matches.
 *
 * After, say, sort by asc matches weights, only index vector modified and became this:
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
 * On insert, we store old head into new elem, and new elem into the place of old head.
 * One thing rest here is indirect ref by position. I.e. we assume that index at position 6 points to match at position 6.
 * However, we can notice, that since it is ring, left elem of 6-th points to it directly by number 6.
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

template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferNGroupSorter : public KBufferGroupSorter_T<COMPGROUP,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferNGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS,HAS_AGGREGATES>;

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
	using KBufferGroupSorter::GetAggregatesWithoutAvgs;
	using KBufferGroupSorter::Distinct;
	using KBufferGroupSorter::FreeMatchPtrs;
	using KBufferGroupSorter::UpdateDistinct;
	using KBufferGroupSorter::RemoveDistinct;
	using KBufferGroupSorter::m_bAvgFinal;

	using CSphGroupSorterSettings::m_tLocGroupby;
	using CSphGroupSorterSettings::m_tLocCount;
	using CSphGroupSorterSettings::m_tLocDistinct;
//	using CSphGroupSorterSettings::m_tLocGroupbyStr; // check! unimplemented?

	using BaseGroupSorter_c::EvalHAVING;
	using BaseGroupSorter_c::AggrUpdate;
	using BaseGroupSorter_c::AggrUngroup;

	using CSphMatchQueueTraits::m_iSize;
	using CSphMatchQueueTraits::m_dData;

	using MatchSorter_c::m_iTotal;
	using MatchSorter_c::m_tJustPushed;
	using MatchSorter_c::m_pSchema;

public:
	/// ctor
	CSphKBufferNGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: KBufferGroupSorter ( pComp, pQuery, tSettings )
		, m_hGroup2Index ( tSettings.m_iMaxMatches*GROUPBY_FACTOR )
		, m_iGLimit ( Min ( pQuery->m_iGroupbyLimit, m_iLimit ) )
	{
#ifndef NDEBUG
		DBG << "Created iruns = " << m_iruns << " ipushed = " << m_ipushed;
#endif
		this->m_dIData.Resize ( m_iSize ); // m_iLimit * GROUPBY_FACTOR
	}

	inline void SetGLimit ( int iGLimit )	{ m_iGLimit = Min ( iGLimit, m_iLimit ); }
	int GetLength() override				{ return Min ( m_iUsed, m_iLimit );	}

	bool	Push ( const CSphMatch & tEntry ) override						{ return PushEx<false> ( tEntry, m_pGrouper->KeyFromMatch(tEntry), false ); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override	{ return PushEx<true> ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), bNewSet ); }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) override
	{
		if ( !GetLength() )
			return 0;

		if ( !m_bFinalized )
		{
			FinalizeChains ();
			PrepareForExport ();
			CountDistinct ();
		}

		auto fnSwap = [&pTo] ( CSphMatch & tSrc ) 	{ // the writer
			Swap ( *pTo, tSrc );
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

	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) override
	{
		if ( !GetLength() )
			return;

		if ( bFinalizeMatches )
		{
			if ( !m_bFinalized )
			{
				FinalizeChains();
				PrepareForExport();
				CountDistinct();
			}

			ProcessData ( tProcessor, m_dFinalizedHeads );
		}
		else
		{
			ProcessData ( tProcessor, GetAllHeads() );

			if_const ( DISTINCT )
			{
				// if we are not finalizing matches, we are using global sorters
				// let's try to remove dupes while we are processing data in separate threads
				// so that the main thread will have fewer data to work with
				m_tUniq.Sort();
				VecTraits_T<SphGroupKey_t> dStub;
				m_tUniq.Compact(dStub);
			}
		}
	}

	// TODO! TEST!
	ISphMatchSorter * Clone () const override
	{
		auto* pClone = this->template CloneSorterT<MYTYPE>();
		pClone->SetGLimit (m_iGLimit);
		return pClone;
	}

	void MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final
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
			if ( !m_bFinalized && bCopyMeta )
				dRhs.m_tUniq = m_tUniq;

			return;
		}

		bool bUniqUpdated = false;
		if ( !m_bFinalized && bCopyMeta )
		{
			m_tUniq.CopyTo ( dRhs.m_tUniq );
			bUniqUpdated = true;
		}

		if ( !m_bFinalized )
		{
			FinalizeChains();
//			PrepareForExport(); // for moving we not need fine-finaled matches; just cleaned is enough
			CountDistinct();
		}

		dRhs.m_bUpdateDistinct = !bUniqUpdated;
		dRhs.SetMerge(true);

		auto iTotal = dRhs.m_iTotal;
		for ( auto iHead : m_dFinalizedHeads )
		{
			auto uGroupKey = m_dData[iHead].GetAttr ( m_tLocGroupby );

			// have to set bNewSet to true
			// as need to fallthrough at PushAlreadyHashed and update count and aggregates values for head match
			// even uGroupKey match already exists
			dRhs.template PushEx<true> ( m_dData[iHead], uGroupKey, true, true );
			for ( int i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				dRhs.template PushEx<false> ( m_dData[i], uGroupKey, false, true );

			DeleteChain ( iHead, false );
		}

		dRhs.m_bUpdateDistinct = true;
		dRhs.SetMerge(false);

		dRhs.m_iTotal = m_iTotal+iTotal;
	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	int m_iStorageSolidFrom = 0; // edge from witch storage is not yet touched and need no chaining freelist
	OpenHash_T<int, SphGroupKey_t> m_hGroup2Index; // used to quickly locate group for incoming match

	int				m_iGLimit;		///< limit per one group
	SphGroupKey_t	m_uLastGroupKey = -1;	///< helps to determine in pushEx whether the new subgroup started
	int				m_iFree = 0;			///< current insertion point
	int				m_iUsed = 0;

	// final cached data valid when everything is finalized
	bool			m_bFinalized = false;	// helper to avoid double work
	CSphVector<int> m_dFinalizedHeads;	/// < sorted finalized heads
	int				m_iLastGroupCutoff;	/// < cutoff edge of last group to fit limit

#ifndef NDEBUG
	int				m_iruns = 0;		///< helpers for conditional breakpoints on debug
	int				m_ipushed = 0;
#endif
	LOC_ADD;

	/*
	 * Every match according to uGroupKey came to own subset.
	 * Head match of each group stored in the hash to quickly locate on next pushes
	 * It hold all calculated stuff from aggregates/group_concat until finalization.
	 */
	template <bool GROUPED>
	bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bNewSet, bool bTailFinalized=false )
	{

#ifndef NDEBUG
		++m_ipushed;
		DBG << "PushEx: tag" << tEntry.m_iTag << ",g" << uGroupKey << ": pushed" << m_ipushed
			<< " g" << GROUPED << " n" << bNewSet;
#endif

		if_const ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			this->m_dJustPopped.Resize ( 0 );
		}

		this->m_bFinalized = false;
		if_const ( HAS_AGGREGATES && m_bAvgFinal )
			CalcAvg ( Avg_e::UNGROUP );

		// place elem into the set
		auto iNew = AllocateMatch ();
		CSphMatch & tNew = m_dData[iNew];

		// if such group already hashed
		int * pGroupIdx = m_hGroup2Index.Find ( uGroupKey );
		if ( pGroupIdx )
			return PushAlreadyHashed ( pGroupIdx, iNew, tEntry, uGroupKey, GROUPED, bNewSet, bTailFinalized );

		// match came from MoveTo of another sorter, it is tail, and it has no group here (m.b. it is already
		// deleted during finalization as one of worst). Just discard the whole group in the case.
		if ( bTailFinalized && !GROUPED )
		{
			DeallocateMatch ( iNew );
			return false;
		}

		m_pSchema->CloneMatch ( tNew, tEntry ); // fixme! check if essential data cloned
		//	else
		//			this->m_tPregroup.CloneWithoutAggrs ( tNew, tEntry );
		//			this->m_tPregroup.CopyAggrs ( tNew, tEntry );


		// submit actual distinct value in all cases
		if ( DISTINCT && m_bUpdateDistinct )
			UpdateDistinct ( tNew, uGroupKey, GROUPED );

		if_const ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( tNew );

		this->m_dIData[iNew] = iNew; // new head - points to self (0-ring)
		Verify ( m_hGroup2Index.Add ( uGroupKey, iNew ));
		++m_iTotal;

		if_const ( GROUPED )
		{
			m_uLastGroupKey = uGroupKey;

			if_const ( HAS_AGGREGATES )
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
	bool	m_bUpdateDistinct = true;
	bool	m_bMerge = false;

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
	int ChainLen ( int iPos ) const
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
			m_tJustPushed = RowTagged_t ( tNew );

		// put after the head
		auto iPrevChain = this->m_dIData[iHead];
		this->m_dIData[iNew] = iPrevChain;
		this->m_dIData[iHead] = iNew;
	}

	// add entry to existing group
	/*
	 * If group is not full, and new match is less than head, it will replace the head.
	 * calculated stuff will be moved and adopted by this new replacement.
	 * If group is full, and new match is less than head, it will be early rejected.
	 * In all other cases new match will be inserted into the group right after head
	 */
	bool PushAlreadyHashed ( int * pHead, int iNew, const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped, bool bNewSet, bool bTailFinalized )
	{
		int & iHead = *pHead;

		assert ( m_dData[iHead].GetAttr ( m_tLocGroupby )==uGroupKey );
		assert ( m_dData[iHead].m_pDynamic[-1]==tEntry.m_pDynamic[-1] );
		DBG << "existing " << m_dData[iHead].m_iTag << "," << uGroupKey
			<< " m_pDynamic: " << m_dData[iHead].m_pDynamic;

		// check if we need to push the match at all
		if ( m_tSubSorter.MatchIsGreater ( tEntry, m_dData[iHead] ) )
			AddToChain ( iNew, tEntry, iHead ); // always add; bad will be filtered later in gc
		else if ( ChainLen ( iHead )>=m_iGLimit ) // less than worst, drop it
			DeallocateMatch ( iNew );
		else
		{
			AddToChain ( iNew, tEntry, iHead );
			this->m_tPregroup.MoveAggrs ( m_dData[iNew], m_dData[iHead] );
			*pHead = iNew;
		}

		auto & tHeadMatch = m_dData[iHead];
		// submit actual distinct value in all cases
		if ( DISTINCT && m_bUpdateDistinct )
			UpdateDistinct ( tEntry, uGroupKey, bGrouped );

		// update group-wide counters
		auto & tLocCount = m_tLocCount;
		if ( bGrouped )
		{
			// it's already grouped match
			// sum grouped matches count
			if ( bNewSet || uGroupKey!=m_uLastGroupKey )
			{
				tHeadMatch.AddCounterAttr ( tLocCount, tEntry );
				m_uLastGroupKey = uGroupKey;
				bNewSet = true;
			}
		} else if ( !bTailFinalized )
		{
			// it's a simple match
			// increase grouped matches count
			tHeadMatch.AddCounterScalar ( tLocCount, 1 );
			bNewSet = true;
		}

		// update aggregates
		if_const ( HAS_AGGREGATES )
		{
			if ( bNewSet )
				AggrUpdate ( tHeadMatch, tEntry, bGrouped, m_bMerge );
		}

		// since it is dupe (i.e. such group is already pushed) - return false;
		return false;
	}

	enum class Avg_e { FINALIZE, UNGROUP };
	void CalcAvg ( Avg_e eGroup )
	{
		if ( this->m_dAvgs.IsEmpty() )
			return;

		m_bAvgFinal = ( eGroup==Avg_e::FINALIZE );

		int64_t i = 0;
		if ( eGroup==Avg_e::FINALIZE )
			for ( auto tData = m_hGroup2Index.Iterate ( &i ); tData.second; tData = m_hGroup2Index.Iterate ( &i ))
				m_dAvgs.Apply ( [this, &tData] ( AggrFunc_i * pAvg ) {
					pAvg->Finalize ( m_dData[*tData.second] );
				});
		else
			for ( auto tData = m_hGroup2Index.Iterate ( &i ); tData.second; tData = m_hGroup2Index.Iterate ( &i ))
				m_dAvgs.Apply ( [this, &tData] ( AggrFunc_i * pAvg ) {
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
				b = j;  // too many elems acquired; continue with left part
			else
				a = i;  // too few elems acquired; continue with right part
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
		// otherwise in d-tr FreDataPtr on non-zero dynamics will be called again with probably another schema and crash
		// FIXME!!! need to keep and restore all members changed by TryAllocateMatch - it'd be better to rewrite code to pass state into TryAllocateMatch or use common code
		auto iFree = m_iFree;
		auto iUsed = m_iUsed;
		auto iSSFrom = m_iStorageSolidFrom;
		for ( auto iElem = TryAllocateMatch (); iElem>=0; iElem = TryAllocateMatch () )
			m_dData[iElem].ResetDynamic ();
		m_iFree = iFree;
		m_iUsed = iUsed;
		m_iStorageSolidFrom = iSSFrom;

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
		for ( auto& iHead : m_dFinalizedHeads )
		{
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
	// if last group is not fit into rest of iLimit, it still kept whole, no fraction performed over it.
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
			else
			{
				 // all quota exceeded, the rest just to be cut totally
				auto iRemoved = DeleteChain ( m_dFinalizedHeads[i], eStage==Stage_e::COLLECT );
				if_const ( DISTINCT )
					dRemovedHeads.Add( iRemoved );
				m_dFinalizedHeads.RemoveFast ( i-- );
			}

		// discard removed distinct
		if_const ( DISTINCT )
			RemoveDistinct ( dRemovedHeads );

		if ( eStage==Stage_e::COLLECT )
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
		if ( !dWorstTail.IsEmpty() )
		{
			BinaryPartitionTail ( dChain, iLimit );
			dChain.Resize ( iLimit );
		}

		// sort if necessary and ensure last elem of chain is the worst one
		if ( eStage==Stage_e::FINAL )
		{
			dChain.Sort( m_tSubSorter ); // sorted in reverse order, so the worst match here is the last one.
			iLimit = dChain.GetLength();
		} else
		{
			assert ( dChain.GetLength ()==iLimit );
			// not sorted, need to find worst match for new head
			int iWorst = 0;
			for (int i=1; i<iLimit; ++i)
			{
				if ( m_tSubSorter.IsLess ( dChain[iWorst], dChain[i] ) )
					iWorst = i;
			}
			::Swap ( dChain[iWorst], dChain[iLimit-1] );
		}

		auto iNewHead = dChain.Last ();

		// move calculated aggregates to the new head
		if ( iNewHead!=*pHead )
		{
			SphGroupKey_t uGroupKey = m_dData[*pHead].GetAttr ( m_tLocGroupby );
			int * pHeadInHash = m_hGroup2Index.Find(uGroupKey);
			assert(pHeadInHash);

			this->m_tPregroup.MoveAggrs ( m_dData[iNewHead], m_dData[*pHead] );
			*pHead = iNewHead;
			*pHeadInHash = iNewHead;
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
		for ( auto i = iNext; i!=iPos; i = iNext )
		{
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

	void ProcessData ( MatchProcessor_i & tProcessor, const IntVec_t & dHeads )
	{
		for ( auto iHead : dHeads )
		{
			tProcessor.Process ( &m_dData[iHead] ); // process top group match
			for ( int i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				tProcessor.Process ( &m_dData[i] ); // process tail matches
		}
	}
};

/////////////////////////////////////////////////////////////////////
/// generic match sorter that understands groupers that return multiple keys per match
template < typename T >
class MultiValueGroupSorterTraits_T : public T
{
	using BASE = T;

public:
	MultiValueGroupSorterTraits_T ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: T ( pComp, pQuery, tSettings )
	{}

	bool Push ( const CSphMatch & tMatch ) override
	{
		this->m_pGrouper->MultipleKeysFromMatch ( tMatch, m_dKeys );

		bool bRes = false;
		for ( auto i : m_dKeys )
			bRes |= BASE::template PushEx<false> ( tMatch, i, false );

		return bRes;
	}

	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		return BASE::template PushEx<true> ( tEntry, tEntry.GetAttr ( BASE::m_tLocGroupby ), bNewSet );
	}

private:
	CSphVector<SphGroupKey_t>	m_dKeys;
};

template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class MultiValueGroupSorter_T : public MultiValueGroupSorterTraits_T <CSphKBufferGroupSorter <COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>
{
	using BASE = MultiValueGroupSorterTraits_T <CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>;
	using MYTYPE = MultiValueGroupSorter_T < COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES >;

public:
	using BASE::BASE;

	ISphMatchSorter * Clone () const final { return this->template CloneSorterT<MYTYPE>(); }
};

template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class MultiValueNGroupSorter_T : public MultiValueGroupSorterTraits_T < CSphKBufferNGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>
{
	using BASE = MultiValueGroupSorterTraits_T <CSphKBufferNGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>;
	using MYTYPE = MultiValueNGroupSorter_T <COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;

public:
	using BASE::BASE;

	ISphMatchSorter * Clone () const final
	{
		auto * pClone = this->template CloneSorterT<MYTYPE>();
		pClone->SetGLimit (this->m_iGLimit);
		return pClone;
	}
};

/////////////////////////////////////////////////////////////////////

/// match sorter with k-buffering and group-by for JSON arrays
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferJsonGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES >
{
public:
	using BASE = CSphKBufferGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
	using MYTYPE = CSphKBufferJsonGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;

	// since we inherit from template, we need to write boring 'using' block
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, DISTINCT, NOTIFICATIONS>;
	using KBufferGroupSorter::m_eGroupBy;
	using KBufferGroupSorter::m_iLimit;
	using KBufferGroupSorter::m_tSubSorter;

	/// ctor
	FWD_BASECTOR( CSphKBufferJsonGroupSorter )

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushMatch(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }

	/// add pre-grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		// re-group it based on the group key
		return BASE::template PushEx<true> ( tEntry, tEntry.GetAttr ( BASE::m_tLocGroupby ), bNewSet );
	}

	ISphMatchSorter * Clone () const final
	{
		return this->template CloneSorterT<MYTYPE>();
	}

private:
	FORCE_INLINE bool PushMatch ( const CSphMatch & tMatch )
	{
		SphGroupKey_t uGroupKey = this->m_pGrouper->KeyFromMatch ( tMatch );

		auto iValue = (int64_t)uGroupKey;
		CSphGrouper * pGrouper = this->m_pGrouper;
		const BYTE * pBlobPool = ((CSphGrouperJsonField*)pGrouper)->GetBlobPool();

		return PushJsonField ( iValue, pBlobPool, [this, &tMatch]( SphAttr_t * pAttr, SphGroupKey_t uMatchGroupKey )
			{
				return BASE::template PushEx<false> ( tMatch, uMatchGroupKey, false, pAttr );
			}
		);
	}
};


/// implicit group-by sorter
/// invoked when no 'group-by', but count(*) or count(distinct attr) are in game
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES>
class CSphImplicitGroupSorter final : public MatchSorter_c, ISphNoncopyable, protected BaseGroupSorter_c
{
	using MYTYPE = CSphImplicitGroupSorter<COMPGROUP, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
	using BASE = MatchSorter_c;

public:
	CSphImplicitGroupSorter ( const ISphMatchComparator * DEBUGARG(pComp), const CSphQuery *, const CSphGroupSorterSettings & tSettings )
		: BaseGroupSorter_c ( tSettings )
	{
		assert ( !DISTINCT || tSettings.m_pDistinctFetcher );
		assert ( !pComp );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);

		if_const ( DISTINCT )
			m_dUniq.Reserve ( 16384 );
		m_iMatchCapacity = 1;

		m_pDistinctFetcher = tSettings.m_pDistinctFetcher;
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) final
	{
		if ( m_pSchema )
		{
			FixupLocators ( m_pSchema, pSchema );
			m_tPregroup.ResetAttrs ();
			m_dAggregates.Apply ( [] ( AggrFunc_i * pAggr ) {SafeDelete ( pAggr ); } );
			m_dAggregates.Resize ( 0 );
		}

		BASE::SetSchema ( pSchema, bRemapCmp );
		SetupBaseGrouper<DISTINCT> ( pSchema );
	}

	bool	IsGroupby () const final { return true; }
	void	SetBlobPool ( const BYTE * pBlobPool ) final
	{
		BlobPool_c::SetBlobPool ( pBlobPool );
		if ( m_pDistinctFetcher )
			m_pDistinctFetcher->SetBlobPool(pBlobPool);
	}

	void SetColumnar ( columnar::Columnar_i * pColumnar ) final
	{
		BASE::SetColumnar(pColumnar);
		BaseGroupSorter_c::SetColumnar(pColumnar);
		if ( m_pDistinctFetcher )
			m_pDistinctFetcher->SetColumnar(pColumnar);
	}

	bool	IsCutoffDisabled() const final { return true; }
	bool	Push ( const CSphMatch & tEntry ) final							{ return PushEx<false>(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final		{ return PushEx<true>(tEntry); }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) final
	{
		assert ( m_bDataInitialized );

		CountDistinct ();

		if_const ( HAS_AGGREGATES )
		{
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Finalize ( m_tData );
		}

		int iCopied = 0;
		if ( EvalHAVING ( m_tData ) )
		{
			iCopied = 1;
			Swap ( *pTo, m_tData );
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
	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) final
	{
		if ( !GetLength() )
			return;

		tProcessor.Process ( &m_tData );

		if ( !bFinalizeMatches )
			m_dUniq.Uniq();
	}

	int		GetLength() final			{ return m_bDataInitialized ? 1 : 0; }
	bool	CanBeCloned() const final	{ return !DISTINCT && BASE::CanBeCloned(); }

	// TODO! test.
	ISphMatchSorter * Clone () const final
	{
		auto pClone = new MYTYPE ( nullptr, nullptr, *this );
		CloneTo ( pClone );
		pClone->SetupBaseGrouperWrp ( pClone->m_pSchema );
		if ( m_pDistinctFetcher )
			pClone->m_pDistinctFetcher = m_pDistinctFetcher->Clone();
		return pClone;
	}

	// FIXME! test CSphImplicitGroupSorter
	void MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final
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

		if ( bCopyMeta )
		{
			auto * pNewData = dRhs.m_dUniq.AddN ( m_dUniq.GetLength() );
			memcpy ( pNewData, m_dUniq.Begin(), m_dUniq.GetLengthBytes() );
		}

		// other step is a bit tricky:
		// we just can't add current count uniq to final; need to append m_dUniq instead,
		// so that final flattening will calculate real uniq count.
		dRhs.AddCount ( m_tData );

		if_const ( HAS_AGGREGATES )
			dRhs.UpdateAggregates ( m_tData, false, true );

		if ( !bCopyMeta && DISTINCT )
			dRhs.UpdateDistinct ( m_tData );
	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	CSphMatch		m_tData;
	bool			m_bDataInitialized = false;
	bool			m_bMerge = false;

	CSphVector<SphUngroupedValue_t>	m_dUniq;

private:
	CSphVector<SphAttr_t> m_dDistinctKeys;
	CSphRefcountedPtr<DistinctFetcher_i> m_pDistinctFetcher;

	inline void SetupBaseGrouperWrp ( ISphSchema * pSchema )	{ SetupBaseGrouper<DISTINCT> ( pSchema ); }
	void	AddCount ( const CSphMatch & tEntry )				{ m_tData.AddCounterAttr ( m_tLocCount, tEntry ); }
	void	UpdateAggregates ( const CSphMatch & tEntry, bool bGrouped = true, bool bMerge = false ) { AggrUpdate ( m_tData, tEntry, bGrouped, bMerge ); }
	void	SetupAggregates ( const CSphMatch & tEntry )		{ AggrSetup ( m_tData, tEntry, m_bMerge ); }

	// submit actual distinct value in all cases
	void UpdateDistinct ( const CSphMatch & tEntry, bool bGrouped = true )
	{
		int iCount = 1;
		if ( bGrouped )
			iCount = (int) tEntry.GetAttr ( m_tLocDistinct );

		assert(m_pDistinctFetcher);
		m_pDistinctFetcher->GetKeys ( tEntry, m_dDistinctKeys );
		for ( auto i : m_dDistinctKeys )
			this->m_dUniq.Add ( { i, iCount } );
	}

	/// add entry to the queue
	template <bool GROUPED>
	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry )
	{
		if_const ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			m_dJustPopped.Resize(0);
		}

		if ( m_bDataInitialized )
		{
			assert ( m_tData.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if_const ( GROUPED )
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
			if_const ( HAS_AGGREGATES )
				UpdateAggregates ( tEntry, GROUPED, m_bMerge );
		}

		if_const ( DISTINCT )
			UpdateDistinct ( tEntry, GROUPED );

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( m_bDataInitialized )
			return false;

		// add first
		m_pSchema->CloneMatch ( m_tData, tEntry );

		// first-time aggregate setup
		if_const ( HAS_AGGREGATES )
			SetupAggregates(tEntry);

		if_const ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( m_tData );

		if_const ( !GROUPED )
		{
			m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
			m_tData.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				m_tData.SetAttr ( m_tLocDistinct, 0 );
		}
		else
		{
			if_const ( HAS_AGGREGATES )
				AggrUngroup ( m_tData );
		}

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
			int iCount = m_dUniq[0].count;

			for ( int i = 1, iLen = m_dUniq.GetLength (); i<iLen; ++i )
			{
				if ( m_dUniq[i-1]==m_dUniq[i] )
					continue;
				iCount += m_dUniq[i].count;
			}

			m_tData.SetAttr ( m_tLocDistinct, iCount );
		}
	}
};


class FastBaseSorter_c : public MatchSorter_c, ISphNoncopyable, protected BaseGroupSorter_c
{
public:
			FastBaseSorter_c ( const CSphGroupSorterSettings & tSettings ) : BaseGroupSorter_c ( tSettings ) {}

	bool	IsGroupby () const final										{ return true; }
	bool	CanBeCloned() const final										{ return false; }
	void	SetMerge ( bool bMerge ) final									{}
	void	Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) final	{ if ( GetLength() ) tProcessor.Process ( &m_tData ); }
	int		GetLength() final												{ return m_bDataInitialized ? 1 : 0; }
	ISphMatchSorter * Clone() const final									{ return nullptr; }
	void	MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) final			{ assert ( 0 && "Not supported"); }
	bool	IsPrecalc() const final											{ return true; }

	int Flatten ( CSphMatch * pTo ) final
	{
		assert ( m_bDataInitialized );
		Swap ( *pTo, m_tData );
		m_iTotal = 0;
		m_bDataInitialized = false;
		return 1;
	}

protected:
	CSphMatch	m_tData;
	bool		m_bDataInitialized = false;
};


// fast count distinct sorter
// works by using precalculated count distinct taken from secondary indexes
class FastCountDistinctSorter_c final : public FastBaseSorter_c
{
public:
	FastCountDistinctSorter_c ( int iCountDistinct, const CSphGroupSorterSettings & tSettings )
		: FastBaseSorter_c ( tSettings )
		, m_iCountDistinct ( iCountDistinct )
	{}

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushEx(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final			{ return PushEx(tEntry); }

private:
	int			m_iCountDistinct = 0;

	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry )
	{
		if ( m_bDataInitialized )
			return false;

		m_pSchema->CloneMatch ( m_tData, tEntry );
		m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
		m_tData.SetAttr ( m_tLocCount, 1 );
		m_tData.SetAttr ( m_tLocDistinct, m_iCountDistinct );

		m_bDataInitialized = true;
		m_iTotal++;
		return true;
	}
};

// fast count sorter
// works by using precalculated count taken from secondary indexes
class FastCountSorter_c final : public FastBaseSorter_c
{
public:
	FastCountSorter_c ( int iCount, const CSphGroupSorterSettings & tSettings )
		: FastBaseSorter_c ( tSettings )
		, m_iCount ( iCount )
	{}

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushEx(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final			{ return PushEx(tEntry); }

private:
	int			m_iCount = 0;

	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry )
	{
		if ( m_bDataInitialized )
			return false;

		m_pSchema->CloneMatch ( m_tData, tEntry );
		m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
		m_tData.SetAttr ( m_tLocCount, 1 );
		m_tData.SetAttr ( m_tLocCount, m_iCount );

		m_bDataInitialized = true;
		m_iTotal++;
		return true;
	}
};


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


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_FLOAT:
			return SPH_KEYPART_FLOAT;

		case SPH_ATTR_DOUBLE:
			return SPH_KEYPART_DOUBLE;

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

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING INSTANTIATION
//////////////////////////////////////////////////////////////////////////

struct Precalculated_t
{
	int64_t		m_iCountDistinct = -1;
	int64_t		m_iCount = -1;
};

#define CREATE_SORTER_4TH(SORTER,COMPGROUP,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS,HAS_AGGREGATES) \
{ \
	BYTE uSelector = 4*(tSettings.m_bDistinct?1:0) + 2*(bHasPackedFactors?1:0) + (HAS_AGGREGATES?1:0); \
	switch ( uSelector ) \
	{ \
	case 0:	return new SORTER<COMPGROUP,false,false,false> ( pComp, pQuery, tSettings ); \
	case 1:	return new SORTER<COMPGROUP,false,false,true>  ( pComp, pQuery, tSettings ); \
	case 2:	return new SORTER<COMPGROUP,false,true, false> ( pComp, pQuery, tSettings ); \
	case 3:	return new SORTER<COMPGROUP,false,true, true>  ( pComp, pQuery, tSettings ); \
	case 4:	return new SORTER<COMPGROUP,true, false,false> ( pComp, pQuery, tSettings ); \
	case 5:	return new SORTER<COMPGROUP,true, false,true>  ( pComp, pQuery, tSettings ); \
	case 6:	return new SORTER<COMPGROUP,true, true, false> ( pComp, pQuery, tSettings ); \
	case 7:	return new SORTER<COMPGROUP,true, true, true>  ( pComp, pQuery, tSettings ); \
	default: assert(0); return nullptr; \
	} \
}

template < typename COMPGROUP >
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const Precalculated_t & tPrecalc )
{
	if ( tPrecalc.m_iCountDistinct!=-1 )
		return new FastCountDistinctSorter_c ( tPrecalc.m_iCountDistinct, tSettings );

	if ( tPrecalc.m_iCount!=-1 )
		return new FastCountSorter_c ( tPrecalc.m_iCount, tSettings );

	BYTE uSelector3rd = 8*( tSettings.m_bJson ? 1:0 ) + 4*( pQuery->m_iGroupbyLimit>1 ? 1:0 ) + 2*( tSettings.m_bImplicit ? 1:0 ) + ( ( tSettings.m_pGrouper && tSettings.m_pGrouper->IsMultiValue() ) ? 1:0 );
	switch ( uSelector3rd )
	{
	case 0:	CREATE_SORTER_4TH ( CSphKBufferGroupSorter,		COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 1: CREATE_SORTER_4TH ( MultiValueGroupSorter_T,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 2: CREATE_SORTER_4TH ( CSphImplicitGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 4:	CREATE_SORTER_4TH ( CSphKBufferNGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 5: CREATE_SORTER_4TH ( MultiValueNGroupSorter_T,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 8: CREATE_SORTER_4TH ( CSphKBufferJsonGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	default: assert(0); return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const Precalculated_t & tPrecalc )
{
	switch ( eGroupFunc )
	{
		case FUNC_GENERIC1:		return sphCreateSorter3rd<MatchGeneric1_fn>	( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn>	( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn>	( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn>	( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn>	( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>		( pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
		default:				return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const Precalculated_t & tPrecalc )
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

	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
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
	ISphExpr *			Clone() const final;

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

	if ( eCmd==SPH_EXPR_UPDATE_DEPENDENT_COLS )
	{
		int iRef = *static_cast<int*>(pArg);
		if ( m_iLat>=iRef )	m_iLat--;
		if ( m_iLon>=iRef )	m_iLon--;
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


ISphExpr * ExprGeodist_t::Clone() const
{
	auto * pClone = new ExprGeodist_t;
	pClone->m_tGeoLatLoc = m_tGeoLatLoc;
	pClone->m_tGeoLongLoc = m_tGeoLongLoc;
	pClone->m_fGeoAnchorLat = m_fGeoAnchorLat;
	pClone->m_fGeoAnchorLong = m_fGeoAnchorLong;
	pClone->m_iLat = m_iLat;
	pClone->m_iLon = m_iLon;

	return pClone;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );
static CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation );
static CSphGrouper * CreateGrouperStringExpr ( ISphExpr * pExpr, ESphCollation eCollation );


bool HasImplicitGrouping ( const CSphQuery & tQuery )
{
	auto fnIsImplicit = [] ( const CSphQueryItem & t )
	{
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	return tQuery.m_sGroupBy.IsEmpty() ? tQuery.m_dItems.any_of(fnIsImplicit) : false;
}

class QueueCreator_c
{
public:
	bool				m_bMulti = false;
	bool				m_bCreate = true;
	bool				m_bZonespanlist = false;
	DWORD				m_uPackedFactorFlags = SPH_FACTOR_DISABLE;

						QueueCreator_c ( const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, CSphString & sError, StrVec_t * pExtra, QueryProfile_c * pProfile );

	bool				SetupComputeQueue();
	bool				SetupGroupQueue();
	bool				SetupQueue();

	CSphRsetSchema &	SorterSchema() const { return *m_pSorterSchema; }
	bool				HasJson() const { return m_tGroupSorterSettings.m_bJson; }
	bool				SetSchemaGroupQueue ( const CSphRsetSchema & tNewSchema );

	/// creates proper queue for given query
	/// may return NULL on error; in this case, error message is placed in sError
	/// if the pUpdate is given, creates the updater's queue and perform the index update
	/// instead of searching
	ISphMatchSorter *	CreateQueue();

private:
	const SphQueueSettings_t &	m_tSettings;
	const CSphQuery &			m_tQuery;
	CSphString &				m_sError;
	StrVec_t *					m_pExtra = nullptr;
	QueryProfile_c *			m_pProfile = nullptr;

	bool						m_bHasCount = false;
	bool						m_bHasGroupByExpr = false;
	sph::StringSet				m_hQueryAttrs;
	std::unique_ptr<CSphRsetSchema> m_pSorterSchema;

	bool						m_bGotGroupby;
	bool						m_bRandomize;
	ESphSortFunc				m_eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc				m_eGroupFunc = FUNC_REL_DESC;
	CSphMatchComparatorState	m_tStateMatch;
	CSphVector<ExtraSortExpr_t> m_dMatchJsonExprs;
	CSphMatchComparatorState	m_tStateGroup;
	CSphVector<ExtraSortExpr_t> m_dGroupJsonExprs;
	CSphGroupSorterSettings		m_tGroupSorterSettings;
	CSphVector<std::pair<int,bool>> m_dGroupColumns;
	bool						m_bHeadWOGroup;
	bool						m_bGotDistinct;
	bool						m_bExprsNeedDocids = false;

	// for sorter to create pooled attributes
	bool						m_bHaveStar = false;

	// fixme! transform to StringSet on end of merge!
	sph::StringSet				m_hQueryColumns; // FIXME!!! unify with Extra schema after merge master into branch
	sph::StringSet				m_hQueryDups;
	sph::StringSet				m_hExtra;

	bool	ParseQueryItem ( const CSphQueryItem & tItem );
	bool	MaybeAddGeodistColumn();
	bool	MaybeAddExprColumn();
	bool	MaybeAddExpressionsFromSelectList();
	bool	AddExpressionsForUpdates();
	bool	MaybeAddGroupbyMagic ( bool bGotDistinct );
	bool	CheckHavingConstraints() const;
	bool	SetupGroupbySettings ( bool bHasImplicitGrouping );
	void	AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount );

	void	ReplaceGroupbyStrWithExprs ( CSphMatchComparatorState & tState, int iNumOldAttrs );
	void	ReplaceStaticStringsWithExprs ( CSphMatchComparatorState & tState );
	void	ReplaceJsonWithExprs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	void	AddColumnarExprsAsAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	void	RemapAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	static void	SetupRemapColJson ( CSphColumnInfo & tRemapCol, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, int iStateAttr ) ;
	const CSphColumnInfo * GetGroupbyStr ( int iAttr, int iNumOldAttrs ) const;

	bool	SetupMatchesSortingFunc();
	bool	SetupGroupSortingFunc ( bool bGotDistinct );
	bool	AddGroupbyStuff();
	bool	SetGroupSorting();
	void	ExtraAddSortkeys ( const int * dAttrs );
	bool	AddStoredFieldExpressions();
	bool	AddColumnarAttributeExpressions();
	void	CreateGrouperByAttr ( ESphAttr eType, const CSphColumnInfo & tGroupByAttr, bool & bGrouperUsesAttrs );
	void	SelectStageForColumnarExpr ( CSphColumnInfo & tExprCol );
	void	FetchDependencyChains ( IntVec_t & dDependentCols );
	void	PropagateEvalStage ( CSphColumnInfo & tExprCol, IntVec_t & dDependentCols );
	bool	SetupDistinctAttr();
	bool	PredictAggregates() const;
	bool	ReplaceWithColumnarItem ( const CSphString & sAttr, ESphEvalStage eStage );
	int		ReduceMaxMatches() const;
	int		AdjustMaxMatches ( int iMaxMatches ) const;
	bool	ConvertColumnarToDocstore();
	const CSphColumnInfo * GetAliasedColumnarAttr ( const CSphColumnInfo & tAttr );
	bool	SetupAggregateExpr ( CSphColumnInfo & tExprCol, const CSphString & sExpr, DWORD uQueryPackedFactorFlags );
	bool	SetupColumnarAggregates ( CSphColumnInfo & tExprCol );
	void	UpdateAggregateDependencies ( CSphColumnInfo & tExprCol );
	int		GetGroupbyAttrIndex() const			{ return GetAliasedAttrIndex ( m_tQuery.m_sGroupBy, m_tQuery, *m_pSorterSchema ); }
	int		GetGroupDistinctAttrIndex() const	{ return GetAliasedAttrIndex ( m_tQuery.m_sGroupDistinct, m_tQuery, *m_pSorterSchema ); }

	bool	CanCalcFastCountDistinct() const;
	bool	CanCalcFastCount() const;
	Precalculated_t FetchPrecalculatedValues() const;

	ISphMatchSorter *	SpawnQueue();
	std::unique_ptr<ISphFilter>	CreateAggrFilter() const;
	void				SetupCollation();
	bool				Err ( const char * sFmt, ... ) const;
};


QueueCreator_c::QueueCreator_c ( const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, CSphString & sError, StrVec_t * pExtra, QueryProfile_c * pProfile )
	: m_tSettings ( tSettings )
	, m_tQuery ( tQuery )
	, m_sError ( sError )
	, m_pExtra ( pExtra )
	, m_pProfile ( pProfile )
	, m_pSorterSchema { std::make_unique<CSphRsetSchema>() }
{
	// short-cuts
	m_sError = "";
	*m_pSorterSchema = m_tSettings.m_tSchema;

	m_dMatchJsonExprs.Resize ( CSphMatchComparatorState::MAX_ATTRS );
	m_dGroupJsonExprs.Resize ( CSphMatchComparatorState::MAX_ATTRS );
}


const CSphColumnInfo * QueueCreator_c::GetAliasedColumnarAttr ( const CSphColumnInfo & tAttr )
{
	if ( !tAttr.IsColumnarExpr() )
		return &tAttr;

	CSphString sAliasedCol;
	tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sAliasedCol );
	const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAliasedCol.cstr() );
	assert(pAttr);
	return pAttr;
}


void QueueCreator_c::CreateGrouperByAttr ( ESphAttr eType, const CSphColumnInfo & tGroupByAttr, bool & bGrouperUsesAttrs )
{
	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;
	const CSphAttrLocator & tLoc = tGroupByAttr.m_tLocator;

	switch ( eType )
	{
	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_FIELD:
		{
			ExprParseArgs_t tExprArgs;
			tExprArgs.m_eCollation = m_tQuery.m_eCollation;

			ISphExprRefPtr_c pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(), tSchema, m_sError, tExprArgs ) };
			m_tGroupSorterSettings.m_pGrouper = new CSphGrouperJsonField ( tLoc, pExpr );
			m_tGroupSorterSettings.m_bJson = true;
		}
		break;

	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:
		// percolate select list push matches with string_ptr

		// check if it is a columnar attr or an expression spawned instead of a columnar attr
		// even if it is an expression, spawn a new one, because a specialized grouper works a lot faster because it doesn't allocate and store string in the match
		if ( tGroupByAttr.IsColumnar() || tGroupByAttr.IsColumnarExpr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarString ( *GetAliasedColumnarAttr(tGroupByAttr), m_tQuery.m_eCollation );
			bGrouperUsesAttrs = false;
		}
		else if ( tGroupByAttr.m_pExpr && !tGroupByAttr.m_pExpr->IsDataPtrAttr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperStringExpr ( tGroupByAttr.m_pExpr, m_tQuery.m_eCollation );
			bGrouperUsesAttrs = false;
		}
		else
			m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperString ( tLoc, m_tQuery.m_eCollation );
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		if ( tGroupByAttr.IsColumnar() || tGroupByAttr.IsColumnarExpr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarMVA ( *GetAliasedColumnarAttr(tGroupByAttr) );
			bGrouperUsesAttrs = false;
			break;
		}

		if ( eType==SPH_ATTR_UINT32SET )
			m_tGroupSorterSettings.m_pGrouper = new GrouperMVA_T<DWORD>(tLoc);
		else
			m_tGroupSorterSettings.m_pGrouper = new GrouperMVA_T<int64_t>(tLoc);
		break;

	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
		if ( tGroupByAttr.IsColumnar() || tGroupByAttr.IsColumnarExpr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarMVA ( *GetAliasedColumnarAttr(tGroupByAttr) );
			bGrouperUsesAttrs = false;
		}
		break;

	case SPH_ATTR_BOOL:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BIGINT:
		if ( tGroupByAttr.IsColumnar() || ( tGroupByAttr.IsColumnarExpr() && tGroupByAttr.m_eStage>SPH_EVAL_PREFILTER ) )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarInt ( *GetAliasedColumnarAttr(tGroupByAttr) );
			bGrouperUsesAttrs = false;
		}
		break;

	default:
		break;
	}

	if ( !m_tGroupSorterSettings.m_pGrouper )
		m_tGroupSorterSettings.m_pGrouper = new CSphGrouperAttr(tLoc);
}


bool QueueCreator_c::SetupDistinctAttr()
{
	if ( m_tQuery.m_sGroupDistinct.IsEmpty() )
		return true;

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;

	int iDistinct = tSchema.GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr () );
	if ( iDistinct<0 )
		return Err ( "group-count-distinct attribute '%s' not found", m_tQuery.m_sGroupDistinct.cstr() );

	const auto & tDistinctAttr = tSchema.GetAttr(iDistinct);
	if ( IsNotRealAttribute(tDistinctAttr) )
		return Err ( "group-count-distinct attribute '%s' not found", m_tQuery.m_sGroupDistinct.cstr() );

	if ( tDistinctAttr.IsColumnar() )
		m_tGroupSorterSettings.m_pDistinctFetcher = CreateColumnarDistinctFetcher ( tDistinctAttr.m_sName, tDistinctAttr.m_eAttrType, m_tQuery.m_eCollation );
	else
		m_tGroupSorterSettings.m_pDistinctFetcher = CreateDistinctFetcher ( tDistinctAttr.m_sName, tDistinctAttr.m_tLocator, tDistinctAttr.m_eAttrType );

	return true;
}


bool QueueCreator_c::SetupGroupbySettings ( bool bHasImplicitGrouping )
{
	if ( m_tQuery.m_sGroupBy.IsEmpty() && !bHasImplicitGrouping )
		return true;

	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
		return Err ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;

	m_tGroupSorterSettings.m_iMaxMatches = m_tSettings.m_iMaxMatches;

	if ( !SetupDistinctAttr() )
		return false;

	CSphString sJsonColumn;
	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_MULTIPLE )
	{
		CSphVector<CSphColumnInfo> dAttrs;
		VecRefPtrs_t<ISphExpr *> dJsonKeys;

		StrVec_t dGroupBy;
		sph::Split ( m_tQuery.m_sGroupBy.cstr (), -1, ",", [&] ( const char * sToken, int iLen )
		{
			CSphString sGroupBy ( sToken, iLen );
			sGroupBy.Trim ();
			dGroupBy.Add ( std::move ( sGroupBy ));
		} );
		dGroupBy.Uniq();

		for ( auto & sGroupBy : dGroupBy )
		{
			CSphString sJsonExpr;
			if ( sphJsonNameSplit ( sGroupBy.cstr(), &sJsonColumn ) )
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

			dAttrs.Add ( tAttr );
			m_dGroupColumns.Add ( { iAttr, true } );

			if ( !sJsonExpr.IsEmpty() )
			{
				ExprParseArgs_t tExprArgs;
				dJsonKeys.Add ( sphExprParse ( sJsonExpr.cstr(), tSchema, m_sError, tExprArgs ) );
			}
			else
				dJsonKeys.Add ( nullptr );
		}

		m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperMulti ( dAttrs, std::move(dJsonKeys), m_tQuery.m_eCollation );
		return true;
	}

	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr(), &sJsonColumn ) )
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

		m_dGroupColumns.Add ( { iAttr, true } );

		ExprParseArgs_t tExprArgs;
		tExprArgs.m_eCollation = m_tQuery.m_eCollation;

		ISphExprRefPtr_c pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(), tSchema, m_sError, tExprArgs ) };
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
	int iGroupBy = GetGroupbyAttrIndex();
	if ( iGroupBy<0 )
		return Err ( "group-by attribute '%s' not found", m_tQuery.m_sGroupBy.cstr() );

	const CSphColumnInfo & tGroupByAttr = tSchema.GetAttr(iGroupBy);
	ESphAttr eType = tGroupByAttr.m_eAttrType;
	CSphAttrLocator tLoc = tGroupByAttr.m_tLocator;
	bool bGrouperUsesAttrs = true;
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
			CreateGrouperByAttr ( eType, tGroupByAttr, bGrouperUsesAttrs );
			break;
		default:
			return Err ( "invalid group-by mode (mode=%d)", m_tQuery.m_eGroupFunc );
	}

	m_dGroupColumns.Add ( { iGroupBy, bGrouperUsesAttrs } );
	return true;
}

// move expressions used in ORDER BY or WITHIN GROUP ORDER BY to presort phase
void QueueCreator_c::AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount )
{
	if ( !iAttrCount )
		return;

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

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override
	{
		EXPR_CLASS_NAME_NOCHECK("ExprSortStringAttrFixup_c");
		uHash = sphFNV64 ( &m_tLocator, sizeof(m_tLocator), uHash );

		return CALC_DEP_HASHES();
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

			CSphFixedVector<BYTE> dBuf ( iTotalLen + 4 + iCount ); // data and tail GAP and space count
			BYTE * pDst = dBuf.Begin();

			// head element
			if ( iCount )
			{
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
				pVal += iElemLen;
			}

			// tail elements separated by space
			for ( int i=1; i<iCount; i++ )
			{
				*pDst++ = ' ';
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
				pVal += iElemLen;
			}

			int iStrLen = int ( pDst-dBuf.Begin() );
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

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override
	{
		EXPR_CLASS_NAME_NOCHECK("ExprSortJson2StringPtr_c");
		CALC_CHILD_HASH(m_pExpr);

		// uHash = sphFNV64 ( &m_tJsonCol, sizeof ( m_tJsonCol ), uHash );	//< that is wrong! Locator may have padding uninitialized data, valgrind will warn!
		uHash = sphCalcLocatorHash ( m_tJsonCol, uHash );					//< that is right, only meaningful fields processed without padding.

		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone() const final
	{
		return new ExprSortJson2StringPtr_c ( *this );
	}

private:
	CSphAttrLocator		m_tJsonCol;				///< JSON attribute to fix
	ISphExprRefPtr_c	m_pExpr;

private:
	ExprSortJson2StringPtr_c ( const ExprSortJson2StringPtr_c & rhs )
		: m_tJsonCol ( rhs.m_tJsonCol )
		, m_pExpr ( SafeClone (rhs.m_pExpr) )
	{}
};


const char * GetInternalAttrPrefix()
{
	return g_sIntAttrPrefix;
}

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


CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new CSphGrouperString<Utf8CIHash_fn>(tLoc);
	case SPH_COLLATION_LIBC_CI:			return new CSphGrouperString<LibcCIHash_fn>(tLoc);
	case SPH_COLLATION_LIBC_CS:			return new CSphGrouperString<LibcCSHash_fn>(tLoc);
	default:							return new CSphGrouperString<BinaryHash_fn>(tLoc);
	}
}


static CSphGrouper * CreateGrouperStringExpr ( ISphExpr * pExpr, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new GrouperStringExpr_T<Utf8CIHash_fn>(pExpr);
	case SPH_COLLATION_LIBC_CI:			return new GrouperStringExpr_T<LibcCIHash_fn>(pExpr);
	case SPH_COLLATION_LIBC_CS:			return new GrouperStringExpr_T<LibcCSHash_fn>(pExpr);
	default:							return new GrouperStringExpr_T<BinaryHash_fn>(pExpr);
	}
}


static CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation )
{
	bool bHaveColumnar = dAttrs.any_of ( []( auto & tAttr ){ return tAttr.IsColumnar() || tAttr.IsColumnarExpr(); } );
	bool bAllColumnar = dAttrs.all_of ( []( auto & tAttr ){ return tAttr.IsColumnar() || tAttr.IsColumnarExpr(); } );

	if ( bAllColumnar )
		return CreateGrouperColumnarMulti ( dAttrs, eCollation );

	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<Utf8CIHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<Utf8CIHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	case SPH_COLLATION_LIBC_CI:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<LibcCIHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<LibcCIHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	case SPH_COLLATION_LIBC_CS:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<LibcCSHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<LibcCSHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	default:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<BinaryHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<BinaryHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );
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
		case FUNC_REL_DESC:		return CreatePlainSorter<MatchRelevanceLt_fn>	( bKbuffer, iMaxMatches, bFactors );
		case FUNC_ATTR_DESC:	return CreatePlainSorter<MatchAttrLt_fn>		( bKbuffer, iMaxMatches, bFactors );
		case FUNC_ATTR_ASC:		return CreatePlainSorter<MatchAttrGt_fn>		( bKbuffer, iMaxMatches, bFactors );
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


void QueueCreator_c::SelectStageForColumnarExpr ( CSphColumnInfo & tExprCol )
{
	if ( !tExprCol.IsColumnarExpr() )
	{
		tExprCol.m_eStage = SPH_EVAL_PREFILTER;
		return;
	}

	// columnar expressions are a special case
	// it is sometimes faster to evaluate them in the filter than to evaluate the expression, store it in the match and then use it in the filter

	// FIXME: add sorters?
	int iRank = 0;
	iRank += tExprCol.m_sName==m_tQuery.m_sGroupBy ? 1 : 0;
	iRank += m_tQuery.m_dFilters.any_of ( [&tExprCol]( const CSphFilterSettings & tFilter ) { return tFilter.m_sAttrName==tExprCol.m_sName; } ) ? 1 : 0;

	if ( iRank>1 )
		tExprCol.m_eStage = SPH_EVAL_PREFILTER;
}


void QueueCreator_c::FetchDependencyChains ( IntVec_t & dDependentCols )
{
	ARRAY_FOREACH ( i, dDependentCols )
	{
		const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dDependentCols[i] );

		// handle chains of dependencies (e.g. SELECT 1+attr f1, f1-1 f2 ... WHERE f2>5)
		if ( tCol.m_pExpr )
			tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );
	}

	dDependentCols.Uniq();
}


void QueueCreator_c::PropagateEvalStage ( CSphColumnInfo & tExprCol, IntVec_t & dDependentCols )
{
	bool bWeight = false;
	for ( auto i : dDependentCols )
	{
		const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr(i);
		bWeight |= tCol.m_bWeight;
	}

	if ( bWeight )
	{
		tExprCol.m_eStage = SPH_EVAL_PRESORT;
		tExprCol.m_bWeight = true;
	}

	for ( auto i : dDependentCols )
	{
		auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr(i) );
		if ( tDep.m_eStage > tExprCol.m_eStage )
			tDep.m_eStage = tExprCol.m_eStage;
	}
}


bool QueueCreator_c::SetupAggregateExpr ( CSphColumnInfo & tExprCol, const CSphString & sExpr, DWORD uQueryPackedFactorFlags )
{
	switch ( tExprCol.m_eAggrFunc )
	{
	case SPH_AGGR_AVG:
		// force AVG() to be computed in doubles
		tExprCol.m_eAttrType = SPH_ATTR_DOUBLE;
		tExprCol.m_tLocator.m_iBitCount = 64;
		break;

	case SPH_AGGR_CAT:
		// force GROUP_CONCAT() to be computed as strings
		tExprCol.m_eAttrType = SPH_ATTR_STRINGPTR;
		tExprCol.m_tLocator.m_iBitCount = ROWITEMPTR_BITS;
		break;

	case SPH_AGGR_SUM:
		if ( tExprCol.m_eAttrType==SPH_ATTR_BOOL )
		{
			tExprCol.m_eAttrType = SPH_ATTR_INTEGER;
			tExprCol.m_tLocator.m_iBitCount = 32;
		} else if ( tExprCol.m_eAttrType==SPH_ATTR_INTEGER )
		{
			tExprCol.m_eAttrType = SPH_ATTR_BIGINT;
			tExprCol.m_tLocator.m_iBitCount = 64;
		}
		break;

	default:
		break;
	}

	// force explicit type conversion for JSON attributes
	if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE && tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		return Err ( "ambiguous attribute type '%s', use INTEGER(), BIGINT() or DOUBLE() conversion functions", sExpr.cstr() );

	if ( uQueryPackedFactorFlags & SPH_FACTOR_JSON_OUT )
		tExprCol.m_eAttrType = SPH_ATTR_FACTORS_JSON;

	return true;
}


bool QueueCreator_c::SetupColumnarAggregates ( CSphColumnInfo & tExprCol )
{
	CSphVector<int> dDependentCols;
	tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );
	FetchDependencyChains ( dDependentCols );

	if ( !dDependentCols.GetLength() )
		return tExprCol.IsColumnarExpr();

	if ( dDependentCols.GetLength()==1 )
	{
		const CSphColumnInfo & tColumnarAttr = m_pSorterSchema->GetAttr ( dDependentCols[0] );
		if ( tColumnarAttr.IsColumnarExpr() )
		{
			CSphString sColumnarCol;
			tColumnarAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarCol );

			// let aggregate expression know that it is working with that columnar attribute
			tExprCol.m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR_COL, &sColumnarCol );

			return true;
		}
	}

	return false;
}


void QueueCreator_c::UpdateAggregateDependencies ( CSphColumnInfo & tExprCol )
{
	/// update aggregate dependencies (e.g. SELECT 1+attr f1, min(f1), ...)
	CSphVector<int> dDependentCols;
	tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );
	FetchDependencyChains ( dDependentCols );
	ARRAY_FOREACH ( j, dDependentCols )
	{
		auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( dDependentCols[j] ) );
		if ( tDep.m_eStage>tExprCol.m_eStage )
			tDep.m_eStage = tExprCol.m_eStage;
	}
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
	bool bColumnar = iAttrIdx>=0 && m_tSettings.m_tSchema.GetAttr(iAttrIdx).IsColumnar();
	bool bPlainAttr = ( ( sExpr=="*" || ( iAttrIdx>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && !bColumnar ) ) &&
		( tItem.m_sAlias.IsEmpty() || tItem.m_sAlias==tItem.m_sExpr ) );
	if ( iAttrIdx>=0 )
	{
		ESphAttr eAttr = m_tSettings.m_tSchema.GetAttr ( iAttrIdx ).m_eAttrType;
		if ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR
			|| eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET )
		{
			if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE )
				return Err ( "can not aggregate non-scalar attribute '%s'",	tItem.m_sExpr.cstr() );

			if ( !bPlainAttr && !bColumnar && ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR ) )
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
		{
			if ( bColumnar )	// we might have several similar aliases for columnar attributes (and they are not plain attrs but expressions)
				return true;
			else
				return Err ( "alias '%s' must be unique (conflicts with another alias)", tItem.m_sAlias.cstr() );
		}
	}

	// a new and shiny expression, lets parse
	CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
	DWORD uQueryPackedFactorFlags = SPH_FACTOR_DISABLE;
	bool bHasZonespanlist = false;
	bool bExprsNeedDocids = false;

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
	tExprParseArgs.m_pNeedDocIds = &bExprsNeedDocids;

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
		tExprCol.m_pExpr = sphExprParse ( sExpr2.cstr(), *m_pSorterSchema, m_sError, tExprParseArgs );
	} else
	{
		tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), *m_pSorterSchema, m_sError, tExprParseArgs );
	}

	m_uPackedFactorFlags |= uQueryPackedFactorFlags;
	m_bZonespanlist |= bHasZonespanlist;
	m_bExprsNeedDocids |= bExprsNeedDocids;
	tExprCol.m_eAggrFunc = tItem.m_eAggrFunc;
	if ( !tExprCol.m_pExpr )
		return Err ( "parse error: %s", m_sError.cstr() );

	// remove original column
	if ( iSorterAttr>=0 )
		m_pSorterSchema->RemoveStaticAttr(iSorterAttr);

	if ( !SetupAggregateExpr ( tExprCol, tItem.m_sExpr, uQueryPackedFactorFlags ) )
		return false;

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

				// so we are about to add a filter condition,
				// but it might depend on some preceding columns (e.g. SELECT 1+attr f1 ... WHERE f1>5)
				// lets detect those and move them to prefilter \ presort phase too
				CSphVector<int> dDependentCols;
				tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );

				SelectStageForColumnarExpr(tExprCol);
				FetchDependencyChains ( dDependentCols );
				PropagateEvalStage ( tExprCol, dDependentCols );

				break;
			}

		// add it!
		// NOTE, "final" stage might need to be fixed up later
		// we'll do that when parsing sorting clause
		m_pSorterSchema->AddAttr ( tExprCol, true );
	} else // some aggregate
	{
		bool bColumnarAggregate = SetupColumnarAggregates(tExprCol);

		// columnar aggregates have their own code path; no need to calculate them in presort
		tExprCol.m_eStage = bColumnarAggregate ? SPH_EVAL_SORTER : SPH_EVAL_PRESORT;

		m_pSorterSchema->AddAttr ( tExprCol, true );
		m_hExtra.Add ( tExprCol.m_sName );

		if ( !bColumnarAggregate )
			UpdateAggregateDependencies ( tExprCol );
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


bool QueueCreator_c::ReplaceWithColumnarItem ( const CSphString & sAttr, ESphEvalStage eStage )
{
	const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
	if ( !pAttr->IsColumnar() )
		return true;

	m_hQueryDups.Delete(sAttr);

	CSphQueryItem tItem;
	tItem.m_sExpr = tItem.m_sAlias = sAttr;
	if ( !ParseQueryItem ( tItem ) )
		return false;

	// force stage
	const CSphColumnInfo * pNewAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
	const_cast<CSphColumnInfo *>(pNewAttr)->m_eStage = Min ( pNewAttr->m_eStage, eStage );

	return true;
}


// Test for @geodist and setup, if any
bool QueueCreator_c::MaybeAddGeodistColumn ()
{
	if ( !m_tQuery.m_bGeoAnchor || m_pSorterSchema->GetAttrIndex ( "@geodist" )>=0 )
		return true;

	// replace columnar lat/lon with expressions before adding geodist
	if ( !ReplaceWithColumnarItem ( m_tQuery.m_sGeoLatAttr, SPH_EVAL_PREFILTER ) ) return false;
	if ( !ReplaceWithColumnarItem ( m_tQuery.m_sGeoLongAttr, SPH_EVAL_PREFILTER ) ) return false;

	auto pExpr = new ExprGeodist_t();
	if ( !pExpr->Setup ( &m_tQuery, *m_pSorterSchema, m_sError ))
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

	return true;
}

// Test for @expr and setup, if any
bool QueueCreator_c::MaybeAddExprColumn ()
{
	if ( m_tQuery.m_eSort!=SPH_SORT_EXPR || m_pSorterSchema->GetAttrIndex ( "@expr" )>=0 )
		return true;

	CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility
	// (i.e. too lazy to fix those tests right now)
	bool bHasZonespanlist;
	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprArgs.m_pZonespanlist = &bHasZonespanlist;

	tCol.m_pExpr = sphExprParse ( m_tQuery.m_sSortBy.cstr (), *m_pSorterSchema, m_sError, tExprArgs );
	if ( !tCol.m_pExpr )
		return false;

	m_bZonespanlist |= bHasZonespanlist;
	tCol.m_eStage = SPH_EVAL_PRESORT;
	m_pSorterSchema->AddAttr ( tCol, true );
	m_hQueryAttrs.Add ( tCol.m_sName );

	return true;
}


bool QueueCreator_c::AddStoredFieldExpressions()
{
	for ( int i = 0; i<m_tSettings.m_tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = m_tSettings.m_tSchema.GetField(i);
		if ( !(tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED) )
			continue;

		CSphQueryItem tItem;
		tItem.m_sExpr = tItem.m_sAlias = tField.m_sName;
		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	return true;
}


bool QueueCreator_c::AddColumnarAttributeExpressions()
{
	for ( int i = 0; i<m_tSettings.m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSettings.m_tSchema.GetAttr(i);
		const CSphColumnInfo * pSorterAttr = m_pSorterSchema->GetAttr ( tAttr.m_sName.cstr() );

		if ( !tAttr.IsColumnar() || ( pSorterAttr && !pSorterAttr->IsColumnar() ) )
			continue;

		m_hQueryDups.Delete ( tAttr.m_sName );

		CSphQueryItem tItem;
		tItem.m_sExpr = tItem.m_sAlias = tAttr.m_sName;
		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	return true;
}


// Add computed items
bool QueueCreator_c::MaybeAddExpressionsFromSelectList ()
{
	// expressions from select items
	if ( !m_tSettings.m_bComputeItems )
		return true;

	if ( !m_tQuery.m_dItems.all_of ( [&] ( const CSphQueryItem & v ) { return ParseQueryItem ( v ); } ))
		return false;

	if ( m_bHaveStar )
	{
		if ( !AddColumnarAttributeExpressions() )
			return false;

		if ( !AddStoredFieldExpressions() )
			return false;
	}

	return true;
}


bool QueueCreator_c::AddExpressionsForUpdates()
{
	if ( !m_tSettings.m_pCollection )
		return true;

	const CSphColumnInfo * pOldDocId = m_pSorterSchema->GetAttr ( sphGetDocidName() );
	if ( !pOldDocId->IsColumnar() && !pOldDocId->IsColumnarExpr() )
		return true;

	if ( pOldDocId->IsColumnar() )
	{
		// add columnar id expressions to update queue. otherwise we won't be able to fetch docids which are needed to run updates/deletes
		CSphQueryItem tItem;
		tItem.m_sExpr = tItem.m_sAlias = sphGetDocidName();
		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	auto * pDocId = const_cast<CSphColumnInfo *> ( m_pSorterSchema->GetAttr ( sphGetDocidName() ) );
	assert(pDocId);
	pDocId->m_eStage = SPH_EVAL_PRESORT;	// update/delete queues don't have real Finalize(), so just evaluate it at presort stage

	return true;
}


bool QueueCreator_c::MaybeAddGroupbyMagic ( bool bGotDistinct )
{
	CSphString sJsonGroupBy;
	// now let's add @groupby etc. if needed
	if ( m_bGotGroupby && m_pSorterSchema->GetAttrIndex ( "@groupby" )<0 )
	{
		ESphAttr eGroupByResult = ( !m_tGroupSorterSettings.m_bImplicit )
				? m_tGroupSorterSettings.m_pGrouper->GetResultType ()
				: SPH_ATTR_INTEGER; // implicit do not have grouper

		// all FACET group by should be the widest possible type
		if ( m_tQuery.m_bFacet || m_tQuery.m_bFacetHead	|| m_bMulti )
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

		// add @groupbystr last in case we need to skip it on sending (like @int_attr_*)
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
	if ( iGroupby>=0 )
	{
		m_tGroupSorterSettings.m_bDistinct = bGotDistinct;
		m_tGroupSorterSettings.m_tLocGroupby = m_pSorterSchema->GetAttr ( iGroupby ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocGroupby.m_bDynamic, "@groupby must be dynamic" );

		int iCount = m_pSorterSchema->GetAttrIndex ( "@count" );
		LOC_CHECK ( iCount>=0, "missing @count" );

		m_tGroupSorterSettings.m_tLocCount = m_pSorterSchema->GetAttr ( iCount ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocCount.m_bDynamic, "@count must be dynamic" );

		int iDistinct = m_pSorterSchema->GetAttrIndex ( "@distinct" );
		if ( bGotDistinct )
		{
			LOC_CHECK ( iDistinct>=0, "missing @distinct" );
			m_tGroupSorterSettings.m_tLocDistinct = m_pSorterSchema->GetAttr ( iDistinct ).m_tLocator;
			LOC_CHECK ( m_tGroupSorterSettings.m_tLocDistinct.m_bDynamic, "@distinct must be dynamic" );
		}
		else
			LOC_CHECK ( iDistinct<=0, "unexpected @distinct" );

		int iGroupbyStr = m_pSorterSchema->GetAttrIndex ( sJsonGroupBy.cstr() );
		if ( iGroupbyStr>=0 )
			m_tGroupSorterSettings.m_tLocGroupbyStr = m_pSorterSchema->GetAttr ( iGroupbyStr ).m_tLocator;
	}

	if ( m_bHasCount )
		LOC_CHECK ( m_pSorterSchema->GetAttrIndex ( "@count" )>=0, "Count(*) or @count is queried, but not available in the schema" );

	#undef LOC_CHECK
	return true;
}

bool QueueCreator_c::CheckHavingConstraints () const
{
	if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty () )
	{
		if ( !m_bGotGroupby )
			return Err ( "can not use HAVING without GROUP BY" );

		// should be column named at group by, or it's alias or aggregate
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


void QueueCreator_c::SetupRemapColJson ( CSphColumnInfo & tRemapCol, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, int iStateAttr )
{
	bool bFunc = dExtraExprs[iStateAttr].m_tKey.m_uMask==0;

	tRemapCol.m_eStage = SPH_EVAL_PRESORT;
	if ( bFunc )
	{
		tRemapCol.m_pExpr = dExtraExprs[iStateAttr].m_pExpr;
		tRemapCol.m_eAttrType = dExtraExprs[iStateAttr].m_eType;
		tState.m_eKeypart[iStateAttr] = Attr2Keypart ( tRemapCol.m_eAttrType );
	}
	else
		tRemapCol.m_pExpr = new ExprSortJson2StringPtr_c ( tState.m_tLocator[iStateAttr], dExtraExprs[iStateAttr].m_pExpr );
}


const CSphColumnInfo * QueueCreator_c::GetGroupbyStr ( int iAttr, int iNumOldAttrs ) const
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	if ( m_tSettings.m_bComputeItems && iAttr>=0 && iAttr<iNumOldAttrs && tSorterSchema.GetAttr(iAttr).m_sName=="@groupby" && m_dGroupColumns.GetLength() )
	{
		// FIXME!!! add support of multi group by
		const CSphColumnInfo & tGroupCol = tSorterSchema.GetAttr ( m_dGroupColumns[0].first );
		if ( tGroupCol.m_eAttrType==SPH_ATTR_STRING || tGroupCol.m_eAttrType==SPH_ATTR_STRINGPTR )
			return &tGroupCol;
	}

	return nullptr;
}


void QueueCreator_c::ReplaceGroupbyStrWithExprs ( CSphMatchComparatorState & tState, int iNumOldAttrs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		const CSphColumnInfo * pGroupStrBase = GetGroupbyStr ( tState.m_dAttrs[i], iNumOldAttrs );
		if ( !pGroupStrBase )
			continue;

		assert ( tState.m_dAttrs[i]>=0 && tState.m_dAttrs[i]<iNumOldAttrs );

		int iRemap = -1;
		if ( pGroupStrBase->m_eAttrType==SPH_ATTR_STRINGPTR )
		{
			// grouping by (columnar) string; and the same string is used in sorting
			// correct the locator and change the evaluation stage to PRESORT
			iRemap = tSorterSchema.GetAttrIndex ( pGroupStrBase->m_sName.cstr() );
			assert ( iRemap>=0 );
			const CSphColumnInfo & tAttr = tSorterSchema.GetAttr(iRemap);
			const_cast<CSphColumnInfo &>(tAttr).m_eStage = SPH_EVAL_PRESORT;
		}
		else if ( !pGroupStrBase->IsColumnar() )
		{
			CSphString sRemapCol;
			sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, pGroupStrBase->m_sName.cstr() );
			iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );

			if ( iRemap==-1 )
			{
				CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
				tRemapCol.m_pExpr = new ExprSortStringAttrFixup_c ( pGroupStrBase->m_tLocator );
				tRemapCol.m_eStage = SPH_EVAL_PRESORT;

				iRemap = tSorterSchema.GetAttrsCount();
				tSorterSchema.AddAttr ( tRemapCol, true );
			}
		}

		if ( iRemap!=-1 )
		{
			tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
			tState.m_tLocator[i] = tSorterSchema.GetAttr(iRemap).m_tLocator;
			tState.m_dAttrs[i] = iRemap;
			tState.m_dRemapped.BitSet ( i );
		}
	}
}


void QueueCreator_c::ReplaceStaticStringsWithExprs ( CSphMatchComparatorState & tState )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		if ( tState.m_eKeypart[i]!=SPH_KEYPART_STRING )
			continue;

		int iRemap = -1;
		int iAttrId = tState.m_dAttrs[i];
		const CSphColumnInfo & tAttr = tSorterSchema.GetAttr(iAttrId);
		if ( tAttr.IsColumnar() )
		{
			CSphString sAttrName = tAttr.m_sName;
			tSorterSchema.RemoveStaticAttr(iAttrId);
			
			CSphColumnInfo tRemapCol ( sAttrName.cstr(), SPH_ATTR_STRINGPTR );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;
			tRemapCol.m_pExpr = CreateExpr_GetColumnarString ( sAttrName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
			tSorterSchema.AddAttr ( tRemapCol, true );

			iRemap = tSorterSchema.GetAttrIndex ( sAttrName.cstr() );
		}
		else
		{
			CSphString sRemapCol;
			sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, tSorterSchema.GetAttr(iAttrId).m_sName.cstr() );
			iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
			if ( iRemap==-1 )
			{
				CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
				tRemapCol.m_eStage = SPH_EVAL_PRESORT;
				tRemapCol.m_pExpr = new ExprSortStringAttrFixup_c ( tState.m_tLocator[i] );

				iRemap = tSorterSchema.GetAttrsCount();
				tSorterSchema.AddAttr ( tRemapCol, true );
			}
		}

		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::ReplaceJsonWithExprs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		if ( dExtraExprs[i].m_tKey.m_sKey.IsEmpty() )
			continue;

		CSphString sRemapCol;
		sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, dExtraExprs[i].m_tKey.m_sKey.cstr() );

		int iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
		if ( iRemap==-1 )
		{
			CSphString sRemapLowercase = sRemapCol;
			sRemapLowercase.ToLower();
			iRemap = tSorterSchema.GetAttrIndex ( sRemapLowercase.cstr() );
		}

		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
			SetupRemapColJson ( tRemapCol, tState, dExtraExprs, i );
			iRemap = tSorterSchema.GetAttrsCount();
			tSorterSchema.AddAttr ( tRemapCol, true );
		}

		tState.m_tLocator[i] = tSorterSchema.GetAttr(iRemap).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::AddColumnarExprsAsAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		ISphExpr * pExpr = dExtraExprs[i].m_pExpr;
		if ( !pExpr || !pExpr->IsColumnar() )
			continue;

		const CSphString & sAttrName = tSorterSchema.GetAttr ( tState.m_dAttrs[i] ).m_sName;
		CSphColumnInfo tRemapCol ( sAttrName.cstr(), dExtraExprs[i].m_eType );
		tRemapCol.m_eStage = SPH_EVAL_PRESORT;
		tRemapCol.m_pExpr = pExpr;
		tRemapCol.m_pExpr->AddRef();

		int iRemap = tSorterSchema.GetAttrsCount();
		tSorterSchema.AddAttr ( tRemapCol, true );

		// remove initial attribute from m_hExtra
		// that way it won't be evaluated twice when it is not in select list
		m_hExtra.Delete(sAttrName);

		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_eKeypart[i] = Attr2Keypart ( dExtraExprs[i].m_eType );
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::RemapAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	// we have extra attrs (expressions) that we created while parsing the sort clause
	// we couldn't add them to the schema at that stage,
	// but now we can. we create attributes, assign internal names and set their expressions

	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;
	int iNumOldAttrs = tSorterSchema.GetAttrsCount();

	ReplaceGroupbyStrWithExprs ( tState, iNumOldAttrs );
	ReplaceStaticStringsWithExprs ( tState );
	ReplaceJsonWithExprs ( tState, dExtraExprs );
	AddColumnarExprsAsAttrs ( tState, dExtraExprs );

	// need another sort keys add after setup remap
	if ( iNumOldAttrs!=tSorterSchema.GetAttrsCount() )
		ExtraAddSortkeys ( tState.m_dAttrs );
}

// matches sorting function
bool QueueCreator_c::SetupMatchesSortingFunc ()
{
	m_bRandomize = false;
	if ( m_tQuery.m_eSort==SPH_SORT_EXTENDED )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( m_tQuery, m_tQuery.m_sSortBy.cstr(), *m_pSorterSchema, m_eMatchFunc, m_tStateMatch, m_dMatchJsonExprs, m_tSettings.m_bComputeItems, m_sError );
		if ( eRes==SORT_CLAUSE_ERROR )
			return false;

		if ( eRes==SORT_CLAUSE_RANDOM )
			m_bRandomize = true;

		ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

		AssignOrderByToPresortStage ( m_tStateMatch.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );
		RemapAttrs ( m_tStateMatch, m_dMatchJsonExprs );
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
		RemapAttrs ( m_tStateMatch, m_dMatchJsonExprs );
	}

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
	ESortClauseParseResult eRes = sphParseSortClause ( m_tQuery, m_tQuery.m_sGroupSortBy.cstr(), *m_pSorterSchema, m_eGroupFunc,	m_tStateGroup, m_dGroupJsonExprs, m_tSettings.m_bComputeItems, m_sError );

	if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
	{
		if ( eRes==SORT_CLAUSE_RANDOM )
			m_sError = "groups can not be sorted by @random";

		return false;
	}

	ExtraAddSortkeys ( m_tStateGroup.m_dAttrs );

	if ( !m_tGroupSorterSettings.m_bImplicit )
	{
		for ( const auto & tGroupColumn : m_dGroupColumns )
			m_hExtra.Add ( m_pSorterSchema->GetAttr ( tGroupColumn.first ).m_sName );
	}

	if ( bGotDistinct )
	{
		m_dGroupColumns.Add ( { m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr() ), true } );
		assert ( m_dGroupColumns.Last().first>=0 );
		m_hExtra.Add ( m_pSorterSchema->GetAttr ( m_dGroupColumns.Last().first ).m_sName );
	}

	// implicit case
	CSphVector<int> dGroupByCols;
	for ( const auto & i : m_dGroupColumns )
		if ( i.second )
			dGroupByCols.Add ( i.first );

	AssignOrderByToPresortStage ( dGroupByCols.Begin(), dGroupByCols.GetLength() );
	AssignOrderByToPresortStage ( m_tStateGroup.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );

	// GroupSortBy str attributes setup
	RemapAttrs ( m_tStateGroup, m_dGroupJsonExprs );
	return true;
}

// set up aggregate filter for grouper
std::unique_ptr<ISphFilter> QueueCreator_c::CreateAggrFilter () const
{
	assert ( m_bGotGroupby );
	if ( m_pSorterSchema->GetAttr ( m_tSettings.m_pAggrFilter->m_sAttrName.cstr() ) )
		return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, m_tSettings.m_pAggrFilter->m_sAttrName,
				*m_pSorterSchema, m_sError );

	// having might reference aliased attributes but @* attributes got stored without alias in sorter schema
	CSphString sHaving;
	for ( const auto & tItem : m_tQuery.m_dItems )
		if ( tItem.m_sAlias==m_tSettings.m_pAggrFilter->m_sAttrName )
		{
			sHaving = tItem.m_sExpr;
			break;
		}

	if ( sHaving=="groupby()" )
		sHaving = "@groupby";
	else if ( sHaving=="count(*)" )
		sHaving = "@count";

	return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, sHaving, *m_pSorterSchema, m_sError );
}

void QueueCreator_c::SetupCollation()
{
	SphStringCmp_fn fnCmp = GetStringCmpFunc ( m_tQuery.m_eCollation );
	m_tStateMatch.m_fnStrCmp = fnCmp;
	m_tStateGroup.m_fnStrCmp = fnCmp;
}

bool QueueCreator_c::AddGroupbyStuff ()
{
	// need schema with group related columns however not need grouper
	m_bHeadWOGroup = ( m_tQuery.m_sGroupBy.IsEmpty () && m_tQuery.m_bFacetHead );
	auto fnIsImplicit = [] ( const CSphQueryItem & t )
	{
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	bool bHasImplicitGrouping = HasImplicitGrouping(m_tQuery);

	// count(*) and distinct wo group by at main query should keep implicit flag
	if ( bHasImplicitGrouping && m_bHeadWOGroup )
		m_bHeadWOGroup = !m_tQuery.m_dRefItems.any_of ( fnIsImplicit );

	if ( !SetupGroupbySettings(bHasImplicitGrouping) )
		return false;

	// or else, check in SetupGroupbySettings() would already fail
	m_bGotGroupby = !m_tQuery.m_sGroupBy.IsEmpty () || m_tGroupSorterSettings.m_bImplicit;
	m_bGotDistinct = !!m_tGroupSorterSettings.m_pDistinctFetcher;

	if ( m_bHasGroupByExpr && !m_bGotGroupby )
		return Err ( "GROUPBY() is allowed only in GROUP BY queries" );

	// check for HAVING constrains
	if ( !CheckHavingConstraints() )
		return false;

	// now let's add @groupby stuff, if necessary
	return MaybeAddGroupbyMagic(m_bGotDistinct);
}

bool QueueCreator_c::SetGroupSorting()
{
	if ( m_bGotGroupby )
	{
		if ( !SetupGroupSortingFunc ( m_bGotDistinct ) )
			return false;

		if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty() )
		{
			auto pFilter = CreateAggrFilter ();
			if ( !pFilter )
				return false;

			m_tGroupSorterSettings.m_pAggrFilterTrait = pFilter.release();
		}
	}

	for ( auto & tIdx: m_hExtra )
	{
		m_hQueryColumns.Add ( tIdx.first );
		if ( m_pExtra )
			m_pExtra->Add ( tIdx.first );
	}

	return true;
}


bool QueueCreator_c::PredictAggregates() const
{
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr(i);
		if ( !(tAttr.m_eAggrFunc==SPH_AGGR_NONE || IsGroupbyMagic ( tAttr.m_sName ) || IsSortStringInternal ( tAttr.m_sName.cstr () )) )
			return true;
	}

	return false;
}


int QueueCreator_c::ReduceMaxMatches() const
{
	assert ( !m_bGotGroupby );
	if ( m_tQuery.m_bExplicitMaxMatches || m_tQuery.m_bHasOuter || !m_tSettings.m_bComputeItems )
		return Max ( m_tSettings.m_iMaxMatches, 1 );

	return Max ( Min ( m_tSettings.m_iMaxMatches, m_tQuery.m_iLimit+m_tQuery.m_iOffset ), 1 );
}


int QueueCreator_c::AdjustMaxMatches ( int iMaxMatches ) const
{
	assert ( m_bGotGroupby );
	if ( m_tQuery.m_bExplicitMaxMatches || m_tSettings.m_bForceSingleThread )
		return iMaxMatches;

	int iGroupbyAttr = GetGroupbyAttrIndex();
	if ( iGroupbyAttr<0 )
		return iMaxMatches;

	int iCountDistinct = m_tSettings.m_fnGetCountDistinct ? m_tSettings.m_fnGetCountDistinct ( m_pSorterSchema->GetAttr(iGroupbyAttr).m_sName ) : -1;
	if ( iCountDistinct > m_tQuery.m_iMaxMatchThresh )
		return iMaxMatches;

	return Max ( iCountDistinct, iMaxMatches );
}


bool QueueCreator_c::CanCalcFastCountDistinct() const
{
	bool bHasAggregates = PredictAggregates();
	return !bHasAggregates && m_tGroupSorterSettings.m_bImplicit && m_tGroupSorterSettings.m_bDistinct && m_tQuery.m_dFilters.IsEmpty() && m_tQuery.m_sQuery.IsEmpty();
}


bool QueueCreator_c::CanCalcFastCount() const
{
	bool bHasAggregates = PredictAggregates();
	return !bHasAggregates && m_tGroupSorterSettings.m_bImplicit && !m_tGroupSorterSettings.m_bDistinct && m_tQuery.m_dFilters.GetLength()==1 && m_tQuery.m_sQuery.IsEmpty();
}


Precalculated_t QueueCreator_c::FetchPrecalculatedValues() const
{
	Precalculated_t tPrecalc;
	if ( CanCalcFastCountDistinct() )
	{
		int iCountDistinctAttr = GetGroupDistinctAttrIndex();
		if ( iCountDistinctAttr>0 && m_tSettings.m_bEnableFastDistinct )
			tPrecalc.m_iCountDistinct = m_tSettings.m_fnGetCountDistinct ? m_tSettings.m_fnGetCountDistinct ( m_pSorterSchema->GetAttr(iCountDistinctAttr).m_sName ) : -1;
	}

	if ( CanCalcFastCount() )
		tPrecalc.m_iCount = m_tSettings.m_fnGetCount ? m_tSettings.m_fnGetCount ( m_tQuery.m_dFilters[0] ) : -1;

	return tPrecalc;
}


ISphMatchSorter * QueueCreator_c::SpawnQueue()
{
	bool bNeedFactors = !!(m_uPackedFactorFlags & SPH_FACTOR_ENABLE);

	if ( m_bGotGroupby )
	{
		m_tGroupSorterSettings.m_iMaxMatches = AdjustMaxMatches ( m_tGroupSorterSettings.m_iMaxMatches );
		if ( m_pProfile )
			m_pProfile->m_iMaxMatches = m_tGroupSorterSettings.m_iMaxMatches;

		Precalculated_t tPrecalc = FetchPrecalculatedValues();
		return sphCreateSorter1st ( m_eMatchFunc, m_eGroupFunc, &m_tQuery, m_tGroupSorterSettings, bNeedFactors, PredictAggregates(), tPrecalc );
	}

	if ( m_tSettings.m_pCollection )
		return new CollectQueue_c ( m_tSettings.m_iMaxMatches, *m_tSettings.m_pCollection );

	int iMaxMatches = ReduceMaxMatches();
	if ( m_pProfile )
		m_pProfile->m_iMaxMatches = iMaxMatches;

	ISphMatchSorter * pResult = CreatePlainSorter ( m_eMatchFunc, m_tQuery.m_bSortKbuffer, iMaxMatches, bNeedFactors );
	if ( !pResult )
		return nullptr;

	return CreateColumnarProxySorter ( pResult, iMaxMatches, *m_pSorterSchema, m_tStateMatch, m_eMatchFunc, bNeedFactors, m_tSettings.m_bComputeItems, m_bMulti );
}


bool QueueCreator_c::SetupComputeQueue ()
{
	return MaybeAddGeodistColumn ()
		&& MaybeAddExprColumn ()
		&& MaybeAddExpressionsFromSelectList ()
		&& AddExpressionsForUpdates();
}

bool QueueCreator_c::SetupGroupQueue ()
{
	return AddGroupbyStuff ()
		&& SetupMatchesSortingFunc ()
		&& SetGroupSorting ();
}

bool QueueCreator_c::ConvertColumnarToDocstore()
{
	// don't use docstore (need to try to keep schemas similar for multiquery to work)
	if ( m_tQuery.m_bFacet || m_tQuery.m_bFacetHead )
		return true;

	// check for columnar attributes that have FINAL eval stage
	// if we have more than 1 of such attributes (and they are also stored), we replace columnar expressions with columnar expressions
	CSphVector<int> dStoredColumnar;
	auto & tSchema = *m_pSorterSchema;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		auto & tAttr = tSchema.GetAttr(i);
		bool bStored = false;
		bool bColumnar = tAttr.m_pExpr && tAttr.m_pExpr->IsColumnar(&bStored);
		if ( bColumnar && bStored && tAttr.m_eStage==SPH_EVAL_FINAL )
			dStoredColumnar.Add(i);
	}

	if ( dStoredColumnar.GetLength()<=1 )
		return true;

	for ( auto i : dStoredColumnar )
	{
		auto & tAttr = const_cast<CSphColumnInfo&>( tSchema.GetAttr(i) );

		CSphString sColumnarAttrName;
		tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarAttrName );
		tAttr.m_pExpr = CreateExpr_GetStoredAttr ( sColumnarAttrName, tAttr.m_eAttrType );
	}

	return true;
}

bool QueueCreator_c::SetupQueue ()
{
	return SetupComputeQueue ()
		&& SetupGroupQueue ()
		&& ConvertColumnarToDocstore();
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
	pTop->SetSchema ( m_pSorterSchema.release(), false );
	pTop->SetState ( m_tStateMatch );
	pTop->SetGroupState ( m_tStateGroup );
	pTop->SetRandom ( m_bRandomize );
	if ( !m_bHaveStar && m_hQueryColumns.GetLength() )
		pTop->SetFilteredAttrs ( m_hQueryColumns, m_tSettings.m_bNeedDocids || m_bExprsNeedDocids );

	if ( m_bRandomize )
	{
		if ( m_tQuery.m_iRandSeed>=0 )
			sphSrand ( (DWORD)m_tQuery.m_iRandSeed );
		else
			sphAutoSrand();
	}

	return pTop;
}

static void ResetRemaps ( CSphMatchComparatorState & tState )
{
	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) && tState.m_eKeypart[i]==SPH_KEYPART_STRINGPTR )
			tState.m_dRemapped.BitClear ( i );
	}
}

bool QueueCreator_c::SetSchemaGroupQueue ( const CSphRsetSchema & tNewSchema )
{
	// need to reissue remap but with existed attributes
	ResetRemaps ( m_tStateMatch );
	ResetRemaps ( m_tStateGroup );

	*m_pSorterSchema = tNewSchema;

	return SetupGroupQueue();
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
	return !tQuery.m_dItems.all_of ( [&tSchema] ( const CSphQueryItem& tItem )
	{
		const CSphString & sExpr = tItem.m_sExpr;

		// all expressions that come from parser are automatically aliased
		assert ( !tItem.m_sAlias.IsEmpty() );

		return sExpr=="*"
			|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && tItem.m_sAlias==sExpr )
			|| IsGroupbyMagic ( sExpr );
	});
}


int GetAliasedAttrIndex ( const CSphString & sAttr, const CSphQuery & tQuery, const ISphSchema & tSchema )
{
	int iAttr = tSchema.GetAttrIndex ( sAttr.cstr() );
	if ( iAttr>=0 )
		return iAttr;

	// try aliased groupby attr (facets)
	ARRAY_FOREACH ( i, tQuery.m_dItems )
	{
		if ( sAttr==tQuery.m_dItems[i].m_sExpr )
			return tSchema.GetAttrIndex ( tQuery.m_dItems[i].m_sAlias.cstr() );
		else if ( sAttr==tQuery.m_dItems[i].m_sAlias )
			return tSchema.GetAttrIndex ( tQuery.m_dItems[i].m_sExpr.cstr() );
	}

	return iAttr;
}


static void CreateSorters ( const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<ISphMatchSorter*> & dSorters, const VecTraits_T<QueueCreator_c> & dCreators,	const VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes )
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


int ApplyImplicitCutoff ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters )
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

	bool bDisableCutoff = dSorters.any_of ( []( auto * pSorter ){ return pSorter->IsCutoffDisabled(); } );
	if ( bDisableCutoff )
		return -1;

	// implicit cutoff when there's no sorting and no grouping 
	if ( ( tQuery.m_sSortBy=="@weight desc" || tQuery.m_sSortBy.IsEmpty() ) && tQuery.m_sGroupBy.IsEmpty() && !tQuery.m_bFacet && !tQuery.m_bFacetHead )
		return tQuery.m_iLimit+tQuery.m_iOffset;

	return -1;
}


ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	QueueCreator_c tCreator ( tQueue, tQuery, sError, pExtra, pProfile );
	if ( !tCreator.SetupQueue () )
		return nullptr;

	return CreateQueue ( tCreator, tRes );
}


static void CreateMultiQueue ( RawVector_T<QueueCreator_c> & dCreators, const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	assert ( dSorters.GetLength()>1 );
	assert ( dSorters.GetLength()==dQueries.GetLength() );
	assert ( dSorters.GetLength()==dErrors.GetLength() );

	dCreators.Reserve_static ( dSorters.GetLength () );
	dCreators.Emplace_back( tQueue, dQueries[0], dErrors[0], pExtra, pProfile );
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
		dCreators.Emplace_back ( tQueue, dQueries[i], dErrors[i], pExtra, pProfile );
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
			if ( !tCol.m_tLocator.m_bDynamic && !tCol.IsColumnar() )
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
					( ( !pMultiCol->m_pExpr && !tCol.m_pExpr ) ||
					( pMultiCol->m_pExpr && tCol.m_pExpr
						&& pMultiCol->m_pExpr->GetHash ( tMultiSchema, SPH_FNV64_SEED, bDisable1 )==tCol.m_pExpr->GetHash ( tSchema, SPH_FNV64_SEED, bDisable2 ) )
					) )
					continue;

				// no need to add a new column, but we need the same schema for the sorters
				if ( tCol.IsColumnar() && pMultiCol->IsColumnarExpr() )
				{
					bHasMulti = true;
					continue;
				}

				if ( !tCol.IsColumnarExpr() || !pMultiCol->IsColumnar() ) 				// need a new column
				{
					tRes.m_bAlowMulti = false; // if attr or expr differs need to create regular sorters and issue search WO multi-query
					return;
				}
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
	for ( QueueCreator_c & tCreator : dCreators )
	{
		if ( !tCreator.m_bCreate )
			continue;

		if ( !tCreator.SetSchemaGroupQueue ( tMultiSchema ) )
			tCreator.m_bCreate = false;
	}
}


void sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	RawVector_T<QueueCreator_c> dCreators;
	CreateMultiQueue ( dCreators, tQueue, dQueries, dSorters, dErrors, tRes, pExtra, pProfile );
	CreateSorters ( dQueries, dSorters, dCreators, dErrors, tRes );
}
