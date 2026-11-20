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

struct TermProducerSetup_t
{
	FieldMask_t					m_tFields;
	PostingWindowContext_t		m_tContext;
	PostingPayloadRequirements_t	m_tPayloads;
	PostingCandidateFilter_c *	m_pFilter = nullptr;

	RowID_t		m_tMin = 0;
	RowID_t		m_uEnd = INVALID_ROWID;
	int			m_iCapacity = 0;
	bool		m_bWideFields = false;
};

enum class TermReadMode_e
{
	QWORD_BATCHED,		// Read posting batches and check fields 0-31 using their stored masks.
	QWORD_WIDE_FIELDS,	// Read postings individually and collect full field masks to check fields 32+.
	BLOCK,				// Apply the supplied candidate filter to posting-block rows, then read TF/hit references only for accepted rows.
	UNSUPPORTED			// No producer supports the requested combination; retain the existing execution path.
};

// Select a term-reading strategy based on field restrictions and candidate filtering.
TermReadMode_e						PlanTermProducer ( const TermProducerSetup_t & tSetup );
std::unique_ptr<PostingProducer_i>	CreateTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup );
