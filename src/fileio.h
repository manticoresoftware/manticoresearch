//
// Copyright (c) 2017-2022, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _fileio_
#define _fileio_

#include "queryprofile.h"
#include "sphinxstd.h"
#include "sphinxdefs.h"

/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
public:
					CSphAutofile() = default;
					CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
					~CSphAutofile();

	int				Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
	void			Close ();
	void			SetTemporary(); ///< would be set if a shit happened and the file is not actual.
	int				GetFD () const { return m_iFD; }
	const char *	GetFilename () const;
	SphOffset_t		GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError );
	SphOffset_t		GetSize ();

	bool			Read ( void * pBuf, int64_t iCount, CSphString & sError );

protected:
	int			m_iFD = -1;					///< my file descriptor
	CSphString	m_sFilename;				///< my file name
	bool		m_bTemporary = false;		///< whether to unlink this file on Close()
	bool		m_bWouldTemporary = false;	///< backup of the m_bTemporary
};


/// file reader with read buffering and int decoder
class CSphReader
{
public:
	QueryProfile_c *	m_pProfile = nullptr;
	ESphQueryState		m_eProfileState { SPH_QSTATE_IO };


				CSphReader ( BYTE * pBuf=NULL, int iSize=0 );
	virtual		~CSphReader ();

	CSphReader & operator = ( const CSphReader & rhs );

	void		SetBuffers ( int iReadBuffer, int iReadUnhinted );
	void		SetFile ( int iFD, const char * sFilename );
	void		SetFile ( const CSphAutofile & tFile );
	void		Reset ();
	void		SeekTo ( SphOffset_t iPos, int iSizeHint );

	void		SkipBytes ( int iCount );
	SphOffset_t	GetPos () const { return m_iPos+m_iBuffPos; }

	void		GetBytes ( void * pData, int iSize );

	int			GetByte ();
	DWORD		GetDword ();
	SphOffset_t	GetOffset ();
	CSphString	GetString ();
	int			GetLine ( char * sBuffer, int iMaxLen );
	bool		Tag ( const char * sTag );

	DWORD		UnzipInt ();
	uint64_t	UnzipOffset ();

	bool					GetErrorFlag () const		{ return m_bError; }
	const CSphString &		GetErrorMessage () const	{ return m_sError; }
	const CSphString &		GetFilename() const			{ return m_sFilename; }
	int						GetBufferSize() const		{ return m_iBufSize; }
	void					ResetError();

	inline RowID_t		UnzipRowid ()	{ return UnzipInt(); }
	inline SphWordID_t	UnzipWordid ()	{ return UnzipOffset(); }

	///< zerocopy method; returns actual length present in buffer (upto iMax)
	inline int GetBytesZerocopy ( const BYTE ** ppData, int64_t iMax )
	{
		if ( m_iBuffPos>=m_iBuffUsed )
		{
			UpdateCache();
			if ( m_iBuffPos>=m_iBuffUsed )
				return 0; // unexpected io failure
		}

		int iChunk = (int)Min ( m_iBuffUsed-m_iBuffPos, iMax );
		*ppData = m_pBuff + m_iBuffPos;
		m_iBuffPos += iChunk;
		return iChunk;
	}

	SphOffset_t				GetFilesize() const;

protected:
	int			m_iFD = -1;
	CSphString m_sFilename;
	int			m_iBuffUsed = 0;	///< how many bytes in buffer are valid

	SphOffset_t	m_iPos = 0;			///< position in the file from witch m_pBuff starts
	BYTE *		m_pBuff;            ///< the buffer
	int			m_iBuffPos = 0;		///< position in the buffer. (so pos in file is m_iPos + m_iBuffPos)

	virtual void		UpdateCache ();

private:
	int			m_iSizeHint = 0;	///< how much do we expect to read (>=m_iReadUnhinted)

	int			m_iBufSize;
	bool		m_bBufOwned = false;
	int			m_iReadUnhinted;	///< how much to read if no hint provided.

	bool		m_bError = false;
	CSphString	m_sError;
};


class FileReader_c: public CSphReader
{
public:
	explicit FileReader_c ( BYTE* pBuf = nullptr, int iSize = 0 )
		: CSphReader ( pBuf, iSize )
	{}

	SphOffset_t GetFilesize () const;

	// added for DebugCheck()
	int GetFD () const { return m_iFD; }
};

/// scoped file reader
class CSphAutoreader : public FileReader_c
{
public:
	CSphAutoreader ( BYTE * pBuf=nullptr, int iSize=0 ) : FileReader_c ( pBuf, iSize ) {}
	~CSphAutoreader () override { Close(); }

	bool		Open ( const CSphString & sFilename, CSphString & sError );
	void		Close ();
};


/// file writer with write buffering and int encoder
class CSphWriter : ISphNoncopyable
{
public:
	virtual			~CSphWriter ();

	void			SetBufferSize ( int iBufferSize );	///< tune write cache size; must be called before OpenFile() or SetFile()

	bool			OpenFile ( const CSphString & sName, CSphString & sError );
	bool			OpenFile ( const CSphString & sName, int iOpenFlags, CSphString & sError );
	void			SetFile ( CSphAutofile & tAuto, SphOffset_t * pSharedOffset, CSphString & sError );
	void			CloseFile ( bool bTruncate = false );	///< note: calls Flush(), ie. IsError() might get true after this call
	void			UnlinkFile (); /// some shit happened (outside) and the file is no more actual.

	void			PutByte ( BYTE uValue );
	void			PutBytes ( const void * pData, int64_t iSize );
	void			PutWord ( WORD uValue ) { PutBytes ( &uValue, sizeof(WORD) ); }
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }
	void			PutString ( const char * szString );
	void			PutString ( const CSphString & sString );
	void			PutString ( Str_t tString ) { PutBytes ( tString.first, tString.second ); };
	void			Tag ( const char * sTag );

	void			SeekTo ( SphOffset_t iPos, bool bTruncate = false );

	void			ZipInt ( DWORD uValue );
	void			ZipOffset ( uint64_t uValue );

	bool			IsError () const	{ return m_bError; }
	SphOffset_t		GetPos () const		{ return m_iPos; }
	CSphString		GetFilename() const	{ return m_sName; }

	virtual void	Flush ();

protected:
	CSphString		m_sName;
	SphOffset_t		m_iPos = -1;
	SphOffset_t		m_iDiskPos = 0;

	int				m_iFD = -1;
	int				m_iPoolUsed = 0;
	BYTE *			m_pBuffer = nullptr;
	BYTE *			m_pPool = nullptr;
	bool			m_bOwnFile = false;
	SphOffset_t	*	m_pSharedOffset = nullptr;
	int				m_iBufferSize = 262144;

	bool			m_bError = false;
	CSphString *	m_pError = nullptr;

private:
	void			UpdatePoolUsed();
};


bool SeekAndWarn ( int iFD, SphOffset_t iPos, const char * szWarnPrefix );

// atomic seek+read wrapper
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset );

/// set throttling options
void sphSetThrottling ( int iMaxIOps, int iMaxIOSize );

/// write blob to file honoring throttling
bool sphWriteThrottled ( int iFD, const void* pBuf, int64_t iCount, const char* sName, CSphString& sError );

/// read blob from file honoring throttling
size_t sphReadThrottled ( int iFD, void* pBuf, size_t iCount );

#endif // _sphinxint_
