// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "stream.h"

namespace postings
{

// Checks framing and encoded payloads before decoding untrusted index data.
class ReaderValidation_c
{
public:
	explicit ReaderValidation_c ( Input_i & tInput );

	bool				Open ( uint64_t uOffset, std::string & sError );
	bool				Next ( uint32_t uMinRow, std::string & sError );
	bool				ReadRows ( std::string & sError );
	bool				ReadFields ( std::string & sError );
	bool				ReadTF ( std::string & sError );
	bool				ReadHitRefs ( std::string & sError );
	bool				Validate ( std::string & sError );

	bool				IsEOF() const	{ return m_bEOF; }
	uint64_t			GetEnd() const	{ return m_uEnd; }
	const TermDesc_t &	GetDesc() const	{ return m_tDesc; }
	const Leaf_t &		GetLeaf() const	{ return m_tLeaf; }

	const std::array<uint32_t,MAX_VALUES> &							GetRows() const			{ return m_dRows; }
	const std::array<uint32_t,MAX_VALUES> &							GetTF() const			{ return m_dTF; }
	const std::array<uint64_t,MAX_VALUES> &							GetHitRefs() const		{ return m_dHitRefs; }
	const std::array<std::array<uint32_t,FIELD_WORDS>,MAX_VALUES> &	GetFields() const		{ return m_dFields; }
	const std::array<uint64_t,COLUMNS> &							GetDecodeCounts() const	{ return m_dDecodeCounts; }

private:
	// The view borrows the input mapping or dBuffer; consume it before either is replaced.
	bool	ReadColumn ( Column_e eColumn, Bytes_t & tData, std::vector<uint8_t> & dBuffer, std::string & sError );
	bool	Fail ( std::string & sError, const char * sMessage );
	bool	CheckFields ( std::string & sError );
	bool	CheckTF ( std::string & sError );
	bool	CheckHitRefs ( std::string & sError );

	Input_i &						m_tInput;
	std::vector<uint8_t>			m_dReadBuffer;
	TermDesc_t						m_tDesc;
	Leaf_t							m_tLeaf;

	uint64_t	m_uNext = 0;
	uint64_t	m_uEnd = 0;
	uint32_t	m_uPrevious = 0;
	bool		m_bHaveLeaf = false;
	bool		m_bEOF = true;
	bool		m_bError = false;

	std::array<bool,COLUMNS>								m_dLoaded {};
	std::array<uint64_t,COLUMNS>							m_dDecodeCounts {};
	alignas(16) std::array<uint32_t,MAX_VALUES>				m_dRows {};
	alignas(16) std::array<uint32_t,MAX_VALUES>				m_dTF {};
	std::array<uint64_t,MAX_VALUES>							m_dHitRefs {};
	std::array<std::array<uint32_t,FIELD_WORDS>,MAX_VALUES>	m_dFields {};
};

}
