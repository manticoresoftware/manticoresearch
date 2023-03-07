//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "memio.h"


MemoryReader_c::MemoryReader_c ( const BYTE * pData, int iLen ) noexcept
	: m_pData ( pData )
	, m_iLen ( iLen )
	, m_pCur ( pData )
{}


MemoryReader_c::MemoryReader_c ( ByteBlob_t dData ) noexcept
	: m_pData ( dData.first )
	, m_iLen ( dData.second )
	, m_pCur ( dData.first )
{}


int MemoryReader_c::GetPos() const
{
	return ( m_pCur - m_pData );
}


void MemoryReader_c::SetPos ( int iOff )
{
	assert ( iOff>=0 && iOff<=m_iLen );
	m_pCur = m_pData + iOff;
}


CSphString MemoryReader_c::GetString()
{
	CSphString sRes;
	DWORD uLen = GetDword();
	if ( uLen )
	{
		sRes.Reserve ( uLen );
		GetBytes ( (BYTE *)sRes.cstr(), uLen );
	}

	return sRes;
}

void MemoryReader_c::GetBytes ( void * pData, int iLen )
{
	if ( !iLen )
		return;

	assert ( m_pCur );
	assert ( m_pCur<m_pData+m_iLen );
	assert ( m_pCur+iLen<=m_pData+m_iLen );
	memcpy ( pData, m_pCur, iLen );
	m_pCur += iLen;
}


const BYTE * MemoryReader_c::Begin() const
{
	return m_pData;
}


int MemoryReader_c::GetLength() const
{
	return m_iLen;
}

bool MemoryReader_c::HasData() const
{
	return GetPos() < m_iLen;
}

DWORD MemoryReader_c::UnzipInt()
{
	return UnzipIntLE ( m_pCur );
}


uint64_t MemoryReader_c::UnzipOffset()
{
	return UnzipOffsetLE ( m_pCur );
}

//////////////////////////////////////////////////////////////////////////

MemoryWriter_c::MemoryWriter_c ( CSphVector<BYTE> & dBuf )
	: m_dBuf ( dBuf )
{}


int MemoryWriter_c::GetPos()
{
	return m_dBuf.GetLength();
}


void MemoryWriter_c::PutString ( const CSphString & sVal )
{
	int iLen = sVal.Length();
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( (const BYTE *)sVal.cstr(), iLen );
}


void MemoryWriter_c::PutString ( const char * sVal )
{
	int iLen = 0;
	if ( sVal )
		iLen = (int) strlen ( sVal );
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( (const BYTE *)sVal, iLen );
}


void MemoryWriter_c::PutBytes ( const void * pData, int64_t iLen )
{
	if ( !iLen )
		return;

	BYTE * pCur = m_dBuf.AddN ( iLen );
	memcpy ( pCur, pData, iLen );
}


void MemoryWriter_c::PutByte ( BYTE uVal )
{
	m_dBuf.Add ( uVal );
}


void MemoryWriter_c::ZipOffset ( uint64_t uVal )
{
	ZipValueLE ( [this] ( BYTE b ) { PutByte ( b ); }, uVal );
}

void MemoryWriter_c::ZipInt ( DWORD uVal )
{
	ZipValueLE ( [this] ( BYTE b ) { PutByte ( b ); }, uVal );
}

//////////////////////////////////////////////////////////////////////////
MemoryReader2_c::MemoryReader2_c ( const BYTE * pData, int iLen )
	: MemoryReader_c ( pData, iLen )
{}


uint64_t MemoryReader2_c::UnzipInt()
{
	return UnzipIntBE(m_pCur);
}


uint64_t MemoryReader2_c::UnzipOffset()
{
	return UnzipOffsetBE(m_pCur);
}

//////////////////////////////////////////////////////////////////////////
MemoryWriter2_c::MemoryWriter2_c ( CSphVector<BYTE> & dBuf )
	: MemoryWriter_c ( dBuf )
{}


void MemoryWriter2_c::ZipOffset ( uint64_t uVal )
{
	ZipValueBE ( [this] ( BYTE b ) { PutByte ( b ); }, uVal );
}


void MemoryWriter2_c::ZipInt ( DWORD uVal )
{
	ZipValueBE ( [this] ( BYTE b ) { PutByte ( b ); }, uVal );
}
