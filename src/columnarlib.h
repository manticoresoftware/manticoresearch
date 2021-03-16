//
// Copyright (c) 2020-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarlib_
#define _columnarlib_

#include "sphinxstd.h"

#if USE_COLUMNAR

#include "columnar.h"
#include "builder.h"

class ISphSchema;

columnar::Columnar_i *	CreateColumnarStorageReader ( const CSphString & sFile, DWORD uNumDocs, CSphString & sError );
columnar::Builder_i *	CreateColumnarBuilder ( const ISphSchema & tSchema, const columnar::Settings_t & tSettings, const CSphString & sFilename, CSphString & sError );

#endif // USE_COLUMNAR

bool			InitColumnar ( CSphString & sError );
void			ShutdownColumnar();
const char *	GetColumnarVersionStr();

#endif // _columnarlib_