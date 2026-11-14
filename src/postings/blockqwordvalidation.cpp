// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "blockqwordvalidation.h"
#include "diskqword.h"
#include "disk.h"
#include <algorithm>

BlockIndexQwordValidation_c::BlockIndexQwordValidation_c() : DiskIndexQwordTraits_c ( false, false ), m_tReader(*this) {}


void BlockIndexQwordValidation_c::OnDocReaderChanged()
{
	SphOffset_t iSize = 0;
	m_pDocMap = m_rdDoclist ? m_rdDoclist->GetMapBase(iSize) : nullptr;
}


void BlockIndexQwordValidation_c::Reset()
{
	ResetDecoderState();
	ResetCursor();
}


void BlockIndexQwordValidation_c::ResetCursor()
{
	m_tDoc.m_tRowID = INVALID_ROWID;
	m_bHitEOF = true;
	m_bOpen = false;
	m_bEOF = false;
	m_iOrdinal = 0;
	m_sError.clear();
	m_tValidation = {};
}


bool BlockIndexQwordValidation_c::Setup ( const DiskIndexQwordSetup_c * pSetup )
{
	m_pQueryError = pSetup->m_pError;
	return pSetup->Setup(this);
}


void BlockIndexQwordValidation_c::SeekDocReader ( SphOffset_t iPos, int iHint )
{
	ResetCursor();
	m_iStart = m_iNext = iPos==1 ? postings::FILE_HEADER_SIZE : iPos;
	m_rdDoclist->SeekTo ( m_iStart, iHint );
}


void BlockIndexQwordValidation_c::SeekHitReader ( SphOffset_t iPos, int iHint )
{
	m_rdHitlist->SeekTo ( iPos==1 ? postings::FILE_HEADER_SIZE : iPos, iHint );
}


SphOffset_t BlockIndexQwordValidation_c::GetDocReaderPos() const
{
	return m_iNext;
}


bool BlockIndexQwordValidation_c::Read ( uint64_t uOffset, uint8_t * pData, size_t iSize, std::string & sError )
{
	if ( uOffset>GetSize() || iSize>GetSize()-uOffset || iSize>INT_MAX )
	{
		sError = "posting read outside file";
		return false;
	}

	assert ( uOffset<=GetSize() && iSize<=GetSize()-uOffset && iSize<=INT_MAX );
	m_rdDoclist->SeekTo ( uOffset, int(iSize) );
	m_rdDoclist->GetBytes ( pData, int(iSize) );
	if ( const char * sReadError = m_rdDoclist->GetError() )
	{
		sError = sReadError;
		return false;
	}

	return true;
}


void BlockIndexQwordValidation_c::Fail()
{
	m_bEOF = true;
	m_tDoc.m_tRowID = INVALID_ROWID;
	if ( m_pQueryError )
		m_pQueryError->SetSprintf ( "block postings: %s", m_sError.c_str() );
}


bool BlockIndexQwordValidation_c::LoadLeaf()
{
	if ( !m_bOpen )
	{
		if ( !m_tReader.Open ( m_iStart, m_sError ) )
		{
			Fail();
			return false;
		}

		m_bOpen = true;
	}

	if ( !m_tReader.Next ( 0, m_sError ) )
	{
		Fail();
		return false;
	}

	if ( m_tReader.IsEOF() )
	{
		m_bEOF = true;
		m_iNext = m_tReader.GetEnd();
		return false;
	}

	if ( !ValidateLeaf() )
	{
		Fail();
		return false;
	}

	m_iNext = m_tReader.GetLeaf().m_uOffset+m_tReader.GetLeaf().m_uBytes;
	m_iOrdinal = 0;
	return true;
}


void BlockIndexQwordValidation_c::SetDoc ( unsigned i )
{
	m_tDoc.m_tRowID = m_tReader.GetRows()[i];
	for ( unsigned f=0; f<postings::FIELD_WORDS; ++f )
		m_dQwordFields[f] = m_tReader.GetFields()[i][f];

	m_bAllFieldsKnown = true;
	m_uMatchHits = m_tReader.GetTF()[i];
	m_iHitlistPos = m_uHitPosition = m_tReader.GetHitRefs()[i];
}


const CSphMatch & BlockIndexQwordValidation_c::GetNextDoc()
{
	if ( m_bEOF || ((!m_bOpen || m_iOrdinal>=m_tReader.GetLeaf().m_uCount) && !LoadLeaf()) )
		m_tDoc.m_tRowID = INVALID_ROWID;
	else
		SetDoc(m_iOrdinal++);

	return m_tDoc;
}


void BlockIndexQwordValidation_c::SeekHitlist ( SphOffset_t uOffset )
{
	m_iHitPos = EMPTY_HIT;
	m_bHitEOF = true;
	m_tValidation.m_dHitTF.fill(0);

	if ( !m_bHasHitlist )
		return;

	if ( uOffset<postings::FILE_HEADER_SIZE || uOffset>=m_pHitFactory->GetFilesize() )
	{
		m_sError = "external hit reference outside file";
		Fail();
		return;
	}

	assert ( uOffset>=postings::FILE_HEADER_SIZE && uOffset<m_pHitFactory->GetFilesize() );
	m_rdHitlist->SeekTo ( uOffset, READ_NO_SIZE_HINT );
	m_bHitEOF = false;
}


Hitpos_t BlockIndexQwordValidation_c::GetNextHit()
{
	if ( m_bHitEOF || !m_bHasHitlist || !m_sError.empty() )
		return EMPTY_HIT;

	DWORD uDelta;
	if ( !ReadCheckedHit(uDelta) )
		return EMPTY_HIT;

	if ( !uDelta )
	{
		m_bHitEOF = true;
		if ( !ValidateHits() )
			Fail();

		return EMPTY_HIT;
	}

	if ( uDelta>UINT32_MAX-m_iHitPos )
	{
		m_sError = "external hit delta overflow";
		Fail();
		return EMPTY_HIT;
	}

	assert ( uDelta<=UINT32_MAX-m_iHitPos );
	m_iHitPos += uDelta;
	++m_tValidation.m_dHitTF[HITMAN::GetField(m_iHitPos)];

	return m_iHitPos;
}


bool BlockIndexQwordValidation_c::ReadCheckedHit ( DWORD & uDelta )
{
	if ( m_rdHitlist->GetPos()>=m_pHitFactory->GetFilesize() )
	{
		m_sError = "unterminated external hit list";
		Fail();
		return false;
	}

	uDelta = 0;
	for ( unsigned i=0; ; ++i )
	{
		if ( i==5 || m_rdHitlist->GetPos()>=m_pHitFactory->GetFilesize() || uDelta>(UINT32_MAX>>7) )
		{
			m_sError = "invalid external hit varint";
			Fail();
			return false;
		}

		BYTE uByte = m_rdHitlist->GetByte();
		if ( const char * sError = m_rdHitlist->GetError() )
		{
			m_sError = sError;
			Fail();
			return false;
		}

		uDelta = (uDelta<<7) | (uByte&127);
		if ( !(uByte&128) )
			break;
	}

	return true;
}


bool BlockIndexQwordValidation_c::ValidateLeaf()
{
	if ( !m_tReader.Validate(m_sError) )
		return false;

	auto & t = m_tValidation;
	t.m_dLeaves.push_back(m_tReader.GetLeaf());

	return true;
}


bool BlockIndexQwordValidation_c::ValidateHits()
{
	uint64_t uTotal = 0;
	for ( unsigned f=0; f<postings::MAX_FIELDS; ++f )
	{
		bool bExpected = (m_tReader.GetFields()[m_iOrdinal-1][f/32] & (1U<<(f%32)))!=0;
		if ( bExpected!=bool(m_tValidation.m_dHitTF[f]) )
		{
			m_sError = "posting field mask/hit list mismatch";
			return false;
		}

		uTotal += m_tValidation.m_dHitTF[f];
	}

	if ( uTotal!=m_tReader.GetTF()[m_iOrdinal-1] )
	{
		m_sError = "posting frequency/hit list mismatch";
		return false;
	}

	return true;
}


bool BlockIndexQwordValidation_c::CheckDirectory ( CSphReader & tSkips, SphOffset_t iOffset, int iFields )
{
	if ( !m_sError.empty() )
		return false;

	if ( m_tReader.GetDesc().m_uFields!=iFields || m_tReader.GetDesc().m_bPositions!=m_bHasHitlist )
	{
		m_sError = "posting capabilities/schema mismatch";
		return false;
	}

	if ( !iOffset )
		return true;

	const auto & dLeaves = m_tValidation.m_dLeaves;
	if ( iOffset<postings::FILE_HEADER_SIZE || iOffset>tSkips.GetFilesize() || uint64_t(tSkips.GetFilesize()-iOffset)<8+20*uint64_t(dLeaves.size()) )
	{
		m_sError = "posting directory outside file";
		return false;
	}

	tSkips.SeekTo ( iOffset, READ_NO_SIZE_HINT );
	char dMagic[4];
	tSkips.GetBytes ( dMagic, sizeof(dMagic) );
	if ( memcmp ( dMagic, "SKP1", 4 ) || tSkips.GetDword()!=dLeaves.size() )
	{
		m_sError = "posting directory header/count mismatch";
		return false;
	}

	for ( const auto & tLeaf : dLeaves )
	{
		DWORD uFirst = tSkips.GetDword();
		DWORD uLast = tSkips.GetDword();
		SphOffset_t iLeaf = tSkips.GetOffset();
		DWORD uMaxTF = tSkips.GetDword();
		if ( uFirst!=tLeaf.m_uFirst || uLast!=tLeaf.m_uLast || uint64_t(iLeaf)!=tLeaf.m_uOffset || uMaxTF!=tLeaf.m_uMaxTF )
		{
			m_sError = "posting directory/leaf mismatch";
			return false;
		}
	}

	if ( tSkips.GetErrorFlag() )
	{
		m_sError = tSkips.GetErrorMessage().cstr();
		return false;
	}

	return true;
}
