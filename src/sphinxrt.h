//
// Copyright (c) 2017-2018, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxrt_
#define _sphinxrt_

#include "sphinx.h"
#include "sphinxutils.h"
#include "sphinxstem.h"
#include "sphinxint.h"

struct CSphReconfigureSettings;
struct CSphReconfigureSetup;
class ISphRtAccum;


/// RAM based updateable backend interface
class ISphRtIndex : public CSphIndex
{
public:
	explicit ISphRtIndex ( const char * sIndexName, const char * sFileName ) : CSphIndex ( sIndexName, sFileName ) {}

	/// get internal schema (to use for Add calls)
	virtual const CSphSchema & GetInternalSchema () const { return m_tSchema; }

	/// insert/update document in current txn
	/// fails in case of two open txns to different indexes
	/// fields are already measured; i.e. not necessary to be asciiz and apply strlen to them
	virtual bool AddDocument ( const VecTraits_T<VecTraits_T<const char >> &dFields, const CSphMatch &tDoc,
		bool bReplace, const CSphString &sTokenFilterOptions, const char ** ppStr, const VecTraits_T<DWORD> &dMvas,
		CSphString &sError, CSphString &sWarning, ISphRtAccum * pAccExt ) = 0;

	/// delete document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError, ISphRtAccum * pAccExt ) = 0;

	/// commit pending changes
	virtual void Commit ( int * pDeleted, ISphRtAccum * pAccExt ) = 0;

	/// undo pending changes
	virtual void RollBack ( ISphRtAccum * pAccExt ) = 0;

	/// check and periodically flush RAM chunk to disk
	virtual void CheckRamFlush () = 0;

	/// forcibly flush RAM chunk to disk
	virtual void ForceRamFlush ( bool bPeriodic=false ) = 0;

	/// get time of last flush, 0 means no flush required
	virtual int64_t GetFlushAge() const = 0;

	/// forcibly save RAM chunk as a new disk chunk
	virtual void ForceDiskChunk () = 0;

	/// attach a disk chunk to current index
	virtual bool AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError ) = 0;

	/// truncate index (that is, kill all data)
	virtual bool Truncate ( CSphString & sError ) = 0;

	virtual void Optimize ( ) = 0;

	/// check settings vs current and return back tokenizer and dictionary in case of difference
	virtual bool IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError ) const = 0;

	/// reconfigure index by using new tokenizer, dictionary and index settings
	/// current data got saved with current settings
	virtual void Reconfigure ( CSphReconfigureSetup & tSetup ) = 0;

	/// get disk chunk
	virtual CSphIndex * GetDiskChunk ( int iChunk ) = 0;
	
	virtual ISphRtAccum * CreateAccum ( CSphString & sError ) = 0;

	// instead of cloning for each AddDocument() call we could just call this method and improve batch inserts speed
	virtual ISphTokenizer * CloneIndexingTokenizer() const = 0;

	virtual void ProhibitSave() = 0;
	
	/// acquire thread-local indexing accumulator
	/// returns NULL if another index already uses it in an open txn
	ISphRtAccum * AcquireAccum ( CSphDict * pDict, ISphRtAccum * pAccExt=nullptr,
		bool bWordDict=true, bool bSetTLS = true, CSphString * sError=nullptr );
};

/// initialize subsystem
class CSphConfigSection;
void sphRTInit ( const CSphConfigSection & hSearchd, bool bTestMode, const CSphConfigSection * pCommon );
void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode );
bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema * pSchema, CSphString * pError, bool bSkipValidation );
void sphRTSetTestMode ();

/// deinitialize subsystem
void sphRTDone ();

/// RT index factory
ISphRtIndex * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );

typedef void ProgressCallbackSimple_t ();

class ISphRtAccum
{
protected:
	ISphRtIndex * m_pIndex=nullptr;		///< my current owner in this thread
	ISphRtAccum () {} // can not create such thing outside of RT index
public:
	virtual ~ISphRtAccum () {}
	ISphRtIndex * GetIndex() const { return m_pIndex; }
};

typedef const QueryParser_i * CreateQueryParser ( bool bJson );


//////////////////////////////////////////////////////////////////////////

enum ESphBinlogReplayFlags
{
	SPH_REPLAY_ACCEPT_DESC_TIMESTAMP = 1,
	SPH_REPLAY_IGNORE_OPEN_ERROR = 2
};

typedef void BinlogFlushWork_t ( void * pLog );

struct BinlogFlushInfo_t
{
	void * m_pLog = nullptr;
	BinlogFlushWork_t * m_fnWork = nullptr;
};

/// replay stored binlog
void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback, BinlogFlushInfo_t & tFlush );


/// Exposed internal stuff (for pq and for testing)

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );

struct RtDoc_t
{
	SphDocID_t m_uDocID = 0;    ///< my document id
	DWORD m_uDocFields = 0;    ///< fields mask
	DWORD m_uHits = 0;    ///< hit count
	DWORD m_uHit = 0;        ///< either index into segment hits, or the only hit itself (if hit count is 1)
};

struct RtWord_t
{
	union
	{
		SphWordID_t m_uWordID;    ///< my keyword id
		const BYTE * m_sWord;
		typename WIDEST<SphWordID_t, const BYTE *>::T m_null = 0;
	};
	DWORD m_uDocs = 0;    ///< document count (for stats and/or BM25)
	DWORD m_uHits = 0;    ///< hit count (for stats and/or BM25)
	DWORD m_uDoc = 0;        ///< index into segment docs
};

struct RtWordCheckpoint_t
{
	union
	{
		SphWordID_t m_uWordID;
		const char * m_sWord;
	};
	int m_iOffset;
};

struct KlistRefcounted_t;

// this is what actually stores index data
// RAM chunk consists of such segments
struct RtSegment_t : ISphNoncopyable
{
protected:
	static const int KLIST_ACCUM_THRESH = 32;

public:
	static CSphAtomic m_iSegments;    ///< age tag sequence generator
	int m_iTag;            ///< segment age tag

	CSphTightVector<BYTE> m_dWords;
	CSphVector<RtWordCheckpoint_t> m_dWordCheckpoints;
	CSphTightVector<uint64_t> m_dInfixFilterCP;
	CSphTightVector<BYTE> m_dDocs;
	CSphTightVector<BYTE> m_dHits;

	int m_iRows = 0;        ///< number of actually allocated rows
	int m_iAliveRows = 0;    ///< number of alive (non-killed) rows
	CSphTightVector<CSphRowitem> m_dRows;        ///< row data storage
	KlistRefcounted_t * m_pKlist;
	bool m_bTlsKlist = false;    ///< whether to apply TLS K-list during merge (must only be used by writer during Commit())
	CSphTightVector<BYTE> m_dStrings;        ///< strings storage
	CSphTightVector<DWORD> m_dMvas;        ///< MVAs storage
	CSphVector<BYTE> m_dKeywordCheckpoints;
	mutable CSphAtomic m_tRefCount;

	RtSegment_t ();
	~RtSegment_t ();

	int64_t GetUsedRam () const;
	int GetMergeFactor () const;
	int GetStride () const;
	const CSphFixedVector<SphDocID_t> &GetKlist () const;
	const CSphRowitem * FindRow ( SphDocID_t uDocid ) const;
	const CSphRowitem * FindAliveRow ( SphDocID_t uDocid ) const;
};

struct RtDocReader_t
{
	const BYTE * m_pDocs = nullptr;
	int m_iLeft = 0;
	RtDoc_t m_tDoc;

	RtDocReader_t ( const RtSegment_t * pSeg, const RtWord_t &tWord );
	RtDocReader_t () = default;
	const RtDoc_t * UnzipDoc ();
};

struct RtWordReader_t
{
	BYTE m_tPackedWord[SPH_MAX_KEYWORD_LEN + 1];
	const BYTE * m_pCur = nullptr;
	const BYTE * m_pMax = nullptr;
	RtWord_t m_tWord;
	int m_iWords = 0;

	bool m_bWordDict;
	int m_iWordsCheckpoint;
	int m_iCheckpoint = 0;

	RtWordReader_t ( const RtSegment_t * pSeg, bool bWordDict, int iWordsCheckpoint );
	void Reset ( const RtSegment_t * pSeg );
	const RtWord_t * UnzipWord ();
};

struct RtHitReader_t
{
	const BYTE * m_pCur = nullptr;
	DWORD m_iLeft = 0;
	DWORD m_uLast = 0;

	RtHitReader_t () = default;
	explicit RtHitReader_t ( const RtSegment_t * pSeg, const RtDoc_t * pDoc );
	DWORD UnzipHit ();
};

struct RtHitReader2_t : public RtHitReader_t
{
	const BYTE * m_pBase = nullptr;
	void Seek ( SphOffset_t uOff, int iHits );
};

/// indexing accumulator
class RtAccum_t : public ISphRtAccum
{
public:
	int m_iAccumDocs = 0;
	CSphTightVector<CSphWordHit> m_dAccum;
	CSphTightVector<CSphRowitem> m_dAccumRows;
	CSphVector<SphDocID_t> m_dAccumKlist;
	CSphTightVector<BYTE> m_dStrings;
	CSphTightVector<DWORD> m_dMvas;
	CSphVector<DWORD> m_dPerDocHitsCount;

	bool m_bKeywordDict;
	CSphDictRefPtr_c m_pDict;
	CSphDict * m_pRefDict = nullptr; // not owned, used only for ==-matching

private:
	ISphRtDictWraperRefPtr_c m_pDictRt;
	bool m_bReplace = false;    ///< insert or replace mode (affects CleanupDuplicates() behavior)
	void ResetDict ();

public:
	explicit RtAccum_t ( bool bKeywordDict );
	void SetupDict ( const ISphRtIndex * pIndex, CSphDict * pDict, bool bKeywordDict );
	void Sort ();

	enum EWhatClear { EPartial = 1, EAccum = 2, ERest = 4, EAll = 7	};
	void Cleanup ( BYTE eWhat = EAll );

	void AddDocument ( ISphHits * pHits, const CSphMatch &tDoc, bool bReplace, int iRowSize, const char ** ppStr,
					   const VecTraits_T<DWORD> &dMvas );
	RtSegment_t * CreateSegment ( int iRowSize, int iWordsCheckpoint );
	void CleanupDuplicates ( int iRowSize );
	void GrabLastWarning ( CSphString &sWarning );
	void SetIndex ( ISphRtIndex * pIndex )	{ m_pIndex = pIndex; }
};

class CSphSource_StringVector : public CSphSource_Document
{
public:
	explicit			CSphSource_StringVector ( const VecTraits_T<const char *> &dFields, const CSphSchema & tSchema );
	explicit			CSphSource_StringVector ( const VecTraits_T<VecTraits_T<const char >> &dFields, const CSphSchema &tSchema );
						~CSphSource_StringVector () override = default;

	bool		Connect ( CSphString & ) override;
	void		Disconnect () override;

	bool HasAttrsConfigured () override { return false; }
	bool IterateStart ( CSphString & ) override { m_iPlainFieldsLength = m_tSchema.GetFieldsCount(); return true; }

	bool		IterateMultivaluedStart ( int, CSphString & ) override { return false; }
	bool		IterateMultivaluedNext () override { return false; }

	SphRange_t			IterateFieldMVAStart ( int ) override { return {}; }

	bool		IterateKillListStart ( CSphString & ) override { return false; }
	bool		IterateKillListNext ( SphDocID_t & ) override { return false; }

	BYTE **		NextDocument ( CSphString & ) override { return m_dFields.Begin(); }
	const int *	GetFieldLengths () const override { return m_dFieldLengths.Begin(); }
	void		SetMorphFields ( const CSphBitvec & tMorphFields ) { m_tMorphFields = tMorphFields; }

protected:
	CSphVector<BYTE *>			m_dFields;
	CSphVector<int>				m_dFieldLengths;
};

#define BLOOM_PER_ENTRY_VALS_COUNT 8
#define BLOOM_HASHES_COUNT 2
#define BLOOM_NGRAM_0 2
#define BLOOM_NGRAM_1 4

struct BloomGenTraits_t
{
	uint64_t * m_pBuf = nullptr;

	explicit BloomGenTraits_t ( uint64_t * pBuf )
		: m_pBuf ( pBuf )
	{}

	void Set ( int iPos, uint64_t uVal )
	{
		m_pBuf[iPos] |= uVal;
	}

	bool IterateNext () const
	{ return true; }
};

struct BloomCheckTraits_t
{
	const uint64_t * m_pBuf = nullptr;
	bool m_bSame = true;

	explicit BloomCheckTraits_t ( const uint64_t * pBuf )
		: m_pBuf ( pBuf )
	{}

	void Set ( int iPos, uint64_t uVal )
	{
		m_bSame = ( ( m_pBuf[iPos] & uVal )==uVal );
	}

	bool IterateNext () const
	{ return m_bSame; }
};

bool BuildBloom ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8,
	int iKeyValCount, BloomGenTraits_t &tBloom );

bool BuildBloom ( const BYTE * sWord, int iLen, int iInfixCodepointCount, bool bUtf8,
	int iKeyValCount, BloomCheckTraits_t &tBloom );

void BuildSegmentInfixes ( RtSegment_t * pSeg, bool bHasMorphology, bool bKeywordDict, int iMinInfixLen,
	int iWordsCheckpoint, bool bUtf8 );

void CopyDocinfo ( CSphMatch &tMatch, const DWORD * pFound );

const CSphRowitem * FindDocinfo ( const RtSegment_t * pSeg, SphDocID_t uDocID, int iStride );

bool ExtractInfixCheckpoints ( const char * sInfix, int iBytes, int iMaxCodepointLength, int iDictCpCount,
	const CSphTightVector<uint64_t> &dFilter, CSphVector<DWORD> &dCheckpoints );

void SetupExactDict ( CSphDictRefPtr_c &pDict, ISphTokenizer * pTokenizer, bool bAddSpecial = true );

void SetupStarDict ( CSphDictRefPtr_c &pDict, ISphTokenizer * pTokenizer );

bool CreateReconfigure ( const CSphString & sIndexName, bool bIsStarDict, const ISphFieldFilter * pFieldFilter,
	const CSphIndexSettings & tIndexSettings, uint64_t uTokHash, uint64_t uDictHash, int iMaxCodepointLength,
	bool bSame, CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError );

// Get global flag of w-available RT
volatile bool &RTChangesAllowed ();

#endif // _sphinxrt_
