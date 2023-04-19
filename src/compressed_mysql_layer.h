//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "networking_daemon.h"
#include "sphinxstd.h"

// payloads below this threshold will be send as 'uncompressed' packets
static const int MIN_COMPRESS_LENGTH = 50;

class MysqlCompressedSocket_c : public AsyncNetBuffer_c
{
protected:
	std::unique_ptr<AsyncNetBuffer_c> m_pFrontend;
	AsyncNetInputBuffer_c& m_tIn;
	GenericOutputBuffer_c& m_tOut;
	BYTE m_uPackedID = 0;

	DWORD ReadLSBSmallDword ( InputBuffer_c& tIn );
	void SendLSBSmallDword ( DWORD uValue );

public:
	explicit MysqlCompressedSocket_c ( std::unique_ptr<AsyncNetBuffer_c> pFrontend );
	void SetWTimeoutUS ( int64_t iTimeoutUS ) final;
	int64_t GetWTimeoutUS() const final;
	void SetTimeoutUS ( int64_t iTimeoutUS ) final;
	int64_t GetTimeoutUS() const final;
	int64_t GetTotalReceived() const final;
	int64_t GetTotalSent() const final;
};


template<typename COMPR>
class MysqlCompressedSocket_T : public MysqlCompressedSocket_c, public COMPR
{
	bool SendPacket ( ByteBlob_t dBlob )
	{
		// send as uncompressed
		if ( dBlob.second < MIN_COMPRESS_LENGTH )
		{
			SendLSBSmallDword ( dBlob.second );
			m_tOut.SendByte ( ++m_uPackedID );
			SendLSBSmallDword ( 0 );
			m_tOut.SendBytes ( dBlob );
			return m_tOut.Flush();
		}

		// compress and send compressed blob
		auto iPos = m_tOut.GetSentCount();
		auto uSpace = COMPR::Common_compressBound ( dBlob.second );
		auto pOut = m_tOut.ReservePlace ( uSpace + 7 );
		m_tOut.SendBytes ( &uSpace, 3 );
		m_tOut.SendByte ( ++m_uPackedID );
		SendLSBSmallDword ( dBlob.second );
		COMPR::Common_compress ( pOut + 7, &uSpace, dBlob.first, dBlob.second );
		m_tOut.Rewind ( iPos );
		SendLSBSmallDword ( uSpace );
		m_tOut.CommitZeroCopy ( uSpace + 4 );
		return m_tOut.Flush();
	}

protected:
	int ReadFromBackend ( int iNeed, int, bool bIntr ) final
	{
		int iRead = 0;
		while ( iNeed > iRead )
		{
			// compressed packet header: 3 bytes of raw size, 1 byte packet id, 3 bytes of uncompressed size
			if ( !m_tIn.ReadFrom ( 7, bIntr ) )
				return -1;

			auto uRawSize = ReadLSBSmallDword ( m_tIn );
			m_uPackedID = m_tIn.GetByte();
			auto uUncompressedSize = (typename COMPR::csize_t) ReadLSBSmallDword ( m_tIn );

			// read raw packet (compressed or uncompressed)
			if ( !m_tIn.ReadFrom ( uRawSize, bIntr ) )
				return -1;

			if ( !uUncompressedSize ) // raw uncompressed, return as is
			{
				m_tIn.GetBytes ( AllocateBuffer ( uRawSize ).begin(), uRawSize );
				iRead += uRawSize;
				continue;
			}

			// compressed input
			const BYTE* pFoo = nullptr;
			m_tIn.GetBytesZerocopy ( &pFoo, uRawSize );
			if (!COMPR::Common_uncompress ( AllocateBuffer ( uUncompressedSize ).begin(), &uUncompressedSize, pFoo, uRawSize ))
				return -1;
			iRead += uUncompressedSize;
		}
		return iRead;
	}

public:
	explicit MysqlCompressedSocket_T ( std::unique_ptr<AsyncNetBuffer_c> pFrontend )
		: MysqlCompressedSocket_c ( std::move ( pFrontend ) )
	{}

	bool SendBuffer ( const VecTraits_T<BYTE>& dData ) final
	{
		ByteBlob_t dBlob = dData;
		static const int iMaxOneCompressedPacketLen = 0x01000000U - 6;
		while ( !::IsEmpty ( dBlob ) )
		{
			auto iSize = Min ( dBlob.second, iMaxOneCompressedPacketLen );
			if ( !SendPacket ( { dBlob.first, iSize } ) )
				return false;
			dBlob.first += iSize;
			dBlob.second -= iSize;
		}
		return true;
	}
};
