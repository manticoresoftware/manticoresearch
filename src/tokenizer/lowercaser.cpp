//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "lowercaser_impl.h"

#include "sphinxstd.h"
#include "std/fnv64.h"


void CSphLowercaser::Reset()
{
	m_dData.Reset ( CHUNK_SIZE );
	m_dData.ZeroVec();
	m_iChunks = 1;
	m_pChunk[0] = m_dData.begin(); // chunk 0 must always be allocated, for utf-8 tokenizer shortcut to work
	for ( int i = 1; i < CHUNK_COUNT; ++i )
		m_pChunk[i] = nullptr;
	InvalidateStoredClones();
}

void CSphLowercaser::SetRemap ( const CSphLowercaser* pLC )
{
	if ( !pLC )
		return;

	m_iChunks = pLC->m_iChunks;
	m_dData.Reset ( m_iChunks * CHUNK_SIZE );
	m_dData.CopyFrom ( pLC->m_dData );

	for ( int i = 0; i < CHUNK_COUNT; ++i )
		m_pChunk[i] = pLC->m_pChunk[i] ? pLC->m_pChunk[i] - pLC->m_dData.begin() + m_dData.begin() : nullptr;
	InvalidateStoredClones();
}

void CSphLowercaser::AddChars ( const char* szChars, DWORD uFlags )
{
	assert ( szChars );
	auto iChars = (int)strlen ( szChars );

	CSphVector<CSphRemapRange> dRemaps;
	for ( int i = 0; i < iChars; ++i )
		dRemaps.Add ( (CSphRemapRange)szChars[i] );

	AddRemaps ( dRemaps, uFlags );
}

bool CSphLowercaser::CheckRemap ( CSphString& sError, const VecTraits_T<CSphRemapRange>& dRemaps, const char* sSource, bool bCanRemap ) const noexcept
{
	// check
	for ( const CSphRemapRange& r : dRemaps )
	{
		for ( int j = r.m_iStart; j <= r.m_iEnd; ++j )
			if ( ToLower ( j ) )
			{
				sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
				return false;
			}

		if ( !bCanRemap )
			continue;

		for ( int j = r.m_iRemapStart; j <= r.m_iRemapStart + r.m_iEnd - r.m_iStart; ++j )
			if ( ToLower ( j ) )
			{
				sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
				return false;
			}
	}

	return true;
}


void CSphLowercaser::AddRemaps ( const VecTraits_T<CSphRemapRange>& dRemaps, DWORD uFlags )
{
	if ( dRemaps.IsEmpty() )
		return;

	// build new chunks map
	enum { was_unused, was_used, is_used_now } dUsed[CHUNK_COUNT];
	for ( int i = 0; i < CHUNK_COUNT; ++i )
		dUsed[i] = m_pChunk[i] ? was_used : was_unused;

	int iNewChunks = m_iChunks;

	for ( const auto& tRemap: dRemaps )
	{
		assert ( tRemap.m_iStart >= 0 && tRemap.m_iStart < MAX_CODE );
		assert ( tRemap.m_iEnd >= 0 && tRemap.m_iEnd < MAX_CODE );
		assert ( tRemap.m_iRemapStart >= 0 && tRemap.m_iRemapStart < MAX_CODE );
		assert ( ( tRemap.m_iRemapStart + tRemap.m_iEnd - tRemap.m_iStart ) >= 0 && ( tRemap.m_iRemapStart + tRemap.m_iEnd - tRemap.m_iStart ) < MAX_CODE );

		for ( int iChunk = ( tRemap.m_iStart >> CHUNK_BITS ); iChunk <= ( tRemap.m_iEnd >> CHUNK_BITS ); ++iChunk )
			if ( dUsed[iChunk] == was_unused )
			{
				dUsed[iChunk] = is_used_now;
				++iNewChunks;
			}
	}

	// alloc new tables and copy, if necessary
	bool bChanged = false;
	if ( iNewChunks > m_iChunks )
	{
		CSphFixedVector<DWORD> dData { iNewChunks * CHUNK_SIZE };
		dData.ZeroVec();

		auto* pChunk = dData.begin();
		for ( int i = 0; i < CHUNK_COUNT; ++i )
		{
			auto* pOldChunk = m_pChunk[i];

			// build new ptr
			if ( dUsed[i] != was_unused )
			{
				m_pChunk[i] = pChunk;
				pChunk += CHUNK_SIZE;
				bChanged = true;
			}

			// copy old data
			if ( dUsed[i] == was_used )
				memcpy ( m_pChunk[i], pOldChunk, CHUNK_BYTES );
		}
		assert ( pChunk - dData.begin() == iNewChunks * CHUNK_SIZE );

		m_dData.SwapData ( dData );
		m_iChunks = iNewChunks;
	}

	// fill new stuff
	for ( const CSphRemapRange &tRemap : dRemaps )
	{
		auto uRemapped = (DWORD)tRemap.m_iRemapStart;
		for ( int j = tRemap.m_iStart; j <= tRemap.m_iEnd; ++j, ++uRemapped )
		{
			assert ( m_pChunk[j >> CHUNK_BITS] );
			auto& uCodepoint = m_pChunk[j >> CHUNK_BITS][j & CHUNK_MASK];
			auto uNew = uRemapped | uFlags | ( uCodepoint & MASK_FLAGS );
			if ( ( uCodepoint & MASK_CODEPOINT ) && ( uFlags & FLAG_CODEPOINT_SPECIAL ) )
				uNew |= FLAG_CODEPOINT_DUAL;
			bChanged |= uCodepoint!=uNew;
			uCodepoint = uNew;
		}
	}
	if ( bChanged )
		InvalidateStoredClones();
}


int CSphLowercaser::GetMaxCodepointLength() const noexcept
{
	DWORD uMax = 0;
	for ( auto pChunk : m_pChunk )
		if ( pChunk )
			for ( int i = 0; i < CHUNK_SIZE; ++i )
			{
				auto uCode = pChunk[i] & MASK_CODEPOINT;
				if ( uMax < uCode )
					uMax = uCode;
			}
	if ( uMax < 0x80 )
		return 1;
	if ( uMax < 0x800 )
		return 2;
	return 3; // actually, 4 once we hit 0x10000
}

uint64_t CSphLowercaser::GetFNV() const noexcept
{
	return sphFNV64 ( m_dData );
}

void CSphLowercaser::InvalidateStoredClones() noexcept
{
	++m_iGeneration;
}

#define UPCLONESTART( name ) \
void CSphLowercaser::Up##name##Clone () const noexcept \
{ \
	if ( m_iGeneration == m_i##name##Gen ) \
		return;\
	\
    ScopedMutex_t _ { m_tLock }; \
    if ( m_iGeneration == m_i##name##Gen ) \
		return; \
	LowercaserRefcountedPtr pLC { new CSphLowercaser }; \
	pLC->SetRemap ( this );

#define UPCLONEEND( name )  \
	m_p##name##LC = pLC.Leak(); \
	m_i##name##Gen = m_iGeneration; \
}

UPCLONESTART ( Query )
	pLC->AddChars ( "\\", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( Query )

UPCLONESTART ( QueryWildExactJson )
	pLC->AddChars ( "*?%=" );
	pLC->AddChars ( "\\", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryWildExactJson )

UPCLONESTART ( QueryWildExact )
	pLC->AddChars ( "*?%=" );
	pLC->AddChars ( "\\=()|-!@~\"/^$<", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryWildExact )

UPCLONESTART ( QueryWildJson )
	pLC->AddChars ( "*?%" );
	pLC->AddChars ( "\\", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryWildJson )

UPCLONESTART ( QueryWild )
	pLC->AddChars ( "*?%" );
	pLC->AddChars ( "\\()|-!@~\"/^$<", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryWild )

UPCLONESTART ( QueryExactJson )
	pLC->AddChars ( "=" );
	pLC->AddChars ( "\\", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryExactJson )

UPCLONESTART ( QueryExact )
	pLC->AddChars ( "=" );
	pLC->AddChars ( "\\=()|-!@~\"/^$<", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( QueryExact )

UPCLONESTART ( Query_ )
	pLC->AddChars ( "\\()|-!@~\"/^$<", FLAG_CODEPOINT_SPECIAL );
UPCLONEEND ( Query_ )

#undef UPCLONEEND
#undef UPCLONESTART
