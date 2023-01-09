//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _datareader_
#define _datareader_

#include "sphinxdefs.h"
#include "indexsettings.h"

#define READ_NO_SIZE_HINT 0

// Reader from file or filemap
class FileBlockReader_i : public ISphRefcountedMT
{
public:
	virtual SphOffset_t	GetPos() const = 0;
	virtual void		SeekTo ( SphOffset_t iPos, int iSizeHint ) = 0;
	virtual void		GetBytes ( BYTE * pData, int iSize ) = 0;
	virtual int			GetBytesZerocopy ( const BYTE *& pData, int iMax ) = 0;
	virtual BYTE		GetByte() = 0;
	virtual DWORD		GetDword() = 0;
	virtual SphOffset_t	GetOffset() = 0;
	virtual DWORD		UnzipInt() = 0;
	virtual uint64_t	UnzipOffset() = 0;
	virtual RowID_t		UnzipRowid() = 0;
	virtual SphWordID_t	UnzipWordid() = 0;
	virtual void		Reset () = 0;
};


using FileBlockReaderPtr_c = CSphRefcountedPtr<FileBlockReader_i>;
class QueryProfile_c;

// producer of readers from file or filemap
class DataReaderFactory_c : public ISphRefcountedMT
{
public:
	enum Kind_e
	{
		DOCS,
		HITS,
		COLUMNAR
	};

	bool						IsValid () const { return m_bValid; }

	virtual uint64_t			GetMappedsize () const = 0;
	virtual uint64_t 			GetCoresize () const = 0;
	virtual SphOffset_t			GetFilesize () const = 0;
	virtual SphOffset_t			GetPos () const = 0;
	virtual void				SeekTo ( SphOffset_t ) = 0;
	virtual FileBlockReader_i *	MakeReader ( BYTE * pBuf, int iSize ) = 0;
	virtual void				SetProfile ( QueryProfile_c * ) {}

protected:
								~DataReaderFactory_c () override {}

	void						SetValid ( bool bValid ) { m_bValid = bValid; }

private:
	bool m_bValid = false;
};

using DataReaderFactoryPtr_c = CSphRefcountedPtr<DataReaderFactory_c>;

DataReaderFactory_c * NewProxyReader ( const CSphString & sFile, CSphString & sError, DataReaderFactory_c::Kind_e eKind, int iReadBuffer, FileAccess_e eAccess );

#endif // _datareader_
