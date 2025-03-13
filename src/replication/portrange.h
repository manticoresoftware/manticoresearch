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

#include "std/generics.h"
#include "std/string.h"

class FreePortList_i;

// managed port got back into global ports list
class ScopedPort_c : public ISphNoncopyable
{
	int m_iPort = -1;
	FreePortList_i * m_pPortsList { nullptr };

public:
	ScopedPort_c() = default;
	ScopedPort_c ( int iPort, FreePortList_i * pPortList );
	~ScopedPort_c();

	ScopedPort_c ( ScopedPort_c&& rhs ) noexcept { Swap (rhs); }
	ScopedPort_c& operator = ( ScopedPort_c&& rhs ) noexcept { Swap (rhs); return *this; }
	void Swap ( ScopedPort_c& rhs ) noexcept { ::Swap ( m_iPort, rhs.m_iPort ); ::Swap ( m_pPortsList, rhs.m_pPortsList ); }

	operator int() const noexcept { return m_iPort; }
	[[nodiscard]] bool IsValid() const noexcept { return m_iPort!=-1;}
};

class FreePortList_i
{
public:
	FreePortList_i() = default;
	virtual ~FreePortList_i() {};

	virtual ScopedPort_c AcquirePort () = 0;
	virtual void Free ( int iPort ) = 0;
};

namespace PortRange
{
	ScopedPort_c AcquirePort();
	void AddPortsRange ( int iPort, int iCount );
	void AddAddr ( const CSphString & sAddr );

	FreePortList_i * Create ( const CSphString & sAddr, int iPort, int iCount );
}