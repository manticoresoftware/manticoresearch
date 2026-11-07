// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "indexformat.h"
#include "streamvalidation.h"

class BlockIndexQwordValidation_c : public DiskIndexQwordTraits_c, private postings::Input_i
{
public:
	BlockIndexQwordValidation_c();

	void				Reset() override;
	void				SetReadErrorTarget ( CSphString * pError ) override	{ m_pQueryError = pError; }
	bool				Setup ( const DiskIndexQwordSetup_c * pSetup ) override;
	void				SeekDocReader ( SphOffset_t iPos, int iHint ) override;
	void				SeekHitReader ( SphOffset_t iPos, int iHint ) override;
	SphOffset_t			GetDocReaderPos() const override;
	const CSphMatch &	GetNextDoc() override;
	void				SeekHitlist ( SphOffset_t uOffset ) override;
	Hitpos_t			GetNextHit() override;
	bool				CheckDirectory ( CSphReader & tSkips, SphOffset_t iOffset, int iFields );
	const char *		GetReadError() const override	{ return m_sError.empty() ? nullptr : m_sError.c_str(); }

private:
	uint64_t		GetSize() const final		{ return m_iDocFileSize; }
	const uint8_t *	GetMappedData() const final	{ return m_pDocMap; }
	void			OnDocReaderChanged() final;
	bool			Read ( uint64_t uOffset, uint8_t * pData, size_t iSize, std::string & sError ) final;
	bool			LoadLeaf();
	void			SetDoc ( unsigned i );
	void			Fail();
	void			ResetCursor();
	bool			ValidateLeaf();
	bool			ValidateStats();
	bool			ValidateHits();
	bool			ReadCheckedHit ( DWORD & uDelta );

	struct Validation_t
	{
		std::array<postings::FieldStats_t,postings::MAX_FIELDS>	m_dStats {};
		std::array<uint32_t,postings::MAX_FIELDS>				m_dHitTF {};
		std::vector<std::array<uint32_t,postings::MAX_VALUES>>	m_dFieldTF;
		std::vector<postings::Leaf_t>							m_dLeaves;
	};

	Validation_t					m_tValidation;
	postings::ReaderValidation_c	m_tReader;
	const BYTE *					m_pDocMap = nullptr;
	std::string						m_sError;
	CSphString *					m_pQueryError = nullptr;

	SphOffset_t	m_iStart = 0;
	SphOffset_t	m_iNext = 0;
	unsigned	m_iOrdinal = 0;
	bool		m_bHitEOF = true;
	bool		m_bOpen = false;
	bool		m_bEOF = false;
};
