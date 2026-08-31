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

#include "std/ints.h"
#include <optional>

namespace sd
{

int status ( const char *szMessage ) noexcept;
void statusf ( const char * sTemplate, ... ) noexcept;
void mainpid (int iPid) noexcept;
void ready() noexcept;
void stopping() noexcept;
void reloading() noexcept;
void extend30s() noexcept;
void keep_alive() noexcept;

[[nodiscard]] std::optional<uint64_t> WatchdogTimeout () noexcept;

}