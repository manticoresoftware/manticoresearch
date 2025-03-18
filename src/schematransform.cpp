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

#include "schematransform.h"

#include "queuecreator.h"
#include "sphinxjson.h"


template <typename FN>
static void FnSortGetStringRemap ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema, FN fnProcess )
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


int GetStringRemapCount ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema )
{
	int iMaps = 0;
	FnSortGetStringRemap ( tDstSchema, tSrcSchema, [&iMaps] ( int, int ) { ++iMaps; } );
	return iMaps;
}


static BYTE * RepackNullMaskStr ( const BYTE * pMask, int iNumDstAttrs, const IntVec_t & dNullRemap )
{
	ByteBlob_t dUnpacked = sphUnpackPtrAttr(pMask);
	int iNumElements = dUnpacked.second*8;
	BitVec_T<BYTE> tSrcMask ( (BYTE*)dUnpacked.first, iNumElements );
	BitVec_T<BYTE> tDstMask ( iNumDstAttrs );
	for ( int i = 0; i < iNumElements; i++ )
		if ( tSrcMask.BitGet(i) && dNullRemap[i]!=-1 )
			tDstMask.BitSet ( dNullRemap[i] );

	return sphPackPtrAttr ( { tDstMask.Begin(), tDstMask.GetSizeBytes() } );
}


static SphAttr_t RepackNullMaskInt ( SphAttr_t uMask, const IntVec_t & dNullRemap )
{
	SphAttr_t uValue = 0;
	for ( int i = 0; i < dNullRemap.GetLength(); i++ )
		if ( ( uMask & ( 1ULL << i ) ) && dNullRemap[i]!=-1 )
			uValue |= 1ULL << dNullRemap[i];

	return uValue;
}


static IntVec_t SetupNullMaskRemap ( const ISphSchema & tOldSchema, const ISphSchema & tNewSchema )
{
	IntVec_t dNullRemap;
	if ( !tOldSchema.GetAttr ( GetNullMaskAttrName() ) )
		return dNullRemap;

	for ( int i = 0; i < tOldSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tOldAttr = tOldSchema.GetAttr(i);
		if ( tOldAttr.m_tLocator.m_bDynamic )
			dNullRemap.Add ( tNewSchema.GetAttrIndex ( tOldAttr.m_sName.cstr() ) );
	}

	return dNullRemap;
}


void RemapNullMask ( VecTraits_T<CSphMatch> & dMatches, const CSphSchema & tOldSchema, CSphSchema & tNewSchema )
{
	const CSphColumnInfo * pOld = tOldSchema.GetAttr ( GetNullMaskAttrName() );
	if ( !pOld )
		return;

	const CSphColumnInfo * pNew = tNewSchema.GetAttr ( GetNullMaskAttrName() );
	assert(pNew);

	IntVec_t dNullRemap = SetupNullMaskRemap ( tOldSchema, tNewSchema );

	if ( ( pOld->m_eAttrType==SPH_ATTR_INTEGER || pOld->m_eAttrType==SPH_ATTR_BIGINT ) && ( pNew->m_eAttrType==SPH_ATTR_INTEGER || pNew->m_eAttrType==SPH_ATTR_BIGINT ) )
	{
		for ( auto & i : dMatches )
			i.SetAttr ( pNew->m_tLocator, RepackNullMaskInt ( i.GetAttr ( pOld->m_tLocator ), dNullRemap ) );
	}
	else
	{
		assert ( pOld->m_eAttrType==SPH_ATTR_STRINGPTR && pNew->m_eAttrType==SPH_ATTR_STRINGPTR );

		for ( auto & i : dMatches )
		{
			BYTE * pOldMask = (BYTE *)i.GetAttr ( pOld->m_tLocator );
			BYTE * pNewMask = RepackNullMaskStr ( pOldMask, tNewSchema.GetAttrsCount(), dNullRemap );
			SafeDeleteArray(pOldMask);
			i.SetAttr ( pNew->m_tLocator, (SphAttr_t)pNewMask );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

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

	[[maybe_unused]] const CSphColumnInfo * pNew = m_tNewSchema.GetAttr ( GetNullMaskAttrName() );
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

	m_dNullRemap = SetupNullMaskRemap ( *pOldSchema, *pNewSchema );
	m_iNumDstAttrs = pNewSchema->GetAttrsCount();
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
		uValue = (SphAttr_t)RepackNullMaskStr ( (const BYTE*)pMatch->GetAttr ( *tAction.m_pFrom ), m_iNumDstAttrs, m_dNullRemap );
		break;

	default:
		assert(false && "Unknown state");
	}

	tResult.SetAttr ( *tAction.m_pTo, uValue );
}

///////////////////////////////////////////////////////////////////////////////

MatchProcessor_i * CreateMatchSchemaTransform ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, GetBlobPoolFromMatch_fn fnGetBlobPool, GetColumnarFromMatch_fn fnGetColumnar )
{
	return new MatchesToNewSchema_c ( pOldSchema, pNewSchema, fnGetBlobPool, fnGetColumnar );
}
