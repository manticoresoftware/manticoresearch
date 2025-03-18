//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "infix_builder.h"
#include "sphinxint.h"

#include "std/crc32.h"

#include <array>

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY, INFIX HASH BUILDER
//////////////////////////////////////////////////////////////////////////

static constexpr int INFIX_ARENA_LENGTH = 1048576;

template<int SIZE>
struct Infix_t
{
	std::array<BYTE, SIZE * sizeof ( DWORD )> m_Data;

#ifndef NDEBUG
	BYTE m_TrailingZero = 0;
#endif

	void Reset()
	{
		m_Data.fill ( 0 );
	}

	bool operator== ( const Infix_t<SIZE>& rhs ) const noexcept
	{
		return m_Data == rhs.m_Data;
	}
};


class InfixIntvec_c
{
private:
	// do not change the order of fields in this union - it matters a lot
	union {
		std::array<DWORD, 4> m_dData;
		struct
		{
			int m_iDynLen;
			int m_iDynLimit;
			DWORD* m_pDynData;
		};
	};

	bool IsDynamic() const noexcept
	{
		return ( m_dData[0] & 0x80000000UL ) != 0;
	}

public:
	InfixIntvec_c()
	{
		m_dData.fill(0);
	}

	~InfixIntvec_c()
	{
		if ( IsDynamic() )
			SafeDeleteArray ( m_pDynData );
	}

	void Add ( DWORD uVal )
	{
		if ( !m_dData[0] )
		{
			// empty
			m_dData[0] = uVal | 0x01000000UL;
			return;
		}

		if ( !IsDynamic() )
		{
			// 1..4 static entries
			int iLen = m_dData[0] >> 24;
			DWORD uLast = m_dData[iLen - 1] & 0x00ffffffUL;

			// redundant
			if ( uVal == uLast )
				return;

			// grow static part
			if ( iLen < 4 )
			{
				m_dData[iLen] = uVal;
				m_dData[0] = ( m_dData[0] & 0x00ffffffUL ) | ( ++iLen << 24 );
				return;
			}

			// dynamize
			DWORD* pDyn = new DWORD[16];
			memcpy ( pDyn, m_dData.data(), 4 * sizeof ( DWORD ) );
			pDyn[0] &= 0x00ffffffUL;
			pDyn[4] = uVal;
			m_iDynLen = 0x80000005UL; // dynamic flag, len=5
			m_iDynLimit = 16;		  // limit=16
			m_pDynData = pDyn;
			return;
		}

		// N dynamic entries
		int iLen = m_iDynLen & 0x00ffffffUL;
		if ( uVal == m_pDynData[iLen - 1] )
			return;
		if ( iLen >= m_iDynLimit )
		{
			m_iDynLimit *= 2;
			auto* pNew = new DWORD[m_iDynLimit];
			memcpy ( pNew, m_pDynData, iLen * sizeof ( DWORD ) );
			delete[] ( std::exchange ( m_pDynData, pNew ) );
		}

		m_pDynData[iLen] = uVal;
		++m_iDynLen;

	}

	int GetLength() const noexcept
	{
		if ( !IsDynamic() )
			return m_dData[0] >> 24;
		return m_iDynLen & 0x00ffffffUL;
	}

	DWORD operator[] ( int iIndex ) const noexcept
	{
		if ( !IsDynamic() )
			return m_dData[iIndex] & 0x00ffffffUL;
		return m_pDynData[iIndex];
	}
};


template<int SIZE>
struct InfixHashEntry_t
{
	Infix_t<SIZE> m_tKey;	///< key, owned by the hash
	InfixIntvec_c m_tValue; ///< data, owned by the hash
	int m_iNext;			///< next entry in hash arena
};


template<int SIZE>
class InfixBuilder_c final: public ISphInfixBuilder
{
	std::array<int, INFIX_ARENA_LENGTH> m_dHash; ///< all the hash entries
	CSphSwapVector<InfixHashEntry_t<SIZE>> m_dArena;
	CSphVector<InfixBlock_t> m_dBlocks;
	CSphTightVector<BYTE> m_dBlocksWords;

private:
	void AddEntry ( const Infix_t<SIZE>& tKey, DWORD uHash, int iCheckpoint )
	{
		uHash &= ( INFIX_ARENA_LENGTH - 1 );

		int iEntry = m_dArena.GetLength();
		InfixHashEntry_t<SIZE>& tNew = m_dArena.Add();
		tNew.m_tKey = tKey;
		tNew.m_tValue.Add ( iCheckpoint ); // len=1, data=iCheckpoint
		tNew.m_iNext = std::exchange ( m_dHash[uHash], iEntry );
	}

	/// get value pointer by key
	InfixIntvec_c* LookupEntry ( const Infix_t<SIZE>& tKey, DWORD uHash )
	{
		uHash &= ( INFIX_ARENA_LENGTH - 1 );
		int iEntry = m_dHash[uHash];
		int iiEntry = 0;

		while ( iEntry )
		{
			if ( m_dArena[iEntry].m_tKey == tKey )
			{
				// mtf it, if needed
				if ( iiEntry )
					m_dArena[iiEntry].m_iNext = std::exchange ( m_dArena[iEntry].m_iNext, std::exchange ( m_dHash[uHash], iEntry ) );

				return &m_dArena[iEntry].m_tValue;
			}
			iiEntry = std::exchange ( iEntry, m_dArena[iEntry].m_iNext );
		}
		return nullptr;
	}

public:
	InfixBuilder_c()
	{
		// init the hash
		for ( auto& uHash : m_dHash )
			uHash = 0;
		m_dArena.Reserve ( INFIX_ARENA_LENGTH );
		m_dArena.Resize ( 1 ); // 0 is a reserved index
	}

	void AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) override;
	void SaveEntries ( CSphWriter& wrDict ) override;
	int64_t SaveEntryBlocks ( CSphWriter& wrDict ) override;
	int GetBlocksWordsSize() const override { return m_dBlocksWords.GetLength(); }
};



/// single-byte case, 2-dword infixes
template<>
void InfixBuilder_c<2>::AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord != MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord < 0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		++pWord;
		--iWordLength;
	}

	Infix_t<2> sKey;
	for ( int p = 0; p <= iWordLength - 2; ++p )
	{
		sKey.Reset();

		auto* pKey = sKey.m_Data.data();
		const BYTE* s = pWord + p;
		const BYTE* sMax = s + Min ( 6, iWordLength - p );

		DWORD uHash = CRC32_start ( *s );
		*pKey++ = *s++; // copy first infix byte

		while ( s < sMax )
		{
			CRC32_step ( uHash, *s );
			*pKey++ = *s++; // copy another infix byte

			InfixIntvec_c * pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}
	}
}


/// UTF-8 case, 3/5-dword infixes
template<int SIZE>
void InfixBuilder_c<SIZE>::AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord != MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord < 0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		++pWord;
		--iWordLength;
	}

	const BYTE* pWordMax = pWord + iWordLength;
#ifndef NDEBUG
	bool bInvalidTailCp = false;
#endif
	int iCodes = 0;					   // codepoints in current word
	std::array<BYTE, SPH_MAX_WORD_LEN + 1> dBytes; // byte offset for each codepoints

	// build an offsets table into the bytestring
	dBytes[0] = 0;
	for ( const BYTE* p = pWord; p < pWordMax && iCodes < SPH_MAX_WORD_LEN; )
	{
		int iLen = sphUtf8CharBytes ( *p );

		// break on tail cut codepoint
		if ( p + iLen > pWordMax )
		{
#ifndef NDEBUG
			bInvalidTailCp = true;
#endif
			break;
		}

		// skip word with large codepoints
		if ( iLen > SIZE )
			return;

		assert ( iLen >= 1 && iLen <= 4 );
		p += iLen;

		dBytes[iCodes + 1] = dBytes[iCodes] + (BYTE)iLen;
		++iCodes;
	}
	assert ( pWord[dBytes[iCodes]] == 0 || iCodes == SPH_MAX_WORD_LEN || bInvalidTailCp );

	// generate infixes
	Infix_t<SIZE> sKey;
	for ( int p = 0; p <= iCodes - 2; ++p )
	{
		sKey.Reset();
		BYTE* pKey = sKey.m_Data.data();
		const BYTE* pKeyMax = pKey + sizeof ( sKey.m_Data );

		const BYTE* s = pWord + dBytes[p];
		const BYTE* sMax = pWord + dBytes[p + Min ( 6, iCodes - p )];

		// copy first infix codepoint
		DWORD uHash = 0xffffffffUL;
		do
		{
			CRC32_step ( uHash, *s );
			*pKey++ = *s++;
		} while ( ( *s & 0xC0 ) == 0x80 );

		assert ( s - ( pWord + dBytes[p] ) == ( dBytes[p + 1] - dBytes[p] ) );

		while ( s < sMax && pKey < pKeyMax && pKey + sphUtf8CharBytes ( *s ) <= pKeyMax )
		{
			// copy next infix codepoint
			do
			{
				CRC32_step ( uHash, *s );
				*pKey++ = *s++;
			} while ( ( *s & 0xC0 ) == 0x80 && pKey < pKeyMax );

			assert ( sphUTF8Len ( (const char*)sKey.m_Data.data(), sizeof ( sKey.m_Data ) ) >= 2 );

			InfixIntvec_c* pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}

		assert ( (size_t)( pKey - (BYTE*)sKey.m_Data.data() ) <= int ( sizeof ( sKey.m_Data ) ) );
	}
}

static inline int ZippedIntSize ( DWORD v ) noexcept
{
	if ( v < ( 1UL << 7 ) )
		return 1;
	if ( v < ( 1UL << 14 ) )
		return 2;
	if ( v < ( 1UL << 21 ) )
		return 3;
	if ( v < ( 1UL << 28 ) )
		return 4;
	return 5;
}

template<int SIZE>
void InfixBuilder_c<SIZE>::SaveEntries ( CSphWriter& wrDict )
{
	// intentionally local to this function
	// we mark the block end with an editcode of 0
	const int INFIX_BLOCK_SIZE = 64;

	wrDict.PutBlob ( g_sTagInfixEntries );

	CSphVector<int> dIndex;
	dIndex.Resize ( m_dArena.GetLength() - 1 );
	dIndex.FillSeq(1);
	dIndex.Sort ( Lesser ( [this] ( int a, int b ) noexcept { return m_dArena[a].m_tKey.m_Data < m_dArena[b].m_tKey.m_Data; } ) );

	m_dBlocksWords.Reserve ( m_dArena.GetLength() / INFIX_BLOCK_SIZE * sizeof ( DWORD ) * SIZE );
	int iBlock = 0;
	int iPrevKey = -1;
	constexpr size_t DWSIZE = sizeof ( DWORD ) * SIZE;
	ARRAY_FOREACH ( iIndex, dIndex )
	{
		InfixIntvec_c& dData = m_dArena[dIndex[iIndex]].m_tValue;
		const char* sKey = (const char*)m_dArena[dIndex[iIndex]].m_tKey.m_Data.data();
		int iChars = ( SIZE == 2 )
					   ? (int)strnlen ( sKey, DWSIZE )
					   : sphUTF8Len ( sKey, (int)DWSIZE );
		assert ( iChars >= 2 && iChars < int ( 1 + sizeof ( Infix_t<SIZE> ) ) );

		// keep track of N-infix blocks
		auto iAppendBytes = (int)strnlen ( sKey, DWSIZE );
		if ( !iBlock )
		{
			int iOff = m_dBlocksWords.GetLength();
			m_dBlocksWords.Resize ( iOff + iAppendBytes + 1 );

			InfixBlock_t& tBlock = m_dBlocks.Add();
			tBlock.m_iInfixOffset = iOff;
			tBlock.m_iOffset = (DWORD)wrDict.GetPos();

			memcpy ( m_dBlocksWords.Begin() + iOff, sKey, iAppendBytes );
			m_dBlocksWords[iOff + iAppendBytes] = '\0';
		}

		// compute max common prefix
		// edit_code = ( num_keep_chars<<4 ) + num_append_chars
		int iEditCode = iChars;
		if ( iPrevKey >= 0 )
		{
			const char* sPrev = (const char*)m_dArena[dIndex[iPrevKey]].m_tKey.m_Data.data();
			const char* sCur = sKey;
			const char* sMax = sCur + iAppendBytes;

			int iKeepChars = 0;
			if constexpr ( SIZE == 2 )
			{
				// SBCS path
				while ( sCur < sMax && *sCur && *sCur == *sPrev )
				{
					++sCur;
					++sPrev;
				}
				iKeepChars = (int)( sCur - sKey );

				assert ( iKeepChars >= 0 && iKeepChars < 16 );
				assert ( iChars - iKeepChars >= 0 );
				assert ( iChars - iKeepChars < 16 );

				iEditCode = ( iKeepChars << 4 ) + ( iChars - iKeepChars );
				iAppendBytes = ( iChars - iKeepChars );
				sKey = sCur;
			}
			else
			{
				// UTF-8 path
				const char* sKeyMax = sCur; // track max matching sPrev prefix in [sKey,sKeyMax)
				while ( sCur < sMax && *sCur && *sCur == *sPrev )
				{
					// current byte matches, move the pointer
					++sCur;
					++sPrev;

					// tricky bit
					// if the next (!) byte is a valid UTF-8 char start (or eof!)
					// then we just matched not just a byte, but a full char
					// so bump the matching prefix boundary and length
					if ( sCur >= sMax || ( *sCur & 0xC0 ) != 0x80 )
					{
						sKeyMax = sCur;
						++iKeepChars;
					}
				}

				assert ( iKeepChars >= 0 && iKeepChars < 16 );
				assert ( iChars - iKeepChars >= 0 );
				assert ( iChars - iKeepChars < 16 );

				iEditCode = ( iKeepChars << 4 ) + ( iChars - iKeepChars );
				iAppendBytes -= (int)( sKeyMax - sKey );
				sKey = sKeyMax;
			}
		}

		// write edit code, postfix
		wrDict.PutByte ( (BYTE)iEditCode );
		wrDict.PutBytes ( sKey, iAppendBytes );

		// compute data length
		int iDataLen = ZippedIntSize ( dData[0] );
		for ( int j = 1; j < dData.GetLength(); ++j )
			iDataLen += ZippedIntSize ( dData[j] - dData[j - 1] );

		// write data length, data
		wrDict.ZipInt ( iDataLen );
		wrDict.ZipInt ( dData[0] );
		for ( int j = 1; j < dData.GetLength(); ++j )
			wrDict.ZipInt ( dData[j] - dData[j - 1] );

		// mark block end, restart deltas
		iPrevKey = iIndex;
		if ( ++iBlock == INFIX_BLOCK_SIZE )
		{
			iBlock = 0;
			iPrevKey = -1;
			wrDict.PutByte ( 0 );
		}
	}

	// put end marker
	if ( iBlock )
		wrDict.PutByte ( 0 );

	const char* pBlockWords = (const char*)m_dBlocksWords.Begin();
	ARRAY_FOREACH ( i, m_dBlocks )
		m_dBlocks[i].m_sInfix = pBlockWords + m_dBlocks[i].m_iInfixOffset;

	if ( wrDict.GetPos() > UINT_MAX ) // FIXME!!! change to int64
		sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", wrDict.GetPos() );
}


template<int SIZE>
int64_t InfixBuilder_c<SIZE>::SaveEntryBlocks ( CSphWriter& wrDict )
{
	// save the blocks
	wrDict.PutBlob ( g_sTagInfixBlocks );

	SphOffset_t iInfixBlocksOffset = wrDict.GetPos();
	assert ( iInfixBlocksOffset <= INT_MAX );

	wrDict.ZipInt ( m_dBlocks.GetLength() );
	ARRAY_FOREACH ( i, m_dBlocks )
	{
		auto iBytes = strlen ( m_dBlocks[i].m_sInfix );
		wrDict.PutByte ( BYTE ( iBytes ) );
		wrDict.PutBytes ( m_dBlocks[i].m_sInfix, iBytes );
		wrDict.ZipInt ( m_dBlocks[i].m_iOffset ); // maybe delta these on top?
	}

	return iInfixBlocksOffset;
}


std::unique_ptr<ISphInfixBuilder> sphCreateInfixBuilder ( int iCodepointBytes, CSphString* pError )
{
	assert ( pError );
	switch ( iCodepointBytes )
	{
	case 0: return nullptr;
	case 1: return std::make_unique<InfixBuilder_c<2>>(); // upto 6x1 bytes, 2 dwords, sbcs
	case 2: return std::make_unique<InfixBuilder_c<3>>(); // upto 6x2 bytes, 3 dwords, utf-8
	case 3: return std::make_unique<InfixBuilder_c<5>>(); // upto 6x3 bytes, 5 dwords, utf-8
	default: pError->SetSprintf ( "unhandled max infix codepoint size %d", iCodepointBytes ); return nullptr;
	}
}