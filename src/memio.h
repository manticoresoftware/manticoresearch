//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxstd.h"
#include "fileio.h"

class MemoryReader_c
{
public:
					MemoryReader_c ( const BYTE * pData, int iLen ) noexcept;
					explicit MemoryReader_c ( ByteBlob_t dData ) noexcept;

	int				GetPos() const;
	void			SetPos ( int iOff );
	uint64_t		UnzipOffset();
	DWORD			UnzipInt();
	CSphString		GetString();
	SphOffset_t		GetOffset();
	DWORD			GetDword();
	void			GetBytes ( void * pData, int iLen );
	const BYTE *	Begin() const;
	int				GetLength() const;
	bool 			HasData() const;

	template<typename T>
	T GetVal ();

	template<typename T>
	void GetVal ( T& tVal );

protected:
	const BYTE *	m_pData = nullptr;
	const int		m_iLen = 0;
	const BYTE *	m_pCur = nullptr;
};

template<typename T>
T GetVal( MemoryReader_c& tReader );

// first DWORD is len, then follows data
template<typename VECTOR>
void GetArray ( VECTOR& dBuf, MemoryReader_c& tIn );
void GetArray ( CSphVector<CSphString>& dBuf, MemoryReader_c& tIn );

class MemoryWriter_c : public Writer_i
{
public:
			MemoryWriter_c ( CSphVector<BYTE> & dBuf );

	int		GetPos();
	void	ZipOffset ( uint64_t uVal ) override;
	void	ZipInt ( DWORD uVal ) override;
	void	PutString ( const CSphString & sVal ) override;
	void	PutString ( const char * szVal ) override;
	void	PutZString ( const CSphString & sVal ) final;
	void	PutZString ( const char * szVal ) final;
	void	PutDword ( DWORD uVal ) override;
	void	PutOffset ( SphOffset_t uValue ) override;
	void	PutWord ( WORD uVal ) override;
	void	PutBytes ( const void * pData, int64_t iLen ) override;
	void	PutByte ( BYTE uVal ) override;
	void	PutUint64 ( uint64_t uVal );

	template<typename T>
	void PutVal ( T tVal );

protected:
	CSphVector<BYTE> & m_dBuf;
};

template<typename T>
void PutVal ( MemoryWriter_c& tWriter, T tVal );

// put DWORD size, then elems
template<typename T>
void SaveArray ( const VecTraits_T<T>& dBuf, MemoryWriter_c& tOut );
void SaveArray ( const VecTraits_T<CSphString>& dBuf, MemoryWriter_c& tOut );

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

	void	ZipOffset ( uint64_t uVal ) override;
	void	ZipInt ( DWORD uVal ) override;
};

#include "memio_impl.h"

#endif // _memio_
