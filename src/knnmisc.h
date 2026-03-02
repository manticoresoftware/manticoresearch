//
// Copyright (c) 2023-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "sphinxdefs.h"
#include "columnarmisc.h"
#include "indexsettings.h"
#include "secondaryindex.h"

class CSphQueryContext;
class MemoryWriter_c;
class MemoryReader_c;

class TableEmbeddings_c
{
public:
	bool	Load ( const CSphString & sAttr, const knn::ModelSettings_t & tSettings, CSphString & sError );
	knn::TextToEmbeddings_i * GetModel ( const CSphString & sAttr ) const;

private:
	SmallStringHash_T<std::unique_ptr<knn::TextToEmbeddings_i>> m_hModels;
};

class EmbeddingsSrc_c
{
public:
			EmbeddingsSrc_c ( int iAttrs );

	void	Add ( int iAttr, CSphVector<char> & dSrc );
	void	Remove ( const CSphFixedVector<RowID_t> & dRowMap );
	const VecTraits_T<char> Get ( RowID_t tRowID, int iAttr ) const;
	bool	Has ( RowID_t tRowID, int iAttr ) const;
	void	Save ( MemoryWriter_c & tWriter ) const;
	void	Load ( MemoryReader_c & tReader );

private:
	CSphVector<CSphVector<CSphVector<char>>> m_dStored;
};

bool							IsKnnDist ( const CSphString & sExpr );
const char *					GetKnnDistAttrName();
const char *					GetKnnDistRescoreAttrName();
void							SetupKNNLimit ( CSphQuery & tQuery );

ISphExpr *						CreateExpr_KNNDist ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr );
ISphExpr *						CreateExpr_KNNDistRescore ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr );
void							NormalizeVec ( VecTraits_T<float> & dData );

void							AddKNNSettings ( StringBuilder_c & sRes, const CSphColumnInfo & tAttr );
void							ReadKNNJson ( bson::Bson_c tRoot, knn::IndexSettings_t & tIS, knn::ModelSettings_t & tMS, CSphString & sKNNFrom );
CSphString						FormatKNNConfigStr ( const CSphVector<NamedKNNSettings_t> & dAttrs );
bool							ParseKNNConfigStr ( const CSphString & sStr, CSphVector<NamedKNNSettings_t> & dParsed, CSphString & sError );
void							FormatKNNSettings ( JsonEscapedBuilder & tOut, const knn::IndexSettings_t & tIndexSettings, const knn::ModelSettings_t & tModelSettings, const CSphString & sKNNFrom );

bool							Str2HNSWSimilarity ( const CSphString & sSimilarity, knn::HNSWSimilarity_e & eSimilarity, CSphString * pError = nullptr );
bool							Str2Quantization ( const CSphString & sQuantization, knn::Quantization_e & eQuantization, CSphString * pError = nullptr );

std::unique_ptr<knn::Builder_i> BuildCreateKNN ( const ISphSchema & tSchema, int64_t iNumElements, CSphVector<std::pair<PlainOrColumnar_t,int>> & dAttrs, const CSphString & sTmpFilename, CSphString & sError );
void							BuildTrainKNN ( RowID_t tRowIDSrc, RowID_t tRowIDDst, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const VecTraits_T<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder );
bool							BuildStoreKNN ( RowID_t tRowIDSrc, RowID_t tRowIDDst, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const VecTraits_T<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder );
std::pair<RowidIterator_i *, bool> CreateKNNIterator ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, knn::KNNFilter_i * pFilter, CSphString & sError );
RowIteratorsWithEstimates_t		CreateKNNIterators ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, knn::KNNFilter_i * pFilter, bool & bError, CSphString & sError );
std::unique_ptr<knn::KNNFilter_i> CreateKNNPrefilter ( const CSphQueryContext & tCtx, const CSphRowitem * pAttrPool, int iStride, int iDynamicSize, int64_t iFilterCount );

ISphMatchSorter *				CreateKNNRescoreSorter ( ISphMatchSorter * pSorter, const KnnSearchSettings_t & tSettings );
