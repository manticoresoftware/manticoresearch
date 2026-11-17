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

#include "andproducer.h"

#include <algorithm>

class AndProducerBase_c : public PostingProducer_i
{
public:
	explicit AndProducerBase_c ( const AndProducerSetup_t & tSetup ) : m_tSetup(tSetup), m_tHint(tSetup.m_tMin) {}

	void HintRowID ( RowID_t tMin ) final { m_tHint = std::max ( m_tHint, tMin ); }

	void SetPayloadRequirements ( PostingPayloadRequirements_t tRequirements ) final
	{
		m_tSetup.m_tPayloads = tRequirements;
		for ( const auto & tTerm : m_tSetup.m_dTerms )
			tTerm.m_pQword->SetPayloadRequirements ( tRequirements.m_bTF || tRequirements.m_bHits, tRequirements.m_bHits );
	}

protected:
	AndProducerSetup_t	m_tSetup;
	RowID_t				m_tHint;
};
