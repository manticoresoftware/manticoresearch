//
// Copyright (c) 2017-2021, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _memio_
#define _memio_

#include "sphinx.h"

class MemoryReader_c
{
public:
					MemoryReader_c ( const BYTE * pData, int iLen );
					MemoryReader_c ( ByteBlob_t dData );

	int				GetPos();
	void			SetPos ( int iOff );
	uint64_t		UnzipOffset();
	DWORD			UnzipInt();
	CSphString		GetString();
	SphOffset_t		GetOffset();
	DWORD			GetDword();
	WORD			GetWord();
	void			GetBytes ( void * pData, int iLen );
	BYTE			GetByte();
	uint64_t		GetUint64();
	const BYTE *	Begin() const;
	int				GetLength() const;

protected:
	const BYTE *	m_pData = nullptr;
	const int		m_iLen = 0;
	const BYTE *	m_pCur = nullptr;
};


class MemoryWriter_c
{
public:
			MemoryWriter_c ( CSphVector<BYTE> & dBuf );

	int		GetPos();
	void	ZipOffset ( uint64_t uVal );
	void	ZipInt ( DWORD uVal );
	void	PutString ( const CSphString & sVal );
	void	PutString ( const char * szVal );
	void	PutDword ( DWORD uVal );
	void	PutOffset ( SphOffset_t uValue );
	void	PutWord ( WORD uVal );
	void	PutBytes ( const void * pData, int iLen );
	void	PutByte ( BYTE uVal );
	void	PutUint64 ( uint64_t uVal );

protected:
	CSphVector<BYTE> & m_dBuf;
};

// fixme: get rid of this
class MemoryReader2_c : public MemoryReader_c
{
public:
				MemoryReader2_c ( const BYTE * pData, int iLen );
		

	uint64_t	UnzipInt();
	uint64_t	UnzipOffset();
};

// fixme: get rid of this
class MemoryWriter2_c : public MemoryWriter_c
{
public:
			MemoryWriter2_c ( CSphVector<BYTE> & dBuf );

	void	ZipOffset ( uint64_t uVal );
	void	ZipInt ( DWORD uVal );
};

#endif // _memio_
