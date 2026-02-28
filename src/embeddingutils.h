//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _embeddingutils_
#define _embeddingutils_

#include "sphinx.h"
#include "attribute.h"
#include "columnarmisc.h"

template <typename TVec>
FORCE_INLINE void AddAllEmbeddingSources ( TVec & dFrom, const ISphSchema & tSchema )
{
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
		dFrom.Add ( { i, true } );
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
		if ( tSchema.GetAttr(i).m_eAttrType == SPH_ATTR_STRING )
			dFrom.Add ( { i, false } );
}

template <typename TVec>
FORCE_INLINE bool ParseEmbeddingFromSetting ( TVec & dFrom, const CSphString & sFrom, const ISphSchema & tSchema, CSphString & sError )
{
	CSphString sTrimmed = sFrom;
	sTrimmed.Trim();
	if ( sTrimmed.IsEmpty() )
	{
		AddAllEmbeddingSources ( dFrom, tSchema );
		return true;
	}

	bool bOk = true;
	sphSplitApply ( sFrom.cstr(), sFrom.Length(), " \t,", [ & ] ( const char * sToken, int iLen )
	{
		if ( !bOk )
			return;

		CSphString sName;
		sName.SetBinary ( sToken, iLen );
		int iAttrId = tSchema.GetAttrIndex ( sName.cstr() );
		int iFieldId = tSchema.GetFieldIndex ( sName.cstr() );

		if ( iFieldId==-1 && iAttrId==-1 )
		{
			sError.SetSprintf ( "embedding source '%s' not found", sName.cstr() );
			bOk = false;
			return;
		}

		if ( iAttrId!=-1 && tSchema.GetAttr(iAttrId).m_eAttrType!=SPH_ATTR_STRING )
		{
			sError.SetSprintf ( "embedding source attribute '%s' is not a string", sName.cstr() );
			bOk = false;
			return;
		}

		dFrom.Add ( { iFieldId==-1 ? iAttrId : iFieldId, iFieldId!=-1 } );
	} );

	return bOk;
}

template <typename TVecAttrs>
FORCE_INLINE bool CollectRebuildEmbeddingAttrs ( const ISphSchema & tSchema, const TVecAttrs & dAttrsWithModels, CSphVector<int> & dRebuildAttrs, CSphString & sError, CSphVector<CSphString> * pSkippedColumnarNames = nullptr )
{
	int iAttrCount = Min ( tSchema.GetAttrsCount(), dAttrsWithModels.GetLength() );
	for ( int i = 0; i < iAttrCount; i++ )
	{
		const auto & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType != SPH_ATTR_FLOAT_VECTOR || !dAttrsWithModels[i].m_pModel || dAttrsWithModels[i].m_dFrom.IsEmpty() )
			continue;
		if ( tAttr.IsColumnar() )
		{
			if ( pSkippedColumnarNames )
				pSkippedColumnarNames->Add ( tAttr.m_sName );
			continue;
		}
		dRebuildAttrs.Add(i);
	}

	return true;
}

template <typename PMODEL>
FORCE_INLINE bool ConvertEmbeddingsChecked ( PMODEL pModel, const CSphString & sAttrName, const CSphVector<CSphString> & dFromTexts, std::vector<std::vector<float>> & dEmbeddings, CSphString & sError )
{
	std::vector<std::string_view> dTexts;
	dTexts.reserve ( dFromTexts.GetLength() );
	for ( const auto & s : dFromTexts )
		dTexts.push_back ( { s.cstr(), (size_t)s.Length() } );

	std::string sErrStl;
	if ( !pModel->Convert ( dTexts, dEmbeddings, sErrStl ) )
	{
		sError = sErrStl.c_str();
		return false;
	}

	if ( dEmbeddings.size() != dTexts.size() )
	{
		sError.SetSprintf ( "Error rebuilding embeddings for attribute '%s': model returned %d vectors for %d input texts",
			sAttrName.cstr(), (int)dEmbeddings.size(), (int)dTexts.size() );
		return false;
	}

	return true;
}

FORCE_INLINE bool EmbeddingFromNeedsDocstore ( const VecTraits_T<std::pair<int,bool>> & dFrom )
{
	for ( const auto & tFromEntry : dFrom )
		if ( tFromEntry.second )
			return true;

	return false;
}

FORCE_INLINE void PrepareEmbeddingColumnarIters ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, columnar::Columnar_i * pColumnar, CSphVector<ScopedTypedIterator_t> & dIters )
{
	dIters.Resize ( dFrom.GetLength() );
	if ( !pColumnar )
		return;

	for ( int iFrom = 0; iFrom < dFrom.GetLength(); iFrom++ )
	{
		const auto & tFromEntry = dFrom[iFrom];
		if ( tFromEntry.second )
			continue;

		const CSphColumnInfo & tFromAttr = tSchema.GetAttr ( tFromEntry.first );
		if ( !tFromAttr.IsColumnar() )
			continue;

		std::string sIterError;
		dIters[iFrom].first = CreateColumnarIterator ( pColumnar, tFromAttr.m_sName.cstr(), sIterError );
	}
}

FORCE_INLINE void BuildEmbeddingFromText ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, const DocstoreDoc_t * pDoc, const BYTE * pBlobRow, CSphVector<ScopedTypedIterator_t> & dColumnarIters, RowID_t tRowID, CSphString & sFrom )
{
	StringBuilder_c sFromBuilder;
	for ( int iFrom = 0; iFrom < dFrom.GetLength(); iFrom++ )
	{
		const auto & tFromEntry = dFrom[iFrom];
		if ( tFromEntry.second )
		{
			if ( !pDoc )
				continue;

			int iFieldId = tFromEntry.first;
			if ( iFieldId >= 0 && iFieldId < pDoc->m_dFields.GetLength() )
			{
				const auto & dF = pDoc->m_dFields[iFieldId];
				if ( dF.GetLength() )
					sFromBuilder.Appendf ( "%.*s ", (int)dF.GetLength(), (const char *)dF.Begin() );
			}
			continue;
		}

		const CSphColumnInfo & tFromAttr = tSchema.GetAttr ( tFromEntry.first );
		if ( tFromAttr.IsColumnar() )
		{
			auto & pColIt = dColumnarIters[iFrom].first;
			if ( pColIt )
			{
				const BYTE * pVal = nullptr;
				int iBytes = pColIt->Get ( tRowID, pVal );
				if ( iBytes > 0 && pVal )
					sFromBuilder.Appendf ( "%.*s ", iBytes, (const char *)pVal );
			}
			continue;
		}

		ByteBlob_t tStrBlob = sphGetBlobAttr ( pBlobRow, tFromAttr.m_tLocator );
		if ( tStrBlob.second > 0 && tStrBlob.first )
			sFromBuilder.Appendf ( "%.*s ", tStrBlob.second, (const char *)tStrBlob.first );
	}

	sFromBuilder.MoveTo ( sFrom );
	sFrom.Trim();
}

#endif
