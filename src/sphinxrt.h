//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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
#include "killlist.h"
#include "attribute.h"
#include "docstore.h"

struct CSphReconfigureSettings;
struct CSphReconfigureSetup;
class RtAccum_t;

const int64_t DEFAULT_RT_MEM_LIMIT = 128 * 1024 * 1024;


/// RAM based updateable backend interface
class RtIndex_i : public CSphIndex
{
public:
	explicit RtIndex_i ( const char * sIndexName, const char * sFileName ) : CSphIndex ( sIndexName, sFileName ) {}

	/// get internal schema (to use for Add calls)
	virtual const CSphSchema & GetInternalSchema () const { return m_tSchema; }

	/// insert/update document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool AddDocument ( const VecTraits_T<VecTraits_T<const char >> &dFields, CSphMatch & tDoc,
		bool bReplace, const CSphString & sTokenFilterOptions, const char ** ppStr, const VecTraits_T<int64_t> & dMvas,
		CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt ) = 0;

	/// delete document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool DeleteDocument ( const VecTraits_T<DocID_t> & dDocs, CSphString & sError, RtAccum_t * pAccExt ) = 0;

	/// commit pending changes
	virtual bool Commit ( int * pDeleted, RtAccum_t * pAccExt ) = 0;

	/// undo pending changes
	virtual void RollBack ( RtAccum_t * pAccExt ) = 0;

	/// forcibly flush RAM chunk to disk
	virtual void ForceRamFlush ( bool bPeriodic=false ) = 0;

	virtual bool IsFlushNeed() const = 0;

	/// get time of last flush happened
	virtual int64_t GetLastFlushTimestamp() const = 0;

	/// forcibly save RAM chunk as a new disk chunk
	virtual bool ForceDiskChunk () = 0;

	/// attach a disk chunk to current index
	virtual bool AttachDiskIndex ( CSphIndex * pIndex, bool bTruncate, bool & bFatal, CSphString & sError ) = 0;

	/// truncate index (that is, kill all data)
	virtual bool Truncate ( CSphString & sError ) = 0;

	virtual void Optimize ( int iCutoff, int iFrom, int iTo ) = 0;

	/// check settings vs current and return back tokenizer and dictionary in case of difference
	virtual bool IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const = 0;

	/// reconfigure index by using new tokenizer, dictionary and index settings
	/// current data got saved with current settings
	virtual bool Reconfigure ( CSphReconfigureSetup & tSetup ) = 0;

	/// get disk chunk
	virtual CSphIndex * GetDiskChunk ( int iChunk ) = 0;
	
	virtual RtAccum_t * CreateAccum ( RtAccum_t * pAccExt, CSphString & sError ) = 0;

	// instead of cloning for each AddDocument() call we could just call this method and improve batch inserts speed
	virtual ISphTokenizer * CloneIndexingTokenizer() const = 0;

	// hint an index that it was deleted and should cleanup its files when destructed
	virtual void IndexDeleted() = 0;

	virtual void ProhibitSave() = 0;
	virtual void EnableSave() = 0;
	virtual void LockFileState ( CSphVector<CSphString> & dFiles ) = 0;
	
	/// acquire thread-local indexing accumulator
	/// returns NULL if another index already uses it in an open txn
	RtAccum_t * AcquireAccum ( CSphDict * pDict, RtAccum_t * pAccExt=nullptr, bool bWordDict=true, bool bSetTLS = true, CSphString * sError=nullptr );

	virtual bool	NeedStoreWordID () const = 0;
	virtual	int64_t	GetMemLimit() const = 0;
};

/// initialize subsystem
class CSphConfigSection;
void sphRTInit ( const CSphConfigSection & hSearchd, bool bTestMode, const CSphConfigSection * pCommon );
void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode );
bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema & tSchema, CSphString & sError, bool bSkipValidation );
bool sphRTSchemaConfigure ( const CSphVector<CSphColumnInfo> & dFields, const CSphVector<CSphColumnInfo> & dAttrs, CSphSchema & tSchema, CSphString & sError, bool bSkipValidation );
void sphRTSetTestMode ();

/// deinitialize subsystem
void sphRTDone ();

/// RT index factory
RtIndex_i * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );

typedef void ProgressCallbackSimple_t ();


//////////////////////////////////////////////////////////////////////////

enum ESphBinlogReplayFlags
{
	SPH_REPLAY_ACCEPT_DESC_TIMESTAMP = 1,
	SPH_REPLAY_IGNORE_OPEN_ERROR = 2
};

/// replay stored binlog
void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags = 0, ProgressCallbackSimple_t * pfnProgressCallback = nullptr );

bool sphFlushBinlogEnabled();
void sphFlushBinlog ();
int64_t sphNextFlushTimestamp();


/// Exposed internal stuff (for pq and for testing)

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );

struct RtDoc_t
{
	RowID_t m_tRowID { INVALID_ROWID };    ///< row id
	DWORD m_uDocFields = 0;            ///< fields mask
	DWORD m_uHits = 0;                ///< hit count
	DWORD m_uHit = 0;                    ///< either index into segment hits, or the only hit itself (if hit count is 1)
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
	bool m_bHasHitlist = true;
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


// this is what actually stores index data
// RAM chunk consists of such segments
struct RtSegment_t final : IndexSegment_c, ISphRefcountedMT
{
public:

	CSphTightVector<BYTE>			m_dWords;
	CSphVector<RtWordCheckpoint_t>	m_dWordCheckpoints;
	CSphTightVector<uint64_t>		m_dInfixFilterCP;
	CSphTightVector<BYTE>			m_dDocs;
	CSphTightVector<BYTE>			m_dHits;

	DWORD							m_uRows { 0 };        ///< number of actually allocated rows
	std::atomic<int64_t>			m_tAliveRows { 0 };    ///< number of alive (non-killed) rows
	CSphTightVector<CSphRowitem>	m_dRows;            ///< row data storage
	CSphTightVector<BYTE>			m_dBlobs;            ///< storage for blob attrs
	CSphVector<BYTE>				m_dKeywordCheckpoints;
	std::atomic<int64_t> *			m_pRAMCounter = nullptr; ///< external RAM counter
	OpenHash_T<RowID_t, DocID_t>	m_tDocIDtoRowID; ///< speeds up docid-rowid lookups
	DeadRowMap_Ram_c				m_tDeadRowMap;
	CSphScopedPtr<DocstoreRT_i>		m_pDocstore{nullptr};


							RtSegment_t ( DWORD uDocs );

	int64_t					GetUsedRam();				// get cached ram usage counter
	void					UpdateUsedRam();			// recalculate ram usage, update index ram counter
	DWORD					GetMergeFactor() const;
	int						GetStride() const;

	const CSphRowitem *		FindAliveRow ( DocID_t tDocid ) const;
	const CSphRowitem *		GetDocinfoByRowID ( RowID_t tRowID ) const;
	RowID_t					GetRowidByDocid ( DocID_t tDocID ) const;

	int						Kill ( DocID_t tDocID ) override;
	int						KillMulti ( const VecTraits_T<DocID_t> & dKlist ) override;

	void					SetupDocstore ( const CSphSchema * pSchema );
	void					BuildDocID2RowIDMap();

private:
	int64_t					m_iUsedRam = 0;			///< ram usage counter

							~RtSegment_t () final;

	void					FixupRAMCounter ( int64_t iDelta ) const;
};

using RtSegmentRefPtf_t = CSphRefcountedPtr<RtSegment_t>;
using constRtSegmentRefPtf_t = CSphRefcountedPtr<const RtSegment_t>;

struct RtDocReader_t
{
	const BYTE * m_pDocs = nullptr;
	int m_iLeft = 0;
	RtDoc_t m_tDoc;

	RtDocReader_t() = default;
	RtDocReader_t ( const RtSegment_t * pSeg, const RtWord_t &tWord );
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
	const ESphHitless m_eHitlessMode = SPH_HITLESS_NONE;

	RtWordReader_t ( const RtSegment_t * pSeg, bool bWordDict, int iWordsCheckpoint, ESphHitless eHitlessMode );
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
	ByteBlob_t GetHitsBlob() const;
};

struct RtHitReader2_t : public RtHitReader_t
{
	const BYTE * m_pBase = nullptr;
	void Seek ( SphOffset_t uOff, int iHits );
};

class CSphSource_StringVector : public CSphSource_Document
{
public:
	explicit			CSphSource_StringVector ( const VecTraits_T<const char *> &dFields, const CSphSchema & tSchema );
	explicit			CSphSource_StringVector ( const VecTraits_T<VecTraits_T<const char >> &dFields, const CSphSchema &tSchema );
						~CSphSource_StringVector () override = default;

	bool		Connect ( CSphString & ) override;
	void		Disconnect () override;

	bool		IterateStart ( CSphString & ) override { m_iPlainFieldsLength = m_tSchema.GetFieldsCount(); return true; }

	bool		IterateMultivaluedStart ( int, CSphString & ) override { return false; }
	bool		IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) override { return false; }

	CSphVector<int64_t> * GetFieldMVA ( int iAttr ) override { return nullptr; }

	bool		IterateKillListStart ( CSphString & ) override { return false; }
	bool		IterateKillListNext ( DocID_t & ) override { return false; }

	BYTE **		NextDocument ( bool &, CSphString & ) override { return m_dFields.Begin(); }
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
	int iWordsCheckpoint, bool bUtf8, ESphHitless eHitlessMode );

bool ExtractInfixCheckpoints ( const char * sInfix, int iBytes, int iMaxCodepointLength, int iDictCpCount,
	const CSphTightVector<uint64_t> &dFilter, CSphVector<DWORD> &dCheckpoints );

void SetupExactDict ( DictRefPtr_c &pDict, ISphTokenizer * pTokenizer, bool bAddSpecial = true );

void SetupStarDict ( DictRefPtr_c &pDict, ISphTokenizer * pTokenizer );

bool CreateReconfigure ( const CSphString & sIndexName, bool bIsStarDict, const ISphFieldFilter * pFieldFilter,
	const CSphIndexSettings & tIndexSettings, uint64_t uTokHash, uint64_t uDictHash, int iMaxCodepointLength, int64_t iMemLimit,
	bool bSame, CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError );

// Get global flag of w-available RT
volatile bool &RTChangesAllowed ();

#endif // _sphinxrt_
