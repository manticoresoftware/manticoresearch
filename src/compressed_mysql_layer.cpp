//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "compressed_mysql_layer.h"

// for UNALIGNED_RAM_ACCESS
#include "sphinxdefs.h"


DWORD MysqlCompressedSocket_c::ReadLSBSmallDword ( InputBuffer_c& tIn )
{
	BYTE dNum[3];
	tIn.GetBytes ( dNum, 3 );
	return (DWORD)dNum[2] << 16 | (DWORD)dNum[1] << 8 | (DWORD)dNum[0];
}

void MysqlCompressedSocket_c::SendLSBSmallDword ( DWORD uValue )
{
#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
	m_tOut.SendBytes ( &uValue, 3 );
#else
	BYTE dNum[3];
	dNum[0] = uValue & 0x000000FFU;
	dNum[1] = ( uValue >> 8 ) & 0x000000FFU;
	dNum[2] = ( uValue >> 16 ) & 0x000000FFU;
	m_tOut.SendBytes ( dNum, 3 );
#endif
}

MysqlCompressedSocket_c::MysqlCompressedSocket_c ( std::unique_ptr<AsyncNetBuffer_c> pFrontend )
	: m_pFrontend ( std::move ( pFrontend ) )
	, m_tIn ( *m_pFrontend )
	, m_tOut ( *m_pFrontend )
{
}

void MysqlCompressedSocket_c::SetWTimeoutUS ( int64_t iTimeoutUS )
{
	m_tOut.SetWTimeoutUS ( iTimeoutUS );
};

int64_t MysqlCompressedSocket_c::GetWTimeoutUS() const
{
	return m_tOut.GetWTimeoutUS();
}

void MysqlCompressedSocket_c::SetTimeoutUS ( int64_t iTimeoutUS )
{
	m_tIn.SetTimeoutUS ( iTimeoutUS );
};

int64_t MysqlCompressedSocket_c::GetTimeoutUS() const
{
	return m_tIn.GetTimeoutUS();
}

int64_t MysqlCompressedSocket_c::GetTotalReceived() const
{
	return m_tIn.GetTotalReceived();
}

int64_t MysqlCompressedSocket_c::GetTotalSent() const
{
	return m_tOut.GetTotalSent();
}
