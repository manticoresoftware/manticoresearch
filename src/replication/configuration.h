//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "searchdaemon.h"
#include "std/string.h"

#pragma once

// returns true, if set data dir and replication ports
bool ReplicationEnabled();

// setup IP, ports and node incoming address
bool SetReplicationListener ( const VecTraits_T<ListenerDesc_t>& dListeners, CSphString & sError );

// listen IP part of address for Galera. The first of 'listen=' config with proto = replication
const char* szListenReplicationIP();

// incoming IP - one set by 'node_address=', or same as szListenReplicationIP, if none
const char* szIncomingIP();

bool HasIncoming() noexcept;

// incoming address (IP:port from API listener)
// used for request to this node from other daemons
const char* szIncomingProto();

// used to filter out own adrress
bool MyIncomingApiAddrBeginsWith ( const char* szHost );

void ReplicationSetIncoming ( CSphString sIncoming );
