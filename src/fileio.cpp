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

#include "fileio.h"
#include "sphinxint.h"

#define SPH_READ_NOPROGRESS_CHUNK (32768*1024)

//////////////////////////////////////////////////////////////////////////

CSphAutofile::CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp )
{
	Open ( sName, iMode, sError, bTemp );
}


CSphAutofile::~CSphAutofile()
{
	Close();
}


static int AutoFileOpen ( const CSphString & sName, int iMode )
{
	int iFD = -1;
#if _WIN32
	if ( iMode==SPH_O_READ )
	{
		intptr_t tFD = (intptr_t)CreateFile ( sName.cstr(), GENERIC_READ , FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		iFD = _open_osfhandle ( tFD, 0 );
	} else
		iFD = ::open ( sName.cstr(), iMode, 0644 );
#else
	iFD = ::open ( sName.cstr(), iMode, 0644 );
#endif

	return iFD;
}


int CSphAutofile::Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp )
{
	assert ( m_iFD==-1 && m_sFilename.IsEmpty() );
	assert ( !sName.IsEmpty() );

	m_iFD = AutoFileOpen ( sName, iMode );
	m_sFilename = sName; // not exactly sure why is this unconditional. for error reporting later, i suppose

	if ( m_iFD<0 )
		sError.SetSprintf ( "failed to open %s: %s", sName.cstr(), strerrorm(errno) );
	else
		m_bTemporary = bTemp; // only if we managed to actually open it

	return m_iFD;
}


void CSphAutofile::Close()
{
	if ( m_iFD>=0 )
	{
		::close ( m_iFD );
		if ( m_bTemporary )
			::unlink ( m_sFilename.cstr() );
	}

	m_iFD = -1;
	m_sFilename = "";
	m_bTemporary = false;
}


int CSphAutofile::LeakID ()
{
	m_sFilename = "";
	m_bTemporary = false;
	return std::exchange ( m_iFD, -1 );
}

void CSphAutofile::SetPersistent()
{
	m_bTemporary = false;
}

const char * CSphAutofile::GetFilename() const
{
	return m_sFilename.scstr();
}


SphOffset_t CSphAutofile::GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError )
{
	struct_stat st;
	if ( stat ( GetFilename(), &st )<0 )
	{
		sError.SetSprintf ( "failed to stat %s: %s", GetFilename(), strerrorm(errno) );
		return -1;
	}
	if ( st.st_size<iMinSize )
	{
		sError.SetSprintf ( "failed to load %s: bad size " INT64_FMT " (at least " INT64_FMT " bytes expected)",
			GetFilename(), (int64_t)st.st_size, (int64_t)iMinSize );
		return -1;
	}
	if ( bCheckSizeT )
	{
		size_t uCheck = (size_t)st.st_size;
		if ( st.st_size!=SphOffset_t(uCheck) )
		{
			sError.SetSprintf ( "failed to load %s: bad size " INT64_FMT " (out of size_t; 4 GB limit on 32-bit machine hit?)",
				GetFilename(), (int64_t)st.st_size );
			return -1;
		}
	}
	return st.st_size;
}


SphOffset_t CSphAutofile::GetSize()
{
	CSphString sTmp;
	return GetSize ( 0, false, sTmp );
}


bool CSphAutofile::Read ( void * pBuf, int64_t iCount, CSphString & sError )
{
	assert ( iCount>=0 );

	int64_t iToRead = iCount;
	BYTE * pCur = (BYTE *)pBuf;
	while ( iToRead>0 )
	{
		int64_t iToReadOnce = Min ( iToRead, SPH_READ_NOPROGRESS_CHUNK );
		int64_t iGot = sphRead ( GetFD(), pCur, (size_t)iToReadOnce );

		if ( iGot==-1 )
		{
			// interrupted by a signal - try again
			if ( errno==EINTR )
				continue;

			sError.SetSprintf ( "read error in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
				GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
			return false;
		}

		// EOF
		if ( iGot==0 )
		{
			sError.SetSprintf ( "unexpected EOF in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
				GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
			return false;
		}

		iToRead -= iGot;
		pCur += iGot;
	}

	if ( iToRead!=0 )
	{
		sError.SetSprintf ( "read error in %s (%s); " INT64_FMT " of " INT64_FMT " bytes read",
			GetFilename(), strerrorm(errno), iCount-iToRead, iCount );
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

CSphReader::CSphReader ( BYTE * pBuf, int iSize )
	: m_pBuff ( pBuf )
	, m_iBufSize ( iSize )
	, m_iReadUnhinted ( DEFAULT_READ_UNHINTED )
{
	assert ( pBuf==NULL || iSize>0 );
}


CSphReader::~CSphReader()
{
	if ( m_bBufOwned )
		SafeDeleteArray ( m_pBuff );
}


void CSphReader::SetBuffers ( int iReadBuffer, int iReadUnhinted )
{
	if ( !m_pBuff )
		m_iBufSize = iReadBuffer;
	m_iReadUnhinted = iReadUnhinted;
}


void CSphReader::SetFile ( int iFD, const char * sFilename )
{
	m_iFD = iFD;
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
	m_sFilename = sFilename;
}


void CSphReader::SetFile ( const CSphAutofile & tFile )
{
	SetFile ( tFile.GetFD(), tFile.GetFilename() );
}


void CSphReader::Reset()
{
	SetFile ( -1, "" );
}


/// sizehint > 0 means we expect to read approx that much bytes
/// sizehint == 0 means no hint, use default (happens later in UpdateCache())
/// sizehint == -1 means reposition and adjust current hint
void CSphReader::SeekTo ( SphOffset_t iPos, int iSizeHint )
{
	assert ( iPos>=0 );
	assert ( iSizeHint>=-1 );

#ifndef NDEBUG
#if PARANOID
	struct_stat tStat;
	fstat ( m_iFD, &tStat );
	if ( iPos > tStat.st_size )
		sphDie ( "INTERNAL ERROR: seeking past the end of file" );
#endif
#endif

	if ( iPos>=m_iPos && iPos<m_iPos+m_iBuffUsed )
	{
		m_iBuffPos = (int)( iPos-m_iPos ); // reposition to proper byte
		m_iSizeHint = iSizeHint - ( m_iBuffUsed - m_iBuffPos ); // we already have some bytes cached, so let's adjust size hint
		assert ( m_iBuffPos<m_iBuffUsed );
	} else
	{
		m_iPos = iPos;
		m_iBuffPos = 0; // for GetPos() to work properly, aaaargh
		m_iBuffUsed = 0;

		if ( iSizeHint==-1 )
		{
			// the adjustment bureau
			// we need to seek but still keep the current hint
			// happens on a skiplist jump, for instance
			int64_t iHintLeft = m_iPos + m_iSizeHint - iPos;
			if ( iHintLeft>0 && iHintLeft<INT_MAX )
				iSizeHint = (int)iHintLeft;
			else
				iSizeHint = 0;
		}

		// get that hint
		assert ( iSizeHint>=0 );
		m_iSizeHint = iSizeHint;
	}
}


void CSphReader::SkipBytes ( int iCount )
{
	// 0 means "no hint", so this clamp works alright
	SeekTo ( m_iPos+m_iBuffPos+iCount, Max ( m_iSizeHint-m_iBuffPos-iCount, 0 ) );
}


void CSphReader::UpdateCache()
{
	CSphScopedProfile tProf ( m_pProfile, m_eProfileState );

	assert ( m_iFD>=0 );

	// alloc buf on first actual read
	if ( !m_pBuff )
	{
		if ( m_iBufSize<=0 )
			m_iBufSize = DEFAULT_READ_BUFFER;

		m_bBufOwned = true;
		m_pBuff = new BYTE [ m_iBufSize ];
	}

	// stream position could be changed externally
	// so let's just hope that the OS optimizes redundant seeks
	SphOffset_t iNewPos = m_iPos + Min ( m_iBuffPos, m_iBuffUsed );

	if ( m_iSizeHint<=0 )
		m_iSizeHint = ( m_iReadUnhinted>0 ) ? m_iReadUnhinted : DEFAULT_READ_UNHINTED;
	int iReadLen = Min ( m_iSizeHint, m_iBufSize );

	m_iBuffPos = 0;
	m_iBuffUsed = sphPread ( m_iFD, m_pBuff, iReadLen, iNewPos ); // FIXME! what about throttling?

	if ( m_iBuffUsed<0 )
	{
		m_iBuffUsed = m_iBuffPos = 0;
		m_bError = true;
		m_sError.SetSprintf ( "pread error in %s: pos=" INT64_FMT ", len=%d, code=%d, msg=%s", m_sFilename.cstr(), (int64_t)iNewPos, iReadLen, errno, strerror(errno) );
		return;
	}

	// all fine, adjust offset and hint
	m_iSizeHint -= m_iBuffUsed;
	m_iPos = iNewPos;
}


int CSphReader::GetByte()
{
	if ( m_iBuffPos>=m_iBuffUsed )
	{
		UpdateCache();
		if ( m_iBuffPos>=m_iBuffUsed )
		{
			m_bError = true;
			m_sError.SetSprintf ( "pread error in %s: pos=" INT64_FMT ", len=%d", m_sFilename.cstr(), (int64_t)m_iPos, 1 );
			return 0; // unexpected io failure
		}
	}

	assert ( m_iBuffPos<m_iBuffUsed );
	return m_pBuff [ m_iBuffPos++ ];
}


void CSphReader::GetBytes ( void * pData, int iSize )
{
	BYTE * pOut = (BYTE*) pData;

	while ( iSize>m_iBufSize )
	{
		int iLen = m_iBuffUsed - m_iBuffPos;
		assert ( iLen<=m_iBufSize );

		memcpy ( pOut, m_pBuff+m_iBuffPos, iLen );
		m_iBuffPos += iLen;
		pOut += iLen;
		iSize -= iLen;
		m_iSizeHint = Max ( m_iReadUnhinted, iSize );

		if ( iSize>0 )
		{
			UpdateCache();
			if ( !m_iBuffUsed )
			{
				m_sError.SetSprintf ( "pread error in %s: pos=" INT64_FMT ", len=%d, code=%d, msg=%s", m_sFilename.cstr(), (int64_t)m_iPos, iSize, errno, strerror(errno) );
				memset ( pData, 0, iSize );
				return; // unexpected io failure
			}
		}
	}

	if ( iSize>m_iBuffUsed-m_iBuffPos )
	{
		// move old buffer tail to buffer head to avoid losing the data
		const int iLen = m_iBuffUsed - m_iBuffPos;
		if ( iLen>0 )
		{
			memcpy ( pOut, m_pBuff+m_iBuffPos, iLen );
			m_iBuffPos += iLen;
			pOut += iLen;
			iSize -= iLen;
		}

		m_iSizeHint = Max ( m_iReadUnhinted, iSize );
		UpdateCache();
		if ( iSize>m_iBuffUsed-m_iBuffPos )
		{
			memset ( pData, 0, iSize ); // unexpected io failure
			m_bError = true;
			m_sError.SetSprintf ( "pread error in %s: pos=" INT64_FMT ", len=%d", m_sFilename.cstr(), (int64_t)m_iPos, iSize );
			return;
		}
	}

	assert ( (m_iBuffPos+iSize)<=m_iBuffUsed );
	memcpy ( pOut, m_pBuff+m_iBuffPos, iSize );
	m_iBuffPos += iSize;
}


int CSphReader::GetLine ( char * sBuffer, int iMaxLen )
{
	int iOutPos = 0;
	iMaxLen--; // reserve space for trailing '\0'

			   // grab as many chars as we can
	while ( iOutPos<iMaxLen )
	{
		// read next chunk if necessary
		if ( m_iBuffPos>=m_iBuffUsed )
		{
			UpdateCache();
			if ( m_iBuffPos>=m_iBuffUsed )
			{
				if ( iOutPos==0 ) return -1; // current line is empty; indicate eof
				break; // return current line; will return eof next time
			}
		}

		// break on CR or LF
		if ( m_pBuff[m_iBuffPos]=='\r' || m_pBuff[m_iBuffPos]=='\n' )
			break;

		// one more valid char
		sBuffer[iOutPos++] = m_pBuff[m_iBuffPos++];
	}

	// skip everything until the newline or eof
	while (true)
	{
		// read next chunk if necessary
		if ( m_iBuffPos>=m_iBuffUsed )
			UpdateCache();

		// eof?
		if ( m_iBuffPos>=m_iBuffUsed )
			break;

		// newline?
		if ( m_pBuff[m_iBuffPos++]=='\n' )
			break;
	}

	// finalize
	sBuffer[iOutPos] = '\0';
	return iOutPos;
}


void CSphReader::ResetError()
{
	m_bError = false;
	m_sError = "";
}


SphOffset_t	CSphReader::GetFilesize() const
{
	assert ( m_iFD>=0 );

	return sphGetFileSize ( m_iFD, nullptr );
}

#if TRACE_UNZIP
std::array<std::atomic<uint64_t>, 5> CSphReader::m_dZip32Stats = { 0 };
std::array<std::atomic<uint64_t>, 10> CSphReader::m_dZip64Stats = { 0 };

DWORD CSphReader::UnzipInt()
{
	DWORD uRes = UnzipValueBE<DWORD> ( [this]() mutable { return GetByte(); } );
	m_dZip32Stats[sphCalcZippedLen ( uRes ) - 1].fetch_add ( 1, std::memory_order_relaxed );
	return uRes;
}


uint64_t CSphReader::UnzipOffset()
{
	uint64_t uRes = UnzipValueBE<uint64_t> ( [this]() mutable { return GetByte(); } );
	m_dZip64Stats[sphCalcZippedLen ( uRes ) - 1].fetch_add ( 1, std::memory_order_relaxed );
	return uRes;
}
#else
DWORD CSphReader::UnzipInt()
{
	return UnzipValueBE<DWORD> ( [this]() mutable { return GetByte(); } );
}


uint64_t CSphReader::UnzipOffset()
{
	return UnzipValueBE<uint64_t> ( [this]() mutable { return GetByte(); } );
}
#endif

CSphReader & CSphReader::operator = ( const CSphReader & rhs )
{
	SetFile ( rhs.m_iFD, rhs.m_sFilename.cstr() );
	SeekTo ( rhs.m_iPos + rhs.m_iBuffPos, rhs.m_iSizeHint );
	return *this;
}


DWORD CSphReader::GetDword()
{
	DWORD uRes = 0;
	GetBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


SphOffset_t CSphReader::GetOffset()
{
	SphOffset_t uRes = 0;
	GetBytes ( &uRes, sizeof(SphOffset_t) );
	return uRes;
}


CSphString CSphReader::GetString()
{
	CSphString sRes;
	DWORD uLen = GetDword ();
	if ( uLen )
	{
		sRes.Reserve ( uLen );
		GetBytes ( (BYTE *) sRes.cstr (), uLen );
	}

	return sRes;
}


CSphString CSphReader::GetZString ()
{
	CSphString sRes;
	auto uLen = UnzipOffset();
	if ( uLen )
	{
		sRes.Reserve ( uLen );
		GetBytes ( (BYTE *) sRes.cstr (), uLen );
	}

	return sRes;
}

bool CSphReader::Tag ( const char * sTag )
{
	if ( m_bError )
		return false;

	assert ( sTag && *sTag ); // empty tags are nonsense
	assert ( strlen(sTag)<64 ); // huge tags are nonsense

	auto iLen = (int) strlen(sTag);
	char sBuf[64];
	GetBytes ( sBuf, iLen );
	if ( !memcmp ( sBuf, sTag, iLen ) )
		return true;
	m_bError = true;
	m_sError.SetSprintf ( "expected tag %s was not found", sTag );
	return false;
}


//////////////////////////////////////////////////////////////////////////

bool CSphAutoreader::Open ( const CSphString & sFilename, CSphString & sError )
{
	assert ( m_iFD<0 );
	assert ( !sFilename.IsEmpty() );

	m_iFD = AutoFileOpen ( sFilename, SPH_O_READ );
	m_iPos = 0;
	m_iBuffPos = 0;
	m_iBuffUsed = 0;
	m_sFilename = sFilename;

	if ( m_iFD<0 )
		sError.SetSprintf ( "failed to open %s: %s", sFilename.cstr(), strerror(errno) );
	return ( m_iFD>=0 );
}


void CSphAutoreader::Close()
{
	if ( m_iFD>=0 )
		::close ( m_iFD	);
	m_iFD = -1;
}


SphOffset_t FileReader_c::GetFilesize() const
{
	assert ( m_iFD>=0 );

	return sphGetFileSize ( m_iFD, nullptr );
}

//////////////////////////////////////////////////////////////////////////

void CSphWriter::SetBufferSize ( int iBufferSize )
{
	if ( iBufferSize!=m_iBufferSize )
	{
		m_iBufferSize = Max ( iBufferSize, 262144 );
		m_pBuffer = nullptr;
	}
}


bool CSphWriter::OpenFile ( const CSphString & sName, CSphString & sErrorBuffer )
{
	return OpenFile ( sName, SPH_O_NEW, sErrorBuffer );
}


bool CSphWriter::OpenFile ( const CSphString & sName, int iOpenFlags, CSphString & sErrorBuffer )
{
	assert ( !sName.IsEmpty() );
	assert ( m_iFD<0 && "already open" );

	m_bOwnFile = true;
	m_sName = sName;
	m_pError = &sErrorBuffer;

	if ( !m_pBuffer )
		m_pBuffer = std::make_unique<BYTE[]> ( m_iBufferSize );

	m_iFD = ::open ( m_sName.cstr(), iOpenFlags, 0644 );
	m_pPool = m_pBuffer.get();
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_iDiskPos = 0;
	m_bError = ( m_iFD<0 );

	if ( m_bError )
		m_pError->SetSprintf ( "failed to create %s: %s" , sName.cstr(), strerror(errno) );

	return !m_bError;
}


void CSphWriter::SetFile ( CSphAutofile & tAuto, SphOffset_t * pSharedOffset, CSphString & sError )
{
	assert ( m_iFD<0 && "already open" );
	m_bOwnFile = false;

	if ( !m_pBuffer )
		m_pBuffer = std::make_unique<BYTE[]> ( m_iBufferSize );

	m_iFD = tAuto.GetFD();
	m_sName = tAuto.GetFilename();
	m_pPool = m_pBuffer.get();
	m_iPoolUsed = 0;
	m_iPos = 0;
	m_iDiskPos = 0;
	m_pSharedOffset = pSharedOffset;
	m_pError = &sError;
	assert ( m_pError );
}


CSphWriter::~CSphWriter()
{
	if ( m_bUnlinkNonClosed && m_bOwnFile )
	{
		if ( m_iFD >= 0 )
			::close ( m_iFD );
		::unlink ( m_sName.cstr() );
	} else
		CloseFile();
}


void CSphWriter::CloseFile ( bool bTruncate )
{
	if ( m_iFD>=0 )
	{
		Flush();
		if ( bTruncate )
			sphTruncate ( m_iFD );
		if ( m_bOwnFile )
			::close ( m_iFD );
		m_iFD = -1;
		m_bUnlinkNonClosed = m_bError;
	}
}


void CSphWriter::UpdatePoolUsed()
{
	if ( m_pPool-m_pBuffer.get() > m_iPoolUsed )
		m_iPoolUsed = m_pPool- m_pBuffer.get();
}


void CSphWriter::PutByte ( BYTE uValue )
{
	assert ( m_pPool );
	if ( m_iPoolUsed==m_iBufferSize )
		Flush();
	*m_pPool++ = uValue;
	UpdatePoolUsed();
	m_iPos++;
}


void CSphWriter::PutBytes ( const void * pData, int64_t iSize )
{
	assert ( m_pPool );
	const BYTE * pBuf = (const BYTE *) pData;
	while ( iSize>0 )
	{
		int iPut = ( iSize<m_iBufferSize ? int(iSize) : m_iBufferSize ); // comparison int64 to int32
		if ( m_iPoolUsed+iPut>m_iBufferSize )
			Flush();
		assert ( m_iPoolUsed+iPut<=m_iBufferSize );

		memcpy ( m_pPool, pBuf, iPut );
		m_pPool += iPut;
		UpdatePoolUsed();
		m_iPos += iPut;

		pBuf += iPut;
		iSize -= iPut;
	}
}


void CSphWriter::ZipInt ( DWORD uValue )
{
	ZipValueBE ( [this] ( BYTE b ) { PutByte ( b ); }, uValue );
}


void CSphWriter::ZipOffset ( uint64_t uValue )
{
	ZipValueBE ( [this] ( BYTE b ) { PutByte ( b ); }, uValue );
}


void CSphWriter::Flush()
{
	if ( m_pSharedOffset && *m_pSharedOffset!=m_iDiskPos )
	{
		auto uMoved = sphSeek ( m_iFD, m_iDiskPos, SEEK_SET );
		if ( uMoved!= m_iDiskPos )
		{
			m_bError = true;
			return;
		}
	}

	if ( !sphWriteThrottled ( m_iFD, m_pBuffer.get(), m_iPoolUsed, m_sName.cstr(), *m_pError ) )
		m_bError = true;

	m_iDiskPos += m_iPoolUsed;
	m_iPoolUsed = 0;
	m_pPool = m_pBuffer.get();

	if ( m_pSharedOffset )
		*m_pSharedOffset = m_iDiskPos;
}


void CSphWriterNonThrottled::Flush ()
{
	if ( m_pSharedOffset && *m_pSharedOffset!=m_iDiskPos )
	{
		auto uMoved = sphSeek ( m_iFD, m_iDiskPos, SEEK_SET );
		if ( uMoved!=m_iDiskPos )
		{
			m_bError = true;
			return;
		}
	}

	if ( !WriteNonThrottled ( m_iFD, m_pBuffer.get (), m_iPoolUsed, m_sName.cstr (), *m_pError ) )
		m_bError = true;

	m_iDiskPos += m_iPoolUsed;
	m_iPoolUsed = 0;
	m_pPool = m_pBuffer.get ();

	if ( m_pSharedOffset )
		*m_pSharedOffset = m_iDiskPos;
}


void CSphWriter::PutString ( const char * szString )
{
	int iLen = szString ? (int) strlen ( szString ) : 0;
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void CSphWriter::PutString ( const CSphString & sString )
{
	int iLen = sString.Length();
	PutDword ( iLen );
	if ( iLen )
		PutBytes ( sString.cstr(), iLen );
}


void CSphWriter::PutZString ( const char * szString )
{
	int iLen = szString ? (int) strlen ( szString ) : 0;
	ZipOffset ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void CSphWriter::PutZString ( const CSphString & sString )
{
	int iLen = sString.Length ();
	ZipOffset ( iLen );
	if ( iLen )
		PutBytes ( sString.cstr (), iLen );
}


void CSphWriter::Tag ( const char * sTag )
{
	assert ( sTag && *sTag ); // empty tags are nonsense
	assert ( strlen(sTag)<64 ); // huge tags are nonsense
	PutBytes ( sTag, strlen(sTag) );
}


bool SeekAndWarn ( int iFD, SphOffset_t iPos, const char * szWarnPrefix )
{
	assert ( szWarnPrefix );
	auto iSeek = sphSeek ( iFD, iPos, SEEK_SET );
	if ( iSeek!=iPos )
	{
		if ( iSeek<0 )
			sphWarning ( "%s : seek error. Error: %d '%s'", szWarnPrefix, errno, strerrorm (errno) );
		else
			sphWarning ( "%s : seek error. Expected: " INT64_FMT ", got " INT64_FMT, szWarnPrefix, (int64_t) iPos, (int64_t) iSeek );
		return false;
	}

	assert ( iSeek==iPos );
	return true;
}


void CSphWriter::SeekTo ( SphOffset_t iPos, bool bTruncate )
{
	assert ( iPos>=0 );

	if ( iPos>=m_iDiskPos && iPos<=( m_iDiskPos + m_iPoolUsed ) )
	{
		// seeking inside the buffer
		// m_iPoolUsed should be always in sync with m_iPos
		// or it breaks seek back at cidxHit
		m_iPoolUsed = (int)( iPos - m_iDiskPos );
		m_pPool = m_pBuffer.get() + m_iPoolUsed;
	} else
	{
		Flush();
		SeekAndWarn ( m_iFD, iPos, "CSphWriter::SeekTo" );

		if ( bTruncate )
			sphTruncate(m_iFD);

		m_pPool = m_pBuffer.get();
		m_iPoolUsed = 0;
		m_iDiskPos = iPos;
	}
	m_iPos = iPos;
}

//////////////////////////////////////////////////////////////////////////

static int g_iIOpsDelay = 0;

static const int g_iLimitIOSize = ( 1UL << 30 ); // same as write chunk limit 1GB:
// on Linux, read()/write() will transfer at most 0x7ffff000 bytes (on both 32 and 64 bit systems).
static int g_iMaxIOSize = g_iLimitIOSize;

static std::atomic<int64_t> g_tmNextIOTime { 0 };

void sphSetThrottling ( int iMaxIOps, int iMaxIOSize )
{
	g_iIOpsDelay = iMaxIOps ? 1000000 / iMaxIOps : iMaxIOps;
	g_iMaxIOSize = iMaxIOSize ? Clamp ( 1, g_iLimitIOSize, iMaxIOSize ) : g_iLimitIOSize;
}


static inline void ThrottleSleep()
{
	if ( !g_iIOpsDelay )
		return;

	auto tmTimer = sphMicroTimer();
	while ( tmTimer < g_tmNextIOTime.load ( std::memory_order_relaxed ) ) // m.b. >1 sleeps if another thread more lucky
	{
		sphSleepMsec ( (int)( g_tmNextIOTime.load ( std::memory_order_relaxed ) - tmTimer ) / 1000 );
		tmTimer = sphMicroTimer();
	}
	g_tmNextIOTime.store ( tmTimer + g_iIOpsDelay, std::memory_order_relaxed );
}


bool sphWriteThrottled ( int iFD, const void* pBuf, int64_t iCount, const char* sName, CSphString& sError )
{
	if ( iCount <= 0 )
		return true;

	// by default, slice ios by at most 1 GB
	int iChunkSize = ( 1UL << 30 );

	// when there's a sane max_iosize (4K to 1GB), use it
	if ( g_iMaxIOSize >= 4096 )
		iChunkSize = Min ( iChunkSize, g_iMaxIOSize );

	CSphIOStats* pIOStats = GetIOStats();
	int64_t iTotalWritten = 0;
	const int64_t iTotalCount = iCount;

	// while there's data, write it chunk by chunk
	auto* p = (const BYTE*)pBuf;
	while ( iCount )
	{
		// wait for a timely occasion
		ThrottleSleep();

		// write (and maybe time)
		int64_t tmTimer = 0;
		if ( pIOStats )
			tmTimer = sphMicroTimer();

		auto iToWrite = (int)Min ( iCount, iChunkSize );
		auto iWritten = (int)::write ( iFD, &p[iTotalWritten], iToWrite );

		if ( pIOStats )
		{
			pIOStats->m_iWriteTime += sphMicroTimer() - tmTimer;
			pIOStats->m_iWriteOps++;
			pIOStats->m_iWriteBytes += iWritten;
		}
		if ( sphInterrupted() && iWritten != iToWrite )
		{
			sError.SetSprintf ( "%s: write interrupted: %d of %d bytes written", sName, iWritten, iToWrite );
			return false;
		}

		// failure? report, bailout
		if ( iWritten<0 )
		{
			if ( iTotalWritten!=iTotalCount )
				sError.SetSprintf ( "%s: write error: %s", sName, strerrorm ( errno ) );
			else
				sError.SetSprintf ( "%s: write error: %s; " INT64_FMT " of " INT64_FMT " bytes written", sName, strerrorm ( errno ), iTotalWritten, iTotalCount );
			return false;
		}

		// success? rinse, repeat
		iCount -= iWritten;
		iTotalWritten += iWritten;
	}
	return true;
}


bool WriteNonThrottled ( int iFD, const void * pBuf, int64_t iCount, const char * sName, CSphString & sError )
{
	if ( iCount<=0 )
		return true;

	CSphIOStats * pIOStats = GetIOStats ();
	int64_t iTotalWritten = 0;
	const int64_t iTotalCount = iCount;

	// while there's data, write it chunk by chunk
	auto * p = (const BYTE *) pBuf;
	while ( iCount )
	{
		int64_t tmTimer = 0;
		if ( pIOStats )
			tmTimer = sphMicroTimer ();

		auto iToWrite = (int) Min ( iCount, 1UL << 30 );
		auto iWritten = (int) ::write ( iFD, &p[iTotalWritten], iToWrite );

		if ( pIOStats )
		{
			pIOStats->m_iWriteTime += sphMicroTimer ()-tmTimer;
			pIOStats->m_iWriteOps++;
			pIOStats->m_iWriteBytes += iWritten;
		}
		if ( sphInterrupted () && iWritten!=iToWrite )
		{
			sError.SetSprintf ( "%s: write interrupted: %d of %d bytes written", sName, iWritten, iToWrite );
			return false;
		}
		// failure? report, bailout
		if ( iWritten<0 )
		{
			if ( iTotalWritten!=iTotalCount )
				sError.SetSprintf ( "%s: write error: %s", sName, strerrorm ( errno ) );
			else
				sError.SetSprintf ( "%s: write error: %s; " INT64_FMT " of " INT64_FMT " bytes written", sName, strerrorm ( errno ), iTotalWritten, iTotalCount );
			return false;
		}

		// success? rinse, repeat
		iCount -= iWritten;
		iTotalWritten += iWritten;
	}
	return true;
}

size_t sphReadThrottled ( int iFD, void* pBuf, size_t iCount )
{
	if ( iCount <= 0 )
		return iCount;

	auto iStep = Min ( iCount, (size_t)g_iMaxIOSize ); // Now always 0 < g_iMaxIOSize < 1 GB
	auto* p = (BYTE*)pBuf;
	size_t nBytesToRead = iCount;
	while ( iCount && !sphInterrupted() )
	{
		ThrottleSleep();
		auto iChunk = (long)Min ( iCount, iStep );
		auto iRead = sphRead ( iFD, p, iChunk );
		p += iRead;
		iCount -= iRead;
		if ( iRead != iChunk )
			break;
	}
	return nBytesToRead - iCount; // FIXME? we sure this is under 2gb?
}

//////////////////////////////////////////////////////////////////////////

#if _WIN32

// atomic seek+read for Windows
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( iBytes==0 )
		return 0;

	CSphIOStats * pIOStats = GetIOStats();
	int64_t tmStart = 0;
	if ( pIOStats )
		tmStart = sphMicroTimer();

	HANDLE hFile;
	hFile = (HANDLE) _get_osfhandle ( iFD );
	if ( hFile==INVALID_HANDLE_VALUE )
		return -1;

	STATIC_SIZE_ASSERT ( SphOffset_t, 8 );
	OVERLAPPED tOverlapped = { 0 };
	tOverlapped.Offset = (DWORD)( iOffset & I64C(0xffffffff) );
	tOverlapped.OffsetHigh = (DWORD)( iOffset>>32 );

	DWORD uRes;
	if ( !ReadFile ( hFile, pBuf, iBytes, &uRes, &tOverlapped ) )
	{
		DWORD uErr = GetLastError();
		if ( uErr==ERROR_HANDLE_EOF )
			return 0;

		errno = uErr; // FIXME! should remap from Win to POSIX
		return -1;
	}

	if ( pIOStats )
	{
		pIOStats->m_iReadTime += sphMicroTimer() - tmStart;
		pIOStats->m_iReadOps++;
		pIOStats->m_iReadBytes += iBytes;
	}

	return uRes;
}

#else
#if HAVE_PREAD

// atomic seek+read for non-Windows systems with pread() call
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	CSphIOStats * pIOStats = GetIOStats();
	if ( !pIOStats )
		return ::pread ( iFD, pBuf, iBytes, iOffset );

	int64_t tmStart = sphMicroTimer();
	int iRes = (int) ::pread ( iFD, pBuf, iBytes, iOffset );
	if ( pIOStats )
	{
		pIOStats->m_iReadTime += sphMicroTimer() - tmStart;
		pIOStats->m_iReadOps++;
		pIOStats->m_iReadBytes += iBytes;
	}
	return iRes;
}

#else

// generic fallback; prone to races between seek and read
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset )
{
	if ( sphSeek ( iFD, iOffset, SEEK_SET )==-1 )
		return -1;

	return sphReadThrottled ( iFD, pBuf, iBytes, &g_tThrottle );
}

#endif // HAVE_PREAD
#endif // _WIN32
