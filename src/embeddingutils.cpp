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

#include <string_view>

#include "sphinxint.h"
#include "searchdsql.h"
#include "columnarmisc.h"
#include "embeddingutils.h"

bool ValidateSettingModel ( const CreateTableAttr_t & tAttr, CSphString & sError )
{
	if ( !tAttr.m_bKNN || tAttr.m_tKNNModel.m_sModelName.empty() )
		return true;

	if ( tAttr.m_bKNNFromSet )
		return true;

	sError.SetSprintf ( "'from' setting empty for KNN attribute '%s'", tAttr.m_tAttr.m_sName.cstr() );
	return false;
}


static void ParseEmbeddingSourcesAll ( CSphVector<std::pair<int,bool>> & dFrom, const ISphSchema & tSchema )
{
	for ( int i=0; i < tSchema.GetFieldsCount(); ++i )
		dFrom.Add ( { i, true } );

	for ( int i=0; i < tSchema.GetAttrsCount(); ++i )
	{
		const auto & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType!=SPH_ATTR_STRING )
			continue;

		if ( tSchema.GetFieldIndex ( tAttr.m_sName.cstr() )!=-1 )
			continue;

		dFrom.Add ( { i, false } );
	}
}


bool ParseEmbeddingSources ( CSphVector<std::pair<int,bool>> & dFrom, const CSphString & sFrom, const ISphSchema & tSchema, CSphString & sError )
{
	dFrom.Reset();

	if ( sFrom.IsEmpty() )
	{
		ParseEmbeddingSourcesAll ( dFrom, tSchema );
		return true;
	}

	StrVec_t dSrc;
	sphSplit ( dSrc, sFrom.cstr(), " \t," );

	for ( const auto & i : dSrc )
	{
		int iAttrId = tSchema.GetAttrIndex ( i.cstr() );
		int iFieldId = tSchema.GetFieldIndex ( i.cstr() );

		if ( iFieldId==-1 && iAttrId==-1 )
		{
			sError.SetSprintf ( "embedding source '%s' not found", i.cstr() );
			return false;
		}

		if ( iAttrId!=-1 && tSchema.GetAttr(iAttrId).m_eAttrType!=SPH_ATTR_STRING )
		{
			sError.SetSprintf ( "embedding source attribute '%s' is not a string", i.cstr() );
			return false;
		}

		dFrom.Add ( { iFieldId==-1 ? iAttrId : iFieldId, iFieldId!=-1 } );
	}

	return true;
}


bool CheckAlterAddEmbedding ( bool bModify, const CSphString & sAttrToAdd, const SqlStmt_t & tStmt, const CSphSchema & tBaseSchema, CSphString & sError )
{
	if ( bModify || tStmt.m_eAlterColType!=SPH_ATTR_FLOAT_VECTOR || !( tStmt.m_uAttrFlags & CSphColumnInfo::ATTR_INDEXED_KNN ) )
		return true;

	CreateTableAttr_t tCreateAttr;
	tCreateAttr.m_tAttr.m_sName		= sAttrToAdd;
	tCreateAttr.m_tAttr.m_eAttrType	= tStmt.m_eAlterColType;
	tCreateAttr.m_bKNN				= true;
	tCreateAttr.m_bKNNFromSet		= tStmt.m_bAlterKnnFromSet;
	tCreateAttr.m_tKNN				= tStmt.m_tAlterKNN;
	tCreateAttr.m_tKNNModel			= tStmt.m_tAlterKNNModel;
	tCreateAttr.m_sKNNFrom			= tStmt.m_sAlterKnnFrom;
	if ( !ValidateSettingModel ( tCreateAttr, sError ) )
		return false;

	if ( tStmt.m_tAlterKNNModel.m_sModelName.empty() )
		return true;

	CSphSchema tProjectedSchema = tBaseSchema;
	CSphColumnInfo tNewAttr ( sAttrToAdd.cstr(), tStmt.m_eAlterColType );
	tNewAttr.m_tKNN = tStmt.m_tAlterKNN;
	tNewAttr.m_tKNNModel = tStmt.m_tAlterKNNModel;
	tNewAttr.m_sKNNFrom = tStmt.m_sAlterKnnFrom;
	tProjectedSchema.AddAttr ( tNewAttr, false );

	CSphVector<std::pair<int,bool>> dFrom;
	return ParseEmbeddingSources ( dFrom, tStmt.m_sAlterKnnFrom, tProjectedSchema, sError );
}

bool ConvertEmbeddings ( knn::TextToEmbeddings_i * pModel, const CSphString & sAttrName, const CSphVector<CSphString> & dFromTexts, std::vector<std::vector<float>> & dEmbeddings, CSphString & sError )
{
	std::vector<std::string_view> dTexts;
	dTexts.reserve ( dFromTexts.GetLength() );
	for ( const auto & sText : dFromTexts )
		dTexts.push_back ( { sText.cstr(), (size_t)sText.Length() } );

	std::string sErrStl;
	if ( !pModel->Convert ( dTexts, dEmbeddings, sErrStl ) )
	{
		sError = sErrStl.c_str();
		return false;
	}

	if ( dEmbeddings.size()!=dTexts.size() )
	{
		sError.SetSprintf ( "error generating embeddings for attribute '%s': model returned %d vectors for %d input texts",
			sAttrName.cstr(), (int)dEmbeddings.size(), (int)dTexts.size() );
		return false;
	}

	return true;
}

AttrUpdateSharedPtr_t CreateFloatVectorAttrUpdate ( const CSphString & sAttrName, const CSphVector<DocID_t> & dDocids, const std::vector<std::vector<float>> & dEmbeddings, int iEmptyVectorDims )
{
	assert ( dEmbeddings.size()==(size_t)dDocids.GetLength() );

	AttrUpdateSharedPtr_t pUpdate { new CSphAttrUpdate };
	TypedAttribute_t & tTyped = pUpdate->m_dAttributes.Add();
	tTyped.m_sName = sAttrName;
	tTyped.m_eType = SPH_ATTR_FLOAT_VECTOR;

	pUpdate->m_dDocids = dDocids;
	const int iRows = dDocids.GetLength();
	pUpdate->m_dRowOffset.Resize ( iRows );
	int iPoolOffset = 0;

	for ( int i = 0; i < iRows; i++ )
	{
		pUpdate->m_dRowOffset[i] = iPoolOffset;
		const auto & dVec = dEmbeddings[i];
		const int iCount = dVec.empty() ? iEmptyVectorDims : (int)dVec.size();

		// Keep parser-compatible float_vector UPDATE pool layout:
		// [uLength=2*count][int64(value_0)...], where each int64 stores float bits in low DWORD.
		pUpdate->m_dPool.Add ( (DWORD)( iCount * 2 ) );
		if ( dVec.empty() )
		{
			for ( int j = 0; j < iEmptyVectorDims; j++ )
				*(int64_t*)pUpdate->m_dPool.AddN(2) = sphF2DW ( 0.0f );
		}
		else
		{
			for ( float fVal : dVec )
				*(int64_t*)pUpdate->m_dPool.AddN(2) = sphF2DW ( fVal );
		}

		iPoolOffset += 1 + iCount * 2;
	}

	pUpdate->m_bIgnoreNonexistent = false;
	pUpdate->m_bStrict = true;
	pUpdate->m_bRebuildEmbeddings = true;
	return pUpdate;
}

bool EmbeddingFromNeedsDocstore ( const VecTraits_T<std::pair<int,bool>> & dFrom )
{
	return dFrom.any_of ( [] ( const auto & tFrom ) { return tFrom.second; } );
}

void GetEmbeddingColumnar ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, columnar::Columnar_i * pColumnar, CSphVector<ScopedTypedIterator_t> & dIters )
{
	dIters.Resize ( dFrom.GetLength() );
	if ( !pColumnar )
		return;

	for ( int iFrom = 0; iFrom < dFrom.GetLength(); iFrom++ )
	{
		const auto & tFrom = dFrom[iFrom];
		if ( tFrom.second )
			continue;

		const CSphColumnInfo & tFromAttr = tSchema.GetAttr ( tFrom.first );
		if ( !tFromAttr.IsColumnar() )
			continue;

		std::string sIterError;
		dIters[iFrom].first = CreateColumnarIterator ( pColumnar, tFromAttr.m_sName.cstr(), sIterError );
	}
}

void BuildEmbeddingFromText ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, const DocstoreDoc_t * pDoc, const BYTE * pBlobRow, CSphVector<ScopedTypedIterator_t> & dColumnarIters, RowID_t tRowID, CSphString & sFrom )
{
	StringBuilder_c sBuilder ( " " );
	for ( int iFrom = 0; iFrom < dFrom.GetLength(); iFrom++ )
	{
		const auto & tFrom = dFrom[iFrom];
		if ( tFrom.second )
		{
			if ( !pDoc )
				continue;

			const int iFieldId = tFrom.first;
			if ( iFieldId >= 0 && iFieldId < pDoc->m_dFields.GetLength() )
			{
				const auto & dField = pDoc->m_dFields[iFieldId];
				if ( dField.GetLength() )
					sBuilder.Appendf ( "%.*s", (int)dField.GetLength(), (const char *)dField.Begin() );
			}
			continue;
		}

		const CSphColumnInfo & tFromAttr = tSchema.GetAttr ( tFrom.first );
		if ( tFromAttr.IsColumnar() )
		{
			auto & pColIt = dColumnarIters[iFrom].first;
			if ( pColIt )
			{
				const BYTE * pVal = nullptr;
				int iBytes = pColIt->Get ( tRowID, pVal );
				if ( iBytes > 0 && pVal )
					sBuilder.Appendf ( "%.*s", iBytes, (const char *)pVal );
			}
			continue;
		}

		ByteBlob_t tStrBlob = sphGetBlobAttr ( pBlobRow, tFromAttr.m_tLocator );
		if ( tStrBlob.first && tStrBlob.second > 0 )
			sBuilder.Appendf ( "%.*s", tStrBlob.second, (const char *)tStrBlob.first );
	}

	sBuilder.MoveTo ( sFrom );
}
