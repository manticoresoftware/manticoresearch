//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/string.h"

CSphString EncodeBase64 ( const CSphString & sValue );
bool DecodeBase64 ( const CSphString & sValue, CSphString & sResult );

void DecodeBinBase64 ( const CSphString & sSrc, CSphVector<BYTE> & dDst );
CSphString EncodeBinBase64 ( const VecTraits_T<BYTE> & dSrc );
