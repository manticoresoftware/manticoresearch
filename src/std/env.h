//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "string.h"

/// use env variables, if available, instead of hard-coded macro
// this returns env FULL_SHARE_DIR, or hardcoded path, or '.' if nothing hardcoded
const char* GET_FULL_SHARE_DIR();

// returns env ICU_DATA_DIR, or hardcoded path, or nullptr if nothing hardcoded
CSphString GetICUDataDir();

// returns env JIEBA_DATA_DIR, or hardcoded path, or nullptr if nothing hardcoded
CSphString GetJiebaDataDir();

// this returns env MANTICORE_MODULES, or GET_FULL_SHARE_DIR()/modules
const char* GET_MANTICORE_MODULES();

// this returns env GALERA_SONAME, or GET_MANTICORE_MODULES()/libgalera_manticore.so
CSphString GET_GALERA_FULLPATH();

// this returns env LIB_MANTICORE_COLUMNAR, or GET_MANTICORE_MODULES()/lib_manticore_columnar.xx (xx=so or dll)
CSphString GetColumnarFullpath();
CSphString GetSecondaryFullpath();
CSphString GetKNNFullpath();

// return value of asked ENV, or default.
// note, default determines the type which to return
bool val_from_env ( const char* szEnvName, bool bDefault );
int val_from_env ( const char* szEnvName, int iDefault );
DWORD dwval_from_env ( const char* szEnvName, DWORD uDefault );
