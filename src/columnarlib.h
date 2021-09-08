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

#include "columnar.h"
#include "builder.h"

class ISphSchema;

columnar::Columnar_i *	CreateColumnarStorageReader ( const CSphString & sFile, DWORD uNumDocs, CSphString & sError );
columnar::Builder_i *	CreateColumnarBuilder ( const ISphSchema & tSchema, const columnar::Settings_t & tSettings, const CSphString & sFilename, CSphString & sError );
void					CheckColumnarStorage ( const CSphString & sFile, DWORD uNumRows, std::function<void (const char*)> fnError, std::function<void (const char*)> fnProgress );

bool			InitColumnar ( CSphString & sError );
void			ShutdownColumnar();
const char *	GetColumnarVersionStr();
int				GetColumnarStorageVersion();
bool			IsColumnarLibLoaded();

#endif // _columnarlib_
