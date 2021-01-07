//
// Copyright (c) 2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "compressed_mysql.h"
#include "networking_daemon.h"

#if USE_ZLIB
#include <zlib.h>

bool IsCompressionAvailable () { return true; }

// payloads below this threshold will be send as 'uncompressed' packets
static const int MIN_COMPRESS_LENGTH = 50;
static const int LEVEL_COMPRESSION = Z_DEFAULT_COMPRESSION;

class MysqlCompressedSocket_c final : public AsyncNetBuffer_c
{
	AsyncNetBufferPtr_c m_pFrontend;
	AsyncNetInputBuffer_c & m_tIn;
	NetGenericOutputBuffer_c & m_tOut;
	BYTE m_uPackedID = 0;

	bool SendPacket ( ByteBlob_t dBlob );

	DWORD ReadLSBSmallDword ( InputBuffer_c & tIn );
	void SendLSBSmallDword ( DWORD uValue );

protected:
	int ReadFromBackend ( int iNeed, int iHaveSpace, bool bIntr ) final;

public:
	explicit MysqlCompressedSocket_c ( AsyncNetBufferPtr_c pFrontend )
		: m_pFrontend ( std::move (pFrontend) )
		, m_tIn ( *m_pFrontend )
		, m_tOut ( *m_pFrontend )
	{
	}

	void SetWTimeoutUS ( int64_t iTimeoutUS ) final
	{
		m_tOut.SetWTimeoutUS ( iTimeoutUS );
	};

	int64_t GetWTimeoutUS () const final
	{
		return m_tOut.GetWTimeoutUS();
	}

	void SetTimeoutUS ( int64_t iTimeoutUS ) final
	{
		m_tIn.SetTimeoutUS ( iTimeoutUS );
	};

	int64_t GetTimeoutUS () const final
	{
		return m_tIn.GetTimeoutUS ();
	}

	bool SendBuffer ( const VecTraits_T<BYTE> & dData ) final;
};

void MakeMysqlCompressedLayer ( AsyncNetBufferPtr_c & pSource )
{
	if ( IsCompressionAvailable () )
		pSource = new MysqlCompressedSocket_c ( pSource );
}

// implementations

void MysqlCompressedSocket_c::SendLSBSmallDword ( DWORD uValue )
{
	#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
	m_tOut.SendBytes (&uValue, 3);
	#else
	BYTE dNum[3];
	dNum[0] = uValue & 0x000000FFU;
	dNum[1] = ( uValue >> 8 ) & 0x000000FFU;
	dNum[2] = ( uValue >> 16 ) & 0x000000FFU;
	m_tOut.SendBytes ( dNum, 3 );
	#endif
}

bool MysqlCompressedSocket_c::SendPacket ( ByteBlob_t dBlob )
{
	// send as uncompressed
	if ( dBlob.second<MIN_COMPRESS_LENGTH )
	{
		SendLSBSmallDword ( dBlob.second );
		m_tOut.SendByte ( ++m_uPackedID );
		SendLSBSmallDword ( 0 );
		m_tOut.SendBytes ( dBlob );
		return m_tOut.Flush ();
	}

	// compress and send compressed blob
	auto iPos = m_tOut.GetSentCount ();
	auto uSpace = (uLong) compressBound ( dBlob.second );
	auto pOut = m_tOut.ReservePlace ( uSpace+7 );
	m_tOut.SendBytes ( &uSpace, 3 );
	m_tOut.SendByte ( ++m_uPackedID );
	SendLSBSmallDword ( dBlob.second );
	compress2 ( pOut+7, &uSpace, dBlob.first, dBlob.second, LEVEL_COMPRESSION );
	m_tOut.Rewind ( iPos );
	SendLSBSmallDword ( uSpace );
	m_tOut.CommitZeroCopy ( uSpace+4 );
	return m_tOut.Flush ();
}

bool MysqlCompressedSocket_c::SendBuffer ( const VecTraits_T<BYTE> & dData )
{
	ByteBlob_t dBlob = dData;
	static const int iMaxOneCompressedPacketLen = 0x01000000U - 6;
	while ( !IsNull ( dBlob ) )
	{
		auto iSize = Min ( dBlob.second, iMaxOneCompressedPacketLen );
		if ( !SendPacket ( { dBlob.first, iSize } ) )
			return false;
		dBlob.first += iSize;
		dBlob.second -= iSize;
	}
	return true;
}

DWORD MysqlCompressedSocket_c::ReadLSBSmallDword ( InputBuffer_c& tIn )
{
	BYTE dNum[3];
	tIn.GetBytes ( dNum, 3 );
	return (DWORD) dNum[2] << 16 | (DWORD) dNum[1] << 8 | (DWORD) dNum[0];
}

int MysqlCompressedSocket_c::ReadFromBackend ( int iNeed, int iHaveSpace, bool bIntr )
{
	int iRead = 0;
	while ( iNeed>iRead )
	{
		// compressed packet header: 3 bytes of raw size, 1 byte packet id, 3 bytes of uncompressed size
		if ( !m_tIn.ReadFrom ( 7, bIntr ) )
			return -1;

		auto uRawSize = ReadLSBSmallDword (m_tIn);
		m_uPackedID = m_tIn.GetByte ();
		auto uUncompressedSize = (uLongf) ReadLSBSmallDword (m_tIn);

		// read raw packet (compressed or uncompressed)
		if ( !m_tIn.ReadFrom ( uRawSize, bIntr ) )
			return -1;

		if ( !uUncompressedSize ) // raw uncompressed, return as is
		{
			m_tIn.GetBytes ( BufferFor( uRawSize ), uRawSize );
			iRead += uRawSize;
			continue;
		}

		// compressed input
		const BYTE * pFoo = nullptr;
		m_tIn.GetBytesZerocopy ( &pFoo, uRawSize );
		auto iZResult = uncompress ( BufferFor ( uUncompressedSize ), &uUncompressedSize, pFoo, uRawSize );
		if ( iZResult!=Z_OK )
			return -1;
		iRead += uUncompressedSize;
	}
	return iRead;
}

#else // USE_ZLIB

bool IsCompressionAvailable () { return false; }
void MakeMysqlCompressedLayer ( AsyncNetBufferPtr_c & pSource ) { };

#endif