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

#include <algorithm>

namespace
{

class TermProducer_c : public TermProducer_i
{
public:
	TermProducer_c ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
		: m_tWord(tWord)
		, m_tSetup(tSetup)
		, m_dDocs(tSetup.m_iCapacity)
	{
		SetPayloadRequirements(tSetup.m_tPayloads);
	}

	void SetPayloadRequirements ( TermPayloadRequirements_t tRequirements ) final
	{
		m_tSetup.m_tPayloads = tRequirements;
		m_tWord.SetPayloadRequirements ( tRequirements.m_bTF || tRequirements.m_bHits, tRequirements.m_bHits );
	}

protected:
	ISphQword &			m_tWord;
	TermProducerSetup_t	m_tSetup;
	CSphFixedVector<QwordDoc_t> m_dDocs;
	int					m_iCount = 0;

	bool CheckContinue() const
	{
		const auto & tContext = m_tSetup.m_tContext;
		return !tContext.m_fnContinue || tContext.m_fnContinue(tContext.m_pContext);
	}
};

template<bool WIDE_FIELDS, bool ROWID_LIMITS>
class QwordTermProducer_T final : public TermProducer_c
{
public:
			QwordTermProducer_T ( ISphQword & tWord, const TermProducerSetup_t & tSetup ) : TermProducer_c ( tWord, tSetup ) {}

	void	HintRowID ( RowID_t tMin ) final { m_tWord.HintRowID(tMin); }

	TermCandidateBatch_t NextCandidates() final
	{
		m_iCount = 0;
		int iReads = 0;
		while ( m_tWord.m_iDocs && m_iCount<m_dDocs.GetLength() )
		{
			if ( (!WIDE_FIELDS || !(iReads++ & 127)) && !CheckContinue() )
				break;

			if constexpr ( WIDE_FIELDS )
				ReadWideRow();
			else
				ReadBatch();
		}

		return { nullptr, m_dDocs.Begin(), m_iCount };
	}

	const QwordDoc_t * ReadPayloads ( const RowID_t * pRows, int iCount ) final
	{
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

	void ReadBatch()
	{
		int iFirst = m_iCount;
		int iWant = m_dDocs.GetLength()-iFirst;
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


class RestrictedBlockTermProducer_c final : public TermProducer_c
{
public:
	RestrictedBlockTermProducer_c ( ISphQword & tWord, const TermProducerSetup_t & tSetup, std::unique_ptr<PostingBlock_i> pCursor )
		: TermProducer_c ( tWord, tSetup )
		, m_tTerm ( std::move(pCursor), *tSetup.m_pFilter, tSetup.m_tContext )
	{}

	void HintRowID ( RowID_t tMin ) final { m_tHint = std::max ( m_tHint, tMin ); }

	TermCandidateBatch_t NextCandidates() final
	{
		m_tRows = m_tTerm.ReadRows ( std::max ( m_tHint, m_tSetup.m_tMin ), m_tSetup.m_uEnd, m_dDocs.GetLength() );
		if ( !m_tRows.m_iCount )
			m_tWord.m_iDocs = 0;

		return { m_tRows.m_pRows, nullptr, m_tRows.m_iCount };
	}

	const QwordDoc_t * ReadPayloads ( const RowID_t * pRows, int iCount ) final
	{
		if ( !pRows )
		{
			pRows = m_tRows.m_pRows;
			iCount = m_tRows.m_iCount;
		}

		assert ( iCount>=0 && iCount<=m_tRows.m_iCount );

		const auto & tPayloads = m_tSetup.m_tPayloads;
		if ( !m_tTerm.GetCursor().ReadPayloads ( pRows, iCount, m_dDocs.Begin(), tPayloads.m_bTF || tPayloads.m_bHits, tPayloads.m_bHits ) )
		{
			m_tWord.m_iDocs = 0;
			return nullptr;
		}

		return m_dDocs.Begin();
	}

private:
	PostingBlockTerm_c	m_tTerm;
	PostingCandidateRows_t m_tRows;
	RowID_t				m_tHint = 0;
};

}


TermReadMode_e PlanTermProducer ( const TermProducerSetup_t & tSetup )
{
	if ( tSetup.m_pFilter )
		return tSetup.m_bWideFields ? TermReadMode_e::UNSUPPORTED : TermReadMode_e::RESTRICTED_BLOCK;

	return tSetup.m_bWideFields ? TermReadMode_e::WIDE_FIELDS : TermReadMode_e::BATCHED;
}


std::unique_ptr<TermProducer_i> CreateTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
{
	assert ( tSetup.m_iCapacity>0 );
	switch ( PlanTermProducer(tSetup) )
	{
	case TermReadMode_e::BATCHED:
		if ( tSetup.m_tMin || tSetup.m_uEnd!=INVALID_ROWID )
			return std::make_unique<QwordTermProducer_T<false,true>> ( tWord, tSetup );

		return std::make_unique<QwordTermProducer_T<false,false>> ( tWord, tSetup );

	case TermReadMode_e::WIDE_FIELDS:
		if ( tSetup.m_tMin || tSetup.m_uEnd!=INVALID_ROWID )
			return std::make_unique<QwordTermProducer_T<true,true>> ( tWord, tSetup );

		return std::make_unique<QwordTermProducer_T<true,false>> ( tWord, tSetup );

	case TermReadMode_e::RESTRICTED_BLOCK:
		{
			auto pCursor = tWord.CreatePostingBlock ( tSetup.m_tFields.GetMask32(), tSetup.m_tContext );
			if ( !pCursor )
				return nullptr;

			return std::make_unique<RestrictedBlockTermProducer_c> ( tWord, tSetup, std::move(pCursor) );
		}

	case TermReadMode_e::UNSUPPORTED:
		return nullptr;
	}

	assert(0);
	return nullptr;
}
