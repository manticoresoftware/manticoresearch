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

#ifndef _sphinxrt_
#define _sphinxrt_

#include "sphinxutils.h"
#include "sphinxstem.h"
#include "sphinxint.h"
#include "killlist.h"
#include "attribute.h"
#include "docstore.h"
#include "columnarrt.h"
#include "coroutine.h"
#include "tokenizer/tokenizer.h"
#include "indexing_sources/source_document.h"

class RtAccum_t;

using VisitChunk_fn = std::function<void ( const CSphIndex* pIndex )>;

struct InsertDocData_t
{
	CSphMatch							m_tDoc;
	CSphVector<VecTraits_T<const char>>	m_dFields;
	CSphVector<const char*>				m_dStrings;
	CSphVector<int64_t>					m_dMvas;

	CSphAttrLocator						m_tDocIDLocator;

	CSphVector<SphAttr_t>				m_dColumnarAttrs;
	int									m_iColumnarID = -1;
	int64_t								m_iTotalBytes = 0;

										explicit InsertDocData_t ( const ISphSchema & tSchema );

	void								SetID ( SphAttr_t tDocID );
	SphAttr_t							GetID() const;
};

struct OptimizeTask_t
{
	enum OptimizeVerb_e {
		eManualOptimize,
		eDrop,
		eCompress,
		eSplit,
		eMerge,
		eAutoOptimize,
	};

	OptimizeVerb_e m_eVerb;
	int m_iCutoff	=	0;
	int m_iFrom		=	-1;
	int m_iTo		=	-1;
	bool m_bByOrder = false;
	CSphString m_sUvarFilter;
	CSphString m_sIndex;
};

struct CSphReconfigureSettings
{
	CSphTokenizerSettings m_tTokenizer;
	CSphDictSettings m_tDict;
	CSphIndexSettings m_tIndex;
	CSphFieldFilterSettings m_tFieldFilter;
	CSphSchema m_tSchema;
	MutableIndexSettings_c m_tMutableSettings;

	bool m_bChangeSchema = false;
};

struct CSphReconfigureSetup
{
	TokenizerRefPtr_c m_pTokenizer;
	DictRefPtr_c m_pDict;
	CSphIndexSettings m_tIndex;
	std::unique_ptr<ISphFieldFilter> m_pFieldFilter;
	CSphSchema m_tSchema;
	MutableIndexSettings_c m_tMutableSettings;

	bool m_bChangeSchema = false;
};

/// RAM based updateable backend interface
class RtIndex_i : public CSphIndexStub
{
public:
	RtIndex_i ( CSphString sIndexName, CSphString sPath ) : CSphIndexStub { std::move ( sIndexName ), std::move ( sPath ) } {}

	/// get internal schema (to use for Add calls)
	virtual const CSphSchema & GetInternalSchema () const { return m_tSchema; }
	virtual uint64_t GetSchemaHash () const = 0;

	/// insert/update document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool AddDocument ( InsertDocData_t & tDoc, bool bReplace, const CSphString & sTokenFilterOptions, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt ) = 0;

	/// delete document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool DeleteDocument ( const VecTraits_T<DocID_t> & dDocs, CSphString & sError, RtAccum_t * pAccExt ) = 0;

	/// commit pending changes
	virtual bool Commit ( int * pDeleted, RtAccum_t * pAccExt, CSphString* pError = nullptr ) = 0;

	/// undo pending changes
	virtual void RollBack ( RtAccum_t * pAccExt ) = 0;

	/// forcibly flush RAM chunk to disk
	virtual void ForceRamFlush ( const char* szReason ) = 0;

	virtual bool IsFlushNeed() const = 0;

	/// get time of last flush happened
	virtual int64_t GetLastFlushTimestamp() const = 0;

	/// forcibly save RAM chunk as a new disk chunk
	virtual bool ForceDiskChunk () = 0;

	/// attach a disk chunk to current index
	virtual bool AttachDiskIndex ( CSphIndex * pIndex, bool bTruncate, bool & bFatal, StrVec_t & dWarnings, CSphString & sError ) { return true; }

	/// truncate index (that is, kill all data)
	virtual bool Truncate ( CSphString & sError ) = 0;

	virtual void Optimize ( OptimizeTask_t tTask ) {}

	/// check settings vs current and return back tokenizer and dictionary in case of difference
	virtual bool IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const = 0;

	/// reconfigure index by using new tokenizer, dictionary and index settings
	/// current data got saved with current settings
	virtual bool Reconfigure ( CSphReconfigureSetup & tSetup ) = 0;

	// generation typically changes on Reconfigure
	virtual int GetAlterGeneration() const { return 0; }

	/// do something const with disk chunk (query settings, status, etc.)
	/// hides internal disk chunks storage
	virtual void ProcessDiskChunk ( int iChunk, VisitChunk_fn&& fnVisitor ) const {};

	/// get disk chunk
	virtual CSphIndex* GetDiskChunk ( int iChunk )
	{
		return nullptr;
	}

	/// bind indexing accumulator
	/// returns false if another index already uses it in an open txn
	virtual bool BindAccum ( RtAccum_t * pAccExt, CSphString * pError ) = 0;

	// instead of cloning for each AddDocument() call we could just call this method and improve batch inserts speed
	virtual TokenizerRefPtr_c CloneIndexingTokenizer() const = 0;

	// hint an index that it was deleted and should cleanup its files when destructed
	virtual void IndexDeleted() = 0;

	virtual void ProhibitSave() = 0;
	virtual void EnableSave() = 0;
	virtual void LockFileState ( CSphVector<CSphString> & dFiles ) = 0;
	
	virtual bool	NeedStoreWordID () const = 0;
	virtual	int64_t	GetMemLimit() const = 0;

protected:
	bool PrepareAccum ( RtAccum_t* pAccExt, bool bWordDict, CSphString* pError );
};

/// initialize subsystem
class CSphConfigSection;
void sphRTInit ( const CSphConfigSection & hSearchd, bool bTestMode, const CSphConfigSection * pCommon );
bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema & tSchema, const CSphIndexSettings & tSettings, CSphString & sError, bool bSkipValidation, bool bPQ );
void sphRTSetTestMode ();

/// RT index factory
std::unique_ptr<RtIndex_i> sphCreateIndexRT ( CSphString sIndexName, CSphString sPath, CSphSchema tSchema, int64_t iRamSize, bool bKeywordDict );

typedef void ProgressCallbackSimple_t ();

//////////////////////////////////////////////////////////////////////////

/// Exposed internal stuff (for pq and for testing)

#define SPH_MAX_KEYWORD_LEN (3*SPH_MAX_WORD_LEN+4)
STATIC_ASSERT ( SPH_MAX_KEYWORD_LEN<255, MAX_KEYWORD_LEN_SHOULD_FITS_BYTE );

struct RtDoc_t
{
	RowID_t m_tRowID { INVALID_ROWID };	///< row id
	DWORD m_uDocFields = 0;				///< fields mask
	DWORD m_uHits = 0;					///< hit count
	DWORD m_uHit = 0;					///< either index into segment hits, or the only hit itself (if hit count is 1)
};


struct RtWord_t
{
	union
	{
		SphWordID_t m_uWordID;    ///< my keyword id
		const BYTE * m_sWord;
		INIT_WITH_0 ( SphWordID_t, const BYTE* );
	};
	DWORD m_uDocs = 0;		///< document count (for stats and/or BM25)
	DWORD m_uHits = 0;		///< hit count (for stats and/or BM25)
	DWORD m_uDoc = 0;		///< index into segment docs
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
	mutable int						m_iLocked = 0;	// if segment currently used in an op
	mutable Threads::Coro::RWLock_c	m_tLock;		// fine-grain lock

	CSphTightVector<BYTE>			m_dWords;
	CSphVector<RtWordCheckpoint_t>	m_dWordCheckpoints;
	CSphTightVector<uint64_t>		m_dInfixFilterCP;
	CSphTightVector<BYTE>			m_dDocs;
	CSphTightVector<BYTE>			m_dHits;

	DWORD							m_uRows = 0;			///< number of actually allocated rows
	std::atomic<int64_t>			m_tAliveRows { 0 };		///< number of alive (non-killed) rows
	CSphTightVector<CSphRowitem>	m_dRows GUARDED_BY ( m_tLock );				///< row data storage
	CSphTightVector<BYTE>			m_dBlobs GUARDED_BY ( m_tLock );            ///< storage for blob attrs
	CSphVector<BYTE>				m_dKeywordCheckpoints;
	std::atomic<int64_t> *			m_pRAMCounter = nullptr;///< external RAM counter
	OpenHash_T<RowID_t, DocID_t>	m_tDocIDtoRowID;		///< speeds up docid-rowid lookups
	DeadRowMap_Ram_c				m_tDeadRowMap;
	std::unique_ptr<DocstoreRT_i>	m_pDocstore;
	std::unique_ptr<ColumnarRT_i>	m_pColumnar;
	const ISphSchema&				m_tSchema;

	mutable bool					m_bConsistent{false};

							RtSegment_t ( DWORD uDocs, const ISphSchema& tSchema );

	int64_t					GetUsedRam() const;				// get cached ram usage counter
	void					UpdateUsedRam() const;			// recalculate ram usage, update index ram counter
	DWORD					GetMergeFactor() const;
	int						GetStride() const;

	const CSphRowitem * 	FindAliveRow ( DocID_t tDocid ) const;
	const CSphRowitem *		GetDocinfoByRowID ( RowID_t tRowID ) const;
	RowID_t					GetAliveRowidByDocid ( DocID_t tDocid ) const;
	RowID_t					GetRowidByDocid ( DocID_t tDocID ) const;

	int						Kill ( DocID_t tDocID ) override;
	int						KillMulti ( const VecTraits_T<DocID_t> & dKlist ) override;

	void					SetupDocstore ( const CSphSchema * pSchema );
	void					BuildDocID2RowIDMap ( const CSphSchema & tSchema );

	void					MaybeAddPostponedUpdate ( const RowsToUpdate_t& dRows, const UpdateContext_t& tCtx );
	void					UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t>& dPostUpdates ) final;

private:
	mutable int64_t			m_iUsedRam = 0;			///< ram usage counter

							~RtSegment_t () final;

	void					FixupRAMCounter ( int64_t iDelta ) const;
	bool					Update_WriteBlobRow ( UpdateContext_t& tCtx, RowID_t tRowID, ByteBlob_t tBlob, int nBlobAttrs, const CSphAttrLocator& tBlobRowLoc, bool& bCritical, CSphString& sError ) final;
};

using RtSegmentRefPtf_t = CSphRefcountedPtr<RtSegment_t>;
using ConstRtSegmentRefPtf_t = CSphRefcountedPtr<const RtSegment_t>;

class RtWordReader_c
{
	BYTE m_tPackedWord[SPH_MAX_KEYWORD_LEN + 1];
	RtWord_t m_tWord;
	int m_iWords = 0;

	bool m_bWordDict;
	int m_iWordsCheckpoint;
	int m_iCheckpoint = 0;
	const ESphHitless m_eHitlessMode = SPH_HITLESS_NONE;

public:
	const BYTE* m_pCur = nullptr;
	const BYTE* m_pMax = nullptr;

	RtWordReader_c ( const RtSegment_t * pSeg, bool bWordDict, int iWordsCheckpoint, ESphHitless eHitlessMode );
	void Reset ( const RtSegment_t * pSeg );
	inline int Checkpoint() const { return m_iCheckpoint; }
	const RtWord_t* UnzipWord();
	inline explicit operator RtWord_t*() { return &m_tWord; }
	inline RtWord_t* operator->() { return &m_tWord; }
	inline const RtWord_t& operator*() const { return m_tWord; }
};

class RtDocReader_c
{
	const BYTE* m_pDocs = nullptr;
	int m_iLeft = 0;
	RtDoc_t m_tDoc;

public:
	RtDocReader_c() = default;
	RtDocReader_c ( const RtSegment_t* pSeg, const RtWord_t& tWord );
	void Init ( const RtSegment_t* pSeg, const RtWord_t& tWord );
	void Reset () { m_pDocs = nullptr; m_iLeft = 0; }
	bool UnzipDoc();
	inline explicit operator RtDoc_t*() { return &m_tDoc; }
	inline RtDoc_t* operator->() { return &m_tDoc; }
	inline const RtDoc_t& operator*() const { return m_tDoc; }
};

class RtHitReader_c
{
	const BYTE* m_pCur = nullptr;
	DWORD m_uLeft = 0;
	DWORD m_uValue = 0;

public:
	RtHitReader_c() = default;
	RtHitReader_c ( const RtSegment_t& dSeg, const RtDoc_t& dDoc );
	void Seek ( const RtSegment_t& dSeg, const RtDoc_t& dDoc );
	void Seek ( const BYTE* pHits, DWORD uHits );
	inline DWORD operator*() const { return m_uValue; }
	DWORD UnzipHit ();
};

ByteBlob_t GetHitsBlob ( const RtSegment_t* pSeg, const RtDoc_t* pDoc );

class CSphSource_StringVector : public CSphSource
{
public:
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

void SetupExactTokenizer ( const TokenizerRefPtr_c& pTokenizer, bool bAddSpecial = true );
void SetupStarTokenizer ( const TokenizerRefPtr_c& pTokenizer );

bool CreateReconfigure ( const CSphString & sIndexName, bool bIsStarDict, const ISphFieldFilter * pFieldFilter,
	const CSphIndexSettings & tIndexSettings, uint64_t uTokHash, uint64_t uDictHash, int iMaxCodepointLength, int64_t iMemLimit,
	bool bSame, CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError );

// Get global flag of w-available RT
volatile bool &RTChangesAllowed () noexcept;

// Get global flag of autooptimize
volatile int & AutoOptimizeCutoffMultiplier() noexcept;
volatile int AutoOptimizeCutoff() noexcept;

using OptimizeExecutorFnPtr = void (*) ( OptimizeTask_t );
volatile OptimizeExecutorFnPtr& OptimizeExecutor() noexcept;

#endif // _sphinxrt_
