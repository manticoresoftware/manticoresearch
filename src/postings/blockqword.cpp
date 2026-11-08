// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "blockqword.h"
#include "diskqword.h"
#include "disk.h"
#include <algorithm>

BlockIndexQword_c::BlockIndexQword_c ( bool bMini, bool bExcluded, int64_t ) : DiskIndexQwordTraits_c ( bMini, bExcluded ), m_tReader(*this) {}


void BlockIndexQword_c::OnDocReaderChanged()
{
	SphOffset_t iSize = 0;
	m_pDocMap = m_rdDoclist ? m_rdDoclist->GetMapBase(iSize) : nullptr;
}


void BlockIndexQword_c::Reset()
{
	ResetDecoderState();
	ResetCursor();
}


void BlockIndexQword_c::ResetCursor()
{
	m_tDoc.m_tRowID = INVALID_ROWID;
	m_bHitEOF = true;
	m_bOpen = false;
	m_bEOF = false;
	m_iOrdinal = 0;
	m_tHint = 0;
	m_sError.clear();
}


bool BlockIndexQword_c::Setup ( const DiskIndexQwordSetup_c * pSetup )
{
	m_pQueryError = pSetup->m_pError;
	return pSetup->Setup(this);
}


void BlockIndexQword_c::SeekDocReader ( SphOffset_t iPos, int iHint )
{
	ResetCursor();
	m_iStart = m_iNext = iPos==1 ? postings::FILE_HEADER_SIZE : iPos;
	m_rdDoclist->SeekTo ( m_iStart, iHint );
}


void BlockIndexQword_c::SeekHitReader ( SphOffset_t iPos, int iHint )
{
	m_rdHitlist->SeekTo ( iPos==1 ? postings::FILE_HEADER_SIZE : iPos, iHint );
}


SphOffset_t BlockIndexQword_c::GetDocReaderPos() const
{
	return m_iNext;
}


bool BlockIndexQword_c::Read ( uint64_t uOffset, uint8_t * pData, size_t iSize, std::string & sError )
{
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


void BlockIndexQword_c::Fail()
{
	m_bEOF = true;
	m_tDoc.m_tRowID = INVALID_ROWID;
	if ( m_pQueryError )
		m_pQueryError->SetSprintf ( "block postings: %s", m_sError.c_str() );
}


bool BlockIndexQword_c::LoadLeaf ( RowID_t tMin )
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

	if ( !m_tReader.Next ( tMin, m_sError ) )
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

	if ( !m_tReader.ReadRows(m_sError) || !m_tReader.ReadFields(m_sError) || (m_bReadTF && !m_tReader.ReadTF(m_sError)) || (m_bReadHits && !m_tReader.ReadHitRefs(m_sError)) )
	{
		Fail();
		return false;
	}

	m_iNext = m_tReader.GetLeaf().m_uOffset+m_tReader.GetLeaf().m_uBytes;
	m_iOrdinal = 0;
	return true;
}


bool BlockIndexQword_c::PositionNext()
{
	while ( !m_bEOF )
	{
		if ( !m_bOpen || m_iOrdinal>=m_tReader.GetLeaf().m_uCount )
			if ( !LoadLeaf(m_tHint) )
				break;

		unsigned iEnd = m_tReader.GetLeaf().m_uCount;
		const auto & dRows = m_tReader.GetRows();
		if ( m_tHint )
		{
			m_iOrdinal = unsigned(std::lower_bound ( dRows.begin()+m_iOrdinal, dRows.begin()+iEnd, m_tHint )-dRows.begin());
			if ( m_iOrdinal==iEnd )
				continue;
		}

		m_tHint = 0;
		return true;
	}

	m_tDoc.m_tRowID = INVALID_ROWID;
	return false;
}


void BlockIndexQword_c::SetDoc ( unsigned i )
{
	m_tDoc.m_tRowID = m_tReader.GetRows()[i];
	for ( unsigned f=0; f<postings::FIELD_WORDS; ++f )
		m_dQwordFields[f] = m_tReader.GetFields()[i][f];

	m_bAllFieldsKnown = true;
	m_uMatchHits = m_bReadTF ? m_tReader.GetTF()[i] : 0;
	m_iHitlistPos = m_uHitPosition = m_bReadHits ? m_tReader.GetHitRefs()[i] : 0;
}


const CSphMatch & BlockIndexQword_c::GetNextDoc()
{
	if ( PositionNext() )
		SetDoc(m_iOrdinal++);

	return m_tDoc;
}


template<bool TF, bool HITS>
int BlockIndexQword_c::ReadDocs ( QwordDoc_t * pDocs, int iMax )
{
	int iGot = 0;
	while ( iGot<iMax && PositionNext() )
	{
		const auto & dRows = m_tReader.GetRows();
		const auto & dFields = m_tReader.GetFields();
		const auto & dTF = m_tReader.GetTF();
		const auto & dHitRefs = m_tReader.GetHitRefs();
		unsigned iEnd = m_iOrdinal+std::min ( unsigned(iMax-iGot), unsigned(m_tReader.GetLeaf().m_uCount)-m_iOrdinal );
		for ( unsigned i=m_iOrdinal; i<iEnd; ++i )
		{
			QwordDoc_t & tDoc = pDocs[iGot++];
			tDoc.m_tRowID = dRows[i];
			tDoc.m_uFields = dFields[i][0];
			tDoc.m_uMatchHits = TF ? dTF[i] : 0;
			tDoc.m_iHitlistPos = HITS ? dHitRefs[i] : 0;
		}

		m_iOrdinal = iEnd;
		SetDoc(iEnd-1);
	}

	return iGot;
}


int BlockIndexQword_c::GetNextDocs ( QwordDoc_t * pDocs, int iMax )
{
	if ( m_bReadHits )
		return ReadDocs<true,true> ( pDocs, iMax );

	return m_bReadTF ? ReadDocs<true,false> ( pDocs, iMax ) : ReadDocs<false,false> ( pDocs, iMax );
}


bool BlockIndexQword_c::HintRowID ( RowID_t tRow )
{
	m_tHint = std::max ( m_tHint, tRow );
	return true;
}


RowID_t BlockIndexQword_c::AdvanceTo ( RowID_t tRow )
{
	if ( m_tDoc.m_tRowID!=INVALID_ROWID && m_tDoc.m_tRowID>=tRow )
		return m_tDoc.m_tRowID;

	HintRowID(tRow);
	return GetNextDoc().m_tRowID;
}


void BlockIndexQword_c::SeekHitlist ( SphOffset_t uOffset )
{
	m_iHitPos = EMPTY_HIT;
	m_bHitEOF = true;

	if ( !m_bHasHitlist )
		return;

	assert ( uOffset>=postings::FILE_HEADER_SIZE && uOffset<m_pHitFactory->GetFilesize() );
	m_rdHitlist->SeekTo ( uOffset, READ_NO_SIZE_HINT );
	m_bHitEOF = false;
}


Hitpos_t BlockIndexQword_c::GetNextHit()
{
	if ( m_bHitEOF || !m_bHasHitlist || !m_sError.empty() )
		return EMPTY_HIT;

	assert ( m_rdHitlist->GetPos()<m_pHitFactory->GetFilesize() );
	DWORD uDelta = m_rdHitlist->UnzipInt();
	if ( const char * sError = m_rdHitlist->GetError() )
	{
		m_sError = sError;
		Fail();
		return EMPTY_HIT;
	}

	if ( !uDelta )
	{
		m_bHitEOF = true;
		return EMPTY_HIT;
	}

	assert ( uDelta<=UINT32_MAX-m_iHitPos );
	m_iHitPos += uDelta;
	return m_iHitPos;
}
