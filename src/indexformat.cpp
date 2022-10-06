//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "indexformat.h"

// let uDocs be DWORD here to prevent int overflow in case of hitless word (highest bit is 1)
int DoclistHintUnpack ( DWORD uDocs, BYTE uHint )
{
	if ( uDocs<(DWORD)DOCLIST_HINT_THRESH )
		return (int)Min ( 8*(int64_t)uDocs, INT_MAX );
	else
		return (int)Min ( 4*(int64_t)uDocs+( int64_t(uDocs)*uHint/64 ), INT_MAX );
}

//////////////////////////////////////////////////////////////////////////

DiskIndexQwordTraits_c::DiskIndexQwordTraits_c ( bool bUseMini, bool bExcluded )
{
	m_bExcluded = bExcluded;

	if ( bUseMini )
	{
		m_pDocsBuf = m_dDoclistBuf;
		m_pHitsBuf = m_dHitlistBuf;
	}
}


void DiskIndexQwordTraits_c::SetDocReader ( DataReaderFactory_c * pReader )
{
	if ( !pReader )
		return;

	m_rdDoclist = pReader->MakeReader ( m_pDocsBuf, MINIBUFFER_LEN );
}


void DiskIndexQwordTraits_c::SetHitReader ( DataReaderFactory_c * pReader )
{
	if ( !pReader )
		return;

	m_rdHitlist = pReader->MakeReader ( m_pHitsBuf, MINIBUFFER_LEN );
}


void DiskIndexQwordTraits_c::ResetDecoderState ()
{
	ISphQword::Reset();
	m_uHitPosition = 0;
	m_uInlinedHit = 0;
	m_uHitState = 0;
	m_tDoc.m_tRowID = INVALID_ROWID;
	m_iHitPos = EMPTY_HIT;
}

//////////////////////////////////////////////////////////////////////////

class CheckpointReader_c
{
public:
	const BYTE * ReadEntry ( const BYTE * pBuf, CSphWordlistCheckpoint & tCP ) const
	{
		tCP.m_uWordID = (SphWordID_t)sphUnalignedRead ( *(SphOffset_t *)pBuf );
		pBuf += sizeof(SphOffset_t);

		tCP.m_iWordlistOffset = sphUnalignedRead ( *(SphOffset_t *)pBuf );
		pBuf += sizeof(SphOffset_t);

		return pBuf;
	}

	int GetStride() const { return m_iSrcStride; }

private:
	int m_iSrcStride = 2*sizeof(SphOffset_t);
};



struct MappedCheckpoint_fn : public ISphNoncopyable
{
	const CSphWordlistCheckpoint *	m_pDstStart;
	const BYTE *					m_pSrcStart;
	const CheckpointReader_c *		m_pReader;

	MappedCheckpoint_fn ( const CSphWordlistCheckpoint * pDstStart, const BYTE * pSrcStart, const CheckpointReader_c * pReader )
		: m_pDstStart ( pDstStart )
		, m_pSrcStart ( pSrcStart )
		, m_pReader ( pReader )
	{}

	CSphWordlistCheckpoint operator() ( const CSphWordlistCheckpoint * pCP ) const
	{
		assert ( m_pDstStart<=pCP );
		const BYTE * pCur = ( pCP - m_pDstStart ) * m_pReader->GetStride() + m_pSrcStart;
		CSphWordlistCheckpoint tEntry;
		m_pReader->ReadEntry ( pCur, tEntry );
		return tEntry;
	}
};

//////////////////////////////////////////////////////////////////////////

struct DiskExpandedEntry_t
{
	int		m_iNameOff;
	int		m_iDocs;
	int		m_iHits;
};


struct DiskExpandedPayload_t
{
	int			m_iDocs;
	int			m_iHits;
	uint64_t	m_uDoclistOff;
	int			m_iDoclistHint;
};


struct Slice64_t
{
	uint64_t	m_uOff;
	int			m_iLen;
};


struct DiskSubstringPayload_t : public ISphSubstringPayload
{
	explicit DiskSubstringPayload_t ( int iDoclists )
		: m_dDoclist ( iDoclists )
	{}

	CSphFixedVector<Slice64_t>	m_dDoclist;
};


//////////////////////////////////////////////////////////////////////////

struct DictEntryDiskPayload_t
{
	DictEntryDiskPayload_t ( bool bPayload, ESphHitless eHitless )
	{
		m_bPayload = bPayload;
		m_eHitless = eHitless;
		if ( bPayload )
			m_dWordPayload.Reserve ( 1000 );

		m_dWordExpand.Reserve ( 1000 );
		m_dWordBuf.Reserve ( 8096 );
	}

	void Add ( const DictEntry_t & tWord, int iWordLen )
	{
		if ( !m_bPayload || !sphIsExpandedPayload ( tWord.m_iDocs, tWord.m_iHits ) ||
			m_eHitless==SPH_HITLESS_ALL || ( m_eHitless==SPH_HITLESS_SOME && ( tWord.m_iDocs & HITLESS_DOC_FLAG )!=0 ) ) // FIXME!!! do we need hitless=some as payloads?
		{
			DiskExpandedEntry_t & tExpand = m_dWordExpand.Add();

			int iOff = m_dWordBuf.GetLength();
			tExpand.m_iNameOff = iOff;
			tExpand.m_iDocs = tWord.m_iDocs;
			tExpand.m_iHits = tWord.m_iHits;
			m_dWordBuf.Resize ( iOff + iWordLen + 1 );
			memcpy ( m_dWordBuf.Begin() + iOff + 1, tWord.m_sKeyword, iWordLen );
			m_dWordBuf[iOff] = (BYTE)iWordLen;

		} else
		{
			DiskExpandedPayload_t & tExpand = m_dWordPayload.Add();
			tExpand.m_iDocs = tWord.m_iDocs;
			tExpand.m_iHits = tWord.m_iHits;
			tExpand.m_uDoclistOff = tWord.m_iDoclistOffset;
			tExpand.m_iDoclistHint = tWord.m_iDoclistHint;
		}
	}

	void Convert ( ISphWordlist::Args_t & tArgs )
	{
		if ( !m_dWordExpand.GetLength() && !m_dWordPayload.GetLength() )
			return;

		int iTotalDocs = 0;
		int iTotalHits = 0;
		if ( m_dWordExpand.GetLength() )
		{
			LimitExpanded ( tArgs.m_iExpansionLimit, m_dWordExpand );

			const BYTE * sBase = m_dWordBuf.Begin();
			ARRAY_FOREACH ( i, m_dWordExpand )
			{
				const DiskExpandedEntry_t & tCur = m_dWordExpand[i];
				int iDocs = tCur.m_iDocs;

				if ( m_eHitless==SPH_HITLESS_SOME )
					iDocs = ( tCur.m_iDocs & HITLESS_DOC_MASK );

				tArgs.AddExpanded ( sBase + tCur.m_iNameOff + 1, sBase[tCur.m_iNameOff], iDocs, tCur.m_iHits );

				iTotalDocs += iDocs;
				iTotalHits += tCur.m_iHits;
			}
		}

		if ( m_dWordPayload.GetLength() )
		{
			LimitExpanded ( tArgs.m_iExpansionLimit, m_dWordPayload );

			DiskSubstringPayload_t * pPayload = new DiskSubstringPayload_t ( m_dWordPayload.GetLength() );
			// sorting by ascending doc-list offset gives some (15%) speed-up too
			sphSort ( m_dWordPayload.Begin(), m_dWordPayload.GetLength(), bind ( &DiskExpandedPayload_t::m_uDoclistOff ) );

			ARRAY_FOREACH ( i, m_dWordPayload )
			{
				const DiskExpandedPayload_t & tCur = m_dWordPayload[i];
				assert ( m_eHitless==SPH_HITLESS_NONE || ( m_eHitless==SPH_HITLESS_SOME && ( tCur.m_iDocs & HITLESS_DOC_FLAG )==0 ) );

				iTotalDocs += tCur.m_iDocs;
				iTotalHits += tCur.m_iHits;
				pPayload->m_dDoclist[i].m_uOff = tCur.m_uDoclistOff;
				pPayload->m_dDoclist[i].m_iLen = tCur.m_iDoclistHint;
			}

			pPayload->m_iTotalDocs = iTotalDocs;
			pPayload->m_iTotalHits = iTotalHits;
			tArgs.m_pPayload = pPayload;
		}
		tArgs.m_iTotalDocs = iTotalDocs;
		tArgs.m_iTotalHits = iTotalHits;
	}

	// sort expansions by frequency desc
	// clip the less frequent ones if needed, as they are likely misspellings
	template < typename T >
	void LimitExpanded ( int iExpansionLimit, CSphVector<T> & dVec ) const
	{
		if ( !iExpansionLimit || dVec.GetLength()<=iExpansionLimit )
			return;

		sphSort ( dVec.Begin(), dVec.GetLength(), ExpandedOrderDesc_T<T>() );
		dVec.Resize ( iExpansionLimit );
	}

	bool								m_bPayload;
	ESphHitless							m_eHitless;
	CSphVector<DiskExpandedEntry_t>		m_dWordExpand;
	CSphVector<DiskExpandedPayload_t>	m_dWordPayload;
	CSphVector<BYTE>					m_dWordBuf;
};

//////////////////////////////////////////////////////////////////////////

CWordlist::~CWordlist ()
{
	Reset();
}


void CWordlist::Reset ()
{
	m_tBuf.Reset ();
	m_dCheckpoints.Reset ( 0 );
	m_pWords.Reset ( 0 );
	SafeDeleteArray ( m_pInfixBlocksWords );
	SafeDelete ( m_pCpReader );
}


bool CWordlist::Preread ( const CSphString & sName, bool bWordDict, int iSkiplistBlockSize, CSphString & sError )
{
	assert ( m_iDictCheckpointsOffset>0 );

	m_bWordDict = bWordDict;
	m_iWordsEnd = m_iDictCheckpointsOffset; // set wordlist end
	m_iSkiplistBlockSize = iSkiplistBlockSize;

	////////////////////////////
	// preload word checkpoints
	////////////////////////////

	////////////////////////////
	// fast path for CRC checkpoints - just maps data and use inplace CP reader
	if ( !bWordDict )
	{
		if ( !m_tBuf.Setup ( sName, sError ) )
			return false;

		m_pCpReader = new CheckpointReader_c;
		return true;
	}

	////////////////////////////
	// regular path that loads checkpoints data

	CSphAutoreader tReader;
	if ( !tReader.Open ( sName, sError ) )
		return false;

	int64_t iFileSize = tReader.GetFilesize();

	int iCheckpointOnlySize = (int)(iFileSize-m_iDictCheckpointsOffset);
	if ( m_iInfixCodepointBytes && m_iInfixBlocksOffset )
		iCheckpointOnlySize = (int)(m_iInfixBlocksOffset - strlen ( g_sTagInfixBlocks ) - m_iDictCheckpointsOffset);

	if ( iFileSize-m_iDictCheckpointsOffset>=UINT_MAX )
	{
		sError.SetSprintf ( "dictionary meta overflow: meta size=" INT64_FMT ", total size=" INT64_FMT ", meta offset=" INT64_FMT,
			iFileSize-m_iDictCheckpointsOffset, iFileSize, (int64_t)m_iDictCheckpointsOffset );
		return false;
	}

	tReader.SeekTo ( m_iDictCheckpointsOffset, iCheckpointOnlySize );

	assert ( m_bWordDict );
	int iArenaSize = iCheckpointOnlySize
		- (sizeof(DWORD)+sizeof(SphOffset_t))*m_dCheckpoints.GetLength()
		+ sizeof(BYTE)*m_dCheckpoints.GetLength();
	assert ( iArenaSize>=0 );
	m_pWords.Reset ( iArenaSize );

	BYTE * pWord = m_pWords.Begin();
	for ( auto & dCheckpoint : m_dCheckpoints )
	{
		dCheckpoint.m_sWord = (char *)pWord;

		const int iLen = tReader.GetDword();
		assert ( iLen>0 );
		assert ( iLen + 1 + ( pWord - m_pWords.Begin() )<=iArenaSize );
		tReader.GetBytes ( pWord, iLen );
		pWord[iLen] = '\0';
		pWord += iLen+1;

		dCheckpoint.m_iWordlistOffset = tReader.GetOffset();
	}

	////////////////////////
	// preload infix blocks
	////////////////////////

	if ( m_iInfixCodepointBytes && m_iInfixBlocksOffset )
	{
		// reading to vector as old version doesn't store total infix words length
		CSphTightVector<BYTE> dInfixWords;
		dInfixWords.Reserve ( (int)m_iInfixBlocksWordsSize );

		tReader.SeekTo ( m_iInfixBlocksOffset, (int)(iFileSize-m_iInfixBlocksOffset) );
		m_dInfixBlocks.Resize ( tReader.UnzipInt() );
		for ( auto & dInfixBlock : m_dInfixBlocks )
		{
			int iBytes = tReader.UnzipInt();

			int iOff = dInfixWords.GetLength();
			dInfixBlock.m_iInfixOffset = (DWORD) iOff; /// FIXME! name convention of m_iInfixOffset
			dInfixWords.Resize ( iOff+iBytes+1 );

			tReader.GetBytes ( dInfixWords.Begin()+iOff, iBytes );
			dInfixWords[iOff+iBytes] = '\0';

			dInfixBlock.m_iOffset = tReader.UnzipInt();
		}

		// fix-up offset to pointer
		m_pInfixBlocksWords = dInfixWords.LeakData();
		ARRAY_FOREACH ( i, m_dInfixBlocks )
			m_dInfixBlocks[i].m_sInfix = (const char *)m_pInfixBlocksWords + m_dInfixBlocks[i].m_iInfixOffset;

		// FIXME!!! store and load that explicitly
		if ( m_dInfixBlocks.GetLength() )
			m_iWordsEnd = m_dInfixBlocks.Begin()->m_iOffset - strlen ( g_sTagInfixEntries );
		else
			m_iWordsEnd -= strlen ( g_sTagInfixEntries );
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	tReader.Close();

	// mapping up only wordlist without meta (checkpoints, infixes, etc)
	if ( !m_tBuf.Setup ( sName, sError ) )
		return false;

	return true;
}


void CWordlist::DebugPopulateCheckpoints()
{
	if ( !m_pCpReader )
		return;

	const BYTE * pCur = m_tBuf.GetReadPtr() + m_iDictCheckpointsOffset;
	ARRAY_FOREACH ( i, m_dCheckpoints )
		pCur = m_pCpReader->ReadEntry ( pCur, m_dCheckpoints[i] );

	SafeDelete(m_pCpReader);
}


const CSphWordlistCheckpoint * CWordlist::FindCheckpointCrc ( SphWordID_t iWordID ) const
{
	if ( m_pCpReader ) // FIXME!!! fall to regular checkpoints after data got read
	{
		MappedCheckpoint_fn tPred ( m_dCheckpoints.Begin(), m_tBuf.GetReadPtr() + m_iDictCheckpointsOffset, m_pCpReader );
		return sphSearchCheckpointCrc( iWordID, m_dCheckpoints, std::move(tPred));
	}

	return sphSearchCheckpointCrc ( iWordID, m_dCheckpoints );
}

const CSphWordlistCheckpoint * CWordlist::FindCheckpointWrd ( const char* sWord, int iWordLen, bool bStarMode ) const
{
	if ( m_pCpReader ) // FIXME!!! fall to regular checkpoints after data got read
	{
		MappedCheckpoint_fn tPred ( m_dCheckpoints.Begin(), m_tBuf.GetReadPtr() + m_iDictCheckpointsOffset, m_pCpReader );
		return sphSearchCheckpointWrd ( sWord, iWordLen, bStarMode, m_dCheckpoints, std::move ( tPred ) );
	}

	return sphSearchCheckpointWrd ( sWord, iWordLen, bStarMode, m_dCheckpoints );
}


bool CWordlist::GetWord ( const BYTE * pBuf, SphWordID_t iWordID, DictEntry_t & tWord ) const
{
	SphWordID_t iLastID = 0;
	SphOffset_t uLastOff = 0;

	while (true)
	{
		// unpack next word ID
		const SphWordID_t iDeltaWord = UnzipWordidBE ( pBuf ); // FIXME! slow with 32bit wordids

		if ( iDeltaWord==0 ) // wordlist chunk is over
			return false;

		iLastID += iDeltaWord;

		// list is sorted, so if there was no match, there's no such word
		if ( iLastID>iWordID )
			return false;

		// unpack next offset
		const SphOffset_t iDeltaOffset = UnzipOffsetBE ( pBuf );
		uLastOff += iDeltaOffset;

		// unpack doc/hit count
		const int iDocs = UnzipIntBE ( pBuf );
		const int iHits = UnzipIntBE ( pBuf );
		SphOffset_t iSkiplistPos = 0;
		if ( iDocs > m_iSkiplistBlockSize )
			iSkiplistPos = UnzipOffsetBE ( pBuf );

		assert ( iDeltaOffset );
		assert ( iDocs );
		assert ( iHits );

		// it matches?!
		if ( iLastID==iWordID )
		{
			UnzipWordidBE ( pBuf ); // might be 0 at checkpoint
			const SphOffset_t iDoclistLen = UnzipOffsetBE ( pBuf );

			tWord.m_iDoclistOffset = uLastOff;
			tWord.m_iDocs = iDocs;
			tWord.m_iHits = iHits;
			tWord.m_iDoclistHint = (int)iDoclistLen;
			tWord.m_iSkiplistOffset = iSkiplistPos;
			return true;
		}
	}
}


const BYTE * CWordlist::AcquireDict ( const CSphWordlistCheckpoint * pCheckpoint ) const
{
	assert ( pCheckpoint );
	assert ( m_dCheckpoints.GetLength() );
	assert ( pCheckpoint>=m_dCheckpoints.Begin() && pCheckpoint<=&m_dCheckpoints.Last() );

	SphOffset_t iOff = pCheckpoint->m_iWordlistOffset;
	if ( m_pCpReader )
	{
		MappedCheckpoint_fn tPred ( m_dCheckpoints.Begin(), m_tBuf.GetReadPtr() + m_iDictCheckpointsOffset, m_pCpReader );
		iOff = tPred ( pCheckpoint ).m_iWordlistOffset;
	}

	assert ( !m_tBuf.IsEmpty() );
	assert ( iOff>0 && iOff<(int64_t)m_tBuf.GetLengthBytes() );

	return m_tBuf.GetReadPtr()+iOff;
}


void CWordlist::GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	assert ( sSubstring && *sSubstring && iSubLen>0 );

	// empty index?
	if ( !m_dCheckpoints.GetLength() )
		return;

	DictEntryDiskPayload_t tDict2Payload ( tArgs.m_bPayload, tArgs.m_eHitless );

	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

	// assume dict=crc never has word with wordid=0, however just don't consider it and explicitly set nullptr.
	const CSphWordlistCheckpoint * pCheckpoint = m_bWordDict ? FindCheckpointWrd ( sSubstring, iSubLen, true ) : nullptr;
	const int iSkipMagic = ( BYTE(*sSubstring)<0x20 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker
	while ( pCheckpoint )
	{
		// decode wordlist chunk
		KeywordsBlockReader_c tDictReader ( AcquireDict ( pCheckpoint ), m_iSkiplistBlockSize );
		while ( tDictReader.UnpackWord() )
		{
			// block is sorted
			// so once keywords are greater than the prefix, no more matches
			int iCmp = sphDictCmp ( sSubstring, iSubLen, (const char *)tDictReader.m_sKeyword, tDictReader.GetWordLen() );
			if ( iCmp<0 )
				break;

			if ( sphInterrupted() )
				break;

			// does it match the prefix *and* the entire wildcard?
			if ( iCmp==0 && sphWildcardMatch ( (const char *)tDictReader.m_sKeyword + iSkipMagic, sWildcard, pWildcard ) )
				tDict2Payload.Add ( tDictReader, tDictReader.GetWordLen() );
		}

		if ( sphInterrupted () )
			break;

		pCheckpoint++;
		if ( pCheckpoint > &m_dCheckpoints.Last() )
			break;

		if ( sphDictCmp ( sSubstring, iSubLen, pCheckpoint->m_sWord, (int) strlen ( pCheckpoint->m_sWord ) )<0 )
			break;
	}

	tDict2Payload.Convert ( tArgs );
}


void CWordlist::GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const
{
	// dict must be of keywords type, and fully cached
	// mmap()ed in the worst case, should we ever banish it to disk again
	if ( m_tBuf.IsEmpty() || !m_dCheckpoints.GetLength() )
		return;

	assert ( !m_pCpReader );

	// extract key1, upto 6 chars from infix start
	int iBytes1 = sphGetInfixLength ( sSubstring, iSubLen, m_iInfixCodepointBytes );

	// lookup key1
	// OPTIMIZE? maybe lookup key2 and reduce checkpoint set size, if possible?
	CSphVector<DWORD> dPoints;
	if ( !sphLookupInfixCheckpoints ( sSubstring, iBytes1, m_tBuf.GetReadPtr(), m_dInfixBlocks, m_iInfixCodepointBytes, dPoints ) )
		return;

	DictEntryDiskPayload_t tDict2Payload ( tArgs.m_bPayload, tArgs.m_eHitless );
	const int iSkipMagic = ( tArgs.m_bHasExactForms ? 1 : 0 ); // whether to skip heading magic chars in the prefix, like NONSTEMMED maker

	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = ( sphIsUTF8 ( sWildcard ) && sphUTF8ToWideChar ( sWildcard, dWildcard, SPH_MAX_WORD_LEN ) ) ? dWildcard : NULL;

	// walk those checkpoints, check all their words
	ARRAY_FOREACH ( i, dPoints )
	{
		// OPTIMIZE? add a quicker path than a generic wildcard for "*infix*" case?
		KeywordsBlockReader_c tDictReader ( m_tBuf.GetReadPtr() + m_dCheckpoints[dPoints[i]-1].m_iWordlistOffset, m_iSkiplistBlockSize );
		while ( tDictReader.UnpackWord() )
		{
			if ( sphInterrupted () )
				break;

			// stemmed terms should not match suffixes
			if ( tArgs.m_bHasExactForms && *tDictReader.m_sKeyword!=MAGIC_WORD_HEAD_NONSTEMMED )
				continue;

			if ( sphWildcardMatch ( (const char *)tDictReader.m_sKeyword+iSkipMagic, sWildcard, pWildcard ) )
				tDict2Payload.Add ( tDictReader, tDictReader.GetWordLen() );
		}

		if ( sphInterrupted () )
			break;
	}

	tDict2Payload.Convert ( tArgs );
}


void CWordlist::SuffixGetChekpoints ( const SuggestResult_t & , const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const
{
	sphLookupInfixCheckpoints ( sSuffix, iLen, m_tBuf.GetReadPtr(), m_dInfixBlocks, m_iInfixCodepointBytes, dCheckpoints );
}


void CWordlist::SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const
{
	assert ( tRes.m_pWordReader );
	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	pReader->Reset ( m_tBuf.GetReadPtr() + m_dCheckpoints[iCP-1].m_iWordlistOffset );
}


bool CWordlist::ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const
{
	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	if ( !pReader->UnpackWord() )
		return false;

	tWord.m_sWord = pReader->GetWord();
	tWord.m_iLen = pReader->GetWordLen();
	tWord.m_iDocs = pReader->m_iDocs;
	return true;
}

//////////////////////////////////////////////////////////////////////////

KeywordsBlockReader_c::KeywordsBlockReader_c ( const BYTE * pBuf, int iSkiplistBlockSize )
	: m_iSkiplistBlockSize ( iSkiplistBlockSize )
{
	Reset ( pBuf );
}


void KeywordsBlockReader_c::Reset ( const BYTE * pBuf )
{
	m_pBuf = pBuf;
	m_sWord[0] = '\0';
	m_iLen = 0;
	m_sKeyword = m_sWord;
}


bool KeywordsBlockReader_c::UnpackWord()
{
	if ( !m_pBuf )
		return false;

	assert ( m_iSkiplistBlockSize>0 );

	// unpack next word
	// must be in sync with DictEnd()!
	BYTE uPack = *m_pBuf++;
	if ( !uPack )
	{
		// ok, this block is over
		m_pBuf = NULL;
		m_iLen = 0;
		return false;
	}

	int iMatch, iDelta;
	if ( uPack & 0x80 )
	{
		iDelta = ( ( uPack>>4 ) & 7 ) + 1;
		iMatch = uPack & 15;
	} else
	{
		iDelta = uPack & 127;
		iMatch = *m_pBuf++;
	}

	assert ( iMatch+iDelta<(int)sizeof(m_sWord)-1 );
	assert ( iMatch<=(int)strlen ( (char *)m_sWord ) );

	memcpy ( m_sWord + iMatch, m_pBuf, iDelta );
	m_pBuf += iDelta;

	m_iLen = iMatch + iDelta;
	m_sWord[m_iLen] = '\0';

	m_iDoclistOffset = UnzipOffsetBE ( m_pBuf );
	m_iDocs = UnzipIntBE ( m_pBuf );
	m_iHits = UnzipIntBE ( m_pBuf );
	m_uHint = ( m_iDocs>=DOCLIST_HINT_THRESH ) ? *m_pBuf++ : 0;
	m_iDoclistHint = DoclistHintUnpack ( m_iDocs, m_uHint );
	if ( m_iDocs > m_iSkiplistBlockSize )
		m_iSkiplistOffset = UnzipOffsetBE ( m_pBuf );
	else
		m_iSkiplistOffset = 0;

	assert ( m_iLen>0 );
	return true;
}
