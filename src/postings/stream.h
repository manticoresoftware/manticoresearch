// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "codec.h"
#include <array>
#include <memory>

namespace postings
{

constexpr unsigned COMPONENT_VERSION = 2;
constexpr size_t MAX_FIELDS = 256;
constexpr size_t FIELD_WORDS = MAX_FIELDS/32;
constexpr size_t TERM_HEADER_SIZE = 40;
constexpr size_t LEAF_HEADER_SIZE = 44;
constexpr size_t MAX_LEAF_BYTES = 512*1024;

enum class Column_e { ROWS, FIELDS, TF, HITREFS, FIELD_TF, COUNT };
constexpr size_t COLUMNS = size_t(Column_e::COUNT);

struct TermDesc_t
{
	StreamDesc_t	m_tRows { Codec_e::PFOR };
	StreamDesc_t	m_tFields { Codec_e::PFOR };
	StreamDesc_t	m_tTF { Codec_e::PFOR };
	StreamDesc_t	m_tHitRefs { Codec_e::BASE_DELTA };

	uint16_t	m_uFields = 0;
	bool		m_bPositions = true;
	bool		m_bFieldTF = true;
};

struct Posting_t
{
	uint32_t	m_uRow = 0;
	uint32_t	m_uTF = 0;
	uint64_t	m_uHitRef = 0;

	std::array<uint32_t,FIELD_WORDS>	m_dFields {};
	std::array<uint32_t,MAX_FIELDS>		m_dFieldTF {};
};

struct FieldStats_t
{
	uint64_t	m_uDocs = 0;
	uint64_t	m_uTF = 0;
};

struct Leaf_t
{
	uint64_t	m_uOffset = 0;
	uint32_t	m_uBytes = 0;
	uint32_t	m_uFirst = 0;
	uint32_t	m_uLast = 0;
	uint32_t	m_uMaxTF = 0;
	uint16_t	m_uCount = 0;
	uint16_t	m_uFieldWords = 0;

	std::array<uint32_t,COLUMNS>	m_dSizes {};
};

class Output_i
{
public:
	virtual ~Output_i() = default;

	virtual uint64_t	GetPos() const = 0;
	virtual bool		Write ( Bytes_t tData, std::string & sError ) = 0;
	// Patch already-written bytes without truncating or moving the append position.
	virtual bool		Patch ( uint64_t uOffset, Bytes_t tData, std::string & sError ) = 0;
};

class Input_i
{
public:
	virtual ~Input_i() = default;

	virtual uint64_t		GetSize() const = 0;
	// Whole-input mapping, valid until the input is replaced; null for buffered input.
	virtual const uint8_t *	GetMappedData() const	{ return nullptr; }
	virtual bool			Read ( uint64_t uOffset, uint8_t * pData, size_t iSize, std::string & sError ) = 0;
};

// One bounded buffer per writer; no complete posting list is materialized.
class Writer_c
{
public:
	explicit Writer_c ( Output_i & tOutput );

	bool						Begin ( const TermDesc_t & tDesc, std::string & sError );
	bool						Add ( const Posting_t & tPosting, std::string & sError );
	bool						Finish ( std::string & sError );

	const std::vector<Leaf_t> &	GetLeaves() const	{ return m_dLeaves; }

private:
	bool	Flush ( std::string & sError );
	bool	EncodeHitRefs ( std::vector<uint8_t> & dOutput, std::string & sError ) const;

	Output_i &	m_tOutput;
	TermDesc_t	m_tDesc;

	std::array<Posting_t,MAX_VALUES>	m_dPostings;
	std::array<FieldStats_t,MAX_FIELDS>	m_dStats {};
	std::vector<Leaf_t>					m_dLeaves;

	size_t		m_iCount = 0;
	uint64_t	m_uStart = 0;
	uint32_t	m_uLast = 0;
	bool		m_bOpen = false;
	bool		m_bHaveRows = false;
};

// Reads leaf headers independently of payloads; each column is decoded on demand.
// Structural invariants are debug assertions; indextool uses ReaderValidation_c.
// I/O errors leave the reader in an error state until Open is called.
class Reader_c
{
public:
	explicit Reader_c ( Input_i & tInput );

	bool				Open ( uint64_t uOffset, std::string & sError );
	bool				Next ( uint32_t uMinRow, std::string & sError );
	bool				ReadRows ( std::string & sError );
	// Read the current leaf's row-container kind, count, bounds and encoded payload
	// without expanding it into row IDs. The borrowed payload remains valid until
	// Next/Open, including across other column reads.
	bool				ReadRowBlock ( RowBlock_t & tBlock, std::string & sError );
	// Decode the field-membership mask for fields 0..31 for each posting in the
	// current leaf, in posting order. pFields must hold at least m_uCount entries.
	// Higher field-mask words are not decoded; GetFields() is not populated.
	bool				ReadLowFields ( uint32_t * pFields, std::string & sError );
	bool				ReadFields ( std::string & sError );
	bool				ReadTF ( std::string & sError );
	bool				ReadHitRefs ( std::string & sError );
	bool				ReadFieldTF ( unsigned uField, std::array<uint32_t,MAX_VALUES> & dTF, std::string & sError );
	bool				ReadStats ( std::array<FieldStats_t,MAX_FIELDS> & dStats, std::string & sError );

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

	struct FieldTFCache_t
	{
		Bytes_t					m_tData;
		std::vector<uint8_t>	m_dBuffer; // owns buffered input while other columns reuse m_dReadBuffer

		std::array<size_t,MAX_FIELDS>					m_dOffsets {};
		std::array<std::vector<uint32_t>,MAX_FIELDS>	m_dValues;
	};

	std::unique_ptr<FieldTFCache_t>	m_pFieldTF;
	Input_i &						m_tInput;
	std::vector<uint8_t>			m_dReadBuffer;
	std::vector<uint8_t>			m_dRowBuffer;
	Bytes_t						m_tRowData;
	TermDesc_t						m_tDesc;
	Leaf_t							m_tLeaf;

	uint64_t	m_uNext = 0;
	uint64_t	m_uEnd = 0;
	uint64_t	m_uStats = 0;
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
