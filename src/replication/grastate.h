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

#include "std/string.h"

// clean up Galera files
void CleanClusterFiles ( const CSphString& sPath );

// set safe_to_bootstrap: 1 at cluster/grastate.dat for Galera to start properly
bool NewClusterForce ( const CSphString& sPath );

bool CheckClusterNew ( const CSphString& sPath );