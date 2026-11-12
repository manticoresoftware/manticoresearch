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

#include "sphinxsearch.h"

// Borrowed, sorted, unique, field-checked candidates. When m_iCount>0, either m_pRows or m_pDocs is not null
struct TermCandidateBatch_t
{
	const RowID_t *		m_pRows = nullptr;
	const QwordDoc_t *	m_pDocs = nullptr;
	int					m_iCount = 0;

	RowID_t	GetRowID ( int i ) const { assert ( i>=0 && i<m_iCount ); return m_pRows ? m_pRows[i] : m_pDocs[i].m_tRowID; }
};

struct TermPayloadRequirements_t
{
	bool	m_bTF = false;
	bool	m_bHits = false;
};

struct TermProducerSetup_t
{
	FieldMask_t					m_tFields;
	PostingWindowContext_t		m_tContext;
	TermPayloadRequirements_t	m_tPayloads;
	PostingCandidateFilter_i *	m_pFilter = nullptr;

	RowID_t		m_tMin = 0;
	RowID_t		m_uEnd = INVALID_ROWID;
	int			m_iCapacity = 0;
	bool		m_bWideFields = false;
};

enum class TermReadMode_e
{
	BATCHED,          // Read posting batches and check fields 0-31 using their stored masks.
	WIDE_FIELDS,      // Read postings individually and collect full field masks to check fields 32+.
	RESTRICTED_BLOCK, // Apply the supplied candidate filter to posting-block rows, then read TF/hit references only for accepted rows.
	UNSUPPORTED      // No producer supports the requested combination; retain the existing execution path.
};

// Exclusive posting traversal of an already set-up qword
class TermProducer_i
{
public:
	virtual ~TermProducer_i() = default;

	// Returns at most the configured capacity
	virtual TermCandidateBatch_t NextCandidates() = 0;
	virtual const QwordDoc_t * ReadPayloads ( const RowID_t * pRows=nullptr, int iCount=0 ) = 0;
	virtual void	HintRowID ( RowID_t tMin ) = 0;

	// Set before NextCandidates(); legacy readers may decode payloads with rows.
	virtual void	SetPayloadRequirements ( TermPayloadRequirements_t tRequirements ) = 0;
};

TermReadMode_e					PlanTermProducer ( const TermProducerSetup_t & tSetup );
std::unique_ptr<TermProducer_i>	CreateTermProducer ( ISphQword & tWord, const TermProducerSetup_t & tSetup );
