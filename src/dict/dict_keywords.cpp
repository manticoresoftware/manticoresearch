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

#include "dict_crc.h"
#include "bin.h"
#include "dict/infix/infix_builder.h"
//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY
//////////////////////////////////////////////////////////////////////////

/// binary search for the first hit with wordid greater than or equal to reference
static CSphWordHit* FindFirstGte ( CSphWordHit* pHits, int iHits, SphWordID_t uID )
{
	if ( pHits->m_uWordID == uID )
		return pHits;

	CSphWordHit* pL = pHits;
	CSphWordHit* pR = pHits + iHits - 1;
	if ( pL->m_uWordID > uID || pR->m_uWordID < uID )
		return NULL;

	while ( pR - pL != 1 )
	{
		CSphWordHit* pM = pL + ( pR - pL ) / 2;
		if ( pM->m_uWordID < uID )
			pL = pM;
		else
			pR = pM;
	}

	assert ( pR - pL == 1 );
	assert ( pL->m_uWordID < uID );
	assert ( pR->m_uWordID >= uID );
	return pR;
}

//////////////////////////////////////////////////////////////////////////
struct KeywordDictFinalization_t : public ISphNonCopyMovable
{
	std::unique_ptr<ISphInfixBuilder> m_pInfixer;
	int m_iWords = 0;
	CSphKeywordDeltaWriter m_tLastKeyword;
	CSphKeywordDeltaWriterV2 m_tLastKeywordV2;

	void ResetLastKeywordKw()
	{
		m_tLastKeyword.Reset();
	}

	void ResetLastKeywordV2()
	{
		m_tLastKeywordV2.Reset();
	}

	template <typename F>
	void PutDeltaKw ( F & WRITER, const BYTE * pWord, int iLen )
	{
		m_tLastKeyword.PutDelta ( WRITER, pWord, iLen );
	}

	template <typename F>
	void PutDeltaV2 ( F & WRITER, const BYTE * pWord, int iLen )
	{
		m_tLastKeywordV2.PutDelta ( WRITER, pWord, iLen );
	}
};

static bool CheckKeywordsV2InfixCheckpoint ( DictFormat_e eDictFormat, int64_t iCheckpoint, CSphString & sError )
{
	if ( eDictFormat!=DictFormat_e::KEYWORDS_V2 || iCheckpoint<=INFIX_CHECKPOINT_ID_MAX )
		return true;

	sError.SetSprintf ( "keywords_32k infix checkpoint id " INT64_FMT " exceeds supported limit %u; checkpoint-id widening is outside 4 GB .spi offset support",
		iCheckpoint, INFIX_CHECKPOINT_ID_MAX );
	return false;
}

class CSphDictKeywords final: public CSphDictCRC<CRCALGO::CRC32>
{
	using BASE = CSphDictCRC<CRCALGO::CRC32>;
private:
	// OPTIMIZE? change pointers to 8:24 locators to save RAM on x64 gear?
	struct HitblockKeyword_t
	{
		SphWordID_t m_uWordid;			// locally unique word id (crc value, adjusted in case of collision)
		HitblockKeyword_t* m_pNextHash; // next hashed entry
		char* m_pKeyword;				// keyword
	};

	struct HitblockException_t
	{
		HitblockKeyword_t* m_pEntry; // hash entry
		SphWordID_t m_uCRC;			 // original unadjusted crc

		bool operator<( const HitblockException_t& rhs ) const
		{
			return m_pEntry->m_uWordid < rhs.m_pEntry->m_uWordid;
		}
	};

	struct DictKeyword_t
	{
		char* m_sKeyword;
		SphOffset_t m_uOff;
		int m_iDocs;
		int m_iHits;
		BYTE m_uHint;
		int m_iKeywordLen;
		int64_t m_iSkiplistPos; ///< position in .spe file
	};

	struct DictBlock_t
	{
		SphOffset_t m_iPos;
		int m_iLen;
	};

public:
	explicit CSphDictKeywords();

	void HitblockBegin() final { m_bHitblock = true; }
	void HitblockPatch ( CSphWordHit* pHits, int iHits ) const final;
	const char* HitblockGetKeyword ( SphWordID_t uWordID ) final;
	int HitblockGetMemUse() final { return m_iMemUse; }
	void HitblockReset() final;

	void DictBegin ( CSphAutofile& tTempDict, CSphAutofile& tDict, int iDictLimit ) final;
	void SortedDictBegin ( CSphAutofile& tDict, int iDictLimit, int iInfixCodepointBytes ) final;
	void DictEntry ( const DictEntry_t& tEntry ) final;
	void DictEndEntries ( SphOffset_t ) final {};
	bool DictEnd ( DictHeader_t* pHeader, int iMemLimit, CSphString& sError ) final;

	SphWordID_t GetWordID ( BYTE* pWord ) final;
	SphWordID_t GetWordIDWithMarkers ( BYTE* pWord ) final;
	SphWordID_t GetWordIDNonStemmed ( BYTE* pWord ) final;
	SphWordID_t GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops ) final;
	DictRefPtr_c Clone() const final { return CloneBase ( new CSphDictKeywords() ); }

	/// full crc and keyword check
	inline static bool FullIsLess ( const HitblockException_t& a, const HitblockException_t& b ) noexcept
	{
		if ( a.m_uCRC != b.m_uCRC )
			return a.m_uCRC < b.m_uCRC;
		return strcmp ( a.m_pEntry->m_pKeyword, b.m_pEntry->m_pKeyword ) < 0;
	}

protected:
	~CSphDictKeywords() final;

private:
	struct DictKeywordTagged_t: public DictKeyword_t
	{
		int m_iBlock;

		static inline bool IsLess ( const DictKeywordTagged_t& a, const DictKeywordTagged_t& b );
	};

	static const int SLOTS = 65536;
	static const int ENTRY_CHUNK = 65536;
	static const int KEYWORD_CHUNK = 1048576;
	static const int DICT_CHUNK = 65536;

	HitblockKeyword_t* m_dHash[SLOTS]; ///< hash by wordid (!)
	CSphVector<HitblockException_t> m_dExceptions;

	bool m_bHitblock; ///< should we store words on GetWordID or not
	int m_iMemUse;	  ///< current memory use by all the chunks
	int m_iDictLimit; ///< allowed memory limit for dict block collection

	CSphVector<HitblockKeyword_t*> m_dEntryChunks; ///< hash chunks, only used when indexing hitblocks
	HitblockKeyword_t* m_pEntryChunk;
	int m_iEntryChunkFree;

	CSphVector<BYTE*> m_dKeywordChunks; ///< keyword storage
	BYTE* m_pKeywordChunk;
	int m_iKeywordChunkFree;

	CSphVector<DictKeyword_t*> m_dDictChunks; ///< dict entry chunks, only used when sorting final dict
	DictKeyword_t* m_pDictChunk;
	int m_iDictChunkFree;

	int m_iTmpFD;						   ///< temp dict file descriptor
	CSphWriter m_wrTmpDict;				   ///< temp dict writer
	CSphVector<DictBlock_t> m_dDictBlocks; ///< on-disk locations of dict entry blocks

	std::array<char, MAX_KEYWORD_BYTES> m_sClippedWord; ///< keyword storage for clipped word
	std::unique_ptr<KeywordDictFinalization_t> m_pFinalizer;
	CSphString m_sError;

private:
	SphWordID_t HitblockGetID ( const char* pWord, int iLen, SphWordID_t uCRC );
	HitblockKeyword_t* HitblockAddKeyword ( DWORD uHash, const char* pWord, int iLen, SphWordID_t uID );

	inline bool IsSorted() const noexcept { return m_iDictLimit==0; }
	bool DictReadEntry ( CSphBin& dBin, DictKeywordTagged_t& tEntry, CSphVector<BYTE>& dKeyword, CSphString& sError );
	void DictFlush();
	void DictEntryNonSorted ( const DictEntry_t& tEntry );
	bool SortedDictEnd ( DictHeader_t* pHeader, CSphString& sError );
	void PutBlockTerminator();
	void PutCheckpoint ( const CSphWordlistCheckpoint& tCheckpoint );
	bool CheckKeywordLength ( const char* sContext, int iLen );
	void ResetDictChunks();
	void ResetCheckpoints();
};

//////////////////////////////////////////////////////////////////////////

CSphDictKeywords::CSphDictKeywords()
	: m_bHitblock ( false )
	, m_iMemUse ( 0 )
	, m_iDictLimit ( 0 )
	, m_pEntryChunk ( nullptr )
	, m_iEntryChunkFree ( 0 )
	, m_pKeywordChunk ( nullptr )
	, m_iKeywordChunkFree ( 0 )
	, m_pDictChunk ( nullptr )
	, m_iDictChunkFree ( 0 )
	, m_iTmpFD ( -1 )
{
	memset ( m_dHash, 0, sizeof ( m_dHash ) );
}

CSphDictKeywords::~CSphDictKeywords()
{
	HitblockReset();
	ResetDictChunks();
	ResetCheckpoints();
}

void CSphDictKeywords::HitblockReset()
{
	m_dExceptions.Resize ( 0 );

	ARRAY_FOREACH ( i, m_dEntryChunks )
		SafeDeleteArray ( m_dEntryChunks[i] );
	m_dEntryChunks.Resize ( 0 );
	m_pEntryChunk = NULL;
	m_iEntryChunkFree = 0;

	ARRAY_FOREACH ( i, m_dKeywordChunks )
		SafeDeleteArray ( m_dKeywordChunks[i] );
	m_dKeywordChunks.Resize ( 0 );
	m_pKeywordChunk = NULL;
	m_iKeywordChunkFree = 0;

	m_iMemUse = 0;

	memset ( m_dHash, 0, sizeof ( m_dHash ) );
}

CSphDictKeywords::HitblockKeyword_t* CSphDictKeywords::HitblockAddKeyword ( DWORD uHash, const char* sWord, int iLen, SphWordID_t uID )
{
	assert ( iLen>0 && iLen<=GetKeywordMaxStoredBytes ( GetSettings().GetDictFormat() ) );

	// alloc entry
	if ( !m_iEntryChunkFree )
	{
		m_pEntryChunk = new HitblockKeyword_t[ENTRY_CHUNK];
		m_iEntryChunkFree = ENTRY_CHUNK;
		m_dEntryChunks.Add ( m_pEntryChunk );
		m_iMemUse += sizeof ( HitblockKeyword_t ) * ENTRY_CHUNK;
	}
	HitblockKeyword_t* pEntry = m_pEntryChunk++;
	m_iEntryChunkFree--;

	// alloc keyword
	iLen++;
	if ( m_iKeywordChunkFree < iLen )
	{
		m_pKeywordChunk = new BYTE[KEYWORD_CHUNK];
		m_iKeywordChunkFree = KEYWORD_CHUNK;
		m_dKeywordChunks.Add ( m_pKeywordChunk );
		m_iMemUse += KEYWORD_CHUNK;
	}

	// fill it
	memcpy ( m_pKeywordChunk, sWord, iLen );
	m_pKeywordChunk[iLen - 1] = '\0';
	pEntry->m_pKeyword = (char*)m_pKeywordChunk;
	pEntry->m_uWordid = uID;
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	// mtf it
	pEntry->m_pNextHash = m_dHash[uHash];
	m_dHash[uHash] = pEntry;

	return pEntry;
}

SphWordID_t CSphDictKeywords::HitblockGetID ( const char* sWord, int iLen, SphWordID_t uCRC )
{
	if ( !m_sError.IsEmpty() )
		return uCRC;

	if ( GetSettings().IsKeywordsV2() )
	{
		if ( !CheckKeywordLength ( "hitblock", iLen ) )
			return uCRC;
	} else if ( iLen > MAX_KEYWORD_BYTES - 4 ) // fix of very long word (zones)
	{
		memcpy ( m_sClippedWord.data(), sWord, MAX_KEYWORD_BYTES - 4 );
		memset ( m_sClippedWord.data() + MAX_KEYWORD_BYTES - 4, 0, 4 );

		CSphString sOrig;
		sOrig.SetBinary ( sWord, iLen );
		sphWarn ( "word overrun buffer, clipped!!!\n"
				  "clipped (len=%d, word='%s')\noriginal (len=%d, word='%s')",
			MAX_KEYWORD_BYTES - 4,
			m_sClippedWord.data(),
			iLen,
			sOrig.cstr() );

		sWord = m_sClippedWord.data();
		iLen = MAX_KEYWORD_BYTES - 4;
		uCRC = sphCRC32 ( m_sClippedWord.data(), MAX_KEYWORD_BYTES - 4 );
	}

	// is this a known one? find it
	// OPTIMIZE? in theory, we could use something faster than crc32; but quick lookup3 test did not show any improvements
	const DWORD uHash = (DWORD)( uCRC % SLOTS );

	HitblockKeyword_t* pEntry = m_dHash[uHash];
	HitblockKeyword_t** ppEntry = &m_dHash[uHash];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid != uCRC )
		{
			// crc mismatch, try next entry
			ppEntry = &pEntry->m_pNextHash;
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		// crc matches, check keyword
		int iWordLen = iLen;
		const char* a = pEntry->m_pKeyword;
		const char* b = sWord;
		while ( *a == *b && iWordLen-- )
		{
			if ( !*a || !iWordLen )
			{
				// known word, mtf it, and return id
				( *ppEntry ) = pEntry->m_pNextHash;
				pEntry->m_pNextHash = m_dHash[uHash];
				m_dHash[uHash] = pEntry;
				return pEntry->m_uWordid;
			}
			++a;
			++b;
		}

		// collision detected!
		// our crc is taken as a wordid, but keyword does not match
		// welcome to the land of very tricky magic
		//
		// pEntry might either be a known exception, or a regular keyword
		// sWord might either be a known exception, or a new one
		// if they are not known, they needed to be added as exceptions now
		//
		// in case sWord is new, we need to assign a new unique wordid
		// for that, we keep incrementing the crc until it is unique
		// a starting point for wordid search loop would be handy
		//
		// let's scan the exceptions vector and work on all this
		//
		// NOTE, beware of the order, it is wordid asc, which does NOT guarantee crc asc
		// example, assume crc(w1)==X, crc(w2)==X+1, crc(w3)==X (collides with w1)
		// wordids will be X, X+1, X+2 but crcs will be X, X+1, X
		//
		// OPTIMIZE, might make sense to use binary search
		// OPTIMIZE, add early out somehow
		SphWordID_t uWordid = uCRC + 1;
		const int iExcLen = m_dExceptions.GetLength();
		int iExc = m_dExceptions.GetLength();
		ARRAY_FOREACH ( i, m_dExceptions )
		{
			const HitblockKeyword_t* pExcWord = m_dExceptions[i].m_pEntry;

			// incoming word is a known exception? just return the pre-assigned wordid
			if ( m_dExceptions[i].m_uCRC == uCRC && strncmp ( pExcWord->m_pKeyword, sWord, iLen ) == 0 )
				return pExcWord->m_uWordid;

			// incoming word collided into a known exception? clear the matched entry; no need to re-add it (see below)
			if ( pExcWord == pEntry )
				pEntry = nullptr;

			// find first exception with wordid greater or equal to our candidate
			if ( pExcWord->m_uWordid >= uWordid && iExc == iExcLen )
				iExc = i;
		}

		// okay, this is a new collision
		// if entry was a regular word, we have to add it
		if ( pEntry )
		{
			m_dExceptions.Add();
			m_dExceptions.Last().m_pEntry = pEntry;
			m_dExceptions.Last().m_uCRC = uCRC;
		}

		// need to assign a new unique wordid now
		// keep scanning both exceptions and keywords for collisions
		while ( true )
		{
			// iExc must be either the first exception greater or equal to current candidate, or out of bounds
			assert ( iExc == iExcLen || m_dExceptions[iExc].m_pEntry->m_uWordid >= uWordid );
			assert ( iExc == 0 || m_dExceptions[iExc - 1].m_pEntry->m_uWordid < uWordid );

			// candidate collides with a known exception? increment it, and keep looking
			if ( iExc < iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid == uWordid )
			{
				++uWordid;
				while ( iExc < iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid < uWordid )
					++iExc;
				continue;
			}

			// candidate collides with a keyword? must be a regular one; add it as an exception, and keep looking
			HitblockKeyword_t* pCheck = m_dHash[(DWORD)( uWordid % SLOTS )];
			while ( pCheck )
			{
				if ( pCheck->m_uWordid == uWordid )
					break;
				pCheck = pCheck->m_pNextHash;
			}

			// no collisions; we've found our unique wordid!
			if ( !pCheck )
				break;

			// got a collision; add it
			HitblockException_t& tColl = m_dExceptions.Add();
			tColl.m_pEntry = pCheck;
			tColl.m_uCRC = pCheck->m_uWordid; // not a known exception; hence, wordid must equal crc

			// and keep looking
			++uWordid;
			continue;
		}

		// and finally, we have that precious new wordid
		// so hash our new unique under its new unique adjusted wordid
		pEntry = HitblockAddKeyword ( (DWORD)( uWordid % SLOTS ), sWord, iLen, uWordid );

		// add it as a collision too
		m_dExceptions.Add();
		m_dExceptions.Last().m_pEntry = pEntry;
		m_dExceptions.Last().m_uCRC = uCRC;

		// keep exceptions list sorted by wordid
		m_dExceptions.Sort();

		return pEntry->m_uWordid;
	}

	// new keyword with unique crc
	pEntry = HitblockAddKeyword ( uHash, sWord, iLen, uCRC );
	return pEntry->m_uWordid;
}


inline bool CSphDictKeywords::DictKeywordTagged_t::IsLess ( const DictKeywordTagged_t& a, const DictKeywordTagged_t& b )
{
	return strcmp ( a.m_sKeyword, b.m_sKeyword ) < 0;
};


bool CSphDictKeywords::CheckKeywordLength ( const char* sContext, int iLen )
{
	const DictFormat_e eDictFormat = GetSettings().GetDictFormat();
	const int iMaxLen = GetKeywordMaxStoredBytes ( eDictFormat );
	if ( iLen>0 && iLen<=iMaxLen )
		return true;

	if ( m_sError.IsEmpty() )
		m_sError.SetSprintf ( "%s %s keyword length %d exceeds limit %d", DictFormatName ( eDictFormat ), sContext, iLen, iMaxLen );
	return false;
}


bool CSphDictKeywords::DictReadEntry ( CSphBin& dBin, DictKeywordTagged_t& tEntry, CSphVector<BYTE>& dKeyword, CSphString& sError )
{
	int iKeywordLen = GetSettings().IsKeywordsV2() ? (int)dBin.UnzipInt() : dBin.ReadByte();
	if ( iKeywordLen < 0 )
	{
		// early eof or read error; flag must be raised
		assert ( dBin.IsError() );
		return true;
	}

	assert ( iKeywordLen > 0 && iKeywordLen <= GetKeywordMaxStoredBytes ( GetSettings().GetDictFormat() ) );
	if ( iKeywordLen<=0 || iKeywordLen>GetKeywordMaxStoredBytes ( GetSettings().GetDictFormat() ) )
	{
		if ( m_sError.IsEmpty() )
			m_sError.SetSprintf ( "corrupted %s temp dictionary keyword length %d", DictFormatName ( GetSettings().GetDictFormat() ), iKeywordLen );
		sError = m_sError;
		return false;
	}

	dKeyword.Resize ( iKeywordLen+1 );
	if ( dBin.ReadBytes ( dKeyword.Begin(), iKeywordLen ) != BIN_READ_OK )
	{
		assert ( dBin.IsError() );
		return true;
	}
	dKeyword[iKeywordLen] = '\0';

	assert ( m_iSkiplistBlockSize > 0 );

	tEntry.m_sKeyword = (char*)dKeyword.Begin();
	tEntry.m_iKeywordLen = iKeywordLen;
	tEntry.m_uOff = dBin.UnzipOffset();
	tEntry.m_iDocs = dBin.UnzipInt();
	tEntry.m_iHits = dBin.UnzipInt();
	tEntry.m_uHint = (BYTE)dBin.ReadByte();
	if ( ( tEntry.m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize )
		tEntry.m_iSkiplistPos = dBin.UnzipOffset();
	else
		tEntry.m_iSkiplistPos = 0;
	return true;
}


void CSphDictKeywords::ResetDictChunks()
{
	m_dDictChunks.for_each ( [] ( auto& dChunk ) { delete[] ( dChunk ); } );
	m_dDictChunks.Resize ( 0 );
	m_pDictChunk = nullptr;
	m_iDictChunkFree = 0;

	m_dKeywordChunks.for_each ( [] ( auto& dChunk ) { delete[] ( dChunk ); } );
	m_dKeywordChunks.Resize ( 0 );
	m_pKeywordChunk = nullptr;
	m_iKeywordChunkFree = 0;

	m_iMemUse = 0;
}


void CSphDictKeywords::ResetCheckpoints()
{
	ARRAY_FOREACH ( i, m_dCheckpoints )
		SafeDeleteArray ( m_dCheckpoints[i].m_szWord );
	m_dCheckpoints.Reset();
}


void CSphDictKeywords::DictBegin ( CSphAutofile& tTempDict, CSphAutofile& tDict, int iDictLimit )
{
	m_sError = CSphString();
	m_iTmpFD = tTempDict.GetFD();

	m_iDictLimit = Max ( iDictLimit, KEYWORD_CHUNK + DICT_CHUNK * (int)sizeof ( DictKeyword_t ) ); // can't use less than 1 chunk

	m_wrTmpDict.CloseFile();
	m_wrTmpDict.SetBufferSize(m_iDictLimit);
	m_wrTmpDict.SetFile ( tTempDict, nullptr, m_sWriterError );

	m_wrDict.CloseFile();
	m_wrDict.SetFile ( tDict, nullptr, m_sWriterError );
	m_wrDict.PutByte ( 1 );
}

void CSphDictKeywords::SortedDictBegin ( CSphAutofile& tDict, int iDictLimit, int iInfixCodepointBytes )
{
	m_sError = CSphString();
	m_iTmpFD = -1;
	m_wrDict.CloseFile();
	m_wrDict.SetFile ( tDict, nullptr, m_sWriterError );
	m_wrDict.PutByte ( 1 );
	assert ( m_wrDict.GetPos() == 1 );

	m_iDictLimit = 0; // 0 assumes we have sorted

	m_pFinalizer = std::make_unique<KeywordDictFinalization_t>();
	CSphString sError;
	m_pFinalizer->m_pInfixer = sphCreateInfixBuilder ( iInfixCodepointBytes, &sError, GetSettings().GetDictFormat() );
	assert ( sError.IsEmpty() );
}

void CSphDictKeywords::PutBlockTerminator()
{
	m_wrDict.ZipInt ( 0 );
	m_wrDict.ZipInt ( 0 );
}

void CSphDictKeywords::PutCheckpoint ( const CSphWordlistCheckpoint& tCheckpoint )
{
	auto iLen = (const int)strlen ( tCheckpoint.m_szWord );

	assert ( tCheckpoint.m_iWordlistOffset > 0 );
	assert ( iLen > 0 && iLen <= GetKeywordMaxStoredBytes ( GetSettings().GetDictFormat() ) );

	if ( GetSettings().IsKeywordsV2() )
		m_wrDict.ZipInt ( iLen );
	else
		m_wrDict.PutDword ( iLen );

	m_wrDict.PutBytes ( tCheckpoint.m_szWord, iLen );
	m_wrDict.PutOffset ( tCheckpoint.m_iWordlistOffset );
}

bool CSphDictKeywords::DictEnd ( DictHeader_t* pHeader, int iMemLimit, CSphString& sError )
{
	if ( IsSorted() )
		return SortedDictEnd ( pHeader, sError );

	assert ( !IsSorted() );
	DictFlush();
	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		return false;
	}

	m_wrTmpDict.CloseFile(); // tricky: file is not owned, so it won't get closed, and iTmpFD won't get invalidated

	if ( m_dDictBlocks.IsEmpty() )
		m_wrDict.CloseFile();

	if ( m_wrTmpDict.IsError() || m_wrDict.IsError() )
	{
		sError.SetSprintf ( "dictionary write error (out of space?)" );
		return false;
	}

	if ( m_dDictBlocks.IsEmpty() )
	{
		pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
		pHeader->m_iDictCheckpoints = 0;
		return true;
	}

	m_pFinalizer = std::make_unique<KeywordDictFinalization_t>();

	// infix builder, if needed
	m_pFinalizer->m_pInfixer = sphCreateInfixBuilder ( pHeader->m_iInfixCodepointBytes, &sError, GetSettings().GetDictFormat() );
	if ( !sError.IsEmpty() )
		return false;

	assert ( m_iSkiplistBlockSize > 0 );

	// initialize readers
	RawVector_T<CSphBin> dBins;
	dBins.Reserve_static ( m_dDictBlocks.GetLength() );

	int iMaxBlock = 0;
	ARRAY_FOREACH ( i, m_dDictBlocks )
		iMaxBlock = Max ( iMaxBlock, m_dDictBlocks[i].m_iLen );

	iMemLimit = Max ( iMemLimit, iMaxBlock * m_dDictBlocks.GetLength() );
	int iBinSize = CSphBin::CalcBinSize ( iMemLimit, m_dDictBlocks.GetLength(), "sort_dict" );
	iBinSize = Max ( iBinSize, GetKeywordBufSize ( GetSettings().GetDictFormat() ) );

	SphOffset_t iSharedOffset = -1;
	ARRAY_FOREACH ( i, m_dDictBlocks )
	{
		auto& dBin = dBins.Add();
		dBin.m_iFileLeft = m_dDictBlocks[i].m_iLen;
		dBin.m_iFilePos = m_dDictBlocks[i].m_iPos;
		dBin.Init ( m_iTmpFD, &iSharedOffset, iBinSize );
	}

	// keywords storage; queue entries point into per-bin buffers until that bin advances
	CSphVector<CSphVector<BYTE>> dKeywords;
	dKeywords.Resize ( dBins.GetLength() );

	// do the sort
	CSphQueue<DictKeywordTagged_t, DictKeywordTagged_t> qWords ( dBins.GetLength() );
	DictKeywordTagged_t tEntry;

	ARRAY_FOREACH ( i, dBins )
	{
		if ( !DictReadEntry ( dBins[i], tEntry, dKeywords[i], sError ) )
			return false;
		if ( dBins[i].IsError() )
		{
			sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", i, dBins.GetLength() );
			return false;
		}

		tEntry.m_iBlock = i;
		qWords.Push ( tEntry );
	}

	bool bHasMorphology = HasMorphology();
	int iWords = 0;
	while ( qWords.GetLength() )
	{
		const DictKeywordTagged_t& tWord = qWords.Root();
		auto iLen = (const int)tWord.m_iKeywordLen;

		// store checkpoints as needed
		if ( ( iWords % SPH_WORDLIST_CHECKPOINT ) == 0 )
		{
			// emit a checkpoint, unless we're at the very dict beginning
			if ( iWords )
				PutBlockTerminator();

			BYTE* szClone = new BYTE[iLen + 1]; // OPTIMIZE? pool these?
			memcpy ( szClone, tWord.m_sKeyword, iLen );
			szClone[iLen] = '\0';

			CSphWordlistCheckpoint& tCheckpoint = m_dCheckpoints.Add();
			tCheckpoint.m_szWord = (char*)szClone;
			tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();

			if ( GetSettings().IsKeywordsV2() )
				m_pFinalizer->ResetLastKeywordV2();
			else
				m_pFinalizer->ResetLastKeywordKw();
		}
		++iWords;

		// write final dict entry
		assert ( iLen );
		assert ( tWord.m_uOff );
		assert ( tWord.m_iDocs );
		assert ( tWord.m_iHits );

		if ( GetSettings().IsKeywordsV2() )
			m_pFinalizer->PutDeltaV2 ( m_wrDict, (const BYTE*)tWord.m_sKeyword, iLen );
		else
			m_pFinalizer->PutDeltaKw ( m_wrDict, (const BYTE*)tWord.m_sKeyword, iLen );
		m_wrDict.ZipOffset ( tWord.m_uOff );
		m_wrDict.ZipInt ( tWord.m_iDocs );
		m_wrDict.ZipInt ( tWord.m_iHits );
		if ( tWord.m_uHint )
			m_wrDict.PutByte ( tWord.m_uHint );
		if ( ( tWord.m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize )
			m_wrDict.ZipOffset ( tWord.m_iSkiplistPos );

		// build infixes
		if ( m_pFinalizer->m_pInfixer )
		{
			int64_t iCheckpoint = m_dCheckpoints.GetLength64();
			if ( !CheckKeywordsV2InfixCheckpoint ( GetSettings().GetDictFormat(), iCheckpoint, sError ) )
				return false;
			m_pFinalizer->m_pInfixer->AddWord ( (const BYTE*)tWord.m_sKeyword, iLen, (int)iCheckpoint, bHasMorphology );
		}

		// next
		int iBin = tWord.m_iBlock;
		qWords.Pop();

		if ( !dBins[iBin].IsDone() )
		{
			if ( !DictReadEntry ( dBins[iBin], tEntry, dKeywords[iBin], sError ) )
				return false;
			if ( dBins[iBin].IsError() )
			{
				sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", iBin, dBins.GetLength() );
				return false;
			}

			tEntry.m_iBlock = iBin;
			qWords.Push ( tEntry );
		}
	}
	return SortedDictEnd ( pHeader, sError );
}

bool CSphDictKeywords::SortedDictEnd ( DictHeader_t * pHeader, CSphString& sError )
{
	assert ( m_pFinalizer );

	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		ResetCheckpoints();
		m_pFinalizer = nullptr;
		return false;
	}

	ARRAY_FOREACH ( i, m_dCheckpoints )
		if ( !CheckKeywordLength ( "checkpoint", (const int)strlen ( m_dCheckpoints[i].m_szWord ) ) )
		{
			sError = m_sError;
			ResetCheckpoints();
			m_pFinalizer = nullptr;
			return false;
		}

	// end of dictionary block
	PutBlockTerminator();

	// flush infix hash entries, if any
	if ( m_pFinalizer->m_pInfixer )
		m_pFinalizer->m_pInfixer->SaveEntries ( m_wrDict );

	// flush wordlist checkpoints (blocks)
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength64();

	ARRAY_FOREACH ( i, m_dCheckpoints )
		PutCheckpoint ( m_dCheckpoints[i] );
	ResetCheckpoints();

	// flush infix hash blocks
	if ( m_pFinalizer->m_pInfixer )
	{
		pHeader->m_iInfixBlocksOffset = m_pFinalizer->m_pInfixer->SaveEntryBlocks ( m_wrDict );
		pHeader->m_iInfixBlocksWordsSize = m_pFinalizer->m_pInfixer->GetBlocksWordsSize();
		if ( !GetSettings().IsKeywordsV2() && pHeader->m_iInfixBlocksOffset > UINT_MAX )
			sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at dictend save", pHeader->m_iInfixBlocksOffset );
	}

	// cleanup stuff we no more need
	m_pFinalizer = nullptr;

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	if ( GetSettings().IsKeywordsV2() )
	{
		m_wrDict.PutBlob ( g_sTagDictHeaderV2 );
		m_wrDict.ZipOffset ( pHeader->m_iDictCheckpoints );
		m_wrDict.ZipOffset ( pHeader->m_iDictCheckpointsOffset );
		m_wrDict.ZipInt ( pHeader->m_iInfixCodepointBytes );
		m_wrDict.ZipOffset ( pHeader->m_iInfixBlocksOffset );
		m_wrDict.ZipOffset ( pHeader->m_iInfixBlocksWordsSize );
	} else
	{
		m_wrDict.PutBlob ( g_sTagDictHeader );
		m_wrDict.ZipInt ( (DWORD)pHeader->m_iDictCheckpoints );
		m_wrDict.ZipOffset ( pHeader->m_iDictCheckpointsOffset );
		m_wrDict.ZipInt ( pHeader->m_iInfixCodepointBytes );
		m_wrDict.ZipInt ( (DWORD)pHeader->m_iInfixBlocksOffset );
	}

	// about it
	m_wrDict.CloseFile();
	if ( m_wrDict.IsError() )
		sError.SetSprintf ( "dictionary write error (out of space?)" );
	return !m_wrDict.IsError();
}

void CSphDictKeywords::DictFlush()
{
	if ( !m_dDictChunks.GetLength() )
		return;

	if ( !m_sError.IsEmpty() )
	{
		ResetDictChunks();
		return;
	}

	assert ( m_dDictChunks.GetLength() && m_dKeywordChunks.GetLength() );
	assert ( m_iSkiplistBlockSize > 0 );

	// sort em
	int iTotalWords = m_dDictChunks.GetLength() * DICT_CHUNK - m_iDictChunkFree;
	CSphVector<DictKeyword_t*> dWords ( iTotalWords );

	int iIdx = 0;
	ARRAY_FOREACH ( i, m_dDictChunks )
	{
		int iWords = DICT_CHUNK;
		if ( i == m_dDictChunks.GetLength() - 1 )
			iWords -= m_iDictChunkFree;

		DictKeyword_t* pWord = m_dDictChunks[i];
		for ( int j = 0; j < iWords; j++ )
			dWords[iIdx++] = pWord++;
	}

	dWords.Sort ( Lesser ( [] ( const CSphDictKeywords::DictKeyword_t* a, const CSphDictKeywords::DictKeyword_t* b ) noexcept {
		return strcmp ( a->m_sKeyword, b->m_sKeyword ) < 0;
	} ) );

	for ( const DictKeyword_t* pWord : dWords )
	{
		auto iLen = (int)strlen ( pWord->m_sKeyword );
		if ( !CheckKeywordLength ( "temp dictionary", iLen ) )
		{
			ResetDictChunks();
			return;
		}
	}

	// write em
	DictBlock_t& tBlock = m_dDictBlocks.Add();
	tBlock.m_iPos = m_wrTmpDict.GetPos();

	for ( const DictKeyword_t* pWord : dWords )
	{
		auto iLen = (int)strlen ( pWord->m_sKeyword );
		if ( GetSettings().IsKeywordsV2() )
			m_wrTmpDict.ZipInt ( iLen );
		else
			m_wrTmpDict.PutByte ( (BYTE)iLen );
		m_wrTmpDict.PutBytes ( pWord->m_sKeyword, iLen );
		m_wrTmpDict.ZipOffset ( pWord->m_uOff );
		m_wrTmpDict.ZipInt ( pWord->m_iDocs );
		m_wrTmpDict.ZipInt ( pWord->m_iHits );
		m_wrTmpDict.PutByte ( pWord->m_uHint );
		assert ( ( ( pWord->m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize ) == ( pWord->m_iSkiplistPos != 0 ) );
		if ( ( pWord->m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize )
			m_wrTmpDict.ZipOffset ( pWord->m_iSkiplistPos );
	}

	tBlock.m_iLen = (int)( m_wrTmpDict.GetPos() - tBlock.m_iPos );

	// clean up buffers
	ResetDictChunks();
}

void CSphDictKeywords::DictEntry ( const DictEntry_t& tEntry )
{
	assert ( tEntry.m_iDocs );
	assert ( tEntry.m_iHits );
	assert ( tEntry.m_iDoclistOffset );
	assert ( tEntry.m_iDoclistLength > 0 );
	assert ( m_iSkiplistBlockSize > 0 );

	if ( !IsSorted() )
		return DictEntryNonSorted ( tEntry );

	if ( !m_sError.IsEmpty() )
		return;

	auto iLen = (int)strlen ( (const char*)tEntry.m_szKeyword );
	if ( !CheckKeywordLength ( "dictionary", iLen ) )
		return;

	// store checkpoints as needed
	if ( ( m_pFinalizer->m_iWords % SPH_WORDLIST_CHECKPOINT ) == 0 )
	{
		// emit a checkpoint, unless we're at the very dict beginning
		if ( m_pFinalizer->m_iWords )
			PutBlockTerminator();

		auto* szClone = new BYTE[iLen + 1]; // OPTIMIZE? pool these?
		memcpy ( szClone, tEntry.m_szKeyword, iLen );
		szClone[iLen] = '\0';

		CSphWordlistCheckpoint& tCheckpoint = m_dCheckpoints.Add();
		tCheckpoint.m_szWord = (const char*)szClone;
		tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();

		if ( GetSettings().IsKeywordsV2() )
			m_pFinalizer->ResetLastKeywordV2();
		else
			m_pFinalizer->ResetLastKeywordKw();
	}
	++m_pFinalizer->m_iWords;

	// write final dict entry
	assert ( iLen );
	if ( GetSettings().IsKeywordsV2() )
		m_pFinalizer->PutDeltaV2 ( m_wrDict, (const BYTE*)tEntry.m_szKeyword, iLen );
	else
		m_pFinalizer->PutDeltaKw ( m_wrDict, (const BYTE*)tEntry.m_szKeyword, iLen );
	m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset );
	m_wrDict.ZipInt ( tEntry.m_iDocs );
	m_wrDict.ZipInt ( tEntry.m_iHits );
	auto uHint = sphDoclistHintPack ( tEntry.m_iDocs & HITLESS_DOC_MASK, tEntry.m_iDoclistLength );
	if ( uHint )
		m_wrDict.PutByte ( uHint );
	if ( ( tEntry.m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize )
		m_wrDict.ZipOffset ( tEntry.m_iSkiplistOffset );

	// build infixes
	if ( m_pFinalizer->m_pInfixer && m_sError.IsEmpty() )
	{
		int64_t iCheckpoint = m_dCheckpoints.GetLength64();
		if ( CheckKeywordsV2InfixCheckpoint ( GetSettings().GetDictFormat(), iCheckpoint, m_sError ) )
			m_pFinalizer->m_pInfixer->AddWord ( (const BYTE*)tEntry.m_szKeyword, iLen, (int)iCheckpoint, HasMorphology() );
	}
}

// non-sorted case - we push all entries into huge temporary file, and then finalize it in DictEnd()
void CSphDictKeywords::DictEntryNonSorted ( const DictEntry_t& tEntry )
{
	assert ( !IsSorted() );

	if ( !m_sError.IsEmpty() )
		return;

	DictKeyword_t* pWord = NULL;
	auto iLen = (int)strlen ( (const char*)tEntry.m_szKeyword ) + 1;
	if ( !CheckKeywordLength ( "dictionary", iLen-1 ) )
		return;

	while ( true )
	{
		// alloc dict entry
		if ( !m_iDictChunkFree )
		{
			if ( m_iDictLimit && ( m_iMemUse + (int)sizeof ( DictKeyword_t ) * DICT_CHUNK ) > m_iDictLimit )
				DictFlush();
			if ( !m_sError.IsEmpty() )
				return;

			m_pDictChunk = new DictKeyword_t[DICT_CHUNK];
			m_iDictChunkFree = DICT_CHUNK;
			m_dDictChunks.Add ( m_pDictChunk );
			m_iMemUse += sizeof ( DictKeyword_t ) * DICT_CHUNK;
		}

		// alloc keyword
		if ( m_iKeywordChunkFree < iLen )
		{
			if ( m_iDictLimit && ( m_iMemUse + KEYWORD_CHUNK ) > m_iDictLimit )
			{
				DictFlush();
				if ( !m_sError.IsEmpty() )
					return;
				continue; // because we just flushed pWord
			}

			m_pKeywordChunk = new BYTE[KEYWORD_CHUNK];
			m_iKeywordChunkFree = KEYWORD_CHUNK;
			m_dKeywordChunks.Add ( m_pKeywordChunk );
			m_iMemUse += KEYWORD_CHUNK;
		}
		// aw kay
		break;
	}

	pWord = m_pDictChunk++;
	--m_iDictChunkFree;
	pWord->m_sKeyword = (char*)m_pKeywordChunk;
	memcpy ( m_pKeywordChunk, tEntry.m_szKeyword, iLen );
	m_pKeywordChunk[iLen - 1] = '\0';
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	pWord->m_uOff = tEntry.m_iDoclistOffset;
	pWord->m_iDocs = tEntry.m_iDocs;
	pWord->m_iHits = tEntry.m_iHits;
	pWord->m_uHint = sphDoclistHintPack ( tEntry.m_iDocs & HITLESS_DOC_MASK, tEntry.m_iDoclistLength );
	pWord->m_iSkiplistPos = 0;
	if ( ( tEntry.m_iDocs & HITLESS_DOC_MASK ) > m_iSkiplistBlockSize )
		pWord->m_iSkiplistPos = tEntry.m_iSkiplistOffset;
}

SphWordID_t CSphDictKeywords::GetWordID ( BYTE* pWord )
{
	SphWordID_t uCRC = BASE::GetWordID ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int)strlen ( (const char*)pWord );
	return HitblockGetID ( (const char*)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDWithMarkers ( BYTE* pWord )
{
	SphWordID_t uCRC = BASE::GetWordIDWithMarkers ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int)strlen ( (const char*)pWord );
	return HitblockGetID ( (const char*)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDNonStemmed ( BYTE* pWord )
{
	SphWordID_t uCRC = BASE::GetWordIDNonStemmed ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int)strlen ( (const char*)pWord );
	return HitblockGetID ( (const char*)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uCRC = BASE::GetWordID ( pWord, iLen, bFilterStops );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	return HitblockGetID ( (const char*)pWord, iLen, uCRC ); // !COMMIT would break, we kind of strcmp inside; but must never get called?
}

/// do hit block patching magic
void CSphDictKeywords::HitblockPatch ( CSphWordHit* pHits, int iHits ) const
{
	if ( !pHits || iHits <= 0 )
		return;

	const CSphVector<HitblockException_t>& dExc = m_dExceptions; // shortcut
	CSphVector<CSphWordHit*> dChunk;

	// reorder hit chunks for exceptions (aka crc collisions)
	for ( int iFirst = 0; iFirst < dExc.GetLength() - 1; )
	{
		// find next span of collisions, iFirst inclusive, iMax exclusive ie. [iFirst,iMax)
		// (note that exceptions array is always sorted)
		SphWordID_t uFirstWordid = dExc[iFirst].m_pEntry->m_uWordid;
		assert ( dExc[iFirst].m_uCRC == uFirstWordid );

		int iMax = iFirst + 1;
		SphWordID_t uSpan = uFirstWordid + 1;
		while ( iMax < dExc.GetLength() && dExc[iMax].m_pEntry->m_uWordid == uSpan )
		{
			iMax++;
			uSpan++;
		}

		// check whether they are in proper order already
		bool bSorted = true;
		for ( int i = iFirst; i < iMax - 1 && bSorted; ++i )
			if ( FullIsLess ( dExc[i + 1], dExc[i] ) )
				bSorted = false;

		// order is ok; skip this span
		if ( bSorted )
		{
			iFirst = iMax;
			continue;
		}

		// we need to fix up these collision hits
		// convert them from arbitrary "wordid asc" to strict "crc asc, keyword asc" order
		// lets begin with looking up hit chunks for every wordid
		dChunk.Resize ( iMax - iFirst + 1 );

		// find the end
		dChunk.Last() = FindFirstGte ( pHits, iHits, uFirstWordid + iMax - iFirst );
		if ( !dChunk.Last() )
		{
			assert ( iMax == dExc.GetLength() && pHits[iHits - 1].m_uWordID == uFirstWordid + iMax - 1 - iFirst );
			dChunk.Last() = pHits + iHits;
		}

		// find the start
		dChunk[0] = FindFirstGte ( pHits, int ( dChunk.Last() - pHits ), uFirstWordid );
		assert ( dChunk[0] && dChunk[0]->m_uWordID == uFirstWordid );

		// find the chunk starts
		for ( int i = 1; i < dChunk.GetLength() - 1; i++ )
		{
			dChunk[i] = FindFirstGte ( dChunk[i - 1], int ( dChunk.Last() - dChunk[i - 1] ), uFirstWordid + i );
			assert ( dChunk[i] && dChunk[i]->m_uWordID == uFirstWordid + i );
		}

		CSphWordHit* pTemp;
		if ( iMax - iFirst == 2 )
		{
			// most frequent case, just two collisions
			// OPTIMIZE? allocate buffer for the smaller chunk, not just first chunk
			pTemp = new CSphWordHit[dChunk[1] - dChunk[0]];
			memcpy ( pTemp, dChunk[0], ( dChunk[1] - dChunk[0] ) * sizeof ( CSphWordHit ) );
			memmove ( dChunk[0], dChunk[1], ( dChunk[2] - dChunk[1] ) * sizeof ( CSphWordHit ) );
			memcpy ( dChunk[0] + ( dChunk[2] - dChunk[1] ), pTemp, ( dChunk[1] - dChunk[0] ) * sizeof ( CSphWordHit ) );
		} else
		{
			// generic case, more than two
			CSphVector<int> dReorder ( iMax - iFirst );
			dReorder.FillSeq();
			dReorder.Sort ( Lesser ( [pBase = &dExc[iFirst]] ( int a, int b ) noexcept { return FullIsLess ( pBase[a], pBase[b] ); } ) );

			// OPTIMIZE? could skip heading and trailing blocks that are already in position
			pTemp = new CSphWordHit[dChunk.Last() - dChunk[0]];
			CSphWordHit* pOut = pTemp;

			ARRAY_FOREACH ( i, dReorder )
			{
				int iChunk = dReorder[i];
				int iChunkHits = int ( dChunk[iChunk + 1] - dChunk[iChunk] );
				memcpy ( pOut, dChunk[iChunk], iChunkHits * sizeof ( CSphWordHit ) );
				pOut += iChunkHits;
			}

			assert ( ( pOut - pTemp ) == ( dChunk.Last() - dChunk[0] ) );
			memcpy ( dChunk[0], pTemp, ( dChunk.Last() - dChunk[0] ) * sizeof ( CSphWordHit ) );
		}

		// patching done
		SafeDeleteArray ( pTemp );
		iFirst = iMax;
	}
}

const char* CSphDictKeywords::HitblockGetKeyword ( SphWordID_t uWordID )
{
	const DWORD uHash = (DWORD)( uWordID % SLOTS );

	HitblockKeyword_t* pEntry = m_dHash[uHash];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid != uWordID )
		{
			// crc mismatch, try next entry
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		return pEntry->m_pKeyword;
	}

	assert ( m_dExceptions.GetLength() );
	ARRAY_FOREACH ( i, m_dExceptions )
		if ( m_dExceptions[i].m_pEntry->m_uWordid == uWordID )
			return m_dExceptions[i].m_pEntry->m_pKeyword;

	sphWarning ( "hash missing value in operator [] (wordid=" INT64_FMT ", hash=%u)", (int64_t)uWordID, uHash );
	assert ( 0 && "hash missing value in operator []" );
	return "\31oops";
}

///////////////////////////////////////////////////////////////////////

DictRefPtr_c sphCreateDictionaryKeywords ( const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, int iSkiplistBlockSize, FilenameBuilder_i* pFilenameBuilder, CSphString& sError )
{
	DictRefPtr_c pDict { new CSphDictKeywords() };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, szIndex, bStripFile, pFilenameBuilder, sError );
	// might be empty due to wrong morphology setup
	if ( pDict )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict;
}
