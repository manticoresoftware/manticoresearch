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

#include "postingproducer.h"

struct AndProducerTerm_t
{
	ISphQword *	m_pQword = nullptr;
	FieldMask_t	m_tFields;
	bool		m_bWideFields = false;
};

struct AndProducerSetup_t
{
	std::vector<AndProducerTerm_t>	m_dTerms;
	std::vector<AndProducerTerm_t>	m_dExcluded;
	PostingWindowContext_t		m_tContext;
	PostingPayloadRequirements_t	m_tPayloads;
	PostingCandidateFilter_c *	m_pFilter = nullptr;

	RowID_t		m_tMin = 0;
	RowID_t		m_uEnd = INVALID_ROWID;
	int			m_iCapacity = 0;
};

enum class AndReadMode_e
{
	BLOCK,		// Intersect posting blocks, then gather payloads for accepted rows.
	QWORD,		// Advance qwords against each other, retaining each match's payloads.
	UNSUPPORTED	// The requested candidate filter or exclusions require block cursors.
};

AndReadMode_e				PlanAndProducer ( const AndProducerSetup_t & tSetup );
std::unique_ptr<PostingProducer_i> CreateAndProducer ( const AndProducerSetup_t & tSetup );
