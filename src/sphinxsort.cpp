//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
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
#include "exprtraits.h"
#include "sortcomp.h"
#include "aggregate.h"
#include "distinct.h"
#include "netreceive_ql.h"
#include "queuecreator.h"

#include <ctime>
#include <cmath>

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


namespace {

template <typename FN>
void FnSortGetStringRemap ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema, FN fnProcess )
{
	for ( int i = 0; i<tDstSchema.GetAttrsCount (); ++i )
	{
		const CSphColumnInfo & tDst = tDstSchema.GetAttr ( i );
		// remap only static strings
		if ( tDst.m_eAttrType==SPH_ATTR_STRINGPTR || !IsSortStringInternal ( tDst.m_sName ) )
			continue;

		auto iSrcCol = tSrcSchema.GetAttrIndex ( tDst.m_sName.cstr ()+sizeof ( GetInternalAttrPrefix() )-1 );
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
	void	Finalize();

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

	if ( tAttr.m_iIndex==-1 )
		tAttr.m_iIndex = m_tOldSchema.GetAttrIndexOriginal ( tAttr.m_sName.cstr() );

	// check if new columnar attributes were added (that were not in the select list originally)
	if ( tAttr.IsColumnar() )
		ReplaceColumnarAttrWithExpression ( tAttr, m_tNewSchema.GetAttrsCount() );

	tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );

	m_tNewSchema.AddAttr ( tAttr, true );
}


void TransformedSchemaBuilder_c::Finalize()
{
	const CSphColumnInfo * pOld = m_tOldSchema.GetAttr ( GetNullMaskAttrName() );
	if ( !pOld )
		return;

	const CSphColumnInfo * pNew = m_tNewSchema.GetAttr ( GetNullMaskAttrName() );
	assert(!pNew);

	CSphColumnInfo tAttr ( GetNullMaskAttrName(), DetermineNullMaskType ( m_tNewSchema.GetAttrsCount() ) );
	m_tNewSchema.AddAttr ( tAttr,  true );
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
	tAttr.m_pExpr = sphExprParse ( tAttr.m_sName.cstr(), m_tNewSchema, nullptr, sError, tExprArgs );
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
			SETZERO,			// set default (0)
			COPY,				// copy as is (plain attribute)
			COPYBLOB,			// deep copy (unpack/pack) the blob
			COPYJSONFIELD,		// json field (packed blob with type)
			EVALEXPR_INT,		// evaluate the expression for the recently added int attribute
			EVALEXPR_BIGINT,	// evaluate the expression for the recently added bigint attribute
			EVALEXPR_STR,		// evaluate the expression for the recently added string attribute
			EVALEXPR_MVA,		// evaluate the expression for the recently added mva attribute
			NULLMASK_INT2INT,	// repack null attribute mask for the new schema
			NULLMASK_STR2INT,	// repack null attribute mask for the new schema
			NULLMASK_INT2STR,	// repack null attribute mask for the new schema
			NULLMASK_STR2STR	// repack null attribute mask for the new schema
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
	int						m_iNumDstAttrs = 0;	// num attrs in dst schema
	CSphVector<MapAction_t>	m_dActions;			// the recipe
	CSphVector<std::pair<CSphAttrLocator, CSphAttrLocator>> m_dRemapCmp;	// remap @int_attr_ATTR -> ATTR
	CSphVector<int>			m_dNullRemap;		// attr remap for null bitmaps
	CSphVector<int>			m_dDataPtrAttrs;	// orphaned attrs we have to free before swap to new attr
	GetBlobPoolFromMatch_fn	m_fnGetBlobPool;	// provides base for pool copying
	GetColumnarFromMatch_fn	m_fnGetColumnar;	// columnar storage getter

	static void			SetupAction ( const CSphColumnInfo & tOld, const CSphColumnInfo & tNew, const ISphSchema * pOldSchema, MapAction_t & tAction );

	void				SetupNullMaskRemap ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema );
	FORCE_INLINE void	ProcessMatch ( CSphMatch * pMatch );
	FORCE_INLINE void	PerformAction ( const MapAction_t & tAction, CSphMatch * pMatch, CSphMatch & tResult, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar );
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

	SetupNullMaskRemap ( pOldSchema, pNewSchema );
	m_iNumDstAttrs = pNewSchema->GetAttrsCount();
}


void MatchesToNewSchema_c::SetupNullMaskRemap ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	if ( !pOldSchema->GetAttr ( GetNullMaskAttrName() ) )
		return;

	for ( int i = 0; i < pOldSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tOldAttr = pOldSchema->GetAttr(i);
		if ( tOldAttr.m_tLocator.m_bDynamic )
			m_dNullRemap.Add ( pNewSchema->GetAttrIndex ( tOldAttr.m_sName.cstr() ) );
	}
}


void MatchesToNewSchema_c::SetupAction ( const CSphColumnInfo & tOld, const CSphColumnInfo & tNew, const ISphSchema * pOldSchema, MapAction_t & tAction )
{
	tAction.m_pFrom = &tOld.m_tLocator;

	if ( tOld.m_sName==GetNullMaskAttrName() )
	{
		bool bOldInt = tOld.m_eAttrType==SPH_ATTR_INTEGER || tOld.m_eAttrType==SPH_ATTR_BIGINT;
		bool bNewInt = tNew.m_eAttrType==SPH_ATTR_INTEGER || tNew.m_eAttrType==SPH_ATTR_BIGINT;

		if ( bOldInt )
			tAction.m_eAction = bNewInt ? MapAction_t::NULLMASK_INT2INT : MapAction_t::NULLMASK_INT2STR;
		else
			tAction.m_eAction = bNewInt ? MapAction_t::NULLMASK_STR2INT : MapAction_t::NULLMASK_STR2STR;

		return;
	}

	// columnar attr replaced by an expression
	// we now need to create an expression that fetches data from columnar storage
	if ( tOld.IsColumnar() && tNew.m_pExpr )
	{
		CSphString		sError;
		ExprParseArgs_t tExprArgs;
		tAction.m_pExpr =  sphExprParse ( tOld.m_sName.cstr(), *pOldSchema, nullptr, sError, tExprArgs );
		assert ( tAction.m_pExpr );

		switch ( tNew.m_eAttrType )
		{
		case SPH_ATTR_STRINGPTR:		tAction.m_eAction = MapAction_t::EVALEXPR_STR;		break;
		case SPH_ATTR_BIGINT:			tAction.m_eAction = MapAction_t::EVALEXPR_BIGINT;	break;
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET_PTR:
		case SPH_ATTR_FLOAT_VECTOR_PTR:	tAction.m_eAction = MapAction_t::EVALEXPR_MVA;		break;
		default:						tAction.m_eAction = MapAction_t::EVALEXPR_INT;		break;
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


void MatchesToNewSchema_c::PerformAction ( const MapAction_t & tAction, CSphMatch * pMatch, CSphMatch & tResult, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar )
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

	case MapAction_t::NULLMASK_INT2INT:
	{
		SphAttr_t uSrcValue = pMatch->GetAttr ( *tAction.m_pFrom );
		for ( int i = 0; i < m_dNullRemap.GetLength(); i++ )
			if ( ( uSrcValue & ( 1ULL << i ) ) && m_dNullRemap[i]!=-1 )
				uValue |= 1ULL << m_dNullRemap[i];
	}
	break;

	case MapAction_t::NULLMASK_STR2INT:
	{
		ByteBlob_t dUnpacked = sphUnpackPtrAttr ( (const BYTE*)pMatch->GetAttr ( *tAction.m_pFrom ) );
		int iNumElements = dUnpacked.second*8;
		BitVec_T<BYTE> tSrcMask ( (BYTE*)dUnpacked.first, iNumElements );
		for ( int i = 0; i < iNumElements; i++ )
			if ( tSrcMask.BitGet(i) && m_dNullRemap[i]!=-1 )
				uValue |= 1ULL << m_dNullRemap[i];
	}
	break;

	case MapAction_t::NULLMASK_INT2STR:
	{
		SphAttr_t uSrcValue = pMatch->GetAttr ( *tAction.m_pFrom );
		BitVec_T<BYTE> tDstMask ( m_iNumDstAttrs );
		for ( int i = 0; i < m_dNullRemap.GetLength(); i++ )
			if ( ( uSrcValue & ( 1ULL << i ) ) && m_dNullRemap[i]!=-1 )
				tDstMask.BitSet ( m_dNullRemap[i] );

		uValue = (SphAttr_t)sphPackPtrAttr ( { tDstMask.Begin(), tDstMask.GetSizeBytes() } );
	}
	break;

	case MapAction_t::NULLMASK_STR2STR:
	{
		ByteBlob_t dUnpacked = sphUnpackPtrAttr ( (const BYTE*)pMatch->GetAttr ( *tAction.m_pFrom ) );
		int iNumElements = dUnpacked.second*8;
		BitVec_T<BYTE> tSrcMask ( (BYTE*)dUnpacked.first, iNumElements );
		BitVec_T<BYTE> tDstMask ( m_iNumDstAttrs );
		for ( int i = 0; i < iNumElements; i++ )
			if ( tSrcMask.BitGet(i) && m_dNullRemap[i]!=-1 )
				tDstMask.BitSet ( m_dNullRemap[i] );

		uValue = (SphAttr_t)sphPackPtrAttr ( { tDstMask.Begin(), tDstMask.GetSizeBytes() } );
	}
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
		const CSphColumnInfo* pId = pOldSchema->GetAttr ( sphGetDocidName() );
		if ( pId )
			tBuilder.AddAttr ( sphGetDocidName() );

		// add the rest
		for ( int i = 0; i<pOldSchema->GetAttrsCount (); i++ )
		{
			const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
			if ( tAttr.m_sName!=sphGetDocidName() && tAttr.m_sName!=GetNullMaskAttrName() )
				tBuilder.AddAttr ( tAttr.m_sName );
		}
	}
	else
	{
		// keep id as the first attribute, then the rest.
		m_dTransformed.any_of ( [&tBuilder] ( const auto& sName ) { auto bID = ( sName==sphGetDocidName() ); if ( bID ) tBuilder.AddAttr(sName); return bID; } );
		m_dTransformed.for_each ( [&tBuilder] ( const auto& sName ) { if ( sName!=sphGetDocidName() && sName!=GetNullMaskAttrName() ) tBuilder.AddAttr(sName); } );
	}

	for ( int i = 0; i <pNewSchema->GetAttrsCount(); ++i )
	{
		auto & pExpr = pNewSchema->GetAttr(i).m_pExpr;
		if ( pExpr )
			pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	tBuilder.Finalize();

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

	int ResetDynamic ( int iMaxUsed )
	{
		for ( int i=0; i<iMaxUsed; i++ )
			m_dData[i].ResetDynamic();

		return -1;
	}

	int ResetDynamicFreeData ( int iMaxUsed )
	{
		for ( int i=0; i<iMaxUsed; i++ )
		{
			m_pSchema->FreeDataPtrs ( m_dData[i] );
			m_dData[i].ResetDynamic();
		}

		return -1;
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
	for ( int i : dOrder )
	{
		const CSphColumnInfo& tCol = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( tCol ) )
			continue;
		pRows->HeadColumn ( tCol.m_sName.cstr(), ESphAttr2MysqlColumn ( tCol.m_eAttrType ) );
	}

	pRows->HeadEnd ( false, 0 );
}

void SendSqlMatch ( const ISphSchema& tSchema, RowBuffer_i* pRows, CSphMatch& tMatch, const BYTE* pBlobPool, const VecTraits_T<int>& dOrder, bool bDynamicDocid )
{
	auto& dRows = *pRows;
	for ( int i : dOrder )
	{
		const CSphColumnInfo& dAttr = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( dAttr ) )
			continue;

		CSphAttrLocator tLoc = dAttr.m_tLocator;
		ESphAttr eAttrType = dAttr.m_eAttrType;

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
				sphMVA2Str ( dMVA, eAttrType == SPH_ATTR_INT64SET, dStr );
				dRows.PutArray ( dStr, false );
				break;
			}

		case SPH_ATTR_INT64SET_PTR:
		case SPH_ATTR_UINT32SET_PTR:
			{
				StringBuilder_c dStr;
				sphPackedMVA2Str ( (const BYTE*)tMatch.GetAttr ( tLoc ), eAttrType == SPH_ATTR_INT64SET_PTR, dStr );
				dRows.PutArray ( dStr, false );
				break;
			}

		case SPH_ATTR_FLOAT_VECTOR:
			{
				StringBuilder_c dStr;
				auto dFloatVec = sphGetBlobAttr ( tMatch, tLoc, pBlobPool );
				sphFloatVec2Str ( dFloatVec, dStr );
				dRows.PutArray ( dStr, false );
			}
			break;

		case SPH_ATTR_FLOAT_VECTOR_PTR:
			{
				StringBuilder_c dStr;
				sphPackedFloatVec2Str ( (const BYTE*)tMatch.GetAttr(tLoc), dStr );
				dRows.PutArray ( dStr, false );
			}
			break;

		case SPH_ATTR_JSON:
			{
				auto pJson = sphGetBlobAttr ( tMatch, tLoc, pBlobPool );
				JsonEscapedBuilder sTmp;
				if ( pJson.second )
					sphJsonFormat ( sTmp, pJson.first );
				dRows.PutArray ( sTmp );
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
				dRows.PutArray ( sTmp );
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

/////////////////////////////////////////////////////////////////////////////
/// group sorting functor
template < typename COMPGROUP >
struct GroupSorter_fn : public CSphMatchComparatorState, public MatchSortAccessor_t
{
	const VecTraits_T<CSphMatch> & m_dBase;

	explicit GroupSorter_fn ( const CSphMatchQueueTraits& dBase )
		: m_dBase ( dBase.GetMatches() )
	{}

	FORCE_INLINE bool IsLess ( int a, int b ) const
	{
		return COMPGROUP::IsLess ( m_dBase[b], m_dBase[a], *this );
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
	template <int DISTINCT>
	inline void SetupBaseGrouper ( ISphSchema * pSchema, CSphVector<AggrFunc_i *> * pAvgs = nullptr )
	{
		m_tPregroup.ResetAttrs();
		ResetAggregates();

		m_tPregroup.SetSchema ( pSchema );
		m_tPregroup.AddRaw ( m_tLocGroupby ); // @groupby
		m_tPregroup.AddRaw ( m_tLocCount ); // @count
		if constexpr ( DISTINCT )
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
template<typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS>
class KBufferGroupSorter_T : public CSphMatchQueueTraits, protected BaseGroupSorter_c
{
	using MYTYPE = KBufferGroupSorter_T<COMPGROUP,UNIQ,DISTINCT,NOTIFICATIONS>;
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
		if constexpr ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

		m_pGrouper = tSettings.m_pGrouper;
		m_pDistinctFetcher = tSettings.m_pDistinctFetcher;
		m_tUniq.SetAccuracy ( tSettings.m_iDistinctAccuracy );
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
		if constexpr ( DISTINCT )
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
	UNIQ						m_tUniq;
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
		for ( int iCount = m_tUniq.CountStart ( uGroup ); iCount; iCount = m_tUniq.CountNext ( uGroup ) )
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

	FORCE_INLINE void FreeMatchPtrs ( int iMatch, bool bNotify=true )
	{
		if ( NOTIFICATIONS && bNotify )
			m_dJustPopped.Add ( RowTagged_t ( m_dData[iMatch] ) );

		m_pSchema->FreeDataPtrs ( m_dData[iMatch] );

		// on final pass we totally wipe match.
		// That is need, since otherwise such 'garbage' matches with non-null m_pDynamic
		// will be targeted in d-tr with FreeDataPtrs with possible another(!) schema
		if ( !bNotify )
			m_dData[iMatch].ResetDynamic ();
	}

	template <bool GROUPED>
	FORCE_INLINE void UpdateDistinct ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey )
	{
		int iCount = 1;
		if constexpr ( GROUPED )
			iCount = (int)tEntry.GetAttr ( m_tLocDistinct );

		assert(m_pDistinctFetcher);

		if constexpr ( DISTINCT==1 )
			m_tUniq.Add ( {uGroupKey, m_pDistinctFetcher->GetKey(tEntry), iCount} );
		else
		{
			m_pDistinctFetcher->GetKeys ( tEntry, this->m_dDistinctKeys );
			for ( auto i : this->m_dDistinctKeys )
				m_tUniq.Add ( {uGroupKey, i, iCount} );
		}	
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
template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferGroupSorter : public KBufferGroupSorter_T<COMPGROUP,UNIQ,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
	bool m_bMatchesFinalized = false;
	int m_iMaxUsed = -1;

protected:
	OpenHashTableFastClear_T <SphGroupKey_t, CSphMatch *> m_hGroup2Match;

	// since we inherit from template, we need to write boring 'using' block
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS>;
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
	using CSphMatchQueueTraits::ResetDynamic;
	using CSphMatchQueueTraits::ResetDynamicFreeData;

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

	bool	Push ( const CSphMatch & tEntry ) override						{ return PushEx<false> ( tEntry, m_pGrouper->KeyFromMatch(tEntry), false, false, true, nullptr ); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) override	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) override			{ return PushEx<true> ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), false, false, true, nullptr ); }
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
			if constexpr ( HAS_AGGREGATES )
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

		if constexpr ( DISTINCT )
			m_tUniq.Reset();

		ResetAfterFlatten ();
		m_iMaxUsed = ResetDynamic ( m_iMaxUsed );

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
			dRhs.m_hGroup2Match = std::move ( m_hGroup2Match );
			dRhs.m_bMatchesFinalized = m_bMatchesFinalized;
			dRhs.m_iMaxUsed = m_iMaxUsed;
			if ( !m_bMatchesFinalized && bCopyMeta )
				dRhs.m_tUniq = std::move(m_tUniq);

			m_iMaxUsed = -1;
			return;
		}

		bool bUniqUpdated = false;
		if ( !m_bMatchesFinalized && bCopyMeta )
		{
			// can not move m_tUniq into dRhs as move invalidates m_tUniq then breaks FinalizeMatches
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
		m_iMaxUsed = ResetDynamicFreeData ( m_iMaxUsed );
	}

	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) override
	{
		if ( !Used() )
			return;

		if ( bFinalizeMatches )
			FinalizeMatches();
		else if constexpr ( DISTINCT )
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

		if constexpr ( DISTINCT )
		{
			// need to clean up matches NOT from m_dIData with current schema
			// as after schema change data_ptr attributes will have garbage in ptr part for matches not processed by tProcessor
			// and global sorters have differrent clean up code path that do not handle this garbage as usual sorters do
			if ( this->m_dIData.GetLength()!=m_iMaxUsed )
			{
				for ( int i=0; i<m_iMaxUsed; i++ )
				{
					CSphMatch & tMatch = m_dData[i];
					if ( !tMatch.m_pStatic ) // clean up match that was in m_dIData set
						continue;

					m_pSchema->FreeDataPtrs ( tMatch );
					tMatch.ResetDynamic ();
				}
			}
		}

	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	template <bool GROUPED>
	bool PushIntoExistingGroup( CSphMatch & tGroup, const CSphMatch & tEntry, SphGroupKey_t uGroupKey, SphAttr_t * pAttr )
	{
		assert ( tGroup.GetAttr ( m_tLocGroupby )==uGroupKey );
		assert ( tGroup.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

		auto & tLocCount = m_tLocCount;
		if constexpr ( GROUPED )
			tGroup.AddCounterAttr ( tLocCount, tEntry );
		else
			tGroup.AddCounterScalar ( tLocCount, 1 );

		if constexpr ( HAS_AGGREGATES )
			AggrUpdate ( tGroup, tEntry, GROUPED, m_bMerge );

		// if new entry is more relevant, update from it
		if ( m_tSubSorter.MatchIsGreater ( tEntry, tGroup ) )
		{
			if constexpr ( NOTIFICATIONS )
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
			KBufferGroupSorter::template UpdateDistinct<GROUPED> ( tEntry, uGroupKey );

		return false; // since it is a dupe
	}

	/// add entry to the queue
	template <bool GROUPED>
	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, [[maybe_unused]] bool bNewSet, [[maybe_unused]] bool bTailFinalized, bool bClearNotify, SphAttr_t * pAttr )
	{
		if constexpr ( NOTIFICATIONS )
		{
			if ( bClearNotify )
			{
				m_tJustPushed = RowTagged_t();
				this->m_dJustPopped.Resize ( 0 );
			}
		}
		auto & tLocCount = m_tLocCount;

		m_bMatchesFinalized = false;
		if ( HAS_AGGREGATES && m_bAvgFinal )
			CalcAvg ( Avg_e::UNGROUP );

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match.Find ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tLocGroupby )==uGroupKey );
			return PushIntoExistingGroup<GROUPED> ( *pMatch, tEntry, uGroupKey, pAttr );
		}

		// submit actual distinct value
		if constexpr ( DISTINCT )
			KBufferGroupSorter::template UpdateDistinct<GROUPED> ( tEntry, uGroupKey );

		// if we're full, let's cut off some worst groups
		if ( Used()==m_iSize )
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );

		// do add
		assert ( Used()<m_iSize );
		CSphMatch & tNew = Add();
		m_pSchema->CloneMatch ( tNew, tEntry );

		if constexpr ( HAS_AGGREGATES )
			AggrSetup ( tNew, tEntry, m_bMerge );

		if constexpr ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( tNew );

		if constexpr ( GROUPED )
		{
			if constexpr ( HAS_AGGREGATES )
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
	CSphVector<SphGroupKey_t> m_dRemove;

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
		if ( DISTINCT && m_bSortByDistinct )
			CountDistinct ();

		CalcAvg ( Avg_e::FINALIZE );

		// relocate best matches to the low part (up to the iBound)
		BinaryPartition (iBound);

		// take worst matches and free them (distinct stuff, data ptrs)
		auto dWorst = this->m_dIData.Slice ( iBound );

		if constexpr ( DISTINCT )
		{
			m_dRemove.Resize(0);
			for ( auto iMatch : dWorst )
				m_dRemove.Add ( m_dData[iMatch].GetAttr ( m_tLocGroupby ));
			RemoveDistinct ( m_dRemove );
		}

		dWorst.Apply ( [this,bFinalize] ( int iMatch ) { FreeMatchPtrs ( iMatch, !bFinalize ); } );

		m_iMaxUsed = Max ( m_iMaxUsed, this->m_dIData.GetLength() ); // memorize it for free dynamics later.
		this->m_dIData.Resize ( iBound );
		m_hGroup2Match.Clear();

		if ( bFinalize )
		{
			SortGroups();
			if ( DISTINCT && !m_bSortByDistinct ) // since they haven't counted at the top
			{
				RebuildHash(); // distinct uses m_hGroup2Match
				CountDistinct();
			}
		} else
		{
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

template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferNGroupSorter : public KBufferGroupSorter_T<COMPGROUP,UNIQ,DISTINCT,NOTIFICATIONS>
{
	using MYTYPE = CSphKBufferNGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS,HAS_AGGREGATES>;

protected:
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS>;
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

	bool	Push ( const CSphMatch & tEntry ) override						{ return PushEx<false> ( tEntry, m_pGrouper->KeyFromMatch(tEntry), false, false, true, nullptr ); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) final	{ assert ( 0 && "Not supported in grouping"); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override	{ return PushEx<true> ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), bNewSet, false, true, nullptr ); }

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
		if constexpr ( DISTINCT )
			m_tUniq.Reset();

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

			if constexpr ( DISTINCT )
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
			dRhs.m_hGroup2Index = std::move ( m_hGroup2Index );
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
				dRhs.m_tUniq = std::move(m_tUniq);

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
			dRhs.template PushEx<true> ( m_dData[iHead], uGroupKey, true, true, true, nullptr );
			for ( int i = this->m_dIData[iHead]; i!=iHead; i = this->m_dIData[i] )
				dRhs.template PushEx<false> ( m_dData[i], uGroupKey, false, true, true, nullptr );

			DeleteChain ( iHead, false );
		}

		dRhs.m_bUpdateDistinct = true;
		dRhs.SetMerge(false);

		dRhs.m_iTotal = m_iTotal+iTotal;
	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	int m_iStorageSolidFrom = 0; // edge from witch storage is not yet touched and need no chaining freelist
	OpenHashTable_T<SphGroupKey_t, int> m_hGroup2Index; // used to quickly locate group for incoming match

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
	bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bNewSet, bool bTailFinalized, bool bClearNotify, [[maybe_unused]] SphAttr_t * pAttr )
	{

#ifndef NDEBUG
		++m_ipushed;
		DBG << "PushEx: tag" << tEntry.m_iTag << ",g" << uGroupKey << ": pushed" << m_ipushed
			<< " g" << GROUPED << " n" << bNewSet;
#endif

		if constexpr ( NOTIFICATIONS )
		{
			if ( bClearNotify )
			{
				m_tJustPushed = RowTagged_t();
				this->m_dJustPopped.Resize ( 0 );
			}
		}

		this->m_bFinalized = false;
		if ( HAS_AGGREGATES && m_bAvgFinal )
			CalcAvg ( Avg_e::UNGROUP );

		// place elem into the set
		auto iNew = AllocateMatch ();
		CSphMatch & tNew = m_dData[iNew];

		// if such group already hashed
		int * pGroupIdx = m_hGroup2Index.Find ( uGroupKey );
		if ( pGroupIdx )
			return PushAlreadyHashed<GROUPED> ( pGroupIdx, iNew, tEntry, uGroupKey, bNewSet, bTailFinalized );

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
			KBufferGroupSorter::template UpdateDistinct<GROUPED> ( tNew, uGroupKey );

		if constexpr ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( tNew );

		this->m_dIData[iNew] = iNew; // new head - points to self (0-ring)
		Verify ( m_hGroup2Index.Add ( uGroupKey, iNew ));
		++m_iTotal;

		if constexpr ( GROUPED )
		{
			m_uLastGroupKey = uGroupKey;

			if constexpr ( HAS_AGGREGATES )
				AggrUngroup ( m_dData[iNew] );
		} else
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if constexpr ( DISTINCT )
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
	FORCE_INLINE void FreeMatch ( int iElem, bool bNotify ) // fixme! intersects with parent by name
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
		if constexpr ( NOTIFICATIONS )
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
	template <bool GROUPED>
	bool PushAlreadyHashed ( int * pHead, int iNew, const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bNewSet, bool bTailFinalized )
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
			KBufferGroupSorter::template UpdateDistinct<GROUPED> ( tEntry, uGroupKey );

		// update group-wide counters
		auto & tLocCount = m_tLocCount;
		if constexpr ( GROUPED )
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
		if constexpr ( HAS_AGGREGATES )
		{
			if ( bNewSet )
				AggrUpdate ( tHeadMatch, tEntry, GROUPED, m_bMerge );
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
			for ( auto tData = m_hGroup2Index.Iterate(i); tData.second; tData = m_hGroup2Index.Iterate(i) )
				m_dAvgs.Apply ( [this, &tData] ( AggrFunc_i * pAvg ) {
					pAvg->Finalize ( m_dData[*tData.second] );
				});
		else
			for ( auto tData = m_hGroup2Index.Iterate(i); tData.second; tData = m_hGroup2Index.Iterate(i) )
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
		for ( auto tData = m_hGroup2Index.Iterate(i); tData.second; tData = m_hGroup2Index.Iterate(i) )
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
		for ( auto tData = m_hGroup2Index.Iterate(i); tData.second; tData = m_hGroup2Index.Iterate(i) )
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
		for ( auto tData = m_hGroup2Index.Iterate(i); tData.second; tData = m_hGroup2Index.Iterate(i) )
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
				if constexpr ( DISTINCT )
					dRemovedHeads.Add( iRemoved );
				m_dFinalizedHeads.RemoveFast ( i-- );
			}

		// discard removed distinct
		if constexpr ( DISTINCT )
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
		if constexpr ( DISTINCT )
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
		ARRAY_FOREACH ( i, m_dKeys )
		{
			SphGroupKey_t tKey = m_dKeys[i];
			// need to clear notifications once per match - not for every pushed value
			bRes |= BASE::template PushEx<false> ( tMatch, tKey, false, false, ( i==0 ), nullptr );
		}

		return bRes;
	}

	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		return BASE::template PushEx<true> ( tEntry, tEntry.GetAttr ( BASE::m_tLocGroupby ), bNewSet, false, true, nullptr );
	}

private:
	CSphVector<SphGroupKey_t>	m_dKeys;
};

template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class MultiValueGroupSorter_T : public MultiValueGroupSorterTraits_T <CSphKBufferGroupSorter <COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>
{
	using BASE = MultiValueGroupSorterTraits_T <CSphKBufferGroupSorter < COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>;
	using MYTYPE = MultiValueGroupSorter_T < COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES >;

public:
	using BASE::BASE;

	ISphMatchSorter * Clone () const final { return this->template CloneSorterT<MYTYPE>(); }
};

template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class MultiValueNGroupSorter_T : public MultiValueGroupSorterTraits_T < CSphKBufferNGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>
{
	using BASE = MultiValueGroupSorterTraits_T <CSphKBufferNGroupSorter < COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>>;
	using MYTYPE = MultiValueNGroupSorter_T <COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;

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
template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES >
class CSphKBufferJsonGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES >
{
public:
	using BASE = CSphKBufferGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
	using MYTYPE = CSphKBufferJsonGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;

	// since we inherit from template, we need to write boring 'using' block
	using KBufferGroupSorter = KBufferGroupSorter_T<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS>;
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
		return BASE::template PushEx<true> ( tEntry, tEntry.GetAttr ( BASE::m_tLocGroupby ), bNewSet, false, true, nullptr );
	}

	ISphMatchSorter * Clone () const final
	{
		return this->template CloneSorterT<MYTYPE>();
	}

private:
	FORCE_INLINE bool PushMatch ( const CSphMatch & tMatch )
	{
		SphGroupKey_t uGroupKey = this->m_pGrouper->KeyFromMatch ( tMatch );
		const BYTE * pBlobPool = this->m_pGrouper->GetBlobPool();
		bool bClearNotify = true;

		return PushJsonField ( uGroupKey, pBlobPool, [this, &tMatch, &bClearNotify]( SphAttr_t * pAttr, SphGroupKey_t uMatchGroupKey )
			{
				bool bPushed = BASE::template PushEx<false> ( tMatch, uMatchGroupKey, false, false, bClearNotify, pAttr );
				bClearNotify = false; // need to clear notifications once per match - not for every pushed value
				return bPushed;
			}
		);
	}
};


/// implicit group-by sorter
/// invoked when no 'group-by', but count(*) or count(distinct attr) are in game
template < typename COMPGROUP, typename UNIQ, int DISTINCT, bool NOTIFICATIONS, bool HAS_AGGREGATES>
class CSphImplicitGroupSorter final : public MatchSorter_c, ISphNoncopyable, protected BaseGroupSorter_c
{
	using MYTYPE = CSphImplicitGroupSorter<COMPGROUP, UNIQ, DISTINCT, NOTIFICATIONS, HAS_AGGREGATES>;
	using BASE = MatchSorter_c;

public:
	CSphImplicitGroupSorter ( const ISphMatchComparator * DEBUGARG(pComp), const CSphQuery *, const CSphGroupSorterSettings & tSettings )
		: BaseGroupSorter_c ( tSettings )
	{
		assert ( !DISTINCT || tSettings.m_pDistinctFetcher );
		assert ( !pComp );

		if constexpr ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);

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
	bool	PushGrouped ( const CSphMatch & tEntry, bool ) final			{ return PushEx<true>(tEntry); }

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo ) final
	{
		assert ( m_bDataInitialized );

		CountDistinct ();

		if constexpr ( HAS_AGGREGATES )
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

		if constexpr ( DISTINCT )
			m_tUniq.Reset();

		return iCopied;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( MatchProcessor_i & tProcessor, bool, bool bFinalizeMatches ) final
	{
		if ( !GetLength() )
			return;

		tProcessor.Process ( &m_tData );

		if ( !bFinalizeMatches )
			m_tUniq.Compact();
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
			if ( bCopyMeta )
				dRhs.m_tUniq = std::move ( m_tUniq );

			return;
		}

		if ( bCopyMeta )
			m_tUniq.CopyTo ( dRhs.m_tUniq );

		// other step is a bit tricky:
		// we just can't add current count uniq to final; need to append m_tUniq instead,
		// so that final flattening will calculate real uniq count.
		dRhs.AddCount ( m_tData );

		if constexpr ( HAS_AGGREGATES )
			dRhs.UpdateAggregates ( m_tData, false, true );

		if ( !bCopyMeta && DISTINCT )
			dRhs.UpdateDistinct ( m_tData );
	}

	void SetMerge ( bool bMerge ) override { m_bMerge = bMerge; }

protected:
	CSphMatch	m_tData;
	bool		m_bDataInitialized = false;
	bool		m_bMerge = false;

	UNIQ		 m_tUniq;

private:
	CSphVector<SphAttr_t> m_dDistinctKeys;
	CSphRefcountedPtr<DistinctFetcher_i> m_pDistinctFetcher;

	inline void SetupBaseGrouperWrp ( ISphSchema * pSchema )	{ SetupBaseGrouper<DISTINCT> ( pSchema ); }
	void	AddCount ( const CSphMatch & tEntry )				{ m_tData.AddCounterAttr ( m_tLocCount, tEntry ); }
	void	UpdateAggregates ( const CSphMatch & tEntry, bool bGrouped = true, bool bMerge = false ) { AggrUpdate ( m_tData, tEntry, bGrouped, bMerge ); }
	void	SetupAggregates ( const CSphMatch & tEntry )		{ AggrSetup ( m_tData, tEntry, m_bMerge ); }

	// submit actual distinct value in all cases
	template <bool GROUPED = true>
	void UpdateDistinct ( const CSphMatch & tEntry )
	{
		int iCount = 1;
		if constexpr ( GROUPED )
			iCount = (int) tEntry.GetAttr ( m_tLocDistinct );

		if constexpr ( DISTINCT==1 )
			m_tUniq.Add ( { 0, m_pDistinctFetcher->GetKey(tEntry), iCount } );
		else
		{
			m_pDistinctFetcher->GetKeys ( tEntry, m_dDistinctKeys );
			for ( auto i : m_dDistinctKeys )
				this->m_tUniq.Add ( { 0, i, iCount } );
		}	
	}

	/// add entry to the queue
	template <bool GROUPED>
	FORCE_INLINE bool PushEx ( const CSphMatch & tEntry )
	{
		if constexpr ( NOTIFICATIONS )
		{
			m_tJustPushed = RowTagged_t();
			m_dJustPopped.Resize(0);
		}

		if ( m_bDataInitialized )
		{
			assert ( m_tData.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if constexpr ( GROUPED )
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
			if constexpr ( HAS_AGGREGATES )
				UpdateAggregates ( tEntry, GROUPED, m_bMerge );
		}

		if constexpr ( DISTINCT )
			UpdateDistinct<GROUPED> ( tEntry );

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( m_bDataInitialized )
			return false;

		// add first
		m_pSchema->CloneMatch ( m_tData, tEntry );

		// first-time aggregate setup
		if constexpr ( HAS_AGGREGATES )
			SetupAggregates(tEntry);

		if constexpr ( NOTIFICATIONS )
			m_tJustPushed = RowTagged_t ( m_tData );

		if constexpr ( !GROUPED )
		{
			m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
			m_tData.SetAttr ( m_tLocCount, 1 );
			if constexpr ( DISTINCT )
				m_tData.SetAttr ( m_tLocDistinct, 0 );
		}
		else
		{
			if constexpr ( HAS_AGGREGATES )
				AggrUngroup ( m_tData );
		}

		m_bDataInitialized = true;
		++m_iTotal;
		return true;
	}

	/// count distinct values if necessary
	void CountDistinct ()
	{
		if constexpr ( !DISTINCT )
			return;

		assert ( m_bDataInitialized );
		m_tData.SetAttr ( m_tLocDistinct, m_tUniq.CountDistinct() );
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
			return true; // always return true, otherwise in RT indexes we won't be able to hit cutoff in disk chunks after the first one

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
class FastCountFilterSorter_c final : public FastBaseSorter_c
{
public:
	FastCountFilterSorter_c ( int iCount, const CSphGroupSorterSettings & tSettings )
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
			return true; // always return true, otherwise in RT indexes we won't be able to hit cutoff in disk chunks after the first one

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

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING INSTANTIATION
//////////////////////////////////////////////////////////////////////////

#define CREATE_SORTER_4TH(SORTER,COMPGROUP,UNIQ,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS,HAS_AGGREGATES) \
{ \
	int iMultiDistict = 0; \
	if ( tSettings.m_bDistinct ) \
	{ \
		assert(tSettings.m_pDistinctFetcher); \
		iMultiDistict = tSettings.m_pDistinctFetcher->IsMultiValue() ? 2 : 1; \
	} \
	BYTE uSelector = 4*iMultiDistict + 2*(bHasPackedFactors?1:0) + (HAS_AGGREGATES?1:0); \
	switch ( uSelector ) \
	{ \
	case 0:	return new SORTER<COMPGROUP,UNIQ,0,	false,false> ( pComp, pQuery, tSettings ); \
	case 1:	return new SORTER<COMPGROUP,UNIQ,0,	false,true>  ( pComp, pQuery, tSettings ); \
	case 2:	return new SORTER<COMPGROUP,UNIQ,0,	true, false> ( pComp, pQuery, tSettings ); \
	case 3:	return new SORTER<COMPGROUP,UNIQ,0,	true, true>  ( pComp, pQuery, tSettings ); \
	case 4:	return new SORTER<COMPGROUP,UNIQ,1, false,false> ( pComp, pQuery, tSettings ); \
	case 5:	return new SORTER<COMPGROUP,UNIQ,1, false,true>  ( pComp, pQuery, tSettings ); \
	case 6:	return new SORTER<COMPGROUP,UNIQ,1, true, false> ( pComp, pQuery, tSettings ); \
	case 7:	return new SORTER<COMPGROUP,UNIQ,1, true, true>  ( pComp, pQuery, tSettings ); \
	case 8:	return new SORTER<COMPGROUP,UNIQ,2, false,false> ( pComp, pQuery, tSettings ); \
	case 9:	return new SORTER<COMPGROUP,UNIQ,2, false,true>  ( pComp, pQuery, tSettings ); \
	case 10:return new SORTER<COMPGROUP,UNIQ,2, true, false> ( pComp, pQuery, tSettings ); \
	case 11:return new SORTER<COMPGROUP,UNIQ,2, true, true>  ( pComp, pQuery, tSettings ); \
	default: assert(0); return nullptr; \
	} \
}

template < typename COMPGROUP >
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const PrecalculatedSorterResults_t & tPrecalc )
{
	if ( tPrecalc.m_iCountDistinct!=-1 )
		return new FastCountDistinctSorter_c ( tPrecalc.m_iCountDistinct, tSettings );

	if ( tPrecalc.m_iCountFilter!=-1 )
		return new FastCountFilterSorter_c ( tPrecalc.m_iCountFilter, tSettings );

	if ( tPrecalc.m_iCount!=-1 )
		return new FastCountFilterSorter_c ( tPrecalc.m_iCount, tSettings );

	bool bUseHLL = tSettings.m_iDistinctAccuracy > 0;

	using Uniq_c			= UniqGrouped_T<ValueWithGroup_t>;
	using UniqSingle_c		= UniqSingle_T<SphAttr_t>;
	using UniqCount_c		= UniqGrouped_T<ValueWithGroupCount_t>;
	using UniqCountSingle_c = UniqSingle_T<ValueWithCount_t>;

	BYTE uSelector3rd = 32*( bUseHLL ? 1 : 0 ) + 16*( tSettings.m_bGrouped ? 1:0 ) + 8*( tSettings.m_bJson ? 1:0 ) + 4*( pQuery->m_iGroupbyLimit>1 ? 1:0 ) + 2*( tSettings.m_bImplicit ? 1:0 ) + ( ( tSettings.m_pGrouper && tSettings.m_pGrouper->IsMultiValue() ) ? 1:0 );
	switch ( uSelector3rd )
	{
	case 0:	CREATE_SORTER_4TH ( CSphKBufferGroupSorter,		COMPGROUP, Uniq_c,		pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 1: CREATE_SORTER_4TH ( MultiValueGroupSorter_T,	COMPGROUP, Uniq_c,		pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 2: CREATE_SORTER_4TH ( CSphImplicitGroupSorter,	COMPGROUP, UniqSingle_c, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 4:	CREATE_SORTER_4TH ( CSphKBufferNGroupSorter,	COMPGROUP, Uniq_c,		pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 5: CREATE_SORTER_4TH ( MultiValueNGroupSorter_T,	COMPGROUP, Uniq_c,		pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 8: CREATE_SORTER_4TH ( CSphKBufferJsonGroupSorter,	COMPGROUP, Uniq_c,		pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 16:CREATE_SORTER_4TH ( CSphKBufferGroupSorter,		COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 17:CREATE_SORTER_4TH ( MultiValueGroupSorter_T,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 18:CREATE_SORTER_4TH ( CSphImplicitGroupSorter,	COMPGROUP, UniqCountSingle_c, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 20:CREATE_SORTER_4TH ( CSphKBufferNGroupSorter,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 21:CREATE_SORTER_4TH ( MultiValueNGroupSorter_T,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 24:CREATE_SORTER_4TH ( CSphKBufferJsonGroupSorter,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 32:CREATE_SORTER_4TH ( CSphKBufferGroupSorter,		COMPGROUP, UniqHLL_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 33:CREATE_SORTER_4TH ( MultiValueGroupSorter_T,	COMPGROUP, UniqHLL_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 34:CREATE_SORTER_4TH ( CSphImplicitGroupSorter,	COMPGROUP, UniqHLLSingle_c, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 36:CREATE_SORTER_4TH ( CSphKBufferNGroupSorter,	COMPGROUP, UniqHLL_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 37:CREATE_SORTER_4TH ( MultiValueNGroupSorter_T,	COMPGROUP, UniqHLL_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 40:CREATE_SORTER_4TH ( CSphKBufferJsonGroupSorter,	COMPGROUP, UniqHLL_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 48:CREATE_SORTER_4TH ( CSphKBufferGroupSorter,		COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 49:CREATE_SORTER_4TH ( MultiValueGroupSorter_T,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 50:CREATE_SORTER_4TH ( CSphImplicitGroupSorter,	COMPGROUP, UniqCountSingle_c, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 52:CREATE_SORTER_4TH ( CSphKBufferNGroupSorter,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 53:CREATE_SORTER_4TH ( MultiValueNGroupSorter_T,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );
	case 56:CREATE_SORTER_4TH ( CSphKBufferJsonGroupSorter,	COMPGROUP, UniqCount_c,	pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates );

	default: assert(0); return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const PrecalculatedSorterResults_t & tPrecalc )
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


ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors, bool bHasAggregates, const PrecalculatedSorterResults_t & tPrecalc )
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

	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors, bHasAggregates, tPrecalc );
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

	bool bDisableCutoff = dSorters.any_of ( []( auto * pSorter ){ return pSorter->IsCutoffDisabled(); } );
	if ( bDisableCutoff )
		return -1;

	// implicit cutoff when there's no sorting and no grouping
	if ( !bFT && ( tQuery.m_sSortBy=="@weight desc" || tQuery.m_sSortBy.IsEmpty() ) && tQuery.m_sGroupBy.IsEmpty() && !tQuery.m_bFacet && !tQuery.m_bFacetHead )
		return tQuery.m_iLimit+tQuery.m_iOffset;

	return -1;
}
