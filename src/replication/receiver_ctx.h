//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "wsrep_cxx.h"

// returns main receiver which will take and apply replication commands from remote nodes
Wsrep::Receiver_i* MakeReceiverCtx ( CSphString sName, Wsrep::Provider_i * pProvider, std::function<void()> fnOnClean );
