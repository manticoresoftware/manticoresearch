//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
#include <optional>

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
CSphString GetKNNEmbeddingsFullpath();

// return value of asked ENV
[[nodiscard]] bool env_exists ( const char* szEnvName ) noexcept; // when just presence of an env is important; value doesn't matter.
[[nodiscard]] std::optional<bool> env_bool ( const char* szEnvName ) noexcept;
[[nodiscard]] std::optional<long> env_long ( const char* szEnvName ) noexcept;
[[nodiscard]] std::optional<unsigned long> env_ulong ( const char* szEnvName ) noexcept;
