//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "searchdsql.h"

enum class EPARAM_TYPE : BYTE
{
	SINGLE,
	VECTOR,
};

struct tScanParam
{
	int iStart;
	int iLen;
	EPARAM_TYPE eType = EPARAM_TYPE::SINGLE;
};

ParseResult_e ScanParameters ( Str_t sQuery, CSphVector<tScanParam>& dParams, CSphString& sError );

bool ValidateVector ( Str_t sVector, CSphString& sError );