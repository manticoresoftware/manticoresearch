//
// Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarlib_
#define _columnarlib_

#include "sphinxexpr.h"

#include "columnar/columnar.h"
#include "columnar/builder.h"
#include "common/schema.h"

class ISphSchema;

std::unique_ptr<columnar::Columnar_i>	CreateColumnarStorageReader ( const CSphString & sFile, DWORD uNumDocs, CSphString & sError );
std::unique_ptr<columnar::Builder_i>	CreateColumnarBuilder ( const ISphSchema & tSchema, const CSphString & sFilename, size_t tBufferSize, CSphString & sError );
void					CheckColumnarStorage ( const CSphString & sFile, DWORD uNumRows, std::function<void (const char*)> fnError, std::function<void (const char*)> fnProgress );

common::AttrType_e	ToColumnarType ( ESphAttr eAttrType, int iBitCount );

bool			InitColumnar ( CSphString & sError );
void			ShutdownColumnar();
const char *	GetColumnarVersionStr();
bool			IsColumnarLibLoaded();

// params rearranged for most frequently used defaults
std::unique_ptr<columnar::Iterator_i> CreateColumnarIterator ( const columnar::Columnar_i * pColumnar, const std::string&  sName, std::string & sError, const columnar::IteratorHints_t & tHints={}, columnar::IteratorCapabilities_t * pCapabilities=nullptr );

#endif // _columnarlib_
