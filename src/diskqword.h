//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "indexformat.h"
#include "sphinxint.h"
#include "skip_cache.h"

// on-disk query word: the term reader over doclists and hitlists of a plain index or an RT disk chunk,
// its setup helper, and the reader policies (buffered file access vs raw pointers into the memory map)

/// prefetch gate: a residency probe and readahead request are issued only when the counter is zero or a power
/// of two, and a cache miss resets it. A cold query therefore prefetches every term while a warm one stops
/// probing after a few terms
struct PrefetchGate_t
{
	DWORD	m_uCounter = 0;

	bool	ShouldProbe()	{ DWORD u = m_uCounter++; return ( u & ( u-1 ) )==0; }
	void	OnMiss()		{ m_uCounter = 0; }
};

/// doclists up to this size are prefetched whole, longer ones only at the start; kernel readahead takes over
/// once sequential reads begin
static const int PREFETCH_WHOLE_DOCLIST_BYTES = 256*1024;
static const int PREFETCH_DOCLIST_HEAD_BYTES = 64*1024;


/// a doclist slice of an expanded term
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


/// everything required to setup search term
class DiskIndexQwordSetup_c final : public ISphQwordSetup
{
public:
						DiskIndexQwordSetup_c ( DataReaderFactoryPtr_c pDoclist, DataReaderFactoryPtr_c pHitlist, const BYTE * pSkips, int iSkiplistBlockSize, bool bSetupReaders, RowID_t iRowsCount );

	ISphQword *			QwordSpawn ( const XQKeyword_t & tWord ) const final;
	QwordType_e			GetQwordType() const final;
	bool				QwordSetup ( ISphQword * ) const final;
	bool				Setup ( ISphQword * ) const;
	ISphQword *			ScanSpawn ( int iAtomPos ) const final;

	/// bring the start of a term's doclist into memory before the first decode, subject to the gate
	void				PrefetchDoclist ( SphOffset_t iOff, int iHint ) const;
	/// the same for every doclist slice of an expanded term, in one request
	void				PrefetchPayload ( const DiskSubstringPayload_t & tPayload ) const;

private:
	mutable PrefetchGate_t		m_tPrefetchGate;	// one per query and chunk, since the setup object is
	DataReaderFactoryPtr_c		m_pDoclist;
	DataReaderFactoryPtr_c		m_pHitlist;
	const BYTE *				m_pSkips;
	int							m_iSkiplistBlockSize = 0;
	bool						m_bSetupReaders = false;
	RowID_t						m_iRowsCount = INVALID_ROWID;

private:
	bool SetupWithWrd ( const DiskIndexQwordTraits_c& tWord, DictEntry_t& tRes ) const;
	bool SetupWithCrc ( const DiskIndexQwordTraits_c& tWord, DictEntry_t& tRes ) const;
};

/// reader policies for DiskIndexQword_c: one decoding interface over the virtual block reader (buffered file access,
/// one virtual call per value) or over raw pointers into the memory map (mmap access, the optimized path)
struct BlockReader_t
{
	static constexpr bool IS_RAW = false;
	FileBlockReader_i *	m_pReader = nullptr;	// owned by DiskIndexQwordTraits_c

	void			Attach ( FileBlockReader_i * pReader )			{ m_pReader = pReader; }
	void			SeekTo ( SphOffset_t iPos, int iHint )			{ m_pReader->SeekTo ( iPos, iHint ); }
	SphOffset_t		GetPos() const									{ return m_pReader->GetPos(); }
	RowID_t			UnzipRowid()									{ return m_pReader->UnzipRowid(); }
	DWORD			UnzipInt()										{ return m_pReader->UnzipInt(); }
	SphOffset_t		UnzipOffset()									{ return m_pReader->UnzipOffset(); }
};


struct RawReader_t
{
	static constexpr bool IS_RAW = true;
	const BYTE *	m_pBase = nullptr;
	const BYTE *	m_pCur = nullptr;
	const BYTE *	m_pEnd = nullptr;

	void Attach ( FileBlockReader_i * pReader )
	{
		SphOffset_t iSize = 0;
		m_pBase = pReader ? pReader->GetMapBase ( iSize ) : nullptr;
		m_pEnd = m_pBase + iSize;
		m_pCur = m_pBase + ( pReader ? pReader->GetPos() : 0 );
	}

	void			SeekTo ( SphOffset_t iPos, int )				{ m_pCur = m_pBase + iPos; }
	SphOffset_t		GetPos() const									{ return m_pCur - m_pBase; }
	FORCE_INLINE bool	Has ( int iBytes ) const					{ return m_pEnd-m_pCur>=iBytes; }
	FORCE_INLINE BYTE	GetByteChecked()							{ return m_pCur<m_pEnd ? *m_pCur++ : 0; }

	// bounds-checked decoding, used for hitlist entries and for the last bytes of the map;
	// bulk doclist decoding goes through DocCursor_t instead. A 32-bit varint takes at most 5 bytes
	FORCE_INLINE DWORD UnzipInt()
	{
		if ( Has ( 5 ) )
			return UnzipIntBE ( m_pCur );
		return UnzipValueBE<DWORD> ( [this]() { return GetByteChecked(); } );
	}

	FORCE_INLINE RowID_t		UnzipRowid()	{ return UnzipInt(); }
	FORCE_INLINE SphOffset_t	UnzipOffset()	{ return UnzipValueBE<SphOffset_t> ( [this]() { return GetByteChecked(); } ); }
};


/// query word from the searcher's point of view
template < bool INLINE_HITS, bool DISABLE_HITLIST_SEEK, typename READER >
class DiskIndexQword_c : public DiskIndexQwordTraits_c
{
public:
	static constexpr QwordType_e QWORD_TYPE = DISABLE_HITLIST_SEEK ? QwordType_e::GENERIC
		: READER::IS_RAW
			? ( INLINE_HITS ? QwordType_e::DISK_INLINE_RAW : QwordType_e::DISK_PLAIN_RAW )
			: ( INLINE_HITS ? QwordType_e::DISK_INLINE_BLOCK : QwordType_e::DISK_PLAIN_BLOCK );

	DiskIndexQword_c ( bool bUseMinibuffer, bool bExcluded, int64_t iIndexId )
		: DiskIndexQwordTraits_c ( bUseMinibuffer, bExcluded )
		, m_iIndexId ( iIndexId )
	{}

	~DiskIndexQword_c()
	{
		if ( m_bSkipFromCache )
		{
			SkipCache::Release ( { m_iIndexId, m_uWordID } );
			m_pSkipData = nullptr;
			m_bSkipFromCache = false;
		}
	}

	void Reset () final
	{
		if ( m_rdDoclist )
			m_rdDoclist->Reset ();
		if ( m_rdHitlist )
			m_rdHitlist->Reset ();

		AttachReaders();
		ResetDecoderState();
	}

	bool WantHitPrefetch() final
	{
		return m_pHitFactory && m_pHitFactory->IsMapped() && m_tHitGate.ShouldProbe();
	}

	void PrefetchHits ( const SphOffset_t * pOffsets, const DWORD * pHits, int iCount ) final
	{
		if ( !m_pHitFactory || iCount<=0 )
			return;

		// exactly the hitlists about to be seeked to: inlined single hits live in the doclist and need nothing;
		// a document's range is sized from its hit count (a couple of bytes per hit), bounded, or one page when unknown
		CSphFixedVector<FileRange_t> dRanges ( iCount );
		int iRanges = 0;
		for ( int i = 0; i < iCount; i++ )
		{
			if ( pOffsets[i]>>63 )
				continue; // inline hit: no hitlist bytes to prefetch.

			SphOffset_t iLen = pHits ? Min ( Max ( (SphOffset_t)pHits[i]*2 + 16, (SphOffset_t)256 ), (SphOffset_t)PREFETCH_DOCLIST_HEAD_BYTES ) : (SphOffset_t)4096;
			dRanges[iRanges++] = { pOffsets[i], iLen };
		}

		if ( iRanges && m_pHitFactory->Prefetch ( dRanges.Begin(), iRanges ) )
			m_tHitGate.OnMiss();
	}

	void		SeekDocReader ( SphOffset_t iPos, int iHint ) final	{ m_tDocs.SeekTo ( iPos, iHint ); }
	void		SeekHitReader ( SphOffset_t iPos, int iHint ) final	{ m_tHits.SeekTo ( iPos, iHint ); }
	SphOffset_t	GetDocReaderPos() const final						{ return m_tDocs.GetPos(); }
	SphOffset_t	GetHitReaderPos() const final						{ return m_tHits.GetPos(); }

	void GetHitlistEntry ()
	{
		assert ( !m_bHitlistOver );
		DWORD iDelta = m_tHits.UnzipInt();
		if ( iDelta )
		{
			m_iHitPos += iDelta;
		} else
		{
			m_iHitPos = EMPTY_HIT;
#ifndef NDEBUG
			m_bHitlistOver = true;
#endif
		}
	}


	RowID_t AdvanceTo ( RowID_t tRowID ) final
	{
		if ( m_tDoc.m_tRowID!=INVALID_ROWID && tRowID<=m_tDoc.m_tRowID )
			return m_tDoc.m_tRowID;

		bool bRewound = HintRowID (tRowID);
		if ( bRewound || m_tDoc.m_tRowID==INVALID_ROWID )
			ReadNext();

		if constexpr ( READER::IS_RAW )
		{
			while ( m_tDoc.m_tRowID < tRowID )
			{
				DocCursor_t tCursor;
				if ( OpenCursor ( tCursor ) )
				{
					do
						DecodeNext ( tCursor );
					while ( m_tDoc.m_tRowID < tRowID && tCursor.Has() );
					CloseCursor ( tCursor );
				} else
					DecodeNext ( m_tDocs ); // bounds-checked tail of the map
			}
		} else
		{
			while ( m_tDoc.m_tRowID < tRowID )
				DecodeNext ( m_tDocs );
		}

		return m_tDoc.m_tRowID;
	}


	bool HintRowID ( RowID_t tRowID ) final
	{
		// tricky bit
		// FindSpan() will match a block where tBaseRowIDPlus1[i] <= tRowID < tBaseRowIDPlus1[i+1]
		// meaning that the subsequent ids decoded will be strictly > RefValue
		// meaning that if previous (!) block ends with tRowID exactly,
		// and we use tRowID itself as RefValue, that document gets lost!

		// first check if we're still inside the last block
		if ( m_iSkipListBlock==-1 )
		{
			if ( !m_pSkipData )
				return true;

			m_iSkipListBlock = FindSpan ( m_pSkipData->m_dSkiplist, tRowID );
			if ( m_iSkipListBlock<0 )
				return false;
		}
		else
		{
			assert(m_pSkipData);
			const auto & dSkiplist = m_pSkipData->m_dSkiplist;

			if ( m_iSkipListBlock < dSkiplist.GetLength()-1 )
			{
				int iNextBlock = m_iSkipListBlock+1;
				RowID_t tNextBlockRowID = dSkiplist[iNextBlock].m_tBaseRowIDPlus1;
				if ( tRowID>=tNextBlockRowID )
				{
					auto dSkips = VecTraits_T<SkiplistEntry_t> ( &dSkiplist[iNextBlock], dSkiplist.GetLength()-iNextBlock );
					m_iSkipListBlock = FindSpan ( dSkips, tRowID );
					if ( m_iSkipListBlock<0 )
						return false;

					m_iSkipListBlock += iNextBlock;
				}
			}
			else // we're already at our last block, no need to search
				return false;
		}

		assert(m_pSkipData);
		const SkiplistEntry_t & t = m_pSkipData->m_dSkiplist[m_iSkipListBlock];
		if ( t.m_iOffset<=m_tDocs.GetPos() )
			return false;

		m_tDocs.SeekTo ( t.m_iOffset, -1 );
		m_tDoc.m_tRowID = t.m_tBaseRowIDPlus1-1;
		m_uHitPosition = m_iHitlistPos = t.m_iBaseHitlistPos;

		return true;
	}

	const CSphMatch & GetNextDoc() override
	{
		ReadNext();
		return m_tDoc;
	}

	int GetNextDocs ( QwordDoc_t * pDocs, int iMax ) override
	{
		int iGot = 0;
		if constexpr ( READER::IS_RAW )
		{
			while ( iGot<iMax )
			{
				DocCursor_t tCursor;
				if ( OpenCursor ( tCursor ) )
				{
					while ( iGot<iMax && tCursor.Has() )
					{
						DecodeNext ( tCursor );
						if ( m_tDoc.m_tRowID==INVALID_ROWID )
						{
							CloseCursor ( tCursor );
							return iGot;
						}

						StoreDoc ( pDocs[iGot++] );
					}
					CloseCursor ( tCursor );
				} else
				{
					DecodeNext ( m_tDocs ); // bounds-checked tail of the map
					if ( m_tDoc.m_tRowID==INVALID_ROWID )
						return iGot;

					StoreDoc ( pDocs[iGot++] );
				}
			}
		} else
		{
			while ( iGot<iMax )
			{
				DecodeNext ( m_tDocs );
				if ( m_tDoc.m_tRowID==INVALID_ROWID )
					break;

				StoreDoc ( pDocs[iGot++] );
			}
		}

		return iGot;
	}

	void SeekHitlist ( SphOffset_t uOff ) final
	{
		if ( uOff >> 63 )
		{
			m_uHitState = 1;
			m_uInlinedHit = (DWORD)uOff; // truncate high dword
		} else
		{
			m_uHitState = 0;
			m_iHitPos = EMPTY_HIT;
			if constexpr ( DISABLE_HITLIST_SEEK )
				assert ( m_tHits.GetPos()==uOff ); // make sure we're where caller thinks we are.
			else
				m_tHits.SeekTo ( uOff, READ_NO_SIZE_HINT );
		}
#ifndef NDEBUG
		m_bHitlistOver = false;
#endif
	}

	Hitpos_t GetNextHit () final
	{
		assert ( m_bHasHitlist );
		switch ( m_uHitState )
		{
		case 0: // read hit from hitlist
			GetHitlistEntry ();
			return m_iHitPos;

		case 1: // return inlined hit
			m_uHitState = 2;
			return m_uInlinedHit;

		case 2: // return end-of-hitlist marker after inlined hit
#ifndef NDEBUG
			m_bHitlistOver = true;
#endif
			m_uHitState = 0;
			return EMPTY_HIT;
		}
		sphDie ( "INTERNAL ERROR: impossible hit emitter state" );
		return EMPTY_HIT;
	}

	bool Setup ( const DiskIndexQwordSetup_c * pSetup ) override
	{
		return pSetup->Setup ( this );
	}

	using is_worddict =  std::integral_constant<bool, !DISABLE_HITLIST_SEEK>;

private:
	int m_iSkipListBlock = -1;
	READER m_tDocs;
	READER m_tHits;
	PrefetchGate_t m_tHitGate;	// one per term: hitlist prefetch requests per CollectHits call follow the same schedule as doclists

	void AttachReaders()
	{
		m_tDocs.Attach ( m_rdDoclist );
		m_tHits.Attach ( m_rdHitlist );
	}

	void OnDocReaderChanged() final { m_tDocs.Attach ( m_rdDoclist ); }
	void OnHitReaderChanged() final { m_tHits.Attach ( m_rdHitlist ); }

	// upper bound of one encoded doclist entry: inline layout is delta(5) + hits(5) + first(5) + field(5) | hitpos delta(10),
	// the non-inline one is delta(5) + hitlist delta(10) + fields(5) + hits(5). A window of this size decodes one entry
	// without bounds checks
	static const int MAX_POSTING_BYTES = 25;

	// raw flavour only: a stack-local copy of the map pointer for the duration of a decode loop, so that the
	// compiler keeps it in a register; same decoding interface as the reader policies
	struct DocCursor_t
	{
		const BYTE *	m_pCur = nullptr;
		const BYTE *	m_pEnd = nullptr;

		FORCE_INLINE bool			Has() const		{ return m_pEnd-m_pCur>=MAX_POSTING_BYTES; }
		FORCE_INLINE RowID_t		UnzipRowid()	{ return UnzipIntBE ( m_pCur ); }
		FORCE_INLINE DWORD			UnzipInt()		{ return UnzipIntBE ( m_pCur ); }
		FORCE_INLINE SphOffset_t	UnzipOffset()	{ return UnzipOffsetBE ( m_pCur ); }
	};

	// raw flavour only: false when fewer than MAX_POSTING_BYTES remain, then the caller decodes through
	// the policy's bounds-checked path instead
	FORCE_INLINE bool OpenCursor ( DocCursor_t & tCursor )
	{
		tCursor.m_pCur = m_tDocs.m_pCur;
		tCursor.m_pEnd = m_tDocs.m_pEnd;
		return tCursor.Has();
	}

	FORCE_INLINE void CloseCursor ( DocCursor_t & tCursor )
	{
		assert ( tCursor.m_pCur<=tCursor.m_pEnd );
		m_tDocs.m_pCur = tCursor.m_pCur;
	}

	FORCE_INLINE void StoreDoc ( QwordDoc_t & tDoc ) const
	{
		tDoc.m_tRowID = m_tDoc.m_tRowID;
		tDoc.m_uFields = m_dQwordFields.GetMask32();
		tDoc.m_uMatchHits = m_uMatchHits;
		tDoc.m_iHitlistPos = m_iHitlistPos;
	}

	// decode one doclist entry from SRC (the virtual reader, or a DocCursor_t over its window)
	template <typename SRC>
	FORCE_INLINE void DecodeNext ( SRC & tSrc )
	{
		RowID_t uDelta = tSrc.UnzipRowid();
		if ( uDelta )
		{
			m_bAllFieldsKnown = false;
			m_tDoc.m_tRowID += uDelta;

			if_const ( INLINE_HITS )
			{
				m_uMatchHits = tSrc.UnzipInt();
				const DWORD uFirst = tSrc.UnzipInt();
				if ( m_uMatchHits==1 && m_bHasHitlist )
				{
					DWORD uField = tSrc.UnzipInt(); // field and end marker
					m_iHitlistPos = uFirst | ( uField << 23 ) | ( U64C(1)<<63 );
					m_dQwordFields.UnsetAll();
					// want to make sure bad field data not cause crash
					m_dQwordFields.Set ( ( uField >> 1 ) & ( (DWORD)SPH_MAX_FIELDS-1 ) );
					m_bAllFieldsKnown = true;
				} else
				{
					m_dQwordFields.Assign32 ( uFirst );
					m_uHitPosition += tSrc.UnzipOffset();
					m_iHitlistPos = m_uHitPosition;
				}
			} else
			{
				SphOffset_t iDeltaPos = tSrc.UnzipOffset();
				assert ( iDeltaPos>=0 );

				m_iHitlistPos += iDeltaPos;

				m_dQwordFields.Assign32 ( tSrc.UnzipInt() );
				m_uMatchHits = tSrc.UnzipInt();
			}
		} else
			m_tDoc.m_tRowID = INVALID_ROWID;
	}

	inline void ReadNext()
	{
		if constexpr ( READER::IS_RAW )
		{
			DocCursor_t tCursor;
			if ( OpenCursor ( tCursor ) )
			{
				DecodeNext ( tCursor );
				CloseCursor ( tCursor );
			} else
				DecodeNext ( m_tDocs ); // bounds-checked tail of the map
		} else
			DecodeNext ( m_tDocs );
	}

private:
	int64_t m_iIndexId = 0;
};


/////////////////////////////////////////////////////////////////////////////

#define WITH_QWORD(INDEX, NO_SEEK, NAME, ACTION)										\
do if ( (( const CSphIndex_VLN *)INDEX)->m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )	\
		{ using NAME = DiskIndexQword_c < true, NO_SEEK, BlockReader_t >; ACTION; }		\
	else																				\
		{ using NAME = DiskIndexQword_c < false, NO_SEEK, BlockReader_t >; ACTION; }	\
while(0)

template < bool INLINE_HITS, typename READER >
class DiskPayloadQword_c : public DiskIndexQword_c<INLINE_HITS, false, READER>
{
	typedef DiskIndexQword_c<INLINE_HITS, false, READER> BASE;

public:
	DiskPayloadQword_c ( const DiskSubstringPayload_t * pPayload, bool bExcluded, DataReaderFactory_c * pDoclist, DataReaderFactory_c * pHitlist, int64_t iIndexId )
		: BASE ( true, bExcluded, iIndexId )
	{
		m_pPayload = pPayload;
		this->m_iDocs = m_pPayload->m_iTotalDocs;
		this->m_iHits = m_pPayload->m_iTotalHits;
		m_iDoclist = 0;

		this->SetDocReader ( pDoclist );
		this->SetHitReader ( pHitlist );
	}

	const CSphMatch & GetNextDoc() final
	{
		const CSphMatch & tMatch = BASE::GetNextDoc();
		assert ( &tMatch==&this->m_tDoc );
		if ( tMatch.m_tRowID==INVALID_ROWID && m_iDoclist<m_pPayload->m_dDoclist.GetLength() )
		{
			BASE::ResetDecoderState();
			SetupReader();
			BASE::GetNextDoc();
			assert ( this->m_tDoc.m_tRowID!=INVALID_ROWID );
		}

		return this->m_tDoc;
	}

	int GetNextDocs ( QwordDoc_t * pDocs, int iMax ) final
	{
		return ISphQword::GetNextDocs ( pDocs, iMax ); // goes through GetNextDoc(), which chains the doclists
	}

	bool Setup ( const DiskIndexQwordSetup_c * ) final
	{
		if ( m_iDoclist>=m_pPayload->m_dDoclist.GetLength() )
			return false;

		SetupReader();
		return true;
	}

private:
	void SetupReader ()
	{
		uint64_t uDocOff = m_pPayload->m_dDoclist[m_iDoclist].m_uOff;
		int iHint = m_pPayload->m_dDoclist[m_iDoclist].m_iLen;
		m_iDoclist++;

		this->SeekDocReader ( uDocOff, iHint );
	}

	const DiskSubstringPayload_t *	m_pPayload;
	int								m_iDoclist;
};
