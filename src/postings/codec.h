// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace postings
{

constexpr size_t MAX_VALUES = 256;
constexpr size_t STREAM_DESCRIPTOR_SIZE = 2;
constexpr size_t INTEGER_HEADER_SIZE = 6;
constexpr size_t ROW_HEADER_SIZE = 20;
constexpr size_t MAX_BITMAP_BITS = 8192;

// Experimental IDs, isolated from all existing on-disk index formats.
enum class Codec_e : uint16_t { INVALID = 0, FOR = 1, PFOR = 2, VARINT = 3, CONSTANT = 4 };
enum class Rows_e : uint8_t { AUTO = 0, DELTA = 1, BITMAP = 2, RANGE = 3 };
enum class Backend_e { AUTO, SCALAR, SIMD };

struct Bytes_t
{
	const uint8_t *	m_pData = nullptr;
	size_t			m_iSize = 0;
};

struct StreamDesc_t
{
	Codec_e	m_eCodec = Codec_e::INVALID;
};

struct IntegerDesc_t
{
	uint8_t		m_uWidth = 0;
	uint8_t		m_uExceptions = 0;
	uint16_t	m_uCount = 0;
	Bytes_t		m_tPayload;
};

// Borrowed payload: valid while the original encoded buffer remains unchanged.
struct RowBlock_t
{
	Rows_e		m_eKind = Rows_e::AUTO;
	uint16_t	m_uCount = 0;
	uint32_t	m_uFirst = 0;
	uint32_t	m_uLast = 0;
	Bytes_t		m_tPayload;
};

bool HasSimd();
const char * BackendName ( Backend_e eBackend = Backend_e::AUTO );

// Read/write the codec identity once per stream; unknown IDs fail explicitly.
bool WriteStreamDescriptor ( StreamDesc_t tStream, std::vector<uint8_t> & dOutput, std::string & sError );
bool ReadStreamDescriptor ( Bytes_t tInput, StreamDesc_t & tStream, std::string & sError );

// Encode a block using its stream codec; failed operations leave output unchanged.
bool EncodeIntegers ( StreamDesc_t tStream, const uint32_t * pValues, size_t iCount, std::vector<uint8_t> & dOutput, std::string & sError, Backend_e eBackend = Backend_e::AUTO );

// Validate the complete frame before exposing its bounded payload.
bool InspectIntegers ( StreamDesc_t tStream, Bytes_t tInput, IntegerDesc_t & tDesc, std::string & sError );

// Decode trusted index data. Structural/semantic checks run only in debug assertions.
// The caller supplies sufficient output capacity and an available backend.
// Use InspectIntegers/InspectRows when checking untrusted or damaged files.
// Into requires disjoint input/output ranges; DecodeIntegers also supports overlap.
bool DecodeIntegersInto ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend = Backend_e::AUTO );
bool DecodeIntegers ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend = Backend_e::AUTO );

bool EncodeRows ( StreamDesc_t tStream, const uint32_t * pRows, size_t iCount, std::vector<uint8_t> & dOutput, std::string & sError, Rows_e eKind = Rows_e::AUTO, Backend_e eBackend = Backend_e::AUTO );

// Includes ordering, endpoint and bitmap-cardinality validation. No borrowed view on error.
bool InspectRows ( StreamDesc_t tStream, Bytes_t tInput, RowBlock_t & tBlock, std::string & sError );
// Into writes directly to the caller; DecodeRows retains input/output overlap support.
bool DecodeRowsInto ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend = Backend_e::AUTO );
bool DecodeRows ( StreamDesc_t tStream, Bytes_t tInput, uint32_t * pOutput, size_t iCapacity, std::string & sError, Backend_e eBackend = Backend_e::AUTO );

}
