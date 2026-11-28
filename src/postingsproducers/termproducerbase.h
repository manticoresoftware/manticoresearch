//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "termproducer.h"

class TermProducerBase_c : public PostingProducer_i
{
public:
	TermProducerBase_c ( ISphQword & tWord, const TermProducerSetup_t & tSetup )
		: m_tWord(tWord)
		, m_tSetup(tSetup)
		, m_dDocs(tSetup.m_iCapacity)
	{
		SetPayloadRequirements(tSetup.m_tPayloads);
	}


	void SetPayloadRequirements ( PostingPayloadRequirements_t tRequirements ) final
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
