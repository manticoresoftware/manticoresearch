//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
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

const char *					GetKnnDistAttrName();
ISphExpr *						CreateExpr_KNNDist ( const CSphVector<float> & dAnchor, const CSphColumnInfo & tAttr );
void							NormalizeVec ( VecTraits_T<float> & dData );

void							operator << ( JsonEscapedBuilder & tOut, const knn::IndexSettings_t & tSettings );
void							AddKNNSettings ( StringBuilder_c & sRes, const CSphColumnInfo & tAttr );
knn::IndexSettings_t			ReadKNNJson ( bson::Bson_c tRoot );
CSphString						FormatKNNConfigStr ( const CSphVector<NamedKNNSettings_t> & dAttrs );
bool							ParseKNNConfigStr ( const CSphString & sStr, CSphVector<NamedKNNSettings_t> & dParsed, CSphString & sError );

bool							Str2HNSWSimilarity ( const CSphString & sSimilarity, knn::HNSWSimilarity_e & eSimilarity, CSphString * pError = nullptr );
bool							Str2Quantization ( const CSphString & sQuantization, knn::Quantization_e & eQuantization, CSphString * pError = nullptr );

std::unique_ptr<knn::Builder_i>	BuildCreateKNN ( const ISphSchema & tSchema, int64_t iNumElements, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError );
void							BuildTrainKNN ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder );
bool							BuildStoreKNN ( RowID_t tRowID, const CSphRowitem * pRow, const BYTE * pPool, CSphVector<ScopedTypedIterator_t> & dIterators, const CSphVector<PlainOrColumnar_t> & dAttrs, knn::Builder_i & tBuilder );
std::pair<RowidIterator_i *, bool> CreateKNNIterator ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, CSphString & sError );
RowIteratorsWithEstimates_t		CreateKNNIterators ( knn::KNN_i * pKNN, const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, bool & bError, CSphString & sError );
