//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinx.h"

class JsonSIBuilder_i
{
public:
	virtual			~JsonSIBuilder_i() = default;

	virtual void	AddRowSize ( SphOffset_t tSize ) = 0;
	virtual bool	Done ( CSphString & sError ) = 0;
};


bool BuildJsonSI ( const CSphString & sAttribute, const CSphRowitem * pPool, int64_t iNumRows, const ISphSchema & tSchema, const BYTE * pBlobPool, const CSphString & sFile, const CSphString & sTmpFile, CSphString & sError );
bool DropJsonSI ( const CSphString & sFile, CSphString & sError );

std::unique_ptr<JsonSIBuilder_i> CreateJsonSIBuilder ( const ISphSchema & tSchema, const CSphString & sSPB, const CSphString & sSIFile, CSphString & sError );
CSphString UnifyJsonFieldName ( const CSphString & sName );
CSphString GetFixedJsonSIAttrName();
