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

// Borrowed, sorted, unique, field-checked candidates. A nonempty batch has either
// row IDs or decoded postings for term 0; the latter also supply its payloads.
struct PostingCandidateBatch_t
{
	const RowID_t *		m_pRows = nullptr;
	const QwordDoc_t *	m_pDocs = nullptr;
	int					m_iCount = 0;

	RowID_t	GetRowID ( int i ) const { assert ( i>=0 && i<m_iCount ); return m_pRows ? m_pRows[i] : m_pDocs[i].m_tRowID; }
};

struct PostingPayloadRequirements_t
{
	bool	m_bTF = false;
	bool	m_bHits = false;
};

// Produces batches of matching row IDs or decoded postings and provides each
// term's requested payloads for those rows, hiding the underlying posting format.
class PostingProducer_i
{
public:
	virtual ~PostingProducer_i() = default;

	// Produce the next batch of matching rows, up to iCapacity entries.
	virtual PostingCandidateBatch_t NextCandidates ( int iCapacity ) = 0;

	// Read a term's field masks and requested payloads for the last candidate batch
	// or a selected subset of its rows.
	virtual const QwordDoc_t * ReadPayloads ( int iTerm=0, const RowID_t * pRows=nullptr, int iCount=0 ) = 0;

	// Guide subsequent reads toward tMin, allowing earlier postings to be skipped.
	virtual void	HintRowID ( RowID_t tMin ) = 0;

	// Specify whether the consumer needs term frequencies and hitlist offsets.
	virtual void	SetPayloadRequirements ( PostingPayloadRequirements_t tRequirements ) = 0;
};
