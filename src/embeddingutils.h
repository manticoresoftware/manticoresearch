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

#ifndef _embeddingutils_
#define _embeddingutils_

#include <vector>

class ISphSchema;
class CSphSchema;
struct SqlStmt_t;
namespace knn { class TextToEmbeddings_i; }

bool ValidateSettingModel ( const CreateTableAttr_t & tAttr, CSphString & sError );
bool ParseEmbeddingSources ( CSphVector<std::pair<int,bool>> & dFrom, const CSphString & sFrom, const ISphSchema & tSchema, CSphString & sError );
bool CheckAlterAddEmbedding ( bool bModify, const CSphString & sAttrToAdd, const SqlStmt_t & tStmt, const CSphSchema & tBaseSchema, CSphString & sError );
bool ConvertEmbeddings ( knn::TextToEmbeddings_i * pModel, const CSphString & sAttrName, const CSphVector<CSphString> & dFromTexts, std::vector<std::vector<float>> & dEmbeddings, CSphString & sError );
AttrUpdateSharedPtr_t CreateFloatVectorAttrUpdate ( const CSphString & sAttrName, const CSphVector<DocID_t> & dDocids, const std::vector<std::vector<float>> & dEmbeddings, int iEmptyVectorDims );
bool EmbeddingFromNeedsDocstore ( const VecTraits_T<std::pair<int,bool>> & dFrom );
void GetEmbeddingColumnar ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, columnar::Columnar_i * pColumnar, CSphVector<ScopedTypedIterator_t> & dIters );
void BuildEmbeddingFromText ( const ISphSchema & tSchema, const VecTraits_T<std::pair<int,bool>> & dFrom, const DocstoreDoc_t * pDoc, const BYTE * pBlobRow, CSphVector<ScopedTypedIterator_t> & dColumnarIters, RowID_t tRowID, CSphString & sFrom );

#endif // _embeddingutils_
