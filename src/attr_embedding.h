//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _attr_embedding_
#define _attr_embedding_

#include "sphinxstd.h"

class CSphIndex;
class ServedIndex_c;
template <typename T> class CSphRefcountedPtr;
struct SqlStmt_t;

void AddAttrToIndex ( const SqlStmt_t & tStmt, CSphIndex * pIdx, CSphString & sError, bool bModify );
bool HandleAlterAddColumn ( const SqlStmt_t & tStmt, const CSphRefcountedPtr<const ServedIndex_c> & pServed, CSphString & sWarning, CSphString & sError );
bool HandleAlterRebuildEmbeddings ( const CSphString & sAttr, const CSphRefcountedPtr<const ServedIndex_c> & pServed, CSphString & sWarning, CSphString & sError );

#endif // _attr_embedding_
