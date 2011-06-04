//
// $Id$
//

//
// Copyright (c) 2001-2011, Andrew Aksyonoff
// Copyright (c) 2008-2011, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxrt.h"
#include "sphinxsearch.h"
#include "sphinxutils.h"

#include <sys/stat.h>
#include <fcntl.h>

#if USE_WINDOWS
#include <io.h> // for open(), close()
#include <errno.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////

#define BINLOG_WRITE_BUFFER		256*1024
#define BINLOG_AUTO_FLUSH		1000000
#define BINLOG_RESTART_SIZE		128*1024*1024

#if USE_64BIT
#define WORDID_MAX				U64C(0xffffffffffffffff)
#else
#define	WORDID_MAX				0xffffffffUL
#endif

// RT hitman
typedef Hitman_c<8> HITMAN;

//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
#define Verify(_expr) assert(_expr)
#else
#define Verify(_expr) _expr
#endif

//////////////////////////////////////////////////////////////////////////

// !COMMIT cleanup extern ref to sphinx.cpp
extern void sphSortDocinfos ( DWORD * pBuf, int iCount, int iStride );

// !COMMIT yes i am when debugging
#ifndef NDEBUG
#define PARANOID 1
#endif

//////////////////////////////////////////////////////////////////////////

template < typename T, typename P >
static inline void ZipT ( CSphVector < BYTE, P > & dOut, T uValue )
{
	do
	{
		BYTE bOut = (BYTE)( uValue & 0x7f );
		uValue >>= 7;
		if ( uValue )
			bOut |= 0x80;
		dOut.Add ( bOut );
	} while ( uValue );
}


template < typename T >
static inline const BYTE * UnzipT ( T * pValue, const BYTE * pIn )
{
	T uValue = 0;
	BYTE bIn;
	int iOff = 0;

	do
	{
		bIn = *pIn++;
		uValue += ( T ( bIn & 0x7f ) ) << iOff;
		iOff += 7;
	} while ( bIn & 0x80 );

	*pValue = uValue;
	return pIn;
}

#define ZipDword ZipT<DWORD>
#define ZipQword ZipT<uint64_t>
#define UnzipDword UnzipT<DWORD>
#define UnzipQword UnzipT<uint64_t>

#if USE_64BIT
#define ZipDocid ZipQword
#define ZipWordid ZipQword
#define UnzipDocid UnzipQword
#define UnzipWordid UnzipQword
#else
#define ZipDocid ZipDword
#define ZipWordid ZipDword
#define UnzipDocid UnzipDword
#define UnzipWordid UnzipDword
#endif

//////////////////////////////////////////////////////////////////////////

struct CmpHit_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b )
	{
		return 	( a.m_iWordID<b.m_iWordID ) ||
			( a.m_iWordID==b.m_iWordID && a.m_iDocID<b.m_iDocID ) ||
			( a.m_iWordID==b.m_iWordID && a.m_iDocID==b.m_iDocID && a.m_iWordPos<b.m_iWordPos );
	}
};

template < typename DOCID = SphDocID_t >
struct RtDoc_tmpl
{
	DOCID						m_uDocID;	///< my document id
	DWORD						m_uFields;	///< fields mask
	DWORD						m_uHits;	///< hit count
	DWORD						m_uHit;		///< either index into segment hits, or the only hit itself (if hit count is 1)
};

template < typename WORDID = SphWordID_t >
struct RtWord_tmpl
{
	WORDID						m_uWordID;	///< my keyword id
	DWORD						m_uDocs;	///< document count (for stats and/or BM25)
	DWORD						m_uHits;	///< hit count (for stats and/or BM25)
	DWORD						m_uDoc;		///< index into segment docs
};

typedef RtDoc_tmpl<> RtDoc_t;
typedef RtWord_tmpl<> RtWord_t;

struct RtWordCheckpoint_t
{
	SphWordID_t					m_uWordID;
	int							m_iOffset;
};

class RtDiskKlist_t : public ISphNoncopyable
{
private:
	static const int				MAX_SMALL_SIZE = 512;
	CSphVector < SphAttr_t >		m_dLargeKlist;
	CSphOrderedHash < bool, SphDocID_t, IdentityHash_fn, MAX_SMALL_SIZE, 11 >	m_hSmallKlist;
	mutable CSphRwlock				m_tRwLargelock;
	mutable CSphRwlock				m_tRwSmalllock;

	void NakedFlush();				// flush without lockers

public:
	RtDiskKlist_t() { m_tRwLargelock.Init(); m_tRwSmalllock.Init(); }
	virtual ~RtDiskKlist_t() { m_tRwLargelock.Done(); m_tRwSmalllock.Done(); }
	void Reset ();
	void Flush()
	{
		if ( m_hSmallKlist.GetLength()==0 )
			return;
		m_tRwSmalllock.WriteLock();
		m_tRwLargelock.WriteLock();
		NakedFlush();
		m_tRwLargelock.Unlock();
		m_tRwSmalllock.Unlock();
	}
	void LoadFromFile ( const char * sFilename );
	void SaveToFile ( const char * sFilename );
	inline void Delete ( SphDocID_t uDoc )
	{
		m_tRwSmalllock.WriteLock();
		if ( !m_hSmallKlist.Exists ( uDoc ) )
			m_hSmallKlist.Add ( true, uDoc );
		if ( m_hSmallKlist.GetLength()>=MAX_SMALL_SIZE )
			NakedFlush();
		m_tRwSmalllock.Unlock();
	}
	inline const SphAttr_t * GetKillList () const { return m_dLargeKlist.Begin(); }
	inline int	GetKillListSize () const { return m_dLargeKlist.GetLength(); }
	inline bool KillListLock() const { return m_tRwLargelock.ReadLock(); }
	inline bool KillListUnlock() const { return m_tRwLargelock.Unlock(); }

	// NOT THREAD SAFE
	bool Exists ( SphDocID_t uDoc )
	{
		return ( m_hSmallKlist.Exists ( uDoc ) || m_dLargeKlist.BinarySearch ( SphAttr_t(uDoc))!=NULL );
	}
};

void RtDiskKlist_t::Reset()
{
	m_dLargeKlist.Reset();
	m_hSmallKlist.Reset();
}

void RtDiskKlist_t::NakedFlush()
{
	if ( m_hSmallKlist.GetLength()==0 )
		return;
	m_hSmallKlist.IterateStart();
	while ( m_hSmallKlist.IterateNext() )
		m_dLargeKlist.Add ( m_hSmallKlist.IterateGetKey() );
	m_dLargeKlist.Uniq();
	m_hSmallKlist.Reset();
}

// is already id32<>id64 safe
void RtDiskKlist_t::LoadFromFile ( const char * sFilename )
{
	m_tRwLargelock.WriteLock();
	m_tRwSmalllock.WriteLock();
	m_hSmallKlist.Reset();
	m_tRwSmalllock.Unlock();

	m_dLargeKlist.Reset();
	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	if ( !sphIsReadable ( sName.cstr(), &sError ) )
	{
		m_tRwLargelock.Unlock();
		return;
	}

	CSphAutoreader rdKlist;
	if ( !rdKlist.Open ( sName, sError ) )
	{
		m_tRwLargelock.Unlock();
		return;
	}

	m_dLargeKlist.Resize ( rdKlist.GetDword() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		uLastDocID += ( SphDocID_t ) rdKlist.UnzipOffset();
		m_dLargeKlist[i] = uLastDocID;
	};
	m_tRwLargelock.Unlock();
}

void RtDiskKlist_t::SaveToFile ( const char * sFilename )
{
	m_tRwLargelock.WriteLock();
	m_tRwSmalllock.WriteLock();
	NakedFlush();
	m_tRwSmalllock.Unlock();

	CSphWriter wrKlist;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.kill", sFilename );
	wrKlist.OpenFile ( sName.cstr(), sError );

	wrKlist.PutDword ( m_dLargeKlist.GetLength() );
	SphDocID_t uLastDocID = 0;
	ARRAY_FOREACH ( i, m_dLargeKlist )
	{
		wrKlist.ZipOffset ( m_dLargeKlist[i] - uLastDocID );
		uLastDocID = ( SphDocID_t ) m_dLargeKlist[i];
	};
	m_tRwLargelock.Unlock();
	wrKlist.CloseFile ();
}

struct RtSegment_t
{
protected:
	static const int			KLIST_ACCUM_THRESH	= 32;

public:
	static CSphStaticMutex		m_tSegmentSeq;
	static int					m_iSegments;	///< age tag sequence generator
	int							m_iTag;			///< segment age tag

	CSphTightVector<BYTE>			m_dWords;
	CSphVector<RtWordCheckpoint_t>	m_dWordCheckpoints;
	CSphTightVector<BYTE>		m_dDocs;
	CSphTightVector<BYTE>		m_dHits;

	int							m_iRows;		///< number of actually allocated rows
	int							m_iAliveRows;	///< number of alive (non-killed) rows
	CSphVector<CSphRowitem>		m_dRows;		///< row data storage
	CSphVector<SphDocID_t>		m_dKlist;		///< sorted K-list
	bool						m_bTlsKlist;	///< whether to apply TLS K-list during merge (must only be used by writer during Commit())
	CSphTightVector<BYTE>		m_dStrings;		///< strings storage
	CSphTightVector<DWORD>		m_dMvas;		///< MVAs storage

	RtSegment_t ()
	{
		m_tSegmentSeq.Lock ();
		m_iTag = m_iSegments++;
		m_tSegmentSeq.Unlock ();
		m_iRows = 0;
		m_iAliveRows = 0;
		m_bTlsKlist = false;
		m_dStrings.Add ( 0 ); // dummy zero offset
		m_dMvas.Add ( 0 ); // dummy zero offset
	}

	int64_t GetUsedRam () const
	{
		// FIXME! gonna break on vectors over 2GB
		return
			m_dWords.GetLimit()*sizeof(m_dWords[0]) +
			m_dDocs.GetLimit()*sizeof(m_dDocs[0]) +
			m_dHits.GetLimit()*sizeof(m_dHits[0]) +
			m_dStrings.GetLimit()*sizeof(m_dStrings[0])+
			m_dMvas.GetLimit()*sizeof(m_dMvas[0]);
	}

	int GetMergeFactor () const
	{
		return m_iRows;
	}

	const CSphRowitem *		FindRow ( SphDocID_t uDocid ) const;
	const CSphRowitem *		FindAliveRow ( SphDocID_t uDocid ) const;
};

int RtSegment_t::m_iSegments = 0;
CSphStaticMutex RtSegment_t::m_tSegmentSeq;


const CSphRowitem * RtSegment_t::FindRow ( SphDocID_t uDocid ) const
{
	// binary search through the rows
	int iStride = m_dRows.GetLength() / m_iRows;
	SphDocID_t uL = DOCINFO2ID ( m_dRows.Begin() );
	SphDocID_t uR = DOCINFO2ID ( &m_dRows[m_dRows.GetLength()-iStride] );

	if ( uDocid==uL )
		return m_dRows.Begin();

	if ( uDocid==uR )
		return &m_dRows[m_dRows.GetLength()-iStride];

	if ( uDocid<uL || uDocid>uR )
		return NULL;

	int iL = 0;
	int iR = m_iRows-1;
	while ( iR-iL>1 )
	{
		int iM = iL + (iR-iL)/2;
		SphDocID_t uM = DOCINFO2ID ( &m_dRows[iM*iStride] );

		if ( uDocid==uM )
			return &m_dRows[iM*iStride];
		else if ( uDocid>uM )
			iL = iM;
		else
			iR = iM;
	}
	return NULL;
}


const CSphRowitem * RtSegment_t::FindAliveRow ( SphDocID_t uDocid ) const
{
	if ( m_dKlist.BinarySearch ( uDocid ) )
		return NULL;
	else
		return FindRow ( uDocid );
}

//////////////////////////////////////////////////////////////////////////

struct RtDocWriter_t
{
	CSphTightVector<BYTE> *		m_pDocs;
	SphDocID_t					m_uLastDocID;

	explicit RtDocWriter_t ( RtSegment_t * pSeg )
		: m_pDocs ( &pSeg->m_dDocs )
		, m_uLastDocID ( 0 )
	{}

	void ZipDoc ( const RtDoc_t & tDoc )
	{
		CSphTightVector<BYTE> & dDocs = *m_pDocs;
		ZipDocid ( dDocs, tDoc.m_uDocID - m_uLastDocID );
		m_uLastDocID = tDoc.m_uDocID;
		ZipDword ( dDocs, tDoc.m_uFields );
		ZipDword ( dDocs, tDoc.m_uHits );
		if ( tDoc.m_uHits==1 )
		{
			ZipDword ( dDocs, tDoc.m_uHit & 0xffffffUL );
			ZipDword ( dDocs, tDoc.m_uHit>>24 );
		} else
			ZipDword ( dDocs, tDoc.m_uHit );
	}

	DWORD ZipDocPtr () const
	{
		return m_pDocs->GetLength();
	}

	void ZipRestart ()
	{
		m_uLastDocID = 0;
	}
};

template < typename DOCID = SphDocID_t >
struct RtDocReader_tmpl
{
	typedef RtDoc_tmpl<DOCID>	RTDOC;
	const BYTE *	m_pDocs;
	int				m_iLeft;
	RTDOC			m_tDoc;

	template < typename RTWORD >
	explicit RtDocReader_tmpl ( const RtSegment_t * pSeg, const RTWORD & tWord )
	{
		m_pDocs = ( pSeg->m_dDocs.Begin() ? pSeg->m_dDocs.Begin() + tWord.m_uDoc : NULL );
		m_iLeft = tWord.m_uDocs;
		m_tDoc.m_uDocID = 0;
	}

	const RTDOC * UnzipDoc_t ()
	{
		if ( !m_iLeft || !m_pDocs )
			return NULL;

		const BYTE * pIn = m_pDocs;
		SphDocID_t uDeltaID;
		pIn = UnzipDocid ( &uDeltaID, pIn );
		RTDOC & mtDoc = *(RTDOC*)&m_tDoc;
		mtDoc.m_uDocID += (DOCID) uDeltaID;
		pIn = UnzipDword ( &mtDoc.m_uFields, pIn );
		pIn = UnzipDword ( &mtDoc.m_uHits, pIn );
		if ( mtDoc.m_uHits==1 )
		{
			DWORD a, b;
			pIn = UnzipDword ( &a, pIn );
			pIn = UnzipDword ( &b, pIn );
			mtDoc.m_uHit = a + ( b<<24 );
		} else
			pIn = UnzipDword ( &mtDoc.m_uHit, pIn );
		m_pDocs = pIn;

		m_iLeft--;
		return &mtDoc;
	}

	inline const RTDOC * UnzipDoc ()
	{
		return UnzipDoc_t();
	}
};

typedef RtDocReader_tmpl<> RtDocReader_t;

static const int RAM_WORDLIST_CHECKPOINT = 1024;

struct RtWordWriter_t
{
	CSphTightVector<BYTE> *				m_pWords;
	CSphVector<RtWordCheckpoint_t> *	m_pCheckpoints;
	SphWordID_t							m_uLastWordID;
	DWORD								m_uLastDoc;
	int									m_iWords;

	explicit RtWordWriter_t ( RtSegment_t * pSeg )
		: m_pWords ( &pSeg->m_dWords )
		, m_pCheckpoints ( &pSeg->m_dWordCheckpoints )
		, m_uLastWordID ( 0 )
		, m_uLastDoc ( 0 )
		, m_iWords ( 0 )
	{
		assert ( !m_pWords->GetLength() );
		assert ( !m_pCheckpoints->GetLength() );
	}

	void ZipWord ( const RtWord_t & tWord )
	{
		CSphTightVector<BYTE> & tWords = *m_pWords;
		if ( ++m_iWords==RAM_WORDLIST_CHECKPOINT )
		{
			RtWordCheckpoint_t & tCheckpoint = m_pCheckpoints->Add();
			tCheckpoint.m_uWordID = tWord.m_uWordID;
			tCheckpoint.m_iOffset = tWords.GetLength();

			m_uLastWordID = 0;
			m_uLastDoc = 0;
			m_iWords = 1;
		}

		ZipWordid ( tWords, tWord.m_uWordID - m_uLastWordID );
		ZipDword ( tWords, tWord.m_uDocs );
		ZipDword ( tWords, tWord.m_uHits );
		ZipDword ( tWords, tWord.m_uDoc - m_uLastDoc );
		m_uLastWordID = tWord.m_uWordID;
		m_uLastDoc = tWord.m_uDoc;
	}
};

template < typename WORDID = SphWordID_t >
struct RtWordReader_tmpl
{
	typedef RtWord_tmpl<WORDID> RTWORD;
	const BYTE *	m_pCur;
	const BYTE *	m_pMax;
	RTWORD			m_tWord;
	int				m_iWords;

	explicit RtWordReader_tmpl ( const RtSegment_t * pSeg )
		: m_iWords ( 0 )
	{
		m_pCur = pSeg->m_dWords.Begin();
		m_pMax = m_pCur + pSeg->m_dWords.GetLength();

		m_tWord.m_uWordID = 0;
		m_tWord.m_uDoc = 0;
	}

	const RTWORD * UnzipWord_t ()
	{
		RTWORD & mtWord = *(RTWORD*)&m_tWord;
		if ( ++m_iWords==RAM_WORDLIST_CHECKPOINT )
		{
			mtWord.m_uWordID = 0;
			mtWord.m_uDoc = 0;
			m_iWords = 1;
		}
		if ( m_pCur>=m_pMax )
			return NULL;

		const BYTE * pIn = m_pCur;
		SphWordID_t uDeltaID;
		DWORD uDeltaDoc;
		pIn = UnzipWordid ( &uDeltaID, pIn );
		pIn = UnzipDword ( &mtWord.m_uDocs, pIn );
		pIn = UnzipDword ( &mtWord.m_uHits, pIn );
		pIn = UnzipDword ( &uDeltaDoc, pIn );
		m_pCur = pIn;

		mtWord.m_uWordID += (WORDID) uDeltaID;
		mtWord.m_uDoc += uDeltaDoc;
		return &mtWord;
	}

	inline const RTWORD * UnzipWord ()
	{
		return UnzipWord_t ();
	}
};

typedef RtWordReader_tmpl<SphWordID_t> RtWordReader_t;

struct RtHitWriter_t
{
	CSphTightVector<BYTE> *		m_pHits;
	DWORD						m_uLastHit;

	explicit RtHitWriter_t ( RtSegment_t * pSeg )
		: m_pHits ( &pSeg->m_dHits )
		, m_uLastHit ( 0 )
	{}

	void ZipHit ( DWORD uValue )
	{
		ZipDword ( *m_pHits, uValue - m_uLastHit );
		m_uLastHit = uValue;
	}

	void ZipRestart ()
	{
		m_uLastHit = 0;
	}

	DWORD ZipHitPtr () const
	{
		return m_pHits->GetLength();
	}
};


struct RtHitReader_t
{
	const BYTE *	m_pCur;
	DWORD			m_iLeft;
	DWORD			m_uLast;

	RtHitReader_t ()
		: m_pCur ( NULL )
		, m_iLeft ( 0 )
		, m_uLast ( 0 )
	{}

	template < typename RTDOC >
	explicit RtHitReader_t ( const RtSegment_t * pSeg, const RTDOC * pDoc )
	{
		m_pCur = &pSeg->m_dHits [ pDoc->m_uHit ];
		m_iLeft = pDoc->m_uHits;
		m_uLast = 0;
	}

	DWORD UnzipHit ()
	{
		if ( !m_iLeft )
			return 0;

		DWORD uValue;
		m_pCur = UnzipDword ( &uValue, m_pCur );
		m_uLast += uValue;
		m_iLeft--;
		return m_uLast;
	}
};


struct RtHitReader2_t : public RtHitReader_t
{
	const BYTE * m_pBase;

	RtHitReader2_t ()
		: m_pBase ( NULL )
	{}

	void Seek ( SphOffset_t uOff, int iHits )
	{
		m_pCur = m_pBase + uOff;
		m_iLeft = iHits;
		m_uLast = 0;
	}
};

//////////////////////////////////////////////////////////////////////////

/// forward ref
struct RtIndex_t;

struct AccDocDup_t
{
	SphDocID_t m_uDocid;
	int m_iDupCount;
};


/// indexing accumulator
struct RtAccum_t
{
	RtIndex_t *					m_pIndex;		///< my current owner in this thread
	int							m_iAccumDocs;
	CSphVector<CSphWordHit>		m_dAccum;
	CSphVector<CSphRowitem>		m_dAccumRows;
	CSphVector<SphDocID_t>		m_dAccumKlist;
	CSphTightVector<BYTE>		m_dStrings;
	CSphTightVector<DWORD>		m_dMvas;
	CSphVector<DWORD>			m_dPerDocHitsCount;

					RtAccum_t();
	void			AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas );
	RtSegment_t *	CreateSegment ( int iRowSize );
	void			CleanupDuplacates ( int iRowSize );
};

/// TLS indexing accumulator (we disallow two uncommitted adds within one thread; and so need at most one)
SphThreadKey_t g_tTlsAccumKey;

/// binlog file view of the index
/// everything that a given log file needs to know about an index
struct BinlogIndexInfo_t
{
	CSphString	m_sName;			///< index name
	int64_t		m_iMinTID;			///< min TID logged by this file
	int64_t		m_iMaxTID;			///< max TID logged by this file
	int64_t		m_iFlushedTID;		///< last flushed TID
	int64_t		m_tmMin;			///< min TID timestamp
	int64_t		m_tmMax;			///< max TID timestamp

	CSphIndex *	m_pIndex;			///< replay only; associated index (might be NULL if we don't serve it anymore!)
	RtIndex_t *	m_pRT;				///< replay only; RT index handle (might be NULL if N/A or non-RT)
	int64_t		m_iPreReplayTID;	///< replay only; index TID at the beginning of this file replay

	BinlogIndexInfo_t ()
		: m_iMinTID ( INT64_MAX )
		, m_iMaxTID ( 0 )
		, m_iFlushedTID ( 0 )
		, m_tmMin ( INT64_MAX )
		, m_tmMax ( 0 )
		, m_pIndex ( NULL )
		, m_pRT ( NULL )
		, m_iPreReplayTID ( 0 )
	{}
};

/// binlog file descriptor
/// file id (aka extension), plus a list of associated index infos
struct BinlogFileDesc_t
{
	int								m_iExt;
	CSphVector<BinlogIndexInfo_t>	m_dIndexInfos;

	BinlogFileDesc_t ()
		: m_iExt ( 0 )
	{}
};

/// Bin Log Operation
enum Blop_e
{
	BLOP_COMMIT			= 1,
	BLOP_UPDATE_ATTRS	= 2,
	BLOP_ADD_INDEX		= 3,
	BLOP_ADD_CACHE		= 4,

	BLOP_TOTAL
};

// forward declaration
class BufferReader_t;
class RtBinlog_c;
struct RtIndex_t;


class BinlogWriter_c : protected CSphWriter
{
public:
					BinlogWriter_c ();
	virtual			~BinlogWriter_c () {}

	virtual	void	Flush ();
	void			Write ();
	void			Fsync ();
	bool			HasUnwrittenData () const { return m_iPoolUsed>0; }
	bool			HasUnsyncedData () const { return m_iLastFsyncPos!=m_iLastWritePos; }

	void			ResetCrc ();	///< restart checksumming
	void			WriteCrc ();	///< finalize and write current checksum to output stream

	void			SetBufferSize ( int iBufferSize )									{ CSphWriter::SetBufferSize ( iBufferSize ); }
	bool			OpenFile ( const CSphString & sName, CSphString & sErrorBuffer )	{ return CSphWriter::OpenFile ( sName, sErrorBuffer ); }
	void			SetFile ( int iFD, SphOffset_t * pSharedOffset )					{ CSphWriter::SetFile ( iFD, pSharedOffset ); }
	void			CloseFile ( bool bTruncate=false )									{ CSphWriter::CloseFile ( bTruncate ); }
	SphOffset_t		GetPos () const														{ return m_iPos; }

	void			PutBytes ( const void * pData, int iSize );
	void			PutString ( const char * szString );
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			ZipValue ( uint64_t uValue );

private:
	int64_t			m_iLastWritePos;
	int64_t			m_iLastFsyncPos;

	DWORD			m_uCRC;
};


class BinlogReader_c : protected CSphAutoreader
{
public:
	bool			Open ( const CSphString & sFilename, CSphString & sError )		{ return CSphAutoreader::Open ( sFilename, sError ); }
	void			Close ()														{ CSphAutoreader::Close(); }
	SphOffset_t		GetFilesize ()													{ return CSphAutoreader::GetFilesize(); }

	void			GetBytes ( void * pData, int iSize );
	CSphString		GetString ();
	DWORD			GetDword ();
	uint64_t		UnzipValue ();

	bool			GetErrorFlag ()													{ return CSphAutoreader::GetErrorFlag(); }
	SphOffset_t		GetPos ()														{ return CSphAutoreader::GetPos(); }

	void			ResetCrc ();
	bool			CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos );

private:
	DWORD			m_uCRC;
};

class RtBinlog_c : public ISphBinlog
{
public:
	RtBinlog_c ();
	~RtBinlog_c ();

	void	BinlogCommit ( const char * sIndexName, int64_t iTID, const RtSegment_t * pSeg, const CSphVector<SphDocID_t> & dKlist );
	void	BinlogUpdateAttributes ( const char * sIndexName, int64_t iTID, const CSphAttrUpdate & tUpd );
	void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown );

	void	Configure ( const CSphConfigSection & hSearchd, bool bTestMode );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback );

	void	CreateTimerThread ();

private:
	static const DWORD		BINLOG_VERSION = 3;

	static const DWORD		BINLOG_HEADER_MAGIC = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
	static const DWORD		BLOP_MAGIC = 0x214e5854;			/// magic 'TXN!' header that marks binlog entry
	static const DWORD		BINLOG_META_MAGIC = 0x494c5053;		/// magic 'SPLI' header that marks binlog meta

	int64_t					m_iFlushTimeLeft;
	volatile int			m_iFlushPeriod;

	enum OnCommitAction_e
	{
		ACTION_NONE,
		ACTION_FSYNC,
		ACTION_WRITE
	};
	OnCommitAction_e		m_eOnCommit;

	CSphMutex				m_tWriteLock; // lock on operation

	int						m_iLockFD;
	CSphString				m_sWriterError;
	BinlogWriter_c			m_tWriter;

	mutable CSphVector<BinlogFileDesc_t>	m_dLogFiles; // active log files

	CSphString				m_sLogPath;

	SphThread_t				m_tUpdateTread;
	bool					m_bReplayMode; // replay mode indicator
	bool					m_bDisabled;

	int						m_iRestartSize; // binlog size restart threshold

	// replay stats
	mutable int				m_iReplayedRows;

private:
	static void				DoAutoFlush ( void * pBinlog );
	int 					GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow );
	void					LoadMeta ();
	void					SaveMeta ();
	void					LockFile ( bool bLock );
	void					DoCacheWrite ();
	void					CheckDoRestart ();
	void					CheckDoFlush ();
	void					OpenNewLog ( int iLastState=0 );

	int						ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, int iBinlog );
	bool					ReplayCommit ( int iBinlog, BinlogReader_c & tReader ) const;
	bool					ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const;
	bool					ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const;
	bool					ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const;
};

static RtBinlog_c * g_pBinlog = NULL;
static bool g_bRTChangesAllowed = false;

/// RAM based index
struct RtQword_t;
struct RtIndex_t : public ISphRtIndex, public ISphNoncopyable
{
private:
	static const DWORD			META_HEADER_MAGIC	= 0x54525053;	///< my magic 'SPRT' header
	static const DWORD			META_VERSION		= 3;			///< current version

private:
	const int					m_iStride;
	CSphVector<RtSegment_t*>	m_pSegments;

	CSphMutex					m_tWriterMutex;
	mutable CSphRwlock			m_tRwlock;

	int64_t						m_iRamSize;
	CSphString					m_sPath;
	CSphVector<CSphIndex*>		m_pDiskChunks;
	int							m_iLockFD;
	mutable RtDiskKlist_t		m_tKlist;

	CSphSchema					m_tOutboundSchema;
	CSphVector<LocatorPair_t>	m_dDynamize;

	int64_t						m_iSavedTID;
	int64_t						m_iSavedRam;
	int64_t						m_tmSaved;

public:
	explicit					RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath );
	virtual						~RtIndex_t ();

	bool						AddDocument ( int iFields, const char ** ppFields, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError );
	bool						AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError );
	bool						DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError );
	void						Commit ();
	void						RollBack ();

	void						CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist );

	void						DumpToDisk ( const char * sFilename );

	virtual void				CheckRamFlush ();

private:
	/// acquire thread-local indexing accumulator
	/// returns NULL if another index already uses it in an open txn
	RtAccum_t *					AcquireAccum ( CSphString * sError=NULL );

	RtSegment_t *				MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist );
	const RtWord_t *			CopyWord ( RtSegment_t * pDst, RtWordWriter_t & tOutWord, const RtSegment_t * pSrc, const RtWord_t * pWord, RtWordReader_t & tInWord, const CSphVector<SphDocID_t> * pAccKlist );
	void						MergeWord ( RtSegment_t * pDst, const RtSegment_t * pSrc1, const RtWord_t * pWord1, const RtSegment_t * pSrc2, const RtWord_t * pWord2, RtWordWriter_t & tOut, const CSphVector<SphDocID_t> * pAccKlist );
	void						CopyDoc ( RtSegment_t * pSeg, RtDocWriter_t & tOutDoc, RtWord_t * pWord, const RtSegment_t * pSrc, const RtDoc_t * pDoc );

	void						SaveMeta ( int iDiskChunks );
	void						SaveDiskHeader ( const char * sFilename, int iCheckpoints, SphOffset_t iCheckpointsPosition, DWORD uKillListSize, DWORD uMinMaxSize, bool bForceID32=false ) const;
	void						SaveDiskData ( const char * sFilename ) const;
	template < typename DOCID, typename WORDID >
	void						SaveDiskDataImpl ( const char * sFilename ) const;
	void						SaveDiskChunk ();
	CSphIndex *					LoadDiskChunk ( int iChunk, bool bStripPath );
	bool						LoadRamChunk ( DWORD uVersion );
	bool						SaveRamChunk ();

public:
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4100)
#endif
	virtual SphAttr_t *			GetKillList () const				{ return NULL; }
	virtual int					GetKillListSize () const			{ return 0; }
	virtual bool				HasDocid ( SphDocID_t ) const		{ assert ( 0 ); return false; }

	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) { return 0; }
	virtual bool				Merge ( CSphIndex * pSource, CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists ) { return false; }

	virtual bool				Prealloc ( bool bMlock, bool bStripPath, CSphString & sWarning );
	virtual void				Dealloc () {}
	virtual bool				Preread ();
	virtual void				SetBase ( const char * sNewBase ) {}
	virtual bool				Rename ( const char * sNewBase ) { return true; }
	virtual bool				Lock () { return true; }
	virtual void				Unlock () {}
	virtual bool				Mlock () { return true; }
	virtual void				PostSetup();
	virtual bool				IsRT() const { return true; }

	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError );
	virtual bool				SaveAttributes () { return true; }

	virtual void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) {}
	virtual void				DebugDumpDocids ( FILE * fp ) {}
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) {}
	virtual int					DebugCheck ( FILE * fp ) { return 0; }
#if USE_WINDOWS
#pragma warning(pop)
#endif

public:
	virtual bool						EarlyReject ( CSphQueryContext * pCtx, CSphMatch & ) const;
	virtual const CSphSourceStats &		GetStats () const { return m_tStats; }

	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const;

	void						CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const;
	const CSphRowitem *			FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID ) const;

	bool						RtQwordSetup ( RtQword_t * pQword, RtSegment_t * pSeg ) const;
	static bool					RtQwordSetupSegment ( RtQword_t * pQword, RtSegment_t * pSeg, bool bSetup );

	virtual const CSphSchema &	GetMatchSchema () const { return m_tOutboundSchema; }
	virtual const CSphSchema &	GetInternalSchema () const { return m_tSchema; }
	int64_t						GetUsedRam () const;

protected:
	CSphSourceStats				m_tStats;
};


RtIndex_t::RtIndex_t ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath )
	: ISphRtIndex ( sIndexName, "rtindex" )
	, m_iStride ( DOCINFO_IDSIZE + tSchema.GetRowSize() )
	, m_iRamSize ( iRamSize )
	, m_sPath ( sPath )
	, m_iLockFD ( -1 )
	, m_iSavedTID ( m_iTID )
	, m_iSavedRam ( 0 )
	, m_tmSaved ( sphMicroTimer() )
{
	MEMORY ( SPH_MEM_IDX_RT );

	m_tSchema = tSchema;

	// schemes strings attributes fix up
	bool bReplaceSchema = false;
	for ( int i=0; i<tSchema.GetAttrsCount() && !bReplaceSchema; i++ )
		bReplaceSchema = ( ( tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_STRING || tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_UINT32SET )
							&& !tSchema.GetAttr(i).m_tLocator.m_bDynamic );

	m_tOutboundSchema = m_tSchema;
	if ( bReplaceSchema )
	{
		m_tOutboundSchema.ResetAttrs();
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			CSphColumnInfo tCol = m_tSchema.GetAttr(i);
			bool bDynamic = tCol.m_tLocator.m_bDynamic;
			if ( ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_UINT32SET ) && !tCol.m_tLocator.m_bDynamic )
			{
				tCol.m_eStage = SPH_EVAL_OVERRIDE;
				bDynamic = true;
			}

			m_tOutboundSchema.AddAttr ( tCol, bDynamic );

			if ( bDynamic )
			{
				LocatorPair_t & tPair = m_dDynamize.Add();
				tPair.m_tFrom = tCol.m_tLocator;
				tPair.m_tTo = m_tOutboundSchema.GetAttr(i).m_tLocator;
			}
		}
	}

#ifndef NDEBUG
	// check that index cols are static
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		assert ( !m_tSchema.GetAttr(i).m_tLocator.m_bDynamic );
#endif

	Verify ( m_tWriterMutex.Init() );
	Verify ( m_tRwlock.Init() );
}


RtIndex_t::~RtIndex_t ()
{
	int64_t tmSave = sphMicroTimer();

	SaveRamChunk ();
	SaveMeta ( m_pDiskChunks.GetLength() );

	Verify ( m_tWriterMutex.Done() );
	Verify ( m_tRwlock.Done() );

	ARRAY_FOREACH ( i, m_pSegments )
		SafeDelete ( m_pSegments[i] );

	ARRAY_FOREACH ( i, m_pDiskChunks )
		SafeDelete ( m_pDiskChunks[i] );

	if ( m_iLockFD>=0 )
		::close ( m_iLockFD );

	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, true );

	tmSave = sphMicroTimer() - tmSave;
	if ( tmSave>=1000 )
	{
		sphInfo ( "rt: index %s: ramchunk saved in %d.%03d sec",
			m_sIndexName.cstr(), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
	}
}

#define SPH_THRESHOLD_SAVE_RAM ( 64*1024*1024 )
static int64_t g_iRtFlushPeriod = 10*60*60; // default period is 10 hours

void RtIndex_t::CheckRamFlush ()
{
	int64_t tmSave = sphMicroTimer();

	if ( m_iTID<=m_iSavedTID || ( tmSave-m_tmSaved )/1000000<g_iRtFlushPeriod )
		return;

	m_tRwlock.ReadLock();
	int64_t iUsedRam = GetUsedRam();
	int64_t iDeltaRam = iUsedRam-m_iSavedRam;
	m_tRwlock.Unlock();

	// save if delta-ram runs over maximum value of ram-threshold or 1/3 of index size
	if ( iDeltaRam < Max ( SPH_THRESHOLD_SAVE_RAM, m_iRamSize/3 ) )
		return;

	m_tWriterMutex.Lock();

	if ( !SaveRamChunk () )
	{
		sphWarning ( "rt auto-save FAILED!!! (index='%s', error '%s')", m_sIndexName.cstr(), m_sLastError.cstr() );
		m_tWriterMutex.Unlock();
		return;
	}
	SaveMeta ( m_pDiskChunks.GetLength() );
	g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	int64_t iWasTID = m_iSavedTID;
	int64_t iWasRam = m_iSavedRam;
	int64_t tmDelta = sphMicroTimer() - m_tmSaved;
	m_iSavedTID = m_iTID;
	m_iSavedRam = iUsedRam;
	m_tmSaved = sphMicroTimer();

	m_tWriterMutex.Unlock();

	tmSave = sphMicroTimer() - tmSave;
	sphInfo ( "rt auto-saved (index='%s', last TID="INT64_FMT", current TID="INT64_FMT", last ram=%d.%03d Mb, current ram=%d.%03d Mb, time delta=%d sec, took=%d.%03d sec)"
		, m_sIndexName.cstr(), iWasTID, m_iTID, (int)(iWasRam/1024/1024), (int)((iWasRam/1024)%1000)
		, (int)(m_iSavedRam/1024/1024), (int)((m_iSavedRam/1024)%1000)
		, (int) (tmDelta/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}

int64_t RtIndex_t::GetUsedRam () const
{
	int64_t iTotal = 0;
	ARRAY_FOREACH ( i, m_pSegments )
		iTotal += m_pSegments[i]->GetUsedRam();

	return iTotal;
}

//////////////////////////////////////////////////////////////////////////
// INDEXING
//////////////////////////////////////////////////////////////////////////

class CSphSource_StringVector : public CSphSource_Document
{
public:
	explicit			CSphSource_StringVector ( int iFields, const char ** ppFields, const CSphSchema & tSchema );
	virtual				~CSphSource_StringVector () {}

	virtual bool		Connect ( CSphString & );
	virtual void		Disconnect ();

	virtual bool		HasAttrsConfigured () { return false; }
	virtual bool		IterateStart ( CSphString & ) { return true; }

	virtual bool		IterateMultivaluedStart ( int, CSphString & ) { return false; }
	virtual bool		IterateMultivaluedNext () { return false; }

	virtual bool		IterateFieldMVAStart ( int, CSphString & ) { return false; }
	virtual bool		IterateFieldMVANext () { return false; }

	virtual bool		IterateKillListStart ( CSphString & ) { return false; }
	virtual bool		IterateKillListNext ( SphDocID_t & ) { return false; }

	virtual BYTE **		NextDocument ( CSphString & ) { return m_dFields.Begin(); }

protected:
	CSphVector<BYTE *>			m_dFields;
	CSphVector<CSphWordHit>		m_dHits;
};


CSphSource_StringVector::CSphSource_StringVector ( int iFields, const char ** ppFields, const CSphSchema & tSchema )
	: CSphSource_Document ( "$stringvector" )
{
	m_tSchema = tSchema;

	m_dFields.Resize ( 1+iFields );
	for ( int i=0; i<iFields; i++ )
	{
		m_dFields[i] = (BYTE*) ppFields[i];
		assert ( m_dFields[i] );
	}
	m_dFields [ iFields ] = NULL;

	m_iMaxHits = 0; // force all hits build
}

bool CSphSource_StringVector::Connect ( CSphString & )
{
	m_tHits.m_dData.Reserve ( 1024 );
	return true;
}

void CSphSource_StringVector::Disconnect ()
{
	m_tHits.m_dData.Reset();
}

bool RtIndex_t::AddDocument ( int iFields, const char ** ppFields, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError )
{
	assert ( g_bRTChangesAllowed );

	if ( !tDoc.m_iDocID )
		return true;

	MEMORY ( SPH_MEM_IDX_RT );

	if ( !bReplace )
	{
		m_tRwlock.ReadLock ();
		ARRAY_FOREACH ( i, m_pSegments )
			if ( FindDocinfo ( m_pSegments[i], tDoc.m_iDocID )
				&& !m_pSegments[i]->m_dKlist.BinarySearch ( tDoc.m_iDocID ) )
		{
			m_tRwlock.Unlock ();
			sError.SetSprintf ( "duplicate id '"UINT64_FMT"'", (uint64_t)tDoc.m_iDocID );
			return false; // already exists and not deleted; INSERT fails
		}
		m_tRwlock.Unlock ();
	}

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( false ) ); // avoid race
	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDictBase = m_pDict;
	if ( pDictBase->HasState() )
	{
		tDictCloned = pDictBase = pDictBase->Clone();
	}
	CSphSource_StringVector tSrc ( iFields, ppFields, m_tOutboundSchema );
	tSrc.Setup ( m_tSettings );
	tSrc.SetTokenizer ( pTokenizer.Ptr() );
	tSrc.SetDict ( pDictBase );

	tSrc.m_tDocInfo.Clone ( tDoc, m_tOutboundSchema.GetRowSize() );

	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );

	return AddDocument ( pHits, tDoc, ppStr, dMvas, sError );
}


void AccumCleanup ( void * pArg )
{
	RtAccum_t * pAcc = (RtAccum_t *) pArg;
	SafeDelete ( pAcc );
}


RtAccum_t * RtIndex_t::AcquireAccum ( CSphString * sError )
{
	RtAccum_t * pAcc = NULL;

	// check that no other index is holding the acc
	pAcc = (RtAccum_t*) sphThreadGet ( g_tTlsAccumKey );
	if ( pAcc && pAcc->m_pIndex!=NULL && pAcc->m_pIndex!=this )
	{
		if ( sError )
			sError->SetSprintf ( "current txn is working with another index ('%s')", pAcc->m_pIndex->m_tSchema.m_sName.cstr() );
		return NULL;
	}

	if ( !pAcc )
	{
		pAcc = new RtAccum_t ();
		sphThreadSet ( g_tTlsAccumKey, pAcc );
		sphThreadOnExit ( AccumCleanup, pAcc );
	}

	assert ( pAcc->m_pIndex==NULL || pAcc->m_pIndex==this );
	pAcc->m_pIndex = this;
	return pAcc;
}

bool RtIndex_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError )
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = AcquireAccum ( &sError );
	if ( pAcc )
		pAcc->AddDocument ( pHits, tDoc, m_tOutboundSchema.GetRowSize(), ppStr, dMvas );

	return ( pAcc!=NULL );
}


RtAccum_t::RtAccum_t ()
	: m_pIndex ( NULL )
	, m_iAccumDocs ( 0 )
{
	m_dStrings.Add ( 0 );
	m_dMvas.Add ( 0 );
}


void RtAccum_t::AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, int iRowSize, const char ** ppStr, const CSphVector<DWORD> & dMvas )
{
	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	// schedule existing copies for deletion
	m_dAccumKlist.Add ( tDoc.m_iDocID );

	// reserve some hit space on first use
	if ( pHits && pHits->Length() && !m_dAccum.GetLength() )
		m_dAccum.Reserve ( 128*1024 );

	// accumulate row data; expect fully dynamic rows
	assert ( !tDoc.m_pStatic );
	assert (!( !tDoc.m_pDynamic && iRowSize!=0 ));
	assert (!( tDoc.m_pDynamic && (int)tDoc.m_pDynamic[-1]!=iRowSize ));

	m_dAccumRows.Resize ( m_dAccumRows.GetLength() + DOCINFO_IDSIZE + iRowSize );
	CSphRowitem * pRow = &m_dAccumRows [ m_dAccumRows.GetLength() - DOCINFO_IDSIZE - iRowSize ];
	DOCINFOSETID ( pRow, tDoc.m_iDocID );

	CSphRowitem * pAttrs = DOCINFO2ATTRS(pRow);
	for ( int i=0; i<iRowSize; i++ )
		pAttrs[i] = tDoc.m_pDynamic[i];

	int iMva = 0;

	const CSphSchema & pSchema = m_pIndex->GetInternalSchema();
	int iAttr = 0;
	for ( int i=0; i<pSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tColumn = pSchema.GetAttr(i);
		if ( tColumn.m_eAttrType==SPH_ATTR_STRING )
		{
			const char * pStr = ppStr ? ppStr[iAttr++] : NULL;
			const int iLen = pStr ? strlen ( pStr ) : 0;

			if ( iLen )
			{
				BYTE dLen[3];
				const int iLenPacked = sphPackStrlen ( dLen, iLen );
				const int iOff = m_dStrings.GetLength();
				assert ( iOff>=1 );
				m_dStrings.Resize ( iOff + iLenPacked + iLen );
				memcpy ( &m_dStrings[iOff], dLen, iLenPacked );
				memcpy ( &m_dStrings[iOff+iLenPacked], pStr, iLen );
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, iOff );
			} else
			{
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, 0 );
			}
		} else if ( tColumn.m_eAttrType==SPH_ATTR_UINT32SET )
		{
			assert ( m_dMvas.GetLength() );
			int iCount = dMvas[iMva];
			if ( iCount )
			{
				int iDst = m_dMvas.GetLength();
				m_dMvas.Resize ( iDst+iCount+1 );
				memcpy ( m_dMvas.Begin()+iDst, dMvas.Begin()+iMva, (iCount+1)*sizeof(dMvas[0]) );
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, iDst );
			} else
			{
				sphSetRowAttr ( pAttrs, tColumn.m_tLocator, 0 );
			}

			iMva += iCount+1;
		}
	}

	// accumulate hits
	int iHits = 0;
	if ( pHits && pHits->Length() )
	{
		iHits = pHits->Length();
		for ( const CSphWordHit * pHit = pHits->First(); pHit<=pHits->Last(); pHit++ )
			m_dAccum.Add ( *pHit );
	}
	m_dPerDocHitsCount.Add ( iHits );

	m_iAccumDocs++;
}


RtSegment_t * RtAccum_t::CreateSegment ( int iRowSize )
{
	if ( !m_iAccumDocs )
		return NULL;

	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	RtSegment_t * pSeg = new RtSegment_t ();

	CSphWordHit tClosingHit;
	tClosingHit.m_iWordID = WORDID_MAX;
	tClosingHit.m_iDocID = DOCID_MAX;
	tClosingHit.m_iWordPos = EMPTY_HIT;
	m_dAccum.Add ( tClosingHit );

	RtDoc_t tDoc;
	tDoc.m_uDocID = 0;
	tDoc.m_uFields = 0;
	tDoc.m_uHits = 0;
	tDoc.m_uHit = 0;

	RtWord_t tWord;
	tWord.m_uWordID = 0;
	tWord.m_uDocs = 0;
	tWord.m_uHits = 0;
	tWord.m_uDoc = 0;

	RtDocWriter_t tOutDoc ( pSeg );
	RtWordWriter_t tOutWord ( pSeg );
	RtHitWriter_t tOutHit ( pSeg );

	Hitpos_t uEmbeddedHit = EMPTY_HIT;
	ARRAY_FOREACH ( i, m_dAccum )
	{
		const CSphWordHit & tHit = m_dAccum[i];

		// new keyword or doc; flush current doc
		if ( tHit.m_iWordID!=tWord.m_uWordID || tHit.m_iDocID!=tDoc.m_uDocID )
		{
			if ( tDoc.m_uDocID )
			{
				tWord.m_uDocs++;
				tWord.m_uHits += tDoc.m_uHits;

				if ( uEmbeddedHit )
				{
					assert ( tDoc.m_uHits==1 );
					tDoc.m_uHit = uEmbeddedHit;
				}

				tOutDoc.ZipDoc ( tDoc );
				tDoc.m_uFields = 0;
				tDoc.m_uHits = 0;
				tDoc.m_uHit = tOutHit.ZipHitPtr();
			}

			tDoc.m_uDocID = tHit.m_iDocID;
			tOutHit.ZipRestart ();
			uEmbeddedHit = 0;
		}

		// new keyword; flush current keyword
		if ( tHit.m_iWordID!=tWord.m_uWordID )
		{
			tOutDoc.ZipRestart ();
			if ( tWord.m_uWordID )
				tOutWord.ZipWord ( tWord );

			tWord.m_uWordID = tHit.m_iWordID;
			tWord.m_uDocs = 0;
			tWord.m_uHits = 0;
			tWord.m_uDoc = tOutDoc.ZipDocPtr();
		}

		// just a new hit
		if ( !tDoc.m_uHits )
		{
			uEmbeddedHit = tHit.m_iWordPos;
		} else
		{
			if ( uEmbeddedHit )
			{
				tOutHit.ZipHit ( uEmbeddedHit );
				uEmbeddedHit = 0;
			}

			tOutHit.ZipHit ( tHit.m_iWordPos );
		}

		tDoc.m_uFields |= 1UL << HITMAN::GetField ( tHit.m_iWordPos );
		tDoc.m_uHits++;
	}

	pSeg->m_iRows = m_iAccumDocs;
	pSeg->m_iAliveRows = m_iAccumDocs;

	// copy and sort attributes
	int iStride = DOCINFO_IDSIZE + iRowSize;
	pSeg->m_dRows.SwapData ( m_dAccumRows );
	pSeg->m_dStrings.SwapData ( m_dStrings );
	pSeg->m_dMvas.SwapData ( m_dMvas );
	sphSortDocinfos ( pSeg->m_dRows.Begin(), pSeg->m_dRows.GetLength()/iStride, iStride );

	// done
	return pSeg;
}


struct AccumDocHits_t
{
	SphDocID_t m_uDocid;
	int m_iDocIndex;
	int m_iHitIndex;
	int m_iHitCount;
};


struct CmpDocHitIndex_t
{
	inline bool IsLess ( const AccumDocHits_t & a, const AccumDocHits_t & b ) const
	{
		return ( a.m_uDocid<b.m_uDocid || ( a.m_uDocid==b.m_uDocid && a.m_iDocIndex<b.m_iDocIndex ) );
	}
};


void RtAccum_t::CleanupDuplacates ( int iRowSize )
{
	if ( m_iAccumDocs<=1 )
		return;

	assert ( m_iAccumDocs==m_dPerDocHitsCount.GetLength() );
	CSphVector<AccumDocHits_t> dDocHits ( m_dPerDocHitsCount.GetLength() );
	int iStride = DOCINFO_IDSIZE + iRowSize;

	int iHitIndex = 0;
	CSphRowitem * pRow = m_dAccumRows.Begin();
	for ( int i=0; i<m_iAccumDocs; i++, pRow+=iStride )
	{
		AccumDocHits_t & tElem = dDocHits[i];
		tElem.m_uDocid = DOCINFO2ID ( pRow );
		tElem.m_iDocIndex = i;
		tElem.m_iHitIndex = iHitIndex;
		tElem.m_iHitCount = m_dPerDocHitsCount[i];
		iHitIndex += m_dPerDocHitsCount[i];
	}

	dDocHits.Sort ( CmpDocHitIndex_t() );

	bool bHasDups = false;;
	for ( int i=0; i<dDocHits.GetLength()-1 && !bHasDups; i++ )
		bHasDups = ( dDocHits[i].m_uDocid==dDocHits[i+1].m_uDocid );

	if ( !bHasDups )
		return;

	// filter out unique - keep duplicates, but not last one
	int iDst = 0;
	int iSrc = 1;
	while ( iSrc<dDocHits.GetLength() )
	{
		bool bDup = ( dDocHits[iDst].m_uDocid==dDocHits[iSrc].m_uDocid );
		iDst += bDup;
		dDocHits[iDst] = dDocHits[iSrc++];
	}
	dDocHits.Resize ( iDst );
	assert ( dDocHits.GetLength() );

	// sort by hit index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iHitIndex ) );

	// clean up hits of duplicates
	for ( int iHit = dDocHits.GetLength()-1; iHit>=0; iHit-- )
	{
		if ( !dDocHits[iHit].m_iHitCount )
			continue;

		int iFrom = dDocHits[iHit].m_iHitIndex;
		int iCount = dDocHits[iHit].m_iHitCount;
		if ( iFrom+iCount<m_dAccum.GetLength() )
		{
			for ( int iDst=iFrom, iSrc=iFrom+iCount; iSrc<m_dAccum.GetLength(); iSrc++, iDst++ )
				m_dAccum[iDst] = m_dAccum[iSrc];
		}
		m_dAccum.Resize ( m_dAccum.GetLength()-iCount );
	}

	// sort by docid index
	dDocHits.Sort ( bind ( &AccumDocHits_t::m_iDocIndex ) );

	// clean up docinfos of duplicates
	for ( int iDoc = dDocHits.GetLength()-1; iDoc>=0; iDoc-- )
	{
		int iDst = dDocHits[iDoc].m_iDocIndex*iStride;
		int iSrc = iDst+iStride;
		while ( iSrc<m_dAccumRows.GetLength() )
		{
			m_dAccumRows[iDst++] = m_dAccumRows[iSrc++];
		}
		m_iAccumDocs--;
		m_dAccumRows.Resize ( m_iAccumDocs*iStride );
	}
}


const RtWord_t * RtIndex_t::CopyWord ( RtSegment_t * pDst, RtWordWriter_t & tOutWord, const RtSegment_t * pSrc, const RtWord_t * pWord, RtWordReader_t & tInWord, const CSphVector<SphDocID_t> * pAccKlist )
{
	RtDocReader_t tInDoc ( pSrc, *pWord );
	RtDocWriter_t tOutDoc ( pDst );

	RtWord_t tNewWord = *pWord;
	tNewWord.m_uDoc = tOutDoc.ZipDocPtr();

	// if flag is there, acc must be there
	// however, NOT vice versa (newly created segments are unaffected by TLS klist)
	assert (!( pSrc->m_bTlsKlist && !pAccKlist ));
#if 0
	// index *must* be holding acc during merge
	assert ( !pAcc || pAcc->m_pIndex==this );
#endif

	// copy docs
	for ( ;; )
	{
		const RtDoc_t * pDoc = tInDoc.UnzipDoc();
		if ( !pDoc )
			break;

		// apply klist
		bool bKill = ( pSrc->m_dKlist.BinarySearch ( pDoc->m_uDocID )!=NULL );
		if ( !bKill && pSrc->m_bTlsKlist )
			bKill = ( pAccKlist->BinarySearch ( pDoc->m_uDocID )!=NULL );

		if ( bKill )
		{
			tNewWord.m_uDocs--;
			tNewWord.m_uHits -= pDoc->m_uHits;
			continue;
		}

		// short route, single embedded hit
		if ( pDoc->m_uHits==1 )
		{
			tOutDoc.ZipDoc ( *pDoc );
			continue;
		}

		// long route, copy hits
		RtHitWriter_t tOutHit ( pDst );
		RtHitReader_t tInHit ( pSrc, pDoc );

		RtDoc_t tDoc = *pDoc;
		tDoc.m_uHit = tOutHit.ZipHitPtr();

		// OPTIMIZE? decode+memcpy?
		for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
			tOutHit.ZipHit ( uValue );

		// copy doc
		tOutDoc.ZipDoc ( tDoc );
	}

	// append word to the dictionary
	if ( tNewWord.m_uDocs )
		tOutWord.ZipWord ( tNewWord );

	// move forward
	return tInWord.UnzipWord ();
}


void RtIndex_t::CopyDoc ( RtSegment_t * pSeg, RtDocWriter_t & tOutDoc, RtWord_t * pWord, const RtSegment_t * pSrc, const RtDoc_t * pDoc )
{
	pWord->m_uDocs++;
	pWord->m_uHits += pDoc->m_uHits;

	if ( pDoc->m_uHits==1 )
	{
		tOutDoc.ZipDoc ( *pDoc );
		return;
	}

	RtHitWriter_t tOutHit ( pSeg );
	RtHitReader_t tInHit ( pSrc, pDoc );

	RtDoc_t tDoc = *pDoc;
	tDoc.m_uHit = tOutHit.ZipHitPtr();
	tOutDoc.ZipDoc ( tDoc );

	// OPTIMIZE? decode+memcpy?
	for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
		tOutHit.ZipHit ( uValue );
}


void RtIndex_t::MergeWord ( RtSegment_t * pSeg, const RtSegment_t * pSrc1, const RtWord_t * pWord1, const RtSegment_t * pSrc2, const RtWord_t * pWord2, RtWordWriter_t & tOut, const CSphVector<SphDocID_t> * pAccKlist )
{
	assert ( pWord1->m_uWordID==pWord2->m_uWordID );

	RtDocWriter_t tOutDoc ( pSeg );

	RtWord_t tWord;
	tWord.m_uWordID = pWord1->m_uWordID;
	tWord.m_uDocs = 0;
	tWord.m_uHits = 0;
	tWord.m_uDoc = tOutDoc.ZipDocPtr();

	RtDocReader_t tIn1 ( pSrc1, *pWord1 );
	RtDocReader_t tIn2 ( pSrc2, *pWord2 );
	const RtDoc_t * pDoc1 = tIn1.UnzipDoc();
	const RtDoc_t * pDoc2 = tIn2.UnzipDoc();

	while ( pDoc1 || pDoc2 )
	{
		if ( pDoc1 && pDoc2 && pDoc1->m_uDocID==pDoc2->m_uDocID )
		{
			// dupe, must (!) be killed in the first segment, might be in both
#if 0
			assert ( pSrc1->m_dKlist.BinarySearch ( pDoc1->m_uDocID )
				|| ( pSrc1->m_bTlsKlist && pAcc && pAcc->m_dAccumKlist.BinarySearch ( pDoc1->m_uDocID ) ) );
#endif
			if ( !pSrc2->m_dKlist.BinarySearch ( pDoc2->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pSrc2->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc2->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc2, pDoc2 );
			pDoc1 = tIn1.UnzipDoc();
			pDoc2 = tIn2.UnzipDoc();

		} else if ( pDoc1 && ( !pDoc2 || pDoc1->m_uDocID < pDoc2->m_uDocID ) )
		{
			// winner from the first segment
			if ( !pSrc1->m_dKlist.BinarySearch ( pDoc1->m_uDocID )
				&& ( !pSrc1->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc1->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc1, pDoc1 );
			pDoc1 = tIn1.UnzipDoc();

		} else
		{
			// winner from the second segment
			assert ( pDoc2 && ( !pDoc1 || pDoc2->m_uDocID < pDoc1->m_uDocID ) );
			if ( !pSrc2->m_dKlist.BinarySearch ( pDoc2->m_uDocID )
				&& ( !pSrc2->m_bTlsKlist || !pAccKlist->BinarySearch ( pDoc2->m_uDocID ) ) )
				CopyDoc ( pSeg, tOutDoc, &tWord, pSrc2, pDoc2 );
			pDoc2 = tIn2.UnzipDoc();
		}
	}

	if ( tWord.m_uDocs )
		tOut.ZipWord ( tWord );
}


#if PARANOID
static void CheckSegmentRows ( const RtSegment_t * pSeg, int iStride )
{
	const CSphVector<CSphRowitem> & dRows = pSeg->m_dRows; // shortcut
	for ( int i=iStride; i<dRows.GetLength(); i+=iStride )
		assert ( DOCINFO2ID ( &dRows[i] ) > DOCINFO2ID ( &dRows[i-iStride] ) );
}
#endif

template < typename DOCID = SphDocID_t >
struct RtRowIterator_tmpl : public ISphNoncopyable
{
protected:
	const CSphRowitem * m_pRow;
	const CSphRowitem * m_pRowMax;
	const DOCID * m_pKlist;
	const DOCID * m_pKlistMax;
	const DOCID * m_pTlsKlist;
	const DOCID * m_pTlsKlistMax;
	const int m_iStride;

public:
	explicit RtRowIterator_tmpl ( const RtSegment_t * pSeg, int iStride, bool bWriter, const CSphVector<DOCID> * pAccKlist )
		: m_pRow ( pSeg->m_dRows.Begin() )
		, m_pRowMax ( pSeg->m_dRows.Begin() + pSeg->m_dRows.GetLength() )
		, m_pKlist ( NULL )
		, m_pKlistMax ( NULL )
		, m_pTlsKlist ( NULL )
		, m_pTlsKlistMax ( NULL )
		, m_iStride ( iStride )
	{
		if ( pSeg->m_dKlist.GetLength() )
		{
			m_pKlist = ( const DOCID* ) pSeg->m_dKlist.Begin();
			m_pKlistMax = m_pKlist + pSeg->m_dKlist.GetLength();
		}

		// FIXME? OPTIMIZE? must not scan tls (open txn) in readers; can implement lighter iterator
		// FIXME? OPTIMIZE? maybe we should just rely on the segment order and don't scan tls klist here
		if ( bWriter && pSeg->m_bTlsKlist && pAccKlist && pAccKlist->GetLength() )
		{
			m_pTlsKlist = pAccKlist->Begin();
			m_pTlsKlistMax = m_pTlsKlist + pAccKlist->GetLength();
		}
	}

	const CSphRowitem * GetNextAliveRow ()
	{
		// while there are rows and k-list entries
		while ( m_pRow<m_pRowMax && ( m_pKlist<m_pKlistMax || m_pTlsKlist<m_pTlsKlistMax ) )
		{
			// get next candidate id
			DOCID uID = DOCINFO2ID_T<DOCID>(m_pRow);

			// check if segment k-list kills it
			while ( m_pKlist<m_pKlistMax && *m_pKlist<uID )
				m_pKlist++;

			if ( m_pKlist<m_pKlistMax && *m_pKlist==uID )
			{
				m_pKlist++;
				m_pRow += m_iStride;
				continue;
			}

			// check if txn k-list kills it
			while ( m_pTlsKlist<m_pTlsKlistMax && *m_pTlsKlist<uID )
				m_pTlsKlist++;

			if ( m_pTlsKlist<m_pTlsKlistMax && *m_pTlsKlist==uID )
			{
				m_pTlsKlist++;
				m_pRow += m_iStride;
				continue;
			}

			// oh, so nobody kills it
			break;
		}

		// oops, out of rows
		if ( m_pRow>=m_pRowMax )
			return NULL;

		// got it, and it's alive!
		m_pRow += m_iStride;
		return m_pRow-m_iStride;
	}
};

typedef RtRowIterator_tmpl<> RtRowIterator_t;

#ifdef PARANOID // sanity check in PARANOID mode
template <typename DOCID>
void VerifyEmptyStrings ( const CSphTightVector<BYTE> & dStorage, const CSphSchema & tSchema, const CSphRowitem * pRow )
{
	if ( dStorage.GetLength()>1 )
		return;

	const DWORD * pAttr = DOCINFO2ATTRS_T<DOCID>(pRow);
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		assert ( tCol.m_eAttrType!=SPH_ATTR_STRING
		|| ( tCol.m_eAttrType==SPH_ATTR_STRING && sphGetRowAttr ( pAttr, tCol.m_tLocator )==0 ) );
	}
}
#endif

static DWORD CopyPackedString ( const BYTE * pSrc, CSphTightVector<BYTE> & dDst )
{
	assert ( pSrc );
	assert ( dDst.GetLength()>=1 );
	const BYTE * pStr = NULL;
	const int iLen = sphUnpackStr ( pSrc, &pStr );
	assert ( iLen>0 );
	assert ( pStr );

	const DWORD uOff = dDst.GetLength();
	const DWORD uWriteLen = iLen + ( pStr - pSrc ); // actual length = strings content length + packed length of string
	dDst.Resize ( uOff + uWriteLen );
	memcpy ( dDst.Begin() + uOff, pSrc, uWriteLen );
	return uOff;
}

static DWORD CopyMva ( const DWORD * pSrc, CSphTightVector<DWORD> & dDst )
{
	assert ( pSrc );
	assert ( dDst.GetLength()>=1 );

	DWORD uCount = *pSrc;
	assert ( uCount );

	DWORD iLen = dDst.GetLength();
	dDst.Resize ( iLen+uCount+1 );
	memcpy ( dDst.Begin()+iLen, pSrc, ( uCount+1 )*sizeof(DWORD) );
	return iLen;
}


void CopyDocid ( SphDocID_t uDocid, CSphTightVector<DWORD> & dDst )
{
	int iLen = dDst.GetLength();
	dDst.Resize ( iLen + sizeof(uDocid) );
	DOCINFOSETID ( dDst.Begin()+iLen, uDocid );
}


static void ExtractLocators ( const CSphSchema & tSchema, ESphAttr eAttrType, CSphVector<CSphAttrLocator> & dLocators )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tColumn = tSchema.GetAttr(i);
		if ( tColumn.m_eAttrType==eAttrType )
			dLocators.Add ( tColumn.m_tLocator );
	}
}


class StorageStringWriter_t : ISphNoncopyable
{
private:
	CSphWriter &					m_tDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageStringWriter_t ( const CSphSchema & tSchema, CSphWriter & tDst )
		: m_tDst ( tDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_STRING, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }
	void SetDocid ( SphDocID_t ) {}

	DWORD CopyAttr ( const BYTE * pSrc )
	{
		assert ( m_tDst.GetPos()>0 && m_tDst.GetPos()<( I64C(1)<<32 ) ); // should be 32 bit offset

		const BYTE * pStr = NULL;
		const int iLen = sphUnpackStr ( pSrc, &pStr );
		assert ( iLen && pStr );

		DWORD uAttr = (DWORD)m_tDst.GetPos();
		const int iWriteLen = iLen + ( pStr - pSrc );
		m_tDst.PutBytes ( pSrc, iWriteLen );
		return uAttr;
	}
};


class StorageStringVector_t : ISphNoncopyable
{
private:
	CSphTightVector<BYTE> &			m_dDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageStringVector_t ( const CSphSchema & tSchema, CSphTightVector<BYTE> & dDst )
		: m_dDst ( dDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_STRING, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }
	void SetDocid ( SphDocID_t ) {}

	DWORD CopyAttr ( const BYTE * pSrc )
	{
		assert ( m_dDst.GetLength()>0 && m_dDst.GetLength()<( I64C(1)<<32 ) ); // should be 32 bit offset
		return CopyPackedString ( pSrc, m_dDst );
	}
};


class StorageMvaWriter_t : ISphNoncopyable
{
private:
	CSphWriter &					m_tDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageMvaWriter_t ( const CSphSchema & tSchema, CSphWriter & tDst )
		: m_tDst ( tDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_UINT32SET, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }

	void SetDocid ( SphDocID_t uDocid )
	{
		m_tDst.PutDocid ( uDocid );
	}

	DWORD CopyAttr ( const DWORD * pSrc )
	{
		assert ( m_tDst.GetPos()>0 && m_tDst.GetPos()<( I64C(1)<<32 ) ); // should be 32 bit offset

		DWORD uCount = *pSrc;
		assert ( uCount );

		SphOffset_t uOff = m_tDst.GetPos();
		assert ( ( uOff%sizeof(DWORD) )==0 );
		m_tDst.PutBytes ( pSrc, ( uCount+1 )*sizeof(DWORD) );

		return MVA_DOWNSIZE ( uOff/sizeof(DWORD) );
	}
};


class StorageMvaVector_t : ISphNoncopyable
{
private:
	CSphTightVector<DWORD> &		m_dDst;
	CSphVector<CSphAttrLocator>		m_dLocators;

public:
	explicit StorageMvaVector_t ( const CSphSchema & tSchema, CSphTightVector<DWORD> & dDst )
		: m_dDst ( dDst )
	{
		ExtractLocators ( tSchema, SPH_ATTR_UINT32SET, m_dLocators );
	}
	const CSphVector<CSphAttrLocator> & GetLocators () const { return m_dLocators; }

	void SetDocid ( SphDocID_t uDocid )
	{
		CopyDocid ( uDocid, m_dDst );
	}

	DWORD CopyAttr ( const DWORD * pSrc )
	{
		assert ( m_dDst.GetLength()>0 && m_dDst.GetLength()<( I64C(1)<<32 ) ); // should be 32 bit offset
		return CopyMva ( pSrc, m_dDst );
	}
};


template <typename DOCID, typename STORAGE, typename SRC>
void CopyFixupStorageAttrs ( const CSphTightVector<SRC> & dSrc, STORAGE & tStorage, CSphRowitem * pRow )
{
	const CSphVector<CSphAttrLocator> & dLocators = tStorage.GetLocators();
	if ( !dLocators.GetLength() )
		return;

	// store string\mva attr for this row
	SphDocID_t uDocid = DOCINFO2ID ( pRow );
	DWORD * pAttr = DOCINFO2ATTRS_T<DOCID>( pRow );
	bool bIdSet = false;
	ARRAY_FOREACH ( i, dLocators )
	{
		const SphAttr_t uOff = sphGetRowAttr ( pAttr, dLocators[i] );
		if ( !uOff )
			continue;

		assert ( uOff && uOff<dSrc.GetLength() );

		if ( !bIdSet )
		{
			tStorage.SetDocid ( uDocid );
			bIdSet = true;
		}

		DWORD uAttr = tStorage.CopyAttr ( dSrc.Begin() + uOff );

		sphSetRowAttr ( pAttr, dLocators[i], uAttr );
	}
}


RtSegment_t * RtIndex_t::MergeSegments ( const RtSegment_t * pSeg1, const RtSegment_t * pSeg2, const CSphVector<SphDocID_t> * pAccKlist )
{
	if ( pSeg1->m_iTag > pSeg2->m_iTag )
		Swap ( pSeg1, pSeg2 );

	RtSegment_t * pSeg = new RtSegment_t ();

	////////////////////
	// merge attributes
	////////////////////

	// check that all the IDs are in proper asc order
#if PARANOID
	CheckSegmentRows ( pSeg1, m_iStride );
	CheckSegmentRows ( pSeg2, m_iStride );
#endif

	// just a shortcut
	CSphVector<CSphRowitem> & dRows = pSeg->m_dRows;
	CSphTightVector<BYTE> & dStrings = pSeg->m_dStrings;
	CSphTightVector<DWORD> & dMvas = pSeg->m_dMvas;

	// we might need less because of dupes, but we can not know yet
	dRows.Reserve ( pSeg1->m_dRows.GetLength() + pSeg2->m_dRows.GetLength() );

	// as each segment has dummy zero we reserve less
	assert ( pSeg1->m_dStrings.GetLength() + pSeg2->m_dStrings.GetLength()>=2 );
	dStrings.Reserve ( pSeg1->m_dStrings.GetLength() + pSeg2->m_dStrings.GetLength() - 2 );
	assert ( pSeg1->m_dMvas.GetLength() + pSeg2->m_dMvas.GetLength()>=2 );
	dMvas.Reserve ( pSeg1->m_dMvas.GetLength() + pSeg2->m_dMvas.GetLength() - 2 );

	StorageStringVector_t tStorageString ( m_tSchema, dStrings );
	StorageMvaVector_t tStorageMva ( m_tSchema, dMvas );

	RtRowIterator_t tIt1 ( pSeg1, m_iStride, true, pAccKlist );
	RtRowIterator_t tIt2 ( pSeg2, m_iStride, true, pAccKlist );

	const CSphRowitem * pRow1 = tIt1.GetNextAliveRow();
	const CSphRowitem * pRow2 = tIt2.GetNextAliveRow();

	while ( pRow1 || pRow2 )
	{
		if ( !pRow2 || ( pRow1 && pRow2 && DOCINFO2ID(pRow1)<DOCINFO2ID(pRow2) ) )
		{
			assert ( pRow1 );
			for ( int i=0; i<m_iStride; i++ )
				dRows.Add ( *pRow1++ );
			CSphRowitem * pDstRow = dRows.Begin() + dRows.GetLength() - m_iStride;
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg1->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg1->m_dMvas, tStorageMva, pDstRow );
			pRow1 = tIt1.GetNextAliveRow();
		} else
		{
			assert ( pRow2 );
			assert ( !pRow1 || ( DOCINFO2ID(pRow1)!=DOCINFO2ID(pRow2) ) ); // all dupes must be killed and skipped by the iterator
			for ( int i=0; i<m_iStride; i++ )
				dRows.Add ( *pRow2++ );
			CSphRowitem * pDstRow = dRows.Begin() + dRows.GetLength() - m_iStride;
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg2->m_dStrings, tStorageString, pDstRow );
			CopyFixupStorageAttrs<SphDocID_t> ( pSeg2->m_dMvas, tStorageMva, pDstRow );
			pRow2 = tIt2.GetNextAliveRow();
		}
		pSeg->m_iRows++;
		pSeg->m_iAliveRows++;
	}

	assert ( pSeg->m_iRows*m_iStride==pSeg->m_dRows.GetLength() );
#if PARANOID
	CheckSegmentRows ( pSeg, m_iStride );
#endif

	//////////////////
	// merge keywords
	//////////////////

	pSeg->m_dWords.Reserve ( pSeg1->m_dWords.GetLength() + pSeg2->m_dWords.GetLength() );
	pSeg->m_dDocs.Reserve ( pSeg1->m_dDocs.GetLength() + pSeg2->m_dDocs.GetLength() );
	pSeg->m_dHits.Reserve ( pSeg1->m_dHits.GetLength() + pSeg2->m_dHits.GetLength() );

	RtWordWriter_t tOut ( pSeg );
	RtWordReader_t tIn1 ( pSeg1 );
	RtWordReader_t tIn2 ( pSeg2 );
	const RtWord_t * pWords1 = tIn1.UnzipWord ();
	const RtWord_t * pWords2 = tIn2.UnzipWord ();

	// merge while there are common words
	for ( ;; )
	{
		while ( pWords1 && pWords2 && pWords1->m_uWordID!=pWords2->m_uWordID )
			if ( pWords1->m_uWordID < pWords2->m_uWordID )
				pWords1 = CopyWord ( pSeg, tOut, pSeg1, pWords1, tIn1, pAccKlist );
			else
				pWords2 = CopyWord ( pSeg, tOut, pSeg2, pWords2, tIn2, pAccKlist );

		if ( !pWords1 || !pWords2 )
			break;

		assert ( pWords1 && pWords2 && pWords1->m_uWordID==pWords2->m_uWordID );
		MergeWord ( pSeg, pSeg1, pWords1, pSeg2, pWords2, tOut, pAccKlist );
		pWords1 = tIn1.UnzipWord();
		pWords2 = tIn2.UnzipWord();
	}

	// copy tails
	while ( pWords1 ) pWords1 = CopyWord ( pSeg, tOut, pSeg1, pWords1, tIn1, pAccKlist );
	while ( pWords2 ) pWords2 = CopyWord ( pSeg, tOut, pSeg2, pWords2, tIn2, pAccKlist );

	assert ( pSeg->m_dRows.GetLength() );
	assert ( pSeg->m_iRows );
	assert ( pSeg->m_iAliveRows==pSeg->m_iRows );
	return pSeg;
}


struct CmpSegments_fn
{
	inline bool IsLess ( const RtSegment_t * a, const RtSegment_t * b )
	{
		return a->GetMergeFactor() > b->GetMergeFactor();
	}
};

void RtIndex_t::Commit ()
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( SPH_MEM_IDX_RT );

	RtAccum_t * pAcc = AcquireAccum();
	if ( !pAcc )
		return;

	// empty txn, just ignore
	if ( !pAcc->m_iAccumDocs && !pAcc->m_dAccumKlist.GetLength() )
	{
		pAcc->m_pIndex = NULL;
		pAcc->m_iAccumDocs = 0;
		pAcc->m_dAccumRows.Resize ( 0 );
		pAcc->m_dStrings.Resize ( 1 );
		pAcc->m_dPerDocHitsCount.Resize ( 0 );
		return;
	}

	// phase 0, build a new segment
	// accum and segment are thread local; so no locking needed yet
	// segment might be NULL if we're only killing rows this txn
	pAcc->CleanupDuplacates ( m_tOutboundSchema.GetRowSize() );
	pAcc->m_dAccum.Sort ( CmpHit_fn() );

	RtSegment_t * pNewSeg = pAcc->CreateSegment ( m_tOutboundSchema.GetRowSize() );
	assert ( !pNewSeg || pNewSeg->m_iRows>0 );
	assert ( !pNewSeg || pNewSeg->m_iAliveRows>0 );
	assert ( !pNewSeg || pNewSeg->m_bTlsKlist==false );

#if PARANOID
	if ( pNewSeg )
		CheckSegmentRows ( pNewSeg, m_iStride );
#endif

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );
	pAcc->m_dStrings.Resize ( 1 ); // handle dummy zero offset
	pAcc->m_dPerDocHitsCount.Resize ( 0 );

	// sort accum klist, too
	pAcc->m_dAccumKlist.Uniq ();

	// now on to the stuff that needs locking and recovery
	CommitReplayable ( pNewSeg, pAcc->m_dAccumKlist );

	// done; cleanup accum
	pAcc->m_pIndex = NULL;
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
}

void RtIndex_t::CommitReplayable ( RtSegment_t * pNewSeg, CSphVector<SphDocID_t> & dAccKlist )
{
	int iNewDocs = pNewSeg ? pNewSeg->m_iRows : 0;

	// phase 1, lock out other writers (but not readers yet)
	// concurrent readers are ok during merges, as existing segments won't be modified yet
	// however, concurrent writers are not
	Verify ( m_tWriterMutex.Lock() );

	// first of all, binlog txn data for recovery
	g_pBinlog->BinlogCommit ( m_sIndexName.cstr(), ++m_iTID, pNewSeg, dAccKlist );

	// let merger know that existing segments are subject to additional, TLS K-list filter
	// safe despite the readers, flag must only be used by writer
	if ( dAccKlist.GetLength() )
		ARRAY_FOREACH ( i, m_pSegments )
	{
		// OPTIMIZE? only need to set the flag if TLS K-list *actually* affects segment
		assert ( m_pSegments[i]->m_bTlsKlist==false );
		m_pSegments[i]->m_bTlsKlist = true;
	}

	// prepare new segments vector
	// create more new segments by merging as needed
	// do not (!) kill processed old segments just yet, as readers might still need them
	CSphVector<RtSegment_t*> dSegments;
	CSphVector<RtSegment_t*> dToKill;

	dSegments = m_pSegments;
	if ( pNewSeg )
		dSegments.Add ( pNewSeg );

	int64_t iRamFreed = 0;

	// enforce RAM usage limit
	int64_t iRamLeft = m_iRamSize;
	ARRAY_FOREACH ( i, dSegments )
		iRamLeft = Max ( 0, iRamLeft - dSegments[i]->GetUsedRam() );

	// skip merging if no rows were added or no memory left
	bool bDump = ( iRamLeft==0 );
	const int MAX_SEGMENTS = 32;
	const int MAX_PROGRESSION_SEGMENT = 8;
	while ( pNewSeg && iRamLeft>0 )
	{
		dSegments.Sort ( CmpSegments_fn() );

		// unconditionally merge if there's too much segments now
		// conditionally merge if smallest segment has grown too large
		// otherwise, we're done
		const int iLen = dSegments.GetLength();
		if ( iLen < ( MAX_SEGMENTS - MAX_PROGRESSION_SEGMENT ) )
			break;
		assert ( iLen>=2 );
		// exit if progression is kept AND lesser MAX_SEGMENTS limit
		if ( dSegments[iLen-2]->GetMergeFactor() > dSegments[iLen-1]->GetMergeFactor()*2 && iLen < MAX_SEGMENTS )
			break;

		// check whether we have enough RAM
#define LOC_ESTIMATE1(_seg,_vec) \
	(int)( ( (int64_t)_seg->_vec.GetLength() ) * _seg->m_iAliveRows / _seg->m_iRows )

#define LOC_ESTIMATE(_vec) \
	( LOC_ESTIMATE1 ( dSegments[iLen-1], _vec ) + LOC_ESTIMATE1 ( dSegments[iLen-2], _vec ) )

		int64_t iEstimate =
			CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dWords ) ) +
			CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dDocs ) ) +
			CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dHits ) ) +
			CSphTightVectorPolicy<BYTE>::Relimit ( 0, LOC_ESTIMATE ( m_dStrings ) );

#undef LOC_ESTIMATE
#undef LOC_ESTIMATE1

		if ( iEstimate>iRamLeft )
		{
			// dump case: can't merge any more AND segments count limit's reached
			bDump = ( ( iRamLeft + iRamFreed )<=iEstimate ) && ( iLen>=MAX_SEGMENTS );
			break;
		}

		// do it
		RtSegment_t * pA = dSegments.Pop();
		RtSegment_t * pB = dSegments.Pop();
		dSegments.Add ( MergeSegments ( pA, pB, &dAccKlist ) );
		dToKill.Add ( pA );
		dToKill.Add ( pB );

		iRamFreed += pA->GetUsedRam() + pB->GetUsedRam();

		int64_t iMerged = dSegments.Last()->GetUsedRam();
		iRamLeft -= Min ( iRamLeft, iMerged );
	}

	// phase 2, obtain exclusive writer lock
	// we now have to update K-lists in (some of) the survived segments
	// and also swap in new segment list
	m_tRwlock.WriteLock ();

	// adjust for an incoming accumulator K-list
	int iTotalKilled = 0;
	if ( dAccKlist.GetLength() )
	{
#ifndef NDEBUG
#if PARANOID
		// check that klist is sorted and unique
		for ( int i=1; i<dAccKlist.GetLength(); i++ )
			assert ( dAccKlist[i-1] < dAccKlist[i] );
#endif
#endif

		// update totals
		// work the original (!) segments, and before (!) updating their K-lists
		int iDiskLiveKLen = dAccKlist.GetLength();
		for ( int i=0; i<iDiskLiveKLen; i++ )
		{
			const SphDocID_t uDocid = dAccKlist[i];

			// check RAM chunk
			bool bRamKilled = false;
			for ( int j=0; j<m_pSegments.GetLength() && !bRamKilled; j++ )
				bRamKilled = ( m_pSegments[j]->FindAliveRow ( uDocid )!=NULL );

			bool bDiskKilled = m_tKlist.Exists ( uDocid );

			// check disk chunks
			bool bKeep = false;
			if ( !bRamKilled || !bDiskKilled )
			{
				for ( int j=m_pDiskChunks.GetLength()-1; j>=0 && !bKeep; j-- )
				{
					if ( m_pDiskChunks[j]->HasDocid ( uDocid ) )
					{
						// we just found the most recent chunk with our suspect docid
						// let's check whether it's already killed by subsequent chunks, or gets killed now
						SphAttr_t uRef = uDocid;
						bKeep = true;
						for ( int k=j+1; k<m_pDiskChunks.GetLength() && bKeep; k++ )
						{
							const CSphIndex * pIndex = m_pDiskChunks[k];
							bKeep &= ( sphBinarySearch ( pIndex->GetKillList(), pIndex->GetKillList() + pIndex->GetKillListSize() - 1, uRef )==NULL );
						}
					}
				}
			}

			if ( bRamKilled || bKeep )
				iTotalKilled++;

			if ( bDiskKilled || !bKeep )
			{
				Swap ( dAccKlist[i], dAccKlist[iDiskLiveKLen-1] );
				iDiskLiveKLen--;
				i--;
			}
		}

		// update K-lists on survivors
		ARRAY_FOREACH ( iSeg, dSegments )
		{
			RtSegment_t * pSeg = dSegments[iSeg];
			if ( !pSeg->m_bTlsKlist )
				continue; // should be fresh enough

			bool bKlistChanged = false;

			// this segment was not created by this txn
			// so we need to merge additional K-list from current txn into it
			ARRAY_FOREACH ( j, dAccKlist )
			{
				SphDocID_t uDocid = dAccKlist[j];
				if ( pSeg->FindAliveRow ( uDocid ) )
				{
					pSeg->m_dKlist.Add ( uDocid );
					pSeg->m_iAliveRows--;
					assert ( pSeg->m_iAliveRows>=0 );
					bKlistChanged = true;
				}
			}

			// we did not check for existence in K-list, only in segment
			// so need to use Uniq(), not just Sort()
			if ( bKlistChanged )
				pSeg->m_dKlist.Uniq ();

			// mark as good
			pSeg->m_bTlsKlist = false;
		}

		// update disk K-list
		// after iDiskLiveKLen are ids already stored on disk - just skip them
		for ( int i=0; i<iDiskLiveKLen; i++ )
			m_tKlist.Delete ( dAccKlist[i] );
	}

	ARRAY_FOREACH ( i, dSegments )
	{
		RtSegment_t * pSeg = dSegments[i];
		if ( pSeg->m_iAliveRows==0 )
		{
			dToKill.Add ( pSeg );
			dSegments.RemoveFast ( i );
			i--;
		}
	}

	// go live!
	Swap ( m_pSegments, dSegments );

	// we can kill retired segments now
	ARRAY_FOREACH ( i, dToKill )
		SafeDelete ( dToKill[i] );

	// update stats
	m_tStats.m_iTotalDocuments += iNewDocs - iTotalKilled;

	// phase 3, enable readers again
	// we might need to dump data to disk now
	// but during the dump, readers can still use RAM chunk data
	Verify ( m_tRwlock.Unlock() );

	if ( bDump )
	{
		SaveDiskChunk();
		g_pBinlog->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );
	}

	// all done, enable other writers
	Verify ( m_tWriterMutex.Unlock() );
}

void RtIndex_t::RollBack ()
{
	assert ( g_bRTChangesAllowed );

	RtAccum_t * pAcc = AcquireAccum();
	if ( !pAcc )
		return;

	// clean up parts we no longer need
	pAcc->m_dAccum.Resize ( 0 );
	pAcc->m_dAccumRows.Resize ( 0 );

	// finish cleaning up and release accumulator
	pAcc->m_pIndex = NULL;
	pAcc->m_iAccumDocs = 0;
	pAcc->m_dAccumKlist.Reset();
}

bool RtIndex_t::DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError )
{
	assert ( g_bRTChangesAllowed );
	MEMORY ( SPH_MEM_IDX_RT_ACCUM );

	RtAccum_t * pAcc = AcquireAccum ( &sError );
	if ( !pAcc )
		return false;

	if ( !iDocs )
		return true;

	assert ( pDocs && iDocs );

	// !COMMIT should handle case when uDoc what inserted in current txn here
	while ( iDocs-- )
		pAcc->m_dAccumKlist.Add ( *pDocs++ );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// LOAD/SAVE
//////////////////////////////////////////////////////////////////////////

struct Checkpoint_t
{
	uint64_t m_uWord;
	uint64_t m_uOffset;
};


void RtIndex_t::DumpToDisk ( const char * sFilename )
{
	MEMORY ( SPH_MEM_IDX_RT );

	Verify ( m_tWriterMutex.Lock() );
	Verify ( m_tRwlock.WriteLock() );
	SaveDiskData ( sFilename );
	Verify ( m_tRwlock.Unlock() );
	Verify ( m_tWriterMutex.Unlock() );
}

// Here is the devil of saving id32 chunk from id64 binary daemon
template < typename DOCID, typename WORDID >
void RtIndex_t::SaveDiskDataImpl ( const char * sFilename ) const
{
	typedef RtDoc_tmpl<DOCID> RTDOC;
	typedef RtWord_tmpl<WORDID> RTWORD;

	CSphString sName, sError;

	CSphWriter wrHits, wrDocs, wrDict, wrRows;
	sName.SetSprintf ( "%s.spp", sFilename ); wrHits.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spd", sFilename ); wrDocs.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spi", sFilename ); wrDict.OpenFile ( sName.cstr(), sError );
	sName.SetSprintf ( "%s.spa", sFilename ); wrRows.OpenFile ( sName.cstr(), sError );

	BYTE bDummy = 1;
	wrDict.PutBytes ( &bDummy, 1 );
	wrDocs.PutBytes ( &bDummy, 1 );
	wrHits.PutBytes ( &bDummy, 1 );

	// we don't have enough RAM to create new merged segments
	// and have to do N-way merge kinda in-place
	CSphVector<RtWordReader_tmpl<WORDID>*> pWordReaders;
	CSphVector<RtDocReader_tmpl<DOCID>*> pDocReaders;
	CSphVector<RtSegment_t*> pSegments;
	CSphVector<const RTWORD*> pWords;
	CSphVector<const RTDOC*> pDocs;

	pWordReaders.Reserve ( m_pSegments.GetLength() );
	pDocReaders.Reserve ( m_pSegments.GetLength() );
	pSegments.Reserve ( m_pSegments.GetLength() );
	pWords.Reserve ( m_pSegments.GetLength() );
	pDocs.Reserve ( m_pSegments.GetLength() );

	// OPTIMIZE? somehow avoid new on iterators maybe?
	ARRAY_FOREACH ( i, m_pSegments )
		pWordReaders.Add ( new RtWordReader_tmpl<WORDID> ( m_pSegments[i] ) );

	ARRAY_FOREACH ( i, pWordReaders )
		pWords.Add ( pWordReaders[i]->UnzipWord() );

	// loop keywords
	static const int WORDLIST_CHECKPOINT = 64;
	CSphVector<Checkpoint_t> dCheckpoints;
	int iWords = 0;

	WORDID uLastWord = 0;
	SphOffset_t uLastDocpos = 0;

	for ( ;; )
	{
		// find keyword with min id
		const RTWORD * pWord = NULL;
		ARRAY_FOREACH ( i, pWords ) // OPTIMIZE? PQ or at least nulls removal here?!
			if ( pWords[i] )
				if ( !pWord || pWords[i]->m_uWordID < pWord->m_uWordID )
					pWord = pWords[i];
		if ( !pWord )
			break;

		// loop all segments that have this keyword
		assert ( pSegments.GetLength()==0 );
		assert ( pDocReaders.GetLength()==0 );
		assert ( pDocs.GetLength()==0 );

		ARRAY_FOREACH ( i, pWords )
			if ( pWords[i] && pWords[i]->m_uWordID==pWord->m_uWordID )
		{
			pSegments.Add ( m_pSegments[i] );
			pDocReaders.Add ( new RtDocReader_tmpl<DOCID> ( m_pSegments[i], *pWords[i] ) );

			const RTDOC * pDoc = pDocReaders.Last()->UnzipDoc();
			while ( pDoc && m_pSegments[i]->m_dKlist.BinarySearch ( pDoc->m_uDocID ) )
				pDoc = pDocReaders.Last()->UnzipDoc();

			pDocs.Add ( pDoc );
		}

		// loop documents
		SphOffset_t uDocpos = wrDocs.GetPos();
		DOCID uLastDoc = 0;
		SphOffset_t uLastHitpos = 0;
		int iDocs = 0;
		int iHits = 0;
		for ( ;; )
		{
			// find alive doc with min id
			int iMinReader = -1;
			ARRAY_FOREACH ( i, pDocs ) // OPTIMIZE?
			{
				if ( !pDocs[i] )
					continue;

				assert ( !pSegments[i]->m_dKlist.BinarySearch ( pDocs[i]->m_uDocID ) );
				if ( iMinReader<0 || pDocs[i]->m_uDocID < pDocs[iMinReader]->m_uDocID )
					iMinReader = i;
			}
			if ( iMinReader<0 )
				break;

			// write doclist entry
			const RTDOC * pDoc = pDocs[iMinReader]; // shortcut
			iDocs++;
			iHits += pDoc->m_uHits;

			wrDocs.ZipOffset ( pDoc->m_uDocID - uLastDoc );
			wrDocs.ZipOffset ( wrHits.GetPos() - uLastHitpos );
			wrDocs.ZipInt ( pDoc->m_uFields );
			wrDocs.ZipInt ( pDoc->m_uHits );
			uLastDoc = pDoc->m_uDocID;
			uLastHitpos = wrHits.GetPos();

			// loop hits from most current segment
			if ( pDoc->m_uHits>1 )
			{
				DWORD uLastHit = 0;
				RtHitReader_t tInHit ( pSegments[iMinReader], pDoc );
				for ( DWORD uValue=tInHit.UnzipHit(); uValue; uValue=tInHit.UnzipHit() )
				{
					wrHits.ZipInt ( uValue - uLastHit );
					uLastHit = uValue;
				}
			} else
			{
				wrHits.ZipInt ( pDoc->m_uHit );
			}
			wrHits.ZipInt ( 0 );

			// fast forward readers
			DOCID uMinID = pDocs[iMinReader]->m_uDocID;
			ARRAY_FOREACH ( i, pDocs )
				while ( pDocs[i] && ( pDocs[i]->m_uDocID<=uMinID || pSegments[i]->m_dKlist.BinarySearch ( pDocs[i]->m_uDocID ) ) )
					pDocs[i] = pDocReaders[i]->UnzipDoc();
		}

		// write dict entry if necessary
		if ( wrDocs.GetPos()!=uDocpos )
		{
			wrDocs.ZipInt ( 0 );

			if ( !iWords )
			{
				Checkpoint_t & tChk = dCheckpoints.Add ();
				tChk.m_uWord = pWord->m_uWordID;
				tChk.m_uOffset = wrDict.GetPos();
			}

			wrDict.ZipOffset ( pWord->m_uWordID - uLastWord );
			wrDict.ZipOffset ( uDocpos - uLastDocpos );
			wrDict.ZipInt ( iDocs );
			wrDict.ZipInt ( iHits );
			uLastWord = pWord->m_uWordID;
			uLastDocpos = uDocpos;

			if ( ++iWords==WORDLIST_CHECKPOINT )
			{
				wrDict.ZipInt ( 0 );
				wrDict.ZipOffset ( wrDocs.GetPos() - uLastDocpos ); // store last hitlist length
				uLastDocpos = 0;
				uLastWord = 0;
				iWords = 0;
			}
		}

		// move words forward
		WORDID uMinID = pWord->m_uWordID; // because pWord contents will move forward too!
		ARRAY_FOREACH ( i, pWords )
			if ( pWords[i] && pWords[i]->m_uWordID==uMinID )
				pWords[i] = pWordReaders[i]->UnzipWord();

		// cleanup
		ARRAY_FOREACH ( i, pDocReaders )
			SafeDelete ( pDocReaders[i] );
		pSegments.Resize ( 0 );
		pDocReaders.Resize ( 0 );
		pDocs.Resize ( 0 );
	}

	// write checkpoints
	wrDict.ZipInt ( 0 ); // indicate checkpoint
	wrDict.ZipOffset ( wrDocs.GetPos() - uLastDocpos ); // store last doclist length

	SphOffset_t iCheckpointsPosition = wrDict.GetPos();
	ARRAY_FOREACH ( i, dCheckpoints )
	{
		wrDict.PutOffset ( dCheckpoints[i].m_uWord );
		wrDict.PutOffset ( dCheckpoints[i].m_uOffset );
	}

	// write attributes
	// the new, template-param aligned iStride instead of index-wide.
	int iStride = DWSIZEOF(DOCID) + m_tSchema.GetRowSize();
	CSphVector<RtRowIterator_tmpl<DOCID>*> pRowIterators ( m_pSegments.GetLength() );
	ARRAY_FOREACH ( i, m_pSegments )
		pRowIterators[i] = new RtRowIterator_tmpl<DOCID> ( m_pSegments[i], iStride, false, NULL );

	CSphVector<const CSphRowitem*> pRows ( m_pSegments.GetLength() );
	ARRAY_FOREACH ( i, pRowIterators )
		pRows[i] = pRowIterators[i]->GetNextAliveRow();

	// prepare to build min-max index for attributes too
	int iTotalDocs = 0;
	ARRAY_FOREACH ( i, m_pSegments )
		iTotalDocs += m_pSegments[i]->m_iAliveRows;
	AttrIndexBuilder_t<DOCID> tMinMaxBuilder ( m_tSchema );
	CSphVector<DWORD> dMinMaxBuffer ( tMinMaxBuilder.GetExpectedSize ( iTotalDocs ) );
	tMinMaxBuilder.Prepare ( dMinMaxBuffer.Begin(), dMinMaxBuffer.Begin() + dMinMaxBuffer.GetLength() );

	sName.SetSprintf ( "%s.sps", sFilename );
	CSphWriter tStrWriter;
	tStrWriter.OpenFile ( sName.cstr(), sError );
	tStrWriter.PutByte ( 0 ); // dummy byte, to reserve magic zero offset

	sName.SetSprintf ( "%s.spm", sFilename );
	CSphWriter tMvaWriter;
	tMvaWriter.OpenFile ( sName.cstr(), sError );
	tMvaWriter.PutDword ( 0 ); // dummy dword, to reserve magic zero offset

	CSphRowitem * pFixedRow = new CSphRowitem[iStride];

#ifndef NDEBUG
	int iStoredDocs = 0;
#endif

	StorageStringWriter_t tStorageString ( m_tSchema, tStrWriter );
	StorageMvaWriter_t tStorageMva ( m_tSchema, tMvaWriter );

	for ( ;; )
	{
		// find min row
		int iMinRow = -1;
		ARRAY_FOREACH ( i, pRows )
			if ( pRows[i] )
				if ( iMinRow<0 || DOCINFO2ID_T<DOCID> ( pRows[i] ) < DOCINFO2ID_T<DOCID> ( pRows[iMinRow] ) )
					iMinRow = i;
		if ( iMinRow<0 )
			break;

#ifndef NDEBUG
		// verify that it's unique
		int iDupes = 0;
		ARRAY_FOREACH ( i, pRows )
			if ( pRows[i] )
				if ( DOCINFO2ID_T<DOCID> ( pRows[i] )==DOCINFO2ID_T<DOCID> ( pRows[iMinRow] ) )
					iDupes++;
		assert ( iDupes==1 );
#endif

		const CSphRowitem * pRow = pRows[iMinRow];

		// strings storage for stored row
		assert ( iMinRow<m_pSegments.GetLength() );
		const RtSegment_t * pSegment = m_pSegments[iMinRow];

#ifdef PARANOID // sanity check in PARANOID mode
		VerifyEmptyStrings<DOCID> ( pSegment->m_dStrings, m_tSchema, pRow );
#endif

		// collect min-max data
		tMinMaxBuilder.Collect ( pRow, pSegment->m_dMvas.Begin(), pSegment->m_dMvas.GetLength(), sError );

		if ( pSegment->m_dStrings.GetLength()>1 || pSegment->m_dMvas.GetLength()>1 ) // should be more then dummy zero elements
		{
			// copy row content as we'll fix up its attrs ( string offset for now )
			memcpy ( pFixedRow, pRow, iStride*sizeof(CSphRowitem) );
			pRow = pFixedRow;

			CopyFixupStorageAttrs<DOCID> ( pSegment->m_dStrings, tStorageString, pFixedRow );
			CopyFixupStorageAttrs<DOCID> ( pSegment->m_dMvas, tStorageMva, pFixedRow );
		}

		// emit it
		wrRows.PutBytes ( pRow, iStride*sizeof(CSphRowitem) );

		// fast forward
		pRows[iMinRow] = pRowIterators[iMinRow]->GetNextAliveRow();
#ifndef NDEBUG
		iStoredDocs++;
#endif
	}

	SafeDeleteArray ( pFixedRow );

	assert ( iStoredDocs==iTotalDocs );

	tMinMaxBuilder.FinishCollect ( false );
	if ( tMinMaxBuilder.GetActualSize() )
		wrRows.PutBytes ( dMinMaxBuffer.Begin(), sizeof(DWORD) * tMinMaxBuilder.GetActualSize() );

	tStrWriter.CloseFile ();

	// write dummy string attributes, mva and kill-list files
	CSphWriter wrDummy;

	// dump killlist
	sName.SetSprintf ( "%s.spk", sFilename );
	wrDummy.OpenFile ( sName.cstr(), sError );
	m_tKlist.Flush();
	m_tKlist.KillListLock();
	DWORD uKlistSize = m_tKlist.GetKillListSize();
	if ( uKlistSize )
		wrDummy.PutBytes ( m_tKlist.GetKillList(), uKlistSize*sizeof ( SphAttr_t ) );
	m_tKlist.Reset();
	m_tKlist.KillListUnlock();
	wrDummy.CloseFile ();

	sName.SetSprintf ( "%s.spm", sFilename ); wrDummy.OpenFile ( sName.cstr(), sError ); wrDummy.CloseFile ();

	// header
	SaveDiskHeader ( sFilename, dCheckpoints.GetLength(), iCheckpointsPosition, uKlistSize, iTotalDocs*iStride, m_bId32to64 );

	// cleanup
	ARRAY_FOREACH ( i, pWordReaders )
		SafeDelete ( pWordReaders[i] );
	ARRAY_FOREACH ( i, pDocReaders )
		SafeDelete ( pDocReaders[i] );
	ARRAY_FOREACH ( i, pRowIterators )
		SafeDelete ( pRowIterators[i] );

	// done
	wrHits.CloseFile ();
	wrDocs.CloseFile ();
	wrDict.CloseFile ();
	wrRows.CloseFile ();
}

void RtIndex_t::SaveDiskData ( const char * sFilename ) const
{
	if ( m_bId32to64 )
		return SaveDiskDataImpl<DWORD,DWORD> (sFilename);
	else
		return SaveDiskDataImpl<SphDocID_t,SphWordID_t> (sFilename);
}

static void WriteFileInfo ( CSphWriter & tWriter, const CSphSavedFile & tInfo )
{
	tWriter.PutOffset ( tInfo.m_uSize );
	tWriter.PutOffset ( tInfo.m_uCTime );
	tWriter.PutOffset ( tInfo.m_uMTime );
	tWriter.PutDword ( tInfo.m_uCRC32 );
}

static void WriteSchemaColumn ( CSphWriter & tWriter, const CSphColumnInfo & tColumn )
{
	int iLen = strlen ( tColumn.m_sName.cstr() );
	tWriter.PutDword ( iLen );
	tWriter.PutBytes ( tColumn.m_sName.cstr(), iLen );

	ESphAttr eAttrType = tColumn.m_eAttrType;
	if ( eAttrType==SPH_ATTR_WORDCOUNT )
		eAttrType = SPH_ATTR_INTEGER;
	tWriter.PutDword ( eAttrType );

	tWriter.PutDword ( tColumn.m_tLocator.CalcRowitem() ); // for backwards compatibility
	tWriter.PutDword ( tColumn.m_tLocator.m_iBitOffset );
	tWriter.PutDword ( tColumn.m_tLocator.m_iBitCount );

	tWriter.PutByte ( tColumn.m_bPayload );
}


void RtIndex_t::SaveDiskHeader ( const char * sFilename, int iCheckpoints, SphOffset_t iCheckpointsPosition, DWORD uKillListSize, DWORD uMinMaxSize, bool bForceID32 ) const
{
	static const DWORD INDEX_MAGIC_HEADER	= 0x58485053;	///< my magic 'SPHX' header
	static const DWORD INDEX_FORMAT_VERSION	= 24;			///< my format version

	CSphWriter tWriter;
	CSphString sName, sError;
	sName.SetSprintf ( "%s.sph", sFilename );
	tWriter.OpenFile ( sName.cstr(), sError );

	// format
	tWriter.PutDword ( INDEX_MAGIC_HEADER );
	tWriter.PutDword ( INDEX_FORMAT_VERSION );

	if ( bForceID32 )
		tWriter.PutDword ( 0 ); // use-32bit
	else
		tWriter.PutDword ( USE_64BIT ); // use-64bit
	tWriter.PutDword ( SPH_DOCINFO_EXTERN );

	// schema
	tWriter.PutDword ( m_tSchema.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_tSchema.m_dFields )
		WriteSchemaColumn ( tWriter, m_tSchema.m_dFields[i] );

	tWriter.PutDword ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( tWriter, m_tSchema.GetAttr(i) );

	tWriter.PutOffset ( 0 ); // min docid

	// wordlist checkpoints
	tWriter.PutOffset ( iCheckpointsPosition );
	tWriter.PutDword ( iCheckpoints );

	// stats
	tWriter.PutDword ( m_tStats.m_iTotalDocuments );
	tWriter.PutOffset ( m_tStats.m_iTotalBytes );

	// index settings
	tWriter.PutDword ( m_tSettings.m_iMinPrefixLen );
	tWriter.PutDword ( m_tSettings.m_iMinInfixLen );
	tWriter.PutByte ( m_tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( m_tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( m_tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( m_tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( m_tSettings.m_eHitless );
	tWriter.PutDword ( SPH_HIT_FORMAT_PLAIN );
	tWriter.PutByte ( 0 ); // m_bIndexSP, v.21+
	tWriter.PutString ( CSphString() ); // m_sZonePrefix, v.22+
	tWriter.PutDword ( 0 ); // m_iBoundaryStep, v.23+
	tWriter.PutDword ( 1 ); // m_iStopwordStep, v.23+

	// tokenizer
	assert ( m_pTokenizer );
	const CSphTokenizerSettings & tSettings = m_pTokenizer->GetSettings ();
	tWriter.PutByte ( tSettings.m_iType );
	tWriter.PutString ( tSettings.m_sCaseFolding.cstr () );
	tWriter.PutDword ( tSettings.m_iMinWordLen );
	tWriter.PutString ( tSettings.m_sSynonymsFile.cstr () );
	WriteFileInfo ( tWriter, m_pTokenizer->GetSynFileInfo () );
	tWriter.PutString ( tSettings.m_sBoundary.cstr () );
	tWriter.PutString ( tSettings.m_sIgnoreChars.cstr () );
	tWriter.PutDword ( tSettings.m_iNgramLen );
	tWriter.PutString ( tSettings.m_sNgramChars.cstr () );
	tWriter.PutString ( tSettings.m_sBlendChars.cstr () );
	tWriter.PutString ( tSettings.m_sBlendMode.cstr () );

	// dictionary
	assert ( m_pDict );

	const CSphDictSettings & tDict = m_pDict->GetSettings ();
	tWriter.PutString ( tDict.m_sMorphology.cstr () );
	tWriter.PutString ( tDict.m_sStopwords.cstr () );

	const CSphVector <CSphSavedFile> & dSWFileInfos = m_pDict->GetStopwordsFileInfos ();
	tWriter.PutDword ( dSWFileInfos.GetLength () );
	ARRAY_FOREACH ( i, dSWFileInfos )
	{
		tWriter.PutString ( dSWFileInfos[i].m_sFilename.cstr () );
		WriteFileInfo ( tWriter, dSWFileInfos[i] );
	}

	const CSphSavedFile & tWFFileInfo = m_pDict->GetWordformsFileInfo ();
	tWriter.PutString ( tDict.m_sWordforms.cstr () );
	WriteFileInfo ( tWriter, tWFFileInfo );
	tWriter.PutDword ( tDict.m_iMinStemmingLen );
	tWriter.PutByte ( 0 ); // m_bWordDict flag, v.21+

	// kill-list size
	tWriter.PutDword ( uKillListSize );

	// min-max count
	tWriter.PutDword ( uMinMaxSize );

	// done
	tWriter.CloseFile ();
}


#if USE_WINDOWS
#undef rename
int rename ( const char * sOld, const char * sNew )
{
	if ( MoveFileEx ( sOld, sNew, MOVEFILE_REPLACE_EXISTING ) )
		return 0;
	errno = GetLastError();
	return -1;
}
#endif


void RtIndex_t::SaveMeta ( int iDiskChunks )
{
	// sanity check
	if ( m_iLockFD<0 )
		return;

	// write new meta
	CSphString sMeta, sMetaNew;
	sMeta.SetSprintf ( "%s.meta", m_sPath.cstr() );
	sMetaNew.SetSprintf ( "%s.meta.new", m_sPath.cstr() );

	CSphString sError;
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMetaNew, sError ) )
		sphDie ( "failed to serialize meta: %s", sError.cstr() ); // !COMMIT handle this gracefully
	wrMeta.PutDword ( META_HEADER_MAGIC );
	wrMeta.PutDword ( META_VERSION );
	wrMeta.PutDword ( iDiskChunks );
	wrMeta.PutDword ( m_tStats.m_iTotalDocuments );
	wrMeta.PutOffset ( m_tStats.m_iTotalBytes ); // FIXME? need PutQword ideally
	wrMeta.PutOffset ( m_iTID );
	wrMeta.CloseFile();

	// rename
	if ( ::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMetaNew.cstr(), sMeta.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully
}


void RtIndex_t::SaveDiskChunk ()
{
	if ( !m_pSegments.GetLength() )
		return;

	MEMORY ( SPH_MEM_IDX_RT );

	// dump it
	CSphString sNewChunk;
	sNewChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), m_pDiskChunks.GetLength() );
	SaveDiskData ( sNewChunk.cstr() );

	// bring new disk chunk online
	CSphIndex * pDiskChunk = LoadDiskChunk ( m_pDiskChunks.GetLength(), false );
	assert ( pDiskChunk );

	// save updated meta
	SaveMeta ( m_pDiskChunks.GetLength()+1 );
	m_iSavedTID = m_iTID;
	m_iSavedRam = 0;
	m_tmSaved = sphMicroTimer();

	// FIXME! add binlog cleanup here once we have binlogs

	// get exclusive lock again, gotta reset RAM chunk now
	Verify ( m_tRwlock.WriteLock() );
	ARRAY_FOREACH ( i, m_pSegments )
		SafeDelete ( m_pSegments[i] );
	m_pSegments.Reset();
	m_pDiskChunks.Add ( pDiskChunk );
	Verify ( m_tRwlock.Unlock() );
}


CSphIndex * RtIndex_t::LoadDiskChunk ( int iChunk, bool bStripPath )
{
	MEMORY ( SPH_MEM_IDX_DISK );

	CSphString sChunk, sError, sWarning;
	sChunk.SetSprintf ( "%s.%d", m_sPath.cstr(), iChunk );

	// !COMMIT handle errors gracefully instead of dying
	CSphIndex * pDiskChunk = sphCreateIndexPhrase ( m_sIndexName.cstr(), sChunk.cstr() );
	if ( !pDiskChunk )
		sphDie ( "disk chunk %s: alloc failed", sChunk.cstr() );

	pDiskChunk->SetWordlistPreload ( m_bPreloadWordlist );

	if ( !pDiskChunk->Prealloc ( false, bStripPath, sWarning ) )
		sphDie ( "disk chunk %s: prealloc failed: %s", sChunk.cstr(), pDiskChunk->GetLastError().cstr() );

	if ( !pDiskChunk->Preread() )
		sphDie ( "disk chunk %s: preread failed: %s", sChunk.cstr(), pDiskChunk->GetLastError().cstr() );

	ISphIndex_VLN * pChunk = dynamic_cast<ISphIndex_VLN*> ( pDiskChunk );
	if ( !pChunk )
		sphDie ( "disk chunk %s: internal error on load, dynamic_cast failed", sChunk.cstr() );
	pChunk->SetDynamize ( m_dDynamize );

	return pDiskChunk;
}


bool RtIndex_t::Prealloc ( bool, bool bStripPath, CSphString & )
{
	MEMORY ( SPH_MEM_IDX_RT );

	// locking uber alles
	// in RT backed case, we just must be multi-threaded
	// so we simply lock here, and ignore Lock/Unlock hassle caused by forks
	assert ( m_iLockFD<0 );

	CSphString sLock;
	sLock.SetSprintf ( "%s.lock", m_sPath.cstr() );
	m_iLockFD = ::open ( sLock.cstr(), SPH_O_NEW, 0644 );
	if ( m_iLockFD<0 )
	{
		m_sLastError.SetSprintf ( "failed to open %s: %s", sLock.cstr(), strerror(errno) );
		return false;
	}
	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock %s: %s", sLock.cstr(), strerror(errno) );
		::close ( m_iLockFD );
		return false;
	}

	// check if we have a meta file (kinda-header)
	CSphString sMeta;
	sMeta.SetSprintf ( "%s.meta", m_sPath.cstr() );

	// no readable meta? no disk part yet
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return true;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, m_sLastError ) )
		return false;

	if ( rdMeta.GetDword()!=META_HEADER_MAGIC )
	{
		m_sLastError.SetSprintf ( "invalid meta file %s", sMeta.cstr() );
		return false;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==0 || uVersion>META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sMeta.cstr(), uVersion, META_VERSION );
		return false;
	}
	const int iDiskChunks = rdMeta.GetDword();
	m_tStats.m_iTotalDocuments = rdMeta.GetDword();
	m_tStats.m_iTotalBytes = rdMeta.GetOffset();
	if ( uVersion>=2 )
		m_iTID = rdMeta.GetOffset();

	// load disk chunks, if any
	for ( int iChunk=0; iChunk<iDiskChunks; iChunk++ )
	{
		m_pDiskChunks.Add ( LoadDiskChunk ( iChunk, bStripPath ) );

		// tricky bit
		// outgoing match schema on disk chunk should be identical to our internal (!) schema
		if ( !m_tSchema.CompareTo ( m_pDiskChunks.Last()->GetMatchSchema(), m_sLastError ) )
			return false;
	}

	// load ram chunk
	bool bRamLoaded = LoadRamChunk ( uVersion );

	// set up values for on timer save
	m_iSavedTID = m_iTID;
	m_iSavedRam = GetUsedRam();
	m_tmSaved = sphMicroTimer();

	return bRamLoaded;
}


bool RtIndex_t::Preread ()
{
	// !COMMIT move disk chunks prereading here
	return true;
}

template < typename T > struct IsPodType { enum { Value = false }; };
template<> struct IsPodType<char> { enum { Value = true }; };
template<> struct IsPodType<BYTE> { enum { Value = true }; };
template<> struct IsPodType<int> { enum { Value = true }; };
template<> struct IsPodType<DWORD> { enum { Value = true }; };
template<> struct IsPodType<uint64_t> { enum { Value = true }; };
template<> struct IsPodType<float> { enum { Value = true }; };


template < typename T, typename P >
static void SaveVector ( CSphWriter & tWriter, const CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.PutDword ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static void LoadVector ( CSphReader & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( tReader.GetDword() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static void SaveVector ( BinlogWriter_c & tWriter, const CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tWriter.ZipValue ( tVector.GetLength() );
	if ( tVector.GetLength() )
		tWriter.PutBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
}


template < typename T, typename P >
static bool LoadVector ( BinlogReader_c & tReader, CSphVector < T, P > & tVector )
{
	STATIC_ASSERT ( IsPodType<T>::Value, NON_POD_VECTORS_ARE_UNSERIALIZABLE );
	tVector.Resize ( (int) tReader.UnzipValue() ); // FIXME? sanitize?
	if ( tVector.GetLength() )
		tReader.GetBytes ( tVector.Begin(), tVector.GetLength()*sizeof(T) );
	return !tReader.GetErrorFlag();
}


bool RtIndex_t::SaveRamChunk ()
{
	MEMORY ( SPH_MEM_IDX_RT );

	CSphString sChunk, sNewChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );
	sNewChunk.SetSprintf ( "%s.ram.new", m_sPath.cstr() );
	m_tKlist.SaveToFile ( m_sPath.cstr() );

	CSphWriter wrChunk;
	if ( !wrChunk.OpenFile ( sNewChunk, m_sLastError ) )
		return false;

	wrChunk.PutDword ( USE_64BIT );
	wrChunk.PutDword ( RtSegment_t::m_iSegments );
	wrChunk.PutDword ( m_pSegments.GetLength() );

	// no locks here, because it's only intended to be called from dtor
	ARRAY_FOREACH ( iSeg, m_pSegments )
	{
		const RtSegment_t * pSeg = m_pSegments[iSeg];
		wrChunk.PutDword ( pSeg->m_iTag );
		SaveVector ( wrChunk, pSeg->m_dWords );
		wrChunk.PutDword ( pSeg->m_dWordCheckpoints.GetLength() );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_iOffset );
			wrChunk.PutOffset ( pSeg->m_dWordCheckpoints[i].m_uWordID );
		}
		SaveVector ( wrChunk, pSeg->m_dDocs );
		SaveVector ( wrChunk, pSeg->m_dHits );
		wrChunk.PutDword ( pSeg->m_iRows );
		wrChunk.PutDword ( pSeg->m_iAliveRows );
		SaveVector ( wrChunk, pSeg->m_dRows );
		SaveVector ( wrChunk, pSeg->m_dKlist );
		SaveVector ( wrChunk, pSeg->m_dStrings );
		SaveVector ( wrChunk, pSeg->m_dMvas );
	}

	wrChunk.CloseFile();
	if ( wrChunk.IsError() )
		return false;

	// rename
	if ( ::rename ( sNewChunk.cstr(), sChunk.cstr() ) )
		sphDie ( "failed to rename ram chunk (src=%s, dst=%s, errno=%d, error=%s)",
			sNewChunk.cstr(), sChunk.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully

	return true;
}


bool RtIndex_t::LoadRamChunk ( DWORD uVersion )
{
	MEMORY ( SPH_MEM_IDX_RT );

	CSphString sChunk;
	sChunk.SetSprintf ( "%s.ram", m_sPath.cstr() );

	if ( !sphIsReadable ( sChunk.cstr(), &m_sLastError ) )
		return true;

	m_tKlist.LoadFromFile ( m_sPath.cstr() );

	CSphAutoreader rdChunk;
	if ( !rdChunk.Open ( sChunk, m_sLastError ) )
		return false;

	bool bId64 = ( rdChunk.GetDword()!=0 );
	if ( bId64!=USE_64BIT )
	{
#if USE_64BIT
// #if 0
		// TODO: may be do this param conditional and push it into the config?
		m_bId32to64 = true;
#else
		m_sLastError.SetSprintf ( "ram chunk dumped by %s binary; this binary is %s",
			bId64 ? "id64" : "id32",
			USE_64BIT ? "id64" : "id32" );
		return false;
#endif
	}

	int iSegmentSeq = rdChunk.GetDword();
	m_pSegments.Resize ( rdChunk.GetDword() ); // FIXME? sanitize

	ARRAY_FOREACH ( iSeg, m_pSegments )
	{
		RtSegment_t * pSeg = new RtSegment_t ();
		m_pSegments[iSeg] = pSeg;

		pSeg->m_iTag = rdChunk.GetDword ();
		LoadVector ( rdChunk, pSeg->m_dWords );
		pSeg->m_dWordCheckpoints.Resize ( rdChunk.GetDword() );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int)rdChunk.GetOffset();
			pSeg->m_dWordCheckpoints[i].m_uWordID = (SphWordID_t)rdChunk.GetOffset();
		}
		LoadVector ( rdChunk, pSeg->m_dDocs );
		LoadVector ( rdChunk, pSeg->m_dHits );
		pSeg->m_iRows = rdChunk.GetDword();
		pSeg->m_iAliveRows = rdChunk.GetDword();
		LoadVector ( rdChunk, pSeg->m_dRows );
		LoadVector ( rdChunk, pSeg->m_dKlist );
		LoadVector ( rdChunk, pSeg->m_dStrings );
		if ( uVersion>=3 )
			LoadVector ( rdChunk, pSeg->m_dMvas );
	}

	RtSegment_t::m_iSegments = iSegmentSeq;
	if ( rdChunk.GetErrorFlag() )
		return false;
	return true;
}

void RtIndex_t::PostSetup()
{
	if ( m_bId32to64 )
	{
		SaveDiskChunk();
		// since the RAM chunk is just stored as id32, we are no more in compat mode
		m_bId32to64 = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// SEARCHING
//////////////////////////////////////////////////////////////////////////

struct RtQword_t : public ISphQword
{
	friend struct RtIndex_t;
	friend struct RtQwordSetup_t;

protected:
	RtDocReader_t *		m_pDocReader;
	CSphMatch			m_tMatch;

	DWORD				m_uNextHit;
	RtHitReader2_t		m_tHitReader;

	RtSegment_t *		m_pSeg;

public:
	RtQword_t ()
		: m_pDocReader ( NULL )
		, m_uNextHit ( 0 )
		, m_pSeg ( NULL )
	{
		m_tMatch.Reset ( 0 );
	}

	virtual ~RtQword_t ()
	{
		SafeDelete ( m_pDocReader );
	}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		for ( ;; )
		{
			const RtDoc_t * pDoc = m_pDocReader->UnzipDoc();
			if ( !pDoc )
			{
				m_tMatch.m_iDocID = 0;
				return m_tMatch;
			}

			if ( m_pSeg->m_dKlist.BinarySearch ( pDoc->m_uDocID ) )
				continue;

			m_tMatch.m_iDocID = pDoc->m_uDocID;
			m_uFields = pDoc->m_uFields;
			m_uMatchHits = pDoc->m_uHits;
			m_iHitlistPos = (uint64_t(pDoc->m_uHits)<<32) + pDoc->m_uHit;
			return m_tMatch;
		}
	}

	virtual void SeekHitlist ( SphOffset_t uOff )
	{
		int iHits = (int)(uOff>>32);
		if ( iHits==1 )
		{
			m_uNextHit = DWORD(uOff);
		} else
		{
			m_uNextHit = 0;
			m_tHitReader.Seek ( DWORD(uOff), iHits );
		}
	}

	virtual Hitpos_t GetNextHit ()
	{
		if ( m_uNextHit==0 )
		{
			return Hitpos_t ( m_tHitReader.UnzipHit() );

		} else if ( m_uNextHit==0xffffffffUL )
		{
			return EMPTY_HIT;

		} else
		{
			DWORD uRes = m_uNextHit;
			m_uNextHit = 0xffffffffUL;
			return Hitpos_t ( uRes );
		}
	}
};


struct RtQwordSetup_t : ISphQwordSetup
{
	RtSegment_t *		m_pSeg;

	virtual ISphQword *	QwordSpawn ( const XQKeyword_t & ) const;
	virtual bool		QwordSetup ( ISphQword * pQword ) const;
};


ISphQword * RtQwordSetup_t::QwordSpawn ( const XQKeyword_t & ) const
{
	return new RtQword_t ();
}


bool RtQwordSetup_t::QwordSetup ( ISphQword * pQword ) const
{
	RtQword_t * pMyWord = dynamic_cast<RtQword_t*> ( pQword );
	if ( !pMyWord )
		return false;

	const RtIndex_t * pIndex = dynamic_cast< const RtIndex_t * > ( m_pIndex );
	if ( !pIndex )
		return false;

	return pIndex->RtQwordSetup ( pMyWord, m_pSeg );
}


bool RtIndex_t::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	// might be needed even when we do not have a filter!
	if ( pCtx->m_bLookupFilter || pCtx->m_bLookupSort )
		CopyDocinfo ( tMatch, FindDocinfo ( (RtSegment_t*)pCtx->m_pIndexData, tMatch.m_iDocID ) );

	ARRAY_FOREACH ( i, m_dDynamize )
		tMatch.SetAttr ( m_dDynamize[i].m_tTo, tMatch.GetAttr ( m_dDynamize[i].m_tFrom ) );

	pCtx->CalcFilter ( tMatch );
	return pCtx->m_pFilter ? !pCtx->m_pFilter->Eval ( tMatch ) : false;
}


void RtIndex_t::CopyDocinfo ( CSphMatch & tMatch, const DWORD * pFound ) const
{
	if ( !pFound )
		return;

	// setup static pointer
	assert ( DOCINFO2ID(pFound)==tMatch.m_iDocID );
	tMatch.m_pStatic = DOCINFO2ATTRS(pFound);

	// FIXME? implement overrides
}


const CSphRowitem * RtIndex_t::FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID ) const
{
	// FIXME! move to CSphIndex, and implement hashing
	if ( pSeg->m_dRows.GetLength()==0 )
		return NULL;

	int iStride = m_iStride;
	int iStart = 0;
	int iEnd = pSeg->m_iRows-1;
	assert ( iStride==( DOCINFO_IDSIZE + m_tSchema.GetRowSize() ) );

	const CSphRowitem * pStorage = pSeg->m_dRows.Begin();
	const CSphRowitem * pFound = NULL;

	if ( uDocID==DOCINFO2ID ( &pStorage [ iStart*iStride ] ) )
	{
		pFound = &pStorage [ iStart*iStride ];

	} else if ( uDocID==DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) )
	{
		pFound = &pStorage [ iEnd*iStride ];

	} else
	{
		while ( iEnd-iStart>1 )
		{
			// check if nothing found
			if (
				uDocID < DOCINFO2ID ( &pStorage [ iStart*iStride ] ) ||
				uDocID > DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) )
				break;
			assert ( uDocID > DOCINFO2ID ( &pStorage [ iStart*iStride ] ) );
			assert ( uDocID < DOCINFO2ID ( &pStorage [ iEnd*iStride ] ) );

			int iMid = iStart + (iEnd-iStart)/2;
			if ( uDocID==DOCINFO2ID ( &pStorage [ iMid*iStride ] ) )
			{
				pFound = &pStorage [ iMid*iStride ];
				break;
			}
			if ( uDocID<DOCINFO2ID ( &pStorage [ iMid*iStride ] ) )
				iEnd = iMid;
			else
				iStart = iMid;
		}
	}

	return pFound;
}

// WARNING, setup is pretty tricky
// for RT queries, we setup qwords several times
// first pass (with NULL segment arg) should sum all stats over all segments
// others passes (with non-NULL segments) should setup specific segment (including local stats)
bool RtIndex_t::RtQwordSetupSegment ( RtQword_t * pQword, RtSegment_t * pCurSeg, bool bSetup )
{
	if ( !pCurSeg )
		return false;

	SphWordID_t uWordID = pQword->m_iWordID;
	RtWordReader_t tReader ( pCurSeg );

	// position reader to the right checkpoint
	const CSphVector<RtWordCheckpoint_t> & dCheckpoints = pCurSeg->m_dWordCheckpoints;
	if ( dCheckpoints.GetLength() )
	{
		if ( dCheckpoints.Begin()->m_uWordID > uWordID )
		{
			tReader.m_pMax = tReader.m_pCur + dCheckpoints.Begin()->m_iOffset;

		} else if ( dCheckpoints.Last().m_uWordID<=uWordID )
		{
			tReader.m_pCur += dCheckpoints.Last().m_iOffset;

		} else
		{
			int L = 0;
			int R = dCheckpoints.GetLength()-1;
			while ( L+1<R )
			{
				int M = L + (R-L)/2;
				if ( uWordID < dCheckpoints[M].m_uWordID )
					R = M;
				else if ( uWordID > dCheckpoints[M].m_uWordID )
					L = M;
				else
				{
					L = M;
					break;
				}
			}

			assert ( dCheckpoints[L].m_uWordID<=uWordID );
			if ( L < dCheckpoints.GetLength()-1 )
			{
				assert ( dCheckpoints[L+1].m_uWordID > uWordID );
				tReader.m_pMax = tReader.m_pCur + dCheckpoints[L+1].m_iOffset;
			}
			tReader.m_pCur += dCheckpoints[L].m_iOffset;
		}
	}

	// find the word between checkpoints
	const RtWord_t * pWord = NULL;
	while ( ( pWord = tReader.UnzipWord() )!=NULL )
	{
		if ( pWord->m_uWordID==uWordID )
		{
			pQword->m_iDocs += pWord->m_uDocs;
			pQword->m_iHits += pWord->m_uHits;
			if ( bSetup )
			{
				SafeDelete ( pQword->m_pDocReader );
				pQword->m_pDocReader = new RtDocReader_t ( pCurSeg, *pWord );
				pQword->m_tHitReader.m_pBase = NULL;
				if ( pCurSeg->m_dHits.GetLength() )
					pQword->m_tHitReader.m_pBase = pCurSeg->m_dHits.Begin();
				pQword->m_pSeg = pCurSeg;
			}
			return true;

		} else if ( pWord->m_uWordID > uWordID )
			return false;
	}
	return false;
}

bool RtIndex_t::RtQwordSetup ( RtQword_t * pQword, RtSegment_t * pSeg ) const
{
	// segment-specific setup pass
	if ( pSeg )
		return RtQwordSetupSegment ( pQword, pSeg, true );

	// stat-only pass
	// loop all segments, gather stats, do not setup anything
	assert ( !pSeg );
	pQword->m_iDocs = 0;
	pQword->m_iHits = 0;

	// we care about the results anyway though
	// because if all (!) segments miss this word, we must notify the caller, right?
	bool bRes = true;
	ARRAY_FOREACH ( i, m_pSegments )
		bRes &= RtQwordSetupSegment ( pQword, m_pSegments[i], false );

	// sanity check
	assert ( !( m_pSegments.GetLength()!=0 && bRes==true && pQword->m_iDocs==0 ) );
	return bRes;
}

static void AddKillListFilter ( CSphVector<CSphFilterSettings> * pExtra, const SphAttr_t * pKillList, int nEntries )
{
	assert ( nEntries && pKillList && pExtra );
	CSphFilterSettings & tFilter = pExtra->Add();
	tFilter.m_bExclude = true;
	tFilter.m_eType = SPH_FILTER_VALUES;
	tFilter.m_uMinValue = pKillList[0];
	tFilter.m_uMaxValue = pKillList[nEntries-1];
	tFilter.m_sAttrName = "@id";
	tFilter.SetExternalValues ( pKillList, nEntries );
}


// FIXME! missing MVA, index_exact_words support
// FIXME? missing enable_star, legacy match modes support
// FIXME? any chance to factor out common backend agnostic code?
// FIXME? do we need to support pExtraFilters?
#ifndef NDEBUG
bool RtIndex_t::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> *, int iTag ) const
#else
bool RtIndex_t::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> *, int ) const
#endif
{
	assert ( ppSorters );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( iSorters );
	for ( int i=0; i<iSorters; i++ )
		if ( ppSorters[i] )
			dSorters.Add ( ppSorters[i] );

	// if we have anything to work with
	if ( dSorters.GetLength()==0 )
	{
		pResult->m_iQueryTime = 0;
		return false;
	}

	// FIXME! too early (how low can you go?)
	m_tRwlock.ReadLock ();

	assert ( pQuery );
	assert ( pResult );
	assert ( iTag==0 );

	MEMORY ( SPH_MEM_IDX_RT_MULTY_QUERY );

	// start counting
	pResult->m_iQueryTime = 0;
	int64_t tmQueryStart = sphMicroTimer();

	// force ext2 mode for them
	// FIXME! eliminate this const breakage
	const_cast<CSphQuery*> ( pQuery )->m_eMode = SPH_MATCH_EXTENDED2;

	// FIXME! slow disk searches could lock out concurrent writes for too long
	// FIXME! each result will point to its own MVA and string pools
	// !COMMIT need to setup disk K-list here

	//////////////////////
	// search disk chunks
	//////////////////////

	bool m_bKlistLocked = false;
	CSphVector<CSphFilterSettings> dExtra;
	// first, collect all the killlists into a vector
	for ( int iChunk = m_pDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
	{
		const int iOldLength = dExtra.GetLength();
		if ( iChunk==m_pDiskChunks.GetLength()-1 )
		{
			// For the topmost chunk we add the killlist from the ram-index
			m_tKlist.Flush();
			m_tKlist.KillListLock();
			if ( m_tKlist.GetKillListSize() )
			{
				// we don't lock in vain...
				m_bKlistLocked = true;
				AddKillListFilter ( &dExtra, m_tKlist.GetKillList(), m_tKlist.GetKillListSize() );
			} else
				m_tKlist.KillListUnlock();
		} else
		{
			const CSphIndex * pDiskChunk = m_pDiskChunks[iChunk+1];
			if ( pDiskChunk->GetKillListSize () )
				AddKillListFilter ( &dExtra, pDiskChunk->GetKillList(), pDiskChunk->GetKillListSize() );
		}

		if ( dExtra.GetLength()==iOldLength )
			dExtra.Add();
	}

	assert ( dExtra.GetLength()==m_pDiskChunks.GetLength() );
	CSphVector<const BYTE *> dDiskStrings ( m_pDiskChunks.GetLength() );
	CSphVector<const DWORD *> dDiskMva ( m_pDiskChunks.GetLength() );
	ARRAY_FOREACH ( iChunk, m_pDiskChunks )
	{
		CSphQueryResult tChunkResult;
		// storing index in matches tag for finding strings attrs offset later, biased against default zero and segments
		const int iTag = m_pSegments.GetLength()+iChunk+1;
		if ( !m_pDiskChunks[iChunk]->MultiQuery ( pQuery, &tChunkResult, iSorters, ppSorters, &dExtra, iTag ) )
		{
			// FIXME? maybe handle this more gracefully (convert to a warning)?
			pResult->m_sError = tChunkResult.m_sError;
			m_tRwlock.Unlock ();
			if ( m_bKlistLocked )
				m_tKlist.KillListUnlock();
			return false;
		}

		tChunkResult.m_hWordStats.IterateStart();
		while ( tChunkResult.m_hWordStats.IterateNext() )
		{
			const CSphQueryResultMeta::WordStat_t & tStat = tChunkResult.m_hWordStats.IterateGet();
			pResult->AddStat ( tChunkResult.m_hWordStats.IterateGetKey(), tStat.m_iDocs, tStat.m_iHits, tStat.m_bExpanded );
		}

		dDiskStrings[iChunk] = tChunkResult.m_pStrings;
		dDiskMva[iChunk] = tChunkResult.m_pMva;
		dExtra.Pop();
	}

	if ( m_bKlistLocked )
		m_tKlist.KillListUnlock();

	////////////////////
	// search RAM chunk
	////////////////////

	// select the sorter with max schema
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	ARRAY_FOREACH ( i, dSorters )
		if ( dSorters[i]->GetSchema().GetRowSize() > iMaxSchemaSize )
		{
			iMaxSchemaSize = dSorters[i]->GetSchema().GetRowSize();
			iMaxSchemaIndex = i;
		}

	// setup calculations and result schema
	CSphQueryContext tCtx;
	if ( !tCtx.SetupCalc ( pResult, dSorters[iMaxSchemaIndex]->GetSchema(), m_tSchema, NULL ) )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDictBase = m_pDict;
	if ( pDictBase->HasState() )
	{
		tDictCloned = pDictBase = pDictBase->Clone();
	}

	// setup search terms
	RtQwordSetup_t tTermSetup;
	tTermSetup.m_pDict = pDictBase;
	tTermSetup.m_pIndex = this;
	tTermSetup.m_eDocinfo = m_tSettings.m_eDocinfo;
	tTermSetup.m_iDynamicRowitems = pResult->m_tSchema.GetDynamicSize();
	if ( pQuery->m_uMaxQueryMsec>0 )
		tTermSetup.m_iMaxTimer = sphMicroTimer() + pQuery->m_uMaxQueryMsec*1000; // max_query_time
	tTermSetup.m_pWarning = &pResult->m_sWarning;
	tTermSetup.m_pSeg = NULL;
	tTermSetup.m_pCtx = &tCtx;

	// bind weights
	tCtx.BindWeights ( pQuery, m_tOutboundSchema );

	// parse query
	XQQuery_t tParsed;
	if ( !sphParseExtendedQuery ( tParsed, pQuery->m_sQuery.cstr(), GetTokenizer(), &m_tOutboundSchema, pDictBase ) )
	{
		pResult->m_sError = tParsed.m_sParseError;
		m_tRwlock.Unlock ();
		return false;
	}

	// transform query if needed (quorum transform, keyword expansion, etc.)
	sphTransformExtendedQuery ( &tParsed.m_pRoot );

	if ( !sphCheckQueryHeight ( tParsed.m_pRoot, pResult->m_sError ) )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( tParsed, pQuery->m_eRanker, pResult, tTermSetup, tCtx ) );
	if ( !pRanker.Ptr() )
	{
		m_tRwlock.Unlock ();
		return false;
	}

	// empty index, empty result
	if ( !m_pSegments.GetLength() && !m_pDiskChunks.GetLength() )
	{
		pResult->m_iQueryTime = 0;
		m_tRwlock.Unlock ();
		return true;
	}

	if ( m_pSegments.GetLength() )
	{
		// setup filters
		// FIXME! setup filters MVA pool
		bool bFullscan = ( pQuery->m_eMode==SPH_MATCH_FULLSCAN || pQuery->m_sQuery.IsEmpty() );
		if ( !tCtx.CreateFilters ( bFullscan, &pQuery->m_dFilters, pResult->m_tSchema, NULL, pResult->m_sError ) )
		{
			m_tRwlock.Unlock ();
			return false;
		}

		// FIXME! OPTIMIZE! check if we can early reject the whole index

		// setup lookup
		// do pre-filter lookup as needed
		// do pre-sort lookup in all cases
		// post-sort lookup is complicated (because of many segments)
		// pre-sort lookup is cheap now anyway, and almost always anyway
		// (except maybe by stupid relevance-sorting-only benchmarks!!)
		tCtx.m_bLookupFilter = ( pQuery->m_dFilters.GetLength() || tCtx.m_dCalcFilter.GetLength() );
		tCtx.m_bLookupSort = true;

		// FIXME! setup overrides

		// do searching
		bool bRandomize = dSorters[0]->m_bRandomize;
		int iCutoff = pQuery->m_iCutoff;
		if ( iCutoff<=0 )
			iCutoff = -1;

		if ( bFullscan )
		{
			// full scan
			// FIXME? OPTIMIZE? add shortcuts here too?
			CSphMatch tMatch;
			tMatch.Reset ( pResult->m_tSchema.GetDynamicSize() );
			tMatch.m_iWeight = 1;

			int iCutoff = pQuery->m_iCutoff;
			if ( iCutoff<=0 )
				iCutoff = -1;

			ARRAY_FOREACH ( iSeg, m_pSegments )
			{
				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				}

				RtRowIterator_t tIt ( m_pSegments[iSeg], m_iStride, false, NULL );
				for ( ;; )
				{
					const CSphRowitem * pRow = tIt.GetNextAliveRow();
					if ( !pRow )
						break;

					tMatch.m_iDocID = DOCINFO2ID(pRow);
					tMatch.m_pStatic = DOCINFO2ATTRS(pRow); // FIXME! overrides
					ARRAY_FOREACH ( j, m_dDynamize )
						tMatch.SetAttr ( m_dDynamize[j].m_tTo, tMatch.GetAttr ( m_dDynamize[j].m_tFrom ) );

					tCtx.CalcFilter ( tMatch );
					if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
						continue;

					tCtx.CalcSort ( tMatch );
					tCtx.CalcFinal ( tMatch ); // OPTIMIZE? could be possibly done later

					// storing segment in matches tag for finding strings attrs offset later, biased against default zero
					tMatch.m_iTag = iSeg+1;

					bool bNewMatch = false;
					ARRAY_FOREACH ( iSorter, dSorters )
						bNewMatch |= dSorters[iSorter]->Push ( tMatch );

					// handle cutoff
					if ( bNewMatch )
						if ( --iCutoff==0 )
							break;
				}

				if ( iCutoff==0 )
					break;
			}

		} else
		{
			// query matching
			ARRAY_FOREACH ( iSeg, m_pSegments )
			{
				tTermSetup.m_pSeg = m_pSegments[iSeg];
				pRanker->Reset ( tTermSetup );

				// for lookups to work
				tCtx.m_pIndexData = m_pSegments[iSeg];

				// set string pool for string on_sort expression fix up
				tCtx.SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
				tCtx.SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				ARRAY_FOREACH ( i, dSorters )
				{
					dSorters[i]->SetStringPool ( m_pSegments[iSeg]->m_dStrings.Begin() );
					dSorters[i]->SetMVAPool ( m_pSegments[iSeg]->m_dMvas.Begin() );
				}

				CSphMatch * pMatch = pRanker->GetMatchesBuffer();
				for ( ;; )
				{
					int iMatches = pRanker->GetMatches();
					if ( iMatches<=0 )
						break;

					for ( int i=0; i<iMatches; i++ )
					{
						assert ( !tCtx.m_bLookupSort || FindDocinfo ( m_pSegments[iSeg], pMatch[i].m_iDocID ) );

						if ( tCtx.m_bLookupSort )
							CopyDocinfo ( pMatch[i], FindDocinfo ( m_pSegments[iSeg], pMatch[i].m_iDocID ) );

						ARRAY_FOREACH ( j, m_dDynamize )
							pMatch[i].SetAttr ( m_dDynamize[j].m_tTo, pMatch[i].GetAttr ( m_dDynamize[j].m_tFrom ) );

						tCtx.CalcSort ( pMatch[i] );
						tCtx.CalcFinal ( pMatch[i] ); // OPTIMIZE? could be possibly done later

						if ( bRandomize )
							pMatch[i].m_iWeight = ( sphRand() & 0xffff );

						if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( pMatch[i] ) )
							continue;

						// storing segment in matches tag for finding strings attrs offset later, biased against default zero
						pMatch[i].m_iTag = iSeg+1;

						bool bNewMatch = false;
						ARRAY_FOREACH ( iSorter, dSorters )
							bNewMatch |= dSorters[iSorter]->Push ( pMatch[i] );

						if ( bNewMatch )
							if ( --iCutoff==0 )
								break;
					}

					if ( iCutoff==0 )
					{
						iSeg = m_pSegments.GetLength();
						break;
					}
				}
			}
		}
	}

	//////////////////////
	// coping match's attributes to external storage in result set
	//////////////////////

	const int iSegmentsTotal = m_pSegments.GetLength();
	const int iStaticSize = m_tSchema.GetStaticSize();
	if ( iStaticSize>0 && iSegmentsTotal>0 )
	{
		MEMORY ( SPH_MEM_IDX_RT_RES_MATCHES );

		// we need to count matches for allocating arena
		// as we are going to fix match's m_pStatic pointers later
		// and copy real match's data to arena

		int iFixupCount = 0;

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];

			const int iMatchesCount = pSorter->GetLength();
			const CSphMatch * pMatches = pSorter->First();

			for ( int i=0; i<iMatchesCount; i++ )
			{
				const int iMatchSegment = pMatches[i].m_iTag-1;
				if ( iMatchSegment>=0 && iMatchSegment< iSegmentsTotal )
					iFixupCount++;
			}
		}

		if ( iFixupCount>0 )
		{
			CSphRowitem * pAttr = new CSphRowitem [ iFixupCount * iStaticSize ];
			pResult->m_dStorage2Free.Add ( pAttr );
#ifndef NDEBUG
			CSphRowitem * pEnd = pAttr + iFixupCount * iStaticSize;
#endif

			ARRAY_FOREACH ( iSorter, dSorters )
			{
				ISphMatchSorter * pSorter = dSorters[iSorter];

				const int iMatchesCount = pSorter->GetLength();
				CSphMatch * pMatches = pSorter->First();

				for ( int i=0; i<iMatchesCount; i++ )
				{
					const int iMatchSegment = pMatches[i].m_iTag-1;
					if ( iMatchSegment>=0 && iMatchSegment< iSegmentsTotal )
					{
						assert ( pAttr+iStaticSize<=pEnd );

						memcpy ( pAttr, pMatches[i].m_pStatic, sizeof(CSphRowitem)*iStaticSize );
						pMatches[i].m_pStatic = pAttr;
						pAttr += iStaticSize;
					}
				}
			}
		}
	}


	//////////////////////
	// fixing string offset and data in resulting matches
	//////////////////////

	MEMORY ( SPH_MEM_IDX_RT_RES_STRINGS );

	CSphVector<CSphAttrLocator> dStringGetLoc;
	CSphVector<CSphAttrLocator> dStringSetLoc;
	CSphVector<CSphAttrLocator> dMvaGetLoc;
	CSphVector<CSphAttrLocator> dMvaSetLoc;
	for ( int i=0; i<pResult->m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tSetInfo = pResult->m_tSchema.GetAttr(i);
		if ( tSetInfo.m_eAttrType==SPH_ATTR_STRING )
		{
			const int iInLocator = m_tSchema.GetAttrIndex ( tSetInfo.m_sName.cstr() );
			assert ( iInLocator>=0 );

			dStringGetLoc.Add ( m_tSchema.GetAttr ( iInLocator ).m_tLocator );
			dStringSetLoc.Add ( tSetInfo.m_tLocator );
		} else if ( tSetInfo.m_eAttrType==SPH_ATTR_UINT32SET )
		{
			const int iInLocator = m_tSchema.GetAttrIndex ( tSetInfo.m_sName.cstr() );
			assert ( iInLocator>=0 );

			dMvaGetLoc.Add ( m_tSchema.GetAttr ( iInLocator ).m_tLocator );
			dMvaSetLoc.Add ( tSetInfo.m_tLocator );
		}

		assert ( ( tSetInfo.m_eAttrType!=SPH_ATTR_STRING && tSetInfo.m_eAttrType!=SPH_ATTR_UINT32SET ) ||
			( ( tSetInfo.m_eAttrType==SPH_ATTR_STRING || tSetInfo.m_eAttrType==SPH_ATTR_UINT32SET ) && tSetInfo.m_tLocator.m_bDynamic ) );
	}
	if ( dStringGetLoc.GetLength() || dMvaGetLoc.GetLength() )
	{
		assert ( !pResult->m_pStrings && !pResult->m_pMva );
		CSphTightVector<BYTE> dStorageString;
		CSphTightVector<DWORD> dStorageMva;
		dStorageString.Add ( 0 );
		dStorageMva.Add ( 0 );

		ARRAY_FOREACH ( iSorter, dSorters )
		{
			ISphMatchSorter * pSorter = dSorters[iSorter];

			const int iMatchesCount = pSorter->GetLength();
			CSphMatch * pMatches = pSorter->First();

			for ( int i=0; i<iMatchesCount; i++ )
			{
				CSphMatch & tMatch = pMatches[i];

				const int iSegCount = m_pSegments.GetLength();
				assert ( tMatch.m_iTag>=1 && tMatch.m_iTag<iSegCount+dDiskStrings.GetLength()+1 );
				assert ( tMatch.m_pDynamic );

				const int iStorageSrc = tMatch.m_iTag-1;
				bool bSegmentMatch = ( iStorageSrc < iSegCount );
				const BYTE * pBaseString = bSegmentMatch ? m_pSegments[iStorageSrc]->m_dStrings.Begin() : dDiskStrings[ iStorageSrc-iSegCount ];
				const DWORD * pBaseMva = bSegmentMatch ? m_pSegments[iStorageSrc]->m_dMvas.Begin() : dDiskMva[ iStorageSrc-iSegCount ];

				ARRAY_FOREACH ( i, dStringGetLoc )
				{
					const SphAttr_t uOff = tMatch.GetAttr ( dStringGetLoc[i] );
					if ( uOff>0 ) // have to fix up only existed attribute
					{
						assert ( uOff<( I64C(1)<<32 ) ); // should be 32 bit offset
						assert ( !bSegmentMatch || (int)uOff<m_pSegments[iStorageSrc]->m_dStrings.GetLength() );

						DWORD uAttr = CopyPackedString ( pBaseString + uOff, dStorageString );
						tMatch.SetAttr ( dStringSetLoc[i], uAttr );

					} else
					{
						tMatch.SetAttr ( dStringSetLoc[i], 0 );
					}
				}

				bool bIdSet = false;
				ARRAY_FOREACH ( i, dMvaGetLoc )
				{
					const SphAttr_t uOff = tMatch.GetAttr ( dMvaGetLoc[i] );
					if ( uOff>0 ) // have to fix up only existed attribute
					{
						assert ( uOff<( I64C(1)<<32 ) ); // should be 32 bit offset
						assert ( !bSegmentMatch || (int)uOff<m_pSegments[iStorageSrc]->m_dMvas.GetLength() );

						if ( !bIdSet )
						{
							CopyDocid ( tMatch.m_iDocID, dStorageMva );
							bIdSet = true;
						}

						DWORD uAttr = CopyMva ( pBaseMva + uOff, dStorageMva );
						tMatch.SetAttr ( dMvaSetLoc[i], uAttr );
					} else
					{
						tMatch.SetAttr ( dMvaSetLoc[i], 0 );
					}
				}
			}
		}

		if ( dStorageString.GetLength()>1 )
		{
			BYTE * pStrings = dStorageString.LeakData ();
			pResult->m_dStorage2Free.Add ( pStrings );
			pResult->m_pStrings = pStrings;
		}
		if ( dStorageMva.GetLength()>1 )
		{
			DWORD * pMva = dStorageMva.LeakData();
			pResult->m_dStorage2Free.Add ( pMva );
			pResult->m_pMva = pMva;
		}
	}

	// query timer
	pResult->m_iQueryTime = int ( ( sphMicroTimer()-tmQueryStart )/1000 );
	m_tRwlock.Unlock ();
	return true;
}

bool RtIndex_t::MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag ) const
{
	// FIXME! OPTIMIZE! implement common subtree cache here
	bool bResult = false;
	for ( int i=0; i<iQueries; i++ )
		if ( MultiQuery ( &ppQueries[i], ppResults[i], 1, &ppSorters[i], pExtraFilters, iTag ) )
			bResult = true;
		else
			ppResults[i]->m_iMultiplier = -1;

	return bResult;
}

bool RtIndex_t::GetKeywords ( CSphVector<CSphKeywordInfo> & dKeywords, const char * sQuery, bool bGetStats, CSphString & sError ) const
{
	m_tRwlock.ReadLock(); // this is actually needed only if they want stats

	RtQword_t tQword;
	CSphString sBuffer ( sQuery );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( m_pTokenizer->Clone ( false ) ); // avoid race
	pTokenizer->SetBuffer ( (BYTE *)sBuffer.cstr(), sBuffer.Length() );

	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	CSphDict * pDictBase = m_pDict;
	if ( pDictBase->HasState() )
	{
		tDictCloned = pDictBase = pDictBase->Clone();
	}

	while ( BYTE * pToken = pTokenizer->GetToken() )
	{
		const char * sToken = (const char *)pToken;
		CSphString sWord ( sToken );
		SphWordID_t iWord = pDictBase->GetWordID ( pToken );
		if ( iWord )
		{
			CSphKeywordInfo & tInfo = dKeywords.Add();
			tInfo.m_sTokenized = sWord;
			tInfo.m_sNormalized = sToken;
			tInfo.m_iDocs = 0;
			tInfo.m_iHits = 0;

			if ( !bGetStats )
				continue;

			tQword.m_iWordID = iWord;
			tQword.m_iDocs = 0;
			tQword.m_iHits = 0;
			ARRAY_FOREACH ( iSeg, m_pSegments )
				RtQwordSetupSegment ( &tQword, m_pSegments[iSeg], false );

			tInfo.m_iDocs = tQword.m_iDocs;
			tInfo.m_iHits = tQword.m_iHits;
		}
	}

	// get stats from disk chunks too
	if ( bGetStats )
		ARRAY_FOREACH ( iChunk, m_pDiskChunks )
	{
		CSphVector<CSphKeywordInfo> dKeywords2;
		if ( !m_pDiskChunks[iChunk]->GetKeywords ( dKeywords2, sQuery, bGetStats, sError ) )
		{
			m_tRwlock.Unlock();
			return false;
		}

		if ( dKeywords.GetLength()!=dKeywords2.GetLength() )
		{
			sError.SetSprintf ( "INTERNAL ERROR: keyword count mismatch (ram=%d, disk[%d]=%d)",
				dKeywords.GetLength(), iChunk, dKeywords2.GetLength() );
			m_tRwlock.Unlock ();
			break;
		}

		ARRAY_FOREACH ( i, dKeywords )
		{
			if ( dKeywords[i].m_sTokenized!=dKeywords2[i].m_sTokenized )
			{
				sError.SetSprintf ( "INTERNAL ERROR: tokenized keyword mismatch (n=%d, ram=%s, disk[%d]=%s)",
					i, dKeywords[i].m_sTokenized.cstr(), iChunk, dKeywords2[i].m_sTokenized.cstr() );
				m_tRwlock.Unlock ();
				break;
			}

			if ( dKeywords[i].m_sNormalized!=dKeywords2[i].m_sNormalized )
			{
				sError.SetSprintf ( "INTERNAL ERROR: normalized keyword mismatch (n=%d, ram=%s, disk[%d]=%s)",
					i, dKeywords[i].m_sTokenized.cstr(), iChunk, dKeywords2[i].m_sTokenized.cstr() );
				m_tRwlock.Unlock ();
				break;
			}

			dKeywords[i].m_iDocs += dKeywords2[i].m_iDocs;
			dKeywords[i].m_iHits += dKeywords2[i].m_iHits;
		}
	}

	m_tRwlock.Unlock();
	return true;
}

// FIXME! might be inconsistent in case disk chunk update fails
int RtIndex_t::UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError )
{
	// check if we have to
	assert ( tUpd.m_dDocids.GetLength()==tUpd.m_dRowOffset.GetLength() );
	if ( !tUpd.m_dDocids.GetLength() )
		return 0;

	// remap update schema to index schema
	CSphVector<CSphAttrLocator> dLocators;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		int iIndex = m_tSchema.GetAttrIndex ( tUpd.m_dAttrs[i].m_sName.cstr() );
		if ( iIndex<0 )
		{
			sError.SetSprintf ( "attribute '%s' not found", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}

		// forbid updates on non-int columns
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(iIndex);
		if (!( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP ))
		{
			sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, or timestamp)", tUpd.m_dAttrs[i].m_sName.cstr() );
			return -1;
		}

		dLocators.Add ( tCol.m_tLocator );
	}
	assert ( dLocators.GetLength()==tUpd.m_dAttrs.GetLength() );

	// get that lock
	m_tRwlock.WriteLock();

	// check if we are empty
	if ( !m_pSegments.GetLength() && !m_pDiskChunks.GetLength() )
	{
		m_tRwlock.Unlock();
		return true;
	}

	// do the update
	int iUpdated = 0;
	DWORD uUpdateMask = 0;

	int iFirst = ( iIndex<0 ) ? 0 : iIndex;
	int iLast = ( iIndex<0 ) ? tUpd.m_dDocids.GetLength() : iIndex+1;
	for ( int iUpd=iFirst; iUpd<iLast; iUpd++ )
	{
		// search segments first
		bool bUpdated = false;
		ARRAY_FOREACH ( iSeg, m_pSegments )
		{
			CSphRowitem * pRow = const_cast<CSphRowitem*> ( m_pSegments[iSeg]->FindAliveRow ( tUpd.m_dDocids[iUpd] ) );
			if ( !pRow )
				continue;

			assert ( DOCINFO2ID(pRow)==tUpd.m_dDocids[iUpd] );
			pRow = DOCINFO2ATTRS(pRow);

			int iPos = tUpd.m_dRowOffset[iUpd];
			ARRAY_FOREACH ( iCol, tUpd.m_dAttrs )
			{
				// plain update
				SphAttr_t uValue = tUpd.m_dPool[iPos];
				sphSetRowAttr ( pRow, dLocators[iCol], uValue );

				iPos++;
				uUpdateMask |= ATTRS_UPDATED;
			}

			bUpdated = true;
			iUpdated++;
		}
		if ( bUpdated )
			continue;

		// check disk K-list now
		// FIXME! optimize away flush
		m_tKlist.Flush();
		m_tKlist.KillListLock();
		const SphAttr_t uRef = tUpd.m_dDocids[iUpd];
		bUpdated = ( sphBinarySearch ( m_tKlist.GetKillList(), m_tKlist.GetKillList() + m_tKlist.GetKillListSize() - 1, uRef )!=NULL );
		m_tKlist.KillListUnlock();
		if ( bUpdated )
			continue;

		// finally, try disk chunks
		for ( int iChunk = m_pDiskChunks.GetLength()-1; iChunk>=0; iChunk-- )
		{
			// run just this update
			// FIXME! might be inefficient in case of big batches (redundant allocs in disk update)
			int iRes = m_pDiskChunks[iChunk]->UpdateAttributes ( tUpd, iUpd, sError );

			// errors are highly unlikely at this point
			// FIXME! maybe emit a warning to client as well?
			if ( iRes<0 )
			{
				sphWarn ( "INTERNAL ERROR: index %s chunk %d update failure: %s", m_sIndexName.cstr(), iChunk, sError.cstr() );
				continue;
			}

			// update stats
			iUpdated += iRes;

			// we only need to update the most fresh chunk
			if ( iRes>0 )
				break;
		}
	}

	// bump the counter, binlog the update!
	assert ( iIndex<0 );
	g_pBinlog->BinlogUpdateAttributes ( m_sIndexName.cstr(), ++m_iTID, tUpd );

	// all done
	m_uAttrsStatus |= uUpdateMask;
	m_tRwlock.Unlock ();
	return iUpdated;
}

//////////////////////////////////////////////////////////////////////////
// BINLOG
//////////////////////////////////////////////////////////////////////////

extern DWORD g_dSphinxCRC32 [ 256 ];


static CSphString MakeBinlogName ( const char * sPath, int iExt )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.%03d", sPath, iExt );
	return sName;
}


BinlogWriter_c::BinlogWriter_c ()
{
	m_iLastWritePos = 0;
	m_iLastFsyncPos = 0;
	ResetCrc();
}


void BinlogWriter_c::ResetCrc ()
{
	m_uCRC = ~((DWORD)0);
}


void BinlogWriter_c::PutBytes ( const void * pData, int iSize )
{
	BYTE * b = (BYTE*) pData;
	for ( int i=0; i<iSize; i++ )
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ *b++) & 0xff ];
	CSphWriter::PutBytes ( pData, iSize );
}


void BinlogWriter_c::PutString ( const char * szString )
{
	int iLen = szString ? strlen ( szString ) : 0;
	ZipValue ( iLen );
	if ( iLen )
		PutBytes ( szString, iLen );
}


void BinlogWriter_c::ZipValue ( uint64_t uValue )
{
	BYTE uBuf[16];
	int iLen = 0;

	while ( uValue>=0x80 )
	{
		uBuf[iLen++] = (BYTE)( 0x80 | ( uValue & 0x7f ) );
		uValue >>= 7;
	}
	uBuf[iLen++] = (BYTE)uValue;

	PutBytes ( uBuf, iLen );
}


void BinlogWriter_c::WriteCrc ()
{
	m_uCRC = ~m_uCRC;
	CSphWriter::PutDword ( m_uCRC );
	m_uCRC = ~((DWORD)0);
}


void BinlogWriter_c::Flush ()
{
	Write();
	Fsync();
}


void BinlogWriter_c::Write ()
{
	if ( m_iPoolUsed<=0 )
		return;

	CSphWriter::Flush();
	m_iLastWritePos = GetPos();
}


#if USE_WINDOWS
int fsync ( int iFD )
{
	// map fd to handle
	HANDLE h = (HANDLE) _get_osfhandle ( iFD );
	if ( h==INVALID_HANDLE_VALUE )
	{
		errno = EBADF;
		return -1;
	}

	// do flush
	if ( FlushFileBuffers(h) )
		return 0;

	// error handling
	errno = EIO;
	if ( GetLastError()==ERROR_INVALID_HANDLE )
		errno = EINVAL;
	return -1;
}
#endif


void BinlogWriter_c::Fsync ()
{
	if ( !HasUnsyncedData() )
		return;

	m_bError = ( fsync ( m_iFD )!=0 );
	if ( m_bError && m_pError )
		m_pError->SetSprintf ( "failed to sync %s: %s" , m_sName.cstr(), strerror(errno) );

	m_iLastFsyncPos = GetPos();
}

//////////////////////////////////////////////////////////////////////////

void BinlogReader_c::ResetCrc ()
{
	m_uCRC = ~(DWORD(0));
}


void BinlogReader_c::GetBytes ( void * pData, int iSize )
{
	CSphReader::GetBytes ( pData, iSize );
	BYTE * b = (BYTE*) pData;
	for ( int i=0; i<iSize; i++ )
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ *b++) & 0xff ];
}


DWORD BinlogReader_c::GetDword ()
{
	DWORD uRes;
	GetBytes ( &uRes, sizeof(DWORD) );
	return uRes;
}


CSphString BinlogReader_c::GetString ()
{
	CSphString sRes;
	int iLen = (int) UnzipValue();
	if ( iLen )
	{
		sRes.Reserve ( iLen );
		GetBytes ( (BYTE*)sRes.cstr(), iLen );
	}
	return sRes;
}


uint64_t BinlogReader_c::UnzipValue ()
{
	uint64_t uRes = 0;
	int iOff = 0, iByte;
	do
	{
		iByte = CSphReader::GetByte();
		uRes += ( (uint64_t)( iByte & 0x7f ) << iOff );
		iOff += 7;
		m_uCRC = (m_uCRC >> 8) ^ g_dSphinxCRC32 [ (m_uCRC ^ (BYTE)iByte) & 0xff ];
	} while ( iByte>=128 );

	return uRes;
}


bool BinlogReader_c::CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos )
{
	DWORD uRef = CSphAutoreader::GetDword();
	m_uCRC = ~m_uCRC;
	if ( uRef!=m_uCRC )
		sphWarning ( "binlog: %s: CRC mismatch (index=%s, tid="INT64_FMT", pos="INT64_FMT")", sOp, sIndexName ? sIndexName : "", iTid, iTxnPos );
	return uRef==m_uCRC;
}

//////////////////////////////////////////////////////////////////////////

RtBinlog_c::RtBinlog_c ()
	: m_iFlushTimeLeft ( 0 )
	, m_iFlushPeriod ( BINLOG_AUTO_FLUSH )
	, m_eOnCommit ( ACTION_NONE )
	, m_iLockFD ( -1 )
	, m_bReplayMode ( false )
	, m_bDisabled ( true )
	, m_iRestartSize ( 0 )
{
	MEMORY ( SPH_MEM_BINLOG );

	Verify ( m_tWriteLock.Init() );

	m_tWriter.SetBufferSize ( BINLOG_WRITE_BUFFER );
}

RtBinlog_c::~RtBinlog_c ()
{
	if ( !m_bDisabled )
	{
		m_iFlushPeriod = 0;
		if ( m_eOnCommit!=ACTION_FSYNC )
			sphThreadJoin ( &m_tUpdateTread );

		DoCacheWrite();
		m_tWriter.CloseFile();
		LockFile ( false );
	}

	Verify ( m_tWriteLock.Done() );
}


void RtBinlog_c::BinlogCommit ( const char * sIndexName, int64_t iTID, const RtSegment_t * pSeg, const CSphVector<SphDocID_t> & dKlist )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_COMMIT );
	m_tWriter.ZipValue ( uIndex );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );

	// save txn data
	if ( !pSeg || !pSeg->m_iRows )
	{
		m_tWriter.ZipValue ( 0 );
	} else
	{
		m_tWriter.ZipValue ( pSeg->m_iRows );
		SaveVector ( m_tWriter, pSeg->m_dWords );
		m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints.GetLength() );
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_iOffset );
			m_tWriter.ZipValue ( pSeg->m_dWordCheckpoints[i].m_uWordID );
		}
		SaveVector ( m_tWriter, pSeg->m_dDocs );
		SaveVector ( m_tWriter, pSeg->m_dHits );
		SaveVector ( m_tWriter, pSeg->m_dRows );
		SaveVector ( m_tWriter, pSeg->m_dStrings );
		SaveVector ( m_tWriter, pSeg->m_dMvas );
	}
	SaveVector ( m_tWriter, dKlist );

	// checksum
	m_tWriter.WriteCrc ();

	// finalize
	CheckDoFlush();
	CheckDoRestart();
	Verify ( m_tWriteLock.Unlock() );
}

void RtBinlog_c::BinlogUpdateAttributes ( const char * sIndexName, int64_t iTID, const CSphAttrUpdate & tUpd )
{
	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	Verify ( m_tWriteLock.Lock() );

	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( sIndexName, iTID, tmNow );

	// header
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_UPDATE_ATTRS );
	m_tWriter.ZipValue ( uIndex );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );

	// update data
	m_tWriter.ZipValue ( tUpd.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		m_tWriter.PutString ( tUpd.m_dAttrs[i].m_sName.cstr() );
		m_tWriter.ZipValue ( tUpd.m_dAttrs[i].m_eAttrType );
	}

	// POD vectors
	SaveVector ( m_tWriter, tUpd.m_dPool );
	SaveVector ( m_tWriter, tUpd.m_dDocids );
	SaveVector ( m_tWriter, tUpd.m_dRowOffset );

	// checksum
	m_tWriter.WriteCrc ();

	// finalize
	CheckDoFlush();
	CheckDoRestart();
	Verify ( m_tWriteLock.Unlock() );
}

// here's been going binlogs with ALL closed indices removing
void RtBinlog_c::NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown )
{
	if ( m_bReplayMode )
		sphInfo ( "index '%s': ramchunk saved. TID="INT64_FMT"", sIndexName, iTID );

	if ( m_bReplayMode || m_bDisabled )
		return;

	MEMORY ( SPH_MEM_BINLOG );
	assert ( bShutdown || m_dLogFiles.GetLength() );

	Verify ( m_tWriteLock.Lock() );

	bool bCurrentLogShut = false;
	const int iPreflushFiles = m_dLogFiles.GetLength();

	// loop through all log files, and check if we can unlink any
	ARRAY_FOREACH ( iLog, m_dLogFiles )
	{
		BinlogFileDesc_t & tLog = m_dLogFiles[iLog];
		bool bUsed = false;

		// update index info for this log file
		ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
		{
			BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

			// this index was just flushed, update flushed TID
			if ( tIndex.m_sName==sIndexName )
			{
				assert ( iTID>=tIndex.m_iFlushedTID );
				tIndex.m_iFlushedTID = Max ( tIndex.m_iFlushedTID, iTID );
			}

			// if max logged TID is greater than last flushed TID, log file still has needed recovery data
			if ( tIndex.m_iFlushedTID < tIndex.m_iMaxTID )
				bUsed = true;
		}

		// it's needed, keep looking
		if ( bUsed )
			continue;

		// hooray, we can remove this log!
		// if this is our current log, we have to close it first
		if ( iLog==m_dLogFiles.GetLength()-1 )
		{
			m_tWriter.CloseFile ();
			bCurrentLogShut = true;
		}

		// do unlink
		CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );
		if ( ::unlink ( sLog.cstr() ) )
			sphWarning ( "binlog: failed to unlink %s: %s (remove it manually)", sLog.cstr(), strerror(errno) );

		// we need to reset it, otherwise there might be leftover data after last Remove()
		m_dLogFiles[iLog] = BinlogFileDesc_t();
		// quit tracking it
		m_dLogFiles.Remove ( iLog-- );
	}

	if ( bCurrentLogShut && !bShutdown )
	{
		// if current log was closed, we need a new one (it will automatically save meta, too)
		OpenNewLog ();

	} else if ( iPreflushFiles!=m_dLogFiles.GetLength() )
	{
		// if we unlinked any logs, we need to save meta, too
		SaveMeta ();
	}

	Verify ( m_tWriteLock.Unlock() );
}

void RtBinlog_c::Configure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	MEMORY ( SPH_MEM_BINLOG );

	const int iMode = hSearchd.GetInt ( "binlog_flush", 2 );
	switch ( iMode )
	{
		case 0:		m_eOnCommit = ACTION_NONE; break;
		case 1:		m_eOnCommit = ACTION_FSYNC; break;
		case 2:		m_eOnCommit = ACTION_WRITE; break;
		default:	sphDie ( "unknown binlog flush mode %d (must be 0, 1, or 2)\n", iMode );
	}

#ifndef DATADIR
#define DATADIR "."
#endif

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : DATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	m_iRestartSize = hSearchd.GetSize ( "binlog_max_log_size", m_iRestartSize );

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LoadMeta();
	}
}

void RtBinlog_c::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback )
{
	if ( m_bDisabled || !hIndexes.GetLength() )
		return;

	// on replay started
	if ( pfnProgressCallback )
		pfnProgressCallback();

	int64_t tmReplay = sphMicroTimer();
	// do replay
	m_bReplayMode = true;
	int iLastLogState = 0;
	ARRAY_FOREACH ( i, m_dLogFiles )
	{
		iLastLogState = ReplayBinlog ( hIndexes, i );
		if ( pfnProgressCallback ) // on each replayed binlog
			pfnProgressCallback();
	}

	if ( m_dLogFiles.GetLength()>0 )
	{
		tmReplay = sphMicroTimer() - tmReplay;
		sphInfo ( "binlog: finished replaying total %d in %d.%03d sec",
			m_dLogFiles.GetLength(),
			(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );
	}

	// FIXME?
	// in some cases, indexes might had been flushed during replay
	// and we might therefore want to update m_iFlushedTID everywhere
	// but for now, let's just wait until next flush for simplicity

	// resume normal operation
	m_bReplayMode = false;
	OpenNewLog ( iLastLogState );
}

void RtBinlog_c::CreateTimerThread ()
{
	if ( !m_bDisabled && m_eOnCommit!=ACTION_FSYNC )
	{
		m_iFlushTimeLeft = sphMicroTimer() + m_iFlushPeriod;
		sphThreadCreate ( &m_tUpdateTread, RtBinlog_c::DoAutoFlush, this );
	}
}

void RtBinlog_c::DoAutoFlush ( void * pBinlog )
{
	assert ( pBinlog );
	RtBinlog_c * pLog = (RtBinlog_c *)pBinlog;
	assert ( !pLog->m_bDisabled );

	while ( pLog->m_iFlushPeriod>0 )
	{
		if ( pLog->m_iFlushTimeLeft < sphMicroTimer() )
		{
			MEMORY ( SPH_MEM_BINLOG );

			pLog->m_iFlushTimeLeft = sphMicroTimer() + pLog->m_iFlushPeriod;

			if ( pLog->m_eOnCommit==ACTION_NONE || pLog->m_tWriter.HasUnwrittenData() )
			{
				Verify ( pLog->m_tWriteLock.Lock() );
				pLog->m_tWriter.Flush();
				Verify ( pLog->m_tWriteLock.Unlock() );
			}

			if ( pLog->m_tWriter.HasUnsyncedData() )
				pLog->m_tWriter.Fsync();
		}

		// sleep N msec before next iter or terminate because of shutdown
		sphSleepMsec ( 100 );
	}
}

int RtBinlog_c::GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow )
{
	MEMORY ( SPH_MEM_BINLOG );
	assert ( m_dLogFiles.GetLength() );

	// OPTIMIZE? maybe hash them?
	BinlogFileDesc_t & tLog = m_dLogFiles.Last();
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( tIndex.m_sName==sName )
		{
			tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
			tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmNow );
			return i;
		}
	}

	// create a new entry
	int iID = tLog.m_dIndexInfos.GetLength();
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add(); // caller must hold a wlock
	tIndex.m_sName = sName;
	tIndex.m_iMinTID = iTID;
	tIndex.m_iMaxTID = iTID;
	tIndex.m_iFlushedTID = 0;
	tIndex.m_tmMin = tmNow;
	tIndex.m_tmMax = tmNow;

	// log this new entry
	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_ADD_INDEX );
	m_tWriter.ZipValue ( iID );
	m_tWriter.PutString ( sName );
	m_tWriter.ZipValue ( iTID );
	m_tWriter.ZipValue ( tmNow );
	m_tWriter.WriteCrc ();

	// return the index
	return iID;
}

void RtBinlog_c::LoadMeta ()
{
	MEMORY ( SPH_MEM_BINLOG );

	CSphString sMeta;
	sMeta.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return;

	CSphString sError;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, sError ) )
		sphDie ( "%s error: %s", sMeta.cstr(), sError.cstr() );

	if ( rdMeta.GetDword()!=BINLOG_META_MAGIC )
		sphDie ( "invalid meta file %s", sMeta.cstr() );

	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion!=BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version", sMeta.cstr(), uVersion, BINLOG_VERSION );

	const bool bLoaded64bit = ( rdMeta.GetByte()==1 );
	if ( bLoaded64bit!=USE_64BIT )
		sphDie ( "USE_64BIT inconsistency (binary=%d, binlog=%d); recovery requires previous binary version", USE_64BIT, bLoaded64bit );

	// load list of active log files
	m_dLogFiles.Resize ( rdMeta.UnzipInt() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, m_dLogFiles )
		m_dLogFiles[i].m_iExt = rdMeta.UnzipInt(); // everything else is saved in logs themselves
}

void RtBinlog_c::SaveMeta ()
{
	MEMORY ( SPH_MEM_BINLOG );

	CSphString sMeta, sMetaOld;
	sMeta.SetSprintf ( "%s/binlog.meta.new", m_sLogPath.cstr() );
	sMetaOld.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );

	CSphString sError;

	// opened and locked, lets write
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMeta, sError ) )
		sphDie ( "failed to open '%s': '%s'", sMeta.cstr(), sError.cstr() );

	wrMeta.PutDword ( BINLOG_META_MAGIC );
	wrMeta.PutDword ( BINLOG_VERSION );
	wrMeta.PutByte ( USE_64BIT );

	// save list of active log files
	wrMeta.ZipInt ( m_dLogFiles.GetLength() );
	ARRAY_FOREACH ( i, m_dLogFiles )
		wrMeta.ZipInt ( m_dLogFiles[i].m_iExt ); // everything else is saved in logs themselves

	wrMeta.CloseFile();

	if ( ::rename ( sMeta.cstr(), sMetaOld.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMeta.cstr(), sMetaOld.cstr(), errno, strerror(errno) ); // !COMMIT handle this gracefully
	sphLogDebug ( "SaveMeta: Done." );
}

void RtBinlog_c::LockFile ( bool bLock )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.lock", m_sLogPath.cstr() );

	if ( bLock )
	{
		assert ( m_iLockFD==-1 );
		const int iLockFD = ::open ( sName.cstr(), SPH_O_NEW, 0644 );

		if ( iLockFD<0 )
			sphDie ( "failed to open '%s': %u '%s'", sName.cstr(), errno, strerror(errno) );

		if ( !sphLockEx ( iLockFD, false ) )
			sphDie ( "failed to lock '%s': %u '%s'", sName.cstr(), errno, strerror(errno) );

		m_iLockFD = iLockFD;
	} else
	{
		SafeClose ( m_iLockFD );
		::unlink ( sName.cstr()	);
	}
}

void RtBinlog_c::OpenNewLog ( int iLastState )
{
	MEMORY ( SPH_MEM_BINLOG );

	// calc new ext
	int iExt = 1;
	if ( m_dLogFiles.GetLength() )
	{
		iExt = m_dLogFiles.Last().m_iExt;
		if ( !iLastState )
			iExt++;
	}

	// create entry
	// we need to reset it, otherwise there might be leftover data after last Remove()
	BinlogFileDesc_t tLog;
	tLog.m_iExt = iExt;
	m_dLogFiles.Add ( tLog );

	// create file
	CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );

	if ( !iLastState ) // reuse the last binlog since it is empty or useless.
		::unlink ( sLog.cstr() );

	if ( !m_tWriter.OpenFile ( sLog.cstr(), m_sWriterError ) )
		sphDie ( "failed to create %s: errno=%d, error=%s", sLog.cstr(), errno, strerror(errno) );

	// emit header
	m_tWriter.PutDword ( BINLOG_HEADER_MAGIC );
	m_tWriter.PutDword ( BINLOG_VERSION );

	// update meta
	SaveMeta();
}

void RtBinlog_c::DoCacheWrite ()
{
	if ( !m_dLogFiles.GetLength() )
		return;
	const CSphVector<BinlogIndexInfo_t> & dIndexes = m_dLogFiles.Last().m_dIndexInfos;

	m_tWriter.PutDword ( BLOP_MAGIC );
	m_tWriter.ResetCrc ();

	m_tWriter.ZipValue ( BLOP_ADD_CACHE );
	m_tWriter.ZipValue ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
	{
		m_tWriter.PutString ( dIndexes[i].m_sName.cstr() );
		m_tWriter.ZipValue ( dIndexes[i].m_iMinTID );
		m_tWriter.ZipValue ( dIndexes[i].m_iMaxTID );
		m_tWriter.ZipValue ( dIndexes[i].m_iFlushedTID );
		m_tWriter.ZipValue ( dIndexes[i].m_tmMin );
		m_tWriter.ZipValue ( dIndexes[i].m_tmMax );
	}
	m_tWriter.WriteCrc ();
}

void RtBinlog_c::CheckDoRestart ()
{
	// restart on exceed file size limit
	if ( m_iRestartSize>0 && m_tWriter.GetPos()>m_iRestartSize )
	{
		MEMORY ( SPH_MEM_BINLOG );

		assert ( m_dLogFiles.GetLength() );

		DoCacheWrite();
		m_tWriter.CloseFile();
		OpenNewLog();
	}
}

void RtBinlog_c::CheckDoFlush ()
{
	if ( m_eOnCommit==ACTION_NONE )
		return;

	if ( m_eOnCommit==ACTION_WRITE && m_tWriter.HasUnwrittenData() )
		m_tWriter.Write();

	if ( m_eOnCommit==ACTION_FSYNC && m_tWriter.HasUnsyncedData() )
	{
		if ( m_tWriter.HasUnwrittenData() )
			m_tWriter.Write();

		m_tWriter.Fsync();
	}
}

int RtBinlog_c::ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, int iBinlog )
{
	assert ( iBinlog>=0 && iBinlog<m_dLogFiles.GetLength() );
	CSphString sError;

	const CSphString sLog ( MakeBinlogName ( m_sLogPath.cstr(), m_dLogFiles[iBinlog].m_iExt ) );
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// open, check, play
	sphInfo ( "binlog: replaying log %s", sLog.cstr() );

	BinlogReader_c tReader;
	if ( !tReader.Open ( sLog, sError ) )
		sphDie ( "binlog: log open error: %s", sError.cstr() );

	const SphOffset_t iFileSize = tReader.GetFilesize();

	if ( !iFileSize )
	{
		sphWarning ( "binlog: empty binlog %s detected, skipping", sLog.cstr() );
		return -1;
	}

	if ( tReader.GetDword()!=BINLOG_HEADER_MAGIC )
		sphDie ( "binlog: log %s missing magic header (corrupted?)", sLog.cstr() );

	DWORD uVersion = tReader.GetDword();
	if ( uVersion!=BINLOG_VERSION || tReader.GetErrorFlag() )
		sphDie ( "binlog: log %s is v.%d, binary is v.%d; recovery requires previous binary version", sLog.cstr(), uVersion, BINLOG_VERSION );

	/////////////
	// do replay
	/////////////

	int dTotal [ BLOP_TOTAL+1 ];
	memset ( dTotal, 0, sizeof(dTotal) );

	// !COMMIT
	// instead of simply replaying everything, we should check whether this binlog is clean
	// by loading and checking the cache stored at its very end
	tLog.m_dIndexInfos.Reset();

	bool bReplayOK = true;
	bool bHaveCacheOp = false;
	int64_t iPos = -1;

	m_iReplayedRows = 0;
	int64_t tmReplay = sphMicroTimer();

	while ( iFileSize!=tReader.GetPos() && !tReader.GetErrorFlag() && bReplayOK )
	{
		iPos = tReader.GetPos();
		if ( tReader.GetDword()!=BLOP_MAGIC )
		{
			sphDie ( "binlog: log missing txn marker at pos="INT64_FMT" (corrupted?)", iPos );
			break;
		}

		tReader.ResetCrc ();
		const uint64_t uOp = tReader.UnzipValue ();

		if ( uOp<=0 || uOp>=BLOP_TOTAL )
			sphDie ( "binlog: unexpected entry (blop="UINT64_FMT", pos="INT64_FMT")", uOp, iPos );

		// FIXME! blop might be OK but skipped (eg. index that is no longer)
		switch ( uOp )
		{
			case BLOP_COMMIT:
				bReplayOK = ReplayCommit ( iBinlog, tReader );
				break;

			case BLOP_UPDATE_ATTRS:
				bReplayOK = ReplayUpdateAttributes ( iBinlog, tReader );
				break;

			case BLOP_ADD_INDEX:
				bReplayOK = ReplayIndexAdd ( iBinlog, hIndexes, tReader );
				break;

			case BLOP_ADD_CACHE:
				if ( bHaveCacheOp )
					sphDie ( "binlog: internal error, second BLOP_ADD_CACHE detected (corruption?)" );
				bHaveCacheOp = true;
				bReplayOK = ReplayCacheAdd ( iBinlog, tReader );
				break;

			default:
				sphDie ( "binlog: internal error, unhandled entry (blop=%d)", (int)uOp );
		}

		dTotal [ uOp ] += bReplayOK?1:0;
		dTotal [ BLOP_TOTAL ]++;
	}

	tmReplay = sphMicroTimer() - tmReplay;

	if ( tReader.GetErrorFlag() )
		sphWarning ( "binlog: log io error at pos="INT64_FMT": %s", iPos, sError.cstr() );

	if ( !bReplayOK )
		sphWarning ( "binlog: replay error at pos="INT64_FMT")", iPos );

	// show additional replay statistics
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		const BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( !hIndexes ( tIndex.m_sName.cstr() ) )
		{
			sphWarning ( "binlog: index %s: missing; tids "INT64_FMT" to "INT64_FMT" skipped!",
				tIndex.m_sName.cstr(), tIndex.m_iMinTID, tIndex.m_iMaxTID );

		} else if ( tIndex.m_iPreReplayTID < tIndex.m_iMaxTID )
		{
			sphInfo ( "binlog: index %s: recovered from tid "INT64_FMT" to tid "INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );

		} else
		{
			sphInfo ( "binlog: index %s: skipped at tid "INT64_FMT" and max binlog tid "INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );
		}
	}

	sphInfo ( "binlog: replay stats: %d rows in %d commits; %d updates; %d indexes",
		m_iReplayedRows, dTotal[BLOP_COMMIT], dTotal[BLOP_UPDATE_ATTRS], dTotal[BLOP_ADD_INDEX] );
	sphInfo ( "binlog: finished replaying %s; %d.%d MB in %d.%03d sec",
		sLog.cstr(),
		(int)(iFileSize/1048576), (int)((iFileSize*10/1048576)%10),
		(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );

	if ( bHaveCacheOp && dTotal[BLOP_TOTAL]==1 ) // only one operation, that is Add Cache - by the fact, empty binlog
		return 1;

	return 0;
}


static BinlogIndexInfo_t & ReplayIndexID ( BinlogReader_c & tReader, BinlogFileDesc_t & tLog, const char * sPlace )
{
	const int64_t iTxnPos = tReader.GetPos();
	const int iVal = (int)tReader.UnzipValue();

	if ( iVal<0 || iVal>=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: %s: unexpected index id (id=%d, max=%d, pos="INT64_FMT")",
			sPlace, iVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );

	return tLog.m_dIndexInfos[iVal];
}


bool RtBinlog_c::ReplayCommit ( int iBinlog, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "commit" );

	// load transaction data
	const int64_t iTID = (int64_t) tReader.UnzipValue();
	const int64_t tmStamp = (int64_t) tReader.UnzipValue();

	RtSegment_t * pSeg = NULL;
	CSphVector<SphDocID_t> dKlist;

	int iRows = (int)tReader.UnzipValue();
	if ( iRows )
	{
		pSeg = new RtSegment_t();
		pSeg->m_iRows = pSeg->m_iAliveRows = iRows;
		m_iReplayedRows += iRows;

		LoadVector ( tReader, pSeg->m_dWords );
		pSeg->m_dWordCheckpoints.Resize ( (int) tReader.UnzipValue() ); // FIXME! sanity check
		ARRAY_FOREACH ( i, pSeg->m_dWordCheckpoints )
		{
			pSeg->m_dWordCheckpoints[i].m_iOffset = (int) tReader.UnzipValue();
			pSeg->m_dWordCheckpoints[i].m_uWordID = (SphWordID_t )tReader.UnzipValue();
		}
		LoadVector ( tReader, pSeg->m_dDocs );
		LoadVector ( tReader, pSeg->m_dHits );
		LoadVector ( tReader, pSeg->m_dRows );
		LoadVector ( tReader, pSeg->m_dStrings );
		LoadVector ( tReader, pSeg->m_dMvas );
	}
	LoadVector ( tReader, dKlist );

	// checksum
	if ( tReader.GetErrorFlag() || !tReader.CheckCrc ( "commit", tIndex.m_sName.cstr(), iTID, iTxnPos ) )
		return false;

	// check TID, time order in log
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: commit: descending tid (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
	if ( tmStamp<tIndex.m_tmMax )
		sphDie ( "binlog: commit: descending time (index=%s, lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

	// only replay transaction when index exists and does not have it yet (based on TID)
	if ( tIndex.m_pRT && iTID > tIndex.m_pRT->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pRT->m_iTID+1 )
			sphWarning ( "binlog: commit: unexpected tid (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_pRT->m_iTID, iTID, iTxnPos );

		// actually replay
		tIndex.m_pRT->CommitReplayable ( pSeg, dKlist );

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pRT->m_iTID = iTID;
	}

	// update info
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
	return true;
}

bool RtBinlog_c::ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const
{
	// load and check index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	uint64_t uVal = tReader.UnzipValue();
	if ( (int)uVal!=tLog.m_dIndexInfos.GetLength() )
		sphDie ( "binlog: indexadd: unexpected index id (id="UINT64_FMT", expected=%d, pos="INT64_FMT")",
			uVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );

	// load data
	CSphString sName = tReader.GetString();

	// FIXME? use this for double checking?
	tReader.UnzipValue (); // TID
	tReader.UnzipValue (); // time

	if ( !tReader.CheckCrc ( "indexadd", sName.cstr(), 0, iTxnPos ) )
		return false;

	// check for index name dupes
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
		if ( tLog.m_dIndexInfos[i].m_sName==sName )
			sphDie ( "binlog: duplicate index name (name=%s, dupeid=%d, pos="INT64_FMT")",
				sName.cstr(), i, iTxnPos );

	// not a dupe, lets add
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add();
	tIndex.m_sName = sName;

	// lookup index in the list of currently served ones
	CSphIndex ** ppIndex = hIndexes ( sName.cstr() );
	CSphIndex * pIndex = ppIndex ? (*ppIndex) : NULL;
	if ( pIndex )
	{
		tIndex.m_pIndex = pIndex;
		if ( pIndex->IsRT() )
			tIndex.m_pRT = (RtIndex_t*)pIndex;
		tIndex.m_iPreReplayTID = pIndex->m_iTID;
		tIndex.m_iFlushedTID = pIndex->m_iTID;
	}

	// all ok
	// TID ranges will be now recomputed as we replay
	return true;
}

bool RtBinlog_c::ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	BinlogIndexInfo_t & tIndex = ReplayIndexID ( tReader, tLog, "update" );

	// load transaction data
	CSphAttrUpdate tUpd;

	int64_t iTID = (int64_t) tReader.UnzipValue();
	int64_t tmStamp = (int64_t) tReader.UnzipValue();

	tUpd.m_dAttrs.Resize ( (DWORD) tReader.UnzipValue() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		tUpd.m_dAttrs[i].m_sName = tReader.GetString();
		tUpd.m_dAttrs[i].m_eAttrType = (ESphAttr) tReader.UnzipValue(); // safe, we'll crc check later
	}
	if ( tReader.GetErrorFlag()
		|| !LoadVector ( tReader, tUpd.m_dPool )
		|| !LoadVector ( tReader, tUpd.m_dDocids )
		|| !LoadVector ( tReader, tUpd.m_dRowOffset )
		|| !tReader.CheckCrc ( "update", tIndex.m_sName.cstr(), iTID, iTxnPos ) )
	{
		return false;
	}

	// check TID, time order in log
	if ( iTID<tIndex.m_iMaxTID )
		sphDie ( "binlog: update: descending tid (index=%s, lasttid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
	if ( tmStamp<tIndex.m_tmMax )
		sphDie ( "binlog: update: descending time (index=%s, lasttime="INT64_FMT", logtime="INT64_FMT", pos="INT64_FMT")",
			tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );

	if ( tIndex.m_pIndex && iTID > tIndex.m_pIndex->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		if ( iTID!=tIndex.m_pIndex->m_iTID+1 )
			sphWarning ( "binlog: update: unexpected tid (index=%s, indextid="INT64_FMT", logtid="INT64_FMT", pos="INT64_FMT")",
				tIndex.m_sName.cstr(), tIndex.m_pIndex->m_iTID, iTID, iTxnPos );

		CSphString sError;
		tIndex.m_pIndex->UpdateAttributes ( tUpd, -1, sError ); // FIXME! check for errors

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pIndex->m_iTID = iTID;
	}

	// update info
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
	return true;
}

bool RtBinlog_c::ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const
{
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// load data
	CSphVector<BinlogIndexInfo_t> dCache;
	dCache.Resize ( (int) tReader.UnzipValue() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, dCache )
	{
		dCache[i].m_sName = tReader.GetString();
		dCache[i].m_iMinTID = tReader.UnzipValue();
		dCache[i].m_iMaxTID = tReader.UnzipValue();
		dCache[i].m_iFlushedTID = tReader.UnzipValue();
		dCache[i].m_tmMin = tReader.UnzipValue();
		dCache[i].m_tmMax = tReader.UnzipValue();
	}
	if ( !tReader.CheckCrc ( "cache", "", 0, iTxnPos ) )
		return false;

	// if we arrived here by replay, let's verify everything
	// note that cached infos just passed checksumming, so the file is supposed to be clean!
	// in any case, broken log or not, we probably managed to replay something
	// so let's just report differences as warnings

	if ( dCache.GetLength()!=tLog.m_dIndexInfos.GetLength() )
	{
		sphWarning ( "binlog: cache mismatch: %d indexes cached, %d replayed",
			dCache.GetLength(), tLog.m_dIndexInfos.GetLength() );
		return true;
	}

	ARRAY_FOREACH ( i, dCache )
	{
		BinlogIndexInfo_t & tCache = dCache[i];
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

		if ( tCache.m_sName!=tIndex.m_sName )
		{
			sphWarning ( "binlog: cache mismatch: index %d name mismatch (%s cached, %s replayed)",
				i, tCache.m_sName.cstr(), tIndex.m_sName.cstr() );
			continue;
		}

		if ( tCache.m_iMinTID!=tIndex.m_iMinTID || tCache.m_iMaxTID!=tIndex.m_iMaxTID )
		{
			sphWarning ( "binlog: cache mismatch: index %s tid ranges mismatch (cached "INT64_FMT" to "INT64_FMT", replayed "INT64_FMT" to "INT64_FMT")",
				tCache.m_sName.cstr(), tCache.m_iMinTID, tCache.m_iMaxTID, tIndex.m_iMinTID, tIndex.m_iMaxTID );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

ISphRtIndex * sphGetCurrentIndexRT()
{
	RtAccum_t * pAcc = (RtAccum_t*) sphThreadGet ( g_tTlsAccumKey );
	if ( pAcc )
		return pAcc->m_pIndex;
	return NULL;
}

ISphRtIndex * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName, DWORD uRamSize, const char * sPath )
{
	MEMORY ( SPH_MEM_IDX_RT );
	return new RtIndex_t ( tSchema, sIndexName, uRamSize, sPath );
}

void sphRTInit ()
{
	MEMORY ( SPH_MEM_BINLOG );

	g_bRTChangesAllowed = false;
	Verify ( RtSegment_t::m_tSegmentSeq.Init() );
	Verify ( sphThreadKeyCreate ( &g_tTlsAccumKey ) );

	g_pBinlog = new RtBinlog_c();
	if ( !g_pBinlog )
		sphDie ( "binlog: failed to create binlog" );
}

void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	assert ( g_pBinlog );
	g_pBinlog->Configure ( hSearchd, bTestMode );
	g_iRtFlushPeriod = hSearchd.GetInt ( "rt_flush_period", (int)g_iRtFlushPeriod );

	// clip period to range ( 10 sec, million years )
	g_iRtFlushPeriod = Max ( g_iRtFlushPeriod, 10 );
	g_iRtFlushPeriod = Min ( g_iRtFlushPeriod, INT64_MAX );
}

void sphRTDone ()
{
	sphThreadKeyDelete ( g_tTlsAccumKey );
	Verify ( RtSegment_t::m_tSegmentSeq.Done() );
	// its valid for "searchd --stop" case
	SafeDelete ( g_pBinlog );
}

void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( SPH_MEM_BINLOG );
	g_pBinlog->Replay ( hIndexes, pfnProgressCallback );
	g_pBinlog->CreateTimerThread();
	g_bRTChangesAllowed = true;
}

//
// $Id$
//
