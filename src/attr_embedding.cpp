//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdaemon.h"
#include "searchdsql.h"
#include "sphinx_alter.h"

#include "embeddingutils.h"
#include "attr_embedding.h"

void AddAttrToIndex ( const SqlStmt_t & tStmt, CSphIndex * pIdx, CSphString & sError, bool bModify )
{
	CSphString sAttrToAdd = tStmt.m_sAlterAttr;
	sAttrToAdd.ToLower();

	bool bIndexed = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED;
	bool bStored = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_STORED;
	bool bAttribute = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_IS_ATTRIBUTE; // beware, m.b. true only for strings

	auto pHasAttr = pIdx->GetMatchSchema ().GetAttr ( sAttrToAdd.cstr () );
	bool bHasField = pIdx->GetMatchSchema ().GetFieldIndex ( sAttrToAdd.cstr () )!=-1;
	const bool bInt2Bigint = ( pHasAttr && pHasAttr->m_eAttrType==SPH_ATTR_INTEGER && pHasAttr->m_eEngine==AttrEngine_e::DEFAULT && tStmt.m_eAlterColType==SPH_ATTR_BIGINT && tStmt.m_eEngine==AttrEngine_e::DEFAULT );

	if ( !bIndexed && pHasAttr )
	{
		if ( !bModify || !bInt2Bigint )
		{
			sError.SetSprintf ( "'%s' attribute already in schema", sAttrToAdd.cstr () );
			return;
		}
	}

	if ( bModify )
	{
		if ( !pHasAttr )
		{
			sError.SetSprintf ( "attribute '%s' does not exist", sAttrToAdd.cstr() );
			return;
		}
		if ( !bInt2Bigint )
		{
			sError.SetSprintf ( "attribute '%s': only alter from rowise int to bigint supported", sAttrToAdd.cstr () );
			return;
		}
	}

	if ( bIndexed && bHasField )
	{
		sError.SetSprintf ( "'%s' field already in schema", sAttrToAdd.cstr() );
		return;
	}

	if ( !bIndexed && bHasField && tStmt.m_eAlterColType!=SPH_ATTR_STRING )
	{
		sError.SetSprintf ( "cannot add attribute that shadows '%s' field", sAttrToAdd.cstr () );
		return;
	}

	if ( !CheckAlterAddEmbedding ( bModify, sAttrToAdd, tStmt, pIdx->GetMatchSchema(), sError ) )
		return;

	AttrAddRemoveCtx_t tCtx;
	tCtx.m_sName = sAttrToAdd;
	tCtx.m_eType = tStmt.m_eAlterColType;
	tCtx.m_iBits = tStmt.m_iBits;
	tCtx.m_uFlags = tStmt.m_uAttrFlags;
	tCtx.m_eEngine = tStmt.m_eEngine;
	tCtx.m_tKNN = tStmt.m_tAlterKNN;
	tCtx.m_tKNNModel = tStmt.m_tAlterKNNModel;
	tCtx.m_sKNNFrom = tStmt.m_sAlterKnnFrom;

	if ( bIndexed || bStored )
	{
		pIdx->AddRemoveField ( true, sAttrToAdd, tStmt.m_uFieldFlags, sError );
		if ( bAttribute )
			pIdx->AddRemoveAttribute ( true, tCtx, sError );

	} else
	{
		pIdx->AddRemoveAttribute ( true, tCtx, sError );
	}
}

static bool IsModelBackedEmbedding ( const SqlStmt_t & tStmt )
{
	return ( tStmt.m_eAlterColType==SPH_ATTR_FLOAT_VECTOR && !!( tStmt.m_uAttrFlags & CSphColumnInfo::ATTR_INDEXED_KNN ) && !tStmt.m_tAlterKNNModel.m_sModelName.empty() );
}

static bool RunEmbeddingPopulateLoop ( const CSphRefcountedPtr<const ServedIndex_c> & pServed, const CSphString & sAttr, EmbeddingPopulate_e eMode, CSphString & sWarning, CSphString & sError )
{
	sWarning = "";

	if ( !pServed )
	{
		sError = "table not found";
		return false;
	}

	ExtUpdState_t tState;
	AT_SCOPE_EXIT ( [&] ()
	{
		WIdx_c pRtFinish { pServed };
		if ( pRtFinish )
			pRtFinish->FinishUpdateEmbeddingState ( tState );
	});

	// init state \ save RAM segments into diskc chunk \ reserve spb files at all disk chunks
	{
		WIdx_c pRt { pServed };
		if ( !pRt->InitUpdateEmbeddingState ( sAttr, eMode, tState, sError ) )
			return false;

		if ( !pRt->ReserveEmbeddingSpace ( 0, tState.m_iDims, sError ) )
			return false;
	}

	if ( sphInterrupted() )
	{
		sError = "operation interrupted";
		return false;
	}

	StrVec_t dUpdateWarnings;
	for ( ;; )
	{
		AttrUpdateSharedPtr_t pUpdate;
		if ( !RIdx_c ( pServed )->GetUpdateEmbedding ( tState, pUpdate, sError ) )
			return false;

		if ( !pUpdate )
			break;

		if ( sphInterrupted() )
		{
			sError = "operation interrupted";
			return false;
		}

		AttrUpdateInc_t tUpd ( pUpdate );
		WIdx_c pRt { pServed };
		if ( eMode==EmbeddingPopulate_e::Empty )
		{
			if ( !pRt->ValidateUpdateEmbedding ( tState, tUpd, sError ) )
				return false;
		}

		bool bCritical = false;
		CSphString sUpdateWarning;
		const int iRes = pRt->CheckThenUpdateAttributes ( tUpd, bCritical, sError, sUpdateWarning );
		if ( iRes<0 )
			return false;
		if ( !sUpdateWarning.IsEmpty() )
			dUpdateWarnings.Add ( sUpdateWarning );

		tState.m_iUpdatedRows += iRes;
		tState.m_iSkippedRows += Max ( 0, pUpdate->m_dDocids.GetLength()-iRes );
	}

	if ( tState.m_iUpdatedRows>0 && !WIdx_c ( pServed )->AlterKNN ( sError ) )
		return false;

	if ( !dUpdateWarnings.IsEmpty() )
	{
		dUpdateWarnings.Uniq();
		sWarning = ConcatWarnings ( dUpdateWarnings );
	}

	return true;
}

bool HandleAlterAddColumn ( const SqlStmt_t & tStmt, const CSphRefcountedPtr<const ServedIndex_c> & pServed, CSphString & sWarning, CSphString & sError )
{
	sWarning = "";

	if ( !pServed )
	{
		sError = "internal error: table descriptor is not available";
		return false;
	}

	if ( IsModelBackedEmbedding ( tStmt ) )
	{
		AddAttrToIndex ( tStmt, WIdx_c ( pServed ), sError, false );
		if ( !sError.IsEmpty() )
			return false;

		CSphString sAttrLower = tStmt.m_sAlterAttr;
		sAttrLower.ToLower();
		return RunEmbeddingPopulateLoop ( pServed, sAttrLower, EmbeddingPopulate_e::Empty, sWarning, sError );
	}

	AddAttrToIndex ( tStmt, WIdx_c ( pServed ), sError, false );
	return sError.IsEmpty();
}

bool HandleAlterRebuildEmbeddings ( const CSphString & sAttr, const CSphRefcountedPtr<const ServedIndex_c> & pServed, CSphString & sWarning, CSphString & sError )
{
	CSphString sAttrLower = sAttr;
	sAttrLower.ToLower();
	return RunEmbeddingPopulateLoop ( pServed, sAttrLower, EmbeddingPopulate_e::AllRows, sWarning, sError );
}
