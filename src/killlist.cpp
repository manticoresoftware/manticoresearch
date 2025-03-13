//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "killlist.h"

#include "sphinxint.h"

//////////////////////////////////////////////////////////////////////////

bool DeadRowMap_c::HasDead() const
{
	return m_bHaveDead;
}

DWORD DeadRowMap_c::GetNumDeads () const
{
	if ( m_iNumDeads<0 )
		m_iNumDeads = HasDead () ? CountDeads () : 0;

	return (DWORD)m_iNumDeads;
}


bool DeadRowMap_c::Set ( RowID_t tRowID, DWORD * pData )
{
	if ( tRowID==INVALID_ROWID )
		return false;

	assert ( tRowID < m_uRows );

	DWORD * pDword = pData + (tRowID>>5);
	DWORD uMask = 1UL<<( tRowID&31 );

#ifdef HAVE_SYNC_FETCH
	DWORD uPrev = __sync_fetch_and_or ( pDword, uMask );
#elif _WIN32
	DWORD uPrev = _InterlockedOr ( (long*)pDword, (long)uMask );
#else
	ScopedMutex_t tLock ( m_tLock );
	DWORD uPrev = *pDword;
	*pDword |= uMask;
#endif

	bool bSet = !( uPrev & uMask );
	m_bHaveDead |= bSet;
	if ( bSet && m_iNumDeads>=0 )
		++m_iNumDeads;

	return bSet;
}

static DWORD CountBits ( const VecTraits_T<DWORD>& dData )
{
	DWORD U = 0;
	dData.Apply ( [&U] ( DWORD uData ) { U += sphBitCount ( uData ); } );
	return U;
}

void DeadRowMap_c::CheckForDead ( const DWORD * pData, const DWORD * pDataEnd )
{
	m_bHaveDead = false;
	while ( pData<pDataEnd && !m_bHaveDead )
		m_bHaveDead |= !!*pData++;

	if ( !m_bHaveDead )
		m_iNumDeads = 0;
	else if ( pData==pDataEnd )
		m_iNumDeads = sphBitCount ( *( pData-1 ) );
}


//////////////////////////////////////////////////////////////////////////


DeadRowMap_Ram_c::DeadRowMap_Ram_c ( DWORD uRows )
{
	Reset ( uRows );
}


bool DeadRowMap_Ram_c::Set ( RowID_t tRowID )
{
	return DeadRowMap_c::Set ( tRowID, m_dData.Begin() );
}


bool DeadRowMap_Ram_c::IsSet ( RowID_t tRowID ) const
{
	return DeadRowMap_c::IsSet ( tRowID, m_dData.Begin() );
}


int64_t DeadRowMap_Ram_c::GetLengthBytes() const
{
	return m_dData.GetLengthBytes64();
}

uint64_t DeadRowMap_Ram_c::GetCoreSize () const
{
	return m_dData.GetLengthBytes64 ();
}

void DeadRowMap_Ram_c::Reset ( DWORD uRows )
{
	m_uRows = uRows;
	m_dData.Reset ( (uRows+31)/32 );
	m_dData.Fill(0);
	m_bHaveDead = false;
	m_iNumDeads = 0;
}


void DeadRowMap_Ram_c::Load ( DWORD uRows, CSphReader & tReader, CSphString & sError )
{
	m_uRows = uRows;
	m_dData.Reset ( (m_uRows+31)/32 );
	if ( uRows & 0x1F )
		m_dData[m_dData.GetLength()-1] = 0; // ensure tail bits after the end are zeroed
	tReader.GetBytes ( m_dData.Begin(), m_dData.GetLength()*sizeof(m_dData[0]) );
	m_iNumDeads = -1;
	CheckForDead ( m_dData.Begin(), m_dData.Begin()+m_dData.GetLength() );
}


void DeadRowMap_Ram_c::Save ( CSphWriter & tWriter ) const
{
	tWriter.PutBytes ( m_dData.Begin(), m_dData.GetLength()*sizeof(m_dData[0]) );
}

DWORD DeadRowMap_Ram_c::CountDeads () const
{
	return CountBits ( m_dData );
}

DWORD DeadRowMap_Ram_c::GetNumAlive() const
{
	return m_uRows-GetNumDeads ();
}



//////////////////////////////////////////////////////////////////////////


DeadRowMap_Disk_c::~DeadRowMap_Disk_c()
{
	CSphString sError;
	Flush ( true, sError );
}


bool DeadRowMap_Disk_c::Set ( RowID_t tRowID )
{
	return DeadRowMap_c::Set ( tRowID, m_tData.GetWritePtr() );
}


bool DeadRowMap_Disk_c::Flush ( bool bWaitComplete, CSphString & sError ) const
{
	return m_tData.Flush ( bWaitComplete, sError );
}


bool DeadRowMap_Disk_c::Prealloc ( DWORD uRows, const CSphString & sFilename, CSphString & sError )
{
	// we'll reset this flag after preread
	m_bHaveDead = true;
	m_uRows = uRows;
	return m_tData.Setup ( sFilename.cstr(), sError, true );
}


void DeadRowMap_Disk_c::Preread ( const char * sIndexName, const char * sFor, bool bMlock )
{
	m_iNumDeads = PrereadMappingCountingBits ( sIndexName, sFor, bMlock, false, m_tData );
	m_bHaveDead = m_iNumDeads>0;
}


void DeadRowMap_Disk_c::Dealloc()
{
	m_tData.Reset();
}


int64_t DeadRowMap_Disk_c::GetLengthBytes() const
{
	return m_tData.GetLengthBytes64();
}

uint64_t DeadRowMap_Disk_c::GetCoreSize () const
{
	return m_tData.GetCoreSize();
}

DWORD DeadRowMap_Disk_c::CountDeads () const
{
	return CountBits ( m_tData );
}

//////////////////////////////////////////////////////////////////////////

bool WriteDeadRowMap ( const CSphString & sFilename, DWORD uTotalDocs, CSphString & sError )
{
	// empty dead row map
	CSphWriter tRowMapWriter;
	if ( !tRowMapWriter.OpenFile ( sFilename, sError ) )
		return false;

	int nEntries = int(( uTotalDocs+31 ) / 32);
	for ( int i=0; i < nEntries; ++i )
		tRowMapWriter.PutDword(0);

	tRowMapWriter.CloseFile();
	if ( tRowMapWriter.IsError() )
	{
		sError.SetSprintf ( "error writing row map to %s", sFilename.cstr() );
		return false;
	}

	return true;
}
