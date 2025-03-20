//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "sphinxutils.h"

void AuthConfigure ( const CSphConfigSection & hSearchd );
bool IsAuthEnabled();

struct AgentConn_t;
void SetSessionAuth ( CSphVector<AgentConn_t *> & dRemotes );
void SetAuth ( const CSphString & sUser, CSphVector<AgentConn_t *> & dRemotes );
void SetAuth ( const CSphString & sUser, AgentConn_t * pAgent );
bool AuthReload ( CSphString & sError );