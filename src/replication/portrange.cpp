//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "sphinxstd.h"
#include "portrange.h"
#include "searchdaemon.h"

#if _WIN32
// Win-specific headers and calls
#include <io.h>
#else
// UNIX-specific headers and calls
#include <netinet/in.h>
#endif

struct PortsRange_t
{
	int m_iPort = 0;
	int m_iCount = 0;
};

static bool IsInetAddrFree ( DWORD uAddr, int iPort )
{
	static struct sockaddr_in iaddr;
	memset ( &iaddr, 0, sizeof ( iaddr ) );
	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = uAddr;
	iaddr.sin_port = htons ( (short)iPort );

	int iSock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSock == -1 )
	{
		sphWarning ( "failed to create TCP socket: %s", sphSockError() );
		return false;
	}

	int iRes = bind ( iSock, (struct sockaddr*)&iaddr, sizeof ( iaddr ) );
	SafeClose ( iSock );

	return ( iRes == 0 );
}

// manage ports pairs for clusters set as Galera replication listener ports range
class FreePortList_c
{
private:
	CSphMutex m_tLock;
	CSphVector<int> m_dFree GUARDED_BY ( m_tLock );
	CSphTightVector <PortsRange_t> m_dPorts GUARDED_BY ( m_tLock );
	DWORD m_uAddr = 0;

public:
	// set range of ports there is could generate ports pairs
	void AddRange ( const PortsRange_t & tPorts )
	{
		assert ( tPorts.m_iPort && tPorts.m_iCount && (tPorts.m_iCount % 2)==0 );
		ScopedMutex_t tLock ( m_tLock );
		m_dPorts.Add ( tPorts );
	}

	void AddAddr ( const CSphString& sAddr )
	{
		m_uAddr = sphGetAddress ( sAddr.cstr(), false, false );
	}

	// get next available range of ports for Galera listener
	// first reuse ports pair that was recently released
	// or pair from range set
	int Get ()
	{
		int iPortMin = -1;
		ScopedMutex_t tLock ( m_tLock );
		while ( m_dPorts.GetLength() || m_dFree.GetLength() )
		{
			if ( !m_dFree.IsEmpty ()) {
				iPortMin = m_dFree.Pop ();
			} else if ( m_dPorts.GetLength ()) {
				assert ( m_dPorts.Last ().m_iCount>=2 );
				PortsRange_t & tPorts = m_dPorts.Last ();
				iPortMin = tPorts.m_iPort;

				tPorts.m_iPort += 2;
				tPorts.m_iCount -= 2;
				if ( !tPorts.m_iCount )
					m_dPorts.Pop ();
			}

			if ( IsInetAddrFree ( m_uAddr, iPortMin ) && IsInetAddrFree ( m_uAddr, iPortMin + 1 ) )
				break;

			iPortMin = -1;
		}
		return iPortMin;
	}

	// free ports pair and add it to free list
	void Free ( int iPort )
	{
		ScopedMutex_t tLock ( m_tLock );
		m_dFree.Add ( iPort );
	}

	static FreePortList_c m_gPorts;
};

// ports pairs manager
FreePortList_c FreePortList_c::m_gPorts;

ScopedPort_c::ScopedPort_c ( int iPort ) : m_iPort ( iPort )
{
}

ScopedPort_c::~ScopedPort_c ()
{
	if ( m_iPort!=-1 )
		FreePortList_c::m_gPorts.Free ( m_iPort );
}

ScopedPort_c PortRange::AcquirePort ()
{
	return ScopedPort_c ( FreePortList_c::m_gPorts.Get ());
}

void PortRange::AddPortsRange ( int iPort, int iCount )
{
	PortsRange_t tPorts;
	tPorts.m_iPort = iPort;
	tPorts.m_iCount = iCount;
	if ((tPorts.m_iCount % 2)!=0 )
		--tPorts.m_iCount;

	FreePortList_c::m_gPorts.AddRange ( tPorts );
}

void PortRange::AddAddr ( const CSphString& sAddr )
{
	FreePortList_c::m_gPorts.AddAddr( sAddr );
}