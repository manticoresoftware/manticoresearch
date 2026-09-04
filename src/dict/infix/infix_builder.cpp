//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
#include "fileio.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <queue>
#include <vector>

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
	static constexpr uintptr_t INLINE_FLAG = 1;
	static constexpr int INLINE_LEN_SHIFT = 1;
	static constexpr int INLINE_VALUE_SHIFT = 3;
	static constexpr int INLINE_VALUE_BITS = 24;
	static constexpr int DYNAMIC_INITIAL_LIMIT = 16;

	uintptr_t m_uData = 0;

	bool IsDynamic() const noexcept { return m_uData && !( m_uData & INLINE_FLAG ); }
	int GetInlineLength() const noexcept { return (int)( ( m_uData >> INLINE_LEN_SHIFT ) & 3 ); }
	DWORD GetInline ( int iIndex ) const noexcept
	{
		return (DWORD)( ( m_uData >> ( INLINE_VALUE_SHIFT + INLINE_VALUE_BITS * iIndex ) ) & INFIX_CHECKPOINT_ID_MAX );
	}
	DWORD* GetDynamic() noexcept { return reinterpret_cast<DWORD*>(m_uData); }
	const DWORD* GetDynamic() const noexcept { return reinterpret_cast<const DWORD*>(m_uData); }

	void Reset()
	{
		if ( IsDynamic() )
			delete[] GetDynamic();
		m_uData = 0;
	}

public:
	InfixIntvec_c() = default;
	InfixIntvec_c ( const InfixIntvec_c& ) = delete;
	InfixIntvec_c& operator= ( const InfixIntvec_c& ) = delete;
	InfixIntvec_c ( InfixIntvec_c&& rhs ) noexcept
		: m_uData ( std::exchange ( rhs.m_uData, 0 ) )
	{}

	InfixIntvec_c& operator= ( InfixIntvec_c&& rhs ) noexcept
	{
		if ( this!=&rhs )
		{
			Reset();
			m_uData = std::exchange ( rhs.m_uData, 0 );
		}
		return *this;
	}

	~InfixIntvec_c() { Reset(); }

	void Add ( DWORD uVal )
	{
		if ( !m_uData )
		{
			m_uData = INLINE_FLAG | ( uintptr_t(1) << INLINE_LEN_SHIFT ) | ( uintptr_t(uVal & INFIX_CHECKPOINT_ID_MAX) << INLINE_VALUE_SHIFT );
			return;
		}

		if ( !IsDynamic() )
		{
			int iLen = GetInlineLength();
			if ( uVal == GetInline ( iLen - 1 ) )
				return;

			if ( iLen==1 )
			{
				m_uData = ( m_uData & ~( uintptr_t(3) << INLINE_LEN_SHIFT ) )
					| ( uintptr_t(2) << INLINE_LEN_SHIFT )
					| ( uintptr_t(uVal & INFIX_CHECKPOINT_ID_MAX) << ( INLINE_VALUE_SHIFT + INLINE_VALUE_BITS ) );
				return;
			}

			auto* pDyn = new DWORD[DYNAMIC_INITIAL_LIMIT + 2];
			pDyn[0] = 3;
			pDyn[1] = DYNAMIC_INITIAL_LIMIT;
			pDyn[2] = GetInline(0);
			pDyn[3] = GetInline(1);
			pDyn[4] = uVal;
			m_uData = reinterpret_cast<uintptr_t>(pDyn);
			assert ( !( m_uData & INLINE_FLAG ) );
			return;
		}

		DWORD* pDyn = GetDynamic();
		int iLen = (int)pDyn[0];
		if ( uVal == pDyn[iLen + 1] )
			return;
		if ( iLen >= (int)pDyn[1] )
		{
			int iLimit = (int)pDyn[1] * 2;
			auto* pNew = new DWORD[iLimit + 2];
			memcpy ( pNew, pDyn, ( iLen + 2 ) * sizeof ( DWORD ) );
			pNew[1] = iLimit;
			delete[] pDyn;
			pDyn = pNew;
			m_uData = reinterpret_cast<uintptr_t>(pDyn);
		}

		pDyn[iLen + 2] = uVal;
		++pDyn[0];
	}

	int GetLength() const noexcept
	{
		return IsDynamic() ? (int)GetDynamic()[0] : GetInlineLength();
	}


	DWORD operator[] ( int iIndex ) const noexcept
	{
		return IsDynamic() ? GetDynamic()[iIndex + 2] : GetInline(iIndex);
	}
};

static_assert ( sizeof(InfixIntvec_c)==sizeof(uintptr_t) );
static_assert ( sizeof(uintptr_t)>=8, "infix checkpoint packing requires a 64-bit build" );


struct InfixHashEntry_t
{
	uint64_t m_uKey = 0;	///< packed key, owned by the hash
	InfixIntvec_c m_tValue; ///< data, owned by the hash
	int m_iNext;			///< next entry in hash arena
};


template<typename T>
class InfixArena_c
{
	static constexpr int BLOCK_LENGTH = 1 << 20;
	std::vector<std::unique_ptr<CSphSwapVector<T>>> m_dBlocks;
	int m_iLength = 0;

public:
	T& Add()
	{
		if ( !( m_iLength % BLOCK_LENGTH ) )
		{
			auto pBlock = std::make_unique<CSphSwapVector<T>>();
			pBlock->Reserve ( BLOCK_LENGTH );
			m_dBlocks.push_back ( std::move ( pBlock ) );
		}

		++m_iLength;
		return m_dBlocks.back()->Add();
	}

	int GetLength() const noexcept { return m_iLength; }
	int GetBlocksCount() const noexcept { return (int)m_dBlocks.size(); }
	int GetBlockLength ( int iBlock ) const noexcept { return m_dBlocks[iBlock]->GetLength(); }

	T& operator[] ( int iIndex ) noexcept
	{
		return (*m_dBlocks[iIndex / BLOCK_LENGTH])[iIndex % BLOCK_LENGTH];
	}

	const T& operator[] ( int iIndex ) const noexcept
	{
		return (*m_dBlocks[iIndex / BLOCK_LENGTH])[iIndex % BLOCK_LENGTH];
	}

	T& GetBlockEntry ( int iBlock, int iIndex ) noexcept { return (*m_dBlocks[iBlock])[iIndex]; }
	const T& GetBlockEntry ( int iBlock, int iIndex ) const noexcept { return (*m_dBlocks[iBlock])[iIndex]; }

	template<typename COMP>
	void SortBlocks ( COMP&& fnComp )
	{
		for ( int i = 0; i < GetBlocksCount(); ++i )
			std::sort ( m_dBlocks[i]->Begin() + ( i ? 0 : 1 ), m_dBlocks[i]->Begin() + m_dBlocks[i]->GetLength(), fnComp );
	}

	void Reset()
	{
		m_dBlocks.clear();
		m_iLength = 0;
	}
};


template<int SIZE>
class InfixBuilder_c final: public ISphInfixBuilder
{
	std::array<int, INFIX_ARENA_LENGTH> m_dHash; ///< all the hash entries
	InfixArena_c<InfixHashEntry_t> m_dArena;
	static constexpr int LONG_KEY_BLOCK_SHIFT = 20;
	static constexpr int LONG_KEY_BLOCK_LENGTH = 1 << LONG_KEY_BLOCK_SHIFT;
	std::vector<std::unique_ptr<CSphTightVector<BYTE>>> m_dLongKeyBlocks;
	CSphVector<InfixBlock_t> m_dBlocks;
	CSphTightVector<BYTE> m_dBlocksWords;
	CSphVector<int> m_dCodepointBytes;
	DictFormat_e m_eDictFormat = DictFormat_e::KEYWORDS;
	int m_iMinInfixLen = 2;

private:
	static constexpr uint64_t LONG_KEY_FLAG = 0x80;

	int GetKeyLength ( uint64_t uKey ) const noexcept { return (int)( uKey & 0x7f ); }
	bool IsLongKey ( uint64_t uKey ) const noexcept { return ( uKey & LONG_KEY_FLAG )!=0; }
	uint64_t GetLongKeyOffset ( uint64_t uKey ) const noexcept { return uKey >> 8; }

	uint64_t StoreKey ( const Infix_t<SIZE>& tKey, uint64_t uInlineKey, int iLength )
	{
		if ( iLength<=7 )
			return uInlineKey | iLength;

		if ( m_dLongKeyBlocks.empty() || m_dLongKeyBlocks.back()->GetLength() + iLength > LONG_KEY_BLOCK_LENGTH )
		{
			auto pBlock = std::make_unique<CSphTightVector<BYTE>>();
			pBlock->Reserve ( LONG_KEY_BLOCK_LENGTH );
			m_dLongKeyBlocks.push_back ( std::move(pBlock) );
		}

		uint64_t uOffset = ( uint64_t(m_dLongKeyBlocks.size()-1) << LONG_KEY_BLOCK_SHIFT ) | m_dLongKeyBlocks.back()->GetLength();
		assert ( uOffset < ( uint64_t(1) << 56 ) );
		m_dLongKeyBlocks.back()->Append ( tKey.m_Data.data(), iLength );
		return ( uOffset << 8 ) | LONG_KEY_FLAG | iLength;
	}

	const BYTE* GetLongKey ( uint64_t uKey ) const noexcept
	{
		uint64_t uOffset = GetLongKeyOffset(uKey);
		return m_dLongKeyBlocks[uOffset >> LONG_KEY_BLOCK_SHIFT]->Begin() + ( uOffset & ( LONG_KEY_BLOCK_LENGTH - 1 ) );
	}

	BYTE GetKeyByte ( uint64_t uKey, int iIndex ) const noexcept
	{
		return IsLongKey(uKey)
			? GetLongKey(uKey)[iIndex]
			: BYTE ( uKey >> ( 56 - iIndex*8 ) );
	}

	bool KeyEquals ( uint64_t uStored, const Infix_t<SIZE>& tKey, uint64_t uInlineKey, int iLength ) const noexcept
	{
		if ( iLength!=GetKeyLength(uStored) )
			return false;
		if ( !IsLongKey(uStored) )
			return uStored==( uInlineKey | iLength );
		return !memcmp ( GetLongKey(uStored), tKey.m_Data.data(), iLength );
	}

	bool KeyLess ( uint64_t a, uint64_t b ) const noexcept
	{
		if ( !IsLongKey(a) && !IsLongKey(b) )
			return a<b;

		int iLenA = GetKeyLength(a);
		int iLenB = GetKeyLength(b);
		int iCommon = Min ( iLenA, iLenB );
		for ( int i = 0; i < iCommon; ++i )
		{
			BYTE uA = GetKeyByte ( a, i );
			BYTE uB = GetKeyByte ( b, i );
			if ( uA!=uB )
				return uA<uB;
		}
		return iLenA<iLenB;
	}

	void UnpackKey ( uint64_t uKey, Infix_t<SIZE>& tKey ) const noexcept
	{
		tKey.Reset();
		for ( int i = 0; i < GetKeyLength(uKey); ++i )
			tKey.m_Data[i] = GetKeyByte ( uKey, i );
	}

	void AddEntry ( const Infix_t<SIZE>& tKey, uint64_t uInlineKey, int iKeyLength, DWORD uHash, int iCheckpoint )
	{
		uHash &= ( INFIX_ARENA_LENGTH - 1 );

		int iEntry = m_dArena.GetLength();
		InfixHashEntry_t& tNew = m_dArena.Add();
		tNew.m_uKey = StoreKey ( tKey, uInlineKey, iKeyLength );
		tNew.m_tValue.Add ( iCheckpoint ); // len=1, data=iCheckpoint
		tNew.m_iNext = std::exchange ( m_dHash[uHash], iEntry );
	}

	/// get value pointer by key
	InfixIntvec_c* LookupEntry ( const Infix_t<SIZE>& tKey, uint64_t uInlineKey, int iKeyLength, DWORD uHash )
	{
		uHash &= ( INFIX_ARENA_LENGTH - 1 );
		int iEntry = m_dHash[uHash];
		int iiEntry = 0;

		while ( iEntry )
		{
			if ( KeyEquals ( m_dArena[iEntry].m_uKey, tKey, uInlineKey, iKeyLength ) )
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
	InfixBuilder_c ( int iMinInfixLen, DictFormat_e eDictFormat )
		: m_eDictFormat ( eDictFormat )
		, m_iMinInfixLen ( Max ( 2, Min ( iMinInfixLen, 6 ) ) )
	{
		assert ( m_iMinInfixLen>=2 );
		// init the hash
		for ( auto& uHash : m_dHash )
			uHash = 0;
		m_dArena.Add(); // 0 is a reserved index
	}

	void AddWord ( const BYTE* pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) override;
	void SaveEntries ( CSphWriter& wrDict ) override;
	int64_t SaveEntryBlocks ( CSphWriter& wrDict ) override;
	int64_t GetBlocksWordsSize() const override { return m_dBlocksWords.GetLength64(); }
	int GetEntriesCount() const override { return Max ( 0, m_dArena.GetLength() - 1 ); }
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
	for ( int p = 0; p <= iWordLength - m_iMinInfixLen; ++p )
	{
		sKey.Reset();

		auto* pKey = sKey.m_Data.data();
		const BYTE* s = pWord + p;
		const BYTE* sMax = s + Min ( 6, iWordLength - p );

		BYTE uByte = *s++;
		DWORD uHash = CRC32_start ( uByte );
		*pKey++ = uByte; // copy first infix byte
		uint64_t uInlineKey = uint64_t(uByte) << 56;
		int iKeyLength = 1;
		int iChars = 1;

		while ( s < sMax )
		{
			uByte = *s++;
			CRC32_step ( uHash, uByte );
			*pKey++ = uByte; // copy another infix byte
			if ( iKeyLength<7 )
				uInlineKey |= uint64_t(uByte) << ( 56 - iKeyLength*8 );
			++iKeyLength;
			if ( ++iChars < m_iMinInfixLen )
				continue;

			InfixIntvec_c * pVal = LookupEntry ( sKey, uInlineKey, iKeyLength, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uInlineKey, iKeyLength, uHash, iCheckpoint );
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
	int iCodes = 0; // codepoints in current word

	// build an offsets table into the bytestring
	m_dCodepointBytes.Resize ( 1 );
	m_dCodepointBytes[0] = 0;
	for ( const BYTE* p = pWord; p < pWordMax; )
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

		m_dCodepointBytes.Add ( m_dCodepointBytes.Last()+iLen );
		++iCodes;
	}
	assert ( pWord[m_dCodepointBytes[iCodes]] == 0 || bInvalidTailCp );

	// generate infixes
	Infix_t<SIZE> sKey;
	for ( int p = 0; p <= iCodes - m_iMinInfixLen; ++p )
	{
		sKey.Reset();
		BYTE* pKey = sKey.m_Data.data();
		const BYTE* pKeyMax = pKey + sizeof ( sKey.m_Data );

		const BYTE* s = pWord + m_dCodepointBytes[p];
		const BYTE* sMax = pWord + m_dCodepointBytes[p + Min ( 6, iCodes - p )];

		// copy first infix codepoint
		DWORD uHash = 0xffffffffUL;
		uint64_t uInlineKey = 0;
		int iKeyLength = 0;
		do
		{
			BYTE uByte = *s++;
			CRC32_step ( uHash, uByte );
			*pKey++ = uByte;
			if ( iKeyLength<7 )
				uInlineKey |= uint64_t(uByte) << ( 56 - iKeyLength*8 );
			++iKeyLength;
		} while ( ( *s & 0xC0 ) == 0x80 );

		assert ( s - ( pWord + m_dCodepointBytes[p] ) == ( m_dCodepointBytes[p + 1] - m_dCodepointBytes[p] ) );
		int iChars = 1;

		while ( s < sMax && pKey < pKeyMax && pKey + sphUtf8CharBytes ( *s ) <= pKeyMax )
		{
			// copy next infix codepoint
			do
			{
				BYTE uByte = *s++;
				CRC32_step ( uHash, uByte );
				*pKey++ = uByte;
				if ( iKeyLength<7 )
					uInlineKey |= uint64_t(uByte) << ( 56 - iKeyLength*8 );
				++iKeyLength;
			} while ( ( *s & 0xC0 ) == 0x80 && pKey < pKeyMax );

			assert ( sphUTF8Len ( (const char*)sKey.m_Data.data(), sizeof ( sKey.m_Data ) ) >= 2 );
			if ( ++iChars < m_iMinInfixLen )
				continue;

			InfixIntvec_c* pVal = LookupEntry ( sKey, uInlineKey, iKeyLength, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uInlineKey, iKeyLength, uHash, iCheckpoint );
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

	auto fnEntryLess = [this] ( const InfixHashEntry_t& a, const InfixHashEntry_t& b ) noexcept
	{
		return KeyLess ( a.m_uKey, b.m_uKey );
	};
	m_dArena.SortBlocks ( fnEntryLess );

	struct Cursor_t { int m_iBlock; int m_iEntry; };
	auto fnCursorGreater = [this] ( const Cursor_t& a, const Cursor_t& b ) noexcept
	{
		return KeyLess ( m_dArena.GetBlockEntry ( b.m_iBlock, b.m_iEntry ).m_uKey,
			m_dArena.GetBlockEntry ( a.m_iBlock, a.m_iEntry ).m_uKey );
	};
	std::priority_queue<Cursor_t, std::vector<Cursor_t>, decltype(fnCursorGreater)> qEntries ( fnCursorGreater );
	for ( int i = 0; i < m_dArena.GetBlocksCount(); ++i )
		if ( m_dArena.GetBlockLength(i) > ( i ? 0 : 1 ) )
			qEntries.push ( { i, i ? 0 : 1 } );

	m_dBlocksWords.Reserve ( m_dArena.GetLength() / INFIX_BLOCK_SIZE * sizeof ( DWORD ) * SIZE );
	int iBlock = 0;
	Infix_t<SIZE> tPrevKey;
	bool bHavePrevKey = false;
	constexpr size_t DWSIZE = sizeof ( DWORD ) * SIZE;
	while ( !qEntries.empty() )
	{
		Cursor_t tCursor = qEntries.top();
		qEntries.pop();
		InfixHashEntry_t& tEntry = m_dArena.GetBlockEntry ( tCursor.m_iBlock, tCursor.m_iEntry );
		InfixIntvec_c& dData = tEntry.m_tValue;
		Infix_t<SIZE> tKey;
		UnpackKey ( tEntry.m_uKey, tKey );
		const char* sKey = (const char*)tKey.m_Data.data();
		int iChars = ( SIZE == 2 )
					   ? (int)strnlen ( sKey, DWSIZE )
					   : sphUTF8Len ( sKey, (int)DWSIZE );
		assert ( iChars >= 2 && iChars < int ( 1 + sizeof ( Infix_t<SIZE> ) ) );

		// keep track of N-infix blocks
		auto iAppendBytes = (int)strnlen ( sKey, DWSIZE );
		if ( !iBlock )
		{
			int64_t iOff = m_dBlocksWords.GetLength64();
			m_dBlocksWords.Resize ( iOff + iAppendBytes + 1 );

			InfixBlock_t& tBlock = m_dBlocks.Add();
			tBlock.m_iInfixOffset = iOff;
			tBlock.m_iOffset = wrDict.GetPos();

			memcpy ( m_dBlocksWords.Begin() + iOff, sKey, iAppendBytes );
			m_dBlocksWords[iOff + iAppendBytes] = '\0';
		}

		// compute max common prefix
		// edit_code = ( num_keep_chars<<4 ) + num_append_chars
		int iEditCode = iChars;
		if ( bHavePrevKey )
		{
			const char* sPrev = (const char*)tPrevKey.m_Data.data();
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
		tPrevKey = tKey;
		bHavePrevKey = true;
		if ( ++iBlock == INFIX_BLOCK_SIZE )
		{
			iBlock = 0;
			bHavePrevKey = false;
			wrDict.PutByte ( 0 );
		}

		if ( ++tCursor.m_iEntry < m_dArena.GetBlockLength ( tCursor.m_iBlock ) )
			qEntries.push ( tCursor );
	}

	// put end marker
	if ( iBlock )
		wrDict.PutByte ( 0 );

	const char* pBlockWords = (const char*)m_dBlocksWords.Begin();
	ARRAY_FOREACH ( i, m_dBlocks )
		m_dBlocks[i].m_sInfix = pBlockWords + m_dBlocks[i].m_iInfixOffset;

	if ( m_eDictFormat!=DictFormat_e::KEYWORDS_V2 && wrDict.GetPos() > UINT_MAX )
		sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", wrDict.GetPos() );

	m_dArena.Reset();
	m_dLongKeyBlocks.clear();
}


template<int SIZE>
int64_t InfixBuilder_c<SIZE>::SaveEntryBlocks ( CSphWriter& wrDict )
{
	// save the blocks
	wrDict.PutBlob ( g_sTagInfixBlocks );

	SphOffset_t iInfixBlocksOffset = wrDict.GetPos();
	assert ( m_eDictFormat==DictFormat_e::KEYWORDS_V2 || iInfixBlocksOffset <= INT_MAX );

	if ( m_eDictFormat==DictFormat_e::KEYWORDS_V2 )
		wrDict.ZipOffset ( m_dBlocks.GetLength64() );
	else
		wrDict.ZipInt ( m_dBlocks.GetLength() );
	ARRAY_FOREACH ( i, m_dBlocks )
	{
		auto iBytes = strlen ( m_dBlocks[i].m_sInfix );
		wrDict.PutByte ( BYTE ( iBytes ) );
		wrDict.PutBytes ( m_dBlocks[i].m_sInfix, iBytes );
		if ( m_eDictFormat==DictFormat_e::KEYWORDS_V2 )
			wrDict.ZipOffset ( m_dBlocks[i].m_iOffset ); // maybe delta these on top?
		else
			wrDict.ZipInt ( (DWORD)m_dBlocks[i].m_iOffset ); // maybe delta these on top?
	}

	return iInfixBlocksOffset;
}


std::unique_ptr<ISphInfixBuilder> sphCreateInfixBuilder ( int iCodepointBytes, int iMinInfixLen, CSphString* pError, DictFormat_e eDictFormat )
{
	assert ( pError );
	switch ( iCodepointBytes )
	{
	case 0: return nullptr;
	case 1: return std::make_unique<InfixBuilder_c<2>>( iMinInfixLen, eDictFormat ); // upto 6x1 bytes, 2 dwords, sbcs
	case 2: return std::make_unique<InfixBuilder_c<3>>( iMinInfixLen, eDictFormat ); // upto 6x2 bytes, 3 dwords, utf-8
	case 3: return std::make_unique<InfixBuilder_c<5>>( iMinInfixLen, eDictFormat ); // upto 6x3 bytes, 5 dwords, utf-8
	default: pError->SetSprintf ( "unhandled max infix codepoint size %d", iCodepointBytes ); return nullptr;
	}
}
