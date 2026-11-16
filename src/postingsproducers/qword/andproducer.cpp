//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "andproducer.h"
#include "../andproducerbase.h"

#include <algorithm>

namespace
{

template<bool ROWID_LIMITS, bool WIDE_FIELDS>
class QwordAndProducer_T final : public AndProducerBase_c
{
public:
	explicit QwordAndProducer_T ( const AndProducerSetup_t & tSetup )
		: AndProducerBase_c(tSetup)
		, m_dTerms(tSetup.m_dTerms.size())
		, m_dRows(tSetup.m_iCapacity)
		, m_dPayloads ( tSetup.m_dTerms.size()*tSetup.m_iCapacity )
	{
		for ( size_t i=0; i<m_dTerms.size(); ++i )
		{
			const auto & tTerm = tSetup.m_dTerms[i];
			m_dTerms[i] = { tTerm.m_pQword, m_dPayloads.data()+i*tSetup.m_iCapacity, tTerm.m_bWideFields ? &m_tSetup.m_dTerms[i].m_tFields : nullptr, INVALID_ROWID, tTerm.m_tFields.GetMask32() };
		}
	}


	PostingCandidateBatch_t NextCandidates ( int iCapacity ) final
	{
		assert ( iCapacity>0 && iCapacity<=m_tSetup.m_iCapacity );
		m_iCount = 0;
		int iCount = 0;
		if ( m_bEOF || m_tHint>=m_tSetup.m_uEnd || ( m_tSetup.m_tContext.m_fnContinue && !m_tSetup.m_tContext.m_fnContinue(m_tSetup.m_tContext.m_pContext) ) )
			return {};

		if ( m_bFirst || ( m_dTerms[0].m_tRowID!=INVALID_ROWID && m_dTerms[0].m_tRowID<m_tHint ) )
		{
			Advance ( 0, m_tHint );
			m_bFirst = false;
		}

		while ( iCount<iCapacity && m_dTerms[0].m_tRowID!=INVALID_ROWID )
		{
			if ( !Intersect() )
				break;

			RowID_t tRow = m_dTerms[0].m_tRowID;
			m_dRows[iCount] = tRow;
			for ( const auto & tTerm : m_dTerms )
			{
				const auto & tWord = *tTerm.m_pQword;
				tTerm.m_pPayloads[iCount] = { tRow, tWord.m_dQwordFields.GetMask32(), tWord.m_uMatchHits, tWord.m_iHitlistPos };
			}

			++iCount;
			Next(0);
		}

		m_bEOF = m_dTerms[0].m_tRowID==INVALID_ROWID;
		if ( m_tSetup.m_tContext.m_pStats )
			m_tSetup.m_tContext.m_pStats->m_iFetchedDocs += iCount;

		m_iCount = iCount;
		return { m_dRows.data(), nullptr, iCount };
	}


	const QwordDoc_t * ReadPayloads ( int iTerm, const RowID_t * pRows, int iCount ) final
	{
		assert ( iTerm>=0 && iTerm<int(m_tSetup.m_dTerms.size()) );
		auto pDocs = m_dTerms[iTerm].m_pPayloads;
		if ( !pRows )
			return pDocs;

		assert ( iCount>=0 && iCount<=m_iCount );

		int iRead = 0;
		for ( int i=0; i<iCount; ++i )
		{
			assert ( !i || pRows[i]>pRows[i-1] );
			while ( iRead<m_iCount && pDocs[iRead].m_tRowID<pRows[i] )
				++iRead;

			assert ( iRead<m_iCount && pDocs[iRead].m_tRowID==pRows[i] );
			if ( i!=iRead )
				pDocs[i] = pDocs[iRead];

			++iRead;
		}

		return pDocs;
	}

private:
	struct Term_t
	{
		ISphQword *		m_pQword;
		QwordDoc_t *	m_pPayloads;
		const FieldMask_t * m_pWideFields;
		RowID_t			m_tRowID;
		DWORD			m_uFields;
	};

	std::vector<Term_t>		m_dTerms;
	std::vector<RowID_t>	m_dRows;
	std::vector<QwordDoc_t>	m_dPayloads;
	int						m_iCount = 0;
	bool					m_bFirst = true;
	bool					m_bEOF = false;
	unsigned				m_uChecksLeft = 32;

	FORCE_INLINE bool PassesFieldCheck ( int iTerm )
	{
		const auto & tTerm = m_dTerms[iTerm];
		auto & tWord = *tTerm.m_pQword;
		if constexpr ( WIDE_FIELDS )
		{
			if ( tTerm.m_pWideFields )
			{
				tWord.CollectHitMask();
				for ( int i=0; i<FieldMask_t::SIZE; ++i )
					if ( tWord.m_dQwordFields[i] & (*tTerm.m_pWideFields)[i] )
						return true;

				return false;
			}
		}

		return ( tWord.m_dQwordFields.GetMask32() & tTerm.m_uFields )!=0;
	}


	FORCE_INLINE bool CheckContinue()
	{
		if ( --m_uChecksLeft )
			return true;

		m_uChecksLeft = 32;
		const auto & tContext = m_tSetup.m_tContext;
		return !tContext.m_fnContinue || tContext.m_fnContinue(tContext.m_pContext);
	}


	FORCE_INLINE RowID_t CheckCandidate ( int iTerm, RowID_t tRow )
	{
		auto & tWord = *m_dTerms[iTerm].m_pQword;
		while ( tRow!=INVALID_ROWID )
		{
			if constexpr ( ROWID_LIMITS )
				if ( tRow>=m_tSetup.m_uEnd )
					break;

			if ( !CheckContinue() )
				break;

			if ( PassesFieldCheck(iTerm) )
				return m_dTerms[iTerm].m_tRowID = tRow;

			tRow = tWord.GetNextDoc().m_tRowID;
		}

		return m_dTerms[iTerm].m_tRowID = INVALID_ROWID;
	}


	FORCE_INLINE RowID_t Next ( int iTerm )
	{
		return CheckCandidate ( iTerm, m_dTerms[iTerm].m_pQword->GetNextDoc().m_tRowID );
	}


	FORCE_INLINE RowID_t Advance ( int iTerm, RowID_t tMin )
	{
		if ( tMin==m_dTerms[iTerm].m_tRowID )
			return tMin;

		return CheckCandidate ( iTerm, m_dTerms[iTerm].m_pQword->AdvanceTo(tMin) );
	}


	bool Intersect()
	{
		RowID_t tMax = m_dTerms[0].m_tRowID;
		for ( int i=1; i<int(m_dTerms.size()); ++i )
		{
			if ( m_dTerms[i].m_tRowID==tMax )
				continue;

			RowID_t tRow = Advance ( i, tMax );
			if ( tRow==INVALID_ROWID )
			{
				m_dTerms[0].m_tRowID = INVALID_ROWID;
				return false;
			}

			if ( tRow>tMax )
			{
				tMax = Advance ( 0, tRow );
				if ( tMax==INVALID_ROWID )
					return false;

				i = 0;
			}
		}

		return true;
	}
};

}


std::unique_ptr<PostingProducer_i> CreateQwordAndProducer ( const AndProducerSetup_t & tSetup, bool bWideFields )
{
	if ( tSetup.m_tMin || tSetup.m_uEnd!=INVALID_ROWID )
	{
		if ( bWideFields )
			return std::make_unique<QwordAndProducer_T<true,true>>(tSetup);

		return std::make_unique<QwordAndProducer_T<true,false>>(tSetup);
	}

	if ( bWideFields )
		return std::make_unique<QwordAndProducer_T<false,true>>(tSetup);

	return std::make_unique<QwordAndProducer_T<false,false>>(tSetup);
}
