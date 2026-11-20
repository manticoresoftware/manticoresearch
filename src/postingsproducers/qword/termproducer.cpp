//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "termproducer.h"
#include "../termproducerbase.h"

#include <algorithm>

namespace
{

template<bool WIDE_FIELDS, bool ROWID_LIMITS>
class QwordTermProducer_T final : public TermProducerBase_c
{
public:
			QwordTermProducer_T ( ISphQword & tWord, const TermProducerSetup_t & tSetup ) : TermProducerBase_c ( tWord, tSetup ) {}


	void	HintRowID ( RowID_t tMin ) final { m_tWord.HintRowID(tMin); }

	PostingCandidateBatch_t NextCandidates ( int iCapacity ) final
	{
		assert ( iCapacity>0 && iCapacity<=m_dDocs.GetLength() );
		m_iCount = 0;
		int iReads = 0;
		while ( m_tWord.m_iDocs && m_iCount<iCapacity )
		{
			if ( (!WIDE_FIELDS || !(iReads++ & 127)) && !CheckContinue() )
				break;

			if constexpr ( WIDE_FIELDS )
				ReadWideRow();
			else
				ReadBatch(iCapacity);
		}

		return { nullptr, m_dDocs.Begin(), m_iCount };
	}


	const QwordDoc_t * ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount ) final
	{
		assert ( iTerm==0 );
		if ( !pRows )
			return m_dDocs.Begin();

		assert ( iCount>=0 && iCount<=m_iCount );
		int iRead = 0;
		for ( int i=0; i<iCount; ++i )
		{
			assert ( !i || pRows[i]>pRows[i-1] );

			while ( iRead<m_iCount && m_dDocs[iRead].m_tRowID<pRows[i] )
				++iRead;

			assert ( iRead<m_iCount && m_dDocs[iRead].m_tRowID==pRows[i] );

			if ( i!=iRead )
				m_dDocs[i] = m_dDocs[iRead];

			++iRead;
		}

		return m_dDocs.Begin();
	}

private:
	bool AcceptRow ( RowID_t tRow )
	{
		if ( tRow==INVALID_ROWID )
		{
			m_tWord.m_iDocs = 0;
			return false;
		}

		if constexpr ( ROWID_LIMITS )
		{
			if ( tRow<m_tSetup.m_tMin )
				return false;

			if ( tRow>=m_tSetup.m_uEnd )
			{
				m_tWord.m_iDocs = 0;
				return false;
			}
		}

		return true;
	}


	void ReadBatch ( int iCapacity )
	{
		int iFirst = m_iCount;
		int iWant = iCapacity-iFirst;
		int iGot = m_tWord.GetNextDocs ( m_dDocs.Begin()+iFirst, iWant );
		DWORD uFields = m_tSetup.m_tFields.GetMask32();
		int iCount = iFirst;
		bool bDone = iGot<iWant;
		for ( int i = iFirst; i<iFirst+iGot; ++i )
		{
			const auto & tDoc = m_dDocs[i];
			if constexpr ( ROWID_LIMITS )
			{
				if ( tDoc.m_tRowID<m_tSetup.m_tMin )
					continue;

				if ( tDoc.m_tRowID>=m_tSetup.m_uEnd )
				{
					bDone = true;
					break;
				}
			}

			if ( !(tDoc.m_uFields & uFields) )
				continue;

			if ( i!=iCount )
				m_dDocs[iCount] = tDoc;

			++iCount;
		}

		m_iCount = iCount;
		if ( bDone )
			m_tWord.m_iDocs = 0;
	}


	void ReadWideRow()
	{
		RowID_t tRow = m_tWord.GetNextDoc().m_tRowID;
		if ( !AcceptRow(tRow) )
			return;

		m_tWord.CollectHitMask();

		bool bMatch = false;
		for ( int i=0; i<FieldMask_t::SIZE && !bMatch; ++i )
			bMatch = (m_tWord.m_dQwordFields[i] & m_tSetup.m_tFields[i])!=0;

		if ( bMatch )
			m_dDocs[m_iCount++] = { tRow, m_tWord.m_dQwordFields.GetMask32(), m_tWord.m_uMatchHits, m_tWord.m_iHitlistPos };
	}
};

}


std::unique_ptr<PostingProducer_i> CreateQwordTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
{
	if ( tSetup.m_bWideFields )
	{
		if ( tSetup.m_tMin || tSetup.m_uEnd!=INVALID_ROWID )
			return std::make_unique<QwordTermProducer_T<true,true>> ( tWord, tSetup );

		return std::make_unique<QwordTermProducer_T<true,false>> ( tWord, tSetup );
	}

	if ( tSetup.m_tMin || tSetup.m_uEnd!=INVALID_ROWID )
		return std::make_unique<QwordTermProducer_T<false,true>> ( tWord, tSetup );

	return std::make_unique<QwordTermProducer_T<false,false>> ( tWord, tSetup );
}
