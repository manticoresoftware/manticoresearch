//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "datareader.h"
#include "sphinxint.h"
#include "fileutils.h"

//////////////////////////////////////////////////////////////////////////

inline static ESphQueryState StateByKind ( DataReaderFactory_c::Kind_e eKind )
{
	switch ( eKind )
	{
	case DataReaderFactory_c::DOCS: return SPH_QSTATE_READ_DOCS;
	case DataReaderFactory_c::HITS: return SPH_QSTATE_READ_HITS;
	default: return SPH_QSTATE_IO;
	}
}

//////////////////////////////////////////////////////////////////////////

class FileBlockReader_c : public FileBlockReader_i
{
public:
	explicit FileBlockReader_c ( const char * szFileName )
		: m_szFileName ( szFileName )
	{}

	RowID_t		UnzipRowid() override { return UnzipInt (); }
	SphWordID_t	UnzipWordid() override { return UnzipOffset (); }

protected:
	const char * m_szFileName = nullptr;
};

//////////////////////////////////////////////////////////////////////////

// imitate CSphReader but fully in memory (intended to be used with mmap)
class ThinMMapReader_c final : public FileBlockReader_c
{
public:
	SphOffset_t GetPos () const final
	{
		if ( !m_pPointer )
			return 0;

		assert ( m_pBase );
		return m_pPointer - m_pBase;
	}

	void SeekTo ( SphOffset_t iPos, int /*iSizeHint*/ ) final
	{
		m_pPointer = m_pBase + iPos;
	}

	void		GetBytes ( BYTE * pData, int iSize ) final;

	int			GetBytesZerocopy ( const BYTE *& pData, int iMax ) final;
	DWORD		GetDword() final;
	SphOffset_t	GetOffset() final;
	DWORD		UnzipInt () final;
	uint64_t	UnzipOffset () final;

	void Reset () final
	{
		m_pPointer = m_pBase;
	}

protected:
	~ThinMMapReader_c() final {}

private:
	friend class MMapFactory_c;

	const BYTE *	m_pBase = nullptr;
	const BYTE *	m_pPointer = nullptr;
	SphOffset_t		m_iSize = 0;

	ThinMMapReader_c ( const BYTE * pArena, SphOffset_t iSize, const char * sFileName )
		: FileBlockReader_c ( sFileName )
	{
		m_pPointer = m_pBase = pArena;
		m_iSize = iSize;
	}

	BYTE GetByte() override
	{
		auto iPos = m_pPointer - m_pBase;
		if ( iPos>=0 && iPos<m_iSize )
			return *m_pPointer++;

		sphWarning( "INTERNAL: out-of-range in ThinMMapReader_c: trying to read '%s' at " INT64_FMT ", from mmap of "
			INT64_FMT ", query most probably would FAIL; report the fact to dev!",
			( m_szFileName ? m_szFileName : "" ), int64_t(iPos), int64_t(m_iSize) );

		return 0; // it's better then crash because of unexpected read out-of-range (file reader does the same there)
	}
};


void ThinMMapReader_c::GetBytes ( BYTE * pData, int iSize )
{
	auto iPos = m_pPointer - m_pBase;
	if ( iPos>=0 && iPos+iSize<=m_iSize )
	{
		memcpy ( pData, m_pPointer, iSize );
		m_pPointer += iSize;
		return;
	}

	sphWarning ( "INTERNAL: out-of-range in ThinMMapReader_c: trying to read %d bytes from '%s' at " INT64_FMT ", from mmap of " INT64_FMT ", query most probably would FAIL; report the fact to dev!",
		iSize, ( m_szFileName ? m_szFileName : "" ), int64_t(iPos), int64_t(m_iSize) );
}


int ThinMMapReader_c::GetBytesZerocopy ( const BYTE *& pData, int iMax )
{
	if ( m_pPointer+iMax > m_pBase+m_iSize )
	{
		pData = m_pPointer;
		return 0;
	}

	pData = m_pPointer;
	m_pPointer += iMax;
	return iMax;
}


DWORD ThinMMapReader_c::GetDword()
{
	DWORD tRes;
	GetBytes ( (BYTE*)&tRes, sizeof(tRes) );
	return tRes;
}


SphOffset_t	ThinMMapReader_c::GetOffset()
{
	SphOffset_t tRes;
	GetBytes ( (BYTE*)&tRes, sizeof(tRes) );
	return tRes;
}


DWORD ThinMMapReader_c::UnzipInt()
{
	return UnzipValueBE<DWORD> ( [this]() mutable { return GetByte(); } );
}


uint64_t ThinMMapReader_c::UnzipOffset()
{
	return UnzipValueBE<uint64_t> ( [this]() mutable { return GetByte(); } );
}

//////////////////////////////////////////////////////////////////////////

class DirectFileReader_c final : public FileBlockReader_c, protected FileReader_c
{
	friend class DirectFactory_c;

public:
	void		SeekTo ( SphOffset_t iPos, int iSizeHint ) final		{ FileReader_c::SeekTo ( iPos, iSizeHint ); }
	void		GetBytes ( BYTE * pData, int iSize ) final				{ FileReader_c::GetBytes ( pData, iSize ); }
	int 		GetBytesZerocopy ( const BYTE *& pData, int iMax ) final { return FileReader_c::GetBytesZerocopy ( &pData, iMax ); }
	SphOffset_t GetPos () const final	{ return FileReader_c::GetPos(); }
	BYTE		GetByte() final			{ return FileReader_c::GetByte(); }
	DWORD		GetDword () final		{ return FileReader_c::GetDword(); }
	SphOffset_t	GetOffset() final		{ return FileReader_c::GetOffset(); }
	DWORD		UnzipInt() final		{ return FileReader_c::UnzipInt(); }
	uint64_t	UnzipOffset() final		{ return FileReader_c::UnzipOffset(); }
	void		Reset() final			{ FileReader_c::Reset(); }

protected:
	explicit DirectFileReader_c ( BYTE * pBuf, int iSize, const char * szFileName )
		: FileBlockReader_c ( szFileName )
		, FileReader_c ( pBuf, iSize )
	{}

	~DirectFileReader_c() final {}
};

//////////////////////////////////////////////////////////////////////////

// producer of readers which access by Seek + Read
class DirectFactory_c final : public DataReaderFactory_c
{
public:
	DirectFactory_c ( const CSphString & sFile, CSphString & sError, ESphQueryState eState, int iReadBuffer, int iReadUnhinted )
		: m_eWorkState ( eState )
		, m_iReadBuffer ( iReadBuffer )
		, m_iReadUnhinted ( iReadUnhinted )
	{
		SetValid ( m_dReader.Open ( sFile, sError ) );
	}

	uint64_t GetMappedsize () const final
	{
		return 0;
	}

	uint64_t GetCoresize () const final
	{
		return 0;
	}

	SphOffset_t GetFilesize () const final
	{
		return m_dReader.GetFilesize();
	}

	SphOffset_t GetPos () const final
	{
		return m_iPos;
	}

	void SeekTo ( SphOffset_t iPos ) final
	{
		m_iPos = iPos;
	}

	// returns depended reader sharing same FD as maker
	FileBlockReader_c * MakeReader ( BYTE * pBuf, int iSize ) final
	{
		auto pFileReader = new DirectFileReader_c ( pBuf, iSize, m_dReader.GetFilename().cstr() );
		pFileReader->SetFile ( m_dReader.GetFD(), m_dReader.GetFilename().cstr() );
		pFileReader->SetBuffers ( m_iReadBuffer, m_iReadUnhinted );
		if ( m_iPos )
			pFileReader->SeekTo ( m_iPos, READ_NO_SIZE_HINT );

		pFileReader->m_pProfile = m_dReader.m_pProfile;
		pFileReader->m_eProfileState = m_eWorkState;
		return pFileReader;
	}

	void SetProfile ( QueryProfile_c * pProfile ) final
	{
		m_dReader.m_pProfile = pProfile;
	}

protected:
	~DirectFactory_c() final {} // d-tr only by Release

private:
	CSphAutoreader	m_dReader;
	ESphQueryState	m_eWorkState;
	SphOffset_t		m_iPos = 0;
	int				m_iReadBuffer = 0;
	int				m_iReadUnhinted = 0;
};

//////////////////////////////////////////////////////////////////////////

// producer of readers which access by MMap
class MMapFactory_c final : public DataReaderFactory_c
{
public:
	MMapFactory_c ( const CSphString & sFile, CSphString & sError, FileAccess_e eAccess )
	{
		SetValid ( m_tBackendFile.Setup ( sFile, sError ) );
		if ( eAccess==FileAccess_e::MLOCK )
			m_tBackendFile.MemLock( sError );
	}

	uint64_t GetMappedsize () const final
	{
		return m_tBackendFile.GetLengthBytes();
	}

	uint64_t GetCoresize () const final
	{
		return m_tBackendFile.GetCoreSize();
	}

	SphOffset_t GetFilesize () const final
	{
		return m_tBackendFile.GetLength64 ();
	}

	SphOffset_t GetPos () const final
	{
		return m_iPos;
	}

	void SeekTo ( SphOffset_t iPos ) final
	{
		m_iPos = iPos;
	}

	// returns depended reader sharing same mmap as maker
	FileBlockReader_c * MakeReader ( BYTE *, int ) final
	{
		auto pReader = new ThinMMapReader_c ( m_tBackendFile.GetReadPtr(),
			m_tBackendFile.GetLength64(), m_tBackendFile.GetFileName() );
		if ( m_iPos )
			pReader->SeekTo ( m_iPos, 0 );
		return pReader;
	}

protected:
	~MMapFactory_c() final {} // d-tr only by Release

private:
	CSphMappedBuffer<BYTE>	m_tBackendFile;
	SphOffset_t				m_iPos = 0;
};

//////////////////////////////////////////////////////////////////////////

DataReaderFactory_c * NewProxyReader ( const CSphString & sFile, CSphString & sError, DataReaderFactory_c::Kind_e eKind, int iReadBuffer, FileAccess_e eAccess )
{
	auto eState = StateByKind ( eKind );
	CSphRefcountedPtr<DataReaderFactory_c> pReader;

	if ( eAccess==FileAccess_e::FILE )
		pReader = new DirectFactory_c ( sFile, sError, eState, iReadBuffer, GetUnhintedBuffer() );
	else
		pReader = new MMapFactory_c ( sFile, sError, eAccess );

	if ( !pReader->IsValid() )
		return nullptr;

	return pReader.Leak();
}
