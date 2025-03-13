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

#include "portrange.h"
#include "searchdaemon.h"
#include "searchdha.h"

static constexpr int g_iDefaultPortBias = 10;
static constexpr int g_iDefaultPortRange = 200;

// incoming address guessed (false) or set via searchd.node_address
bool g_bHasIncoming = false;
// incoming IP part of address set by searchd.node_address or took from listener
static CSphString g_sIncomingIP;
// incoming address (IP:port from API listener) used for request to this node from other daemons
static CSphString g_sIncomingApiPoint;
// listen IP part of address for Galera
static CSphString g_sListenReplicationIP;

// setup IP, ports and node incoming address
bool SetReplicationListener ( const VecTraits_T<ListenerDesc_t> & dListeners, CSphString & sError )
{
	bool bGotReplicationPorts = false;
	for ( const ListenerDesc_t& tListen : dListeners )
	{
		if ( tListen.m_eProto!=Proto_e::REPLICATION )
			continue;

		const bool bBadCount = ( tListen.m_iPortsCount<2 );
		const bool bBadRange = ( ( tListen.m_iPortsCount%2 )!=0 && ( tListen.m_iPortsCount-1 )<2 );
		if ( bBadCount || bBadRange )
			sphFatal ( "invalid replication ports count %d, should be at least 2", tListen.m_iPortsCount+1 );

		// can not use 0.0.0.0 due to Galera error at ReplicatorSMM::InitConfig::InitConfig
		if ( tListen.m_uIP != 0 )
		{
			std::array<char, SPH_ADDRESS_SIZE> sListenerIP {};
			sphFormatIP ( sListenerIP.data(), SPH_ADDRESS_SIZE, tListen.m_uIP );
			if ( g_sListenReplicationIP.IsEmpty() )
			{
				g_sListenReplicationIP = sListenerIP.data();
				PortRange::AddAddr ( g_sListenReplicationIP );
			} else if ( g_sListenReplicationIP != (const char*)sListenerIP.data() )
			{
				sphWarning ( "multiple replication IP ('%s') found but only 1st IP '%s' used", sListenerIP.data(), g_sListenReplicationIP.cstr() );
			}
		} else
		{
			g_sListenReplicationIP = g_bHasIncoming ? g_sIncomingIP : "127.0.0.1";
			if ( g_sIncomingIP.IsEmpty() )
				sphWarning ( "can not set '0.0.0.0' as Galera IP, '%s' used", g_sListenReplicationIP.cstr() );
			else
				sphLogDebugRpl ( "set '%s' as Galera IP", g_sListenReplicationIP.cstr() );
		}

		PortRange::AddPortsRange (tListen.m_iPort, tListen.m_iPortsCount);
		bGotReplicationPorts = true;
	}

	int iAPIPort = dListeners.GetFirst ( [&] ( const ListenerDesc_t & tListen ) { return tListen.m_eProto==Proto_e::SPHINX; } );
	if ( iAPIPort==-1 )
	{
		sError = "no 'listen' is found, cannot set incoming addresses, replication is disabled";
		return false;
	}

	if ( !bGotReplicationPorts )
	{
		const ListenerDesc_t& tListen = dListeners[iAPIPort];

		if ( tListen.m_uIP != 0 )
		{
			std::array<char, SPH_ADDRESS_SIZE> sListenerIP {};
			sphFormatIP ( sListenerIP.data(), SPH_ADDRESS_SIZE, tListen.m_uIP );
			g_sListenReplicationIP = sListenerIP.data();
		} else
		{
			g_sListenReplicationIP = "127.0.0.1";
		}

		PortRange::AddPortsRange ( tListen.m_iPort + g_iDefaultPortBias, g_iDefaultPortRange );
		PortRange::AddAddr ( g_sListenReplicationIP );
	}

	if ( !g_bHasIncoming )
		g_sIncomingIP = g_sListenReplicationIP;

	sphLogDebugRpl ( "listens: Galera '%s', own '%s:%d'", g_sListenReplicationIP.cstr(), g_sIncomingIP.cstr(), dListeners[iAPIPort].m_iPort );
	g_sIncomingApiPoint.SetSprintf ( "%s:%d", g_sIncomingIP.cstr(), dListeners[iAPIPort].m_iPort );
	if ( !IsConfigless() )
		sError = "data_dir option is missing in config, replication is disabled";

	return IsConfigless();
}

void ReplicationSetIncoming ( CSphString sIncoming )
{
	g_sIncomingIP = sIncoming;
	g_bHasIncoming = !g_sIncomingIP.IsEmpty();
}

const char* szIncomingIP()
{
	return g_sIncomingIP.cstr();
}

bool HasIncoming() noexcept
{
	return g_bHasIncoming;
}

const char* szListenReplicationIP()
{
	return g_sListenReplicationIP.cstr();
}

const char* szIncomingProto()
{
	return g_sIncomingApiPoint.cstr();
}

bool MyIncomingApiAddrBeginsWith ( const char* szHost )
{
	return g_sIncomingApiPoint.Begins ( szHost );
}
