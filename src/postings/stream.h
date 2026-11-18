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

// Block postings on disk (component 4). Fields below appear in write order.
// [x] is optional, x+ means one or more, and | separates alternatives.
// Fixed-width integers are little-endian. See README.md beside this header for codec framing.
//
// .spi keyword dictionary
//   dictionary-entry = delta-keyword posting-locator [doclist-hint] [directory-offset]
//   posting-locator  = positive-spd-offset doc-count hit-count
//                    | 0 row-id packed-hit
//   Locator values are big-endian base-128 varints. In disk format 76, zero marks a
//   positional term with one document and one hit. The row ID is absolute within the
//   chunk. The entire posting lives here. A positive offset points to a .spd doclist.
//   The hint is present for doc-count >= DOCLIST_HINT_THRESH and the .spe offset for
//   doc-count > skiplist_block_size (both use the count with the hitless flag masked).
//
// .spd document lists
//   doclist-entry = short-header columns | term-header leaf+
//   short-header  = "PSS1" bytes:u16 count:u8 flags:u8 fields:u16 single-field:u16
//                   max-tf:u32 column-bytes[4]:u16
//   term-header   = "PST2" version:u16 flags:u16 fields:u16 reserved:u16
//                   column-codecs[4]:u16 reserved:u32 absolute-end:u64
//   leaf          = "BLK1" bytes:u32 count:u16 field-code:u16 first-row:u32 last-row:u32
//                   max-tf:u32 column-bytes[4]:u32 columns
//   columns       = rows [field-masks] [term-frequencies] [hit-refs]
//   Short lists (1..16 rows) store the first row and subsequent gaps as unsigned LEB128.
//   Normal leaves (1..256 rows) store a row container with inclusive bounds and either
//   encoded gaps, a bitmap relative to first-row, or a consecutive range with empty payload.
//   Each column has its own byte length. A shared single field and a constant TF (= max-tf)
//   take zero column bytes. Hitless terms omit hit-refs.
//
// .spd hit references (INLINE_HITS codec)
//   hit-refs = 0 external-offsets
//            | 1 inline-values
//            | 2 inline-bitmap inline-byte-length:u16 inline-values external-offsets
//   Each inline value is one unsigned LEB128 packed hit. In mixed mode, bit i (LSB first)
//   selects posting i for the inline array. Both arrays preserve posting order.
//   External offsets are base-relative and resolve to absolute byte offsets in .spp.
//
// .spp positions for one term in one document
//   hitlist-entry = packed-hit-delta+ 0
//   Each delta is a big-endian base-128 varint, starting from packed hit zero per document.
//   A packed hit has the field ID in bits 31..24, field-end flag in bit 23 and word
//   position in bits 22..0. Dictionary and leaf inline values store that hit directly.
//
// .spe leaf directory for each .spd doclist
//   directory-entry = "SKP1" leaf-count:u32 (first-row:u32 last-row:u32 spd-offset:u64 max-tf:u32)+
//   A short list has one directory leaf. A dictionary singleton has no directory entry.
//   The .spd, .spp and .spe file headers are 0x01, "BPD1"/"BPH1"/"BPS1" respectively,
//   then the component version as a u32.

constexpr unsigned COMPONENT_VERSION = 4;
constexpr uint64_t INLINE_HIT = uint64_t(1)<<63;
constexpr size_t MAX_FIELDS = 256;
constexpr size_t FIELD_WORDS = MAX_FIELDS/32;
constexpr size_t TERM_HEADER_SIZE = 32;
constexpr size_t LEAF_HEADER_SIZE = 40;
constexpr size_t MAX_LEAF_BYTES = 512*1024;

// Hit-reference layout within a leaf.
enum class HitRefMode_e : uint8_t { ALL_EXTERNAL = 0, ALL_INLINE = 1, MIXED = 2 };

enum class Column_e { ROWS, FIELDS, TF, HITREFS, COUNT };
constexpr size_t COLUMNS = size_t(Column_e::COUNT);

struct TermDesc_t
{
	StreamDesc_t	m_tRows { Codec_e::PFOR };
	StreamDesc_t	m_tFields { Codec_e::PFOR };
	StreamDesc_t	m_tTF { Codec_e::PFOR };
	StreamDesc_t	m_tHitRefs { Codec_e::BASE_DELTA };

	uint16_t	m_uFields = 0;
	bool		m_bPositions = true;
};

struct Posting_t
{
	uint32_t	m_uRow = 0;		// Internal row ID within the disk index or chunk.
	uint32_t	m_uTF = 0;		// Number of occurrences of this term in the document.
	uint64_t	m_uHitRef = 0;	// Absolute .spp byte offset, or INLINE_HIT | packed 32-bit hit when TF is 1.

	// Fields containing the term: field f is bit f%32 in word f/32.
	std::array<uint32_t,FIELD_WORDS>	m_dFields {};
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
	uint16_t	m_uSingleField = MAX_FIELDS;
	uint16_t	m_uHeaderSize = LEAF_HEADER_SIZE;
	bool		m_bShort = false;

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
	explicit Writer_c ( Output_i & tOutput, Output_i * pHits = nullptr );

	bool						Begin ( const TermDesc_t & tDesc, std::string & sError );
	bool						Add ( const Posting_t & tPosting, std::string & sError );
	bool						Finish ( std::string & sError );
	bool						FinishDictionarySingleton ( int64_t & iLocator );

	const std::vector<Leaf_t> &	GetLeaves() const	{ return m_dLeaves; }

private:
	bool	WriteShort ( std::string & sError );
	bool	Flush ( std::string & sError );
	bool	EncodeHitRefs ( std::vector<uint8_t> & dOutput, std::string & sError ) const;
	bool	PrepareHitRefs ( bool bShort, std::vector<uint8_t> & dOutput, std::string & sError );

	Output_i &	m_tOutput;
	Output_i *	m_pHits;
	TermDesc_t	m_tDesc;

	std::array<Posting_t,MAX_VALUES>	m_dPostings;
	std::vector<Leaf_t>					m_dLeaves;

	size_t		m_iCount = 0;
	uint64_t	m_uStart = 0;
	uint32_t	m_uLast = 0;
	bool		m_bOpen = false;
	bool		m_bHaveRows = false;
};

class HitRefView_c;

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
	// Prepare the current leaf for individual hit-reference lookups by posting ordinal.
	bool				ReadHitRefView ( HitRefView_c & tView, std::string & sError );

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
	void	OpenDictionarySingleton ( uint64_t uLocator );
	bool	Fail ( std::string & sError, const char * sMessage );
	bool	CheckFields ( std::string & sError );
	bool	CheckTF ( std::string & sError );
	bool	CheckHitRefs ( std::string & sError );

	Input_i &						m_tInput;
	std::vector<uint8_t>			m_dReadBuffer;
	std::vector<uint8_t>			m_dRowBuffer;
	Bytes_t						m_tRowData;
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
